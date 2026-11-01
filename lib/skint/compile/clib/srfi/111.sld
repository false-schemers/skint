;[esl] this file is for demo purposes only; the whole lib is built-in

(define-library (srfi 111)
  ;; DS: Skint's copy imports the box builtins from (skint); the
;; compiled world takes them from the (scheme box) shadow library
;; (same bindings, cover-macro form) instead.
(import (only (scheme box) box? box unbox set-box!))
  (export box? box unbox set-box!))
