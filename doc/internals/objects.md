## Scheme objects on top of the collector's five categories

[memory.md](memory.md) describes what the garbage collector sees: False, Immediate,
Block, Native, Foreign. That is too coarse to run Scheme on. This document
describes the layer that subdivides three of those categories into actual Scheme
types, and the macro and function layer in `n.h` that the rest of the system uses to
work with them.

Everything here is declared in `n.h`; the implementations that are not macros live
in `n.c`. Both are hand-maintained. Optional types are added by the headers under
`opt/`, which `n.h` includes conditionally.

### Immediates

Two shapes of immediate share the low bits of the word.

An *im0* has the low two bits `11` and thirty bits of signed payload. There is
exactly one im0 type — fixnums:

```c
#define isim0(o) (((o) & 3) == 3)
#define mkim0(v) (obj)((((v) & 0x3fffffff) << 2) | 3)
```

Under `NAN_BOXING` the test additionally requires the top 32 bits to be zero, since
a NaN-boxed flonum could otherwise collide. The range is
`FIXNUM_MIN` = −536870912 to `FIXNUM_MAX` = 536870911 in both memory models — a
fixnum is thirty bits regardless of word size, which is why the same bytecode and
the same literals work on 32-bit and 64-bit builds. `getim0s` re-signs the payload
on the way out.

A *tagged immediate* has the low two bits `01`, a secondary tag in bits 2–7, and
twenty-four bits of payload:

```c
#define isimm(o, t) (((o) & 0xff) == (((t) << 2) | 1))
#define mkimm(v, t) (obj)((((v) & 0xffffff) << 8) | ((t) << 2) | 1)
```

The tag field is six bits wide, so up to 64 immediate types are possible. Those
assigned so far:

| Tag | Name | Payload |
|---|---|---|
| 0 | `TRUE_ITAG` | none — `#t` is the single word `1` |
| 2 | `CHAR_ITAG` | character code |
| 3 | `NULL_ITAG` | none — the empty list |
| 4 | `SYMBOL_ITAG` | index into the symbol table |
| 5 | `VOID_ITAG` | none — a deterministic stand-in for an unspecified value |
| 6 | `UNIT_ITAG` | none — zero results reaching a single-value context |
| 7 | `EOF_ITAG` | none |
| 8 | `SHEBANG_ITAG` | directive index |

The gaps in that column are free for new immediate types. Symbols carry an index
into `symt`, a table of interned string data held in `n.c` entirely outside the
Scheme heap; `internsym`, `internsdata`, `symbolname` and `symsdata` are its
interface. Because the payload is twenty-four bits, a symbol index is bounded well
below the heap's capacity, and because the table is C-side, symbols cost the
collector nothing.

Every immediate type goes through `mkimm` and takes a tag from that table.
`obj_from_sztag` is not an immediate constructor, despite producing odd words: it
builds block headers, and what it yields overlaps the immediate space in ways that
depend on its arguments. It is not a way to mint a new singleton.

`void` deserves a word on why it exists as an object at all. Most of the expressions
R7RS leaves unspecified need no value, and on an accumulator machine returning no
particular value is free — the code simply returns with whatever is in `ac`. `void` is
the deterministic alternative, for results a REPL should not echo and for operations
whose natural result would be large or surprising. `stack.md` has the detail.

Finally, `#f` is the word `0` and `#t` is the word `1`, chosen so that C's
conventions carry through: `bool_obj(b)` is `b ? mkimm(0, TRUE_ITAG) : 0`, and
`is_bool(o)` is `!(o & ~(obj)1)`. Every object other than `#f` is true in a
conditional.

### Blocks

A block is a run of cells with a header word in front of it. The header holds the
number of cells and a two-bit *microtag* that says what kind of block it is:

```
   (size << 3) | (mtag << 1) | 1
```

`obj_from_sztag(n, m)` builds one, `size_from_obj` and `mtag_from_obj` read the two
fields back, and `obj_is_blkhdr(o, m)` asks whether a word is a header of kind `m`.
The low bit is 1 for the same reason it always was: it makes the header odd, so
`notaptr` answers true for it and the collector can tell a header from a forwarding
pointer. Everything the collector does is unchanged by the tag — it copies the
header word verbatim and reads only the size.

There are four kinds, and the header alone decides which:

| Microtag | Kind | Cell 0 | Cells 1 on |
|---|---|---|---|
| `TYPED_MTAG` | record, values tuple | the rtd | the fields |
| `PACKED_MTAG` | box, pair | payload | payload |
| `CLOSURE_MTAG` | closure | the code vector | the captured display |
| `VECTOR_MTAG` | vector | element | elements |

Only a `TYPED` block spends a cell saying what it is. In every other kind all the
cells are payload, which is why the kind has to be read from the header — a pair's
car is at cell 0, and it can hold anything at all.

Two tests cover the four kinds, and both are one load and one compare:

