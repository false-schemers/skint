;[esl] adapted from the original ref. impl. by Antero Mejr
;[esl] tailored to numbers supported by Skint (fixnums/flonums/full tower) 

;; Property-based testing extension for SRFI 64.
;; SPDX-License-Identifier: MIT
;; Copyright 2024 Antero Mejr <mail@antr.me>

;; Permission is hereby granted, free of charge, to any person
;; obtaining a copy of this software and associated documentation
;; files (the "Software"), to deal in the Software without
;; restriction, including without limitation the rights to use, copy,
;; modify, merge, publish, distribute, sublicense, and/or sell copies
;; of the Software, and to permit persons to whom the Software is
;; furnished to do so, subject to the following conditions:

;; The above copyright notice and this permission notice shall be
;; included in all copies or substantial portions of the Software.

;; THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
;; EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
;; MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
;; NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
;; BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
;; ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
;; CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
;; SOFTWARE.

(define-library (srfi 252)

  (import (scheme base)
          (scheme case-lambda)
          ;(scheme complex)
          (srfi 1)
          (srfi 64)
          (srfi 158)
          (srfi 194)
          (srfi 143)
          (srfi 144))

  (cond-expand 
    (full-numeric-tower
      (import (scheme complex))))

  (export test-property test-property-expect-fail test-property-skip
          test-property-error test-property-error-type
          property-test-runner
          ;; Generator procedures
          boolean-generator bytevector-generator
          char-generator string-generator symbol-generator
          ;; exact number generators
          ;exact-complex-generator 
          exact-integer-generator
          exact-number-generator 
          ;exact-rational-generator
          exact-real-generator
          ;exact-integer-complex-generator
          ;; inexact number generators
          ;inexact-complex-generator 
          inexact-integer-generator
          inexact-number-generator 
          ;inexact-rational-generator
          inexact-real-generator
          ;; Unions of numerical generators
          ;complex-generator 
          integer-generator
          number-generator 
          ;rational-generator
          real-generator
          ;; Special generators
          list-generator-of pair-generator-of procedure-generator-of
          vector-generator-of)

  (cond-expand 
    (full-numeric-tower
      (export 
        exact-complex-generator
        exact-rational-generator
        exact-integer-complex-generator
        inexact-complex-generator
        inexact-rational-generator
        complex-generator
        rational-generator)))
        
  (cond-expand
    (full-numeric-tower
      (include "252/full.scm"))
    (else
      (include "252/abridged.scm"))))
