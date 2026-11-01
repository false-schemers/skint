;;; time.sld -- (scheme time) as a real library, for compiled programs.
;;;
;;; Shadow counterpart of Skint's predefined (scheme time): the predefined
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

(define-library (scheme time)
  (import (only (skint) define lambda define-syntax syntax-rules if _ ...)
          (rename (only (skint) apply) (apply %builtin-apply))
          (rename (only (skint) jiffies-per-second current-second current-jiffy)
            (jiffies-per-second %builtin-jiffies-per-second)
            (current-second %builtin-current-second)
            (current-jiffy %builtin-current-jiffy)))

  (export
    (rename %cover-jiffies-per-second jiffies-per-second)
    (rename %cover-current-second current-second)
    (rename %cover-current-jiffy current-jiffy))

  (begin
    (define-syntax %cover-jiffies-per-second
      (syntax-rules ()
        [(_ . args) (%builtin-jiffies-per-second . args)]
        [_ %residual-jiffies-per-second]))
    (define %residual-jiffies-per-second (lambda () (%builtin-jiffies-per-second)))
    (define-syntax jiffies-per-second %cover-jiffies-per-second)
    (define-syntax %cover-current-second
      (syntax-rules ()
        [(_ . args) (%builtin-current-second . args)]
        [_ %residual-current-second]))
    (define %residual-current-second (lambda () (%builtin-current-second)))
    (define-syntax current-second %cover-current-second)
    (define-syntax %cover-current-jiffy
      (syntax-rules ()
        [(_ . args) (%builtin-current-jiffy . args)]
        [_ %residual-current-jiffy]))
    (define %residual-current-jiffy (lambda () (%builtin-current-jiffy)))
    (define-syntax current-jiffy %cover-current-jiffy)
))
