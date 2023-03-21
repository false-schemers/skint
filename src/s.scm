
;---------------------------------------------------------------------------------------------
; Scheme library functions
;---------------------------------------------------------------------------------------------

;---------------------------------------------------------------------------------------------
; helpers
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
; Derived expression types
;---------------------------------------------------------------------------------------------

;cond
;case
;and
;or
;when
;unless
;cond-expand

;let  -- including named let
;let*
;letrec
;letrec*
;let-values
;let*-values

;do
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
; (fixnum? x)
; (fxzero? x)
; (fxpositive? x)
; (fxnegative? x)
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
; (fixnum->flonum x)


;---------------------------------------------------------------------------------------------
; Inexact floating-point numbers (flonums)
;---------------------------------------------------------------------------------------------

; integrables:
;
; (flonum? x)
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
; (fl<? x y z ...)
; (fl<=? x y z ...)
; (fl>? x y z ...)
; (fl>=? x y z ...)
; (fl=? x y z ...)
; (fl!=? x y)
; (flmin x y)
; (flmax x y)
; (flonum->fixnum x)


;---------------------------------------------------------------------------------------------
; Numbers (fixnums or flonums)
;---------------------------------------------------------------------------------------------

; integrables:
;
; (number? x)
; (integer? x)
; (complex? x) == number? what about inf and nan?
; (real? x) == number? what about inf and nan?
; (rational? x) == number? what about inf and nan?
; (exact-integer? x) == fixnum?

; (exact? x)
; (inexact? x)

; (finite? x)
; (infinite? x)
; (nan? x)
; (zero? x)
; (positive? x)
; (negative? x)
; (even? x)
; (odd? x)

; (+ x ...)
; (* x ...)
; (- x y ...)
; (/ x y ...)

; (< x y z ...)
; (<= x y z ...)
; (> x y z ...)
; (>= x y z ...)
; (= x y z ...)

; (abs x)

; (truncate-quotient x y)
; (truncate-remainder x y)
; (quotient x y) == truncate-quotient
; (remainder x y) == truncate-remainder

; (floor-quotient x y)
; (floor-remainder x y)
; (modulo x y) = floor-remainder

(define (floor/ x y)
  (%sdmv (floor-quotient x y) (floor-remainder x y)))

(define (truncate/ x y)
  (%sdmv (truncate-quotient x y) (truncate-remainder x y)))


;---------------------------------------------------------------------------------------------
; Booleans
;---------------------------------------------------------------------------------------------

; integrables:
;
; (boolean? x)
; (not x)


;---------------------------------------------------------------------------------------------
; Characters
;---------------------------------------------------------------------------------------------

; (char? x)

; (char-cmp c1 c2)
; (char=? c1 c2 c ...)
; (char<? c1 c2 c ...)
; (char>? c1 c2 c ...)
; (char<=? c1 c2 c ...)
; (char>=? c1 c2 c ...)

; (char-ci-cmp c1 c2)
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

; (char->integer c) 
; (integer->char n)

;char-foldcase
;digit-value


;---------------------------------------------------------------------------------------------
; Symbols
;---------------------------------------------------------------------------------------------

; (symbol? x)
; (symbol->string y)
; (string->symbol s)


;---------------------------------------------------------------------------------------------
; Null and Pairs
;---------------------------------------------------------------------------------------------

; integrables:
;
; (null? x)
; (pair? x)
; (car x)
; (set-car! x v)
; (cdr x)
; (set-cdr! x v)
; (caar x) ... (cddddr x)
; (cons x y)


;---------------------------------------------------------------------------------------------
; Lists
;---------------------------------------------------------------------------------------------

; (list? x)

