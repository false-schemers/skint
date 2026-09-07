(define-library (skint time)

  (import (scheme base) (scheme inexact)
          (only (skint hidden) %microtime %microclock %tz-offset))

  (export current-second current-microsecond 
          current-timezone-offset process-microsecond
          time-duration time-monotonic time-utc time-tai time-process time-thread
          make-time time? time-type time-nanosecond time-second
          set-time-type! set-time-nanosecond! set-time-second!
          time-resolution current-time
          time-utc->time-tai time-tai->time-utc
          time-monotonic->time-utc time-monotonic->time-tai
          time-utc->time-monotonic time-tai->time-monotonic
          time-difference add-duration subtract-duration
          time-compare time<=? time<? time=? time>=? time>?)

(begin

;; SKINT builtins:
;; (%microtime)   -> flonum (whole microseconds since POSIX epoch)
;; (%microclock)  -> flonum (whole microseconds spent in the process)
;; (%tz-offset)   -> fixnum (local UTC offset in seconds)

;; cast to exact if tower is supported
(cond-expand
  (full-numeric-tower
   (define-syntax tower-exact
     (syntax-rules ()
       ((_ exp) (let ((x exp)) (if (integer? x) (exact x) x))))))
  (else
   (define-syntax tower-exact
     (syntax-rules ()
       ((_ exp) exp)))))

;; improved R7RS (scheme time) standard procedure
(define (current-second)
  (let ((utc-us (%microtime)))
    (when (nan? utc-us)
      (error "current-second: system clock unavailable"))
    (let ((utc-sec (/ utc-us 1000000.0)))
      (tower-exact (+ utc-sec (leap-second-delta utc-sec))))))

;; microsecond-precision TAI anchor
(define (current-microsecond)
  (let ((utc-us (%microtime)))
    (when (nan? utc-us)
      (error "current-microsecond: system clock unavailable"))
    (let ((utc-sec (/ utc-us 1000000.0)))
      (tower-exact (+ utc-us (* (leap-second-delta utc-sec) 1000000.0))))))

;; current host timezone offset
(define (current-timezone-offset)
  (%tz-offset))

(define (process-microsecond)
  (let ((us (%microclock)))
    (when (nan? us)
      (error "process-microsecond: CPU clock unavailable"))
    (tower-exact us)))


;; SRFI 19 -like layer

;; --- Time Type Identifiers ---
(define time-duration  'time-duration)  ; Time interval/difference
(define time-monotonic 'time-monotonic) ; Monotonic clock point
(define time-utc       'time-utc)       ; Coordinated Universal Time
(define time-tai       'time-tai)       ; International Atomic Time
(define time-process   'time-process)   ; CPU time spent in process
(define time-thread    'time-thread)    ; CPU time spent in current thread

;; --- time record definition (immutable) ---
(define-record-type time
  (%make-time type nanosecond second)
  time?
  (type       time-type set-time-type!)
  (nanosecond time-nanosecond set-time-nanosecond!)
  (second     time-second set-time-second!))

(define (make-time type nanosecond second)
  (%make-time type (tower-exact nanosecond) (tower-exact second)))

(define *leap-second-table*
  '((1483228800.0 . 37)  ; 2017-01-01
    (1435708800.0 . 36)
    (1341100800.0 . 35)
    (1230768000.0 . 34)
    (1136073600.0 . 33)
    (915148800.0  . 32)
    (867715200.0  . 31)
    (820454400.0  . 30)
    (773020800.0  . 29)
    (741484800.0  . 28)
    (709948800.0  . 27)
    (662688000.0  . 26)
    (631152000.0  . 25)
    (567993600.0  . 24)
    (489024000.0  . 23)
    (425865600.0  . 22)
    (394329600.0  . 21)
    (362793600.0  . 20)
    (315532800.0  . 19)
    (283996800.0  . 18)
    (252460800.0  . 17)
    (220924800.0  . 16)
    (189302400.0  . 15)
    (157766400.0  . 14)
    (126230400.0  . 13)
    (94694400.0   . 12)
    (78796800.0   . 11)
    (63072000.0   . 10)))

(define (leap-second-delta utc-sec)
  (let loop ((table *leap-second-table*))
    (cond
      ((null? table) 0)
      ((>= utc-sec (caar table)) (cdar table))
      (else (loop (cdr table))))))


;; --- core clock queries ---

(define (time-resolution . opt-type)
  (let ((type (if (pair? opt-type) (car opt-type) time-utc)))
    (case type ; we always return 100 for 1 microsecond resolution in nanoseconds
      ((time-utc time-tai time-monotonic time-process time-thread time-duration) 1000)
      (else (error "time-resolution: unsupported time type" type)))))

(define (current-time . opt-type)
  (let ((type (if (pair? opt-type) (car opt-type) time-utc)))   
    (define (decompose-microsecs us error-msg return)
      (when (nan? us)
        (error error-msg))
      (let* ((sec (floor (/ us 1000000.0)))
             (nsec (* (- us (* sec 1000000.0)) 1000.0)))
        (return sec nsec)))
    (case type
      ((time-utc)
       (decompose-microsecs (%microtime) "current-time: system clock unavailable"
         (lambda (sec nsec)
           (make-time time-utc nsec sec))))
      ((time-tai time-monotonic)
       (decompose-microsecs (%microtime) "current-time: system clock unavailable"
         (lambda (sec nsec)
           (let ((tai-sec (+ sec (leap-second-delta sec))))
             (make-time type nsec tai-sec)))))
      ((time-process time-thread)
       (decompose-microsecs (%microclock) "current-time: process CPU clock unavailable"
         (lambda (sec nsec)
           (make-time type nsec sec))))
      (else
       (error "current-time: unsupported time type" type)))))


;; --- core conversions (time-monotonic == time-tai) ---

(define (utc->tai t out-type)
  (let* ((sec (time-second t))
         (delta (leap-second-delta sec)))
    (make-time out-type (time-nanosecond t) (+ sec delta))))

(define (tai->utc t out-type)
  (let* ((tai-sec (time-second t))
         (approx-utc (- tai-sec 37.0))  ; Start with max known offset
         (delta1 (leap-second-delta approx-utc))
         (utc-refined (- tai-sec delta1))
         (delta2 (leap-second-delta utc-refined)))
    (make-time out-type (time-nanosecond t) (- tai-sec delta2))))

(define (time-utc->time-tai t)
  (unless (eq? (time-type t) time-utc)
    (error "time-utc->time-tai: expected time-utc object" t))
  (utc->tai t time-tai)) 

(define (time-tai->time-utc t)
  (unless (eq? (time-type t) time-tai)
    (error "time-tai->time-utc: expected time-tai object" t))
  (tai->utc t time-utc))

(define (time-monotonic->time-utc t)
  (unless (eq? (time-type t) time-monotonic)
    (error "time-monotonic->time-utc: expected time-monotonic object" t))
  (tai->utc t time-utc))

(define (time-monotonic->time-tai t)
  (unless (eq? (time-type t) time-monotonic)
    (error "time-monotonic->time-tai: expected time-monotonic object" t))
  (make-time time-tai (time-nanosecond t) (time-second t)))

(define (time-utc->time-monotonic t)
  (unless (eq? (time-type t) time-utc)
    (error "time-utc->time-monotonic: expected time-utc object" t))
  (utc->tai t time-monotonic))

(define (time-tai->time-monotonic t)
  (unless (eq? (time-type t) time-tai)
    (error "time-tai->time-monotonic: expected time-tai object" t))
  (make-time time-monotonic (time-nanosecond t) (time-second t)))


;; --- time operations ---

(define nanosecond-base 1000000000.0)

(define (check-time-type-match proc-name t1 t2)
  (unless (and (time? t1) (time? t2))
    (error (string-append proc-name ": expected time objects") t1 t2))
  (unless (eq? (time-type t1) (time-type t2))
    (error (string-append proc-name ": time type mismatch") 
           (time-type t1) (time-type t2))))

;; normalize absolute time records: nanoseconds must be in [0, 1e9)
(define (normalize-time-absolute type sec nsec)
  (let* ((sec-adjust (floor (/ nsec nanosecond-base)))
         (ns-rem (- nsec (* sec-adjust nanosecond-base))))
    (make-time type ns-rem (+ sec sec-adjust))))

;; normalize duration records: fields must share the same sign, nanoseconds in (-1e9, 1e9)
(define (normalize-time-duration sec nsec)
  (let* ((sec-adjust (truncate (/ nsec nanosecond-base)))
         (ns-rem (- nsec (* sec-adjust nanosecond-base)))
         (s-res (+ sec sec-adjust)))
    (cond
      ((and (not (zero? s-res)) (not (zero? ns-rem))
            (not (eqv? (negative? s-res) (negative? ns-rem))))
       ;; opposite signs: borrow 1.0 from s-res and adjust ns-rem
       (if (negative? s-res)
           (make-time time-duration (- ns-rem nanosecond-base) (+ s-res 1.0))
           (make-time time-duration (+ ns-rem nanosecond-base) (- s-res 1.0))))
      (else
       ;; same sign or a component is zero
       (make-time time-duration ns-rem s-res)))))


;; --- duration arithmetic ---

;; (time-difference time1 time2) -> time-duration
(define (time-difference t1 t2)
  (check-time-type-match "time-difference" t1 t2)
  (normalize-time-duration (- (time-second t1) (time-second t2))
                           (- (time-nanosecond t1) (time-nanosecond t2))))

;; (add-duration time1 duration) -> time
(define (add-duration t1 duration)
  (unless (and (time? t1) (time? duration))
    (error "add-duration: expected time objects" t1 duration))
  (unless (eq? (time-type duration) time-duration)
    (error "add-duration: second argument must be time-duration" (time-type duration)))
  (normalize-time-absolute (time-type t1)
                           (+ (time-second t1) (time-second duration))
                           (+ (time-nanosecond t1) (time-nanosecond duration))))

;; (subtract-duration time1 duration) -> time
(define (subtract-duration t1 duration)
  (unless (and (time? t1) (time? duration))
    (error "subtract-duration: expected time objects" t1 duration))
  (unless (eq? (time-type duration) time-duration)
    (error "subtract-duration: second argument must be time-duration" (time-type duration)))
  (normalize-time-absolute (time-type t1)
                           (- (time-second t1) (time-second duration))
                           (- (time-nanosecond t1) (time-nanosecond duration))))


;; --- comparison procedures ---

;; primitive 3-way comparator (-1, 0, +1)
(define (time-compare t1 t2)
  (check-time-type-match "time-compare" t1 t2)
  (let ((s1 (time-second t1))
        (s2 (time-second t2)))
    (cond
      ((< s1 s2) -1)
      ((> s1 s2)  1)
      (else
       (let ((ns1 (time-nanosecond t1))
             (ns2 (time-nanosecond t2)))
         (cond
           ((< ns1 ns2) -1)
           ((> ns1 ns2)  1)
           (else 0)))))))

(define (time=? t1 t2)
  (= (time-compare t1 t2) 0))

(define (time<? t1 t2)
  (= (time-compare t1 t2) -1))

(define (time<=? t1 t2)
  (<= (time-compare t1 t2) 0))

(define (time>? t1 t2)
  (= (time-compare t1 t2) 1))

(define (time>=? t1 t2)
  (>= (time-compare t1 t2) 0))

))
     