(import (scheme base) (scheme write) (scheme repl))
(import (skint apropos))
(import (skint print))
(import (only (skint hidden) evaluate-top-form))

(include "test.scm")

;; ---------------------------------------------------------------------------
;; Fixtures and helpers
;; ---------------------------------------------------------------------------

;; A library defined here, so that the tests have one whose exports they know
;; exactly.  Defining it at the top level is how a library reaches the system
;; without a .sld file, and apropos has to see it.
(define-library (demo apropos)
  (export demo-widget demo-gadget)
  (import (scheme base))
  (begin
    (define demo-widget 1)
    (define demo-gadget 2)
    ;; not exported: must never be reported
    (define demo-secret 3)))

(import (demo apropos))

;; the bare symbols at the front of an apropos-list result
(define (ids-of l)
  (if (and (pair? l) (symbol? (car l))) (cons (car l) (ids-of (cdr l))) '()))

;; the library rows after them
(define (rows-of l)
  (if (and (pair? l) (symbol? (car l))) (rows-of (cdr l)) l))

;; the row for one library, or #f
(define (row-of name l) (assoc name (rows-of l)))

;; is every element of L strictly less than the next under LESS?
(define (ascending? less? l)
  (or (null? l) (null? (cdr l))
      (and (less? (car l) (cadr l)) (ascending? less? (cdr l)))))

(define (sym<? a b) (string<? (symbol->string a) (symbol->string b)))

(define (fmt x) (let ([p (open-output-string)]) (write x p) (get-output-string p)))

;; what apropos prints, as a list of lines
(define (shown . args)
  (let ([p (open-output-string)])
    (parameterize ([current-output-port p]) (apply apropos args))
    (split-lines (get-output-string p))))

(define (split-lines str)
  (let loop ([i 0] [start 0] [acc '()])
    (cond [(= i (string-length str))
           (reverse (if (> i start) (cons (substring str start i) acc) acc))]
          [(char=? (string-ref str i) #\newline)
           (loop (+ i 1) (+ i 1) (cons (substring str start i) acc))]
          [else (loop (+ i 1) start acc)])))

(define (longest-line l)
  (let loop ([l l] [n 0])
    (if (null? l) n (loop (cdr l) (max n (string-length (car l)))))))

;; does every continuation line start with the two-space indent?
(define (indented? l)
  (or (null? l)
      (and (or (not (char=? (string-ref (car l) 0) #\space))
               (and (char=? (string-ref (car l) 0) #\space)
                    (char=? (string-ref (car l) 1) #\space)
                    (not (char=? (string-ref (car l) 2) #\space))))
           (indented? (cdr l)))))


(display "\n--- what apropos-list returns ---\n")

;; symbols for the environment, then one pair per library
(test '(exact-integer-sqrt) (ids-of (apropos-list 'exact-integer-sq)))
(test-assert (pair? (rows-of (apropos-list 'exact-integer-sq))))
(test-assert (row-of '(scheme base) (apropos-list 'exact-integer-sq)))

;; nothing matches: the empty list, and nothing printed
(test '() (apropos-list 'no-such-identifier-anywhere))
(test '() (shown 'no-such-identifier-anywhere))

;; a symbol and the string of its name select the same entries
(test (apropos-list 'exact-integer-sq) (apropos-list "exact-integer-sq"))
(test (apropos-list 'demo-) (apropos-list "demo-"))

;; the match is on a substring, anywhere in the name
(test-assert (memq 'exact-integer? (ids-of (apropos-list 'act-int))))
(test-assert (memq 'exact-integer? (ids-of (apropos-list "integer?"))))

;; and it is case sensitive
(test '() (apropos-list 'EXACT-INTEGER-SQ))

;; every known library shows up under the pattern that matches everything
(test-assert (row-of '(scheme base) (apropos-list "")))
(test-assert (row-of '(skint) (apropos-list "")))
(test-assert (row-of '(skint print) (apropos-list "")))
(test-assert (row-of '(demo apropos) (apropos-list "")))


(display "\n--- the environment part ---\n")

;; the default environment is the interaction environment
(test (apropos-list 'exact-integer-sq)
      (apropos-list 'exact-integer-sq (interaction-environment)))

;; identifiers bound there include syntax as well as variables
(test-assert (memq 'car (ids-of (apropos-list 'car))))
(test-assert (memq 'define-values (ids-of (apropos-list 'define-values))))

;; an explicit environment reports only what is bound in it
(test '(set-car!) (ids-of (apropos-list 'set-car! (scheme-report-environment 5))))
(test '() (ids-of (apropos-list 'set-car! (null-environment 5))))

;; the library rows do not depend on the environment
(test (rows-of (apropos-list 'set-car!))
      (rows-of (apropos-list 'set-car! (null-environment 5))))

;; the identifiers are sorted and carry no duplicates, though several libraries
;; export them
(test-assert (ascending? sym<? (ids-of (apropos-list 'vector-fill))))
(test-assert (ascending? sym<? (ids-of (apropos-list ""))))


(display "\n--- only what a library exports ---\n")

;; both exports of the demo library, and not its private definition
(test '((demo apropos) demo-gadget demo-widget) (row-of '(demo apropos) (apropos-list 'demo-)))
(test '() (apropos-list 'demo-secret))

;; the same for a library that came from a .sld file: print-width is exported,
;; print-code is a parameter it keeps to itself
(test '((skint print) print-width) (row-of '(skint print) (apropos-list 'print-width)))
(test '() (apropos-list 'print-code))

;; a library not yet loaded is not reported on at all
(test #f (row-of '(srfi 1) (apropos-list 'delete-duplicates)))

;; ids within a row are sorted
(test-assert (ascending? sym<? (cdr (row-of '(skint) (apropos-list 'vector)))))

;; rows come in library-name order, with a name that is a prefix of another
;; first -- (skint) before (skint hidden)
(parameterize ([apropos-expose-hidden #t])
  (test '((scheme base) (scheme r5rs) (skint) (skint hidden))
        (map car (rows-of (apropos-list 'vector-fill)))))
(test '((scheme base) (scheme r5rs) (skint))
      (map car (rows-of (apropos-list 'set-car!))))


(display "\n--- (skint hidden) is left out unless asked for ---\n")

;; The interpreter's own inside changes between releases and is not part of any
;; contract, so it is not reported by default.  `repl-read' is one of the names
;; the implementation explicitly assigns to (skint hidden) and to nothing else.
(test #f (apropos-expose-hidden))
(test '() (apropos-list 'repl-read))
(test '() (shown 'repl-read))

(parameterize ([apropos-expose-hidden #t])
  (test '((skint hidden) repl-read) (row-of '(skint hidden) (apropos-list 'repl-read)))
  (test '("(skint hidden):" "  repl-read") (shown 'repl-read)))

;; and the parameter is a parameter
(test #f (apropos-expose-hidden))
(test '() (apropos-list 'repl-read))

;; only that one library is affected; every other row is there either way
(test (rows-of (apropos-list 'exact-integer-sq))
      (parameterize ([apropos-expose-hidden #t])
        (rows-of (apropos-list 'exact-integer-sq))))
(test-assert (row-of '(skint print) (apropos-list 'print-width)))

;; Suppressing the row does not hide a binding that really is in the
;; environment.  This file imports one name from (skint hidden), and that name
;; is reported where it is bound however the parameter is set -- only the
;; library row is withheld.
(test '(evaluate-top-form) (ids-of (apropos-list 'evaluate-top)))
(test '() (rows-of (apropos-list 'evaluate-top)))
(parameterize ([apropos-expose-hidden #t])
  (test '(evaluate-top-form) (ids-of (apropos-list 'evaluate-top)))
  (test '((skint hidden) evaluate-top-form repl-evaluate-top-form)
        (row-of '(skint hidden) (apropos-list 'evaluate-top))))


(display "\n--- what apropos prints ---\n")

;; the environment heading, then one heading per library
(test '("interaction environment:"
        "  demo-gadget, demo-widget"
        "(demo apropos):"
        "  demo-gadget, demo-widget")
      (shown 'demo-))

;; the interaction environment is named as such however it was reached, and any
;; other environment is just an environment
(test "interaction environment:" (car (shown 'demo- (interaction-environment))))
(test-assert (member "environment:" (shown 'set-car! (scheme-report-environment 5))))

;; with nothing bound in the environment there is no heading for it
(test "((scheme base) set-car!)"
      (fmt (row-of '(scheme base) (apropos-list 'set-car! (null-environment 5)))))
(test-assert (not (member "environment:" (shown 'set-car! (null-environment 5)))))

;; long lists are filled and indented, and stay inside the width where a name
;; can be broken off
(test-assert (> (length (shown 'vector)) 8))
(test-assert (<= (longest-line (shown 'vector)) 78))
(test-assert (indented? (shown 'vector)))

;; a heading is written the way the library name reads
(test-assert (member "(skint print):" (shown 'print-width)))


(display "\n--- errors ---\n")

(test-error (apropos-list 42))
(test-error (apropos-list #t))
(test-error (apropos 42))
(test-error (apropos-list 'car 42))
(test-error (apropos 'car "not an environment"))

(test-end)
