(import (srfi 131) (rename (srfi 136) (define-record-type define-record-type/136)))
(include "test.scm")

(test-begin "srfi-131")

; [esl] adapted from tests by Marc Nieper-Wißkirchen

;; Copyright (C) Marc Nieper-Wißkirchen (2016).  All Rights Reserved. 

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

(test-assert ;"Predicate"
  (let ()
    (define-record-type <record>
      (make-record)
      record?)
    (record? (make-record))))
  
(test-assert ;"Disjoint type"
  (let ()
    (define-record-type <record>
      (make-record)
      record?)
    (not (vector? (make-record)))))

(test-equal ;"Record fields"
  '(1 2 3)
  (let ()
    (define-record-type <record>
      (make-record foo baz)
      record?
      (foo foo)
      (bar bar set-bar!)
      (baz baz))
    (define record (make-record 1 3))
    (set-bar! record 2)
    (list (foo record) (bar record) (baz record))))

(test-equal ;"Subtypes"
  '(#t #f)
  (let ()
    (define-record-type <parent>
      (make-parent)
      parent?)
    (define-record-type (<child> <parent>)
      (make-child)
      child?)
    (list (parent? (make-child)) (child? (make-parent)))))

(test-equal ;"Inheritance of constructor"
  '(1 2)
  (let ()
    (define-record-type <parent>
      (make-parent foo)
      parent?
      (foo foo))
    (define-record-type (<child> <parent>)
      (make-child foo bar)
      child?
      (bar bar))
    (define child (make-child 1 2))
    (list (foo child) (bar child))))

(test-equal ;"Default constructors"
  1
  (let ()
    (define-record-type <parent>
      (make-parent foo)
      #f
      (foo foo))
    (define-record-type (<child> <parent>)
      make-child
      child?)
    (define child (make-child 1))
    (foo child)))
  (test-assert ;"Missing parent"
   (let ()
     (define-record-type (<record> #f)
       (make-record)
       record?)
     (record? (make-record))))

(test-equal ;"Interoperability with SRFI 136"
  '(#t #t 1 2)
  (let ()
    (define-record-type/136 <parent>
      (make-parent foo)
      parent?
      (foo foo))
    (define-record-type (<child> <parent>)
      (make-child foo bar)
      child?
      (bar bar))
    (define record (make-child 1 2))
    (list (parent? record)
	  (child? record)
	  (foo record)
	  (bar record))))

(test-end)

