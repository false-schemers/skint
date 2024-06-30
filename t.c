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
  "]4}'(y8:<symbol>),.3q?{.3Y0}{f},.0?{.0]5}'(y8:<string>),.4q?{.4S0}{f},"
  ".0?{.0]6}.4,.6v,.0?{.0]7}.5p?{'(y3:...),.6aq?{.5dp?{.5ddu}{f}}{f}?{.5d"
  "a,.7v}{.5dp?{'(y3:...),.6daq?{.5ddu}{f}}{f}?{.5a,'(y1:*),.1q?{.7L0}{${"
  ".9,,#0.0,.5,&2{%1.0u,.0?{.0]2}.1p?{${.3a,:0,@(y11:sexp-match?)[02}?{.1"
  "d,:1^[21}f]2}f]2}.!0.0^_1[01}}_1}{.6p?{${.8a,.8a,@(y11:sexp-match?)[02"
  "}?{${.8d,.8d,@(y11:sexp-match?)[02}}{f}}{f}}}}{f},.0?{.0]8}.6V0?{.7V0?"
  "{${.9X0,.9X0,@(y11:sexp-match?)[02}}{f}}{f},.0?{.0]9}.7Y2?{.8Y2?{.8z,."
  "8z,@(y11:sexp-match?)[92}f]9}f]9",

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
  "'0,#0.0,&1{%!0'1,:0^I+:!0.0u?{'(i10),:0^X6,'(s1:#)S6X5]1}.0aY0?{'(i10)"
  ",:0^X6,'(s1:#)S6,.1aX4S6X5]1}'0:!0]1}_1@!(y6:gensym)",

  "P", "posq",
  "%2'0,.2,,#0.0,.4,&2{%2.0u?{f]2}.0a,:0q?{.1]2}'1,.2I+,.1d,:1^[22}.!0.0^"
  "_1[22",

  "P", "rassq",
  "%2.1p?{.1a,.0d,.2q?{.0]3}.2d,.2,@(y5:rassq)[32}f]2",

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

  "A", "val-core?", "pair?",

  "A", "location?", "box?",

  "A", "make-location", "box",

  "A", "location-val", "unbox",

  "A", "location-set-val!", "set-box!",

  "P", "location-special?",
  "%1.0zp~]1",

  "P", "new-id",
  "%3,#0.3,.3,.3,l3.!0.0,&1{%0:0^]0}]4",

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
  "%3.1b,.3,.1,.3,&3{%2.0,:0q?{.1,'(l2:y3:ref;y4:set!;),.1A1?{:1]3}f]3}.1"
  ",.1,:2[22}]4",

  "P", "add-local-var",
  "%3.2,.2,'(y3:ref),l2,.2,@(y17:extend-xenv-local)[33",

  "P", "xenv-lookup",
  "%3${.4,.4,.4[02},.0?{.0]4}.3,${.5,@(y7:id->sym)[01},.4,l3,'(s38:transf"
  "ormer: invalid identifier access),@(y6:error*)[42",

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
  ".1v?{.6,.3,@(y12:xform-syntax)[72}'(y5:quote),.1v?{.6,.3,@(y11:xform-q"
  "uote)[72}'(y4:set!),.1v?{.6,.3,@(y10:xform-set!)[72}'(y4:set&),.1v?{.6"
  ",.3,@(y10:xform-set&)[72}'(y2:if),.1v?{.6,.3,@(y8:xform-if)[72}'(y6:la"
  "mbda),.1v?{.6,.3,@(y12:xform-lambda)[72}'(y7:lambda*),.1v?{.6,.3,@(y13"
  ":xform-lambda*)[72}'(y5:letcc),.1v?{.6,.3,@(y11:xform-letcc)[72}'(y6:w"
  "ithcc),.1v?{.6,.3,@(y12:xform-withcc)[72}'(y4:body),.1v?{.4,.7,.4,@(y1"
  "0:xform-body)[73}'(y5:begin),.1v?{.4,.7,.4,@(y11:xform-begin)[73}'(y6:"
  "define),.1v?{.6,.3,@(y12:xform-define)[72}'(y13:define-syntax),.1v?{.6"
  ",.3,@(y19:xform-define-syntax)[72}'(y13:syntax-lambda),.1v?{.4,.7,.4,@"
  "(y19:xform-syntax-lambda)[73}'(y12:syntax-rules),.1v?{.6,.3,@(y18:xfor"
  "m-syntax-rules)[72}'(y13:syntax-length),.1v?{.6,.3,@(y19:xform-syntax-"
  "length)[72}'(y12:syntax-error),.1v?{.6,.3,@(y18:xform-syntax-error)[72"
  "}.1U0?{.6,.3,.3,@(y16:xform-integrable)[73}.1K0?{.6,${.9,.9,.6[02},.6,"
  "@(y5:xform)[73}.6,.3,.3,@(y10:xform-call)[73",

  "P", "xform-syntax",
  "%2${.2,@(y6:list1?)[01}?{.0a]2}.0,'(y6:syntax)c,'(s20:improper syntax "
  "form),@(y7:x-error)[22",

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
  "@(y8:idslist?)[01}}{f}}{f}}{f}?{.2aa,${f,'(y6:lambda)b,'(y6:lambda),@("
  "y6:new-id)[03},.4d,.5adc,.1c,${${.6,@(y7:id->sym)[01},@(y6:gensym)[01}"
  ",${.(i12),.3,.7,@(y13:add-local-var)[03},.(i10),.(i15),.3c,.(i15),.5c,"
  ".(i15),.8c,.4,:0^[(i16)5}.4,'(s20:improper define form),@(y7:x-error)["
  "(i11)2}'(y13:define-syntax),.1v?{${.4,@(y6:list2?)[01}?{${.4a,@(y3:id?"
  ")[01}}{f}?{.2a,.3da,${.(i10),'(l1:y9:undefined;),.5,@(y17:extend-xenv-"
  "local)[03},.8,.(i13),tc,.(i13),.4c,.(i13),.6c,.4,:0^[(i14)5}.4,'(s27:i"
  "mproper define-syntax form),@(y7:x-error)[(i11)2}.1K0?{.5,${.9,.8,.6[0"
  "2}c,.(i10),.(i10),.(i10),.(i10),:0^[(i11)5}:1,.7,.(i12),.(i12)A8,.(i12"
  ")A8,.(i12)A8,@(y12:xform-labels)[(i11)6}:1,.1,.6,.6A8,.6A8,.6A8,@(y12:"
  "xform-labels)[56}.!0.0^_1[35",

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

  "P", "xform-syntax-lambda",
  "%3${.2,@(y7:list2+?)[01}?{${.2a,@(y3:id?),@(y6:andmap)[02}}{f}?{.0d,.2"
  ",.2a,.5,.3,.2,.4,&4{%2${.2,@(y7:list1+?)[01}?{.0dg,:1gI=}{f}?{:0,.1d,:"
  "1,,#0.5,.1,:3,:2,&4{%3.0u?{${:1,.5,:0,@(y10:xform-body)[03},'(y6:synta"
  "x),l2]3}${.4,${:3,.7a,t,@(y5:xform)[03},.4a,@(y17:extend-xenv-local)[0"
  "3},.2d,.2d,:2^[33}.!0.0^_1[23}.0,'(s33:invalid syntax-lambda applicati"
  "on),@(y7:x-error)[22}]6}.0,'(y13:syntax-lambda)c,'(s27:improper syntax"
  "-lambda body),@(y7:x-error)[32",

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
  "%1,,,,,#0#1#2#3#4${f,'(y5:begin)b,'(y5:begin),@(y6:new-id)[03}.!0.0,&1"
  "{%2${.2,@(y6:list2?)[01}?{.0daS0}{f}~?{${.2,'(s14:invalid syntax),@(y7"
  ":x-error)[02}}${.2da,@(y18:push-current-file!)[01}:0^,l1]2}.!1.0,&1{%2"
  "${.2,@(y6:list1?)[01}~?{${.2,'(s14:invalid syntax),@(y7:x-error)[02}}$"
  "{@(y17:pop-current-file!)[00}:0^,l1]2}.!2${f,.4^b,'(y7:push-cf),@(y6:n"
  "ew-id)[03}.!3${f,.5^b,'(y6:pop-cf),@(y6:new-id)[03}.!4.3,.5,.7,.3,&4{%"
  "2${.2,@(y7:list1+?)[01}~?{${.2,'(s14:invalid syntax),@(y7:x-error)[02}"
  "}n,.1d,,#0:3,:2,.2,:1,:0,&5{%2.0u?{${.3A9,@(y7:%25append),@(y13:apply-"
  "to-list)[02},:0^c]2}${.2a,@(y32:file-resolve-relative-to-current)[01},"
  "${:1,.3,@(y15:read-file-sexps)[02},n,n,:3^cc,.1L6,n,.3c,:4^cc,.4,.1c,."
  "4d,:2^[52}.!0.0^_1[22}]6",

  "P", "if-feature-available-transformer",
  "%2.0L0?{'4,.1g=}{f}?{.0ddda,.1dda,.2da,${${.4,@(y17:xform-sexp->datum)"
  "[01},@(y18:feature-available?)[01}?{.1]5}.2]5}.0,'(s14:invalid syntax)"
  ",@(y7:x-error)[22",

  "P", "if-library-available-transformer",
  "%2.0L0?{'4,.1g=}{f}?{.0ddda,.1dda,.2da,${${.4,@(y17:xform-sexp->datum)"
  "[01},@(y18:library-available?)[01}?{.1]5}.2]5}.0,'(s14:invalid syntax)"
  ",@(y7:x-error)[22",

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
  "find-free)[22},@(y13:apply-to-list)[22}'(l2:y6:define;y13:define-synta"
  "x;),.1aA0?{.0d,.1,&1{%!0:0,'(s25:misplaced definition form),@(y7:c-err"
  "or)[12},@(y13:apply-to-list)[22}'(y16:record-case-miss)]2",

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
  "aq?{.0d,.2,&1{%2:0,.2,@(y9:find-sets)[22},@(y13:apply-to-list)[22}'(l2"
  ":y6:define;y13:define-syntax;),.1aA0?{.0d,.1,&1{%!0:0,'(s25:misplaced "
  "definition form),@(y7:c-error)[12},@(y13:apply-to-list)[22}'(y16:recor"
  "d-case-miss)]2",

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
  "arg)[12}]1},@(y13:apply-to-list)[72}'(y2:if),.1aq?{.0d,.6,.6,.6,.6,.6,"
  ".(i12),&6{%3${:0,f,:4,:3,:2,:1,.8,@(y7:codegen)[07}:0,'(c?)W0:0,'(c{)W"
  "0${:0,:5,:4,:3,:2,:1,.9,@(y7:codegen)[07}:0,'(c})W0:5?{:0,:5,:4,:3,:2,"
  ":1,.8,@(y7:codegen)[37}'(l1:y5:begin;),.3e,.0?{.0]4}.3?{:0,'(c{)W0${:0"
  ",:5,:4,:3,:2,:1,.(i11),@(y7:codegen)[07}:0,'(c})W0]4}f]4},@(y13:apply-"
  "to-list)[72}'(y6:lambda),.1aq?{.0d,.6,.8,.7,.7,.7,.7,&6{%2${.2,@(y15:f"
  "latten-idslist)[01},${:3,${.5,.8,@(y9:find-free)[02},@(y9:set-minus)[0"
  "2},${.3,.6,@(y9:find-sets)[02},${:0,.4A8,,#0.0,:4,:1,:3,&4{%2.0u?{]2}$"
  "{:2,f,:0,n,:1,.8,.8a,'(y3:ref),l2,@(y7:codegen)[07}:2,'(c,)W0.1,fc,.1d"
  ",:3^[22}.!0.0^_1[02}:4,'(c&)W0${:4,.4g,@(y20:write-serialized-arg)[02}"
  ":4,'(c{)W0.3L0?{:4,'(c%25)W0${:4,.6g,@(y20:write-serialized-arg)[02}}{"
  ":4,'(c%25)W0:4,'(c!)W0${:4,${.8,@(y17:idslist-req-count)[01},@(y20:wri"
  "te-serialized-arg)[02}}${'0,.5,,#0.0,.6,:4,&3{%2.0u?{]2}${:1,.3a,@(y11"
  ":set-member?)[02}?{:0,'(c#)W0${:0,.4,@(y20:write-serialized-arg)[02}}'"
  "1,.2I+,.1d,:2^[22}.!0.0^_1[02}${:4,.5g,:3,${${.(i10),:2,@(y13:set-inte"
  "rsect)[02},.8,@(y9:set-union)[02},.7,.9,.(i12),@(y7:codegen)[07}:4,'(c"
  "})W0_1_1_1:5?{:4,'(c])W0:4,:5,@(y20:write-serialized-arg)[22}]2},@(y13"
  ":apply-to-list)[72}'(y7:lambda*),.1aq?{.0d,.6,.8,.5,.7,.9,.7,&6{%!0${:"
  "0,.3A8,,#0.0,:4,:3,:2,:1,&5{%2.0u?{]2}${:3,f,:0,:1,:2,.8,.8ada,@(y7:co"
  "degen)[07}:3,'(c%25)W0:3,'(cx)W0:3,'(c,)W0.1,fc,.1d,:4^[22}.!0.0^_1[02"
  "}:4,'(c&)W0${:4,.3g,@(y20:write-serialized-arg)[02}:4,'(c{)W0${'0,.3,,"
  "#0.0,:4,&2{%2.0u?{]2}.0aa,.0a,.1da,:0,'(c|)W0.0?{:0,'(c!)W0}${:0,.4,@("
  "y20:write-serialized-arg)[02}${:0,.7,@(y20:write-serialized-arg)[02}_1"
  "_1_1'1,.2I+,.1d,:1^[22}.!0.0^_1[02}:4,'(c%25)W0:4,'(c%25)W0:4,'(c})W0:"
  "5?{:4,'(c])W0:4,:5,@(y20:write-serialized-arg)[12}]1},@(y13:apply-to-l"
  "ist)[72}'(y5:letcc),.1aq?{.0d,.4,.7,.7,.6,.6,.(i12),&6{%2.0,l1,${.2,.5"
  ",@(y9:find-sets)[02},${.2,${.6,:5,@(y9:set-minus)[02},@(y9:set-union)["
  "02},:4?{:0,'(ck)W0${:0,:4,@(y20:write-serialized-arg)[02}:0,'(c,)W0${."
  "3,.6,@(y11:set-member?)[02}?{:0,'(c#)W0:0,'(c0)W0}:0,'1,:4I+,:3,.3,:2,"
  ":1,.9c,.(i10),@(y7:codegen)[57}:0,'(c$)W0:0,'(c{)W0:0,'(ck)W0:0,'(c0)W"
  "0:0,'(c,)W0${.3,.6,@(y11:set-member?)[02}?{:0,'(c#)W0:0,'(c0)W0}${:0,f"
  ",:3,.5,:2,:1,fc,fc,.(i11)c,.(i12),@(y7:codegen)[07}:0,'(c_)W0${:0,'3,@"
  "(y20:write-serialized-arg)[02}:0,'(c})W0]5},@(y13:apply-to-list)[72}'("
  "y6:withcc),.1aq?{.0d,.7,.3,.5,.7,.9,&5{%2'(l3:y5:quote;y3:ref;y6:lambd"
  "a;),.2aA0?{${:4,f,:0,:1,:2,:3,.9,@(y7:codegen)[07}:4,'(c,)W0${:4,f,:0,"
  ":1,:2,:3,fc,.8,@(y7:codegen)[07}:4,'(cw)W0:4,'(c!)W0]2}${:4,f,:0,:1,:2"
  ",:3,.9,n,'(y6:lambda),l3,@(y7:codegen)[07}:4,'(c,)W0${:4,f,:0,:1,:2,:3"
  ",fc,.8,@(y7:codegen)[07}:4,'(cw)W0]2},@(y13:apply-to-list)[72}'(y10:in"
  "tegrable),.1aq?{.0d,.6,.8,.5,.7,.9,.7,&6{%!1'0,.2U8,.2U6,.0,'(l4:c0;c1"
  ";c2;c3;),.1A1?{${:0,.6A8,,#0.0,:4,:3,:2,:1,&5{%2.0u?{]2}${:3,f,:0,:1,:"
  "2,.8,.8a,@(y7:codegen)[07}.0du~?{:3,'(c,)W0}.1,fc,.1d,:4^[22}.!0.0^_1["
  "02}${:4,.5,@(y12:write-string)[02}}{'(cp),.1v?{.3u?{'1,.5U8,${:4,.3,@("
  "y12:write-string)[02}_1}{'1,.4gI-,${:0,.7A8,,#0.0,:4,:3,:2,:1,&5{%2.0u"
  "?{]2}${:3,f,:0,:1,:2,.8,.8a,@(y7:codegen)[07}.0du~?{:3,'(c,)W0}.1,fc,."
  "1d,:4^[22}.!0.0^_1[02}${'0,,#0.0,.8,:4,.7,&4{%1:0,.1I<!?{]1}${:1,:2,@("
  "y12:write-string)[02}'1,.1I+,:3^[11}.!0.0^_1[01}_1}}{'(cm),.1v?{.3du?{"
  "'1,.5U8,${:4,f,:1,:2,:3,:0,.(i12)a,@(y7:codegen)[07}${:4,.3,@(y12:writ"
  "e-string)[02}_1}{'1,.4gI-,${:0,.7A8,,#0.0,:4,:3,:2,:1,&5{%2.0u?{]2}${:"
  "3,f,:0,:1,:2,.8,.8a,@(y7:codegen)[07}.0du~?{:3,'(c,)W0}.1,fc,.1d,:4^[2"
  "2}.!0.0^_1[02}${'0,,#0.0,.8,:4,.7,&4{%1:0,.1I<!?{]1}${:1,:2,@(y12:writ"
  "e-string)[02}'1,.1I+,:3^[11}.!0.0^_1[01}_1}}{'(cc),.1v?{.3A8,'1,.5gI-,"
  "${:4,f,:1,:2,:3,:0,.9a,@(y7:codegen)[07}:4,'(c,)W0${:0,fc,.4d,,#0.0,:4"
  ",:3,:2,:1,&5{%2.0u?{]2}${:3,f,:0,:1,:2,.8,.8a,@(y7:codegen)[07}.0du~?{"
  ":3,'(c,)W0:3,'(c,)W0}.1,fc,fc,.1d,:4^[22}.!0.0^_1[02}${'0,,#0.0,.9,:4,"
  ".7,&4{%1:0,.1I<!?{]1}.0I=0~?{:1,'(c;)W0}${:1,:2,@(y12:write-string)[02"
  "}'1,.1I+,:3^[11}.!0.0^_1[01}_2}{'(cx),.1v?{'1,.4gI-,${:0,.7A8,,#0.0,:4"
  ",:3,:2,:1,&5{%2.0u?{]2}${:3,f,:0,:1,:2,.8,.8a,@(y7:codegen)[07}.0du~?{"
  ":3,'(c,)W0}.1,fc,.1d,:4^[22}.!0.0^_1[02}${'0,,#0.0,.8,:4,.7,&4{%1:0,.1"
  "I<!?{]1}${:1,:2,@(y12:write-string)[02}'1,.1I+,:3^[11}.!0.0^_1[01}_1}{"
  "'(cu),.1v?{.3u?{${:4,'1,.8U8,@(y12:write-string)[02}}{${:4,f,:1,:2,:3,"
  ":0,.(i11)a,@(y7:codegen)[07}}${:4,.5,@(y12:write-string)[02}}{'(cb),.1"
  "v?{.3du?{${:4,'1,.8U8,@(y12:write-string)[02}}{${:4,f,:1,:2,:3,:0,.(i1"
  "1)da,@(y7:codegen)[07}}:4,'(c,)W0${:4,f,:1,:2,:3,:0,fc,.(i11)a,@(y7:co"
  "degen)[07}${:4,.5,@(y12:write-string)[02}}{'(ct),.1v?{.3ddu?{${:4,'1,."
  "8U8,@(y12:write-string)[02}}{${:4,f,:1,:2,:3,:0,.(i11)dda,@(y7:codegen"
  ")[07}}:4,'(c,)W0${:4,f,:1,:2,:3,:0,fc,.(i11)da,@(y7:codegen)[07}:4,'(c"
  ",)W0${:4,f,:1,:2,:3,:0,fc,fc,.(i11)a,@(y7:codegen)[07}${:4,.5,@(y12:wr"
  "ite-string)[02}}{'(c#),.1v?{${:0,.6A8,,#0.0,:4,:3,:2,:1,&5{%2.0u?{]2}$"
  "{:3,f,:0,:1,:2,.8,.8a,@(y7:codegen)[07}:3,'(c,)W0.1,fc,.1d,:4^[22}.!0."
  "0^_1[02}${:4,.5,@(y12:write-string)[02}${:4,.6g,@(y20:write-serialized"
  "-arg)[02}}{${.3,'(s27:unsupported integrable type),@(y7:c-error)[02}}}"
  "}}}}}}}_1_2:5?{:4,'(c])W0:4,:5,@(y20:write-serialized-arg)[22}]2},@(y1"
  "3:apply-to-list)[72}'(y4:call),.1aq?{.0d,.7,.4,.6,.8,.6,.(i11),&6{%!1'"
  "(y6:lambda),.2aq?{.1daL0?{.1dag,.1gI=}{f}}{f}?{${:1,.3A8,,#0.0,:5,:4,:"
  "3,:2,&5{%2.0u?{]2}${:3,f,:0,:1,:2,.8,.8a,@(y7:codegen)[07}:3,'(c,)W0.1"
  ",fc,.1d,:4^[22}.!0.0^_1[02}.1da,.2dda,${.3,.3,@(y9:find-sets)[02},${.2"
  ",${.7,:3,@(y9:set-minus)[02},@(y9:set-union)[02},:1,.4L6,${'0,.7,,#0.0"
  ",.8,:5,&3{%2.0u?{]2}${:1,.3a,@(y11:set-member?)[02}?{:0,'(c#)W0${:0,.4"
  ",@(y20:write-serialized-arg)[02}}'1,.2I+,.1d,:2^[22}.!0.0^_1[02}:0?{:5"
  ",.6g,:0I+,:2,.4,:4,.5,.9,@(y7:codegen)[77}${:5,f,:2,.6,:4,.7,.(i11),@("
  "y7:codegen)[07}:5,'(c_)W0:5,.6g,@(y20:write-serialized-arg)[72}:0?{${:"
  "1,.3A8,,#0.0,:5,:4,:3,:2,.(i11),&6{%2.0u?{:4,f,:1,:2,:3,.6,:0,@(y7:cod"
  "egen)[27}${:4,f,:1,:2,:3,.8,.8a,@(y7:codegen)[07}:4,'(c,)W0.1,fc,.1d,:"
  "5^[22}.!0.0^_1[02}:5,'(c[)W0${:5,:0,@(y20:write-serialized-arg)[02}:5,"
  ".1g,@(y20:write-serialized-arg)[22}:5,'(c$)W0:5,'(c{)W0${:1,fc,fc,.3A8"
  ",,#0.0,:5,:4,:3,:2,.(i11),&6{%2.0u?{:4,f,:1,:2,:3,.6,:0,@(y7:codegen)["
  "27}${:4,f,:1,:2,:3,.8,.8a,@(y7:codegen)[07}:4,'(c,)W0.1,fc,.1d,:5^[22}"
  ".!0.0^_1[02}:5,'(c[)W0${:5,'0,@(y20:write-serialized-arg)[02}${:5,.3g,"
  "@(y20:write-serialized-arg)[02}:5,'(c})W0]2},@(y13:apply-to-list)[72}'"
  "(y3:asm),.1aq?{.0d,.6,.8,&2{%1${:0,.3,@(y12:write-string)[02}:1?{:0,'("
  "c])W0:0,:1,@(y20:write-serialized-arg)[12}]1},@(y13:apply-to-list)[72}"
  "'(y4:once),.1aq?{.0d,.7,.7,.7,.7,.7,.7,&6{%2:5,:4,:3,:2,:1,:0,n,n,.9c,"
  "n,n,tc,'(y5:quote)cc,.9c,'(y5:gset!)cc,'(y5:begin)cc,n,'(y5:begin)cc,n"
  ",n,tc,'(y5:quote)cc,n,.9c,'(y4:gref)cc,'(y3:eq?)U5c,'(y10:integrable)c"
  "c,'(y2:if)c,@(y7:codegen)[27},@(y13:apply-to-list)[72}'(l2:y6:define;y"
  "13:define-syntax;),.1aA0?{.0d,.1,&1{%!0:0,'(s25:misplaced definition f"
  "orm),@(y7:c-error)[12},@(y13:apply-to-list)[72}'(y16:record-case-miss)"
  "]7",

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

  "P", "current-file-stack",
  "%0@(y20:*current-file-stack*)]0",

  "P", "set-current-file-stack!",
  "%1.0@!(y20:*current-file-stack*)]1",

  "P", "current-file",
  "%0@(y20:*current-file-stack*)p?{@(y20:*current-file-stack*)a]0}f]0",

  "P", "push-current-file!",
  "%1${@(y8:string=?),@(y20:*current-file-stack*),.4,@(y7:%25member)[03}?"
  "{${.2,'(s33:circularity in include file chain),@(y7:x-error)[02}}@(y20"
  ":*current-file-stack*),.1c@!(y20:*current-file-stack*)]1",

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

  "P", "read-file-sexps",
  "%2.1,&1{%1:0?{t,.1P79}n,,#0.2,.1,&2{%1${:1,@(y14:read-code-sexp)[01},."
  "0R8?{.1A9]2}.1,.1c,:0^[21}.!0.0^_1[11},.1,@(y20:call-with-input-file)["
  "22",

  "P", "call-with-input-file/lib",
  "%3${.2,@(y27:resolve-input-file/lib-name)[01},.2,.1,.5,&3{%0:0,:1,:2,&"
  "3{%1:0?{t,.1P79}.0,:1,:2[12},:1,@(y20:call-with-input-file)[02},.1,@(y"
  "17:with-current-file)[42",

  "P", "call-with-file/lib-sexps",
  "%3.2,&1{%2n,,#0.3,:0,.2,&3{%1${:2,@(y14:read-code-sexp)[01},.0R8?{.1A9"
  ",:1[21}.1,.1c,:0^[21}.!0.0^_1[21},.2,.2,@(y24:call-with-input-file/lib"
  ")[33",

  "P", "for-each-file/lib-sexp",
  "%3.0,&1{%2,#0.2,.1,:0,&3{%0${:2,@(y14:read-code-sexp)[01},.0R8~?{${.2,"
  ":0[01}:1^[10}]1}.!0.0^_1[20},.3,.3,@(y24:call-with-input-file/lib)[33",

  "P", "fully-qualified-library-prefixed-name",
  "%2.1,'(y1:?),.2Y0?{.2}{${.4,@(y16:listname->symbol)[01}},@(y13:symbol-"
  "append)[23",

  "P", "env-lookup",
  "%3.0K0?{.2,'(l2:y3:ref;y4:set!;),.1A1?{.1,@(y7:old-den)[41}f]4}.1,,#0."
  "2,.5,.2,&3{%1.0p?{:2,.1aaq?{:1,'(y3:ref),.1v?{.1ad]2}f]2}.0d,:0^[11}.0"
  "V0?{.0V3,.0,:2H2,.0,.3V4,.0,:2A3,.0?{:1,'(y3:ref),.1v?{.1da]6}.1dda?{f"
  "]6}.1da]6}:2U5,.0?{.0}{:2,'(y3:ref),l2}_1b,.2,f,.2,:2,l3c,.4,.7V5.0]6}"
  ".0S0?{'(l4:y3:ref;y4:set!;y6:define;y13:define-syntax;),:1A0?{:2X4,.1S"
  "6X5,'(y3:ref),@(y18:*root-environment*),.2,@(y10:env-lookup)[23}f]1}.0"
  "K0?{:1,:2,.2[12}f]1}.!0.0^_1[31",

  "C", 0,
  "n,'(i101)V2@!(y18:*root-environment*)",

  "P", "define-in-root-environment!",
  "%3@(y18:*root-environment*),.0V3,.0,.3H2,.0,.3V4,.0,.5A3,.0?{.6,.1dsa."
  "7,.1ddsa]8}.1,.8,.8,.8,l3c,.3,.6V5]8",

  "C", 0,
  "${t,${f,@(y24:make-include-transformer)[01}b,'(y7:include),@(y27:defin"
  "e-in-root-environment!)[03}",

  "C", 0,
  "${t,${t,@(y24:make-include-transformer)[01}b,'(y10:include-ci),@(y27:d"
  "efine-in-root-environment!)[03}",

  "C", 0,
  "${t,@(y32:if-feature-available-transformer)b,'(y20:if-feature-availabl"
  "e),@(y27:define-in-root-environment!)[03}",

  "C", 0,
  "${t,@(y32:if-library-available-transformer)b,'(y20:if-library-availabl"
  "e),@(y27:define-in-root-environment!)[03}",

  "C", 0,
  "&0{%2t,.2,.2,@(y27:define-in-root-environment!)[23},${U1,,#0.0,.5,&2{%"
  "1.0u?{'(y2:ok)]1}.0d,.1a,.0d,.1a,.1Y0,.0?{.0}{.2N0}_1?{${.3b,.3,:0[02}"
  ".3,:1^[51}.1p?{'(y12:syntax-rules),.2aq}{f}?{,,#0#1&0{%2.1,@(y18:*root"
  "-environment*),.2,@(y10:env-lookup)[23}.!0${.5da,@(y3:id?)[01}?{${.5dd"
  "d,.6dda,.7da,.5^,@(y13:syntax-rules*)[04}}{${.5dd,.6da,f,.5^,@(y13:syn"
  "tax-rules*)[04}}.!1${.3^b,.5,:0[02}.5,:1^[71}f]5}.!0.0^_1[01}_1",

  "P", "root-environment",
  "%2.1,@(y18:*root-environment*),.2,@(y10:env-lookup)[23",

  "C", 0,
  "n@!(y20:*std-lib->alist-env*)",

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
  "b),.1v?{'(l2:y6:scheme;y4:base;)]2}]2}.!0&0{%3.0d,.2A3,.0?{.3,.1sd]4}."
  "1d,.4,.4cc,.2sd]4}.!1&0{%1@(y20:*std-lib->alist-env*),.1A5,.0?{.0]2}n,"
  ".2c,@(y20:*std-lib->alist-env*),.1c@!(y20:*std-lib->alist-env*).0]3}.!"
  "2.3d,.4a,,#0.0,.5,.5,.8,&4{%2.1u?{${'(y3:ref),.3,@(y16:root-environmen"
  "t)[02},.1,${'(l1:y4:repl;),:0^[01},:2^[23}${${'(y3:ref),.5,@(y16:root-"
  "environment)[02},.3,${${.9a,:1^[01},:0^[01},:2^[03}.1d,.1,:3^[22}.!0.0"
  "^_1[42},@(y10:%25for-each1)[02}",

  "P", "std-lib->alist-env",
  "%1@(y20:*std-lib->alist-env*),.1A5,.0?{.0d]2}f]2",

  "P", "std-lib->env",
  "%1${.2,@(y18:std-lib->alist-env)[01},.0?{.0,.0,&1{%2'(y3:ref),.2q?{:0,"
  ".1A3,.0?{.0d]3}f]3}f]2}]3}f]2",

  "C", 0,
  "${@(y20:*std-lib->alist-env*),${f,'(y6:syntax)b,'(y6:syntax),@(y6:new-"
  "id)[03},.0,&1{%1.0a,.1d,${.3,@(y16:listname->symbol)[01},,#0.2,:0,&2{%"
  "2:1,'(l1:y5:begin;)c,'(y5:quote),l2,:0,l2]2}.!0t,.1^b,.3,@(y27:define-"
  "in-root-environment!)[53}_1,@(y10:%25for-each1)[02}",

  "P", "adjoin-env",
  "%2.0u?{.1]2}${.3,.3d,@(y10:adjoin-env)[02},${'(y3:ref),.3,.5aa,@(y10:e"
  "nv-lookup)[03},.0?{.0,.0,.4adq?{.2]5}.0,.4ad,.5aa,'(s39:multiple ident"
  "ifier bindings on import:),@(y7:c-error)[54}.1,.3ac]4",

  "P", "adjoin-env/shadow",
  "%2.0u?{.1]2}${.3,.3d,@(y17:adjoin-env/shadow)[02},${'(y3:ref),.3,.5aa,"
  "@(y10:env-lookup)[03},.0?{.0,@(y1:a),.4adq?{.2]5}${.5ad,'(y4:now:),@(y"
  "1:a),'(y4:was:),.9aa,'(s42:old identifier binding shadowed on import:)"
  ",@(y9:c-warning)[06}.2,.4ac]5}.1,.3ac]4",

  "P", "make-local-env",
  "%3.2,.1,,#0.0,.5,&2{%2.0u?{${:0,@(y11:lib-public?)[01}?{t,.2L6]2}${:0,"
  "@(y30:fully-qualified-library-prefix)[01},.2L6]2}.0a,${.2,'(y8:<symbol"
  ">),@(y11:sexp-match?)[02}?{.2}{${.2,'(l3:y6:rename;y8:<symbol>;y8:<sym"
  "bol>;),@(y11:sexp-match?)[02}?{.2}{${.3a,'(s30:invalid export spec in "
  "export:),@(y7:c-error)[02}}}_1,.1d,:1^[22}.!0.0^_1[32",

  "P", "make-export-env",
  "%3,#0.3,.3,.3,&3{%3.2,.2A3?{:0,.2,'(s32:duplicate external id in expor"
  "t:),@(y7:c-error)[33}:2,.1A3,.0?{.0,.4,.1d,.5cc]5}.3,${.4,:1,@(y32:ful"
  "ly-qualified-library-location)[02},.4cc]4}.!0${.4,@(y11:lib-public?)[0"
  "1}?{.1,.0?{.0}{.4p}_1?{.2,'(s26:module cannot be imported:),@(y7:c-err"
  "or)[42}n]4}n,.2,,#0.3,.1,&2{%2.0u?{.1]2}.0a,${.2,'(y8:<symbol>),@(y11:"
  "sexp-match?)[02}?{${.4,.4a,.5a,:1^[03}}{${.2,'(l3:y6:rename;y8:<symbol"
  ">;y8:<symbol>;),@(y11:sexp-match?)[02}?{${.4,.4adda,.5ada,:1^[03}}{${."
  "3a,'(s30:invalid export spec in export:),@(y7:c-error)[02}}}_1,.1d,:0^"
  "[22}.!0.0^_1[42",

  "C", 0,
  "n@!(y20:*library-info-cache*)",

  "P", "make-library-info",
  "%0f,'4V2]0",

  "P", "library-available?",
  "%1@(y20:*library-info-cache*),.1A5?{t]1}.0S0?{.0,@(y32:file-resolve-re"
  "lative-to-current)[11}.0p?{.0L0}{f}?{.0,@(y17:find-library-path)[11}f]"
  "1",

  "P", "lookup-library-info",
  "%1@(y20:*library-info-cache*),.1A5,.0?{.0d]2}${.3,@(y14:std-lib->alist"
  ")[01},.0?{.0,,#0${@(y17:make-library-info)[00}.!0@(y20:*library-info-c"
  "ache*),.1^,.6cc@!(y20:*library-info-cache*)n,'0,.2^V5.1,'1,.2^V5${.3,@"
  "(y3:car),@(y5:%25map1)[02},'2,.2^V5n,'3,.2^V5.0^]5}${@(y17:make-librar"
  "y-info)[00},@(y20:*library-info-cache*),.1,.5cc@!(y20:*library-info-ca"
  "che*).0]4",

  "P", "get-library-info",
  "%3,,#0#1${.4,@(y19:lookup-library-info)[01}.!0.0,&1{%4.0,'0,:0^V5.1,'1"
  ",:0^V5.2,'2,:0^V5.3,'3,:0^V5]4}.!1'0,.1^V4~?{${.3,.5,.7,&3{%1:2^,.1,:1"
  ",:0[13},f,.6,@(y24:call-with-file/lib-sexps)[03}}'3,.1^V4,'2,.2^V4,'1,"
  ".3^V4,'0,.4^V4,.8[54",

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
  "y6:define),.1q?{.1dau}{f}?{.2,.2dda,@(y18:repl-eval-top-form)[32}'(y6:"
  "define),.1q?{${.4,.4d,@(y12:xform-define)[02},${'(y6:define),.3da,.7,@"
  "(y11:xenv-lookup)[03},.0?{${.2z,'(l2:y3:ref;y1:*;),@(y11:sexp-match?)["
  "02}}{f}?{.1dda,.1zda,'(y4:set!),l3,@(y30:repl-compile-and-run-core-exp"
  "r)[51}.4,.2da,'(s40:identifier cannot be (re)defined in env:),@(y7:x-e"
  "rror)[53}'(y13:define-syntax),.1q?{${.4,.4d,@(y19:xform-define-syntax)"
  "[02},${'(y13:define-syntax),.3da,.7,@(y11:xenv-lookup)[03},.0?{.1dda,."
  "1sz}{${.6,.4da,'(s50:identifier cannot be (re)defined as syntax in env"
  ":),@(y7:x-error)[03}}@(y9:*verbose*)?{Po,'(s18:SYNTAX INSTALLED: )W4Po"
  ",.2daW5PoW6]5}]5}.0K0?{.2,${.5,.5,.5[02},@(y18:repl-eval-top-form)[32}"
  ".0U0?{${.4,.4d,.4,@(y16:xform-integrable)[03},@(y30:repl-compile-and-r"
  "un-core-expr)[31}.0Y0?{${.4,.4,f,@(y5:xform)[03},@(y30:repl-compile-an"
  "d-run-core-expr)[31}${.4,.4d,.4,@(y10:xform-call)[03},@(y30:repl-compi"
  "le-and-run-core-expr)[31}${.3,.3,f,@(y5:xform)[03},@(y30:repl-compile-"
  "and-run-core-expr)[21",

  "P", "repl-read",
  "%2.1?{PoW6Po,.2W4Po,'(s1: )W4}.0,@(y14:read-code-sexp)[21",

  "P", "repl-from-port",
  "%3,#0${@(y18:current-file-stack)[00}.!0${k0,.0,${.2,.9,.(i11),.(i10),&"
  "4{%0:3,&1{%!0.0,&1{%0:0,@(y6:values),@(y13:apply-to-list)[02},:0[11},:"
  "0,:1,:2,&3{%0${:1,:2,@(y9:repl-read)[02},,#0:0,:2,:1,.3,&4{%1.0R8~?{${"
  ":3,.3,@(y18:repl-eval-top-form)[02}${:1,:2,@(y9:repl-read)[02},:0^[11}"
  "]1}.!0.0^_1[01},@(y16:call-with-values)[02},.(i10),.(i10),.(i10),.(i10"
  "),.7,&5{%1${k0,.0,${.6,:1,:2,:3,:4,&5{%0:4,${.2,@(y13:error-object?)[0"
  "1}?{Pe,.0,${.4,@(y20:error-object-message)[01}W4.0W6${${.5,@(y22:error"
  "-object-irritants)[01},.3,&1{%1:0,.1W5:0W6]1},@(y10:%25for-each1)[02}_"
  "1${:3^,@(y23:set-current-file-stack!)[01}:0?{:0,:1,:2,@(y14:repl-from-"
  "port)[13}]1}Pe,.0,'(s14:Unknown error:)W4.0W6.0,.2W5.0W6_1${:3^,@(y23:"
  "set-current-file-stack!)[01}:0?{:0,:1,:2,@(y14:repl-from-port)[13}]1},"
  ":0[01}_1_3}[10},@(y22:with-exception-handler)[02}_1_3}[40",

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
  "%0${n,@(y23:set-current-file-stack!)[01}'(s6:skint]),@(y16:repl-enviro"
  "nment),Pi,@(y14:repl-from-port)[03",

  0, 0, 0
};
