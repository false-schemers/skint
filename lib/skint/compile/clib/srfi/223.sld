;[esl] adapted from the original ref. imp. by Daphne Preston-Kendal
; (C) 2021 Daphne Preston-Kendal (text and sample implementation in Scheme). 
; (C) 1992–2019 Python Software Foundation (guideline algorithm implementation in Python).

(define-library (srfi 223)
  (import (scheme base)
          (scheme case-lambda))

  (export bisect-left bisect-right
          bisection
          vector-bisect-left vector-bisect-right)

(begin

(define (bisect-left a val ref less? lo hi)
  (if (>= lo hi) lo
      (let ((mid (+ lo (floor-quotient (- hi lo) 2)))) ;[esl*] was (floor-quotient (+ lo hi) 2)
        (if (less? (ref a mid) val)
            (bisect-left a val ref less? (+ mid 1) hi)
            (bisect-left a val ref less? lo mid)))))

(define (bisect-right a val ref less? lo hi)
  (if (>= lo hi) lo
      (let ((mid (+ lo (floor-quotient (- hi lo) 2)))) ;[esl*] was (floor-quotient (+ lo hi) 2)
        (if (less? val (ref a mid))
            (bisect-right a val ref less? lo mid)
            (bisect-right a val ref less? (+ mid 1) hi)))))

(define bisection
  (case-lambda
    ((ref lo-hi-proc)
     (values
      (case-lambda
       ((a val less?)
        (let-values (((lo hi) (lo-hi-proc a)))
          (bisect-left a val ref less? lo hi)))
       ((a val less? lo hi)
        (bisect-left a val ref less? lo hi)))
      (case-lambda
       ((a val less?)
        (let-values (((lo hi) (lo-hi-proc a)))
          (bisect-right a val ref less? lo hi)))
       ((a val less? lo hi)
        (bisect-right a val ref less? lo hi)))))
    ((ref)
     (bisection ref
       (lambda (a) (error "both lo and hi arguments must be given to this procedure"))))))

(define-values (vector-bisect-left vector-bisect-right)
  (bisection vector-ref (lambda (v) (values 0 (vector-length v)))))

))  
