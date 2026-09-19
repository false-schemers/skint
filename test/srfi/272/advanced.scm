(import (srfi 272 advanced))
(import (scheme base) (scheme read) (scheme write))
(import (only (skint print) print-width))
(import (only (skint) make-numvector numvector-set!))
(include "../test.scm")

(test-begin "srfi-272-advanced")

(define (printed obj . kv)
  (let ((p (open-output-string)))
    (apply pp obj p kv)
    (get-output-string p)))

;; --------------------------------------------------- a separate implementation

; the first three libraries are (skint print) repackaged and share its
; parameters; this one is the portable printer and shares nothing with it
(test #f (eq? pp-width print-width))

(test "(a b)\n" (printed '(a b)))
(test "(1 2 3 4 5 6\n 7 8)\n" (printed '(1 2 3 4 5 6 7 8) pp-width 12))

;; ---------------------------------------------------------------- parameters

(test "(let ([a 1]) a)\n" (printed '(let ((a 1)) a) pp-code #t pp-brackets #t))
(test "(let ((a 1)) a)\n" (printed '(let ((a 1)) a) pp-code #t pp-brackets #f))
(test "(1 (2 (...)))\n" (printed '(1 (2 (3 (4)))) pp-level 2))
(test "(1 2 3 ...)\n" (printed '(1 2 3 4 5 6) pp-length 3))
(test "#xff\n" (printed 255 pp-radix 16))

; pp-lines stops after the given number of lines and marks the cut
(test "(a (b c)\n (d (e f)) ..)\n\n" (printed '(a (b c) (d (e f)) g) pp-width 10 pp-lines 2))

;; -------------------------------------------------------- sharing and cycles

(define (shared-pair) (let ((y (list 1 2))) (list y y)))
(define (cyclic) (let ((c (list 1 2))) (set-cdr! (cdr c) c) c))

(test "((1 2) (1 2))\n" (printed (shared-pair)))
(test "#0=(1 2 . #0#)\n" (printed (cyclic)))
(test "(#0=(1 2) #0#)\n" (printed (shared-pair) pp-graph #t))
(test "(#0=(1 2) #0#)\n" (let ((p (open-output-string)))
                           (pprint-shared (shared-pair) p)
                           (get-output-string p)))

;; ---------------------------------------------------------- numerical vectors

; this library knows every numerical vector type skint has
(define (filled type n)
  (let ((v (make-numvector type n)))
    (when (< type 10)
      (let loop ((i 0)) (when (< i n) (numvector-set! v i 1) (loop (+ i 1)))))
    v))

(test "#u32(1 1 1 1\n     1 1)\n" (printed (filled 4 6) pp-width 12))
(test "#s32(1 1 1 1\n     1 1)\n" (printed (filled 5 6) pp-width 12))
(test "#u64(1 1 1 1\n     1 1)\n" (printed (filled 6 6) pp-width 12))
(test "#s64(1 1 1 1\n     1 1)\n" (printed (filled 7 6) pp-width 12))
(test "#c64(0.0+0.0i\n     0.0+0.0i\n     0.0+0.0i)\n" (printed (filled 14 3) pp-width 20))
(test "#c128(0.0+0.0i\n      0.0+0.0i\n      0.0+0.0i)\n" (printed (filled 15 3) pp-width 20))

; and the ones it already knew
(test "#s8(1 1 1 1\n    1 1)\n" (printed (filled 1 6) pp-width 12))
(test "#u16(1 1 1 1\n     1 1)\n" (printed (filled 2 6) pp-width 12))

; each of them reads back as what was printed
(define (roundtrips? type)
  (let ((v (filled type 3)))
    (equal? v (read (open-input-string (printed v))))))
(test #t (roundtrips? 4))
(test #t (roundtrips? 5))
(test #t (roundtrips? 6))
(test #t (roundtrips? 7))
(test #t (roundtrips? 14))
(test #t (roundtrips? 15))

(test-end)
