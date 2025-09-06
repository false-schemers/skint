;; R7RS small implementation of lambda-optional, assuming an optimized
;; case-lambda, by Daphne Preston-Kendal.

(define-library (srfi 227)
  (import (scheme base)
          (scheme case-lambda))

  (export (rename lambda-optional opt-lambda)
          (rename lambda-optional* opt*-lambda)
          let-optionals
          let-optionals*)
(begin

(define-syntax lambda-optional
  (syntax-rules ()
    ((_ (req-name ... (opt-name opt-default) . rest) . body)
     (%lambda-optional-parse (req-name ...)
                             ((opt-name opt-default optarg))
                             rest
                             . body))
    ((_ formals . body)
     (lambda formals . body))))

(define-syntax %lambda-optional-parse
  (syntax-rules ()
    ;; workaround for the greediness of ellipsis
    ((_ (req-name ... (opt-name opt-default) . more)
        ((n d a) ...) rest
        . body)
     (%lambda-optional-parse (req-name ...)
                             ((opt-name opt-default optarg)
                              (n d a) ...
                              . more)
                             rest
                             . body))
    ;;; all optional args gathered
    ((_ (req-name ...) ((opt-name opt-default optarg) ...) rest . body)
     (%lambda-optional-generate-temporaries
      (req-name ...)
      (req-name ...)
      ()
      ((opt-name opt-default optarg) ...)
      rest . body))))

(define-syntax %lambda-optional-generate-temporaries
  (syntax-rules ()
    ((_ (req-name ...)
        (name-w/o-gensym . more-names-w/o-gensyms)
        gensyms
        opts rest . body)
     (%lambda-optional-generate-temporaries
      (req-name ...)
      more-names-w/o-gensyms
      (reqarg . gensyms)
      opts rest . body))
    ((_ (req-name ...)
        ()
        (req-gensym ...)
        ((opt-name opt-default opt-gensym) ...)
        rest . body)
     (%lambda-optional-case-lambda
      () (req-name ...) (req-gensym ...)
      (opt-name ...)
      (opt-default ...)
      (opt-gensym ...)
      rest . body))))

(define-syntax %lambda-optional-case-lambda
  (syntax-rules ()
    ((_ (clause ...) (req-name ...) (req-gensym ...)
        (opt-name . more-opt-names)
        (default . more-defaults)
        (opt-gensym . more-opt-gensyms)
        rest . body)
     (%lambda-optional-case-lambda
      (clause ...
       ((req-gensym ...)
        (req-gensym ...
         default
         . more-defaults)))
      (req-name ... opt-name)
      (req-gensym ... opt-gensym)
      more-opt-names more-defaults more-opt-gensyms
      rest . body))
    ((_ (((clause-formals ...) (clause-reals ...)) ...)
        (req-name ...) gensyms () () () rest  . body)
     (letrec ((f (case-lambda ((clause-formals ...) (f clause-reals ...)) ...
                              ((req-name ... . rest) . body))))
       f))))

(define-syntax define-optional
  (syntax-rules ()
    ((_ (name . formals) . body)
     (define name (lambda-optional formals . body)))))

(define-syntax lambda-optional*
  (syntax-rules ()
    ((_ (req-name ... (opt-name opt-default) . rest) . body)
     (%lambda-optional*-parse (req-name ...)
                              ((opt-name opt-default))
                              rest
                              . body))
    ((_ formals . body)
     (lambda formals . body))))

(define-syntax %lambda-optional*-parse
  (syntax-rules ()
    ((_ (req-name ... (opt-name opt-default) . more)
        ((n d) ...) rest
        . body)
     (%lambda-optional*-parse (req-name ...)
                             ((opt-name opt-default)
                              (n d) ...
                              . more)
                             rest
                             . body))
    ;;; all optional args gathered
    ((_ (req-name ...) ((opt-name opt-default) ...) rest . body)
     (%lambda-optional*-case-lambda
      ()
      (req-name ...)
      ((opt-name opt-default) ...)
      rest . body))))

(define-syntax %lambda-optional*-case-lambda
  (syntax-rules ()
    ((_ (clause ...) (req-name ...)
        ((opt-name opt-default) . more-opts)
        rest . body)
     (%lambda-optional*-case-lambda
      (clause ...
       ((req-name ...) (req-name ... opt-default)))
      (req-name ... opt-name)
      more-opts rest . body))
    ((_ (((clause-formals ...) (clause-reals ...)) ...) (req-name ...)
        ()
        rest . body)
     (letrec ((f (case-lambda ((clause-formals ...) (f clause-reals ...)) ...
                              ((req-name ... . rest) . body))))
       f))))

(define-syntax define-optional*
  (syntax-rules ()
    ((_ (name . formals) . body)
     (define name (lambda-optional* formals . body)))))

(define-syntax let-optionals
  (syntax-rules ()
    ((_ expr opt-formals body1 ... body2)
     (apply (lambda-optional opt-formals body1 ... body2) expr))))

(define-syntax let-optionals*
  (syntax-rules ()
    ((_ expr opt-formals body1 ... body2)
     (apply (lambda-optional* opt-formals body1 ... body2) expr))))

))