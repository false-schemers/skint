
;;; cups-base.scm -- CUPS above-CUPS layer, part 1: utilities, AST, pre-CPS passes.
;;; Derived from compile-x.txt (Skint <core> + sfc 3.sf/4.sf lineage); see ../README.md.
;;; This file is concatenated with cups-signatures.scm, cups-passes.scm,
;;; cups-front.scm and cups-main.scm into ../cups-all.scm (make all). Do not edit
;;; ../cups-all.scm directly.

;--------------------------------------------------------------------------------------------------
; Utils
;--------------------------------------------------------------------------------------------------


(define-syntax define-inline
  (syntax-rules ()
    [(_ (op . ll) . body)
     (define-syntax op (lambda ll . body))]
    [(_ op val)
     (define-syntax op val)]))

; andmap ormap list* come Skint

(define-inline compose
  (lambda (f g) (lambda (x) (f (g x)))))

(define-inline complement
  (lambda (f) (lambda (x) (not (f x)))))

(define (iota n m) ; returns the list (n n+1 n+2 ... m-1)
  (if (< n m) (cons n (iota (+ n 1) m)) '()))

(define (keep f lst)
  (cond [(null? lst) '()]
        [(f (car lst)) (cons (car lst) (keep f (cdr lst)))]
        [else (keep f (cdr lst))]))

(define-inline (adjoinq x s)
  (if (memq x s) s (cons x s)))

(define (subsetq? s1 s2)
  (andmap (lambda (x) (memq x s2)) s1))

(define (seteq? s1 s2)
  (and (subsetq? s1 s2) (subsetq? s2 s1)))
  
(define (unionq s1 s2)
  (cond [(null? s1) s2]
        [(memq (car s1) s2) (unionq (cdr s1) s2)]
        [else (cons (car s1) (unionq (cdr s1) s2))]))

(define (intersectionq s1 s2)
  (cond [(null? s1) '()]
        [(memq (car s1) s2) (cons (car s1) (intersectionq (cdr s1) s2))]
        [else (intersectionq (cdr s1) s2)]))

(define (setdiffq s1 s2)
  (cond [(null? s1) '()]
        [(memq (car s1) s2) (setdiffq (cdr s1) s2)]
        [else (cons (car s1) (setdiffq (cdr s1) s2))]))

(define (reduce-right f base lst)
  (if (null? lst)
      base
      (f (car lst) (reduce-right f base (cdr lst))))) 

(define (reduce-left f base lst)
  (if (null? lst)
      base
      (reduce-left f (f (car lst) base) (cdr lst))))

(define (reduce-right/singular-identity f base lst)
  (if (null? lst) 
      base
      (let loop ([lst lst])
        (if (null? (cdr lst))
            (car lst)
            (f (car lst) (loop (cdr lst)))))))

(define (reduce-right/right-seed f base lst)
   (if (null? lst)
       base
       (let loop ([lst lst])
         (if (null? (cdr lst))
             (f (car lst) base)
             (f (car lst) (loop (cdr lst)))))))

(define (posq x lst)
  (let loop ([lst lst] [i 0])
    (cond [(null? lst) #f]
          [(eq? (car lst) x) i]
          [else (loop (cdr lst) (+ i 1))])))

(define (string-append* lst)
  (define (rslen lst)
    (let loop ([lst lst] [len 0])
      (if (null? lst) 
          len 
          (loop (cdr lst) (+ len (string-length (car lst)))))))
  (let ([str (make-string (rslen lst))])
    (let copy ([lst lst] [pos 0])
      (if (null? lst)
          str
          (let ([s (car lst)])
            (let ([endpos (+ pos (string-length s))])
              (let loop ([i 0])
                (if (= (+ pos i) endpos)
                    (copy (cdr lst) endpos)
                    (begin
                      (string-set! str (+ pos i) (string-ref s i))
                      (loop (+ i 1)))))))))))

(define (symbol* args)
  (string->symbol
    (string-append*
      (map (lambda (x) 
        (if (symbol? x) (symbol->string x)
          (if (number? x) (number->string x) x)))
        args))))

(define-syntax symbol
  (syntax-rules () [(_ a ...) (symbol* (list a ...))]))

(define-syntax andapp 
  (syntax-rules () [(_ f a ...) (andmap f (list a ...))]))

(define-syntax orapp 
  (syntax-rules () [(_ f a ...) (ormap f (list a ...))]))

; variants: thin macro layer over tagged vectors

(define-syntax define-variant
  (syntax-rules ()
    [(define-variant name () ([field0 index0] ...))
     (begin
       (define-syntax name
         (lambda (field0 ...)
           (vector 'name field0 ...)))
       (define-syntax 
         (... string->id 
           (... string-append (... id->string name) "?") define-variant)
         (lambda (object)
           (and (vector? object)
             (= (vector-length object) (... length (name field0 ...)))
             (eq? (vector-ref object 0) 'name))))
       (define-syntax 
         (... string->id 
           (... string-append (... id->string name) "->" 
             (... id->string field0)) define-variant)
         (lambda (object)
           (vector-ref object index0)))
       ...)]
    [(define-variant name (field0 field ...) (pair ...))
     (define-variant name (field ...)  
       (pair ... [field0 (... length (name pair ...))]))]
    [(define-variant name (field0 ...))
     (define-variant name (field0 ...) ())]))

(define-syntax variant-case
  (syntax-rules (else)
    [(variant-case (a . d) clause ...)
     (let ([var (a . d)]) (variant-case var clause ...))]
    [(variant-case var) (error "no variant-case clause matches" var)]
    [(variant-case var (else exp1 exp2 ...)) (body exp1 exp2 ...)]
    [(variant-case var [name (field ...) exp1 exp2 ...] clause ...)
     (if ((... string->id (... string-append (... id->string name) "?") variant-case) var)
         (let ([field ((... string->id (... string-append (... id->string name) "->" 
                         (... id->string field)) variant-case) 
                       var)] ...)
           exp1 exp2 ...)
         (variant-case var clause ...))]))

;------------------------------------------------------------------------------

; ids/variables and id lists/environments:

(define (var-id? bnd) (and (pair? bnd) (eq? (car bnd) 'var)))
(define-inline (var-id sym ts xt) (list 'var sym ts xt))
(define-inline id->symbol cadr)
(define-inline id->ts caddr)
(define-inline id->ctype cadddr)

(define (id<? i1 i2) ; total ordering used for sets
  (< (id->ts i1) (id->ts i2)))

(define-inline (extendify bindings env)
  (append bindings env))

(define (lookupicate sym env)
  (cond [(null? env) #f]
        [(eq? (id->symbol (car env)) sym) (car env)]
        [else (lookupicate sym (cdr env))]))

(define last-id-timestamp 0)

(define (reset-timestamps)
  (set! last-id-timestamp 0))
  
(define (timestamp)
  (set! last-id-timestamp (+ last-id-timestamp 1))
  last-id-timestamp)

(define (global-id sym)
  (var-id sym (- (timestamp)) #f))

(define (global-id? id)
  (negative? (id->ts id)))

(define (global-id-constant sym)
  (var-id sym (- (timestamp)) #t))

(define (global-id-constant? id)
  (and (negative? (id->ts id)) (id->ctype id)))

(define (global-id-private-constant name)
  (let ([ts (timestamp)])
    (var-id (symbol name "#" ts) (- ts) 'private)))

(define (global-id-private-constant? id)
  (and (negative? (id->ts id)) (eq? (id->ctype id) 'private)))

(define (lexical-id sym)
  (var-id sym (timestamp) #f))

(define (rename-id id)
  (var-id (id->symbol id) (timestamp) #f))

(define (prefix-id p id)
  (var-id (symbol p (id->symbol id)) (timestamp) #f))

(define (postfix-id id p)
  (var-id (symbol (id->symbol id) p) (timestamp) #f))

(define (label-id sym)
  (var-id sym (timestamp) #t))

(define (label-id? id)
  (and (not (global-id? id)) (eq? (id->ctype id) #t)))

(define (id->uname id)
  (cond [(global-id-private-constant? id) (id->symbol id)] 
        [(global-id-constant? id) (id->symbol id)] 
        [(global-id? id) (id->symbol id)]
        [(id->ctype id) (symbol (id->symbol id) ":" (id->ts id))]
        [else (symbol (id->symbol id) "." (id->ts id))]))

(define begin-id-symbol (symbol "."))

;------------------------------------------------------------------------------

; effects are used to classify primitives

(define-inline (no-effect) 'no-effect) ; functional, can be eliminated if not used
(define-inline (*-effect) '*-effect) ; allocates, but has no other effects
(define-inline (?-effect) '?-effect) ; only observes effects of others; doesn't alloc
(define-inline (!-effect) '!-effect) ; produces effects; doesn't alloc
(define-inline (?!-effect) '?!-effect) ; has & observes effects; doesn't alloc
(define-inline (*?-effect) '*?-effect) ; allocates, observes effects of others
(define-inline (*!-effect) '*!-effect) ; allocates, produces effects
(define-inline (*?!-effect) '*?!-effect) ; anything but the nasal demons

; memory model-dependent effects. nb: optimized code may be nonportable!
(define (%flonums-immediate?) #t) ; CUPS: 64-bit NaN-boxing (smarv.html sec.3)
(define (%-effect) (if (%flonums-immediate?) (no-effect) (*-effect)))
(define (%?-effect) (if (%flonums-immediate?) (?-effect) (*?-effect)))

(define-inline (ref-transparent-effect? eff) ; exp can be moved freely
  (eq? eff 'no-effect))

(define-inline (removable-if-dead-effect? eff) ; exp can be removed
  ;; An observer is NOT removable just because its value is dead.  The
  ;; table classifies the checked accessors and the pair accessors as
  ;; ?-effect ("observes the effects of others"), and those raise on a
  ;; wrong type: raising is observable whatever becomes of the value, so
  ;; dropping one changes what the program does.  Only a computation that
  ;; is pure, or that merely allocates, can go.  Allocation is unobservable
  ;; except through exhaustion, which is why *-effect stays.  [cco]
  (memq eff '(no-effect *-effect)))

(define-inline (gc-safe-effect? eff) ; cannot cause gc
  (memq eff '(no-effect ?-effect !-effect ?!-effect)))

(define *integrable-effects*
'((eq?) (eqv?) (equal? ?) (box *) (unbox ?) (set-box! !)
  (car ?) (set-car! !) (cdr ?) (set-cdr! !) (caar ?) (cadr ?)
  (cdar ?) (cddr ?) (null?) (pair?) (cons *) (not) (fxzero?)
  (fxpositive?) (fxnegative?) (fxeven?) (fxodd?) (fx+) (fx-)
  (fx*) (fx/) (fxquotient) (fxremainder) (fx<?) (fx>?) (fx<=?)
  (fx>=?) (fx=?) (fx!=?) (fxmin) (fxmax) (fxneg) (fxabs)
  (fixnum->flonum %) (fixnum?) (fxmodquo) (fxmodulo)
  (fxeucquo) (fxeucrem) (fxgcd) (fxexpt %) (fxsqrt) (fxnot)
  (fxand) (fxior) (fxxor) (fxsll) (fxsrl) (fxsra) (flzero?) (%ckp ?) (%ckv ?) (%cks ?) (%ckc ?) (%cki ?) (%ckj ?) (%ckn ?) (flsign-bit) (flfrexp * ?) (flldexp %)
  (flpositive?) (flnegative?) (fleven?) (flodd?) (flinteger?)
  (flnan?) (flfinite?) (flinfinite?) (fl+ %) (fl- %) (fl* %)
  (fl/ %) (flquotient %) (flremainder %) (fl<?) (fl>?) (fl<=?)
  (fl>=?) (fl=?) (fl!=?) (flmin) (flmax) (flneg) (flabs %)
  (flgcd %) (flexpt %) (flsqrt %) (flonum->fixnum)
  (flmodquo %) (flmodulo %) (flfloor %) (flceiling %)
  (fltruncate %) (flround %) (flonum?) (flexp %) (fllog %)
  (flsin %) (flcos %) (fltan %) (flasin %) (flacos %)
  (flatan %) (zero?) (positive?) (negative?) (+ %) (- %) (* %)
  (/ %) (<) (>) (<=) (>=) (=) (!=) (min) (max) (neg %) (abs %)
  (gcd %) (expt %) (sqrt %) (floor-quotient %)
  (floor-remainder %) (truncate-quotient %)
  (truncate-remainder %) (number?) (exp %) (log %) (sin %)
  (cos %) (tan %) (asin %) (acos %) (atan %) (rational?)
  (integer?) (nan?) (finite?) (infinite?) (even?) (odd?)
  (exact %) (inexact %) (floor %) (ceiling %) (truncate %)
  (round %) (list? ?) (list *) (make-list *) (length ?)
  (list-ref ?) (list-set! ? !) (list-cat * ?) (list-copy * ?)
  (circular?) (memq ?) (memv ?) (meme ?) (assq ?) (assv ?)
  (asse ?) (list-tail ?) (last-pair ?) (reverse * ?)
  (reverse! ? !) (char?) (char-whitespace?) (char-lower-case?)
  (char-upper-case?) (char-alphabetic?) (char-numeric?)
  (char-upcase) (char-downcase) (char-foldcase) (digit-value)
  (char-cmp) (char=?) (char<?) (char>?) (char<=?) (char>=?)
  (char-ci-cmp) (char-ci=?) (char-ci<?) (char-ci>?)
  (char-ci<=?) (char-ci>=?) (string?) (string *)
  (make-string *) (string-length) (string-ref ?)
  (string-set! !) (string-cat * ?) (substring * ?)
  (string-position ?) (string-upcase * ?)
  (string-downcase * ?) (string-foldcase * ?) (string-cmp ?)
  (string=? ?) (string<? ?) (string>? ?) (string<=? ?)
  (string>=? ?) (string-ci-cmp ?) (string-ci=? ?)
  (string-ci<? ?) (string-ci>? ?) (string-ci<=? ?)
  (string-ci>=? ?) (vector?) (vector *) (make-vector *)
  (vector-length) (vector-ref ?) (vector-set! !)
  (vector-cat *) (bytevector?) (bytevector *)
  (make-bytevector *) (bytevector-length)
  (bytevector-u8-ref ?) (bytevector-u8-set! !)
  (subbytevector * ?) (bytevector=? ?) (record?)
  (make-record *) (record-length) (record-ref ?)
  (record-set! !) (record-type-descriptor)
  (%vector->list1 * ?) (list->vector * ?) (%string->list1 * ?)
  (list->string * ?) (symbol->string *) (string->symbol ?)
  (fixnum->string *) (string->fixnum ?) (char->integer)
  (integer->char) (list->bytevector * ?) (flonum->string *)
  (string->flonum % ?) (number->string *) (string->number % ?)
  (symbol?) (boolean?) (box?) (shebang?) (symbol->shebang *)
  (shebang->symbol *) (void?) (void *) (%current-input-port ?)
  (%current-output-port ?) (%current-error-port ?)
  (%set-current-input-port! !) (%set-current-output-port! !)
  (%set-current-error-port! !) (procedure?) (input-port?)
  (output-port?) (tty-port?) (standard-input-port)
  (standard-output-port) (standard-error-port)
  (input-port-open? ?) (output-port-open? ?)
  (%open-input-file * ? !) (%open-output-file * ? !)
  (%open-binary-input-file * ? !)
  (%open-binary-output-file * ? !) (open-input-string * ?)
  (open-output-string * ?) (open-input-bytevector * ?)
  (open-output-bytevector * ?) (close-input-port ? !)
  (close-output-port ? !) (flush-output-port ? !)
  (port-fold-case? ?) (set-port-fold-case! ? !)
  (get-output-string * ?) (get-output-bytevector * ?)
  (read-char ? !) (peek-char ? !) (char-ready? ? !)
  (read-u8 ? !) (peek-u8 ? !) (u8-ready? ? !) (eof-object?)
  (eof-object) (write-char ? !) (write-u8 ? !) (display ? !)
  (write ? !) (newline ? !) (write-shared ? !)
  (write-simple ? !) (file-exists? ?) (delete-file ? !)
  (rename-file ? !) (get-environment-variable * ?)
  (current-jiffy * !) (jiffies-per-second *)
  (current-second *) (directory-separator) (immediate-hash)
  (complex?) (real?) (exact-integer?) (exact?) (inexact?)
  (modulo %) (quotient %) (remainder %) (exact->inexact %)
  (inexact->exact %) (boolean=?) (symbol=?) (caaar ?)
  (caadr ?) (cadar ?) (caddr ?) (cdaar ?) (cdadr ?) (cddar ?)
  (cdddr ?) (caaaar ?) (caaadr ?) (caadar ?) (caaddr ?)
  (cadaar ?) (cadadr ?) (caddar ?) (cadddr ?) (cdaaar ?)
  (cdaadr ?) (cdadar ?) (cdaddr ?) (cddaar ?) (cddadr ?)
  (cdddar ?) (cddddr ?)))

;;; Integrables that can raise, which is a different question from the
;;; effect they carry.
;;;
;;; The lattice answers two questions with one value, and they want
;;; different answers here.  Division is referentially transparent, so
;;; (/ 6 2) must still fold to 3 -- and the fold is guarded, so an
;;; operation that raises at compile time simply keeps its primapp and
;;; raises at run time instead.  But it is not removable when its value is
;;; dead, because dividing by zero stops the program.  Reclassifying the
;;; entry would fix the second and break the first, so the raising fact
;;; lives here, beside the table rather than inside it, and only
;;; exp-removable-if-dead? consults it.
;;;
;;; Every entry below was measured: the host rejects the stated argument.
;;; That makes the list sound for what it names and silent about what it
;;; does not, which a side list cannot help; the rewrite should fold this
;;; into the table as a raising component whose default is "may raise", so
;;; that an unannotated operation fails safe.  [cco]
(define *raising-integrables*
  '(/                              ; a zero divisor
    floor-quotient floor-remainder truncate-quotient truncate-remainder
    quotient remainder modulo       ; separate integrables from the above
    fx/ fxquotient fxremainder fxmodulo fxmodquo fxeucquo fxeucrem
    expt fxexpt                    ; (expt 0 -1)
    fxsqrt                         ; a negative argument
    exact inexact->exact           ; an infinity or a NaN
    log                            ; an exact zero
    atan                           ; (atan 0 0)
    ;; Added when the runtime gained tag checking.  These used to return
    ;; nonsense on a wrong type and so could not stop the program; now
    ;; they report, which by the rule above makes them non-removable.  The
    ;; two facts live in different files, so the rule is easy to break
    ;; without noticing: whenever a check is added to the runtime, ask
    ;; what it does to this set.  Measured cost on the filter probe: none,
    ;; 77 procedures kept either way, because a dead library definition
    ;; with a removable right-hand side is almost always a lambda.  [cco]
    + - * / < > <= >= = !=         ; the generic slow paths convert
    min max expt sqrt exp          ; through num_f64, which now reports
    sin cos tan asin acos
    flexpt flexp flsin flcos fltan ; the same macros, so the same path
    flasin flacos fllog10
    flsinh flcosh fltanh flgcd
    zero? positive? negative?      ; convert inline, guarded by cknum
    string-length vector-length    ; tag-checked since the length is read
    bytevector-length))            ; out of the payload

;; a primapp's prim is an integrable index here, but not every caller has
;; checked that; integrable-global throws on an index it does not know, so
;; an unknown one answers "may raise", which is the safe side
(define (integrable-may-raise? i)
  (if (and (integer? i) (integrable? i))
      (and (memq (integrable-global i) *raising-integrables*) #t)
      #t))

(define (integrable-effect i)
  (define q (assq (integrable-global i) *integrable-effects*))
  (cond [(not q)                  (*?!-effect)]
        [(equal? (cdr q) '())      (no-effect)]
        [(equal? (cdr q) '(*))      (*-effect)]
        [(equal? (cdr q) '(%))      (%-effect)]
        [(equal? (cdr q) '(?))      (?-effect)]
        [(equal? (cdr q) '(!))      (!-effect)]
        [(equal? (cdr q) '(? !))   (?!-effect)]
        [(equal? (cdr q) '(* ?))   (*?-effect)]
        [(equal? (cdr q) '(% ?))   (%?-effect)]
        [(equal? (cdr q) '(* ? !)) (*?!-effect)]
        [else                      (*?!-effect)]))

;------------------------------------------------------------------------------

; core expressions, stored directly as abstract syntax tree nodes

(define-variant quote-exp (obj))                      ; quote expression
(define-variant var-exp (id))                         ; variable reference
(define-variant varassign-exp (id exp))               ; variable assignment
(define-variant varaddress-exp (id))                  ; variable address
(define-variant if-exp (test-exp then-exp else-exp))  ; conditional
(define-variant primapp-exp (effect prim rands))      ; primitive application
(define-variant app-exp (rator rands))                ; application
(define-variant lambda-exp (ids body))                ; lambda expression
(define-variant ilambda-exp (ids id body))            ; improper lambda expression
(define-variant lambda*-exp (arities lambdas))        ; lambda* expression
(define-variant asm-exp (cstr))                       ; asm expression (rare)
(define-variant once-exp (id exp))                    ; once expression (top)
(define-variant fix-exp (ids lams body))              ; lambda-only letrec
(define-variant letcc-exp (id body))                  ; reify continuation
(define-variant withcc-exp (cont-exp exp))            ; reflect continuation


;------------------------------------------------------------------------------

; void expression is a distinguished literal expression -- not actual (void)!
; (define-variant void-exp ()), virtual

(define (void-exp) 
  (primapp-exp (no-effect) "" '()))
(define (void-exp? exp) 
  (and (primapp-exp? exp) (equal? (primapp-exp->prim exp) "")))

(define (void-or-false-exp? exp) ; xpand-labels uses void, s.scm letrec* uses (quote #f)
  (or (void-exp? exp)
      (and (quote-exp? exp) (eq? #f (quote-exp->obj exp)))))


;------------------------------------------------------------------------------

; gvarassign expression is an assignment to a global variable
; (define-variant gvarassign-exp (id exp)), virtual

(define-inline (gvarassign-exp id exp) 
  (varassign-exp id exp))

(define (gvarassign-exp? exp) 
  (and (varassign-exp? exp) (global-id? (varassign-exp->id exp))))

(define-inline (gvarassign-exp->id exp)  (varassign-exp->id exp))
(define-inline (gvarassign-exp->exp exp)  (varassign-exp->exp exp))


;------------------------------------------------------------------------------

; let expression is a pattern for lambda applications
; (define-variant let-exp (ids rands body)), virtual

(define (let-exp ids rands body) 
  (app-exp (lambda-exp ids body) rands))

(define (let-exp? exp) 
  (and (app-exp? exp)
       (let ([rator (app-exp->rator exp)])
         (and (lambda-exp? rator)
              (= (length (lambda-exp->ids rator)) 
                 (length (app-exp->rands exp)))))))

(define-inline (let-exp->ids exp) 
  (lambda-exp->ids (app-exp->rator exp)))

(define-inline (let-exp->rands exp) 
  (app-exp->rands exp))

(define-inline (let-exp->body exp) 
  (lambda-exp->body (app-exp->rator exp)))


;------------------------------------------------------------------------------

; identity-lambda expression is a pattern for likes of (lambda (x) x)
; (define-variant identity-lambda-exp ()), virtual

(define (identity-lambda-exp)
  (let ([id (lexical-id '_)])
    (lambda-exp (list id) (var-exp id)))) 

(define (identity-lambda-exp? exp)
  (and (lambda-exp? exp)
       (let ([ids (lambda-exp->ids exp)] 
             [body (lambda-exp->body exp)])
         (and (pair? ids) (null? (cdr ids))
              (var-exp? body) (eq? (car ids) (var-exp->id body))))))


;------------------------------------------------------------------------------

; identity-let expression is a pattern for identity lambda applications
; (define-variant identity-let-exp (exp)), virtual

(define (identity-let-exp exp)
  (app-exp (identity-lambda-exp) (list exp)))

(define (identity-let-exp? exp)
  (and (app-exp? exp)
       (identity-lambda-exp? (app-exp->rator exp))
       (let ([rands (app-exp->rands exp)]) 
         (and (pair? rands) (null? (cdr rands))))))

(define-inline (identity-let-exp->exp exp) 
  (car (app-exp->rands exp)))


;------------------------------------------------------------------------------

; null-let expression is a pattern for superfluous lambda applications
; (define-variant null-let-exp (body)), virtual

(define (null-let-exp body) 
  (app-exp (lambda-exp '() body) '()))

(define (null-let-exp? exp)
  (and (app-exp? exp)
       (null? (app-exp->rands exp))
       (let ([rator (app-exp->rator exp)]) 
         (and (lambda-exp? rator)
              (null? (lambda-exp->ids rator))))))

(define-inline (null-let-exp->body exp) 
  (lambda-exp->body (app-exp->rator exp)))


;------------------------------------------------------------------------------

; degenerate-let expression is either identity-let or null-let
; (define-variant degenerate-let-exp (body)), virtual

(define (degenerate-let-exp? exp)
  (or (null-let-exp? exp) (identity-let-exp? exp)))

(define (degenerate-let-exp->body exp)
  (if (null? (app-exp->rands exp))
      (null-let-exp->body exp)
      (identity-let-exp->exp exp)))


;------------------------------------------------------------------------------

; begin expression is a pattern for lambda applications with unreferenced vars
; (define-variant begin-exp (exp1 exp2)), virtual

(define (begin-exp exp1 exp2) 
  (app-exp (lambda-exp (list (lexical-id '_)) exp2) 
    (list exp1)))

(define (begin-exp? exp) 
  (variant-case exp
    [let-exp (ids rands body)
     (and (pair? ids) (null? (cdr ids))
          (not (var-referenced-in-exp? (car ids) body)))]
    [else #f]))

(define-inline (begin-exp->exp1 exp) 
  (car (app-exp->rands exp)))

(define-inline (begin-exp->exp2 exp) 
  (lambda-exp->body (app-exp->rator exp)))


;------------------------------------------------------------------------------

; rec expression is a pattern for self-recursive lambda definition
; (define-variant rec-exp (id lam)), virtual

(define (rec-exp? exp)
  (and (fix-exp? exp)
       (let ([ids (fix-exp->ids exp)] [body (fix-exp->body exp)])
         (and (pair? ids) (null? (cdr ids)) (var-exp? body)
              (eq? (car ids) (var-exp->id body))))))

(define-inline (rec-exp->id exp) 
  (car (fix-exp->ids exp)))

(define-inline (rec-exp->lam exp) 
  (car (fix-exp->lams exp)))


;------------------------------------------------------------------------------

; loop expression is a pattern for self-recursive lambda application
; (define-variant loop-exp (id lam rands)), virtual

(define (loop-exp? exp)
  (and (app-exp? exp) (rec-exp? (app-exp->rator exp))))

(define-inline (loop-exp->id exp) 
  (rec-exp->id (app-exp->rator exp)))

(define-inline (loop-exp->lam exp) 
  (rec-exp->lam (app-exp->rator exp)))

(define-inline (loop-exp->rands exp) 
  (app-exp->rands exp))


;------------------------------------------------------------------------------

; labelapp expression is a pattern for label variable applications
; (define-variant labelapp-exp (id rands)), virtual

(define (labelapp-exp id rands) 
  (app-exp (var-exp id) rands)) 

(define (labelapp-exp? exp)
  (and (app-exp? exp) 
    (var-exp? (app-exp->rator exp))
    (label-id? (var-exp->id (app-exp->rator exp)))))

(define-inline (labelapp-exp->id exp) 
  (var-exp->id (app-exp->rator exp)))

(define-inline (labelapp-exp->rands exp) 
  (app-exp->rands exp))


;------------------------------------------------------------------------------

; varassign* expression is a pattern for special let/set!/... combination
; used in expansions of standard-conformant letrec expressions
; (define-variant varassign*-exp (ids rands)), virtual

(define (varassign*-exp ids rands)
  (let ([tmp-ids (map (lambda (id) (lexical-id (id->symbol id))) ids)])
    (let-exp tmp-ids rands
      (let recur ([ids ids] [tmp-ids tmp-ids])
        (if (null? ids)
            (void-exp)
            (begin-exp
              (varassign-exp (car ids) (var-exp (car tmp-ids))) 
              (recur (cdr ids) (cdr tmp-ids))))))))

(define (varassign*-exp? exp)
  (variant-case exp
    [let-exp (ids rands body)
     (let assigns? ([tmp-ids ids] [body body])
       (or (and (null? tmp-ids) (void-exp? body))
           (variant-case body
             [begin-exp (exp1 exp2)
              (and (variant-case exp1
                     [varassign-exp (id exp)
                      (and (var-exp? exp)
                        (eq? (var-exp->id exp) (car tmp-ids))
                        (not (memq id ids)))]
                     [else #f])
                   (assigns? (cdr tmp-ids) exp2))]
             [else #f])))]
    [else #f]))

(define (varassign*-exp->ids exp)
  (let assignees ([body (let-exp->body exp)])
    (if (void-exp? body)
        '()
        (cons 
          (varassign-exp->id (begin-exp->exp1 body)) 
          (assignees (begin-exp->exp2 body)))))) 

(define-inline (varassign*-exp->rands exp)
  (let-exp->rands exp))


;------------------------------------------------------------------------------

; letrec expression is a pattern for special let/set!/... combination
; (define-variant letrec-exp (ids rands body)), virtual

(define (letrec-exp ids rands body)
  (let-exp ids (map (lambda (exp) (void-exp)) rands)
    (begin-exp (varassign*-exp ids rands) body)))
    
(define (letrec-exp? exp)
  (variant-case exp
    [let-exp (ids rands body)
     (and (pair? ids) 
          (andmap void-or-false-exp? rands)
          (begin-exp? body)
          (let ([exp1 (begin-exp->exp1 body)])
            (and (varassign*-exp? exp1)
                 (equal? ids (varassign*-exp->ids exp1)))))]
    [else #f]))

(define-inline (letrec-exp->ids exp)
  (let-exp->ids exp))

(define-inline (letrec-exp->rands exp)
  (varassign*-exp->rands (begin-exp->exp1 (let-exp->body exp))))

(define-inline (letrec-exp->body exp)
  (begin-exp->exp2 (let-exp->body exp)))


;------------------------------------------------------------------------------

; letrec* expression is a pattern for special let/set!/... combination
; (define-variant letrec*-exp (ids rands body)), virtual

(define (letrec*-exp ids rands body)
  (let-exp ids (map (lambda (exp) (void-exp)) rands)
    (let recur ([ids ids] [rands rands])
      (if (null? rands)
          body
          (begin-exp
            (varassign-exp (car ids) (car rands)) 
            (recur (cdr ids) (cdr rands)))))))

(define (letrec*-exp? exp)
  (variant-case exp
    [let-exp (ids rands body)
     (and (pair? ids) 
          (andmap void-or-false-exp? rands)
          (let inits? ([ids ids] [body body])
            (or (null? ids)
                (variant-case body
                  [begin-exp (exp1 exp2)
                   (and (variant-case exp1
                          [varassign-exp (id exp)
                           (eq? id (car ids))]
                          [else #f])
                        (inits? (cdr ids) exp2))]
                  [else #f]))))]
    [else #f]))

(define-inline (letrec*-exp->ids exp)
  (let-exp->ids exp))

(define (letrec*-exp->rands exp)
  (let inits ([ids (let-exp->ids exp)] [body (let-exp->body exp)])
    (if (null? ids)
        '()
        (cons 
          (varassign-exp->exp (begin-exp->exp1 body)) 
          (inits (cdr ids) (begin-exp->exp2 body)))))) 

(define (letrec*-exp->body exp)
  (let skip-inits ([ids (let-exp->ids exp)] [body (let-exp->body exp)])
    (if (null? ids)
        body
        (skip-inits (cdr ids) (begin-exp->exp2 body)))))

;------------------------------------------------------------------------------

; box expression is a pattern for read/write storage cell allocation
; (define-variant box-exp (exp)), virtual

(define the-box-prim (lookup-integrable 'box))

(define (box-exp exp)
  (primapp-exp (*-effect)
    the-box-prim (list exp))) 

(define (box-exp? exp) 
  (and (primapp-exp? exp) 
       (eq? (primapp-exp->prim exp) the-box-prim)))

(define-inline (box-exp->exp exp)
  (car (primapp-exp->rands exp)))


;------------------------------------------------------------------------------

; boxref expression is a pattern for read/write storage cell reference
; (define-variant boxref-exp (box)), virtual

(define the-boxref-prim (lookup-integrable 'unbox))

(define (boxref-exp box)
  (primapp-exp (?-effect) 
     the-boxref-prim (list box)))

(define (boxref-exp? exp) 
  (and (primapp-exp? exp) 
       (eq? (primapp-exp->prim exp) the-boxref-prim)))

(define-inline (boxref-exp->box exp)
  (car (primapp-exp->rands exp)))


;------------------------------------------------------------------------------

; boxset expression is a pattern for read/write storage cell mutation
; (define-variant boxset-exp (box exp)), virtual

(define the-boxset-prim (lookup-integrable 'set-box!))

(define (boxset-exp box exp)
  (primapp-exp (!-effect) 
     the-boxset-prim (list box exp)))

(define (boxset-exp? exp) 
  (and (primapp-exp? exp) 
       (eq? (primapp-exp->prim exp) the-boxset-prim)))

(define-inline (boxset-exp->box exp)
  (car (primapp-exp->rands exp)))

(define-inline (boxset-exp->exp exp)
  (cadr (primapp-exp->rands exp)))


;------------------------------------------------------------------------------

; letbox expression is a pattern for special let-box combination left by
; remove-assignments pass

(define (letbox-exp bids ids body)
  (let-exp bids (map (compose box-exp var-exp) ids) body))

(define (letbox-exp? exp)
  (variant-case exp
    [let-exp (ids rands body)
     (let loop ([ids ids] [rands rands])
       (if (and (null? ids) (null? rands))
           #t
           (let ([rand (car rands)])
             (and (box-exp? rand)
                  (var-exp? (box-exp->exp rand))
                  (not (var-assigned-in-exp? (car ids) body))
                  (not (var-referenced-in-exp? (var-exp->id (box-exp->exp rand)) body))
                  (loop (cdr ids) (cdr rands))))))]
    [else #f]))

(define-inline (letbox-exp->bids exp)
  (let-exp->ids exp))

(define (letbox-exp->ids exp)
  (map (compose var-exp->id box-exp->exp) (let-exp->rands exp)))

(define-inline (letbox-exp->body exp)
  (let-exp->body exp))

;------------------------------------------------------------------------------

; memoization of var/exp attributes (ideally, it should use weak hash table)

(define *memoization-optsz* 500)
(define *memoization-limit* 1000)

(define (memoizate-exp-attribute fn)
  (define memo '())
  (define mlen 0)
  (lambda (exp)
    (cond [(not exp) (set! memo '()) (set! mlen 0)]
          [(assq exp memo) => cdr]
          [else (let ([res (fn exp)]) 
                  (set! memo (cons (cons exp res) memo))
                  (set! mlen (+ mlen 1))
                  (when (> mlen *memoization-limit*)
                    (set-cdr! (list-tail memo *memoization-optsz*) '())
                    (set! mlen (length memo)))
                  res)])))

(define (memoizate-var-attribute fn)
  (define memo '())
  (define mlen 0)
  (lambda (id exp)
    (cond [(not id) (set! memo '()) (set! mlen 0)]
          [(assq exp memo) => 
           (lambda (exp&idmemo)
             (cond [(assq id (cdr exp&idmemo)) => cdr]
                   [else (let ([res (fn id exp)]) 
                           (set-cdr! exp&idmemo
                             (cons (cons id res) (cdr exp&idmemo)))
                           res)]))]
          [else (let ([res (fn id exp)]) 
                  (set! memo 
                    (cons (cons exp (list (cons id res))) memo))
                  (set! mlen (+ mlen 1))
                  (when (> mlen *memoization-limit*)
                    (set-cdr! (list-tail memo *memoization-optsz*) '())
                    (set! mlen (length memo)))
                  res)])))


;------------------------------------------------------------------------------

; information on variable use (free/global variables) and ctypes

; description of single instance of variable use
(define-variant var-use (assignment-to? application-of?))

(define exp-vinfo 
  (memoizate-exp-attribute
    (lambda (exp) ; => ((id . var-use) ...)
      (variant-case exp
        [var-exp (id) 
         (list (cons id (var-use #f #f)))]
        [varassign-exp (id exp)
         (cons (cons id (var-use #t #f)) (exp-vinfo exp))]
        [varaddress-exp (id)
         (list (cons id (var-use #t #f)))]
        [if-exp (test-exp then-exp else-exp) 
         (append (exp-vinfo test-exp) (exp-vinfo then-exp) (exp-vinfo else-exp))]
        [primapp-exp (effect prim rands)
         (let loop ([rands rands] [n 0] [vi '()])
           (if (null? rands) vi
             (let ([exp (car rands)])
               (variant-case exp
                 [var-exp (id) 
                  (let ([id&vu (cons id (var-use #f #f))])
                    (loop (cdr rands) (+ n 1) (cons id&vu vi)))]
                 [else (loop (cdr rands) (+ n 1) (append (exp-vinfo exp) vi))]))))]
        [app-exp (rator rands)
         (reduce-right (lambda (exp vi) (append (exp-vinfo exp) vi))
           (variant-case rator
             [var-exp (id) (list (cons id (var-use #f #t)))]
             [else (exp-vinfo rator)])
           rands)]
        [lambda-exp (ids body)
         (keep (lambda (id&vu) (not (memq (car id&vu) ids)))
           (exp-vinfo body))]
        [ilambda-exp (ids id body)
         (keep (lambda (id&vu) (not (memq (car id&vu) (cons id ids))))
           (exp-vinfo body))]
        [lambda*-exp (arities lambdas)
         (apply append (map exp-vinfo lambdas))]
        [once-exp (id exp) (exp-vinfo exp)] 
        [fix-exp (ids lams body)
         (keep (lambda (id&vu) (not (memq (car id&vu) ids)))
           (reduce-right (lambda (exp vi) (append (exp-vinfo exp) vi))
             (exp-vinfo body) lams))]
        [letcc-exp (id body)
         (keep (lambda (id&vu) (not (eq? (car id&vu) id)))
           (exp-vinfo body))]     
        [withcc-exp (cont-exp exp)
         (append (exp-vinfo cont-exp) (exp-vinfo exp))]
        [else '()]))))

(define var-uses-in-exp  ;id exp => (var-use ...)
  (memoizate-var-attribute
    (lambda (id exp)
      (let loop ([vinfo (exp-vinfo exp)] [uses '()])
        (cond [(null? vinfo) uses]
              [(eq? id (caar vinfo)) 
               (loop (cdr vinfo) (cons (cdar vinfo) uses))]
              [else (loop (cdr vinfo) uses)])))))

(define (var-referenced-in-exp? id exp)
  (pair? (var-uses-in-exp id exp)))
  
(define (var-assigned-in-exp? id exp)
  (ormap var-use->assignment-to? (var-uses-in-exp id exp)))

(define (var-reference-count id exp)
  (length (var-uses-in-exp id exp)))

(define (var-assignment-count id exp)
  (reduce-left (lambda (vu n) (if (var-use->assignment-to? vu) (+ n 1) n))
    0 (var-uses-in-exp id exp)))

(define (var-only-applied-in-exp? id exp)
  (andmap var-use->application-of? (var-uses-in-exp id exp))) 

(define exp->free-vars ; exp -> {id ...}
  (memoizate-exp-attribute
    (lambda (exp)
      (reduce-left 
        (lambda (id&vu res) 
          (adjoinq (car id&vu) res))
        '() (exp-vinfo exp)))))

(define exp->free-lexvars ; exp -> {id ...}
  (memoizate-exp-attribute
    (lambda (exp)
      (reduce-left 
        (lambda (id&vu res)
          (if (global-id? (car id&vu)) res (adjoinq (car id&vu) res)))
        '() (exp-vinfo exp)))))
        
;------------------------------------------------------------------------------

; parser for macroexpanded source code (core scheme)

(define (parse-core core return) ;=> (return exp known-refs)
  (define (pcore x e)
    (let p ([x x])
      (record-case x
        [quote (obj)
         (quote-exp obj)]
        [gref (gid) 
         (var-exp (make-id gid e #f))] ; CUPS: unified eq? global ids (gref/gset!/ref/const)
        [gset! (gid exp) 
         (varassign-exp (make-id gid e #f) (p exp))]
        [const (id) 
         (var-exp (make-id id e #t))]
        [ref (id) 
         (var-exp (make-id id e #f))]
        [set! (id exp) 
         (varassign-exp (make-id id e #f) (p exp))]
        [set& (id) 
         (varaddress-exp (make-id id e #f))]
        [lambda (idsi exp)
         (let ([ids (map lexical-id (flatten-idslist idsi))])
           (if (list? idsi) (lambda-exp ids (pcore exp (extendify ids e)))
               (ilambda-exp (cdr ids) (car ids) (pcore exp (extendify ids e)))))]
        [lambda* clauses
         (lambda*-exp (map car clauses) (map (compose p cadr) clauses))]
        [letcc (kid exp)
         (let ([id (lexical-id kid)])
           (letcc-exp id (pcore exp (extendify (list id) e))))]
        [withcc (kexp exp)
         (withcc-exp (p kexp) (p exp))]
        [if (ce te ee)
         (if-exp (p ce) (p te) (p ee))]
        [begin exps
         (reduce-right/singular-identity 
           begin-exp (void-exp) (map p exps))]
        [integrable (i . args)
         (primapp-exp (integrable-effect i) i (map p args))]
        [call (exp . args)
         (app-exp (p exp) (map p args))]
        [asm (cstr)
         (asm-exp cstr)]
        [once (gid exp)
         (if (memq gid seen-once-ids)
             (void-exp)
             (begin (set! seen-once-ids (cons gid seen-once-ids)) (p exp)))]
        [(define define-syntax define-library import) tail 
         (c-error "misplaced definition form" x)]
        [else (c-error "unexpected <core> form" x)])))
  (define seen-once-ids '()) ; CUPS: static (once ...) dedup for whole-program parse
  (define global-e 
    '())
  (define (make-id sym e const?)
    (or (lookupicate sym e)
        (lookupicate sym global-e)
        (let ([id (if const? (global-id-constant sym) (global-id sym))])
          (set! global-e (cons id global-e))
          id)))
  (let ([exp (pcore core '())])
    (return exp (map id->symbol global-e))))


;------------------------------------------------------------------------------

; Transformation of letrec and letrec* into fix-exp

; See @article{
;  key = "Wad:05"
;  author = "Oscar Waddell and Dipanwita Sarkar and R. Kent Dybvig"
;  title = "Fixing Letrec: A Faithful Yet Efficient Implementation of Scheme's 
;  Recursive Binding Construct"
;  journal = "Higher-Order and Symbolic Computation"
;  volume = "18"
;  number = "3-4"
;  year = "2005"
;  url = "http://www.cs.indiana.edu/~dyb/pubs/fixing-letrec.pdf"
; }

(define (fix-letrecs exp)

  (define (fl-simple? exp) ; having no way to access its continuation
    (variant-case exp
      [quote-exp (obj) #t]
      [var-exp (id) #t]
      [if-exp (test-exp then-exp else-exp)
       (andapp fl-simple? test-exp then-exp else-exp)]
      [primapp-exp (effect prim rands)
       (and (ref-transparent-effect? effect)
         (andmap fl-simple? rands))]
      [lambda-exp (ids body) #t]
      [ilambda-exp (ids id body) #t]
      [lambda*-exp (arities lambdas) #t]
      [let-exp (ids rands body)
       (and (andmap fl-simple? rands) (fl-simple? body))]
      [withcc-exp (cont-exp exp)
       (andapp fl-simple? cont-exp exp)]
      [else #f]))

  (define-variant bnd (kind id exp))
  (define (u-bnd id exp) (bnd 'u id exp))     ; unreferenced
  (define (u-bnd? b) (eq? 'u (bnd->kind b)))
  (define (s-bnd id exp) (bnd 's id exp))     ; simple
  (define (s-bnd? b) (eq? 's (bnd->kind b)))
  (define (l-bnd id exp) (bnd 'l id exp))     ; lambda
  (define (l-bnd? b) (eq? 'l (bnd->kind b)))
  (define (c-bnd id exp) (bnd 'c id exp))     ; complex
  (define (c-bnd? b) (eq? 'c (bnd->kind b)))

  (define (classify-bindings ids rands body)
    (define rr&b-vi ; ((id . var-use) ...)
      (reduce-right (lambda (exp vi) (append (exp-vinfo exp) vi))
        (exp-vinfo body) rands))
    ;; variables assigned (or address-taken) by some init of this
    ;; group: a "simple" init is evaluated early, before the
    ;; group's varassign* runs the complex inits in order, so it
    ;; must not reference any of them -- its value could otherwise
    ;; be read before a sibling init's assignment lands (this is
    ;; not letrec-id reference, already excluded below, but free
    ;; variables mutated through varassign/varaddress, e.g. a
    ;; define init reading a variable a previous define's init
    ;; updates via (set& ...) or set!)
    (define assigned-ids
      (map car
        (keep (lambda (id&vu) (var-use->assignment-to? (cdr id&vu)))
              (reduce-right (lambda (exp vi) (append (exp-vinfo exp) vi))
                '() rands))))
    (define (classify id exp)
      (define id-vu* 
        (map cdr (keep (lambda (id&vu) (eq? (car id&vu) id)) rr&b-vi)))
      (cond 
        [(null? id-vu*) 
         (u-bnd id exp)] ; unreferenced
        [(ormap var-use->assignment-to? id-vu*)
         (c-bnd id exp)] ; complex 
        [(and (fl-simple? exp)
           (let ([vi (exp-vinfo exp)])
             (and (andmap (lambda (id) (not (assq id vi))) ids)
                  (andmap (lambda (id&vu)
                            (not (memq (car id&vu) assigned-ids)))
                          vi))))    
         (s-bnd id exp)] ; simple
        [(or (lambda-exp? exp) (ilambda-exp? exp) (lambda*-exp? exp))
         (l-bnd id exp)] ; lambda
        [else (c-bnd id exp)])) ; remaining are complex 
    (map classify ids rands))
  
  (define (fix exp)
    (variant-case exp
      [quote-exp (obj) exp]
      [var-exp (id) exp]
      [varassign-exp (id exp)
       (varassign-exp id (fix exp))]
      [varaddress-exp (id) exp]
      [if-exp (test-exp then-exp else-exp)
       (if-exp (fix test-exp) (fix then-exp) (fix else-exp))]
      [degenerate-let-exp (body)
       (fix body)]
      [letrec-exp (ids rands body)
       (let ([bnd* (classify-bindings ids rands body)])
         (let ([ub* (keep u-bnd? bnd*)] [sb* (keep s-bnd? bnd*)]
               [lb* (keep l-bnd? bnd*)] [cb* (keep c-bnd? bnd*)])
           (let-exp
             (append 
               (map bnd->id sb*)
               (map bnd->id cb*))
             (append 
               (map (compose fix bnd->exp) sb*) 
               (map (lambda (b) (void-exp)) cb*))
             (refix 
               (map bnd->id lb*) 
               (map (compose fix bnd->exp) lb*)
               (reduce-right/right-seed
                 begin-exp
                 (if (null? cb*)
                     (fix body)
                     (begin-exp
                       (varassign*-exp 
                         (map bnd->id cb*)
                         (map (compose fix bnd->exp) cb*))
                       (fix body)))
                 (map (compose fix bnd->exp) ub*))))))] 
      [letrec*-exp (ids rands body)
       (let ([bnd* (classify-bindings ids rands body)])
         (let ([sb* (keep s-bnd? bnd*)] [cb* (keep c-bnd? bnd*)]
               [lb* (keep l-bnd? bnd*)])
           (let-exp
             (append 
               (map bnd->id sb*)
               (map bnd->id cb*))
             (append 
               (map (compose fix bnd->exp) sb*) 
               (map (lambda (b) (void-exp)) cb*))
             (refix 
               (map bnd->id lb*) 
               (map (compose fix bnd->exp) lb*)
               (let recur ([cub* (keep (lambda (b) (or (c-bnd? b) (u-bnd? b))) bnd*)])
                 (if (null? cub*)
                     (fix body)
                     (let ([b (car cub*)])
                       (begin-exp
                         (if (c-bnd? b)
                             (varassign-exp (bnd->id b) (fix (bnd->exp b)))
                             (fix (bnd->exp b))) 
                         (recur (cdr cub*))))))))))]
      [primapp-exp (effect prim rands)
       (primapp-exp effect prim (map fix rands))]
      [app-exp (rator rands)
       (app-exp (fix rator) (map fix rands))]
      [lambda-exp (ids body)
       (lambda-exp ids (fix body))]
      [ilambda-exp (ids id body)
       (ilambda-exp ids id (fix body))]
      [lambda*-exp (arities lambdas)
       (lambda*-exp arities (map fix lambdas))]
      [asm-exp (cstr) exp]
      [once-exp (id exp) 
       (once-exp id (fix exp))]
      [letcc-exp (id body)
       (letcc-exp id (fix body))]
      [withcc-exp (cont-exp exp)
       (withcc-exp (fix cont-exp) (fix exp))]))
    
  (define (reclassify-lambda-bindings ids lams)
    (define (reclassify id lam)
      (let ([vi (exp-vinfo lam)])
        (if (andmap (lambda (id) (not (assq id vi))) ids)
            (s-bnd id lam) ; new simple
            (l-bnd id lam)))) ; remains a lambda 
    (map reclassify ids lams))

  (define (refix ids lams body)
    (let ([bnd* (reclassify-lambda-bindings ids lams)])
      (let ([sb* (keep s-bnd? bnd*)] [lb* (keep l-bnd? bnd*)])
        (if (null? sb*) ; remaining lambdas are recursive
            (if (null? lb*) body (fix-exp ids lams body)) ; don't partition for now
            (let-exp (map bnd->id sb*) (map bnd->exp sb*)
              (refix (map bnd->id lb*) (map bnd->exp lb*) body))))))

  (fix exp))


;------------------------------------------------------------------------------

; Assignment elimination

(define (remove-assignments exp)

  (define (box-subst exp substs)
  
    (define (box exp)
      (variant-case exp
        [quote-exp (obj) exp]
        [gvarassign-exp (id exp) ; globals remain assignable
         (gvarassign-exp id (box exp))] 
        [var-exp (id)
         (cond [(assq id substs) => 
                (lambda (id&box) (boxref-exp (cdr id&box)))]
               [else exp])]
        [varassign-exp (id exp)
         (cond [(assq id substs) => 
                (lambda (id&box) 
                  (boxset-exp (cdr id&box) (box exp)))]
               [else (c-error "boxing error???")])]
        [varaddress-exp (id)
         (cond [(assq id substs) =>
                (lambda (id&box) (cdr id&box))]
               [else (c-error "boxing error???")])]
        [if-exp (test-exp then-exp else-exp)
         (if-exp (box test-exp) (box then-exp) (box else-exp))]
        [degenerate-let-exp (body)
         (box body)]
        [primapp-exp (effect prim rands)
         (primapp-exp effect prim (map box rands))]
        [app-exp (rator rands)
         (app-exp (box rator) (map box rands))]
        [fix-exp (ids lams body) ; leave as-is for now
         (fix-exp ids (map box lams) (box body))]
        [lambda-exp (ids body)
         (lambda-exp ids (box-body ids body))]
        [ilambda-exp (ids id body)
         (ilambda-exp ids id (box-body (cons id ids) body))]
        [lambda*-exp (arities lambdas)
         (lambda*-exp arities (map box lambdas))]
        [asm-exp (cstr) exp]
        [once-exp (id exp) (once-exp id (box exp))]
        [letcc-exp (id body)
         (if (var-assigned-in-exp? id body)
             (let ([box-id (lexical-id (symbol (id->symbol id) "-box"))])
               (letcc-exp id
                 (let-exp (list box-id) (list (box-exp (var-exp id)))
                   (box-subst body (cons (cons id (var-exp box-id)) substs)))))
             (letcc-exp id (box body)))]
        [withcc-exp (cont-exp exp)
         (withcc-exp (box cont-exp) (box exp))]))
    
    (define (box-body in-ids body)
      (let loop ([in-ids in-ids] [let-ids '()]
                 [let-rands '()] [substs substs])
           (if (null? in-ids)
               (let-exp (reverse let-ids) (reverse let-rands)
                  (box-subst body substs))
               (let ([id (car in-ids)])
                 (if (var-assigned-in-exp? id body)
                     (let ([box-id (lexical-id (symbol (id->symbol id) "-box"))])
                       (loop (cdr in-ids) (cons box-id let-ids) 
                         (cons (box-exp (var-exp id)) let-rands)
                           (cons (cons id (var-exp box-id)) substs)))
                     (loop (cdr in-ids) let-ids let-rands substs))))))

    (box exp)) 
     
  (box-subst exp '()))


;------------------------------------------------------------------------------

; converter of processed expression back to core expression

(define (unparse-core exp)

  (define (core exp)
    (variant-case exp
      [void-exp () '(begin)]
      [quote-exp (obj) (list 'quote obj)]
      [var-exp (id)
       (list (if (global-id? id) 'gref 'ref) (id->uname id))]
      [varassign-exp (id exp)
       (list (if (global-id? id) 'gset! 'set!) (id->uname id) (core exp))]
      [varaddress-exp (id)
       (list 'set& (id->uname id))]
      [if-exp (test-exp then-exp else-exp)
       (list 'if (core test-exp) (core then-exp) (core else-exp))]
      [degenerate-let-exp (body)
       (core body)]
      [begin-exp (exp1 exp2)
       (let ([core2 (core exp2)])
         (if (eq? (car core2) 'begin)
             (cons 'begin (cons (core exp1) (cdr core2)))
             (list 'begin (core exp1) core2)))]
      [fix-exp (ids lams body)
       (core (letrec-exp ids lams body))]
      [primapp-exp (effect prim rands)
       (if (integrable? prim)
           (list* 'integrable prim (map core rands))
           (c-error "invalid integrable?" exp))]
      [app-exp (rator rands)
       (cons 'call (cons (core rator) (map core rands)))]
      [lambda-exp (ids body)
       (list 'lambda (map id->uname ids) (core body))]
      [ilambda-exp (ids id body)
       (list 'lambda (append (map id->uname ids) (id->uname id)) (core body))]
      [lambda*-exp (arities lambdas)
       (cons 'lambda* (map (lambda (a l) (list a (core l))) arities lambdas))]
      [asm-exp (cstr)
       (list 'asm cstr)]
      [once-exp (id exp)
       (list 'once (id->uname id) (core exp))]
      [letcc-exp (id body)
       (list 'letcc (id->uname id) (core body))]
      [withcc-exp (cont-exp exp)
       (list 'withcc (core cont-exp) (core exp))]   
      [else (c-error "unknown exp" exp)]))
  
  (core exp))



;------------------------------------------------------------------------------

; unparser for parsed expression (for debuggung &c.)

(define (source exp)
  (define (source-body exp) ;=> (s ...)
    (variant-case exp
      [degenerate-let-exp (body) (source-body body)]
      [begin-exp (exp1 exp2) (cons (source exp1) (source-body exp2))]
      [else (list (source exp))]))    
  (variant-case exp
    [void-exp ()
     '%%void]
    [quote-exp (obj)
     (list 'quote obj)]
    [var-exp (id)
     (list 'ref (id->uname id))]
    [varassign-exp (id exp)
     (list 'set! (id->uname id) (source exp))]
    [varaddress-exp (id)
     (list 'set& (id->uname id))]
    [if-exp (test-exp then-exp else-exp)
     (list 'if (source test-exp) (source then-exp) (source else-exp))]
    [degenerate-let-exp (body)
     (source body)]
    [letbox-exp (bids ids body)
     (list* 'letbox 
       (map (lambda (bi i) (list (id->uname bi) (id->uname i))) bids ids)
       (source-body body))]
    [letrec-exp (ids rands body)
     (list* 'letrec 
       (map (lambda (id exp) (list (id->uname id) (source exp))) ids rands)
       (source-body body))]
    [letrec*-exp (ids rands body)
     (list* 'letrec* 
       (map (lambda (id exp) (list (id->uname id) (source exp))) ids rands)
       (source-body body))]
    [begin-exp (exp1 exp2)
     (list* 'begin (source exp1) (source-body exp2))]
    [let-exp (ids rands body)
     (list* 'let 
       (map (lambda (id exp) (list (id->uname id) (source exp))) ids rands)
       (source-body body))]
    [fix-exp (ids lams body)
     (list* 'let '() 
       (append
         (map (lambda (id lam)
                (variant-case lam
                  [lambda-exp (ids body)
                   (list* 'define 
                     (map id->uname (cons id ids))
                     (source-body body))]))
               ids lams)
         (source-body body)))]
    [primapp-exp (effect prim rands)
     (list* 'integrable effect prim (map source rands))]
    [app-exp (rator rands)
     (cons 'call (cons (source rator) (map source rands)))]
    [lambda-exp (ids body)
     (list* 'lambda (map id->uname ids) (source-body body))]
    [ilambda-exp (ids id body)
     (list* 'lambda (append (map id->uname ids) (id->uname id)) (source-body body))]
    [lambda*-exp (arities lambdas)
     (cons 'lambda* (map (lambda (a l) (list a (source l))) arities lambdas))]
    [asm-exp (cstr)
     (list 'asm cstr)]
    [once-exp (id exp)
     (list 'once (id->uname id) (source exp))]
    [letcc-exp (id body)
     (list* 'letcc (id->uname id)
       (source-body body))]
    [withcc-exp (cont-exp exp)
     (list 'withcc (source cont-exp) (source exp))]   
    [else (c-error "unknown exp" exp)]))
