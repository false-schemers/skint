(define-library (srfi 227 definition)
  (import (scheme base)) 
  (import (srfi 227)) 
  (export define-optionals define-optionals*)

(begin

(define-syntax define-optionals
  (syntax-rules ()
    ((_ (name . vars) . body)
     (define name (opt-lambda vars . body)))))

(define-syntax define-optionals*
  (syntax-rules ()
    ((_ (name . vars) . body)
     (define name (opt*-lambda vars . body)))))

))

