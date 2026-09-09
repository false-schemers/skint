## The threaded-code virtual machine

SKINT executes threaded code: an `obj` vector in which each instruction word is the
address of a C function, and each function ends by tail-calling the next one. There
is no dispatch loop and no switch. The design rests on one observation about modern
C compilers — that a group of same-prototype functions tail-calling each other
compiles to unconditional jumps, with the machine's own registers carrying the
state across them.

Everything in this document is in `i.c`. The instruction table it is generated from
is `i.h`, described in [bytecode.md](bytecode.md).

### The code representation

A closure is a heap block whose cell 0 is its code and whose remaining cells are its
captured display. The code is a *vector* whose elements are, in sequence,
instruction words and their operands:

```
    closure:  [ code vector ] [ display 0 ] [ display 1 ] ...
    code:     [ cxi_sref ] [ 2 ] [ cxi_push ] [ cxi_lit ] [ "foo" ] [ cxi_call ] [ 1 ] ...
```

An instruction word is an `obj` that yields an `ins_t` when passed through
`ins_from_obj`. There are two forms of that macro, selected by
`VM_INS_CODE_ALIGNED`:

```c
#ifdef VM_INS_CODE_ALIGNED   /* direct representation */
#define ins_from_obj(x) ((ins_t)(x))
#else                        /* indirect representation (alignment needed) */
#define ins_from_obj(x) (*(ins_t*)(x))
#endif
```

The direct form stores the function's address in the code vector, and requires that
address to be even so it reads as a pointer rather than an immediate. That is what
`nochecks` — `__attribute__((no_stack_protector, aligned(8)))` — is for, and it is
why `VM_INS_CODE_ALIGNED` is defined only in the clang branch. Everywhere else each
instruction gets a static one-element array holding its address, and the code vector
holds the address of that box; `declare_instruction_global` emits whichever form
applies.

Either way, every word in a code vector is an immediate, a heap object, or an
out-of-heap pointer — the three things the collector can classify — so a code vector
is an ordinary traced vector with nothing special about it. Literal operands and
global boxes embedded in the code are traced and relocated like anything else.

### Registers

Six words at the base of the register file are the VM's own state:

```c
#define rk   (r[0])   /* run result / unwind flag */
#define ra   (r[1])   /* accumulator (argument count during a call) */
#define rx   (r[2])   /* next instruction, as an index into the code vector */
#define rd   (r[3])   /* current closure: [0] = code, [1..] = display */
#define rs   (r[4])   /* stack pointer, as an offset from r */
#define rz   (r[5])   /* stack red zone: r + VM_STACK_LEN - VM_STACK_RSZ */
```

Everything above them is the VM stack: `VM_REGC` is 6, `VM_STACK_LEN` is 256000, and
`VM_STACK_RSZ` is 256 words of headroom so that an overflow check can be a single
comparison against `rz` rather than a bounds test on every push. On entry to the VM,
`execute-thunk-closure` grows the register file to `VM_REGC + VM_STACK_LEN` words and
pins `r` at its base; unlike the `#F`-generated code that shares the same file, the
VM never shifts its window.

