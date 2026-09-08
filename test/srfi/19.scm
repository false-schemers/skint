(import (srfi 19))

;[esl] count errors
(define *error-count* 0)

;; tests from Larceny 
;; (taken from the SRFI-19 web page on srfi.schemers.org and modified slightly)

(define (writeln . xs)
  (for-each display xs)
  (newline))

(define (fail token . more)
  (set! ultimate-exit-status 1)
  (displayln "Error: test failed: ")
  (writeln token)
  (if (not (null? more))
      (for-each writeln more))
  (newline)
  (set! *error-count* (+ *error-count* 1)) ;[esl]+
  #f)

(or (not (null? (list (current-time 'time-tai)
		      (current-time 'time-utc)
		      (current-time 'time-monotonic)
		      (current-time 'time-thread)
		      (current-time 'time-process))))
    (fail "Creating time structures"))

(or (not (null? (list (time-resolution 'time-tai)
		      (time-resolution 'time-utc)
		      (time-resolution 'time-monotonic)
		      (time-resolution 'time-thread)
		      (time-resolution 'time-process))))
    (fail "Testing time resolutions"))

(or (let ((t1 (make-time 'time-utc 0 1))
	  (t2 (make-time 'time-utc 0 1))
	  (t3 (make-time 'time-utc 0 2))
	  (t11 (make-time 'time-utc 1001 1))
	  (t12 (make-time 'time-utc 1001 1))
	  (t13 (make-time 'time-utc 1001 2)))
      (and (time=? t1 t2)
	   (time>? t3 t2)
	   (time<? t2 t3)
	   (time>=? t1 t2)
	   (time>=? t3 t2)
	   (time<=? t1 t2)
	   (time<=? t2 t3)
	   (time=? t11 t12)
	   (time>? t13 t12)
	   (time<? t12 t13)
	   (time>=? t11 t12)
	   (time>=? t13 t12)
	   (time<=? t11 t12)
	   (time<=? t12 t13)))
    (fail "Time comparisons (time=?, etc.)"))

(or (let ((t1 (make-time 'time-utc 0 3000))
	  (t2 (make-time 'time-utc 0 1000))
	  (t3 (make-time 'time-duration 0 2000))
	  (t4 (make-time 'time-duration 0 -2000)))
      (and (time=? t3 (time-difference t1 t2))
	   (time=? t4 (time-difference t2 t1))))
    (fail "Time difference"))

(define (test-one-utc-tai-edge utc tai-diff tai-last-diff)
  (let* (;; right on the edge they should be the same
	 (utc-basic (make-time 'time-utc 0 utc))
	 (tai-basic (make-time 'time-tai 0 (+ utc tai-diff)))
	 (utc->tai-basic (time-utc->time-tai utc-basic))
	 (tai->utc-basic (time-tai->time-utc tai-basic))
	 ;; a second before they should be the old diff
	 (utc-basic-1 (make-time 'time-utc 0 (- utc 1)))
	 (tai-basic-1 (make-time 'time-tai 0 (- (+ utc tai-last-diff) 1)))
	 (utc->tai-basic-1 (time-utc->time-tai utc-basic-1))
	 (tai->utc-basic-1 (time-tai->time-utc tai-basic-1))
	 ;; a second later they should be the new diff
	 (utc-basic+1 (make-time 'time-utc 0 (+ utc 1)))
	 (tai-basic+1 (make-time 'time-tai 0 (+ (+ utc tai-diff) 1)))
	 (utc->tai-basic+1 (time-utc->time-tai utc-basic+1))
	 (tai->utc-basic+1 (time-tai->time-utc tai-basic+1))
	 ;; ok, let's move the clock half a month or so plus half a second
	 (shy (* 15 24 60 60))
	 (hs (/ (expt 10 9) 2))
	 ;; a second later they should be the new diff
	 (utc-basic+2 (make-time 'time-utc hs (+ utc shy)))
	 (tai-basic+2 (make-time 'time-tai hs (+ (+ utc tai-diff) shy)))
	 (utc->tai-basic+2 (time-utc->time-tai utc-basic+2))
	 (tai->utc-basic+2 (time-tai->time-utc tai-basic+2))
	 )
    (and (time=? utc-basic tai->utc-basic)
	 (time=? tai-basic utc->tai-basic)
	 (time=? utc-basic-1 tai->utc-basic-1)
	 (time=? tai-basic-1 utc->tai-basic-1)
	 (time=? utc-basic+1 tai->utc-basic+1)
	 (time=? tai-basic+1 utc->tai-basic+1)
	 (time=? utc-basic+2 tai->utc-basic+2)
	 (time=? tai-basic+2 utc->tai-basic+2) 
	 )))

(or (and (test-one-utc-tai-edge 915148800.  32 31)
	 (test-one-utc-tai-edge 867715200.  31 30)
	 (test-one-utc-tai-edge 820454400.  30 29)
	 (test-one-utc-tai-edge 773020800.  29 28)
	 (test-one-utc-tai-edge 741484800.  28 27)
	 (test-one-utc-tai-edge 709948800.  27 26)
	 (test-one-utc-tai-edge 662688000.  26 25)
	 (test-one-utc-tai-edge 631152000.  25 24)
	 (test-one-utc-tai-edge 567993600.  24 23)
	 (test-one-utc-tai-edge 489024000.  23 22)
	 (test-one-utc-tai-edge 425865600.  22 21)
	 (test-one-utc-tai-edge 394329600.  21 20)
	 (test-one-utc-tai-edge 362793600.  20 19)
	 (test-one-utc-tai-edge 315532800.  19 18)
	 (test-one-utc-tai-edge 283996800.  18 17)
	 (test-one-utc-tai-edge 252460800.  17 16)
	 (test-one-utc-tai-edge 220924800.  16 15)
	 (test-one-utc-tai-edge 189302400.  15 14)
	 (test-one-utc-tai-edge 157766400.  14 13)
	 (test-one-utc-tai-edge 126230400.  13 12)
	 (test-one-utc-tai-edge 94694400.   12 11)
	 (test-one-utc-tai-edge 78796800.   11 10)
	 (test-one-utc-tai-edge 63072000.   10  0)
	 (test-one-utc-tai-edge 0.           0  0);; at the epoch
	 (test-one-utc-tai-edge 10.          0  0);; close to it ...
	 (test-one-utc-tai-edge 1045789645. 32 32);; about now ...
	 )
    (fail "TAI-UTC Conversions"))

(define (tm:date= d1 d2)
  (and (= (date-year d1) (date-year d2))
       (= (date-month d1) (date-month d2))
       (= (date-day d1) (date-day d2))
       (= (date-hour d1) (date-hour d2))
       (= (date-second d1) (date-second d2))
       (= (date-nanosecond d1) (date-nanosecond d2))
       (= (date-zone-offset d1) (date-zone-offset d2))))

(or (and (tm:date= (time-tai->date (make-time time-tai 0 (+ 915148800. 29)) 0)
		   (make-date 0 58 59 23 31 12 1998 0))
	 (tm:date= (time-tai->date (make-time time-tai 0 (+ 915148800. 30)) 0)
		   (make-date 0 59 59 23 31 12 1998 0))
	 (tm:date= (time-tai->date (make-time time-tai 0 (+ 915148800. 31)) 0)
		   (make-date 0 60 59 23 31 12 1998 0))
	 (tm:date= (time-tai->date (make-time time-tai 0 (+ 915148800. 32)) 0)
		   (make-date 0 0 0 0 1 1 1999 0)))
    (fail "TAI-Date Conversions"))

(or (and (time=? (make-time time-utc 0 (- 915148800. 2))
		 (date->time-utc (make-date 0 58 59 23 31 12 1998 0)))
	 (time=? (make-time time-utc 0 (- 915148800. 1))
		 (date->time-utc (make-date 0 59 59 23 31 12 1998 0)))
	 ;; yes, I think this is acutally right.
	 (time=? (make-time time-utc 0 (- 915148800. 0))
		 (date->time-utc (make-date 0 60 59 23 31 12 1998 0)))
	 (time=? (make-time time-utc 0 (- 915148800. 0))
		 (date->time-utc (make-date 0 0 0 0 1 1 1999 0)))
	 (time=? (make-time time-utc 0 (+ 915148800. 1))
		 (date->time-utc (make-date 0 1 0 0 1 1 1999 0))))
    (fail "Date-UTC Conversions"))

(or (let ((ct-utc (make-time time-utc 6320000 1045944859.))
	  (ct-tai (make-time time-tai 6320000 1045944891.))
	  (cd (make-date 6320000 19 14 15 22 2 2003 -18000)))
      (and (time=? ct-utc (date->time-utc cd))
	   (time=? ct-tai (date->time-tai cd))))
    (fail "TZ Offset conversions"))

;;; regression test for bug reported by Eduardo Cavazos

(let ((nsec (let loop ((old 0))
              (let ((new (time-nanosecond (current-time))))
                (if (< new old)
                    old
                    (loop new))))))
  (or (> nsec 900000000.)
      (fail "nanosecond regression")))

(writeln "Done.")
(exit *error-count*) ;[esl]+

#| 
raw material for additional tests
Note: not all of them are necessarily correct!

(define (assert-equal label expected actual)
  (if (equal? expected actual)
      (display (string-append "PASS: " label "\n"))
      (error (string-append "FAIL: " label) expected actual)))

;; 1. Standard Epochs
(assert-equal "Epoch 1970-01-01 UTC -> JD"
              2440587.5
              (time-utc->julian-day (make-time time-utc 0 0)))

(assert-equal "J2000.0 Epoch -> JD"
              2451545.0
              (date->julian-day (make-date 0 0 0 12 1 1 2000 0)))

(assert-equal "MJD Zero Epoch (1858-11-17)"
              0.0
              (date->modified-julian-day (make-date 0 0 0 0 17 11 1858 0)))

;; 2. Round-trip Whole-Second Stability Test (Quantization fix verification)
(let loop ((s 0) (failures 0))
  (if (= s 100000)
      (assert-equal "100,000 Whole-Second Round-trips (0 Failures)" 0 failures)
      (let* ((t (make-time time-utc 0 (* s 977)))
             (r (julian-day->time-utc (time-utc->julian-day t))))
        (loop (+ s 1) (if (time=? t r) failures (+ failures 1))))))

;; 3. Leap Second Direct Offsetting (Second 60)
(let* ((d60 (make-date 0 60 59 23 31 12 2016 0))
       (jd60 (date->julian-day d60))
       (d59 (make-date 0 59 59 23 31 12 2016 0))
       (jd59 (date->julian-day d59)))
  (assert-equal "Leap second JD is exactly 1 second (1/86400) past 23:59:59"
                (/ 1 86400)
                (- jd60 jd59)))

(define (assert-equal label expected actual)
  (if (equal? expected actual)
      (display (string-append "PASS: " label "\n"))
      (error (string-append "FAIL: " label) expected actual)))

(define (assert-true label condition)
  (if condition
      (display (string-append "PASS: " label "\n"))
      (error (string-append "FAIL: " label))))

;; 1. Epoch Anchors
assert-equal "Epoch 1970-01-01 UTC -> JD"
             2440587.5
             (time-utc->julian-day (make-time time-utc 0 0)))

assert-equal "Epoch 1970-01-01 UTC -> MJD"
             40587.0
             (time-utc->modified-julian-day (make-time time-utc 0 0)))

assert-equal "J2000.0 Epoch -> JD"
             2451545.0
             (date->julian-day (make-date 0 0 0 12 1 1 2000 0)))

assert-equal "MJD Zero Epoch (1858-11-17)"
             0.0
             (date->modified-julian-day (make-date 0 0 0 0 17 11 1858 0)))

;; 2. Leap Second Equivalence (:60 maps to next midnight)
(assert-equal "Leap second :60 date equals next-day midnight JD"
             (date->julian-day (make-date 0 0 0 0 1 1 2017 0))
             (date->julian-day (make-date 0 60 59 23 31 12 2016 0)))

;; 3. Whole-Second Round-trip Sweep (Testing 10 µs and 100 µs quantizers)
(let loop ((s 0) (bad-jd 0) (bad-mjd 0))
  (if (= s 20000)
      (assert-equal "20,000 Whole-Second Round-trips (JD & MJD 0 Failures)"
                    '(0 0)
                    (list bad-jd bad-mjd))
      (let* ((t  (make-time time-utc 0 (+ 1700000000 (* s 9973))))
             (r1 (julian-day->time-utc (time-utc->julian-day t)))
             (r2 (modified-julian-day->time-utc (time-utc->modified-julian-day t))))
        (loop (+ s 1)
              (if (time=? t r1) bad-jd (+ bad-jd 1))
              (if (time=? t r2) bad-mjd (+ bad-mjd 1))))))

;; 4. Sub-Second Decoding Precision Bound
(assert-true "JD decode error stays within 100 µs quantum bound"
             (<= (time-nanosecond
                  (time-difference (julian-day->time-utc
                                    (time-utc->julian-day (make-time time-utc 123456789 1700000000)))
                                   (make-time time-utc 123456789 1700000000)))
                 100000))


;; The four new converters exist and round-trip
(let ((t (make-time time-tai 0 1700000037)))
  (time=? t (modified-julian-day->time-tai (time-tai->modified-julian-day t))))  ; => #t

;; JD and MJD agree to within a few ulp (deliberate ~1 ulp difference, see #5)
(< (abs (- (- (date->julian-day (make-date 0 0 0 12 1 1 2024 0)) 2400000.5)
           (date->modified-julian-day (make-date 0 0 0 12 1 1 2024 0))))
   1e-9)                                                          ; => #t

;; Documented range boundary: JD stays exact near year 6771, breaks past 2^23
(let* ((d (make-date 0 0 0 0 1 1 6771 0))
       (t (date->time-utc d)))
  (time=? t (julian-day->time-utc (time-utc->julian-day t))))     ; => #t

;; MJD range boundary near year 3300
(let* ((d (make-date 0 0 0 0 1 1 3290 0))
       (t (date->time-utc d)))
  (time=? t (modified-julian-day->time-utc (time-utc->modified-julian-day t)))) ; => #t

;; Non-finite input is rejected rather than silently producing NaN (after #4)
(julian-day->time-utc +inf.0)                                     ; => error



(= (date->julian-day (make-date 0 0 0 0 1 1 1970 0)) 2440587.5)
(= (date->modified-julian-day (make-date 0 0 0 0 1 1 1970 0)) 40587.0)
(= (date->julian-day (make-date 0 0 0 0 1 1 2000 0)) 2451544.5)   ; J2000 midnight UTC
(= (date->julian-day (make-date 0 0 0 0 1 1 1 0)) 1721425.5)      ; 0001-01-01 proleptic
(time=? t (julian-day->time-utc (time-utc->julian-day t)))         ; whole-second identity
(= (date->julian-day leap-60-date)                                 ; :60 == next midnight
   (date->julian-day next-midnight-date))

(define d  (make-date 123456789 5 4 15 9 7 2024 -14400))   ; Tue 2024-07-09 15:04:05.123456789 -0400
(define d0 (make-date 0 0 0 0 1 1 1970 0))
(define dl (make-date 0 60 59 23 31 12 2016 0))            ; leap second

(date->string d "~Y-~m-~d ~H:~M:~S")   ; => "2024-07-09 15:04:05"
(date->string d "~a ~A ~b ~B")         ; => "Tue Tuesday Jul July"
(date->string d "~d|~e|~k|~l|~I|~p")   ; => "09| 9|15| 3|03|PM"
(date->string d "~j ~N ~y ~z")         ; => "191 123456789 24 -0400"
(date->string d "~f")                  ; => "05.123456789"   (#5: fraction kept, no padding when 0)
(date->string (make-date 0 5 4 15 9 7 2024 0) "~f")  ; => "05"  (not "05.000000000")
(date->string dl "~s")                 ; => must not error   (#3)
(date->string d0 "~U|~W|~V")           ; => "00|00|01"        (#2, #4)
(date->string (make-date 0 0 0 0 1 1 2021 0) "~V")   ; => "53"
(date->string (make-date 0 0 0 0 30 12 2019 0) "~V") ; => "01"
(date->string d "~4")                  ; => "2024-07-09T15:04:05-0400"
(date->string d "100~~%~t~n")          ; => "100~%<tab><newline>"

;; inexact circulation — every field a whole flonum
(let ((di (make-date 123456789.0 5.0 4.0 15.0 9.0 7.0 2024.0 -14400.0)))
  (equal? (date->string di "~Y-~m-~d ~H:~M:~S.~N ~a ~w ~z")
          (date->string d  "~Y-~m-~d ~H:~M:~S.~N ~a ~w ~z")))   ; => #t

;; ~s at 10-digit magnitude must not go exponential (#6)
(date->string (make-date 0 0 0 0 1 1 2024 0) "~s")   ; => "1704067200"

;; Basic round-trips
(string->date "Fri Jul 14 20:28:42-0400 2000" "~a ~b ~d ~H:~M:~S~z ~Y")
;; => day=14, month=7, hour=20, zone=-14400, year=2000

(string->date "2000-07-14T20:28:42+05:30" "~Y-~m-~dT~H:~M:~S~z")
;; => year=2000, month=7, day=14, hour=20, min=28, sec=42, zone=19800

;; 12-hour clock
(string->date "07:00:00 PM" "~I:~M:~S ~p")   ; => hour 19
(string->date "12:00:00 AM" "~I:~M:~S ~p")   ; => hour 0
(string->date "12:00:00 PM" "~I:~M:~S ~p")   ; => hour 12
(string->date "01:30:00 AM" "~I:~M:~S ~p")   ; => hour 1

;; Nanoseconds (promotion under 46% of values)
(date-nanosecond (string->date "600000000" "~N"))     ; => 600000000.0 (inexact)
(date-nanosecond (string->date "5" "~N"))             ; => 500000000.0 (inexact)
(date-nanosecond (string->date "0" "~N"))             ; => 0 (exact)
(date-nanosecond (string->date "999999999" "~N"))     ; => 999999999.0 (inexact, exact in binary64)

;; Fractional seconds (formatter inverse)
(string->date "5.200000000" "~S.~N")  ; => sec=5, ns=200000000.0 ✓

;; Two-digit years (dynamic "within 50 years" window in 2024)
(string->date "74" "~y")   ; => 2074
(string->date "99" "~y")   ; => 1999
(string->date "00" "~y")   ; => 2000
(string->date "24" "~y")   ; => 2024 (current year edge case)

;; Unbounded year
(string->date "10000" "~Y")   ; => 10000 (not 1000)
(string->date "999" "~Y")     ; => 999

;; Locale names (case-insensitive)
(string->date "FRI" "~a")     ; => does not crash, matches via string-ci=?
(string->date "JAN" "~b")     ; => month 1

;; Literal characters
(string->date "2024-05-12" "~Y-~m-~d")  ; => matches dashes exactly

;; ~N round-trip: 9-digit integer, exact or inexact, no overflow
(equal? 200000000 (string->date "200000000" "~N") ...) ; ns = 200000000.0 okay

;; Large year with sign, unbounded
(string->date "+23000-01-01" "~Y-~m-~d")  ; yr = 23000.0

;; Negative expanded year (formatter <> parser if you want full symmetry)
(string->date "-0001-01-01" "~Y-~m-~d")  ; yr = -1.0

;; ~y sliding window (current 2024)
(string->date "74" "~y")  ; => 2074.0
(string->date "99" "~y")  ; => 1999.0

;; ~z with colon and range guard
(string->date "+05:30" "~z")  ; => 19800
(string->date "+24:00" "~z")  ; => error (hours > 23)

;; Trailing garbage rejected
(string->date "2024-01-01X" "~Y-~m-~d")  ; => error

|#
