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
| 1 | `VOID_ITAG` | none — the unspecified value |
| 2 | `CHAR_ITAG` | character code |
| 3 | `NULL_ITAG` | none — the empty list |
| 4 | `SYMBOL_ITAG` | index into the symbol table |
| 7 | `EOF_ITAG` | none |
| 8 | `SHEBANG_ITAG` | directive index |

The gaps in that column are free for new immediate types. Symbols carry an index
into `symt`, a table of interned string data held in `n.c` entirely outside the
Scheme heap; `internsym`, `internsdata`, `symbolname` and `symsdata` are its
interface. Because the payload is twenty-four bits, a symbol index is bounded well
below the heap's capacity, and because the table is C-side, symbols cost the
collector nothing.

Three further immediates are built with `obj_from_size` rather than `mkimm`, and are
compared for identity rather than decoded:

```c
#define obj_from_ktrap() (obj_from_size(0x5D56F806))
#define obj_from_unit()  (obj_from_size(0x6DF6F577))
```

`unit` is the value delivered when zero results reach a context expecting one — a
zero-element tuple. `ktrap` marks a continuation slot that must not be used.

Finally, `#f` is the word `0` and `#t` is the word `1`, chosen so that C's
conventions carry through: `obj_from_bool(b)` is `b ? mkimm(0, TRUE_ITAG) : 0`, and
`is_bool_obj(o)` is `!(o & ~(obj)1)`. Every object other than `#f` is true in a
conditional.

### Blocks

A block is a run of cells with an immediate length in the header word. Three
different kinds of Scheme object are blocks, and they are told apart by what sits in
*cell 0* — never by anything the collector can see.

*Tagged blocks* put a small `obj_from_size(t)` in cell 0 and the payload from cell 1
on. `istagged(o, t)` checks `hblkref(o, 0) == obj_from_size(t)`, `taggedlen` is the
block length minus one, and `taggedref(o, t, i)` is `&hblkref(o, i+1)`. Three tags
are assigned:

| Tag | Type | Cells after the tag |
|---|---|---|
| 1 | `VECTOR_BTAG` | the elements |
| 2 | `BOX_BTAG` | one — boxes back `set!` variables, global cells and promises |
| 3 | `PAIR_BTAG` | two — car and cdr |

Tag 0 is used for *tuples*, the object that carries multiple values through a
context that expects one. It is defined in `i.c` rather than `n.h`, since only the
VM builds and consumes them.

*Typed blocks* put a symbol in cell 0 and the fields from cell 1 on; these are
records. `istyped` recognises them, `typedtype` reads cell 0 back and `typedlen` and
`typedref` address the fields. The type descriptor being a symbol is not an
accident of convenience — the comment on `new-record-type` in `pre/s.scm` spells out
the reason:

> should be something like `(cons name fields)`, but that would complicate
> `procedure?` check that now relies on block tag being a non-immediate object

A record type descriptor must be an *immediate* so that a record can never be
mistaken for a closure, and it must be unique per `define-record-type` evaluation
because R7RS requires those records to be generative. Symbols satisfy both, so
`new-record-type` interns one:

```scheme
(string->symbol (string-append "rtd://" (symbol->string name)
                               ":" (number->string *rtd-count*)))
```

which is why a record prints as `#<record rtd://point:2 1 2>`.

*Closures* put a pointer in cell 0 — the code — and the captured display from cell 1
on. `isprocedure` is therefore the complement of the other two: a heap block whose
cell 0 satisfies `isaptr`. It also accepts any non-null pointer that is not a heap
address at all, which came from the `#F` compiler: it allocated environment-free
global procedures in static C memory, as one-word blocks holding a code pointer.
`k.c` no longer contains any, but instruction words are static C pointers of the
same shape — so the allowance still has teeth, and `procedure?` can answer `#t` for
one. Those are Foreign values to the collector, and `procedurelen` treats them as
one-element closures. Use `closure?` when the answer has to mean a real closure;
see [builtins.md](builtins.md).

