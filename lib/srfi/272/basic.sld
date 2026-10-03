; SPDX-FileCopyrightText: 2026 Sergei Egorov
;
; SPDX-License-Identifier: MIT

; Basic Pretty Printing library, for Skint
;
; (skint print) repackaged; see (srfi 272 intermediate), which this library
; is, with narrower exports.

(define-library (srfi 272 basic)

  (import (srfi 272 intermediate))

  ; procedures
  (export pp pprint pprint-shared pprint-simple)

  ; parameters
  (export pp-width pp-circle pp-graph))
