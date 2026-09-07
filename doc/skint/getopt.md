# `(skint getopt)` — command line option parsing

A small AT&T/POSIX-style command line option parser, plus a matching `--help`
formatter. This is the parser SKINT itself uses for its own command line; the
library exposes it so scripts and programs can parse their arguments the same way
without pulling in a larger option-parsing SRFI.

```scheme
(import (skint getopt))
```

The library exports two procedures:

```scheme
(get-next-command-line-option args optmap return)
(print-command-line-options optmap [port])
```

The library needs nothing but `(scheme base)` on the client side, so it is usable
from ordinary R7RS programs.

## The option map

Both procedures are driven by an **option map**: a list of five-element records,
one per option.

```scheme
(define *options*
 '([verbose "-v" "--verbose" #f     "Increase output verbosity"]
   [output  "-o" "--output"  "FILE" "Write output to FILE"]
   [name    #f   "--name"    "NAME" "Set the name"]
   [count   "-n" #f          "N"    "Repeat N times"]
   [help    "-h" "--help"    #f     "Display this help"]))
```

| Position | Field | Meaning |
|---|---|---|
| 1 | *keysym* | Symbol identifying the option; handed back to your code |
| 2 | *short* | Short form, exactly two characters (`"-v"`), or `#f` |
| 3 | *long* | Long form (`"--verbose"`), or `#f` |
| 4 | *argname* | Placeholder for the option's argument, or `#f` if it takes none |
| 5 | *help* | One-line description, used by `print-command-line-options` |

The fourth field does double duty: it is the *name* shown in `--help` output, and
its presence is what marks the option as taking an argument. `#f` means the option
is a flag.

At least one of *short* and *long* must be present. The two are distinguished by
length, not by dashes: a token of exactly two characters is matched against the
*short* field, anything longer against the *long* field. So a short option must be
two characters (`"-v"`, not `"-vv"`), and a long option must be longer than two.

## `(get-next-command-line-option args optmap return)`

Parses **one** option from the front of `args` and calls

```scheme
(return keysym optarg restargs)
```

exactly once, with:

- **`keysym`** — the symbol from the matched record, or `#f` if there are no more
  options to parse;
- **`optarg`** — the option's argument as a string, or `#f` if it takes none;
- **`restargs`** — the arguments remaining after what was just consumed.

The procedure returns whatever `return` returns, which makes a driving loop
natural to write:

```scheme
(define (parse args)
  (let loop ([args args] [found '()])
    (get-next-command-line-option args *options*
      (lambda (keysym optarg restargs)
        (if keysym
            (loop restargs (cons (cons keysym optarg) found))
            (values (reverse found) restargs))))))

(parse '("-v" "--output=out.txt" "a.scm" "b.scm"))
; => ((verbose . #f) (output . "out.txt"))  and  ("a.scm" "b.scm")
```

### Accepted syntax

Arguments may be attached or separate, in both short and long forms:

```scheme
"--output=f.txt"    ; long, attached with =
"--output" "f.txt"  ; long, separate
"-of.txt"           ; short, attached
"-o" "f.txt"        ; short, separate
```

Flags may be clustered behind a single dash, and an argument-taking option may
end the cluster:

```scheme
"-vq"               ; => verbose, then quiet
"-vo" "f.txt"       ; => verbose, then output="f.txt"
```

An option argument is taken verbatim, so values that look like options are fine:

```scheme
"-n" "-5"           ; => count="-5"
"-n-5"              ; => count="-5"
```

### Where parsing stops

`keysym` comes back `#f` in four situations, which differ in what they leave in
`restargs`:

| Input | `restargs` | Note |
|---|---|---|
| `()` | `()` | Nothing left |
| `("--" …)` | the `…` | The `--` is **consumed** |
| `("-" …)` | `("-" …)` | The `-` is **kept** — conventionally means stdin |
| `("file" …)` | `("file" …)` | First non-option operand, kept |

Note the third and fourth rows: a lone `-` and an ordinary operand are both left
in place for you to handle.

**Options are not permuted.** Parsing stops at the first non-option argument, and
anything option-shaped after it is left untouched:

```scheme
(parse '("-v" "file.scm" "-q"))
; => ((verbose . #f))  and  ("file.scm" "-q")   ; -q is NOT parsed
```

This is POSIX behaviour, not GNU behaviour. It is what lets a script pass its own
trailing arguments through to a program it invokes without them being eaten.

### Bad command lines

These are errors, and since command lines come from users rather than from your
own code, they are the cases to think about:

- an option that is not in the option map
- an argument-taking option with nothing left to take
- an argument attached to a flag, as in `--verbose=x`

A bare negative number is option-shaped, so it counts as an unknown option rather
than an operand:

```scheme
(parse '("-5"))     ; an error, not the operand "-5"
```

If your program takes negative numbers as operands, put them after `--`. As an
*argument* to an option (`-n -5`) they work without ceremony.

If you want to print your own usage message, check the arguments before parsing
rather than relying on how a bad command line is reported.

## `(print-command-line-options optmap [port])`

Writes the option map as an aligned `--help` listing. `port` defaults to the
current output port, so the listing follows `parameterize` and
`with-output-to-string` as you would expect. Returns the column offset at which
the help text begins, so you can align additional lines with it.

```scheme
(print-command-line-options *options*)
```

```
  -v, --verbose        Increase output verbosity
  -o, --output=FILE    Write output to FILE
  --name=NAME          Set the name
  -n N                 Repeat N times
  -h, --help           Display this help
```

