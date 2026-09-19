
; [esl] adapted from the reference implementation by Alex Shinn
;; Copyright (c) 2006-2019 Alex Shinn.  All rights reserved.
;; BSD-style license: http://synthcode.com/license.txt

(define-library (srfi 166 base)
  (import (scheme base)
          (scheme char)
          (scheme complex)
          (scheme inexact)
          (scheme repl)
          (scheme write)
          (srfi 1)
          (srfi 69)
          (srfi 130)
          (rename (srfi 165)
                  (computation-each sequence)
                  (computation-with! with!)
                  (computation-forked forked)
                  (make-computation-environment-variable make-state-variable))
          ;[esl*] moved from chibi namespace; was (chibi show shared)
          (srfi 166 shared))
  (export
   ;; basic
   show displayed written written-shared written-simply
   escaped maybe-escaped
   numeric numeric/comma numeric/si numeric/fitted
   nl fl space-to tab-to nothing each each-in-list
   joined joined/prefix joined/suffix joined/last joined/dot
   joined/range padded padded/right padded/both
   trimmed trimmed/right trimmed/both trimmed/lazy
   fitted fitted/right fitted/both output-default
   ;; computations
   fn with with! forked call-with-output
   ;; state variables
   make-state-variable
   port row col width output writer pad-char ellipsis
   string-width substring/width substring/preserve
   radix precision decimal-sep decimal-align sign-rule
   comma-sep comma-rule word-separator? ambiguous-is-wide?
   pretty-environment)

(begin

(define-syntax let-optionals*
  (syntax-rules ()
    ((let-optionals* opt-ls () . body)
      (let () . body)) ; [esl*] was (begin . body) -- changed to allow internal definitions!
    ((let-optionals* (op . args) vars . body)
      (let ((tmp (op . args)))
        (let-optionals* tmp vars . body)))
    ((let-optionals* tmp ((var default) . rest) . body)
      (let ((var (if (pair? tmp) (car tmp) default))
            (tmp2 (if (pair? tmp) (cdr tmp) '())))
        (let-optionals* tmp2 rest . body)))
    ((let-optionals* tmp tail . body)
      (let ((tail tmp)) . body))))

;;> The minimal base formatting combinators and show interface.

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(define-syntax fn
  (syntax-rules ()
    ((fn . x)
     (computation-fn . x))))

;; The base formatting handles outputting raw strings and a simple,
;; configurable handler for formatting objects.

;; Utility - default value of string-width.
(define (substring-length str . o)
  (let ((start (if (pair? o) (car o) 0))
        (end (if (and (pair? o) (pair? (cdr o))) (cadr o) (string-length str))))
    (- end start)))

(define (call-with-output-string proc)
  (let ((out (open-output-string)))
    (proc out)
    (let ((res (get-output-string out)))
      (close-output-port out)
      res)))

;;> Raw output - displays str to the formatter output port and updates
;;> row and col.
(define (output-default str)
  (fn (port (r row) (c col) string-width)
    (let ((nl-index (string-index-right str #\newline)))
      (write-string str port)
      (if (string-cursor>? nl-index (string-cursor-start str))
          (with! (row (+ r (string-count str (lambda (ch) (eqv? ch #\newline)))))
                 (col (string-width str (string-cursor->index str nl-index))))
          (with! (col (+ c (string-width str))))))))

(define-computation-type make-show-env show-run
  (port (current-output-port))
  (col 0)
  (row 0)
  (width 78)
  (radix 10)
  (pad-char #\space)
  (output output-default)
  (string-width substring-length)
  (substring/width substring)
  (substring/preserve #f)
  (word-separator? char-whitespace?)
  (ambiguous-is-wide? #f)
  (ellipsis "")
  (decimal-align #f)
  (decimal-sep #f)
  (comma-sep #f)
  (comma-rule #f)
  (sign-rule #f)
  (precision #f)
  (writer #f)
  (pretty-environment (interaction-environment)))

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;;> \procedure{(show out [args ...])}
;;>
;;> Run the combinators \var{args}, accumulating the output to
;;> \var{out}, which is either an output port or a boolean, with
;;> \scheme{#t} indicating \scheme{current-output-port} and
;;> \scheme{#f} to collect the output as a string.
(define (show out . args)
  (cond
   ((output-port? out)
    (if (any procedure? args)
        (show-run (sequence (with! (port out)) (each-in-list args)))
        (for-each (lambda (arg) (display arg out)) args)))
   ((eq? #t out)
    (apply show (current-output-port) args))
   ((eq? #f out)
    (call-with-output-string
      (lambda (out)
        (apply show out args))))
   (else
    (error "unknown output to show, expected a port or #t/#f" out))))

;;> Temporarily bind the parameters in the body \var{x}.
(define-syntax with
  (syntax-rules ()
    ((with params x ... y)
     (computation-with params (each x ... y)))))

;;> The noop formatter.  Generates no output and leaves the state
;;> unmodified.
(define nothing (fn () (with!)))

;;> Formats a displayed version of x - if a string or char, outputs the
;;> raw characters (as with `display'), if x is already a formatter
;;> defers to that, otherwise outputs a written version of x.
(define (displayed x)
  (cond
   ((procedure? x) x)
   ((string? x) (fn ((output1 output)) (output1 x)))
   ((char? x) (displayed (string x)))
   (else (written x))))

;;> Formats a written version of x, as with `write'.  The formatting
;;> can be updated with the \scheme{'writer} field.
(define (written x)
  (fn (writer) ((or writer written-default) x)))

;;> Takes a single list of formatters, combined in sequence with
;;> \scheme{each}.
(define (each-in-list args)
  (if (pair? args)
      (if (pair? (cdr args))
          (sequence (displayed (car args)) (each-in-list (cdr args)))
          (fn () (displayed (car args))))
      nothing))

;;> Combines each of the formatters in a sequence using
;;> \scheme{displayed}, so that strings and chars will be output
;;> directly and other objects will be \scheme{written}.
(define (each . args)
  (each-in-list args))

;;> Captures the output of \var{producer} and formats the result with
;;> \var{consumer}.
(define (call-with-output producer consumer)
  (let ((out (open-output-string)))
    (forked (with ((port out) (output output-default)) producer)
            (fn () (consumer (get-output-string out))))))

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;;> \section{String utilities}

(define (write-to-string x)
  (call-with-output-string (lambda (out) (write x out))))

(define (string-replace-all str ch1 ch2)
  (let ((out (open-output-string)))
    (string-for-each
     (lambda (ch) (display (if (eqv? ch ch1) ch2 ch) out))
     str)
    (get-output-string out)))

(define (string-intersperse-right str sep rule)
  (let ((start (string-cursor-start str)))
    (let lp ((i (string-cursor-end str))
             (rule rule)
             (res '()))
      (let* ((offset (if (pair? rule) (car rule) rule))
             (i2 (if offset (string-cursor-back str i offset) start)))
        (if (string-cursor<=? i2 start)
            (apply string-append (cons (substring/cursors str start i) res))
            (lp i2
                (if (and (pair? rule) (not (null? (cdr rule)))) (cdr rule) rule)
                (cons sep (cons (substring/cursors str i2 i) res))))))))

;;> Outputs the string str, escaping any quote or escape characters.
;;> If esc-ch, which defaults to #\\, is #f, escapes only the
;;> quote-ch, which defaults to #\", by doubling it, as in SQL strings
;;> and CSV values.  If renamer is provided, it should be a procedure
;;> of one character which maps that character to its escape value,
;;> e.g. #\newline => #\n, or #f if there is no escape value.

(define (escaped fmt . o)
  (let-optionals* o ((quot #\")
                     (esc #\\)
                     (rename (lambda (x) #f)))
    (let ((esc-str (cond ((char? esc) (string esc))
                         ((not esc) (string quot))
                         (else esc))))
      (fn ((orig-output output))
        (define (output* str)
          (let ((start (string-cursor-start str))
                (end (string-cursor-end str)))
            (let lp ((i start) (j start))
              (define (collect)
                (if (eq? i j) "" (substring/cursors str i j)))
              (if (string-cursor>=? j end)
                  (orig-output (collect))
                  (let ((c (string-ref/cursor str j))
                        (j2 (string-cursor-next str j)))
                    (cond
                     ((or (eqv? c quot) (eqv? c esc))
                      (each (orig-output (collect))
                            (orig-output esc-str)
                            (fn () (lp j j2))))
                     ((rename c)
                      => (lambda (c2)
                           (each (orig-output (collect))
                                 (orig-output esc-str)
                                 (orig-output (if (char? c2) (string c2) c2))
                                 (fn () (lp j2 j2)))))
                     (else
                      (lp i j2))))))))
        (with ((output output*))
          fmt)))))

;;> Only escape if there are special characters, in which case also
;;> wrap in quotes.  For writing symbols in |...| escapes, or CSV
;;> fields, etc.  The predicate indicates which characters cause
;;> slashification - this is in addition to automatic slashifying when
;;> either the quote or escape char is present.

(define (maybe-escaped fmt pred . o)
  (let-optionals* o ((quot #\")
                     (esc #\\)
                     (rename (lambda (x) #f)))
    (define (esc? c) (or (eqv? c quot) (eqv? c esc) (rename c) (pred c)))
    (call-with-output
     fmt
     (lambda (str)
       (if (string-cursor<? (string-index str esc?) (string-cursor-end str))
           (each quot (escaped str quot esc rename) quot)
           (displayed str))))))

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; numeric formatting

(define (char-mirror c)
  (case c ((#\() #\)) ((#\[) #\]) ((#\{) #\}) ((#\<) #\>) (else c)))

(define (integer-log a base)
  (if (zero? a)
      0
      ;; (exact (ceiling (/ (log (+ a 1)) (log base))))
      (do ((ndigits 1 (+ ndigits 1))
           (p base (* p base)))
          ((> p a) ndigits))))

;; The original fmt algorithm was based on "Printing Floating-Point
;; Numbers Quickly and Accurately" by Burger and Dybvig
;; (FP-Printing-PLDI96.pdf).  It had grown unwieldy with formatting
;; special cases, so the below is a simplification which tries to rely
;; on number->string for common cases.

(define unspec (list 'unspecified))

(define-syntax default
  (syntax-rules ()
    ((default var dflt) (if (eq? var unspec) dflt var))))

(define (numeric n . o)
  (let-optionals* o ((rad unspec) (prec unspec) (sgn unspec)
                     (comma unspec) (commasep unspec) (decsep unspec))
    (fn (radix precision sign-rule
               comma-rule comma-sep decimal-sep decimal-align)
      (let* ((radix (default rad radix))
             (precision (default prec precision))
             (sign-rule (default sgn sign-rule))
             (comma-rule (default comma comma-rule))
             (comma-sep (default commasep comma-sep))
             (dec-sep (default decsep
                        (or decimal-sep (if (eqv? comma-sep #\.) #\, #\.))))
             (dec-ls (if (char? dec-sep)
                         (list dec-sep)
                         (reverse (string->list dec-sep)))))
        ;; General formatting utilities.
        (define (get-scale q)
          (expt radix (- (integer-log q radix) 1)))
        (define (char-digit d)
          (cond ((char? d) d)
                ((< d 10) (integer->char (+ d (char->integer #\0))))
                (else (integer->char (+ (- d 10) (char->integer #\a))))))
        (define (digit-value ch)
          (let ((res (- (char->integer ch) (char->integer #\0))))
            (if (<= 0 res 9)
                res
                ch)))
        (define (round-up ls)
          (let lp ((ls ls) (res '()))
            (cond
             ((null? ls)
              (append (reverse res) '(1)))
             ((not (number? (car ls)))
              (lp (cdr ls) (cons (car ls) res)))
             ((= (car ls) (- radix 1))
              (lp (cdr ls) (cons 0 res)))
             (else
              (append (reverse res) (cons (+ 1 (car ls)) (cdr ls)))))))
        ;[esl*] without bignums, we can't use the 'general' algorithm
        (define (maybe-round n d ls)
          (let* ((q (quotient n d))
                 (digit (* 2 (if (>= q radix) (quotient q (get-scale q)) q))))
            (if (or (> digit radix)
                    (and (= digit radix)
                         (let ((prev (find integer? ls)))
                           (and prev (odd? prev)))))
                (round-up ls)
                ls)))
        ;[esl*] without bignums, we can't use the 'general' algorithm
        (define (maybe-trim-zeros i res inexact?)
          (if (and (not precision) (positive? i))
              (let lp ((res res))
                (cond
                 ((and (pair? res) (eqv? 0 (car res))) (lp (cdr res)))
                 ((and (pair? res)
                       (eqv? (car dec-ls) (car res))
                       (null? (cdr dec-ls)))
                  (if inexact?
                      (cons 0 res)      ; "1.0"
                      (cdr res)))       ; "1"
                 (else res)))
              res))
        ;[esl*] without bignums, we can't use the 'general' algorithm
        ;; General slow loop to generate digits one at a time, for
        ;; non-standard radixes or writing rationals with a fixed
        ;; precision.
        (define (gen-general n-orig)
          (let* ((p (exact n-orig))
                 (n (numerator p))
                 (d (denominator p)))
            (let lp ((n n)
                     (i (if (zero? p) -1 (- (integer-log p radix))))
                     (res '()))
              (cond
               ;; Use a fixed precision if specified, otherwise generate
               ;; 15 decimals.
               ((if precision (< i precision) (< i 16))
                (let ((res (if (zero? i)
                               (append dec-ls (if (null? res) (cons 0 res) res))
                               res))
                      (q (quotient n d)))
                  (cond
                   ((< i -1)
                    (let* ((scale (expt radix (- -1 i)))
                           (digit (quotient q scale))
                           (n2 (- n (* d digit scale))))
                      (lp n2 (+ i 1) (cons digit res))))
                   (else
                    (lp (* (remainder n d) radix)
                        (+ i 1)
                        (cons q res))))))
               (else
                (reverse-list->string
                 (map char-digit
                      (maybe-trim-zeros i (maybe-round n d res) (inexact? n-orig)))))))))
        ;; Generate a fixed precision decimal result by post-editing the
        ;; result of string->number.
        (define (gen-fixed n)
          (cond
           ;[esl-] without bignums, we can't rely on 'exact' not failing
           #;((and (eqv? radix 10) (zero? precision) (inexact? n))
            (number->string (exact (round n))))
           ;[esl+] 
           ((and (zero? precision) (exact? n))
            (number->string n))         
           ((and (eqv? radix 10) (or (integer? n) (inexact? n)))
            (let* ((s (number->string n))
                   (start (string-cursor-start s))
                   (end (string-cursor-end s))
                   (dec (string-index s #\.))
                   (e (string-index s #\e))
                   (digits (- (string-cursor->index s end)
                              (string-cursor->index s dec))))
              (cond
               ((string-cursor<? e end)
                (if (positive? precision)
                    (let* ((e2 (string-cursor-next s e))
                           (exp (string->number (substring/cursors s e2 end))))
                      (if (<= (- exp) precision)
                          ; [esl*] can't use the original (gen-general n)
                          s 
                          ;; Experimental, prefer to retain the
                          ;; exponent instead of just rounding to 0.
                          (let ((n2 (string->number
                                     (substring/cursors s start e))))
                            (string-append
                             (gen-fixed n2)
                             (substring/cursors s e end)))))
                    ; [esl*] can't use the original (gen-general n)
                    s #;(gen-general n)))
               ((string-cursor=? dec end)
                (string-append s (if (char? dec-sep) (string dec-sep) dec-sep)
                               (make-string precision #\0)))
               ((<= digits precision)
                (string-append s (make-string (- precision digits -1) #\0)))
               (else
                (let* ((last
                        (string-cursor-back s end (- digits precision 1)))
                       (res (substring/cursors s start last)))
                  (if (and
                       (string-cursor<? last end)
                       (let ((next (digit-value (string-ref/cursor s last))))
                         (or (> next 5)
                             (and (= next 5)
                                  (string-cursor>? last start)
                                  (memv (digit-value
                                         (string-ref/cursor
                                          s (string-cursor-prev s last)))
                                        '(1 3 5 7 9))))))
                      (reverse-list->string
                       (map char-digit
                            (round-up
                             (reverse
                              (map digit-value (string->list res))))))
                      res))))))
           (else
            ;[esl*] without bignums, we can't use the 'general' algorithm
            (cond-expand
              (full-numeric-tower (gen-general n))
              (else (error "invalid radix for numeric formatting" radix n)))
            #;(gen-general n))))
        ;; Generate any unsigned real number.
        (define (gen-positive-real n)
          (cond
           (precision
            (gen-fixed n))
           ((memv radix (if (exact? n) '(2 8 10 16) '(10)))
            (number->string n radix))
           (else
            ;[esl*] without bignums, we can't use the 'general' algorithm 
            (cond-expand
              (full-numeric-tower (gen-general n))
              (else (error "invalid radix for numeric formatting" radix n)))
            #;(gen-general n))))
        ;; Insert commas according to the current comma-rule.
        (define (insert-commas str)
          (let* ((dec-pos (if (string? dec-sep)
                              (or (string-contains str dec-sep)
                                  (string-cursor-end str))
                              (string-index str dec-sep)))
                 (left (substring/cursors str (string-cursor-start str) dec-pos))
                 (right (string-copy/cursors str dec-pos))
                 (sep (cond ((char? comma-sep) (string comma-sep))
                            ((string? comma-sep) comma-sep)
                            ((eqv? #\, dec-sep) ".")
                            (else ","))))
            (string-append
             (string-intersperse-right left sep comma-rule)
             right)))
        ;; Post-process a positive real number with decimal char fixup
        ;; and commas as needed.
        (define (wrap-comma n)
          (if (and (not precision) (exact? n) (not (integer? n)))
              (string-append (wrap-comma (numerator n))
                             "/"
                             (wrap-comma (denominator n)))
              (let* ((s0 (gen-positive-real n))
                     (s1 (if (or (eqv? #\. dec-sep)
                                 (equal? "." dec-sep))
                             s0
                             (string-replace-all s0 #\. dec-sep))))
                (if comma-rule (insert-commas s1) s1))))
        ;; Wrap the sign of a real number, forcing a + prefix or using
        ;; parentheses (n) for negatives according to sign-rule.

        (define-syntax is-neg-zero?
          (syntax-rules ()
            ((_ n)
             (is-neg-zero? (-0.0) n))
            ((_ (0.0) n)                ; -0.0 is not distinguished?
             #f)
            ((_ (-0.0) n)
             (eqv? -0.0 n))))
        (define (negative?* n)
          (or (negative? n)
              (is-neg-zero? n)))
        (define (wrap-sign n sign-rule)
          (cond
           ((negative?* n)
            (cond
             ((char? sign-rule)
              (string-append (string sign-rule)
                             (wrap-comma (- n))
                             (string (char-mirror sign-rule))))
             ((pair? sign-rule)
              (string-append (car sign-rule)
                             (wrap-comma (- n))
                             (cdr sign-rule)))
             (else
              (string-append "-" (wrap-comma (- n))))))
           ((eq? #t sign-rule)
            (string-append "+" (wrap-comma n)))
           (else
            (wrap-comma n))))
        ;; Format a single real number with padding as necessary.
        (define (format n sign-rule)
          (cond
           ((finite? n)
            (let* ((s (wrap-sign n sign-rule))
                   (dec-pos (if decimal-align
                                (string-cursor->index
                                 s
                                 (if (char? dec-sep)
                                     (string-index s dec-sep)
                                     (or (string-contains s dec-sep)
                                         (string-cursor-end s))))
                                0))
                   (diff (- (or decimal-align 0) dec-pos 1)))
              (if (positive? diff)
                  (string-append (make-string diff #\space) s)
                  s)))
           (else
            (number->string n))))
        ;; Write any number.
        (define (write-complex n)
          (cond
           ((and radix (not (and (integer? radix) (<= 2 radix 36))))
            (error "invalid radix for numeric formatting" radix n))
           ((zero? (imag-part n))
            (displayed (format (real-part n) sign-rule)))
           (else
            (each (format (real-part n) sign-rule)
                  (format (imag-part n) #t)
                  "i"))))
        (write-complex n)))))

(define numeric/si
  (let* ((names10 '#("" "k" "M" "G" "T" "E" "P" "Z" "Y"))
         (names-10 '#("" "m" "µ" "n" "p" "f" "a" "z" "y"))
         (names2 (list->vector
                  (cons ""
                        (cons "Ki" (map (lambda (s) (string-append s "i"))
                                        (cddr (vector->list names10)))))))
         (names-2 (list->vector
                   (cons ""
                         (map (lambda (s) (string-append s "i"))
                              (cdr (vector->list names-10)))))))
    (define (round-to n k)
      (/ (round (* n k)) k))
    (lambda (n . o)
      (let-optionals* o ((base 1000)
                         (separator ""))
        (if (zero? n)
            "0"
            (let* ((log-n (log (abs n)))
                   (names  (if (negative? log-n)
                               (if (= base 1024) names-2 names-10)
                               (if (= base 1024) names2 names10)))
                   (k (min (exact ((if (negative? log-n) ceiling floor)
                                   (/ (abs log-n) (log base))))
                           (- (vector-length names) 1)))
                   (n2 (round-to (/ (abs n)
                                    (expt base (if (negative? log-n) (- k) k)))
                                 10)))
              (each (if (negative? n) "-" "")
                    (if (integer? n2)
                        (number->string (exact n2))
                        (inexact n2))
                    ;; (if (zero? k) "" separator)
                    separator
                    (vector-ref names k))))))))

;; Force a number into a fixed width, print as #'s if doesn't fit.
;; Needs to be wrapped in PADDED if you want to expand to the width.

(define (numeric/fitted width n . args)
  (call-with-output
   (apply numeric n args)
   (lambda (str)
     (if (> (string-length str) width)
         (fn (precision decimal-sep comma-sep)
           (let ((prec (if (and (pair? args) (pair? (cdr args)))
                           (cadr args)
                           precision)))
             (if (and prec (not (zero? prec)))
                 (let* ((dec-sep
                         (or decimal-sep
                             (if (eqv? #\. comma-sep) #\, #\.)))
                        (diff (- width (+ prec
                                          (if (char? dec-sep)
                                              1
                                              (string-length dec-sep))))))
                   (each (if (positive? diff) (make-string diff #\#) "")
                         dec-sep (make-string prec #\#)))
                 (displayed (make-string width #\#)))))
         (displayed str)))))

(define (numeric/comma n . o)
  (fn ((orig-comma-rule comma-rule))
    (with ((comma-rule (if (pair? o) (car o) (or orig-comma-rule 3))))
      (apply numeric n (if (pair? o) (cdr o) '())))))

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; written

(define (write-with-shares obj shares)
  (fn ((orig-radix radix) precision)
    (let ((write-number
           ;; Shortcut for numeric values.  Try to rely on
           ;; number->string for standard radixes and no precision,
           ;; otherwise fall back on numeric but resetting to a usable
           ;; radix.
           (cond
            ((and (not precision)
                  (assv orig-radix
                        '((16 . "#x") (10 . "") (8 . "#o") (2 . "#b"))))
             => (lambda (cell)
                  (lambda (n)
                    (cond
                     ((eqv? orig-radix 10)
                      (displayed (number->string n (car cell))))
                     ((exact? n)
                      (each (cdr cell) (number->string n (car cell))))
                     (else
                      (with ((radix 10)) (numeric n)))))))
            (else (lambda (n) (with ((radix 10)) (numeric n)))))))
      ;; `wr' is the recursive writer closing over the shares.
      (let wr ((obj obj))
        (call-with-shared-ref
         obj shares each
         (fn ()
           (cond
            ((pair? obj)
             (each "("
                   (fn ()
                     (let lp ((ls obj))
                       (let ((rest (cdr ls)))
                         (each (wr (car ls))
                               (cond
                                ((null? rest)
                                 nothing)
                                ((pair? rest)
                                 (each
                                  " "
                                  (call-with-shared-ref/cdr
                                   rest shares each
                                   (fn () (lp rest)))))
                                (else
                                 (each " . " (wr rest))))))))
                   ")"))
            ((vector? obj)
             (let ((len (vector-length obj)))
               (if (zero? len)
                   (displayed "#()")
                   (each "#("
                         (wr (vector-ref obj 0))
                         (fn ()
                           (let lp ((i 1))
                             (if (>= i len)
                                 nothing
                                 (each " " (wr (vector-ref obj i))
                                       (fn () (lp (+ i 1)))))))
                         ")"))))
            ((number? obj)
             (write-number obj))
            (else
             (displayed (write-to-string obj))))))))))

;; The default formatter for `written', overriden with the `writer'
;; variable.  Intended to be equivalent to `write', using datum labels
;; for shared notation iff there are cycles in the object.

(define (written-default obj)
  (fn ()
    (write-with-shares obj (extract-shared-objects obj #t))))

;; Writes the object showing the full shared structure.

(define (written-shared obj)
  (fn ()
    (write-with-shares obj (extract-shared-objects obj #f))))

;; The only expensive part, in both time and memory, of handling
;; shared structures when writing is building the initial table, so
;; for the efficient version we just skip that and re-use the writing
;; code.

(define (written-simply obj)
  (fn ()
    (write-with-shares obj (extract-shared-objects #f #f))))

;;> A library of procedures for formatting Scheme objects to text in
;;> various ways, and for easily concatenating, composing and
;;> extending these formatters.

;;> \section{Background}
;;>
;;> There are several approaches to text formatting.  Building strings
;;> to \scheme{display} is not acceptable, since it doesn't scale to
;;> very large output.  The simplest realistic idea, and what people
;;> resort to in typical portable Scheme, is to interleave
;;> \scheme{display} and \scheme{write} and manual loops, but this is
;;> both extremely verbose and doesn't compose well.  A simple concept
;;> such as padding space can't be achieved directly without somehow
;;> capturing intermediate output.
;;>
;;> The traditional approach is to use templates - typically strings,
;;> though in theory any object could be used and indeed Emacs'
;;> mode-line format templates allow arbitrary sexps.  Templates can
;;> use either escape sequences (as in C's \cfun{printf} and
;;> \hyperlink["http://en.wikipedia.org/wiki/Format_(Common_Lisp)"]{CL's}
;;> \scheme{format}) or pattern matching (as in Visual Basic's
;;> \cfun{Format},
;;> \hyperlink["http://search.cpan.org/~dconway/Perl6-Form-0.04/Form.pm"}{Perl6's}
;;> \cfun{form}, and SQL date formats).  The primary disadvantage of
;;> templates is the relative difficulty (usually impossibility) of
;;> extending them, their opaqueness, and the unreadability that
;;> arises with complex formats.  Templates are not without their
;;> advantages, but they are already addressed by other libraries such
;;> as
;;> \hyperlink["http://srfi.schemers.org/srfi-28/srfi-28.html"]{SRFI-28}
;;> and
;;> \hyperlink["http://srfi.schemers.org/srfi-48/srfi-48.html"]{SRFI-48}.
;;>
;;> This library takes a combinator approach.  Formats are nested chains
;;> of closures, which are called to produce their output as needed.
;;> The primary goal of this library is to have, first and foremost, a
;;> maximally expressive and extensible formatting library.  The next
;;> most important goal is scalability - to be able to handle
;;> arbitrarily large output and not build intermediate results except
;;> where necessary.  The third goal is brevity and ease of use.

;;> \section{Interface}

;;> \procedure{(show out [args ...])}
;;>
;;> The primary interface.  Analogous to CL's \scheme{format}, the first
;;> argument is either an output port or a boolean, with \scheme{#t}
;;> indicating \scheme{current-output-port} and \scheme{#f} indicating a
;;> string port.  The remaining arguments are formatters, combined as with
;;> \scheme{each}, run with output to the given destination.  If \var{out}
;;> is \scheme{#f} then the accumulated output is returned, otherwise
;;> the result is unspecified.

;;> \section{Formatters}

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Spacing

;;> Output a single newline.
(define nl (displayed "\n"))

;;> "Fresh line" - output a newline iff we're not at the start of a
;;> fresh line.
(define fl
  (fn (col) (if (zero? col) nothing nl)))

;;> Move to a given tab-stop (using spaces, not tabs).
(define (tab-to . o)
  (fn (col pad-char)
    (let* ((tab-width (if (pair? o) (car o) 8))
           (rem (modulo col tab-width)))
      (if (positive? rem)
          (displayed (make-string (- tab-width rem) pad-char))
          nothing))))

;;> Move to an explicit column.
(define (space-to where)
  (fn (col pad-char)
    (displayed (make-string (max 0 (- where col)) pad-char))))

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Padding and trimming

;;> Pad the result of \scheme{(each-in-list ls)} to at least
;;> \var{width} characters, equally applied to the left and right,
;;> with any extra odd padding applied to the right.  Uses the value
;;> of \scheme{pad-char} for padding, defaulting to \scheme{#\\space}.
(define (padded/both width . ls)
  (call-with-output
   (each-in-list ls)
   (lambda (str)
     (fn (string-width pad-char)
       (let ((diff (- width (string-width str))))
         (if (positive? diff)
             (let* ((diff/2 (quotient diff 2))
                    (left (make-string diff/2 pad-char))
                    (right (if (even? diff)
                               left
                               (make-string (+ 1 diff/2) pad-char))))
               (each left str right))
             (displayed str)))))))

;;> As \scheme{padded/both} but only applies padding on the right.
(define (padded/right width . ls)
  (fn ((col1 col))
    (each (each-in-list ls)
          (fn ((col2 col) pad-char)
            (displayed (make-string (max 0 (- width (- col2 col1)))
                                    pad-char))))))

;;> As \scheme{padded/both} but only applies padding on the left.
(define (padded/left width . ls)
  (call-with-output
   (each-in-list ls)
   (lambda (str)
     (fn (string-width pad-char)
       (let ((diff (- width (string-width str))))
         (each (make-string (max 0 diff) pad-char) str))))))

;;> An alias for \scheme{padded/left}.
(define padded padded/left)

;; General buffered trim - capture the output apply a trimmer.
(define (trimmed/buffered width producer proc)
  (call-with-output
   producer
   (lambda (str)
     (fn (string-width)
       (let* ((str-width (string-width str))
              (diff (- str-width width)))
         (displayed (if (positive? diff)
                        (proc str str-width diff)
                        str)))))))

;;> Trims the result of \scheme{(each-in-list ls)} to at most
;;> \var{width} characters, removed from the right.  If any characters
;;> are removed, then the value of \scheme{ellipsis} (default empty)
;;> is used in its place (trimming additional characters as needed to
;;> be sure the final output doesn't exceed \var{width}).
(define (trimmed/right width . ls)
  (trimmed/buffered
   width
   (each-in-list ls)
   (lambda (str str-width diff)
     (fn (ellipsis string-width substring/width substring/preserve)
       (let* ((ell (if (char? ellipsis) (string ellipsis) (or ellipsis "")))
              (ell-len (string-width ell))
              (diff (- (+ str-width ell-len) width))
              (end (- width ell-len)))
         (each (if substring/preserve
                   (substring/preserve (substring/width str -1 0))
                   nothing)
               (if (negative? diff)
                   nothing
                   (substring/width str 0 end))
               ell
               (if (and substring/preserve (< end str-width))
                   (substring/preserve (substring/width str end str-width))
                   nothing)))))))

;;> As \scheme{trimmed/right} but removes from the left.
(define (trimmed/left width . ls)
  (trimmed/buffered
   width
   (each-in-list ls)
   (lambda (str str-width diff)
     (fn (ellipsis string-width substring/width substring/preserve)
       (let* ((ell (if (char? ellipsis) (string ellipsis) (or ellipsis "")))
              (ell-len (string-width ell))
              (diff (- (+ str-width ell-len) width)))
         (each (if (and substring/preserve (positive? diff))
                   (substring/preserve (substring/width str 0 diff))
                   nothing)
               ell
               (if (negative? diff)
                   nothing
                   (substring/width str diff str-width))))))))

;;> An alias for \scheme{trimmed/left}.
(define trimmed trimmed/left)

;;> As \scheme{trimmed} but removes equally from both the left and the
;;> right, removing extra odd characters from the right, and inserting
;;> \scheme{ellipsis} on both sides.
(define (trimmed/both width . ls)
  (trimmed/buffered
   width
   (each-in-list ls)
   (lambda (str str-width diff)
     (fn (ellipsis string-width substring/width substring/preserve)
       (let* ((ell (if (char? ellipsis) (string ellipsis) (or ellipsis "")))
              (ell-len (string-width ell))
              (diff (- (+ str-width ell-len ell-len) width))
              (left (quotient diff 2))
              (right (- str-width (quotient (+ diff 1) 2))))
         (each
          (if substring/preserve
              (substring/preserve (substring/width str 0 left))
              nothing)
          (if (negative? diff)
              ell
              (each ell (substring/width str left right) ell))
          (if substring/preserve
              (substring/preserve (substring/width str right str-width))
              nothing)))))))

;;> A \scheme{trimmed}, but truncates and terminates immediately if
;;> more than \var{width} characters are generated by \var{ls}.  Thus
;;> \var{ls} may lazily generate an infinite amount of output safely
;;> (e.g. \scheme{write-simple} on an infinite list).  The nature of
;;> this procedure means only truncating on the right is meaningful.
(define (trimmed/lazy width . ls)
  (fn ((orig-output output) string-width substring/width)
    (call-with-current-continuation
     (lambda (return)
       (let ((chars-written 0)
             (orig-output (or orig-output output-default)))
         (define (output* str)
           (let ((len (string-width str)))
             (set! chars-written (+ chars-written len))
             (if (> chars-written width)
                 (let* ((end (max 0 (- len (- chars-written width))))
                        (s (substring/width str 0 end)))
                   (each (orig-output s)
                         (with! (output orig-output))
                         (fn () (return nothing))))
                 (orig-output str))))
         (with ((output output*))
           (each-in-list ls)))))))

;;> Fits the result of \scheme{(each-in-list ls)} to exactly
;;> \var{width} characters, padding or trimming on the right as
;;> needed.
(define (fitted/right width . ls)
  (padded/right width (trimmed/right width (each-in-list ls))))

;;> As \scheme{fitted} but pads/trims from the left.
(define (fitted/left width . ls)
  (padded/left width (trimmed/left width (each-in-list ls))))

;;> An alias for \scheme{fitted/left}.
(define fitted fitted/left)

;;> As \scheme{fitted} but pads/trims equally from both the left and
;;> the right.
(define (fitted/both width . ls)
  (padded/both width (trimmed/both width (each-in-list ls))))

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Joining and interspersing

(define (joined/general elt-f last-f dot-f init-ls sep)
  (fn ()
    (let lp ((ls init-ls))
      (cond
       ((pair? ls)
        (each (if (eq? ls init-ls) nothing sep)
              ((if (and last-f (null? (cdr ls))) last-f elt-f) (car ls))
              (lp (cdr ls))))
       ((and dot-f (not (null? ls)))
        (each (if (eq? ls init-ls) nothing sep) (dot-f ls)))
       (else
        nothing)))))

;;> \procedure{(joined elt-f ls [sep])}
;;>
;;> Joins the result of applying \var{elt-f} to each element of the
;;> list \var{ls} together with \var{sep}, which defaults to the empty
;;> string.
(define (joined elt-f ls . o)
  (joined/general elt-f #f #f ls (if (pair? o) (car o) "")))

;;> As \scheme{joined} but treats the separator as a prefix, inserting
;;> before every element instead of between.
(define (joined/prefix elt-f ls . o)
  (if (null? ls)
      nothing
      (let ((sep (if (pair? o) (car o) "")))
        (each sep (joined elt-f ls sep)))))

;;> As \scheme{joined} but treats the separator as a suffix, inserting
;;> after every element instead of between.
(define (joined/suffix elt-f ls . o)
  (if (null? ls)
      nothing
      (let ((sep (if (pair? o) (car o) "")))
        (each (joined elt-f ls sep) sep))))

;;> As \scheme{joined} but applies \var{last-f}, instead of
;;> \var{elt-f}, to the last element of \var{ls}, useful for
;;> e.g. commas separating a list with "and" before the final element.
(define (joined/last elt-f last-f ls . o)
  (joined/general elt-f last-f #f ls (if (pair? o) (car o) "")))

;;> As \scheme{joined} but if \var{ls} is a dotted list applies
;;> \var{dot-f} to the dotted tail as a final element.
(define (joined/dot elt-f dot-f ls . o)
  (joined/general elt-f #f dot-f ls (if (pair? o) (car o) "")))

;;> As \scheme{joined} but counts from \var{start} to \var{end}
;;> (exclusive), formatting each integer in the range.  If \var{end}
;;> is \scheme{#f} or unspecified, produces an infinite stream of
;;> output.
(define (joined/range elt-f start . o)
  (let ((end (and (pair? o) (car o)))
        (sep (if (and (pair? o) (pair? (cdr o))) (cadr o) "")))
    (let lp ((i start))
      (if (and end (>= i end))
          nothing
          (each (if (> i start) sep nothing)
                (elt-f i)
                (fn () (lp (+ i 1))))))))

))
