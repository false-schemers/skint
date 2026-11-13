;;; SPDX-FileCopyrightText: 2024 Sergei Egorov
;;; SPDX-License-Identifier: MIT

(define-library (srfi 257)
  (import (scheme base))
  (export
    ; aux keywords re-exported from (scheme base)
    _ ... => quote quasiquote unquote unquote-splicing
    match
    ~value
    ~cons ~list ~append ~append/ng ~append/t
    ~etc ~etcse ; et cetera sine errore
    ~vector ~vector-append ~vector-append/ng
    ~string ~string-append ~string-append/ng
    ~vector->list ~string->list ~list->vector
    ~list->string ~string->symbol ~symbol->string
    ~string->number ~number->string
    ~null? ~pair? ~list? ~boolean? ~number?
    ~integer? ~vector? ~string? ~symbol? ~char?
    ~and ~or ~not ~cut!
    ~list* ~list-no-order ~list-no-order* ~= ~?
    ~prop ~test ~iterate
    ~if-id-member ~replace-specials
    define-match-pattern define-record-match-pattern
    value etc)


(begin
; The matcher as implememted here is quite light on optimization and error checking.
; Support for #& box read syntax is commented-out, but, if uncommented, works
; on systems supporting both features (e.g. Chez Scheme). Box-related patterns are
; conditionally available in the 'box' sublibrary.


;
; 1) helper macros, used throughout all parts of the implementation
;

; (classify x if-ellipsis if-underscore if-var if-other)
; returns one of four expressions based on type of x, in 4 steps
; Should work on R7RS, R6RS, and earlier syntax-rules expanders

(define-syntax classify
  (syntax-rules ()
    ((_ (x . y) ke ku kv kf) kf)
    ((_ #(x ...) ke ku kv kf) kf)
    ;((_ #&x ke ku kv kf) kf)
    ((_ atom ke ku kv kf) ; ellipsis test after Taylor Campbell
     (let-syntax ((e? (syntax-rules () ((_ (x atom) t f) t) ((_ x t f) f))))
       (e? (1 2 3) ke (classify-nonellipsis-atom atom ku kv kf))))))

(define-syntax classify-nonellipsis-atom
  (syntax-rules ()
    ((_ atom ku kv kf) ; symbol test after Oleg Kiselyof
     (let-syntax ((s? (syntax-rules () ((_ atom t f) t) ((_ x t f) f))))
       (s? abracadabra (classify-nonellipsis-symbol atom ku kv) kf)))))

(define-syntax classify-nonellipsis-symbol
  (syntax-rules ()
    ((_ symbol ku kv) ; see if symbol acts like a variable
     (let-syntax ((b (syntax-rules () ((_ symbol k t f) (k symbol t f))))
                  (k (syntax-rules () ((_ () t f) t) ((_ x t f) f))))
       (b () k (classify-nonellipsis-var symbol ku kv) ku)))))

(define-syntax classify-nonellipsis-var
  (syntax-rules ()
    ((_ var ku kv) ; needed for R4RS/R5RS systems where _ is a regular var
     (let-syntax ((u? (syntax-rules (var) ((_ var t f) t) ((_ x t f) f))))
       (u? _ ku kv)))))

; check if v is not among vars in (a ...) list, using free-id= rules for comparison
; NB: neither v, nor any of a-s should be ellipsis (...) or underscore (_) identifiers

(define-syntax if-new-var
  (syntax-rules ()
    ((_ v (a ...) kt kf)
     (let-syntax ((ck (syntax-rules (a ...) ((_ a) kf) ... ((_ x) kt)))) (ck v)))))

; calculate set minus on two set of vars, using free-id= rules for comparison
; NB: neither set should contain ellipsis (...) and underscore (_) identifiers

(define-syntax varset-minus
  (syntax-rules () ; (_ v* mv* k () . a*) ==> (k v-mv* . a*)
    ((_ () mv* k rv* . a*) (k rv* . a*))
    ((_ (v . v*) mv* k rv* . a*)
     (if-new-var v mv*
       (varset-minus v* mv* k (v . rv*) . a*)
       (varset-minus v* mv* k rv* . a*)))))

; (replace-specials ell und sexp . k) invokes k with reassembled sexp,
; replacing ... with ell and _ with und (_ and ... are recognized according to the free-id= rules)

(define-syntax replace-specials
  (syntax-rules ()
    ((_ ell und sexp . cont)
     (letrec-syntax
       ((subx
         (syntax-rules ()
           ((_ ev uv () k . a*) (k () . a*))
           ((_ ev uv (x . y)  k . a*) (subx ev uv x subcdr ev uv y k . a*))
           ;((_ ev uv #&x k . a*) (subx ev uv x rebox k . a*))
           ((_ ev uv #(x (... ...)) k . a*) (subx ev uv (x (... ...)) revec k . a*))
           ((_ ev uv x k . a*) (classify x (k ev . a*) (k uv . a*) (k x . a*) (k x . a*)))))
        (subcdr
         (syntax-rules ()
           ((_ sx ev uv y k . a*) (subx ev uv y repair sx k . a*))))
        (repair (syntax-rules () ((_ sy sx k . a*) (k (sx . sy) . a*))))
        ;(rebox  (syntax-rules () ((_ sx k . a*) (k #&sx . a*))))
        (revec  (syntax-rules () ((_ (sx (... ...)) k . a*) (k #(sx (... ...)) . a*)))))
       (subx ell und sexp . cont)))))

; Petrofsky's extraction (after Al* Petrofsky via Oleg Kiselyof)

(define-syntax extract
  (syntax-rules ()
    ((_ _id _x _c)
     (letrec-syntax
       ((tr (syntax-rules (_id)
              ((_ x _id tail (k il . a*)) (k (x . il) . a*))
              ((_ d (x . y) tail c) (tr x x (y . tail) c))
              ((_ d1 d2 () (k il . a*)) (k (_id . il) . a*))
              ((_ d1 d2 (x . y) c) (tr x x y c)))))
       (tr _x _x () _c)))))

(define-syntax extract*
  (syntax-rules ()
    ((_ id* x c) (extract* () id* x c))
    ((_ il () x (k () . a*)) (k il . a*))
    ((_ il (id . id*) x c)
     (extract id x (extract* il id* x c)))))


;
; 2) base matcher macro framework/protocol
;

(define-syntax match
  (syntax-rules ()
    ((_ exp clause ...)
     (let ((xv exp))
       (match-var xv clause ...)))))

; internal macro to jump-start var collection and code generation;
; submatch below accepts only vars as first argument

(define-syntax match-var
  (syntax-rules (=>)
    ((_ xv) (if #f #f))
    ((_ xv (pat (=> nv) exp ...) . clauses)
     (let* ((nv (lambda () (match-var xv . clauses))) (b nv))
       (let-syntax ((n (syntax-rules () ((_ k . a*) (k () . a*)))))
         (submatch xv pat (b n) (let () exp ...) (nv)))))
    ((_ xv (pat (=> nv bv) exp ...) . clauses)
     (let* ((nv (lambda () (match-var xv . clauses))) (bv nv))
       (let-syntax ((n (syntax-rules () ((_ k . a*) (k () . a*)))))
         (submatch xv pat (bv n) (let () exp ...) (nv)))))
    ((_ xv (pat exp ...) . clauses)
     (let* ((next (lambda () (match-var xv . clauses))) (b next))
       (let-syntax ((n (syntax-rules () ((_ k . a*) (k () . a*)))))
         (submatch xv pat (b n) (let () exp ...) (next)))))))

; main var scanner / code generator

; Submatch operates in two modes: as pattern variable scanner and as code generator
; xv is a variable let-bound to the current expression being matched by the pattern
; c is 'context' (as described below, context format depends on mode of operation)
; p is a pattern (either primitive or a macro name followed by args / subpatterns)
; kt is the right-hand-side expr, usually complex, so it shouldn't be duplicated
; kf is the failure expression, usually just a call of a variable, can be duplicated
; - in scanner mode, it is invoked with () for xv and kf parameters and (n) context,
; where n is a macro thunk, returning a list of pairs (var . tmpid), with vars unique
; in free-id=? sense; this list is grown by syntactically rebinding n to a new list
; and expanding kt in its context. The main work is done by submatch, but external
; matchers have to cooperate by calling submatch with their sub-pattern arguments
; - in codegen mode, it is invoked with an id for xv and (b n) context, where n
; is as stated above (except that it assembles a list of vars, with no tempids),
; and b is the name of the lexically closest backtracking point (also a thunk).
; In this mode, submatch'es task is to build the matching code; it does that on
; its own for primitive patterns, but needs cooperation from external matchers for
; all non-primitive patterns

(define-syntax submatch
  (syntax-rules (quote quasiquote)
    ; scan for vars
    ((_ () () (n) kt ()) kt)
    ((_ () (quote lit) (n) kt ()) kt)
    ; generate code
    ((_ xv () c kt kf)
     (if (null? xv) kt kf))
    ((_ xv (quote lit) c kt kf)
     (if (equal? xv (quote lit)) kt kf))
    ((_ xv (quasiquote qq) c kt kf)
     (submatch-qq xv qq c kt kf))
    ((_ xv (h . t) c kt kf)
     (h xv t c kt kf))
    ; scan atom for vars
    ((_ () a (n) kt ())
     (classify a
       (syntax-error "... used as a variable name")
       kt ; _ is not a var
       (let-syntax
         ((k (syntax-rules ()
               ((_ v* t*)
                (if-new-var a v*
                  ; rebind n to a syntax 'thunk' returning extended name-temp alist
                  (let-syntax ((n (syntax-rules () ((_ k . args) (k (a . v*) (t . t*) . args)))))
                    kt)
                  kt)))))
         (n k))
       kt))
    ; generate atom code
    ((_ xv a (b n) kt kf)
     (classify a
       (syntax-error "... used as a variable name")
       kt ; _ matches anything
       (let-syntax
         ((k (syntax-rules ()
               ((_ v*)
                (if-new-var a v*
                  (let ((a xv))
                    ; rebind n to a syntax 'thunk' returning extended name list
                    (let-syntax ((n (syntax-rules () ((_ k . args) (k (a . v*) . args)))))
                      kt))
                  (if (equal? xv a) kt kf)))))) ; non-linear check ror repeated vars
         (n k))
       (if (equal? xv (quote a)) kt kf)))))

; quasiquote matcher expands into a combination of specific matchers defined below;
; multiple splicing patterns at the same level do greedy matching, maximizing lengths
; of left/upstream segments; other 'solutions' can be obtained via backtracking

(define-syntax submatch-qq
  (syntax-rules (unquote unquote-splicing)
    ((_ xv ,p c kt kf)
     (submatch xv p c kt kf))
    ((_ xv (,@lp) c kt kf)
     (submatch xv lp c kt kf))
    ((_ xv (,@lp . dp) c kt kf)
     (submatch xv (~append lp `dp) c kt kf))
    ((_ xv (ap . dp) c kt kf)
     (submatch xv (~cons `ap `dp) c kt kf))
    ((_ xv #(p ...) c kt kf)
     (submatch xv (~list->vector `(p ...)) c kt kf))
    ;((_ xv #&p c kt kf)
    ; (submatch xv (~box `p) c kt kf))
    ((_ xv lit c kt kf)
     (submatch xv (quote lit) c kt kf))))

; pattern var scanner -- uses var extraction protocol (submatch) and Petrofsky's extractor;
; var extraction protocol returns arbitrarily colored vars, distinct w.r.t free-id=?;
; Petrofsky's extractor locates their pattern-colored versions suitable for binding.
; It wouldn't be necessary if not for ~or and ~etc matchers that need to bind all vars
; NB: The extractor can't accidentally hit on a non-pattern identifier because we explicitly
; prohibit use of pattern variables in non-pattern subelements of patterns

(define-syntax extract-pattern-vars
  (syntax-rules () ; (_ p (rk . a*)) ==> (rk v* t* . a*)
    ((_ p (rk . a*))
     (let-syntax ((n (syntax-rules () ((_ k . args) (k () () . args)))))
       (submatch () p (n) ; scan for vars protocol
         (let-syntax
           ((k (syntax-rules ()
                 ((_ v* t*) (extract-pattern-vars p v* t* (rk . a*))))))
           (n k))
         ())))
     ((_ p v* t* (rk . a*)) ; rescan to get colors right (in bound-id=? sense)
      (extract* v* p (rk () t* . a*)))))

(define-syntax extract-new-pattern-vars
  (syntax-rules () ; (_ p n (rk . a*)) ==> (rk () nv* . a*)
    ((_ pat names cont)
     (letrec-syntax
       ((step1 (syntax-rules () ((_ mv* p c) (extract-pattern-vars p (step2 mv* c)))))
        (step2 (syntax-rules () ((_ v* t* mv* c) (varset-minus v* mv* step3 () c))))
        (step3 (syntax-rules () ((_ v-mv* (rk . a*)) (rk () v-mv* . a*)))))
       (names step1 pat cont)))))

;
; 3) matchers used by quasiquote notation
;

(define-syntax ~cons
  (syntax-rules ()
    ((_ () (ap dp) (n) kt ()) ; scan ap, dp for vars
     (submatch () ap (n) (submatch () dp (n) kt ()) ()))
    ((_ xv (ap dp) c kt kf)
     (if (pair? xv)
         (let ((axv (car xv)) (dxv (cdr xv)))
           (submatch axv ap c (submatch dxv dp c kt kf) kf))
         kf))))

(define-syntax ~list
  (syntax-rules ()
    ((_ xv () c kt kf)
     (submatch xv '() c kt kf))
    ((_ xv (p . p*) c kt kf)
     (submatch xv (~cons p (~list . p*)) c kt kf))))

(define-syntax ~list->vector
  (syntax-rules ()
    ((_ () (p) (n) kt ()) ; scan for vars
     (submatch () p (n) kt ()))
    ((_ xv (p) c kt kf)
     (if (vector? xv)
         (let ((yv (vector->list xv))) (submatch yv p c kt kf))
         kf))))

(define (match:append-greedy-start xv try) ; internal
  (let loop ((txv xv) (rxv '()))
    (if (pair? txv) (loop (cdr txv) (cons (car txv) rxv)) (try rxv txv))))

(define-syntax ~append
  (syntax-rules ()
    ((_ () p* (n) kt ()) ; scan for vars
     (submatch () (~and . p*) (n) kt ())) ; union
    ((_ xv () c kt kf)
     (submatch xv '() c kt kf))
    ((_ xv (p) c kt kf)
     (submatch xv p c kt kf))
    ((_ xv (hp tp) (b n) kt kf)
     (let ((f (lambda () kf))) ; in scope of 'parent' b
       (define (try rxv txv)
         (define (b) (if (pair? rxv) (try (cdr rxv) (cons (car rxv) txv)) (f)))
         (let ((hxv (reverse rxv))) ; match head first
           (submatch hxv hp (b n) (submatch txv tp (b n) kt (b)) (b))))
       (match:append-greedy-start xv try)))
    ((_ xv (p . p*) c kt kf)
     (submatch xv (~append p (~append . p*)) c kt kf))))

;(define-syntax ~box
;  (syntax-rules ()
;    ((_ () (p) (n) kt ()) ; scan for vars
;     (submatch () p (n) kt ()))
;    ((_ xv (p) c kt kf)
;     (if (box? xv)
;        (let ((yv (unbox xv))) (submatch yv p c kt kf))
;        kf))))


;
; 4) additional useful matchers, going beyond common core specified by SRFI-200
;

; 'value' matcher compares xv with the value of runtime-calculated expression via equal?

(define-syntax ~value
  (syntax-rules ()
    ((_ () (e) (n) kt ()) kt) ; scan for vars
    ((_ xv (e) c kt kf) (if (equal? xv e) kt kf))))


; non-greedy matcher for (possibly improper) lists

(define-syntax ~append/ng
  (syntax-rules ()
    ((_ () p* (n) kt ()) ; scan for vars
     (submatch () (~and . p*) (n) kt ())) ; union
    ((_ xv () c kt kf)
     (submatch xv '() c kt kf))
    ((_ xv (p) c kt kf)
     (submatch xv p c kt kf))
    ((_ xv (hp tp) (b n) kt kf)
     (let ((f (lambda () kf))) ; in scope of 'parent' b
       (let loop ((hxv '()) (txv xv)) ; match head first
         (define (b) (if (pair? txv) (loop (append hxv (list (car txv))) (cdr txv)) (f)))
         (submatch hxv hp (b n) (submatch txv tp (b n) kt (b)) (b)))))
    ((_ xv (p . p*) c kt kf)
     (submatch xv (~append/ng p (~append/ng . p*)) c kt kf))))

; non-iterative matcher for append with fixed-length rhs list (first arg, may be improper)
; may be used to implement single-ellipsis append patterns as popularized by syntax-rules

(define (match:append-tail-length l)
  (let loop ((l l) (n 0))
    (if (pair? l) (loop (cdr l) (+ n 1)) n)))

(define (match:append-tail-start xv n fail try) ;=> (try head tail) or (fail)
  (let loop ((l xv) (n n))
    (if (zero? n)
        (let loop ((lag xv) (lead l) (head '()))
          (if (pair? lead)
              (loop (cdr lag) (cdr lead) (cons (car lag) head))
              (try (reverse head) lag)))
        (if (pair? l) (loop (cdr l) (- n 1)) (fail)))))

(define-syntax ~append/t
  (syntax-rules ()
    ((_ () (t hp tp) (n) kt ()) ; scan for vars
     (submatch () (~and hp tp) (n) kt ())) ; union
    ((_ xv (t hp tp) c kt kf)
     (let ((f (lambda () kf)))
       (match:append-tail-start xv (if (integer? 't) 't (match:append-tail-length 't)) f
         (lambda (hxv txv) (submatch hxv hp c (submatch txv tp c kt (f)) (f))))))))

; general-purpose parameterized backtracking matcher

; (~iterate start head tail v* p) builds a backtracking matcher
; that produces a stream of 'solutions' to be matched against p;
; v* is a list of state variables (names mostly used for exposition)
; - start is invoked with original value and two continuation procedures:
; first one accepts 'seed' values for state variables if start succeeds,
; the second one accepts no values and is called if start fails
; - head accepts current values of state variables and returns a
; single value to be matched against pattern p
; - tail accepts the same two continuations as start, followed by
; the current values of state variables; it should either call its
; first continuation to continue with new values of state vars,
; or the second one to signal that there are no more 'solutions'
; note: start head tail can be procedures add/or macros
; note: both try and f should be called in tail positions!
(define-syntax ~iterate
  (syntax-rules ()
    ; scan for vars
    ((_ () (start head tail v* p) (n) kt ())
     (submatch () p (n) kt ()))
    ; generate code
    ((_ xv (start head tail v* p) (b n) kt kf)
     (let ((f (lambda () kf))) ; in scope of 'parent' b
       (define (try . v*)
         (define (b) (tail try f . v*))
         (let ((mxv (head . v*)))
           (submatch mxv p (b n) kt (b))))
       (start xv try f)))))


; 'popular' parameterized matchers for vector-like sequences

(define-syntax ~seq-append
  (syntax-rules ()
    ((_ () (x? x-length subx nullx . p*) (n) kt ()) ; scan for vars
     (submatch () (~and . p*) (n) kt ()))
    ((_ xv (x? x-length subx nullx) c kt kf)
     (submatch xv nullx c kt kf))
    ((_ xv (x? x-length subx nullx p) c kt kf)
     (submatch xv p c kt kf))
    ((_ xv (x? x-length subx nullx hp tp) (b n) kt kf)
     (let ((l (x-length xv)) (f (lambda () kf)))
       (let loop ((i l))
         (define (b) (if (> i 0) (loop (- i 1)) (f)))
         (let ((hxv (subx xv 0 i))) ; match head first
           (submatch hxv hp (b n)
             (let ((txv (subx xv i l)))
               (submatch txv tp (b n) kt (b))) (b))))))
  ((_ xv (x? x-length subx nullx p . p*) c kt kf)
   (submatch xv (~seq-append x? x-length subx nullx p
      (~seq-append x? x-length subx nullx . p*)) c kt kf))))

(define-syntax ~seq-append/ng
  (syntax-rules ()
    ((_ () (x? x-length subx nullx . p*) (n) kt ()) ; scan for vars
     (submatch () (~and . p*) (n) kt ()))
    ((_ xv (x? x-length subx nullx) c kt kf)
     (submatch xv nullx c kt kf))
    ((_ xv (x? x-length subx nullx p) c kt kf)
     (submatch xv p c kt kf))
    ((_ xv (x? x-length subx nullx hp tp) (b n) kt kf)
    (let ((l (x-length xv)) (f (lambda () kf)))
      (let loop ((i 0))
        (define (b) (if (< i l) (loop (+ i 1)) (f)))
        (let ((hxv (subx xv 0 i))) ; match head first
          (submatch hxv hp (b n)
            (let ((txv (subx xv i l)))
              (submatch txv tp (b n) kt (b))) (b))))))
  ((_ xv (x? x-length subx nullx p . p*) c kt kf)
   (submatch xv (~seq-append/ng x? x-length subx nullx p
      (~seq-append/ng x? x-length subx nullx . p*)) c kt kf))))


;
; 5) boolean matchers
;

; 'and'-matcher (all pattern vars are bound in the rest of the patterns)

(define-syntax ~and
  (syntax-rules ()
    ((_ xv () c kt kf)
     kt)
    ((_ xv (p) c kt kf)
     (submatch xv p c kt kf))
    ((_ xv (p . p*) c kt kf)
     (submatch xv p c (submatch xv (~and . p*) c kt kf) kf))))

; 'or'-matcher (on success, all pattern vars are bound to #f except for those in the matching case)

(define-syntax ~or
  (syntax-rules ()
    ((_ () p* (n) kt ()) ; scan for vars
     (submatch () (~and . p*) (n) kt ())) ; union
    ((_ xv () c kt kf)
     kf)
    ((_ xv (p) c kt kf)
     (submatch xv p c kt kf))
    ((_ xv p* (b n) kt kf)
     (extract-new-pattern-vars (~and . p*) n (~or xv p* (b n) kt kf)))
    ((_ () (v ...) xv p* c kt kf)
     (let ((v #f) ... (lkt (lambda (v ...) kt)))
       (submatch xv (match:or-aux . p*) c (lkt v ...) kf)))))

(define-syntax match:or-aux ; helper for ~or
  (syntax-rules ()
    ((_ xv () c kt kf)
     kf)
    ((_ xv (p) c kt kf)
     (submatch xv p c kt kf))
    ((_ xv (p . p*) (b n) kt kf) ; kt can be duplicated
     (let ((b (lambda () (submatch xv (match:or-aux . p*) (b n) kt kf)))) ; in scope of 'parent' b
       (submatch xv p (b n) kt (b))))))

; 'not'-matcher (no additional pattern vars, if present, are bound in the rest of the patterns)

(define-syntax ~not
  (syntax-rules ()
    ((_ () p* (n) kt ()) kt) ; scan for vars: no vars escape
    ((_ xv (p) c kt kf)
     (let ((t (lambda () kt)) (f (lambda () kf)))
       (submatch xv p c (f) (t))))
    ((_ xv (p ...) c kt kf)
     (submatch xv (~and (~not p) ...) c kt kf))))


;
; 6)  ~etc: ... - like list matchers
;

; Nonlinear part of ~etc works as follows: code for p is generated in
; empty 'n' (i.e. we skip nonlinear checks against vars on the left);
; when all p's own vars are bound to full lists, a precondition code
; is built that compares their values with the existing values of
; common variables of p and patterns upstream and triggers a failure
; if they are different; then the 'n' lists are merged for the patterns
; downstream (this part is easy -- just scan p in the usual manner)

(define-syntax match:etc-nl-test
  (syntax-rules ()
    ((_ ov* () () e*) (and . e*))
    ((_ ov* (iv . iv*) (it . it*) e*)
     (if-new-var iv ov*
       (match:etc-nl-test ov* iv* it* e*)
       (match:etc-nl-test ov* iv* it*
         ((equal? iv (reverse it)) . e*))))))

(define-syntax ~etc
  (syntax-rules ()
    ((_ () (p) (n) kt ()) ; scan for vars
     (submatch () p (n) kt ()))
    ((_ xv (p) c kt kf)
     (extract-pattern-vars p (~etc xv p c kt kf)))
    ((_ (v ...) (t ...) xv p (b n) kt kf)
     (let loop ((lxv xv) (t '()) ...)
       (cond ((null? lxv)
              (if (n match:etc-nl-test (v ...) (t ...) ())
                  (let ((v (reverse t)) ...) kt)
                  kf))
             ((pair? lxv)
              (let ((axv (car lxv)) (dxv (cdr lxv)))
                (let-syntax ((n (syntax-rules () ((_ k . a*) (k () . a*)))))
                  (submatch axv p (b n) (loop dxv (cons v t) ...) kf))))
             (else kf))))))

(define-syntax ~etcse ; sine errore
  (syntax-rules ()
    ((_ () (p) (n) kt ()) ; scan for vars
     (submatch () p (n) kt ()))
    ((_ xv (p) c kt kf)
     (extract-pattern-vars p (~etcse xv p c kt kf)))
    ((_ (v ...) (t ...) xv p (b n) kt kf)
     (let loop ((lxv xv) (t '()) ...)
       (cond ((null? lxv)
              (if (n match:etc-nl-test (v ...) (t ...) ())
                  (let ((v (reverse t)) ...) kt)
                  kf))
             ((pair? lxv)
              (let ((axv (car lxv)) (dxv (cdr lxv)))
                (let-syntax ((n (syntax-rules () ((_ k . a*) (k () . a*)))))
                  (submatch axv p (b n) ; just skip failed submatches
                    (loop dxv (cons v t) ...) (loop dxv t ...)))))
             (else kf))))))


;
; 7) general-purpose matchers with a function/predicate parameter
;

; 'property access' matcher: when matching x,
; (~prop f => p ...)            matches the result(s) of (f x) to p ...
; (~prop f (arg ...) => p ...)  matches the result(s) of (f x arg ...) to p ...
(define-syntax ~prop
  (syntax-rules (=>)
    ; scan for vars
    ((_ () (x->y => . p*)         (n) kt ()) (submatch () (~and . p*) (n) kt ()))
    ((_ () (x->y (a ...) => . p*) (n) kt ()) (submatch () (~and . p*) (n) kt ()))
    ; generate code
    ((_ xv (x->y => p) c kt kf)
     (let ((yv (x->y xv)))
       (submatch yv p c kt kf)))
    ((_ xv (x->y => . p*) c kt kf)
     (let ((yv (call-with-values (lambda () (x->y xv)) list)))
       (submatch yv (~list . p*) c kt kf)))
    ((_ xv (x->y (a ...) => p) c kt kf)
     (let ((yv (x->y xv a ...)))
       (submatch yv p c kt kf)))
    ((_ xv (x->y (a ...) => . p*) c kt kf)
     (let ((yv (call-with-values (lambda () (x->y xv a ...)) list)))
       (submatch yv (~list . p*) c kt kf)))))

; 'predicate test' matcher: when matching x,
; (~test f)                     fails if (f x) returns #f
; (~test f (arg ...))           fails if (f x arg ...) returns #f
; (~test f => p)                fails if (f x) returns #f, otherwise matches result to p
; (~test f (arg ...) => p)      fails if (f x) returns #f, otherwise matches result to p
(define-syntax ~test
  (syntax-rules (=>)
    ; scan for vars
    ((_ () (x?)              (n) kt ()) kt)
    ((_ () (x? (a ...))      (n) kt ()) kt)
    ((_ () (x? => p)         (n) kt ()) (submatch () p (n) kt ()))
    ((_ () (x? (a ...) => p) (n) kt ()) (submatch () p (n) kt ()))
    ; generate code
    ((_ xv (x?) c kt kf)
     (if (x? xv) kt kf))
    ((_ xv (x? (a ...)) c kt kf)
     (if (x? xv a ...) kt kf))
    ((_ xv (x? => p) c kt kf)
     (let ((v (x? xv))) (if v (submatch v p c kt kf) kf)))
    ((_ xv (x? (a ...) => p) c kt kf)
     (let ((v (x? xv a ...))) (if v (submatch v p c kt kf) kf)))))


;
; 8) special matchers serving as building blocks for custom matchers
;

; 'literal check' matcher : (~if-id-member a (l ...) pt pf)
; uses pt if a is in (l ...), pf otherwise; uses free-id=? rules
(define-syntax ~if-id-member
  (syntax-rules ()
    ((_ xv (a (l ...) pt pf) c kt kf)
     (classify a
       (syntax-error "... used as a variable name")
       (syntax-error "_ used as a variable name")
       (if-new-var a (l ...) (submatch xv pf c kt kf) (submatch xv pt c kt kf))
       (submatch yv pt c kt kf)))))

; (~replace-specials new-ellipsis new-underscore p) matches against p after replacing
; ... in p with new-ellipsis and _ with new-underscore
(define-syntax ~replace-specials
  (syntax-rules ()
    ((_ xv (ne nu p) c kt kf)
     (replace-specials ne nu p ~replace-specials #f xv c kt kf))
    ((_ newp #f xv c kt kf)
     (submatch xv newp c kt kf))))

; 'cut' matcher (does not allow backtracking into p)
(define-syntax ~cut!
  (syntax-rules ()
    ((_ () (p) (n) kt ()) ; scan for vars
     (submatch () p (n) kt ()))
    ((_ xv (p) (b n) kt kf)
     (let ((b! b)) (submatch xv p (b n) (let ((b b!)) kt) kf)))))


;
; 9) extending the matcher
;

(define-syntax define-match-pattern
  (syntax-rules ()
    ((_ ~dm (l ...) ((* . args) p) ...)
     (define-syntax ~dm
       (syntax-rules (l ...)
         ((_ xv args c kt kf)
          (submatch xv p c kt kf)) ...)))))

(define-syntax define-record-match-pattern
  (syntax-rules ()
    ((_ (~name v ...) pred? (v1 acc . _) ...)
     (define-match-pattern ~name ()
       ((_ v ...) (~and (~test pred?) (~prop acc => v1) ...))))))


; NB: all new matchers below are defined via define-match-pattern (no more submatch/hand-coding)


;
; 10) convenience matchers for popular scheme data types
;

; this matcher is already defined in section 3) :
; (define-match-pattern ~list->vector () ((_ p) (~and (~test vector?) (~prop vector->list => p))))

(define-match-pattern ~null?            () ((_ p ...) (~and (~test null?)    p ...)))
(define-match-pattern ~pair?            () ((_ p ...) (~and (~test pair?)    p ...)))
(define-match-pattern ~list?            () ((_ p ...) (~and (~test list?)    p ...)))
(define-match-pattern ~boolean?         () ((_ p ...) (~and (~test boolean?) p ...)))
(define-match-pattern ~number?          () ((_ p ...) (~and (~test number?)  p ...)))
(define-match-pattern ~integer?         () ((_ p ...) (~and (~test integer?) p ...)))
(define-match-pattern ~vector?          () ((_ p ...) (~and (~test vector?)  p ...)))
(define-match-pattern ~string?          () ((_ p ...) (~and (~test string?)  p ...)))
(define-match-pattern ~symbol?          () ((_ p ...) (~and (~test symbol?)  p ...)))
(define-match-pattern ~char?            () ((_ p ...) (~and (~test char?)    p ...)))

(define-match-pattern ~vector->list     () ((_ p)     (~and (~test list?)   (~prop list->vector => p))))
(define-match-pattern ~string->list     () ((_ p)     (~and (~test list?)   (~prop list->string => p))))
(define-match-pattern ~list->string     () ((_ p)     (~and (~test string?) (~prop string->list => p))))
(define-match-pattern ~string->symbol   () ((_ p)     (~and (~test symbol?) (~prop symbol->string => p))))
(define-match-pattern ~symbol->string   () ((_ p)     (~and (~test string?) (~prop string->symbol => p))))

(define-match-pattern ~vector           () ((_ p ...) (~and (~test vector?) (~prop vector->list => (~list p ...)))))
(define-match-pattern ~string           () ((_ p ...) (~and (~test string?) (~prop string->list => (~list p ...)))))

(define-match-pattern ~string-append    () ((_ p ...) (~seq-append    string? string-length substring "" p ...)))
(define-match-pattern ~string-append/ng () ((_ p ...) (~seq-append/ng string? string-length substring "" p ...)))
(define-match-pattern ~vector-append    () ((_ p ...) (~seq-append    vector? vector-length vector-copy '#() p ...)))
(define-match-pattern ~vector-append/ng () ((_ p ...) (~seq-append/ng vector? vector-length vector-copy '#() p ...)))

(define-match-pattern ~number->string   () ((_ p)     (~and (~test string?) (~prop string->number => p)))
                                           ((_ p rx)  (~and (~test string?) (~prop string->number (rx) => p))))
(define-match-pattern ~string->number   () ((_ p)     (~and (~test number?) (~prop number->string => p)))
                                           ((_ p rx)  (~and (~test number?) (~prop number->string (rx) => p))))

;
; 11) additional convenience/compatibility matchers
;

; WCS-like = property matcher
(define-match-pattern ~= ()
  ((~= f p) (~prop f => p)))

; WCS-like ? predicate matcher
(define-match-pattern ~? ()
  ((~? f p ...) (~and (~test f) p ...)))


; Racket-like list* (a.k.a. cons*)

(define-match-pattern ~list* ()
  ((~list* p) p)
  ((~list* p . p*) (~cons p (~list* . p*))))

; Racket-like list-no-order

(define-syntax match:cno-start
  (syntax-rules () ((_ xv try f) (if (pair? xv) (try '() xv) (f)))))

(define-syntax match:cno-head
  (syntax-rules () ((_ h t) (cons (car t) (append h (cdr t))))))

(define-syntax match:cno-tail
  (syntax-rules () ((_ try f h t) (if (pair? (cdr t)) (try (cons (car t) h) (cdr t)) (f)))))

(define-match-pattern ~cons-no-order ()
  ((~cons-no-order pe pr)
   (~iterate match:cno-start match:cno-head match:cno-tail (h t) (~cons pe pr))))

(define-match-pattern ~list-no-order ()
  ((~list-no-order) '())
  ((~list-no-order p) (~list p))
  ((~list-no-order p . p*) (~cons-no-order p (~list-no-order . p*))))

(define-match-pattern ~list-no-order* ()
  ((~list-no-order* p) p)
  ((~list-no-order* p . p*) (~cons-no-order p (~list-no-order* . p*))))


;========================================================================================
;
;  Complementary pieces of templating
;
;========================================================================================

; By construction, patterns resemble corresponding Scheme constructors, and can be
; combined in the same way, so regular base Scheme already supplies cons, list, and
; scores of other functions that re-construct what similar-looking patterns de-construct:
;
; pattern: (~list (~cons a b))   template (regular Scheme): (list (cons a b))
;
; This proposal's quasiquote patterns also mirror regular Scheme's quasiquote templates:
;
; pattern: `(,(~cons a b))   template (regular Scheme): `(,(cons a b))
; pattern: `(,@a ,@b)        template (regular Scheme): `(,@a ,@b)
;
; This correspondence is not designed to be 1-to-1, because needs of patterns and Scheme
; 'templates', i.e. constructor expressions are different. Still, there are pieces of the
; pattern language that call for equally expressive template forms. In particular, one
; may find it convenient to have a templating counterpart to ~etc patterns.
;
; Unlike patterns though, the choice of regular scheme expressions for templating does
; not allow us to collect what would act as template variables easily -- procedures
; won't cooperate. What we can do is to limit subforms of 'etc' templates, using a
; restricted grammar that makes this possible:
;
; <template expression> -> <value expression> | <etc expression> | <expression>
;
; <value expression> -> (value <expression>)
;
; <etc expression> -> (etc <template>)
;
;    <template> ->
;     <template var>      ; symbol
;   | <constant>          ; char, string, number, ...
;   | (quote <datum>)
;   | (value <expression>)
;   | (<name> <template> ...)
;
; NB: <name> should be restricted to names of forms that have expressions as subforms;
; this disqualify forms like lambda, let, do, forms with internal definitions etc.
;
; This restricted form allows scanning for template vars due to its limited nature.
; Scanning of <etc expression> should produce one or more template vars, which are
; expected to be lists of the same length at runtime, so the following transformation
; can produce the actual output expression when 'etc' form is macroexpanded:
;
; (etc (list x* y* (value foo))) ==> (map (lambda (x* y*) (list x* y* foo)) x* y*)
;
; Note that (value x) parts, not scanned for template vars, are just replicated.
; In all other respects, value is just the identity macro.

(define-syntax value ; need to be imported together with etc
  (syntax-rules () ((_ x) x)))

; scanning for template vars

(define-syntax extract-template-vars
  (syntax-rules () ; (_ t rk . a*) => (rk v* . a*)
    ((_ tpl . cont)
     (letrec-syntax
       ((err
         (syntax-rules ()
           ((_ t) (syntax-error "etc: invalid template" t))))
        (ext
         (syntax-rules (quote value) ; quasiquote
           ((_ (quote x)      k . a*) (k () . a*))
           ((_ (value x)      k . a*) (k () . a*))
           ((_ ()             k . a*) (err ()))
           ((_ (n)            k . a*) (k () . a*))
           ((_ (n t)          k . a*) (ext t k . a*))
           ((_ (n t . t*)     k . a*) (ext t extcdr (n . t*) k . a*))
           ;((_ #&x            k . a*) (err #&x))
           ((_ #(x (... ...)) k . a*) (err #(x (... ...))))
           ((_ x              k . a*)
            (classify x (err x) (err x) (k (x) . a*) (k () . a*)))))
        (extcdr
         (syntax-rules ()
           ((_ v* t k . a*) (ext t merge v* k . a*))))
        (merge
         (syntax-rules ()
           ((_ () rv*         k . a*) (k rv* . a*))
           ((_ (v . v*) rv*   k . a*)
            (if-new-var v rv* (merge v* (v . rv*) k . a*) (merge v* rv* k . a*))))))
       (ext tpl . cont)))))

; expander for (etc <template>)

(define-syntax etc
  (syntax-rules ()
    ((_ t)
     (classify t
       (syntax-error "etc: ... used as a template variable")
       (syntax-error "etc: _ used as a template variable")
       t ; optimization: for template var t, (etc t) ==> t
       (extract-template-vars t etc t)))
    ((_ () t) (syntax-error "etc: no template variables" t))
    ((_ (v ...) t) (map (lambda (v ...) t) v ...))))

; end
))
