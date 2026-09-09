
(define-library (skint fx)

  (import
    (only (skint)
      fixnum?
      fx=? fx<? fx>? fx<=? fx>=? fx!=?
      fxzero? fxpositive? fxnegative? fxodd? fxeven?
      fx+ fx- fx* fx/ fxneg fxabs
      fxquotient fxremainder
      fxmodquo fxmodulo
      fxeucquo fxeucrem
      fxmin fxmax
      fxgcd fxexpt %fxsqrt
      fxaddc fxsubc fxmulc fxfmar
      fxnot fxand fxior fxxor fxeqv
      fxsll fxsra fxsrl
      fxlength fxbit-count
      fixnum->flonum))

  (export
    fixnum?
    fx=? fx<? fx>? fx<=? fx>=? fx!=?
    fxzero? fxpositive? fxnegative? fxodd? fxeven?
    fx+ fx- fx* fx/ fxneg fxabs
    fxquotient fxremainder
    fxmodquo fxmodulo
    fxeucquo fxeucrem
    fxmin fxmax
    fxgcd fxexpt %fxsqrt
    fxaddc fxsubc fxmulc fxfmar
    fxnot fxand fxior fxxor fxeqv
    fxsll fxsra fxsrl
    fxlength fxbit-count
    fixnum->flonum))
