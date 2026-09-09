## The memory model, as the collector sees it

This is the bottom layer of SKINT. It describes what an `obj` is, how the heap is
laid out, and what the garbage collector does — nothing above that. The collector's
view is deliberately coarse: it knows five categories of object and nothing at all
about pairs, strings, closures, or the virtual machine. The next layer up, which
turns those five categories into Scheme types, is described in
[objects.md](objects.md).

The code discussed here lives in `n.h` (declarations and macros, hand-maintained)
and in the "basic runtime" section at the end of `k.c`, which holds the collector
itself.

That second half is worth a word of warning. It is not SKINT's code: it is the
standard runtime the `#F` compiler emits for any program it compiles, and it arrives
in `k.c` because `k.sf` is compiled by `sfc`. `pre/ksf2c.ssc` then applies a short
list of line-level patches to it — dropping `static` from `cxg_hsize`,
`cxg_gccount` and `cxg_bumpcount` so that `i.c` can expose them to Scheme, and
replacing the `(char*)` casts in the pointer tests with `(cxoint_t)` casts so the
arithmetic is correct when `obj` is an `int64_t` under `NAN_BOXING`. Changing the
collector means changing it in `sfc` and regenerating, or adding to that patch list.

### Two memory models

An `obj` is one machine word. Which word, and how a flonum fits into it, depends on
how the interpreter was built.

| Build | `obj` | Pointer test | Flonums |
|---|---|---|---|
| default | `ptrdiff_t` (32 or 64 bit) | low bit clear | boxed on the heap (`FLONUMS_BOXED`) |
| `-DNAN_BOXING` | `int64_t` | low bit clear *and* top 16 bits clear | NaN-boxed inside the word |

Under `NAN_BOXING` the interpreter assumes a 48-bit address space, so the top 16
bits of a real pointer are always zero and are free to be used as a tag. A flonum
is stored as the bitwise complement of its IEEE double, which puts a non-zero
pattern in those top 16 bits; that is the whole of `is_flonum_obj`:

```c
#define is_flonum_obj(o) (((o) & 0xffff000000000000ULL) != 0ULL)
```

Everything in this document holds for both models. Where they differ, the
difference is confined to the two macros `notaptr` and `notobjptr`, and the
collector calls those rather than testing bits itself.

### The five categories

Given an arbitrary word, the collector classifies it by asking at most two
questions. In `n.h` these are `notaptr` (is the word a tagged immediate rather
than an address?) and `notobjptr` (does the word address a cell in the current
from-space?).

*False* is the single word `(obj)0`. It is Scheme's `#f`, and it is deliberately
the null pointer so that C's truth conventions carry over unchanged: `#t` is the
word `1`, and `is_bool_obj(o)` is `!(o & ~(obj)1)`. Being neither a tagged
immediate nor a usable address, `#f` needs no work from the collector; it matters
here only because the collector relies on no valid block header ever being zero,
which is asserted in both `toheap2` and `finalize`.

*Immediate* is any word that fails `isaptr` — in the default model, any odd word.
The payload travels in the word itself; there is nothing to trace and nothing to
copy. Fixnums, characters, symbols, `#t`, the empty list, the end-of-file object
and the block-size headers described below are all immediates.

*Block* is a pointer to a run of consecutive cells in the Scheme heap. The word
immediately below the pointed-to cell is a header holding the length as an
immediate, so a block of `n` cells occupies `n + 1` words:

```
    [ obj_from_size(n) ] [ cell 0 ] [ cell 1 ] ... [ cell n-1 ]
                          ^
                          the obj points here
```

`hbsz(n)` is the total word count `n + 1`, `hblklen(p)` reads the length back, and
`hblkref(p, i)` is cell `i`. The collector copies all `n + 1` words and traces every
cell; it never asks what the cells mean.

*Native* is a two-cell wrapper around something that does not live in the Scheme
heap and may need releasing. The header slot holds a pointer to a `cxtype_t`
descriptor instead of a size, and the single cell below it holds uninterpreted
data — in practice a pointer into the C heap:

```c
typedef struct {
  const char *tname;    /* name, for debugging */
  void (*free)(void*);  /* deallocator */
} cxtype_t;
```

```
    [ cxtype_t* ] [ data ]
                   ^
                   the obj points here
```

`hpushptr(p, pt, l)` builds one, `isnative(o, tp)` recognises one belonging to a
given type, and `getnative` reads the data cell out. Strings, bytevectors, ports,
bignums and — in a build without `NAN_BOXING` — flonums are all natives. This is
the only category the collector finalizes.

