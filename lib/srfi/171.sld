
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Copyright 2019 Linus Björnstam
;;
;; You may use this code under either the license in the SRFI document or the
;; license below.
;;
;; Permission to use, copy, modify, and/or distribute this software for any
;; purpose with or without fee is hereby granted, provided that the above
;; copyright notice and this permission notice appear in all source copies.
;; The software is provided "as is", without any express or implied warranties.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


(define-library (srfi 171)
  (import (scheme base)
          (scheme case-lambda)
          (scheme write)
          (srfi 9)
          ;(only (scheme vector) vector->list)
          (srfi 69)
          (srfi 171 meta)
          (only (skint) reverse!))
  (export rcons reverse-rcons
          rcount
          rany
          revery

          list-transduce
          vector-transduce
          string-transduce
          bytevector-u8-transduce
          port-transduce
          generator-transduce

          tmap
          tfilter
          tremove
          treplace
          tfilter-map
          tdrop
          tdrop-while
          ttake
          ttake-while
          tconcatenate
          tappend-map
          tdelete-neighbor-duplicates
          tdelete-duplicates
          tflatten
          tsegment
          tpartition
          tadd-between
          tenumerate
          tlog)

(begin

;[esl]* multi-value compose w/o fold
(define (compose . functions)
  (cond ((null? functions) values)
        ((null? (cdr functions)) (car functions))
        (else (let ((f1 (car functions)) (f2 (apply compose (cdr functions))))
                (lambda args (call-with-values (lambda () (apply f2 args)) f1))))))

;; A special value to be used as a placeholder where no value has been set and #f
;; doesn't cut it. Not exported, and not really needed.
(define-record-type <nothing>
  (make-nothing)
  nothing?)
(define nothing (make-nothing))


; [esl-] already exported by the 'meta' sublibrary
;; helper function which ensures x is reduced.
#;(define (ensure-reduced x)
  (if (reduced? x)
      x
      (reduced x)))


; [esl-] already exported by the 'meta' sublibrary
;; helper function that wraps a reduced value twice since reducing functions (like list-reduce)
;; unwraps them. tconcatenate is a good example: it re-uses it's reducer on it's input using list-reduce.
;; If that reduction finishes early and returns a reduced value, list-reduce would "unreduce"
;; that value and try to continue the transducing process.
#;(define (preserving-reduced f)
  (lambda (a b)
    (let ((return (f a b)))
      (if (reduced? return)
          (reduced return)
          return))))


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Reducing functions meant to be used at the end at the transducing
;; process.    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;; a transducer-friendly cons with the empty list as identity
(define rcons
  (case-lambda
    (() '())
    ((lst) (reverse! lst))
    ((lst x) (cons x lst))))


(define reverse-rcons
  (case-lambda
    (() '())
    ((lst) lst)
    ((lst x) (cons x lst))))


;; Use this as the f in transduce to count the amount of elements passed through.
;; (transduce (tfilter odd?) tcount (list 1 2 3)) => 2
(define rcount
  (case-lambda
    (() 0)
    ((result) result)
    ((result input)
     (+ 1 result))))


;; These two take a predicate and returns reducing functions that behave
;; like any and every from srfi-1
(define (rany pred)
  (case-lambda
    (() #f)
    ((result) result)
    ((result input)
     (let ((test (pred input)))
       (if test
           (reduced test)
           #f)))))


(define (revery pred)
  (case-lambda
    (() #t)
    ((result) result)
    ((result input)
     (let ((test (pred input)))
       (if (and result test)
           test
           (reduced #f))))))


(define list-transduce
  (case-lambda
    ((xform f coll)
     (list-transduce xform f (f) coll))
    ((xform f init coll)
     (let* ((xf (xform f))
            (result (list-reduce xf init coll)))
       (xf result)))))

(define vector-transduce
  (case-lambda
    ((xform f coll)
     (vector-transduce xform f (f) coll))
    ((xform f init coll)
     (let* ((xf (xform f))
            (result (vector-reduce xf init coll)))
       (xf result)))))

(define string-transduce
  (case-lambda
    ((xform f coll)
     (string-transduce xform f (f) coll))
    ((xform f init coll)
     (let* ((xf (xform f))
            (result (string-reduce xf init coll)))
       (xf result)))))

(define bytevector-u8-transduce
  (case-lambda
    ((xform f coll)
     (bytevector-u8-transduce xform f (f) coll))
    ((xform f init coll)
     (let* ((xf (xform f))
            (result (bytevector-u8-reduce xf init coll)))
       (xf result)))))

(define port-transduce
  (case-lambda
    ((xform f by)
     (generator-transduce xform f by))
    ((xform f by port)
     (port-transduce xform f (f) by port))
    ((xform f init by port)
     (let* ((xf (xform f))
            (result (port-reduce xf init by port)))
       (xf result)))))

(define generator-transduce
  (case-lambda
    ((xform f gen)
     (generator-transduce xform f (f) gen))
    ((xform f init gen)
     (let* ((xf (xform f))
            (result (generator-reduce xf init gen)))
       (xf result)))))


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Transducers!    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(define (tmap f)
  (lambda (reducer)
    (case-lambda
      (() (reducer))
      ((result) (reducer result)) 
      ((result input)
       (reducer result (f input))))))


(define (tfilter pred)
  (lambda (reducer)
    (case-lambda
      (() (reducer))
      ((result) (reducer result))
      ((result input)
       (if (pred input)
           (reducer result input)
           result)))))


(define (tremove pred)
  (lambda (reducer)
    (case-lambda
      (() (reducer))
      ((result) (reducer result))
      ((result input)
       (if (not (pred input))
           (reducer result input)
           result)))))


(define (tfilter-map f) 
  (compose (tmap f) (tfilter values)))


(define (make-replacer map)
  (cond
   ((list? map)
    (lambda (x)
      (let ((replacer? (assoc x map)))
        (if replacer?
            (cdr replacer?)
            x))))
   ((hash-table? map)
    (lambda (x)
      (hash-table-ref/default map x x)))
   ((procedure? map) map)
   (else
    (error "Unsupported mapping in treplace" map))))


(define (treplace map)
  (tmap (make-replacer map)))


(define (tdrop n)
  (lambda (reducer)
    (let ((new-n (+ 1 n)))
      (case-lambda
        (() (reducer))
        ((result) (reducer result))
        ((result input)
         (set! new-n (- new-n 1))
         (if (positive? new-n)
             result
             (reducer result input)))))))


(define (tdrop-while pred)
  (lambda (reducer)
    (let ((drop? #t))
      (case-lambda
        (() (reducer))
        ((result) (reducer result))
        ((result input)
         (if (and (pred input) drop?)
             result
             (begin
               (set! drop? #f)
               (reducer result input))))))))


(define (ttake n)
  (lambda (reducer)
    ;; we need to reset new-n for every new transduction
    (let ((new-n n))
      (case-lambda
        (() (reducer))
        ((result) (reducer result))
        ((result input)
         (let ((result (if (positive? new-n)
                           (reducer result input)
                           result)))
           (set! new-n (- new-n 1))
           (if (not (positive? new-n))
               (ensure-reduced result)
               result)))))))



(define ttake-while
  (case-lambda
    ((pred) (ttake-while pred (lambda (result input) result)))
    ((pred retf)
     (lambda (reducer)
       (let ((take? #t))
         (case-lambda
           (() (reducer))
           ((result) (reducer result))
           ((result input)
            (if (and take? (pred input))
                (reducer result input)
                (begin
                  (set! take? #f)
                  (ensure-reduced (retf result input)))))))))))



(define (tconcatenate reducer)
  (let ((preserving-reducer (preserving-reduced reducer)))
    (case-lambda
      (() (reducer))
      ((result) (reducer result))
      ((result input)
       (list-reduce preserving-reducer result input)))))


(define (tappend-map f)
  (compose (tmap f) tconcatenate))



;; Flattens everything and passes each value through the reducer
;; (list-transduce tflatten rcons (list 1 2 (list 3 4 '(5 6) 7 8))) => (1 2 3 4 5 6 7 8)
(define tflatten
  (lambda (reducer)
    (case-lambda
      (() '())
      ((result) (reducer result))
      ((result input)
       (if (list? input)
           (list-reduce (preserving-reduced (tflatten reducer)) result input)
           (reducer result input))))))



;; removes duplicate consecutive elements
(define tdelete-neighbor-duplicates
  (case-lambda
    (() (tdelete-neighbor-duplicates equal?))
    ((equality-pred?) 
     (lambda (reducer)
       (let ((prev nothing))
         (case-lambda
           (() (reducer))
           ((result) (reducer result))
           ((result input)
            (if (equality-pred? prev input)
                result
                (begin
                  (set! prev input)
                  (reducer result input))))))))))


;; Deletes all duplicates that passes through.
(define tdelete-duplicates
  (case-lambda
    (() (tdelete-duplicates equal?))
    ((equality-pred?)
     (lambda (reducer)
       (let ((already-seen (make-hash-table equality-pred?)))
         (case-lambda
           (() (reducer))
           ((result) (reducer result))
           ((result input)
            (if (hash-table-exists? already-seen input)
                result
                (begin
                  (hash-table-set! already-seen input #t)
                  (reducer result input))))))))))

;; Partitions the input into lists of N items. If the input stops it flushes whatever
;; it has collected, which may be shorter than n.
(define (tsegment n)
  (if (not (and (integer? n) (positive? n)))
      (error "argument to tsegment must be a positive integer")
      (lambda (reducer)
        (let ((i 0)
              (collect (make-vector n)))
          (case-lambda
            (() (reducer))
            ((result)
             ;; if there is anything collected when we are asked to quit
             ;; we flush it to the remaining transducers
             (let ((result
                    (if (zero? i)
                        result
                        (reducer result (vector->list collect 0 i)))))
               (set! i 0)
               ;; now finally, pass it downstreams
               (if (reduced? result)
                   (reducer (unreduce result))
                   (reducer result))))
            ((result input)
             (vector-set! collect i input)
             (set! i (+ i 1))
             ;; If we have collected enough input we can pass it on downstream
             (if (< i n)
                 result
                 (let ((next-input (vector->list collect 0 i)))
                   (set! i 0)
                   (reducer result next-input)))))))))


(define (tpartition f)
  (lambda (reducer)
    (let* ((prev nothing)
           (collect '()))
      (case-lambda
        (() (reducer))
        ((result)
         (let ((result
                (if (null? collect)
                    result
                    (reducer result (reverse! collect)))))
           (set! collect '())
           (if (reduced? result)
               (reducer (unreduce result))
               (reducer result))))
        ((result input)
         (let ((fout (f input)))
           (cond
            ((or (equal? fout prev) (nothing? prev)) ; collect
             (set! prev fout)
             (set! collect (cons input collect))
             result)
            (else ; flush what we collected already to the reducer
             (let ((next-input  (reverse! collect)))
               (set! prev fout)
               (set! collect (list input))
               (reducer result next-input))))))))))


;; Interposes element between each value pushed through the transduction.
(define (tadd-between elem)
  (lambda (reducer)
    (let ((send-elem? #f))
      (case-lambda
        (() (reducer))
        ((result)
         (reducer result))
        ((result input)
         (if send-elem?
             (let ((result (reducer result elem)))
               (if (reduced? result)
                   result
                   (reducer result input)))
             (begin
               (set! send-elem? #t)
               (reducer result input))))))))


;; indexes every value passed through in a cons pair as in (index . value). By default starts at 0
(define tenumerate
  (case-lambda
    (() (tenumerate 0))
    ((n)
     (lambda (reducer)
       (let ((n n))
         (case-lambda
           (() (reducer))
           ((result) (reducer result))
           ((result input)
            (let ((input (cons n input)))
              (set! n (+ n 1))
              (reducer result input)))))))))


(define tlog
  (case-lambda
    (() (tlog (lambda (result input) (write input) (newline))))
    ((log-function)
     (lambda (reducer)
       (case-lambda
         (() (reducer))
         ((result) (reducer result))
         ((result input)
          (log-function result input)
          (reducer result input)))))))

))
