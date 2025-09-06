(define-library (srfi 27)
  (import (skint)) ; fl ops &c. 
  (export random-integer random-real default-random-source
          make-random-source random-source?
          random-source-state-ref random-source-state-set!
          random-source-randomize! random-source-pseudo-randomize!
          random-source-make-integers random-source-make-reals)

(begin
; 54-BIT FLONUM IMPLEMENTATION OF THE "MRG32K3A"-GENERATOR
; ========================================================
;
; Sebastian.Egner@philips.com, 2002.
;
; This code is a floating point implementation in Scheme of Pierre L'Ecuyer's 
; MRG32k3a generator. The code is based on Brad Lucier's code for this generator. 
; Please refer to Brad's original posting in the SRFI-27 discussion archive and 
; to his posting of an optimization.
;
; history of this file:
;   SE, 18-Mar-2002: initial version
;   SE, 25-Mar-2002: adapted to new interface
;   SE, 04-Apr-2002: debugged; simplified
;   SE, 10-Apr-2002: incorporated Brad Lucier's optimizations
; 
; [esl] ported to Skint

; Generator
; =========
;
; Pierre L'Ecuyer's MRG32k3a generator is a Combined Multiple Recursive 
; Generator. It produces the sequence {(x[1,n] - x[2,n]) mod m1 : n} 
; defined by the two recursive generators
;
;   x[1,n] = (               a12 x[1,n-2] + a13 x[1,n-3]) mod m1,
;   x[2,n] = (a21 x[2,n-1] +                a23 x[2,n-3]) mod m2,
;
; where the constants are
;   m1       = 4294967087 = 2^32 - 209    modulus of 1st component
;   m2       = 4294944443 = 2^32 - 22853  modulus of 2nd component
;   a12      =  1403580                   recursion coefficients
;   a13      =  -810728
;   a21      =   527612
;   a23      = -1370589
;
; The generator passes all tests of G. Marsaglia's Diehard testsuite.
; Its period is (m1^3 - 1)(m2^3 - 1)/2 which is nearly 2^191.
; L'Ecuyer reports: "This generator is well-behaved in all dimensions
; up to at least 45: ..." [with respect to the spectral test, SE].
;
; The period is maximal for all values of the seed as long as the
; state of both recursive generators is not entirely zero.
;
; As the successor state is a linear combination of previous
; states, it is possible to advance the generator by more than one
; iteration by applying a linear transformation. The following
; publication provides detailed information on how to do that:
;
;    [1] P. L'Ecuyer, R. Simard, E. J. Chen, W. D. Kelton:
;        An Object-Oriented Random-Number Package With Many Long 
;        Streams and Substreams. 2001.
;        To appear in Operations Research.
;
;   (mrg32k3a-pack-state unpacked-state) -> packed-state
;   (mrg32k3a-unpack-state packed-state) -> unpacked-state
;      pack/unpack a state of the generator. The core generator works
;      on packed states, passed as an explicit argument, only. This
;      allows native code implementations to store their state in a
;      suitable form. Unpacked states are #(x10 x11 x12 x20 x21 x22) 
;      with integer x_ij. Pack/unpack need not allocate new objects
;      in case packed and unpacked states are identical.
;
;   (mrg32k3a-random-range) -> m-max
;   (mrg32k3a-random-integer packed-state range) -> x in {0..range-1}
;      advance the state of the generator and return the next random
;      range-limited integer. 
;        Note that the state is not necessarily advanced by just one 
;      step because we use the rejection method to avoid any problems 
;      with distribution anomalies.
;        The range argument must be an exact integer in {1..m-max}.
;      It can be assumed that range is a fixnum if the Scheme system
;      has such a number representation.
;
;   (mrg32k3a-random-real packed-state) -> x in (0,1)
;      advance the state of the generator and return the next random
;      real number between zero and one (both excluded). The type of
;      the result should be a flonum if possible.


(define (mrg32k3a-random-m1 state) ; from Brad Lucier
  (let* ((x10 (fl- (fl* 1403580.0 (vector-ref state 1)) 
                   (fl* 810728.0 (vector-ref state 2))))
         (y10 (fl- x10 (fl* (flfloor (fl/ x10 4294967087.0)) 4294967087.0)))
         (x20 (fl- (fl* 527612.0 (vector-ref state 3)) 
                   (fl* 1370589.0 (vector-ref state 5))))
         (y20 (fl- x20 (fl* (flfloor (fl/ x20 4294944443.0)) 4294944443.0)))
         (dx (fl- y10 y20))
         (dy (fl- dx (* (flfloor (fl/ dx 4294967087.0)) 4294967087.0))))
    (vector-set! state 5 (vector-ref state 4))
    (vector-set! state 4 (vector-ref state 3))
    (vector-set! state 3 y20)
    (vector-set! state 2 (vector-ref state 1))
    (vector-set! state 1 (vector-ref state 0))
    (vector-set! state 0 y10)
    dy))

(define (mrg32k3a-pack-state unpacked-state)
  (list->vector (map inexact (vector->list unpacked-state))))

(define (mrg32k3a-unpack-state state)
  (list->vector (map exact (vector->list state))))

(define (mrg32k3a-random-range)
  536870911) ; 2^29-1

(define (mrg32k3a-random-integer state range) ; from Brad Lucier
  (let* ((n (inexact range))
         (q (flfloor (fl/ 4294967087.0 n)))
    (qn (fl* q n)))
    (do ((x (mrg32k3a-random-m1 state) (mrg32k3a-random-m1 state)))
      ((fl<? x qn)
       (exact (flfloor (fl/ x q)))))))

(define (mrg32k3a-random-real state)
  (* 0.0000000002328306549295728 (fl+ 1.0 (mrg32k3a-random-m1 state))))

(define mrg32k3a-m1 4294967087.0) ; modulus of component 1
(define mrg32k3a-m2 4294944443.0) ; modulus of component 2

(define mrg32k3a-initial-state ; 0 3 6 9 12 15 of A^16, see below
  '#(1062452522.0
     2961816100.0 
      342112271.0 
     2854655037.0 
     3321940838.0 
     3542344109.0))

; The value obtained from the system time is feed into a very
; simple pseudo random number generator. This in turn is used
; to obtain numbers to randomize the state of the MRG32k3a
; generator, avoiding period degeneration.

(define (:random-source-current-time)
  (inexact (current-jiffy)))

(define (mrg32k3a-randomize-state/seed state seed)
  ;; G. Marsaglia's simple 16-bit generator with carry
  (let* ((m 65536.0)
         (x (flmodulo seed m))) ;[esl*] was (:random-source-current-time) instead of seed
    (define (random-m)
      (let ((y (flmodulo x m)))
        (set! x (fl+ (fl* 30903.0 y) (flquotient x m)))
        y))
    (define (random n) ; m < n < m^2
      (flmodulo (fl+ (fl* (random-m) m) (random-m)) n))
    ; modify the state
    (let ((m1 mrg32k3a-m1) (m2 mrg32k3a-m2) (s state))
      (vector
        (fl+ 1.0 (flmodulo (fl+ (vector-ref s 0) (random (fl- m1 1.0))) (fl- m1 1.0)))
        (flmodulo (fl+ (vector-ref s 1) (random m1)) m1)
        (flmodulo (fl+ (vector-ref s 2) (random m1)) m1)
        (fl+ 1.0 (flmodulo (fl+ (vector-ref s 3) (random (fl- m2 1.0))) (fl- m2 1.0)))
        (flmodulo (fl+ (vector-ref s 4) (random m2)) m2)
        (flmodulo (fl+ (vector-ref s 5) (random m2)) m2)))))

(define (mrg32k3a-randomize-state state)
  (mrg32k3a-randomize-state/seed state (:random-source-current-time)))

(define (mrg32k3a-pseudo-randomize-state state i j)
  ; [esl] this is a terrible kluge, but will do for now 
  (define seed (inexact (+ i (* 65521.0 j))))  
  (mrg32k3a-randomize-state/seed state seed))

; [esl+]
(define fx-width 
  (do ([x 1 (fxsll x 1)] [n 0 (fx+ n 1)]) [(fxnegative? x) (fx+ n 1)]))
(define fx-greatest 
  (let ([x (fxexpt 2 (fx- fx-width 2))]) (fx+ x (fx- x 1))))

; SRFI interface

(define-record-type :random-source
  (:random-source-make state-ref state-set! randomize! pseudo-randomize! make-integers make-reals)
  :random-source?
  (state-ref :random-source-state-ref)
  (state-set! :random-source-state-set!)
  (randomize! :random-source-randomize!)
  (pseudo-randomize! :random-source-pseudo-randomize!)
  (make-integers :random-source-make-integers)
  (make-reals :random-source-make-reals))

(define (make-random-source)
  (let ((state (vector-copy mrg32k3a-initial-state)))
    (:random-source-make
     (lambda () (vector-copy state))
     (lambda (new-state) (set! state new-state))
     (lambda () (set! state (mrg32k3a-randomize-state state)))
     (lambda (i j) (set! state (mrg32k3a-pseudo-randomize-state state i j)))
     (lambda ()
       (lambda (n)
         (cond
          ((and (integer? n) (<= 1 n (+ 1.0 (inexact fx-greatest)))) ;[esl+] extended range by 1
           (mrg32k3a-random-integer state n))           
          (else (error "range must be an integer in [1..fx-greatest+1]" n)))))
     (lambda args
       (cond
        ((null? args)
         (lambda () 
           (mrg32k3a-random-real state)))
        ((null? (cdr args))
         (let ((unit (car args)))
           (cond
            ((not (and (flonum? unit) (fl<? 0.0 unit 1.0)))
             (error "unit must be real in (0,1)" unit))
            ((fl<=? (fl- (fl/ 1.0 unit) 1.0) mrg32k3a-m1)
             (lambda () 
               (mrg32k3a-random-real state)))
            (else ; skint can't use mp arithmetics
             (error "unsupported unit value" unit)))))
        (else
         (error "illegal arguments" args)))))))

(define random-source? 
  :random-source?)

(define (random-source-state-ref s)
  ((:random-source-state-ref s)))

(define (random-source-state-set! s state)
  ((:random-source-state-set! s) state))

(define (random-source-randomize! s)
  ((:random-source-randomize! s)))

(define (random-source-pseudo-randomize! s i j)
  ((:random-source-pseudo-randomize! s) i j))

(define (random-source-make-integers s)
  ((:random-source-make-integers s)))

(define (random-source-make-reals s . unit)
  (apply (:random-source-make-reals s) unit))  

(define default-random-source 
  (make-random-source))

(define random-integer
  (random-source-make-integers default-random-source))

(define random-real
  (random-source-make-reals default-random-source))

))

