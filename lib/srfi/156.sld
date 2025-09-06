
(define-library (srfi 156)
  (import (scheme base))
  (export is isnt)

(begin
;; original implementation by Panicz Maciej Godek 

(define-syntax infix/postfix
  (syntax-rules ()
    ((infix/postfix x somewhat?)
      (somewhat? x))

    ((infix/postfix left related-to? right)
      (related-to? left right))

    ((infix/postfix left related-to? right . likewise)
      (let ((right* right))
        (and (infix/postfix left related-to? right*)
            (infix/postfix right* . likewise))))))

(define-syntax extract-placeholders
  (syntax-rules (_)
    ((extract-placeholders final () () body)
      (final (infix/postfix . body)))

    ((extract-placeholders final () args body)
      (lambda args (final (infix/postfix . body))))

    ((extract-placeholders final (_ op . rest) (args ...) (body ...))
      (extract-placeholders final rest (args ... arg) (body ... arg op)))

    ((extract-placeholders final (arg op . rest) args (body ...))
      (extract-placeholders final rest args (body ... arg op)))

    ((extract-placeholders final (_) (args ...) (body ...))
      (extract-placeholders final () (args ... arg) (body ... arg)))

    ((extract-placeholders final (arg) args (body ...))
      (extract-placeholders final () args (body ... arg)))))

(define-syntax identity-syntax
  (syntax-rules ()
    ((identity-syntax form)
      form)))

(define-syntax is
  (syntax-rules ()
    ((is . something)
      (extract-placeholders identity-syntax something () ()))))

(define-syntax isnt
  (syntax-rules ()
    ((isnt . something)
      (extract-placeholders not something () ()))))
))

