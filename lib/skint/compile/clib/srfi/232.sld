(define-library (srfi 232)
  (import (scheme base) (scheme case-lambda))
  (export curried define-curried)

;[esl] adapted from ref. imp. by Wolfgang Corcoran-Mathe

(begin

;;; SPDX-License-Identifier: MIT
;;; SPDX-FileCopyrightText: 2024 Wolfgang Corcoran-Mathe
;;; (C) 2022 Wolfgang Corcoran-Mathe

(define-syntax curried
  (syntax-rules ()
    ((curried formals exp ...)
     (curried-1 formals (begin exp ...)))))

(define-syntax curried-1
  (syntax-rules ()
    ((curried-1 () exp) exp)
    ((curried-1 (arg0 arg1 ...) exp)
     (one-or-more (arg0 arg1 ...) exp))
    ((curried-1 (arg0 arg1 ... . rest) exp)
     (rest-args (arg0 arg1 ... . rest) exp))
    ((curried-1 args exp) (lambda args exp))))

(define-syntax one-or-more
  (syntax-rules ()
    ((one-or-more (arg0 arg1 ...) exp)
     (letrec
      ((f (case-lambda
            (() f)       ; app. to no args -> original function
            ((arg0 arg1 ...) exp)
            ((arg0 arg1 ... . rest)
             (apply (f arg0 arg1 ...) rest))
            (args (more-args f args)))))
       f))))

(define-syntax rest-args
  (syntax-rules ()
    ((rest-args (arg0 arg1 ... . rest) exp)
     (letrec ((f (case-lambda
                   (() f)
                   ((arg0 arg1 ... . rest) exp)
                   (args (more-args f args)))))
       f))))

(define (more-args f current)
  (lambda args (apply f (append current args))))

(define-syntax define-curried
  (syntax-rules ()
    ((define-curried (var . formals) exp ...)
     (define var
       (curried formals exp ...)))))

))
