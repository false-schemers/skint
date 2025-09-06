(import (srfi 98))

;[esl] count errors
(define *error-count* 0)

; Larceny Test suite for SRFI 98
;
(define (writeln . xs)
  (for-each display xs)
  (newline))

(define (fail token . more)
  (writeln "Error: test failed: " token)
  (set! *error-count* (+ *error-count* 1)) ;[esl]+
  #f)

(or (string? (get-environment-variable "PATH"))
    (fail 'PATH))

(or (eq? #f (get-environment-variable "Unlikely To Be Any Such Thing"))
    (fail 'Unlikely))

;[esl] missing definition
(define (for-all p l)
  (if (pair? l) (and (p (car l)) (for-all p (cdr l))) #t))

(or (let ((alist (get-environment-variables)))
      (and (list? alist)
           (for-all pair? alist)
           (or (assoc "PATH" alist) (assoc "Path" alist)))) ;[esl] Win path spelled in camel case
    (fail 'get-environment-variables))

(writeln "Done.")
(exit *error-count*) ;[esl]+

