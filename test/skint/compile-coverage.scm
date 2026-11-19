(import (scheme base) (scheme write) (scheme read) (scheme file)
        (scheme eval) (scheme repl) (scheme char)
        (skint compile)
        (only (skint hidden)
              *root-name-registry* library-exports val-library? location-val
              find-library-path))

(include "test.scm")

;;; Coverage of the R7RS-small vocabulary by the compiled world.
;;;
;;; The compiler's clib describes the libraries a standalone MARV executable
;;; provides.  A name can be absent from it in three ways, and only the first
;;; is visible by reading an export list:
;;;
;;;   1. not exported at all, so an import of it fails;
;;;   2. exported but never defined, which the compiler reports as a global
;;;      defined by no library;
;;;   3. exported and defined, but its definition calls a prim that the srt
;;;      never defined, which marv reports as an unknown instruction only when
;;;      a program happens to reach it.
;;;
;;; This test finds all three without marv and without a toolchain, by
;;; compiling a probe that references every value the host's own (scheme ...)
;;; libraries export and then comparing the module's srt references against
;;; what the srt headers declare.
;;;
;;; The vocabulary is taken from the running Skint rather than written out
;;; here, so it cannot drift from the implementation it is meant to mirror.
;;; [cco]

;; ---------------------------------------------------------------------------
;; The vocabulary: what this Skint's own (scheme ...) libraries export
;; ---------------------------------------------------------------------------

(define (registry-libraries nr r)
  (let loop ([l (vector-ref nr (- (vector-length nr) 1))] [r r])
    (cond [(null? l) r]
          [else
           (let ([name (caar l)] [val (location-val (cdar l))])
             (loop (cdr l)
                   (if (and (val-library? val) (not (assoc name r)))
                       (cons (cons name (library-exports val)) r)
                       r)))])))

