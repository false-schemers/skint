;;; box.sld -- (scheme box) as a real library, for compiled programs.
;;;
;;; Shadow counterpart of Skint's predefined (scheme box): the predefined
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

(define-library (scheme box)
  (import (only (skint) define lambda define-syntax syntax-rules if _ ...)
          (rename (only (skint) apply) (apply %builtin-apply))
          (rename (only (skint) set-box! unbox box box?)
            (set-box! %builtin-set-box!)
            (unbox %builtin-unbox)
            (box %builtin-box)
            (box? %builtin-box?)))

  (export
    (rename %cover-set-box! set-box!)
    (rename %cover-unbox unbox)
    (rename %cover-box box)
    (rename %cover-box? box?))

  (begin
    (define-syntax %cover-set-box!
      (syntax-rules ()
        [(_ . args) (%builtin-set-box! . args)]
        [_ %residual-set-box!]))
    (define %residual-set-box! (lambda (a0 a1) (%builtin-set-box! a0 a1)))
    (define-syntax set-box! %cover-set-box!)
    (define-syntax %cover-unbox
      (syntax-rules ()
        [(_ . args) (%builtin-unbox . args)]
        [_ %residual-unbox]))
    (define %residual-unbox (lambda (a0) (%builtin-unbox a0)))
    (define-syntax unbox %cover-unbox)
    (define-syntax %cover-box
      (syntax-rules ()
        [(_ . args) (%builtin-box . args)]
        [_ %residual-box]))
    (define %residual-box (lambda (a0) (%builtin-box a0)))
    (define-syntax box %cover-box)
    (define-syntax %cover-box?
      (syntax-rules ()
        [(_ . args) (%builtin-box? . args)]
        [_ %residual-box?]))
    (define %residual-box? (lambda (a0) (%builtin-box? a0)))
    (define-syntax box? %cover-box?)
))