```c
#define is_tagged(o, m)   (isobjptr(o) && obj_is_blkhdr(blkhdr(o), m))
#define is_packed(o, n)   (isobjptr(o) && blkhdr(o) == obj_from_packed(n))
```

`is_tagged` asks only the kind, for blocks whose size varies. `is_packed` compares
the whole header, so it asks the kind and the size in a single comparison — which is
what makes a size *be* a type for the packed kinds. Allocation mirrors them:
`hend_tagged(n, m)` and `hend_packed(n)`.

*Packed blocks* are the ones whose size settles their type. A box is one cell and a
pair is two, so `is_box(o)` is `is_packed(o, 1)` and `is_pair(o)` is `is_packed(o, 2)`;
`pair_car` and `pair_cdr` are cells 0 and 1 with nothing in front of them. A pair is
three words including its header, and a box two.

*Vectors* put their elements from cell 0 on, so `vector_len` is the block length and
`vector_ref(v, i)` is `block_ref(v, i)`. A vector of *n* elements is *n*+1 words.

*Closures* keep their code vector in cell 0 and their captured display from cell 1
on, and `is_procedure(o)` is `is_tagged(o, CLOSURE_MTAG)` — the tag and nothing else.
It no longer has to reason about what cell 0 holds, which is what the old test did:
it asked whether cell 0 pointed into the heap, and every other block kind had to
keep something that was *not* a pointer in cell 0 to stay out of its way. Assertions
in `n.c` still check the rest — that cell 0 really is a code vector of at least one
instruction word. An instruction word is a static C pointer outside the heap, so
`isobjptr` rejects it before the tag is read; the printer knows about those
separately and shows one as `#<instruction @…>`.

*Typed blocks* keep a *record type descriptor* in cell 0 and the fields from cell 1
on. `is_typed` is the kind test, `typed_type` reads the rtd back, and `typed_len` and
`typed_ref` address the fields.

The rtd may be **any object except `#f`**. Records are told apart by `eq?` on it and
nothing more is asked of it: it may be a symbol, a pair, a vector, a procedure. The
one reserved value is `#f`, which marks the other inhabitant of this kind — the
*tuple* that carries multiple values through a context expecting one. So
`is_record(o)` is a typed block whose rtd is not `TUPLE_RTD`, and `is_tuple(o)` is one
whose rtd is. `make-record` rejects `#f` and accepts everything else.

That freedom is new. While a closure was recognised by a pointer in cell 0, an rtd
had to be an immediate or a record would have read as a procedure.

`new-record-type` in `pre/s.scm` still makes one symbol per record type, and now does
so by choice rather than by constraint: a symbol prints readably, so a record shows
as `#<record rtd://point:2 1 2>`, and the `:2` says which of several record types of
that name this one is. R7RS requires `define-record-type` records to be generative,
which is what the counter is for — whatever `new-record-type` returns must be fresh
per evaluation.

#### How wide a block can be

Three bits of the header go to the tag and the low marker, leaving the size 29 bits
on a build where an object is 32 bits wide. That is the exact width of a fixnum, and
it is enough: the largest vector a program can index has `FIXNUM_MAX` elements, and
since no cell is spent on a tag, such a vector is `FIXNUM_MAX` cells. On a 64-bit
build the size field is far wider than any heap.

### Natives

A native wraps a pointer to something outside the Scheme heap, with a `cxtype_t*`
in the header slot serving simultaneously as the type and as the deallocator the
collector calls when the object dies. `is_native(o, tp)` is a pointer comparison
against the type's global, and `hpushptr(p, pt, l)` allocates one.

| Type global | Wraps | Present when |
|---|---|---|
| `STRING_NTAG` | string data | always |
| `BYTEVECTOR_NTAG` | bytevector data | always |
| `FLONUM_NTAG` | a `double` | `FLONUMS_BOXED`, i.e. no `NAN_BOXING` |
| `BIGNUM_NTAG`, `FATNUM_NTAG` | bignum / rational-complex | `OPT_TOWER` |
| `IPORT_*_NTAG`, `OPORT_*_NTAG` | port state | always (tty variants under `OPT_ENHTTY`) |

Ports are the one place where the type descriptor carries more than a name and a
deallocator. `cxtype_port_t` extends `cxtype_t` with a direction flag and a
`getch`/`ungetch`/`putch`/`ctl` vtable, and all port types live in one contiguous
array `cxt_port_types[PORTTYPES_MAX]`. That makes `is_iport` and `is_oport` a range
check on the header pointer plus a direction bit, rather than a comparison against
each port type in turn:

```c
pt = (cxtype_t*)objptr_from_obj(o)[-1];
if (pt >= (cxtype_t*)&cxt_port_types[0] &&
    pt <  (cxtype_t*)&cxt_port_types[PORTTYPES_MAX] &&
    (((cxtype_port_t*)pt)->spt & SPT_INPUT)) ...
```

