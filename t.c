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
  "l6:y12:syntax-rules;l1:y4:else;;l2:l4:y11:record-case;py2:pa;y2:ir;;y6"
  ":clause;y3:...;;l3:y3:let;l1:l2:y2:id;py2:pa;y2:ir;;;;l4:y11:record-ca"
  "se;y2:id;y6:clause;y3:...;;;;l2:l2:y11:record-case;y2:id;;l2:y5:quote;"
  "y16:record-case-miss;;;l2:l3:y11:record-case;y2:id;l3:y4:else;y3:exp;y"
  "3:...;;;l3:y5:begin;y3:exp;y3:...;;;l2:l5:y11:record-case;y2:id;l4:y3:"
  "key;y3:ids;y3:exp;y3:...;;y6:clause;y3:...;;l4:y2:if;l3:y3:eq?;l2:y3:c"
  "ar;y2:id;;l2:y5:quote;y3:key;;;l3:y5:apply;l4:y6:lambda;y3:ids;y3:exp;"
  "y3:...;;l2:y3:cdr;y2:id;;;l4:y11:record-case;y2:id;y6:clause;y3:...;;;"
  ";",

  "P", "syntax-match?",
  "%2'(y1:*),.1q,.0?{.0]3}.1,.3e,.0?{.0]4}.2p?{'(y1:$),.3aq?{.2dp?{.2ddu}"
  "{f}}{f}?{.2da,.4q]4}.2dp?{'(y3:...),.3daq?{.2ddu}{f}}{f}?{.2a,,#0.0,.2"
  ",&2{%1.0u,.0?{.0]2}.1p?{${.3a,:0,@(y13:syntax-match?)[02}?{.1d,:1^[21}"
  "f]2}f]2}.!0.5,.1^[61}.3p?{${.5a,.5a,@(y13:syntax-match?)[02}?{.3d,.3d,"
  "@(y13:syntax-match?)[42}f]4}f]4}f]4",

  "C", 0,
  "'0,#0.0,&1{%!0'1,:0^I+:!0.0u?{'(i10),:0^X6,'(s1:#)S6X5]1}.0aY0?{'(i10)"
  ",:0^X6,'(s1:#)S6,.1aX4S6X5]1}'0:!0]1}_1@!(y6:gensym)",

  "P", "posq",
  "%2'0,.2,,#0.0,.4,&2{%2.0u?{f]2}.0a,:0q?{.1]2}'1,.2I+,.1d,:1^[22}.!0.0^"
  "_1[22",

  "P", "list-diff",
  "%2.0u,.0?{.0}{.2,.2q}_1?{n]2}${.3,.3d,@(y9:list-diff)[02},.1ac]2",

  "P", "pair*",
  "%!1.0,.2,,#0.0,&1{%2.1u?{.0]2}${.3d,.4a,:0^[02},.1c]2}.!0.0^_1[22",

  "P", "andmap",
  "%2.1p?{${.3a,.3[01}?{.1d,.1,@(y6:andmap)[22}f]2}t]2",

  "P", "list1?",
  "%1.0p?{.0du]1}f]1",

  "P", "list1+?",
  "%1.0p?{.0dL0]1}f]1",

  "P", "list2?",
  "%1.0p?{.0d,@(y6:list1?)[11}f]1",

  "P", "list2+?",
  "%1.0p?{.0d,@(y7:list1+?)[11}f]1",

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

  "A", "val-core?", "pair?",

  "A", "binding?", "pair?",

  "A", "make-binding", "cons",

  "A", "binding-val", "cdr",

  "P", "binding-special?",
  "%1.0dp~]1",

  "A", "binding-sym", "car",

  "A", "binding-set-val!", "set-cdr!",

  "P", "new-id",
  "%1,#0.1,l1.!0.0,&1{%0:0^]0}]2",

  "P", "old-den",
  "%1${.2[00}a]1",

  "P", "id?",
  "%1.0Y0,.0?{.0]2}.1K0]2",

  "P", "id->sym",
  "%1.0Y0?{.0]1}${.2,@(y7:old-den)[01},@(y8:den->sym)[11",

  "P", "den->sym",
  "%1.0Y0?{.0]1}.0a]1",

  "P", "extend-xenv",
  "%3.0,.3,.3,&3{%1.0,:0q?{:1]1}.0,:2[11}]3",

  "P", "add-binding",
  "%3.1,${.3,@(y7:id->sym)[01}c,.1,.4,@(y11:extend-xenv)[33",

  "P", "add-var",
  "%3.1,'(y3:ref),l2,${.3,@(y7:id->sym)[01}c,.1,.4,@(y11:extend-xenv)[33",

  "P", "xform-sexp->datum",
  "%1.0,,#0.0,&1{%1${.2,@(y3:id?)[01}?{.0,@(y7:id->sym)[11}.0p?{${.2d,:0^"
  "[01},${.3a,:0^[01}c]1}.0V0?{${.2X0,:0^,@(y5:%25map1)[02}X1]1}.0]1}.!0."
  "0^_1[11",

  "P", "x-error",
  "%!1.0,.2,'(s13:transformer: )S6,@(y6:error*)[22",

  "P", "xform",
  "%3${.3,@(y3:id?)[01}?{${.4,.4,@(y9:xform-ref)[02},.1?{.0]4}.0U0?{.0U7,"
  "'(y3:ref),l2]4}.0K0?{.3,${.6,.6,.5[02},.3,@(y5:xform)[43}.0p~?{.0,'(s2"
  "7:improper use of syntax form),@(y7:x-error)[42}.0]4}.1p~?{.2,.2,l1,@("
  "y11:xform-quote)[32}.1a,.2d,${.6,.4,t,@(y5:xform)[03},.0,'(y6:syntax),"
  ".1v?{.2a]7}'(y5:quote),.1v?{.6,.3,@(y11:xform-quote)[72}'(y4:set!),.1v"
  "?{.6,.3,@(y10:xform-set!)[72}'(y4:set&),.1v?{.6,.3,@(y10:xform-set&)[7"
  "2}'(y2:if),.1v?{.6,.3,@(y8:xform-if)[72}'(y6:lambda),.1v?{.6,.3,@(y12:"
  "xform-lambda)[72}'(y7:lambda*),.1v?{.6,.3,@(y13:xform-lambda*)[72}'(y5"
  ":letcc),.1v?{.6,.3,@(y11:xform-letcc)[72}'(y6:withcc),.1v?{.6,.3,@(y12"
  ":xform-withcc)[72}'(y4:body),.1v?{.6,.3,@(y10:xform-body)[72}'(y5:begi"
  "n),.1v?{.6,.3,@(y11:xform-begin)[72}'(y6:define),.1v?{.6,.3,@(y12:xfor"
  "m-define)[72}'(y13:define-syntax),.1v?{.6,.3,@(y19:xform-define-syntax"
  ")[72}'(y13:syntax-lambda),.1v?{.6,.3,@(y19:xform-syntax-lambda)[72}'(y"
  "12:syntax-rules),.1v?{.6,.3,@(y18:xform-syntax-rules)[72}'(y13:syntax-"
  "length),.1v?{.6,.3,@(y19:xform-syntax-length)[72}'(y12:syntax-error),."
  "1v?{.6,.3,@(y18:xform-syntax-error)[72}.1U0?{.6,.3,.3,@(y16:xform-inte"
  "grable)[73}.1K0?{.6,${.9,.9,.6[02},.6,@(y5:xform)[73}.6,.3,.3,@(y10:xf"
  "orm-call)[73",

  "P", "xform-ref",
  "%2${.2,.4[01},.0Y0?{.0,'(s14:unexpected den),@(y7:x-error)[32}'(y3:..."
  "),.1dq?{'(s19:improper use of ...),@(y7:x-error)[31}.0d]3",

  "P", "xform-quote",
  "%2${.2,@(y6:list1?)[01}?{${.2a,@(y17:xform-sexp->datum)[01},'(y5:quote"
  "),l2]2}.0,'(y5:quote)c,'(s19:improper quote form),@(y7:x-error)[22",

  "P", "xform-set!",
  "%2${.2,@(y6:list2?)[01}?{${.2a,@(y3:id?)[01}}{f}?{${.3,.3da,f,@(y5:xfo"
  "rm)[03},${.3a,.5[01},.0Y0?{.0,'(s22:unexpected den in set!),@(y7:x-err"
  "or)[42}${.2,@(y16:binding-special?)[01}?{.1,.1sd'(l1:y5:begin;)]4}.0d,"
  "'(y3:ref),.1aq?{.2,.1da,'(y4:set!),l3]5}'(s27:set! to non-identifier f"
  "orm),@(y7:x-error)[51}.0,'(y4:set!)c,'(s18:improper set! form),@(y7:x-"
  "error)[22",

  "P", "xform-set&",
  "%2${.2,@(y6:list1?)[01}?{${.2a,.4[01},.0Y0?{.0,'(s22:unexpected den in"
  " set&),@(y7:x-error)[32}${.2,@(y16:binding-special?)[01}?{'(s22:set& o"
  "f a non-variable),@(y7:x-error)[31}.0d,'(y3:ref),.1aq?{.0da,'(y4:set&)"
  ",l2]4}'(s22:set& of a non-variable),@(y7:x-error)[41}.0,'(y4:set&)c,'("
  "s18:improper set& form),@(y7:x-error)[22",

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
  "6,.4,.6,@(y7:add-var)[03},.4d,:0^[53}.0u?{${.3,:1d,@(y10:xform-body)[0"
  "2},.3A8,'(y6:lambda),l3]3}.0,${${.4,@(y7:id->sym)[01},@(y6:gensym)[01}"
  ",${.5,.3,.5,@(y7:add-var)[03},${.2,:1d,@(y10:xform-body)[02},.2,.7A8L6"
  ",'(y6:lambda),l3]6}.!0.0^_1[23}.0,'(y6:lambda)c,'(s20:improper lambda "
  "body),@(y7:x-error)[22",

  "P", "xform-lambda*",
  "%2.0L0?{${.2,.4,&1{%1${.2,@(y6:list2?)[01}?{${.2a,@(y6:list2?)[01}?{.0"
  "aaI0?{.0adaY1}{f}}{f},.0?{.0}{${.3a,@(y8:idslist?)[01}}_1}{f}?{${:0,.3"
  "da,f,@(y5:xform)[03},${.3a,@(y15:normalize-arity)[01},l2]1}.0,'(s23:im"
  "proper lambda* clause),@(y7:x-error)[12},@(y5:%25map1)[02},'(y7:lambda"
  "*)c]2}.0,'(y7:lambda*)c,'(s21:improper lambda* form),@(y7:x-error)[22",

  "P", "xform-letcc",
  "%2${.2,@(y7:list2+?)[01}?{${.2a,@(y3:id?)[01}}{f}?{.0a,${${.4,@(y7:id-"
  ">sym)[01},@(y6:gensym)[01},${${.7,.5,.7,@(y7:add-var)[03},.5d,@(y10:xf"
  "orm-body)[02},.1,'(y5:letcc),l3]4}.0,'(y5:letcc)c,'(s19:improper letcc"
  " form),@(y7:x-error)[22",

  "P", "xform-withcc",
  "%2${.2,@(y7:list2+?)[01}?{${.3,.3d,@(y10:xform-body)[02},${.4,.4a,f,@("
  "y5:xform)[03},'(y6:withcc),l3]2}.0,'(y6:withcc)c,'(s20:improper withcc"
  " form),@(y7:x-error)[22",

  "P", "xform-body",
  "%2.0u?{'(y5:begin),l1]2}${.2,@(y6:list1?)[01}?{.1,.1a,f,@(y5:xform)[23"
  "}.0L0~?{.0,'(y4:body)c,'(s18:improper body form),@(y7:x-error)[22}.0,n"
  ",n,n,.5,,#0.0,&1{%5.4p?{.4ap}{f}?{.4d,.5a,.0a,.1d,${.6,.4,t,@(y5:xform"
  ")[03},.0,'(y5:begin),.1v?{.2L0?{.5,.3L6,.(i10),.(i10),.(i10),.(i10),:0"
  "^[(i11)5}.4,'(s19:improper begin form),@(y7:x-error)[(i11)2}'(y6:defin"
  "e),.1v?{${.4,@(y6:list2?)[01}?{.2au}{f}?{.2da,.6,.(i11),fc,.(i11),.3c,"
  ".(i11),fc,.(i11),:0^[(i12)5}${.4,@(y6:list2?)[01}?{${.4a,@(y3:id?)[01}"
  "}{f}?{.2a,.3da,${${.5,@(y7:id->sym)[01},@(y6:gensym)[01},${.(i11),.3,."
  "6,@(y7:add-var)[03},.9,.(i14),.3c,.(i14),.5c,.(i14),.7c,.4,:0^[(i15)5}"
  "${.4,@(y7:list2+?)[01}?{.2ap?{${.4aa,@(y3:id?)[01}?{${.4ad,@(y8:idslis"
  "t?)[01}}{f}}{f}}{f}?{.2aa,${'(y6:lambda),'(y6:lambda)c,@(y6:new-id)[01"
  "},.4d,.5adc,.1c,${${.6,@(y7:id->sym)[01},@(y6:gensym)[01},${.(i12),.3,"
  ".7,@(y7:add-var)[03},.(i10),.(i15),.3c,.(i15),.5c,.(i15),.8c,.4,:0^[(i"
  "16)5}.4,'(s20:improper define form),@(y7:x-error)[(i11)2}'(y13:define-"
  "syntax),.1v?{${.4,@(y6:list2?)[01}?{${.4a,@(y3:id?)[01}}{f}?{.2a,.3da,"
  "${.(i10),'(l1:y9:undefined;),.5,@(y11:add-binding)[03},.8,.(i13),tc,.("
  "i13),.4c,.(i13),.6c,.4,:0^[(i14)5}.4,'(s27:improper define-syntax form"
  "),@(y7:x-error)[(i11)2}.1K0?{.5,${.9,.8,.6[02}c,.(i10),.(i10),.(i10),."
  "(i10),:0^[(i11)5}.6,.(i11),.(i11)A8,.(i11)A8,.(i11)A8,@(y12:xform-labe"
  "ls)[(i11)5}.0,.5,.5A8,.5A8,.5A8,@(y12:xform-labels)[55}.!0.0^_1[25",

  "P", "xform-labels",
  "%5n,n,.4,.4,.4,,#0.0,.(i11),.(i11),&3{%5.0u?{${:0,:1,&1{%1:0,.1,f,@(y5"
  ":xform)[13},@(y5:%25map1)[02},.4A8L6,${.2,@(y6:list1?)[01}?{.0a}{.0,'("
  "y5:begin)c},.6u?{.0]7}${.8,&0{%1'(l1:y5:begin;)]1},@(y5:%25map1)[02},."
  "1,.8A8,'(y6:lambda),l3,'(y4:call),@(y5:pair*)[73}.0a~?{.4,.4,${:1,.6a,"
  "f,@(y5:xform)[03}c,.4d,.4d,.4d,:2^[55}.2aY0?{.4,.3ac,.4,${:1,.6a,.6a,l"
  "2,@(y10:xform-set!)[02}c,.4d,.4d,.4d,:2^[55}${:1,.4a,t,@(y5:xform)[03}"
  ",${.3a,:1[01}sd.4,.4,.4d,.4d,.4d,:2^[55}.!0.0^_1[55",

  "P", "xform-begin",
  "%2.0L0?{${.2,.4,&1{%1:0,.1,f,@(y5:xform)[13},@(y5:%25map1)[02},.0p?{.0"
  "du}{f}?{.0a]3}.0,'(y5:begin)c]3}.0,'(y6:begin!)c,'(s19:improper begin "
  "form),@(y7:x-error)[22",

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

  "P", "xform-syntax-lambda",
  "%2${.2,@(y7:list2+?)[01}?{${.2a,@(y3:id?),@(y6:andmap)[02}}{f}?{.0d,.2"
  ",.2a,.2,.1,.3,&3{%2${.2,@(y7:list1+?)[01}?{.0dg,:1gI=}{f}?{:0,.1d,:1,,"
  "#0.5,.1,:2,&3{%3.0u?{${.4,:0,@(y10:xform-body)[02},'(y6:syntax),l2]3}$"
  "{.4,${:2,.7a,t,@(y5:xform)[03},.4a,@(y11:add-binding)[03},.2d,.2d,:1^["
  "33}.!0.0^_1[23}.0,'(s33:invalif syntax-lambda application),@(y7:x-erro"
  "r)[22}]5}.0,'(y13:syntax-lambda)c,'(s27:improper syntax-lambda body),@"
  "(y7:x-error)[22",

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
  "}~]1}.!1.4,&1{%1.0p?{.0a,:0^[11}f]1}.!2&0{%1.0p?{'(y3:...),.1dq]1}f]1}"
  ".!3.3,.9,.(i11),&3{%1:0?{:0,.1q]1}${.2,@(y3:id?)[01}?{${.2,:1[01},:2^["
  "11}f]1}.!4.2,&1{%3n,.2,.2,,#0:0,.1,.8,&3{%3${.2,@(y3:id?)[01}?{.1?{${."
  "2,:0[01}}{f}?{.2,.1c]3}.2]3}.0V0?{.2,.2,.2X0,:1^[33}.0p?{${.2d,:2^[01}"
  "?{${.4,.4,.4dd,:1^[03},t,.2a,:1^[33}${.4,.4,.4d,:1^[03},.2,.2a,:1^[33}"
  ".2]3}.!0.0^_1[33}.!5.5,.2,.4,.3,.(i12),&5{%3k3,.0,,#0.1,&1{%0f,:0[01}."
  "!0n,.5,.5,,#0.4,.1,:4,:3,:2,:1,:0,.(i16),&8{%3,#0:7,.4,&2{%1.0?{:0]1}:"
  "1^[10}.!0${.3,@(y3:id?)[01}?{${.3,:2^[01}?{${.4,@(y3:id?)[01}?{${.3,:1"
  "[01},${.5,:0[01}q}{f},.1^[41}.3,.3,.3cc]4}.1V0?{.2V0,.0?{.0}{${:7^[00}"
  "}_1.3,.3X0,.3X0,:6^[43}.1p~?{.2,.2e,.1^[41}${.3d,:3^[01}?{.1ddg,.3L0?{"
  ".3g}{${:7^[00}},.1,.1I-,.0<0?{${:7^[00}}.0,.6A6,.3,.7A8A6A8,${:4^,t,.("
  "i10)a,:5^[03},,#0:6,.9,&2{%1${n,.3,:0a,:1^[03},@(y3:cdr),@(y5:%25map1)"
  "[12}.!0${.(i12),.6,.(i12)dd,:6^[03},${${.7,.6^,@(y5:%25map1)[02},.5c,@"
  "(y4:list)c,@(y4:%25map),@(y13:apply-to-list)[02}L6](i11)}.2p?{${.5,.5d"
  ",.5d,:6^[03},.3a,.3a,:6^[43}:7^[40}.!0.0^_1[63}.!6.8,.2,.7,.5,&4{%3,,,"
  "#0#1#2${${.9,&1{%1:0,.1A3~]1},t,.(i10),:1^[03},:3,&1{%1${${.4,:0[01},@"
  "(y6:new-id)[01},.1c]1},@(y5:%25map1)[02}.!0${:2^,f,.7,:1^[03}.!1.1,:1,"
  "&2{%1:1,&1{%1:0^,.1A0]1},t,.2,:0^[13}.!2.5,.5,,#0.8,.4,:0,.8,.4,&5{%2."
  "0,,#0.0,.4,:0,:1,:2,:3,:4,&7{%1${.2,@(y3:id?)[01}?{:5,.1A3,.0?{.0}{:0,"
  ".2A3,.0?{.0}{:1^,.3A3}_1}_1d]1}.0V0?{${.2X0,:6^[01}X1]1}.0p?{${.2d,:2^"
  "[01}}{f}?{${.2a,:3^[01},,,#0#1:5,&1{%1:0,.1A3d]1}.!0.2,.4,:4,&3{%!0${."
  "2,:2,@(y4:cons),@(y5:%25map2)[03},:1a,:0^[12}.!1.2u?{${.5dd,:6^[01},${"
  ".6a,:6^[01}c]4}${.4,.3^,@(y5:%25map1)[02},${.6dd,:6^[01},${.3,.6^c,@(y"
  "4:%25map),@(y13:apply-to-list)[02}L6]5}.0p?{${.2d,:6^[01},${.3a,:6^[01"
  "}c]1}.0]1}.!0.0^_1[21}.!0.0^_1[62}.!7.(i11),.8,.8,&3{%2:2,,#0:0,.3,.5,"
  ":1,.4,&5{%1.0u?{${:3,'(s14:invalid syntax),@(y7:x-error)[02}}.0a,.0a,."
  "1da,${:2,:3,.5,:4^[03},.0?{.0,.0,.3,.5,:1^[63}.4d,:0^[51}.!0.0^_1[21}]"
  "(i12)",

  "P", "lookup-in-transformer-env",
  "%2.0K0?{.0,@(y7:old-den)[21}.1,,#0.2,.1,&2{%1.0p?{:1,.1aaq?{.0a]1}.0d,"
  ":0^[11}t,.1q?{:1U5,.0?{.0}{:1,'(y3:ref),l2}_1,:1c,@(y10:*root-env*),.1"
  "c@!(y10:*root-env*).0]2}f]1}.!0.0^_1[21",

  "C", 0,
  "${t,U1,,#0.0,&1{%2.0u?{.1]2}.0d,.1a,.0d,.1a,.1Y0,.0?{.0}{.2N0}_1?{.5,."
  "2,.2cc,.4,:0^[62}.1p?{'(y12:syntax-rules),.2aq}{f}?{,,#0#1&0{%1@(y10:*"
  "root-env*),.1,@(y25:lookup-in-transformer-env)[12}.!0${.5da,@(y3:id?)["
  "01}?{${.5ddd,.6dda,.7da,.5^,@(y13:syntax-rules*)[04}}{${.5dd,.6da,f,.5"
  "^,@(y13:syntax-rules*)[04}}.!1.7,.2^,.4cc,.6,:0^[82}.5,.2,'(y1:?),.3,l"
  "3c,.4,:0^[62}.!0.0^_1[02}@!(y10:*root-env*)",

  "P", "root-env",
  "%1@(y10:*root-env*),.1,@(y25:lookup-in-transformer-env)[12",

  "P", "error*",
  "%2.1,.1c,@(y5:error),@(y13:apply-to-list)[22",

  "P", "transform!",
  "%1${@(y8:root-env),.3,t,@(y5:xform)[03},${.2,'(l3:y13:define-syntax;y1"
  ":*;y1:*;),@(y13:syntax-match?)[02}?{${.2da,@(y3:id?)[01}}{f}?{${@(y10:"
  "*root-env*),.3da,@(y25:lookup-in-transformer-env)[02},.0?{.1dda,.1sd}_"
  "1}.0]2",

  "P", "visit",
  "%1,#0${.3,@(y15:open-input-file)[01}.!0${${.4^,@(y4:read)[01},,#0.4,.1"
  ",&2{%1.0R8~?{${.2,@(y10:transform!)[01},Po,.1W5PoW6_1${:1^,@(y4:read)["
  "01},:0^[11}]1}.!0.0^_1[01}.0^P60]2",

  0, 0, 0
};
