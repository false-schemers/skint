(import (srfi 272 fancy))
(import (scheme base) (scheme file) (scheme read) (scheme write))
(import (only (skint) make-numvector numvector-set!))
(include "../test.scm")

(test-begin "srfi-272-fancy")

(define (printed obj . kv)
  (let ((p (open-output-string)))
    (apply pp obj p kv)
    (get-output-string p)))

(define (contents fn)
  (call-with-input-file fn
    (lambda (p)
      (let loop ((acc '()))
        (let ((c (read-char p)))
          (if (eof-object? c)
              (list->string (reverse acc))
              (loop (cons c acc))))))))

(define (contains? s sub)
  (let ((n (string-length s)) (m (string-length sub)))
    (let loop ((i 0))
      (cond ((> (+ i m) n) #f)
            ((string=? (substring s i (+ i m)) sub) #t)
            (else (loop (+ i 1)))))))

;; ------------------------------------------ everything advanced does, and more

(test "(a b)\n" (printed '(a b)))
(test "(let ([a 1]) a)\n" (printed '(let ((a 1)) a) pp-code #t pp-brackets #t))
(test "(1 2 3 ...)\n" (printed '(1 2 3 4 5 6) pp-length 3))

; the extended numerical vector hooks are here too
(define (filled type n)
  (let ((v (make-numvector type n)))
    (when (< type 10)
      (let loop ((i 0)) (when (< i n) (numvector-set! v i 1) (loop (+ i 1)))))
    v))
(test "#u64(1 1 1 1\n     1 1)\n" (printed (filled 6 6) pp-width 12))
(test "#c64(0.0+0.0i\n     0.0+0.0i\n     0.0+0.0i)\n" (printed (filled 14 3) pp-width 20))
(test "#c128(0.0+0.0i\n      0.0+0.0i\n      0.0+0.0i)\n" (printed (filled 15 3) pp-width 20))

;; --------------------------------------------- pprint-file preserves comments

(define in-name "272-fancy-in.tmp")
(define out-name "272-fancy-out.tmp")
(define html-name "272-fancy-out.html")

(for-each (lambda (f) (when (file-exists? f) (delete-file f)))
          (list in-name out-name html-name))

(call-with-output-file in-name
  (lambda (p)
    (display ";; -*- pp-width: 30 -*-" p) (newline p)
    (display "; a leading comment" p) (newline p)
    (display "(define (f x)" p) (newline p)
    (display "  (if (> x 0) (* x 2) (- x)))" p) (newline p)))

(pprint-file in-name out-name)
(define out-text (contents out-name))

; unlike the intermediate library, this one keeps the comments
(test #t (contains? out-text "; a leading comment"))
(test #t (contains? out-text ";; -*- pp-width: 30 -*-"))
(test #t (contains? out-text "(define (f x)"))

; and the code still reads back
(test #t (let ((p (open-input-string out-text)))
           (let loop ((last #f))
             (let ((x (read p)))
               (if (eof-object? x)
                   (equal? last '(define (f x) (if (> x 0) (* x 2) (- x))))
                   (loop x))))))

;; ----------------------------------------------------------- html output

(pprint-file/html in-name html-name)
(define html-text (contents html-name))

(test #t (contains? html-text "<!DOCTYPE html>"))
(test #t (contains? html-text "<pre"))
(test #t (contains? html-text "define"))

(for-each delete-file (list in-name out-name html-name))

(test-end)
