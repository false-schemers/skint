(define-library
  (srfi 225 core)

  (import (scheme base)
          (scheme case-lambda)
          (srfi 1)
          (srfi 128)
          (srfi 225 indexes))
  (cond-expand
    ((library (srfi 145)) (import (srfi 145)))
    (else (include "assumptions.scm")))

  (include "core-impl.scm")
  (include-library-declarations "core-exports.scm")
  (export make-dto-private
          make-modified-dto
          procvec
          dict-procedures-count))
