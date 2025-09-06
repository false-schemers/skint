(import (srfi 137))
(include "test.scm")

(test-begin "srfi-137")

;[esl] adapted from ref. imp. test by John Cowan, Marc Nieper-Wißkirchen 
(test-equal 'reia
  (let-values
    (((reia-payload make-reia reia? reia-ref make-reia-subtype) (make-type 'reia)))
    (reia-payload)))

(test-assert
  (let-values 
    (((reia-payload1 . reia1*)  (make-type 'reia))
     ((reia-payload2 . reia2*)  (make-type 'reia)))
      (not (eq? reia-payload1 reia-payload2))))

(let*-values
  (((reia-payload
     make-reia
     reia?
     reia-ref
     make-reia-subtype)
    (make-type 'reia))
   ((daughter-payload
     make-daughter
     daughter?
     daughter-ref
     make-daughter-subtype)
    (make-reia-subtype 'daughter))
   ((son-payload
     make-son
     son?
     son-ref
     make-son-subtype)
    (make-reia-subtype 'son))
   ((grand-daughter-payload
     make-grand-daughter
     grand-daughter?
     grand-daughter-ref
     make-grand-daughter-subtype)
    (make-daughter-subtype 'grand-daughter)))
	(test-assert (reia? (make-reia #f)))
	(test-assert (reia? (make-daughter #f)))
	(test-assert (not (daughter? (make-reia #f))))
	(test-assert (not (son? (make-daughter #f))))
	(test-assert (reia? (make-grand-daughter #f))))
	   
(test-equal 'payload
  (let-values
    (((reia-payload
		 	 make-reia
			 reia?
			 reia-ref
			 make-reia-subtype)
			(make-type 'reia)))
	  (reia-ref (make-reia 'payload))))

(test-end)
