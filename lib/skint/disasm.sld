
(define-library (skint disasm)

  (import
    (scheme base)
    (scheme cxr)
    (only (skint) box? unbox)
    (only (skint hidden)
      instruction-table
      deserialize-code
      write-serialized-arg
      closure->vector
      global-store
      lookup-global
      lookup-integrable
      integrable?
      integrable-code
      integrable-type
      integrable-global
      sexp-match?))

  (export
    da
    da-code
    da-bytecode
    da-core
    da-procedure
    da-name
    da-global
    da-prune-globals
    da-void-for-empty-begin)

(begin

;; ---------------------------------------------------------------------------
;; Reverse table: instruction word -> (encoding . etyp)
;; ---------------------------------------------------------------------------

(define %tbl '())
(define halt-word #f)
(define br-word #f)

(define (lookup w) (let ([a (assq w %tbl)]) (and a (cdr a))))
(define (enc-of w) (car (lookup w)))
(define (etyp-of w) (cdr (lookup w)))

;; Entries whose encoding is #f are the instructions the compiler never emits:
;; halt, which the decoder appends at end of stream, and br, which it synthesizes
;; between the arms of a two-way branch.  One decode of the empty string tells
;; them apart.
(define (build-table!)
  (let* ([t (instruction-table)] [n (vector-length t)])
    (set! halt-word (vector-ref (deserialize-code "") 0))
    (let loop ([i 0] [named 0] [anon '()])
      (if (>= i n)
          (begin
            (set! br-word (let pick ([l anon])
                            (cond [(null? l) #f]
                                  [(eq? (car l) halt-word) (pick (cdr l))]
                                  [else (car l)])))
            named)
          (let ([w (vector-ref t i)]
                [e (vector-ref t (+ i 1))]
                [ty (vector-ref t (+ i 2))])
            (if e
                (begin (set! %tbl (cons (cons w (cons e ty)) %tbl))
                       (loop (+ i 3) (+ named 1) anon))
                (loop (+ i 3) named (cons w anon))))))))


;; ---------------------------------------------------------------------------
;; Global boxes back to their symbols
;;
;; Decoding a string interns any global it mentions, so the map goes stale in
;; use.  The store is a hash table whose buckets grow by consing at the front,
;; so a refresh only has to walk the new heads: keep a shallow copy of the
;; bucket vector, and for each bucket that is not eq? to its copy, scan the new
;; entries until the tail is eq? to the copy's list.  (User's algorithm.)
;; ---------------------------------------------------------------------------

(define %boxes '())        ; alist: box -> symbol
(define %store-copy #f)    ; shallow copy of the bucket-head vector

(define (refresh-boxes!)
  (let* ([gs (global-store)] [n (vector-length gs)])
    (unless %store-copy (set! %store-copy (make-vector n '())))
    (let loop ([i 0] [added 0])
      (if (>= i n)
          added
          (let ([cur (vector-ref gs i)] [old (vector-ref %store-copy i)])
            (if (eq? cur old)
                (loop (+ i 1) added)               ; bucket untouched
                (let scan ([l cur] [added added])
                  ;; the null? test only matters if a bucket were ever rebuilt
                  ;; rather than prepended; then this degrades to a full re-add
                  (if (or (null? l) (eq? l old))
                      (begin (vector-set! %store-copy i cur)
                             (loop (+ i 1) added))
                      (begin
                        (set! %boxes (cons (cons (cdar l) (caar l)) %boxes))
                        (scan (cdr l) (+ added 1)))))))))))

(define (box->symbol b)
  (let ([a (assq b %boxes)])
    (cond [a (cdr a)]
          [else (refresh-boxes!)
                (let ([a (assq b %boxes)])
                  (if a (cdr a) (error "da-code: global box with no name" b)))])))


;; ---------------------------------------------------------------------------
;; da-code
;; ---------------------------------------------------------------------------

(define (%da-code cv)
  (let ([out (open-output-string)])
    (define (arg x) (write-serialized-arg x out))
    (define (emit s) (write-string s out))
    (define (walk cv i end)
      (when (< i end)
        (let ([w (vector-ref cv i)])
          (cond
            ;; the decoder appends halt at end of stream; drop it again
            [(eq? w halt-word) (walk cv (+ i 1) end)]
            [(not (lookup w))
             (error "da-code: unknown instruction word at index" i)]
            [else
             (let ([e (enc-of w)] [ty (etyp-of w)])
               (emit e)
               (cond
                 [(eqv? ty 0) (walk cv (+ i 1) end)]
                 [(eqv? ty 1) (arg (vector-ref cv (+ i 1))) (walk cv (+ i 2) end)]
                 [(eqv? ty 2) (arg (vector-ref cv (+ i 1)))
                              (arg (vector-ref cv (+ i 2)))
                              (walk cv (+ i 3) end)]
                 ;; a global reference carries its box; recover the name
                 [(eqv? ty #\g) (arg (box->symbol (vector-ref cv (+ i 1))))
                                (walk cv (+ i 2) end)]
                 ;; andbo swallows the following zero-operand instruction
                 [(eqv? ty #\a) (emit (enc-of (vector-ref cv (+ i 1))))
                                (walk cv (+ i 2) end)]
                 ;; save: operand is the block length, block spliced inline
                 [(eqv? ty #\s)
                  (let* ([n (vector-ref cv (+ i 1))] [b (+ i 2)])
                    (emit "{") (walk cv b (+ b n)) (emit "}")
                    (walk cv (+ b n) end))]
                 ;; dclose: an operand, then a nested code vector
                 [(eqv? ty #\d)
                  (arg (vector-ref cv (+ i 1)))
                  (let ([sub (vector-ref cv (+ i 2))])
                    (emit "{") (walk sub 0 (vector-length sub)) (emit "}"))
                  (walk cv (+ i 3) end)]
                 ;; branch: one arm, or two with a synthesized br between them.
                 ;; the two-arm layout is  brnot n1+2  arm1  br  n2  arm2
                 [(eqv? ty #\b)
                  (let* ([k (vector-ref cv (+ i 1))] [b (+ i 2)] [stop (+ b k)])
                    (if (and (>= k 2) (eq? (vector-ref cv (- stop 2)) br-word))
                        (let ([n2 (vector-ref cv (- stop 1))])
                          (emit "{") (walk cv b (- stop 2)) (emit "}")
                          (emit "{") (walk cv stop (+ stop n2)) (emit "}")
                          (walk cv (+ stop n2) end))
                        (begin
                          (emit "{") (walk cv b stop) (emit "}")
                          (walk cv stop end))))]
                 [else (error "da-code: unhandled etyp" ty)]))]))))
    (walk cv 0 (vector-length cv))
    (get-output-string out)))

;; the code vector of a live procedure
(define (procedure-code p) (vector-ref (closure->vector p) 0))

;; --- what an entry point will take -----------------------------------------
;; Every stage of the disassembler can start from further up: a procedure, or the
;; global name of one, stands for its code vector, which stands for its bytecode,
;; which stands for its Core.  These reduce an argument to what the next stage
;; needs and answer #f when it cannot be got there, so a wrong type is a #f and
;; only a malformed value of the right type is an error.

;; The index of an integrable, standing for the global name its generated
;; wrapper procedure is filed under -- a convenience for typing at a REPL, where
;; an index is what the Core of a disassembly shows.  integrable? is the only one
;; of the three accessors that is total over indices, so it is asked first.
(define (integrable-name x)
  (and (integer? x)
       (exact? x)
       (>= x 0)
       (integrable? x)
       (let ([n (guard (e (#t #f)) (integrable-global x))])
         (and (symbol? n) n))))

;; the global name X stands for: itself if it is one, or the one an integrable
;; index leads to
(define (global-name-of x)
  (cond [(symbol? x) x]
        [(integrable-name x)]
        [else #f]))

;; a live procedure, or #f.  A symbol is a global name and an exact non-negative
;; integer is an integrable index that leads to one: the store is only read, so
;; asking about a name it has never seen leaves it unseen.
(define (as-procedure x)
  (cond [(procedure? x) x]
        [(global-name-of x)
         => (lambda (n)
              (let ([loc (lookup-global n)])
                (and loc (let ([v (unbox loc)]) (and (procedure? v) v)))))]
        [else #f]))

;; a code vector, or #f.  Only a closure has one; a continuation or a primitive
;; entered from C does not, and closure->vector says so by failing, which is a
;; #f here rather than an error.
(define (as-code-vector x)
  (cond [(vector? x) x]
        [(as-procedure x)
         => (lambda (p)
              (let ([v (guard (e (#t #f)) (closure->vector p))])
                (and (vector? v) (> (vector-length v) 0)
                     (let ([cv (vector-ref v 0)]) (and (vector? cv) cv)))))]
        [else #f]))

;; a bytecode string, or #f
(define (as-bytecode x)
  (cond [(string? x) x]
        [(as-code-vector x) => %da-code]
        [else #f]))

;; A bytecode string that decodes back to the code vector of X, or #f when no
;; code vector can be got out of X at all.  A malformed code vector is an error.
(define (da-code x)
  (let ([cv (as-code-vector x)])
    (and cv (%da-code cv))))


;; ---------------------------------------------------------------------------
;; Checks

;; ---------------------------------------------------------------------------
;; da-bytecode : bytecode string -> a Core expression that compiles back to it
;;
;; Inverse of codegen.  The bytecode is a postfix encoding of the Core tree, so
;; this is a stack machine whose values are Core expressions rather than data.
;; ---------------------------------------------------------------------------

;; --- decomposing fused encodings -------------------------------------------
;; A fused encoding is a base encoding plus baked-in operand digits, optionally
;; followed by ^ (unbox) and/or , (push).  Anything else -- "!" or a letter --
;; makes a distinct instruction, not a fusion.  See doc/internals/bytecode.md.

(define (digit? c) (and (char>=? c #\0) (char<=? c #\9)))

;; => (list base-enc baked-digits post pre) where post is a list of 'iref /
;; 'push applied after the instruction, and pre one applied before it
(define (normalize enc0)
  (let* ([pre (if (and (> (string-length enc0) 1)
                       (char=? (string-ref enc0 0) #\,)
                       (enc->word (substring enc0 1 (string-length enc0))))
                  (list 'push)
                  '())]
         [enc (if (null? pre) enc0 (substring enc0 1 (string-length enc0)))]
         [n (string-length enc)]
         [post '()])
    ;; peel trailing , and ^ as long as what is left is still an encoding
    (let peel ([n n])
      (let ([c (string-ref enc (- n 1))])
        (cond [(and (> n 1) (or (char=? c #\,) (char=? c #\^))
                    (enc->word (substring enc 0 (- n 1))))
               (set! post (cons (if (char=? c #\,) 'push 'iref) post))
               (peel (- n 1))]
              [else
               ;; Now peel trailing digits while the remainder is an encoding.
               ;; A digit only counts as a baked-in operand when what is left is
               ;; the same operation with an operand removed: zerop's "=0" is not
               ;; eq's "=" with a 0 baked in, it is a different builtin, so an
               ;; encoding that names an integrable of its own is left alone.
               (let dig ([m n] [ds '()])
                 (cond [(and (> m 1) (digit? (string-ref enc (- m 1)))
                             (enc->word (substring enc 0 (- m 1)))
                             (not (enc->integrable (substring enc 0 m))))
                        (dig (- m 1) (cons (- (char->integer (string-ref enc (- m 1)))
                                              (char->integer #\0))
                                           ds))]
                       [else (list (substring enc 0 m) ds post pre)]))])))))

;; encoding -> word, for the decomposition above
(define %enc->word '())
(define (enc->word e) (let ([a (assoc e %enc->word)]) (and a (cdr a))))
(define (build-enc-map!)
  (let loop ([l %tbl] [n 0])
    (if (null? l)
        n
        (begin (set! %enc->word (cons (cons (car (cdr (car l))) (car (car l)))
                                      %enc->word))
               (loop (cdr l) (+ n 1))))))

;; instruction word -> (base-enc baked post), computed once
(define %norm '())
(define (norm-of w) (cdr (assq w %norm)))
(define (build-norm!)
  (let loop ([l %tbl] [n 0])
    (if (null? l)
        n
        (begin (set! %norm (cons (cons (car (car l)) (normalize (car (cdr (car l)))))
                                 %norm))
               (loop (cdr l) (+ n 1))))))


;; --- synthesized names ------------------------------------------------------
;; .a .b .c ... for stack locals, :a :b :c ... for display variables, and a ^
;; prefix when the variable is boxed, so ^.a is a local that gets assigned.
;; Globals keep their real names.  A name is fixed where the variable is bound,
;; so an inner lambda refers to an outer local by its .x name, not by a :x one --
;; the : series is for display slots with no binder in the fragment being read.

(define %lctr 0)
(define %dctr 0)

(define (letters n)          ; 0 -> "a", 25 -> "z", 26 -> "aa", ...
  (let loop ([n n] [s ""])
    (let ([s (string-append (string (integer->char (+ (char->integer #\a)
                                                     (remainder n 26))))
                            s)])
      (if (< n 26) s (loop (- (quotient n 26) 1) s)))))

(define (local-name boxed?)
  (let ([s (string-append "." (letters %lctr))])
    (set! %lctr (+ %lctr 1))
    (string->symbol (if boxed? (string-append "^" s) s))))

(define (display-name boxed?)
  (let ([s (string-append ":" (letters %dctr))])
    (set! %dctr (+ %dctr 1))
    (string->symbol (if boxed? (string-append "^" s) s))))

;; A display list that grows on demand, for fragments whose free variables have
;; no binder in view.  The second cell records which slots were dereferenced --
;; that is, which of the closure's free variables are boxed because something
;; assigns them.  A caller holding the actual display needs to know: a boxed
;; slot holds a box, an unboxed one holds the value itself.
(define (make-dsp l) (list l (quote ())))
(define (dsp-boxed d) (cadr d))
(define (dsp-ref d n boxed?)
  (let grow ()
    (when (<= (length (car d)) n)
      (set-car! d (append (car d) (list (display-name #f))))
      (grow)))
  (when (and boxed? (not (memv n (cadr d))))
    (set-car! (cdr d) (cons n (cadr d))))
  (list-ref (car d) n))


;; --- symbolic slots ---------------------------------------------------------
;; a slot is (v . name) for a named variable, (t . expr) for a pushed temp,
;; (b name . expr) for a temp that turned out to be a binding, or (c . name)
;; for the variable a letcc binds

;; a synthesized name marked as holding a box
(define (boxed-name? nm)
  (let ([s (symbol->string nm)])
    (and (> (string-length s) 0) (char=? (string-ref s 0) #\^))))
;; the display of a closure is filled from pushes of its free variables
(define (free-name e)
  (if (and (pair? e) (memq (car e) (quote (ref set&))) (pair? (cdr e)) (symbol? (cadr e)))
      (cadr e)
      (give-up-on (quote display))))


(define (slot-var name) (cons 'v name))
(define (slot-tmp e) (cons 't e))
(define (slot-cc name) (cons 'c name))
(define (slot-name s) (cdr s))
;; (a b c) with x => (a b x)
(define (repl-last l x)
  (if (null? (cdr l)) (list x) (cons (car l) (repl-last (cdr l) x))))

;; Some instructions live only in hand-written procedures -- call/cc,
;; call-with-values, values and friends -- and are never emitted by codegen.
;; Bytecode containing one has no Core preimage, so da-bytecode gives up.
(define %bail #f)
(define %why #f)
(define (give-up) (%bail #f))
(define (give-up-on e) (set! %why e) (%bail #f))


;; --- reading one instruction --------------------------------------------------
;; => (base ops next post pre), with the baked-in operand digits of a fused
;; encoding and the ones stored inline after the instruction word merged

(define (decode-at cv i)
  (let* ([w (vector-ref cv i)]
         [nm (if (lookup w) (norm-of w) (give-up-on (quote unknown-word)))]
         [baked (cadr nm)]
         ;; The operands of a fused encoding belong to the instruction it is
         ;; fused from: jdref is jdcge with the first operand baked in, so the
         ;; count comes from the base encoding, not from the fused word.
         [ty (etyp-of (enc->word (car nm)))]
         ;; how many operand slots sit inline after the instruction word:
         ;; g is a box, d/s/b each carry a count, a carries none
         [ninline (cond [(eqv? ty 0) 0] [(eqv? ty 1) 1] [(eqv? ty 2) 2]
                        [(eqv? ty #\g) 1] [(eqv? ty #\d) 1]
                        [(eqv? ty #\s) 1] [(eqv? ty #\b) 1]
                        [else 0])]
         [need (- ninline (length baked))]
         [ops (append baked
                      (let g ([j 0] [r '()])
                        (if (>= j need) (reverse r)
                            (g (+ j 1) (cons (vector-ref cv (+ i 1 j)) r)))))])
    (list (car nm) ops (+ i 1 (max 0 need)) (caddr nm) (list-ref nm 3))))

;; --- stepping over an instruction, structure and all --------------------------
;; save splices its block inline, dclose stores a nested code vector in one cell,
;; a branch splices its arm, and andbo swallows the instruction after it

(define (instr-next cv i)
  (let* ([d (decode-at cv i)] [ops (cadr d)] [next (caddr d)]
         [ty (etyp-of (enc->word (car d)))])
    (cond [(eqv? ty #\s) (+ next (car ops))]
          [(eqv? ty #\b) (+ next (car ops))]
          [(eqv? ty #\d) (+ next 1)]
          [(eqv? ty #\a) (+ next 1)]
          [else next])))

;; A branch with one arm is either a tail if, whose else arm is the code after
;; the block, or a statement if with a void else arm, where that code is simply
;; what follows the whole form.  They are told apart by the arm: codegen gives a
;; tail arm a return or a tail call, and one is never left to fall through.
(define (arm-falls-through? cv b stop)
  (let loop ([i b] [last #f])
    (if (>= i stop)
        (not (and last (or (string=? last "]") (string=? last "[")
                           (string=? last "w") (string=? last "w!")
                           (string=? last "%%"))))
        (loop (instr-next cv i) (car (decode-at cv i))))))

;; --- lambda* ------------------------------------------------------------------
;; A case-lambda closure has no arity preamble: its block is a run of jdceq /
;; jdcge entries dispatching on the argument count into the display, one entry
;; per clause, ending in aerr.  The clause bodies are the display itself.

(define (lambda*-block? cv)
  (and (> (vector-length cv) 0)
       (let ([w (vector-ref cv 0)])
         (and (lookup w)
              (let ([b (car (norm-of w))])
                (or (string=? b "|") (string=? b "|!")))))))

;; => the clause arities, as codegen's (count rest?) pairs, in clause order
(define (lambda*-arities cv)
  (let loop ([i 0] [r '()])
    (if (>= i (vector-length cv))
        (give-up-on (quote arity-run))
        (let* ([d (decode-at cv i)] [base (car d)] [ops (cadr d)] [next (caddr d)])
          (cond [(string=? base "%%") (reverse r)]
                [(string=? base "|") (loop next (cons (list (car ops) #f) r))]
                [(string=? base "|!") (loop next (cons (list (car ops) #t) r))]
                [else (give-up-on (quote arity-entry))])))))

;; each clause was checked with %ckx before being pushed into the display
(define (ckx-body e)
  (if (and (pair? e) (eq? (car e) (quote integrable))
           (pair? (cddr e)) (null? (cdr (cddr e))))
      (car (cddr e))
      (give-up-on (quote ckx))))

;; --- the decompiler ---------------------------------------------------------
;; walks cv[i..end) with symbolic stack stk and display names dsp;
;; returns the Core expression the region leaves in the accumulator

(define (dcmp cv i end stk dsp fbase)
  (define ac #f)
  (define live #f)      ; ac holds a value nothing has consumed yet
  (define stmts (quote ()))  ; values discarded before use: a begin sequence
  ;; a temp slot referenced as a variable becomes one, so that a let-like call
  ;; can be rebuilt from the bindings when its adrop closes the frame
  ;; the stack is only as deep as the bytecode says; a mismatch means the
  ;; bytecode did not come from codegen, so give up rather than crash
  (define (nth j)
    (if (or (< j 0) (>= j (length stk))) (give-up-on (quote stack)) (list-ref stk j)))
  (define (drop! j)
    (if (or (< j 0) (> j (length stk))) (give-up-on (quote stack))
        (set! stk (list-tail stk j))))
  (define (car-slot) (if (null? stk) (give-up-on (quote stack)) (car stk)))
  ;; A reference to a boxed variable carries a ^; a bare reference to one is
  ;; either codegen's set& -- the box itself, which is what the builtins taking
  ;; an out parameter want -- or the push of a free variable into a display,
  ;; which codegen also emits without the dereference.  Both read back as set&:
  ;; the display push is consumed by dclose and never reaches the Core.
  (define (ref-slot n boxed?)
    (let ([s (nth n)])
      (when (eq? (car s) (quote t))
        (let ([nm (local-name (and boxed? #t))])
          (set-cdr! s (cons nm (cdr s)))
          (set-car! s (quote b))))
      (let ([nm (cond [(eq? (car s) (quote v)) (cdr s)]
                      [(eq? (car s) (quote b)) (cadr s)]
                      [(eq? (car s) (quote c)) (cdr s)]
                      [else #f])])
        (cond [(not nm) (cdr s)]
              [(and (not boxed?) (boxed-name? nm)) (list (quote set&) nm)]
              [else (list (quote ref) nm)]))))
  ;; the save frame slots hold no expression; if one is ever read as a value the
  ;; stack model has drifted, which means this was not codegen's output
  (define (slot-expr s)
    (let ([e (cond [(eq? (car s) (quote b)) (cddr s)] [else (cdr s)])])
      (or e (give-up-on (quote placeholder)))))
  ;; the name a slot is known by, promoting a temp if it is being assigned
  (define (slot-varname n)
    (let ([s (nth n)])
      (when (eq? (car s) (quote t))
        (let ([nm (local-name #t)])
          (set-cdr! s (cons nm (cdr s)))
          (set-car! s (quote b))))
      (cond [(eq? (car s) (quote b)) (cadr s)]
            [else (cdr s)])))
  ;; a region that computed nothing into ac is an empty begin: codegen emits no
  ;; code for one, and the accumulator is left holding whatever the test left
  ;; Every push is remembered by stack depth, together with the statements that
  ;; were pending when it happened: those ran before the frame was opened, so
  ;; they belong outside the let-like call the frame turns into.  A push is
  ;; also flagged when its expression already refers to a slot of the frame
  ;; below it -- bindings pushed as one frame cannot see each other, codegen
  ;; compiles their expressions against placeholder slots, so such a push must
  ;; have come from a let-like call nested inside that one.
  (define marks (quote ()))
  (define (prune-marks! lvl)
    (set! marks (let f ([l marks])
                  (cond [(null? l) (quote ())]
                        [(> (car (car l)) lvl) (f (cdr l))]
                        [else (cons (car l) (f (cdr l)))]))))
  (define (mark-at d)
    (let f ([l marks])
      (cond [(null? l) #f]
            [(= (car (car l)) d) (car l)]
            [else (f (cdr l))])))
  (define (nested-here?)
    (let scan ([l stk] [k (- (length stk) fbase)])
      (cond [(<= k 0) #f]
            [(eq? (car (car l)) (quote b)) #t]
            [else (scan (cdr l) (- k 1))])))
  (define (mark! nested)
    (prune-marks! (- (length stk) 1))
    (set! marks (cons (list (length stk) nested stmts) marks)))
  (define (push-slot! e)
    (let ([nested (nested-here?)])
      (set! stk (cons (slot-tmp e) stk))
      (mark! nested)
      (set! live #f)))
  ;; the continuation letcc binds is a frame of its own
  (define (push-cc! name)
    (set! stk (cons (slot-cc name) stk))
    (mark! #t)
    (set! live #f))
  ;; wrap a value in a list of pending statements, newest first
  (define (wrap e ss)
    (cond [(and e (null? ss)) e]
          [(null? ss) (list (quote begin))]
          [e (cons (quote begin) (reverse (cons e ss)))]
          [else (cons (quote begin) (reverse ss))]))
  ;; the pending statements newer than a saved tail
  (define (stmts-upto tail)
    (let f ([l stmts])
      (cond [(or (null? l) (eq? l tail)) (quote ())]
            [else (cons (car l) (f (cdr l)))])))
  ;; the accumulator as an expression: an assignment whose value expression
  ;; emitted no code at all was an empty begin
  (define (acval) (if live ac (list (quote begin))))
  (define (seq e) (wrap e stmts))
  (define (take-seq e) (let ([r (seq e)]) (set! stmts (quote ())) r))
  (define (result) (seq (if live ac #f)))
  ;; A let-like call in tail position has no adrop: its frame is simply included
  ;; in the count the return drops.  A letcc binder closes on its own; a run of
  ;; ordinary bindings closes as one let-like call, which is what codegen
  ;; produced in the first place -- but only down to the nearest nested frame.
  ;; Statements pending since the frame's first push go into its body, the older
  ;; ones stay outside.
  (define (close-count v n0)
    (let loop ([cur v] [n n0])
      (cond
        [(<= n 0) cur]
        [(eq? (car (car-slot)) 'c)
         (let* ([nm (cdr (car-slot))]
                [mk (mark-at (length stk))]
                [tail (if mk (car (cddr mk)) (quote ()))]
                [inner (wrap cur (stmts-upto tail))])
           (set! stmts tail)
           (set! stk (cdr stk))
           (prune-marks! (length stk))
           (loop (list (quote letcc) nm inner) (- n 1)))]
        [else
         (let* ([lvl (length stk)]
                [brk (let f ([l marks] [best #f])
                       (cond [(null? l) best]
                             [(and (cadr (car l)) (<= (car (car l)) lvl)
                                   (or (not best) (> (car (car l)) best)))
                              (f (cdr l) (car (car l)))]
                             [else (f (cdr l) best)]))]
                [room (if brk (+ (- lvl brk) 1) (- lvl fbase))]
                [m (let g ([j 0])
                     (if (or (>= j n) (>= j room)
                             (eq? (car (nth j)) (quote c)))
                         j
                         (g (+ j 1))))]
                [frame (let g ([j 0] [r (quote ())])
                         (if (>= j m) (reverse r)
                             (g (+ j 1) (cons (nth j) r))))]
                [names (map (lambda (s)
                              (if (eq? (car s) (quote b)) (cadr s) (local-name #f)))
                            frame)]
                [exprs (map slot-expr frame)]
                [mk (mark-at (- lvl (- m 1)))]
                [tail (if mk (car (cddr mk)) (quote ()))]
                [inner (wrap cur (stmts-upto tail))])
           (when (<= m 0) (give-up-on (quote frame)))
           (set! stmts tail)
           (drop! m)
           (prune-marks! (length stk))
           (loop (cons (quote call) (cons (list (quote lambda) names inner) exprs))
                 (- n m)))])))
  ;; close everything this region owns
  (define (close-frames v) (close-count v (- (length stk) fbase)))
  (let loop ([i i])
    (if (>= i end)
        (result)
        (let* ([w (vector-ref cv i)])
          (cond
            [(eq? w halt-word) (loop (+ i 1))]
            [else
             (let* ([d (decode-at cv i)]
                    [base (car d)] [ops (cadr d)] [next (caddr d)]
                    [post (list-ref d 3)] [pre (list-ref d 4)])
               ;; an instruction that consumes ac and replaces it
               (define (finish e j)
                 (set! ac e)
                 (set! live #t)
                 (let post-loop ([p post])
                   (cond [(null? p) (loop j)]
                         [(eq? (car p) 'iref) (post-loop (cdr p))]
                         [(eq? (car p) 'push)
                          (push-slot! ac)
                          (post-loop (cdr p))])))
               ;; a leaf: whatever was in ac was never used, so it was a
               ;; statement in a begin sequence
               (define (newval e j)
                 (when live (set! stmts (cons ac stmts)))
                 (finish e j))
               ;; a fused leading push happens before the instruction runs
               (unless (null? pre)
                 (push-slot! (if live ac (list (quote begin)))))
               (cond
                 ;; --- leaves ---
                 [(string=? base "'") (newval (list 'quote (car ops)) next)]
                 [(string=? base "f") (newval ''#f next)]
                 [(string=? base "t") (newval ''#t next)]
                 [(string=? base "n") (newval ''() next)]
                 [(string=? base ".") (newval (ref-slot (car ops) (memq (quote iref) post)) next)]
                 [(string=? base ":") (newval (list 'ref (dsp-ref dsp (car ops) (memq (quote iref) post))) next)]
                 ;; iref is normally fused onto the reference before it; on its
                 ;; own it says that reference was to a box and wanted the value,
                 ;; which turns a set& reading back into an ordinary ref
                 [(string=? base "^")
                  (if (and (pair? ac) (eq? (car ac) (quote set&)))
                      (finish (list (quote ref) (cadr ac)) next)
                      (loop next))]
                 [(string=? base "@") (newval (list 'ref (box->symbol (car ops))) next)]
                 [(string=? base "`") (newval (list 'set& (box->symbol (car ops))) next)]
                 ;; --- push ---
                 ;; a push with nothing fresh in ac is codegen pushing the value
                 ;; of an expression that emitted no code at all: an empty begin
                 [(string=? base ",")
                  (push-slot! (if live ac (list (quote begin))))
                  (loop next)]
                 ;; --- assignment ---
                 [(string=? base "@!")
                  (finish (list (quote gset!) (box->symbol (car ops)) (acval)) next)]
                 [(string=? base ".!")
                  (finish (list (quote set!) (slot-varname (car ops)) (acval)) next)]
                 [(string=? base ":!")
                  (finish (list (quote set!) (dsp-ref dsp (car ops) #t) (acval)) next)]
                 ;; --- arity preamble and boxing markers: no Core of their own ---
                 [(string=? base "%") (loop next)]
                 [(string=? base "%!") (loop next)]
                 ;; --- andbo: a comparison chain.  Every argument but the first
                 ;; and the last was pushed twice; the instruction before this
                 ;; one consumed the copy and this one pops the original, so the
                 ;; whole run is a single n-ary integrable.  The copy carried no
                 ;; expression, so put the original in its place.
                 [(string=? base ";")
                  (let* ([w2 (vector-ref cv next)]
                         [nm2 (and (lookup w2) (norm-of w2))]
                         [ig2 (and nm2 (enc->integrable (car nm2)))])
                    (unless (and ig2 (pair? ac) (eq? (car ac) (quote integrable))
                                 (eqv? (car (cdr ac)) ig2)
                                 (pair? stk) (pair? (cdr stk)))
                      (give-up-on (quote andbo)))
                    (let* ([real (slot-expr (car-slot))]
                           [rest (cdr stk)]
                           [newarg (slot-expr (car rest))])
                      (set! stk (cdr rest))
                      (finish (append (repl-last ac real) (list newarg)) (+ next 1))))]
                 ;; sbox marks a binding the body assigns: its slot holds a box
                 [(string=? base "#")
                  (let ([sl (nth (car ops))])
                    (when (eq? (car sl) (quote t))
                      (set-cdr! sl (cons (local-name #t) (cdr sl)))
                      (set-car! sl (quote b))))
                  (loop next)]
                 ;; --- adrop closes a let-like call: the dropped slots were its
                 ;; argument frame, and any of them the body referenced by name
                 ;; became a binding
                 [(string=? base "_")
                  ;; the count says exactly how many slots the frame holds, but
                  ;; never more than this region owns -- a non-tail letcc drops
                  ;; its variable together with the two save frame slots
                  (let* ([n (car ops)]
                         [avail (max 0 (- (length stk) fbase))]
                         [m (if (< n avail) n avail)]
                         ;; the statements pending before the frame was opened
                         ;; belong to the sequence around this value, not to it
                         [body (close-count (if live ac #f) m)])
                    (drop! (- n m))
                    (finish body next))]
                 ;; --- letcc: capture, push the variable, run the body.  Both
                 ;; forms are followed by a push and, if the variable is
                 ;; assigned, a # marker; the body is closed by the return or
                 ;; the adrop that drops the variable again.
                 [(or (string=? base "k") (string=? base "k0"))
                  (let* ([j next]                       ; the push
                         [j (+ j 1)]
                         [bx (and (< j end)
                                  (let ([w2 (vector-ref cv j)])
                                    (and (lookup w2)
                                         (string=? (car (norm-of w2)) "#"))))]
                         [j (if bx (+ j (if (null? (cadr (norm-of (vector-ref cv j)))) 2 1)) j)])
                    (push-cc! (local-name bx))
                    (loop j))]
                 ;; --- withcc: the continuation is in ac, the body under it ---
                 [(string=? base "w!")
                  (let ([e (slot-expr (car-slot))])
                    (set! stk (cdr stk))
                    (finish (list 'withcc ac e) next))]
                 [(string=? base "w")
                  ;; the body was thunked, so unwrap the (lambda () x)
                  (let* ([th (slot-expr (car-slot))]
                         [e (if (and (pair? th) (eq? (car th) 'lambda)) (caddr th) th)])
                    (set! stk (cdr stk))
                    (finish (list 'withcc ac e) next))]
                 ;; --- return ---
                 [(string=? base "]") (take-seq (close-frames (if live ac #f)))]
                 ;; --- branch ---
                 ;; Three shapes come out of codegen.  Two arms with a br between
                 ;; them is a non-tail if: its value goes on being used, so the
                 ;; walk continues after the second arm.  One arm that falls
                 ;; through is a statement if with a void else arm -- the failed
                 ;; test leaves #f in ac and the walk continues after the block.
                 ;; One arm that returns is a tail if, and everything left in the
                 ;; region is its else arm.
                 [(or (string=? base "?") (string=? base "~?"))
                  (let* ([cexp (if (string=? base "?")
                                   ac
                                   (list (quote integrable)
                                         (lookup-integrable (quote not)) ac))]
                         [kk (car ops)] [b next] [stop (+ b kk)]
                         [fb (length stk)])
                    (cond
                      [(and (>= kk 2) (eq? (vector-ref cv (- stop 2)) br-word))
                       (let* ([n2 (vector-ref cv (- stop 1))]
                              [t (dcmp cv b (- stop 2) stk dsp fb)]
                              [e (dcmp cv stop (+ stop n2) stk dsp fb)])
                         (finish (list (quote if) cexp t e) (+ stop n2)))]
                      [(and (< stop end) (arm-falls-through? cv b stop))
                       (let ([t (dcmp cv b stop stk dsp fb)])
                         (finish (list (quote if) cexp t (list (quote begin))) stop))]
                      [else
                       (let* ([t (dcmp cv b stop stk dsp fb)]
                              [e (if (>= stop end)
                                     (list (quote begin))
                                     (dcmp cv stop end stk dsp fb))])
                         (take-seq (close-frames (list (quote if) cexp t e))))]))]
                 ;; --- non-tail call frame ---
                 [(string=? base "$")
                  (let* ([n (car ops)] [b next]
                         ;; save pushes two frame slots
                         [inner (dcmp cv b (+ b n)
                                      (cons (slot-tmp #f) (cons (slot-tmp #f) stk))
                                      dsp (+ (length stk) 2))])
                    (newval inner (+ b n)))]
                 ;; --- closure ---
                 [(string=? base "&")
                  (let* ([nfree (car ops)] [sub (vector-ref cv next)])
                    (cond
                      ;; a case-lambda: the display holds the clause bodies
                      [(lambda*-block? sub)
                       (let* ([arities (lambda*-arities sub)]
                              [bodies (let g ([j 0] [r '()])
                                        (if (>= j nfree) (reverse r)
                                            (g (+ j 1)
                                               (cons (ckx-body (slot-expr (nth j)))
                                                     r))))])
                         (unless (= (length arities) nfree) (give-up-on (quote arity-count)))
                         (drop! nfree)
                         (newval (cons (quote lambda*) (map list arities bodies))
                                 (+ next 1)))]
                      [else
                       (let ([freenames (let g ([j 0] [r '()])
                                          (if (>= j nfree) (reverse r)
                                              (g (+ j 1)
                                                 (cons (free-name (slot-expr (nth j))) r))))])
                         (drop! nfree)
                         (newval (dcmp-lambda sub freenames) (+ next 1)))]))]
                 ;; --- calls ---
                 ;; scall's first operand is how many stack slots the call drops
                 ;; on entry: in tail position that count covers the let-like
                 ;; frames and letcc binders the region owns, which have no adrop
                 ;; of their own.  A non-tail call has a zero count and ends its
                 ;; save block, so either way the region ends here.
                 [(string=? base "[")
                  (let* ([kdrop (car ops)] [nargs (cadr ops)] [f ac]
                         [args (let g ([j 0] [r '()])
                                 (if (>= j nargs) (reverse r)
                                     (g (+ j 1) (cons (slot-expr (nth j)) r))))])
                    (drop! nargs)
                    (prune-marks! (length stk))
                    (let* ([avail (max 0 (- (length stk) fbase))]
                           [m (if (< kdrop avail) kdrop avail)]
                           [body (take-seq (close-count (cons (quote call) (cons f args)) m))])
                      (drop! (- kdrop m))
                      body))]
                 [(string=? base "[0")
                  (let* ([nargs (car ops)] [f ac]
                         [args (let g ([j 0] [r '()])
                                 (if (>= j nargs) (reverse r)
                                     (g (+ j 1) (cons (slot-expr (nth j)) r))))])
                    (drop! nargs)
                    (finish (cons 'call (cons f args)) next))]
                 ;; --- integrables ---
                 [(enc->integrable base)
                  => (lambda (ig)
                       (let ([ty2 (integrable-type ig)])
                         (if (eqv? ty2 #\#)
                             ;; a constructor: every argument is on the stack,
                             ;; and the operand says how many.  Hand-written code
                             ;; can carry something else there -- vector's own
                             ;; body carries a list -- so this is checked and not
                             ;; assumed, or the count would be used as a number.
                             (let* ([cnt (if (and (pair? ops) (integer? (car ops)) (exact? (car ops))
                                                  (>= (car ops) 0))
                                             (car ops)
                                             (give-up-on (quote constructor-count)))]
                                    [args (let g ([j 0] [r '()])
                                            (if (>= j cnt) (reverse r)
                                                (g (+ j 1)
                                                   (cons (slot-expr (nth j)) r))))])
                               (drop! cnt)
                               (newval (cons (quote integrable) (cons ig args)) next))
                             ;; otherwise the first argument is in ac and the
                             ;; rest, if any, are on the stack.  The folding and
                             ;; comparison classes emit the instruction once per
                             ;; extra argument, so each occurrence is binary.
                             (let* ([nargs (case ty2
                                             [(#\0) 0] [(#\1) 1] [(#\2) 2] [(#\3) 3]
                                             [(#\4) 4] [(#\5) 5]
                                             [(#\u) 1] [(#\b) 2] [(#\t) 3]
                                             [else 2])]
                                    [args (if (= nargs 0)
                                              '()
                                              (cons ac
                                                    (let g ([j 0] [r '()])
                                                      (if (>= j (- nargs 1)) (reverse r)
                                                          (g (+ j 1)
                                                             (cons (slot-expr (nth j)) r))))))])
                               (drop! (max 0 (- nargs 1)))
                               ;; a builtin taking no arguments leaves whatever
                               ;; was in the accumulator unused: it is a leaf
                               ((if (= nargs 0) newval finish)
                                (cons (quote integrable) (cons ig args)) next)))))]
                 [else (give-up-on base)]))])))))

;; encoding -> integrable index, built once
(define %enc->ig '())
(define (enc->integrable e) (let ([a (assoc e %enc->ig)]) (and a (cdr a))))
(define (build-ig-map!)
  (let skip ([i 0])
    (cond [(> i 4096) 0]
          [(not (integrable? i)) (skip (+ i 1))]
          [else (let run ([j i] [n 0])
                  (if (integrable? j)
                      (let ([e (integrable-code j 0)])
                        (when (and e (> (string-length e) 0))
                          (set! %enc->ig (cons (cons e j) %enc->ig)))
                        (run (+ j 1) (+ n 1)))
                      n))])))

(define (dcmp-branch cv k b end stk dsp cexp)
  ;; the arms share the enclosing frame, so they must not close it themselves
  (let* ([stop (+ b k)] [fbase (length stk)])
    (if (and (>= k 2) (eq? (vector-ref cv (- stop 2)) br-word))
        (let* ([n2 (vector-ref cv (- stop 1))]
               [t (dcmp cv b (- stop 2) stk dsp fbase)]
               [e (dcmp cv stop (+ stop n2) stk dsp fbase)])
          (list 'if cexp t e))
        (let ([t (dcmp cv b stop stk dsp fbase)])
          (if (>= stop end)
              (list 'if cexp t '(begin))
              (list 'if cexp t (dcmp cv stop end stk dsp fbase)))))))

(define (dcmp-lambda cv freenames)
  ;; the block opens with %n or %!n giving the arity, then one # marker per
  ;; parameter that gets assigned in the body -- which is exactly what decides
  ;; whether the name carries a ^.  A hand-written block starts with neither.
  (let* ([w0 (vector-ref cv 0)]
         [nm (if (lookup w0) (norm-of w0) (give-up-on (quote block-unknown-word)))]
         [base (car nm)]
         [ok (or (string=? base "%") (string=? base "%!") (give-up-on (string-append "block:" base)))]
         [baked (cadr nm)]
         [cnt (if (null? baked) (vector-ref cv 1) (car baked))]
         [rest? (string=? base "%!")]
         [nslots (if rest? (+ cnt 1) cnt)]
         [after-arity (if (null? baked) 2 1)])
    ;; collect the # markers
    (let scan ([j after-arity] [boxed '()])
      (if (and (< j (vector-length cv))
               (let ([w (vector-ref cv j)])
                 (and (lookup w) (string=? (car (norm-of w)) "#"))))
          (let* ([w (vector-ref cv j)] [b (cadr (norm-of w))])
            (if (null? b)
                (scan (+ j 2) (cons (vector-ref cv (+ j 1)) boxed))
                (scan (+ j 1) (cons (car b) boxed))))
          ;; slot 0 is the first parameter, or the rest argument when there is one
          (let* ([ids (let g ([k 0] [r '()])
                        (if (>= k nslots)
                            (reverse r)
                            (g (+ k 1)
                               (cons (local-name (and (memv k boxed) #t)) r))))]
                 [stk (map slot-var ids)]
                 [dsp (make-dsp freenames)]
                 [ignore (unless %top-dsp (set! %top-dsp dsp))]
                 [body (dcmp cv j (vector-length cv) stk dsp nslots)])
            (list 'lambda
                  (if rest?
                      ;; flatten-idslist puts the rest argument first, so slot 0
                      ;; is the rest and slots 1.. are the required ones
                      (let f ([l (cdr ids)])
                        (if (null? l) (car ids) (cons (car l) (f (cdr l)))))
                      ids)
                  body))))))

;; --- a well-formedness check on the result ------------------------------------
;; Feeding malformed Core to the compiler raises an error a guard cannot catch,
;; so the reconstruction is checked before it is handed back: anything that does
;; not typecheck as Core is reported as no preimage at all.

(define (every? p l)
  (cond [(null? l) #t] [(not (pair? l)) #f]
        [(p (car l)) (every? p (cdr l))] [else #f]))

(define (idslist-ok? x)
  (cond [(symbol? x) #t] [(null? x) #t]
        [(pair? x) (and (symbol? (car x)) (idslist-ok? (cdr x)))]
        [else #f]))

(define (core-ok? x)
  (and (pair? x) (symbol? (car x))
       (let ([tail (cdr x)])
         (case (car x)
           [(quote) (and (pair? tail) (null? (cdr tail)))]
           [(ref gref set&)
            (and (pair? tail) (symbol? (car tail)) (null? (cdr tail)))]
           [(set! gset!)
            (and (pair? tail) (symbol? (car tail)) (pair? (cdr tail))
                 (null? (cddr tail)) (core-ok? (cadr tail)))]
           [(begin) (every? core-ok? tail)]
           [(if) (and (pair? tail) (pair? (cdr tail)) (pair? (cddr tail))
                      (null? (cdr (cddr tail))) (every? core-ok? tail))]
           [(lambda) (and (pair? tail) (idslist-ok? (car tail))
                          (pair? (cdr tail)) (null? (cddr tail))
                          (core-ok? (cadr tail)))]
           [(lambda*)
            (and (pair? tail)
                 (every? (lambda (c)
                           (and (pair? c) (pair? (car c)) (pair? (cdr c))
                                (null? (cddr c)) (core-ok? (cadr c))))
                         tail))]
           [(letcc) (and (pair? tail) (symbol? (car tail)) (pair? (cdr tail))
                         (null? (cddr tail)) (core-ok? (cadr tail)))]
           [(withcc) (and (pair? tail) (pair? (cdr tail)) (null? (cddr tail))
                          (core-ok? (car tail)) (core-ok? (cadr tail)))]
           [(call) (and (pair? tail) (every? core-ok? tail))]
           [(integrable) (and (pair? tail) (every? core-ok? (cdr tail)))]
           [(asm) (and (pair? tail) (string? (car tail)) (null? (cdr tail)))]
           [else #f]))))

;; the display list the outermost region grew, so a caller that has the actual
;; display values can bind the names this decoding invented
(define %top-dsp #f)

;; Decode a whole instruction stream: what compile-to-string produces for a
;; top-level expression, which is a &n{...} block or a bare sequence.
(define (%dcmp-stream cv)
  (let* ([dsp (make-dsp (quote ()))]
         [c (call-with-current-continuation
              (lambda (k)
                (set! %bail k) (set! %why #f)
                ;; names are per-call, so the same bytecode always reads the same
                (set! %lctr 0) (set! %dctr 0) (set! %top-dsp dsp)
                (dcmp cv 0 (vector-length cv) (quote ()) dsp 0)))])
    (cond [(not c) #f]
          [(core-ok? c) c]
          [else (set! %why (quote malformed)) #f])))

;; A code vector is framed one of two ways, and the first instruction says which:
;; the body of a procedure opens with the arity preamble -- % or %! -- and a whole
;; instruction stream never does.  This has to be decided by looking rather than
;; by trying both: the stream decoder does not always refuse a body, and where it
;; does not it reads the arity preamble as something harmless and hands back the
;; body without its lambda.
;;
;; A body has no display in view, so its free variables get invented names.
(define (arity-preamble? cv)
  (and (> (vector-length cv) 0)
       (let ([w (vector-ref cv 0)])
         (and (lookup w)
              (let ([base (car (norm-of w))])
                (or (string=? base "%") (string=? base "%!")))))))

(define (%core-of-code-vector cv)
  (if (arity-preamble? cv)
      (da-closure cv (quote ()))
      (%dcmp-stream cv)))

;; A Core expression that compiles back to the bytecode of X, or #f when the
;; bytecode has no Core preimage -- which is the answer for the hand-written and
;; generated procedures the compiler never emitted.  #f also when no bytecode can
;; be got out of X at all.  A malformed bytecode string is an error.
(define (da-bytecode x)
  (cond [(string? x) (%core-of-code-vector (deserialize-code x))]
        [(as-code-vector x) => %core-of-code-vector]
        [else #f]))

;; Decompile a code vector that is a closure body, given names for its display.
;; Unlike da-bytecode this needs no wrapper block, so the caller decides what
;; the free variables are called and can match them up with the real display.
(define (da-closure cv freenames)
  (let* (
         [c (call-with-current-continuation
              (lambda (k)
                (set! %bail k) (set! %why #f)
                (set! %lctr 0) (set! %dctr (length freenames))
                (set! %top-dsp #f)
                (dcmp-lambda cv freenames)))])
    (cond [(not c) #f]
          [(core-ok? c) c]
          [else (set! %why (quote malformed)) #f])))

;; ---------------------------------------------------------------------------
;; da-core : a Core expression -> a Scheme form that expands back to it
;;
;; Inverse of the expander, as far as that can be inverted.  Every derived form
;; the expander erases is put back: let and let*, letrec and letrec*, let-values
;; and let*-values, internal definitions including define-values, named let, do,
;; cond, case, and, or, when, unless.  The test is not that the Scheme reads the
;; same as the original -- names are gone by this point -- but that re-expanding
;; it gives back the same program, which is checked by comparing compiled
;; bytecode.
;; ---------------------------------------------------------------------------

;; --- names ------------------------------------------------------------------

;; Globals reach Core under their store names, which carry the prefix of the
;; environment that allocated them: `repl://?x', `lib://skint/print?pp*',
;; `prog://<module>?x'.  The full name is what the store actually holds and is
;; worth seeing sometimes; the bare identifier is what reads like source.  This
;; chooses.
;;
;; Pruning is also what makes the round trip work for names the REPL allocated:
;; written back in full, `repl://?x' is an identifier like any other and
;; re-expanding it yields `repl://?repl://?x'.  Names belonging to a library do
;; not round-trip either way -- only an import can reach them.
(define da-prune-globals (make-parameter #t (lambda (x) (and x #t))))

;; A store name is <prefix>?<identifier> with `://' inside the prefix.  The
;; prefix is %-encoded, so it cannot hold a literal ?; the first one after the
;; `://' therefore ends it.  Anything else -- a built-in name like `car', or a
;; library's own initialisation flag -- is returned untouched.
(define (demangle sym)
  (if (not (da-prune-globals))
      sym
      (let* ([s (symbol->string sym)] [n (string-length s)])
        (let scan ([i 0])
          (cond
            [(> (+ i 3) n) sym]
            [(and (char=? (string-ref s i) #\:)
                  (char=? (string-ref s (+ i 1)) #\/)
                  (char=? (string-ref s (+ i 2)) #\/))
             (let mark ([j (+ i 3)])
               (cond [(>= j n) sym]
                     [(char=? (string-ref s j) #\?)
                      (if (>= (+ j 1) n) sym (string->symbol (substring s (+ j 1) n)))]
                     [else (mark (+ j 1))]))]
            [else (scan (+ i 1))])))))

;; the synthesized names carry a ^ when the variable is boxed; that is a fact
;; about the compiled code, not about the source, so it goes away here
(define (unbox-name sym)
  (let ([s (symbol->string sym)])
    (if (and (> (string-length s) 1) (char=? (string-ref s 0) #\^))
        (string->symbol (substring s 1 (string-length s)))
        sym)))

(define (varname sym) (demangle (unbox-name sym)))

;; --- small helpers ----------------------------------------------------------

(define (core-ref? x)
  (or (sexp-match? (quote (ref <symbol>)) x) (sexp-match? (quote (const <symbol>)) x)))
(define (ref-of x) (varname (cadr x)))
(define (empty-begin? x) (equal? x '(begin)))
(define (false-quote? x) (equal? x '(quote #f)))

;; is X a reference to the variable V?
(define (ref-to? x v) (and (core-ref? x) (eq? (cadr x) v)))

;; how many times does variable V occur free in X?  counts refs and set!s
(define (count-refs v x)
  (cond [(not (pair? x)) 0]
        [(sexp-match? '(quote *) x) 0]
        [(and (core-ref? x) (eq? (cadr x) v)) 1]
        [(sexp-match? '(set& <symbol>) x) (if (eq? (cadr x) v) 1 0)]
        [(sexp-match? '(set! <symbol> *) x)
         (+ (if (eq? (cadr x) v) 1 0) (count-refs v (caddr x)))]
        [else (let loop ([l x] [n 0])
                (cond [(pair? l) (loop (cdr l) (+ n (count-refs v (car l))))]
                      [else n]))]))

;; the body of a lambda/let is a sequence; splice a begin into it.  An empty
;; begin is kept whole: a body has to have at least one form in it.
(define (body-forms x)
  (if (and (pair? x) (eq? (car x) (quote begin)) (pair? (cdr x)))
      (cdr x)
      (list x)))

;; --- the rewriter -----------------------------------------------------------

;; A Scheme form for X.  A pair is taken to be Core already; anything else is
;; put through da-bytecode first, so a procedure, a global name, a code vector
;; and a bytecode string all work.  #f when nothing can be got out of X.
(define (da-core x)
  (cond [(pair? x) (sx x)]
        [(da-bytecode x) => sx]
        [else #f]))

(define (sx x)
  (cond
    [(sexp-match? '(quote *) x) (quotation (cadr x))]
    [(core-ref? x) (ref-of x)]
    [(sexp-match? (quote (const <symbol>)) x) (varname (cadr x))]
    [(sexp-match? (quote (gref <symbol>)) x) (varname (cadr x))]
    [(sexp-match? '(set& <symbol>) x) (list 'set& (varname (cadr x)))]
    [(sexp-match? '(set! <symbol> *) x) (list 'set! (varname (cadr x)) (sx (caddr x)))]
    [(sexp-match? '(gset! <symbol> *) x) (list 'set! (varname (cadr x)) (sx (caddr x)))]
    [(sexp-match? '(begin * ...) x) (sx-begin (cdr x))]
    [(sexp-match? '(if * * *) x) (sx-if (cadr x) (caddr x) (cadddr x))]
    [(sexp-match? '(lambda * *) x) (sx-lambda (cadr x) (caddr x))]
    [(sexp-match? '(lambda* * ...) x) (sx-case-lambda (cdr x))]
    [(sexp-match? '(letcc <symbol> *) x) (list 'letcc (varname (cadr x)) (sx (caddr x)))]
    [(sexp-match? '(withcc * *) x) (list 'withcc (sx (cadr x)) (sx (caddr x)))]
    [(sexp-match? '(integrable <number> * ...) x) (sx-integrable (cadr x) (cddr x))]
    [(sexp-match? '(asm <string>) x) x]
    [(sexp-match? '(call * * ...) x) (sx-call (cadr x) (cddr x))]
    [(sexp-match? '(call *) x) (sx-call (cadr x) '())]
    [else x]))

;; a literal needs quoting unless it is self-evaluating
(define (quotation v)
  (if (or (boolean? v) (number? v) (char? v) (string? v) (bytevector? v) (vector? v))
      v
      (list 'quote v)))

;; Core's begin with no forms is "no value in particular": the compiler emits
;; nothing for it and whatever is in the accumulator stays there.  Written back
;; as (begin) that is exact, but few readers take it for a value, so the output
;; says (void) instead -- the same value under a name.
;;
;; The two are not the same code: (begin) emits nothing where (void) emits the
;; void integrable, so a form carrying one does not compile back byte for byte.
;; The round-trip checks turn the substitution off for that reason, and it is
;; tested on its own.
(define da-void-for-empty-begin (make-parameter #t (lambda (x) (and x #t))))

(define (no-value-form)
  (if (da-void-for-empty-begin) '(void) '(begin)))

(define (sx-begin forms)
  (cond [(null? forms) (no-value-form)]
        [(null? (cdr forms)) (sx (car forms))]
        [else (cons 'begin (map sx forms))]))

(define (sx-integrable ig args)
  (let ([name (integrable-global ig)])
    (if (symbol? name)
        (cons name (map sx args))
        (cons (list 'integrable ig) (map sx args)))))

;; --- if, and cond / and / or / when / unless -------------------------------

(define (sx-if c t e)
  (cond
    ;; (if c body (begin)) is when; the same with a negated test is unless
    [(and (empty-begin? e) (sexp-match? '(integrable <number> *) c)
          (eq? (integrable-global (cadr c)) 'not))
     (cons 'unless (cons (sx (caddr c)) (map sx (body-forms t))))]
    [(empty-begin? e)
     (cons 'when (cons (sx c) (map sx (body-forms t))))]
    ;; (if a b '#f) is and
    [(false-quote? e) (flatten-and (sx c) (sx t))]
    ;; an if whose else arm is another if is a cond
    [(sexp-match? '(if * * *) e) (sx-cond (list (list c t)) e)]
    [else (list 'if (sx c) (sx t) (sx e))]))

(define (flatten-and c t)
  (if (and (pair? t) (eq? (car t) 'and))
      (cons 'and (cons c (cdr t)))
      (list 'and c t)))

;; Collect a chain of ifs into one cond; CLAUSES is reversed.  A chain that
;; bottoms out in an empty begin stops one step early: cond supplies #f when no
;; clause matches, where the original supplied nothing, so the last step has to
;; stay an if -- which reads back as a when.
(define (sx-cond clauses e)
  (cond
    [(and (sexp-match? (quote (if * * *)) e) (not (empty-begin? (cadddr e))))
     (sx-cond (cons (list (cadr e) (caddr e)) clauses) (cadddr e))]
    [else
     (let ([arms (map (lambda (c) (cons (sx (car c)) (map sx (body-forms (cadr c)))))
                      (reverse clauses))])
       (cond [(false-quote? e) (cons 'cond arms)]
             [(empty-begin? e) (cons 'cond arms)]
             [else (append (cons 'cond arms)
                           (list (cons 'else (map sx (body-forms e)))))]))]))

;; --- lambda -----------------------------------------------------------------

(define (sx-formals ids)
  (cond [(symbol? ids) (varname ids)]
        [(null? ids) '()]
        [(pair? ids) (cons (varname (car ids)) (sx-formals (cdr ids)))]
        [else ids]))

(define (sx-lambda ids body)
  (cons 'lambda (cons (sx-formals ids) (sx-body body))))

(define (sx-case-lambda clauses)
  (cons 'case-lambda
        (map (lambda (cl)
               (let ([lam (cadr cl)])
                 (if (sexp-match? '(lambda * *) lam)
                     (cons (sx-formals (cadr lam)) (sx-body (caddr lam)))
                     (list '_ (sx lam)))))
             clauses)))

;; A body may open with internal definitions, which come from a letrec-shaped
;; call whose initializers are all placeholders.  Each definition is either
;;   (one <var> <expr>)                  an ordinary define
;;   (many <formals> <producer>)         a define-values
(define (sx-body x)
  (let ([d (letrec-shape x)])
    (if (and d (eq? (car d) 'defines))
        (append (map sx-definition (cadr d))
                (map sx (body-forms (caddr d))))
        (map sx (body-forms x)))))

;; A procedure is defined the short way, (define (f . formals) body ...), which
;; the expander turns back into exactly (define f (lambda formals body ...)).
(define (sx-definition d)
  (if (eq? (car d) 'one)
      (let ([e (sx (caddr d))])
        (if (and (pair? e) (eq? (car e) 'lambda) (pair? (cdr e)) (pair? (cddr e)))
            (cons 'define (cons (cons (cadr d) (cadr e)) (cddr e)))
            (list 'define (cadr d) e)))
      (list 'define-values (cadr d) (sx (caddr d)))))

;; --- recognizing the letrec / letrec* shape ---------------------------------
;; (call (lambda (v ...) (begin <definition> ... rest ...)) init ...)
;; with every initializer a placeholder and the variables defined in order.
;; => (list 'defines defs body)  when the placeholders are empty begins
;;    (list 'letrec* defs body)  when they are (quote #f)
;; NB the two differ in the code they compile to, so they must not be swapped.

(define (letrec-shape x)
  (and (sexp-match? '(call (lambda (<symbol> ...) *) * ...) x)
       (let* ([lam (cadr x)] [ids (cadr lam)] [body (caddr lam)] [inits (cddr x)])
         (and (pair? ids)
              (= (length ids) (length inits))
              (sexp-match? '(begin * ...) body)
              ;; The two kinds of placeholder are told apart by the initializers,
              ;; and which one it is has to be carried into the statements: a
              ;; store of (quote #f) is a clear in a letrec* group and an
              ;; ordinary definition in a body, since (define x #f) compiles to
              ;; exactly that.  Testing for either would read half the
              ;; definitions of a large body as clears.
              (let ([clear? (cond [(every? empty-begin? inits) empty-begin?]
                                  [(every? false-quote? inits) false-quote?]
                                  [else #f])])
                (and clear?
                     (let ([kind (if (eq? clear? empty-begin?) 'defines 'letrec*)])
                       (let loop ([vs ids] [forms (cdr body)] [ds '()])
                         (cond
                           [(null? vs)
                            (and (pair? forms) (list kind (reverse ds) (splice-begin forms)))]
                           ;; one statement, one group of definitions
                           [(definition-group vs forms clear?)
                            => (lambda (r)
                                 (loop (car r) (cadr r) (append (caddr r) ds)))]
                           [else #f])))))))))

;; --- one statement's worth of definitions -----------------------------------
;; Consecutive stores compile to consecutive sseti instructions, and sseti
;; leaves the accumulator alone, so a run of them shares one computed value.
;; Read back from bytecode that arrives as a nest, LAST variable outermost:
;;
;;   (set! v3 (set! v2 (set! v1 e)))
;;
;; means v1 gets e and v2, v3 get the same value.  When e is a real expression
;; v1 is an ordinary define and the rest were only being cleared; when e is a
;; placeholder all of them were.  The expander writes the clears as separate
;; statements instead, so a run of those is absorbed too.
;;
;; Cleared variables have to be filled by something, and what fills them is the
;; call-with-values that define-values expands to.
;;
;; => (list remaining-vars remaining-forms defs-in-reverse) or #f

(define (definition-group vs forms clear?)
  (and (pair? forms)
       (let ([nest (store-nest (car forms))])
         (and nest
              (let* ([nvars (car nest)] [val (cadr nest)])
                (and (prefix-of? nvars vs)
                     (let* ([lead? (not (clear? val))]
                            [defs (if lead?
                                      (list (list 'one (varname (car nvars)) val))
                                      '())]
                            [cleared (if lead? (cdr nvars) nvars)])
                       (let absorb ([cleared cleared]
                                    [rest (cdr forms)]
                                    [used (length nvars)])
                         (cond
                           ;; Nothing was cleared: a plain define on its own, and
                           ;; nothing to absorb.  This has to be tested BEFORE
                           ;; looking at the next statement: (quote #f) is both
                           ;; the letrec* placeholder and an ordinary value, so a
                           ;; following (set! x '#f) that is really a definition
                           ;; would otherwise be taken for a clear and the whole
                           ;; group would fail for want of a call-with-values.
                           [(null? cleared)
                            (list (list-tail vs used) rest defs)]
                           ;; the expander clears one variable per statement
                           [(and (pair? rest)
                                 (let ([n2 (store-nest (car rest))])
                                   (and n2
                                        (clear? (cadr n2))
                                        (prefix-of? (car n2) (list-tail vs used))
                                        n2)))
                            => (lambda (n2)
                                 (absorb (append cleared (car n2)) (cdr rest)
                                         (+ used (length (car n2)))))]
                           ;; what was cleared must be filled by call-with-values
                           [(and (pair? rest) (cwv-formals (car rest) cleared))
                            => (lambda (formals)
                                 (list (list-tail vs used) (cdr rest)
                                       (cons (list 'many formals (caddr (caddr (car rest))))
                                             defs)))]
                           [else #f])))))))))

;; a nest of stores => (list variables-in-frame-order innermost-value) or #f
(define (store-nest stmt)
  (let walk ([s stmt] [got '()])
    (and (sexp-match? '(set! <symbol> *) s)
         (let ([got (cons (cadr s) got)] [inner (caddr s)])
           (if (sexp-match? '(set! <symbol> *) inner)
               (walk inner got)
               (list got inner))))))

(define (prefix-of? p l)
  (cond [(null? p) #t]
        [(and (pair? l) (eq? (car p) (car l))) (prefix-of? (cdr p) (cdr l))]
        [else #f]))

;; a call-with-values whose consumer assigns exactly VARS from its own formals,
;; one for one and in order => the define-values formals, shaped like the
;; consumer's, or #f
(define (cwv-formals stmt vars)
  (and (sexp-match? '(call * (lambda () *) (lambda * *)) stmt)
       (let ([op (cadr stmt)] [consumer (cadddr stmt)])
         (and (core-ref? op) (eq? (cadr op) 'call-with-values)
              (let* ([temps (cadr consumer)]
                     [flat (flatten-formals temps)]
                     [asgs (body-forms (caddr consumer))])
                (and (= (length flat) (length vars))
                     (= (length asgs) (length vars))
                     (let check ([a asgs] [t flat] [v vars])
                       (cond [(null? a) #t]
                             [(and (sexp-match? '(set! <symbol> (ref <symbol>)) (car a))
                                   (eq? (cadr (car a)) (car v))
                                   (eq? (cadr (caddr (car a))) (car t)))
                              (check (cdr a) (cdr t) (cdr v))]
                             [else #f]))
                     (reshape-formals temps vars)))))))

(define (flatten-formals f)
  (cond [(symbol? f) (list f)]
        [(null? f) '()]
        [(pair? f) (cons (car f) (flatten-formals (cdr f)))]
        [else '()]))

;; the shape of TEMPLATE with NAMES substituted position by position
(define (reshape-formals template names)
  (cond [(symbol? template) (varname (car names))]
        [(null? template) '()]
        [(pair? template)
         (cons (varname (car names)) (reshape-formals (cdr template) (cdr names)))]
        [else '()]))

(define (every? p l)
  (cond [(null? l) #t] [(p (car l)) (every? p (cdr l))] [else #f]))

(define (splice-begin forms)
  (if (null? (cdr forms)) (car forms) (cons 'begin forms)))
;; --- let-values and let*-values ---------------------------------------------
;; Both expand to a nest of call-with-values, one level per clause.  The
;; difference is what the consumer's formals are: let*-values uses the
;; programmer's own variables, so each clause's initializer is evaluated with
;; the previous clauses in scope; let-values binds fresh temporaries and renames
;; them all at once in a let around the body, so that no initializer can see
;; another clause's variables.  That renaming let is what tells the two apart.
;;
;; A clause binding exactly one variable never reaches call-with-values at all --
;; let*-values rewrites [(a) x] to an ordinary let -- so a consumer with one
;; formal is a call-with-values that was written as one, and is left alone.
;; So is one whose producer is not a literal thunk, since both forms always
;; build the thunk themselves.

;; => (list formals producer consumer-body) or #f
(define (cwv-parts x)
  (and (sexp-match? '(call * (lambda () *) (lambda * *)) x)
       (let ([op (cadr x)] [consumer (cadddr x)])
         (and (core-ref? op) (eq? (cadr op) 'call-with-values)
              (let ([formals (cadr consumer)])
                (and (not (and (pair? formals) (null? (cdr formals))))
                     (list formals (caddr (caddr x)) (caddr consumer))))))))

;; the longest nest of them, outermost first: a consumer that does nothing but
;; the next call-with-values is a clause and not a body
;; => (list ((formals producer) ...) innermost-body) or #f
(define (cwv-nest x)
  (let loop ([x x] [cls '()])
    (let ([p (cwv-parts x)])
      (and p
           (let* ([cls (cons (list (car p) (cadr p)) cls)]
                  [forms (body-forms (caddr p))])
             (if (and (null? (cdr forms)) (cwv-parts (car forms)))
                 (loop (car forms) cls)
                 (list (reverse cls) (caddr p))))))))

(define (take-n l n) (if (= n 0) '() (cons (car l) (take-n (cdr l) (- n 1)))))

;; the clauses of a let*-values: the formals as they stand
(define (cwv-clauses cls)
  (map (lambda (c) (list (sx-formals (car c)) (sx (cadr c)))) cls))

;; the clauses of a let-values: the formals reshaped over the names the renaming
;; let gave them, taken in order, one clause's worth at a time
(define (renamed-clauses cls vars)
  (let loop ([cls cls] [vs vars] [r '()])
    (if (null? cls)
        (reverse r)
        (let* ([tpl (car (car cls))] [k (length (flatten-formals tpl))])
          (loop (cdr cls) (list-tail vs k)
                (cons (list (reshape-formals tpl (take-n vs k)) (sx (cadr (car cls))))
                      r))))))

(define (let-values-shape x)
  (let ([n (cwv-nest x)])
    (and n
         (let* ([cls (car n)] [body (cadr n)]
                [temps (let g ([l cls])
                         (if (null? l) '()
                             (append (flatten-formals (car (car l))) (g (cdr l)))))])
           (and (sexp-match? '(call (lambda (<symbol> ...) *) * ...) body)
                (let* ([lam (cadr body)] [vars (cadr lam)] [inits (cddr body)])
                  (and (= (length vars) (length temps))
                       (= (length inits) (length temps))
                       (let ok ([i inits] [t temps])
                         (or (null? i)
                             (and (ref-to? (car i) (car t)) (ok (cdr i) (cdr t)))))
                       ;; the renaming is the only use each temporary has
                       (every? (lambda (t) (= 1 (count-refs t x))) temps)
                       (cons 'let-values
                             (cons (renamed-clauses cls vars)
                                   (sx-body (caddr lam))))))))))) 

(define (let*-values-shape x)
  (let ([n (cwv-nest x)])
    (and n (cons 'let*-values
                 (cons (cwv-clauses (car n)) (sx-body (cadr n)))))))

;; --- call, and the forms that hide inside one -------------------------------

(define (sx-call f args)
  (cond
    ;; a named let, and the do that compiles to one
    [(named-let-shape f args) => (lambda (r) r)]
    ;; (call (lambda (v) (if (ref v) (ref v) e)) c) is or
    [(or-shape f args) => (lambda (r) r)]
    ;; (call (lambda (v) (if (ref v) (f (ref v)) e)) c) is a cond with =>
    [(arrow-cond-shape f args) => (lambda (r) r)]
    ;; (call (lambda (key) <eqv?/memv chain on key>) e) is case
    [(case-shape f args) => (lambda (r) r)]
    ;; letrec and letrec* in expression position.  A define-values group has no
    ;; letrec binding to be written as, so a body carrying one stays a plain let
    ;; with the assignments left in it.
    [(let ([d (letrec-shape (cons (quote call) (cons f args)))])
       (and d (every? (lambda (b) (eq? (car b) (quote one))) (cadr d)) d))
     => (lambda (d)
          (let ([bs (map (lambda (b) (list (cadr b) (sx (caddr b)))) (cadr d))]
                [body (map sx (body-forms (caddr d)))])
            (cons (if (eq? (car d) (quote defines)) (quote letrec) (quote letrec*))
                  (cons bs body))))]
    ;; a plain let
    [(and (sexp-match? '(lambda (<symbol> ...) *) f)
          (= (length (cadr f)) (length args)))
     (sx-let (cadr f) args (caddr f))]
    ;; let-values and let*-values, which are nests of call-with-values
    [(let-values-shape (cons (quote call) (cons f args))) => (lambda (r) r)]
    [(let*-values-shape (cons (quote call) (cons f args))) => (lambda (r) r)]
    [else (cons (sx f) (map sx args))]))

;; (call (lambda (v ...) body) e ...) => (let ((v e) ...) body), or let* when
;; the lets are nested one variable at a time
(define (sx-let ids args body)
  (let ([bs (map (lambda (v e) (list (varname v) (sx e))) ids args)])
    (if (and (= (length ids) 1) (nested-let? body))
        (let ([inner (sx body)])
          (cons 'let* (cons (append bs (cadr inner)) (cddr inner))))
        (cons 'let (cons bs (sx-body body))))))

;; is BODY itself a one-variable let that we would print as let?
(define (nested-let? body)
  (and (sexp-match? '(call (lambda (<symbol>) *) *) body)
       (not (letrec-shape body))
       (not (or-shape (cadr body) (cddr body)))
       (not (arrow-cond-shape (cadr body) (cddr body)))
       (not (case-shape (cadr body) (cddr body)))
       (let ([r (sx body)]) (and (pair? r) (memq (car r) '(let let*))))))

;; --- named let and do -------------------------------------------------------
;; (call (call (lambda (f) (begin (set! f (lambda (v ...) body)) (ref f)))
;;             (begin))
;;       e ...)

(define (named-let-shape f args)
  (and (sexp-match? '(call (lambda (<symbol>) (begin (set! <symbol> (lambda * *)) (ref <symbol>))) (begin)) f)
       (let* ([lam (cadr f)] [v (car (cadr lam))] [body (caddr lam)]
              [setf (cadr body)] [lp (caddr setf)] [ids (cadr lp)])
         (and (eq? (cadr setf) v)
              (eq? (cadr (caddr body)) v)
              (list? ids)
              (= (length ids) (length args))
              (or (do-shape v ids (caddr lp) args)
                  (cons 'let
                        (cons (varname v)
                              (cons (map (lambda (id e) (list (varname id) (sx e))) ids args)
                                    (sx-body (caddr lp))))))))))

;; a do loop is a named let whose body is a single if that either finishes or
;; calls itself in tail position with one step expression per variable
(define (do-shape v ids body args)
  (and (sexp-match? '(if * * *) body)
       (let* ([test (cadr body)] [then (caddr body)] [else- (cadddr body)])
         (let-values ([(fin cmds+call) (values then else-)])
           (and (self-call-tail? v ids cmds+call)
                (let* ([forms (body-forms cmds+call)]
                       [cmds (reverse (cdr (reverse forms)))]
                       [call (car (reverse forms))]
                       [steps (cddr call)])
                  (cons 'do
                        (cons (map (lambda (id e s)
                                     (if (ref-to? s id)
                                         (list (varname id) (sx e))
                                         (list (varname id) (sx e) (sx s))))
                                   ids args steps)
                              (cons (cons (sx test)
                                          (if (empty-begin? fin) '() (map sx (body-forms fin))))
                                    (map sx cmds))))))))))

;; does X end with a tail call of V with one argument per variable?
(define (self-call-tail? v ids x)
  (let* ([forms (body-forms x)] [last (car (reverse forms))])
    (and (sexp-match? '(call (ref <symbol>) * ...) last)
         (eq? (cadr (cadr last)) v)
         (= (length (cddr last)) (length ids))
         ;; the loop variable must not be used for anything else
         (every? (lambda (f) (= 0 (count-refs v f)))
                 (cons (reverse (cdr (reverse forms))) (cddr last))))))

;; --- or ---------------------------------------------------------------------
;; (call (lambda (v) (if (ref v) (ref v) e)) c), v used nowhere else

(define (or-shape f args)
  (and (sexp-match? '(lambda (<symbol>) (if (ref <symbol>) (ref <symbol>) *)) f)
       (= (length args) 1)
       (let* ([v (car (cadr f))] [body (caddr f)] [e (cadddr body)])
         (and (eq? (cadr (cadr body)) v)
              (eq? (cadr (caddr body)) v)
              (= 0 (count-refs v e))
              (flatten-or (sx (car args)) (sx e))))))

(define (flatten-or c e)
  (if (and (pair? e) (eq? (car e) 'or))
      (cons 'or (cons c (cdr e)))
      (list 'or c e)))

;; --- cond with => -----------------------------------------------------------
;; (call (lambda (v) (if (ref v) (f (ref v)) e)) c), v used only there

(define (arrow-cond-shape f args)
  (and (sexp-match? '(lambda (<symbol>) (if (ref <symbol>) (call * (ref <symbol>)) *)) f)
       (= (length args) 1)
       (let* ([v (car (cadr f))] [body (caddr f)]
              [app (caddr body)] [recv (cadr app)] [e (cadddr body)])
         (and (eq? (cadr (cadr body)) v)
              (eq? (cadr (caddr app)) v)
              (= 0 (count-refs v recv))
              (= 0 (count-refs v e))
              (let ([rest (sx e)])
                (list 'cond
                      (list (sx (car args)) '=> (sx recv))
                      (cons 'else (list rest))))))))

;; --- case -------------------------------------------------------------------
;; (call (lambda (key) <chain of eqv?/memv tests on key>) e)

(define (case-shape f args)
  (and (sexp-match? '(lambda (<symbol>) *) f)
       (= (length args) 1)
       (let* ([v (car (cadr f))] [body (caddr f)])
         (let loop ([x body] [arms '()])
           (cond
             [(case-test x v)
              => (lambda (data)
                   (loop (cadddr x)
                         (cons (cons data (map sx (body-forms (caddr x)))) arms)))]
             [(null? arms) #f]
             ;; the chain has to have consumed every mention of the key
             [(> (count-refs v x) 0) #f]
             [else
              (cons 'case
                    (cons (sx (car args))
                          (append (reverse arms)
                                  (list (cons 'else (map sx (body-forms x)))))))])))))

;; an (if (eqv? key 'd) ...) or (if (memv key '(d ...)) ...) test on V
(define (case-test x v)
  (and (sexp-match? '(if (integrable <number> (ref <symbol>) (quote *)) * *) x)
       (let* ([t (cadr x)] [ig (integrable-global (cadr t))] [datum (cadr (cadddr t))])
         (and (eq? (cadr (caddr t)) v)
              (case ig
                [(eqv?) (list datum)]
                [(memv) (and (list? datum) datum)]
                [else #f])))))


;; --- identifying a procedure by name ----------------------------------------
;; Some procedures have no Core preimage at all: the integrable wrappers that
;; rds_intgtab synthesizes, the optional-argument dispatchers, and hand-written
;; ones like values and call/cc.  There is still an exact answer to "what is
;; this" -- the store files it under a name, and eq? finds it.  A lookup, not a
;; guess.
;;
;; A value can sit under more than one name: (define promise? box?) gives that
;; procedure two.  Prefer the name that is an integrable's own, then one with no
;; environment prefix, then the shortest, then alphabetical order -- so the
;; answer does not depend on which bucket the symbol happened to hash into.

(define (prefixed-name? sym)
  (let* ([s (symbol->string sym)] [n (string-length s)])
    (let scan ([i 0])
      (cond [(> (+ i 3) n) #f]
            [(and (char=? (string-ref s i) #\:)
                  (char=? (string-ref s (+ i 1)) #\/)
                  (char=? (string-ref s (+ i 2)) #\/)) #t]
            [else (scan (+ i 1))]))))

;; every store name whose location holds VAL
(define (store-names-of val)
  (let* ([gs (global-store)] [n (vector-length gs)])
    (let loop ([i 0] [r (quote ())])
      (if (>= i n)
          r
          (loop (+ i 1)
                (let scan ([l (vector-ref gs i)] [r r])
                  (cond [(null? l) r]
                        [(eq? (unbox (cdar l)) val) (scan (cdr l) (cons (caar l) r))]
                        [else (scan (cdr l) r)])))))))

(define (better-name? a b)
  (let ([ia (and (lookup-integrable a) #t)] [ib (and (lookup-integrable b) #t)])
    (cond [(not (eq? ia ib)) ia]
          [(not (eq? (prefixed-name? a) (prefixed-name? b))) (not (prefixed-name? a))]
          [else (let ([sa (symbol->string a)] [sb (symbol->string b)])
                  (cond [(< (string-length sa) (string-length sb)) #t]
                        [(> (string-length sa) (string-length sb)) #f]
                        [else (string<? sa sb)]))])))

;; the name the store files P under, or #f if nothing holds it
(define (da-name p)
  (let loop ([l (store-names-of p)] [best #f])
    (cond [(null? l) (and best (varname best))]
          [(or (not best) (better-name? (car l) best)) (loop (cdr l) (car l))]
          [else (loop (cdr l) best)])))

;; --- whole procedures -------------------------------------------------------
;; A closure carries its free variables in a display.  Naming those :a :b ... in
;; display order lets the values be put back: the lambda goes inside a let that
;; binds each name to what the display actually holds.  A slot the code
;; dereferences holds a box, because something assigns that variable, so the
;; binding takes the box's contents and re-expansion boxes it again.

(define (display-var i)
  (string->symbol (string-append ":" (letters i))))

;; --- case-lambda ------------------------------------------------------------
;; A case-lambda does not compile to one procedure with several arities.  It
;; compiles to a dispatcher whose display holds one ordinary closure per clause,
;; and whose code is a run of "if the argument count is this, tail call display
;; slot n" -- jdceq for an exact count, jdcge for a clause with a rest argument,
;; jdref for one that takes anything -- ending in aerr for a call that matches no
;; clause at all.  The auto-generated optional-argument dispatchers have the same
;; shape.
;;
;; So the clauses have to be read one at a time and put back together.  Each
;; carries its own display, and where two of them close over the same variable
;; they hold the same cell, so the cells are named once across all the clauses
;; and bound in one let outside.  That is also the only way an assigned free
;; variable comes back as one variable rather than as one per clause.

;; the display slots the dispatcher hands control to, in order, or #f if CV is
;; not a dispatcher
(define (dispatcher-slots cv)
  (let ([n (vector-length cv)])
    (let loop ([i 0] [r (quote ())])
      (and (< i n)
           (lookup (vector-ref cv i))
           (let* ([d (decode-at cv i)] [base (car d)] [ops (cadr d)] [next (caddr d)])
             (cond
               ;; the error at the end is what makes it a dispatcher and not
               ;; some other run of jumps
               [(string=? base "%%") (and (pair? r) (= next n) (reverse r))]
               ;; jdceq and jdcge take (count slot); jdref normalizes to jdcge
               ;; with the count baked in, so it arrives the same way
               [(or (string=? base "|") (string=? base "|!"))
                (and (= (length ops) 2) (loop next (cons (cadr ops) r)))]
               [else #f]))))))

;; the cells of a closure vector, in display order
(define (closure-cells v)
  (let loop ([i 1] [r (quote ())])
    (if (>= i (vector-length v)) (reverse r) (loop (+ i 1) (cons (vector-ref v i) r)))))

;; Position of CELL in CELLS, by identity.  Two clauses that close over the same
;; variable hold the same cell, which is what makes one name serve both.  Two
;; different variables holding one eq? value are merged as well; for an assigned
;; variable that cannot happen, since the cell is then a box, and for an
;; unassigned one the two bindings are indistinguishable anyway.
(define (cell-index cell cells)
  (let loop ([l cells] [i 0])
    (cond [(null? l) #f] [(eq? (car l) cell) i] [else (loop (cdr l) (+ i 1))])))

(define (add-cell cell cells)
  (if (cell-index cell cells) cells (append cells (list cell))))

(define (%da-case-lambda v slots)
  (let ([clauses (let loop ([l slots] [r (quote ())])
                   (cond [(null? l) (reverse r)]
                         [(and (integer? (car l)) (exact? (car l)) (>= (car l) 0)
                               (< (+ (car l) 1) (vector-length v)))
                          (loop (cdr l) (cons (vector-ref v (+ (car l) 1)) r))]
                         [else #f]))])
    (and clauses
         (let ([vecs (let loop ([l clauses] [r (quote ())])
                       (cond [(null? l) (reverse r)]
                             [else
                              (let ([cv (guard (e (#t #f)) (closure->vector (car l)))])
                                (and (vector? cv) (> (vector-length cv) 0)
                                     (vector? (vector-ref cv 0))
                                     (loop (cdr l) (cons cv r))))]))])
           (and vecs (%assemble-case-lambda vecs))))))

(define (%assemble-case-lambda vecs)
  ;; one pass to collect the cells every clause between them closes over
  (let ([cells (let loop ([l vecs] [cells (quote ())])
                 (if (null? l)
                     cells
                     (loop (cdr l)
                           (let g ([cs (closure-cells (car l))] [cells cells])
                             (if (null? cs) cells (g (cdr cs) (add-cell (car cs) cells)))))))])
    (let ([names (let loop ([i 0] [r (quote ())])
                   (if (>= i (length cells)) (reverse r) (loop (+ i 1) (cons (display-var i) r))))])
      ;; and one pass to read each clause with those names in place of its display
      (let loop ([l vecs] [arms (quote ())] [boxed (quote ())])
        (cond
          [(null? l)
           (let ([body (cons (quote case-lambda) (reverse arms))])
             (if (null? cells)
                 body
                 (cons (quote let)
                       (cons (let g ([cs cells] [ns names] [bs (quote ())])
                               (if (null? cs)
                                   (reverse bs)
                                   (g (cdr cs) (cdr ns)
                                      (cons (list (car ns)
                                                  (quotation
                                                    (if (and (memq (car cs) boxed) (box? (car cs)))
                                                        (unbox (car cs))
                                                        (car cs))))
                                            bs))))
                             (list body)))))]
          [else
           (let* ([cs (closure-cells (car l))]
                  [ns (let g ([cs cs] [r (quote ())])
                        (if (null? cs)
                            (reverse r)
                            (g (cdr cs) (cons (list-ref names (cell-index (car cs) cells)) r))))]
                  [core (da-closure (vector-ref (car l) 0) ns)])
             (and core
                  (let ([form (sx core)]
                        ;; which of this clause's own slots hold a box
                        [mine (let g ([ix (dsp-boxed %top-dsp)] [r (quote ())])
                                (if (null? ix)
                                    r
                                    (g (cdr ix)
                                       (if (< (car ix) (length cs))
                                           (cons (list-ref cs (car ix)) r)
                                           r))))])
                    (and (pair? form) (eq? (car form) (quote lambda))
                         (loop (cdr l)
                               (cons (cons (cadr form) (cddr form)) arms)
                               (append mine boxed))))))])))))

;; Code with no Core preimage still has an answer: the name the store files the
;; procedure under.  A bare symbol cannot be confused with a disassembly, which
;; is always a lambda, let or case-lambda form.
(define (%da-procedure p)
  ;; Not every procedure is a closure -- a continuation is not -- and one that is
  ;; not has no code vector to read.  Its name is then all there is to say.
  (let ([v (guard (e (#t #f)) (closure->vector p))])
    (cond
      [(not (and (vector? v) (> (vector-length v) 0) (vector? (vector-ref v 0))))
       (da-name p)]
      [(dispatcher-slots (vector-ref v 0))
       => (lambda (slots) (or (%da-case-lambda v slots) (da-name p)))]
      [else (%da-closure-vector p v)])))

(define (%da-closure-vector p v)
  (let* ([cv (vector-ref v 0)]
         [n (- (vector-length v) 1)]
         [names (let loop ([i 0] [r (quote ())])
                  (if (>= i n) (reverse r) (loop (+ i 1) (cons (display-var i) r))))]
         [core (da-closure cv names)])
    (if (not core)
        (da-name p)
         (let ([form (da-core core)]
               [boxed (dsp-boxed %top-dsp)])
           (if (= n 0)
               form
               (cons (quote let)
                     (cons (let loop ([i 0] [bs (quote ())])
                             (if (>= i n)
                                 (reverse bs)
                                 (let* ([cell (vector-ref v (+ i 1))]
                                        [val (if (and (memv i boxed) (box? cell))
                                                 (unbox cell)
                                                 cell)])
                                   (loop (+ i 1)
                                         (cons (list (display-var i) (quotation val)) bs)))))
                           (list form))))))))

;; --- disassembling by name --------------------------------------------------

;; The whole chain for a procedure, or for the global name of one: #f if there is
;; no procedure to be had, the name the store files it under when its code has no
;; Core preimage, and otherwise the Scheme form.
(define (da-procedure x)
  (let ([p (as-procedure x)])
    (and p (%da-procedure p))))

;; The disassembly of whatever procedure the store holds under a global name, or
;; #f if it holds nothing, holds something that is not a procedure, or has no
;; location for that name at all.  A global name is what appears in a gref, so
;; it is `car' for a built-in and `lib://skint/print?pp*' for a library's, not
;; the bare identifier da-prune-globals shows; an integrable index stands for one
;; too.  Nothing is allocated: asking about a name the store has never seen
;; leaves it unseen.
(define (da-global x)
  (let ([n (global-name-of x)])
    (and n (da-procedure n))))


;; --- da: the whole chain, from whatever there is ----------------------------

;; The display names the decoding invented, in slot order.
(define (dsp-names d) (and d (car d)))

;; The disassembly of a code fragment -- a code vector or a bytecode string that
;; is not a whole closure.  The free variables of such a fragment have names but
;; no values: the display they would have come from is not there.  Rather than
;; hand back a form with dangling references, bind each of them to ? in a let
;; around the form, which says both what is missing and where it would go.
;;
;; Such a let does not compile back to the fragment, and is not meant to: ? is
;; not a value.
(define (da-fragment cv)
  (let ([core (%core-of-code-vector cv)])
    (and core
         (let* ([form (sx core)]
                [names (dsp-names %top-dsp)])
           (if (or (not names) (null? names))
               form
               (cons 'let
                     (cons (map (lambda (n) (list (varname n) '?)) names)
                           (list form))))))))

;; Whatever X is, the most that can be said about it:
;;
;;   a procedure, or the global name of one  the whole chain, display and all,
;;                                          or the name the store files it under
;;                                          when its code has no Core preimage
;;   a code vector                          its disassembly, with ? for a display
;;   a bytecode string                      the same, decoded from the string
;;   a Core expression                      the Scheme form it came from
;;
;; #f when X is none of those, or is one but yields nothing -- a name the store
;; does not hold, a procedure with no code vector, bytecode the compiler never
;; emitted.  Only a malformed value of the right type is an error.
(define (da x)
  (cond [(as-procedure x) => %da-procedure]
        [(vector? x) (da-fragment x)]
        [(string? x) (da-fragment (deserialize-code x))]
        [(pair? x) (sx x)]
        [else #f]))

;; --- built once, when the library is loaded ---------------------------------
;; Everything above reads the instruction table rather than a hand-written list,
;; so the reverse maps have to exist before any entry point runs.  The order is
;; not free: %enc->ig has to be there before %norm, which consults it to tell a
;; baked-in operand from a digit that is part of an integrable's own encoding.

(build-table!)
(build-enc-map!)
(build-ig-map!)
(build-norm!)

))
