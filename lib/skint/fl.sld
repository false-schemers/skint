
(define-library (skint fl)

  (import
    (only (skint)
      flonum?
      fl=? fl<? fl>? fl<=? fl>=? fl!=?
      flzero? flpositive? flnegative? flodd? fleven?
      flinteger? flnan? flfinite? flinfinite?
      fl+ fl- fl* fl/ flneg flabs
      flquotient flremainder flmodquo flmodulo
      flmin flmax
      flgcd flexpt flsqrt
      flfloor flceiling fltruncate flround
      flexp fllog fllog10
      flsin flcos fltan flasin flacos flatan
      flsinh flcosh fltanh
      flldexp flmodf flfrexp
      flonum->fixnum))

  (export
    flonum?
    fl=? fl<? fl>? fl<=? fl>=? fl!=?
    flzero? flpositive? flnegative? flodd? fleven?
    flinteger? flnan? flfinite? flinfinite?
    fl+ fl- fl* fl/ flneg flabs
    flquotient flremainder flmodquo flmodulo
    flmin flmax
    flgcd flexpt flsqrt
    flfloor flceiling fltruncate flround
    flexp fllog fllog10
    flsin flcos fltan flasin flacos flatan
    flsinh flcosh fltanh
    flldexp flmodf flfrexp
    flonum->fixnum)

  ;; available only where the host provides the C99 <math.h> functions

  (cond-expand
    (c99-math
      (import
        (only (skint c99-math)
          flcopysign flsign-bit fladjacent
          flnormalized? fldenormalized?
          flexponent flilogb
          fl+* flposdiff
          flexp2 flexp-1 flcbrt flhypot
          fllog1+ fllog2
          flasinh flacosh flatanh
          %flremquo
          flgamma fllgamma flerf flerfc))
      (export
        flcopysign flsign-bit fladjacent
        flnormalized? fldenormalized?
        flexponent flilogb
        fl+* flposdiff
        flexp2 flexp-1 flcbrt flhypot
        fllog1+ fllog2
        flasinh flacosh flatanh
        %flremquo
        flgamma fllgamma flerf flerfc))
    (else))

  ;; available only where the host provides the XSI extensions to C99

  (cond-expand
    (xsi-math
      (import (only (skint xsi-math) flfirst-bessel flsecond-bessel))
      (export flfirst-bessel flsecond-bessel))
    (else)))
