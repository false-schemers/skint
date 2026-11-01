;;; cxr.sld -- (scheme cxr) as a real library, for compiled programs.
;;;
;;; Shadow counterpart of Skint's predefined (scheme cxr): the predefined
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
;;; (scheme cxr) owns all 28 names, depth 2 included;
;;; (scheme base) re-exports the depth-2 covers from here, so
;;; the two libraries share those bindings and programs
;;; importing both see no duplicate-binding conflict.  The
;;; syntax comes straight from (skint) to keep the dependency
;;; direction acyclic: base -> cxr -> skint.

(define-library (scheme cxr)
  (import (only (skint) define lambda define-syntax syntax-rules _ ...)
          (rename (only (skint) apply) (apply %builtin-apply))
          (rename (only (skint) caar cadr cdar cddr caaar caadr cadar caddr cdaar cdadr cddar cdddr caaaar caaadr caadar caaddr cadaar cadadr caddar cadddr cdaaar cdaadr cdadar cdaddr cddaar cddadr cdddar cddddr)
            (caar %builtin-caar)
            (cadr %builtin-cadr)
            (cdar %builtin-cdar)
            (cddr %builtin-cddr)
            (caaar %builtin-caaar)
            (caadr %builtin-caadr)
            (cadar %builtin-cadar)
            (caddr %builtin-caddr)
            (cdaar %builtin-cdaar)
            (cdadr %builtin-cdadr)
            (cddar %builtin-cddar)
            (cdddr %builtin-cdddr)
            (caaaar %builtin-caaaar)
            (caaadr %builtin-caaadr)
            (caadar %builtin-caadar)
            (caaddr %builtin-caaddr)
            (cadaar %builtin-cadaar)
            (cadadr %builtin-cadadr)
            (caddar %builtin-caddar)
            (cadddr %builtin-cadddr)
            (cdaaar %builtin-cdaaar)
            (cdaadr %builtin-cdaadr)
            (cdadar %builtin-cdadar)
            (cdaddr %builtin-cdaddr)
            (cddaar %builtin-cddaar)
            (cddadr %builtin-cddadr)
            (cdddar %builtin-cdddar)
            (cddddr %builtin-cddddr)))

  (export
    (rename %cover-caar caar)
    (rename %cover-cadr cadr)
    (rename %cover-cdar cdar)
    (rename %cover-cddr cddr)
    (rename %cover-caaar caaar)
    (rename %cover-caadr caadr)
    (rename %cover-cadar cadar)
    (rename %cover-caddr caddr)
    (rename %cover-cdaar cdaar)
    (rename %cover-cdadr cdadr)
    (rename %cover-cddar cddar)
    (rename %cover-cdddr cdddr)
    (rename %cover-caaaar caaaar)
    (rename %cover-caaadr caaadr)
    (rename %cover-caadar caadar)
    (rename %cover-caaddr caaddr)
    (rename %cover-cadaar cadaar)
    (rename %cover-cadadr cadadr)
    (rename %cover-caddar caddar)
    (rename %cover-cadddr cadddr)
    (rename %cover-cdaaar cdaaar)
    (rename %cover-cdaadr cdaadr)
    (rename %cover-cdadar cdadar)
    (rename %cover-cdaddr cdaddr)
    (rename %cover-cddaar cddaar)
    (rename %cover-cddadr cddadr)
    (rename %cover-cdddar cdddar)
    (rename %cover-cddddr cddddr))

  (begin
    (define-syntax %cover-caar
      (syntax-rules ()
        [(_ . args) (%builtin-caar . args)]
        [_ %residual-caar]))
    (define %residual-caar (lambda (a0) (%builtin-caar a0)))
    (define-syntax caar %cover-caar)
    (define-syntax %cover-cadr
      (syntax-rules ()
        [(_ . args) (%builtin-cadr . args)]
        [_ %residual-cadr]))
    (define %residual-cadr (lambda (a0) (%builtin-cadr a0)))
    (define-syntax cadr %cover-cadr)
    (define-syntax %cover-cdar
      (syntax-rules ()
        [(_ . args) (%builtin-cdar . args)]
        [_ %residual-cdar]))
    (define %residual-cdar (lambda (a0) (%builtin-cdar a0)))
    (define-syntax cdar %cover-cdar)
    (define-syntax %cover-cddr
      (syntax-rules ()
        [(_ . args) (%builtin-cddr . args)]
        [_ %residual-cddr]))
    (define %residual-cddr (lambda (a0) (%builtin-cddr a0)))
    (define-syntax cddr %cover-cddr)
    (define-syntax %cover-caaar
      (syntax-rules ()
        [(_ . args) (%builtin-caaar . args)]
        [_ %residual-caaar]))
    (define %residual-caaar (lambda (a0) (%builtin-caaar a0)))
    (define-syntax caaar %cover-caaar)
    (define-syntax %cover-caadr
      (syntax-rules ()
        [(_ . args) (%builtin-caadr . args)]
        [_ %residual-caadr]))
    (define %residual-caadr (lambda (a0) (%builtin-caadr a0)))
    (define-syntax caadr %cover-caadr)
    (define-syntax %cover-cadar
      (syntax-rules ()
        [(_ . args) (%builtin-cadar . args)]
        [_ %residual-cadar]))
    (define %residual-cadar (lambda (a0) (%builtin-cadar a0)))
    (define-syntax cadar %cover-cadar)
    (define-syntax %cover-caddr
      (syntax-rules ()
        [(_ . args) (%builtin-caddr . args)]
        [_ %residual-caddr]))
    (define %residual-caddr (lambda (a0) (%builtin-caddr a0)))
    (define-syntax caddr %cover-caddr)
    (define-syntax %cover-cdaar
      (syntax-rules ()
        [(_ . args) (%builtin-cdaar . args)]
        [_ %residual-cdaar]))
    (define %residual-cdaar (lambda (a0) (%builtin-cdaar a0)))
    (define-syntax cdaar %cover-cdaar)
    (define-syntax %cover-cdadr
      (syntax-rules ()
        [(_ . args) (%builtin-cdadr . args)]
        [_ %residual-cdadr]))
    (define %residual-cdadr (lambda (a0) (%builtin-cdadr a0)))
    (define-syntax cdadr %cover-cdadr)
    (define-syntax %cover-cddar
      (syntax-rules ()
        [(_ . args) (%builtin-cddar . args)]
        [_ %residual-cddar]))
    (define %residual-cddar (lambda (a0) (%builtin-cddar a0)))
    (define-syntax cddar %cover-cddar)
    (define-syntax %cover-cdddr
      (syntax-rules ()
        [(_ . args) (%builtin-cdddr . args)]
        [_ %residual-cdddr]))
    (define %residual-cdddr (lambda (a0) (%builtin-cdddr a0)))
    (define-syntax cdddr %cover-cdddr)
    (define-syntax %cover-caaaar
      (syntax-rules ()
        [(_ . args) (%builtin-caaaar . args)]
        [_ %residual-caaaar]))
    (define %residual-caaaar (lambda (a0) (%builtin-caaaar a0)))
    (define-syntax caaaar %cover-caaaar)
    (define-syntax %cover-caaadr
      (syntax-rules ()
        [(_ . args) (%builtin-caaadr . args)]
        [_ %residual-caaadr]))
    (define %residual-caaadr (lambda (a0) (%builtin-caaadr a0)))
    (define-syntax caaadr %cover-caaadr)
    (define-syntax %cover-caadar
      (syntax-rules ()
        [(_ . args) (%builtin-caadar . args)]
        [_ %residual-caadar]))
    (define %residual-caadar (lambda (a0) (%builtin-caadar a0)))
    (define-syntax caadar %cover-caadar)
    (define-syntax %cover-caaddr
      (syntax-rules ()
        [(_ . args) (%builtin-caaddr . args)]
        [_ %residual-caaddr]))
    (define %residual-caaddr (lambda (a0) (%builtin-caaddr a0)))
    (define-syntax caaddr %cover-caaddr)
    (define-syntax %cover-cadaar
      (syntax-rules ()
        [(_ . args) (%builtin-cadaar . args)]
        [_ %residual-cadaar]))
    (define %residual-cadaar (lambda (a0) (%builtin-cadaar a0)))
    (define-syntax cadaar %cover-cadaar)
    (define-syntax %cover-cadadr
      (syntax-rules ()
        [(_ . args) (%builtin-cadadr . args)]
        [_ %residual-cadadr]))
    (define %residual-cadadr (lambda (a0) (%builtin-cadadr a0)))
    (define-syntax cadadr %cover-cadadr)
    (define-syntax %cover-caddar
      (syntax-rules ()
        [(_ . args) (%builtin-caddar . args)]
        [_ %residual-caddar]))
    (define %residual-caddar (lambda (a0) (%builtin-caddar a0)))
    (define-syntax caddar %cover-caddar)
    (define-syntax %cover-cadddr
      (syntax-rules ()
        [(_ . args) (%builtin-cadddr . args)]
        [_ %residual-cadddr]))
    (define %residual-cadddr (lambda (a0) (%builtin-cadddr a0)))
    (define-syntax cadddr %cover-cadddr)
    (define-syntax %cover-cdaaar
      (syntax-rules ()
        [(_ . args) (%builtin-cdaaar . args)]
        [_ %residual-cdaaar]))
    (define %residual-cdaaar (lambda (a0) (%builtin-cdaaar a0)))
    (define-syntax cdaaar %cover-cdaaar)
    (define-syntax %cover-cdaadr
      (syntax-rules ()
        [(_ . args) (%builtin-cdaadr . args)]
        [_ %residual-cdaadr]))
    (define %residual-cdaadr (lambda (a0) (%builtin-cdaadr a0)))
    (define-syntax cdaadr %cover-cdaadr)
    (define-syntax %cover-cdadar
      (syntax-rules ()
        [(_ . args) (%builtin-cdadar . args)]
        [_ %residual-cdadar]))
    (define %residual-cdadar (lambda (a0) (%builtin-cdadar a0)))
    (define-syntax cdadar %cover-cdadar)
    (define-syntax %cover-cdaddr
      (syntax-rules ()
        [(_ . args) (%builtin-cdaddr . args)]
        [_ %residual-cdaddr]))
    (define %residual-cdaddr (lambda (a0) (%builtin-cdaddr a0)))
    (define-syntax cdaddr %cover-cdaddr)
    (define-syntax %cover-cddaar
      (syntax-rules ()
        [(_ . args) (%builtin-cddaar . args)]
        [_ %residual-cddaar]))
    (define %residual-cddaar (lambda (a0) (%builtin-cddaar a0)))
    (define-syntax cddaar %cover-cddaar)
    (define-syntax %cover-cddadr
      (syntax-rules ()
        [(_ . args) (%builtin-cddadr . args)]
        [_ %residual-cddadr]))
    (define %residual-cddadr (lambda (a0) (%builtin-cddadr a0)))
    (define-syntax cddadr %cover-cddadr)
    (define-syntax %cover-cdddar
      (syntax-rules ()
        [(_ . args) (%builtin-cdddar . args)]
        [_ %residual-cdddar]))
    (define %residual-cdddar (lambda (a0) (%builtin-cdddar a0)))
    (define-syntax cdddar %cover-cdddar)
    (define-syntax %cover-cddddr
      (syntax-rules ()
        [(_ . args) (%builtin-cddddr . args)]
        [_ %residual-cddddr]))
    (define %residual-cddddr (lambda (a0) (%builtin-cddddr a0)))
    (define-syntax cddddr %cover-cddddr)
))
