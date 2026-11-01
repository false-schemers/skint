
(define-library (srfi 60)
  (import (scheme base))
  (import (only (skint) 
    bitwise-not bitwise-and bitwise-ior bitwise-xor
    arithmetic-shift integer-length bit-count))

  (export
    ; reexports / aliases
    bitwise-not (rename bitwise-not lognot)
    bitwise-and (rename bitwise-and logand)
    bitwise-ior (rename bitwise-ior logior)
    bitwise-xor (rename bitwise-xor logxor)
    arithmetic-shift (rename arithmetic-shift ash)
    bit-count (rename bit-count logcount)
    integer-length
    ; defined here
    bitwise-if (rename bitwise-if bitwise-merge)
    bit-set? (rename bit-set? logbit?)
    any-bits-set? (rename any-bits-set? logtest)
    first-set-bit (rename first-set-bit log2-binary-factors)
    copy-bit
    bit-field
    copy-bit-field
    rotate-bit-field reverse-bit-field
    integer->list list->integer
    booleans->integer)

(begin
;; [esl*] ported to Skint relying on its builtins for basics
;;;; "logical.scm", bit access and operations for integers for Scheme
;;; Copyright (C) 1991, 1993, 2001, 2003, 2005 Aubrey Jaffer
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

;@
(define (any-bits-set? n1 n2)
  (not (zero? (bitwise-and n1 n2))))
;@
(define (bit-set? index n)
  (any-bits-set? (expt 2 index) n))
;@
(define (copy-bit index to bool)
  (if bool
      (bitwise-ior to (arithmetic-shift 1 index))
      (bitwise-and to (bitwise-not (arithmetic-shift 1 index)))))
;@
(define (bitwise-if mask n0 n1)
  (bitwise-ior (bitwise-and mask n0)
               (bitwise-and (bitwise-not mask) n1)))
;@
(define (bit-field n start end)
  (bitwise-and (bitwise-not (arithmetic-shift -1 (- end start)))
               (arithmetic-shift n (- start))))
;@
(define (copy-bit-field to from start end)
  (bitwise-if (arithmetic-shift (bitwise-not (arithmetic-shift -1 (- end start))) start)
              (arithmetic-shift from start)
              to))
;@
(define (rotate-bit-field n count start end)
  (define width (- end start))
  (set! count (modulo count width))
  (let ((mask (bitwise-not (arithmetic-shift -1 width))))
    (define zn (bitwise-and mask (arithmetic-shift n (- start))))
    (bitwise-ior 
      (arithmetic-shift
        (bitwise-ior (bitwise-and mask (arithmetic-shift zn count))
                     (arithmetic-shift zn (- count width)))
        start)
      (bitwise-and (bitwise-not (arithmetic-shift mask start)) n))))
;@
(define (first-set-bit n)
  (+ -1 (integer-length (bitwise-and n (- n)))))

(define (bit-reverse k n)
  (do ((m (if (negative? n) (bitwise-not n) n) (arithmetic-shift m -1))
       (k (+ -1 k) (+ -1 k))
       (rvs 0 (bitwise-ior (arithmetic-shift rvs 1) (bitwise-and 1 m))))
      ((negative? k) (if (negative? n) (bitwise-not rvs) rvs))))
;@
(define (reverse-bit-field n start end)
  (define width (- end start))
  (let ((mask (bitwise-not (arithmetic-shift -1 width))))
    (define zn (bitwise-and mask (arithmetic-shift n (- start))))
    (bitwise-ior (arithmetic-shift (bit-reverse width zn) start)
            (bitwise-and (bitwise-not (arithmetic-shift mask start)) n))))
;@
(define (integer->list k . len)
  (if (null? len)
      (do ((k k (arithmetic-shift k -1))
           (lst '() (cons (odd? k) lst)))
          ((<= k 0) lst))
      (do ((idx (+ -1 (car len)) (+ -1 idx))
           (k k (arithmetic-shift k -1))
           (lst '() (cons (odd? k) lst)))
          ((negative? idx) lst))))
;@
(define (list->integer bools)
  (do ((bs bools (cdr bs))
       (acc 0 (+ acc acc (if (car bs) 1 0))))
      ((null? bs) acc)))

(define (booleans->integer . bools)
  (list->integer bools))

))
