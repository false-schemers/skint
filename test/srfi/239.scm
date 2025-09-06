(import (srfi 239))
(include "test.scm")

;[esl] adapted from  Marc Nieper-Wißkirchen's tests
(test-begin "srfi-239")

;; Copyright (C) Marc Nieper-Wißkirchen (2021).  All Rights Reserved.

;; Permission is hereby granted, free of charge, to any person
;; obtaining a copy of this software and associated documentation
;; files (the "Software"), to deal in the Software without
;; restriction, including without limitation the rights to use, copy,
;; modify, merge, publish, distribute, sublicense, and/or sell copies
;; of the Software, and to permit persons to whom the Software is
;; furnished to do so, subject to the following conditions:

;; The above copyright notice and this permission notice shall be
;; included in all copies or substantial portions of the Software.

;; THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
;; EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
;; MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
;; NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
;; BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
;; ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
;; CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
;; SOFTWARE.

(define type-of
  (lambda (x)
    (list-case x
      [(_ . _) 'pair]
      [() 'null]
      [_ 'atom])))

(test-assert (eq? 'pair (type-of '(a . b))))
(test-assert (eq? 'null (type-of '())))
(test-assert (eq? 'atom (type-of 'x)))

(define fold
  (lambda (proc seed ls)
    (let f ([acc seed] [ls ls])
      (list-case ls
        [(h . t) (f (proc h acc) t)]
        [() acc]
        [_ (error "fold: not a list" ls)])))) ;[esl*] -- no assertion-violation in R7RS

(test-equal '(3 2 1 . 0)
  (fold cons 0 '(1 2 3)))

(test-assert 
  (guard (exc
          [(error-object? exc) #t] ;[esl*] -- no assertion-violation? in R7RS
          [else #f])
   (fold cons 0 '(1 2 . 3))))

(test-assert 
  (list-case '(1 . 2)
    [_ #f]
    [(_ . _) #t]))

(test-assert 
  (guard (exc
           [(error-object? exc) #t] ;[esl*] -- no assertion-violation? in R7RS
           [else #f])
    (list-case 0
      [(_ . _) #f]
      [() #f])))

(test-end)

