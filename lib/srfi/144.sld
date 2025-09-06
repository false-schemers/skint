
(define-library (srfi 144)
  (export
   fl-e
   fl-1/e
   fl-e-2
   fl-e-pi/4
   fl-log2-e
   fl-log10-e
   fl-log-2
   fl-1/log-2
   fl-log-3
   fl-log-pi
   fl-log-10
   fl-1/log-10
   fl-pi
   fl-1/pi
   fl-2pi
   fl-pi/2
   fl-pi/4
   fl-2/sqrt-pi
   fl-pi-squared
   fl-degree
   fl-2/pi
   fl-sqrt-2
   fl-sqrt-3
   fl-sqrt-5
   fl-sqrt-10
   fl-1/sqrt-2
   fl-cbrt-2
   fl-cbrt-3
   fl-4thrt-2
   fl-phi
   fl-log-phi
   fl-1/log-phi
   fl-euler
   fl-e-euler
   fl-sin-1
   fl-cos-1
   fl-gamma-1/2
   fl-gamma-1/3
   fl-gamma-2/3
   fl-greatest
   fl-least
   fl-epsilon
   fl-fast-fl+*
   fl-integer-exponent-zero
   fl-integer-exponent-nan
   flonum
   fladjacent              ; c99-math builtin
   flcopysign              ; c99-math builtin
   make-flonum
   flinteger-fraction
   flexponent              ; c99-math builtin
   flinteger-exponent
   flnormalized-fraction-exponent
   flsign-bit              ; c99-math builtin
   flonum?                 ; builtin
   fl=?                    ; builtin
   fl<?                    ; builtin
   fl>?                    ; builtin
   fl<=?                   ; builtin
   fl>=?                   ; builtin
   flunordered?
   flmax                   ; builtin
   flmin                   ; builtin
   flinteger?              ; builtin
   flzero?                 ; builtin
   flpositive?             ; builtin
   flnegative?             ; builtin
   flodd?                  ; builtin
   fleven?                 ; builtin
   flfinite?               ; builtin
   flinfinite?             ; builtin
   flnan?                  ; builtin
   flnormalized?           ; c99-math builtin
   fldenormalized?         ; c99-math builtin
   fl+                     ; builtin
   fl*                     ; builtin
   fl+*                    ; c99-math builtin
   fl-                     ; builtin 
   fl/                     ; builtin
   flabs                   ; builtin
   flabsdiff
   flposdiff               ; c99-math builtin
   flsgn
   flnumerator
   fldenominator
   flfloor                 ; builtin
   flceiling               ; builtin
   flround                 ; builtin
   fltruncate              ; builtin
   flexp                   ; builtin
   flexp2                  ; c99-math builtin
   flexp-1                 ; c99-math builtin
   flsquare
   flsqrt                  ; builtin
   flcbrt                  ; c99-math builtin
   flhypot                 ; c99-math builtin
   flexpt                  ; builtin
   fllog                   ; builtin
   fllog1+                 ; c99-math builtin
   fllog2                  ; c99-math builtin
   fllog10                 ; builtin
   make-fllog-base
   flsin                   ; builtin
   flcos                   ; builtin
   fltan                   ; builtin
   flasin                  ; builtin
   flacos                  ; builtin
   flatan                  ; builtin
   flsinh                  ; builtin
   flcosh                  ; builtin
   fltanh                  ; builtin
   flasinh                 ; c99-math builtin
   flacosh                 ; c99-math builtin
   flatanh                 ; c99-math builtin
   flquotient              ; builtin
   flremainder             ; builtin
   flremquo
   flgamma                 ; c99-math builtin
   flloggamma              ; c99-math builtin
   flfirst-bessel          ; xsi-math builtin
   flsecond-bessel         ; xsi-math builtin
   flerf                   ; c99-math builtin
   flerfc)                 ; c99-math builtin

  (import (skint))

(cond-expand
  (c99-math (import (skint c99-math))
; re-exported directly:  
;   fladjacent
;   flcopysign
;   flexponent
;   flsign-bit
;   flnormalized?
;   fldenormalized?
;   fl+*
;   flposdiff
;   flexp2
;   flexp-1
;   flcbrt
;   flhypot
;   fllog1+
;   fllog2
;   flasinh
;   flacosh
;   flatanh
;   flremquo
;   flgamma
;   flerf
;   flerfc
; also imported:
;   fllgamma
;   flilogb
;   %flremquo
))
(cond-expand
  (xsi-math (import (skint xsi-math))
; re-exported directly:  
;   flfirst-bessel
;   flsecond-bessel
))

(begin
;;; [willc] Flonum Constants
;;; The values are derived from the Chicken mathh.egg, but are not subject to copyright.

;; The precision is greater than those found in <math.h>
;; but in practice not perfectly representable

(define fl-e            2.7182818284590452353602874713526624977572)   ; e
(define fl-1/e          0.3678794411714423215955237701614608674458)   ; 1/e
(define fl-e-2          7.3890560989306502272304274605750078131803)   ; e^2
(define fl-e-pi/4       2.1932800507380154565597696592787382234616)   ; e^(pi/4)
(define fl-log2-e       1.44269504088896340735992468100189214)        ; log_2(e)
(define fl-log10-e      0.434294481903251827651128918916605082)       ; log_10(e)
(define fl-log-2        0.6931471805599453094172321214581765680755)   ; ln(2)
(define fl-1/log-2      1.4426950408889634073599246810018921374266)   ; 1/ln(2)
(define fl-log-3        1.0986122886681096913952452369225257046475)   ; ln(3)
(define fl-log-pi       1.1447298858494001741434273513530587116473)   ; ln(pi)
(define fl-log-10       2.3025850929940456840179914546843642076011)   ; ln(10)
(define fl-1/log-10     0.4342944819032518276511289189166050822944)   ; 1/ln(10)
(define fl-pi           3.1415926535897932384626433832795028841972)   ; pi
(define fl-1/pi         0.3183098861837906715377675267450287240689)   ; 1/pi
(define fl-2pi          6.2831853071795862319959269370883703231812)   ; pi * 2
(define fl-pi/2         1.57079632679489661923132169163975144)        ; pi/2
(define fl-2/pi         0.636619772367581343075535053490057448)       ; 2/pi
(define fl-pi/4         0.785398163397448309615660845819875721)       ; pi/4
(define fl-2/sqrt-pi    1.12837916709551257389615890312154517)        ; 2/sqrt(pi)
(define fl-sqrt-pi      1.7724538509055160272981674833411451827975)   ; sqrt(pi)
(define fl-pi-squared   9.8696044010893586188344909998761511353137)   ; pi^2
(define fl-degree       0.0174532925199432957692369076848861271344)   ; pi/180
(define fl-gamma-1/2    1.7724538509055160272981674833411451827975)   ; gamma(1/2)
(define fl-gamma-1/3    2.6789385347077476336556929409746776441287)   ; gamma(1/3)
(define fl-gamma-2/3    1.3541179394264004169452880281545137855193)   ; gamma(2/3)
(define fl-sqrt-2       1.4142135623730950488016887242096980785697)   ; sqrt(2)
(define fl-sqrt-3       1.7320508075688772935274463415058723669428)   ; sqrt(3)
(define fl-sqrt-5       2.2360679774997896964091736687312762354406)   ; sqrt(5)
(define fl-sqrt-10      3.1622776601683793319988935444327185337196)   ; sqrt(10)
(define fl-cbrt-2       1.2599210498948731647672106072782283505703)   ; cubert(2)
(define fl-cbrt-3       1.4422495703074083823216383107801095883919)   ; cubert(3)
(define fl-4thrt-2      1.1892071150027210667174999705604759152930)   ; fourthrt(2)
(define fl-1/sqrt-2     0.7071067811865475244008443621048490392848)   ; 1/sqrt(2)
(define fl-phi          1.6180339887498948482045868343656381177203)   ; phi
(define fl-log-phi      0.4812118250596034474977589134243684231352)   ; ln(phi)
(define fl-1/log-phi    2.0780869212350275376013226061177957677422)   ; 1/ln(phi)
(define fl-euler        0.5772156649015328606065120900824024310422)   ; euler
(define fl-e-euler      1.7810724179901979852365041031071795491696)   ; e^euler
(define fl-sin-1        0.8414709848078965066525023216302989996226)   ; sin(1)
(define fl-cos-1        0.5403023058681397174009366074429766037323)   ; cos(1)

;; [willc], [esl] regular ops replaced by fl ops

(define precision-bits ; IEEE double has 53 bits of precision
  (let loop ((bits 0) (x 1.0))
    (if (fl=? x (fl+ x 1.0)) bits (loop (fx+ bits 1) (fl* 2.0 x)))))

(define fl-greatest
  (let loop ((x (fl- (flexpt 2.0 (inexact precision-bits)) 1.0)))
    (if (flfinite? (fl* 2.0 x)) (loop (fl* 2.0 x)) x)))

(define fl-least
  (let loop ((x 1.0))
    (if (fl>? (fl/ x 2.0) 0.0) (loop (fl/ x 2.0)) x)))

(define fl-epsilon
  (let loop ((eps 1.0))
    (if (fl=? 1.0 (fl+ 1.0 eps)) (fl* 2.0 eps) (loop (fl/ eps 2.0)))))

(define fl-integer-exponent-zero                ; arbitrary
  (exact (fl- (fllog fl-least 2.0) 1.0)))

(define fl-integer-exponent-nan                 ; arbitrary
  (fx- fl-integer-exponent-zero 1))

(define (flonum x) 
  (if (real? x) (inexact x) +nan.0))

(define (make-flonum x n)
  (flldexp x (exact n))) 

(define (flinteger-fraction x)
  (let* ([i 0.0] [f (flmodf x (set& i))]) (values i f)))

(define (flnormalized-fraction-exponent x)
  (let* ([p 0] [f (flfrexp x (set& p))]) (values f p)))

(define (flsquare x) (fl* x x))

(define (flsgn x) (flcopysign 1.0 x))

(define (flabsdiff x y) (flabs (fl- x y)))

(define (flunordered? x y)
  (or (flnan? x) (flnan? y)))

(define (make-fllog-base base) ;[esl] removed flop1
  (if (fl>? base 1.0)
      (lambda (x) (fllog x base))
      (error "argument to make-fllog-base must be greater than 1.0" base)))

;[esl]
(define (flnumerator x)
  (cond [(flnan? x) x]
        [(flinfinite? x) x]
        [else (let lp ([x x] [n 1.0]) (if (flinteger? x) x (lp (fl* x 2.0) (fl* n 2.0))))]))

;[esl]
(define (fldenominator x)
  (cond [(flnan? x) x]
        [(flinfinite? x) 1.0]
        [else (let lp ([x x] [n 1.0]) (if (flinteger? x) n (lp (fl* x 2.0) (fl* n 2.0))))]))
              
)

(cond-expand
((or (not c99-math) (not xsi-math))
(include "144/polyfill.scm")
))


(cond-expand
(c99-math
(begin
(define fl-fast-fl+* #t)

(define (flinteger-exponent x)
  (cond [(flzero? x) fl-integer-exponent-zero]
        [(flnan? x) fl-integer-exponent-nan]
        [else (flilogb x)]))

(define (flremquo x y)
  (let* ([q 0] [r (%flremquo x y (set& q))]) (values r q)))
        
(define (flloggamma x)
  (values (fllgamma x) ;[shirok], [esl] changed to fl ops
    (cond [(fl<=? 0.0 x) 1.0]
          [(flinfinite? x) +nan.0] ; sign of gamma(-inf.0) undecidable
          [(flnan? x) +nan.0]
          [(flodd? (flfloor x)) -1.0]
          [else 1.0])))
))
(else
(include "144/c99-polyfill.scm")
))

(cond-expand
(xsi-math
(begin
  ; all is re-exported
))
(else
(include "144/xsi-polyfill.scm")
))

;;; parts above marked as [willc] are
;;; Copyright (C) William D Clinger (2016).
;;;
;;; Permission is hereby granted, free of charge, to any person
;;; obtaining a copy of this software and associated documentation
;;; files (the "Software"), to deal in the Software without
;;; restriction, including without limitation the rights to use,
;;; copy, modify, merge, publish, distribute, sublicense, and/or
;;; sell copies of the Software, and to permit persons to whom the
;;; Software is furnished to do so, subject to the following
;;; conditions:
;;;
;;; The above copyright notice and this permission notice shall be
;;; included in all copies or substantial portions of the Software.
;;;
;;; THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
;;; EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
;;; OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
;;; NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
;;; HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
;;; WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
;;; FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
;;; OTHER DEALINGS IN THE SOFTWARE.

;;; parts above marked [shirok] are
;;;   Copyright (c) 2019-2025  Shiro Kawai  <shiro@acm.org>
;;;
;;;   Redistribution and use in source and binary forms, with or without
;;;   modification, are permitted provided that the following conditions
;;;   are met:
;;;
;;;   1. Redistributions of source code must retain the above copyright
;;;      notice, this list of conditions and the following disclaimer.
;;;
;;;   2. Redistributions in binary form must reproduce the above copyright
;;;      notice, this list of conditions and the following disclaimer in the
;;;      documentation and/or other materials provided with the distribution.
;;;
;;;   3. Neither the name of the authors nor the names of its contributors
;;;      may be used to endorse or promote products derived from this
;;;      software without specific prior written permission.
;;;
;;;   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
;;;   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
;;;   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
;;;   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
;;;   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
;;;   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
;;;   TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
;;;   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
;;;   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
;;;   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
;;;   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

)
