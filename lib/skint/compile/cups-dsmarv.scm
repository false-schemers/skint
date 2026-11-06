;;; cups-dsmarv.scm -- DS-MARV: a direct-style backend branch.
;;;
;;; Loaded after cups-base, cups-signatures, cups-passes and
;;; cups-front (see ../cups-ds).  It reuses the whole CUPS front:
;;; Skint expansion (expand-program-file), parse-core, fix-letrecs,
;;; remove-assignments, analyze-globals (the library shaking),
;;; constant-fold, beta-substitute, name-init-chain-lets and
;;; lift-quoted-constants -- and then, instead of cps-convert,
;;; lowers the direct-style AST (quote/var/if/primapp/app/lambda/
;;; fix/letcc/withcc exps) to a modified S-MARV module.
;;;
;;; The discipline (Skint's, made explicit; see
;;; ../stack-vs-cps-research.md):
;;;
;;;   * The program becomes a stream of INSTRUCTIONS: MARV funcs
;;;     with one uniform signature
;;;       (param $sp i64) (param $fp i64) (param $env i64)
;;;       (param $ac i64) (param $r ptr) (param $hp ptr)
;;;       (param $nargs i32) (result ptr)
;;;     transferred to ONLY by (return_call ...) /
;;;     (return_call_indirect ...).  The chain is entered through
;;;     a (trampoline_func $ds_run ...) called from $main, and
;;;     every inter-procedure transfer -- calls (ds-transfer-args,
;;;     ds-transfer-indirect) and returns (ds-transfer-return) --
;;;     is a trampoline_jump / trampoline_jump_indirect, so the
;;;     C stack resets at every procedure boundary no matter how
;;;     the host C compiler treats a plain return_call; chaining
;;;     within a procedure stays a plain return_call.  The chain
;;;     unwinds exactly once, when the $ds_halt instruction
;;;     returns (ptr.null) into $main.  The pair (next-instruction,
;;;     stack) is Skint's implicit continuation made explicit: the
;;;     "next instruction" travels as a funcref in the frame's
;;;     return slot; sequential flow chains to statically named
;;;     instructions; joins and returns go through funcref values.
;;;
;;;   * $r is the value stack AND the GC root set: $scmrt:collect
;;;     is called with (r, sp) and scans/updates $r[0..$sp), like
;;;     Skint's r[0..sp].  Frame layout ($fp = arg 0):
;;;       fp-4 saved env | fp-3 return funcref | fp-2 saved $fp |
;;;       fp-1 env mirror (root for current env)
;;;     An ordinary call pushes 4 header words + args and
;;;     return_calls the callee.  NOTHING is heap-allocated for
;;;     control -- no continuation closures, ever.
;;;
;;;   * Closures are (code envvec) pairs; envvec = [parent vars..]
;;;     allocated once per activation of a capturing frame (Skint
;;;     displays).  Captured+assigned variables are boxes, so
;;;     call/cc stack snapshots cannot rewind mutations.
;;;     call/cc (letcc/withcc exps): capture = copy $r[0..$sp)
;;;     into a heap vector; throw = copy back and transfer
;;;     through the restored return funcref.
;;;
;;;   * --balance N is the unit-size knob: an instruction holds at
;;;     most ~N AST work nodes (calls/branches/joins always bound
;;;     instructions; the counter cuts straight-line sequences at
;;;     statement boundaries).  Small N: many small instructions,
;;;     maximal instruction reuse (identical instruction bodies
;;;     are hash-consed into one MARV func -- stats say how much);
;;;     large N: fat basic-block instructions, fewer transfers.
;;;
;;; Stage limitations (errors, not silent miscompilation):
;;; lambda* (case-lambda), asm, varaddress, and prims outside
;;; the implemented table are rejected.

;;; ---------------------------------------------------------------
;;; utilities / output
;;; ---------------------------------------------------------------

(define (ds-die . xs) (apply error (cons "cups-dsmarv" xs)))


