# `(skint ivset)` — integer interval sets

Sets of exact integers represented as sorted vectors of disjoint, non-touching
closed intervals. The representation is compact and the set operations cost time
proportional to the number of *intervals*, not the number of *elements*, which
makes it practical to manipulate sets spanning the whole Unicode code point range.

Because the intervals are held sorted in a flat vector, membership testing is a
**binary search: O(log N), where N is the number of intervals** — independent of
how many integers the set actually contains. A set covering all 1114112 Unicode
code points in a handful of intervals is searched in a handful of steps.

This library backs SKINT's implementations of SRFI 14 (character sets), SRFI 115
(regular expressions) and SRFI 252, where the alternative — bitmaps or hash sets
over `#x110000` code points — would be prohibitive. It is useful on its own
wherever you need dense integer sets, code point ranges, or coverage maps.

The library needs nothing but `(scheme base)`, so it is portable R7RS.

```scheme
(import (skint ivset))
```

## Representation

An **interval set** (*ivs*) is a vector of an even number of exact integers, read
as consecutive `first`/`last` pairs. Both endpoints are **inclusive**:

```scheme
#()                  ; the empty set
#(42 42)             ; {42}
#(1 3 7 8 20 20)     ; {1,2,3} ∪ {7,8} ∪ {20}
```

An **interval list** (*ivl*) is the same information as a list of pairs, which is
often more convenient to write and to pattern-match:

```scheme
((1 . 3) (7 . 8) (20 . 20))
```

### Canonical form

Every ivs produced by this library satisfies three invariants:

1. **Sorted** — intervals appear in increasing order.
2. **Valid** — in each interval, `first <= last`.
3. **Separated** — consecutive intervals are neither overlapping nor *touching*;
   there is always a gap of at least one integer between them. So `{1,2,3}` and
   `{4,5}` merge into the single interval `1..5`, never `(1 . 3) (4 . 5)`.

The third invariant is what makes `ivs=?` a simple `equal?` on vectors: two sets
with the same members always have the same representation.

Every procedure here that returns an interval set returns a canonical one, and
every procedure that takes one expects a canonical one.

## Conversions

### `(ivs->ivl ivs)` → *interval list*

Unpacks an ivs into a list of `(first . last)` pairs.

```scheme
(ivs->ivl #(1 3 7 8 20 20))    ; => ((1 . 3) (7 . 8) (20 . 20))
(ivs->ivl #())                 ; => ()
```

### `(ivl->ivs ivl)` → *ivs*

Packs a list of `(first . last)` pairs into an ivs. It is an error if `ivl` is not
in canonical form as described above.

```scheme
(ivl->ivs '((1 . 3) (7 . 8)))  ; => #(1 3 7 8)
(ivl->ivs '())                 ; => #()
```

This is the natural way to write a set literal spanning large ranges, and the
usual pairing with `ivs->ivl` for hand-written interval processing:

```scheme
;; keep only the intervals wider than one element
(define (wide-only ivs)
  (ivl->ivs
    (let loop ([ivl (ivs->ivl ivs)])
      (cond [(null? ivl) '()]
            [(> (cdar ivl) (caar ivl)) (cons (car ivl) (loop (cdr ivl)))]
            [else (loop (cdr ivl))]))))

(wide-only #(1 3 7 7 20 25))   ; => #(1 3 20 25)
```

Dropping intervals, as above, keeps the list canonical. Transforming endpoints
generally does not, so build such results with `list->ivs` or the set operations
instead.

### `(ivs->list ivs)` → *list of integers*

Every member, in increasing order. Cost is proportional to the *cardinality*, so
do not call this on a set covering a large range.

```scheme
(ivs->list #(1 3 7 8 20 20))   ; => (1 2 3 7 8 20)
```

### `(list->ivs list)` → *ivs*

Builds a canonical set from a list of exact integers. The input need not be
sorted or duplicate-free; runs of consecutive integers are coalesced.

```scheme
(list->ivs '(1 2 3 7 8 20))    ; => #(1 3 7 8 20 20)
(list->ivs '(5 1 3 2 1 5 4))   ; => #(1 5)
(list->ivs '(42))              ; => #(42 42)
(list->ivs '())                ; => #()
(list->ivs '(-5 -4 -3 0 1))    ; => #(-5 -3 0 1)
```

### `(predicate->ivs pred start end)` → *ivs*

Applies `pred` to each integer in the **half-open** range `[start, end)` and
returns the set of those for which it returned a true value.

```scheme
(predicate->ivs even? 0 10)            ; => #(0 0 2 2 4 4 6 6 8 8)
(predicate->ivs (lambda (x) #t) 0 5)   ; => #(0 4)
```

Note that `end` is *exclusive*, matching SRFI 14's `ucs-range->char-set` rather
than the inclusive convention used by the intervals themselves. Calling `pred`
once per integer in the range makes this a tool for building a table once at
startup, not something to call in a loop.

## Queries

### `(ivs-size ivs)` → *exact integer*

Cardinality — the number of members, not the number of intervals.

```scheme
(ivs-size #(1 3 7 8 20 20))    ; => 6
(ivs-size #())                 ; => 0
(ivs-size #(0 1114111))        ; => 1114112
```

### `(ivs-contains? ivs e)` → *boolean*

Membership test. Performs a **binary search** over the interval vector, so it
runs in **O(log N) time, where N is the number of intervals** — not the number of
members. A table of several hundred intervals — the scale of a Unicode character
class — settles in about ten comparisons, whatever its cardinality.

