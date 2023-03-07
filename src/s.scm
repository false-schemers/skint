
;---------------------------------------------------------------------------------------------
; Scheme library functions
;---------------------------------------------------------------------------------------------

(define-syntax define-inline
  (syntax-rules ()
    [(_ (id v ...) rid expr)
     (begin
       (define-syntax id
         (syntax-rules ()
           [(_ v ...) expr] ; NB: do not use the same var twice!
           [(_ . r) (rid . r)] ; NB: use syntax-error?
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

(define-inline (fxzero? x) %residual-fxzero? (%izerop x))

(define-inline (fxpositive? x) %residual-fxpositive? (%iposp x))

(define-inline (fxnegative? x) %residual-fxnegative? (%inegp x))

(define-inline (fx+ x y) %residual-fx+ (%iadd x y))

(define-inline (fx* x y) %residual-fx* (%imul x y))

(define-inline (fx- x y) %residual-fx- (%isub x y))

(define-inline (fx/ x y) %residual-fx/ (%idiv x y))

(define-inline (fxquotient x y) %residual-fxquotient (%iquo x y))

(define-inline (fxremainder x y) %residual-fxremainder (%irem x y))

(define-inline (fxmodquo x y) %residual-fxmodquo (%imqu x y))

(define-inline (fxmodulo x y) %residual-fxmodulo (%imlo x y))

(define-inline (fxeucquo x y) %residual-fxeucquo (%ieuq x y)) ;euclidean-quotient

(define-inline (fxeucrem x y) %residual-fxeucrem (%ieur x y)) ;euclidean-remainder

(define-inline (fxneg x) %residual-fxneg (%ineg x))

(define-inline (fxabs x) %residual-fxabs (%iabs x))

(define-inline (fx<? x y) %residual-fx<? (%ilt x y))

(define-inline (fx<=? x y) %residual-fx<=? (%ile x y))

(define-inline (fx>? x y) %residual-fx>? (%igt x y))

(define-inline (fx>=? x y) %residual-fx>=? (%ige x y))

(define-inline (fx=? x y) %residual-fx=? (%ieq x y))

(define-inline (fxmin x y) %residual-fxmin (%imin x y))

(define-inline (fxmax x y) %residual-fxmax (%imax x y))

(define-inline (fixnum->flonum x) %residual-fixnum->flonum (%itoj x))


;---------------------------------------------------------------------------------------------
; Inexact floating-point numbers (flonums)
;---------------------------------------------------------------------------------------------

(define-inline (flonum? x) %residual-flonum? (%flop x))

(define-inline (flzero? x) %residual-flzero? (%jzerop x))

(define-inline (flpositive? x) %residual-flpositive? (%jposp x))

(define-inline (flnegative? x) %residual-flnegative? (%jnegp x))

(define-inline (flinteger? x) %residual-flinteger? (%jintp x)) 

(define-inline (flnan? x) %residual-flnan? (%jnanp x)) 

(define-inline (flinfinite? x) %residual-flinfinite? (%jinfp x)) 

(define-inline (flfinite? x) %residual-flfinite? (%jfinp x)) 

(define-inline (fleven? x) %residual-fleven? (%jevnp x))

(define-inline (flodd? x) %residual-flodd? (%joddp x))

(define-inline (fl+ x y) %residual-fl+ (%jadd x y))

(define-inline (fl- x y) %residual-fl- (%jsub x y))

(define-inline (fl* x y) %residual-fl* (%jmul x y))

(define-inline (fl/ x y) %residual-fl/ (%jdiv x y))

(define-inline (flneg x) %residual-flneg (%jneg x))

(define-inline (flabs x) %residual-flabs (%jabs x))

(define-inline (fl<? x y) %residual-fl<? (%jlt x y))

(define-inline (fl<=? x y) %residual-fl<=? (%jle x y))

(define-inline (fl>? x y) %residual-fl>? (%jgt x y))

(define-inline (fl>=? x y) %residual-fl>=? (%jge x y))

(define-inline (fl=? x y) %residual-fl=? (%jeq x y))

(define-inline (flmin x y) %residual-flmin (%jmin x y))

(define-inline (flmax x y) %residual-flmax (%jmax x y))

(define-inline (flonum->fixnum x) %residual-flonum->fixnum (%jtoi x))


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

(define-inline (finite? x) %residual-finite? (%finp x)) 

(define-inline (infinite? x) %residual-infinite? (%infp x)) 

(define-inline (nan? x) %residual-nan? (%nanp x)) 

(define-inline (zero? x) %residual-zero? (%zerop x))

(define-inline (positive? x) %residual-positive? (%posp x))

(define-inline (negative? x) %residual-negative? (%negp x))

(define-inline (even? x) %residual-even? (%evnp x))

(define-inline (odd? x) %residual-odd? (%oddp x))

(define-syntax min
  (syntax-rules ()
    [(_ x) x]
    [(_ x y) (%min x y)]
    [(_ x y z ...) (min (min x y) z ...)]
    [(_ . args) (%residual-min . args)]
    [_ %residual-min]))

(define-syntax max
  (syntax-rules ()
    [(_ x) x]
    [(_ x y) (%max x y)]
    [(_ x y z ...) (max (max x y) z ...)]
    [(_ . args) (%residual-max . args)]
    [_ %residual-max]))

(define-syntax + 
  (syntax-rules ()
    [(_) 0] 
    [(_ x) (%ckn x)]
    [(_ x y) (%add x y)]
    [(_ x y z ...) (+ (+ x y) z ...)]
    [_ %residual+]))

(define-syntax *
  (syntax-rules ()
    [(_) 1]
    [(_ x) (%ckn x)]
    [(_ x y) (%mul x y)]
    [(_ x y z ...) (* (* x y) z ...)]
    [_ %residual*]))

(define-syntax -
  (syntax-rules ()
    [(_ x) (%neg x)]
    [(_ x y) (%sub x y)]
    [(_ x y z ...) (- (- x y) z ...)]
    [(_ . args) (%residual- . args)]
    [_ %residual-]))

(define-syntax /
  (syntax-rules ()
    [(_ x) (%div 1 x)]
    [(_ x y) (%div x y)]
    [(_ x y z ...) (/ (/ x y) z ...)]
    [(_ . args) (%residual/ . args)]
    [_ %residual/]))

(define-syntax =
  (syntax-rules ()
    [(_ x y) (%eq x y)] 
    [(_ x y z ...) (let ([t y]) (and (= x t) (= t z ...)))]
    [(_ . args) (%residual= . args)]
    [_ %residual=]))

(define-syntax <
  (syntax-rules ()
    [(_ x y) (%lt x y)]
    [(_ x y z ...) (let ([t y]) (and (< x t) (< t z ...)))]
    [(_ . args) (%residual< . args)]
    [_ %residual<]))

(define-syntax >
  (syntax-rules ()
    [(_ x y) (%gt x y)]
    [(_ x y z ...) (let ([t y]) (and (> x t) (> t z ...)))]
    [(_ . args) (%residual> . args)]
    [_ %residual>]))

(define-syntax <=
  (syntax-rules ()
    [(_ x y) (%le x y)]
    [(_ x y z ...) (let ([t y]) (and (<= x t) (<= t z ...)))]
    [(_ . args) (%residual<= . args)]
    [_ %residual<=]))

(define-syntax >=
  (syntax-rules ()
    [(_ x y) (%ge x y)]
    [(_ x y z ...) (let ([t y]) (and (>= x t) (>= t z ...)))]
    [(_ . args) (%residual>= . args)]
    [_ %residual>=]))

(define-inline (abs x) %residual-abs (%abs x))

(define-inline (quotient x y) %residual-quotient (%quo x y))
(define-inline (remainder x y) %residual-remainder (%rem x y))

(define-syntax truncate-quotient quotient)
(define-syntax truncate-remainder remainder)

(define-inline (modquo x y) %residual-modquo (%mqu x y))
(define-inline (modulo x y) %residual-modulo (%mlo x y))

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

(define-inline (char-cmp x y) %residual-char-cmp (%ccmp x y))
(define-inline (char=? x y) %residual-char=? (%ceq x y))
(define-inline (char<? x y) %residual-char<? (%clt x y))
(define-inline (char<=? x y) %residual-char<=? (%cle x y))
(define-inline (char>? x y) %residual-char>? (%cgt x y))
(define-inline (char>=? x y) %residual-char>=? (%cge x y))

(define-inline (char-ci-cmp x y) %residual-char-cmp (%cicmp x y))
(define-inline (char-ci=? x y) %residual-char-ci=? (%cieq x y))
(define-inline (char-ci<? x y) %residual-char-ci<? (%cilt x y))
(define-inline (char-ci<=? x y) %residual-char-ci<=? (%cile x y))
(define-inline (char-ci>? x y) %residual-char-ci>? (%cigt x y))
(define-inline (char-ci>=? x y) %residual-char-ci>=? (%cige x y))

(define-inline (char-alphabetic? x) %residual-char-alphabetic? (%calp x))
(define-inline (char-numeric? x) %residual-char-numeric? (%cnup x))
(define-inline (char-whitespace? x) %residual-char-whitespace? (%cwsp x))
(define-inline (char-upper-case? x) %residual-char-upper-case? (%cucp x))
(define-inline (char-lower-case? x) %residual-char-lower-case? (%clcp x))
(define-inline (char-upcase x) %residual-char-upcase (%cupc x))
(define-inline (char-downcase x) %residual-char-downcase (%cdnc x))

(define-inline (char->integer x) %residual-char->integer (%ctoi x))
(define-inline (integer->char x) %residual-integer->char (%itoc x))

;char-foldcase
;digit-value


;---------------------------------------------------------------------------------------------
; Symbols
;---------------------------------------------------------------------------------------------

(define-inline (symbol? x) %residual-symbol? (%symp x))

(define-inline (symbol->string x) %residual-symbol->string (%ytos x))

(define-inline (string->symbol x) %residual-string->symbol (%stoy x))


;---------------------------------------------------------------------------------------------
; Null and Pairs
;---------------------------------------------------------------------------------------------

(define-inline (null? x) %residual-null? (%nullp x))

(define-inline (pair? x) %residual-pair? (%pairp x))

(define-inline (car x) %residual-car (%car x))

(define-inline (set-car! x v) %residual-set-car! (%setcar x v))

(define-inline (cdr x) %residual-cdr (%cdr x))

(define-inline (set-cdr! x v) %residual-set-cdr! (%setcdr x v))

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
    [(_ . args) (%residual-make-list . args)]
    [_ %residual-make-list]))

(define-syntax list 
  (syntax-rules ()
    [(_) '()]
    [(_ x) (%cons x '())]
    [(_ x ...) (%list x ...)]
    [_ %residual-list]))

(define-inline (length x) %residual-length (%llen x))

(define-inline (list-ref x i) %residual-list-ref (%lget x i))

(define-inline (list-set! x i v) %residual-list-set! (%lput x i v))

(define-syntax append
  (syntax-rules ()
    [(_) '()] [(_ x) x]
    [(_ x y) (%lcat x y)]
    [(_ x y z ...) (%lcat x (append y z ...))]
    [_ %residual-append]))

(define-inline (memq v y) %residual-memq (%memq v y))

(define-inline (memv v y) %residual-memv (%memv v (%ckl y)))  ; TODO: make sure memv checks list

(define (%member x l eq)
  (and (pair? l) (if (eq x (%car l)) l (%member x (%cdr l) eq))))

(define-syntax member
  (syntax-rules ()
    [(_ v y) (%meme v (%ckl y))] ; TODO: make sure meme checks list
    [(_ v y eq) (%member v y eq)]
    [(_ . args) (%residual-member . args)]
    [_ %residual-member]))

(define-inline (assq v y) %residual-assq (%assq v y))

(define-inline (assv v y) %residual-assv (%assv v (%ckl y)))  ; TODO: make sure assv checks list

(define (%assoc v al eq) 
  (and (pair? al) (if (eq v (car (%car al))) (%car al) (%assoc v (%cdr al) eq))))

(define-syntax assoc
  (syntax-rules ()
    [(_ v al) (%asse v (%ckl al))] ; TODO: make sure asse checks list
    [(_ v al eq) (%assoc v al eq)]
    [(_ . args) (%residual-assoc . args)]
    [_ %residual-assoc]))

(define-inline (list-copy x) %residual-list-copy (%lcat x '()))

(define-inline (list-tail x i) %residual-list-tail (%ltail x i))

(define-inline (last-pair x) %residual-last-pair (%lpair x))

(define-inline (reverse x) %residual-reverse (%lrev x))

(define-inline (reverse! x) %residual-reverse! (%lrevi x))

(define-syntax list*
  (syntax-rules ()
    [(_ x) x]
    [(_ x y) (%cons x y)]
    [(_ x y z ...) (%cons x (list* y z ...))]
    [(_ . args) (%residual-list* . args)]
    [_ %residual-list*]))

(define-syntax cons* list*)


;---------------------------------------------------------------------------------------------
; Vectors
;---------------------------------------------------------------------------------------------

(define-inline (vector? x) %residual-vector? (%vecp x))

(define-syntax vector %vec)

(define-syntax make-vector
  (syntax-rules ()
    [(_ n) (%vmk n #f)]
    [(_ n v) (%vmk n v)]
    [(_ . args) (%residual-make-vector . args)]
    [_ %residual-make-vector]))

(define-inline (vector-length x) %residual-vector-length (%vlen x))

(define-inline (vector-ref x i) %residual-vector-ref (%vget x i))

(define-inline (vector-set! x i v) %residual-vector-set! (%vput x i v))

(define-syntax vector-append
  (syntax-rules ()
    [(_) '#()] [(_ x) (%ckv x)]
    [(_ x y) (%vcat x y)]
    [(_ x y z ...) (vector-append x (vector-append y z ...))]
    [_ %residual-vector-append]))

(define-inline (vector->list x) %residual-vector->list (%vtol x))

(define-inline (list->vector x) %residual-list->vector (%ltov x))

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
    [(_ c ...) (%str c ...)]
    [_ %residual-string]))

(define-syntax make-string
  (syntax-rules ()
    [(_ x) (%smk x #\space)]
    [(_ x y) (%smk x y)]
    [(_ . args) (%residual-make-string . args)]
    [_ %residual-make-string]))

(define-inline (string-length x) %residual-string-length (%slen x))

(define-inline (string-ref x i) %residual-string-ref (%sget x i))

(define-inline (string-set! x i v) %residual-string-set! (%sput x i v))

(define-syntax string-append
  (syntax-rules ()
    [(_) ""] [(_ x) (%cks x)]
    [(_ x y) (%scat x y)]
    [(_ x y z ...) (string-append x (string-append y z ...))]
    [_ %residual-string-append]))

(define-inline (substring x s e) %residual-substring (%ssub x s e))

(define-inline (string-cmp x y) %residual-string-cmp (%scmp x y))
(define-inline (string=? x y) %residual-string<? (%seq x y))
(define-inline (string<? x y) %residual-string<? (%slt x y))
(define-inline (string<=? x y) %residual-string<=? (%sle x y))
(define-inline (string>? x y) %residual-string>? (%sgt x y))
(define-inline (string>=? x y) %residual-string>=? (%sge x y))
(define-inline (string-ci-cmp x y) %residual-string-cmp (%sicmp x y))
(define-inline (string-ci=? x y) %residual-string<? (%sieq x y))
(define-inline (string-ci<? x y) %residual-string<? (%silt x y))
(define-inline (string-ci<=? x y) %residual-string<=? (%sile x y))
(define-inline (string-ci>? x y) %residual-string>? (%sigt x y))
(define-inline (string-ci>=? x y) %residual-string>=? (%sige x y))

(define-inline (string->list x) %residual-string->list (%stol x))

(define-inline (list->string x) %residual-list->string (%ltos x))

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

(define-inline (fixnum->string x r) %residual-fixnum->string (%itos x r))

(define-inline (string->fixnum x r) %residual-string->fixnum (%stoi x r))

(define-inline (flonum->string x) %residual-flonum->string (%jtos x))

(define-inline (string->flonum x) %residual-string->flonum (%stoj x))

(define-syntax number->string
  (syntax-rules ()
    [(_ x r) (%ntos x r)]
    [(_ x) (%ntos x 10)]
    [(_ . args) (%residual-number->string . args)]
    [_ %residual-number->string]))

(define-syntax string->number
  (syntax-rules ()
    [(_ x r) (%ston x r)]
    [(_ x) (%ston x 10)]
    [(_ . args) (%residual-string->number . args)]
    [_ %residual-string->number]))


;---------------------------------------------------------------------------------------------
; Control features
;---------------------------------------------------------------------------------------------

(define-inline (procedure? x) %residual-procedure? (%funp x))

(define-syntax apply
  (syntax-rules ()
    [(_ p l) (%appl p l)]
    [(_ p a b ... l) (%appl p (list* a b ... l))]
    [(_ . args) (%residual-apply . args)]
    [_ %residual-apply]))

(define-syntax call/cc %ccc)

(define-syntax call-with-current-continuation call/cc)

(define-syntax values %sdmv)

(define-syntax call-with-values %cwmv)

(define-syntax map
  (syntax-rules ()
    [(_ fun lst)
     (let ([f fun]) 
       (let loop ([l lst]) 
         (if (pair? l) (cons (f (%car l)) (loop (%cdr l))) '())))]
    [(_ . args) (%residual-map . args)]
    [_ %residual-map])) 

(define-syntax for-each
  (syntax-rules ()
    [(_ fun lst)
     (let ([f fun]) 
       (let loop ([l lst]) 
         (if (pair? l) (begin (f (%car l)) (loop (%cdr l))))))]
    [(_ . args) (%residual-for-each . args)]
    [_ %residual-for-each]))

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


;---------------------------------------------------------------------------------------------
; I/O Ports
;---------------------------------------------------------------------------------------------

(define-inline (input-port? x) %residual-input-port? (%ipp x))

(define-inline (output-port? x) %residual-output-port? (%opp x))

(define-inline (current-input-port) %residual-current-input-port (%sip))

(define-inline (current-output-port) %residual-current-output-port (%sop))

(define-inline (current-error-port) %residual-current-error-port (%sep))

(define-inline (open-output-string) %residual-open-output-string (%oos))

(define-inline (open-input-file x) %residual-open-input-file (%otip x))

(define-inline (open-output-file x) %residual-open-output-file (%otop x))

(define-inline (open-input-string x) %residual-open-input-string (%ois x))

(define-inline (close-input-port x) %residual-close-input-port (%cip x))

(define-inline (close-output-port x) %residual-close-output-port (%cop x))

(define-inline (get-output-string x) %residual-get-output-string (%gos x))

;call-with-port
;call-with-input-file
;call-with-output-file
;port?
;input-port-open?
;output-port-open?
;with-input-from-file
;with-output-to-file
;open-binary-input-file
;open-binary-output-file
;close-port
;open-input-bytevector
;open-output-bytevector
;get-output-bytevector


;---------------------------------------------------------------------------------------------
; Input
;---------------------------------------------------------------------------------------------

;read
;read-char
;peek-char
;read-line
;char-ready?
;read-string
;read-u8
;peek-u8
;u8-ready?
;read-bytevector
;read-bytevector!

(define-inline (eof-object? x) %residual-eof-object? (%eofp x))
;eof-object


;---------------------------------------------------------------------------------------------
; Output
;---------------------------------------------------------------------------------------------

(define-syntax write-char
  (syntax-rules ()
    [(_ x) (%wrc x (%sop))]
    [(_ x p) (%wrc x p)]
    [(_ . args) (%residual-write-char . args)]
    [_ %residual-write-char]))

(define-syntax write-string
  (syntax-rules ()
    [(_ x) (%wrs x (%sop))]
    [(_ x p) (%wrs x p)]
    [(_ . args) (%residual-write-string . args)]
    [_ %residual-write-string]))

(define-syntax display
  (syntax-rules ()
    [(_ x) (%wrcd x (%sop))]
    [(_ x p) (%wrcd x p)]
    [(_ . args) (%residual-display . args)]
    [_ %residual-display]))

(define-syntax write
  (syntax-rules ()
    [(_ x) (%wrcw x (%sop))]
    [(_ x p) (%wrcw x p)]
    [(_ . args) (%residual-write . args)]
    [_ %residual-write]))

(define-syntax newline
  (syntax-rules ()
    [(_)  (%wrnl (%sop))]
    [(_ p) (%wrnl p)]
    [(_ . args) (%residual-newline . args)]
    [_ %residual-newline]))

(define-syntax write-shared
  (syntax-rules ()
    [(_ x) (%wrhw x (%sop))]
    [(_ x p) (%wrhw x p)]
    [(_ . args) (%residual-write-shared . args)]
    [_ %residual-write-shared]))

(define-syntax write-simple
  (syntax-rules ()
    [(_ x) (%wriw x (%sop))]
    [(_ x p) (%wriw x p)]
    [(_ . args) (%residual-write-simple . args)]
    [_ %residual-write-simple]))


;---------------------------------------------------------------------------------------------
; System interface
;---------------------------------------------------------------------------------------------

;load
;file-exists?
;delete-file
;command-line
;exit
;emergency-exit
;get-environment-variable
;get-environment-variables
;current-second
;current-jiffy
;jiffies-per-second
;features


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

(define %residual-write-char (unary-binary-adaptor write-char))
(define %residual-write-string (unary-binary-adaptor write-string))
(define %residual-newline (nullary-unary-adaptor newline))
(define %residual-display (unary-binary-adaptor display))
(define %residual-write (unary-binary-adaptor write))
(define %residual-write-simple (unary-binary-adaptor write-simple))
(define %residual-write-shared (unary-binary-adaptor write-shared))
