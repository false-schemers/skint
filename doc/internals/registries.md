## Name registries: where identifiers get their meanings

[store.md](store.md) describes the run-time half of naming: a table mapping global
names to locations that hold values. This page describes the expand-time half. A
*name registry* maps an identifier to its *denotation* — what the expander needs to
know in order to expand a form that mentions it. Registries are consulted only
while expanding; nothing in a running program looks at one.

Everything here lives in `pre/t.scm`. The comment block at the head of the
expander, just above `location?`, is the authority on the grammar of denotations,
and the section headed *Expand-time name registries* holds the table itself.

### Denotations are locations

An environment never hands back a value. It hands back a *location* — a mutable
cell holding the value:

```scheme
(define-syntax  location?            box?)
(define-syntax  make-location        box)
(define-syntax  location-val         unbox)
(define-syntax  location-set-val!    set-box!)
```

A location is an ordinary box, and the aliases are `define-syntax` so they cost
nothing. The indirection is the point: `define-syntax` on an already-bound name
changes the value in place, and every environment that has handed that location out
sees the change without being rebuilt.

What a location can hold:

| Value | Means | Example |
|---|---|---|
| a symbol | one of the expander's own builtins | `define`, `lambda`, `body`, `import` |
| a fixnum | an index into the integrables table | `car`, `cons` |
| a procedure | a macro transformer, called with the form and the use environment | `let`, `case` |
| a vector | a library: its initialization code and its export list | `(scheme base)` |
| a void | allocated but not yet given a meaning | a name being defined |
| a pair | Core Scheme — a reference to a variable | `(ref repl://?x)` |

