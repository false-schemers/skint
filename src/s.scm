
;---------------------------------------------------------------------------------------------
; SCHEME LIBRARY
;---------------------------------------------------------------------------------------------


;---------------------------------------------------------------------------------------------
; Derived expression types
;---------------------------------------------------------------------------------------------

; builtins:
;
; (quote const)
; (set! id expr)
; (set& id)
; (if expr1 expr2)
; (if expr1 expr2 expr3)
; (begin expr ...)
; (lambda args expr ...)
; (lambda* [arity expr] ...)
; (body expr ...) -- lexical scope for definitions
; (letcc id expr) 
; (withcc expr expr ...) 
; (define id expr)
; (define (id . args) expr ...)
; (define-syntax kw form)
; (syntax-lambda (id ...) form ...) 
; (syntax-rules (lit ...) [pat templ] ...) 
; (syntax-rules ellipsis (lit ...) [pat templ] ...)
; (cond-expand [ftest exp ...] ...) 

(define-syntax let-syntax
  (syntax-rules ()
    [(_ ([kw init] ...))
     (begin)]
    [(_ ([kw init] ...) . forms)
     ((syntax-lambda (kw ...) . forms)
      init ...)]))

(define-syntax letrec-syntax
  (syntax-rules ()
    [(_ ([key trans] ...) . forms) 
     (body (define-syntax key trans) ... . forms)]))

(define-syntax letrec
  (syntax-rules ()
    [(_ ([var init] ...) . forms)
     (body (define var init) ... . forms)]))

