
(define-syntax define-inline
  (syntax-rules ()
    [(_ (id v ...) rid expr)
     (begin
       (define-syntax id
         (syntax-rules ()
           [(_ v ...) expr] ; NB: do not use the same var twice!
           [_ rid]))
       (define rid (lambda (v ...) expr)))]))


;---------------------------------------------------------------------------------------------
; Equivalence predicates
;---------------------------------------------------------------------------------------------

(define-inline (eq? x y) %residual-eq? (%isq x y))

(define-inline (eqv? x y) %residual-eqv? (%isv x y))

(define-inline (equal? x y) %residual-equal? (%ise x y))


;---------------------------------------------------------------------------------------------
; Exact integer numbers (fixnums)
;---------------------------------------------------------------------------------------------

(define-inline (fixnum? x) %residual-fixnum? (%fixp x))

(define-inline (fxzero? x) %residual-fxzero? (%izerop (%cki x)))

(define-inline (fxpositive? x) %residual-fxpositive? (%iposp (%cki x)))

(define-inline (fxnegative? x) %residual-fxnegative? (%inegp (%cki x)))

(define-inline (fx+ x y) %residual-fx+ (%iadd (%cki x) (%cki y)))

(define-inline (fx* x y) %residual-fx* (%imul (%cki x) (%cki y)))

(define-inline (fx- x y) %residual-fx- (%isub (%cki x) (%cki y)))

(define-inline (fx/ x y) %residual-fx/ (%idiv (%cki x) (%cki y)))

(define-inline (fxquotient x y) %residual-fxquotient (%iquo (%cki x) (%cki y)))

(define-inline (fxremainder x y) %residual-fxremainder (%irem (%cki x) (%cki y)))

(define-inline (fxmodquo x y) %residual-fxmodquo (%imqu (%cki x) (%cki y)))

(define-inline (fxmodulo x y) %residual-fxmodulo (%imlo (%cki x) (%cki y)))

(define-inline (fxeucquo x y) %residual-fxeucquo (%ieuq (%cki x) (%cki y))) ;euclidean-quotient

(define-inline (fxeucrem x y) %residual-fxeucrem (%ieur (%cki x) (%cki y))) ;euclidean-remainder

(define-inline (fxneg x) %residual-fxneg (%ineg (%cki x)))

(define-inline (fxabs x) %residual-fxabs (%iabs (%cki x)))

(define-inline (fx<? x y) %residual-fx<? (%ilt (%cki x) (%cki y)))

(define-inline (fx<=? x y) %residual-fx<=? (%ile (%cki x) (%cki y)))

(define-inline (fx>? x y) %residual-fx>? (%igt (%cki x) (%cki y)))

(define-inline (fx>=? x y) %residual-fx>=? (%ige (%cki x) (%cki y)))

(define-inline (fx=? x y) %residual-fx=? (%ieq (%cki x) (%cki y)))

(define-inline (fxmin x y) %residual-fxmin (%imin (%cki x) (%cki y)))

(define-inline (fxmax x y) %residual-fxmax (%imax (%cki x) (%cki y)))

(define-inline (fixnum->flonum x) %residual-fixnum->flonum (%itoj (%cki x)))


;---------------------------------------------------------------------------------------------
; Inexact floating-point numbers (flonums)
;---------------------------------------------------------------------------------------------

(define-inline (flonum? x) %residual-flonum? (%flop x))

(define-inline (flzero? x) %residual-flzero? (%jzerop (%ckj x)))

(define-inline (flpositive? x) %residual-flpositive? (%jposp (%ckj x)))

(define-inline (flnegative? x) %residual-flnegative? (%jnegp (%ckj x)))

(define-inline (flinteger? x) %residual-flinteger? (%jintp (%ckj x))) 

(define-inline (flnan? x) %residual-flnan? (%jnanp (%ckj x))) 

(define-inline (flinfinite? x) %residual-flinfinite? (%jinfp (%ckj x))) 

(define-inline (flfinite? x) %residual-flfinite? (%jfinp (%ckj x))) 

(define-inline (fleven? x) %residual-fleven? (%jevnp (%ckj x)))

(define-inline (flodd? x) %residual-flodd? (%joddp (%ckj x)))

(define-inline (fl+ x y) %residual-fl+ (%jadd (%ckj x) (%ckj y)))

