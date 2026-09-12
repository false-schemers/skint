## Builtins: the instruction tables and the bindings built from them

`i.h` is the single source of truth for everything the interpreter provides in C.
One line in it can produce a VM instruction, a bytecode encoding, an entry the
compiler inlines, and a first-class Scheme procedure. No other file carries a second
copy of any of that; the only thing a new builtin needs elsewhere is a line in the
library export table, which says nothing about the builtin except where its name is
visible. This page describes how the file is swept into tables, what each table is
for, and how a name in it becomes something a Scheme program can call.

[bytecode.md](bytecode.md) covers the column meanings, the operand types and the
naming conventions that keep the encodings decodable; [vm.md](vm.md) covers how an
instruction runs. This page is about the machinery around them.

### What one line can produce

A line in `i.h` is either a `declare_instruction` or a `declare_integrable`, and
what it produces depends on which columns are filled in.

| Population | Line | `enc` | `igname` | Produces |
|---|---|---|---|---|
| VM machinery | `declare_instruction` | set | `NULL` | an instruction and an encoding |
| builtin procedure | `declare_instruction` | set | set | that, plus inlining and a global |
| inline-only builtin | `declare_instruction` | set | set, `lcode` is `INLINED` | that, but no global |
| aliased builtin | `declare_integrable` | set | set | inlining and a global, no instruction |
| plain global | `declare_integrable` | `NULL` | set, `lcode` explicit | a global procedure only |

The distinction that matters at run time is the `arity` column, not the macro. An
entry whose `arity` is `0` — the integer zero, not the character `'0'` — is
machinery and is not an integrable; every real class is a printable character, and
the code tests `igtype >= ' '` to tell them apart.

`declare_integrable` exists for the cases where an instruction would be redundant.
`modulo` and `quotient` are other names for instructions that already exist, so
they reuse an encoding; `caddr` and its relatives compile to a run of `car`/`cdr`
instructions, so their "encoding" is several instructions long and there is nothing
to implement. Both still need a global procedure and both are still worth inlining.
The last row is different again: `values`, `call-with-values` and `apply-to-list`
need a global procedure and nothing else, so they carry an explicit `lcode` and no
encoding at all.

### The four sweeps

`i.h` is an X-macro table. It opens by defining `declare_instruction` and
`declare_integrable` according to whichever `VM_GEN_*` macro is set, and closes by
undefining them, so it can be included repeatedly:

| Mode | `declare_instruction` becomes | `declare_integrable` becomes |
|---|---|---|
| `VM_GEN_DEFGLOBAL` | `declare_instruction_global(name)` | nothing |
| `VM_GEN_ENCTABLE` | `declare_enctable_entry(name, enc, etyp)` | nothing |
| `VM_GEN_INTGTABLE` | `declare_intgtable_entry(enc, igname, arity, lcode)` | the same |
| none | `extern obj cx_ins_2D<name>;` | nothing |

`declare_integrable` expanding to nothing in the first two modes is the whole
mechanism behind the aliases: an entry with no instruction of its own gets no
instruction global and no trie entry, so reusing another instruction's encoding
cannot collide with it.

The file also supplies defaults for three helper macros unless the includer has
already defined them: `glue` for token pasting, `AUTOGL` as `NULL`, and `INLINED`
as the empty string. That is why `AUTOGL` and `INLINED` read as words in the table
but are really just two distinct values of the `lcode` column.

#### Where the sweeps sit, and why

The order in `i.c` is forced by what each sweep needs:

1. every `define_instruction` body, because
2. the `VM_GEN_DEFGLOBAL` include takes the address of each one, then
3. `intgtab`, which needs neither — it holds only strings — and last
4. `enctab`, which holds the *address of* each instruction global.

An instruction whose body needs one of the tables therefore cannot simply name it.
`instruction-table` is the case in point: its body has to sit above the global
definitions, and `enctab` sits below them, so it reaches the table through two
forward-declared accessors, `enctab_count` and `enctab_ref`. The comment above
those two says as much; keep it if you move anything.

