
(define-library (srfi 151)
  (import (scheme base) (scheme case-lambda))
  ; NB: in Skint, all exact integers are fixnums
  (import (rename (skint)
          (fxnot bitwise-not)
          (fxand bitwise-and)
          (fxior bitwise-ior)
          (fxxor bitwise-xor)
          (fxeqv bitwise-eqv)))
  (import (rename (srfi 143)
          (fxbit-count bit-count)
          (fxlength integer-length)
          (fxif bitwise-if) 
          (fxfirst-set-bit first-set-bit)))
  (export bitwise-not bitwise-and bitwise-ior bitwise-xor bitwise-eqv
          bitwise-nand bitwise-nor bitwise-andc1 bitwise-andc2
          bitwise-orc1 bitwise-orc2)
  (export arithmetic-shift bit-count integer-length bitwise-if 
          bit-set? copy-bit bit-swap any-bit-set? every-bit-set?  first-set-bit)
  (export bit-field bit-field-any? bit-field-every?  bit-field-clear bit-field-set
          bit-field-replace  bit-field-replace-same
          bit-field-rotate bit-field-reverse)
  (export bits->list list->bits bits->vector vector->bits bits
          bitwise-fold bitwise-for-each bitwise-unfold make-bitwise-generator)

(begin

;;; [esl] returns lower fx-width bits of 'ideal' a-shift result
(define (arithmetic-shift/fxbits i count)
  (cond [(fxnegative? count)
         (let ([count (fxneg count)])
           (if (fx>=? count fx-width) ; >> overflow
               (if (fxnegative? i) -1 0)
               (fxsra i count)))]
        [(fx>=? count fx-width) 0] ; << overflow
        [else (fxsll i count)]))

;;; [esl] checked for overflow
(define (arithmetic-shift i count)
  (cond [(fxnegative? count)
         (let ([count (fxneg count)])
           (if (fx>=? count fx-width) ; >> overflow
               (if (fxnegative? i) -1 0)
               (fxsra i count)))]
        [(or (fxzero? i) (fxzero? count)) i]
        [else ; check for loss of information on << overflow
         (let ([j (* i (expt 2 count))])
           (if (fixnum? j) j 
               (error "arithmetic-shift range error" i count)))]))

;;; [esl] checked for overflow
(define (bit-set? index i)
  (cond [(fx<? -1 index fx-width)
         (fxbit-set? index i)]
        [(fx>=? index fx-width)
         (fxnegative? i)]      
        [else (error "copy-bit index out of range" index)]))

;;; [esl] checked for overflow
(define (copy-bit index i bool)
  (cond [(fx<? -1 index fx-width) 
         (fxcopy-bit index i bool)]
        [(and (fx>=? index fx-width) (boolean=? bool (fxnegative? i)))
         i] ; index out of range, but same as virtual one
        [else (error "copy-bit index out of range" index)]))

;;; [esl] NB: i can't have more than fx-width-1 non-sign bits
(define (bit-field i start end)
  (if (fx<=? 0 start end)
      (let* ([n (fx- end start)] [mask (fxnot (arithmetic-shift/fxbits -1 n))])
        (fxand mask (arithmetic-shift/fxbits i (fx- start))))
      (error "invalid bit-field range" start end)))

;;;
;;; SPDX-FileCopyrightText: Olin Shivers
;;;
;;; SPDX-License-Identifier: LicenseRef-Public-Domain
;;; Parts by John Cowan; public domain.
;;; modified by [esl] to use fx functions directly

(define (bitwise-nand  i j)  (fxnot (fxand i j)))
(define (bitwise-nor   i j)  (fxnot (fxior i j)))
(define (bitwise-andc1 i j)  (fxand (fxnot i) j))	
(define (bitwise-andc2 i j)  (fxand i (fxnot j)))	
(define (bitwise-orc1  i j)  (fxior (fxnot i) j))
(define (bitwise-orc2  i j)  (fxior i (fxnot j)))

;;; [esl] made syntax for guaranteed inlining
(define-syntax mask ; local
  (syntax-rules () 
    [(_ start end) 
     (if (fx<=? 0 start end)
         (fxnot (arithmetic-shift/fxbits -1 (fx- end start)))
         (error "invalid bit field range" start end))]))

(define (any-bit-set? test-bits n) 
  (not (fxzero? (fxand test-bits n))))

(define (every-bit-set? test-bits n) 
  (fx=? test-bits (fxand test-bits n)))

(define (bit-field-any? i start end)
  (let ([m (mask start end)]) 
    (not (fxzero? (fxand (arithmetic-shift i (fx- start)) m)))))

(define (bit-field-every? i start end)
  (let ([m (mask start end)]) 
    (fx=? m (fxand (arithmetic-shift i (fx- start)) m))))

;;[esl] modified to work with 'virtual' sign extended fields
(define (bit-field-replace dst src start end)
  (if (fx<=? 0 start end fx-width) ; field inside fixnum
      (let ([m (fxnot (fxsll -1 (fx- end start)))])
        (fxior (fxand dst (fxnot (fxsll m start))) (fxsll (fxand src m) start)))
      (let loop ([dst dst] [s start] [src src])
        (if (fx>=? s end) dst ; NB: copy-bit below may fail
            (loop (copy-bit s dst (fxodd? src)) (fx+ s 1) 
                  (arithmetic-shift src -1))))))  

;; Integrating i-b-f reduces nicely.
(define (bit-field-clear n start end)
  (bit-field-replace n 0 start end))

;; Counterpart to above, not in SRFI 33, written by John Cowan, public domain
(define (bit-field-set n start end)
  (bit-field-replace n -1 start end))

;; [esl] use loop for outside-of-fixnum ranges
(define (bit-field-replace-same dst src start end)
  (if (fx<=? 0 start end fx-width) ; fields inside fixnum
      (let ([m (fxnot (fxsll -1 (fx- end start)))])
        (bitwise-if (fxsll m start) src dst))
      (let loop ([dst dst] [s start])
        (if (fx>=? s end) dst ; NB: copy-bit below may fail
            (loop (copy-bit s dst (bit-set? s src)) (fx+ s 1))))))

;; [esl] use loop for outside-of-fixnum ranges
(define (bit-field-rotate src count start end)
  (if (fx<=? 0 start end fx-width) ; field inside fixnum
      (fxbit-field-rotate src count start end)
      (let ([l (fx- end start)])
        (if (fx<? l 2) src
            (let loop ([dst src] [s start])
              (if (fx>=? s end) dst ; NB: copy-bit below may fail
                  (let ([i (fx+ start (fxmodulo (fx+ (fx- s start) count) l))])
                    (loop (copy-bit i dst (bit-set? s src)) (fx+ s 1)))))))))

;; [esl] use loop for outside-of-fixnum ranges
(define (bit-field-reverse src start end)
  (if (fx<=? 0 start end fx-width) ; field inside fixnum
      (fxbit-field-reverse src start end)
      (let loop ([src src] [s start] [e end])
        (if (fx>? (fx- e s) 1) ; NB: bit-swap below may fail
            (loop (bit-swap s (fx- e 1) src) (fx+ s 1) (fx- e 1))
            src))))

;[esl] use case-lambda
(define bits->list
  (case-lambda
    [(k) 
     (do ([k k (fxsra k 1)] [lst '() (cons (fxodd? k) lst)])
       [(fx<=? k 0) (reverse lst)])]
    [(k len)
     (do ([idx (fx- len 1) (fx- idx 1)] [k k (fxsra k 1)] [lst '() (cons (fxodd? k) lst)])
       [(fxnegative? idx) (reverse lst)])]))

;[esl] check fixnum range
(define (list->bits bools)
  (do ([bs (reverse bools) (cdr bs)] [acc 0 (+ acc acc (if (car bs) 1 0))])
    [(null? bs) acc]
    (unless (fixnum? acc) (error "list->bits out of fixnum range" bools))))

(define (bits . bools)
  (list->bits bools))

;;; Copyright John Cowan 2017
;;;
;;; SPDX-License-Identifier: MIT

(define bits->vector
  (case-lambda
    ((i) (list->vector (bits->list i)))
    ((i len) (list->vector (bits->list i len)))))

(define (vector->bits vector) (list->bits (vector->list vector)))

(define (bit-swap n1 n2 i)
  (let ((n1-bit (bit-set? n1 i))
        (n2-bit (bit-set? n2 i)))
    (copy-bit n2 (copy-bit n1 i n2-bit) n1-bit)))

(define (bitwise-fold proc seed i)
  (let ((len (integer-length i)))
    (let loop ((n 0) (r seed))
      (if (= n len)
        r
        (loop (+ n 1) (proc (bit-set? n i) r))))))

(define (bitwise-for-each proc i)
  (let ((len (integer-length i)))
    (let loop ((n 0))
      (when (< n len)
        (proc (bit-set? n i))
        (loop (+ n 1))))))

(define (bitwise-unfold stop? mapper successor seed)
  (let loop ((n 0) (result 0) (state seed))
    (if (stop? state)
        result
        (loop (+ n 1)
              (copy-bit n result (mapper state))
              (successor state)))))

;[esl]? NB: never reaches eof: this is by design!
(define (make-bitwise-generator i)
  (lambda ()
    (let ([bit (bit-set? 0 i)])
      (set! i (fxsra i 1))
      bit)))

))
