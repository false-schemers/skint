# `(skint format)` — formatted output

SKINT's built-in formatting procedures: `format`, `fprintf` and `printf`, plus the
six parameters that control the directives with pluggable behaviour. The directive
set follows SRFI 48 (*Intermediate Format Strings*) and adds a few directives of
its own.

```scheme
(import (skint format))
```

These are the same procedures the interaction environment already exposes, so a
script does not need this import to call `format` — the library exists so that
programs and libraries, which start with nothing imported, can ask for them by
name.

## Procedures

### `(format destination format-string arg ...)`

The destination selects where the output goes and what is returned:

| `destination` | Output goes to | Returns |
|---|---|---|
| `#f` | nowhere | the formatted **string** |
| a string | nowhere — the string is the format string | the formatted **string** |
| `#t` | current output port | unspecified |
| a port | that port | unspecified |

```scheme
(format #f "~a and ~s" "plain" "written")   ; => "plain and \"written\""
(format "[~a]" 42)                          ; => "[42]"
(format #t "hello~%")                       ; writes to stdout
(format port "hello~%")                     ; writes to that port
```

The second row is the shorthand that makes `(format "[~a]" 42)` work: when the
first argument is a string it is taken as the format string itself, with `#f`
implied as the destination.

### `(fprintf port format-string arg ...)`

Writes to `port`. This is the underlying engine — `format` is a thin dispatcher
over it.

```scheme
(let ([p (open-output-string)])
  (fprintf p "to port: ~a" 42)
  (get-output-string p))                    ; => "to port: 42"
```

### `(printf format-string arg ...)`

`fprintf` to the current output port. Equivalent to `(format #t ...)`.

```scheme
(printf "hello ~a~%" 42)        ; writes "hello 42\n"
```

## Directives

Directive characters are case-insensitive: `~A` and `~a` are the same.

### Literal text and whitespace

| Directive | Effect |
|---|---|
| `~~` | A literal `~` |
| `~%` | Newline |
| `~t` | Tab |
| `~_` | Space |
| `~&` | Fresh line — see note below |
| `~!` | Flush the port |

```scheme
(format #f "~~")        ; => "~"
(format #f "a~%b")      ; => "a\nb"
(format #f "a~tb")      ; => "a\tb"
(format #f "a~_b")      ; => "a b"
```

