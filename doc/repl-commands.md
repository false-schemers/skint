## REPL commands

At the prompt, a line beginning with a comma is a command to the interpreter
rather than an expression to evaluate:

```
skint] ,pwd
C:\Users\ESL\scheme\
```

Type `,help` for the list, or `,h` for short. What follows describes each one.

Commands are for working interactively and nothing else. They are available only
when skint is reading from a terminal — piping a script into skint, or `,load`ing
a file, evaluates every line as Scheme, and a line starting with a comma is then
read as a `quote`-like form and fails. Nothing you write in a file needs to know
about them.

A command occupies one line. The name comes first, and the rest of the line is
its argument; a command that takes an expression therefore needs that expression
to fit on the line.

### Loading and importing

`,load <filename>`

Reads and evaluates a file, as `load` would. The name needs no quotes, and
spaces in it are taken as part of it:

```
skint] ,load my file.scm
```

`,im <library> ...`

Imports libraries, in a shorthand for the common cases:

| written | means |
|---|---|
| a symbol | `(skint <symbol>)` |
| an exact non-negative integer | `(srfi <integer>)` |
| a list | itself |

So

```
skint] ,im fx 120 (foo bar) 31 fl
```

does what

```scheme
(import (skint fx) (srfi 120) (foo bar) (srfi 31) (skint fl))
```

does. Anything that is none of the three — a string, a negative number, a list
with something other than identifiers and integers in it — is reported and
nothing at all is imported, so a mistyped name cannot leave you with half of what
you asked for.

With nothing after it, `,im` imports nothing and says so.

Note that a plain `import` expression is still there when you want it; `,im` only
saves typing, and it cannot express `only`, `except`, `prefix` or `rename`.

### Tracing

`,tr <name> ...`
<br>`,untr <name> ...`

Start and stop tracing the procedures those names are bound to. Both fetch the
`(skint trace)` library the first time they are used, so nothing has to be
imported first:

```
skint] (define (f x) (* x 2))
skint] (define (g x) (+ (f x) 1))
skint] ,tr f g
(f g)
skint] (g 5)
|(g 5)
| (f 5)
| 10
|11
11
skint] ,untr f
(f)
skint] (g 5)
|(g 5)
|11
11
```

Each answers with the names it acted on. A trace shows the call, then the calls
made inside it one level further in, then each result against its call.

`,tr`
<br>`,untr`

With no names, `,tr` answers the names being traced, and `,untr` stops tracing
all of them, answering with the names it untraced. Continuing from above:

```
skint] ,tr
(g)
skint] ,untr
(g)
skint] ,tr
()
```

These are `(trace)` and `(untrace)`. The rest of what the library offers is in
[doc/skint/trace.md](skint/trace.md).

An argument that is not a name is reported and nothing is traced.

### Printing and disassembling

`,pp <expression>`

Evaluate the expression and pretty-print the value, fetching the
`(skint print)` library the first time it is used:

```
skint] ,pp (map (lambda (i) (list i (* i i))) '(1 2 3 4 5 6 7 8 9 10 11 12))
((1 1) (2 4) (3 9) (4 16) (5 25) (6 36) (7 49) (8 64) (9 81)
  (10 100) (11 121) (12 144))
```

This is `(pretty-print <expression>)` and nothing more, so the layout obeys
whatever the printer's parameters currently say about width, depth, radix and
the rest — see [doc/skint/print.md](skint/print.md).

`,da <procedure>`

Show what a procedure is made of, as a Scheme expression, pretty-printed. Both
`(skint print)` and `(skint disasm)` are fetched on first use:

```
skint] (define (fact n) (if (= n 0) 1 (* n (fact (- n 1)))))
skint] ,da fact
(lambda (.a) (if (= .a 0) 1 (* .a (fact (- .a 1)))))
```

The names beginning with a dot are invented: a compiled procedure no longer
carries the names its variables had in the source. `,da` also takes an
expression, a code vector or a bytecode string, and answers `#f` when it cannot
make an expression out of what it was given —
[doc/skint/disasm.md](skint/disasm.md) says what the answer means and how far to
trust it.

*A global name may be written without quoting it.* Names in the global store
have a `://` in them — `repl://?fact` for something you defined at the prompt,
`lib://skint/disasm?da-global` for a library's — and `,da` quotes such a symbol
for you:

```
skint] ,da lib://skint/disasm?da-global
(lambda (.a)
  (cond [(global-name-of .a) => da-procedure] [else #f]))
```

