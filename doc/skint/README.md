## The (skint ...) libraries

One page per library, describing what it exports and how to use it. Each is imported
the usual way — `(import (skint fx))` — and each has a test file of the same name
under `test/skint/`.

Up: [documentation index](../README.md). For how the interpreter is built rather than
how it is used, see [internals/](../internals/README.md).

### Numbers

[fx.md](fx.md) — *(skint fx)*, fixnum arithmetic: the operations that stay inside a
machine word, with the overflow behaviour spelled out.

[fl.md](fl.md) — *(skint fl)*, flonum arithmetic, including the parts that depend on
what the host's libm provides.

[ivset.md](ivset.md) — *(skint ivset)*, sets of integers held as sorted intervals,
for when the membership is dense in runs.

### Time and dates

[time.md](time.md) — *(skint time)*, points in time and durations.

[date.md](date.md) — *(skint date)*, calendar dates, built on the above.

### Output

[format.md](format.md) — *(skint format)*, formatted output in the `~a` `~s` `~d`
tradition, with the directive table.

[print.md](print.md) — *(skint print)*, configurable printing and a pretty-printer:
width, depth and length limits, cycle marking, and the parameters that control them.

### Looking at a running interpreter

[apropos.md](apropos.md) — *(skint apropos)*, finding identifiers by name across the
libraries and the global store.

[disasm.md](disasm.md) — *(skint disasm)*, reading compiled code back: bytecode, core
forms, and the source-like rendering.

[debug.md](debug.md) — *(skint debug)*, the stack an error left behind: the frames
behind the `,db` command, and how to print them.

[trace.md](trace.md) — *(skint trace)*, tracing procedure calls and returns.

### Command lines

[getopt.md](getopt.md) — *(skint getopt)*, parsing command line options.

### Reaching these from the prompt

Several of these have a comma command that imports the library on first use, so you
need not import it by hand while exploring — see
[repl-commands.md](../repl-commands.md).
