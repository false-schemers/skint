(import (scheme base) (scheme read) (scheme write))
(import (skint print))
(import (only (skint) make-numvector numvector-set!))

(include "test.scm")

;; ---------------------------------------------------------------------------
;; Helpers
;; ---------------------------------------------------------------------------

;; Inputs are read from strings so that datum labels (#0= ... #0#) can be
;; written literally; each helper returns what the printer produced.

(define (datum str) (read (open-input-string str)))

;; multi-line, code-oriented printing
(define (pp width str . kv)
  (let ([p (open-output-string)])
    (apply pretty-print (datum str) p print-width width kv)
    (get-output-string p)))

;; single-line printing
(define (pr width str . kv)
  (let ([p (open-output-string)])
    (apply print (datum str) p print-width width kv)
    (get-output-string p)))

;; pretty-print under print-level / print-length
(define (cut level length str)
  (parameterize ([print-level level] [print-length length])
    (pp 80 str)))

;; the lines of a printed result, without the trailing newline
(define (lines str)
  (let loop ([i 0] [start 0] [acc '()])
    (cond [(= i (string-length str))
           (reverse (if (> i start) (cons (substring str start i) acc) acc))]
          [(char=? (string-ref str i) #\newline)
           (loop (+ i 1) (+ i 1) (cons (substring str start i) acc))]
          [else (loop (+ i 1) start acc)])))

;; does printing OBJ and reading it back give the same datum?
(define (round-trips? obj . kv)
  (let ([p (open-output-string)])
    (apply pretty-print obj p kv)
    (equal? obj (read (open-input-string (get-output-string p))))))

;; print applied to an object rather than to a string to be read
(define (printed obj . kv)
  (let ([p (open-output-string)])
    (apply print obj p kv)
    (get-output-string p)))

;; what one of the standard output procedures produces, for comparison
(define (via proc obj)
  (let ([p (open-output-string)])
    (proc obj p)
    (get-output-string p)))

;; A spread of data with no cycle in it, so that every marking mode can be
;; applied to all of it.  Quote forms are included: as data they are lists,
;; and print writes them as lists, the way write does.
(define acyclic-data
  (list 1 -2 3.5 #\a #\newline "a string" 'sym (string->symbol "odd sym")
        (string->symbol "") '() '(1 2 3) '(1 . 2) #(1 2 #(3)) #u8(1 2 3) #t #f
        (list "" #\space 1.0 -0.0)
        (list 'quote 'x) (datum "`(,a ,@b)") (datum "(f '(a b) `c)")
        (datum "(#0=(1 2) #0#)")                    ; shared, not cyclic
        (datum "#((#0=(a) #1=(b) #0#) #1#)")))

(define cyclic-data
  (list (datum "#0=(1 2 . #0#)")
        (datum "#0=(a . #0#)")
        (datum "#0=#(#0#)")
        (datum "(#0=(a) #1=(b . #1#) #0#)")
        (datum "#0='#0#")))


(display "\n--- print follows write ---\n")

;; One line, however long, and no newline of its own.
(test "1" (pr 40 "1"))
(test "(a b c)" (pr 40 "(a b c)"))
(test "\"a string\"" (pr 40 "\"a string\""))
(test "(aaaa bbbb cccc dddd eeee ffff)" (pr 20 "(aaaa bbbb cccc dddd eeee ffff)"))
(test "(let ((x 1) (y 2)) (display x))" (pr 20 "(let ((x 1) (y 2)) (display x))"))

;; and character for character what write produces, sharing and cycles included
(for-each (lambda (obj) (test (via write obj) (printed obj)))
          (append acyclic-data cyclic-data))

;; The reader's abbreviations belong to code: print spells a quote form out, as
;; write does, laid out over lines or not; only pretty-print abbreviates.
(test "(quote x)" (printed (list 'quote 'x)))
(test "(quote (a b))" (printed (datum "'(a b)")))
(test "(quasiquote ((unquote a) (unquote-splicing b)))" (printed (datum "`(,a ,@b)")))
(test "#0=(quote #0#)" (printed (datum "#0='#0#")))
(test "(quote\n x)\n" (printed (list 'quote 'x) print-indent 0 print-width 5))
(test "'x\n" (pp 40 "'x"))


(display "\n--- print-graph and print-circle: the three marking modes ---\n")

;; print-graph on marks every substructure that occurs more than once, which is
;; what write-shared does
(for-each (lambda (obj) (test (via write-shared obj) (printed obj print-graph #t)))
          (append acyclic-data cyclic-data))

;; both off marks nothing, which is what write-simple does.  It is an error to
;; print a cyclic datum this way, so only the acyclic data is used here.
(for-each (lambda (obj) (test (via write-simple obj) (printed obj print-circle #f)))
          acyclic-data)

;; print-circle alone, the default, is what makes a cycle terminate
(test "#0=(1 2 . #0#)" (pr 100 "#0=(1 2 . #0#)"))
(test "((a b) (a b))"  (pr 100 "(#0=(a b) #0#)"))

;; print-graph wins over print-circle when both are given
(test "#0=(1 2 . #0#)" (pr 100 "#0=(1 2 . #0#)" print-graph #t print-circle #f))
(test "(#0=(a b) #0#)" (pr 100 "(#0=(a b) #0#)" print-graph #t print-circle #f))


(display "\n--- print-indent: inline, or laid out over lines ---\n")

;; The default is inline: one line, and no trailing newline.  An indent turns
;; line breaking on, and the result then ends with a newline.
(test "(a b c)"   (printed '(a b c)))
(test "(a b c)\n" (printed '(a b c) print-indent 0))
(test "(a b c)\n" (printed '(a b c) print-indent 4))

;; print-width only has an effect once there is an indent
(test "(aaaa bbbb cccc dddd)" (printed '(aaaa bbbb cccc dddd) print-width 12))
(test "(aaaa\n bbbb\n cccc\n dddd)\n"
      (printed '(aaaa bbbb cccc dddd) print-width 12 print-indent 0))

;; the indent is the column the first line is assumed to start at, so it shifts
;; every line after the first
(test "(aaaa\n           bbbb\n           cccc\n           dddd)\n"
      (printed '(aaaa bbbb cccc dddd) print-width 20 print-indent 10))

;; An indent gets line breaking, but not code layout: print lays a form out as
;; data, one element under the next, where pretty-print indents it as code.
(test "(define\n (f x)\n (if\n  (> x 0)\n  (g x)\n  (h x)))\n"
      (pr 24 "(define (f x) (if (> x 0) (g x) (h x)))" print-indent 0))
(test "(define (f x)\n  (if (> x 0)\n      (g x)\n      (h x)))\n"
      (pp 24 "(define (f x) (if (> x 0) (g x) (h x)))"))


(display "\n--- pretty-print: breaking lines to the width ---\n")

(test "1\n" (pp 40 "1"))
(test "'(a b)\n" (pp 40 "'(a b)"))
(test "'(a . b)\n" (pp 40 "'(a . b)"))
(test "`(,a ,@b)\n" (pp 40 "`(,a ,@b)"))

;; pretty-print always uses square brackets for binding lists; print follows
;; print-brackets, which pretty-print overrides
(test "(let\n  ([x 1]\n   [y 2]\n   [zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz 3])\n  (display x)\n  (display y))\n"
      (pp 80 "(let ((x 1) (y 2) (zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz 3)) (display x) (display y))"))
(test "(aaaa bbbb cccc dddd\n      eeee ffff)\n" (pp 20 "(aaaa bbbb cccc dddd eeee ffff)"))
(test "(let ([x 1] [y 2])\n  (display x))\n" (pp 20 "(let ((x 1) (y 2)) (display x))"))

(display "\n--- datum labels: cycles are always marked ---\n")

(test "#0=(a . #0#)\n" (pp 40 "#0=(a . #0#)"))
(test "(1 . #0=(2 . #0#))\n" (pp 40 "(1 . #0=(2 . #0#))"))
(test "#0=(1 #0# 3)\n" (pp 40 "#0=(1 #0# 3)"))
(test "(#0=(1 #0# 3) #0#)\n" (pp 40 "(#0=(1 #0# 3) #0#)"))
(test "(#0=(1 . #0#) #1=(1 . #1#))\n" (pp 40 "(#0=(1 . #0#) #1=(1 . #1#))"))
(test "(#0=(a b . #0#) '#1=(a b a b . #1#))\n" (pp 40 "(#0=(a b . #0#) '#1=(a b a b . #1#))"))
(test "#0=((1 . 2) (1 . 2) (3 . 4) . #0#)\n" (pp 40 "#0=((1 . 2) (1 . 2) (3 . 4) . #0#)"))
(test "#0=#(#0#)\n" (pp 40 "#0=#(#0#)"))
(test "#0=#(1 #0#)\n" (pp 40 "#0=#(1 #0#)"))
(test "#0=#(1 #0# 3)\n" (pp 40 "#0=#(1 #0# 3)"))
(test "(#0=#(1 #0# 3))\n" (pp 40 "(#0=#(1 #0# 3))"))
(test "#0=#(#0# 2 #0#)\n" (pp 40 "#0=#(#0# 2 #0#)"))
(test "#0=(a . #0#)\n" (pp 100 "#0=(a . #0#)"))
(test "(#0=(a . #0#) #0#)\n" (pp 100 "(#0=(a . #0#) #0#)"))
(test "#0=#(#0#)\n" (pp 100 "#0=#(#0#)"))
(test "#0=(a (b) . #0#)\n" (pp 100 "#0=(a . (#1=(b) . #0#))"))
(test "(#0=(a . #0#) #1=#(#1#))\n" (pp 100 "(#0=(a . #0#) #1=#(#1#))"))
(test "(1 2 3)\n" (pp 100 "(1 2 3)"))
(test "#0=(a . #0#)\n" (pp 100 "#0=(a . #0#)"))
(test "#0=#(#0#)\n" (pp 100 "#0=#(#0#)"))
(test "(#0=(1 . #0#) #1=(2 . #1#))\n" (pp 100 "(#0=(1 . #0#) #1=(2 . #1#))"))
(test "(#0=(a . #0#) #0#)\n" (pp 100 "(#0=(a . #0#) #0#)"))

(display "\n--- shared structure is expanded by default ---\n")

;; With graph marking off, which is the default, a datum that is merely shared
;; prints as a tree.  A CYCLIC datum still gets a label, so printing terminates.

(test "((1 . 2) (1 . 2) (3 . 4) (1 . 2) (1 . 2) (3 . 4))"
      (pr 100 "(#0=(1 . 2) #1=(1 . 2) #2=(3 . 4) #0# #1# #2#)"))
(test "((a b) (a b))"        (pr 100 "(#0=(a b) #0#)"))
(test "((a) (a))"            (pr 100 "(#0=(a) #0#)"))
(test "#(#(1) #(1))"         (pr 100 "#(#0=#(1) #0#)"))
(test "#(((a) (b) (a)) (b))" (pr 100 "#((#0=(a) #1=(b) #0#) #1#)"))

;; the same inputs with graph marking on
(test "(#0=(a b) #0#)"       (pr 100 "(#0=(a b) #0#)" print-graph #t))
(test "(#0=(a) #0#)"         (pr 100 "(#0=(a) #0#)" print-graph #t))
(test "#(#0=#(1) #0#)"       (pr 100 "#(#0=#(1) #0#)" print-graph #t))

(display "\n--- print-graph #t on shared and cyclic structure ---\n")

;; --- with ((print-graph #t))
(parameterize ([print-graph #t])
(test "#0=(a . #0#)\n" (pp 40 "#0=(a . #0#)"))
(test "(1 . #0=(2 . #0#))\n" (pp 40 "(1 . #0=(2 . #0#))"))
(test "#0=(1 #0# 3)\n" (pp 40 "#0=(1 #0# 3)"))
(test "(#0=(1 #0# 3) #0#)\n" (pp 40 "(#0=(1 #0# 3) #0#)"))
(test "(#0=(1 . #0#) #1=(1 . #1#))\n" (pp 40 "(#0=(1 . #0#) #1=(1 . #1#))"))
(test "(#0=(a b . #0#) '#1=(a b a b . #1#))\n" (pp 40 "(#0=(a b . #0#) '#1=(a b a b . #1#))"))
(test "(#0=(1 . 2) #1=(1 . 2) #2=(3 . 4) #0#\n  #1# #2#)\n" (pp 40 "(#0=(1 . 2) #1=(1 . 2) #2=(3 . 4) #0# #1# #2#)"))
(test "#0=((1 . 2) (1 . 2) (3 . 4) . #0#)\n" (pp 40 "#0=((1 . 2) (1 . 2) (3 . 4) . #0#)"))
(test "#0=#(#0#)\n" (pp 40 "#0=#(#0#)"))
(test "#0=#(1 #0#)\n" (pp 40 "#0=#(1 #0#)"))
(test "#0=#(1 #0# 3)\n" (pp 40 "#0=#(1 #0# 3)"))
(test "(#0=#(1 #0# 3))\n" (pp 40 "(#0=#(1 #0# 3))"))
(test "#0=#(#0# 2 #0#)\n" (pp 40 "#0=#(#0# 2 #0#)"))
(test "#0=(a . #0#)\n" (pp 100 "#0=(a . #0#)"))
(test "(#0=(a . #0#) #0#)\n" (pp 100 "(#0=(a . #0#) #0#)"))
(test "#0=#(#0#)\n" (pp 100 "#0=#(#0#)"))
(test "(#0=(a b) #0#)\n" (pp 100 "(#0=(a b) #0#)"))
(test "#0=(#1=(a) #1# . #0#)\n" (pp 100 "#0=(#1=(a) #1# . #0#)"))
(test "(#0=(a . #0#) #1=(b c) #1#)\n" (pp 100 "(#0=(a . #0#) #1=(b c) #1#)"))
(test "#0=(a (b) . #0#)\n" (pp 100 "#0=(a . (#1=(b) . #0#))"))
(test "#0=(#(#1=(a b) #1#) . #0#)\n" (pp 100 "#0=(#(#1=(a b) #1#) . #0#)"))
(test "(#0=(a . #0#) #1=#(#1#))\n" (pp 100 "(#0=(a . #0#) #1=#(#1#))"))
(test "#0=(#1=(a . #0#) #1#)\n" (pp 100 "#0=(#1=(a . #0#) #1#)"))
(test "(1 2 3)\n" (pp 100 "(1 2 3)"))
(test "(#0=(a) #0#)\n" (pp 100 "(#0=(a) #0#)"))
(test "#0=(a . #0#)\n" (pp 100 "#0=(a . #0#)"))
(test "(#0=(a) #1=(b . #1#) #0#)\n" (pp 100 "(#0=(a) #1=(b . #1#) #0#)"))
(test "#(#0=#(1) #0#)\n" (pp 100 "#(#0=#(1) #0#)"))
(test "#0=#(#0#)\n" (pp 100 "#0=#(#0#)"))
(test "(#0=(1 . #0#) #1=(2 . #1#))\n" (pp 100 "(#0=(1 . #0#) #1=(2 . #1#))"))
(test "#0=(#1=(a . #0#) . #1#)\n" (pp 100 "#0=(#1=(a . #0#) . #1#)"))
(test "(#0=(a . #0#) #0#)\n" (pp 100 "(#0=(a . #0#) #0#)"))
(test "#((#0=(a) #1=(b) #0#) #1#)\n" (pp 100 "#((#0=(a) #1=(b) #0#) #1#)"))
)

(display "\n--- print-level and print-length ---\n")

(test "a\n" (cut 0 10 "a"))
(test "(...)\n" (cut 0 10 "(a b c)"))
(test "#(...)\n" (cut 0 10 "#(a b c)"))
(test "(a b c)\n" (cut 1 10 "(a b c)"))
(test "((...) c)\n" (cut 1 10 "((a b) c)"))
(test "#((...) c)\n" (cut 1 10 "#((a b) c)"))
(test "(a b ...)\n" (cut 1 2 "(a b c d)"))
(test "#(a b ...)\n" (cut 1 2 "#(a b c d)"))
(test "(a b . c)\n" (cut 1 2 "(a b . c)"))
(test "((a b) (c d))\n" (cut 2 10 "((a b) (c d))"))
(test "(((...)) b)\n" (cut 2 10 "(((a)) b)"))
(test "#(#(a b) #(c #(...)))\n" (cut 2 10 "#(#(a b) #(c #(d)))"))
(test "((a b ...) (d e ...) ...)\n" (cut 2 2 "((a b c) (d e f) (g h i))"))
(test "#((a b ...) (d e ...) ...)\n" (cut 2 2 "#((a b c) (d e f) (g h i))"))
(test "(a (b c d ...) f ...)\n" (cut 2 3 "(a (b c d e) f g)"))
(test "(a b c d . e)\n" (cut 3 10 "(a b c d . e)"))
(test "#(a (b . c) #(d e (f g)))\n" (cut 3 10 "#(a (b . c) #(d e (f g)))"))
(test "#(a (b . c) #(d e (...)))\n" (cut 2 10 "#(a (b . c) #(d e (f g)))"))
(test "(a . b)\n" (cut 1 10 "(a . b)"))
(test "(a . b)\n" (cut 1 1 "(a . b)"))
(test "((a . b) . c)\n" (cut 2 10 "((a . b) . c)"))
(test "((a . b) . c)\n" (cut 2 1 "((a . b) . c)"))
(test "(a b ...)\n" (cut 3 2 "(a b c . d)"))
(test "(...)\n" (cut 0 #f "(a b c)"))
(test "(a (...) d)\n" (cut 1 #f "(a (b c) d)"))
(test "(a (b (...)) d)\n" (cut 2 #f "(a (b (c)) d)"))
(test "#(...)\n" (cut 0 #f "#(1 2 3)"))
(test "#(1 #(...) 3)\n" (cut 1 #f "#(1 #(2) 3)"))
(test "(a b . #(...))\n" (cut 1 #f "(a b . #(c))"))
(test "(...)\n" (cut #f 0 "(a b c)"))
(test "(a ...)\n" (cut #f 1 "(a b c)"))
(test "(a b c)\n" (cut #f 3 "(a b c)"))
(test "#(...)\n" (cut #f 0 "#(1 2 3)"))
(test "#(1 2 ...)\n" (cut #f 2 "#(1 2 3 4)"))
(test "(a ...)\n" (cut #f 1 "(a b . c)"))
(test "(a b . c)\n" (cut #f 2 "(a b . c)"))
(test "(a b ...)\n" (cut #f 2 "(a b c . d)"))
(test "((a ...) ...)\n" (cut #f 1 "((a b) (c d))"))
(test "((...) ...)\n" (cut 1 1 "((a b) (c d))"))
(test "(((...) ...) ...)\n" (cut 2 1 "(((a) b) c)"))
(test "(...)\n" (cut 0 0 "(a b c)"))
(test "((...) (...) ...)\n" (cut 1 2 "((a b c) (d e f) (g h i))"))
(test "#(#(...) #(...) ...)\n" (cut 1 2 "#(#(a b c) #(d e f) #(g h i))"))
(test "(a b . #(...))\n" (cut 1 #f "(a b . #(c d))"))
(test "((...) . c)\n" (cut 1 #f "((a . b) . c)"))
;; --- with ((print-graph #t))
(parameterize ([print-graph #t])
(test "(...)\n" (cut 0 #f "#0=(a . #0#)"))
(test "#0=(a . #0#)\n" (cut 1 #f "#0=(a . #0#)"))
(test "(a ...)\n" (cut 1 1 "#0=(a b c . #0#)"))
(test "(#0=(a b) #0# ...)\n" (cut 2 2 "(#0=(a b) #0# #0#)"))
(test "((...) (...) ...)\n" (cut 1 2 "(#0=(a b) #0# #0#)"))
(test "(#(a (...)) b)\n" (cut 2 #f "#0=(#(a #0#) b)"))
(test "(#(...) b)\n" (cut 1 #f "#0=(#(a #0#) b)"))
(test "(#(a ...) ...)\n" (cut 2 1 "#0=(#(a #0#) b)"))
(test "(#0=(a . b) #0# . #0#)\n" (cut 2 #f "(#0=(a . b) #0# . #0#)"))
(test "((...) (...) a . b)\n" (cut 1 #f "(#0=(a . b) #0# . #0#)"))
(test "#0=((a . b) (#0# . c) ...)\n" (cut 3 2 "#0=((a . b) (#0# . c) d)"))
(test "((...) ...)\n" (cut 1 1 "#0=((a . b) (#0# . c) d)"))
(test "#0=(#(#0#) . #0#)\n" (cut 2 #f "#0=(#(#0#) . #0#)"))
(test "#0=(#(...) . #0#)\n" (cut 1 #f "#0=(#(#0#) . #0#)"))
(test "(#0=(a b) #0# ...)\n" (cut 2 2 "#0=(#1=(a b) #1# . #0#)"))
(test "((...) (...) ...)\n" (cut 1 2 "#0=(#1=(a b) #1# . #0#)"))
(test "(a #0=#(b (...)) . #0#)\n" (cut 2 #f "#0=(a . (#1=#(b #0#) . #1#))"))
(test "(a #(...) . #(...))\n" (cut 1 #f "#0=(a . (#1=#(b #0#) . #1#))"))
(test "(a ...)\n" (cut 2 1 "#0=(a . (#1=#(b #0#) . #1#))"))
(test "(#0=(a . #0#) #1=#(#1#) (b c) ...)\n" (cut 3 3 "(#0=(a . #0#) #1=#(#1#) #2=(b c) #2#)"))
(test "(...)\n" (cut 0 #f "#0=(a . #0#)"))
(test "#0=(a . #0#)\n" (cut 1 #f "#0=(a . #0#)"))
(test "((...) (...))\n" (cut 1 #f "(#0=(a b) #0#)"))
(test "((a ...) ...)\n" (cut #f 1 "(#0=(a b c) #0#)"))
(test "#(...)\n" (cut 0 #f "#0=#(#0#)"))
(test "#(#(...))\n" (cut 1 #f "#0=#(#0#)"))
(test "((...) (...) (...))\n" (cut 1 #f "(#0=(a) #1=(#0#) #1#)"))
(test "(a ...)\n" (cut #f 1 "(a #0=(b) #0#)"))
(test "(a (b) ...)\n" (cut #f 2 "(a #0=(b) c #0#)"))
(test "#0=(a #1=(b . #0#) . #1#)\n" (cut 2 #f "#0=(a #1=(b . #0#) . #1#)"))
(test "((...) (...))\n" (cut 1 #f "(#0=(a . b) #0#)"))
(test "(a ...)\n" (cut #f 1 "#0=(a b . #0#)"))
(test "(a b ...)\n" (cut #f 2 "#0=(a b . #0#)"))
(test "(...)\n" (cut 0 #f "(#0=() #0#)"))
(test "(() ())\n" (cut 1 #f "(#0=() #0#)"))
(test "#(#(...) ...)\n" (cut 1 1 "#(#0=#(a b c) #0#)"))
(test "#(#(a ...) ...)\n" (cut #f 1 "#(#0=#(a b c) #0#)"))
(test "((...) b ...)\n" (cut 1 2 "(#0=(a) b c . #0#)"))
(test "((...) ...)\n" (cut 1 1 "((#0=(a) #0#) (#1=(b) #1#))"))
(test "#((...) (...))\n" (cut 1 #f "#0=#(#1=(a . #0#) #1#)"))
(test "(...)\n" (cut 0 0 "#0=(a #1=(b . #2=#(#1# c)) (#2# . d) #0# . #3=(e . #3#))"))
(test "(...)\n" (cut 0 1 "#0=(a #1=(b . #2=#(#1# c)) (#2# . d) #0# . #3=(e . #3#))"))
(test "(...)\n" (cut 0 2 "#0=(a #1=(b . #2=#(#1# c)) (#2# . d) #0# . #3=(e . #3#))"))
(test "(...)\n" (cut 0 3 "#0=(a #1=(b . #2=#(#1# c)) (#2# . d) #0# . #3=(e . #3#))"))
(test "(...)\n" (cut 1 0 "#0=(a #1=(b . #2=#(#1# c)) (#2# . d) #0# . #3=(e . #3#))"))
(test "(a ...)\n" (cut 1 1 "#0=(a #1=(b . #2=#(#1# c)) (#2# . d) #0# . #3=(e . #3#))"))
(test "(a (...) ...)\n" (cut 1 2 "#0=(a #1=(b . #2=#(#1# c)) (#2# . d) #0# . #3=(e . #3#))"))
(test "(a (...) (...) ...)\n" (cut 1 3 "#0=(a #1=(b . #2=#(#1# c)) (#2# . d) #0# . #3=(e . #3#))"))
(test "(...)\n" (cut 2 0 "#0=(a #1=(b . #2=#(#1# c)) (#2# . d) #0# . #3=(e . #3#))"))
(test "(a ...)\n" (cut 2 1 "#0=(a #1=(b . #2=#(#1# c)) (#2# . d) #0# . #3=(e . #3#))"))
(test "(a (b . #(...)) ...)\n" (cut 2 2 "#0=(a #1=(b . #2=#(#1# c)) (#2# . d) #0# . #3=(e . #3#))"))
(test "(a (b . #(...)) (#(...) . d) ...)\n" (cut 2 3 "#0=(a #1=(b . #2=#(#1# c)) (#2# . d) #0# . #3=(e . #3#))"))
(test "(...)\n" (cut 3 0 "#0=(a #1=(b . #2=#(#1# c)) (#2# . d) #0# . #3=(e . #3#))"))
(test "(a ...)\n" (cut 3 1 "#0=(a #1=(b . #2=#(#1# c)) (#2# . d) #0# . #3=(e . #3#))"))
(test "(a #0=(b . #(#0# c)) ...)\n" (cut 3 2 "#0=(a #1=(b . #2=#(#1# c)) (#2# . d) #0# . #3=(e . #3#))"))
(test "(a #0=(b . #1=#(#0# c)) (#1# . d) ...)\n" (cut 3 3 "#0=(a #1=(b . #2=#(#1# c)) (#2# . d) #0# . #3=(e . #3#))"))
(test "(...)\n" (cut 0 0 "#0=(#1=(a (b)) #2=#(c d #1#) (e . #0#) f)"))
(test "(...)\n" (cut 0 1 "#0=(#1=(a (b)) #2=#(c d #1#) (e . #0#) f)"))
(test "(...)\n" (cut 0 2 "#0=(#1=(a (b)) #2=#(c d #1#) (e . #0#) f)"))
(test "(...)\n" (cut 0 3 "#0=(#1=(a (b)) #2=#(c d #1#) (e . #0#) f)"))
(test "(...)\n" (cut 1 0 "#0=(#1=(a (b)) #2=#(c d #1#) (e . #0#) f)"))
(test "((...) ...)\n" (cut 1 1 "#0=(#1=(a (b)) #2=#(c d #1#) (e . #0#) f)"))
(test "((...) #(...) ...)\n" (cut 1 2 "#0=(#1=(a (b)) #2=#(c d #1#) (e . #0#) f)"))
(test "((...) #(...) (...) ...)\n" (cut 1 3 "#0=(#1=(a (b)) #2=#(c d #1#) (e . #0#) f)"))
(test "(...)\n" (cut 2 0 "#0=(#1=(a (b)) #2=#(c d #1#) (e . #0#) f)"))
(test "((a ...) ...)\n" (cut 2 1 "#0=(#1=(a (b)) #2=#(c d #1#) (e . #0#) f)"))
(test "((a (...)) #(c d ...) ...)\n" (cut 2 2 "#0=(#1=(a (b)) #2=#(c d #1#) (e . #0#) f)"))
(test "#0=((a (...)) #(c d (...)) (e . #0#) ...)\n" (cut 2 3 "#0=(#1=(a (b)) #2=#(c d #1#) (e . #0#) f)"))
(test "(...)\n" (cut 3 0 "#0=(#1=(a (b)) #2=#(c d #1#) (e . #0#) f)"))
(test "((a ...) ...)\n" (cut 3 1 "#0=(#1=(a (b)) #2=#(c d #1#) (e . #0#) f)"))
(test "((a (b)) #(c d ...) ...)\n" (cut 3 2 "#0=(#1=(a (b)) #2=#(c d #1#) (e . #0#) f)"))
(test "#0=(#1=(a (b)) #(c d #1#) (e . #0#) ...)\n" (cut 3 3 "#0=(#1=(a (b)) #2=#(c d #1#) (e . #0#) f)"))
(test "'`,,@(3 . #(a b c d ...))\n" (cut 3 4 "'`,,@(3 . #(a b c d e f g))))"))
(test "'...\n" (cut 0 0 "#0='#0#"))
(test "'...\n" (cut 0 1 "#0='#0#"))
(test "#0='#0#\n" (cut 1 0 "#0='#0#"))
(test "#0='#0#\n" (cut 1 1 "#0='#0#"))
(cond-expand (skint
  (test "#&#&#&#&...\n" (cut 3 4 "#&#&#&#&(3 . #(a b c d e f g))))"))
  (test "#&...\n" (cut 0 0 "#0=#&#0#"))
  (test "#&...\n" (cut 0 1 "#0=#&#0#"))
  (test "#&...\n" (cut 1 0 "#0=#&#0#"))
  (test "#&#&...\n" (cut 1 1 "#0=#&#0#"))
) (else))
)

(display "\n--- print-level and print-length together ---\n")

;; one form seen through every combination that changes the result
(define cut-form "(if (member x y) (+ (car x) 3) '(foo . #(a b c d \"Baz\")))")

(test "(...)\n"                       (cut 0 1 cut-form))
(test "(if ...)\n"                    (cut 1 1 cut-form))
(test "(if (...) ...)\n"              (cut 1 2 cut-form))
(test "(if (...) (...) ...)\n"        (cut 1 3 cut-form))
(test "(if (...) (...) '...)\n"       (cut 1 4 cut-form))
(test "(if ...)\n"                    (cut 2 1 cut-form))
(test "(if (member x ...) ...)\n"     (cut 2 2 cut-form))
(test "(if (member x y) (+ (...) 3) ...)\n" (cut 2 3 cut-form))
(test "(if (member x ...) ...)\n"     (cut 3 2 cut-form))
(test "(if (member x y) (+ (car x) 3) ...)\n" (cut 3 3 cut-form))
(test "(if (member x y) (+ (car x) 3) '(foo . #(a b c d ...)))\n" (cut 3 4 cut-form))
(test "(if (member x y) (+ (car x) 3) '(foo . #(a b c d \"Baz\")))\n" (cut 3 5 cut-form))

(display "\n--- print-radix ---\n")

;; a prefix is printed whenever the radix is not 10
(define bv "#u8(0 10 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30)")

(test "#u8(0 10 2 3 4 5 6 7\n    8 9 10 11 12 13\n    14 15 16 17 18\n    19 20 21 22 23\n    24 25 26 27 28\n    29 30)\n"
      (pp 20 bv))
(test "#u8(0 10 2 3 4 5 6 7\n    8 9 10 11 12 13\n    14 15 16 17 18\n    19 20 21 22 23\n    24 25 26 27 28\n    29 30)\n"
      (pp 20 bv print-radix 10))
(test "#u8(#o0 #o12 #o2 #o3\n    #o4 #o5 #o6 #o7\n    #o10 #o11 #o12\n    #o13 #o14 #o15\n    #o16 #o17 #o20\n    #o21 #o22 #o23\n    #o24 #o25 #o26\n    #o27 #o30 #o31\n    #o32 #o33 #o34\n    #o35 #o36)\n"
      (pp 20 bv print-radix 8))
(test "#u8(#x0 #xa #x2 #x3\n    #x4 #x5 #x6 #x7\n    #x8 #x9 #xa #xb\n    #xc #xd #xe #xf\n    #x10 #x11 #x12\n    #x13 #x14 #x15\n    #x16 #x17 #x18\n    #x19 #x1a #x1b\n    #x1c #x1d #x1e)\n"
      (pp 20 bv print-radix 16))
(test "#b1010" (pr 40 "10" print-radix 2))
(test "#o12"   (pr 40 "10" print-radix 8))
(test "10"     (pr 40 "10" print-radix 10))
(test "#xa"    (pr 40 "10" print-radix 16))

(display "\n--- print-brackets ---\n")

;; Brackets are a code convention, and print prints data, so it takes no notice
;; of the parameter.
(test "(let ((x 1) (y 2)) (display x))" (pr 40 "(let ((x 1) (y 2)) (display x))" print-brackets #t))
(test "(let ((x 1) (y 2)) (display x))" (pr 40 "(let ((x 1) (y 2)) (display x))" print-brackets #f))
;; pretty-print prints code, and brackets binding lists unless told not to
(test "(let ([x 1] [y 2])\n  (display x))\n" (pp 20 "(let ((x 1) (y 2)) (display x))"))
(test "(let ((x 1) (y 2))\n  (display x))\n" (pp 20 "(let ((x 1) (y 2)) (display x))" print-brackets #f))

(display "\n--- pretty-style ---\n")

(test '(_ d . body) (pretty-style 'lambda))
(test '(_ dc* . body) (pretty-style 'let*))
(test #f (pretty-style 'no-such-form-at-all))
;; setting takes effect; its return value is not specified
(pretty-style (quote my-binding-form) (quote (_ dc* . body)))
(test '(_ dc* . body) (pretty-style 'my-binding-form))
(test "(my-binding-form\n  ([x 1] [y 2])\n  (display x))\n"
      (pp 20 "(my-binding-form ((x 1) (y 2)) (display x))"))

(display "\n--- pretty-print's own settings are defaults ---\n")

;; pretty-print prints code: laid out over lines, binding lists in brackets.
(test "(let ([x 1] [y 2])\n  (display x))\n" (pp 20 "(let ((x 1) (y 2)) (display x))"))

;; Each of the settings it supplies can be overridden by the caller, because it
;; appends them to the caller's arguments and the first occurrence wins.
(test "(let ((x 1) (y 2))\n  (display x))\n"
      (pp 20 "(let ((x 1) (y 2)) (display x))" print-brackets #f))
(test "(let ([x 1] [y 2]) (display x))"
      (pp 20 "(let ((x 1) (y 2)) (display x))" print-indent #f))

;; It supplies nothing for the marking mode, so it follows write as print does.
(test "((a b) (a b))\n" (pp 100 "(#0=(a b) #0#)"))
(test "(#0=(a b) #0#)\n" (pp 100 "(#0=(a b) #0#)" print-graph #t))

(display "\n--- parameter validation ---\n")

(test-error (parameterize ([print-width 0]) 'unreachable))
(test-error (parameterize ([print-width -1]) 'unreachable))
(test-error (parameterize ([print-width 1.5]) 'unreachable))
(test-error (parameterize ([print-radix 3]) 'unreachable))
(test-error (parameterize ([print-level -1]) 'unreachable))
(test-error (parameterize ([print-length -1]) 'unreachable))
(test-error (parameterize ([print-indent -1]) 'unreachable))
(test-error (parameterize ([print-indent 1.5]) 'unreachable))
;; an invalid value is rejected wherever it is given
(test-error (print 1 (open-output-string) print-width 0))
;; a keyword with no value, or an unknown one, is rejected
(test-error (print 1 (open-output-string) print-width))
;; the first occurrence of a keyword wins
(test "1" (pr 40 "1" print-width 0))

(test 80 (print-width))
(test #t (print-circle))
(test #f (print-graph))
(test 10 (print-radix))
(test #f (print-length))
(test #f (print-level))
(test #f (print-indent))
(test #f (print-brackets))

(display "\n--- printing to the current output port ---\n")

(test-output "(a b)"   (print '(a b)))
(test-output "(a b)\n" (pretty-print '(a b)))

(define sexp1
'(define (ast-pretty a . opt-port)
  (define (width x)
    (cond
      ((string? x) (+ (string-length x) 2))
      ((char? x) 3)
      ((number? x) 5)
      ((symbol? x) (string-length (symbol->string x)))
      ((boolean? x) 2)
      ((null? x) 2)
      (else 10)))
  (define (ast-width a)
    (cond
      ((ast-atom? a) (width (ast-atom->val a)))
      ((ast-null? a) 2)
      ((ast-pair? a)
       (if (ast-list? a)
           (let loop ((a a) (sum 0) (count 0))
             (if (ast-null? a)
                 (+ sum count 1)
                 (loop
                   (ast-cdr a)
                   (+ sum (ast-width (ast-car a)))
                   (+ count 1))))
           (+ 1 (ast-width (ast-car a)) 3
              (ast-width (ast-cdr a)) 1)))
      (else 0)))
  (define (ast-width-hello a)
    (display "hello!\n")
    (cond
      ((ast-atom? a) (width (ast-atom->val a)))
      ((ast-null? a) 2)
      ((ast-pair? a)
       (if (ast-list? a)
           (let loop ((a a) (sum 0) (count 0))
             (if (ast-null? a)
                 (+ sum count 1)
                 (loop
                   (ast-cdr a)
                   (+ sum (ast-width (ast-car a)))
                   (+ count 1))))
           (+ 1 (ast-width (ast-car a)) 3
              (ast-width (ast-cdr a)) 1)))
      (else (cond ((foo) => (lambda (v) (bar v 0)))
                  (else (begin (newline) (if a #t #f)))))))
  (define (break? cur-col width max-col)
    (> (+ cur-col width) max-col))
  (define (print-indent n port)
    (do ((i 0 (+ i 1))) ((= i n)) (display "  " port)))
  (define (pretty a col indent port max-col)
    (cond
      ((ast-atom? a)
       (let ((w (width (ast-atom->val a))))
         (write (ast-atom->val a) port)
         (+ col w)))
      ((ast-null? a) (display "()" port) (+ col 2))
      ((ast-pair? a)
       (if (ast-list? a)
           (let* ((total-width (ast-width a))
                  (remaining (- max-col col))
                  (inline? (<= total-width remaining)))
             (if inline?
                 (begin
                   (display "(" port)
                   (let loop ((first? #t)
                              (rest a)
                              (cur-col (+ col 1)))
                     (if (ast-null? rest)
                         (begin (display ")" port) (+ cur-col 1))
                         (let* ((elem-width (ast-width
                                              (ast-car rest)))
                                (new-col (if first?
                                             (pretty
                                               (ast-car rest) cur-col
                                               indent port max-col)
                                             (begin
                                               (display " " port)
                                               (pretty
                                                 (ast-car rest)
                                                 (+ cur-col 1) indent
                                                 port max-col)))))
                           (loop #f (ast-cdr rest) new-col)))))
                 (begin
                   (display "(" port)
                   (let ((new-indent (+ indent 1)))
                     (let loop ((rest a) (cur-col (* new-indent 2)))
                       (if (ast-null? rest)
                           (begin (display ")" port) (+ cur-col 1))
                           (let* ((elem (ast-car rest))
                                  (next-col (pretty elem
                                              cur-col new-indent port
                                              max-col)))
                             (unless (ast-null? (ast-cdr rest))
                               (newline port)
                               (print-indent new-indent port))
                             (loop
                               (ast-cdr rest)
                               (* new-indent 2)))))))))
           (begin
             (display "(" port)
             (let ((car-col (pretty (ast-car a) (+ col 1) indent
                              port max-col)))
               (display " . " port)
               (let ((cdr-col (pretty (ast-cdr a) (+ car-col 3)
                                indent port max-col)))
                 (display ")" port)
                 (+ cdr-col 1))))))))
  (let ((port (if (null? opt-port)
                  (current-output-port)
                  (car opt-port))))
    (pretty a 0 0 port 80)
    (newline port))))

(define sexp2
'(define (%read port simple? ci?)
  (define-syntax r-error
    (syntax-rules () ((_ p msg a ...) (read-error p msg a ...))))
  (define fold-case? (or ci? (port-fold-case? port)))
  (define shared '())
  (define (make-shared-ref loc) (lambda () (unbox loc)))
  (define (shared-ref? form) (procedure? form))
  (define (patch-ref! form) (if (procedure? form) (patch-ref! (form)) form))
  (define (patch-shared! form)
    (cond ((pair? form)
           (if (procedure? (car form)) 
               (set-car! form (patch-ref! (car form)))
               (patch-shared! (car form)))
           (if (procedure? (cdr form)) 
               (set-cdr! form (patch-ref! (cdr form)))
               (patch-shared! (cdr form))))
          ((vector? form)
           (let loop ((i 0))
             (when (fx<? i (vector-length form))
               (let ((fi (vector-ref form i)))
                 (if (procedure? fi) 
                     (vector-set! form i (patch-ref! fi))
                     (patch-shared! fi)))
               (loop (fx+ i 1)))))
          ((box? form)
           (if (procedure? (unbox form))
               (set-box! form (patch-shared! (unbox form)))
               (patch-shared! (unbox form))))))
  (define (patch-shared form) (patch-shared! form) form)           

  (define reader-token-marker #f)
  (define close-paren #f)
  (define close-bracket #f)
  (define dot #f)
  (define () ; idless
    (let ((rtm (list 'reader-token)))
          (set! reader-token-marker rtm)
          (set! close-paren (cons rtm "right parenthesis"))
          (set! close-bracket (cons rtm "right bracket"))
          (set! dot (cons rtm "\" . \""))))

  (define (reader-token? form)
    (and (pair? form) (eq? (car form) reader-token-marker)))

  (define (char-hex-digit? c)
    (let ((scalar-value (char->integer c)))
      (or (and (>= scalar-value 48) (<= scalar-value 57))
          (and (>= scalar-value 65) (<= scalar-value 70))
          (and (>= scalar-value 97) (<= scalar-value 102)))))

  (define (char-delimiter? c)
    (or (char-whitespace? c)
        (char=? c #\)) (char=? c #\()
        (char=? c #\)) (char=? c #\()
        (char=? c #\") (char=? c #\|)
        (char=? c #\;)))

  (define (sub-read-carefully p)
    (let ((form (sub-read p)))
      (cond ((eof-object? form)
             (r-error p "unexpected end of file"))
            ((reader-token? form) 
             (r-error p "unexpected token:" (cdr form)))
            (else form))))

  (define (sub-read-shebang p)
    (if (eqv? (peek-char p) #\space)
        (string->symbol (string-trim-whitespace (read-line p)))
        (sub-read-carefully p)))

  (define (sub-read p)
    ; bumped code using %read-ahead builtin/instruction
    (let ((c (%read-ahead fold-case? p)))
      (cond ((not (char? c)) c) ; handled by %read-ahead
            ((char=? c #\() (sub-read-list c p close-paren #t))
            ((char=? c #\)) close-paren)
            ((char=? c #\() (sub-read-list c p close-bracket #t))
            ((char=? c #\)) close-bracket)
            ((char=? c #\.) dot)
            ((char=? c #\') (list 'quote (sub-read-carefully p)))
            ((char=? c #\`) (list 'quasiquote (sub-read-carefully p)))
            ((char=? c #\n) (r-error p "unsupported number syntax (implementation restriction)"))
            ((char=? c #\d) (r-error p "invalid delimiter"))
            ((char=? c #\y) (r-error p "unsupported symbol syntax"))
            ((char=? c #\z) (r-error p "invalid token"))
            ((char=? c #\,)
             (let ((next (peek-char p)))
               (cond ((eof-object? next)
                      (r-error p "end of file after ,"))
                     ((char=? next #\@)
                      (read-char p)
                      (list 'unquote-splicing (sub-read-carefully p)))
                     (else (list 'unquote (sub-read-carefully p))))))
            ((char=? c #\")
             (let loop ((l '()))
               (let ((c (read-char p)))
                 (cond ((eof-object? c)
                        (r-error p "end of file within a string"))
                       ((char=? c #\\)
                        (let ((e (sub-read-strsym-char-escape p 'string)))
                          (loop (if e (cons e l) l))))
                       ((char=? c #\") (list->string (reverse! l)))
                       (else (loop (cons c l)))))))
            ((char=? c #\|)
             (let loop ((l '()))
               (let ((c (read-char p)))
                 (cond ((eof-object? c)
                        (r-error p "end of file within a |symbol|"))
                       ((char=? c #\\)
                        (let ((e (sub-read-strsym-char-escape p 'symbol)))
                          (loop (if e (cons e l) l))))
                       ((char=? c #\|) (string->symbol (list->string (reverse! l))))
                       (else (loop (cons c l)))))))
            ((char=? c #\#)
             (let ((c (peek-char p)))
               (cond ((eof-object? c) (r-error p "end of file after #"))
                     ((char=? c #\!)
                      (read-char p)
                      (let ((name (sub-read-shebang p)))
                        (case name
                          ((fold-case no-fold-case) 
                           (set! fold-case? (eq? name 'fold-case)) 
                           (set-port-fold-case! p fold-case?)
                           (sub-read p))
                          (else (if (symbol? name) 
                                    (symbol->shebang name)
                                    (r-error p "unexpected name after #!" name))))))
                     ((or (char-ci=? c #\t) (char-ci=? c #\f) (char-ci=? c #\s) (char-ci=? c #\u))
                      (let ((name (sub-read-carefully p)))
                        (case name 
                          ((t true) #t) 
                          ((f false) #f)
                          ((u8)  (list->numvector (sub-read-numerical-list p name) 0))
                          ((s8)  (list->numvector (sub-read-numerical-list p name) 1))
                          ((u16) (list->numvector (sub-read-numerical-list p name) 2))
                          ((s16) (list->numvector (sub-read-numerical-list p name) 3))
                          ((f32) (list->numvector (sub-read-numerical-list p name) 10))
                          ((f64) (list->numvector (sub-read-numerical-list p name) 11))
                          (else (r-error p "unexpected name after #" name)))))
                     ((char=? c #\&)
                      (read-char p)
                      (box (sub-read-carefully p)))
                     ((char=? c #\;)
                      (read-char p)
                      (sub-read-carefully p) 
                      (sub-read p))
                     ((char=? c #\|)
                      (read-char p)
                      (let recur () ;starts right after opening #|
                        (let ((next (read-char p)))
                          (cond
                            ((eof-object? next)
                             (r-error p "end of file in #| comment"))
                            ((char=? next #\|)
                             (let ((next (peek-char p)))
                               (cond
                                 ((eof-object? next)
                                  (r-error p "end of file in #| comment"))
                                 ((char=? next #\#) (read-char p))
                                 (else (recur)))))
                            ((char=? next #\#)
                             (let ((next (peek-char p)))
                               (cond
                                 ((eof-object? next)
                                  (r-error p "end of file in #| comment"))
                                 ((char=? next #\|) (read-char p) (recur) (recur))
                                 (else (recur)))))
                            (else (recur)))))
                      (sub-read p))
                     ((char=? c #\() ;)
                      (read-char p)
                      (list->vector (sub-read-list c p close-paren #f)))
                     ((char=? c #\\)
                      (read-char p)
                      (let ((c (peek-char p)))
                        (cond
                          ((eof-object? c)
                           (r-error p "end of file after #\\"))
                          ((char=? #\x c)
                           (read-char p)
                           (if (char-delimiter? (peek-char p))
                               c
                               (sub-read-x-char-escape p #f)))
                          ((char-alphabetic? c)
                           (let ((name (sub-read-carefully p)))
                             (if (= (string-length (symbol->string name)) 1)
                                 c
                                 (case name
                                   ((null) (integer->char #x00))
                                   ((space) #\space)
                                   ((alarm) #\alarm)
                                   ((backspace) #\backspace)
                                   ((delete) (integer->char #x7F)) ; todo: support by SFC
                                   ((escape) (integer->char #x1B))
                                   ((tab) #\tab)
                                   ((newline linefeed) #\newline)
                                   ;((vtab) #\vtab)
                                   ;((page) #\page)
                                   ((return) #\return)
                                   (else (r-error p "unknown #\\ name" name))))))
                          (else (read-char p) c))))
                     ((char-numeric? c)
                      (when simple? (r-error p "#N=/#N# notation is not allowed in this mode")) 
                      (let loop ((l '()))
                        (let ((c (read-char p)))
                          (cond ((eof-object? c)
                                 (r-error p "end of file within a #N notation"))
                                ((char-numeric? c)
                                 (loop (cons c l)))
                                ((char=? c #\#) 
                                 (let* ((s (list->string (reverse! l))) (n (string->number s)))
                                   (cond ((and (fixnum? n) (assq n shared)) => cdr)
                                         (else (r-error p "unknown #n# reference:" s)))))   
                                ((char=? c #\=) 
                                 (let* ((s (list->string (reverse! l))) (n (string->number s)))
                                   (cond ((not (fixnum? n)) (r-error p "invalid #n= reference:" s))
                                         ((assq n shared) (r-error p "duplicate #n= tag:" n)))
                                   (let ((loc (box #f)))
                                     (set! shared (cons (cons n (make-shared-ref loc)) shared))
                                     (let ((form (sub-read-carefully p)))
                                       (cond ((shared-ref? form) (r-error p "#n= has another label as target" s))
                                             (else (set-box! loc form) form))))))
                                (else (r-error p "invalid terminator for #N notation"))))))
                     (else (r-error p "unknown # syntax" c)))))
            (else (r-error p "illegal character read" c)))))

  (define (sub-read-list c p close-token dot?)
    (let ((form (sub-read p)))
      (if (eq? form dot)
          (r-error p "missing car -- ( immediately followed by .") ;)
          (let recur ((form form))
            (cond ((eof-object? form)
                   (r-error p "eof inside list -- unbalanced parentheses"))
                  ((eq? form close-token) '())
                  ((eq? form dot)
                   (if dot?
                       (let* ((last-form (sub-read-carefully p))
                              (another-form (sub-read p)))
                         (if (eq? another-form close-token)
                             last-form
                             (r-error p "randomness after form after dot" another-form)))
                       (r-error p "dot in #(...)")))
                  ((reader-token? form)
                   (r-error p "error inside list --" (cdr form)))
                  (else (cons form (recur (sub-read p)))))))))

  (define (sub-read-numerical-list p ts)
    (unless (eq? (read-char p) #\()
      (r-error p (format "invalid ~avector syntax" ts)))
    (let recur ((form (sub-read p)))
      (cond ((eof-object? form)
             (r-error p (format "eof inside ~avector" ts)))
            ((eq? form close-paren) '())
            ((reader-token? form)
             (r-error p (format "error inside ~avector --" ts) (cdr form)))
            ((or (and (eq? ts 'u8)  (fixnum? form) (fx<=? 0 form 255))
                 (and (eq? ts 's8)  (fixnum? form) (fx<=? -128 form 127))
                 (and (eq? ts 'u16) (fixnum? form) (fx<=? 0 form 65535))
                 (and (eq? ts 's16) (fixnum? form) (fx<=? -32768 form 32767))
                 (and (eq? ts 'f32) (flonum? form))
                 (and (eq? ts 'f64) (flonum? form)))
             (cons form (recur (sub-read p))))
            (else (r-error p (format "invalid ~a inside ~avector --" ts ts) form)))))

  (define (sub-read-strsym-char-escape p what)
    (let ((c (read-char p)))
      (if (eof-object? c)
          (r-error p "end of file within a" what))
      (cond ((or (char=? c #\\) (char=? c #\") (char=? c #\|)) c)
            ((char=? c #\a) #\alarm)
            ((char=? c #\b) #\backspace)
            ((char=? c #\t) #\tab)
            ((char=? c #\n) #\newline)
            ;((char=? c #\v) #\vtab)
            ;((char=? c #\f) #\page)
            ((char=? c #\r) #\return)
            ((char=? c #\x) (sub-read-x-char-escape p #t))
            ((and (eq? what 'string) (char-whitespace? c))
             (let loop ((gotnl (char=? c #\newline)) (nc (peek-char p)))
               (cond ((or (eof-object? nc) (not (char-whitespace? nc)))
                      (if gotnl #f (r-error p "no newline in line ending escape")))
                     ((and gotnl (char=? nc #\newline)) #f)
                     (else (read-char p) (loop (or gotnl (char=? nc #\newline)) (peek-char p))))))
            (else (r-error p "invalid char escape in" what ': c)))))

  (define (sub-read-x-char-escape p in-string?)
    (define (rev-digits->char l)
      (if (null? l)
          (r-error p "\\x escape sequence is too short")
          (integer->char (string->fixnum (list->string (reverse! l)) 16))))
    (let loop ((c (peek-char p)) (l '()) (cc 0))
      (cond ((eof-object? c)
             (if in-string?
               (r-error p "end of file within a string")
               (rev-digits->char l)))
            ((and in-string? (char=? c #\;))
             (read-char p)
             (rev-digits->char l))
            ((and (not in-string?) (char-delimiter? c))
             (rev-digits->char l))
            ((not (char-hex-digit? c))
             (r-error p "unexpected char in \\x escape sequence" c))
            ((> cc 2)
             (r-error p "\\x escape sequence is too long"))
            (else
             (read-char p)
             (loop (peek-char p) (cons c l) (+ cc 1))))))
            
  ; body of %read
  (let ((form (sub-read port)))
    (if (not (reader-token? form))
        (if (null? shared) form (patch-shared form))
        (r-error port "unexpected token:" (cdr form))))))

(display "\n--- larger inputs: the printed form reads back unchanged ---\n")

(test #t (round-trips? sexp1))
(test #t (round-trips? sexp1 print-width 40))
(test #t (round-trips? sexp1 print-width 200))
(test #t (round-trips? sexp2))
(test #t (round-trips? sexp2 print-width 90))
(test #t (round-trips? sexp2 print-width 40))

;; and it really is broken across lines
(define (line-count obj . kv)
  (length (lines (let ([p (open-output-string)])
                   (apply pretty-print obj p kv)
                   (get-output-string p)))))

(test #t (> (line-count sexp1) 50))
(test #t (> (line-count sexp2 print-width 90) 50))

;; a narrower width produces more lines than a wider one
(test #t (> (line-count sexp1 print-width 40) (line-count sexp1 print-width 200)))
(test #t (> (line-count sexp2 print-width 40) (line-count sexp2 print-width 200)))

;; print with an indent breaks lines too, laying the same input out as data
(test #t (> (line-count sexp1) 1))
(test #t (> (length (lines (printed sexp1 print-indent 0))) 1))
;; and without one it is a single line
(test 1 (length (lines (printed sexp1))))



(display "\n--- print-cursor: a left column marking where one pair starts ---\n")

;; pretty-print or print with a cursor, returning the output
(define (ppc width obj cursor . kv)
  (let ([p (open-output-string)])
    (apply pretty-print obj p print-width width print-cursor cursor kv)
    (get-output-string p)))
(define (prc obj cursor . kv)
  (let ([p (open-output-string)])
    (apply print obj p print-cursor cursor kv)
    (get-output-string p)))

;; small list and string helpers, (scheme base) only
(define (keep ok? l)
  (cond [(null? l) '()] [(ok? (car l)) (cons (car l) (keep ok? (cdr l)))] [else (keep ok? (cdr l))]))
(define (all? ok? l) (or (null? l) (and (ok? (car l)) (all? ok? (cdr l)))))
(define (some? ok? l) (and (pair? l) (or (ok? (car l)) (some? ok? (cdr l)))))
(define (contains? s sub)
  (let ([n (string-length s)] [m (string-length sub)])
    (let loop ([i 0])
      (and (<= (+ i m) n)
           (or (string=? (substring s i (+ i m)) sub) (loop (+ i 1)))))))
(define (marked-lines str) (keep (lambda (l) (char=? (string-ref l 0) #\>)) (lines str)))
(define (count-char c s)
  (let loop ([i 0] [k 0])
    (if (= i (string-length s)) k (loop (+ i 1) (if (char=? (string-ref s i) c) (+ k 1) k)))))
(define (unbracket s)
  (let ([t (string-copy s)])
    (do ([i 0 (+ i 1)]) [(= i (string-length t)) t]
      (case (string-ref t i)
        [(#\[) (string-set! t i #\()]
        [(#\]) (string-set! t i #\))]
        [else #f]))))

(test #f (print-cursor))

;; single-line output has no column: brackets are the only mark there, and they
;; apply to print as well as pretty-print
(let ([l (datum "(a (b c) d)")])
  (test "(a (b c) d)" (prc l (cadr l)))                         ; brackets off: print's default
  (test "(a [b c] d)" (prc l (cadr l) print-brackets #t))
  (test "[a (b c) d]" (prc l l print-brackets #t))
  (test "(a (b c) d)" (prc l (list 'b 'c) print-brackets #t))   ; equal, not the same pair
  (test "(a (b c) d)" (prc l (list 1) print-brackets #t)))
;; without a cursor, print still takes no notice of print-brackets
(test "(let ((x 1)) x)" (printed (datum "(let ((x 1)) x)") print-brackets #t))
;; pretty-print kept on one line
(let ([lt (datum "(let ((x 1) (y 2)) (display x) (+ x y))")])
  (test "(let ((x 1) (y 2)) (display x) [+ x y])" (ppc 80 lt (cadddr lt) print-indent #f)))
;; print laid out over lines gets both
(let ([l (datum "(a (b c) d)")])
  (test " (list\n> [a (b c) d]\n> [a (b c) d])\n"
    (prc (list 'list l l) l print-indent 0 print-width 12 print-brackets #t)))
;; a cycle inside the cursor, on one line
(let ([cyc (list 'q 'r)])
  (set-cdr! (cdr cyc) cyc)
  (let ([anc (list 'wrap cyc)])
    (test "(f [wrap #0=(q r . #0#)])" (prc (list 'f anc) anc print-brackets #t))))

;; pretty-print brackets the cursor form, and only it, in place of its parentheses
(define fact-code (datum "(define (fact n) (if (< n 2) 1 (* n (fact (- n 1)))))"))
(test " (define (fact n)\n   (if (< n 2)\n       1\n       (* n\n>         [fact (- n\n                   1)])))\n"
  (ppc 20 fact-code (caddr (cadddr (caddr fact-code)))))
;; without brackets the column alone marks it
(test " (define (fact n)\n   (if (< n 2)\n       1\n       (* n\n>         (fact (- n\n                   1)))))\n"
  (ppc 20 fact-code (caddr (cadddr (caddr fact-code))) print-brackets #f))
;; laid out over lines, the column stays even when the form fits on one:
;; it follows the mode, not the line count
(test ">(define (fact n) (if (< n 2) 1 [* n (fact (- n 1))]))\n"
  (ppc 80 fact-code (cadddr (caddr fact-code))))
;; a cursor that is not in the datum marks nothing and brackets nothing
(test " (define (fact n) (if (< n 2) 1 (* n (fact (- n 1)))))\n"
  (ppc 80 fact-code (list 'fact)))

;; with a cursor, binding lists lose their brackets; without one they keep them
(let ([lt (datum "(let ((x 1) (y 2)) (display x) (+ x y))")])
  (test "(let ([x 1] [y 2]) (display x) (+ x y))\n" (pp 80 "(let ((x 1) (y 2)) (display x) (+ x y))"))
  (test ">(let ((x 1) (y 2)) (display x) [+ x y])\n" (ppc 80 lt (cadddr lt)))
  (test ">(let ((x 1) [y 2]) (display x) (+ x y))\n" (ppc 80 lt (cadr (cadr lt)))))

;; printed twice: both occurrences are marked and bracketed
(let* ([g (list 'g 'x)] [top (list 'list g (list 'something 'else 'here) g)])
  (test ">(list [g x]\n       (something else\n         here)\n>      [g x])\n"
    (ppc 22 top g))
  ;; under print-graph only the labelled occurrence has parentheses to replace
  (test ">(list #0=[g x]\n       (something else\n         here)\n       #0#)\n"
    (ppc 22 top g print-graph #t))
  ;; with a base indent the column sits at that indent on every line
  (test " (list\n    > (g x)\n      (something\n       else\n       here)\n    > (g x))\n"
    (prc top g print-indent 4 print-width 24)))

;; the cursor contains a cycle, so the printer works from a marked copy of it
(let* ([cyc (list 'q 'r)])
  (set-cdr! (cdr cyc) cyc)
  (let* ([anc (list 'wrap cyc 'tail)]
         [top (list 'begin anc (list 'other 'stuff))]
         [marked (marked-lines (ppc 18 top anc))])
    (test 1 (length marked))
    (test #t (and (pair? marked) (contains? (car marked) "[wrap")))))

;; a cut-off shell has parentheses; a pair hidden inside a cut has none
(let ([deep (datum "(a (b (c (d (e f)))))")])
  (test ">(a (b [...]))\n" (ppc 80 deep (cadr (cadr deep)) print-level 2))
  (test " (a (b (...)))\n" (ppc 80 deep (cadr (cadr (cadr deep))) print-level 2)))

;; a list laid out by a style pattern of its own: guard's (var clause ...)
(define guard-code (datum "(guard (e [(string? e) (display e)] [else (raise e)]) (risky 1) (risky 2))"))
(test " (guard\n>  [e ((string? e) (display e))\n      (else (raise e))]\n   (risky 1)\n   (risky 2))\n"
  (ppc 30 guard-code (cadr guard-code)))

;; clauses, quoted data and abbreviations
(let ([cnd (datum "(cond [(null? x) 'empty] [(pair? x) (car x)] [else 'other])")])
  (test " (cond ((null? x)\n        'empty)\n>      [(pair? x)\n        (car x)]\n       (else\n        'other))\n"
    (ppc 20 cnd (caddr cnd))))
(let ([qd (datum "(f '(a b) c)")])
  (test ">(f '[a b] c)\n" (ppc 80 qd (cadr (cadr qd)))))
;; a quote form prints as 'x, with no parentheses to replace
(let ([qt (list 'f (list 'quote 'x) 'y)])
  (test ">(f 'x y)\n" (ppc 80 qt (cadr qt))))

;; anything but #f or a pair is refused
(test-error (parameterize ([print-cursor 'sym]) 'unreachable))
(test-error (parameterize ([print-cursor 42]) 'unreachable))
(test-error (parameterize ([print-cursor (vector 1)]) 'unreachable))
(test-error (prc '(a) #t))

;; The oracle, over sub-forms of a datum pretty-printed with a cursor on each:
;; - stripped of the column, and with its brackets turned back into parentheses,
;;   the output is exactly what printing without a cursor or brackets gives, so
;;   nothing moves;
;; - the column holds only > and space, and the form is marked at least once;
;; - unless the form prints abbreviated, the lines holding a [ are exactly the
;;   marked lines, [ and ] balance, and a marked line holds [ and the head symbol.
(define (subforms x)
  (let walk ([x x] [acc '()])
    (if (pair? x)
        (let loop ([l x] [acc (cons x acc)])
          (if (pair? l) (loop (cdr l) (walk (car l) acc)) acc))
        acc)))
;; every step-th sub-form: each check re-prints the whole datum, so checking
;; all of a large one is quadratic for little extra coverage
(define (every-nth step l)
  (let loop ([l l] [i 0] [acc '()])
    (cond [(null? l) (reverse acc)]
          [(= 0 (remainder i step)) (loop (cdr l) (+ i 1) (cons (car l) acc))]
          [else (loop (cdr l) (+ i 1) acc)])))
(define (cursor-faults obj width step)
  (define plain (lines (ppc width obj #f print-brackets #f)))
  (define (fault? f)
    (let* ([out (ppc width obj f)] [ls (lines out)]
           [body (map (lambda (l) (substring l 1 (string-length l))) ls)]
           [abbrev? (memq (car f) '(quote quasiquote unquote unquote-splicing))]
           [head (and (symbol? (car f)) (not abbrev?) (symbol->string (car f)))]
           [marks (map (lambda (l) (char=? (string-ref l 0) #\>)) ls)]
           [brackets (map (lambda (b) (contains? b "[")) body)])
      (not (and (equal? (map unbracket body) plain)
                (all? (lambda (l) (memv (string-ref l 0) '(#\space #\>))) ls)
                (some? (lambda (m) m) marks)
                (or abbrev?
                    (and (equal? marks brackets)
                         (= (count-char #\[ out) (count-char #\] out))
                         (or (not head)
                             (some? (lambda (l) (contains? l (string-append "[" head)))
                                    (marked-lines out)))))))))
  (keep fault? (every-nth step (subforms obj))))
(test '() (cursor-faults fact-code 80 1))
(test '() (cursor-faults fact-code 20 1))
(test '() (cursor-faults guard-code 80 1))
(test '() (cursor-faults guard-code 18 1))
(test '() (cursor-faults sexp1 30 7))

(display "\n--- print hooks ---\n")

;; a record type the printer knows nothing about, for the hooks to print
(define-record-type <pt> (make-pt x y) pt? (x pt-x) (y pt-y))

(define (printed obj . kv)
  (let ([p (open-output-string)])
    (apply print obj p kv)
    (get-output-string p)))

;; an atom hook: the object is printed whole, by the hook's writer
(define pt-atom
  (add-print-hook (print-hooks) pt?
    (atom-print-hook #f (lambda (x) 5) (lambda (x p) (write-string "#<pt>" p)))))
(test "(#<pt> 1)" (printed (list (make-pt 1 2) 1) print-hooks pt-atom))
(test "(#<pt> 1)" (parameterize ([print-hooks pt-atom]) (printed (list (make-pt 1 2) 1))))

;; a list hook: the printer lays out the contents, so parameters apply to them
(define pt-list
  (add-print-hook (print-hooks) pt?
    (glist-print-hook "#pt(" (lambda (x) (list (pt-x x) (pt-y x)))
                      (lambda (l) (make-pt (car l) (cadr l))) ")")))
(test "#pt(1 (a b))" (printed (make-pt 1 '(a b)) print-hooks pt-list))
(test "#pt(#xff #x2)" (printed (make-pt 255 2) print-hooks pt-list print-radix 16))
(test "#pt(1 ...)" (printed (make-pt 1 2) print-hooks pt-list print-length 1))
(test "#pt(#0=(a) #0#)"
      (let ([l (list 'a)]) (printed (make-pt l l) print-hooks pt-list print-graph #t)))

;; a read-macro hook: a prefix and the one datum under it
(define pt-rmac
  (add-print-hook (print-hooks) pt?
    (rmac-print-hook "#!" pt-x (lambda (x) (make-pt x #f)))))
(test "#!(1 2)" (printed (make-pt '(1 2) #f) print-hooks pt-rmac))

;; a binary-vector hook: a prefix, a length and an element reader
(define pt-bvec
  (add-print-hook (print-hooks) pt?
    (bvec-print-hook "#p(" (lambda (x) 2)
                     (lambda (x i) (if (= i 0) (pt-x x) (pt-y x))) ")")))
(test "#p(#b1 #b10)" (printed (make-pt 1 2) print-hooks pt-bvec print-radix 2))

;; with no hook given, the predicate's own true value is the hook
(define pt-by-pred
  (add-print-hook (print-hooks)
    (lambda (x)
      (and (pt? x)
           (atom-print-hook #f (lambda (x) 4)
             (lambda (x p) (write-string (if (pt-x x) "#<t>" "#<f>") p)))))))
(test "(#<t> #<f>)" (printed (list (make-pt 1 2) (make-pt #f 2)) print-hooks pt-by-pred))

;; a new entry goes ahead of the ones already there; one for a predicate already
;; there takes that entry's place, so it stays behind entries added after it;
;; and the hooks added to are left as they were
(define (pt-named s)
  (atom-print-hook #f (lambda (x) (string-length s)) (lambda (x p) (write-string s p))))
(define (pt-too? x) (pt? x))
(define pt-both (add-print-hook pt-atom pt-too? (pt-named "#<too>")))
(test "#<too>" (printed (make-pt 1 2) print-hooks pt-both))
(test "#<too>" (printed (make-pt 1 2) print-hooks (add-print-hook pt-both pt? (pt-named "#<new>"))))
(test "#<new>" (printed (make-pt 1 2) print-hooks (add-print-hook pt-atom pt? (pt-named "#<new>"))))
(test "#<pt>" (printed (make-pt 1 2) print-hooks pt-atom))

;; outside a print call the hooks are not in force
(test-assert (not (string=? (printed (make-pt 1 2)) "#<pt>")))

;; print-hooks takes what add-print-hook makes

;; ---------------------------------------------------------------------------
;; Homogeneous numerical vectors
;; ---------------------------------------------------------------------------

;; every type skint has is laid out by the printer rather than left to write,
;; so each of them breaks across lines and each of them reads back

(define (nv type n)
  (let ([v (make-numvector type n)])
    (when (< type 10) ; only the integer types take an exact value
      (let loop ([i 0]) (when (< i n) (numvector-set! v i 1) (loop (+ i 1)))))
    v))

(define (reads-back? v)
  (equal? v (read (open-input-string (printed v)))))

(test "#u8(1 1 1)" (printed (nv 0 3)))
(test "#s8(1 1 1)" (printed (nv 1 3)))
(test "#u16(1 1 1)" (printed (nv 2 3)))
(test "#s16(1 1 1)" (printed (nv 3 3)))
(test "#u32(1 1 1)" (printed (nv 4 3)))
(test "#s32(1 1 1)" (printed (nv 5 3)))
(test "#u64(1 1 1)" (printed (nv 6 3)))
(test "#s64(1 1 1)" (printed (nv 7 3)))
(test "#f32(0.0 0.0)" (printed (nv 10 2)))
(test "#f64(0.0 0.0)" (printed (nv 11 2)))
(test "#c64(0.0+0.0i 0.0+0.0i)" (printed (nv 14 2)))
(test "#c128(0.0+0.0i 0.0+0.0i)" (printed (nv 15 2)))

(test-assert (reads-back? (nv 4 3)))
(test-assert (reads-back? (nv 7 3)))
(test-assert (reads-back? (nv 10 3)))
(test-assert (reads-back? (nv 14 3)))
(test-assert (reads-back? (nv 15 3)))

;; with an indent they break at the width, which is what the hook buys over
;; leaving the vector to write
(test "#u32(1 1 1 1\n     1 1)\n"
  (printed (nv 4 6) print-indent 0 print-width 12))
(test "#c128(0.0+0.0i\n      0.0+0.0i\n      0.0+0.0i)\n"
  (printed (nv 15 3) print-indent 0 print-width 20))

;; NOTE: for f32, c64 and c128 the printer does NOT agree with write, and it
;; is write that is wrong: it drops the decimal point, so that #f32(1.5 2) and
;; #c64(1.5+0i 2+0i) come out of write and cannot be read back at all.  f64 is
;; the only inexact type write gets right.  The printer's output reads back for
;; every type, which is what the oracle above asks of it.
(test-error (print 1 (open-output-string) print-hooks 5))

(display "\n--- All tests complete. ---\n")

(test-end)
