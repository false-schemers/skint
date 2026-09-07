(define-library (skint date)

  (import 
    (scheme base) (scheme char) (skint time))

  (export
    ;; Re-exported verbatim from (skint time)
    current-second current-microsecond 
    current-timezone-offset process-microsecond
    time-duration time-monotonic time-utc time-tai time-process time-thread
    make-time time? time-type time-nanosecond time-second
    time-resolution current-time
    time-utc->time-tai time-tai->time-utc
    time-monotonic->time-utc time-monotonic->time-tai
    time-utc->time-monotonic time-tai->time-monotonic
    time-difference add-duration subtract-duration
    time-compare time<=? time<? time=? time>=? time>?

    ;; Date record & accessors
    make-date date? date-nanosecond date-second date-minute date-hour
    date-day date-month date-year date-zone-offset

    ;; Date queries
    date-year-day date-week-day date-week-number current-date

    ;; Time <-> Date conversions
    time-utc->date time-tai->date time-monotonic->date
    date->time-utc date->time-tai date->time-monotonic

    ;; ISO 8601 / RFC 3339 I/O
    date->iso-8601 iso-8601->date)
  (export leap-second-delta) ; debugging only
  
(begin

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

;; date records are immutable
(define-record-type date
  (make-date nanosecond second minute hour day month year zone-offset)
  date?
  (nanosecond  date-nanosecond)   ; integer 0..999,999,999 (inexact whole flonum when > 2^29-1)
  (second      date-second)       ; exact integer 0..60
  (minute      date-minute)       ; exact integer 0..59
  (hour        date-hour)         ; exact integer 0..23
  (day         date-day)          ; exact integer 1..31
  (month       date-month)        ; exact integer 1..12
  (year        date-year)         ; exact integer
  (zone-offset date-zone-offset)) ; exact integer -86400..86400

(define (date-with-second date new-second)
  (make-date (date-nanosecond date)
             (exact new-second)
             (exact (date-minute date))
             (exact (date-hour date))
             (exact (date-day date))
             (exact (date-month date))
             (exact (date-year date))
             (exact (date-zone-offset date))))


;; exactness-agnostic string & digit helpers

(define (digit-char d)
  (integer->char (+ 48 (exact d))))

(define (nat->string n)
  (let loop ((n n) (acc '()))
    (let ((q (floor-quotient  n 10))
          (d (floor-remainder n 10)))
      (let ((acc (cons (digit-char d) acc)))
        (if (zero? q) (list->string acc) (loop q acc))))))

(define (pad0 s w)
  (let ((k (string-length s)))
    (if (>= k w) s (string-append (make-string (- w k) #\0) s))))

(define (fmt-2-digits n) 
  (pad0 (nat->string n) 2))

(define (fmt-4-digits n)
  (let* ((neg (negative? n))
         (abs-n (abs n))
         (s     (nat->string abs-n)))
    (string-append (cond (neg "-") ((> (string-length s) 4) "+") (else ""))
                   (pad0 s 4))))

(define (fmt-nanosecond nsec)
  (if (zero? nsec)
      ""
      (string-append "." (pad0 (nat->string nsec) 9))))

(define (digits->integer str start end field-name)
  (let loop ((i start) (val 0))
    (cond ((= i end) val)
          ((char<=? #\0 (string-ref str i) #\9)
           (loop (+ i 1)
                 (+ (* val 10) (- (char->integer (string-ref str i)) 48))))
          (else (error "iso-8601->date: non-digit in"
                       field-name (substring str start end))))))

(define (parse-year str end)
  (let ((c (string-ref str 0)))
    (cond ((char=? c #\-) (- (digits->integer str 1 end 'year)))
          ((char=? c #\+)    (digits->integer str 1 end 'year))
          (else              (digits->integer str 0 end 'year)))))


;; gregorian calendar arithmetic

(define (leap-year? year)
  (let ((y (exact year)))
    (and (zero? (floor-remainder y 4))
         (or (not (zero? (floor-remainder y 100)))
             (zero? (floor-remainder y 400))))))

(define (days-before-month month year)
  (let* ((m (exact month))
         (cum (case m
                ((1) 0)   ((2) 31)  ((3) 59)  ((4) 90)
                ((5) 120) ((6) 151) ((7) 181) ((8) 212)
                ((9) 243) ((10) 273) ((11) 304) ((12) 334)
                (else (error "days-before-month: invalid month" month)))))
    (if (and (> m 2) (leap-year? year))
        (+ cum 1)
        cum)))

(define (days-since-epoch year month day)
  (let* ((y (- (exact year) 1))
         (leap-days (+ (floor-quotient y 4)
                       (- (floor-quotient y 100))
                       (floor-quotient y 400)))
         (raw-days (+ (* (exact year) 365) leap-days)))
    (+ (- raw-days 719527)
       (days-before-month (exact month) (exact year))
       (- (exact day) 1))))


;; --- Date Queries ---

(define (date-year-day d)
  (+ (days-before-month (date-month d) (date-year d)) (date-day d)))

;; Day of the week: 0 = Sunday, 1 = Monday, ..., 6 = Saturday
(define (date-week-day d)
  (let ((days (days-since-epoch (date-year d) (date-month d) (date-day d))))
    (floor-remainder (+ days 4) 7)))

;; Note: date-week-number returns 1-based weeks where Week 1 contains Jan 1.
;; (Deviates from SRFI-19's 0-based reference implementation for ISO-like 1-based consistency).
;; opt-start-day: 0 = Sunday (default), 1 = Monday, ..., 6 = Saturday.
(define (date-week-number d . opt-start-day)
  (let* ((start-day (if (pair? opt-start-day) (car opt-start-day) 0))
         (y (date-year d))
         (yday (date-year-day d))
         (wday-jan1 (floor-remainder (+ (days-since-epoch y 1 1) 4) 7))
         (offset-jan1 (floor-remainder (- (+ wday-jan1 7) start-day) 7)))
    (+ 1 (floor-quotient (+ (- yday 1) offset-jan1) 7))))


;; --- Time <-> Date Conversions ---

(define (time-utc->date time . opt-tz)
  (unless (and (time? time) (eq? (time-type time) time-utc))
    (error "time-utc->date: expected time-utc object" time))
  (let* ((offset  (if (pair? opt-tz) (car opt-tz) (current-timezone-offset)))
         (adj-sec (+ (time-second time) offset))
         (days    (floor-quotient adj-sec 86400))
         (rem-sec (floor-remainder adj-sec 86400))
         (hour    (floor-quotient rem-sec 3600))
         (m-rem   (floor-remainder rem-sec 3600))
         (min     (floor-quotient m-rem 60))
         (sec     (floor-remainder m-rem 60))
         (est-y   (+ 1970 (floor-quotient days 366))))
    
    (let loop ((y est-y))
      (let ((d-start (days-since-epoch y 1 1)))
        (if (< days d-start)
            (loop (- y 1))
            (let ((d-next (days-since-epoch (+ y 1) 1 1)))
              (if (>= days d-next)
                  (loop (+ y 1))
                  (let* ((d-year (- days d-start))
                         (m (let m-loop ((mon 1))
                              (if (or (= mon 12) (< d-year (days-before-month (+ mon 1) y)))
                                  mon
                                  (m-loop (+ mon 1))))))
                    (make-date (tower-exact (time-nanosecond time))
                               (exact sec)
                               (exact min)
                               (exact hour)
                               (exact (+ (- d-year (days-before-month m y)) 1))
                               (exact m)
                               (exact y)
                               (exact offset))))))))))

#;(define (date->time-utc date)
  (unless (date? date)
    (error "date->time-utc: expected date object" date))
  (when (= (date-second date) 60)
    (error "date->time-utc: second=60 represents a leap second; use date->time-tai" date))
  (let* ((days (days-since-epoch (date-year date) (date-month date) (date-day date)))
         (secs (+ (* days 86400)
                  (* (date-hour date) 3600)
                  (* (date-minute date) 60)
                  (date-second date)
                  (- (date-zone-offset date)))))
    (make-time time-utc (date-nanosecond date) secs)))
;; SRFI-19 semantics: purely arithmetic.  time-utc is a leap-second-blind
;; count, so a leap-second date (second = 60) yields the same time-utc as
;; the following 00:00:00.  Use date->time-tai to preserve the leap second.
(define (date->time-utc date)
  (unless (date? date)
    (error "date->time-utc: expected date object" date))
  (let* ((days (days-since-epoch (date-year date) (date-month date) (date-day date)))
         (secs (+ (* days 86400)
                  (* (date-hour date) 3600)
                  (* (date-minute date) 60)
                  (date-second date)
                  (- (date-zone-offset date)))))
    (make-time time-utc (date-nanosecond date) secs)))

(define (time-tai->date time . opt-tz)
  (unless (and (time? time) (eq? (time-type time) time-tai))
    (error "time-tai->date: expected time-tai object" time))
  (let* ((offset   (if (pair? opt-tz) (car opt-tz) (current-timezone-offset)))
         (utc-time (time-tai->time-utc time))
         (tai-back (time-utc->time-tai utc-time))
         (diff     (- (time-second time) (time-second tai-back))))
    (if (zero? diff)
        (time-utc->date utc-time offset)
        (let* ((prev-tai  (make-time time-tai
                                     (time-nanosecond time)
                                     (- (time-second time) 1)))
               (prev-date (time-utc->date (time-tai->time-utc prev-tai) offset)))
          (date-with-second prev-date 60)))))
#;(define (time-tai->date time . opt-tz)
  (unless (and (time? time) (eq? (time-type time) time-tai))
    (error "time-tai->date: expected time-tai object" time))
  (let* ((offset   (if (pair? opt-tz) (car opt-tz) (current-timezone-offset)))
         (utc-time (time-tai->time-utc time))
         (next-tai (make-time time-tai
                              (time-nanosecond time)
                              (+ (time-second time) 1)))
         (next-utc (time-tai->time-utc next-tai)))
    (if (= (time-second utc-time) (time-second next-utc))
        ;; 1. It's a leap second! Map to :60 of the previous second's date.
        (let* ((prev-tai  (make-time time-tai
                                     (time-nanosecond time)
                                     (- (time-second time) 1)))
               (prev-date (time-utc->date (time-tai->time-utc prev-tai) offset)))
          (tm:date-with-second prev-date 60))
        ;; 2. Normal second. Convert directly.
        (time-utc->date utc-time offset))))
#;(define (time-tai->date time . opt-tz)
  (unless (and (time? time) (eq? (time-type time) time-tai))
    (error "time-tai->date: expected time-tai object" time))
  (let* ((offset   (if (pair? opt-tz) (car opt-tz) (current-timezone-offset)))
         (utc-time (time-tai->time-utc time))
         (tai-back (time-utc->time-tai utc-time))
         (diff     (- (time-second time) (time-second tai-back))))
    (if (zero? diff)
        (time-utc->date utc-time offset)
        ;; Only treat as leap second if the UTC date is the same day
        (let* ((utc-date (time-utc->date utc-time 0))
               (prev-tai (make-time time-tai
                                    (time-nanosecond time)
                                    (- (time-second time) 1)))
               (prev-date (time-utc->date (time-tai->time-utc prev-tai) 0)))
          (if (and (= (date-year utc-date) (date-year prev-date))
                   (= (date-month utc-date) (date-month prev-date))
                   (= (date-day utc-date) (date-day prev-date)))
              ;; Same calendar day - this is a leap second
              (let ((leap-date (date-with-second prev-date 60)))
                (time-utc->date (date->time-utc leap-date) offset))
              ;; Different calendar day - not a leap second
              (time-utc->date utc-time offset))))))

(define (date->time-tai date)
  (unless (date? date)
    (error "date->time-tai: expected date object" date))
  (let ((sec (date-second date)))
    (if (= sec 60)
        (let* ((date-59 (date-with-second date 59))
               (tai-59  (time-utc->time-tai (date->time-utc date-59)))
               (nsec    (time-nanosecond tai-59))
               (sec-59  (time-second tai-59))
               (cand    (make-time time-tai nsec (+ sec-59 1)))
               (check   (time-tai->date cand (date-zone-offset date))))
          (unless (= (date-second check) 60)
            (error "date->time-tai: second=60 is not valid at this date/time" date))
          cand)
        (time-utc->time-tai (date->time-utc date)))))

(define (time-monotonic->date time . opt-tz)
  (unless (and (time? time) (eq? (time-type time) time-monotonic))
    (error "time-monotonic->date: expected time-monotonic object" time))
  (let ((offset (if (pair? opt-tz) (car opt-tz) (current-timezone-offset))))
    (time-tai->date (time-monotonic->time-tai time) offset)))

(define (date->time-monotonic date)
  (unless (date? date)
    (error "date->time-monotonic: expected date object" date))
  (time-tai->time-monotonic (date->time-tai date)))

(define (current-date . opt-tz)
  (let ((offset (if (pair? opt-tz) (car opt-tz) (current-timezone-offset))))
    (time-utc->date (current-time time-utc) offset)))


;; --- ISO 8601 / RFC 3339 Formatting and Parsing ---

(define (date->iso-8601 date)
  (unless (date? date)
    (error "date->iso-8601: expected date object" date))
  (let* ((nsec-str (fmt-nanosecond (date-nanosecond date)))
         (off (date-zone-offset date))
         (off-str (cond ((zero? off) "Z")
                        (else
                         (let* ((abs-off (abs off))
                                (sign    (if (< off 0) "-" "+"))
                                (hours   (floor-quotient abs-off 3600))
                                (mins    (floor-quotient (floor-remainder abs-off 3600) 60)))
                           (string-append sign
                                          (fmt-2-digits hours)
                                          ":"
                                          (fmt-2-digits mins)))))))
    (string-append (fmt-4-digits (date-year date)) "-"
                   (fmt-2-digits (date-month date)) "-"
                   (fmt-2-digits (date-day date)) "T"
                   (fmt-2-digits (date-hour date)) ":"
                   (fmt-2-digits (date-minute date)) ":"
                   (fmt-2-digits (date-second date))
                   nsec-str
                   off-str)))

;; Note: iso-8601->date treats naive timestamps (lacking timezone specification)
;; as UTC (offset 0).
(define (iso-8601->date str)
  (unless (string? str)
    (error "iso-8601->date: expected string" str))
  (let ((len (string-length str)))
    (let* ((year-end (let loop ((i (if (and (> len 0)
                                           (or (char=? (string-ref str 0) #\+)
                                               (char=? (string-ref str 0) #\-)))
                                       1 0)))
                       (cond ((>= i len) (error "iso-8601->date: missing month separator" str))
                             ((char=? (string-ref str i) #\-) i)
                             (else (loop (+ i 1))))))
           (year (parse-year str year-end))
           (idx (+ year-end 1)))
      
      (when (> (+ idx 14) len)
        (error "iso-8601->date: string too short after year" str))
      
      (let* ((month (digits->integer str idx (+ idx 2) 'month))
             (_ (unless (and (>= month 1) (<= month 12))
                  (error "iso-8601->date: month out of range 1..12" month)))
             (_ (unless (char=? (string-ref str (+ idx 2)) #\-)
                  (error "iso-8601->date: expected '-' after month" str)))
             (day (digits->integer str (+ idx 3) (+ idx 5) 'day))
             (_ (unless (and (>= day 1) (<= day 31))
                  (error "iso-8601->date: day out of range 1..31" day)))
             (t-char (string-ref str (+ idx 5)))
             (_ (unless (or (char=? t-char #\T) (char=? t-char #\t) (char=? t-char #\space))
                  (error "iso-8601->date: expected time separator (T or space)" str)))
             (hour (digits->integer str (+ idx 6) (+ idx 8) 'hour))
             (_ (unless (char=? (string-ref str (+ idx 8)) #\:)
                  (error "iso-8601->date: expected ':' after hour" str)))
             (min (digits->integer str (+ idx 9) (+ idx 11) 'minute))
             (_ (unless (char=? (string-ref str (+ idx 11)) #\:)
                  (error "iso-8601->date: expected ':' after minute" str)))
             (sec (digits->integer str (+ idx 12) (+ idx 14) 'second))
             (cur-idx (+ idx 14)))
        
        (define (parse-offset pos nsec)
          (if (>= pos len)
              (make-date (tower-exact nsec) sec min hour day month year 0)
              (let ((ch (string-ref str pos)))
                (cond
                 ((or (char=? ch #\Z) (char=? ch #\z))
                  (make-date (tower-exact nsec) sec min hour day month year 0))
                 ((or (char=? ch #\+) (char=? ch #\-))
                  (when (< (- len pos) 6)
                    (error "iso-8601->date: malformed timezone offset" str))
                  (unless (char=? (string-ref str (+ pos 3)) #\:)
                    (error "iso-8601->date: expected ':' in timezone offset" str))
                  (let* ((sign (if (char=? ch #\-) -1 1))
                         (h    (digits->integer str (+ pos 1) (+ pos 3) 'tz-hour))
                         (m    (digits->integer str (+ pos 4) (+ pos 6) 'tz-minute)))
                    (make-date (tower-exact nsec) sec min hour day month year (* sign (+ (* h 3600) (* m 60))))))
                 (else (error "iso-8601->date: unexpected character at timezone offset" ch))))))

        (if (and (< cur-idx len) (char=? (string-ref str cur-idx) #\.))
            (let loop ((i (+ cur-idx 1)) (digits '()))
              (if (and (< i len) (char<=? #\0 (string-ref str i) #\9))
                  (loop (+ i 1) (cons (string-ref str i) digits))
                  (let* ((num-digits (length digits))
                         (_ (when (zero? num-digits)
                              (error "iso-8601->date: trailing decimal point with no digits" str)))
                         (raw-str (list->string (reverse digits)))
                         (padded-str (if (< num-digits 9)
                                         (string-append raw-str (make-string (- 9 num-digits) #\0))
                                         (substring raw-str 0 9)))
                         (nsec (digits->integer padded-str 0 9 'nanosecond)))
                    (parse-offset i nsec))))
            (parse-offset cur-idx 0))))))

))
