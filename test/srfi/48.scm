(import (srfi 48))
(include "test.scm")

;;[esl]: many ~F tests here had to be changed because Skint switches to exponential representation
;; when absolute value hits ~1e15; in exponential representation, the specified "precision"/"digits" 
;; param is ignored (strictly speaking, this is no longer a "fixed" format, as defined in CL). 
;; In any case, if width is larger then the printed length, left padding is added.


(test-begin "srfi-48")

;; tests, based on the original Ken Dickey's examples
;;

(test-equal "    0.33" (format "~8,2F" (/ 1 3)))

(test-equal "    32" (format "~6F" 32)) ; CL: "  32.0"

(test-equal "   32.00" (format "~8,2F" 32))

(test-equal "4321.00" (format "~1,2F" 4321))

(test-equal "3200000.0" (format "~8F" 32e5)) ; "   3.2e6" is also valid!

(test-equal "Hello, World!" (format "Hello, ~a" "World!"))

(test-equal "Error, list is too short: (one \"two\" 3)" (format "Error, list is too short: ~s" (quote (one "two" 3))))

(test-equal "test me" (format "test me"))

(test-equal "this is a \"test\"" (format "~a ~s ~a ~s" (quote this) (quote is) "a" "test"))

(test-equal "#d32 #x20 #o40 #b100000\n" (format "#d~d #x~x #o~o #b~b~%" 32 32 32 32))

(test-equal "a new test" (format "~a ~? ~a" (quote a) "~s" (quote (new)) (quote test)))