(define (%make-list n i)
  (let loop ([n (%ckk n)] [l '()])
    (if (fx<=? n 0) l (loop (fx- n 1) (cons i l))))) 

(define-syntax make-list
  (syntax-rules ()
    [(_ n) (%make-list n #f)]  ; #f > (void)
    [(_ n i) (%make-list n i)]
    [(_ . args) (%residual-make-list . args)]
    [_ %residual-make-list]))

; (list x ...)
; (length l)
; (list-ref l i)
; (list-set! l i x)
; (list-cat l1 l2)

(define-syntax append
  (syntax-rules ()
    [(_) '()] [(_ x) x]
    [(_ x y) (list-cat x y)]
    [(_ x y z ...) (list-cat x (append y z ...))]
    [_ %residual-append]))

; (memq v l)
; (memv v l)  ; TODO: make sure memv checks list
; (meme v l)  ; TODO: make sure meme checks list

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

; (assq v y)
; (assv v y) ; TODO: make sure assv checks list
; (asse v y) ; TODO: make sure asse checks list

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

(define-inline (list-copy x) %residual-list-copy (%lcat x '()))

; (list-tail l i)
; (last-pair l)
; (reverse l)
; (reverse! l)

(define-syntax list*
  (syntax-rules ()
    [(_ x) x]
    [(_ x y) (cons x y)]
    [(_ x y z ...) (cons x (list* y z ...))]
    [(_ . args) (%residual-list* . args)]
    [_ %residual-list*]))

(define-syntax cons* list*)


;---------------------------------------------------------------------------------------------
; Vectors
;---------------------------------------------------------------------------------------------

; (vector? x)
; (vector x ...)
; (make-vector n (i #f))
; (vector-length v)
; (vector-ref v i)
; (vector-set! v i x)
; (list->vector x)
; (vector-cat v1 v2)

(define (subvector->list vec start end)
  (let loop ([i (fx- end 1)] [l '()])
    (if (fx<? i start) l (loop (fx- i 1) (cons (vector-ref vec i) l)))))

(define %vector->list
  (case-lambda
     [(vec) (%vtol vec)]
     [(vec start) (subvector->list vec start (vector-length vec))]
     [(vec start end) (subvector->list vec start end)]))

(define-syntax vector->list
  (syntax-rules ()
    [(_ x) (%vtol x)]
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
; Strings
;---------------------------------------------------------------------------------------------

; (string? x)
; (string c ...)
; (make-string n (i #\space))
; (string-length s)
; (string-ref x i)
; (string-set! x i v)
; (list->string l)
; (string-cat s1 s2)
; (substring s from to)

(define (substring->list str start end)
  (let loop ([i (fx- end 1)] [l '()])
    (if (fx<? i start) l (loop (fx- i 1) (cons (string-ref str i) l)))))

(define %string->list
  (case-lambda
     [(str) (%stol str)]
     [(str start) (substring->list str start (string-length str))]
     [(str start end) (substring->list str start end)]))

(define-syntax string->list
  (syntax-rules ()
    [(_ x) (%stol x)]
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

; (string-cmp s1 s2)
; (string=? s1 s2 s ...)
; (string<? s1 s2 s ...)
; (string>? s1 s2 s ...)
; (string<=? s1 s2 s ...)
; (string>=? s1 s2 s ...)

; (string-ci-cmp s1 s2)
; (string-ci=? s1 s2 s ...)
; (string-ci<? s1 s2 s ...)
; (string-ci>? s1 s2 s ...)
; (string-ci<=? s1 s2 s ...)
; (string-ci>=? s1 s2 s ...)

;string-upcase
;string-downcase
;string-foldcase


;---------------------------------------------------------------------------------------------
; Conversions
;---------------------------------------------------------------------------------------------

; (fixnum->string x (r 10))
; (string->fixnum s (r 10))
; (flonum->string x)
; (string->flonum s)
; (number->string x (r 10))
; (string->number s (r 10))


;---------------------------------------------------------------------------------------------
; Control features
;---------------------------------------------------------------------------------------------

; (procedure? x)

(define-syntax apply
  (syntax-rules ()
    [(_ p l) (%appl p l)]
    [(_ p a b ... l) (%appl p (list* a b ... l))]
    [(_ . args) (%residual-apply . args)]
    [_ %residual-apply]))

(define-inline (call/cc f) %residual-call/cc (letcc k (f k)))

(define-syntax call-with-current-continuation call/cc)

(define-syntax values %sdmv)

(define-syntax call-with-values %cwmv)

(define-syntax map
  (syntax-rules ()
    [(_ fun lst)
     (let ([f fun]) 
       (let loop ([l lst]) 
         (if (pair? l) (cons (f (car l)) (loop (cdr l))) '())))]
    [(_ . args) (%residual-map . args)]
    [_ %residual-map])) 

(define-syntax for-each
  (syntax-rules ()
    [(_ fun lst)
     (let ([f fun]) 
       (let loop ([l lst]) 
         (if (pair? l) (begin (f (car l)) (loop (cdr l))))))]
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


; (read-char (p (current-input-port)))
; (peek-char (p (current-input-port)))
; (char-ready? (p (current-input-port)))

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

; (eof-object? x)
; (eof-object)

;read
;read-string
;read-u8
;peek-u8
;u8-ready?
;read-bytevector
;read-bytevector!



;---------------------------------------------------------------------------------------------
; Output
;---------------------------------------------------------------------------------------------

 
; (write-char c (p (current-output-port)))
; (write-string s (p (current-output-port)))
; (display x (p (current-output-port)))
; (write x (p (current-output-port)))
; (newline (p (current-output-port)))
; (write-shared x (p (current-output-port)))
; (write-simple x (p (current-output-port)))

;flush-output-port


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

(define %residual-make-list (unary-binary-adaptor make-list))

(define-syntax minmax-reducer
  (syntax-rules ()
    [(_ f)
     (lambda (x . args)
       (let loop ([x x] [args args])
         (if (null? args)
             x
             (loop (f x (car args)) (cdr args)))))]))

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
