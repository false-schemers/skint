## Inlining: built-in names compile to instructions

Most built-in procedures are *integrables*: procedures the compiler can turn into
an instruction instead of a call ([bytecode.md](bytecode.md#integrables) has the
instruction-table side, [builtins.md](builtins.md) how the table is built). This
page follows a use of such a name from the expander to the bytecode, and then
collects what that decision means for redefining built-ins, for the REPL, and for
the debugger.

Up: [internals index](README.md).

### A built-in name denotes an instruction

The decision is made at expansion time, from the name's denotation alone — the
global store is not consulted. A built-in integrable is registered in the root
registry with a fixnum denotation, its index into the integrables table
([registries.md](registries.md#denotations-are-locations)):

```
skint] ,rref car
#&181
```

The number is not fixed; it moves when the instruction set changes. Nothing is
inlined by guesswork about what a variable currently holds: a name inlines exactly
when the environment the expander is working in says it denotes an integrable.
Every environment that imports `car` from a standard library gets that same
denotation, so library code inlines as the REPL does.

Not every built-in qualifies. The `@` arity class is never inlined (`values`,
`call-with-values`, `apply-to-list`), and built-in procedures that are not
integrables at all denote `(const <name>)`, an immutable reference to a global.

### Calls

`xpand` returns the fixnum only for a name in operator position; everything else
has to be Core. An application whose operator is an integrable goes to
`xpand-integrable` in `pre/t.scm`, which asks `integrable-argc-match?` whether the
argument count fits the integrable's arity class. If it does, the call becomes an
`integrable` Core form and codegen emits the instruction; if not, it becomes an
ordinary call of the global procedure, which checks its arguments when it runs.
Nothing is rejected at expansion either way.

| Form | Core | Bytecode |
|---|---|---|
| `(car x)` | `(integrable 181 (ref x))` | `@(y9:repl://?x)a` |
| `(car x y)` | `(call (ref car) (ref x) (ref y))` | `${@(y9:repl://?y),@(y9:repl://?x),@(y3:car)[02}` |
| `(+ a b c)` | `(integrable 326 (ref a) (ref b) (ref c))` | `@(y9:repl://?c),@(y9:repl://?b),@(y9:repl://?a)++` |
| `(< a b c d)` | `(integrable 334 (ref a) (ref b) (ref c) (ref d))` | `@(y9:repl://?d),@(y9:repl://?c),,@(y9:repl://?b),,@(y9:repl://?a)<;<;<` |
| `(let ([car cdr]) (car x))` | `(call (lambda (car#11) (call (ref car#11) (ref x))) (const cdr))` | an ordinary call |

(The global names are those of the REPL; the renamed local is written without its
bars.) The last row is hygiene doing its job: inside the `let`, `car` denotes a
local variable, so there is nothing to inline.

*A comparison of several arguments is one instruction chain.* `(< a b c d)` pushes
every inner argument twice, compares the first pair, and each `;` (`andbo`)
consumes the copy, runs the next comparison on the original, and stops the chain
at the first false result. It is still a single expression, and a failure at any
step — an argument that is not a number — is a failure of the whole comparison.
`lib/skint/disasm.sld` reads the chain back as one n-ary `integrable` node, grown in
place at each step so that a cursor at any step's end means that node.

### Values

Anywhere but operator position, `xpand` turns the fixnum into `(const <name>)`:

| Form | Core |
|---|---|
| `(map car l)` | `(call (const %map1) (const car) (ref l))` |
| `(apply car l)` | `(call (const apply-to-list) (const car) (ref l))` |

`(const car)` is the global procedure `rds_intgtab` synthesized from the
instruction's encoding at startup ([bytecode.md](bytecode.md#global-procedures-for-free)),
so a built-in passed as a value is a real procedure with a frame of its own when it
runs. For most arity classes that procedure is template bytecode with no Core
preimage, which is why `(da 'car)` answers only the name `car`. The folding and
comparison classes loop over a rest argument, and their procedures do read back as
Scheme.

### Nothing can change a built-in

Inlined code holds the instruction, not a reference to a location, so there is
nothing a later assignment could reach. The expander therefore refuses to pretend
otherwise: `set!` of a built-in name is rejected at expansion, whether the name
denotes an integrable or a `(const <name>)`, in every environment.

In a program run with `--program` and in a library, `define` of an imported
built-in name is rejected as well, and so is `set!`. That is R7RS's rule: section
5.2 makes it an error in a program or library to redefine or mutate an imported
binding.

The same section goes on: "However, a REPL should permit these actions." Skint's
REPL — and a script run with `skint file.scm` or `--script`, which is evaluated in
the same interaction environment — permits the definition but not the assignment,
and the definition does not change the built-in. It installs a new binding in the
user registry, `(ref repl://?car)`, a different global
([registries.md](registries.md#registries-and-the-store)). Code expanded after it
calls that global; code expanded before it keeps the instruction:

```scheme
(define (early l) (car l))         ; compiled to the car instruction
(define (car x) 'mine)
(define (late l) (car l))          ; compiled to a call of repl://?car
(list (early (list 5)) (late (list 5)))   ; => (5 mine)
```

Built-in code is unaffected — `assq` still uses the instruction — and removing the
user binding with `,urem! car` gives new code the built-in again, while `late` goes
on calling `repl://?car`. R7RS also allows a REPL whose top-level definitions act
as assignments to locations that already exist; skint's are new bindings instead,
which is what inlining requires.

The two procedures above disassemble alike, because `da-prune-globals` shows a
global by its identifier:

```scheme
(da early)                                        ; => (lambda (.a) (car .a))
(da late)                                         ; => (lambda (.a) (car .a))
(parameterize ([da-prune-globals #f]) (da late))  ; => (lambda (.a) (repl://?car .a))
```

`da-code` shows the difference too: `%1.0a]1` for `early`, a call of
`repl://?car` for `late`.

### What the debugger sees

An inlined call makes no frame, because nothing is called. A failure in one is
reported in the frame whose code holds the instruction, and the offset in that
frame is just past the instruction that failed, so the debugger marks the whole
integrable expression:

```
skint] (< 1 2 3 'a 5 6)
Failure in vm:
argument is not a number a
Type ,db to enter the debugger.
skint] ,db
Failure: number a
  0: #<procedure @0x7f980b2c4320> @18
     >(lambda () [< 1 2 3 'a 5 6])
```

There is no frame for `<`, and the `,da <` listing plays no part, because the
procedure `<` was never called. When a built-in *is* called as a procedure — passed
to `map`, applied — its global procedure has a frame like any other. For `car` that
frame shows the name and no code, since the template has no Core preimage; for `<`
it shows the loop of the comparison procedure, with its own inlined `<` marked.
