(import (srfi 19))

(include "test.scm")

;; ---------------------------------------------------------------------------
;; Helpers
;; ---------------------------------------------------------------------------
;;
;; Second counts at epoch scale exceed the fixnum range of a build without the
;; numeric tower, where they cannot be read as exact literals at all. They are
;; written as whole flonums throughout, which reads in either build, and
;; compared with time=? or = so that exactness never enters into it.

(define (parts t) (list (time-type t) (time-second t) (time-nanosecond t)))

(define (date-parts d)
  (list (date-nanosecond d) (date-second d) (date-minute d) (date-hour d)
        (date-day d) (date-month d) (date-year d) (date-zone-offset d)))

;; compared numerically: a field may arrive exact or inexact depending on the
;; build and on the route taken, and either way it denotes the same date
(define (same-date? d1 d2)
  (let loop ([a (date-parts d1)] [b (date-parts d2)])
    (cond [(null? a) (null? b)]
          [(null? b) #f]
          [(= (car a) (car b)) (loop (cdr a) (cdr b))]
          [else #f])))

;; a reference moment: Tue 2024-07-09 15:04:05.123456789 -0400
(define ref-date (make-date 123456789 5 4 15 9 7 2024 -14400))


(display "\n--- time type constants ---\n")

(test "time-utc"       'time-utc       time-utc)
(test "time-tai"       'time-tai       time-tai)
(test "time-monotonic" 'time-monotonic time-monotonic)
(test "time-duration"  'time-duration  time-duration)
(test "time-process"   'time-process   time-process)
(test "time-thread"    'time-thread    time-thread)


(display "\n--- time objects ---\n")

(test "time? recognizes a time object" #t (time? (make-time time-utc 0 0)))
(test "time? rejects a non-time"       #f (time? 42))

(test "make-time takes type, nanosecond, second"
  '(time-utc 456 123)
  (parts (make-time time-utc 123 456)))

(test "set-time-type! changes the type"
  'time-duration
  (let ([t (make-time time-utc 5 10)]) (set-time-type! t time-duration) (time-type t)))

(test "set-time-nanosecond! changes the nanosecond"
  999
  (let ([t (make-time time-utc 5 10)]) (set-time-nanosecond! t 999) (time-nanosecond t)))

(test "set-time-second! changes the second"
  20
  (let ([t (make-time time-utc 5 10)]) (set-time-second! t 20) (time-second t)))


(display "\n--- comparisons ---\n")

(test "time=? on equal times"    #t (time=?  (make-time time-utc 0 1) (make-time time-utc 0 1)))
(test "time=? on unequal times"  #f (time=?  (make-time time-utc 0 1) (make-time time-utc 0 2)))
(test "time<? when less"         #t (time<?  (make-time time-utc 0 1) (make-time time-utc 0 2)))
(test "time>? when greater"      #t (time>?  (make-time time-utc 0 2) (make-time time-utc 0 1)))
(test "time<=? when equal"       #t (time<=? (make-time time-utc 0 2) (make-time time-utc 0 2)))
(test "time>=? when equal"       #t (time>=? (make-time time-utc 0 2) (make-time time-utc 0 2)))

(test "nanoseconds order equal seconds"
  '(#t #t #f)
  (let ([a (make-time time-utc 1001 1)] [b (make-time time-utc 2001 1)])
    (list (time<? a b) (time>? b a) (time=? a b))))

(test "time-compare returns -1, 0, 1"
  '(-1 0 1)
  (list (time-compare (make-time time-utc 0 1) (make-time time-utc 0 2))
        (time-compare (make-time time-utc 0 2) (make-time time-utc 0 2))
        (time-compare (make-time time-utc 0 3) (make-time time-utc 0 2))))


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

(test "add-duration adds seconds"
  #t
  (time=? (make-time time-utc 0 1500)
          (add-duration (make-time time-utc 0 1000) (make-time time-duration 0 500))))

(test "subtract-duration subtracts seconds"
  #t
  (time=? (make-time time-utc 0 500)
          (subtract-duration (make-time time-utc 0 1000) (make-time time-duration 0 500))))

(test "adding then subtracting a duration returns to the start"
  #t
  (let ([a (make-time time-utc 12345 1000)] [d (make-time time-duration 54321 77)])
    (time=? a (subtract-duration (add-duration a d) d))))


(display "\n--- TAI and UTC conversions ---\n")

;; Around each leap second insertion the two scales differ by a known number of
;; seconds. For a UTC instant on the boundary, the second before it, the second
;; after it and a fortnight later, converting in either direction must land on
;; the matching instant in the other scale.

(define (utc-probes utc) (list (- utc 1) utc (+ utc 1) (+ utc 1296000)))

(define (tai-probes utc tai-diff prev-diff)
  (list (- (+ utc prev-diff) 1)
        (+ utc tai-diff)
        (+ (+ utc tai-diff) 1)
        (+ (+ utc tai-diff) 1296000)))

(define (utc->tai-seconds utc)
  (map (lambda (s) (time-second (time-utc->time-tai (make-time time-utc 0 s))))
       (utc-probes utc)))

(define (tai->utc-seconds utc tai-diff prev-diff)
  (map (lambda (s) (time-second (time-tai->time-utc (make-time time-tai 0 s))))
       (tai-probes utc tai-diff prev-diff)))

(define (=list a b)
  (and (= (length a) (length b))
       (let loop ([a a] [b b])
         (cond [(null? a) #t]
               [(= (car a) (car b)) (loop (cdr a) (cdr b))]
               [else #f]))))

;; boundary UTC second, offset from it on, offset before it
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
        #t (=list (tai-probes utc tai-diff prev-diff) (utc->tai-seconds utc)))
      (test (string-append "tai->utc across the boundary at " label)
        #t (=list (utc-probes utc) (tai->utc-seconds utc tai-diff prev-diff)))))
  leap-edges)

(test "utc to tai and back is the identity"
  #t
  (let ([t (make-time time-utc 12345 1045789645.0)])
    (time=? t (time-tai->time-utc (time-utc->time-tai t)))))

(test "monotonic time shares the TAI scale"
  #t
  (let ([u (make-time time-utc 0 915148800.0)])
    (= (time-second (time-utc->time-monotonic u))
       (time-second (time-utc->time-tai u)))))

(test "monotonic and tai round-trip through each other"
  #t
  (let ([m (make-time time-monotonic 42 915148832.0)])
    (time=? m (time-tai->time-monotonic (time-monotonic->time-tai m)))))


(display "\n--- dates ---\n")

(test "date? recognizes a date"  #t (date? (make-date 0 0 0 0 1 1 2000 0)))
(test "date? rejects a non-date" #f (date? 42))

(test "make-date takes nanosecond second minute hour day month year offset"
  '(123456789 5 4 15 9 7 2024 -14400)
  (date-parts ref-date))

(test "1970-01-01 was a Thursday" 4 (date-week-day (make-date 0 0 0 0 1 1 1970 0)))
(test "2024-07-09 was a Tuesday"  2 (date-week-day ref-date))
(test "9 July is day 191 of 2024" 191 (date-year-day ref-date))
(test "31 December of a leap year is day 366"
  366 (date-year-day (make-date 0 0 0 0 31 12 2000 0)))

(test "date-week-number counts from the given first day"
  '(1 1)
  (let ([d (make-date 0 0 0 0 1 1 2000 0)])
    (list (date-week-number d 0) (date-week-number d 1))))


(display "\n--- dates and times ---\n")

(test "the epoch converts to second zero"
  #t
  (time=? (make-time time-utc 0 0) (date->time-utc (make-date 0 0 0 0 1 1 1970 0))))

(test "date to time and back is the identity"
  #t
  (let ([d (make-date 0 30 45 12 25 12 2023 0)])
    (same-date? d (time-utc->date (date->time-utc d) 0))))

(test "the leap second at the end of 1998"
  #t
  (same-date? (make-date 0 60 59 23 31 12 1998 0)
              (time-tai->date (make-time time-tai 0 (+ 915148800.0 31)) 0)))

(test "the first second of 1999"
  #t
  (same-date? (make-date 0 0 0 0 1 1 1999 0)
              (time-tai->date (make-time time-tai 0 (+ 915148800.0 32)) 0)))

(test "a date with a zone offset converts to the right UTC instant"
  #t
  (time=? (make-time time-utc 6320000 1045944859.0)
          (date->time-utc (make-date 6320000 19 14 15 22 2 2003 -18000))))

(test "the same date converts to the right TAI instant"
  #t
  (time=? (make-time time-tai 6320000 1045944891.0)
          (date->time-tai (make-date 6320000 19 14 15 22 2 2003 -18000))))


(display "\n--- Julian Day ---\n")

(test "the epoch is JD 2440587.5"
  2440587.5 (time-utc->julian-day (make-time time-utc 0 0)))

(test "1970-01-01 as a date is JD 2440587.5"
  2440587.5 (date->julian-day (make-date 0 0 0 0 1 1 1970 0)))

(test "J2000.0, noon on 2000-01-01, is JD 2451545.0"
  2451545.0 (date->julian-day (make-date 0 0 0 12 1 1 2000 0)))

(test "midnight on 2000-01-01 is JD 2451544.5"
  2451544.5 (date->julian-day (make-date 0 0 0 0 1 1 2000 0)))

(test "0001-01-01 proleptic is JD 1721425.5"
  1721425.5 (date->julian-day (make-date 0 0 0 0 1 1 1 0)))

(test "a leap second and the following midnight share a Julian Day"
  #t
  (= (date->julian-day (make-date 0 60 59 23 31 12 2016 0))
     (date->julian-day (make-date 0 0 0 0 1 1 2017 0))))

(test "current-julian-day returns a number" #t (number? (current-julian-day)))

(test "julian-day->time-utc inverts time-utc->julian-day"
  #t
  (let ([t (make-time time-utc 0 1700000000.0)])
    (time=? t (julian-day->time-utc (time-utc->julian-day t)))))

(test "the TAI converter round-trips"
  #t
  (let ([t (make-time time-tai 0 1700000037.0)])
    (time=? t (julian-day->time-tai (time-tai->julian-day t)))))

(test "the monotonic converter round-trips"
  #t
  (let ([t (make-time time-monotonic 0 1700000037.0)])
    (time=? t (julian-day->time-monotonic (time-monotonic->julian-day t)))))

(test "julian-day->date inverts date->julian-day"
  #t
  (let ([d (make-date 0 0 0 0 1 1 2000 0)])
    (same-date? d (julian-day->date (date->julian-day d) 0))))

(test-error "a non-finite Julian Day is rejected"
  (julian-day->time-utc (/ 1.0 0.0)))

(test-error "a NaN Julian Day is rejected"
  (julian-day->time-utc (/ 0.0 0.0)))


(display "\n--- Modified Julian Day ---\n")

(test "1858-11-17 is MJD zero"
  0.0 (date->modified-julian-day (make-date 0 0 0 0 17 11 1858 0)))

(test "the epoch is MJD 40587.0"
  40587.0 (date->modified-julian-day (make-date 0 0 0 0 1 1 1970 0)))

(test "MJD is JD less 2400000.5"
  #t
  (let ([d (make-date 0 0 0 12 1 1 2024 0)])
    (< (abs (- (- (date->julian-day d) 2400000.5)
               (date->modified-julian-day d)))
       1e-9)))

(test "current-modified-julian-day returns a number"
  #t (number? (current-modified-julian-day)))

(test "modified-julian-day->time-utc round-trips"
  #t
  (let ([t (make-time time-utc 0 1700000000.0)])
    (time=? t (modified-julian-day->time-utc (time-utc->modified-julian-day t)))))

(test "the TAI converter round-trips"
  #t
  (let ([t (make-time time-tai 0 1700000037.0)])
    (time=? t (modified-julian-day->time-tai (time-tai->modified-julian-day t)))))

(test "the monotonic converter round-trips"
  #t
  (let ([t (make-time time-monotonic 0 1700000037.0)])
    (time=? t (modified-julian-day->time-monotonic
               (time-monotonic->modified-julian-day t)))))

(test "modified-julian-day->date inverts date->modified-julian-day"
  #t
  (let ([d (make-date 0 0 0 0 1 1 2000 0)])
    (same-date? d (modified-julian-day->date (date->modified-julian-day d) 0))))


(display "\n--- day-number round-trip sweep ---\n")

;; Both converters quantize the sub-second part, so whole-second instants must
;; survive a round trip exactly across a wide span of dates.

(test "2000 whole-second instants round-trip through JD and MJD"
  '(0 0)
  (let loop ([s 0] [bad-jd 0] [bad-mjd 0])
    (if (= s 2000)
        (list bad-jd bad-mjd)
        (let* ([t  (make-time time-utc 0 (+ 1700000000.0 (* s 9973)))]
               [r1 (julian-day->time-utc (time-utc->julian-day t))]
               [r2 (modified-julian-day->time-utc (time-utc->modified-julian-day t))])
          (loop (+ s 1)
                (if (time=? t r1) bad-jd (+ bad-jd 1))
                (if (time=? t r2) bad-mjd (+ bad-mjd 1)))))))

(test "the JD decoding error stays inside the quantum"
  #t
  (let ([t (make-time time-utc 123456789 1700000000.0)])
    (<= (time-nanosecond (time-difference (julian-day->time-utc (time-utc->julian-day t)) t))
        100000)))

(test "a date near year 6771 still round-trips through JD"
  #t
  (let ([t (date->time-utc (make-date 0 0 0 0 1 1 6771 0))])
    (time=? t (julian-day->time-utc (time-utc->julian-day t)))))

(test "a date near year 3290 still round-trips through MJD"
  #t
  (let ([t (date->time-utc (make-date 0 0 0 0 1 1 3290 0))])
    (time=? t (modified-julian-day->time-utc (time-utc->modified-julian-day t)))))


(display "\n--- date->string ---\n")

(test "numeric date and time fields"
  "2024-07-09 15:04:05"
  (date->string ref-date "~Y-~m-~d ~H:~M:~S"))

(test "weekday and month names"
  "Tue Tuesday Jul July"
  (date->string ref-date "~a ~A ~b ~B"))

(test "~h is a synonym for ~b"
  (date->string ref-date "~b")
  (date->string ref-date "~h"))

(test "padded and unpadded day and hour, and the meridiem"
  "09| 9|15| 3|03|PM"
  (date->string ref-date "~d|~e|~k|~l|~I|~p"))

(test "day of year, nanosecond, two-digit year and zone"
  "191 123456789 24 -0400"
  (date->string ref-date "~j ~N ~y ~z"))

(test "day of week as a number"
  "2"
  (date->string ref-date "~w"))

(test "seconds since the epoch"
  "1704067200"
  (date->string (make-date 0 0 0 0 1 1 2024 0) "~s"))

(test "seconds since the epoch on a leap-second date"
  "1483228800"
  (date->string (make-date 0 60 59 23 31 12 2016 0) "~s"))

(test "week numbers from Sunday, from Monday, and ISO"
  "00|00|01"
  (date->string (make-date 0 0 0 0 1 1 1970 0) "~U|~W|~V"))

(test "an ISO week belonging to the previous year"
  "53"
  (date->string (make-date 0 0 0 0 1 1 2021 0) "~V"))

(test "an ISO week belonging to the following year"
  "01"
  (date->string (make-date 0 0 0 0 30 12 2019 0) "~V"))

(test "the composite date and time forms"
  '("07/09/24" "15:04:05" "03:04:05 PM" "Tue Jul 09 15:04:05-0400 2024")
  (map (lambda (f) (date->string ref-date f)) '("~D" "~X" "~r" "~c")))

(test "the ISO 8601 composite forms"
  '("2024-07-09" "15:04:05-0400" "15:04:05" "2024-07-09T15:04:05-0400" "2024-07-09T15:04:05")
  (map (lambda (f) (date->string ref-date f)) '("~1" "~2" "~3" "~4" "~5")))

(test "literal tilde, newline and tab"
  "100~%\t\n"
  (date->string ref-date "100~~%~t~n"))

(test "every field may be a whole flonum"
  (date->string ref-date "~Y-~m-~d ~H:~M:~S.~N ~a ~w ~z")
  (date->string (make-date 123456789.0 5.0 4.0 15.0 9.0 7.0 2024.0 -14400.0)
                "~Y-~m-~d ~H:~M:~S.~N ~a ~w ~z"))


(display "\n--- string->date ---\n")

(test "the locale date and time format"
  '(14 7 20 -14400 2000)
  (let ([x (string->date "Fri Jul 14 20:28:42-0400 2000" "~a ~b ~d ~H:~M:~S~z ~Y")])
    (list (date-day x) (date-month x) (date-hour x) (date-zone-offset x) (date-year x))))

(test "an ISO 8601 timestamp with a zone offset"
  '(2000 7 14 20 28 42 19800)
  (let ([x (string->date "2000-07-14T20:28:42+05:30" "~Y-~m-~dT~H:~M:~S~z")])
    (list (date-year x) (date-month x) (date-day x)
          (date-hour x) (date-minute x) (date-second x) (date-zone-offset x))))

(test "the twelve-hour clock with a meridiem"
  '(19 0 12 1)
  (map (lambda (s) (date-hour (string->date s "~I:~M:~S ~p")))
       '("07:00:00 PM" "12:00:00 AM" "12:00:00 PM" "01:30:00 AM")))

(test "month names are read case-insensitively"
  '(7 7 1)
  (list (date-month (string->date "JUL" "~b"))
        (date-month (string->date "July" "~B"))
        (date-month (string->date "jan" "~b"))))

(test "weekday names are accepted and discarded"
  #t
  (and (date? (string->date "FRI" "~a")) (date? (string->date "Tuesday" "~A"))))

(test "padded and unpadded numeric fields"
  '(9 9 15 15 4 5 7)
  (list (date-day (string->date "09" "~d"))
        (date-day (string->date " 9" "~e"))
        (date-hour (string->date "15" "~k"))
        (date-hour (string->date "15" "~H"))
        (date-minute (string->date "04" "~M"))
        (date-second (string->date "05" "~S"))
        (date-month (string->date "07" "~m"))))

(test "a two-digit year uses a sliding window"
  '(2074 1999 2000)
  (map (lambda (s) (date-year (string->date s "~y"))) '("74" "99" "00")))

(test "a four-or-more digit year is read as written"
  '(10000 999)
  (map (lambda (s) (date-year (string->date s "~Y"))) '("10000" "999")))

(test "an expanded year may carry a sign"
  '(23000 -1)
  (list (date-year (string->date "+23000-01-01" "~Y-~m-~d"))
        (date-year (string->date "-0001-01-01" "~Y-~m-~d"))))

;; written as flonums so the literals read in a build without the numeric tower,
;; where a nanosecond above about 5*10^8 exceeds the fixnum range
(test "a nanosecond field is read as a plain integer"
  '(0.0 5.0 600000000.0 999999999.0)
  (map (lambda (s) (exact->inexact (date-nanosecond (string->date s "~N"))))
       '("0" "5" "600000000" "999999999")))

(test "a fractional second splits into seconds and nanoseconds"
  '(5 200000000)
  (let ([x (string->date "5.200000000" "~S.~N")])
    (list (date-second x) (inexact->exact (date-nanosecond x)))))

(test "zone offsets in RFC 822 and colon-separated forms"
  '(19800 -14400 0 0 0 -18000)
  (map (lambda (s) (date-zone-offset (string->date s "~z")))
       '("+0530" "-0400" "+0000" "Z" "z" "-05:00")))

(test "literal characters in the template must match"
  '(2024 5 12)
  (let ([x (string->date "2024-05-12" "~Y-~m-~d")])
    (list (date-year x) (date-month x) (date-day x))))

(test-error "a zone offset with hours out of range"
  (string->date "+24:00" "~z"))

(test-error "trailing text the template does not account for"
  (string->date "2024-01-01X" "~Y-~m-~d"))


(display "\n--- string round trips ---\n")

(test "the ISO 8601 composite form reparses to the same fields"
  '(2024 7 9 15 4 5 -14400)
  (let ([x (string->date (date->string ref-date "~4") "~Y-~m-~dT~H:~M:~S~z")])
    (list (date-year x) (date-month x) (date-day x)
          (date-hour x) (date-minute x) (date-second x) (date-zone-offset x))))

(test "the locale format reparses to the same fields"
  '(2024 7 9 15 4 5 -14400)
  (let ([x (string->date (date->string ref-date "~c") "~a ~b ~d ~H:~M:~S~z ~Y")])
    (list (date-year x) (date-month x) (date-day x)
          (date-hour x) (date-minute x) (date-second x) (date-zone-offset x))))

(test "every zone offset the formatter writes is read back unchanged"
  '(19800 -14400 0 5400 -34200)
  (map (lambda (off)
         (date-zone-offset
          (string->date (date->string (make-date 0 0 0 0 1 1 2000 off) "~z") "~z")))
       '(19800 -14400 0 5400 -34200)))


(display "\n--- the clock ---\n")

(test "current-time defaults to UTC" 'time-utc (time-type (current-time)))

(test "current-time honours each time type"
  (list time-utc time-tai time-monotonic time-process time-thread)
  (map (lambda (ty) (time-type (current-time ty)))
       (list time-utc time-tai time-monotonic time-process time-thread)))

(test "time-resolution is a positive integer for each type"
  #t
  (let loop ([tys (list time-utc time-tai time-monotonic time-process time-thread)])
    (cond [(null? tys) #t]
          [(positive? (time-resolution (car tys))) (loop (cdr tys))]
          [else #f])))

(test "current-date returns a date"        #t (date? (current-date)))
(test "current-date accepts a zone offset" 0  (date-zone-offset (current-date 0)))

(test "the UTC clock does not run backwards"
  #t
  (let* ([a (current-time time-utc)] [b (current-time time-utc)]) (time<=? a b)))

;; regression test for a bug reported by Eduardo Cavazos: the nanosecond field
;; must reach the top of its range rather than saturating early
(test "the nanosecond field spans its full range"
  #t
  (> (let loop ([old 0])
       (let ([new (time-nanosecond (current-time))])
         (if (< new old) old (loop new))))
     900000000.0))


(display "\n--- All tests complete. ---\n")

(test-end)
