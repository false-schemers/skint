# SKINT - Cheap and Fast R7RS Scheme Interpreter
                         
SKINT is a portable interpreter for the R7RS Scheme programming language. 
It can be built from five C source files with a single command. There is no distributives or packages: 
just compile the source files with your favorite C compiler, link them with the standard C runtime libraries 
and be done with it. For some platforms, precompiled binaries are available (please see [releases](https://github.com/false-schemers/skint/releases)).

## Installation

Here's how you can compile SKINT on a unix box using GCC:

```
gcc -o skint [skint].c -lm
```

Some compilers link `<math.h>` library automatically, some require explicit option like `-lm` above. It can be built on 32-bit 
and 64-bit systems (tested on Windows and Linux).

For much better performance (especially in floating-point calculations), you may add optimization options, e.g.:  

```
gcc -o skint -O3 -DNDEBUG -DNAN_BOXING [skint].c -lm
```

NAN_BOXING option assumes that upper 16 bit of heap pointers are zero (48-bit address space). If this assumption holds,
it is recommended to use it on 64-bit systems.

The resulting interpreter has no dependencies (except C runtime and standard -lm math library) and can be run from any location.
If compiled statically, it can be easily moved between systems with the same ABI.


## Scheme Compatibility

SKINT is true to basic Scheme principles -- it features precise garbage collector, supports proper tail recursion, `call/cc`, 
`dynamic-wind`, multiple return values, and has a hygienic macro system and a library system. It is almost fully compatible 
with R7RS-small, but it has the following known limitations and deviations from the standard:

  *  fixnums are 30 bit long, flonums are doubles
  *  no support for bignums/rational/complex numbers
  *  no support for Unicode; strings are 8-bit clean, use system locale
  *  source code literals cannot be circular (R7RS allows this)
  
Some features of the R7RS-Small standard are not yet implemented or implemented in a simplified or non-conforming way:

  *  `read` procedure is always case-sensitive (all ports operate in no-fold-case mode)
  *  `#!fold-case` and `#!no-fold-case` directives have no effect 
  *  `include` and `include-ci` forms work in case-sensitive mode
  *  `current-jiffy` and `jiffies-per-second` return inexact integers
  *  `current-second` is defined as C `difftime(time(0), 0)+37`

Here are some details on SKINT's interactive Read-Eval-Print-Loop (REPL) and evaluation/libraries support:

  *  `read` supports R7RS notation for circular structures, but both `eval` and `load` reject them
  *  all supported R7RS-small forms are available in the built-in `(skint)` library and REPL environment
  *  `-I` and `-A` command-line options extend library search path; initial path is `./`
  *  `cond-expand` checks against `(features)` and available libraries
  *  `environment` dynamically fetches library definitions from `.sld` files
  *  both `eval` and `load` accept optional environment argument
  *  command-line options can be shown by running `skint --help` 
  *  both `import` and `define-library` forms can be entered interactively into REPL
  *  REPL supports single-line `comma-commands` — type `,help` for a full list 
  
Please note that SKINT's interaction environment exposes bindings for all supported R7RS-small procedures 
and syntax forms directly, so there is no need to use `import`. All R7RS-small libraries are built-in and
do not rely on any external .sld files.

## Origins

Parts of SKINT's run-time system and startup code are written in [#F](https://github.com/false-schemers/sharpF), 
a language for building Scheme-like systems. Its #F source code can be found there in `precursors` directory:

[skint/pre](https://raw.githubusercontent.com/false-schemers/skint/master/pre)

SKINT's hygienic macroexpander is derived from Al Petrofsky's EIOD (please see the t.scm file for the original copyright notice).
SKINT's VM and compiler follow the stack machine approach described in "Three Implementation Models for Scheme" thesis by R. Kent Dybvig
([TR87-011](https://www.cs.unc.edu/techreports/87-011.pdf), 1987).
Supporting library code comes from #F's [LibL library](https://raw.githubusercontent.com/false-schemers/sharpF/master/lib/libl.sf).

## Family

Please see [SIOF](https://github.com/false-schemers/siof) repository for a single-file R7RS-small interpreter. It is
more portable and easier to build, but runs significantly slower.