(import (srfi 115)) ;(srfi 130)

(include "test.scm")

;;;; SPDX-FileCopyrightText: 2015 - 2019 Alex Shinn
;;;;
;;;; SPDX-License-Identifier: BSD-3-Clause

(define (maybe-match->list rx str . o)
  (let ((res (apply regexp-matches rx str o)))
    (and res (regexp-match->list res))))

(define-syntax test-re
  (syntax-rules ()
    ((test-re res rx str start end)
    (test res (maybe-match->list rx str start end)))
    ((test-re res rx str start)
    (test-re res rx str start (string-length str)))
    ((test-re res rx str)
    (test-re res rx str 0))))

(define (maybe-search->list rx str . o)
  (let ((res (apply regexp-search rx str o)))
    (and res (regexp-match->list res))))

(define-syntax test-re-search
  (syntax-rules ()
    ((test-re-search res rx str start end)
    (test res (maybe-search->list rx str start end)))
    ((test-re-search res rx str start)
    (test-re-search res rx str start (string-length str)))
    ((test-re-search res rx str)
    (test-re-search res rx str 0))))

(test-begin "regexp")

(test-re '("ababc" "abab")
        '(: ($ (* "ab")) "c")
        "ababc")

(test-re '("ababc" "abab")
        '(: ($ (* "ab")) "c")
        "xababc"
        1)

(test-re-search '("y") '(: "y") "xy")

(test-re-search '("ababc" "abab")
                '(: ($ (* "ab")) "c")
                "xababc")

(test-re #f
        '(: (* any) ($ "foo" (* any)) ($ "bar" (* any)))
        "fooxbafba")

(test-re '("fooxbarfbar" "fooxbarf" "bar")
        '(: (* any) ($ "foo" (* any)) ($ "bar" (* any)))
        "fooxbarfbar")

(test-re '("abcd" "abcd")
        '($ (* (or "ab" "cd")))
        "abcd")