```scheme
(ivs-contains? #(7 8) 8)       ; => #t
(ivs-contains? #(7 8) 9)       ; => #f
```

This is the operation the representation is optimized for, and the reason a
character-class test is cheap enough to put in an inner loop.

### `(ivs=? ivs1 ivs2)` → *boolean*

Set equality. Canonical form is unique, so two sets with the same members are
always represented the same way.

```scheme
(ivs=? (list->ivs '(1 2 3)) (ivl->ivs '((1 . 3))))   ; => #t
```

### `(ivs<=? ivs1 ivs2)` → *boolean*

Subset test: true when every member of `ivs1` is a member of `ivs2`.

```scheme
(ivs<=? (list->ivs '(2 3)) (list->ivs '(1 2 3 4)))   ; => #t
(ivs<=? (list->ivs '(2 9)) (list->ivs '(1 2 3 4)))   ; => #f
```

### `(ivs-disjoint? ivs1 ivs2)` → *boolean*

True when the two sets share no members.

```scheme
(ivs-disjoint? (list->ivs '(1 2)) (list->ivs '(5 6)))   ; => #t
(ivs-disjoint? (list->ivs '(1 2)) (list->ivs '(2 6)))   ; => #f
```

## Set operations

All three take two ivs arguments and return a fresh canonical ivs. They work on
the interval structure, so cost scales with the number of intervals rather than
the number of members.

### `(ivs-union ivs1 ivs2)` → *ivs*

```scheme
(ivs-union (list->ivs '(1 2 3)) (list->ivs '(4 5)))   ; => #(1 5)
(ivs-union (list->ivs '(1 2 3)) (list->ivs '(5 6)))   ; => #(1 3 5 6)
```

The first case shows the separation invariant at work: `1..3` and `4..5` touch,
so they are merged into one interval.

### `(ivs-intersection ivs1 ivs2)` → *ivs*

```scheme
(ivs-intersection (list->ivs '(1 2 3 4)) (list->ivs '(3 4 5)))   ; => #(3 4)
```

### `(ivs-difference ivs1 ivs2)` → *ivs*

Members of `ivs1` that are not members of `ivs2`.

```scheme
(ivs-difference (list->ivs '(1 2 3 4 5)) (list->ivs '(3)))       ; => #(1 2 4 5)
```

## Iteration

### `(ivs-for-each proc ivs)` → *unspecified*

Applies `proc` to each member in increasing order. Like `ivs->list`, this visits
every *element*, so the cost is the cardinality.

```scheme
(let ([acc '()])
  (ivs-for-each (lambda (e) (set! acc (cons e acc))) (list->ivs '(1 2 7)))
  (reverse acc))                ; => (1 2 7)
```

To iterate over *intervals* instead — usually what you want for large sets — go
through `ivs->ivl`:

```scheme
(for-each (lambda (iv) (report (car iv) (cdr iv))) (ivs->ivl big-set))
```

## Idioms

### Complement

There is no complement operation, because a set has no inherent universe.
Subtract from an explicit one:

```scheme
(define universe (ivl->ivs '((0 . 255))))
(ivs->ivl (ivs-difference universe (ivl->ivs '((10 . 20)))))
; => ((0 . 9) (21 . 255))
```

A universe need not be one interval: a set of all valid Unicode code points, for
instance, excludes the surrogate range and the non-characters, and so has several.

### Adding or removing single elements

There is no `ivs-adjoin` / `ivs-delete`; compose from a singleton:

```scheme
(define (ivs-adjoin ivs e) (ivs-union ivs (ivl->ivs (list (cons e e)))))
(define (ivs-delete ivs e) (ivs-difference ivs (ivl->ivs (list (cons e e)))))
```

For adding several elements at once, `(ivs-union ivs (list->ivs elements))` is
better than repeated adjoining.

### Literal tables

Because an ivs is an ordinary vector of integers, generated tables can be
embedded directly as self-evaluating literals — no constructor call, no
initialization cost:

```scheme
(define ivs:hangul-l #(#x1100 #x115f #xa960 #xa97c))
```

This is the practical way to ship a large character-class table: compute it once
with `predicate->ivs`, write the resulting vector out, and paste the literal into
your source.

## Summary

| Procedure | Result | Cost scales with |
|---|---|---|
| `(ivs->ivl ivs)` | interval list | intervals |
| `(ivl->ivs ivl)` | ivs (`ivl` must be canonical) | intervals |
| `(ivs->list ivs)` | list of integers | **members** |
| `(list->ivs list)` | ivs | length of list |
| `(predicate->ivs pred start end)` | ivs | `end - start` |
| `(ivs-size ivs)` | exact integer | intervals |
| `(ivs-contains? ivs e)` | boolean | **O(log N)** — binary search |
| `(ivs=? ivs1 ivs2)` | boolean | intervals |
| `(ivs<=? ivs1 ivs2)` | boolean | intervals |
| `(ivs-disjoint? ivs1 ivs2)` | boolean | intervals |
| `(ivs-union ivs1 ivs2)` | ivs | intervals |
| `(ivs-intersection ivs1 ivs2)` | ivs | intervals |
| `(ivs-difference ivs1 ivs2)` | ivs | intervals |
| `(ivs-for-each proc ivs)` | unspecified | **members** |

Throughout, *N* is the number of intervals in the set, not its cardinality.
`ivs-contains?` is the standout at O(log N); `ivs->list` and `ivs-for-each` are
the only operations that touch every member, and so are the only ones whose cost
depends on cardinality. Everything else stays in interval space.
