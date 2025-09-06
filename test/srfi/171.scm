(import (srfi 1) (srfi 171))
(include "test.scm")

; [esl] poor man's compose for simple tests
; (srf1 171 contains the multi-arg/multi-val one, but does not export it for some reason)
(define compose
  (lambda (f g)
    (lambda args
      (f (apply g args)))))


; [esl] based on the original reference implementation tests

(test-begin "srfi-171")

(define (add1 x) (+ x 1))


(define numeric-list (iota 5))
(define numeric-vec (list->vector numeric-list))
(define string "0123456789abcdef")
(define list-of-chars (string->list string))
(define replace-alist '((1 . s) (2 . c) (3 . h) (4 . e) (5 . m)))
(define (replace-function val)
  (case val
    ((1) 's)
    ((2) 'c)
    ((3) 'h)
    ((4) 'e)
    ((5) 'm)
    (else val)))


(test-begin "transducers")
(test-equal '(1 2 3 4 5) (list-transduce (tmap add1) rcons numeric-list))
(test-equal '(0 2 4) (list-transduce (tfilter even?) rcons numeric-list))
(test-equal '(1 3 5) (list-transduce (compose (tfilter even?) (tmap add1)) rcons numeric-list))
(test-equal '(1 3 5) (list-transduce (tfilter-map
                                      (lambda (x) (if (even? x) (+ x 1) #f))) rcons numeric-list))

(test-equal (string-transduce (tmap char->integer) rcons string) (list-transduce (tmap char->integer) rcons list-of-chars))
(test-equal 6 (string-transduce (tfilter char-alphabetic?) rcount string))
(test-equal (list-transduce (tremove char-alphabetic?) rcount list-of-chars) (string-transduce (tremove char-alphabetic?) rcount string))
(test-equal '(s c h e m e  r o c k s) (list-transduce (treplace replace-alist) rcons '(1 2 3 4 5 4 r o c k s) ))
(test-equal '(s c h e m e  r o c k s) (list-transduce (treplace replace-function) rcons '(1 2 3 4 5 4 r o c k s) ))


(test-equal 6 (list-transduce (ttake 4) + numeric-list))
(test-equal 7 (list-transduce (tdrop 3) + numeric-list))

(test-equal '(3 4) (list-transduce (tdrop-while (lambda (x) (< x 3))) rcons numeric-list))

(test-equal '(0 1 2) (list-transduce (ttake-while (lambda (x) (< x 3))) rcons numeric-list))

(test-equal '(0 1 2 3 4) (list-transduce tconcatenate rcons '((0 1) (2 3) (4))))

(test-equal '(1 2 2 4 3 6) (list-transduce (tappend-map (lambda (x) (list x (* x 2)))) rcons '(1 2 3)))

(test-equal '(1 2 1 2 3) (list-transduce (tdelete-neighbor-duplicates) rcons '(1 1 1 2 2 1 2 3 3)))

(test-equal '(1 2 3 4) (list-transduce (tdelete-duplicates) rcons '(1 1 2 1 2 3 3 1 2 3 4 4)))

(test-equal '(1 2 3 4 5 6 7 8 9) (list-transduce tflatten rcons '((1 2) 3 (4 (5 6) 7) 8 (9))))

(test-equal '((1 1 1 1) (2 2 2 2) (3 3 3) (4 4 4 4)) (list-transduce (tpartition even?) rcons '(1 1 1 1 2 2 2 2 3 3 3 4 4 4 4)))

(test-equal '((0 1) (2 3) (4)) (vector-transduce (tsegment 2) rcons numeric-vec))

(test-equal '(0 and 1 and 2 and 3 and 4) (list-transduce (tadd-between 'and) rcons numeric-list))

(test-equal '((-1 . 0) (0 . 1) (1 . 2) (2 . 3) (3 . 4)) (list-transduce (tenumerate (- 1)) rcons numeric-list))

(test-end "transducers")



(test-begin "x-transduce")
(test-equal 15 (list-transduce (tmap add1) + numeric-list))
(test-equal 15 (list-transduce (tmap add1) + 0 numeric-list))

(test-equal 15 (vector-transduce (tmap add1) + numeric-vec))
(test-equal 15 (vector-transduce (tmap add1) + 0 numeric-vec))

;; This should really close it's port. I know.
(test-equal 15 (port-transduce (tmap add1) + read (open-input-string "0 1 2 3 4")))
(test-equal 15 (port-transduce (tmap add1) + 0 read (open-input-string "0 1 2 3 4")))

;; Converts each numeric char to it's corresponding number (+ 1) and sums them.
(test-equal 15 (string-transduce  (tmap (lambda (x) (- (char->integer x) 47))) + "01234"))
(test-equal 15 (string-transduce  (tmap (lambda (x) (- (char->integer x) 47))) + 0 "01234"))

(test-equal '(1 2 3) (parameterize ((current-input-port (open-input-string "1 2 3")))
                       (generator-transduce (tmap (lambda (x) x)) rcons read)))

(test-equal '(1 2 3) (parameterize ((current-input-port (open-input-string "1 2 3")))
                       (generator-transduce (tmap (lambda (x) x)) rcons '() read)))

(test-end)

