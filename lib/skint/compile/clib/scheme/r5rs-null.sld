;;; r5rs-null.sld -- (scheme r5rs-null) as a real library, for compiled programs.
;;;
;;; Shadow counterpart of Skint's predefined (scheme r5rs-null): the predefined
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

(define-library (scheme r5rs-null)
  (import
          (only (scheme base)
            unquote-splicing
            unquote
            syntax-rules
            set!
            quote
            quasiquote
            or
            letrec-syntax
            letrec
            let-syntax
            let*
            let
            lambda
            if
            else
            do
            define-syntax
            define
            cond
            case
            begin
            and
            =>
            ...)
          (only (scheme lazy)
            delay)
)
  (export
    unquote-splicing
    unquote
    syntax-rules
    set!
    quote
    quasiquote
    or
    letrec-syntax
    letrec
    let-syntax
    let*
    let
    lambda
    if
    else
    do
    define-syntax
    define
    cond
    case
    begin
    and
    =>
    ...
    delay)
)
