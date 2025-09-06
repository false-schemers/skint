
;[esl] adapted from the reference implementation by Daphne Preston-Kendal

(define-library (srfi 228)
  (import (scheme base)
	  (srfi 1)
          (srfi 128)
	  (srfi 151))
  (export make-wrapper-comparator
          make-product-comparator
          make-sum-comparator
          comparator-one
          comparator-zero)
(begin

(define (make-wrapper-comparator type-test unwrap contents-comparator)
  (make-comparator
   type-test
   (lambda (a b)
     ((comparator-equality-predicate contents-comparator)
      (unwrap a)
      (unwrap b)))
   (if (comparator-ordering-predicate contents-comparator)
       (lambda (a b)
         ((comparator-ordering-predicate contents-comparator)
          (unwrap a)
          (unwrap b)))
       #f)
  (if (comparator-hash-function contents-comparator)
      (lambda (x)
        ((comparator-hash-function contents-comparator) x))
      #f)))

(define (make-product-comparator . comparators)
  (if (null? comparators)
      comparator-one
      (let* ((type-tests
              (delete-duplicates
               (map comparator-type-test-predicate comparators)
               eq?))
             (type-test
              (lambda (val)
                (every (lambda (test) (test val)) type-tests))))
        (make-comparator
         type-test
         (lambda (a b)
           (every (lambda (cmp)
                    ((comparator-equality-predicate cmp) a b))
                  comparators))
         (if (every comparator-ordered? comparators)
             (lambda (a b)
               (let loop ((cmps comparators))
                 (cond ((null? cmps) #f)
                       (((comparator-ordering-predicate (car cmps)) a b) #t)
                       (((comparator-equality-predicate (car cmps)) a b) (loop (cdr cmps)))
                       (else #f))))
             #f)
         (if (every comparator-hashable? comparators)
             (lambda (x)
               (fold bitwise-xor
                     0
                     (map (lambda (cmp)
                            ((comparator-hash-function cmp) x))
                          comparators)))
             #f)))))

(define (comparator-index comparators val)
  (list-index
   (lambda (cmp)
     ((comparator-type-test-predicate cmp) val))
   comparators))

(define (make-sum-comparator . comparators)
  (if (null? comparators)
      comparator-zero
      (make-comparator
       (lambda (x)
         (any
          (lambda (cmp)
            ((comparator-type-test-predicate cmp) x))
          comparators))
       (lambda (a b)
         (let ((a-cmp-idx (comparator-index comparators a))
               (b-cmp-idx (comparator-index comparators b)))
           (if (not (= a-cmp-idx b-cmp-idx))
               #f
               (let ((cmp (list-ref comparators a-cmp-idx)))
                 ((comparator-equality-predicate cmp) a b)))))
       (if (every comparator-ordered? comparators)
           (lambda (a b)
             (let ((a-cmp-idx (comparator-index comparators a))
                   (b-cmp-idx (comparator-index comparators b)))
               (cond ((< a-cmp-idx b-cmp-idx) #t)
                     ((> a-cmp-idx b-cmp-idx) #f)
                     (else
                      (let ((cmp (list-ref comparators a-cmp-idx)))
                        ((comparator-ordering-predicate cmp) a b))))))
           #f)
       (if (every comparator-hashable? comparators)
           (lambda (x)
             (let ((cmp (find (lambda (cmp) ((comparator-type-test-predicate cmp) x))
                              comparators)))
               ((comparator-hash-function cmp) x)))
           #f))))

(define comparator-one
  (make-comparator
   (lambda (x) #t)
   (lambda (a b) #t)
   (lambda (a b) #f)
   (lambda (x) 0)))

(define comparator-zero
  (make-comparator
   (lambda (x) #f)
   (lambda (a b) (error "can't compare" a b))
   (lambda (a b) (error "can't compare" a b))
   (lambda (x) (error "can't hash" x))))

))
