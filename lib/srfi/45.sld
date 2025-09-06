
(define-library (srfi 45)
  (export delay lazy force eager)
  (import (scheme base))
  (import (srfi 111))
(begin
(define-syntax lazy
  (syntax-rules ()
    ((lazy exp)
     (box (cons 'lazy (lambda () exp))))))

(define (eager x)
  (box (cons 'eager x)))

(define-syntax delay
  (syntax-rules ()
    ((delay exp) (lazy (eager exp)))))

(define (force promise)
  (let ((content (unbox promise)))
    (case (car content)
      ((eager) (cdr content))
      ((lazy)  (let* ((promise* ((cdr content)))        
                      (content  (unbox promise)))                      ; * 
                 (if (not (eqv? (car content) 'eager))                 ; *
                     (begin (set-car! content (car (unbox promise*)))
                            (set-cdr! content (cdr (unbox promise*)))
                            (set-box! promise* content)))
                 (force promise))))))

; (*) These two lines re-fetch and check the original promise in case 
;     the first line of the let* caused it to be forced.  For an example  
;     where this happens, see reentrancy test 3 below.
))