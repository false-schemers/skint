(import (srfi 45))
(include "test.scm")


(test-begin "srfi-45")
;=========================================================================
; TESTS AND BENCHMARKS:
;=========================================================================

;=========================================================================
; Memoization test 1:

(define s (delay (begin (display 'hello) 1)))

(test-output "hello"
(force s)
(force s)
               ;===> Should display 'hello once
)

;=========================================================================
; Memoization test 2:

(test-output "bonjour"
(let ((s (delay (begin (display 'bonjour) 2))))
  (+ (force s) (force s)))

               ;===> Should display 'bonjour once
)

;=========================================================================
; Memoization test 3: (pointed out by Alejandro Forero Cuervo) 

(define r (delay (begin (display 'hi) 1)))
(define s (lazy r))
(define t (lazy s))

(test-output "hi"
(force t)
(force r)
               ;===> Should display 'hi once
)

;=========================================================================
; Memoization test 4: Stream memoization 

(define (stream-drop s index)
  (lazy
   (if (zero? index)
       s
       (stream-drop (cdr (force s)) (- index 1)))))

(define (ones)
  (delay (begin
           (display 'ho)
           (cons 1 (ones)))))

(define s (ones))

(test-output "hohohohoho"
(car (force (stream-drop s 4)))
(car (force (stream-drop s 4)))

               ;===> Should display 'ho five times
)

;=========================================================================
; Reentrancy test 1: from R5RS

(define count 0)
(define p
  (delay (begin (set! count (+ count 1))
                (if (> count x)
                    count
                    (force p)))))
(define x 5)
(test 6 (force p))                     ;===>  6
(set! x 10)
(test 6 (force p))                     ;===>  6
       

;=========================================================================
; Reentrancy test 2: from SRFI 40

(define f
  (let ((first? #t))
    (delay
      (if first?
          (begin
            (set! first? #f)
            (force f))
          'second))))

(test 'second (force f))                     ;===> 'second 

;=========================================================================
; Reentrancy test 3: due to John Shutt

(define q
  (let ((count 5))
    (define (get-count) count)
    (define p (delay (if (<= count 0)
                         count
                         (begin (set! count (- count 1))
                                (force p)
                                (set! count (+ count 2))
                                count))))
    (list get-count p)))
(define get-count (car q))
(define p (cadr q))

(test 5 (get-count))  ; =>   5
(test 0 (force p))    ; =>   0
(test 10 (get-count))  ; =>   10

;=========================================================================
; Test leaks:  All the leak tests should run in bounded space.

;=========================================================================
; Leak test 1: Infinite loop in bounded space.

(define (loop) (lazy (loop)))
;(force (loop))                               ;==> bounded space

;=========================================================================
; Leak test 2: Pending memos should not accumulate 
;              in shared structures.

(define s (loop))
;(force s)                                    ;==> bounded space 

;=========================================================================
; Leak test 3: Safely traversing infinite stream.

(define (from n)
  (delay (cons n (from (+ n 1)))))

(define (traverse s)
  (lazy (traverse (cdr (force s)))))

;(force (traverse (from 0)))                  ;==> bounded space

;=========================================================================
; Leak test 4: Safely traversing infinite stream 
;              while pointer to head of result exists.

(define s (traverse (from 0)))  
;(force s)                                    ;==> bounded space

;=========================================================================
; Convenient list deconstructor used below.

(define-syntax match
  (syntax-rules ()
    ((match exp 
       (()      exp1)
       ((h . t) exp2))
     (let ((lst exp))
       (cond ((null? lst) exp1)
             ((pair? lst) (let ((h (car lst))
                                (t (cdr lst)))
                            exp2))
             (else 'match-error))))))

;========================================================================
; Leak test 5: Naive stream-filter should run in bounded space.
;              Simplest case.

(define (stream-filter p? s)
  (lazy (match (force s)
          (()      (delay '())) 
          ((h . t) (if (p? h)
                       (delay (cons h (stream-filter p? t)))
                       (stream-filter p? t))))))

;(force (stream-filter (lambda (n) (= n 10000000000))
;                      (from 0)))
                                             ;==> bounded space

;========================================================================
; Leak test 6: Another long traversal should run in bounded space.

; The stream-ref procedure below does not strictly need to be lazy.  
; It is defined lazy for the purpose of testing safe compostion of 
; lazy procedures in the times3 benchmark below (previous 
; candidate solutions had failed this).  

(define (stream-ref s index)
  (lazy
   (match (force s)
     (()      'error)
     ((h . t) (if (zero? index)
                  (delay h)
                  (stream-ref t (- index 1)))))))

; Check that evenness is correctly implemented - should terminate:

(test 0 (force (stream-ref (stream-filter zero? (from 0))
                   0)))                              ;==> 0

(define s (stream-ref (from 0) 100000000))
;(force s)                                          ;==> bounded space

;======================================================================
; Leak test 7: Infamous example from SRFI 40. 

(define (times3 n)
  (stream-ref (stream-filter
               (lambda (x) (zero? (modulo x n)))
               (from 0))
              3))

(force (times3 7))
;(force (times3 100000000))                        ;==> bounded space

(test-end)
