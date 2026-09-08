## (skint time) — points in time and durations

Time objects in the style of SRFI 19: a point in time, or a duration, held as a
whole number of seconds since an epoch plus a nanosecond fraction. Several time
scales are provided, with conversions between them, along with comparison and
arithmetic operations and access to the system clock.

```scheme
(import (skint time))
```

This library and [`(skint date)`](date.md) are the foundation a full `(srfi 19)`
will be built on; it covers the time half of that SRFI.

The library needs nothing but `(scheme base)` and `(scheme inexact)` on the
client side.

### Time types

Six constants name the scales a time object can be measured on. Each is a symbol.

| Constant | Meaning |
|---|---|
| `time-utc` | Coordinated Universal Time |
| `time-tai` | International Atomic Time |
| `time-monotonic` | A clock that does not step backwards |
| `time-duration` | An interval rather than an instant |
| `time-process` | CPU time consumed by the process |
| `time-thread` | CPU time consumed by the current thread |

Two of these overlap in the present implementation. `time-monotonic` is carried
on the same scale as `time-tai`, so a monotonic instant and the TAI instant with
the same second count denote the same moment; and because SKINT is
single-threaded, `time-process` and `time-thread` measure the same thing. They
remain distinct *types* — a comparison between a monotonic time and a TAI time is
an error, as between any two differing types.

### Time objects

`(make-time type nanosecond second)` → *time*

Note the order: the nanosecond precedes the second.

```scheme
(make-time time-utc 123 456)      ; 456 seconds and 123 nanoseconds
```

`(time? obj)` → *boolean*

`(time-type time)` → *symbol*
<br>`(time-nanosecond time)` → *integer*
<br>`(time-second time)` → *integer*

`(set-time-type! time type)`
<br>`(set-time-nanosecond! time integer)`
<br>`(set-time-second! time integer)`

Time objects are mutable. Changing the type reinterprets the same numbers on a
different scale rather than converting between scales; use the conversion
procedures for that.

### Comparisons

`(time=? t1 t2)` → *boolean*
<br>`(time<? t1 t2)` → *boolean*
<br>`(time>? t1 t2)` → *boolean*
<br>`(time<=? t1 t2)` → *boolean*
<br>`(time>=? t1 t2)` → *boolean*

Seconds are compared first, nanoseconds break ties.

```scheme
(time=? (make-time time-utc 0 1) (make-time time-utc 0 1))   ; => #t
(time<? (make-time time-utc 1000 1) (make-time time-utc 2000 1))   ; => #t
```

It is an error if the two arguments do not have the same time type.

`(time-compare t1 t2)` → *-1, 0 or 1*

A three-way comparison, negative when `t1` precedes `t2`, zero when they are
equal, positive when `t1` follows. Convenient for sorting.

```scheme
(time-compare (make-time time-utc 0 1) (make-time time-utc 0 2))   ; => -1
(time-compare (make-time time-utc 0 2) (make-time time-utc 0 2))   ; => 0
(time-compare (make-time time-utc 0 3) (make-time time-utc 0 2))   ; => 1
```

This is not part of SRFI 19.

### Arithmetic

`(time-difference t1 t2)` → *time*

The interval from `t2` to `t1`, as a `time-duration`. Negative when `t1` precedes
`t2`. It is an error if the arguments differ in time type.

```scheme
(time-difference (make-time time-utc 0 3000) (make-time time-utc 0 1000))
; a duration of 2000 seconds
```

`(add-duration time duration)` → *time*
<br>`(subtract-duration time duration)` → *time*

The result takes the time type of the first argument. Nanoseconds carry and
borrow across the second field. Both return a fresh time object and leave their
arguments alone.

```scheme
(add-duration (make-time time-utc 600000000.0 1)
              (make-time time-duration 600000000.0 0))
; 2 seconds and 200000000 nanoseconds
```

### Converting between scales

