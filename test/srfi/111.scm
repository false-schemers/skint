(import (srfi 111))
(include "test.scm")

(test-begin "srfi-111")

(test-assert (box? #&42))

(define b (box 42))

(test-assert (box? b))

(test-equal 42 (unbox b))

(set-box! b 24)

(test-equal 24 (unbox b))

(test-end)

