(define-library (srfi 48)
  (export format)
  (import (scheme base) (scheme write)
          (only (skint)
            flonum->string string-position 
            format format-pretty-print format-fixed-print 
            format-fresh-line format-help-string))

(begin

; left padding only for SRFI-48
(define (display-padded str width char p)
  (if (< width 0) ; no width specified
      (display str p)
      (let ([off (- width (string-length str))])
        (if (positive? off) (display (make-string off char) p))
        (display str p))))

(define (format-fixed nos width digits p)
  (cond
    [(and (< width 0) (< digits 0))
     (display nos p)]
    [(string? nos)
     (if (< width 0) 
         (display nos p)
         (display-padded nos width #\space p))]
    [(number? nos)
     (let ((str (if (>= digits 0) 
                    (flonum->string (inexact nos) digits)
                    (number->string nos))))
       (display-padded str width #\space p))]
    [else ; extension: format as if formatted via ~a
     (if (< width 0)
         (display nos p)
         (let* ((sp (open-output-string)) (str (begin (display nos sp) (get-output-string sp))))
           (display-padded nos width #\space p)))]))

;; install format-fixed as ~f formatter
(format-fixed-print format-fixed)

))