so that line means what `,da 'lib://skint/disasm?da-global` means. Quoting it
yourself works as well. This is the only place where a command rewrites what you
typed, and it applies to `,da` alone: `,pp` quotes nothing, and the same symbol
there is an ordinary variable reference.

With nothing after them, both commands fetch their libraries, say so, and stop.
That is a short way to bring the libraries in when what you want next is
`pretty-print` or `da` written out in full.

### Looking things up

`,ap <name>`

List the names that contain `<name>`, grouped by where they come from. This is
`apropos` from `(skint apropos)`, which is fetched the first time. The name may
be given as a symbol or as a string:

```
skint] ,ap string-for
interaction environment:
  string-for-each
(scheme base):
  string-for-each
(skint):
  string-for-each
```

[doc/skint/apropos.md](skint/apropos.md) describes what is listed and what is
left out. With nothing after it, `,ap` fetches the library, says so, and stops.

The commands below are for finding out what the interpreter currently thinks a
name means. What they print is an internal object written out, so treat the
shape of it as something to read rather than something to depend on.

`,ref <name>`

What `<name>` means where you are typing — its binding, and inside that binding
either a reference to a variable, a macro, a built-in operation, or one of the
core syntactic keywords:

```
skint] (define (f x) (car x))
skint] ,ref f
#&(ref repl://?f)
skint] ,ref lambda
#&lambda
```

`<name>` may also be a library name in parentheses, and then what you get is the
library, or `#f` if there is no such library loaded.

*Asking about a name you have never used creates it.* Skint gives a global a
binding the first time anything mentions it, and `,ref` mentions it. So `,ref`
on a misspelling leaves the misspelling behind as a variable with no value.

`,gs <name>`

The location a global variable's value lives in, or `#f` if the store has never
heard of the name. The name here is the *global name*, which for a built-in is
the identifier itself and for something you defined at the prompt has a
`repl://?` in front of it:

```
skint] (define zz 1)
skint] ,gs repl://?zz
#&1
skint] ,gs car
#&#<procedure ...>
```

Unlike `,ref`, this creates nothing.

`,gs`

The whole store, with every global in it. It is very large.

### Name registries

Skint keeps the names it knows in two tables: one for everything that came with
the interpreter or arrived by importing, and one for what you have defined at the
prompt. Your table is consulted first, which is what lets a definition of yours
shadow a built-in name.

`,rref <name>`
<br>`,uref <name>`

Look `<name>` up in the first or the second, without creating anything. `#f` when
it is not there — which is itself the answer to "which of the two is this name
in?":

```
skint] (define zz 1)
skint] ,uref zz
#&(ref repl://?zz)
skint] ,rref zz
#f
skint] ,rref car
#&181
```

The number in that last one identifies a built-in operation; which number goes
with which operation is not fixed.

`,rrem! <name>`
<br>`,urem! <name>`

Remove `<name>` from that table, so that the interpreter stops knowing it.
Removing one of your own definitions is how you get back to the built-in meaning
of a name you have shadowed. Removing a built-in name is possible and rarely a
good idea.

`,rnr`
<br>`,unr`

Print the whole of one table or the other. The first is very large.

### Timing and the working directory

`,time <expression>`

Evaluate the expression, print its value, and then say how long it took:

```
skint] ,time (length (make-list 200000))
200000
; elapsed time: 8.0 ms.
```

`,pwd`
<br>`,cd <directory>`

Show or change skint's working directory, which is what relative filenames are
resolved against. The directory name needs no quotes.

`,sh <command line>`

Hand a command line to the system shell and wait for it. Again, no quotes:

```
skint] ,sh ls -l
```

### Messages, memory and version

`,q`
<br>`,q-`

Turn off, and back on, the informational notes skint prints while it works —
which library it is fetching, how many bindings an import brought in. Errors are
not affected.

`,v`
<br>`,v-`

Turn a much more detailed running commentary on and off. This is for looking at
what the expander and compiler are doing and is not otherwise useful.

`,si`

Version, features, library search path, and how much work the memory manager has
done so far.

`,gc`

Force a garbage collection, then print what `,si` prints. Collecting is also how
objects that are no longer reachable get finalized, so this is the way to close
ports you have lost track of.

### When a command is not understood

A name that is not a command, or a command given the wrong number or wrong sort
of arguments, is reported and nothing happens:

```
skint] ,ref car cdr
syntax error in repl command
type ,help to see available commands
```

`,help`
<br>`,h`

The list of commands, one line each.
