## SKINT documentation

Three kinds of thing live here, and they are written for different readers.

| Where | For | Covers |
|---|---|---|
| [skint/](skint/README.md) | people using the interpreter | the `(skint ...)` libraries, one page each |
| [internals/](internals/README.md) | people working on the interpreter | how the thing is built, from the collector up |
| this directory | both | the interactive interpreter itself, and one language extension |

### In this directory

[repl-commands.md](repl-commands.md) — the comma commands available at the prompt:
importing, tracing, disassembling, apropos, pretty-printing and the rest. A line
beginning with a comma is a command rather than an expression.

[syntax-rules.md](syntax-rules.md) — SKINT's extensions to R7RS `syntax-rules`. The
standard features are all present; this page describes what is added on top, chiefly
the pattern and template escapes.

`skint.1.in` is the manual page template. It is not documentation to read directly:
`@VERSION@`, `@DATE@` and `@LIBDIR@` are substituted at build time.

### Elsewhere in the tree

The top-level [README](../README.md) covers building and installing. Library sources
are under `lib/`, their tests under `test/`, and each library documented in
[skint/](skint/README.md) has a test file named after it.
