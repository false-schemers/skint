
(define-library (srfi 38)
  (import (scheme base) (scheme read) (scheme write) (scheme case-lambda))
  (export read-with-shared-structure write-with-shared-structure)
  (begin
    (define read-with-shared-structure read)
    (define write-with-shared-structure
      (case-lambda
        ((obj) (write-shared obj))
        ((obj port) (write-shared obj port))
        ((obj port optarg) (write-shared obj port))))))
