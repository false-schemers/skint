;;; file.sld -- (scheme file) as a real library, for compiled programs.
;;;
;;; Shadow counterpart of Skint's predefined (scheme file): the predefined
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

(define-library (scheme file)
  (import (only (scheme base)
            define lambda define-syntax syntax-rules if or apply error
            call-with-port current-input-port current-output-port
            parameterize _ ...)
          (only (skint hidden)
            %open-input-file %open-output-file
            %open-binary-input-file %open-binary-output-file)
          (rename (only (skint) apply) (apply %builtin-apply))
          (rename (only (skint) file-exists? delete-file)
            (file-exists? %builtin-file-exists?)
            (delete-file %builtin-delete-file)))

  (export
    (rename %cover-file-exists? file-exists?)
    (rename %cover-delete-file delete-file)
    open-input-file
    open-output-file
    open-binary-input-file
    open-binary-output-file
    call-with-input-file
    call-with-output-file
    with-input-from-file
    with-output-to-file)

  (begin
    ;; the s.scm definitions call %open-input-file & friends; those
    ;; names are bound by the (skint hidden) import above.

    (define (file-error msg . args) (apply error msg args))

(define (with-input-from-port port thunk) ; +
  (parameterize ([current-input-port port]) (thunk)))

(define (with-output-to-port port thunk) ; +
  (parameterize ([current-output-port port]) (thunk)))

(define (open-input-file fn)
  (or (%open-input-file fn)
      (file-error "cannot open input file" fn)))

(define (open-output-file fn)
  (or (%open-output-file fn)
      (file-error "cannot open output file" fn)))

(define (open-binary-input-file fn)
  (or (%open-binary-input-file fn)
      (file-error "cannot open binary input file" fn)))

(define (open-binary-output-file fn)
  (or (%open-binary-output-file fn)
      (file-error "cannot open binary output file" fn)))

(define (call-with-input-file fname proc)
  (call-with-port (open-input-file fname) proc))

(define (call-with-output-file fname proc)
  (call-with-port (open-output-file fname) proc))

(define (with-input-from-file fname thunk)
  (call-with-input-file fname (lambda (p) (with-input-from-port p thunk))))

(define (with-output-to-file fname thunk)
  (call-with-output-file fname (lambda (p) (with-output-to-port p thunk))))

    (define-syntax %cover-file-exists?
      (syntax-rules ()
        [(_ . args) (%builtin-file-exists? . args)]
        [_ %residual-file-exists?]))
    (define %residual-file-exists? (lambda (a0) (%builtin-file-exists? a0)))
    (define-syntax file-exists? %cover-file-exists?)
    (define-syntax %cover-delete-file
      (syntax-rules ()
        [(_ . args) (%builtin-delete-file . args)]
        [_ %residual-delete-file]))
    (define %residual-delete-file (lambda (a0) (%builtin-delete-file a0)))
    (define-syntax delete-file %cover-delete-file)
))
