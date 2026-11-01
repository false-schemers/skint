;;; r5rs.sld -- (scheme r5rs) as a real library, for compiled programs.
;;;
;;; Shadow counterpart of Skint's predefined (scheme r5rs): the predefined
;;; library exports bare bindings to Skint-initialized globals, which a
;;; standalone executable does not have.  Here every procedure name is
;;; either
;;;   * a cover macro: a call in matching position expands to the Skint
;;;     builtin (imported renamed as %builtin-<name>), a bare identifier
;;;     expands to %residual-<name>, an ordinary definition in this
;;;     library (so the compiled program owns a real closure); or
;;;   * a definition copied from Skint's pre/s.scm (the macro/procedure
;;;     library the interpreter itself is built from), adapted only to
;;;     refer to this library's own bindings; or
;;;   * core syntax / an auxiliary identifier re-exported unchanged --
;;;     expansion-time machinery, not a runtime global.
;;; Only builtin denotations are taken from Skint, via (skint) and, for
;;; %-prefixed builtin operations (skint) does not export, (skint hidden).
;;; Composition library: re-exports the constituent shadow
;;; libraries' bindings unchanged.  exact->inexact and
;;; inexact->exact are the (scheme base) inexact/exact bindings
;;; under their R5RS names.  eval and the environment
;;; constructors are NOT provided: (scheme eval) is out of
;;; scope for the compiler, and a compiled program has no
;;; interaction/report environments to construct; the names
;;; below fail loudly if called.

