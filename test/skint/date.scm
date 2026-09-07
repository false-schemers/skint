;;; simple test procedures

(import (skint date))

(define s19-tests (list))

(define (define-s19-test! name thunk)
  (let ((name (if (symbol? name) name (string->symbol name)))
	(pr (assoc name s19-tests)))
    (if pr
	(set-cdr! pr thunk)
	(set! s19-tests (append s19-tests (list (cons name thunk)))))))

(define (run-s19-test name thunk verbose)
  (if verbose (begin (display ";;; Running ") (display name)))
  (let ((result (thunk)))
    (if verbose (begin (display ": ") (display (not (not result))) (newline)))
    result))

(define (run-s19-tests . verbose)
  (let ((runs 0) (goods 0) (bads 0) (verbose (if (cdr verbose) (cdr verbose) #f)))
    (for-each (lambda (pr)
		(set! runs (+ runs 1))
		(if (run-s19-test (car pr) (cdr pr) verbose)
		    (set! goods (+ goods 1))
		    (set! bads (+ bads 1))))
	      s19-tests)
    (if verbose
	(begin
	  (display ";;; Results: Runs: ")
	  (display runs)
	  (display "; Goods: ")
	  (display goods)
	  (display "; Bads: ")
	  (display bads)
	  (if (> runs 0)
	      (begin
		(display "; Pass rate: ")
		(display (/ goods runs)))
	      (display "; No tests."))
	  (newline)))
    (exit bads)))

(set! s19-tests (list))

(define (tm:date= d1 d2)
  (and (= (date-year d1) (date-year d2))
       (= (date-month d1) (date-month d2))
       (= (date-day d1) (date-day d2))
       (= (date-hour d1) (date-hour d2))
       (= (date-second d1) (date-second d2))
       (= (date-nanosecond d1) (date-nanosecond d2))
       (= (date-zone-offset d1) (date-zone-offset d2))))

(define-s19-test! "TAI-Date Conversions"
  (lambda ()
    (and
     (tm:date= (time-tai->date (make-time time-tai 0 (+ 915148800.0 29)) 0)
	       (make-date 0 58 59 23 31 12 1998 0))
     (tm:date= (time-tai->date (make-time time-tai 0 (+ 915148800.0 30)) 0)
	       (make-date 0 59 59 23 31 12 1998 0))
     (tm:date= (time-tai->date (make-time time-tai 0 (+ 915148800.0 31)) 0)
	       (make-date 0 60 59 23 31 12 1998 0))
     (tm:date= (time-tai->date (make-time time-tai 0 (+ 915148800.0 32)) 0)
	       (make-date 0 0 0 0 1 1 1999 0)))))

(define-s19-test! "Date-UTC Conversions"
  (lambda ()
    (and
     (time=? (make-time time-utc 0 (- 915148800.0 2))
	     (date->time-utc (make-date 0 58 59 23 31 12 1998 0)))
     (time=? (make-time time-utc 0 (- 915148800.0 1))
	     (date->time-utc (make-date 0 59 59 23 31 12 1998 0)))
     ;; yes, I think this is acutally right.
     (time=? (make-time time-utc 0 (- 915148800.0 0))
	     (date->time-utc (make-date 0 60 59 23 31 12 1998 0)))
     (time=? (make-time time-utc 0 (- 915148800.0 0))
	     (date->time-utc (make-date 0 0 0 0 1 1 1999 0)))
     (time=? (make-time time-utc 0 (+ 915148800.0 1))
	     (date->time-utc (make-date 0 1 0 0 1 1 1999 0))))))

(define-s19-test! "TZ Offset conversions"
  (lambda ()
    (let ((ct-utc (make-time time-utc 6320000 1045944859.0))
	  (ct-tai (make-time time-tai 6320000 1045944891.0))
	  (cd (make-date 6320000 19 14 15 22 2 2003 -18000)))
      (and
       (time=? ct-utc (date->time-utc cd))
       (time=? ct-tai (date->time-tai cd))))))

(begin (newline) (run-s19-tests #t))
