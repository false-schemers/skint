(import (skint time))

(include "test.scm")

;; ---------------------------------------------------------------------------
;; Helpers
;; ---------------------------------------------------------------------------
;;
;; Second counts at epoch scale exceed the fixnum range of a build without the
;; numeric tower, where they cannot even be read as literals. They are written
;; as whole flonums throughout, which reads in either build, and compared with
;; = or time=? so that exactness never enters into it.

(define (secs t) (time-second t))
(define (nsecs t) (time-nanosecond t))

;; (type second nanosecond) of a time object, as a list
(define (parts t) (list (time-type t) (time-second t) (time-nanosecond t)))


(display "\n--- time type constants ---\n")

(test "time-utc"       'time-utc       time-utc)
(test "time-tai"       'time-tai       time-tai)
(test "time-monotonic" 'time-monotonic time-monotonic)
(test "time-duration"  'time-duration  time-duration)
(test "time-process"   'time-process   time-process)
(test "time-thread"    'time-thread    time-thread)

(test "the six constants are distinct"
  6
  (length (list time-utc time-tai time-monotonic time-duration time-process time-thread)))


(display "\n--- time objects and accessors ---\n")

(test "time? recognizes a time object"    #t (time? (make-time time-utc 0 0)))
(test "time? rejects a non-time"          #f (time? 42))
(test "time? rejects a list"              #f (time? '(time-utc 0 0)))

(test "make-time takes type, nanosecond, second"
  '(time-utc 456 123)
  (parts (make-time time-utc 123 456)))

(test "accessors read back what was stored"
  '(time-duration 7 9)
  (parts (make-time time-duration 9 7)))


(display "\n--- mutators ---\n")

(test "set-time-type! changes the type"
  'time-duration
  (let ([t (make-time time-utc 5 10)]) (set-time-type! t time-duration) (time-type t)))

(test "set-time-nanosecond! changes the nanosecond"
  999
  (let ([t (make-time time-utc 5 10)]) (set-time-nanosecond! t 999) (time-nanosecond t)))

(test "set-time-second! changes the second"
  20
  (let ([t (make-time time-utc 5 10)]) (set-time-second! t 20) (time-second t)))

(test "mutating one field leaves the others alone"
  '(time-utc 10 42)
  (let ([t (make-time time-utc 5 10)]) (set-time-nanosecond! t 42) (parts t)))


(display "\n--- comparisons ---\n")

(test "time=? on equal times"        #t (time=?  (make-time time-utc 0 1) (make-time time-utc 0 1)))
(test "time=? on unequal times"      #f (time=?  (make-time time-utc 0 1) (make-time time-utc 0 2)))
(test "time<? when less"             #t (time<?  (make-time time-utc 0 1) (make-time time-utc 0 2)))
(test "time<? when equal"            #f (time<?  (make-time time-utc 0 2) (make-time time-utc 0 2)))
(test "time>? when greater"          #t (time>?  (make-time time-utc 0 2) (make-time time-utc 0 1)))
(test "time<=? when equal"           #t (time<=? (make-time time-utc 0 2) (make-time time-utc 0 2)))
(test "time<=? when less"            #t (time<=? (make-time time-utc 0 1) (make-time time-utc 0 2)))
(test "time>=? when equal"           #t (time>=? (make-time time-utc 0 2) (make-time time-utc 0 2)))
(test "time>=? when greater"         #t (time>=? (make-time time-utc 0 2) (make-time time-utc 0 1)))

;; nanoseconds break ties between equal second counts
(test "nanoseconds order equal seconds"
  '(#t #t #f)
  (let ([a (make-time time-utc 1000 1)] [b (make-time time-utc 2000 1)])
    (list (time<? a b) (time>? b a) (time=? a b))))

(test "equal seconds and nanoseconds compare equal"
  #t
  (time=? (make-time time-utc 1001 1) (make-time time-utc 1001 1)))

(test "time-compare returns -1, 0, 1"
  '(-1 0 1)
  (list (time-compare (make-time time-utc 0 1) (make-time time-utc 0 2))
        (time-compare (make-time time-utc 0 2) (make-time time-utc 0 2))
        (time-compare (make-time time-utc 0 3) (make-time time-utc 0 2))))

(test "time-compare orders by nanosecond too"
  '(-1 1)
  (list (time-compare (make-time time-utc 1 5) (make-time time-utc 2 5))
        (time-compare (make-time time-utc 2 5) (make-time time-utc 1 5))))

;; comparing across time types is an error
(test-error "comparing utc with a duration"
  (time=? (make-time time-utc 0 1) (make-time time-duration 0 1)))

(test-error "comparing monotonic with tai"
  (time=? (make-time time-monotonic 0 1) (make-time time-tai 0 1)))


(display "\n--- arithmetic ---\n")

(test "time-difference yields a duration"
  'time-duration
  (time-type (time-difference (make-time time-utc 0 3000) (make-time time-utc 0 1000))))

(test "time-difference of later minus earlier"
  #t
  (time=? (make-time time-duration 0 2000)
          (time-difference (make-time time-utc 0 3000) (make-time time-utc 0 1000))))

(test "time-difference of earlier minus later is negative"
  #t
  (time=? (make-time time-duration 0 -2000)
          (time-difference (make-time time-utc 0 1000) (make-time time-utc 0 3000))))

(test "time-difference borrows across the nanosecond field"
  '(0 500000000)
  (let ([r (time-difference (make-time time-utc 0 1) (make-time time-utc 500000000 0))])
    (list (inexact->exact (secs r)) (inexact->exact (nsecs r)))))

(test "add-duration keeps the type of the first argument"
  'time-utc
  (time-type (add-duration (make-time time-utc 0 1000) (make-time time-duration 0 500))))

(test "add-duration adds seconds"
  #t
  (time=? (make-time time-utc 0 1500)
          (add-duration (make-time time-utc 0 1000) (make-time time-duration 0 500))))

(test "add-duration carries out of the nanosecond field"
  '(2 200000000)
  (let ([r (add-duration (make-time time-utc 600000000.0 1)
                         (make-time time-duration 600000000.0 0))])
    (list (inexact->exact (secs r)) (inexact->exact (nsecs r)))))

(test "subtract-duration subtracts seconds"
  #t
  (time=? (make-time time-utc 0 500)
          (subtract-duration (make-time time-utc 0 1000) (make-time time-duration 0 500))))

(test "add-duration leaves its argument unchanged"
  1000
  (let ([a (make-time time-utc 0 1000)])
    (add-duration a (make-time time-duration 0 5))
    (secs a)))

(test "subtract-duration leaves its argument unchanged"
  1000
  (let ([a (make-time time-utc 0 1000)])
    (subtract-duration a (make-time time-duration 0 5))
    (secs a)))

(test "adding then subtracting a duration returns to the start"
  #t
  (let ([a (make-time time-utc 12345 1000)] [d (make-time time-duration 54321 77)])
    (time=? a (subtract-duration (add-duration a d) d))))


(display "\n--- TAI and UTC conversions ---\n")

;; Around each leap second insertion the two scales differ by a known number of
;; seconds. For a UTC instant on the boundary, the second before it and the
;; second after it, converting in either direction must land on the matching
;; instant in the other scale. `tai-diff' is the offset from that boundary on,
;; `prev-diff' the offset that applied before it.

;; the four UTC instants probed around a boundary
(define (utc-probes utc) (list (- utc 1) utc (+ utc 1) (+ utc 1296000)))

;; the TAI instants they should map to
(define (tai-probes utc tai-diff prev-diff)
  (list (- (+ utc prev-diff) 1)
        (+ utc tai-diff)
        (+ (+ utc tai-diff) 1)
        (+ (+ utc tai-diff) 1296000)))

(define (utc->tai-seconds utc)
  (map (lambda (s) (time-second (time-utc->time-tai (make-time time-utc 0 s)))) (utc-probes utc)))

(define (tai->utc-seconds utc tai-diff prev-diff)
  (map (lambda (s) (time-second (time-tai->time-utc (make-time time-tai 0 s))))
       (tai-probes utc tai-diff prev-diff)))

(define (=list a b) (and (= (length a) (length b)) (equal? (map (lambda (x y) (= x y)) a b) (map (lambda (x) #t) a))))

;; each entry: boundary UTC second, offset from it on, offset before it
(define leap-edges
  (list (list 915148800.0 32 31) (list 867715200.0 31 30) (list 820454400.0 30 29)
        (list 773020800.0 29 28) (list 741484800.0 28 27) (list 709948800.0 27 26)
        (list 662688000.0 26 25) (list 631152000.0 25 24) (list 567993600.0 24 23)
        (list 489024000.0 23 22) (list 425865600.0 22 21) (list 394329600.0 21 20)
        (list 362793600.0 20 19) (list 315532800.0 19 18) (list 283996800.0 18 17)
        (list 252460800.0 17 16) (list 220924800.0 16 15) (list 189302400.0 15 14)
        (list 157766400.0 14 13) (list 126230400.0 13 12) (list 94694400.0  12 11)
        (list 78796800.0  11 10) (list 63072000.0  10 0)  (list 0.0 0 0)
        (list 10.0 0 0) (list 1045789645.0 32 32)))

(for-each
  (lambda (edge)
    (let* ([utc (car edge)] [tai-diff (cadr edge)] [prev-diff (caddr edge)]
           [label (number->string utc)])
      (test (string-append "utc->tai across the boundary at " label)
        #t
        (=list (tai-probes utc tai-diff prev-diff) (utc->tai-seconds utc)))
      (test (string-append "tai->utc across the boundary at " label)
        #t
        (=list (utc-probes utc) (tai->utc-seconds utc tai-diff prev-diff)))))
  leap-edges)

(test "converting utc to tai and back is the identity"
  #t
  (let ([t (make-time time-utc 12345 1045789645.0)])
    (time=? t (time-tai->time-utc (time-utc->time-tai t)))))

(test "utc->tai preserves nanoseconds"
  12345
  (time-nanosecond (time-utc->time-tai (make-time time-utc 12345 1045789645.0))))


(display "\n--- monotonic time shares the TAI scale ---\n")

(test "utc->monotonic agrees with utc->tai"
  #t
  (let ([u (make-time time-utc 0 915148800.0)])
    (= (time-second (time-utc->time-monotonic u))
       (time-second (time-utc->time-tai u)))))

(test "monotonic->utc agrees with tai->utc"
  #t
  (= (time-second (time-monotonic->time-utc (make-time time-monotonic 0 915148832.0)))
     (time-second (time-tai->time-utc (make-time time-tai 0 915148832.0)))))

(test "utc->monotonic produces a monotonic time"
  'time-monotonic
  (time-type (time-utc->time-monotonic (make-time time-utc 0 915148800.0))))

(test "monotonic->tai produces a tai time"
  'time-tai
  (time-type (time-monotonic->time-tai (make-time time-monotonic 0 915148832.0))))

(test "tai->monotonic produces a monotonic time"
  'time-monotonic
  (time-type (time-tai->time-monotonic (make-time time-tai 0 915148832.0))))

(test "monotonic and tai round-trip through each other"
  #t
  (let ([m (make-time time-monotonic 42 915148832.0)])
    (time=? m (time-tai->time-monotonic (time-monotonic->time-tai m)))))


(display "\n--- the clock ---\n")

(test "current-time defaults to UTC"
  'time-utc
  (time-type (current-time)))

(test "current-time honours each time type"
  (list time-utc time-tai time-monotonic time-process time-thread)
  (map (lambda (ty) (time-type (current-time ty)))
       (list time-utc time-tai time-monotonic time-process time-thread)))

(test "current-time returns a time object"
  #t
  (time? (current-time)))

(test "time-resolution is a positive integer"
  #t
  (let ([r (time-resolution)]) (and (integer? r) (positive? r))))

(test "time-resolution accepts each time type"
  #t
  (let loop ([tys (list time-utc time-tai time-monotonic time-process time-thread)])
    (cond [(null? tys) #t]
          [(positive? (time-resolution (car tys))) (loop (cdr tys))]
          [else #f])))

(test "the UTC clock does not run backwards"
  #t
  (let* ([a (current-time time-utc)] [b (current-time time-utc)]) (time<=? a b)))

(test "the monotonic clock does not run backwards"
  #t
  (let* ([a (current-time time-monotonic)] [b (current-time time-monotonic)]) (time<=? a b)))


(display "\n--- direct clock readings ---\n")

(test "current-second is a number"       #t (number? (current-second)))
(test "current-microsecond is a number"  #t (number? (current-microsecond)))
(test "process-microsecond is a number"  #t (number? (process-microsecond)))

(test "current-second is positive"       #t (positive? (current-second)))
(test "current-microsecond is positive"  #t (positive? (current-microsecond)))
(test "process-microsecond is not negative" #t (not (negative? (process-microsecond))))

(test "current-microsecond and current-second agree to within a second"
  #t
  (< (abs (- (/ (current-microsecond) 1000000.0) (current-second))) 2.0))

(test "current-timezone-offset is an integer number of seconds"
  #t
  (integer? (current-timezone-offset)))

(test "current-timezone-offset is within a day of UTC"
  #t
  (< (abs (current-timezone-offset)) 86401))


(display "\n--- numeric representation ---\n")

;; Second counts at epoch scale do not fit in a fixnum. Without the numeric
;; tower they are whole flonums; with it they stay exact. Either way they are
;; integers, and compare equal to the same value written the other way.

(test "epoch-scale seconds are integers"
  #t
  (integer? (time-second (time-utc->time-tai (make-time time-utc 0 915148800.0)))))

(test "a whole flonum and an exact integer compare equal as times"
  #t
  (time=? (make-time time-utc 0 456) (make-time time-utc 0 456.0)))

(test "with or without the tower, they are still whole numbers"
      #t
      (= 0 (- (time-second (time-utc->time-tai (make-time time-utc 0 915148800.0)))
              (floor (time-second (time-utc->time-tai (make-time time-utc 0 915148800.0)))))))

;; skint test files are executed as scripts, so this is the standard paradigm for
;; testing feature-dependent parts that rely on a modified reader

(cond-expand 
  ((not full-numeric-tower)
   (display "\n--- All tests complete. ---\n")
   (test-end)))


;; if the script hasn't exited, we are executing in tower mode here

(display "\n--- with the numeric tower ---\n")

(test "an epoch-scale second count may be written exactly"
  915148800
  (time-second (make-time time-utc 0 915148800)))

(test "an exact second count stays exact"
  #t
  (exact? (time-second (make-time time-utc 0 915148800))))

(test "conversion normalizes an inexact argument to exact"
  #t
  (exact? (time-second (time-utc->time-tai (make-time time-utc 0 915148800.0)))))

(test "conversion of an exact argument stays exact"
  #t
  (exact? (time-second (time-utc->time-tai (make-time time-utc 0 915148800)))))

(test "the converted value is the same either way"
  #t
  (= (time-second (time-utc->time-tai (make-time time-utc 0 915148800.0)))
     (time-second (time-utc->time-tai (make-time time-utc 0 915148800)))))

(test "add-duration keeps its result exact"
  #t
  (exact? (time-second (add-duration (make-time time-utc 0 915148800)
                                     (make-time time-duration 0 1)))))

(test "time-difference keeps its result exact"
  #t
  (exact? (time-second (time-difference (make-time time-utc 0 915148800)
                                        (make-time time-utc 0 1)))))

;; Past 2^53 a flonum can no longer distinguish adjacent integers, so exact
;; arithmetic is not merely tidier here — it is the difference between keeping
;; a second count and losing it.

(test "a flonum cannot represent 2^53 + 1"
  #t
  (= (exact->inexact 9007199254740993) (exact->inexact 9007199254740992)))

(test "an exact second count past 2^53 survives intact"
  9007199254740993
  (time-second (make-time time-utc 0 9007199254740993)))

(test "arithmetic past 2^53 stays precise"
  9007199254740994
  (time-second (add-duration (make-time time-utc 0 9007199254740993)
                             (make-time time-duration 0 1))))

(test "two instants one second apart past 2^53 remain distinguishable"
  #t
  (time<? (make-time time-utc 0 9007199254740993)
          (make-time time-utc 0 9007199254740994)))


(display "\n--- All tests complete. ---\n")

(test-end)
