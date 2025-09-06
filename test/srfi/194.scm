(import (srfi 1) (srfi 27) (srfi 133) (srfi 158) (srfi 194))
(include "test.scm")

;[esl] adapted from original tests by
;;; SPDX-FileCopyrightText: 2020 John Cowan
;;; SPDX-FileCopyrightText: 2020 Arvydas Silanskas
;;; SPDX-FileCopyrightText: 2024 Bradley J Lucier
;;; SPDX-License-Identifier: MIT

(define-syntax test-approximate
  (syntax-rules ()
    ((_ target value max-delta)
     (test-assert (and (<= value (+ target max-delta))
                  (>= value (- target max-delta)))))))

(test-begin "srfi-194")

;;; syntax just we can plop it at top and still allow internal `define`s
(define-syntax reset-source!
  (syntax-rules ()
    ((_)
     (define _ (random-source-pseudo-randomize! (current-random-source) 0 0)))))

(define (reset-source!*)
  (random-source-pseudo-randomize! (current-random-source) 0 0))

(define (assert-number-generator/all-in-range gen from to)
  (test-assert
    (generator-every
      (lambda (num)
        (and (>= num from)
             (< num to)))
      (gtake gen 1000))))

(define (assert-number-generator gen from to)
  (define range (- to from))
  (define lower-quarter (+ from (* 0.25 range)))
  (define upper-quarter (- to (* 0.25 range)))
  (assert-number-generator/all-in-range gen from to)

  (test-assert
    (generator-any
      (lambda (num)
        (and (>= num from)
             (< num lower-quarter)))
      (gtake gen 1000)))

  (test-assert
    (generator-any
      (lambda (num)
        (and (>= num lower-quarter)
             (< num upper-quarter)))
      (gtake gen 1000)))

  (test-assert
    (generator-any
      (lambda (num)
        (and (>= num upper-quarter)
             (< num to)))
      (gtake gen 1000))))

(define (assert-int-generator gen byte-size signed?)
  (define from (if signed?
                   (- (expt 2 (- byte-size 1)))
                   0))
  (define to (if signed?
                 (expt 2 (- byte-size 1))
                 (expt 2 byte-size)))
  (assert-number-generator gen from to))


(test-group "Test clamp real number"
            (reset-source!)
            (test-equal 10.0 (clamp-real-number 5.0 10.0 11))
            (test-equal 5.0 (clamp-real-number 5.0 10.0 2.0))
            (test-equal 7.5 (clamp-real-number 5.0 10.0 7.5)))

(test-group "Test with-random-source basic syntax"
            (reset-source!)
            (with-random-source default-random-source
                                (lambda () (make-random-integer-generator 0 10))))

