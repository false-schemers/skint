## The stack, continuations and multiple values

skint is an accumulator machine, so a good deal of what other interpreters push and
pop never leaves a register here. What does reach memory reaches the VM stack, where
Scheme's control state lives rather than on the C stack. This page covers the
convention that keeps the stack quiet, the stack itself, and the two things built
directly on top of it: continuations, which are copies of the stack, and multiple
values, which travel on it.

`vm.md` covers the registers and the dispatch loop; this page assumes them.

### The accumulator, and why the stack sees so little traffic

skint is an accumulator machine, and that is the first thing to know about its stack:
most work never reaches it.

One register, `ac`, carries the single implicit operand and the result of every
instruction. So the *instruction* calling convention — which is not the procedure
calling convention described under Frames below — is:

*the leftmost operand arrives in `ac`, any remaining operands are on the stack, and
the result is left in `ac`.*

Operands are evaluated right to left. Each is computed in `ac` and pushed, so by the
time the instruction runs the leftmost one is still sitting in `ac` and never went to
memory at all. `(vector-ref v i)` compiles to

    %2 .1 , .1 V4 ]2

— load the second parameter, push it, load the first, then `vector-ref`, which takes
its vector from `ac` and pops the index. Note that the two `.1`s name different
slots: operand offsets are relative to the current top, so a push shifts them, and
the compiler tracks the depth as it goes.

The payoff is that a one-operand instruction needs no stack traffic whatsoever, and
such instructions chain directly, each consuming and producing `ac`:

    (cdr (car x))                   %1 .0 ad ]1
    (car (car (cdr (cdr x))))       %1 .0 dd aa ]1
    (cadddr x)                      %1 .0 dd da ]1

Load the argument once, then run the chain. Nothing is pushed, nothing is popped, and
no intermediate is written anywhere.

Two separate things are on show there. The chain is what the accumulator convention
buys. The *pairing* is a second effect on top of it: `aa`, `ad`, `da` and `dd` are
single fused instructions in their own right, and the decoder's longest-match trie
picks them out of the chain without anyone asking — see `bytecode.md`. So `cadddr`,
which is an integrable whose encoding is literally the four characters `ddda`,
decodes to two instructions rather than four, and its entire body is five words with
the arity check and the return included.

This is also why `ac` is worth a real machine register. On toolchains that can
guarantee it, `ac` is a genuine C parameter and the compiler keeps it in a register
across the tail-call chain; elsewhere it aliases `ra`, a slot in the register file,
and the same code runs a little slower. `vm.md` describes the conditional under
`VM_AC_IN_REG`. Either way the instruction set is written as though `ac` were free,
which is what makes chains like the above the normal case rather than an
optimisation.

The stack, then, is for what will not fit in one register: the second and later
operands of an instruction, arguments to a real call, saved return frames, and
anything a computation must keep alive across a call.

#### Unspecified values, and why void exists

Scheme specifies a great many expressions as returning an unspecified value, which
means any value at all. On an accumulator machine that is free: there is always
something in `ac`, so a form that need not return anything in particular can return
by doing nothing whatsoever.

