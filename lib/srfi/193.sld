
(define-library (srfi 193)
  (export command-line command-name command-args script-file script-directory)
  (import (scheme base) 
          (rename (scheme process-context) (command-line *command-line))
          (only (skint) directory-separator current-directory) 
          (only (skint hidden) %command-line current-file-stack current-file
            path-directory path-strip-directory path-strip-extension
            base-path-separator file-resolve-relative-to-base-path))

(begin

; command names will be resolved relative to this (assume no change before import!)
(define start-directory (current-directory))

; in a program/script, command line is changed not to include Skint interpreter
(define in-program? (not (equal? (%command-line) (*command-line)))) ; -s -p etc.

; this srfi, if imported, changes the definition of command-line to make sure 
; that its non-script first argument is "" (as in this srfi); it also takes 
; into account a top-level file currently being loaded/included into REPL
(define (command-line) ; NB: not a parameter, unlike the builtin!
  (define cl (*command-line))
  (cond [in-program? cl] ; not a REPL, load is immaterial
        [(and (= 1 (length (current-file-stack))) (current-file)) =>
         (lambda (cf) (cons cf (cdr cl)))]
        [else (cons "" (cdr cl))]))

(define (command-name)
  (let ((filename (car (command-line))))
    (and (not (string=? filename ""))
         (path-strip-extension (path-strip-directory filename)))))

(define (command-args)
  (cdr (command-line)))

(define (script-file)
  (let ((filename (car (command-line))))
    (and (not (string=? filename ""))
         (file-resolve-relative-to-base-path filename start-directory))))

(define (script-directory)
  (let ((filename (script-file)))
    (and filename
         (let ((directory (path-directory filename))) 
           (if (base-path-separator directory)
               directory
               (string-append directory (string (directory-separator))))))))

))