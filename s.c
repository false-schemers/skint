/* s.c -- generated via skint -c s.scm */

char *s_code[] = {

  "complex?",
  "l4:y12:syntax-rules;n;l2:py1:_;y4:args;;py7:number?;y4:args;;;l2:y1:_;"
  "y7:number?;;",

  "real?",
  "l4:y12:syntax-rules;n;l2:py1:_;y4:args;;py7:number?;y4:args;;;l2:y1:_;"
  "y7:number?;;",

  "rational?",
  "l4:y12:syntax-rules;n;l2:py1:_;y4:args;;py8:integer?;y4:args;;;l2:y1:_"
  ";y8:integer?;;",

  "exact-integer?",
  "l4:y12:syntax-rules;n;l2:py1:_;y4:args;;py7:fixnum?;y4:args;;;l2:y1:_;"
  "y7:fixnum?;;",

  "exact?",
  "l5:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l2:y7:fixnum?;l2:y4:%25ckn;y1:x"
  ";;;;l2:py1:_;y12:syntax-rules;;py16:%25residual-exact?;y12:syntax-rule"
  "s;;;l2:y1:_;y16:%25residual-exact?;;",

  0,
  "&0{%1.0%nI0]1}@!(y16:%25residual-exact?)",

  "inexact?",
  "l5:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l2:y7:flonum?;l2:y4:%25ckn;y1:x"
  ";;;;l2:py1:_;y12:syntax-rules;;py18:%25residual-inexact?;y12:syntax-ru"
  "les;;;l2:y1:_;y18:%25residual-inexact?;;",

  0,
  "&0{%1.0%nJ0]1}@!(y18:%25residual-inexact?)",

  "truncate-quotient",
  "l4:y12:syntax-rules;n;l2:py1:_;y4:args;;py8:quotient;y4:args;;;l2:y1:_"
  ";y8:quotient;;",

  "truncate-remainder",
  "l4:y12:syntax-rules;n;l2:py1:_;y4:args;;py9:remainder;y4:args;;;l2:y1:"
  "_;y9:remainder;;",

  "floor-quotient",
  "l4:y12:syntax-rules;n;l2:py1:_;y4:args;;py6:modquo;y4:args;;;l2:y1:_;y"
  "6:modquo;;",

  "floor-remainder",
  "l4:y12:syntax-rules;n;l2:py1:_;y4:args;;py6:modulo;y4:args;;;l2:y1:_;y"
  "6:modulo;;",

  0,
  "&0{%2.1,.1G4,.2,.2G3,@(y5:%25sdmv)[22}@!(y6:floor/)",

  0,
  "&0{%2.1,.1G6,.2,.2G5,@(y5:%25sdmv)[22}@!(y9:truncate/)",

  "char?",
  "l5:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l2:y6:%25charp;y1:x;;;l2:py1:_;"
  "y12:syntax-rules;;py15:%25residual-char?;y12:syntax-rules;;;l2:y1:_;y1"
  "5:%25residual-char?;;",

  0,
  "&0{%1.0C0]1}@!(y15:%25residual-char?)",

  "char-cmp",
  "l5:y12:syntax-rules;n;l2:l3:y1:_;y1:x;y1:y;;l3:y5:%25ccmp;y1:x;y1:y;;;"
  "l2:py1:_;y12:syntax-rules;;py18:%25residual-char-cmp;y12:syntax-rules;"
  ";;l2:y1:_;y18:%25residual-char-cmp;;",

  0,
  "&0{%2.1,.1O0]2}@!(y18:%25residual-char-cmp)",

  "char=?",
  "l5:y12:syntax-rules;n;l2:l3:y1:_;y1:x;y1:y;;l3:y4:%25ceq;y1:x;y1:y;;;l"
  "2:py1:_;y12:syntax-rules;;py16:%25residual-char=?;y12:syntax-rules;;;l"
  "2:y1:_;y16:%25residual-char=?;;",

  0,
  "&0{%2.1,.1C=]2}@!(y16:%25residual-char=?)",

  "char<?",
  "l5:y12:syntax-rules;n;l2:l3:y1:_;y1:x;y1:y;;l3:y4:%25clt;y1:x;y1:y;;;l"
  "2:py1:_;y12:syntax-rules;;py16:%25residual-char<?;y12:syntax-rules;;;l"
  "2:y1:_;y16:%25residual-char<?;;",

  0,
  "&0{%2.1,.1C<]2}@!(y16:%25residual-char<?)",

  "char<=?",
  "l5:y12:syntax-rules;n;l2:l3:y1:_;y1:x;y1:y;;l3:y4:%25cle;y1:x;y1:y;;;l"
  "2:py1:_;y12:syntax-rules;;py17:%25residual-char<=?;y12:syntax-rules;;;"
  "l2:y1:_;y17:%25residual-char<=?;;",

  0,
  "&0{%2.1,.1C>!]2}@!(y17:%25residual-char<=?)",

  "char>?",
  "l5:y12:syntax-rules;n;l2:l3:y1:_;y1:x;y1:y;;l3:y4:%25cgt;y1:x;y1:y;;;l"
  "2:py1:_;y12:syntax-rules;;py16:%25residual-char>?;y12:syntax-rules;;;l"
  "2:y1:_;y16:%25residual-char>?;;",

  0,
  "&0{%2.1,.1C>]2}@!(y16:%25residual-char>?)",

  "char>=?",
  "l5:y12:syntax-rules;n;l2:l3:y1:_;y1:x;y1:y;;l3:y4:%25cge;y1:x;y1:y;;;l"
  "2:py1:_;y12:syntax-rules;;py17:%25residual-char>=?;y12:syntax-rules;;;"
  "l2:y1:_;y17:%25residual-char>=?;;",

  0,
  "&0{%2.1,.1C<!]2}@!(y17:%25residual-char>=?)",

  "char-ci-cmp",
  "l5:y12:syntax-rules;n;l2:l3:y1:_;y1:x;y1:y;;l3:y6:%25cicmp;y1:x;y1:y;;"
  ";l2:py1:_;y12:syntax-rules;;py18:%25residual-char-cmp;y12:syntax-rules"
  ";;;l2:y1:_;y18:%25residual-char-cmp;;",

  0,
  "&0{%2.1,.1O1]2}@!(y18:%25residual-char-cmp)",

  "char-ci=?",
  "l5:y12:syntax-rules;n;l2:l3:y1:_;y1:x;y1:y;;l3:y5:%25cieq;y1:x;y1:y;;;"
  "l2:py1:_;y12:syntax-rules;;py19:%25residual-char-ci=?;y12:syntax-rules"
  ";;;l2:y1:_;y19:%25residual-char-ci=?;;",

  0,
  "&0{%2.1,.1Ci=]2}@!(y19:%25residual-char-ci=?)",

  "char-ci<?",
  "l5:y12:syntax-rules;n;l2:l3:y1:_;y1:x;y1:y;;l3:y5:%25cilt;y1:x;y1:y;;;"
  "l2:py1:_;y12:syntax-rules;;py19:%25residual-char-ci<?;y12:syntax-rules"
  ";;;l2:y1:_;y19:%25residual-char-ci<?;;",

  0,
  "&0{%2.1,.1Ci<]2}@!(y19:%25residual-char-ci<?)",

  "char-ci<=?",
  "l5:y12:syntax-rules;n;l2:l3:y1:_;y1:x;y1:y;;l3:y5:%25cile;y1:x;y1:y;;;"
  "l2:py1:_;y12:syntax-rules;;py20:%25residual-char-ci<=?;y12:syntax-rule"
  "s;;;l2:y1:_;y20:%25residual-char-ci<=?;;",

  0,
  "&0{%2.1,.1Ci>!]2}@!(y20:%25residual-char-ci<=?)",

  "char-ci>?",
  "l5:y12:syntax-rules;n;l2:l3:y1:_;y1:x;y1:y;;l3:y5:%25cigt;y1:x;y1:y;;;"
  "l2:py1:_;y12:syntax-rules;;py19:%25residual-char-ci>?;y12:syntax-rules"
  ";;;l2:y1:_;y19:%25residual-char-ci>?;;",

  0,
  "&0{%2.1,.1Ci>]2}@!(y19:%25residual-char-ci>?)",

  "char-ci>=?",
  "l5:y12:syntax-rules;n;l2:l3:y1:_;y1:x;y1:y;;l3:y5:%25cige;y1:x;y1:y;;;"
  "l2:py1:_;y12:syntax-rules;;py20:%25residual-char-ci>=?;y12:syntax-rule"
  "s;;;l2:y1:_;y20:%25residual-char-ci>=?;;",

  0,
  "&0{%2.1,.1Ci<!]2}@!(y20:%25residual-char-ci>=?)",

  "char-alphabetic?",
  "l5:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l2:y5:%25calp;y1:x;;;l2:py1:_;y"
  "12:syntax-rules;;py26:%25residual-char-alphabetic?;y12:syntax-rules;;;"
  "l2:y1:_;y26:%25residual-char-alphabetic?;;",

  0,
  "&0{%1.0C4]1}@!(y26:%25residual-char-alphabetic?)",

  "char-numeric?",
  "l5:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l2:y5:%25cnup;y1:x;;;l2:py1:_;y"
  "12:syntax-rules;;py23:%25residual-char-numeric?;y12:syntax-rules;;;l2:"
  "y1:_;y23:%25residual-char-numeric?;;",

  0,
  "&0{%1.0C5]1}@!(y23:%25residual-char-numeric?)",

  "char-whitespace?",
  "l5:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l2:y5:%25cwsp;y1:x;;;l2:py1:_;y"
  "12:syntax-rules;;py26:%25residual-char-whitespace?;y12:syntax-rules;;;"
  "l2:y1:_;y26:%25residual-char-whitespace?;;",

  0,
  "&0{%1.0C1]1}@!(y26:%25residual-char-whitespace?)",

  "char-upper-case?",
  "l5:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l2:y5:%25cucp;y1:x;;;l2:py1:_;y"
  "12:syntax-rules;;py26:%25residual-char-upper-case?;y12:syntax-rules;;;"
  "l2:y1:_;y26:%25residual-char-upper-case?;;",

  0,
  "&0{%1.0C3]1}@!(y26:%25residual-char-upper-case?)",

  "char-lower-case?",
  "l5:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l2:y5:%25clcp;y1:x;;;l2:py1:_;y"
  "12:syntax-rules;;py26:%25residual-char-lower-case?;y12:syntax-rules;;;"
  "l2:y1:_;y26:%25residual-char-lower-case?;;",

  0,
  "&0{%1.0C2]1}@!(y26:%25residual-char-lower-case?)",

  "char-upcase",
  "l5:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l2:y5:%25cupc;y1:x;;;l2:py1:_;y"
  "12:syntax-rules;;py21:%25residual-char-upcase;y12:syntax-rules;;;l2:y1"
  ":_;y21:%25residual-char-upcase;;",

  0,
  "&0{%1.0C6]1}@!(y21:%25residual-char-upcase)",

  "char-downcase",
  "l5:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l2:y5:%25cdnc;y1:x;;;l2:py1:_;y"
  "12:syntax-rules;;py23:%25residual-char-downcase;y12:syntax-rules;;;l2:"
  "y1:_;y23:%25residual-char-downcase;;",

  0,
  "&0{%1.0C7]1}@!(y23:%25residual-char-downcase)",

  "char->integer",
  "l5:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l2:y5:%25ctoi;y1:x;;;l2:py1:_;y"
  "12:syntax-rules;;py23:%25residual-char->integer;y12:syntax-rules;;;l2:"
  "y1:_;y23:%25residual-char->integer;;",

  0,
  "&0{%1.0X8]1}@!(y23:%25residual-char->integer)",

  "integer->char",
  "l5:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l2:y5:%25itoc;y1:x;;;l2:py1:_;y"
  "12:syntax-rules;;py23:%25residual-integer->char;y12:syntax-rules;;;l2:"
  "y1:_;y23:%25residual-integer->char;;",

  0,
  "&0{%1.0X9]1}@!(y23:%25residual-integer->char)",

  "symbol?",
  "l5:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l2:y5:%25symp;y1:x;;;l2:py1:_;y"
  "12:syntax-rules;;py17:%25residual-symbol?;y12:syntax-rules;;;l2:y1:_;y"
  "17:%25residual-symbol?;;",

  0,
  "&0{%1.0Y0]1}@!(y17:%25residual-symbol?)",

  "symbol->string",
  "l5:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l2:y5:%25ytos;y1:x;;;l2:py1:_;y"
  "12:syntax-rules;;py24:%25residual-symbol->string;y12:syntax-rules;;;l2"
  ":y1:_;y24:%25residual-symbol->string;;",

  0,
  "&0{%1.0X4]1}@!(y24:%25residual-symbol->string)",

  "string->symbol",
  "l5:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l2:y5:%25stoy;y1:x;;;l2:py1:_;y"
  "12:syntax-rules;;py24:%25residual-string->symbol;y12:syntax-rules;;;l2"
  ":y1:_;y24:%25residual-string->symbol;;",

  0,
  "&0{%1.0X5]1}@!(y24:%25residual-string->symbol)",

  "list?",
  "l5:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l2:y6:%25listp;y1:x;;;l2:py1:_;"
  "y12:syntax-rules;;py15:%25residual-list?;y12:syntax-rules;;;l2:y1:_;y1"
  "5:%25residual-list?;;",

  0,
  "&0{%1.0L0]1}@!(y15:%25residual-list?)",

  0,
  "&0{%2n,.1%k,,#0.4,.1,&2{%2'0,.1I>!?{.1]2}.1,:1c,'1,.2I-,:0^[22}.!0.0^_"
  "1[22}@!(y10:%25make-list)",

  "make-list",
  "l6:y12:syntax-rules;n;l2:l2:y1:_;y1:n;;l3:y10:%25make-list;y1:n;f;;;l2"
  ":l3:y1:_;y1:n;y1:i;;l3:y10:%25make-list;y1:n;y1:i;;;l2:py1:_;y4:args;;"
  "py19:%25residual-make-list;y4:args;;;l2:y1:_;y19:%25residual-make-list"
  ";;",

  "list",
  "l6:y12:syntax-rules;n;l2:l1:y1:_;;l2:y5:quote;n;;;l2:l2:y1:_;y1:x;;l3:"
  "y4:cons;y1:x;l2:y5:quote;n;;;;l2:l3:y1:_;y1:x;y3:...;;l3:y5:%25list;y1"
  ":x;y3:...;;;l2:y1:_;y14:%25residual-list;;",

  "length",
  "l5:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l2:y5:%25llen;y1:x;;;l2:py1:_;y"
  "12:syntax-rules;;py16:%25residual-length;y12:syntax-rules;;;l2:y1:_;y1"
  "6:%25residual-length;;",

  0,
  "&0{%1.0g]1}@!(y16:%25residual-length)",

  "list-ref",
  "l5:y12:syntax-rules;n;l2:l3:y1:_;y1:x;y1:i;;l3:y5:%25lget;y1:x;y1:i;;;"
  "l2:py1:_;y12:syntax-rules;;py18:%25residual-list-ref;y12:syntax-rules;"
  ";;l2:y1:_;y18:%25residual-list-ref;;",

  0,
  "&0{%2.1,.1L4]2}@!(y18:%25residual-list-ref)",

  "list-set!",
  "l5:y12:syntax-rules;n;l2:l4:y1:_;y1:x;y1:i;y1:v;;l4:y5:%25lput;y1:x;y1"
  ":i;y1:v;;;l2:py1:_;y12:syntax-rules;;py19:%25residual-list-set!;y12:sy"
  "ntax-rules;;;l2:y1:_;y19:%25residual-list-set!;;",

  0,
  "&0{%3.2,.2,.2L5]3}@!(y19:%25residual-list-set!)",

  "append",
  "l7:y12:syntax-rules;n;l2:l1:y1:_;;l2:y5:quote;n;;;l2:l2:y1:_;y1:x;;y1:"
  "x;;l2:l3:y1:_;y1:x;y1:y;;l3:y5:%25lcat;y1:x;y1:y;;;l2:l5:y1:_;y1:x;y1:"
  "y;y1:z;y3:...;;l3:y5:%25lcat;y1:x;l4:y6:append;y1:y;y1:z;y3:...;;;;l2:"
  "y1:_;y16:%25residual-append;;",

  "memq",
  "l5:y12:syntax-rules;n;l2:l3:y1:_;y1:v;y1:y;;l3:y5:%25memq;y1:v;y1:y;;;"
  "l2:py1:_;y12:syntax-rules;;py14:%25residual-memq;y12:syntax-rules;;;l2"
  ":y1:_;y14:%25residual-memq;;",

  0,
  "&0{%2.1,.1A0]2}@!(y14:%25residual-memq)",

  "memv",
  "l5:y12:syntax-rules;n;l2:l3:y1:_;y1:v;y1:y;;l3:y5:%25memv;y1:v;l2:y4:%"
  "25ckl;y1:y;;;;l2:py1:_;y12:syntax-rules;;py14:%25residual-memv;y12:syn"
  "tax-rules;;;l2:y1:_;y14:%25residual-memv;;",

  0,
  "&0{%2.1%l,.1A1]2}@!(y14:%25residual-memv)",

  0,
  "&0{%3.1p?{${.3a,.3,.6[02}?{.1]3}.2,.2d,.2,@(y7:%25member)[33}f]3}@!(y7"
  ":%25member)",

  "member",
  "l6:y12:syntax-rules;n;l2:l3:y1:_;y1:v;y1:y;;l3:y5:%25meme;y1:v;l2:y4:%"
  "25ckl;y1:y;;;;l2:l4:y1:_;y1:v;y1:y;y2:eq;;l4:y7:%25member;y1:v;y1:y;y2"
  ":eq;;;l2:py1:_;y4:args;;py16:%25residual-member;y4:args;;;l2:y1:_;y16:"
  "%25residual-member;;",

  "assq",
  "l5:y12:syntax-rules;n;l2:l3:y1:_;y1:v;y1:y;;l3:y5:%25assq;y1:v;y1:y;;;"
  "l2:py1:_;y12:syntax-rules;;py14:%25residual-assq;y12:syntax-rules;;;l2"
  ":y1:_;y14:%25residual-assq;;",

  0,
  "&0{%2.1,.1A3]2}@!(y14:%25residual-assq)",

  "assv",
  "l5:y12:syntax-rules;n;l2:l3:y1:_;y1:v;y1:y;;l3:y5:%25assv;y1:v;l2:y4:%"
  "25ckl;y1:y;;;;l2:py1:_;y12:syntax-rules;;py14:%25residual-assv;y12:syn"
  "tax-rules;;;l2:y1:_;y14:%25residual-assv;;",

  0,
  "&0{%2.1%l,.1A4]2}@!(y14:%25residual-assv)",

  0,
  "&0{%3.1p?{${.3aa,.3,.6[02}?{.1a]3}.2,.2d,.2,@(y6:%25assoc)[33}f]3}@!(y"
  "6:%25assoc)",

  "assoc",
  "l6:y12:syntax-rules;n;l2:l3:y1:_;y1:v;y2:al;;l3:y5:%25asse;y1:v;l2:y4:"
  "%25ckl;y2:al;;;;l2:l4:y1:_;y1:v;y2:al;y2:eq;;l4:y6:%25assoc;y1:v;y2:al"
  ";y2:eq;;;l2:py1:_;y4:args;;py15:%25residual-assoc;y4:args;;;l2:y1:_;y1"
  "5:%25residual-assoc;;",

  "list-copy",
  "l5:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l3:y5:%25lcat;y1:x;l2:y5:quote;"
  "n;;;;l2:py1:_;y12:syntax-rules;;py19:%25residual-list-copy;y12:syntax-"
  "rules;;;l2:y1:_;y19:%25residual-list-copy;;",

  0,
  "&0{%1n,.1L6]1}@!(y19:%25residual-list-copy)",

  "list-tail",
  "l5:y12:syntax-rules;n;l2:l3:y1:_;y1:x;y1:i;;l3:y6:%25ltail;y1:x;y1:i;;"
  ";l2:py1:_;y12:syntax-rules;;py19:%25residual-list-tail;y12:syntax-rule"
  "s;;;l2:y1:_;y19:%25residual-list-tail;;",

  0,
  "&0{%2.1,.1A6]2}@!(y19:%25residual-list-tail)",

  "last-pair",
  "l5:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l2:y6:%25lpair;y1:x;;;l2:py1:_;"
  "y12:syntax-rules;;py19:%25residual-last-pair;y12:syntax-rules;;;l2:y1:"
  "_;y19:%25residual-last-pair;;",

  0,
  "&0{%1.0A7]1}@!(y19:%25residual-last-pair)",

  "reverse",
  "l5:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l2:y5:%25lrev;y1:x;;;l2:py1:_;y"
  "12:syntax-rules;;py17:%25residual-reverse;y12:syntax-rules;;;l2:y1:_;y"
  "17:%25residual-reverse;;",

  0,
  "&0{%1.0A8]1}@!(y17:%25residual-reverse)",

  "reverse!",
  "l5:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l2:y6:%25lrevi;y1:x;;;l2:py1:_;"
  "y12:syntax-rules;;py18:%25residual-reverse!;y12:syntax-rules;;;l2:y1:_"
  ";y18:%25residual-reverse!;;",

  0,
  "&0{%1.0A9]1}@!(y18:%25residual-reverse!)",

  "list*",
  "l7:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;y1:x;;l2:l3:y1:_;y1:x;y1:y;;l3:"
  "y4:cons;y1:x;y1:y;;;l2:l5:y1:_;y1:x;y1:y;y1:z;y3:...;;l3:y4:cons;y1:x;"
  "l4:y5:list*;y1:y;y1:z;y3:...;;;;l2:py1:_;y4:args;;py15:%25residual-lis"
  "t*;y4:args;;;l2:y1:_;y15:%25residual-list*;;",

  "cons*",
  "l4:y12:syntax-rules;n;l2:py1:_;y4:args;;py5:list*;y4:args;;;l2:y1:_;y5"
  ":list*;;",

  "vector?",
  "l5:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l2:y5:%25vecp;y1:x;;;l2:py1:_;y"
  "12:syntax-rules;;py17:%25residual-vector?;y12:syntax-rules;;;l2:y1:_;y"
  "17:%25residual-vector?;;",

  0,
  "&0{%1.0V0]1}@!(y17:%25residual-vector?)",

  "vector",
  "l4:y12:syntax-rules;n;l2:py1:_;y4:args;;py4:%25vec;y4:args;;;l2:y1:_;y"
  "4:%25vec;;",

  "make-vector",
  "l6:y12:syntax-rules;n;l2:l2:y1:_;y1:n;;l3:y4:%25vmk;y1:n;f;;;l2:l3:y1:"
  "_;y1:n;y1:v;;l3:y4:%25vmk;y1:n;y1:v;;;l2:py1:_;y4:args;;py21:%25residu"
  "al-make-vector;y4:args;;;l2:y1:_;y21:%25residual-make-vector;;",

  "vector-length",
  "l5:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l2:y5:%25vlen;y1:x;;;l2:py1:_;y"
  "12:syntax-rules;;py23:%25residual-vector-length;y12:syntax-rules;;;l2:"
  "y1:_;y23:%25residual-vector-length;;",

  0,
  "&0{%1.0V3]1}@!(y23:%25residual-vector-length)",

  "vector-ref",
  "l5:y12:syntax-rules;n;l2:l3:y1:_;y1:x;y1:i;;l3:y5:%25vget;y1:x;y1:i;;;"
  "l2:py1:_;y12:syntax-rules;;py20:%25residual-vector-ref;y12:syntax-rule"
  "s;;;l2:y1:_;y20:%25residual-vector-ref;;",

  0,
  "&0{%2.1,.1V4]2}@!(y20:%25residual-vector-ref)",

  "vector-set!",
  "l5:y12:syntax-rules;n;l2:l4:y1:_;y1:x;y1:i;y1:v;;l4:y5:%25vput;y1:x;y1"
  ":i;y1:v;;;l2:py1:_;y12:syntax-rules;;py21:%25residual-vector-set!;y12:"
  "syntax-rules;;;l2:y1:_;y21:%25residual-vector-set!;;",

  0,
  "&0{%3.2,.2,.2V5]3}@!(y21:%25residual-vector-set!)",

  "list->vector",
  "l5:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l2:y5:%25ltov;y1:x;;;l2:py1:_;y"
  "12:syntax-rules;;py22:%25residual-list->vector;y12:syntax-rules;;;l2:y"
  "1:_;y22:%25residual-list->vector;;",

  0,
  "&0{%1.0X1]1}@!(y22:%25residual-list->vector)",

  0,
  "&0{%3n,'1,.4I-,,#0.3,.1,.6,&3{%2:0,.1I<?{.1]2}.1,.1,:2V4c,'1,.2I-,:1^["
  "22}.!0.0^_1[32}@!(y15:subvector->list)",

  "vector->list",
  "l5:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l2:y5:%25vtol;y1:x;;;l2:py1:_;y"
  "1:r;;py22:%25residual-vector->list;y1:r;;;l2:y1:_;y22:%25residual-vect"
  "or->list;;",

  0,
  "&0{%3.2,.2,.2,@(y15:subvector->list)[33}%x,&0{%2.0V3,.2,.2,@(y15:subve"
  "ctor->list)[23}%x,&0{%1.0X0]1}%x,&3{|10|21|32%%}@!(y22:%25residual-vec"
  "tor->list)",

  0,
  "&0{%5.1,.1V3I-,.4I+,.5In,.4,.3I>!?{.4,.3,,#0.3,.7,.6,.3,&4{%2:3,.2I<!,"
  ".0?{.0]3}.2,:2V4,.2,:1V5.2'1,.3I+,.2'1,.3I+,:0^[32}.!0.0^_1[62}'1,.1I-"
  ",'1,.6,.8I-I-,.4I+,,#0.7,.7,.6,.3,&4{%2:3,.2I<,.0?{.0]3}.2,:2V4,.2,:1V"
  "5.2'1,.3I-,.2'1,.3I-,:0^[32}.!0.0^_1[62}@!(y15:subvector-copy!)",

  0,
  "&0{%5.4,.4,.4,.4,.4,@(y15:subvector-copy!)[55}%x,&0{%4.2V3,.4,.4,.4,.4"
  ",@(y15:subvector-copy!)[45}%x,&0{%3.2V3,'0,.4,.4,.4,@(y15:subvector-co"
  "py!)[35}%x,&3{|30|41|52%%}@!(y12:vector-copy!)",

  0,
  "&0{%3f,.2,.4I-V2,${.5,.5,.5,'0,.6,@(y15:subvector-copy!)[05}.0]4}@!(y9"
  ":subvector)",

  0,
  "&0{%3.2,.2,.2,@(y9:subvector)[33}%x,&0{%2.0V3,.2,.2,@(y9:subvector)[23"
  "}%x,&0{%1.0V3,'0,.2,@(y9:subvector)[13}%x,&3{|10|21|32%%}@!(y11:vector"
  "-copy)",

  0,
  "&0{%4.2,,#0.5,.4,.4,.3,&4{%1:3,.1I<!,.0?{.0]2}:2,.2,:1V5.1'1,.2I+,:0^["
  "21}.!0.0^_1[41}@!(y15:subvector-fill!)",

  0,
  "&0{%4.3,.3,.3,.3,@(y15:subvector-fill!)[44}%x,&0{%3.0V3,.3,.3,.3,@(y15"
  ":subvector-fill!)[34}%x,&0{%2.0V3,'0,.3,.3,@(y15:subvector-fill!)[24}%"
  "x,&3{|20|31|42%%}@!(y12:vector-fill!)",

  0,
  "&0{%5.1,.1S3I-,.4I+,.5In,.4,.3,,#0.0,.5,.8,.6,&4{%2:0,.2I<!?{:2]2}.1,:"
  "1V4,.1,:2S5.1'1,.2I+,.1'1,.2I+,:3^[22}.!0.0^_1[62}@!(y22:subvector-str"
  "ing-copy!)",

  0,
  "&0{%3.2,.2,.2,'0,'(c ),.6,.8I-S2,@(y22:subvector-string-copy!)[35}@!(y"
  "17:subvector->string)",

  0,
  "&0{%3.2,.2,.2,@(y17:subvector->string)[33}%x,&0{%2.0V3,.2,.2,@(y17:sub"
  "vector->string)[23}%x,&0{%1.0V3,'0,.2,@(y17:subvector->string)[13}%x,&"
  "3{|10|21|32%%}@!(y14:vector->string)",

  0,
  "&0{%1'0,.1,,#0.0,&1{%2.0u?{.1]2}.0aV3,.2I+,.1d,:0^[22}.!0.0^_1[12}@!(y"
  "18:vectors-sum-length)",

  0,
  "&0{%2'0,.2,,#0.0,.4,&2{%2.0u?{:0]2}.0d,.1a,.0V3,${.2,'0,.5,.9,:0,@(y15"
  ":subvector-copy!)[05}.0,.5I+,.3,:1^[52}.!0.0^_1[22}@!(y18:vectors-copy"
  "-into!)",

  0,
  "&0{%!0.0,f,${.4,@(y18:vectors-sum-length)[01}V2,@(y18:vectors-copy-int"
  "o!)[12}@!(y23:%25residual-vector-append)",

  "vector-append",
  "l7:y12:syntax-rules;n;l2:l1:y1:_;;l2:y5:quote;v0:;;;l2:l2:y1:_;y1:x;;l"
  "2:y4:%25ckv;y1:x;;;l2:l3:y1:_;y1:x;y1:y;;l3:y5:%25vcat;y1:x;y1:y;;;l2:"
  "py1:_;y1:r;;py23:%25residual-vector-append;y1:r;;;l2:y1:_;y23:%25resid"
  "ual-vector-append;;",

  "string?",
  "l5:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l2:y5:%25strp;y1:x;;;l2:py1:_;y"
  "12:syntax-rules;;py17:%25residual-string?;y12:syntax-rules;;;l2:y1:_;y"
  "17:%25residual-string?;;",

  0,
  "&0{%1.0S0]1}@!(y17:%25residual-string?)",

  "string",
  "l4:y12:syntax-rules;n;l2:l3:y1:_;y1:c;y3:...;;l3:y4:%25str;y1:c;y3:..."
  ";;;l2:y1:_;y16:%25residual-string;;",

  "make-string",
  "l6:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l3:y4:%25smk;y1:x;c ;;;l2:l3:y1"
  ":_;y1:x;y1:y;;l3:y4:%25smk;y1:x;y1:y;;;l2:py1:_;y4:args;;py21:%25resid"
  "ual-make-string;y4:args;;;l2:y1:_;y21:%25residual-make-string;;",

  "string-length",
  "l5:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l2:y5:%25slen;y1:x;;;l2:py1:_;y"
  "12:syntax-rules;;py23:%25residual-string-length;y12:syntax-rules;;;l2:"
  "y1:_;y23:%25residual-string-length;;",

  0,
  "&0{%1.0S3]1}@!(y23:%25residual-string-length)",

  "string-ref",
  "l5:y12:syntax-rules;n;l2:l3:y1:_;y1:x;y1:i;;l3:y5:%25sget;y1:x;y1:i;;;"
  "l2:py1:_;y12:syntax-rules;;py20:%25residual-string-ref;y12:syntax-rule"
  "s;;;l2:y1:_;y20:%25residual-string-ref;;",

  0,
  "&0{%2.1,.1S4]2}@!(y20:%25residual-string-ref)",

  "string-set!",
  "l5:y12:syntax-rules;n;l2:l4:y1:_;y1:x;y1:i;y1:v;;l4:y5:%25sput;y1:x;y1"
  ":i;y1:v;;;l2:py1:_;y12:syntax-rules;;py21:%25residual-string-set!;y12:"
  "syntax-rules;;;l2:y1:_;y21:%25residual-string-set!;;",

  0,
  "&0{%3.2,.2,.2S5]3}@!(y21:%25residual-string-set!)",

  "string-append",
  "l7:y12:syntax-rules;n;l2:l1:y1:_;;s0:;;l2:l2:y1:_;y1:x;;l2:y4:%25cks;y"
  "1:x;;;l2:l3:y1:_;y1:x;y1:y;;l3:y5:%25scat;y1:x;y1:y;;;l2:l5:y1:_;y1:x;"
  "y1:y;y1:z;y3:...;;l3:y13:string-append;y1:x;l4:y13:string-append;y1:y;"
  "y1:z;y3:...;;;;l2:y1:_;y23:%25residual-string-append;;",

  "list->string",
  "l5:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l2:y5:%25ltos;y1:x;;;l2:py1:_;y"
  "12:syntax-rules;;py22:%25residual-list->string;y12:syntax-rules;;;l2:y"
  "1:_;y22:%25residual-list->string;;",

  0,
  "&0{%1.0X3]1}@!(y22:%25residual-list->string)",

  0,
  "&0{%3n,'1,.4I-,,#0.3,.1,.6,&3{%2:0,.1I<?{.1]2}.1,.1,:2S4c,'1,.2I-,:1^["
  "22}.!0.0^_1[32}@!(y15:substring->list)",

  "string->list",
  "l5:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l2:y5:%25stol;y1:x;;;l2:py1:_;y"
  "1:r;;py22:%25residual-string->list;y1:r;;;l2:y1:_;y22:%25residual-stri"
  "ng->list;;",

  0,
  "&0{%3.2,.2,.2,@(y15:substring->list)[33}%x,&0{%2.0S3,.2,.2,@(y15:subst"
  "ring->list)[23}%x,&0{%1.0X2]1}%x,&3{|10|21|32%%}@!(y22:%25residual-str"
  "ing->list)",

  0,
  "&0{%5.1,.1S3I-,.4I+,.5In,.4,.3I>!?{.4,.3,,#0.3,.7,.6,.3,&4{%2:3,.2I<!,"
  ".0?{.0]3}.2,:2S4,.2,:1S5.2'1,.3I+,.2'1,.3I+,:0^[32}.!0.0^_1[62}'1,.1I-"
  ",'1,.6,.8I-I-,.4I+,,#0.7,.7,.6,.3,&4{%2:3,.2I<,.0?{.0]3}.2,:2S4,.2,:1S"
  "5.2'1,.3I-,.2'1,.3I-,:0^[32}.!0.0^_1[62}@!(y15:substring-copy!)",

  0,
  "&0{%5.4,.4,.4,.4,.4,@(y15:substring-copy!)[55}%x,&0{%4.2S3,.4,.4,.4,.4"
  ",@(y15:substring-copy!)[45}%x,&0{%3.2S3,'0,.4,.4,.4,@(y15:substring-co"
  "py!)[35}%x,&3{|30|41|52%%}@!(y12:string-copy!)",

  "substring",
  "l5:y12:syntax-rules;n;l2:l4:y1:_;y1:x;y1:s;y1:e;;l4:y5:%25ssub;y1:x;y1"
  ":s;y1:e;;;l2:py1:_;y12:syntax-rules;;py19:%25residual-substring;y12:sy"
  "ntax-rules;;;l2:y1:_;y19:%25residual-substring;;",

  0,
  "&0{%3.2,.2,.2S7]3}@!(y19:%25residual-substring)",

  0,
  "&0{%3.2,.2,.2S7]3}%x,&0{%2.0S3,.2,.2S7]2}%x,&0{%1.0S3,'0,.2S7]1}%x,&3{"
  "|10|21|32%%}@!(y11:string-copy)",

  0,
  "&0{%4.2,,#0.5,.4,.4,.3,&4{%1:3,.1I<!,.0?{.0]2}:2,.2,:1S5.1'1,.2I+,:0^["
  "21}.!0.0^_1[41}@!(y15:substring-fill!)",

  0,
  "&0{%4.3,.3,.3,.3,@(y15:substring-fill!)[44}%x,&0{%3.0S3,.3,.3,.3,@(y15"
  ":substring-fill!)[34}%x,&0{%2.0S3,'0,.3,.3,@(y15:substring-fill!)[24}%"
  "x,&3{|20|31|42%%}@!(y12:string-fill!)",

  0,
  "&0{%5.1,.1V3I-,.4I+,.5In,.4,.3,,#0.0,.5,.8,.6,&4{%2:0,.2I<!?{:2]2}.1,:"
  "1S4,.1,:2V5.1'1,.2I+,.1'1,.2I+,:3^[22}.!0.0^_1[62}@!(y22:substring-vec"
  "tor-copy!)",

  0,
  "&0{%3.2,.2,.2,'0,f,.6,.8I-V2,@(y22:substring-vector-copy!)[35}@!(y17:s"
  "ubstring->vector)",

  0,
  "&0{%3.2,.2,.2,@(y17:substring->vector)[33}%x,&0{%2.0S3,.2,.2,@(y17:sub"
  "string->vector)[23}%x,&0{%1.0S3,'0,.2,@(y17:substring->vector)[13}%x,&"
  "3{|10|21|32%%}@!(y14:string->vector)",

  0,
  "&0{%1'0,.1,,#0.0,&1{%2.0u?{.1]2}.0aS3,.2I+,.1d,:0^[22}.!0.0^_1[12}@!(y"
  "18:strings-sum-length)",

  0,
  "&0{%2'0,.2,,#0.0,.4,&2{%2.0u?{:0]2}.0d,.1a,.0S3,${.2,'0,.5,.9,:0,@(y15"
  ":substring-copy!)[05}.0,.5I+,.3,:1^[52}.!0.0^_1[22}@!(y18:strings-copy"
  "-into!)",

  0,
  "&0{%!0.0,'(c ),${.4,@(y18:strings-sum-length)[01}S2,@(y18:strings-copy"
  "-into!)[12}@!(y23:%25residual-string-append)",

  "string-append",
  "l7:y12:syntax-rules;n;l2:l1:y1:_;;s0:;;l2:l2:y1:_;y1:x;;l2:y4:%25cks;y"
  "1:x;;;l2:l3:y1:_;y1:x;y1:y;;l3:y5:%25scat;y1:x;y1:y;;;l2:py1:_;y1:r;;p"
  "y23:%25residual-string-append;y1:r;;;l2:y1:_;y23:%25residual-string-ap"
  "pend;;",

  "string-cmp",
  "l5:y12:syntax-rules;n;l2:l3:y1:_;y1:x;y1:y;;l3:y5:%25scmp;y1:x;y1:y;;;"
  "l2:py1:_;y12:syntax-rules;;py20:%25residual-string-cmp;y12:syntax-rule"
  "s;;;l2:y1:_;y20:%25residual-string-cmp;;",

  0,
  "&0{%2.1,.1O2]2}@!(y20:%25residual-string-cmp)",

  "string=?",
  "l5:y12:syntax-rules;n;l2:l3:y1:_;y1:x;y1:y;;l3:y4:%25seq;y1:x;y1:y;;;l"
  "2:py1:_;y12:syntax-rules;;py18:%25residual-string<?;y12:syntax-rules;;"
  ";l2:y1:_;y18:%25residual-string<?;;",

  0,
  "&0{%2.1,.1S=]2}@!(y18:%25residual-string<?)",

  "string<?",
  "l5:y12:syntax-rules;n;l2:l3:y1:_;y1:x;y1:y;;l3:y4:%25slt;y1:x;y1:y;;;l"
  "2:py1:_;y12:syntax-rules;;py18:%25residual-string<?;y12:syntax-rules;;"
  ";l2:y1:_;y18:%25residual-string<?;;",

  0,
  "&0{%2.1,.1S<]2}@!(y18:%25residual-string<?)",

  "string<=?",
  "l5:y12:syntax-rules;n;l2:l3:y1:_;y1:x;y1:y;;l3:y4:%25sle;y1:x;y1:y;;;l"
  "2:py1:_;y12:syntax-rules;;py19:%25residual-string<=?;y12:syntax-rules;"
  ";;l2:y1:_;y19:%25residual-string<=?;;",

  0,
  "&0{%2.1,.1S>!]2}@!(y19:%25residual-string<=?)",

  "string>?",
  "l5:y12:syntax-rules;n;l2:l3:y1:_;y1:x;y1:y;;l3:y4:%25sgt;y1:x;y1:y;;;l"
  "2:py1:_;y12:syntax-rules;;py18:%25residual-string>?;y12:syntax-rules;;"
  ";l2:y1:_;y18:%25residual-string>?;;",

  0,
  "&0{%2.1,.1S>]2}@!(y18:%25residual-string>?)",

  "string>=?",
  "l5:y12:syntax-rules;n;l2:l3:y1:_;y1:x;y1:y;;l3:y4:%25sge;y1:x;y1:y;;;l"
  "2:py1:_;y12:syntax-rules;;py19:%25residual-string>=?;y12:syntax-rules;"
  ";;l2:y1:_;y19:%25residual-string>=?;;",

  0,
  "&0{%2.1,.1S<!]2}@!(y19:%25residual-string>=?)",

  "string-ci-cmp",
  "l5:y12:syntax-rules;n;l2:l3:y1:_;y1:x;y1:y;;l3:y6:%25sicmp;y1:x;y1:y;;"
  ";l2:py1:_;y12:syntax-rules;;py20:%25residual-string-cmp;y12:syntax-rul"
  "es;;;l2:y1:_;y20:%25residual-string-cmp;;",

  0,
  "&0{%2.1,.1O3]2}@!(y20:%25residual-string-cmp)",

  "string-ci=?",
  "l5:y12:syntax-rules;n;l2:l3:y1:_;y1:x;y1:y;;l3:y5:%25sieq;y1:x;y1:y;;;"
  "l2:py1:_;y12:syntax-rules;;py18:%25residual-string<?;y12:syntax-rules;"
  ";;l2:y1:_;y18:%25residual-string<?;;",

  0,
  "&0{%2.1,.1Si=]2}@!(y18:%25residual-string<?)",

  "string-ci<?",
  "l5:y12:syntax-rules;n;l2:l3:y1:_;y1:x;y1:y;;l3:y5:%25silt;y1:x;y1:y;;;"
  "l2:py1:_;y12:syntax-rules;;py18:%25residual-string<?;y12:syntax-rules;"
  ";;l2:y1:_;y18:%25residual-string<?;;",

  0,
  "&0{%2.1,.1Si<]2}@!(y18:%25residual-string<?)",

  "string-ci<=?",
  "l5:y12:syntax-rules;n;l2:l3:y1:_;y1:x;y1:y;;l3:y5:%25sile;y1:x;y1:y;;;"
  "l2:py1:_;y12:syntax-rules;;py19:%25residual-string<=?;y12:syntax-rules"
  ";;;l2:y1:_;y19:%25residual-string<=?;;",

  0,
  "&0{%2.1,.1Si>!]2}@!(y19:%25residual-string<=?)",

  "string-ci>?",
  "l5:y12:syntax-rules;n;l2:l3:y1:_;y1:x;y1:y;;l3:y5:%25sigt;y1:x;y1:y;;;"
  "l2:py1:_;y12:syntax-rules;;py18:%25residual-string>?;y12:syntax-rules;"
  ";;l2:y1:_;y18:%25residual-string>?;;",

  0,
  "&0{%2.1,.1Si>]2}@!(y18:%25residual-string>?)",

  "string-ci>=?",
  "l5:y12:syntax-rules;n;l2:l3:y1:_;y1:x;y1:y;;l3:y5:%25sige;y1:x;y1:y;;;"
  "l2:py1:_;y12:syntax-rules;;py19:%25residual-string>=?;y12:syntax-rules"
  ";;;l2:y1:_;y19:%25residual-string>=?;;",

  0,
  "&0{%2.1,.1Si<!]2}@!(y19:%25residual-string>=?)",

  "procedure?",
  "l5:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l2:y5:%25funp;y1:x;;;l2:py1:_;y"
  "12:syntax-rules;;py20:%25residual-procedure?;y12:syntax-rules;;;l2:y1:"
  "_;y20:%25residual-procedure?;;",

  0,
  "&0{%1.0K0]1}@!(y20:%25residual-procedure?)",

  "apply",
  "l6:y12:syntax-rules;n;l2:l3:y1:_;y1:p;y1:l;;l3:y5:%25appl;y1:p;y1:l;;;"
  "l2:l6:y1:_;y1:p;y1:a;y1:b;y3:...;y1:l;;l3:y5:%25appl;y1:p;l5:y5:list*;"
  "y1:a;y1:b;y3:...;y1:l;;;;l2:py1:_;y4:args;;py15:%25residual-apply;y4:a"
  "rgs;;;l2:y1:_;y15:%25residual-apply;;",

  "call/cc",
  "l5:y12:syntax-rules;n;l2:l2:y1:_;y1:f;;l3:y5:letcc;y1:k;l2:y1:f;y1:k;;"
  ";;l2:py1:_;y12:syntax-rules;;py17:%25residual-call/cc;y12:syntax-rules"
  ";;;l2:y1:_;y17:%25residual-call/cc;;",

  0,
  "&0{%1k1,.0,.2[21}@!(y17:%25residual-call/cc)",

  "call-with-current-continuation",
  "l4:y12:syntax-rules;n;l2:py1:_;y4:args;;py7:call/cc;y4:args;;;l2:y1:_;"
  "y7:call/cc;;",

  "values",
  "l4:y12:syntax-rules;n;l2:py1:_;y4:args;;py5:%25sdmv;y4:args;;;l2:y1:_;"
  "y5:%25sdmv;;",

  "call-with-values",
  "l4:y12:syntax-rules;n;l2:py1:_;y4:args;;py5:%25cwmv;y4:args;;;l2:y1:_;"
  "y5:%25cwmv;;",

  "map",
  "l5:y12:syntax-rules;n;l2:l3:y1:_;y3:fun;y3:lst;;l3:y3:let;l1:l2:y1:f;y"
  "3:fun;;;l4:y3:let;y4:loop;l1:l2:y1:l;y3:lst;;;l4:y2:if;l2:y5:pair?;y1:"
  "l;;l3:y4:cons;l2:y1:f;l2:y3:car;y1:l;;;l2:y4:loop;l2:y3:cdr;y1:l;;;;l2"
  ":y5:quote;n;;;;;;l2:py1:_;y4:args;;py13:%25residual-map;y4:args;;;l2:y"
  "1:_;y13:%25residual-map;;",

  "for-each",
  "l5:y12:syntax-rules;n;l2:l3:y1:_;y3:fun;y3:lst;;l3:y3:let;l1:l2:y1:f;y"
  "3:fun;;;l4:y3:let;y4:loop;l1:l2:y1:l;y3:lst;;;l3:y2:if;l2:y5:pair?;y1:"
  "l;;l3:y5:begin;l2:y1:f;l2:y3:car;y1:l;;;l2:y4:loop;l2:y3:cdr;y1:l;;;;;"
  ";;;l2:py1:_;y4:args;;py18:%25residual-for-each;y4:args;;;l2:y1:_;y18:%"
  "25residual-for-each;;",

  0,
  "&0{%!2.0u?{.2S3,'(c ),.1S2,'0,,#0.0,.3,.8,.8,.7,&5{%1:0,.1I<!?{:3]1}${"
  ".2,:2S4,:1[01},.1,:3S5.0'1,.1I+,:4^[11}.!0.0^_1[51}${@(y22:%25residual"
  "-string->list),${.5,.8c,,#0.4,.1,&2{%1.0p?{${.2d,:0^[01},${.3a,:1[01}c"
  "]1}n]1}.!0.0^_1[01}_1,.4c,@(y13:%25residual-map),@(y5:%25appl)[02}X3]3"
  "}@!(y10:string-map)",

  0,
  "&0{%!2.0u?{.2V3,f,.1V2,'0,,#0.0,.3,.8,.8,.7,&5{%1:0,.1I<!?{:3]1}${.2,:"
  "2V4,:1[01},.1,:3V5.0'1,.1I+,:4^[11}.!0.0^_1[51}${@(y22:%25residual-vec"
  "tor->list),${.5,.8c,,#0.4,.1,&2{%1.0p?{${.2d,:0^[01},${.3a,:1[01}c]1}n"
  "]1}.!0.0^_1[01}_1,.4c,@(y13:%25residual-map),@(y5:%25appl)[02}X1]3}@!("
  "y10:vector-map)",

  0,
  "&0{%!2.0u?{.2S3,'0,,#0.2,.6,.6,.3,&4{%1:3,.1I<!,.0?{.0]2}${.3,:2S4,:1["
  "01}.1'1,.2I+,:0^[21}.!0.0^_1[41}@(y22:%25residual-string->list),${.3,."
  "6c,,#0.4,.1,&2{%1.0p?{${.2d,:0^[01},${.3a,:1[01}c]1}n]1}.!0.0^_1[01}_1"
  ",.2c,@(y18:%25residual-for-each),@(y5:%25appl)[32}@!(y15:string-for-ea"
  "ch)",

  0,
  "&0{%!2.0u?{.2V3,'0,,#0.2,.6,.6,.3,&4{%1:3,.1I<!,.0?{.0]2}${.3,:2V4,:1["
  "01}.1'1,.2I+,:0^[21}.!0.0^_1[41}@(y22:%25residual-vector->list),${.3,."
  "6c,,#0.4,.1,&2{%1.0p?{${.2d,:0^[01},${.3a,:1[01}c]1}n]1}.!0.0^_1[01}_1"
  ",.2c,@(y18:%25residual-for-each),@(y5:%25appl)[32}@!(y15:vector-for-ea"
  "ch)",

  "input-port?",
  "l5:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l2:y4:%25ipp;y1:x;;;l2:py1:_;y1"
  "2:syntax-rules;;py21:%25residual-input-port?;y12:syntax-rules;;;l2:y1:"
  "_;y21:%25residual-input-port?;;",

  0,
  "&0{%1.0P00]1}@!(y21:%25residual-input-port?)",

  "output-port?",
  "l5:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l2:y4:%25opp;y1:x;;;l2:py1:_;y1"
  "2:syntax-rules;;py22:%25residual-output-port?;y12:syntax-rules;;;l2:y1"
  ":_;y22:%25residual-output-port?;;",

  0,
  "&0{%1.0P01]1}@!(y22:%25residual-output-port?)",

  "input-port-open?",
  "l5:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l2:y5:%25ipop;y1:x;;;l2:py1:_;y"
  "12:syntax-rules;;py26:%25residual-input-port-open?;y12:syntax-rules;;;"
  "l2:y1:_;y26:%25residual-input-port-open?;;",

  0,
  "&0{%1.0P20]1}@!(y26:%25residual-input-port-open?)",

  "output-port-open?",
  "l5:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l2:y5:%25opop;y1:x;;;l2:py1:_;y"
  "12:syntax-rules;;py27:%25residual-output-port-open?;y12:syntax-rules;;"
  ";l2:y1:_;y27:%25residual-output-port-open?;;",

  0,
  "&0{%1.0P21]1}@!(y27:%25residual-output-port-open?)",

  "current-input-port",
  "l5:y12:syntax-rules;n;l2:l1:y1:_;;l1:y4:%25sip;;;l2:py1:_;y12:syntax-r"
  "ules;;py28:%25residual-current-input-port;y12:syntax-rules;;;l2:y1:_;y"
  "28:%25residual-current-input-port;;",

  0,
  "&0{%0P10]0}@!(y28:%25residual-current-input-port)",

  "current-output-port",
  "l5:y12:syntax-rules;n;l2:l1:y1:_;;l1:y4:%25sop;;;l2:py1:_;y12:syntax-r"
  "ules;;py29:%25residual-current-output-port;y12:syntax-rules;;;l2:y1:_;"
  "y29:%25residual-current-output-port;;",

  0,
  "&0{%0P11]0}@!(y29:%25residual-current-output-port)",

  "current-error-port",
  "l5:y12:syntax-rules;n;l2:l1:y1:_;;l1:y4:%25sep;;;l2:py1:_;y12:syntax-r"
  "ules;;py28:%25residual-current-error-port;y12:syntax-rules;;;l2:y1:_;y"
  "28:%25residual-current-error-port;;",

  0,
  "&0{%0P12]0}@!(y28:%25residual-current-error-port)",

  "open-output-string",
  "l5:y12:syntax-rules;n;l2:l1:y1:_;;l1:y4:%25oos;;;l2:py1:_;y12:syntax-r"
  "ules;;py28:%25residual-open-output-string;y12:syntax-rules;;;l2:y1:_;y"
  "28:%25residual-open-output-string;;",

  0,
  "&0{%0P51]0}@!(y28:%25residual-open-output-string)",

  "open-input-file",
  "l5:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l2:y5:%25otip;y1:x;;;l2:py1:_;y"
  "12:syntax-rules;;py25:%25residual-open-input-file;y12:syntax-rules;;;l"
  "2:y1:_;y25:%25residual-open-input-file;;",

  0,
  "&0{%1.0P40]1}@!(y25:%25residual-open-input-file)",

  "open-output-file",
  "l5:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l2:y5:%25otop;y1:x;;;l2:py1:_;y"
  "12:syntax-rules;;py26:%25residual-open-output-file;y12:syntax-rules;;;"
  "l2:y1:_;y26:%25residual-open-output-file;;",

  0,
  "&0{%1.0P41]1}@!(y26:%25residual-open-output-file)",

  "open-input-string",
  "l5:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l2:y4:%25ois;y1:x;;;l2:py1:_;y1"
  "2:syntax-rules;;py27:%25residual-open-input-string;y12:syntax-rules;;;"
  "l2:y1:_;y27:%25residual-open-input-string;;",

  0,
  "&0{%1.0P50]1}@!(y27:%25residual-open-input-string)",

  "close-input-port",
  "l5:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l2:y4:%25cip;y1:x;;;l2:py1:_;y1"
  "2:syntax-rules;;py26:%25residual-close-input-port;y12:syntax-rules;;;l"
  "2:y1:_;y26:%25residual-close-input-port;;",

  0,
  "&0{%1.0P60]1}@!(y26:%25residual-close-input-port)",

  "close-output-port",
  "l5:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l2:y4:%25cop;y1:x;;;l2:py1:_;y1"
  "2:syntax-rules;;py27:%25residual-close-output-port;y12:syntax-rules;;;"
  "l2:y1:_;y27:%25residual-close-output-port;;",

  0,
  "&0{%1.0P61]1}@!(y27:%25residual-close-output-port)",

  "get-output-string",
  "l5:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l2:y4:%25gos;y1:x;;;l2:py1:_;y1"
  "2:syntax-rules;;py27:%25residual-get-output-string;y12:syntax-rules;;;"
  "l2:y1:_;y27:%25residual-get-output-string;;",

  0,
  "&0{%1.0P9]1}@!(y27:%25residual-get-output-string)",

  0,
  "&0{%1.0P00,.0?{.0]2}.1P01]2}@!(y5:port?)",

  0,
  "&0{%1.0P00?{.0P60}.0P01?{.0P61]1}]1}@!(y10:close-port)",

  0,
  "&0{%2.0,&1{%!0${:0,@(y10:close-port)[01}.0,@(y5:%25sdmv),@(y5:%25appl)"
  "[12},.1,.3,&2{%0:1,:0[01},@(y5:%25cwmv)[22}@!(y14:call-with-port)",

  0,
  "&0{%2.1,.1P40,@(y14:call-with-port)[22}@!(y20:call-with-input-file)",

  0,
  "&0{%2.1,.1P41,@(y14:call-with-port)[22}@!(y21:call-with-output-file)",

  "read-char",
  "l6:y12:syntax-rules;n;l2:l1:y1:_;;l2:y4:%25rdc;l1:y4:%25sip;;;;l2:l2:y"
  "1:_;y1:p;;l2:y4:%25rdc;y1:p;;;l2:py1:_;y4:args;;py19:%25residual-read-"
  "char;y4:args;;;l2:y1:_;y19:%25residual-read-char;;",

  "peek-char",
  "l6:y12:syntax-rules;n;l2:l1:y1:_;;l2:y5:%25rdac;l1:y4:%25sip;;;;l2:l2:"
  "y1:_;y1:p;;l2:y5:%25rdac;y1:p;;;l2:py1:_;y4:args;;py19:%25residual-pee"
  "k-char;y4:args;;;l2:y1:_;y19:%25residual-peek-char;;",

  "char-ready?",
  "l5:y12:syntax-rules;n;l2:l1:y1:_;;l2:y5:%25rdcr;l1:y4:%25sip;;;;l2:l2:"
  "y1:_;y1:p;;l2:y5:%25rdcr;y1:p;;;l2:y1:_;y21:%25residual-char-ready?;;",

  0,
  "&0{%1P51,t,,#0.3,.3,.2,&3{%1:2R0,.0R8,.0?{.0}{'(c%0a),.2C=}_1?{.0R8?{."
  "1}{f}?{.0]2}:1P9,:1P61.0]3}'(c%0d),.1C=?{f,:0^[21}${:1,.3,@(y4:%25wrc)"
  "[02}f,:0^[21}.!0.0^_1[21}@!(y10:%25read-line)",

  "read-line",
  "l5:y12:syntax-rules;n;l2:l1:y1:_;;l2:y10:%25read-line;l1:y4:%25sip;;;;"
  "l2:l2:y1:_;y1:p;;l2:y10:%25read-line;y1:p;;;l2:y1:_;y19:%25residual-re"
  "ad-line;;",

  "eof-object?",
  "l5:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l2:y5:%25eofp;y1:x;;;l2:py1:_;y"
  "12:syntax-rules;;py21:%25residual-eof-object?;y12:syntax-rules;;;l2:y1"
  ":_;y21:%25residual-eof-object?;;",

  0,
  "&0{%1.0R8]1}@!(y21:%25residual-eof-object?)",

  "eof-object",
  "l5:y12:syntax-rules;n;l2:l1:y1:_;;l1:y4:%25eof;;;l2:py1:_;y12:syntax-r"
  "ules;;py20:%25residual-eof-object;y12:syntax-rules;;;l2:y1:_;y20:%25re"
  "sidual-eof-object;;",

  0,
  "&0{%0R9]0}@!(y20:%25residual-eof-object)",

  0,
  "&0{%!0.0]1}@!(y14:%25residual-list)",

  0,
  "&0{%!1.0u?{f,.2,@(y10:%25make-list)[22}.0a,.2,@(y10:%25make-list)[22}@"
  "!(y19:%25residual-make-list)",

  0,
  "&0{%!1.0u?{f,.2V2]2}.0a,.2V2]2}@!(y21:%25residual-make-vector)",

  0,
  "&0{%!1.0u?{'(c ),.2S2]2}.0a,.2S2]2}@!(y21:%25residual-make-string)",

  "minmax-reducer",
  "l3:y12:syntax-rules;n;l2:l2:y1:_;y1:f;;l3:y6:lambda;py1:x;y4:args;;l4:"
  "y3:let;y4:loop;l2:l2:y1:x;y1:x;;l2:y4:args;y4:args;;;l4:y2:if;l2:y5:nu"
  "ll?;y4:args;;y1:x;l3:y4:loop;l3:y1:f;y1:x;l2:y3:car;y4:args;;;l2:y3:cd"
  "r;y4:args;;;;;;;",

  0,
  "&0{%!2.0u?{.2%l,.2A2]3}.0a,.3,.3,@(y7:%25member)[33}@!(y16:%25residual"
  "-member)",

  0,
  "&0{%!2.0u?{.2%l,.2A5]3}.0a,.3,.3,@(y6:%25assoc)[33}@!(y15:%25residual-"
  "assoc)",

  0,
  "&0{%!1.0,.2,,#0.0,&1{%2.1u?{.0]2}${.3d,.4a,:0^[02},.1c]2}.!0.0^_1[22}@"
  "!(y15:%25residual-list*)",

  0,
  "&0{%!2${.2,.5,,#0.0,&1{%2.1u?{.0]2}${.3d,.4a,:0^[02},.1c]2}.!0.0^_1[02"
  "},.2,@(y5:%25appl)[32}@!(y15:%25residual-apply)",

  0,
  "&0{%!2.0u?{n,.3,,#0.0,.5,&2{%2.0p?{.1,${.3a,:0[01}c,.1d,:1^[22}.1A9]2}"
  ".!0.0^_1[32}n,.1,.4c,,#0.0,.5,&2{%2${.2,,#0.0,&1{%1.0u,.0?{.0]2}.1ap?{"
  ".1d,:0^[21}f]2}.!0.0^_1[01}?{.1,${@(y3:car),${.6,,#0.4,.1,&2{%1.0p?{${"
  ".2d,:0^[01},${.3a,:1[01}c]1}n]1}.!0.0^_1[01}_1,:0,@(y5:%25appl)[02}c,@"
  "(y3:cdr),${.4,,#0.4,.1,&2{%1.0p?{${.2d,:0^[01},${.3a,:1[01}c]1}n]1}.!0"
  ".0^_1[01}_1,:1^[22}.1A9]2}.!0.0^_1[32}@!(y13:%25residual-map)",

  0,
  "&0{%!2.0u?{.2,,#0.3,.1,&2{%1.0p?{${.2a,:1[01}.0d,:0^[11}]1}.!0.0^_1[31"
  "}.0,.3c,,#0.3,.1,&2{%1${.2,,#0.0,&1{%1.0u,.0?{.0]2}.1ap?{.1d,:0^[21}f]"
  "2}.!0.0^_1[01}?{${@(y3:car),${.5,,#0.4,.1,&2{%1.0p?{${.2d,:0^[01},${.3"
  "a,:1[01}c]1}n]1}.!0.0^_1[01}_1,:1,@(y5:%25appl)[02}@(y3:cdr),${.3,,#0."
  "4,.1,&2{%1.0p?{${.2d,:0^[01},${.3a,:1[01}c]1}n]1}.!0.0^_1[01}_1,:0^[11"
  "}]1}.!0.0^_1[31}@!(y18:%25residual-for-each)",

  0,
  "&0{%!0.0,,#0.0,&1{%1.0u?{n]1}.0du?{.0a]1}${.2d,:0^[01},.1aL6]1}.!0.0^_"
  "1[11}@!(y16:%25residual-append)",

  0,
  "&0{%!0.0,,#0.0,&1{%1.0u?{'(s0:)]1}.0du?{.0a]1}${.2d,:0^[01},.1aS6]1}.!"
  "0.0^_1[11}@!(y23:%25residual-string-append)",

  0,
  "&0{%!0.0,,#0.0,&1{%1.0u?{'(v0:)]1}.0du?{.0a]1}${.2d,:0^[01},.1aV6]1}.!"
  "0.0^_1[11}@!(y23:%25residual-vector-append)",

  0,
  "&0{%!0.0u?{P10R0]1}.0aR0]1}@!(y19:%25residual-read-char)",

  0,
  "&0{%!0.0u?{P10R1]1}.0aR1]1}@!(y19:%25residual-peek-char)",

  0,
  "&0{%!0.0u?{P10R2]1}.0aR2]1}@!(y21:%25residual-char-ready?)",

  0,
  "&0{%!0.0u?{P10,@(y10:%25read-line)[11}.0a,@(y10:%25read-line)[11}@!(y1"
  "9:%25residual-read-line)",

  0, 0
};
