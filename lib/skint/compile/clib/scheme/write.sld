;;; write.sld -- (scheme write) as a real library, for compiled programs.
;;;
;;; Shadow counterpart of Skint's predefined (scheme write): the predefined
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

(define-library (scheme write)
  (import (only (skint) define lambda define-syntax syntax-rules if _ ...)
          (rename (only (skint) apply) (apply %builtin-apply))
          (only (scheme base) pair? car)
          (rename (only (skint) write-simple write-shared write display)
            (write-simple %builtin-write-simple)
            (write-shared %builtin-write-shared)
            (write %builtin-write)
            (display %builtin-display)))

  (export
    (rename %cover-write-simple write-simple)
    (rename %cover-write-shared write-shared)
    (rename %cover-write write)
    (rename %cover-display display))

  (begin
    (define-syntax %cover-write-simple
      (syntax-rules ()
        [(_ . args) (%builtin-write-simple . args)]
        [_ %residual-write-simple]))
    (define %residual-write-simple
      (lambda (a0 . args)
        (if (pair? args) (%builtin-write-simple a0 (car args)) (%builtin-write-simple a0))))
    (define-syntax write-simple %cover-write-simple)
    (define-syntax %cover-write-shared
      (syntax-rules ()
        [(_ . args) (%builtin-write-shared . args)]
        [_ %residual-write-shared]))
    (define %residual-write-shared
      (lambda (a0 . args)
        (if (pair? args) (%builtin-write-shared a0 (car args)) (%builtin-write-shared a0))))
    (define-syntax write-shared %cover-write-shared)
    (define-syntax %cover-write
      (syntax-rules ()
        [(_ . args) (%builtin-write . args)]
        [_ %residual-write]))
    (define %residual-write
      (lambda (a0 . args)
        (if (pair? args) (%builtin-write a0 (car args)) (%builtin-write a0))))
    (define-syntax write %cover-write)
    (define-syntax %cover-display
      (syntax-rules ()
        [(_ . args) (%builtin-display . args)]
        [_ %residual-display]))
    (define %residual-display
      (lambda (a0 . args)
        (if (pair? args) (%builtin-display a0 (car args)) (%builtin-display a0))))
    (define-syntax display %cover-display)
))
