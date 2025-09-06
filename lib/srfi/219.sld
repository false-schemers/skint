
;[esl] original reference implementation by Lassi Kortela

(define-library (srfi 219)
  (export define)
  (import (rename (scheme base) (define native-define)))
  (begin  
    (define-syntax define
      (syntax-rules ()
        ((define ((head . outer-args) . args) . body)
          (define (head . outer-args) (lambda args . body)))
        ((define head . body)
          (native-define head . body))))
  ))