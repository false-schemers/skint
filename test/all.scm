(import (skint))
(import (only (skint hidden) file-resolve-relative-to-base-path file-resolve-relative-to-current %system %command-line))

(define *tests* '(
skint
srfi
))

(define *results* '())

(define (cmdline-base)
  (let loop ([l (%command-line)] [b ""])
    (if (or (null? l) (null? (cdr l)))
        b 
        (loop (cdr l) (string-append b (car l) " ")))))

(define (run-test n)
  ; assumes skint is in the current directory 
  (define dname (format "~a" n))
  (define fname (file-resolve-relative-to-base-path "all.scm" dname))
  (define cmd (format "~a ~a" (cmdline-base) (file-resolve-relative-to-current fname)))
  (let ([res (%system cmd)])
    (set! *results* (cons (cons n res) *results*))))

(for-each run-test *tests*)

(format #t "~%~%----------------------------------------------------------------------------~%")
(format #t "CUMULATIVE RESULTS FOR ALL LIBRARY ROOTS:~%")
(format #t "----------------------------------------------------------------------------~%~%")

(define *total-fail-count* 0)

(define (show-result n&r)
  (if (zero? (cdr n&r))
      (format #t "~a: \tok~%" (car n&r))
      (format #t "~a: \t~a failed~%" (car n&r) (cdr n&r)))
  (set! *total-fail-count* (+ *total-fail-count* (cdr n&r))) 
)
 
(for-each show-result (reverse *results*))

(format #t "~%~%LIBRARY ROOTS TESTED: ~a~%" (length *tests*))
(format #t "CUMULATIVE FAILURES: ~a~%" *total-fail-count*)

(exit *total-fail-count*)
