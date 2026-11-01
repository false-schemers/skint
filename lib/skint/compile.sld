;;; (skint compile) -- the whole-program compiler: Scheme to MARV.
;;;
;;; (compile-program INPUT [OUTPUT] PARAM VAL ...)
;;;
;;;   INPUT   a filename, an input port, or a list of forms.
;;;   OUTPUT  a filename, an output port, #t (the current output port,
;;;           the default), or #f (return the module as an
;;;           S-expression instead of writing it).
;;;   PARAM VAL ...  parameters and their values.  An explicit pair
;;;           overrides the parameter's current value, so
;;;             (compile-program "foo.scm" cp-optimize 2)
;;;           and
;;;             (parameterize ((cp-optimize 2)) (compile-program "foo.scm"))
;;;           mean the same thing.
;;;
;;;   cp-optimize   the optimization level, an exact integer 0..3,
;;;                 default 1.  Level 1 is what the compiler does; the
;;;                 other levels are accepted and validated but not yet
;;;                 interpreted.
;;;
;;; INPUT and OUTPUT filenames are used exactly as given, so they mean
;;; what they would to any other tool: the working directory decides.
;;; The compiler's own files -- srt/ and clib/ under compile/ -- are
;;; found from this library's own path instead, via find-library-path of
;;; its own name, so nothing depends on the working directory or on the
;;; caller's -I.
;;;
;;; The emitted MARV module is named $ds$program, deliberately not after
;;; the program: MARV's own libraries are unprefixed (math, string,
;;; stdio, time, ...), so a program called math.scm would otherwise have
;;; collided with one of them.
;;;
;;; Compiled code is linked against clib/, whose (scheme ...) and (srfi
;;; ...) libraries are NOT Skint's: they describe the world a standalone
;;; MARV executable provides -- towerless arithmetic, (scheme complex)
;;; as stubs, no exception system.  The shadow environment in
;;; cups-front.scm resolves those names against clib/ by name, so the
;;; library path is neither consulted nor modified and the host's own
;;; libraries are untouched.
;;;
;;; A compiled program reports and exits 1 on a non-recoverable error
;;; (the car of a number, a vector index out of range).  The compiler's
;;; own failure on a bad input program raises through error, which is
;;; catchable.  [cco]

(define-library (skint compile)
  ;; The sources were written as a top-level program, where the REPL's
  ;; environment already supplied the standard bindings and (skint
  ;; hidden) only added to them.  A library body has only what it
  ;; declares, so the base language has to be named too.  [cco]
  ;; Named imports only.  (skint hidden) is the interpreter.s inside, so
  ;; importing it whole would tie this library to every future change in
  ;; it; naming what is used also means the two utilities the compiler
  ;; defines itself -- posq and string-append* -- simply are not
  ;; imported, with no (except ...) needed.
  (import (scheme base) (scheme write) (scheme read) (scheme char)
          (scheme file) (scheme inexact) (scheme cxr)
          (scheme process-context)
          ;; (skint) is the RUNTIME library plus the core forms; (skint
          ;; hidden) is the expander.  The split is not cosmetic: a name
          ;; taken from the wrong one is silently accepted and resolves to
          ;; an unbound global of this library's own, which fails only when
          ;; the reference is reached.
          ;;
          ;; string->id and id->string are here because the computed
          ;; identifier template escapes in define-variant, (... string->id
          ;; ...), are matched by NAME: without those bindings in scope the
          ;; escape is not recognised.  body is a core form, which a library
          ;; body does not have unless it says so.  [cco]
          (only (skint) string->id id->string body
            void list* list-head string-position directory-separator)
          (only (skint hidden)
            ;; the expander
            xpand xpand-call xpand-define-library xpand-import
            xpand-integrable xpand-labels
            preprocess-define preprocess-define-syntax
            preprocess-mixed-forms preprocess-cond-expand
            top-defined-id-lookup xenv-lookup flatten-idslist
            make-controlled-environment make-sld-environment
            ;; names, locations and libraries
            *root-name-registry* make-location location-val
            location-set-val! location?
            val-library? val-transformer? val-integrable? val-builtin?
            library-exports library-code library-available?
            integrable? integrable-global lookup-integrable
            ;; libraries on disk
            fetch-library find-library-path listname->path
            ;; paths and the current file
            path-directory path-strip-directory path-strip-extension
            path-relative? base-path-separator
            dir-add-separator file-resolve-relative-to-base-path
            current-file with-current-file call-with-current-input-file
            ;; record-case is a MACRO, and an unimported macro name does
            ;; not fail at load: Skint resolves globals at decode time, so
            ;; (record-case x [quote (obj) ...] ...) was read as a CALL whose
            ;; second argument is a quote form with two operands -- which is
            ;; what "improper quote form" was reporting.  [cco]
            record-case
            ;; reading, writing and reporting
            read-code-sexp read-file-sexps sexp-match? symbol-append
            gensym features x-error c-error
            andmap ormap list1? list2+?
            ))
  (export compile-program cp-optimize)
  (include "compile/cups-base.scm")
  (include "compile/cups-signatures.scm")
  (include "compile/cups-passes.scm")
  (include "compile/cups-front.scm")
  (include "compile/cups-dsmarv.scm"))
