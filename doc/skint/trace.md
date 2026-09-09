## (skint trace) — tracing procedure calls

A tracing package modelled on Chez Scheme's. A traced procedure prints its
arguments when it is called and its results when it returns, indented to show how
calls nest, so that the shape of a computation can be read off the transcript.

```scheme
(import (skint trace))
```

Tracing is a debugging aid: it changes when output appears, how much a call costs,
and — as [Differences from Chez Scheme](#differences-from-chez-scheme) explains —
how much stack a loop uses. Nothing here is meant for production code.

### Reading a trace

Each call prints two lines: the call itself, written as `(name argument ...)`, and
the value it returned. Both carry an indentation prefix showing the nesting depth —
alternating `|` and space, one character per level, so the outermost call is `|`,
one level in is `| `, two levels in is `| |`. From depth ten on, the prefix is
replaced by the depth in brackets: `[10]`.

```scheme
(define half
  (trace-lambda half (x)
    (cond [(zero? x) 0]
          [(odd? x) (half (- x 1))]
          [(even? x) (+ (half (- x 1)) 1)])))

(half 5)
```

```
|(half 5)
| (half 4)
| |(half 3)
| | (half 2)
| | |(half 1)
| | | (half 0)
| | | 0
| | |0
| | 1
| |1
| 2
|2
```

A procedure returning several values prints them on one line, separated by spaces;
one returning no values prints an empty line.

### trace-lambda

`(trace-lambda name formals body ...)` → *procedure*

A `lambda` that traces itself under `name`. The formals may be anything `lambda`
accepts — a proper list, an improper one, or a single rest argument — and the body
may open with internal definitions, as any body may.

```scheme
(define add (trace-lambda add (a b) (+ a b)))
(add 1 2)
```

```
|(add 1 2)
|3
```

`name` is only a label for the trace; it need not be the variable the procedure is
bound to, and it is not bound inside the body. To trace a recursive procedure by
its own name, define it in the ordinary way and let the body refer to the variable,
as `half` does above.

### trace-case-lambda

`(trace-case-lambda name clause ...)` → *procedure*

A `case-lambda` that traces itself under `name`. The clause that runs is not shown;
what the trace records is the call and its result.

```scheme
(define pick
  (trace-case-lambda pick
    [(x) (list 'one x)]
    [(x y) (list 'two x y)]
    [args (list 'many args)]))

(pick 7 8)
```

```
|(pick 7 8)
|(two 7 8)
```

### trace-let

`(trace-let name ((var init) ...) body ...)` → *values of the body*

A named `let` that traces each entry into the loop under `name`. It is exactly

```scheme
((letrec ([name (trace-lambda name (var ...) body ...)]) name) init ...)
```

so the body may hold internal definitions, and the bindings may be written with
brackets like any others.

With no bindings at all it becomes a way to show the value of one expression at the
current depth, which is useful for looking inside a larger trace:

```scheme
(trace-let step () (* 6 7))
```

```
|(step)
|42
```

### trace-do

`(trace-do ((var init step ...) ...) (test result ...) command ...)` → *values of
the results*

A `do` loop that prints its variables on each iteration, under the name `do`, and
its result when the loop ends. As in SKINT's own `do`, the step of a binding may be
omitted, the result sequence is a body, and the commands are a body too.

```scheme
(trace-do ([old '(a b c) (cdr old)]
           [new '() (cons (car old) new)])
  ((null? old) new))
```

```
|(do (a b c) ())
|(do (b c) (a))
|(do (c) (b a))
|(do () (c b a))
|(c b a)
```

Every iteration prints at the same depth and one result line closes the loop,
because the iteration is a tail call that this form controls and can therefore
recognise. `trace-do` is the one form here that does not deepen with each step, and
the only one that leaves the loop's tail call intact.

### trace-define

`(trace-define (var . formals) body ...)`
<br>`(trace-define var expr)`

Defines `var` as a traced procedure named `var`. The first form is a procedure
definition, taking the same formals as `define` does. The second takes an
expression that must produce a procedure, and wraps it.

```scheme
(trace-define (plus x y) (+ x y))
(trace-define timesthree (lambda (x) (* x 3)))
```

Like `define`, it may appear at top level or at the start of any body.

### trace and untrace

`(trace var ...)` → *list of names*
<br>`(trace)` → *list of names*
<br>`(untrace var ...)` → *list of names*
<br>`(untrace)` → *list of names*

`trace` replaces the value of each variable with a traced procedure wrapping what
was there, and returns the names it was given. `untrace` puts the original values
back and returns the names it actually restored. Called with no arguments, `trace`
lists what is currently traced and `untrace` restores all of it.

```scheme
(define (sq x) (* x x))
(trace sq)        ; => (sq)
(sq 3)
```

```
|(sq 3)
|9
```

A variable counts as traced only while it still holds the procedure that `trace`
installed. Assigning it anything else — including redefining it — silently drops it
from the list, so a later `untrace` will not overwrite the new value:

```scheme
(trace sq)                 ; => (sq)
(define (sq x) (+ x x))    ; redefined, so no longer traced
(trace)                    ; => ()
(untrace sq)               ; => (), and sq keeps its new definition
```

It is the value that is traced, not the binding. A traced procedure saved elsewhere
before `untrace` goes on tracing.

It is an error to trace a variable that cannot be assigned. Bindings imported from
a library — which is where every standard procedure comes from — are immutable, so
`(trace car)` is rejected when it is expanded. To watch calls that go through such
a procedure, bind it to a variable of your own and trace that, remembering that only
calls through your variable are traced:

```scheme
(define mycar car)
(trace mycar)     ; => (mycar)
```

### trace-output-port and trace-print

`(trace-output-port)` → *port*
<br>`(trace-output-port port)`
<br>`(trace-print)` → *procedure*
<br>`(trace-print proc)`

Two parameters controlling where trace output goes and how it is written.
`trace-output-port` starts out holding the current output port as it stood when the
library was first imported; it is not tied to `current-output-port` afterwards.
`trace-print` holds a procedure of an object and a port, and starts out as `write`.

```scheme
(define p (open-output-string))
(parameterize ([trace-output-port p])
  (add 1 2))
(get-output-string p)              ; => "|(add 1 2)\n|3\n"
```

Because `write` is the default, strings in a trace appear quoted. Supply another
printer to change that, or to abbreviate large arguments:

```scheme
(parameterize ([trace-print (lambda (x port) (display "<" port)
                                             (display x port)
                                             (display ">" port))])
  (add 1 2))
```

```
|<(add 1 2)>
|<3>
```

### Differences from Chez Scheme

Four differences are worth knowing, all of them consequences of what a library
written in Scheme can observe.

*Tail calls are not distinguished from other calls.* Chez prints a tail call at the
same depth as its caller and shows a single result for both, because a tail call
returns to the same place. Here every traced call deepens the indentation and
prints its own result. Whether a call is in tail position is not something the
calling procedure can find out, so the distinction cannot be reconstructed. The one
exception is `trace-do`, whose loop belongs to the form itself and is therefore
known to be a tail call.

*A traced procedure is not tail recursive.* Printing the result means the call
cannot be a tail call, so a traced loop consumes stack in proportion to the number
of iterations, and a loop that runs indefinitely untraced will exhaust the stack
once traced. Trace an inner procedure rather than the loop itself when this
matters, or use `trace-do`, which keeps its tail call.

*Only assignable variables can be traced,* as described under `trace` above.

*There is no `trace-define-syntax`.* Chez can trace a macro transformer, printing
each form it is given and the form it produces. A transformer here is a procedure
that the expander builds and calls, and the only ways to make one are `syntax-rules`
and `syntax-lambda`; neither can wrap an existing transformer, and expansion is not
reachable from ordinary code. Tracing macro expansion would need support from the
interpreter itself.
