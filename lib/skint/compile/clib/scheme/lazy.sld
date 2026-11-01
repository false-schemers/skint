;;; lazy.sld -- (scheme lazy) as a real library, for compiled programs.
;;;
;;; Shadow counterpart of Skint's predefined (scheme lazy): the predefined
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

(define-library (scheme lazy)
  (import (only (scheme base)
            define define-syntax syntax-rules lambda if let let*
            cons car cdr set-car! set-cdr! unless _ ...)
          (only (scheme box) box box? unbox set-box!))
  (export promise? make-promise force delay delay-force)
  (begin
(define promise? box?)

(define (make-promise o) (box (cons #t o)))

(define (make-lazy-promise o) (box (cons #f o)))

(define (force p)
  (let ([pc (unbox p)])
    (if (car pc)
        (cdr pc)
        (let* ([newp ((cdr pc))] [pc (unbox p)])
          (unless (car pc)
            (set-car! pc (car (unbox newp)))
            (set-cdr! pc (cdr (unbox newp)))
            (set-box! newp pc))
          (force p)))))

(define-syntax delay-force
  (syntax-rules () [(_ x) (make-lazy-promise (lambda () x))]))

(define-syntax delay
  (syntax-rules () [(_ x) (delay-force (make-promise x))]))

))
