(import (srfi 130) (srfi 14))

(include "test.scm")

(define (sc str c)
  (string-index->cursor str c))
(define (string-index->index str pred . o)
  (string-cursor->index str (apply string-index str pred o)))
(define (string-index-right->index str pred . o)
  (string-cursor->index str (apply string-index-right str pred o)))
(define char-set:not-letter (char-set-complement char-set:letter))

(test-begin "srfi-130")

;; tests from adapted chibi, which were
;; adapted from Gauche's SRFI 13 tests, via Chicken

(test "string-null?" #f (string-null? "abc"))
(test "string-null?" #t (string-null? ""))
(test "string-every" #t (string-every #\a ""))
(test "string-every" #t (string-every #\a "aaaa"))
(test "string-every" #f (string-every #\a "aaba"))
;;[esl]- accepting charsets is not in a 130 spec
;(test "string-every" #t (string-every char-set:lower-case "aaba"))
;(test "string-every" #f (string-every char-set:lower-case "aAba"))
;(test "string-every" #t (string-every char-set:lower-case ""))
(test "string-every" (char->integer #\A) (string-every (lambda (x) (char->integer x)) "aAbA"))
(test "string-every" #t (string-every (lambda (x) (error "hoge")) ""))
(test "string-any" #t (string-any #\a "aaaa"))
(test "string-any" #f (string-any #\a "Abcd"))
(test "string-any" #f (string-any #\a ""))
;;[esl]- accepting charsets is not in a 130 spec
;(test "string-any" #t (string-any char-set:lower-case "ABcD"))
;(test "string-any" #f (string-any char-set:lower-case "ABCD"))
;(test "string-any" #f (string-any char-set:lower-case ""))
(test "string-any" (char->integer #\a)
  (string-any (lambda (x) (char->integer x)) "aAbA"))

(test "string-tabulate" "0123456789"
  (string-tabulate (lambda (code)
                      (integer->char (+ code (char->integer #\0))))
                    10))
(test "string-tabulate" ""
  (string-tabulate (lambda (code)
                      (integer->char (+ code (char->integer #\0))))
                    0))

(test "reverse-list->string" "cBa"
  (reverse-list->string '(#\a #\B #\c)))
(test "reverse-list->string" ""
  (reverse-list->string '()))

(test "substring/cursors" "cde" (substring/cursors "abcde" 2 5))
(test "substring/cursors" "cd"  (substring/cursors "abcde" 2 4))

(test "string-copy!" "abCDEfg"
  (let ((x (string-copy "abcdefg")))
    (string-copy! x 2 "CDE")
    x))
(test "string-copy!" "abCDEfg"
  (let ((x (string-copy "abcdefg")))
    (string-copy! x 2 "ZABCDE" 3)
    x))
(test "string-copy!" "abCDEfg"
  (let ((x (string-copy "abcdefg")))
    (string-copy! x 2 "ZABCDEFG" 3 6)
    x))

(test "string-take" "Pete S"  (string-take "Pete Szilagyi" 6))
(test "string-take" ""        (string-take "Pete Szilagyi" 0))
(test "string-take" "Pete Szilagyi" (string-take "Pete Szilagyi" 13))
(test "string-drop" "zilagyi" (string-drop "Pete Szilagyi" 6))
(test "string-drop" "Pete Szilagyi" (string-drop "Pete Szilagyi" 0))
(test "string-drop" ""        (string-drop "Pete Szilagyi" 13))

(test "string-take-right" "rules" (string-take-right "Beta rules" 5))
(test "string-take-right" ""      (string-take-right "Beta rules" 0))
(test "string-take-right" "Beta rules" (string-take-right "Beta rules" 10))
(test "string-drop-right" "Beta " (string-drop-right "Beta rules" 5))
(test "string-drop-right" "Beta rules" (string-drop-right "Beta rules" 0))
(test "string-drop-right" ""      (string-drop-right "Beta rules" 10))

(test "string-pad" "  325" (string-pad "325" 5))
(test "string-pad" "71325" (string-pad "71325" 5))
(test "string-pad" "71325" (string-pad "8871325" 5))
(test "string-pad" "~~325" (string-pad "325" 5 #\~))
(test "string-pad" "~~~25" (string-pad "325" 5 #\~ 1))
(test "string-pad" "~~~~2" (string-pad "325" 5 #\~ 1 2))
(test "string-pad-right" "325  " (string-pad-right "325" 5))
(test "string-pad-right" "71325" (string-pad-right "71325" 5))
(test "string-pad-right" "88713" (string-pad-right "8871325" 5))
(test "string-pad-right" "325~~" (string-pad-right "325" 5 #\~))
(test "string-pad-right" "25~~~" (string-pad-right "325" 5 #\~ 1))
(test "string-pad-right" "2~~~~" (string-pad-right "325" 5 #\~ 1 2))

(test "string-trim"  "a b c d  \n"
  (string-trim "  \t  a b c d  \n"))
(test "string-trim"  "\t  a b c d  \n"
  (string-trim "  \t  a b c d  \n" #\space))
(test "string-trim"  "a b c d  \n"
  (string-trim "4358948a b c d  \n" char-numeric?))

(test "string-trim-right"  "  \t  a b c d"
  (string-trim-right "  \t  a b c d  \n"))
(test "string-trim-right"  "  \t  a b c d  "
  (string-trim-right "  \t  a b c d  \n" (lambda (ch) (eqv? ch #\newline))))
(test "string-trim-right"  "349853a b c d"
  (string-trim-right "349853a b c d03490" char-numeric?))

(test "string-trim-both"  "a b c d"
  (string-trim-both "  \t  a b c d  \n"))
(test "string-trim-both"  "  \t  a b c d  "
  (string-trim-both "  \t  a b c d  \n" (lambda (ch) (eqv? ch #\newline))))
(test "string-trim-both"  "a b c d"
  (string-trim-both "349853a b c d03490" char-numeric?))

;; TODO: bunch of string= families

(test "string-prefix-length" 5
  (string-prefix-length "cancaNCAM" "cancancan"))
(test "string-suffix-length" 2
  (string-suffix-length "CanCan" "cankancan"))

(test "string-prefix?" #t (string-prefix? "abcd" "abcdefg"))
(test "string-prefix?" #f (string-prefix? "abcf" "abcdefg"))
(test "string-suffix?" #t (string-suffix? "defg" "abcdefg"))
(test "string-suffix?" #f (string-suffix? "aefg" "abcdefg"))

(test "string-index #1" 4
  (string-index->index "abcd:efgh:ijkl" (lambda (ch) (eqv? ch #\:))))
(test "string-index #2" 4
  (string-index->index "abcd:efgh;ijkl" (lambda (ch) (char-set-contains? char-set:not-letter ch))))
(test "string-index #3" 14
  (string-index->index "abcd:efgh;ijkl" (lambda (ch) (char-set-contains? char-set:digit ch))))
(test "string-index #4" 9
  (string-index->index "abcd:efgh:ijkl" (lambda (ch) (eqv? ch #\:)) 5))
(test "string-index-right #1" 5
  (string-index-right->index "abcd:efgh;ijkl" (lambda (ch) (eqv? ch #\:))))
(test "string-index-right #2" 10
  (string-index-right->index "abcd:efgh;ijkl" (lambda (ch) (char-set-contains? char-set:not-letter ch))))
(test "string-index-right #3" 14
  (string-index-right->index "abcd:efgh;ijkl" char-alphabetic?))
(test "string-index-right #4" 10
  (string-index-right->index "abcd:efgh;ijkl" (lambda (ch) (char-set-contains? char-set:not-letter ch)) 7))

(test "string-count #1" 2
  (string-count "abc def\tghi jkl" (lambda (ch) (eqv? ch #\space))))
(test "string-count #2" 3
  (string-count "abc def\tghi jkl" char-whitespace?))
(test "string-count #3" 2
  (string-count "abc def\tghi jkl" char-whitespace? 4))
(test "string-count #4" 1
  (string-count "abc def\tghi jkl" char-whitespace? 4 9))
(let ((s "Ma mere l'oye"))
  (test-assert "string-contains"
    (string-contains s "mer"))
  (test "string-contains" #f
    (string-contains s "Mer"))
  (test-assert "string-contains"
    (string-contains s "mer" 1))
  (test-assert "string-contains"
    (string-contains s "mer" 1 8))
  (test-not "string-contains"
            (string-contains s "mer" 4 8))
  (test-not "string-contains"
            (string-contains s "mer" 1 5)))
(let ((s "eek -- it's a geek."))
  (test 15 (string-cursor->index s (string-contains-right s "ee")))
  (test 15 (string-cursor->index s (string-contains-right s "ee" 12 18)))
  (test 19 (string-cursor->index s (string-contains-right s "")))
  (test 0 (string-cursor->index "" (string-contains-right "" "")))
  (test #f (string-contains-right s "kee" 12 18)))

(test "string-reverse" "nomel on nolem on"
  (string-reverse "no melon no lemon"))
(test "string-reverse" "nomel on"
  (string-reverse "no melon no lemon" 9))
(test "string-reverse" "on"
  (string-reverse "no melon no lemon" 9 11))

(test "string-append" #f
  (let ((s "test")) (eq? s (string-append s))))
(test "string-concatenate" #f
  (let ((s "test")) (eq? s (string-concatenate (list s)))))
(test "string-concatenate" "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"
  (string-concatenate
    '("A" "B" "C" "D" "E" "F" "G" "H"
      "I" "J" "K" "L" "M" "N" "O" "P"
      "Q" "R" "S" "T" "U" "V" "W" "X" "Y" "Z"
      "a" "b" "c" "d" "e" "f" "g" "h"
      "i" "j" "k" "l" "m" "n" "o" "p"
      "q" "r" "s" "t" "u" "v" "w" "x" "y" "z")))
(test "string-concatenate-reverse" "zyxwvutsrqponmlkjihgfedcbaZYXWVUTSRQPONMLKJIHGFEDCBA"
  (string-concatenate-reverse
    '("A" "B" "C" "D" "E" "F" "G" "H"
      "I" "J" "K" "L" "M" "N" "O" "P"
      "Q" "R" "S" "T" "U" "V" "W" "X" "Y" "Z"
      "a" "b" "c" "d" "e" "f" "g" "h"
      "i" "j" "k" "l" "m" "n" "o" "p"
      "q" "r" "s" "t" "u" "v" "w" "x" "y" "z")))
(test "string-concatenate-reverse" #f
  (let ((s "test"))
    (eq? s (string-concatenate-reverse (list s)))))

(test "string-map" "svool"
  (string-map (lambda (c)
                (integer->char (- 219 (char->integer c))))
              "hello"))
;; (test "string-map" "vool"
;;   (string-map (lambda (c)
;;                 (integer->char (- 219 (char->integer c))))
;;               "hello" 1))
;; (test "string-map" "vo"
;;   (string-map (lambda (c)
;;                 (integer->char (- 219 (char->integer c))))
;;               "hello" 1 3))

(test "string-fold" '(#\o #\l #\l #\e #\h . #t)
  (string-fold cons #t "hello"))
(test "string-fold" '(#\l #\e . #t)
  (string-fold cons #t "hello" 1 3))
(test "string-fold-right" '(#\h #\e #\l #\l #\o . #t)
  (string-fold-right cons #t "hello"))
(test "string-fold-right" '(#\e #\l . #t)
  (string-fold-right cons #t "hello" 1 3))

(test "string-unfold" "hello"
  (string-unfold null? car cdr '(#\h #\e #\l #\l #\o)))
(test "string-unfold" "hi hello"
  (string-unfold null? car cdr '(#\h #\e #\l #\l #\o) "hi "))
(test "string-unfold" "hi hello ho"
  (string-unfold null? car cdr
                  '(#\h #\e #\l #\l #\o) "hi "
                  (lambda (x) " ho")))

(test "string-unfold-right" "olleh"
  (string-unfold-right null? car cdr '(#\h #\e #\l #\l #\o)))
(test "string-unfold-right" "olleh hi"
  (string-unfold-right null? car cdr '(#\h #\e #\l #\l #\o) " hi"))
(test "string-unfold-right" "ho olleh hi"
  (string-unfold-right null? car cdr
                        '(#\h #\e #\l #\l #\o) " hi"
                        (lambda (x) "ho ")))

(test "string-for-each" "CLtL"
  (let ((out (open-output-string))
        (prev #f))
    (string-for-each (lambda (c)
                        (if (or (not prev)
                                (char-whitespace? prev))
                            (write-char c out))
                        (set! prev c))
                      "Common Lisp, the Language")

    (get-output-string out)))
;; (test "string-for-each" "oLtL"
;;   (let ((out (open-output-string))
;;         (prev #f))
;;     (string-for-each (lambda (c)
;;                        (if (or (not prev)
;;                                (char-whitespace? prev))
;;                            (write-char c out))
;;                        (set! prev c))
;;                      "Common Lisp, the Language" 1)
;;     (get-output-string out)))
;; (test "string-for-each" "oL"
;;   (let ((out (open-output-string))
;;         (prev #f))
;;     (string-for-each (lambda (c)
;;                        (if (or (not prev)
;;                                (char-whitespace? prev))
;;                            (write-char c out))
;;                        (set! prev c))
;;                      "Common Lisp, the Language" 1 10)
;;     (get-output-string out)))

(test "string-for-each-cursor" '(4 3 2 1 0)
  (let ((r '()))
    (string-for-each-cursor (lambda (i) (set! r (cons i r))) "hello")
    (map (lambda (sc) (string-cursor->index "hello" sc)) r)))
(test "string-for-each-cursor" '(4 3 2 1)
  (let ((r '()))
    (string-for-each-cursor (lambda (i) (set! r (cons i r))) "hello" 1)
    (map (lambda (sc) (string-cursor->index "hello" sc)) r)))
(test "string-for-each-cursor" '(2 1)
  (let ((r '()))
    (string-for-each-cursor (lambda (i) (set! r (cons i r))) "hello" 1 3)
    (map (lambda (sc) (string-cursor->index "hello" sc)) r)))

(test "string-replicate" "cdefab"
  (string-replicate "abcdef" 2 8))
(test "string-replicate" "efabcd"
  (string-replicate "abcdef" -2 4))
(test "string-replicate" "abcabca"
  (string-replicate "abc" 0 7))
(test "string-replicate" "defdefd"
  (string-replicate "abcdefg" 0 7 3 6))
(test "string-replicate" ""
  (string-replicate "abcdefg" 9 9 3 6))

(test "string-replace" "abcdXYZghi"
  (string-replace "abcdefghi" "XYZ" 4 6))
(test "string-replace" "abcdZghi"
  (string-replace "abcdefghi" "XYZ" 4 6 2))
(test "string-replace" "abcdZefghi"
  (string-replace "abcdefghi" "XYZ" 4 4 2))
(test "string-replace" "abcdefghi"
  (string-replace "abcdefghi" "XYZ" 4 4 1 1))
(test "string-replace" "abcdhi"
  (string-replace "abcdefghi" "" 4 7))

(test "string-filter" "rrrr"
  (string-filter (lambda (ch) (eqv? ch #\r))
                  "Help make programs run, run, RUN!"))
(test "string-filter" "HelpmakeprogramsrunrunRUN"
  (string-filter char-alphabetic? "Help make programs run, run, RUN!"))

(test "string-filter" "programsrunrun"
  (string-filter (lambda (c) (char-lower-case? c))
                  "Help make programs run, run, RUN!"
                  10))
(test "string-filter" ""
  (string-filter (lambda (c) (char-lower-case? c)) ""))
(test "string-remove" "Help make pogams un, un, RUN!"
  (string-remove (lambda (ch) (eqv? ch #\r))
                  "Help make programs run, run, RUN!"))
(test "string-remove" "   , , !"
  (string-remove char-alphabetic? "Help make programs run, run, RUN!"))
(test "string-remove" " , , RUN!"
  (string-remove (lambda (c) (char-lower-case? c))
                  "Help make programs run, run, RUN!"
                  10))
(test "string-remove" ""
  (string-remove (lambda (c) (char-lower-case? c)) ""))

(test "foo:bar:baz"
    (string-join '("foo" "bar" "baz") ":"))
(test "foo:bar:baz:"
    (string-join '("foo" "bar" "baz") ":" 'suffix))
(test "" (string-join '() ":"))
(test "" (string-join '("") ":"))
(test "" (string-join '() ":" 'suffix))
(test ":" (string-join '("") ":" 'suffix))

(test '("foo" "bar" "baz")
    (string-split "foo:bar:baz" ":"))
(test '("foo" "bar" "baz" "")
    (string-split "foo:bar:baz:" ":"))
(test '("foo" "bar" "baz")
    (string-split "foo:bar:baz:" ":" 'suffix))
(test '("foo" "bar:baz:")
    (string-split "foo:bar:baz:" ":" 'suffix 1))
(test '("foo" "bar" "baz:")
    (string-split "foo:bar:baz:" ":" 'suffix 2))
(test '() (string-split "" ":"))
(test '() (string-split "" ":" 'suffix))
(test '("") (string-split ":" ":" 'suffix))
(test '("foo" "bar" "baz")
    (string-split ":foo:bar:baz" ":" 'prefix))

;; from SRFI 130 test suite
(test '() (string-split "" ""))
(test '("a" "b" "c") (string-split "abc" ""))
(test '("too" "" "much" "" "data")
    (string-split "too  much  data" " "))
(test '("" "there" "ya" "go" "")
    (string-split "***there***ya***go***" "***"))
(test '() (string-split "" "" 'infix))
(test '("a" "b" "c") (string-split "abc" "" 'infix))
(test '("too" "" "much" "" "data")
    (string-split "too  much  data" " " 'infix))
(test '("" "there" "ya" "go" "")
    (string-split "***there***ya***go***" "***" 'infix))
(test-error (string-split "" "" 'strict-infix))
(test '("a" "b" "c") (string-split "abc" "" 'strict-infix))
(test '("too" "" "much" "" "data")
    (string-split "too  much  data" " " 'strict-infix))
(test '("" "there" "ya" "go" "")
    (string-split "***there***ya***go***" "***" 'strict-infix))
(test '() (string-split "" "" 'prefix))
(test '("a" "b" "c") (string-split "abc" "" 'prefix))
(test '("too" "" "much" "" "data")
    (string-split "too  much  data" " " 'prefix))
(test '("there" "ya" "go" "")
    (string-split "***there***ya***go***" "***" 'prefix))
(test '() (string-split "" "" 'suffix))
(test '("a" "b" "c") (string-split "abc" "" 'suffix))
(test '("too" "" "much" "" "data")
    (string-split "too  much  data" " " 'suffix))
(test '("" "there" "ya" "go")
    (string-split "***there***ya***go***" "***" 'suffix))
(test '() (string-split "" "" 'infix #f))
(test '("a" "b" "c") (string-split "abc" "" 'infix #f))
(test '("too" "" "much" "" "data")
    (string-split "too  much  data" " " 'infix #f))
(test '("" "there" "ya" "go" "")
    (string-split "***there***ya***go***" "***" 'infix #f))
(test '("a" "b" "c") (string-split "abc" "" 'strict-infix #f))
(test '("too" "" "much" "" "data")
    (string-split "too  much  data" " " 'strict-infix #f))
(test '("" "there" "ya" "go" "")
    (string-split "***there***ya***go***" "***" 'strict-infix #f))
(test '() (string-split "" "" 'prefix #f))
(test '("a" "b" "c") (string-split "abc" "" 'prefix #f))
(test '("too" "" "much" "" "data")
    (string-split "too  much  data" " " 'prefix #f))
(test '("there" "ya" "go" "")
    (string-split "***there***ya***go***" "***" 'prefix #f))
(test '() (string-split "" "" 'suffix #f))
(test '("a" "b" "c") (string-split "abc" "" 'suffix #f))
(test '("too" "" "much" "" "data")
    (string-split "too  much  data" " " 'suffix #f))
(test '("" "there" "ya" "go")
    (string-split "***there***ya***go***" "***" 'suffix #f))
(test-error (string-split "" "" 'strict-infix 3))
(test '("a" "b" "c") (string-split "abc" "" 'strict-infix 3))
(test '("too" "" "much" " data")
    (string-split "too  much  data" " " 'strict-infix 3))
(test '("" "there" "ya" "go***")
    (string-split "***there***ya***go***" "***" 'strict-infix 3))
(test '() (string-split "" "" 'prefix 3))
(test '("a" "b" "c") (string-split "abc" "" 'prefix 3))
(test '("too" "" "much" " data")
    (string-split "too  much  data" " " 'prefix 3))
(test '("there" "ya" "go***")
    (string-split "***there***ya***go***" "***" 'prefix 3))
(test '() (string-split "" "" 'suffix 3))
(test '("a" "b" "c") (string-split "abc" "" 'suffix 3))
(test '("too" "" "much" " data")
    (string-split "too  much  data" " " 'suffix 3))
(test '("" "there" "ya" "go***")
    (string-split "***there***ya***go***" "***" 'suffix 3))
(test-error (string-split "" "" 'strict-infix 3 0))
(test '("b" "c") (string-split "abc" "" 'strict-infix 3 1))
(test '("oo" "" "much" " data")
    (string-split "too  much  data" " " 'strict-infix 3 1))
(test '("**there" "ya" "go" "")
    (string-split "***there***ya***go***" "***" 'strict-infix 3 1))
(test '() (string-split "" "" 'prefix 3 0))
(test '("b" "c") (string-split "abc" "" 'prefix 3 1))
(test '("oo" "" "much" " data")
    (string-split "too  much  data" " " 'prefix 3 1))
(test '("**there" "ya" "go" "")
    (string-split "***there***ya***go***" "***" 'prefix 3 1))
(test '() (string-split "" "" 'suffix 3 0))
(test '("b" "c") (string-split "abc" "" 'suffix 3 1))
(test '("oo" "" "much" " data")
    (string-split "too  much  data" " " 'suffix 3 1))
(test '("**there" "ya" "go")
    (string-split "***there***ya***go***" "***" 'suffix 3 1))
(test-error (string-split "" "" 'strict-infix 3 0 0))
(test '("b") (string-split "abc" "" 'strict-infix 3 1 2))
(test '("oo" "" "much" " ")
    (string-split "too  much  data" " " 'strict-infix 3 1 11))
(test '() (string-split "" "" 'prefix 3 0 0))
(test '("b") (string-split "abc" "" 'prefix 3 1 2))
(test '("oo" "" "much" " ")
    (string-split "too  much  data" " " 'prefix 3 1 11))
(test '() (string-split "" "" 'suffix 3 0 0))
(test '("b") (string-split "abc" "" 'suffix 3 1 2))
(test '("oo" "" "much" " ")
    (string-split "too  much  data" " " 'suffix 3 1 11))

;;; Regression tests: check that reported bugs have been fixed

;; From: Matthias Radestock <matthias@sorted.org>
;; Date: Wed, 10 Dec 2003 21:05:22 +0100
                                  ;
;; Chris Double has found the following bug in the reference implementation:
                                  ;
;;  (string-contains "xabc" "ab") => 1    ;good
;;  (string-contains "aabc" "ab") => #f   ;bad
                                  ;
;; Matthias.

(test "string-contains" 1
  (string-cursor->index "aabc" (string-contains "aabc" "ab")))

(test "string-contains" 5
  (string-cursor->index "ababdabdabxxas" (string-contains "ababdabdabxxas" "abdabx")))

;; (message continues)
;;
;; PS: There is also an off-by-one error in the bounds check of the
;; unoptimized version of string-contains that is included as commented out
;; code in the reference implementation. This breaks things like
;; (string-contains "xab" "ab") and (string-contains "ab" "ab").

;; This off-by-one bug has been fixed in the comments of the version
;; of SRFI-13 shipped with Larceny.  In a version of the code without
;; the fix the following test will catch the bug:

(test "string-contains" 0
  (string-cursor->index "ab" (string-contains "ab" "ab")))

;; even string-cursor->index/index->cursor are polymorphic :(
(let* ((s "abc")
        (i 1)
        (sc (string-index->cursor s i)))
  (test i (string-cursor->index s sc))
  (test i (string-cursor->index s i))
  (test sc (string-index->cursor s sc))
  (test sc (string-index->cursor s i)))

(test-end)
