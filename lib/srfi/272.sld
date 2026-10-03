; SPDX-FileCopyrightText: 2026 Sergei Egorov
;
; SPDX-License-Identifier: MIT

; Bare-bones Pretty Printing library, for Skint
;
; (skint print) repackaged; see (srfi 272 intermediate), which this library
; is, with a single export.  Skint's pretty-print already meets this library's
; requirements -- object plus optional port, laid out within print-width, a
; terminating newline, quote and its relatives abbreviated, and cycles marked
; with datum labels rather than diverging.

(define-library (srfi 272)

  (import (srfi 272 intermediate))

  (export pp))
