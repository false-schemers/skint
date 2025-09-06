(import (srfi 176))
(include "test.scm")

(test-begin "srfi-176")

(define (elt-valid? x)
  (or (symbol? x) (string? x) (and (number? x) (exact-integer? x) (>= x 0))))

(define (elt*-valid? x*)
  (or (null? x*) (and (elt-valid? (car x*)) (elt*-valid? (cdr x*)))))

(define (entry-valid? p)
  (and (pair? p) (symbol? (car p)) (list? (cdr p)) (elt*-valid? (cdr p))))

(define (entry*-valid? p*)
  (or (null? p*) (and (entry-valid? (car p*)) (entry*-valid? (cdr p*)))))

(define (version-alist-valid? al)
  (and (list? al) (entry*-valid? al))) 

(test-assert (version-alist-valid? (version-alist)))

(test-end)

