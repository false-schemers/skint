(import (srfi 5))
(include "test.scm")

(test-begin "srfi-5")

; [esl] code adapted from the original specification by
; Copyright (C) Andy Gaynor (1999). All Rights Reserved. 

(test-equal 55
  (let (fibonacci (n 10) (i 0) (f0 0) (f1 1))
    (if (= i n)
      f0
      (fibonacci n (+ i 1) f1 (+ f0 f1)))))

(test-equal "345"
  (let (blast (port (open-output-string)) . (x (+ 1 2) 4 5))
    (if (null? x)
        (get-output-string port)
        (begin
          (write (car x) port)
          (apply blast port (cdr x))))))

(test-end)