*Foreign* is a non-null pointer that does not address the current from-space. The
collector leaves such a word exactly as it found it. This category is what makes it
safe for ordinary C values to sit in traced slots, and the interpreter uses that
freedom liberally: `cxtype_t` pointers in native headers, the addresses of
statically allocated closures emitted by the `#F` compiler, the red-zone pointer the
VM keeps in `r[5]`, the C string literals the type-check macros push on the VM stack
before reporting a failure, and the C function pointer the tower instructions stash
in the accumulator on their way to a helper.

Note what is *not* in the list: there is no category for "block of a particular
kind". Vectors, pairs, records and closures are all just blocks. Their distinctions
are made one layer up, by convention about what goes in cell 0, and the collector
is unaware of them.

### The heap

Two semispaces, each `cxg_hsize` words. `cxg_heap` is the base of the current
from-space and `cxg_hp` the allocation pointer. Allocation runs *downward*: the
free region is `[cxg_heap, hp)` and live data occupies `[hp, cxg_heap + hsize)`.
Every construction macro therefore writes cells with `*--hp` in reverse order and
finishes with `hendblk`, which lays down the header and returns the resulting
`obj`:

```c
#define hendblk(n) (*--hp = obj_from_size(n), (obj)(hp+1))
```

The initial size is `HEAP_SIZE` (131072 words) per semispace, and it only ever
doubles.

#### The heap mask

`notobjptr` has to answer two questions at once — is this word an address rather
than an immediate, and does it point into the current from-space? — and it answers
both with a single mask:

```c
#define notobjptr(o) (((char*)(o) - (char*)cxg_heap) & cxg_hmask)
```

where the mask is built as

```c
m = 1 | ~((cxoint_t)s * sizeof(obj) - 1);
```

Because the byte size of a semispace is always a power of two, `~(bytes - 1)`
clears exactly when the offset from the base lies in `[0, bytes)`, and the extra
`| 1` adds the requirement that the word be even. One `and` against one cached
constant does the whole classification. Note that only the *size* must be a power
of two; the base address is whatever `realloc` returned.

### Roots

There are exactly two root sources, and `relocate` in `k.c` scans them in this
order.

*The register file* is a flat `obj` array addressed by `cxg_regs`, with `cxg_rend`
marking its end. It is `malloc`ed memory, entirely separate from the Scheme heap and
grown by `realloc` on demand. The collector receives two pointers into it, `regs`
and `regp`, and scans the half-open range between them. What that range means
differs between the interpreter's two kinds of code, and the collector does not care
which it is looking at:

- Code generated by the `#F` compiler (`k.c`, and the runtime pieces of `n.c`)
  treats the file as a sliding window of virtual registers. `r` is the current
  window base and shifts upward as continuations are entered; `hreserve(n, l)`
  passes `r` and `r + l`, where `l` is the number of registers live at that point.
  Everything below the current window is unreachable — the code is
  continuation-passing, so windows are never returned to — and everything above
  `r + l` is uninitialised.
- The threaded-code VM (`i.c`) keeps `r` pinned at the base of the file, uses
  `r[0]`–`r[5]` as its shadow registers, and uses everything above them as its
  stack. Its `hp_reserve(n)` passes `r` and the current stack pointer.

`cxm_rgc(regs, needs)` grows the file when it runs short and, if `regs` is above the
base, slides `needs` words back down to it. That is what keeps a long chain of
window shifts from exhausting the file.

*A chain of static global tables.* A module that owns `obj` globals declares an
array of their addresses and links a `cxroot_t` record into `cxg_rootp`:

```c
typedef struct cxroot_tag {
  int globc; obj **globv;
  struct cxroot_tag *next;
} cxroot_t;
```

