
(define-library (srfi 137)
  (export make-type)
  (import (scheme base) (only (skint) record? make-record record-ref record-set!))

(begin

;[esl] based on ref. imp. by John Cowan, Marc Nieper-Wißkirchen 
;[esl] uses Skint record primitives

(define (%make-type proper-ancestors payload)
  ; pick rtd that won't clash with ones produced by regular make-record-type: 
  (define instance-rtd 'instance) 
  (define (type-accessor . rest)
    (if (null? rest) payload ancestors))
  (define (constructor payload)
    (define x (make-record instance-rtd 2))
    (record-set! x 0 ancestors)
    (record-set! x 1 payload)
    x)
  (define (predicate x)
    (and (record? x instance-rtd)
	       (memq constructor (record-ref x 0))
	       #t))
  (define (accessor x)
    (unless (predicate x) 
      (error "not an instance of the correct type" x))
    (record-ref x 1))
  (define (make-subtype payload) 
    (%make-type ancestors payload))
  (define ancestors 
    (cons constructor proper-ancestors))
  (values type-accessor constructor predicate accessor make-subtype))

(define (make-type payload) (%make-type '() payload))

))
