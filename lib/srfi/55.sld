
(define-library (srfi 55)
  (export require-extension)
  (import (skint))
  (begin
   (define-syntax require-extension
     (syntax-rules ()
       ((require-extension (prefix mod ...))
        (begin (import (prefix mod) ...)))))))
