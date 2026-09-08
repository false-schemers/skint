## Bytecode: an intermediate language, not an execution format

SKINT has a bytecode, and no bytecode interpreter. Nothing ever executes it. It is
the compiler's output format and nothing more: a compact, printable, serializable
encoding that an internal converter turns into the threaded code described in
[vm.md](vm.md) before anything runs.

That is what lets it be simple and short. A bytecode designed to be interpreted has
to trade decoding cost against density; this one has no decoding cost to pay,
because decoding happens once. What it buys instead is that a compiled program is an
ordinary C string — which is exactly how the standard library ships. `s.c` and `t.c`
are nothing but tables of these strings baked into the binary.

The producer is `codegen` in `pre/t.scm`. The consumer is `rds_seq` and its
neighbours in `i.c`. The table both sides agree on is `i.h`.

### The encoding

Bytecode is printable ASCII. An instruction is one to four characters; operands
follow immediately, with no separator.

An *operand* is either a single decimal digit, which denotes that small non-negative
integer, or a parenthesised serialized datum:

```scheme
(define (write-serialized-arg arg port)
  (if (and (number? arg) (fixnum? arg) (fx<=? 0 arg) (fx<=? arg 9))
      (write-char (string-ref "0123456789" arg) port)
      (begin (write-char #\( port)
             (write-serialized-sexp arg port)
             (write-char #\) port))))
```

The digit case is not an optimisation detail — it is what makes instruction fusion
possible, as the next section explains.

A *block* is a sequence of instructions between `{` and `}`. Blocks are how the
encoding expresses code that is jumped over rather than fallen into: the arms of a
branch, the body a `save` returns past, the body of a closure.

A *serialized datum* inside an operand uses a small self-describing grammar, written
by `write-serialized-sexp` and read by `rds_sexp`:

| Prefix | Datum |
|---|---|
| `f` `t` `n` | `#f`, `#t`, `()` |
| `c` | character, followed by one encoded character |
| `i` | fixnum, in decimal |
| `j` | flonum |
| `x` | other number, in hex |
| `y<n>:` | symbol of `n` characters |
| `s<n>:` | string of `n` characters |
| `l<n>:` | list of `n` elements |
| `p` | pair — two elements |
| `v<n>:` | vector of `n` elements |
| `b<n>:` / `h<n>:<t>:` | bytevector / typed numeric vector |
| `z` | box, one element |