Note that the string data a `STRING_NTAG` points at is not an `obj` array and is
never traced; it is `malloc`ed memory freed by the type's `free` method. The same
goes for bytevectors, bignums and port state. A native's data cell is
*uninterpreted* as far as the collector is concerned.

### Flonums, and why the model matters

This is the one Scheme type whose category changes with the build.

Without `NAN_BOXING`, `FLONUMS_BOXED` is defined and a flonum is a native holding a
`malloc`ed `double`. Constructing one allocates:

```c
#define hflonum_obj(l, f) hpushptr(dupflonum(f), FLONUM_NTAG, l)
```

With `NAN_BOXING`, a flonum is the bitwise complement of its IEEE bit pattern,
stored inline. Constructing one allocates nothing and the test is a mask:

```c
#define is_flonum(o) (((o) & 0xffff000000000000ULL) != 0ULL)
static double get_flonum(obj o) { union iod u; u.i = ~o; return u.d; }
```

Code that builds flonums must therefore reserve heap space in the boxed model and
must not assume it in the other. The macros hide the difference — `hp_flonum_obj(x)`
in `i.c` expands to a reserving `hp_pushptr` or to a plain word construction — but the
*reservation* cannot be hidden, which is why arithmetic instructions reserve before
they compute even though a NaN-boxed build needs nothing.

### The n.h interface

The naming is regular, and the regularity is worth learning because it tells you what
a name does before you look it up.

`is_X(o)` → does this `obj` hold an `X`
<br>`get_X(o)` → extract the C value
<br>`X_obj(v)` → build the `obj`, for the kinds that need no allocation
<br>`hX_obj(l, v)` / `hp_X_obj(v)` → build one that does allocate

The `h` and `hp_` prefixes are the allocation convention and they are not
interchangeable. `h` is for hand-written C — `hflonum_obj(l, f)`, `hstring_obj(l, s)`
— and takes the count of live registers as its first argument, because it may
collect. `hp_` is for instruction bodies — `hp_string_obj(s)` — where the live set is
implied by `sp` and the macro unloads and reloads the shadow registers around a
collection. The two differ in arity, so reaching for the wrong one is a compile
error rather than a silent bug. `hreserve(n, l)` and `hp_reserve(n)` are the same
split one level down.

Three conventions run through the header.

*Everything is a macro under `NDEBUG` and a function otherwise.* Accessors,
predicates and the fixnum operations are all declared twice:

```c
#ifdef NDEBUG
  #define is_typed(o) is_tagged(o, TYPED_MTAG)
#else
  extern int is_typed(obj o);
#endif
```

The debug forms in `n.c` carry the assertions — type tags, index bounds, fixnum
range — and the release forms carry none. A representation change must be made in
both, and the two must agree on every answer: a predicate that is *broader* with
assertions on than in release is a bug, not extra checking. See
[notes.md](notes.md) [7] for what the debug predicates assert and why the quick ones
are sound without it.

*Release predicates are the minimum test that is correct given the conventions.*
`is_procedure` is a heap block whose header carries `CLOSURE_MTAG`, and nothing more;
everything that would make it thorough lives in the debug form. A few of these must
stay macros rather than static functions because they sit in the call path, where the
extra inlining step perturbs register allocation — [notes.md](notes.md) [1].

*The fixnum operations are total.* `fxadd`, `fxmul`, `fxdiv` and the rest are
allowed to return garbage on overflow but are not allowed to fail, except for
genuinely undefined cases such as division by zero. Their `ASSERT`s are documentation
only — `ASSERT` is defined as `(void)0`. Callers that need overflow detection use the
separate `fxaddc`/`fxsubc`/`fxmulc` forms, which return a carry through a pointer,
or check the range themselves as the tower instructions do.

### Adding a type

The shape of the work follows from which category the new type belongs to.

*A new immediate* needs a free tag between 0 and 63, `is`/`mk`/`get` macros in the
pattern of `CHAR_ITAG`, and nothing at all from the collector. Build it with `mkimm`
and nothing else — `obj_from_sztag` looks like it would do and does not, as the tag
table above explains.

*A new kind of block* is the rarest of the three, because there are four microtags
and all four are taken. A type that fits an existing kind needs nothing from the
object layer: anything with a type descriptor is a typed block with its own rtd, and
a fixed-size object can join the packed kind if its size is free — 1 and 2 are the
box and the pair. Every cell of a block is traced automatically whatever the kind.

*A new native* needs a `cxtype_t` with a real `free`, an extern for its global, and
`is`/`get`/`hpush` macros. The data it points at is invisible to the collector, so
it must not contain `obj` values — if it needs to reference Scheme objects, it
should be a block, not a native.

In all three cases `n.h` and `n.c` are themselves the source of truth and are edited
directly. They were generated from `pre/n.sf` in an early version; that has not been
true for several releases, and the generator is gone — `pre/n.sf` is not the place to
make the change.
