
(define-library (srfi 17)
  (import (rename (scheme base) (set! original-set!)) (only (skint) void))
  (export set! setter getter-with-setter)

(begin

;[esl] adapted from
; SRFI 17 reference implementation for Twobit, contributed
; by Lars Thomas Hansen <lth@ccs.neu.edu>

(define-syntax set!
  (syntax-rules ()
    ((set! (?e0 ?e1 ...) ?v)
     ((setter ?e0) ?e1 ... ?v))
    ((set! ?i ?v)
     (original-set! ?i ?v)))) ;[esl*]

(define setter 
  (let ((setters (list (cons car  set-car!)
                       (cons cdr  set-cdr!)
                       (cons caar (lambda (p v) (set-car! (car p) v)))
                       (cons cadr (lambda (p v) (set-car! (cdr p) v)))
                       (cons cdar (lambda (p v) (set-cdr! (car p) v)))
                       (cons cddr (lambda (p v) (set-cdr! (cdr p) v)))
                       (cons vector-ref vector-set!)
                       (cons string-ref string-set!))))
    (letrec ((setter
              (lambda (proc)
                (let ((probe (assv proc setters)))
                  (if probe
                      (cdr probe)
                      (error "No setter for " proc)))))
             (set-setter!
              (lambda (proc setter)
                (set! setters (cons (cons proc setter) setters))
                (void))))
      (set-setter! setter set-setter!)
      setter)))

; Contributed by Per Bothner, from the SRFI document.

(define (getter-with-setter get set)
  (let ((proc (lambda args (apply get args))))
    (set! (setter proc) set)
    proc)) 

))
