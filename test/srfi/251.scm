(import (srfi 251))
(include "test.scm")

(test-begin "srfi-251")

(test-equal "the result is: 42"
  (let ((x 0) (p (open-output-string)))
    (display "the result is" p)
    (define (foo) x)
    (define x 42)
    (display ": " p)
    (display (foo) p)
    (get-output-string p)))

(test-equal "the result is: 0"
  (let ((x 0) (p (open-output-string)))
    (display "the result is" p)
    (define (foo) x)
    (display ": " p)
    (define xx 42)
    (display (foo) p)
    (get-output-string p)))

(test-equal "the result is: 0"
  (let ((x 0) (p (open-output-string)))
    (define-syntax define-thunk
      (syntax-rules ()
        ((_ i v) (define (i) v))))
    (display "the result is" p)
    (display ": " p)
    (define xx 42)
    (define-thunk foo x)
    (display (foo) p)
    (get-output-string p)))

(test-end)

