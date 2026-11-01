;;; fx.sld -- (skint fx) as a real library, for compiled programs.
;;;
;;; Shadow counterpart of Skint's (skint fx) (itself a pure re-export
;;; of (skint) builtins).  Every exported name here is backed by a DS
;;; prim: call-position uses expand through cover macros to the Skint
;;; builtin (imported renamed as %builtin-<name>), which the compiler
;;; then recognises as the prim; a bare identifier expands to
;;; %residual-<name>, an ordinary definition in this library (so the
;;; compiled program owns a real closure).  Skint's fx operations are
;;; wraparound fixnum operations: fx+/fx-/fx* ride the $scmi:iadd /
;;; isub / imul macros, and the n-ary fixnum comparisons ride the
;;; compiler's pairwise-chain handling for fx*/fl* comparators, so
;;; cover macros (which forward any operands) have Skint's variadic
;;; surface; the 0-argument identities are spelled by 0-argument
;;; cover clauses.  Residuals are fixed-arity (2, or 1 for unary
;;; operations): a residual called with more arguments through apply
;;; is not Skint's procedure, an accepted limitation.
;;;
;;; Operations with no binary DS prim are plain library procedures:
;;; fxfmar (exact wide intermediate on the generic operators), fx/
;;; (left fold of fxquotient), fxeucquo/fxeucrem (from the truncating
;;; pair), fxgcd (Euclid on fxmodulo), fxexpt (0 on overflow, per
;;; Skint), and the box-writing operations %fxsqrt / fxaddc / fxsubc /
;;; fxmulc, whose call positions reach the compiler's box-prim
;;; handling (the box operand is usually (set& var)).
;;; immediate-hash is Skint integrable 591 (the hshim instruction),
;;; so imported unrenamed (as (scheme base) imports fx+) its
;;; applications arrive as primapps and hit the DS prim table's
;;; $scmi:immhash entry directly.

