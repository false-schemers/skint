
(define-library (srfi 151)
  (import (scheme base) (scheme case-lambda))

  (export bitwise-not bitwise-and bitwise-ior bitwise-xor bitwise-eqv
          bitwise-nand bitwise-nor bitwise-andc1 bitwise-andc2
          bitwise-orc1 bitwise-orc2)
  (export arithmetic-shift bit-count integer-length bitwise-if 
          bit-set? copy-bit bit-swap any-bit-set? every-bit-set?  first-set-bit)
  (export bit-field bit-field-any? bit-field-every?  bit-field-clear bit-field-set
          bit-field-replace  bit-field-replace-same
          bit-field-rotate bit-field-reverse)
  (export bits->list list->bits bits->vector vector->bits bits
          bitwise-fold bitwise-for-each bitwise-unfold make-bitwise-generator)

(cond-expand
  [(and skint (not full-numeric-tower))
   (import (skint))
   (import (rename (srfi 143) (fxif bitwise-if) (fxfirst-set-bit first-set-bit)))
   (include "151/abridged.scm")]
  [(and skint full-numeric-tower)
   (import (only (skint) bitwise-not bitwise-and bitwise-ior bitwise-xor))
   (import (only (skint) arithmetic-shift integer-length bit-count))
   (include "151/full.scm")]))
