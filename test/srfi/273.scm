;; SPDX-FileCopyrightText: 2026 Artyom Bologov
;; SPDX-License-Identifier: MIT

;;; Permission is hereby granted, free of charge, to any person
;;; obtaining a copy of this software and associated documentation
;;; files (the "Software"), to deal in the Software without
;;; restriction, including without limitation the rights to use,
;;; copy, modify, merge, publish, distribute, sublicense, and/or
;;; sell copies of the Software, and to permit persons to whom the
;;; Software is furnished to do so, subject to the following
;;; conditions:
;;;
;;; The above copyright notice and this permission notice shall be
;;; included in all copies or substantial portions of the Software.
;;;
;;; THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
;;; EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
;;; OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
;;; NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
;;; HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
;;; WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
;;; FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
;;; OTHER DEALINGS IN THE SOFTWARE.

(features (cons 'debug (features)))

(import (scheme base))
(import (scheme eval))
(import (scheme repl))
(import (srfi 253)
        (srfi 273))
(include "test.scm")

(define-check email? string?)
(define-check positive-integer?
  (lambda (x) (and (integer? x) (positive? x))))

(define-syntax check-arg-true
  (syntax-rules ()
    ((_ pred val)
     (begin
       (check-arg pred val)
       #t))))

(test-begin "srfi-273")
(test-assert (check-arg-true email? "srfi-273@srfi.schemers.org"))
(test-assert (check-arg-true positive-integer? 3))
(test-error (check-arg-true positive-integer? 0))
(test-error (check-arg-true positive-integer? -8))

;; Post-declaration for standard symbols
(test-assert (begin
               (declare-checked (negative? (x real?)) => (boolean?))
               #t))
(test-assert (begin
               (declare-checked (call/cc (k procedure?)))
               #t))
(test-end "srfi-273")
