(import (scheme base) (scheme write) (scheme eval) (scheme repl))
(import (skint disasm))
(import (skint print))

;; The disassembler is checked against the compiler: what it produces is right
;; when compiling it again gives the same bytecode.  That needs the expander and
;; the compiler, and a code vector to start from.
;; Re-expanding a disassembly needs every name it mentions to be in scope, and
;; the output names integrables canonically -- some of which, like %port?, live
;; only in (skint hidden).  So the whole library comes in, not a few names.
(import (skint hidden))

(include "test.scm")

;; ---------------------------------------------------------------------------
;; Fixtures and helpers
;; ---------------------------------------------------------------------------

(define (procedure-code p) (vector-ref (closure->vector p) 0))

;; every value the global store holds that satisfies KEEP?, as (name . value)
(define (global-values keep?)
  (let* ([gs (global-store)] [n (vector-length gs)])
    (let loop ([i 0] [r '()])
      (if (>= i n)
          r
          (let scan ([l (vector-ref gs i)] [r r])
            (if (null? l)
                (loop (+ i 1) r)
                (let ([v (unbox (cdar l))])
                  (scan (cdr l) (if (keep? v) (cons (cons (caar l) v) r) r)))))))))

;; Every closure the store holds.  Not every procedure: procedure? may also
;; answer #t for any pointer outside the heap, depending on the build, and the
;; store holds two instruction words -- (skint disasm)'s own halt-word and br-word.
(define *procedures* (global-values closure?))

;; the instruction words, as instruction-table lists them
(define *instruction-words*
  (let ([t (instruction-table)])
    (let loop ([i 0] [r '()])
      (if (>= i (vector-length t)) r (loop (+ i 3) (cons (vector-ref t i) r))))))

;; A code vector is framed one of two ways: one decoded from a whole instruction
;; stream ends with halt, one decoded from a {...} block does not.  da-code emits
;; the same characters either way, so accept whichever framing reproduces it.
(define (decode-block s)
  (vector-ref (deserialize-code (string-append "&0{" s "}")) 2))

(define (code-round-trips? p)
  (let* ([cv (procedure-code p)] [back (da-code cv)])
    (or (equal? (deserialize-code back) cv)
        (equal? (decode-block back) cv))))

;; The whole chain for one procedure: code vector -> bytecode -> Core -> Scheme
;; -> bytecode.  The last step goes through the expander and the compiler, so
;; equality of the two strings is equality up to renaming, which is all a
;; disassembler can be asked for.  (begin) rather than (void) is wanted here --
;; see the section on that below.
(define (chain-round-trips? p)
  (let* ([want (string-append "&0{" (da-code p) "}")]
         [core (da-bytecode p)]
         [form (and core (parameterize ([da-void-for-empty-begin #f]) (da-core core)))]
         [got (and form (guard (e (#t #f)) (compile-to-string (expand form))))])
    (and got (equal? got want))))

;; the same for a source expression rather than a live procedure
(define (source-round-trips? src)
  (let* ([core (expand src)]
         [want (compile-to-string core)]
         [form (parameterize ([da-void-for-empty-begin #f]) (da-core core))]
         [got (and form (guard (e (#t #f)) (compile-to-string (expand form))))])
    (and got (equal? got want))))

;; how many of a list satisfy PRED
(define (count-if pred l)
  (let loop ([l l] [n 0])
    (cond [(null? l) n] [(pred (car l)) (loop (cdr l) (+ n 1))] [else (loop (cdr l) n)])))

(define (filter-if pred l)
  (let loop ([l l] [r '()])
    (cond [(null? l) (reverse r)]
          [(pred (car l)) (loop (cdr l) (cons (car l) r))]
          [else (loop (cdr l) r)])))

;; A procedure that carries a display cannot be put through the whole chain: its
;; body refers to slots of a closure that is not being rebuilt.  A display slot
;; is written :n, and a colon also turns up inside a serialized argument, so this
;; test is deliberately crude -- it rejects more than it has to, and every
;; procedure it lets through is one that must round-trip.  save/disasm-sweep.scm
;; draws the line exactly and covers several times as many.
(define (display-free? p)
  (let ([s (guard (e (#t #f)) (da-code p))])
    (and s (not (memv #\: (string->list s))))))


(display "\n--- closure? ---\n")

(test #t (closure? (lambda (x) x)))
(test #t (closure? car))
(test #f (closure? (vector-ref (deserialize-code "") 0)))
(test #f (closure? (vector 1 2)))
(test #f (closure? 'car))
(test #f (closure? (cons 1 2)))
(test #f (closure? 42))

;; procedure? and closure? ask different questions -- any heap pointer in cell 0
;; against a code vector there -- and agree on everything the VM builds, in every
;; build.  So nothing in the store answers one and not the other.
(test '() (map car (global-values (lambda (v) (if (procedure? v) (not (closure? v)) (closure? v))))))

;; no instruction word is a procedure, in any build, and one does not print as one
(test '() (filter-if procedure? *instruction-words*))
(test '() (filter-if closure? *instruction-words*))
(test #t (let ((p (open-output-string)))
           (display (vector-ref (deserialize-code "") 0) p)
           (let ((s (get-output-string p)))
             (and (>= (string-length s) 14)
                  (string=? (substring s 0 14) "#<instruction ")))))

;; and the disassembler declines them without an error
(test #f (da-code (vector-ref (deserialize-code "") 0)))


(display "\n--- da-code: every closure in the store re-encodes ---\n")

(test-assert (> (length *procedures*) 500))
(test (length *procedures*) (count-if (lambda (n+p) (code-round-trips? (cdr n+p))) *procedures*))

;; and it is a string for every one of them, whether or not it has a Core preimage
(test (length *procedures*)
      (count-if (lambda (n+p) (string? (da-code (cdr n+p)))) *procedures*))


(display "\n--- the derived forms da-core has to put back ---\n")

(define derived-forms
  '((lambda (x) x)
    (lambda (x y) (cons x y))
    (lambda args args)
    (lambda (a . b) (cons a b))
    (let ([x 1] [y 2]) (cons x y))
    (let* ([x 1] [y x]) (cons x y))
    (let* ([x 1] [y x] [z y]) (list x y z))
    (letrec ([f (lambda (n) (f n))]) (f 1))
    (letrec* ([a 1] [b (+ a 1)]) (cons a b))
    (lambda (x) (define a 1) (define b 2) (cons a b))
    (let loop ([i 0]) (if (< i 3) (loop (+ i 1)) i))
    (do ([i 0 (+ i 1)]) ((= i 3) i))
    (do ([i 0 (+ i 1)] [acc '() (cons i acc)]) ((= i 3) acc) (display i))
    (cond [(a) 1] [(b) 2] [else 3])
    (cond [(a) 1] [(b) 2])
    (cond [(a) => f] [else 3])
    (and a b c)
    (or a b c)
    (when a b c)
    (unless a b c)
    (case x [(1) 'a] [else 'b])
    (case x [(1 2) 'a] [(3) 'b] [else 'c])
    (if a b c)
    (begin a b c)
    (lambda (x) (set! x 1) x)
    (letcc k (k 5))
    (case-lambda [(x) x] [(x y) (cons x y)])
    (lambda (f) (lambda (y) (f (f y))))
    (lambda (x) (+ x 1 2))
    (lambda (a b c) (< a b c))
    (vector 1 2 3)
    (values 1 2)
    (lambda (x) (if (a) (b) (if (c) (d))))
    ;; multiple values
    (lambda (x y) (define-values (d r) (floor/ x y)) (values (+ d r) (- d r)))
    (lambda (x) (define-values (a) (values x)) a)
    (lambda (x) (define-values vals (values x x)) vals)
    (lambda (x) (define-values (a . rest) (values x x)) (cons a rest))
    (lambda (x) (define-values (a b) (values 1 2)) (define c 3) (list a b c))
    (lambda (x) (define c 3) (define-values (a b) (values 1 2)) (list a b c))
    (lambda (x) (define (f a b) (+ a b)) (define (g a . r) r) (define (h . all) all) (list (f x x) (g x) (h)))
    ;; the four- and five-argument integrables
    (lambda (a b c d) (fxfmar a b c (+ d 1)))
    (lambda (x) (inexact->string x 10 #f 3))
    (lambda (p) (%port-location p (box #f) (box #f) #f #f))
    (lambda (x y) (let-values ([(a b) (floor/ x y)]) (cons a b)))
    (lambda (x y) (let*-values ([(a b) (floor/ x y)]) (cons a b)))
    (lambda (x y) (let-values ([(a b) (floor/ x y)] [(c d) (floor/ y x)]) (list a b c d)))
    (lambda (x y) (let*-values ([(a b) (floor/ x y)] [(c d) (floor/ a b)]) (list a b c d)))
    (lambda (x) (let-values ([vs (values x x)]) vs))
    (lambda (x) (let-values ([(a . r) (values x x)]) (cons a r)))
    (lambda (x) (let-values ([(a) (values x)]) a))
    (lambda (x) (let-values ([(a b) (values 1 2)]) (define c 3) (list a b c)))
    (lambda (p k) (call-with-values p k))
    (lambda (x k) (call-with-values (lambda () (values x x)) k))
    (lambda (p) (call-with-values p (lambda (a b) (cons a b))))))

(test (length derived-forms) (count-if source-round-trips? derived-forms))

;; those three are compiled inline, not as calls of their globals
(test 'integrable (car (expand '(fxfmar a b c d))))
(test 'integrable (car (expand '(inexact->string x 10 #f 3))))
(test 'integrable (car (expand '(%port-location p a b c d))))

;; and taking them back apart from bytecode needs their operand counts: read as
;; two operands, the rest of the stack turns into a let that was never there
(define (bytecode-round-trips? src)
  (let* ([want (compile-to-string (expand src))]
         [form (parameterize ([da-void-for-empty-begin #f]) (da-core (da-bytecode want)))])
    (equal? want (compile-to-string (expand form)))))
(test-assert (bytecode-round-trips? '(lambda (a b c d) (fxfmar a b c (+ d 1)))))
(test-assert (bytecode-round-trips? '(lambda (x) (inexact->string x 10 #f 3))))
(test-assert (bytecode-round-trips? '(lambda (p) (%port-location p (box #f) (box #f) #f #f))))

;; and what a few of them look like, since the point is that they read as source
(define (form-of src) (parameterize ([da-prune-globals #t]) (da-core (expand src))))

(test-assert (memq 'let (list (car (form-of '(let ([x 1]) x))))))
(test-assert (memq 'do (list (car (form-of '(do ([i 0 (+ i 1)]) ((= i 3) i)))))))
(test-assert (memq 'cond (list (car (form-of '(cond [(a) 1] [(b) 2] [else 3]))))))
(test-assert (memq 'case (list (car (form-of '(case x [(1) 'a] [else 'b]))))))
(test '(and a b c) (form-of '(and a b c)))
(test '(or a b c) (form-of '(or a b c)))


(display "\n--- the whole chain, over the procedures that carry no display ---\n")

(define *display-free* (filter-if (lambda (n+p) (display-free? (cdr n+p))) *procedures*))

(test-assert (> (length *display-free*) 20))
(test (length *display-free*)
      (count-if (lambda (n+p)
                  (or (not (da-bytecode (cdr n+p)))       ; no Core preimage: nothing to check
                      (chain-round-trips? (cdr n+p))))
                *display-free*))


(display "\n--- every entry point takes what the ones above it take ---\n")

;; list-copy has a Core preimage; car is an integrable wrapper and has none.
(define lc-code (procedure-code list-copy))
(define lc-bytes (da-code lc-code))

(test-assert (string? lc-bytes))
(test lc-bytes (da-code list-copy))
(test lc-bytes (da-code 'list-copy))

(test (da-bytecode lc-bytes) (da-bytecode lc-code))
(test (da-bytecode lc-bytes) (da-bytecode list-copy))
(test (da-bytecode lc-bytes) (da-bytecode 'list-copy))

(test (da-core lc-bytes) (da-core lc-code))
(test (da-core lc-bytes) (da-core list-copy))
(test (da-core lc-bytes) (da-core 'list-copy))

(test (da-procedure list-copy) (da-procedure 'list-copy))
(test (da-procedure list-copy) (da-global 'list-copy))

;; a Core expression is taken as one
(test '(f 1) (da-core '(call (ref f) (quote 1))))
(test '(f 1) (da '(call (ref f) (quote 1))))

;; a wrong type is #f, not an error
(test #f (da-code lc-bytes))              ; a string is not a code vector
(test #f (da-code 42))
(test #f (da-code "not a global name"))
(test #f (da-bytecode 42))
(test #f (da-bytecode '(begin)))          ; a Core expression is not bytecode
(test #f (da-core 42))
(test #f (da-procedure lc-code))          ; a code vector is not a procedure
(test #f (da-procedure 42))
(test #f (da 42))
(test #f (da '()))
(test #f (da #t))

;; a global name nothing holds, or that holds something else
(test #f (da-code 'no-such-global-at-all))
(test #f (da-procedure 'no-such-global-at-all))
(test #f (da 'no-such-global-at-all))
(eval '(define not-a-procedure 9) (interaction-environment))
(test #f (da 'repl://?not-a-procedure))
(test #f (da-global 'repl://?not-a-procedure))
(test #f (da-global 42))


(display "\n--- code with no Core preimage answers with its name ---\n")

;; car is a generated integrable wrapper: da-code works, da-bytecode does not,
;; and what is left to say is the name the store files it under.
(test-assert (string? (da-code car)))
(test #f (da-bytecode car))
(test 'car (da-procedure car))
(test 'car (da-procedure 'car))
(test 'car (da car))
(test 'car (da 'car))
(test 'car (da-name car))

;; two names for one procedure: the integrable's own name is preferred
(test-assert (eq? promise? box?))
(test 'box? (da-name box?))

;; a procedure no global holds has no name
(test #f (da-name (lambda (x) x)))

;; a hand-written one: also no preimage, also named
(test 'values (da-procedure values))
(test 'values (da 'values))


(display "\n--- da-prune-globals ---\n")

;; A global reaches Core under the store name of the environment that allocated
;; it.  Pruned, it reads as source; unpruned, it says where the binding lives.
(eval '(define disasm-test-target (lambda (x) (disasm-test-helper x)))
      (interaction-environment))

(test '(lambda (.a) (disasm-test-helper .a))
      (da 'repl://?disasm-test-target))
(test '(lambda (.a) (repl://?disasm-test-helper .a))
      (parameterize ([da-prune-globals #f]) (da 'repl://?disasm-test-target)))

;; a built-in name has no prefix to prune either way
(eval '(define disasm-test-car (lambda (x) (car x))) (interaction-environment))
(test '(lambda (.a) (car .a)) (da 'repl://?disasm-test-car))
(test '(lambda (.a) (car .a))
      (parameterize ([da-prune-globals #f]) (da 'repl://?disasm-test-car)))

;; Core handed over directly keeps the names it came with -- here the expander's
;; own gensyms -- since only the bytecode path has to invent them.
(let ([form (da-core (expand '(lambda (x) (car x))))])
  (test 'lambda (car form))
  (test 1 (length (cadr form)))
  (test (list 'car (car (cadr form))) (caddr form)))

(test #t (da-prune-globals))


(display "\n--- an empty begin reads back as (void) ---\n")

;; Core's begin with no forms is a value, and (void) names that value.  They are
;; not the same code, so a form carrying one does not compile back byte for byte:
;; the round-trip checks above ask for the exact form instead.
(test '(void) (da-core '(begin)))
(test '(void) (da '(begin)))
(test '(f (void)) (da-core '(call (ref f) (begin))))
(test '(lambda (x) (void)) (da-core '(lambda (x) (begin))))
(test '(begin a (void)) (da-core '(begin (ref a) (begin))))

;; a genuine call of void reads back the same way -- it is the same value
(test '(void) (da-core (expand '(void))))

(parameterize ([da-void-for-empty-begin #f])
  (test '(begin) (da-core '(begin)))
  (test '(lambda (x) (begin)) (da-core '(lambda (x) (begin)))))

(test #t (da-void-for-empty-begin))


(display "\n--- da, and the display it may not have ---\n")

(define (make-adder n) (lambda (x) (+ x n)))
(define (make-counter start) (let ([n start]) (lambda () (set! n (+ n 1)) n)))

;; a live closure brings its display, so the let binds values
(test '(let ((:a 7)) (lambda (.a) (+ .a :a))) (da (make-adder 7)))
;; a boxed slot holds a box, and what is bound is its contents
(test '(let ((:a 10)) (lambda () (set! :a (+ :a 1)) :a)) (da (make-counter 10)))

;; the same code without the closure has the names but no values
(test '(let ((:a ?)) (lambda (.a) (+ .a :a))) (da (procedure-code (make-adder 7))))
(test (da (procedure-code (make-adder 7))) (da (da-code (make-adder 7))))

;; a procedure with no free variables needs no wrapper either way
(test (da list-copy) (da (procedure-code list-copy)))
(test-assert (eq? 'lambda (car (da (procedure-code list-copy)))))

;; a whole instruction stream, which is what the compiler emits for a top-level
;; expression, decodes as one
(test '(lambda (.a) .a) (da (compile-to-string (expand '(lambda (x) x)))))



(display "\n--- case-lambda ---\n")

;; A case-lambda does not compile to one procedure with several arities: it
;; compiles to a dispatcher whose display holds one closure per clause.  So the
;; clauses are read one at a time and put back together.
(define cl-plain (case-lambda [(x) x] [(x y) (cons x y)]))

(test '(case-lambda ((.a) .a) ((.a .b) (cons .a .b))) (da cl-plain))

;; a clause with a rest argument, and one that takes anything
(test '(case-lambda ((.a) .a) ((.b . .a) (cons .b .a)))
      (da (case-lambda [(x) x] [(x . rest) (cons x rest)])))
(test '(case-lambda (.a .a)) (da (case-lambda [args args])))

;; The clauses each carry their own display, and where they close over the same
;; variable they hold the same cell -- so it is named once and bound outside.
(test '(let ((:a 5)) (case-lambda ((.a) (+ .a :a)) ((.a .b) (cons .a .b))))
      (da (let ([k 5]) (case-lambda [(x) (+ x k)] [(x y) (cons x y)]))))

;; and that is what makes an assigned free variable come back as one variable
;; rather than as one per clause
(test '(let ((:a 0)) (case-lambda (() :a) ((.a) (set! :a .a) :a)))
      (da (let ([n 0]) (case-lambda [() n] [(x) (set! n x) n]))))

;; The optional-argument procedures the implementation generates have the same
;; dispatcher shape, so they read back the same way.
(test-assert (eq? 'case-lambda (car (da 'string-copy))))
(test-assert (eq? 'case-lambda (car (da 'vector-fill!))))

;; A dispatcher's clauses live in its display, so a code vector on its own says
;; nothing at all -- unlike an ordinary procedure, where the names survive.
(test #f (da (procedure-code cl-plain)))
(test #f (da-bytecode cl-plain))


(display "\n--- an integrable index stands for a global name ---\n")

;; Core shows an integrable as its index, so being able to hand that index
;; straight back is a convenience when reading a disassembly at a REPL.  The
;; index is looked up in the integrable table and followed to the global its
;; generated wrapper is filed under, so everything that takes a name takes one.
;;
;; Indices are not stable: they say where an instruction sits in the table, and
;; the table grows.  This is why the tests below find the index rather than
;; writing one down.
(define car-index
  (cadr (caddr (da-bytecode (compile-to-string (expand '(lambda (x) (car x))))))))

(test-assert (exact-integer? car-index))
(test 'car (da car-index))
(test 'car (da-procedure car-index))
(test 'car (da-global car-index))
(test (da-code 'car) (da-code car-index))
(test (da-bytecode 'car) (da-bytecode car-index))
(test (da-core 'car) (da-core car-index))

;; one whose wrapper does have a readable form
(define void-index (cadr (expand (quote (void)))))
(test-assert (exact-integer? void-index))
(test '(lambda () (void)) (da void-index))

;; nothing that is not an index of an integrable
(test #f (da 0))              ; in range, but machinery rather than an integrable
(test #f (da -1))
(test #f (da 999999))
(test #f (da 1.5))
(test #f (da-code 999999))
(test #f (da-global 999999))
(test #f (da-global 1.5))
(test-assert (string? (da-code cl-plain)))


(display "\n--- internal definitions of a large body ---\n")

;; (quote #f) is both the letrec* placeholder and an ordinary value, so which of
;; the two a store means depends on the group it is in.  Reading it as a
;; placeholder in a body would turn every (define x #f) into a variable waiting
;; for a call-with-values, and the whole body would fall back to a let of
;; placeholders.
(test-assert (eq? 'define (car (caddr (da-core (expand '(lambda (p) (define a #f) (define b 1) (cons a b))))))))
(test '(lambda (.a) (define .b #f) (define .c 1) (cons .b .c))
      (da-core (da-bytecode (compile-to-string
                              (expand '(lambda (p) (define a #f) (define b 1) (cons a b)))))))

;; the same with a define-values group before the #f definition, which is where
;; the two readings collide
(test-assert
  (let ([form (da-core (da-bytecode
                         (compile-to-string
                           (expand '(lambda (p . r)
                                      (define-values (a b) (values 1 2))
                                      (define c #f)
                                      (define d 3)
                                      (list a b c d))))))])
    (and (eq? 'lambda (car form))
         (eq? 'define-values (car (caddr form)))
         (eq? 'define (car (cadddr form))))))

;; A local procedure is defined the short way, whatever its formals look like,
;; and anything else keeps the long one.
(test '(lambda (.a)
         (define (.b .f .g) (+ .f .g))
         (define (.c .i . .h) .h)
         (define (.d . .j) .j)
         (define .e 3)
         (list (.b .a .e) (.c .a) (.d)))
      (da-core (da-bytecode
                 (compile-to-string
                   (expand '(lambda (p)
                              (define (f a b) (+ a b))
                              (define (g a . r) r)
                              (define (h . all) all)
                              (define k 3)
                              (list (f p k) (g p) (h))))))))

;; and a real library procedure with a body of this shape
(test-assert (eq? 'lambda (car (da 'lib://skint/print?pp))))
(test-assert (eq? 'define-values (car (caddr (da 'lib://skint/print?pp)))))

(display "\n--- cursors ---\n")

;; Every entry point takes a cursor into its input and a procedure to receive
;; the same place in its output.  The cursor is an index just past an instruction,
;; as the VM keeps one: where a call returns, or where a failing instruction had
;; got to.  The receiver is called whatever happens, with #f when there is no such
;; place.  TRACKED returns (result . what it was handed).
(define (tracked proc x c)
  (let* ([got 'never] [res (proc x c (lambda (v) (set! got v)))])
    (cons res got)))

(define (every? ok? l) (or (null? l) (and (ok? (car l)) (every? ok? (cdr l)))))

;; the way down from X to the pair P, as car/cdr steps; #f if P is not in X
(define (path-to x p)
  (let walk ([x x] [up '()])
    (cond [(eq? x p) (reverse up)]
          [(pair? x) (or (walk (car x) (cons 'a up)) (walk (cdr x) (cons 'd up)))]
          [else #f])))

;; the pair TRACKED handed over, as a path into the result it came with
(define (tracked-path proc x c)
  (let ([r (tracked proc x c)]) (and (cdr r) (path-to (car r) (cdr r)))))

;; Return points found the way the VM makes them: save's operand is the length of
;; the block after it, and the call made inside resumes just past the block.
(define save-word (vector-ref (deserialize-code "${}") 0))
(define (return-points p)
  (let ([cv (procedure-code p)])
    (let loop ([i 0] [r '()])
      (if (>= i (vector-length cv))
          (reverse r)
          (loop (+ i 1) (if (eq? (vector-ref cv i) save-word)
                            (cons (+ i 2 (vector-ref cv (+ i 1))) r)
                            r))))))

(define (cur-sum l) (if (null? l) 0 (+ (car l) (cur-sum (cdr l)))))
(define (cur-seq f) (f 1) (f 2) (f 3))
(define (cur-defs n) (define (sq y) (* y y)) (define z (sq n)) (+ z (sq 2)))
(define (cur-or f g) (or (f) (g)))
(define (cur-let* f) (let ([a (f)]) (let ([b (f a)]) (let ([c (f b)]) (list a b c)))))
(define (cur-dv f) (define-values (a b) (f)) (list a b))
(define (cur-rev x y) (if (null? x) y (cur-rev (cdr x) (cons (car x) y))))
(define (make-caller f) (lambda (x) (f (f x))))
(define (cur-nest f) (list (f (lambda (x) (f x) (f x) x))))
(define cur-cl (case-lambda [(a) (list (a))] [(a b) (list (a) b)]))

(define *cursor-procs* (list cur-sum cur-seq cur-defs cur-or cur-dv (make-caller car)))

(test-assert (every? (lambda (p) (pair? (return-points p))) *cursor-procs*))
(test '() (return-points cur-rev))

;; The workflow this is for: a frame on a captured stack is a closure next to the
;; index it resumes at, and that index is a cursor as it stands.
(define (stack-frames k)
  (let ([v (closure->vector k)])
    (let loop ([i 2] [r '()])
      (if (>= (+ i 1) (vector-length v))
          r
          (let ([p (vector-ref v i)] [n (vector-ref v (+ i 1))])
            (if (and (closure? p) (integer? n) (exact? n) (>= n 0)
                     (<= n (vector-length (procedure-code p))))
                (loop (+ i 2) (cons (cons p n) r))
                (loop (+ i 1) r)))))))
(define *frames* #f)
(define (cur-probe) (call/cc (lambda (k) (set! *frames* (stack-frames k)))) 0)
(define (cur-probe-sum l) (if (null? l) (cur-probe) (+ (car l) (cur-probe-sum (cdr l)))))
(cur-probe-sum '(1 2))

(define (frame-view fr)
  (let* ([r (tracked da (car fr) (cdr fr))] [out (open-output-string)])
    (print (car r) out print-cursor (cdr r) print-brackets #t)
    (get-output-string out)))
(test '("(lambda (.a) (if (null? .a) (cur-probe) (+ (car .a) [cur-probe-sum (cdr .a)])))"
        "(lambda (.a) (if (null? .a) (cur-probe) (+ (car .a) [cur-probe-sum (cdr .a)])))")
      (map frame-view (filter-if (lambda (fr) (eq? (car fr) cur-probe-sum)) *frames*)))

;; a cursor changes what the receiver hears, never what is returned
(for-each
  (lambda (p)
    (let ([c (car (return-points p))])
      (test (da p) (car (tracked da p c)))
      (test (da-procedure p) (car (tracked da-procedure p c)))
      (test (da-code p) (car (tracked da-code p c)))
      (test (da-bytecode p) (car (tracked da-bytecode p c)))
      (test (da-core p) (car (tracked da-core p c)))))
  *cursor-procs*)
(test (da cur-cl) (car (tracked da cur-cl 0)))

;; every return point is somewhere in the disassembly: at worst the whole of it
(test-assert
  (every? (lambda (p) (every? (lambda (c) (tracked-path da p c)) (return-points p)))
          *cursor-procs*))

;; and so it is for the library's own procedures, at their first return point
(test (length *display-free*)
      (count-if (lambda (n+p)
                  (let* ([p (cdr n+p)] [rps (return-points p)] [r (and (pair? rps) (tracked da p (car rps)))])
                    (or (not r) (not (pair? (car r))) (path-to (car r) (cdr r)))))
                *display-free*))

;; Any index at all gives either nothing or a pair of the form -- never a symbol
;; or a constant that a variable or a literal became.
(test-assert
  (every? (lambda (p)
            (let ([len (vector-length (procedure-code p))])
              (let loop ([i 0])
                (or (> i len)
                    (and (let ([r (tracked da p i)]) (or (not (cdr r)) (path-to (car r) (cdr r))))
                         (loop (+ i 1)))))))
          *cursor-procs*))

;; An index inside an instruction has no counterpart -- save's operand, say -- and
;; neither has one past an instruction that completes nothing, as save itself does.
(test-assert
  (every? (lambda (p)
            (every? (lambda (c)
                      (let* ([i (let find ([i 0])
                                  (if (and (eq? (vector-ref (procedure-code p) i) save-word)
                                           (eqv? (+ i 2 (vector-ref (procedure-code p) (+ i 1))) c))
                                      i
                                      (find (+ i 1))))])
                        (and (not (cdr (tracked da p (+ i 1))))
                             (not (cdr (tracked da-code p (+ i 1))))
                             (not (cdr (tracked da-bytecode p (+ i 1))))
                             (not (cdr (tracked da p (+ i 2))))
                             (not (cdr (tracked da-bytecode p (+ i 2)))))))
                    (return-points p)))
          (list cur-sum cur-seq cur-nest)))
(test #f (cdr (tracked da cur-sum -1)))
(test #f (cdr (tracked da cur-sum (+ 1 (vector-length (procedure-code cur-sum))))))

;; The stages compose.  Going the whole way from a return point lands where going
;; a stage at a time does: through its offset in the bytecode string, and through
;; the Core call.  A string decodes into fresh pairs, so what is compared is where
;; the cursor is, not which pair it is.  da-core binds no display, so these are
;; procedures without one.
(define (stages-agree? p c)
  (let* ([want (tracked-path da p c)]
         [bytes (tracked da-code p c)]
         [core (tracked da-bytecode p c)])
    (and want (cdr bytes) (cdr core)
         (equal? want (tracked-path da-core (car core) (cdr core)))
         (equal? want (tracked-path da (car bytes) (cdr bytes)))
         (equal? (path-to (car core) (cdr core))
                 (tracked-path da-bytecode (car bytes) (cdr bytes))))))

(for-each
  (lambda (p) (test-assert (every? (lambda (c) (stages-agree? p c)) (return-points p))))
  (list cur-sum cur-seq cur-defs cur-or cur-dv))

;; A return point's offset is the end of its save block.  An offset means the
;; index written there -- the first of them, where an instruction writes nothing --
;; and no other offset means anything: not one inside an operand, and not one
;; inside a nested lambda, whose code has indices of its own.
(for-each
  (lambda (p)
    (let* ([bytes (da-code p)]
           [len (vector-length (procedure-code p))]
           [offs (let loop ([i len] [r '()])
                   (if (< i 0) r (loop (- i 1) (cons (cons (cdr (tracked da-code p i)) i) r))))])
      (test-assert (every? (lambda (c) (char=? #\} (string-ref bytes (- (cdr (tracked da-code p c)) 1))))
                           (return-points p)))
      (test-assert
        (let loop ([o 0])
          (or (> o (string-length bytes))
              (let ([m (assv o offs)])
                (and (if m
                         (equal? (tracked-path da bytes o) (tracked-path da p (cdr m)))
                         (not (cdr (tracked da bytes o))))
                     (loop (+ o 1)))))))))
  (list cur-seq cur-nest cur-sum))

;; What a debugger would show at each return point of a procedure.
(define (cursor-views p) (map (lambda (c) (frame-view (cons p c))) (return-points p)))

(test '("(lambda (.a) [.a 1] (.a 2) (.a 3))"
        "(lambda (.a) (.a 1) [.a 2] (.a 3))")
      (cursor-views cur-seq))
(test '("(lambda (.a) (define (.b .d) (* .d .d)) (define .c [.b .a]) (+ .c (.b 2)))"
        "(lambda (.a) (define (.b .d) (* .d .d)) (define .c (.b .a)) (+ .c [.b 2]))")
      (cursor-views cur-defs))
(test '("(lambda (.a .b) (or [.a] (.b)))") (cursor-views cur-or))
;; nested lets are translated once to see whether they merge, and again for real
(test '("(lambda (.a) (let* ((.b [.a]) (.c (.a .b)) (.d (.a .c))) (list .b .c .d)))"
        "(lambda (.a) (let* ((.b (.a)) (.c [.a .b]) (.d (.a .c))) (list .b .c .d)))"
        "(lambda (.a) (let* ((.b (.a)) (.c (.a .b)) (.d [.a .c])) (list .b .c .d)))")
      (cursor-views cur-let*))
;; a call absorbed into a derived form shows the nearest form that is still there
(test '("[lambda (.a) (define-values (.b .c) (.a)) (list .b .c)]") (cursor-views cur-dv))

;; A failure keeps an instruction pointer too.  The frame on top of a failure
;; object's stack is the procedure that failed and the index just past the
;; instruction that failed, and the expression that instruction completes is the
;; one that failed.
(define (failing-frame thunk)
  (guard (e [(failure-object? e)
             (let* ([v (closure->vector e)] [n (vector-length v)] [nirr (vector-ref v (- n 4))])
               (cons (vector-ref v (- n 6 nirr)) (vector-ref v (- n 5 nirr))))])
    (thunk)
    #f))
(define (cur-f1 x) (list (car x)))
(define (cur-f2 v i) (list (vector-ref v i) 2))
(define (cur-f3 x) (car x))
(define (cur-f4 x) (list (x 1)))
(define (cur-f5 x) (x 1))
(define (cur-f6 x c) (list (if c (car x) (cdr x))))
(define (cur-f7 x) (list (< 1 2 x 5)))

(test "(lambda (.a) (list [car .a]))" (frame-view (failing-frame (lambda () (cur-f1 5)))))
(test "(lambda (.a .b) (list [vector-ref .a .b] 2))"
      (frame-view (failing-frame (lambda () (cur-f2 (vector 1) 3)))))
(test "(lambda (.a) [car .a])" (frame-view (failing-frame (lambda () (cur-f3 5)))))
;; a call that fails on a non-procedure fails where it would have returned
(test "(lambda (.a) (list [.a 1]))" (frame-view (failing-frame (lambda () (cur-f4 5)))))
(test "(lambda (.a) [.a 1])" (frame-view (failing-frame (lambda () (cur-f5 5)))))
;; an arm ends where its if does, and the arm is what failed
(test "(lambda (.a .b) (list (if .b (car .a) [cdr .a])))"
      (frame-view (failing-frame (lambda () (cur-f6 5 #f)))))
;; a comparison of several arguments is one expression, whichever step fails
(test "(lambda (.a) (list [< 1 2 .a 5]))" (frame-view (failing-frame (lambda () (cur-f7 'a)))))
(test "(lambda () [< 1 2 3 (quote a) 5 6])"
      (frame-view (failing-frame
                    (lambda () (eval '(< 1 2 3 'a 5 6) (scheme-report-environment 5))))))
(test-assert
  (every? (lambda (thunk) (let ([fr (failing-frame thunk)]) (stages-agree? (car fr) (cdr fr))))
          (list (lambda () (cur-f1 5)) (lambda () (cur-f2 (vector 1) 3)) (lambda () (cur-f4 5)))))
;; A top-level expression runs as a thunk whose code has no arity check -- eval
;; makes one -- and reads back as that thunk.
(test "(lambda () (+ (+ 1 [car 5]) 2))"
      (frame-view (failing-frame
                    (lambda () (eval '(+ 1 (car 5) 2) (scheme-report-environment 5))))))
;; a procedure with no readable form fails with nothing to point at
(test '(car . #f) (let ([fr (failing-frame (lambda () (map car '(5))))]) (tracked da (car fr) (cdr fr))))

;; a display is outside the code, so a closure and its bare code vector put the
;; cursor in the same place
(let ([p (make-caller car)])
  (test-assert
    (every? (lambda (c) (equal? (tracked-path da p c) (tracked-path da (procedure-code p) c)))
            (return-points p))))

;; a case-lambda's dispatcher makes no calls; its clauses are closures of their own
(test #f (cdr (tracked da cur-cl 0)))

;; Core in, with a call of it as the cursor
(let* ([r (tracked da-bytecode cur-sum (car (return-points cur-sum)))]
       [core (car r)] [call (cdr r)])
  (test 'call (car call))
  (test '(cur-sum (cdr .a)) (cdr (tracked da-core core call)))
  (test '(cur-sum (cdr .a)) (cdr (tracked da core call)))
  ;; a cursor is a place in this Core, so an equal copy of it is not one
  (test #f (cdr (tracked da-core core (list-copy call))))
  (test #f (cdr (tracked da-core core 3))))

;; no place to be had: the receiver still hears, and hears #f
(test '(cur-sum . #f) (tracked da-name cur-sum #f))
(test '(#f . #f) (tracked da-global 'no-such-global-anywhere 0))
(test '(#f . #f) (tracked da #t 0))
(test #f (cdr (tracked da cur-sum #f)))

(display "\n--- da-print-hook ---\n")

(define (hooked obj)
  (let ([p (open-output-string)])
    (print obj p print-hooks (add-print-hook (print-hooks) da-print-hook))
    (get-output-string p)))
(define (written obj)
  (let ([p (open-output-string)]) (write obj p) (get-output-string p)))
(define (contains? s sub)
  (let loop ([i 0])
    (and (<= (+ i (string-length sub)) (string-length s))
         (or (string=? sub (substring s i (+ i (string-length sub)))) (loop (+ i 1))))))
(define (prefix? pfx s)
  (and (<= (string-length pfx) (string-length s))
       (string=? pfx (substring s 0 (string-length pfx)))))

;; a named procedure gets its name after #<procedure, and keeps write's address
(test-assert (prefix? "#<procedure car @" (hooked car)))
(test (string-append "#<procedure car" (substring (written car) 11 (string-length (written car))))
      (hooked car))
(test-assert (prefix? "#<procedure da @" (hooked da)))
(test-assert (prefix? "#<procedure box? @" (hooked box?)))
;; the name is da-name's, so it follows da-prune-globals
(test-assert (prefix? "#<procedure lib://skint/disasm?da @"
                      (parameterize ([da-prune-globals #f]) (hooked da))))

;; no name, or not a procedure: the hook declines and write's form stays
;; (a procedure a global holds is named, so this one is kept in a list)
(define anon-list (list (lambda (x) x)))
(define (anon) (car anon-list))
(test #f (da-print-hook (anon)))
(test #f (da-print-hook 42))
(test #f (da-print-hook 'car))
(test (written (anon)) (hooked (anon)))
(test "(1 \"s\" #(a))" (hooked '(1 "s" #(a))))

;; inside a disassembly, where the procedures a closure holds are quoted
(define holds-car (let ([c car] [i (anon)]) (lambda (x) (i (c x)))))
(let ([s (let ([p (open-output-string)])
           (pretty-print (da holds-car) p
                         print-hooks (add-print-hook (print-hooks) da-print-hook))
           (get-output-string p))])
  (test-assert (contains? s "'#<procedure car @"))
  (test-assert (contains? s "'#<procedure @")))

(display "\n--- malformed input of the right type ---\n")

;; A vector that is not a code vector is the right type with the wrong contents.
;; da-code has to name every instruction word it writes, so it raises; the
;; decoders can always answer "nothing could be made of this", so they do.
(test-error (da-code (vector 1 2 3)))
(test #f (da (vector 1 2 3)))
(test #f (da-bytecode (vector 1 2 3)))
(test #f (da-core (vector 1 2 3)))

(test-end)
