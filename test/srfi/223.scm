(import (srfi 223))
(include "test.scm")

(test-begin "srfi-223")

;; OEIS A003056
#|
(define (nth-inverted-triangular n)
  ;; The OEIS formula yields the sequence 0, 1, 2, 2, 3, 3, 3, ...
  ;; instead of 0, 1, 1, 2, 2, 2, 3, 3, 3, 3 ..., so this is
  ;; implemented in terms of A002024, which makes it correct.
  (- (floor (+ 1/2 (exact (sqrt (* 2 (+ n 1)))))) 1))
(define (inverted-triangular-ref _ n) (nth-inverted-triangular n))

(test-group "Generalized bisection procedures"
  (test "left bisection for n for the nth inverted triangular number ≥ x."
        3 (bisect-left #f 2 inverted-triangular-ref < 0 100))
  (test "right bisection for n for the nth inverted triangular number < x."
        6 (bisect-right #f 2 inverted-triangular-ref < 0 100))

  ;; test for integer overflow
  (test "left bisection for a massive sequence size"
        68719439628
        (bisect-left #f 370727 inverted-triangular-ref < 0 (expt 2 48)))
  (test "right bisection for a massive sequence size"
        68719810356
        (bisect-right #f 370727 inverted-triangular-ref < 0 (expt 2 48))))
|#

(test-group "Bisections over vectors"
  (test "left bisection over a vector"
        1 (vector-bisect-left #(1 2 2 3 5) 2 <))
  (test "right bisection over a vector"
        3 (vector-bisect-right #(1 2 2 3 5) 2 <)))

(test-group "Bisections over vectors of things other than numbers"
  (test "basic left bisection over a vector of characters"
        1 (vector-bisect-left #(#\A #\Z #\a #\z) #\B char<?))
  (test "basic right bisection over a vector of characters"
        2 (vector-bisect-right #(#\A #\Z #\a #\z) #\Z char<?)))

#| [esl-] requires Unicode support
;; These tests, meant to ensure that everything works properly on
;; larger vectors, may not work in Schemes which don’t provide full
;; Unicode support.
;;
;; The list of BMP blocks ends before the surrogate pairs because some
;; Schemes (arguably correctly) don’t allow unpaired surrogates as
;; character objects.
(define low-bmp-block-starts
  #(#\null #\x80 #\x100 #\x180 #\x250 #\x2B0 #\x300 #\x370 #\x400
    #\x500 #\x530 #\x590 #\x600 #\x700 #\x750 #\x780 #\x7C0 #\x800
    #\x840 #\x860 #\x8A0 #\x900 #\x980 #\xA00 #\xA80 #\xB00 #\xB80
    #\xC00 #\xC80 #\xD00 #\xD80 #\xE00 #\xE80 #\xF00 #\x1000 #\x10A0
    #\x1100 #\x1200 #\x1380 #\x13A0 #\x1400 #\x1680 #\x16A0 #\x1700
    #\x1720 #\x1740 #\x1760 #\x1780 #\x1800 #\x18B0 #\x1900 #\x1950
    #\x1980 #\x19E0 #\x1A00 #\x1A20 #\x1AB0 #\x1B00 #\x1B80 #\x1BC0
    #\x1C00 #\x1C50 #\x1C80 #\x1C90 #\x1CC0 #\x1CD0 #\x1D00 #\x1D80
    #\x1DC0 #\x1E00 #\x1F00 #\x2000 #\x2070 #\x20A0 #\x20D0 #\x2100
    #\x2150 #\x2190 #\x2200 #\x2300 #\x2400 #\x2440 #\x2460 #\x2500
    #\x2580 #\x25A0 #\x2600 #\x2700 #\x27C0 #\x27F0 #\x2800 #\x2900
    #\x2980 #\x2A00 #\x2B00 #\x2C00 #\x2C60 #\x2C80 #\x2D00 #\x2D30
    #\x2D80 #\x2DE0 #\x2E00 #\x2E80 #\x2F00 #\x2FF0 #\x3000 #\x3040
    #\x30A0 #\x3100 #\x3130 #\x3190 #\x31A0 #\x31C0 #\x31F0 #\x3200
    #\x3300 #\x3400 #\x4DC0 #\x4E00 #\xA000 #\xA490 #\xA4D0 #\xA500
    #\xA640 #\xA6A0 #\xA700 #\xA720 #\xA800 #\xA830 #\xA840 #\xA880
    #\xA8E0 #\xA900 #\xA930 #\xA960 #\xA980 #\xA9E0 #\xAA00 #\xAA60
    #\xAA80 #\xAAE0 #\xAB00 #\xAB30 #\xAB70 #\xABC0 #\xAC00 #\xD7B0))

(test-group "Bisections over larger vectors of non-numbers"
  (test "find the number of the Unicode block for schwa"
        4 (- (vector-bisect-right low-bmp-block-starts #\x259 char<?) 1))
  (test "find the number of the Unicode block for Devanagari long a"
        21 (- (vector-bisect-right low-bmp-block-starts #\x906 char<?) 1))
  (test "find the number of the Unicode block for em dash"
        71 (- (vector-bisect-right low-bmp-block-starts #\x2014 char<?) 1))
  (test "find the number of the Unicode block for Cherokee small he"
        144 (- (vector-bisect-right low-bmp-block-starts #\xAB7E char<?) 1)))
|#

(define-syntax receive
  (syntax-rules ()
    ((receive formals expression body ...)
     (call-with-values (lambda () expression)
                       (lambda formals body ...)))))

;[esl]+ ceiling-quotient is nonstandard; code below taken from MIT Scheme

(define (ceiling/ n d)
  (if (and (exact-integer? n) (exact-integer? d))
      (cond ((and (negative? n) (negative? d))
             (ceiling-/- n d))
            ((negative? n)
             (let ((n (- 0 n)))
               (values (- 0 (quotient n d)) (- 0 (remainder n d)))))
            ((negative? d)
             (let ((d (- 0 d)))
               (values (- 0 (quotient n d)) (remainder n d))))
            (else
             (ceiling+/+ n d)))
      (let ((q (ceiling (/ n d))))
        (values q (- n (* d q))))))

(define (ceiling-/- n d)
  (let ((n (- 0 n)) (d (- 0 d)))
    (let ((q (quotient n d)) (r (remainder n d)))
      (if (zero? r)
          (values q r)
          (values (+ q 1) (- d r))))))

(define (ceiling+/+ n d)
  (let ((q (quotient n d)) (r (remainder n d)))
    (if (zero? r)
        (values q r)
        (values (+ q 1) (- r d)))))

(define (ceiling-quotient n d)
  (if (and (exact-integer? n) (exact-integer? d))
      (cond ((and (negative? n) (negative? d))
             (receive (q r) (ceiling-/- n d) r q))
            ((negative? n) (- 0 (quotient (- 0 n) d)))
            ((negative? d) (- 0 (quotient n (- 0 d))))
            (else (receive (q r) (ceiling+/+ n d) r q)))
      (ceiling (/ n d))))

(define (ceiling-remainder n d)
  (if (and (exact-integer? n) (exact-integer? d))
      (cond ((and (negative? n) (negative? d))
             (receive (q r) (ceiling-/- n d) q r))
            ((negative? n) (- 0 (remainder (- 0 n) d)))
            ((negative? d) (remainder n (- 0 d)))
            (else (receive (q r) (ceiling+/+ n d) q r)))
      (- n (* d (ceiling (/ n d))))))

;[esl] end of ceiling-quotient


;; Example of a vector with negative indexes: the mvector accessor for
;; a vector guarantees that index 0 will always refer to the median
;; value in the vector, provided the vector is sorted.
(define-record-type <mvector>
  (vector->mvector vector)
  mvector?
  (vector mvector-vector mvector-vector-set!))

(define (mvector . vals)
  (vector->mvector (apply vector vals)))

(define (mvector-size mvector) (vector-length (mvector-vector mvector)))
(define (mvector-first-idx mvector)
  (- (floor-quotient (mvector-size mvector) 2)))
(define (mvector-last-idx mvector)
  (- (ceiling-quotient (mvector-size mvector) 2)
     1))
(define (mvector-ref mvector idx)
  (vector-ref (mvector-vector mvector)
              (+ (abs (mvector-first-idx mvector)) idx)))

(define-values (mvector-bisect-left mvector-bisect-right)
  (bisection mvector-ref
             (lambda (mv)
               (values (mvector-first-idx mv)
                       (+ 1 (mvector-last-idx mv))))))

(define mvtest-even (mvector 1 1 2 3 5 8 13 21))
(define mvtest-odd (mvector 1 1 2 3 5 8 13 21 34))

(test-group "Implementation of example sequence type with negative indexes"
  (test "first-idx of even-sized mvector" -4 (mvector-first-idx mvtest-even))
  (test "first-idx of odd-sized mvector" -4 (mvector-first-idx mvtest-odd))

  (test "last-idx of even-size mvector" 3 (mvector-last-idx mvtest-even))
  (test "last-idx of odd-size mvector" 4 (mvector-last-idx mvtest-odd))

  (test "min value of even-sized mvector"
        1 (mvector-ref mvtest-even (mvector-first-idx mvtest-even)))
  (test "min value of odd-sized mvector"
        1 (mvector-ref mvtest-odd (mvector-first-idx mvtest-odd)))

  (test "max value of even-sized mvector"
        21 (mvector-ref mvtest-even (mvector-last-idx mvtest-even)))
  (test "max value of odd-sized mvector"
        34 (mvector-ref mvtest-odd (mvector-last-idx mvtest-odd)))

  (test "median value of odd-sized mvector"
        5 (mvector-ref mvtest-odd 0))
  (test "median value of even-sized mvector"
        5 (mvector-ref mvtest-even 0)))

(test-group "Bisection of sequence type with negative indexes"
  (test "left bisection returning negative result"
        -2 (mvector-bisect-left mvtest-odd 2 <))
  (test "right bisection returning negative result"
        -1 (mvector-bisect-right mvtest-odd 2 <))

  (test "left bisection returning positive result"
        1 (mvector-bisect-left mvtest-odd 8 <))
  (test "right bisection returning positive result"
        2 (mvector-bisect-right mvtest-odd 9 <)))

(test-end)

