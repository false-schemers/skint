
(define-library (srfi 5)
  (export let)
  (import (rename (scheme base) (let standard-let)))

(begin  

; [esl] code taken from the original reference implementation by
; Copyright (C) Andy Gaynor (1999). All Rights Reserved. 

(define-syntax let
  (syntax-rules ()
    ;; No bindings: use standard-let.
    ((let () body ...)
     (standard-let () body ...))
    ;; Or call a lambda.
    ;; ((lambda () body ...))

    ;; All standard bindings: use standard-let.
    ((let ((var val) ...) body ...)
     (standard-let ((var val) ...) body ...))
    ;; Or call a lambda.
    ;; ((lambda (var ...) body ...) val ...)

    ;; One standard binding: loop.
    ;; The all-standard-bindings clause didn't match,
    ;; so there must be a rest binding.
    ((let ((var val) . bindings) body ...)
     (let-loop #f bindings (var) (val) (body ...)))

    ;; Signature-style name: loop.
    ((let (name binding ...) body ...)
     (let-loop name (binding ...) () () (body ...)))

    ;; defun-style name: loop.
    ((let name bindings body ...)
     (let-loop name bindings () () (body ...)))))

(define-syntax let-loop
  (syntax-rules ()
    ;; Standard binding: destructure and loop.
    ((let-loop name ((var0 val0) binding ...) (var ...     ) (val ...     ) body)
     (let-loop name (            binding ...) (var ... var0) (val ... val0) body))

    ;; Rest binding, no name: use standard-let, listing the rest values.
    ;; Because of let's first clause, there is no "no bindings, no name" clause.
    ((let-loop #f (rest-var rest-val ...) (var ...) (val ...) body)
     (standard-let ((var val) ... (rest-var (list rest-val ...))) . body))
    ;; Or call a lambda with a rest parameter on all values.
    ;; ((lambda (var ... . rest-var) . body) val ... rest-val ...))
    ;; Or use one of several other reasonable alternatives.

    ;; No bindings, name: call a letrec'ed lambda.
    ((let-loop name () (var ...) (val ...) body)
     ((letrec ((name (lambda (var ...) . body)))
        name)
      val ...))

    ;; Rest binding, name: call a letrec'ed lambda.
    ((let-loop name (rest-var rest-val ...) (var ...) (val ...) body)
     ((letrec ((name (lambda (var ... . rest-var) . body)))
        name)
      val ... rest-val ...))))
  
))
