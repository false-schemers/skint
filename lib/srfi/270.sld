(define-library (srfi 270)
  (import (scheme base) (scheme write) (scheme case-lambda))
  (import (only (skint hidden) inexact->string))
  (export write-hexadecimal-float)

;[esl] uses skint internal formatter 

(begin

(define write-hexadecimal-float 
  (case-lambda 
     [(z) (write-hexadecimal-float z (current-output-port))]
     [(z p) (display (inexact->string z 16 #\a 13) p)]))

))
