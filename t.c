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

  "A", "make-location", "box",

  "A", "location-val", "unbox",

  "A", "location-set-val!", "set-box!",

  "P", "location-special?",
  "%1.0zp~]1",

  "P", "new-id",
  "%2,#0.2,.2c.!0.0,&1{%0:0^]0}]3",

  "P", "old-sym",
  "%1${.2[00}a]1",

  "P", "old-den",
  "%1${.2[00}d]1",

  "P", "id?",
  "%1.0Y0,.0?{.0]2}.1K0]2",

  "P", "id->sym",
  "%1.0Y0?{.0]1}.0,@(y7:old-sym)[11",

  "P", "extend-xenv",
  "%3.0,.3,.3,&3{%1.0,:0q?{:1]1}.0,:2[11}]3",

  "P", "add-location",
  "%3.1b,.1,.4,@(y11:extend-xenv)[33",

  "P", "add-var",
  "%3.1,'(y3:ref),l2b,.1,.4,@(y11:extend-xenv)[33",

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
  "%2${.2,.4[01},'(y3:...),.1zq?{'(s19:improper use of ...),@(y7:x-error)"
  "[31}.0z]3",

  "P", "xform-quote",
  "%2${.2,@(y6:list1?)[01}?{${.2a,@(y17:xform-sexp->datum)[01},'(y5:quote"
  "),l2]2}.0,'(y5:quote)c,'(s19:improper quote form),@(y7:x-error)[22",

  "P", "xform-set!",
  "%2${.2,@(y6:list2?)[01}?{${.2a,@(y3:id?)[01}}{f}?{${.3,.3da,f,@(y5:xfo"
  "rm)[03},${.3a,.5[01},${.2,@(y17:location-special?)[01}?{.1,.1sz'(l1:y5"
  ":begin;)]4}.0z,'(y3:ref),.1aq?{.2,.1da,'(y4:set!),l3]5}'(s27:set! to n"
  "on-identifier form),@(y7:x-error)[51}.0,'(y4:set!)c,'(s18:improper set"
  "! form),@(y7:x-error)[22",

  "P", "xform-set&",
  "%2${.2,@(y6:list1?)[01}?{${.2a,.4[01},${.2,@(y17:location-special?)[01"
  "}?{'(s22:set& of a non-variable),@(y7:x-error)[31}.0z,'(y3:ref),.1aq?{"
  ".0da,'(y4:set&),l2]4}'(s22:set& of a non-variable),@(y7:x-error)[41}.0"
  ",'(y4:set&)c,'(s18:improper set& form),@(y7:x-error)[22",

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
  "t?)[01}}{f}}{f}}{f}?{.2aa,${'(y6:lambda)b,'(y6:lambda),@(y6:new-id)[02"
  "},.4d,.5adc,.1c,${${.6,@(y7:id->sym)[01},@(y6:gensym)[01},${.(i12),.3,"
  ".7,@(y7:add-var)[03},.(i10),.(i15),.3c,.(i15),.5c,.(i15),.8c,.4,:0^[(i"
  "16)5}.4,'(s20:improper define form),@(y7:x-error)[(i11)2}'(y13:define-"
  "syntax),.1v?{${.4,@(y6:list2?)[01}?{${.4a,@(y3:id?)[01}}{f}?{.2a,.3da,"
  "${.(i10),'(l1:y9:undefined;),.5,@(y12:add-location)[03},.8,.(i13),tc,."
  "(i13),.4c,.(i13),.6c,.4,:0^[(i14)5}.4,'(s27:improper define-syntax for"
  "m),@(y7:x-error)[(i11)2}.1K0?{.5,${.9,.8,.6[02}c,.(i10),.(i10),.(i10),"
  ".(i10),:0^[(i11)5}.6,.(i11),.(i11)A8,.(i11)A8,.(i11)A8,@(y12:xform-lab"
  "els)[(i11)5}.0,.5,.5A8,.5A8,.5A8,@(y12:xform-labels)[55}.!0.0^_1[25",

  "P", "xform-labels",
  "%5n,n,.4,.4,.4,,#0.0,.(i11),.(i11),&3{%5.0u?{${:0,:1,&1{%1:0,.1,f,@(y5"
  ":xform)[13},@(y5:%25map1)[02},.4A8L6,${.2,@(y6:list1?)[01}?{.0a}{.0,'("
  "y5:begin)c},.6u?{.0]7}${.8,&0{%1'(l1:y5:begin;)]1},@(y5:%25map1)[02},."
  "1,.8A8,'(y6:lambda),l3,'(y4:call),@(y5:pair*)[73}.0a~?{.4,.4,${:1,.6a,"
  "f,@(y5:xform)[03}c,.4d,.4d,.4d,:2^[55}.2aY0?{.4,.3ac,.4,${:1,.6a,.6a,l"
  "2,@(y10:xform-set!)[02}c,.4d,.4d,.4d,:2^[55}${:1,.4a,t,@(y5:xform)[03}"
  ",${.3a,:1[01}sz.4,.4,.4d,.4d,.4d,:2^[55}.!0.0^_1[55",

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
  "{.4,${:2,.7a,t,@(y5:xform)[03},.4a,@(y12:add-location)[03},.2d,.2d,:1^"
  "[33}.!0.0^_1[23}.0,'(s33:invalif syntax-lambda application),@(y7:x-err"
  "or)[22}]5}.0,'(y13:syntax-lambda)c,'(s27:improper syntax-lambda body),"
  "@(y7:x-error)[22",

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
  ".(i11),&3{%1:0?{:0,.1q]1}${.2,@(y3:id?)[01}?{${.2,:1[01},:2^[11}f]1}.!"
  "4.2,&1{%3n,.2,.2,,#0:0,.1,.8,&3{%3${.2,@(y3:id?)[01}?{.1?{${.2,:0[01}}"
  "{f}?{.2,.1c]3}.2]3}.0V0?{.2,.2,.2X0,:1^[33}.0p?{${.2d,:2^[01}?{${.4,.4"
  ",.4dd,:1^[03},t,.2a,:1^[33}${.4,.4,.4d,:1^[03},.2,.2a,:1^[33}.2]3}.!0."
  "0^_1[33}.!5.5,.2,.4,.3,.(i12),&5{%3k3,.0,,#0.1,&1{%0f,:0[01}.!0n,.5,.5"
  ",,#0.4,.1,:4,:3,:2,:1,:0,.(i16),&8{%3,#0:7,.4,&2{%1.0?{:0]1}:1^[10}.!0"
  "${.3,@(y3:id?)[01}?{${.3,:2^[01}?{${.4,@(y3:id?)[01}?{${.3,:1[01},${.5"
  ",:0[01}q}{f},.1^[41}.3,.3,.3cc]4}.1V0?{.2V0,.0?{.0}{${:7^[00}}_1.3,.3X"
  "0,.3X0,:6^[43}.1p~?{.2,.2e,.1^[41}${.3d,:3^[01}?{.1ddg,.3L0?{.3g}{${:7"
  "^[00}},.1,.1I-,.0<0?{${:7^[00}}.0,.6A6,.3,.7A8A6A8,${:4^,t,.(i10)a,:5^"
  "[03},,#0:6,.9,&2{%1${n,.3,:0a,:1^[03},@(y3:cdr),@(y5:%25map1)[12}.!0${"
  ".(i12),.6,.(i12)dd,:6^[03},${${.7,.6^,@(y5:%25map1)[02},.5c,@(y4:list)"
  "c,@(y4:%25map),@(y13:apply-to-list)[02}L6](i11)}.2p?{${.5,.5d,.5d,:6^["
  "03},.3a,.3a,:6^[43}:7^[40}.!0.0^_1[63}.!6.8,.2,.7,.5,&4{%3,,,#0#1#2${$"
  "{.9,&1{%1:0,.1A3~]1},t,.(i10),:1^[03},:3,&1{%1${${.4,:0[01},${.5,@(y7:"
  "id->sym)[01},@(y6:new-id)[02},.1c]1},@(y5:%25map1)[02}.!0${:2^,f,.7,:1"
  "^[03}.!1.1,:1,&2{%1:1,&1{%1:0^,.1A0]1},t,.2,:0^[13}.!2.5,.5,,#0.8,.4,:"
  "0,.8,.4,&5{%2.0,,#0.0,.4,:0,:1,:2,:3,:4,&7{%1${.2,@(y3:id?)[01}?{:5,.1"
  "A3,.0?{.0}{:0,.2A3,.0?{.0}{:1^,.3A3}_1}_1d]1}.0V0?{${.2X0,:6^[01}X1]1}"
  ".0p?{${.2d,:2^[01}}{f}?{${.2a,:3^[01},,,#0#1:5,&1{%1:0,.1A3d]1}.!0.2,."
  "4,:4,&3{%!0${.2,:2,@(y4:cons),@(y5:%25map2)[03},:1a,:0^[12}.!1.2u?{${."
  "5dd,:6^[01},${.6a,:6^[01}c]4}${.4,.3^,@(y5:%25map1)[02},${.6dd,:6^[01}"
  ",${.3,.6^c,@(y4:%25map),@(y13:apply-to-list)[02}L6]5}.0p?{${.2d,:6^[01"
  "},${.3a,:6^[01}c]1}.0]1}.!0.0^_1[21}.!0.0^_1[62}.!7.(i11),.8,.8,&3{%2:"
  "2,,#0:0,.3,.5,:1,.4,&5{%1.0u?{${:3,'(s14:invalid syntax),@(y7:x-error)"
  "[02}}.0a,.0a,.1da,${:2,:3,.5,:4^[03},.0?{.0,.0,.3,.5,:1^[63}.4d,:0^[51"
  "}.!0.0^_1[21}](i12)",

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

  "P", "find-free*",
  "%2.0u?{n]2}${.3,.3d,@(y10:find-free*)[02},${.4,.4a,@(y9:find-free)[02}"
  ",@(y9:set-union)[22",

  "P", "find-free",
  "%2'(y5:quote),.1aq?{.0d,&0{%1n]1},@(y13:apply-to-list)[22}'(y3:ref),.1"
  "aq?{.0d,.2,&1{%1${:0,.3,@(y11:set-member?)[02}?{n]1}.0,l1]1},@(y13:app"
  "ly-to-list)[22}'(y4:set!),.1aq?{.0d,.2,&1{%2${:0,.4,@(y9:find-free)[02"
  "},${:0,.4,@(y11:set-member?)[02}?{n}{.1,l1},@(y9:set-union)[22},@(y13:"
  "apply-to-list)[22}'(y4:set&),.1aq?{.0d,.2,&1{%1${:0,.3,@(y11:set-membe"
  "r?)[02}?{n]1}.0,l1]1},@(y13:apply-to-list)[22}'(y6:lambda),.1aq?{.0d,."
  "2,&1{%2${:0,${.5,@(y15:flatten-idslist)[01},@(y9:set-union)[02},.2,@(y"
  "9:find-free)[22},@(y13:apply-to-list)[22}'(y7:lambda*),.1aq?{.0d,.2,&1"
  "{%!0:0,${.3,@(y4:cadr),@(y5:%25map1)[02},@(y10:find-free*)[12},@(y13:a"
  "pply-to-list)[22}'(y5:letcc),.1aq?{.0d,.2,&1{%2${:0,.3,l1,@(y9:set-uni"
  "on)[02},.2,@(y9:find-free)[22},@(y13:apply-to-list)[22}'(y6:withcc),.1"
  "aq?{.0d,.2,&1{%2${:0,.4,@(y9:find-free)[02},${:0,.4,@(y9:find-free)[02"
  "},@(y9:set-union)[22},@(y13:apply-to-list)[22}'(y2:if),.1aq?{.0d,.2,&1"
  "{%3${${:0,.7,@(y9:find-free)[02},${:0,.7,@(y9:find-free)[02},@(y9:set-"
  "union)[02},${:0,.4,@(y9:find-free)[02},@(y9:set-union)[32},@(y13:apply"
  "-to-list)[22}'(y5:begin),.1aq?{.0d,.2,&1{%!0:0,.1,@(y10:find-free*)[12"
  "},@(y13:apply-to-list)[22}'(y10:integrable),.1aq?{.0d,.2,&1{%!1:0,.1,@"
  "(y10:find-free*)[22},@(y13:apply-to-list)[22}'(y4:call),.1aq?{.0d,.2,&"
  "1{%!1${:0,.3,@(y10:find-free*)[02},${:0,.5,@(y9:find-free)[02},@(y9:se"
  "t-union)[22},@(y13:apply-to-list)[22}'(y6:define),.1aq?{.0d,.1,&1{%!0:"
  "0,'(s21:misplaced define form),@(y7:c-error)[12},@(y13:apply-to-list)["
  "22}'(y16:record-case-miss)]2",

  "P", "find-sets*",
  "%2.0u?{n]2}${.3,.3d,@(y10:find-sets*)[02},${.4,.4a,@(y9:find-sets)[02}"
  ",@(y9:set-union)[22",

  "P", "find-sets",
  "%2'(y5:quote),.1aq?{.0d,&0{%1n]1},@(y13:apply-to-list)[22}'(y3:ref),.1"
  "aq?{.0d,&0{%1n]1},@(y13:apply-to-list)[22}'(y4:set!),.1aq?{.0d,.2,&1{%"
  "2${:0,.4,@(y9:find-sets)[02},${:0,.4,@(y11:set-member?)[02}?{.1,l1}{n}"
  ",@(y9:set-union)[22},@(y13:apply-to-list)[22}'(y4:set&),.1aq?{.0d,.2,&"
  "1{%1${:0,.3,@(y11:set-member?)[02}?{.0,l1]1}n]1},@(y13:apply-to-list)["
  "22}'(y6:lambda),.1aq?{.0d,.2,&1{%2${${.4,@(y15:flatten-idslist)[01},:0"
  ",@(y9:set-minus)[02},.2,@(y9:find-sets)[22},@(y13:apply-to-list)[22}'("
  "y7:lambda*),.1aq?{.0d,.2,&1{%!0:0,${.3,@(y4:cadr),@(y5:%25map1)[02},@("
  "y10:find-sets*)[12},@(y13:apply-to-list)[22}'(y5:letcc),.1aq?{.0d,.2,&"
  "1{%2${.2,l1,:0,@(y9:set-minus)[02},.2,@(y9:find-sets)[22},@(y13:apply-"
  "to-list)[22}'(y6:withcc),.1aq?{.0d,.2,&1{%2${:0,.4,@(y9:find-sets)[02}"
  ",${:0,.4,@(y9:find-sets)[02},@(y9:set-union)[22},@(y13:apply-to-list)["
  "22}'(y5:begin),.1aq?{.0d,.2,&1{%!0:0,.1,@(y10:find-sets*)[12},@(y13:ap"
  "ply-to-list)[22}'(y2:if),.1aq?{.0d,.2,&1{%3${${:0,.7,@(y9:find-sets)[0"
  "2},${:0,.7,@(y9:find-sets)[02},@(y9:set-union)[02},${:0,.4,@(y9:find-s"
  "ets)[02},@(y9:set-union)[32},@(y13:apply-to-list)[22}'(y10:integrable)"
  ",.1aq?{.0d,.2,&1{%!1:0,.1,@(y10:find-sets*)[22},@(y13:apply-to-list)[2"
  "2}'(y4:call),.1aq?{.0d,.2,&1{%!1${:0,.3,@(y10:find-sets*)[02},${:0,.5,"
  "@(y9:find-sets)[02},@(y9:set-union)[22},@(y13:apply-to-list)[22}'(y6:d"
  "efine),.1aq?{.0d,.1,&1{%!0:0,'(s21:misplaced define form),@(y7:c-error"
  ")[12},@(y13:apply-to-list)[22}'(y16:record-case-miss)]2",

  "P", "codegen",
  "%7'(y5:quote),.1aq?{.0d,.6,.8,&2{%1.0,t,.1v?{:0,'(ct)W0}{f,.1v?{:0,'(c"
  "f)W0}{n,.1v?{:0,'(cn)W0}{:0,'(c')W0${:0,.4,@(y20:write-serialized-arg)"
  "[02}}}}_1:1?{:0,'(c])W0:0,:1,@(y20:write-serialized-arg)[12}]1},@(y13:"
  "apply-to-list)[72}'(y3:ref),.1aq?{.0d,.6,.8,.5,.7,.6,&5{%1${:0,.3,@(y4"
  ":posq)[02},.0?{.0,:3,'(c.)W0${:3,.3,@(y20:write-serialized-arg)[02}${:"
  "1,.5,@(y11:set-member?)[02}?{:3,'(c^)W0}_1}{${:2,.4,@(y4:posq)[02},.0?"
  "{.0,:3,'(c:)W0${:3,.3,@(y20:write-serialized-arg)[02}${:1,.6,@(y11:set"
  "-member?)[02}?{:3,'(c^)W0}_1}{:3,'(c@)W0${:3,.5,@(y20:write-serialized"
  "-arg)[02}}_1}_1:4?{:3,'(c])W0:3,:4,@(y20:write-serialized-arg)[12}]1},"
  "@(y13:apply-to-list)[72}'(y4:set!),.1aq?{.0d,.6,.8,.5,.5,.8,.(i10),&6{"
  "%2${:4,f,:0,:1,:3,:2,.9,@(y7:codegen)[07}${:2,.3,@(y4:posq)[02},.0?{.0"
  ",:4,'(c.)W0:4,'(c!)W0${:4,.3,@(y20:write-serialized-arg)[02}_1}{${:3,."
  "4,@(y4:posq)[02},.0?{.0,:4,'(c:)W0:4,'(c!)W0${:4,.3,@(y20:write-serial"
  "ized-arg)[02}_1}{:4,'(c@)W0:4,'(c!)W0${:4,.5,@(y20:write-serialized-ar"
  "g)[02}}_1}_1:5?{:4,'(c])W0:4,:5,@(y20:write-serialized-arg)[22}]2},@(y"
  "13:apply-to-list)[72}'(y4:set&),.1aq?{.0d,.6,.8,.5,.5,&4{%1${:0,.3,@(y"
  "4:posq)[02},.0?{.0,:2,'(c.)W0${:2,.3,@(y20:write-serialized-arg)[02}_1"
  "}{${:1,.4,@(y4:posq)[02},.0?{.0,:2,'(c:)W0${:2,.3,@(y20:write-serializ"
  "ed-arg)[02}_1}{:2,'(c`)W0${:2,.5,@(y20:write-serialized-arg)[02}}_1}_1"
  ":3?{:2,'(c])W0:2,:3,@(y20:write-serialized-arg)[12}]1},@(y13:apply-to-"
  "list)[72}'(y5:begin),.1aq?{.0d,.6,.8,.4,.6,.8,.(i10),&6{%!0${.2,,#0.0,"
  ":3,:2,:1,:0,:4,:5,&7{%1.0p?{.0dp?{f}{:0},${:1,.3,:2,:3,:4,:5,.9a,@(y7:"
  "codegen)[07}.1d,:6^[21}]1}.!0.0^_1[01}:5?{.0u}{f}?{:4,'(c])W0:4,:5,@(y"
  "20:write-serialized-arg)[12}]1},@(y13:apply-to-list)[72}'(y2:if),.1aq?"
  "{.0d,.6,.6,.6,.6,.6,.(i12),&6{%3${:0,f,:4,:3,:2,:1,.8,@(y7:codegen)[07"
  "}:0,'(c?)W0:0,'(c{)W0${:0,:5,:4,:3,:2,:1,.9,@(y7:codegen)[07}:0,'(c})W"
  "0:5?{:0,:5,:4,:3,:2,:1,.8,@(y7:codegen)[37}'(l1:y5:begin;),.3e,.0?{.0]"
  "4}.3?{:0,'(c{)W0${:0,:5,:4,:3,:2,:1,.(i11),@(y7:codegen)[07}:0,'(c})W0"
  "]4}f]4},@(y13:apply-to-list)[72}'(y6:lambda),.1aq?{.0d,.6,.8,.7,.7,.7,"
  ".7,&6{%2${.2,@(y15:flatten-idslist)[01},${:3,${.5,.8,@(y9:find-free)[0"
  "2},@(y9:set-minus)[02},${.3,.6,@(y9:find-sets)[02},${:0,.4A8,,#0.0,:4,"
  ":1,:3,&4{%2.0u?{]2}${:2,f,:0,n,:1,.8,.8a,'(y3:ref),l2,@(y7:codegen)[07"
  "}:2,'(c,)W0.1,fc,.1d,:3^[22}.!0.0^_1[02}:4,'(c&)W0${:4,.4g,@(y20:write"
  "-serialized-arg)[02}:4,'(c{)W0.3L0?{:4,'(c%25)W0${:4,.6g,@(y20:write-s"
  "erialized-arg)[02}}{:4,'(c%25)W0:4,'(c!)W0${:4,${.8,@(y17:idslist-req-"
  "count)[01},@(y20:write-serialized-arg)[02}}${'0,.5,,#0.0,.6,:4,&3{%2.0"
  "u?{]2}${:1,.3a,@(y11:set-member?)[02}?{:0,'(c#)W0${:0,.4,@(y20:write-s"
  "erialized-arg)[02}}'1,.2I+,.1d,:2^[22}.!0.0^_1[02}${:4,.5g,:3,${${.(i1"
  "0),:2,@(y13:set-intersect)[02},.8,@(y9:set-union)[02},.7,.9,.(i12),@(y"
  "7:codegen)[07}:4,'(c})W0_1_1_1:5?{:4,'(c])W0:4,:5,@(y20:write-serializ"
  "ed-arg)[22}]2},@(y13:apply-to-list)[72}'(y7:lambda*),.1aq?{.0d,.6,.8,."
  "5,.7,.9,.7,&6{%!0${:0,.3A8,,#0.0,:4,:3,:2,:1,&5{%2.0u?{]2}${:3,f,:0,:1"
  ",:2,.8,.8ada,@(y7:codegen)[07}:3,'(c%25)W0:3,'(cx)W0:3,'(c,)W0.1,fc,.1"
  "d,:4^[22}.!0.0^_1[02}:4,'(c&)W0${:4,.3g,@(y20:write-serialized-arg)[02"
  "}:4,'(c{)W0${'0,.3,,#0.0,:4,&2{%2.0u?{]2}.0aa,.0a,.1da,:0,'(c|)W0.0?{:"
  "0,'(c!)W0}${:0,.4,@(y20:write-serialized-arg)[02}${:0,.7,@(y20:write-s"
  "erialized-arg)[02}_1_1_1'1,.2I+,.1d,:1^[22}.!0.0^_1[02}:4,'(c%25)W0:4,"
  "'(c%25)W0:4,'(c})W0:5?{:4,'(c])W0:4,:5,@(y20:write-serialized-arg)[12}"
  "]1},@(y13:apply-to-list)[72}'(y5:letcc),.1aq?{.0d,.4,.7,.7,.6,.6,.(i12"
  "),&6{%2.0,l1,${.2,.5,@(y9:find-sets)[02},${.2,${.6,:5,@(y9:set-minus)["
  "02},@(y9:set-union)[02},:4?{:0,'(ck)W0${:0,:4,@(y20:write-serialized-a"
  "rg)[02}:0,'(c,)W0${.3,.6,@(y11:set-member?)[02}?{:0,'(c#)W0:0,'(c0)W0}"
  ":0,'1,:4I+,:3,.3,:2,:1,.9c,.(i10),@(y7:codegen)[57}:0,'(c$)W0:0,'(c{)W"
  "0:0,'(ck)W0:0,'(c0)W0:0,'(c,)W0${.3,.6,@(y11:set-member?)[02}?{:0,'(c#"
  ")W0:0,'(c0)W0}${:0,f,:3,.5,:2,:1,fc,fc,.(i11)c,.(i12),@(y7:codegen)[07"
  "}:0,'(c_)W0${:0,'3,@(y20:write-serialized-arg)[02}:0,'(c})W0]5},@(y13:"
  "apply-to-list)[72}'(y6:withcc),.1aq?{.0d,.7,.3,.5,.7,.9,&5{%2'(l3:y5:q"
  "uote;y3:ref;y6:lambda;),.2aA0?{${:4,f,:0,:1,:2,:3,.9,@(y7:codegen)[07}"
  ":4,'(c,)W0${:4,f,:0,:1,:2,:3,fc,.8,@(y7:codegen)[07}:4,'(cw)W0:4,'(c!)"
  "W0]2}${:4,f,:0,:1,:2,:3,.9,n,'(y6:lambda),l3,@(y7:codegen)[07}:4,'(c,)"
  "W0${:4,f,:0,:1,:2,:3,fc,.8,@(y7:codegen)[07}:4,'(cw)W0]2},@(y13:apply-"
  "to-list)[72}'(y10:integrable),.1aq?{.0d,.6,.8,.5,.7,.9,.7,&6{%!1'0,.2U"
  "8,.2U6,.0,'(l4:c0;c1;c2;c3;),.1A1?{${:0,.6A8,,#0.0,:4,:3,:2,:1,&5{%2.0"
  "u?{]2}${:3,f,:0,:1,:2,.8,.8a,@(y7:codegen)[07}.0du~?{:3,'(c,)W0}.1,fc,"
  ".1d,:4^[22}.!0.0^_1[02}${:4,.5,@(y12:write-string)[02}}{'(cp),.1v?{.3u"
  "?{'1,.5U8,${:4,.3,@(y12:write-string)[02}_1}{'1,.4gI-,${:0,.7A8,,#0.0,"
  ":4,:3,:2,:1,&5{%2.0u?{]2}${:3,f,:0,:1,:2,.8,.8a,@(y7:codegen)[07}.0du~"
  "?{:3,'(c,)W0}.1,fc,.1d,:4^[22}.!0.0^_1[02}${'0,,#0.0,.8,:4,.7,&4{%1:0,"
  ".1I<!?{]1}${:1,:2,@(y12:write-string)[02}'1,.1I+,:3^[11}.!0.0^_1[01}_1"
  "}}{'(cm),.1v?{.3du?{'1,.5U8,${:4,f,:1,:2,:3,:0,.(i12)a,@(y7:codegen)[0"
  "7}${:4,.3,@(y12:write-string)[02}_1}{'1,.4gI-,${:0,.7A8,,#0.0,:4,:3,:2"
  ",:1,&5{%2.0u?{]2}${:3,f,:0,:1,:2,.8,.8a,@(y7:codegen)[07}.0du~?{:3,'(c"
  ",)W0}.1,fc,.1d,:4^[22}.!0.0^_1[02}${'0,,#0.0,.8,:4,.7,&4{%1:0,.1I<!?{]"
  "1}${:1,:2,@(y12:write-string)[02}'1,.1I+,:3^[11}.!0.0^_1[01}_1}}{'(cc)"
  ",.1v?{.3A8,'1,.5gI-,${:4,f,:1,:2,:3,:0,.9a,@(y7:codegen)[07}:4,'(c,)W0"
  "${:0,fc,.4d,,#0.0,:4,:3,:2,:1,&5{%2.0u?{]2}${:3,f,:0,:1,:2,.8,.8a,@(y7"
  ":codegen)[07}.0du~?{:3,'(c,)W0:3,'(c,)W0}.1,fc,fc,.1d,:4^[22}.!0.0^_1["
  "02}${'0,,#0.0,.9,:4,.7,&4{%1:0,.1I<!?{]1}.0I=0~?{:1,'(c;)W0}${:1,:2,@("
  "y12:write-string)[02}'1,.1I+,:3^[11}.!0.0^_1[01}_2}{'(cx),.1v?{'1,.4gI"
  "-,${:0,.7A8,,#0.0,:4,:3,:2,:1,&5{%2.0u?{]2}${:3,f,:0,:1,:2,.8,.8a,@(y7"
  ":codegen)[07}.0du~?{:3,'(c,)W0}.1,fc,.1d,:4^[22}.!0.0^_1[02}${'0,,#0.0"
  ",.8,:4,.7,&4{%1:0,.1I<!?{]1}${:1,:2,@(y12:write-string)[02}'1,.1I+,:3^"
  "[11}.!0.0^_1[01}_1}{'(cu),.1v?{.3u?{${:4,'1,.8U8,@(y12:write-string)[0"
  "2}}{${:4,f,:1,:2,:3,:0,.(i11)a,@(y7:codegen)[07}}${:4,.5,@(y12:write-s"
  "tring)[02}}{'(cb),.1v?{.3du?{${:4,'1,.8U8,@(y12:write-string)[02}}{${:"
  "4,f,:1,:2,:3,:0,.(i11)da,@(y7:codegen)[07}}:4,'(c,)W0${:4,f,:1,:2,:3,:"
  "0,fc,.(i11)a,@(y7:codegen)[07}${:4,.5,@(y12:write-string)[02}}{'(ct),."
  "1v?{.3ddu?{${:4,'1,.8U8,@(y12:write-string)[02}}{${:4,f,:1,:2,:3,:0,.("
  "i11)dda,@(y7:codegen)[07}}:4,'(c,)W0${:4,f,:1,:2,:3,:0,fc,.(i11)da,@(y"
  "7:codegen)[07}:4,'(c,)W0${:4,f,:1,:2,:3,:0,fc,fc,.(i11)a,@(y7:codegen)"
  "[07}${:4,.5,@(y12:write-string)[02}}{'(c#),.1v?{${:0,.6A8,,#0.0,:4,:3,"
  ":2,:1,&5{%2.0u?{]2}${:3,f,:0,:1,:2,.8,.8a,@(y7:codegen)[07}:3,'(c,)W0."
  "1,fc,.1d,:4^[22}.!0.0^_1[02}${:4,.5,@(y12:write-string)[02}${:4,.6g,@("
  "y20:write-serialized-arg)[02}}{${.3,'(s27:unsupported integrable type)"
  ",@(y7:c-error)[02}}}}}}}}}}_1_2:5?{:4,'(c])W0:4,:5,@(y20:write-seriali"
  "zed-arg)[22}]2},@(y13:apply-to-list)[72}'(y4:call),.1aq?{.0d,.7,.4,.6,"
  ".8,.6,.(i11),&6{%!1'(y6:lambda),.2aq?{.1daL0?{.1dag,.1gI=}{f}}{f}?{${:"
  "1,.3A8,,#0.0,:5,:4,:3,:2,&5{%2.0u?{]2}${:3,f,:0,:1,:2,.8,.8a,@(y7:code"
  "gen)[07}:3,'(c,)W0.1,fc,.1d,:4^[22}.!0.0^_1[02}.1da,.2dda,${.3,.3,@(y9"
  ":find-sets)[02},${.2,${.7,:3,@(y9:set-minus)[02},@(y9:set-union)[02},:"
  "1,.4L6,${'0,.7,,#0.0,.8,:5,&3{%2.0u?{]2}${:1,.3a,@(y11:set-member?)[02"
  "}?{:0,'(c#)W0${:0,.4,@(y20:write-serialized-arg)[02}}'1,.2I+,.1d,:2^[2"
  "2}.!0.0^_1[02}:0?{:5,.6g,:0I+,:2,.4,:4,.5,.9,@(y7:codegen)[77}${:5,f,:"
  "2,.6,:4,.7,.(i11),@(y7:codegen)[07}:5,'(c_)W0:5,.6g,@(y20:write-serial"
  "ized-arg)[72}:0?{${:1,.3A8,,#0.0,:5,:4,:3,:2,.(i11),&6{%2.0u?{:4,f,:1,"
  ":2,:3,.6,:0,@(y7:codegen)[27}${:4,f,:1,:2,:3,.8,.8a,@(y7:codegen)[07}:"
  "4,'(c,)W0.1,fc,.1d,:5^[22}.!0.0^_1[02}:5,'(c[)W0${:5,:0,@(y20:write-se"
  "rialized-arg)[02}:5,.1g,@(y20:write-serialized-arg)[22}:5,'(c$)W0:5,'("
  "c{)W0${:1,fc,fc,.3A8,,#0.0,:5,:4,:3,:2,.(i11),&6{%2.0u?{:4,f,:1,:2,:3,"
  ".6,:0,@(y7:codegen)[27}${:4,f,:1,:2,:3,.8,.8a,@(y7:codegen)[07}:4,'(c,"
  ")W0.1,fc,.1d,:5^[22}.!0.0^_1[02}:5,'(c[)W0${:5,'0,@(y20:write-serializ"
  "ed-arg)[02}${:5,.3g,@(y20:write-serialized-arg)[02}:5,'(c})W0]2},@(y13"
  ":apply-to-list)[72}'(y6:define),.1aq?{.0d,.1,&1{%!0:0,'(s21:misplaced "
  "define form),@(y7:c-error)[12},@(y13:apply-to-list)[72}'(y16:record-ca"
  "se-miss)]7",

  "P", "compile-to-string",
  "%1P51,${.2,f,${n,.8,@(y9:find-free)[02},n,n,n,.9,@(y7:codegen)[07}.0P9"
  "0]2",

  "P", "env-lookup",
  "%3.0K0?{.0,@(y7:old-den)[31}.1,,#0.4,.3,.2,&3{%1.0p?{:1,.1aaq?{.0ad]1}"
  ".0d,:0^[11}.0V0?{.0V3,.0,:1H2,.0,.3V4,.0,:1A3,.0?{.0d]5}:1U5,.0?{.0}{:"
  "1,'(y3:ref),l2}_1b,.2,.1,:1cc,.4,.7V5.0]6}.0S0?{:2?{:1X4,.1S6X5,t,@(y1"
  "8:*root-environment*),.2,@(y10:env-lookup)[23}f]1}f]1}.!0.0^_1[31",

  "C", 0,
  "'(i101),n,.1V2,,#0.2,.2,&2{%2:1,.1H2,.0,:0V4,.0,.3A3,.0?{.4,.1sd]5}.1,"
  ".5,.5cc,.3,:0V5]5}.!0${U1,,#0.0,.5,.7,&3{%1.0u?{:0]1}.0d,.1a,.0d,.1a,."
  "1Y0,.0?{.0}{.2N0}_1?{${.3b,.3,:1^[02}.3,:2^[51}.1p?{'(y12:syntax-rules"
  "),.2aq}{f}?{,,#0#1&0{%1t,@(y18:*root-environment*),.2,@(y10:env-lookup"
  ")[13}.!0${.5da,@(y3:id?)[01}?{${.5ddd,.6dda,.7da,.5^,@(y13:syntax-rule"
  "s*)[04}}{${.5dd,.6da,f,.5^,@(y13:syntax-rules*)[04}}.!1${.3^b,.5,:1^[0"
  "2}.5,:2^[71}f]5}.!0.0^_1[01}_1_1_1@!(y18:*root-environment*)",

  "P", "root-environment",
  "%1t,@(y18:*root-environment*),.2,@(y10:env-lookup)[13",

  "P", "transform!",
  "%1${@(y16:root-environment),.3,t,@(y5:xform)[03},${.2,'(l3:y13:define-"
  "syntax;y1:*;y1:*;),@(y13:syntax-match?)[02}?{${.2da,@(y3:id?)[01}}{f}?"
  "{${t,@(y18:*root-environment*),.4da,@(y10:env-lookup)[03},.0?{.1dda,.1"
  "sz}_1}.0]2",

  "P", "error*",
  "%2.1,.1c,@(y5:error),@(y13:apply-to-list)[22",

  "P", "visit",
  "%1,#0${.3,@(y15:open-input-file)[01}.!0${${.4^,@(y4:read)[01},,#0.4,.1"
  ",&2{%1.0R8~?{${.2,@(y10:transform!)[01},Po,.1W5PoW6_1${:1^,@(y4:read)["
  "01},:0^[11}]1}.!0.0^_1[01}.0^P60]2",

  "P", "visit/c",
  "%1,#0${.3,@(y15:open-input-file)[01}.!0${${.4^,@(y4:read)[01},,#0.4,.1"
  ",&2{%1.0R8~?{${.2,@(y10:transform!)[01},Po,.1W5PoW6${.2,,#0.0,&1{%1'(y"
  "5:begin),.1aq?{.0d,:0,&1{%!0.0,:0^,@(y10:%25for-each1)[12},@(y13:apply"
  "-to-list)[12}'(y6:define),.1aq?{.0d,:0,&1{%2.1,.1,'(y4:set!),l3,:0^[21"
  "},@(y13:apply-to-list)[12}'(y13:define-syntax),.1aq?{.0d,&0{%2]2},@(y1"
  "3:apply-to-list)[12}Po,${.3,@(y17:compile-to-string)[01}W5PoW6]1}.!0.0"
  "^_1[01}_1${:1^,@(y4:read)[01},:0^[11}]1}.!0.0^_1[01}.0^P60]2",

  0, 0, 0
};