(define-syntax letrec*
  (syntax-rules ()
    [(_ ([var expr] ...) . forms)
     (let ([var #f] ...)
       (set! var expr)
       ...
       (body . forms))]))

(define-syntax let
  (syntax-rules ()
    [(_ () . forms) (body . forms)]
    [(_ ([var init] ...) . forms)
     ((lambda (var ...) . forms) init ...)]
    [(_ name ([var init] ...) . forms)
     ((letrec ((name (lambda (var ...) . forms))) name) init ...)]))

(define-syntax let*
  (syntax-rules ()
    [(_ () . forms) 
     (body . forms)]
    [(_ (first . more) . forms)
     (let (first) (let* more . forms))]))

(define-syntax let*-values
  (syntax-rules ()
    [(_ () . forms) (body . forms)]
    [(_ ([(a) x] . b*) . forms) (let ([a x]) (let*-values b* . forms))]
    [(_ ([aa x] . b*) . forms) (call-with-values (lambda () x) (lambda aa (let*-values b* . forms)))]))

(define-syntax %let-values-loop
  (syntax-rules ()
    [(_ (new-b ...) new-aa x map-b* () () . forms)
     (let*-values (new-b ... [new-aa x]) (let map-b* . forms))]
    [(_ (new-b ...) new-aa old-x map-b* () ([aa x] . b*) . forms)
     (%let-values-loop (new-b ... [new-aa old-x]) () x map-b* aa b* . forms)]
    [(_ new-b* (new-a ...) x (map-b ...) (a . aa) b* . forms)
     (%let-values-loop new-b* (new-a ... tmp-a) x (map-b ... [a tmp-a]) aa b* . forms)]
    [(_ new-b* (new-a ...) x (map-b ...) a b* . forms) 
     (%let-values-loop new-b* (new-a ... . tmp-a) x (map-b ... [a tmp-a]) () b* . forms)]))

(define-syntax let-values
  (syntax-rules ()
    [(_ () . forms) (let () . forms)]
    [(_ ([aa x] . b*) . forms)
     (%let-values-loop () () x () aa b* . forms)]))

(define-syntax %define-values-loop
  (syntax-rules ()
    [(_ new-aa ([a tmp-a] ...) () x)
     (begin
       (define a (begin)) ...
       (define () (call-with-values (lambda () x) (lambda new-aa (set! a tmp-a) ...))))]
    [(_ (new-a ...) (map-a ...) (a . aa) x) 
     (%define-values-loop (new-a ... tmp-a) (map-a ... [a tmp-a]) aa x)]
    [(_ (new-a ...) (map-a ...) a x) 
     (%define-values-loop (new-a ... . tmp-a) (map-a ... [a tmp-a]) () x)]))

(define-syntax define-values
  (syntax-rules ()
    [(_ () x) (define () (call-with-values (lambda () x) (lambda ())))] ; use idless define
    [(_ aa x) (%define-values-loop () () aa x)]))

(define-syntax and
  (syntax-rules ()
    [(_) #t]
    [(_ test) test]
    [(_ test . tests) (if test (and . tests) #f)]))

(define-syntax or
  (syntax-rules ()
    [(_) #f]
    [(_ test) test]
    [(_ test . tests) (let ([x test]) (if x x (or . tests)))]))

(define-syntax cond
  (syntax-rules (else =>)
    [(_) #f]
    [(_ (else . exps)) (begin . exps)]
    [(_ (x) . rest) (or x (cond . rest))]
    [(_ (x => proc) . rest) (let ([tmp x]) (cond [tmp (proc tmp)] . rest))]
    [(_ (x . exps) . rest) (if x (begin . exps) (cond . rest))]))

(define-syntax %case-test
  (syntax-rules () 
    [(_ k ()) #f]
    [(_ k (datum)) (eqv? k 'datum)] 
    [(_ k data) (memv k 'data)]))

(define-syntax %case
  (syntax-rules (else =>)
    [(_ key) (begin)]
    [(_ key (else => resproc))
     (resproc key)]
    [(_ key (else expr ...))
     (begin expr ...)]
    [(_ key ((datum ...) => resproc) . clauses)
     (if (%case-test key (datum ...))
         (resproc key)
         (%case key . clauses))]
    [(_ key ((datum ...) expr ...) . clauses)
     (if (%case-test key (datum ...))
         (begin expr ...)
         (%case key . clauses))]))

(define-syntax case
  (syntax-rules ()
    [(_ x . clauses) (let ([key x]) (%case key . clauses))]))

(define-syntax %do-step 
  (syntax-rules () 
    [(_ x) x] [(_ x y) y]))

(define-syntax do
  (syntax-rules ()
    [(_ ([var init step ...] ...)
       [test expr ...]
       command ...)
     (let loop ([var init] ...)
       (if test
           (begin expr ...)
           (let () command ...
             (loop (%do-step var step ...) ...))))]))


(define-syntax quasiquote
  (syntax-rules (unquote unquote-splicing quasiquote)
    [(_ ,x) x]
    [(_ (,@x . y)) (append x `y)]
    [(_ `x . d) (cons 'quasiquote (quasiquote (x) d))]
    [(_ ,x   d) (cons 'unquote (quasiquote (x) . d))]
    [(_ ,@x  d) (cons 'unquote-splicing (quasiquote (x) . d))]
    [(_ (x . y) . d) (cons (quasiquote x . d) (quasiquote y . d))]
    [(_ #(x ...) . d) (list->vector (quasiquote (x ...) . d))]
    [(_ x . d) 'x]))

(define-syntax when
  (syntax-rules ()
    [(_ test . rest) (if test (begin . rest))]))

(define-syntax unless
  (syntax-rules ()
    [(_ test . rest) (if (not test) (begin . rest))]))

(define-syntax case-lambda
  (syntax-rules ()
    [(_ [args . forms] ...) (lambda* [args (lambda args . forms)] ...)]))

;---------------------------------------------------------------------------------------------
; Delayed evaluation
;---------------------------------------------------------------------------------------------

(define promise? box?)

(define (make-promise o) (box (cons #t o)))
(define (make-lazy-promise o) (box (cons #f o)))

(define (force p)
  (let ([pc (unbox p)])
    (if (car pc)
        (cdr pc)
        (let* ([newp ((cdr pc))] [pc (unbox p)])
          (unless (car pc)
            (set-car! pc (car (unbox newp)))
            (set-cdr! pc (cdr (unbox newp)))
            (set-box! newp pc))
          (force p)))))

(define-syntax delay-force
  (syntax-rules () [(_ x) (make-lazy-promise (lambda () x))]))

(define-syntax delay
  (syntax-rules () [(_ x) (delay-force (make-promise x))]))


;---------------------------------------------------------------------------------------------
; Dynamic bindings
;---------------------------------------------------------------------------------------------

(define make-parameter
  (case-lambda
    [(value) 
     (case-lambda 
       [() value]
       [(x) (set! value x)]
       [(x s) (if s (set! value x) x)])]
    [(init converter)
     (let ([value (converter init)])
       (case-lambda 
         [() value]
         [(x) (set! value (converter x))]
         [(x s) (if s (set! value x) (converter x))]))]))

(define-syntax %parameterize-loop
  (syntax-rules ()
    [(_ ([param value p old new] ...) () body)
     (let ([p param] ...)
       (let ([old (p)] ... [new (p value #f)] ...)
         (dynamic-wind
           (lambda () (p new #t) ...)
           (lambda () . body)
           (lambda () (p old #t) ...))))]
    [(_ args ([param value] . rest) body)
     (%parameterize-loop ([param value p old new] . args) rest body)]))

(define-syntax parameterize
  (syntax-rules ()
    [(_ ([param value] ...) . body)
     (%parameterize-loop () ([param value] ...) body)]))


;---------------------------------------------------------------------------------------------
; Record type definitions
;---------------------------------------------------------------------------------------------

; integrables:
;
; (record? x (rtd))
; (make-record rtd n (fill #f))
; (record-length r)
; (record-ref r i)
; (record-set! r i v)

(define (new-record-type name fields) ; stub
  (cons name fields))

; see http://okmij.org/ftp/Scheme/macro-symbol-p.txt
(define-syntax %id-eq?? 
  (syntax-rules ()
    [(_ id b kt kf)
     ((syntax-lambda (id ok) ((syntax-rules () [(_ b) (id)]) ok))
      (syntax-rules () [(_) kf]) (syntax-rules () [(_) kt]))]))

(define-syntax %id-assq??
  (syntax-rules ()
    [(_ id () kt kf) 
     kf]
    [(_ id ([id0 . r0] . idr*) kt kf) 
     (%id-eq?? id id0 (kt . r0) (%id-assq?? id idr* kt kf))]))
 
(define-syntax %drt-init
  (syntax-rules ()
    [(_  r () fi* (x ...)) 
     (begin x ... r)]
    [(_  r (id0 . id*) fi* (x ...))
     (%id-assq?? id0 fi* 
       (syntax-rules () [(_ i0) (%drt-init r id* fi* (x ... (record-set! r i0 id0)))]) 
       (syntax-error "id in define-record-type constructor is not a field:" id0))]))

(define-syntax %drt-unroll
  (syntax-rules ()
    [(_ rtn (consn id ...) predn () ([f i] ...) ([a ia] ...) ([m im] ...))
     (begin
       (define rtn 
         (new-record-type 'rtn '(f ...)))
       (define consn 
         (lambda (id ...) 
           (let ([r (make-record rtn (syntax-length (f ...)))])
             (%drt-init r (id ...) ([f i] ...) ()))))
       (define predn 
         (lambda (obj) (record? obj rtn)))
       (define a 
         (lambda (obj) (record-ref obj ia)))
       ...
       (define m 
         (lambda (obj val) (record-set! obj im val)))
       ...)]
    [(_ rtn cf* predn ([fn accn] fam ...) (fi ...) (ai ...) (mi ...))
     (%drt-unroll rtn cf* predn (fam ...) 
       (fi ... [fn (syntax-length (fi ...))]) 
       (ai ... [accn (syntax-length (fi ...))]) 
       (mi ...))]
    [(_  rtn cf* predn ([fn accn modn] fam ...) (fi ...) (ai ...) (mi ...))
     (%drt-unroll rtn cf* predn (fam ...) 
       (fi ... [fn (syntax-length (fi ...))]) 
       (ai ... [accn (syntax-length (fi ...))]) 
       (mi ... [modn (syntax-length (fi ...))]))]))

(define-syntax define-record-type
  (syntax-rules ()
    [(_ rtn (consn id ...) predn (fn . am) ...)
     (%drt-unroll rtn (consn id ...) predn ((fn . am) ...) () () ())]))


;---------------------------------------------------------------------------------------------
; Equivalence predicates
;---------------------------------------------------------------------------------------------

; integrables:
;
; (eq? x y)
; (eqv? x y)
; (equal? x y)


;---------------------------------------------------------------------------------------------
; Boxes, aka cells
;---------------------------------------------------------------------------------------------

; integrables:
;
; (box? x)
; (box x)
; (unbox x)
; (set-box! x y)


;---------------------------------------------------------------------------------------------
; Exact integer numbers (fixnums)
;---------------------------------------------------------------------------------------------

; integrables:
;
; (fixnum? o)
; (fxzero? x)
; (fxpositive? x)
; (fxnegative? x)
; (fxeven? x)
; (fxodd? x)
; (fx+ x ...)
; (fx* x ...)
; (fx- x y ...)
; (fx/ x y ...)
; (fxquotient x y)
; (fxremainder x y)
; (fxmodquo x y) +
; (fxmodulo x y) 
; (fxeucquo x y) + a.k.a. euclidean-quotient, R6RS div
; (fxeucrem x y) + a.k.a. euclidean-remainder, R6RS mod
; (fxneg x)
; (fxabs x)
; (fx<? x y z ...)
; (fx<=? x y z ...)
; (fx>? x y z ...)
; (fx>=? x y z ...)
; (fx=? x y z ...)
; (fx!=? x y) +
; (fxmin x y)
; (fxmax x y)
; (fxneg x)
; (fxabs x)
; (fxgcd x y)
; (fxexpt x y)
; (fxsqrt x)
; (fxnot x)
; (fxand x ...)
; (fxior x ...)
; (fxxor x ...)
; (fxsll x y)
; (fxsrl x y)
; (fixnum->flonum x)
; (fixnum->string x (radix 10))
; (string->fixnum s (radix 10))

;TBD:
;
;fx-width
;fx-greatest
;fx-least
;fxarithmetic-shift-right
;fxarithmetic-shift-left
;fxlength cf. integer-length (+ 1 (integer-length i)) 
;  is the number of bits needed to represent i in a signed twos-complement representation
;  0 => 0, 1 => 1, -1 => 0, 7 => 3, -7 => 3, 8 => 4, -8 => 3
;fxbit-count cf. bit-count
;  Returns the population count of 1's (i >= 0) or 0's (i < 0)
;  0 => 0, -1 => 0, 7 => 3, 13 => 3, -13 => 2  


;---------------------------------------------------------------------------------------------
; Inexact floating-point numbers (flonums)
;---------------------------------------------------------------------------------------------

; integrables:
;
; (flonum? o)
; (flzero? x)
; (flpositive? x)
; (flnegative? x)
; (flinteger? x)
; (flnan? x)
; (flinfinite? x)
; (flfinite? x)
; (fleven? x)
; (flodd? x)
; (fl+ x ...)
; (fl* x ...)
; (fl- x y ...)
; (fl/ x y ...)
; (flneg x)
; (flabs x)
; (flgcd x y)
; (flexpt x y)
; (flsqrt x)
; (flfloor x)
; (flceiling x)
; (fltruncate x)
; (flround x)
; (flexp x)
; (fllog x (base fl-e))
; (flsin x)
; (flcos x)
; (fltan x)
; (flasin x)
; (flacos x)
; (flatan (y) x)
; (fl<? x y z ...)
; (fl<=? x y z ...)
; (fl>? x y z ...)
; (fl>=? x y z ...)
; (fl=? x y z ...)
; (fl!=? x y) +
; (flmin x y)
; (flmax x y)
; (flonum->fixnum x)
; (flonum->string x)
; (string->flonum s)


;---------------------------------------------------------------------------------------------
; Numbers (fixnums or flonums)
;---------------------------------------------------------------------------------------------

; integrables:
;
; (number? x)
; (complex? x) == number? what about inf and nan?
; (real? x) == number? what about inf and nan?
; (rational? x)
; (integer? x)
; (exact? x)
; (inexact? x)
; (exact-integer? x) == fixnum?
; (finite? x)
; (infinite? x)
; (nan? x)
; (= x y z ...)
; (< x y z ...)
; (> x y z ...)
; (<= x y z ...)
; (>= x y z ...)
; (zero? x)
; (positive? x)
; (negative? x)
; (odd? x)
; (even? x)
; (max x y ...) 
; (min x y ...) 
; (+ x ...)
; (* x ...)
; (- x y ...)
; (/ x y ...)
; (abs x)
; (floor-quotient x y)
; (floor-remainder x y)
; (truncate-quotient x y)
; (truncate-remainder x y)
; (quotient x y) == truncate-quotient
; (remainder x y) == truncate-remainder
; (modulo x y) == floor-remainder
; (gcd x y)
; (floor x)
; (ceiling x)
; (truncate x)
; (round x)
; (exp x)
; (log x (base fl-e))
; (sin x)
; (cos x)
; (tan x)
; (asin x)
; (acos x)
; (atan (y) x)
; (sqrt x)
; (expt x y)
; (inexact x)
; (exact x)
; (number->string x (radix 10))
; (string->number x (radix 10))

(define (floor/ x y)
  (values (floor-quotient x y) (floor-remainder x y)))

(define (truncate/ x y)
  (values (truncate-quotient x y) (truncate-remainder x y)))

(define (lcm . args)
  (if (null? args) 1
      (let loop ([x (car args)] [args (cdr args)])
        (if (null? args) x
            (let* ([y (car args)] [g (gcd x y)])
              (loop (if (zero? g) g (* (quotient (abs x) g) (abs y))) (cdr args)))))))

(define (numerator n) n)

(define (denominator n) 1)

(define (rationalize n d) n)

(define (square x) (* x x))

(define (exact-integer-sqrt x)
  (let ([r (fxsqrt x)])
    (values r (- x (* r r)))))

(define (make-rectangular r i)
  (if (= i 0) r (error "make-rectangular: nonzero imag part not supported" i)))

(define (make-polar m a)
  (cond [(= a 0) m]
        [(= a 3.141592653589793238462643) (- m)]
        [else (error "make-polar: angle not supported" a)]))

(define (real-part x) x)

(define (imag-part x) 0)

(define (magnitude x) (abs x))

(define (angle x) (if (negative? x) 3.141592653589793238462643 0))


;---------------------------------------------------------------------------------------------
; Booleans
;---------------------------------------------------------------------------------------------

; integrables:
;
; (boolean? x)
; (not x)
; (boolean=? x y z ...)


;---------------------------------------------------------------------------------------------
; Null and Pairs
;---------------------------------------------------------------------------------------------

; integrables:
;
; (null? x)
; (pair? x)
; (cons x y)
; (car p)
; (cdr p)
; (set-car! p v)
; (set-cdr! p v)
; (caar p)
; ...
; (cddddr p)


;---------------------------------------------------------------------------------------------
; Lists
;---------------------------------------------------------------------------------------------

; integrables:
;
; (list? x)
; (list x ...)
; (make-list n (i #f))
; (length l)
; (list-ref l i)
; (list-set! l i x)
; (list-cat l1 l2)  + 2-arg append
; (memq v l)  ; TODO: make sure memq doesn't fail on improper/circular list
; (memv v l)  ; TODO: make sure memv doesn't fail on improper/circular list
; (meme v l)  + 2-arg member; TODO: make sure meme checks list ^
; (assq v y)   
; (assv v y) ; TODO: make sure assv checks list
; (asse v y) + 2-arg assoc; TODO: make sure asse checks list ^
; (list-tail l i)
; (last-pair l)
; (reverse l)
; (reverse! l) +
; (list-copy l) ; TODO: make sure list-copy checks list for circularity
; (circular? x) +

(define (%append . args)
  (let loop ([args args])
    (cond [(null? args) '()]
          [(null? (cdr args)) (car args)]
          [else (list-cat (car args) (loop (cdr args)))])))

(define-syntax append
  (syntax-rules ()
    [(_) '()] [(_ x) x]
    [(_ x y) (list-cat x y)]
    [(_ x y z ...) (list-cat x (append y z ...))]
    [_ %append]))

(define (%member v l . ?eq)
  (if (null? ?eq) 
      (meme v l)
      (let loop ([v v] [l l] [eq (car ?eq)])
        (and (pair? l) 
             (if (eq v (car l)) 
                 l 
                 (loop v (cdr l) eq))))))

(define-syntax member
  (syntax-rules ()
    [(_ v l) (meme v l)] 
    [(_ . args) (%member . args)]
    [_ %member]))

(define (%assoc v al . ?eq)
  (if (null? ?eq) 
      (asse v al)
      (let loop ([v v] [al al] [eq (car ?eq)])
        (and (pair? al) 
             (if (eq v (caar al)) 
                 (car al) 
                 (loop v (cdr al) eq))))))

(define-syntax assoc
  (syntax-rules ()
    [(_ v al) (asse v al)] 
    [(_ . args) (%assoc . args)]
    [_ %assoc]))

(define (list-copy obj)
  (let loop ([obj obj])
    (if (pair? obj)
        (cons (car obj) (loop (cdr obj)))
        obj)))

(define (%list* x . l)
  (let loop ([x x] [l l])
    (if (null? l) x (cons x (loop (car l) (cdr l))))))

(define-syntax list*
  (syntax-rules ()
    [(_ x) x]
    [(_ x y) (cons x y)]
    [(_ x y z ...) (cons x (list* y z ...))]
    [(_ . args) (%list* . args)]
    [_ %list*]))

(define-syntax cons* list*)


;---------------------------------------------------------------------------------------------
; Symbols
;---------------------------------------------------------------------------------------------

; integrables:
;
; (symbol? x)
; (symbol->string y)
; (string->symbol s)
; (symbol=? x y z ...)


;---------------------------------------------------------------------------------------------
; Characters
;---------------------------------------------------------------------------------------------

; integrables:
;
; (char? x)
; (char-cmp c1 c2) +
; (char=? c1 c2 c ...)
; (char<? c1 c2 c ...)
; (char>? c1 c2 c ...)
; (char<=? c1 c2 c ...)
; (char>=? c1 c2 c ...)
; (char-ci-cmp c1 c2) +
; (char-ci=? c1 c2 c ...)
; (char-ci<? c1 c2 c ...)
; (char-ci>? c1 c2 c ...)
; (char-ci<=? c1 c2 c ...)
; (char-ci>=? c1 c2 c ...)
; (char-alphabetic? c)
; (char-numeric? x)
; (char-whitespace? c)
; (char-upper-case? c)
; (char-lower-case? c)
; (char-upcase c)
; (char-downcase c)
; (char-foldcase c)
; (digit-value c)
; (char->integer c) 
; (integer->char n)


;---------------------------------------------------------------------------------------------
; Strings
;---------------------------------------------------------------------------------------------

; integrables:
;
; (string? x)
; (string c ...)
; (make-string n (i #\space))
; (string-length s)
; (string-ref x i)
; (string-set! x i v)
; (list->string l)
; (%string->list1 s) + 
; (string-cat s1 s2) +
; (substring s from to)
; (string-position s c) +
; (string-cmp s1 s2) +
; (string=? s1 s2 s ...)
; (string<? s1 s2 s ...)
; (string>? s1 s2 s ...)
; (string<=? s1 s2 s ...)
; (string>=? s1 s2 s ...)
; (string-ci-cmp s1 s2) +
; (string-ci=? s1 s2 s ...)
; (string-ci<? s1 s2 s ...)
; (string-ci>? s1 s2 s ...)
; (string-ci<=? s1 s2 s ...)
; (string-ci>=? s1 s2 s ...)
; (string-upcase s)
; (string-downcase s)
; (string-foldcase s)

(define (substring->list str start end)
  (let loop ([i (fx- end 1)] [l '()])
    (if (fx<? i start) l (loop (fx- i 1) (cons (string-ref str i) l)))))

(define %string->list
  (case-lambda
     [(str) (%string->list1 str)]
     [(str start) (substring->list str start (string-length str))]
     [(str start end) (substring->list str start end)]))

(define-syntax string->list
  (syntax-rules ()
    [(_ x) (%string->list1 x)]
    [(_ . r) (%string->list . r)]
    [_ %string->list]))

(define (substring-copy! to at from start end)
  (let ([limit (fxmin end (fx+ start (fx- (string-length to) at)))])
    (if (fx<=? at start)
        (do ([i at (fx+ i 1)] [j start (fx+ j 1)]) 
          [(fx>=? j limit)]
          (string-set! to i (string-ref from j)))
        (do ([i (fx+ at (fx- (fx- end start) 1)) (fx- i 1)] [j (fx- limit 1) (fx- j 1)])
          [(fx<? j start)]
          (string-set! to i (string-ref from j))))))

(define string-copy!
  (case-lambda
     [(to at from) (substring-copy! to at from 0 (string-length from))]
     [(to at from start) (substring-copy! to at from start (string-length from))]
     [(to at from start end) (substring-copy! to at from start end)]))

(define string-copy 
  (case-lambda
     [(str) (substring str 0 (string-length str))]  ; TODO: %scpy ?
     [(str start) (substring str start (string-length str))]
     [(str start end) (substring str start end)]))

(define (substring-fill! str c start end)
  (do ([i start (fx+ i 1)]) [(fx>=? i end)] (string-set! str i c)))

(define string-fill!
  (case-lambda
     [(str c) (substring-fill! str c 0 (string-length str))]
     [(str c start) (substring-fill! str c start (string-length str))]
     [(str c start end) (substring-fill! str c start end)]))

(define (substring-vector-copy! to at from start end)
  (let ([limit (fxmin end (fx+ start (fx- (vector-length to) at)))])
    (do ([i at (fx+ i 1)] [j start (fx+ j 1)]) 
      [(fx>=? j limit) to]
      (vector-set! to i (string-ref from j)))))

(define (substring->vector str start end)
  (substring-vector-copy! (make-vector (fx- end start)) 0 str start end))

(define string->vector
  (case-lambda
     [(str) (substring->vector str 0 (string-length str))]
     [(str start) (substring->vector str start (string-length str))]
     [(str start end) (substring->vector str start end)]))

(define (strings-sum-length strs)
  (let loop ([strs strs] [l 0])
    (if (null? strs) l (loop (cdr strs) (fx+ l (string-length (car strs)))))))

(define (strings-copy-into! to strs)
  (let loop ([strs strs] [i 0])
    (if (null? strs)
        to
        (let ([str (car strs)] [strs (cdr strs)])
          (let ([len (string-length str)])
            (substring-copy! to i str 0 len)
            (loop strs (fx+ i len)))))))  

(define (%string-append . strs)
  (strings-copy-into! (make-string (strings-sum-length strs)) strs))

(define-syntax string-append
  (syntax-rules ()
    [(_) ""] [(_ x) (%cks x)]
    [(_ x y) (string-cat x y)]
    [(_ . r) (%string-append . r)]
    [_ %string-append]))


;---------------------------------------------------------------------------------------------
; Vectors
;---------------------------------------------------------------------------------------------

; integrables:
;
; (vector? x)
; (vector x ...)
; (make-vector n (i #f))
; (vector-length v)
; (vector-ref v i)
; (vector-set! v i x)
; (%vector->list1 v) +
; (list->vector l)
; (vector-cat v1 v2) +

(define (subvector->list vec start end)
  (let loop ([i (fx- end 1)] [l '()])
    (if (fx<? i start) l (loop (fx- i 1) (cons (vector-ref vec i) l)))))

(define %vector->list
  (case-lambda
     [(vec) (%vector->list1 vec)]
     [(vec start) (subvector->list vec start (vector-length vec))]
     [(vec start end) (subvector->list vec start end)]))

(define-syntax vector->list
  (syntax-rules ()
    [(_ x) (%vector->list1 x)]
    [(_ . r) (%vector->list . r)]
    [_ %vector->list]))

(define (subvector-copy! to at from start end)
  (let ([limit (fxmin end (fx+ start (fx- (vector-length to) at)))])
    (if (fx<=? at start)
        (do ([i at (fx+ i 1)] [j start (fx+ j 1)]) 
          [(fx>=? j limit)]
          (vector-set! to i (vector-ref from j)))
        (do ([i (fx+ at (fx- (fx- end start) 1)) (fx- i 1)] [j (fx- limit 1) (fx- j 1)])
          [(fx<? j start)]
          (vector-set! to i (vector-ref from j))))))

(define vector-copy!
  (case-lambda
     [(to at from) (subvector-copy! to at from 0 (vector-length from))]
     [(to at from start) (subvector-copy! to at from start (vector-length from))]
     [(to at from start end) (subvector-copy! to at from start end)]))

(define (subvector vec start end)  ; TODO: %vsub?
  (let ([v (make-vector (fx- end start))])
    (subvector-copy! v 0 vec start end)
    v))

(define vector-copy
  (case-lambda
     [(vec) (subvector vec 0 (vector-length vec))] ; TODO: %vcpy ?
     [(vec start) (subvector vec start (vector-length vec))]
     [(vec start end) (subvector vec start end)]))

(define (subvector-fill! vec x start end)
  (do ([i start (fx+ i 1)]) [(fx>=? i end)] (vector-set! vec i x)))

(define vector-fill!
  (case-lambda
     [(vec x) (subvector-fill! vec x 0 (vector-length vec))]
     [(vec x start) (subvector-fill! vec x start (vector-length vec))]
     [(vec x start end) (subvector-fill! vec x start end)]))

(define (subvector-string-copy! to at from start end)
  (let ([limit (fxmin end (fx+ start (fx- (string-length to) at)))])
    (do ([i at (fx+ i 1)] [j start (fx+ j 1)]) 
      [(fx>=? j limit) to]
      (string-set! to i (vector-ref from j)))))

(define (subvector->string vec start end)
  (subvector-string-copy! (make-string (fx- end start)) 0 vec start end))

(define vector->string
  (case-lambda
     [(vec) (subvector->string vec 0 (vector-length vec))]
     [(vec start) (subvector->string vec start (vector-length vec))]
     [(vec start end) (subvector->string vec start end)]))

(define (vectors-sum-length vecs)
  (let loop ([vecs vecs] [l 0])
    (if (null? vecs) l (loop (cdr vecs) (fx+ l (vector-length (car vecs)))))))

(define (vectors-copy-into! to vecs)
  (let loop ([vecs vecs] [i 0])
    (if (null? vecs)
        to
        (let ([vec (car vecs)] [vecs (cdr vecs)])
          (let ([len (vector-length vec)])
            (subvector-copy! to i vec 0 len)
            (loop vecs (fx+ i len)))))))  

(define (%vector-append . vecs)
  (vectors-copy-into! (make-vector (vectors-sum-length vecs)) vecs))

(define-syntax vector-append
  (syntax-rules ()
    [(_) '#()] [(_ x) (%ckv x)]
    [(_ x y) (vector-cat x y)]
    [(_ . r) (%vector-append . r)]
    [_ %vector-append]))


;---------------------------------------------------------------------------------------------
; Bytevectors
;---------------------------------------------------------------------------------------------

; integrables:
;
; (bytevector? x)
; (make-bytevector n (u8 0))
; (bytevector u8 ...)
; (bytevector-length b)
; (bytevector-u8-ref b i)
; (bytevector-u8-set! b i u8)
; (list->bytevector l)
; (subbytevector b from to) +
; (bytevector=? b1 b2 b ...)

(define (subbytevector->list bvec start end)
  (let loop ([i (fx- end 1)] [l '()])
    (if (fx<? i start) l (loop (fx- i 1) (cons (bytevector-u8-ref bvec i) l)))))

(define bytevector->list
  (case-lambda
     [(bvec) (subbytevector->list bvec 0 (bytevector-length bvec))]
     [(bvec start) (subbytevector->list bvec start (bytevector-length bvec))]
     [(bvec start end) (subbytevector->list bvec start end)]))

(define (subbytevector-copy! to at from start end)
  (let ([limit (fxmin end (fx+ start (fx- (bytevector-length to) at)))])
    (if (fx<=? at start)
        (do ([i at (fx+ i 1)] [j start (fx+ j 1)]) 
          [(fx>=? j limit)]
          (bytevector-u8-set! to i (bytevector-u8-ref from j)))
        (do ([i (fx+ at (fx- end start 1)) (fx- i 1)] [j (fx- limit 1) (fx- j 1)])
          [(fx<? j start)]
          (bytevector-u8-set! to i (bytevector-u8-ref from j))))))

(define bytevector-copy!
  (case-lambda
     [(to at from) (subbytevector-copy! to at from 0 (bytevector-length from))]
     [(to at from start) (subbytevector-copy! to at from start (bytevector-length from))]
     [(to at from start end) (subbytevector-copy! to at from start end)]))

(define bytevector-copy
  (case-lambda
     [(bvec) (subbytevector bvec 0 (bytevector-length bvec))]
     [(bvec start) (subbytevector bvec start (bytevector-length bvec))]
     [(bvec start end) (subbytevector bvec start end)]))

(define (subbytevector-fill! bvec b start end)
  (do ([i start (fx+ i 1)]) [(fx>=? i end)] (bytevector-u8-set! bvec i b)))

(define bytevector-fill!
  (case-lambda
     [(bvec b) (subbytevector-fill! bvec b 0 (bytevector-length bvec))]
     [(bvec b start) (subbytevector-fill! bvec b start (bytevector-length bvec))]
     [(bvec b start end) (subbytevector-fill! bvec b start end)]))

(define (%bytevectors-sum-length bvecs)
  (let loop ([bvecs bvecs] [l 0])
    (if (null? bvecs) l (loop (cdr bvecs) (fx+ l (bytevector-length (car bvecs)))))))

(define (%bytevectors-copy-into! to bvecs)
  (let loop ([bvecs bvecs] [i 0])
    (if (null? bvecs) to
        (let ([bvec (car bvecs)] [bvecs (cdr bvecs)])
          (let ([len (bytevector-length bvec)])
            (subbytevector-copy! to i bvec 0 len)
            (loop bvecs (fx+ i len)))))))  

(define (bytevector-append . bvecs)
  (%bytevectors-copy-into! (make-bytevector (%bytevectors-sum-length bvecs)) bvecs))

(define (subutf8->string vec start end)
  (let ([p (open-output-string)])
    (write-subbytevector vec start end p)
    ; todo: make a single operation: get-final-output-string (can reuse cbuf?)
    (let ([s (get-output-string p)]) (close-output-port p) s)))

(define utf8->string
  (case-lambda
    [(bvec) (subutf8->string bvec 0 (bytevector-length bvec))]
    [(bvec start) (subutf8->string bvec start (bytevector-length bvec))]
    [(bvec start end) (subutf8->string bvec start end)]))

(define (substring->utf8 str start end)
  (let ([p (open-output-bytevector)])
    (write-substring str start end p)
    ; todo: make a single operation: get-final-output-bytevector (can reuse cbuf?)
    (let ([v (get-output-bytevector p)]) (close-output-port p) v)))

(define string->utf8
  (case-lambda
    [(str) (substring->utf8 str 0 (string-length str))]
    [(str start) (substring->utf8 str start (string-length str))]
    [(str start end) (substring->utf8 str start end)]))


;---------------------------------------------------------------------------------------------
; Control features
;---------------------------------------------------------------------------------------------

; integrables:
;
; (procedure? x)
; (values x ...)
; (call-with-values thunk receiver)

; builtins:
;
; (dynamic-wind before during after)


(define (%apply p x . l)
  (apply-to-list p 
    (let loop ([x x] [l l])
      (if (null? l) x (cons x (loop (car l) (cdr l)))))))

(define-syntax apply
  (syntax-rules ()
    [(_ p l) (apply-to-list p l)]
    [(_ p a b ... l) (apply-to-list p (list* a b ... l))]
    [(_ . args) (%apply . args)]
    [_ %apply]))

; call/cc and dynamic-wind

(define (%call/cc p) (letcc k (p k)))

(define-syntax call/cc
  (syntax-rules ()
    [(_ p) (letcc k (p k))]
    [(_ . args) (%call/cc . args)]
    [_ %call/cc])) 

(define-syntax call-with-current-continuation call/cc)

(define (%map1 p l)
  (let loop ([l l] [r '()])
    (if (pair? l) 
        (loop (cdr l) (cons (p (car l)) r)) 
        (reverse! r))))

(define (%map2 p l1 l2)
  (let loop ([l1 l1] [l2 l2] [r '()])
    (if (and (pair? l1) (pair? l2))
        (loop (cdr l1) (cdr l2) (cons (p (car l1) (car l2)) r))
        (reverse! r))))

(define (%map p l . l*)
  (cond [(null? l*) (%map1 p l)]
        [(null? (cdr l*)) (%map2 p l (car l*))] 
        [else
         (let loop ([l* (cons l l*)] [r '()])
           (if (let lp ([l* l*]) 
                 (or (null? l*) (and (pair? (car l*)) (lp (cdr l*)))))
               (loop (%map1 cdr l*) (cons (apply p (%map1 car l*)) r))
               (reverse! r)))]))

(define-syntax map
  (syntax-rules ()
    [(_ p l) (%map1 p l)]
    [(_ p l1 l2) (%map2 p l1 l2)]
    [(_ . args) (%map . args)]
    [_ %map])) 

(define (%for-each1 p l)
  (let loop ([l l]) 
    (if (pair? l) 
        (begin (p (car l))
               (loop (cdr l))))))

(define (%for-each2 p l1 l2)
  (let loop ([l1 l1] [l2 l2]) 
    (if (and (pair? l1) (pair? l2)) 
        (begin (p (car l1) (car l2)) 
               (loop (cdr l1) (cdr l2))))))

(define (%for-each p l . l*)
  (cond [(null? l*) (%for-each1 p l)]
        [(null? (cdr l*)) (%for-each2 p l (car l*))]
        [else
         (let loop ([l* (cons l l*)])
           (if (let lp ([l* l*]) 
                 (or (null? l*) (and (pair? (car l*)) (lp (cdr l*)))))
               (begin (apply p (map car l*)) (loop (map cdr l*)))))]))

(define-syntax for-each
  (syntax-rules ()
    [(_ p l) (%for-each1 p l)]
    [(_ p l1 l2) (%for-each2 p l1 l2)]
    [(_ . args) (%for-each . args)]
    [_ %for-each]))

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
; Exceptions
;---------------------------------------------------------------------------------------------

(define (abort) (%abort))

(define (reset) (%exit 1))

(define (set-reset-handler! fn) (set! reset fn))

(define (print-error-message prefix args ep)
  (define (pr-where args ep)
    (when (pair? args) 
      (cond [(not (car args)) 
             (write-string ": " ep)
             (pr-msg (cdr args) ep)]
            [(symbol? (car args)) 
             (write-string " in " ep) (write (car args) ep) (write-string ": " ep)
             (pr-msg (cdr args) ep)]
            [else 
             (write-string ": " ep)
             (pr-msg args ep)]))) 
  (define (pr-msg args ep)
    (when (pair? args) 
      (cond [(string? (car args))
             (display (car args) ep)
             (pr-rest (cdr args) ep)]
            [else (pr-rest args ep)])))
   (define (pr-rest args ep)
     (when (pair? args)
       (write-char #\space ep) (write (car args) ep)
       (pr-rest (cdr args) ep)))
   (cond [(or (string? prefix) (symbol? prefix)) 
          (write-string prefix ep)]
         [else (write-string "Error" ep)])
   (pr-where args ep)
   (newline ep))

(define (simple-error . args)
  (let ([ep (current-error-port)])
    (newline ep)
    (print-error-message "Error" args ep)
    (reset)))

(define (assertion-violation . args)
  (let ([ep (current-error-port)])
    (newline ep)
    (print-error-message "Assertion violation" args ep)
    (%exit 1)))

(define-record-type <error-object>
  (error-object kind message irritants)
  error-object?
  (kind error-object-kind)
  (message error-object-message)
  (irritants error-object-irritants))

(define (error msg . args)
  (raise (error-object #f msg args)))

(define current-exception-handler 
  (make-parameter
    (letrec 
      ([default-handler
        (case-lambda 
          [() default-handler] ; make this one its own parent 
          [(obj) 
           (if (error-object? obj)
               (apply simple-error (error-object-kind obj) (error-object-message obj) (error-object-irritants obj)) 
               (simple-error #f "unhandled exception" obj))])])
      default-handler)))

(define (with-exception-handler handler thunk)
  (let ([eh (current-exception-handler)])
    (parameterize ([current-exception-handler (case-lambda [() eh] [(obj) (handler obj)])])
      (thunk)))) 

(define (raise obj)
  (let ([eh (current-exception-handler)])
    (parameterize ([current-exception-handler (eh)])
      (eh obj)
      (raise (error-object 'raise "exception handler returned" (list eh obj))))))

(define (raise-continuable obj)
  (let ([eh (current-exception-handler)])
    (parameterize ([current-exception-handler (eh)])
      (eh obj))))

(define-syntax %guard-aux
  (syntax-rules (else =>)
    [(_ reraise (else result1 result2 ...))
     (begin result1 result2 ...)]
    [(_ reraise (test => result))
     (let ([temp test]) (if temp (result temp) reraise))]
    [(_ reraise (test => result) clause1 clause2 ...)
     (let ([temp test])
       (if temp
           (result temp)
           (%guard-aux reraise clause1 clause2 ...)))]
    [(_ reraise (test)) (or test reraise)]
    [(_ reraise (test) clause1 clause2 ...)
     (let ([temp test])
       (if temp temp (%guard-aux reraise clause1 clause2 ...)))]
    [(_ reraise (test result1 result2 ...))
     (if test (begin result1 result2 ...) reraise)]
    [(_ reraise (test result1 result2 ...) clause1 clause2 ...)
     (if test
         (begin result1 result2 ...)
         (%guard-aux reraise clause1 clause2 ...))]))

(define-syntax guard
  (syntax-rules ()
    [(guard (var clause ...) e1 e2 ...)
    ((call/cc
      (lambda (guard-k)
        (with-exception-handler
          (lambda (condition)
            ((call/cc
                (lambda (handler-k)
                  (guard-k
                    (lambda ()
                      (let ([var condition])
                        (%guard-aux
                          (handler-k
                            (lambda ()
                              (raise-continuable condition)))
                          clause
                          ...))))))))
          (lambda ()
            (call-with-values
              (lambda () e1 e2 ...)
              (lambda args
                (guard-k (lambda () (apply values args))))))))))]))

(define (read-error msg . args)
  (raise (error-object 'read msg args)))

(define (read-error? obj)
  (and (error-object? obj) (eq? (error-object-kind obj) 'read))) 

(define (file-error msg . args)
  (raise (error-object 'file msg args)))

(define (file-error? obj)
  (and (error-object? obj) (eq? (error-object-kind obj) 'file))) 


;---------------------------------------------------------------------------------------------
; Environments and evaluation
;---------------------------------------------------------------------------------------------

;TBD:
;
;environment
;scheme-report-environment
;null-environment
;interaction-environment
;eval


;---------------------------------------------------------------------------------------------
; I/O Ports
;---------------------------------------------------------------------------------------------

; integrables:
;
; (input-port? x)
; (output-port? x)
; (input-port-open? p)
; (output-port-open? p)
; (%current-input-port) +
; (%current-output-port) +
; (%current-error-port) +
; (%set-current-input-port! p) +
; (%set-current-output-port! p) +
; (%set-current-error-port! p) +
; (standard-input-port) +
; (standard-output-port) +
; (standard-error-port) +
; (%open-input-file s) +
; (%open-binary-input-file s) +
; (%open-output-file x) +
; (%open-binary-output-file x) +
; (close-input-port p)
; (close-output-port p)
; (open-input-string s)
; (open-output-string)
; (get-output-string p)
; (open-input-bytevector b)
; (open-output-bytevector)
; (get-output-bytevector p)

(define (port? x) (or (input-port? x) (output-port? x)))
(define textual-port? port?) ; all ports are bimodal
(define binary-port? port?)  ; all ports are bimodal

(define %current-input-port-parameter
  (case-lambda 
    [() (%current-input-port)]
    [(p) (%set-current-input-port! p)]
    [(p s) (if s (%set-current-input-port! p) p)]))

(define-syntax current-input-port
  (syntax-rules ()
    [(_) (%current-input-port)]
    [(_ p) (%set-current-input-port! p)]
    [(_ . r) (%current-input-port-parameter . r)]
    [_ %current-input-port-parameter]))

(define %current-output-port-parameter
  (case-lambda 
    [() (%current-output-port)]
    [(p) (%set-current-output-port! p)]
    [(p s) (if s (%set-current-output-port! p) p)]))

(define-syntax current-output-port
  (syntax-rules ()
    [(_) (%current-output-port)]
    [(_ p) (%set-current-output-port! p)]
    [(_ . r) (%current-output-port-parameter . r)]
    [_ %current-output-port-parameter]))

(define %current-error-port-parameter
  (case-lambda 
    [() (%current-error-port)]
    [(p) (%set-current-error-port! p)]
    [(p s) (if s (%set-current-error-port! p) p)]))

(define-syntax current-error-port
  (syntax-rules ()
    [(_) (%current-error-port)]
    [(_ p) (%set-current-error-port! p)]
    [(_ . r) (%current-error-port-parameter . r)]
    [_ %current-error-port-parameter]))

(define (open-input-file fn)
  (or (%open-input-file fn)
      (file-error "cannot open input file" fn)))

(define (open-output-file fn)
  (or (%open-output-file fn)
      (file-error "cannot open output file" fn)))

(define (open-binary-input-file fn)
  (or (%open-binary-input-file fn)
      (file-error "cannot open binary input file" fn)))

(define (open-binary-output-file fn)
  (or (%open-binary-output-file fn)
      (file-error "cannot open binary output file" fn)))

(define (close-port p)
  (if (input-port? p) (close-input-port p))
  (if (output-port? p) (close-output-port p)))

(define (call-with-port port proc)
  (call-with-values (lambda () (proc port))
    (lambda vals (close-port port) (apply values vals))))

(define (call-with-input-file fname proc)
  (call-with-port (open-input-file fname) proc)) 

(define (call-with-output-file fname proc)
  (call-with-port (open-output-file fname) proc)) 

(define (with-input-from-port port thunk) ; +
  (parameterize ([current-input-port port]) (thunk)))

(define (with-output-to-port port thunk) ; +
  (parameterize ([current-output-port port]) (thunk)))

(define (with-input-from-file fname thunk)
  (call-with-input-file fname (lambda (p) (with-input-from-port p thunk))))

(define (with-output-to-file fname thunk)
  (call-with-output-file fname (lambda (p) (with-output-to-port p thunk))))


;---------------------------------------------------------------------------------------------
; Input
;---------------------------------------------------------------------------------------------

; integrables:
;
; (read-char (p (current-input-port)))
; (peek-char (p (current-input-port)))
; (char-ready? (p (current-input-port)))
; (read-u8 (p (current-input-port)))
; (peek-u8 (p (current-input-port)))
; (u8-ready? (p (current-input-port)))
; (eof-object? x)
; (eof-object)

(define (read-line . ?p)
  (let ([p (if (null? ?p) (current-input-port) (car ?p))]
        [op (open-output-string)])
    (let loop ([read-nothing? #t])
      (let ([c (read-char p)])
        (cond [(or (eof-object? c) (char=? c #\newline))
               (if (and (eof-object? c) read-nothing?) 
                   c
                   (let ([s (get-output-string op)]) 
                     (close-output-port op) 
                     s))]
              [(char=? c #\return) (loop #f)]
              [else (write-char c op) (loop #f)]))))) 

(define (read-substring! str start end p)
  (let loop ([i start])
    (if (fx>=? i end) (fx- i start)
        (let ([c (read-char p)])
          (cond [(eof-object? c) (if (fx=? i start) c (fx- i start))]
                [else (string-set! str i c) (loop (fx+ i 1))])))))

(define (read-substring k p)
  (let ([str (make-string k)])
    (let ([r (read-substring! str 0 k p)])
      (if (eof-object? r) r
          (if (fx=? r k) str (substring str 0 r))))))

(define read-string!
  (case-lambda
    [(str) (read-substring! str 0 (string-length str) (current-input-port))]
    [(str p) (read-substring! str 0 (string-length str) p)]
    [(str p start) (read-substring! str start (string-length str) p)]
    [(str p start end) (read-substring! str start end p)]))

(define read-string
  (case-lambda
    [(k) (read-substring k (current-input-port))]
    [(k p) (read-substring k p)]))

(define (read-subbytevector! bvec start end p)
  (let loop ([i start])
    (if (fx>=? i end) (fx- i start)
        (let ([u8 (read-u8 p)])
          (cond [(eof-object? u8) (if (fx=? i start) u8 (fx- i start))]
                [else (bytevector-u8-set! bvec i u8) (loop (fx+ i 1))])))))

(define (read-subbytevector k p)
  (let ([bvec (make-bytevector k)])
    (let ([r (read-subbytevector! bvec 0 k p)])
      (if (eof-object? r) r
          (if (fx=? r k) bvec (subbytevector bvec 0 r))))))

(define read-bytevector!
  (case-lambda
    [(bvec) (read-subbytevector! bvec 0 (bytevector-length bvec) (current-input-port))]
    [(bvec p) (read-subbytevector! bvec 0 (bytevector-length bvec) p)]
    [(bvec p start) (read-subbytevector! bvec start (bytevector-length bvec) p)]
    [(bvec p start end) (read-subbytevector! bvec start end p)]))

(define read-bytevector
  (case-lambda
    [(k) (read-subbytevector k (current-input-port))]
    [(k p) (read-subbytevector k p)]))

(define (%read port simple?)
  (define-syntax r-error
    (syntax-rules () [(_ p msg a ...) (read-error msg a ... 'port: p)]))
  (define fold-case? (port-fold-case? port))
  (define shared '())
  (define (make-shared-ref loc) (lambda () (unbox loc)))
  (define (shared-ref? form) (procedure? form))
  (define (patch-ref! form) (if (procedure? form) (patch-ref! (form)) form))
  (define (patch-shared! form)
    (cond [(pair? form)
           (if (procedure? (car form)) 
               (set-car! form (patch-ref! (car form)))
               (patch-shared! (car form)))
           (if (procedure? (cdr form)) 
               (set-cdr! form (patch-ref! (cdr form)))
               (patch-shared! (cdr form)))]
          [(vector? form)
           (let loop ([i 0])
             (when (fx<? i (vector-length form))
               (let ([fi (vector-ref form i)])
                 (if (procedure? fi) 
                     (vector-set! form i (patch-ref! fi))
                     (patch-shared! fi)))
               (loop (fx+ i 1))))]
          [(box? form)
           (if (procedure? (unbox form))
               (set-box! form (patch-shared! (unbox form)))
               (patch-shared! (unbox form)))]))
  (define (patch-shared form) (patch-shared! form) form)           

  (define reader-token-marker #f)
  (define close-paren #f)
  (define close-bracket #f)
  (define dot #f)
  (define () ; idless
    (let ([rtm (list 'reader-token)])
          (set! reader-token-marker rtm)
          (set! close-paren (cons rtm "right parenthesis"))
          (set! close-bracket (cons rtm "right bracket"))
          (set! dot (cons rtm "\" . \""))))

  (define (reader-token? form)
    (and (pair? form) (eq? (car form) reader-token-marker)))

  (define (char-symbolic? c)
    (string-position c
      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz!$%&*/:<=>?^_~0123456789+-.@"))

  (define (char-hex-digit? c)
    (let ([scalar-value (char->integer c)])
      (or (and (>= scalar-value 48) (<= scalar-value 57))
          (and (>= scalar-value 65) (<= scalar-value 70))
          (and (>= scalar-value 97) (<= scalar-value 102)))))

  (define (char-delimiter? c)
    (or (char-whitespace? c)
        (char=? c #\)) (char=? c #\()
        (char=? c #\]) (char=? c #\[)
        (char=? c #\") (char=? c #\;)))

  (define (sub-read-carefully p)
    (let ([form (sub-read p)])
      (cond [(eof-object? form)
             (r-error p "unexpected end of file")]
            [(reader-token? form) 
             (r-error p "unexpected token:" (cdr form))]
            [else form])))

  (define (sub-read p)
    (let ([c (read-char p)])
      (cond [(eof-object? c) c]
            [(char-whitespace? c) (sub-read p)]
            [(char=? c #\() (sub-read-list c p close-paren #t)]
            [(char=? c #\)) close-paren]
            [(char=? c #\[) (sub-read-list c p close-bracket #t)]
            [(char=? c #\]) close-bracket]
            [(char=? c #\') (list 'quote (sub-read-carefully p))]
            [(char=? c #\`) (list 'quasiquote (sub-read-carefully p))]
            [(char-symbolic? c) (sub-read-number-or-symbol c p)]
            [(char=? c #\;)
             (let loop ([c (read-char p)])
               (or (eof-object? c) (char=? c #\newline)
                   (loop (read-char p))))
             (sub-read p)]
            [(char=? c #\,)
             (let ([next (peek-char p)])
               (cond [(eof-object? next)
                      (r-error p "end of file after ,")]
                     [(char=? next #\@)
                      (read-char p)
                      (list 'unquote-splicing (sub-read-carefully p))]
                     [else (list 'unquote (sub-read-carefully p))]))]
            [(char=? c #\")
             (let loop ([l '()])
               (let ([c (read-char p)])
                 (cond [(eof-object? c)
                        (r-error p "end of file within a string")]
                       [(char=? c #\\)
                        (let ([e (sub-read-strsym-char-escape p 'string)])
                          (loop (if e (cons e l) l)))]
                       [(char=? c #\") (list->string (reverse! l))]
                       [else (loop (cons c l))])))]
            [(char=? c #\|)
             (let loop ([l '()])
               (let ([c (read-char p)])
                 (cond [(eof-object? c)
                        (r-error p "end of file within a |symbol|")]
                       [(char=? c #\\)
                        (let ([e (sub-read-strsym-char-escape p 'symbol)])
                          (loop (if e (cons e l) l)))]
                       [(char=? c #\|) (string->symbol (list->string (reverse! l)))]
                       [else (loop (cons c l))])))]
            [(char=? c #\#)
             (let ([c (peek-char p)])
               (cond [(eof-object? c) (r-error p "end of file after #")]
                     [(or (char-ci=? c #\t) (char-ci=? c #\f))
                      (let ([name (sub-read-carefully p)])
                        (case name [(t true) #t] [(f false) #f]
                          [else (r-error p "unexpected name after #" name)]))]
                     [(or (char-ci=? c #\b) (char-ci=? c #\o)
                          (char-ci=? c #\d) (char-ci=? c #\x)
                          (char-ci=? c #\i) (char-ci=? c #\e))
                      (sub-read-number-or-symbol #\# p)]
                     [(char=? c #\&)
                      (read-char p)
                      (box (sub-read-carefully p))]
                     [(char=? c #\;)
                      (read-char p)
                      (sub-read-carefully p) 
                      (sub-read p)]
                     [(char=? c #\|)
                      (read-char p)
                      (let recur () ;starts right after opening #|
                        (let ([next (read-char p)])
                          (cond
                            [(eof-object? next)
                             (r-error p "end of file in #| comment")]
                            [(char=? next #\|)
                             (let ([next (peek-char p)])
                               (cond
                                 [(eof-object? next)
                                  (r-error p "end of file in #| comment")]
                                 [(char=? next #\#) (read-char p)]
                                 [else (recur)]))]
                            [(char=? next #\#)
                             (let ([next (peek-char p)])
                               (cond
                                 [(eof-object? next)
                                  (r-error p "end of file in #| comment")]
                                 [(char=? next #\|) (read-char p) (recur) (recur)]
                                 [else (recur)]))]
                            [else (recur)])))
                      (sub-read p)]
                     [(char=? c #\() ;)
                      (read-char p)
                      (list->vector (sub-read-list c p close-paren #f))]
                     [(char=? c #\u)
                      (read-char p)
                      (if (and (eq? (read-char p) #\8) (eq? (read-char p) #\())
                          (list->bytevector (sub-read-byte-list p))
                          (r-error p "invalid bytevector syntax"))]
                     [(char=? c #\\)
                      (read-char p)
                      (let ([c (peek-char p)])
                        (cond
                          [(eof-object? c)
                           (r-error p "end of file after #\\")]
                          [(char=? #\x c)
                           (read-char p)
                           (if (char-delimiter? (peek-char p))
                               c
                               (sub-read-x-char-escape p #f))]
                          [(char-alphabetic? c)
                           (let ([name (sub-read-carefully p)])
                             (if (= (string-length (symbol->string name)) 1)
                                 c
                                 (case name
                                   [(null) (integer->char #x00)]
                                   [(space) #\space]
                                   [(alarm) #\alarm]
                                   [(backspace) #\backspace]
                                   [(delete) (integer->char #x7F)] ; todo: support by SFC
                                   [(escape) (integer->char #x1B)]
                                   [(tab) #\tab]
                                   [(newline linefeed) #\newline]
                                   [(vtab) #\vtab]
                                   [(page) #\page]
                                   [(return) #\return]
                                   [else (r-error p "unknown #\\ name" name)])))]
                          [else (read-char p) c]))]
                     [(char-numeric? c)
                      (when simple? (r-error p "#N=/#N# notation is not allowed in this mode")) 
                      (let loop ([l '()])
                        (let ([c (read-char p)])
                          (cond [(eof-object? c)
                                 (r-error p "end of file within a #N notation")]
                                [(char-numeric? c)
                                 (loop (cons c l))]
                                [(char=? c #\#) 
                                 (let* ([s (list->string (reverse! l))] [n (string->number s)])
                                   (cond [(and (fixnum? n) (assq n shared)) => cdr]
                                         [else (r-error "unknown #n# reference:" s)]))]   
                                [(char=? c #\=) 
                                 (let* ([s (list->string (reverse! l))] [n (string->number s)])
                                   (cond [(not (fixnum? n)) (r-error "invalid #n= reference:" s)]
                                         [(assq n shared) (r-error "duplicate #n= tag:" n)])
                                   (let ([loc (box #f)])
                                     (set! shared (cons (cons n (make-shared-ref loc)) shared))
                                     (let ([form (sub-read-carefully p)])
                                       (cond [(shared-ref? form) (r-error "#n= has another label as target" s)]
                                             [else (set-box! loc form) form]))))]
                                [else (r-error p "invalid terminator for #N notation")])))]
                     [else (r-error p "unknown # syntax" c)]))]
            [else (r-error p "illegal character read" c)])))

  (define (sub-read-list c p close-token dot?)
    (let ([form (sub-read p)])
      (if (eq? form dot)
          (r-error p "missing car -- ( immediately followed by .") ;)
          (let recur ([form form])
            (cond [(eof-object? form)
                   (r-error p "eof inside list -- unbalanced parentheses")]
                  [(eq? form close-token) '()]
                  [(eq? form dot)
                   (if dot?
                       (let* ([last-form (sub-read-carefully p)]
                              [another-form (sub-read p)])
                         (if (eq? another-form close-token)
                             last-form
                             (r-error p "randomness after form after dot" another-form)))
                       (r-error p "dot in #(...)"))]
                  [(reader-token? form)
                   (r-error p "error inside list --" (cdr form))]
                  [else (cons form (recur (sub-read p)))])))))

  (define (sub-read-byte-list p)
    (let recur ([form (sub-read p)])
      (cond [(eof-object? form)
              (r-error p "eof inside bytevector")]
            [(eq? form close-paren) '()]
            [(reader-token? form)
              (r-error p "error inside bytevector --" (cdr form))]
            [(or (not (fixnum? form)) (fx<? form 0) (fx>? form 255))
              (r-error p "invalid byte inside bytevector --" form)]
            [else (cons form (recur (sub-read p)))])))

  (define (sub-read-strsym-char-escape p what)
    (let ([c (read-char p)])
      (if (eof-object? c)
          (r-error p "end of file within a" what))
      (cond [(or (char=? c #\\) (char=? c #\") (char=? c #\|)) c]
            [(char=? c #\a) #\alarm]
            [(char=? c #\b) #\backspace]
            [(char=? c #\t) #\tab]
            [(char=? c #\n) #\newline]
            [(char=? c #\v) #\vtab]
            [(char=? c #\f) #\page]
            [(char=? c #\r) #\return]
            [(char=? c #\x) (sub-read-x-char-escape p #t)]
            [(and (eq? what 'string) (char-whitespace? c))
             (let loop ([gotnl (char=? c #\newline)] [nc (peek-char p)])
               (cond [(or (eof-object? nc) (not (char-whitespace? nc)))
                      (if gotnl #f (r-error p "no newline in line ending escape"))]
                     [(and gotnl (char=? nc #\newline)) #f]
                     [else (read-char p) (loop (or gotnl (char=? nc #\newline)) (peek-char p))]))]
            [else (r-error p "invalid char escape in" what ': c)])))

  (define (sub-read-x-char-escape p in-string?)
    (define (rev-digits->char l)
      (if (null? l)
          (r-error p "\\x escape sequence is too short")
          (integer->char (string->fixnum (list->string (reverse! l)) 16))))
    (let loop ([c (peek-char p)] [l '()] [cc 0])
      (cond [(eof-object? c)
             (if in-string?
               (r-error p "end of file within a string")
               (rev-digits->char l))]
            [(and in-string? (char=? c #\;))
             (read-char p)
             (rev-digits->char l)]
            [(and (not in-string?) (char-delimiter? c))
             (rev-digits->char l)]
            [(not (char-hex-digit? c))
             (r-error p "unexpected char in \\x escape sequence" c)]
            [(> cc 2)
             (r-error p "\\x escape sequence is too long")]
            [else
             (read-char p)
             (loop (peek-char p) (cons c l) (+ cc 1))])))

  (define (suspect-number-or-symbol-peculiar? hash? c l s)
    (cond [(or hash? (char-numeric? c)) #f]
          [(or (string-ci=? s "+i") (string-ci=? s "-i")) #f]
          [(or (string-ci=? s "+nan.0") (string-ci=? s "-nan.0")) #f]
          [(or (string-ci=? s "+inf.0") (string-ci=? s "-inf.0")) #f]
          [(or (char=? c #\+) (char=? c #\-))
           (cond [(null? (cdr l)) #t]
                 [(char=? (cadr l) #\.) (and (pair? (cddr l)) (not (char-numeric? (caddr l))))]
                 [else (not (char-numeric? (cadr l)))])]
          [else (and (char=? c #\.) (pair? (cdr l)) (not (char-numeric? (cadr l))))]))

  (define (sub-read-number-or-symbol c p)
    (let loop ([c (peek-char p)] [l (list c)] [hash? (char=? c #\#)])
      (cond [(or (eof-object? c) (char-delimiter? c))
             (let* ([l (reverse! l)] [c (car l)] [s (list->string l)])
               (if (or hash? (char-numeric? c) 
                     (char=? c #\+) (char=? c #\-) (char=? c #\.))   
                   (cond [(string=? s ".") dot]
                         [(suspect-number-or-symbol-peculiar? hash? c l s)
                          (if fold-case? 
                              (string->symbol (string-foldcase s))
                              (string->symbol s))]
                         [(string->number s)]
                         [else (r-error p "unsupported number syntax (implementation restriction)" s)])
                   (if fold-case? 
                       (string->symbol (string-foldcase s))
                       (string->symbol s))))]
            [(char=? c #\#)
             (read-char p) 
             (loop (peek-char p) (cons c l) #t)]
            [(char-symbolic? c)
             (read-char p) 
             (loop (peek-char p) (cons c l) hash?)]
            [else (r-error p "unexpected number/symbol char" c)])))
            
  ; body of %read
  (let ([form (sub-read port)])
    (if (not (reader-token? form))
        (if (null? shared) form (patch-shared form))
        (r-error port "unexpected token:" (cdr form)))))

(define read
  (case-lambda
    [() (%read (current-input-port) #f)]
    [(p) (%read p #f)]))

(define read-simple
  (case-lambda
    [() (%read (current-input-port) #t)]
    [(p) (%read p #t)]))


;---------------------------------------------------------------------------------------------
; Output
;---------------------------------------------------------------------------------------------
 
; integrables:
;
; (write x (p (current-output-port)))
; (write-shared x (p (current-output-port)))
; (write-simple x (p (current-output-port)))
; (display x (p (current-output-port)))
; (newline (p (current-output-port)))
; (write-char c (p (current-output-port)))
; (%write-string1 s p) +
; (write-u8 u8 (p (current-output-port)))
; (%write-bytevector1 b p) +
; (flush-output-port (p (current-output-port)))

(define (write-substring from start end p)
  (do ([i start (fx+ i 1)]) [(fx>=? i end)] (write-char (string-ref from i) p)))

(define write-string
  (case-lambda
    [(str) (%write-string1 str (current-output-port))]
    [(str p) (%write-string1 str p)]
    [(str p start) (write-substring str start (string-length str) p)]
    [(str p start end) (write-substring str start end p)]))

(define (write-subbytevector from start end p)
  (do ([i start (fx+ i 1)]) [(fx>=? i end)] (write-u8 (bytevector-u8-ref from i) p)))

(define write-bytevector
  (case-lambda
    [(bvec) (%write-bytevector1 bvec (current-output-port))]
    [(bvec p) (%write-bytevector1 bvec p)]
    [(bvec p start) (write-subbytevector bvec start (bytevector-length bvec) p)]
    [(bvec p start end) (write-subbytevector bvec start end p)]))


;---------------------------------------------------------------------------------------------
; System interface
;---------------------------------------------------------------------------------------------

; integrables:
;
; (file-exists? s)
; (delete-file s)
; (rename-file sold snew) +
; (%argv-ref i) +
; (get-environment-variable s)
; (current-second)
; (current-jiffy)
; (jiffies-per-second)

(define (command-line)
  (let loop ([r '()] [i 0])
    (let ([arg (%argv-ref i)])
      (if arg 
          (loop (cons arg r) (fx+ i 1))
          (reverse! r)))))

(define (features) '(r7rs exact-closed skint skint-1.0.0))

(define (feature-available? f)
  (and (symbol? f) (memq f (features))))

;TBD:
;
;load
;exit
;emergency-exit
;get-environment-variables
