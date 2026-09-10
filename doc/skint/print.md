## (skint print) — configurable printing and a pretty-printer

Two printing procedures with knobs on. `print` writes a datum the way `write`
does, under the control of a set of parameters; `pretty-print` lays a datum out
over several lines, indenting it the way Scheme code is normally indented.

```scheme
(import (skint print))
```

Both are aimed at reading output rather than at feeding it back to a program,
although what they produce can be read back: the printed form of a datum is a
datum that `read` accepts, unless a parameter has been set to truncate it.

### print

`(print obj)` → *unspecified*
<br>`(print obj port)` → *unspecified*
<br>`(print obj parameter value ...)` → *unspecified*
<br>`(print obj port parameter value ...)` → *unspecified*

Writes `obj`. Without a port, output goes to the current output port. Strings
and characters are written, not displayed, so the result reads back as the same
datum.

```scheme
(print "hi")                              ; prints "hi"
(print '(a b c))                          ; prints (a b c)
```

With no parameters set, `print` produces exactly what `write` from
`(scheme write)` produces: one line, no newline of its own, and datum labels
only where they are needed to keep a circular datum finite. The only difference
is that `print` abbreviates a quote form — `'x` where `write` writes
`(quote x)`.

Every parameter below changes some part of that. `print-indent` is the one that
changes the most: with it set, output is broken across lines and ends with a
newline.

### pretty-print

`(pretty-print obj)` → *unspecified*
<br>`(pretty-print obj port)` → *unspecified*
<br>`(pretty-print obj parameter value ...)` → *unspecified*
<br>`(pretty-print obj port parameter value ...)` → *unspecified*

Writes `obj` over as many lines as it needs, indented as code, followed by a
newline.

```scheme
(pretty-print '(define (f x) (if (> x 0) (g x) (h x))) print-width 24)
```

```scheme
(define (f x)
  (if (> x 0)
      (g x)
      (h x)))
```