(define-library (skint fx)
  (import (only (skint) define lambda define-syntax syntax-rules if let
                  let* and or when unless cond begin set! quote _ ...)
          (only (skint) + * remainder pair? null? car cdr not)
          (only (skint hidden) immediate-hash)
          ;; flonum? is re-exported from (skint fl) so that a program
          ;; importing both libraries sees the one binding Skint has
          (only (skint fl) flonum?)
          (rename (only (skint) fixnum? fx+ fx- fx* fx=?
                          fx<? fx<=? fx>? fx>=? fx!=? fxzero?
                          fxpositive? fxnegative? fxodd? fxeven?
                          fxneg fxabs fxquotient fxremainder fxmodquo
                          fxmodulo fxmin fxmax fxnot fxand fxior fxxor
                          fxeqv fxsll fxsra fxsrl fxlength fxbit-count
                          fixnum->flonum flonum->fixnum flfloor fl=?
                          expt fxaddc fxsubc fxmulc %fxsqrt
                          fl<? fl>?)
            (fixnum? %builtin-fixnum?)
            (fx+ %builtin-fx+)
            (fx- %builtin-fx-)
            (fx* %builtin-fx*)
            (fx=? %builtin-fx=?)
            (fx<? %builtin-fx<?)
            (fx<=? %builtin-fx<=?)
            (fx>? %builtin-fx>?)
            (fx>=? %builtin-fx>=?)
            (fx!=? %builtin-fx!=?)
            (fxzero? %builtin-fxzero?)
            (fxpositive? %builtin-fxpositive?)
            (fxnegative? %builtin-fxnegative?)
            (fxodd? %builtin-fxodd?)
            (fxeven? %builtin-fxeven?)
            (fxneg %builtin-fxneg)
            (fxabs %builtin-fxabs)
            (fxquotient %builtin-fxquotient)
            (fxremainder %builtin-fxremainder)
            (fxmodquo %builtin-fxmodquo)
            (fxmodulo %builtin-fxmodulo)
            (fxmin %builtin-fxmin)
            (fxmax %builtin-fxmax)
            (fxnot %builtin-fxnot)
            (fxand %builtin-fxand)
            (fxior %builtin-fxior)
            (fxxor %builtin-fxxor)
            (fxeqv %builtin-fxeqv)
            (fxsll %builtin-fxsll)
            (fxsra %builtin-fxsra)
            (fxsrl %builtin-fxsrl)
            (fxlength %builtin-fxlength)
            (fxbit-count %builtin-fxbit-count)
            (fixnum->flonum %builtin-fixnum->flonum)
            (flonum->fixnum %builtin-flonum->fixnum)
            (flfloor %builtin-flfloor)
            (fl=? %builtin-fl=?)
            (fl<? %builtin-fl<?)
            (fl>? %builtin-fl>?)
            (expt %builtin-expt)
            (fxaddc %builtin-fxaddc)
            (fxsubc %builtin-fxsubc)
            (fxmulc %builtin-fxmulc)
            (%fxsqrt %builtin-%fxsqrt)))

  (export
    (rename %cover-fixnum? fixnum?)
    flonum?
    (rename %cover-fx=? fx=?)
    (rename %cover-fx<? fx<?)
    (rename %cover-fx>? fx>?)
    (rename %cover-fx<=? fx<=?)
    (rename %cover-fx>=? fx>=?)
    (rename %cover-fx!=? fx!=?)
    (rename %cover-fxzero? fxzero?)
    (rename %cover-fxpositive? fxpositive?)
    (rename %cover-fxnegative? fxnegative?)
    (rename %cover-fxodd? fxodd?)
    (rename %cover-fxeven? fxeven?)
    (rename %cover-fx+ fx+)
    (rename %cover-fx- fx-)
    (rename %cover-fx* fx*)
    fx/
    (rename %cover-fxneg fxneg)
    (rename %cover-fxabs fxabs)
    (rename %cover-fxquotient fxquotient)
    (rename %cover-fxremainder fxremainder)
    (rename %cover-fxmodquo fxmodquo)
    (rename %cover-fxmodulo fxmodulo)
    fxeucquo fxeucrem
    (rename %cover-fxmin fxmin)
    (rename %cover-fxmax fxmax)
    fxgcd fxexpt %fxsqrt
    (rename %cover-fxaddc fxaddc)
    (rename %cover-fxsubc fxsubc)
    (rename %cover-fxmulc fxmulc)
    fxfmar
    (rename %cover-fxnot fxnot)
    (rename %cover-fxand fxand)
    (rename %cover-fxior fxior)
    (rename %cover-fxxor fxxor)
    (rename %cover-fxeqv fxeqv)
    (rename %cover-fxsll fxsll)
    (rename %cover-fxsra fxsra)
    (rename %cover-fxsrl fxsrl)
    (rename %cover-fxlength fxlength)
    (rename %cover-fxbit-count fxbit-count)
    (rename %cover-fixnum->flonum fixnum->flonum)
    (rename %cover-immediate-hash immediate-hash))

  (begin
    (define-syntax %cover-fixnum?
      (syntax-rules ()
        [(_ . args) (%builtin-fixnum? . args)]
        [_ %residual-fixnum?]))
    (define %residual-fixnum? (lambda (a0) (%builtin-fixnum? a0)))
    (define-syntax %cover-fx=?
      (syntax-rules ()
        [(_) #t]
        [(_ a) #t]
        [(_ . args) (%builtin-fx=? . args)]
        [_ %residual-fx=?]))
    (define %residual-fx=? (lambda (a0 a1) (%builtin-fx=? a0 a1)))
    (define-syntax %cover-fx<?
      (syntax-rules ()
        [(_) #t]
        [(_ a) #t]
        [(_ . args) (%builtin-fx<? . args)]
        [_ %residual-fx<?]))
    (define %residual-fx<? (lambda (a0 a1) (%builtin-fx<? a0 a1)))
    (define-syntax %cover-fx>?
      (syntax-rules ()
        [(_) #t]
        [(_ a) #t]
        [(_ . args) (%builtin-fx>? . args)]
        [_ %residual-fx>?]))
    (define %residual-fx>? (lambda (a0 a1) (%builtin-fx>? a0 a1)))
    (define-syntax %cover-fx<=?
      (syntax-rules ()
        [(_) #t]
        [(_ a) #t]
        [(_ . args) (%builtin-fx<=? . args)]
        [_ %residual-fx<=?]))
    (define %residual-fx<=? (lambda (a0 a1) (%builtin-fx<=? a0 a1)))
    (define-syntax %cover-fx>=?
      (syntax-rules ()
        [(_) #t]
        [(_ a) #t]
        [(_ . args) (%builtin-fx>=? . args)]
        [_ %residual-fx>=?]))
    (define %residual-fx>=? (lambda (a0 a1) (%builtin-fx>=? a0 a1)))
    (define-syntax %cover-fx!=?
      (syntax-rules ()
        [(_ . args) (%builtin-fx!=? . args)]
        [_ %residual-fx!=?]))
    (define %residual-fx!=? (lambda (a0 a1) (%builtin-fx!=? a0 a1)))

    (define-syntax %cover-fxzero?
      (syntax-rules ()
        [(_ . args) (%builtin-fxzero? . args)]
        [_ %residual-fxzero?]))
    (define %residual-fxzero? (lambda (a0) (%builtin-fxzero? a0)))
    (define-syntax %cover-fxpositive?
      (syntax-rules ()
        [(_ . args) (%builtin-fxpositive? . args)]
        [_ %residual-fxpositive?]))
    (define %residual-fxpositive? (lambda (a0) (%builtin-fxpositive? a0)))
    (define-syntax %cover-fxnegative?
      (syntax-rules ()
        [(_ . args) (%builtin-fxnegative? . args)]
        [_ %residual-fxnegative?]))
    (define %residual-fxnegative? (lambda (a0) (%builtin-fxnegative? a0)))
    (define-syntax %cover-fxodd?
      (syntax-rules ()
        [(_ . args) (%builtin-fxodd? . args)]
        [_ %residual-fxodd?]))
    (define %residual-fxodd? (lambda (a0) (%builtin-fxodd? a0)))
    (define-syntax %cover-fxeven?
      (syntax-rules ()
        [(_ . args) (%builtin-fxeven? . args)]
        [_ %residual-fxeven?]))
    (define %residual-fxeven? (lambda (a0) (%builtin-fxeven? a0)))

    (define-syntax %cover-fx+
      (syntax-rules ()
        [(_) 0]
        [(_ . args) (%builtin-fx+ . args)]
        [_ %residual-fx+]))
    (define %residual-fx+ (lambda (a0 a1) (%builtin-fx+ a0 a1)))
    (define-syntax %cover-fx-
      (syntax-rules ()
        [(_ a) (%builtin-fxneg a)]
        [(_ . args) (%builtin-fx- . args)]
        [_ %residual-fx-]))
    (define %residual-fx- (lambda (a0 a1) (%builtin-fx- a0 a1)))
    (define-syntax %cover-fx*
      (syntax-rules ()
        [(_) 1]
        [(_ . args) (%builtin-fx* . args)]
        [_ %residual-fx*]))
    (define %residual-fx* (lambda (a0 a1) (%builtin-fx* a0 a1)))

    (define (fx/ a . rest)
      (if (null? rest)
          (%builtin-fxquotient 1 a)
          (let loop ([acc a] [l rest])
            (if (null? l)
                acc
                (loop (%builtin-fxquotient acc (car l)) (cdr l))))))

    (define-syntax %cover-fxneg
      (syntax-rules ()
        [(_ . args) (%builtin-fxneg . args)]
        [_ %residual-fxneg]))
    (define %residual-fxneg (lambda (a0) (%builtin-fxneg a0)))
    (define-syntax %cover-fxabs
      (syntax-rules ()
        [(_ . args) (%builtin-fxabs . args)]
        [_ %residual-fxabs]))
    (define %residual-fxabs (lambda (a0) (%builtin-fxabs a0)))
    (define-syntax %cover-fxquotient
      (syntax-rules ()
        [(_ . args) (%builtin-fxquotient . args)]
        [_ %residual-fxquotient]))
    (define %residual-fxquotient
      (lambda (a0 a1) (%builtin-fxquotient a0 a1)))
    (define-syntax %cover-fxremainder
      (syntax-rules ()
        [(_ . args) (%builtin-fxremainder . args)]
        [_ %residual-fxremainder]))
    (define %residual-fxremainder
      (lambda (a0 a1) (%builtin-fxremainder a0 a1)))
    (define-syntax %cover-fxmodquo
      (syntax-rules ()
        [(_ . args) (%builtin-fxmodquo . args)]
        [_ %residual-fxmodquo]))
    (define %residual-fxmodquo (lambda (a0 a1) (%builtin-fxmodquo a0 a1)))
    (define-syntax %cover-fxmodulo
      (syntax-rules ()
        [(_ . args) (%builtin-fxmodulo . args)]
        [_ %residual-fxmodulo]))
    (define %residual-fxmodulo (lambda (a0 a1) (%builtin-fxmodulo a0 a1)))

    (define (fxeucquo a b)
      (let ([q (%builtin-fxquotient a b)] [r (%builtin-fxremainder a b)])
        (if (%builtin-fxnegative? r)
            (if (%builtin-fxpositive? b)
                (%builtin-fx- q 1)
                (%builtin-fx+ q 1))
            q)))
    (define (fxeucrem a b)
      (let ([r (%builtin-fxremainder a b)])
        (if (%builtin-fxnegative? r)
            (%builtin-fx+ r (%builtin-fxabs b))
            r)))

    (define-syntax %cover-fxmin
      (syntax-rules ()
        [(_ . args) (%builtin-fxmin . args)]
        [_ %residual-fxmin]))
    (define %residual-fxmin (lambda (a0 a1) (%builtin-fxmin a0 a1)))
    (define-syntax %cover-fxmax
      (syntax-rules ()
        [(_ . args) (%builtin-fxmax . args)]
        [_ %residual-fxmax]))
    (define %residual-fxmax (lambda (a0 a1) (%builtin-fxmax a0 a1)))

    (define (fxgcd . args)
      (let loop ([acc 0] [l args])
        (if (null? l)
            acc
            (loop (let gcd2 ([a acc] [b (car l)])
                    (if (%builtin-fxzero? b)
                        a
                        (gcd2 b (%builtin-fxmodulo a b))))
                  (cdr l)))))

    (define (fxexpt b e)
      ;; Skint: the exact result, or 0 when it overflows the fixnum
      ;; range; computed through a double, exact well past 2^30
      (let ([r (if (and (%builtin-fxzero? b) (%builtin-fxzero? e))
                   0.0
                   (%builtin-expt (%builtin-fixnum->flonum b)
                              (%builtin-fixnum->flonum e)))])
        (if (and (%builtin-fl=? r (%builtin-flfloor r))
                 (not (%builtin-fl<? r -536870912.0))
                 (not (%builtin-fl>? r 536870911.0)))
            (%builtin-flonum->fixnum r)
            0)))
    (define (%fxsqrt x . box)
      (if (pair? box)
          (%builtin-%fxsqrt x (car box))
          (%builtin-%fxsqrt x)))

    (define-syntax %cover-fxaddc
      (syntax-rules ()
        [(_ . args) (%builtin-fxaddc . args)]
        [_ %residual-fxaddc]))
    (define %residual-fxaddc
      (lambda (a0 a1 a2) (%builtin-fxaddc a0 a1 a2)))
    (define-syntax %cover-fxsubc
      (syntax-rules ()
        [(_ . args) (%builtin-fxsubc . args)]
        [_ %residual-fxsubc]))
    (define %residual-fxsubc
      (lambda (a0 a1 a2) (%builtin-fxsubc a0 a1 a2)))
    (define-syntax %cover-fxmulc
      (syntax-rules ()
        [(_ . args) (%builtin-fxmulc . args)]
        [_ %residual-fxmulc]))
    (define %residual-fxmulc
      (lambda (a0 a1 a2) (%builtin-fxmulc a0 a1 a2)))

    (define (fxfmar i j k r) (remainder (+ (* i j) k) r))

    (define-syntax %cover-fxnot
      (syntax-rules ()
        [(_ . args) (%builtin-fxnot . args)]
        [_ %residual-fxnot]))
    (define %residual-fxnot (lambda (a0) (%builtin-fxnot a0)))
    (define-syntax %cover-fxand
      (syntax-rules ()
        [(_) -1]
        [(_ . args) (%builtin-fxand . args)]
        [_ %residual-fxand]))
    (define %residual-fxand (lambda (a0 a1) (%builtin-fxand a0 a1)))
    (define-syntax %cover-fxior
      (syntax-rules ()
        [(_) 0]
        [(_ . args) (%builtin-fxior . args)]
        [_ %residual-fxior]))
    (define %residual-fxior (lambda (a0 a1) (%builtin-fxior a0 a1)))
    (define-syntax %cover-fxxor
      (syntax-rules ()
        [(_) 0]
        [(_ . args) (%builtin-fxxor . args)]
        [_ %residual-fxxor]))
    (define %residual-fxxor (lambda (a0 a1) (%builtin-fxxor a0 a1)))
    (define-syntax %cover-fxeqv
      (syntax-rules ()
        [(_) -1]
        [(_ . args) (%builtin-fxeqv . args)]
        [_ %residual-fxeqv]))
    (define %residual-fxeqv (lambda (a0 a1) (%builtin-fxeqv a0 a1)))
    (define-syntax %cover-fxsll
      (syntax-rules ()
        [(_ . args) (%builtin-fxsll . args)]
        [_ %residual-fxsll]))
    (define %residual-fxsll (lambda (a0 a1) (%builtin-fxsll a0 a1)))
    (define-syntax %cover-fxsra
      (syntax-rules ()
        [(_ . args) (%builtin-fxsra . args)]
        [_ %residual-fxsra]))
    (define %residual-fxsra (lambda (a0 a1) (%builtin-fxsra a0 a1)))
    (define-syntax %cover-fxsrl
      (syntax-rules ()
        [(_ . args) (%builtin-fxsrl . args)]
        [_ %residual-fxsrl]))
    (define %residual-fxsrl (lambda (a0 a1) (%builtin-fxsrl a0 a1)))
    (define-syntax %cover-fxlength
      (syntax-rules ()
        [(_ . args) (%builtin-fxlength . args)]
        [_ %residual-fxlength]))
    (define %residual-fxlength (lambda (a0) (%builtin-fxlength a0)))
    (define-syntax %cover-fxbit-count
      (syntax-rules ()
        [(_ . args) (%builtin-fxbit-count . args)]
        [_ %residual-fxbit-count]))
    (define %residual-fxbit-count (lambda (a0) (%builtin-fxbit-count a0)))
    (define-syntax %cover-fixnum->flonum
      (syntax-rules ()
        [(_ . args) (%builtin-fixnum->flonum . args)]
        [_ %residual-fixnum->flonum]))
    (define %residual-fixnum->flonum
      (lambda (a0) (%builtin-fixnum->flonum a0)))

    (define-syntax %cover-immediate-hash
      (syntax-rules ()
        [(_ . args) (immediate-hash . args)]
        [_ %residual-immediate-hash]))
    (define %residual-immediate-hash
      (lambda (a0 a1) (immediate-hash a0 a1)))
))
