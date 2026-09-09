## (skint fx) — fixnum arithmetic

SKINT's built-in fixnum operations, gathered into one library. Most of them are also
exported by `(srfi 143)`, which adds the constants and the derived bit operations
this library does not have; the ones here are the primitives, plus a handful that
SRFI 143 does not describe — the floor and Euclidean division operators, the raw
shifts, the carry primitives, and `fxfmar`.

```scheme
(import (skint fx))
```

These are the same procedures the interaction environment already exposes, so a
script does not need this import to call `fx+` — the library exists so that programs
and libraries, which start with nothing imported, can ask for them by name.

### Fixnums

A *fixnum* is an exact integer that fits in the fixnum range, which is *w* bits
wide: the closed interval from −2^(*w*−1) to 2^(*w*−1)−1. The width is not part
of this library; `(srfi 143)` binds it as `fx-width`, along with `fx-greatest`
and `fx-least`.

Every procedure here takes fixnums and, where it returns a number, returns a fixnum.
It is an error if an argument is not a fixnum.

### Overflow wraps

SRFI 143 leaves what happens on overflow open: an implementation may signal, may
wrap around, or may fail outright. SKINT wraps. The arithmetic, shift and division
procedures reduce their result modulo 2^*w* back into the fixnum range — the
two's-complement wraparound a machine register of that width would perform.

That makes the fixnum operations a ring. Addition, subtraction and multiplication
are exact modulo 2^*w*, so an expression built from them gives the same answer as
the corresponding exact computation reduced at the end, however far the intermediate
values strayed outside the range:

```scheme
(fx+ fx-greatest 1)             ; => fx-least
(fx+ fx-greatest fx-greatest)   ; => -2
(fx* fx-greatest fx-greatest)   ; => 1
(fxsll 1 (- fx-width 1))        ; => fx-least
```

The same rule explains the boundary cases at the least fixnum, whose magnitude is
one greater than the largest fixnum: `(fxneg fx-least)`, `(fxabs fx-least)` and
`(fxquotient fx-least -1)` are all `fx-least`.

`fxexpt` is the one exception: instead of wrapping it reports the overflow, by
returning `0`. Its entry below explains why. Code that needs to detect an overflow
in any of the others rather than absorb it should use the carry procedures, which
keep the whole result in two fixnums.

### Predicates

`(fixnum? obj)` → *boolean*

Is `obj` an exact integer within the fixnum range? Unlike the rest of the library
this accepts any object.

`(fxzero? i)` → *boolean*
<br>`(fxpositive? i)` → *boolean*
<br>`(fxnegative? i)` → *boolean*
<br>`(fxodd? i)` → *boolean*
<br>`(fxeven? i)` → *boolean*

As `zero?`, `positive?`, `negative?`, `odd?` and `even?`.

`(fx=? i j ...)` → *boolean*
<br>`(fx<? i j ...)` → *boolean*
<br>`(fx>? i j ...)` → *boolean*
<br>`(fx<=? i j ...)` → *boolean*
<br>`(fx>=? i j ...)` → *boolean*

As `=`, `<`, `>`, `<=` and `>=`. Two or more arguments; the comparison chains, so
`(fx<? 1 2 3)` is true.

`(fx!=? i j)` → *boolean*

The negation of `fx=?`, for exactly two arguments.

### Arithmetic

`(fx+ i ...)` → *fixnum*
<br>`(fx* i ...)` → *fixnum*

Sum and product. With no arguments they return the corresponding identity, `0` and
`1`.

`(fx- i j ...)` → *fixnum*
<br>`(fx/ i j ...)` → *fixnum*

Difference and quotient, associating to the left. With one argument they return the
additive inverse and the reciprocal. `fx/` is exact division: it is an error if any
of the divisions leaves a remainder.

```scheme
(fx+ 1 2 3 4)     ; => 10
(fx+)             ; => 0
(fx- 10 1 2)      ; => 7
(fx- 10)          ; => -10
(fx/ 12 3)        ; => 4
```