(define-inline (fl- x y) %residual-fl- (%jsub (%ckj x) (%ckj y)))

(define-inline (fl* x y) %residual-fl* (%jmul (%ckj x) (%ckj y)))

(define-inline (fl/ x y) %residual-fl/ (%jdiv (%ckj x) (%ckj y)))

(define-inline (flneg x) %residual-flneg (%jneg (%ckj x)))

(define-inline (flabs x) %residual-flabs (%jabs (%ckj x)))

(define-inline (fl<? x y) %residual-fl<? (%jlt (%ckj x) (%ckj y)))

(define-inline (fl<=? x y) %residual-fl<=? (%jle (%ckj x) (%ckj y)))

(define-inline (fl>? x y) %residual-fl>? (%jgt (%ckj x) (%ckj y)))

(define-inline (fl>=? x y) %residual-fl>=? (%jge (%ckj x) (%ckj y)))

(define-inline (fl=? x y) %residual-fl=? (%jeq (%ckj x) (%ckj y)))

(define-inline (flmin x y) %residual-flmin (%jmin (%ckj x) (%ckj y)))

(define-inline (flmax x y) %residual-flmax (%jmax (%ckj x) (%ckj y)))

(define-inline (flonum->fixnum x) %residual-flonum->fixnum (%jtoi (%ckj x)))


;---------------------------------------------------------------------------------------------
; Numbers (fixnums or flonums)
;---------------------------------------------------------------------------------------------

(define-inline (number? x) %residual-number? (%nump x))

(define-inline (integer? x) %residual-integer? (%intp x))

(define-syntax complex? number?)

(define-syntax real? number?)

(define-syntax rational? integer?)

(define-syntax exact-integer? fixnum?)

(define-inline (exact? x) %residual-exact? (%fixp (%ckn x)))

(define-inline (inexact? x) %residual-inexact? (%flop (%ckn x)))

(define-inline (finite? x) %residual-finite? (%finp (%ckn x))) 

(define-inline (infinite? x) %residual-infinite? (%infp (%ckn x))) 

(define-inline (nan? x) %residual-nan? (%nanp (%ckn x))) 

(define-inline (zero? x) %residual-zero? (%zerop (%ckn x)))

(define-inline (positive? x) %residual-positive? (%posp (%ckn x)))

(define-inline (negative? x) %residual-negative? (%negp (%ckn x)))

(define-inline (even? x) %residual-even? (%evnp (%ckn x)))

(define-inline (odd? x) %residual-odd? (%oddp (%ckn x)))

(define-syntax min
  (syntax-rules ()
    [(_ x) x]
    [(_ x y) (%min (%ckn x) (%ckn y))]
    [(_ x y z ...) (min (min x y) z ...)]
    [_ %residual-min]))

(define-syntax max
  (syntax-rules ()
    [(_ x) x]
    [(_ x y) (%max (%ckn x) (%ckn y))]
    [(_ x y z ...) (max (max x y) z ...)]
    [_ %residual-max]))

(define-syntax + 
  (syntax-rules ()
    [(_) 0] 
    [(_ x) (%ckn x)]
    [(_ x y) (%add (%ckn x) (%ckn y))]
    [(_ x y z ...) (+ (+ x y) z ...)]
    [_ %residual+]))

(define-syntax *
  (syntax-rules ()
    [(_) 1]
    [(_ x) (%ckn x)]
    [(_ x y) (%mul (%ckn x) (%ckn y))]
    [(_ x y z ...) (* (* x y) z ...)]
    [_ %residual*]))

(define-syntax -
  (syntax-rules ()
    [(_ x) (%neg (%ckn x))]
    [(_ x y) (%sub (%ckn x) (%ckn y))]
    [(_ x y z ...) (- (- x y) z ...)]
    [_ %residual-]))

(define-syntax /
  (syntax-rules ()
    [(_ x) (%div 1 (%ckn x))]
    [(_ x y) (%div (%ckn x) (%ckn y))]
    [(_ x y z ...) (/ (/ x y) z ...)]
    [_ %residual/]))

(define-syntax =
  (syntax-rules ()
    [(_ x y) (%eq (%ckn x) (%ckn y))] 
    [(_ x y z ...) (let ([t y]) (and (= x t) (= t z ...)))]
    [_ %residual=]))