So the discrimination among block kinds is entirely a matter of cell 0 holding a
small size immediate, a symbol immediate, or a pointer. The invariant that makes it
sound is that cell 0 is never user data — a pair's car is at cell 1, not cell 0 —
so no Scheme value can ever be mistaken for a tag.

### Natives

A native wraps a pointer to something outside the Scheme heap, with a `cxtype_t*`
in the header slot serving simultaneously as the type and as the deallocator the
collector calls when the object dies. `isnative(o, tp)` is a pointer comparison
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
array `cxt_port_types[PORTTYPES_MAX]`. That makes `isiport` and `isoport` a range
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
#define obj_from_flonum(l, f) hpushptr(dupflonum(f), FLONUM_NTAG, l)
```

With `NAN_BOXING`, a flonum is the bitwise complement of its IEEE bit pattern,
stored inline. Constructing one allocates nothing and the test is a mask:

```c
#define is_flonum_obj(o) (((o) & 0xffff000000000000ULL) != 0ULL)
static double flonum_from_obj(obj o) { union iod u; u.i = ~o; return u.d; }
```

Code that builds flonums must therefore reserve heap space in the boxed model and
must not assume it in the other. The macros hide the difference — `flonum_obj(x)` in
`i.c` expands to a reserving `hp_pushptr` or to a plain word construction — but the
*reservation* cannot be hidden, which is why arithmetic instructions reserve before
they compute even though a NaN-boxed build needs nothing.

### The n.h interface

Above the representation sits a naming discipline that the `#F` compiler's generated
code relies on. For each Scheme type `X` with C representation `X_t`:

`is_X_obj(o)` → does this `obj` hold an `X`
<br>`X_from_obj(o)` → extract the C value
<br>`obj_from_X(v)` → build the `obj` (heap-allocating types take a live-register
count as the first argument)
<br>`is_X_Y(v)` / `Y_from_X(v)` → the cross-type conversions and constant-folded
predicates the compiler emits, most of them trivially `0`, `1`, or the identity

The trivial-looking ones exist so that generated code can name any conversion
without the generator having to know which are possible: `is_bool_fixnum(i)` expands
to `((void)(i), 0)` and disappears.

Two further conventions run through the header.

*Everything is a macro under `NDEBUG` and a function otherwise.* Accessors,
predicates and the fixnum operations are all declared twice:

```c
#ifdef NDEBUG
  #define taggedref(o, t, i) (&hblkref(o, (i)+1))
#else
  extern obj* taggedref(obj o, int t, int i);
#endif
```

The debug forms in `n.c` carry the assertions — type tags, index bounds, fixnum
range — and the release forms carry none. A representation change must be made in
both.

*The fixnum operations are total.* `fxadd`, `fxmul`, `fxdiv` and the rest are
allowed to return garbage on overflow but are not allowed to fail, except for
genuinely undefined cases such as division by zero. Their `ASSERT`s are documentation
only — `ASSERT` is defined as `(void)0`. Callers that need overflow detection use the
separate `fxaddc`/`fxsubc`/`fxmulc` forms, which return a carry through a pointer,
or check the range themselves as the tower instructions do.

### Adding a type

The shape of the work follows from which category the new type belongs to.

*A new immediate* needs a free tag between 0 and 63, `is`/`mk`/`get` macros in the
pattern of `CHAR_ITAG`, and nothing at all from the collector.

*A new tagged block* needs a free small tag and the `istagged`/`taggedref` wrappers.
Cell 0 must hold `obj_from_size(t)`, and every element from cell 1 on is traced
automatically.

*A new native* needs a `cxtype_t` with a real `free`, an extern for its global, and
`is`/`get`/`hpush` macros. The data it points at is invisible to the collector, so
it must not contain `obj` values — if it needs to reference Scheme objects, it
should be a block, not a native.

In all three cases `n.h` and `n.c` are themselves the source of truth and are edited
directly. They were generated from `pre/n.sf` in an early version; that has not been
true for several releases, and the generator is gone — `pre/n.sf` is not the place to
make the change.