(test-equal "3  2 2  3 \n" (format #f "~a ~? ~a ~%" 3 " ~s ~s " (quote (2 2)) 3))

(test-equal "#0=(a b c . #0#)" (format "~w" (let ( (c '(a b c)) ) (set-cdr! (cddr c) c) c)))

(test-equal "   32.00" (format "~8,2F" 32))

(test-equal "   1.949" (format "~8,3F" (sqrt 3.8)))

(test-equal "345670000000.00" (format "~8,2F" 3.4567e11))

(test-equal " 0.333" (format "~6,3F" (/ 1 3)))

(test-equal "  12" (format "~4F" 12)) ; CL: "12.0"

(test-equal " 123.346" (format "~8,3F" 123.3456))

(test-equal "123.346" (format "~6,3F" 123.3456))

(test-equal "123.346" (format "~2,3F" 123.3456))


;; extra tests

(test-equal "     foo"   (format "~8,3F" "foo"))

(test-equal "  12.346" (format "~8,3F" 12.3456))

(test-equal "123.346" (format "~6,3F" 123.3456))

(test-equal "123.346" (format "~4,3F" 123.3456))

(test-equal "6.38" (format "~4,2f" 6.375)) ; could be "6.37"

;; adapted from CLtL
;[esl]: CL uses w to round, we don't!

(define (foo x) (format "~6,2F|~6F|~,2F|~F" x x x x))

(test-equal "  3.14|3.14159|3.14|3.14159" (foo 3.14159)) ; CL: "  3.14|3.1416|3.14|3.14159"

(test-equal " -3.14|-3.14159|-3.14|-3.14159" (foo -3.14159)) ; CL: " -3.14|-3.142|-3.14|-3.14159"

(test-equal "100.00| 100.0|100.00|100.0" (foo 100.0))

(test-equal "1234.00|1234.0|1234.00|1234.0" (foo 1234.0))

(test-equal "  0.01| 0.006|0.01|0.006" (foo 0.006))

;;; tests, adapted from
;;; SPDX-FileCopyrightText: 2017 Hamayama <hamay1010@gmail.com>
;;; SPDX-FileCopyrightText: 2025  Retropikzel <retropikzel@iki.fi>
;;;
;;; SPDX-License-Identifier: MIT

(define pi 3.141592653589793)

(test-equal (format "test ~s" 'me) (format #f "test ~a" "me"))

(test-equal "    32" (format "~6F" 32)) ;CL: "  32.0"

(test-equal "  32.0" (format "~6F" 32.0))

(test-equal "  3.2e46" (format "~8F" 3.2e46)) ;[esl]: too big for ~f 

(test-equal " 3.2e-44" (format "~8F" 3.2e-44)) ;[esl]: SIC! precision not specified, CL: "     0.0"

(test-equal "  3.2e21" (format "~8F" 3.2e21)) ;[esl]: too big for ~f

(test-equal "  3200.0" (format "~8F" 3200.0))

(test-equal "320000000000.00" (format "~8,2F" 3.2e11))

(test-equal "      1.2345" (format "~12F" 1.2345))

(test-equal "        1.23" (format "~12,2F" 1.2345))

(test-equal "       1.234" (format "~12,3F" 1.2345))

(test-equal "345670000000.00" (format "~8,2F" 3.4567e11))

(test-equal "\n" (format "~A~A" (list->string (list #\newline)) ""))

(test-equal "a new test" (format "~a ~? ~a" (quote a) "~s" (quote (new)) (quote test)))

(test-equal "a new test, yes!" (format "~a ~?, ~a!" (quote a) "~s ~a" (quote (new test)) (quote yes)))

;[esl]> extra tests to show switch to exponential representation on big numbers

(test-equal "3456700000000000.00" (format "~8,2F" 3.4567e15)) ;[esl]: no #; CL: "3456700000000000.0#"
(test-equal "3.4567e16" (format "~8,2F" 3.4567e16)) ;[esl]: too big for ~f; CL: "34567000000000000.##"
(test-equal "3.4567e17" (format "~8,2F" 3.4567e17)) ;[esl]: too big for ~f; CL: "34567000000000000#.##"
(test-equal "3.4567e18" (format "~8,2F" 3.4567e18)) ;[esl]: too big for ~f; CL: "34567000000000000##.##"
(test-equal "3.4567e19" (format "~8,2F" 3.4567e19)) ;[esl]: too big for ~f; CL: "34567000000000000###.##"

;[esl]< back to regular 

(test-equal "3.4567e20" (format "~8,2F" 3.4567e20)) ;[esl]: too big for ~f; CL: "34567000000000000####.##"

(test-equal "3.4567e21" (format "~8,2F" 3.4567e21)) ;[esl]: too big for ~f; CL: "34567000000000000#####.##"

(test-equal "3.4567e22" (format "~8,2F" 3.4567e22)) ;[esl]: too big for ~f; CL: "34567000000000000######.##"

(test-equal "3.4567e23" (format "~8,2F" 3.4567e23)) ;[esl]: too big for ~f; CL: "34567000000000000#######.##"

(test-equal "3.4567e24" (format "~8,0F" 3.4567e24)) ;[esl]: too big for ~f; CL: "34567000000000000########."

(test-equal "3.4567e24" (format "~8,1F" 3.4567e24)) ;[esl]: too big for ~f; CL: "34567000000000000########.#"

(test-equal "3.4567e24" (format "~8,2F" 3.4567e24)) ;[esl]: too big for ~f; CL: "34567000000000000########.##"

(test-equal "3.4567e24" (format "~8,3F" 3.4567e24)) ;[esl]: too big for ~f; CL:  "34567000000000000########.###"

(test-equal "3.5567e24" (format "~8,0F" 3.5567e24)) ;[esl]: too big for ~f; CL: "35567000000000000########."

(test-equal "3.5567e24" (format "~8,1F" 3.5567e24)) ;[esl]: too big for ~f; CL: "35567000000000000########.#"

(test-equal "3.5567e24" (format "~8,2F" 3.5567e24)) ;[esl]: too big for ~f; CL: "35567000000000000########.##"

(test-equal "       -0." (format "~10,0F" -3e-4))

(test-equal "      -0.0" (format "~10,1F" -3e-4))

(test-equal "     -0.00" (format "~10,2F" -3e-4))

(test-equal "    -0.000" (format "~10,3F" -3e-4))

(test-equal "   -0.0003" (format "~10,4F" -3e-4))

(test-equal "  -0.00030" (format "~10,5F" -3e-4))

(test-equal "     1.020" (format "~10,3F" 1.02))

(test-equal "     1.025" (format "~10,3F" 1.025))

(test-equal "     1.026" (format "~10,3F" 1.0256))

(test-equal "     1.002" (format "~10,3F" 1.002))

(test-equal "     1.002" (format "~10,3F" 1.0025))

(test-equal "     1.003" (format "~10,3F" 1.00256))

(test-equal "    0.33" (format "~8,2F" (/ 1 3)))

(test-equal "    32" (format "~6F" 32))

(test-equal "   32.00" (format "~8,2F" 32))

(test-equal "4321.00" (format "~1,2F" 4321))

(test-equal "3200000.0" (format "~8F" 3200000.0)) ;[esl] w is not used for formatting; CL:  "3200000."

(test-equal "Hello, World!" (format "Hello, ~a" "World!"))

(test-equal "Error, list is too short: (one \"two\" 3)" (format "Error, list is too short: ~s" (quote (one "two" 3))))

(test-equal "test me" (format "test me"))

(test-equal "this is a \"test\"" (format "~a ~s ~a ~s" (quote this) (quote is) "a" "test"))

(test-equal "#d32 #x20 #o40 #b100000" (format #f "#d~d #x~x #o~o #b~b" 32 32 32 32))

(test-equal "a new test" (format "~a ~? ~a" (quote a) "~s" (quote (new)) (quote test)))

(test-equal "3  2 2  3 \n" (format #f "~a ~? ~a ~%" 3 " ~s ~s " (quote (2 2)) 3))

(test-equal "#0=(a b c . #0#)" (format "~w" (let ((c (list (quote a) (quote b) (quote c)))) (set-cdr! (cddr c) c) c)))

(test-equal "   32.00" (format "~8,2F" 32))

(test-equal "345670000000.00" (format "~8,2F" 3.4567e11))

(test-equal " 0.333" (format "~6,3F" (/ 1 3)))

(test-equal "  12" (format "~4F" 12)) ; CL: "12.0"

(test-equal " 123.346" (format "~8,3F" 123.3456))

(test-equal "123.346" (format "~6,3F" 123.3456))

(test-equal "123.346" (format "~2,3F" 123.3456))

(test-equal "0" (format "~F" 0)) ; CL: "0.0"

(test-equal "1" (format "~F" 1)) ; CL: "1.0"

(test-equal "123" (format "~F" 123)) ; CL: "123.0"

(test-equal "0.456" (format "~F" 0.456))

(test-equal "123.456" (format "~F" 123.456))

(test-equal "-1" (format "~F" -1)) ; CL: "-1.0"

(test-equal "-123" (format "~F" -123)) ; CL: "-123.0"

(test-equal "-0.456" (format "~F" -0.456))

(test-equal "-123.456" (format "~F" -123.456))

(test-equal "123" (format "~0F" 123)) ; CL: "123."

(test-equal "123" (format "~1F" 123)) ; CL: "123."

(test-equal "123" (format "~2F" 123)) ; CL: "123."

(test-equal "123" (format "~3F" 123)) ; CL: "123."

(test-equal " 123" (format "~4F" 123)) ; CL: "123."

(test-equal "  123" (format "~5F" 123)) ; CL: "123.0"

(test-equal "-123" (format "~3F" -123)) ; CL: "-123."

(test-equal "-123" (format "~4F" -123)) ; CL: "-123."

(test-equal " -123" (format "~5F" -123)) ; CL: "-123."

(test-equal "  -123" (format "~6F" -123)) ; CL: "-123.0"

(test-equal "123." (format "~1,0F" 123))

(test-equal "123.0" (format "~1,1F" 123))

(test-equal "123.00" (format "~1,2F" 123))

(test-equal "0.12" (format "~1,2F" 0.123)) ; CL: ".12"

(test-equal "0.123" (format "~1,3F" 0.123)) ; CL: ".123"

(test-equal "0.1230" (format "~1,4F" 0.123)) ; CL: ".1230"

(test-equal "-123." (format "~1,0F" -123)) ; CL: "-123."

(test-equal "-123.0" (format "~1,1F" -123)) ; CL: "-123.0"

(test-equal "-123.00" (format "~1,2F" -123)) ; CL: "-123.00"

(test-equal "-0.12" (format "~1,2F" -0.123)) ; CL: "-.12"

(test-equal "-0.123" (format "~1,3F" -0.123)) ; CL: "-.123"

(test-equal "-0.1230" (format "~1,4F" -0.123)) ; CL: "-.1230"

(test-equal "123." (format "~1,0F" 123.456))

(test-equal "123.5" (format "~1,1F" 123.456))

(test-equal "123.46"     (format "~1,2F"   123.456))

(test-equal "-123." (format "~1,0F" -123.456))

(test-equal "-123.5" (format "~1,1F" -123.456))

(test-equal "-123.46" (format "~1,2F" -123.456))

(test-assert (member (format "~1,1F" 123.05) '("123.0" "123.1"))) ; both variants allowed

(test-equal "123.2" (format "~1,1F" 123.15))

(test-equal "124.0" (format "~1,1F" 123.95))

(test-assert (member (format "~1,1F" -123.05) '("-123.0" "-123.1"))) ; both variants allowed

(test-equal "-123.2" (format "~1,1F" -123.15))

(test-equal "-124.0" (format "~1,1F" -123.95))

(test-equal "1000.00" (format "~1,2F" 999.995))

(test-equal "-1000.00" (format "~1,2F" -999.995))

(test-equal "1." (format "~1,0F" 1.49))

(test-assert (member (format "~1,0F" 1.5) '("1." "2."))) ; both variants allowed

(test-equal "2." (format "~1,0F" 1.51))

(test-equal "2." (format "~1,0F" 2.49))

(test-assert (member (format "~1,0F" 2.5) '("3." "2."))) ; both variants allowed

(test-equal "3." (format "~1,0F" 2.51))

(test-equal "+inf.0" (format "~F" +inf.0))

(test-equal "-inf.0" (format "~F" -inf.0))

(test-equal "+nan.0" (format "~F" +nan.0))

(test-equal "0.0" (format "~F" 0.0))

(test-equal "-0.0" (format "~F" -0.0))

(test-equal "+inf.0" (format "~1F" +inf.0))

(test-equal "-inf.0" (format "~1F" -inf.0))

(test-equal "+nan.0" (format "~1F" +nan.0))

(test-equal "0.0" (format "~1F" 0.0)) ; CL: "0."

(test-equal "-0.0" (format "~1F" -0.0)) ; CL: "-0."

(test-equal "+inf.0" (format "~1,0F" +inf.0))

(test-equal "-inf.0" (format "~1,0F" -inf.0))

(test-equal "+nan.0" (format "~1,0F" +nan.0))

(test-equal "0." (format "~1,0F" 0.0)) ; CL: "0."

(test-equal "-0." (format "~1,0F" -0.0)) ; CL: "-0."

(test-equal "+inf.0" (format "~1,1F" +inf.0))

(test-equal "-inf.0" (format "~1,1F" -inf.0))

(test-equal "+nan.0" (format "~1,1F" +nan.0))

(test-equal "0.0" (format "~1,1F" 0.0))

(test-equal "-0.0" (format "~1,1F" -0.0))

(test-equal "31.41592653589793" (format "~F" (* pi 10)))

(test-equal "0.33333" (format "~1,5F" (/ 1 3))) ; CL: ".33333"

(test-equal "-0.33333" (format "~1,5F" (/ -1 3))) ; CL: "-.33333"

(test-equal "0.142857142857" (format "~1,12F" (/ 1 7))) ; CL: ".142857142857"

(test-equal "1.797693e308" (format "~F" 1.797693e308)) ; CL: very long number full of #s

(test-equal "1.797693e308" (format "~1F" 1.797693e308)) ; CL: very long number full of #s

(test-equal "1.797693e308" (format "~1,0F" 1.797693e308)) ;[esl] too big for ~f, so not "2.e308"; CL: very long number full of #s

(test-equal "1.797693e308" (format "~1,1F" 1.797693e308)) ;[esl] too big for ~f, so not "1.8e308"; CL: very long number full of #s

(test-equal "-1.797693e308" (format "~F" -1.797693e308)) ; CL: very long number full of #s

(test-equal "-1.797693e308" (format "~1F" -1.797693e308)) ; CL: very long number full of #s

(test-equal "-1.797693e308" (format "~1,0F" -1.797693e308)) ;[esl] too big for ~f, so not "-2.e308";  CL: very long number full of #s

(test-equal "-1.797693e308" (format "~1,1F" -1.797693e308)) ;[esl] too big for ~f, so not "-1.8e308"; CL: very long number full of #s

(test-equal "2.225074e-308" (format "~F" 2.225074e-308)) ; CL: very long number full of #s

(test-equal "5.01" (format "~1,2F" 5.015))

(test-equal "6.00" (format "~1,2F" 5.999))

(test-equal "123." (format "~1,0F" 123.0))

(test-equal "0.1" (format "~F" 0.1))

(test-equal "1" (format "~1f" 1)) ; CL: "1."

(test-equal "1e100" (format "~1,0F" 1e100)) ;[esl] too big for ~f, so not  "1.e100" ; CL: very long number full of #s

(test-equal "1." (format "~1,0F" 1))

(test-equal "0." (format "~1,0F" 0.1)) ; CL: "."

(test-equal "0.0" (format "~1,1F" 0.01)) ; CL: ".0"

(test-equal "1.23e20" (format "~0,3F" 1.23e20)) ;[esl] too big for ~f, so not "1.230e20"; CL: "123000000000000000###.###"

(test-equal "0.000" (format "~0,3F" 1.23e-20)) ;[esl] not "1.230e-20"; CL: ".000"

(test-equal "3456900000000000.000" (format "~8,3F" 3.4569e15)) ;[esl] no #; CL: "3456900000000000.0##"

(test-equal "   3.457" (format "~8,3F" 3.4569))

(test-equal "3456000000000000.00" (format "~8,2F" 3.456e15)) ;[esl] no #; CL: "3456900000000000.0##"

(test-equal "    3.46" (format "~8,2F" 3.456))

(test-equal "       -0." (format "~10,0F" -3e-4))

(test-equal "      -0.0" (format "~10,1F" -3e-4))

(test-equal "     -0.00" (format "~10,2F" -3e-4))

(test-equal "    -0.000" (format "~10,3F" -3e-4))

(test-equal "   -0.0003" (format "~10,4F" -3e-4))

(test-equal "    0.0000" (format "~10,4F" 3e-5))

(test-equal "     1.020" (format "~10,3F" 1.02))

(test-equal "     1.025" (format "~10,3F" 1.025))

(test-equal "     1.026" (format "~10,3F" 1.0256))

(test-equal "     1.002" (format "~10,3F" 1.002))

(test-equal "     1.002" (format "~10,3F" 1.0025))

(test-equal "     1.003" (format "~10,3F" 1.00256))

(test-equal "1.000012" (format "~8,6F" 1.00001234))

(test-equal "   1.00" (format "~7,2F" 0.997554209949891))

(test-equal "   1.00" (format "~7,2F" 0.99755))

(test-equal "   1.00" (format "~7,2F" 0.9975))

(test-equal "   1.00" (format "~7,2F" 0.997))

(test-equal "   0.99" (format "~7,2F" 0.99))

(test-equal "  18.00" (format "~7,2F" 18.0000000000008))

(test-equal "    -15." (format "~8,0F" -14.99995999999362))

(test-end)
