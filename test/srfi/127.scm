(import (srfi 127))

(include "test.scm")

;;; Copyright (c) 2009-2021 Alex Shinn
;;; All rights reserved.
;;; 
;;; Redistribution and use in source and binary forms, with or without
;;; modification, are permitted provided that the following conditions
;;; are met:
;;; 1. Redistributions of source code must retain the above copyright
;;;    notice, this list of conditions and the following disclaimer.
;;; 2. Redistributions in binary form must reproduce the above copyright
;;;    notice, this list of conditions and the following disclaimer in the
;;;    documentation and/or other materials provided with the distribution.
;;; 3. The name of the author may not be used to endorse or promote products
;;;    derived from this software without specific prior written permission.
;;; 
;;; THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
;;; IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
;;; OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
;;; IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
;;; INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
;;; NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
;;; DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
;;; THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
;;; (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
;;; THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

;; Make-generator for tests cloned from SRFI 121
(define (make-generator . args)
  (lambda ()
    (if (null? args)
        (eof-object)
        (let ((next (car args)))
          (set! args (cdr args))
          next))))

;; Make-lseq creates an lseq, like list, but guarantees the use of
;; a generator.
(define (make-lseq . args)
  (generator->lseq (apply make-generator args)))

(define (factorial n)
  (cond
   ((< n 0) #f)
   ((= n 0) 1)
   (else (* n (factorial (- n 1))))))

(test-begin "srfi-127")

(let ((one23 (make-lseq 1 2 3)))
  (test 1 (car one23))
  (test-assert (procedure? (cdr one23)))
  (test '(1 2 3) (lseq-realize one23)))

(test-assert (lseq? '()))
(test-assert (lseq? '(1 2 3)))
(test-assert (lseq? (make-lseq 1 2 3)))
(test-assert (lseq? (cons 'x (lambda () 'x))))

(test-assert (lseq=? = '() '()))
(test-assert (lseq=? = '(1 2 3) '(1 2 3)))
(test-assert (lseq=? = (make-lseq 1 2 3) (make-lseq 1 2 3)))
(test-assert (lseq=? = (make-lseq 1 2 3) '(1 2 3)))

;[esl] car does not have to throw an exception
;(test-error (lseq-car (make-generator)))
(test 1 (lseq-car (make-lseq 1 2 3)))
(test 1 (lseq-car '(1 2 3)))
;[esl] car does not have to throw an exception
;(test-error (lseq-car 2))

;[esl] car does not have to throw an exception
;(test-error (lseq-first (make-generator)))
(test 1 (lseq-first (make-lseq 1 2 3)))
(test 1 (lseq-first '(1 2 3)))
;[esl] car does not have to throw an exception
;(test-error (lseq-first 2))

;[esl] car does not have to throw an exception
;(test-error (lseq-cdr (make-generator)))
(test 2 (lseq-cdr '(1 . 2)))
(test 2 (lseq-car (lseq-cdr '(1 2 3))))
(test 2 (lseq-car (lseq-cdr (make-lseq 1 2 3))))

;[esl] car does not have to throw an exception
;(test-error (lseq-rest (make-generator)))
(test 2 (lseq-rest '(1 . 2)))
(test 2 (lseq-car (lseq-rest '(1 2 3))))
(test 2 (lseq-car (lseq-rest (make-lseq 1 2 3))))
;[esl] car does not have to throw an exception
;(test-error (lseq-rest 2))

;[esl] car does not have to throw an exception
;(test-error (lseq-ref '() 0))
(test 1 (lseq-ref '(1) 0))
(test 2 (lseq-ref '(1 2) 1))
;[esl] car does not have to throw an exception
;(test-error (lseq-ref (make-lseq) 0))
(test 1 (lseq-ref (make-lseq 1) 0))
(test 1 (lseq-ref (make-lseq 1 2) 0))
(test 2 (lseq-ref (make-lseq 1 2) 1))

;[esl] car does not have to throw an exception
;(test-error (lseq-take '() 1))
;[esl] car does not have to throw an exception
;(test-error (lseq-take (make-lseq) 1))
(test-assert (procedure? (cdr (lseq-take '(1 2 3 4 5) 3)))) ; test laziness
(test '(1 2 3) (lseq-realize (lseq-take '(1 2 3 4 5) 3)))

;[esl] car does not have to throw an exception
;(test-error (lseq-drop '() 1))
;[esl] car does not have to throw an exception
;(test-error (lseq-drop (make-lseq 1) 2))
(test '(3 4 5) (lseq-realize (lseq-drop '(1 2 3 4 5) 2)))
(test '(3 4 5) (lseq-realize (lseq-drop (make-lseq 1 2 3 4 5) 2)))

(test '() (lseq-realize '()))
(test '(1 2 3) (lseq-realize '(1 2 3)))
(test '() (lseq-realize (make-lseq)))
(test '(1 2 3) (lseq-realize (make-lseq 1 2 3)))

(let ((g (lseq->generator '(1 2 3))))
  (test 1 (g))
  (test 2 (g))
  (test 3 (g))
  (test-assert (eof-object? (g))))

(let ((g (lseq->generator (make-lseq 1 2 3))))
  (test 1 (g))
  (test 2 (g))
  (test 3 (g))
  (test-assert (eof-object? (g))))

(test 0 (lseq-length '()))
(test 3 (lseq-length '(1 2 3)))
(test 3 (lseq-length (make-lseq 1 2 3)))

(test '(1 2 3 a b c) (lseq-realize (lseq-append '(1 2 3) '(a b c))))
(let ((one23abc (lseq-append (make-lseq 1 2 3) (make-lseq 'a 'b 'c))))
  (test-assert (procedure? (cdr one23abc)))
  (test-assert (lseq-realize one23abc)))

(let ((one2345 (make-lseq 1 2 3 4 5))
      (oddeven (make-lseq 'odd 'even 'odd 'even 'odd 'even 'odd 'even)))
  (test '((one 1 odd) (two 2 even) (three 3 odd))
      (lseq-realize (lseq-zip '(one two three) one2345 oddeven))))

(test '() (lseq-map - '()))
(test '(-1 -2 -3) (lseq-realize (lseq-map - '(1 2 3))))
(test '(-1 -2 -3) (lseq-realize (lseq-map - (make-lseq 1 2 3))))
(test-assert (procedure? (cdr (lseq-map - '(1 2 3)))))

(let* ((output '())
       (out! (lambda (x) (set! output (cons x output)))))
  (lseq-for-each out! '())
  (test '() output)
  (lseq-for-each out! '(a b c))
  (test '(c b a) output)
  (lseq-for-each out! (make-lseq 1 2 3))
  (test '(3 2 1 c b a) output))

(test '() (lseq-filter odd? '()))

(let ((odds (lseq-filter odd? '(1 2 3 4 5))))
  (test-assert (procedure? (cdr odds)))
  (test '(1 3 5) (lseq-realize odds))
  (test '(1 3 5) (lseq-realize (lseq-filter odd? (make-lseq 1 2 3 4 5)))))

(test '() (lseq-remove even? '()))
(let ((odds (lseq-remove even? '(1 2 3 4 5))))
  (test-assert (procedure? (cdr odds)))
  (test '(1 3 5) (lseq-realize odds))
  (test '(1 3 5) (lseq-realize (lseq-remove even? (make-lseq 1 2 3 4 5)))))

(test 4 (lseq-find even? '(3 1 4 1 5 9 2 6)))
(test 4 (lseq-find even? (make-lseq 3 1 4 1 5 9 2 6)))
(test #f (lseq-find negative? (make-lseq 1 2 3 4 5)))

(test '(-8 -5 0 0) (lseq-realize (lseq-find-tail even? '(3 1 37 -8 -5 0 0))))
(test '(-8 -5 0 0) (lseq-realize (lseq-find-tail even?
                                                 (make-lseq 3 1 37 -8 -5 0 0))))
(test #f (lseq-find-tail even? '()))
(test #f (lseq-find-tail negative? (make-lseq 1 2 3 4 5)))

(test '(2 18) (lseq-realize (lseq-take-while even? '(2 18 3 10 22 9))))
(test '(2 18) (lseq-realize (lseq-take-while even?
                                             (make-lseq 2 18 3 10 22 9))))
(test '(2 18) (lseq-realize (lseq-take-while even?
                                             (make-lseq 2 18 3 10 22 9))))

(test '(3 10 22 9) (lseq-drop-while even? '(2 18 3 10 22 9)))
(test '(3 10 22 9) (lseq-realize (lseq-drop-while even?
                                                  (make-lseq 2 18 3 10 22 9))))

(test #t (lseq-any integer? '(a 3 b 2.7)))
(test #t (lseq-any integer? (make-lseq 'a 3 'b 2.7)))
(test #f (lseq-any integer? '(a 3.1 b 2.7)))
(test #f (lseq-any integer? (make-lseq 'a 3.1 'b 2.7)))
(test #t (lseq-any < '(3 1 4 1 5) '(2 7 1 8 2)))
(test 6 (lseq-any factorial '(-1 -2 3 4)))
(test 6 (lseq-any factorial (make-lseq -1 -2 3 4)))

(test 24 (lseq-every factorial '(1 2 3 4)))
(test 24 (lseq-every factorial (make-lseq 1 2 3 4)))

(test 2 (lseq-index even? '(3 1 4 1 5 9)))
(test 1 (lseq-index < '(3 1 4 1 5 9 2 5 6) '(2 7 1 8 2)))
(test #f (lseq-index = '(3 1 4 1 5 9 2 5 6) '(2 7 1 8 2)))

(test '(a b c) (lseq-realize (lseq-memq 'a '(a b c))))
(test '(a b c) (lseq-realize (lseq-memq 'a (make-lseq 'a 'b 'c))))
(test #f (lseq-memq 'a (make-lseq 'b 'c 'd)))
(test #f (lseq-memq (list 'a) '(b c d)))
(test #f (lseq-memq (list 'a) (make-lseq 'b 'c 'd)))

(test '(101 102) (lseq-realize (lseq-memv 101 (make-lseq 100 101 102))))

(test '((a) c) (lseq-realize (lseq-member (list 'a) (make-lseq 'b '(a) 'c))))
(test '(2 3) (lseq-realize (lseq-member 2.0 (make-lseq 1 2 3) =)))

(test-end)