`(fxneg i)` → *fixnum*
<br>`(fxabs i)` → *fixnum*

Negation and absolute value. Both return the least fixnum unchanged, since its
magnitude is not itself a fixnum.

`(fxmin i j ...)` → *fixnum*
<br>`(fxmax i j ...)` → *fixnum*

Smallest and largest argument. One or more arguments.

`(fxgcd i ...)` → *fixnum*

Greatest common divisor of the absolute values, so the mathematically correct result
is never negative. With no arguments it returns `0`.

`(fxexpt i j)` → *fixnum*

`i` raised to the power `j`. It is an error if `j` is negative.

Alone among the arithmetic procedures, `fxexpt` does not wrap when the result is
not a fixnum. It returns `0` instead. Zero is also the honest answer when `i` is
zero, so a zero result means overflow whenever `i` is not — and that is the test to
make:

```scheme
(let ([r (fxexpt i j)])
  (if (and (fxzero? r) (not (fxzero? i)))
      (overflowed)
      r))
```

The asymmetry is deliberate. Exponentiation overflows readily — for a base of 2 or
more it takes only a modest exponent — while a zero result is rare, because it
needs a zero base, so `0` is a signal that costs almost nothing in false alarms.
Checking the value afterwards is one comparison. Deciding beforehand whether `i`
raised to `j` will fit means doing most of the work of computing it, since there is
no cheap bound to test. So `fxexpt` puts the check where it is affordable, and
wrapping — which would destroy the evidence — would take that away.

### Division

Three pairs of operators, differing in how the quotient rounds and hence in the sign
of the remainder. In every case the quotient and remainder satisfy
`i = q*j + r`, and it is an error if `j` is zero.

`(fxquotient i j)` → *fixnum*
<br>`(fxremainder i j)` → *fixnum*

Truncated division: the quotient rounds toward zero, so the remainder takes the sign
of `i`.

`(fxmodquo i j)` → *fixnum*
<br>`(fxmodulo i j)` → *fixnum*

Floor division: the quotient rounds toward negative infinity, so the remainder takes
the sign of `j`.

`(fxeucquo i j)` → *fixnum*
<br>`(fxeucrem i j)` → *fixnum*

Euclidean division: the remainder is never negative.

```scheme
(fxquotient -13 4)  (fxremainder -13 4)   ; => -3   -1
(fxmodquo   -13 4)  (fxmodulo    -13 4)   ; => -4    3
(fxeucquo   -13 4)  (fxeucrem    -13 4)   ; => -4    3

(fxquotient 13 -4)  (fxremainder 13 -4)   ; => -3    1
(fxmodquo   13 -4)  (fxmodulo    13 -4)   ; => -4   -3
(fxeucquo   13 -4)  (fxeucrem    13 -4)   ; => -3    1
```

All three agree when both arguments are positive.

### Square root

`(%fxsqrt i)` → *fixnum*
<br>`(%fxsqrt i box)` → *fixnum*

The largest fixnum whose square does not exceed `i`. Given a second argument, it
also stores `i` minus the square of that root into the box, so that the root and the
stored remainder together reconstruct `i`. It is an error if `i` is negative.

The box is an ordinary box, made with `box` and read back with `unbox`, both from
`(scheme box)`:

```scheme
(import (scheme box))

(let* ([rem  (box 0)]
       [root (%fxsqrt 32 rem)])
  (list root (unbox rem)))                ; => (5 7)
```

Note the `let*`. The order in which the arguments of a call are evaluated is
unspecified, so read the box in a step that is sequenced after the call, never in
the same argument list.

`(srfi 143)` wraps this as `fxsqrt`, which returns the root and remainder as two
values instead.

### Bitwise operations

Each argument is treated as a two's-complement integer of the fixnum width.

`(fxnot i)` → *fixnum*

Bitwise complement.

