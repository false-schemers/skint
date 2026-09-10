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
derived form is recovered only if the shape it compiles to is unambiguous. What
*is* guaranteed is stronger and more useful: with one documented exception, the
form that comes back compiles to the same bytecode as the code it came from. It
is a different program on the page and the same program underneath.

The exception is `(void)`; see [The empty expression](#the-empty-expression).

Some code has no readable form at all — the built-in procedures that were written
by hand or generated rather than compiled from Scheme. For those, `da` answers
with the name the procedure is filed under instead of a form, which is why the
result of `da` is worth testing with `pair?` or `symbol?`:

```scheme
(da car)          ⇒ car          ; a generated wrapper: no Scheme behind it
(da values)       ⇒ values       ; hand-written
```

### da

`(da x)` → *form*, *symbol* or `#f`

The whole chain, from whatever there is. `x` may be:

| `x` | and `da` gives |
|---|---|
| a procedure | its disassembly, with its closure's values bound around it |
| a symbol | the same for whatever the global store holds under that name |
| a code vector | its disassembly, with `?` where the closure's values would be |
| a string | the same, decoded from bytecode |
| a pair | the Scheme form for a Core expression |

`#f` when `x` is none of those, or is one of them but yields nothing: a name the
store does not hold, a name holding something that is not a procedure, or code
with no readable form and no name.

A symbol is a *global name* — the name the store files a binding under, not
necessarily the identifier you type. Built-ins are filed under their own names,
so `'car` works; something defined at the REPL is filed under `repl://?name`.
See [doc/internals/store.md](../internals/store.md) if that matters to you.

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

A disassembly can be fed back to `eval`, but it names things the way the compiler
does, and some of those names are not in scope everywhere. Integrable procedures
come back under their canonical names, and a few of those — `%port?` and its
kind — belong to `(skint hidden)` rather than to any standard library. Importing
that library puts them in scope.

For the same reason a form that mentions a library's private global cannot be
re-expanded at the REPL at all: only an import reaches those names, and there is
no import that reaches a name a library did not export.

### What a disassembly is good for

Reading it. What the compiler did with a piece of code, whether a `case` became a
jump table or a chain of comparisons, what a macro expanded to in the end, what a
closure is holding onto and therefore what it keeps alive.

It is not a decompiler in the sense of recovering someone's source, and it is not
a stable interface to build on: which derived forms are recovered, and what the
invented names look like, may change.
