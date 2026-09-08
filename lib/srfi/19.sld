(define-library (srfi 19)

  (import 
    (scheme base) (scheme inexact) (scheme cxr) (scheme char) (scheme write)
    (skint date))  ; (skint date) re-exports (skint time) API

  (export
    ;; Constants
    time-duration time-monotonic time-process time-tai time-thread time-utc
    ;; Current time and clock resolution
    current-date current-julian-day current-modified-julian-day
    current-time time-resolution
    ;; Time object and accessors
    make-time time? time-type time-nanosecond time-second
    set-time-type! set-time-nanosecond! set-time-second!
    copy-time
    ;; Time comparison procedures
    time<=? time<? time=? time>=? time>?
    ;; Time arithmetic procedures
    time-difference (rename time-difference time-difference!)
    add-duration (rename add-duration add-duration!)
    subtract-duration (rename subtract-duration subtract-duration!)
    ;; Date object and accessors
    make-date date? date-nanosecond date-second date-minute date-hour
    date-day date-month date-year date-zone-offset
    date-year-day date-week-day date-week-number 
    ;; Time/Date/Julian Day/Modified Julian Day Converters
    date->julian-day date->modified-julian-day 
    date->time-monotonic date->time-tai date->time-utc
    julian-day->date julian-day->time-monotonic
    julian-day->time-tai julian-day->time-utc
    modified-julian-day->date modified-julian-day->time-monotonic
    modified-julian-day->time-tai modified-julian-day->time-utc
    time-monotonic->date time-monotonic->julian-day
    time-monotonic->modified-julian-day 
    time-monotonic->time-tai (rename time-monotonic->time-tai time-monotonic->time-tai!)
    time-monotonic->time-utc (rename time-monotonic->time-utc time-monotonic->time-utc!)
    time-tai->date time-tai->julian-day time-tai->modified-julian-day 
    time-tai->time-monotonic (rename time-tai->time-monotonic time-tai->time-monotonic!)
    time-tai->time-utc (rename time-tai->time-utc time-tai->time-utc!)
    time-utc->date time-utc->julian-day
    time-utc->modified-julian-day 
    time-utc->time-monotonic (rename time-utc->time-monotonic time-utc->time-monotonic!)
    time-utc->time-tai (rename time-utc->time-tai time-utc->time-tai!)
    ;; Date to String/String to Date Converters
    date->string string->date)
  
(begin

(define (copy-time t)
  (make-time (time-type t) (time-nanosecond t) (time-second t)))

;; Precision-Quantized Seconds Decoder & Internal Helpers

(define (finite-real? x)
  (and (real? x) (finite? x)))

;; Decode total flonum UTC seconds into a make-time object.
;; Snaps sub-seconds to the nearest `quantum` nanoseconds. Whole-second round-trips
;; are exact whenever cumulative float representation error < quantum / 2.
;; Invariant: `quantum` must divide 1000000000.0 exactly.
;; - JD  quantum: 100,000 ns (100 us) - exact whole-second round-trips while
;;   |JD| < 2^23, i.e. roughly year -21,000 to +18,200.
;; - MJD quantum:  10,000 ns  (10 us) - exact whole-second round-trips while
;;   |MJD| < 2^19, i.e. roughly year 500 to 3300.
(define (seconds->time-utc total-sec quantum)
  (let* ((sec-fl   (floor total-sec))
         (nsec-raw (* quantum
                      (round (/ (* (- total-sec sec-fl) 1000000000.0) quantum)))))
    (let-values (((extra-sec final-nsec) (floor/ nsec-raw 1000000000.0)))
      (make-time time-utc final-nsec (+ sec-fl extra-sec)))))


;; Julian Day (JD) Arithmetic
;; Epoch: 1970-01-01 00:00:00 UTC = JD 2440587.5

(define (time-utc->julian-day time)
  (unless (and (time? time) (eq? (time-type time) time-utc))
    (error "time-utc->julian-day: expected time-utc object" time))
  (+ 2440587.5 (/ (+ (time-second time) (/ (time-nanosecond time) 1000000000.0)) 86400.0)))

(define (julian-day->time-utc jd)
  (unless (finite-real? jd)
    (error "julian-day->time-utc: expected finite real number" jd))
  (seconds->time-utc (* (- jd 2440587.5) 86400.0) 100000)) ;; 100 us quantum

(define (date->julian-day date)
  (unless (date? date)
    (error "date->julian-day: expected date object" date))
  (time-utc->julian-day (date->time-utc date))) ; was date->time-utc*

(define (julian-day->date jd . opt-tz)
  (let ((time (julian-day->time-utc jd)))
    (if (pair? opt-tz)
        (time-utc->date time (car opt-tz))
        (time-utc->date time))))

(define (time-tai->julian-day time)
  (unless (and (time? time) (eq? (time-type time) time-tai))
    (error "time-tai->julian-day: expected time-tai object" time))
  ;; Route through date to preserve leap-second alignment table-independently
  (date->julian-day (time-tai->date time 0)))

(define (julian-day->time-tai jd)
  (time-utc->time-tai (julian-day->time-utc jd)))

(define (time-monotonic->julian-day time)
  (unless (and (time? time) (eq? (time-type time) time-monotonic))
    (error "time-monotonic->julian-day: expected time-monotonic object" time))
  (time-tai->julian-day (time-monotonic->time-tai time)))

(define (julian-day->time-monotonic jd)
  (time-tai->time-monotonic (julian-day->time-tai jd)))

(define (current-julian-day)
  (time-utc->julian-day (current-time time-utc)))

;; Modified Julian Day (MJD) Arithmetic
;; Epoch: 1970-01-01 00:00:00 UTC = MJD 40587.0
;; Note: MJD is evaluated directly to maintain maximum floating-point precision;
;; therefore, (- (date->julian-day d) 2400000.5) and (date->modified-julian-day d)
;; deliberately diverge by ~1 ulp in favor of MJD's higher accuracy.

(define (time-utc->modified-julian-day time)
  (unless (and (time? time) (eq? (time-type time) time-utc))
    (error "time-utc->modified-julian-day: expected time-utc object" time))
  (+ 40587.0 (/ (+ (time-second time) (/ (time-nanosecond time) 1000000000.0)) 86400.0)))

(define (modified-julian-day->time-utc mjd)
  (unless (finite-real? mjd)
    (error "modified-julian-day->time-utc: expected finite real number" mjd))
  (seconds->time-utc (* (- mjd 40587.0) 86400.0) 10000)) ;; 10 us quantum

(define (date->modified-julian-day date)
  (unless (date? date)
    (error "date->modified-julian-day: expected date object" date))
  (time-utc->modified-julian-day (date->time-utc date))) ; was date->time-utc*

(define (modified-julian-day->date mjd . opt-tz)
  (let ((time (modified-julian-day->time-utc mjd)))
    (if (pair? opt-tz)
        (time-utc->date time (car opt-tz))
        (time-utc->date time))))

(define (time-tai->modified-julian-day time)
  (unless (and (time? time) (eq? (time-type time) time-tai))
    (error "time-tai->modified-julian-day: expected time-tai object" time))
  ;; Route through date to preserve leap-second alignment
  (date->modified-julian-day (time-tai->date time 0)))

(define (modified-julian-day->time-tai mjd)
  (time-utc->time-tai (modified-julian-day->time-utc mjd)))

(define (time-monotonic->modified-julian-day time)
  (unless (and (time? time) (eq? (time-type time) time-monotonic))
    (error "time-monotonic->modified-julian-day: expected time-monotonic object" time))
  (time-tai->modified-julian-day (time-monotonic->time-tai time)))

(define (modified-julian-day->time-monotonic mjd)
  (time-tai->time-monotonic (modified-julian-day->time-tai mjd)))

(define (current-modified-julian-day)
  (time-utc->modified-julian-day (current-time time-utc)))


;; Locale Data & Formatting Directives

(define locale-number-separator ".")

(define locale-abbr-weekday-vector
  #("Sun" "Mon" "Tue" "Wed" "Thu" "Fri" "Sat"))
(define locale-long-weekday-vector
  #("Sunday" "Monday" "Tuesday" "Wednesday" "Thursday" "Friday" "Saturday"))
(define locale-abbr-month-vector
  #("" "Jan" "Feb" "Mar" "Apr" "May" "Jun" "Jul" "Aug" "Sep" "Oct" "Nov" "Dec"))
(define locale-long-month-vector
  #("" "January" "February" "March" "April" "May" "June" "July" "August" "September" "October" "November" "December"))

(define locale-am "AM")
(define locale-pm "PM")

(define locale-date-time-format "~a ~b ~d ~H:~M:~S~z ~Y")
(define locale-short-date-format "~m/~d/~y")
(define locale-time-format "~H:~M:~S")

(define (nat-str n)
  (let loop ((n n) (acc '()))
    (let ((acc (cons (integer->char (+ 48 (exact (floor-remainder n 10)))) acc))
          (q   (floor-quotient n 10)))
      (if (zero? q) (list->string acc) (loop q acc)))))

(define (int-str n)
  (if (negative? n)
      (string-append "-" (nat-str (- n)))
      (nat-str n)))

(define (padding n pad length)
  (let* ((str (int-str n))
         (len (string-length str)))
    (if (or (not pad) (>= len length))
        str
        (string-append (make-string (- length len) pad) str))))

(define (last-n-digits i n)
  (floor-remainder (abs i) (expt 10 n)))

(define (tz-printer offset port)
  (let ((off (exact offset)))
    (cond
     ((= off 0) (display "Z" port))
     ((negative? off) (display "-" port))
     (else (display "+" port)))
    (unless (= off 0)
      (let* ((abs-off (abs off))
             (hours (quotient abs-off 3600))
             (minutes (quotient (remainder abs-off 3600) 60)))
        (display (padding hours #\0 2) port)
        (display (padding minutes #\0 2) port)))))

;; ISO 8601 week number calculation (~V)
(define (iso-weeks-in-year y)
  (if (or (= 4 (exact (date-week-day (make-date 0 0 0 0 1 1 y 0))))
          (= 4 (exact (date-week-day (make-date 0 0 0 0 31 12 y 0)))))
      53 52))

(define (iso-week-number d)
  (let* ((y      (date-year d))
         (yday   (exact (date-year-day d)))
         (iso-wd (+ 1
                    (floor-remainder
                     (+ (exact (date-week-day d)) 6)
                     7)))
         (w      (floor-quotient (+ (- yday iso-wd) 10) 7)))
    (cond ((< w 1)                        (iso-weeks-in-year (- y 1)))
          ((> w (iso-weeks-in-year y)) 1)
          (else                           w))))

;; strftime %U / %W week number calculation (0 = Sunday, 1 = Monday)
(define (strftime-week-number d start-day)
  (let* ((yday0 (- (exact (date-year-day d)) 1))
         (dow   (floor-remainder (- (exact (date-week-day d)) start-day) 7)))
    (floor-quotient (+ yday0 (- 7 dow)) 7)))

;; Strict ISO 8601 year and date formatting helpers (~1, ~4, ~5)
(define (iso-year-string y)
  (let* ((neg?   (negative? y))
         (digits (nat-str (if neg? (- y) y)))
         (width  (string-length digits))
         (body   (if (< width 4)
                     (string-append
                      (make-string (- 4 width) #\0)
                      digits)
                     digits))
         (sign   (cond (neg? "-")
                       ((> width 4) "+")
                       (else ""))))
    (string-append sign body)))

(define (iso-date-string d)
  (string-append
   (iso-year-string (date-year d))
   "-"
   (padding (date-month d) #\0 2)
   "-"
   (padding (date-day d) #\0 2)))

(define directives
  `((#\~ . ,(lambda (d pad p) (display #\~ p)))
    (#\a . ,(lambda (d pad p) (display (vector-ref locale-abbr-weekday-vector (exact (date-week-day d))) p)))
    (#\A . ,(lambda (d pad p) (display (vector-ref locale-long-weekday-vector (exact (date-week-day d))) p)))
    (#\b . ,(lambda (d pad p) (display (vector-ref locale-abbr-month-vector (exact (date-month d))) p)))
    (#\h . ,(lambda (d pad p) (display (vector-ref locale-abbr-month-vector (exact (date-month d))) p)))
    (#\B . ,(lambda (d pad p) (display (vector-ref locale-long-month-vector (exact (date-month d))) p)))
    (#\c . ,(lambda (d pad p) (display (date->string d locale-date-time-format) p)))
    (#\d . ,(lambda (d pad p) (display (padding (date-day d) pad 2) p)))
    (#\D . ,(lambda (d pad p) (display (date->string d "~m/~d/~y") p)))
    (#\e . ,(lambda (d pad p) (display (padding (date-day d) #\space 2) p)))
    (#\f . ,(lambda (d pad p)
              (display (int-str (date-second d)) p)
              (display locale-number-separator p)
              (display (padding (date-nanosecond d) #\0 9) p)))
    (#\H . ,(lambda (d pad p) (display (padding (date-hour d) pad 2) p)))
    (#\I . ,(lambda (d pad p)
              (let* ((hr (exact (date-hour d)))
                     (h12 (cond ((= hr 0) 12) ((> hr 12) (- hr 12)) (else hr))))
                (display (padding h12 pad 2) p))))
    (#\j . ,(lambda (d pad p) (display (padding (date-year-day d) pad 3) p)))
    (#\k . ,(lambda (d pad p) (display (padding (date-hour d) #\space 2) p)))
    (#\l . ,(lambda (d pad p)
              (let* ((hr (exact (date-hour d)))
                     (h12 (cond ((= hr 0) 12) ((> hr 12) (- hr 12)) (else hr))))
                (display (padding h12 #\space 2) p))))
    (#\m . ,(lambda (d pad p) (display (padding (date-month d) pad 2) p)))
    (#\M . ,(lambda (d pad p) (display (padding (date-minute d) pad 2) p)))
    (#\n . ,(lambda (d pad p) (newline p)))
    (#\N . ,(lambda (d pad p) (display (padding (date-nanosecond d) pad 9) p)))
    (#\p . ,(lambda (d pad p) (display (if (> (exact (date-hour d)) 11) locale-pm locale-am) p)))
    (#\r . ,(lambda (d pad p) (display (date->string d "~I:~M:~S ~p") p)))
    (#\s . ,(lambda (d pad p) (display (int-str (time-second (date->time-utc d))) p))) ; was date->time-utc*
    (#\S . ,(lambda (d pad p) (display (padding (date-second d) pad 2) p)))
    (#\t . ,(lambda (d pad p) (write-char #\tab p)))
    (#\T . ,(lambda (d pad p) (display (date->string d "~H:~M:~S") p)))
    (#\U . ,(lambda (d pad p) (display (padding (strftime-week-number d 0) #\0 2) p)))
    (#\V . ,(lambda (d pad p) (display (padding (iso-week-number d) #\0 2) p)))
    (#\w . ,(lambda (d pad p) (display (exact (date-week-day d)) p)))
    (#\x . ,(lambda (d pad p) (display (date->string d locale-short-date-format) p)))
    (#\X . ,(lambda (d pad p) (display (date->string d locale-time-format) p)))
    (#\W . ,(lambda (d pad p) (display (padding (strftime-week-number d 1) #\0 2) p)))
    (#\y . ,(lambda (d pad p) (display (padding (last-n-digits (date-year d) 2) pad 2) p)))
    (#\Y . ,(lambda (d pad p) (display (int-str (date-year d)) p)))
    (#\z . ,(lambda (d pad p) (tz-printer (date-zone-offset d) p)))
    (#\Z . ,(lambda (d pad p) #f))
    (#\1 . ,(lambda (d pad p) (display (iso-date-string d) p)))
    (#\2 . ,(lambda (d pad p) (display (date->string d "~H:~M:~S~z") p)))
    (#\3 . ,(lambda (d pad p) (display (date->string d "~H:~M:~S") p)))
    (#\4 . ,(lambda (d pad p) (display (string-append (iso-date-string d) "T" (date->string d "~H:~M:~S~z")) p)))
    (#\5 . ,(lambda (d pad p) (display (string-append (iso-date-string d) "T" (date->string d "~H:~M:~S")) p)))))

(define (get-formatter char)
  (let ((associated (assoc char directives)))
    (if associated (cdr associated) #f)))

(define (date->string date . format-string)
  (let* ((fmt (if (pair? format-string) (car format-string) "~c"))
         (len (string-length fmt))
         (port (open-output-string)))
    (let loop ((i 0))
      (if (>= i len)
          (get-output-string port)
          (let ((c (string-ref fmt i)))
            (if (not (char=? c #\~))
                (begin
                  (write-char c port)
                  (loop (+ i 1)))
                (if (>= (+ i 1) len)
                    (error "date->string: trailing tilde in format string" fmt)
                    (let* ((m (string-ref fmt (+ i 1)))
                           (has-mod? (or (char=? m #\-) (char=? m #\_)))
                           (pad (cond ((char=? m #\-) #f)
                                      ((char=? m #\_) #\space)
                                      (else #\0)))
                           (dir-idx (if has-mod? (+ i 2) (+ i 1))))
                      (if (>= dir-idx len)
                          (error "date->string: incomplete directive" fmt)
                          (let ((dir-char (string-ref fmt dir-idx)))
                            (cond
                             ((char=? dir-char #\~)
                              (write-char #\~ port))
                             ((get-formatter dir-char)
                              => (lambda (f) (f date pad port)))
                             (else
                              (error "date->string: unknown format directive" dir-char)))
                            (loop (+ dir-idx 1))))))))))))


;; Date Parsing & Scanning (Production Ready & SRFI-19 Compliant)

(define (char->int ch)
  (let ((n (- (char->integer ch) 48)))
    (if (and (>= n 0) (<= n 9))
        n
        (error "string->date: non-digit character" ch))))

(define (char-ascii-numeric? ch)
  (and (char? ch)
       (char>=? ch #\0)
       (char<=? ch #\9)))

(define (integer-reader upto port)
  (let loop ((nchars 0) (accum 0))
    (let ((ch (peek-char port)))
      (if (or (eof-object? ch)
              (not (char-ascii-numeric? ch))
              (and upto (>= nchars upto)))
          accum
          (begin
            (read-char port)
            (loop (+ nchars 1) (+ (* accum 10) (char->int ch))))))))

(define (make-integer-reader upto)
  (lambda (port)
    (integer-reader upto port)))

(define (integer-reader-exact n port)
  (let loop ((nchars 0) (accum 0) (padding-ok #t))
    (if (>= nchars n)
        accum
        (let ((ch (peek-char port)))
          (cond
           ((eof-object? ch)
            (error "string->date: premature EOF reading integer"))
           ((char-ascii-numeric? ch)
            (read-char port)
            (loop (+ nchars 1) (+ (* accum 10) (char->int ch)) #f))
           (padding-ok
            (read-char port)
            (loop (+ nchars 1) accum #t))
           (else
            (error "string->date: non-numeric character in integer field" ch)))))))

(define (make-integer-exact-reader n)
  (lambda (port)
    (integer-reader-exact n port)))

(define (signed-integer-reader port)
  (let ((ch (peek-char port)))
    (cond
     ((and (not (eof-object? ch)) (char=? ch #\-))
      (read-char port)
      (let ((next-ch (peek-char port)))
        (if (or (eof-object? next-ch) (not (char-ascii-numeric? next-ch)))
            (error "string->date: expected digits after '-' in year field")
            (- (integer-reader #f port)))))
     ((and (not (eof-object? ch)) (char=? ch #\+))
      (read-char port)
      (let ((next-ch (peek-char port)))
        (if (or (eof-object? next-ch) (not (char-ascii-numeric? next-ch)))
            (error "string->date: expected digits after '+' in year field")
            (integer-reader #f port))))
     (else
      (let ((next-ch (peek-char port)))
        (if (or (eof-object? next-ch) (not (char-ascii-numeric? next-ch)))
            (error "string->date: expected digits in year field")
            (integer-reader #f port)))))))

(define (year-start? ch)
  (or (char-ascii-numeric? ch)
      (char=? ch #\+)
      (char=? ch #\-)))

(define (natural-year n)
  (let* ((current-year (date-year (current-date)))
         (current-century (* 100 (floor-quotient current-year 100)))
         (trial (+ current-century n)))
    (cond
     ((> (- trial current-year) 50) (- trial 100))
     ((< (- trial current-year) -50) (+ trial 100))
     (else trial))))

(define (natural-year-reader port)
  (let ((val (integer-reader-exact 2 port)))
    (natural-year val)))

(define (zone-reader port)
  (let ((ch (read-char port)))
    (if (eof-object? ch)
        (error "string->date: unexpected EOF reading time zone")
        (cond
         ((or (char=? ch #\Z) (char=? ch #\z)) 0)
         ((or (char=? ch #\+) (char=? ch #\-))
          (let* ((sign (if (char=? ch #\-) -1 1))
                 (h1 (read-char port))
                 (h2 (read-char port)))
            (if (or (eof-object? h1) (eof-object? h2)
                    (not (char-ascii-numeric? h1)) (not (char-ascii-numeric? h2)))
                (error "string->date: malformed time zone hours")
                (let ((hours (+ (* (char->int h1) 10) (char->int h2))))
                  (when (> hours 23)
                    (error "string->date: time zone hours out of range" hours))
                  (when (and (not (eof-object? (peek-char port)))
                             (char=? (peek-char port) #\:))
                    (read-char port))
                  (let* ((m1 (read-char port)) (m2 (read-char port)))
                    (if (or (eof-object? m1) (eof-object? m2)
                            (not (char-ascii-numeric? m1)) (not (char-ascii-numeric? m2)))
                        (error "string->date: malformed time zone minutes")
                        (let ((mins (+ (* (char->int m1) 10) (char->int m2))))
                          (when (> mins 59)
                            (error "string->date: time zone minutes out of range" mins))
                          (* sign (+ (* hours 3600) (* mins 60))))))))))
         (else (error "string->date: invalid time zone character" ch))))))

(define (vector-index vec str start-idx)
  (let loop ((i start-idx))
    (cond ((>= i (vector-length vec)) #f)
          ((string-ci=? (vector-ref vec i) str) i)
          (else (loop (+ i 1))))))

(define (locale-abbr-weekday->index str)
  (vector-index locale-abbr-weekday-vector str 0))
(define (locale-long-weekday->index str)
  (vector-index locale-long-weekday-vector str 0))
(define (locale-abbr-month->index str)
  (vector-index locale-abbr-month-vector str 1))
(define (locale-long-month->index str)
  (vector-index locale-long-month-vector str 1))

(define (locale-reader port indexer)
  (let ((string-port (open-output-string)))
    (define (read-char-string)
      (let ((ch (peek-char port)))
        (if (and (not (eof-object? ch)) (char-alphabetic? ch))
            (begin (write-char (read-char port) string-port)
                   (read-char-string))
            (get-output-string string-port))))
    (let* ((str (read-char-string))
           (index (if (> (string-length str) 0) (indexer str) #f)))
      (if index
          index
          (error "string->date: invalid locale string" str)))))

(define (make-locale-reader indexer)
  (lambda (port)
    (locale-reader port indexer)))

(define (ampm-reader port)
  (let ((string-port (open-output-string)))
    (let loop ()
      (let ((ch (peek-char port)))
        (if (and (not (eof-object? ch)) (char-alphabetic? ch))
            (begin (write-char (read-char port) string-port) (loop))
            (let ((str (get-output-string string-port)))
              (cond
               ((string-ci=? str "pm") 'pm)
               ((string-ci=? str "am") 'am)
               (else (error "string->date: expected AM or PM" str)))))))))

(define (make-char-id-reader char)
  (lambda (port)
    (let ((ch (read-char port)))
      (if (and (not (eof-object? ch)) (char=? char ch))
          char
          (error "string->date: invalid literal character match" char)))))

(define read-directives
  (let ((ireader9 (make-integer-reader 9))
        (ireader2 (make-integer-reader 2))
        (eireader2 (make-integer-exact-reader 2))
        (locale-reader-abbr-weekday (make-locale-reader locale-abbr-weekday->index))
        (locale-reader-long-weekday (make-locale-reader locale-long-weekday->index))
        (locale-reader-abbr-month   (make-locale-reader locale-abbr-month->index))
        (locale-reader-long-month   (make-locale-reader locale-long-month->index))
        (char-fail (lambda (ch) #t)))
    (list
     (list #\~ char-fail (make-char-id-reader #\~) 'none)
     (list #\a char-alphabetic? locale-reader-abbr-weekday 'none)
     (list #\A char-alphabetic? locale-reader-long-weekday 'none)
     (list #\b char-alphabetic? locale-reader-abbr-month 'mon)
     (list #\B char-alphabetic? locale-reader-long-month 'mon)
     (list #\d char-ascii-numeric? ireader2 'day)
     (list #\e char-fail eireader2 'day)
     (list #\h char-alphabetic? locale-reader-abbr-month 'mon)
     (list #\H char-ascii-numeric? ireader2 'hr)
     (list #\I char-ascii-numeric? ireader2 'hr)
     (list #\k char-fail eireader2 'hr)
     (list #\l char-fail eireader2 'hr)
     (list #\m char-ascii-numeric? ireader2 'mon)
     (list #\M char-ascii-numeric? ireader2 'min)
     (list #\N char-ascii-numeric? ireader9 'ns)
     (list #\p char-alphabetic? ampm-reader 'ampm)
     (list #\S char-ascii-numeric? ireader2 'sec)
     (list #\y char-fail natural-year-reader 'yr)
     (list #\Y year-start? signed-integer-reader 'yr)
     (list #\z (lambda (c)
                 (or (char=? c #\Z) (char=? c #\z)
                     (char=? c #\+) (char=? c #\-)))
           zone-reader 'tz))))

(define (skip-until port skipper template-string)
  (let loop ()
    (let ((ch (peek-char port)))
      (if (eof-object? ch)
          (error "string->date: unexpected EOF matching template" template-string)
          (unless (skipper ch)
            (read-char port)
            (loop))))))

(define (string->date input-string template-string)
  (let ((port (open-input-string input-string))
        (str-len (string-length template-string)))
    (let loop ((index 0) (ns 0) (sec 0) (min 0) (hr 0) (day 1) (mon 1)
               (yr 1970) (tz (current-timezone-offset)))
      (if (>= index str-len)
          (if (eof-object? (peek-char port))
              (make-date ns sec min hr day mon yr tz)
              (error "string->date: trailing input after template"))
          (let ((current-char (string-ref template-string index)))
            (if (not (char=? current-char #\~))
                (let ((port-char (read-char port)))
                  (if (or (eof-object? port-char)
                          (not (char=? current-char port-char)))
                      (error "string->date: input does not match template literal" current-char)
                      (loop (+ index 1) ns sec min hr day mon yr tz)))
                (if (>= (+ index 1) str-len)
                    (error "string->date: trailing tilde in template" template-string)
                    (let* ((format-char (string-ref template-string (+ index 1)))
                           (format-info (assoc format-char read-directives)))
                      (if (not format-info)
                          (error "string->date: unknown directive" format-char)
                          (let ((skipper (cadr format-info))
                                (reader  (caddr format-info))
                                (field   (cadddr format-info)))
                            (skip-until port skipper template-string)
                            (let ((val (reader port)))
                              (if (eof-object? val)
                                  (error "string->date: premature EOF reading field" format-char)
                                  (let ((ns   (if (eq? field 'ns) val ns))
                                        (sec  (if (eq? field 'sec) val sec))
                                        (min  (if (eq? field 'min) val min))
                                        (hr   (cond ((and (eq? field 'ampm) (eq? val 'pm) (< hr 12)) (+ hr 12))
                                                    ((and (eq? field 'ampm) (eq? val 'am) (= hr 12)) 0)
                                                    ((eq? field 'hr) val)
                                                    (else hr)))
                                        (day  (if (eq? field 'day) val day))
                                        (mon  (if (eq? field 'mon) val mon))
                                        (yr   (if (eq? field 'yr) val yr))
                                        (tz   (if (eq? field 'tz) val tz)))
                                    (loop (+ index 2) ns sec min hr day mon yr tz))))))))))))))

))
