# Syntax-rules extensions

SKINT implements all standard features of R7RS `syntax-rules`, including custom ellipsis, non-final ellipsis patterns, non-binding underscore pattern, and `(... tpl)` template escapes. It also has the following incompatibilities and extensions:

## Keyword at the beginning of the pattern

Current implementation is incompatible with R7RS in one aspect: the keyword at the beginning of the pattern in a `<syntax rule>` *is* considered a pattern variable (please see below for why it is important). In the future, we will allow this behavior only through simple pattern escape mechanism.

## Support for boxes

Boxes, as defined by SRFI-111 and the future `(scheme box)` library, are supported natively, and can be parts of both patterns and templates. See examples of their use below. 

## Simple pattern escape

A pattern of the form `(<ellipsis> <pattern>)` where `<ellipsis>` is the current ellipsis is interpreted as if it were `<pattern>`, but ellipses and underscores in `<pattern>` lose their special meaning, e.g.:

```scheme
(define-syntax underscored
  (syntax-rules ()
    [(_ (... _) (... ...)) (list (... ...) (... _))]))

(underscored 1 2)
; => (2 1) 
```

This feature may be useful in macro-generators to make sure that any identifier can play a role of a pattern variable.

## Named pattern escapes

A pattern of the form `(<ellipsis> <predicate name> <pattern>)` where `<ellipsis>` is the current ellipsis is interpreted as if it were `<pattern>`, with additional constraint that the S-expression it matches should also satisfy the constraint specified by `<predicate name>`. Predicate names are compared to predefined symbols according to `free-identifier=?` rules. The following named pattern escapes are supported:

  * `(... number? <pattern>)`
  * `(... exact-integer? <pattern>)`
  * `(... boolean? <pattern>)`
  * `(... char? <pattern>)`
  * `(... string? <pattern>)`
  * `(... bytevector? <pattern>)`
  * `(... id? <pattern>)`

All but the last predicate have the same meaning as the corresponding Scheme procedures. The `id?` predicate checks if the corresponding S-expression is either a symbol or a syntax object representing an identifier.

The rationale for adding these escapes is obvious: while `syntax-rules`-based macros can perform very complex calculations with structured S-expressions, they lack an ability to deal with *atomic* S-expressions (with the exception of identifiers – they can be recognized, but the technique for that is quite complicated).

Example (also uses box templates):

```scheme
(define-syntax wrap-by-type
  (syntax-rules ()
    [(_ (... string? x)) '#&x]
    [(_ (... number? x)) '#(x)]
    [(_ x) 'x]))

(list (wrap-by-type 42) (wrap-by-type "yes") (wrap-by-type #\c))) 
; => (#(42) #&"yes" #\c)
```

## Named template escapes

A template of the form `(<ellipsis> <converter name> <template+>)` where `<ellipsis>` is the current ellipsis is interpreted as follows. First, `<template+>` (which can be any nonempty sequence of `<template>`s), is instantiated recursively, resulting in a list of S-expressions. These S-expressions become arguments to a converter specified by `<converter name>`. It is a syntax error to apply converters to a wrong type or number of arguments. Converter names are compared to predefined symbols according to `free-identifier=?` rules.  The following named template escapes are supported:

  * `(... number->string <template>)`
  * `(... string->number <template>)`
  * `(... list->string <template>)`
  * `(... string->list <template>)`
  * `(... list->bytevector <template>)`
  * `(... bytevector->list <template>)`
  * `(... length <template>)`
  * `(... make-list <template> <template>)`
  * `(... char<=? <template+>)`
  * `(... <= <template+>)`
  * `(... + <template+>)`
  * `(... - <template+>)`
  * `(... id->string <template>)`
  * `(... string->id <template>)`
  * `(... string->id <template> <id template>)`

All but the last three converters have the same meaning as the corresponding Scheme procedures. The `id->string` converter expects either a symbol or a syntax object representing an identifier and produces a string containing a “quote name” of the identifier (the result of applying `symbol->string` to the original name supplied by the user after all substitutions).

The `string->id` converter allows one to produce identifiers having the same syntax properties as identifiers explicitly introduced as part of macro definitions or macro uses. In two-argument case, the properies are copied from `<id template>`, which, after all substitutions are performed, should instantiate to an identifier serving as a prototype. If it is not provided, the `string->id` identifier itself is used as `<id template>`. The `<template>` argument should instantiate into a string, which is then converted to a symbol via `string->symbol` and then turned into an identifier syntax object *as if* it was introduced side-by-side with the prototype identifier (same expression, same expansion phase).

Examples:

