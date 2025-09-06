(define-library (srfi 160 base)
  (import (scheme base))
  (import (only (skint) fixnum? flonum? fx<=?))
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
  (export ; NB: these are exported but not defined (as per SRFI requirements)
    u32vector? make-u32vector u32vector u32vector-length u32vector-ref
    u32vector-set! u32vector->list list->u32vector
    s32vector? make-s32vector s32vector s32vector-length s32vector-ref
    s32vector-set! s32vector->list list->s32vector
    u64vector? make-u64vector u64vector u64vector-length u64vector-ref
    u64vector-set! u64vector->list list->u64vector
    s64vector? make-s64vector s64vector s64vector-length s64vector-ref
    s64vector-set! s64vector->list list->s64vector)
  (export ; re-exported from SRFI-4
    f32vector? make-f32vector f32vector f32vector-length f32vector-ref
    f32vector-set! f32vector->list list->f32vector
    f64vector? make-f64vector f64vector f64vector-length f64vector-ref
    f64vector-set! f64vector->list list->f64vector)
  (export ; NB: only u8? s8? u16? s16? f32? f64? are defined 
    u8? s8? u16? s16? u32? s32? u64? s64? f32? f64? c64? c128?)

(begin
(define (u8? x)  (and (fixnum? x) (fx<=? 0 x 255)))
(define (s8? x)  (and (fixnum? x) (fx<=? -128 x 127)))
(define (u16? x) (and (fixnum? x) (fx<=? 0 x 65535)))
(define (s16? x) (and (fixnum? x) (fx<=? -32768 x 32767)))
(define (f32? x) (flonum? x))
(define (f64? x) (flonum? x))
))