Forms whose head is a known keyword are laid out according to that keyword's
style — `cond` clauses one per line, a `define` body indented by two, and so on.
See [pretty-style](#pretty-style) for the registry that decides this, and for how
to add an entry for a macro of your own.

`pretty-print` is `print` with three settings supplied: indentation on, code
layout rather than data layout, and square brackets on binding lists. They are
supplied as *defaults*, so an argument of your own overrides any of them — see
[Passing parameters](#passing-parameters).

### Passing parameters

Every parameter listed under [Parameters](#parameters) can be set in three ways:
by `parameterize`, by calling the parameter, or by passing the parameter object
itself followed by a value as trailing arguments to `print` or `pretty-print`.

```scheme
(print '(255 256) print-radix 16)          ; prints (#xff #x100)

(parameterize ([print-radix 16])
  (print '(255 256)))                      ; the same
```

The parameter object is the key, so there is no separate namespace of keyword
names to learn, and a misspelling is an unbound-variable error rather than a
silently ignored option. An argument list that does not alternate parameter and
value, or that names something that is not one of these parameters, is an error.

*The first occurrence of a parameter wins.* This is what makes
`pretty-print`'s own three settings defaults: it appends them behind whatever you
passed, so

```scheme
(pretty-print '(let ((x 1)) x) print-brackets #f)
```

prints `(let ((x 1)) x)` and not `(let ([x 1]) x)`.

### Parameters

| Parameter | Value | Default | Effect |
|---|---|---|---|
| `print-indent` | `#f` or exact non-negative integer | `#f` | the column output starts at, or `#f` to stay on one line |
| `print-width` | exact positive integer | `80` | the column output tries to stay within |
| `print-circle` | boolean | `#t` | mark circular structure with datum labels |
| `print-graph` | boolean | `#f` | mark all shared structure, circular or not |
| `print-radix` | `2`, `8`, `10` or `16` | `10` | radix for exact integers |
| `print-length` | `#f` or exact non-negative integer | `#f` | how many elements of a sequence to show |
| `print-level` | `#f` or exact non-negative integer | `#f` | how deep to descend into nested structure |
| `print-brackets` | boolean | `#f` | write binding lists with square brackets |

Setting one to a value outside its range is an error, signalled when the value is
installed:

```scheme
(parameterize ([print-radix 3]) 'unreachable)   ; error: invalid value for print-radix
```

### One line or many

`print-indent` decides whether output is broken across lines at all. `#f`, the
default, means inline: one line however long it is, and no newline at the end, so
that the result can be embedded in a line the caller is building.

```scheme
(print '(a b c))                           ; prints (a b c), no newline
```

An exact non-negative integer instead turns line breaking on and makes `print`
end its output with a newline. The number is the column the first line is taken
to start at, and every line after the first is indented by it, so a caller that
has already written a prefix can say how wide that prefix was.

```scheme
(print '(aaaa bbbb cccc dddd) print-width 12 print-indent 0)
```

```scheme
(aaaa
 bbbb
 cccc
 dddd)
```

`print-width` is what line breaking aims at, and so it does nothing at all while
output is inline. It is a target, not a bound: a line can overshoot it when a
datum has no break point — a long symbol or string, or a run of closing
parentheses.

Line breaking is not the same as code layout. `print` lays a form out as data,
one element under the next; `pretty-print` indents it as code:

```scheme
(print '(define (f x) (if (> x 0) (g x) (h x))) print-width 24 print-indent 0)
```

```scheme
(define
 (f x)
 (if
  (> x 0)
  (g x)
  (h x)))
```

### Shared and circular structure

Two parameters decide whether structure that occurs more than once is marked with
datum labels, and between them they give three modes.

`print-circle` on and `print-graph` off, which is the default, labels only what
has to be labelled for printing to terminate: a circular datum gets a label, and
anything merely shared is printed once for each occurrence. This is what R7RS
`write` does.

```scheme
(define x (list 1 2))
(print (list x x))                         ; prints ((1 2) (1 2))

(define y (list 1 2))
(set-cdr! (cdr y) y)
(print y)                                  ; prints #0=(1 2 . #0#)
```

`print-graph` on labels every shared substructure, whether or not the sharing is
circular, and overrides `print-circle`. This is what R7RS `write-shared` does.

```scheme
(print (list x x) print-graph #t)          ; prints (#0=(1 2) #0#)
(print y print-graph #t)                   ; prints #0=(1 2 . #0#)
```

Both off labels nothing, which is what R7RS `write-simple` does.

```scheme
(print (list x x) print-circle #f)         ; prints ((1 2) (1 2))
```

*It is an error to print a circular datum with both off.* There is nothing to
stop the descent, and the call does not return.

### Truncating output

`print-level` and `print-length` bound how much of a large structure is shown.
Anything omitted is replaced by `...`, and the container it was in keeps its
shape, so a truncated list still reads as a list and a truncated vector as a
vector.

`print-length` limits the number of elements printed from each sequence:

```scheme
(print '(a b c d) print-length 2)          ; prints (a b ...)
```

`print-level` limits how deeply nesting is followed; a structure below the limit
becomes an empty container with `...` inside:

```scheme
(print '(a (b (c))) print-level 2)         ; prints (a (b (...)))
```

Either can be `#f`, which is the default and means no limit. They compose, and
each applies at every level of the datum. A pair's tail after a dot is not
counted as an element, so `(a b . c)` under `print-length 2` prints in full.

Truncated output is for reading, not for reading back: `...` is not a datum that
reconstructs what was elided.

### Radix

`print-radix` selects the radix for exact integers. Radix `10` prints them bare;
any other radix prints the corresponding prefix, so the result still reads back as
the same number.

```scheme
(print '(255 256) print-radix 16)          ; prints (#xff #x100)
(print 10 print-radix 2)                   ; prints #b1010
```

Inexact numbers are printed in the machine-readable form the implementation uses
for them, whatever the radix.

### Brackets

`print-brackets` writes the binding list of a form such as `let` with square
brackets instead of parentheses, which is the usual Scheme convention for
distinguishing bindings from calls.

Telling a binding list from a call is a matter of reading the form as code, so the
parameter has an effect only where that is being done — which is
`pretty-print`. `print` prints data, and takes no notice of it.

```scheme
(pretty-print '(let ((x 1)) x))            ; prints (let ([x 1]) x)
(pretty-print '(let ((x 1)) x) print-brackets #f)   ; prints (let ((x 1)) x)
(print '(let ((x 1)) x) print-brackets #t)          ; prints (let ((x 1)) x)
```

### pretty-style

`(pretty-style keyword)` → *style* or `#f`
<br>`(pretty-style keyword style)` → *unspecified*

Reads or sets the layout style `pretty-print` uses for forms whose head is
`keyword`. With one argument it returns the style registered for that keyword, or
`#f` if there is none; with two it registers `style` and returns an unspecified
value.

```scheme
(pretty-style 'lambda)                     ; => (_ d . body)
(pretty-style 'cond)                       ; => (_ . ec*)
(pretty-style 'no-such-form)               ; => #f
```

A style is a list matched against the elements of the form. The head position is
always `_`. Later positions, and the tail after a dot, are one of:

| Symbol | Means |
|---|---|
| `_` | the keyword itself; only ever appears first |
| `e` | an expression |
| `d` | a datum |
| `i` | an identifier |
| `i?` | an identifier if one is there, otherwise nothing is consumed |
| `dc` | one clause, its parts printed as data |
| `ec` | one clause, its parts printed as expressions |
| `dc*` | a list of clauses printed as data — a binding list |
| `ec*` | a list of clauses printed as expressions — `cond` arms |
| `body` | the rest is a body: one form per line, indented |
| `fill` | the rest is packed, as many per line as fit |

Registering a style for a macro of your own makes `pretty-print` lay it out like
the standard form it resembles:

```scheme
(pretty-style 'my-binding-form '(_ dc* . body))

(pretty-print '(my-binding-form ((x 1) (y 2)) (display x)) print-width 20)
```

```scheme
(my-binding-form
  ([x 1] [y 2])
  (display x))
```

A keyword with no registered style is laid out as an ordinary procedure call: the
head, then its arguments packed onto as many lines as they need.

The exact placement of line breaks within a style is heuristic and may change;
what a style fixes is which elements are treated as bindings, clauses, or body,
not the column each one lands in.

### Relation to the standard output procedures

`(scheme write)` supplies `write`, `write-shared`, `write-simple` and `display`.
This library overlaps them and adds to them:

| Want | Use |
|---|---|
| a datum on one line, marking only where needed | `print`, or `write` |
| a datum on one line, all sharing marked | `print` with `print-graph` on, or `write-shared` |
| a datum on one line, no marking, no cycles | `print` with `print-circle` off, or `write-simple` |
| any of those broken across lines | `print` with `print-indent` set |
| a datum laid out as code | `pretty-print` |
| a datum with numbers in another radix, or cut short | `print` |
| text without quoting | `display` |

None of the standard procedures ends its output with a newline, and neither does
`print` until `print-indent` is set. `pretty-print` always does.
