(import (skint fl))

(include "test.scm")

;; ---------------------------------------------------------------------------
;; Helpers
;; ---------------------------------------------------------------------------

(define posinf (fl/ 1.0 0.0))
(define neginf (fl/ -1.0 0.0))
(define nan (fl- posinf posinf))
(define negzero (fl* -1.0 0.0))


(display "\n--- predicates ---\n")

(test #t (flonum? 1.0))
(test #f (flonum? 1))
(test #f (flonum? "1.0"))
(test #t (flonum? posinf))
(test #t (flonum? nan))

(test #t (fl=? 1.0 1.0 1.0))
(test #f (fl=? 1.0 2.0 2.0))
(test #t (fl<? 1.0 2.0 3.0))
(test #f (fl<? 1.0 1.0 2.0))
(test #t (fl>? 3.0 2.0 1.0))
(test #f (fl>? 2.0 1.0 1.0))
(test #t (fl<=? 1.0 1.0 2.0))
(test #f (fl<=? 1.0 2.0 1.0))
(test #t (fl>=? 2.0 1.0 1.0))
(test #f (fl>=? 1.0 2.0 1.0))

(test "fl!=? is the negation of fl=?" #t (fl!=? 1.0 2.0))
(test #f (fl!=? 2.0 2.0))

(test #t (flzero? 0.0))
(test #t (flzero? negzero))
(test #f (flzero? 1.0))
(test #t (flpositive? 1.0))
(test #f (flpositive? 0.0))
(test #f (flpositive? -1.0))
(test #t (flnegative? -1.0))
(test #f (flnegative? 0.0))
(test "negative zero is not negative" #f (flnegative? negzero))

(test #t (flinteger? 3.0))
(test #f (flinteger? 3.5))
(test #f (flinteger? posinf))
(test #f (flinteger? nan))
(test #t (flodd? 3.0))
(test #f (flodd? 4.0))
(test #t (fleven? 4.0))
(test #f (fleven? 3.0))

(test #t (flnan? nan))
(test #f (flnan? 1.0))
(test #t (flfinite? 1.0))
(test #f (flfinite? posinf))
(test #f (flfinite? nan))
(test #t (flinfinite? posinf))
(test #t (flinfinite? neginf))
(test #f (flinfinite? 1.0))
(test #f (flinfinite? nan))


(display "\n--- basic arithmetic ---\n")

(test 6.0 (fl+ 1.0 2.0 3.0))
(test "fl+ with no arguments is the additive identity" 0.0 (fl+))
(test 1.0 (fl+ 1.0))
(test "fl* with no arguments is the multiplicative identity" 1.0 (fl*))
(test 24.0 (fl* 2.0 3.0 4.0))
(test "fl- with one argument negates" -3.0 (fl- 3.0))
(test "fl- associates to the left" 7.0 (fl- 10.0 1.0 2.0))
(test "fl/ with one argument is the reciprocal" 0.5 (fl/ 2.0))
(test 2.0 (fl/ 12.0 3.0 2.0))
(test -3.0 (flneg 3.0))
(test 7.0 (flabs -7.0))
(test 7.0 (flabs 7.0))

(test "flmin with no arguments is positive infinity" #t (flinfinite? (flmin)))
(test #t (flpositive? (flmin)))
(test "flmax with no arguments is negative infinity" #t (flinfinite? (flmax)))
(test #t (flnegative? (flmax)))
(test 1.0 (flmin 3.0 1.0 2.0))
(test 3.0 (flmax 3.0 1.0 2.0))
(test 3.0 (flmin 3.0))

(test "flgcd with no arguments is zero" 0.0 (flgcd))
(test 6.0 (flgcd 12.0 18.0))
(test 2.0 (flgcd 12.0 18.0 8.0))

(test 1024.0 (flexpt 2.0 10.0))
(test~= 1.4142135623730951 (flsqrt 2.0))
(test 4.0 (flsqrt 16.0))
(test "flsqrt of negative zero is negative zero" #t (flnegative? (flcopysign 1.0 (flsqrt negzero))))


(display "\n--- rounding ---\n")

(test 2.0 (flfloor 2.3))
(test -2.0 (flfloor -1.2))
(test -1.0 (flceiling -1.2))
(test 3.0 (flceiling 2.3))
(test -1.0 (fltruncate -1.7))
(test 1.0 (fltruncate 1.7))
(test "flround rounds halfway cases to even" 0.0 (flround 0.5))
(test 2.0 (flround 1.5))
(test 2.0 (flround 2.5))
(test 4.0 (flround 3.5))
(test 2.0 (flround 2.4))


(display "\n--- integer division ---\n")

(test -3.0 (flquotient -13.0 4.0))
(test -1.0 (flremainder -13.0 4.0))
(test 3.0 (flquotient 13.0 4.0))
(test 1.0 (flremainder 13.0 4.0))
(test "flmodquo is the floor quotient" -4.0 (flmodquo -13.0 4.0))
(test "flmodulo follows the sign of the divisor" 3.0 (flmodulo -13.0 4.0))
(test -4.0 (flmodquo 13.0 -4.0))
(test -3.0 (flmodulo 13.0 -4.0))


(display "\n--- exponents, logarithms and trigonometry ---\n")

(test 1.0 (flexp 0.0))
(test~= 2.718281828459045 (flexp 1.0))
(test~= 4.605170185988092 (fllog 100.0))
(test 0.0 (fllog 1.0))
(test "a second argument to fllog is the base" 2.0 (fllog 100.0 10.0))
(test 3.0 (fllog 8.0 2.0))
(test 2.0 (fllog10 100.0))

(test 0.0 (flsin 0.0))
(test 1.0 (flcos 0.0))
(test 0.0 (fltan 0.0))
(test 0.0 (flasin 0.0))
(test~= 1.5707963267948966 (flacos 0.0))
(test~= 0.7853981633974483 (flatan 1.0))
(test "a second argument to flatan selects the quadrant"
  #t (fl=? (flatan 1.0 1.0) (flatan 1.0)))
(test~= 2.356194490192345 (flatan 1.0 -1.0))

(test 0.0 (flsinh 0.0))
(test 1.0 (flcosh 0.0))
(test 0.0 (fltanh 0.0))


(display "\n--- decomposition ---\n")

(test "flldexp scales by a power of two" 12.0 (flldexp 1.5 3))
(test 1.5 (flldexp 1.5 0))
(test 0.75 (flldexp 1.5 -1))

(test "flmodf returns the fraction and boxes the integral part"
  '(0.75 3.0)
  (let* ([ip 0.0] [fr (flmodf 3.75 (set& ip))]) (list fr ip)))

(test "both parts of flmodf carry the sign"
  '(-0.75 -3.0)
  (let* ([ip 0.0] [fr (flmodf -3.75 (set& ip))]) (list fr ip)))

(test "flfrexp returns the fraction and boxes the exponent"
  '(0.75 4)
  (let* ([e 0] [fr (flfrexp 12.0 (set& e))]) (list fr e)))

(test "the fraction is in [0.5, 1.0)"
  #t
  (let* ([e 0] [fr (flfrexp 12.0 (set& e))])
    (and (fl<=? 0.5 fr) (fl<? fr 1.0))))

(test "flfrexp and flldexp are inverses"
  12.0
  (let* ([e 0] [fr (flfrexp 12.0 (set& e))]) (flldexp fr e)))


(display "\n--- conversion ---\n")

(test "flonum->fixnum truncates toward zero" 3 (flonum->fixnum 3.7))
(test -3 (flonum->fixnum -3.7))
(test 0 (flonum->fixnum 0.0))
(test #t (fixnum? (flonum->fixnum 3.0)))


(cond-expand
  (c99-math

(display "\n--- c99-math: sign and adjacency ---\n")

(test -3.0 (flcopysign 3.0 -1.0))
(test 3.0 (flcopysign 3.0 1.0))
(test 3.0 (flcopysign -3.0 1.0))
(test "the sign of the second argument wins even for zero"
  #t (fl<? (flcopysign 1.0 negzero) 0.0))

(test 0 (flsign-bit 1.0))
(test 1 (flsign-bit -1.0))
(test "negative zero has its sign bit set" 1 (flsign-bit negzero))
(test 0 (flsign-bit 0.0))
(test 1 (flsign-bit neginf))

(test "fladjacent steps toward the second argument"
  #t (fl>? (fladjacent 1.0 2.0) 1.0))
(test #t (fl<? (fladjacent 1.0 0.0) 1.0))
(test "no step is taken when the arguments are equal" 1.0 (fladjacent 1.0 1.0))
(test "the step is the smallest one representable"
  #t (fl=? 1.0 (fladjacent (fladjacent 1.0 2.0) 0.0)))

(test #t (flnormalized? 1.0))
(test #f (flnormalized? 0.0))
(test #f (fldenormalized? 1.0))
(test #f (fldenormalized? 0.0))

(display "\n--- c99-math: exponent ---\n")

(test "flexponent is the exponent as a flonum" 3.0 (flexponent 8.0))
(test 0.0 (flexponent 1.0))
(test -1.0 (flexponent 0.5))
(test "flilogb is the same value as an exact integer" 3 (flilogb 8.0))
(test 0 (flilogb 1.0))
(test #t (fixnum? (flilogb 8.0)))

(display "\n--- c99-math: arithmetic ---\n")

(test "fl+* multiplies and adds with a single rounding" 10.0 (fl+* 2.0 3.0 4.0))
(test 0.0 (fl+* 0.0 0.0 0.0))
(test "flposdiff clamps a negative difference to zero" 0.0 (flposdiff 3.0 5.0))
(test 2.0 (flposdiff 5.0 3.0))
(test 0.0 (flposdiff 1.0 1.0))

(test 1024.0 (flexp2 10.0))
(test 1.0 (flexp2 0.0))
(test "flexp-1 is accurate near zero" 0.0 (flexp-1 0.0))
(test~= 1.718281828459045 (flexp-1 1.0))
(test 3.0 (flcbrt 27.0))
(test -3.0 (flcbrt -27.0))
(test 5.0 (flhypot 3.0 4.0))
(test 0.0 (fllog1+ 0.0))
(test~= 0.6931471805599453 (fllog1+ 1.0))
(test 3.0 (fllog2 8.0))
(test 0.0 (fllog2 1.0))

(test 0.0 (flasinh 0.0))
(test 0.0 (flacosh 1.0))
(test 0.0 (flatanh 0.0))
(test~= 0.881373587019543 (flasinh 1.0))

(display "\n--- c99-math: special functions ---\n")

(test "flgamma of n is (n-1)!" 24.0 (flgamma 5.0))
(test 1.0 (flgamma 1.0))
(test 6.0 (flgamma 4.0))
(test "fllgamma is the log of the absolute value" 0.0 (fllgamma 1.0))
(test~= 3.1780538303479458 (fllgamma 5.0))
(test "fllgamma is defined where flgamma is negative"
  #t (flpositive? (fllgamma -0.5)))

(test 0.0 (flerf 0.0))
(test 1.0 (flerfc 0.0))
(test "flerf and flerfc sum to one" #t (fl=? 1.0 (fl+ (flerf 0.5) (flerfc 0.5))))

(display "\n--- c99-math: %flremquo ---\n")

(test "the remainder is the rounded one, and the box gets the quotient bits"
  '(1.0 3)
  (let* ([q 0] [r (%flremquo 13.0 4.0 (set& q))]) (list r q)))

(test "the quotient is signed"
  -3
  (let* ([q 0] [r (%flremquo -13.0 4.0 (set& q))]) q))

(test "rounding is to nearest, so the remainder may be negative"
  #t
  (let* ([q 0] [r (%flremquo 15.0 4.0 (set& q))]) (flnegative? r)))

  )
  (else
    (display "\n--- c99-math not present: those tests skipped ---\n")))


(cond-expand
  (xsi-math

(display "\n--- xsi-math: Bessel functions ---\n")

(test~= 0.7651976865579665 (flfirst-bessel 0 1.0))
(test~= 0.44005058574493355 (flfirst-bessel 1 1.0))
(test~= 0.08825696421567694 (flsecond-bessel 0 1.0))
(test "the order is an exact integer" #t (flonum? (flfirst-bessel 2 1.0)))

  )
  (else
    (display "\n--- xsi-math not present: those tests skipped ---\n")))


(display "\n--- All tests complete. ---\n")

(test-end)
