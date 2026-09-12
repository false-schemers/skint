(import (skint fx))

(include "test.scm")

;; ---------------------------------------------------------------------------
;; Helpers
;; ---------------------------------------------------------------------------

;; the fixnum range, derived the same way (srfi 143) derives it
(define fx-width
  (do ([x 1 (fxsll x 1)] [n 0 (fx+ n 1)]) [(fxnegative? x) (fx+ n 1)]))

(define fx-greatest
  (let ([x (fxexpt 2 (fx- fx-width 2))]) (fx+ x (fx- x 1))))

(define fx-least (fx+ fx-greatest 1))

;; 2^(w-2), the largest power of two that is still a fixnum
(define fx-half (fxexpt 2 (fx- fx-width 2)))


(display "\n--- predicates ---\n")

(test #t (fixnum? 32767))
(test #f (fixnum? 1.1))
(test #t (fixnum? 0))
(test #f (fixnum? "0"))
(test #t (fixnum? fx-greatest))
(test #t (fixnum? fx-least))

(test #t (fx=? 1 1 1))
(test #f (fx=? 1 2 2))
(test #f (fx=? 1 1 2))
(test #f (fx=? 1 2 3))
(test #t (fx<? 1 2 3))
(test #f (fx<? 1 1 2))
(test #t (fx>? 3 2 1))
(test #f (fx>? 2 1 1))
(test #t (fx<=? 1 1 2))
(test #f (fx<=? 1 2 1))
(test #t (fx>=? 2 1 1))
(test #f (fx>=? 1 2 1))
(test '(#t #f) (list (fx<=? 1 1 2) (fx<=? 2 1 3)))

(test "fx!=? is the negation of fx=?" #t (fx!=? 1 2))
(test #f (fx!=? 2 2))

(test #t (fxzero? 0))
(test #f (fxzero? 1))
(test #f (fxpositive? 0))
(test #t (fxpositive? 1))
(test #f (fxpositive? -1))
(test #f (fxnegative? 0))
(test #f (fxnegative? 1))
(test #t (fxnegative? -1))
(test #f (fxodd? 0))
(test #t (fxodd? 1))
(test #t (fxodd? -1))
(test #f (fxodd? 102))
(test #t (fxeven? 0))
(test #f (fxeven? 1))
(test #t (fxeven? -2))
(test #t (fxeven? 102))


(display "\n--- basic arithmetic ---\n")

(test 7 (fx+ 3 4))
(test 12 (fx* 4 3))
(test -1 (fx- 3 4))
(test -3 (fxneg 3))
(test 7 (fxabs -7))
(test 7 (fxabs 7))

(test "fx+ folds over any number of arguments" 10 (fx+ 1 2 3 4))
(test "fx+ with no arguments is the additive identity" 0 (fx+))
(test 1 (fx+ 1))
(test "fx* with no arguments is the multiplicative identity" 1 (fx*))
(test 24 (fx* 2 3 4))
(test "fx- with one argument negates" -10 (fx- 10))
(test "fx- associates to the left" 7 (fx- 10 1 2))

(test "fx/ is exact division" 4 (fx/ 12 3))
(test 2 (fx/ 12 3 2))
(test -4 (fx/ 12 -3))

(test 4 (fxmax 3 4))
(test 5 (fxmax 3 5 4))
(test 3 (fxmin 3 4))
(test 3 (fxmin 3 5 4))
(test "fxmin of one argument is that argument" 3 (fxmin 3))
(test 9 (fxmax 1 9 4))

(test "fxgcd with no arguments is zero" 0 (fxgcd))
(test 6 (fxgcd 12 18))
(test 2 (fxgcd 12 18 8))
(test "fxgcd ignores signs" 6 (fxgcd -12 18))
(test 12 (fxgcd 12))

(test 1024 (fxexpt 2 10))
(test 1 (fxexpt 2 0))
(test -8 (fxexpt -2 3))
(test 1 (fxexpt 1 1000))
(test -1 (fxexpt -1 1001))

;; fxexpt does not wrap: it returns 0 when the result is not a fixnum, which is
;; distinguishable from a real zero result because that needs a zero base

(test "the largest representable power of two" fx-half (fxexpt 2 (fx- fx-width 2)))
(test "one step further does not fit, so zero comes back" 0 (fxexpt 2 (fx- fx-width 1)))
(test "but the same power of -2 does fit" fx-least (fxexpt -2 (fx- fx-width 1)))
(test 0 (fxexpt -2 fx-width))
(test 0 (fxexpt 3 100))
(test "a zero base also gives zero" 0 (fxexpt 0 3))
(test "so the overflow test has to exclude a zero base"
  (quote (#t #f))
  (list (and (fxzero? (fxexpt 3 100)) (not (fxzero? 3)))
        (and (fxzero? (fxexpt 0 3)) (not (fxzero? 0)))))


(display "\n--- overflow wraps modulo 2^fx-width ---\n")

(test "adding past the top wraps to the bottom" fx-least (fx+ fx-greatest 1))
(test "subtracting past the bottom wraps to the top" fx-greatest (fx- fx-least 1))
(test -2 (fx+ fx-greatest fx-greatest))
(test 0 (fx+ fx-least fx-least))
(test -1 (fx- fx-greatest fx-least))

(test "multiplication wraps too" 1 (fx* fx-greatest fx-greatest))
(test 0 (fx* fx-least fx-least))
(test fx-least (fx* fx-least -1))

(test "the least fixnum is its own negation" fx-least (fxneg fx-least))
(test "and its own absolute value" fx-least (fxabs fx-least))
(test "and its own quotient by -1" fx-least (fxquotient fx-least -1))
(test fx-least (fx/ fx-least -1))
(test fx-least (fx- 0 fx-least))

(test "shifting a bit off the top wraps" fx-least (fxsll 1 (fx- fx-width 1)))
(test -2 (fxsll fx-greatest 1))

;; the operations therefore form a ring modulo 2^fx-width: 4*2^(w-2) is 2^w,
;; which is congruent to zero, so adding it changes nothing

(test "2^fx-width is congruent to zero" 0 (fx* fx-half 4))
(test 0 (fx+ (fx* 2 fx-half) (fx* 2 fx-half)))
(test "adding a multiple of 2^fx-width leaves a value alone" 7 (fx+ (fx* fx-half 4) 7))
(test fx-half (fx* fx-half 5))

(test "an exact computation and a wrapped one agree modulo 2^fx-width"
  #t
  (fx=? (fx* (fx+ fx-greatest 3) 5)
        (fx+ (fx* fx-greatest 5) 15)))

(display "\n--- the three division operators ---\n")

;; truncated: the quotient rounds toward zero, the remainder follows the dividend
(test 2 (fxquotient 5 2))
(test -2 (fxquotient -5 2))
(test -2 (fxquotient 5 -2))
(test 2 (fxquotient -5 -2))
(test 1 (fxremainder 13 4))
(test -1 (fxremainder -13 4))
(test 1 (fxremainder 13 -4))
(test -1 (fxremainder -13 -4))

;; floor: the quotient rounds toward negative infinity, the remainder follows the divisor
(test -4 (fxmodquo -13 4))
(test 3 (fxmodulo -13 4))
(test -4 (fxmodquo 13 -4))
(test -3 (fxmodulo 13 -4))
(test 3 (fxmodquo 13 4))
(test 1 (fxmodulo 13 4))

;; euclidean: the remainder is never negative
(test -4 (fxeucquo -13 4))
(test 3 (fxeucrem -13 4))
(test -3 (fxeucquo 13 -4))
(test 1 (fxeucrem 13 -4))
(test 3 (fxeucquo 13 4))
(test 1 (fxeucrem 13 4))

(test "all three agree when the signs are positive"
  '(3 1 3 1 3 1)
  (list (fxquotient 13 4) (fxremainder 13 4)
        (fxmodquo 13 4) (fxmodulo 13 4)
        (fxeucquo 13 4) (fxeucrem 13 4)))


(display "\n--- %fxsqrt ---\n")

(test "one argument gives the root alone" 5 (%fxsqrt 32))
(test 0 (%fxsqrt 0))
(test 1 (%fxsqrt 1))
(test 6 (%fxsqrt 36))
(test 6 (%fxsqrt 48))

(test "the box receives the remainder"
  '(5 7)
  (let* ([rem 0] [root (%fxsqrt 32 (set& rem))]) (list root rem)))

(test "an exact square leaves a zero remainder"
  '(6 0)
  (let* ([rem -1] [root (%fxsqrt 36 (set& rem))]) (list root rem)))

(test "root and remainder reconstruct the argument"
  32
  (let* ([rem 0] [root (%fxsqrt 32 (set& rem))]) (fx+ (fx* root root) rem)))


(display "\n--- bitwise operations ---\n")

(test -1 (fxnot 0))
(test 0 (fxnot -1))
(test -11 (fxnot 10))
(test 36 (fxnot -37))

(test 0 (fxand #b0 #b1))
(test 6 (fxand 14 6))
(test 10 (fxand 11 26))
(test 4 (fxand 37 12))
(test #b110 (fxand -2 #b111))
(test "fxand with no arguments is all ones" -1 (fxand))
(test 14 (fxior 10 12))
(test 11 (fxior 3 10))
(test "fxior with no arguments is zero" 0 (fxior))
(test 6 (fxxor 10 12))
(test 9 (fxxor 3 10))
(test "fxxor with no arguments is zero" 0 (fxxor))

(test "fxeqv is the complement of fxxor" -7 (fxeqv 10 12))
(test "fxeqv with no arguments is all ones" -1 (fxeqv))
(test 10 (fxeqv 10))
(test -1 (fxeqv 5 5))

(test 16 (fxsll 1 4))
(test -16 (fxsll -1 4))
(test 1 (fxsll 1 0))
(test 16 (fxsra 256 4))
(test "fxsra propagates the sign" -16 (fxsra -256 4))
(test -1 (fxsra -1 1))
(test 16 (fxsrl 256 4))
(test "fxsrl shifts in zeros from the fixnum width"
  fx-greatest
  (fxsrl -1 1))

(test 0 (fxlength 0))
(test 1 (fxlength 1))
(test 0 (fxlength -1))
(test 3 (fxlength 7))
(test 3 (fxlength -7))
(test 4 (fxlength 8))
(test 3 (fxlength -8))
(test 8 (fxlength 128))
(test 8 (fxlength 255))
(test 9 (fxlength 256))
(test (fx- fx-width 1) (fxlength fx-least))
(test (fx- fx-width 1) (fxlength fx-greatest))

(test 2 (fxbit-count 12))
(test 0 (fxbit-count 0))
(test 1 (fxbit-count 1))
(test 1 (fxbit-count 2))
(test 2 (fxbit-count 3))
(test 8 (fxbit-count 255))
(test "for a negative argument fxbit-count counts the zero bits" 0 (fxbit-count -1))
(test 1 (fxbit-count -2))
(test 1 (fxbit-count -3))
(test 2 (fxbit-count -4))
(test 3 (fxbit-count -8))
(test 4 (fxbit-count -16))


(display "\n--- carry operations ---\n")

;; each takes the incoming carry in a box, returns the high part and
;; leaves the low part in the box

(test "fxaddc splits a sum that overflows"
  '(1 -2)
  (let* ([c 0] [q (fxaddc fx-greatest fx-greatest (set& c))]) (list q c)))

(test "fxaddc leaves a small sum in the box"
  '(0 8)
  (let* ([c 5] [q (fxaddc 1 2 (set& c))]) (list q c)))

(test "fxaddc of zeros is zero"
  '(0 0)
  (let* ([c 0] [q (fxaddc 0 0 (set& c))]) (list q c)))

(test "fxsubc leaves a small difference in the box"
  '(0 -1)
  (let* ([c 0] [q (fxsubc 1 2 (set& c))]) (list q c)))

(test "fxsubc splits a difference that underflows"
  (quote (-1 0))
  (let* ([c 1] [q (fxsubc fx-least fx-greatest (set& c))]) (list q c)))

(test "fxmulc splits a product that overflows"
  (list (fx- fx-half 1) 1)
  (let* ([c 0] [q (fxmulc fx-greatest fx-greatest (set& c))]) (list q c)))

(test "fxmulc leaves a small product in the box"
  '(0 6)
  (let* ([c 0] [q (fxmulc 2 3 (set& c))]) (list q c)))

(test "the incoming carry is added to the product"
  (quote (0 10))
  (let* ([c 4] [q (fxmulc 2 3 (set& c))]) (list q c)))

(test "the low part is balanced, so it may be negative"
  (quote (1 -4))
  (let* ([c 0] [q (fxmulc (fx- fx-half 1) 4 (set& c))]) (list q c)))


(display "\n--- fxfmar ---\n")

;; (x*y + z) remainder m, with the multiplication carried out in wider
;; arithmetic so intermediate overflow cannot lose bits

(test "the intermediate product is not truncated to a fixnum"
  921
  (fxfmar 536870911 536870911 0 1000))

(test 7 (fxfmar 7 6 5 10))
(test 0 (fxfmar 5 4 0 10))
(test "z is added before the remainder is taken" 7 (fxfmar 0 0 7 10))
(test 2 (fxfmar 3 4 0 10))

(test "the remainder is truncated, so it follows the sign of the dividend"
  -7 (fxfmar -7 6 5 10))
(test 7 (fxfmar 7 6 5 -10))
(test -7 (fxfmar -7 6 5 -10))

(test "it agrees with fxremainder when nothing overflows"
  #t
  (fx=? (fxfmar 3 4 5 7) (fxremainder (fx+ (fx* 3 4) 5) 7)))


(display "\n--- conversion ---\n")

(test 42.0 (fixnum->flonum 42))
(test 0.0 (fixnum->flonum 0))
(test -1.0 (fixnum->flonum -1))
(test #t (flonum? (fixnum->flonum 3)))


(display "\n--- All tests complete. ---\n")

(test-end)
