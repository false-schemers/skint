(define-library (srfi 160 base)
  (import (scheme base) (scheme case-lambda))
  (import (only (skint) fixnum? flonum? fx<? fx<=? integer-length bytevector->list 
    make-numvector numvector? numvector-length numvector-ref numvector-set! list->numvector))
  (import (srfi 4))
  (export ; re-exported from SRFI-4
    u8vector? make-u8vector u8vector u8vector-length u8vector-ref u8vector-set!
    u8vector->list list->u8vector
    s8vector? make-s8vector s8vector s8vector-length s8vector-ref s8vector-set!
    s8vector->list list->s8vector
    u16vector? make-u16vector u16vector u16vector-length u16vector-ref
    u16vector-set! u16vector->list list->u16vector
    s16vector? make-s16vector s16vector s16vector-length s16vector-ref
    s16vector-set! s16vector->list list->s16vector)
  (export ; re-exported from SRFI-4
    f32vector? make-f32vector f32vector f32vector-length f32vector-ref
    f32vector-set! f32vector->list list->f32vector
    f64vector? make-f64vector f64vector f64vector-length f64vector-ref
    f64vector-set! f64vector->list list->f64vector)
  (export u8? s8? u16? s16? f32? f64?)
  (export ; always exported but defined only in full-tower mode in SRFI-4
    u32vector? make-u32vector u32vector u32vector-length u32vector-ref
    u32vector-set! u32vector->list list->u32vector
    s32vector? make-s32vector s32vector s32vector-length s32vector-ref
    s32vector-set! s32vector->list list->s32vector
    u64vector? make-u64vector u64vector u64vector-length u64vector-ref
    u64vector-set! u64vector->list list->u64vector
    s64vector? make-s64vector s64vector s64vector-length s64vector-ref
    s64vector-set! s64vector->list list->s64vector)
  (export ; these are always exported but defined only in full-tower mode
    u32? s32? u64? s64?)
  (export ; these are always exported but defined only in full-tower mode
    make-c64vector make-c128vector c64vector c128vector c64vector? c128vector?
    c64vector-length c128vector-length c64vector-ref c128vector-ref
    c64vector-set! c128vector-set! c64vector->list c128vector->list
    c64? c128?)

(begin

(define (u8? x)  (and (fixnum? x) (fx<=? 0 x 255)))
(define (s8? x)  (and (fixnum? x) (fx<=? -128 x 127)))
(define (u16? x) (and (fixnum? x) (fx<=? 0 x 65535)))
(define (s16? x) (and (fixnum? x) (fx<=? -32768 x 32767)))
(define (f32? x) (flonum? x))
(define (f64? x) (flonum? x))

; [esl*] implementation on top of Skint numvecs

(cond-expand 
(full-numeric-tower ;[esl]

(define-syntax c64 14)
(define-syntax c128 15)

(define (u32? x) (and (exact-integer? x) (>= x 0) (fx<=? (integer-length x) 32)))
(define (s32? x) (and (exact-integer? x) (fx<? (integer-length x) 32)))
(define (u64? x) (and (exact-integer? x) (>= x 0) (fx<=? (integer-length x) 64)))
(define (s64? x) (and (exact-integer? x) (fx<? (integer-length x) 64)))
(define (c64? n) (inexact? n))
(define (c128? n) (inexact? n))

(define *type-names* #("c64" "c128"))

(define-syntax numvector-check
  (syntax-rules ()
    [(_ typ exp)
     (let ([t typ] [x exp]) 
       (cond [(numvector? x t) x]
             [else (error (string-append "invalid " (vector-ref *type-names* (- t c64)) "vector") x)]))]))

(define (c64vector? x) (numvector? x c64))
(define (c128vector? x) (numvector? x c128))

(define (c64vector-length x) (numvector-length (numvector-check c64 x)))
(define (c128vector-length x) (numvector-length (numvector-check c128 x)))

(define (c64vector-ref x i) (numvector-ref (numvector-check c64 x) i))
(define (c128vector-ref x i) (numvector-ref (numvector-check c128 x) i))

(define (c64vector-set! x i n) (numvector-set! (numvector-check c64 x) i n))
(define (c128vector-set! x i n) (numvector-set! (numvector-check c128 x) i n))

(define (list->c64vector l) (list->numvector l c64))
(define (list->c128vector l) (list->numvector l c128))

(define (c64vector . l) (list->numvector l c64))
(define (c128vector . l) (list->numvector l c128))

(define (make-numvector-procedure t)
  (case-lambda
    [(len) (make-numvector t len)]
    [(len init) 
     (let ([v (make-numvector t len)])
       (do ([i 0 (+ i 1)]) [(>= i len) v] (numvector-set! v i init)))]))

(define make-c64vector (make-numvector-procedure c64))
(define make-c128vector (make-numvector-procedure c128))

(define (numvector->list v start end)
  (let loop ([i (- end 1)] [l '()])
    (if (< i start) l (loop (- i 1) (cons (numvector-ref v i) l)))))

(define (numvector->list-procedure t)
  (case-lambda
    [(v) (numvector->list v 0 (numvector-length (numvector-check t v)))]
    [(v start) (numvector->list v start (numvector-length (numvector-check t v)))]
    [(v start end) (numvector->list (numvector-check t v) start end)]))

(define c64vector->list (numvector->list-procedure c64))
(define c128vector->list (numvector->list-procedure c128))))

))
