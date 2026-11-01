;;; case-lambda.sld -- (scheme case-lambda) as a real library, for compiled programs.
;;;
;;; Shadow counterpart of Skint's predefined (scheme case-lambda): the predefined
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

(define-library (scheme case-lambda)
  (import (only (scheme base)
            define-syntax syntax-rules lambda if let error
            null? pair? car cdr _ ...))
  (export case-lambda)
  (begin
    ;; case-lambda, spelled without lambda* (Skint's multi-arity
    ;; core form, which the compiled backend does not have): the
    ;; expansion dispatches on the shape of the argument list.
    (define-syntax %cl-try
      (syntax-rules ()
        [(_ () a (body ...) fb)
         (if (null? a) (let () body ...) fb)]
        [(_ (p . rest) a (body ...) fb)
         (if (pair? a)
             (let ([p (car a)]) (%cl-try rest (cdr a) (body ...) fb))
             fb)]
        [(_ p a (body ...) fb)
         (let ([p a]) body ...)]))

    (define-syntax %cl-chain
      (syntax-rules ()
        [(_ a) (error "case-lambda: no matching clause" a)]
        [(_ a (fmls . body) . rest)
         (%cl-try fmls a body (%cl-chain a . rest))]))

    (define-syntax case-lambda
      (syntax-rules ()
        [(_ (fmls . body) ...)
         (lambda args (%cl-chain args (fmls . body) ...))]))

))
