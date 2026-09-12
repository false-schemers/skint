## SKINT internals

Notes on how the interpreter is built, for people working on it rather than with it.
User-level documentation lives in `doc/skint/` and describes libraries; this
directory describes the machine underneath.

| Document | Covers |
|---|---|
| [memory.md](memory.md) | what an `obj` is, the heap, and the garbage collector's five object categories |
| [objects.md](objects.md) | how those five categories are subdivided into Scheme types, and the `n.h` interface |
| [vm.md](vm.md) | the threaded-code VM: registers, dispatch, the trampoline, and writing instructions |
| [bytecode.md](bytecode.md) | the compiler's intermediate language and its conversion to threaded code |
| [builtins.md](builtins.md) | how `i.h` becomes the instruction tables, and how those become Scheme bindings |
| [store.md](store.md) | the global store: where non-local variables get their locations, and how names resolve to them |
| [registries.md](registries.md) | expand-time name registries: denotations, environments, and how libraries are bound to their names |

### The layers

Four layers sit between the C compiler and a running Scheme program, and each is
deliberately unaware of the one above it.

*The collector* sees five kinds of word — False, Immediate, Block, Native, Foreign —
and knows nothing else. It can copy and trace the entire heap without a single
object-layout descriptor.

*The object layer* subdivides Immediate, Block and Native into Scheme types by
convention: a secondary tag in the immediate word, the contents of cell 0 in a block,
a type-descriptor pointer in a native's header. All of it is invisible to the layer
below.

*The virtual machine* executes threaded code — an `obj` vector of C function
addresses and their operands, each function tail-calling the next. Its state lives in
C function parameters, with shadow copies in the register file so the collector can
see them.

*The compiler* emits a printable bytecode that nothing ever executes. It is an
intermediate language, converted to threaded code once by a routine inside the VM.

### Where things are

| File | Holds | Generated from |
|---|---|---|
| `n.h`, `n.c` | object representation, strings, ports, numeric helpers | hand-written |
| `k.h` | the VM register protocol and the VM entry points the kernel calls | hand-written |
| `k.c` | kernel globals, startup, `main`, and the garbage collector | hand-written |
| `i.h` | the instruction table — encodings, operand types, integrables | hand-written |
| `i.c` | the VM: instruction implementations and the bytecode decoder | hand-written |
| `s.h` | platform and toolchain conditionals | hand-written |
| `s.c` | the standard library, as bytecode strings | `pre/s.scm` |
| `t.c` | expander, compiler, library system and REPL, as bytecode strings | `pre/t.scm` |
| `opt/` | tower, Unicode and enhanced-tty subsystems | mixed |

`s.c` and `t.c` are build artifacts and are committed only because regenerating
them needs a working `skint`. Edit the `pre/` sources for those two.

`n.h` and `n.c` are not in that category. They were generated from `pre/n.sf`
originally, but have been maintained by hand for several releases and the script
that generated them no longer exists; edit them directly. `pre/n.sf` is a leftover
of that era and describes nothing that is still built — it does not describe `n.h` or
`n.c`, and changing it will not change them.

`s.h` is where the feature-test macros are selected, and those are only honored
before the C library headers are read — so **every translation unit must include
`s.h` before `n.h`, `i.h` and `k.h`**. All three check for it and stop the build
with an `#error` rather than compiling against a different configuration from the
rest of the program.

`i.h` has no include guard, on purpose: it is the instruction table, and each
translation unit reads it several times with a different `VM_GEN_*` macro defined to
turn the table into a different piece of code. So it must contain nothing but the
table and the macros that expand it — which is why the VM register protocol and the
`vm_*` declarations live in `k.h` instead. **Where both are included, `k.h` comes
after `i.h`.** `k.c` needs only `k.h`: it no longer knows anything about
instructions.

`k.c` itself used to be generated, from `pre/k.sf` by an external compiler
(`sfc`), and patched by `pre/ksf2c.ssc`; it is hand-written now, and neither of
those `pre/` files describes anything that is still built. The collector at the end
of `k.c` is a special case: it is `sfc`'s own runtime rather than code written for
SKINT, and it is kept as it came, with those patches folded in. See [memory.md](memory.md).

### Build-time switches that change the internals

| Switch | Effect |
|---|---|
| `NAN_BOXING` | 64-bit `obj` with inline NaN-boxed flonums; assumes a 48-bit address space |
| `NDEBUG` | the `n.h` accessors become macros without assertions rather than checking functions |
| `OPT_TOWER` | bignums, rationals and complex numbers; adds native types and a second instruction file |
| `OPT_UNICODE` | multi-byte string representation |
| `OPT_ENHTTY` | two more port types |

Two more are chosen by the compiler rather than by the builder, and both change the
VM's calling convention: `VM_MUSTTAIL_GUARANTEE`, when guaranteed tail calls are
available, and `VM_AC_IN_REG`, when the accumulator can be passed in a register. See
[vm.md](vm.md#dispatch).
