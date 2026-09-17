## Implementation notes

Numbered notes referenced from the sources. A comment block long enough to push code
off the screen lives here instead, and the source keeps a one-line pointer:

```c
/* see notes.md [3] */
```

Numbers are permanent: add at the end, and leave a retired entry in place saying what
became of it rather than renumbering. Nothing outside this file depends on the order.

### [1] Why a procedure is recognised by its header alone

`n.h`, the procedure section.

A procedure — a VM closure — is a block whose header carries `CLOSURE_MTAG`, so
`is_procedure` is that one test: a heap pointer whose header word has the closure
tag. Nothing about cell 0 enters into it. The debug versions in `n.c` return the
same answers and assert the rest of the shape — that cell 0 really is a code vector
of at least one instruction word.

This replaced an older test that read cell 0 and asked whether it pointed into the
heap. That worked because every other block kind was obliged to keep a non-pointer
there — a size immediate for vectors, boxes, pairs and tuples, a symbol for a
record's type — and it is the obligation that made a record type descriptor have to
be an immediate. With the kind in the header, cell 0 is free: a pair's car is cell 0,
and an rtd can be any object but `#f`.

`is_procedure` is a macro rather than a static function because it sits in every call
instruction, where the extra inlining step costs the register allocator six
instructions per call. It evaluates its argument twice, so pass it a variable, as
every caller does.

### [2] What a failure object is, and how it is built

`i.c`, `build_fail_object`.

It is shaped as an ordinary continuation — the same adapter code at `[0]`, the
dynamic state at `[1]`, a stack image from `[2]` — so `closure->vector` and anything
that walks a continuation work on it unchanged, and `wck`/`wckr`/`rck` accept it as
the real thing.

What makes it safe is the top of its stack image. `wckr` restores the image and pops
the topmost two slots as the return frame, so those two slots hold
`cx_failure_halt_closure` and a zero offset. Invoking a failure object therefore
returns straight into a bare `halt` and resets cleanly, instead of resuming a
computation whose stack is meaningless. The failing stack is not re-enterable, and
this is how that is enforced without a special case anywhere else.

Layout of the finished block, length `n + 4` where `n` is the captured region:

```
[0]      cx_continuation_adapter_code
[1]      cx_dynamic_state
[2..]    the vm stack as it stood, oldest first, ending with the irritants,
         their count, and the message
[len-2]  cx_failure_halt_closure   ) the return frame that makes an
[len-1]  0                         ) accidental call halt
```

So from the end: message at `len-3`, count at `len-4`, irritants below that. The
caller has already pushed the irritants and their count; `build_fail_object` pushes
the message, snapshots, and leaves a halt return frame on the now-empty stack for the
handler call that follows.

`doc/internals/stack.md` covers the surrounding machinery.

### [3] Why the failing frame is pushed, and why it is guarded

`i.c`, `push_failing_frame`.

The failing procedure is in `rd` and the point it failed at in `ip` — registers, not
stack — so a plain snapshot of the stack misses the innermost frame, the one that
actually failed. Pushing it the way `save` does makes a walk of the captured stack
start at the failure point rather than at its caller.

It is guarded because not every failure arrives with `rd` and `ip` agreeing: the
stack overflow raised from `callsubi()` runs with `rd` already holding the callee
while `ip` still addresses the caller's code, where the offset would be meaningless.
Better no innermost frame than a false one. The red zone, `VM_STACK_RSZ`, leaves room
for these two words even when the failure is the overflow itself.

For anything that walks this: the values sitting above this frame are whatever
happened to be on the stack mid-instruction, so unlike a frame that a completed
`save` pushed, its extent is arbitrary. A walker must not assume the innermost frame
is shaped like the rest.

### [4] The bytecode of dynamic-wind's internal lambda

`i.c`, the module table entry for `dynamic-wind`.

```
,    save argc by pushing it on top of args in stack
${   push new frame for return from %dynamic-state-reroot!
:0   get 'here' dynamic state from internal lambda's display
,    put it on the stack for dynamic-state-reroot!
@(y22:%25dynamic-state-reroot!)   get the d-s-r! procedure
[01  call it with 1 argument ('here' dynamic state)
}    we will return here when d-s-r! is finished
_!   pop saved argc from stack into ac register
K6   use sdmv opcode to return args from the lambda
```

`%x` procedure checks are inserted for early error detection. Ending in `sdmv` is
what makes `dynamic-wind` transparent to multiple values.

### [5] The bytecode of the continuation adapter

`i.c`, the `"K"` module table entry.

```
k!   first attempt; does not return if nothing to un/re-wind
,    save argc by pushing it on top of args in stack
${   push new frame for return from %dynamic-state-reroot!
:0   get old dynamic state from continuation's display
,    put it on the stack for dynamic-state-reroot!
@(y22:%25dynamic-state-reroot!)   get the d-s-r! procedure
[01  call it with 1 argument (old dynamic state)
}    we will return here when d-s-r! is finished
_!   pop saved argc from stack; we are ready to retry
k!   retry; should not return this time
%%   signal an (argument?) error if we return
```

### [6] Why closure? is kept apart from procedure?

`i.c`, the `clop` instruction.

`closure?` answers whether its argument is a heap-allocated VM closure, i.e. a block
whose cell 0 is a code vector. That is the thorough form of the test `procedure?`
makes: `procedure?` settles for any heap pointer in cell 0, which nothing but a code
vector can be, so the two agree on every object the VM builds.

They are kept apart because `closure?` is the one that cannot be fooled by a
hand-made block, and because `(skint disasm)` reads closures and wants to be sure of
one. Their bodies are also deliberately not identical, so that `/OPT:ICF` has no pair
to fold — see the `.text$vm` arrangement.

### [7] What the debug predicates in n.c assert

`n.c`, the typed section and the procedure section.

They give the same answers as the quick tests in `n.h`, with the convention asserted.

For typed objects: that the object is a block — not a native — and has a cell 0 of
its own for the rtd. The quick version reads the header alone, which is sound
because a native's header holds a type-descriptor pointer, and a pointer can never
carry a microtag pattern: a header is odd, a type pointer is not.

For procedures the assertion runs in both directions: on a yes, that the object
really is a well-formed closure; on a no, that nothing closure-shaped was passed
over.

