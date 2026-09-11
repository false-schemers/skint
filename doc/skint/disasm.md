## (skint disasm) — reading compiled code back

SKINT compiles Scheme to a bytecode of printable characters and runs that. This
library goes the other way: from a procedure, or from any of the intermediate
forms, back to something a person can read.

```scheme
(import (skint disasm))
```

The short version is one procedure. `(da x)` takes whatever you have and gives
back the most that can be said about it:

```scheme
(define (make-adder n) (lambda (x) (+ x n)))

(da (make-adder 7))     ⇒ (let ((:a 7)) (lambda (.a) (+ .a :a)))
(da 'list-copy)         ⇒ (lambda (.a) (let .b ((.c .a)) (if (pair? .c) ...)))
(da car)                ⇒ car
(da 42)                 ⇒ #f
```

Local names are gone by the time code is compiled, so the disassembly invents
them: `.a`, `.b`, … for variables the code binds, and `:a`, `:b`, … for the free
variables a closure carries. Globals keep the names they had.

### What comes back, and how good it is

A disassembly is not the source. Names are invented, macros are gone, and a
derived form is recovered only if the shape it compiles to is unambiguous.

What is guaranteed is narrower than it first sounds, but it is the useful thing:
*compiling the form again produces the same bytecode, provided it is read back
in an environment that gives its names the same meanings they had.* It is a
different program on the page and the same program underneath — as long as the
page is read the same way. That proviso is not a formality; the conditions are
listed under [Re-expanding what comes back](#re-expanding-what-comes-back), and
some of them fail for perfectly ordinary procedures.

Two things are outside the guarantee whatever the environment. `(void)` is
written where the compiler had an empty `begin`, which is the same value and one
extra instruction — see [The empty expression](#the-empty-expression). And a
form that binds `?` for a display it does not have, or that rebuilds a
`case-lambda` from a dispatcher, is a description rather than a program; both are
noted where they come up.

Some code has no readable form at all: the built-in procedures written by hand or
generated in the runtime rather than compiled from Scheme. For those, `da`
answers with the name the procedure is filed under instead of a form, which is
why the result of `da` is worth testing with `pair?` or `symbol?`:

```scheme
(da car)          ⇒ car          ; a wrapper generated around the instruction
(da values)       ⇒ values       ; hand-written
```

### da

`(da x)` → *form*, *symbol* or `#f`

The whole chain, from whatever there is. `x` may be:

| `x` | and `da` gives |
|---|---|
| a procedure | its disassembly, with its closure's values bound around it |
| a symbol | the same for whatever the global store holds under that name |
| a fixnum | the same, for the global an integrable index leads to |
| a code vector | its disassembly, with `?` where the closure's values would be |
| a string | the same, decoded from bytecode |
| a pair | the Scheme form for a Core expression |

`#f` when `x` is none of those, or is one of them but yields nothing: a name the
store does not hold, a name holding something that is not a procedure, or code
with no readable form and no name. A procedure that is not a closure has no code
to read, so for one of those `da` gives the name the store files it under, or
`#f`.

A symbol is a *global name* — the name the store files a binding under, not
necessarily the identifier you type. Built-ins are filed under their own names,
so `'car` works; something defined at the REPL is filed under `repl://?name`.
See [doc/internals/store.md](../internals/store.md) if that matters to you.

An exact non-negative integer is taken as an *integrable index*, which is how
Core shows a built-in operation, and is followed to the global that operation's
wrapper procedure is filed under. It is there so that an index read off a
disassembly can be handed straight back:

```scheme
(define (unwrap p) (car p))

(da-bytecode 'repl://?unwrap)   ⇒ (lambda (.a) (integrable 181 (ref .a)))
(da 181)                        ⇒ car
```

*This is a convenience for working at a REPL and nothing more.* An index says
where an instruction happens to sit in a table that grows with the instruction
set, so it means nothing outside the session that produced it. Do not write one
down, and do not put one in a program.

### The stages

`da` is the four stages of the compiler run backwards, and each is available on
its own. Every one of them accepts anything the stages above it accept, so you
can start wherever you have something and stop wherever you like.

`(da-code x)` → *string* or `#f`
<br>`(da-bytecode x)` → *Core expression* or `#f`
<br>`(da-core x)` → *form* or `#f`
<br>`(da-procedure x)` → *form*, *symbol* or `#f`

`da-code` gives the bytecode string for the code vector of `x`, where `x` is a
code vector, a procedure or a global name. The string is exactly what the
compiler would have written: decoding it gives the code vector back. The
encoding itself is an implementation matter — see
[doc/internals/bytecode.md](../internals/bytecode.md) — so what one of these
strings looks like is not something to depend on.

`da-bytecode` gives the Core Scheme expression that compiles to the bytecode of
`x`, where `x` is any of the above or a bytecode string. Core Scheme is the
compiler's intermediate language: `lambda`, `if`, `begin`, `call`, `ref`, `set!`,
`quote` and a handful more, with no derived forms left. `#f` when the bytecode
has no Core expression behind it.

```scheme
(define (call-it f) (f))

(da-bytecode 'repl://?call-it)   ⇒ (lambda (.a) (call (ref .a)))
(da-core 'repl://?call-it)       ⇒ (lambda (.a) (.a))
```

`da-core` turns a Core expression into Scheme, putting back the derived forms the
expander erased. It takes any of the above, or a Core expression directly.
Internal definitions come back as `define`s, and one whose value is a procedure
comes back the short way, `(define (f a . rest) ...)` rather than
`(define f (lambda (a . rest) ...))`.

`da-procedure` is `da` restricted to a procedure or the global name of one; it is
what `da` calls for those. `(da-global sym)` is the same thing for a name only,
and is kept because asking specifically about a name reads better in some code.

`(da-name p)` → *symbol* or `#f`

The name the global store files the procedure `p` under, or `#f` if nothing
holds it. This is what the other entry points fall back to. Where a procedure
has more than one name, the one that reads best is chosen: an integrable's own
name over an alias, an unprefixed name over a store-prefixed one, the shorter
over the longer.

```scheme
(da-name car)              ⇒ car
(da-name (lambda (x) x))   ⇒ #f
```

Finding a name means walking the store, so `da-name` is much the slowest thing
here; it matters only when disassembling in bulk.

### Wrong types and bad values

Every entry point answers `#f` for an argument of a type it cannot use. That is
deliberate: asking `(da x)` about an arbitrary value is a reasonable thing to do,
and a `#f` is a better answer than an error.

An argument of the *right* type whose contents are wrong is a different matter.
The decoders can always give up and answer `#f` — "nothing could be made of
this" is a real answer about a code vector. `da-code` cannot: it has to write a
name for every instruction word it is given, so a vector that is not a code
vector is an error.

```scheme
(da 42)                    ⇒ #f            ; not a type it can use
(da (vector 1 2 3))        ⇒ #f            ; a vector, but nothing to decode
(da-code (vector 1 2 3))              ; error: unknown instruction word
```

It is an error to pass `da-bytecode` a string that is not bytecode.

### Closures, and the display

A procedure that refers to variables from an enclosing scope carries them in a
*display*. `da` names those `:a`, `:b`, … in display order and binds them in a
`let` around the lambda, so the whole thing reads as one form:

```scheme
(define (make-counter start) (let ([n start]) (lambda () (set! n (+ n 1)) n)))

(da (make-counter 10))   ⇒ (let ((:a 10)) (lambda () (set! :a (+ :a 1)) :a))
```

Given a code vector or a bytecode string rather than a live procedure, those
names are all that can be recovered — the values were in the closure, and there
is no closure. `da` still binds them, with `?` for a value, so that the form
says what is missing and where it would go:

```scheme
(da (da-code (make-adder 7)))   ⇒ (let ((:a ?)) (lambda (.a) (+ .a :a)))
```

Such a `let` is a display, not a program: `?` is not a value, and the form will
not compile.

A display entry that the code assigns holds a box rather than the value. `da`
binds the box's contents, so what you see is the variable's value and not its
cell.

### case-lambda

A `case-lambda` does not compile to one procedure with several arities. It
compiles to a *dispatcher* — a procedure whose whole body is "if the argument
count is this, hand over to that clause" — over a display holding one ordinary
closure per clause. `da` recognises that shape and puts the `case-lambda` back:

```scheme
(da (case-lambda [(x) x] [(x y) (cons x y)]))
  ⇒ (case-lambda ((.a) .a) ((.a .b) (cons .a .b)))
```

The clauses each carry their own display, and where two of them close over the
same variable they hold the same cell, so it is named once and bound outside the
`case-lambda` rather than once per clause. For an assigned variable that is not
a nicety but the only correct reading, since the clauses share one cell:

```scheme
(da (let ([n 0]) (case-lambda [() n] [(x) (set! n x) n])))
  ⇒ (let ((:a 0)) (case-lambda (() :a) ((.a) (set! :a .a) :a)))
```

The procedures that take optional arguments have the same dispatcher shape, so
they read back the same way — which is how a good part of the built-in library
comes to have a readable form at all:

```scheme
(da 'string-copy)
  ⇒ (case-lambda ((.a) (substring .a 0 (string-length .a)))
                 ((.a .b) (substring .a .b (string-length .a)))
                 ((.a .b .c) (substring .a .b .c)))
```

A dispatcher keeps its clauses in its display, so unlike an ordinary procedure it
has nothing to say without one: `da` of such a code vector or bytecode string is
`#f`, not a `case-lambda` with `?` for the clauses.

### Global names

`(da-prune-globals)` → *boolean*
<br>`(da-prune-globals boolean)` → *unspecified*

A global reaches the compiler under the name the store files it under, which
carries the prefix of the environment that allocated it. This parameter, `#t` by
default, decides whether a disassembly shows the bare identifier or the whole
name:

```scheme
(define (f x) (g x))

(da 'repl://?f)          ⇒ (lambda (.a) (g .a))

(parameterize ([da-prune-globals #f])
  (da 'repl://?f))       ⇒ (lambda (.a) (repl://?g .a))
```

Pruned reads as source; unpruned says where each binding actually lives, which
is what you want when two libraries define the same identifier. Built-in names
have no prefix and look the same either way.

### The empty expression

`(da-void-for-empty-begin)` → *boolean*
<br>`(da-void-for-empty-begin boolean)` → *unspecified*

Core Scheme has an empty `begin`, meaning "no value in particular". Written back
literally it is exact, but `(begin)` does not read as a value to most people, so
the output says `(void)` instead — the same value under a name. This parameter,
`#t` by default, controls that.

The two are the same value and not the same code: `(begin)` compiles to nothing
at all, `(void)` to one instruction. So a form containing `(void)` is the one
place where a disassembly does not compile back to the bytecode it came from. Set
the parameter to `#f` if that matters — when comparing bytecode, for instance:

```scheme
(da-core '(begin))      ⇒ (void)

(parameterize ([da-void-for-empty-begin #f])
  (da-core '(begin)))   ⇒ (begin)
```

Where this shows up in practice is the internal definitions of a body the
disassembler could not fold back into `define`s: their slots are cleared before
they are filled, and a cleared slot is an empty `begin`.

### Re-expanding what comes back

A disassembly can be fed back to `eval`, and when it can be, it compiles to the
bytecode it came from. What has to hold for that:

*The standard forms have to mean what they usually mean.* The output uses `let`,
`let*`, `letrec`, `letrec*`, `let-values`, `let*-values`, `define`,
`define-values`, `do`, `cond`, `case`, `and`, `or`, `when`, `unless`,
`case-lambda`, `lambda`, `if`, `begin`, `set!` and `quote`, and it uses them for
what they normally denote. A local binding or a redefinition that shadows one of
them changes what the form means.

*The globals it names have to be reachable under those names.* With
`da-prune-globals` on, a global appears as its bare identifier, and re-expanding
it binds whatever that identifier means where the form is read. For a built-in
that is the same location; for something defined at the REPL it is the same
location as long as it is the same REPL. For a library's own global it is *not*:
only an import reaches those, and no import reaches a name a library did not
export. Turning pruning off does not help — the full store name is just another
identifier to the expander.

*The integrables it names have to resolve to those integrables.* They come back
under their canonical names, and a few of those — `%port?` and its kind — belong
to `(skint hidden)` rather than to any standard library, so that library has to
be imported.

*The globals it assigns have to be assignable.* Built-in names are registered
immutable, so the expander refuses `set!` to one — correctly, whatever the code
it is refusing came from. A procedure that assigns a built-in global therefore
cannot be re-expanded at all.

None of that makes the form wrong. It makes the form a reading of the code rather
than a substitute for it, which is what a disassembly is.

### What a disassembly is good for

Reading it. What the compiler did with a piece of code, whether a `case` became a
jump table or a chain of comparisons, what a macro expanded to in the end, what a
closure is holding onto and therefore what it keeps alive.

It is not a decompiler in the sense of recovering someone's source, and it is not
a stable interface to build on: which derived forms are recovered, and what the
invented names look like, may change.
