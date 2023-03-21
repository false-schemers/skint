/* s.c -- generated via skint -c s.scm */

char *s_code[] = {

  0,
  "&0{%2.1,.1G4,.2,.2G3,@(y5:%25sdmv)[22}@!(y6:floor/)",

  0,
  "&0{%2.1,.1G6,.2,.2G5,@(y5:%25sdmv)[22}@!(y9:truncate/)",

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

  "append",
  "l7:y12:syntax-rules;n;l2:l1:y1:_;;l2:y5:quote;n;;;l2:l2:y1:_;y1:x;;y1:"
  "x;;l2:l3:y1:_;y1:x;y1:y;;l3:y8:list-cat;y1:x;y1:y;;;l2:l5:y1:_;y1:x;y1"
  ":y;y1:z;y3:...;;l3:y8:list-cat;y1:x;l4:y6:append;y1:y;y1:z;y3:...;;;;l"
  "2:y1:_;y16:%25residual-append;;",

  0,
  "&0{%!2.0u?{.2,.2A2]3}.0a,.3,.3,,#0.0,&1{%3.1p?{${.3a,.3,.6[02}?{.1]3}."
  "2,.2d,.2,:0^[33}f]3}.!0.0^_1[33}@!(y7:%25member)",

  "member",
  "l5:y12:syntax-rules;n;l2:l3:y1:_;y1:v;y1:l;;l3:y4:meme;y1:v;y1:l;;;l2:"
  "py1:_;y4:args;;py7:%25member;y4:args;;;l2:y1:_;y7:%25member;;",

  0,
  "&0{%!2.0u?{.2,.2A5]3}.0a,.3,.3,,#0.0,&1{%3.1p?{${.3aa,.3,.6[02}?{.1a]3"
  "}.2,.2d,.2,:0^[33}f]3}.!0.0^_1[33}@!(y6:%25assoc)",

  "assoc",
  "l5:y12:syntax-rules;n;l2:l3:y1:_;y1:v;y2:al;;l3:y4:asse;y1:v;y2:al;;;l"
  "2:py1:_;y4:args;;py6:%25assoc;y4:args;;;l2:y1:_;y6:%25assoc;;",

  "list-copy",
  "l5:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l3:y5:%25lcat;y1:x;l2:y5:quote;"
  "n;;;;l2:py1:_;y12:syntax-rules;;py19:%25residual-list-copy;y12:syntax-"
  "rules;;;l2:y1:_;y19:%25residual-list-copy;;",

  0,
  "&0{%1n,.1,@(y5:%25lcat)[12}@!(y19:%25residual-list-copy)",

  "list*",
  "l7:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;y1:x;;l2:l3:y1:_;y1:x;y1:y;;l3:"
  "y4:cons;y1:x;y1:y;;;l2:l5:y1:_;y1:x;y1:y;y1:z;y3:...;;l3:y4:cons;y1:x;"
  "l4:y5:list*;y1:y;y1:z;y3:...;;;;l2:py1:_;y4:args;;py15:%25residual-lis"
  "t*;y4:args;;;l2:y1:_;y15:%25residual-list*;;",

  "cons*",
  "l4:y12:syntax-rules;n;l2:py1:_;y4:args;;py5:list*;y4:args;;;l2:y1:_;y5"
  ":list*;;",

  "vector",
  "l4:y12:syntax-rules;n;l2:py1:_;y4:args;;py4:%25vec;y4:args;;;l2:y1:_;y"
  "4:%25vec;;",

  0,
  "&0{%3n,'1,.4I-,,#0.3,.1,.6,&3{%2:0,.1I<?{.1]2}.1,.1,:2V4c,'1,.2I-,:1^["
  "22}.!0.0^_1[32}@!(y15:subvector->list)",

  0,
  "&0{%3.2,.2,.2,@(y15:subvector->list)[33}%x,&0{%2.0V3,.2,.2,@(y15:subve"
  "ctor->list)[23}%x,&0{%1.0X0]1}%x,&3{|10|21|32%%}@!(y13:%25vector->list"
  ")",

  "vector->list",
  "l5:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l2:y5:%25vtol;y1:x;;;l2:py1:_;y"
  "1:r;;py13:%25vector->list;y1:r;;;l2:y1:_;y13:%25vector->list;;",

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
  "o!)[12}@!(y14:%25vector-append)",

  "vector-append",
  "l7:y12:syntax-rules;n;l2:l1:y1:_;;l2:y5:quote;v0:;;;l2:l2:y1:_;y1:x;;l"
  "2:y4:%25ckv;y1:x;;;l2:l3:y1:_;y1:x;y1:y;;l3:y10:vector-cat;y1:x;y1:y;;"
  ";l2:py1:_;y1:r;;py14:%25vector-append;y1:r;;;l2:y1:_;y14:%25vector-app"
  "end;;",

  "string",
  "l4:y12:syntax-rules;n;l2:l3:y1:_;y1:c;y3:...;;l3:y4:%25str;y1:c;y3:..."
  ";;;l2:y1:_;y16:%25residual-string;;",

  0,
  "&0{%3n,'1,.4I-,,#0.3,.1,.6,&3{%2:0,.1I<?{.1]2}.1,.1,:2S4c,'1,.2I-,:1^["
  "22}.!0.0^_1[32}@!(y15:substring->list)",

  0,
  "&0{%3.2,.2,.2,@(y15:substring->list)[33}%x,&0{%2.0S3,.2,.2,@(y15:subst"
  "ring->list)[23}%x,&0{%1.0X2]1}%x,&3{|10|21|32%%}@!(y13:%25string->list"
  ")",

  "string->list",
  "l5:y12:syntax-rules;n;l2:l2:y1:_;y1:x;;l2:y5:%25stol;y1:x;;;l2:py1:_;y"
  "1:r;;py13:%25string->list;y1:r;;;l2:y1:_;y13:%25string->list;;",

  0,
  "&0{%5.1,.1S3I-,.4I+,.5In,.4,.3I>!?{.4,.3,,#0.3,.7,.6,.3,&4{%2:3,.2I<!,"
  ".0?{.0]3}.2,:2S4,.2,:1S5.2'1,.3I+,.2'1,.3I+,:0^[32}.!0.0^_1[62}'1,.1I-"
  ",'1,.6,.8I-I-,.4I+,,#0.7,.7,.6,.3,&4{%2:3,.2I<,.0?{.0]3}.2,:2S4,.2,:1S"
  "5.2'1,.3I-,.2'1,.3I-,:0^[32}.!0.0^_1[62}@!(y15:substring-copy!)",

  0,
  "&0{%5.4,.4,.4,.4,.4,@(y15:substring-copy!)[55}%x,&0{%4.2S3,.4,.4,.4,.4"
  ",@(y15:substring-copy!)[45}%x,&0{%3.2S3,'0,.4,.4,.4,@(y15:substring-co"
  "py!)[35}%x,&3{|30|41|52%%}@!(y12:string-copy!)",

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
  "-into!)[12}@!(y14:%25string-append)",

  "string-append",
  "l7:y12:syntax-rules;n;l2:l1:y1:_;;s0:;;l2:l2:y1:_;y1:x;;l2:y4:%25cks;y"
  "1:x;;;l2:l3:y1:_;y1:x;y1:y;;l3:y10:string-cat;y1:x;y1:y;;;l2:py1:_;y1:"
  "r;;py14:%25string-append;y1:r;;;l2:y1:_;y14:%25string-append;;",

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
  ".2,:2S4,:1[01},.1,:3S5.0'1,.1I+,:4^[11}.!0.0^_1[51}${@(y13:%25string->"
  "list),${.5,.8c,,#0.4,.1,&2{%1.0p?{${.2d,:0^[01},${.3a,:1[01}c]1}n]1}.!"
  "0.0^_1[01}_1,.4c,@(y13:%25residual-map),@(y5:%25appl)[02}X3]3}@!(y10:s"
  "tring-map)",

  0,
  "&0{%!2.0u?{.2V3,f,.1V2,'0,,#0.0,.3,.8,.8,.7,&5{%1:0,.1I<!?{:3]1}${.2,:"
  "2V4,:1[01},.1,:3V5.0'1,.1I+,:4^[11}.!0.0^_1[51}${@(y13:%25vector->list"
  "),${.5,.8c,,#0.4,.1,&2{%1.0p?{${.2d,:0^[01},${.3a,:1[01}c]1}n]1}.!0.0^"
  "_1[01}_1,.4c,@(y13:%25residual-map),@(y5:%25appl)[02}X1]3}@!(y10:vecto"
  "r-map)",

  0,
  "&0{%!2.0u?{.2S3,'0,,#0.2,.6,.6,.3,&4{%1:3,.1I<!,.0?{.0]2}${.3,:2S4,:1["
  "01}.1'1,.2I+,:0^[21}.!0.0^_1[41}@(y13:%25string->list),${.3,.6c,,#0.4,"
  ".1,&2{%1.0p?{${.2d,:0^[01},${.3a,:1[01}c]1}n]1}.!0.0^_1[01}_1,.2c,@(y1"
  "8:%25residual-for-each),@(y5:%25appl)[32}@!(y15:string-for-each)",

  0,
  "&0{%!2.0u?{.2V3,'0,,#0.2,.6,.6,.3,&4{%1:3,.1I<!,.0?{.0]2}${.3,:2V4,:1["
  "01}.1'1,.2I+,:0^[21}.!0.0^_1[41}@(y13:%25vector->list),${.3,.6c,,#0.4,"
  ".1,&2{%1.0p?{${.2d,:0^[01},${.3a,:1[01}c]1}n]1}.!0.0^_1[01}_1,.2c,@(y1"
  "8:%25residual-for-each),@(y5:%25appl)[32}@!(y15:vector-for-each)",

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

  0,
  "&0{%!0P51,.1u?{P10}{.1a},t,,#0.2,.4,.2,&3{%1:2R0,.0R8,.0?{.0}{'(c%0a),"
  ".2C=}_1?{.0R8?{.1}{f}?{.0]2}:1P9,:1P61.0]3}'(c%0d),.1C=?{f,:0^[21}:1,."
  "1W0f,:0^[21}.!0.0^_1[31}@!(y9:read-line)",

  0,
  "&0{%!0.0]1}@!(y14:%25residual-list)",

  0,
  "&0{%!1.0u?{f,.2,@(y10:%25make-list)[22}.0a,.2,@(y10:%25make-list)[22}@"
  "!(y19:%25residual-make-list)",

  "minmax-reducer",
  "l3:y12:syntax-rules;n;l2:l2:y1:_;y1:f;;l3:y6:lambda;py1:x;y4:args;;l4:"
  "y3:let;y4:loop;l2:l2:y1:x;y1:x;;l2:y4:args;y4:args;;;l4:y2:if;l2:y5:nu"
  "ll?;y4:args;;y1:x;l3:y4:loop;l3:y1:f;y1:x;l2:y3:car;y4:args;;;l2:y3:cd"
  "r;y4:args;;;;;;;",

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

  0, 0
};
