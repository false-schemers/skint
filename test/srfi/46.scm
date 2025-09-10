(import (srfi 46))
(include "test.scm")

(test-begin "srfi-46")

;[esl] tests made from examples in the original SRFI

;;; Examples of the user-specified ellipsis token extension

;;; Utility macro for CPS macros
(define-syntax apply-syntactic-continuation
  (syntax-rules ()
    ((apply-syntactic-continuation (?k ?env ...) . ?args)
     (?k ?env ... . ?args))))

;;; Generates a list of temporaries, for example to implement LETREC
;;; (see below), and 'returns' it by CPS.
(define-syntax generate-temporaries
  (syntax-rules ()
    ((generate-temporaries ?origs ?k)
     (letrec-syntax
         ((aux (syntax-rules ::: ()
                 ;; We use a trick here: pass the continuation _again_
                 ;; to AUX in case it contains ellipsis.  If we stuck
                 ;; it right into AUX's template, AUX would process the
                 ;; ellipsis in ?K as ellipsis for something in the AUX
                 ;; macro.
                 ((aux ?temps () ?k*)
                  (apply-syntactic-continuation ?k* ?temps))
                 ;; Be careful about the ellipsis!
                 ((aux (?temp :::) (?x ?more :::) ?k*)
                  (aux (?temp ::: new-temp)
                       (?more :::)
                       ?k*)))))
       (aux () ?origs ?k)))))

