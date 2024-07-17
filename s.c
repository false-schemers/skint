/* s.c -- generated via skint -c s.scm */
#include "s.h"
#include "n.h"

extern int is_tty_port(obj o)
{
  FILE *fp = NULL;
  if ((cxtype_t*)iportvt(o) == IPORT_FILE_NTAG) fp = (FILE*)iportdata(o);
  else if ((cxtype_t*)oportvt(o) == OPORT_FILE_NTAG) fp = (FILE*)oportdata(o); 
  if (!fp) return 0;
  return isatty(fileno(fp));
}

#ifdef WIN32
int dirsep = '\\';
#else
int dirsep = '/';
#endif

extern char *argv_ref(int idx)
{
  char **pv = cxg_argv;
  /* be careful with indexing! */
  if (idx < 0) return NULL;
  while (idx-- > 0) if (*pv++ == NULL) return NULL;
  return *pv;
}

#if defined(WIN32)
#define cxg_envv _environ
#elif defined(__linux) || defined(__APPLE__)
#define cxg_envv environ
#else /* add more systems? */
char **cxg_envv = { NULL };
#endif

extern char *envv_ref(int idx)
{
  char **pv = cxg_envv;
  /* be careful with indexing! */
  if (idx < 0) return NULL;
  while (idx-- > 0) if (*pv++ == NULL) return NULL;
  return *pv;
}

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
  "l5:y12:syntax-rules;n;l2:py1:_;pn;y5:forms;;;py4:body;y5:forms;;;l2:py"
  "1:_;pl2:l2:y3:var;y4:init;;y3:...;;y5:forms;;;l3:py6:lambda;pl2:y3:var"
  ";y3:...;;y5:forms;;;y4:init;y3:...;;;l2:py1:_;py4:name;pl2:l2:y3:var;y"
  "4:init;;y3:...;;y5:forms;;;;l3:l3:y6:letrec;l1:l2:y4:name;py6:lambda;p"
  "l2:y3:var;y3:...;;y5:forms;;;;;y4:name;;y4:init;y3:...;;;",

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

  "C", 0,
  "&0{%2${.2,.4[01},#0.2,.1,&2{%2.1?{.0:!0]2}.0,:1[21}%x,.3,.2,&2{%1${.2,"
  ":1[01}:!0]1}%x,.2,&1{%0:0^]0}%x,&3{|00|11|22%%}]3}%x,&0{%1#0.0,&1{%2.1"
  "?{.0:!0]2}.0]2}%x,.1,&1{%1.0:!0]1}%x,.2,&1{%0:0^]0}%x,&3{|00|11|22%%}]"
  "1}%x,&2{|10|21%%}@!(y14:make-parameter)",

  "S", "%parameterize-loop",
  "l4:y12:syntax-rules;n;l2:l4:y1:_;l2:l5:y5:param;y5:value;y1:p;y3:old;y"
  "3:new;;y3:...;;n;y4:body;;l3:y3:let;l2:l2:y1:p;y5:param;;y3:...;;l3:y3"
  ":let;l4:l2:y3:old;l1:y1:p;;;y3:...;l2:y3:new;l3:y1:p;y5:value;f;;;y3:."
  "..;;l4:y12:dynamic-wind;l4:y6:lambda;n;l3:y1:p;y3:new;t;;y3:...;;py6:l"
  "ambda;pn;y4:body;;;l4:y6:lambda;n;l3:y1:p;y3:old;t;;y3:...;;;;;;l2:l4:"
  "y1:_;y4:args;pl2:y5:param;y5:value;;y4:rest;;y4:body;;l4:y18:%25parame"
  "terize-loop;pl5:y5:param;y5:value;y1:p;y3:old;y3:new;;y4:args;;y4:rest"
  ";y4:body;;;",

  "S", "parameterize",
  "l3:y12:syntax-rules;n;l2:py1:_;pl2:l2:y5:param;y5:value;;y3:...;;y4:bo"
  "dy;;;l4:y18:%25parameterize-loop;n;l2:l2:y5:param;y5:value;;y3:...;;y4"
  ":body;;;",

  "P", "new-record-type",
  "%2'(l1:s6:rtd://;),.2,.2c,,#0.0,&1{%2.0u?{${.3A8,@(y14:%25string-appen"
  "d),@(y13:apply-to-list)[02}X5]2}.0du?{.1,.1aX4c,.1d,:0^[22}.1,.1aX4c,'"
  "(s1::)c,.1d,:0^[22}.!0.0^_1[22",

  "S", "%id-eq??",
  "l3:y12:syntax-rules;n;l2:l5:y1:_;y2:id;y1:b;y2:kt;y2:kf;;l3:l3:y13:syn"
  "tax-lambda;l2:y2:id;y2:ok;;l2:l3:y12:syntax-rules;n;l2:l2:y1:_;y1:b;;l"
  "1:y2:id;;;;y2:ok;;;l3:y12:syntax-rules;n;l2:l1:y1:_;;y2:kf;;;l3:y12:sy"
  "ntax-rules;n;l2:l1:y1:_;;y2:kt;;;;;",

  "S", "%id-assq??",
  "l4:y12:syntax-rules;n;l2:l5:y1:_;y2:id;n;y2:kt;y2:kf;;y2:kf;;l2:l5:y1:"
  "_;y2:id;ppy3:id0;y2:r0;;y4:idr*;;y2:kt;y2:kf;;l5:y8:%25id-eq??;y2:id;y"
  "3:id0;py2:kt;y2:r0;;l5:y10:%25id-assq??;y2:id;y4:idr*;y2:kt;y2:kf;;;;",

  "S", "%drt-init",
  "l4:y12:syntax-rules;n;l2:l5:y1:_;y1:r;n;y3:fi*;l2:y1:x;y3:...;;;l4:y5:"
  "begin;y1:x;y3:...;y1:r;;;l2:l5:y1:_;y1:r;py3:id0;y3:id*;;y3:fi*;l2:y1:"
  "x;y3:...;;;l5:y10:%25id-assq??;y3:id0;y3:fi*;l3:y12:syntax-rules;n;l2:"
  "l2:y1:_;y2:i0;;l5:y9:%25drt-init;y1:r;y3:id*;y3:fi*;l3:y1:x;y3:...;l4:"
  "y11:record-set!;y1:r;y2:i0;y3:id0;;;;;;l3:y12:syntax-error;s52:id in d"
  "efine-record-type constructor is not a field:;y3:id0;;;;",

  "S", "%drt-unroll",
  "l5:y12:syntax-rules;n;l2:l8:y1:_;y3:rtn;l3:y5:consn;y2:id;y3:...;;y5:p"
  "redn;n;l2:l2:y1:f;y1:i;;y3:...;;l2:l2:y1:a;y2:ia;;y3:...;;l2:l2:y1:m;y"
  "2:im;;y3:...;;;l8:y5:begin;l3:y6:define;y3:rtn;l3:y15:new-record-type;"
  "l2:y5:quote;y3:rtn;;l2:y5:quote;l2:y1:f;y3:...;;;;;l3:y6:define;y5:con"
  "sn;l3:y6:lambda;l2:y2:id;y3:...;;l3:y3:let;l1:l2:y1:r;l3:y11:make-reco"
  "rd;y3:rtn;l2:y13:syntax-length;l2:y1:f;y3:...;;;;;;l5:y9:%25drt-init;y"
  "1:r;l2:y2:id;y3:...;;l2:l2:y1:f;y1:i;;y3:...;;n;;;;;l3:y6:define;y5:pr"
  "edn;l3:y6:lambda;l1:y3:obj;;l3:y7:record?;y3:obj;y3:rtn;;;;l3:y6:defin"
  "e;y1:a;l3:y6:lambda;l1:y3:obj;;l3:y10:record-ref;y3:obj;y2:ia;;;;y3:.."
  ".;l3:y6:define;y1:m;l3:y6:lambda;l2:y3:obj;y3:val;;l4:y11:record-set!;"
  "y3:obj;y2:im;y3:val;;;;y3:...;;;l2:l8:y1:_;y3:rtn;y3:cf*;y5:predn;l3:l"
  "2:y2:fn;y4:accn;;y3:fam;y3:...;;l2:y2:fi;y3:...;;l2:y2:ai;y3:...;;l2:y"
  "2:mi;y3:...;;;l8:y11:%25drt-unroll;y3:rtn;y3:cf*;y5:predn;l2:y3:fam;y3"
  ":...;;l3:y2:fi;y3:...;l2:y2:fn;l2:y13:syntax-length;l2:y2:fi;y3:...;;;"
  ";;l3:y2:ai;y3:...;l2:y4:accn;l2:y13:syntax-length;l2:y2:fi;y3:...;;;;;"
  "l2:y2:mi;y3:...;;;;l2:l8:y1:_;y3:rtn;y3:cf*;y5:predn;l3:l3:y2:fn;y4:ac"
  "cn;y4:modn;;y3:fam;y3:...;;l2:y2:fi;y3:...;;l2:y2:ai;y3:...;;l2:y2:mi;"
  "y3:...;;;l8:y11:%25drt-unroll;y3:rtn;y3:cf*;y5:predn;l2:y3:fam;y3:...;"
  ";l3:y2:fi;y3:...;l2:y2:fn;l2:y13:syntax-length;l2:y2:fi;y3:...;;;;;l3:"
  "y2:ai;y3:...;l2:y4:accn;l2:y13:syntax-length;l2:y2:fi;y3:...;;;;;l3:y2"
  ":mi;y3:...;l2:y4:modn;l2:y13:syntax-length;l2:y2:fi;y3:...;;;;;;;",

  "S", "define-record-type",
  "l3:y12:syntax-rules;n;l2:l6:y1:_;y3:rtn;l3:y5:consn;y2:id;y3:...;;y5:p"
  "redn;py2:fn;y2:am;;y3:...;;l8:y11:%25drt-unroll;y3:rtn;l3:y5:consn;y2:"
  "id;y3:...;;y5:predn;l2:py2:fn;y2:am;;y3:...;;n;n;n;;;",

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

  "P", "abort",
  "%0tZ8]0",

  "P", "reset",
  "%0'1Z9]0",

  "P", "set-reset-handler!",
  "%1.0@!(y5:reset)]1",

  "P", "print-error-message",
  "%3,,,#0#1#2.1,&1{%2.0p?{.0a~?{${.3,'(s2:: ),@(y12:write-string)[02}.1,"
  ".1d,:0^[22}.0aY0?{${.3,'(s4: in ),@(y12:write-string)[02}.1,.1aW5${.3,"
  "'(s2:: ),@(y12:write-string)[02}.1,.1d,:0^[22}${.3,'(s2:: ),@(y12:writ"
  "e-string)[02}.1,.1,:0^[22}]2}.!0.2,&1{%2.0p?{.0aS0?{.1,.1aW4.1,.1d,:0^"
  "[22}.1,.1,:0^[22}]2}.!1.2,&1{%2.0p?{.1,'(c )W0.1,.1aW5.1,.1d,:0^[22}]2"
  "}.!2.3S0,.0?{.0}{.4Y0}_1?{${.7,.6,@(y12:write-string)[02}}{${.7,'(s5:E"
  "rror),@(y12:write-string)[02}}${.7,.7,.4^[02}.5W6]6",

  "P", "simple-error",
  "%!0Pe,.0W6${.2,.4,'(s5:Error),@(y19:print-error-message)[03}@(y5:reset"
  ")[20",

  "P", "assertion-violation",
  "%!0Pe,.0W6${.2,.4,'(s19:Assertion violation),@(y19:print-error-message"
  ")[03}@(y5:reset)[20",

  "C", 0,
  "${'(l3:y4:kind;y7:message;y9:irritants;),'(y14:<error-object>),@(y15:n"
  "ew-record-type)[02}@!(y14:<error-object>)",

  "P", "error-object",
  "%3f,'3,@(y14:<error-object>)O2,.1,'0,.2O5.2,'1,.2O5.3,'2,.2O5.0]4",

  "P", "error-object?",
  "%1@(y14:<error-object>),.1O0]1",

  "P", "error-object-kind",
  "%1'0,.1O4]1",

  "P", "error-object-message",
  "%1'1,.1O4]1",

  "P", "error-object-irritants",
  "%1'2,.1O4]1",

  "P", "error",
  "%!1${.2,.4,f,@(y12:error-object)[03},@(y5:raise)[21",

  "C", 0,
  "${,#0&0{%1${.2,@(y13:error-object?)[01}?{${.2,@(y22:error-object-irrit"
  "ants)[01},${.3,@(y20:error-object-message)[01}c,${.3,@(y17:error-objec"
  "t-kind)[01}c,@(y12:simple-error),@(y13:apply-to-list)[12}.0,'(s19:unha"
  "ndled exception),f,@(y12:simple-error)[13}%x,.1,&1{%0:0^]0}%x,&2{|00|1"
  "1%%}.!0.0^_1,@(y14:make-parameter)[01}@!(y25:current-exception-handler"
  ")",

  "P", "with-exception-handler",
  "%2${@(y25:current-exception-handler)[00},@(y25:current-exception-handl"
  "er),${f,.5,&1{%1.0,:0[11}%x,.5,&1{%0:0]0}%x,&2{|00|11%%},.4[02},${.3[0"
  "0},.0,.3,&2{%0t,:1,:0[02},.6,&1{%0:0[00},.3,.5,&2{%0t,:1,:0[02},@(y12:"
  "dynamic-wind)[63",

  "P", "raise",
  "%1${@(y25:current-exception-handler)[00},@(y25:current-exception-handl"
  "er),${f,${.6[00},.4[02},${.3[00},.0,.3,&2{%0t,:1,:0[02},.4,.6,&2{%0${:"
  "0,:1[01}${:0,:1,l2,'(s26:exception handler returned),'(y5:raise),@(y12"
  ":error-object)[03},@(y5:raise)[01},.3,.5,&2{%0t,:1,:0[02},@(y12:dynami"
  "c-wind)[53",

  "P", "raise-continuable",
  "%1${@(y25:current-exception-handler)[00},@(y25:current-exception-handl"
  "er),${f,${.6[00},.4[02},${.3[00},.0,.3,&2{%0t,:1,:0[02},.5,.5,&2{%0:1,"
  ":0[01},.3,.5,&2{%0t,:1,:0[02},@(y12:dynamic-wind)[53",

  "S", "%guard-aux",
  "l9:y12:syntax-rules;l2:y4:else;y2:=>;;l2:l3:y1:_;y7:reraise;l4:y4:else"
  ";y7:result1;y7:result2;y3:...;;;l4:y5:begin;y7:result1;y7:result2;y3:."
  "..;;;l2:l3:y1:_;y7:reraise;l3:y4:test;y2:=>;y6:result;;;l3:y3:let;l1:l"
  "2:y4:temp;y4:test;;;l4:y2:if;y4:temp;l2:y6:result;y4:temp;;y7:reraise;"
  ";;;l2:l6:y1:_;y7:reraise;l3:y4:test;y2:=>;y6:result;;y7:clause1;y7:cla"
  "use2;y3:...;;l3:y3:let;l1:l2:y4:temp;y4:test;;;l4:y2:if;y4:temp;l2:y6:"
  "result;y4:temp;;l5:y10:%25guard-aux;y7:reraise;y7:clause1;y7:clause2;y"
  "3:...;;;;;l2:l3:y1:_;y7:reraise;l1:y4:test;;;l3:y2:or;y4:test;y7:rerai"
  "se;;;l2:l6:y1:_;y7:reraise;l1:y4:test;;y7:clause1;y7:clause2;y3:...;;l"
  "3:y3:let;l1:l2:y4:temp;y4:test;;;l4:y2:if;y4:temp;y4:temp;l5:y10:%25gu"
  "ard-aux;y7:reraise;y7:clause1;y7:clause2;y3:...;;;;;l2:l3:y1:_;y7:rera"
  "ise;l4:y4:test;y7:result1;y7:result2;y3:...;;;l4:y2:if;y4:test;l4:y5:b"
  "egin;y7:result1;y7:result2;y3:...;;y7:reraise;;;l2:l6:y1:_;y7:reraise;"
  "l4:y4:test;y7:result1;y7:result2;y3:...;;y7:clause1;y7:clause2;y3:...;"
  ";l4:y2:if;y4:test;l4:y5:begin;y7:result1;y7:result2;y3:...;;l5:y10:%25"
  "guard-aux;y7:reraise;y7:clause1;y7:clause2;y3:...;;;;",

  "S", "guard",
  "l3:y12:syntax-rules;n;l2:l5:y5:guard;l3:y3:var;y6:clause;y3:...;;y2:e1"
  ";y2:e2;y3:...;;l1:l2:y7:call/cc;l3:y6:lambda;l1:y7:guard-k;;l3:y22:wit"
  "h-exception-handler;l3:y6:lambda;l1:y9:condition;;l1:l2:y7:call/cc;l3:"
  "y6:lambda;l1:y9:handler-k;;l2:y7:guard-k;l3:y6:lambda;n;l3:y3:let;l1:l"
  "2:y3:var;y9:condition;;;l4:y10:%25guard-aux;l2:y9:handler-k;l3:y6:lamb"
  "da;n;l2:y17:raise-continuable;y9:condition;;;;y6:clause;y3:...;;;;;;;;"
  ";l3:y6:lambda;n;l3:y16:call-with-values;l5:y6:lambda;n;y2:e1;y2:e2;y3:"
  "...;;l3:y6:lambda;y4:args;l2:y7:guard-k;l3:y6:lambda;n;l3:y5:apply;y6:"
  "values;y4:args;;;;;;;;;;;;",

  "P", "read-error",
  "%!1${.2,.4,'(y4:read),@(y12:error-object)[03},@(y5:raise)[21",

  "P", "read-error?",
  "%1${.2,@(y13:error-object?)[01}?{'(y4:read),${.3,@(y17:error-object-ki"
  "nd)[01}q]1}f]1",

  "P", "file-error",
  "%!1${.2,.4,'(y4:file),@(y12:error-object)[03},@(y5:raise)[21",

  "P", "file-error?",
  "%1${.2,@(y13:error-object?)[01}?{'(y4:file),${.3,@(y17:error-object-ki"
  "nd)[01}q]1}f]1",

  "P", "port?",
  "%1.0P00,.0?{.0]2}.1P01]2",

  "C", 0,
  "@(y5:port?)@!(y13:textual-port?)",

  "C", 0,
  "@(y5:port?)@!(y12:binary-port?)",

  "C", 0,
  "&0{%2.1?{.0Psi]2}.0]2}%x,&0{%1.0Psi]1}%x,&0{%0Pi]0}%x,&3{|00|11|22%%}@"
  "!(y29:%25current-input-port-parameter)",

  "S", "current-input-port",
  "l6:y12:syntax-rules;n;l2:l1:y1:_;;l1:y19:%25current-input-port;;;l2:l2"
  ":y1:_;y1:p;;l2:y24:%25set-current-input-port!;y1:p;;;l2:py1:_;y1:r;;py"
  "29:%25current-input-port-parameter;y1:r;;;l2:y1:_;y29:%25current-input"
  "-port-parameter;;",

  "C", 0,
  "&0{%2.1?{.0Pso]2}.0]2}%x,&0{%1.0Pso]1}%x,&0{%0Po]0}%x,&3{|00|11|22%%}@"
  "!(y30:%25current-output-port-parameter)",

  "S", "current-output-port",
  "l6:y12:syntax-rules;n;l2:l1:y1:_;;l1:y20:%25current-output-port;;;l2:l"
  "2:y1:_;y1:p;;l2:y25:%25set-current-output-port!;y1:p;;;l2:py1:_;y1:r;;"
  "py30:%25current-output-port-parameter;y1:r;;;l2:y1:_;y30:%25current-ou"
  "tput-port-parameter;;",

  "C", 0,
  "&0{%2.1?{.0Pse]2}.0]2}%x,&0{%1.0Pse]1}%x,&0{%0Pe]0}%x,&3{|00|11|22%%}@"
  "!(y29:%25current-error-port-parameter)",

  "S", "current-error-port",
  "l6:y12:syntax-rules;n;l2:l1:y1:_;;l1:y19:%25current-error-port;;;l2:l2"
  ":y1:_;y1:p;;l2:y24:%25set-current-error-port!;y1:p;;;l2:py1:_;y1:r;;py"
  "29:%25current-error-port-parameter;y1:r;;;l2:y1:_;y29:%25current-error"
  "-port-parameter;;",

  "P", "open-input-file",
  "%1.0P40,.0?{.0]2}.1,'(s22:cannot open input file),@(y10:file-error)[22",

  "P", "open-output-file",
  "%1.0P41,.0?{.0]2}.1,'(s23:cannot open output file),@(y10:file-error)[2"
  "2",

  "P", "open-binary-input-file",
  "%1.0P42,.0?{.0]2}.1,'(s29:cannot open binary input file),@(y10:file-er"
  "ror)[22",

  "P", "open-binary-output-file",
  "%1.0P43,.0?{.0]2}.1,'(s30:cannot open binary output file),@(y10:file-e"
  "rror)[22",

  "P", "close-port",
  "%1.0P00?{.0P60}.0P01?{.0P61]1}]1",

  "P", "call-with-port",
  "%2.0,&1{%!0${:0,@(y10:close-port)[01}.0,@(y6:values),@(y13:apply-to-li"
  "st)[12},.1,.3,&2{%0:1,:0[01},@(y16:call-with-values)[22",

  "P", "call-with-input-file",
  "%2.1,${.3,@(y15:open-input-file)[01},@(y14:call-with-port)[22",

  "P", "call-with-output-file",
  "%2.1,${.3,@(y16:open-output-file)[01},@(y14:call-with-port)[22",

  "P", "with-input-from-port",
  "%2@(y29:%25current-input-port-parameter),${f,.4,.4[02},${.3[00},.0,.3,"
  "&2{%0t,:1,:0[02},.5,&1{%0:0[00},.3,.5,&2{%0t,:1,:0[02},@(y12:dynamic-w"
  "ind)[53",

  "P", "with-output-to-port",
  "%2@(y30:%25current-output-port-parameter),${f,.4,.4[02},${.3[00},.0,.3"
  ",&2{%0t,:1,:0[02},.5,&1{%0:0[00},.3,.5,&2{%0t,:1,:0[02},@(y12:dynamic-"
  "wind)[53",

  "P", "with-input-from-file",
  "%2.1,&1{%1:0,.1,@(y20:with-input-from-port)[12},.1,@(y20:call-with-inp"
  "ut-file)[22",

  "P", "with-output-to-file",
  "%2.1,&1{%1:0,.1,@(y19:with-output-to-port)[12},.1,@(y21:call-with-outp"
  "ut-file)[22",

  "P", "read-line",
  "%!0P51,.1u?{Pi}{.1a},t,,#0.2,.4,.2,&3{%1:2R0,.0R8,.0?{.0}{'(c%0a),.2C="
  "}_1?{.0R8?{.1}{f}?{.0]2}:1P90,:1P61.0]3}'(c%0d),.1C=?{f,:0^[21}:1,.1W0"
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
  "x,&0{%1Pi,.1S3,'0,.3,@(y15:read-substring!)[14}%x,&4{|10|21|32|43%%}@!"
  "(y12:read-string!)",

  "C", 0,
  "&0{%2.1,.1,@(y14:read-substring)[22}%x,&0{%1Pi,.1,@(y14:read-substring"
  ")[12}%x,&2{|10|21%%}@!(y11:read-string)",

  "P", "read-subbytevector!",
  "%4.1,,#0.5,.4,.4,.3,.8,&5{%1:0,.1I<!?{:3,.1I-]1}:4R3,.0R8?{:3,.2I=?{.0"
  "]2}:3,.2I-]2}.0,.2,:2B5'1,.2I+,:1^[21}.!0.0^_1[41",

  "P", "read-subbytevector",
  "%2'0,.1B2,${.4,.4,'0,.5,@(y19:read-subbytevector!)[04},.0R8?{.0]4}.2,."
  "1I=?{.1]4}.0,'0,.3B7]4",

  "C", 0,
  "&0{%4.1,.4,.4,.3,@(y19:read-subbytevector!)[44}%x,&0{%3.1,.1B3,.4,.3,@"
  "(y19:read-subbytevector!)[34}%x,&0{%2.1,.1B3,'0,.3,@(y19:read-subbytev"
  "ector!)[24}%x,&0{%1Pi,.1B3,'0,.3,@(y19:read-subbytevector!)[14}%x,&4{|"
  "10|21|32|43%%}@!(y16:read-bytevector!)",

  "C", 0,
  "&0{%2.1,.1,@(y18:read-subbytevector)[22}%x,&0{%1Pi,.1,@(y18:read-subby"
  "tevector)[12}%x,&2{|10|21%%}@!(y15:read-bytevector)",

  "P", "%read",
  "%2,,,,,,,,,,,,,,,,,,,,,,,#0#1#2#3#4#5#6#7#8#9#(i10)#(i11)#(i12)#(i13)#"
  "(i14)#(i15)#(i16)#(i17)#(i18)#(i19)#(i20)#(i21)#(i22).(i23)P78.!0n.!1&"
  "0{%1.0,&1{%0:0z]0}]1}.!2&0{%1.0K0]1}.!3.4,&1{%1.0K0?{${.2[00},:0^[11}."
  "0]1}.!4.5,.5,&2{%1.0p?{.0aK0?{${.2a,:0^[01},.1sa}{${.2a,:1^[01}}.0dK0?"
  "{${.2d,:0^[01},.1sd]1}.0d,:1^[11}.0V0?{'0,,#0.2,:0,:1,.3,&4{%1:3V3,.1I"
  "<?{.0,:3V4,.0K0?{${.2,:2^[01},.2,:3V5}{${.2,:1^[01}}_1'1,.1I+,:0^[11}]"
  "1}.!0.0^_1[11}.0Y2?{.0zK0?{${.2z,:1^[01},.1sz]1}.0z,:1^[11}f]1}.!5.5,&"
  "1{%1${.2,:0^[01}.0]1}.!6f.!7f.!8f.!9f.!(i10)'(y12:reader-token),l1,.0."
  "!8'(s17:right parenthesis),.1c.!9'(s13:right bracket),.1c.!(i10)'(s5:%"
  "22 . %22),.1c.!(i11)_1.7,&1{%1.0p?{:0^,.1aq]1}f]1}.!(i11)&0{%1'(s80:AB"
  "CDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz!$%25&*/:<=>?^_~0123"
  "456789+-.@),.1S8]1}.!(i12)&0{%1.0X8,'(i48),.1<!?{'(i57),.1>!}{f},.0?{."
  "0]3}'(i65),.2<!?{'(i70),.2>!}{f},.0?{.0]4}'(i97),.3<!?{'(i102),.3>!]4}"
  "f]4}.!(i13)&0{%1.0C1,.0?{.0]2}'(c)),.2C=,.0?{.0]3}'(c(),.3C=,.0?{.0]4}"
  "'(c]),.4C=,.0?{.0]5}'(c[),.5C=,.0?{.0]6}'(c%22),.6C=,.0?{.0]7}'(c;),.7"
  "C=]7}.!(i14).(i16),.(i12),&2{%1${.2,:1^[01},.0R8?{.1,'(y5:port:),'(s22"
  ":unexpected end of file),@(y10:read-error)[23}${.2,:0^[01}?{.1,'(y5:po"
  "rt:),.2d,'(s17:unexpected token:),@(y10:read-error)[24}.0]2}.!(i15).9,"
  ".(i13),.(i21),.3,.(i26),.(i21),.(i14),.(i24),.(i26),.(i29),.(i24),.(i1"
  "4),.(i27),.(i15),.(i15),.(i39),&(i16){%1.0R0,.0R8?{.0]2}.0C1?{.1,:(i10"
  ")^[21}'(c(),.1C=?{t,:9^,.3,.3,:8^[24}'(c)),.1C=?{:9^]2}'(c[),.1C=?{t,:"
  "(i15)^,.3,.3,:8^[24}'(c]),.1C=?{:(i15)^]2}'(c'),.1C=?{${.3,:3^[01},'(y"
  "5:quote),l2]2}'(c`),.1C=?{${.3,:3^[01},'(y10:quasiquote),l2]2}${.2,:(i"
  "14)^[01}?{.1,.1,:(i11)^[22}'(c;),.1C=?{${.3R0,,#0.5,.1,&2{%1.0R8,.0?{."
  "0]2}'(c%0a),.2C=,.0?{.0]3}:1R0,:0^[31}.!0.0^_1[01}.1,:(i10)^[21}'(c,),"
  ".1C=?{.1R1,.0R8?{.2,'(y5:port:),'(s19:end of file after ,),@(y10:read-"
  "error)[33}'(c@),.1C=?{.2R0${.4,:3^[01},'(y16:unquote-splicing),l2]3}${"
  ".4,:3^[01},'(y7:unquote),l2]3}'(c%22),.1C=?{n,,#0.3,:(i13),.2,&3{%1:2R"
  "0,.0R8?{:2,'(y5:port:),'(s27:end of file within a string),@(y10:read-e"
  "rror)[23}'(c%5c),.1C=?{${'(y6:string),:2,:1^[02},.0?{.2,.1c}{.2},:0^[3"
  "1}'(c%22),.1C=?{.1A9X3]2}.1,.1c,:0^[21}.!0.0^_1[21}'(c|),.1C=?{n,,#0.3"
  ",:(i13),.2,&3{%1:2R0,.0R8?{:2,'(y5:port:),'(s29:end of file within a |"
  "symbol|),@(y10:read-error)[23}'(c%5c),.1C=?{${'(y6:symbol),:2,:1^[02},"
  ".0?{.2,.1c}{.2},:0^[31}'(c|),.1C=?{.1A9X3X5]2}.1,.1c,:0^[21}.!0.0^_1[2"
  "1}'(c#),.1C=?{.1R1,.0R8?{.2,'(y5:port:),'(s19:end of file after #),@(y"
  "10:read-error)[33}'(c!),.1C=?{.2R0${.4,:3^[01},.0,'(l2:y9:fold-case;y1"
  "2:no-fold-case;),.1A1?{'(y9:fold-case),.2q:!(i12):(i12)^,.5P79.4,:(i10"
  ")^[51}.1Y0?{.1Y6]5}.4,'(y5:port:),.3,'(s24:unexpected name after #!),@"
  "(y10:read-error)[54}'(ct),.1Ci=,.0?{.0}{'(cf),.2Ci=}_1?{${.4,:3^[01},."
  "0,'(l2:y1:t;y4:true;),.1A1?{t]5}'(l2:y1:f;y5:false;),.1A1?{f]5}.4,'(y5"
  ":port:),.3,'(s23:unexpected name after #),@(y10:read-error)[54}'(cb),."
  "1Ci=,.0?{.0}{'(co),.2Ci=,.0?{.0}{'(cd),.3Ci=,.0?{.0}{'(cx),.4Ci=,.0?{."
  "0}{'(ci),.5Ci=,.0?{.0}{'(ce),.6Ci=}_1}_1}_1}_1}_1?{.2,'(c#),:(i11)^[32"
  "}'(c&),.1C=?{.2R0${.4,:3^[01}b]3}'(c;),.1C=?{.2R0${.4,:3^[01}.2,:(i10)"
  "^[31}'(c|),.1C=?{.2R0${,#0.5,.1,&2{%0:1R0,.0R8?{:1,'(y5:port:),'(s25:e"
  "nd of file in #| comment),@(y10:read-error)[13}'(c|),.1C=?{:1R1,.0R8?{"
  ":1,'(y5:port:),'(s25:end of file in #| comment),@(y10:read-error)[23}'"
  "(c#),.1C=?{:1R0]2}:0^[20}'(c#),.1C=?{:1R1,.0R8?{:1,'(y5:port:),'(s25:e"
  "nd of file in #| comment),@(y10:read-error)[23}'(c|),.1C=?{:1R0${:0^[0"
  "0}:0^[20}:0^[20}:0^[10}.!0.0^_1[00}.2,:(i10)^[31}'(c(),.1C=?{.2R0${f,:"
  "9^,.6,.5,:8^[04}X1]3}'(cu),.1C=?{.2R0'(c8),.3R0q?{'(c(),.3R0q}{f}?{${."
  "4,:7^[01}E1]3}.2,'(y5:port:),'(s25:invalid bytevector syntax),@(y10:re"
  "ad-error)[33}'(c%5c),.1C=?{.2R0.2R1,.0R8?{.3,'(y5:port:),'(s20:end of "
  "file after #%5c),@(y10:read-error)[43}.0,'(cx)C=?{.3R0${.5R1,:5^[01}?{"
  ".0]4}f,.4,:6^[42}.0C4?{${.5,:3^[01},'1,.1X4S3=?{.1]5}.0,'(y4:null),.1v"
  "?{'0X9]6}'(y5:space),.1v?{'(c )]6}'(y5:alarm),.1v?{'(c%07)]6}'(y9:back"
  "space),.1v?{'(c%08)]6}'(y6:delete),.1v?{'(i127)X9]6}'(y6:escape),.1v?{"
  "'(i27)X9]6}'(y3:tab),.1v?{'(c%09)]6}'(l2:y7:newline;y8:linefeed;),.1A1"
  "?{'(c%0a)]6}'(y4:vtab),.1v?{'(c%0b)]6}'(y4:page),.1v?{'(c%0c)]6}'(y6:r"
  "eturn),.1v?{'(c%0d)]6}.5,'(y5:port:),.3,'(s15:unknown #%5c name),@(y10"
  ":read-error)[64}.3R0.0]4}.0C5?{:0?{${.4,'(y5:port:),'(s44:#N=/#N# nota"
  "tion is not allowed in this mode),@(y10:read-error)[03}}n,,#0.4,.1,:4,"
  ":3,:2,:1,&6{%1:5R0,.0R8?{:5,'(y5:port:),'(s32:end of file within a #N "
  "notation),@(y10:read-error)[23}.0C5?{.1,.1c,:4^[21}'(c#),.1C=?{.1A9X3,"
  "'(i10),.1E9,.0I0?{:0^,.1A3}{f},.0?{.0d]5}'(s22:unknown #n# reference:)"
  ",'(y5:port:),.4,@(y10:read-error)[53}'(c=),.1C=?{.1A9X3,'(i10),.1E9,.0"
  "I0~?{${'(s22:invalid #n= reference:),'(y5:port:),.5,@(y10:read-error)["
  "03}}{:0^,.1A3?{${'(s18:duplicate #n= tag:),'(y5:port:),.4,@(y10:read-e"
  "rror)[03}}{f}}fb,:0^,${.3,:1^[01},.3cc:!0${:5,:2^[01},${.2,:3^[01}?{'("
  "s31:#n= has another label as target),'(y5:port:),.5,@(y10:read-error)["
  "63}.0,.2sz.0]6}:5,'(y5:port:),'(s34:invalid terminator for #N notation"
  "),@(y10:read-error)[23}.!0.0^_1[31}.2,'(y5:port:),.2,'(s16:unknown # s"
  "yntax),@(y10:read-error)[34}.1,'(y5:port:),.2,'(s22:illegal character "
  "read),@(y10:read-error)[24}.!(i16).(i16),.(i12),.(i17),.(i13),&4{%4${."
  "3,:3^[01},:0^,.1q?{.2,'(y5:port:),'(s42:missing car -- ( immediately f"
  "ollowed by .),@(y10:read-error)[53}.0,,#0.0,.5,:3,:2,.(i10),:1,.(i11),"
  ":0,&8{%1.0R8?{:6,'(y5:port:),'(s41:eof inside list -- unbalanced paren"
  "theses),@(y10:read-error)[13}:1,.1q?{n]1}:0^,.1q?{:3?{${:6,:2^[01},${:"
  "6,:5^[01},:1,.1q?{.1]3}:6,'(y5:port:),.2,'(s31:randomness after form a"
  "fter dot),@(y10:read-error)[34}:6,'(y5:port:),'(s13:dot in #(...)),@(y"
  "10:read-error)[13}${.2,:4^[01}?{:6,'(y5:port:),.2d,'(s20:error inside "
  "list --),@(y10:read-error)[14}${${:6,:5^[01},:7^[01},.1c]1}.!0.0^_1[51"
  "}.!(i17).(i16),.9,.(i13),&3{%1${.2,:2^[01},,#0.0,.3,:2,:0,:1,&5{%1.0R8"
  "?{:3,'(y5:port:),'(s21:eof inside bytevector),@(y10:read-error)[13}:0^"
  ",.1q?{n]1}${.2,:1^[01}?{:3,'(y5:port:),.2d,'(s26:error inside bytevect"
  "or --),@(y10:read-error)[14}.0I0~,.0?{.0}{'0,.2I<,.0?{.0}{'(i255),.3I>"
  "}_1}_1?{:3,'(y5:port:),.2,'(s33:invalid byte inside bytevector --),@(y"
  "10:read-error)[14}${${:3,:2^[01},:4^[01},.1c]1}.!0.0^_1[11}.!(i18).(i2"
  "0),&1{%2.0R0,.0R8?{${.3,'(y5:port:),.6,'(s20:end of file within a),@(y"
  "10:read-error)[04}}'(c%5c),.1C=,.0?{.0}{'(c%22),.2C=,.0?{.0}{'(c|),.3C"
  "=}_1}_1?{.0]3}'(ca),.1C=?{'(c%07)]3}'(cb),.1C=?{'(c%08)]3}'(ct),.1C=?{"
  "'(c%09)]3}'(cn),.1C=?{'(c%0a)]3}'(cv),.1C=?{'(c%0b)]3}'(cf),.1C=?{'(c%"
  "0c)]3}'(cr),.1C=?{'(c%0d)]3}'(cx),.1C=?{t,.2,:0^[32}'(y6:string),.3q?{"
  ".0C1}{f}?{.1R1,'(c%0a),.2C=,,#0.0,.5,&2{%2.1R8,.0?{.0}{.2C1~}_1?{.0?{f"
  "]2}:0,'(y5:port:),'(s32:no newline in line ending escape),@(y10:read-e"
  "rror)[23}.0?{'(c%0a),.2C=}{f}?{f]2}:0R0:0R1,.1,.0?{.0}{'(c%0a),.4C=}_1"
  ",:1^[22}.!0.0^_1[32}.1,'(y5:port:),.2,'(y1::),.6,'(s22:invalid char es"
  "cape in),@(y10:read-error)[36}.!(i19).(i14),.(i14),&2{%2,#0.1,&1{%1.0u"
  "?{:0,'(y5:port:),'(s31:%5cx escape sequence is too short),@(y10:read-e"
  "rror)[13}'(i16),.1A9X3X7X9]1}.!0'0,n,.3R1,,#0.0,.6,:0,.7,.(i10),:1,&6{"
  "%3.0R8?{:1?{:4,'(y5:port:),'(s27:end of file within a string),@(y10:re"
  "ad-error)[33}.1,:2^[31}:1?{'(c;),.1C=}{f}?{:4R0.1,:2^[31}:1~?{${.2,:0^"
  "[01}}{f}?{.1,:2^[31}${.2,:3^[01}~?{:4,'(y5:port:),.2,'(s37:unexpected "
  "char in %5cx escape sequence),@(y10:read-error)[34}'2,.3>?{:4,'(y5:por"
  "t:),'(s30:%5cx escape sequence is too long),@(y10:read-error)[33}:4R0'"
  "1,.3+,.2,.2c,:4R1,:5^[33}.!0.0^_1[33}.!(i20)&0{%4.0,.0?{.0}{.2C5}_1?{f"
  "]4}'(s2:+i),.4Si=,.0?{.0}{'(s2:-i),.5Si=}_1?{f]4}'(s6:+nan.0),.4Si=,.0"
  "?{.0}{'(s6:-nan.0),.5Si=}_1?{f]4}'(s6:+inf.0),.4Si=,.0?{.0}{'(s6:-inf."
  "0),.5Si=}_1?{f]4}'(c+),.2C=,.0?{.0}{'(c-),.3C=}_1?{.2du?{t]4}'(c.),.3d"
  "aC=?{.2ddp?{.2ddaC5~]4}f]4}.2daC5~]4}'(c.),.2C=?{.2dp?{.2daC5~]4}f]4}f"
  "]4}.!(i21).(i14),.(i22),.(i12),.3,.(i16),&5{%2'(c#),.1C=,.1,l1,.3R1,,#"
  "0.5,.1,:0,:1,:2,:3,:4,&7{%3.0R8,.0?{.0}{${.3,:0^[01}}_1?{.1A9,.0a,.1X3"
  ",.5,.0?{.0}{.2C5,.0?{.0}{'(c+),.4C=,.0?{.0}{'(c-),.5C=,.0?{.0}{'(c.),."
  "6C=}_1}_1}_1}_1?{'(s1:.),.1S=?{:2^]6}${.2,.5,.5,.(i10),:1^[04}?{:3^?{."
  "0SfX5]6}.0X5]6}'(i10),.1E9,.0?{.0]7}:6,'(y5:port:),.3,'(s54:unsupporte"
  "d number syntax (implementation restriction)),@(y10:read-error)[74}:3^"
  "?{.0SfX5]6}.0X5]6}'(c#),.1C=?{:6R0t,.2,.2c,:6R1,:5^[33}${.2,:4^[01}?{:"
  "6R0.2,.2,.2c,:6R1,:5^[33}:6,'(y5:port:),.2,'(s29:unexpected number/sym"
  "bol char),@(y10:read-error)[34}.!0.0^_1[23}.!(i22)${.(i25),.(i19)^[01}"
  ",${.2,.(i15)^[01}~?{.2^u?{.0](i26)}.0,.8^[(i26)1}.(i24),'(y5:port:),.2"
  "d,'(s17:unexpected token:),@(y10:read-error)[(i26)4",

  "C", 0,
  "&0{%1f,.1,@(y5:%25read)[12}%x,&0{%0f,Pi,@(y5:%25read)[02}%x,&2{|00|11%"
  "%}@!(y4:read)",

  "C", 0,
  "&0{%1t,.1,@(y5:%25read)[12}%x,&0{%0t,Pi,@(y5:%25read)[02}%x,&2{|00|11%"
  "%}@!(y11:read-simple)",

  "P", "write-substring",
  "%4.1,,#0.0,.3,.7,.7,&4{%1:0,.1I<!?{]1}:1,.1,:2S4W0'1,.1I+,:3^[11}.!0.0"
  "^_1[41",

  "C", 0,
  "&0{%4.1,.4,.4,.3,@(y15:write-substring)[44}%x,&0{%3.1,.1S3,.4,.3,@(y15"
  ":write-substring)[34}%x,&0{%2.1,.1W1]2}%x,&0{%1Po,.1W1]1}%x,&4{|10|21|"
  "32|43%%}@!(y12:write-string)",

  "P", "write-subbytevector",
  "%4.1,,#0.0,.3,.7,.7,&4{%1:0,.1I<!?{]1}:1,.1,:2B4W2'1,.1I+,:3^[11}.!0.0"
  "^_1[41",

  "C", 0,
  "&0{%4.1,.4,.4,.3,@(y19:write-subbytevector)[44}%x,&0{%3.1,.1B3,.4,.3,@"
  "(y19:write-subbytevector)[34}%x,&0{%2.1,.1W3]2}%x,&0{%1Po,.1W3]1}%x,&4"
  "{|10|21|32|43%%}@!(y16:write-bytevector)",

  "P", "%command-line",
  "%0'0,n,,#0.0,&1{%2.1Z0,.0?{'1,.3I+,.2,.2c,:0^[32}.1A9]3}.!0.0^_1[02",

  "C", 0,
  "${${@(y13:%25command-line)[00},@(y14:make-parameter)[01}@!(y12:command"
  "-line)",

  "P", "features",
  "%0'(l4:y4:r7rs;y12:exact-closed;y5:skint;y11:skint-1.0.0;)]0",

  "P", "feature-available?",
  "%1.0Y0?{${@(y8:features)[00},.1A0]1}f]1",

  "C", 0,
  "f,#0.0,&1{%0:0^,.0?{.0]1}'0,n,,#0.0,:0,&2{%2.1Z2,.0?{.0,'(c=)S8,.0?{.0"
  ",'0,.3S7}{.1},.1?{.2S3,'1,.3I+,.4S7}{'(s0:)},'1,.6I+,.5,.2,.4cc,:1^[62"
  "}.1A9:!0:0^]3}.!0.0^_1[12}_1@!(y25:get-environment-variables)",

  "P", "emergency-exit",
  "%!0.0u?{tZ9]1}.0aZ9]1",

  "C", 0,
  "@(y14:emergency-exit)@!(y4:exit)",

  "P", "%make-exit",
  "%1.0,&1{%!0.0u?{t,:0[11}.0a,:0[11}]1",

  "C", 0,
  "${k0,.0,${.2,@(y10:%25make-exit)[01}@!(y4:exit)'(y8:continue)_1_3},'(y"
  "8:continue),.1q~?{${.2,@(y14:emergency-exit)[01}}_1",

  "C", 0,
  "${@(y5:write),@(y14:make-parameter)[01}@!(y19:format-pretty-print)",

  "C", 0,
  "${&0{%4.3,.1W4]4},@(y14:make-parameter)[01}@!(y18:format-fixed-print)",

  "C", 0,
  "${@(y7:newline),@(y14:make-parameter)[01}@!(y17:format-fresh-line)",

  "C", 0,
  "${'(s78:supported directives: ~~ ~%25 ~& ~t ~_ ~a ~s ~w ~y ~c ~b ~o ~d"
  " ~x ~f ~? ~k ~* ~!),@(y14:make-parameter)[01}@!(y18:format-help-string"
  ")",

  "P", "fprintf",
  "%!2,,,,#0#1#2#3&0{%1.0p?{.0a]1}'(s35:format: no argument for ~ directi"
  "ve),@(y5:error)[11}.!0&0{%1.0p?{.0a]1}'(s30:format: incomplete ~ direc"
  "tive),@(y5:error)[11}.!1&0{%3.1N0?{.2,.1,.3E8W4]3}.2,.2W5]3}.!2.1,&1{%"
  "3.1,${.3,:0^[01},.2,,#0.0,:0,.8,&3{%3.1C5?{'0,.3z<?{'0,.3sz}.1Cv,'(i10"
  "),.4z*+,.3sz.2,${.3d,:1^[01},.2d,:2^[33}'(c,),.2C=?{:0,${.3d,:1^[01},."
  "2d,:2^[33}.0]3}.!0.0^_1[33}.!3.4,.7X2,,#0.0,.9,.8,.6,.9,&5{%2.0u?{.1]2"
  "}'(c~),.1aC=?{.0du?{${'(s34:format: incomplete escape sequence),@(y5:e"
  "rror)[01}}'(i-1),#0'(i-1),#0${.2,.4,.6d,:2^[03},.0aCd,'(c*),.1v?{.5dd,"
  ".2d,:4^[62}'(c~),.1v?{:3,'(c~)W0.5,.2d,:4^[62}'(c%25),.1v?{:3W6.5,.2d,"
  ":4^[62}'(ct),.1v?{:3,'(c%09)W0.5,.2d,:4^[62}'(c_),.1v?{:3,'(c )W0.5,.2"
  "d,:4^[62}'(c&),.1v?{${:3,${@(y17:format-fresh-line)[00}[01}.5,.2d,:4^["
  "62}'(c!),.1v?{:3P71.5,.2d,:4^[62}'(cs),.1v?{:3,${.8,:1^[01}W5.5d,.2d,:"
  "4^[62}'(ca),.1v?{:3,${.8,:1^[01}W4.5d,.2d,:4^[62}'(cw),.1v?{:3,${.8,:1"
  "^[01}W7.5d,.2d,:4^[62}'(cc),.1v?{:3,${.8,:1^[01}W0.5d,.2d,:4^[62}'(cb)"
  ",.1v?{${:3,${.(i10),:1^[01},'2,:0^[03}.5d,.2d,:4^[62}'(co),.1v?{${:3,$"
  "{.(i10),:1^[01},'8,:0^[03}.5d,.2d,:4^[62}'(cd),.1v?{${:3,${.(i10),:1^["
  "01},'(i10),:0^[03}.5d,.2d,:4^[62}'(cx),.1v?{${:3,${.(i10),:1^[01},'(i1"
  "6),:0^[03}.5d,.2d,:4^[62}'(ch),.1v?{:3,${@(y18:format-help-string)[00}"
  "W4.5,.2d,:4^[62}'(cy),.1v?{${:3,${.(i10),:1^[01},${@(y19:format-pretty"
  "-print)[00}[02}.5d,.2d,:4^[62}'(cf),.1v?{${:3,.5^,.7^,${.(i12),:1^[01}"
  ",${@(y18:format-fixed-print)[00}[04}.5d,.2d,:4^[62}'(l2:c?;ck;),.1A1?{"
  "${${${.(i11),:1^[01},:1^[01},${.(i10),:1^[01}X2,:4^[02}.5dd,.2d,:4^[62"
  "}.1a,'(s32:format: unrecognized ~ directive),@(y5:error)[62}:3,.1aW0.1"
  ",.1d,:4^[22}.!0.0^_1[72",

  "P", "format",
  "%!1f,.2q,.0?{.0}{.2S0}_1?{P51,.2?{.1,.3c}{.1},${.2,.4c,@(y7:fprintf),@"
  "(y13:apply-to-list)[02}.1P90]4}t,.2q?{.0,Poc,@(y7:fprintf),@(y13:apply"
  "-to-list)[22}.0,.2c,@(y7:fprintf),@(y13:apply-to-list)[22",

  "P", "write-to-string",
  "%1P51,.0,.2W5.0P90,.1P61.0]3",

  "P", "read-from-string",
  "%1.0P50,${${k0,.0,${.2,.9,&2{%0:1,&1{%!0.0,&1{%0:0,@(y6:values),@(y13:"
  "apply-to-list)[02},:0[11},:0,&1{%0:0,@(y4:read)[01},@(y16:call-with-va"
  "lues)[02},.3,&1{%1${k0,.0,${.6,&1{%0:0,R9]1},:0[01}_1_3}[10},@(y22:wit"
  "h-exception-handler)[02}_1_3}[00},.1P60.0]3",

  0, 0, 0
};
