(import (srfi 26))
(include "test.scm")

(test-begin "srfi-26: cut")

(let ((x 'orig))
  (let ((f (cute list x)))
    (set! x 'wrong)
    (test '(orig) (f))))

(let ((x 'wrong))
  (let ((f (cut list x)))
    (set! x 'right)
    (test '(right) (f))))

(test-end)