;;; testing random source, which is implementation specific
(cond-expand
  (gauche
    (import
      (gauche base)
      (math mt-random))
    (test-group "Test with-random-source"
                (reset-source!)
                ;; create and consume generators that are made with different source
                ;; with various order, and check that order doesn't change the outcome
                (define (test-multiple-sources gen1-maker gen1-expect
                                               gen2-maker gen2-expect)

                  ;;create gen1, consume gen1, create gen2, consume gen2
                  (let ((gen1 (gen1-maker)))
                   (test-equal (generator->list gen1) gen1-expect)
                   (let ((gen2 (gen2-maker)))
                    (test-equal (generator->list gen2) gen2-expect)))

                  ;; create gen1, create gen2, consume gen1, consume gen2
                  (let ((gen1 (gen1-maker))
                        (gen2 (gen2-maker)))
                    (test-equal (generator->list gen1) gen1-expect)
                    (test-equal (generator->list gen2) gen2-expect)))

                (define multiple-sources-testcase
                  (list (lambda ()
                          (gtake (with-random-source
                                   (make <mersenne-twister> :seed 0)
                                   (lambda () (make-random-integer-generator 0 10)))
                                 5))
                        '(5 5 7 8 6)
                        (lambda ()
                          (gtake (with-random-source
                                   (make <mersenne-twister> :seed 1)
                                   (lambda () (make-random-integer-generator 0 10)))
                                 5))
                        '(4 9 7 9 0)))
                (apply test-multiple-sources multiple-sources-testcase)))
  (else
   #f))

(test-group "Test make-random-source-generator"
            (reset-source!)
            (define (make-numbers src-gen)
              (define gen1 (with-random-source (src-gen) (lambda () (make-random-integer-generator 0 100))))
              (define gen2 (with-random-source (src-gen) (lambda () (make-random-real-generator 0. 100.))))
              (generator->list
                (gappend
                  (gtake gen1 10)
                  (gtake gen2 10))))

            (test-equal
              (make-numbers (make-random-source-generator 0))
              (make-numbers (make-random-source-generator 0)))
            (test-assert
              (not (equal? (make-numbers (make-random-source-generator 0))
                           (make-numbers (make-random-source-generator 1))))))
(test-group "Test random int"
            (reset-source!)
            (assert-number-generator
              (make-random-integer-generator 1 100)
              1 100)

            (for-each
              (lambda (testcase)
                (define make-gen (car testcase))
                (define byte-size (cadr testcase))
                (define signed? (caddr testcase))
                (assert-int-generator (make-gen) byte-size signed?))
              (list
                (list make-random-u8-generator 8 #f)
                (list make-random-s8-generator 8 #t)
                (list make-random-u16-generator 16 #f)
                (list make-random-s16-generator 16 #t)
                ;[esl-] nop support for these:
                #;(list make-random-u32-generator 32 #f)
                #;(list make-random-s32-generator 32 #t)
                #;(list make-random-u64-generator 64 #f)
                #;(list make-random-s64-generator 64 #t)))

            ;;test u1 separately, since it will fail quarter checks due to small range
            (assert-number-generator/all-in-range (make-random-u1-generator) 0 2)
            (test-assert
              (generator-any
                (lambda (v) (= v 0))
                (gtake (make-random-u1-generator) 100)))
            (test-assert
              (generator-any
                (lambda (v) (= v 1))
                (gtake (make-random-u1-generator) 100))))

(test-group "Test random real"
            (reset-source!)
            (assert-number-generator
              (make-random-real-generator 1.0 5.0)
              1.0 5.0)

            (test-assert
              (generator-any
                (lambda (v)
                  (not (= v (floor v))))
                (make-random-real-generator 1.0 5.0))))

#| [esl] need support for complex numbers
(test-group "Test complex rectangular"
            (reset-source!)

            (assert-number-generator
              (gmap
                real-part
                (make-random-rectangular-generator -10.0 10.0 -100.0 100.0))
              -10 10)

            (assert-number-generator
              (gmap
                imag-part
                (make-random-rectangular-generator -100.0 100.0 -10.0 10.0))
              -10 10)

            (test-assert
              (generator-any
                (lambda (num)
                  (and (not (= 0 (real-part num)))
                       (not (= 0 (imag-part num)))))
                (make-random-rectangular-generator -10.0 10.0 -10.0 10.0))))
(test-group "Test complex polar"
            (reset-source!)
            (define PI (* 4 (atan 1.0)))

            (define (test-polar g origin mag-from mag-to angle-from angle-to test-converge-origin)
              (assert-number-generator
                (gmap
                  (lambda (num)
                    (angle (- num origin)))
                  g)
                angle-from angle-to)

              (assert-number-generator
                (gmap
                  (lambda (num)
                    (magnitude (- num origin)))
                  g)
                mag-from mag-to)

              ;; split generated area through circle at 0.5*(mag-from + mag-to)
              ;; and validate generated points in them proportional to their area
              (let* ((outter-count 0)
                     (inner-count 0)
                     (donut-area (lambda (r1 r2) (- (* r1 r1) (* r2 r2))))
                     (mag-mid (/ (+ mag-from mag-to) 2.))
                     (expected-fraction (/ (donut-area mag-to mag-mid)
                                           (donut-area mag-mid mag-from))))
                (generator-for-each
                  (lambda (point)
                    (if (< (magnitude (- point origin)) mag-mid)
                        (set! inner-count (+ 1 inner-count))
                        (set! outter-count (+ 1 outter-count))))
                  (gtake g 10000))
                (test-approximate expected-fraction (/ outter-count inner-count) 0.2))

              ;; test points converge to center
              (when test-converge-origin
                (let ((sum 0+0i))
                  (generator-for-each
                    (lambda (point) (set! sum (+ point sum)))
                    (gtake g 1000))
                  (test-approximate (real-part origin) (real-part (/ sum 1000.)) 0.1)
                  (test-approximate (imag-part origin) (imag-part (/ sum 1000.)) 0.1))))


            (test-polar (make-random-polar-generator 0. 1.)
                        0+0i 0. 1. (- PI) PI #t)

            (test-polar (make-random-polar-generator 2+5i 1. 2.)
                        2+5i 1. 2. (- PI) PI #t)

            (test-polar (make-random-polar-generator 1. 2. -1. 1.)
                        0+0i 1. 2. -1. 1. #f)

            (test-polar (make-random-polar-generator -1+3i 0. 2. (- PI) PI)
                        -1+3i 0. 2. (- PI) PI #t))
|#

(test-group "Test random bool"
            (reset-source!)
            (test-assert
              (generator-every
                (lambda (v)
                  (or (eq? v #t)
                      (eq? v #f)))
                (gtake (make-random-boolean-generator) 10000)))

            (test-assert
              (generator-any
                (lambda (v)
                  (eq? #t v))
                (make-random-boolean-generator)))

            (test-assert
              (generator-any
                (lambda (v)
                  (eq? #f v))
                (make-random-boolean-generator))))

(test-group "Test random char"
            (reset-source!)
            (test-assert
              (generator-every
                (lambda (v)
                  (or (equal? v #\a)
                      (equal? v #\b)))
                (gtake (make-random-char-generator "ab")
                       10000)))

            (test-assert
              (generator-any
                (lambda (v)
                  (equal? v #\a))
                (make-random-char-generator "ab")))

            (test-assert
              (generator-any
                (lambda (v)
                  (equal? v #\b))
                (make-random-char-generator "ab"))))

(test-group "Test random string"
            (reset-source!)
            (test-assert
              (generator-every
                (lambda (str)
                  (and (< (string-length str) 5)
                       (every (lambda (c)
                                (or (equal? c #\a)
                                    (equal? c #\b)))
                              (string->list str))))
                (gtake (make-random-string-generator 5 "ab")
                       10000)))

            (test-assert
              (generator-any
                (lambda (str)
                  (equal? "abb" str))
                (make-random-string-generator 4 "ab"))))

(test-group "Test Bernoulli"
            (reset-source!)
            (define g (make-bernoulli-generator 0.7))
            (define expect 7000)
            (define actual (generator-count
                             (lambda (i) (= i 1))
                             (gtake g 10000)))
            (define ratio (inexact (/ actual expect)))
            (test-assert (> ratio 0.9))
            (test-assert (< ratio 1.1)))

(test-group "Test categorical"
            (reset-source!)
            (define result-vec (vector 0 0 0))
            (define expect-vec (vector 2000 5000 3000))
            (define wvec (vector 20 50 30))
            (define g (make-categorical-generator wvec))
            (generator-for-each
              (lambda (i)
                (vector-set! result-vec i (+ 1 (vector-ref result-vec i))))
              (gtake g 10000))
            (vector-for-each
              (lambda (result expect)
                (define ratio (inexact (/ result expect)))
                (test-approximate 1.0 ratio 0.1))
              result-vec
              expect-vec))

;[esl] changed to use flonums
(test-group "Test poisson"
            (reset-source!)
            ;;TODO import from somewhere?
            (define (fact k)
              (cond
                ((<= k 1) 1)
                (else (* k (fact (- k 1))))))
            (define (expected-fraction L k)
              (/ (* (inexact (expt L k)) (inexact (exp (- L))))
                 (fact (inexact k))))

            (define (test-poisson L poisson-gen test-points)
              (generator-every
                (lambda (k)
                  (define expect (expected-fraction L k))
                  (define actual (/ (generator-count
                                      (lambda (i) (= i k))
                                      (gtake poisson-gen 10000))
                                    10000))
                  (define ratio (/ actual expect))
                  ;[esl] if factorial overflows flonum, skip the test
                  (unless (nan? ratio)
                    (test-assert (> ratio 0.8)) ;[esl*] was 8/10
                    (test-assert (< ratio 1.2)))) ;[esl*] was 12/10
                (list->generator test-points)))

            (test-poisson 2 (make-poisson-generator 2) '(1 2 3))
            (test-poisson 40 (make-poisson-generator 40) '(30 40 50))
            (test-poisson 280 (make-poisson-generator 280) '(260 280 300)))

(test-group "Test normal"
            (reset-source!)
            (define frac-at-1dev 0.34134)
            (define frac-at-2dev 0.47725)
            (define frac-at-3dev 0.49865)

            (define (test-normal-at-point gen count-from count-to expected-fraction)
              (define actual (/ (generator-count
                                  (lambda (n)
                                    (and (>= n count-from)
                                         (< n count-to)))
                                  (gtake gen 10000))
                                10000.0))
              (test-assert (and (> actual (* 0.9 expected-fraction))
                                (< actual (* 1.1 expected-fraction)))))

            (define (test-normal gen mean deviation)
              (test-normal-at-point gen mean (+ mean deviation) frac-at-1dev)
              (test-normal-at-point gen mean (+ mean (* 2 deviation)) frac-at-2dev)
              (test-normal-at-point gen mean (+ mean (* 3 deviation)) frac-at-3dev))

            (test-normal (make-normal-generator) 0.0 1.0)
            (test-normal (make-normal-generator 1.0) 1.0 1.0)
            (test-normal (make-normal-generator 1.0 2.0) 1.0 2.0))

(test-group "Test exponential"
            (reset-source!)
            (define (expected-fraction mean x)
              (- 1 (exp (* (- (/ 1.0 mean)) x))))

            (define (test-exp-at-point gen count-to expected)
              (define actual (/ (generator-count
                                  (lambda (n)
                                    (< n count-to))
                                  (gtake gen 10000))
                                10000.0))
              (test-assert (> actual (* 0.9 expected)))
              (test-assert (< actual (* 1.1 expected))))

            (define (test-exp gen mean)
              (test-exp-at-point gen 1 (expected-fraction mean 1))
              (test-exp-at-point gen 2 (expected-fraction mean 2))
              (test-exp-at-point gen 3 (expected-fraction mean 3)))

            (test-exp (make-exponential-generator 1) 1)
            (test-exp (make-exponential-generator 1.5) 1.5))

(test-group "Test geometric"
            (reset-source!)
            (define (expected-fraction p x)
              (* (expt (- 1 p) (- x 1)) p))

            (define (test-geom-at-point gen p x)
              (define expected (expected-fraction p x))
              (define actual (/ (generator-count
                                  (lambda (n)
                                    (= n x))
                                  (gtake gen 100000))
                                100000))
              (define ratio (/ actual expected))
              (test-assert (> ratio 0.9))
              (test-assert (< ratio 1.1)))

            (define (test-geom gen p)
              (test-geom-at-point gen p 1)
              (test-geom-at-point gen p 3)
              (test-geom-at-point gen p 5))

            (test-geom (make-geometric-generator 0.5) 0.5))

(test-group "Test uniform sampling"
            (reset-source!)
            (test-equal
              '()
              (generator->list (gsampling)))
            (test-equal
              '()
              (generator->list (gsampling (generator) (generator))))
            (test-equal
              '(1 1 1)
              (generator->list (gsampling (generator) (generator 1 1 1))))
            (test-assert
              (generator-any
                (lambda (el)
                  (= el 1))
                (gsampling (circular-generator 1) (circular-generator 2))))
            (test-assert
              (generator-any
                (lambda (el)
                  (= el 2))
                (gsampling (circular-generator 1) (circular-generator 2)))))

;;; See zipf-test.scm
(test-group "Test Zipf sampling"
            (reset-source!)
; SPDX-FileCopyrightText: 2020 Arvydas Silanskas
; SPDX-FileCopyrightText: 2020 Linas Vepštas
; SPDX-License-Identifier: MIT
;
; zipf-test.scm
; Unit tests for the Zipf (zeta) distribution.
;
; Created by Linas Vepstas 10 July 2020
; Part of srfi-194

; srfi-133 compatible API
(define zvector-map vector-map)
(define z2vector-map vector-map)
(define zvector-fold vector-fold)

; srfi-43 compatible API
;(define (zvector-map f vec)
;  (vector-map (lambda (i x) (f x)) vec))
;(define (z2vector-map f veca vecb)
;  (vector-map (lambda (i x y) (f x y)) veca vecb))
;(define (zvector-fold f knil vec)
;  (vector-fold (lambda (i x elt) (f x elt)) knil vec))

; ------------------------------------------------------------------
; Debug utility for gnuplot graphing.
; You can use this to dump a vector to a tab-delimited file.
(define (vector-to-file vec filename)
  (define (write-vec)
    (for-each
      (lambda (i)
        (define index (+ i 1))
        (define val (vector-ref vec i))
        (display index)
        (display "  ")
        (display val)
        (newline))
      (iota (vector-length vec))))
  (with-output-to-file filename write-vec))

; ------------------------------------------------------------------
; Test harness for exploring the Zipf (Riemann/Hurwicz zeta) distribution
; parameter space.
;
;     (test-zipf TEST-ID NVOCAB ESS QUE REPS TOL)
;
; * TEST-ID -- String ID, for debugging.
; * The next three parameters are presented to the generator as
;     (make-zipf-generator NVOCAB ESS QUE)
;   ++  NVOCAB -- Size of vocabulary to select from.
;   ++  ESS -- The Riemann zeta "s" exponent.
;   ++  QUE -- The Hurwicz zeta "q" offset.
; * REPS -- The number of samples to draw from the distribution.
; * TOL -- The test tolerance, governing the expected failure rate.
;
; The algorithm is roughly:
;   Take REPS samples (make-zipf-generator NVOCAB ESS QUE)
;   Accumulate them into NVOCAB histogram bins.
;   Normalize counts to unit probability (i.e. divide by NVOCAB)
;
; The resulting distribution should uniformly converge to C/(k+q)^s
; for 1 <= k <= NVOCAB where C is a normalization constant.
;
; This compares the actual distribution to the expected convergent
; and reports an error if it is not within TOL of the convergent.
; i.e. it computes the Banach l_0 norm of (distribution-convergent)
; TOL is to be given in units of standard deviations. So, for example,
; setting TOL to 6 gives a six-sigma bandpass, allowing the tests to
; usually pass.
;
; The keyword here is "usually". The tail of the Zipf distribution is
; generally quite thin, and experiences a lot of statistical variation.
; There does not seem to be any published theory of exactly how the
; central limit theorm can be applied to estimate the distribution of
; the tail. The code below assumes an approximate gaussian distribution,
; computes it and tests it; however certain parameter ranges violate
; this assumption. Thus, this test will fail from time to time,
; depending on the luck of the draw. If it fails, it should be repeated
; once or twice, ubtil it passes.
;
(define (test-zipf-once TEST-ID NVOCAB ESS QUE REPS TOL)

  ; Default random number generator
  (define ZGEN make-zipf-generator)

  ; Bin-counter containing accumulated histogram.
  (define bin-counts
    (let ((bin-counts (make-vector NVOCAB 0)))
     ; Accumulate samples into the histogram.
     (generator-for-each
       (lambda (SAMP)
         (define offset (- SAMP 1))
         (vector-set! bin-counts offset (+ 1 (vector-ref bin-counts offset))))
       (gtake (ZGEN NVOCAB ESS QUE) REPS))
     bin-counts))

  ; Verify the distribution is within tolerance.
  ; This is written out long-hand for easier debuggability.

  ; Frequency is normalized to be 0.0 to 1.0
  (define frequency (zvector-map (lambda (n) (/ n REPS)) bin-counts))
  (define probility (zvector-map (lambda (n) (inexact n)) frequency))

  ; Sequence 1..NVOCAB
  (define seq (list->vector (iota NVOCAB 1)))

  ; Sequence  1/(k+QUE)^ESS
  (define inv-pow
    (zvector-map (lambda (k) (expt (+ k QUE) (- (inexact ESS)))) seq))

  ; Hurwicz harmonic number sum_1..NVOCAB 1/(k+QUE)^ESS
  (define hnorm
    (zvector-fold
      (lambda (sum cnt) (+ sum cnt)) 0 inv-pow))

  ; The expected distribution.
  (define expect
    (zvector-map (lambda (x) (/ x hnorm)) inv-pow))

  ; Convert to floating point.
  (define prexpect (zvector-map (lambda (x) (inexact x)) expect))

  ; The difference.
  (define diff (z2vector-map (lambda (x y) (- x y)) probility prexpect))

  ; Re-weight the tail by k^{s/2}. This seems give a normal error
  ; distribution. ... at least, for small q. Problems for large q
  ; and with undersampling; so we hack around that.
  (define err-dist
    (if (< 10 QUE) diff
        (z2vector-map (lambda (i x) (* x (expt (+ i 1) (* 0.5 ESS))))
                    (list->vector (iota (vector-length diff)))
                    diff)))

  ; Normalize to unit root-mean-square.
  (define rms (/ 1 (sqrt (* 2 3.141592653 REPS))))
  (define norm-dist (zvector-map (lambda (x) (/ x rms)) err-dist))

  ; Maximum deviation from expected distribution (l_0 norm)
  (define l0-norm
    (zvector-fold
      (lambda (sum x) (if (< sum (abs x)) (abs x) sum)) 0 norm-dist))

  ; The mean.
  (define mean
    (/ (zvector-fold (lambda (sum x) (+ sum x)) 0 norm-dist)
       NVOCAB))

  (define root-mean-square
    (sqrt (/ (zvector-fold (lambda (sum x) (+ sum (* x x))) 0 norm-dist)
             NVOCAB)))

  ; Test for uniform convergence.
  ; (test-assert TEST-ID (<= l0-norm TOL))
  (define tol-result (<= l0-norm TOL))

  ; Should not random walk too far away.
  ; Could tighten this with a proper theory of the error distribution.
  ; (test-assert TEST-ID (< (abs mean) 3))
  (define mean-result (< (abs mean) 3))
  ; I don't understand the error distribution ....
  ; (test-assert (and (< 0.4 root-mean-square) (< root-mean-square 1.5)))

  ; Sanity check. The total counts in the bins should be equal to REPS.
  ; If this fails, the test harness itself is broken.
  (test-assert TEST-ID
    (equal? REPS
            (zvector-fold
              (lambda (sum cnt) (+ sum cnt)) 0 bin-counts)))

  ; Utility debug printing
  ;(vector-to-file probility "probility.dat")
  ;(vector-to-file prexpect "prexpect.dat")
  ;(vector-to-file diff "diff.dat")

  (list tol-result mean-result))

; ------------------------------------------------------------------
; Sometimes the Zip test fails, due to random variation. It should
; pass, if attempted a second time. This gives it three chances.
; If it fails three times, then something bad is happening.
(define (test-zipf TEST-ID NVOCAB ESS QUE REPS TOL)

  ;; Three strikes, you're out.
  (define RETRY 3)

  (define num-failures
    (list-index
     (lambda (n)
       (define rc (test-zipf-once TEST-ID NVOCAB ESS QUE REPS TOL))
       (and (first rc) (second rc)))
     (iota RETRY)))

  ; `num-failures` will be #f if it failed each and every time.
  ;(if (number? num-failures)
  ;  (if (< 0 num-failures)
  ;    (format #t "Test '~A' out of bounds ~A times.\n" TEST-ID num-failures))
  ;  (format #t "Error: Test '~A' failed every time!\n" TEST-ID))

  ; Announce excessive repeated failures.
  (test-assert TEST-ID num-failures)
)

; ------------------------------------------------------------------
; Explore the parameter space.
(define (zipf-test-group)
  ; (test-begin "srfi-194-zipf")

  ; The unit test computes something that is "almost" a standard
  ; deviation for the error distribution. Except, maybe not quite,
  ; I don't fully understand the theory. So most tests seem to come
  ; in fine in well-under a six-sigma deviation, but some of the wilder
  ; parameter choices misbehave, so six-sigma doesn't always work.
  ; Also, when the number of bins is large, its easy to under-sample;
  ; some bins end up empty and the std-dev is thrown off as a result.
  ; Thus, the tolerance bounds below are hand-adjusted.
  (define six-sigma 6.0)

  (define hack-que 3.0)

  ; Zoom into s->1
  (test-zipf "zoom-1"   30 1.1     0 1000 six-sigma)
  (test-zipf "zoom-2"   30 1.01    0 1000 six-sigma)
  (test-zipf "zoom-3"   30 1.001   0 1000 six-sigma)
  (test-zipf "zoom-4"   30 1.0001  0 1000 six-sigma)
  (test-zipf "zoom-5"   30 1.00001 0 1000 six-sigma)

  (test-zipf "zoom-6"   30 (+ 1 1e-6)  0 1000 six-sigma)
  (test-zipf "zoom-8"   30 (+ 1 1e-8)  0 1000 six-sigma)
  (test-zipf "zoom-10"  30 (+ 1 1e-10) 0 1000 six-sigma)
  (test-zipf "zoom-12"  30 (+ 1 1e-12) 0 1000 six-sigma)
  (test-zipf "zoom-14"  30 (+ 1 1e-14) 0 1000 six-sigma)
  (test-zipf "zoom-inf" 30 1           0 1000 six-sigma)

  ; Verify improving uniform convergence
  (test-zipf "uniform-1" 30 1  0 10000   six-sigma)
  (test-zipf "uniform-2" 30 1  0 100000  six-sigma)

  ; Larger vocabulary
  (test-zipf "mid-voc-1" 300 1.1     0 10000 six-sigma)
  (test-zipf "mid-voc-2" 300 1.01    0 10000 six-sigma)
  (test-zipf "mid-voc-3" 300 1.001   0 10000 six-sigma)
  (test-zipf "mid-voc-4" 300 1.0001  0 10000 six-sigma)
  (test-zipf "mid-voc-5" 300 1.00001 0 10000 six-sigma)

  ; Larger vocabulary. Take more samples....
  (test-zipf "large-voc-1" 3701 1.1     0 40000 six-sigma)
  (test-zipf "large-voc-2" 3701 1.01    0 40000 six-sigma)
  (test-zipf "large-voc-3" 3701 1.001   0 40000 six-sigma)
  (test-zipf "large-voc-4" 3701 1.0001  0 40000 six-sigma)
  (test-zipf "large-voc-5" 3701 1.00001 0 40000 six-sigma)

  ; Huge vocabulary; few samples. Many bins will be empty,
  ; causing the std-dev to get large.
  (test-zipf "huge-voc-1" 43701 (+ 1 1e-6)  0 60000 9.5)
  (test-zipf "huge-voc-2" 43701 (+ 1 1e-7)  0 60000 9.5)
  (test-zipf "huge-voc-3" 43701 (+ 1 1e-9)  0 60000 9.5)
  (test-zipf "huge-voc-4" 43701 (+ 1 1e-12) 0 60000 9.5)
  (test-zipf "huge-voc-5" 43701 1           0 60000 9.5)

  ; Large s, small range
  (test-zipf "big-s-lo-1" 5 1.1     0 1000 six-sigma)
  (test-zipf "big-s-lo-2" 5 2.01    0 1000 six-sigma)
  (test-zipf "big-s-lo-3" 5 4.731   0 1000 six-sigma)
  (test-zipf "big-s-lo-4" 5 9.09001 0 1000 six-sigma)
  (test-zipf "big-s-lo-5" 5 13.45   0 1000 8.0)

  ; Large s, larger range. Most histogram bins will be empty
  ; so allow much larger error margins. There are excessively
  ; frequent large failures in this bunch.
  (test-zipf "bis-mid-1" 130 1.5     0 30000 six-sigma)
  (test-zipf "bis-mid-2" 130 2.03    0 30000 9.0)
  (test-zipf "bis-mid-3" 130 4.5     0 30000 36.0) ; This one is problematic
  (test-zipf "bis-mid-4" 130 6.66    0 30000 24.0)

  ; Verify that accuracy improves with more samples.
  (test-zipf "samp-bi-1" 129 1.1     0 10000 six-sigma)
  (test-zipf "samp-bi-2" 129 1.01    0 10000 six-sigma)
  (test-zipf "samp-bi-3" 129 1.001   0 10000 six-sigma)
  (test-zipf "samp-bi-4" 129 1.0001  0 10000 six-sigma)
  (test-zipf "samp-bi-5" 129 1.00001 0 10000 six-sigma)

  ; Non-zero Hurwicz parameter
  (test-zipf "hurw-1" 131 1.1     0.3    10000 six-sigma)
  (test-zipf "hurw-2" 131 1.1     1.3    10000 six-sigma)
  (test-zipf "hurw-3" 131 1.1     6.3    10000 six-sigma)
  (test-zipf "hurw-4" 131 1.1     20.23  10000 six-sigma)

  ; Negative Hurwicz parameter. Must be greater than branch point at -0.5.
  (test-zipf "hneg-1" 81 1.1     -0.1   1000 six-sigma)
  (test-zipf "hneg-2" 81 1.1     -0.3   1000 six-sigma)
  (test-zipf "hneg-3" 81 1.1     -0.4   1000 six-sigma)
  (test-zipf "hneg-4" 81 1.1     -0.499 1000 six-sigma)

  ; A walk into a stranger corner of the parameter space.
  (test-zipf "big-h-1" 131 1.1     41.483 10000 hack-que)
  (test-zipf "big-h-2" 131 2.1     41.483 10000 hack-que)
  (test-zipf "big-h-3" 131 6.1     41.483 10000 hack-que)
  (test-zipf "big-h-4" 131 16.1    41.483 10000 hack-que)
  (test-zipf "big-h-5" 131 46.1    41.483 10000 hack-que)
  (test-zipf "big-h-6" 131 96.1    41.483 10000 hack-que)

  ; A still wilder corner of the parameter space.
  (test-zipf "huhu-1" 131 1.1     1841.4 10000 hack-que)
  (test-zipf "huhu-2" 131 1.1     1.75e6 10000 hack-que)
  (test-zipf "huhu-3" 131 2.1     1.75e6 10000 hack-que)
  (test-zipf "huhu-4" 131 12.1    1.75e6 10000 hack-que)
  (test-zipf "huhu-5" 131 42.1    1.75e6 10000 hack-que)

  ; Lets try s less than 1
  (test-zipf "small-s-1" 35 0.9     0 1000 six-sigma)
  (test-zipf "small-s-2" 35 0.99    0 1000 six-sigma)
  (test-zipf "small-s-3" 35 0.999   0 1000 six-sigma)
  (test-zipf "small-s-4" 35 0.9999  0 1000 six-sigma)
  (test-zipf "small-s-5" 35 0.99999 0 1000 six-sigma)

  ; Attempt to force an overflow
  (test-zipf "ovfl-1" 437 (- 1 1e-6)  0 1000 six-sigma)
  (test-zipf "ovfl-2" 437 (- 1 1e-7)  0 1000 six-sigma)
  (test-zipf "ovfl-3" 437 (- 1 1e-9)  0 1000 six-sigma)
  (test-zipf "ovfl-4" 437 (- 1 1e-12) 0 1000 six-sigma)

  ; Almost flat distribution
  (test-zipf "flat-1" 36 0.8     0 1000 six-sigma)
  (test-zipf "flat-2" 36 0.5     0 1000 six-sigma)
  (test-zipf "flat-3" 36 0.1     0 1000 six-sigma)

  ; A visit to crazy-town -- increasing, not decreasing exponent
  (test-zipf "neg-s-1" 36 0.0     0 1000 six-sigma)
  (test-zipf "neg-s-2" 36 -0.1    0 1000 six-sigma)
  (test-zipf "neg-s-3" 36 -1.0    0 1000 six-sigma)
  (test-zipf "neg-s-4" 36 -3.0    0 1000 six-sigma)

  ; More crazy with some Hurwicz on top.
  (test-zipf "neg-shu-1" 16 0.0     0.5 1000 six-sigma)
  (test-zipf "neg-shu-2" 16 -0.2    2.5 1000 six-sigma)
  (test-zipf "neg-shu-3" 16 -1.3    10  1000 six-sigma)
  (test-zipf "neg-shu-4" 16 -2.9    100 1000 six-sigma))

(zipf-test-group))

(test-group "Test sphere"

;;; SPDX-FileCopyrightText: 2020 Arvydas Silanskas
;;; SPDX-FileCopyrightText: 2024 Bradley J Lucier
;;; SPDX-License-Identifier: MIT
;;;
;;; Take REPS samples from unit sphere, verify random distribution.
;;;
;;; This test checks that:
;;; * Every sample has unit length, within numerical tolerance.
;;; * The REPS samples are uniformly distributed.
;;; * Rotations of the REPS samples are uniformly distributed.
(define (test-sphere sphereg dim-sizes REPS rotate?)
  (define random-int (random-source-make-integers (current-random-source)))
  (define random-real (random-source-make-reals (current-random-source)))
  (define N (- (vector-length dim-sizes) 1))

  ;; Fix list of samples
  (define samples
    (generator->list (gtake sphereg REPS)))

  (define (l2-norm VEC)
    (sqrt (vector-fold
            (lambda (sum x l) (+ sum (/ (* x x)
                                        (* l l))))
            0
            VEC
            dim-sizes)))

  ;; Rotate the j'th amnd k'th coordinates of a vector VEC
  ;; by cosine co and sine si
  (define (pair-rot VEC j k co si)
    (define oj (vector-ref VEC j))
    (define ok (vector-ref VEC k))
    (define nj (+ (* co oj) (* si ok)))
    (define nk (+ (* (- si) oj) (* co ok)))
    (list->vector
      (map (lambda (index)
             (cond
               ((= index j) nj)
               ((= index k) nk)
               (else (vector-ref VEC index))))
           (iota (vector-length VEC)))))

  ;; Apply a random rotation to a collection of vectors
  (define how-many-rots
    (if (< 10 N) 10 N))

  (define (arb-rot VEC-LIST)
    (define j (random-int N))
    (define k (+ j 1 (random-int (- N j))))
    (define theta (* 3.14 (random-real)))
    (define co (cos theta))
    (define si (sin theta))
    (define rvl
      (map (lambda (vec)
             (pair-rot vec j k co si))
           VEC-LIST))
    (if (not (= 0 (random-int how-many-rots)))
        (arb-rot rvl)
        rvl))

  ;; Expect a vector approaching zero. That is, each individual
  ;; coordinate should be uniformly randomly distributed in the
  ;; interval [-1,1]. The sum of REPS samples of these should
  ;; converge to zero. The standard deviation of a uniform
  ;; distribution is sqrt(REPS/12).
  ;; https://en.wikipedia.org/wiki/Continuous_uniform_distribution
  ;; So setting max bound of 9 stddev should allow it to usually
  ;; pass.
  (define (converge-to-zero samples)
    (fold (lambda (acc sample) (vector-map + sample acc))
          (make-vector REPS 0.0)
          samples))

  (define (should-be-zero samples)
    (l2-norm (converge-to-zero samples)))

  (define (norm-should-be-zero samples)
    (/ (should-be-zero samples) (sqrt (/ REPS 12.0))))

  (define (check-zero samples)
    (define num-stddev 9.0)
    (define zz (norm-should-be-zero samples))
    (test-assert (< zz num-stddev)))

  ;; maximum allowed tolerance for radius deviation
  (define EPS (* 2e-15 (sqrt N)))

  ;; Each individual sphere radius should be 1.0 to within float
  ;; tolerance.
  (for-each
    (lambda (SAMP)
      (test-approximate 1.0 (l2-norm SAMP) EPS))
    samples)

  ;; The distribution should be zero
  (check-zero samples)

  ;; Rotate wildly. Should still be uniform.
  (when rotate?
    (for-each
      (lambda (junk) (check-zero (arb-rot samples)))
      (make-list 12))))

(define (test-ball ballg dim-sizes)
  (define (l2-norm VEC)
    (sqrt (vector-fold
            (lambda (sum x l) (+ sum (/ (* x x)
                                        (* l l))))
            0
            VEC
            dim-sizes)))

  (define (test-ball-generates-on-radius radius err)
    (test-assert
      (generator-any
        (lambda (vec)
          (define n (l2-norm vec))
          (and (> n (- radius err))
               (< n (+ radius err))))
        (gtake ballg 10000))))

  (define (test-ball-avg-zero N)
    (define vec-sum
      (generator-fold
        (lambda (vec acc)
          (vector-map + vec acc))
        (make-vector (vector-length dim-sizes) 0.0)
        (gtake ballg N)))
    (define avg-vec
      (vector-map
        (lambda (e)
          (/ e N))
        vec-sum))
    (define n (l2-norm avg-vec))
    (test-assert (< n 1)))

  (test-ball-generates-on-radius 0.0 0.1)
  (test-ball-generates-on-radius 0.5 0.1)
  (test-ball-generates-on-radius 1.0 0.1)

  (test-ball-avg-zero 5000))

  ;; Number of standard deviation difference from the expected value
  ;; before we say a test failed.  If set to 2, then one out of
  ;; 20 tests will fail even if the code is correct.  Setting it to
  ;; 3 means that only one out of a 1000 tests will fail even if the
  ;; code is correct.

  (define STDs 3)

  (define (test-ellipsoid a b N)

    (define epsilon 1e-10)

    (define pi (* 4 (atan 1)))

    (define g (make-ellipsoid-generator (vector a b)))

    (define points (generator->list (gtake g N)))

    ;; The points on the "top" of the ellipse, with
    ;; x between -a/2 and a/2

    (define top
      (filter (lambda (v)
                (and (<  (- (/ a 2)) (vector-ref v 0) (/ a 2))
                     (<  0 (vector-ref v 1))))
              points))

    ;; The points on the "right" of the ellipse, with
    ;; y between -b/2 and b/2

    (define right
      (filter (lambda (v)
                (and (< (- (/ b 2)) (vector-ref v 1) (/ b 2))
                     (< 0 (vector-ref v 0))))
              points))

    (define (arc-length a b)
      ;; parametrization: (a\cos t, b\sin t)
      ;; this is the norm of the derivative
      (lambda (t)
        (sqrt (+ (square (* a (sin t)))
                 (square (* b (cos t)))))))

    (define (simpsons-rule f t0 t1 N)
      ;; O(Delta^4) numerical integration
      ;; integrate f between t0 and t1 with N intervals
      (let* ((Delta (/ (- t1 t0) N))
             (sum1 (do ((i 0 (+ i 1))
                        (sum 0. (+ sum
                                   (f (+ t0 (* i Delta)))
                                   (f (+ t0 (* (+ i 1) Delta))))))
                       ((= i N) sum)))
             (sum2 (do ((i 0 (+ i 1))
                        (sum 0. (+ sum (f (+ t0 (* (+ i 0.5) Delta))))))
                       ((= i N) sum))))
        (* Delta (/ (+ (* 4 sum2) sum1) 6))))

    (define p-right
      (/ (simpsons-rule (arc-length a b) (- (* pi 0.16666666666666666)) (* pi 0.16666666666666666) 100) ; 1/6
         (simpsons-rule (arc-length a b) 0 (* pi 2) 100)))

    (define p-top
      (/ (simpsons-rule (arc-length a b) (* pi 0.3333333333333333) (* pi 0.6666666666666666) 100) ; 1/3 2/3
         (simpsons-rule (arc-length a b) 0 (* pi 2) 100)))

    ;; test that they're all more-or-less on the ellipse

    (test-assert (every (lambda (p)
                          (< (abs (- (sqrt (+ (square (/ (vector-ref p 0) a))
                                              (square (/ (vector-ref p 1) b))))
                                     1))
                             epsilon))
                        points))

    (for-each (lambda (p m)
                ;; p = probability of landing in arc, m = measured number
                (test-assert (< (abs (- (* p N) m))
                                (* STDs (sqrt (* N p (- 1 p)))))))
              (list p-right p-top)
              (map length (list right top)))
    )

  (define (test-ellipse a b N)

    ;; This test with two-dimensional ellipses stands in for all
    ;; dimensions, as the code to generate points is independent
    ;; of dimension

    (define pi (* 4 (atan 1)))

    (define interior-points
      (generator->list (gtake (make-ball-generator (vector a b)) N)))

    (define (in-ellipse? point)
      (< (+ (square (/ (vector-ref point 0) a))
            (square (/ (vector-ref point 1) b)))
         1))

    ;; Find points inside rectangles inside various parts
    ;; of the ellipse

    (define center
      (filter (lambda (v)
                (and (< (- (/ a 4))
                        (vector-ref v 0)
                        (/ a 4))
                     (< (- (/ b 4))
                        (vector-ref v 1)
                        (/ b 4))))
              interior-points))

    (define right-x
      ;; (right-x b/4) is on the ellipse
      (* a 0.9682458365518543))  ; (sqrt 15/16)

    (define right
      (filter (lambda (v)
                (and (< (- right-x (/ a 2))
                        (vector-ref v 0)
                        right-x)
                     (< (- (/ b 4))
                        (vector-ref v 1)
                        (/ b 4))))
              interior-points))

    (define top-y
      ;; (a/4 top-y) is on the ellipse
      (* b 0.9682458365518543)) ; (sqrt 15/16)

    (define top
      (filter (lambda (v)
                (and (< (- (/ a 4))
                        (vector-ref v 0)
                        (/ a 4))
                     (< (- top-y (/ b 2))
                        (vector-ref v 1)
                        top-y)))
              interior-points))

    ;; p is he fraction of the area in the ellipse
    ;; contained in the rectangles (it's all the same).

    #;(define p
    (/ (* (/ a 2) (/ b 2))
    (* pi a b)))

    (define p (/ (* 4 pi)))

    ;; check that all the points are truly inside the ellipse.

    (test-assert (every in-ellipse? interior-points))

    (for-each (lambda (p m)
    ;; p = probability of landing in rectangle,
    ;; m = measured number of events
    (test-assert (< (abs (- (* p N) m))
                    (* STDs (sqrt (* N p (- 1 p)))))))
  (list p p p)
  (map length (list center right top))))
  
(reset-source!*)
(cond-expand
  (gambit
  ;; Gambit fails this test, but I think the generator code is correct.
  #t)
  (else
  (test-sphere (make-sphere-generator 1) (vector 1.0 1.0) 200 #t)))
(test-sphere (make-sphere-generator 2) (vector 1.0 1.0 1.0) 200 #t)
(test-sphere (make-sphere-generator 3) (vector 1.0 1.0 1.0 1.0) 200 #t)

(reset-source!*)
(cond-expand
  (gambit
  ;; Gambit fails this test, but I think the generator code is correct.
  #t)
  (else
  (test-sphere (make-ellipsoid-generator (vector 1.0 1.0)) (vector 1.0 1.0) 200 #t)))
(test-sphere (make-ellipsoid-generator (vector 1.0 1.0 1.0)) (vector 1.0 1.0 1.0) 200 #t)
(test-sphere (make-ellipsoid-generator (vector 1.0 1.0 1.0 1.0)) (vector 1.0 1.0 1.0 1.0) 200 #t)

(reset-source!*)
(test-sphere (make-ellipsoid-generator (vector 1.0 3.0)) (vector 1.0 3.0) 200 #f)
(test-sphere (make-ellipsoid-generator (vector 1.0 3.0 5.0)) (vector 1.0 3.0 5.0) 200 #f)
(test-sphere (make-ellipsoid-generator (vector 1.0 3.0 5.0 7.0)) (vector 1.0 3.0 5.0 7.0) 200 #f)

(reset-source!*)
(test-ball (make-ball-generator 2) (vector 1.0 1.0))
(test-ball (make-ball-generator 3) (vector 1.0 1.0 1.0))
(test-ball (make-ball-generator (vector 1.0 3.0)) (vector 1.0 3.0))
(test-ball (make-ball-generator (vector 1.0 3.0 5.0)) (vector 1.0 3.0 5.0))

(reset-source!*)

(test-ellipsoid 1 1 10000)
(test-ellipsoid 5 5 10000)
(test-ellipsoid 10 2 10000)

;; test the unit ball
(test-ellipse 1 1 10000)

;; test the scaled unit ball
(test-ellipse 5 5 10000)

;; test an eccentric ellipse
(test-ellipse 10 2 10000))

(test-group "Test binomial"
            (reset-source!)
            (define (factorial n)
              (if (<= n 1)
                  1
                  (* n (factorial (- n 1)))))
            (define (C n k)
              (/ (factorial n)
                 (* (factorial k) (factorial (- n k)))))
            (define (expected-frac n p k)
              (* (C n k) (expt p k) (expt (- 1 p) (- n k))))

            (define (test-binomial n p count)
              (define g (make-binomial-generator n p))
              (define counts (make-vector (+ n 1) 0))
              (generator-for-each
                (lambda (x)
                  (vector-set! counts x (+ 1 (vector-ref counts x))))
                (gtake g count))
              (for-each
                (lambda (k)
                  (define expected (* count (expected-frac n p k) ))
                  (define actual (vector-ref counts k))
                  (cond
                    ((= expected 0)
                     (test-equal 0 actual))
                    ;;hacky.. testing values with very low probability fails
                    ((> expected (* 0.001 count)) ; 1/1000
                     (test-approximate 1.0 (/ actual expected) 0.2))))
                (iota (+ n 1))))

            (test-binomial 1 0 100)
            (test-binomial 1 1 100)
            (test-binomial 1 0. 100)
            (test-binomial 1 1. 100)
            (test-binomial 10 0 100)
            (test-binomial 10 1 100)
            (test-binomial 10 0. 100)
            (test-binomial 10 1. 100)
            (test-binomial 10 0.25 100000)
            (test-binomial 40 0.375 1000000))

(test-end)