Each element of a compound datum is terminated by `;`. Anything below space or above
`~`, together with the three characters `%`, `"` and `\`, is escaped as `%XX`,
`%uXXXX` or `%UXXXXXXXX`, which is what keeps the whole thing embeddable in a C
string literal.

A worked example, the code `k.c` hands to the decoder to start the read-eval-print
loop:

```
${@(y4:repl)[00}
```

`$` is `save`, whose operand is a block; the block is `@(y4:repl)[00`, and `save`
records the point just past it as where to resume. Inside, `@` is `gref` with the
operand `(y4:repl)` — the serialized symbol `repl` — and `[00` is `call0`. So:
push a return point, fetch the global `repl`, call it with no arguments.

### The table

Every instruction is one line in `i.h`:

```c
declare_instruction(name, enc, etyp, igname, arity, lcode)
```

| Column | Meaning |
|---|---|
| `name` | C identifier; the instruction function is `cxi_<name>` and its global `cx_ins_<name>` |
| `enc` | the bytecode encoding, or `NULL` for an instruction the compiler never emits |
| `etyp` | what operands the decoder must read (below) |
| `igname` | the Scheme name, if this instruction is also an integrable |
| `arity` | the integrable's argument-count class |
| `lcode` | how to build the integrable's global procedure |

The file is an X-macro table, included several times with different definitions of
`declare_instruction`:

- with `VM_GEN_DEFGLOBAL`, to define `cx_ins_<name>` for every instruction;
- with `VM_GEN_ENCTABLE`, to build `enctab`, the array the decoder's trie is built
  from;
- with `VM_GEN_INTGTABLE`, to build `intgtab`, the integrables table;
- with none of them, to emit `extern` declarations for everything.

There are 591 `declare_instruction` lines and 35 `declare_integrable` lines — the
latter for integrables that reuse another instruction's encoding, or that are
compiled inline with no instruction of their own.

Two instructions have `enc` of `NULL` and so never appear in bytecode at all.
`halt` is appended by the decoder when it reaches the end of a stream, and `br` is
synthesized by the decoder when it sees a two-armed branch.

#### Operand types

The `etyp` column tells the decoder what to consume after the instruction's
characters, and it must match what the instruction function reads with `*ip++`.

| `etyp` | Decoder reads | Used by |
|---|---|---|
| `0` | nothing | most instructions |
| `1` | one operand | `lit`, `sref`, `dref`, `sbox`, `call`, `sreturn`, `adrop`, `atest`, `lck`, … |
| `2` | two operands | `scall`, `jdceq`, `jdcge` |
| `'g'` | one operand, resolved to a global's box | `gref`, `gset`, `gloc` |
| `'a'` | one following zero-operand instruction, taken as an operand | `andbo` |
| `'s'` | a block; the operand becomes its length, and the block is spliced inline | `save` |
| `'d'` | one operand, then a block converted to a vector | `dclose` |
| `'b'` | a block, or two blocks — in which case a `br` is inserted between them | `brnot`, `brt` |

The `'g'` case is where a global name becomes a location. `rds_global_loc` hashes the
symbol into the `*globals*` vector and either finds the existing `(sym . #&sym)`
entry or prepends a new one, so the code vector ends up holding the box itself. A
global reference at run time is one indirection, with no lookup.

The decoder also has branches for `etyp` values `'f'`, `'t'` and `'n'`, which would
append an inline `#f`, `#t` or `()` operand. No instruction currently uses them.

### The decoder

`rds_stoc` turns a bytecode string into a code vector. The work is in `rds_seq`,
which loops over the stream, and `rds_prefix`, which identifies one instruction.

The encodings are held in a trie, built once by `encmap_add` from `enctab` and
cached in the static `encmap`. Each node carries the instruction global `g` (or 0 if
no instruction ends there) and its `etyp`. `rds_prefix` walks it:

```c
static struct embranch *rds_prefix(obj port)
{
  struct embranch *pbr = get_encmap(); int c;
  down: while ((c = iportpeekc(port)) != EOF) {
    struct emtrans *ptr = pbr->ptr;
    while (ptr != NULL) {
      if (ptr->c != c) { if (ptr->ptr == NULL) break; ptr = ptr->ptr; continue; }
      pbr = ptr->pbr; iportgetc(port);
      goto down;
    }
    break;
  }
  return pbr;
}
```

It consumes characters greedily for as long as a transition exists, and stops the
moment one does not. *It never backtracks.* If the walk ends on a node with no
instruction, the whole decode fails; `rds_seq` returns an end-of-file marker and
the caller reports a bad code string.

So: the longest sequence of characters that forms a valid trie path is always the
instruction chosen.

### Fusion

That rule is not a limitation to work around — it is the mechanism by which one
threaded-code instruction does the work of several bytecode instructions.

The compiler emits instructions independently. `codegen` writes `'` for a literal
and then its operand, writes `,` for a push, writes `]` for a return and then the
number of stack slots to drop. It knows nothing about fusion. But the encodings are
laid out so that the decoder's greedy walk collapses the common combinations:

| Emitted as | Decoded as | Effect |
|---|---|---|
| `'` + `0` | `lit0` | load the literal 0 |
| `'` + `0` + `,` | `pushlit0` | load 0 and push it |
| `.` + `3` | `sref3` | fetch stack slot 3 |
| `.` + `3` + `,` | `pushsref3` | fetch and push |
| `.` + `0` + `^` + `,` | `pushsrefi0` | fetch slot 0, unbox, push |
| `]` + `0` | `return` | return dropping nothing |
| `[` + `0` | `call` | `scall` with zero slots to shift |
| `[` + `0` + `0` | `call0` | call with no arguments |
| `[` + `2` + `3` | `scall23` | shift 2, call with 3 arguments |

The families are systematic. `sref` takes one operand and `sref0`…`sref9` are the
ten specialisations with that operand baked in; `scall` takes two and `[1`…`[4`
bake the first while `[10`…`[44` bake both. Each fused form is a single C function
that does in a few machine instructions what the pieces would have done in several
tail calls.

This is where the density comes from, and it costs the emitter nothing.

### The design rule that comes with it

Because the decoder takes the longest match and cannot backtrack, the encoding set
has an invariant that nothing in the build checks:

> If one instruction's encoding is a prefix of another's, the longer instruction
> must mean exactly what the shorter one followed by the remaining characters would
> have meant.

Both halves of that matter, and both are easy to violate.

*Extending by a digit takes an operand away.* An operand can begin with a digit, so
if `X` has `etyp` 1 and `X9` also exists, the stream `X9` will decode as `X9` and
never as `X` with operand 9. That is intentional for `sref`/`sref9` and for
`sreturn`/`sreturn4`, where the two readings agree. It would be a silent
miscompilation for any pair where they do not.

*Extending by anything else takes the next instruction away.* After a zero-operand
instruction the next instruction starts immediately, so if `X` has `etyp` 0 and
`X,` exists, the stream `X,` decodes as the single fused instruction. Again correct
for `sref0`/`pushsref0`, and again silent if the two differ.

*The characters that are safe for a genuinely new instruction* are those that can
neither start an operand nor start an instruction that would legitimately follow.
Operands begin with a digit or `(`, so the convention in the table is to extend with
`!`, `^` or a letter: `_` is `adrop` and `_!` is `pop`; `%` is `atest` and `%!` is
`shrarg`, `%%` is `aerr`, and `%p`, `%l`, `%v` … are the type checks. None of those
suffixes can begin an operand, so no reading is ambiguous.

A useful way to check a proposed encoding: write down every stream in which the
shorter form is followed by a legal operand or a legal next instruction, and confirm
the longer form does the same thing to each. If it does not, pick different
characters.

One instruction needs a note of its own. `andbo` (`;`, `etyp` `'a'`) explicitly
consumes the *following* instruction as its operand, and requires that instruction to
have `etyp` 0. That is how comparison chains short-circuit: `codegen` emits `;`
between the repeated comparison operations for `fx<?` and friends.

### Integrables

An integrable is a Scheme procedure the compiler can compile to an instruction
rather than a call. The last three columns of the table describe that.

`igname` is the Scheme name; a `NULL` means the instruction is VM machinery with no
Scheme-level equivalent. `arity` is the argument-count class, checked by
`integrable-argc-match?` in `pre/t.scm` before an application is turned into an
`integrable` core form:

| Class | Arguments | Class | Arguments |
|---|---|---|---|
| `0` `1` `2` `3` | exactly that many | `p` | zero or more, folded |
| `u` | zero or one | `m` | one or more, folded |
| `b` | one or two | `c` | two or more, chained comparison |
| `t` | two or three | `x` | one or more, folded (min/max shape) |
| `#` | zero or more, `lcode` required | `@` | never inlined, `lcode` required |

If the argument count does not match, the application falls back to an ordinary call
of the global procedure — nothing is rejected, it is only compiled differently.

Classes `4` and `5` also appear, on `fxfmar`, `inexact->string` and
`%port-location`, and `rds_intgtab` has templates for them. But
`integrable-argc-match?` does not list them, so its `else` clause answers `#f` and
applications of those three always compile to an ordinary call of the synthesized
global rather than to the instruction.

#### Alternate encodings

The `enc` field can hold more than one encoding, separated by NUL bytes. Only the
first is put in the decoder's trie, because `encmap_add` stops at the NUL;
`integrable_code(pi, n)` selects field `n`, and the compiler uses the later fields
to supply what an omitted argument would have meant:

```c
declare_instruction(iadd,  "I+\0'0",    0, "fx+",     'p', AUTOGL)
declare_instruction(isub,  "I-\0I-!",   0, "fx-",     'm', AUTOGL)
declare_instruction(rd8,   "R3\0Pi",    0, "read-u8", 'u', AUTOGL)
declare_instruction(wrc,   "W0\0Po",    0, "write-char", 'b', AUTOGL)
```

`(fx+)` compiles to field 1, `'0` — the literal zero. `(fx- x)` compiles to field 1,
`I-!` — negation. `(read-u8)` compiles to field 1, `Pi`, which fetches the current
input port, and then the ordinary `R3`. `(write-char c)` fetches the current output
port with `Po` and proceeds as the two-argument form. The folding classes `p`, `m`
and `x` emit the arguments and then repeat field 0 once per extra argument, so
`(fx+ a b c d)` is three `I+` instructions in a row.

#### Global procedures for free

Every integrable also has to exist as a first-class procedure, because it can be
passed to `map`. Rather than write those by hand, `rds_intgtab` synthesizes each one
*as bytecode* from the instruction's own encoding, at startup:

```c
case '1': sprintf(lbuf, "%%1_!%s]0", pe->enc); break;
```

For `car`, whose encoding is `a`, that yields `%1_!a]0` — check for exactly one
argument, pop it into the accumulator, take the car, return. The same is done for
each arity class, with rather more elaborate templates for the folding and
comparison classes, which have to loop over a rest argument.

The `lcode` column overrides this. `AUTOGL` (which is `NULL` unless the including
file redefines it) means "use the template". `INLINED` (the empty string) means the
integrable has no global procedure at all — the type checks `%ckp`, `%ckl` and their
kin are only ever compiled inline. And an explicit string is used verbatim, which is
how the three entries at the bottom of the table are defined with no instruction of
their own:

```c
declare_integrable(NULL, NULL, 0, "apply-to-list",    '@', "%2_!K3")
declare_integrable(NULL, NULL, 0, "call-with-values", '@', "%2_!K4")
declare_integrable(NULL, NULL, 0, "values",           '@', "K6")
```

### Changing the instruction set

The dependencies run in one direction, which makes the order of work clear.

1. Add the `declare_instruction` line to `i.h`, choosing an encoding against the
   longest-match rule above.
2. Write `define_instruction(name)` in `i.c`, reading exactly as many operands with
   `*ip++` as the `etyp` column promises.
3. If the compiler should emit it, teach `codegen` in `pre/t.scm` to write those
   characters. If it is an integrable, the `igname`, `arity` and `lcode` columns are
   enough — the expander finds it through `lookup-integrable`.
4. Regenerate `t.c` if `pre/t.scm` changed.

Removing or renaming an encoding is the riskier direction, because the bytecode
strings already baked into `s.c` and `t.c` were produced against the old table. They
must be regenerated together, which needs a working `skint` — see the note on the
bootstrap in `pre/`.
