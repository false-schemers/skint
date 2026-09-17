## Implementation notes

Numbered notes referenced from the sources. A comment block long enough to push code
off the screen lives here instead, and the source keeps a one-line pointer:

```c
/* see notes.md [3] */
```

Numbers are permanent: add at the end, and leave a retired entry in place saying what
became of it rather than renumbering. Nothing outside this file depends on the order.

### [1] Why a procedure is recognised by cell 0 alone

`n.h`, the procedure section.

A procedure — a VM closure — is a block with a pointer to its code vector in cell 0.
No other block kind can look like that: tuples, vectors, boxes and pairs keep a size
immediate in cell 0, records keep a symbol immediate, and a native keeps a type
pointer in its header word with its payload pointer — which lies outside the heap —
in cell 0. So the quick test reads cell 0 and needs no header or size check of its
own. The debug versions in `n.c` return the same answers and assert the whole
convention on the way.

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

`n.c`, the tagged/typed section and the procedure section.

They give the same answers as the quick tests in `n.h`, with the convention asserted.

For tagged and typed objects: that the object is a block — not a native — and has a
cell 0 of its own. The quick versions never look at the header, which is sound only
because a native keeps its payload pointer in cell 0, and that can be neither a small
size immediate nor a symbol, and because no block is ever built with a length of
zero.

For procedures the assertion runs in both directions: on a yes, that the object
really is a well-formed closure; on a no, that nothing closure-shaped was passed
over. The second is the one that catches a block carrying a foreign pointer in cell 0
— the shape of the static procedures sfc used to emit, which nothing constructs now.
