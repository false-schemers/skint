(import (srfi 55))
(include "test.scm")

(test-begin "srfi-55")

(require-extension (srfi 8 31))

(define f (rec (f n)
		((rec (g k l)
		   (if (zero? k) l
		     (g (- k 1) (* k l)))) n 1)))

(test 3628806
  (receive (x y) (values (f 3) (f 10)) (+ x y)))

(test-end)

