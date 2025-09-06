(import (srfi 236))
(include "test.scm")

(define-syntax check
  (syntax-rules (=>)
    [(_) #t]
    [(_ e => r . more) (begin (test r e) (check . more))]))

;[esl] adapted from tests by Shiro Kawai

(test-begin "srfi-236")

;; just to test edge case
(check
 (let ((v 'ok))
   (independently)
   v)
 => 'ok)

(check
 (let ((v (cons #f #f)))
   (independently
    (set-car! v 'ok))
   v)
 => '(ok . #f))

(define (set-car+cdr! p x y)
  (independently
   (set-car! p x)
   (set-cdr! p y)))

(check
 (let ((p (cons 1 2)))
   (set-car+cdr! p 10 20)
   p)
 => '(10 . 20))

(check
 (let ((v (vector 1 2 3)))
   (independently
    (vector-set! v 0 10)
    (vector-set! v 1 20)
    (vector-set! v 2 30))
   v)
 => '#(10 20 30))

(check
 (let ((x '()))
   (independently
    (set! x (cons 1 x))
    (set! x (cons 2 x)))
   (apply + x))
 => 3)

(test-end)

