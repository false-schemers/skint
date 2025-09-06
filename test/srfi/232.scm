(import (srfi 232))
(include "test.scm")

;[esl] adapted from tests by Wolfgang Corcoran-Mathe

(test-begin "srfi-232")

;;; SPDX-License-Identifier: MIT
;;; SPDX-FileCopyrightText: 2024 Wolfgang Corcoran-Mathe

(test-eqv 5 ((curried (x y) (+ x y)) 2 3))
(test-eqv 5 (((curried (x y) (+ x y)) 2) 3))
(test-eqv 5 ((curried (w x y z) (+ w x y z)) 1 1 1 2))
(test-eqv 5 ((((curried (w x y z) (+ w x y z)) 1) 1) 1 2))
(test-eqv 5 (((curried (w x y z) (+ w x y z)) 1) 1 1 2))
(test-eqv 5 (((curried (w x y z) (+ w x y z)) 1 1) 1 2))
(test-eqv 5 (((curried (w x y z) (+ w x y z)) 1 1 1) 2))
(test-eqv 5 (((((curried (w x y z) (+ w x y z)) 1) 1) 1) 2))

(test-equal '(3 (3 4))
            ((curried (a b . rest) (list (+ a b) rest)) 1 2 3 4))
(test-equal
  '(3 (3 4))
  (((curried (a b . rest) (list (+ a b) rest)) 1) 2 3 4))
(test-equal '(3 ()) ((curried (a b . rest) (list (+ a b) rest)) 1 2))

(test-eqv 3 ((curried () (curried (x y) (+ x y))) 1 2))
(test-eqv 3 (((curried () (curried (x y) (+ x y))) 1) 2))

;; "... while these behaviors are decidedly not wrong, they are
;;  perhaps mildly unsettling."
(test-eqv 2
          ((curried (a)
              (curried ()
                (curried ()
                  (curried (b) b)))) 1 2))
(test-eqv 4 (((((((((curried (a b c) 4)))))))) 1 2 3))

(test-eqv 20 ((curried (x y) (curried (z) (* z (+ x y)))) 2 3 4))
(test-eqv 20 (((curried (x y) (curried (z) (* z (+ x y)))) 2) 3 4))

(test-end)

