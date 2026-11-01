## (skint compile) — the whole-program Scheme to MARV compiler

`(skint compile)` translates a Scheme program into a MARV module. MARV is a
portable assembly intermediate language; its own tool chain turns the module
into a native executable. The compiler runs as an ordinary Skint library, so
the host is a Skint interpreter, while the program it produces is standalone
and carries none of Skint's own state.

The compiler is a whole-program compiler. It expands the program and every
library the program imports, shakes out what is unreachable, and emits one
module. There is no separate compilation and no linking of Scheme units.

A compiled program runs against a different library set from the one the host
provides. That set is written for a standalone executable and is narrower than
either R7RS or Skint. The sections on capabilities below describe where it
stops.

### Interface

`(compile-program input)` → *unspecified*
<br>`(compile-program input output)` → *unspecified, or a MARV module*
<br>`(compile-program input output param val ...)` → *unspecified, or a MARV module*
<br>`(compile-program input param val ...)` → *unspecified*

*input* is the program to compile. It may be a filename, an input port, or a
list of forms already read. A filename is used exactly as given, so it means
what it would to any other tool and the working directory decides.

*output* says where the module goes. A filename writes it to that file, again
used exactly as given. An output port writes it there. `#t`, which is the
default, writes it to the current output port. `#f` writes nothing and returns
the module as an S-expression instead; a MARV module is a list structure, so
this is the whole module and not a rendering of it.

Each *param* *val* pair sets a parameter for the duration of the call. An
explicit pair overrides the parameter's current value, so

```scheme
(compile-program "foo.scm" cp-optimize 2)
(parameterize ((cp-optimize 2)) (compile-program "foo.scm"))
```

mean the same thing. *output* is omitted exactly when the argument in its place
is a parameter, so both argument shapes above are unambiguous.

It is an error if a *param* is not a parameter, or if a *param* is given
without a *val*.

A second call compiles independently of the first; nothing of one program's
module carries into the next.

`(cp-optimize)` → *exact integer*

The optimization level, an exact integer from 0 to 3. The default is 1 and is
what the compiler does. The other levels are accepted and validated, and are
not yet interpreted. It is an error to set the parameter to anything else.

### What it emits

The module is named `$ds$program`. The name is fixed rather than taken from the
program, because MARV's own libraries are unprefixed and a program named after
one of them would otherwise collide.

Ahead of the module the compiler emits the three includes its output is written
against, in this order:

```scheme
(include system $scmrt)
(include $scmn)
(include system $scmi)
```

The order matters, and `system` matters: it is what makes MARV link each
library's companion C file.

### Dependencies

Compiling needs a Skint interpreter and nothing else. The host needs no numeric
tower; the compiler computes the 32-bit constants it emits as whole values
whether the host holds them exactly or as flonums.

Turning the module into an executable needs MARV, which is not part of this
tree. The runtime the module is written against is, under `srt/`: `scmrt` is
the runtime kernel, `scmn` the object primitives, `scmi` the instruction
macros. Each is a MARV header, and `scmrt` and `scmi` have companion C files
that MARV compiles at link time. MARV has to be told where they are, as both a
library and an include path, and `srt/` is found beside this file. Nothing in
the compiler reads `srt/`; it is there for the link step.

### The language a compiled program is written in

A compiled program is ordinary R7RS Scheme and imports libraries normally. What
differs is which libraries exist, and what the ones that exist can do.

These `(scheme ...)` libraries are provided: base, box, case-lambda, char,
complex, cxr, file, inexact, lazy, process-context, r5rs, r5rs-null, read,
time, write.

A set of `(srfi ...)` libraries is provided as well, and of Skint's own
libraries `(skint fx)` and `(skint fl)`. `clib/` beside this file is the
authority on which libraries a compiled program gets; the ones with a file of
their own are rewritten for the compiled world, and the rest are taken from the
host's sources and re-expanded against that world.

These are not provided, and it is an error to import one: `(scheme load)`,
`(scheme repl)`, and the libraries parked under `clib-deferred/` in the
compiler's scratch tree, whose README says what each one still needs.

Skint's other libraries are not available to a compiled program. They are
written against the interpreter, so a program that imports one may compile
until it reaches a name the runtime has nothing to lower, and then it is an
error.

### Limitations compared to R7RS

#### Numbers

A compiled program has fixnums and flonums. It has no exact rationals, no
complex numbers and no bignums, and its fixnums are 30 bits wide. An exact
result outside that range becomes a flonum, which is what a towerless Skint
does, so a towerless host agrees with the compiled program on where that
boundary lies.

It is an error for a program to contain an exact rational literal, a complex
literal, or an exact integer literal outside the fixnum range.

`(scheme complex)` is provided over the reals, following a towerless Skint
rather than inventing a policy: `real-part` answers its argument, `imag-part`
an exact zero, `magnitude` the absolute value, `angle` an exact zero or pi. The
two constructors accept an exact zero as their second argument and report that
the result cannot be represented for anything else.

#### eval

A compiled program cannot use `eval`; it is an error. `(scheme eval)` is the
interpreter and the library manager, which is outside a compiler's scope.
Importing the library is not itself refused, so the error appears when `eval`
is reached.

#### Reading

`(scheme read)` is a real reader, with these differences. Only `#u8(...)`
constructs a typed vector; the other typed-vector syntaxes and the `#e` and
`#i` exactness prefixes are an error, there being no object to construct.
Rational and complex literals read back as symbols. Case folding applies to the
datum being read in that call and is not a property of the port.

#### Errors

A compiled program reports a non-recoverable error and exits with a non-zero
status. Error reporting is not a contract: do not depend on the wording.

### Limitations compared to Skint

The compiler accepts the language Skint accepts, including Skint's
`define-syntax` extension, because it uses Skint's own expander. The limits are
in the runtime, not in the syntax.

Beyond the numeric tower and the library set above, a compiled program is not
an interpreter: it has no global store to define into at run time, no name
registries, and no library manager. The tooling libraries that depend on those
have no counterpart here.

### Layout

| Path | What it is |
|---|---|
| `cups-base.scm` | utilities, the expression representation, the shared passes |
| `cups-signatures.scm` | primitive signatures and effects |
| `cups-passes.scm` | the middle-end passes |
| `cups-front.scm` | whole-program expansion, and the environment that serves `clib/` |
| `cups-dsmarv.scm` | the direct-style back end, the MARV printer, and the entry point |
| `clib/` | the libraries a compiled program is linked against |
| `srt/` | the MARV runtime the emitted module is written against |

The sources are `(include)`d by `../compile.sld` in the order above. They are
one program split for reading, not separate units: a definition in a later file
may shadow an earlier one.

[cco]
