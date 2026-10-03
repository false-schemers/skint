(import (srfi 272 basic))
(import (scheme base) (scheme read) (scheme write))
(import (only (skint print) print-width print-circle print-graph pretty-print))
(include "../test.scm")

(test-begin "srfi-272-basic")

(define (printed proc obj . kv)
  (let ((p (open-output-string)))
    (apply proc obj p kv)
    (get-output-string p)))

;; ------------------------------------------------------------ the parameters

; this library is (skint print) repackaged, so its parameters are that
; printer's own: the same objects, not copies
(test #t (eq? pp-width print-width))
(test #t (eq? pp-circle print-circle))
(test #t (eq? pp-graph print-graph))

; and so a setting made either way is seen by both
(test 40 (parameterize ((pp-width 40)) (print-width)))
(test 55 (parameterize ((print-width 55)) (pp-width)))

(define wide '(let ((alpha 1) (beta 2) (gamma 3) (delta 4) (epsilon 5))
                (list alpha beta gamma delta epsilon)))

(test #t (string=? (printed pp wide pp-width 30)
                   (printed pretty-print wide print-width 30)))

;; -------------------------------------------------------- sharing and cycles

(define (shared-pair) (let ((y (list 1 2))) (list y y)))
(define (cyclic) (let ((c (list 1 2))) (set-cdr! (cdr c) c) c))

; the default is write's behaviour: cycles marked, plain sharing not
(test "((1 2) (1 2))\n" (printed pp (shared-pair)))
(test "#0=(1 2 . #0#)\n" (printed pp (cyclic)))

; pp-graph marks everything shared
(test "(#0=(1 2) #0#)\n" (printed pp (shared-pair) pp-graph #t))

; pp-circle off, pp-graph off: nothing is marked
(test "((1 2) (1 2))\n" (printed pp (shared-pair) pp-circle #f))

;; ------------------------------------------------------- the three shortcuts

; pprint is write, pprint-shared is write-shared, and neither consults the
; parameters: the hardwired values win
(test "((1 2) (1 2))\n" (printed pprint (shared-pair)))
(test "((1 2) (1 2))\n" (printed pprint (shared-pair) pp-graph #t))
(test "#0=(1 2 . #0#)\n" (printed pprint (cyclic)))
(test "(#0=(1 2) #0#)\n" (printed pprint-shared (shared-pair)))
(test "(#0=(1 2) #0#)\n" (printed pprint-shared (shared-pair) pp-graph #f))

; pprint-simple marks nothing; it is only safe on acyclic data
(test "((1 2) (1 2))\n" (printed pprint-simple (shared-pair)))
(test "((1 2) (1 2))\n" (printed pprint-simple (shared-pair) pp-graph #t))

; all three take the port and still end with a newline
(test "(a b)\n" (printed pprint '(a b)))
(test "(a b)\n" (printed pprint-shared '(a b)))
(test "(a b)\n" (printed pprint-simple '(a b)))

(test-end)