(define-library (scheme r5rs)
  (import
          (only (scheme base)
            zero?
            write-char
            vector?
            vector-set!
            vector-ref
            vector-length
            vector-fill!
            vector->list
            vector
            values
            unquote-splicing
            unquote
            truncate
            syntax-rules
            symbol?
            symbol->string
            substring
            string?
            string>?
            string>=?
            string=?
            string<?
            string<=?
            string-set!
            string-ref
            string-length
            string-fill!
            string-copy
            string-append
            string->symbol
            string->number
            string->list
            string
            set-cdr!
            set-car!
            set!
            round
            reverse
            remainder
            real?
            read-char
            rationalize
            rational?
            quotient
            quote
            quasiquote
            procedure?
            positive?
            peek-char
            pair?
            output-port?
            or
            odd?
            numerator
            number?
            number->string
            null?
            not
            newline
            negative?
            modulo
            min
            memv
            memq
            member
            max
            map
            make-vector
            make-string
            list?
            list-tail
            list-ref
            list->vector
            list->string
            list
            letrec-syntax
            letrec
            let-syntax
            let*
            let
            length
            lcm
            lambda
            integer?
            integer->char
            input-port?
            inexact?
            if
            gcd
            for-each
            floor
            expt
            exact?
            even?
            eqv?
            equal?
            eq?
            eof-object?
            else
            dynamic-wind
            do
            denominator
            define-syntax
            define
            current-output-port
            current-input-port
            cons
            cond
            complex?
            close-output-port
            close-input-port
            char>?
            char>=?
            char=?
            char<?
            char<=?
            char-ready?
            char->integer
            ceiling
            cdr
            cddr
            cdar
            case
            car
            call-with-values
            call-with-current-continuation
            cadr
            caar
            boolean?
            begin
            assv
            assq
            assoc
            apply
            append
            and
            abs
            >=
            >
            =>
            =
            <=
            <
            /
            ...
            -
            +
            *)
          (only (scheme cxr)
            cddddr
            cdddar
            cddadr
            cddaar
            cdaddr
            cdadar
            cdaadr
            cdaaar
            cadddr
            caddar
            cadadr
            cadaar
            caaddr
            caadar
            caaadr
            caaaar
            cdddr
            cddar
            cdadr
            cdaar
            caddr
            cadar
            caadr
            caaar)
          (only (scheme char)
            string-ci>?
            string-ci>=?
            string-ci=?
            string-ci<?
            string-ci<=?
            char-whitespace?
            char-upper-case?
            char-upcase
            char-numeric?
            char-lower-case?
            char-downcase
            char-ci>?
            char-ci>=?
            char-ci=?
            char-ci<?
            char-ci<=?
            char-alphabetic?)
          (only (scheme inexact)
            tan
            sqrt
            sin
            log
            exp
            cos
            atan
            asin
            acos)
          (only (scheme write)
            write
            display)
          (only (scheme file)
            with-output-to-file
            with-input-from-file
            open-output-file
            open-input-file
            call-with-output-file
            call-with-input-file)
          (only (scheme lazy)
            force
            delay)
          (rename (only (scheme base) inexact exact)
            (inexact exact->inexact) (exact inexact->exact))
          (only (scheme read) read)
          (only (scheme base) define error))
  (export
    zero?
    write-char
    vector?
    vector-set!
    vector-ref
    vector-length
    vector-fill!
    vector->list
    vector
    values
    unquote-splicing
    unquote
    truncate
    syntax-rules
    symbol?
    symbol->string
    substring
    string?
    string>?
    string>=?
    string=?
    string<?
    string<=?
    string-set!
    string-ref
    string-length
    string-fill!
    string-copy
    string-append
    string->symbol
    string->number
    string->list
    string
    set-cdr!
    set-car!
    set!
    round
    reverse
    remainder
    real?
    read-char
    rationalize
    rational?
    quotient
    quote
    quasiquote
    procedure?
    positive?
    peek-char
    pair?
    output-port?
    or
    odd?
    numerator
    number?
    number->string
    null?
    not
    newline
    negative?
    modulo
    min
    memv
    memq
    member
    max
    map
    make-vector
    make-string
    list?
    list-tail
    list-ref
    list->vector
    list->string
    list
    letrec-syntax
    letrec
    let-syntax
    let*
    let
    length
    lcm
    lambda
    integer?
    integer->char
    input-port?
    inexact?
    if
    gcd
    for-each
    floor
    expt
    exact?
    even?
    eqv?
    equal?
    eq?
    eof-object?
    else
    dynamic-wind
    do
    denominator
    define-syntax
    define
    current-output-port
    current-input-port
    cons
    cond
    complex?
    close-output-port
    close-input-port
    char>?
    char>=?
    char=?
    char<?
    char<=?
    char-ready?
    char->integer
    ceiling
    cdr
    cddr
    cdar
    case
    car
    call-with-values
    call-with-current-continuation
    cadr
    caar
    boolean?
    begin
    assv
    assq
    assoc
    apply
    append
    and
    abs
    >=
    >
    =>
    =
    <=
    <
    /
    ...
    -
    +
    *
    cddddr
    cdddar
    cddadr
    cddaar
    cdaddr
    cdadar
    cdaadr
    cdaaar
    cadddr
    caddar
    cadadr
    cadaar
    caaddr
    caadar
    caaadr
    caaaar
    cdddr
    cddar
    cdadr
    cdaar
    caddr
    cadar
    caadr
    caaar
    string-ci>?
    string-ci>=?
    string-ci=?
    string-ci<?
    string-ci<=?
    char-whitespace?
    char-upper-case?
    char-upcase
    char-numeric?
    char-lower-case?
    char-downcase
    char-ci>?
    char-ci>=?
    char-ci=?
    char-ci<?
    char-ci<=?
    char-alphabetic?
    tan
    sqrt
    sin
    log
    exp
    cos
    atan
    asin
    acos
    write
    display
    with-output-to-file
    with-input-from-file
    open-output-file
    open-input-file
    call-with-output-file
    call-with-input-file
    force
    delay
    exact->inexact
    inexact->exact
    read
    eval load interaction-environment
    scheme-report-environment null-environment)
  (begin
    (define (eval . args)
      (error "eval: not available in compiled programs"))
    (define (load . args)
      (error "load: not available in compiled programs"))
    (define (interaction-environment . args)
      (error "interaction-environment: not available in compiled programs"))
    (define (scheme-report-environment . args)
      (error "scheme-report-environment: not available in compiled programs"))
    (define (null-environment . args)
      (error "null-environment: not available in compiled programs"))))
