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


### [8] Why a bytevector output port carries a length of its own

`n.h`, the bytevector output port section; `n.c`, `bvoctl`.

A bytevector output port is a `cbuf_t`, and until positions arrived its `fill` was
both the write position and the end of the data: `get-output-bytevector` handed back
`fill - buf` bytes. `CTLOP_SETPOS` breaks that identity. Seeking back to byte 3 of a
ten-byte port and writing one byte there has to leave the other nine in place, which
is what the same seek does to a file; moving `fill` back to 3 would instead have made
the port three bytes long and the write would have ended it at four.

So the port is a `bvofile_t`: the buffer, plus `hwl`, the length the data had when a
backward seek moved `fill` off the end of it. The effective length is the larger of
the two, which is what `bvolen` returns and what `gos` and `gob` ask for. Nothing
else changes. `hwl` starts at zero and stays there unless a seek moves back, so a
port that is only ever appended to behaves exactly as it did; and the `cbuf_t` is the
first member, so `cbputc` goes on serving as the port's `putch` untouched.

The field is on this port rather than on `cbuf_t` because `cbuf_t` is also the
scratch buffer used throughout `n.c`, where `fill` is the length and nothing seeks.
The string output port is still a plain `cbuf_t` and answers -1 to both position
operations; whenever text ports get positions, it will need the same treatment.

Seeking past the end of the data is refused rather than zero-filling, on both kinds
of bytevector port: growing the buffer with bytes nobody wrote is a bigger promise
than the operation needs to make.

Two things follow for `gos` and `gob` in `i.c`, which hand back what such a port has
collected. The length has to come from `bvolen` rather than from `fill - buf`, or a
port that was seeked back loses its tail; and `cbdata` must not be used to get at the
bytes, because it writes its terminating zero *at* `fill`, which on such a port is a
byte of the data. The two instructions therefore take the bytevector port apart
themselves and leave `cbdata` to the string port, where `fill` really is the end.

### [9] How a port's position is passed around

`s.h`, the file offset section; `n.h`, the `ctlop_t` declaration; `n.c`, `bvictl`,
`bvoctl` and `bfctl`; `i.c`, the port position instructions.

A ctl method answers `-1` for an operation it does not implement, `0` on success,
and a positive code on failure, so a caller can tell "this port cannot do that" from
"that did not work". `noctl` is the method that refuses everything. The two position
operations are

```c
int xxxctl(CTLOP_POS, void *pdata, int64_t *ppos);
int xxxctl(CTLOP_SETPOS, void *pdata, int64_t *ppos, int origin);
```

where `origin` is `SEEK_SET`, `SEEK_CUR` or `SEEK_END` as for `fseek`. A null `ppos`
asks whether the operation is there at all — `-1` no, `0` yes — and passes no origin,
which lets a caller find out what a port can do without disturbing it. The failure
codes are `1`, the operation itself failed, and `2`, the position is out of range.

#### Why the ctl layer counts in int64_t

A byte offset is an integer, and a port has no business knowing that Scheme counts in
something else. Everything above the ports is free to change — `%port-tell` could
start handing back a record, or a rational — without a port method being touched.

It also gave the conversion one home instead of two. While positions travelled as
whole doubles, `n.c` had to test for wholeness and for the `int64_t` range on the way
in, and for double representability on the way out, and it needed `floor` and `ldexp`
to do it. None of that is in `n.c` now, and there is no floating point anywhere in
the port code.

#### What the instructions do with it

`%port-tell` returns the position as a fixnum when it fits one; failing that, as a
bignum where there is a tower, since a bignum holds any offset exactly; and failing
that, as a whole flonum. That last case is what puts a ceiling on a build with no
tower: past 2^53 a double can no longer name every integer, so rather than hand back
a rounded position the instruction answers `#f`, which the `(srfi 192)` cover turns
into a file error. The cap is deliberately the simple `> 2^53` rule and not a test of
whether each particular value survives the round trip: some larger integers do and
some do not, and a `port-position` that worked at 1e16 but failed at 1e16+1 would be
worse than one that declines predictably.

`%port-seek` accepts any integer form — a fixnum, a whole flonum, or a bignum with
the tower — and converts it to `int64_t`. A value that is not an integer is a type
error, as passing a string would be. A value that is an integer but names no
reachable position is not: it answers `#f`, and the cover raises a file error. The
origin is `0`, `1` or `2` in Scheme rather than the `SEEK_*` constants, whose values
C does not fix.

#### Offsets and the platform

`fileoff_t` in `s.h` is the widest offset the platform offers — `_ftelli64` and
`_fseeki64` on MSVC and MinGW, `ftello` and `fseeko` where `_POSIX_VERSION` is at
least 200112L, and plain `ftell` and `fseek` otherwise. A stream can outgrow a 32-bit
offset, so on a system whose `off_t` is still 32 bits the fallback is all there is and
positions past 2gb are out of reach. `FILEOFF_IMAX` is what an `int64_t` position is
checked against before it is narrowed.

The binary file ports are opened in binary mode, so an offset is a plain byte count.
A byte pushed back with `ungetc` has already moved the stream's own position back, and
`fseek` discards the pushback, so neither operation needs to adjust for it.

`bfctl` serves both binary file ports and passes everything else through to `fctl`,
which is where `CTLOP_OFL` is handled; the text file output port keeps `fctl` alone,
since text ports have no positions yet.

#### Two checks that are easy to misread

In `bvictl` and `bvoctl` the incoming offset is tested against the span of the data
*before* the origin is added, not only after. The result has to land in `[0, span]`,
so an offset wider than the span could not get there whatever the origin — and doing
it in that order is also what keeps the addition from overflowing.

In `bfctl` a negative `SEEK_SET` position is refused with code `2` rather than being
handed to `fseek`, which would fail it with code `1`. It is the one out-of-range
position that can be named without asking the stream: a relative one only turns out
to be out of range when `fseek` says so. The bytevector ports answer `2` for the same
mistake, so the two kinds of port agree.
