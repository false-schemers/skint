(import (srfi 272))
(import (scheme base) (scheme read) (scheme write))
(import (only (skint hidden) file-resolve-relative-to-current %system %command-line))
(include "test.scm")

(test-begin "srfi-272")

;; ------------------------------------------------- the bare-bones pp export

(define (printed obj)
  (let ((p (open-output-string)))
    (pp obj p)
    (get-output-string p)))

(define (lines s)
  (let loop ((i 0) (n 0))
    (cond ((= i (string-length s)) n)
          ((char=? (string-ref s i) #\newline) (loop (+ i 1) (+ n 1)))
          (else (loop (+ i 1) n)))))

; the output is laid out and always ends with a newline
(test "(a b)\n" (printed '(a b)))
(test "()\n" (printed '()))
(test "#(1 2 3)\n" (printed (vector 1 2 3)))
(test "17\n" (printed 17))

; quote and its relatives are abbreviated
(test "'(a b)\n" (printed ''(a b)))
(test "'(a . b)\n" (printed '(quote (a . b))))
(test "`(,a ,@b)\n" (printed '`(,a ,@b)))

; a form too wide for the page is broken over several lines
(test #t (> (lines (printed '(define (fact n)
                               (if (= n 0) 1 (* n (fact (- n 1)))))))
            0))
(test #t (> (lines (printed
                     '(let ((alpha 1) (beta 2) (gamma 3) (delta 4) (epsilon 5)
                            (zeta 6) (eta 7) (theta 8) (iota 9) (kappa 10))
                        (list alpha beta gamma delta epsilon zeta))))
            1))

; whatever write can round-trip, pp can round-trip
(define (roundtrip obj)
  (read (open-input-string (printed obj))))
(test '(1 "two" #\3 (4 . 5) #(6) #u8(7 8)) (roundtrip '(1 "two" #\3 (4 . 5) #(6) #u8(7 8))))
(test '(a (b (c (d)))) (roundtrip '(a (b (c (d))))))

; the port argument is optional
(test #t (string? (let ((p (open-output-string)))
                    (parameterize ((current-output-port p))
                      (pp '(x y))
                      (get-output-string p)))))

;; --------------------------------------------- the rest of the SRFI, by level

(define (cmdline-base)
  (let loop ((l (%command-line)) (b ""))
    (if (or (null? l) (null? (cdr l)))
        b
        (loop (cdr l) (string-append b (car l) " ")))))

(define (run-sub name)
  (define fname (string-append "272/" name ".scm"))
  (define cmd (string-append (cmdline-base) (file-resolve-relative-to-current fname)))
  (format #t "~%----------------------------------------------------------------~%")
  (format #t "(srfi 272 ~a): ~a~%" name cmd)
  (format #t "----------------------------------------------------------------~%")
  (%system cmd))

; each sub-test exits with its own failure count, so zero is the test
(test 0 (run-sub "basic"))
(test 0 (run-sub "intermediate"))
(test 0 (run-sub "advanced"))
(test 0 (run-sub "fancy"))
(test 0 (run-sub "show"))

(test-end)