Each line is indented two spaces; the forms are joined with `", "`; an argument
name is attached with `=` to a long form and with a space to a short-only form;
and the help column is padded to clear the widest entry, plus four spaces. The
call above returns `23`, the offset of the help column.

A record must have at least one of the two forms; one with neither is an error.

## Worked example: SKINT's own command line

SKINT parses its own command line with these two procedures. Reduced to its
structure, the option map and driver look like this:

```scheme
(define *skint-options*
 '([verbose        "-v" "--verbose"        #f     "Increase output verbosity"]
   [quiet          "-q" "--quiet"          #f     "Suppress nonessential messages"]
   [append-libdir  "-A" "--append-libdir"  "DIR"  "Append a library search directory"]
   [prepend-libdir "-I" "--prepend-libdir" "DIR"  "Prepend a library search directory"]
   [define-feature "-D" "--define-feature" "NAME" "Add name to the list of features"]
   [undef-feature  "-U" "--undef-feature"  "NAME" "Remove name from the list of features"]
   [eval           "-e" "--eval"           "SEXP" "Evaluate and print an expression"]
   [load           "-l" "--load"           "FILE" "Load file and continue processing"]
   [script         "-s" "--script"         "FILE" "Run file as a Scheme script"]
   [program        "-p" "--program"        "FILE" "Run file as a Scheme program"]
   [version        "-V" "--version"        #f     "Display version info"]
   [help           "-h" "--help"           #f     "Display this help"]))
```

The help text SKINT prints for `--help` is `print-command-line-options` applied to
that map:

```scheme
(define (print-help!)
  (format #t "SKINT Scheme Interpreter v~a~%" *skint-version*)
  (format #t "usage: skint [OPTION]... [FILE] [ARG]...~%~%")
  (format #t "Options:~%")
  (print-command-line-options *skint-options* (current-output-port))
  (format #t "~%")
  (format #t "'--' ends options processing. Standalone FILE argument is treated as a script.~%")
  (format #t "If no FILE is given, skint enters Read-Eval-Print loop (stdin>eval-print>stdout)~%"))
```

And the driver is a loop that acts on each option as it arrives, threading one
extra piece of state — whether a REPL should still be entered at the end:

```scheme
(let loop ([args (cdr (command-line))] [repl? #t])
  (get-next-command-line-option args *skint-options* ;=>
    (lambda (keysym optarg restargs)
      (sexp-case (if optarg (list keysym optarg) (list keysym))
        [(verbose) (set! *verbose* #t) (loop restargs #t)]
        [(quiet)   (set! *quiet* #t)   (loop restargs #t)]
        [(append-libdir *)  (append-library-path! optarg)  (loop restargs #t)]
        [(prepend-libdir *) (prepend-library-path! optarg) (loop restargs #t)]
        [(load *)    (load optarg)      (loop restargs #t)]
        [(eval *)    (eval! optarg #t)  (loop restargs #f)]
        [(script *)  (set! *quiet* #t) (exit (run-script optarg restargs))]
        [(program *) (set! *quiet* #t) (exit (run-program optarg restargs))]
        [(version)   (print-version!) (loop '() #f)]
        [(help)      (print-help!)    (loop '() #f)]
        [(#f) (cond [(pair? restargs)
                     (set! *quiet* #t) (exit (run-script (car restargs) (cdr restargs)))]
                    [(not repl?) (exit #t)])]))))
```

Three things in this driver are worth copying:

- **The `(#f)` clause is where the operands are handled.** Reaching it means
  options are exhausted; `restargs` then holds the script name and everything
  after it. `run-script` gets `(car restargs)` as the file and `(cdr restargs)` as
  the script's own arguments — which is precisely why the parser must not permute,
  since a script's arguments may themselves look like options.
- **Terminating options loop with `'()`.** `--version` and `--help` recurse with
  an empty argument list rather than returning directly, so control still lands in
  the `(#f)` clause and exits along one path.
- **`-s` and `-p` never come back.** They hand `restargs` straight to the script or
  program and `exit` with its status.

## A complete program

```scheme
(import (scheme base) (scheme write) (scheme process-context) (skint getopt))

(define *options*
 '([verbose "-v" "--verbose" #f     "Say more about what is happening"]
   [output  "-o" "--output"  "FILE" "Write output to FILE instead of stdout"]
   [help    "-h" "--help"    #f     "Display this help"]))

(define (usage)
  (display "usage: demo [OPTION]... [FILE]...\n\nOptions:\n")
  (print-command-line-options *options* (current-output-port)))

(define (main args)
  (let loop ([args args] [verbose? #f] [outfile #f])
    (get-next-command-line-option args *options*
      (lambda (keysym optarg restargs)
        (case keysym
          [(verbose) (loop restargs #t outfile)]
          [(output)  (loop restargs verbose? optarg)]
          [(help)    (usage) (exit 0)]
          [(#f)      (when verbose?
                       (display "output: ") (write outfile) (newline))
                     (display "files: ") (write restargs) (newline)])))))

(main (cdr (command-line)))
```

```
$ skint demo.scm -v --output=out.txt a.scm b.scm
output: "out.txt"
files: ("a.scm" "b.scm")
```

## Limitations

- No optional option arguments — an option either always takes one or never does.
- No permutation of options past the first operand (by design; see above).
- No automatic `--help` / `--version` handling; wire those up yourself, as SKINT's
  own driver does.
- Repeat counting (`-vvv`), mutually exclusive groups, and argument type
  conversion are all the caller's business. Clustering makes `-vvv` parse as three
  `verbose` keysyms, so counting is a matter of incrementing in your own loop.