(define *ds-op* #f)
(define (ds-iota n) (iota 0 n))
(define (ds-out s) (display s *ds-op*) (newline *ds-op*))

;;; MARV as data, and the one procedure that makes characters of it
;;; ---------------------------------------------------------------
;;;
;;; MARV's syntax is fully parenthesised, so a MARV program IS Scheme
;;; data: a form is a list whose head is a symbol, its operands are
;;; forms, symbols and numbers, and the only strings in it are MARV's
;;; own string literals.  Nothing in the generator concatenates MARV
;;; text and nothing downstream reads any back; marv-write walks the
;;; data to the port once, at the end.
;;;
;;; (%text s) is the migration escape: a leaf still built as characters,
;;; which the writer prints verbatim.  A plain string is a MARV string
;;; literal and is written with its quotes and escapes.  The number of
;;; %text leaves left is what says how much of the generator has not
;;; been converted yet, and it is meant to reach nought. [cco]

;; MARV spells an integer constant and a float constant differently, and
;; the only float operand it carries is f64.const's, so THE FORM says
;; which -- not the host's notion of exactness.  That distinction used to
;; be read off the number itself, which made the output depend on the
;; host's numeric tower: on a towerless Skint the 32-bit constants arrive
;; as whole FLONUMS and would have been spelled "4294967295.0", which
;; MARV reads as a float.  They are not wrong, only inexact: a flonum
;; carries every integer up to 2^53, well above the 2^32 these reach.
;; [cco]
(define *marv-float-heads* '(f64.const f32.const))

;; A MARV integer constant, spelled without a decimal point whatever the
;; host made of the value.  `exact` is no help on a towerless host: the
;; value has no exact representation there, which is the whole reason it
;; is a flonum.  So the digits come from number->string, and a value that
;; is not whole is a fault rather than something to round. [cco]
(define (marv-integer-string n)
  (cond [(exact? n) (number->string n)]
        [(not (integer? n)) (ds-die "not a whole MARV integer" n)]
        [else
         (let* ([s (number->string n)] [k (string-length s)])
           (if (and (> k 2) (string=? (substring s (- k 2) k) ".0"))
               (substring s 0 (- k 2))
               (ds-die "no integer spelling for this MARV constant" n)))]))

(define (marv-write x port) (marv-write-at x port #f))

(define (marv-write-at x port float?)
  (cond [(and (pair? x) (eq? (car x) '%text))
         (display (cadr x) port)]
        [(pair? x)
         ;; only the operands of a float head are floats, and a float
         ;; operand is always a number, so the flag never has to nest
         (let ([fop? (and (symbol? (car x))
                          (memq (car x) *marv-float-heads*) #t)])
           (write-char #\( port)
           (let loop ([xs x] [first #t])
             (cond [(null? xs) (write-char #\) port)]
                   [else (unless first (write-char #\space port))
                         (marv-write-at (car xs) port (and fop? (not first)))
                         (loop (cdr xs) #f)])))]
        [(symbol? x) (display x port)]
        ;; a MARV string literal: the escaping is the writer's, which is
        ;; why what a form carries is the payload and not characters
        [(string? x) (write-char #\" port)
                     (display (escape-blob x) port)
                     (write-char #\" port)]
        [(bytevector? x) (write x port)]
        [(number? x) (if float?
                         (display (inexact x) port)
                         (display (marv-integer-string x) port))]
        [else (ds-die "not a MARV datum" x)]))

;;; The pretty-printer.  A form that CONTAINS INSTRUCTIONS breaks: its
;;; head, and whatever of it names rather than computes, stay on the
;;; opening line, each instruction inside it goes on a line of its own
;;; indented by two, and its closing paren returns to the head's
;;; indent.  A form that computes a value stays on one line however
;;; deep it nests, because it is one instruction's worth of work.
;;;
;;; So the shape of the output is the shape of the control flow, which
;;; is what someone reading it wants to see: the arms of an if, the
;;; body of a loop and the instructions of a func are each a column.
;;; There is no line width to fit -- a value expression is as long as
;;; it is -- because the only thing being laid out is which instruction
;;; is which. [cco]

;; (head name instr ...): the name belongs on the opening line
(define *marv-named-blocks* '(module func trampoline_func loop block))
;; (head instr ...): everything after the head is an instruction
(define *marv-bare-blocks* '(then else))
;; A func's signature and locals are not instructions, so they do not
;; each earn a line; they go together on one, where they read as the
;; boilerplate they are.
(define *marv-header-heads* '(param result local export))

(define (marv-print x port indent)
  (cond
   [(not (pair? x)) (marv-line x port indent)]
   [(memq (car x) *marv-named-blocks*)
    (marv-block (list (car x) (cadr x)) (cddr x) port indent)]
   [(memq (car x) *marv-bare-blocks*)
    (marv-block (list (car x)) (cdr x) port indent)]
   [(and (eq? (car x) 'if) (marv-statement-if? x))
    ;; the condition computes a value, so it stays on the opening line;
    ;; the arms hold instructions
    (marv-block (list (car x) (cadr x)) (cddr x) port indent)]
   [else (marv-line x port indent)]))

;; An if that yields a value declares its result type and its arms hold
;; one expression each, so it is part of the instruction that reads it
;; and stays inline.  An if without a result type is a statement.
(define (marv-statement-if? x)
  (not (and (pair? (cdr x)) (pair? (cadr x)) (eq? (car (cadr x)) 'result))))

(define (marv-header? x)
  (and (pair? x) (memq (car x) *marv-header-heads*)))

(define (marv-block opening members port indent)
  (marv-indent indent port)
  (write-char #\( port)
  (display (car opening) port)
  (for-each (lambda (e) (write-char #\space port) (marv-write e port))
            (cdr opening))
  (newline port)
  ;; the run of header forms, if any, on one line
  (let loop ([ms members] [first #t])
    (cond [(and (pair? ms) (marv-header? (car ms)))
           (if first (marv-indent (+ indent 2) port) (write-char #\space port))
           (marv-write (car ms) port)
           (loop (cdr ms) #f)]
          [else
           (unless first (newline port))
           (for-each (lambda (e) (marv-print e port (+ indent 2))) ms)]))
  (marv-indent indent port)
  (write-char #\) port)
  (newline port))
(define (marv-line x port indent)
  (marv-indent indent port)
  (marv-write x port)
  (newline port))

(define (marv-indent n port)
  (do ([i 0 (+ i 1)]) ((>= i n)) (write-char #\space port)))

;; one MARV form on its own line, at an indent
(define (ds-out-form x indent)
  (do ([i 0 (+ i 1)]) ((>= i indent)) (write-char #\space *ds-op*))
  (marv-write x *ds-op*)
  (newline *ds-op*))

;; Names are symbols; ds-name spells one out of pieces, which is the
;; only thing the old text concatenation is still needed for.
(define (ds-name . xs)
  (string->symbol
   (apply string-append
          (map (lambda (x)
                 (cond [(string? x) x]
                       [(number? x) (number->string x)]
                       [(symbol? x) (symbol->string x)]
                       [else (ds-die "ds-name" x)]))
               xs))))

;; A register name reaches an IL form either as a symbol written by
;; hand or as one the generator spelled with ds-name, and in a few
;; places still as the string it used to be when MARV was text.  One
;; coercion covers all three, and the writer never sees a string where
;; a name belongs -- which it would print with its quotes. [cco]
;; A unit or procedure name: a symbol, which is what a MARV label is.
(define (ds-label x)
  (cond [(symbol? x) x]
        [(string? x) (string->symbol x)]
        [else (ds-die "not a label" x)]))

;; (local.get r) with the register coercion applied: the one way to
;; build one, so a name that is still a string cannot reach the
;; writer and be quoted as a MARV string literal. [cco]
(define (ds-get r) (list 'local.get (ds-reg r)))

(define (ds-marv-payload x)
  (if (string? x) (ds-die "an unconverted MARV leaf" x) x))

(define (ds-reg r)
  (let ([s (cond [(symbol? r) (symbol->string r)]
                 [(string? r) r]
                 [else (ds-die "not a register name" r)])])
    ;; A name written by hand in an IL form omits the $ -- (get ac) --
    ;; while one the generator spelled already carries it.  Deciding
    ;; which by asking whether it is a symbol stopped working the moment
    ;; ds-fresh-temp started answering symbols: every temp came out as
    ;; $$t0.  Ask the name itself. [cco]
    (if (and (> (string-length s) 0) (char=? (string-ref s 0) #\$))
        (string->symbol s)
        (ds-name "$" s))))

(define (i64c n) (list 'i64.const n))

;; an i32 count as a fixnum object: (payload<<2)|3 in the low 32 bits,
;; which is the same encoding ds-immediate-expr spells for a literal
(define (ds-fixnum-of i32-form)
  (list 'i64.or
        (list 'i64.shl (list 'i64.extend_i32_u i32-form) '(i64.const 2))
        '(i64.const 3)))

;; slot address / access ($r + 8*(fp+k)); slot at fp-3 holds a
;; funcref and is accessed with the ptr forms.  The index is a number
;; for an ordinary slot, or a MARV form where it is computed at run
;; time -- which is what the hand-written units need, and is why they
;; no longer carry a second copy of these three. [cco]
(define (slot-addr k)
  (list 'ptr.add64 '(local.get $r)
        (list 'i64.shl
              (list 'i64.add '(local.get $fp)
                    (if (number? k) (i64c k) k))
              '(i64.const 3))))
(define (slot-load k) (list 'i64.load 'offset 0 (slot-addr k)))
(define (slot-store k v) (list 'i64.store 'offset 0 (slot-addr k) v))

;; envvec cell i lives at byte 8*i of the env object
(define (env-cell env-expr idx)
  (list 'i64.load 'offset (* 8 idx)
        (list 'ptr.add64 '(ptr.null) env-expr)))
(define (env-walk-expr depth)
  ;; depth 0 = the current frame's envvec, read from the $env local.
  ;;
  ;; This used to read the fp-1 mirror slot every time, on the grounds
  ;; that "the $env local goes stale at any reserve".  That had it
  ;; backwards: ds-reserve emits (env-refresh), which is exactly
  ;; (local.set $env (slot-load -1)), so a reserve LEAVES $env fresh.
  ;; The one place the two diverged was the envvec build storing a new
  ;; envvec to the mirror without updating the local, and that now
  ;; updates it, so the local is always equal to the slot and is the
  ;; cheaper of the two to read. [cco]
  (let loop ([d depth] [e '(local.get $env)])
    (if (= d 0) e (loop (- d 1) (env-cell e 0)))))

;;; ---------------------------------------------------------------
;;; The Unit IL
;;; ---------------------------------------------------------------
;;;
;;; A unit's payload is a list of instructions.  An instruction is
;;; either an IL form, which is a list, or a string, which is text the
;;; generator built directly.  Both may appear in the same unit, so call
;;; sites convert one family at a time and the output does not move:
;;; ds-close-unit! prints the IL forms through ds-il->text, and the
;;; printer reproduces exactly the text the generator used to build.
;;;
;;; The rule that keeps the two halves apart: the IL decides what runs,
;;; the printer decides how it is spelled.  Anything that affects which
;;; unit runs next belongs in the IL.  Anything that differs only between
;;; MARV dialects or link modes belongs here.
;;;
;;; Forms so far:
;;;   (sp-set k)        $sp := $fp + k
;;;   (const r N)       register r := the immediate N
;;;   (set r opnd)      local r := opnd (the general assignment)
;;;   (load k)          $ac := slot k
;;;   (load-to r k)     local r := slot k
;;;   (load-fn k)       $p := the funcref in slot k
;;;   (code-ptr o)      $p := the code pointer of the closure o
;;;   (slot-ptr k)      $p := the ADDRESS of slot k
;;;   (store k opnd)    slot k := opnd
;;;   (global-store id o)  the global named id := o
;;;   (cell-store e i o)   cell i of the envvec e := o
;;;   (arity-error n)   report a call of the wrong arity
;;;   (halt)            end the unit without transferring
;;;   (rt name)         expand the named runtime macro
;;;   (rt1 name o)      the same, with one operand
;;;   (rt2 name o p)    the same, with two
;;;   (push opnd)       the value goes on the operand stack
;;;   (cell-set o i v)  cell i of heap block o := v
;;;   (reserve w b)     collect unless the heap covers the request
;;;   (alloc r kind n)  local r := a fresh block of n cells
;;;   (fn-store d nm)   the funcref nm := at address d
;;;   (env-refresh)     reload $env from the fp-1 mirror
;;;   (box-set o v)     the box o := v
;;;   (if-inline f t)   a two-armed test inside one unit
;;;   (guard c body)    run body only if the i32 condition holds
;;;   (branch c t e)    the general two-armed statement test
;;;   (loop lbl body)   a MARV loop the body branches back into
;;;   (br lbl)          leave a block, or begin a loop's next turn
;;;   (br-if lbl c)     the same, only if the i32 condition holds
;;;   (step r n)        advance the counter in r by n
;;;   (rtcall name a ...)  a runtime call for what it does, not its value
;;;   (block lbl body)  a MARV block the body branches out of
;;;   (chain t)         sequential flow to the named unit
;;;   (fork uf ut)      end the unit by testing and chaining
;;;   (jump t e n s f)  transfer to a named target
;;;   (jump-ind c e n s f)  transfer through a funcref value
;;;   (ret te tf)       tail-return through the popped frame
;;;
;;; An operand is a value an instruction reads:
;;;   (get r)           a local
;;;   (imm N)           an encoded object
;;;   (slot k)          the contents of frame slot k
;;;   (cmp op a b)      a comparison, read as a condition
;;;   (rep op a ...)    a value in the runtime's own representation
;;;   (cell-ref o i)    cell i of an object, read raw (see cell-store)
;;;   (i32 x)           an i64 value narrowed to i32
;;;   (rtcall name a ...)  a call into the runtime
;;;   (nargs64)         the argument count, widened to i64
;;;   (marv x)          a MARV form the IL does not yet name
;;; [cco]

;; An instruction lowers to one MARV form, or to several when it has
;; arms.  ds-il-marv lowers a unit's instructions into the list of MARV
;; forms the writer puts one per line.
(define (ds-il-marv xs)
  (let loop ([xs xs] [acc '()])
    (cond [(null? xs) (reverse acc)]
          [else
           (let ([m (ds-lower (car xs))])
             (loop (cdr xs)
                   (if (and (pair? m) (eq? (car m) '%seq))
                       (append (reverse (cdr m)) acc)
                       (cons m acc))))])))

(define (ds-lower form)
  (case (car form)
    [(sp-set)
     (list 'local.set '$sp
           (list 'i64.add '(local.get $fp) (i64c (cadr form))))]
    [(const)
     ;; (const ac N): $ac := the immediate N.  The IL carries the
     ;; encoded object, not the Scheme value, because the encoding is
     ;; the generator's business and the lowering only spells it.
     (list 'local.set (ds-name "$" (cadr form)) (i64c (caddr form)))]
    [(load)
     (list 'local.set '$ac (slot-load (cadr form)))]
    [(load-to)
     ;; (load-to r k): local r := slot k.  Distinct from (load k),
     ;; which always targets $ac; here r is a temp the generator
     ;; allocated, so its name arrives carrying its own $.
     (list 'local.set (ds-reg (cadr form)) (slot-load (caddr form)))]
    [(code-ptr)
     ;; (code-ptr o): $p := the code pointer of the closure o, which is
     ;; what an indirect call jumps through.  The object counterpart of
     ;; load-fn, which reads one out of a frame slot instead. [cco]
     (list 'local.set '$p (ds-code-ptr (ds-opnd (cadr form))))]
    [(load-fn)
     ;; the return slot holds a funcref, which is read with the ptr
     ;; forms rather than i64.load, and lands in $p
     (list 'local.set '$p
           (list 'ptr.load 'offset 0 (slot-addr (cadr form))))]
    [(slot-ptr)
     ;; (slot-ptr k): $p := the address of slot k, not its contents.
     ;; The runtime macros that build a pair read their two operands
     ;; from consecutive slots through $p, so pointing $p at the first
     ;; of them is part of that protocol and belongs in the IL. [cco]
     (list 'local.set '$p (slot-addr (cadr form)))]
    [(set)
     ;; (set r opnd): local r := opnd.  The general assignment; the
     ;; named forms above are the cases worth naming.
     (list 'local.set (ds-reg (cadr form)) (ds-opnd (caddr form)))]
    [(push)
     ;; (push opnd): the value goes on the MARV operand stack, where
     ;; the next operation's effect clause expects to find it.  A bare
     ;; form is a push in MARV, so this lowers to its operand alone.
     ;; It is an instruction rather than a loose operand because one
     ;; that leaves a value behind is not the same as one that does
     ;; not, and a peephole fusing a push into its consumer has to be
     ;; able to tell them apart. [cco]
     (ds-opnd (cadr form))]
    [(alloc)
     ;; (alloc r kind n): local r := a fresh block of n cells.  The
     ;; kind chooses the header, which is what distinguishes a vector
     ;; from the packed block a record or a pair is made of.
     (list 'local.set (ds-reg (cadr form))
           (list (ds-name "$scmn:hend_" (caddr form))
                 (ds-opnd (cadddr form))))]
    [(fn-store)
     ;; a funcref is a code pointer, so it is written with the ptr
     ;; forms rather than i64.store
     (list 'ptr.store 'offset (if (pair? (cdddr form)) (cadddr form) 0)
           (ds-marv-payload (cadr form))
           (list 'ref.func (ds-label (caddr form))))]
    [(reserve)
     ;; (reserve words bytes): collect unless the heap can cover the
     ;; request.  Both operands are i32: a reserve for a vector of
     ;; runtime length is computed, a constant one passes (imm-i32 n).
     (list 'if
           (list 'ptr.lt_u '(local.get $hp)
                 (list 'ptr.add '(global.get $scmrt:heap_limit)
                       (ds-opnd (caddr form))))
           (list 'then
                 (list 'local.set '$hp
                       (list 'call '$scmrt:collect
                             '(local.get $r)
                             '(i32.wrap_i64 (local.get $sp))
                             '(local.get $hp)
                             (ds-opnd (cadr form))))))]
    [(env-refresh)
     ;; a collection moves the envvec, so $env is reloaded from the
     ;; fp-1 mirror slot rather than trusted
     (list 'local.set '$env (slot-load -1))]
    [(rt)
     ;; (rt name): expand the runtime macro of that name here.  Its
     ;; operands are already where the macro's effect clause says they
     ;; are, in $ac and on the MARV operand stack, so the form carries
     ;; only the name.
     (list (ds-name "$scmi:" (cadr form)))]
    [(rt1)
     (list (ds-name "$scmi:" (cadr form)) (ds-opnd (caddr form)))]
    [(rt2)
     (list (ds-name "$scmi:" (cadr form))
           (ds-opnd (caddr form)) (ds-opnd (cadddr form)))]
    [(cell-set)
     ;; (cell-set obj i val): cell i of a heap block.  Vectors and
     ;; records are both reached this way, which is why the operation
     ;; is named for the representation rather than for either type.
     (list '$scmn:block_set (ds-opnd (cadr form))
           (list 'i32.const (caddr form))
           (ds-opnd (cadddr form)))]
    [(box-set)
     (list '$scmn:box_set (ds-opnd (cadr form)) (ds-opnd (caddr form)))]
    [(store)
     (slot-store (cadr form) (ds-opnd (caddr form)))]
    [(global-store)
     ;; (global-store id opnd): the global named by id := opnd.  Where
     ;; that store lives and how its address is reached is the link
     ;; mode's business; the IL carries the name. [cco]
     (global-store-expr (cadr form) (ds-opnd (caddr form)))]
    [(cell-store)
     ;; (cell-store env i opnd): cell i of the envvec env := opnd.
     ;; Not cell-set: an envvec is reached raw, off the null base, with
     ;; no header read and no barrier, which is the whole difference
     ;; between a frame's cells and those of a vector or a record.
     ;; Together with store, global-store and box-set this completes
     ;; the four places a variable can live. [cco]
     (list 'i64.store 'offset (* 8 (caddr form))
           (list 'ptr.add64 '(ptr.null) (ds-opnd (cadr form)))
           (ds-opnd (cadddr form)))]
    [(frame-fill)
     ;; (frame-fill npar rest? bound): give every slot in
     ;; [fp+npar, fp+bound) a valid value, so a collection that scans
     ;; the frame before those slots are first written finds objects
     ;; rather than whatever the previous incarnation left.
     ;;
     ;; A fixed-arity proc fills with ONE memory.fill.  The dummy is #f
     ;; rather than '() so that it can: memory.fill replicates a single
     ;; byte, and #f is the zero word while '() is 13, which no byte
     ;; repeats into.  Both are valid objects and neither is ever read
     ;; -- the fill exists for the collector, not for the program -- so
     ;; the choice is free, and it buys a byte count that is a
     ;; compile-time constant and no cursor or branch at all.  This is
     ;; paid on every call, so tak, whose frame is 14 slots past its
     ;; parameters, spent 30% of its time in the loop this replaces.
     ;; A rest-arg proc keeps the loop: its lower bound is $nargs, so
     ;; the count is dynamic and may be negative when nargs exceeds the
     ;; static bound, which the loop's unsigned compare already
     ;; handles. [cco]
     (let* ([npar (cadr form)] [rest? (caddr form)] [bound (cadddr form)]
            [cursor
             (list 'local.set '$t7
                   (list 'i64.add '(local.get $fp)
                         (if rest?
                             '(i64.extend_i32_u (local.get $nargs))
                             (i64c npar))))])
       (unless bound (ds-die "frame fill bound was never set" form))
       (if (not rest?)
           ;; nothing to cover when the frame is all parameters
           (if (<= bound npar)
               '(%seq)
               (list 'memory.fill
                     (list 'ptr.add64 '(local.get $r)
                           (list 'i64.shl
                                 (list 'i64.add '(local.get $fp) (i64c npar))
                                 '(i64.const 3)))
                     '(i32.const 0)
                     (list 'i32.const (* 8 (- bound npar)))))
           (cons '%seq
                 (list cursor
                       (list 'block '$ds_fill_done
                             (list 'loop '$ds_fill_lp
                                   (list 'br_if '$ds_fill_done
                                         (list 'i64.ge_u '(local.get $t7)
                                               (list 'i64.add '(local.get $fp)
                                                     (i64c bound))))
                                   (list 'i64.store 'offset 0
                                         (list 'ptr.add64 '(local.get $r)
                                               (list 'i64.shl '(local.get $t7)
                                                     '(i64.const 3)))
                                         (i64c 13))
                                   (list 'local.set '$t7
                                         (list 'i64.add '(local.get $t7)
                                               '(i64.const 1)))
                                   '(br $ds_fill_lp)))))))]
    [(marv)
     ;; (marv x): an instruction still built as a MARV form by the
     ;; generator rather than named by a verb of its own.  The
     ;; instruction-level counterpart of the (marv x) operand, and
     ;; the honest measure of what the IL does not yet name. [cco]
     (ds-marv-payload (cadr form))]
    [(arity-error)
     ;; (ptr.null), not (i64.const 0): arity_error's first parameter is
     ;; declared `ptr`, which is i32 on the wasm target, so an i64 zero
     ;; is a type error there.  The C and rv64g targets never noticed,
     ;; because both lower a call argument without checking its width.
     ;; [cco]
     (list 'drop
           (list 'call '$scmrt:arity_error '(ptr.null)
                 '(local.get $nargs)
                 (list 'i32.const (cadr form))))]
    [(halt)
     ;; the only instruction that ends a unit without transferring,
     ;; which is why it is an instruction and not a loose datum: every
     ;; other way out of a unit is already one. [cco]
     '(return (ptr.null))]
    [(if-inline)
     ;; (if-inline false-arm true-arm): both arms are instruction
     ;; lists generated into this same unit, which is why the arms are
     ;; the form's operands rather than a run of parens around them.
     ;; The false arm comes first because the test asks whether $ac is
     ;; the false object, and MARV's (then ...) answers that question.
     ;; Nothing in an arm may fork the unit; ds-gen-if only chooses
     ;; this shape where no arm contains a call. [cco]
     (list 'if '(i64.eq (local.get $ac) (i64.const 0))
           (cons 'then (ds-il-marv (cadr form)))
           (cons 'else (ds-il-marv (caddr form))))]
    [(branch)
     ;; (branch cond then-arm else-arm): the general two-armed
     ;; statement test.  guard is its one-armed case, and if-inline
     ;; is the case whose condition is "is $ac the false object",
     ;; which is common enough in generated code to name. [cco]
     (list (quote if) (ds-opnd (cadr form))
           (cons (quote then) (ds-il-marv (caddr form)))
           (cons (quote else) (ds-il-marv (cadddr form))))]
    [(guard)
     ;; (guard cond body): run body only if the i32 condition holds.
     ;; One-armed, so there is no arm to choose between and no (else).
     (list 'if (ds-opnd (cadr form))
           (cons 'then (ds-il-marv (caddr form))))]
    [(rtcall)
     ;; A runtime call can stand as an instruction, where its value is
     ;; not wanted: exit_process and port_write_char are called for
     ;; what they do.  Same lowering either way, so it delegates. [cco]
     (ds-opnd form)]
    [(br)
     ;; (br lbl) and (br-if lbl c): control leaves a block, or begins a
     ;; loop's next turn.  Apart from falling off the end of a body
     ;; these are the only ways out of one, so a reading of the IL that
     ;; wants to know where a loop can exit has to be able to see them.
     ;; [cco]
     (list 'br (ds-label (cadr form)))]
    [(br-if)
     (list 'br_if (ds-label (cadr form)) (ds-opnd (caddr form)))]
    [(step)
     ;; (step r n): advance the counter in r by n, negative for a loop
     ;; that counts down.  Every open-coded loop has exactly one, and
     ;; naming it is what lets a reading of the IL tell a cursor from
     ;; any other assignment.  Whether it comes out as an add or a sub
     ;; is a spelling question, so it is settled here. [cco]
     (let ([r (cadr form)] [n (caddr form)])
       (list 'local.set (ds-reg r)
             (if (< n 0)
                 (list 'i64.sub (ds-get r) (i64c (- n)))
                 (list 'i64.add (ds-get r) (i64c n)))))]
    [(loop)
     ;; (loop label body): a MARV loop.  The br_if that branches back
     ;; to label is an ordinary instruction inside the body, because
     ;; which test an iteration uses is the generator's business; what
     ;; the form says is where the body begins and ends. [cco]
     (cons 'loop (cons (cadr form) (ds-il-marv (caddr form))))]
    [(block)
     ;; (block label body): the counterpart, branched out of rather
     ;; than back into. [cco]
     (cons 'block (cons (cadr form) (ds-il-marv (caddr form))))]
    ;; Transfers.  The IL names the target and says whether the
    ;; transfer is direct or through a value; whether it is spelled
    ;; return_call or trampoline_jump is a link-mode question and is
    ;; settled here, once, rather than at each call site.
    [(chain)
     (cons 'return_call (cons (ds-label (cadr form)) (ds-abi-tail 0)))]
    [(fork)
     ;; (fork u-false u-true): the unit ends by testing $ac and
     ;; chaining to one of two units.  Both are sequential flow inside
     ;; the same procedure, so both are return_call and neither goes
     ;; through the trampoline; this is the one place a unit has two
     ;; successors. [cco]
     (list 'if '(i64.eq (local.get $ac) (i64.const 0))
           (list 'then
                 (cons 'return_call
                       (cons (ds-label (cadr form)) (ds-abi-tail 0))))
           (list 'else
                 (cons 'return_call
                       (cons (ds-label (caddr form)) (ds-abi-tail 0)))))]
    [(jump)
     (let ([target (cadr form)] [env-expr (caddr form)]
           [nargs (cadddr form)] [spv (list-ref form 4)]
           [fpv (list-ref form 5)])
       (append (list (ds-verb) (ds-label target)
                     (ds-marv-payload spv) (ds-marv-payload fpv)
                     (ds-marv-payload env-expr))
               (list '(local.get $ac) '(local.get $r) '(local.get $hp)
                     (ds-nargs nargs))))]
    [(jump-ind)
     (let ([code-expr (cadr form)] [env-expr (caddr form)]
           [nargs (cadddr form)] [spv (list-ref form 4)]
           [fpv (list-ref form 5)])
       (append (list (ds-verb-i)
                     '(param i64 i64 i64 i64 ptr ptr i32) '(result ptr)
                     (ds-marv-payload spv) (ds-marv-payload fpv)
                     (ds-marv-payload env-expr))
               (list '(local.get $ac) '(local.get $r) '(local.get $hp)
                     (ds-nargs nargs) code-expr)))]
    [(ret)
     ;; the frame is already popped into the two temps the form names
     (let ([te (cadr form)] [tf (caddr form)])
       (append (list (ds-verb-i)
                     '(param i64 i64 i64 i64 ptr ptr i32) '(result ptr)
                     '(i64.sub (local.get $fp) (i64.const 4))
                     (ds-get tf) (ds-get te))
               (list '(local.get $ac) '(local.get $r) '(local.get $hp)
                     '(i32.const 0) '(local.get $p))))]
    [else (ds-die "no lowering for IL form" form)]))

;; An argument count is a constant at a compiled call site and a form in
; the hand-written units, which spread a list whose length they counted.
(define (ds-nargs n) (if (number? n) (list 'i32.const n) n))

;; the five registers every transfer passes, and the argument count
(define (ds-abi-tail nargs)
  (list '(local.get $sp) '(local.get $fp) '(local.get $env)
        '(local.get $ac) '(local.get $r) '(local.get $hp)
        (list 'i32.const nargs)))

;; An operand is a value an instruction reads.  (get r) is a local: a
;; symbol names it without the $, which is how an IL form is written by
;; hand, and a name the generator built already carries its own $.
;; (marv x) is a MARV form the generator built itself, and is what the
;; old (text "...") escape became once MARV stopped being characters.
;; The operand heads, which is a closed set.  No MARV form shares one,
;; so an argument position can take either an operand or an already
;; built MARV value and ds-val can tell which without being told --
;; that is what lets the operand forms nest, so a runtime call can sit
;; inside an accessor without either of them knowing about the other.
;; [cco]
(define *il-operand-heads*
  '(get imm imm-i32 slot cmp rep rtcall nargs64 i32 cell-ref marv))

(define (ds-val x)
  (if (and (pair? x) (memq (car x) *il-operand-heads*)) (ds-opnd x) x))

(define (ds-opnd o)
  (case (car o)
    [(get) (ds-get (cadr o))]
    [(imm) (i64c (cadr o))]
    [(imm-i32) (list 'i32.const (cadr o))]
    [(slot) (slot-load (cadr o))]
    ;; A marv payload is a MARV form.  A string there is a leaf nobody
    ;; finished converting, and the writer would quote it as a MARV
    ;; string literal rather than fail -- which is how two of them
    ;; reached the output before this said so. [cco]
    [(marv) (ds-marv-payload (cadr o))]
    ;; (rep op a ...): a value read out of, or built in, the runtime's
    ;; own representation -- unwrap a fixnum, take a pair's cdr, ask a
    ;; block its length, box a double.  One form for the whole family,
    ;; because what varies between them is only which accessor: they
    ;; all compute a value from values, which is what makes them
    ;; operands and not instructions, and which is the question a
    ;; later pass asks when it wants to know whether an operand can be
    ;; moved.  The arguments are MARV forms, since every one of them
    ;; is already a value. [cco]
    [(rep) (cons (ds-name "$scmn:" (cadr o)) (map ds-val (cddr o)))]
    ;; (cell-ref o i): cell i of an object reached RAW, off the null
    ;; base -- the read side of cell-store, not of cell-set.  Those two
    ;; are not duals: cell-set goes through $scmn:block_set, which reads
    ;; a header and may post a barrier, while this and cell-store are a
    ;; plain load and store at a byte offset.  Pairing it with the wrong
    ;; one changed every program.s output, which is what the gate is
    ;; for.  (i32 x): an i64 value narrowed to i32, which every count
    ;; handed to the runtime has to be. [cco]
    [(cell-ref) (list 'i64.load 'offset (* 8 (caddr o))
                      (list 'ptr.add64 '(ptr.null) (ds-val (cadr o))))]
    [(i32) (list 'i32.wrap_i64 (ds-val (cadr o)))]
    ;; (cmp op a b): a comparison.  Every one of them is read by a
    ;; guard, a branch or a br-if, so what the IL gains by naming them
    ;; is that a condition is recognisable as one -- which is what a
    ;; pass that wants to invert a test, or fuse it into the branch
    ;; that reads it, has to start from. [cco]
    [(cmp) (list (ds-name "" (cadr o)) (ds-val (caddr o)) (ds-val (cadddr o)))]
    ;; (rtcall name a ...): a call into the runtime.  Distinct from rep
    ;; because a call may do anything, where an accessor only reads;
    ;; anything reasoning about effects needs the two kept apart. [cco]
    [(rtcall) (cons 'call (cons (ds-name "$scmrt:" (cadr o))
                              (map ds-val (cddr o))))]
    ;; (nargs64): the incoming argument count widened to i64, which is
    ;; what the units that walk the argument area compare against. [cco]
    [(nargs64) '(i64.extend_i32_u (local.get $nargs))]
    [else (ds-die "no lowering for IL operand" o)]))

(define *ds-balance* 24)
(define *ds-dedup?* #t)
(define *ds-trampoline?* #t)    ; #f: plain return_call chains (old output)

;; inter-procedure transfers reset the C stack through the chain's
;; trampoline; sequential chaining within a procedure does not
(define (ds-verb) (if *ds-trampoline?* (quote trampoline_jump) (quote return_call)))
(define (ds-verb-i)
  (if *ds-trampoline?* (quote trampoline_jump_indirect) (quote return_call_indirect)))

;;; ---------------------------------------------------------------
;;; front pipeline (CUPS front minus CPS)
;;; ---------------------------------------------------------------

(define (ds-compile-core core modname)
  (let* ([exp (parse-core core (lambda (e gl) e))]
         [exp (fix-letrecs exp)]
         [exp (remove-assignments exp)]
         [exp (analyze-globals exp)]
         [exp (constant-fold exp)]
         [exp (beta-substitute exp #f #f)]
         [exp (name-init-chain-lets exp)]
         [exp (lift-quoted-constants exp)])
    exp))

;;; ---------------------------------------------------------------
;;; AST utilities
;;; ---------------------------------------------------------------

(define (flatten-begin exp)
  (cond [(begin-exp? exp)
         (append (flatten-begin (begin-exp->exp1 exp))
                 (flatten-begin (begin-exp->exp2 exp)))]
        [(void-exp? exp) '()]
        [else (list exp)]))

(define (exp-cost e)
  (variant-case e
    [quote-exp (obj) 1]
    [var-exp (id) 1]
    [varassign-exp (id exp) (+ 1 (exp-cost exp))]
    [varaddress-exp (id) 1]
    [if-exp (test-exp then-exp else-exp)
     (+ 1 (exp-cost test-exp) (exp-cost then-exp) (exp-cost else-exp))]
    [primapp-exp (effect prim rands)
     (+ 1 (apply + (map exp-cost rands)))]
    [app-exp (rator rands) (+ 1 (exp-cost rator)
                              (apply + (map exp-cost rands)))]
    [lambda-exp (ids body) 3]
    [ilambda-exp (ids id body) 3]
    [lambda*-exp (arities lambdas) 100]
    [fix-exp (ids lams body) (+ 1 (exp-cost body))]
    [letcc-exp (id body) (+ 2 (exp-cost body))]
    [withcc-exp (cont-exp exp) (+ 2 (exp-cost cont-exp) (exp-cost exp))]
    [once-exp (id exp) (exp-cost exp)]
    [else 1]))

(define (exp-contains-call? e)
  (variant-case e
    [app-exp (rator rands) #t]
    [letcc-exp (id body) #t]
    [withcc-exp (cont-exp exp) #t]
    [if-exp (test-exp then-exp else-exp)
     (or (exp-contains-call? test-exp) (exp-contains-call? then-exp)
         (exp-contains-call? else-exp))]
    [primapp-exp (effect prim rands)
     (let loop ([xs rands])
       (cond [(null? xs) #f] [(exp-contains-call? (car xs)) #t]
             [else (loop (cdr xs))]))]
    [varassign-exp (id exp) (exp-contains-call? exp)]
    [fix-exp (ids lams body) (exp-contains-call? body)]
    [once-exp (id exp) (exp-contains-call? exp)]
    [else #f]))

;;; ---------------------------------------------------------------
;;; program model: bindings, frames, procs
;;; ---------------------------------------------------------------

;; binding: (vector 'bnd id kind idx)
;;   kind 'slot|'boxslot: idx = stack slot; 'cell|'boxcell: idx =
;;   envvec cell (1-based; 0 is the parent link); 'glob: gstore idx;
;;   'proc: idx indexes *ds-procs* (local known proc binding)
(define (make-binding id) (vector 'bnd id 'slot 0 #f #f))
(define (bnd-id b) (vector-ref b 1))
(define (bnd-kind b) (vector-ref b 2))
(define (bnd-kind-set! b k) (vector-set! b 2 k))
(define (bnd-idx b) (vector-ref b 3))
(define (bnd-idx-set! b i) (vector-set! b 3 i))
(define (bnd-assigned? b) (vector-ref b 4))
(define (bnd-assigned-set! b v) (vector-set! b 4 v))
(define (bnd-captured? b) (vector-ref b 5))
(define (bnd-captured-set! b v) (vector-set! b 5 v))

;; frame: one lambda activation's bindings (vector 'frame proc
;; bindings cells?) -- bindings an alist id->binding, newest first
(define (make-frame proc) (vector 'frame proc '() #f))
(define (frame-proc f) (vector-ref f 1))
(define (frame-bindings f) (vector-ref f 2))
(define (frame-bindings-set! f v) (vector-set! f 2 v))
(define (frame-find f id)
  (cond [(assq id (frame-bindings f)) => cdr]
        [else (ds-die "unbound" (id->uname id))]))
(define (frame-cells? f) (vector-ref f 3))
(define (frame-cells-set! f v) (vector-set! f 3 v))
(define (frame-add! f id)
  (let ([b (make-binding id)])
    (frame-bindings-set! f (cons (cons id b) (frame-bindings f)))
    b))

;; proc: (vector 'proc name params rest body entry frame gid
;;               maxslot ncells shared?)
(define (make-proc name params rest body gid)
  (vector 'proc name params rest body #f #f gid 0 0 #f '()))
(define (proc-name p) (vector-ref p 1))
(define (proc-name-set! p v) (vector-set! p 1 v))
(define (proc-params p) (vector-ref p 2))
(define (proc-rest p) (vector-ref p 3))
(define (proc-body p) (vector-ref p 4))
(define (proc-entry p) (vector-ref p 5))
(define (proc-entry-set! p v) (vector-set! p 5 v))
(define (proc-frame p) (vector-ref p 6))
(define (proc-frame-set! p v) (vector-set! p 6 v))
(define (proc-gid p) (vector-ref p 7))
(define (proc-maxslot p) (vector-ref p 8))
(define (proc-maxslot-set! p v) (vector-set! p 8 v))
(define (proc-ncells p) (vector-ref p 9))
(define (proc-ncells-set! p v) (vector-set! p 9 v))
(define (proc-def-frames p) (vector-ref p 11))
(define (proc-def-frames-set! p v) (vector-set! p 11 v))

(define *ds-procs* '())          ; all procs, creation order
(define *ds-node-procs* '())     ; alist lambda node -> proc
(define *ds-main-proc* #f)
(define *ds-globals* '())        ; alist global id -> gstore idx
(define *ds-global-procs* '())   ; alist global id -> proc

;; Globals that some (set! g ...) assigns.  A global's entry in
;; *ds-global-procs* records the procedure its DEFINITION made, and an
;; assignment replaces that value at run time, so such a global must not
;; be direct-called: the call would reach the definition's procedure for
;; the whole run.  nboyer and sboyer are built exactly that way --
;; (define (test-boyer . args) #t) and later (set! test-boyer (lambda
;; (alist term n) ...)) -- so the direct call reached the placeholder:
;; nboyer answered #t, and sboyer was rejected for calling a
;; "0-argument" procedure with three arguments. [cco]
(define *ds-assigned-globals* '())
(define *ds-need-throw* #f)
;; Undefined Skint-auxiliary globals (%map1, %for-each, ... --
;; produced when borrowed libraries expand Skint's own map/for-each
;; macro family) are aliased by ds-compile-program to the shadow
;; (scheme base) definitions of the same name; lib/scheme/base.sld
;; carries the DS implementations of that whole family.
(define *ds-proc-seq* 0)

(define (ds-register-proc! p node gid)
  (set! *ds-procs* (cons p *ds-procs*))
  (when node
    (set! *ds-node-procs* (cons (cons node p) *ds-node-procs*)))
  (when gid (set! *ds-global-procs* (cons (cons gid p) *ds-global-procs*)))
  p)

(define (ds-proc-of-node node)
  (cond [(assq node *ds-node-procs*) => cdr]
        [else (ds-die "no proc for lambda node")]))

(define (global-idx id)
  (cond [(assq id *ds-globals*) => cdr]
        [else
         (let ([n (length *ds-globals*)])
           (set! *ds-globals* (cons (cons id n) *ds-globals*))
           n)]))
(define (global-ref-expr id)
  (list 'i64.load 'offset (* 8 (global-idx id)) '(ref.data $gstore)))
(define (global-store-expr id v)
  (list 'i64.store 'offset (* 8 (global-idx id))
        '(ref.data $gstore) v))

;;; ---------------------------------------------------------------
;;; scan: build procs/frames, mark captured/assigned.
;;; scope = list of frames, innermost first.  A reference found in
;;; a frame other than the innermost one is a capture.
;;; ---------------------------------------------------------------

(define (ds-scan-exp e scope)
  (variant-case e
    [quote-exp (obj) #f]
    [var-exp (id) (ds-scan-ref id scope #f)]
    [varassign-exp (id exp)
     (when (and (global-id? id) (not (memq id *ds-assigned-globals*)))
       (set! *ds-assigned-globals* (cons id *ds-assigned-globals*)))
     (ds-scan-ref id scope #t)
     (ds-scan-exp exp scope)]
    [varaddress-exp (id)
     ;; (set& x): the callee writes through the box, so the
     ;; binding must be boxed exactly as if it were assigned
     (unless (global-id? id) (ds-scan-ref id scope #t))]
    [if-exp (test-exp then-exp else-exp)
     (ds-scan-exp test-exp scope)
     (ds-scan-exp then-exp scope)
     (ds-scan-exp else-exp scope)]
    [primapp-exp (effect prim rands)
     (for-each (lambda (x) (ds-scan-exp x scope)) rands)]
    [app-exp (rator rands)
     ;; let-exp: lambda in operator position binds in same frame
     (cond [(lambda-exp? rator)
            (let ([fr (car scope)])
              (for-each (lambda (id) (frame-add! fr id))
                        (lambda-exp->ids rator))
              (ds-scan-exp (lambda-exp->body rator) scope)
              (for-each (lambda (x) (ds-scan-exp x scope)) rands))]
           [(ilambda-exp? rator) (ds-die "operator ilambda unsupported")]
           [else
            (ds-scan-exp rator scope)
            (for-each (lambda (x) (ds-scan-exp x scope)) rands)])]
    [lambda-exp (ids body) (ds-scan-lambda e ids #f body #f scope)]
    [ilambda-exp (ids id body) (ds-scan-lambda e ids id body #f scope)]
    [lambda*-exp (arities lambdas) (ds-die "lambda* unsupported")]
    [fix-exp (ids lams body)
     (let ([fr (car scope)])
       (for-each (lambda (id) (frame-add! fr id)) ids)
       (for-each (lambda (lam) (ds-scan-exp lam scope)) lams)
       (ds-scan-exp body scope))]
    [letcc-exp (id body)
     (frame-add! (car scope) id)
     (set! *ds-need-throw* #t)
     (ds-scan-exp body scope)]
    [withcc-exp (cont-exp exp)
     (ds-scan-exp cont-exp scope)
     (ds-scan-exp exp scope)]
    [once-exp (id exp) (ds-scan-exp exp scope)]
    [else (ds-die "scan: unsupported exp" (unparse-core e))]))

(define (ds-scan-ref id scope assign?)
  (cond [(global-id? id) (global-idx id)]
        [else
         (let loop ([sc scope])
           (cond [(null? sc) (ds-die "unbound variable" (id->uname id))]
                 [(assq id (frame-bindings (car sc)))
                  => (lambda (pr)
                       (unless (eq? (car sc) (car scope))
                         (bnd-captured-set! (cdr pr) #t))
                       (when assign? (bnd-assigned-set! (cdr pr) #t)))]
                 [else (loop (cdr sc))]))]))

(define (ds-scan-lambda node ids rest body gid scope)
  (let* ([p (make-proc #f ids rest body gid)]
         [fr (make-frame p)])
    (proc-frame-set! p fr)
    (proc-def-frames-set! p scope)
    (ds-register-proc! p node gid)
    (for-each (lambda (id) (frame-add! fr id)) ids)
    (when rest (frame-add! fr rest))
    (ds-scan-exp body (cons fr scope))
    p))

(define (ds-finalize-frame! fr)
  ;; decide binding kinds; assign envvec cell indices
  (let ([cells 0])
    (for-each
     (lambda (pr)
       (let ([b (cdr pr)])
         (cond [(and (bnd-assigned? b) (bnd-captured? b))
                (bnd-kind-set! b 'boxcell) (set! cells (+ cells 1))
                (bnd-idx-set! b cells)]
               [(bnd-captured? b)
                (bnd-kind-set! b 'cell) (set! cells (+ cells 1))
                (bnd-idx-set! b cells)]
               [(bnd-assigned? b) (bnd-kind-set! b 'boxslot)]
               [else (bnd-kind-set! b 'slot)])))
     (frame-bindings fr))
    (frame-cells-set! fr (> cells 0))
    cells))

;;; collect lambdas of a body in scan (pre-order) creation order
(define (ds-body-lambdas e)
  (variant-case e
    [lambda-exp (ids body) (cons e (ds-body-lambdas body))]
    [ilambda-exp (ids id body) (cons e (ds-body-lambdas body))]
    [if-exp (test-exp then-exp else-exp)
     (append (ds-body-lambdas test-exp) (ds-body-lambdas then-exp)
             (ds-body-lambdas else-exp))]
    [primapp-exp (effect prim rands) (apply append (map ds-body-lambdas rands))]
    [app-exp (rator rands)
     (append (ds-body-lambdas rator) (apply append (map ds-body-lambdas rands)))]
    [varassign-exp (id exp) (ds-body-lambdas exp)]
    [fix-exp (ids lams body)
     (append (apply append (map ds-body-lambdas lams)) (ds-body-lambdas body))]
    [letcc-exp (id body) (ds-body-lambdas body)]
    [withcc-exp (cont-exp exp)
     (append (ds-body-lambdas cont-exp) (ds-body-lambdas exp))]
    [once-exp (id exp) (ds-body-lambdas exp)]
    [else '()]))

;;; ---------------------------------------------------------------
;;; literals: Skint rds blob + $scmrt:deserialize_all (as in the
;;; CPS converter); non-immediates get literal global cells
;;; ---------------------------------------------------------------

(define *ds-lit-inits* '())   ; (idx datum offset), idx = gstore idx
(define *ds-lit-blob* "")

;; The compiled program's fixnum geometry.  Each is spelled as an
;; expression because a towerless host's READER rejects an exact literal
;; outside its own 30-bit fixnum range, which is exactly where the last
;; two sit; -2^29 is representable there but unreadable, because the
;; reader takes the magnitude first.  The arithmetic over them stays
;; whole either way -- exact on a tower host, a whole flonum on a
;; towerless one -- and marv-integer-string spells the result without a
;; decimal point in both cases. [cco]
(define *ds-fx-least* (- -536870911 1))             ; -2^29
(define *ds-fx-greatest* 536870911)                 ;  2^29-1
(define *ds-fx-wrap* (* 1024 1048576))              ;  2^30
(define *ds-word-mask* (- (* 4 *ds-fx-wrap*) 1))    ;  2^32-1

(define (datum-immediate? d)
  (cond [(and (integer? d) (exact? d))
         (<= *ds-fx-least* d *ds-fx-greatest*)]
        [(number? d) (inexact? d)]
        [(or (boolean? d) (null? d) (char? d)) #t]
        [else #f]))

(define (hex-pad n w)
  (let ([h (number->string n 16)])
    (string-append (make-string (max 0 (- w (string-length h))) #\0) h)))

(define (rds-char cp port)
  (cond [(and (>= cp 33) (<= cp 126) (not (= cp 37)))
         (write-char (integer->char cp) port)]
        [(<= cp 255) (display "%" port) (display (hex-pad cp 2) port)]
        [(<= cp 65535) (display "%u" port) (display (hex-pad cp 4) port)]
        [else (display "%U" port) (display (hex-pad cp 8) port)]))

(define (utf8-bytes cp)
  (cond [(< cp 128) (list cp)]
        [(< cp 2048) (list (+ 192 (quotient cp 64)) (+ 128 (modulo cp 64)))]
        [(< cp 65536)
         (list (+ 224 (quotient cp 4096))
               (+ 128 (modulo (quotient cp 64) 64))
               (+ 128 (modulo cp 64)))]
        [else
         (list (+ 240 (quotient cp 262144))
               (+ 128 (modulo (quotient cp 4096) 64))
               (+ 128 (modulo (quotient cp 64) 64))
               (+ 128 (modulo cp 64)))]))

(define (datum->rds d port)
  (cond
   [(eq? d #f) (display "f" port)]
   [(eq? d #t) (display "t" port)]
   [(null? d) (display "n" port)]
   [(char? d) (display "c" port) (rds-char (char->integer d) port)]
   [(and (integer? d) (exact? d)
         (<= *ds-fx-least* d *ds-fx-greatest*))
    (display "i" port) (display d port)]
   ;; A flonum serializes as itself.  Anything else numeric -- an exact
   ;; integer outside the fixnum range, a ratio, a complex -- has no
   ;; compiled-runtime representation, and (inexact d) used to convert it
   ;; SILENTLY: the chudnovsky and pi benchmarks compiled and then ran
   ;; forever on flonums standing in for bignums.  A literal the runtime
   ;; cannot hold is an error. [cco]
   [(and (number? d) (inexact? d))
    (display "j" port) (display (number->string d) port)]
   [(number? d)
    (ds-die "this number has no compiled-runtime representation" d)]
   [(string? d)
    (display "s" port) (display (string-length d) port) (display ":" port)
    (for-each (lambda (ch) (rds-char (char->integer ch) port))
              (string->list d))]
   [(symbol? d)
    (let ([bytes (apply append
                        (map (lambda (ch) (utf8-bytes (char->integer ch)))
                             (string->list (symbol->string d))))])
      (display "y" port) (display (length bytes) port) (display ":" port)
      (for-each (lambda (b) (rds-char b port)) bytes))]
   [(pair? d)
    (if (list? d)
        (begin
          (display "l" port) (display (length d) port) (display ":" port)
          (for-each (lambda (e) (datum->rds e port) (display ";" port)) d))
        (begin
          (display "p" port)
          (datum->rds (car d) port) (display ";" port)
          (datum->rds (cdr d) port) (display ";" port)))]
   [(vector? d)
    (display "v" port) (display (vector-length d) port) (display ":" port)
    (for-each (lambda (e) (datum->rds e port) (display ";" port))
              (vector->list d))]
   [(bytevector? d)
    (display "b" port) (display (bytevector-length d) port)
    (display ":" port)
    (for-each (lambda (i) (display (hex-pad (bytevector-u8-ref d i) 2) port))
              (ds-iota (bytevector-length d)))]
   [else (ds-die "cannot serialize literal datum" d)]))

(define (datum->rds-string d)
  (let ([port (open-output-string)])
    (datum->rds d port)
    (get-output-string port)))

(define (ds-collect-body-literals! forms)
  (for-each ds-collect-literals! forms))

(define *ds-lit-found* '())

(define (ds-collect-literals! exp)
  (let ([found *ds-lit-found*])
    (let walk ([e exp])
      (variant-case e
        [quote-exp (obj)
         (unless (datum-immediate? obj)
           (unless (let loop ([xs found])
                     (cond [(null? xs) #f]
                           [(equal? (car xs) obj) #t]
                           [else (loop (cdr xs))]))
             (set! found (cons obj found))))]
        [varassign-exp (id exp) (walk exp)]
        [if-exp (test-exp then-exp else-exp)
         (walk test-exp) (walk then-exp) (walk else-exp)]
        [primapp-exp (effect prim rands) (for-each walk rands)]
        [app-exp (rator rands) (walk rator) (for-each walk rands)]
        [lambda-exp (ids body) (walk body)]
        [ilambda-exp (ids id body) (walk body)]
        [fix-exp (ids lams body) (for-each walk lams) (walk body)]
        [letcc-exp (id body) (walk body)]
        [withcc-exp (cont-exp exp) (walk cont-exp) (walk exp)]
        [once-exp (id exp) (walk exp)]
        [else #f]))
    (let ([port (open-output-string)] [off 0])
      (set! *ds-lit-found* found)
      (set! *ds-lit-inits*
            (map (lambda (d)
                   (let ([s (datum->rds-string d)])
                     (display s port)
                     (let ([o off])
                       (set! off (+ off (string-length s)))
                       (list (global-idx (ds-name "litq#" o))
                             d o))))
                 (reverse found)))
      (set! *ds-lit-blob* (get-output-string port)))))

;; Static-area words a literal will occupy once rds_sexp has
;; deserialized it.  One entry per shape the deserializer allocates,
;; and the counts are its own: static_block(hdr, n) takes n+1 words, so
;; a pair is 3, a vector of n is n+1, and a string or bytevector is the
;; 2-word native header (its payload is malloc'd, not static).  A
;; symbol is interned in a malloc'd table and an immediate is no
;; object at all, so both are free.  A proper list of n allocates n
;; pairs; an improper one is the 'p' case, one pair at a time. [cco]
(define (ds-static-words d)
  (cond
   [(or (boolean? d) (null? d) (char? d) (number? d) (symbol? d)) 0]
   [(string? d) 2]
   [(bytevector? d) 2]
   [(pair? d)
    (if (list? d)
        (+ (* 3 (length d))
           (apply + (map ds-static-words d)))
        (+ 3 (ds-static-words (car d)) (ds-static-words (cdr d))))]
   [(vector? d)
    (+ 1 (vector-length d)
       (apply + (map ds-static-words (vector->list d))))]
   [else 0]))

;; What to ask init for: every literal, plus the runtime's own immortal
;; objects.  Its fixed ones are small and known -- three standard ports
;; at 2 words each, the 3-cell port-slot block, the values tag -- but
;; the command line is one string and one pair per argument, so that
;; part is a per-argv allowance rather than a count.  The slack also
;; covers global_loc, whose boxes are 2 words and which the compiled
;; module reaches only through the name-lookup path. [cco]
(define (ds-static-words-needed)
  (+ (apply + (map (lambda (e) (ds-static-words (cadr e))) *ds-lit-inits*))
     (* 2 (length *ds-globals*))     ; a global_loc box each, if used
     4096))                          ; ports, values tag, ~800 argv entries

(define (ds-lit-entry d)
  (let loop ([xs *ds-lit-inits*])
    (cond [(null? xs) #f]
          [(equal? (cadr (car xs)) d) (car xs)]
          [else (loop (cdr xs))])))

(define (escape-blob s)
  (let ([port (open-output-string)])
    (for-each
     (lambda (ch)
       (let ([cp (char->integer ch)])
         (if (and (>= cp 32) (<= cp 126) (not (= cp 34)) (not (= cp 92)))
             (write-char ch port)
             (begin (display "\\x" port) (display (hex-pad cp 2) port)
                    (display ";" port)))))
     (string->list s))
    (get-output-string port)))

(define (ds-littab-string)
  (let ([port (open-output-string)])
    (for-each
     (lambda (en)
       (for-each
        (lambda (v)
          (write-char (integer->char (modulo v 256)) port)
          (write-char (integer->char (modulo (quotient v 256) 256)) port)
          (write-char (integer->char (modulo (quotient v 65536) 256)) port)
          (write-char (integer->char (quotient v 16777216)) port))
        (list (* 8 (car en)) (caddr en))))
     *ds-lit-inits*)
    (get-output-string port)))

;; The $littab startup table is bytes, so it travels as a bytevector
;; and the writer spells it #u8(...).  It must not be a MARV string: an
;; \xNN; escape in one of those is a Unicode code point, which the cc
;; emitter UTF-8-encodes, so every byte >= 128 would expand and shift
;; the rest of the table.  [cco]
(define (ds-littab-bytes s)
  (let* ([cs (string->list s)]
         [bv (make-bytevector (length cs) 0)])
    (let loop ([cs cs] [i 0])
      (cond [(null? cs) bv]
            [else (bytevector-u8-set! bv i (char->integer (car cs)))
                  (loop (cdr cs) (+ i 1))]))))

;; immediate datum -> the MARV form for that object
(define (ds-immediate-expr d)
  (cond [(eq? d #f) (i64c 0)]
        [(eq? d #t) (i64c 1)]
        [(null? d) (i64c 13)]
        [(char? d) (i64c (+ 9 (* 256 (char->integer d))))]
        [(and (integer? d) (exact? d))
         ;; fixnum object: low 32 bits hold (payload<<2)|3 with a
         ;; 30-bit two's-complement payload, high 32 bits zero
         ;; (NaN boxing).  A negative d must wrap into the unsigned
         ;; 32-bit pattern; the signed value (e.g. -5 for -2) would
         ;; become the i64 0xFFFF...FB, which is not a fixnum.
         (i64c (+ 3 (* 4 (modulo d *ds-fx-wrap*))))]
        [(number? d) (list '$scmn:flonum_obj
                           (list 'f64.const (inexact d)))]
        [else (ds-die "not immediate" d)]))

;;; ---------------------------------------------------------------
;;; prim tables (name -> scmi macro / instruction), staged on $ac
;;; with one MARV stack operand for binaries -- as in cups-smarv
;;; ---------------------------------------------------------------

;; Whether the GENERIC arithmetic operators can allocate.
;;
;; In this runtime they cannot, and scmi.mvh says so of its own slow
;; paths: "none of them allocates (every value is an immediate)".
;; Their signatures prove it rather than promise it -- add_slow and
;; friends are (param $a i64) (param $b i64) (result i64), with no $r
;; and no $hp, so they have no heap to allocate in.  What makes it true
;; is the NaN-boxed representation in scmn/scmi, where a flonum is the
;; complement of an f64 bit pattern and so an immediate like a fixnum:
;; + has nothing to cons.  That is a property of the representation and
;; not of any backend, so it holds on cc, wasm and rv64g alike.
;;
;; Every (+ - * / quotient floor-quotient expt) site used to reserve 2
;; words regardless, and a reserve is not free: it emits a heap-limit
;; compare and branch, refreshes $sp, and -- because a reserve is a
;; point where a collection may occur -- it ends the region within
;; which a value may be cached in a MARV local rather than reloaded
;; from its slot.  Arithmetic is frequent, so those regions were short.
;;
;; WHEN FULL TOWER SUPPORT ARRIVES this stops holding for the generic
;; operators, which will cons bignums and ratios.  Set this to #t and
;; they reserve again.  The fl and fx operators never allocate even
;; then, which is why they are not in the list the flag guards. [cco]
(define *ds-generic-arith-allocates* #f)

(define *ds-prim-bin*
  '((+ . add) (- . sub) (* . mul) (/ . div)
    ;; fx* on fixnum operands behave exactly like the generic ops
    (fx+ . iadd) (fx- . isub) (fx* . imul) (fx=? . ieq)
    (fx<? . ilt) (fx<=? . ile) (fx>? . igt) (fx>=? . ige)
    (fx!=? . ine) (fxquotient . iquo) (fxremainder . irem)
    (fxmodquo . fquo) (fxmin . imin) (fxmax . imax)
    (fl=? . jeq) (fl<? . jlt) (fl>? . jgt) (fl<=? . jle)
    (fl>=? . jge) (fl!=? . jne) (flexpt . expt) (flldexp . jldexp)
    (flquotient . jquo) (flmodquo . fquo) (flmodulo . fmod)
    (fxgcd . fxgcd) (flgcd . flgcd) (fxexpt . fxexpt)
    (fxeqv . ieqv)
    (< . lt) (<= . le) (> . gt) (>= . ge) (= . eq)
    (quotient . quo) (remainder . rem) (modulo . mod)
    ;; fxmodulo on fixnums is exactly the floored $mod fast path;
    ;; immediate-hash is Skint integrable 591 (its hshim instruction)
    (fxmodulo . mod) (immediate-hash . immhash)
    ;; fixnum bitwise / shifts over the $scmi:i* macros; the generic
    ;; bitwise-* names coincide with them on DS's fixnum-only integers
    (fxand . iand) (fxior . iior) (fxxor . ixor)
    (fxsll . iasl) (fxsra . iasr) (fxsrl . ilsr)
    (bitwise-and . iand) (bitwise-ior . iior) (bitwise-xor . ixor)
    (arithmetic-shift . ash)
    (truncate-quotient . quo) (truncate-remainder . rem)
    (floor-quotient . fquo) (floor-remainder . mod)
    (!= . ne) (string=? . streq)
    (flmin . jmin) (flmax . jmax)
    (min . gmin) (max . gmax) (expt . expt)
    (eq? . isq) (eqv? . isv) (equal? . ise) (boolean=? . isq)
    (char=? . ceq) (char<? . clt) (char>? . cgt)
    (char<=? . cle) (char>=? . cge)
    (fl+ . jadd) (fl- . jsub) (fl* . jmul) (fl/ . jdiv)
    (vector-ref . vget) (string-ref . sget)
    (list-ref . lref) (list-tail . ltail)
    (bytevector-u8-ref . bvget) (numvector-ref . nvget)
    (numvector? . nvp) (list->numvector . lnv)))

(define *ds-prim-un*
  '((not . not) (car . car) (cdr . cdr) (length . llen)
    (null? . nullp) (pair? . pairp) (boolean? . boolp)
    (char? . charp) (string? . strp) (vector? . vecp)
    (procedure? . procp) (integer? . intp)
    (exact? . exactp) (inexact? . inxp)
    (char-upper-case? . cupp) (char-lower-case? . clowp)
    (sin . gsin) (cos . gcos) (tan . gtan)
    (asin . gasin) (acos . gacos) (exp . gexp) (sqrt . gsqrt)
    (input-port? . inportp) (output-port? . outportp)
    (box? . boxp) (record? . recp) (symbol? . symp) (eof-object? . eofp)
    (number? . nump) (exact-integer? . xintp) (list? . listp)
    (fixnum? . fixp) (flonum? . flop)
    (fxzero? . izerop) (fxpositive? . iposp) (fxnegative? . inegp)
    (fxodd? . ioddp) (fxeven? . ievnp) (fxneg . ineg) (fxabs . iabs)
    (fxlength . ilen) (fxbit-count . ibcnt)
    (fxnot . inot) (bitwise-not . inot)
    (integer-length . ilen) (bit-count . ibcnt)
    (char-alphabetic? . calphap) (char-numeric? . cnump)
    (char-whitespace? . cspacep) (digit-value . cdigv)
    (char->integer . ctoi) (integer->char . itoc)
    (char-upcase . cupc) (char-downcase . cdnc)
    (char-foldcase . cflc)
    (string-length . slen) (bytevector? . bvp)
    (bytevector-length . bvlen) (string->symbol . strsym)
    (numvector-length . nvlen)
    (flneg . jneg) (flsqrt . jsqrt)
    (flsin . gsin) (flcos . gcos) (fltan . gtan)
    (flasin . gasin) (flacos . gacos) (flexp . gexp) (fllog10 . glog10)
    (flinteger? . jintp)
    (flsinh . gsinh) (flcosh . gcosh) (fltanh . gtanh)
    (flabs . jabs) (flzero? . jzerop) (flpositive? . jposp)
    (flnegative? . jnegp)
    (flfloor . jfloor) (flceiling . jceil) (fltruncate . jtrunc)
    (flround . jround) (flsgn . jsgn) (flsignbit . jsignbit)
    (flnan? . jnanp) (flinfinite? . jinfp) (flfinite? . jfinp)
    (flsign-bit . jsignbit)
    (abs . gabs) (neg . gneg) (truncate . trunc) (exact . exact)
    (inexact . inexact) (zero? . gzerop) (positive? . gposp)
    (negative? . gnegp)
    (fixnum->flonum . itoj) (flonum->fixnum . jtoi)
    (unbox . unbox)))

(define (ds-cxr-ops name)
  (let ([s (symbol->string name)])
    (and (> (string-length s) 2)
         (char=? (string-ref s 0) #\c)
         (char=? (string-ref s (- (string-length s) 1)) #\r)
         ;; letters are collected outermost-first ("cadr" -> (car cdr));
         ;; application must run innermost-first, i.e. in the order
         ;; the consing produced -- no reverse.
         (let loop ([i 1] [ops '()])
           (cond [(= i (- (string-length s) 1)) ops]
                 [(char=? (string-ref s i) #\a) (loop (+ i 1) (cons 'car ops))]
                 [(char=? (string-ref s i) #\d) (loop (+ i 1) (cons 'cdr ops))]
                 [else #f])))))

;;; ---------------------------------------------------------------
;;; code generation: instructions, transfers, expressions
;;; ---------------------------------------------------------------

(define *ds-units* '())         ; (name lines maxlocal startdepth refs)
(define *ds-unit-seq* 0)
(define *ds-cur-name* #f)
(define *ds-cur-lines* '())
(define *ds-cur-maxlocal* 0)
(define *ds-cur-startdepth* 0)
(define *ds-cur-proc* #f)
;; per-proc compile state
(define *ds-slot* 0)            ; next free slot index (params at 0..)
(define *ds-scope* '())         ; layers: (bindings-alist . frame)
(define *ds-cost* 0)            ; ops accumulated in current unit

(define (ds-e line)
  ;; One peephole, applied as instructions are appended: a (load k)
  ;; whose IMMEDIATELY preceding instruction is (store k (get ac)) is
  ;; dropped, because $ac still holds exactly what that store put in
  ;; slot k.  The store stays -- the slot has to hold the value for
  ;; later reads and for the collector -- only the read-back goes.
  ;;
  ;; "Immediately preceding" is what makes this safe rather than a
  ;; dataflow question: nothing can have run in between, so no branch,
  ;; no transfer and no collection can have invalidated $ac.  It also
  ;; cannot reach across a ds-capture, which rebinds *ds-cur-lines* to
  ;; '() -- a captured arm's first instruction simply finds no
  ;; predecessor and the pattern does not fire.
  ;;
  ;; The pair arises wherever a staged value is used at once, and
  ;; ds-prim-args now makes it common: with the own-slot
  ;; pass-through, a second operand that is already a variable emits
  ;; nothing, so the binary-prim path stores operand one and reads it
  ;; straight back. [cco]
  (if (and (pair? line)
           (eq? (car line) 'load)
           (pair? *ds-cur-lines*)
           (equal? (car *ds-cur-lines*)
                   (list 'store (cadr line) (list 'get 'ac))))
      #f
      (begin
        (set! *ds-cur-lines* (cons line *ds-cur-lines*))
        (set! *ds-cost* (+ *ds-cost* 1)))))

;; Generate into a fresh instruction list and answer it, so an arm
;; can be an operand of the form that owns it.  Safe only where the
;; arm cannot fork the unit, which is exactly where an inline if is
;; chosen; *ds-cost* keeps accumulating across the capture, because
;; the balance heuristic counts the whole unit. [cco]
(define *ds-capturing* 0)

(define (ds-capture thunk)
  (let ([saved *ds-cur-lines*])
    (set! *ds-cur-lines* '())
    (set! *ds-capturing* (+ *ds-capturing* 1))
    (thunk)
    (set! *ds-capturing* (- *ds-capturing* 1))
    (let ([got (reverse *ds-cur-lines*)])
      (set! *ds-cur-lines* saved)
      got)))

(define (ds-new-unit-name)
  (set! *ds-unit-seq* (+ *ds-unit-seq* 1))
  (ds-name "$ds$u" *ds-unit-seq*))

(define (ds-begin-unit name startdepth)
  (set! *ds-cur-name* name)
  (set! *ds-cur-lines* '())
  (set! *ds-cur-maxlocal* 0)
  (set! *ds-cur-startdepth* startdepth)
  (set! *ds-cost* 0)
  (ds-e (list 'sp-set startdepth)))

(define (ds-close-unit!)
  ;; A fork inside a capture would store the captured delta as the
  ;; whole unit and then restore the outer list over it, losing the
  ;; arm without a trace.  Every caller that captures has a reason it
  ;; cannot fork; this says so loudly rather than relying on that
  ;; reasoning staying true. [cco]
  (unless (= *ds-capturing* 0)
    (ds-die "unit closed inside a capture" *ds-cur-name*))
  (when *ds-cur-name*
    ;; A unit's payload is its instruction list.  Nothing between here
    ;; and the writer looks at characters, so the paren-counting
    ;; balance check that used to live here has no job: a tree cannot
    ;; be unbalanced. [cco]
    (set! *ds-units*
          (cons (list *ds-cur-name*
                      (reverse *ds-cur-lines*)
                      *ds-cur-maxlocal* *ds-cur-startdepth*)
                *ds-units*))
    (set! *ds-cur-name* #f)))

(define (ds-fresh-temp)
  (let ([t *ds-cur-maxlocal*])
    (set! *ds-cur-maxlocal* (+ t 1))
    (ds-name "$t" t)))

(define (ds-alloc-slot!)
  (let ([s *ds-slot*])
    (set! *ds-slot* (+ s 1))
    (when (> *ds-slot* (proc-maxslot *ds-cur-proc*))
      (proc-maxslot-set! *ds-cur-proc* *ds-slot*))
    s))

(define (ds-scope-lookup scope id)
  ;; -> (binding depth): depth = number of cells-frames whose
  ;; envvec must be walked from the current env to reach the frame
  ;; holding the binding (0 = current frame's envvec context)
  (let loop ([sc scope])
    (cond [(null? sc) #f]
          [(assq id (frame-bindings (car sc)))
           => (lambda (pr)
                (let cnt ([xs scope] [n 0])
                  (cond [(eq? (car xs) (car sc))
                         (list (cdr pr) n)]
                        [else
                         (cnt (cdr xs)
                              (+ n (if (frame-cells? (car xs)) 1 0)))])))]
          [else (loop (cdr sc))])))

(define (ds-cur-frame) (car *ds-scope*))

;; value of a variable as a pure load expression, or #f if the
;; variable needs instructions (box deref) -> $ac holds it
(define (ds-var-expr scope id)
  (cond [(global-id? id) (global-ref-expr id)]
        [else
         (let ([lk (ds-scope-lookup scope id)])
           (unless lk (ds-die "unbound variable" (id->uname id)))
           (let ([b (car lk)] [depth (cadr lk)])
             (case (bnd-kind b)
               [(slot boxslot) (slot-load (bnd-idx b))]
               [(cell boxcell)
                (env-cell (env-walk-expr depth) (bnd-idx b))]
               [else (ds-die "var kind" (bnd-kind b))])))]))

(define (ds-var-boxed? scope id)
  (and (not (global-id? id))
       (let ([lk (ds-scope-lookup scope id)])
         (and lk (memq (bnd-kind (car lk)) '(boxslot boxcell))))))

;; generate exp with result in $ac (fallthrough mode)
(define (ds-gen-expr e) (ds-gen-value e *ds-scope*))

;; an operand expression for prim staging: cheap loads inline,
;; anything else is generated into a fresh slot first
(define (ds-operand-expr e scope)
  (cond [(var-exp? e)
         (let ([id (var-exp->id e)])
           (if (ds-var-boxed? scope id)
               (ds-eval-to-slot e scope)
               (ds-var-expr scope id)))]
        [(quote-exp? e)
         (let ([d (quote-exp->obj e)])
           (if (datum-immediate? d)
               (ds-immediate-expr d)
               (ds-eval-to-slot e scope)))]
        [else (ds-eval-to-slot e scope)]))

(define (ds-eval-to-slot e scope)
  (let ([s (ds-alloc-slot!)] [saved *ds-scope*])
    (set! *ds-scope* scope)
    (ds-gen-expr e)
    (set! *ds-scope* saved)
    (ds-e (list 'store s (list 'get 'ac)))
    (slot-load s)))

(define (ds-transfer target)
  ;; return_call the named instruction with current params, ac=value
  (ds-e (list 'chain target)))

(define (ds-transfer-args target env-expr nargs spv fpv)
  (ds-e (list 'jump target env-expr nargs spv fpv)))

(define (ds-transfer-indirect code-expr env-expr nargs spv fpv)
  (ds-e (list 'jump-ind code-expr env-expr nargs spv fpv)))

(define (ds-transfer-return)
  ;; pop the frame through the header slots
  (let ([te (ds-fresh-temp)] [tf (ds-fresh-temp)])
    (ds-e (list 'load-fn -3))
    (ds-e (list 'load-to te -4))
    (ds-e (list 'load-to tf -2))
    (ds-e (list 'ret te tf))))

;; When compiling a rest-arg procedure, the arg slots extend to
;; fp+nargs, a dynamic bound the static slot count cannot express.
;; While *ds-sp-dynamic* is set (an i64 expression for the frame's
;; dynamic extent), every $sp refresh takes the max of the static
;; bound and the dynamic one, so collections at proc entry (envvec
;; build, rest-list build) scan all arg slots.  Over-covering is
;; safe: slots above the static extent hold args or frame-birth
;; fill, both valid values.
(define *ds-sp-dynamic* #f)

(define (ds-refresh-sp!)
  ;; roots are $r[0..$sp): $sp must cover every live slot of the
  ;; current frame, including slots allocated mid-unit
  (if *ds-sp-dynamic*
      ;; a rest-arg frame has no static top, so $sp is the larger of
      ;; the static slot count and the incoming argument count
      (ds-e (list 'set "$sp"
                  (list 'marv
                        (let ([stat (list 'i64.add '(local.get $fp)
                                          (i64c *ds-slot*))])
                          (list 'if '(result i64)
                                (list 'i64.ge_u stat *ds-sp-dynamic*)
                                (list 'then stat)
                                (list 'else *ds-sp-dynamic*))))))
      (ds-e (list 'sp-set *ds-slot*))))

(define (ds-reserve words)
  ;; GC if the heap cannot cover words more words; roots are
  ;; $r[0..$sp) and the gstore range.  $ac must be dead or spilled.
  (ds-refresh-sp!)
  (ds-e (list 'reserve (list 'imm-i32 words)
              (list 'imm-i32 (* 8 words))))
  (ds-e '(env-refresh)))

;; Reserve room for a list built from a sequence: three words per
;; element.  The element count is an i32 form the runtime computes,
;; which is why this cannot be the constant ds-reserve. [cco]
(define (ds-reserve-by-length n-i32-form)
  (ds-reserve-expr (list 'i32.mul n-i32-form '(i32.const 3))
                   (list 'i32.mul n-i32-form '(i32.const 24))))

(define (ds-reserve-expr words-i32-expr bytes-i32-expr)
  (ds-refresh-sp!)
  (ds-e (list 'reserve (list 'marv words-i32-expr)
              (list 'marv bytes-i32-expr)))
  (ds-e '(env-refresh)))

(define (ds-emit-arity-error expected)
  (ds-e (list 'arity-error expected))
  (ds-e '(halt)))

(define (ds-check-proc! expr)
  ;; indirect calls jump through word 0 of the callee value;
  ;; anything that is not a closure (microtag 2) must report and
  ;; exit(1) (Skint behaviour) instead of crashing.  Emitted
  ;; after operator and operands are evaluated, matching
  ;; Skint's check timing.  See $scmi:ckproc.
  (ds-e (list 'rt1 "ckproc" (list 'marv expr))))

;;; ---------------------------------------------------------------
;;; expression generation (fallthrough: result in $ac)
;;; ---------------------------------------------------------------

(define (ds-gen-value e scope)
  (set! *ds-cost* (+ *ds-cost* (exp-cost e)))
  (variant-case e
    [quote-exp (obj)
     (cond [(datum-immediate? obj)
            (ds-e (list 'set "$ac" (list 'marv (ds-immediate-expr obj))))]
           [(ds-lit-entry obj)
            => (lambda (en)
                 (ds-e (list 'set "$ac"
                             (list 'marv
                                   (list 'i64.load 'offset (* 8 (car en))
                                         '(ref.data $gstore))))))]
           [else (ds-die "uncollected literal")])]
    [var-exp (id)
     (cond [(global-id? id)
            (ds-e (list 'set "$ac" (list 'marv (global-ref-expr id))))]
           [(ds-var-boxed? scope id)
            (ds-e (list 'set "$ac" (list 'marv (ds-var-expr scope id))))
            (ds-e (list 'rt 'unbox))]
           [else
            (ds-e (list 'set "$ac" (list 'marv (ds-var-expr scope id))))])]
    [varassign-exp (id exp)
     (ds-gen-value exp scope)
     (cond [(global-id? id)
            (ds-e (list 'global-store id '(get ac)))]
           [else
            (let ([lk (ds-scope-lookup scope id)])
              (unless lk (ds-die "assign unbound" (id->uname id)))
              (let ([b (car lk)] [depth (cadr lk)])
                (case (bnd-kind b)
                  [(slot) (ds-e (list 'store (bnd-idx b) (list 'get 'ac)))]
                  [(boxslot boxcell)
                   (let ([t (ds-fresh-temp)])
                     (ds-e (list 'set t
                                 (case (bnd-kind b)
                                   [(boxslot) (list 'slot (bnd-idx b))]
                                   [else (list 'marv
                                               (env-cell (env-walk-expr depth)
                                                         (bnd-idx b)))])))
                     (ds-e (list 'box-set (list 'get t) '(get ac))))]
                  [else (ds-die "assign to cell (front should box)")])))])
     (ds-e (list 'const 'ac 21))]
    [varaddress-exp (id)
     ;; the value of (set& x) is x's box itself; scan forced the
     ;; binding into a box slot/cell, whose contents the box is
     (cond [(global-id? id)
            (ds-die "varaddress of a global is unsupported"
                    (id->uname id))]
           [else
            (let ([lk (ds-scope-lookup scope id)])
              (unless lk (ds-die "varaddress unbound" (id->uname id)))
              (unless (memq (bnd-kind (car lk)) '(boxslot boxcell))
                (ds-die "varaddress of unboxed variable"
                        (id->uname id)))
              (ds-e (list 'set "$ac"
                          (list 'marv (ds-var-expr scope id)))))])]
    [if-exp (test-exp then-exp else-exp)
     (ds-gen-if test-exp then-exp else-exp scope #f)]
    [primapp-exp (effect prim rands) (ds-gen-primapp prim rands scope)]
    [app-exp (rator rands) (ds-gen-app rator rands scope #f)]
    [lambda-exp (ids body)
     (ds-gen-closure (ds-proc-of-node e) scope)]
    [ilambda-exp (ids id body)
     (ds-gen-closure (ds-proc-of-node e) scope)]
    [fix-exp (ids lams body) (ds-gen-fix ids lams body scope #f)]
    [letcc-exp (id body) (ds-gen-letcc id body scope #f)]
    [withcc-exp (cont-exp exp) (ds-gen-withcc cont-exp exp scope #f)]
    [once-exp (id exp) (ds-gen-value exp scope)]
    [else (ds-die "gen: unsupported exp" (unparse-core e))]))

;; statement: value discarded; balance cuts happen here
(define (ds-gen-stmt e scope)
  (ds-gen-value e scope)
  (ds-maybe-cut))

(define (ds-maybe-cut)
  (when (and (= *ds-inline* 0) (>= *ds-cost* *ds-balance*))
    (let ([next (ds-new-unit-name)])
      (ds-transfer next)
      (ds-close-unit!)
      (ds-begin-unit next *ds-slot*))))

;; expression sequence; last form in tail? position of the unit run
(define (ds-gen-seq forms scope tail?)
  (cond [(null? forms) (ds-e (list 'const 'ac 21))
                       (when tail? (ds-transfer-return))]
        [(null? (cdr forms))
         (if tail? (ds-gen-tail (car forms) scope)
             (ds-gen-value (car forms) scope))]
        [else
         (ds-gen-stmt (car forms) scope)
         (ds-gen-seq (cdr forms) scope tail?)]))

(define (ds-gen-tail e scope)
  (variant-case e
    [if-exp (test-exp then-exp else-exp)
     (ds-gen-if test-exp then-exp else-exp scope #t)]
    [fix-exp (ids lams body) (ds-gen-fix ids lams body scope #t)]
    [letcc-exp (id body) (ds-gen-letcc id body scope #t)]
    [app-exp (rator rands) (ds-gen-app rator rands scope #t)]
    [withcc-exp (cont-exp exp) (ds-gen-withcc cont-exp exp scope #t)]
    [once-exp (id exp) (ds-gen-tail exp scope)]
    [else (ds-gen-value e scope) (ds-transfer-return)]))

;;; ---------------------------------------------------------------
;;; if / fix / closures / continuations
;;; ---------------------------------------------------------------

;; While an inline (single-unit) if is being emitted, *ds-inline*
;; is positive, and nothing inside its arms may fork the unit
;; (no unit-path if, no balance cut): the enclosing (if ...) form
;; stays open across the whole arm, so a nested unit fork would
;; leave the outer if unclosed and drop its closing parens into
;; the wrong unit.  Calls in the arms are still excluded by
;; exp-contains-call? at every level, so force-inlining when
;; *ds-inline* > 0 only suspends the cost heuristic, which is
;; not compositional (*ds-cost* grows as generation descends).
(define *ds-inline* 0)

(define (ds-gen-if test then else scope tail?)
  (ds-gen-value test scope)
  (cond [(and (not tail?)
              (not (exp-contains-call? then))
              (not (exp-contains-call? else))
              (or (> *ds-inline* 0)
                  (<= (+ *ds-cost* (exp-cost then) (exp-cost else))
                      *ds-balance*)))
         (set! *ds-inline* (+ *ds-inline* 1))
         (let* ([fa (ds-capture (lambda () (ds-gen-value else scope)))]
                [ta (ds-capture (lambda () (ds-gen-value then scope)))])
           (set! *ds-inline* (- *ds-inline* 1))
           (ds-e (list 'if-inline fa ta)))]
        [else
         (let ([ua (ds-new-unit-name)] [ub (ds-new-unit-name)]
               [uj (and (not tail?) (ds-new-unit-name))]
               [depth *ds-slot*])
           (ds-e (list 'fork ub ua))
           (ds-close-unit!)
           (ds-begin-unit ua depth)
           (if tail? (ds-gen-tail then scope)
               (begin (ds-gen-value then scope) (ds-transfer uj)))
           (ds-close-unit!)
           (ds-begin-unit ub depth)
           (if tail? (ds-gen-tail else scope)
               (begin (ds-gen-value else scope) (ds-transfer uj)))
           (ds-close-unit!)
           (unless tail? (ds-begin-unit uj depth)))]))

;; store $ac into the location of id (local or global)
(define (ds-store-binding scope id)
  (cond [(global-id? id) (ds-e (list 'global-store id '(get ac)))]
        [else
         (let ([lk (ds-scope-lookup scope id)])
           (unless lk (ds-die "store unbound" (id->uname id)))
           (let ([b (car lk)] [depth (cadr lk)])
             (case (bnd-kind b)
               [(slot) (ds-e (list 'store (bnd-idx b) (list 'get 'ac)))]
               [(cell)
                (ds-e (list 'cell-store (list 'marv (env-walk-expr depth))
                            (bnd-idx b) '(get ac)))]
               [(boxslot boxcell)
                (let ([t (ds-fresh-temp)])
                  (ds-e (list 'set t
                              (case (bnd-kind b)
                                [(boxslot) (list 'slot (bnd-idx b))]
                                [else (list 'marv
                                            (env-cell (env-walk-expr depth)
                                                      (bnd-idx b)))])))
                  (ds-e (list 'box-set (list 'get t) '(get ac))))]
               [else (ds-die "store kind" (bnd-kind b))])))]))

;; create a heap box holding $ac; result (box obj) in $t-local named
(define (ds-make-box!)
  ;; root the value in a slot first: the reserve may collect, and a
  ;; heap value held only in a register would go stale
  (let ([t (ds-fresh-temp)] [sv (ds-alloc-slot!)])
    (ds-e (list 'store sv (list 'get 'ac)))
    (ds-reserve 2)
    (ds-e (list 'set t '(rep hend_box)))
    (ds-e (list 'box-set (list 'get t) (list 'slot sv)))
    t))

;; bind one let/fix variable: value is in $ac; id's binding lives
;; in the frame at the top of scope
(define (ds-bind-value! scope id)
  (let ([lk (ds-scope-lookup scope id)])
    (unless lk (ds-die "bind unbound" (id->uname id)))
    (let ([b (car lk)])
      (case (bnd-kind b)
        [(slot) (bnd-idx-set! b (ds-alloc-slot!))
                (ds-e (list 'store (bnd-idx b) (list 'get 'ac)))]
        [(boxslot)
         (let ([t (ds-make-box!)])
           (bnd-idx-set! b (ds-alloc-slot!))
           (ds-e (list 'store (bnd-idx b) (list 'get t))))]
        [(cell boxcell)
         (when (eq? (bnd-kind b) 'boxcell)
           (ds-e (list 'set "$ac" (list 'get (ds-make-box!)))))
         (ds-e (list 'cell-store (list 'marv (env-walk-expr (cadr lk)))
                     (bnd-idx b) '(get ac)))]
        [else (ds-die "bind kind" (bnd-kind b))]))))

(define (ds-gen-fix ids lams body scope tail?)
  (for-each (lambda (id lam) (ds-gen-closure (ds-proc-of-node lam) scope)
              (ds-bind-value! scope id))
            ids lams)
  (ds-gen-seq (flatten-begin body) scope tail?))

(define (ds-gen-closure p scope)
  (ds-reserve 3)
  (ds-e (list 'set "$ac" '(rep hend_tagged (i32.const 2) (i32.const 2))))
  (ds-e (list 'fn-store '(ptr.add64 (ptr.null) (local.get $ac))
              (proc-entry p)))
  (ds-e (list 'cell-set '(get ac) 1 '(slot -1))))

(define *ds-cap-seq* 0)

;; side unit: emit emit! as a complete separate instruction
(define (ds-emit-side-unit! name startdepth emit!)
  (let ([sn *ds-cur-name*] [sl *ds-cur-lines*] [sm *ds-cur-maxlocal*]
        [ss *ds-cur-startdepth*] [sc *ds-cost*] [slot *ds-slot*])
    (set! *ds-cur-name* name)
    (set! *ds-cur-lines* '())
    (set! *ds-cur-maxlocal* 0)
    (set! *ds-cur-startdepth* startdepth)
    (ds-e (list 'sp-set startdepth))
    (emit!)
    (ds-close-unit!)
    (set! *ds-cur-name* sn) (set! *ds-cur-lines* sl)
    (set! *ds-cur-maxlocal* sm) (set! *ds-cur-startdepth* ss)
    (set! *ds-cost* sc) (set! *ds-slot* slot)))

(define (ds-gen-letcc id body scope tail?)
  (let ([Lc (ds-new-unit-name)] [base *ds-slot*]
        [sv (ds-alloc-slot!)] [tv (ds-alloc-slot!)]
        [ti (ds-fresh-temp)])
    (set! *ds-cap-seq* (+ *ds-cap-seq* 1))
    (let ([lp (ds-name "$ds_cap" *ds-cap-seq* "_lp")])
      ;; snapshot vec := [fp sp stack...]; live sp first
      (ds-e (list 'sp-set *ds-slot*))
      ;; (i64.const 3), not (i32.const 3): the addend of an i64.add must
      ;; be i64, as the (i64.const 2) three lines down already has it.
      ;; Only the wasm target checks, and this is the continuation
      ;; snapshot, so it was the one remaining invalid module shape --
      ;; ctak, fibc, maze, puzzle, quicksort and read0 all failed here
      ;; and nowhere else. [cco]
      (let ([words '(i32.wrap_i64 (i64.add (local.get $sp) (i64.const 3)))])
        (ds-reserve-expr words (list 'i32.shl words '(i32.const 3))))
      (ds-e (list 'alloc "$ac" "vector"
                  '(marv (i32.wrap_i64 (i64.add (local.get $sp)
                                                (i64.const 2))))))
      (ds-e (list 'cell-set '(get ac) 0 '(get fp)))
      (ds-e (list 'cell-set '(get ac) 1 '(get sp)))
      (ds-e (list 'store sv (list 'get 'ac)))
      (ds-e (list 'set ti '(imm 0)))
      ;; snapshot stack := $r[0..$sp): one bulk copy (memmove);
      ;; the MARV-level word loop this replaces is markedly
      ;; slower at these sizes, and the count is dynamic
      (ds-e (list 'marv
                  (list 'memory.copy
                        (list 'ptr.add64
                              (list 'ptr.add64 '(ptr.null) (slot-load sv))
                              '(i64.const 16))
                        '(local.get $r)
                        '(i32.wrap_i64 (i64.shl (local.get $sp)
                                               (i64.const 3))))))
      ;; continuation closure := [$ds_throw snapshot Lc]
      (ds-reserve 4)
      (ds-e (list 'set "$ac"
                  '(rep hend_tagged (i32.const 3) (i32.const 2))))
      (ds-e (list 'fn-store '(ptr.add64 (ptr.null) (local.get $ac))
                  '$ds_throw))
      ;; cell 1 is the stack snapshot
      (ds-e (list 'cell-set '(get ac) 1 (list 'slot sv)))
      ;; The resume point is cell 2 of the closure: a code pointer, so a
      ;; funcref store like the one at cell 0, just at another offset.
      ;; It has to be an fn-store rather than a (marv ...) payload
      ;; because Lc is a UNIT NAME, and ds-rename-instrs only reaches
      ;; the operand positions it knows -- a name buried in a payload
      ;; stopped being renamed, which silently cost six collapsed units
      ;; on a program using call/cc. [cco]
      (ds-e (list 'fn-store '(ptr.add64 (ptr.null) (local.get $ac))
                  Lc 16))
      (ds-e (list 'store tv (list 'get 'ac)))
      (ds-e (list 'load tv))
      ;; a letcc id is not a let/fix binding, so nothing else ever
      ;; assigns its frame slot: bnd-idx would stay at the default 0
      ;; and the store below would clobber parameter slot 0 (and
      ;; reads of the continuation as a plain value would read that
      ;; parameter).  This only matters when the continuation is
      ;; used as a value -- direct (k v) applications are rewritten
      ;; to withcc by the expander and never read the binding.
      (let ([lk (ds-scope-lookup scope id)])
        (unless lk (ds-die "letcc unbound" (id->uname id)))
        (let ([b (car lk)])
          (case (bnd-kind b)
            [(slot) (bnd-idx-set! b (ds-alloc-slot!))
                    (ds-store-binding scope id)]
            [(boxslot) (bnd-idx-set! b (ds-alloc-slot!))
                       (let ([t (ds-make-box!)])
                         (ds-e (list 'store (bnd-idx b) (list 'get t))))]
            [else (ds-store-binding scope id)])))
      (cond [tail?
             ;; continuation of a letcc in tail position = frame return
             (ds-emit-side-unit! Lc base
                                 (lambda () (ds-transfer-return)))
             (ds-gen-tail body scope)]
            [else
             ;; body value falls through into the continuation unit
             (ds-gen-value body scope)
             (ds-transfer Lc)
             (ds-close-unit!)
             (ds-begin-unit Lc base)]))))

(define (ds-gen-withcc cont val scope tail?)
  (let ([cs (ds-eval-to-slot cont scope)] [vs *ds-slot*])
    (ds-gen-value val scope)
    (ds-e (list 'store vs (list 'get 'ac)))
    ;; invoke the continuation closure like a call: $env = the
    ;; closure itself ($ds_throw reads snap/Lc from its cells),
    ;; $fp points at the value slot
    (ds-e (list 'jump '$ds_throw cs 0
                '(local.get $sp)
                (list 'i64.add '(local.get $fp) (i64c vs))))))

;;; ---------------------------------------------------------------
;;; calls (all via return_call)
;;; ---------------------------------------------------------------

(define (ds-known-target rator scope)
  ;; => proc for a statically known callee, else #f
  (cond [(var-exp? rator)
         (let ([id (var-exp->id rator)])
           (and (global-id? id)
           (not (memq id *ds-assigned-globals*))
                (cond [(assq id *ds-global-procs*) => cdr] [else #f])))]
        [else #f]))

;; A direct call to a statically known global: its argument count has to
;; match the definition.  The report names the GLOBAL rather than the
;; generated proc and gives both counts, because "arity p158 3" said
;; neither which procedure of the program was meant nor what it wanted.
;; [cco]
(define (ds-check-arity p nargs gid)
  (let ([np (length (proc-params p))]
        [who (if gid (id->uname gid) (proc-name p))])
    (cond [(proc-rest p)
           (when (< nargs np)
             (ds-die "call has too few arguments" who nargs 'needs-at-least np))]
          [(not (= nargs np))
           (ds-die "call has the wrong number of arguments" who nargs 'needs np)])))

;; stage arguments to fresh slots (evaluated left to right)
(define (ds-stage-args rands scope)
  (map (lambda (r)
         (let ([sl (ds-alloc-slot!)])
           (ds-gen-value r scope)
           (ds-e (list 'store sl (list 'get 'ac)))
           sl))
       rands))

(define (ds-slide-args! ts)
  (if (and (> (length ts) 4)
           ;; staged slots must form a consecutive run; ds-gen-value
           ;; for a complex argument can allocate slots of its own
           ;; between two staged arguments, leaving gaps -- in that
           ;; case only the per-word slide below is correct
           (let loop ([t ts] [s (car ts)])
             (or (null? t)
                 (and (= (car t) s) (loop (cdr t) (+ s 1))))))
      ;; staged args sit consecutively at $r[$fp + car(ts) .. +n);
      ;; slide the whole run down to $r[$fp .. $fp+n) with one bulk
      ;; copy (memmove semantics: ranges overlap, dst < src)
      (ds-e (list 'marv
                  (list 'memory.copy
                        '(ptr.add64 (local.get $r)
                                   (i64.shl (local.get $fp) (i64.const 3)))
                        (list 'ptr.add64 '(local.get $r)
                              (list 'i64.shl
                                    (list 'i64.add '(local.get $fp)
                                          (i64c (car ts)))
                                    '(i64.const 3)))
                        (list 'i32.const (* 8 (length ts))))))
      (let ([tt (ds-fresh-temp)])
        (for-each (lambda (sl j)
                    (ds-e (list 'load-to tt sl))
                    (ds-e (list 'store j (list 'get tt))))
                  ts (ds-iota (length ts))))))

(define (ds-sp-after n) (list (quote i64.add) (quote (local.get $fp)) (i64c n)))

;; finish a non-tail call: header + transfer, then open the
;; continuation unit; caller's *ds-slot* becomes hbase
(define (ds-finish-call! target env-expr code-expr nargs hbase)
  (let ([k (ds-new-unit-name)])
    (ds-e (list 'store hbase (list 'get 'env)))
    (ds-e (list 'fn-store (slot-addr (+ hbase 1)) k))
    (ds-e (list 'store (+ hbase 2) (list 'get 'fp)))
    (if target
        (ds-transfer-args target env-expr nargs
                          (ds-sp-after (+ hbase 4 nargs))
                          (ds-sp-after (+ hbase 4)))
        (ds-transfer-indirect code-expr env-expr nargs
                              (ds-sp-after (+ hbase 4 nargs))
                              (ds-sp-after (+ hbase 4))))
    (ds-close-unit!)
    (ds-begin-unit k hbase)
    (set! *ds-slot* hbase)))

(define (ds-gen-app rator rands scope tail?)
  (cond
   ;; let: lambda in operator position (parallel binding)
   [(lambda-exp? rator)
    (let ([ids (lambda-exp->ids rator)] [saved *ds-slot*])
      (unless (= (length ids) (length rands)) (ds-die "let arity"))
      (let ([ts (ds-stage-args rands scope)])
        (for-each (lambda (id sl)
                    (ds-e (list 'load sl))
                    (ds-bind-value! scope id))
                  ids ts))
      (ds-gen-seq (flatten-begin (lambda-exp->body rator)) scope tail?)
      (set! *ds-slot* saved))]
   [(ilambda-exp? rator) (ds-die "operator ilambda")]
   [else
    (let ([target (ds-known-target rator scope)] [n (length rands)])
      (when target
        (ds-check-arity target n
          (and (var-exp? rator) (var-exp->id rator))))
      (cond
       [tail?
        (let ([ts (ds-stage-args rands scope)])
          (cond [target
                 (ds-slide-args! ts)
                 (ds-transfer-args (proc-entry target) '(i64.const 0) n
                                   (ds-sp-after n) '(local.get $fp))]
                [else
                 (ds-gen-value rator scope)
                 (let ([cs (ds-alloc-slot!)])
                   (ds-e (list 'store cs (list 'get 'ac)))
                   (ds-check-proc! (slot-load cs))
                   (ds-slide-args! ts)
                   (ds-transfer-indirect
                    (ds-code-ptr (slot-load cs))
                    (slot-load cs) n
                    (ds-sp-after n) '(local.get $fp)))]))]
       [else
        (let ([cs #f] [hbase #f])
          (unless target
            (ds-gen-value rator scope)
            (set! cs (ds-alloc-slot!))
            (ds-e (list 'store cs (list 'get 'ac))))
          ;; Choose the header base only now, after the operator
          ;; has been evaluated: a computed operator (an if/letcc/
          ;; closure rator ending in a join unit) may have advanced
          ;; *ds-slot* since this ds-gen-app was entered.  Reserving
          ;; the header from the entry-time slot count can place the
          ;; continuation word (hbase+1) on top of the operator slot
          ;; cs, so the call would jump through the continuation's
          ;; code word instead of the operator closure.
          (set! hbase (if target *ds-slot* (+ cs 1)))
          ;; reserve header+arg area against nested allocations
          (set! *ds-slot* (+ hbase 4 n))
          (let loop ([rs rands] [j 0])
            (unless (null? rs)
              (ds-gen-value (car rs) scope)
              (ds-e (list 'store (+ hbase 4 j) (list 'get 'ac)))
              (loop (cdr rs) (+ j 1))))
          (unless target (ds-check-proc! (slot-load cs)))
          (if target
              (ds-finish-call! (proc-entry target) '(i64.const 0) #f
                               n hbase)
              (ds-finish-call! #f (slot-load cs)
                               (ds-code-ptr (slot-load cs))
                               n hbase)))]))]))

;; word 0 of a closure object is its code pointer, which is what an
;; indirect call jumps through
(define (ds-code-ptr obj)
  (list 'ptr.load 'offset 0 (list 'ptr.add64 '(ptr.null) obj)))

;;; ---------------------------------------------------------------
;;; prims
;;; ---------------------------------------------------------------


;; The slot a variable reference ALREADY occupies, or #f.  Only slot
;; kind qualifies: boxslot holds a box rather than the value, and cell
;; kinds live in an envvec.  Depth is not consulted because every
;; slot-kind binding is in the current physical frame -- ds-var-expr
;; ignores it for exactly that reason. [cco]
(define (ds-rand-own-slot r scope)
  (and (var-exp? r)
       (let ([id (var-exp->id r)])
         (and (not (global-id? id))
              (let ([lk (ds-scope-lookup scope id)])
                (and lk
                     (eq? (bnd-kind (car lk)) 'slot)
                     (bnd-idx (car lk))))))))

;; Prim operands as DESCRIPTORS rather than bare slot indices, so that
;; an operand needing no slot at all can say so.  A descriptor is
;; whatever ds-opnd accepts: (slot k), or (marv form) for an immediate.
;;
;; The win over ds-prim-args is the immediate case.  A constant operand
;; used to be staged like any other -- set $ac to the constant, store
;; it to a fresh slot, read it back -- which is two instructions and a
;; slot for a value the instruction can simply carry.  (- x 1) showed
;; it plainly: tak's inner call set $ac to 7, stored it to slot 11 and
;; loaded slot 11 straight back.  Constants as prim operands are
;; everywhere -- (+ i 1), (vector-ref v 0), comparisons against 0 --
;; and takl's module staged 194 of them.  Each slot recovered also
;; lowers maxslot, and so the per-call frame fill.
;;
;; Flonums go through (marv ...) rather than (imm ...) because
;; ds-immediate-expr renders them as $scmn:flonum_obj of an f64.const,
;; not as a plain integer; (marv ...) carries any of them. [cco]
(define (ds-prim-opnds rands scope)
  (map (lambda (r)
         (cond
          [(and (quote-exp? r) (datum-immediate? (quote-exp->obj r)))
           (list 'marv (ds-immediate-expr (quote-exp->obj r)))]
          [(ds-rand-own-slot r scope)
           => (lambda (k) (list 'slot k))]
          [else
           (let ([s (ds-alloc-slot!)])
             (ds-gen-value r scope)
             (ds-e (list 'store s (list 'get 'ac)))
             (list 'slot s))]))
       rands))

;; put a descriptor's value in $ac.  A slot keeps the (load k)
;; spelling, because that is the form the ds-e peephole recognises.
(define (ds-load-opnd d)
  (if (eq? (car d) 'slot)
      (ds-e (list 'load (cadr d)))
      (ds-e (list 'set "$ac" d))))

(define (ds-prim-args rands scope)
  ;; Evaluate all args to slots and return the slot list -- except that
  ;; an argument already sitting in a slot of its own is passed through
  ;; by that slot instead of being copied into a fresh one.
  ;;
  ;; The copy this removes was pure waste: (< y x) staged two
  ;; parameters that were already in slots 0 and 1, so tak's entry unit
  ;; stored each to a new slot and loaded it straight back.  About 1200
  ;; store/reload round-trips per module were this, some 5% of every
  ;; line emitted, and each fresh slot also raised maxslot and so the
  ;; frame fill and $sp with it.
  ;;
  ;; Three things make the pass-through safe, and all three are checked
  ;; rather than assumed.  Slot kind means UNASSIGNED -- an assigned
  ;; local is boxslot -- so no other argument's evaluation can change
  ;; the value between here and the use, which is what the old snapshot
  ;; into a fresh slot was protecting.  No consumer writes into a
  ;; staged slot; they only load and push, so the variable cannot be
  ;; clobbered through its alias.  And ds-slide-args!, which does need
  ;; a consecutive run, is only ever given ds-stage-args' slots, never
  ;; these.  ds-operand-expr states the same idea for an operand
  ;; EXPRESSION, but it is dead code and these 34 call sites want a
  ;; slot index, so this is that idea where the callers can take
  ;; it. [cco]
  (map (lambda (r)
         (or (ds-rand-own-slot r scope)
             (let ([s (ds-alloc-slot!)])
               (ds-gen-value r scope)
               (ds-e (list 'store s (list 'get 'ac)))
               s)))
       rands))

(define (ds-gen-primapp prim rands scope)
  (let ([name (prim->sym prim)])
    (cond
     [(equal? prim "") (ds-e (list 'const 'ac 21))]
     [(not name) (ds-die "prim without name" prim)]
     [(and (eq? name '-) (= (length rands) 1))
      (ds-gen-value (car rands) scope)
      (ds-e (list 'rt 'gneg))]
     [(and (eq? name '/) (= (length rands) 1))
      (let ([sl (ds-alloc-slot!)])
        (ds-gen-value (car rands) scope)
        (ds-e (list 'store sl (list 'get 'ac)))
        (ds-reserve 2)
        (ds-e (list 'const 'ac 7))
        (ds-e (list 'push (list 'slot sl)))
        (ds-e (list 'rt 'div)))]
     [(memq name '(floor ceiling truncate round))
      (ds-gen-value (car rands) scope)
      (ds-e (list 'guard
                  '(marv (i32.eqz ($scmn:is_fixnum (local.get $ac))))
                  (list (list 'rt (case name
                                      [(floor) "jfloor"] [(ceiling) "jceil"]
                                      [(truncate) "jtrunc"] [else "jround"])))))]
     [(eq? name 'vector-length)
      (ds-gen-value (car rands) scope)
      (ds-e (list 'set "$ac"
                  (list 'marv (ds-fixnum-of
                                '($scmn:vector_len (local.get $ac))))))]
     [(eq? name 'make-list)
      (let ([sn (ds-alloc-slot!)] [sf (ds-alloc-slot!)]
            [sr (ds-alloc-slot!)] [tc (ds-fresh-temp)])
        (ds-gen-value (car rands) scope)
        (ds-e (list 'store sn (list 'get 'ac)))
        (if (> (length rands) 1)
            (begin (ds-gen-value (cadr rands) scope)
                   (ds-e (list 'store sf (list 'get 'ac))))
            (ds-e (list 'store sf (list 'imm 0))))
        (ds-e (list 'const 'ac 13))
        (ds-e (list 'store sr (list 'get 'ac)))
        (ds-e (list 'set tc
                      (list 'rep 'get_fixnum (slot-load sn))))
        (set! *ds-cap-seq* (+ *ds-cap-seq* 1))
        (let ([lp (ds-name "$ds_ml" *ds-cap-seq* "_lp")]
              [rs2 (ds-alloc-slot!)])
          (ds-e
           (list 'guard
                 (list 'cmp 'i64.gt_s (ds-get tc) '(i64.const 0))
                 (list
                  (list 'loop lp
                        (ds-capture
                         (lambda ()
                           (ds-e (list 'store rs2 (list 'get 'ac)))
                           (ds-reserve 3)
                           (ds-e (list 'slot-ptr rs2))
                           (ds-e (list 'load sf))
                           (ds-e (list 'push (list 'slot sr)))
                           (ds-e '(rt1 cons_to (get p)))
                           (ds-e (list 'load rs2))
                           (ds-e (list 'store sr (list 'get 'ac)))
                           (ds-e (list 'step tc -1))
                           (ds-e (list 'br-if lp
                                       (list 'cmp 'i64.gt_s (ds-get tc)
                                             '(i64.const 0))))))))))
          (ds-e (list 'load sr))))]
     [(eq? name 'box)
      (ds-gen-value (car rands) scope)
      (let ([t (ds-make-box!)])
        (ds-e (list 'set "$ac" (list 'get t))))]
     [(eq? name 'set-box!)
      (let ([ss (ds-prim-args rands scope)])
        (ds-e (list 'load (cadr ss)))
        (ds-e (list 'box-set (list 'slot (car ss)) '(get ac)))
        (ds-e (list 'const 'ac 21)))]
     [(eq? name 'cons)
      (let ([ss (ds-prim-args rands scope)] [rs (ds-alloc-slot!)]
            [p (ds-fresh-temp)])
        (ds-reserve 3)
        (ds-e (list 'slot-ptr rs))
        (ds-e (list 'load (car ss)))
        (ds-e (list 'push (list 'slot (cadr ss))))
        (ds-e '(rt1 cons_to (get p)))
        (ds-e (list 'load rs)))]
     [(eq? name 'make-record)
      ;; (make-record rtd n): a PACKED block of max(n+1,3) cells,
      ;; rtd in slot 0, fields (initially #f) in slots 1..n.  The
      ;; 3-cell floor keeps records disjoint from pairs (PACKED,
      ;; exactly 2 cells) and boxes (1 cell); see $scmi:recp.
      (let ([ss (ds-prim-args rands scope)]
            [tn (ds-fresh-temp)] [tsz (ds-fresh-temp)]
            [ti (ds-fresh-temp)])
        (ds-e (list 'set tn
                    (list 'rep 'get_fixnum (slot-load (cadr ss)))))
        (ds-e (list 'set tsz
                    (list 'marv (list 'i64.add (ds-get tn)
                                      '(i64.const 1)))))
        (ds-e (list 'guard
                    (list 'cmp 'i64.lt_s (ds-get tsz) '(i64.const 3))
                    (list (list 'set tsz '(imm 3)))))
        (let ([cells (list 'i32.wrap_i64
                           (list 'i64.add (ds-get tsz)
                                 '(i64.const 1)))])
          (ds-reserve-expr cells
                           (list 'i32.shl cells '(i32.const 3))))
        (ds-e (list 'alloc "$ac" "packed"
                    (list 'i32 (list 'get tsz))))
        (ds-e (list 'cell-set '(get ac) 0 (list 'slot (car ss))))
        (ds-e (list 'set ti '(imm 1)))
        (ds-e
         (list 'loop '$ds_mr_lp
               (ds-capture
                (lambda ()
                  (ds-e (list 'marv
                              (list 'i64.store 'offset 0
                                    (list 'ptr.add64
                                          '(ptr.add64 (ptr.null)
                                                      (local.get $ac))
                                          (list 'i64.shl (ds-get ti)
                                                '(i64.const 3)))
                                    '(i64.const 0))))
                  (ds-e (list 'step ti 1))
                  (ds-e (list 'br-if '$ds_mr_lp
                              (list 'cmp 'i64.lt_u (ds-get ti)
                                    (ds-get tsz)))))))))]
     [(eq? name 'make-vector)
      (let ([ss (ds-prim-args (list (car rands)) scope)]
            [tn (ds-fresh-temp)] [ti (ds-fresh-temp)])
        (when (> (length rands) 1) (ds-gen-value (cadr rands) scope))
        (let ([fs (if (> (length rands) 1)
                      (let ([s (ds-alloc-slot!)])
                        (ds-e (list 'store s (list 'get 'ac))) s)
                      #f)])
          (ds-e (list 'set tn
                      (list 'rep 'get_fixnum (slot-load (car ss)))))
          (let ([cells (list 'i32.wrap_i64
                             (list 'i64.add (ds-get tn)
                                   '(i64.const 1)))])
            (ds-reserve-expr cells
                             (list 'i32.shl cells '(i32.const 3))))
          (ds-e (list 'alloc "$ac" "vector"
                      (list 'i32 (list 'get tn))))
          (let ([fill (if fs (slot-load fs) '(i64.const 0))])
            (ds-e (list 'set ti '(imm 0)))
            (ds-e
             (list 'block '$ds_mv_done
                   (list
                    (list 'loop '$ds_mv_lp
                          (ds-capture
                           (lambda ()
                             (ds-e (list 'br-if '$ds_mv_done
                                         (list 'marv
                                               (list 'i64.ge_u
                                                     (ds-get ti)
                                                     (ds-get tn)))))
                             (ds-e (list 'marv
                                         (list 'i64.store 'offset 0
                                               (list 'ptr.add64
                                                     '(ptr.add64 (ptr.null)
                                                                 (local.get $ac))
                                                     (list 'i64.shl
                                                           (ds-get ti)
                                                           '(i64.const 3)))
                                               fill)))
                             (ds-e (list 'set ti
                                         (list 'marv
                                               (list 'i64.add
                                                     (ds-get ti)
                                                     '(i64.const 1)))))
                             (ds-e '(br $ds_mv_lp)))))))))))]
     [(eq? name 'vector)
      (let ([ss (ds-prim-args rands scope)])
        (ds-reserve (+ 1 (length ss)))
        (ds-e (list 'alloc "$ac" "vector" (list 'imm-i32 (length ss))))
        (let loop ([ss ss] [j 0])
          (unless (null? ss)
            (ds-e (list 'cell-set '(get ac) j (list 'slot (car ss))))
            (loop (cdr ss) (+ j 1)))))]
     ;; vector-set! and the pair setters are emitted inline rather than
     ;; through $vput and $setcar, so the checks in those macros never
     ;; applied here and have to be emitted alongside.  [cco]
     [(eq? name 'vector-set!)
      (let ([ss (ds-prim-args rands scope)])
        (ds-e (list 'rt2 "ckblk" (list 'slot (car ss))
                    (list 'slot (cadr ss))))
        ;; the index is a fixnum object, and vector_set takes an i32
        (ds-e (list 'marv
                    (list '$scmn:vector_set (slot-load (car ss))
                          (list 'i32.wrap_i64
                                (list '$scmn:get_fixnum
                                      (slot-load (cadr ss))))
                          (slot-load (caddr ss)))))
        (ds-e (list 'const 'ac 21)))]
     [(memq name '(set-car! set-cdr!))
      (let ([ss (ds-prim-args rands scope)])
        (ds-e (list 'rt1 "ckpair" (list 'slot (car ss))))
        (ds-e (list 'cell-set (list 'slot (car ss))
                    (if (eq? name 'set-car!) 0 1)
                    (list 'slot (cadr ss))))
        (ds-e (list 'const 'ac 21)))]
     [(eq? name 'list)
      (let ([ss (ds-prim-args rands scope)])
        (ds-e (list 'const 'ac 13))
        (let loop ([ss (reverse ss)])
          (unless (null? ss)
            (let ([rs (ds-alloc-slot!)] [p (ds-fresh-temp)]
                  [cd (ds-alloc-slot!)])
              (ds-e (list 'store cd (list 'get 'ac)))
              (ds-reserve 3)
              (ds-e (list 'slot-ptr rs))
              (ds-e (list 'load (car ss)))
              (ds-e (list 'push (list 'slot cd)))
              (ds-e '(rt1 cons_to (get p)))
              (ds-e (list 'load rs)))
            (loop (cdr ss)))))]
     [(eq? name 'reverse)
      (let ([ss (ds-prim-args rands scope)])
        (ds-e (list 'load (car ss)))
        (ds-e (list 'rt 'llen))
        (ds-reserve-by-length
         '(i32.wrap_i64 ($scmn:get_fixnum (local.get $ac))))
        (ds-e (list 'load (car ss)))
        (ds-e (list 'rt 'lrev)))]
     [(memq name '(append list-cat))
      (let ([ss (ds-prim-args rands scope)])
        (ds-e (list 'load (car ss)))
        (ds-e (list 'rt 'llen))
        (ds-reserve-by-length
         '(i32.wrap_i64 ($scmn:get_fixnum (local.get $ac))))
        (ds-e (list 'load (car ss)))
        (ds-e (list 'push (list 'slot (cadr ss))))
        (ds-e (list 'rt 'lcat)))]
     [(memq name '(memq memv meme assq assv asse))
      (let ([ss (ds-prim-args rands scope)])
        (ds-e (list 'load (cadr ss)))
        (ds-e (list 'push (list 'slot (car ss))))
        (ds-e (list 'rt name)))]
     [(eq? name 'string-append)
      (let ([ss (ds-prim-args rands scope)])
        (cond [(null? ss)
               (ds-reserve 2)
               (ds-e (list 'const 'ac 3))
               (ds-e '(push (imm 9)))
               (ds-e (list 'rt 'smk))]
              [(null? (cdr ss))
               (ds-e (list 'load (car ss)))]
              [else
               (let ([sa (ds-alloc-slot!)])
                 (ds-e (list 'load (car ss)))
                 (ds-e (list 'store sa (list 'get 'ac)))
                 (let loop ([rest (cdr ss)])
                   (unless (null? rest)
                     (ds-reserve 2)
                     (ds-e (list 'load sa))
                     (ds-e (list 'push (list 'slot (car rest))))
                     (ds-e (list 'rt 'scat))
                     (ds-e (list 'store sa (list 'get 'ac)))
                     (loop (cdr rest))))
                 (ds-e (list 'load sa)))]))]
     [(eq? name 'string)
      (let ([ss (ds-prim-args rands scope)] [tl (ds-alloc-slot!)])
        (ds-reserve (+ (* 3 (length ss)) 2))
        (ds-e (list 'const 'ac 13))
        (ds-e (list 'store tl (list 'get 'ac)))
        (let loop ([rest (reverse ss)])
          (unless (null? rest)
            (let ([rs (ds-alloc-slot!)] [p (ds-fresh-temp)])
              (ds-e (list 'slot-ptr rs))
              (ds-e (list 'load (car rest)))
              (ds-e (list 'push (list 'slot tl)))
              (ds-e '(rt1 cons_to (get p)))
              (ds-e (list 'load rs))
              (ds-e (list 'store tl (list 'get 'ac)))
              (loop (cdr rest)))))
        (ds-e (list 'load tl))
        (ds-e (list 'rt 'lstr)))]
     [(eq? name 'substring)
      (let ([ss (ds-prim-args rands scope)])
        (ds-reserve 2)
        (ds-e (list 'load (car ss)))
        (ds-e (list 'push (list 'slot (cadr ss))))
        (ds-e (list 'push (list 'slot (caddr ss))))
        (ds-e (list 'rt 'ssub)))]
     [(eq? name 'string-set!)
      (let ([ss (ds-prim-args rands scope)])
        (ds-e (list 'load (car ss)))
        (ds-e (list 'push (list 'slot (caddr ss))))
        (ds-e (list 'push (list 'slot (cadr ss))))
        (ds-e (list 'rt 'sput))
        (ds-e (list 'const 'ac 21)))]
     [(eq? name 'make-string)
      (let ([ss (ds-prim-args rands scope)])
        (ds-reserve 2)
        (ds-e (list 'load (car ss)))
        (ds-e (list 'push (if (= (length ss) 2) (list 'slot (cadr ss)) '(imm 9))))
        (ds-e (list 'rt 'smk)))]
     [(eq? name 'make-bytevector)
      (let ([ss (ds-prim-args rands scope)])
        (ds-reserve 2)
        (ds-e (list 'load (car ss)))
        (ds-e (list 'rt 'bvnew))
        (when (= (length ss) 2)
          (ds-e (list 'push (list 'slot (cadr ss))))
          (ds-e (list 'rt 'bvfill))))]
     [(eq? name 'make-numvector)
      (let ([ss (ds-prim-args rands scope)])
        (ds-reserve 2)
        (ds-e (list 'load (car ss)))
        (ds-e (list 'push (list 'slot (cadr ss))))
        (ds-e (list (quote rt) (quote nvnew))))]
     [(eq? name 'bytevector->list)
      (let ([ss (ds-prim-args rands scope)])
        (ds-reserve-by-length
         (list 'call '$scmrt:bytevector_length (slot-load (car ss))))
        (ds-e (list 'load (car ss)))
        (ds-e (list 'rt 'bvlist)))]
     [(eq? name 'numvector-set!)
      (let ([ss (ds-prim-args rands scope)])
        (ds-e (list 'load (car ss)))
        (ds-e (list 'push (list 'slot (cadr ss))))
        (ds-e (list 'push (list 'slot (caddr ss))))
        (ds-e (list 'rt 'nvput))
        (ds-e (list 'const 'ac 21)))]
     [(eq? name 'bytevector-u8-set!)
      (let ([ss (ds-prim-args rands scope)])
        (ds-e (list 'load (car ss)))
        (ds-e (list 'push (list 'slot (cadr ss))))
        (ds-e (list 'push (list 'slot (caddr ss))))
        (ds-e (list 'rt 'bvput))
        (ds-e (list 'const 'ac 21)))]
     [(eq? name 'number->string)
      (let ([ss (ds-prim-args rands scope)])
        (ds-reserve 2)
        (ds-e (list 'load (car ss)))
        (ds-e (list 'push (if (= (length ss) 2) (list 'slot (cadr ss)) '(imm 43))))
        (ds-e (list 'rt 'n2s)))]
     [(eq? name (quote %string->list1))
      (let ([ss (ds-prim-args rands scope)])
        (ds-reserve-by-length
         (list '$scmn:string_len (slot-load (car ss))))
        (ds-e (list 'load (car ss)))
        (ds-e (list 'rt 'slist)))]
     [(eq? name 'string->number)
      (let ([ss (ds-prim-args rands scope)])
        (ds-e (list 'set "$ac"
                    (list 'rtcall 'string_to_number
                                (slot-load (car ss))
                                (if (= (length ss) 2)
                                    (slot-load (cadr ss))
                                    '(i64.const 43))))))]
     [(eq? name 'flfrexp)
      (let ([ss (ds-prim-args rands scope)])
        (ds-e (list 'set "$ac"
                    (list 'rtcall 'frexp_into_box
                            (slot-load (car ss))
                            (slot-load (cadr ss))))))]
     [(memq name '(%current-input-port %current-output-port
                   %current-error-port current-jiffy eof-object))
      (ds-e (list 'set "$ac"
                  (case name
                    [(eof-object) '(imm 29)]
                    [else
                     (list 'rtcall
                           (case name
                             [(%current-input-port) 'current_input_port]
                             [(%current-output-port) 'current_output_port]
                             [(%current-error-port) 'current_error_port]
                             [else 'current_jiffy]))])))]
     [(memq name '(%set-current-input-port! %set-current-output-port!
                   %set-current-error-port!))
      (let ([ss (ds-prim-args rands scope)])
        (ds-e (list 'rtcall
                    (case name
                      [(%set-current-input-port!) 'set_current_input_port]
                      [(%set-current-output-port!) 'set_current_output_port]
                      [else 'set_current_error_port])
                    (slot-load (car ss))))
        (ds-e (list 'const 'ac 21)))] ; SCM_VOID
     ;; %port? takes an optional and-mask, defaulting to 7 (the '7 in
     ;; i.h's signature); 31 is the fixnum 7.  It was in the unary
     ;; table, so the two-argument form the shadow (scheme base) uses
     ;; for textual-port? and binary-port? lost its mask.  [cco]
     [(eq? name '%port?)
      (let ([ss (ds-prim-args rands scope)])
        (ds-e (list 'load (car ss)))
        (ds-e (list 'push (if (= (length ss) 2) (list 'slot (cadr ss)) '(imm 31))))
        (ds-e (list 'rt 'portp)))]
     [(eq? name 'open-output-string)
      (ds-reserve 2)
      (ds-e (list 'rt 'soutport))]
     [(eq? name 'open-output-bytevector)
      (ds-reserve 2)
      (ds-e (list 'rt 'boutport))]
     [(memq name '(symbol->string list->string string-downcase
                   open-input-file open-output-file
                   %open-input-file %open-output-file
                   %open-binary-input-file %open-binary-output-file
                   open-input-string get-output-string
                   open-input-bytevector get-output-bytevector
                   get-environment-variable %argv-ref %envv-ref))
      (let ([sl (ds-alloc-slot!)])
        (ds-gen-value (car rands) scope)
        (ds-e (list 'store sl (list 'get 'ac)))
        (ds-reserve 2)
        (ds-e (list 'load sl))
        (ds-e (list 'rt (case name
                        [(symbol->string) "symstr"]
                        [(list->string) "lstr"]
                        [(string-downcase) "sdwn"]
                        [(open-input-file) "openin"]
                        [(open-output-file) "openout"]
                        [(%open-input-file) "openin2"]
                        [(%open-output-file) "openout2"]
                        [(%open-binary-input-file) "openin2"]
                        [(%open-binary-output-file) "openout2"]
                        [(open-input-string) "sinport"]
                        [(get-output-string) "getostr"]
                        [(open-input-bytevector) "binport"]
                        [(get-output-bytevector) "getbvo"]
                        [(%argv-ref) (set! *ds-need-argv* #t) "argvref"]
                        [(%envv-ref) "envvref"]
                        [else "getenv"]))))]
     [(memq name '(close-input-port close-output-port close-port
                   flush-output-port))
      (if (null? rands)
          (ds-e (list 'set "$ac"
                      '(rtcall current_output_port)))
          (ds-gen-value (car rands) scope))
      (ds-e (list 'rtcall
                  (if (eq? name 'flush-output-port)
                      'flush_port 'close_port)
                  '(get ac)))
      (ds-e (list 'const 'ac 21))]
     [(memq name '(read-char peek-char))
      (if (null? rands)
          (ds-e (list 'set "$ac"
                      '(rtcall current_input_port)))
          (ds-gen-value (car rands) scope))
      (ds-e (list 'rt (if (eq? name 'read-char) 'rdchar 'pkchar)))]
     [(memq name '(input-port-open? output-port-open?))
      (ds-gen-value (car rands) scope)
      (ds-e (list 'set "$ac"
                  (list 'rep 'bool_obj
                        (list 'rtcall
                              (if (eq? name 'input-port-open?)
                                  'port_input_open_p
                                  'port_output_open_p)
                              '(get ac)))))]
     [(memq name '(atan flatan))
      (let ([ss (ds-prim-args rands scope)])
        (ds-e (list 'load (car ss)))
        (if (= (length ss) 2)
            (begin (ds-e (list 'push (list 'slot (cadr ss))))
                   (ds-e (list 'rt 'gatan2)))
            (ds-e (list 'rt 'gatan))))]
     [(memq name '(log fllog))
      (let ([ss (ds-prim-args rands scope)])
        (ds-e (list 'load (car ss)))
        (ds-e (list 'rt 'glog))
        (when (= (length ss) 2)
          (let ([sl (ds-alloc-slot!)] [s2 (ds-alloc-slot!)])
            (ds-reserve 2)
            (ds-e (list 'store sl (list 'get 'ac)))
            (ds-e (list 'load (cadr ss)))
            (ds-e (list 'rt 'glog))
            (ds-e (list 'store s2 (list 'get 'ac)))
            (ds-e (list 'load sl))
            (ds-e (list 'push (list 'slot s2)))
            (ds-e (list 'rt 'div)))))]
     [(eq? name 'current-second)
      (ds-e (list 'rt 'cursec))]
     [(eq? name 'jiffies-per-second)
      ;; scmrt jiffies are milliseconds (see ds-jps-unit)
      (ds-e (list 'const 'ac 4003))]
     [(memq name '(file-exists? delete-file))
      (ds-gen-value (car rands) scope)
      (ds-e (list 'set "$ac"
                  (list 'rep 'bool_obj
                        (list 'rtcall
                              (if (eq? name 'file-exists?)
                                  'file_exists_p 'delete_file_p)
                              '(get ac)))))]
     [(eq? name '%system)
      (ds-gen-value (car rands) scope)
      (ds-e (list 'set "$ac"
                  '(rtcall system_cmd (local.get $ac))))]
     [(memq name '(exit %exit))
      (if (null? rands)
          (ds-e (list 'const 'ac 3))
          (ds-gen-value (car rands) scope))
      (ds-e '(rtcall exit_process (local.get $ac)))
      (ds-e (list 'const 'ac 21))]
     [(memq name '(display write write-simple write-shared write-char newline))
      (cond [(eq? name 'newline)
             (if (null? rands)
                 (ds-e (list 'rt 'newline))
                 (begin (ds-gen-value (car rands) scope)
                        (ds-e '(rtcall port_write_char
                                          (get ac) (i32.const 10)))))]
            [(and (eq? name 'write-char) (= (length rands) 2))
             (let ([ss (ds-prim-args rands scope)])
               (ds-e (list 'rtcall 'port_write_char
                           (slot-load (cadr ss))
                           (list 'i32 (list 'rep 'get_char
                                              (slot-load (car ss)))))))]
            [(= (length rands) 2)
             (let ([ss (ds-prim-args rands scope)])
               (ds-e (list 'rtcall
                           (case name
                             [(display) 'display_obj_port]
                             [(write-shared) 'write_shared_obj_port]
                             [else 'write_obj_port])
                           (slot-load (cadr ss))
                           (slot-load (car ss)))))]
            [else (ds-gen-value (car rands) scope)
                  (ds-e (list 'rt (case name
                                  [(display) "display"]
                                  [(write write-simple) "write"]
                                  [(write-shared) "write_shared"]
                                  [else "writechar"])))])
      (ds-e (list 'const 'ac 21))]
     [(and (= (length rands) 1) (ds-cxr-ops name))
      => (lambda (ops)
           (ds-gen-value (car rands) scope)
           (for-each (lambda (op) (ds-e (list 'rt op))) ops))]
     [(memq name '(%fxsqrt fxaddc fxsubc fxmulc flmodf flfrexp))
      ;; box-writing builtins: the last operand is the box object
      ;; (usually produced by a varaddress / (set& x) operand);
      ;; the C helper fills its cell, the primary result is in $ac
      (let ([ss (ds-prim-args rands scope)])
        (case name
          [(%fxsqrt)
           (cond [(= (length ss) 1)
                  (ds-e (list 'load (car ss)))
                  (ds-e (list 'rt 'fxsqrt))]
                 [(= (length ss) 2)
                  (ds-e (list 'load (car ss)))
                  (ds-e (list 'push (list 'slot (cadr ss))))
                  (ds-e (list 'rt 'fxsqrtb))]
                 [else (ds-die "%fxsqrt arity" name)])]
          [(flmodf flfrexp)
           (unless (= (length ss) 2) (ds-die "box prim arity" name))
           (ds-e (list 'load (car ss)))
           (ds-e (list 'push (list 'slot (cadr ss))))
           (ds-e (list 'rt (case name [(flmodf) "flmodfb"] [else "flfrexpb"])))]
          [else
           (unless (= (length ss) 3) (ds-die "carry prim arity" name))
           (ds-e (list 'load (car ss)))
           (ds-e (list 'push (list 'slot (cadr ss))))
           (ds-e (list 'push (list 'slot (caddr ss))))
           (ds-e (list 'rt (case name
                           [(fxaddc) "fxaddc"] [(fxsubc) "fxsubc"]
                           [else "fxmulc"])))]))]
     [(assq name *ds-prim-bin*)
      => (lambda (pr)
           (let ([ss (ds-prim-opnds rands scope)])
             (when (and *ds-generic-arith-allocates*
                        (memq name '(+ - * / quotient floor-quotient expt)))
               (ds-reserve 2))
             (when (and (memq name '(< <= > >= = eq? eqv? equal?
                                     fx=? fx<? fx<=? fx>? fx>=? fx!=?
                                     fl=? fl<? fl<=? fl>? fl>=? fl!=?
                                     char=? char<? char>? char<=? char>=? boolean=?))
                        (not (= (length ss) 2)))
               ;; n-ary comparison: pairwise chain, ANDed
               (cond [(< (length ss) 2) (ds-e (list 'const 'ac 1))]
                     [else
                      (let ([sr (ds-alloc-slot!)])
                        (ds-load-opnd (car ss))
                        (ds-e (list 'push (cadr ss)))
                        (ds-e (list 'rt (cdr pr)))
                        (ds-e (list 'store sr (list 'get 'ac)))
                        (let loop ([prev (cadr ss)] [rest (cddr ss)])
                          (unless (null? rest)
                            ;; the guards are siblings, not nested: each
                            ;; re-reads the running result, so a false one
                            ;; leaves every later comparison unrun. [cco]
                            (ds-e
                             (list 'guard
                                   (list 'cmp 'i64.ne (slot-load sr)
                                         '(i64.const 0))
                                   (ds-capture
                                    (lambda ()
                                      (ds-load-opnd prev)
                                      (ds-e (list 'push (car rest)))
                                      (ds-e (list 'rt (cdr pr)))
                                      (ds-e (list 'store sr (list 'get 'ac)))))))
                            (loop (car rest) (cdr rest))))
                        (ds-e (list 'load sr)))]))
             (unless (and (memq name '(< <= > >= = eq? eqv? equal?
                                       fx=? fx<? fx<=? fx>? fx>=? fx!=?
                                       fl=? fl<? fl<=? fl>? fl>=? fl!=?
                                       char=? char<? char>? char<=? char>=? boolean=?))
                          (not (= (length ss) 2)))
               (cond [(and (memq name '(/ fl/)) (= (length ss) 1))
                      ;; one-argument / and fl/ are reciprocals:
                      ;; compute 1 (resp. 1.0) divided by the operand
                      (ds-e (list 'const 'ac 7))
                      (when (eq? name 'fl/) (ds-e (list 'rt 'itoj)))
                      (ds-e (list 'push (car ss)))
                      (ds-e (list 'rt (cdr pr)))]
                     [(and (memq name '(- fx- fl-)) (= (length ss) 1))
                      ;; one-argument - fx- fl- are negations
                      (ds-load-opnd (car ss))
                      (ds-e (list 'rt (case name
                                      [(-) "gneg"] [(fx-) "ineg"]
                                      [else "jneg"])))]
                     [else
                      ;; zero-argument call of a variadic prim:
                      ;; its identity element (matching the
                      ;; first-class binfold units exactly), or
                      ;; an arity error for the rest (-, /, min)
                      (when (null? ss)
                        (case name
                          [(+ fx+ fxior fxxor)
                           (ds-e (list 'const 'ac 3))]
                          [(* fx*)
                           (ds-e (list 'const 'ac 7))]
                          [(fxand fxeqv)
                           (ds-e (list 'const 'ac *ds-word-mask*))]
                          [(fl+)
                           (ds-e (list 'set "$ac" '(rep flonum_obj (f64.const 0.0))))]
                          [(fl*)
                           (ds-e (list 'set "$ac" '(rep flonum_obj (f64.const 1.0))))]
                          [else (ds-die "arity" name 0)]))
                      (unless (null? ss)
                        (ds-load-opnd (car ss))
                        (when (and (eq? name 'immediate-hash) (= (length ss) 1))
                          ;; Skint's hshim bound is optional; supply boxed 0,
                          ;; which $scmi:immhash reads as "default bound"
                          (ds-e '(push (imm 3)))
                          (ds-e (list 'rt 'immhash)))
                        (let loop ([ss (if (and (eq? name 'immediate-hash)
                                                (= (length ss) 1))
                                           '() (cdr ss))])
                          (unless (null? ss)
                            (ds-e (list 'push (car ss)))
                            (ds-e (list 'rt (cdr pr)))
                            (when (pair? (cdr ss))
                              (let ([s (ds-alloc-slot!)])
                                (ds-e (list 'store s (list 'get 'ac)))
                                (ds-e (list 'load s))))
                            (loop (cdr ss)))))]))))]
     [(assq name *ds-prim-un*)
      => (lambda (pr)
           (cond [(memq name '(fixnum->flonum flabs flfloor flceiling
                               fltruncate flround flsgn inexact exact
                               abs truncate string->symbol))
                  (let ([sl (ds-alloc-slot!)])
                    (ds-gen-value (car rands) scope)
                    (ds-e (list 'store sl (list 'get 'ac)))
                    (ds-reserve 2)
                    (ds-e (list 'load sl))
                    (ds-e (list 'rt (cdr pr))))]
                 [else
                  (ds-gen-value (car rands) scope)
                  (ds-e (list 'rt (cdr pr)))]))]
     [(eq? name '-)
      (ds-gen-value (car rands) scope)
      (ds-e (list 'rt 'gneg))]
     [(eq? name '/)
      (let ([sl (ds-alloc-slot!)])
        (ds-gen-value (car rands) scope)
        (ds-e (list 'store sl (list 'get 'ac)))
        (ds-reserve 2)
        (ds-e (list 'const 'ac 7))
        (ds-e (list 'push (list 'slot sl)))
        (ds-e (list 'rt 'div)))]
     [else (when (get-environment-variable "DS_DEBUG")
             (display (list 'prim-in-proc
                            (and *ds-cur-proc* (proc-gid *ds-cur-proc*)
                                 (id->uname (proc-gid *ds-cur-proc*))))
                      (current-error-port))
             (newline (current-error-port)))
           (ds-die "prim unsupported in DS stage" name)])))

;;; ---------------------------------------------------------------
;;; proc compilation
;;; ---------------------------------------------------------------

;; The entry unit's frame-fill instruction was emitted before
;; proc-maxslot was known; now it is.  Rebuilding that one instruction
;; is all this takes.  It replaces ds-patch-fill-lines, which existed
;; only because the text had already been assembled: that had to find
;; the fill by matching a placeholder constant in a line, and -- in the
;; branch that replaces the loop with straight-line stores -- by
;; scanning for the line holding the loop's closing parens. [cco]
(define (ds-set-fill-bound! instrs bound)
  (map (lambda (i)
         (if (eq? (car i) 'frame-fill)
             (list 'frame-fill (cadr i) (caddr i) bound)
             i))
       instrs))

(define (ds-compile-proc p)
  (let* ([fr (proc-frame p)]
         [entry (proc-entry p)]
         [params (proc-params p)]
         [npar (length params)]
         [scope (cons fr (proc-def-frames p))])
    (proc-entry-set! p entry)
    (set! *ds-cur-proc* p)
    (set! *ds-sp-dynamic*
          (and (proc-rest p)
               '(i64.add (local.get $fp)
                         (i64.extend_i32_u (local.get $nargs)))))
    (set! *ds-slot* 0)
    ;; param slot indices (params come first in frame order added,
    ;; but assign explicitly by name order)
    (for-each (lambda (id j)
                (let ([b (frame-find fr id)])
                  (when (memq (bnd-kind b) '(slot boxslot))
                    (bnd-idx-set! b j))))
              params (ds-iota npar))
    (set! *ds-slot* npar)
    (when (proc-rest p)
      (let ([b (frame-find fr (proc-rest p))])
        ;; slot kinds live at fp+npar; cell kinds keep the envvec
        ;; cell index assigned by ds-finalize-frame!
        (when (memq (bnd-kind b) '(slot boxslot))
          (bnd-idx-set! b npar))
        (set! *ds-slot* (+ npar 1))))
    (proc-maxslot-set! p *ds-slot*)
    (ds-begin-unit entry npar)
    ;; Frame-birth fill: every slot in [fp+npar, fp+maxslot) must
    ;; hold a valid value before any collection can scan it -- a
    ;; later unit's startdepth or a reserve refresh may cover slots
    ;; whose first write in this frame incarnation has not happened
    ;; yet (staging slots, arm-dependent bindings, slots inherited
    ;; from a tail-call-replaced frame).  One instruction says so; its
    ;; bound is proc-maxslot, which is not known yet and is filled in
    ;; by ds-set-fill-bound! once this proc is compiled.  Whether it
    ;; lowers to a loop or to straight-line stores is a code-shape
    ;; question and so belongs to ds-lower, not here. [cco]
    (ds-e (list 'frame-fill npar (proc-rest p) #f))
    ;; arity check
    (cond [(proc-rest p)
           (ds-e (list 'guard
                       (list 'marv
                             (list 'i32.lt_u '(local.get $nargs)
                                   (list 'i32.const npar)))
                       (ds-capture (lambda () (ds-emit-arity-error npar)))))]
          [(not (eq? p *ds-main-proc*))
           (ds-e (list 'guard
                       (list 'marv
                             (list 'i32.ne '(local.get $nargs)
                                   (list 'i32.const npar)))
                       (ds-capture (lambda () (ds-emit-arity-error npar)))))])
    ;; $env arrives as the closure object; the envvec is cell 1
    (ds-e (list 'set "$env"
                '(marv (if (result i64)
                           (i64.ne (local.get $env) (i64.const 0))
                           (then (i64.load offset 8
                                           (ptr.add64 (ptr.null)
                                                      (local.get $env))))
                           (else (i64.const 0))))))
    ;; env mirror first (root for the incoming env)
    (ds-e (list 'store -1 (list 'get 'env)))
    ;; build the frame envvec when this frame has cells
    (when (frame-cells? fr)
      (let ([nc (proc-ncells p)] [te (ds-fresh-temp)])
        ;; envvec = [parent, cell1..cellN]: N+1 slots + header.
        ;; Pre-fill every cell with () and root the envvec in the
        ;; mirror slot at once: bindings filled later (let/fix cells)
        ;; must never leave an uninitialized cell for a collection
        ;; to scan, and boxing a param may itself collect.
        (ds-reserve (+ nc 2))
        (ds-e (list 'alloc te "vector" (list 'imm-i32 (+ nc 1))))
        (ds-e (list 'cell-set (list 'get te) 0 '(slot -1)))
        (do ([j 1 (+ j 1)]) ((> j nc))
          (ds-e (list 'cell-set (list 'get te) j '(imm 13))))
        (ds-e (list 'store -1 (list 'get te)))
        ;; Keep $env a true mirror of slot -1.  This is the only place
        ;; the two could diverge: the mirror gets a NEW envvec here,
        ;; while $env still holds the incoming one.  Everywhere else
        ;; they already agree -- the other write of slot -1 stores $env
        ;; itself, and ds-reserve emits (env-refresh) right after every
        ;; reserve, so a collection cannot separate them either.  With
        ;; this one instruction per procedure entry the invariant is
        ;; total, which is what lets env-walk-expr read the local
        ;; instead of the slot: slot -1 was the most reloaded slot in
        ;; the output by a wide margin, 360 of 938 removable repeated
        ;; loads in takl. [cco]
        (ds-e (list 'set "$env" (list 'get te)))
        ;; Params into cells / boxes.  te holds the envvec, and the
        ;; mirror slot at -1 is the copy a collection updates, so te
        ;; has to be reloaded from it after anything that can collect.
        ;; Only the boxcell arm can: ds-make-box! allocates.  A cell
        ;; arm just copies a frame slot into an envvec cell, and
        ;; cell-set lowers to $scmn:block_set, a bare i64.store with
        ;; no barrier -- so a run of cell params needs ONE reload
        ;; between them, not one apiece.  Reloading every time was the
        ;; single commonest redundant load in the output: takl's
        ;; $ds$p25 fetched slot -1 seven times in one collection-free
        ;; region, three of those as whole dead instructions that
        ;; reloaded a temp with the value it already held. [cco]
        (let ([stale #t])
          (for-each
           (lambda (id j)
             (let ([b (frame-find fr id)])
               (case (bnd-kind b)
                 [(cell)
                  (when stale
                    (ds-e (list 'load-to te -1))
                    (set! stale #f))
                  (ds-e (list 'cell-set (list 'get te) (bnd-idx b)
                              (list 'slot j)))]
                 [(boxcell)
                  (ds-e (list 'load j))
                  (let ([tb (ds-make-box!)])
                    ;; the box allocation above may have moved the
                    ;; envvec, so this reload is the load-bearing one
                    (ds-e (list 'load-to te -1))
                    (set! stale #f)
                    (ds-e (list 'cell-set (list 'get te) (bnd-idx b)
                                (list 'get tb))))]
                 [else #f])))
           params (ds-iota npar)))))
    ;; boxslot params: replace slot contents with a box
    (for-each
     (lambda (id j)
       (let ([b (frame-find fr id)])
         (when (eq? (bnd-kind b) 'boxslot)
           (ds-e (list 'load j))
           (let ([tb (ds-make-box!)])
             (ds-e (list 'store j (list 'get tb)))))))
     params (ds-iota npar))
    ;; rest arg list
    (when (proc-rest p)
      (let ([rb (frame-find fr (proc-rest p))]
            [ti (ds-fresh-temp)] [th (ds-fresh-temp)])
        ;; cell kinds keep their finalize-assigned envvec index
        (when (memq (bnd-kind rb) '(slot boxslot))
          (bnd-idx-set! rb npar))
        (ds-e (list 'const 'ac 13))
        (ds-e (list 'set ti '(nargs64)))
        ;; The guard and the loop own their bodies, so the reserve and
        ;; the consing walk are instructions inside them rather than
        ;; lines between a matching pair of parens. [cco]
        (ds-e
         (list 'guard
               (list 'marv (list 'i64.gt_u (ds-get ti)
                                 (i64c npar)))
               (ds-capture
                (lambda ()
                  (let ([extra (list 'i32.sub '(local.get $nargs)
                                     (list 'i32.const npar))])
                    (ds-reserve-expr
                     (list 'i32.mul extra '(i32.const 3))
                     (list 'i32.shl (list 'i32.mul extra '(i32.const 3))
                           '(i32.const 3))))
                  ;; list-so-far lives in $ac / th (no GC inside: pre-reserved)
                  (ds-e
                   (list 'loop '$ds_rest_lp
                         (ds-capture
                          (lambda ()
                            (ds-e (list 'set ti
                                        (list 'marv
                                              (list 'i64.sub
                                                    (ds-get ti)
                                                    '(i64.const 1)))))
                            (ds-e (list 'set th '(get ac)))
                            (ds-e (list 'set "$ac"
                                        (list 'marv
                                              (list 'i64.load 'offset 0
                                                    (list 'ptr.add64
                                                          '(local.get $r)
                                                          (list 'i64.shl
                                                                (list 'i64.add
                                                                      '(local.get $fp)
                                                                      (ds-get ti))
                                                                '(i64.const 3)))))))
                            (ds-e (list 'push (list 'get th)))
                            (ds-e (list 'rt 'cons))
                            (ds-e (list 'br-if '$ds_rest_lp
                                        (list 'marv
                                              (list 'i64.gt_u
                                                    (ds-get ti)
                                                    (i64c npar)))))))))))))
        ;; deliver the list according to the binding kind: a captured
        ;; rest binding lives in its envvec cell (boxed if assigned)
        (case (bnd-kind rb)
          [(slot) (ds-e (list 'store (bnd-idx rb) (list 'get 'ac)))]
          [(boxslot)
           (let ([tb (ds-make-box!)])
             (ds-e (list 'store (bnd-idx rb) (list 'get tb))))]
          [(cell)
           (ds-e (list 'cell-set '(slot -1) (bnd-idx rb) '(get ac)))]
          [(boxcell)
           (let ([tb (ds-make-box!)])
             ;; this was built without block_set's closing paren; the
             ;; form supplies it, and no battery program reaches the
             ;; path, which is why nothing had failed to assemble. [cco]
             (ds-e (list 'cell-set '(slot -1) (bnd-idx rb)
                         (list 'get tb))))]) ))
    ;; install host-wrapper closures into their global cells
    (when (eq? p *ds-main-proc*)
      (for-each (lambda (pr)
                  (ds-gen-closure (cdr pr) scope)
                  (ds-e (list 'global-store (car pr) '(get ac))))
                *ds-wrapper-globals*))
    ;; body
    (set! *ds-scope* scope)
    (if (eq? p *ds-main-proc*)
        (begin (ds-gen-seq (proc-body p) scope #f)
               (ds-transfer '$ds_halt))
        (ds-gen-seq (flatten-begin (proc-body p)) scope #t))
    (ds-close-unit!)
    ;; the frame-fill bound is known now that this proc is compiled
    (set! *ds-units*
          (map (lambda (u)
                 (if (equal? (car u) entry)
                     (list (car u)
                           (ds-set-fill-bound! (cadr u) (proc-maxslot p))
                           (caddr u) (cadddr u))
                     u))
               *ds-units*))
    (set! *ds-sp-dynamic* #f)))

;;; ---------------------------------------------------------------
;;; unit emission + hash-consing (dedup)
;;; ---------------------------------------------------------------

;; Canonical names under iterative equivalence: references renamed by
;; canonical class, so chains of identical units collapse.  The key is
;; the renamed instructions LOWERED, not the instruction list itself.
;; That matters: two units can carry different IL and emit the same
;; code, because a register name reaches a form as a symbol at one site
;; and as a string at another, and (set "$ac" ...) is not `equal?` to
;; (set $ac ...) though both emit (local.set $ac ...).  Keying on the
;; IL silently stopped collapsing those, which showed up as 31K of
;; extra output and a shifted procedure numbering.  Keying on what is
;; actually emitted is also the right question to ask: two units are
;; interchangeable when their code is, however their IL spelled it.
;; Nothing is turned into text either way -- assoc compares the lowered
;; trees with equal?. [cco]
(define (ds-canon-map units)
  (let ([name->key (map (lambda (u) (cons (car u) (car u))) units)])
    (define (key-of name) (cond [(assoc name name->key) => cdr] [else name]))
    (let iterate ([n 0])
      (let* ([groups '()])
        (for-each
         (lambda (u)
           (let* ([k (ds-il-marv (ds-rename-instrs (cadr u) key-of))]
                  [hit (assoc k groups)])
             (if hit (set-cdr! hit (cons (car u) (cdr hit)))
                 (set! groups (cons (cons k (list (car u))) groups)))))
         units)
        (let ([newmap
               (apply append
                      (map (lambda (g)
                             (let ([canon (ds-min-name (cdr g))])
                               (map (lambda (nm) (cons nm canon))
                                    (cdr g))))
                           groups))])
          (cond [(or (>= n 6) (equal? newmap name->key)) newmap]
                [else (set! name->key newmap) (iterate (+ n 1))]))))))

(define (ds-min-name names)
  (let loop ([xs (cdr names)] [best (car names)])
    (cond [(null? xs) best]
          [(string<? (symbol->string (car xs)) (symbol->string best))
           (loop (cdr xs) (car xs))]
          [else (loop (cdr xs) best)])))

;; Rename the unit and procedure references in an instruction list.
;; A name can reach an instruction in exactly four forms and five
;; operand positions, which is the whole of it: nothing else carries
;; one, and no (marv ...) operand does either, because
;; ds-new-unit-name is the only thing that spells a $ds$ name.  That is
;; why this is a walk over five positions rather than a scan for a
;; token in text. [cco]
(define (ds-rename-instrs instrs key-of)
  (map (lambda (i) (ds-rename-instr i key-of)) instrs))

(define (ds-rename-instr form key-of)
  (case (car form)
    [(chain) (list 'chain (key-of (cadr form)))]
    [(fork) (list 'fork (key-of (cadr form)) (key-of (caddr form)))]
    [(jump) (cons 'jump (cons (key-of (cadr form)) (cddr form)))]
    [(fn-store) (append (list 'fn-store (cadr form) (key-of (caddr form)))
                        (cdddr form))]
    ;; the forms with arms carry instructions, so they recurse
    [(if-inline)
     (list 'if-inline
           (ds-rename-instrs (cadr form) key-of)
           (ds-rename-instrs (caddr form) key-of))]
    [(guard)
     (list 'guard (cadr form) (ds-rename-instrs (caddr form) key-of))]
    [(loop)
     (list 'loop (cadr form) (ds-rename-instrs (caddr form) key-of))]
    [(block)
     (list 'block (cadr form) (ds-rename-instrs (caddr form) key-of))]
    [else form]))

;;; The shared throw instruction (continuation invocation).  The
;;; hand-written units below are MARV data like any other, so a reader
;;; can see their instructions one per line and the writer treats them
;;; no differently from generated ones. [cco]
(define (ds-throw-unit)
  (list '$ds_throw
        (list
         ;; the value being thrown is the single argument at fp
         (ds-fp-load "$t0" 0)
         ;; the continuation closure carries [code snapshot resume]
         (list 'set "$t1" '(cell-ref (get env) 1))
         (list 'marv (list 'local.set '$p
                           (list 'ptr.load 'offset 16
                                 '(ptr.add64 (ptr.null) (local.get $env)))))
         (list 'set "$t2" '(cell-ref (get t1) 1))
         ;; restore $r[0..sp) from snapshot cells [2..2+sp): bulk copy
         (list 'marv
               (list 'memory.copy '(local.get $r)
                     '(ptr.add64 (ptr.add64 (ptr.null) (local.get $t1))
                                 (i64.const 16))
                     '(i32.wrap_i64 (i64.shl (local.get $t2)
                                             (i64.const 3)))))
         (list 'set "$fp" '(cell-ref (get t1) 0))
         (ds-fp-load "$t5" -1)
         (list 'set "$ac" '(get t0))
         ;; resume where the letcc was, with the restored frame
         (list 'jump-ind '(local.get $p) '(local.get $t5) 0
               '(local.get $t2) '(local.get $fp)))
        8 0))

;;; ---------------------------------------------------------------
;;; host wrappers: a prim referenced as a first-class global
;;; value gets a synthesized eta-expansion proc; its closure is
;;; installed into the global cell at $main entry.
;;; ---------------------------------------------------------------

(define *ds-wrapper-globals* '())   ; alist global id -> wrapper proc

(define *ds-need-atl* #f)           ; apply-to-list unit required
(define *ds-need-argv* #f)          ; %argv-ref emitted: $main must capture argv
(define *ds-binfolds* '())          ; names needing binfold units

;; a DS-convention entry point that is not compiled from AST:
;; a pseudo-proc so known-target calls and closure init work
(define (ds-register-special! gid entry npar rest?)
  (let* ([pids (map (lambda (j)
                      (list 'var (ds-name "s" j) 1 #f))
                    (ds-iota npar))]
         [p (make-proc #f pids
                       (and rest? (list 'var 'srest 1 #f)) #f gid)])
    (proc-entry-set! p entry)
    (set! *ds-wrapper-globals* (cons (cons gid p) *ds-wrapper-globals*))
    (set! *ds-global-procs* (cons (cons gid p) *ds-global-procs*))
    p))

;; tail-return through the frame header (fp-3 code-ref ptr slot,
;; fp-4 saved env, fp-2 saved fp), value in $ac; used by the raw
;; synthesized units below.  Temps $t5/$t6, $p.
(define (ds-raw-tail-return)
  (list
   (list 'load-fn -3)
   (ds-fp-load "$t5" -4)
   (ds-fp-load "$t6" -2)
   '(ret "$t5" "$t6")))

;; load r[fp+k] into temp t; k is a number or a computed MARV form
(define (ds-fp-load t k)
  (list 'set t (list 'marv (slot-load k))))

;; apply-to-list: args are (f list); spread the list over this
;; frame's arg area and tail-call f with the element count.
(define (ds-atl-unit)
  (list '$ds$atl
        (list
         (ds-fp-load "$t0" 0)                   ; f
         '(rt1 ckproc (get t0))
         (ds-fp-load "$t1" 1)                   ; list cursor
         (list 'set "$t2" '(imm 0))             ; count
         ;; walk the list once to count it
         (list 'block '$ds_atl_cdone
               (list
                (list 'loop '$ds_atl_clp
                      (list
                       '(br-if $ds_atl_cdone (marv
                                     (i64.eq (local.get $t1)
                                             (i64.const 13))))
                       '(set "$t1" (rep pair_cdr (local.get $t1)))
                       '(step "$t2" 1)
                       '(br $ds_atl_clp)))))
         (ds-fp-load "$t1" 1)                   ; cursor again
         (list 'set "$t3" '(imm 0))             ; index
         ;; walk it again, storing each element into the arg area
         (list 'block '$ds_atl_xdone
               (list
                (list 'loop '$ds_atl_xlp
                      (list
                       '(br-if $ds_atl_xdone (marv
                                     (i64.eq (local.get $t3)
                                             (local.get $t2))))
                       (list 'marv
                             (slot-store '(local.get $t3)
                                         '($scmn:pair_car (local.get $t1))))
                       '(set "$t1" (rep pair_cdr (local.get $t1)))
                       '(step "$t3" 1)
                       '(br $ds_atl_xlp)))))
         (list 'code-ptr '(get t0))
         (list 'jump-ind '(local.get $p) '(local.get $t0)
               '(i32.wrap_i64 (local.get $t2))
               '(i64.add (local.get $fp) (local.get $t2))
               '(local.get $fp)))
        8 0))

;; ---------------------------------------------------------------
;; iteration host procedures %map1 %map2 %for-each1 %for-each2,
;; plus variadic %append and list.  Entry unit + continuation
;; unit; iteration state lives in root slots beyond the args
;; (fp+K), the element proc is called with the DS non-tail
;; protocol, result arrives in $ac in the continuation unit.
;; ---------------------------------------------------------------
(define *ds-extra-units* '())       ; raw units to emit

;; ds-fps, ds-fpsx, ds-fp-store, ds-fp-load-expr and ds-fp-load-stmt
;; used to live here, each a second spelling of slot-addr, slot-store
;; or slot-load for the hand-written units.  They were separate only
;; because those units built text and these built it differently;
;; slot-addr now takes a computed index, so there is one of each. [cco]

;; a reserve as a bare instruction: the hand-written units manage $sp
;; themselves and do not want ds-reserve's refresh around it
(define (ds-reserve-instr words)
  (list 'reserve (list 'imm-i32 words) (list 'imm-i32 (* 8 words))))

;; non-tail DS-protocol call of the closure in temp tf; args were
;; staged at fp+hbase+4..; header at hbase: env 0, k-ptr, saved fp.
(define (ds-host-call-lines tf cont hbase nargs)
  (list
   (list 'store hbase '(imm 0))
   (list 'fn-store (slot-addr (+ hbase 1)) cont)
   (list 'store (+ hbase 2) '(get fp))
   (list 'rt1 'ckproc (list 'get tf))
   (list 'code-ptr (list 'get tf))
   ;; Not the jump-ind form: that passes the live $ac through, and this
   ;; call deliberately passes nought there instead.  Spelled as data
   ;; rather than given an extra operand, because nothing else wants
   ;; one. [cco]
   (list 'marv
         (append (list (ds-verb-i)
                       '(param i64 i64 i64 i64 ptr ptr i32)
                       '(result ptr)
                       (ds-sp-after (+ hbase 4 nargs))
                       (ds-sp-after (+ hbase 4))
                       (ds-get tf)
                       '(i64.const 0)
                       '(local.get $r) '(local.get $hp))
                 (list (ds-nargs nargs) '(local.get $p))))))

;; (the variadic list constructor, command-line, features,
;; error, exit and jiffies-per-second used to be synthesized
;; here; all are lib/scheme definitions now -- see
;; dsbuild/Checkpoint-17-shadow-libraries.md)

;; values: 1 arg is returned as-is; otherwise build a vector whose
;; the (skint) `error` builtin for libraries that import it from
;; (skint) (SRFI-143/144): report the first argument and exit via
;; $scmrt:error_raise; never returns
(define (ds-error-unit)
  (list '$ds$error
        (list
         (list 'branch '(cmp i32.eq (local.get $nargs) (i32.const 0))
               (list (list 'set "$t1" '(imm 21)))
               (list (ds-fp-load "$t1" 0)))
         '(marv (drop (call $scmrt:error_raise (local.get $t1))))
         '(marv (unreachable)))
        8 0))

;; values: one argument is returned as it is; otherwise build a vector
;; whose element 0 is the runtime's unique values tag.
(define (ds-values-unit)
  (list '$ds$values
        (append
         (list
          (list 'branch '(cmp i32.eq (local.get $nargs) (i32.const 1))
                (list (list 'set "$ac"
                            (list 'marv (slot-load 0))))
                (let ([n '(i64.add (local.get $t0) (i64.const 2))])
                  (list
                   (list 'set "$t0"
                         '(nargs64))
                   ;; This was an unconditional collect on every
                   ;; multi-value call: `hide` in the benchmark harness
                   ;; calls values once per iteration, which forced one
                   ;; full GC per iteration (deriv: 10M collects for
                   ;; 10M iterations).  Guarded now exactly as every
                   ;; other reserve is.
                   (list 'reserve
                         (list 'marv (list 'i32.wrap_i64 n))
                         (list 'marv (list 'i32.wrap_i64
                                           (list 'i64.shl n
                                                 '(i64.const 3)))))
                   (list 'alloc "$t1" "vector"
                         '(marv (i32.add (i32.wrap_i64 (local.get $t0))
                                         (i32.const 1))))
                   '(cell-set (get t1) 0 (rtcall values_tag))
                   (list 'set "$t2" '(imm 0))
                   (list 'block '$ds_vl_done
                         (list
                          (list 'loop '$ds_vl_lp
                                (list
                                 '(br-if $ds_vl_done (marv
                                               (i64.ge_u (local.get $t2)
                                                         (local.get $t0))))
                                 (list 'marv
                                       (list '$scmn:block_set
                                             '(local.get $t1)
                                             '(i32.add (i32.wrap_i64
                                                        (local.get $t2))
                                                       (i32.const 1))
                                             (slot-load '(local.get $t2))))
                                 '(step "$t2" 1)
                                 '(br $ds_vl_lp)))))
                   (list 'set "$ac" '(get t1))))))
         (ds-raw-tail-return))
        8 0))

;; call-with-values: non-tail call of the producer (arg0) through
;; the DS host-call protocol; the continuation unit (which shares
;; this frame) unpacks $ac and tail-calls the receiver (arg1).
(define (ds-cwv-units)
  (list
   (list '$ds$cwv
         (append
          (list (ds-fp-load "$t0" 0))
          (ds-host-call-lines "$t0" '$ds$cwvk 4 0))
         8 0)
   (list '$ds$cwvk
         (append
          (list
           (ds-fp-load "$t0" 1)
           (list 'set "$t1" '(imm 0))
           ;; Is $ac a values tuple?  The block_ref has to be guarded
           ;; by is_vector separately: MARV's i32.and evaluates both
           ;; sides, so the one-expression form dereferenced $ac even
           ;; when it held an immediate (void, a fixnum, ...) -- a
           ;; latent segfault that GCC's optimizer happened to mask by
           ;; eliminating the trapping load, and that Clang executes.
           ;; $t4 holds this flag, and the temps are i64 locals, so the
           ;; flag is stored as i64 and the branch tests it against
           ;; zero.  It used to be stored as (imm-i32 0) and then as a
           ;; bare i64.eq, whose result is i32 -- two i32 values into
           ;; an i64 local, and then an i64 used directly as an `if`
           ;; condition, which wants i32.  Only the wasm target checks
           ;; any of that; C and rv64g lower all three without
           ;; complaint. [cco]
           (list 'set "$t4" '(imm 0))
           (list 'guard '(rep is_vector (local.get $ac))
                 (list (list 'set "$t4"
                             '(marv (i64.extend_i32_u
                                     (i64.eq ($scmn:block_ref
                                              (local.get $ac) (i32.const 0))
                                             (call $scmrt:values_tag)))))))
           (list 'branch '(cmp i64.ne (local.get $t4) (i64.const 0))
                 ;; a tuple: spread its elements over the arg area
                 (list
                  (list 'set "$t3" '(get ac))
                  (list 'set "$t1"
                        '(marv (i64.extend_i32_u
                                (i32.sub ($scmn:block_len (local.get $t3))
                                         (i32.const 1)))))
                  (list 'set "$t2" '(imm 0))
                  (list 'block '$ds_cw_done
                        (list
                         (list 'loop '$ds_cw_lp
                               (list
                                '(br-if $ds_cw_done (marv
                                              (i64.ge_u (local.get $t2)
                                                        (local.get $t1))))
                                (list 'marv
                                      (slot-store
                                       '(local.get $t2)
                                       '($scmn:block_ref
                                         (local.get $t3)
                                         (i32.add (i32.wrap_i64
                                                   (local.get $t2))
                                                  (i32.const 1)))))
                                '(step "$t2" 1)
                                '(br $ds_cw_lp))))))
                 ;; a single value: it is the only argument
                 (list
                  (list 'marv (slot-store 0 '(local.get $ac)))
                  (list 'set "$t1" '(imm 1))))
           (list 'rt1 'ckproc '(get t0))
           (list 'code-ptr '(get t0))
           (list 'jump-ind '(local.get $p) '(local.get $t0)
                 '(i32.wrap_i64 (local.get $t1))
                 '(i64.add (local.get $fp) (local.get $t1))
                 '(local.get $fp)))
          '())
         8 0)))

;; (the open-*-file / current-*-port / vector / make-vector /
;; io / string / string-append units that used to live here are
;; gone for the same reason)

(define (ds-binfold-entry name)
  (ds-name "$ds$bf_"
           (case name [(+) "plus"] [(-) "minus"] [(*) "star"]
                 [(/) "slash"] [(min) "min"] [(max) "max"]
                 [(fl+) "flplus"] [(fl-) "flminus"]
                 [(fl*) "flstar"] [(fl/) "flslash"]
                 [(flmin) "flmin"] [(flmax) "flmax"]
                 [(fx+) "fxplus"] [(fx-) "fxminus"]
                 [(fx*) "fxstar"] [(fxand) "fxand"]
                 [(fxior) "fxior"] [(fxxor) "fxxor"]
                 [(fxeqv) "fxeqv"] [(fxmin) "fxmin"]
                 [(bitwise-and) "bitand"]
                 [(bitwise-ior) "bitior"]
                 [(bitwise-xor) "bitxor"]
                 [else "fxmax"])))

;; variadic first-class wrappers for the arithmetic prims (generic,
;; fx and fl): a name taken as a value is Skint's variadic
;; procedure, so the unit left-folds the argument area, starting
;; from the identity (when there is one); one-argument / fl/ are
;; reciprocals and one-argument - fx- fl- are negations
(define (ds-binfold-unit name)
  (let* ([macro (cdr (assq name *ds-prim-bin*))]
         [need1 (if (memq name '(+ * fl+ fl* fx+ fx* fxand fxior
                                 fxxor fxeqv bitwise-and bitwise-ior
                                 bitwise-xor))
                    #f #t)]
         [ident (case name
                  [(+ fx+ fxior bitwise-ior bitwise-xor) (i64c 3)]
                  [(*) (i64c 7)]
                  [(fx*) (i64c 7)]
                  [(fxand fxeqv bitwise-and) (i64c *ds-word-mask*)]
                  [(fl+) '($scmn:flonum_obj (f64.const 0.0))]
                  [(fl*) '($scmn:flonum_obj (f64.const 1.0))]
                  [else #f])]
         [one-arg (case name
                    [(/ fl/) 'recip]
                    [(- fx- fl-) 'neg]
                    [else #f])]
         [negmacro (case name [(-) 'gneg] [(fx-) 'ineg]
                         [else 'jneg])]
         [lp (ds-binfold-entry name)])
    (list lp
          (append
           ;; with no identity to fall back on, no arguments is an error
           (if need1
               (list (list 'guard
                           '(cmp i32.eq (local.get $nargs) (i32.const 0))
                           (list (list 'arity-error 1) '(halt))))
               '())
           (list
            ;; the accumulator starts from the identity, or from arg0
            (if ident
                (list 'set "$t1" (list 'marv ident))
                (ds-fp-load "$t1" 0))
            (list 'set "$t0" (list 'imm (if ident 0 1))))
           ;; one-argument reciprocal / negation (identity-less names
           ;; only, i.e. the accumulator holds the sole argument)
           (if (and one-arg (not ident))
               (list
                (list 'guard
                      '(cmp i32.eq (local.get $nargs) (i32.const 1))
                      (append
                       (case one-arg
                         [(recip)
                          (append (list (list 'set "$ac" '(imm 7)))
                                  (if (eq? name 'fl/)
                                      (list '(rt itoj)) '())
                                  (list '(push (get t1))
                                        (list 'rt macro)))]
                         [else
                          (list (list 'set "$ac" '(get t1))
                                (list 'rt negmacro))])
                       (list (list 'set "$t1" '(get ac))))))
               '())
           ;; fold the argument area into $t1, index in $t0
           (list
            (list 'block '$ds_bf_done
                  (list
                   (list 'loop '$ds_bf_lp
                         (list
                          '(br-if $ds_bf_done (marv
                                        (i64.ge_u (local.get $t0)
                                                  (i64.extend_i32_u
                                                   (local.get $nargs)))))
                          (ds-reserve-instr 2)
                          (list 'set "$ac" '(get t1))
                          (list 'push (list 'marv
                                            (slot-load '(local.get $t0))))
                          (list 'rt macro)
                          (list 'set "$t1" '(get ac))
                          '(step "$t0" 1)
                          '(br $ds_bf_lp)))))
            (list 'set "$ac" '(get t1)))
           (if ident '() (list (list 'set "$ac" '(get t1))))
           (ds-raw-tail-return))
          8 0)))

(define (ds-sym->prim name)
  ;; (guarded: integrable-global throws on unknown indices)
  (call/cc (lambda (esc)
             (with-exception-handler
              (lambda (e) (esc #f))
              (lambda () (lookup-integrable name))))))

(define (ds-aux-name? name)
  ;; a Skint-internal helper name (%map1, %map2, %map, %for-each...)?
  (let ([s (symbol->string name)])
    (and (> (string-length s) 0)
         (char=? (string-ref s 0) #\%))))

(define (ds-global-prim-name id)
  ;; (prog|lib)://<path>?<name> -> <name>; a plain name stays
  ;; itself.  The name proper may end in ? (boolean=?, flfinite?)
  ;; or itself contain no separator at all, so split at the
  ;; FIRST ? after the "://" (the path has no ? in it), never
  ;; at the last ? in the string.
  (let ([s (symbol->string (id->uname id))])
    (let loop ([i 0])
      (cond [(> (+ i 2) (string-length s)) (string->symbol s)]
            [(and (char=? (string-ref s i) #\:)
                  (char=? (string-ref s (+ i 1)) #\/)
                  (char=? (string-ref s (+ i 2)) #\/))
             (let scan ([j (+ i 3)])
               (cond [(>= j (string-length s)) (string->symbol s)]
                     [(char=? (string-ref s j) #\?)
                      (string->symbol
                       (substring s (+ j 1) (string-length s)))]
                     [else (scan (+ j 1))]))]
            [else (loop (+ i 1))]))))

(define (ds-wrapper-arity name)
  (cond [(memq name '(substring vector-set! string-set!)) 3]
        [(memq name '(memq memv meme assq assv asse
                      list-cat)) 2]
        [(memq name '(symbol->string list->string string-downcase
                      reverse
                      open-input-file open-output-file
                      %open-input-file %open-output-file
                      open-input-string get-output-string
                      open-input-bytevector get-output-bytevector
                      input-port-open? output-port-open?
                      get-environment-variable
                      close-input-port close-output-port close-port
                      flush-output-port read-char peek-char
                      file-exists? delete-file %system
                      make-string number->string %string->list1
                      string->number)) 1]
        [(memq name '(cons set-box!)) 2]
        [(assq name *ds-prim-bin*) 2]
        [(assq name *ds-prim-un*) 1]
        [(ds-cxr-ops name) 1]
        [(memq name '(box display write write-char write-shared)) 1]
        [(memq name '(open-output-bytevector)) 0]
        [else #f]))

(define (ds-make-wrapper! gid name arity)
  (let* ([pids (map (lambda (j)
                      (list 'var (ds-name "w" j) 1 #f))
                    (ds-iota arity))]
         [prim (ds-sym->prim name)]
         [body (primapp-exp '!-effect prim
                            (map (lambda (pid) (var-exp pid)) pids))]
         [p (make-proc #f pids #f body gid)]
         [fr (make-frame p)])
    (proc-frame-set! p fr)
    (proc-def-frames-set! p '())
    (for-each (lambda (pid) (frame-add! fr pid)) pids)
    (set! *ds-wrapper-globals* (cons (cons gid p) *ds-wrapper-globals*))
    (set! *ds-procs* (cons p *ds-procs*))
    (set! *ds-global-procs* (cons (cons gid p) *ds-global-procs*))
    p))

;; name-init-chain-lets (cups-passes) turns init-spine lets into
;; assignments to generated tmpname#<n> globals nested inside begin
;; forms, so they never appear in the top-level defined-globals
;; list; they are genuinely defined (the assignment precedes the
;; single call, which goes indirect through the global slot)
(define (ds-generated-global? gid)
  (let ([s (symbol->string (id->uname gid))])
    (and (>= (string-length s) 8)
         (string=? (substring s 0 8) "tmpname#"))))

(define (ds-synthesize-wrappers! defined-globals)
  (for-each
   (lambda (pr)
     (let ([gid (car pr)])
       (unless (or (memq gid defined-globals)
                   (ds-generated-global? gid)
                   (assq gid *ds-global-procs*))
         (let ([name (ds-global-prim-name gid)])
           ;; only genuine prims are materialized here: the values
           ;; protocol, apply-to-list, variadic generic arithmetic,
           ;; and fixed-arity prim eta-wrappers (else branch).
           ;; every library-level name (error, exit, features,
           ;; command-line, vector, string-append, ...) is defined
           ;; by lib/scheme/*.sld; an unresolved one is a loud
           ;; error, never a synthesized host procedure
           (cond [(eq? name 'error)
                  (set! *ds-extra-units*
                        (cons (ds-error-unit) *ds-extra-units*))
                  (ds-register-special! gid '$ds$error 0 #t)]
                 [(eq? name 'values)
                  (set! *ds-extra-units*
                        (cons (ds-values-unit) *ds-extra-units*))
                  (ds-register-special! gid '$ds$values 0 #t)]
                 [(eq? name 'call-with-values)
                  (set! *ds-extra-units*
                        (append (ds-cwv-units) *ds-extra-units*))
                  (ds-register-special! gid '$ds$cwv 2 #f)]
                 [(eq? name 'apply-to-list)
                  (set! *ds-need-atl* #t)
                  (ds-register-special! gid '$ds$atl 2 #f)]
                 [(memq name '(+ - * / min max
                               fl+ fl- fl* fl/ flmin flmax
                               fx+ fx- fx* fxand fxior fxxor fxeqv
                               fxmin fxmax
                               bitwise-and bitwise-ior bitwise-xor))
                  (set! *ds-binfolds* (cons name *ds-binfolds*))
                  (ds-register-special! gid (ds-binfold-entry name) 0 #t)]
                 [else
                  (let ([arity (and name (ds-wrapper-arity name))])
                    (if (and arity (ds-sym->prim name))
                        (ds-make-wrapper! gid name arity)
                        (ds-die
                         "global defined by no library and not a DS prim"
                         name (id->uname gid))))])))))
   ;; after shaking, only globals referenced from kept code can need
   ;; wrappers; before shaking (no shake yet), consider all of them
   (if *ds-referenced-globals*
       (ds-filter (lambda (pr) (memq (car pr) *ds-referenced-globals*))
                  *ds-globals*)
       *ds-globals*)))

;;; ---------------------------------------------------------------
;;; program assembly + module emission
;;; ---------------------------------------------------------------

;;; ---------------------------------------------------------------
;;; shaking: reachability-based dead-definition elimination.
;;;
;;; Shadow libraries (lib/scheme/*.sld) contribute dozens of global
;;; definitions per library, but a compiled program typically uses a
;;; handful.  Compiling every definition would require DS prims for
;;; every builtin mentioned in any library body, so unreachable
;;; global lambda definitions are dropped before codegen.
;;; Roots are all top-level forms that are not global lambda defines
;;; (program expressions, non-lambda defines, library init side
;;; effects); reachability flows through proc bodies (free globals)
;;; and through nested lambda nodes.  Sets *ds-referenced-globals*
;;; (every global referenced from kept code) and
;;; *ds-kept-defined-globals* for the wrapper-synthesis phase.
;;; ---------------------------------------------------------------

(define *ds-referenced-globals* #f)
(define *ds-kept-defined-globals* '())

(define (ds-filter p xs)
  (cond [(null? xs) '()]
        [(p (car xs)) (cons (car xs) (ds-filter p (cdr xs)))]
        [else (ds-filter p (cdr xs))]))

(define (ds-free-globals exp)
  (ds-filter global-id? (exp->free-vars exp)))

(define (ds-lambda-nodes exp)
  (variant-case exp
    [lambda-exp (ids body) (cons exp (ds-lambda-nodes body))]
    [ilambda-exp (ids id body) (cons exp (ds-lambda-nodes body))]
    [lambda*-exp (arities lambdas)
     (apply append (map ds-lambda-nodes lambdas))]
    [varassign-exp (id exp) (ds-lambda-nodes exp)]
    [if-exp (test-exp then-exp else-exp)
     (append (ds-lambda-nodes test-exp) (ds-lambda-nodes then-exp)
             (ds-lambda-nodes else-exp))]
    [primapp-exp (effect prim rands)
     (apply append (map ds-lambda-nodes rands))]
    [app-exp (rator rands)
     (apply append (map ds-lambda-nodes (cons rator rands)))]
    [once-exp (id exp) (ds-lambda-nodes exp)]
    [fix-exp (ids lams body)
     (apply append (map ds-lambda-nodes (cons body lams)))]
    [letcc-exp (id body) (ds-lambda-nodes body)]
    [withcc-exp (cont-exp exp)
     (append (ds-lambda-nodes cont-exp) (ds-lambda-nodes exp))]
    [else '()]))

;; A top-level definition whose global nobody wants has three possible
;; fates, not two: see the three-way decision at the end of ds-shake!.
(define (ds-def-removable? f)
  (and (varassign-exp? f)
       (exp-removable-if-dead? (varassign-exp->exp f))))

(define (ds-shake! main forms)
  (define (def-gid f)
    (and (varassign-exp? f)
         (global-id? (varassign-exp->id f))
         (varassign-exp->id f)))
  (define (lambda-def-gid f)
    (and (varassign-exp? f)
         (global-id? (varassign-exp->id f))
         (or (lambda-exp? (varassign-exp->exp f))
             (ilambda-exp? (varassign-exp->exp f)))
         (varassign-exp->id f)))
  ;; non-lambda define exps, scanned for references when kept
  (define nonlambda-defs
    (let loop ([fs forms] [al '()])
      (cond [(null? fs) al]
            [(and (def-gid (car fs)) (not (lambda-def-gid (car fs))))
             (loop (cdr fs)
                   (cons (cons (def-gid (car fs))
                               (varassign-exp->exp (car fs))) al))]
            [else (loop (cdr fs) al)])))
  ;; parent -> child procs, from scan-time def-frames
  (define children
    (let loop ([ps *ds-procs*] [al '()])
      (cond [(null? ps) al]
            [(and (not (eq? (car ps) main))
                  (pair? (proc-def-frames (car ps))))
             (let* ([par (frame-proc (car (proc-def-frames (car ps))))]
                    [pr (assq par al)])
               (if pr
                   (begin (set-cdr! pr (cons (car ps) (cdr pr)))
                          (loop (cdr ps) al))
                   (loop (cdr ps) (cons (cons par (list (car ps))) al))))]
            [else (loop (cdr ps) al)])))
  ;; A top-level lambda define is scanned twice: once as the global
  ;; proc (direct calls) and once as a plain node proc (the closure
  ;; main creates and stores in the global).  The pair is kept or
  ;; dropped together; nested lambdas resolve (via *ds-node-procs*)
  ;; to the twin's subtree, so a kept global proc's own children are
  ;; dead duplicates.
  (define node->gid
    (let loop ([fs forms] [al '()])
      (cond [(null? fs) al]
            [(lambda-def-gid (car fs)) =>
             (lambda (g) (loop (cdr fs)
                                (cons (cons (varassign-exp->exp (car fs)) g)
                                      al)))]
            [else (loop (cdr fs) al)])))
  (define twin-of-gidproc '()) ; alist gid-proc -> twin proc
  (define twin-procs '())
  (for-each
   (lambda (ng)
     (cond [(assq (car ng) *ds-node-procs*) =>
            (lambda (np)
              (cond [(assq (cdr ng) *ds-global-procs*) =>
                     (lambda (gp)
                       (set! twin-of-gidproc
                             (cons (cons (cdr gp) (cdr np)) twin-of-gidproc))
                       (set! twin-procs (cons (cdr np) twin-procs)))]))]))
   node->gid)
  (define all-define-gids (map def-gid (ds-filter def-gid forms)))
  (let ([kept-gids '()] [kept-procs '()] [referenced '()] [defined '()]
        [work '()])
    (define (keep-proc! p)
      (unless (memq p kept-procs)
        (set! kept-procs (cons p kept-procs))
        (set! work (cons p work))))
    (define (keep-node! node)
      (cond [(assq node *ds-node-procs*) =>
             (lambda (pr) (keep-proc! (cdr pr)))]))
    (define (keep-gid! g)
      (unless (memq g referenced) (set! referenced (cons g referenced)))
      (unless (memq g kept-gids)
        (set! kept-gids (cons g kept-gids))
        (when (memq g all-define-gids) (set! defined (cons g defined)))
        (cond [(assq g *ds-global-procs*) =>
               (lambda (pr)
                 (keep-proc! (cdr pr))
                 (cond [(assq (cdr pr) twin-of-gidproc) =>
                        (lambda (tp) (keep-proc! (cdr tp)))]))]
              [(assq g nonlambda-defs) =>
               (lambda (pr)
                 ;; a gid may be defined by several forms (a
                 ;; library body and its included polyfill can
                 ;; both define it; the later store wins at
                 ;; runtime) -- keep the lambda nodes and free
                 ;; globals of every defining form, not just the
                 ;; first in the alist
                 (for-each
                  (lambda (d)
                    (when (eq? (car d) g)
                      (for-each keep-gid! (ds-free-globals (cdr d)))
                      (for-each keep-node!
                                (ds-lambda-nodes (cdr d)))))
                  nonlambda-defs))]
              ;; an undefined Skint-auxiliary global (%map1 &c)
              ;; with a same-named shadow definition: keep that
              ;; definition so the aliasing pass in
              ;; ds-compile-program can resolve this gid to it
              [(and (ds-aux-name? (ds-global-prim-name g))
                    (let loop ([gs all-define-gids])
                      (cond [(null? gs) #f]
                            [(eq? (ds-global-prim-name (car gs))
                                  (ds-global-prim-name g))
                             (car gs)]
                            [else (loop (cdr gs))])))
               => keep-gid!])))
    (define (scan-body! p)
      (for-each keep-gid! (ds-free-globals (proc-body p)))
      (unless (assq p twin-of-gidproc)
        (cond [(assq p children) =>
               (lambda (pr) (for-each keep-proc! (cdr pr)))])))
    (keep-proc! main)
    ;; seed: free globals of root forms (non-define top-level forms:
    ;; program expressions and library init side effects)
    (for-each
     (lambda (f)
       (when (or (not (def-gid f)) (not (ds-def-removable? f)))
         (for-each keep-gid! (ds-free-globals f))
         (for-each keep-node! (ds-lambda-nodes f))))
     forms)
    (let loop ()
      (unless (null? work)
        (let ([p (car work)])
          (set! work (cdr work))
          (unless (eq? p main) (scan-body! p))
          (loop))))
    (when (get-environment-variable "DS_DEBUG")
      (display (list 'kept-gids (map id->uname kept-gids)) (current-error-port))
      (newline (current-error-port))
      (display (list 'shake 'forms (length forms)
                     'defs (length defined) 'kept-procs (length kept-procs)
                     'kept-gids (length kept-gids)
                     'procs-total (length *ds-procs*))
               (current-error-port))
      (newline (current-error-port)))
    (set! *ds-procs* (ds-filter (lambda (p) (memq p kept-procs)) *ds-procs*))
    (set! *ds-global-procs*
          (ds-filter (lambda (pr) (memq (car pr) kept-gids))
                     *ds-global-procs*))
    (set! *ds-referenced-globals* referenced)
    (set! *ds-kept-defined-globals* defined)
    ;; Three outcomes for a top-level definition.  The global is wanted, so
    ;; the form stays as it is.  It is not wanted and its right-hand side
    ;; cannot be observed, so the form goes.  It is not wanted but its
    ;; right-hand side can be observed, so the evaluation stays as a
    ;; statement and only the store and the global slot go: dropping the
    ;; store is what the shakedown is entitled to, dropping the effect is
    ;; not.  [cco]
    (let loop ([fs forms] [acc '()])
      (cond [(null? fs) (reverse acc)]
            [(let ([g (def-gid (car fs))])
               (and g (not (memq g kept-gids))))
             (if (ds-def-removable? (car fs))
                 (loop (cdr fs) acc)
                 (loop (cdr fs)
                       (cons (varassign-exp->exp (car fs)) acc)))]
            [else (loop (cdr fs) (cons (car fs) acc))]))))

(define (ds-compile-program forms modname)
  (set! *ds-procs* '())
  (set! *ds-lit-found* '())
  (set! *ds-node-procs* '())
  (set! *ds-globals* '())
  (set! *ds-global-procs* '())
  (set! *ds-assigned-globals* '())
  (set! *ds-need-throw* #f)
  (set! *ds-need-atl* #f)
  (set! *ds-need-argv* #f)
  (set! *ds-binfolds* '())
  (set! *ds-extra-units* '())
  (set! *ds-proc-seq* 0)
  (set! *ds-units* '())
  (set! *ds-unit-seq* 0)
  (set! *ds-cap-seq* 0)
  (set! *ds-wrapper-globals* '())
  (set! *ds-referenced-globals* #f)
  (set! *ds-kept-defined-globals* '())
  ;; main proc: body = the top-level forms themselves
  (let* ([main (make-proc "main" '() #f forms #f)]
         [mfr (make-frame main)]
         [defined-globals
          (let loop ([fs forms] [acc '()])
            (cond [(null? fs) acc]
                  [(and (varassign-exp? (car fs))
                        (global-id? (varassign-exp->id (car fs))))
                   (loop (cdr fs) (cons (varassign-exp->id (car fs)) acc))]
                  [else (loop (cdr fs) acc)]))])
    (proc-frame-set! main mfr)
    (set! *ds-main-proc* main)
    (set! *ds-procs* (list main))
    ;; scan top forms (lambda definitions of globals get gids)
    (let ([mscope (list mfr)])
      (for-each
       (lambda (f)
         (cond [(and (varassign-exp? f) (global-id? (varassign-exp->id f))
                     (lambda-exp? (varassign-exp->exp f)))
                (ds-scan-lambda (varassign-exp->exp f)
                                (lambda-exp->ids (varassign-exp->exp f)) #f
                                (lambda-exp->body (varassign-exp->exp f))
                                (varassign-exp->id f) mscope)
                ;; the store itself is generated as a statement
                (ds-scan-exp (varassign-exp->exp f) mscope)]
               [(and (varassign-exp? f) (global-id? (varassign-exp->id f))
                     (ilambda-exp? (varassign-exp->exp f)))
                (ds-scan-lambda (varassign-exp->exp f)
                                (ilambda-exp->ids (varassign-exp->exp f))
                                (ilambda-exp->id (varassign-exp->exp f))
                                (ilambda-exp->body (varassign-exp->exp f))
                                (varassign-exp->id f) mscope)
                (ds-scan-exp (varassign-exp->exp f) mscope)]
               [else (ds-scan-exp f mscope)]))
       forms))
    ;; def-frames for nested procs were recorded during scan;
    ;; main's def-frames are empty
    (proc-def-frames-set! main '())
    ;; drop unreachable global definitions (shadow libraries)
    (let ([new-forms (ds-shake! main forms)])
      (vector-set! main 4 new-forms)
      (set! defined-globals *ds-kept-defined-globals*))
    ;; aux aliasing: an undefined Skint-auxiliary global (%map1 &c)
    ;; whose base name is defined by a shadow library lambda
    ;; (lib/scheme/base.sld's own %map1/%for-each family) becomes
    ;; an alias of that proc (calls resolve directly; value
    ;; references get the same closure installed into their own
    ;; slot).
    (begin
      (let ([pdefs (let loop ([gs defined-globals] [acc '()])
                     (cond [(null? gs) acc]
                           [(and (ds-aux-name? (ds-global-prim-name (car gs)))
                                 (assq (car gs) *ds-global-procs*))
                            (loop (cdr gs) (cons (car gs) acc))]
                           [else (loop (cdr gs) acc)]))])
        (for-each
         (lambda (pr)
           (let ([gid (car pr)])
             (unless (or (not (ds-aux-name? (ds-global-prim-name gid)))
                         (memq gid defined-globals)
                         (assq gid *ds-global-procs*))
               (let ([nm (ds-global-prim-name gid)])
                 (let loop ([ds pdefs])
                   (unless (null? ds)
                     (if (eq? (ds-global-prim-name (car ds)) nm)
                         (let ([pp (cdr (assq (car ds)
                                              *ds-global-procs*))])
                           (set! *ds-global-procs*
                                 (cons (cons gid pp) *ds-global-procs*))
                           (set! *ds-wrapper-globals*
                                 (cons (cons gid pp)
                                       *ds-wrapper-globals*)))
                         (loop (cdr ds)))))))))
         *ds-globals*)))
    ;; host wrappers for prims referenced as values
    (ds-synthesize-wrappers! defined-globals)
    (when (get-environment-variable "DS_DEBUG_WRAPPERS")
      (display "DS-WRAP wrappers:" (current-error-port))
      (for-each (lambda (pr)
                  (display " " (current-error-port))
                  (display (ds-global-prim-name (car pr))
                           (current-error-port)))
                *ds-wrapper-globals*)
      (display " binfolds:" (current-error-port))
      (for-each (lambda (nm)
                  (display " " (current-error-port))
                  (display nm (current-error-port)))
                *ds-binfolds*)
      (display " extra:" (current-error-port))
      (for-each (lambda (u)
                  (display " " (current-error-port))
                  (display (car u) (current-error-port)))
                *ds-extra-units*)
      (display (if *ds-need-atl* " atl" "") (current-error-port))
      (newline (current-error-port)))
    ;; finalize all frames
    (for-each (lambda (p)
                (proc-ncells-set! p (ds-finalize-frame! (proc-frame p))))
              *ds-procs*)
    ;; literals over all bodies
    (for-each (lambda (p) (ds-collect-body-literals! (proc-body p)))
              *ds-procs*)
    ;; code: names/entries first, so calls can reference procs
    ;; whose bodies are compiled later
    (let ([ps (reverse *ds-procs*)])
      (for-each (lambda (p)
                  (set! *ds-proc-seq* (+ *ds-proc-seq* 1))
                  (proc-name-set! p (ds-name "p" *ds-proc-seq*))
                  (proc-entry-set! p
                                   (ds-name "$ds$" (proc-name p))))
                ps)
      (for-each ds-compile-proc ps))
    (when *ds-need-throw*
      (set! *ds-units* (cons (ds-throw-unit) *ds-units*)))
    (when *ds-need-atl*
      (set! *ds-units* (cons (ds-atl-unit) *ds-units*)))
    (for-each (lambda (nm)
                (set! *ds-units* (cons (ds-binfold-unit nm) *ds-units*)))
              *ds-binfolds*)
    (for-each (lambda (u)
                (set! *ds-units* (cons u *ds-units*)))
              *ds-extra-units*)
    *ds-main-proc*))

;; The ABI every instruction has: eight parameters and one result.
(define (ds-abi-signature)
  (list '(param $sp i64) '(param $fp i64) '(param $env i64)
        '(param $ac i64) '(param $r ptr) '(param $hp ptr)
        '(param $nargs i32) '(result ptr)))

;; One unit as a MARV func.  The locals are whatever temps the unit
;; allocated, with a floor of eight because the hand-written units use
;; up to that many.
(define (ds-unit-func u)
  (append (list 'func (car u))
          (ds-abi-signature)
          (let loop ([i 0] [acc '()])
            (if (>= i (max 8 (caddr u)))
                (reverse acc)
                (loop (+ i 1)
                      (cons (list 'local (ds-name "$t" i) 'i64) acc))))
          (list '(local $p ptr))
          (ds-il-marv (cadr u))))

;; the call that enters the program, from $main or from the trampoline
(define (ds-enter-call target)
  (list 'call target '(i64.const 4) '(i64.const 4) '(i64.const 0)
        '(i64.const 0) '(local.get $r) '(local.get $hp) '(i32.const 0)))

(define (ds-main-func canon)
  (append
   (list 'func '$main '(export "main")
         '(param $argc i32) '(param $argv ptr) '(result i32)
         '(local $hp ptr) '(local $r ptr)
         '(local.set $r (ref.data $roots))
         (list 'local.set '$hp
               (list 'call '$scmrt:init '(i32.const 131072)
                     '(i32.const 1024)
                     (list 'i32.const (ds-static-words-needed)))))
   ;; argv capture only when the %argv-ref prim was emitted: programs
   ;; that never look at the command line link against runtimes
   ;; without argv support too
   (if *ds-need-argv*
       (list '(call $scmrt:set_argv (local.get $argc) (local.get $argv)))
       '())
   (if (pair? *ds-globals*)
       (list (list 'call '$scmrt:register_roots_range
                   '(ref.data $gstore)
                   (list 'i32.const (length *ds-globals*))))
       '())
   (if (pair? *ds-lit-inits*)
       (list (list 'local.set '$hp
                   (list 'call '$scmrt:deserialize_all
                         '(ref.data $lits) '(ref.data $littab)
                         (list 'i32.const (length *ds-lit-inits*))
                         '(ref.data $gstore) '(local.get $r)
                         '(local.get $hp))))
       '())
   (list '(ptr.store offset 8 (ref.data $roots) (ref.func $ds_halt))
         (list 'drop (ds-enter-call
                      (if *ds-trampoline?* '$ds_run
                          (canon (proc-entry *ds-main-proc*)))))
         '(return (i32.const 0)))))

;; The banner is a comment, not MARV, so it is the one thing here still
;; written as text.  It is a list of strings rather than output, so a
;; caller asking for the module as data gets no stray printing.
(define (ds-module-banner)
  (append
   (list ";;; generated by (skint compile) (DS-MARV, direct style; no CPS)."
         ";;; Every instruction is a MARV func of one signature, chained")
   (if *ds-trampoline?*
       (list ";;; within a procedure by return_call; calls and returns are"
             ";;; trampoline_jump/_indirect through $ds_run.")
       (list ";;; within a procedure by return_call; calls and returns are"
             ";;; by return_call/return_call_indirect."))))

;; The whole program as MARV data: the include forms and the module.
;; This is what compile-program returns when its OUTPUT argument is #f,
;; and the reason that mode costs nothing -- the module was already
;; built as one form before anything was printed. [cco]
;; The MARV module name is FIXED, and deliberately not derived from the
;; program.s filename.  MARV.s own libraries are unprefixed -- math,
;; string, stdio, time, stdlib and the rest under its lib/ -- so a
;; program called math.scm would have emitted (module $math) against a
;; system library of that name.  Everything this compiler emits already
;; carries the $ds$ or $ds_ prefix, so a module name in that family
;; cannot collide with MARV.s names or with the runtime.s $scm ones.
;; The program is linked against the same library set every time, so
;; the name carries no information worth risking a clash for.
;;
;; modname is still wanted, but only for the expander.s global-name
;; prefix (prog://NAME?ident), which is what error messages show. [cco]
(define $ds-module-name '$ds$program)

(define (ds-build-module modname)
  (let* ([units (reverse *ds-units*)]
         [cmap (if *ds-dedup?* (ds-canon-map units)
                   (map (lambda (u) (cons (car u) (car u))) units))]
         [canon (lambda (nm)
                  (cond [(assoc nm cmap) => cdr] [else nm]))]
         [kept (let loop ([us units] [seen '()] [acc '()])
                 (cond [(null? us) (reverse acc)]
                       [(member (canon (caar us)) seen)
                        (loop (cdr us) seen acc)]
                       [else (loop (cdr us)
                                   (cons (canon (caar us)) seen)
                                   (cons (car us) acc))]))])
    (set! *ds-stats-units* (length units))
    (set! *ds-stats-kept* (length kept))
    (list
     '(include system $scmrt)
     '(include $scmn)
     '(include system $scmi)
     (append
      (list 'module $ds-module-name)
      ;; Skint grows its register/stack region with realloc (cxm_rgc in
      ;; k.c); the DS root stack is a static segment, so give it
      ;; comparable headroom: 64 MiB of BSS (pages are only touched as
      ;; the stack actually grows).  1 MiB was overflowed by deep
      ;; non-tail recursion (ack, conform).
      (list '(data $roots var align 8 (size 67108864))
            (list 'data '$gstore 'var 'align 8
                  (list 'size (* 8 (max 1 (length *ds-globals*))))))
      ;; the blob is a MARV string literal, so it travels as its payload
      ;; and the writer escapes it; the startup table is bytes, so it
      ;; travels as a bytevector
      (if (pair? *ds-lit-inits*)
          (list (list 'data '$lits 'const 'align 1 *ds-lit-blob*)
                (list 'data '$littab 'const 'align 4
                      (ds-littab-bytes (ds-littab-string))))
          '())
      (map (lambda (u)
             (ds-unit-func
              (list (canon (car u))
                    (ds-rename-instrs (cadr u) canon)
                    (caddr u) (cadddr u))))
           kept)
      (list (append (list 'func '$ds_halt) (ds-abi-signature)
                    (list '(return (ptr.null)))))
      (if *ds-trampoline?*
          (list (append (list 'trampoline_func '$ds_run)
                        (ds-abi-signature)
                        (list (cons 'return_call
                                    (cdr (ds-enter-call
                                          (canon (proc-entry
                                                  *ds-main-proc*))))))))
          '())
      (list (ds-main-func canon))))))

;; Print what ds-build-module built, to a port.
(define (ds-write-module forms port)
  (set! *ds-op* port)
  (for-each ds-out (ds-module-banner))
  (for-each (lambda (f) (marv-print f port 0)) forms))

(define *ds-stats-units* 0)
(define *ds-stats-kept* 0)

;;; ---------------------------------------------------------------
;;; the public entry point
;;; ---------------------------------------------------------------

;; Optimization level, an exact integer 0..3.  Level 1 is the default
;; and means what the compiler does today, so that packaging changed no
;; behaviour; the other levels are accepted and validated but not yet
;; interpreted.  Do not invent meanings for them before there is
;; something to attach. [cco]
(define cp-optimize
  (make-parameter 1
    (lambda (v)
      (unless (and (exact-integer? v) (<= 0 v 3))
        (error "cp-optimize: not an optimization level 0..3" v))
      v)))

;; EVERY per-compile global, reset together.  This is what a library
;; needs and a one-shot program did not: without it a second call
;; accumulates the first program's units, globals and literals, which
;; would pass any test that compiles once per process and fail the
;; moment anyone compiles twice.  Config that the caller sets --
;; *ds-balance*, *ds-dedup?*, *ds-trampoline?*, *ds-features* -- is
;; deliberately NOT reset here. [cco]
(define (ds-reset!)
  (set! *ds-op* #f)
  (set! *ds-procs* '())
  (set! *ds-node-procs* '())
  (set! *ds-main-proc* #f)
  (set! *ds-globals* '())
  (set! *ds-global-procs* '())
  (set! *ds-assigned-globals* '())
  (set! *ds-need-throw* #f)
  (set! *ds-proc-seq* 0)
  (set! *ds-lit-inits* '())
  (set! *ds-lit-blob* "")
  (set! *ds-lit-found* '())
  (set! *ds-units* '())
  (set! *ds-unit-seq* 0)
  (set! *ds-cur-name* #f)
  (set! *ds-cur-lines* '())
  (set! *ds-cur-maxlocal* 0)
  (set! *ds-cur-startdepth* 0)
  (set! *ds-cur-proc* #f)
  (set! *ds-slot* 0)
  (set! *ds-scope* '())
  (set! *ds-cost* 0)
  (set! *ds-capturing* 0)
  (set! *ds-sp-dynamic* #f)
  (set! *ds-inline* 0)
  (set! *ds-cap-seq* 0)
  (set! *ds-wrapper-globals* '())
  (set! *ds-need-atl* #f)
  (set! *ds-need-argv* #f)
  (set! *ds-binfolds* '())
  (set! *ds-extra-units* '())
  (set! *ds-stats-units* 0)
  (set! *ds-stats-kept* 0))

;; parameterize over a list of PARAM VAL PARAM VAL ..., so that an
;; explicit argument and a surrounding parameterize share one mechanism
;; and the explicit one wins by being innermost
(define (ds-call-with-params pairs thunk)
  (cond [(null? pairs) (thunk)]
        [(null? (cdr pairs))
         (error "compile-program: a parameter without a value" (car pairs))]
        [else
         (let ([p (car pairs)] [v (cadr pairs)])
           (unless (procedure? p)
             (error "compile-program: not a parameter" p))
           (parameterize ([p v])
             (ds-call-with-params (cddr pairs) thunk)))]))

;; (compile-program INPUT [OUTPUT] PARAM VAL ...)
;;
;; INPUT   a filename, an input port, or a list of forms.
;; OUTPUT  a filename, an output port, #t (the current output port,
;;         the default) or #f (return the module as an S-expression).
;; PARAM VAL ...  parameters and their values; an explicit one
;;         overrides the parameter's current value.
;;
;; INPUT and OUTPUT filenames are used as given, with no manipulation,
;; so they resolve against the working directory like any other tool's
;; arguments.  The compiler's own resources are addressed from its
;; captured base path instead -- never from here. [cco]
(define (compile-program input . rest)
  ;; OUTPUT is omitted exactly when the first optional argument is a
  ;; parameter, which is a procedure; no output designator is one.
  (let* ([omitted? (and (pair? rest) (procedure? (car rest)))]
         [output (if (or (null? rest) omitted?) #t (car rest))]
         [pairs (if (or (null? rest) omitted?) rest (cdr rest))])
    (ds-call-with-params
     pairs
     (lambda ()
       (ds-reset!)
       (let* ([modname (if (string? input)
                           (program-module-name input)
                           'program)]
              [core (expand-program-files (list input) modname)]
              [forms (flatten-begin (ds-compile-core core modname))])
         (ds-compile-program forms modname)
         (let ([marv (ds-build-module modname)])
           (cond [(eq? output #f) marv]
                 [(eq? output #t)
                  (ds-write-module marv (current-output-port))]
                 [(string? output)
                  (call-with-output-file output
                    (lambda (op) (ds-write-module marv op)))]
                 [else (ds-write-module marv output)])))))))
