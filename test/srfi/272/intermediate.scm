(import (srfi 272 intermediate))
(import (scheme base) (scheme file) (scheme read) (scheme write))
(import (only (skint print) print-radix print-length print-level print-brackets))
(import (only (skint) make-numvector numvector-set!))
(include "../test.scm")

(test-begin "srfi-272-intermediate")

(define (printed obj . kv)
  (let ((p (open-output-string)))
    (apply pp obj p kv)
    (get-output-string p)))

;; ------------------------------------------------------- keyword arguments

(test #t (eq? pp-radix print-radix))
(test #t (eq? pp-length print-length))
(test #t (eq? pp-level print-level))

(test "#xff\n" (printed 255 pp-radix 16))
(test "#b1010\n" (printed 10 pp-radix 2))
(test "255\n" (printed 255))

(test "(1 2 3 ...)\n" (printed '(1 2 3 4 5 6) pp-length 3))
(test "(1 (2 (...)))\n" (printed '(1 (2 (3 (4)))) pp-level 2))

; the first occurrence of a key wins
(test "#xff\n" (printed 255 pp-radix 16 pp-radix 8))

; a parameter set globally is seen when no keyword overrides it
(test "#o377\n" (parameterize ((pp-radix 8)) (printed 255)))
(test "#xff\n" (parameterize ((pp-radix 8)) (printed 255 pp-radix 16)))

; pp* splices a trailing list of keys and values
(test "#xff\n" (let ((p (open-output-string)))
                 (pp* 255 p (list pp-radix 16))
                 (get-output-string p)))
(test "#xff\n" (let ((p (open-output-string)))
                 (pp* 255 p pp-radix 16 '())
                 (get-output-string p)))

; the keys are skint's own parameter objects, so any of them may be passed,
; including ones this SRFI does not name
(test "(let ((a 1)) a)\n" (printed '(let ((a 1)) a) print-brackets #f))
(test "(let ([a 1]) a)\n" (printed '(let ((a 1)) a) print-brackets #t))

;; ---------------------------------------------------------- numerical vectors

; (skint print) lays out s8, u16, s16, f32 and f64 itself; this library adds
; the rest, so that they break over lines rather than being written flat
(define (filled type n)
  (let ((v (make-numvector type n)))
    (when (< type 10) ; only the integer types take an exact value
      (let loop ((i 0)) (when (< i n) (numvector-set! v i 1) (loop (+ i 1)))))
    v))

(define (narrow v) (printed v pp-width 12))

(test "#u32(1 1 1 1\n     1 1)\n" (narrow (filled 4 6)))
(test "#s32(1 1 1 1\n     1 1)\n" (narrow (filled 5 6)))
(test "#u64(1 1 1 1\n     1 1)\n" (narrow (filled 6 6)))
(test "#s64(1 1 1 1\n     1 1)\n" (narrow (filled 7 6)))

; the ones skint already knew still work
(test "#s8(1 1 1 1\n    1 1)\n" (narrow (filled 1 6)))
(test "#u16(1 1 1 1\n     1 1)\n" (narrow (filled 2 6)))

; and every type at least round-trips through read
(define (roundtrips? type)
  (let ((v (filled type 3)))
    (equal? v (read (open-input-string (printed v))))))
(test #t (roundtrips? 4))
(test #t (roundtrips? 7))
(test #t (roundtrips? 11))
(test #t (roundtrips? 14))
(test #t (roundtrips? 15))

; the complex ones are the widest, and they break like the rest
(test "#c64(0.0+0.0i\n     0.0+0.0i\n     0.0+0.0i)\n"
  (printed (filled 14 3) pp-width 20))
(test "#c128(0.0+0.0i\n      0.0+0.0i\n      0.0+0.0i)\n"
  (printed (filled 15 3) pp-width 20))

;; ------------------------------------------------------------- pretty-style

(test #t (pair? (pretty-style 'let)))
(test #t (begin (pretty-style 'my-special-form (pretty-style 'let))
                (pair? (pretty-style 'my-special-form))))

;; -------------------------------------------------------------- pprint-file

(define in-name "272-in.tmp")
(define out-name "272-out.tmp")

(when (file-exists? in-name) (delete-file in-name))
(when (file-exists? out-name) (delete-file out-name))

(call-with-output-file in-name
  (lambda (p)
    (display "; a leading comment" p) (newline p)
    (display "(define (f x) (if (> x 0) (* x 2) (- x)))" p) (newline p)
    (display "(display 1)" p) (newline p)))

(pprint-file in-name out-name)

(define out-text
  (call-with-input-file out-name
    (lambda (p)
      (let loop ((acc '()))
        (let ((c (read-char p)))
          (if (eof-object? c)
              (list->string (reverse acc))
              (loop (cons c acc))))))))

; the top-level comment survives, and both forms come back
(test #t (and (string? out-text) (> (string-length out-text) 0)))
(test #t (let ((p (open-input-string out-text)))
           (and (equal? '(define (f x) (if (> x 0) (* x 2) (- x))) (read p))
                (equal? '(display 1) (read p))
                (eof-object? (read p)))))

(delete-file in-name)
(delete-file out-name)

(test-end)
