/* t.c -- generated via skint -c t.scm */

char *t_code[] = {

  "P", "set-member?",
  "%2.1u?{f]2}.1a,.1q?{t]2}.1d,.1,@(y11:set-member?)[22",

  "P", "set-cons",
  "%2${.3,.3,@(y11:set-member?)[02}?{.1]2}.1,.1c]2",

  "P", "set-union",
  "%2.0u?{.1]2}${.3,.3a,@(y8:set-cons)[02},.1d,@(y9:set-union)[22",

  "P", "set-minus",
  "%2.0u?{n]2}${.3,.3a,@(y11:set-member?)[02}?{.1,.1d,@(y9:set-minus)[22}"
  "${.3,.3d,@(y9:set-minus)[02},.1ac]2",

  "P", "set-intersect",
  "%2.0u?{n]2}${.3,.3a,@(y11:set-member?)[02}?{${.3,.3d,@(y13:set-interse"
  "ct)[02},.1ac]2}.1,.1d,@(y13:set-intersect)[22",

  "S", "record-case",
  "l7:y12:syntax-rules;l1:y4:else;;l2:l4:y11:record-case;py2:pa;y2:ir;;y6"
  ":clause;y3:...;;l3:y3:let;l1:l2:y2:id;py2:pa;y2:ir;;;;l4:y11:record-ca"
  "se;y2:id;y6:clause;y3:...;;;;l2:l2:y11:record-case;y2:id;;l2:y5:quote;"
  "y16:record-case-miss;;;l2:l3:y11:record-case;y2:id;l3:y4:else;y3:exp;y"
  "3:...;;;l3:y5:begin;y3:exp;y3:...;;;l2:l5:y11:record-case;y2:id;l4:l2:"
  "y3:key;y3:...;;y3:ids;y3:exp;y3:...;;y6:clause;y3:...;;l4:y2:if;l3:y4:"
  "memq;l2:y3:car;y2:id;;l2:y5:quote;l2:y3:key;y3:...;;;;l3:y5:apply;l4:y"
  "6:lambda;y3:ids;y3:exp;y3:...;;l2:y3:cdr;y2:id;;;l4:y11:record-case;y2"
  ":id;y6:clause;y3:...;;;;l2:l5:y11:record-case;y2:id;l4:y3:key;y3:ids;y"
  "3:exp;y3:...;;y6:clause;y3:...;;l4:y2:if;l3:y3:eq?;l2:y3:car;y2:id;;l2"
  ":y5:quote;y3:key;;;l3:y5:apply;l4:y6:lambda;y3:ids;y3:exp;y3:...;;l2:y"
  "3:cdr;y2:id;;;l4:y11:record-case;y2:id;y6:clause;y3:...;;;;",

  "P", "sexp-match?",
  "%2'(y1:*),.1q,.0?{.0]3}'(y4:<id>),.2q?{.2Y0,.0?{.0}{.3K0}_1}{f},.0?{.0"
  "]4}'(y8:<symbol>),.3q?{.3Y0}{f},.0?{.0]5}'(y8:<number>),.4q?{.4N0}{f},"
  ".0?{.0]6}'(y8:<string>),.5q?{.5S0}{f},.0?{.0]7}'(y8:<vector>),.6q?{.6V"
  "0}{f},.0?{.0]8}'(y5:<box>),.7q?{.7Y2}{f},.0?{.0]9}.7,.9v,.0?{.0](i10)}"
  ".8p?{'(y3:...),.9aq?{.8dp?{.8ddu}{f}}{f}?{.8da,.(i10)v}{.8dp?{'(y3:..."
  "),.9daq?{.8ddu}{f}}{f}?{.8a,'(y1:*),.1q?{.(i10)L0}{${.(i12),,#0.0,.5,&"
  "2{%1.0u,.0?{.0]2}.1p?{${.3a,:0,@(y11:sexp-match?)[02}?{.1d,:1^[21}f]2}"
  "f]2}.!0.0^_1[01}}_1}{.9p?{${.(i11)a,.(i11)a,@(y11:sexp-match?)[02}?{${"
  ".(i11)d,.(i11)d,@(y11:sexp-match?)[02}}{f}}{f}}}}{f},.0?{.0](i11)}.9V0"
  "?{.(i10)V0?{${.(i12)X0,.(i12)X0,@(y11:sexp-match?)[02}}{f}}{f},.0?{.0]"
  "(i12)}.(i10)Y2?{.(i11)Y2?{.(i11)z,.(i11)z,@(y11:sexp-match?)[(i12)2}f]"
  "(i12)}f](i12)",

  "S", "sexp-case",
  "l6:y12:syntax-rules;l1:y4:else;;l2:l4:y1:_;l2:y3:key;y3:...;;y7:clause"
  "s;y3:...;;l3:y3:let;l1:l2:y8:atom-key;l2:y3:key;y3:...;;;;l4:y9:sexp-c"
  "ase;y8:atom-key;y7:clauses;y3:...;;;;l2:l3:y1:_;y3:key;l4:y4:else;y7:r"
  "esult1;y7:result2;y3:...;;;l4:y5:begin;y7:result1;y7:result2;y3:...;;;"
  "l2:l3:y1:_;y3:key;l4:y3:pat;y7:result1;y7:result2;y3:...;;;l3:y2:if;l3"
  ":y11:sexp-match?;l2:y5:quote;y3:pat;;y3:key;;l4:y5:begin;y7:result1;y7"
  ":result2;y3:...;;;;l2:l6:y1:_;y3:key;l4:y3:pat;y7:result1;y7:result2;y"
  "3:...;;y6:clause;y7:clauses;y3:...;;l4:y2:if;l3:y11:sexp-match?;l2:y5:"
  "quote;y3:pat;;y3:key;;l4:y5:begin;y7:result1;y7:result2;y3:...;;l5:y9:"
  "sexp-case;y3:key;y6:clause;y7:clauses;y3:...;;;;",

  "P", "symbol-append",
  "%!0${${.4,@(y14:symbol->string),@(y5:%25map1)[02},@(y14:%25string-appe"
  "nd),@(y13:apply-to-list)[02}X5]1",

  "C", 0,
  "'0,#0.0,&1{%!0'1,:0^I+:!0.0u,.0?{.0}{.1aY0~}_1?{'(i10),:0^X6,'(s1:#)S6"
  "X5]1}'(i10),:0^X6,'(s1:#)S6,.1aX4S6X5]1}_1@!(y6:gensym)",

  "P", "posq",
  "%2'0,.2,,#0.0,.4,&2{%2.0u?{f]2}.0a,:0q?{.1]2}'1,.2I+,.1d,:1^[22}.!0.0^"
  "_1[22",

  "P", "rassq",
  "%2.1p?{.1a,.0d,.2q?{.0]3}.2d,.2,@(y5:rassq)[32}f]2",

  "P", "remove!",
  "%3.1,f,f,,#0.0,.7,.6,&3{%3.2p~?{.1?{.2,.2sd.0]3}.2]3}${.4a,:0,:1[02}?{"
  ".2d,.2,.2,:2^[33}.1?{.2,.2sd.2d,.3,.2,:2^[33}.2d,.3,.4,:2^[33}.!0.0^_1"
  "[33",

  "P", "list-diff",
  "%2.0u,.0?{.0}{.2,.2q}_1?{n]2}${.3,.3d,@(y9:list-diff)[02},.1ac]2",

  "P", "pair*",
  "%!1.0,.2,,#0.0,&1{%2.1u?{.0]2}${.3d,.4a,:0^[02},.1c]2}.!0.0^_1[22",

  "P", "append*",
  "%1.0u?{n]1}.0du?{.0a]1}${.2d,@(y7:append*)[01},.1aL6]1",

  "P", "string-append*",
  "%1.0,@(y14:%25string-append),@(y13:apply-to-list)[12",

  "P", "andmap",
  "%2.1p?{${.3a,.3[01}?{.1d,.1,@(y6:andmap)[22}f]2}t]2",

  "P", "ormap",
  "%2.1p?{${.3a,.3[01},.0?{.0]3}.2d,.2,@(y5:ormap)[32}f]2",

  "P", "list1?",
  "%1.0p?{.0du]1}f]1",

  "P", "list1+?",
  "%1.0p?{.0dL0]1}f]1",

  "P", "list2?",
  "%1.0p?{.0d,@(y6:list1?)[11}f]1",

  "P", "list2+?",
  "%1.0p?{.0d,@(y7:list1+?)[11}f]1",

  "P", "list3?",
  "%1.0p?{.0d,@(y6:list2?)[11}f]1",

  "P", "read-code-sexp",
  "%1.0,@(y11:read-simple)[11",

  "P", "error*",
  "%2${.3,.3,f,@(y12:error-object)[03},@(y5:raise)[21",

  "P", "warning*",
  "%2Pe,.2,.2,'(s9:Warning: )S6,@(y19:print-error-message)[23",

  "P", "idslist?",
  "%1.0u?{t]1}.0p?{${.2a,@(y3:id?)[01}?{.0d,@(y8:idslist?)[11}f]1}.0,@(y3"
  ":id?)[11",

  "P", "normalize-arity",
  "%1${.2,@(y6:list2?)[01}?{.0aI0?{.0daY1}{f}}{f}?{.0]1}.0,'0,,#0.0,&1{%2"
  ".1p?{.1d,.1,'1I+,:0^[22}.1u?{f,.1,l2]2}t,.1,l2]2}.!0.0^_1[12",

  "P", "flatten-idslist",
  "%1.0L0?{.0]1}n,.1,,#0.0,&1{%2.0p?{.1,.1ac,.1d,:0^[22}.0u?{.1A9]2}.1A9,"
  ".1c]2}.!0.0^_1[12",

  "P", "idslist-req-count",
  "%1.0p?{${.2d,@(y17:idslist-req-count)[01},'1I+]1}'0]1",

  "A", "location?", "box?",

  "A", "make-location", "box",

  "A", "location-val", "unbox",

  "A", "location-set-val!", "set-box!",

  "A", "val-void?", "void?",

  "A", "val-core?", "pair?",

  "A", "val-transformer?", "procedure?",

  "A", "val-integrable?", "integrable?",

  "A", "val-builtin?", "symbol?",

  "A", "val-library?", "vector?",

  "A", "make-library", "vector",

  "S", "library-code",
  "l3:y12:syntax-rules;n;l2:l2:y1:_;y1:l;;l3:y10:vector-ref;y1:l;i0;;;",

  "S", "library-exports",
  "l3:y12:syntax-rules;n;l2:l2:y1:_;y1:l;;l3:y10:vector-ref;y1:l;i1;;;",

  "S", "library-set-exports!",
  "l3:y12:syntax-rules;n;l2:l3:y1:_;y1:l;y1:v;;l4:y11:vector-set!;y1:l;i1"
  ";y1:v;;;",

  "P", "location-special?",
  "%1.0zp~]1",

  "P", "new-id",
  "%3,#0.3,.3,.3,l3.!0.0,&1{%0:0^]0}]4",

  "A", "new-id?", "procedure?",

  "P", "old-sym",
  "%1${.2[00}a]1",

  "P", "old-den",
  "%1${.2[00}da]1",

  "P", "old-literals",
  "%1${.2[00}dda,.0?{.0}{&0{%0n]0}}_1[10",

  "P", "id?",
  "%1.0Y0,.0?{.0]2}.1K0]2",

  "P", "id->sym",
  "%1.0Y0?{.0]1}.0,@(y7:old-sym)[11",

  "P", "id-rename-as",
  "%2.0,,#0.0,.4,&2{%1.0Y0?{:0]1}${.2,@(y12:old-literals)[01},${.2,.4,@(y"
  "5:rassq)[02},.0~?{${.4,'(s46:id-rename-as failed: not found in its own"
  " lits),@(y7:x-error)[02}}${.2a,:1^[01},.2,.1A3,.0?{.0d]5}.1]5}.!0.0^_1"
  "[21",

  "P", "extend-xenv-local",
  "%3.1b,.1p?{.3,.1,.3,&3{%2.0,:0e?{.1,'(l2:y3:ref;y4:set!;),.1A1?{:1]3}f"
  "]3}.1,.1,:2[22}]4}.3,.1,.3,&3{%2.0,:0q?{.1,'(l2:y3:ref;y4:set!;),.1A1?"
  "{:1]3}f]3}.1,.1,:2[22}]4",

  "P", "add-local-var",
  "%3.2,.2,'(y3:ref),l2,.2,@(y17:extend-xenv-local)[33",

  "P", "xenv-lookup",
  "%3${.4,.4,.4[02},.0?{.0]4}.3,${.5,@(y17:xform-sexp->datum)[01},.4,l3,'"
  "(s38:transformer: invalid identifier access),@(y6:error*)[42",

  "P", "xenv-ref",
  "%2'(y3:ref),.2,.2,@(y11:xenv-lookup)[23",

  "P", "xform-sexp->datum",
  "%1.0,,#0.0,&1{%1${.2,@(y3:id?)[01}?{.0,@(y7:id->sym)[11}.0p?{${.2d,:0^"
  "[01},${.3a,:0^[01}c]1}.0V0?{${.2X0,:0^,@(y5:%25map1)[02}X1]1}.0]1}.!0."
  "0^_1[11",

  "P", "x-error",
  "%!1.0,.2,'(s13:transformer: )S6,@(y6:error*)[22",

  "P", "check-syntax",
  "%3${.2,.4,@(y11:sexp-match?)[02}~?{.0,.3,@(y7:x-error)[32}]3",

  "C", 0,
  "${f,'(y6:lambda)b,'(y6:lambda),@(y6:new-id)[03}@!(y9:lambda-id)",

  "C", 0,
  "${f,'(y5:begin)b,'(y5:begin),@(y6:new-id)[03}@!(y8:begin-id)",

  "C", 0,
  "${f,'(y6:define)b,'(y6:define),@(y6:new-id)[03}@!(y9:define-id)",

  "C", 0,
  "${f,'(y13:define-syntax)b,'(y13:define-syntax),@(y6:new-id)[03}@!(y16:"
  "define-syntax-id)",

  "C", 0,
  "${f,'(y12:syntax-quote)b,'(y12:syntax-quote),@(y6:new-id)[03}@!(y15:sy"
  "ntax-quote-id)",

  "P", "xform",
  "%3${.3,@(y3:id?)[01}?{${.4,.4,@(y9:xform-ref)[02},.1?{.0]4}.0U0?{.0U7,"
  "'(y5:const),l2]4}.0K0?{.3,${.6,.6,.5[02},.3,@(y5:xform)[43}.0V0?{.2,.1"
  ",'(s23:improper use of library),@(y7:x-error)[43}.0Y8?{${.4,@(y7:id->s"
  "ym)[01},.3,.2,'(s15:id has no value),@(y7:x-error)[44}.0p~?{.0,'(s27:i"
  "mproper use of syntax form),@(y7:x-error)[42}.0]4}.1p~?{.2,.2,l1,@(y11"
  ":xform-quote)[32}.1a,.2d,${.6,.4,t,@(y5:xform)[03},.0,'(y5:quote),.1v?"
  "{.6,.3,@(y11:xform-quote)[72}'(y4:set!),.1v?{.6,.3,@(y10:xform-set!)[7"
  "2}'(y4:set&),.1v?{.6,.3,@(y10:xform-set&)[72}'(y2:if),.1v?{.6,.3,@(y8:"
  "xform-if)[72}'(y6:lambda),.1v?{.6,.3,@(y12:xform-lambda)[72}'(y7:lambd"
  "a*),.1v?{.6,.3,@(y13:xform-lambda*)[72}'(y5:letcc),.1v?{.6,.3,@(y11:xf"
  "orm-letcc)[72}'(y6:withcc),.1v?{.6,.3,@(y12:xform-withcc)[72}'(y4:body"
  "),.1v?{.4,.7,.4,@(y10:xform-body)[73}'(y5:begin),.1v?{.4,.7,.4,@(y11:x"
  "form-begin)[73}'(y6:define),.1v?{.6,.3,@(y12:xform-define)[72}'(y13:de"
  "fine-syntax),.1v?{.6,.3,@(y19:xform-define-syntax)[72}'(y12:syntax-quo"
  "te),.1v?{.6,.3,@(y18:xform-syntax-quote)[72}'(y13:syntax-lambda),.1v?{"
  ".4,.7,.4,@(y19:xform-syntax-lambda)[73}'(y12:syntax-rules),.1v?{.6,.3,"
  "@(y18:xform-syntax-rules)[72}'(y13:syntax-length),.1v?{.6,.3,@(y19:xfo"
  "rm-syntax-length)[72}'(y12:syntax-error),.1v?{.6,.3,@(y18:xform-syntax"
  "-error)[72}'(y14:define-library),.1v?{f,.7,.4,.6,@(y20:xform-define-li"
  "brary)[74}'(y6:import),.1v?{f,.7,.4,.6,@(y12:xform-import)[74}.1U0?{.6"
  ",.3,.3,@(y16:xform-integrable)[73}.1K0?{.6,${.9,.9,.6[02},.6,@(y5:xfor"
  "m)[73}.1V0?{.5,.2,'(s23:improper use of library),@(y7:x-error)[73}.1Y8"
  "?{.5,.2,'(s24:use of uninitialized val),@(y7:x-error)[73}.1p~?{.1,'(s2"
  "7:improper use of syntax form),@(y7:x-error)[72}.6,.3,.3,@(y10:xform-c"
  "all)[73",

  "P", "xform-quote",
  "%2${.2,@(y6:list1?)[01}?{${.2a,@(y17:xform-sexp->datum)[01},'(y5:quote"
  "),l2]2}.0,'(y5:quote)c,'(s19:improper quote form),@(y7:x-error)[22",

  "P", "xform-ref",
  "%2${.2,.4,@(y8:xenv-ref)[02},'(y3:...),.1zq?{'(s19:improper use of ..."
  "),@(y7:x-error)[31}.0z]3",

  "P", "xform-set!",
  "%2${.2,@(y6:list2?)[01}?{${.2a,@(y3:id?)[01}}{f}?{${.3,.3da,f,@(y5:xfo"
  "rm)[03},${'(y4:set!),.4a,.6,@(y11:xenv-lookup)[03},${.2,@(y17:location"
  "-special?)[01}?{.2,'(y4:set!)c,'(s38:set! to macro or integrable ident"
  "ifier),@(y7:x-error)[42}.0z,'(y3:ref),.1aq?{.2,.1da,'(y4:set!),l3]5}.3"
  ",'(y4:set!)c,'(s19:set! is not allowed),@(y7:x-error)[52}.0,'(y4:set!)"
  "c,'(s18:improper set! form),@(y7:x-error)[22",

  "P", "xform-set&",
  "%2${.2,@(y6:list1?)[01}?{${'(y4:set!),.3a,.5,@(y11:xenv-lookup)[03},${"
  ".2,@(y17:location-special?)[01}?{.1,'(y4:set&)c,'(s38:set& of macro or"
  " integrable identifier),@(y7:x-error)[32}.0z,'(y3:ref),.1aq?{.0da,'(y4"
  ":set&),l2]4}.2,'(y4:set!)c,'(s19:set& is not allowed),@(y7:x-error)[42"
  "}.0,'(y4:set&)c,'(s18:improper set& form),@(y7:x-error)[22",

  "P", "xform-if",
  "%2.0L0?{${.2,.4,&1{%1:0,.1,f,@(y5:xform)[13},@(y5:%25map1)[02},.0g,'2,"
  ".1v?{'(l1:l1:y5:begin;;),.2L6,'(y2:if)c]4}'3,.1v?{.1,'(y2:if)c]4}.2,'("
  "y2:if)c,'(s17:malformed if form),@(y7:x-error)[42}.0,'(y2:if)c,'(s16:i"
  "mproper if form),@(y7:x-error)[22",

  "P", "xform-call",
  "%3.1L0?{${.3,.5,&1{%1:0,.1,f,@(y5:xform)[13},@(y5:%25map1)[02},.0u?{'("
  "y6:lambda),.2aq?{.1dau}{f}}{f}?{.1dda]4}.0,.2,'(y4:call),@(y5:pair*)[4"
  "3}.1,.1c,'(s20:improper application),@(y7:x-error)[32",

  "P", "integrable-argc-match?",
  "%2.0,'(c0),.1v?{'0,.3=]3}'(c1),.1v?{'1,.3=]3}'(c2),.1v?{'2,.3=]3}'(c3)"
  ",.1v?{'3,.3=]3}'(cp),.1v?{'0,.3<!]3}'(cm),.1v?{'1,.3<!]3}'(cc),.1v?{'2"
  ",.3<!]3}'(cx),.1v?{'1,.3<!]3}'(cu),.1v?{'1,.3,,'0>!;>!]3}'(cb),.1v?{'2"
  ",.3,,'1>!;>!]3}'(ct),.1v?{'3,.3,,'2>!;>!]3}'(c#),.1v?{'0,.3<!]3}'(c@),"
  ".1v?{f]3}f]3",

  "P", "xform-integrable",
  "%3${.3g,.3U6,@(y22:integrable-argc-match?)[02}?{${.3,.5,&1{%1:0,.1,f,@"
  "(y5:xform)[13},@(y5:%25map1)[02},.1c,'(y10:integrable)c]3}.2,.2,.2U7,'"
  "(y3:ref),l2,@(y10:xform-call)[33",

  "P", "xform-lambda",
  "%2${.2,@(y7:list1+?)[01}?{${.2a,@(y8:idslist?)[01}}{f}?{n,.2,.2a,,#0.4"
  ",.1,&2{%3.0p?{.0a,${${.4,@(y7:id->sym)[01},@(y6:gensym)[01},.4,.1c,${."
  "6,.4,.6,@(y13:add-local-var)[03},.4d,:0^[53}.0u?{${f,.4,:1d,@(y10:xfor"
  "m-body)[03},.3A8,'(y6:lambda),l3]3}.0,${${.4,@(y7:id->sym)[01},@(y6:ge"
  "nsym)[01},${.5,.3,.5,@(y13:add-local-var)[03},${f,.3,:1d,@(y10:xform-b"
  "ody)[03},.2,.7A8L6,'(y6:lambda),l3]6}.!0.0^_1[23}.0,'(y6:lambda)c,'(s2"
  "0:improper lambda body),@(y7:x-error)[22",

  "P", "xform-lambda*",
  "%2.0L0?{${.2,.4,&1{%1${.2,@(y6:list2?)[01}?{${.2a,@(y6:list2?)[01}?{.0"
  "aaI0?{.0adaY1}{f}}{f},.0?{.0}{${.3a,@(y8:idslist?)[01}}_1}{f}?{${:0,.3"
  "da,f,@(y5:xform)[03},${.3a,@(y15:normalize-arity)[01},l2]1}.0,'(s23:im"
  "proper lambda* clause),@(y7:x-error)[12},@(y5:%25map1)[02},'(y7:lambda"
  "*)c]2}.0,'(y7:lambda*)c,'(s21:improper lambda* form),@(y7:x-error)[22",

  "P", "xform-letcc",
  "%2${.2,@(y7:list2+?)[01}?{${.2a,@(y3:id?)[01}}{f}?{.0a,${${.4,@(y7:id-"
  ">sym)[01},@(y6:gensym)[01},${f,${.8,.6,.8,@(y13:add-local-var)[03},.6d"
  ",@(y10:xform-body)[03},.1,'(y5:letcc),l3]4}.0,'(y5:letcc)c,'(s19:impro"
  "per letcc form),@(y7:x-error)[22",

  "P", "xform-withcc",
  "%2${.2,@(y7:list2+?)[01}?{f,${.4,.4d,@(y10:xform-body)[02},${.5,.5a,f,"
  "@(y5:xform)[03},'(y6:withcc),l4]2}.0,'(y6:withcc)c,'(s20:improper with"
  "cc form),@(y7:x-error)[22",

  "P", "xform-body",
  "%3.0u?{'(y5:begin),l1]3}${.2,@(y6:list1?)[01}?{.1,.1a,.4,@(y5:xform)[3"
  "3}.0L0~?{.0,'(y4:body)c,'(s18:improper body form),@(y7:x-error)[32}.0,"
  "n,n,n,.5,,#0.8,.1,&2{%5.4p?{.4ap}{f}?{.4d,.5a,.0a,.1d,${.6,.4,t,@(y5:x"
  "form)[03},.0,'(y5:begin),.1v?{.2L0?{.5,.3L6,.(i10),.(i10),.(i10),.(i10"
  "),:0^[(i11)5}.4,'(s19:improper begin form),@(y7:x-error)[(i11)2}'(y6:d"
  "efine),.1v?{${.4,@(y6:list2?)[01}?{.2au}{f}?{.2da,.6,.(i11),fc,.(i11),"
  ".3c,.(i11),fc,.(i11),:0^[(i12)5}${.4,@(y6:list2?)[01}?{${.4a,@(y3:id?)"
  "[01}}{f}?{.2a,.3da,${${.5,@(y7:id->sym)[01},@(y6:gensym)[01},${.(i11),"
  ".3,.6,@(y13:add-local-var)[03},.9,.(i14),.3c,.(i14),.5c,.(i14),.7c,.4,"
  ":0^[(i15)5}${.4,@(y7:list2+?)[01}?{.2ap?{${.4aa,@(y3:id?)[01}?{${.4ad,"
  "@(y8:idslist?)[01}}{f}}{f}}{f}?{.2aa,.3d,.4adc,@(y9:lambda-id)c,${${.5"
  ",@(y7:id->sym)[01},@(y6:gensym)[01},${.(i11),.3,.6,@(y13:add-local-var"
  ")[03},.9,.(i14),.3c,.(i14),.5c,.(i14),.7c,.4,:0^[(i15)5}.4,'(s20:impro"
  "per define form),@(y7:x-error)[(i11)2}'(y13:define-syntax),.1v?{${.4,@"
  "(y6:list2?)[01}?{${.4a,@(y3:id?)[01}}{f}?{.2a,.3da,${.(i10),'(l1:y9:un"
  "defined;),.5,@(y17:extend-xenv-local)[03},.8,.(i13),tc,.(i13),.4c,.(i1"
  "3),.6c,.4,:0^[(i14)5}.4,'(s27:improper define-syntax form),@(y7:x-erro"
  "r)[(i11)2}'(y14:define-library),.1v?{${.4,@(y7:list2+?)[01}?{${.4a,@(y"
  "9:listname?)[01}}{f}?{${f,.9,.6,.8,@(y20:xform-define-library)[04},.0d"
  "a,.1dda,${.(i11),.3,.5,@(y17:extend-xenv-local)[03},.9,.(i14),.(i14),."
  "(i14),.4,:0^[(i15)5}.4,'(s28:improper define-library form),@(y7:x-erro"
  "r)[(i11)2}'(y6:import),.1v?{.2L0?{${f,.9,.6,.8,@(y12:xform-import)[04}"
  ",.0da,'0,.1V4,'1,.2V4,.(i10),.1,,#0.(i10),.1,.(i14),.(i19),.(i19),.(i1"
  "9),:0,.(i11),&8{%2.0u?{:0,@(y15:syntax-quote-id),l2,:5,:4,fc,:3,.3c,:2"
  ",fc,.6,:1^[35}.0ad,${.3aa,:7,@(y12:id-rename-as)[02},.3,.2,.2,&3{%2:0,"
  ".1q?{'(y3:ref),.2q?{:1]2}f]2}.1,.1,:2[22},.3d,:6^[42}.!0.0^_1[(i15)2}."
  "4,'(s20:improper import form),@(y7:x-error)[(i11)2}.1K0?{.5,${.9,.8,.6"
  "[02}c,.(i10),.(i10),.(i10),.(i10),:0^[(i11)5}:1,.7,.(i12),.(i12)A8,.(i"
  "12)A8,.(i12)A8,@(y12:xform-labels)[(i11)6}:1,.1,.6,.6A8,.6A8,.6A8,@(y1"
  "2:xform-labels)[56}.!0.0^_1[35",

  "P", "xform-labels",
  "%6,#0${.5,&0{%1t,.1q]1},@(y6:andmap)[02}.!0n,n,.5,.5,.5,,#0.0,.(i12),."
  "8,.(i15),.(i14),&5{%5.0u?{:2^?{${:0,@(y6:list1?)[01}}{f}?{:3,:0a,:1,@("
  "y5:xform)[53}${:0,:3,&1{%1:0,.1,f,@(y5:xform)[13},@(y5:%25map1)[02},.4"
  "A8L6,${.2,@(y6:list1?)[01}?{.0a}{.0,'(y5:begin)c},.6u?{.0]7}${.8,&0{%1"
  "'(l1:y5:begin;)]1},@(y5:%25map1)[02},.1,.8A8,'(y6:lambda),l3,'(y4:call"
  "),@(y5:pair*)[73}.0a~?{.4,.4,${:3,.6a,f,@(y5:xform)[03}c,.4d,.4d,.4d,:"
  "4^[55}.2aY0?{.4,.3ac,.4,${:3,.6a,.6a,l2,@(y10:xform-set!)[02}c,.4d,.4d"
  ",.4d,:4^[55}${:3,.4a,t,@(y5:xform)[03},${'(y4:set!),.4a,:3,@(y11:xenv-"
  "lookup)[03}sz.4,.4,.4d,.4d,.4d,:4^[55}.!0.0^_1[75",

  "P", "xform-begin",
  "%3.0L0?{${.2,@(y6:list1?)[01}?{.1,.1a,.4,@(y5:xform)[33}${.2,.4,&1{%1:"
  "0,.1,f,@(y5:xform)[13},@(y5:%25map1)[02},'(y5:begin)c]3}.0,'(y5:begin)"
  "c,'(s19:improper begin form),@(y7:x-error)[32",

  "P", "xform-define",
  "%2${.2,@(y6:list2?)[01}?{.0au}{f}?{.1,.1da,f,@(y5:xform)[23}${.2,@(y6:"
  "list2?)[01}?{${.2a,@(y3:id?)[01}}{f}?{${.3,.3da,f,@(y5:xform)[03},${.3"
  "a,@(y7:id->sym)[01},'(y6:define),l3]2}${.2,@(y7:list2+?)[01}?{.0ap?{${"
  ".2aa,@(y3:id?)[01}?{${.2ad,@(y8:idslist?)[01}}{f}}{f}}{f}?{${.3,.3d,.4"
  "adc,@(y12:xform-lambda)[02},${.3aa,@(y7:id->sym)[01},'(y6:define),l3]2"
  "}.0,'(y6:define)c,'(s20:improper define form),@(y7:x-error)[22",

  "P", "xform-define-syntax",
  "%2${.2,@(y6:list2?)[01}?{${.2a,@(y3:id?)[01}}{f}?{${.3,.3da,t,@(y5:xfo"
  "rm)[03},${.3a,@(y7:id->sym)[01},'(y13:define-syntax),l3]2}.0,'(y13:def"
  "ine-syntax)c,'(s27:improper define-syntax form),@(y7:x-error)[22",

  "P", "xform-syntax-quote",
  "%2${.2,@(y6:list1?)[01}?{.0a]2}.0,'(y12:syntax-quote)c,'(s26:improper "
  "syntax-quote form),@(y7:x-error)[22",

  "P", "xform-syntax-lambda",
  "%3${.2,@(y7:list2+?)[01}?{${.2a,@(y3:id?),@(y6:andmap)[02}}{f}?{.0d,.2"
  ",.2a,.5,.3,.2,.4,&4{%2${.2,@(y7:list1+?)[01}?{.0dg,:1gI=}{f}?{:0,.1d,:"
  "1,,#0.5,.1,:3,:2,&4{%3.0u?{${:1,.5,:0,@(y10:xform-body)[03},@(y15:synt"
  "ax-quote-id),l2]3}${.4,${:3,.7a,t,@(y5:xform)[03},.4a,@(y17:extend-xen"
  "v-local)[03},.2d,.2d,:2^[33}.!0.0^_1[23}.0,'(s33:invalid syntax-lambda"
  " application),@(y7:x-error)[22}]6}.0,'(y13:syntax-lambda)c,'(s27:impro"
  "per syntax-lambda body),@(y7:x-error)[32",

  "P", "xform-syntax-rules",
  "%2${.2,@(y7:list2+?)[01}?{${.2a,@(y3:id?)[01}?{${.2da,@(y3:id?),@(y6:a"
  "ndmap)[02}}{f}}{f}?{.0dd,.1da,.2a,.4,@(y13:syntax-rules*)[24}${.2,@(y7"
  ":list1+?)[01}?{${.2a,@(y3:id?),@(y6:andmap)[02}}{f}?{.0d,.1a,f,.4,@(y1"
  "3:syntax-rules*)[24}.0,'(y12:syntax-rules)c,'(s26:improper syntax-rule"
  "s form),@(y7:x-error)[22",

  "P", "xform-syntax-length",
  "%2${.2,@(y6:list1?)[01}?{.0aL0}{f}?{.0ag,'(y5:quote),l2]2}.0,'(y13:syn"
  "tax-length)c,'(s27:improper syntax-length form),@(y7:x-error)[22",

  "P", "xform-syntax-error",
  "%2${.2,@(y17:xform-sexp->datum),@(y5:%25map1)[02},${.2,@(y7:list1+?)[0"
  "1}?{.0aS0}{f}?{.0,@(y7:x-error),@(y13:apply-to-list)[32}.1,'(y12:synta"
  "x-error)c,'(s26:improper syntax-error form),@(y7:x-error)[32",

  "P", "syntax-rules*",
  "%4,,,,,,,,#0#1#2#3#4#5#6#7.(i10),&1{%1:0,.1A0]1}.!0.0,&1{%1${.2,:0^[01"
  "}~]1}.!1.4,&1{%1.0p?{.0a,:0^[11}f]1}.!2&0{%1'(y3:...),.1zq]1}.!3.3,.9,"
  ".(i11),&3{%1:0?{:0,.1q]1}${.2,@(y3:id?)[01}?{${.2,:1,@(y8:xenv-ref)[02"
  "},:2^[11}f]1}.!4.2,&1{%3n,.2,.2,,#0:0,.1,.8,&3{%3${.2,@(y3:id?)[01}?{."
  "1?{${.2,:0[01}}{f}?{.2,.1c]3}.2]3}.0V0?{.2,.2,.2X0,:1^[33}.0p?{${.2d,:"
  "2^[01}?{${.4,.4,.4dd,:1^[03},t,.2a,:1^[33}${.4,.4,.4d,:1^[03},.2,.2a,:"
  "1^[33}.2]3}.!0.0^_1[33}.!5.5,.2,.4,.3,.(i12),&5{%3k3,.0,,#0.1,&1{%0f,:"
  "0[01}.!0n,.5,.5,,#0.4,.1,:4,:3,:2,:1,:0,.(i16),&8{%3,#0:7,.4,&2{%1.0?{"
  ":0]1}:1^[10}.!0${.3,@(y3:id?)[01}?{${.3,:2^[01}?{${.4,@(y3:id?)[01}?{$"
  "{.3,:1,@(y8:xenv-ref)[02},${.5,:0,@(y8:xenv-ref)[02}q}{f},.1^[41}.3,.3"
  ",.3cc]4}.1V0?{.2V0,.0?{.0}{${:7^[00}}_1.3,.3X0,.3X0,:6^[43}.1p~?{.2,.2"
  "e,.1^[41}${.3d,:3^[01}?{.1ddg,.3L0?{.3g}{${:7^[00}},.1,.1I-,.0<0?{${:7"
  "^[00}}.0,.6A6,.3,.7A8A6A8,${:4^,t,.(i10)a,:5^[03},,#0:6,.9,&2{%1${n,.3"
  ",:0a,:1^[03},@(y3:cdr),@(y5:%25map1)[12}.!0${.(i12),.6,.(i12)dd,:6^[03"
  "},${${.7,.6^,@(y5:%25map1)[02},.5c,@(y4:list)c,@(y4:%25map),@(y13:appl"
  "y-to-list)[02}L6](i11)}.2p?{${.5,.5d,.5d,:6^[03},.3a,.3a,:6^[43}:7^[40"
  "}.!0.0^_1[63}.!6.8,.2,.7,.5,&4{%3,,,#0#1#2,#0${${.(i10),&1{%1:0,.1A3~]"
  "1},t,.(i11),:1^[03},:3,.4,&2{%1${:0,&1{%0:0^]0},${.5,:1,@(y8:xenv-ref)"
  "[02},${.6,@(y7:id->sym)[01},@(y6:new-id)[03},.1c]1},@(y5:%25map1)[02}."
  "!0.0^_1.!0${:2^,f,.7,:1^[03}.!1.1,:1,&2{%1:1,&1{%1:0^,.1A0]1},t,.2,:0^"
  "[13}.!2.5,.5,,#0.8,.4,:0,.8,.4,&5{%2.0,,#0.0,.4,:0,:1,:2,:3,:4,&7{%1${"
  ".2,@(y3:id?)[01}?{:5,.1A3,.0?{.0}{:0,.2A3,.0?{.0}{:1^,.3A3}_1}_1d]1}.0"
  "V0?{${.2X0,:6^[01}X1]1}.0p?{${.2d,:2^[01}}{f}?{${.2a,:3^[01},,,#0#1:5,"
  "&1{%1:0,.1A3d]1}.!0.2,.4,:4,&3{%!0${.2,:2,@(y4:cons),@(y5:%25map2)[03}"
  ",:1a,:0^[12}.!1.2u?{${.5dd,:6^[01},${.6a,:6^[01}c]4}${.4,.3^,@(y5:%25m"
  "ap1)[02},${.6dd,:6^[01},${.3,.6^c,@(y4:%25map),@(y13:apply-to-list)[02"
  "}L6]5}.0p?{${.2d,:6^[01},${.3a,:6^[01}c]1}.0]1}.!0.0^_1[21}.!0.0^_1[62"
  "}.!7.(i11),.8,.8,&3{%2:2,,#0:0,.3,.5,:1,.4,&5{%1.0u?{${:3,'(s14:invali"
  "d syntax),@(y7:x-error)[02}}.0a,.0a,.1da,${:2,:3,.5,:4^[03},.0?{.0,.0,"
  ".3,.5,:1^[63}.4d,:0^[51}.!0.0^_1[21}](i12)",

  "P", "make-include-transformer",
  "%1,,,,#0#1#2#3&0{%2${.2,@(y6:list2?)[01}?{.0daS0}{f}~?{${.2,'(s14:inva"
  "lid syntax),@(y7:x-error)[02}}${.2da,@(y18:push-current-file!)[01}@(y8"
  ":begin-id),l1]2}.!0&0{%2${.2,@(y6:list1?)[01}~?{${.2,'(s14:invalid syn"
  "tax),@(y7:x-error)[02}}${@(y17:pop-current-file!)[00}@(y8:begin-id),l1"
  "]2}.!1${f,.3^b,'(y7:push-cf),@(y6:new-id)[03}.!2${f,.4^b,'(y6:pop-cf),"
  "@(y6:new-id)[03}.!3.2,.4,.6,&3{%2${.2,@(y7:list1+?)[01}~?{${.2,'(s14:i"
  "nvalid syntax),@(y7:x-error)[02}}n,.1d,,#0.3,:2,:1,.3,:0,&5{%2.0u?{${."
  "3A9,@(y7:%25append),@(y13:apply-to-list)[02},@(y8:begin-id)c]2}${.2a,@"
  "(y32:file-resolve-relative-to-current)[01},.0S0?{.0F0}{f},.0?{t}{${:4,"
  ".5a,'(s14:cannot include),@(y7:x-error)[03}},${:0,.5,@(y15:read-file-s"
  "exps)[02},n,n,:2^cc,.1L6,n,.5c,:3^cc,.6,.1c,.6d,:1^[72}.!0.0^_1[22}]5",

  "P", "preprocess-cond-expand",
  "%3,#0.0,.2,.5,&3{%3${'(y4:else),.3,:1[02}?{.1[30}${.2,@(y3:id?)[01}?{$"
  "{${.4,@(y7:id->sym)[01},@(y18:feature-available?)[01}?{.1[30}.2[30}${."
  "2,@(y6:list2?)[01}?{${'(y7:library),.3a,:1[02}}{f}?{${:0,${.5da,@(y17:"
  "xform-sexp->datum)[01},@(y18:library-available?)[02}?{.1[30}.2[30}${.2"
  ",@(y7:list1+?)[01}?{${'(y3:and),.3a,:1[02}}{f}?{.0du?{.1[30}.0ddu?{.2,"
  ".2,.2da,:2^[33}.2,.3,.3,.3,:2,&4{%0:3,:2,:1dd,:1ac,:0^[03},.2da,:2^[33"
  "}${.2,@(y7:list1+?)[01}?{${'(y2:or),.3a,:1[02}}{f}?{.0du?{.2[30}.0ddu?"
  "{.2,.2,.2da,:2^[33}.2,.2,.2,:2,&4{%0:3,:2,:1dd,:1ac,:0^[03},.2,.2da,:2"
  "^[33}${.2,@(y6:list2?)[01}?{${'(y3:not),.3a,:1[02}}{f}?{.1,.3,.2da,:2^"
  "[33}.0,'(s39:invalid cond-expand feature requirement),@(y7:x-error)[32"
  "}.!0${'(s26:invalid cond-expand syntax),'(l3:y4:<id>;l3:y1:*;y1:*;y3:."
  "..;;y3:...;),.6,@(y12:check-syntax)[03}.2d,,#0.0,.3,&2{%1.0u?{n]1}.0,:"
  "1,&2{%0:1d,:0^[01},.1,&1{%0:0ad]0},.2aa,:0^[13}.!0.0^_1[41",

  "P", "make-cond-expand-transformer",
  "%0&0{%2,#0.2,&1{%2${.2,@(y3:id?)[01}?{${.3,@(y16:root-environment),@(y"
  "8:xenv-ref)[02},${.3,:0,@(y8:xenv-ref)[02}q]2}f]2}.!0${.4,.4,.4^,@(y22"
  ":preprocess-cond-expand)[03},@(y8:begin-id)c]3}]0",

  "P", "adjoin-code",
  "%2'(l1:y5:begin;),.1e?{.1]2}'(l1:y5:begin;),.2e?{.0]2}${.2,'(l3:y5:beg"
  "in;y1:*;y3:...;),@(y11:sexp-match?)[02}?{${.3,'(l3:y5:begin;y1:*;y3:.."
  ".;),@(y11:sexp-match?)[02}}{f}?{.1d,.1dL6,'(y5:begin)c]2}${.2,'(l3:y5:"
  "begin;y1:*;y3:...;),@(y11:sexp-match?)[02}?{.1,l1,.1dL6,'(y5:begin)c]2"
  "}${.3,'(l3:y5:begin;y1:*;y3:...;),@(y11:sexp-match?)[02}?{.1d,.1c,'(y5"
  ":begin)c]2}.1,.1,'(y5:begin),l3]2",

  "P", "adjoin-eals",
  "%2.0u?{.1]2}${.3,.3d,@(y11:adjoin-eals)[02},.0,.2aaA3,.0?{.0,.0d,.4adq"
  "?{.2]5}.0,.4a,'(s38:multiple identifier bindings on import),@(y7:x-err"
  "or)[53}.1,.3ac]4",

  "P", "adjoin-esps",
  "%2.0u?{.1]2}${.3,.3d,@(y11:adjoin-esps)[02},.1a,.1,.1A2?{.1]4}.1,.1aA3"
  ",.0?{.0,.0,.3,'(s28:duplicate identifier exports),@(y7:x-error)[63}${."
  "4,.4d,@(y5:rassq)[02},.0?{.0,.0,.4,'(s30:conflicting identifier export"
  "s),@(y7:x-error)[73}.3,.3c]6",

  "P", "preprocess-import-sets",
  "%2,,#0#1&0{%1${.2,@(y6:list2?)[01}?{${.2a,@(y3:id?)[01}?{.0da,@(y3:id?"
  ")[11}f]1}f]1}.!0&0{%1${.2,@(y3:id?)[01},.0?{.0]2}.1I0]2}.!1${'(s21:inv"
  "alid import syntax),'(l3:y4:<id>;y1:*;y3:...;),.6,@(y12:check-syntax)["
  "03}.2a,${'(y4:only),.3,@(y12:id-rename-as)[02},${'(y6:except),.4,@(y12"
  ":id-rename-as)[02},${'(y6:rename),.5,@(y12:id-rename-as)[02},${'(y6:pr"
  "efix),.6,@(y12:id-rename-as)[02},${'(y7:library),.7,@(y12:id-rename-as"
  ")[02},,#0.(i10),.9,.3,.3,.(i11),.8,.8,.(i11),.(i13),&9{%2,#0${.3,@(y7:"
  "list2+?)[01}?{.1dap}{f}.!0.0^?{:0,.2aq?{${.3dd,@(y3:id?),@(y6:andmap)["
  "02}}{f}}{f}?{.1,.3,&2{%2${${:1dd,@(y7:id->sym),@(y5:%25map1)[02},.4,,#"
  "0.0,&1{%2.0u?{.0]2}.1,.1aaA0?{${.3,.3d,:0^[02},.1ac]2}.1,.1d,:0^[22}.!"
  "0.0^_1[02},.1,:0[22},.2da,:5^[32}.0^?{:1,.2aq?{${.3dd,@(y3:id?),@(y6:a"
  "ndmap)[02}}{f}}{f}?{.1,.3,&2{%2${${:1dd,@(y7:id->sym),@(y5:%25map1)[02"
  "},.4,,#0.0,&1{%2.0u?{.0]2}.1,.1aaA0?{.1,.1d,:0^[22}${.3,.3d,:0^[02},.1"
  "ac]2}.!0.0^_1[02},.1,:0[22},.2da,:5^[32}.0^?{:2,.2aq?{${.3d,@(y6:list2"
  "?)[01}?{${.3dda,@(y3:id?)[01}}{f}}{f}}{f}?{.1,.3,&2{%2${${:1dda,@(y7:i"
  "d->sym)[01},.4,,#0.0,&1{%2.0u?{.0]2}${.2aa,.4,@(y13:symbol-append)[02}"
  ",${.4,.4d,:0^[02},.2ad,.2cc]3}.!0.0^_1[02},.1,:0[22},.2da,:5^[32}.0^?{"
  ":3,.2aq?{${.3dd,:4^,@(y6:andmap)[02}}{f}}{f}?{.1,.3,&2{%2${${:1dd,@(y1"
  "7:xform-sexp->datum)[01},.4,,#0.0,&1{%2.0u?{.0]2}.1,.1aaA3,.0?{.0,${.5"
  ",.5d,:0^[02},.3ad,.2dacc]4}${.4,.4d,:0^[02},.2ac]3}.!0.0^_1[02},.1,:0["
  "22},.2da,:5^[32}${.3,@(y7:list2+?)[01}?{:6,.2aq}{f}?{${:8,.4,@(y18:pre"
  "process-library)[02},.0d,.1a,.5[42}${.3,@(y7:list1+?)[01}?{${.3,:7^,@("
  "y6:andmap)[02}}{f}?{${.3,@(y17:xform-sexp->datum)[01},${:8,.3,@(y9:xfo"
  "rm-ref)[02},.0V0~?{${.2,.4,'(s15:invalid library),@(y7:x-error)[03}}'1"
  ",.1V4,'0,.2V4,.6[52}.1,'(s28:invalid import set in import),@(y7:x-erro"
  "r)[32}.!0n,'(l1:y5:begin;),.(i11)d,,#0.0,.5,&2{%3.0u?{.2,.2c]3}.2,.2,."
  "2,:1,&4{%2${:3,.4,@(y11:adjoin-eals)[02},${.3,:2,@(y11:adjoin-code)[02"
  "},:1d,:0^[23},.1a,:0^[32}.!0.0^_1[(i11)3",

  "P", "preprocess-library-declarations",
  "%2${'(s35:invalid library declarations syntax),'(l3:y4:<id>;l3:y4:<id>"
  ";y1:*;y3:...;;y3:...;),.4,@(y12:check-syntax)[03}.0a,${'(y6:export),.3"
  ",@(y12:id-rename-as)[02},${'(y6:import),.4,@(y12:id-rename-as)[02},${'"
  "(y7:include),.5,@(y12:id-rename-as)[02},${'(y10:include-ci),.6,@(y12:i"
  "d-rename-as)[02},${'(y5:begin),.7,@(y12:id-rename-as)[02},${'(y6:renam"
  "e),.8,@(y12:id-rename-as)[02},${'(y11:cond-expand),.9,@(y12:id-rename-"
  "as)[02},${'(y7:push-cf),.(i10),@(y12:id-rename-as)[02},${'(y6:pop-cf),"
  ".(i11),@(y12:id-rename-as)[02},${'(y20:library-declarations),.(i12),@("
  "y12:id-rename-as)[02},${'(y28:include-library-declarations),.(i13),@(y"
  "12:id-rename-as)[02},${f,${f,@(y24:make-include-transformer)[01}b,'(y7"
  ":include),@(y6:new-id)[03},${f,${t,@(y24:make-include-transformer)[01}"
  "b,'(y10:include-ci),@(y6:new-id)[03},,#0.0,.9,&2{%2.0u?{.1A9]2}${.2a,@"
  "(y3:id?)[01}?{${.2a,@(y7:id->sym)[01},${.4,.3,.4c,l1,@(y11:adjoin-esps"
  ")[02},.2d,:1^[32}${.2a,'(l3:y4:<id>;y4:<id>;y4:<id>;),@(y11:sexp-match"
  "?)[02}?{:0,.1aaq}{f}?{${.3,${.5adda,@(y7:id->sym)[01},${.6ada,@(y7:id-"
  ">sym)[01}c,l1,@(y11:adjoin-esps)[02},.1d,:1^[22}${.2a,@(y17:xform-sexp"
  "->datum)[01},'(s27:invalid export spec element),@(y7:x-error)[22}.!0n,"
  "n,n,'(l1:y5:begin;),.(i19)d,,#0.(i19),.7,.(i20),.(i13),.(i17),.(i25),."
  "(i28),.(i16),.(i29),.(i21),.(i21),.(i28),.(i20),.(i29),.(i21),.(i30),."
  "(i16),&(i17){%5.0u?{.4,.4,.4,.4,l4]5}.0d,.1a,:(i16),.1aq?{.6,${.8,${n,"
  ".7d,:(i15)^[02},@(y11:adjoin-esps)[02},.6,.6,.5,:0^[75}${.2,@(y6:list2"
  "?)[01}?{:(i14),.1aq?{:(i14),.1daq}{f}}{f}?{@(y18:import-transformer)b,"
  "'(y6:import)c,l1,.7,.7,${.9,.5,@(y11:adjoin-eals)[02},.7,.6,:0^[85}:(i"
  "14),.1aq?{${:(i10),.3,@(y22:preprocess-import-sets)[02},.0d,.1a,.9,.9,"
  "${.(i11),.6,@(y11:adjoin-eals)[02},${.5,.(i12),@(y11:adjoin-code)[02},"
  ".8,:0^[(i10)5}:(i13),.1aq?{.6,.6,.6,.6,.5,.5dL6,:0^[75}:(i12),.1aq?{:("
  "i11),&1{%2${.2,@(y3:id?)[01}?{${.3,:0,@(y12:id-rename-as)[02},.1q]2}f]"
  "2},.7,.7,.7,.7,:(i10),.7L6,${.8d,.8,@(y22:preprocess-cond-expand)[02}L"
  "6,:0^[85}:7,.1aq?{${'(s35:invalid library declarations syntax),'(l2:y4"
  ":<id>;y8:<string>;),.4,@(y12:check-syntax)[03}${.2da,@(y18:push-curren"
  "t-file!)[01}.6,.6,.6,.6,.5,:0^[75}:6,.1aq?{${'(s35:invalid library dec"
  "larations syntax),'(l1:y4:<id>;),.4,@(y12:check-syntax)[03}${@(y17:pop"
  "-current-file!)[00}.6,.6,.6,.6,.5,:0^[75}:9,.1aq?{${'(s43:invalid incl"
  "ude-library-declarations syntax),'(l3:y4:<id>;y8:<string>;y3:...;),.4,"
  "@(y12:check-syntax)[03}.1,.1dA8,,#0:8,:7,:6,.3,:0,.(i11),.(i13),.(i15)"
  ",.(i17),&9{%2.0u?{:0,:1,:2,:3,.5,:4^[25}${.2a,@(y32:file-resolve-relat"
  "ive-to-current)[01},.0S0?{.0F0}{f},.0?{t}{${:8,.5a,'(s27:cannot includ"
  "e declarations),@(y7:x-error)[03}},${f,.5,@(y15:read-file-sexps)[02},."
  "5,n,:6cc,.1L6,n,.5c,:7cc,.5d,:5^[62}.!0.0^_1[72}:5,.1aq?{${'(s42:inval"
  "id include library declaration syntax),'(l3:y4:<id>;y8:<string>;y3:..."
  ";),.4,@(y12:check-syntax)[03}n,.1d,:4cc,.7L6,.6,.6,.6,.5,:0^[75}:3,.1a"
  "q?{${'(s45:invalid include-ci library declaration syntax),'(l3:y4:<id>"
  ";y8:<string>;y3:...;),.4,@(y12:check-syntax)[03}n,.1d,:2cc,.7L6,.6,.6,"
  ".6,.5,:0^[75}:1,.1aq?{${.2d,@(y17:xform-sexp->datum)[01},.7L6,.6,.6,.6"
  ",.5,:0^[75}f]7}.!0.0^_1[(i17)5",

  "P", "preprocess-library",
  "%2,#0.1,&1{%1${:0,@(y7:list2+?)[01}?{${:0da,@(y3:id?)[01}}{f}?{${.2,@("
  "y7:id->sym)[01},${:0da,@(y7:id->sym)[01},@(y37:fully-qualified-library"
  "-prefixed-name)[12}${.2,@(y7:id->sym)[01},@(y6:gensym)[11}.!0${'(s22:i"
  "nvalid library syntax),'(l3:y1:*;y1:*;y3:...;),.5,@(y12:check-syntax)["
  "03}${.3,@(y7:list2+?)[01}?{${.3da,@(y3:id?)[01}}{f}?{${.3da,@(y7:id->s"
  "ym)[01}}{f},.0?{.2dd}{.2d},${.6,.3,.7ac,@(y31:preprocess-library-decla"
  "rations)[02},.0a,.1da,.2dda,.3ddda,${.(i11),.(i10)^,.6,@(y27:make-cont"
  "rolled-environment)[03},n,,#0.(i12),.1,.4,&3{%2.0u?{.1]2}.0d,.1a,.0p?{"
  ".0a,.1d,${:0,.4,t,@(y5:xform)[03},'(y5:begin),.1q?{.1L0~?{${.5,'(s19:i"
  "mproper begin form),@(y7:x-error)[02}}.6,.5,.3L6,:1^[72}'(y6:define),."
  "1q?{${.3,@(y6:list2?)[01}?{.1au}{f}}{f}?{.6,.5,.3daL6,:1^[72}'(y6:defi"
  "ne),.1q?{${:0,.4,@(y12:xform-define)[02},${'(y6:define),.3da,:0,@(y11:"
  "xenv-lookup)[03},.0Y2~?{${.7,.4da,'(s24:unexpected define for id),@(y7"
  ":x-error)[03}}.8,.2dda,.2zda,'(y4:set!),l3c,.7,:1^[92}'(y13:define-syn"
  "tax),.1q?{${:0,.4,@(y19:xform-define-syntax)[02},${'(y13:define-syntax"
  "),.3da,:0,@(y11:xenv-lookup)[03},.0Y2~?{${.7,.4da,'(s31:unexpected def"
  "ine-syntax for id),@(y7:x-error)[03}}.1dda,.1sz.8,.7,:1^[92}'(y14:defi"
  "ne-library),.1q?{${f,:2,.5,.7,@(y20:xform-define-library)[04},${'(y13:"
  "define-syntax),.3da,:2,@(y11:xenv-lookup)[03},.0Y2~?{${.7,.4da,'(s32:u"
  "nexpected define-library for id),@(y7:x-error)[03}}.1dda,.1sz.8,.7,:1^"
  "[92}'(y6:import),.1q?{${f,:0,.5,.7,@(y12:xform-import)[04},.0da,'0,.1V"
  "4,'1,.2V4,${'(y6:import),.3,:0[02}~?{${.9,'(s33:broken import inside l"
  "ibrary code),@(y7:x-error)[02}}.(i10),.2c,.9,:1^[(i11)2}.0K0?{.6,.5,${"
  ":0,.8,.6[02}c,:1^[72}.0U0?{.6,${:0,.5,.5,@(y16:xform-integrable)[03}c,"
  ".5,:1^[72}.6,${:0,.7,f,@(y5:xform)[03}c,.5,:1^[72}.3,${:0,.4,f,@(y5:xf"
  "orm)[03}c,.2,:1^[42}.!0${n,.6,.4^[02},.0A9,'(y5:begin)c,${.(i13)?{.2,."
  "(i14),'(y4:once),l3}{.2},.(i11),@(y11:adjoin-code)[02},.4,.8,,#0.8,.1,"
  ".5,&3{%2.0u?{.1A9,:0c]2}.0aa,.1ad,${'(y3:ref),.4,:2[02},.0~?{.2,'(s16:"
  "cannot export id),@(y7:x-error)[52}${.2,@(y17:location-special?)[01}?{"
  ".4,.1,.3cc,.4d,:1^[52}.0z,.0p~,.0?{.0}{'(l2:y3:ref;y5:const;),.2aA0}_1"
  "?{.5,.2,.4cc,.5d,:1^[62}.0,.4,'(s27:cannot export code alias id),@(y7:"
  "x-error)[63}.!0.0^_1[(i16)2",

  "P", "xform-define-library",
  "%4${.3,@(y7:list2+?)[01}?{${.3a,@(y9:listname?)[01}}{f}?{${.3a,@(y17:x"
  "form-sexp->datum)[01},.4?{${.2,@(y16:listname->symbol)[01}}{f},.0?{.3d"
  ",.1c}{.3d},.3c,${.7,.3,@(y18:preprocess-library)[02},.0d,.1a,V12,.4,'("
  "y14:define-library),l3]8}.1,.1c,'(s28:improper define-library form),@("
  "y7:x-error)[42",

  "P", "xform-import",
  "%4.1L0?{${.4,.4,.4c,@(y22:preprocess-import-sets)[02},.0d,.1a,V12,'(y6"
  ":import),l2]5}.1,.1c,'(s20:improper import form),@(y7:x-error)[42",

  "P", "write-serialized-char",
  "%2'(c%25),.1C=,.0?{.0}{'(c%22),.2C=,.0?{.0}{'(c%5c),.3C=,.0?{.0}{'(c )"
  ",.4C<,.0?{.0}{'(c~),.5C>}_1}_1}_1}_1?{.1,'(c%25)W0'(i16),.1X8X6,'1,.1S"
  "3I=?{.2,'(c0)W0}.2,.1,@(y12:write-string)[32}.1,.1W0]2",

  "P", "write-serialized-byte",
  "%2'(i16),.1X6,'1,.1S3I=?{.2,'(c0)W0}.2,.1,@(y12:write-string)[32",

  "P", "write-serialized-size",
  "%2${.3,'(i10),.4X6,@(y12:write-string)[02}.1,'(c:)W0]2",

  "P", "write-serialized-element",
  "%2${.3,.3,@(y21:write-serialized-sexp)[02}.1,'(c;)W0]2",

  "P", "write-serialized-sexp",
  "%2f,.1q?{.1,'(cf)W0]2}t,.1q?{.1,'(ct)W0]2}n,.1q?{.1,'(cn)W0]2}.0C0?{.1"
  ",'(cc)W0.1,.1,@(y21:write-serialized-char)[22}.0N0?{.1,.1%nI0?{'(ci)}{"
  "'(cj)}W0.1,'(i10),.2E8,@(y12:write-string)[22}.0L0?{.1,'(cl)W0${.3,.3g"
  ",@(y21:write-serialized-size)[02}.0,,#0.0,.4,&2{%1.0u?{]1}${:0,.3a,@(y"
  "24:write-serialized-element)[02}.0d,:1^[11}.!0.0^_1[21}.0p?{.1,'(cp)W0"
  "${.3,.3a,@(y24:write-serialized-element)[02}.1,.1d,@(y24:write-seriali"
  "zed-element)[22}.0V0?{.1,'(cv)W0${.3,.3V3,@(y21:write-serialized-size)"
  "[02}'0,,#0.0,.3,.5,&3{%1:1V3,.1I=?{]1}${:0,.3,:1V4,@(y24:write-seriali"
  "zed-element)[02}'1,.1I+,:2^[11}.!0.0^_1[21}.0S0?{.1,'(cs)W0${.3,.3S3,@"
  "(y21:write-serialized-size)[02}'0,,#0.0,.3,.5,&3{%1:1S3,.1I=?{]1}${:0,"
  ".3,:1S4,@(y21:write-serialized-char)[02}'1,.1I+,:2^[11}.!0.0^_1[21}.0B"
  "0?{.1,'(cb)W0${.3,.3B3,@(y21:write-serialized-size)[02}'0,,#0.0,.3,.5,"
  "&3{%1:1B3,.1I=?{]1}${:0,.3,:1B4,@(y21:write-serialized-byte)[02}'1,.1I"
  "+,:2^[11}.!0.0^_1[21}.0Y0?{.1,'(cy)W0.0X4,${.4,.3S3,@(y21:write-serial"
  "ized-size)[02}'0,,#0.0,.3,.6,&3{%1:1S3,.1I=?{]1}${:0,.3,:1S4,@(y21:wri"
  "te-serialized-char)[02}'1,.1I+,:2^[11}.!0.0^_1[31}.0Y2?{.1,'(cz)W0.1,."
  "1z,@(y24:write-serialized-element)[22}.0,'(s21:cannot encode literal),"
  "@(y7:c-error)[22",

  "P", "write-serialized-arg",
  "%2.0N0?{.0%nI0?{.0,'0I>!?{'9,.1I>!}{f}}{f}}{f}?{.1,.1,'(s10:0123456789"
  ")S4W0]2}.1,'(c()W0${.3,.3,@(y21:write-serialized-sexp)[02}.1,'(c))W0]2",

  "P", "c-error",
  "%!1.0,.2,'(s10:compiler: )S6,@(y6:error*)[22",

  "P", "c-warning",
  "%!1.0,.2,'(s10:compiler: )S6,@(y8:warning*)[22",

  "P", "find-free*",
  "%2.0u?{n]2}${.3,.3d,@(y10:find-free*)[02},${.4,.4a,@(y9:find-free)[02}"
  ",@(y9:set-union)[22",

  "P", "find-free",
  "%2'(y5:quote),.1aq?{.0d,&0{%1n]1},@(y13:apply-to-list)[22}'(y4:gref),."
  "1aq?{.0d,&0{%1n]1},@(y13:apply-to-list)[22}'(y5:gset!),.1aq?{.0d,.2,&1"
  "{%2:0,.2,@(y9:find-free)[22},@(y13:apply-to-list)[22}'(l2:y3:ref;y5:co"
  "nst;),.1aA0?{.0d,.2,&1{%1${:0,.3,@(y11:set-member?)[02}?{n]1}.0,l1]1},"
  "@(y13:apply-to-list)[22}'(y4:set!),.1aq?{.0d,.2,&1{%2${:0,.4,@(y9:find"
  "-free)[02},${:0,.4,@(y11:set-member?)[02}?{n}{.1,l1},@(y9:set-union)[2"
  "2},@(y13:apply-to-list)[22}'(y4:set&),.1aq?{.0d,.2,&1{%1${:0,.3,@(y11:"
  "set-member?)[02}?{n]1}.0,l1]1},@(y13:apply-to-list)[22}'(y6:lambda),.1"
  "aq?{.0d,.2,&1{%2${:0,${.5,@(y15:flatten-idslist)[01},@(y9:set-union)[0"
  "2},.2,@(y9:find-free)[22},@(y13:apply-to-list)[22}'(y7:lambda*),.1aq?{"
  ".0d,.2,&1{%!0:0,${.3,@(y4:cadr),@(y5:%25map1)[02},@(y10:find-free*)[12"
  "},@(y13:apply-to-list)[22}'(y5:letcc),.1aq?{.0d,.2,&1{%2${:0,.3,l1,@(y"
  "9:set-union)[02},.2,@(y9:find-free)[22},@(y13:apply-to-list)[22}'(y6:w"
  "ithcc),.1aq?{.0d,.2,&1{%2${:0,.4,@(y9:find-free)[02},${:0,.4,@(y9:find"
  "-free)[02},@(y9:set-union)[22},@(y13:apply-to-list)[22}'(y2:if),.1aq?{"
  ".0d,.2,&1{%3${${:0,.7,@(y9:find-free)[02},${:0,.7,@(y9:find-free)[02},"
  "@(y9:set-union)[02},${:0,.4,@(y9:find-free)[02},@(y9:set-union)[32},@("
  "y13:apply-to-list)[22}'(y5:begin),.1aq?{.0d,.2,&1{%!0:0,.1,@(y10:find-"
  "free*)[12},@(y13:apply-to-list)[22}'(y10:integrable),.1aq?{.0d,.2,&1{%"
  "!1:0,.1,@(y10:find-free*)[22},@(y13:apply-to-list)[22}'(y4:call),.1aq?"
  "{.0d,.2,&1{%!1${:0,.3,@(y10:find-free*)[02},${:0,.5,@(y9:find-free)[02"
  "},@(y9:set-union)[22},@(y13:apply-to-list)[22}'(y3:asm),.1aq?{.0d,&0{%"
  "1n]1},@(y13:apply-to-list)[22}'(y4:once),.1aq?{.0d,.2,&1{%2:0,.2,@(y9:"
  "find-free)[22},@(y13:apply-to-list)[22}'(l4:y6:define;y13:define-synta"
  "x;y14:define-library;y6:import;),.1aA0?{.0d,.1,&1{%!0:0,'(s25:misplace"
  "d definition form),@(y7:c-error)[12},@(y13:apply-to-list)[22}.0,'(s22:"
  "unexpected <core> form),@(y7:c-error)[22",

  "P", "find-sets*",
  "%2.0u?{n]2}${.3,.3d,@(y10:find-sets*)[02},${.4,.4a,@(y9:find-sets)[02}"
  ",@(y9:set-union)[22",

  "P", "find-sets",
  "%2'(y5:quote),.1aq?{.0d,&0{%1n]1},@(y13:apply-to-list)[22}'(y4:gref),."
  "1aq?{.0d,&0{%1n]1},@(y13:apply-to-list)[22}'(y5:gset!),.1aq?{.0d,.2,&1"
  "{%2:0,.2,@(y9:find-sets)[22},@(y13:apply-to-list)[22}'(l2:y3:ref;y5:co"
  "nst;),.1aA0?{.0d,&0{%1n]1},@(y13:apply-to-list)[22}'(y4:set!),.1aq?{.0"
  "d,.2,&1{%2${:0,.4,@(y9:find-sets)[02},${:0,.4,@(y11:set-member?)[02}?{"
  ".1,l1}{n},@(y9:set-union)[22},@(y13:apply-to-list)[22}'(y4:set&),.1aq?"
  "{.0d,.2,&1{%1${:0,.3,@(y11:set-member?)[02}?{.0,l1]1}n]1},@(y13:apply-"
  "to-list)[22}'(y6:lambda),.1aq?{.0d,.2,&1{%2${${.4,@(y15:flatten-idslis"
  "t)[01},:0,@(y9:set-minus)[02},.2,@(y9:find-sets)[22},@(y13:apply-to-li"
  "st)[22}'(y7:lambda*),.1aq?{.0d,.2,&1{%!0:0,${.3,@(y4:cadr),@(y5:%25map"
  "1)[02},@(y10:find-sets*)[12},@(y13:apply-to-list)[22}'(y5:letcc),.1aq?"
  "{.0d,.2,&1{%2${.2,l1,:0,@(y9:set-minus)[02},.2,@(y9:find-sets)[22},@(y"
  "13:apply-to-list)[22}'(y6:withcc),.1aq?{.0d,.2,&1{%2${:0,.4,@(y9:find-"
  "sets)[02},${:0,.4,@(y9:find-sets)[02},@(y9:set-union)[22},@(y13:apply-"
  "to-list)[22}'(y5:begin),.1aq?{.0d,.2,&1{%!0:0,.1,@(y10:find-sets*)[12}"
  ",@(y13:apply-to-list)[22}'(y2:if),.1aq?{.0d,.2,&1{%3${${:0,.7,@(y9:fin"
  "d-sets)[02},${:0,.7,@(y9:find-sets)[02},@(y9:set-union)[02},${:0,.4,@("
  "y9:find-sets)[02},@(y9:set-union)[32},@(y13:apply-to-list)[22}'(y10:in"
  "tegrable),.1aq?{.0d,.2,&1{%!1:0,.1,@(y10:find-sets*)[22},@(y13:apply-t"
  "o-list)[22}'(y4:call),.1aq?{.0d,.2,&1{%!1${:0,.3,@(y10:find-sets*)[02}"
  ",${:0,.5,@(y9:find-sets)[02},@(y9:set-union)[22},@(y13:apply-to-list)["
  "22}'(y3:asm),.1aq?{.0d,&0{%1n]1},@(y13:apply-to-list)[22}'(y4:once),.1"
  "aq?{.0d,.2,&1{%2:0,.2,@(y9:find-sets)[22},@(y13:apply-to-list)[22}'(l4"
  ":y6:define;y13:define-syntax;y14:define-library;y6:import;),.1aA0?{.0d"
  ",.1,&1{%!0:0,'(s25:misplaced definition form),@(y7:c-error)[12},@(y13:"
  "apply-to-list)[22}.0,'(s22:unexpected <core> form),@(y7:c-error)[22",

  "P", "codegen",
  "%7'(y5:quote),.1aq?{.0d,.6,.8,&2{%1.0,t,.1v?{:0,'(ct)W0}{f,.1v?{:0,'(c"
  "f)W0}{n,.1v?{:0,'(cn)W0}{:0,'(c')W0${:0,.4,@(y20:write-serialized-arg)"
  "[02}}}}_1:1?{:0,'(c])W0:0,:1,@(y20:write-serialized-arg)[12}]1},@(y13:"
  "apply-to-list)[72}'(y4:gref),.1aq?{.0d,.6,.8,&2{%1:0,'(c@)W0${:0,.3,@("
  "y20:write-serialized-arg)[02}:1?{:0,'(c])W0:0,:1,@(y20:write-serialize"
  "d-arg)[12}]1},@(y13:apply-to-list)[72}'(y5:gset!),.1aq?{.0d,.6,.8,.4,."
  "6,.8,.(i10),&6{%2${:4,f,:0,:1,:2,:3,.9,@(y7:codegen)[07}:4,'(c@)W0:4,'"
  "(c!)W0${:4,.3,@(y20:write-serialized-arg)[02}:5?{:4,'(c])W0:4,:5,@(y20"
  ":write-serialized-arg)[22}]2},@(y13:apply-to-list)[72}'(l2:y3:ref;y5:c"
  "onst;),.1aA0?{.0d,.6,.8,.5,.7,.6,&5{%1${:0,.3,@(y4:posq)[02},.0?{.0,:3"
  ",'(c.)W0${:3,.3,@(y20:write-serialized-arg)[02}${:1,.5,@(y11:set-membe"
  "r?)[02}?{:3,'(c^)W0}_1}{${:2,.4,@(y4:posq)[02},.0?{.0,:3,'(c:)W0${:3,."
  "3,@(y20:write-serialized-arg)[02}${:1,.6,@(y11:set-member?)[02}?{:3,'("
  "c^)W0}_1}{:3,'(c@)W0${:3,.5,@(y20:write-serialized-arg)[02}}_1}_1:4?{:"
  "3,'(c])W0:3,:4,@(y20:write-serialized-arg)[12}]1},@(y13:apply-to-list)"
  "[72}'(y4:set!),.1aq?{.0d,.6,.8,.5,.5,.8,.(i10),&6{%2${:4,f,:0,:1,:3,:2"
  ",.9,@(y7:codegen)[07}${:2,.3,@(y4:posq)[02},.0?{.0,:4,'(c.)W0:4,'(c!)W"
  "0${:4,.3,@(y20:write-serialized-arg)[02}_1}{${:3,.4,@(y4:posq)[02},.0?"
  "{.0,:4,'(c:)W0:4,'(c!)W0${:4,.3,@(y20:write-serialized-arg)[02}_1}{:4,"
  "'(c@)W0:4,'(c!)W0${:4,.5,@(y20:write-serialized-arg)[02}}_1}_1:5?{:4,'"
  "(c])W0:4,:5,@(y20:write-serialized-arg)[22}]2},@(y13:apply-to-list)[72"
  "}'(y4:set&),.1aq?{.0d,.6,.8,.5,.5,&4{%1${:0,.3,@(y4:posq)[02},.0?{.0,:"
  "2,'(c.)W0${:2,.3,@(y20:write-serialized-arg)[02}_1}{${:1,.4,@(y4:posq)"
  "[02},.0?{.0,:2,'(c:)W0${:2,.3,@(y20:write-serialized-arg)[02}_1}{:2,'("
  "c`)W0${:2,.5,@(y20:write-serialized-arg)[02}}_1}_1:3?{:2,'(c])W0:2,:3,"
  "@(y20:write-serialized-arg)[12}]1},@(y13:apply-to-list)[72}'(y5:begin)"
  ",.1aq?{.0d,.6,.8,.4,.6,.8,.(i10),&6{%!0${.2,,#0.0,:3,:2,:1,:0,:4,:5,&7"
  "{%1.0p?{.0dp?{f}{:0},${:1,.3,:2,:3,:4,:5,.9a,@(y7:codegen)[07}.1d,:6^["
  "21}]1}.!0.0^_1[01}:5?{.0u}{f}?{:4,'(c])W0:4,:5,@(y20:write-serialized-"
  "arg)[12}]1},@(y13:apply-to-list)[72}'(y2:if),.1aq?{.0d,.7,.3,.5,.7,.9,"
  ".(i11),&6{%3${:5,f,:1,:2,:3,:4,.8,@(y7:codegen)[07}:5,'(c?)W0:5,'(c{)W"
  "0${:5,:0,:1,:2,:3,:4,.9,@(y7:codegen)[07}:5,'(c})W0:0?{:5,:0,:1,:2,:3,"
  ":4,.8,@(y7:codegen)[37}'(l1:y5:begin;),.3e,.0?{.0]4}:5,'(c{)W0${:5,:0,"
  ":1,:2,:3,:4,.(i11),@(y7:codegen)[07}:5,'(c})W0]4},@(y13:apply-to-list)"
  "[72}'(y6:lambda),.1aq?{.0d,.6,.8,.7,.7,.7,.7,&6{%2${.2,@(y15:flatten-i"
  "dslist)[01},${:3,${.5,.8,@(y9:find-free)[02},@(y9:set-minus)[02},${.3,"
  ".6,@(y9:find-sets)[02},${:0,.4A8,,#0.0,:4,:1,:3,&4{%2.0u?{]2}${:2,f,:0"
  ",n,:1,.8,.8a,'(y3:ref),l2,@(y7:codegen)[07}:2,'(c,)W0.1,fc,.1d,:3^[22}"
  ".!0.0^_1[02}:4,'(c&)W0${:4,.4g,@(y20:write-serialized-arg)[02}:4,'(c{)"
  "W0.3L0?{:4,'(c%25)W0${:4,.6g,@(y20:write-serialized-arg)[02}}{:4,'(c%2"
  "5)W0:4,'(c!)W0${:4,${.8,@(y17:idslist-req-count)[01},@(y20:write-seria"
  "lized-arg)[02}}${'0,.5,,#0.0,.6,:4,&3{%2.0u?{]2}${:1,.3a,@(y11:set-mem"
  "ber?)[02}?{:0,'(c#)W0${:0,.4,@(y20:write-serialized-arg)[02}}'1,.2I+,."
  "1d,:2^[22}.!0.0^_1[02}${:4,.5g,:3,${${.(i10),:2,@(y13:set-intersect)[0"
  "2},.8,@(y9:set-union)[02},.7,.9,.(i12),@(y7:codegen)[07}:4,'(c})W0_1_1"
  "_1:5?{:4,'(c])W0:4,:5,@(y20:write-serialized-arg)[22}]2},@(y13:apply-t"
  "o-list)[72}'(y7:lambda*),.1aq?{.0d,.6,.8,.5,.7,.9,.7,&6{%!0${:0,.3A8,,"
  "#0.0,:4,:3,:2,:1,&5{%2.0u?{]2}${:3,f,:0,:1,:2,.8,.8ada,@(y7:codegen)[0"
  "7}:3,'(c%25)W0:3,'(cx)W0:3,'(c,)W0.1,fc,.1d,:4^[22}.!0.0^_1[02}:4,'(c&"
  ")W0${:4,.3g,@(y20:write-serialized-arg)[02}:4,'(c{)W0${'0,.3,,#0.0,:4,"
  "&2{%2.0u?{]2}.0aa,.0a,.1da,:0,'(c|)W0.0?{:0,'(c!)W0}${:0,.4,@(y20:writ"
  "e-serialized-arg)[02}${:0,.7,@(y20:write-serialized-arg)[02}_1_1_1'1,."
  "2I+,.1d,:1^[22}.!0.0^_1[02}:4,'(c%25)W0:4,'(c%25)W0:4,'(c})W0:5?{:4,'("
  "c])W0:4,:5,@(y20:write-serialized-arg)[12}]1},@(y13:apply-to-list)[72}"
  "'(y5:letcc),.1aq?{.0d,.4,.7,.7,.6,.6,.(i12),&6{%2.0,l1,${.2,.5,@(y9:fi"
  "nd-sets)[02},${.2,${.6,:5,@(y9:set-minus)[02},@(y9:set-union)[02},:4?{"
  ":0,'(ck)W0${:0,:4,@(y20:write-serialized-arg)[02}:0,'(c,)W0${.3,.6,@(y"
  "11:set-member?)[02}?{:0,'(c#)W0:0,'(c0)W0}:0,'1,:4I+,:3,.3,:2,:1,.9c,."
  "(i10),@(y7:codegen)[57}:0,'(c$)W0:0,'(c{)W0:0,'(ck)W0:0,'(c0)W0:0,'(c,"
  ")W0${.3,.6,@(y11:set-member?)[02}?{:0,'(c#)W0:0,'(c0)W0}${:0,f,:3,.5,:"
  "2,:1,fc,fc,.(i11)c,.(i12),@(y7:codegen)[07}:0,'(c_)W0${:0,'3,@(y20:wri"
  "te-serialized-arg)[02}:0,'(c})W0]5},@(y13:apply-to-list)[72}'(y6:withc"
  "c),.1aq?{.0d,.7,.3,.5,.7,.9,&5{%2'(l3:y5:quote;y3:ref;y6:lambda;),.2aA"
  "0?{${:4,f,:0,:1,:2,:3,.9,@(y7:codegen)[07}:4,'(c,)W0${:4,f,:0,:1,:2,:3"
  ",fc,.8,@(y7:codegen)[07}:4,'(cw)W0:4,'(c!)W0]2}${:4,f,:0,:1,:2,:3,.9,n"
  ",'(y6:lambda),l3,@(y7:codegen)[07}:4,'(c,)W0${:4,f,:0,:1,:2,:3,fc,.8,@"
  "(y7:codegen)[07}:4,'(cw)W0]2},@(y13:apply-to-list)[72}'(y10:integrable"
  "),.1aq?{.0d,.6,.8,.5,.7,.9,.7,&6{%!1'0,.2U8,.2U6,.0,'(l4:c0;c1;c2;c3;)"
  ",.1A1?{${:0,.6A8,,#0.0,:4,:3,:2,:1,&5{%2.0u?{]2}${:3,f,:0,:1,:2,.8,.8a"
  ",@(y7:codegen)[07}.0du~?{:3,'(c,)W0}.1,fc,.1d,:4^[22}.!0.0^_1[02}${:4,"
  ".5,@(y12:write-string)[02}}{'(cp),.1v?{.3u?{'1,.5U8,${:4,.3,@(y12:writ"
  "e-string)[02}_1}{'1,.4gI-,${:0,.7A8,,#0.0,:4,:3,:2,:1,&5{%2.0u?{]2}${:"
  "3,f,:0,:1,:2,.8,.8a,@(y7:codegen)[07}.0du~?{:3,'(c,)W0}.1,fc,.1d,:4^[2"
  "2}.!0.0^_1[02}${'0,,#0.0,.8,:4,.7,&4{%1:0,.1I<!?{]1}${:1,:2,@(y12:writ"
  "e-string)[02}'1,.1I+,:3^[11}.!0.0^_1[01}_1}}{'(cm),.1v?{.3du?{'1,.5U8,"
  "${:4,f,:1,:2,:3,:0,.(i12)a,@(y7:codegen)[07}${:4,.3,@(y12:write-string"
  ")[02}_1}{'1,.4gI-,${:0,.7A8,,#0.0,:4,:3,:2,:1,&5{%2.0u?{]2}${:3,f,:0,:"
  "1,:2,.8,.8a,@(y7:codegen)[07}.0du~?{:3,'(c,)W0}.1,fc,.1d,:4^[22}.!0.0^"
  "_1[02}${'0,,#0.0,.8,:4,.7,&4{%1:0,.1I<!?{]1}${:1,:2,@(y12:write-string"
  ")[02}'1,.1I+,:3^[11}.!0.0^_1[01}_1}}{'(cc),.1v?{.3A8,'1,.5gI-,${:4,f,:"
  "1,:2,:3,:0,.9a,@(y7:codegen)[07}:4,'(c,)W0${:0,fc,.4d,,#0.0,:4,:3,:2,:"
  "1,&5{%2.0u?{]2}${:3,f,:0,:1,:2,.8,.8a,@(y7:codegen)[07}.0du~?{:3,'(c,)"
  "W0:3,'(c,)W0}.1,fc,fc,.1d,:4^[22}.!0.0^_1[02}${'0,,#0.0,.9,:4,.7,&4{%1"
  ":0,.1I<!?{]1}.0I=0~?{:1,'(c;)W0}${:1,:2,@(y12:write-string)[02}'1,.1I+"
  ",:3^[11}.!0.0^_1[01}_2}{'(cx),.1v?{'1,.4gI-,${:0,.7A8,,#0.0,:4,:3,:2,:"
  "1,&5{%2.0u?{]2}${:3,f,:0,:1,:2,.8,.8a,@(y7:codegen)[07}.0du~?{:3,'(c,)"
  "W0}.1,fc,.1d,:4^[22}.!0.0^_1[02}${'0,,#0.0,.8,:4,.7,&4{%1:0,.1I<!?{]1}"
  "${:1,:2,@(y12:write-string)[02}'1,.1I+,:3^[11}.!0.0^_1[01}_1}{'(cu),.1"
  "v?{.3u?{${:4,'1,.8U8,@(y12:write-string)[02}}{${:4,f,:1,:2,:3,:0,.(i11"
  ")a,@(y7:codegen)[07}}${:4,.5,@(y12:write-string)[02}}{'(cb),.1v?{.3du?"
  "{${:4,'1,.8U8,@(y12:write-string)[02}}{${:4,f,:1,:2,:3,:0,.(i11)da,@(y"
  "7:codegen)[07}}:4,'(c,)W0${:4,f,:1,:2,:3,:0,fc,.(i11)a,@(y7:codegen)[0"
  "7}${:4,.5,@(y12:write-string)[02}}{'(ct),.1v?{.3ddu?{${:4,'1,.8U8,@(y1"
  "2:write-string)[02}}{${:4,f,:1,:2,:3,:0,.(i11)dda,@(y7:codegen)[07}}:4"
  ",'(c,)W0${:4,f,:1,:2,:3,:0,fc,.(i11)da,@(y7:codegen)[07}:4,'(c,)W0${:4"
  ",f,:1,:2,:3,:0,fc,fc,.(i11)a,@(y7:codegen)[07}${:4,.5,@(y12:write-stri"
  "ng)[02}}{'(c#),.1v?{${:0,.6A8,,#0.0,:4,:3,:2,:1,&5{%2.0u?{]2}${:3,f,:0"
  ",:1,:2,.8,.8a,@(y7:codegen)[07}:3,'(c,)W0.1,fc,.1d,:4^[22}.!0.0^_1[02}"
  "${:4,.5,@(y12:write-string)[02}${:4,.6g,@(y20:write-serialized-arg)[02"
  "}}{${.3,'(s27:unsupported integrable type),@(y7:c-error)[02}}}}}}}}}}_"
  "1_2:5?{:4,'(c])W0:4,:5,@(y20:write-serialized-arg)[22}]2},@(y13:apply-"
  "to-list)[72}'(y4:call),.1aq?{.0d,.7,.4,.6,.8,.6,.(i11),&6{%!1'(y6:lamb"
  "da),.2aq?{.1daL0?{.1dag,.1gI=}{f}}{f}?{${:1,.3A8,,#0.0,:5,:4,:3,:2,&5{"
  "%2.0u?{]2}${:3,f,:0,:1,:2,.8,.8a,@(y7:codegen)[07}:3,'(c,)W0.1,fc,.1d,"
  ":4^[22}.!0.0^_1[02}.1da,.2dda,${.3,.3,@(y9:find-sets)[02},${.2,${.7,:3"
  ",@(y9:set-minus)[02},@(y9:set-union)[02},:1,.4L6,${'0,.7,,#0.0,.8,:5,&"
  "3{%2.0u?{]2}${:1,.3a,@(y11:set-member?)[02}?{:0,'(c#)W0${:0,.4,@(y20:w"
  "rite-serialized-arg)[02}}'1,.2I+,.1d,:2^[22}.!0.0^_1[02}:0?{:5,.6g,:0I"
  "+,:2,.4,:4,.5,.9,@(y7:codegen)[77}${:5,f,:2,.6,:4,.7,.(i11),@(y7:codeg"
  "en)[07}:5,'(c_)W0:5,.6g,@(y20:write-serialized-arg)[72}:0?{${:1,.3A8,,"
  "#0.0,:5,:4,:3,:2,.(i11),&6{%2.0u?{:4,f,:1,:2,:3,.6,:0,@(y7:codegen)[27"
  "}${:4,f,:1,:2,:3,.8,.8a,@(y7:codegen)[07}:4,'(c,)W0.1,fc,.1d,:5^[22}.!"
  "0.0^_1[02}:5,'(c[)W0${:5,:0,@(y20:write-serialized-arg)[02}:5,.1g,@(y2"
  "0:write-serialized-arg)[22}:5,'(c$)W0:5,'(c{)W0${:1,fc,fc,.3A8,,#0.0,:"
  "5,:4,:3,:2,.(i11),&6{%2.0u?{:4,f,:1,:2,:3,.6,:0,@(y7:codegen)[27}${:4,"
  "f,:1,:2,:3,.8,.8a,@(y7:codegen)[07}:4,'(c,)W0.1,fc,.1d,:5^[22}.!0.0^_1"
  "[02}:5,'(c[)W0${:5,'0,@(y20:write-serialized-arg)[02}${:5,.3g,@(y20:wr"
  "ite-serialized-arg)[02}:5,'(c})W0]2},@(y13:apply-to-list)[72}'(y3:asm)"
  ",.1aq?{.0d,.6,.8,&2{%1${:0,.3,@(y12:write-string)[02}:1?{:0,'(c])W0:0,"
  ":1,@(y20:write-serialized-arg)[12}]1},@(y13:apply-to-list)[72}'(y4:onc"
  "e),.1aq?{.0d,.7,.7,.7,.7,.7,.7,&6{%2:5,:4,:3,:2,:1,:0,n,n,.9c,n,n,tc,'"
  "(y5:quote)cc,.9c,'(y5:gset!)cc,'(y5:begin)cc,n,'(y5:begin)cc,n,n,tc,'("
  "y5:quote)cc,n,.9c,'(y4:gref)cc,'(y3:eq?)U5c,'(y10:integrable)cc,'(y2:i"
  "f)c,@(y7:codegen)[27},@(y13:apply-to-list)[72}'(l4:y6:define;y13:defin"
  "e-syntax;y14:define-library;y6:import;),.1aA0?{.0d,.1,&1{%!0:0,'(s25:m"
  "isplaced definition form),@(y7:c-error)[12},@(y13:apply-to-list)[72}.0"
  ",'(s22:unexpected <core> form),@(y7:c-error)[72",

  "P", "compile-to-string",
  "%1P51,${.2,f,${n,.8,@(y9:find-free)[02},n,n,n,.9,@(y7:codegen)[07}.0P9"
  "0]2",

  "P", "compile-to-thunk-code",
  "%1P51,${.2,'0,${n,.8,@(y9:find-free)[02},n,n,n,.9,@(y7:codegen)[07}.0P"
  "90]2",

  "P", "path-directory",
  "%1.0X2A8,,#0.0,&1{%1.0u?{'(s0:)]1}'(l3:c%5c;c/;c:;),.1aA1?{.0A8X3]1}.0"
  "d,:0^[11}.!0.0^_1[11",

  "P", "base-path-separator",
  "%1.0X2A8,.0u?{f]2}'(l2:c%5c;c/;),.1aA1?{.0a]2}f]2",

  "P", "path-relative?",
  "%1.0X2,.0u?{f]2}'(l2:c%5c;c/;),.1aA1?{f]2}'3,.1g>?{.0aC4?{'(c:),.1dav?"
  "{'(c%5c),.1ddav}{f}}{f}}{f}?{f]2}t]2",

  "P", "file-resolve-relative-to-base-path",
  "%2${.2,@(y14:path-relative?)[01}?{${.3,@(y19:base-path-separator)[01}}"
  "{f}?{.0,.2S6]2}.0]2",

  "C", 0,
  "n@!(y20:*current-file-stack*)",

  "P", "current-file-stack",
  "%0@(y20:*current-file-stack*)]0",

  "P", "set-current-file-stack!",
  "%1.0@!(y20:*current-file-stack*)]1",

  "P", "current-file",
  "%0@(y20:*current-file-stack*)p?{@(y20:*current-file-stack*)a]0}f]0",

  "P", "push-current-file!",
  "%1${@(y8:string=?),@(y20:*current-file-stack*),.4,@(y7:%25member)[03}?"
  "{${.2,'(s32:circularity in nested file chain),@(y7:x-error)[02}}@(y20:"
  "*current-file-stack*),.1c@!(y20:*current-file-stack*)]1",

  "P", "pop-current-file!",
  "%0@(y20:*current-file-stack*)u~?{@(y20:*current-file-stack*)d@!(y20:*c"
  "urrent-file-stack*)]0}]0",

  "P", "with-current-file",
  "%2&0{%0@(y17:pop-current-file!)[00},.2,.2,&1{%0:0,@(y18:push-current-f"
  "ile!)[01},@(y12:dynamic-wind)[23",

  "P", "file-resolve-relative-to-current",
  "%1${.2,@(y14:path-relative?)[01}?{${@(y12:current-file)[00},.0?{${.2,@"
  "(y14:path-directory)[01},.2,@(y34:file-resolve-relative-to-base-path)["
  "22}.1]2}.0]1",

  "P", "lnpart?",
  "%1${.2,@(y3:id?)[01},.0?{.0]2}.1I0]2",

  "P", "listname?",
  "%1${.2,@(y7:list1+?)[01}?{.0,@(y7:lnpart?),@(y6:andmap)[12}f]1",

  "P", "mangle-symbol->string",
  "%1,#0'(l5:c!;c$;c-;c_;c=;).!0n,.2X4X2,,#0.0,.4,&2{%2.0u?{.1A8X3]2}.0aC"
  "2,.0?{.0}{.1aC5}_1?{.1,.1ac,.1d,:1^[22}:0^,.1aA1?{.1,.1ac,.1d,:1^[22}'"
  "(i16),.1aX8E8,'2,.1S3<?{.0,'(s1:0)S6}{.0},.0SdX2,'(c%25)c,.4,.1A8L6,.4"
  "d,:1^[52}.!0.0^_1[22",

  "P", "listname->symbol",
  "%1,,,,#0#1#2#3'(s0:).!0'(s5:lib:/).!1'(s1:/).!2'(s1:/).!3.4L0~?{${.6,'"
  "(s20:invalid library name),@(y7:x-error)[02}}.1^,l1,.5,,#0.7,.1,.8,.8,"
  ".7,&5{%2.0u?{${.3,:0^cA8,@(y14:%25string-append),@(y13:apply-to-list)["
  "02}X5]2}.0aY0?{.1,:1^c,${.3a,@(y21:mangle-symbol->string)[01}c,.1d,:3^"
  "[22}.0aI0?{.1,:2^c,'(i10),.2aE8c,.1d,:3^[22}:4,'(s20:invalid library n"
  "ame),@(y7:x-error)[22}.!0.0^_1[52",

  "P", "listname-segment->string",
  "%1.0Y0?{.0,@(y21:mangle-symbol->string)[11}.0I0?{'(i10),.1E8]1}.0,'(s3"
  "3:invalid library name name element),@(y7:c-error)[12",

  "P", "listname->path",
  "%3,#0${.4,@(y19:base-path-separator)[01},.0?{.0,S11}{${.5,'(s38:librar"
  "y path does not end in separator),@(y7:c-error)[02}}_1.!0n,.2,,#0.5,.7"
  ",.2,.6,&4{%2.0p?{.1u?{.1,${.3a,@(y24:listname-segment->string)[01}c}{."
  "1,:0^c,${.3a,@(y24:listname-segment->string)[01}c},.1d,:1^[22}:3,${.4,"
  ":2cA8,@(y14:string-append*)[01},@(y34:file-resolve-relative-to-base-pa"
  "th)[22}.!0.0^_1[42",

  "C", 0,
  "'(l1:s2:./;)@!(y19:*library-path-list*)",

  "P", "add-library-path!",
  "%1${.2,@(y19:base-path-separator)[01}?{.0,l1,@(y19:*library-path-list*"
  ")L6@!(y19:*library-path-list*)]1}.0,'(s46:library path should end in d"
  "irectory separator),@(y7:c-error)[12",

  "P", "find-library-path",
  "%1@(y19:*library-path-list*),,#0.0,.3,&2{%1.0p?{${'(s4:.sld),.3a,:0,@("
  "y14:listname->path)[03},.0?{.0F0}{f}?{.0]2}.1d,:1^[21}f]1}.!0.0^_1[11",

  "P", "read-port-sexps",
  "%1n,,#0.2,.1,&2{%1${:1,@(y14:read-code-sexp)[01},.0R8?{.1A9]2}.1,.1c,:"
  "0^[21}.!0.0^_1[11",

  "P", "read-file-sexps",
  "%2.1,&1{%1:0?{t,.1P79}.0,@(y15:read-port-sexps)[11},.1,@(y20:call-with"
  "-input-file)[22",

  "P", "library-available?",
  "%2${.2,@(y9:listname?)[01}~?{f]2}${.3,.3,@(y19:find-library-in-env)[02"
  "},.0?{.0]3}.1,@(y17:find-library-path)[31",

  "P", "fully-qualified-library-prefixed-name",
  "%2.1,'(y1:?),.2Y0?{.2}{${.4,@(y16:listname->symbol)[01}},@(y13:symbol-"
  "append)[23",

  "P", "fetch-library",
  "%2,,#0#1.3,.3,&2{%1@(y7:*quiet*)~?{Pe,.0,'(s11:; fetching )W4.0,:0W5.0"
  ",'(s14: library from )W4.0,.2W4.0W6_1}.0,:1,:0,&3{%0${f,:2,@(y15:read-"
  "file-sexps)[02},${.2,'(l1:l4:y14:define-library;y1:*;y1:*;y3:...;;),@("
  "y11:sexp-match?)[02}?{${t,:1,.4ad,.5aa,@(y20:xform-define-library)[04}"
  ",${.2,'(l3:y14:define-library;y1:*;y1:*;),@(y11:sexp-match?)[02}?{:0,."
  "1dae?{.0ddaV0}{f}}{f}?{.0dda]2}.0,.2,:2,:0,'(s46:library autoloader: i"
  "nternal transformer error),@(y7:x-error)[25}.0,:2,:0,'(s49:library aut"
  "oloader: unexpected forms in .sld file),@(y7:x-error)[14},.1,@(y17:wit"
  "h-current-file)[12}.!0${.4,@(y17:find-library-path)[01}.!1.1^?{.1^,.1^"
  "[41}f]4",

  "P", "make-name-registry",
  "%1,,#0#1'(l40:i1;i11;i31;i41;i61;i71;i101;i131;i151;i181;i191;i211;i24"
  "1;i251;i271;i281;i311;i331;i401;i421;i431;i461;i491;i521;i541;i571;i60"
  "1;i631;i641;i661;i691;i701;i751;i761;i811;i821;i881;i911;i941;i971;).!"
  "0${@(y2:<=),.3^,.6,@(y7:%25member)[03},.0?{.0a}{'(i991)}_1.!1n,'1,.3^+"
  "V2]3",

  "P", "eal->name-registry",
  "%1n,.1,V12]1",

  "P", "eal-name-registry-import!",
  "%2${.3,'0,.4V4,@(y11:adjoin-eals)[02},'0,.2V5]2",

  "P", "name-lookup",
  "%3'1,.1V3-,.2p?{.0}{.0,.3H2},.0,.3V4,.4p?{.0,.5A5}{.0,.5A3},.0?{.0d]7}"
  ".6?{${.7,.9[01},.0~?{f]8}.0Y2?{.0]8}.6p?{.2,.7A5}{.2,.7A3},.4,.7V4,.1?"
  "{.8,'(s31:recursive library dependence on),@(y7:x-error)[(i10)2}.2b,.1"
  ",.1,.(i11)cc,.7,.(i10)V5.0](i11)}f]7",

  "P", "name-install!",
  "%3'1,.1V3-,.2p?{.0}{.0,.3H2},.0,.3V4,.4p?{.0,.5A5}{.0,.5A3},.0?{.6,.1d"
  "q}{f}?{'(y4:same)]7}.0?{.6,.1sd'(y8:modified)]7}.1,.7,.7cc,.3,.6V5'(y5"
  ":added)]7",

  "P", "name-remove!",
  "%2'1,.1V3-,.2p?{.0}{.0,.3H2},${&0{%2.0,.2ae]2},.3,.6V4,.7,@(y7:remove!"
  ")[03},.1,.4V5]4",

  "C", 0,
  "${'(i300),@(y18:make-name-registry)[01}@!(y20:*root-name-registry*)",

  "C", 0,
  "${'0,,#0.0,&1{%1.0U5,.0?{.1U0?{.1U7,.0Y0?{${.4,&1{%1:0]1},.3,@(y20:*ro"
  "ot-name-registry*),@(y11:name-lookup)[03}}_1}'1,.2+,:0^[21}]2}.!0.0^_1"
  "[01}",

  "C", 0,
  "${U1,,#0.0,&1{%1.0u~?{.0a,.1d,.1a,.2d,.0Y0,.0?{.0}{.1U0}_1?{${.2,&1{%1"
  ":0]1},.4,@(y20:*root-name-registry*),@(y11:name-lookup)[03}.2,:0^[51}."
  "0p?{'(y12:syntax-rules),.1aq}{f}?{,,#0#1&0{%2.0K0?{.0,@(y7:old-den)[21"
  "}&0{%1.0,'(y3:ref),l2]1},.1,@(y20:*root-name-registry*),@(y11:name-loo"
  "kup)[23}.!0${.4da,@(y3:id?)[01}?{${.4ddd,.5dda,.6da,.5^,@(y13:syntax-r"
  "ules*)[04}}{${.4dd,.5da,f,.5^,@(y13:syntax-rules*)[04}}.!1${.3,&1{%1:0"
  "^]1},.6,@(y20:*root-name-registry*),@(y11:name-lookup)[03}.4,:0^[71}f]"
  "5}]1}.!0.0^_1[01}",

  "C", 0,
  "${&0{%1f,@(y24:make-include-transformer)[11},'(y7:include),@(y20:*root"
  "-name-registry*),@(y11:name-lookup)[03}",

  "C", 0,
  "${&0{%1t,@(y24:make-include-transformer)[11},'(y10:include-ci),@(y20:*"
  "root-name-registry*),@(y11:name-lookup)[03}",

  "C", 0,
  "${&0{%1@(y28:make-cond-expand-transformer)[10},'(y11:cond-expand),@(y2"
  "0:*root-name-registry*),@(y11:name-lookup)[03}",

  "C", 0,
  "${'(l462:l3:y1:*;y1:v;y1:b;;l3:y1:+;y1:v;y1:b;;l3:y1:-;y1:v;y1:b;;l4:y"
  "3:...;y1:v;y1:u;y1:b;;l3:y1:/;y1:v;y1:b;;l3:y1:<;y1:v;y1:b;;l3:y2:<=;y"
  "1:v;y1:b;;l3:y1:=;y1:v;y1:b;;l4:y2:=>;y1:v;y1:u;y1:b;;l3:y1:>;y1:v;y1:"
  "b;;l3:y2:>=;y1:v;y1:b;;l2:y1:_;y1:b;;l3:y3:abs;y1:v;y1:b;;l4:y3:and;y1"
  ":v;y1:u;y1:b;;l3:y6:append;y1:v;y1:b;;l3:y5:apply;y1:v;y1:b;;l3:y5:ass"
  "oc;y1:v;y1:b;;l3:y4:assq;y1:v;y1:b;;l3:y4:assv;y1:v;y1:b;;l4:y5:begin;"
  "y1:v;y1:u;y1:b;;l2:y12:binary-port?;y1:b;;l2:y9:boolean=?;y1:b;;l3:y8:"
  "boolean?;y1:v;y1:b;;l2:y10:bytevector;y1:b;;l2:y17:bytevector-append;y"
  "1:b;;l2:y15:bytevector-copy;y1:b;;l2:y16:bytevector-copy!;y1:b;;l2:y17"
  ":bytevector-length;y1:b;;l2:y17:bytevector-u8-ref;y1:b;;l2:y18:bytevec"
  "tor-u8-set!;y1:b;;l2:y11:bytevector?;y1:b;;l3:y4:caar;y1:v;y1:b;;l3:y4"
  ":cadr;y1:v;y1:b;;l3:y30:call-with-current-continuation;y1:v;y1:b;;l2:y"
  "14:call-with-port;y1:b;;l3:y16:call-with-values;y1:v;y1:b;;l2:y7:call/"
  "cc;y1:b;;l3:y3:car;y1:v;y1:b;;l4:y4:case;y1:v;y1:u;y1:b;;l3:y4:cdar;y1"
  ":v;y1:b;;l3:y4:cddr;y1:v;y1:b;;l3:y3:cdr;y1:v;y1:b;;l3:y7:ceiling;y1:v"
  ";y1:b;;l3:y13:char->integer;y1:v;y1:b;;l3:y11:char-ready?;y1:v;y1:b;;l"
  "3:y7:char<=?;y1:v;y1:b;;l3:y6:char<?;y1:v;y1:b;;l3:y6:char=?;y1:v;y1:b"
  ";;l3:y7:char>=?;y1:v;y1:b;;l3:y6:char>?;y1:v;y1:b;;l2:y5:char?;y1:b;;l"
  "3:y16:close-input-port;y1:v;y1:b;;l3:y17:close-output-port;y1:v;y1:b;;"
  "l2:y10:close-port;y1:b;;l3:y8:complex?;y1:v;y1:b;;l4:y4:cond;y1:v;y1:u"
  ";y1:b;;l2:y11:cond-expand;y1:b;;l3:y4:cons;y1:v;y1:b;;l2:y18:current-e"
  "rror-port;y1:b;;l3:y18:current-input-port;y1:v;y1:b;;l3:y19:current-ou"
  "tput-port;y1:v;y1:b;;l4:y6:define;y1:v;y1:u;y1:b;;l2:y18:define-record"
  "-type;y1:b;;l4:y13:define-syntax;y1:v;y1:u;y1:b;;l2:y13:define-values;"
  "y1:b;;l3:y11:denominator;y1:v;y1:b;;l4:y2:do;y1:v;y1:u;y1:b;;l3:y12:dy"
  "namic-wind;y1:v;y1:b;;l4:y4:else;y1:v;y1:u;y1:b;;l2:y10:eof-object;y1:"
  "b;;l3:y11:eof-object?;y1:v;y1:b;;l3:y3:eq?;y1:v;y1:b;;l3:y6:equal?;y1:"
  "v;y1:b;;l3:y4:eqv?;y1:v;y1:b;;l2:y5:error;y1:b;;l2:y7:library;y1:b;;l2"
  ":y22:error-object-irritants;y1:b;;l2:y20:error-object-message;y1:b;;l2"
  ":y13:error-object?;y1:b;;l3:y5:even?;y1:v;y1:b;;l2:y5:exact;y1:b;;l2:y"
  "18:exact-integer-sqrt;y1:b;;l2:y14:exact-integer?;y1:b;;l3:y6:exact?;y"
  "1:v;y1:b;;l3:y4:expt;y1:v;y1:b;;l2:y8:features;y1:b;;l2:y11:file-error"
  "?;y1:b;;l3:y5:floor;y1:v;y1:b;;l2:y14:floor-quotient;y1:b;;l2:y15:floo"
  "r-remainder;y1:b;;l2:y6:floor/;y1:b;;l2:y17:flush-output-port;y1:b;;l3"
  ":y8:for-each;y1:v;y1:b;;l3:y3:gcd;y1:v;y1:b;;l2:y21:get-output-bytevec"
  "tor;y1:b;;l2:y17:get-output-string;y1:b;;l2:y5:guard;y1:b;;l4:y2:if;y1"
  ":v;y1:u;y1:b;;l2:y7:include;y1:b;;l2:y10:include-ci;y1:b;;l2:y7:inexac"
  "t;y1:b;;l3:y8:inexact?;y1:v;y1:b;;l2:y16:input-port-open?;y1:b;;l3:y11"
  ":input-port?;y1:v;y1:b;;l3:y13:integer->char;y1:v;y1:b;;l3:y8:integer?"
  ";y1:v;y1:b;;l4:y6:lambda;y1:v;y1:u;y1:b;;l3:y3:lcm;y1:v;y1:b;;l3:y6:le"
  "ngth;y1:v;y1:b;;l4:y3:let;y1:v;y1:u;y1:b;;l4:y4:let*;y1:v;y1:u;y1:b;;l"
  "2:y11:let*-values;y1:b;;l4:y10:let-syntax;y1:v;y1:u;y1:b;;l2:y10:let-v"
  "alues;y1:b;;l4:y6:letrec;y1:v;y1:u;y1:b;;l2:y7:letrec*;y1:b;;l4:y13:le"
  "trec-syntax;y1:v;y1:u;y1:b;;l3:y4:list;y1:v;y1:b;;l3:y12:list->string;"
  "y1:v;y1:b;;l3:y12:list->vector;y1:v;y1:b;;l2:y9:list-copy;y1:b;;l3:y8:"
  "list-ref;y1:v;y1:b;;l2:y9:list-set!;y1:b;;l3:y9:list-tail;y1:v;y1:b;;l"
  "3:y5:list?;y1:v;y1:b;;l2:y15:make-bytevector;y1:b;;l2:y9:make-list;y1:"
  "b;;l2:y14:make-parameter;y1:b;;l3:y11:make-string;y1:v;y1:b;;l3:y11:ma"
  "ke-vector;y1:v;y1:b;;l3:y3:map;y1:v;y1:b;;l3:y3:max;y1:v;y1:b;;l3:y6:m"
  "ember;y1:v;y1:b;;l3:y4:memq;y1:v;y1:b;;l3:y4:memv;y1:v;y1:b;;l3:y3:min"
  ";y1:v;y1:b;;l3:y6:modulo;y1:v;y1:b;;l3:y9:negative?;y1:v;y1:b;;l3:y7:n"
  "ewline;y1:v;y1:b;;l3:y3:not;y1:v;y1:b;;l3:y5:null?;y1:v;y1:b;;l3:y14:n"
  "umber->string;y1:v;y1:b;;l3:y7:number?;y1:v;y1:b;;l3:y9:numerator;y1:v"
  ";y1:b;;l3:y4:odd?;y1:v;y1:b;;l2:y21:open-input-bytevector;y1:b;;l2:y17"
  ":open-input-string;y1:b;;l2:y22:open-output-bytevector;y1:b;;l2:y18:op"
  "en-output-string;y1:b;;l4:y2:or;y1:v;y1:u;y1:b;;l2:y17:output-port-ope"
  "n?;y1:b;;l3:y12:output-port?;y1:v;y1:b;;l3:y5:pair?;y1:v;y1:b;;l2:y12:"
  "parameterize;y1:b;;l3:y9:peek-char;y1:v;y1:b;;l2:y7:peek-u8;y1:b;;l2:y"
  "5:port?;y1:b;;l3:y9:positive?;y1:v;y1:b;;l3:y10:procedure?;y1:v;y1:b;;"
  "l4:y10:quasiquote;y1:v;y1:u;y1:b;;l4:y5:quote;y1:v;y1:u;y1:b;;l3:y8:qu"
  "otient;y1:v;y1:b;;l2:y5:raise;y1:b;;l2:y17:raise-continuable;y1:b;;l3:"
  "y9:rational?;y1:v;y1:b;;l3:y11:rationalize;y1:v;y1:b;;l2:y15:read-byte"
  "vector;y1:b;;l2:y16:read-bytevector!;y1:b;;l3:y9:read-char;y1:v;y1:b;;"
  "l2:y11:read-error?;y1:b;;l2:y9:read-line;y1:b;;l2:y11:read-string;y1:b"
  ";;l2:y7:read-u8;y1:b;;l3:y5:real?;y1:v;y1:b;;l3:y9:remainder;y1:v;y1:b"
  ";;l3:y7:reverse;y1:v;y1:b;;l3:y5:round;y1:v;y1:b;;l3:y4:set!;y1:v;y1:b"
  ";;l3:y8:set-car!;y1:v;y1:b;;l3:y8:set-cdr!;y1:v;y1:b;;l2:y6:square;y1:"
  "b;;l3:y6:string;y1:v;y1:b;;l3:y12:string->list;y1:v;y1:b;;l3:y14:strin"
  "g->number;y1:v;y1:b;;l3:y14:string->symbol;y1:v;y1:b;;l2:y12:string->u"
  "tf8;y1:b;;l2:y14:string->vector;y1:b;;l3:y13:string-append;y1:v;y1:b;;"
  "l3:y11:string-copy;y1:v;y1:b;;l2:y12:string-copy!;y1:b;;l3:y12:string-"
  "fill!;y1:v;y1:b;;l2:y15:string-for-each;y1:b;;l3:y13:string-length;y1:"
  "v;y1:b;;l2:y10:string-map;y1:b;;l3:y10:string-ref;y1:v;y1:b;;l3:y11:st"
  "ring-set!;y1:v;y1:b;;l3:y9:string<=?;y1:v;y1:b;;l3:y8:string<?;y1:v;y1"
  ":b;;l3:y8:string=?;y1:v;y1:b;;l3:y9:string>=?;y1:v;y1:b;;l3:y8:string>"
  "?;y1:v;y1:b;;l3:y7:string?;y1:v;y1:b;;l3:y9:substring;y1:v;y1:b;;l3:y1"
  "4:symbol->string;y1:v;y1:b;;l2:y8:symbol=?;y1:b;;l3:y7:symbol?;y1:v;y1"
  ":b;;l2:y12:syntax-error;y1:b;;l4:y12:syntax-rules;y1:v;y1:u;y1:b;;l2:y"
  "13:textual-port?;y1:b;;l3:y8:truncate;y1:v;y1:b;;l2:y17:truncate-quoti"
  "ent;y1:b;;l2:y18:truncate-remainder;y1:b;;l2:y9:truncate/;y1:b;;l2:y9:"
  "u8-ready?;y1:b;;l2:y6:unless;y1:b;;l4:y7:unquote;y1:v;y1:u;y1:b;;l4:y1"
  "6:unquote-splicing;y1:v;y1:u;y1:b;;l2:y12:utf8->string;y1:b;;l3:y6:val"
  "ues;y1:v;y1:b;;l3:y6:vector;y1:v;y1:b;;l3:y12:vector->list;y1:v;y1:b;;"
  "l2:y14:vector->string;y1:b;;l2:y13:vector-append;y1:b;;l2:y11:vector-c"
  "opy;y1:b;;l2:y12:vector-copy!;y1:b;;l3:y12:vector-fill!;y1:v;y1:b;;l2:"
  "y15:vector-for-each;y1:b;;l3:y13:vector-length;y1:v;y1:b;;l2:y10:vecto"
  "r-map;y1:b;;l3:y10:vector-ref;y1:v;y1:b;;l3:y11:vector-set!;y1:v;y1:b;"
  ";l3:y7:vector?;y1:v;y1:b;;l2:y4:when;y1:b;;l2:y22:with-exception-handl"
  "er;y1:b;;l2:y16:write-bytevector;y1:b;;l3:y10:write-char;y1:v;y1:b;;l2"
  ":y12:write-string;y1:b;;l2:y8:write-u8;y1:b;;l3:y5:zero?;y1:v;y1:b;;l2"
  ":y11:case-lambda;y1:l;;l3:y16:char-alphabetic?;y1:v;y1:h;;l3:y10:char-"
  "ci<=?;y1:v;y1:h;;l3:y9:char-ci<?;y1:v;y1:h;;l3:y9:char-ci=?;y1:v;y1:h;"
  ";l3:y10:char-ci>=?;y1:v;y1:h;;l3:y9:char-ci>?;y1:v;y1:h;;l3:y13:char-d"
  "owncase;y1:v;y1:h;;l2:y13:char-foldcase;y1:h;;l3:y16:char-lower-case?;"
  "y1:v;y1:h;;l3:y13:char-numeric?;y1:v;y1:h;;l3:y11:char-upcase;y1:v;y1:"
  "h;;l3:y16:char-upper-case?;y1:v;y1:h;;l3:y16:char-whitespace?;y1:v;y1:"
  "h;;l2:y11:digit-value;y1:h;;l3:y12:string-ci<=?;y1:v;y1:h;;l3:y11:stri"
  "ng-ci<?;y1:v;y1:h;;l3:y11:string-ci=?;y1:v;y1:h;;l3:y12:string-ci>=?;y"
  "1:v;y1:h;;l3:y11:string-ci>?;y1:v;y1:h;;l2:y15:string-downcase;y1:h;;l"
  "2:y15:string-foldcase;y1:h;;l2:y13:string-upcase;y1:h;;l3:y5:angle;y1:"
  "v;y1:o;;l3:y9:imag-part;y1:v;y1:o;;l3:y9:magnitude;y1:v;y1:o;;l3:y10:m"
  "ake-polar;y1:v;y1:o;;l3:y16:make-rectangular;y1:v;y1:o;;l3:y9:real-par"
  "t;y1:v;y1:o;;l3:y5:caaar;y1:v;y1:a;;l3:y5:caadr;y1:v;y1:a;;l3:y5:cadar"
  ";y1:v;y1:a;;l3:y5:caddr;y1:v;y1:a;;l3:y5:cdaar;y1:v;y1:a;;l3:y5:cdadr;"
  "y1:v;y1:a;;l3:y5:cddar;y1:v;y1:a;;l3:y5:cdddr;y1:v;y1:a;;l3:y6:caaaar;"
  "y1:v;y1:a;;l3:y6:caaadr;y1:v;y1:a;;l3:y6:caadar;y1:v;y1:a;;l3:y6:caadd"
  "r;y1:v;y1:a;;l3:y6:cadaar;y1:v;y1:a;;l3:y6:cadadr;y1:v;y1:a;;l3:y6:cad"
  "dar;y1:v;y1:a;;l3:y6:cadddr;y1:v;y1:a;;l3:y6:cdaaar;y1:v;y1:a;;l3:y6:c"
  "daadr;y1:v;y1:a;;l3:y6:cdadar;y1:v;y1:a;;l3:y6:cdaddr;y1:v;y1:a;;l3:y6"
  ":cddaar;y1:v;y1:a;;l3:y6:cddadr;y1:v;y1:a;;l3:y6:cdddar;y1:v;y1:a;;l3:"
  "y6:cddddr;y1:v;y1:a;;l2:y11:environment;y1:e;;l3:y4:eval;y1:v;y1:e;;l3"
  ":y20:call-with-input-file;y1:v;y1:f;;l3:y21:call-with-output-file;y1:v"
  ";y1:f;;l2:y11:delete-file;y1:f;;l2:y12:file-exists?;y1:f;;l2:y22:open-"
  "binary-input-file;y1:f;;l2:y23:open-binary-output-file;y1:f;;l3:y15:op"
  "en-input-file;y1:v;y1:f;;l3:y16:open-output-file;y1:v;y1:f;;l3:y20:wit"
  "h-input-from-file;y1:v;y1:f;;l3:y19:with-output-to-file;y1:v;y1:f;;l4:"
  "y4:acos;y1:v;y1:z;y1:i;;l4:y4:asin;y1:v;y1:z;y1:i;;l4:y4:atan;y1:v;y1:"
  "z;y1:i;;l4:y3:cos;y1:v;y1:z;y1:i;;l4:y3:exp;y1:v;y1:z;y1:i;;l3:y7:fini"
  "te?;y1:z;y1:i;;l2:y9:infinite?;y1:i;;l3:y3:log;y1:v;y1:i;;l2:y4:nan?;y"
  "1:i;;l3:y3:sin;y1:v;y1:i;;l3:y4:sqrt;y1:v;y1:i;;l3:y3:tan;y1:v;y1:i;;l"
  "4:y5:delay;y1:v;y1:u;y1:z;;l2:y11:delay-force;y1:z;;l3:y5:force;y1:v;y"
  "1:z;;l2:y12:make-promise;y1:z;;l2:y8:promise?;y1:z;;l3:y4:load;y1:v;y1"
  ":d;;l2:y12:command-line;y1:s;;l2:y14:emergency-exit;y1:s;;l2:y4:exit;y"
  "1:s;;l2:y24:get-environment-variable;y1:s;;l2:y25:get-environment-vari"
  "ables;y1:s;;l3:y7:display;y1:w;y1:v;;l2:y14:exact->inexact;y1:v;;l2:y1"
  "4:inexact->exact;y1:v;;l3:y23:interaction-environment;y1:p;y1:v;;l2:y1"
  "6:null-environment;y1:v;;l3:y4:read;y1:r;y1:v;;l2:y25:scheme-report-en"
  "vironment;y1:v;;l3:y5:write;y1:w;y1:v;;l2:y13:current-jiffy;y1:t;;l2:y"
  "14:current-second;y1:t;;l2:y18:jiffies-per-second;y1:t;;l2:y12:write-s"
  "hared;y1:w;;l2:y12:write-simple;y1:w;;l3:y4:box?;y1:x;i111;;l3:y3:box;"
  "y1:x;i111;;l3:y5:unbox;y1:x;i111;;l3:y8:set-box!;y1:x;i111;;l3:y6:form"
  "at;i28;i48;;l1:y7:fprintf;;l1:y19:format-pretty-print;;l1:y18:format-f"
  "ixed-print;;l1:y17:format-fresh-line;;l1:y18:format-help-string;;l1:y4"
  ":set&;;l1:y7:lambda*;;l1:y4:body;;l1:y5:letcc;;l1:y6:withcc;;l1:y13:sy"
  "ntax-lambda;;l1:y13:syntax-length;;l1:y7:record?;;l1:y11:make-record;;"
  "l1:y13:record-length;;l1:y10:record-ref;;l1:y11:record-set!;;l1:y7:fix"
  "num?;;l1:y11:fxpositive?;;l1:y11:fxnegative?;;l1:y7:fxeven?;;l1:y6:fxo"
  "dd?;;l1:y3:fx+;;l1:y3:fx*;;l1:y3:fx-;;l1:y3:fx/;;l1:y10:fxquotient;;l1"
  ":y11:fxremainder;;l1:y8:fxmodquo;;l1:y8:fxmodulo;;l1:y8:fxeucquo;;l1:y"
  "8:fxeucrem;;l1:y5:fxneg;;l1:y5:fxabs;;l1:y4:fx<?;;l1:y5:fx<=?;;l1:y4:f"
  "x>?;;l1:y5:fx>=?;;l1:y4:fx=?;;l1:y5:fx!=?;;l1:y5:fxmin;;l1:y5:fxmax;;l"
  "1:y5:fxneg;;l1:y5:fxabs;;l1:y5:fxgcd;;l1:y6:fxexpt;;l1:y6:fxsqrt;;l1:y"
  "5:fxnot;;l1:y5:fxand;;l1:y5:fxior;;l1:y5:fxxor;;l1:y5:fxsll;;l1:y5:fxs"
  "rl;;l1:y14:fixnum->flonum;;l1:y14:fixnum->string;;l1:y14:string->fixnu"
  "m;;l1:y7:flonum?;;l1:y7:flzero?;;l1:y11:flpositive?;;l1:y11:flnegative"
  "?;;l1:y10:flinteger?;;l1:y6:flnan?;;l1:y11:flinfinite?;;l1:y9:flfinite"
  "?;;l1:y7:fleven?;;l1:y6:flodd?;;l1:y3:fl+;;l1:y3:fl*;;l1:y3:fl-;;l1:y3"
  ":fl/;;l1:y5:flneg;;l1:y5:flabs;;l1:y5:flgcd;;l1:y6:flexpt;;l1:y6:flsqr"
  "t;;l1:y7:flfloor;;l1:y9:flceiling;;l1:y10:fltruncate;;l1:y7:flround;;l"
  "1:y5:flexp;;l1:y5:fllog;;l1:y5:flsin;;l1:y5:flcos;;l1:y5:fltan;;l1:y6:"
  "flasin;;l1:y6:flacos;;l2:y6:flatan;l1:y1:y;;;l1:y4:fl<?;;l1:y5:fl<=?;;"
  "l1:y4:fl>?;;l1:y5:fl>=?;;l1:y4:fl=?;;l1:y5:fl!=?;;l1:y5:flmin;;l1:y5:f"
  "lmax;;l1:y14:flonum->fixnum;;l1:y14:flonum->string;;l1:y14:string->flo"
  "num;;l1:y8:list-cat;;l1:y4:meme;;l1:y4:asse;;l1:y8:reverse!;;l1:y9:cir"
  "cular?;;l1:y8:char-cmp;;l1:y11:char-ci-cmp;;l1:y10:string-cat;;l1:y15:"
  "string-position;;l1:y10:string-cmp;;l1:y13:string-ci-cmp;;l1:y10:vecto"
  "r-cat;;l1:y16:bytevector->list;;l1:y16:list->bytevector;;l1:y13:subbyt"
  "evector;;l1:y19:standard-input-port;;l1:y20:standard-output-port;;l1:y"
  "19:standard-error-port;;l1:y11:rename-file;;),&0{%1,,,,#0#1#2#3&0{%1.0"
  ",'(y1:w),.1v?{'(l2:y6:scheme;y5:write;)]2}'(y1:t),.1v?{'(l2:y6:scheme;"
  "y4:time;)]2}'(y1:p),.1v?{'(l2:y6:scheme;y4:repl;)]2}'(y1:r),.1v?{'(l2:"
  "y6:scheme;y4:read;)]2}'(y1:v),.1v?{'(l2:y6:scheme;y4:r5rs;)]2}'(y1:u),"
  ".1v?{'(l2:y6:scheme;y9:r5rs-null;)]2}'(y1:d),.1v?{'(l2:y6:scheme;y4:lo"
  "ad;)]2}'(y1:z),.1v?{'(l2:y6:scheme;y4:lazy;)]2}'(y1:s),.1v?{'(l2:y6:sc"
  "heme;y15:process-context;)]2}'(y1:i),.1v?{'(l2:y6:scheme;y7:inexact;)]"
  "2}'(y1:f),.1v?{'(l2:y6:scheme;y4:file;)]2}'(y1:e),.1v?{'(l2:y6:scheme;"
  "y4:eval;)]2}'(y1:o),.1v?{'(l2:y6:scheme;y7:complex;)]2}'(y1:h),.1v?{'("
  "l2:y6:scheme;y4:char;)]2}'(y1:l),.1v?{'(l2:y6:scheme;y11:case-lambda;)"
  "]2}'(y1:a),.1v?{'(l2:y6:scheme;y3:cxr;)]2}'(y1:b),.1v?{'(l2:y6:scheme;"
  "y4:base;)]2}'(y1:x),.1v?{'(l2:y6:scheme;y3:box;)]2}.1I0?{.1,'(y4:srfi)"
  ",l2]2}.1,l1]2}.!0&0{%1${&0{%1n,'(l1:y5:begin;),V12]1},.3,@(y20:*root-n"
  "ame-registry*),@(y11:name-lookup)[03}z]1}.!1&0{%3'1,.1V4,.0,.3A3,.0?{."
  "4,.1sd]5}.1,.5,.5cc,'1,.4V5]5}.!2&0{%1&0{%1.0,'(y5:const),l2]1},.1,@(y"
  "20:*root-name-registry*),@(y11:name-lookup)[13}.!3.4d,.5a,,#0.0,.6,.5,"
  ".7,.(i10),&5{%2.1u?{${.2,:0^[01},.1,${'(l1:y4:repl;),:1^[01},:3^[23}${"
  "${.4,:0^[01},.3,${${.9a,:2^[01},:1^[01},:3^[03}.1d,.1,:4^[22}.!0.0^_1["
  "52},@(y10:%25for-each1)[02}",

  "C", 0,
  "${'(i200),@(y18:make-name-registry)[01}@!(y20:*user-name-registry*)",

  "P", "make-readonly-environment",
  "%1.0,&1{%2.0K0?{.1,'(l2:y3:ref;y4:set!;),.1A1?{.1,@(y7:old-den)[31}f]3"
  "}'(y3:ref),.2q?{f,.1,:0,@(y11:name-lookup)[23}f]2}]1",

  "P", "make-controlled-environment",
  "%3,,#0#1${.4,@(y18:eal->name-registry)[01}.!0${'(i100),@(y18:make-name"
  "-registry)[01}.!1.1,.1,.5,.7,&4{%2.0K0?{.1,'(l2:y3:ref;y4:set!;),.1A1?"
  "{.1,@(y7:old-den)[31}f]3}'(y3:ref),.2q?{:2,.1,:1,:0,&4{%1${f,:2,:3^,@("
  "y11:name-lookup)[03},.0?{.0]2}:2Y0?{${:2,:1[01},'(y3:ref),l2]2}'(y3:re"
  "f),:2,:0[22},.1,:3^,@(y11:name-lookup)[23}'(l2:y4:set!;y6:define;),.2A"
  "0?{.0Y0?{.0,:1,:2,&3{%1${f,:2,:0^,@(y11:name-lookup)[03}~?{${:2,:1[01}"
  ",'(y3:ref),l2]1}f]1},.1,:3^,@(y11:name-lookup)[23}f]2}'(y13:define-syn"
  "tax),.2q?{.0,:2,&2{%1${f,:1,:0^,@(y11:name-lookup)[03}~?{Y9]1}f]1},.1,"
  ":3^,@(y11:name-lookup)[23}'(y6:import),.2q?{${.2,'(l2:py8:<symbol>;zy1"
  ":*;;;y3:...;),@(y11:sexp-match?)[02}}{f}?{.0,,#0:3,&1{%1${f,.3a,:0^,@("
  "y11:name-lookup)[03},.0?{.0,.0,.3d,.4a,'(s32:imported name shadows loc"
  "al name),@(y7:x-error)[34}f]2}.!0${.3,.3^,@(y10:%25for-each1)[02}${.3,"
  ":2^,@(y25:eal-name-registry-import!)[02}t]4}f]2}]5",

  "P", "make-sld-environment",
  "%1,#0.0,.2,&2{%2'(y3:ref),.2q~?{f]2}.0K0?{.0,@(y7:old-den)[21}'(y14:de"
  "fine-library),.1q?{'(y14:define-library)b]2}${.2,@(y9:listname?)[01}~?"
  "{f]2}:1,.1,&2{%1:1^,:0,@(y13:fetch-library)[12},.1,:0,@(y11:name-looku"
  "p)[23}.!0.0^]2",

  "P", "make-repl-environment",
  "%3,#0.3,&1{%1.0,:0,@(y37:fully-qualified-library-prefixed-name)[12}.!0"
  ".2,.2,.2,&3{%2.0K0?{.1,'(l2:y3:ref;y4:set!;),.1A1?{.1,@(y7:old-den)[31"
  "}f]3}'(y3:ref),.2q?{.0,:1,:0,&3{%1${:2,:1,&2{%1${:1,@(y9:listname?)[01"
  "}?{${:0,@(y20:make-sld-environment)[01},.0,:1,@(y13:fetch-library)[22}"
  "f]1},:2,:1,@(y11:name-lookup)[03},.0?{.0]2}:2Y0?{${:2,:0^[01},'(y3:ref"
  "),l2]2}f]2},.1,:2,@(y11:name-lookup)[23}'(y4:set!),.2q?{.0Y0?{.0,:0,:1"
  ",&3{%1${f,:2,:0,@(y11:name-lookup)[03}~?{${:2,:1^[01},'(y3:ref),l2]1}f"
  "]1},.1,:2,@(y11:name-lookup)[23}f]2}'(y6:define),.2q?{.0Y0?{:0,.1,&2{%"
  "1${:0,:1^[01},'(y3:ref),l2]1},.1,:2,@(y11:name-lookup)[23}f]2}'(y13:de"
  "fine-syntax),.2q?{&0{%1Y9]1},.1,:2,@(y11:name-lookup)[23}'(y6:import),"
  ".2q?{${.2,'(l2:py8:<symbol>;zy1:*;;;y3:...;),@(y11:sexp-match?)[02}}{f"
  "}?{'0,'0,'0,.3,,#0:2,:1,.2,&3{%4.0u?{.3,.3,.3,l3]4}.0d,.1ad,.2aa,${.2,"
  ":2,@(y12:name-remove!)[02}${.3,.3,:1,@(y13:name-install!)[03},'(y4:sam"
  "e),.1v?{.7,.7,'1,.8+,.6,:0^[84}'(y8:modified),.1v?{.7,'1,.8+,.7,.6,:0^"
  "[84}'(y5:added),.1v?{'1,.8+,.7,.7,.6,:0^[84}]8}.!0.0^_1[24}f]2}]4",

  "P", "find-library-in-env",
  "%2${'(y3:ref),.3,.5[02},.0?{.0z,.0V0?{.0]4}f]4}f]3",

  "C", 0,
  "${@(y20:*root-name-registry*),@(y25:make-readonly-environment)[01}@!(y"
  "16:root-environment)",

  "C", 0,
  "${'(y7:repl://),@(y20:*user-name-registry*),@(y20:*root-name-registry*"
  "),@(y21:make-repl-environment)[03}@!(y16:repl-environment)",

  "C", 0,
  "f@!(y9:*verbose*)",

  "C", 0,
  "f@!(y7:*quiet*)",

  "P", "repl-compile-and-run-core-expr",
  "%1@(y9:*verbose*)?{Po,'(s12:TRANSFORM =>)W4PoW6Po,.1W5PoW6}.0p~?{${.2,"
  "'(s29:unexpected transformed output),@(y7:x-error)[02}}f,${.3,@(y21:co"
  "mpile-to-thunk-code)[01},#1@(y9:*verbose*)?{Po,'(s20:COMPILE-TO-STRING"
  " =>)W4PoW6Po,.1W4PoW6Po,'(s17:DECODE+EXECUTE =>)W4PoW6Z3.!1}.0U4,U91,$"
  "{.2[00},@(y9:*verbose*)?{Po,'(s14:Elapsed time: )W4Po,Z4,.5^,Z3-/,'(i1"
  "000)*W5Po,'(s4: ms.)W4PoW6}Y9,.1q~?{Po,.1W5PoW6]5}]5",

  "P", "repl-eval-top-form",
  "%2.0p?{${.3,.3a,t,@(y5:xform)[03},'(y5:begin),.1q?{.1d,,#0.4,.1,&2{%1."
  "0p?{${:1,.3a,@(y18:repl-eval-top-form)[02}.0d,:0^[11}]1}.!0.0^_1[31}'("
  "y6:define),.1q?{.1dau}{f}?{.2,.2dda,@(y18:repl-eval-top-form)[32}'(y6:"
  "define),.1q?{${.4,.4d,@(y12:xform-define)[02},${'(y6:define),.3da,.7,@"
  "(y11:xenv-lookup)[03},.0?{${.2z,'(l2:y3:ref;y1:*;),@(y11:sexp-match?)["
  "02}}{f}?{.1dda,.1zda,'(y4:set!),l3,@(y30:repl-compile-and-run-core-exp"
  "r)[51}.4,.2da,'(s52:identifier cannot be (re)defined as variable in en"
  "v:),@(y7:x-error)[53}'(y13:define-syntax),.1q?{${.4,.4d,@(y19:xform-de"
  "fine-syntax)[02},${'(y13:define-syntax),.3da,.7,@(y11:xenv-lookup)[03}"
  ",.0Y2~?{${.5,.4da,'(s31:unexpected define-syntax for id),@(y7:x-error)"
  "[03}}.1dda,.1sz@(y9:*verbose*)?{Po,'(s18:SYNTAX INSTALLED: )W4Po,.2daW"
  "5PoW6]5}]5}'(y14:define-library),.1q?{${t,.5,.5d,.6a,@(y20:xform-defin"
  "e-library)[04},${'(y13:define-syntax),.3da,.7,@(y11:xenv-lookup)[03},."
  "0Y2~?{${.5,.4da,'(s32:unexpected define-library for id),@(y7:x-error)["
  "03}}.1dda,.1sz@(y9:*verbose*)?{Po,'(s19:LIBRARY INSTALLED: )W4Po,.2daW"
  "5PoW6]5}]5}'(y6:import),.1q?{${t,.5,.5d,.6a,@(y12:xform-import)[04},.0"
  "da,'0,.1V4,'1,.2V4,${'(y6:import),.3,.(i10)[02},.0~?{${.3,.(i10),'(s49"
  ":failed to import to env, import is not supported:),@(y7:x-error)[03}}"
  "@(y7:*quiet*)~,.0?{.0}{@(y9:*verbose*)}_1?{${.2,'(l3:y8:<number>;y8:<n"
  "umber>;y8:<number>;),@(y11:sexp-match?)[02}}{f}?{@(y9:*verbose*)?{Po,'"
  "(s8:IMPORT: )W4}{Po,'(s10:; import: )W4}Po,.1aW5Po,'(s24: bindings are"
  " the same, )W4Po,.1daW5Po,'(s11: modified, )W4Po,.1ddaW5Po,'(s7: added"
  "%0a)W4}_1.1,@(y30:repl-compile-and-run-core-expr)[71}.0K0?{.2,${.5,.5,"
  ".5[02},@(y18:repl-eval-top-form)[32}.0U0?{${.4,.4d,.4,@(y16:xform-inte"
  "grable)[03},@(y30:repl-compile-and-run-core-expr)[31}.0Y0?{${.4,.4,f,@"
  "(y5:xform)[03},@(y30:repl-compile-and-run-core-expr)[31}${.4,.4d,.4,@("
  "y10:xform-call)[03},@(y30:repl-compile-and-run-core-expr)[31}${.3,.3,f"
  ",@(y5:xform)[03},@(y30:repl-compile-and-run-core-expr)[21",

  "P", "repl-read",
  "%2.1?{PoW6Po,.2W4Po,'(s1: )W4}.0,@(y14:read-code-sexp)[21",

  "P", "repl-exec-command",
  "%3,,#0#1${${k0,.0,${.2,.(i12),&2{%0:1,&1{%!0.0,&1{%0:0,@(y6:values),@("
  "y13:apply-to-list)[02},:0[11},:0,&1{%0:0P50,@(y15:read-port-sexps)[01}"
  ",@(y16:call-with-values)[02},.3,&1{%1${k0,.0,${.6,&1{%0:0,Y9]1},:0[01}"
  "_1_3}[10},@(y22:with-exception-handler)[02}_1_3}[00}.!0.0^,.3c.!1${.3^"
  ",'(l2:y3:say;y5:hello;),@(y11:sexp-match?)[02}?{.4,'(s13:Well, hello!%"
  "0a)W4]5}${.3^,'(l2:y3:ref;y8:<symbol>;),@(y11:sexp-match?)[02}?{.4,${'"
  "(y3:ref),.4^a,@(y16:repl-environment)[02}W5.4W6]5}${.3^,'(l2:y3:ref;l3"
  ":y1:*;y1:*;y3:...;;),@(y11:sexp-match?)[02}?{.4,${'(y3:ref),.4^a,@(y16"
  ":repl-environment)[02}W5.4W6]5}${.3^,'(l1:y3:rnr;),@(y11:sexp-match?)["
  "02}?{.4,@(y20:*root-name-registry*)W5.4W6]5}${.3^,'(l2:y4:rref;y1:*;),"
  "@(y11:sexp-match?)[02}?{.4,${f,.4^a,@(y20:*root-name-registry*),@(y11:"
  "name-lookup)[03}W5.4W6]5}${.3^,'(l2:y5:rrem!;y1:*;),@(y11:sexp-match?)"
  "[02}?{${f,.3^a,@(y20:*root-name-registry*),@(y11:name-lookup)[03}?{${."
  "2^a,@(y20:*root-name-registry*),@(y12:name-remove!)[02}.4,'(s6:done!%0"
  "a)W4]5}.4,'(s16:name not found: )W4.4,@(y4:name)W5.4W6]5}${.3^,'(l1:y3"
  ":unr;),@(y11:sexp-match?)[02}?{.4,@(y20:*user-name-registry*)W5.4W6]5}"
  "${.3^,'(l2:y4:uref;y1:*;),@(y11:sexp-match?)[02}?{.4,${f,.4^a,@(y20:*u"
  "ser-name-registry*),@(y11:name-lookup)[03}W5.4W6]5}${.3^,'(l2:y5:urem!"
  ";y1:*;),@(y11:sexp-match?)[02}?{${f,.3^a,@(y20:*user-name-registry*),@"
  "(y11:name-lookup)[03}?{${.2^a,@(y20:*user-name-registry*),@(y12:name-r"
  "emove!)[02}.4,'(s6:done!%0a)W4]5}.4,'(s16:name not found: )W4.4,@(y4:n"
  "ame)W5.4W6]5}${.3^,'(l2:y4:peek;y1:*;),@(y11:sexp-match?)[02}?{.0^aS0?"
  "{.4,.1^aF0?{'(s12:file exists%0a)}{'(s20:file does not exist%0a)}W4]5}"
  ".0^aY0?{.4,.1^aX4F0?{'(s12:file exists%0a)}{'(s20:file does not exist%"
  "0a)}W4]5}.4,'(s37:invalid file name; use double quotes%0a)W4]5}${.3^,'"
  "(l1:y1:v;),@(y11:sexp-match?)[02}?{t@!(y9:*verbose*)]5}${.3^,'(l2:y7:v"
  "erbose;y2:on;),@(y11:sexp-match?)[02}?{t@!(y9:*verbose*)]5}${.3^,'(l2:"
  "y7:verbose;y3:off;),@(y11:sexp-match?)[02}?{f@!(y9:*verbose*)]5}${.3^,"
  "'(l1:y1:q;),@(y11:sexp-match?)[02}?{t@!(y7:*quiet*)]5}${.3^,'(l2:y5:qu"
  "iet;y2:on;),@(y11:sexp-match?)[02}?{t@!(y7:*quiet*)]5}${.3^,'(l2:y5:qu"
  "iet;y3:off;),@(y11:sexp-match?)[02}?{f@!(y7:*quiet*)]5}${.3^,'(l1:y4:h"
  "elp;),@(y11:sexp-match?)[02}?{.4,'(s20:Available commands:%0a)W4.4,'(s"
  "42: ,say hello     -- displays nice greeting%0a)W4.4,'(s40: ,peek <fna"
  "me>  -- check if file exists%0a)W4.4,'(s50: ,q             -- disable "
  "informational messages%0a)W4.4,'(s50: ,quiet on      -- disable inform"
  "ational messages%0a)W4.4,'(s49: ,quiet off     -- enable informational"
  " messages%0a)W4.4,'(s37: ,v             -- turn verbosity on%0a)W4.4,'"
  "(s37: ,verbose on    -- turn verbosity on%0a)W4.4,'(s38: ,verbose off "
  "  -- turn verbosity off%0a)W4.4,'(s38: ,verbose off   -- turn verbosit"
  "y off%0a)W4.4,'(s54: ,ref <name>    -- show current denotation for <na"
  "me>%0a)W4.4,'(s43: ,rnr           -- show root name registry%0a)W4.4,'"
  "(s48: ,rref <name>   -- lookup name in root registry%0a)W4.4,'(s50: ,r"
  "rem! <name>  -- remove name from root registry%0a)W4.4,'(s43: ,unr    "
  "       -- show user name registry%0a)W4.4,'(s48: ,uref <name>   -- loo"
  "kup name in user registry%0a)W4.4,'(s50: ,urem! <name>  -- remove name"
  " from user registry%0a)W4.4,'(s29: ,help          -- this help%0a)W4]5"
  "}.4,'(s29:syntax error in repl command%0a)W4.4,'(s37:type ,help to see"
  " available commands%0a)W4]5",

  "P", "repl-from-port",
  "%3,#0${@(y18:current-file-stack)[00}.!0${k0,.0,${.2,.9,.(i11),.(i10),&"
  "4{%0:3,&1{%!0.0,&1{%0:0,@(y6:values),@(y13:apply-to-list)[02},:0[11},:"
  "0,:1,:2,&3{%0${:1,:2,@(y9:repl-read)[02},,#0:0,:2,:1,.3,&4{%1.0R8~?{:1"
  "?{${.2,'(l2:y7:unquote;y1:*;),@(y11:sexp-match?)[02}}{f}?{${Po,${:2,@("
  "y9:read-line)[01},.4da,@(y17:repl-exec-command)[03}}{${:3,.3,@(y18:rep"
  "l-eval-top-form)[02}}${:1,:2,@(y9:repl-read)[02},:0^[11}]1}.!0.0^_1[01"
  "},@(y16:call-with-values)[02},.(i10),.(i10),.(i10),.(i10),.7,&5{%1${k0"
  ",.0,${.6,:1,:2,:3,:4,&5{%0:4,${.2,@(y13:error-object?)[01}?{Pe,.0,${.4"
  ",@(y20:error-object-message)[01}W4.0W6${${.5,@(y22:error-object-irrita"
  "nts)[01},.3,&1{%1:0,.1W5:0W6]1},@(y10:%25for-each1)[02}_1${:3^,@(y23:s"
  "et-current-file-stack!)[01}:0?{:0,:1,:2,@(y14:repl-from-port)[13}]1}Pe"
  ",.0,'(s14:Unknown error:)W4.0W6.0,.2W5.0W6_1${:3^,@(y23:set-current-fi"
  "le-stack!)[01}:0?{:0,:1,:2,@(y14:repl-from-port)[13}]1},:0[01}_1_3}[10"
  "},@(y22:with-exception-handler)[02}_1_3}[40",

  "P", "repl-file",
  "%2,#0${.3,@(y15:open-input-file)[01}.!0${f,.5,.4^,@(y14:repl-from-port"
  ")[03}.0^P60]3",

  "P", "benchmark-file",
  "%1,#0${.3,@(y15:open-input-file)[01}.!0${${.4^,@(y14:read-code-sexp)[0"
  "1},'(l2:y4:load;s7:libl.sf;),@(y11:sexp-match?)[02}~?{${.3,'(s32:unexp"
  "ected benchmark file format),@(y5:error)[02}}${f,@(y16:repl-environmen"
  "t),.4^,@(y14:repl-from-port)[03}${@(y16:repl-environment),'(l2:y4:main"
  ";f;),@(y18:repl-eval-top-form)[02}.0^P60]2",

  "P", "repl",
  "%0${n,@(y23:set-current-file-stack!)[01}'(s6:skint]),@(y16:repl-enviro"
  "nment),Pi,@(y14:repl-from-port)[03",

  0, 0, 0
};