```scheme
(define-syntax with-math-defines 
  (syntax-rules () 
    [(_ x) 
     ((lambda ((... string->id "pi") (... string->id "e")) x) 
      3.14 2.72)]))

(with-math-defines (+ pi e))     
; => 5.86 
```

```scheme
(let-syntax 
  ([define-math-constants
    (syntax-rules () 
      [(ref-id)
       (begin (define (... string->id "pi" ref-id) 3.14)
              (define (... string->id "e" ref-id) 2.72))])])
  (define-math-constants)
  (+ pi e))
; => 5.86 
```  

Note that in the last example the keyword `ref-id` at the beginning of the pattern was used to bring in the `define-math-constants` from the macro use to serve as a prototype id for introduced `pi` and `e` identifiers, allowing them to capture the corresponding identifier names typed in by the user in `(+ pi e)`. Since this keyword, according to R7RS rules, should not be treated as a pattern variable, in the future wi will require using simple pattern escape instead.

Here are some more examples:

```scheme
(define-syntax loop-until-break
  (syntax-rules ()
    [(ref-id e ...)
     (call/cc 
       (lambda ((... string->id "break" ref-id))
         (let loop () e ... (loop))))]))

(let ([n 10] [steps 0] [break write])
  (loop-until-break
    ; break here refers to the escape continuation
    (when (= n 4) (break steps))
    (set! n (- n 1))
    (set! steps (+ steps 1))))
; => 6
```

To demonstrate combined use of different converters, here is a thin macro layer over tagged vectors:

```scheme
(define-syntax define-variant
  (syntax-rules ()
    [(define-variant name () ([field0 index0] ...))
     (begin
       (define-syntax name
         (lambda (field0 ...)
           (vector 'name field0 ...)))
       (define-syntax 
         (... string->id 
           (... string-append (... id->string name) "?") define-variant)
         (lambda (object)
           (and (vector? object)
             (= (vector-length object) (... length (name field0 ...)))
             (eq? (vector-ref object 0) 'name))))
       (define-syntax 
         (... string->id 
           (... string-append (... id->string name) "->" 
             (... id->string field0)) define-variant)
         (lambda (object)
           (vector-ref object index0)))
       ...)]
    [(define-variant name (field0 field ...) (pair ...))
     (define-variant name (field ...)  
       (pair ... [field0 (... length (name pair ...))]))]
    [(define-variant name (field0 ...))
     (define-variant name (field0 ...) ())]))

(define-syntax variant-case
  (syntax-rules (else)
    [(variant-case (a . d) clause ...)
     (let ([var (a . d)]) (variant-case var clause ...))]
    [(variant-case var) (error "no variant-case clause matches" var)]
    [(variant-case var (else exp1 exp2 ...)) (body exp1 exp2 ...)]
    [(variant-case var [name (field ...) exp1 exp2 ...] clause ...)
     (if ((... string->id (... string-append (... id->string name) "?") variant-case) var)
         (let ([field ((... string->id (... string-append (... id->string name) "->" 
                         (... id->string field)) variant-case) 
                       var)] ...)
           exp1 exp2 ...)
         (variant-case var clause ...))]))

(let ()
  (define-variant point (x y))
  (define-variant kons (a d))
  (define-syntax pair->a car)
  (define-syntax pair->d cdr)
  (define (println v)
    (variant-case v ; nb: pair? is already defined
      [point (x y) (format #t "[point x=~s y=~s]~%" x y)]
      [kons (a d) (format #t "[kons a=~s d=~s]~%" a d)]
      [pair (a d) (format #t "[pair a=~s d=~s]~%" a d)]
      [else (format #t "[unknown ~s]~%" v)]))
  (define p (point 1 4))
  (define k (kons 'a 'd))
  (define c (cons 'a 'd))
  (format #t "p = ~s k = ~s c = ~s~%" p k c)
  (println p)
  (println k)
  (println c))

; prints:
; p = #(point 1 4) k = #(kons a d) c = (a . d)
; [point x=1 y=4]
; [kons a=a d=d]
; [pair a=a d=d]
```

## Why stop here?

The above collection of named escapes is selected as *almost* minimal one. Its purpose is not make `syntax-rules`-based macro programming more convenient, but just extend it to non-structural S-expressions, so it is possible to recognize them and work with them by converting them to another form if a need arises. Arithmetics is limited to what one can do using lists as Peano numbers; for chars, access to ordering is provided, to support simple char ranges. One can imitate `string-append` converter without a dedicated converter, but this unnecessarily complicates generation of identifiers, which is a major use case.


