;; A SKETCH, not a finished library.  It exists to answer one question: can the
;; stack captured in a failure object be walked and printed frame by frame?
;;
;; On load it installs a handler into the current-exception-handler parameter,
;; keeping the previous value as the fall-back.  A failure object is walked and
;; printed, then handed on to the fall-back so that reporting and reset happen
;; as before; anything else goes straight to the fall-back.

(define-library (skint debug)

  (import (scheme base) (scheme write) (scheme case-lambda) (scheme cxr))
  (import (skint print))
  (import (only (skint) failure-object? failure-object-message
                        failure-object-irritants))
  (import (only (skint hidden) closure? closure->vector
                               current-exception-handler))

  (export failure-frames print-failure-frames
          install-failure-debugger! uninstall-failure-debugger!)

(begin

;; ---------------------------------------------------------------------------
;; Walking the captured stack
;; ---------------------------------------------------------------------------
;;
;; The failure object is a vector.  Slots [0 .. frames-end) are the captured vm
;; stack, OLDEST FIRST; above them sit the irritants, their count, and the
;; message.
;;
;; A call pushes exactly two words (i.c, save): the calling procedure, then
;; the offset at which it will resume inside its OWN code vector.  return
;; pops them in reverse.  Between two such pairs lie that frame working
;; values -- arguments being accumulated, saved temporaries.
;;
;; So a frame start is: a closure, followed by an exact non-negative fixnum
;; that is a legal offset into that closure own code vector.
;;
;; THIS IS A HEURISTIC, NOT A PARSE.  It never MISSES a real frame -- every
;; frame satisfies it by construction -- but it does report extra ones: a
;; procedure stored as DATA next to a small exact fixnum passes the same test.
;; Verified, not assumed: (boom my-proc 2 (list 1)) puts my-proc on the
;; stack beside a fixnum and the test accepts it.  Read a backtrace from here
;; as a superset of the truth.

(define (code-of p) (vector-ref (closure->vector p) 0))

(define (frame-start? v i)
  (and (< (+ i 1) (vector-length v))
       (closure? (vector-ref v i))
       (let ([n (vector-ref v (+ i 1))])
         (and (integer? n) (exact? n) (>= n 0)
              (<= n (vector-length (code-of (vector-ref v i))))))))

;; first slot that is no longer part of the captured stack
(define (frames-end obj)
  (- (vector-length obj) 2 (length (failure-object-irritants obj))))

(define (frame-starts obj end)
  (let loop ([i 0] [r '()])
    (cond [(>= i end) (reverse r)]
          [(frame-start? obj i) (loop (+ i 2) (cons i r))]
          [else (loop (+ i 1) r)])))

;; => a list of (procedure return-offset (working-value ...)), INNERMOST FIRST
(define (failure-frames obj)
  (let* ([end (frames-end obj)]
         [starts (frame-starts obj end)])
    (let loop ([ss starts] [r '()])
      (if (null? ss)
          r                                  ; consed while ascending => reversed
          (let* ([i (car ss)]
                 [next (if (null? (cdr ss)) end (cadr ss))]
                 [vals (let g ([j (+ i 2)] [a '()])
                         (if (>= j next) (reverse a)
                             (g (+ j 1) (cons (vector-ref obj j) a))))])
            (loop (cdr ss)
                  (cons (list (vector-ref obj i) (vector-ref obj (+ i 1)) vals)
                        r)))))))

;; ---------------------------------------------------------------------------
;; Printing
;; ---------------------------------------------------------------------------

;; everything is printed with length and level capped at 3, so one deep or long
;; value cannot bury the trace

(define (put x port) (print x port print-length 3 print-level 3))

(define print-failure-frames
  (case-lambda
    [(obj) (print-failure-frames obj (current-error-port))]
    [(obj port)
     (display "Failure: " port)
     (display (failure-object-message obj) port)
     (for-each (lambda (a) (display " " port) (put a port))
               (failure-object-irritants obj))
     (newline port)
     (let loop ([fs (failure-frames obj)] [n 0])
       (unless (null? fs)
         (let ([f (car fs)])
           (display "  " port) (display n port) (display ": " port)
           (put (car f) port)
           (display " @" port) (display (cadr f) port)
           (for-each (lambda (v) (display " " port) (put v port)) (caddr f))
           (newline port))
         (loop (cdr fs) (+ n 1))))]))

;; ---------------------------------------------------------------------------
;; The handler
;; ---------------------------------------------------------------------------
;;
;; A handler must answer (h) with its PARENT -- that is how raise finds the
;; next one up before calling this one -- so it is a case-lambda, like the
;; default handler it replaces.

(define previous-handler (current-exception-handler))

(define debug-handler
  (case-lambda
    [() previous-handler]
    [(obj)
     (when (failure-object? obj)
       (print-failure-frames obj (current-error-port)))
     (previous-handler obj)]))

(define (install-failure-debugger!) (current-exception-handler debug-handler))
(define (uninstall-failure-debugger!) (current-exception-handler previous-handler))

(install-failure-debugger!)

))
