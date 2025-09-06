; need polyfill for:
;   flfirst-bessel
;   flsecond-bessel


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
;;; Bessel functions
;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;;; FIXME: This isn't as accurate as it should be, it's hard to test
;;; because it combines so many different algorithms and intervals,
;;; and it underflows to zero too soon.
;;;
;;; FIXME: To reduce discontinuities at the boundaries, results
;;; near boundaries should be computed as weighted averages of
;;; the results returned by algorithms used on the two sides of
;;; the boundary.
;;;
;;; FIXME: Several numerical constants, flagged by FIXME comments,
;;; seem to work for Larceny's double precision but might need to
;;; be changed for other precisions.  They are unlikely to be
;;; optimal even for double precision.

(define (flfirst-bessel n x)
  (define (nan-protected y)
    (if (flfinite? y) y 0.0))
  (check-flonum! 'flfirst-bessel x)
  (cond ;(c-functions-are-available
        ; (jn n x))
        ((< n 0)
         (let ((result (flfirst-bessel (- n) x)))
           (if (even? n) result (- result))))

        ((< x 0)
         (let ((result (flfirst-bessel n (- x))))
           (if (even? n) result (- result))))

        ((= x +inf.0)
         0.0)

        (else
         (case n
          ((0)    (cond ((fl<? x 4.5)     ; FIXME
                         (eqn9.1.10 n x))
                        ((fl<? x 93.0)    ; FIXME
                         (eqn9.1.18 n x))
                        (else
                         (eqn9.2.5 n x))))
          ((1)    (cond ((fl<? x 11.0)    ; FIXME
                         (eqn9.1.10-fast n x))
                        ((fl<? x 300.0)   ; FIXME
                         (eqn9.1.75 n x))
                        ((fl<? x 1e12)    ; FIXME
                         (eqn9.2.5 n x))
                        (else
                         (eqn9.2.1 n x))))
          ((2)    (cond ((fl<? x 10.0)    ; FIXME
                         (eqn9.1.10-fast n x))
                        ((fl<? x 1e19)    ; FIXME
                         (eqn9.1.27-first-bessel n x))
                        (else
                         ;; FIXME
                         0.0)))
          ((3)    (cond ((fl<? x 10.0)    ; FIXME
                         (eqn9.1.10-fast n x))
                        ((fl<? x 1e6)     ; FIXME
                         (eqn9.1.27-first-bessel n x))
                        (else
                         (nan-protected (eqn9.2.5 n x)))))
          (else   (cond ((fl<? x 12.0)    ; FIXME
                         (nan-protected (eqn9.1.10-fast n x)))
                        ((fl<? x 150.0)   ; FIXME
                         (nan-protected (if (fl>? (inexact n) x)
                                            (method9.12ex1 n x)
                                            (eqn9.1.75 n x))))
                        ((fl<? x 1e18)    ; FIXME
                         (nan-protected (eqn9.1.27-first-bessel n x)))
                        (else
                         ;; FIXME
                         0.0)))))))

(define (flsecond-bessel n x)
  (check-flonum! 'flsecond-bessel x)
  (cond ;(c-functions-are-available
        ; (yn n x))
        ((< n 0)
         (let ((result (flsecond-bessel (- n) x)))
           (if (even? n) result (- result))))

        ((fl<? x 0.0)
         +nan.0)

        ((fl=? x 0.0)
         -inf.0)

        ((fl=? x +inf.0)
         0.0)

        (else
         (case n
          ((0)    (cond ((fl<? x 14.5)        ; FIXME
                         (eqn9.1.13 0 x))
                        (else
                         (eqn9.2.6 0 x))))
          ((1)    (cond ((fl<? x 1e12)        ; FIXME
                         (eqn9.1.16 n x))
                        (else
                         (eqn9.2.6 n x))))
          ((2 3)  (cond (else
                         (eqn9.1.27-second-bessel n x))))
          (else   (let ((ynx (eqn9.1.27-second-bessel n x)))
                    (if (flnan? ynx)
                        -inf.0
                        ynx)))))))

;;; For multiples of 1/16:
;;;
;;; For n = 0, this agrees with C99 jn for 0 <= x <= 1.5
;;; and disagrees by no more than 1 bit for 0 <= x <= 2.0.
;;; For n = 1, this disagrees by no more than 1 bit for 0 <= x <= 2.5.
;;;
;;;     n    0 <= x <= xmax    bits
;;;
;;;     0    0 <= x <= 1.5      0
;;;     0    0 <= x <= 2.0      1
;;;     1    0 <= x <= 2.5      1
;;;     2    0 <= x <= 3.0      2
;;;     3    0 <= x <= 2.5      4
;;;     4    0 <= x <= 2.5      4
;;;     5    0 <= x <= 3.0      3
;;;     6    0 <= x <= 4.0      3
;;;     7    0 <= x <= 6.5      4
;;;     8    0 <= x <= 2.0      3
;;;     9    0 <= x <= 1.5      4
;;;    10    0 <= x <= 4.5      4
;;;    20    0 <= x <= 3.5      6
;;;    20    0 <= x <= 6.0      8
;;;    30    0 <= x <= 1.0      6
;;;    40    0 <= x <= 1.5      6
;;;    50    0 <= x <= 1.0      6


;;; It should become more accurate for larger n but less accurate for
;;; larger x.  Should be okay if n > x.

(define (eqn9.1.10 n x)
  (fl* (inexact (expt (* 0.5 x) n))
       (polynomial-at (flsquare x)
                      (cond ((= n 0)
                             eqn9.1.10-coefficients-0)
                            ((= n 1)
                             eqn9.1.10-coefficients-1)
                            (else
                             (eqn9.1.10-coefficients n))))))

(define (eqn9.1.10-coefficients n)
  (define (loop k prev)
    (if (flzero? (inexact prev))
        '()
        (let ((c (/ (* -0.25 prev) k (+ n k)))) ; -1/4
          (cons c (loop (+ k 1) c)))))
  (let ((c (/ (fact n))))
    (map inexact (cons c (loop 1 c)))))

(define eqn9.1.10-coefficients-0
  (eqn9.1.10-coefficients 0))

(define eqn9.1.10-coefficients-1
  (eqn9.1.10-coefficients 1))

;;; This is faster than using exact arithmetic to compute coefficients
;;; at call time, and it seems to be about as accurate.

(define (eqn9.1.10-fast n x)
  (let* ((y (fl* 0.5 x))
         (y2 (fl- (fl* y y)))
         (bound (+ 25.0 (inexact n))))
    (define (loop k n+k)
      (if (fl>? n+k bound)
          1.0
          (fl+ 1.0
               (fl* (fl/ y2 (fl* k n+k))
                    (loop (fl+ 1.0 k) (fl+ 1.0 n+k))))))
    (fl/ (fl* (inexact (expt y n))
              (loop 1.0 (fl+ 1.0 (inexact n))))
         (factorial (inexact n)))))

;;; Equation 9.1.11 :
;;;
;;;     Y_n(x) = - (1 / (pi (x/2)^n))
;;;                  \sum_{k=0}^{n-1} ((n - k - 1)!/(k!)) (x^2/4)^k
;;;              + (2/pi) log (x/2) J_n(x)
;;;              - ((x/2)^n / pi)
;;;                  \sum_{k=0}^\infty ((psi(k+1) + psi(n+k+1)) / (k! (n+k)!))
;;;                                    (x^/4)^k
;;; where
;;;
;;;     psi (1) = - gamma
;;;     psi (n) = - gamma + \sum_{k=1}^{n-1} (1/k)

;;; Equation 9.1.13 is the special case for Y_0(x) :
;;;
;;;     Y_0(x) = (2/pi) (log (x/2) + gamma) J_0(x)
;;;            + (2/pi) ((x^2/4)^1 / (1!)^2
;;;                          - (1 + 1/2) (x^2/4)^2 / (2!)^2
;;;                          + (1 + 1/2 + 1/3) (x^2/4)^3 / (3!)^2
;;;                          - ...)

(define (eqn9.1.13 n x)
  (if (not (= n 0)) (error "eqn9.1.13 requires n=0"))
  (fl* 2.0
       fl-1/pi
       (fl+ (fl* (fl+ (fllog (fl/ x 2.0)) fl-euler)
                 (flfirst-bessel 0 x))
            (polynomial-at (fl* 0.25 x x)
                           eqn9.1.13-coefficients))))

(define eqn9.1.13-coefficients
  (map (lambda (k)
         (cond ((= k 0) 0.0)
               ((= k 1) 1.0)
               (else
                ;; (1 + 1/2 + 1/3 + ... + 1/k) / (k!)^2
                (let ((c (/ (apply + (map / (cdr (iota (+ k 1)))))
                            (let ((k! (fact k)))
                              (* k! k!)))))
                  (inexact (if (even? k) (- c) c))))))
       (iota 25))) ; FIXME

;;; Equation 9.1.16 :
;;;
;;;     J_{n+1}(x) Y_n(x) - J_n(x) Y_{n+1}(x) = 2 / (pi x)
;;; so
;;;     Y_{n+1}(x) = (J_{n+1}(x) Y_n(x) - (2 / (pi x))) / J_n(x)

(define (eqn9.1.16 n+1 x)
  (if (= 0 n+1)
      (flsecond-bessel 0 x)
      (let ((n (- n+1 1)))
        (fl/ (fl- (fl* (flfirst-bessel n+1 x) (flsecond-bessel n x))
                  (fl/ 2.0 (fl* fl-pi x)))
             (flfirst-bessel n x)))))

;;; Equation 9.1.18 :
;;;
;;;     J_0(x) = (1 / \pi) \int_0^\pi cos (x sin \theta) d\theta
;;;            = (1 / \pi) \int_0^\pi cos (x cos \theta) d\theta

(define (eqn9.1.18 n x)
  (if (> n 0)
      (flfirst-bessel n x)
      (fl* fl-1/pi
           (definite-integral 0.0
                              fl-pi
                              (lambda (theta)
                                (flcos (fl* x (flsin theta))))
                              128))))

;;; Equation 9.1.27 says
;;;
;;; J_{n-1}(x) + J_{n+1}(x) = (2n/x) J_n(x)
;;;
;;; J_{n+1}(x) = (2n/x) J_n(x) - J_{n-1}(x)
;;;
;;; J_{n-1}(x) = (2n/x) J_n(x) - J_{n+1}(x)
;;;
;;; Y_{n-1}(x) + Y_{n+1}(x) = (2n/x) Y_n(x)
;;;
;;; Y_{n+1}(x) = (2n/x) Y_n(x) - Y_{n-1}(x)
;;;
;;; Y_{n-1}(x) = (2n/x) Y_n(x) - Y_{n+1}(x)
;;;
;;; This has too much roundoff error if n > x or if x and n have
;;; the same magnitude.

(define (eqn9.1.27-first-bessel n x)
  (eqn9.1.27 flfirst-bessel n x))

(define (eqn9.1.27-second-bessel n x)
  (eqn9.1.27 flsecond-bessel n x))

(define (eqn9.1.27 f n0 x)
  (define (loop n jn jn-1)
    (cond ((= n n0)
           jn)
          (else
           (loop (+ n 1)
                 (fl- (fl* (fl/ (inexact (+ n n)) x) jn)
                      jn-1)
                 jn))))
  (if (<= n0 1)
      (f n0 x)
      (loop 1 (f 1 x) (f 0 x))))

;;; For x < n, Abramowitz and Stegun 9.12 Example 1 suggests this method:
;;;
;;;     1.  Choose odd N large enough so J_N(x) is essentially zero.
;;;     2.  Choose an arbitrary trial value, say 1.0, for J_{N-1}(x).
;;;     3.  Use equation 9.1.27 to estimate the relative values
;;;         of J_{N-2}(x), J_{N-3}(x), ...
;;;     4.  Normalize using equation 9.1.46 :
;;;
;;;             1 = J_0(x) + 2 J_2(x) + 2 J_4(x) + 2 J_6(x) + ...

(define (method9.12ex1 n0 x)
  (define (loop n jn jn+1 jn0 sumEvens)
    (if (= n 0)
        (fl/ jn0 (+ jn sumEvens sumEvens))
        (let ((jn-1 (fl- (fl/ (fl* 2.0 (inexact n) jn) x) jn+1)))
          (loop (- n 1)
                jn-1
                jn
                (if (= n n0) jn jn0)
                (if (even? n) (fl+ jn sumEvens) sumEvens)))))
  (let* ((n (min 200 (+ n0 20))) ; FIXME
         (jn+1 (fl/ x (fl* 2.0 (inexact n))))
         (jn 1.0))
    (loop (- n 1) jn jn+1 0.0 0.0)))

;;; Equation 9.1.75 states an equality between J_n(x)/J_{n-1}(x)
;;; and a continued fraction.
;;;
;;; Precondition: |x| > 0
;;;
;;; This works very well provided (flfirst-bessel x 0) is accurate
;;; and x is small enough for it to run in reasonable time.

(define (eqn9.1.75 n x)
  (define k (max 10 (* 2 (exact (flceiling x)))))
  (define (loop x2 m i)
    (if (> i k)
        (fl/ 1.0 (fl* m x2))
        (fl/ 1.0
             (fl- (fl* m x2)
                  (loop x2 (+ m 1.0) (+ i 1))))))
  (if (and (> n 0)
           (flpositive? x)
           (fl<? x 1e3))
; (if (and (> n 3) (flpositive? x))
      (fl* (eqn9.1.75 (- n 1) x)
           (loop (fl/ 2.0 x) (inexact n) 0))
      (flfirst-bessel n x)))

;;; Equation 9.1.89 :
;;;
;;;     Y_0(x) = 2/pi (log (x/2) + gamma) J_0(x)
;;;                 - 4/pi \sum_{k=1}^\infty (-1)^k J_{2k}(x)/k
;;;
;;; To reduce roundoff error, the infinite sum is computed
;;; non-tail-recursively.
;;;
;;; FIXME: not used at present, so I've commented it out.

#;
(define (eqn9.1.89 n x)
  (define (sum k)
    (let* ((k2 (+ k k))
           (j2k (flfirst-bessel k2 x))
           (y (if (even? k) j2k (fl- j2k))))
      (if (flzero? y)
          y
          (fl+ y (sum (+ k 1))))))
  (if (not (= n 0)) (error "eqn9.1.89 requires n=0"))
  (fl- (fl* 2.0
            fl-1/pi
            (fl+ (fllog (fl/ x 2.0)) fl-euler)
            (flfirst-bessel 0 x))
       (fl* 4.0 fl-1/pi (sum 1))))
            
            

;;; Equation 9.2.1 states an asymptotic approximation that agrees
;;; with C99 jn to 6 decimal places for n = 0 and x = 1e6.

(define (eqn9.2.1 n x)
  (fl* (flsqrt (/ 2.0 (fl* fl-pi x)))
       (flcos (fl- x (fl* fl-pi (fl+ (fl* 0.5 (inexact n)) 0.25))))))

;;; Equation 9.2.2 states an asymptotic approximation for Y_n.
;;;
;;; FIXME: not used at present, so I've commented it out.

#;
(define (eqn9.2.2 n x)
  (fl* (flsqrt (/ 2.0 (fl* fl-pi x)))
       (flsin (fl- x (fl* fl-pi (fl+ (fl* 0.5 (inexact n)) 0.25))))))

;;; Equation 9.2.5 : For large x,
;;;
;;;     J_n(x) = sqrt (2/(pi x)) [ P(n, x) cos theta - Q (n, x) sin theta ]
;;;
;;; where
;;;
;;;     theta = x - (n/2 + 1/4) pi
;;;
;;; and P(n, x) and Q(n, x) are defined by equations 9.2.9 and 9.2.10.

(define (eqn9.2.5 n x)
  (let ((theta (fl- x (fl* (fl+ (/ n 2.0) 0.25) fl-pi))))
    (fl* (flsqrt (fl/ 2.0 (fl* fl-pi x)))
         (fl- (fl* (eqn9.2.9 n x) (flcos theta))
              (fl* (eqn9.2.10 n x) (flsin theta))))))

;;; Equation 9.2.6 : For large x,
;;;
;;;     Y_n(x) = sqrt (2/(pi x)) [ P(n, x) sin theta + Q (n, x) cos theta ]
;;;
;;; where
;;;
;;;     theta = x - (n/2 + 1/4) pi
;;;
;;; and P(n, x) and Q(n, x) are defined by equations 9.2.9 and 9.2.10.

(define (eqn9.2.6 n x)
  (let ((theta (fl- x (fl* (fl+ (/ n 2.0) 0.25) fl-pi))))
    (fl* (flsqrt (fl/ 2.0 (fl* fl-pi x)))
         (fl+ (fl* (eqn9.2.9 n x) (flsin theta))
              (fl* (eqn9.2.10 n x) (flcos theta))))))

(define (eqn9.2.9 n x) ; returns P(n, x)
  (define mu (fl* 4.0 (flsquare (inexact n))))
  (define (coefficients k2 p fact2k)
    (let ((c (fl/ p fact2k)))
      (if (fl>? k2 20.0) ; FIXME
          (list c)
          (cons c (coefficients (fl+ k2 2.0)
                                (fl* p
                                     (fl- mu (flsquare (fl+ k2 1.0)))
                                     (fl- mu (flsquare (fl+ k2 3.0))))
                                (fl* fact2k
                                     (fl+ k2 1.0)
                                     (fl+ k2 2.0)))))))
  (polynomial-at (fl- (fl/ (flsquare (fl* 8.0 x))))
                 (coefficients 0.0 1.0 1.0)))

(define (eqn9.2.10 n x) ; returns Q(n, x)
  (define mu (fl* 4.0 (flsquare (inexact n))))
  (define (coefficients k2+1 p fact2k+1)
    (let ((c (fl/ p fact2k+1)))
      (if (fl>? k2+1 20.0) ; FIXME
          (list c)
          (cons c (coefficients (fl+ k2+1 2.0)
                                (fl* p
                                     (fl- mu (flsquare (fl+ k2+1 2.0)))
                                     (fl- mu (flsquare (fl+ k2+1 4.0))))
                                (fl* fact2k+1
                                     (fl+ k2+1 1.0)
                                     (fl+ k2+1 2.0)))))))
  (fl* (fl/ (fl* 8.0 x))
       (polynomial-at (fl- (fl/ (flsquare (fl* 8.0 x))))
                      (coefficients 1.0 (fl- mu 1.0) 1.0))))

;;; Equation 9.4.3 is a polynomial approximation attributed to
;;; E E Allen, Analytical approximations, Math. Tables Aids Comp. 8,
;;; 240-241 (1954), and Polynomial approximations to some modified
;;; Bessel functions, Math. Tables Aids Comp. 10, 162-164 (156)
;;; (with permission).
;;;
;;; This is commented out because Newman's similar polynomial
;;; approximation is simpler and has better error bounds.

#;
(define (eqn9.4.3 n x)
  (if (> n 0)
      (flfirst-bessel n x)
      (fl* (fl/ (flsqrt x))                              ; modulus
           (polynomial-at (fl/ 3.0 x)
                          '(+0.79788456
                            -0.00000077
                            -0.00552740
                            -0.00009512
                            +0.00137237
                            -0.00072805
                            +0.00014476))
           (flcos (fl+ x                                 ; phase
                       (polynomial-at (fl/ 3.0 x)
                                      '(-0.78539816
                                        -0.04166397
                                        -0.00003954
                                        +0.00262573
                                        -0.00054125
                                        -0.00029333
                                        +0.00013558)))))))

;;; J N Newman's polynomial approximation for x >= 3, from Table 4.

#;
(define (newman-table4 n x)
  (if (> n 0)
      (flfirst-bessel n x)
      (fl* (fl/ (flsqrt x))
           (polynomial-at (flsquare (fl/ 3.0 x))
                          '(+0.79788454
                            -0.00553897
                            +0.00099336
                            -0.00044346
                            +0.00020445
                            -0.00004959))
           (flcos (fl+ x
                       (fl- fl-pi/4)
                       (fl* (fl/ 3.0 x)
                            (polynomial-at (flsquare (fl/ 3.0 x))
                                           '(-0.04166592
                                             +0.00239399
                                             +0.00073984
                                             -0.00031099
                                             -0.00007605))))))))

