
(define-library (srfi 143)
  (export
   fx-width
   fx-greatest
   fx-least
   fixnum?             ; builtin
   fx=?                ; builtin
   fx<?                ; builtin
   fx>?                ; builtin
   fx<=?               ; builtin
   fx>=?               ; builtin
   fxzero?             ; builtin
   fxpositive?         ; builtin
   fxnegative?         ; builtin
   fxodd?              ; builtin
   fxeven?             ; builtin
   fxmax               ; builtin
   fxmin               ; builtin
   fx+                 ; builtin
   fx-                 ; builtin
   fxneg               ; builtin
   fx*                 ; builtin
   fxquotient          ; builtin
   fxremainder         ; builtin
   fxabs               ; builtin
   fxsquare
   fxsqrt              ; builtin %fxsqrt
   fx+/carry
   fx-/carry
   fx*/carry
   fxnot               ; builtin
   fxand               ; builtin
   fxior               ; builtin
   fxxor               ; builtin
   fxarithmetic-shift
   fxarithmetic-shift-left
   fxarithmetic-shift-right
   fxbit-count
   fxlength
   fxif
   fxbit-set?
   fxcopy-bit
   fxfirst-set-bit
   fxbit-field
   fxbit-field-rotate
   fxbit-field-reverse)
  (import (skint))

(begin

(define fx-width 
  (do ([x 1 (fxsll x 1)] [n 0 (fx+ n 1)]) [(fxnegative? x) (fx+ n 1)]))

(define fx-greatest 
  (let ([x (fxexpt 2 (fx- fx-width 2))]) (fx+ x (fx- x 1))))

(define fx-least 
  (fx+ fx-greatest 1))

(unless (and (fxpositive? fx-greatest) (fxnegative? fx-least))
  (error "bogus fixnum constants" fx-width fx-greatest fx-least))

(define (fxsqrt x)
  (let* ([rem 0] [srt (%fxsqrt x (set& rem))])
    (values srt rem)))

(define (fxsquare i) 
  (* i i))

(define (fx+/carry i j k)
  (let ([q (fxaddc i j (set& k))])
    (values k q)))

(define (fx-/carry i j k)
  (let ([q (fxsubc i j (set& k))])
    (values k q)))

(define (fx*/carry i j k)
  (let ([q (fxmulc i j (set& k))])
    (values k q)))

(define (fxarithmetic-shift i count)
  (if (fx<? count 0) (fxsra i (fxneg count)) (fxsll i count)))

(define fxarithmetic-shift-left fxsll)
(define fxarithmetic-shift-right fxsra)

(define (popc i c) ; internal; adds count of raw bits in i to c 
  (if (fxzero? i) c (popc (fxand i (fx- i 1)) (fx+ c 1))))

(define (fxbit-count i) ; same, but treats negatives specially
  (let ([c (popc i 0)])
    (if (fxnegative? i) (fx- fx-width c) c)))

(define (fxlength n)
  (let loop ([n n] [t 0])
    (case n
      [(0 -1) t]
      [(1 -2) (fx+ 1 t)]
      [(2 3 -3 -4) (fx+ 2 t)]
      [(4 5 6 7 -5 -6 -7 -8) (fx+ 3 t)]
      [else (loop (fxsra n 4) (fx+ 4 t))])))

(define (fxif mask i j) 
  (fxior (fxand mask i) (fxand (fxnot mask) j)))

(define (fxbit-set? index i)
  (not (fxzero? (fxand (fxsll 1 index) i))))

(define (fxcopy-bit index i b)
  (if b 
      (fxior i (fxsll 1 index))
      (fxand i (fxnot (fxsll 1 index)))))

(define (fxfirst-set-bit i)
  (if (fxzero? i) -1 (fx- (popc (fxxor i (fx- i 1)) 0) 1)))

(define (fxbit-field n start end)
  (fxand (fxnot (fxsll -1 (fx- end start))) (fxsra n start)))

(define (fxbit-field-rotate n count start end)
  (let* ([width (fx- end start)] [count (fxmodulo count width)])
    (let* ([mask (fxnot (fxsll -1 width))] [zn (fxand mask (fxsra n start))])
      (fxior (fxsll (fxior (fxand mask (fxsll zn count)) (fxsra zn (fx- width count))) start)
             (fxand (fxnot (fxsll mask start)) n)))))

(define (bit-reverse k n) internal;
  (do ([m (if (fxnegative? n) (fxnot n) n) (fxsra m 1)]
       [k (fx- k 1) (fx- k 1)]
       [rvs 0 (fxior (fxsll rvs 1) (fxand 1 m))])
    [(fxnegative? k) (if (fxnegative? n) (fxnot rvs) rvs)]))

(define (fxbit-field-reverse n start end)
  (let* ([width (fx- end start)] [mask (fxnot (fxsll -1 width))])
    (let ([zn (fxand mask (fxsra n start))])
      (fxior (fxsll (bit-reverse width zn) start)
             (fxand (fxnot (fxsll mask start)) n)))))
 
))