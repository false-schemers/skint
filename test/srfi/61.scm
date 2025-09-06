(import (srfi 61))
(include "test.scm")

(test-begin "srfi-61")

(define (port->char-list port) 
  (cond ((read-char port) char? => (lambda (c) (cons c (port->char-list port)))) 
  (else '())))

(test '(#\1 #\2 #\3 #\5 #\7) (port->char-list (open-input-string "12357")))

(test-end)
