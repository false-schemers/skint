(import (srfi 54))

(include "test.scm")
(test-begin "srfi-54")

(define (writeln . xs)
  (for-each display xs)
  (newline))

(define-syntax test-cat
  (syntax-rules ()
    ((_ cat res) (test-equal res cat))))     

; Larceny tests for SRFI-54

(test-cat (cat 129.995 -10 2.)
            "130.00    ")

(test-cat (cat 129.995 10 2.)
            "    130.00")

(test-cat (cat 129 2.)
            "#e129.00")

(test-cat (cat 129 -2.)
            "129.00")

(test-cat (cat 129 10 #\* 'octal 'sign)
            "****#o+201")

(test-cat (cat 129 10 #\0 'octal 'sign)
            "#o+0000201")

;(test-cat (cat 10.5 'octal)
;            "#i#o25/2")

;(test-cat (cat 10.5 'octal 'exact)
;            "#o25/2")

;(test-cat (cat 10.5 'octal (list string-upcase))
;            "#I#O25/2")

;(test-cat (cat 10.5 'octal (list string-upcase) '(-4))
;            "25/2")

(test-cat (cat 123000000 'flonum)
            "1.23e+8")

(test-cat (cat 1.23456789e+25 'fixnum)
            "12345678900000000000000000.0")

(test-cat (cat 129.995 10 2. 'sign '("$"))
            "  $+130.00")

(test-cat (cat 129.995 10 2. 'sign '("$" -3))
            "     $+130")

(test-cat (cat 129.995 10 2. '("The number is " "."))
            "The number is 130.00.")

(test-cat (cat "abcdefg" '(3 . 1))
            "abcg")

(test-cat (cat "abcdefg" '(3 1))
            "c")

(test-cat (cat 123456789 'sign '(#\,))
            "+123,456,789")

(test-cat (cat "abcdefg" 'sign '(#\,))
            "abcdefg")

(test-cat (cat "abcdefg" 'sign '(#\: 2))
            "a:bc:de:fg")

(test-cat (cat "abcdefg" 'sign '(#\: -2))
            "ab:cd:ef:g")

(test-cat (cat '(#\a "str" s))
            "(a str s)")

(test-cat (cat '(#\a "str" s) '(-1 -1))
            "a str s")

(test-cat (cat '(#\a "str" s) write)
            "(#\\a \"str\" s)")

(test-cat (let ((p (open-output-string)))
              (cat 'String 10 p)
              (get-output-string p))
            "    String")

(test-output  "    String" (cat 'String 10 #t)) 
;(or (parameterize ((current-output-port (open-output-string)))
;      (equal? (begin (cat 'String 10 #t)
;                     (get-output-string (current-output-port)))
;              "    String"))

(define-record-type :example
  (make-example num str)
  example?
  (num get-num set-num!)
  (str get-str set-str!))

(define (record-writer object string-port)
  (if (example? object)
      (begin (display (get-num object) string-port)
      (display "-" string-port)
      (display (get-str object) string-port))
      (display object string-port)))

(define (record-display object string-port)
  (display (get-num object) string-port)
  (display "-" string-port)
  (display (get-str object) string-port))

(define ex (make-example 123 "string"))

#|
(test-cat (cat ex 20)
            (let ((p (open-output-string)))
              (write ex p)
              (let ((s (get-output-string p)))
                (string-append (make-string (- 20 (string-length s)) #\space)
                               s))))
(or (equal? (cat ex 20)
            (let ((p (open-output-string)))
              (write ex p)
              (let ((s (get-output-string p)))
                (string-append (make-string (- 20 (string-length s)) #\space)
                               s))))
    (fail 'cat27))
|#

(test-cat (cat ex 20 record-writer)
            "          123-string")

(test-cat (cat "str" 20 record-writer)
            "                 str")

(test-output
   (string-append
             "    plus:   12,345,678.90"
             "\n"
             "   minus:     -123,456.79"
             "\n"
             "     net:   12,222,222.11"
             "\n"
             "      ex:         1234-ex"
             "\n"
             "    file:       today.txt"
             "\n")
  (let ((plus 12345678.901)
        (minus -123456.789)
        (ex (make-example 1234 "ex"))
        (file "today.txt"))
    (for-each (lambda (x y)
                (cat x 10 #t)
                (cat y
                     15
                     (if (example? y)
                         record-display
                         display)
                     2. '(#\,) #t)
                (newline))
              (list "plus: " "minus: " "net: " "ex: " "file: ")
              (list plus minus (+ plus minus) ex file))))
 
#|
(test-cat (parameterize ((current-output-port (open-output-string)))
              (let ((plus 12345678.901)
                    (minus -123456.789)
                    (ex (make-example 1234 "ex"))
                    (file "today.txt"))
                (for-each (lambda (x y)
                            (cat x 10 #t)
                            (cat y
                                 15
                                 (if (example? y)
                                     record-display
                                     display)
                                 2. '(#\,) #t)
                            (newline))
                          (list "plus: " "minus: " "net: " "ex: " "file: ")
                          (list plus minus (+ plus minus) ex file))
                (get-output-string (current-output-port))))
            (string-append
             "    plus:   12,345,678.90"
             "\n"
             "   minus:     -123,456.79"
             "\n"
             "     net:   12,222,222.11"
             "\n"
             "      ex:         1234-ex"
             "\n"
             "    file:       today.txt"
             "\n"))
|#

(test-end)
