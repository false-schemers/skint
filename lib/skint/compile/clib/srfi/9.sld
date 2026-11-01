;;; 9.sld -- (srfi 9) defining records, compiled-code version.
;;; SRFI 9 is a single syntax, define-record-type, which the
;;; shadow (scheme base) already provides with identical syntax
;;; and semantics; this library simply re-exports it so that
;;; libraries written against (srfi 9) (e.g. Skint's srfi 134)
;;; work unchanged.

(define-library (srfi 9)
  (import (scheme base))
  (export define-record-type))
