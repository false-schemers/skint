(import (skint date))

(include "test.scm")

;; ---------------------------------------------------------------------------
;; Helpers
;; ---------------------------------------------------------------------------
;;
;; Second counts at epoch scale exceed the fixnum range of a build without the
;; numeric tower, so they are written as whole flonums, which reads in either
;; build, and compared with time=? rather than eqv?.

;; a date at midnight UTC
(define (utc-midnight y m d) (make-date 0 0 0 0 d m y 0))

;; the eight components of a date, as a list
(define (parts d)
  (list (date-nanosecond d) (date-second d) (date-minute d) (date-hour d)
        (date-day d) (date-month d) (date-year d) (date-zone-offset d)))

(define (same-date? d1 d2) (equal? (parts d1) (parts d2)))


(display "\n--- date objects and accessors ---\n")

(test "date? recognizes a date"   #t (date? (utc-midnight 2000 1 1)))
(test "date? rejects a non-date"  #f (date? 42))
(test "date? rejects a time"      #f (date? (make-time time-utc 0 0)))

(test "make-date takes nanosecond second minute hour day month year offset"
  '(123 45 30 12 25 12 2023 3600)
  (parts (make-date 123 45 30 12 25 12 2023 3600)))

(test "accessors read back the epoch"
  '(0 0 0 0 1 1 1970 0)
  (parts (utc-midnight 1970 1 1)))

(test "a negative zone offset is preserved"
  -18000
  (date-zone-offset (make-date 0 0 0 12 4 7 1999 -18000)))

(test "second may be 60 for a leap second"
  60
  (date-second (make-date 0 60 59 23 31 12 1998 0)))


(display "\n--- day of the week ---\n")

;; Sunday = 0
(test "1970-01-01 was a Thursday"  4 (date-week-day (utc-midnight 1970 1 1)))
(test "2000-01-01 was a Saturday"  6 (date-week-day (utc-midnight 2000 1 1)))
(test "1999-01-01 was a Friday"    5 (date-week-day (utc-midnight 1999 1 1)))
(test "1998-12-31 was a Thursday"  4 (date-week-day (utc-midnight 1998 12 31)))
(test "2023-12-25 was a Monday"    1 (date-week-day (utc-midnight 2023 12 25)))
(test "2024-02-29 was a Thursday"  4 (date-week-day (utc-midnight 2024 2 29)))

(test "a whole week advances through every day exactly once"
  '(0 1 2 3 4 5 6)
  (let loop ([d 1] [acc '()])
    (if (> d 7) (reverse acc)
        (loop (+ d 1) (cons (date-week-day (utc-midnight 2024 9 d)) acc)))))


(display "\n--- day of the year ---\n")

(test "1 January is day 1"                1   (date-year-day (utc-midnight 2000 1 1)))
(test "31 December of a leap year is 366" 366 (date-year-day (utc-midnight 2000 12 31)))
(test "31 December of a common year is 365" 365 (date-year-day (utc-midnight 1999 12 31)))
(test "29 February is day 60"             60  (date-year-day (utc-midnight 2024 2 29)))
(test "1 March in a leap year is day 61"  61  (date-year-day (utc-midnight 2000 3 1)))

;; 1900 is divisible by 4 but not by 400, so it is not a leap year
(test "1900 is not a leap year"           60  (date-year-day (utc-midnight 1900 3 1)))
(test "2000 is a leap year"               61  (date-year-day (utc-midnight 2000 3 1)))


(display "\n--- week of the year ---\n")

(test "the first days of 2000, counting weeks from Sunday"
  '(1 2 2)
  (map (lambda (d) (date-week-number (utc-midnight 2000 1 d) 0)) '(1 2 3)))

(test "the same days, counting weeks from Monday"
  '(1 1 2)
  (map (lambda (d) (date-week-number (utc-midnight 2000 1 d) 1)) '(1 2 3)))

(test "the starting day of the week shifts the boundary"
  #t
  (let ([d (utc-midnight 2000 1 2)])
    (not (= (date-week-number d 0) (date-week-number d 1)))))


(display "\n--- date to time ---\n")

(test "the epoch converts to second zero"
  #t
  (time=? (make-time time-utc 0 0) (date->time-utc (utc-midnight 1970 1 1))))

(test "date->time-utc produces a UTC time"
  'time-utc
  (time-type (date->time-utc (utc-midnight 2000 1 1))))

(test "date->time-tai produces a TAI time"
  'time-tai
  (time-type (date->time-tai (utc-midnight 2000 1 1))))

(test "date->time-monotonic produces a monotonic time"
  'time-monotonic
  (time-type (date->time-monotonic (utc-midnight 2000 1 1))))

(test "2000-01-01 is 946684800 seconds after the epoch"
  #t
  (= 946684800.0 (time-second (date->time-utc (utc-midnight 2000 1 1)))))

(test "a zone offset shifts the instant"
  #t
  (= (- (time-second (date->time-utc (make-date 0 0 0 12 15 6 2020 0)))
        (time-second (date->time-utc (make-date 0 0 0 12 15 6 2020 3600))))
     3600))

(test "nanoseconds survive the conversion"
  6320000
  (time-nanosecond (date->time-utc (make-date 6320000 19 14 15 22 2 2003 -18000))))


(display "\n--- time to date ---\n")

(test "second zero converts to the epoch"
  #t
  (same-date? (utc-midnight 1970 1 1) (time-utc->date (make-time time-utc 0 0) 0)))

(test "date to time and back is the identity"
  #t
  (let ([d (make-date 0 30 45 12 25 12 2023 0)])
    (same-date? d (time-utc->date (date->time-utc d) 0))))

(test "the round trip preserves a zone offset"
  -18000
  (date-zone-offset (time-utc->date (date->time-utc (make-date 0 0 0 12 4 7 1999 -18000)) -18000)))

(test "time-tai->date inverts date->time-tai"
  #t
  (let ([d (make-date 0 30 45 12 25 12 2023 0)])
    (same-date? d (time-tai->date (date->time-tai d) 0))))

(test "time-monotonic->date inverts date->time-monotonic"
  #t
  (let ([d (make-date 0 30 45 12 25 12 2023 0)])
    (same-date? d (time-monotonic->date (date->time-monotonic d) 0))))

(test "current-date returns a date"          #t (date? (current-date)))
(test "current-date accepts a zone offset"   0  (date-zone-offset (current-date 0)))


(display "\n--- the leap second at the end of 1998 ---\n")

;; 1998-12-31T23:59:60Z was a real leap second; TAI ran 31 seconds ahead of UTC
;; before it and 32 seconds ahead from 1999-01-01 on.

(test "one second before the leap second"
  #t
  (same-date? (make-date 0 59 59 23 31 12 1998 0)
              (time-tai->date (make-time time-tai 0 (+ 915148800.0 30)) 0)))

(test "the leap second itself"
  #t
  (same-date? (make-date 0 60 59 23 31 12 1998 0)
              (time-tai->date (make-time time-tai 0 (+ 915148800.0 31)) 0)))

(test "the first second of 1999"
  #t
  (same-date? (make-date 0 0 0 0 1 1 1999 0)
              (time-tai->date (make-time time-tai 0 (+ 915148800.0 32)) 0)))

(test "the leap second and the following second share a UTC instant"
  #t
  (time=? (date->time-utc (make-date 0 60 59 23 31 12 1998 0))
          (date->time-utc (make-date 0 0 0 0 1 1 1999 0))))

(test "the second before the leap second is one earlier"
  #t
  (time=? (make-time time-utc 0 (- 915148800.0 1))
          (date->time-utc (make-date 0 59 59 23 31 12 1998 0))))


(display "\n--- zone offsets ---\n")

(test "a date with an offset converts to the right UTC instant"
  #t
  (time=? (make-time time-utc 6320000 1045944859.0)
          (date->time-utc (make-date 6320000 19 14 15 22 2 2003 -18000))))

(test "the same date converts to the right TAI instant"
  #t
  (time=? (make-time time-tai 6320000 1045944891.0)
          (date->time-tai (make-date 6320000 19 14 15 22 2 2003 -18000))))

(test "offsets east and west of UTC differ by their sum"
  #t
  (= (- (time-second (date->time-utc (make-date 0 0 0 12 15 6 2020 -18000)))
        (time-second (date->time-utc (make-date 0 0 0 12 15 6 2020 19800))))
     37800))


(display "\n--- date to ISO 8601 string ---\n")

(test "the epoch"
  "1970-01-01T00:00:00Z"
  (date->iso-8601 (utc-midnight 1970 1 1)))

(test "a zero offset is written as Z"
  "2000-01-01T00:00:00Z"
  (date->iso-8601 (utc-midnight 2000 1 1)))

(test "fields are zero padded"
  "2001-02-03T04:05:06Z"
  (date->iso-8601 (make-date 0 6 5 4 3 2 2001 0)))

(test "the year is padded to four digits"
  "0001-01-01T00:00:00Z"
  (date->iso-8601 (utc-midnight 1 1 1)))

(test "nanoseconds appear as nine fractional digits"
  "2023-12-25T12:45:30.123456789Z"
  (date->iso-8601 (make-date 123456789 30 45 12 25 12 2023 0)))

(test "a fractional part is omitted when the nanosecond is zero"
  "2023-12-25T12:45:30Z"
  (date->iso-8601 (make-date 0 30 45 12 25 12 2023 0)))

(test "a fraction below a tenth keeps its leading zeros"
  "2023-12-25T12:45:30.000000001Z"
  (date->iso-8601 (make-date 1 30 45 12 25 12 2023 0)))

(test "an offset east of UTC"
  "2020-06-15T10:30:00+05:30"
  (date->iso-8601 (make-date 0 0 30 10 15 6 2020 19800)))

(test "an offset west of UTC"
  "1999-07-04T12:00:00-05:00"
  (date->iso-8601 (make-date 0 0 0 12 4 7 1999 -18000)))

(test "a leap second is written as :60"
  "1998-12-31T23:59:60Z"
  (date->iso-8601 (make-date 0 60 59 23 31 12 1998 0)))

(test "a year before the common era takes a sign"
  "-0001-01-01T00:00:00Z"
  (date->iso-8601 (utc-midnight -1 1 1)))

(test "a year beyond four digits takes a leading plus"
  "+12345-01-01T00:00:00Z"
  (date->iso-8601 (utc-midnight 12345 1 1)))

(test-error "date->iso-8601 on a non-date"
  (date->iso-8601 42))

(test-error "date->iso-8601 on a time object"
  (date->iso-8601 (make-time time-utc 0 0)))


(display "\n--- ISO 8601 string to date ---\n")

(test "the epoch"
  '(0 0 0 0 1 1 1970 0)
  (parts (iso-8601->date "1970-01-01T00:00:00Z")))

(test "a full timestamp"
  '(0 6 5 4 3 2 2001 0)
  (parts (iso-8601->date "2001-02-03T04:05:06Z")))

(test "a lowercase z is accepted"
  0
  (date-zone-offset (iso-8601->date "1970-01-01T00:00:00z")))

(test "a lowercase t separator is accepted"
  10
  (date-hour (iso-8601->date "2020-06-15t10:30:00Z")))

(test "a space separator is accepted"
  10
  (date-hour (iso-8601->date "2020-06-15 10:30:00Z")))

(test "an omitted offset is read as UTC"
  0
  (date-zone-offset (iso-8601->date "2020-06-15T10:30:00")))

(test "an offset east of UTC"
  19800
  (date-zone-offset (iso-8601->date "2020-06-15T10:30:00+05:30")))

(test "an offset west of UTC"
  -18000
  (date-zone-offset (iso-8601->date "1999-07-04T12:00:00-05:00")))

(test "nine fractional digits give nanoseconds"
  123456789
  (date-nanosecond (iso-8601->date "2023-12-25T12:45:30.123456789Z")))

(test "a single fractional digit is tenths of a second"
  500000000
  (date-nanosecond (iso-8601->date "2023-12-25T12:45:30.5Z")))

(test "three fractional digits are milliseconds"
  123000000
  (date-nanosecond (iso-8601->date "2023-12-25T12:45:30.123Z")))

(test "a leap second is accepted"
  60
  (date-second (iso-8601->date "1998-12-31T23:59:60Z")))

(test "a signed year is accepted"
  -1
  (date-year (iso-8601->date "-0001-01-01T00:00:00Z")))

(test "a leading plus on the year is accepted"
  2020
  (date-year (iso-8601->date "+2020-06-15T10:30:00Z")))

(test "a year beyond four digits is accepted"
  12345
  (date-year (iso-8601->date "+12345-01-01T00:00:00Z")))


(display "\n--- ISO 8601 round trips ---\n")

(for-each
  (lambda (s)
    (test (string-append "round trip: " s) s (date->iso-8601 (iso-8601->date s))))
  '("1970-01-01T00:00:00Z"
    "2001-02-03T04:05:06Z"
    "2023-12-25T12:45:30.123456789Z"
    "2020-06-15T10:30:00+05:30"
    "1999-07-04T12:00:00-05:00"
    "1998-12-31T23:59:60Z"
    "0001-01-01T00:00:00Z"
    "-0001-01-01T00:00:00Z"
    "+12345-01-01T00:00:00Z"))

(test "a date survives a trip through its string form"
  #t
  (let ([d (make-date 123456789 30 45 12 25 12 2023 19800)])
    (same-date? d (iso-8601->date (date->iso-8601 d)))))


(display "\n--- malformed ISO 8601 input ---\n")

(test-error "an empty string"              (iso-8601->date ""))
(test-error "a non-string argument"        (iso-8601->date 42))
(test-error "no separator after the year"  (iso-8601->date "20200615T103000Z"))
(test-error "a truncated timestamp"        (iso-8601->date "2020-06-15T10"))
(test-error "a month above twelve"         (iso-8601->date "2020-13-15T10:30:00Z"))
(test-error "a day above thirty-one"       (iso-8601->date "2020-06-32T10:30:00Z"))
(test-error "a bad date-time separator"    (iso-8601->date "2020-06-15X10:30:00Z"))
(test-error "a bad zone designator"        (iso-8601->date "2020-06-15T10:30:00Q"))
(test-error "an offset without a colon"    (iso-8601->date "2020-06-15T10:30:00+0530"))
(test-error "a truncated offset"           (iso-8601->date "2020-06-15T10:30:00+05"))

;; skint test files are executed as scripts, so this is the standard paradigm for
;; testing feature-dependent parts that rely on a modified reader

(cond-expand 
  ((not full-numeric-tower)
   (display "\n--- All tests complete. ---\n")
   (test-end)))


;; if the script hasn't exited, we are executing in tower mode here

(display "\n--- with the numeric tower ---\n")

(test "date->time-utc yields an exact second count"
  #t
  (exact? (time-second (date->time-utc (utc-midnight 2000 1 1)))))

(test "date->time-tai yields an exact second count"
  #t
  (exact? (time-second (date->time-tai (utc-midnight 2000 1 1)))))

(test "the epoch-scale value may be written exactly"
  946684800
  (time-second (date->time-utc (utc-midnight 2000 1 1))))

(test "a time built from an exact literal converts to the expected date"
  #t
  (same-date? (utc-midnight 2000 1 1)
              (time-utc->date (make-time time-utc 0 946684800) 0)))

(test "an exact and an inexact second count give the same date"
  #t
  (same-date? (time-utc->date (make-time time-utc 0 946684800) 0)
              (time-utc->date (make-time time-utc 0 946684800.0) 0)))

;; Dates far from the epoch need second counts that a flonum would round. The
;; tower keeps them exact, so distant years survive the round trip.

(test "a year far in the future converts to an exact second count"
  #t
  (exact? (time-second (date->time-utc (utc-midnight 100000 1 1)))))

(test "a year far in the future survives the round trip"
  #t
  (same-date? (utc-midnight 100000 1 1)
              (time-utc->date (date->time-utc (utc-midnight 100000 1 1)) 0)))

(test "a year far in the future writes as an ISO 8601 timestamp"
  "+100000-01-01T00:00:00Z"
  (date->iso-8601 (utc-midnight 100000 1 1)))

(test "and reads back again"
  100000
  (date-year (iso-8601->date "+100000-01-01T00:00:00Z")))

(test "date components remain exact"
  #t
  (let ([d (iso-8601->date "2023-12-25T12:45:30.123456789Z")])
    (and (exact? (date-year d)) (exact? (date-nanosecond d)) (exact? (date-second d)))))


(display "\n--- All tests complete. ---\n")

(test-end)
