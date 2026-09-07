(import (skint format))

(include "test.scm")

;; ---------------------------------------------------------------------------
;; Helpers
;; ---------------------------------------------------------------------------

;; format to a string, the common case in these tests
(define (f fs . args) (apply format #f fs args))

;; capture what a procedure writes to a port it is handed
(define (to-string proc)
  (let ([p (open-output-string)])
    (proc p)
    (get-output-string p)))


(display "\n--- format destinations ---\n")

(test "#f destination returns a string"
  "plain and \"written\""
  (format #f "~a and ~s" "plain" "written"))

(test "string first argument is the format string"
  "[42]"
  (format "[~a]" 42))

(test "port destination writes to the port"
  "hi!"
  (to-string (lambda (p) (format p "~a!" 'hi))))

(test "#t destination writes to the current output port"
  "x"
  (to-string (lambda (p) (parameterize ([current-output-port p]) (format #t "x")))))

(test "empty format string yields empty string"
  ""
  (f ""))

(test "text with no directives passes through"
  "just text"
  (f "just text"))


(display "\n--- fprintf ---\n")

(test "fprintf writes to its port"
  "to port: 42"
  (to-string (lambda (p) (fprintf p "to port: ~a" 42))))


(display "\n--- printf ---\n")

(test "printf writes to the current output port"
  "hello 42\n"
  (to-string (lambda (p) (parameterize ([current-output-port p])
                           (printf "hello ~a~%" 42)))))

(test "printf works with no arguments beyond the format string"
  "plain\n"
  (to-string (lambda (p) (parameterize ([current-output-port p])
                           (printf "plain~%")))))

(test "printf agrees with format #t"
  (to-string (lambda (p) (parameterize ([current-output-port p]) (format #t "~a-~s" 1 "x"))))
  (to-string (lambda (p) (parameterize ([current-output-port p]) (printf "~a-~s" 1 "x")))))


(display "\n--- literal text and whitespace ---\n")

(test "tilde escape"      "~"     (f "~~"))
(test "newline"           "a\nb"  (f "a~%b"))
(test "tab"               "a\tb"  (f "a~tb"))
(test "space"             "a b"   (f "a~_b"))
(test "fresh line"        "a\nb"  (f "a~&b"))


(display "\n--- objects ---\n")

(test "~a displays"       "hi"                (f "~a" "hi"))
(test "~s writes"         "\"hi\""            (f "~s" "hi"))
(test "~a on a list"      "(1 x y)"           (f "~a" '(1 "x" #\y)))
(test "~s on a list"      "(1 \"x\" #\\y)"    (f "~s" '(1 "x" #\y)))
(test "~w writes shared"  "(1 2)"             (f "~w" '(1 2)))
(test "~c writes a char"  "Z"                 (f "~c" #\Z))
(test "~y defaults to write" "(a \"b\")"      (f "~y" '(a "b")))

(test "directive characters are case-insensitive"
  (f "~a ~s" "x" "x")
  (f "~A ~S" "x" "x"))


(display "\n--- integers by radix ---\n")

(test "~b binary"          "1010"  (f "~b" 10))
(test "~o octal"           "100"   (f "~o" 64))
(test "~d decimal"         "255"   (f "~d" 255))
(test "~x hexadecimal"     "ff"    (f "~x" 255))
(test "~x negative"        "-ff"   (f "~x" -255))

(display "\n--- flonums through radix directives ---\n")

(test "~b on a flonum"   "0.1"     (f "~b" 0.5))
(test "~x on a flonum"   "0.8"     (f "~x" 0.5))
(test "~o on a flonum"   "100.0"   (f "~o" 64.0))
(test "~x uses uppercase digits" "3.23D70A3D70A3E" (f "~x" 3.14))

(test "~x output round-trips through string->number"
  3.14
  (string->number (f "~x" 3.14) 16))

(test "~b output round-trips through string->number"
  3.14
  (string->number (f "~b" 3.14) 2))

(test "~b exponent marker is e, digits in binary"
  "1.100100111110010110010011100110100000100011001110101e1100011"
  (f "~b" 1e30))

(test "~o exponent marker is e, digits in octal"
  "1.44762623464043165e41"
  (f "~o" 1e30))

(test "~x exponent marker is C99 p, digits in decimal"
  "C.9F2C9CD04675p96"
  (f "~x" 1e30))

;; the ~b exponent is binary: 1100011 = 99, i.e. mantissa x 2^99
(test "~b exponent reads as binary" 99 (string->number "1100011" 2))
;; the ~o exponent is octal: 41 = 33, i.e. mantissa x 8^33
(test "~o exponent reads as octal"  33 (string->number "41" 8))

(test "infinities pass through every radix"
  '("+inf.0" "+inf.0" "+inf.0" "+inf.0")
  (list (f "~b" (/ 1.0 0.0)) (f "~o" (/ 1.0 0.0))
        (f "~d" (/ 1.0 0.0)) (f "~x" (/ 1.0 0.0))))

(test "NaN passes through"          "+nan.0" (f "~x" (/ 0.0 0.0)))
(test "negative zero is preserved"  "-0.0"   (f "~o" -0.0))


(display "\n--- inexact numbers, width and precision ---\n")

(test "~f bare"                "3.14159"     (f "~f" 3.14159))
(test "~f precision only"      "3.14"        (f "~,2f" 3.14159))
(test "~f width and precision" "    3.14"    (f "~8,2f" 3.14159))
(test "~e width and precision" "   1.235e3"  (f "~10,3e" 1234.5678))
(test "~g width and precision" "    1.23e3"  (f "~10,3g" 1234.5678))
(test "~F accepts a string, as SRFI 48 specifies" "notanum" (f "~f" "notanum"))

(test "width is a minimum, not a truncation"
  "3.14159"
  (f "~2f" 3.14159))

;; with no precision the number is printed by number->string: no coercion,
;; no exponential form, and the three directives agree
(test "~f without a precision leaves an exact argument exact" "42" (f "~f" 42))
(test "~e without a precision leaves an exact argument exact" "42" (f "~e" 42))
(test "~g without a precision leaves an exact argument exact" "42" (f "~g" 42))

(test "~e without a precision does not use exponential form"
  "1234.5678"
  (f "~e" 1234.5678))

(test "the three directives agree when no precision is given"
  '("1234.5678" "1234.5678" "1234.5678")
  (list (f "~f" 1234.5678) (f "~e" 1234.5678) (f "~g" 1234.5678)))

(test "width still applies without a precision"
  "   1234.5678"
  (f "~12e" 1234.5678))

(test "a precision selects the style"
  '("  1234.568" "   1.235e3" "    1.23e3")
  (list (f "~10,3f" 1234.5678) (f "~10,3e" 1234.5678) (f "~10,3g" 1234.5678)))


(display "\n--- control directives ---\n")

(test "~? indirection"      "[1-2]"  (f "~?" "[~a-~a]" '(1 2)))
(test "~k is the same as ~?" (f "~?" "[~a-~a]" '(1 2)) (f "~k" "[~a-~a]" '(1 2)))
(test "~* skips one argument"    "13"  (f "~a~*~a" 1 2 3))
(test "~N* skips N arguments"    "14"  (f "~a~2*~a" 1 2 3 4))
(test "~0@* rewinds to the first" "121" (f "~a~a~0@*~a" 1 2))
(test "~1@* jumps to the second"  "122" (f "~a~a~1@*~a" 1 2))

(test "~h inserts the help string"
  (format-help-string)
  (f "~h"))


(display "\n--- malformed format strings ---\n")

;; these are errors; how they are reported is not specified, so only check that
;; the malformed string is rejected rather than quietly producing output

(test-error "a directive with no argument left"
  (f "~a"))

(test-error "a format string ending in a bare tilde"
  (f "abc~"))

(test-error "an unrecognized directive character"
  (f "~q" 1))

(test-error "an unrecognized escape is not passed through silently"
  (f (string #\a #\~ #\n #\b)))

(test-error "a ~* index outside the argument list"
  (f "~9*" 1))


(display "\n--- parameters ---\n")

(test "format-pretty-print controls ~y"
  "(a b)"
  (parameterize ([format-pretty-print display]) (f "~y" '(a "b"))))

(test "format-fresh-line controls ~&"
  "xy"
  (parameterize ([format-fresh-line (lambda (p) #f)]) (f "x~&y")))

(test "format-fixed-print controls ~f"
  "<F>"
  (parameterize ([format-fixed-print (lambda (arg wd dd p) (display "<F>" p))])
    (f "~f" 1.5)))

(test "format-exponential-print controls ~e"
  "<E>"
  (parameterize ([format-exponential-print (lambda (arg wd dd p) (display "<E>" p))])
    (f "~e" 1.5)))

(test "format-general-print controls ~g"
  "<G>"
  (parameterize ([format-general-print (lambda (arg wd dd p) (display "<G>" p))])
    (f "~g" 1.5)))

(test "format-help-string controls ~h"
  "my help"
  (parameterize ([format-help-string "my help"]) (f "~h")))

(test "numeric formatters receive -1 for an unspecified width"
  '(-1 -1)
  (let ([seen #f])
    (parameterize ([format-fixed-print (lambda (arg wd dd p) (set! seen (list wd dd)))])
      (f "~f" 1.5))
    seen))

(test "numeric formatters receive the parsed width and precision"
  '(8 2)
  (let ([seen #f])
    (parameterize ([format-fixed-print (lambda (arg wd dd p) (set! seen (list wd dd)))])
      (f "~8,2f" 1.5))
    seen))

(test "parameters are restored after the dynamic extent"
  "(a \"b\")"
  (begin
    (parameterize ([format-pretty-print display]) (f "~y" '(a "b")))
    (f "~y" '(a "b"))))


(display "\n--- SRFI 48 conformance ---\n")

;; worked examples from the SRFI 48 specification that SKINT reproduces as given
(test "spec: ~8,2F on an exact integer"  "   32.00"   (f "~8,2F" 32))
(test "spec: ~1,2F overflows the width"  "4321.00"    (f "~1,2F" 4321))
(test "spec: ~8,3F rounds"               " 123.346"   (f "~8,3F" 123.3456))
(test "spec: ~6,3F fills the width"      "123.346"    (f "~6,3F" 123.3456))
(test "spec: ~2,3F overflows the width"  "123.346"    (f "~2,3F" 123.3456))
(test "spec: ~8,3F pads a string"        "     foo"   (f "~8,3F" "foo"))
(test "spec: mixed ~a and ~s"            "this is a \"test\""
  (f "~a ~s ~a ~s" 'this 'is "a" "test"))
(test "spec: ~? indirection"             "a new test" (f "~a ~? ~a" 'a "~s" '(new) 'test))
(test "spec: radix directives"           "#d32 #x20 #o40 #b100000"
  (f "#d~d #x~x #o~o #b~b" 32 32 32 32))

(test "spec: ~wF without a precision leaves the number exact" "    32" (f "~6F" 32))
(test "spec: ~4F on an exact integer"                        "  12"   (f "~4F" 12))

(test "~& does not track columns"
  "\n1\n\n2\n\n\n3\n"           ; SRFI 48 specifies "\n1\n2\n3\n"
  (f "~&1~&~&2~&~&~&3~%"))

(test-error "too few arguments is still an error"
  (f "~a ~a" 1))

;; SRFI 48 specifies ~wF for strings and numbers; both take the width
(test "~wF pads numbers"   "      42"  (f "~8f" 42))
(test "~wF pads strings"   "     foo"  (f "~8f" "foo"))


(display "\n--- All tests complete. ---\n")

(test-end)
