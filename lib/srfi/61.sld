
(define-library (srfi 61)
  (export cond)
  (import (except (scheme base) cond))
  (begin
;;; Copyright (C) 2004 Taylor Campbell. All rights reserved.

;;; Made an R7RS library by Taylan Ulrich Bayirli/Kammer, Copyright (C) 2014.

;;; Permission is hereby granted, free of charge, to any person obtaining a copy
;;; of this software and associated documentation files (the "Software"), to
;;; deal in the Software without restriction, including without limitation the
;;; rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
;;; sell copies of the Software, and to permit persons to whom the Software is
;;; furnished to do so, subject to the following conditions:

;;; The above copyright notice and this permission notice shall be included in
;;; all copies or substantial portions of the Software.

;;; THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
;;; IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
;;; FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
;;; AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
;;; LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
;;; FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
;;; IN THE SOFTWARE.

    (define-syntax cond
      (syntax-rules (=> else)

        ((cond (else else1 else2 ...))
         ;; The (if #t (begin ...)) wrapper ensures that there may be no
         ;; internal definitions in the body of the clause.  R5RS mandates
         ;; this in text (by referring to each subform of the clauses as
         ;; <expression>) but not in its reference implementation of `cond',
         ;; which just expands to (begin ...) with no (if #t ...) wrapper.
         (if #t (begin else1 else2 ...)))

        ((cond (test => receiver) more-clause ...)
         (let ((t test))
              (cond/maybe-more t
                               (receiver t)
                               more-clause ...)))

        ((cond (generator guard => receiver) more-clause ...)
         (call-with-values (lambda () generator)
                           (lambda t
                                   (cond/maybe-more (apply guard    t)
                                                    (apply receiver t)
                                                    more-clause ...))))

        ((cond (test) more-clause ...)
         (let ((t test))
              (cond/maybe-more t t more-clause ...)))

        ((cond (test body1 body2 ...) more-clause ...)
         (cond/maybe-more test
                          (begin body1 body2 ...)
                          more-clause ...))))

    (define-syntax cond/maybe-more
      (syntax-rules ()
        ((cond/maybe-more test consequent)
         (if test
             consequent))
        ((cond/maybe-more test consequent clause ...)
         (if test
             consequent
             (cond clause ...)))))

 ))