The Core case is the common one, and it comes in two forms. `(ref <name>)` is an
ordinary variable: it can be read, assigned and `set&`'d. `(const <name>)` is the
immutable variant — `xpand-set!` rejects it — and it is what the built-in names
get, so a program cannot assign to `exact-integer-sqrt`. `<name>` in both is a
global name in the store's sense, so this is the join between the two tables; see
[Registries and the store](#registries-and-the-store).

A denotation that is not a pair is called a *special*, and `location-special?` is
the test. `xpand` returns one only in operator position; elsewhere an integrable
in value position is converted to `(const <name>)` so that it can be passed around
as a procedure.

### A registry is a hash table with one extra bucket

```
    registry:  #( bucket bucket ... bucket listnames )
    bucket:    ((symbol . location) ...)
    listnames: ((library-name . location) ...)
```

`make-name-registry` takes an approximate size and rounds it up to the next prime
from a fixed list, then adds one bucket:

```scheme
(make-vector (+ prime 1) '())   ; last bucket used for listnames
```

Symbolic names are distributed over the first `prime` buckets by
`immediate-hash`, the same hash the store uses — a symbol's own `obj` bits, which
are a small stable number the collector never changes. See
[store.md](store.md#interning-a-name).

*Library names are all in the last bucket.* A library name is a list, not a
symbol, so it has no immediate bits to hash; `name-lookup` sends every pair to
`(- (vector-length nr) 1)` and searches that alist with `assoc` instead of `assq`.
The bucket is one linear list, which is fine: there are far fewer libraries than
identifiers.

The operations, and one protocol worth knowing:

`(name-lookup nr name mkdefval)` → *location* or `#f`
<br>`(name-install! nr name loc)` → `same`, `modified` or `added`
<br>`(name-remove! nr name)`

`name-lookup` is both the lookup and the allocator. When the name is absent it
calls `mkdefval` with the name, and reads the answer three ways: `#f` means give up
and return `#f`; a location means one was found elsewhere, so adopt it *without*
installing anything; anything else is a fresh denotation to be wrapped in a new
location, installed and returned. `mkdefval` is allowed to run arbitrary code —
autoloading a library does exactly that — so the bucket is re-read afterwards, and
finding the name there now is reported as `recursive library dependence on`.

An export list has the same shape as a bucket, `((id . location) ...)`, which lets
the two be converted into each other. `eal->name-registry` builds a registry from
an export list, and it has a deliberate special case: asked for size 1 it returns
`(vector eal '())` — a two-bucket registry whose bucket 0 *is* the export list,
sharing its pairs rather than copying them. `eal-name-registry-import!` recognises
that shape and adds to it with `adjoin-eals`, which is the operation that reports
`multiple identifier bindings on import` when two imports disagree about a name.

### The three process-wide registries

| Registry | Holds |
|---|---|
| `*root-name-registry*` | everything the expander knows by default |
| `*hidden-name-registry*` | the internal names built-in macros expand into |
| `*user-name-registry*` | what has been defined interactively |

The root registry is created large, the user registry smaller, and *the hidden
registry is created with size 1 on purpose*: a one-bucket registry's bucket 0 is
its entire alist, which is exactly the shape of a library export list, so
`(skint hidden)` can export the very pairs the registry holds rather than a copy of
them. The comment on its definition says as much.

Library names go into whichever registry the request reaches, and both the root and
the user registry hold some — see
[Library names live in registries too](#library-names-live-in-registries-too).

### How they are filled at startup

[builtins.md](builtins.md#registering-names-for-the-expander) describes the passes
that populate the root registry: the integrables by index, the initial
transformers, and the long table that distributes names over libraries. Three
details of that belong here.

*A name exported by several libraries is one location seen several times.* The
table's `get-loc` interns the name in the root registry, defaulting its denotation
to `(const <name>)`, and `put-loc!` then puts that same location into each library's
export list. `car` in `(scheme base)` and `car` in `(scheme r5rs)` are not two
bindings that happen to agree; they are one cell.

*The cleanup pass hides what is not public.* After the table, every symbolic entry
of the root registry that is not in `(skint)`'s export list is removed from it and
installed in the hidden registry — the location is moved, not copied. What remains
in the root registry afterwards is exactly `(skint)`'s export list, which is why an
interactive session sees the library's names and nothing else.

*Built-in macros keep working anyway.* The transformers built at startup are closed
over `builtin-sr-environment`, which searches the hidden registry first and the
root registry second. Their free identifiers are resolved when the macro is *used*,
so moving a name out of the root registry does not break them:

```scheme
(case 2 [(1) 'one] [(2) 'two] [else 'other])   ; => two
```

`case` expands into `%case`, which is in the hidden registry and reachable from no
user environment at all. This is the whole reason the hidden registry exists.

Last, `(skint hidden)`'s own export list is assembled: the hidden registry's single
bucket, adjoined with whatever the table assigned to `(skint hidden)` directly, plus
a `(const <id>)` binding for every name then in the global store that is not
already accounted for. That final step is what makes the interpreter's internals
importable, and it is a snapshot — a global created later is not in it.

### Environments are closures over registries

An environment is a procedure of two arguments, an identifier and an *access
type*:

`(env <identifier> <access-type>)` → *location* or *registry* or `#f`

| Access type | Asks for | Allocates |
|---|---|---|
| `peek` | where the name resides, or would reside | never |
| `ref` | the location, for reading | yes, on most environments |
| `set!` | the location, for assignment | yes, but must not shadow |
| `define` | the location, for a definition | yes, and may shadow |
| `define-syntax` | the location, for a syntax or library definition | yes, and may shadow |
| `import` | to merge an export list in | installs the list |

`#f` means refusal, and `xenv-lookup` is the wrapper that turns it into an error —
`library not found` for a list name asked for by `ref`, and
`transformer: invalid identifier access` for everything else. That second message
is what `(set! car 1)` produces at the REPL: assignment is not allowed to shadow a
root-registry name, so the environment refuses.

*`peek` is the only non-allocating access,* and it answers two ways: with the
location when the name is bound, and with *the registry the name would go into*
when it is not. Both halves matter.

The first makes `peek` the way to ask a question about an environment without
changing it. Environments are closures and cannot be enumerated, so asking is all
there is; `ref` would intern every name asked about. `(skint apropos)` is built on
this.

The second is what lets a renamed identifier put off having a binding. Identifiers
produced by `syntax-rules` are not symbols — they are thunks, and `new-id?` is the
test — and a thunk cannot be a registry key. `new-literal-id` therefore records the
`peek` answer inside the identifier, and if the identifier is later used freely and
does need a location, `gensym-ref-value-helper` interns one in the registry that
was recorded, under a gensym for a definition and under the plain name for a
reference. Most renamed identifiers never reach that point, which is the whole
intent: `new-id-lookup` returns the recorded registry unchanged for a `peek`, and
only converts to a real binding when one is unavoidable.

Environments over registries are built by five constructors:

| Constructor | Reads | New names go to |
|---|---|---|
| `make-readonly-environment` | one registry | nowhere — `ref` does not allocate |
| `make-controlled-environment` | an import list, plus a fallback environment for list names | a local registry, or the store under a prefix |
| `make-sld-environment` | the root registry | nowhere; list names autoload |
| `make-repl-environment` | a user registry, then a root registry | the user registry |
| `make-historic-report-environment` | one library's export list | as for a controlled environment |

`make-controlled-environment` is the one libraries and programs get. It keeps two
registries of its own — an import registry built from the import list and a local
registry for what the body defines — and refuses to let a definition shadow an
import. Its third argument is consulted only for list names, so that a library body
can still refer to other libraries.

Lexical scope does not involve registries at all. `extend-xenv-local` builds a
one-binding closure in front of an existing environment, so a nest of `lambda`s is
a chain of closures; only names that escape to the top level need a registry entry.

### The standard environment procedures

| Expression | Is |
|---|---|
| `(interaction-environment)` | `repl-environment`, over the user and root registries, prefix `repl://` |
| `(scheme-report-environment 5)` | a controlled environment over `(scheme r5rs)`'s exports, prefix `r5rs://` |
| `(null-environment 5)` | the same over `(scheme r5rs-null)` |
| `(environment <import set> ...)` | a read-only environment over a one-bucket registry built from those imports |

`(environment ...)` is worth a note. It processes the import sets with the root
environment, runs their initialization code immediately, then wraps the resulting
export list in `eal->name-registry` at size 1 and makes it read-only. Because that
registry's list-name bucket is empty and `make-readonly-environment` does not
allocate, list names cannot be resolved in it — such an environment is for `eval`,
not for further imports or library definitions.

The prefixes in that table only apply to names an environment *allocates*. An
imported name keeps the location it was imported with, so `(scheme-report-environment 5)`
resolves `car` to the same integrable denotation the root registry has, not to
`(ref r5rs://?car)`.

### Library names live in registries too

A library is a value like any other — a two-slot vector of initialization code and
an export list — and it is bound to a list name in a registry's last bucket. Four
places in `pre/t.scm` connect a list name to a library, and three of them carry a
comment saying so — `part n/4 of listname <-> library interaction`: resolving an
import set, `xpand-define-library`, and `xpand-import`. The fourth, unmarked, is
the top-level installation in `evaluate-top-form`.

Which registry a library lands in depends on how it arrived:

- *Predefined* libraries are interned by the startup table, in the root registry.
- *Loaded* ones arrive through `fetch-library`, which is the `mkdefval` the
  repl and sld environments pass to `name-lookup` for an unknown list name. It
  finds the `.sld` file, expands the `define-library` form in a
  `make-sld-environment`, and returns the library — so a successful load interns
  it, and a failed one leaves the name unbound so that the file can be fixed and
  retried. The root registry again.
- *Defined* ones — a `define-library` form typed at the top level — go through
  `(env <listname> 'define-syntax)`, and the repl environment allocates for that
  in the *user* registry.

A tool that wants every library the system knows about has to look in both last
buckets, and take the user registry's entry first, since it shadows.

An interactive `import` is a case of its own. The repl environment implements the
`import` access type by walking the export list, doing `name-remove!` on the user
registry and `name-install!` on the root registry for each name, and returning the
counts of bindings that were the same, modified and added — which is where the
REPL's `; import: 0 bindings are the same, 0 modified, 9 added` line comes from. The
`name-remove!` half has a visible consequence: an import silently discards an
interactive definition of the same name.

```scheme
> (define pretty-print 'mine)
> (import (skint print))
; fetching (skint print) library from lib\skint\print.sld
; import: 0 bindings are the same, 0 modified, 9 added
> pretty-print
#<procedure ...>
```

### Registries and the store

The two tables are consulted at different times and hold different things, and
[store.md](store.md#the-store-is-not-an-environment) draws the line. The join is
the `(ref <name>)` and `(const <name>)` denotations: a registry says which global
*name* an identifier stands for, and the store says which run-time *location* that
name is filed under. Each environment carries a `global` function that decides the
name — `repl://?x` for the REPL, `lib://skint/print?x` inside a library — and once
it has been applied the identifier is gone.

Two levels of indirection, then, and shadowing works at the registry level only:

```scheme
> (define car 2)
> car
2
> (cons 1 2)
(1 . 2)
```

The new binding is a user-registry entry denoting `(ref repl://?car)`, which is a
different global name, and so a different store location, from `car`. Code already
compiled holds the location for `car` itself and goes on using it. Assigning to a
global is the opposite case: there the location is the same one, so every compiled
reference sees the new value.


### What is fixed and what grows

A registry's bucket count is chosen when it is created and never changes; buckets
grow. Nothing rehashes, and nothing shrinks except `name-remove!`, used by the
interactive `import`. The hidden registry's single bucket is a linear scan by
design, and it is only ever consulted while expanding a built-in macro.

Locations are never discarded. Moving a name between registries moves its location,
and a name removed from one registry is either being installed in another or was
just shadowed by an import — so a denotation handed out earlier stays valid.