(test "ab"
    (regexp-match-submatch
    (regexp-matches '(or (-> foo "ab") (-> foo "cd")) "ab")
    'foo))

(test "cd"
    (regexp-match-submatch
    (regexp-matches '(or (-> foo "ab") (-> foo "cd")) "cd")
    'foo))

;; non-deterministic case from issue #229
(let* ((elapsed '(: (** 1 2 num) ":" num num (? ":" num num)))
        (span (rx ,elapsed "-" ,elapsed)))
(test-re-search '("1:45:02-2:06:13") span " 1:45:02-2:06:13 "))

(test-re '("ababc" "abab")
        '(: bos ($ (* "ab")) "c")
        "ababc")
(test-re '("ababc" "abab")
        '(: ($ (* "ab")) "c" eos)
        "ababc")
(test-re '("ababc" "abab")
        '(: bos ($ (* "ab")) "c" eos)
        "ababc")
(test-re #f
        '(: bos ($ (* "ab")) eos "c")
        "ababc")
(test-re #f
        '(: ($ (* "ab")) bos "c" eos)
        "ababc")

(test-re '("ababc" "abab")
        '(: bol ($ (* "ab")) "c")
        "ababc")
(test-re '("ababc" "abab")
        '(: ($ (* "ab")) "c" eol)
        "ababc")
(test-re '("ababc" "abab")
        '(: bol ($ (* "ab")) "c" eol)
        "ababc")
(test-re #f
        '(: bol ($ (* "ab")) eol "c")
        "ababc")
(test-re #f
        '(: ($ (* "ab")) bol "c" eol)
        "ababc")
(test-re '("\nabc\n" "abc")
        '(: (* #\newline) bol ($ (* alpha)) eol (* #\newline))
        "\nabc\n")
(test-re #f
        '(: (* #\newline) bol ($ (* alpha)) eol (* #\newline))
        "\n'abc\n")
(test-re #f
        '(: (* #\newline) bol ($ (* alpha)) eol (* #\newline))
        "\nabc.\n")

(test-re '("ababc" "abab")
        '(: bow ($ (* "ab")) "c")
        "ababc")
(test-re '("ababc" "abab")
        '(: ($ (* "ab")) "c" eow)
        "ababc")
(test-re '("ababc" "abab")
        '(: bow ($ (* "ab")) "c" eow)
        "ababc")
(test-re #f
        '(: bow ($ (* "ab")) eow "c")
        "ababc")
(test-re #f
        '(: ($ (* "ab")) bow "c" eow)
        "ababc")
(test-re '("  abc  " "abc")
        '(: (* space) bow ($ (* alpha)) eow (* space))
        "  abc  ")
(test-re #f
        '(: (* space) bow ($ (* alpha)) eow (* space))
        " 'abc  ")
(test-re #f
        '(: (* space) bow ($ (* alpha)) eow (* space))
        " abc.  ")
(test-re '("abc  " "abc")
        '(: ($ (* alpha)) (* any))
        "abc  ")
(test-re '("abc  " "")
        '(: ($ (*? alpha)) (* any))
        "abc  ")
(test-re '("<em>Hello World</em>" "em>Hello World</em")
        '(: "<" ($ (* any)) ">" (* any))
        "<em>Hello World</em>")
(test-re '("<em>Hello World</em>" "em")
        '(: "<" ($ (*? any)) ">" (* any))
        "<em>Hello World</em>")
(test-re-search '("foo") '(: "foo") " foo ")
(test-re-search #f '(: nwb "foo" nwb) " foo ")
(test-re-search '("foo") '(: nwb "foo" nwb) "xfoox")

(test-re '("beef")
        '(* (/"af"))
        "beef")

(test-re '("12345beef" "beef")
        '(: (* numeric) ($ (* (/"af"))))
        "12345beef")

(let ((number '($ (+ numeric))))
(test '("555" "867" "5309")
    (cdr
        (regexp-match->list
        (regexp-search `(: ,number "-" ,number "-" ,number)
                        "555-867-5309"))))
(test '("555" "5309")
    (cdr
        (regexp-match->list
        (regexp-search `(: ,number "-" (w/nocapture ,number) "-" ,number)
                        "555-867-5309")))))

(test-re '("12345BeeF" "BeeF")
        '(: (* numeric) (w/nocase ($ (* (/"af")))))
        "12345BeeF")

(test-re #f '(* lower) "abcD")
(test-re '("abcD") '(w/nocase (* lower)) "abcD")

;;[esl]- systems not supporting full Unicode won't be able to read this
#|(cond-expand
(full-unicode
(test-re '("σζ") '(* lower) "σζ")
(test-re '("Σ") '(* upper) "Σ")
(test-re '("\x01C5;") '(* title) "\x01C5;")
(test-re '("σζ\x01C5;") '(w/nocase (* lower)) "σζ\x01C5;")

(test-re '("кириллица") '(* alpha) "кириллица")
(test-re #f '(w/ascii (* alpha)) "кириллица")
(test-re '("кириллица") '(w/nocase "КИРИЛЛИЦА") "кириллица")

(test-re '("１２３４５") '(* numeric) "１２３４５")
(test-re #f '(w/ascii (* numeric)) "１２３４５")

(test-re '("한") 'grapheme "한")
(test-re '("글") 'grapheme "글")

(test-re '("한") '(: bog grapheme eog) "한")
(test-re #f '(: "ᄒ" bog grapheme eog "ᆫ") "한")

(test '("a" "b" "c") (regexp-extract 'grapheme "abc"))
(test '("a" " " "b" " " "c") (regexp-extract 'grapheme "a b c"))
(test '("a" "\n" "b" "\r\n" "c") (regexp-extract 'grapheme "a\nb\r\nc"))
(test '("a\x0300;" "b\x0301;\x0302;" "c\x0303;\x0304;\x0305;")
        (regexp-extract 'grapheme "a\x0300;b\x0301;\x0302;c\x0303;\x0304;\x0305;"))
(test '("한" "글") (regexp-extract 'grapheme "한글")))
(else))|#

(test '("123" "456" "789") (regexp-extract '(+ numeric) "abc123def456ghi789"))
(test '("123" "456" "789") (regexp-extract '(* numeric) "abc123def456ghi789"))
(test '("abc" "def" "ghi" "") (regexp-split '(+ numeric) "abc123def456ghi789"))
(test '("abc" "def" "ghi" "")
    (regexp-split '(* numeric) "abc123def456ghi789"))
(test '("a" "b") (regexp-split '(+ whitespace) "a b"))
(test '("a" "" "b")
    (regexp-split '(",;") "a,,b"))
(test '("a" "" "b" "")
    (regexp-split '(",;") "a,,b,"))
(test '("")
    (regexp-partition '(* numeric) ""))
(test '("abc" "123" "def" "456" "ghi")
    (regexp-partition '(* numeric) "abc123def456ghi"))
(test '("abc" "123" "def" "456" "ghi" "789")
    (regexp-partition '(* numeric) "abc123def456ghi789"))

(cond-expand
(full-unicode
(test '("한" "글")
        (regexp-extract
        'grapheme
        (utf8->string '#u8(#xe1 #x84 #x92 #xe1 #x85 #xa1 #xe1 #x86 #xab
                                #xe1 #x84 #x80 #xe1 #x85 #xb3 #xe1 #x86 #xaf)))))
(else))

(test "abc def" (regexp-replace '(+ space) "abc \t\n def" " "))
(test "  abc-abc"
    (regexp-replace '(: ($ (+ alpha)) ":" (* space)) "  abc: " '(1 "-" 1)))
(test "  abc-  abc"
    (regexp-replace '(: ($ (+ alpha)) ":" (* space)) "  abc: " '(1 "-" pre 1)))

(test "-abc \t\n d ef  "
    (regexp-replace '(+ space) "  abc \t\n d ef  " "-" 0))
(test "-abc \t\n d ef  "
    (regexp-replace '(+ space) "  abc \t\n d ef  " "-" 0 #f 0))
(test "  abc-d ef  "
    (regexp-replace '(+ space) "  abc \t\n d ef  " "-" 0 #f 1))
(test "  abc \t\n d-ef  "
    (regexp-replace '(+ space) "  abc \t\n d ef  " "-" 0 #f 2))
(test "  abc \t\n d ef-"
    (regexp-replace '(+ space) "  abc \t\n d ef  " "-" 0 #f 3))
(test "  abc \t\n d ef  "
    (regexp-replace '(+ space) "  abc \t\n d ef  " "-" 0 #f 4))
(test " abc d ef " (regexp-replace-all '(+ space) "  abc \t\n d ef  " " "))

(test "bc pre: <<<bc >>> match1: <<<def>>> post: <<<gh>>>gh"
    (regexp-replace
    '(: ($ (+ alpha)) ":" (* space))
    "abc def: ghi"
    '("pre: <<<" pre ">>> match1: <<<" 1 ">>> post: <<<" post ">>>")
    1 11))

(test-end)
