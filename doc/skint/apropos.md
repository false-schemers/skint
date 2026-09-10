## (skint apropos) — finding identifiers by name

Two procedures that answer "what is there with `print` in the name?". `apropos`
prints the answer for a person to read; `apropos-list` returns it for a program
to work with. Both follow Chez Scheme's procedures of the same names.

```scheme
(import (skint apropos))
```

The answer has two halves: the identifiers bound in an environment, and the
identifiers exported by each library the system knows about. The one library
left out of the second half is the implementation's own -- see
[The interpreter's own inside](#the-interpreters-own-inside).

### apropos

`(apropos s)` → *unspecified*
`(apropos s env)` → *unspecified*

Writes to the current output port the identifiers whose names contain `s`,
grouped by where they are bound. `s` is a symbol or a string; a symbol stands
for its name. Without an environment, the search covers the interaction
environment.

```scheme
(import (skint print))
(apropos 'print-w)
```

```
interaction environment:
  print-width
(skint print):
  print-width
```

The first group is the environment. After it comes one group per library that
exports a matching identifier, in library-name order. Each group lists its
identifiers alphabetically, separated by commas, indented by two spaces and
filled to fit within 78 columns.

```scheme
(apropos 'print)
```

```
interaction environment:
  format-exponential-print, format-fixed-print, format-general-print,
  format-pretty-print, fprintf, pretty-print, print, print-brackets,
  print-circle, print-graph, print-indent, print-length, print-level,
  print-radix, print-width, printf
(skint):
  format-exponential-print, format-fixed-print, format-general-print,
  format-pretty-print, fprintf, printf
(skint print):
  pretty-print, print, print-brackets, print-circle, print-graph,
  print-indent, print-length, print-level, print-radix, print-width
```

A group with nothing in it is left out, and a search that matches nothing prints
nothing at all. A name too long to fit is not broken, so a line can overshoot.

### apropos-list

`(apropos-list s)` → *list*
`(apropos-list s env)` → *list*

Returns what `apropos` would print. The list holds the environment's matching
identifiers first, as symbols, and then one entry per library, as a pair of the
library name and the list of its matching exports.

```scheme
(apropos-list 'exact-integer-sq)
  ⇒ (exact-integer-sqrt
     ((scheme base) exact-integer-sqrt)
     ((skint) exact-integer-sqrt))
```

A symbol in the list is therefore an identifier you can use as it stands, and a
pair tells you which library to import to get one you cannot.

The symbols come first and every element after the first pair is a pair, so the
two halves can be separated by testing one element:

```scheme
(define (environment-part l)
  (if (and (pair? l) (symbol? (car l))) (cons (car l) (environment-part (cdr l))) '()))

(define (library-part l)
  (if (and (pair? l) (symbol? (car l))) (library-part (cdr l)) l))
```

Nothing matching gives the empty list.

### Matching

`s` matches an identifier when its text occurs anywhere in the identifier's
name, as a substring. Matching is case sensitive, and there are no wildcards or
character classes — `(apropos-list "")` matches everything.

```scheme
(apropos-list 'integer-sq)     ; the same as
(apropos-list "integer-sq")    ; this
```

It is an error to pass anything but a symbol or a string.

### Which environment

The default is the interaction environment, which at the REPL is where
everything visible without an import lives: the built-in procedures and syntax,
whatever you have defined, and whatever you have imported.

Any environment may be given instead: `(environment ...)` from `(scheme eval)`,
`(scheme-report-environment 5)` or `(null-environment 5)` from `(scheme r5rs)`,
or one built by `(skint hidden)`. Only the first group depends on it; the
library groups do not.

```scheme
(apropos-list 'set-car! (null-environment 5))
  ⇒ (((scheme base) set-car!) ((scheme r5rs) set-car!) ((skint) set-car!))
```

`set-car!` is exported by those three libraries and bound in none of the
bindings `(null-environment 5)` provides, so the environment half is empty while
the library half is unchanged. `apropos` heads the first group with
`interaction environment:` when the environment is the interaction environment
and with `environment:` otherwise.

It is an error to pass anything that is not an environment.

### Which libraries

Every library the implementation has predefined, and every library that has
since been loaded from a `.sld` file or defined by a `define-library` form at
the top level. A library that exists as a file but has not been imported yet is
not among them: nothing has read it, so nothing knows what it exports.

```scheme
(apropos-list 'delete-dup)          ⇒ ()
(import (srfi 1))
(apropos-list 'delete-dup)
  ⇒ (delete-duplicates delete-duplicates!
     ((srfi 1) delete-duplicates delete-duplicates!))
```

*Only exported identifiers are reported.* A library's private definitions are
as real as its exports and live in the same place, but they are not reachable by
name and are never listed:

```scheme
(define-library (demo one)
  (export widget)
  (import (scheme base))
  (begin (define widget 1) (define secret 2)))

(apropos-list 'widget)     ⇒ (((demo one) widget))
(apropos-list 'secret)     ⇒ ()
```

Libraries are ordered by name: part by part, numbers before symbols, and a name
that is a prefix of another first, so `(skint)` precedes `(skint print)`.

### The interpreter's own inside

`(apropos-expose-hidden)` → *boolean*
<br>`(apropos-expose-hidden boolean)` → *unspecified*

A parameter, `#f` by default. While it is false — and it is a `parameterize`
away from being true — nothing exported by `(skint hidden)` is reported.

That library is where the implementation keeps the machinery it needs and no
program should reach for: the expander, the compiler, the reader, the name
registries. Its contents change from release to release and are not part of any
contract, so listing them among a library's documented exports would be
misleading.

```scheme
(apropos-list 'repl-read)          ⇒ ()

(parameterize ([apropos-expose-hidden #t])
  (apropos-list 'repl-read))       ⇒ (((skint hidden) repl-read))
```

Only that library's own group is withheld. An identifier of its that you have
imported yourself is still reported in the environment group, because that is
where it is bound:

```scheme
(import (only (skint hidden) evaluate-top-form))

(apropos-list 'evaluate-top)       ⇒ (evaluate-top-form)
```

### Mentioning a name binds it

SKINT interns a previously unknown global variable on first reference,
definition or assignment. It is an error to read such a variable's value before
it has been defined or assigned — an error SKINT does not currently report — but
the variable exists from that first mention, so `apropos` and `apropos-list`
list it like any other if its name contains the input string:

```scheme
(apropos-list 'nowhere-defined)     ⇒ ()
(if #f nowhere-defined 'never-mind) ⇒ never-mind
(apropos-list 'nowhere-defined)     ⇒ (nowhere-defined)
```

So an identifier in the first group is one that has a binding, which is not
quite the same as one that has a value. A typo can put a name there, and there
is no way to take it back out. The library groups are unaffected: they are read
from export lists, which nothing typed at the REPL can add to.
