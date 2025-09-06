
(define-library (srfi 29)
  ; [esl!] Skint's ~N@* implemented in CL-like form!
  (import 
    (scheme base) (srfi 28)
    (only (skint) format ; import from srfi 28, which imports from skint
      current-language current-country current-locale-details))
  (export format ; re-export the very same skint import
    current-language current-country current-locale-details 
    declare-bundle! store-bundle! load-bundle! localized-template)

(begin
; [esl] adapted from ref. imp. by Scott G. Miller
; (C) 2002 Scott G. Miller

;; The association list in which bundles will be stored
(define *localization-bundles* '())

;; [esl*] current-language and current-country functions are Skint built-ins

;; The load-bundle! and store-bundle! both return #f in this
;; reference implementation.  A compliant implementation need
;; not rewrite these procedures.
(define load-bundle!
  (lambda (bundle-specifier)
    #f))

(define store-bundle!
  (lambda (bundle-specifier)
    #f))

;; Declare a bundle of templates with a given bundle specifier
(define declare-bundle!
  (letrec ((remove-old-bundle
            (lambda (specifier bundle)
              (cond ((null? bundle) '())
                    ((equal? (caar bundle) specifier)
                     (cdr bundle))
                    (else (cons (car bundle)
                                (remove-old-bundle specifier
                                                   (cdr bundle))))))))
    (lambda (bundle-specifier bundle-assoc-list)
      (set! *localization-bundles*
            (cons (cons bundle-specifier bundle-assoc-list)
                  (remove-old-bundle bundle-specifier
                                     *localization-bundles*))))))

;;Retrieve a localized template given its package name and a template name
(define localized-template
  (letrec ((rdc
            (lambda (ls)
              (if (null? (cdr ls))
                  '()
                  (cons (car ls) (rdc (cdr ls))))))
           (find-bundle
            (lambda (specifier template-name)
              (cond ((assoc specifier *localization-bundles*) =>
                     (lambda (bundle) bundle))
                    ((null? specifier) #f)
                    (else (find-bundle (rdc specifier)
                                       template-name))))))
    (lambda (package-name template-name)
      (let loop ((specifier (cons package-name
                                  (list (current-language)
                                        (current-country)))))
        (and (not (null? specifier))
             (let ((bundle (find-bundle specifier template-name)))
               (and bundle
                    (cond ((assq template-name bundle) => cdr)
                          ((null? (cdr specifier)) #f)
                          (else (loop (rdc specifier)))))))))))

))
