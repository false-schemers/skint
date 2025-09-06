(import (srfi 128) (srfi 153))
(include "test.scm")

(define (oset-print o)
  (write (oset->list o))
  (newline))

(define default-comparator (make-default-comparator))

(define number-comparator
  (make-comparator number? = < #f))

(define string-ci-comparator
  (make-comparator string? string-ci=? string-ci<? #f))

(define string-comparator
  (make-comparator string? string=? string<? #f))

; [esl-] eq-based tests commented out: SRFI 128 does not guarantee
; eq comparators having a working ordering predicate, to quote:
; "The ordering function is implementation-defined, except that it must 
; conform to the rules for ordering functions. It may signal an error instead"
;(define eq-comparator (make-eq-comparator))

(define vlist '())

(define (failure) 'fail)

(define default-test-equal? (current-test-comparator))

(define (oset-equal? x y)
  (cond
    ((and (oset? x) (oset? y)) (oset=? x y))
    ((oset? x) #f)
    ((oset? y) #f)
    (else (default-test-equal? x y))))

(current-test-comparator oset-equal?)

; oset0 = {}
(define oset0 (oset number-comparator))

; oset1 = oset2 = {1, 2, 3, 4, 5} both settable
(define oset1 (oset number-comparator 5 4 3 2 1))
(define oset2 (oset/ordered number-comparator 1 2 3 4 5))

; oset3 = oset4 = {100, 200, 300, 400, 500}
; oset3 settable, oset4 not settable
(define oset3 (oset-unfold
                (lambda (x) (< x 1))
                (lambda (x) (* x 100))
                (lambda (x) (- x 1))
                5 number-comparator))


(define oset4 (oset-unfold/ordered
                (lambda (x) (= x 5))
                (lambda (x) (* x 100))
                (lambda (x) (+ x 1))
                1 number-comparator))

; oset5 = {"a", "b", "c", "d", "e"} case insensitive, settable
; oset5a = {"A", "b", "c", "d", "e"} case insensitive, settable
; oset5z = {"A", "b", "c", "d", "e", "Z"} case insensitive, settable
(define oset5 (oset string-ci-comparator "a" "b" "c" "d" "e"))
(define oset5a (oset string-ci-comparator "A" "b" "c" "d" "e"))
(define oset5z (oset string-ci-comparator "A" "b" "c" "d" "e" "Z"))

; oset6 = {1, 2, 3, 4, 5} not settable
; oset6s = {1, 2, 3, 4} not settable
; oset6l = {1, 2, 3, 4, 5, 6} not settable
; oset6e = {2, 4} not settable
; oset6o = {1, 3, 5} not settable
; oset6l = {1, 2} not settable
; oset6m = {3} not settable
; oset6h = {4, 5} not settable
; oset6lm = {1, 2, 3} not settable
; oset6mh = {3, 4, 5} not settable
(define oset6 (oset number-comparator 1 2 3 4 5))
(define oset6s (oset number-comparator 1 2 3 4))
(define oset6l (oset number-comparator 1 2 3 4 5 6))
(define oset6e (oset number-comparator 2 4))
(define oset6o (oset number-comparator 1 3 5))
(define oset6l (oset number-comparator 1 2))
(define oset6m (oset number-comparator 3))
(define oset6h (oset number-comparator 4 5))
(define oset6lm (oset number-comparator 1 2 3))
(define oset6mh (oset number-comparator 3 4 5))

; oset7 = {"a", "b", "c", "d", "e"} not settable
(define oset7 (oset string-comparator "a" "b" "c" "d" "e"))

; oset8 = {"a", "b", "c", "d", "e", 1, 2, 3, 4, 5}
(define oset8 (oset default-comparator "a" "b" "c" "d" "e" 1 2 3 4 5))

; oset9 = {1, 2}
(define oset9 (oset number-comparator 1 2))

(test-begin "srfi-153")

;; Constructors

(test-assert (oset-contains? oset1 1))
(test-assert (oset-contains? oset1 2))
(test-assert (oset-contains? oset1 3))
(test-assert (oset-contains? oset1 4))
(test-assert (oset-contains? oset1 5))
(test 5 (oset-size oset1))

(test-assert (oset-contains? oset2 1))
(test-assert (oset-contains? oset2 2))
(test-assert (oset-contains? oset2 3))
(test-assert (oset-contains? oset2 4))
(test-assert (oset-contains? oset2 5))
(test 5 (oset-size oset2))

(test oset3 (oset number-comparator 100 200 300 400 500))
(test oset4 (oset number-comparator 100 200 300 400 500))
(test-assert (oset-contains? oset5 "a"))
(test-assert (oset-contains? oset5 "A"))
(test 10 (oset-size oset8))

(test oset3 (oset-accumulate
             (lambda (terminate i)
               (if (> i 500)
                   (terminate)
                   (values i (+ i 100))))
             number-comparator
             100))

(test-assert
  (let-values (((set last)
                (oset-accumulate
                 (lambda (terminate i)
                   (if (< i 1)
                       (terminate i)
                       (values (* i 100) (- i 1))))
                 number-comparator
                 5)))
    (and (oset=? set oset4) (zero? last))))


;; Predicates

(test-assert (oset? oset1))
(test-assert (oset? oset2))
(test-assert (oset? oset3))
(test-assert (oset? oset4))
(test-assert (oset? oset5))

(test-assert (oset-contains? oset1 3))
(test-not (oset-contains? oset1 10))

(test-assert (oset-empty? oset0))
(test-not (oset-empty? oset1))

(test-not (oset-disjoint? oset1 oset2))
(test-not (oset-disjoint? oset1 oset2))

;; Accessors

(test 1 (oset-member oset1 1 (failure)))
(test 'fail (oset-member oset1 100 (failure)))
(test-equal string-ci=? "a" (oset-member oset5 "A" (failure)))
(test 'fail (oset-member oset5 "z" (failure)))

(test-equal eq? number-comparator (oset-element-comparator oset4))

;; Updaters

(test (oset number-comparator 1 2 3 4 5 6 7) (oset-adjoin oset1 6 7))
; oset2 = {1, 2, 3, 4, 5, 6, 7}
(set! oset2 (oset-adjoin oset2 6 7))
(test oset2 (oset number-comparator 1 2 3 4 5 6 7))
(test oset5 (oset-adjoin oset5 "A"))
(test oset5a (oset-adjoin/replace oset5 "A"))
(test oset5z (oset-adjoin/replace oset5 "A" "Z"))

; oset5 = {"A", "b", "c", "d", "e"}
(test oset5 (oset string-ci-comparator "A" "b" "c" "d" "e"))

(test (oset number-comparator 1 2 3) (oset-delete oset1 4 5))
(test (oset number-comparator 1 2 3) (oset-delete-all oset1 (list 4 5)))
; oset1 = {1, 2, 3, 4}
(set! oset1 (oset-delete oset1 5))
(test oset1 (oset number-comparator 1 2 3 4))
; oset1 = {1, 2, 3}
(set! oset1 (oset-delete-all oset1 (list 4)))
(test oset1 (oset number-comparator 1 2 3))

(test 'fail (oset-pop oset0 failure))
(test 'fail (oset-pop/reverse oset0 failure))

(test-assert
  (let-values (((o x) (oset-pop oset2 failure)))
    (and (oset=? o (oset number-comparator 2 3 4 5 6 7))
         (= x 1))))

(test-assert
  (let-values (((o x) (oset-pop/reverse oset2 failure)))
    (and (oset=? o (oset number-comparator 1 2 3 4 5 6))
         (= x 7))))

(set! vlist (call-with-values (lambda () (oset-pop oset2 failure)) list))
; oset2 = {2, 3, 4, 5, 6, 7}
(set! oset2 (car vlist))
(test oset2 (oset number-comparator 2 3 4 5 6 7))
(test 1 (cadr vlist))

;; The whole oset
(test 5 (oset-size oset6))
(test 1 (oset-find odd? oset6 failure))
(test 'fail (oset-find zero? oset6 failure))

(test 2 (oset-count even? oset6))
(test 0 (oset-count zero? oset6))

(test-assert (oset-any? odd? (oset number-comparator 1 2 4 6 8)))
(test-assert (oset-every? even? (oset number-comparator 2 4 6 8)))

;; Mapping and folding

;(test oset7 (oset-map symbol->string string-ci-comparator (oset eq-comparator 'a 'b 'c 'd 'e)))
;(test oset7 (oset-map/monotone symbol->string string-ci-comparator (oset eq-comparator 'a 'b 'c 'd 'e)))

(test '(5 4 3 2 1)
      (let ((r '()))
        (oset-for-each
          (lambda (i) (set! r (cons i r)))
          oset6)
	r))

(test 15 (oset-fold + 0 oset6))
(test 1 (oset-fold - 0 oset9))
(test -1 (oset-fold/reverse - 0 oset9))
(test "edcba" (oset-fold string-append "" oset7))
(test oset6 (oset-filter number? oset8))
(test oset7 (oset-remove number? oset8))
(set! vlist
  (call-with-values
    (lambda () (oset-partition number? oset8))
    list))

(test-assert
  (and (oset=? (car vlist) oset6)
       (oset=? (cadr vlist) oset7)))

(test '(100 200 300 400 500) (oset->list oset3))
(test oset3 (list->oset number-comparator '(100 200 300 400 500)))
(test oset3 (list->oset/ordered number-comparator '(100 200 300 400 500)))

(test-assert (oset=? oset6 oset6))
(test-assert (oset<? oset6s oset6))
(test-assert (oset>? oset6 oset6l))
(test-assert (oset<=? oset6s oset6))
(test-assert (oset<=? oset6s oset6s))
(test-assert (oset>=? oset6 oset6l))
(test-assert (oset>=? oset6l oset6l))

(test oset6 (oset-union oset6e oset6o))
(test oset0 (oset-intersection oset6e oset6o))
(test oset6e (oset-difference oset6 oset6o))
(test (oset number-comparator 3 4 5 6)
  (oset-xor (oset number-comparator 1 2 3 4)
              (oset number-comparator 1 2 5 6)))

(test 1 (oset-min-element oset6))
(test 5 (oset-max-element oset6))
(test 1 (oset-element-predecessor oset6 2 failure))
(test 'fail (oset-element-predecessor oset6 1 failure))
(test 5 (oset-element-successor oset6 4 failure))
(test 'fail (oset-element-successor oset6 5 failure))

(test oset6l (oset-range< oset6 3))
(test oset6m (oset-range= oset6 3))
(test oset6h (oset-range> oset6 3))
(test oset6lm (oset-range<= oset6 3))
(test oset6mh (oset-range>= oset6 3))

(test-end)