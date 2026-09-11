(define-library (skint disassemble)
  (import (scheme base)
          (scheme write)
          (only (skint hidden) %closure-code-ref %procedure->inst-name))

  (export disassemble)

  (begin
    (define (print-disassembled vec)
      (vector-for-each
       (lambda (elem)
         (cond
          ((vector? elem)
           (display "{\n")
           (print-disassembled elem)
           (display "\n}\n"))
          (else
           (if (%procedure->inst-name elem)
               (display (%procedure->inst-name elem))
               (write elem))
           (newline))))
       vec))
    (define (disassemble proc)
      (let collect-code ((idx 0)
                         (acc '()))
        (let ((code (%closure-code-ref proc idx)))
          (if code
              (collect-code (+ idx 1)
                            (cons code acc))
              (begin
                (print-disassembled
                 (list->vector
                  (reverse acc)))
                (values))))))))