(define-syntax <
  (syntax-rules ()
    [(_ x y) (%lt (%ckn x) (%ckn y))]
    [(_ x y z ...) (let ([t y]) (and (< x t) (< t z ...)))]
    [_ %residual<]))

(define-syntax >
  (syntax-rules ()
    [(_ x y) (%gt (%ckn x) (%ckn y))]
    [(_ x y z ...) (let ([t y]) (and (> x t) (> t z ...)))]
    [_ %residual>]))

(define-syntax <=
  (syntax-rules ()
    [(_ x y) (%le (%ckn x) (%ckn y))]
    [(_ x y z ...) (let ([t y]) (and (<= x t) (<= t z ...)))]
    [_ %residual<=]))

(define-syntax >=
  (syntax-rules ()
    [(_ x y) (%ge (%ckn x) (%ckn y))]
    [(_ x y z ...) (let ([t y]) (and (>= x t) (>= t z ...)))]
    [_ %residual>=]))

(define-inline (abs x) %residual-abs (%abs (%ckn x)))

(define-inline (quotient x y) %residual-quotient (%quo (%ckn x)))
(define-inline (remainder x y) %residual-remainder (%rem (%ckn x)))

(define-syntax truncate-quotient quotient)
(define-syntax truncate-remainder remainder)

(define-inline (modquo x y) %residual-modquo (%mqu (%ckn x)))
(define-inline (modulo x y) %residual-modulo (%mlo (%ckn x)))

(define-syntax floor-quotient modquo)
(define-syntax floor-remainder modulo)

;floor/
;truncate/


;---------------------------------------------------------------------------------------------
; Booleans
;---------------------------------------------------------------------------------------------

(define-inline (boolean? x) %residual-boolean? (%boolp x))

(define-inline (not x) %residual-not (%not x))


;---------------------------------------------------------------------------------------------
; Characters
;---------------------------------------------------------------------------------------------

(define-inline (char? x) %residual-char? (%charp x))

(define-inline (char-cmp x y) %residual-char-cmp (%ccmp (%ckc x) (%ckc y)))
(define-inline (char=? x y) %residual-char=? (%ceq (%ckc x) (%ckc y)))
(define-inline (char<? x y) %residual-char<? (%clt (%ckc x) (%ckc y)))
(define-inline (char<=? x y) %residual-char<=? (%cle (%ckc x) (%ckc y)))
(define-inline (char>? x y) %residual-char>? (%cgt (%ckc x) (%ckc y)))
(define-inline (char>=? x y) %residual-char>=? (%cge (%ckc x) (%ckc y)))

(define-inline (char-ci-cmp x y) %residual-char-cmp (%cicmp (%ckc x) (%ckc y)))
(define-inline (char-ci=? x y) %residual-char-ci=? (%cieq (%ckc x) (%ckc y)))
(define-inline (char-ci<? x y) %residual-char-ci<? (%cilt (%ckc x) (%ckc y)))
(define-inline (char-ci<=? x y) %residual-char-ci<=? (%cile (%ckc x) (%ckc y)))
(define-inline (char-ci>? x y) %residual-char-ci>? (%cigt (%ckc x) (%ckc y)))
(define-inline (char-ci>=? x y) %residual-char-ci>=? (%cige (%ckc x) (%ckc y)))

(define-inline (char-alphabetic? x) %residual-char-alphabetic? (%calp (%ckc x)))
(define-inline (char-numeric? x) %residual-char-numeric? (%cnup (%ckc x)))
(define-inline (char-whitespace? x) %residual-char-whitespace? (%cwsp (%ckc x)))
(define-inline (char-upper-case? x) %residual-char-upper-case? (%cucp (%ckc x)))
(define-inline (char-lower-case? x) %residual-char-lower-case? (%clcp (%ckc x)))
(define-inline (char-upcase x) %residual-char-upcase (%cupc (%ckc x)))
(define-inline (char-downcase x) %residual-char-downcase (%cdnc (%ckc x)))

(define-inline (char->integer x) %residual-char->integer (%ctoi (%ckc x)))
(define-inline (integer->char x) %residual-integer->char (%itoc (%cki x)))

;char-foldcase
;digit-value


