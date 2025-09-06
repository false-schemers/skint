
(define-library (srfi 66)
  (import (scheme base)
          (only (skint) bytevector->list list->bytevector bytevector=?))
  (export 
    (rename bytevector? u8vector?)
    (rename make-bytevector make-u8vector)
    (rename bytevector u8vector)
    (rename bytevector->list u8vector->list)
    (rename list->bytevector list->u8vector)
    (rename bytevector-length u8vector-length)
    (rename bytevector-u8-ref u8vector-ref)
    (rename bytevector-u8-set! u8vector-set!)
    (rename bytevector=? u8vector=?)
    (rename bytevector-copy u8vector-copy)
    u8vector-compare
    u8vector-copy!) ; incompatible w/bytevector version!
(begin

(define (u8vector-copy! source source-start target target-start count)
  (if (>= source-start target-start)
      (do ((i 0 (+ i 1)))
	  ((= i count))
        (bytevector-u8-set! target
          (+ target-start i) 
          (bytevector-u8-ref source (+ source-start i))))
      (do ((i (- count 1) (- i 1)))
	  ((= i -1))
        (bytevector-u8-set! target
          (+ target-start i) 
          (bytevector-u8-ref source (+ source-start i))))))

(define (u8vector-compare bytevector-1 bytevector-2)
  (let ((length-1 (bytevector-length bytevector-1))
        (length-2 (bytevector-length bytevector-2)))
    (cond
      ((< length-1 length-2) -1)
      ((> length-1 length-2)  1)
      (else
       (let loop ((i 0))
         (if (= i length-1)
             0
             (let ((elt-1 (bytevector-u8-ref bytevector-1 i))
                   (elt-2 (bytevector-u8-ref bytevector-2 i)))
               (cond ((< elt-1 elt-2) -1)
                     ((> elt-1 elt-2)  1)
                     (else (loop (+ i 1)))))))))))

))