The compiler takes that literally. An empty `begin` emits no code at all, and the
missing alternative of a two-armed `if` is exactly an empty `begin`:

    (lambda () (begin))          %0 ]0
    (lambda (x) (if x 1))        %1 .0 ?{ '1 ]1 } ]1

In the second, the then-arm returns `1`; the else path falls straight through to the
return with `ac` untouched, still holding the value the test left there. That is why
`(if #f #f)` answers `#f` — not because a missing arm means false, but because the
test's own value is what happens to be in the accumulator. Writing the arm out as
`(if x 1 (void))` costs one instruction, `Y9`.

Free is not the same as useful, though, and the REPL is where it stops being useful.
The REPL cannot know whether the form someone typed meant to return a value, so an
arbitrary accumulator leftover echoed back at a prompt is not "unspecified", it is
noise — and a wrong answer to the question the user asked.

Hence `void`: a single deterministic object which is still unspecified as far as the
user is concerned, and which the REPL declines to print.
`repl-evaluate-top-form` in `pre/t.scm` suppresses its output for exactly one value
that satisfies `void?`, and prints anything else.

So the convention, which is a convenience for interactive work and not a contract, is
to return `(void)` from an operation whose natural result would be large, ugly or
surprising, and to leave the accumulator alone where the result does not matter and
nobody will see it. `(if #f #f)` is the second kind; it could just as well have been
the first.

### Where the stack lives

There is no separate allocation. The register file is one `obj` array of
`VM_REGC + VM_STACK_LEN` words: the first `VM_REGC` are the registers, and everything
above them is the stack.

```
r[0] .. r[VM_REGC-1]     rk ra rx rd rs rz
r[VM_REGC] ..            the stack, growing upwards
```

`k.c`'s `main` allocates it once, through `cxm_rgc`, and it is never reallocated
while the VM runs. Because the registers and the stack are one array, the collector
traces both by scanning `r` up to `sp` — there is no separate root set for stack
contents, which is why the whole live state of a computation is visible to the
collector without any frame descriptors.

`sp` is a C pointer held in an instruction parameter. Its shadow is `rs`, an offset
from `r` rather than a pointer, so the collector may move nothing and the value stays
meaningful:

```c
#define unload_sp()   (rs = fixnum_obj(sp - r))
#define reload_sp()   (sp = r + get_fixnum(rs))
```

The accessors are the obvious ones — `spush`, `spop`, `sdrop`, `sgrow`, and `sref(i)`
for the *i*th word below the top, so `sref(0)` is the topmost. Under `_DEBUG` each of
them runs `_sck`, which asserts the pointer stays inside the array; in a release
build they compile to a pointer bump.

#### Overflow

`rz` marks a red zone short of the end, `VM_STACK_RSZ` words from the top, and the
check is a single comparison:

```c
#define check_sp()  if (unlikely(sp > (obj*)rz)) fail("stack overflow")
```

It appears only in `callsubi()`, so the depth is tested once per call rather than
once per push. That is what the red zone is for: between two checks an instruction
may push several words, and the headroom absorbs them. Anything that pushes without
going through a call — including the failure machinery described below — is relying
on that headroom.

### Frames

A frame is two words. `save` pushes the current closure and the offset to resume at
inside that closure's own code:

```c
define_instruction(save) {
  int dx = get_fixnum(*ip++);
  spush(rd);
  spush(fixnum_obj(ip + dx - &vector_ref(procedure_ref(rd, 0), 0)));
  gonexti();
}
```

`return` takes them back in the other order:

```c
define_instruction(return) {
  rx = spop();
  rd = spop();
  retfromi();
}
```

So the topmost word of a frame is the offset and the one below it is the procedure.
A tail call pushes no frame at all, which is worth remembering when reading a stack:
a chain of tail calls leaves no trace, and that is correct rather than lossy.

Arguments are separate from the frame, and a real call uses a different convention
from the instruction one above: it leaves the argument *count* in the accumulator and
all the arguments on the stack, with the *first* on top. The callee is in `ac` at the
moment of the call, having been loaded last for the same right-to-left reason:
`(g a b)` compiles to `%3 .2 , .2 , .2 [32`. Inside the callee, `(lambda (a b) a)`
compiles to `%2.0]2` and `(lambda (a b) b)` to `%2.1]2`, so `sref(0)` names the first
parameter. Every mechanism on this page follows that order.

#### Reading a stack image

Given a stack image and nothing else, a frame start can be recognised — a closure,
followed by an exact non-negative offset that is legal for *that closure's own* code
vector — but this is a heuristic and not a parse. It never misses a real frame, since
every frame satisfies it by construction, and it does report extra ones: a procedure
stored as data next to a small fixnum passes the same test. Nothing in the VM records
frame boundaries, so there is no exact alternative short of decoding each callee's
stack effect.

### Continuations

A continuation is a closure whose free variables are a copy of the stack:

```
[0]    continuation-adapter-code     a code vector, shared by every continuation
[1]    the dynamic state at capture
[2..]  the stack image, oldest word first
```

`lck` and `lck0` build it — `hp_reserve`, then `objcpy` of the live stack into the
block, then the two header cells. Capturing therefore costs a copy proportional to
the current depth, and nothing else; there is no stack segmentation or copy-on-write.

Because a continuation is an ordinary closure, `closure->vector` will show you all of
this, and `closure-ref` and `closure-length` will read it without copying the block —
which matters, since the image can be as large as the whole stack.

#### Invoking one

The adapter code's first instruction is `rck`, and it begins by comparing the
captured dynamic state against the current one. If they differ, it falls through to
the rest of the adapter, which calls `%dynamic-state-reroot!` to unwind and rewind
the `dynamic-wind` chain and then re-enters `rck` — this time with the states equal.

Once they agree, reinstating the stack is a copy in the other direction. `wckr` is
the common path, for a continuation invoked with one value:

```c
sp = r + VM_REGC;
objcpy(sp, &procedure_ref(x, 2), n);
sp += n;
ac = o;
rx = spop();
rd = spop();
retfromi();
```

Note the last three lines: after restoring the image it pops the topmost two words
*as a return frame*. That is a property worth stating on its own, because other
things depend on it — the top of a continuation's stack image is always the frame it
will return through.

#### The dynamic state is not on the stack

`cx_dynamic_state` is a rooted C global, not a stack word. Discarding the stack
therefore does not lose the `dynamic-wind` chain, and `parameterize` — which is built
on `dynamic-wind` over a mutable closure cell — keeps working across any control
transfer that goes through a continuation, including ones that abandon the stack
entirely.

#### A stack image put to another use

A VM failure object is a continuation in every respect but one: the return frame at
the top of its image is a distinguished closure whose code is a bare `halt`. It is
built with the same adapter code and the same dynamic-state cell, so `closure->vector`
and anything that walks a continuation work on it unchanged, and the invocation path
accepts it as genuine — but because `wckr` returns through the topmost frame, calling
one halts cleanly instead of resuming a computation whose stack is meaningless. The
failure machinery is assembled by `build_fail_object` in `i.c`; what it borrows from here is exactly the
frame-on-top rule above.

### Returning several values

An instruction leaves one value, in the accumulator. Several values are therefore a
protocol rather than a representation, and it has two halves: a fast path where the
values never leave the stack, and two boxed forms for when they have to pass through
a single-value slot.

#### Producing: values

`values` is the `sdmv` instruction, entered with the values on the stack and their
count in the accumulator. It has three cases.

One value is the ordinary case and costs nothing extra — pop it into the accumulator
and return through the frame.

Otherwise `sdmv` looks *past* the values, at `sref(n)` and `sref(n+1)`, for a marker
left by `call-with-values`. If it is there, the consumer is right below it: the values
are slid down over the marker with `objmove_left` and the consumer is tail-called
with them as its arguments. No allocation, and no intermediate object.

If the marker is absent the values are being returned into a slot that can hold only
one, so they are boxed: no values becomes the *unit* object, and several become a
*tuple* built by `hend_tuple`.

#### Consuming: call-with-values

`call-with-values` is an integrable whose body is an arity check, a pop into the
accumulator, and the `cwmv` instruction. `cwmv` pushes three words — the consumer,
`callmv-adapter-closure`, and a zero offset — and then calls the producer with no
arguments.

Those three words are deliberately shaped like a frame: a procedure and a return
offset, with the consumer stashed below them. So a producer that returns *one* value
does not need to know anything about `call-with-values` at all; it returns through
what looks like an ordinary frame, and lands in the adapter closure's code, which is
the `rcmv` instruction. And a producer that returns several recognises the adapter
closure in that frame and takes the fast path instead.

`rcmv` is the single-value landing pad, and it normalises whatever arrived:

```c
if (is_unit(val))       ac = fixnum_obj(0);
else if (is_tuple(val)) { spread it onto the stack; ac = count }
else                    { spush(val); ac = fixnum_obj(1); }
```

then tail-calls the consumer. `rck` carries the same three-way logic for the case
where a *continuation* is invoked with some number of values and its saved stack top
turns out to be a `call-with-values` marker.

`cwmv` has one further path. When the consumer is itself a continuation whose dynamic
state still matches, the whole thing runs in constant space: the continuation's image
becomes the stack and the producer is called there directly.

#### The two boxed forms, and what they look like from Scheme

Neither boxed form is a degradation — both are unboxed again by the next consumer
that asks for values. Bind one to a variable to see it, then hand it on:

    (define t (let ([x (values 1 2)]) x))       t  =>  #<values 1 2>
    (define u (let ([x (values)])   x))         u  =>  #<values>

    (call-with-values (lambda () t) (lambda a a))  =>  (1 2)
    (call-with-values (lambda () u) (lambda a a))  =>  ()

A tuple that escapes into ordinary code is a block whose cell 0 is `obj_from_size(0)`
— the same discriminator the object layer uses for tuples — and it writes as
`#<values 1 2>`. It is not a vector, a pair or a procedure, and the values protocol
is the only thing that makes one.

Zero values become the unit object, a tagged immediate under `UNIT_ITAG` with no
payload, which writes as `#<values>`. It is compared for identity, never decoded, and
there is no unit predicate exposed to Scheme — only `is_unit` in C, so from Scheme
it is recognisable by what it is not: it satisfies no type predicate at all.

The REPL is one of those consumers, which is why a unit almost never reaches a
prompt: a top-level form's value returns through `rcmv`, so a unit result is unboxed
to zero values and there is nothing to echo. `repl-evaluate-top-form` prints each
value it receives, so `(values 1 2)` typed at a prompt prints two lines.

Both cases are outside what R7RS defines — returning zero or several values to a
continuation that wants exactly one is an error — so this is latitude rather than
deviation. It is worth knowing when reading a confusing value in a REPL transcript.

#### dynamic-wind passes values through

`dynamic-wind` is not a C instruction; it is bytecode in the module table in `i.c`,
and its internal lambda ends in `sdmv` — the comment above the encoding says so
directly. That is why it is transparent to the protocol:

    (call-with-values
      (lambda () (dynamic-wind (lambda () #f) (lambda () (values 1 2 3)) (lambda () #f)))
      (lambda a a))                                                      =>  (1 2 3)

with zero values passing through the same way. `%dynamic-state-reroot!`, the routine
the continuation adapter calls, is bytecode in that same table.
