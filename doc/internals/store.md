## The store: where global variables live

R7RS draws a careful line: an identifier names either a type of syntax or *a
location where a value can be stored*, and a variable is said to be bound to that
location rather than to the value in it. SKINT keeps that line sharp. Identifiers
and their meanings are an expand-time affair, resolved in environments; locations
are a run-time affair, and the ones that are not local live in a single table called
the *store*.

A variable that no enclosing binding form accounts for gets its location from the
store. Such variables are informally called *globals*, and the symbol under which
the store files a location is that global's *global name*.

### The table

The store is the value of the runtime variable `*globals*` — `cx__2Aglobals_2A` in C
after name mangling. It is an ordinary Scheme vector used as a hash table with
overflow lists:

```
    *globals*: #( bucket bucket bucket ... )
    bucket:    ((global-name . location) (global-name . location) ...)
    location:  a box
```

Every key is a symbol and every value is a box. Nothing else ever appears in it.

The vector is created with a fixed number of buckets — a prime, 991 — during the
bootstrap, and *it is never resized and never pruned*. Buckets simply grow. It is
one of the collector's roots, so every location and every value in it stays live for
the process; see [memory.md](memory.md).

The store is *flat*. A location filed under a built-in name, one belonging to a
library, and one a user typed at the REPL are the same kind of thing in the same
table, distinguished only by the shape of the name.

### Global names

Three populations, told apart by prefix.

*Built-in names are the identifiers themselves.* `car`'s location is filed under
the symbol `car`. These are created while the interpreter starts up — by
`rds_intgtab`, which builds a global procedure for every integrable, and by the
module tables that install the library written in Scheme; see
[builtins.md](builtins.md). After startup no further unprefixed name is ever
*generated*: the expander always prefixes what it allocates.

*Library names are URI-shaped.* A definition inside `(define-library (skint
print) ...)` is filed under `lib://skint/print?print-width`. The prefix is built by
`listname->symbol`: `lib:/`, then `/` and one segment per element of the library
name, then `?` and the identifier. An integer element contributes its decimal
digits, so `(srfi 1)` yields `lib://srfi/1?remove!`. A symbol element passes
through lowercase letters, digits and the characters `! $ - _ =` unchanged and
`%`-encodes everything else as two lowercase hex digits, which is what keeps a name
readable and unambiguous at once.

*Every* library-level definition gets such a name, whether or not it is exported.
Exporting controls which identifiers an importer may bind; it has no bearing on the
store. Two libraries can define the same identifier without colliding because their
prefixes differ, which is the whole point of the scheme.