The instruction globals themselves are `obj` variables holding either the
function's address or the address of a one-element array holding it, depending on
`VM_INS_CODE_ALIGNED` — see [vm.md](vm.md#the-code-representation). Either way the
value points outside the Scheme heap, so the collector classifies it as Foreign and
never traces it. They are deliberately *not* in the root list in `k.c`.

### enctab and the decoder's trie

`enctab` is an array of `{ obj *pg; const char *enc; int etyp; }` terminated by a
`NULL` entry. It stores a *pointer to* each instruction global rather than its
value, because the globals are initialised by the C startup code while the table is
a static initialiser.

The trie the decoder actually consults, `encmap`, is built lazily from `enctab` on
the first call to `rds_prefix`, which is the first `vm_decode` in `k.c`. It is
`calloc`ed node by node and never freed; there is one of it per process.

Two properties of `encmap_add` are worth knowing before editing the table:

- It copies characters up to the first NUL, so only the first of several
  NUL-separated alternate encodings enters the trie. The later fields are reachable
  only through `integrable-code`.
- If two entries reach the same node it prints `encmap[dup]` and exits. A duplicated
  encoding is therefore a fatal startup failure rather than a silent mis-decode —
  but it fails at the first decode, not at compile time.

### intgtab and what an integrable is

`intgtab` is an array of
`{ int sym; char *igname; int igtype; char *enc; char *lcode; }`, with `sym` filled
in and the array sorted in place the first time anything looks at it.
`sort_intgtab` interns every `igname` and sorts by symbol, so lookup by name is a
`bsearch`; entries with no `igname` intern as zero and collect at the front.

**An integrable is identified by its index into the sorted array.** That index is a
run-time handle and nothing more: it is not stable across builds, and nothing
serializes it. A Core `integrable` form holds one, but Core lives only in memory —
`codegen` writes the *encoding*, never the index, so no index ever reaches a
bytecode string or a `.c` file.

`isintegrable` accepts an index only if the entry has a printable `igtype`, an
`igname` and an `enc`. That last condition is what excludes the `'@'` entries: they
carry a name and an explicit `lcode` but no encoding, so they are ordinary global
procedures the compiler will never inline. `(lookup-integrable 'values)` is `#f` for
exactly that reason.

#### Reflection from Scheme

`(skint hidden)` exposes the tables, mainly so that tools like a disassembler can be
written in Scheme rather than in C:

| Procedure | Returns |
|---|---|
| `(integrable? x)` | whether `x` is an integrable handle — total, never signals |
| `(lookup-integrable x)` | by symbol: the handle or `#f`; by index: the handle, `#<void>` for a machinery slot, or `#f` out of range |
| `(integrable-type ig)` | the arity class, as a character |
| `(integrable-global ig)` | the Scheme name, as a symbol |
| `(integrable-code ig n)` | encoding field `n`, or `#f` |
| `(instruction-table)` | a flat vector, three elements per `enctab` entry: the instruction word, its encoding or `#f`, and its `etyp` |
| `(closure->vector p)` | a closure's cells: the code vector first, then its display |
| `(closure? x)` | whether `x` is a VM closure: a heap block whose cell 0 is a code vector |

`closure?` is the test to use before `closure->vector`, not `procedure?`. The
procedure test comes from the runtime SKINT inherited, where a procedure without a
display is a foreign pointer, so in some builds `procedure?` answers `#t` for any
aligned pointer outside the heap — an instruction word included. R7RS allows that,
since the only thing a program can do with a procedure is call it. `closure?`
answers the same in every build.

Only `integrable?` and the index form of `lookup-integrable` are total. The three
accessors begin with `ckg`, so they signal on anything that is not an integrable,
in range or not. `integrable-code` answers `#f` for field 1 unless the arity class
is one of `p m b u t` — those are the only classes with an alternate encoding.

### Global procedures built at startup

Every integrable has to exist as a first-class procedure as well, because it can be
passed to `map`. Rather than write those by hand, `rds_intgtab` synthesizes each one
*as bytecode* from the instruction's own encoding, before any library code runs:
`vm_install_global_lambdas`, which `k.c` calls at startup, is a direct call into
it.

The `lcode` column decides how:

- `AUTOGL` — build the code from a template chosen by the arity class.
- `INLINED` — the empty string; skip the entry entirely. The type checks `%ckp`,
  `%ckl` and their kin are only ever compiled inline, so they get no global. Using
  one in value position yields the unassigned global, which is to say the symbol
  itself.
- an explicit string — use it verbatim. Required for classes `#` and `@`, which have
  no template and assert if one is asked for.

The fixed-arity templates:

```c
case '0': sprintf(lbuf, "%%0%s]0",   pe->enc);  /* (f)       */
case '1': sprintf(lbuf, "%%1_!%s]0", pe->enc);  /* (f x)     */
case '2': sprintf(lbuf, "%%2_!%s]0", pe->enc);  /* (f x y)   */
case '3': sprintf(lbuf, "%%3_!%s]0", pe->enc);  /* (f x y z) */
case '4': sprintf(lbuf, "%%4_!%s]0", pe->enc);
case '5': sprintf(lbuf, "%%5_!%s]0", pe->enc);
```

`%1` checks for exactly one argument, `_!` pops it into the accumulator, the
encoding follows, and `]0` returns dropping nothing. For `car`, whose encoding is
`a`, that is `%1_!a]0`.

The optional-argument classes use the alternate encoding field to supply what the
omitted argument would have meant, and end in `%%` — `aerr` — so a wrong argument
count fails there rather than falling off the end:

```c
case 'u': sprintf(lbuf, "%%!0.0u?{%s%s]1}.0du?{.0a%s]1}%%%%", pe1, pe0, pe0);
case 'b': sprintf(lbuf, "%%!1.0u?{%s,.2%s]2}.0du?{.0a,.2%s]2}%%%%", pe1, pe0, pe0);
case 't': sprintf(lbuf, "%%!2.0u?{%s,.3,.3%s]3}.0du?{.0a,.3,.3%s]3}%%%%", pe1, pe0, pe0);
```

`read-u8` is declared `"R3\0Pi"`, so its zero-argument arm is `Pi` — fetch the
current input port — followed by the ordinary `R3`.

The folding and comparison classes are longer: each builds a rest-argument loop
closing over a boxed local so the loop can call itself.

| Class | Shape |
|---|---|
| `p` | zero or more, folded; no arguments yields field 1 |
| `m` | one or more; one argument yields field 1 applied to it |
| `c` | two or more, chained comparison, short-circuiting on `#f` |
| `x` | one or more, folded, `min`/`max` shape |

Field 1 need not be a single instruction. `fx/` is declared `"I/\0,'1I/"`: its
one-argument arm pushes the argument, loads the literal `1` and divides, which is
the reciprocal.

However the string is arrived at, the rest is the same for every entry: intern the
name, find or create its box in `*globals*`, decode the string into a code vector
with the same `rds_seq` the compiler's output goes through, wrap it in a closure,
and store that in the box. The synthesized procedures are ordinary closures;
nothing distinguishes them at run time from ones the compiler produced.

One hard limit is worth remembering: the templates are formatted into a 200-byte
buffer. A long encoding in a folding class is the only realistic way to overflow it.

### The startup sequence

`run_kernel` in `k.c` is the whole of the bootstrap, and its top-to-bottom order is
the order in which things become available:

1. `*globals*` is created — a vector of buckets, each a list of `(sym . #&sym)`
   pairs. An unassigned global's box holds its own symbol, which is why referring to
   a name that was never defined yields the symbol rather than an error.
2. `*transformers*` starts empty, and `callmv-adapter-closure` is built by decoding
   the two-character string `K5`.
3. `vm_install_global_lambdas` runs `rds_intgtab`: every integrable's global
   procedure now exists.
4. `vm_initialize_modules` runs `init_module` over `i_code`, then `s_code`, then
   `t_code`.
5. The repl thunk is decoded from `${@(y4:repl)[00}` and executed, and re-executed
   from scratch if it returns anything but `#t`, which is how an error exit lands
   back at a fresh prompt.

Step 3 has to precede step 4 because the library sources call the builtins.

The `vm_*` functions are `i.c`'s entry points for exactly this code, and `k.c` is
their only caller. They stand where `k.c` used to reach into a table of
host-procedure cases, back when it was generated from `pre/k.sf`.

#### Module tables

`i_code`, `s_code` and `t_code` are `char *` arrays of name/data pairs terminated by
two `NULL`s. A pair may be preceded by a one-character tag saying how to install it:

| Tag | Name | Data | Effect |
|---|---|---|---|
| `P` | global name | bytecode | decode, close, and store in the global's box |
| `S` | keyword | serialized sexp | install as a transformer in `*transformers*` |
| `A` | new keyword | old keyword | copy an existing transformer under another name |
| `B` | keyword | keyword or `0` | bind a keyword to itself, or to another, as a builtin denotation |
| `C` | `0` | bytecode | decode and execute as a thunk, for its effect |
| `K` | `0` | bytecode | decode into `continuation-adapter-code` |

An untagged pair is treated as `S`, and an untagged pair with a `NULL` name as `C`.

`i_code` is hand-written and short. It installs the special-form keywords the
expander needs before anything can be expanded — `quote`, `lambda`, `letcc`, `...`
and the rest — sets the three current-port variables, and supplies the three
hand-coded procedures whose bytecode no compiler could have produced:
`%dynamic-state-reroot!`, `dynamic-wind`, and the continuation adapter. Each of
those three carries a comment explaining its bytecode instruction by instruction.
Together with a handful of one-liners in `k.c` and the port initialisation
just above them, they are all the hand-written bytecode there is, so the comments
are load-bearing.

`s_code` and `t_code` are generated from `pre/s.scm` and `pre/t.scm` by
`pre/scm2c.ssc`, which expands each top-level form and picks a tag from the result:

- `(define name (lambda ...))` whose compiled form is a plain `&0{...}` closure
  becomes a `P` entry holding just the block contents. Anything else becomes a `C`
  entry for `(set! name value)`.
- `(define-syntax name value)` becomes an `A` entry when the value is a bare symbol,
  and an `S` entry otherwise.
- Everything else becomes a `C` entry.

The `B` tag is not emitted by the generator; it appears only in the hand-written
`i_code`.

### Registering names for the expander

Being in `intgtab` is not enough to make a name visible to Scheme code — the
expander looks names up in a registry, and `pre/t.scm` populates it near the end of
the file, in three passes.

The first walks `intgtab` by index and registers every integrable under its own
name:

```scheme
(let loop ([i 0])
  (let ([li (lookup-integrable i)])
    (when li
      (when (integrable? i)
        (let ([name (integrable-global i)])
          (when (symbol? name) (name-lookup *root-name-registry* name (lambda (name) i)))))
      (loop (+ i 1)))))
```

The loop runs until `lookup-integrable` returns `#f`, which is why the index form
distinguishes "in range but not an integrable" (`#<void>`) from "out of range"
(`#f`): the first must not stop the walk.

The second pass walks `(initial-transformers)` — the `*transformers*` list that
`i_code`, `s_code` and `t_code` filled in — and registers the macros.

The third distributes the names over libraries. A long table maps each name to the
libraries that export it, by one-letter key: `b` is `(scheme base)`, `v` is
`(scheme r5rs)`, `a` is `(scheme cxr)`, and so on; a bare integer means `(srfi n)`,
and a name with no keys at all goes to `(skint)`. Anything still left in the root
registry afterwards is moved to `(skint hidden)`, which is where the reflection
procedures above are found.

A name that reaches Scheme but is never registered is not an error — it is simply
not visible.

### Conditionally compiled builtins

A few instructions exist only when the host provides the underlying C functions.
They are bracketed in `i.h`:

```c
#ifdef C99_MATH_LIB
declare_instruction(jcopysign,  "M10", 0, "flcopysign",      '2', AUTOGL)
...
#endif
#ifdef XSI_MATH_LIB
declare_instruction(jjn,        "M50", 0, "flfirst-bessel",  '2', AUTOGL)
declare_instruction(jyn,        "M51", 0, "flsecond-bessel", '2', AUTOGL)
#endif
```

`C99_MATH_LIB` and `XSI_MATH_LIB` are decided in `n.h`, from feature-test macros
that `s.h` selects — which is why `s.h` has to be included first by every
translation unit, and why `n.h`, `i.h` and `k.h` refuse to compile without it. A
module that got the configuration wrong would build its half of the tables against a
different instruction set from everyone else's, and nothing at run time would say
so.

Leaving them out of a build removes them from all three tables at once, so nothing
downstream breaks — but nothing downstream *knows*, either. Three separate places
have to agree:

1. `host_sig` in `n.c` writes a character into the host signature string for each
   switch: position 8 is `9` when `C99_MATH_LIB` is on, position 9 is `x` when
   `XSI_MATH_LIB` is.
2. `pre/s.scm` reads those positions and conses `c99-math` or `xsi-math` onto
   `*features*`.
3. The library table in `pre/t.scm` tags the affected names with `. c99-math` or
   `. xsi-math`, and the distributing pass skips a name whose tag is not in
   `*features*`.

Adding a fourth conditional group means touching all three, and regenerating `s.c`
and `t.c`.

`OPT_TOWER` is *not* one of these. It replaces the bodies of the generic math
instructions — `i.c` includes `opt/i_tower.c` in place of the flonum-only
definitions — but the instruction set, the encodings and the globals are identical
either way. A program cannot tell which build it is running on by looking at the
tables; it has to ask `*features*`.

### Adding a builtin

[bytecode.md](bytecode.md#changing-the-instruction-set) lists the steps for an
instruction. For one that is also a Scheme procedure, the extra work is:

1. Fill in `igname` and `arity`. If a template fits, leave `lcode` as `AUTOGL` and
   the global procedure appears by itself.
2. If the arity class is `#` or `@`, write the `lcode` string; there is no template,
   and the assert fires if you forget.
3. If the procedure takes an optional argument, add the alternate encoding after a
   NUL, and check that the class is one whose alternate field is read — `p`, `m`,
   `b`, `u` or `t`.
4. Add the name to the library table in `pre/t.scm` with the keys of the libraries
   that should export it, and regenerate `t.c`. Without this the procedure exists
   but no library exports it.
5. If the name is meant for users rather than for the implementation, document it
   in `doc/skint/`.

A new arity class, as opposed to a new builtin in an existing class, is more
work: the class has to be known in all four places listed in
[bytecode.md](bytecode.md#integrables) — the templates, the expander, the
compiler and the disassembler.
