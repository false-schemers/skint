(import (scheme base))
(import (skint debug))
(import (only (skint) current-debugger))

(include "test.scm")

;; The debugger reads commands from a terminal and walks a real error's stack,
;; which a test script can do little with; it is checked by hand, in terminal
;; sessions.  What is checked here is that the library loads, that what it
;; exports is there, and that loading it installs the debugger.

(display "\n--- exports ---\n")

(test-assert (procedure? failure-frames))
(test-assert (procedure? continuation-frames))
(test-assert (procedure? print-failure-frames))
(test-assert (procedure? print-frames))
(test-assert (procedure? debugger))

(display "\n--- loading installs the debugger ---\n")

(test-assert (eq? debugger (current-debugger)))

(display "\n--- with nothing to show ---\n")

(define (err-output thunk)
  (let ([p (open-output-string)])
    (parameterize ([current-error-port p]) (thunk))
    (get-output-string p)))
(define (output thunk)
  (let ([p (open-output-string)]) (thunk p) (get-output-string p)))

;; no error: a message, and no command loop
(test "No error to debug.\n" (err-output (lambda () (debugger #f))))
(test "No error to debug.\n" (err-output (lambda () (debugger))))

;; no frames: nothing printed
(test "" (output (lambda (p) (print-frames '() p))))

(test-end)