;;; Instead of having lots of auxiliary clauses in LETREC, like in the
;;; R5RS sample implementation, we use GENERATE-TEMPORARIES.  Instead
;;; of 'returning,' like an ordinary function, we create a continuation
;;;  for GENERATE-TEMPORARIES with LET-SYNTAX.  Since this continuation
;;; uses ellipsis, we must use the ellipsis token extension.
(define-syntax let-rec
  (syntax-rules ()
    ((_ ((?var ?init) ...) ?body1 ?body2 ...)
     (let-syntax
         ((k (syntax-rules ::: ()
               ;; Use the same trick as with the continuations in
               ;; GENERATE-TEMPORARIES.  Be careful about the ellipsis!
               ((k ((?var* ?init*) :::)
                   (?body1* ?body2* :::)
                   ;; Here are the actual arguments to the continuation
                   ;; -- the previous bits of the pattern were just the
                   ;; 'environment' of the continuation --:
                   (?temp :::))
                (let ((?var* (if #f #f)) ; Get an 'unspecific' value.
                      :::)
                  (let ((?temp ?init*) :::)
                    (set! ?var* ?temp) :::
                    (let () ?body1* ?body2* :::)))))))
       (generate-temporaries (?var ...)
         ;; Pass K the environment.  GENERATE-TEMPORARIES will add the
         ;; temporary variable list argument.
         (k ((?var ?init) ...) (?body1 ?body2 ...)))))))

(test '(#t #f #t #f #t #f #t) 
  (let-rec ((is-even? (lambda (x) (if (= x 0) #t (is-odd? (- x 1)))))
            (is-odd? (lambda (x) (if (= x 0) #f (is-even? (- x 1))))))
    (map is-even? '(0 1 2 3 4 5 6))))

;;; The next example uses two other macros that we don't define here:
;;; SYNTAX-SYMBOL? and UNION.  (SYNTACTIC-SYMBOL? <x> <sk> <fk>)
;;; expands to SK if X is a symbol or FK otherwise.  

;[esl]+ after O.K.
(define-syntax syntax-symbol?
  (syntax-rules ()
    ((_ (x . y) kt kf) kf)
    ((_ #(x ...) kt kf) kf)
    ((_ maybe-symbol kt kf)
     (let-syntax 
       ((test (syntax-rules ()
                ((_ maybe-symbol t f) t)
                ((_ x t f) f))))
       (test abracadabra kt kf)))))

;;; (UNION <s1> <s2> <k>) applies K with APPLY-SYNTACTIC-CONTINUATION 
;;; to the union of the syntactic lists S1 and S2.  Both of SYNTACTIC-SYMBOL? 
;;; and UNION are possible to implement here, but we sha'n't bother with them, as
;;; we wish only to demonstrate an example of macros generating macro-
;;; generating macros, and they provide no such examples.

;[esl]+ after O.K.

(define-syntax syntax-eq? 
  (syntax-rules ()
    [(_ id b kt kf)
     ((syntax-lambda (id ok) ((syntax-rules () [(_ b) (id)]) ok))
      (syntax-rules () [(_) kf]) (syntax-rules () [(_) kt]))]))

(define-syntax syntax-member?
  (syntax-rules ()
    [(_ id () kt kf) 
     kf]
    [(_ id (id0 . id*) kt kf) 
     (syntax-eq? id id0 kt (syntax-member? id id* kt kf))]))

(define-syntax union
  (syntax-rules ()
    ((_ () s2 k)
     (apply-syntactic-continuation k s2))
    ((_ (id . id*) (s ...) k)
     (syntax-member? id (s ...)
       (union id* (s ...) k)
       (union id* (s ... id) k)))))

;;; ALL-SYMBOLS digs out all the symbols in a syntax.
(define-syntax all-symbols
  (syntax-rules ()
    ((all-symbols (?x . ?y) ?k)
     (let-syntax
         ((k (syntax-rules :::0 ()
               ((k ?y* ?k*  (?symbol :::0))
                (let-syntax
                    ((k* (syntax-rules :::1 ()
                           ;; Doubly nested ellipsis: we use another
                           ;; distinct ellipsis token.
                           ((k* ?k** (?symbol* :::1))
                            (union (?symbol  :::0)
                                   (?symbol* :::1)
                                   ?k**)))))
                  (all-symbols ?y* (k* ?k*)))))))
       (all-symbols ?x (k ?y ?k))))

    ((all-symbols #(?x ...) ?k)
     (all-symbols (?x ...) ?k))

    ((all-symbols ?x ?k)
     (syntax-symbol? ?x
       (apply-syntactic-continuation ?k (?x))
       (apply-syntactic-continuation ?k ())))))

(test '(frotz mumble zot quux baz bar foo)
  (all-symbols (foo 4 bar #(#t (baz (#f quux)) zot) (mumble #(frotz)))
               (quote)))

;;; This example demonstrates the hygienic renaming of the ellipsis
;;; identifiers.

(test '((1) 2 (3) (4))
  (let-syntax
      ((f (syntax-rules ()
            ((f ?e)
            (let-syntax
                ((g (syntax-rules ::: ()
                      ((g (??x ?e) (??y :::))
                        '((??x) ?e (??y) :::)))))
              (g (1 2) (3 4)))))))
    (f :::)))
; => ((1) 2 (3) (4)), if hygienic rules of ellipsis identifiers are
; correctly implemented, not ((1) (2) (3) (4))

;;; --------------------
;;; Examples of tail patterns

;;; This example of the tail pattern extension is a crippled version of
;;; R5RS's BEGIN special form.  (It is crippled because it does not
;;; support internal definitions or commands within its body returning
;;; fewer or more than one value.)

(define-syntax fake-begin
  (syntax-rules ()
    ((fake-begin ?body ... ?tail)
     (let* ((ignored ?body) ...) ?tail))))

(test-equal "Hello, world!"
  (let ((p (open-output-string)))
    (fake-begin
      (display "Hello," p)
      (write-char #\space p)
      (display "world!" p)
      (newline)
      (get-output-string p))))
    
;;; For example,
;;;   (FAKE-BEGIN
;;;     (DISPLAY "Hello,")
;;;     (WRITE-CHAR #\SPACE)
;;;     (DISPLAY "world!")
;;;     (NEWLINE))
;;; would expand to
;;;   (LET* ((IGNORED~0 (DISPLAY "Hello,"))
;;;          (IGNORED~1 (WRITE-CHAR #\SPACE))
;;;          (IGNORED~2 (DISPLAY "world!")))
;;;     (NEWLINE))
;;; where IGNORED~n is a hygienic renaming of the identifier IGNORED
;;; from FAKE-BEGIN's output template.

(test '(1 (2 3 4) 5)
  (let-syntax
    ((foo (syntax-rules ()
            ((foo ?x ?y ... ?z)
             (list ?x (list ?y ...) ?z)))))
    (foo 1 2 3 4 5)))
; => (1 (2 3 4) 5)

(test-end)

