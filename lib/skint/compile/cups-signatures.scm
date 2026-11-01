;;; cups-signatures.scm -- CUPS part 2: declarative integrable signatures.
;;; Replaces #F's C-string parsing (prim-ctype / c-argref-ctype /
;;; typecheck-prim-ctype in 6.sf) for Skint's symbolic integrables.
;;; A signature is (ret-type arg-type ...) where a type is one of
;;;   fx fl bool char obj #f        (#f / obj = boxed Scheme object)
;;; `check` maps a type predicate to the type it proves; `assert` maps a
;;; %ck* assertion integrable to the type it establishes.  Used by
;;; constant-fold (pre-CPS) and unbox-values (post-CPS).

(define (prim->sym prim)
  (cond [(symbol? prim) prim]
        [(integer? prim) (integrable-global prim)]
        [else #f]))

(define (prim->index prim)
  (cond [(integer? prim) prim]
        [(symbol? prim) (lookup-integrable prim)]
        [else #f]))

(define *integrable-signatures*
  (let ()
    (define (grp syms sig) (map (lambda (s) (cons s sig)) syms))
    (append
      ;; fixnum arithmetic / bitwise / shifts: fx x fx -> fx, predicates -> bool
      (grp '(fx+ fx- fx* fxquotient fxremainder fxmodulo fxmodquo
             fxeucquo fxeucrem fxgcd fxand fxior fxxor fxnot fxsll fxsrl fxsra
             fxmin fxmax fxabs fxneg)
           '(fx fx fx))
      (grp '(fx/ fxdiv) '(fx fx fx))
      (grp '(fx<? fx>? fx<=? fx>=? fx=? fx!=? fxzero? fxpositive? fxnegative?
             fxeven? fxodd?)
           '(bool fx fx))
      (grp '(fixnum?) '(bool obj))
      (grp '(fixnum->flonum) '(fl fx))
      (grp '(flonum->fixnum) '(fx fl))
      ;; flonum arithmetic
      (grp '(fl+ fl- fl* fl/ flmin flmax flneg flabs flfloor flceiling
             fltruncate flround flexp fllog flsin flcos fltan flasin flacos
             flatan flexpt flsqrt flgcd flmodquo flmodulo flquotient flremainder)
           '(fl fl fl))
      (grp '(fl<? fl>? fl<=? fl>=? fl=? fl!=? flzero? flpositive? flnegative?
             fleven? flodd? flinteger? flnan? flfinite? flinfinite?)
           '(bool fl fl))
      (grp '(flsinh flcosh fltanh fllog10) '(fl fl))
      (grp '(flldexp) '(fl fl fx))
      (grp '(flonum?) '(bool obj))
      ;; generic numeric (conservative: only predicates are typed)
      (grp '(zero? positive? negative? even? odd? number? integer? rational?
             exact? inexact? complex? real? nan? finite? infinite?
             exact-integer?)
           '(bool obj obj))
      (grp '(char? boolean? symbol? pair? null? vector? bytevector? string?
             procedure? box? void? record? list? circular? eof-object?)
           '(bool obj))
      (grp '(char->integer) '(fx obj))
      (grp '(integer->char) '(obj fx))
      (grp '(car cdr caar cadr cdar cddr caaar caadr cadar caddr cdaar cdadr
             cddar cdddr caaaar caaadr caadar caaddr cadaar cadadr caddar
             cadddr cdaaar cdaadr cdadar cdaddr cddaar cddadr cdddar cddddr
             unbox vector-length string-length bytevector-length length
             list-ref list-tail last-pair)
           '(obj obj))
      (grp '(cons) '(obj obj obj))
      (grp '(eq? eqv?) '(bool obj obj))
      '())))

(define (integrable-signature prim) ; => (ret arg ...) or #f
  (cond [(prim->sym prim) => (lambda (s) (assq s *integrable-signatures*))]
        [else #f]))

(define (sig-ret sig) (and sig (cadr sig)))
(define (sig-args sig) (and sig (cddr sig)))
(define (sig-arg sig n)
  (let ([a (sig-args sig)]) (and a (< n (length a)) (list-ref a n))))

;; type predicate symbol -> type it proves about its argument
(define *typecheck-table*
  '((fixnum? . fx) (flonum? . fl) (char? . char) (boolean? . bool)))

(define (typecheck-prim-ctype prim) ; our ctype symbols, not #F C strings
  (cond [(prim->sym prim) =>
         (lambda (s) (cond [(assq s *typecheck-table*) => cdr] [else #f]))]
        [else #f]))

;; %ck* assertion integrable -> type it establishes about its argument
(define *typeassert-table*
  '((%cki . fx) (%ckj . fx) (%ckc . char) (%ckp . obj) (%ckv . obj)
    (%cks . obj) (%ckn . fx)))

(define (typeassert-prim-ctype prim)
  (cond [(prim->sym prim) =>
         (lambda (s) (cond [(assq s *typeassert-table*) => cdr] [else #f]))]
        [else #f]))

(define (prim-ret-ctype prim)
  (let ([sig (integrable-signature prim)]) (and sig (sig-ret sig))))

;; ctype of the n-th argument when used in a typed context (unboxing)
(define (prim-arg-ctype prim n)
  (let ([sig (integrable-signature prim)]) (and sig (sig-arg sig n))))

(define (cvar-id sym ctype) (var-id sym (timestamp) ctype))
(define (cvar-id? id)
  (and (not (global-id? id)) (memq (id->ctype id) '(fx fl bool char))))

;; exp-ctype: best static ctype of an expression under env ((id . ctype)...)
(define (exp-ctype exp env)
  (variant-case exp
    [var-exp (id) (cond [(assq id env) => cdr] [else #f])]
    [quote-exp (obj)
     (cond [(and (integer? obj) (exact? obj)) 'fx]
           [(and (number? obj) (inexact? obj)) 'fl]
           [(boolean? obj) 'bool]
           [(char? obj) 'char]
           [else #f])]
    [if-exp (test-exp then-exp else-exp)
     (let ([a (exp-ctype then-exp env)] [b (exp-ctype else-exp env)])
       (and a b (eq? a b) a))]
    [let-exp (ids rands body)
     (exp-ctype body
       (append (map (lambda (id rand) (cons id (exp-ctype rand env)))
                    ids rands)
               env))]
    [primapp-exp (effect prim rands) (prim-ret-ctype prim)]
    [else #f]))