(define *reference* (registry-libraries *root-name-registry* '()))

(define (reference-exports lib)
  (let ([row (assoc lib *reference*)])
    (if row (map car (cdr row)) '())))

;; The libraries the compiled world claims to provide.  (scheme eval), (scheme
;; load) and (scheme repl) are deliberately not among them: they are
;; meaningless without an evaluator, and clib has no file for them.
(define *provided*
  '((scheme base) (scheme case-lambda) (scheme char) (scheme complex)
    (scheme cxr) (scheme file) (scheme inexact) (scheme lazy)
    (scheme process-context) (scheme read) (scheme time) (scheme write)
    (scheme r5rs)))

;; R7RS auxiliary syntax.  These are exported, and this Skint will even
;; evaluate some of them, but they denote no value in any implementation, so a
;; reference to one is not a coverage question.
(define *auxiliary* '(else => _ ... unquote unquote-splicing library))

;; A special form cannot be referenced as a value.  Which names those are is
;; decided by evaluating each one rather than by inspecting its denotation:
;; append, map, apply and call/cc are transformers in this implementation and
;; are referenceable all the same.
(define (referenceable? name env)
  (and (not (memq name *auxiliary*))
       (call-with-current-continuation
        (lambda (k)
          (with-exception-handler
           (lambda (e) (k #f))
           (lambda () (eval name env) #t))))))

(define (sym<? a b) (string<? (symbol->string a) (symbol->string b)))
(define (merge a b)
  (cond [(null? a) b] [(null? b) a]
        [(sym<? (car a) (car b)) (cons (car a) (merge (cdr a) b))]
        [else (cons (car b) (merge a (cdr b)))]))
(define (msort l)
  (if (or (null? l) (null? (cdr l))) l
      (let loop ([l l] [a '()] [b '()])
        (cond [(null? l) (merge (msort a) (msort b))]
              [(null? (cdr l)) (merge (msort (cons (car l) a)) (msort b))]
              [else (loop (cddr l) (cons (car l) a) (cons (cadr l) b))]))))

(define (merge-strings a b)
  (cond [(null? a) b] [(null? b) a]
        [(string<? (car a) (car b)) (cons (car a) (merge-strings (cdr a) b))]
        [else (cons (car b) (merge-strings a (cdr b)))]))
(define (msort-strings l)
  (if (or (null? l) (null? (cdr l))) l
      (let loop ([l l] [a '()] [b '()])
        (cond [(null? l) (merge-strings (msort-strings a) (msort-strings b))]
              [(null? (cdr l))
               (merge-strings (msort-strings (cons (car l) a)) (msort-strings b))]
              [else (loop (cddr l) (cons (car l) a) (cons (cadr l) b))]))))

(define (split-vocabulary lib)
  ;; -> (values referenceable-names special-forms)
  (let ([env (environment lib)])
    (let loop ([ns (reference-exports lib)] [v '()] [f '()])
      (cond [(null? ns) (values (msort v) (msort f))]
            [(memq (car ns) *auxiliary*) (loop (cdr ns) v f)]
            [(referenceable? (car ns) env) (loop (cdr ns) (cons (car ns) v) f)]
            [else (loop (cdr ns) v (cons (car ns) f))]))))

;; ---------------------------------------------------------------------------
;; What the srt declares
;; ---------------------------------------------------------------------------

;; srt/ sits beside the library's own .sld, exactly as the compiler finds it.
(define (srt-path file)
  (let* ([sld (find-library-path '(skint compile))]
         [n (string-length sld)])
    (string-append (if (and (> n 4) (string=? (substring sld (- n 4) n) ".sld"))
                       (substring sld 0 (- n 4))
                       sld)
                   "/srt/" file)))

(define (read-one path)
  (call-with-port (open-input-file path) (lambda (p) (read p))))

(define (dollar-name sym)        ; $not -> "not"
  (let ([s (symbol->string sym)])
    (if (and (> (string-length s) 0) (char=? (string-ref s 0) #\$))
        (substring s 1 (string-length s))
        s)))

;; (interface $scmi (macro $x ...) (func $y ...) (global $z ...) ...)
(define (interface-entry-points path)
  (let* ([form (read-one path)]
         [modname (dollar-name (cadr form))])
    (let loop ([ms (cddr form)] [r '()])
      (cond [(null? ms) r]
            [(and (pair? (car ms))
                  (memq (caar ms) '(macro func global))
                  (pair? (cdar ms))
                  (symbol? (cadar ms)))
             (loop (cdr ms)
                   (cons (string-append "$" modname ":"
                                        (dollar-name (cadar ms)))
                         r))]
            [else (loop (cdr ms) r)]))))

(define *srt-declared*
  (append (interface-entry-points (srt-path "scmn.mvh"))
          (interface-entry-points (srt-path "scmi.mvh"))
          (interface-entry-points (srt-path "scmrt.mvh"))))

;; ---------------------------------------------------------------------------
;; Compiling a probe
;; ---------------------------------------------------------------------------

;; A probe imports the library under test, defines one identity procedure, and
;; references every value from a top-level expression so that nothing can be
;; dropped as unreachable.  The core library has to be imported too: in a
;; program that does not import it, (define (f x) x) is read as a call.
(define (probe-forms lib names)
  (cons (if (or (equal? lib '(scheme base)) (equal? lib '(scheme r5rs)))
            (list 'import lib)
            (list 'import '(scheme base) lib))
        (cons '(define (%u x) x)
              (map (lambda (n) (list '%u n)) names))))

;; The compiler reports an undefined global through error, with its own message
;; first and the offending name among the irritants, so the name is the first
;; irritant that is a symbol.
(define (first-symbol l)
  (cond [(null? l) 'unknown]
        [(symbol? (car l)) (car l)]
        [else (first-symbol (cdr l))]))

;; -> (values module-or-#f undefined-name-or-#f)
(define (try-compile forms)
  (call-with-current-continuation
   (lambda (k)
     (with-exception-handler
      (lambda (e)
        (k #f (first-symbol (if (error-object? e)
                                (error-object-irritants e)
                                '()))))
      (lambda () (values (compile-program forms #f) #f))))))

;; Compile the probe, dropping any name the compiler rejects as undefined and
;; retrying, so one run reports every such name rather than only the first.
(define (compile-probe lib names)
  (let loop ([ns names] [undefined '()] [guard 0])
    (if (> guard 200)
        (values #f (msort undefined))
        (let-values ([(mod bad) (try-compile (probe-forms lib ns))])
          (cond [mod (values mod (msort undefined))]
                [(memq bad ns)
                 (loop (remove-name bad ns) (cons bad undefined) (+ guard 1))]
                [else (values #f (msort (cons bad undefined)))])))))

(define (remove-name x l)
  (let loop ([l l] [r '()])
    (cond [(null? l) (reverse r)]
          [(eq? (car l) x) (loop (cdr l) r)]
          [else (loop (cdr l) (cons (car l) r))])))

;; every $module:name symbol the module mentions
(define (module-entry-points mod)
  (let ([seen '()])
    (let walk ([x mod])
      (cond [(pair? x) (walk (car x)) (walk (cdr x))]
            [(symbol? x)
             (let ([s (symbol->string x)])
               (when (and (> (string-length s) 4)
                          (string=? (substring s 0 4) "$scm")
                          (memv #\: (string->list s))
                          (not (member s seen)))
                 (set! seen (cons s seen))))]
            [else #f]))
    seen))

;; ---------------------------------------------------------------------------
;; The run
;; ---------------------------------------------------------------------------

(define *used* '())          ; srt entry points the probes reached
(define *uncovered* '())     ; (lib . special-forms) a reference cannot reach

(for-each
 (lambda (lib)
   (let-values ([(names forms) (split-vocabulary lib)])
     (set! *uncovered* (cons (cons lib forms) *uncovered*))
     (let-values ([(mod undefined) (compile-probe lib names)])
       ;; the harness drops a test's name, so the library is named here and
       ;; carried inside the tested value, which is what a failure prints
       (display "; ") (write lib)
       (display " -- ") (display (length names)) (display " references")
       (newline)
       ;; 1. every exported name must be defined somewhere in clib
       (test '() (if (null? undefined) '() (cons lib undefined)))
       ;; 2. the probe must compile at all
       (test '() (if mod '() (list lib 'probe-did-not-compile)))
       (when mod
         (for-each (lambda (e) (unless (member e *used*)
                                 (set! *used* (cons e *used*))))
                   (module-entry-points mod))))))
 *provided*)

;; 3. every srt entry point the vocabulary reaches must be declared by the srt
(define *phantom*
  (let loop ([u *used*] [r '()])
    (cond [(null? u) r]
          [(member (car u) *srt-declared*) (loop (cdr u) r)]
          [else (loop (cdr u) (cons (car u) r))])))

(test '() (msort-strings *phantom*))

;; ---------------------------------------------------------------------------
;; What this test does not reach
;; ---------------------------------------------------------------------------

(display "; srt entry points declared: ")
(display (length *srt-declared*))
(display ", reached by a reference: ")
(display (- (length *used*) (length *phantom*)))
(newline)
(display "; special forms, which a reference cannot reach:")
(newline)
(for-each (lambda (row)
            (unless (null? (cdr row))
              (display ";   ") (write (car row)) (display ":")
              (for-each (lambda (s) (display " ") (display s)) (cdr row))
              (newline)))
          (reverse *uncovered*))

(test-end)
