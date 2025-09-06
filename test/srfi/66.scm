(import (srfi 66))
(include "test.scm")

;; adopted from Larceny
(test-begin "srfi-66")

(test-assert
    (and (u8vector? (make-u8vector 10 23))
         (equal? (u8vector->list (u8vector 0 5 6 7 8 255))
                 '(0 5 6 7 8 255))
         (equal? (u8vector->list (list->u8vector '(255 254 1 0)))
                 '(255 254 1 0))
         (= (u8vector-length (u8vector 1 2 3 4)) 4)
         (= (u8vector-ref (u8vector 20 21 22 23 24) 0) 20)
         (= (u8vector-ref (u8vector 20 21 22 23 24) 4) 24)))

(let ((bv (u8vector 20 21 22 23 24)))
  (u8vector-set! bv 0 50)
  (u8vector-set! bv 4 255)
  (test-assert (and (u8vector=? bv (u8vector 50 21 22 23 255))
           (= -1 (u8vector-compare (u8vector) bv))
           (= -1 (u8vector-compare (u8vector 255) bv))
           (=  0 (u8vector-compare bv bv))
           (=  1 (u8vector-compare bv (u8vector 255 255 255 255)))))
  (u8vector-copy! (u8vector 100 99 98 97 96) 1 bv 0 3)
  (test-assert (u8vector=? bv (u8vector 99 98 97 23 255)))
  (u8vector-copy! bv 1 bv 0 4)
  (test-assert (u8vector=? bv (u8vector 98 97 23 255 255)))
  (u8vector-copy! bv 0 bv 2 3)
  (test-assert (u8vector=? bv (u8vector 98 97 98 97 23)))
  (test-assert (and (u8vector=? (u8vector-copy bv) bv)
                    (not (eq? (u8vector-copy bv) bv)))))

(test-end)

