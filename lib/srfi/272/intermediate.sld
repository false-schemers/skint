; SPDX-FileCopyrightText: 2026 Sergei Egorov
;
; SPDX-License-Identifier: MIT

; Intermediate Pretty Printing library, for Skint
;
; Skint's own pretty printer is (skint print), and the first three libraries
; of this SRFI are that printer repackaged.  pp is pretty-print, and every
; parameter this level has IS the corresponding print- parameter -- the same
; object, not a copy -- so setting either moves both and (skint print) follows
; this SRFI.  Since the keys of pp's keyword arguments are just parameter
; objects, any other (skint print) parameter can be handed to pp as well:
; print-brackets and print-cursor among them.
;
; (srfi 272) and (srfi 272 basic) are this library with narrower exports.
; The advanced and fancy libraries are a separate, portable implementation
; and share no bindings with these.
;
; pprint-file is the only piece that is not already in (skint print).

(define-library (srfi 272 intermediate)

  (import (scheme base) (scheme char) (scheme file) (scheme read) (scheme write))
  (import (skint print))

  ; procedures
  (export pp pp* pprint pprint-shared pprint-simple pprint-file)

  ; configuration
  (export pretty-style)

  ; parameters
  (export pp-width pp-circle pp-graph pp-radix pp-length pp-level)

  (begin

    ; remap: the same parameter objects skint's own printer uses
    (define pp-width print-width)
    (define pp-circle print-circle)
    (define pp-graph print-graph)
    (define pp-radix print-radix)
    (define pp-length print-length)
    (define pp-level print-level)

    ; obj, an optional port, then keyword arguments
    (define pp pretty-print)

    ; keyword arguments followed by a list of them
    (define (pp* obj arg . args)
      (define (cons* arg . args)
        (let loop ((xs (cons arg args)))
          (if (null? (cdr xs)) (car xs) (cons (car xs) (loop (cdr xs))))))
      (apply pp obj (apply cons* arg args)))

    (define (with-port rest k)
      (if (and (pair? rest) (output-port? (car rest)))
          (k (car rest) (cdr rest))
          (k (current-output-port) rest)))

    ; the hardwired pairs come first, so they win over the caller's
    (define (pprint-simple obj . rest)
      (with-port rest
        (lambda (port kv) (pp* obj port pp-graph #f pp-circle #f kv))))

    (define (pprint obj . rest)
      (with-port rest
        (lambda (port kv) (pp* obj port pp-graph #f pp-circle #t kv))))

    (define (pprint-shared obj . rest)
      (with-port rest
        (lambda (port kv) (pp* obj port pp-graph #t pp-circle #t kv))))

    ; reads input file, pretty-prints it to output file or current output
    ; top-level line comments are preserved
    (define (pprint-file ifn . opt-ofn)
      (define (skip-ws ip)
        (let ((c (peek-char ip)))
          (when
            (and (char? c) (not (char=? c #\newline)) (char-whitespace? c))
            (read-char ip)
            (skip-ws ip))))
      (define (copy-top-line-comments ip op postpp)
        (skip-ws ip)
        (let ((c (peek-char ip)))
          (cond ((and (char? c) (char=? c #\;))
                 (display (read-line ip) op)
                 (newline op)
                 (copy-top-line-comments ip op #f))
                ((and (char? c) (char=? c #\newline))
                 (read-char ip)
                 (unless postpp (newline op))
                 (copy-top-line-comments ip op #f)))))
      (define (pf ip op)
        (let loop ((postpp #f))
          (copy-top-line-comments ip op postpp)
          (let ((obj (read ip)))
            (unless (eof-object? obj) (pp obj op) (loop #t)))))
      (call-with-input-file ifn
        (lambda (ip)
          (if (null? opt-ofn)
              (pf ip (current-output-port))
              (call-with-output-file (car opt-ofn)
                (lambda (op) (pf ip op)))))))))
