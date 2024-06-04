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

  "P", "sexp-match?",
  "%2'(y1:*),.1q,.0?{.0]3}'(y8:<symbol>),.2q?{.2Y0}{f},.0?{.0]4}'(y8:<str"
  "ing>),.3q?{.3S0}{f},.0?{.0]5}.3,.5q,.0?{.0]6}.4p?{'(y3:...),.5aq?{.4dp"
  "?{.4ddu}{f}}{f}?{.4da,.6q]6}.4dp?{'(y3:...),.5daq?{.4ddu}{f}}{f}?{.4a,"
  "'(y1:*),.1q?{.6L0]7}.6,,#0.0,.3,&2{%1.0u,.0?{.0]2}.1p?{${.3a,:0,@(y11:"
  "sexp-match?)[02}?{.1d,:1^[21}f]2}f]2}.!0.0^_1[71}.5p?{${.7a,.7a,@(y11:"
  "sexp-match?)[02}?{.5d,.5d,@(y11:sexp-match?)[62}f]6}f]6}f]6",

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

  "P", "append*",
  "%1.0u?{n]1}.0du?{.0a]1}${.2d,@(y7:append*)[01},.1aL6]1",

  "P", "string-append*",
  "%1.0,@(y14:%25string-append),@(y13:apply-to-list)[12",

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

  "P", "extend-xenv-local",
  "%3.1b,.3,.1,.3,&3{%2.0,:0q?{.1,'(l2:y3:ref;y4:set!;),.1A1?{:1]3}f]3}.1"
  ",.1,:2[22}]4",

  "P", "add-local-var",
  "%3.2,.2,'(y3:ref),l2,.2,@(y17:extend-xenv-local)[33",

  "P", "xenv-lookup",
  "%3${.4,.4,.4[02},.0?{.0]4}.3,.3,l2,'(s38:transformer: invalid identifi"
  "er access),@(y6:error*)[42",

  "P", "xenv-ref",
  "%2'(y3:ref),.2,.2,@(y11:xenv-lookup)[23",

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
  "%2${.2,.4,@(y8:xenv-ref)[02},'(y3:...),.1zq?{'(s19:improper use of ..."
  "),@(y7:x-error)[31}.0z]3",

  "P", "xform-quote",
  "%2${.2,@(y6:list1?)[01}?{${.2a,@(y17:xform-sexp->datum)[01},'(y5:quote"
  "),l2]2}.0,'(y5:quote)c,'(s19:improper quote form),@(y7:x-error)[22",

  "P", "xform-set!",
  "%2${.2,@(y6:list2?)[01}?{${.2a,@(y3:id?)[01}}{f}?{${.3,.3da,f,@(y5:xfo"
  "rm)[03},${'(y4:set!),.4a,.6,@(y11:xenv-lookup)[03},${.2,@(y17:location"
  "-special?)[01}?{.1,.1sz'(l1:y5:begin;)]4}.0z,'(y3:ref),.1aq?{.2,.1da,'"
  "(y4:set!),l3]5}'(s27:set! to non-identifier form),@(y7:x-error)[51}.0,"
  "'(y4:set!)c,'(s18:improper set! form),@(y7:x-error)[22",

  "P", "xform-set&",
  "%2${.2,@(y6:list1?)[01}?{${'(y4:set!),.3a,.5,@(y11:xenv-lookup)[03},${"
  ".2,@(y17:location-special?)[01}?{'(s22:set& of a non-variable),@(y7:x-"
  "error)[31}.0z,'(y3:ref),.1aq?{.0da,'(y4:set&),l2]4}'(s22:set& of a non"
  "-variable),@(y7:x-error)[41}.0,'(y4:set&)c,'(s18:improper set& form),@"
  "(y7:x-error)[22",

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
  "6,.4,.6,@(y13:add-local-var)[03},.4d,:0^[53}.0u?{${.3,:1d,@(y10:xform-"
  "body)[02},.3A8,'(y6:lambda),l3]3}.0,${${.4,@(y7:id->sym)[01},@(y6:gens"
  "ym)[01},${.5,.3,.5,@(y13:add-local-var)[03},${.2,:1d,@(y10:xform-body)"
  "[02},.2,.7A8L6,'(y6:lambda),l3]6}.!0.0^_1[23}.0,'(y6:lambda)c,'(s20:im"
  "proper lambda body),@(y7:x-error)[22",

  "P", "xform-lambda*",
  "%2.0L0?{${.2,.4,&1{%1${.2,@(y6:list2?)[01}?{${.2a,@(y6:list2?)[01}?{.0"
  "aaI0?{.0adaY1}{f}}{f},.0?{.0}{${.3a,@(y8:idslist?)[01}}_1}{f}?{${:0,.3"
  "da,f,@(y5:xform)[03},${.3a,@(y15:normalize-arity)[01},l2]1}.0,'(s23:im"
  "proper lambda* clause),@(y7:x-error)[12},@(y5:%25map1)[02},'(y7:lambda"
  "*)c]2}.0,'(y7:lambda*)c,'(s21:improper lambda* form),@(y7:x-error)[22",

  "P", "xform-letcc",
  "%2${.2,@(y7:list2+?)[01}?{${.2a,@(y3:id?)[01}}{f}?{.0a,${${.4,@(y7:id-"
  ">sym)[01},@(y6:gensym)[01},${${.7,.5,.7,@(y13:add-local-var)[03},.5d,@"
  "(y10:xform-body)[02},.1,'(y5:letcc),l3]4}.0,'(y5:letcc)c,'(s19:imprope"
  "r letcc form),@(y7:x-error)[22",

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
  "6,@(y13:add-local-var)[03},.9,.(i14),.3c,.(i14),.5c,.(i14),.7c,.4,:0^["
  "(i15)5}${.4,@(y7:list2+?)[01}?{.2ap?{${.4aa,@(y3:id?)[01}?{${.4ad,@(y8"
  ":idslist?)[01}}{f}}{f}}{f}?{.2aa,${'(y6:lambda)b,'(y6:lambda),@(y6:new"
  "-id)[02},.4d,.5adc,.1c,${${.6,@(y7:id->sym)[01},@(y6:gensym)[01},${.(i"
  "12),.3,.7,@(y13:add-local-var)[03},.(i10),.(i15),.3c,.(i15),.5c,.(i15)"
  ",.8c,.4,:0^[(i16)5}.4,'(s20:improper define form),@(y7:x-error)[(i11)2"
  "}'(y13:define-syntax),.1v?{${.4,@(y6:list2?)[01}?{${.4a,@(y3:id?)[01}}"
  "{f}?{.2a,.3da,${.(i10),'(l1:y9:undefined;),.5,@(y17:extend-xenv-local)"
  "[03},.8,.(i13),tc,.(i13),.4c,.(i13),.6c,.4,:0^[(i14)5}.4,'(s27:imprope"
  "r define-syntax form),@(y7:x-error)[(i11)2}.1K0?{.5,${.9,.8,.6[02}c,.("
  "i10),.(i10),.(i10),.(i10),:0^[(i11)5}.6,.(i11),.(i11)A8,.(i11)A8,.(i11"
  ")A8,@(y12:xform-labels)[(i11)5}.0,.5,.5A8,.5A8,.5A8,@(y12:xform-labels"
  ")[55}.!0.0^_1[25",

  "P", "xform-labels",
  "%5n,n,.4,.4,.4,,#0.0,.(i11),.(i11),&3{%5.0u?{${:0,:1,&1{%1:0,.1,f,@(y5"
  ":xform)[13},@(y5:%25map1)[02},.4A8L6,${.2,@(y6:list1?)[01}?{.0a}{.0,'("
  "y5:begin)c},.6u?{.0]7}${.8,&0{%1'(l1:y5:begin;)]1},@(y5:%25map1)[02},."
  "1,.8A8,'(y6:lambda),l3,'(y4:call),@(y5:pair*)[73}.0a~?{.4,.4,${:1,.6a,"
  "f,@(y5:xform)[03}c,.4d,.4d,.4d,:2^[55}.2aY0?{.4,.3ac,.4,${:1,.6a,.6a,l"
  "2,@(y10:xform-set!)[02}c,.4d,.4d,.4d,:2^[55}${:1,.4a,t,@(y5:xform)[03}"
  ",${'(y4:set!),.4a,:1,@(y11:xenv-lookup)[03}sz.4,.4,.4d,.4d,.4d,:2^[55}"
  ".!0.0^_1[55",

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
  "{.4,${:2,.7a,t,@(y5:xform)[03},.4a,@(y17:extend-xenv-local)[03},.2d,.2"
  "d,:1^[33}.!0.0^_1[23}.0,'(s33:invalid syntax-lambda application),@(y7:"
  "x-error)[22}]5}.0,'(y13:syntax-lambda)c,'(s27:improper syntax-lambda b"
  "ody),@(y7:x-error)[22",

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
  "}.!0.0^_1[63}.!6.8,.2,.7,.5,&4{%3,,,#0#1#2${${.9,&1{%1:0,.1A3~]1},t,.("
  "i10),:1^[03},:3,&1{%1${${.4,:0,@(y8:xenv-ref)[02},${.5,@(y7:id->sym)[0"
  "1},@(y6:new-id)[02},.1c]1},@(y5:%25map1)[02}.!0${:2^,f,.7,:1^[03}.!1.1"
  ",:1,&2{%1:1,&1{%1:0^,.1A0]1},t,.2,:0^[13}.!2.5,.5,,#0.8,.4,:0,.8,.4,&5"
  "{%2.0,,#0.0,.4,:0,:1,:2,:3,:4,&7{%1${.2,@(y3:id?)[01}?{:5,.1A3,.0?{.0}"
  "{:0,.2A3,.0?{.0}{:1^,.3A3}_1}_1d]1}.0V0?{${.2X0,:6^[01}X1]1}.0p?{${.2d"
  ",:2^[01}}{f}?{${.2a,:3^[01},,,#0#1:5,&1{%1:0,.1A3d]1}.!0.2,.4,:4,&3{%!"
  "0${.2,:2,@(y4:cons),@(y5:%25map2)[03},:1a,:0^[12}.!1.2u?{${.5dd,:6^[01"
  "},${.6a,:6^[01}c]4}${.4,.3^,@(y5:%25map1)[02},${.6dd,:6^[01},${.3,.6^c"
  ",@(y4:%25map),@(y13:apply-to-list)[02}L6]5}.0p?{${.2d,:6^[01},${.3a,:6"
  "^[01}c]1}.0]1}.!0.0^_1[21}.!0.0^_1[62}.!7.(i11),.8,.8,&3{%2:2,,#0:0,.3"
  ",.5,:1,.4,&5{%1.0u?{${:3,'(s14:invalid syntax),@(y7:x-error)[02}}.0a,."
  "0a,.1da,${:2,:3,.5,:4^[03},.0?{.0,.0,.3,.5,:1^[63}.4d,:0^[51}.!0.0^_1["
  "21}](i12)",

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

  "P", "compile-to-thunk-code",
  "%1P51,${.2,'0,${n,.8,@(y9:find-free)[02},n,n,n,.9,@(y7:codegen)[07}.0P"
  "90]2",

  "P", "path-strip-directory",
  "%1n,.1X2A8,,#0.0,&1{%2.0u?{.1X3]2}'(l3:c%5c;c/;c:;),.1aA1?{.1X3]2}.1,."
  "1ac,.1d,:0^[22}.!0.0^_1[12",

  "P", "path-directory",
  "%1.0X2A8,,#0.0,&1{%1.0u?{'(s0:)]1}'(l3:c%5c;c/;c:;),.1aA1?{.0A8X3]1}.0"
  "d,:0^[11}.!0.0^_1[11",

  "P", "path-strip-extension",
  "%1.0X2A8,,#0.0,.3,&2{%1.0u?{:0]1}'(c.),.1av?{.0dA8X3]1}'(l3:c%5c;c/;c:"
  ";),.1aA1?{:0]1}.0d,:1^[11}.!0.0^_1[11",

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

  "P", "current-file",
  "%0@(y20:*current-file-stack*)p?{@(y20:*current-file-stack*)a]0}f]0",

  "P", "with-current-file",
  "%2&0{%0@(y20:*current-file-stack*)d@!(y20:*current-file-stack*)]0},.2,"
  ".2,&1{%0@(y20:*current-file-stack*),:0c@!(y20:*current-file-stack*)]0}"
  ",@(y12:dynamic-wind)[23",

  "P", "file-resolve-relative-to-current",
  "%1${.2,@(y14:path-relative?)[01}?{${@(y12:current-file)[00},.0?{${.2,@"
  "(y14:path-directory)[01},.2,@(y34:file-resolve-relative-to-base-path)["
  "22}.1]2}.0]1",

  "P", "listname-segment->string",
  "%1.0Y0?{.0X4]1}.0N0?{'(i10),.1E8]1}.0S0?{.0]1}.0,'(s34:invalid symboli"
  "c file name element),@(y7:c-error)[12",

  "C", 0,
  "'(s1:_)@!(y17:modname-separator)",

  "P", "listname->modname",
  "%1,#0@(y17:modname-separator).!0n,.2,,#0.0,.4,&2{%2.0p?{.1u?{.1,${.3a,"
  "@(y24:listname-segment->string)[01}c}{.1,:0^c,${.3a,@(y24:listname-seg"
  "ment->string)[01}c},.1d,:1^[22}.1A8,@(y14:string-append*)[21}.!0.0^_1["
  "22",

  "P", "listname->path",
  "%3,#0${.4,@(y19:base-path-separator)[01},.0?{.0,S11}{${.5,'(s38:librar"
  "y path does not end in separator),@(y7:c-error)[02}}_1.!0n,.2,,#0.5,.7"
  ",.2,.6,&4{%2.0p?{.1u?{.1,${.3a,@(y24:listname-segment->string)[01}c}{."
  "1,:0^c,${.3a,@(y24:listname-segment->string)[01}c},.1d,:1^[22}:3,${.4,"
  ":2cA8,@(y14:string-append*)[01},@(y34:file-resolve-relative-to-base-pa"
  "th)[22}.!0.0^_1[42",

  "C", 0,
  "n@!(y19:*library-path-list*)",

  "P", "add-library-path!",
  "%1${.2,@(y19:base-path-separator)[01}?{.0,l1,@(y19:*library-path-list*"
  ")L6@!(y19:*library-path-list*)]1}.0,'(s46:library path should end in d"
  "irectory separator),@(y7:c-error)[12",

  "P", "find-library-path",
  "%1@(y19:*library-path-list*),,#0.2,.1,&2{%1.0u?{f]1}${'(s4:.sld),.3a,:"
  "1,@(y14:listname->path)[03},.0?{.0F0}{f}?{.0]2}.1d,:0^[21}.!0.0^_1[11",

  "P", "resolve-input-file/lib-name",
  "%1,#0.1S0?{${.3,@(y32:file-resolve-relative-to-current)[01}}{${.3,@(y1"
  "7:find-library-path)[01}}.!0.0^~?{.1S0?{${.3,'(s35:cannot resolve file"
  " name to a file:),@(y7:c-error)[02}}{${@(y19:*library-path-list*),'(y2"
  ":in),.5,'(s38:cannot resolve library name to a file:),@(y7:c-error)[04"
  "}}}.0^F0~?{${.2^,'(y2:=>),.5,'(s56:cannot resolve file or library name"
  " to an existing file:),@(y7:c-error)[04}}.0^]2",

  "P", "call-with-input-file/lib",
  "%3${.2,@(y27:resolve-input-file/lib-name)[01},.2,.1,.5,&3{%0:0,:1,:2,&"
  "3{%1:0?{${t,.3,@(y19:set-port-fold-case!)[02}}.0,:1,:2[12},:1,@(y20:ca"
  "ll-with-input-file)[02},.1,@(y17:with-current-file)[42",

  "P", "call-with-file/lib-sexps",
  "%3.2,&1{%2n,,#0.3,:0,.2,&3{%1${:2,@(y14:read-code-sexp)[01},.0R8?{.1A9"
  ",:1[21}.1,.1c,:0^[21}.!0.0^_1[21},.2,.2,@(y24:call-with-input-file/lib"
  ")[33",

  "P", "for-each-file/lib-sexp",
  "%3.0,&1{%2,#0.2,.1,:0,&3{%0${:2,@(y14:read-code-sexp)[01},.0R8~?{${.2,"
  ":0[01}:1^[10}]1}.!0.0^_1[20},.3,.3,@(y24:call-with-input-file/lib)[33",

  "P", "file/lib->modname",
  "%1.0p?{.0L0}{f}?{.0,@(y17:listname->modname)[11}.0S0?{${.2,@(y20:path-"
  "strip-directory)[01},@(y20:path-strip-extension)[11}.0,'(s29:illegal f"
  "ile or library name:),@(y7:c-error)[12",

  "P", "file/lib/stdin->modname",
  "%1.0S0?{'(s1:-),.1S=}{f}?{'(s5:stdin)]1}.0,@(y17:file/lib->modname)[11",

  "P", "fully-qualified-prefix",
  "%1'(s1:.),.1S6]1",

  "P", "fully-qualified-library-prefix",
  "%1${.2,@(y17:file/lib->modname)[01},@(y22:fully-qualified-prefix)[11",

  "P", "fully-qualified-library-prefixed-name",
  "%2.1X4,'(s1:.),${.4,@(y17:file/lib->modname)[01},@(y14:%25string-appen"
  "d)[23",

  "P", "env-lookup",
  "%3.0K0?{.2,'(l2:y3:ref;y4:set!;),.1A1?{.1,@(y7:old-den)[41}f]4}.1,,#0."
  "4,.3,.2,&3{%1.0p?{:1,.1aaq?{:2,'(y3:ref),.1v?{.1ad]2}f]2}.0d,:0^[11}.0"
  "V0?{.0V3,.0,:1H2,.0,.3V4,.0,:1A3,.0?{.0d]5}:1U5,.0?{.0}{:1,'(y3:ref),l"
  "2}_1b,.2,.1,:1cc,.4,.7V5.0]6}.0S0?{'(l4:y3:ref;y4:set!;y6:define;y13:d"
  "efine-syntax;),:2A0?{:1X4,.1S6X5,'(y3:ref),@(y18:*root-environment*),."
  "2,@(y10:env-lookup)[23}f]1}f]1}.!0.0^_1[31",

  "C", 0,
  "'(i101),n,.1V2,,#0.2,.2,&2{%2:1,.1H2,.0,:0V4,.0,.3A3,.0?{.4,.1sd]5}.1,"
  ".5,.5cc,.3,:0V5]5}.!0${U1,,#0.0,.5,.7,&3{%1.0u?{:0]1}.0d,.1a,.0d,.1a,."
  "1Y0,.0?{.0}{.2N0}_1?{${.3b,.3,:1^[02}.3,:2^[51}.1p?{'(y12:syntax-rules"
  "),.2aq}{f}?{,,#0#1&0{%2.1,@(y18:*root-environment*),.2,@(y10:env-looku"
  "p)[23}.!0${.5da,@(y3:id?)[01}?{${.5ddd,.6dda,.7da,.5^,@(y13:syntax-rul"
  "es*)[04}}{${.5dd,.6da,f,.5^,@(y13:syntax-rules*)[04}}.!1${.3^b,.5,:1^["
  "02}.5,:2^[71}f]5}.!0.0^_1[01}_1_1_1@!(y18:*root-environment*)",

  "P", "root-environment",
  "%2.1,@(y18:*root-environment*),.2,@(y10:env-lookup)[23",

  "C", 0,
  "n@!(y14:*std-lib->env*)",

  "C", 0,
  "${'(l343:l3:y1:*;y1:v;y1:b;;l3:y1:+;y1:v;y1:b;;l3:y1:-;y1:v;y1:b;;l4:y"
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
  "v;y1:b;;l3:y4:eqv?;y1:v;y1:b;;l2:y5:error;y1:b;;l2:y22:error-object-ir"
  "ritants;y1:b;;l2:y20:error-object-message;y1:b;;l2:y13:error-object?;y"
  "1:b;;l3:y5:even?;y1:v;y1:b;;l2:y5:exact;y1:b;;l2:y18:exact-integer-sqr"
  "t;y1:b;;l2:y14:exact-integer?;y1:b;;l3:y6:exact?;y1:v;y1:b;;l3:y4:expt"
  ";y1:v;y1:b;;l2:y8:features;y1:b;;l2:y11:file-error?;y1:b;;l3:y5:floor;"
  "y1:v;y1:b;;l2:y14:floor-quotient;y1:b;;l2:y15:floor-remainder;y1:b;;l2"
  ":y6:floor/;y1:b;;l2:y17:flush-output-port;y1:b;;l3:y8:for-each;y1:v;y1"
  ":b;;l3:y3:gcd;y1:v;y1:b;;l2:y21:get-output-bytevector;y1:b;;l2:y17:get"
  "-output-string;y1:b;;l2:y5:guard;y1:b;;l4:y2:if;y1:v;y1:u;y1:b;;l2:y7:"
  "include;y1:b;;l2:y10:include-ci;y1:b;;l2:y7:inexact;y1:b;;l3:y8:inexac"
  "t?;y1:v;y1:b;;l2:y16:input-port-open?;y1:b;;l3:y11:input-port?;y1:v;y1"
  ":b;;l3:y13:integer->char;y1:v;y1:b;;l3:y8:integer?;y1:v;y1:b;;l4:y6:la"
  "mbda;y1:v;y1:u;y1:b;;l3:y3:lcm;y1:v;y1:b;;l3:y6:length;y1:v;y1:b;;l4:y"
  "3:let;y1:v;y1:u;y1:b;;l4:y4:let*;y1:v;y1:u;y1:b;;l2:y11:let*-values;y1"
  ":b;;l4:y10:let-syntax;y1:v;y1:u;y1:b;;l2:y10:let-values;y1:b;;l4:y6:le"
  "trec;y1:v;y1:u;y1:b;;l2:y7:letrec*;y1:b;;l4:y13:letrec-syntax;y1:v;y1:"
  "u;y1:b;;l3:y4:list;y1:v;y1:b;;l3:y12:list->string;y1:v;y1:b;;l3:y12:li"
  "st->vector;y1:v;y1:b;;l2:y9:list-copy;y1:b;;l3:y8:list-ref;y1:v;y1:b;;"
  "l2:y9:list-set!;y1:b;;l3:y9:list-tail;y1:v;y1:b;;l3:y5:list?;y1:v;y1:b"
  ";;l2:y15:make-bytevector;y1:b;;l2:y9:make-list;y1:b;;l2:y14:make-param"
  "eter;y1:b;;l3:y11:make-string;y1:v;y1:b;;l3:y11:make-vector;y1:v;y1:b;"
  ";l3:y3:map;y1:v;y1:b;;l3:y3:max;y1:v;y1:b;;l3:y6:member;y1:v;y1:b;;l3:"
  "y4:memq;y1:v;y1:b;;l3:y4:memv;y1:v;y1:b;;l3:y3:min;y1:v;y1:b;;l3:y6:mo"
  "dulo;y1:v;y1:b;;l3:y9:negative?;y1:v;y1:b;;l3:y7:newline;y1:v;y1:b;;l3"
  ":y3:not;y1:v;y1:b;;l3:y5:null?;y1:v;y1:b;;l3:y14:number->string;y1:v;y"
  "1:b;;l3:y7:number?;y1:v;y1:b;;l3:y9:numerator;y1:v;y1:b;;l3:y4:odd?;y1"
  ":v;y1:b;;l2:y21:open-input-bytevector;y1:b;;l2:y17:open-input-string;y"
  "1:b;;l2:y22:open-output-bytevector;y1:b;;l2:y18:open-output-string;y1:"
  "b;;l4:y2:or;y1:v;y1:u;y1:b;;l2:y17:output-port-open?;y1:b;;l3:y12:outp"
  "ut-port?;y1:v;y1:b;;l3:y5:pair?;y1:v;y1:b;;l2:y12:parameterize;y1:b;;l"
  "3:y9:peek-char;y1:v;y1:b;;l2:y7:peek-u8;y1:b;;l2:y5:port?;y1:b;;l3:y9:"
  "positive?;y1:v;y1:b;;l3:y10:procedure?;y1:v;y1:b;;l4:y10:quasiquote;y1"
  ":v;y1:u;y1:b;;l4:y5:quote;y1:v;y1:u;y1:b;;l3:y8:quotient;y1:v;y1:b;;l2"
  ":y5:raise;y1:b;;l2:y17:raise-continuable;y1:b;;l3:y9:rational?;y1:v;y1"
  ":b;;l3:y11:rationalize;y1:v;y1:b;;l2:y15:read-bytevector;y1:b;;l2:y16:"
  "read-bytevector!;y1:b;;l3:y9:read-char;y1:v;y1:b;;l2:y11:read-error?;y"
  "1:b;;l2:y9:read-line;y1:b;;l2:y11:read-string;y1:b;;l2:y7:read-u8;y1:b"
  ";;l3:y5:real?;y1:v;y1:b;;l3:y9:remainder;y1:v;y1:b;;l3:y7:reverse;y1:v"
  ";y1:b;;l3:y5:round;y1:v;y1:b;;l3:y4:set!;y1:v;y1:b;;l3:y8:set-car!;y1:"
  "v;y1:b;;l3:y8:set-cdr!;y1:v;y1:b;;l2:y6:square;y1:b;;l3:y6:string;y1:v"
  ";y1:b;;l3:y12:string->list;y1:v;y1:b;;l3:y14:string->number;y1:v;y1:b;"
  ";l3:y14:string->symbol;y1:v;y1:b;;l2:y12:string->utf8;y1:b;;l2:y14:str"
  "ing->vector;y1:b;;l3:y13:string-append;y1:v;y1:b;;l3:y11:string-copy;y"
  "1:v;y1:b;;l2:y12:string-copy!;y1:b;;l3:y12:string-fill!;y1:v;y1:b;;l2:"
  "y15:string-for-each;y1:b;;l3:y13:string-length;y1:v;y1:b;;l2:y10:strin"
  "g-map;y1:b;;l3:y10:string-ref;y1:v;y1:b;;l3:y11:string-set!;y1:v;y1:b;"
  ";l3:y9:string<=?;y1:v;y1:b;;l3:y8:string<?;y1:v;y1:b;;l3:y8:string=?;y"
  "1:v;y1:b;;l3:y9:string>=?;y1:v;y1:b;;l3:y8:string>?;y1:v;y1:b;;l3:y7:s"
  "tring?;y1:v;y1:b;;l3:y9:substring;y1:v;y1:b;;l3:y14:symbol->string;y1:"
  "v;y1:b;;l2:y8:symbol=?;y1:b;;l3:y7:symbol?;y1:v;y1:b;;l2:y12:syntax-er"
  "ror;y1:b;;l4:y12:syntax-rules;y1:v;y1:u;y1:b;;l2:y13:textual-port?;y1:"
  "b;;l3:y8:truncate;y1:v;y1:b;;l2:y17:truncate-quotient;y1:b;;l2:y18:tru"
  "ncate-remainder;y1:b;;l2:y9:truncate/;y1:b;;l2:y9:u8-ready?;y1:b;;l2:y"
  "6:unless;y1:b;;l4:y7:unquote;y1:v;y1:u;y1:b;;l4:y16:unquote-splicing;y"
  "1:v;y1:u;y1:b;;l2:y12:utf8->string;y1:b;;l3:y6:values;y1:v;y1:b;;l3:y6"
  ":vector;y1:v;y1:b;;l3:y12:vector->list;y1:v;y1:b;;l2:y14:vector->strin"
  "g;y1:b;;l2:y13:vector-append;y1:b;;l2:y11:vector-copy;y1:b;;l2:y12:vec"
  "tor-copy!;y1:b;;l3:y12:vector-fill!;y1:v;y1:b;;l2:y15:vector-for-each;"
  "y1:b;;l3:y13:vector-length;y1:v;y1:b;;l2:y10:vector-map;y1:b;;l3:y10:v"
  "ector-ref;y1:v;y1:b;;l3:y11:vector-set!;y1:v;y1:b;;l3:y7:vector?;y1:v;"
  "y1:b;;l2:y4:when;y1:b;;l2:y22:with-exception-handler;y1:b;;l2:y16:writ"
  "e-bytevector;y1:b;;l3:y10:write-char;y1:v;y1:b;;l2:y12:write-string;y1"
  ":b;;l2:y8:write-u8;y1:b;;l3:y5:zero?;y1:v;y1:b;;l2:y11:case-lambda;y1:"
  "l;;l3:y16:char-alphabetic?;y1:v;y1:h;;l3:y10:char-ci<=?;y1:v;y1:h;;l3:"
  "y9:char-ci<?;y1:v;y1:h;;l3:y9:char-ci=?;y1:v;y1:h;;l3:y10:char-ci>=?;y"
  "1:v;y1:h;;l3:y9:char-ci>?;y1:v;y1:h;;l3:y13:char-downcase;y1:v;y1:h;;l"
  "2:y13:char-foldcase;y1:h;;l3:y16:char-lower-case?;y1:v;y1:h;;l3:y13:ch"
  "ar-numeric?;y1:v;y1:h;;l3:y11:char-upcase;y1:v;y1:h;;l3:y16:char-upper"
  "-case?;y1:v;y1:h;;l3:y16:char-whitespace?;y1:v;y1:h;;l2:y11:digit-valu"
  "e;y1:h;;l3:y12:string-ci<=?;y1:v;y1:h;;l3:y11:string-ci<?;y1:v;y1:h;;l"
  "3:y11:string-ci=?;y1:v;y1:h;;l3:y12:string-ci>=?;y1:v;y1:h;;l3:y11:str"
  "ing-ci>?;y1:v;y1:h;;l2:y15:string-downcase;y1:h;;l2:y15:string-foldcas"
  "e;y1:h;;l2:y13:string-upcase;y1:h;;l3:y5:angle;y1:v;y1:o;;l3:y9:imag-p"
  "art;y1:v;y1:o;;l3:y9:magnitude;y1:v;y1:o;;l3:y10:make-polar;y1:v;y1:o;"
  ";l3:y16:make-rectangular;y1:v;y1:o;;l3:y9:real-part;y1:v;y1:o;;l3:y5:c"
  "aaar;y1:v;y1:x;;l3:y5:caadr;y1:v;y1:x;;l3:y5:cadar;y1:v;y1:x;;l3:y5:ca"
  "ddr;y1:v;y1:x;;l3:y5:cdaar;y1:v;y1:x;;l3:y5:cdadr;y1:v;y1:x;;l3:y5:cdd"
  "ar;y1:v;y1:x;;l3:y5:cdddr;y1:v;y1:x;;l3:y6:caaaar;y1:v;y1:x;;l3:y6:caa"
  "adr;y1:v;y1:x;;l3:y6:caadar;y1:v;y1:x;;l3:y6:caaddr;y1:v;y1:x;;l3:y6:c"
  "adaar;y1:v;y1:x;;l3:y6:cadadr;y1:v;y1:x;;l3:y6:caddar;y1:v;y1:x;;l3:y6"
  ":cadddr;y1:v;y1:x;;l3:y6:cdaaar;y1:v;y1:x;;l3:y6:cdaadr;y1:v;y1:x;;l3:"
  "y6:cdadar;y1:v;y1:x;;l3:y6:cdaddr;y1:v;y1:x;;l3:y6:cddaar;y1:v;y1:x;;l"
  "3:y6:cddadr;y1:v;y1:x;;l3:y6:cdddar;y1:v;y1:x;;l3:y6:cddddr;y1:v;y1:x;"
  ";l2:y11:environment;y1:e;;l3:y4:eval;y1:v;y1:e;;l3:y20:call-with-input"
  "-file;y1:v;y1:f;;l3:y21:call-with-output-file;y1:v;y1:f;;l2:y11:delete"
  "-file;y1:f;;l2:y12:file-exists?;y1:f;;l2:y22:open-binary-input-file;y1"
  ":f;;l2:y23:open-binary-output-file;y1:f;;l3:y15:open-input-file;y1:v;y"
  "1:f;;l3:y16:open-output-file;y1:v;y1:f;;l3:y20:with-input-from-file;y1"
  ":v;y1:f;;l3:y19:with-output-to-file;y1:v;y1:f;;l4:y4:acos;y1:v;y1:z;y1"
  ":i;;l4:y4:asin;y1:v;y1:z;y1:i;;l4:y4:atan;y1:v;y1:z;y1:i;;l4:y3:cos;y1"
  ":v;y1:z;y1:i;;l4:y3:exp;y1:v;y1:z;y1:i;;l3:y7:finite?;y1:z;y1:i;;l2:y9"
  ":infinite?;y1:i;;l3:y3:log;y1:v;y1:i;;l2:y4:nan?;y1:i;;l3:y3:sin;y1:v;"
  "y1:i;;l3:y4:sqrt;y1:v;y1:i;;l3:y3:tan;y1:v;y1:i;;l4:y5:delay;y1:v;y1:u"
  ";y1:z;;l2:y11:delay-force;y1:z;;l3:y5:force;y1:v;y1:z;;l2:y12:make-pro"
  "mise;y1:z;;l2:y8:promise?;y1:z;;l3:y4:load;y1:v;y1:d;;l2:y12:command-l"
  "ine;y1:s;;l2:y14:emergency-exit;y1:s;;l2:y4:exit;y1:s;;l2:y24:get-envi"
  "ronment-variable;y1:s;;l2:y25:get-environment-variables;y1:s;;l3:y7:di"
  "splay;y1:w;y1:v;;l2:y14:exact->inexact;y1:v;;l2:y14:inexact->exact;y1:"
  "v;;l3:y23:interaction-environment;y1:p;y1:v;;l2:y16:null-environment;y"
  "1:v;;l3:y4:read;y1:r;y1:v;;l2:y25:scheme-report-environment;y1:v;;l3:y"
  "5:write;y1:w;y1:v;;l2:y13:current-jiffy;y1:t;;l2:y14:current-second;y1"
  ":t;;l2:y18:jiffies-per-second;y1:t;;l2:y12:write-shared;y1:w;;l2:y12:w"
  "rite-simple;y1:w;;l1:y4:box?;;l1:y3:box;;l1:y5:unbox;;l1:y8:set-box!;;"
  "),&0{%1,,,#0#1#2&0{%1.0,'(y1:w),.1v?{'(l2:y6:scheme;y5:write;)]2}'(y1:"
  "t),.1v?{'(l2:y6:scheme;y4:time;)]2}'(y1:p),.1v?{'(l2:y6:scheme;y4:repl"
  ";)]2}'(y1:r),.1v?{'(l2:y6:scheme;y4:read;)]2}'(y1:v),.1v?{'(l2:y6:sche"
  "me;y4:r5rs;)]2}'(y1:u),.1v?{'(l2:y6:scheme;y9:r5rs-null;)]2}'(y1:d),.1"
  "v?{'(l2:y6:scheme;y4:load;)]2}'(y1:z),.1v?{'(l2:y6:scheme;y4:lazy;)]2}"
  "'(y1:s),.1v?{'(l2:y6:scheme;y15:process-context;)]2}'(y1:i),.1v?{'(l2:"
  "y6:scheme;y7:inexact;)]2}'(y1:f),.1v?{'(l2:y6:scheme;y4:file;)]2}'(y1:"
  "e),.1v?{'(l2:y6:scheme;y4:eval;)]2}'(y1:o),.1v?{'(l2:y6:scheme;y7:comp"
  "lex;)]2}'(y1:h),.1v?{'(l2:y6:scheme;y4:char;)]2}'(y1:l),.1v?{'(l2:y6:s"
  "cheme;y11:case-lambda;)]2}'(y1:x),.1v?{'(l2:y6:scheme;y3:cxr;)]2}'(y1:"
  "b),.1v?{'(l2:y6:scheme;y4:base;)]2}]2}.!0&0{%3.0V3,.0,.3H2,.0,.3V4,.0,"
  ".5A3,.0?{.6,.1sd]7}.1,.7,.7cc,.3,.6V5]7}.!1&0{%1@(y14:*std-lib->env*),"
  ".1A5,.0?{.0d]2}'(l2:y5:skint;y4:repl;),.2q?{'(i101)}{'(i37)},n,.1V2,@("
  "y14:*std-lib->env*),.1,.5cc@!(y14:*std-lib->env*).0]4}.!2.3d,.4a,,#0.0"
  ",.5,.5,.8,&4{%2.1u?{${'(y3:ref),.3,@(y16:root-environment)[02},.1,${'("
  "l2:y5:skint;y4:repl;),:0^[01},:2^[23}${${'(y3:ref),.5,@(y16:root-envir"
  "onment)[02},.3,${${.9a,:1^[01},:0^[01},:2^[03}.1d,.1,:3^[22}.!0.0^_1[4"
  "2},@(y10:%25for-each1)[02}",

  "P", "std-lib->env",
  "%1@(y14:*std-lib->env*),.1A5,.0?{.0,.0d,.0V3,'(l2:y5:skint;y4:repl;),."
  "5q?{.0,.2,&2{%2:1,.1H2,.0,:0V4,.0,.3A3,.0?{.0d]5}.3,'(y3:ref),l2b,.2,."
  "1,.6cc,.4,:0V5.0]6}]5}.1,.1,&2{%2'(y3:ref),.2q?{:0,.1H2,.0,:1V4,.0,.3A"
  "3,.0?{.0d]5}f]5}f]2}]5}f]2",

  "P", "visit-top-form",
  "%2.0p?{${.3,.3a,t,@(y5:xform)[03},'(y5:begin),.1q?{.1d,,#0.4,.1,&2{%1."
  "0p?{${:1,.3a,@(y14:visit-top-form)[02}.0d,:0^[11}]1}.!0.0^_1[31}'(y6:d"
  "efine),.1q?{${.4,.4d,@(y12:xform-define)[02},${'(y6:define),.3da,.7,@("
  "y11:xenv-lookup)[03},.0?{${.2z,'(l2:y3:ref;y1:*;),@(y11:sexp-match?)[0"
  "2}}{f}?{t]5}.4,.2da,'(s40:identifier cannot be (re)defined in env:),@("
  "y7:x-error)[53}'(y13:define-syntax),.1q?{${.4,.4d,@(y19:xform-define-s"
  "yntax)[02},${'(y13:define-syntax),.3da,.7,@(y11:xenv-lookup)[03},.0?{."
  "1dda,.1sz]5}.4,.2da,'(s50:identifier cannot be (re)defined as syntax i"
  "n env:),@(y7:x-error)[53}.0K0?{.2,${.5,.5,.5[02},@(y14:visit-top-form)"
  "[32}.0U0?{t]3}.0Y0?{t]3}t]3}t]2",

  "P", "eval-top-form",
  "%2.0p?{${.3,.3a,t,@(y5:xform)[03},'(y5:begin),.1q?{.1d,,#0.4,.1,&2{%1."
  "0p?{${:1,.3a,@(y13:eval-top-form)[02}.0d,:0^[11}]1}.!0.0^_1[31}'(y6:de"
  "fine),.1q?{${.4,.4d,@(y12:xform-define)[02},${'(y6:define),.3da,.7,@(y"
  "11:xenv-lookup)[03},.0?{${.2z,'(l2:y3:ref;y1:*;),@(y11:sexp-match?)[02"
  "}}{f}?{.1dda,.1zda,'(y4:set!),l3,@(y25:compile-and-run-core-expr)[51}."
  "4,.2da,'(s40:identifier cannot be (re)defined in env:),@(y7:x-error)[5"
  "3}'(y13:define-syntax),.1q?{${.4,.4d,@(y19:xform-define-syntax)[02},${"
  "'(y13:define-syntax),.3da,.7,@(y11:xenv-lookup)[03},.0?{.1dda,.1sz]5}."
  "4,.2da,'(s50:identifier cannot be (re)defined as syntax in env:),@(y7:"
  "x-error)[53}.0K0?{.2,${.5,.5,.5[02},@(y13:eval-top-form)[32}.0U0?{${.4"
  ",.4d,.4,@(y16:xform-integrable)[03},@(y25:compile-and-run-core-expr)[3"
  "1}.0Y0?{${.4,.4,f,@(y5:xform)[03},@(y25:compile-and-run-core-expr)[31}"
  "${.4,.4d,.4,@(y10:xform-call)[03},@(y25:compile-and-run-core-expr)[31}"
  "${.3,.3,f,@(y5:xform)[03},@(y25:compile-and-run-core-expr)[21",

  "C", 0,
  "f@!(y9:*verbose*)",

  "P", "compile-and-run-core-expr",
  "%1.0p~?{${.2,'(s29:unexpected transformed output),@(y7:x-error)[02}}@("
  "y9:*verbose*)?{Po,.1W5PoW6}'(y6:define),.1aq?{'(y4:set!),.1sa}${.2,@(y"
  "21:compile-to-thunk-code)[01},@(y9:*verbose*)?{Po,.1W5PoW6}.0U4,U91,${"
  ".2[00},@(y9:*verbose*)?{Po,.1W5PoW6]4}]4",

  "P", "visit/v",
  "%1,#0${.3,@(y15:open-input-file)[01}.!0${${.4^,@(y14:read-code-sexp)[0"
  "1},,#0.4,.1,&2{%1.0R8~?{@(y9:*verbose*)?{Po,.1W5PoW6}${@(y16:root-envi"
  "ronment),.3,@(y14:visit-top-form)[02}@(y9:*verbose*)?{PoW6}${:1^,@(y14"
  ":read-code-sexp)[01},:0^[11}]1}.!0.0^_1[01}.0^P60]2",

  "P", "visit/x",
  "%1,#0${.3,@(y15:open-input-file)[01}.!0${${.4^,@(y14:read-code-sexp)[0"
  "1},,#0.4,.1,&2{%1.0R8~?{@(y9:*verbose*)?{Po,.1W5PoW6}${@(y16:root-envi"
  "ronment),.3,@(y13:eval-top-form)[02}@(y9:*verbose*)?{PoW6}${:1^,@(y14:"
  "read-code-sexp)[01},:0^[11}]1}.!0.0^_1[01}.0^P60]2",

  "P", "repl-environment",
  "%2.1,@(y18:*root-environment*),.2,@(y10:env-lookup)[23",

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
  "y6:define),.1q?{${.4,.4d,@(y12:xform-define)[02},${'(y6:define),.3da,."
  "7,@(y11:xenv-lookup)[03},.0?{${.2z,'(l2:y3:ref;y1:*;),@(y11:sexp-match"
  "?)[02}}{f}?{.1dda,.1zda,'(y4:set!),l3,@(y30:repl-compile-and-run-core-"
  "expr)[51}.4,.2da,'(s40:identifier cannot be (re)defined in env:),@(y7:"
  "x-error)[53}'(y13:define-syntax),.1q?{${.4,.4d,@(y19:xform-define-synt"
  "ax)[02},${'(y13:define-syntax),.3da,.7,@(y11:xenv-lookup)[03},.0?{.1dd"
  "a,.1sz}{${.6,.4da,'(s50:identifier cannot be (re)defined as syntax in "
  "env:),@(y7:x-error)[03}}@(y9:*verbose*)?{Po,'(s18:SYNTAX INSTALLED: )W"
  "4Po,.2daW5PoW6]5}]5}.0K0?{.2,${.5,.5,.5[02},@(y18:repl-eval-top-form)["
  "32}.0U0?{${.4,.4d,.4,@(y16:xform-integrable)[03},@(y30:repl-compile-an"
  "d-run-core-expr)[31}.0Y0?{${.4,.4,f,@(y5:xform)[03},@(y30:repl-compile"
  "-and-run-core-expr)[31}${.4,.4d,.4,@(y10:xform-call)[03},@(y30:repl-co"
  "mpile-and-run-core-expr)[31}${.3,.3,f,@(y5:xform)[03},@(y30:repl-compi"
  "le-and-run-core-expr)[21",

  "P", "repl-read",
  "%2.1?{PoW6Po,.2W4Po,'(s1: )W4}.0,@(y14:read-code-sexp)[21",

  "P", "repl-from-port",
  "%3${k0,.0,${.2,.8,.(i10),.9,&4{%0:3,&1{%!0.0,&1{%0:0,@(y6:values),@(y1"
  "3:apply-to-list)[02},:0[11},:0,:1,:2,&3{%0${:1,:2,@(y9:repl-read)[02},"
  ",#0:0,:2,:1,.3,&4{%1.0R8~?{${:3,.3,@(y18:repl-eval-top-form)[02}${:1,:"
  "2,@(y9:repl-read)[02},:0^[11}]1}.!0.0^_1[01},@(y16:call-with-values)[0"
  "2},.9,.9,.9,.6,&4{%1${k0,.0,${.6,:1,:2,:3,&4{%0:3,${.2,@(y13:error-obj"
  "ect?)[01}?{Pe,.0,${.4,@(y20:error-object-message)[01}W4.0W6${${.5,@(y2"
  "2:error-object-irritants)[01},.3,&1{%1:0,.1W5:0W6]1},@(y10:%25for-each"
  "1)[02}_1:0?{:0,:1,:2,@(y14:repl-from-port)[13}]1}Pe,.0,'(s14:Unknown e"
  "rror:)W4.0W6.0,.2W5.0W6_1:0?{:0,:1,:2,@(y14:repl-from-port)[13}]1},:0["
  "01}_1_3}[10},@(y22:with-exception-handler)[02}_1_3}[30",

  "P", "repl-file",
  "%2,#0${.3,@(y15:open-input-file)[01}.!0${f,.5,.4^,@(y14:repl-from-port"
  ")[03}.0^P60]3",

  "P", "benchmark-file",
  "%1,#0${.3,@(y15:open-input-file)[01}.!0${${.4^,@(y14:read-code-sexp)[0"
  "1},'(l2:y4:load;s7:libl.sf;),@(y11:sexp-match?)[02}~?{${.3,'(s32:unexp"
  "ected benchmark file format),@(y5:error)[02}}${f,@(y16:repl-environmen"
  "t),.4^,@(y14:repl-from-port)[03}${@(y16:repl-environment),'(l2:y4:main"
  ";f;),@(y18:repl-eval-top-form)[02}.0^P60]2",

  "P", "run-repl",
  "%0'(s6:skint]),@(y16:repl-environment),Pi,@(y14:repl-from-port)[03",

  0, 0, 0
};