`~&` is nominally a fresh-line — a newline only if not already at the start of one
— but SKINT does not track output columns, so it behaves exactly like `~%`. See
[`~&` always emits a newline](#-always-emits-a-newline).

### Objects

| Directive | Effect |
|---|---|
| `~a` | `display` the argument |
| `~s` | `write` the argument |
| `~w` | `write-shared` the argument |
| `~y` | Pretty-print the argument — see note below |
| `~c` | `write-char` the argument |

```scheme
(format #f "~a" '(1 "x" #\y))   ; => "(1 x y)"
(format #f "~s" '(1 "x" #\y))   ; => "(1 \"x\" #\\y)"
(format #f "~c" #\Z)            ; => "Z"
```

`~y` behaves as `~s`, since SKINT has no pretty printer. See
[`~y` writes rather than pretty-prints](#y-writes-rather-than-pretty-prints).

### Numbers by radix

| Directive | Radix |
|---|---|
| `~b` | 2 |
| `~o` | 8 |
| `~d` | 10 |
| `~x` | 16 |

```scheme
(format #f "~b" 10)     ; => "1010"
(format #f "~o" 64)     ; => "100"
(format #f "~d" 255)    ; => "255"
(format #f "~x" 255)    ; => "ff"
(format #f "~x" -255)   ; => "-ff"
```

It is an error if the argument is not a number.

#### Flonums in a non-decimal radix

These directives call `number->string`, which in SKINT accepts **flonums in every
radix** — not just exact integers. The result is exact and round-trips through
`string->number` at the same radix, so no precision is lost.

```scheme
(format #f "~b" 0.5)    ; => "0.1"
(format #f "~x" 0.5)    ; => "0.8"
(format #f "~o" 64.0)   ; => "100.0"
(format #f "~x" 3.14)   ; => "3.23D70A3D70A3E"

(string->number (format #f "~x" 3.14) 16)   ; => 3.14
```

Hex digits come out uppercase.

When the value needs an exponent, the marker and its base differ by radix:

| Directive | Marker | Exponent is a power of | Exponent digits are in |
|---|---|---|---|
| `~b` | `e` | 2 | binary |
| `~o` | `e` | 8 | octal |
| `~d` | `e` | 10 | decimal |
| `~x` | `p` | **2** (C99 hex-float convention) | **decimal** |

```scheme
(format #f "~d" 1e30)   ; => "1e30"
(format #f "~b" 1e30)   ; => "1.100100111110010110010011100110100000100011001110101e1100011"
(format #f "~o" 1e30)   ; => "1.44762623464043165e41"
(format #f "~x" 1e30)   ; => "C.9F2C9CD04675p96"
```

In the `~b` case the exponent `1100011` is itself binary — 99 — so the value is
mantissa × 2⁹⁹. In the `~o` case `41` is octal — 33 — giving mantissa × 8³³. The
`~x` case follows C99: `p` always introduces a power of *two* written in decimal,
so `p96` means × 2⁹⁶, not × 16⁹⁶.

The special values pass through unchanged in every radix, as does negative zero:

```scheme
(format #f "~x" +inf.0)   ; => "+inf.0"
(format #f "~b" +nan.0)   ; => "+nan.0"
(format #f "~o" -0.0)     ; => "-0.0"
```

SRFI 48 says only that the argument "is a number which is output in *radix*
radix", so it neither requires nor forbids this; implementations that restrict
these directives to exact integers are equally conformant.

### Inexact numbers, with width and precision

| Directive | Style |
|---|---|
| `~f` | Fixed point |
| `~e` | Exponential |
| `~g` | General (shorter of the two) |

Each accepts an optional width and precision, written `~w,df` — width first,
then a comma, then the number of digits after the point. Either may be omitted.
Output is right-aligned in the given width; the width is a minimum, never a
truncation.

**The precision is what selects the style.** Given one, the argument is converted
to inexact and rendered fixed, exponential or general as the directive says:

```scheme
(format #f "~10,3f" 1234.5678)  ; => "  1234.568"
(format #f "~10,3e" 1234.5678)  ; => "   1.235e3"
(format #f "~10,3g" 1234.5678)  ; => "    1.23e3"
(format #f "~,2f" 3.14159)      ; => "3.14"
(format #f "~8,2f" 3.14159)     ; => "    3.14"
```

With **no** precision, all three behave alike: the number is printed by
`number->string`, with no coercion and no exponential form, then padded to the
width. So an exact argument stays exact.

```scheme
(format #f "~f" 42)             ; => "42"
(format #f "~e" 42)             ; => "42"
(format #f "~f" 1234.5678)      ; => "1234.5678"
(format #f "~e" 1234.5678)      ; => "1234.5678"
(format #f "~12e" 1234.5678)    ; => "   1234.5678"
```

This matches SRFI 48, which specifies that `~w,dF` converts "as if added to 0.0"
while plain `~wF` leaves the number alone.

A string argument is padded to the width but otherwise passed through, as SRFI 48
specifies. It is an error if the argument is neither a number nor a string.

```scheme
(format #f "~8,3F" "foo")       ; => "     foo"
```

Note that the exponential form is SKINT's own (`1.235e3`), not C's `1.235e+03`.

### Control

| Directive | Effect |
|---|---|
| `~?` | Indirection: next argument is a format string, the one after is its argument *list* |
| `~k` | Identical to `~?` |
| `~*` | Skip the next argument |
| `~N*` | Skip `N` arguments |
| `~N@*` | Jump to argument `N`, counting from the start of the whole argument list |
| `~h` | Insert the help string |

```scheme
(format #f "~?" "[~a-~a]" '(1 2))     ; => "[1-2]"
(format #f "~a~*~a" 1 2 3)            ; => "13"
(format #f "~a~2*~a" 1 2 3 4)         ; => "14"
(format #f "~a~a~0@*~a" 1 2)          ; => "121"
(format #f "~a~a~1@*~a" 1 2)          ; => "122"
```

The distinction in the last two: `~N*` is relative to the current position, while
`~N@*` is absolute — it re-indexes into the original argument list, so `~0@*`
rewinds to the first argument.

`~h` inserts the current value of `format-help-string`, which by default lists
every supported directive:

```scheme
(format #f "~h")
; => "supported directives: ~~ ~% ~& ~t ~_ ~a ~s ~w ~y ~c ~b ~o ~d ~x ~e ~f ~g ~? ~k ~* ~N@* ~!"
```

### Unknown directives

Any directive character not in the tables above raises *"unrecognized ~ directive"*,
carrying the offending character as its irritant. Typos are loud rather than
silent — there is no pass-through of unrecognized escapes.

## Parameters

Six directives dispatch through parameter objects, so their behaviour can be
replaced — globally by calling the parameter, or for a dynamic extent with
`parameterize`.

| Parameter | Controls | Default |
|---|---|---|
| `format-pretty-print` | `~y` | `write` |
| `format-fixed-print` | `~f` | the built-in fixed-point renderer |
| `format-exponential-print` | `~e` | the built-in exponential renderer |
| `format-general-print` | `~g` | the built-in general renderer |
| `format-fresh-line` | `~&` | `newline` |
| `format-help-string` | `~h` | the directive list shown above |

`format-pretty-print` and `format-fresh-line` hold a procedure of one or two
arguments — the object and the port, or just the port for `~&`. The three numeric
parameters hold a procedure of four arguments, `(arg width digits port)`, where
`width` and `digits` are `-1` when unspecified.

```scheme
(parameterize ([format-pretty-print display])
  (format #f "~y" '(a "b")))                ; => "(a b)"

(parameterize ([format-fresh-line (lambda (p) #f)])
  (format #f "x~&y"))                       ; => "xy"

(parameterize ([format-fixed-print (lambda (arg wd dd p) (display "<F>" p))])
  (format #f "~f" 1.5))                     ; => "<F>"

(parameterize ([format-help-string "my help"])
  (format #f "~h"))                         ; => "my help"
```

## Malformed format strings

These are errors:

- a directive that needs an argument when none is left
- a format string that ends in a bare `~`
- a directive character that is not one of those listed above
- a `~N*` or `~N@*` index outside the argument list

As elsewhere in Scheme, "an error" leaves the response open; do not write code
that depends on a particular message or on the error being catchable. Treat these
as mistakes to fix in the format string, not as a signalling mechanism.

It is also an error to supply more arguments than the format string consumes.

## Conformance with SRFI 48

SKINT implements **every directive in the SRFI 48 table** — `~a ~s ~w ~d ~x ~o ~b
~c ~y ~? ~k ~F ~~ ~t ~% ~& ~_ ~h` — and the `format` signature matches: the port
argument is optional, `#f` or omission returns a string, `#t` and a port write and
return an unspecified value.

All ten of the worked examples in the SRFI's specification produce exactly the
output it gives, with nothing extra imported:

```scheme
(format #f "~8,2F" 32)          ; => "   32.00"
(format #f "~6F" 32)            ; => "    32"
(format #f "~1,2F" 4321)        ; => "4321.00"
(format #f "~8,3F" 123.3456)    ; => " 123.346"
(format #f "~6,3F" 123.3456)    ; => "123.346"
(format #f "~2,3F" 123.3456)    ; => "123.346"
(format #f "~8,3F" "foo")       ; => "     foo"
(format #f "~4F" 12)            ; => "  12"
(format #f "~a ~s ~a ~s" 'this 'is "a" "test")   ; => "this is a \"test\""
(format #f "~a ~? ~a" 'a "~s" '(new) 'test)      ; => "a new test"
```

Three directives are worth knowing about if you are porting code between
implementations.

### `~&` may emit a newline where another implementation would not

SRFI 48 describes freshline as emitting a newline "if it is known that the
previous output was not a newline". SKINT does not track output columns, so `~&`
emits one unconditionally and consecutive `~&` directives each produce a line. Do
not rely on `~&` collapsing. Supply your own `format-fresh-line` if you need
column tracking.

### `~y` output is not laid out

`~y` pretty-prints its argument. SRFI 48 permits a pretty printer that simply
writes, which is what the default renderer does, so do not depend on any
particular layout. Supply your own `format-pretty-print` for real formatting.

### `~h`

`~h` inserts a single line naming the supported directives, rather than the
multi-line synopsis some implementations produce. Rebind `format-help-string` to
change it.

## Relationship to the SRFI ports

Three libraries expose `format`:

| Library | Exports | Notes |
|---|---|---|
| `(srfi 28)` | `format` | Bare re-export of the built-in procedure |
| `(srfi 48)` | `format` | Bare re-export of the built-in procedure |
| `(skint format)` | `format` `printf` `fprintf` + 6 parameters | The full native interface |

All three are views onto one engine and **behave identically** — same directive
set, same output, no import-order effects. Which one you import is a statement
about portability, not behaviour: reach for `(srfi 28)` or `(srfi 48)` in code
meant to run on other Schemes, and `(skint format)` when you want `printf`,
`fprintf`, or the parameters.

Importing `(srfi 48)` neither adds nor removes conformance; the directives behave
as the section above describes whichever library you came through.

## Extensions beyond SRFI 48

Directives the SRFI 48 table does not include:

- `~!` — flush the port; common in other `format` dialects.
- `~e` and `~g` — exponential and general float formats. SRFI 48 has only `~F`.
- `~*`, `~N*` and `~N@*` — argument skipping and absolute jumps, after Common Lisp.
- Width and precision on `~e` and `~g`, not just `~F`.

Plus `printf` and `fprintf` themselves, and the six parameters, which make `~y`,
`~&`, `~e`, `~f`, `~g` and `~h` replaceable. SRFI 48 specifies nothing equivalent;
they are the supported way to change how a directive renders without writing your
own `format`.

Note that `~w` is **not** an extension: it is in the SRFI 48 table as
*WriteCircular*, specified as `write-with-shared-structure`, which is what SKINT's
`write-shared` provides.
