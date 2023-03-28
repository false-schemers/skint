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

  "C", 0,
  "@(y4:box?)@!(y8:promise?)",

  "P", "make-promise",
  "%1.0,tcb]1",

  "P", "make-lazy-promise",
  "%1.0,fcb]1",

  "P", "force",
  "%1.0z,.0a?{.0d]2}${.2d[00},.2z,.0a~?{.1za,.1sa.1zd,.1sd.0,.2sz}.3,@(y5"
  ":force)[41",

  "S", "delay-force",
  "l3:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l2:y17:make-lazy-promise;l3:y6:"
  "lambda;n;y1:x;;;;",

  "S", "delay",
  "l3:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l2:y11:delay-force;l2:y12:make-"
  "promise;y1:x;;;;",

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

  "P", "subbytevector->list",
  "%3n,'1,.4I-,,#0.3,.1,.6,&3{%2:0,.1I<?{.1]2}.1,.1,:2B4c,'1,.2I-,:1^[22}"
  ".!0.0^_1[32",

  "C", 0,
  "&0{%3.2,.2,.2,@(y19:subbytevector->list)[33}%x,&0{%2.0B3,.2,.2,@(y19:s"
  "ubbytevector->list)[23}%x,&0{%1.0B3,'0,.2,@(y19:subbytevector->list)[1"
  "3}%x,&3{|10|21|32%%}@!(y16:bytevector->list)",

  "P", "subbytevector-copy!",
  "%5.1,.1B3I-,.4I+,.5In,.4,.3I>!?{.4,.3,,#0.0,.5,.8,.6,&4{%2:0,.2I<!?{]2"
  "}.1,:1B4,.1,:2B5'1,.2I+,'1,.2I+,:3^[22}.!0.0^_1[62}'1,.1I-,'1,.6,.8I-I"
  "-,.4I+,,#0.0,.5,.8,.(i10),&4{%2:0,.2I<?{]2}.1,:1B4,.1,:2B5'1,.2I-,'1,."
  "2I-,:3^[22}.!0.0^_1[62",

  "C", 0,
  "&0{%5.4,.4,.4,.4,.4,@(y19:subbytevector-copy!)[55}%x,&0{%4.2B3,.4,.4,."
  "4,.4,@(y19:subbytevector-copy!)[45}%x,&0{%3.2B3,'0,.4,.4,.4,@(y19:subb"
  "ytevector-copy!)[35}%x,&3{|30|41|52%%}@!(y16:bytevector-copy!)",

  "C", 0,
  "&0{%3.2,.2,.2B7]3}%x,&0{%2.0B3,.2,.2B7]2}%x,&0{%1.0B3,'0,.2B7]1}%x,&3{"
  "|10|21|32%%}@!(y15:bytevector-copy)",

  "P", "subbytevector-fill!",
  "%4.2,,#0.0,.3,.5,.8,&4{%1:0,.1I<!?{]1}:1,.1,:2B5'1,.1I+,:3^[11}.!0.0^_"
  "1[41",

  "C", 0,
  "&0{%4.3,.3,.3,.3,@(y19:subbytevector-fill!)[44}%x,&0{%3.0B3,.3,.3,.3,@"
  "(y19:subbytevector-fill!)[34}%x,&0{%2.0B3,'0,.3,.3,@(y19:subbytevector"
  "-fill!)[24}%x,&3{|20|31|42%%}@!(y16:bytevector-fill!)",

  "P", "%bytevectors-sum-length",
  "%1'0,.1,,#0.0,&1{%2.0u?{.1]2}.0aB3,.2I+,.1d,:0^[22}.!0.0^_1[12",

  "P", "%bytevectors-copy-into!",
  "%2'0,.2,,#0.0,.4,&2{%2.0u?{:0]2}.0d,.1a,.0B3,${.2,'0,.5,.9,:0,@(y19:su"
  "bbytevector-copy!)[05}.0,.5I+,.3,:1^[52}.!0.0^_1[22",

  "P", "bytevector-append",
  "%!0.0,'0,${.4,@(y23:%25bytevectors-sum-length)[01}B2,@(y23:%25bytevect"
  "ors-copy-into!)[12",

  "P", "subutf8->string",
  "%3P51,${.2,.6,.6,.6,@(y19:write-subbytevector)[04}.0P90,.1P61.0]5",

  "C", 0,
  "&0{%3.2,.2,.2,@(y15:subutf8->string)[33}%x,&0{%2.0B3,.2,.2,@(y15:subut"
  "f8->string)[23}%x,&0{%1.0B3,'0,.2,@(y15:subutf8->string)[13}%x,&3{|10|"
  "21|32%%}@!(y12:utf8->string)",

  "P", "substring->utf8",
  "%3P53,${.2,.6,.6,.6,@(y15:write-substring)[04}.0P91,.1P61.0]5",

  "C", 0,
  "&0{%3.2,.2,.2,@(y15:substring->utf8)[33}%x,&0{%2.0S3,.2,.2,@(y15:subst"
  "ring->utf8)[23}%x,&0{%1.0S3,'0,.2,@(y15:substring->utf8)[13}%x,&3{|10|"
  "21|32%%}@!(y12:string->utf8)",

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

  "P", "read-error",
  "%!1.0,.2Z7]2",

  "P", "port?",
  "%1.0P00,.0?{.0]2}.1P01]2",

  "C", 0,
  "@(y5:port?)@!(y13:textual-port?)",

  "C", 0,
  "@(y5:port?)@!(y12:binary-port?)",

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
  "=}_1?{.0R8?{.1}{f}?{.0]2}:1P90,:1P61.0]3}'(c%0d),.1C=?{f,:0^[21}:1,.1W"
  "0f,:0^[21}.!0.0^_1[31",

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

  "P", "read-subbytevector!",
  "%4.1,,#0.5,.4,.4,.3,.8,&5{%1:0,.1I<!?{:3,.1I-]1}:4R3,.0R8?{:3,.2I=?{.0"
  "]2}:3,.2I-]2}.0,.2,:2B5'1,.2I+,:1^[21}.!0.0^_1[41",

  "P", "read-subbytevector",
  "%2'0,.1B2,${.4,.4,'0,.5,@(y19:read-subbytevector!)[04},.0R8?{.0]4}.2,."
  "1I=?{.1]4}.0,'0,.3B7]4",

  "C", 0,
  "&0{%4.1,.4,.4,.3,@(y19:read-subbytevector!)[44}%x,&0{%3.1,.1B3,.4,.3,@"
  "(y19:read-subbytevector!)[34}%x,&0{%2.1,.1B3,'0,.3,@(y19:read-subbytev"
  "ector!)[24}%x,&0{%1P10,.1B3,'0,.3,@(y19:read-subbytevector!)[14}%x,&4{"
  "|10|21|32|43%%}@!(y16:read-bytevector!)",

  "C", 0,
  "&0{%2.1,.1,@(y18:read-subbytevector)[22}%x,&0{%1P10,.1,@(y18:read-subb"
  "ytevector)[12}%x,&2{|10|21%%}@!(y15:read-bytevector)",

  "P", "%read",
  "%2,,,,,,,,,,,,,,,,,,,,,,#0#1#2#3#4#5#6#7#8#9#(i10)#(i11)#(i12)#(i13)#("
  "i14)#(i15)#(i16)#(i17)#(i18)#(i19)#(i20)#(i21)n.!0&0{%1.0,&1{%0:0z]0}]"
  "1}.!1&0{%1.0K0]1}.!2.3,&1{%1.0K0?{${.2[00},:0^[11}.0]1}.!3.4,.4,&2{%1."
  "0p?{.0aK0?{${.2a,:0^[01},.1sa}{${.2a,:1^[01}}.0dK0?{${.2d,:0^[01},.1sd"
  "]1}.0d,:1^[11}.0V0?{'0,,#0.2,:0,:1,.3,&4{%1:3V3,.1I<?{.0,:3V4,.0K0?{${"
  ".2,:2^[01},.2,:3V5}{${.2,:1^[01}}_1'1,.1I+,:0^[11}]1}.!0.0^_1[11}.0Y2?"
  "{.0zK0?{${.2z,:1^[01},.1sz]1}.0z,:1^[11}f]1}.!4.4,&1{%1${.2,:0^[01}.0]"
  "1}.!5f.!6f.!7f.!8f.!9'(y12:reader-token),l1,.0.!7'(s17:right parenthes"
  "is),.1c.!8'(s13:right bracket),.1c.!9'(s5:%22 . %22),.1c.!(i10)_1.6,&1"
  "{%1.0p?{:0^,.1aq]1}f]1}.!(i10)&0{%1'(s80:ABCDEFGHIJKLMNOPQRSTUVWXYZabc"
  "defghijklmnopqrstuvwxyz!$%25&*/:<=>?^_~0123456789+-.@),.1S8]1}.!(i11)&"
  "0{%1.0X8,'(i48),.1<!?{'(i57),.1>!}{f},.0?{.0]3}'(i65),.2<!?{'(i70),.2>"
  "!}{f},.0?{.0]4}'(i97),.3<!?{'(i102),.3>!]4}f]4}.!(i12)&0{%1.0C1,.0?{.0"
  "]2}'(c)),.2C=,.0?{.0]3}'(c(),.3C=,.0?{.0]4}'(c]),.4C=,.0?{.0]5}'(c[),."
  "5C=,.0?{.0]6}'(c%22),.6C=,.0?{.0]7}'(c;),.7C=]7}.!(i13).(i15),.(i11),&"
  "2{%1${.2,:1^[01},.0R8?{.1,'(y5:port:),'(s22:unexpected end of file),@("
  "y10:read-error)[23}${.2,:0^[01}?{.1,'(y5:port:),.2d,'(s17:unexpected t"
  "oken:),@(y10:read-error)[24}.0]2}.!(i14).8,.(i12),.(i20),.(i24),.(i19)"
  ",.(i12),.(i22),.(i24),.(i27),.(i22),.(i12),.(i25),.(i13),.(i13),.(i37)"
  ",&(i15){%1.0R0,.0R8?{.0]2}.0C1?{.1,:(i10)^[21}'(c(),.1C=?{t,:9^,.3,.3,"
  ":8^[24}'(c)),.1C=?{:9^]2}'(c[),.1C=?{t,:(i14)^,.3,.3,:8^[24}'(c]),.1C="
  "?{:(i14)^]2}'(c'),.1C=?{${.3,:3^[01},'(y5:quote),l2]2}'(c`),.1C=?{${.3"
  ",:3^[01},'(y10:quasiquote),l2]2}${.2,:(i13)^[01}?{.1,.1,:(i11)^[22}'(c"
  ";),.1C=?{${.3R0,,#0.5,.1,&2{%1.0R8,.0?{.0]2}'(c%0a),.2C=,.0?{.0]3}:1R0"
  ",:0^[31}.!0.0^_1[01}.1,:(i10)^[21}'(c,),.1C=?{.1R1,.0R8?{.2,'(y5:port:"
  "),'(s19:end of file after ,),@(y10:read-error)[33}'(c@),.1C=?{.2R0${.4"
  ",:3^[01},'(y16:unquote-splicing),l2]3}${.4,:3^[01},'(y7:unquote),l2]3}"
  "'(c%22),.1C=?{n,,#0.3,:(i12),.2,&3{%1:2R0,.0R8?{:2,'(y5:port:),'(s27:e"
  "nd of file within a string),@(y10:read-error)[23}'(c%5c),.1C=?{${'(y6:"
  "string),:2,:1^[02},.0?{.2,.1c}{.2},:0^[31}'(c%22),.1C=?{.1A9X3]2}.1,.1"
  "c,:0^[21}.!0.0^_1[21}'(c|),.1C=?{n,,#0.3,:(i12),.2,&3{%1:2R0,.0R8?{:2,"
  "'(y5:port:),'(s29:end of file within a |symbol|),@(y10:read-error)[23}"
  "'(c%5c),.1C=?{${'(y6:symbol),:2,:1^[02},.0?{.2,.1c}{.2},:0^[31}'(c|),."
  "1C=?{.1A9X3X5]2}.1,.1c,:0^[21}.!0.0^_1[21}'(c#),.1C=?{.1R1,.0R8?{.2,'("
  "y5:port:),'(s19:end of file after #),@(y10:read-error)[33}'(ct),.1Ci=,"
  ".0?{.0}{'(cf),.2Ci=}_1?{${.4,:3^[01},.0,'(l2:y1:t;y4:true;),.1A1?{t]5}"
  "'(l2:y1:f;y5:false;),.1A1?{f]5}.4,'(y5:port:),.3,'(s23:unexpected name"
  " after #),@(y10:read-error)[54}'(cb),.1Ci=,.0?{.0}{'(co),.2Ci=,.0?{.0}"
  "{'(cd),.3Ci=,.0?{.0}{'(cx),.4Ci=,.0?{.0}{'(ci),.5Ci=,.0?{.0}{'(ce),.6C"
  "i=}_1}_1}_1}_1}_1?{.2,'(c#),:(i11)^[32}'(c&),.1C=?{.2R0${.4,:3^[01}b]3"
  "}'(c;),.1C=?{.2R0${.4,:3^[01}.2,:(i10)^[31}'(c|),.1C=?{.2R0${,#0.5,.1,"
  "&2{%0:1R0,.0R8?{:1,'(y5:port:),'(s25:end of file in #| comment),@(y10:"
  "read-error)[13}'(c|),.1C=?{:1R1,.0R8?{:1,'(y5:port:),'(s25:end of file"
  " in #| comment),@(y10:read-error)[23}'(c#),.1C=?{:1R0]2}:0^[20}'(c#),."
  "1C=?{:1R1,.0R8?{:1,'(y5:port:),'(s25:end of file in #| comment),@(y10:"
  "read-error)[23}'(c|),.1C=?{:1R0${:0^[00}:0^[20}:0^[20}:0^[10}.!0.0^_1["
  "00}.2,:(i10)^[31}'(c(),.1C=?{.2R0${f,:9^,.6,.5,:8^[04}X1]3}'(cu),.1C=?"
  "{.2R0'(c8),.3R0q?{'(c(),.3R0q}{f}?{${.4,:7^[01}E1]3}.2,'(y5:port:),'(s"
  "25:invalid bytevector syntax),@(y10:read-error)[33}'(c%5c),.1C=?{.2R0."
  "2R1,.0R8?{.3,'(y5:port:),'(s20:end of file after #%5c),@(y10:read-erro"
  "r)[43}.0,'(cx)C=?{.3R0${.5R1,:5^[01}?{.0]4}f,.4,:6^[42}.0C4?{${.5,:3^["
  "01},'1,.1X4S3=?{.1]5}.0,'(y4:null),.1v?{'0X9]6}'(y5:space),.1v?{'(c )]"
  "6}'(y5:alarm),.1v?{'(c%07)]6}'(y9:backspace),.1v?{'(c%08)]6}'(y6:delet"
  "e),.1v?{'(i127)X9]6}'(y6:escape),.1v?{'(i27)X9]6}'(y3:tab),.1v?{'(c%09"
  ")]6}'(l2:y7:newline;y8:linefeed;),.1A1?{'(c%0a)]6}'(y4:vtab),.1v?{'(c%"
  "0b)]6}'(y4:page),.1v?{'(c%0c)]6}'(y6:return),.1v?{'(c%0d)]6}.5,'(y5:po"
  "rt:),.3,'(s15:unknown #%5c name),@(y10:read-error)[64}.3R0.0]4}.0C5?{:"
  "0?{${.4,'(y5:port:),'(s44:#N=/#N# notation is not allowed in this mode"
  "),@(y10:read-error)[03}}n,,#0.4,.1,:4,:3,:2,:1,&6{%1:5R0,.0R8?{:5,'(y5"
  ":port:),'(s32:end of file within a #N notation),@(y10:read-error)[23}."
  "0C5?{.1,.1c,:4^[21}'(c#),.1C=?{.1A9X3,'(i10),.1E9,.0I0?{:0^,.1A3}{f},."
  "0?{.0d]5}'(s22:unknown #n# reference:),'(y5:port:),.4,@(y10:read-error"
  ")[53}'(c=),.1C=?{.1A9X3,'(i10),.1E9,.0I0~?{${'(s22:invalid #n= referen"
  "ce:),'(y5:port:),.5,@(y10:read-error)[03}}{:0^,.1A3?{${'(s18:duplicate"
  " #n= tag:),'(y5:port:),.4,@(y10:read-error)[03}}{f}}fb,:0^,${.3,:1^[01"
  "},.3cc:!0${:5,:2^[01},${.2,:3^[01}?{'(s31:#n= has another label as tar"
  "get),'(y5:port:),.5,@(y10:read-error)[63}.0,.2sz.0]6}:5,'(y5:port:),'("
  "s34:invalid terminator for #N notation),@(y10:read-error)[23}.!0.0^_1["
  "31}.2,'(y5:port:),.2,'(s16:unknown # syntax),@(y10:read-error)[34}.1,'"
  "(y5:port:),.2,'(s22:illegal character read),@(y10:read-error)[24}.!(i1"
  "5).(i15),.(i11),.(i16),.(i12),&4{%4${.3,:3^[01},:0^,.1q?{.2,'(y5:port:"
  "),'(s42:missing car -- ( immediately followed by .),@(y10:read-error)["
  "53}.0,,#0.0,.5,:3,:2,.(i10),:1,.(i11),:0,&8{%1.0R8?{:6,'(y5:port:),'(s"
  "41:eof inside list -- unbalanced parentheses),@(y10:read-error)[13}:1,"
  ".1q?{n]1}:0^,.1q?{:3?{${:6,:2^[01},${:6,:5^[01},:1,.1q?{.1]3}:6,'(y5:p"
  "ort:),.2,'(s31:randomness after form after dot),@(y10:read-error)[34}:"
  "6,'(y5:port:),'(s13:dot in #(...)),@(y10:read-error)[13}${.2,:4^[01}?{"
  ":6,'(y5:port:),.2d,'(s20:error inside list --),@(y10:read-error)[14}${"
  "${:6,:5^[01},:7^[01},.1c]1}.!0.0^_1[51}.!(i16).(i15),.8,.(i12),&3{%1${"
  ".2,:2^[01},,#0.0,.3,:2,:0,:1,&5{%1.0R8?{:3,'(y5:port:),'(s21:eof insid"
  "e bytevector),@(y10:read-error)[13}:0^,.1q?{n]1}${.2,:1^[01}?{:3,'(y5:"
  "port:),.2d,'(s26:error inside bytevector --),@(y10:read-error)[14}.0I0"
  "~,.0?{.0}{'0,.2I<,.0?{.0}{'(i255),.3I>}_1}_1?{:3,'(y5:port:),.2,'(s33:"
  "invalid byte inside bytevector --),@(y10:read-error)[14}${${:3,:2^[01}"
  ",:4^[01},.1c]1}.!0.0^_1[11}.!(i17).(i19),&1{%2.0R0,.0R8?{${.3,'(y5:por"
  "t:),.6,'(s20:end of file within a),@(y10:read-error)[04}}'(c%5c),.1C=,"
  ".0?{.0}{'(c%22),.2C=,.0?{.0}{'(c|),.3C=}_1}_1?{.0]3}'(ca),.1C=?{'(c%07"
  ")]3}'(cb),.1C=?{'(c%08)]3}'(ct),.1C=?{'(c%09)]3}'(cn),.1C=?{'(c%0a)]3}"
  "'(cv),.1C=?{'(c%0b)]3}'(cf),.1C=?{'(c%0c)]3}'(cr),.1C=?{'(c%0d)]3}'(cx"
  "),.1C=?{t,.2,:0^[32}'(y6:string),.3q?{.0C1}{f}?{.1R1,'(c%0a),.2C=,,#0."
  "0,.5,&2{%2.1R8,.0?{.0}{.2C1~}_1?{.0?{f]2}:0,'(y5:port:),'(s32:no newli"
  "ne in line ending escape),@(y10:read-error)[23}.0?{'(c%0a),.2C=}{f}?{f"
  "]2}:0R0:0R1,.1,.0?{.0}{'(c%0a),.4C=}_1,:1^[22}.!0.0^_1[32}.1,'(y5:port"
  ":),.2,'(y1::),.6,'(s22:invalid char escape in),@(y10:read-error)[36}.!"
  "(i18).(i13),.(i13),&2{%2,#0.1,&1{%1.0u?{:0,'(y5:port:),'(s31:%5cx esca"
  "pe sequence is too short),@(y10:read-error)[13}'(i16),.1A9X3X7X9]1}.!0"
  "'0,n,.3R1,,#0.0,.6,:0,.7,.(i10),:1,&6{%3.0R8?{:1?{:4,'(y5:port:),'(s27"
  ":end of file within a string),@(y10:read-error)[33}.1,:2^[31}:1?{'(c;)"
  ",.1C=}{f}?{:4R0.1,:2^[31}:1~?{${.2,:0^[01}}{f}?{.1,:2^[31}${.2,:3^[01}"
  "~?{:4,'(y5:port:),.2,'(s37:unexpected char in %5cx escape sequence),@("
  "y10:read-error)[34}'2,.3>?{:4,'(y5:port:),'(s30:%5cx escape sequence i"
  "s too long),@(y10:read-error)[33}:4R0'1,.3+,.2,.2c,:4R1,:5^[33}.!0.0^_"
  "1[33}.!(i19)&0{%4.0,.0?{.0}{.2C5}_1?{f]4}'(s2:+i),.4Si=,.0?{.0}{'(s2:-"
  "i),.5Si=}_1?{f]4}'(s6:+nan.0),.4Si=,.0?{.0}{'(s6:-nan.0),.5Si=}_1?{f]4"
  "}'(s6:+inf.0),.4Si=,.0?{.0}{'(s6:-inf.0),.5Si=}_1?{f]4}'(c+),.2C=,.0?{"
  ".0}{'(c-),.3C=}_1?{.2du?{t]4}'(c.),.3daC=?{.2ddp?{.2ddaC5~]4}f]4}.2daC"
  "5~]4}'(c.),.2C=?{.2dp?{.2daC5~]4}f]4}f]4}.!(i20).(i13),.(i21),.(i11),."
  "(i14),&4{%2'(c#),.1C=,.1,l1,.3R1,,#0.5,.1,:0,:1,:2,:3,&6{%3.0R8,.0?{.0"
  "}{${.3,:0^[01}}_1?{.1A9,.0a,.1X3,.5,.0?{.0}{.2C5,.0?{.0}{'(c+),.4C=,.0"
  "?{.0}{'(c-),.5C=,.0?{.0}{'(c.),.6C=}_1}_1}_1}_1?{'(s1:.),.1S=?{:2^]6}$"
  "{.2,.5,.5,.(i10),:1^[04}?{.0X5]6}'(i10),.1E9,.0?{.0]7}:5,'(y5:port:),."
  "3,'(s54:unsupported number syntax (implementation restriction)),@(y10:"
  "read-error)[74}.0X5]6}'(c#),.1C=?{:5R0t,.2,.2c,:5R1,:4^[33}${.2,:3^[01"
  "}?{:5R0.2,.2,.2c,:5R1,:4^[33}:5,'(y5:port:),.2,'(s29:unexpected number"
  "/symbol char),@(y10:read-error)[34}.!0.0^_1[23}.!(i21)${.(i24),.(i18)^"
  "[01},${.2,.(i14)^[01}~?{.1^u?{.0](i25)}.0,.7^[(i25)1}.(i23),'(y5:port:"
  "),.2d,'(s17:unexpected token:),@(y10:read-error)[(i25)4",

  "C", 0,
  "&0{%1f,.1,@(y5:%25read)[12}%x,&0{%0f,P10,@(y5:%25read)[02}%x,&2{|00|11"
  "%%}@!(y4:read)",

  "C", 0,
  "&0{%1t,.1,@(y5:%25read)[12}%x,&0{%0t,P10,@(y5:%25read)[02}%x,&2{|00|11"
  "%%}@!(y11:read-simple)",

  "P", "write-substring",
  "%4.1,,#0.0,.3,.7,.7,&4{%1:0,.1I<!?{]1}:1,.1,:2S4W0'1,.1I+,:3^[11}.!0.0"
  "^_1[41",

  "C", 0,
  "&0{%4.1,.4,.4,.3,@(y15:write-substring)[44}%x,&0{%3.1,.1S3,.4,.3,@(y15"
  ":write-substring)[34}%x,&0{%2.1,.1W1]2}%x,&0{%1P11,.1W1]1}%x,&4{|10|21"
  "|32|43%%}@!(y12:write-string)",

  "P", "write-subbytevector",
  "%4.1,,#0.0,.3,.7,.7,&4{%1:0,.1I<!?{]1}:1,.1,:2B4W2'1,.1I+,:3^[11}.!0.0"
  "^_1[41",

  "C", 0,
  "&0{%4.1,.4,.4,.3,@(y19:write-subbytevector)[44}%x,&0{%3.1,.1B3,.4,.3,@"
  "(y19:write-subbytevector)[34}%x,&0{%2.1,.1W3]2}%x,&0{%1P11,.1W3]1}%x,&"
  "4{|10|21|32|43%%}@!(y16:write-bytevector)",

  "P", "command-line",
  "%0'0,n,,#0.0,&1{%2.1Z0,.0?{'1,.3I+,.2,.2c,:0^[32}.1A9]3}.!0.0^_1[02",

  0, 0, 0
};
