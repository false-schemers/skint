(import (srfi 63))
(include "test.scm")

;; adopted from Larceny

(test-begin "srfi-63")

(define-syntax test-arr
  (syntax-rules ()
    ((_ cat res) (test-equal res cat))))   

(test-assert (array? '#()))

(test-assert (array? '#(a b c 4.0)))

(test-assert (not (array? '())))

(test-assert (not (array? '(a b c 4.0))))

(test-arr 'a 'a)

(test-arr '(a) '(a))

(test-arr '(a (b) c) '(a (b) c))

(test-arr "abc" "abc")

(test-arr 2 2)

(test-arr (make-vector 5 'a) (make-vector 5 'a))

(test-arr (make-array (A:fixN32b 4) 5 3) (make-array (A:fixN32b 4) 5 3))

(test-arr (make-array '#(foo) 3 3) (make-array '#(foo) 3 3))

(test-assert (not (equal? '() 'b)))

(test-assert (not (equal? 4 4.0)))

(test-assert (not (equal? 'a 'b)))

(test-assert (not (equal? '() '(a))))

(test-assert (not (equal? '(a (d) c) '(a (b) c))))

(test-assert (not (equal? "abc" "abcd")))

(test-assert (not (equal? 2 2.0)))

(test-assert (not (equal? (make-vector 5 'a) (make-vector 6 'a))))

(test-assert (not (equal? (make-array (A:fixN32b 4) 5 3) (make-array (A:fixN32b 2) 5 3))))

(test-assert (not (equal? (make-array '#(foo) 3 3) (make-array '#(baz) 3 3))))

(test-arr 0 (array-rank 4.5))

(test-arr 2 (array-rank (make-array '#(foo) 3 3)))

(test-arr '(3 7) (array-dimensions (make-array '#(foo) 3 7)))

(let ()
  (define fred (make-array '#(#f) 8 8))
  (define freds-diagonal
    (make-shared-array fred (lambda (i) (list i i)) 8))
  (define ignored1 (array-set! freds-diagonal 'foo 3))
  (define ignored2 (or (equal? 'foo (array-ref fred 3 3))
                       (fail 'make-shared-array:0)))
  (define freds-center
    (make-shared-array fred (lambda (i j) (list (+ 3 i) (+ 3 j)))
                       2 2))
  (test-equal 'foo (array-ref freds-center 0 0)))

(test-arr (array->list (list->array 2 '#() '((1 2) (3 4))))
            '((1 2) (3 4)))

(test-arr (array->list (list->array 0 '#() 3))
            3)

(test-arr (array->list (vector->array '#(1 2 3 4) '#() 2 2))
            '((1 2) (3 4)))

(test-arr (array->vector (vector->array '#(1 2 3 4) '#() 2 2))
            '#(1 2 3 4))

(test-arr #f (array-in-bounds? (make-array '#(#f) 0 0) 0))

(test-arr #f (array-in-bounds? (make-array '#(#f) 1 0) 0 0))

(test-arr #f (array-in-bounds? (make-array '#(#f) 0 1) 0 0))

(test-arr #t (array-in-bounds? (make-array '#(#f) 4 5) 3 4))

(test-arr #f (array-in-bounds? (make-array '#(#f) 4 5) 3 5))

(test-arr #f (array-in-bounds? (make-array '#(#f) 4 5) 4 4))

(let ((m 4)
      (n 5)
      (o 6))
  (define (f i j k)
    (* (+ i 1) (+ j j 3) (+ k k k 5)))
  (define a (make-array '#() m n o))
  (do ((i 0 (+ i 1)))
      ((= i m))
    (do ((j 0 (+ j 1)))
        ((= j n))
      (do ((k 0 (+ k 1)))
          ((= k o))
        (array-set! a (f i j k) i j k))))
  (do ((k 0 (+ k 1)))
      ((= k o))
    (do ((j 0 (+ j 1)))
        ((= j n))
      (do ((i 0 (+ i 1)))
          ((= i m))
        (test-assert (= (f i j k)
               (array-ref a i j k)))))))

(define fixnum-max 536870911) ;[esl] Skint's largest positive fixnum
;[esl] most of these are useless because their numbers can't be represented in Skint numbers;
;the useful one are A:bool A:fixN8b A:fixN16b A:floR64b

(let ((prototypes
       (list
        (A:floC128b 3) ;esl: Skint does not support complex numbers with nonzero imaginary part, e.g. 3+4i
        (A:floC128b)
        (A:floC64b 3.14159) ;esl: Skint does not support complex notation, e.g. 3.14159+0i
        (A:floC64b)
        (A:floC32b 3.0)
        (A:floC32b)
        (A:floC16b 3)
        (A:floC16b)
        (A:floR128b 45.0)
        (A:floR128b)
        (A:floR64b 45)
        (A:floR64b)
        (A:floR32b 0)
        (A:floR32b)
        (A:floR16b 255)
        (A:floR16b)
        (A:floQ128d fixnum-max) ;[esl]* ; was(expt 2.0 126)
        (A:floQ128d)
        (A:floQ64d 4.5)
        (A:floQ64d)
        (A:floQ32d 4)
        (A:floQ32d)
        (A:fixZ64b fixnum-max) ;[esl]*
        (A:fixZ64b)
        (A:fixZ32b fixnum-max) ;[esl]*
        (A:fixZ32b)
        (A:fixZ16b 60000)
        (A:fixZ16b)
        (A:fixZ8b 255)
        (A:fixZ8b)
        (A:fixN64b fixnum-max) ;[esl]*
        (A:fixN64b)
        (A:fixN32b fixnum-max) ;[esl]*
        (A:fixN32b)
        (A:fixN16b 65535)
        (A:fixN16b)
        (A:fixN8b 255)
        (A:fixN8b)
        (A:bool #t)
        (A:bool))))
  (for-each (lambda (a)
              (test-assert (array? a)))
            prototypes))

(test-end)

