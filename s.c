char *module_s[] = {
  "S", "eq?",
  "l4:y12:syntax-rules;n;l2:l3:y1:_;y1:x;y1:y;;l3:y4:%25isq;y1:x;y1:y;;;l"
  "2:y1:_;y13:%25residual-eq?;;",

  "P", "%residual-eq?",
  "&0{%2.1,.1q]2}",

  "S", "eqv?",
  "l4:y12:syntax-rules;n;l2:l3:y1:_;y1:x;y1:y;;l3:y4:%25isv;y1:x;y1:y;;;l"
  "2:y1:_;y14:%25residual-eqv?;;",

  "P", "%residual-eqv?",
  "&0{%2.1,.1v]2}",

  "S", "equal?",
  "l4:y12:syntax-rules;n;l2:l3:y1:_;y1:x;y1:y;;l3:y4:%25ise;y1:x;y1:y;;;l"
  "2:y1:_;y16:%25residual-equal?;;",

  "P", "%residual-equal?",
  "&0{%2.1,.1e]2}",

  "S", "fixnum?",
  "l4:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l2:y5:%25fixp;y1:x;;;l2:y1:_;y1"
  "7:%25residual-fixnum?;;",

  "P", "%residual-fixnum?",
  "&0{%1.0I0]1}",

  "S", "fxzero?",
  "l4:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l2:y7:%25izerop;l2:y4:%25cki;y1"
  ":x;;;;l2:y1:_;y17:%25residual-fxzero?;;",

  "P", "%residual-fxzero?",
  "&0{%1.0%iI=0]1}",

  "S", "fxpositive?",
  "l4:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l2:y6:%25iposp;l2:y4:%25cki;y1:"
  "x;;;;l2:y1:_;y21:%25residual-fxpositive?;;",

  "P", "%residual-fxpositive?",
  "&0{%1.0%iI>0]1}",

  "S", "fxnegative?",
  "l4:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l2:y6:%25inegp;l2:y4:%25cki;y1:"
  "x;;;;l2:y1:_;y21:%25residual-fxnegative?;;",

  "P", "%residual-fxnegative?",
  "&0{%1.0%iI<0]1}",

  "S", "fx+",
  "l4:y12:syntax-rules;n;l2:l3:y1:_;y1:x;y1:y;;l3:y5:%25iadd;l2:y4:%25cki"
  ";y1:x;;l2:y4:%25cki;y1:y;;;;l2:y1:_;y13:%25residual-fx+;;",

  "P", "%residual-fx+",
  "&0{%2.1%i,.1%iI+]2}",

  "S", "fx*",
  "l4:y12:syntax-rules;n;l2:l3:y1:_;y1:x;y1:y;;l3:y5:%25imul;l2:y4:%25cki"
  ";y1:x;;l2:y4:%25cki;y1:y;;;;l2:y1:_;y13:%25residual-fx*;;",

  "P", "%residual-fx*",
  "&0{%2.1%i,.1%iI*]2}",

  "S", "fx-",
  "l4:y12:syntax-rules;n;l2:l3:y1:_;y1:x;y1:y;;l3:y5:%25isub;l2:y4:%25cki"
  ";y1:x;;l2:y4:%25cki;y1:y;;;;l2:y1:_;y13:%25residual-fx-;;",

  "P", "%residual-fx-",
  "&0{%2.1%i,.1%iI-]2}",

  "S", "fx/",
  "l4:y12:syntax-rules;n;l2:l3:y1:_;y1:x;y1:y;;l3:y5:%25idiv;l2:y4:%25cki"
  ";y1:x;;l2:y4:%25cki;y1:y;;;;l2:y1:_;y13:%25residual-fx/;;",

  "P", "%residual-fx/",
  "&0{%2.1%i,.1%iI/]2}",

  "S", "fxquotient",
  "l4:y12:syntax-rules;n;l2:l3:y1:_;y1:x;y1:y;;l3:y5:%25iquo;l2:y4:%25cki"
  ";y1:x;;l2:y4:%25cki;y1:y;;;;l2:y1:_;y20:%25residual-fxquotient;;",

  "P", "%residual-fxquotient",
  "&0{%2.1%i,.1%iIq]2}",

  "S", "fxremainder",
  "l4:y12:syntax-rules;n;l2:l3:y1:_;y1:x;y1:y;;l3:y5:%25irem;l2:y4:%25cki"
  ";y1:x;;l2:y4:%25cki;y1:y;;;;l2:y1:_;y21:%25residual-fxremainder;;",

  "P", "%residual-fxremainder",
  "&0{%2.1%i,.1%iIr]2}",

  "S", "fxmodquo",
  "l4:y12:syntax-rules;n;l2:l3:y1:_;y1:x;y1:y;;l3:y5:%25imqu;l2:y4:%25cki"
  ";y1:x;;l2:y4:%25cki;y1:y;;;;l2:y1:_;y18:%25residual-fxmodquo;;",

  "P", "%residual-fxmodquo",
  "&0{%2.1%i,.1%iI3]2}",

  "S", "fxmodulo",
  "l4:y12:syntax-rules;n;l2:l3:y1:_;y1:x;y1:y;;l3:y5:%25imlo;l2:y4:%25cki"
  ";y1:x;;l2:y4:%25cki;y1:y;;;;l2:y1:_;y18:%25residual-fxmodulo;;",

  "P", "%residual-fxmodulo",
  "&0{%2.1%i,.1%iI4]2}",

  "S", "fxeucquo",
  "l4:y12:syntax-rules;n;l2:l3:y1:_;y1:x;y1:y;;l3:y5:%25ieuq;l2:y4:%25cki"
  ";y1:x;;l2:y4:%25cki;y1:y;;;;l2:y1:_;y18:%25residual-fxeucquo;;",

  "P", "%residual-fxeucquo",
  "&0{%2.1%i,.1%iI5]2}",

  "S", "fxeucrem",
  "l4:y12:syntax-rules;n;l2:l3:y1:_;y1:x;y1:y;;l3:y5:%25ieur;l2:y4:%25cki"
  ";y1:x;;l2:y4:%25cki;y1:y;;;;l2:y1:_;y18:%25residual-fxeucrem;;",

  "P", "%residual-fxeucrem",
  "&0{%2.1%i,.1%iI6]2}",

  "S", "fxneg",
  "l4:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l2:y5:%25ineg;l2:y4:%25cki;y1:x"
  ";;;;l2:y1:_;y15:%25residual-fxneg;;",

  "P", "%residual-fxneg",
  "&0{%1.0%iI-!]1}",

  "S", "fxabs",
  "l4:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l2:y5:%25iabs;l2:y4:%25cki;y1:x"
  ";;;;l2:y1:_;y15:%25residual-fxabs;;",

  "P", "%residual-fxabs",
  "&0{%1.0%iIa]1}",

  "S", "fx<?",
  "l4:y12:syntax-rules;n;l2:l3:y1:_;y1:x;y1:y;;l3:y4:%25ilt;l2:y4:%25cki;"
  "y1:x;;l2:y4:%25cki;y1:y;;;;l2:y1:_;y14:%25residual-fx<?;;",

  "P", "%residual-fx<?",
  "&0{%2.1%i,.1%iI<]2}",

  "S", "fx<=?",
  "l4:y12:syntax-rules;n;l2:l3:y1:_;y1:x;y1:y;;l3:y4:%25ile;l2:y4:%25cki;"
  "y1:x;;l2:y4:%25cki;y1:y;;;;l2:y1:_;y15:%25residual-fx<=?;;",

  "P", "%residual-fx<=?",
  "&0{%2.1%i,.1%iI>!]2}",

  "S", "fx>?",
  "l4:y12:syntax-rules;n;l2:l3:y1:_;y1:x;y1:y;;l3:y4:%25igt;l2:y4:%25cki;"
  "y1:x;;l2:y4:%25cki;y1:y;;;;l2:y1:_;y14:%25residual-fx>?;;",

  "P", "%residual-fx>?",
  "&0{%2.1%i,.1%iI>]2}",

  "S", "fx>=?",
  "l4:y12:syntax-rules;n;l2:l3:y1:_;y1:x;y1:y;;l3:y4:%25ige;l2:y4:%25cki;"
  "y1:x;;l2:y4:%25cki;y1:y;;;;l2:y1:_;y15:%25residual-fx>=?;;",

  "P", "%residual-fx>=?",
  "&0{%2.1%i,.1%iI<!]2}",

  "S", "fx=?",
  "l4:y12:syntax-rules;n;l2:l3:y1:_;y1:x;y1:y;;l3:y4:%25ieq;l2:y4:%25cki;"
  "y1:x;;l2:y4:%25cki;y1:y;;;;l2:y1:_;y14:%25residual-fx=?;;",

  "P", "%residual-fx=?",
  "&0{%2.1%i,.1%iI=]2}",

  "S", "fxmin",
  "l4:y12:syntax-rules;n;l2:l3:y1:_;y1:x;y1:y;;l3:y5:%25imin;l2:y4:%25cki"
  ";y1:x;;l2:y4:%25cki;y1:y;;;;l2:y1:_;y15:%25residual-fxmin;;",

  "P", "%residual-fxmin",
  "&0{%2.1%i,.1%iIn]2}",

  "S", "fxmax",
  "l4:y12:syntax-rules;n;l2:l3:y1:_;y1:x;y1:y;;l3:y5:%25imax;l2:y4:%25cki"
  ";y1:x;;l2:y4:%25cki;y1:y;;;;l2:y1:_;y15:%25residual-fxmax;;",

  "P", "%residual-fxmax",
  "&0{%2.1%i,.1%iIx]2}",

  "S", "fixnum->flonum",
  "l4:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l2:y5:%25itoj;l2:y4:%25cki;y1:x"
  ";;;;l2:y1:_;y24:%25residual-fixnum->flonum;;",

  "P", "%residual-fixnum->flonum",
  "&0{%1.0%iIj]1}",

  "S", "flonum?",
  "l4:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l2:y5:%25flop;y1:x;;;l2:y1:_;y1"
  "7:%25residual-flonum?;;",

  "P", "%residual-flonum?",
  "&0{%1.0J0]1}",

  "S", "flzero?",
  "l4:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l2:y7:%25jzerop;l2:y4:%25ckj;y1"
  ":x;;;;l2:y1:_;y17:%25residual-flzero?;;",

  "P", "%residual-flzero?",
  "&0{%1.0%jJ=0]1}",

  "S", "flpositive?",
  "l4:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l2:y6:%25jposp;l2:y4:%25ckj;y1:"
  "x;;;;l2:y1:_;y21:%25residual-flpositive?;;",

  "P", "%residual-flpositive?",
  "&0{%1.0%jJ>0]1}",

  "S", "flnegative?",
  "l4:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l2:y6:%25jnegp;l2:y4:%25ckj;y1:"
  "x;;;;l2:y1:_;y21:%25residual-flnegative?;;",

  "P", "%residual-flnegative?",
  "&0{%1.0%jJ<0]1}",

  "S", "flinteger?",
  "l4:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l2:y6:%25jintp;l2:y4:%25ckj;y1:"
  "x;;;;l2:y1:_;y20:%25residual-flinteger?;;",

  "P", "%residual-flinteger?",
  "&0{%1.0%jJw]1}",

  "S", "flnan?",
  "l4:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l2:y6:%25jnanp;l2:y4:%25ckj;y1:"
  "x;;;;l2:y1:_;y16:%25residual-flnan?;;",

  "P", "%residual-flnan?",
  "&0{%1.0%jJu]1}",

  "S", "flinfinite?",
  "l4:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l2:y6:%25jinfp;l2:y4:%25ckj;y1:"
  "x;;;;l2:y1:_;y21:%25residual-flinfinite?;;",

  "P", "%residual-flinfinite?",
  "&0{%1.0%jJh]1}",

  "S", "flfinite?",
  "l4:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l2:y6:%25jfinp;l2:y4:%25ckj;y1:"
  "x;;;;l2:y1:_;y19:%25residual-flfinite?;;",

  "P", "%residual-flfinite?",
  "&0{%1.0%jJf]1}",

  "S", "fleven?",
  "l4:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l2:y6:%25jevnp;l2:y4:%25ckj;y1:"
  "x;;;;l2:y1:_;y17:%25residual-fleven?;;",

  "P", "%residual-fleven?",
  "&0{%1.0%jJe]1}",

  "S", "flodd?",
  "l4:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l2:y6:%25joddp;l2:y4:%25ckj;y1:"
  "x;;;;l2:y1:_;y16:%25residual-flodd?;;",

  "P", "%residual-flodd?",
  "&0{%1.0%jJo]1}",

  "S", "fl+",
  "l4:y12:syntax-rules;n;l2:l3:y1:_;y1:x;y1:y;;l3:y5:%25jadd;l2:y4:%25ckj"
  ";y1:x;;l2:y4:%25ckj;y1:y;;;;l2:y1:_;y13:%25residual-fl+;;",

  "P", "%residual-fl+",
  "&0{%2.1%j,.1%jJ+]2}",

  "S", "fl-",
  "l4:y12:syntax-rules;n;l2:l3:y1:_;y1:x;y1:y;;l3:y5:%25jsub;l2:y4:%25ckj"
  ";y1:x;;l2:y4:%25ckj;y1:y;;;;l2:y1:_;y13:%25residual-fl-;;",

  "P", "%residual-fl-",
  "&0{%2.1%j,.1%jJ-]2}",

  "S", "fl*",
  "l4:y12:syntax-rules;n;l2:l3:y1:_;y1:x;y1:y;;l3:y5:%25jmul;l2:y4:%25ckj"
  ";y1:x;;l2:y4:%25ckj;y1:y;;;;l2:y1:_;y13:%25residual-fl*;;",

  "P", "%residual-fl*",
  "&0{%2.1%j,.1%jJ*]2}",

  "S", "fl/",
  "l4:y12:syntax-rules;n;l2:l3:y1:_;y1:x;y1:y;;l3:y5:%25jdiv;l2:y4:%25ckj"
  ";y1:x;;l2:y4:%25ckj;y1:y;;;;l2:y1:_;y13:%25residual-fl/;;",

  "P", "%residual-fl/",
  "&0{%2.1%j,.1%jJ/]2}",

  "S", "flneg",
  "l4:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l2:y5:%25jneg;l2:y4:%25ckj;y1:x"
  ";;;;l2:y1:_;y15:%25residual-flneg;;",

  "P", "%residual-flneg",
  "&0{%1.0%jJ-!]1}",

  "S", "flabs",
  "l4:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l2:y5:%25jabs;l2:y4:%25ckj;y1:x"
  ";;;;l2:y1:_;y15:%25residual-flabs;;",

  "P", "%residual-flabs",
  "&0{%1.0%jJa]1}",

  "S", "fl<?",
  "l4:y12:syntax-rules;n;l2:l3:y1:_;y1:x;y1:y;;l3:y4:%25jlt;l2:y4:%25ckj;"
  "y1:x;;l2:y4:%25ckj;y1:y;;;;l2:y1:_;y14:%25residual-fl<?;;",

  "P", "%residual-fl<?",
  "&0{%2.1%j,.1%jJ<]2}",

  "S", "fl<=?",
  "l4:y12:syntax-rules;n;l2:l3:y1:_;y1:x;y1:y;;l3:y4:%25jle;l2:y4:%25ckj;"
  "y1:x;;l2:y4:%25ckj;y1:y;;;;l2:y1:_;y15:%25residual-fl<=?;;",

  "P", "%residual-fl<=?",
  "&0{%2.1%j,.1%jJ>!]2}",

  "S", "fl>?",
  "l4:y12:syntax-rules;n;l2:l3:y1:_;y1:x;y1:y;;l3:y4:%25jgt;l2:y4:%25ckj;"
  "y1:x;;l2:y4:%25ckj;y1:y;;;;l2:y1:_;y14:%25residual-fl>?;;",

  "P", "%residual-fl>?",
  "&0{%2.1%j,.1%jJ>]2}",

  "S", "fl>=?",
  "l4:y12:syntax-rules;n;l2:l3:y1:_;y1:x;y1:y;;l3:y4:%25jge;l2:y4:%25ckj;"
  "y1:x;;l2:y4:%25ckj;y1:y;;;;l2:y1:_;y15:%25residual-fl>=?;;",

  "P", "%residual-fl>=?",
  "&0{%2.1%j,.1%jJ<!]2}",

  "S", "fl=?",
  "l4:y12:syntax-rules;n;l2:l3:y1:_;y1:x;y1:y;;l3:y4:%25jeq;l2:y4:%25ckj;"
  "y1:x;;l2:y4:%25ckj;y1:y;;;;l2:y1:_;y14:%25residual-fl=?;;",

  "P", "%residual-fl=?",
  "&0{%2.1%j,.1%jJ=]2}",

  "S", "flmin",
  "l4:y12:syntax-rules;n;l2:l3:y1:_;y1:x;y1:y;;l3:y5:%25jmin;l2:y4:%25ckj"
  ";y1:x;;l2:y4:%25ckj;y1:y;;;;l2:y1:_;y15:%25residual-flmin;;",

  "P", "%residual-flmin",
  "&0{%2.1%j,.1%jJn]2}",

  "S", "flmax",
  "l4:y12:syntax-rules;n;l2:l3:y1:_;y1:x;y1:y;;l3:y5:%25jmax;l2:y4:%25ckj"
  ";y1:x;;l2:y4:%25ckj;y1:y;;;;l2:y1:_;y15:%25residual-flmax;;",

  "P", "%residual-flmax",
  "&0{%2.1%j,.1%jJx]2}",

  "S", "flonum->fixnum",
  "l4:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l2:y5:%25jtoi;l2:y4:%25ckj;y1:x"
  ";;;;l2:y1:_;y24:%25residual-flonum->fixnum;;",

  "P", "%residual-flonum->fixnum",
  "&0{%1.0%jJi]1}",

  "S", "number?",
  "l4:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l2:y5:%25nump;y1:x;;;l2:y1:_;y1"
  "7:%25residual-number?;;",

  "P", "%residual-number?",
  "&0{%1.0N0]1}",

  "S", "integer?",
  "l4:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l2:y5:%25intp;y1:x;;;l2:y1:_;y1"
  "8:%25residual-integer?;;",

  "P", "%residual-integer?",
  "&0{%1.0N4]1}",

  "S", "complex?",
  "y7:number?",

  "S", "real?",
  "y7:number?",

  "S", "rational?",
  "y8:integer?",

  "S", "exact-integer?",
  "y7:fixnum?",

  "S", "exact?",
  "l4:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l2:y5:%25fixp;l2:y4:%25ckn;y1:x"
  ";;;;l2:y1:_;y16:%25residual-exact?;;",

  "P", "%residual-exact?",
  "&0{%1.0%nI0]1}",

  "S", "inexact?",
  "l4:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l2:y5:%25flop;l2:y4:%25ckn;y1:x"
  ";;;;l2:y1:_;y18:%25residual-inexact?;;",

  "P", "%residual-inexact?",
  "&0{%1.0%nJ0]1}",

  "S", "finite?",
  "l4:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l2:y5:%25finp;l2:y4:%25ckn;y1:x"
  ";;;;l2:y1:_;y17:%25residual-finite?;;",

  "P", "%residual-finite?",
  "&0{%1.0%nN6]1}",

  "S", "infinite?",
  "l4:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l2:y5:%25infp;l2:y4:%25ckn;y1:x"
  ";;;;l2:y1:_;y19:%25residual-infinite?;;",

  "P", "%residual-infinite?",
  "&0{%1.0%nN7]1}",

  "S", "nan?",
  "l4:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l2:y5:%25nanp;l2:y4:%25ckn;y1:x"
  ";;;;l2:y1:_;y14:%25residual-nan?;;",

  "P", "%residual-nan?",
  "&0{%1.0%nN5]1}",

  "S", "zero?",
  "l4:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l2:y6:%25zerop;l2:y4:%25ckn;y1:"
  "x;;;;l2:y1:_;y15:%25residual-zero?;;",

  "P", "%residual-zero?",
  "&0{%1.0%n=0]1}",

  "S", "positive?",
  "l4:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l2:y5:%25posp;l2:y4:%25ckn;y1:x"
  ";;;;l2:y1:_;y19:%25residual-positive?;;",

  "P", "%residual-positive?",
  "&0{%1.0%n>0]1}",

  "S", "negative?",
  "l4:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l2:y5:%25negp;l2:y4:%25ckn;y1:x"
  ";;;;l2:y1:_;y19:%25residual-negative?;;",

  "P", "%residual-negative?",
  "&0{%1.0%n<0]1}",

  "S", "even?",
  "l4:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l2:y5:%25evnp;l2:y4:%25ckn;y1:x"
  ";;;;l2:y1:_;y15:%25residual-even?;;",

  "P", "%residual-even?",
  "&0{%1.0%nN8]1}",

  "S", "odd?",
  "l4:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l2:y5:%25oddp;l2:y4:%25ckn;y1:x"
  ";;;;l2:y1:_;y14:%25residual-odd?;;",

  "P", "%residual-odd?",
  "&0{%1.0%nN9]1}",

  "S", "min",
  "l6:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;y1:x;;l2:l3:y1:_;y1:x;y1:y;;l3:"
  "y4:%25min;l2:y4:%25ckn;y1:x;;l2:y4:%25ckn;y1:y;;;;l2:l5:y1:_;y1:x;y1:y"
  ";y1:z;y3:...;;l4:y3:min;l3:y3:min;y1:x;y1:y;;y1:z;y3:...;;;l2:y1:_;y13"
  ":%25residual-min;;",

  "S", "max",
  "l6:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;y1:x;;l2:l3:y1:_;y1:x;y1:y;;l3:"
  "y4:%25max;l2:y4:%25ckn;y1:x;;l2:y4:%25ckn;y1:y;;;;l2:l5:y1:_;y1:x;y1:y"
  ";y1:z;y3:...;;l4:y3:max;l3:y3:max;y1:x;y1:y;;y1:z;y3:...;;;l2:y1:_;y13"
  ":%25residual-max;;",

  "S", "+",
  "l7:y12:syntax-rules;n;l2:l1:y1:_;;i0;;l2:l2:y1:_;y1:x;;l2:y4:%25ckn;y1"
  ":x;;;l2:l3:y1:_;y1:x;y1:y;;l3:y4:%25add;l2:y4:%25ckn;y1:x;;l2:y4:%25ck"
  "n;y1:y;;;;l2:l5:y1:_;y1:x;y1:y;y1:z;y3:...;;l4:y1:+;l3:y1:+;y1:x;y1:y;"
  ";y1:z;y3:...;;;l2:y1:_;y10:%25residual+;;",

  "S", "*",
  "l7:y12:syntax-rules;n;l2:l1:y1:_;;i1;;l2:l2:y1:_;y1:x;;l2:y4:%25ckn;y1"
  ":x;;;l2:l3:y1:_;y1:x;y1:y;;l3:y4:%25mul;l2:y4:%25ckn;y1:x;;l2:y4:%25ck"
  "n;y1:y;;;;l2:l5:y1:_;y1:x;y1:y;y1:z;y3:...;;l4:y1:*;l3:y1:*;y1:x;y1:y;"
  ";y1:z;y3:...;;;l2:y1:_;y10:%25residual*;;",

  "S", "-",
  "l6:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l2:y4:%25neg;l2:y4:%25ckn;y1:x;"
  ";;;l2:l3:y1:_;y1:x;y1:y;;l3:y4:%25sub;l2:y4:%25ckn;y1:x;;l2:y4:%25ckn;"
  "y1:y;;;;l2:l5:y1:_;y1:x;y1:y;y1:z;y3:...;;l4:y1:-;l3:y1:-;y1:x;y1:y;;y"
  "1:z;y3:...;;;l2:y1:_;y10:%25residual-;;",

  "S", "/",
  "l6:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l3:y4:%25div;i1;l2:y4:%25ckn;y1"
  ":x;;;;l2:l3:y1:_;y1:x;y1:y;;l3:y4:%25div;l2:y4:%25ckn;y1:x;;l2:y4:%25c"
  "kn;y1:y;;;;l2:l5:y1:_;y1:x;y1:y;y1:z;y3:...;;l4:y1:/;l3:y1:/;y1:x;y1:y"
  ";;y1:z;y3:...;;;l2:y1:_;y10:%25residual/;;",

  "S", "=",
  "l5:y12:syntax-rules;n;l2:l3:y1:_;y1:x;y1:y;;l3:y3:%25eq;l2:y4:%25ckn;y"
  "1:x;;l2:y4:%25ckn;y1:y;;;;l2:l5:y1:_;y1:x;y1:y;y1:z;y3:...;;l3:y3:let;"
  "l1:l2:y1:t;y1:y;;;l3:y3:and;l3:y1:=;y1:x;y1:t;;l4:y1:=;y1:t;y1:z;y3:.."
  ".;;;;;l2:y1:_;y10:%25residual=;;",

  "S", "<",
  "l5:y12:syntax-rules;n;l2:l3:y1:_;y1:x;y1:y;;l3:y3:%25lt;l2:y4:%25ckn;y"
  "1:x;;l2:y4:%25ckn;y1:y;;;;l2:l5:y1:_;y1:x;y1:y;y1:z;y3:...;;l3:y3:let;"
  "l1:l2:y1:t;y1:y;;;l3:y3:and;l3:y1:<;y1:x;y1:t;;l4:y1:<;y1:t;y1:z;y3:.."
  ".;;;;;l2:y1:_;y10:%25residual<;;",

  "S", ">",
  "l5:y12:syntax-rules;n;l2:l3:y1:_;y1:x;y1:y;;l3:y3:%25gt;l2:y4:%25ckn;y"
  "1:x;;l2:y4:%25ckn;y1:y;;;;l2:l5:y1:_;y1:x;y1:y;y1:z;y3:...;;l3:y3:let;"
  "l1:l2:y1:t;y1:y;;;l3:y3:and;l3:y1:>;y1:x;y1:t;;l4:y1:>;y1:t;y1:z;y3:.."
  ".;;;;;l2:y1:_;y10:%25residual>;;",

  "S", "<=",
  "l5:y12:syntax-rules;n;l2:l3:y1:_;y1:x;y1:y;;l3:y3:%25le;l2:y4:%25ckn;y"
  "1:x;;l2:y4:%25ckn;y1:y;;;;l2:l5:y1:_;y1:x;y1:y;y1:z;y3:...;;l3:y3:let;"
  "l1:l2:y1:t;y1:y;;;l3:y3:and;l3:y2:<=;y1:x;y1:t;;l4:y2:<=;y1:t;y1:z;y3:"
  "...;;;;;l2:y1:_;y11:%25residual<=;;",

  "S", ">=",
  "l5:y12:syntax-rules;n;l2:l3:y1:_;y1:x;y1:y;;l3:y3:%25ge;l2:y4:%25ckn;y"
  "1:x;;l2:y4:%25ckn;y1:y;;;;l2:l5:y1:_;y1:x;y1:y;y1:z;y3:...;;l3:y3:let;"
  "l1:l2:y1:t;y1:y;;;l3:y3:and;l3:y2:>=;y1:x;y1:t;;l4:y2:>=;y1:t;y1:z;y3:"
  "...;;;;;l2:y1:_;y11:%25residual>=;;",

  "S", "abs",
  "l4:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l2:y4:%25abs;l2:y4:%25ckn;y1:x;"
  ";;;l2:y1:_;y13:%25residual-abs;;",

  "P", "%residual-abs",
  "&0{%1.0%nG0]1}",

  "S", "quotient",
  "l4:y12:syntax-rules;n;l2:l3:y1:_;y1:x;y1:y;;l2:y4:%25quo;l2:y4:%25ckn;"
  "y1:x;;;;l2:y1:_;y18:%25residual-quotient;;",

  "P", "%residual-quotient",
  "&0{%2.0%nG5]2}",

  "S", "remainder",
  "l4:y12:syntax-rules;n;l2:l3:y1:_;y1:x;y1:y;;l2:y4:%25rem;l2:y4:%25ckn;"
  "y1:x;;;;l2:y1:_;y19:%25residual-remainder;;",

  "P", "%residual-remainder",
  "&0{%2.0%nG6]2}",

  "S", "truncate-quotient",
  "y8:quotient",

  "S", "truncate-remainder",
  "y9:remainder",

  "S", "modquo",
  "l4:y12:syntax-rules;n;l2:l3:y1:_;y1:x;y1:y;;l2:y4:%25mqu;l2:y4:%25ckn;"
  "y1:x;;;;l2:y1:_;y16:%25residual-modquo;;",

  "P", "%residual-modquo",
  "&0{%2.0%nG3]2}",

  "S", "modulo",
  "l4:y12:syntax-rules;n;l2:l3:y1:_;y1:x;y1:y;;l2:y4:%25mlo;l2:y4:%25ckn;"
  "y1:x;;;;l2:y1:_;y16:%25residual-modulo;;",

  "P", "%residual-modulo",
  "&0{%2.0%nG4]2}",

  "S", "floor-quotient",
  "y6:modquo",

  "S", "floor-remainder",
  "y6:modulo",

  "S", "boolean?",
  "l4:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l2:y6:%25boolp;y1:x;;;l2:y1:_;y"
  "18:%25residual-boolean?;;",

  "P", "%residual-boolean?",
  "&0{%1.0Y1]1}",

  "S", "not",
  "l4:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l2:y4:%25not;y1:x;;;l2:y1:_;y13"
  ":%25residual-not;;",

  "P", "%residual-not",
  "&0{%1.0~]1}",

  "S", "char?",
  "l4:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l2:y6:%25charp;y1:x;;;l2:y1:_;y"
  "15:%25residual-char?;;",

  "P", "%residual-char?",
  "&0{%1.0C0]1}",

  "S", "char-cmp",
  "l4:y12:syntax-rules;n;l2:l3:y1:_;y1:x;y1:y;;l3:y5:%25ccmp;l2:y4:%25ckc"
  ";y1:x;;l2:y4:%25ckc;y1:y;;;;l2:y1:_;y18:%25residual-char-cmp;;",

  "P", "%residual-char-cmp",
  "&0{%2.1%c,.1%cO0]2}",

  "S", "char=?",
  "l4:y12:syntax-rules;n;l2:l3:y1:_;y1:x;y1:y;;l3:y4:%25ceq;l2:y4:%25ckc;"
  "y1:x;;l2:y4:%25ckc;y1:y;;;;l2:y1:_;y16:%25residual-char=?;;",

  "P", "%residual-char=?",
  "&0{%2.1%c,.1%cC=]2}",

  "S", "char<?",
  "l4:y12:syntax-rules;n;l2:l3:y1:_;y1:x;y1:y;;l3:y4:%25clt;l2:y4:%25ckc;"
  "y1:x;;l2:y4:%25ckc;y1:y;;;;l2:y1:_;y16:%25residual-char<?;;",

  "P", "%residual-char<?",
  "&0{%2.1%c,.1%cC<]2}",

  "S", "char<=?",
  "l4:y12:syntax-rules;n;l2:l3:y1:_;y1:x;y1:y;;l3:y4:%25cle;l2:y4:%25ckc;"
  "y1:x;;l2:y4:%25ckc;y1:y;;;;l2:y1:_;y17:%25residual-char<=?;;",

  "P", "%residual-char<=?",
  "&0{%2.1%c,.1%cC>!]2}",

  "S", "char>?",
  "l4:y12:syntax-rules;n;l2:l3:y1:_;y1:x;y1:y;;l3:y4:%25cgt;l2:y4:%25ckc;"
  "y1:x;;l2:y4:%25ckc;y1:y;;;;l2:y1:_;y16:%25residual-char>?;;",

  "P", "%residual-char>?",
  "&0{%2.1%c,.1%cC>]2}",

  "S", "char>=?",
  "l4:y12:syntax-rules;n;l2:l3:y1:_;y1:x;y1:y;;l3:y4:%25cge;l2:y4:%25ckc;"
  "y1:x;;l2:y4:%25ckc;y1:y;;;;l2:y1:_;y17:%25residual-char>=?;;",

  "P", "%residual-char>=?",
  "&0{%2.1%c,.1%cC<!]2}",

  "S", "char-ci-cmp",
  "l4:y12:syntax-rules;n;l2:l3:y1:_;y1:x;y1:y;;l3:y6:%25cicmp;l2:y4:%25ck"
  "c;y1:x;;l2:y4:%25ckc;y1:y;;;;l2:y1:_;y18:%25residual-char-cmp;;",

  "P", "%residual-char-cmp",
  "&0{%2.1%c,.1%cO1]2}",

  "S", "char-ci=?",
  "l4:y12:syntax-rules;n;l2:l3:y1:_;y1:x;y1:y;;l3:y5:%25cieq;l2:y4:%25ckc"
  ";y1:x;;l2:y4:%25ckc;y1:y;;;;l2:y1:_;y19:%25residual-char-ci=?;;",

  "P", "%residual-char-ci=?",
  "&0{%2.1%c,.1%cCi=]2}",

  "S", "char-ci<?",
  "l4:y12:syntax-rules;n;l2:l3:y1:_;y1:x;y1:y;;l3:y5:%25cilt;l2:y4:%25ckc"
  ";y1:x;;l2:y4:%25ckc;y1:y;;;;l2:y1:_;y19:%25residual-char-ci<?;;",

  "P", "%residual-char-ci<?",
  "&0{%2.1%c,.1%cCi<]2}",

  "S", "char-ci<=?",
  "l4:y12:syntax-rules;n;l2:l3:y1:_;y1:x;y1:y;;l3:y5:%25cile;l2:y4:%25ckc"
  ";y1:x;;l2:y4:%25ckc;y1:y;;;;l2:y1:_;y20:%25residual-char-ci<=?;;",

  "P", "%residual-char-ci<=?",
  "&0{%2.1%c,.1%cCi>!]2}",

  "S", "char-ci>?",
  "l4:y12:syntax-rules;n;l2:l3:y1:_;y1:x;y1:y;;l3:y5:%25cigt;l2:y4:%25ckc"
  ";y1:x;;l2:y4:%25ckc;y1:y;;;;l2:y1:_;y19:%25residual-char-ci>?;;",

  "P", "%residual-char-ci>?",
  "&0{%2.1%c,.1%cCi>]2}",

  "S", "char-ci>=?",
  "l4:y12:syntax-rules;n;l2:l3:y1:_;y1:x;y1:y;;l3:y5:%25cige;l2:y4:%25ckc"
  ";y1:x;;l2:y4:%25ckc;y1:y;;;;l2:y1:_;y20:%25residual-char-ci>=?;;",

  "P", "%residual-char-ci>=?",
  "&0{%2.1%c,.1%cCi<!]2}",

  "S", "char-alphabetic?",
  "l4:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l2:y5:%25calp;l2:y4:%25ckc;y1:x"
  ";;;;l2:y1:_;y26:%25residual-char-alphabetic?;;",

  "P", "%residual-char-alphabetic?",
  "&0{%1.0%cC4]1}",

  "S", "char-numeric?",
  "l4:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l2:y5:%25cnup;l2:y4:%25ckc;y1:x"
  ";;;;l2:y1:_;y23:%25residual-char-numeric?;;",

  "P", "%residual-char-numeric?",
  "&0{%1.0%cC5]1}",

  "S", "char-whitespace?",
  "l4:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l2:y5:%25cwsp;l2:y4:%25ckc;y1:x"
  ";;;;l2:y1:_;y26:%25residual-char-whitespace?;;",

  "P", "%residual-char-whitespace?",
  "&0{%1.0%cC1]1}",

  "S", "char-upper-case?",
  "l4:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l2:y5:%25cucp;l2:y4:%25ckc;y1:x"
  ";;;;l2:y1:_;y26:%25residual-char-upper-case?;;",

  "P", "%residual-char-upper-case?",
  "&0{%1.0%cC3]1}",

  "S", "char-lower-case?",
  "l4:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l2:y5:%25clcp;l2:y4:%25ckc;y1:x"
  ";;;;l2:y1:_;y26:%25residual-char-lower-case?;;",

  "P", "%residual-char-lower-case?",
  "&0{%1.0%cC2]1}",

  "S", "char-upcase",
  "l4:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l2:y5:%25cupc;l2:y4:%25ckc;y1:x"
  ";;;;l2:y1:_;y21:%25residual-char-upcase;;",

  "P", "%residual-char-upcase",
  "&0{%1.0%cC6]1}",

  "S", "char-downcase",
  "l4:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l2:y5:%25cdnc;l2:y4:%25ckc;y1:x"
  ";;;;l2:y1:_;y23:%25residual-char-downcase;;",

  "P", "%residual-char-downcase",
  "&0{%1.0%cC7]1}",

  "S", "char->integer",
  "l4:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l2:y5:%25ctoi;l2:y4:%25ckc;y1:x"
  ";;;;l2:y1:_;y23:%25residual-char->integer;;",

  "P", "%residual-char->integer",
  "&0{%1.0%cX8]1}",

  "S", "integer->char",
  "l4:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l2:y5:%25itoc;l2:y4:%25cki;y1:x"
  ";;;;l2:y1:_;y23:%25residual-integer->char;;",

  "P", "%residual-integer->char",
  "&0{%1.0%iX9]1}",

  "S", "symbol?",
  "l4:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l2:y5:%25symp;y1:x;;;l2:y1:_;y1"
  "7:%25residual-symbol?;;",

  "P", "%residual-symbol?",
  "&0{%1.0Y0]1}",

  "S", "symbol->string",
  "l4:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l2:y5:%25ytos;l2:y4:%25cky;y1:x"
  ";;;;l2:y1:_;y24:%25residual-symbol->string;;",

  "P", "%residual-symbol->string",
  "&0{%1.0%yX4]1}",

  "S", "string->symbol",
  "l4:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l2:y5:%25stoy;l2:y4:%25cks;y1:x"
  ";;;;l2:y1:_;y24:%25residual-string->symbol;;",

  "P", "%residual-string->symbol",
  "&0{%1.0%sX5]1}",

  "S", "null?",
  "l4:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l2:y6:%25nullp;y1:x;;;l2:y1:_;y"
  "15:%25residual-null?;;",

  "P", "%residual-null?",
  "&0{%1.0u]1}",

  "S", "pair?",
  "l4:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l2:y6:%25pairp;y1:x;;;l2:y1:_;y"
  "15:%25residual-pair?;;",

  "P", "%residual-pair?",
  "&0{%1.0p]1}",

  "S", "car",
  "l4:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l2:y4:%25car;l2:y4:%25ckp;y1:x;"
  ";;;l2:y1:_;y13:%25residual-car;;",

  "P", "%residual-car",
  "&0{%1.0%pa]1}",

  "S", "set-car!",
  "l4:y12:syntax-rules;n;l2:l3:y1:_;y1:x;y1:v;;l3:y7:%25setcar;l2:y4:%25c"
  "kp;y1:x;;y1:v;;;l2:y1:_;y18:%25residual-set-car!;;",

  "P", "%residual-set-car!",
  "&0{%2.1,.1%pa!]2}",

  "S", "cdr",
  "l4:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l2:y4:%25cdr;l2:y4:%25ckp;y1:x;"
  ";;;l2:y1:_;y13:%25residual-cdr;;",

  "P", "%residual-cdr",
  "&0{%1.0%pd]1}",

  "S", "set-cdr!",
  "l4:y12:syntax-rules;n;l2:l3:y1:_;y1:x;y1:v;;l3:y7:%25setcdr;l2:y4:%25c"
  "kp;y1:x;;y1:v;;;l2:y1:_;y18:%25residual-set-cdr!;;",

  "P", "%residual-set-cdr!",
  "&0{%2.1,.1%pd!]2}",

  "S", "c?r",
  "l5:y12:syntax-rules;l2:y1:a;y1:d;;l2:l2:y3:c?r;y1:x;;y1:x;;l2:l5:y3:c?"
  "r;y1:a;y1:?;y3:...;y1:x;;l2:y3:car;l4:y3:c?r;y1:?;y3:...;y1:x;;;;l2:l5"
  ":y3:c?r;y1:d;y1:?;y3:...;y1:x;;l2:y3:cdr;l4:y3:c?r;y1:?;y3:...;y1:x;;;"
  ";",

  "S", "caar",
  "l4:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l4:y3:c?r;y1:a;y1:a;y1:x;;;l2:y"
  "1:_;y14:%25residual-caar;;",

  "P", "%residual-caar",
  "&0{%1.0%pa%pa]1}",

  "S", "cadr",
  "l4:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l4:y3:c?r;y1:a;y1:d;y1:x;;;l2:y"
  "1:_;y14:%25residual-cadr;;",

  "P", "%residual-cadr",
  "&0{%1.0%pd%pa]1}",

  "S", "cdar",
  "l4:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l4:y3:c?r;y1:d;y1:a;y1:x;;;l2:y"
  "1:_;y14:%25residual-cdar;;",

  "P", "%residual-cdar",
  "&0{%1.0%pa%pd]1}",

  "S", "cddr",
  "l4:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l4:y3:c?r;y1:d;y1:d;y1:x;;;l2:y"
  "1:_;y14:%25residual-cddr;;",

  "P", "%residual-cddr",
  "&0{%1.0%pd%pd]1}",

  "S", "caaar",
  "l4:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l5:y3:c?r;y1:a;y1:a;y1:a;y1:x;;"
  ";l2:y1:_;y15:%25residual-caaar;;",

  "P", "%residual-caaar",
  "&0{%1.0%pa%pa%pa]1}",

  "S", "caadr",
  "l4:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l5:y3:c?r;y1:a;y1:a;y1:d;y1:x;;"
  ";l2:y1:_;y15:%25residual-caadr;;",

  "P", "%residual-caadr",
  "&0{%1.0%pd%pa%pa]1}",

  "S", "cadar",
  "l4:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l5:y3:c?r;y1:a;y1:d;y1:a;y1:x;;"
  ";l2:y1:_;y15:%25residual-cadar;;",

  "P", "%residual-cadar",
  "&0{%1.0%pa%pd%pa]1}",

  "S", "caddr",
  "l4:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l5:y3:c?r;y1:a;y1:d;y1:d;y1:x;;"
  ";l2:y1:_;y15:%25residual-caddr;;",

  "P", "%residual-caddr",
  "&0{%1.0%pd%pd%pa]1}",

  "S", "cdaar",
  "l4:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l5:y3:c?r;y1:d;y1:a;y1:a;y1:x;;"
  ";l2:y1:_;y15:%25residual-cdaar;;",

  "P", "%residual-cdaar",
  "&0{%1.0%pa%pa%pd]1}",

  "S", "cdadr",
  "l4:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l5:y3:c?r;y1:d;y1:a;y1:d;y1:x;;"
  ";l2:y1:_;y15:%25residual-cdadr;;",

  "P", "%residual-cdadr",
  "&0{%1.0%pd%pa%pd]1}",

  "S", "cddar",
  "l4:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l5:y3:c?r;y1:d;y1:d;y1:a;y1:x;;"
  ";l2:y1:_;y15:%25residual-cddar;;",

  "P", "%residual-cddar",
  "&0{%1.0%pa%pd%pd]1}",

  "S", "cdddr",
  "l4:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l5:y3:c?r;y1:d;y1:d;y1:d;y1:x;;"
  ";l2:y1:_;y15:%25residual-cdddr;;",

  "P", "%residual-cdddr",
  "&0{%1.0%pd%pd%pd]1}",

  "S", "caaaar",
  "l4:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l6:y3:c?r;y1:a;y1:a;y1:a;y1:a;y"
  "1:x;;;l2:y1:_;y16:%25residual-caaaar;;",

  "P", "%residual-caaaar",
  "&0{%1.0%pa%pa%pa%pa]1}",

  "S", "caaadr",
  "l4:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l6:y3:c?r;y1:a;y1:a;y1:a;y1:d;y"
  "1:x;;;l2:y1:_;y16:%25residual-caaadr;;",

  "P", "%residual-caaadr",
  "&0{%1.0%pd%pa%pa%pa]1}",

  "S", "caadar",
  "l4:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l6:y3:c?r;y1:a;y1:a;y1:d;y1:a;y"
  "1:x;;;l2:y1:_;y16:%25residual-caadar;;",

  "P", "%residual-caadar",
  "&0{%1.0%pa%pd%pa%pa]1}",

  "S", "caaddr",
  "l4:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l6:y3:c?r;y1:a;y1:a;y1:d;y1:d;y"
  "1:x;;;l2:y1:_;y16:%25residual-caaddr;;",

  "P", "%residual-caaddr",
  "&0{%1.0%pd%pd%pa%pa]1}",

  "S", "cadaar",
  "l4:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l6:y3:c?r;y1:a;y1:d;y1:a;y1:a;y"
  "1:x;;;l2:y1:_;y16:%25residual-cadaar;;",

  "P", "%residual-cadaar",
  "&0{%1.0%pa%pa%pd%pa]1}",

  "S", "cadadr",
  "l4:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l6:y3:c?r;y1:a;y1:d;y1:a;y1:d;y"
  "1:x;;;l2:y1:_;y16:%25residual-cadadr;;",

  "P", "%residual-cadadr",
  "&0{%1.0%pd%pa%pd%pa]1}",

  "S", "caddar",
  "l4:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l6:y3:c?r;y1:a;y1:d;y1:d;y1:a;y"
  "1:x;;;l2:y1:_;y16:%25residual-caddar;;",

  "P", "%residual-caddar",
  "&0{%1.0%pa%pd%pd%pa]1}",

  "S", "cadddr",
  "l4:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l6:y3:c?r;y1:a;y1:d;y1:d;y1:d;y"
  "1:x;;;l2:y1:_;y16:%25residual-cadddr;;",

  "P", "%residual-cadddr",
  "&0{%1.0%pd%pd%pd%pa]1}",

  "S", "cdaaar",
  "l4:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l6:y3:c?r;y1:d;y1:a;y1:a;y1:a;y"
  "1:x;;;l2:y1:_;y16:%25residual-cdaaar;;",

  "P", "%residual-cdaaar",
  "&0{%1.0%pa%pa%pa%pd]1}",

  "S", "cdaadr",
  "l4:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l6:y3:c?r;y1:d;y1:a;y1:a;y1:d;y"
  "1:x;;;l2:y1:_;y16:%25residual-cdaadr;;",

  "P", "%residual-cdaadr",
  "&0{%1.0%pd%pa%pa%pd]1}",

  "S", "cdadar",
  "l4:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l6:y3:c?r;y1:d;y1:a;y1:d;y1:a;y"
  "1:x;;;l2:y1:_;y16:%25residual-cdadar;;",

  "P", "%residual-cdadar",
  "&0{%1.0%pa%pd%pa%pd]1}",

  "S", "cdaddr",
  "l4:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l6:y3:c?r;y1:d;y1:a;y1:d;y1:d;y"
  "1:x;;;l2:y1:_;y16:%25residual-cdaddr;;",

  "P", "%residual-cdaddr",
  "&0{%1.0%pd%pd%pa%pd]1}",

  "S", "cddaar",
  "l4:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l6:y3:c?r;y1:d;y1:d;y1:a;y1:a;y"
  "1:x;;;l2:y1:_;y16:%25residual-cddaar;;",

  "P", "%residual-cddaar",
  "&0{%1.0%pa%pa%pd%pd]1}",

  "S", "cddadr",
  "l4:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l6:y3:c?r;y1:d;y1:d;y1:a;y1:d;y"
  "1:x;;;l2:y1:_;y16:%25residual-cddadr;;",

  "P", "%residual-cddadr",
  "&0{%1.0%pd%pa%pd%pd]1}",

  "S", "cdddar",
  "l4:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l6:y3:c?r;y1:d;y1:d;y1:d;y1:a;y"
  "1:x;;;l2:y1:_;y16:%25residual-cdddar;;",

  "P", "%residual-cdddar",
  "&0{%1.0%pa%pd%pd%pd]1}",

  "S", "cddddr",
  "l4:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l6:y3:c?r;y1:d;y1:d;y1:d;y1:d;y"
  "1:x;;;l2:y1:_;y16:%25residual-cddddr;;",

  "P", "%residual-cddddr",
  "&0{%1.0%pd%pd%pd%pd]1}",

  "S", "cons",
  "l4:y12:syntax-rules;n;l2:l3:y1:_;y1:x;y1:y;;l3:y5:%25cons;y1:x;y1:y;;;"
  "l2:y1:_;y14:%25residual-cons;;",

  "P", "%residual-cons",
  "&0{%2.1,.1c]2}",

  "S", "list?",
  "l4:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l2:y6:%25listp;y1:x;;;l2:y1:_;y"
  "15:%25residual-list?;;",

  "P", "%residual-list?",
  "&0{%1.0L0]1}",

  "P", "%make-list",
  "&0{%2n,.1%k,,#0.4,.1,&2{%2'0,.1I>!?{.1]2}.1,:1c,'1,.2I-,:0^[22}.!0.0^_"
  "1[22}",

  "S", "make-list",
  "l5:y12:syntax-rules;n;l2:l2:y1:_;y1:n;;l3:y10:%25make-list;y1:n;f;;;l2"
  ":l3:y1:_;y1:n;y1:i;;l3:y10:%25make-list;y1:n;y1:i;;;l2:y1:_;y19:%25res"
  "idual-make-list;;",

  "S", "list",
  "l6:y12:syntax-rules;n;l2:l1:y1:_;;l2:y5:quote;n;;;l2:l2:y1:_;y1:x;;l3:"
  "y5:%25cons;y1:x;l2:y5:quote;n;;;;l2:l3:y1:_;y1:x;y3:...;;l3:y5:%25list"
  ";y1:x;y3:...;;;l2:y1:_;y14:%25residual-list;;",

  "S", "length",
  "l4:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l2:y5:%25llen;l2:y4:%25ckl;y1:x"
  ";;;;l2:y1:_;y16:%25residual-length;;",

  "P", "%residual-length",
  "&0{%1.0%lg]1}",

  "S", "list-ref",
  "l4:y12:syntax-rules;n;l2:l3:y1:_;y1:x;y1:i;;l3:y5:%25lget;l2:y4:%25ckl"
  ";y1:x;;l2:y4:%25cki;y1:i;;;;l2:y1:_;y18:%25residual-list-ref;;",

  "P", "%residual-list-ref",
  "&0{%2.1%i,.1%lL4]2}",

  "S", "list-set!",
  "l4:y12:syntax-rules;n;l2:l4:y1:_;y1:x;y1:i;y1:v;;l4:y5:%25lput;l2:y4:%"
  "25ckl;y1:x;;l2:y4:%25cki;y1:i;;y1:v;;;l2:y1:_;y19:%25residual-list-set"
  "!;;",

  "P", "%residual-list-set!",
  "&0{%3.2,.2%i,.2%lL5]3}",

  "S", "append",
  "l7:y12:syntax-rules;n;l2:l1:y1:_;;l2:y5:quote;n;;;l2:l2:y1:_;y1:x;;y1:"
  "x;;l2:l3:y1:_;y1:x;y1:y;;l3:y5:%25lcat;l2:y4:%25ckl;y1:x;;y1:y;;;l2:l5"
  ":y1:_;y1:x;y1:y;y1:z;y3:...;;l3:y5:%25lcat;l2:y4:%25ckl;y1:x;;l4:y6:ap"
  "pend;y1:y;y1:z;y3:...;;;;l2:y1:_;y16:%25residual-append;;",

  "S", "memq",
  "l4:y12:syntax-rules;n;l2:l3:y1:_;y1:v;y1:y;;l3:y5:%25memq;y1:v;l2:y4:%"
  "25ckl;y1:y;;;;l2:y1:_;y14:%25residual-memq;;",

  "P", "%residual-memq",
  "&0{%2.1%l,.1A0]2}",

  "S", "memv",
  "l4:y12:syntax-rules;n;l2:l3:y1:_;y1:v;y1:y;;l3:y5:%25memv;y1:v;l2:y4:%"
  "25ckl;y1:y;;;;l2:y1:_;y14:%25residual-memv;;",

  "P", "%residual-memv",
  "&0{%2.1%l,.1A1]2}",

  "P", "%member",
  "&0{%3.1p?{${.3a,.3,.6[02}?{.1]3}.2,.2d,.2,@(y7:%25member)[33}f]3}",

  "S", "member",
  "l5:y12:syntax-rules;n;l2:l3:y1:_;y1:v;y1:y;;l3:y5:%25meme;y1:v;l2:y4:%"
  "25ckl;y1:y;;;;l2:l4:y1:_;y1:v;y1:y;y2:eq;;l4:y7:%25member;y1:v;y1:y;y2"
  ":eq;;;l2:y1:_;y16:%25residual-member;;",

  "S", "assq",
  "l4:y12:syntax-rules;n;l2:l3:y1:_;y1:v;y1:y;;l3:y5:%25assq;y1:v;l2:y4:%"
  "25ckl;y1:y;;;;l2:y1:_;y14:%25residual-assq;;",

  "P", "%residual-assq",
  "&0{%2.1%l,.1A3]2}",

  "S", "assv",
  "l4:y12:syntax-rules;n;l2:l3:y1:_;y1:v;y1:y;;l3:y5:%25assv;y1:v;l2:y4:%"
  "25ckl;y1:y;;;;l2:y1:_;y14:%25residual-assv;;",

  "P", "%residual-assv",
  "&0{%2.1%l,.1A4]2}",

  "P", "%assoc",
  "&0{%3.1p?{${.3a%pa,.3,.6[02}?{.1a]3}.2,.2d,.2,@(y6:%25assoc)[33}f]3}",

  "S", "assoc",
  "l5:y12:syntax-rules;n;l2:l3:y1:_;y1:v;y2:al;;l3:y5:%25asse;y1:v;l2:y4:"
  "%25ckl;y2:al;;;;l2:l4:y1:_;y1:v;y2:al;y2:eq;;l4:y6:%25assoc;y1:v;y2:al"
  ";y2:eq;;;l2:y1:_;y15:%25residual-assoc;;",

  "S", "list-copy",
  "l4:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l3:y5:%25lcat;l2:y4:%25ckl;y1:x"
  ";;l2:y5:quote;n;;;;l2:y1:_;y19:%25residual-list-copy;;",

  "P", "%residual-list-copy",
  "&0{%1n,.1%lL6]1}",

  "S", "list-tail",
  "l4:y12:syntax-rules;n;l2:l3:y1:_;y1:x;y1:i;;l3:y6:%25ltail;l2:y4:%25ck"
  "l;y1:x;;l2:y4:%25cki;y1:i;;;;l2:y1:_;y19:%25residual-list-tail;;",

  "P", "%residual-list-tail",
  "&0{%2.1%i,.1%lA6]2}",

  "S", "last-pair",
  "l4:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l2:y6:%25lpair;l2:y4:%25ckp;y1:"
  "x;;;;l2:y1:_;y19:%25residual-last-pair;;",

  "P", "%residual-last-pair",
  "&0{%1.0%pA7]1}",

  "S", "reverse",
  "l4:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l2:y5:%25lrev;l2:y4:%25ckl;y1:x"
  ";;;;l2:y1:_;y17:%25residual-reverse;;",

  "P", "%residual-reverse",
  "&0{%1.0%lA8]1}",

  "S", "reverse!",
  "l4:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l2:y6:%25lrevi;l2:y4:%25ckl;y1:"
  "x;;;;l2:y1:_;y18:%25residual-reverse!;;",

  "P", "%residual-reverse!",
  "&0{%1.0%lA9]1}",

  "S", "list*",
  "l6:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;y1:x;;l2:l3:y1:_;y1:x;y1:y;;l3:"
  "y5:%25cons;y1:x;y1:y;;;l2:l5:y1:_;y1:x;y1:y;y1:z;y3:...;;l3:y5:%25cons"
  ";y1:x;l4:y5:list*;y1:y;y1:z;y3:...;;;;l2:y1:_;y15:%25residual-list*;;",

  "S", "cons*",
  "y5:list*",

  "S", "map",
  "l4:y12:syntax-rules;n;l2:l3:y1:_;y3:fun;y3:lst;;l3:y3:let;l1:l2:y1:f;y"
  "3:fun;;;l4:y3:let;y4:loop;l1:l2:y1:l;y3:lst;;;l4:y2:if;l2:y5:pair?;y1:"
  "l;;l3:y4:cons;l2:y1:f;l2:y4:%25car;y1:l;;;l2:y4:loop;l2:y4:%25cdr;y1:l"
  ";;;;l2:y5:quote;n;;;;;;l2:y1:_;y13:%25residual-map;;",

  "S", "for-each",
  "l4:y12:syntax-rules;n;l2:l3:y1:_;y3:fun;y3:lst;;l3:y3:let;l1:l2:y1:f;y"
  "3:fun;;;l4:y3:let;y4:loop;l1:l2:y1:l;y3:lst;;;l3:y2:if;l2:y5:pair?;y1:"
  "l;;l3:y5:begin;l2:y1:f;l2:y4:%25car;y1:l;;;l2:y4:loop;l2:y4:%25cdr;y1:"
  "l;;;;;;;;l2:y1:_;y18:%25residual-for-each;;",

  "S", "vector?",
  "l4:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l2:y5:%25vecp;y1:x;;;l2:y1:_;y1"
  "7:%25residual-vector?;;",

  "P", "%residual-vector?",
  "&0{%1.0V0]1}",

  "S", "vector",
  "y4:%25vec",

  "S", "make-vector",
  "l5:y12:syntax-rules;n;l2:l2:y1:_;y1:n;;l3:y4:%25vmk;l2:y4:%25ckk;y1:n;"
  ";f;;;l2:l3:y1:_;y1:n;y1:v;;l3:y4:%25vmk;l2:y4:%25ckk;y1:n;;y1:v;;;l2:y"
  "1:_;y21:%25residual-make-vector;;",

  "S", "vector-length",
  "l4:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l2:y5:%25vlen;l2:y4:%25ckv;y1:x"
  ";;;;l2:y1:_;y23:%25residual-vector-length;;",

  "P", "%residual-vector-length",
  "&0{%1.0%vV3]1}",

  "S", "vector-ref",
  "l4:y12:syntax-rules;n;l2:l3:y1:_;y1:x;y1:i;;l3:y5:%25vget;l2:y4:%25ckv"
  ";y1:x;;l2:y4:%25cki;y1:i;;;;l2:y1:_;y20:%25residual-vector-ref;;",

  "P", "%residual-vector-ref",
  "&0{%2.1%i,.1%vV4]2}",

  "S", "vector-set!",
  "l4:y12:syntax-rules;n;l2:l4:y1:_;y1:x;y1:i;y1:v;;l4:y5:%25vput;l2:y4:%"
  "25ckv;y1:x;;l2:y4:%25cki;y1:i;;y1:v;;;l2:y1:_;y21:%25residual-vector-s"
  "et!;;",

  "P", "%residual-vector-set!",
  "&0{%3.2,.2%i,.2%vV5]3}",

  "S", "vector-append",
  "l7:y12:syntax-rules;n;l2:l1:y1:_;;l2:y5:quote;v0:;;;l2:l2:y1:_;y1:x;;l"
  "2:y4:%25ckv;y1:x;;;l2:l3:y1:_;y1:x;y1:y;;l3:y5:%25vcat;l2:y4:%25ckv;y1"
  ":x;;l2:y4:%25ckv;y1:y;;;;l2:l5:y1:_;y1:x;y1:y;y1:z;y3:...;;l3:y13:vect"
  "or-append;y1:x;l4:y13:vector-append;y1:y;y1:z;y3:...;;;;l2:y1:_;y23:%2"
  "5residual-vector-append;;",

  "S", "vector->list",
  "l4:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l2:y5:%25vtol;l2:y4:%25ckv;y1:x"
  ";;;;l2:y1:_;y22:%25residual-vector->list;;",

  "P", "%residual-vector->list",
  "&0{%1.0%vX0]1}",

  "S", "list->vector",
  "l4:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l2:y5:%25ltov;l2:y4:%25ckl;y1:x"
  ";;;;l2:y1:_;y22:%25residual-list->vector;;",

  "P", "%residual-list->vector",
  "&0{%1.0%lX1]1}",

  "S", "string?",
  "l4:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l2:y5:%25strp;y1:x;;;l2:y1:_;y1"
  "7:%25residual-string?;;",

  "P", "%residual-string?",
  "&0{%1.0S0]1}",

  "S", "string",
  "l4:y12:syntax-rules;n;l2:l3:y1:_;y1:c;y3:...;;l3:y4:%25str;l2:y4:%25ck"
  "c;y1:c;;y3:...;;;l2:y1:_;y16:%25residual-string;;",

  "S", "make-string",
  "l5:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l3:y4:%25smk;l2:y4:%25ckk;y1:x;"
  ";c ;;;l2:l3:y1:_;y1:x;y1:y;;l3:y4:%25smk;l2:y4:%25ckk;y1:x;;l2:y4:%25c"
  "kc;y1:y;;;;l2:y1:_;y21:%25residual-make-string;;",

  "S", "string-length",
  "l4:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l2:y5:%25slen;l2:y4:%25cks;y1:x"
  ";;;;l2:y1:_;y23:%25residual-string-length;;",

  "P", "%residual-string-length",
  "&0{%1.0%sS3]1}",

  "S", "string-ref",
  "l4:y12:syntax-rules;n;l2:l3:y1:_;y1:x;y1:i;;l3:y5:%25sget;l2:y4:%25cks"
  ";y1:x;;l2:y4:%25cki;y1:i;;;;l2:y1:_;y20:%25residual-string-ref;;",

  "P", "%residual-string-ref",
  "&0{%2.1%i,.1%sS4]2}",

  "S", "string-set!",
  "l4:y12:syntax-rules;n;l2:l4:y1:_;y1:x;y1:i;y1:v;;l4:y5:%25sput;l2:y4:%"
  "25cks;y1:x;;l2:y4:%25cki;y1:i;;l2:y4:%25ckc;y1:v;;;;l2:y1:_;y21:%25res"
  "idual-string-set!;;",

  "P", "%residual-string-set!",
  "&0{%3.2%c,.2%i,.2%sS5]3}",

  "S", "string-append",
  "l7:y12:syntax-rules;n;l2:l1:y1:_;;s0:;;l2:l2:y1:_;y1:x;;l2:y4:%25cks;y"
  "1:x;;;l2:l3:y1:_;y1:x;y1:y;;l3:y5:%25scat;l2:y4:%25cks;y1:x;;l2:y4:%25"
  "cks;y1:y;;;;l2:l5:y1:_;y1:x;y1:y;y1:z;y3:...;;l3:y13:string-append;y1:"
  "x;l4:y13:string-append;y1:y;y1:z;y3:...;;;;l2:y1:_;y23:%25residual-str"
  "ing-append;;",

  "S", "substring",
  "l4:y12:syntax-rules;n;l2:l4:y1:_;y1:x;y1:s;y1:e;;l4:y5:%25ssub;l2:y4:%"
  "25cks;y1:x;;l2:y4:%25cki;y1:s;;l2:y4:%25cki;y1:e;;;;l2:y1:_;y19:%25res"
  "idual-substring;;",

  "P", "%residual-substring",
  "&0{%3.2%i,.2%i,.2%sS7]3}",

  "S", "string-cmp",
  "l4:y12:syntax-rules;n;l2:l3:y1:_;y1:x;y1:y;;l3:y5:%25scmp;l2:y4:%25cks"
  ";y1:x;;l2:y4:%25cks;y1:y;;;;l2:y1:_;y20:%25residual-string-cmp;;",

  "P", "%residual-string-cmp",
  "&0{%2.1%s,.1%sO2]2}",

  "S", "string=?",
  "l4:y12:syntax-rules;n;l2:l3:y1:_;y1:x;y1:y;;l3:y4:%25seq;l2:y4:%25cks;"
  "y1:x;;l2:y4:%25cks;y1:y;;;;l2:y1:_;y18:%25residual-string<?;;",

  "P", "%residual-string<?",
  "&0{%2.1%s,.1%sS=]2}",

  "S", "string<?",
  "l4:y12:syntax-rules;n;l2:l3:y1:_;y1:x;y1:y;;l3:y4:%25slt;l2:y4:%25cks;"
  "y1:x;;l2:y4:%25cks;y1:y;;;;l2:y1:_;y18:%25residual-string<?;;",

  "P", "%residual-string<?",
  "&0{%2.1%s,.1%sS<]2}",

  "S", "string<=?",
  "l4:y12:syntax-rules;n;l2:l3:y1:_;y1:x;y1:y;;l3:y4:%25sle;l2:y4:%25cks;"
  "y1:x;;l2:y4:%25cks;y1:y;;;;l2:y1:_;y19:%25residual-string<=?;;",

  "P", "%residual-string<=?",
  "&0{%2.1%s,.1%sS>!]2}",

  "S", "string>?",
  "l4:y12:syntax-rules;n;l2:l3:y1:_;y1:x;y1:y;;l3:y4:%25sgt;l2:y4:%25cks;"
  "y1:x;;l2:y4:%25cks;y1:y;;;;l2:y1:_;y18:%25residual-string>?;;",

  "P", "%residual-string>?",
  "&0{%2.1%s,.1%sS>]2}",

  "S", "string>=?",
  "l4:y12:syntax-rules;n;l2:l3:y1:_;y1:x;y1:y;;l3:y4:%25sge;l2:y4:%25cks;"
  "y1:x;;l2:y4:%25cks;y1:y;;;;l2:y1:_;y19:%25residual-string>=?;;",

  "P", "%residual-string>=?",
  "&0{%2.1%s,.1%sS<!]2}",

  "S", "string-ci-cmp",
  "l4:y12:syntax-rules;n;l2:l3:y1:_;y1:x;y1:y;;l3:y6:%25sicmp;l2:y4:%25ck"
  "s;y1:x;;l2:y4:%25cks;y1:y;;;;l2:y1:_;y20:%25residual-string-cmp;;",

  "P", "%residual-string-cmp",
  "&0{%2.1%s,.1%sO3]2}",

  "S", "string-ci=?",
  "l4:y12:syntax-rules;n;l2:l3:y1:_;y1:x;y1:y;;l3:y5:%25sieq;l2:y4:%25cks"
  ";y1:x;;l2:y4:%25cks;y1:y;;;;l2:y1:_;y18:%25residual-string<?;;",

  "P", "%residual-string<?",
  "&0{%2.1%s,.1%sSi=]2}",

  "S", "string-ci<?",
  "l4:y12:syntax-rules;n;l2:l3:y1:_;y1:x;y1:y;;l3:y5:%25silt;l2:y4:%25cks"
  ";y1:x;;l2:y4:%25cks;y1:y;;;;l2:y1:_;y18:%25residual-string<?;;",

  "P", "%residual-string<?",
  "&0{%2.1%s,.1%sSi<]2}",

  "S", "string-ci<=?",
  "l4:y12:syntax-rules;n;l2:l3:y1:_;y1:x;y1:y;;l3:y5:%25sile;l2:y4:%25cks"
  ";y1:x;;l2:y4:%25cks;y1:y;;;;l2:y1:_;y19:%25residual-string<=?;;",

  "P", "%residual-string<=?",
  "&0{%2.1%s,.1%sSi>!]2}",

  "S", "string-ci>?",
  "l4:y12:syntax-rules;n;l2:l3:y1:_;y1:x;y1:y;;l3:y5:%25sigt;l2:y4:%25cks"
  ";y1:x;;l2:y4:%25cks;y1:y;;;;l2:y1:_;y18:%25residual-string>?;;",

  "P", "%residual-string>?",
  "&0{%2.1%s,.1%sSi>]2}",

  "S", "string-ci>=?",
  "l4:y12:syntax-rules;n;l2:l3:y1:_;y1:x;y1:y;;l3:y5:%25sige;l2:y4:%25cks"
  ";y1:x;;l2:y4:%25cks;y1:y;;;;l2:y1:_;y19:%25residual-string>=?;;",

  "P", "%residual-string>=?",
  "&0{%2.1%s,.1%sSi<!]2}",

  "S", "string->list",
  "l4:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l2:y5:%25stol;l2:y4:%25cks;y1:x"
  ";;;;l2:y1:_;y22:%25residual-string->list;;",

  "P", "%residual-string->list",
  "&0{%1.0%sX2]1}",

  "S", "list->string",
  "l4:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l2:y5:%25ltos;l2:y4:%25ckl;y1:x"
  ";;;;l2:y1:_;y22:%25residual-list->string;;",

  "P", "%residual-list->string",
  "&0{%1.0%lX3]1}",

  "S", "fixnum->string",
  "l4:y12:syntax-rules;n;l2:l3:y1:_;y1:x;y1:r;;l3:y5:%25itos;l2:y4:%25cki"
  ";y1:x;;l2:y4:%25cki;y1:r;;;;l2:y1:_;y24:%25residual-fixnum->string;;",

  "P", "%residual-fixnum->string",
  "&0{%2.1%i,.1%iX6]2}",

  "S", "string->fixnum",
  "l4:y12:syntax-rules;n;l2:l3:y1:_;y1:x;y1:r;;l3:y5:%25stoi;l2:y4:%25cks"
  ";y1:x;;l2:y4:%25cki;y1:r;;;;l2:y1:_;y24:%25residual-string->fixnum;;",

  "P", "%residual-string->fixnum",
  "&0{%2.1%i,.1%sX7]2}",

  "S", "flonum->string",
  "l4:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l2:y5:%25jtos;l2:y4:%25ckj;y1:x"
  ";;;;l2:y1:_;y24:%25residual-flonum->string;;",

  "P", "%residual-flonum->string",
  "&0{%1.0%jE6]1}",

  "S", "string->flonum",
  "l4:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l2:y5:%25stoj;l2:y4:%25cks;y1:x"
  ";;;;l2:y1:_;y24:%25residual-string->flonum;;",

  "P", "%residual-string->flonum",
  "&0{%1.0%sE7]1}",

  "S", "number->string",
  "l5:y12:syntax-rules;n;l2:l3:y1:_;y1:x;y1:r;;l3:y5:%25ntos;l2:y4:%25cki"
  ";y1:x;;l2:y4:%25cki;y1:r;;;;l2:l2:y1:_;y1:x;;l3:y5:%25ntos;l2:y4:%25ck"
  "i;y1:x;;i10;;;l2:y1:_;y24:%25residual-number->string;;",

  "S", "string->number",
  "l5:y12:syntax-rules;n;l2:l3:y1:_;y1:x;y1:r;;l3:y5:%25ston;l2:y4:%25cks"
  ";y1:x;;l2:y4:%25cki;y1:r;;;;l2:l2:y1:_;y1:x;;l3:y5:%25ston;l2:y4:%25ck"
  "s;y1:x;;i10;;;l2:y1:_;y24:%25residual-string->number;;",

  "S", "procedure?",
  "l4:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l2:y5:%25funp;y1:x;;;l2:y1:_;y2"
  "0:%25residual-procedure?;;",

  "P", "%residual-procedure?",
  "&0{%1.0K0]1}",

  "S", "apply",
  "l5:y12:syntax-rules;n;l2:l3:y1:_;y1:p;y1:l;;l3:y5:%25appl;y1:p;l2:y4:%"
  "25ckl;y1:l;;;;l2:l6:y1:_;y1:p;y1:a;y1:b;y3:...;y1:l;;l3:y5:%25appl;y1:"
  "p;l5:y5:list*;y1:a;y1:b;y3:...;y1:l;;;;l2:y1:_;y15:%25residual-apply;;",

  "S", "call/cc",
  "y4:%25ccc",

  "S", "call-with-current-continuation",
  "y7:call/cc",

  "S", "input-port?",
  "l4:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l2:y4:%25ipp;y1:x;;;l2:y1:_;y21"
  ":%25residual-input-port?;;",

  "P", "%residual-input-port?",
  "&0{%1.0P00]1}",

  "S", "output-port?",
  "l4:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l2:y4:%25opp;y1:x;;;l2:y1:_;y22"
  ":%25residual-output-port?;;",

  "P", "%residual-output-port?",
  "&0{%1.0P01]1}",

  "S", "eof-object?",
  "l4:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l2:y5:%25eofp;y1:x;;;l2:y1:_;y2"
  "1:%25residual-eof-object?;;",

  "P", "%residual-eof-object?",
  "&0{%1.0Y9]1}",

  "S", "current-input-port",
  "l4:y12:syntax-rules;n;l2:l1:y1:_;;l1:y4:%25sip;;;l2:y1:_;y28:%25residu"
  "al-current-input-port;;",

  "P", "%residual-current-input-port",
  "&0{%0P10]0}",

  "S", "current-ouput-port",
  "l4:y12:syntax-rules;n;l2:l1:y1:_;;l1:y4:%25sop;;;l2:y1:_;y28:%25residu"
  "al-current-ouput-port;;",

  "P", "%residual-current-ouput-port",
  "&0{%0P11]0}",

  "S", "current-error-port",
  "l4:y12:syntax-rules;n;l2:l1:y1:_;;l1:y4:%25sep;;;l2:y1:_;y28:%25residu"
  "al-current-error-port;;",

  "P", "%residual-current-error-port",
  "&0{%0P12]0}",

  "S", "open-output-string",
  "l4:y12:syntax-rules;n;l2:l1:y1:_;;l1:y4:%25oos;;;l2:y1:_;y28:%25residu"
  "al-open-output-string;;",

  "P", "%residual-open-output-string",
  "&0{%0P51]0}",

  "S", "open-input-file",
  "l4:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l2:y5:%25otip;l2:y4:%25cks;y1:x"
  ";;;;l2:y1:_;y25:%25residual-open-input-file;;",

  "P", "%residual-open-input-file",
  "&0{%1.0%sP40]1}",

  "S", "open-output-file",
  "l4:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l2:y5:%25otop;l2:y4:%25cks;y1:x"
  ";;;;l2:y1:_;y26:%25residual-open-output-file;;",

  "P", "%residual-open-output-file",
  "&0{%1.0%sP41]1}",

  "S", "open-input-string",
  "l4:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l2:y4:%25ois;l2:y4:%25cks;y1:x;"
  ";;;l2:y1:_;y27:%25residual-open-input-string;;",

  "P", "%residual-open-input-string",
  "&0{%1.0%sP50]1}",

  "S", "close-input-port",
  "l4:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l2:y4:%25cip;l2:y4:%25ckr;y1:x;"
  ";;;l2:y1:_;y26:%25residual-close-input-port;;",

  "P", "%residual-close-input-port",
  "&0{%1.0%rP60]1}",

  "S", "close-output-port",
  "l4:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l2:y4:%25cop;l2:y4:%25ckw;y1:x;"
  ";;;l2:y1:_;y27:%25residual-close-output-port;;",

  "P", "%residual-close-output-port",
  "&0{%1.0%wP61]1}",

  "S", "get-output-string",
  "l4:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l2:y4:%25gos;l2:y4:%25ckw;y1:x;"
  ";;;l2:y1:_;y27:%25residual-get-output-string;;",

  "P", "%residual-get-output-string",
  "&0{%1.0%wP9]1}",

  "S", "write-char",
  "l4:y12:syntax-rules;n;l2:l3:y1:_;y1:x;y1:p;;l3:y4:%25wrc;l2:y4:%25ckc;"
  "y1:x;;l2:y4:%25ckw;y1:p;;;;l2:y1:_;y20:%25residual-write-char;;",

  "P", "%residual-write-char",
  "&0{%2.1%w,.1%cW0]2}",

  "S", "write-string",
  "l4:y12:syntax-rules;n;l2:l3:y1:_;y1:x;y1:p;;l3:y4:%25wrs;l2:y4:%25cks;"
  "y1:x;;l2:y4:%25ckw;y1:p;;;;l2:y1:_;y22:%25residual-write-string;;",

  "P", "%residual-write-string",
  "&0{%2.1%w,.1%sW1]2}",

  "S", "display",
  "l4:y12:syntax-rules;n;l2:l3:y1:_;y1:x;y1:p;;l3:y5:%25wrcd;y1:x;l2:y4:%"
  "25ckw;y1:p;;;;l2:y1:_;y17:%25residual-display;;",

  "P", "%residual-display",
  "&0{%2.1%w,.1W4]2}",

  "S", "write",
  "l4:y12:syntax-rules;n;l2:l3:y1:_;y1:x;y1:p;;l3:y5:%25wrcw;y1:x;l2:y4:%"
  "25ckw;y1:p;;;;l2:y1:_;y15:%25residual-write;;",

  "P", "%residual-write",
  "&0{%2.1%w,.1W5]2}",

  "S", "newline",
  "l4:y12:syntax-rules;n;l2:l2:y1:_;y1:p;;l2:y5:%25wrnl;l2:y4:%25ckw;y1:p"
  ";;;;l2:y1:_;y17:%25residual-newline;;",

  "P", "%residual-newline",
  "&0{%1.0%wW6]1}",

  "S", "write-shared",
  "l4:y12:syntax-rules;n;l2:l3:y1:_;y1:x;y1:p;;l3:y5:%25wrhw;y1:x;l2:y4:%"
  "25ckw;y1:p;;;;l2:y1:_;y22:%25residual-write-shared;;",

  "P", "%residual-write-shared",
  "&0{%2.1%w,.1W7]2}",

  "S", "write-simple",
  "l4:y12:syntax-rules;n;l2:l3:y1:_;y1:x;y1:p;;l3:y5:%25wriw;y1:x;l2:y4:%"
  "25ckw;y1:p;;;;l2:y1:_;y22:%25residual-write-simple;;",

  "P", "%residual-write-simple",
  "&0{%2.1%w,.1W8]2}",

  "P", "%residual-list",
  "&0{%!0.0]1}",

  "P", "%residual-make-list",
  "&0{%!1.0u?{f,.2,@(y10:%25make-list)[22}.0%pa,.2,@(y10:%25make-list)[22"
  "}",

  "P", "%residual-make-vector",
  "&0{%!1.0u?{f,.2%kV2]2}.0%pa,.2%kV2]2}",

  "P", "%residual-make-string",
  "&0{%!1.0u?{'(c ),.2%kS2]2}.0%pa%c,.2%kS2]2}",

  "P", "%residual=",
  "&0{%!0.0u,.0?{.0]2}.1%pd,.2%pa,,#0.0,&1{%2.1u,.0?{.0]3}.2%pa,.0%n,.3%n"
  "=?{.3%pd,.1,:0^[42}f]4}.!0.0^_1[22}",

  "P", "%residual<",
  "&0{%!0.0u,.0?{.0]2}.1%pd,.2%pa,,#0.0,&1{%2.1u,.0?{.0]3}.2%pa,.0%n,.3%n"
  "<?{.3%pd,.1,:0^[42}f]4}.!0.0^_1[22}",

  "P", "%residual>",
  "&0{%!0.0u,.0?{.0]2}.1%pd,.2%pa,,#0.0,&1{%2.1u,.0?{.0]3}.2%pa,.0%n,.3%n"
  ">?{.3%pd,.1,:0^[42}f]4}.!0.0^_1[22}",

  "P", "%residual<=",
  "&0{%!0.0u,.0?{.0]2}.1%pd,.2%pa,,#0.0,&1{%2.1u,.0?{.0]3}.2%pa,.0%n,.3%n"
  ">!?{.3%pd,.1,:0^[42}f]4}.!0.0^_1[22}",

  "P", "%residual>=",
  "&0{%!0.0u,.0?{.0]2}.1%pd,.2%pa,,#0.0,&1{%2.1u,.0?{.0]3}.2%pa,.0%n,.3%n"
  "<!?{.3%pd,.1,:0^[42}f]4}.!0.0^_1[22}",

  "S", "minmax-reducer",
  "l3:y12:syntax-rules;n;l2:l2:y1:_;y1:f;;l3:y6:lambda;py1:x;y4:args;;l4:"
  "y3:let;y4:loop;l2:l2:y1:x;y1:x;;l2:y4:args;y4:args;;;l4:y2:if;l2:y5:nu"
  "ll?;y4:args;;y1:x;l3:y4:loop;l3:y1:f;y1:x;l2:y3:car;y4:args;;;l2:y3:cd"
  "r;y4:args;;;;;;;",

  "P", "%residual-min",
  "&0{%!1.0,.2,,#0.0,&1{%2.1u?{.0]2}.1%pd,.2%pa%n,.2%nM2,:0^[22}.!0.0^_1["
  "22}",

  "P", "%residual-max",
  "&0{%!1.0,.2,,#0.0,&1{%2.1u?{.0]2}.1%pd,.2%pa%n,.2%nM3,:0^[22}.!0.0^_1["
  "22}",

  "P", "%residual+",
  "&0{%!0.0u?{'0]1}.0%pd,.1%pa,,#0.0,&1{%2.1u?{.0]2}.1%pd,.2%pa%n,.2%n+,:"
  "0^[22}.!0.0^_1[12}",

  "P", "%residual*",
  "&0{%!0.0u?{'1]1}.0%pd,.1%pa,,#0.0,&1{%2.1u?{.0]2}.1%pd,.2%pa%n,.2%n*,:"
  "0^[22}.!0.0^_1[12}",

  "P", "%residual-",
  "&0{%!1.0u?{.1%n-!]2}.0,.2,,#0.0,&1{%2.1u?{.0]2}.1%pd,.2%pa%n,.2%n-,:0^"
  "[22}.!0.0^_1[22}",

  "P", "%residual/",
  "&0{%!1.0u?{.1%n,'1/]2}.0,.2,,#0.0,&1{%2.1u?{.0]2}.1%pd,.2%pa%n,.2%n/,:"
  "0^[22}.!0.0^_1[22}",

  "P", "%residual-member",
  "&0{%!2.0u?{.2%l,.2A2]3}.0%pa,.3,.3,@(y7:%25member)[33}",

  "P", "%residual-assoc",
  "&0{%!2.0u?{.2%l,.2A5]3}.0%pa,.3,.3,@(y6:%25assoc)[33}",

  "P", "%residual-list*",
  "&0{%!1.0,.2,,#0.0,&1{%2.1u?{.0]2}${.3%pd,.4%pa,:0^[02},.1c]2}.!0.0^_1["
  "22}",

  "P", "%residual-apply",
  "&0{%!2${.2,.5,,#0.0,&1{%2.1u?{.0]2}${.3%pd,.4%pa,:0^[02},.1c]2}.!0.0^_"
  "1[02}%l,.2K3]3}",

  "P", "%residual-map",
  "&0{%!2.0u?{n,.3,,#0.0,.5,&2{%2.0p?{.1,${.3%pa,:0[01}c,.1%pd,:1^[22}.1%"
  "lA9]2}.!0.0^_1[32}n,.1,.4c,,#0.0,.5,&2{%2${.2,,#0.0,&1{%1.0u,.0?{.0]2}"
  ".1%pap?{.1%pd,:0^[21}f]2}.!0.0^_1[01}?{.1,@(y13:%25residual-car),${.4,"
  ",#0.4,.1,&2{%1.0p?{${.2d,:0^[01},${.3a,:1[01}c]1}n]1}.!0.0^_1[01}_1%l,"
  ":0K3c,@(y13:%25residual-cdr),${.4,,#0.4,.1,&2{%1.0p?{${.2d,:0^[01},${."
  "3a,:1[01}c]1}n]1}.!0.0^_1[01}_1,:1^[22}.1%lA9]2}.!0.0^_1[32}",

  "P", "%residual-for-each",
  "&0{%!2.0u?{.2,,#0.3,.1,&2{%1.0p?{${.2%pa,:1[01}.0%pd,:0^[11}]1}.!0.0^_"
  "1[31}.0,.3c,,#0.3,.1,&2{%1${.2,,#0.0,&1{%1.0u,.0?{.0]2}.1%pap?{.1%pd,:"
  "0^[21}f]2}.!0.0^_1[01}?{@(y13:%25residual-car),${.3,,#0.4,.1,&2{%1.0p?"
  "{${.2d,:0^[01},${.3a,:1[01}c]1}n]1}.!0.0^_1[01}_1%l,:1K3@(y13:%25resid"
  "ual-cdr),${.3,,#0.4,.1,&2{%1.0p?{${.2d,:0^[01},${.3a,:1[01}c]1}n]1}.!0"
  ".0^_1[01}_1,:0^[11}]1}.!0.0^_1[31}",

  "P", "string-map",
  "&0{%!2.0u?{.2%sS3,'(c ),.1%kS2,'0,,#0.0,.3,.7,.9,.7,&5{%1:0%i,.1%iI<!?"
  "{:3]1}${.2%i,:1%sS4,:2[01}%c,.1%i,:3%sS5.0'1%i,.1%iI+,:4^[11}.!0.0^_1["
  "51}@(y22:%25residual-string->list),${.3,.6c,,#0.4,.1,&2{%1.0p?{${.2d,:"
  "0^[01},${.3a,:1[01}c]1}n]1}.!0.0^_1[01}_1,.2c,@(y13:%25residual-map)K3"
  "%lX3]3}",

  "P", "vector-map",
  "&0{%!2.0u?{.2%vV3,f,.1%kV2,'0,,#0.0,.3,.8,.8,.7,&5{%1:0%i,.1%iI<!?{:3]"
  "1}${.2%i,:2%vV4,:1[01},.1%i,:3%vV5.0'1%i,.1%iI+,:4^[11}.!0.0^_1[51}@(y"
  "22:%25residual-vector->list),${.3,.6c,,#0.4,.1,&2{%1.0p?{${.2d,:0^[01}"
  ",${.3a,:1[01}c]1}n]1}.!0.0^_1[01}_1,.2c,@(y13:%25residual-map)K3%lX1]3"
  "}",

  "P", "string-for-each",
  "&0{%!2.0u?{.2%sS3,'0,,#0.2,.6,.6,.3,&4{%1:3%i,.1%iI<!,.0?{.0]2}${.3%i,"
  ":2%sS4,:1[01}.1'1%i,.2%iI+,:0^[21}.!0.0^_1[41}@(y22:%25residual-string"
  "->list),${.3,.6c,,#0.4,.1,&2{%1.0p?{${.2d,:0^[01},${.3a,:1[01}c]1}n]1}"
  ".!0.0^_1[01}_1,.2c,@(y18:%25residual-for-each)K3]3}",

  "P", "vector-for-each",
  "&0{%!2.0u?{.2%vV3,'0,,#0.2,.6,.6,.3,&4{%1:3%i,.1%iI<!,.0?{.0]2}${.3%i,"
  ":2%vV4,:1[01}.1'1%i,.2%iI+,:0^[21}.!0.0^_1[41}@(y22:%25residual-vector"
  "->list),${.3,.6c,,#0.4,.1,&2{%1.0p?{${.2d,:0^[01},${.3a,:1[01}c]1}n]1}"
  ".!0.0^_1[01}_1,.2c,@(y18:%25residual-for-each)K3]3}",

  "P", "%residual-append",
  "&0{%!0.0,,#0.0,&1{%1.0u?{n]1}.0%pdu?{.0%pa]1}${.2%pd,:0^[01},.1%pa%lL6"
  "]1}.!0.0^_1[11}",

  "P", "%residual-string-append",
  "&0{%!0.0,,#0.0,&1{%1.0u?{'(s0:)]1}.0%pdu?{.0%pa]1}${.2%pd,:0^[01}%s,.1"
  "%pa%sS6]1}.!0.0^_1[11}",

  "P", "%residual-vector-append",
  "&0{%!0.0,,#0.0,&1{%1.0u?{'(v0:)]1}.0%pdu?{.0%pa]1}${.2%pd,:0^[01}%v,.1"
  "%pa%vV6]1}.!0.0^_1[11}",

  "P", "%residual-number->string",
  "&0{%!1.0u?{'(i10),.2%iE8]2}.0%pa%i,.2%iE8]2}",

  "P", "%residual-string->number",
  "&0{%!1.0u?{'(i10),.2%sE9]2}.0%pa%i,.2%sE9]2}",

  0, 0, 0
};
