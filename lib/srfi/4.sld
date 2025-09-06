
(define-library (srfi 4)
  (import (scheme base) (scheme case-lambda))
  (import (only (skint)
    bytevector->list 
    make-numvector numvector? numvector-length numvector-ref numvector-set! list->numvector))
  (export
    u8vector? make-u8vector u8vector u8vector-length u8vector-ref u8vector-set!
    u8vector->list list->u8vector
    s8vector? make-s8vector s8vector s8vector-length s8vector-ref s8vector-set!
    s8vector->list list->s8vector
    u16vector? make-u16vector u16vector u16vector-length u16vector-ref
    u16vector-set! u16vector->list list->u16vector
    s16vector? make-s16vector s16vector s16vector-length s16vector-ref
    s16vector-set! s16vector->list list->s16vector
    ; larger integers are out of Skint fixnum range; support is not required
    ;   u32vector? make-u32vector u32vector u32vector-length u32vector-ref
    ;   u32vector-set! u32vector->list list->u32vector
    ;   s32vector? make-s32vector s32vector s32vector-length s32vector-ref
    ;   s32vector-set! s32vector->list list->s32vector
    ;   u64vector? make-u64vector u64vector u64vector-length u64vector-ref
    ;   u64vector-set! u64vector->list list->u64vector
    ;   s64vector? make-s64vector s64vector s64vector-length s64vector-ref
    ;   s64vector-set! s64vector->list list->s64vector
    f32vector? make-f32vector f32vector f32vector-length f32vector-ref
    f32vector-set! f32vector->list list->f32vector
    f64vector? make-f64vector f64vector f64vector-length f64vector-ref
    f64vector-set! f64vector->list list->f64vector)

(begin
;; [esl]

; internal Skint numvector subtypes
(define-syntax u8 0)
(define-syntax s8 1)
(define-syntax u16 2)
(define-syntax s16 3)
(define-syntax f32 10)
(define-syntax f64 11)

(define *type-names* 
  #("u8" "s8" "u16" "s16" "u32" "s32" "u64" "s64" "u128" "s128" "f32" "f64"))

(define-syntax numvector-check
  (syntax-rules ()
    [(_ typ exp)
     (let ([t typ] [x exp]) 
       (cond [(numvector? x t) x]
             [else (error (string-append "invalid " (vector-ref *type-names* t) "vector") x)]))]))

(define (u8vector? x) (numvector? x u8))
(define (s8vector? x) (numvector? x s8))
(define (u16vector? x) (numvector? x u16))
(define (s16vector? x) (numvector? x s16))
(define (f32vector? x) (numvector? x f32))
(define (f64vector? x) (numvector? x f64))

(define (u8vector-length x) (numvector-length (numvector-check u8 x)))
(define (s8vector-length x) (numvector-length (numvector-check s8 x)))
(define (u16vector-length x) (numvector-length (numvector-check u16 x)))
(define (s16vector-length x) (numvector-length (numvector-check s16 x)))
(define (f32vector-length x) (numvector-length (numvector-check f32 x)))
(define (f64vector-length x) (numvector-length (numvector-check f64 x)))

(define (u8vector-ref x i) (numvector-ref (numvector-check u8 x) i))
(define (s8vector-ref x i) (numvector-ref (numvector-check s8 x) i))
(define (u16vector-ref x i) (numvector-ref (numvector-check u16 x) i))
(define (s16vector-ref x i) (numvector-ref (numvector-check s16 x) i))
(define (f32vector-ref x i) (numvector-ref (numvector-check f32 x) i))
(define (f64vector-ref x i) (numvector-ref (numvector-check f64 x) i))

(define (u8vector-set! x i n) (numvector-set! (numvector-check u8 x) i n))
(define (s8vector-set! x i n) (numvector-set! (numvector-check s8 x) i n))
(define (u16vector-set! x i n) (numvector-set! (numvector-check u16 x) i n))
(define (s16vector-set! x i n) (numvector-set! (numvector-check s16 x) i n))
(define (f32vector-set! x i n) (numvector-set! (numvector-check f32 x) i n))
(define (f64vector-set! x i n) (numvector-set! (numvector-check f64 x) i n))

(define (list->u8vector l) (list->numvector l u8))
(define (list->s8vector l) (list->numvector l s8))
(define (list->u16vector l) (list->numvector l u16))
(define (list->s16vector l) (list->numvector l s16))
(define (list->f32vector l) (list->numvector l f32))
(define (list->f64vector l) (list->numvector l f64))

(define (u8vector . l) (list->numvector l u8))
(define (s8vector . l) (list->numvector l s8))
(define (u16vector . l) (list->numvector l u16))
(define (s16vector . l) (list->numvector l s16))
(define (f32vector . l) (list->numvector l f32))
(define (f64vector . l) (list->numvector l f64))

(define (make-numvector-procedure t)
  (case-lambda
    [(len) (make-numvector t len)]
    [(len init) 
     (let ([v (make-numvector t len)])
       (do ([i 0 (+ i 1)]) [(>= i len) v] (numvector-set! v i init)))]))

(define make-u8vector make-bytevector) ; == (make-numvector-procedure u8)
(define make-s8vector (make-numvector-procedure s8))
(define make-u16vector (make-numvector-procedure u16))
(define make-s16vector (make-numvector-procedure s16))
(define make-f32vector (make-numvector-procedure f32))
(define make-f64vector (make-numvector-procedure f64))

(define (numvector->list v start end)
  (let loop ([i (- end 1)] [l '()])
    (if (< i start) l (loop (- i 1) (cons (numvector-ref v i) l)))))

(define (numvector->list-procedure t)
  (case-lambda
    [(v) (numvector->list v 0 (numvector-length (numvector-check t v)))]
    [(v start) (numvector->list v start (numvector-length (numvector-check t v)))]
    [(v start end) (numvector->list (numvector-check t v) start end)]))

;; NB: extended to support start/end for SRFI-160 compatibility
(define u8vector->list (numvector->list-procedure u8))
(define s8vector->list (numvector->list-procedure s8))
(define u16vector->list (numvector->list-procedure u16))
(define s16vector->list (numvector->list-procedure s16))
(define f32vector->list (numvector->list-procedure f32))
(define f64vector->list (numvector->list-procedure f64))

))
