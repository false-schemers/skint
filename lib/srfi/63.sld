(define-library (srfi 63)

  (export array?
          ;[esl] : this one is supposed to override equal? originally,
          ; but we REALLY can't replace equal?!!! Skint's equal? compares records
          ; structurally, so it can compare array records as expected
          equal? ; just re-exported from (scheme base)
          array-rank
          array-dimensions
          make-array
          make-shared-array
          list->array
          array->list
          vector->array
          array->vector
          array-in-bounds?
          array-ref
          array-set!
          A:floC128b
          A:floC64b
          A:floC32b
          A:floC16b
          A:floR128b
          A:floR64b
          A:floR32b
          A:floR16b
          A:floQ128d
          A:floQ64d
          A:floQ32d
          A:fixZ64b
          A:fixZ32b
          A:fixZ16b
          A:fixZ8b
          A:fixN64b
          A:fixN32b
          A:fixN16b
          A:fixN8b
          A:bool)

  (import (scheme base))

(begin

;;; Conflicts with (scheme base): equal?
;;; Modifications by ESL are marked by ;[esl].


;;;;"array.scm" Arrays for Scheme
; Copyright (C) 2001, 2003 Aubrey Jaffer
;
;Permission to copy this software, to modify it, to redistribute it,
;to distribute modified versions, and to use it for any purpose is
;granted, subject to the following restrictions and understandings.
;
;1.  Any copy made of this software must include this copyright notice
;in full.
;
;2.  I have made no warranty or representation that the operation of
;this software will be error-free, and I am under no obligation to
;provide any services, by way of maintenance, update, or otherwise.
;
;3.  In conjunction with products arising from the use of this
;material, there shall be no use of my name in any advertising,
;promotional, or sales literature without prior written consent in
;each case.

;;@code{(require 'array)} or @code{(require 'srfi-63)}
;;@ftindex array

(define-record-type <array>
  (construct dimensions scales offset store)
  array:array?
  (dimensions dimensions)
  (scales scales)
  (offset offset)
  (store store))

(define (array:dimensions array)
  (cond ((vector? array) (list (vector-length array)))
        ((string? array) (list (string-length array)))
        (else (dimensions array))))

(define (array:scales array)
  (cond ((vector? array) '(1))
        ((string? array) '(1))
        (else (scales array))))

(define (array:store array)
  (cond ((vector? array) array)
        ((string? array) array)
        (else (store array))))

(define (array:offset array)
  (cond ((vector? array) 0)
        ((string? array) 0)
        (else (offset array))))

;[esl] keep simple 1-dimensional arrays in their plain form
(define (array:construct dimensions scales offset store)
  (if (and (= (length dimensions) 1) (equal? scales '(1)) (= offset 0))
      store
      (construct dimensions scales offset store)))

;;@args obj
;;Returns @code{#t} if the @1 is an array, and @code{#f} if not.
(define (array? obj)
  (or (vector? obj) (string? obj) (array:array? obj)))

;;@noindent
;;@emph{Note:} Arrays are not disjoint from other Scheme types.
;;Vectors and possibly strings also satisfy @code{array?}.
;;A disjoint array predicate can be written:
;;
;;@example
;;(define (strict-array? obj)
;;  (and (array? obj) (not (string? obj)) (not (vector? obj))))
;;@end example

;;@body
;;Returns @code{#t} if @1 and @2 have the same rank and dimensions and the
;;corresponding elements of @1 and @2 are @code{equal?}.

;;@body
;;@0 recursively compares the contents of pairs, vectors, strings, and
;;@emph{arrays}, applying @code{eqv?} on other objects such as numbers
;;and symbols.  A rule of thumb is that objects are generally @0 if
;;they print the same.  @0 may fail to terminate if its arguments are
;;circular data structures.
;;
;;@example
;;(equal? 'a 'a)                             @result{}  #t
;;(equal? '(a) '(a))                         @result{}  #t
;;(equal? '(a (b) c)
;;        '(a (b) c))                        @result{}  #t
;;(equal? "abc" "abc")                       @result{}  #t
;;(equal? 2 2)                               @result{}  #t
;;(equal? (make-vector 5 'a)
;;        (make-vector 5 'a))                @result{}  #t
;;(equal? (make-array (A:fixN32b 4) 5 3)
;;        (make-array (A:fixN32b 4) 5 3))    @result{}  #t
;;(equal? (make-array '#(foo) 3 3)
;;        (make-array '#(foo) 3 3))          @result{}  #t
;;(equal? (lambda (x) x)
;;        (lambda (y) y))                    @result{}  @emph{unspecified}
;;@end example
;[esl] : this one is supposed to override equal? originally,
; but we REALLY can't replace equal?!!! Skint's equal? compares records
; structurally, so it can compare array records as expected
#|
(define (equal? obj1 obj2)
  (cond ((eqv? obj1 obj2) #t)
        ((or (pair? obj1) (pair? obj2))
         (and (pair? obj1) (pair? obj2)
              (equal? (car obj1) (car obj2))
              (equal? (cdr obj1) (cdr obj2))))
        ((or (string? obj1) (string? obj2))
         (and (string? obj1) (string? obj2)
              (string=? obj1 obj2)))
        ((or (vector? obj1) (vector? obj2))
         (and (vector? obj1) (vector? obj2)
              (equal? (vector-length obj1) (vector-length obj2))
              (do ((idx (+ -1 (vector-length obj1)) (+ -1 idx)))
                  ((or (negative? idx)
                       (not (equal? (vector-ref obj1 idx)
                                    (vector-ref obj2 idx))))
                   (negative? idx)))))
        ((or (array? obj1) (array? obj2))
         (and (array? obj1) (array? obj2)
              (equal? (array:dimensions obj1) (array:dimensions obj2))
              (equal? (array:store obj1) (array:store obj2))))
        (else #f)))
|#

;;@body
;;Returns the number of dimensions of @1.  If @1 is not an array, 0 is
;;returned.
(define (array-rank obj)
  (if (array? obj) (length (array:dimensions obj)) 0))

;;@args array
;;Returns a list of dimensions.
;;
;;@example
;;(array-dimensions (make-array '#() 3 5))
;;   @result{} (3 5)
;;@end example
(define array-dimensions array:dimensions)

;;@args prototype k1 @dots{}
;;
;;Creates and returns an array of type @1 with dimensions @2, @dots{}
;;and filled with elements from @1.  @1 must be an array, vector, or
;;string.  The implementation-dependent type of the returned array
;;will be the same as the type of @1; except if that would be a vector
;;or string with rank not equal to one, in which case some variety of
;;array will be returned.
;;
;;If the @1 has no elements, then the initial contents of the returned
;;array are unspecified.  Otherwise, the returned array will be filled
;;with the element at the origin of @1.
(define (make-array prototype . dimensions)
  (define tcnt (apply * dimensions))
  (let ((store
         (if (string? prototype)
             (case (string-length prototype)
               ((0) (make-string tcnt))
               (else (make-string tcnt
                                  (string-ref prototype 0))))
             (let ((pdims (array:dimensions prototype)))
               (case (apply * pdims)
                 ((0) (make-vector tcnt))
                 (else (make-vector tcnt
                                    (apply array-ref prototype
                                           (map (lambda (x) 0) pdims)))))))))
    (define (loop dims scales)
      (if (null? dims)
          (array:construct dimensions (cdr scales) 0 store)
          (loop (cdr dims) (cons (* (car dims) (car scales)) scales))))
    (loop (reverse dimensions) '(1))))
;;@args prototype k1 @dots{}
;;@0 is an alias for @code{make-array}.
(define create-array make-array)

;;@args array mapper k1 @dots{}
;;@0 can be used to create shared subarrays of other
;;arrays.  The @var{mapper} is a function that translates coordinates in
;;the new array into coordinates in the old array.  A @var{mapper} must be
;;linear, and its range must stay within the bounds of the old array, but
;;it can be otherwise arbitrary.  A simple example:
;;
;;@example
;;(define fred (make-array '#(#f) 8 8))
;;(define freds-diagonal
;;  (make-shared-array fred (lambda (i) (list i i)) 8))
;;(array-set! freds-diagonal 'foo 3)
;;(array-ref fred 3 3)
;;   @result{} FOO
;;(define freds-center
;;  (make-shared-array fred (lambda (i j) (list (+ 3 i) (+ 3 j)))
;;                     2 2))
;;(array-ref freds-center 0 0)
;;   @result{} FOO
;;@end example
(define (make-shared-array array mapper . dimensions)
  (define odl (array:scales array))
  (define rank (length dimensions))
  (define shape
    (map (lambda (dim) (if (list? dim) dim (list 0 (+ -1 dim)))) dimensions))
  (do ((idx (+ -1 rank) (+ -1 idx))
       (uvt (append (cdr (vector->list (make-vector rank 0))) '(1))
            (append (cdr uvt) '(0)))
       (uvts '() (cons uvt uvts)))
      ((negative? idx)
       (let ((ker0 (apply + (map * odl (apply mapper uvt)))))
         (array:construct
          (map (lambda (dim) (+ 1 (- (cadr dim) (car dim)))) shape)
          (map (lambda (uvt) (- (apply + (map * odl (apply mapper uvt))) ker0))
               uvts)
          (apply +
                 (array:offset array)
                 (map * odl (apply mapper (map car shape))))
          (array:store array))))))

;;@args rank proto list
;;@3 must be a rank-nested list consisting of all the elements, in
;;row-major order, of the array to be created.
;;
;;@0 returns an array of rank @1 and type @2 consisting of all the
;;elements, in row-major order, of @3.  When @1 is 0, @3 is the lone
;;array element; not necessarily a list.
;;
;;@example
;;(list->array 2 '#() '((1 2) (3 4)))
;;                @result{} #2A((1 2) (3 4))
;;(list->array 0 '#() 3)
;;                @result{} #0A 3
;;@end example
(define (list->array rank proto lst)
  (define dimensions
    (do ((shp '() (cons (length row) shp))
         (row lst (car lst))
         (rnk (+ -1 rank) (+ -1 rnk)))
        ((negative? rnk) (reverse shp))))
  (let ((nra (apply make-array proto dimensions)))
    (define (l2ra dims idxs row)
      (cond ((null? dims)
             (apply array-set! nra row (reverse idxs)))
            (else
             (if (not (eqv? (car dims) (length row)))
                 (error "Array not rectangular:" dims dimensions))
             (do ((idx 0 (+ 1 idx))
                  (row row (cdr row)))
                 ((>= idx (car dims)))
               (l2ra (cdr dims) (cons idx idxs) (car row))))))
    (l2ra dimensions '() lst)
    nra))

;;@args array
;;Returns a rank-nested list consisting of all the elements, in
;;row-major order, of @1.  In the case of a rank-0 array, @0 returns
;;the single element.
;;
;;@example
;;(array->list #2A((ho ho ho) (ho oh oh)))
;;                @result{} ((ho ho ho) (ho oh oh))
;;(array->list #0A ho)
;;                @result{} ho
;;@end example
(define (array->list ra)
  (define (ra2l dims idxs)
    (if (null? dims)
        (apply array-ref ra (reverse idxs))
        (do ((lst '() (cons (ra2l (cdr dims) (cons idx idxs)) lst))
             (idx (+ -1 (car dims)) (+ -1 idx)))
            ((negative? idx) lst))))
  (ra2l (array-dimensions ra) '()))

;;@args vect proto dim1 @dots{}
;;@1 must be a vector of length equal to the product of exact
;;nonnegative integers @3, @dots{}.
;;
;;@0 returns an array of type @2 consisting of all the elements, in
;;row-major order, of @1.  In the case of a rank-0 array, @1 has a
;;single element.
;;
;;@example
;;(vector->array #(1 2 3 4) #() 2 2)
;;                @result{} #2A((1 2) (3 4))
;;(vector->array '#(3) '#())
;;                @result{} #0A 3
;;@end example
(define (vector->array vect prototype . dimensions)
  (define vdx (vector-length vect))
  (if (not (eqv? vdx (apply * dimensions)))
      (error "Vector length does not equal product of dimensions:"
             vdx dimensions))
  (let ((ra (apply make-array prototype dimensions)))
    (define (v2ra dims idxs)
      (cond ((null? dims)
             (set! vdx (+ -1 vdx))
             (apply array-set! ra (vector-ref vect vdx) (reverse idxs)))
            (else
             (do ((idx (+ -1 (car dims)) (+ -1 idx)))
                 ((negative? idx) vect)
               (v2ra (cdr dims) (cons idx idxs))))))
    (v2ra dimensions '())
    ra))

;;@args array
;;Returns a new vector consisting of all the elements of @1 in
;;row-major order.
;;
;;@example
;;(array->vector #2A ((1 2)( 3 4)))
;;                @result{} #(1 2 3 4)
;;(array->vector #0A ho)
;;                @result{} #(ho)
;;@end example
(define (array->vector ra)
  (define dims (array-dimensions ra))
  (let* ((vdx (apply * dims))
         (vect (make-vector vdx)))
    (define (ra2v dims idxs)
      (if (null? dims)
          (let ((val (apply array-ref ra (reverse idxs))))
            (set! vdx (+ -1 vdx))
            (vector-set! vect vdx val)
            vect)
          (do ((idx (+ -1 (car dims)) (+ -1 idx)))
              ((negative? idx) vect)
            (ra2v (cdr dims) (cons idx idxs)))))
    (ra2v dims '())))

(define (array:in-bounds? array indices)
  (do ((bnds (array:dimensions array) (cdr bnds))
       (idxs indices (cdr idxs)))
      ((or (null? bnds)
           (null? idxs)
           (not (integer? (car idxs)))
           (not (< -1 (car idxs) (car bnds))))
       (and (null? bnds) (null? idxs)))))

;;@args array index1 @dots{}
;;Returns @code{#t} if its arguments would be acceptable to
;;@code{array-ref}.
(define (array-in-bounds? array . indices)
  (array:in-bounds? array indices))

;;@args array k1 @dots{}
;;Returns the (@2, @dots{}) element of @1.
(define (array-ref array . indices)
  (define store (array:store array))
  (or (array:in-bounds? array indices)
      (error "Bad indices:" indices))
  ((if (string? store) string-ref vector-ref)
   store (apply + (array:offset array) (map * (array:scales array) indices))))

;;@args array obj k1 @dots{}
;;Stores @2 in the (@3, @dots{}) element of @1.  The value returned
;;by @0 is unspecified.
(define (array-set! array obj . indices)
  (define store (array:store array))
  (or (array:in-bounds? array indices)
      (error "Bad indices:" indices))
  ((if (string? store) string-set! vector-set!)
   store (apply + (array:offset array) (map * (array:scales array) indices))
   obj))

;;@noindent
;;These functions return a prototypical uniform-array enclosing the
;;optional argument (which must be of the correct type).  If the
;;uniform-array type is supported by the implementation, then it is
;;returned; defaulting to the next larger precision type; resorting
;;finally to vector.

(define (make-prototype-checker name pred? creator)
  (lambda args
    (case (length args)
      ((1) (if (pred? (car args))
               (creator (car args))
               (error "Incompatible type:" name (car args))))
      ((0) (creator))
      (else (error "Wrong number of arguments:" name args)))))

(define (integer-bytes?? n)
  (lambda (obj)
    (and (integer? obj)
         (exact? obj)
         (or (negative? n) (not (negative? obj)))
         (do ((num obj (quotient num 256))
              (n (+ -1 (abs n)) (+ -1 n)))
             ((or (zero? num) (negative? n))
              (zero? num))))))

;;@args z
;;@args
;;Returns an inexact 128.bit flonum complex uniform-array prototype.
(define A:floC128b (make-prototype-checker 'A:floC128b complex? vector))
;;@args z
;;@args
;;Returns an inexact 64.bit flonum complex uniform-array prototype.
(define A:floC64b (make-prototype-checker 'A:floC64b complex? vector))
;;@args z
;;@args
;;Returns an inexact 32.bit flonum complex uniform-array prototype.
(define A:floC32b (make-prototype-checker 'A:floC32b complex? vector))
;;@args z
;;@args
;;Returns an inexact 16.bit flonum complex uniform-array prototype.
(define A:floC16b (make-prototype-checker 'A:floC16b complex? vector))

;;@args z
;;@args
;;Returns an inexact 128.bit flonum real uniform-array prototype.
(define A:floR128b (make-prototype-checker 'A:floR128b real? vector))
;;@args z
;;@args
;;Returns an inexact 64.bit flonum real uniform-array prototype.
(define A:floR64b (make-prototype-checker 'A:floR64b real? vector))
;;@args z
;;@args
;;Returns an inexact 32.bit flonum real uniform-array prototype.
(define A:floR32b (make-prototype-checker 'A:floR32b real? vector))
;;@args z
;;@args
;;Returns an inexact 16.bit flonum real uniform-array prototype.
(define A:floR16b (make-prototype-checker 'A:floR16b real? vector))

;;@args z
;;@args
;;Returns an exact 128.bit decimal flonum rational uniform-array prototype.
(define A:floQ128d (make-prototype-checker 'A:floQ128d rational? vector))
;;@args z
;;@args
;;Returns an exact 64.bit decimal flonum rational uniform-array prototype.
(define A:floQ64d (make-prototype-checker 'A:floQ64d rational? vector))
;;@args z
;;@args
;;Returns an exact 32.bit decimal flonum rational uniform-array prototype.
(define A:floQ32d (make-prototype-checker 'A:floQ32d rational? vector))

;;@args n
;;@args
;;Returns an exact binary fixnum uniform-array prototype with at least
;;64 bits of precision.
(define A:fixZ64b (make-prototype-checker 'A:fixZ64b (integer-bytes?? -8) vector))
;;@args n
;;@args
;;Returns an exact binary fixnum uniform-array prototype with at least
;;32 bits of precision.
(define A:fixZ32b (make-prototype-checker 'A:fixZ32b (integer-bytes?? -4) vector))
;;@args n
;;@args
;;Returns an exact binary fixnum uniform-array prototype with at least
;;16 bits of precision.
(define A:fixZ16b (make-prototype-checker 'A:fixZ16b (integer-bytes?? -2) vector))
;;@args n
;;@args
;;Returns an exact binary fixnum uniform-array prototype with at least
;;8 bits of precision.
(define A:fixZ8b (make-prototype-checker 'A:fixZ8b (integer-bytes?? -1) vector))

;;@args k
;;@args
;;Returns an exact non-negative binary fixnum uniform-array prototype with at
;;least 64 bits of precision.
(define A:fixN64b (make-prototype-checker 'A:fixN64b (integer-bytes?? 8) vector))
;;@args k
;;@args
;;Returns an exact non-negative binary fixnum uniform-array prototype with at
;;least 32 bits of precision.
(define A:fixN32b (make-prototype-checker 'A:fixN32b (integer-bytes?? 4) vector))
;;@args k
;;@args
;;Returns an exact non-negative binary fixnum uniform-array prototype with at
;;least 16 bits of precision.
(define A:fixN16b (make-prototype-checker 'A:fixN16b (integer-bytes?? 2) vector))
;;@args k
;;@args
;;Returns an exact non-negative binary fixnum uniform-array prototype with at
;;least 8 bits of precision.
(define A:fixN8b (make-prototype-checker 'A:fixN8b (integer-bytes?? 1) vector))

;;@args bool
;;@args
;;Returns a boolean uniform-array prototype.
(define A:bool (make-prototype-checker 'A:bool boolean? vector))

))

