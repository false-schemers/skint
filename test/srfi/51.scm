(import (srfi 51))
(include "test.scm")

(test-begin "srfi-51")

(define caller vector)
(define rest-list '(x 1))

(test-values (values 'x 1) (rest-values rest-list)) ; => x 1

(test-values (values 'x 1) (rest-values rest-list 2)) ; => x 1

(test-values (values 'x 1) (rest-values caller rest-list)) ; => x 1

(test-values (values 'x 1) (rest-values caller rest-list -3)) ; => x 1

(test-error (rest-values rest-list -2 'y 3 1))
;  => error too many defaults (y 3 1) default-list (<= (length default-list) 2)

(test-error (rest-values 'caller rest-list 1 '(x y z)))
; => error too many arguments (x 1) rest-list (<= (length rest-list) 1) caller

(test-error (rest-values caller rest-list 2 (list 'x 'y 'z) (cons "str" string?)))
; => error incorrect argument 1 arg (<procedure string?> arg) <procedure caller>

(test-error (rest-values rest-list 2 '(y z) `(100 . ,number?)))
; => error unmatched argument x arg (member arg (y z))

(test-error (rest-values "caller: bad argument" rest-list 2 '(y z) `(100 . ,number?)))
; => error caller: bad argument x arg (member arg (y z))

(test-error (rest-values 'caller rest-list (list 'x 'y) (cons 1 number?)))
;  => error bad optional argument (x y) option
;    (or (boolean? option) (integer? option) (memq option (list + -))) caller

(test-values (values 'x 1 "str") (rest-values rest-list - 'y 100 "str"))
; => x 1 "str"

(test-values (values 'x 1 "str") (rest-values rest-list + `(x y z) `(100 . ,number?) `("str" . ,string?)))
; => x 1 "str"

(test-values (values 'x 1 "str") (rest-values rest-list #t `(x y z) `(100 . ,number?) `("str" . ,string?)))
; => x 1 "str"

(test-values (values 1 "str" 'x) (rest-values rest-list #t `(100 . ,number?) `("str" . ,string?) `(x y z)))
; => 1 "str" x

(test-error (rest-values rest-list #t `(100 . ,number?) `("str" . ,string?) `(y z)))
; => error bad argument (x) rest-list (null? rest-list)

(test-values (values 1 "str" 'y 'x) (rest-values rest-list #f `(100 . ,number?) `("str" . ,string?) `(y z)))
; => 1 "str" y x

(define recaller vector)
(define str    "string")
(define num    2)

(test-error (arg-and num (number? num) (< num 2)))
;       => error incorrect argument 2 num (< num 2)

(test-error (arg-and caller num (number? num) (< num 2)))
;       => error incorrect argument 2 num (< num 2) <procedure caller>

(test-error (arg-and 'caller num (number? num) (< num 2)))
;       => error incorrect argument 2 num (< num 2) caller

(test-error (arg-and "caller: bad argument" num (number? num) (< num 2)))
;       => error caller: bad argument 2 num (< num 2)

(test-error (arg-ands (str (string? str) (< (string-length str) 7))
	  ("caller: bad argument" num (number? num) (< num 2))))
;       => error caller: bad argument 2 num (< num 2)

(test-error (arg-ands ("caller: bad argument" str (string? str) (< (string-length str) 7))
	  (num (number? num) (< num 2))))
;       => error incorrect argument 2 num (< num 2)

(test-error (arg-ands common 'caller
	  (str (string? str) (< (string-length str) 7))
	  (num (number? num) (< num 2))))
;       => error incorrect argument 2 num (< num 2) caller

(test-error (arg-ands common "caller: bad argument"
	  (str (string? str) (< (string-length str) 7))
	  ("caller: incorrect argument" num (number? num) (< num 2))))
;       => error caller: incorrect argument 2 num (< num 2)

(test-error (err-and 'caller
	 (string? str) (< (string-length str) 7) (number? num) (< num 2)))
;       => error false expression (< num 2) caller

(test-error (err-ands (caller (string? str) (< (string-length str) 7))
	  ("num failed test in caller" (number? num) (< num 2))))
;       => error num failed test in caller (< num 2)

#|
(define (read-line . p-d)
  ;; p-d should be (<input-port> <symbol>).
  (receive (p d) (rest-values p-d 2
			      (cons (current-input-port) input-port?)
			      (list 'trim 'concat 'split...))
    ...))
(define (read-line . p-d)
  (receive (p d) (rest-values p-d -2 (current-input-port) 'trim)
    (arg-ands (p (input-port? p))
	      (d (memq d '(trim concat split...))))
    ...))
(define (read-line . p-d)
  ;; p-d can be (<input-port> <symbol>) or (<symbol> <input-port>).
  (receive (p d) (rest-values p-d #t
			      (cons (current-input-port) input-port?)
			      (list 'trim 'concat 'split...))
    ...))

(define (delete x ls . predicate)
  (let ((pred (rest-values 'delete predicate 1 (list equal? eqv? eq?))))
    ...))
(define (delete x ls . predicate)
  (let ((pred (rest-values 'delete predicate -1 equal?)))
    (err-and 'delete (list? ls) (memq pred (list equal? eqv? eq?)))
    ...))

(define (substring str . start-end)
  (let ((str-len (arg-and substring str (string? str) (string-length str))))
    (receive (start end) (rest-values substring start-end -2 0 str-len)
      (arg-ands common substring
		(start (integer? start) (<= 0 start str-len))
		(end (integer? end) (<= start end str-len)))
      ...)))

(define (procedure-with-sequential-binding-arguments . a-b-c)
  (receive (a b c) (rest-values a-b-c -3 10 #f #f)
    (let* ((b (or b (+ a 10)))
	   (c (or c (+ a b))))
      ...)))
|#

(test-end)

