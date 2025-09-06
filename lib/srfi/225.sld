;[esl] adapted from the original ref. imp. by Arvydas Silanskas 

(define-library (srfi 225)
  (import
    (scheme base)
    (srfi 1)
    (srfi 128)
    (srfi 225 core)
    (srfi 225 default-impl)
    (srfi 225 indexes))

  (include-library-declarations "225/core-exports.scm")
  (include-library-declarations "225/indexes-exports.scm")
  (export make-dto)

  ;; common implementations
  (import (srfi 225 alist-impl))
  (export
    make-alist-dto
    eqv-alist-dto
    equal-alist-dto)

  ;; library-dependent DTO exports
  ;; and implementations
  ;;
  ;;srfi-69-dto
  ;;hash-table-dto
  ;;srfi-126-dto
  ;;mapping-dto
  ;;hash-mapping-dto

  (cond-expand
    ((library (srfi 69))
     (import (srfi 225 srfi-69-impl))
     (export srfi-69-dto))
    (else))

  (cond-expand
    ((library (srfi 125))
     (import (srfi 225 srfi-125-impl))
     (export hash-table-dto))
    (else))

  (cond-expand
    ((library (srfi 126))
     (import (srfi 225 srfi-126-impl))
     (export srfi-126-dto))
    (else))

  (cond-expand
    ((and (library (srfi 146))
          (library (srfi 146 hash)))
     (import (srfi 225 srfi-146-impl)
             (srfi 225 srfi-146-hash-impl))
     (export mapping-dto
             hash-mapping-dto))
    (else)))
