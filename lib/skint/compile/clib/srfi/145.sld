
(define-library (srfi 145)
  (export assume)
  (import (scheme base))
  (cond-expand
    ((or elide-assumptions (and (not assumptions) (not debug)))
     (begin
       (define-syntax assume
         (syntax-rules ()
           ((assume expr obj ...)
            expr)
           ((assume)
            (syntax-error "assume requires an expression"))))))
    (else
     (begin
       (define-syntax assume
         (syntax-rules ()
           ((assume expr obj ...)
            (unless expr (assumption-failed 'expr (lambda () (list obj ...)))))
           ((assume)
            (syntax-error "assume requires an expression")))))))
(begin
;; adapted from Larceny
(define (assumption-failed expr thunk)
  (let ((info (guard (c (else c)) (thunk))))
    (cond ((error-object? info)
           (apply error "Assumption failed, and then raised an exception: "
              expr (error-object-message info) (error-object-irritants info)))
          ((and (pair? info) (string? (car info)))
           (apply error (string-append "Assumption failed: " (car info)) expr (cdr info)))
          (else 
           (apply error "Assumption failed: " expr info)))))

))
