;;; complex.sld -- (scheme complex) as a real library, for compiled programs.
;;;
;;; Shadow counterpart of Skint's predefined (scheme complex).  The runtime
;;; has no complex numbers, and neither has a towerless Skint, so this
;;; follows what a towerless Skint does rather than inventing a policy.
;;; There the six are builtin instructions whose standard-build definitions
;;; are trivial: real-part returns its argument, imag-part returns an exact
;;; zero, magnitude is abs, angle is an exact zero or pi, and the two
;;; constructors accept an exact zero for the second argument and refuse
;;; anything else.  Each one checks that its argument is a number first.
;;;
;;; Written over (scheme base) alone, so it needs no primitive of its own
;;; and therefore nothing from the backend's prim table or from srt.  A
;;; compiled program reaches it through (srfi 128), which imports the
;;; library for real-part and imag-part in number-hash.  [cco]

(define-library (scheme complex)
  (import (scheme base))

  (export real-part imag-part magnitude angle
          make-rectangular make-polar)

  (begin
    ;; the message the constructors refuse with.  A result off the real
    ;; axis is not representable, which is a property of the runtime and
    ;; not of the arguments, so it is said that way.
    (define (no-complex who a b)
      (error (string-append who ": result cannot be represented")
             a b))

    (define (ckn who x)
      (if (number? x) x (error (string-append who ": not a number") x)))

    (define (real-part z) (ckn "real-part" z))

    (define (imag-part z) (ckn "imag-part" z) 0)

    (define (magnitude z) (abs (ckn "magnitude" z)))

    ;; an exact zero for a non-negative argument, pi for a negative one,
    ;; whatever the argument's own exactness: that is what the towerless
    ;; instruction does, and (angle 2.0) is 0 rather than 0.0 there
    (define (angle z)
      (if (negative? (ckn "angle" z)) 3.141592653589793 0))

    ;; the only imaginary part this runtime can represent is an exact zero
    (define (make-rectangular x y)
      (ckn "make-rectangular" x)
      (if (eqv? y 0) x (no-complex "make-rectangular" x y)))

    ;; likewise the only angle that leaves the result on the real axis
    (define (make-polar r theta)
      (ckn "make-polar" r)
      (if (eqv? theta 0) r (no-complex "make-polar" r theta)))
))
