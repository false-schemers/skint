;;; process-context.sld -- (scheme process-context) as a real library, for compiled programs.
;;;
;;; Shadow counterpart of Skint's predefined (scheme process-context): the predefined
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
;;; command-line, exit, emergency-exit and get-environment-variables
;;; are runtime facilities (argv/environment access, process
;;; exit): they are defined here as ordinary procedures over
;;; the corresponding builtins, which the backends treat as
;;; prims in call position.  No compiler synthesizes them.

(define-library (scheme process-context)
  (import (only (skint) define lambda define-syntax syntax-rules if _ ...)
          (rename (only (skint) apply) (apply %builtin-apply))
          (only (scheme base) pair? car cons reverse + let cond
            string-length string-ref char->integer substring =)
          (only (skint hidden) %exit %argv-ref %envv-ref)
          (only (skint) quote else)
          (rename (only (skint) get-environment-variable)
            (get-environment-variable %builtin-get-environment-variable)))

  (export
    get-environment-variables
    (rename %cover-get-environment-variable get-environment-variable)
    exit
    emergency-exit
    command-line)

  (begin
    ;; over the %envv-ref prim ("name=value" or #f)
    (define (%env-kv kvs)
      (let ([n (string-length kvs)])
        (let loop ([i 0])
          (cond [(= i n) (cons kvs "")]
                [(= (char->integer (string-ref kvs i)) 61)
                 (cons (substring kvs 0 i)
                       (substring kvs (+ i 1) n))]
                [else (loop (+ i 1))]))))
    (define (get-environment-variables)
      (let loop ([r '()] [i 0])
        (let ([kvs (%envv-ref i)])
          (if kvs (loop (cons (%env-kv kvs) r) (+ i 1))
              (reverse r)))))
    (define-syntax %cover-get-environment-variable
      (syntax-rules ()
        [(_ . args) (%builtin-get-environment-variable . args)]
        [_ %residual-get-environment-variable]))
    (define %residual-get-environment-variable (lambda (a0) (%builtin-get-environment-variable a0)))
    (define-syntax get-environment-variable %cover-get-environment-variable)
    ;; exit/emergency-exit bottom out in the %exit prim
    ;; (the optional argument defaults to success, and the
    ;; prim converts #f/#t/a fixnum to an exit status).  A
    ;; compiled program has no REPL continuation for exit
    ;; to escape to, so exit is emergency-exit here, as in
    ;; Skint batch programs.
    (define (exit . args)
      (if (pair? args) (%exit (car args)) (%exit #t)))
    (define (emergency-exit . args)
      (if (pair? args) (%exit (car args)) (%exit #t)))
    ;; over the %argv-ref prim (string or #f per index),
    ;; Skint's own %command-line definition
    (define (command-line)
      (let loop ([r '()] [i 0])
        (let ([arg (%argv-ref i)])
          (if arg (loop (cons arg r) (+ i 1)) (reverse r)))))
))
