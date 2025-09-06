(define-library (srfi 162)
  (import (srfi 128))
  (export comparator-max comparator-min
          comparator-max-in-list comparator-min-in-list)
  (export default-comparator boolean-comparator real-comparator
          char-comparator char-ci-comparator
          string-comparator string-ci-comparator
          list-comparator vector-comparator
          eq-comparator eqv-comparator equal-comparator))
