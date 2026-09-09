## (skint fl) — flonum arithmetic

SKINT's built-in flonum operations, gathered into one library. Most of them are also
exported by `(srfi 144)`, which adds the mathematical constants and the derived
procedures this library does not have; the ones here are the primitives, plus a few
that SRFI 144 does not describe — the floor division operators, `flgcd`, and the
decomposition procedures in their box form.

```scheme
(import (skint fl))
```

Part of the library is conditional. The procedures that stand for C99 `<math.h>`
functions are exported only where the host provides them, and the two Bessel
functions only where the host provides the XSI extensions as well. See
[Conditional exports](#conditional-exports).

### Flonums

A *flonum* is an inexact real number in the implementation's floating-point
representation. It is an error if an argument to any of these procedures is not a
flonum, except where noted; in particular an exact `2` is not a flonum and `2.0` is.

Where the mathematically correct result is not a real number, the result is a NaN.
Nothing here signals on overflow, underflow, or a domain error — the IEEE result
stands, so `(fl/ 1.0 0.0)` is an infinity rather than a complaint.

### Predicates

`(flonum? obj)` → *boolean*

Is `obj` a flonum? Unlike the rest of the library this accepts any object.

`(flzero? x)` → *boolean*
<br>`(flpositive? x)` → *boolean*
<br>`(flnegative? x)` → *boolean*
<br>`(flodd? x)` → *boolean*
<br>`(fleven? x)` → *boolean*

As `zero?`, `positive?`, `negative?`, `odd?` and `even?`. Negative zero is zero and
is not negative. It is an error if the argument to `flodd?` or `fleven?` is not an
integral flonum.

`(flinteger? x)` → *boolean*
<br>`(flnan? x)` → *boolean*
<br>`(flfinite? x)` → *boolean*
<br>`(flinfinite? x)` → *boolean*

Is `x` an integer, a NaN, finite, or an infinity? A NaN is neither finite nor
infinite.

`(fl=? x y ...)` → *boolean*
<br>`(fl<? x y ...)` → *boolean*
<br>`(fl>? x y ...)` → *boolean*
<br>`(fl<=? x y ...)` → *boolean*
<br>`(fl>=? x y ...)` → *boolean*

As `=`, `<`, `>`, `<=` and `>=`, following IEEE rules — a NaN compares false against
everything, itself included. Two or more arguments; the comparison chains.

`(fl!=? x y)` → *boolean*

The negation of `fl=?`, for exactly two arguments.

### Arithmetic

`(fl+ x ...)` → *flonum*
<br>`(fl* x ...)` → *flonum*

Sum and product. With no arguments they return the corresponding identity, `0.0` and
`1.0`.

`(fl- x y ...)` → *flonum*
<br>`(fl/ x y ...)` → *flonum*

Difference and quotient, associating to the left. With one argument they return the
additive and multiplicative inverse.

```scheme
(fl+ 1.0 2.0 3.0)   ; => 6.0
(fl- 10.0 1.0 2.0)  ; => 7.0
(fl- 3.0)           ; => -3.0
(fl/ 2.0)           ; => 0.5
```

`(flneg x)` → *flonum*
<br>`(flabs x)` → *flonum*

Negation and absolute value.

`(flmin x ...)` → *flonum*
<br>`(flmax x ...)` → *flonum*

Smallest and largest argument. With no arguments they return positive and negative
infinity respectively, which are the identities for these operations.

`(flgcd x ...)` → *flonum*

Greatest common divisor of integral flonums. With no arguments it returns `0.0`.

`(flexpt x y)` → *flonum*
<br>`(flsqrt x)` → *flonum*

`x` raised to the power `y`, and the square root of `x`. `flsqrt` of negative zero
is negative zero.

### Rounding

`(flfloor x)` → *flonum*
<br>`(flceiling x)` → *flonum*
<br>`(fltruncate x)` → *flonum*
<br>`(flround x)` → *flonum*

The nearest integral flonum below, above, toward zero, and nearest overall.
`flround` breaks a tie by rounding to even, so `(flround 0.5)` is `0.0` and
`(flround 1.5)` is `2.0`.

### Integer division

`(flquotient x y)` → *flonum*
<br>`(flremainder x y)` → *flonum*

Truncated division on integral flonums: the quotient rounds toward zero, so the
remainder takes the sign of `x`.

`(flmodquo x y)` → *flonum*
<br>`(flmodulo x y)` → *flonum*

Floor division: the quotient rounds toward negative infinity, so the remainder takes
the sign of `y`.

```scheme
(flquotient -13.0 4.0)  (flremainder -13.0 4.0)   ; => -3.0  -1.0
(flmodquo   -13.0 4.0)  (flmodulo    -13.0 4.0)   ; => -4.0   3.0
```

### Exponents and logarithms

`(flexp x)` → *flonum*

*e* raised to the power `x`.

`(fllog x)` → *flonum*
<br>`(fllog x base)` → *flonum*

The natural logarithm of `x`, or its logarithm in the given base.

`(fllog10 x)` → *flonum*

The base-10 logarithm of `x`.

### Trigonometric functions

`(flsin x)` → *flonum*
<br>`(flcos x)` → *flonum*
<br>`(fltan x)` → *flonum*
<br>`(flasin x)` → *flonum*
<br>`(flacos x)` → *flonum*

Sine, cosine, tangent, and the first two inverses.

`(flatan x)` → *flonum*
<br>`(flatan y x)` → *flonum*

Arc tangent. With two arguments, the arc tangent of `y/x` placed in the quadrant
selected by the signs of both, in the range from −π to π.

`(flsinh x)` → *flonum*
<br>`(flcosh x)` → *flonum*
<br>`(fltanh x)` → *flonum*

Hyperbolic sine, cosine and tangent.

### Decomposition

Two of these deliver a second result through a box — an ordinary box, made with
`box` and read back with `unbox`, both from `(scheme box)`.

`(flldexp x n)` → *flonum*

`x` multiplied by two raised to the exact integer `n`.

`(flmodf x box)` → *flonum*

Splits `x` into its fractional and integral parts. The fractional part is returned
and the integral part is stored into the box; both carry the sign of `x`.

`(flfrexp x box)` → *flonum*

Splits `x` into a fraction and an exponent. The fraction is returned, with absolute
value at least 0.5 and less than 1.0; the exponent is stored into the box as an
exact integer. Passing the two to `flldexp` reconstructs `x`.

```scheme
(import (scheme box))

(let* ([e  (box 0)]
       [fr (flfrexp 12.0 e)])
  (list fr (unbox e)))                    ; => (0.75 4)
```

Note the `let*`. The order in which the arguments of a call are evaluated is
unspecified, so read the box in a step that is sequenced after the call, never in
the same argument list.

`(srfi 144)` wraps these as `make-flonum`, `flinteger-fraction` and
`flnormalized-fraction-exponent`, which return both parts as two values instead.

### Conversion

`(flonum->fixnum x)` → *fixnum*

The fixnum obtained by truncating `x` toward zero. It is an error if that value is
not a fixnum. The opposite conversion, `fixnum->flonum`, is in
[(skint fx)](fx.md).

### Conditional exports

The remaining procedures stand for functions of the host's C library, and are
exported only where that library provides them. Two feature identifiers say which:
`c99-math` for the C99 `<math.h>` additions, and `xsi-math` for the XSI extensions
on top of those. Both appear in the value of `(features)` when present, so code that
must run on either kind of host can guard its use:

```scheme
(cond-expand
  (c99-math (flhypot 3.0 4.0))
  (else     (flsqrt (fl+ (fl* 3.0 3.0) (fl* 4.0 4.0)))))
```

Importing `(skint fl)` on a host without these features is not an error; the names
are simply not bound.

#### With the c99-math feature

`(flcopysign x y)` → *flonum*

The magnitude of `x` with the sign of `y`.

`(flsign-bit x)` → *fixnum*

`0` if the sign bit of `x` is clear and `1` if it is set, so negative zero gives `1`.

`(fladjacent x y)` → *flonum*

The flonum next to `x` in the direction of `y`, or `x` itself if the two are equal.
There is no flonum strictly between `x` and the result.

`(flnormalized? x)` → *boolean*
<br>`(fldenormalized? x)` → *boolean*

Is `x` a normalized number, or a subnormal one? Zero is neither.

`(flexponent x)` → *flonum*
<br>`(flilogb x)` → *fixnum*

The binary exponent of `x`, as a flonum and as an exact integer.

`(fl+* x y z)` → *flonum*

`x*y + z`, computed as if to infinite precision and rounded once.

`(flposdiff x y)` → *flonum*

The difference of `x` and `y` if that is non-negative, and zero otherwise.

`(flexp2 x)` → *flonum*
<br>`(flexp-1 x)` → *flonum*
<br>`(flcbrt x)` → *flonum*
<br>`(flhypot x y)` → *flonum*

Two raised to the power `x`; *e* raised to the power `x`, less one; the cube root of
`x`; and the length of the hypotenuse of a right triangle with sides `x` and `y`.
`flexp-1` is much more accurate than `flexp` for `x` near zero.

`(fllog1+ x)` → *flonum*
<br>`(fllog2 x)` → *flonum*

The natural logarithm of `x + 1`, and the base-2 logarithm of `x`. `fllog1+` is much
more accurate than `fllog` for `x` near zero.

`(flasinh x)` → *flonum*
<br>`(flacosh x)` → *flonum*
<br>`(flatanh x)` → *flonum*

Inverse hyperbolic sine, cosine and tangent.

`(flgamma x)` → *flonum*
<br>`(fllgamma x)` → *flonum*

The gamma function of `x`, and the natural logarithm of its absolute value.
`fllgamma` is defined over a much wider range than `flgamma`, which overflows
quickly, and it is finite where the gamma function is negative.

`(flerf x)` → *flonum*
<br>`(flerfc x)` → *flonum*

The error function and its complement, `1 - (flerf x)`.

`(%flremquo x y box)` → *flonum*

The remainder of `x` divided by `y` with the quotient rounded to nearest, which may
therefore be negative where `flremainder` would be positive. The low-order bits of
that quotient are stored into the box as a correctly signed exact integer. The
number of bits kept is implementation-dependent, which is enough to preserve the
quadrant or octant when reducing an argument for an inverse trigonometric function.

`(srfi 144)` wraps this as `flremquo`, which returns both parts as two values.

#### With the xsi-math feature

`(flfirst-bessel n x)` → *flonum*
<br>`(flsecond-bessel n x)` → *flonum*

The Bessel functions of the first and second kind of order `n` applied to `x`. `n`
is an exact integer.

### Relation to (srfi 144)

`(srfi 144)` is the portable interface and is the better default. It exports most of
what is here under the same names, adds the mathematical and implementation
constants, and supplies the operations this library lacks — `flonum`, `flsquare`,
`flabsdiff`, `flsgn`, `flnumerator`, `fldenominator`, `flunordered?`,
`make-fllog-base`, and the multiple-value forms of the decomposition procedures. It
also fills in the C99 additions with portable definitions on a host that lacks them,
so code written against it does not need the feature guards described above.

Reach for `(skint fl)` when you want the primitives without that layer, or when you
want one of the operations SRFI 144 does not describe: `fl!=?`, `flgcd`, `flneg`,
`flmodquo`, `flmodulo`, `flonum->fixnum`, or the decomposition procedures in their
box form.
