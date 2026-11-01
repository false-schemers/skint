;;; inexact.sld -- (scheme inexact) as a real library, for compiled programs.
;;;
;;; Shadow counterpart of Skint's predefined (scheme inexact): the predefined
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

(define-library (scheme inexact)
  (import (only (skint) define lambda define-syntax syntax-rules if _ ...)
          (rename (only (skint) apply) (apply %builtin-apply))
          (only (scheme base) pair? car)
          (rename (only (skint) flonum? flfinite? flinfinite? flnan?)
            (flonum? %builtin-flonum?) (flfinite? %builtin-flfinite?)
            (flinfinite? %builtin-flinfinite?) (flnan? %builtin-flnan?))
          (rename (only (skint) atan log)
            (atan %builtin-atan) (log %builtin-log))
          (rename (only (skint) tan sqrt sin exp cos asin acos)
            (tan %builtin-tan)
            (sqrt %builtin-sqrt)
            (sin %builtin-sin)
            (exp %builtin-exp)
            (cos %builtin-cos)
            (asin %builtin-asin)
            (acos %builtin-acos)))

  (export
    (rename %cover-tan tan)
    (rename %cover-sqrt sqrt)
    (rename %cover-sin sin)
    nan?
    log
    infinite?
    finite?
    (rename %cover-exp exp)
    (rename %cover-cos cos)
    atan
    (rename %cover-asin asin)
    (rename %cover-acos acos))

  (begin
    (define-syntax %cover-tan
      (syntax-rules ()
        [(_ . args) (%builtin-tan . args)]
        [_ %residual-tan]))
    (define %residual-tan (lambda (a0) (%builtin-tan a0)))
    (define-syntax tan %cover-tan)
    (define-syntax %cover-sqrt
      (syntax-rules ()
        [(_ . args) (%builtin-sqrt . args)]
        [_ %residual-sqrt]))
    (define %residual-sqrt (lambda (a0) (%builtin-sqrt a0)))
    (define-syntax sqrt %cover-sqrt)
    (define-syntax %cover-sin
      (syntax-rules ()
        [(_ . args) (%builtin-sin . args)]
        [_ %residual-sin]))
    (define %residual-sin (lambda (a0) (%builtin-sin a0)))
    (define-syntax sin %cover-sin)
    (define (nan? x) (if (%builtin-flonum? x) (%builtin-flnan? x) #f))
    ;; log takes one argument (log z) or two (log z base)
    (define (log x . rest)
      (if (pair? rest) (%builtin-log x (car rest))
          (%builtin-log x)))
    (define (infinite? x) (if (%builtin-flonum? x) (%builtin-flinfinite? x) #f))
    ;; finite?/infinite?/nan? over all reals: exact numbers
    ;; are always finite and never NaN; flonums defer to the
    ;; fl* primitives.  (Defining these here keeps the generic
    ;; names out of every backend's prim table.)
    (define (finite? x) (if (%builtin-flonum? x) (%builtin-flfinite? x) #t))
    (define-syntax %cover-exp
      (syntax-rules ()
        [(_ . args) (%builtin-exp . args)]
        [_ %residual-exp]))
    (define %residual-exp (lambda (a0) (%builtin-exp a0)))
    (define-syntax exp %cover-exp)
    (define-syntax %cover-cos
      (syntax-rules ()
        [(_ . args) (%builtin-cos . args)]
        [_ %residual-cos]))
    (define %residual-cos (lambda (a0) (%builtin-cos a0)))
    (define-syntax cos %cover-cos)
    ;; atan takes one argument (atan z) or two (atan y x);
    ;; each arity is a backend prim in call position
    (define (atan x . rest)
      (if (pair? rest) (%builtin-atan x (car rest))
          (%builtin-atan x)))
    (define-syntax %cover-asin
      (syntax-rules ()
        [(_ . args) (%builtin-asin . args)]
        [_ %residual-asin]))
    (define %residual-asin (lambda (a0) (%builtin-asin a0)))
    (define-syntax asin %cover-asin)
    (define-syntax %cover-acos
      (syntax-rules ()
        [(_ . args) (%builtin-acos . args)]
        [_ %residual-acos]))
    (define %residual-acos (lambda (a0) (%builtin-acos a0)))
    (define-syntax acos %cover-acos)
))