Note that `rz` holds a raw `obj*` into the register file. The collector sees it as a
Foreign value and leaves it alone — see [memory.md](memory.md#the-five-categories).

#### Working registers and shadow registers

Those six slots are not what the instructions actually use. The working state is
passed *as function parameters* so the C compiler can keep it in machine registers
across the tail calls:

```c
typedef obj* regcall (*ins_t)(IPARAMS);

#ifdef VM_AC_IN_REG
#define IPARAMS  obj ac, obj *noalias r, obj *noalias ip, obj *noalias sp, obj *noalias hp
#else
#define IPARAMS  obj *r, obj *noalias ip, obj *noalias sp, obj *noalias hp
#define ac ra
#endif
```

`r` is the register-file base, `ip` the instruction pointer into the code vector,
`sp` the stack pointer, `hp` the heap allocation pointer, and `ac` the accumulator.
`hp` is threaded through as the return value as well as a parameter, which is why
every instruction returns `obj*`.

The accumulator is the one that varies: where the compiler supports `__regcall`
(clang 13 and later, outside Win64 calling conventions), `VM_AC_IN_REG` is defined
and `ac` is a sixth parameter; otherwise `ac` is simply `ra`, that is `r[1]`.

The collector cannot see a C parameter. So each working register has a *shadow* in
the register file, and there are macro pairs to move values between them:

| Working | Shadow | Unload / reload |
|---|---|---|
| `ac` | `ra` = `r[1]` | `unload_ac()` / `reload_ac()` — no-ops when `ac` *is* `ra` |
| `ip` | `rx` = `r[2]` | `unload_ip()` / `reload_ip()` — converts to and from an index |
| `sp` | `rs` = `r[4]` | `unload_sp()` / `reload_sp()` — converts to and from an offset |

`ip` and `sp` are stored as fixnums rather than pointers because both the code vector
and the register file can move: the code vector is a heap object that a collection
relocates, and the register file is `realloc`ed when it grows.

```c
#define unload_ip() (rx = obj_from_fixnum(ip - &vectorref(vmcloref(rd, 0), 0)))
#define reload_ip() (ip = &vectorref(vmcloref(rd, 0), fixnum_from_obj(rx)))
```

`hp` needs no shadow — the collector is handed it directly and returns the new one.
`r` needs none because the VM keeps it at the file's base.

### Dispatch

```c
#define gonexti()  musttail return ((ins_from_obj(*ip))(IARGS1))
#define goi(name)  musttail return (glue(cxi_, name)(IARGS))
#define goih(ih)   musttail return ((ih)(IARGS))
#define unwindi(c) return (unload_ac(), unload_ip(), rd = (c), hp)
```

`gonexti` is the ordinary end of an instruction: fetch the next word, call it with
`ip` advanced past it. `goi` transfers to a named instruction without advancing `ip`
— `gsqrt` is literally `{ goi(isqrt); }` — and `goih` transfers to a helper.
`unwindi` leaves the VM altogether, returning `hp` with `rd` set to the reason.

`musttail` is `__attribute__((musttail))` under clang and empty elsewhere, so
`gonexti` is either a guaranteed jump or an ordinary `return f(...)` that the
compiler will usually, but not certainly, turn into one.

#### The trampoline

Because "usually" is not "certainly", calls and returns go through a level of
indirection that resets the C stack. `VM_MUSTTAIL_GUARANTEE` decides which:

```c
#ifdef VM_MUSTTAIL_GUARANTEE
  #define callsubi()  check_sp(); reload_ip(); gonexti()
  #define retfromi()  reload_ip(); gonexti()
  #define trampcnd()  (0)
#else
  #define callsubi()  check_sp(); unload_sp(); return hp
  #define retfromi()  unload_sp(); return hp
  #define trampcnd()  (rd)
#endif
```

and the entry point in `vmhost` runs both cases with the same loop:

```c
do {
  reload_ac(); reload_ip(); reload_sp();
  hp = (ins_from_obj(*ip))(IARGS1);
} while (likely(trampcnd()));
```

Where tail calls are guaranteed, `trampcnd()` is 0 and the loop body runs exactly
once — the entire program executes as one chain of jumps, and the loop is left only
when an instruction calls `unwindi(0)`. Where they are not, every Scheme-level call
and return returns to the loop, which reloads the working registers from their
shadows and re-enters. `unwindi(0)` sets `rd` to 0 and so terminates the loop in
both configurations, which is why instructions never need to know which one they are
compiled under.

The trampoline bounds the C stack by the length of the longest straight-line run of
instructions between one call or return and the next — not by the Scheme program's
recursion depth. That is what makes the fallback merely slower rather than
unusable.

#### Calls and returns

Scheme's control state lives on the VM stack, not the C stack. `save` pushes the
current closure and the index to resume at:

```c
define_instruction(save) {
  int dx = get_fixnum(*ip++);
  spush(rd);
  spush(fixnum_obj(ip + dx - &vector_ref(vmcloref(rd, 0), 0)));
  gonexti();
}
```

Its operand is the length of the code block that follows, so `ip + dx` is the
instruction after it — the return point. `call` then installs the callee as `rd`,
sets `rx` to 0, puts the argument count in `ac`, and hands control over with
`callsubi()`. `return` pops the two words back into `rx` and `rd` and uses
`retfromi()`. Arguments travel with the first in `ac` and the rest on the stack.

A continuation is a closure over the saved stack: `lck` copies the live stack into a
fresh closure whose code is `continuation-adapter-code` and whose cell 1 records the
dynamic state at capture. `wck` and `wckr` reinstate it, either by copying the saved
stack straight back when the dynamic state still matches, or by routing through the
adapter closure so that the `dynamic-wind` chain can be unwound and rewound first.

#### One result per instruction, and the box convention

An instruction leaves exactly one value, in the accumulator. Returning several is
possible — a tuple built by `hend_tuple`, with `sdmv` (`values`), `cwmv`
(`call-with-values`) and the continuation adapter between them arranging for the
consumer to receive it — but that path is involved and comparatively slow, and none
of it belongs on the fast path of a builtin.

So a builtin with a second result to deliver does not return two values. It takes an
extra argument, a box, and stores the second result there: `%fxsqrt` puts the
remainder in its box, `flmodf` the integral part, `flfrexp` the exponent,
`%flremquo` the quotient bits, and `fxaddc`, `fxsubc` and `fxmulc` the low half of
the split. This is a C out-parameter carried across the boundary unchanged, and it
keeps each of these to a single instruction.

The conventional Scheme interface is then a thin wrapper in whichever library wants
it, written with the `set&` special form — which makes a box aliasing a variable —
and `values`:

```scheme
(define (fxsqrt x)
  (let* ([rem 0] [srt (%fxsqrt x (set& rem))])
    (values srt rem)))
```

`(srfi 143)` and `(srfi 144)` are where those wrappers live: `fxsqrt`,
`fx+/carry`, `fx-/carry`, `fx*/carry`, `flremquo`, `flinteger-fraction` and
`flnormalized-fraction-exponent` all have this shape. The cost of multiple values is
paid once, in Scheme, by the callers who want that interface — never by the
instruction.

The `%` on two of those names is not a mark of the convention itself. It is there
because the plain name was already spoken for by the wrapper: `(srfi 143)` defines
`fxsqrt` and `(srfi 144)` defines `flremquo`, so the primitives underneath had to be
called something else. `flmodf`, `flfrexp` and the carry instructions keep plain
names because their standard counterparts are called `flinteger-fraction`,
`flnormalized-fraction-exponent` and `fx+/carry` — no collision to avoid.

### Allocating inside an instruction

The reserve-then-build discipline is the same as in `#F`-generated code, but the
macro is different, and the difference is the point of this section.

```c
#define hp_reserve(n) do { \
 if (unlikely(hp < cxg_heap + (n))) { \
   unload_ac(); unload_ip();   \
   hp = cxm_hgc(r, sp, hp, n); \
   reload_ac(); reload_ip();   \
 } } while (0)
```

Compare `n.h`'s version, which `#F`-generated code uses:

```c
#define hreserve(n, l) ((hp < cxg_heap + (n)) ? hp = cxm_hgc(r, r+(l), hp, n) : hp)
```

Three differences, all consequences of how the two kinds of code use the register
file:

- *The live range.* `#F` code passes `r` and `r + l`, where `l` is a live-register
  count the compiler computes per site. The VM passes `r` and `sp` — everything from
  the base of the file to the top of the stack is live, and no count is needed.
- *Unload and reload.* `#F` code keeps everything in the register file already, so
  there is nothing to spill. The VM must write `ac` and `ip` to their shadows before
  collecting and read them back afterwards, because a collection moves the code
  vector out from under `ip`.
- *`sp` and `r` are left alone.* `hp_reserve` does not touch them, because a
  collection does not move the register file; only `cxm_rgc` does, and the VM never
  calls it after entry.

`hp_reserve_inline` is the expression form, used where a reserve has to happen inside
a larger expression, and `hp_pushptr(p, pt)` is the reserve-and-build for a native.

Two rules follow for instruction writers. Everything live must be in the register
file or on the stack across a reserve — an `obj` held only in a C local will dangle.
And the `flonum_obj` and `bignum_obj` constructors allocate in the boxed model, so
an instruction that produces one must reserve for it even though a `NAN_BOXING`
build needs nothing; see [objects.md](objects.md#flonums-and-why-the-model-matters).

### Type checks and failure

Every operand check is one macro that either falls through or leaves the
instruction:

```c
#define cki(x) do { obj _x = (x); if (unlikely(!is_fixnum(_x))) \
  { ac = _x; spush((obj)"fixnum"); musttail return cxi_failactype(IARGS); } } while (0)
```

There is one of these per type — `ckp` pair, `ckl` list, `ckv` vector, `ckc` char,
`cks` string, `ckb` bytevector, `cki` fixnum, `ckj` flonum, `ckn` number, `ckk`
non-negative fixnum, `ck8` byte, `cky` symbol, `ckr` input port, `ckw` output port,
`ckx` procedure, `ckz` box, `cko` record, `ckg` integrable, `cku` end of a proper
list, `cksb` directive — plus `fail(msg)` for a message with no offending value and
`failtype(x, msg)` for an explicit one.

Note what the failing path does with the message: it pushes a C string literal onto
the VM stack, which is a traced region. This is safe because the collector's Foreign
category covers exactly that — an out-of-heap pointer in a traced slot is left
untouched. The same trick carries a C function pointer in `ac` into the tower
helpers.

These report and unwind; they do not raise a Scheme condition. That is why `guard`
cannot catch a builtin's complaint.

### The leaf and helper split

A C compiler treats a function that makes a real call as needing a full prologue and
epilogue — saving callee-saved registers, adjusting the frame — even if the call
happens on a path that is almost never taken. For an instruction that is meant to be
a handful of machine instructions, that is the entire cost.

So instructions with a hot path that calls nothing put the cold path in a separate
function, and the fast path becomes a de-facto leaf. The helper is declared with

```c
#define define_instrhelper(name) \
  static obj* regcall outofline nochecks name(IPARAMS)
```

where `outofline` is `__attribute__((noinline))`. It has the same prototype as an
instruction, so reaching it is a tail call like any other, and it can `gonexti()`
when it is done as though nothing unusual had happened.

The tower's arithmetic is the clearest case. `add` handles fixnums and flonums
inline and hands everything else off:

```c
define_instruction(add) {
  obj x = ac, y = spop();
  if (likely(are_fixnums(x, y))) {
    long lx = get_fixnum(x), ly = get_fixnum(y);
    int64_t llz = (int64_t)lx + (int64_t)ly;
    if (likely(llz >= FIXNUM_MIN && llz <= FIXNUM_MAX)) ac = fixnum_obj((long)llz);
    else { spush(x); spush(y); ac = (obj)&fnadd; goih(tower_binary); }
  } else {
    ...
  }
  gonexti();
}
```

Two things are worth copying from this. The operands are *pushed* before the
handoff, so they are traced across the allocation the helper will do. And the
generic operation is passed as a C function pointer in `ac`, so one helper —
`tower_binary` — serves every binary arithmetic instruction rather than each one
carrying its own copy of the widening logic. `tower_unary`, `tower_binary_push`,
`tower_cmp`, `tower_eq` and `tower_isqrt` are the full set.

`cxi_fail` and `cxi_failactype` are helpers for the same reason: they call
`fprintf`, and inlining that into every type check would give every instruction a
frame.

### Writing an instruction

The prototype is fixed. `define_instruction(name)` opens it, and it must leave
through `gonexti`, `goi`, `goih`, `callsubi`, `retfromi`, `unwindi`, or one of the
failure macros — never by falling off the end.

The hazard worth stating plainly: *the register names are not reserved, and the C
compiler cannot enforce that they are.* Three groups behave differently when
shadowed.

- `r`, `ip`, `sp` and `hp` are function parameters. A local variable with one of
  those names compiles cleanly and silently breaks the instruction.
- `ac` is a parameter under `VM_AC_IN_REG` and the macro `ra` otherwise. A local
  named `ac` therefore shadows silently under clang and fails to compile under GCC
  and MSVC — the worst of both, since the mistake will not show up in the build you
  are testing.
- `rk`, `ra`, `rx`, `rd`, `rs` and `rz` are macros expanding to `r[N]`. Declaring a
  local with one of those names is a syntax error, which is at least immediate.

The same applies to the accessor macros `sref`, `dref` and `gref`, the stack
operations `spush`, `spop`, `sdrop` and `sgrow`, and the whole `ck*` family. By
convention instructions use short, unambiguous locals — `x`, `y`, `n`, `i`, `c`,
`p`, `l` — and nothing that could collide.

Two more rules that are easy to get wrong:

*`ip` must be advanced exactly once per operand.* `*ip++` reads an operand; the
number of operands must match the `etyp` column in `i.h` for that instruction, since
that is what the decoder used when it laid the code vector out.

*Do not hold a raw pointer across a reserve.* Any `obj*` obtained from a heap object
before `hp_reserve` points into from-space afterwards. Re-derive it, or keep the
`obj` in a traced slot and index from it again.