;---------------------------------------------------------------------------------------------
; Symbols
;---------------------------------------------------------------------------------------------

(define-inline (symbol? x) %residual-symbol? (%symp x))

(define-inline (symbol->string x) %residual-symbol->string (%ytos (%cky x)))

(define-inline (string->symbol x) %residual-string->symbol (%stoy (%cks x)))


;---------------------------------------------------------------------------------------------
; Null and Pairs
;---------------------------------------------------------------------------------------------

(define-inline (null? x) %residual-null? (%nullp x))

(define-inline (pair? x) %residual-pair? (%pairp x))

(define-inline (car x) %residual-car (%car (%ckp x)))

(define-inline (set-car! x v) %residual-set-car! (%setcar (%ckp x) v))

(define-inline (cdr x) %residual-cdr (%cdr (%ckp x)))

(define-inline (set-cdr! x v) %residual-set-cdr! (%setcdr (%ckp x) v))

(define-syntax c?r
  (syntax-rules (a d)
    [(c?r x) x]
    [(c?r a ? ... x) (car (c?r ? ... x))]
    [(c?r d ? ... x) (cdr (c?r ? ... x))]))

(define-inline (caar x) %residual-caar (c?r a a x))
(define-inline (cadr x) %residual-cadr (c?r a d x))
(define-inline (cdar x) %residual-cdar (c?r d a x))
(define-inline (cddr x) %residual-cddr (c?r d d x))
(define-inline (caaar x) %residual-caaar (c?r a a a x))
(define-inline (caadr x) %residual-caadr (c?r a a d x))
(define-inline (cadar x) %residual-cadar (c?r a d a x))
(define-inline (caddr x) %residual-caddr (c?r a d d x))
(define-inline (cdaar x) %residual-cdaar (c?r d a a x))
(define-inline (cdadr x) %residual-cdadr (c?r d a d x))
(define-inline (cddar x) %residual-cddar (c?r d d a x))
(define-inline (cdddr x) %residual-cdddr (c?r d d d x))
(define-inline (caaaar x) %residual-caaaar (c?r a a a a x))
(define-inline (caaadr x) %residual-caaadr (c?r a a a d x))
(define-inline (caadar x) %residual-caadar (c?r a a d a x))
(define-inline (caaddr x) %residual-caaddr (c?r a a d d x))
(define-inline (cadaar x) %residual-cadaar (c?r a d a a x))
(define-inline (cadadr x) %residual-cadadr (c?r a d a d x))
(define-inline (caddar x) %residual-caddar (c?r a d d a x))
(define-inline (cadddr x) %residual-cadddr (c?r a d d d x))
(define-inline (cdaaar x) %residual-cdaaar (c?r d a a a x))
(define-inline (cdaadr x) %residual-cdaadr (c?r d a a d x))
(define-inline (cdadar x) %residual-cdadar (c?r d a d a x))
(define-inline (cdaddr x) %residual-cdaddr (c?r d a d d x))
(define-inline (cddaar x) %residual-cddaar (c?r d d a a x))
(define-inline (cddadr x) %residual-cddadr (c?r d d a d x))
(define-inline (cdddar x) %residual-cdddar (c?r d d d a x))
(define-inline (cddddr x) %residual-cddddr (c?r d d d d x))

(define-inline (cons x y) %residual-cons (%cons x y))


;---------------------------------------------------------------------------------------------
; Lists
;---------------------------------------------------------------------------------------------

(define-inline (list? x) %residual-list? (%listp x))

