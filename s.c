/* s.c -- generated via skint -c s.scm */

char *s_code[] = {

  "S", "let-syntax",
  "l4:y12:syntax-rules;n;l2:l2:y1:_;l2:l2:y2:kw;y4:init;;y3:...;;;l1:y5:b"
  "egin;;;l2:py1:_;pl2:l2:y2:kw;y4:init;;y3:...;;y5:forms;;;l3:py13:synta"
  "x-lambda;pl2:y2:kw;y3:...;;y5:forms;;;y4:init;y3:...;;;",

  "S", "letrec-syntax",
  "l3:y12:syntax-rules;n;l2:py1:_;pl2:l2:y3:key;y5:trans;;y3:...;;y5:form"
  "s;;;py4:body;pl3:y13:define-syntax;y3:key;y5:trans;;py3:...;y5:forms;;"
  ";;;",

  "S", "letrec",
  "l3:y12:syntax-rules;n;l2:py1:_;pl2:l2:y3:var;y4:init;;y3:...;;y5:forms"
  ";;;py4:body;pl3:y6:define;y3:var;y4:init;;py3:...;y5:forms;;;;;",

  "S", "letrec*",
  "l3:y12:syntax-rules;n;l2:py1:_;pl2:l2:y3:var;y4:expr;;y3:...;;y5:forms"
  ";;;l5:y3:let;l2:l2:y3:var;f;;y3:...;;l3:y4:set!;y3:var;y4:expr;;y3:..."
  ";py4:body;y5:forms;;;;",

  "S", "let",
  "l4:y12:syntax-rules;n;l2:py1:_;pl2:l2:y3:var;y4:init;;y3:...;;y5:forms"
  ";;;l3:py6:lambda;pl2:y3:var;y3:...;;y5:forms;;;y4:init;y3:...;;;l2:py1"
  ":_;py4:name;pl2:l2:y3:var;y4:init;;y3:...;;y5:forms;;;;l3:l3:y6:letrec"
  ";l1:l2:y4:name;py6:lambda;pl2:y3:var;y3:...;;y5:forms;;;;;y4:name;;y4:"
  "init;y3:...;;;",

  "S", "let*",
  "l4:y12:syntax-rules;n;l2:py1:_;pn;y5:forms;;;py4:body;y5:forms;;;l2:py"
  "1:_;ppy5:first;y4:more;;y5:forms;;;l3:y3:let;l1:y5:first;;py4:let*;py4"
  ":more;y5:forms;;;;;",

  "S", "let*-values",
  "l5:y12:syntax-rules;n;l2:py1:_;pn;y5:forms;;;py4:body;y5:forms;;;l2:py"
  "1:_;ppl2:l1:y1:a;;y1:x;;y2:b*;;y5:forms;;;l3:y3:let;l1:l2:y1:a;y1:x;;;"
  "py11:let*-values;py2:b*;y5:forms;;;;;l2:py1:_;ppl2:y2:aa;y1:x;;y2:b*;;"
  "y5:forms;;;l3:y16:call-with-values;l3:y6:lambda;n;y1:x;;l3:y6:lambda;y"
  "2:aa;py11:let*-values;py2:b*;y5:forms;;;;;;",

  "S", "%let-values-loop",
  "l6:y12:syntax-rules;n;l2:py1:_;pl2:y5:new-b;y3:...;;py6:new-aa;py1:x;p"
  "y6:map-b*;pn;pn;y5:forms;;;;;;;;l3:y11:let*-values;l3:y5:new-b;y3:...;"
  "l2:y6:new-aa;y1:x;;;py3:let;py6:map-b*;y5:forms;;;;;l2:py1:_;pl2:y5:ne"
  "w-b;y3:...;;py6:new-aa;py5:old-x;py6:map-b*;pn;ppl2:y2:aa;y1:x;;y2:b*;"
  ";y5:forms;;;;;;;;py16:%25let-values-loop;pl3:y5:new-b;y3:...;l2:y6:new"
  "-aa;y5:old-x;;;pn;py1:x;py6:map-b*;py2:aa;py2:b*;y5:forms;;;;;;;;;l2:p"
  "y1:_;py6:new-b*;pl2:y5:new-a;y3:...;;py1:x;pl2:y5:map-b;y3:...;;ppy1:a"
  ";y2:aa;;py2:b*;y5:forms;;;;;;;;py16:%25let-values-loop;py6:new-b*;pl3:"
  "y5:new-a;y3:...;y5:tmp-a;;py1:x;pl3:y5:map-b;y3:...;l2:y1:a;y5:tmp-a;;"
  ";py2:aa;py2:b*;y5:forms;;;;;;;;;l2:py1:_;py6:new-b*;pl2:y5:new-a;y3:.."
  ".;;py1:x;pl2:y5:map-b;y3:...;;py1:a;py2:b*;y5:forms;;;;;;;;py16:%25let"
  "-values-loop;py6:new-b*;ppy5:new-a;py3:...;y5:tmp-a;;;py1:x;pl3:y5:map"
  "-b;y3:...;l2:y1:a;y5:tmp-a;;;pn;py2:b*;y5:forms;;;;;;;;;",

  "S", "let-values",
  "l4:y12:syntax-rules;n;l2:py1:_;pn;y5:forms;;;py3:let;pn;y5:forms;;;;l2"
  ":py1:_;ppl2:y2:aa;y1:x;;y2:b*;;y5:forms;;;py16:%25let-values-loop;pn;p"
  "n;py1:x;pn;py2:aa;py2:b*;y5:forms;;;;;;;;;",

  "S", "%define-values-loop",
  "l5:y12:syntax-rules;n;l2:l5:y1:_;y6:new-aa;l2:l2:y1:a;y5:tmp-a;;y3:..."
  ";;n;y1:x;;l4:y5:begin;l3:y6:define;y1:a;l1:y5:begin;;;y3:...;l3:y6:def"
  "ine;n;l3:y16:call-with-values;l3:y6:lambda;n;y1:x;;l4:y6:lambda;y6:new"
  "-aa;l3:y4:set!;y1:a;y5:tmp-a;;y3:...;;;;;;l2:l5:y1:_;l2:y5:new-a;y3:.."
  ".;;l2:y5:map-a;y3:...;;py1:a;y2:aa;;y1:x;;l5:y19:%25define-values-loop"
  ";l3:y5:new-a;y3:...;y5:tmp-a;;l3:y5:map-a;y3:...;l2:y1:a;y5:tmp-a;;;y2"
  ":aa;y1:x;;;l2:l5:y1:_;l2:y5:new-a;y3:...;;l2:y5:map-a;y3:...;;y1:a;y1:"
  "x;;l5:y19:%25define-values-loop;py5:new-a;py3:...;y5:tmp-a;;;l3:y5:map"
  "-a;y3:...;l2:y1:a;y5:tmp-a;;;n;y1:x;;;",

  "S", "define-values",
  "l4:y12:syntax-rules;n;l2:l3:y1:_;n;y1:x;;l3:y6:define;n;l3:y16:call-wi"
  "th-values;l3:y6:lambda;n;y1:x;;l2:y6:lambda;n;;;;;l2:l3:y1:_;y2:aa;y1:"
  "x;;l5:y19:%25define-values-loop;n;n;y2:aa;y1:x;;;",

  "S", "and",
  "l5:y12:syntax-rules;n;l2:l1:y1:_;;t;;l2:l2:y1:_;y4:test;;y4:test;;l2:p"
  "y1:_;py4:test;y5:tests;;;l4:y2:if;y4:test;py3:and;y5:tests;;f;;;",

  "S", "or",
  "l5:y12:syntax-rules;n;l2:l1:y1:_;;f;;l2:l2:y1:_;y4:test;;y4:test;;l2:p"
  "y1:_;py4:test;y5:tests;;;l3:y3:let;l1:l2:y1:x;y4:test;;;l4:y2:if;y1:x;"
  "y1:x;py2:or;y5:tests;;;;;",

  "S", "cond",
  "l7:y12:syntax-rules;l2:y4:else;y2:=>;;l2:l1:y1:_;;f;;l2:l2:y1:_;py4:el"
  "se;y4:exps;;;py5:begin;y4:exps;;;l2:py1:_;pl1:y1:x;;y4:rest;;;l3:y2:or"
  ";y1:x;py4:cond;y4:rest;;;;l2:py1:_;pl3:y1:x;y2:=>;y4:proc;;y4:rest;;;l"
  "3:y3:let;l1:l2:y3:tmp;y1:x;;;py4:cond;pl2:y3:tmp;l2:y4:proc;y3:tmp;;;y"
  "4:rest;;;;;l2:py1:_;ppy1:x;y4:exps;;y4:rest;;;l4:y2:if;y1:x;py5:begin;"
  "y4:exps;;py4:cond;y4:rest;;;;",

  "S", "%case-test",
  "l5:y12:syntax-rules;n;l2:l3:y1:_;y1:k;n;;f;;l2:l3:y1:_;y1:k;l1:y5:datu"
  "m;;;l3:y4:eqv?;y1:k;l2:y5:quote;y5:datum;;;;l2:l3:y1:_;y1:k;y4:data;;l"
  "3:y4:memv;y1:k;l2:y5:quote;y4:data;;;;",

  "S", "%case",
  "l7:y12:syntax-rules;l2:y4:else;y2:=>;;l2:l2:y1:_;y3:key;;l1:y5:begin;;"
  ";l2:l3:y1:_;y3:key;l3:y4:else;y2:=>;y7:resproc;;;l2:y7:resproc;y3:key;"
  ";;l2:l3:y1:_;y3:key;l3:y4:else;y4:expr;y3:...;;;l3:y5:begin;y4:expr;y3"
  ":...;;;l2:py1:_;py3:key;pl3:l2:y5:datum;y3:...;;y2:=>;y7:resproc;;y7:c"
  "lauses;;;;l4:y2:if;l3:y10:%25case-test;y3:key;l2:y5:datum;y3:...;;;l2:"
  "y7:resproc;y3:key;;py5:%25case;py3:key;y7:clauses;;;;;l2:py1:_;py3:key"
  ";pl3:l2:y5:datum;y3:...;;y4:expr;y3:...;;y7:clauses;;;;l4:y2:if;l3:y10"
  ":%25case-test;y3:key;l2:y5:datum;y3:...;;;l3:y5:begin;y4:expr;y3:...;;"
  "py5:%25case;py3:key;y7:clauses;;;;;",

  "S", "case",
  "l3:y12:syntax-rules;n;l2:py1:_;py1:x;y7:clauses;;;l3:y3:let;l1:l2:y3:k"
  "ey;y1:x;;;py5:%25case;py3:key;y7:clauses;;;;;",

  "S", "%do-step",
  "l4:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;y1:x;;l2:l3:y1:_;y1:x;y1:y;;y1:"
  "y;;",

  "S", "do",
  "l3:y12:syntax-rules;n;l2:l5:y1:_;l2:l4:y3:var;y4:init;y4:step;y3:...;;"
  "y3:...;;l3:y4:test;y4:expr;y3:...;;y7:command;y3:...;;l4:y3:let;y4:loo"
  "p;l2:l2:y3:var;y4:init;;y3:...;;l4:y2:if;y4:test;l3:y5:begin;y4:expr;y"
  "3:...;;l5:y3:let;n;y7:command;y3:...;l3:y4:loop;l4:y8:%25do-step;y3:va"
  "r;y4:step;y3:...;;y3:...;;;;;;",

  "S", "quasiquote",
  "l10:y12:syntax-rules;l3:y7:unquote;y16:unquote-splicing;y10:quasiquote"
  ";;l2:l2:y1:_;l2:y7:unquote;y1:x;;;y1:x;;l2:l2:y1:_;pl2:y16:unquote-spl"
  "icing;y1:x;;y1:y;;;l3:y6:append;y1:x;l2:y10:quasiquote;y1:y;;;;l2:py1:"
  "_;pl2:y10:quasiquote;y1:x;;y1:d;;;l3:y4:cons;l2:y5:quote;y10:quasiquot"
  "e;;l3:y10:quasiquote;l1:y1:x;;y1:d;;;;l2:l3:y1:_;l2:y7:unquote;y1:x;;y"
  "1:d;;l3:y4:cons;l2:y5:quote;y7:unquote;;py10:quasiquote;pl1:y1:x;;y1:d"
  ";;;;;l2:l3:y1:_;l2:y16:unquote-splicing;y1:x;;y1:d;;l3:y4:cons;l2:y5:q"
  "uote;y16:unquote-splicing;;py10:quasiquote;pl1:y1:x;;y1:d;;;;;l2:py1:_"
  ";ppy1:x;y1:y;;y1:d;;;l3:y4:cons;py10:quasiquote;py1:x;y1:d;;;py10:quas"
  "iquote;py1:y;y1:d;;;;;l2:py1:_;pv2:y1:x;y3:...;;y1:d;;;l2:y12:list->ve"
  "ctor;py10:quasiquote;pl2:y1:x;y3:...;;y1:d;;;;;l2:py1:_;py1:x;y1:d;;;l"
  "2:y5:quote;y1:x;;;",

  "S", "when",
  "l3:y12:syntax-rules;n;l2:py1:_;py4:test;y4:rest;;;l3:y2:if;y4:test;py5"
  ":begin;y4:rest;;;;",

  "S", "unless",
  "l3:y12:syntax-rules;n;l2:py1:_;py4:test;y4:rest;;;l3:y2:if;l2:y3:not;y"
  "4:test;;py5:begin;y4:rest;;;;",

  "S", "case-lambda",
  "l3:y12:syntax-rules;n;l2:l3:y1:_;py4:args;y5:forms;;y3:...;;l3:y7:lamb"
  "da*;l2:y4:args;py6:lambda;py4:args;y5:forms;;;;y3:...;;;",

  "P", "floor/",
  "%2.1,.1Nm,.2,.2Nl,@(y6:values)[22",

  "P", "truncate/",
  "%2.1,.1Nr,.2,.2Nq,@(y6:values)[22",

  "P", "lcm",
  "%!0.0u?{'1]1}.0d,.1a,,#0.0,&1{%2.1u?{.0]2}.1a,.0,.2Ng,.3d,.1=0?{.1}{.2"
  "Na,.2,.5NaNq*},:0^[42}.!0.0^_1[12",

  "P", "numerator",
  "%1.0]1",

  "P", "denominator",
  "%1'1]1",

  "P", "rationalize",
  "%2.0]2",

  "P", "square",
  "%1.0,.1*]1",

  "P", "exact-integer-sqrt",
  "%1.0It,.0,.1*,.2-,.1,@(y6:values)[22",

  "P", "make-rectangular",
  "%2'0,.2=?{.0]2}.1,'(s49:make-rectangular: nonzero imag part not suppor"
  "ted),@(y5:error)[22",

  "P", "make-polar",
  "%2'0,.2=?{.0]2}'(j3.14159265358979),.2=?{.0-!]2}.1,'(s31:make-polar: a"
  "ngle not supported),@(y5:error)[22",

  "P", "real-part",
  "%1.0]1",

  "P", "imag-part",
  "%1'0]1",

  "P", "magnitude",
  "%1.0Na]1",

  "P", "angle",
  "%1.0<0?{'(j3.14159265358979)]1}'0]1",

  "P", "%append",
  "%!0.0,,#0.0,&1{%1.0u?{n]1}.0du?{.0a]1}${.2d,:0^[01},.1aL6]1}.!0.0^_1[1"
  "1",

  "S", "append",
  "l7:y12:syntax-rules;n;l2:l1:y1:_;;l2:y5:quote;n;;;l2:l2:y1:_;y1:x;;y1:"
  "x;;l2:l3:y1:_;y1:x;y1:y;;l3:y8:list-cat;y1:x;y1:y;;;l2:l5:y1:_;y1:x;y1"
  ":y;y1:z;y3:...;;l3:y8:list-cat;y1:x;l4:y6:append;y1:y;y1:z;y3:...;;;;l"
  "2:y1:_;y7:%25append;;",

  "P", "%member",
  "%!2.0u?{.2,.2A2]3}.0a,.3,.3,,#0.0,&1{%3.1p?{${.3a,.3,.6[02}?{.1]3}.2,."
  "2d,.2,:0^[33}f]3}.!0.0^_1[33",

  "S", "member",
  "l5:y12:syntax-rules;n;l2:l3:y1:_;y1:v;y1:l;;l3:y4:meme;y1:v;y1:l;;;l2:"
  "py1:_;y4:args;;py7:%25member;y4:args;;;l2:y1:_;y7:%25member;;",

  "P", "%assoc",
  "%!2.0u?{.2,.2A5]3}.0a,.3,.3,,#0.0,&1{%3.1p?{${.3aa,.3,.6[02}?{.1a]3}.2"
  ",.2d,.2,:0^[33}f]3}.!0.0^_1[33",

  "S", "assoc",
  "l5:y12:syntax-rules;n;l2:l3:y1:_;y1:v;y2:al;;l3:y4:asse;y1:v;y2:al;;;l"
  "2:py1:_;y4:args;;py6:%25assoc;y4:args;;;l2:y1:_;y6:%25assoc;;",

  "P", "list-copy",
  "%1.0,,#0.0,&1{%1.0p?{${.2d,:0^[01},.1ac]1}.0]1}.!0.0^_1[11",

  "P", "%list*",
  "%!1.0,.2,,#0.0,&1{%2.1u?{.0]2}${.3d,.4a,:0^[02},.1c]2}.!0.0^_1[22",

  "S", "list*",
  "l7:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;y1:x;;l2:l3:y1:_;y1:x;y1:y;;l3:"
  "y4:cons;y1:x;y1:y;;;l2:l5:y1:_;y1:x;y1:y;y1:z;y3:...;;l3:y4:cons;y1:x;"
  "l4:y5:list*;y1:y;y1:z;y3:...;;;;l2:py1:_;y4:args;;py6:%25list*;y4:args"
  ";;;l2:y1:_;y6:%25list*;;",

  "A", "cons*", "list*",

  "P", "substring->list",
  "%3n,'1,.4I-,,#0.3,.1,.6,&3{%2:0,.1I<?{.1]2}.1,.1,:2S4c,'1,.2I-,:1^[22}"
  ".!0.0^_1[32",

  "C", 0,
  "&0{%3.2,.2,.2,@(y15:substring->list)[33}%x,&0{%2.0S3,.2,.2,@(y15:subst"
  "ring->list)[23}%x,&0{%1.0X2]1}%x,&3{|10|21|32%%}@!(y13:%25string->list"
  ")",

  "S", "string->list",
  "l5:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l2:y14:%25string->list1;y1:x;;;"
  "l2:py1:_;y1:r;;py13:%25string->list;y1:r;;;l2:y1:_;y13:%25string->list"
  ";;",

  "P", "substring-copy!",
  "%5.1,.1S3I-,.4I+,.5In,.4,.3I>!?{.4,.3,,#0.0,.5,.8,.6,&4{%2:0,.2I<!?{]2"
  "}.1,:1S4,.1,:2S5'1,.2I+,'1,.2I+,:3^[22}.!0.0^_1[62}'1,.1I-,'1,.6,.8I-I"
  "-,.4I+,,#0.0,.5,.8,.(i10),&4{%2:0,.2I<?{]2}.1,:1S4,.1,:2S5'1,.2I-,'1,."
  "2I-,:3^[22}.!0.0^_1[62",

  "C", 0,
  "&0{%5.4,.4,.4,.4,.4,@(y15:substring-copy!)[55}%x,&0{%4.2S3,.4,.4,.4,.4"
  ",@(y15:substring-copy!)[45}%x,&0{%3.2S3,'0,.4,.4,.4,@(y15:substring-co"
  "py!)[35}%x,&3{|30|41|52%%}@!(y12:string-copy!)",

  "C", 0,
  "&0{%3.2,.2,.2S7]3}%x,&0{%2.0S3,.2,.2S7]2}%x,&0{%1.0S3,'0,.2S7]1}%x,&3{"
  "|10|21|32%%}@!(y11:string-copy)",

  "P", "substring-fill!",
  "%4.2,,#0.0,.3,.5,.8,&4{%1:0,.1I<!?{]1}:1,.1,:2S5'1,.1I+,:3^[11}.!0.0^_"
  "1[41",

  "C", 0,
  "&0{%4.3,.3,.3,.3,@(y15:substring-fill!)[44}%x,&0{%3.0S3,.3,.3,.3,@(y15"
  ":substring-fill!)[34}%x,&0{%2.0S3,'0,.3,.3,@(y15:substring-fill!)[24}%"
  "x,&3{|20|31|42%%}@!(y12:string-fill!)",

  "P", "substring-vector-copy!",
  "%5.1,.1V3I-,.4I+,.5In,.4,.3,,#0.0,.5,.8,.6,&4{%2:0,.2I<!?{:2]2}.1,:1S4"
  ",.1,:2V5'1,.2I+,'1,.2I+,:3^[22}.!0.0^_1[62",

  "P", "substring->vector",
  "%3.2,.2,.2,'0,f,.6,.8I-V2,@(y22:substring-vector-copy!)[35",

  "C", 0,
  "&0{%3.2,.2,.2,@(y17:substring->vector)[33}%x,&0{%2.0S3,.2,.2,@(y17:sub"
  "string->vector)[23}%x,&0{%1.0S3,'0,.2,@(y17:substring->vector)[13}%x,&"
  "3{|10|21|32%%}@!(y14:string->vector)",

  "P", "strings-sum-length",
  "%1'0,.1,,#0.0,&1{%2.0u?{.1]2}.0aS3,.2I+,.1d,:0^[22}.!0.0^_1[12",

  "P", "strings-copy-into!",
  "%2'0,.2,,#0.0,.4,&2{%2.0u?{:0]2}.0d,.1a,.0S3,${.2,'0,.5,.9,:0,@(y15:su"
  "bstring-copy!)[05}.0,.5I+,.3,:1^[52}.!0.0^_1[22",

  "P", "%string-append",
  "%!0.0,'(c ),${.4,@(y18:strings-sum-length)[01}S2,@(y18:strings-copy-in"
  "to!)[12",

  "S", "string-append",
  "l7:y12:syntax-rules;n;l2:l1:y1:_;;s0:;;l2:l2:y1:_;y1:x;;l2:y4:%25cks;y"
  "1:x;;;l2:l3:y1:_;y1:x;y1:y;;l3:y10:string-cat;y1:x;y1:y;;;l2:py1:_;y1:"
  "r;;py14:%25string-append;y1:r;;;l2:y1:_;y14:%25string-append;;",

  "P", "subvector->list",
  "%3n,'1,.4I-,,#0.3,.1,.6,&3{%2:0,.1I<?{.1]2}.1,.1,:2V4c,'1,.2I-,:1^[22}"
  ".!0.0^_1[32",

  "C", 0,
  "&0{%3.2,.2,.2,@(y15:subvector->list)[33}%x,&0{%2.0V3,.2,.2,@(y15:subve"
  "ctor->list)[23}%x,&0{%1.0X0]1}%x,&3{|10|21|32%%}@!(y13:%25vector->list"
  ")",

  "S", "vector->list",
  "l5:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l2:y14:%25vector->list1;y1:x;;;"
  "l2:py1:_;y1:r;;py13:%25vector->list;y1:r;;;l2:y1:_;y13:%25vector->list"
  ";;",

  "P", "subvector-copy!",
  "%5.1,.1V3I-,.4I+,.5In,.4,.3I>!?{.4,.3,,#0.0,.5,.8,.6,&4{%2:0,.2I<!?{]2"
  "}.1,:1V4,.1,:2V5'1,.2I+,'1,.2I+,:3^[22}.!0.0^_1[62}'1,.1I-,'1,.6,.8I-I"
  "-,.4I+,,#0.0,.5,.8,.(i10),&4{%2:0,.2I<?{]2}.1,:1V4,.1,:2V5'1,.2I-,'1,."
  "2I-,:3^[22}.!0.0^_1[62",

  "C", 0,
  "&0{%5.4,.4,.4,.4,.4,@(y15:subvector-copy!)[55}%x,&0{%4.2V3,.4,.4,.4,.4"
  ",@(y15:subvector-copy!)[45}%x,&0{%3.2V3,'0,.4,.4,.4,@(y15:subvector-co"
  "py!)[35}%x,&3{|30|41|52%%}@!(y12:vector-copy!)",

  "P", "subvector",
  "%3f,.2,.4I-V2,${.5,.5,.5,'0,.6,@(y15:subvector-copy!)[05}.0]4",

  "C", 0,
  "&0{%3.2,.2,.2,@(y9:subvector)[33}%x,&0{%2.0V3,.2,.2,@(y9:subvector)[23"
  "}%x,&0{%1.0V3,'0,.2,@(y9:subvector)[13}%x,&3{|10|21|32%%}@!(y11:vector"
  "-copy)",

  "P", "subvector-fill!",
  "%4.2,,#0.0,.3,.5,.8,&4{%1:0,.1I<!?{]1}:1,.1,:2V5'1,.1I+,:3^[11}.!0.0^_"
  "1[41",

  "C", 0,
  "&0{%4.3,.3,.3,.3,@(y15:subvector-fill!)[44}%x,&0{%3.0V3,.3,.3,.3,@(y15"
  ":subvector-fill!)[34}%x,&0{%2.0V3,'0,.3,.3,@(y15:subvector-fill!)[24}%"
  "x,&3{|20|31|42%%}@!(y12:vector-fill!)",

  "P", "subvector-string-copy!",
  "%5.1,.1S3I-,.4I+,.5In,.4,.3,,#0.0,.5,.8,.6,&4{%2:0,.2I<!?{:2]2}.1,:1V4"
  ",.1,:2S5'1,.2I+,'1,.2I+,:3^[22}.!0.0^_1[62",

  "P", "subvector->string",
  "%3.2,.2,.2,'0,'(c ),.6,.8I-S2,@(y22:subvector-string-copy!)[35",

  "C", 0,
  "&0{%3.2,.2,.2,@(y17:subvector->string)[33}%x,&0{%2.0V3,.2,.2,@(y17:sub"
  "vector->string)[23}%x,&0{%1.0V3,'0,.2,@(y17:subvector->string)[13}%x,&"
  "3{|10|21|32%%}@!(y14:vector->string)",

  "P", "vectors-sum-length",
  "%1'0,.1,,#0.0,&1{%2.0u?{.1]2}.0aV3,.2I+,.1d,:0^[22}.!0.0^_1[12",

  "P", "vectors-copy-into!",
  "%2'0,.2,,#0.0,.4,&2{%2.0u?{:0]2}.0d,.1a,.0V3,${.2,'0,.5,.9,:0,@(y15:su"
  "bvector-copy!)[05}.0,.5I+,.3,:1^[52}.!0.0^_1[22",

  "P", "%vector-append",
  "%!0.0,f,${.4,@(y18:vectors-sum-length)[01}V2,@(y18:vectors-copy-into!)"
  "[12",

  "S", "vector-append",
  "l7:y12:syntax-rules;n;l2:l1:y1:_;;l2:y5:quote;v0:;;;l2:l2:y1:_;y1:x;;l"
  "2:y4:%25ckv;y1:x;;;l2:l3:y1:_;y1:x;y1:y;;l3:y10:vector-cat;y1:x;y1:y;;"
  ";l2:py1:_;y1:r;;py14:%25vector-append;y1:r;;;l2:y1:_;y14:%25vector-app"
  "end;;",

  "P", "%apply",
  "%!2${.2,.5,,#0.0,&1{%2.1u?{.0]2}${.3d,.4a,:0^[02},.1c]2}.!0.0^_1[02},."
  "2,@(y13:apply-to-list)[32",

  "S", "apply",
  "l6:y12:syntax-rules;n;l2:l3:y1:_;y1:p;y1:l;;l3:y13:apply-to-list;y1:p;"
  "y1:l;;;l2:l6:y1:_;y1:p;y1:a;y1:b;y3:...;y1:l;;l3:y13:apply-to-list;y1:"
  "p;l5:y5:list*;y1:a;y1:b;y3:...;y1:l;;;;l2:py1:_;y4:args;;py6:%25apply;"
  "y4:args;;;l2:y1:_;y6:%25apply;;",

  "P", "%call/cc",
  "%1k1,.0,.2[21",

  "S", "call/cc",
  "l5:y12:syntax-rules;n;l2:l2:y1:_;y1:p;;l3:y5:letcc;y1:k;l2:y1:p;y1:k;;"
  ";;l2:py1:_;y4:args;;py8:%25call/cc;y4:args;;;l2:y1:_;y8:%25call/cc;;",

  "A", "call-with-current-continuation", "call/cc",

  "P", "%map1",
  "%2n,.2,,#0.0,.4,&2{%2.0p?{.1,${.3a,:0[01}c,.1d,:1^[22}.1A9]2}.!0.0^_1["
  "22",

  "P", "%map2",
  "%3n,.3,.3,,#0.0,.5,&2{%3.0p?{.1p}{f}?{.2,${.4a,.4a,:0[02}c,.2d,.2d,:1^"
  "[33}.2A9]3}.!0.0^_1[33",

  "P", "%map",
  "%!2.0u?{.2,.2,@(y5:%25map1)[32}.0du?{.0a,.3,.3,@(y5:%25map2)[33}n,.1,."
  "4c,,#0.0,.5,&2{%2${.2,,#0.0,&1{%1.0u,.0?{.0]2}.1ap?{.1d,:0^[21}f]2}.!0"
  ".0^_1[01}?{.1,${${.5,@(y3:car),@(y5:%25map1)[02},:0,@(y13:apply-to-lis"
  "t)[02}c,${.3,@(y3:cdr),@(y5:%25map1)[02},:1^[22}.1A9]2}.!0.0^_1[32",

  "S", "map",
  "l6:y12:syntax-rules;n;l2:l3:y1:_;y1:p;y1:l;;l3:y5:%25map1;y1:p;y1:l;;;"
  "l2:l4:y1:_;y1:p;y2:l1;y2:l2;;l4:y5:%25map2;y1:p;y2:l1;y2:l2;;;l2:py1:_"
  ";y4:args;;py4:%25map;y4:args;;;l2:y1:_;y4:%25map;;",

  "P", "%for-each1",
  "%2.1,,#0.2,.1,&2{%1.0p?{${.2a,:1[01}.0d,:0^[11}]1}.!0.0^_1[21",

  "P", "%for-each2",
  "%3.2,.2,,#0.3,.1,&2{%2.0p?{.1p}{f}?{${.3a,.3a,:1[02}.1d,.1d,:0^[22}]2}"
  ".!0.0^_1[32",

  "P", "%for-each",
  "%!2.0u?{.2,.2,@(y10:%25for-each1)[32}.0du?{.0a,.3,.3,@(y10:%25for-each"
  "2)[33}.0,.3c,,#0.3,.1,&2{%1${.2,,#0.0,&1{%1.0u,.0?{.0]2}.1ap?{.1d,:0^["
  "21}f]2}.!0.0^_1[01}?{${${.4,@(y3:car),@(y5:%25map1)[02},:1,@(y13:apply"
  "-to-list)[02}${.2,@(y3:cdr),@(y5:%25map1)[02},:0^[11}]1}.!0.0^_1[31",

  "S", "for-each",
  "l6:y12:syntax-rules;n;l2:l3:y1:_;y1:p;y1:l;;l3:y10:%25for-each1;y1:p;y"
  "1:l;;;l2:l4:y1:_;y1:p;y2:l1;y2:l2;;l4:y10:%25for-each2;y1:p;y2:l1;y2:l"
  "2;;;l2:py1:_;y4:args;;py9:%25for-each;y4:args;;;l2:y1:_;y9:%25for-each"
  ";;",

  "P", "string-map",
  "%!2.0u?{.2S3,'(c ),.1S2,'0,,#0.0,.3,.8,.8,.7,&5{%1:0,.1I<!?{:3]1}${.2,"
  ":2S4,:1[01},.1,:3S5'1,.1I+,:4^[11}.!0.0^_1[51}${${.4,.7c,@(y13:%25stri"
  "ng->list),@(y5:%25map1)[02},.4c,@(y4:%25map),@(y13:apply-to-list)[02}X"
  "3]3",

  "P", "vector-map",
  "%!2.0u?{.2V3,f,.1V2,'0,,#0.0,.3,.8,.8,.7,&5{%1:0,.1I<!?{:3]1}${.2,:2V4"
  ",:1[01},.1,:3V5'1,.1I+,:4^[11}.!0.0^_1[51}${${.4,.7c,@(y13:%25vector->"
  "list),@(y5:%25map1)[02},.4c,@(y4:%25map),@(y13:apply-to-list)[02}X1]3",

  "P", "string-for-each",
  "%!2.0u?{.2S3,'0,,#0.0,.5,.7,.5,&4{%1:0,.1I<!?{]1}${.2,:1S4,:2[01}'1,.1"
  "I+,:3^[11}.!0.0^_1[41}${.2,.5c,@(y13:%25string->list),@(y5:%25map1)[02"
  "},.2c,@(y9:%25for-each),@(y13:apply-to-list)[32",

  "P", "vector-for-each",
  "%!2.0u?{.2V3,'0,,#0.0,.5,.7,.5,&4{%1:0,.1I<!?{]1}${.2,:1V4,:2[01}'1,.1"
  "I+,:3^[11}.!0.0^_1[41}${.2,.5c,@(y13:%25vector->list),@(y5:%25map1)[02"
  "},.2c,@(y9:%25for-each),@(y13:apply-to-list)[32",

  "P", "error",
  "%!1.0,.2Z7]2",

  "P", "port?",
  "%1.0P00,.0?{.0]2}.1P01]2",

  "P", "close-port",
  "%1.0P00?{.0P60}.0P01?{.0P61]1}]1",

  "P", "call-with-port",
  "%2.0,&1{%!0${:0,@(y10:close-port)[01}.0,@(y6:values),@(y13:apply-to-li"
  "st)[12},.1,.3,&2{%0:1,:0[01},@(y16:call-with-values)[22",

  "P", "call-with-input-file",
  "%2.1,.1P40,@(y14:call-with-port)[22",

  "P", "call-with-output-file",
  "%2.1,.1P41,@(y14:call-with-port)[22",

  "P", "read-line",
  "%!0P51,.1u?{P10}{.1a},t,,#0.2,.4,.2,&3{%1:2R0,.0R8,.0?{.0}{'(c%0a),.2C"
  "=}_1?{.0R8?{.1}{f}?{.0]2}:1P9,:1P61.0]3}'(c%0d),.1C=?{f,:0^[21}:1,.1W0"
  "f,:0^[21}.!0.0^_1[31",

  "P", "read-substring!",
  "%4.1,,#0.5,.4,.4,.3,.8,&5{%1:0,.1I<!?{:3,.1I-]1}:4R0,.0R8?{:3,.2I=?{.0"
  "]2}:3,.2I-]2}.0,.2,:2S5'1,.2I+,:1^[21}.!0.0^_1[41",

  "P", "read-substring",
  "%2'(c ),.1S2,${.4,.4,'0,.5,@(y15:read-substring!)[04},.0R8?{.0]4}.2,.1"
  "I=?{.1]4}.0,'0,.3S7]4",

  "C", 0,
  "&0{%4.1,.4,.4,.3,@(y15:read-substring!)[44}%x,&0{%3.1,.1S3,.4,.3,@(y15"
  ":read-substring!)[34}%x,&0{%2.1,.1S3,'0,.3,@(y15:read-substring!)[24}%"
  "x,&0{%1P10,.1S3,'0,.3,@(y15:read-substring!)[14}%x,&4{|10|21|32|43%%}@"
  "!(y12:read-string!)",

  "C", 0,
  "&0{%2.1,.1,@(y14:read-substring)[22}%x,&0{%1P10,.1,@(y14:read-substrin"
  "g)[12}%x,&2{|10|21%%}@!(y11:read-string)",

  0, 0, 0
};