`(time-utc->time-tai time)` → *time*
<br>`(time-tai->time-utc time)` → *time*
<br>`(time-utc->time-monotonic time)` → *time*
<br>`(time-monotonic->time-utc time)` → *time*
<br>`(time-tai->time-monotonic time)` → *time*
<br>`(time-monotonic->time-tai time)` → *time*

UTC and TAI differ by a whole number of seconds that changes at each leap second
insertion; the conversions consult a table of those boundaries. Nanoseconds are
carried through unchanged.

```scheme
;; 1999-01-01T00:00:00Z, from which point TAI ran 32 seconds ahead
(time-second (time-utc->time-tai (make-time time-utc 0 915148800.0)))
; => 915148832
```

Converting in one direction and back yields the original instant. Because
monotonic time shares the TAI scale, the monotonic conversions differ from the
TAI ones only in the time type they attach.

It is an error to hand any of these a time whose type is not the one named on the
left of the arrow.

### The clock

`(current-time [type])` → *time*

The current time on the given scale, `time-utc` by default. All six types are
accepted.

`(time-resolution [type])` → *integer*

The clock's resolution in nanoseconds, for the given scale.

### Reading the clock directly

Three procedures return plain numbers rather than time objects. They are not part
of SRFI 19. For seconds since the epoch, the standard `current-second` of
`(scheme time)` serves.

`(current-microsecond)` → *integer*

Whole microseconds since the epoch.

`(process-microsecond)` → *integer*

CPU time consumed by this process, in microseconds.

`(current-timezone-offset)` → *integer*

The local time zone's offset from UTC in seconds, positive east of Greenwich.

### How numbers are represented

Second counts at epoch scale are far larger than a fixnum, so how they are
represented depends on how SKINT was built.

- *Without the numeric tower*, they are whole flonums — `915148832.0` rather
  than `915148832`. This is what lets the library work at all in that
  configuration: a value that large has no exact representation available.
- *With the numeric tower*, they stay exact, and the library avoids introducing
  inexactness that was not already there.

Both satisfy `integer?`, which is what the arithmetic and the conversions rely
on; Scheme's `integer?` is true of any number with no fractional part, whole
flonums included. Compare times with `time=?` and numbers with `=`, never with
`eqv?`, and code written either way runs unchanged in both configurations.

One consequence worth knowing when writing literals: in a build without the
tower, an epoch-scale integer such as `915148800` cannot be read at all, and must
be written `915148800.0`. The same applies to nanosecond values above about
5·10⁸. A flonum literal reads correctly in both configurations, so it is the
portable choice for code meant to run in either.

This is a property of the reader rather than of these procedures, which has one
practical consequence. A `cond-expand` cannot shield such a literal from within,
because the form containing it is read as a whole before any expansion:

```scheme
;; fails to read without the tower, even though the branch is not taken
(cond-expand (full-numeric-tower (display 915148800)) (else 'skip))
```

A file loaded as a program or script is read one form at a time, though, so a
`cond-expand` that returns from or exits the file keeps the reader from ever
reaching what follows. That is how to write a section that only a tower build
should even parse:

```scheme
(cond-expand ((not full-numeric-tower) (exit 0)))

;; only reached, and only read, with the tower
(display 915148800)
```

### Relationship to SRFI 19

This library implements the time half of SRFI 19, and departs from it in a few
places.

Provided beyond the SRFI: `time-compare`, and the three direct clock readings
above.

Not provided here: `copy-time`, the in-place `time-difference!`, `add-duration!`
and `subtract-duration!` variants, and everything concerning Julian Day numbers.
The date half — date objects, calendar queries and conversions between times and
dates — is in [`(skint date)`](date.md).

`time-monotonic` sharing the TAI scale, and `time-thread` matching
`time-process`, are implementation choices the SRFI leaves open: it requires
monotonic time to increase from an implementation-dependent epoch, and describes
process and thread time as implementation dependent.
