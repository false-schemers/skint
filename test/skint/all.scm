(import (skint))
(import (only (skint hidden) file-resolve-relative-to-current %system %command-line))

(define *tests* '(
ivset
))

(define *results* '())

(define (cmdline-base)
  (let loop ([l (%command-line)] [b ""])
    (if (or (null? l) (null? (cdr l)))
        b 
        (loop (cdr l) (string-append b (car l) " ")))))

(define (run-test n)
  ; assumes skint is in the current directory 
  (define fname (format "~a.scm" n))
  (define cmd (format "~a ~a" (cmdline-base) (file-resolve-relative-to-current fname)))
  (format #t "~%~%----------------------------------------------------------------------------~%")
  (format #t "RUNNING TESTS FOR LIBRARY ~a: ~a~%" n cmd)
  (format #t "----------------------------------------------------------------------------~%")
  (let ([res (%system cmd)])
    (set! *results* (cons (cons n res) *results*))))

(for-each run-test *tests*)

(format #t "~%~%----------------------------------------------------------------------------~%")
(format #t "CUMULATIVE RESULTS:~%")
(format #t "----------------------------------------------------------------------------~%~%")

(define *total-fail-count* 0)

(define (show-result n&r)
  (if (zero? (cdr n&r))
      (format #t "~a: \tok~%" (car n&r))
      (format #t "~a: \t~a failed~%" (car n&r) (cdr n&r)))
  (set! *total-fail-count* (+ *total-fail-count* (cdr n&r))) 
)
 
(for-each show-result (reverse *results*))

(format #t "~%~%LIBRARIES TESTED: ~a~%" (length *tests*))
(format #t "CUMULATIVE FAILURES: ~a~%" *total-fail-count*)

(exit *total-fail-count*)
