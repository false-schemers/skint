
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

;cond-expand

;delay
;delay-force


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
; (fxmodquo x y) 
; (fxmodulo x y) 
; (fxeucquo x y) a.k.a. euclidean-quotient, R6RS div
; (fxeucrem x y) a.k.a. euclidean-remainder, R6RS mod
; (fxneg x)
; (fxabs x)
; (fx<? x y z ...)
; (fx<=? x y z ...)
; (fx>? x y z ...)
; (fx>=? x y z ...)
; (fx=? x y z ...)
; (fx!=? x y)
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
; (fl!=? x y)
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
; (rational? x) == number? what about inf and nan?
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
; (memq v l)
; (memv v l)  ; TODO: make sure memv checks list
; (meme v l)  + 2-arg member; TODO: make sure meme checks list
; (assq v y)   
; (assv v y) ; TODO: make sure assv checks list
; (asse v y) + 2-arg assoc; TODO: make sure asse checks list
; (list-tail l i)
; (last-pair l)
; (reverse l)
; (reverse! l) +

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

;list-copy


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

;string-upcase
;string-downcase
;string-foldcase


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

; (bytevector? x)
; (make-bytevector n (u8 0))
; (bytevector u8 ...)
; (bytevector-length b)
; (bytevector-u8-ref b i)
; (bytevector-u8-set! b i u8)
; (list->bytevector l)
; (subbytevector b from to)
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

;utf8->string
;string->utf8


;---------------------------------------------------------------------------------------------
; Control features
;---------------------------------------------------------------------------------------------

; integrables:
;
; (procedure? x)
; (values x ...)
; (call-with-values thunk receiver)

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

(define (%call/cc p) (letcc k (p k)))

(define-syntax call/cc
  (syntax-rules ()
    [(_ p) (letcc k (p k))]
    [(_ . args) (%call/cc . args)]
    [_ %call/cc])) 

(define-syntax call-with-current-continuation call/cc)

;dynamic-wind

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

;TBD:
;
;with-exception-handler
;raise
;raise-continuable
;error-object?
;error-object-message
;error-object-irritants
;read-error?
;file-error?

(define (error msg . args) 
  (%panic msg args)) ; should work for now

(define (read-error msg . args)
  (%panic msg args)) ; should work for now


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
; (current-input-port)
; (current-output-port)
; (current-error-port)
; (open-output-string)
; (open-input-file s)
; (open-output-file x)
; (open-input-string x)
; (close-input-port x)
; (close-output-port x)
; (get-output-string x)

(define (port? x) (or (input-port? x) (output-port? x)))

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

;with-input-from-file  -- requires parameterize
;with-output-to-file   -- requires parameterize
;open-binary-input-file
;open-binary-output-file
;open-input-bytevector
;open-output-bytevector
;get-output-bytevector


;---------------------------------------------------------------------------------------------
; Input
;---------------------------------------------------------------------------------------------

; integrables:
;
; (read-char (p (current-input-port)))
; (peek-char (p (current-input-port)))
; (char-ready? (p (current-input-port)))
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

;read

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

;read-u8
;peek-u8
;u8-ready?
;read-bytevector
;read-bytevector!


;---------------------------------------------------------------------------------------------
; Output
;---------------------------------------------------------------------------------------------
 
; integrables:
;
; (write-char c (p (current-output-port)))
; (write-string s (p (current-output-port)))
; (display x (p (current-output-port)))
; (write x (p (current-output-port)))
; (newline (p (current-output-port)))
; (write-shared x (p (current-output-port)))
; (write-simple x (p (current-output-port)))
; (flush-output-port p)


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

