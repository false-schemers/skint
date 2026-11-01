
(define-library (srfi 31)
  (export rec)
  (import (scheme base))
  (begin
    (define-syntax rec
      (syntax-rules ()
        ((rec (name . args) . body)
         (letrec ((name (lambda args . body))) name))
        ((rec name expr)
         (letrec ((name expr)) name))))))