Libraries loaded from a `.sld` file also own one prefix-only name with no `?`
segment — `lib://skint/print` — used as the initialisation flag; see
[Running a library body once](#running-a-library-body-once). Built-in libraries
such as `(scheme base)` have neither a flag nor `lib://` names: they were registered
during startup and re-export the unprefixed built-in locations.

*Everything typed interactively is prefixed too.* The REPL allocates
`repl://?name`, a program run as a script allocates `prog://<module>?name`, and the
historic `(scheme r5rs)` environments allocate `r5rs://?name`. These come from the
same routine as the library names, with a symbol standing in for the library path.

### Interning a name

`rds_global_loc` in `i.c` is the only routine that adds to the store:

```c
uint64_t v = (uint64_t)ra; int i = (int)(v % base);
obj p = isassv(ra, vectorref(cx__2Aglobals_2A, i));
if (ispair(p)) ra = cdr(p);
else { /* prepend (sym . #&sym) to *globals* */ ... }
```

Three things in that are worth spelling out.

*The hash is the symbol's own bits.* A symbol is an immediate carrying its index
in the interned symbol table, so its `obj` value is a small stable number that the
collector never changes — a perfectly good key, and free. It also means the bucket a
name lands in depends on the order symbols happened to be interned, so it is not
stable from one run to the next. The *name* is the durable identity; the index is
not.

*New entries are prepended.* A bucket is therefore in reverse order of creation,
and an entry, once made, never moves. A tool that has walked the store can watch for
new arrivals by remembering each bucket's head and scanning only what appears in
front of it.

*A fresh location is initialised to its own name.* `*--hp = ra` puts the symbol
into the box it is about to be filed under.

### Referencing something that was never defined

That last detail is a deliberate diagnostic. SKINT does not signal an error when a
variable turns out to have no value — R7RS permits an implementation to say nothing
— so the question becomes how to notice. Because an untouched location holds its own
global name, an unresolved reference evaluates to a symbol that says exactly what
went looking and where:

```scheme
> nowhere-defined
repl://?nowhere-defined
```

A value like that is almost never a legitimate one, so it is a reliable sign. It
shows up in the wild for a reference to a name a library did not in fact export,
which SKINT also declines to complain about:

```scheme
(import (only (skint getopt) parse-options))   ; not one of its exports
parse-options                                  ; => repl://?parse-options
```

Assigning to an undefined variable is not an error either; `set!` allocates the
location like any other reference.

### Names become locations once, at decode time

A global name reaches the bytecode as a serialized symbol operand — `@(y3:car)` —
and this is the one place where the operand type `'g'` in the instruction table
matters: the decoder resolves the symbol through `rds_global_loc` while it is
building the code vector, and stores *the box* in the vector, not the name.

```scheme
(deserialize-code "@(y3:car)]0")
;; the operand is car's own location
```

So the three instructions that touch globals never search for anything:

| Instruction | Encoding | Does |
|---|---|---|
| `gref` | `@` | `ac = boxref(operand)` |
| `gset` | `@!` | `boxref(operand) = ac` |
| `gloc` | `` ` `` | `ac = operand` — the location itself, for `set&` |

A global reference at run time is one indirection. A global assignment is one store.
`set&` of a global is free: the location is already sitting in the code.

A consequence worth keeping in mind: because the code vector holds the location and
not the name, redefining a global changes what every compiled reference sees — they
all share the one box — but nothing can ever *rebind* a compiled reference to a
different location.

Note that a name containing `%` is escaped in the bytecode, since `%XX` is how the
serializer spells characters it cannot write literally: `%for-each1` appears as
`y10:%25for-each1`. See [bytecode.md](bytecode.md#the-encoding).

### Running a library body once

The Core form `(once <global-name> <init>)` is how a library body is kept from
running twice, and it uses the store for its flag. It compiles as if it had been
written:

```scheme
(if (eq? (gref <global-name>) #t) (begin) (begin (gset! <global-name> #t) <init>))
```

The global name used is the library's prefix without a `?` segment. Before the
library is loaded that name is not in the store at all; the guard's own reference
interns it, and the fresh location holds its own name, which is not `eq?` to `#t`,
so the body runs and sets the flag. `lib://skint/print` holds `#t` from then on.

### The store from Scheme

`(skint hidden)` exposes the table and a faithful re-implementation of the interning
routine:

| Procedure | Returns |
|---|---|
| `(global-store)` | the bucket vector itself, not a copy |
| `(lookup-global name)` | the location, or `#f` if the name has none |
| `(lookup-global name #t)` | the location, allocating it if necessary |
| `(immediate-hash x base)` | `x`'s bucket index, the same one the C side computes |

`lookup-global` in `pre/t.scm` mirrors `rds_global_loc` line for line — same hash,
same prepend, same "initialise to the name" — and the two must be kept in step if
either changes.

Because a location is an ordinary box, this is full read/write access to any global:

```scheme
(define loc (lookup-global 'repl://?some-var #t))
(set-box! loc 42)
some-var                                        ; => 42
```

`immediate-hash` is only meaningful for immediates, which is what its name says: it
answers `0` for anything allocated in the heap, except that a boxed flonum hashes on
its bits. Since store keys are always symbols, and symbols are immediates, that is
enough.

The invariants a tool may rely on: keys are symbols, values are boxes, entries are
prepended and never moved, and the vector is never replaced. `(skint disasm)` uses
all four — it recovers a name from a location by walking the store, and keeps that
map current by rescanning only the fronts of buckets that changed.

### The store is not an environment

These are two different tables and it is worth not confusing them.

An *environment* maps an identifier to its expand-time *denotation*: a location
whose value is a Core expression, a macro transformer, an integrable, a library,
or one of the builtin keywords. It is consulted by the expander, it is scoped,
and it disappears once expansion is done. Environments are built over name
registries, which are the subject of [registries.md](registries.md).

The *store* maps a global name to a location. It is consulted at decode time and
at run time, it has no scope at all, and it lasts as long as the process.

The bridge between them is one function per environment — called `global` in the
expander — which turns an identifier into the global name its location will be filed
under. `make-repl-environment` builds `repl://?`-prefixed names, a library's
environment builds `lib://...?`-prefixed ones. Once the expander has applied it, the
identifier is gone: the Core form `(gref lib://skint/print?print-width)` carries a
global name, not an identifier, and needs no environment to be understood. The same
is true of `(ref <name>)` when the name is not a local — `ref` and `gref` compile
identically.

That is why bytecode is self-contained. A string of bytecode can be decoded and run
without knowing anything about the environments the expander was using, because
every name in it has already been resolved to something the store can file.

### Costs

The store never shrinks, so a long REPL session that defines many names keeps them
all, along with their values. The bound is the program's own size; nothing
accumulates on its own.

The fixed bucket count is a bet that the number of globals stays within a few
multiples of it. It is a good bet at present — with the built-in names and the
libraries loaded, the longest bucket in a fresh interpreter holds two entries — but a
much larger program would degrade the table into linear scans of a few dozen
entries. Nothing in the design prevents rehashing into a larger vector; it just has
not been needed. Only `rds_global_loc` and `lookup-global` would have to agree on
when to do it.