At present exactly one module does: `k.c` registers its globals, among them
`*globals*` (the vector in which every Scheme global's box is interned),
`*transformers*`, `*dynamic-state*` and the three current-port cells. Everything
else in the Scheme world is reachable from those. `s.c` and `t.c` contain no `obj`
globals at all — they are tables of serialized strings — so they register nothing.

### The collector

A Cheney-style copying collector over the two semispaces, in about forty lines at
the end of `k.c`. Three functions do the work.

#### toheap2

`toheap2(p, hp, h1, m1, h2, m2)` is the whole classification and copy, applied to
one slot `p`. It reads `o = *p` and:

1. If `o` is not a pointer into from-space — immediate, false, or foreign — it
   returns with the slot untouched. This one test covers three of the five
   categories.
2. Otherwise it reads the header word `fo` at `op[-1]`.
3. If `fo` is an immediate, this is a *block*. The header and all `size_from_obj(fo)`
   cells are copied down into to-space, the slot is updated, and the old header slot
   is overwritten with the new address.
4. If `fo` is a pointer that does not address to-space, this is a *native*. Its two
   words are copied, the slot is updated, and again the old header slot is
   overwritten with the new address.
5. Otherwise `fo` is a pointer into to-space, which can only mean it is a
   forwarding address left by an earlier visit; the slot is set to it.

Overwriting the header slot with a to-space address is the forwarding mark, and
case 5 is how it is recognised. It works precisely because a header can otherwise
only be an immediate (block) or an out-of-heap pointer (native) — the two cases
tested first.

#### relocate

```c
for (p = regs; p < regp; ++p)   hp = toheap2(p, ...);   /* register file */
for (; pr; pr = pr->next)       /* each module's globals */
for (p = he2; p > hp; --p)      hp = toheap2(p-1, ...); /* Cheney scan */
```

The third loop is the transitive closure. To-space also fills downward from its top
`he2`, so scanning downward from `he2` walks copied objects in the order they were
copied; `hp` keeps descending as the scan discovers more, and the loop ends when the
scan pointer meets it. No explicit worklist and no scan pointer variable are needed.

The scan visits *every word* of to-space, including block headers and native type
pointers, and this is safe for exactly the reason given above: a header is either an
immediate or an out-of-heap pointer, and `toheap2` skips both. The collector needs
no map of object layouts.

#### finalize

Unreachable natives must have their `free` method called, and a copying collector
never visits unreachable objects — so finalization is a separate linear walk over
from-space, from the old allocation pointer up to the end. At each object the header
says what to do:

- an immediate: a block, skip that many cells;
- a pointer outside to-space: a native that was *not* forwarded, so it is garbage —
  call `((cxtype_t*)fo)->free(data)` and skip its one data cell;
- a pointer into to-space: a forwarding address, so the object survived; follow it
  to the copy and read *its* header to learn how many cells to skip.

This walk is skipped (`he1` is passed as 0) on the extra copy that a heap growth
performs, because at that point the objects have already been accounted for.

#### cxm_hgc

The entry point, called from `hreserve` and `hp_reserve` when the free region is too
small. It collects, and then decides whether the result leaves enough room:

```c
if (h1) hp = relocate(...), needs += (h2 + hs - hp)*2;  /* make heap half empty */
if (hs < needs) { ... double until it fits, copy again ... }
h1 = h2; h2 = h;
```

The `*2` is the growth policy: the requested size is inflated to twice the surviving
data, so a collection that leaves the heap more than half full triggers a doubling
rather than a series of collections that each recover almost nothing. Growth
reallocates both semispaces to the new power-of-two size and performs a second copy
into the larger space. `cxg_gccount` counts collections and `cxg_bumpcount` counts
doublings; both are readable from Scheme, along with the current semispace size in
words:

```
(%gc)          ; collect now, return the new collection count
(%gc-count)    ; collections so far
(%bump-count)  ; heap doublings so far
(%heap-size)   ; semispace size in words
```

### Allocating safely

The contract every allocation site must honour follows from the fact that a
collection moves objects and updates only what it can see.

*Reserve before you build.* `hreserve(n, l)` (in `#F`-generated code) and
`hp_reserve(n)` (in VM instructions) may collect, so all the space a composite
object needs must be reserved in one call before any of it is written. Sites that
build several objects at once ask for the sum:

```c
hreserve(boxbsz()*1 + pairbsz()*2, sp-r);
```

*Everything live must be in a scanned slot at that moment.* An `obj` held only in a
C local is invisible to the collector and will dangle. In `#F`-generated code the
live registers are `r[0]` through `r[l-1]`; in the VM they are `r[0]`–`r[5]` and the
stack below `sp`. This is why VM instructions push their operands before taking a
slow path that can allocate, and why the accumulator and instruction pointer are
written back to their shadow registers around every reserve — see
[vm.md](vm.md#working-registers-and-shadow-registers).

*A reserve invalidates raw addresses.* Any `obj*` derived from a heap object before
the call points into from-space afterwards. Re-derive it.

*Foreign values are free.* A C pointer, a string literal, a function address — any
of these can sit in a traced slot across a collection with no ceremony. That is the
point of the Foreign category, and it is used deliberately rather than tolerated.
