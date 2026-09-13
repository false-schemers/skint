## Tests

Three drivers sit here, each gathering a different body of tests. They are plain
Scheme programs: the interpreter runs them, nothing else is involved.

| Driver | Gathers | Make target |
|---|---|---|
| `test.scm` | the core R7RS and SKINT suite, written inline | `make test` |
| `test-libraries.scm` | everything under `skint/` and `srfi/` | `make libtest` |
| `test-options.scm` | the suites that only apply to optional builds | `make opttest` |

`test.scm` is self-contained — R7RS conformance and SKINT's own behaviour, collected
from several sources — and needs no libraries at all. It reports a single pass count.

`test-libraries.scm` walks the subdirectories, running each `all.scm` in turn, and
reports a cumulative failure count per group. `skint/` covers the `(skint ...)`
libraries documented in [../doc/skint/](../doc/skint/README.md), one file per library;
`srfi/` covers the supported SRFIs, one file per SRFI number.

`test-options.scm` runs the suites under `opt/` — the numeric tower and Unicode —
each only if the interpreter was built with the corresponding option. On a build
without them it reports that it tested nothing, which is a pass, not a skip to worry
about.

### Running them from make

The targets above run against the interpreter just built in the source directory, so
use them after `make` and before `make install`:

```
make
make test
make libtest
make opttest
make install
```

Nothing here depends on an installed interpreter, and running the tests after
installing would exercise the installed library directory rather than this one.

### Running a test file directly

Any single file can be run on its own, which is what you want while working on one
library:

```
./skint test/skint/fx.scm
./skint test/srfi/1.scm
```

Run these *from the root of the source tree*. SKINT's default library search path is
`.:lib:...` — the current directory, then `lib`, then the built-in directory — and
those first two entries are relative to wherever you are. From the root, `lib/` is
found and the test's imports resolve.

From anywhere else they do not, and the failure says so:

```
Error:
library not found (skint fx)
```

Point the interpreter at the right directory when that happens. Note it is `lib` you
must name, not the tree root:

```
cd test
../skint -I ../lib skint/fx.scm      # -I prepends, -A appends
```

or set the search path wholesale, which replaces the default rather than adding to
it:

```
SKINT_LIBDIRS="..:../lib:..." ../skint skint/fx.scm
```

`...` in either form stands for the interpreter's built-in library directory, so keep
it in the list unless you mean to exclude the installed libraries.

### Adding a test

A new library test goes in `skint/` named after the library, and its name is added to
the list in `skint/all.scm`. A SRFI test goes in `srfi/` named after the number, and
into `srfi/all.scm`. Both directories have a `test.scm` holding the shared harness,
which each file includes; core-suite additions go directly into `test.scm` here.
