;; Copyright (C) Marc Nieper-Wißkirchen (2017).  All Rights
;; Reserved.

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

(define-library (srfi 153)
  (export oset oset/ordered oset-unfold oset-unfold/ordered
	  oset-accumulate
	  oset? oset-contains? oset-empty? oset-disjoint?
	  oset-member oset-element-comparator
	  oset-adjoin oset-adjoin/replace oset-delete oset-delete-all
          oset-pop oset-pop/reverse
	  oset-size oset-find oset-count oset-any? oset-every?
	  oset-map oset-map/monotone oset-for-each oset-fold oset-fold/reverse
	  oset-filter oset-remove oset-partition
	  oset->list list->oset list->oset/ordered
	  oset=? oset<? oset>? oset<=? oset>=?
	  oset-union oset-intersection oset-difference oset-xor
          oset-min-element oset-max-element
          oset-element-predecessor oset-element-successor
          oset-range= oset-range< oset-range> oset-range<= oset-range>=
          oset-split oset-catenate)

;; Specialized obag functions (not implemented)
;	  obag-sum obag-product
;	  obag-unique-size obag-element-count obag-for-each-unique obag-fold-unique
  (import (scheme base)
	  (scheme case-lambda)
	  (scheme char)
          (srfi 128)
	  (srfi 146))
  
(begin
;;;; Implementation for SRFI 153 atop SRFI 146 (mappings)

(define-record-type <oset>
  (S mapping)   ; S constructs oset
  oset?
  (mapping M))  ; M extracts mapping

(define (list->oset comp elems)
  (S (alist->mapping comp (map (lambda (elem) (cons elem 1)) elems))))

(define (list->oset/ordered comp elems)
  (S (alist->mapping/ordered comp (map (lambda (elem) (cons elem 1)) elems))))

(define (oset comp . elems)
  (list->oset comp elems))

(define (oset/ordered comp . elems)
  (list->oset/ordered comp elems))

(define (oset-unfold stop? mapper successor seed comp)
  (S (mapping-unfold stop? (lambda (seed) (values (mapper seed) 1))
    successor seed comp)))

(define (oset-unfold/ordered stop? mapper successor seed comp)
  (S (mapping-unfold/ordered stop? (lambda (seed) (values (mapper seed) 1))
    successor seed comp)))

(define (oset-accumulate proc comp . seeds)
  (call-with-current-continuation
   (lambda (return)
     (let loop ((m (mapping comp)) (seeds seeds))
       (let-values (((e . seeds*)
                     (apply proc
                            (lambda objs (apply return (S m) objs))
                            seeds)))
         (loop (mapping-adjoin m e 1) seeds*))))))

(define (oset-contains? oset elem)
  (mapping-contains? (M oset) elem))

(define (oset-empty? oset)
  (mapping-empty? (M oset)))

(define (oset-disjoint? oset1 oset2)
  (mapping-disjoint? (M oset1) (M oset2)))

(define (oset-member oset elem default)
  (if (oset-contains? oset elem) elem default))

(define (oset-element-comparator oset)
  (mapping-key-comparator (M oset)))

(define (alternate keys value)
  (define (alternate-acc keys result)
    (if (null? keys)
      (reverse result)
      (alternate-acc
         (cdr keys)
         (cons value (cons (car keys) result)))))
  (alternate-acc keys '()))

(define (oset-adjoin oset . elems)
  (S (apply mapping-adjoin (M oset) (alternate elems 1))))

;; Inefficient; needs two traversals.
(define (oset-adjoin/replace oset . elements)
  (apply oset-adjoin (apply oset-delete oset elements) elements))

(define (oset-delete oset . elems)
  (oset-delete-all oset elems))

(define (oset-delete-all oset elem-list)
  (S (mapping-delete-all (M oset) elem-list)))

(define failval (list 'failval))

(define oset-pop
  (case-lambda
    ((oset)
       (oset-pop oset (lambda () (error "oset-pop failure"))))
    ((oset fail)
       (let-values (((mapping key value)
                     (mapping-pop
		       (M oset)
		       (lambda () (values failval failval failval)))))
       (if (eq? key failval)
          (fail)
	  (values (S mapping) key))))))

;; Inefficient; needs two traversals.
(define oset-pop/reverse
  (case-lambda
    ((oset)
       (oset-pop/reverse oset (lambda () (error "oset-pop/reverse failure"))))
    ((oset fail)
     ;; Catch a possible error from mapping-max-key. Unfortunately,
     ;; this also catches everything else.
     (guard (junk (else (fail)))
       (let* ((m (M oset))
              (high (mapping-max-key m)))
         (values (S (mapping-range< m high)) high))))))

(define (oset-size oset)
  (mapping-size (M oset)))

(define (oset-find pred oset failure)
  (let-values (((key value)
                (mapping-find
                  (lambda (key value) (pred key))
                  (M oset)
                  (lambda () (values failval failval)))))
    (if (eq? key failval)
      (failure)
      key)))

(define (oset-count pred oset)
  (mapping-count (lambda (key value) (pred key)) (M oset)))

(define (oset-any? pred oset)
  (mapping-any? (lambda (key value) (pred key)) (M oset)))

(define (oset-every? pred oset)
  (mapping-every? (lambda (key value) (pred key)) (M oset)))

(define (oset-map proc comparator oset)
  (S (mapping-map (lambda (key value) (values (proc key) value))
                 comparator (M oset))))

(define (oset-map/monotone proc comparator oset)
  (S (mapping-map/monotone (lambda (key value) (values (proc key) value))
                 comparator (M oset))))

(define (oset-for-each proc oset)
  (mapping-for-each (lambda (key value) (values (proc key) value)) (M oset)))

(define (oset-fold proc nil oset)
  (mapping-fold (lambda (key value acc) (proc key acc)) nil (M oset)))

(define (oset-fold/reverse proc nil oset)
  (mapping-fold/reverse (lambda (key value acc) (proc key acc)) nil (M oset)))

(define (oset-filter pred oset)
  (S (mapping-filter (lambda (key value) (pred key)) (M oset))))

(define (oset-remove pred oset)
  (S (mapping-remove (lambda (key value) (pred key)) (M oset))))

(define (oset-partition pred oset)
  (let-values (((in out)
                (mapping-partition
                  (lambda (key value) (pred key))
                  (M oset))))
    (values (S in) (S out))))

(define (oset->list oset)
  (map car (mapping->alist (M oset))))

(define (oset=? . osets)
  (S (apply mapping=? (map M osets))))

(define (oset<? . osets)
  (S (apply mapping<? (map M osets))))

(define (oset>? . osets)
  (S (apply mapping>? (map M osets))))

(define (oset<=? . osets)
  (S (apply mapping<=? (map M osets))))

(define (oset>=? . osets)
  (S (apply mapping>=? (map M osets))))

(define (oset-union . osets)
  (S (apply mapping-union (map M osets))))

(define (oset-intersection . osets)
  (S (apply mapping-intersection (map M osets))))

(define (oset-difference . osets)
  (S (apply mapping-difference (map M osets))))

(define (oset-xor oset1 oset2)
  (S (mapping-xor (M oset1) (M oset2))))

(define (oset-min-element oset)
  (mapping-min-key (M oset)))

(define (oset-max-element oset)
  (mapping-max-key (M oset)))

(define (oset-element-predecessor oset obj failure)
  (mapping-key-predecessor (M oset) obj failure))

(define (oset-element-successor oset obj failure)
  (mapping-key-successor (M oset) obj failure))

(define (oset-range= oset obj)
  (S (mapping-range= (M oset) obj)))

(define (oset-range< oset obj)
  (S (mapping-range< (M oset) obj)))

(define (oset-range> oset obj)
  (S (mapping-range> (M oset) obj)))

(define (oset-range<= oset obj)
  (S (mapping-range<= (M oset) obj)))

(define (oset-range>= oset obj)
  (S (mapping-range>= (M oset) obj)))

(define (oset-split oset obj)
  (S (mapping-split (M oset) obj)))

(define (oset-catenate oset obj)
  (S (mapping-catenate (M oset) obj)))
))
