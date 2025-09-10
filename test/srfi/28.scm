(import (srfi 28))
(include "test.scm")

;;[esl]: selected tests from SRFI 48 by Ken Dickey


(test-begin "srfi-28")

;; tests, based on the original Ken Dickey's examples
;;

(test-equal "Hello, World!" (format "Hello, ~a" "World!"))

(test-equal "Error, list is too short: (one \"two\" 3)" (format "Error, list is too short: ~s" (quote (one "two" 3))))

(test-equal "test me" (format "test me"))

(test-equal "this is a \"test\"" (format "~a ~s ~a ~s" (quote this) (quote is) "a" "test"))

;;; selected tests, adapted from
;;; SPDX-FileCopyrightText: 2017 Hamayama <hamay1010@gmail.com>
;;; SPDX-FileCopyrightText: 2025  Retropikzel <retropikzel@iki.fi>
;;;
;;; SPDX-License-Identifier: MIT

(define pi 3.141592653589793)

(test-equal (format "test ~s" 'me) (format #f "test ~a" "me"))

(test-equal "\n" (format "~A~A" (list->string (list #\newline)) ""))

(test-equal "Hello, World!" (format "Hello, ~a" "World!"))

(test-equal "Error, list is too short: (one \"two\" 3)" (format "Error, list is too short: ~s" (quote (one "two" 3))))

(test-equal "test me" (format "test me"))

(test-equal "this is a \"test\"" (format "~a ~s ~a ~s" (quote this) (quote is) "a" "test"))

(test-end)