(define (%make-list n i)
  (let loop ([n (%ckk n)] [l '()])
    (if (%ile n 0) l (loop (%isub n 1) (cons i l))))) 

(define-syntax make-list
  (syntax-rules ()
    [(_ n) (%make-list n #f)]  ; #f > (void)
    [(_ n i) (%make-list n i)]
    [_ %residual-make-list]))

(define-syntax list 
  (syntax-rules ()
    [(_) '()]
    [(_ x) (%cons x '())]
    [(_ x ...) (%list x ...)]
    [_ %residual-list]))

(define-inline (length x) %residual-length (%llen (%ckl x))) ; optimize via combo instruction "%lg"?

(define-inline (list-ref x i) %residual-list-ref (%lget (%ckl x) (%cki i))) ; check for range, optimize combo?

(define-inline (list-set! x i v) %residual-list-set! (%lput (%ckl x) (%cki i) v)) ; check for range, optimize combo?

(define-syntax append
  (syntax-rules ()
    [(_) '()] [(_ x) x]
    [(_ x y) (%lcat (%ckl x) y)]
    [(_ x y z ...) (%lcat (%ckl x) (append y z ...))]
    [_ %residual-append]))

(define-inline (memq v y) %residual-memq (%memq v (%ckl y)))  ; optimize combo?

(define-inline (memv v y) %residual-memv (%memv v (%ckl y)))  ; optimize combo?

(define (%member x l eq)
  (and (pair? l) (if (eq x (%car l)) l (%member x (%cdr l) eq))))

(define-syntax member
  (syntax-rules ()
    [(_ v y) (%meme v (%ckl y))]
    [(_ v y eq) (%member v y eq)]
    [_ %residual-member]))

(define-inline (assq v y) %residual-assq (%assq v (%ckl y)))  ; check for a-list; optimize combo?

(define-inline (assv v y) %residual-assv (%assv v (%ckl y)))  ; check for a-list; optimize combo?

(define (%assoc v al eq) 
  (and (pair? al) (if (eq v (car (%car al))) (%car al) (%assoc v (%cdr al) eq))))

(define-syntax assoc
  (syntax-rules ()
    [(_ v al) (%asse v (%ckl al))]
    [(_ v al eq) (%assoc v al eq)]
    [_ %residual-assoc]))

(define-inline (list-copy x) %residual-list-copy (%lcat (%ckl x) '()))

(define-inline (list-tail x i) %residual-list-tail (%ltail (%ckl x) (%cki i))) ; check for range, optimize combo?

(define-inline (last-pair x) %residual-last-pair (%lpair (%ckp x)))

(define-inline (reverse x) %residual-reverse (%lrev (%ckl x))) ; optimize combo?

(define-inline (reverse! x) %residual-reverse! (%lrevi (%ckl x))) ; optimize combo?

(define-syntax list*
  (syntax-rules ()
    [(_ x) x]
    [(_ x y) (%cons x y)]
    [(_ x y z ...) (%cons x (list* y z ...))]
    [_ %residual-list*]))

(define-syntax cons* list*)

(define-syntax map
  (syntax-rules ()
    [(_ fun lst)
     (let ([f fun]) 
       (let loop ([l lst]) 
         (if (pair? l) (cons (f (%car l)) (loop (%cdr l))) '())))]
    [_ %residual-map])) 

(define-syntax for-each
  (syntax-rules ()
    [(_ fun lst)
     (let ([f fun]) 
       (let loop ([l lst]) 
         (if (pair? l) (begin (f (%car l)) (loop (%cdr l))))))]
    [_ %residual-for-each]))


;---------------------------------------------------------------------------------------------
; Vectors
;---------------------------------------------------------------------------------------------

(define-inline (vector? x) %residual-vector? (%vecp x))

(define-syntax vector %vec)

(define-syntax make-vector
  (syntax-rules ()
    [(_ n) (%vmk (%ckk n) #f)]
    [(_ n v) (%vmk (%ckk n) v)]
    [_ %residual-make-vector]))

(define-inline (vector-length x) %residual-vector-length (%vlen (%ckv x))) ; optimize combo?

(define-inline (vector-ref x i) %residual-vector-ref (%vget (%ckv x) (%cki i))) ; check for range, optimize combo?

(define-inline (vector-set! x i v) %residual-vector-set! (%vput (%ckv x) (%cki i) v)) ; check for range, optimize combo?

(define-syntax vector-append
  (syntax-rules ()
    [(_) '#()] [(_ x) (%ckv x)]
    [(_ x y) (%vcat (%ckv x) (%ckv y))]
    [(_ x y z ...) (vector-append x (vector-append y z ...))]
    [_ %residual-vector-append]))

(define-inline (vector->list x) %residual-vector->list (%vtol (%ckv x)))

(define-inline (list->vector x) %residual-list->vector (%ltov (%ckl x)))

;vector->list/1/2/3
;vector-copy/1/2/3=subvector
;vector-copy!/2/3/4/5 (to at from start end) 
;vector-fill!/2/3/4 (vector val start end) 
;vector->string/1/2/3
;string->vector/1/2/3


;---------------------------------------------------------------------------------------------
; Strings
;---------------------------------------------------------------------------------------------

(define-inline (string? x) %residual-string? (%strp x))

(define-syntax string
  (syntax-rules ()
    [(_ c ...) (%str (%ckc c) ...)]
    [_ %residual-string]))

(define-syntax make-string
  (syntax-rules ()
    [(_ x) (%smk (%ckk x) #\space)]
    [(_ x y) (%smk (%ckk x) (%ckc y))]
    [_ %residual-make-string]))

(define-inline (string-length x) %residual-string-length (%slen (%cks x))) ; optimize combo?

(define-inline (string-ref x i) %residual-string-ref (%sget (%cks x) (%cki i))) ; check for range, optimize combo?

(define-inline (string-set! x i v) %residual-string-set! (%sput (%cks x) (%cki i) (%ckc v))) ; check for range, optimize combo?

(define-syntax string-append
  (syntax-rules ()
    [(_) ""] [(_ x) (%cks x)]
    [(_ x y) (%scat (%cks x) (%cks y))]
    [(_ x y z ...) (string-append x (string-append y z ...))]
    [_ %residual-string-append]))

(define-inline (substring x s e) %residual-substring (%ssub (%cks x) (%cki s) (%cki e)))  ; check for range

(define-inline (string-cmp x y) %residual-string-cmp (%scmp (%cks x) (%cks y)))
(define-inline (string=? x y) %residual-string<? (%seq (%cks x) (%cks y)))
(define-inline (string<? x y) %residual-string<? (%slt (%cks x) (%cks y)))
(define-inline (string<=? x y) %residual-string<=? (%sle (%cks x) (%cks y)))
(define-inline (string>? x y) %residual-string>? (%sgt (%cks x) (%cks y)))
(define-inline (string>=? x y) %residual-string>=? (%sge (%cks x) (%cks y)))
(define-inline (string-ci-cmp x y) %residual-string-cmp (%sicmp (%cks x) (%cks y)))
(define-inline (string-ci=? x y) %residual-string<? (%sieq (%cks x) (%cks y)))
(define-inline (string-ci<? x y) %residual-string<? (%silt (%cks x) (%cks y)))
(define-inline (string-ci<=? x y) %residual-string<=? (%sile (%cks x) (%cks y)))
(define-inline (string-ci>? x y) %residual-string>? (%sigt (%cks x) (%cks y)))
(define-inline (string-ci>=? x y) %residual-string>=? (%sige (%cks x) (%cks y)))

(define-inline (string->list x) %residual-string->list (%stol (%cks x)))

(define-inline (list->string x) %residual-list->string (%ltos (%ckl x)))  ; list-of-chars test

;string-upcase
;string-downcase
;string-foldcase
;string->list/1/2/3
;string-copy/1/2/3=substring
;string-copy!/2/3/4/5 (to at from start end) 
;string-fill!/2/3/4 (string v start end) 


;---------------------------------------------------------------------------------------------
; Conversions
;---------------------------------------------------------------------------------------------

(define-inline (fixnum->string x r) %residual-fixnum->string (%itos (%cki x) (%cki r)))

(define-inline (string->fixnum x r) %residual-string->fixnum (%stoi (%cks x) (%cki r)))

(define-inline (flonum->string x) %residual-flonum->string (%jtos (%ckj x)))

(define-inline (string->flonum x) %residual-string->flonum (%stoj (%cks x)))

(define-syntax number->string
  (syntax-rules ()
    [(_ x r) (%ntos (%cki x) (%cki r))]
    [(_ x) (%ntos (%cki x) 10)]
    [_ %residual-number->string]))

(define-syntax string->number
  (syntax-rules ()
    [(_ x r) (%ston (%cks x) (%cki r))]
    [(_ x) (%ston (%cks x) 10)]
    [_ %residual-string->number]))


;---------------------------------------------------------------------------------------------
; Control features
;---------------------------------------------------------------------------------------------

(define-inline (procedure? x) %residual-procedure? (%funp x))

(define-syntax apply
  (syntax-rules ()
    [(_ p l) (%appl p (%ckl l))]  ; -- check for proc?
    [(_ p a b ... l) (%appl p (list* a b ... l))]
    [_ %residual-apply]))


(define-syntax call/cc %ccc) ; (%ccc (%ckr1 k)) -- check for 1-arg proc?

(define-syntax call-with-current-continuation call/cc)

;map
;string-map
;vector-map
;for-each
;string-for-each
;vector-for-each
;values
;call-with-values


;---------------------------------------------------------------------------------------------
; I/O Ports
;---------------------------------------------------------------------------------------------

(define-inline (input-port? x) %residual-input-port? (%ipp x))

(define-inline (output-port? x) %residual-output-port? (%opp x))

(define-inline (eof-object? x) %residual-eof-object? (%eofp x))

(define-inline (current-input-port) %residual-current-input-port (%sip))

(define-inline (current-ouput-port) %residual-current-ouput-port (%sop))

(define-inline (current-error-port) %residual-current-error-port (%sep))

(define-inline (open-output-string) %residual-open-output-string (%oos))

(define-inline (open-input-file x) %residual-open-input-file (%otip (%cks x)))

(define-inline (open-output-file x) %residual-open-output-file (%otop (%cks x)))

(define-inline (open-input-string x) %residual-open-input-string (%ois (%cks x)))

(define-inline (close-input-port x) %residual-close-input-port (%cip (%ckr x)))

(define-inline (close-output-port x) %residual-close-output-port (%cop (%ckw x)))

(define-inline (get-output-string x) %residual-get-output-string (%gos (%ckw x)))


;---------------------------------------------------------------------------------------------
; Output
;---------------------------------------------------------------------------------------------

(define-inline (write-char x p) %residual-write-char (%wrc (%ckc x) (%ckw p)))

(define-inline (write-string x p) %residual-write-string (%wrs (%cks x) (%ckw p)))

(define-inline (display x p) %residual-display (%wrcd x (%ckw p)))

(define-inline (write x p) %residual-write (%wrcw x (%ckw p)))

(define-inline (newline p) %residual-newline (%wrnl (%ckw p)))

(define-inline (write-shared x p) %residual-write-shared (%wrhw x (%ckw p)))

(define-inline (write-simple x p) %residual-write-simple (%wriw x (%ckw p)))


;---------------------------------------------------------------------------------------------
; Residual versions of vararg procedures
;---------------------------------------------------------------------------------------------

(define-syntax nullary-unary-adaptor
  (syntax-rules ()
    [(_ f)
     (lambda args
       (if (null? args) (f) (f (car args))))]))

(define-syntax nullary-unary-binary-adaptor
  (syntax-rules ()
    [(_ f)
     (lambda args
       (if (null? args) (f) (if (null? (cdr args)) (f (car args)) (f (car args) (cadr args)))))])) 

(define-syntax unary-binary-adaptor
  (syntax-rules ()
    [(_ f)
     (lambda (x . args)
       (if (null? args) (f x) (f x (car args))))]))

(define-syntax unary-binary-ternary-adaptor
  (syntax-rules ()
    [(_ f)
     (lambda (x . args)
       (if (null? args) (f x) (if (null? (cdr args)) (f x (car args)) (f x (car args) (cadr args)))))])) 

(define-syntax unary-binary-ternary-quaternary-adaptor
  (syntax-rules ()
    [(_ f)
     (lambda (x . args)
       (if (null? args) (f x) (if (null? (cdr args)) (f x (car args)) 
         (if (null? (cddr args)) (f x (car args) (cadr args)) (f x (car args) (cadr args) (caddr args))))))])) 

(define-syntax binary-ternary-adaptor
  (syntax-rules ()
    [(_ f)
     (lambda (x y . args)
       (if (null? args) (f x y) (f x y (car args))))]))

(define-syntax cmp-reducer
  (syntax-rules ()
    [(_ f)
     (lambda args
       (or (null? args)
           (let loop ([x (car args)] [args (cdr args)])
             (or (null? args)
                 (let ([y (car args)])
                   (and (f x y) (loop y (cdr args))))))))]))

(define (%residual-list . l) l)

(define %residual-make-list (unary-binary-adaptor make-list))
(define %residual-make-vector (unary-binary-adaptor make-vector))
(define %residual-make-string (unary-binary-adaptor make-string))

(define %residual= (cmp-reducer =))  
(define %residual< (cmp-reducer <))
(define %residual> (cmp-reducer >))
(define %residual<= (cmp-reducer <=))
(define %residual>= (cmp-reducer >=))

(define-syntax minmax-reducer
  (syntax-rules ()
    [(_ f)
     (lambda (x . args)
       (let loop ([x x] [args args])
         (if (null? args)
             x
             (loop (f x (car args)) (cdr args)))))]))

(define %residual-min (minmax-reducer min))
(define %residual-max (minmax-reducer max))

(define-syntax addmul-reducer
  (syntax-rules ()
    [(_ f s)
     (lambda args
       (if (null? args)
           s
           (let loop ([x (car args)] [args (cdr args)])
             (if (null? args)
                 x
                 (loop (f x (car args)) (cdr args))))))]))

(define %residual+ (addmul-reducer + 0))
(define %residual* (addmul-reducer * 1))

(define-syntax subdiv-reducer
  (syntax-rules ()
    [(_ f)
     (lambda (x . args)
       (if (null? args)
           (f x)
           (let loop ([x x] [args args])
             (if (null? args)
                 x
                 (loop (f x (car args)) (cdr args))))))]))

(define %residual- (subdiv-reducer -))
(define %residual/ (subdiv-reducer /))

(define %residual-member (binary-ternary-adaptor member)) 
(define %residual-assoc (binary-ternary-adaptor assoc))

(define (%residual-list* x . l)
  (let loop ([x x] [l l])
    (if (null? l) x (cons x (loop (car l) (cdr l))))))

(define (%residual-apply f x . l)
  (apply f 
    (let loop ([x x] [l l])
      (if (null? l) x (cons x (loop (car l) (cdr l)))))))

(define (%residual-map p l . l*)
  (if (null? l*) 
      (let loop ([l l] [r '()])
        (if (pair? l) (loop (cdr l) (cons (p (car l)) r)) (reverse! r)))
      (let loop ([l* (cons l l*)] [r '()])
        (if (let lp ([l* l*]) (or (null? l*) (and (pair? (car l*)) (lp (cdr l*)))))
            (loop (map cdr l*) (cons (apply p (map car l*)) r))
            (reverse! r)))))

(define (%residual-for-each p l . l*)
  (if (null? l*) 
      (let loop ([l l]) (if (pair? l) (begin (p (car l)) (loop (cdr l)))))
      (let loop ([l* (cons l l*)])
        (if (let lp ([l* l*]) (or (null? l*) (and (pair? (car l*)) (lp (cdr l*)))))
            (begin (apply p (map car l*)) (loop (map cdr l*)))))))

(define (string-map p s . s*)
  (if (null? s*)
      (let* ([len (string-length s)] [res (make-string len)])
        (do ([i 0 (fx+ i 1)]) [(fx>=? i len) res]
           (string-set! res i (p (string-ref s i)))))
      (list->string (apply map p (map string->list (cons s s*))))))

(define (vector-map p v . v*)
  (if (null? v*)
      (let* ([len (vector-length v)] [res (make-vector len)])
        (do ([i 0 (fx+ i 1)]) [(fx>=? i len) res]
          (vector-set! res i (p (vector-ref v i)))))
      (list->vector (apply map p (map vector->list (cons v v*))))))

(define (string-for-each p s . s*)
  (if (null? s*) 
      (let ([len (string-length s)])
        (do ([i 0 (fx+ i 1)]) [(fx>=? i len)] (p (string-ref s i))))
      (apply for-each p (map string->list (cons s s*)))))

(define (vector-for-each p v . v*)
  (if (null? v*)
      (let ([len (vector-length v)])
        (do ([i 0 (fx+ i 1)]) [(fx>=? i len)] (p (vector-ref v i))))
      (apply for-each p (map vector->list (cons v v*)))))

(define-syntax append-reducer
  (syntax-rules ()
    [(_ f s)
     (lambda args
       (let loop ([args args])
         (cond [(null? args) s]
               [(null? (cdr args)) (car args)]
               [else (f (car args) (loop (cdr args)))])))]))

(define %residual-append (append-reducer append '()))
(define %residual-string-append (append-reducer string-append ""))
(define %residual-vector-append (append-reducer vector-append '#()))

(define %residual-number->string (unary-binary-adaptor number->string))
(define %residual-string->number (unary-binary-adaptor string->number))

