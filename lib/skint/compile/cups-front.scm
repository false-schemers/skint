;;; cups-front.scm -- CUPS part 4: whole-program expansion front end.
;;; Reuses Skint's macro/library engine ((skint hidden): xpand,
;;; preprocess-mixed-forms, make-controlled-environment, ...) exactly
;;; as compile-x.txt's compile-file does, but collects ALL top-level
;;; <core> forms of a program (including imported library code, which
;;; arrives as (once ...) wrapped init code) into one (begin ...) core
;;; for whole-program parse-core.  Library (once ...) deduplication
;;; then happens statically in parse-core.

;;; --- shadow standard libraries ----------------------------------------
;;; (scheme xxx) libraries come predefined in Skint's root registry as
;;; references to Skint-initialized globals.  That is fine for the
;;; interpreter, but a compiled program must be a fully independent
;;; executable, so imports of those libraries must not resolve there.
;;; For the libraries listed in *shadow-library-names*, imports are
;;; instead served from real .sld sources under lib/ (e.g.
;;; lib/scheme/cxr.sld): such a library imports each builtin renamed,
;;; defines a residual global for each name, and exports a cover macro
;;; under the original name, so call-position uses still expand to the
;;; builtin while value-position uses denote the library's own
;;; definitions.  make-sld-environment consults the root registry before
;;; fetch-library, so merely putting the .sld on the library path would
;;; not shadow the predefined library -- hence this derived environment,
;;; which intercepts exactly the shadowed listnames and delegates
;;; everything else.  (No (scheme eval): it is the whole interpreter and
;;; library manager, hence out of scope for a compiler.)
;;;
;;; The same interception serves compiled-code-specific SRFI libraries.
;;; They live at the paths their names denote under the compiler's own
;;; clib/ -- clib/srfi/N.sld, clib/skint/ivset.sld -- exactly like the
;;; (scheme xxx) libraries at clib/scheme/*.sld, and are fetched from
;;; there by path rather than by searching the library path.
;;;
;;; A shadowed name with no file under clib/ is BORROWED: the host's own
;;; source for it is re-expanded against this environment, which is
;;; enough for a library that is portable Scheme over the shadow
;;; (scheme base).  Most of the names below have no clib/ file and rely
;;; on exactly that.  It is also a trap: a REWRITTEN shadow library
;;; that is missing from clib/ is borrowed just as quietly, and the
;;; program then compiles against the interpreter's world with nothing
;;; said.  (skint fl), (skint fx) and (skint ivset) were packaged late
;;; for exactly that reason; the only thing that caught it was a
;;; byte-comparison against the command-line compiler's output.  [cco]
;;; This matters because Skint's own (srfi N) targets the interpreter:
;;; it may use builtins the MARV srt runtime does not support and host
;;; feature assumptions a standalone executable cannot rely on.  The
;;; shadow copies contain only portable Scheme written against the
;;; shadow libraries, plus (where a builtin has no portable definition)
;;; Skint builtins dressed as cover macros/residuals exactly like the
;;; (scheme xxx) libraries -- restricted to builtins srt supports.

(define *shadow-library-names*
  '((scheme base) (scheme case-lambda) (scheme char)
    (scheme cxr) (scheme file) (scheme inexact) (scheme lazy)
    (scheme process-context) (scheme r5rs)
    (scheme complex) (scheme r5rs-null) (scheme read) (scheme time)
    (scheme write) (scheme box)
    ;; compiled-code-specific SRFIs: lib/srfi/N.sld
    (srfi 1) (srfi 8) (srfi 26) (srfi 31) (srfi 41) (srfi 61)
    (srfi 9) (srfi 63) (srfi 95) (srfi 98) (srfi 111) (srfi 117)
    (srfi 127) (srfi 132) (srfi 133) (srfi 134)
    (srfi 141) (srfi 158) (srfi 14) (srfi 219) (srfi 69)
    (srfi 60)
    ;; internal helper libraries served to shadow SRFIs:
    (skint ivset) (skint fx) (skint fl)
    (srfi 143)
    (srfi 128) (srfi 116) (srfi 125) (srfi 113)
    (srfi 2) (srfi 121) (srfi 145) (srfi 146)
    ;; wave of dependency-satisfied borrows (Checkpoint 34);
    ;; (srfi 160) itself is a metadata stub, the real libraries
    ;; are its sublibraries (c64/c128 excluded: complex vectors)
    (srfi 4)
    (srfi 5) (srfi 6) (srfi 11) (srfi 16) (srfi 22) (srfi 23)
    (srfi 25) (srfi 30) (srfi 34) (srfi 35) (srfi 37) (srfi 38)
    (srfi 39) (srfi 42) (srfi 45) (srfi 62) (srfi 64) (srfi 71)
    (srfi 86) (srfi 156) (srfi 160) (srfi 173) (srfi 175)
    (srfi 189) (srfi 196) (srfi 197) (srfi 214) (srfi 217)
    (srfi 221) (srfi 222) (srfi 223) (srfi 227) (srfi 232)
    (srfi 235) (srfi 236) (srfi 239) (srfi 244) (srfi 251)
    (srfi 253) (srfi 257)
    (srfi 160 base) (srfi 160 f32) (srfi 160 f64) (srfi 160 s8)
    (srfi 160 s16) (srfi 160 u8) (srfi 160 u16)
    (srfi 227 definition)
    (srfi 257 box) (srfi 257 misc)))

(define *dropped-library-names*
  ;; not provided at all: (scheme load) and (scheme repl) are
  ;; meaningless without (scheme eval), which is out of scope for
  ;; a compiler.  Imports of these must fail as unprovided -- they used
  ;; to fall through to Skint's predefined libraries and be silently
  ;; served by the host, which no compiled program may rely on.
  ;;
  ;; (scheme complex) was dropped here too, which cost more than it
  ;; saved: (srfi 128) imports it for real-part and imag-part, so every
  ;; program reaching a comparator failed to compile.  It is now a
  ;; shadow library over reals, which is all a runtime without complex
  ;; numbers can answer, and the two constructors that could only answer
  ;; with a complex number report instead of guessing.  [cco]
  ;;
  ;; (srfi 257 rx) imports (srfi 264), which is in neither list, so
  ;; it is served by the base environment: its own (scheme base) is the
  ;; HOST's predefined one, and a compiled program would be linked
  ;; against the interpreter's world through it.  Dropped until (srfi
  ;; 264) is given a place of its own; (srfi 257), (srfi 257 box) and
  ;; (srfi 257 misc) do not reach it and stay. [cco]
  ;;
  ;; Eight more that a compiled program cannot use, each verified by
  ;; compiling (import (scheme base) <lib>) for every shadowed name.
  ;; Their sources are parked under save/compiler/clib-deferred/, whose
  ;; README says what each one needs; dropping them means an import
  ;; fails as unprovided instead of compiling against something that is
  ;; not there.
  ;;
  ;;   (srfi 151)       its towerless cond-expand branch imports (skint)
  ;;                    WHOLESALE, which collides with clib's own
  ;;                    (scheme base) on `*`
  ;;   (srfi 115)       imports (srfi 151), so it inherits that
  ;;   (srfi 146 hash)  the same collision through its own chain
  ;;   (srfi 144)       wants even? as a DS prim
  ;;   (srfi 160 s32/s64/u32/u64)
  ;;                    s32vector? and friends are no DS prim: Skint
  ;;                    builds those four numvector types only with
  ;;                    OPT_TOWER, so srt has nothing to lower them to.
  ;;                    base/f32/f64/s8/s16/u8/u16 compile and stay.
  ;; [cco]
  '((scheme load) (scheme repl) (srfi 257 rx)
    (srfi 115) (srfi 144) (srfi 151) (srfi 146 hash)
    (srfi 160 s32) (srfi 160 s64) (srfi 160 u32) (srfi 160 u64)))

;;; ---------------------------------------------------------------
;;; where the compiler's own files are
;;; ---------------------------------------------------------------
;;;
;;; Found from the library's OWN NAME.  (skint compile) must be
;;; mappable to a path -- import could not work otherwise -- and
;;; find-library-path is that mapping, so it answers the .sld this code
;;; was loaded from.  Stripping the extension names the directory
;;; beside it, which is where srt/ and clib/ live.
;;;
;;; This is better than reading (current-file) at load time: that is a
;;; dynamic stack, valid only while the file is being read, so it would
;;; have to be captured during expansion and carried.  A name can be
;;; resolved whenever it is wanted.  [cco]

(define (cp-library-name) '(skint compile))

(define (cp-base-path)
  (let ([sld (find-library-path (cp-library-name))])
    (if sld
        (dir-add-separator (path-strip-extension sld))
        ;; Not installed as a library yet: the experiment's own layout,
        ;; where the caller's -I supplies everything.
        "")))

(define (cp-resource name)
  (file-resolve-relative-to-base-path name (cp-base-path)))

;; Fetch a library from a KNOWN path, rather than by searching the
;; library path.  This is what fetch-library does once it has resolved a
;; name, in the same order with the same pieces; what it does not offer
;; is a way to say where to look, and the shadow environment knows that
;; exactly -- the names it serves live under clib/, by Skint's own
;; name-to-path mapping.  So nothing is appended or prepended to
;; *library-directory-list*: a lookup that reaches this environment is
;; answered from the compiler's own tree, and every other lookup is
;; delegated untouched.  [cco]
(define (cp-fetch-library-at listname filepath sld-env)
  (with-current-file filepath
    (lambda ()
      (let ([sexps (read-file-sexps filepath #f)])
        (unless (sexp-match? '((define-library * * ...)) sexps)
          (error "shadow library: unexpected forms in .sld file"
                 listname filepath))
        (let ([core (xpand-define-library (caar sexps) (cdar sexps)
                                          sld-env #t)])
          (unless (and (sexp-match? '(define-library * *) core)
                       (equal? (cadr core) listname)
                       (val-library? (caddr core)))
            (error "shadow library: internal transformer error"
                   listname filepath))
          (caddr core))))))

;; Where a shadowed name lives in the compiler's own tree, or #f when
;; the compiler is not installed as a library and the caller's -I is
;; supplying them instead.
(define (cp-shadow-library-path listname)
  (let ([clib (cp-resource "clib")])
    (and (not (string=? clib ""))
         (let ([p (listname->path listname (dir-add-separator clib) ".sld")])
           (and p (file-exists? p) p)))))

(define (make-shadow-sld-environment)
  (define base (make-sld-environment *root-name-registry*))
  (define cache '()) ; alist: shadowed listname -> location of its library
  (define (fetch id)
    (let ([p (cp-shadow-library-path id)])
      (if p
          (cp-fetch-library-at id p env)
          ;; not installed: the library path, as the driver arranged it
          (fetch-library id env))))
  (define (env id at)
    (cond [(and (memq at '(ref peek)) (member id *dropped-library-names*))
           #f]
          [(and (memq at '(ref peek)) (member id *shadow-library-names*))
           (cond [(assoc id cache) => cdr]
                 [else (let ([lib (fetch id)])
                         (and lib
                              (let ([loc (make-location lib)])
                                (set! cache (cons (cons id loc) cache))
                                loc)))])]
          [else (base id at)]))
  env)
(define *shadow-sld-environment* (make-shadow-sld-environment))

;;; Shadow libraries are found by Skint's ordinary name->path mapping
;;; (listname->path appends the components under a directory), applied
;;; to clib/ instead of to the library path: (scheme base) is
;;; clib/scheme/base.sld, (srfi 14) is clib/srfi/14.sld, (skint ivset)
;;; is clib/skint/ivset.sld.  Nothing is appended or prepended to
;;; *library-directory-list*, so the host's own libraries are untouched
;;; and the result does not depend on the caller's -I or working
;;; directory; the interception above is what makes the shadowed names
;;; come from there rather than from Skint's predefined registry.

;;; --- cond-expand world for compiled code ------------------------------
;;; cond-expand, both in library declarations and in program code, must
;;; describe the world the compiled program will run in, not the host
;;; interpreter's.  Feature tests therefore see only *ds-features*:
;;; (r7rs) plus what a standalone MARV executable provides.  Library
;;; tests, (library (srfi 1)) and friends, are answered by Skint's
;;; library-available? through the shadow environment's 'peek, so they
;;; reflect exactly the libraries compiled code can import.  Skint's
;;; features is a parameter; expand-program-files binds it for the
;;; whole expansion of a program (libraries included).
;; `skint` is claimed: a compiled program implements Skint's
;; dialect surface (DS prims plus the shadow libraries), so the
;; `skint` cond-expand branches of borrowed Skint sources -- the
;; ones written against Skint builtins -- are the applicable
;; ones.  The tower features stay unclaimed: no full numeric
;; tower, no full unicode, no C99/XSI math.
(define *ds-features* '(r7rs skint))

;; library-available? decides (library ...) cond-expand tests.  Skint's
;; version answers from the host's world (root registry, then
;; find-library-path on disk), which mostly coincides with the
;; compiled-code world because the shadow environment's 'peek is
;; consulted first and dropped libraries fail at import time anyway.
;; Known wart: (library (scheme eval)) and friends test true (the
;; host provides them) although importing them into compiled code is
;; rejected as dropped.  The binding cannot be corrected from here:
;; a define in this file only shadows it in the script environment
;; (preprocess-cond-expand resolves the name in Skint's own
;; environment), and set! on the core binding is denied by Skint's
;; identifier-access rules.

(define (program-module-name ifile)
  (string->symbol (path-strip-extension (path-strip-directory ifile))))

;; A program source is a filename, an input port, or a list of forms.
;; Reading is the only part that differs between them, so it is the only
;; part this dispatches on: the environment, the global-name prefix and
;; the feature set are shared.  A filename is opened as given -- the
;; caller.s working directory decides what it means. [cco]
(define (for-each-program-form src proc)
  (cond [(string? src)
         ;; Opened exactly as given: the caller.s working directory
         ;; decides what the name means, as it would for any other
         ;; tool.  NOT call-with-current-input-file, which resolves
         ;; relative to the file being read -- that is right for an
         ;; (include) nested inside a program and wrong for the
         ;; program named by the caller.  with-current-file still
         ;; records it, so those nested includes resolve against the
         ;; program itself. [cco]
         (let ([iport (open-input-file src)])
           (with-current-file src
             (lambda () (for-each-program-form iport proc)))
           (close-input-port iport))]
        [(input-port? src)
         (let loop ([x (read-code-sexp src)])
           (unless (eof-object? x) (proc x) (loop (read-code-sexp src))))]
        [(list? src) (for-each proc src)]
        [else (error "not a program source" src)]))

(define (expand-program-files ifiles modname) ; => (begin <core> ...)
  (define global (lambda (n) (symbol-append 'prog:// modname '? n)))
  (define ial (list (cons 'import (make-location 'import))))
  (define env (make-controlled-environment ial global
                  *shadow-sld-environment*))
  (define cores '())
  (define (emit-core! c) (set! cores (cons c cores)))
  (define (expand-top x)
    (if (pair? x)
        (let ([hval (xpand #t (car x) env)])
          (cond
            [(eq? hval 'begin) (for-each expand-top (cdr x))]
            [(eq? hval 'define)
             (let ([tail (preprocess-define (car x) (cdr x))])
               (if (list1? tail)
                   (expand-top (car tail))
                   (let ([loc (top-defined-id-lookup env (car tail) 'define)])
                     (emit-core! (list 'gset! (cadr (location-val loc))
                                       (xpand #f (cadr tail) env))))))]
            [(eq? hval 'define-syntax)
             (let* ([tail (preprocess-define-syntax (car x) (cdr x))]
                    [loc (top-defined-id-lookup env (car tail) 'define-syntax)])
               (location-set-val! loc (xpand #t (cadr tail) env)))]
            [(eq? hval 'define-library)
             (let* ([core (xpand-define-library (car x) (cdr x) env #t)]
                    [loc (xenv-lookup env (cadr core) 'define-syntax)])
               (location-set-val! loc (caddr core)))]
            [(eq? hval 'import)
             (let* ([core (xpand-import (car x) (cdr x) env #t)]
                    [l (cadr core)])
               (env (library-exports l) 'import)
               (emit-core! (library-code l)))]
            [(val-transformer? hval)
             (let* ([nx (hval x env)] [hv (and (pair? nx) (xpand #t (car nx) env))])
               (cond [(and (eq? hv 'begin) (list2+? nx))
                      (emit-core! (preprocess-mixed-forms (cdr nx) env env))]
                     [(memq hv '(define define-syntax))
                      (emit-core! (preprocess-mixed-forms (list nx) env env))]
                     [else (expand-top nx)]))]
            [(val-integrable? hval) (emit-core! (xpand-integrable hval (cdr x) env))]
            [(val-builtin? hval) (emit-core! (xpand #f x env))]
            [else (emit-core! (xpand-call hval (cdr x) env))]))
        (emit-core! (xpand #f x env))))
  (parameterize ([features *ds-features*])
    (for-each (lambda (src) (for-each-program-form src expand-top)) ifiles))
  (cons 'begin (reverse cores)))
