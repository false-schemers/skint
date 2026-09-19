; SPDX-FileCopyrightText: 2026 Sergei Egorov
;
; SPDX-License-Identifier: MIT

; Show library integration

(define-library (srfi 272 show)
  (import
    (scheme base)
    (srfi 166 base)
    (srfi 272 advanced))

  (export
    pprinted pprinted*
    pp-width pp-circle pp-graph pp-radix pp-length
    pp-level pp-lines pp-indent pp-tab pp-max-tab pp-miser-width
    pp-inline-width pp-brackets pp-code pp-pretty pp-newline
    pp-color pp-emit pp-tint pp-decorate pp-styles pp-hooks)

  (begin

    (define (pprinted obj . kv*)
      (fn (width) ; default width is taken from show env
        (let* ((ppe (append kv* (list pp-width width)))
               (g (apply make-pprint-generator obj ppe)))
          (let lp ((s (g)))
            (if (eof-object? s)
                nothing
                (each s (fn () (lp (g)))))))))

    (define (pprinted* obj arg . args)
      (define (cons* arg . args)
        (let loop ((xs (cons arg args)))
          (if (null? (cdr xs))
              (car xs)
              (cons (car xs) (loop (cdr xs))))))
      (apply pprinted obj (apply cons* arg args)))

  ))
