(import (srfi 272 show))
(import (srfi 166))
(import (scheme base) (scheme write))
(include "../test.scm")

(test-begin "srfi-272-show")

(define (out fmt) (show #f fmt))

(define sexp '(let ((alpha 1) (beta 2) (gamma 3) (delta 4))
                (list alpha beta gamma delta)))

;; ------------------------------------------- pprinted is an ordinary formatter

; the datum is laid out, and the layout always ends with a newline
(test "(a b)\n" (out (pprinted '(a b))))
(test "42\n" (out (pprinted 42)))
(test "()\n" (out (pprinted '())))
(test "#(1 2 3)\n" (out (pprinted (vector 1 2 3))))
(test "'(a b)\n" (out (pprinted ''(a b))))

; it composes with the rest of SRFI 166, in sequence and in the middle of a line
(test "(a b)\n(c d)\n" (out (each (pprinted '(a b)) (pprinted '(c d)))))
(test "x = (a b)\n!" (out (each "x = " (pprinted '(a b)) "!")))

; the pp-produced newlines are real ones, so nl after a layout doubles up
(test "(a b)\n\n" (out (each (pprinted '(a b)) nl)))

;; ------------------------------------------------------------ width comes from
;; ---------------------------------------------------------- the show environment

(define narrow
  (string-append "(let\n ((alpha 1)\n  (beta 2)\n  (gamma 3)\n  (delta 4))\n"
                 " (list alpha beta\n  gamma delta))\n"))
(define wide
  (string-append "(let ((alpha 1) (beta 2) (gamma 3) (delta 4))\n"
                 "  (list alpha beta gamma delta))\n"))

(test narrow (out (with ((width 20)) (pprinted sexp))))
(test wide (out (with ((width 78)) (pprinted sexp))))

; and it wins over an explicit pp-width, both ways round: the spec has the show
; width override any explicitly or implicitly specified value of pp-width
(test wide (out (with ((width 78)) (pprinted sexp pp-width 20))))
(test narrow (out (with ((width 20)) (pprinted sexp pp-width 78))))

;; --------------------------------------------- every other keyword passes through

(test "(1 2 3 ...)\n" (out (pprinted '(1 2 3 4 5 6) pp-length 3)))
(test "(let ([a 1]) a)\n" (out (pprinted '(let ((a 1)) a) pp-code #t pp-brackets #t)))
(test "(#xff)\n" (out (pprinted '(255) pp-radix 16)))
(test "(a (b (c (...))))\n" (out (pprinted '(a (b (c (d)))) pp-level 3)))

; shared and cyclic structure is labelled, as it is for pp itself
(test "(#0=(1 2) #0#)\n"
      (let ((x (list 1 2))) (out (pprinted (list x x) pp-graph #t))))
(test "#0=(1 . #0#)\n"
      (let ((x (list 1 2))) (set-cdr! x x) (out (pprinted x))))

;; ------------------------------------------------------- pprinted* splices a list

(test "(1 2 ...)\n" (out (pprinted* '(1 2 3 4 5 6) (list pp-length 2))))
(test "(let ([a 1]) a)\n"
      (out (pprinted* '(let ((a 1)) a) pp-code #t (list pp-brackets #t))))
(test "(a b)\n" (out (pprinted* '(a b) '())))
(test wide (out (with ((width 78)) (pprinted* sexp (list pp-width 20)))))

;; ------------------------------------------- the parameters are re-exported here

(test #t (let loop ((l (list pp-width pp-circle pp-graph pp-radix pp-length
                             pp-level pp-lines pp-indent pp-tab pp-max-tab
                             pp-miser-width pp-inline-width pp-brackets pp-code
                             pp-pretty pp-newline pp-color pp-emit pp-tint
                             pp-decorate pp-styles pp-hooks)))
           (or (null? l) (and (procedure? (car l)) (loop (cdr l))))))

;; ------------------------------------------------- the example from the SRFI text

(define func
  '(define (fold kons knil ls)
     (let lp ((ls ls) (acc knil))
       (if (null? ls) acc (lp (cdr ls) (kons (car ls) acc))))))

(define doc
  (string-append
    "The fundamental list iterator.  Applies KONS to each "
    "element of LS and the result of the previous application, "
    "beginning with KNIL.  With KONS as CONS and KNIL as '(), "
    "equivalent to REVERSE."))

(test (string-append
        "(define (fold kons knil ls)           ; The    fundamental   list   iterator.\n"
        "  (let lp ([ls ls] [acc knil])        ; Applies   KONS  to  each  element  of\n"
        "    (if (null? ls)                    ; LS  and  the  result  of the previous\n"
        "        acc                           ; application,   beginning  with  KNIL.\n"
        "        (lp (cdr ls)                  ; With  KONS  as  CONS and KNIL as '(),\n"
        "            (kons (car ls) acc)))))   ; equivalent to REVERSE.\n")
      (out (columnar (pprinted func pp-brackets #t) " ; " (justified doc))))

(test-end)
