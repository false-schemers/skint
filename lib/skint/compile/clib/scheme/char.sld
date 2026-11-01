;;; char.sld -- (scheme char) as a real library, for compiled programs.
;;;
;;; Shadow counterpart of Skint's predefined (scheme char): the predefined
;;; library exports bare bindings to Skint-initialized globals, which a
;;; standalone executable does not have.  Here every procedure name is
;;; either
;;;   * a cover macro: a call in matching position expands to the Skint
;;;     builtin (imported renamed as %builtin-<name>), a bare identifier
;;;     expands to %residual-<name>, an ordinary definition in this
;;;     library (so the compiled program owns a real closure); or
;;;   * a definition copied from Skint's pre/s.scm (the macro/procedure
;;;     library the interpreter itself is built from), adapted only to
;;;     refer to this library's own bindings; or
;;;   * core syntax / an auxiliary identifier re-exported unchanged --
;;;     expansion-time machinery, not a runtime global.
;;; Only builtin denotations are taken from Skint, via (skint) and, for
;;; %-prefixed builtin operations (skint) does not export, (skint hidden).

(define-library (scheme char)
  (import (only (skint) define lambda define-syntax syntax-rules if _ ...)
          (rename (only (skint) apply) (apply %builtin-apply))
          (only (scheme base) string-map and or let null? car cdr
            char=? char<? char>? char<=? char>=?
            string=? string<? string>? string<=? string>=?)
          (rename (only (skint) digit-value char-whitespace? char-upper-case? char-upcase char-numeric? char-lower-case? char-foldcase char-downcase char-alphabetic?)
            (digit-value %builtin-digit-value)
            (char-whitespace? %builtin-char-whitespace?)
            (char-upper-case? %builtin-char-upper-case?)
            (char-upcase %builtin-char-upcase)
            (char-numeric? %builtin-char-numeric?)
            (char-lower-case? %builtin-char-lower-case?)
            (char-foldcase %builtin-char-foldcase)
            (char-downcase %builtin-char-downcase)
            (char-alphabetic? %builtin-char-alphabetic?)))

  (export
    string-upcase
    string-foldcase
    string-downcase
    string-ci>?
    string-ci>=?
    string-ci=?
    string-ci<?
    string-ci<=?
    (rename %cover-digit-value digit-value)
    (rename %cover-char-whitespace? char-whitespace?)
    (rename %cover-char-upper-case? char-upper-case?)
    (rename %cover-char-upcase char-upcase)
    (rename %cover-char-numeric? char-numeric?)
    (rename %cover-char-lower-case? char-lower-case?)
    (rename %cover-char-foldcase char-foldcase)
    (rename %cover-char-downcase char-downcase)
    char-ci>?
    char-ci>=?
    char-ci=?
    char-ci<?
    char-ci<=?
    (rename %cover-char-alphabetic? char-alphabetic?))

  (begin
    ;; string-upcase has no compiled-runtime primitive; it maps
    ;; char-upcase over the string ((scheme base)'s string-map).
    (define (string-upcase s) (string-map char-upcase s))
    ;; string-foldcase has no compiled-runtime primitive; it maps
    ;; char-foldcase over the string ((scheme base)'s string-map).
    (define (string-foldcase s) (string-map char-foldcase s))
    ;; string-downcase has no compiled-runtime primitive; it maps
    ;; char-downcase over the string ((scheme base)'s string-map).
    (define (string-downcase s) (string-map char-downcase s))
    ;; string-ci>? has no compiled-runtime primitive; it is
    ;; string>? over string-foldcaseed arguments.
    (define (string-ci>? a b . rest)
      (let loop ([a a] [b b] [rest rest])
        (and (string>? (string-foldcase a) (string-foldcase b))
             (or (null? rest) (loop b (car rest) (cdr rest))))))
    ;; string-ci>=? has no compiled-runtime primitive; it is
    ;; string>=? over string-foldcaseed arguments.
    (define (string-ci>=? a b . rest)
      (let loop ([a a] [b b] [rest rest])
        (and (string>=? (string-foldcase a) (string-foldcase b))
             (or (null? rest) (loop b (car rest) (cdr rest))))))
    ;; string-ci=? has no compiled-runtime primitive; it is
    ;; string=? over string-foldcaseed arguments.
    (define (string-ci=? a b . rest)
      (let loop ([a a] [b b] [rest rest])
        (and (string=? (string-foldcase a) (string-foldcase b))
             (or (null? rest) (loop b (car rest) (cdr rest))))))
    ;; string-ci<? has no compiled-runtime primitive; it is
    ;; string<? over string-foldcaseed arguments.
    (define (string-ci<? a b . rest)
      (let loop ([a a] [b b] [rest rest])
        (and (string<? (string-foldcase a) (string-foldcase b))
             (or (null? rest) (loop b (car rest) (cdr rest))))))
    ;; string-ci<=? has no compiled-runtime primitive; it is
    ;; string<=? over string-foldcaseed arguments.
    (define (string-ci<=? a b . rest)
      (let loop ([a a] [b b] [rest rest])
        (and (string<=? (string-foldcase a) (string-foldcase b))
             (or (null? rest) (loop b (car rest) (cdr rest))))))
    (define-syntax %cover-digit-value
      (syntax-rules ()
        [(_ . args) (%builtin-digit-value . args)]
        [_ %residual-digit-value]))
    (define %residual-digit-value (lambda (a0) (%builtin-digit-value a0)))
    (define-syntax digit-value %cover-digit-value)
    (define-syntax %cover-char-whitespace?
      (syntax-rules ()
        [(_ . args) (%builtin-char-whitespace? . args)]
        [_ %residual-char-whitespace?]))
    (define %residual-char-whitespace? (lambda (a0) (%builtin-char-whitespace? a0)))
    (define-syntax char-whitespace? %cover-char-whitespace?)
    (define-syntax %cover-char-upper-case?
      (syntax-rules ()
        [(_ . args) (%builtin-char-upper-case? . args)]
        [_ %residual-char-upper-case?]))
    (define %residual-char-upper-case? (lambda (a0) (%builtin-char-upper-case? a0)))
    (define-syntax char-upper-case? %cover-char-upper-case?)
    (define-syntax %cover-char-upcase
      (syntax-rules ()
        [(_ . args) (%builtin-char-upcase . args)]
        [_ %residual-char-upcase]))
    (define %residual-char-upcase (lambda (a0) (%builtin-char-upcase a0)))
    (define-syntax char-upcase %cover-char-upcase)
    (define-syntax %cover-char-numeric?
      (syntax-rules ()
        [(_ . args) (%builtin-char-numeric? . args)]
        [_ %residual-char-numeric?]))
    (define %residual-char-numeric? (lambda (a0) (%builtin-char-numeric? a0)))
    (define-syntax char-numeric? %cover-char-numeric?)
    (define-syntax %cover-char-lower-case?
      (syntax-rules ()
        [(_ . args) (%builtin-char-lower-case? . args)]
        [_ %residual-char-lower-case?]))
    (define %residual-char-lower-case? (lambda (a0) (%builtin-char-lower-case? a0)))
    (define-syntax char-lower-case? %cover-char-lower-case?)
    (define-syntax %cover-char-foldcase
      (syntax-rules ()
        [(_ . args) (%builtin-char-foldcase . args)]
        [_ %residual-char-foldcase]))
    (define %residual-char-foldcase (lambda (a0) (%builtin-char-foldcase a0)))
    (define-syntax char-foldcase %cover-char-foldcase)
    (define-syntax %cover-char-downcase
      (syntax-rules ()
        [(_ . args) (%builtin-char-downcase . args)]
        [_ %residual-char-downcase]))
    (define %residual-char-downcase (lambda (a0) (%builtin-char-downcase a0)))
    (define-syntax char-downcase %cover-char-downcase)
    ;; char-ci>? has no compiled-runtime primitive; it is
    ;; char>? over char-foldcaseed arguments.
    (define (char-ci>? a b . rest)
      (let loop ([a a] [b b] [rest rest])
        (and (char>? (char-foldcase a) (char-foldcase b))
             (or (null? rest) (loop b (car rest) (cdr rest))))))
    ;; char-ci>=? has no compiled-runtime primitive; it is
    ;; char>=? over char-foldcaseed arguments.
    (define (char-ci>=? a b . rest)
      (let loop ([a a] [b b] [rest rest])
        (and (char>=? (char-foldcase a) (char-foldcase b))
             (or (null? rest) (loop b (car rest) (cdr rest))))))
    ;; char-ci=? has no compiled-runtime primitive; it is
    ;; char=? over char-foldcaseed arguments.
    (define (char-ci=? a b . rest)
      (let loop ([a a] [b b] [rest rest])
        (and (char=? (char-foldcase a) (char-foldcase b))
             (or (null? rest) (loop b (car rest) (cdr rest))))))
    ;; char-ci<? has no compiled-runtime primitive; it is
    ;; char<? over char-foldcaseed arguments.
    (define (char-ci<? a b . rest)
      (let loop ([a a] [b b] [rest rest])
        (and (char<? (char-foldcase a) (char-foldcase b))
             (or (null? rest) (loop b (car rest) (cdr rest))))))
    ;; char-ci<=? has no compiled-runtime primitive; it is
    ;; char<=? over char-foldcaseed arguments.
    (define (char-ci<=? a b . rest)
      (let loop ([a a] [b b] [rest rest])
        (and (char<=? (char-foldcase a) (char-foldcase b))
             (or (null? rest) (loop b (car rest) (cdr rest))))))
    (define-syntax %cover-char-alphabetic?
      (syntax-rules ()
        [(_ . args) (%builtin-char-alphabetic? . args)]
        [_ %residual-char-alphabetic?]))
    (define %residual-char-alphabetic? (lambda (a0) (%builtin-char-alphabetic? a0)))
    (define-syntax char-alphabetic? %cover-char-alphabetic?)
))
