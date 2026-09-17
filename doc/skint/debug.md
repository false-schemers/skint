## (skint debug) — the stack an error left behind

The library behind the `,db` REPL command. It walks the stack captured when an
error happened and prints it frame by frame, each frame with the code it was
running. Most of the time it is used through `,db`, which fetches it on first
use; how the debugger is driven is described with that command, in
[repl-commands.md](../repl-commands.md#debugging).

```scheme
(import (skint debug))
```

Importing the library, or loading it any other way, installs its debugger as
the one `,db` runs. Nothing else changes: errors are reported, and the REPL
recovers from them, as before.

### Frames

A *frame* is a call that had not returned when the stack was captured. It is a
list of three things: the procedure, an exact integer saying where in that
procedure's code it stopped, and a list of the values it was working with.

`(failure-frames failure)` → *list of frames*
<br>`(continuation-frames k)` → *list of frames*

The frames of the stack captured in `failure`, an object for which
`failure-object?` from `(skint)` answers true, or in the continuation `k`.
The frame that was running comes first and its callers follow, outermost last.

The whole stack is there, the frames of whatever ran the program included. The
list is found by looking for what a frame looks like, so it can hold frames
that are not calls at all — a procedure and a small integer that happened to be
stored side by side look the same. Read it as everything that might be a frame.

### Printing

`(print-failure-frames failure)` → *unspecified*
<br>`(print-failure-frames failure port)` → *unspecified*

Writes the failure's message, then every frame of `failure-frames`, to `port`,
by default the current error port.

`(print-frames frames port)` → *unspecified*

Writes each of `frames` to `port`: a line with its number, its procedure and
where it stopped, and the values it was working with, followed, where the
procedure has a readable form, by that form with the expression the frame
stopped at marked. The form is what `da` from
[(skint disasm)](disasm.md) gives, and the mark is the one
[`print-cursor`](print.md#cursor) makes.

Values are cut short where they are long or deep, and a procedure is written
with its name where it has one, the way `da-print-hook` writes it.

```scheme
(import (only (skint) failure-object?))

(define (h x) (vector-ref x 10))

(guard (e ((failure-object? e) (print-failure-frames e)))
  (h (vector 1 2)))
```

```
Failure: argument is not a valid vector index: 10
  0: h @6
     >(lambda (.a) [vector-ref .a 10])
  ...
```

### The debugger

`(debugger)` → *unspecified*
<br>`(debugger last-error)` → *unspecified*

The debugger `,db` runs, with what the REPL kept of the last error as
`last-error`. With no argument, or `#f`, it says there is no error to debug and
returns. Otherwise it lists the program's frames, leaving out the REPL's own and
those of the error reporting, and reads commands from the current input port
until told to stop. Its output goes to the current error port.

What the REPL keeps of an error is its own business, so a program has no
`last-error` to pass; the procedure is exported so that the debugger can be
told apart and put back:

```scheme
(eq? debugger (current-debugger))   ⇒ #t   ; once the library is loaded
```
