;;; fl.sld -- (skint fl) as a real library, for compiled programs.
;;;
;;; Shadow counterpart of Skint's (skint fl) (itself a pure re-export
;;; of (skint) builtins), in the established cover-macro style: a
;;; call position expands to the Skint builtin (imported renamed as
;;; %builtin-<name>), which the compiler recognises as a DS prim; a
;;; bare identifier expands to %residual-<name>, an ordinary
;;; fixed-arity definition in this library.  The n-ary flonum
;;; comparisons ride the compiler's pairwise-chain handling for
;;; fx*/fl* comparators, so covers (which forward any operands) have
;;; Skint's variadic surface there too; 0-argument cover clauses
;;; spell the identities.
;;;
;;; The C99/XSI-only exports of Skint's (skint fl) are gated there
;;; on (skint-c99-math) / (skint-xsi-math) features, which compiled
;;; programs do not have, so they are not part of this surface
;;; (SRFI-144 grows them from its own polyfills when the features
;;; are absent, exactly as under Skint).
;;;
;;; Plain library procedures: flquotient / flremainder / flmodquo /
;;; flmodulo (the four division flavours from fl/, fltruncate and
;;; flfloor), flgcd (Euclid), flodd? / fleven? / flinteger? (from
;;; flremainder / flfloor), fllog10, and the two-argument-tolerant
;;; fllog / flatan (whose 2-argument call positions reach the
;;; compiler's atan2 / log-base handling directly).  flmodf and
;;; flfrexp keep cover macros: their call positions reach the
;;; compiler's box-prim handling, the box operand usually being
;;; (set& var).

(define-library (skint fl)
  (import (only (skint) define lambda define-syntax syntax-rules if let
                  let* and or when unless cond begin set! quote _ ...)
          (only (skint) pair? null? car cdr not)
          (rename (only (skint) flonum? fl=? fl<? fl>? fl<=? fl>=? fl!=?
                          flzero? flpositive? flnegative? flnan?
                          flfinite? flinfinite? fl+ fl- fl* fl/ flneg
                          flabs flmin flmax flexpt flsqrt flfloor
                          flceiling fltruncate flround flexp fllog
                          flsin flcos fltan flasin flacos flatan
                          flsinh flcosh fltanh flldexp flmodf flfrexp
                          fllog10 flonum->fixnum)
            (flonum? %builtin-flonum?)
            (fl=? %builtin-fl=?)
            (fl<? %builtin-fl<?)
            (fl>? %builtin-fl>?)
            (fl<=? %builtin-fl<=?)
            (fl>=? %builtin-fl>=?)
            (fl!=? %builtin-fl!=?)
            (flzero? %builtin-flzero?)
            (flpositive? %builtin-flpositive?)
            (flnegative? %builtin-flnegative?)
            (flnan? %builtin-flnan?)
            (flfinite? %builtin-flfinite?)
            (flinfinite? %builtin-flinfinite?)
            (fl+ %builtin-fl+)
            (fl- %builtin-fl-)
            (fl* %builtin-fl*)
            (fl/ %builtin-fl/)
            (flneg %builtin-flneg)
            (flabs %builtin-flabs)
            (flmin %builtin-flmin)
            (flmax %builtin-flmax)
            (flexpt %builtin-flexpt)
            (flsqrt %builtin-flsqrt)
            (flfloor %builtin-flfloor)
            (flceiling %builtin-flceiling)
            (fltruncate %builtin-fltruncate)
            (flround %builtin-flround)
            (flexp %builtin-flexp)
            (fllog %builtin-fllog)
            (flsin %builtin-flsin)
            (flcos %builtin-flcos)
            (fltan %builtin-fltan)
            (flasin %builtin-flasin)
            (flacos %builtin-flacos)
            (flatan %builtin-flatan)
            (flsinh %builtin-flsinh)
            (flcosh %builtin-flcosh)
            (fltanh %builtin-fltanh)
            (flldexp %builtin-flldexp)
            (flmodf %builtin-flmodf)
            (flfrexp %builtin-flfrexp)
            (fllog10 %builtin-fllog10)
            (flonum->fixnum %builtin-flonum->fixnum)))

  (export
    (rename %cover-flonum? flonum?)
    (rename %cover-fl=? fl=?)
    (rename %cover-fl<? fl<?)
    (rename %cover-fl>? fl>?)
    (rename %cover-fl<=? fl<=?)
    (rename %cover-fl>=? fl>=?)
    (rename %cover-fl!=? fl!=?)
    (rename %cover-flzero? flzero?)
    (rename %cover-flpositive? flpositive?)
    (rename %cover-flnegative? flnegative?)
    flodd? fleven? flinteger?
    (rename %cover-flnan? flnan?)
    (rename %cover-flfinite? flfinite?)
    (rename %cover-flinfinite? flinfinite?)
    (rename %cover-fl+ fl+)
    (rename %cover-fl- fl-)
    (rename %cover-fl* fl*)
    (rename %cover-fl/ fl/)
    (rename %cover-flneg flneg)
    (rename %cover-flabs flabs)
    flquotient flremainder flmodquo flmodulo
    (rename %cover-flmin flmin)
    (rename %cover-flmax flmax)
    flgcd
    (rename %cover-flexpt flexpt)
    (rename %cover-flsqrt flsqrt)
    (rename %cover-flfloor flfloor)
    (rename %cover-flceiling flceiling)
    (rename %cover-fltruncate fltruncate)
    (rename %cover-flround flround)
    (rename %cover-flexp flexp)
    fllog fllog10
    (rename %cover-flsin flsin)
    (rename %cover-flcos flcos)
    (rename %cover-fltan fltan)
    (rename %cover-flasin flasin)
    (rename %cover-flacos flacos)
    flatan
    (rename %cover-flsinh flsinh)
    (rename %cover-flcosh flcosh)
    (rename %cover-fltanh fltanh)
    (rename %cover-flldexp flldexp)
    (rename %cover-flmodf flmodf)
    (rename %cover-flfrexp flfrexp)
    (rename %cover-flonum->fixnum flonum->fixnum))

  (begin
    (define-syntax %cover-flonum?
      (syntax-rules ()
        [(_ . args) (%builtin-flonum? . args)]
        [_ %residual-flonum?]))
    (define %residual-flonum? (lambda (a0) (%builtin-flonum? a0)))

    (define-syntax %cover-fl=?
      (syntax-rules ()
        [(_) #t]
        [(_ a) #t]
        [(_ . args) (%builtin-fl=? . args)]
        [_ %residual-fl=?]))
    (define %residual-fl=? (lambda (a0 a1) (%builtin-fl=? a0 a1)))
    (define-syntax %cover-fl<?
      (syntax-rules ()
        [(_) #t]
        [(_ a) #t]
        [(_ . args) (%builtin-fl<? . args)]
        [_ %residual-fl<?]))
    (define %residual-fl<? (lambda (a0 a1) (%builtin-fl<? a0 a1)))
    (define-syntax %cover-fl>?
      (syntax-rules ()
        [(_) #t]
        [(_ a) #t]
        [(_ . args) (%builtin-fl>? . args)]
        [_ %residual-fl>?]))
    (define %residual-fl>? (lambda (a0 a1) (%builtin-fl>? a0 a1)))
    (define-syntax %cover-fl<=?
      (syntax-rules ()
        [(_) #t]
        [(_ a) #t]
        [(_ . args) (%builtin-fl<=? . args)]
        [_ %residual-fl<=?]))
    (define %residual-fl<=? (lambda (a0 a1) (%builtin-fl<=? a0 a1)))
    (define-syntax %cover-fl>=?
      (syntax-rules ()
        [(_) #t]
        [(_ a) #t]
        [(_ . args) (%builtin-fl>=? . args)]
        [_ %residual-fl>=?]))
    (define %residual-fl>=? (lambda (a0 a1) (%builtin-fl>=? a0 a1)))
    (define-syntax %cover-fl!=?
      (syntax-rules ()
        [(_ . args) (%builtin-fl!=? . args)]
        [_ %residual-fl!=?]))
    (define %residual-fl!=? (lambda (a0 a1) (%builtin-fl!=? a0 a1)))

    (define-syntax %cover-flzero?
      (syntax-rules ()
        [(_ . args) (%builtin-flzero? . args)]
        [_ %residual-flzero?]))
    (define %residual-flzero? (lambda (a0) (%builtin-flzero? a0)))
    (define-syntax %cover-flpositive?
      (syntax-rules ()
        [(_ . args) (%builtin-flpositive? . args)]
        [_ %residual-flpositive?]))
    (define %residual-flpositive? (lambda (a0) (%builtin-flpositive? a0)))
    (define-syntax %cover-flnegative?
      (syntax-rules ()
        [(_ . args) (%builtin-flnegative? . args)]
        [_ %residual-flnegative?]))
    (define %residual-flnegative? (lambda (a0) (%builtin-flnegative? a0)))
    (define-syntax %cover-flnan?
      (syntax-rules ()
        [(_ . args) (%builtin-flnan? . args)]
        [_ %residual-flnan?]))
    (define %residual-flnan? (lambda (a0) (%builtin-flnan? a0)))
    (define-syntax %cover-flfinite?
      (syntax-rules ()
        [(_ . args) (%builtin-flfinite? . args)]
        [_ %residual-flfinite?]))
    (define %residual-flfinite? (lambda (a0) (%builtin-flfinite? a0)))
    (define-syntax %cover-flinfinite?
      (syntax-rules ()
        [(_ . args) (%builtin-flinfinite? . args)]
        [_ %residual-flinfinite?]))
    (define %residual-flinfinite? (lambda (a0) (%builtin-flinfinite? a0)))

    (define (flinteger? x)
      (and (%builtin-flfinite? x)
           (%builtin-fl=? x (%builtin-flfloor x))))
    (define (flodd? x)
      (and (flinteger? x)
           (%builtin-fl=? (%builtin-flabs (flremainder x 2.0)) 1.0)))
    (define (fleven? x)
      (and (flinteger? x)
           (%builtin-fl=? (flremainder x 2.0) 0.0)))

    (define-syntax %cover-fl+
      (syntax-rules ()
        [(_) 0.0]
        [(_ . args) (%builtin-fl+ . args)]
        [_ %residual-fl+]))
    (define %residual-fl+ (lambda (a0 a1) (%builtin-fl+ a0 a1)))
    (define-syntax %cover-fl-
      (syntax-rules ()
        [(_ a) (%builtin-flneg a)]
        [(_ . args) (%builtin-fl- . args)]
        [_ %residual-fl-]))
    (define %residual-fl- (lambda (a0 a1) (%builtin-fl- a0 a1)))
    (define-syntax %cover-fl*
      (syntax-rules ()
        [(_) 1.0]
        [(_ . args) (%builtin-fl* . args)]
        [_ %residual-fl*]))
    (define %residual-fl* (lambda (a0 a1) (%builtin-fl* a0 a1)))
    (define-syntax %cover-fl/
      (syntax-rules ()
        [(_ a) (%builtin-fl/ 1.0 a)]
        [(_ . args) (%builtin-fl/ . args)]
        [_ %residual-fl/]))
    (define %residual-fl/ (lambda (a0 a1) (%builtin-fl/ a0 a1)))
    (define-syntax %cover-flneg
      (syntax-rules ()
        [(_ . args) (%builtin-flneg . args)]
        [_ %residual-flneg]))
    (define %residual-flneg (lambda (a0) (%builtin-flneg a0)))
    (define-syntax %cover-flabs
      (syntax-rules ()
        [(_ . args) (%builtin-flabs . args)]
        [_ %residual-flabs]))
    (define %residual-flabs (lambda (a0) (%builtin-flabs a0)))

    (define (flquotient a b)
      (%builtin-fltruncate (%builtin-fl/ a b)))
    (define (flremainder a b)
      (%builtin-fl- a (%builtin-fl* (flquotient a b) b)))
    (define (flmodquo a b)
      (%builtin-flfloor (%builtin-fl/ a b)))
    (define (flmodulo a b)
      (%builtin-fl- a (%builtin-fl* (flmodquo a b) b)))

    (define-syntax %cover-flmin
      (syntax-rules ()
        [(_ . args) (%builtin-flmin . args)]
        [_ %residual-flmin]))
    (define %residual-flmin (lambda (a0 a1) (%builtin-flmin a0 a1)))
    (define-syntax %cover-flmax
      (syntax-rules ()
        [(_ . args) (%builtin-flmax . args)]
        [_ %residual-flmax]))
    (define %residual-flmax (lambda (a0 a1) (%builtin-flmax a0 a1)))

    (define (flgcd . args)
      (let loop ([acc 0.0] [l args])
        (if (null? l)
            acc
            (loop (let gcd2 ([a acc] [b (%builtin-flabs (car l))])
                    (if (%builtin-flzero? b)
                        a
                        (gcd2 b (flmodulo a b))))
                  (cdr l)))))

    (define-syntax %cover-flexpt
      (syntax-rules ()
        [(_ . args) (%builtin-flexpt . args)]
        [_ %residual-flexpt]))
    (define %residual-flexpt (lambda (a0 a1) (%builtin-flexpt a0 a1)))
    (define-syntax %cover-flsqrt
      (syntax-rules ()
        [(_ . args) (%builtin-flsqrt . args)]
        [_ %residual-flsqrt]))
    (define %residual-flsqrt (lambda (a0) (%builtin-flsqrt a0)))
    (define-syntax %cover-flfloor
      (syntax-rules ()
        [(_ . args) (%builtin-flfloor . args)]
        [_ %residual-flfloor]))
    (define %residual-flfloor (lambda (a0) (%builtin-flfloor a0)))
    (define-syntax %cover-flceiling
      (syntax-rules ()
        [(_ . args) (%builtin-flceiling . args)]
        [_ %residual-flceiling]))
    (define %residual-flceiling (lambda (a0) (%builtin-flceiling a0)))
    (define-syntax %cover-fltruncate
      (syntax-rules ()
        [(_ . args) (%builtin-fltruncate . args)]
        [_ %residual-fltruncate]))
    (define %residual-fltruncate (lambda (a0) (%builtin-fltruncate a0)))
    (define-syntax %cover-flround
      (syntax-rules ()
        [(_ . args) (%builtin-flround . args)]
        [_ %residual-flround]))
    (define %residual-flround (lambda (a0) (%builtin-flround a0)))
    (define-syntax %cover-flexp
      (syntax-rules ()
        [(_ . args) (%builtin-flexp . args)]
        [_ %residual-flexp]))
    (define %residual-flexp (lambda (a0) (%builtin-flexp a0)))

    (define (fllog x . rest)
      (if (pair? rest)
          (%builtin-fllog x (car rest))
          (%builtin-fllog x)))
    (define (fllog10 x) (%builtin-fllog10 x))

    (define-syntax %cover-flsin
      (syntax-rules ()
        [(_ . args) (%builtin-flsin . args)]
        [_ %residual-flsin]))
    (define %residual-flsin (lambda (a0) (%builtin-flsin a0)))
    (define-syntax %cover-flcos
      (syntax-rules ()
        [(_ . args) (%builtin-flcos . args)]
        [_ %residual-flcos]))
    (define %residual-flcos (lambda (a0) (%builtin-flcos a0)))
    (define-syntax %cover-fltan
      (syntax-rules ()
        [(_ . args) (%builtin-fltan . args)]
        [_ %residual-fltan]))
    (define %residual-fltan (lambda (a0) (%builtin-fltan a0)))
    (define-syntax %cover-flasin
      (syntax-rules ()
        [(_ . args) (%builtin-flasin . args)]
        [_ %residual-flasin]))
    (define %residual-flasin (lambda (a0) (%builtin-flasin a0)))
    (define-syntax %cover-flacos
      (syntax-rules ()
        [(_ . args) (%builtin-flacos . args)]
        [_ %residual-flacos]))
    (define %residual-flacos (lambda (a0) (%builtin-flacos a0)))

    (define (flatan y . x)
      (if (pair? x)
          (%builtin-flatan y (car x))
          (%builtin-flatan y)))

    (define-syntax %cover-flsinh
      (syntax-rules ()
        [(_ . args) (%builtin-flsinh . args)]
        [_ %residual-flsinh]))
    (define %residual-flsinh (lambda (a0) (%builtin-flsinh a0)))
    (define-syntax %cover-flcosh
      (syntax-rules ()
        [(_ . args) (%builtin-flcosh . args)]
        [_ %residual-flcosh]))
    (define %residual-flcosh (lambda (a0) (%builtin-flcosh a0)))
    (define-syntax %cover-fltanh
      (syntax-rules ()
        [(_ . args) (%builtin-fltanh . args)]
        [_ %residual-fltanh]))
    (define %residual-fltanh (lambda (a0) (%builtin-fltanh a0)))
    (define-syntax %cover-flldexp
      (syntax-rules ()
        [(_ . args) (%builtin-flldexp . args)]
        [_ %residual-flldexp]))
    (define %residual-flldexp (lambda (a0 a1) (%builtin-flldexp a0 a1)))

    (define-syntax %cover-flmodf
      (syntax-rules ()
        [(_ . args) (%builtin-flmodf . args)]
        [_ %residual-flmodf]))
    (define %residual-flmodf (lambda (a0 a1) (%builtin-flmodf a0 a1)))
    (define-syntax %cover-flfrexp
      (syntax-rules ()
        [(_ . args) (%builtin-flfrexp . args)]
        [_ %residual-flfrexp]))
    (define %residual-flfrexp (lambda (a0 a1) (%builtin-flfrexp a0 a1)))
    (define-syntax %cover-flonum->fixnum
      (syntax-rules ()
        [(_ . args) (%builtin-flonum->fixnum . args)]
        [_ %residual-flonum->fixnum]))
    (define %residual-flonum->fixnum
      (lambda (a0) (%builtin-flonum->fixnum a0)))
))
