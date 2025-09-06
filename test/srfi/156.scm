(import (srfi 156) (srfi 1))
(include "test.scm")

(test-begin "srfi-156")
;; tests from the original implementation by Panicz Maciej Godek 

;; 3. A micro unit-test framework for the purpose of demonstation

;;[esl] modified to use test-assert
(define-syntax e.g.
  (syntax-rules ()
    ((e.g. expression)
     (test-assert expression)
     #;(let ((result expression))
       (if (not result)
	   (error "test failed: " 'expression)
	   result)))))

;; 4. Examples/tests:

;; single argument/unary predicate:

(e.g.
 (is 1 odd?))

(e.g.
 (isnt 2 odd?))

(e.g.
 (is '() null?))

(e.g.
 (is procedure? procedure?))

(e.g.
 (isnt 5 procedure?))

;; two arguments:

(e.g.
 (is 1 < 2))

(e.g.
 (isnt 1 < 1))

(e.g.
 (is (+ 2 2) = 4))

(e.g.
 (is 'x eq? 'x))

(e.g.
 (is procedure? eq? procedure?))

(e.g.
 (eq? (is eq? eq? eq?)
      (eq? eq? eq?)))

(e.g.
 (is (is eq? eq? eq?) eq? (eq? eq? eq?)))

(e.g.
 (is 'y memq '(x y z)))

(e.g.
 (is '(1) member '(() (1) (2) (1 2))))

(e.g.
 (isnt 'x eq? 'y))

(e.g.
 (is '(a b c) equal? '(a b c)))

(e.g.
 (isnt '(a b c) equal? '(c b a)))

(e.g.
 (is 0 = 0.0))

(e.g.
 (is 1.0 = 1))

(e.g.
 (isnt 1 = 0))

(define (divisible-by? x y)
  (is (modulo x y) = 0))

(e.g.
 (is 9 divisible-by? 3))

(e.g.
 (isnt 3 divisible-by? 9))

;; ending with unary predicate:

(e.g.
 (is 1 < 2 even?))

(e.g.
 (isnt 1 < 2 odd?))

(e.g.
 (isnt 2 < 1 even?))

(e.g.
 (is 0 = 0.0 zero?))

(e.g.
 (isnt 1.0 = 1 zero?))

;; three arguments:

(e.g.
 (is 1 < 2 <= 3))

;[esl]- complex #s are not supported 
;(e.g.
; (is 0 = 0.0 = 0+0i = 0.0+0.0i))

(e.g.
 (isnt 1 <= 2 < 2))

;; predicates don't need to be transitive
;; (although that's not particularly elegant):

(e.g.
 (is 1 < 2 > 1.5))

(e.g.
 (isnt 1 < 2 > 3))

(e.g.
 (isnt 3 < 2 < 1))

(e.g.
 (is 'x member '(x y) member '((x y) (y x))))

;; more arguments:

(e.g.
 (is -0.4 < -0.1 <= 0 = 0.0 < 0.1 < 0.4))

(e.g.
 (isnt -0.4 < -0.1 <= 0 = 0.0 < 0.1 < -0.1))

;[esl]- complex #s are not supported 
;(e.g.
; (is 0 = 0.0 = 0+0i = 0.0+0.0i = (+) < (*) = 1 = 1.0 = 1+0i = 1.0+0.0i))

;; ending with unary predicate:

(e.g.
 (is -0.4 < -0.1 <= 0 <= 0.0 < 0.1 < 0.4 <= 2 even?))

(e.g.
 (isnt -0.4 < -0.1 <= 0 <= 0.0 < 0.1 < 0.4 <= 2 odd?))

;; as procedures (with underscore):

(e.g.
 (equal? (filter (isnt _ even?) '(2 4 5 6 7 8))
	 '(5 7)))

(e.g.
 (equal? (filter (is _ < 2) '(1 3 2 0))
	 '(1 0)))

(e.g.
 (equal? (filter (is 1 < _) '(1 3 2 0))
	 '(3 2)))

(e.g.
 (equal? (filter (is 3 < _ <= 5) '(2 3 4 5 6 7))
	 '(4 5)))

(e.g.
 (equal? (filter (is 'x memq _) '((a b) (x) (p q) (x y) (c d) (z x)))
	 '((x) (x y) (z x))))

(e.g.
 (equal? (filter (isnt 'x memq _) '((a b) (x) (p q) (x y) (c d) (z x)))
	 '((a b) (p q) (c d))))

(e.g.
 (equal? (filter (isnt 3 < _ <= 5) '(2 3 4 5 6 7))
	 '(2 3 6 7)))

(e.g.
 (equal? (filter (is _ eq? 'a) '(m a m a))
	 '(a a)))

(e.g.
 (equal? (filter (isnt 'a eq? _) '(m a m a))
	 '(m m)))

;; multiple underscores:

(e.g.
 ((is _ < 2 < _) 1 3))

(e.g.
 ((isnt 1 < _ <= _ < 3) 2 4))

(e.g.
 ((is _ < _ even?) 1 2))

(e.g.
 ((isnt _ < _ odd?) 1 2))

(e.g.
 ((is 1 < _ <= 3 < _ <= 5 < _) 3 5 6))

(e.g.
 ((isnt 1 < _ <= 3 < _ <= 5 < _) 3 3 6))

(e.g.
 ((is 1 < _ <= 3 < _ <= 5 < _ even?) 3 5 6))

(e.g.
 ((isnt 1 < _ <= 3 < _ <= 5 < _ odd?) 3 5 6))

(test-end)

