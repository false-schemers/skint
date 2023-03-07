/* t.c -- generated via skint -c t.scm */

char *t_code[] = {

  0,
  "&0{%2.1u?{f]2}.1a,.1q?{t]2}.1d,.1,@(y11:set-member?)[22}@!(y11:set-mem"
  "ber?)",

  0,
  "&0{%2${.3,.3,@(y11:set-member?)[02}?{.1]2}.1,.1c]2}@!(y8:set-cons)",

  0,
  "&0{%2.0u?{.1]2}${.3,.3a,@(y8:set-cons)[02},.1d,@(y9:set-union)[22}@!(y"
  "9:set-union)",

  0,
  "&0{%2.0u?{n]2}${.3,.3a,@(y11:set-member?)[02}?{.1,.1d,@(y9:set-minus)["
  "22}${.3,.3d,@(y9:set-minus)[02},.1ac]2}@!(y9:set-minus)",

  0,
  "&0{%2.0u?{n]2}${.3,.3a,@(y11:set-member?)[02}?{${.3,.3d,@(y13:set-inte"
  "rsect)[02},.1ac]2}.1,.1d,@(y13:set-intersect)[22}@!(y13:set-intersect)",

  "record-case",
  "l6:y12:syntax-rules;l1:y4:else;;l2:l4:y11:record-case;py2:pa;y2:ir;;y6"
  ":clause;y3:...;;l3:y3:let;l1:l2:y2:id;py2:pa;y2:ir;;;;l4:y11:record-ca"
  "se;y2:id;y6:clause;y3:...;;;;l2:l2:y11:record-case;y2:id;;l2:y5:quote;"
  "y16:record-case-miss;;;l2:l3:y11:record-case;y2:id;l3:y4:else;y3:exp;y"
  "3:...;;;l3:y5:begin;y3:exp;y3:...;;;l2:l5:y11:record-case;y2:id;l4:y3:"
  "key;y3:ids;y3:exp;y3:...;;y6:clause;y3:...;;l4:y2:if;l3:y3:eq?;l2:y3:c"
  "ar;y2:id;;l2:y5:quote;y3:key;;;l3:y5:apply;l4:y6:lambda;y3:ids;y3:exp;"
  "y3:...;;l2:y3:cdr;y2:id;;;l4:y11:record-case;y2:id;y6:clause;y3:...;;;"
  ";",

  0,
  "&0{%2'(y1:*),.1q,.0?{.0]3}.1,.3e,.0?{.0]4}.2p?{'(y1:$),.3aq?{.2dp?{.2d"
  "du}{f}}{f}?{.2da,.4q]4}.2dp?{'(y3:...),.3daq?{.2ddu}{f}}{f}?{.2a,,#0.0"
  ",.2,&2{%1.0u,.0?{.0]2}.1p?{${.3a,:0,@(y13:syntax-match?)[02}?{.1d,:1^["
  "21}f]2}f]2}.!0.5,.1^[61}.3p?{${.5a,.5a,@(y13:syntax-match?)[02}?{.3d,."
  "3d,@(y13:syntax-match?)[42}f]4}f]4}f]4}@!(y13:syntax-match?)",

  0,
  "'0,#0.0,&1{%!0'1,:0^I+:!0.0u?{'(i10),:0^X6,'(s1:#)S6X5]1}.0aY0?{'(i10)"
  ",:0^X6,'(s1:#)S6,.1aX4S6X5]1}'0:!0]1}_1@!(y6:gensym)",

  0,
  "&0{%2'0,.2,,#0.0,.4,&2{%2.0u?{f]2}.0a,:0q?{.1]2}'1,.2I+,.1d,:1^[22}.!0"
  ".0^_1[22}@!(y4:posq)",

  0,
  "&0{%2.0u,.0?{.0}{.2,.2q}_1?{n]2}${.3,.3d,@(y9:list-diff)[02},.1ac]2}@!"
  "(y9:list-diff)",

  0,
  "&0{%!1.0,.2,,#0.0,&1{%2.1u?{.0]2}${.3d,.4a,:0^[02},.1c]2}.!0.0^_1[22}@"
  "!(y5:pair*)",

  0,
  "&0{%1.0p?{.0du]1}f]1}@!(y6:list1?)",

  0,
  "&0{%1.0p?{.0d,@(y6:list1?)[11}f]1}@!(y6:list2?)",

  0,
  "&0{%1.0p?{.0d,@(y6:list2?)[11}f]1}@!(y6:list3?)",

  0,
  "&0{%1.0p?{.0d,@(y6:list3?)[11}f]1}@!(y6:list4?)",

  0,
  "&0{%1${.2,@(y6:list2?)[01}?{.0aI0?{.0daY1}{f}}{f}?{.0]1}.0,'0,,#0.0,&1"
  "{%2.1p?{.1d,.1,'1I+,:0^[22}.1u?{f,.1,l2]2}t,.1,l2]2}.!0.0^_1[12}@!(y15"
  ":normalize-arity)",

  0,
  "&0{%1.0L0?{.0]1}n,.1,,#0.0,&1{%2.0p?{.1,.1ac,.1d,:0^[22}.0u?{.1A9]2}.1"
  "A9,.1c]2}.!0.0^_1[12}@!(y15:flatten-idslist)",

  0,
  "&0{%1.0p?{${.2d,@(y17:idslist-req-count)[01},'1I+]1}'0]1}@!(y17:idslis"
  "t-req-count)",

  0,
  "@(y15:%25residual-pair?)@!(y9:val-core?)",

  0,
  "&0{%1.0p~]1}@!(y12:val-special?)",

  0,
  "@(y15:%25residual-pair?)@!(y8:binding?)",

  0,
  "@(y14:%25residual-cons)@!(y12:make-binding)",

  0,
  "@(y13:%25residual-cdr)@!(y11:binding-val)",

  0,
  "&0{%1.0d,@(y12:val-special?)[11}@!(y16:binding-special?)",

  0,
  "@(y13:%25residual-car)@!(y11:binding-sym)",

  0,
  "@(y18:%25residual-set-cdr!)@!(y16:binding-set-val!)",

  0,
  "@(y14:%25residual-assq)@!(y16:find-top-binding)",

  0,
  "&0{%1,#0n,.2c.!0.0,&1{%0:0^]0}]2}@!(y6:new-id)",

  0,
  "&0{%1${.2[00}a]1}@!(y7:old-den)",

  0,
  "&0{%1.0Y0,.0?{.0]2}.1K0]2}@!(y3:id?)",

  0,
  "&0{%1.0Y0?{.0]1}${.2,@(y7:old-den)[01},@(y8:den->sym)[11}@!(y7:id->sym"
  ")",

  0,
  "&0{%1.0Y0?{.0]1}.0,@(y11:binding-sym)[11}@!(y8:den->sym)",

  0,
  "&0{%1.0Y0?{.0]1}.0,@(y7:old-den)[11}@!(y10:empty-xenv)",

  0,
  "&0{%3.0,.3,.3,&3{%1.0,:0q?{:1]1}.0,:2[11}]3}@!(y11:extend-xenv)",

  0,
  "&0{%3${.3,${.5,@(y7:id->sym)[01},@(y12:make-binding)[02},.1,.4,@(y11:e"
  "xtend-xenv)[33}@!(y11:add-binding)",

  0,
  "&0{%3${.3,'(y3:ref),l2,${.5,@(y7:id->sym)[01},@(y12:make-binding)[02},"
  ".1,.4,@(y11:extend-xenv)[33}@!(y7:add-var)",

  0,
  "&0{%3${.3,@(y3:id?)[01}?{${.4,.4,@(y9:xform-ref)[02},.0K0?{.1~}{f}?{.3"
  ",${.6,.6,.5[02},.3,@(y5:xform)[43}.0]4}.1p~?{.2,.2,@(y11:xform-quote)["
  "32}.1a,.2d,${.6,.4,t,@(y5:xform)[03},.0,'(l1:y6:syntax;)%l,.1A1?{.2a]7"
  "}'(l1:y5:quote;)%l,.1A1?{.6,.3a,@(y11:xform-quote)[72}'(l1:y4:set!;)%l"
  ",.1A1?{.6,.3da,.4a,@(y10:xform-set!)[73}'(l1:y5:begin;)%l,.1A1?{.6,.3,"
  "@(y11:xform-begin)[72}'(l1:y2:if;)%l,.1A1?{.6,.3,@(y8:xform-if)[72}'(l"
  "1:y6:lambda;)%l,.1A1?{.6,.3,@(y12:xform-lambda)[72}'(l1:y7:lambda*;)%l"
  ",.1A1?{.6,.3,@(y13:xform-lambda*)[72}'(l1:y4:body;)%l,.1A1?{.6,.3,@(y1"
  "0:xform-body)[72}'(l1:y6:define;)%l,.1A1?{.6,.3da,.4a,@(y12:xform-defi"
  "ne)[73}'(l1:y13:define-syntax;)%l,.1A1?{.6,.3da,.4a,@(y19:xform-define"
  "-syntax)[73}t?{.1K0?{.6,${.9,.9,.6[02},.6,@(y5:xform)[73}.6,.3,.3,@(y1"
  "0:xform-call)[73}f]7}@!(y5:xform)",

  0,
  "&0{%2${.2,,#0.0,&1{%1${.2,@(y3:id?)[01}?{.0,@(y7:id->sym)[11}.0p?{${.2"
  "d,:0^[01},${.3a,:0^[01}c]1}.0V0?{:0^,${${.5,@(y12:vector->list)[01},,#"
  "0.4,.1,&2{%1.0p?{${.2d,:0^[01},${.3a,:1[01}c]1}n]1}.!0.0^_1[01}_1X1]1}"
  ".0]1}.!0.0^_1[01},'(y5:quote),l2]2}@!(y11:xform-quote)",

  0,
  "&0{%2${.2,.4[01},.0Y0?{.0,'(y3:ref),l2]3}.0,@(y11:binding-val)[31}@!(y"
  "9:xform-ref)",

  0,
  "&0{%3${.4,.4,f,@(y5:xform)[03},${.3,.6[01},.0Y0?{.1,.1,'(y4:set!),l3]5"
  "}${.2,@(y16:binding-special?)[01}?{${.3,.3,@(y16:binding-set-val!)[02}"
  "'(l1:y5:begin;)]5}${.2,@(y11:binding-val)[01},'(y3:ref),.1aq?{.2,.1da,"
  "'(y4:set!),l3]6}'(s27:set! to non-identifier form),'(y9:transform),@(y"
  "5:error)[62}@!(y10:xform-set!)",

  0,
  "&0{%2.0L0?{.1,&1{%1:0,.1,f,@(y5:xform)[13},${.3,,#0.4,.1,&2{%1.0p?{${."
  "2d,:0^[01},${.3a,:1[01}c]1}n]1}.!0.0^_1[01}_1,.0p?{.0du}{f}?{.0a]3}.0,"
  "'(y5:begin)c]3}'(s19:improper begin form),'(y9:transform),@(y5:error)["
  "22}@!(y11:xform-begin)",

  0,
  "&0{%2.0L0?{.1,&1{%1:0,.1,f,@(y5:xform)[13},${.3,,#0.4,.1,&2{%1.0p?{${."
  "2d,:0^[01},${.3a,:1[01}c]1}n]1}.!0.0^_1[01}_1,.0g,'(l1:i2;)%l,.1A1?{'("
  "l1:l1:y5:begin;;),.2L6,'(y2:if)c]4}'(l1:i3;)%l,.1A1?{.1,'(y2:if)c]4}t?"
  "{'(s17:malformed if form),'(y9:transform),@(y5:error)[42}f]4}'(s16:imp"
  "roper if form),'(y9:transform),@(y5:error)[22}@!(y8:xform-if)",

  0,
  "&0{%3.1L0?{.2,&1{%1:0,.1,f,@(y5:xform)[13},${.4,,#0.4,.1,&2{%1.0p?{${."
  "2d,:0^[01},${.3a,:1[01}c]1}n]1}.!0.0^_1[01}_1,.0u?{'(y6:lambda),.2aq?{"
  ".1dau}{f}}{f}?{.1dda]4}.0,.2,'(y4:call),@(y5:pair*)[43}'(s20:improper "
  "application),'(y9:transform),@(y5:error)[32}@!(y10:xform-call)",

  0,
  "&0{%2.0L0?{n,.2,.2a,,#0.4,.1,&2{%3.0p?{.0a,${${.4,@(y7:id->sym)[01},@("
  "y6:gensym)[01},.4,.1c,${.6,.4,.6,@(y7:add-var)[03},.4d,:0^[53}.0u?{${."
  "3,:1d,@(y10:xform-body)[02},.3A8,'(y6:lambda),l3]3}.0,${${.4,@(y7:id->"
  "sym)[01},@(y6:gensym)[01},${.5,.3,.5,@(y7:add-var)[03},${.2,:1d,@(y10:"
  "xform-body)[02},.2,.7A8L6,'(y6:lambda),l3]6}.!0.0^_1[23}'(s20:improper"
  " lambda body),'(y9:transform),@(y5:error)[22}@!(y12:xform-lambda)",

  0,
  "&0{%2.0L0?{.1,&1{%1${.2,@(y6:list2?)[01}?{${:0,.3da,f,@(y5:xform)[03},"
  "${.3a,@(y15:normalize-arity)[01},l2]1}'(s23:improper lambda* clause),'"
  "(y9:transform),@(y5:error)[12},${.3,,#0.4,.1,&2{%1.0p?{${.2d,:0^[01},$"
  "{.3a,:1[01}c]1}n]1}.!0.0^_1[01}_1,'(y7:lambda*)c]2}'(s21:improper lamb"
  "da* form),'(y9:transform),@(y5:error)[22}@!(y13:xform-lambda*)",

  0,
  "&0{%2.0u?{n,'(y5:begin)c]2}.0,n,n,n,.5,,#0.0,&1{%5.4p?{.4ap}{f}?{.4d,."
  "5a,.0a,${.5,.3,t,@(y5:xform)[03},.0,'(l1:y5:begin;)%l,.1A1?{.4,.4dL6,."
  "9,.9,.9,.9,:0^[(i10)5}'(l1:y6:define;)%l,.1A1?{.3da,.4dda,${${.5,@(y7:"
  "id->sym)[01},@(y6:gensym)[01},${.(i10),.3,.6,@(y7:add-var)[03},.8,.(i1"
  "3),.3c,.(i13),.5c,.(i13),.7c,.4,:0^[(i14)5}'(l1:y13:define-syntax;)%l,"
  ".1A1?{.3da,.4dda,${.9,'(l1:y9:undefined;),.5,@(y11:add-binding)[03},.7"
  ",.(i12),tc,.(i12),.4c,.(i12),.6c,.4,:0^[(i13)5}t?{.1K0?{.4,${.8,.7,.6["
  "02}c,.9,.9,.9,.9,:0^[(i10)5}.5,.(i10),.(i10)A8,.(i10)A8,.(i10)A8,@(y12"
  ":xform-labels)[(i10)5}f](i10)}.0,.5,.5A8,.5A8,.5A8,@(y12:xform-labels)"
  "[55}.!0.0^_1[25}@!(y10:xform-body)",

  0,
  "&0{%5n,n,.4,.4,.4,,#0.0,.(i11),.(i11),&3{%5.0u?{:1,&1{%1:0,.1,f,@(y5:x"
  "form)[13},${:0,,#0.4,.1,&2{%1.0p?{${.2d,:0^[01},${.3a,:1[01}c]1}n]1}.!"
  "0.0^_1[01}_1,.4A8L6,.0p?{.0du}{f}?{.0a}{.0,'(y5:begin)c},.6u?{.0]7}&0{"
  "%1'(l1:y5:begin;)]1},${.9,,#0.4,.1,&2{%1.0p?{${.2d,:0^[01},${.3a,:1[01"
  "}c]1}n]1}.!0.0^_1[01}_1,.1,.8A8,'(y6:lambda),l3,'(y4:call),@(y5:pair*)"
  "[73}.2aY0?{.4,.3ac,.4,${:1,.6a,.6a,@(y10:xform-set!)[03}c,.4d,.4d,.4d,"
  ":2^[55}${${:1,.6a,t,@(y5:xform)[03},${.5a,:1[01},@(y16:binding-set-val"
  "!)[02}.4,.4,.4d,.4d,.4d,:2^[55}.!0.0^_1[55}@!(y12:xform-labels)",

  0,
  "&0{%3${.2,@(y3:id?)[01}?{${.4,.4,f,@(y5:xform)[03},${.3,@(y7:id->sym)["
  "01},'(y6:define),l3]3}'(s29:define of non-identifier form),'(y9:transf"
  "orm),@(y5:error)[32}@!(y12:xform-define)",

  0,
  "&0{%3${.2,@(y3:id?)[01}?{${.4,.4,t,@(y5:xform)[03},${.3,@(y7:id->sym)["
  "01},'(y13:define-syntax),l3]3}'(s36:define-syntax of non-identifier fo"
  "rm),'(y9:transform),@(y5:error)[32}@!(y19:xform-define-syntax)",

  0,
  "${&0{%2.0,'(y3:...),@(y5:error)[22},'(y3:...),@(y12:make-binding)[02}@"
  "!(y30:denotation-of-default-ellipsis)",

  0,
  "@(y30:denotation-of-default-ellipsis),${'(y4:body),'(y4:body),@(y12:ma"
  "ke-binding)[02},${'(y7:lambda*),'(y7:lambda*),@(y12:make-binding)[02},"
  "${'(y6:lambda),'(y6:lambda),@(y12:make-binding)[02},${'(y2:if),'(y2:if"
  "),@(y12:make-binding)[02},${'(y5:begin),'(y5:begin),@(y12:make-binding"
  ")[02},${'(y4:set!),'(y4:set!),@(y12:make-binding)[02},${'(y5:quote),'("
  "y5:quote),@(y12:make-binding)[02},${'(y13:define-syntax),'(y13:define-"
  "syntax),@(y12:make-binding)[02},${'(y6:define),'(y6:define),@(y12:make"
  "-binding)[02},${'(y6:syntax),'(y6:syntax),@(y12:make-binding)[02},l(i1"
  "1)@!(y14:*transformers*)",

  0,
  "&0{%1${@(y14:*transformers*),.3,@(y16:find-top-binding)[02},${.2,@(y8:"
  "binding?)[01}?{${.2,@(y11:binding-val)[01},.0p?{'(y12:syntax-rules),.1"
  "aq}{f}?{${${.4,t,@(y9:transform)[02},.4,@(y16:binding-set-val!)[02}}_1"
  ".0]2}.1Y0?{${.3,'(y3:ref),l2,.4,@(y12:make-binding)[02},@(y14:*transfo"
  "rmers*),.1c@!(y14:*transformers*).0]3}.1,@(y7:old-den)[21}@!(y19:top-t"
  "ransformer-env)",

  0,
  "&0{%2.1,${.3,@(y19:top-transformer-env)[01},@(y16:binding-set-val!)[22"
  "}@!(y20:install-transformer!)",

  0,
  "&0{%4${.5,.5,.5,@(y19:top-transformer-env),@(y13:syntax-rules*)[04},.1"
  ",@(y20:install-transformer!)[42}@!(y26:install-transformer-rules!)",

  0,
  "&0{%!2${f,@(y6:gensym)[01}.0u?{@(y19:top-transformer-env)}{.0a},.3,.3,"
  "@(y5:xform)[33}@!(y9:transform)",

  0,
  "&0{%4,,,,,,,#0#1#2#3#4#5#6.9,&1{%1:0,.1A0]1}.!0.0,&1{%1${.2,:0^[01}~]1"
  "}.!1.3,&1{%1.0p?{.0a,:0^[11}f]1}.!2.7,.9,&2{%1:0?{:0,.1q]1}${.2,@(y3:i"
  "d?)[01}?{@(y30:denotation-of-default-ellipsis),${.3,:1[01}q]1}f]1}.!3."
  "2,&1{%3n,.2,.2,,#0:0,.1,.8,&3{%3${.2,@(y3:id?)[01}?{.1?{${.2,:0[01}}{f"
  "}?{.2,.1c]3}.2]3}.0V0?{.2,.2,${.4,@(y12:vector->list)[01},:1^[33}.0p?{"
  "${.2d,:2^[01}?{${.4,.4,.4dd,:1^[03},t,.2a,:1^[33}${.4,.4,.4d,:1^[03},."
  "2,.2a,:1^[33}.2]3}.!0.0^_1[33}.!4.4,.2,.4,.3,.(i11),&5{%3.1,.1,.4,:0,:"
  "1,:2,:3,:4,&8{%1,#0.1,&1{%0f,:0[01}.!0n,:7,:6,,#0.4,.1,:0,:1,:2,:3,:4,"
  ":5,&8{%3,#0:7,.4,&2{%1.0?{:0]1}:1^[10}.!0${.3,@(y3:id?)[01}?{${.3,:2^["
  "01}?{${.4,@(y3:id?)[01}?{${.3,:1[01},${.5,:0[01}q}{f},.1^[41}.3,.3,.3c"
  "c]4}.1V0?{.2V0,.0?{.0}{${:7^[00}}_1.3,${.5,@(y12:vector->list)[01},${."
  "5,@(y12:vector->list)[01},:6^[43}.1p~?{.2,.2e,.1^[41}${.3d,:3^[01}?{.1"
  "ddg,.3L0?{.3g}{${:7^[00}},.1,.1I-,.0<0?{${:7^[00}}.0,.6A6,.3,.7A8A6A8,"
  "${:4^,t,.(i10)a,:5^[03},,#0.8,:6,&2{%1@(y13:%25residual-cdr),${n,.4,:1"
  "a,:0^[03},,#0.2,.1,&2{%1.0p?{${.2d,:0^[01},${.3a,:1[01}c]1}n]1}.!0.0^_"
  "1[21}.!0${.(i12),.6,.(i12)dd,:6^[03},${.3^,${.8,,#0.4,.1,&2{%1.0p?{${."
  "2d,:0^[01},${.3a,:1[01}c]1}n]1}.!0.0^_1[01}_1,.5c,@(y14:%25residual-li"
  "st)c,@(y13:%25residual-map),@(y5:%25appl)[02}L6](i11)}.2p?{${.5,.5d,.5"
  "d,:6^[03},.3a,.3a,:6^[43}:7^[40}.!0.0^_1[23},@(y4:%25ccc)[31}.!5.7,.2,"
  ".6,.5,&4{%3,,,#0#1#2:3,&1{%1${${.4,:0[01},@(y6:new-id)[01},.1c]1},${${"
  ".(i10),&1{%1:0,.1A3~]1},t,.(i11),:1^[03},,#0.4,.1,&2{%1.0p?{${.2d,:0^["
  "01},${.3a,:1[01}c]1}n]1}.!0.0^_1[01}_1.!0${:2^,f,.7,:1^[03}.!1.1,:1,&2"
  "{%1:1,&1{%1:0^,.1A0]1},t,.2,:0^[13}.!2.5,.5,,#0.8,.4,.2,.8,:0,&5{%2.0,"
  ",#0:0,:1,:2,.6,.4,:3,:4,&7{%1${.2,@(y3:id?)[01}?{:3,.1A3,.0?{.0}{:0,.2"
  "A3,.0?{.0}{:1^,.3A3}_1}_1d]1}.0V0?{${${.4,@(y12:vector->list)[01},:2^["
  "01}X1]1}.0p?{${.2d,:6^[01}?{${.2a,:5^[01},,,#0#1:3,&1{%1:0,.1A3d]1}.!0"
  ".2,.4,:4,&3{%!0${.2,:2,@(y14:%25residual-cons),@(y13:%25residual-map)["
  "03},:1a,:0^[12}.!1.0^,${.5,,#0.4,.1,&2{%1.0p?{${.2d,:0^[01},${.3a,:1[0"
  "1}c]1}n]1}.!0.0^_1[01}_1,${.6dd,:2^[01},${.3,.6^c,@(y13:%25residual-ma"
  "p),@(y5:%25appl)[02}L6]5}${.2d,:2^[01},${.3a,:2^[01}c]1}.0]1}.!0.0^_1["
  "21}.!0.0^_1[62}.!6.(i10),.7,.7,&3{%2:2,,#0:0,.3,.5,:1,.4,&5{%1.0u?{${:"
  "3,'(s14:invalid syntax),'(y9:transform),@(y5:error)[03}}.0a,.0a,.1da,$"
  "{:2,:3,.5,:4^[03},.0?{.0,.0,.3,.5,:1^[63}.4d,:0^[51}.!0.0^_1[21}](i11)"
  "}@!(y13:syntax-rules*)",

  0,
  "${&0{%2,#0${${'(y6:syntax),'(y6:syntax),@(y12:make-binding)[02},@(y6:n"
  "ew-id)[01}.!0${.3da,@(y3:id?)[01}?{${.3ddd,.4dda,.5da,.7,@(y13:syntax-"
  "rules*)[04},.1^,l2]3}${.3dd,.4da,f,.7,@(y13:syntax-rules*)[04},.1^,l2]"
  "3},'(y12:syntax-rules),@(y20:install-transformer!)[02}",

  0,
  "${${@(y19:top-transformer-env),'(y6:define),'(y6:define),@(y11:add-bin"
  "ding)[03},${'(l2:l2:py1:_;ppy4:name;y4:args;;y5:forms;;;l3:y6:define;y"
  "4:name;py6:lambda;py4:args;y5:forms;;;;;l2:l3:y1:_;y4:name;y3:exp;;l3:"
  "y6:define;y4:name;y3:exp;;;),n,f,.5,@(y13:syntax-rules*)[04}_1,'(y6:de"
  "fine),@(y20:install-transformer!)[02}",

  "install-sr-transformer!",
  "l4:y12:syntax-rules;l2:y5:quote;y12:syntax-rules;;l2:l3:y1:_;l2:y5:quo"
  "te;y4:name;;py12:syntax-rules;pl2:y3:lit;y3:...;;y5:rules;;;;l5:y26:in"
  "stall-transformer-rules!;l2:y5:quote;y4:name;;f;l2:y5:quote;l2:y3:lit;"
  "y3:...;;;l2:y5:quote;y5:rules;;;;l2:l3:y1:_;l2:y5:quote;y4:name;;py12:"
  "syntax-rules;py8:ellipsis;pl2:y3:lit;y3:...;;y5:rules;;;;;l5:y26:insta"
  "ll-transformer-rules!;l2:y5:quote;y4:name;;l2:y5:quote;y8:ellipsis;;l2"
  ":y5:quote;l2:y3:lit;y3:...;;;l2:y5:quote;y5:rules;;;;",

  0,
  "${'(l1:l2:py1:_;pl2:l2:y3:key;y5:trans;;y3:...;;y5:forms;;;py4:body;pl"
  "3:y13:define-syntax;y3:key;y5:trans;;py3:...;y5:forms;;;;;),n,f,'(y13:"
  "letrec-syntax),@(y26:install-transformer-rules!)[04}",

  0,
  "${'(l2:l2:py1:_;pn;y5:forms;;;py4:body;y5:forms;;;l2:py1:_;ppl2:y3:key"
  ";y5:trans;;y8:bindings;;y5:forms;;;l3:y13:letrec-syntax;l1:l2:y4:temp;"
  "y5:trans;;;l3:y10:let-syntax;y8:bindings;py13:letrec-syntax;pl1:l2:y3:"
  "key;y4:temp;;;y5:forms;;;;;;),n,f,'(y10:let-syntax),@(y26:install-tran"
  "sformer-rules!)[04}",

  0,
  "${'(l1:l2:py1:_;pl2:l2:y3:var;y4:init;;y3:...;;y5:forms;;;py4:body;pl3"
  ":y6:define;y3:var;y4:init;;py3:...;y5:forms;;;;;),n,f,'(y6:letrec),@(y"
  "26:install-transformer-rules!)[04}",

  0,
  "${'(l2:l2:py1:_;pl2:l2:y3:var;y4:init;;y3:...;;y5:forms;;;l3:py6:lambd"
  "a;pl2:y3:var;y3:...;;y5:forms;;;y4:init;y3:...;;;l2:py1:_;py4:name;pl2"
  ":l2:y3:var;y4:init;;y3:...;;y5:forms;;;;l3:l3:y6:letrec;l1:l2:y4:name;"
  "py6:lambda;pl2:y3:var;y3:...;;y5:forms;;;;;y4:name;;y4:init;y3:...;;;)"
  ",n,f,'(y3:let),@(y26:install-transformer-rules!)[04}",

  0,
  "${'(l2:l2:py1:_;pn;y5:forms;;;py4:body;y5:forms;;;l2:py1:_;ppy5:first;"
  "y4:more;;y5:forms;;;l3:y3:let;l1:y5:first;;py4:let*;py4:more;y5:forms;"
  ";;;;),n,f,'(y4:let*),@(y26:install-transformer-rules!)[04}",

  0,
  "${'(l3:l2:l1:y1:_;;t;;l2:l2:y1:_;y4:test;;y4:test;;l2:py1:_;py4:test;y"
  "5:tests;;;l4:y2:if;y4:test;py3:and;y5:tests;;f;;;),n,f,'(y3:and),@(y26"
  ":install-transformer-rules!)[04}",

  0,
  "${'(l3:l2:l1:y1:_;;f;;l2:l2:y1:_;y4:test;;y4:test;;l2:py1:_;py4:test;y"
  "5:tests;;;l3:y3:let;l1:l2:y1:x;y4:test;;;l4:y2:if;y1:x;y1:x;py2:or;y5:"
  "tests;;;;;),n,f,'(y2:or),@(y26:install-transformer-rules!)[04}",

  0,
  "${'(l5:l2:l1:y1:_;;f;;l2:l2:y1:_;py4:else;y4:exps;;;py5:begin;y4:exps;"
  ";;l2:py1:_;pl1:y1:x;;y4:rest;;;l3:y2:or;y1:x;py4:cond;y4:rest;;;;l2:py"
  "1:_;pl3:y1:x;y2:=>;y4:proc;;y4:rest;;;l3:y3:let;l1:l2:y3:tmp;y1:x;;;py"
  "4:cond;pl2:y3:tmp;l2:y4:proc;y3:tmp;;;y4:rest;;;;;l2:py1:_;ppy1:x;y4:e"
  "xps;;y4:rest;;;l4:y2:if;y1:x;py5:begin;y4:exps;;py4:cond;y4:rest;;;;),"
  "'(l2:y4:else;y2:=>;),f,'(y4:cond),@(y26:install-transformer-rules!)[04"
  "}",

  0,
  "${'(l2:l2:l3:y1:_;y1:k;y4:else;;t;;l2:l3:y1:_;y1:k;y5:atoms;;l3:y4:mem"
  "v;y1:k;l2:y5:quote;y5:atoms;;;;),'(l1:y4:else;),f,'(y9:case-test),@(y2"
  "6:install-transformer-rules!)[04}",

  0,
  "${'(l1:l2:l4:y1:_;y1:x;py4:test;y5:exprs;;y3:...;;l3:y3:let;l1:l2:y3:k"
  "ey;y1:x;;;l3:y4:cond;pl3:y9:case-test;y3:key;y4:test;;y5:exprs;;y3:..."
  ";;;;),n,f,'(y4:case),@(y26:install-transformer-rules!)[04}",

  0,
  "${'(l1:l2:l5:y1:_;l2:py3:var;py4:init;y4:step;;;y3:...;;y6:ending;y4:e"
  "xpr;y3:...;;l4:y3:let;y4:loop;l2:l2:y3:var;y4:init;;y3:...;;l3:y4:cond"
  ";y6:ending;l4:y4:else;y4:expr;y3:...;l3:y4:loop;py5:begin;py3:var;y4:s"
  "tep;;;y3:...;;;;;;),n,f,'(y2:do),@(y26:install-transformer-rules!)[04}",

  0,
  "${'(l8:l2:l2:y1:_;l2:y7:unquote;y1:x;;;y1:x;;l2:l2:y1:_;pl2:y16:unquot"
  "e-splicing;y1:x;;y1:y;;;l3:y6:append;y1:x;l2:y10:quasiquote;y1:y;;;;l2"
  ":py1:_;pl2:y10:quasiquote;y1:x;;y1:d;;;l3:y4:cons;l2:y5:quote;y10:quas"
  "iquote;;l3:y10:quasiquote;l1:y1:x;;y1:d;;;;l2:l3:y1:_;l2:y7:unquote;y1"
  ":x;;y1:d;;l3:y4:cons;l2:y5:quote;y7:unquote;;py10:quasiquote;pl1:y1:x;"
  ";y1:d;;;;;l2:l3:y1:_;l2:y16:unquote-splicing;y1:x;;y1:d;;l3:y4:cons;l2"
  ":y5:quote;y16:unquote-splicing;;py10:quasiquote;pl1:y1:x;;y1:d;;;;;l2:"
  "py1:_;ppy1:x;y1:y;;y1:d;;;l3:y4:cons;py10:quasiquote;py1:x;y1:d;;;py10"
  ":quasiquote;py1:y;y1:d;;;;;l2:py1:_;pv2:y1:x;y3:...;;y1:d;;;l2:y12:lis"
  "t->vector;py10:quasiquote;pl2:y1:x;y3:...;;y1:d;;;;;l2:py1:_;py1:x;y1:"
  "d;;;l2:y5:quote;y1:x;;;),'(l3:y7:unquote;y16:unquote-splicing;y10:quas"
  "iquote;),f,'(y10:quasiquote),@(y26:install-transformer-rules!)[04}",

  0,
  "${'(l1:l2:l2:y1:_;y3:exp;;l2:y12:make-delayed;l3:y6:lambda;n;y3:exp;;;"
  ";),n,f,'(y5:delay),@(y26:install-transformer-rules!)[04}",

  0,
  "${'(l1:l2:py1:_;py4:test;y4:rest;;;l3:y2:if;y4:test;py5:begin;y4:rest;"
  ";;;),n,f,'(y4:when),@(y26:install-transformer-rules!)[04}",

  0,
  "${'(l1:l2:py1:_;py4:test;y4:rest;;;l3:y2:if;l2:y3:not;y4:test;;py5:beg"
  "in;y4:rest;;;;),n,f,'(y6:unless),@(y26:install-transformer-rules!)[04}",

  0,
  "${'(l1:l2:l3:y1:_;py4:args;y4:body;;y3:...;;l3:y7:lambda*;l2:y4:args;p"
  "y6:lambda;py4:args;y4:body;;;;y3:...;;;),n,f,'(y11:case-lambda),@(y26:"
  "install-transformer-rules!)[04}",

  0, 0
};