`(fxand i ...)` → *fixnum*
<br>`(fxior i ...)` → *fixnum*
<br>`(fxxor i ...)` → *fixnum*
<br>`(fxeqv i ...)` → *fixnum*

Bitwise and, inclusive or, exclusive or, and equivalence — `fxeqv` being the
complement of `fxxor`. With no arguments each returns its identity: `-1` for `fxand`
and `fxeqv`, `0` for `fxior` and `fxxor`.

`(fxsll i count)` → *fixnum*
<br>`(fxsra i count)` → *fixnum*
<br>`(fxsrl i count)` → *fixnum*

Shift left, shift right arithmetic, and shift right logical. `fxsra` propagates the
sign bit; `fxsrl` shifts in zeros from the top of the fixnum width, so a negative
argument becomes a large positive result. It is an error if `count` is negative or
is not less than the fixnum width.

```scheme
(fxsll 1 4)      ; => 16
(fxsra -256 4)   ; => -16
(fxsrl -256 4)   ; => a large positive fixnum
```

`(fxlength i)` → *fixnum*

The number of bits needed to represent `i`, not counting a sign bit; for a negative
argument, the number needed to represent its complement.

`(fxbit-count i)` → *fixnum*

For a non-negative argument, the number of one bits. For a negative argument, the
number of zero bits, so `(fxbit-count -1)` is `0`.

### Carry operations

`(fxaddc i j box)` → *fixnum*
<br>`(fxsubc i j box)` → *fixnum*
<br>`(fxmulc i j box)` → *fixnum*

Add, subtract or multiply in a width wider than a fixnum, and split the exact result
into two fixnums. The box carries a fixnum in and a fixnum out: on entry it holds a
third operand, added to the sum, subtracted from the difference, or added to the
product; on exit it holds the low part of the result. The high part is returned.

The split is balanced, so the low part left in the box may be negative:

```scheme
(let* ([c    (box 0)]
       [high (fxmulc 2 3 c)])
  (list high (unbox c)))                  ; => (0 6)

(let* ([c    (box 0)]
       [high (fxmulc fx-greatest fx-greatest c)])
  (list high (unbox c)))                  ; => (268435455 1) at fx-width 30
```

These are the one place where an overflow is reported rather than absorbed, so they
are the way to multiply or add exactly at the top of the fixnum range.

`(srfi 143)` wraps these as `fx+/carry`, `fx-/carry` and `fx*/carry`, which take the
third operand directly and return both parts as two values.

### Fused multiply-add-remainder

`(fxfmar i j k m)` → *fixnum*

The remainder of `(i*j + k)` divided by `m`, with the multiplication and addition
carried out in a width wider than a fixnum. The intermediate product is therefore
not subject to the fixnum rule and no bits are lost, which is what makes this usable
as the inner step of modular arithmetic over the whole fixnum range:

```scheme
(fxfmar 7 6 5 10)     ; => 7,  the remainder of 47 divided by 10
```

The remainder is the truncated one, so it takes the sign of `(i*j + k)`. It is an
error if `m` is zero.

### Conversion

`(fixnum->flonum i)` → *flonum*

The flonum nearest to `i`. The opposite conversion, `flonum->fixnum`, is in
[(skint fl)](fl.md).

### Relation to (srfi 143)

`(srfi 143)` is the portable interface and is the better default. It exports most of
what is here under the same names, adds the fixnum-range constants, and supplies the
operations this library lacks — `fxsquare`, `fxsqrt`, `fx+/carry` and its siblings,
`fxarithmetic-shift` and the whole family of single-bit and bit-field procedures.

Reach for `(skint fx)` when you want the primitives without that layer, or when you
want one of the operations SRFI 143 does not describe: `fx/`, `fx!=?`, `fxgcd`,
`fxexpt`, `fxeqv`, the floor and Euclidean division operators, the raw shifts, the
carry primitives in their box form, or `fxfmar`.
