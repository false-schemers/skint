;;; Copyright (C) William D Clinger (2016).
;;;
;;; Permission is hereby granted, free of charge, to any person
;;; obtaining a copy of this software and associated documentation
;;; files (the "Software"), to deal in the Software without
;;; restriction, including without limitation the rights to use,
;;; copy, modify, merge, publish, distribute, sublicense, and/or
;;; sell copies of the Software, and to permit persons to whom the
;;; Software is furnished to do so, subject to the following
;;; conditions:
;;;
;;; The above copyright notice and this permission notice shall be
;;; included in all copies or substantial portions of the Software.
;;;
;;; THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
;;; EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
;;; OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
;;; NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
;;; HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
;;; WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
;;; FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
;;; OTHER DEALINGS IN THE SOFTWARE.

(define fl-fast-fl+* #f)

; need polyfills for:

;   fladjacent
(define fladjacent
  (lambda (x y)
    (define (loop y)
      (let* ((y3 (fl+ (fl* 0.999755859375 x) (fl* 0.000244140625 y))))
        (cond ((fl<? x y3 y)
              (loop y3))
              ((fl<? y y3 x)
              (loop y3))
              (else
              (loop2 y)))))
    (define (loop2 y)
      (let* ((y2 (fl/ (fl+ x y) 2.0))
            (y2 (if (flinfinite? y2)
                    (fl+ (fl* 0.5 x) (fl* 0.5 y))
                    y2)))
        (cond ((fl=? x y2)
              y)
              ((fl=? y y2)
              y)
              (else
              (loop2 y2)))))
    (cond ((flinfinite? x)
          (cond ((fl<? x y) (fl- fl-greatest))
                ((fl>? x y) fl-greatest)
                (else x)))
          ((fl=? x y)
          x)
          ((flzero? x)
          (if (flpositive? y)
              fl-least
              (fl- fl-least)))
          ((fl<? x y)
          (loop (flmin y
                        fl-greatest
                        (flmax (* 2.0 x)
                              (* 0.5 x)))))
          ((fl>? x y)
          (loop (flmax y
                        (fl- fl-greatest)
                        (flmin (* 2.0 x)
                              (* 0.5 x)))))
          (else    ; x or y is a NaN
          x))))

;   flcopysign
(define flcopysign
  (lambda (x y)
    (if (= (flsign-bit x) (flsign-bit y))
        x
        (fl- x))))

;   flexponent
(define (flexponent x)
  (floor (fllog2 (flabs x))))

;   flinteger-exponent
(define (flinteger-exponent x)
  (exact (flexponent x)))

;   flsign-bit
(define (flsign-bit x)
  (cond ((fl<? x 0.0) 1)
        ((eqv? x -0.0) 1)
        (else 0)))

;   flnormalized?
(define flnormalized?
  (lambda (x)
    (check-flonum! 'flnormalized? x)
    (let ((x (flabs x)))
      (and (flfinite? x)
           (fl<? (fl/ fl-greatest) x)))))

;   fldenormalized?
(define fldenormalized?
  (lambda (x)
    (check-flonum! 'fldenormalized? x)
    (let ((x (flabs x)))
      (and (flfinite? x)
           (fl<? 0.0 x)
           (fl<=? x (fl/ fl-greatest))))))

;   fl+*
;;; [esl] Implement extra-precision FMA using pairs of doubles
;;; 

; binds r(a+b) and e(a+b)
(define-syntax let-2-sum
  (syntax-rules ()
    [(_ [r e] [a b] . body)
     (let* ([r (fl+ a b)] [bb (fl- r a)] 
            [e (fl+ (fl- a (fl- r bb)) (fl- b bb))]) 
       . body)]))

; specialized version of 3-sum that just returns r(ar+ae+b)
(define-syntax r-3-sum
  (syntax-rules ()
    [(_ ar ae b)
     (let-2-sum [sr se] [ar b]
       (fl+ sr (fl+ se ae)))]))

(define-syntax fma:splitter (syntax-rules () [(_) 134217729.0]))
(define-syntax fma:threshold (syntax-rules () [(_) 6.69692879491417e299]))
(define-syntax fma:-threshold (syntax-rules () [(_) -6.69692879491417e299]))
(define-syntax fma:2^-28 (syntax-rules () [(_) 3.7252902984619140625e-9]))
(define-syntax fma:2^28 (syntax-rules () [(_) 268435456.0]))

; binds hi/lo halves of a
(define-syntax let-split
  (syntax-rules ()
    [(_ [hi lo] [a] . body)
     (if (fl<=? (fma:-threshold) a (fma:threshold))
         (let* ([t (fl* a (fma:splitter))] 
                [hi (fl- t (fl- t a))] 
                [lo (fl- a hi)])
           . body)
         (let* ([a1 (fl* a (fma:2^-28))]
                [t (fl* a1 (fma:splitter))]
                [hi1 (fl- t (fl- t a1))]
                [hi (fl* (fma:2^28) hi1)]
                [lo (fl* (fma:2^28) (fl- a1 hi1))])
           . body))]))

; binds r(a*b) and e(a*b)
(define-syntax let-2-prod
  (syntax-rules ()
    [(_ [r e] [a b] . body)
     (let ([r (fl* a b)])
       (let-split [ahi alo] [a]
         (let-split [bhi blo] [b]
           (let ([e (fl+ (fl+ (fl- (fl* ahi bhi) r)
                              (fl* ahi blo)
                              (fl* alo bhi)
                         (fl* alo blo)))])
             . body))))]))

(define (fl+* x y z)
  (if (or (flnan? x) (flnan? y) (flnan? z)) 
      +nan.0
      (let-2-prod [r e] [x y]
        (r-3-sum r e z))))

;(if (and (flfinite? x) (flfinite? y) (flfinite? z))

#|
(define fl+*
  (flop3 'fl+*
     (lambda (x y z) (fl+ (fl* x y) z))))
#;(cond (c-functions-are-available
      (fma x y z))
      ((and (flfinite? x) (flfinite? y))
      (if (flfinite? z)
          (let ((x (exact x))
                (y (exact y))
                (z (exact z)))
            (flonum (+ (* x y) z)))
          z))
      (else
      (fl+ (fl* x y) z)))
|#

(define (flposdiff x y) 
  (let ((diff (fl- x y))) (if (flnegative? diff) 0.0 diff)))

;   flexp2
(define flexp2 (flop1 'flexp2 (lambda (x) (flexpt 2.0 x))))

;   flexp-1
;;; e^x = \sum_n (z^n / (n!))
;;;
;;; FIXME: the number of terms and the constant 0.5 seem reasonable
;;; for IEEE double precision, but the number of terms might need
;;; to be increased for higher precisions.
(define flexp-1
  (flop1 'flexp-1
         (let ((coefs (cons 0.0
                            (map fl/
                                 (map factorial
                                      '(1.0 2.0 3.0 4.0 5.0
                                        6.0 7.0 8.0 9.0 10.0
                                        11.0 12.0 13.0 14.0 15.0))))))
           (lambda (x)
             (cond ((fl<? (flabs x) 0.5)    ; FIXME
                    (polynomial-at x coefs))
                   (else
                    (fl- (flexp x) 1.0)))))))

;   flcbrt
(define flcbrt
  (flop1 'flcbrt
         (lambda (x)
           (cond ((flnegative? x)
                  (fl- (flcbrt (fl- x))))
                 (else
                  (flexpt x (fl/ 3.0)))))))

;   flhypot
(define flhypot
  (flop2 'flhypot
         (lambda (x y)
           (cond ((flzero? x) (flabs y))
                 ((flzero? y) (flabs x))
                 ((or (flinfinite? x) (flinfinite? y)) +inf.0)
                 ((flnan? x) x)
                 ((flnan? y) y)
                 ((fl>? y x) (flhypot y x))
                 (else
                  (let* ((y/x (fl/ y x))
                         (root (flsqrt (fl+ 1.0 (fl* y/x y/x)))))
                    (fl* (flabs x) root)))))))

;   fllog1+
(define fllog1+
  (flop1 'fllog1+
         (lambda (x)
           (let ((u (fl+ 1.0 x)))
             (cond ((fl=? u 1.0)
                    x) ;; gets sign of zero result correct
                   ((fl=? u x)
                    (fllog u)) ;; large arguments and infinities
                   (else
                    (fl* (fllog u) (fl/ x (fl- u 1.0)))))))))

;   fllog2
(define fllog2 (flop1 'fllog2 (lambda (x) (log x 2.0))))

;   flasinh
(define flasinh
  (flop1 'flasinh
         (lambda (x)
           (cond ((or (flinfinite? x)
                      (flnan? x))
                  x)
                 ((flnegative? x)
                  (fl- (flasinh (fl- x))))
                 ((fl<? x 3.725290298461914e-9)   ;; (flexpt 2. -28.)
                  x)
                 ((fl<? x 2.)
                  ;; the naive formula is
                  ;; (log (+ x (sqrt (+ (square x) 1))))

                  ;; We want to
                  ;; 1.  Use exact operations when possible (no roundoff)
                  ;; 2.  Add or subtract things of differing magnitudes,
                  ;;     so for most arguments at most one roundoff error.

                  ;; Biggeset possible problem near x=0, so we write
                  ;; (sqrt (+ 1 (square x)))
                  ;; as
                  ;; (+ 1 (- (sqrt (+ 1 (square x))) 1))
                  ;; and then multiply the second part in
                  ;; numerator and denominator by
                  ;; (+ (sqrt (+ 1 (square x))) 1)

                  (let ((x^2 (flsquare x)))
                    (fllog1+ (fl+ x
                                  (fl/ x^2
                                       (fl+ 1.
                                            (flsqrt (fl+ 1.0 x^2))))))))
                 ((fl<? x 268435456.) ;; (flexpt 2. 28.)
                  (let ((x^2 (flsquare x)))
                    (fllog (fl+ (fl* 2. x) ;; exact
                                ;; the rest is small
                                (fl/ 1.
                                     (fl+ x
                                          (flsqrt (fl+ 1.0 x^2))))))))
                 (else
                  (fl+ (fllog x) fl-log-2))))))
;   flacosh
(define flacosh
  (flop1 'flacosh
         (lambda (x)
           (cond ((flnan? x) x)
                 ((fl<? x 1.0) +nan.0)
                 ((fl<? x 2.0)
                  ;; the naive formula is
                  ;; (log (+ x (sqrt (- (square x) 1))))

                  ;; We want to
                  ;; 1.  Use exact operations when possible (no roundoff)
                  ;; 2.  Add or subtract things of differing magnitudes,
                  ;;     so for most arguments at most one roundoff error.

                  (let ((x-1 (fl- x 1.))) ;; exact
                    (fllog1+ (fl+ x-1 ;; smaller than next expression
                                  (flsqrt (fl+ (fl* 2. x-1) ;; exact
                                               ;; relatively small
                                               (flsquare x-1)))))))
                 ((fl<? x 268435456.) ;; (flexpt 2. 28.)
                  (fllog (fl- (fl* 2. x) ;; exact
                              ;; next is smaller
                              (fl/ (fl+ x (flsqrt (fl* (fl- x 1.) ;; exact
                                                       (fl+ x 1.) ;; exact
                                                       )))))))
                 (else
                  (fl+ (fllog x) fl-log-2))))))

;   flatanh
(define flatanh
  (flop1 'flatanh
         (lambda (x)
           (cond ((fl<? x 0.)
                  (fl- (flatanh (fl- x))))
                 ;; we rewrite
                 ;; (/ (+ 1 x) (- 1 x))
                 ;; as
                 ;; (+ 1 (* 2 (/ x (- 1 x))))
                 ;; and call fllog1+ instead of fllog
                 (else
                  (fl* +0.5                                    ;; exact
                       (fllog1+ (fl* +2.0                      ;; exact
                                     (fl/ x
                                          (fl- 1.0 x)))))))))) ;; exact


;   flremquo
(define (flremquo x y)
  (check-flonum! 'flremquo x)
  (check-flonum! 'flremquo y)
  (let* ((quo (flround (fl/ x y)))
         (rem (fl- x (fl* y quo))))
    (values rem
            (exact quo))))


;   flgamma
;   flloggamma

;;; Copyright (C) William D Clinger (2016).
;;; 
;;; Permission is hereby granted, free of charge, to any person
;;; obtaining a copy of this software and associated documentation
;;; files (the "Software"), to deal in the Software without
;;; restriction, including without limitation the rights to use,
;;; copy, modify, merge, publish, distribute, sublicense, and/or
;;; sell copies of the Software, and to permit persons to whom the
;;; Software is furnished to do so, subject to the following
;;; conditions:
;;; 
;;; The above copyright notice and this permission notice shall be
;;; included in all copies or substantial portions of the Software.
;;; 
;;; THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
;;; EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
;;; OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
;;; NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
;;; HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
;;; WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
;;; FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
;;; OTHER DEALINGS IN THE SOFTWARE. 

;;; References
;;;
;;; Milton Abramowitz and Irene A Stegun [editors].
;;; Handbook of Mathematical Functions With Formulas, Graphs, and
;;; Mathematical Tables.  United States Department of Commerce.
;;; National Bureau of Standards Applied Mathematics Series, 55,
;;; June 1964.  Fifth Printing, August 1966, with corrections.
;;;
;;; R W Hamming.  Numerical Methods for Scientists and Engineers.
;;; McGraw-Hill, 1962.
;;;
;;; Donald E Knuth.  The Art of Computer Programming, Volume 2,
;;; Seminumerical Algorithms, Second Edition.  Addison-Wesley, 1981.
;;;
;;; J N Newman.  Approximations for the Bessel and Struve Functions.
;;; Mathematics of Computation, 43(168), October 1984, pages 551-556.

;;; I have deliberately avoided recent references, and have also
;;; avoided looking at any code or pseudocode for these or similar
;;; functions.

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;
;;; Gamma function
;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;;; Abramowitz and Stegun 6.1.5 ::  z! = Gamma(z+1)
;;; Abramowitz and Stegun 6.1.15 :  Gamma(z+1) = z Gamma(z)
;;;
;;; Gamma(x+2) = (x+1) Gamma(x+1) = (x+1) x Gamma(x)
;;; Gamma(x) = Gamma(x+2) / (x (x + 1))
;;;
;;; Those equations reduce the computation of Gamma(x) to the range
;;;     1.0 <= x <= 2.0
;;;
;;; The following definition is more accurate than C99 tgamma
;;; with gcc, Linux, and double precision.  The alarmingly large
;;; absolute errors near 16.0 and -2.0e-16 are small relative
;;; errors.  At x=16.0, tgamma returns a non-integer result,
;;; but flgamma returns the correct integer result.

(define (flgamma x)
  (check-flonum! 'flgamma x)
  (cond ((fl>=? x flgamma:upper-cutoff)
         +inf.0)
        ((fl<=? x flgamma:lower-cutoff)
         (cond ((= x -inf.0)
                +nan.0)
               ((flinteger? x)    ; pole error
                +nan.0)
               ((flodd? (fltruncate x)) 0.0)
               (else -0.0)))
        (else (Gamma x))))

(define (Gamma x)
  (cond ((fl>? x 2.0)
         (let ((x (fl- x 2.0)))
           (fl* x (fl+ x 1.0) (Gamma x))))
        ((fl=? x 2.0)
         1.0)
        ((fl>? x 1.0)
         (let ((x (fl- x 1.0)))
           (fl* x (Gamma x))))
        ((fl=? x 1.0)
         1.0)
        ((fl=? x 0.0)
         +inf.0)
        ((fl<? x 0.0)
         (if (flinteger? x)    ; pole error
             +nan.0
             (fl/ (Gamma (fl+ x 2.0)) x (fl+ x 1.0))))
        (else
         (fl/ (polynomial-at x gamma-coefs)))))

;;; Series expansion for 1/Gamma(x), from Abramowitz and Stegun 6.1.34

(define gamma-coefs
  '(0.0
    1.0
    +0.5772156649015329
    -0.6558780715202538
    -0.0420026350340952
    +0.1665386113822915 ; x^5
    -0.0421977345555443
    -0.0096219715278770
    +0.0072189432466630
    -0.0011651675918591
    -0.0002152416741149 ; x^10
    +0.0001280502823882
    -0.0000201348547807
    -0.0000012504934821
    +0.0000011330272320
    -0.0000002056338417 ; x^15
    +0.0000000061160950
    +0.0000000050020075
    -0.0000000011812746
    +0.0000000001043427
    +0.0000000000077823 ; x^20
    -0.0000000000036968
    +0.0000000000005100
    -0.0000000000000206
    -0.0000000000000054
    +0.0000000000000014 ; x^25
    +0.0000000000000001
    ))

;;; If x >= flgamma:upper-cutoff, then (Gamma x) is +inf.0

(define flgamma:upper-cutoff
  (do ((x 2.0 (+ x 1.0)))
      ((flinfinite? (Gamma x))
       x)))

;;; If x <= flgamma:lower-cutoff, then (Gamma x) is a zero or NaN

(define flgamma:lower-cutoff
  (do ((x -2.0 (- x 1.0)))
      ((flzero?
        (Gamma (fladjacent x 0.0)))
       x)))

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;
;;; log (Gamma (x))
;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;;; Returns two values:
;;;     log ( |Gamma(x)| )
;;;     sgn (Gamma(x))
;;;
;;; The draft spec is unclear concerning sgn (Gamma(x)),
;;; but (flsgn x) returns (flcopysign 1.0 x) so I'm assuming
;;; sgn (Gamma(x)) is +1.0 for positive and -1.0 for negative.
;;; For real x, Gamma(x) is never actually zero, but it is
;;; undefined if x is zero or a negative integer.

;;; For small absolute values, this is trivial.
;;; Abramowitz and Stegun give several asymptotic formulas
;;; that might be good enough for large values of x.
;;;
;;; 6.1.41 :  As x goes to positive infinity, log (Gamma (x)) goes to
;;;
;;;     (x - 1/2) log x - x + 1/2 log (2 pi)
;;;         + 1/(12x) - 1/(360x^3) + 1/(1260x^5) - 1/(1680x^7) + ...
;;;
;;; 6.1.48 states a continued fraction.

(define (flloggamma x)
  (check-flonum! 'flloggamma x)
  (cond ((flinfinite? x)
         (if (flpositive? x)
             (values x 1.0)
             (values +inf.0 +nan.0)))
        ((fl>=? x flloggamma:upper-threshold)
         (values (eqn6.1.48 x) 1.0))
        ((fl>? x 0.0)
         (let ((g (flgamma x)))
           (values (log g) 1.0)))
        (else
         (let ((g (flgamma x)))
           (values (log (flabs g))
                   (flcopysign 1.0 g))))))

;;; This doesn't seem to be as accurate as the continued fraction
;;; of equation 6.1.48, so it's commented out for now.

#;(define (eqn6.1.41 x)
  (let* ((x^2 (fl* x x))
         (x^3 (fl* x x^2))
         (x^5 (fl* x^2 x^3))
         (x^7 (fl* x^2 x^5)))
    (fl+ (fl* (fl- x 0.5) (fllog x))
         (fl- x)
         (fl* 0.5 (fllog fl-2pi))
         (fl/ (fl* 12.0 x))
         (fl/ (fl* 360.0 x^3))
         (fl/ (fl* 1260.0 x^5))
         (fl/ (fl* 1680.0 x^7)))))

;[esl] replaced #i ratios with fp constants
(define (eqn6.1.48 x)
  (let ((+ fl+)
        (/ fl/))
    (+ (fl* (fl- x 0.5) (fllog x))
       (fl- x)
       (fl* 0.5 (fllog fl-2pi))
       (/ 0.08333333333333333 ;#i1/12
          (+ x
             (/ 0.03333333333333333 ;#i1/30
                (+ x
                   (/ 0.2523809523809524 ;#i53/210
                      (+ x
                         (/ 0.5256064690026954 ;#i195/371
                            (+ x
                               (/ 1.0115230681268417 ;#i22999/22737
                                  (+ x
                                     (/ 1.5174736491532874 ;#i29944523/19733142
                                        (+ x
                                           (/ 2.2694889742049598 ;#i109535241009/48264275462
                                              (+ x)))))))))))))))))

;;; With IEEE double precision, eqn6.1.48 is at least as accurate as
;;; (log (flgamma x)) starting around x = 20.0

(define flloggamma:upper-threshold 20.0)


;   flerf
;   flerfc

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;
;;; Error functions
;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(define (flerf x)
  (check-flonum! 'flerf x)
  (cond ((flnegative? x)
         (fl- (flerf (fl- x))))
        ((fl<? x 2.0)
         (eqn7.1.6 x))
        ((fl<? x +inf.0)
         (- 1.0 (eqn7.1.14 x)))
        ((fl=? x +inf.0)
         1.0)
        (else x)))

(define (flerfc x)
  (check-flonum! 'flerfc x)
  (cond ((flnegative? x)
         (fl- 2.0 (flerfc (fl- x))))
        ((fl<? x 2.0)
         (eqn7.1.2 x))
        ((fl<? x +inf.0)
         (eqn7.1.14 x))
        ((fl=? x +inf.0)
         0.0)
        (else x)))

;;; Equation numbers are from Abramowitz and Stegun.

;;; If the step size is small enough for good accuracy,
;;; the integration is pretty slow.
;;;
;;; FIXME: not used at present, so I've commented it out.

#;(define (eqn7.1.1 x)
  (fl* fl-2/sqrt-pi
       (definite-integral 0.0 x (lambda (t) (flexp (fl- (flsquare t)))))))

(define (eqn7.1.2 x)
  (fl- 1.0 (flerf x)))

;;; Equation 7.1.6 :
;;;
;;;     erf x = (2 / sqrt(pi))
;;;             exp(-x^2)
;;;             \sum_{n=0}^\infty (2^n / (1 * 3 * ... * (2n+1))) x^(2n+1)
;;;
;;;           = (2 / sqrt(pi))
;;;             exp(-x^2)
;;;             x
;;;             \sum_{n=0}^\infty (2^n / (1 * 3 * ... * (2n+1))) (x^2)^n

(define (eqn7.1.6 x)
  (let ((x^2 (flsquare x)))
    (fl* fl-2/sqrt-pi
         (flexp (fl- x^2))
         x
         (polynomial-at x^2 eqn7.1.6-coefficients))))

(define eqn7.1.6-coefficients
  (let ()
    (define (loop n p)
      (if (> n 32) ; FIXME
          '()
          (let ((p (fl* p (inexact (+ (* 2 n) 1)))))
            (cons (fl/ (inexact (expt 2.0 n)) p)
                  (loop (+ n 1) p)))))
    (loop 0 1.0)))

;;; Equation 7.1.14 :
;;;
;;;     2 e^(x^2) \int_x^\infty e^(-t^2) dt
;;;   = 1 / (x + (1/2 / (x + (1 / (x + (3/2 / (x + (2 / (x + ...
;;;   = x (1/x) (1 / (x + (1/2 / (x + (1 / (x + (3/2 / (x + (2 / (x + ...
;;;   = x (1 / (x (x + (1/2 / (x + (1 / (x + (3/2 / (x + (2 / (x + ...
;;;   = x (1 / (x^2 + (1/2 / (1 + (1 / (x^2 + (3/2 / (1 + (2 / (x^2 + ...
;;;
;;;     erfc(x) = (2 / sqrt(pi)) \int_x^\infty e^(-t^2) dt
;;; so
;;;     erfc(x) = (1 / (sqrt(pi) e^(x^2)))
;;;                   times the continued fraction

(define (eqn7.1.14 x)
  (define (continued-fraction x)
    (fl/ 1.0 (fl+ x (loop 1 0.5))))
  (define (loop k frac)
    (if (> k 70) ; FIXME
        1.0
        (fl/ frac (fl+ x (loop (+ k 1) (fl+ frac 0.5))))))
  (fl/ (continued-fraction x)
       (fl* (flsqrt fl-pi)
            (flexp (flsquare x)))))
