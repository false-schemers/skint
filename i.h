/* i.h -- instructions */

#ifndef glue
#define glue(a, b) a##b
#endif
#ifndef AUTOGL
#define AUTOGL NULL
#endif
#ifndef INLINED
#define INLINED ""
#endif

#if defined(VM_GEN_DEFGLOBAL)
#define declare_instruction(name, enc, etyp, igname, arity, lcode) \
  declare_instruction_global(name)
#define declare_instrshadow(name, enc, etyp, igname, arity, lcode) 
#define declare_integrable(name, enc, etyp, igname, arity, lcode) 
#elif defined(VM_GEN_ENCTABLE)
#define declare_instruction(name, enc, etyp, igname, arity, lcode) \
  declare_enctable_entry(name, enc, etyp)
#define declare_instrshadow(name, enc, etyp, igname, arity, lcode) \
  declare_enctable_entry(name, enc, etyp)
#define declare_integrable(name, enc, etyp, igname, arity, lcode) 
#elif defined(VM_GEN_INTGTABLE)
#define declare_instruction(name, enc, etyp, igname, arity, lcode) \
  declare_intgtable_entry(enc, igname, arity, lcode)
#define declare_instrshadow(name, enc, etyp, igname, arity, lcode) \
  declare_intgtable_entry(enc, igname, arity, lcode)
#define declare_integrable(name, enc, etyp, igname, arity, lcode) \
  declare_intgtable_entry(enc, igname, arity, lcode)
#else /* regular include */
#define declare_instruction(name, enc, etyp, igname, arity, lcode) \
  extern obj glue(cx_ins_2D, name); 
#define declare_instrshadow(name, enc, etyp, igname, arity, lcode) 
#define declare_integrable(name, enc, etyp, igname, arity, lcode) 
extern obj vmcases[]; /* vm host */
#endif

/* basic vm machinery */
declare_instruction(halt,     NULL,       0, NULL, 0, NULL) 
declare_instruction(litf,     "f",        0, NULL, 0, NULL)  
declare_instruction(litt,     "t",        0, NULL, 0, NULL)  
declare_instruction(litn,     "n",        0, NULL, 0, NULL)  
declare_instruction(lit,      "'",        1, NULL, 0, NULL)  
declare_instruction(sref,     ".",        1, NULL, 0, NULL)
declare_instruction(dref,     ":",        1, NULL, 0, NULL)     
declare_instruction(gref,     "@",      'g', NULL, 0, NULL)   
declare_instruction(iref,     "^",        0, NULL, 0, NULL)  
declare_instruction(iset,     "^!",       0, NULL, 0, NULL)  
declare_instruction(dclose,   "&",      'd', NULL, 0, NULL) 
declare_instruction(sbox,     "#",        1, NULL, 0, NULL)  
declare_instruction(br,       NULL,     'b', NULL, 0, NULL) 
declare_instruction(brnot,    "?",      'b', NULL, 0, NULL) 
declare_instruction(brt,      "~?",     'b', NULL, 0, NULL)  
declare_instruction(andbo,    ";",      'a', NULL, 0, NULL)  
declare_instruction(sseti,    ".!",       1, NULL, 0, NULL) 
declare_instruction(dseti,    ":!",       1, NULL, 0, NULL)
declare_instruction(gloc,     "`",      'g', NULL, 0, NULL)   
declare_instruction(gset,     "@!",     'g', NULL, 0, NULL)
declare_instruction(appl,     "K3",       0, NULL, 0, NULL)
declare_instruction(cwmv,     "K4",       0, NULL, 0, NULL)
declare_instruction(rcmv,     "K5",       0, NULL, 0, NULL)
declare_instruction(sdmv,     "K6",       0, NULL, 0, NULL)
declare_instruction(lck,      "k",        1, NULL, 0, NULL)
declare_instruction(lck0,     "k0",       0, NULL, 0, NULL)
declare_instruction(rck,      "k!",       0, NULL, 0, NULL)
declare_instruction(wck,      "w",        0, NULL, 0, NULL)
declare_instruction(wckr,     "w!",       0, NULL, 0, NULL)
declare_instruction(save,     "$",      's', NULL, 0, NULL)
declare_instruction(push,     ",",        0, NULL, 0, NULL) 
declare_instruction(jdceq,    "|",        2, NULL, 0, NULL)  
declare_instruction(jdcge,    "|!",       2, NULL, 0, NULL)  
declare_instruction(jdref,    "|!0",      1, NULL, 0, NULL) 
declare_instruction(call,     "[0",       1, NULL, 0, NULL) 
declare_instruction(scall,    "[",        2, NULL, 0, NULL)  
declare_instruction(return,   "]0",       0, NULL, 0, NULL) 
declare_instruction(sreturn,  "]",        1, NULL, 0, NULL)  
declare_instruction(adrop,    "_",        1, NULL, 0, NULL) 
declare_instruction(pop,      "_!",       0, NULL, 0, NULL) 
declare_instruction(atest,    "%",        1, NULL, 0, NULL)
declare_instruction(shrarg,   "%!",       1, NULL, 0, NULL)
declare_instruction(aerr,     "%%",       0, NULL, 0, NULL)

/* popular instruction combos */
declare_instruction(shlit,    ",'",       1, NULL, 0, NULL)
declare_instruction(shi0,     ",'0",      0, NULL, 0, NULL)
declare_instruction(pushlitf, "f,",       0, NULL, 0, NULL)  
declare_instruction(pushlitt, "t,",       0, NULL, 0, NULL)  
declare_instruction(pushlitn, "n,",       0, NULL, 0, NULL)  
declare_instruction(lit0,     "'0",       0, NULL, 0, NULL)  
declare_instruction(lit1,     "'1",       0, NULL, 0, NULL)  
declare_instruction(lit2,     "'2",       0, NULL, 0, NULL)  
declare_instruction(lit3,     "'3",       0, NULL, 0, NULL)  
declare_instruction(lit4,     "'4",       0, NULL, 0, NULL)  
declare_instruction(lit5,     "'5",       0, NULL, 0, NULL)  
declare_instruction(lit6,     "'6",       0, NULL, 0, NULL)  
declare_instruction(lit7,     "'7",       0, NULL, 0, NULL)  
declare_instruction(lit8,     "'8",       0, NULL, 0, NULL)  
declare_instruction(lit9,     "'9",       0, NULL, 0, NULL)  
declare_instruction(pushlit0, "'0,",      0, NULL, 0, NULL)  
declare_instruction(pushlit1, "'1,",      0, NULL, 0, NULL)  
declare_instruction(pushlit2, "'2,",      0, NULL, 0, NULL)  
declare_instruction(pushlit3, "'3,",      0, NULL, 0, NULL)  
declare_instruction(pushlit4, "'4,",      0, NULL, 0, NULL)  
declare_instruction(pushlit5, "'5,",      0, NULL, 0, NULL)  
declare_instruction(pushlit6, "'6,",      0, NULL, 0, NULL)  
declare_instruction(pushlit7, "'7,",      0, NULL, 0, NULL)  
declare_instruction(pushlit8, "'8,",      0, NULL, 0, NULL)  
declare_instruction(pushlit9, "'9,",      0, NULL, 0, NULL)  

declare_instruction(sref0,    ".0",       0, NULL, 0, NULL)
declare_instruction(sref1,    ".1",       0, NULL, 0, NULL)
declare_instruction(sref2,    ".2",       0, NULL, 0, NULL)
declare_instruction(sref3,    ".3",       0, NULL, 0, NULL)
declare_instruction(sref4,    ".4",       0, NULL, 0, NULL)
declare_instruction(sref5,    ".5",       0, NULL, 0, NULL)
declare_instruction(sref6,    ".6",       0, NULL, 0, NULL)
declare_instruction(sref7,    ".7",       0, NULL, 0, NULL)
declare_instruction(sref8,    ".8",       0, NULL, 0, NULL)
declare_instruction(sref9,    ".9",       0, NULL, 0, NULL)
declare_instruction(pushsref0, ".0,",     0, NULL, 0, NULL)
declare_instruction(pushsref1, ".1,",     0, NULL, 0, NULL)
declare_instruction(pushsref2, ".2,",     0, NULL, 0, NULL)
declare_instruction(pushsref3, ".3,",     0, NULL, 0, NULL)
declare_instruction(pushsref4, ".4,",     0, NULL, 0, NULL)
declare_instruction(pushsref5, ".5,",     0, NULL, 0, NULL)
declare_instruction(pushsref6, ".6,",     0, NULL, 0, NULL)
declare_instruction(pushsref7, ".7,",     0, NULL, 0, NULL)
declare_instruction(pushsref8, ".8,",     0, NULL, 0, NULL)
declare_instruction(pushsref9, ".9,",     0, NULL, 0, NULL)

declare_instruction(srefi0,    ".0^",     0, NULL, 0, NULL)
declare_instruction(srefi1,    ".1^",     0, NULL, 0, NULL)
declare_instruction(srefi2,    ".2^",     0, NULL, 0, NULL)
declare_instruction(srefi3,    ".3^",     0, NULL, 0, NULL)
declare_instruction(srefi4,    ".4^",     0, NULL, 0, NULL)
declare_instruction(pushsrefi0, ".0^,",   0, NULL, 0, NULL)
declare_instruction(pushsrefi1, ".1^,",   0, NULL, 0, NULL)
declare_instruction(pushsrefi2, ".2^,",   0, NULL, 0, NULL)
declare_instruction(pushsrefi3, ".3^,",   0, NULL, 0, NULL)
declare_instruction(pushsrefi4, ".4^,",   0, NULL, 0, NULL)

declare_instruction(dref0,    ":0",       0, NULL, 0, NULL)
declare_instruction(dref1,    ":1",       0, NULL, 0, NULL)
declare_instruction(dref2,    ":2",       0, NULL, 0, NULL)
declare_instruction(dref3,    ":3",       0, NULL, 0, NULL)
declare_instruction(dref4,    ":4",       0, NULL, 0, NULL)
declare_instruction(pushdref0, ":0,",     0, NULL, 0, NULL)
declare_instruction(pushdref1, ":1,",     0, NULL, 0, NULL)
declare_instruction(pushdref2, ":2,",     0, NULL, 0, NULL)
declare_instruction(pushdref3, ":3,",     0, NULL, 0, NULL)
declare_instruction(pushdref4, ":4,",     0, NULL, 0, NULL)

declare_instruction(drefi0,    ":0^",     0, NULL, 0, NULL)
declare_instruction(drefi1,    ":1^",     0, NULL, 0, NULL)
declare_instruction(drefi2,    ":2^",     0, NULL, 0, NULL)
declare_instruction(drefi3,    ":3^",     0, NULL, 0, NULL)
declare_instruction(drefi4,    ":4^",     0, NULL, 0, NULL)
declare_instruction(pushdrefi0, ":0^,",   0, NULL, 0, NULL)
declare_instruction(pushdrefi1, ":1^,",   0, NULL, 0, NULL)
declare_instruction(pushdrefi2, ":2^,",   0, NULL, 0, NULL)
declare_instruction(pushdrefi3, ":3^,",   0, NULL, 0, NULL)
declare_instruction(pushdrefi4, ":4^,",   0, NULL, 0, NULL)

declare_instruction(call0,    "[00",      0, NULL, 0, NULL) 
declare_instruction(call1,    "[01",      0, NULL, 0, NULL) 
declare_instruction(call2,    "[02",      0, NULL, 0, NULL) 
declare_instruction(call3,    "[03",      0, NULL, 0, NULL) 
declare_instruction(call4,    "[04",      0, NULL, 0, NULL)

declare_instruction(scall1,   "[1",       1, NULL, 0, NULL) 
declare_instruction(scall10,  "[10",      0, NULL, 0, NULL) 
declare_instruction(scall11,  "[11",      0, NULL, 0, NULL) 
declare_instruction(scall12,  "[12",      0, NULL, 0, NULL) 
declare_instruction(scall13,  "[13",      0, NULL, 0, NULL) 
declare_instruction(scall14,  "[14",      0, NULL, 0, NULL)
declare_instruction(scall2,   "[2",       1, NULL, 0, NULL) 
declare_instruction(scall20,  "[20",      0, NULL, 0, NULL) 
declare_instruction(scall21,  "[21",      0, NULL, 0, NULL) 
declare_instruction(scall22,  "[22",      0, NULL, 0, NULL) 
declare_instruction(scall23,  "[23",      0, NULL, 0, NULL) 
declare_instruction(scall24,  "[24",      0, NULL, 0, NULL)
declare_instruction(scall3,   "[3",       1, NULL, 0, NULL) 
declare_instruction(scall30,  "[30",      0, NULL, 0, NULL) 
declare_instruction(scall31,  "[31",      0, NULL, 0, NULL) 
declare_instruction(scall32,  "[32",      0, NULL, 0, NULL) 
declare_instruction(scall33,  "[33",      0, NULL, 0, NULL) 
declare_instruction(scall34,  "[34",      0, NULL, 0, NULL)
declare_instruction(scall4,   "[4",       1, NULL, 0, NULL) 
declare_instruction(scall40,  "[40",      0, NULL, 0, NULL) 
declare_instruction(scall41,  "[41",      0, NULL, 0, NULL) 
declare_instruction(scall42,  "[42",      0, NULL, 0, NULL) 
declare_instruction(scall43,  "[43",      0, NULL, 0, NULL) 
declare_instruction(scall44,  "[44",      0, NULL, 0, NULL)

declare_instruction(sreturn1, "]1",       0, NULL, 0, NULL) 
declare_instruction(sreturn2, "]2",       0, NULL, 0, NULL) 
declare_instruction(sreturn3, "]3",       0, NULL, 0, NULL) 
declare_instruction(sreturn4, "]4",       0, NULL, 0, NULL) 

declare_instruction(atest0,   "%0",       0, NULL, 0, NULL) 
declare_instruction(atest1,   "%1",       0, NULL, 0, NULL) 
declare_instruction(atest2,   "%2",       0, NULL, 0, NULL) 
declare_instruction(atest3,   "%3",       0, NULL, 0, NULL) 
declare_instruction(atest4,   "%4",       0, NULL, 0, NULL) 

declare_instruction(brnotlt,  "<?",     'b', NULL, 0, NULL) 
declare_instruction(pushsub,  "-,",       0, NULL, 0, NULL) 

/* type checks */
declare_instruction(ckp,      "%p",       0, "%ckp", 1, INLINED)
declare_instruction(ckl,      "%l",       0, "%ckl", 1, INLINED) 
declare_instruction(ckv,      "%v",       0, "%ckv", 1, INLINED) 
declare_instruction(ckc,      "%c",       0, "%ckc", 1, INLINED) 
declare_instruction(cks,      "%s",       0, "%cks", 1, INLINED) 
declare_instruction(cki,      "%i",       0, "%cki", 1, INLINED) 
declare_instruction(ckj,      "%j",       0, "%ckj", 1, INLINED) 
declare_instruction(ckn,      "%n",       0, "%ckn", 1, INLINED) 
declare_instruction(ckk,      "%k",       0, "%ckk", 1, INLINED) 
declare_instruction(cky,      "%y",       0, "%cky", 1, INLINED) 
declare_instruction(ckr,      "%r",       0, "%ckr", 1, INLINED) 
declare_instruction(ckw,      "%w",       0, "%ckw", 1, INLINED) 
declare_instruction(ckx,      "%x",       0, "%ckx", 1, INLINED) 
declare_instruction(ckz,      "%z",       0, "%ckz", 1, INLINED) 

/* intrinsics (no arg checks), integrables and globals */
declare_instruction(isq,      "q",        0, "eq?",               '2', AUTOGL)
declare_instruction(isv,      "v",        0, "eqv?",              '2', AUTOGL)
declare_instruction(ise,      "e",        0, "equal?",            '2', AUTOGL)
declare_instruction(box,      "b",        0, "box",               '1', AUTOGL)
declare_instruction(unbox,    "z",        0, "unbox",             '1', AUTOGL)
declare_instruction(setbox,   "z!",       0, "set-box!",          '2', AUTOGL)
declare_instruction(car,      "a",        0, "car",               '1', AUTOGL)
declare_instruction(setcar,   "a!",       0, "set-car!",          '2', AUTOGL)
declare_instruction(cdr,      "d",        0, "cdr",               '1', AUTOGL)
declare_instruction(setcdr,   "d!",       0, "set-cdr!",          '2', AUTOGL)
declare_instruction(caar,     "aa",       0, "caar",              '1', AUTOGL)
declare_instruction(cadr,     "da",       0, "cadr",              '1', AUTOGL)
declare_instruction(cdar,     "ad",       0, "cdar",              '1', AUTOGL)
declare_instruction(cddr,     "dd",       0, "cddr",              '1', AUTOGL)
declare_instruction(nullp,    "u",        0, "null?",             '1', AUTOGL)
declare_instruction(pairp,    "p",        0, "pair?",             '1', AUTOGL)
declare_instruction(cons,     "c",        0, "cons",              '2', AUTOGL)
declare_instruction(not,      "~",        0, "not",               '1', AUTOGL)
declare_instruction(izerop,   "I=0",      0, "fxzero?",           '1', AUTOGL)
declare_instruction(iposp,    "I>0",      0, "fxpositive?",       '1', AUTOGL)
declare_instruction(inegp,    "I<0",      0, "fxnegative?",       '1', AUTOGL)
declare_instruction(ievnp,    "Ie",       0, "fxeven?",           '1', AUTOGL)
declare_instruction(ioddp,    "Io",       0, "fxodd?",            '1', AUTOGL)
declare_instruction(iadd,  "I+\0'0",      0, "fx+",               'p', AUTOGL)
declare_instruction(isub,  "I-\0I-!",     0, "fx-",               'm', AUTOGL)
declare_instruction(imul,  "I*\0'1",      0, "fx*",               'p', AUTOGL)
declare_instruction(idiv,  "I/\0,'1I/",   0, "fx/",               'm', AUTOGL)
declare_instruction(iquo,     "Iq",       0, "fxquotient",        '2', AUTOGL)
declare_instruction(irem,     "Ir",       0, "fxremainder",       '2', AUTOGL)
declare_instruction(ilt,      "I<",       0, "fx<?",              'c', AUTOGL)
declare_instruction(igt,      "I>",       0, "fx>?",              'c', AUTOGL)
declare_instruction(ile,      "I>!",      0, "fx<=?",             'c', AUTOGL)
declare_instruction(ige,      "I<!",      0, "fx>=?",             'c', AUTOGL)
declare_instruction(ieq,      "I=",       0, "fx=?",              'c', AUTOGL)
declare_instruction(ine,      "I=!",      0, "fx!=?",             '2', AUTOGL)
declare_instruction(imin,     "In",       0, "fxmin",             'x', AUTOGL)
declare_instruction(imax,     "Ix",       0, "fxmax",             'x', AUTOGL)
declare_instruction(ineg,     "I-!",      0, "fxneg",             '1', AUTOGL)
declare_instruction(iabs,     "Ia",       0, "fxabs",             '1', AUTOGL)
declare_instruction(itoj,     "Ij",       0, "fixnum->flonum",    '1', AUTOGL)
declare_instruction(fixp,     "I0",       0, "fixnum?",           '1', AUTOGL)
declare_instruction(imqu,     "I3",       0, "fxmodquo",          '2', AUTOGL)
declare_instruction(imlo,     "I4",       0, "fxmodulo",          '2', AUTOGL)
declare_instruction(ieuq,     "I5",       0, "fxeucquo",          '2', AUTOGL)
declare_instruction(ieur,     "I6",       0, "fxeucrem",          '2', AUTOGL)
declare_instruction(igcd,     "I7",       0, "fxgcd",             '2', AUTOGL)
declare_instruction(ipow,     "I8",       0, "fxexpt",            '2', AUTOGL)
declare_instruction(isqrt,    "I9",       0, "fxsqrt",            '1', AUTOGL)
declare_instruction(inot,     "D0",       0, "fxnot",             '1', AUTOGL)
declare_instruction(iand,  "D1\0'(i-1)",  0, "fxand",             'p', AUTOGL)
declare_instruction(iior,  "D2\0'0",      0, "fxior",             'p', AUTOGL)
declare_instruction(ixor,  "D3\0'0",      0, "fxxor",             'p', AUTOGL)
declare_instruction(iasl,     "D4",       0, "fxsll",             '2', AUTOGL)
declare_instruction(iasr,     "D5",       0, "fxsrl",             '2', AUTOGL)
declare_instruction(jzerop,   "J=0",      0, "flzero?",           '1', AUTOGL)
declare_instruction(jposp,    "J>0",      0, "flpositive?",       '1', AUTOGL)
declare_instruction(jnegp,    "J<0",      0, "flnegative?",       '1', AUTOGL)
declare_instruction(jevnp,    "Je",       0, "fleven?",           '1', AUTOGL)
declare_instruction(joddp,    "Jo",       0, "flodd?",            '1', AUTOGL)
declare_instruction(jintp,    "Jw",       0, "flinteger?",        '1', AUTOGL)
declare_instruction(jnanp,    "Ju",       0, "flnan?",            '1', AUTOGL)
declare_instruction(jfinp,    "Jf",       0, "flfinite?",         '1', AUTOGL)
declare_instruction(jinfp,    "Jh",       0, "flinfinite?",       '1', AUTOGL)
declare_instruction(jadd, "J+\0'(j0)",    0, "fl+",               'p', AUTOGL)  
declare_instruction(jsub, "J-\0J-!",      0, "fl-",               'm', AUTOGL)
declare_instruction(jmul, "J*\0'(j1)",    0, "fl*",               'p', AUTOGL)
declare_instruction(jdiv, "J/\0,'(j1)J/", 0, "fl/",               'm', AUTOGL)
declare_instruction(jquo,     "Jq",       0, "flquotient",        '2', AUTOGL)
declare_instruction(jrem,     "Jr",       0, "flremainder",       '2', AUTOGL)
declare_instruction(jlt,      "J<",       0, "fl<?",              'c', AUTOGL)
declare_instruction(jgt,      "J>",       0, "fl>?",              'c', AUTOGL)
declare_instruction(jle,      "J>!",      0, "fl<=?",             'c', AUTOGL)
declare_instruction(jge,      "J<!",      0, "fl>=?",             'c', AUTOGL)
declare_instruction(jeq,      "J=",       0, "fl=?",              'c', AUTOGL)
declare_instruction(jne,      "J=!",      0, "fl!=?",             '2', AUTOGL)
declare_instruction(jmin,     "Jn",       0, "flmin",             'x', AUTOGL)
declare_instruction(jmax,     "Jx",       0, "flmax",             'x', AUTOGL)
declare_instruction(jneg,     "J-!",      0, "flneg",             '1', AUTOGL)
declare_instruction(jabs,     "Ja",       0, "flabs",             '1', AUTOGL)
declare_instruction(jtoi,     "Ji",       0, "flonum->fixnum",    '1', AUTOGL)
declare_instruction(flop,     "J0",       0, "flonum?",           '1', AUTOGL)
declare_instruction(jmqu,     "J3",       0, "flmodquo",          '2', AUTOGL)
declare_instruction(jmlo,     "J4",       0, "flmodulo",          '2', AUTOGL)
declare_instruction(jfloor,   "H0",       0, "flfloor",           '1', AUTOGL)
declare_instruction(jceil,    "H1",       0, "flceiling",         '1', AUTOGL)
declare_instruction(jtrunc,   "H2",       0, "fltruncate",        '1', AUTOGL)
declare_instruction(jround,   "H3",       0, "flround",           '1', AUTOGL)
declare_instruction(zerop,    "=0",       0, "%zerop", 1, INLINED)
declare_instruction(posp,     ">0",       0, "%posp", 1, INLINED)
declare_instruction(negp,     "<0",       0, "%negp", 1, INLINED)
declare_instruction(add,      "+",        0, "%add", 2, INLINED)  
declare_instruction(sub,      "-",        0, "%sub", 2, INLINED)
declare_instruction(mul,      "*",        0, "%mul", 2, INLINED)
declare_instruction(div,      "/",        0, "%div", 2, INLINED)
declare_instruction(lt,       "<",        0, "%lt", 2, INLINED)
declare_instruction(gt,       ">",        0, "%gt", 2, INLINED)
declare_instruction(le,       ">!",       0, "%le", 2, INLINED)
declare_instruction(ge,       "<!",       0, "%ge", 2, INLINED)
declare_instruction(eq,       "=",        0, "%eq", 2, INLINED)
declare_instruction(ne,       "=!",       0, "%ne", 2, INLINED)
declare_instruction(neg,      "-!",       0, "%neg", 1, INLINED)
declare_instruction(abs,      "G0",       0, "%abs", 1, INLINED)
declare_instruction(mqu,      "G3",       0, "%mqu", 2, INLINED)
declare_instruction(mlo,      "G4",       0, "%mlo", 2, INLINED)
declare_instruction(quo,      "G5",       0, "%quo", 2, INLINED)
declare_instruction(rem,      "G6",       0, "%rem", 2, INLINED)
declare_instruction(nump,     "N0",       0, "%nump", 1, INLINED)
declare_instruction(intp,     "N4",       0, "%intp", 1, INLINED)
declare_instruction(nanp,     "N5",       0, "%nanp", 1, INLINED)
declare_instruction(finp,     "N6",       0, "%finp", 1, INLINED)
declare_instruction(infp,     "N7",       0, "%infp", 1, INLINED)
declare_instruction(evnp,     "N8",       0, "%evnp", 1, INLINED)
declare_instruction(oddp,     "N9",       0, "%oddp", 1, INLINED)
declare_instruction(ntoi,     "M0",       0, "%ntoi", 1, INLINED)
declare_instruction(ntoj,     "M1",       0, "%ntoj", 1, INLINED)
declare_instruction(min,      "M2",       0, "%min", 2, INLINED)
declare_instruction(max,      "M3",       0, "%max", 2, INLINED)
declare_instruction(listp,    "L0",       0, "%listp", 1, INLINED)
declare_instruction(list,     "l",        1, "%list", -1, "%!0_!]0")
declare_instrshadow(list,     "L1",       1, NULL, 0, INLINED)
declare_instruction(llen,     "g",        0, "%llen", 1, INLINED)
declare_instrshadow(llen,     "L3",       0, NULL, 0, INLINED)  
declare_instruction(lget,     "L4",       0, "%lget", 2, INLINED)
declare_instruction(lput,     "L5",       0, "%lput", 3, INLINED)
declare_instruction(lcat,     "L6",       0, "%lcat", 2, INLINED)
declare_instruction(memq,     "A0",       0, "%memq", 2, INLINED)
declare_instruction(memv,     "A1",       0, "%memv", 2, INLINED)
declare_instruction(meme,     "A2",       0, "%meme", 2, INLINED)
declare_instruction(assq,     "A3",       0, "%assq", 2, INLINED)
declare_instruction(assv,     "A4",       0, "%assv", 2, INLINED)
declare_instruction(asse,     "A5",       0, "%asse", 2, INLINED)
declare_instruction(ltail,    "A6",       0, "%ltail", 2, INLINED)
declare_instruction(lpair,    "A7",       0, "%lpair", 1, INLINED)
declare_instruction(lrev,     "A8",       0, "%lrev", 1, INLINED)
declare_instruction(lrevi,    "A9",       0, "%lrevi", 1, INLINED)
declare_instruction(charp,    "C0",       0, "%charp", 1, INLINED)
declare_instruction(cwsp,     "C1",       0, "%cwsp", 1, INLINED)
declare_instruction(clcp,     "C2",       0, "%clcp", 1, INLINED)
declare_instruction(cucp,     "C3",       0, "%cucp", 1, INLINED)
declare_instruction(calp,     "C4",       0, "%calp", 1, INLINED)
declare_instruction(cnup,     "C5",       0, "%cnup", 1, INLINED)
declare_instruction(cupc,     "C6",       0, "%cupc", 1, INLINED)
declare_instruction(cdnc,     "C7",       0, "%cdnc", 1, INLINED)

declare_instruction(ceq,      "C=",       0, "%ceq", 2, INLINED)
declare_instruction(clt,      "C<",       0, "%clt", 2, INLINED)
declare_instruction(cgt,      "C>",       0, "%cgt", 2, INLINED)
declare_instruction(cle,      "C>!",      0, "%cle", 2, INLINED)
declare_instruction(cge,      "C<!",      0, "%cge", 2, INLINED)
declare_instruction(cieq,     "Ci=",      0, "%cieq", 2, INLINED)
declare_instruction(cilt,     "Ci<",      0, "%cilt", 2, INLINED)
declare_instruction(cigt,     "Ci>",      0, "%cigt", 2, INLINED)
declare_instruction(cile,     "Ci>!",     0, "%cile", 2, INLINED)
declare_instruction(cige,     "Ci<!",     0, "%cige", 2, INLINED)

declare_instruction(strp,     "S0",       0, "%strp", 1, INLINED)
declare_instruction(str,      "S1",       1, "%str", -1, "%!0.0X3]1")
declare_instruction(smk,      "S2",       0, "%smk", 2, INLINED)
declare_instruction(slen,     "S3",       0, "%slen", 1, INLINED)
declare_instruction(sget,     "S4",       0, "%sget", 2, INLINED)
declare_instruction(sput,     "S5",       0, "%sput", 3, INLINED)
declare_instruction(scat,     "S6",       0, "%scat", 2, INLINED)
declare_instruction(ssub,     "S7",       0, "%ssub", 3, INLINED)

declare_instruction(seq,      "S=",       0, "%seq", 2, INLINED)
declare_instruction(slt,      "S<",       0, "%slt", 2, INLINED)
declare_instruction(sgt,      "S>",       0, "%sgt", 2, INLINED)
declare_instruction(sle,      "S>!",      0, "%sle", 2, INLINED)
declare_instruction(sge,      "S<!",      0, "%sge", 2, INLINED)
declare_instruction(sieq,     "Si=",      0, "%sieq", 2, INLINED)
declare_instruction(silt,     "Si<",      0, "%silt", 2, INLINED)
declare_instruction(sigt,     "Si>",      0, "%sigt", 2, INLINED)
declare_instruction(sile,     "Si>!",     0, "%sile", 2, INLINED)
declare_instruction(sige,     "Si<!",     0, "%sige", 2, INLINED)


declare_instruction(vecp,     "V0",       0, "%vecp", 1, INLINED)
declare_instruction(vec,      "V1",       1, "%vec", -1, "%!0.0X1]1")
declare_instruction(vmk,      "V2",       0, "%vmk", 2, INLINED)
declare_instruction(vlen,     "V3",       0, "%vlen", 1, INLINED)
declare_instruction(vget,     "V4",       0, "%vget", 2, INLINED)
declare_instruction(vput,     "V5",       0, "%vput", 3, INLINED)
declare_instruction(vcat,     "V6",       0, "%vcat", 2, INLINED)
declare_instruction(vtol,     "X0",       0, "%vtol", 1, INLINED)
declare_instruction(ltov,     "X1",       0, "%ltov", 1, INLINED)
declare_instruction(stol,     "X2",       0, "%stol", 1, INLINED)
declare_instruction(ltos,     "X3",       0, "%ltos", 1, INLINED)
declare_instruction(ytos,     "X4",       0, "%ytos", 1, INLINED)
declare_instruction(stoy,     "X5",       0, "%stoy", 1, INLINED)
declare_instruction(itos,     "X6",       0, "%itos", 2, INLINED)
declare_instruction(stoi,     "X7",       0, "%stoi", 2, INLINED)
declare_instruction(ctoi,     "X8",       0, "%ctoi", 1, INLINED)
declare_instruction(itoc,     "X9",       0, "%itoc", 1, INLINED)
declare_instruction(jtos,     "E6",       0, "%jtos", 1, INLINED)
declare_instruction(stoj,     "E7",       0, "%stoj", 1, INLINED)
declare_instruction(ntos,     "E8",       0, "%ntos", 2, INLINED)
declare_instruction(ston,     "E9",       0, "%ston", 2, INLINED)
declare_instruction(ccmp,     "O0",       0, "%ccmp", 2, INLINED)
declare_instruction(cicmp,    "O1",       0, "%cicmp", 2, INLINED)
declare_instruction(scmp,     "O2",       0, "%scmp", 2, INLINED)
declare_instruction(sicmp,    "O3",       0, "%sicmp", 2, INLINED)
declare_instruction(symp,     "Y0",       0, "%symp", 1, INLINED)
declare_instruction(boolp,    "Y1",       0, "boolean?",          '1', AUTOGL)
declare_instruction(boxp,     "Y2",       0, "box?",              '1', AUTOGL)
declare_instruction(funp,     "K0",       0, "%funp", 1, INLINED)
declare_instruction(ipp,      "P00",      0, "%ipp", 1, INLINED)
declare_instruction(opp,      "P01",      0, "%opp", 1, INLINED)
declare_instruction(sip,      "P10",      0, "%sip", 0, INLINED)
declare_instruction(sop,      "P11",      0, "%sop", 0, INLINED)
declare_instruction(sep,      "P12",      0, "%sep", 0, INLINED)
declare_instruction(ipop,     "P20",      0, "%ipop", 1, INLINED)
declare_instruction(opop,     "P21",      0, "%opop", 1, INLINED)
declare_instruction(otip,     "P40",      0, "%otip", 1, INLINED)
declare_instruction(otop,     "P41",      0, "%otop", 1, INLINED)
declare_instruction(ois,      "P50",      0, "%ois", 1, INLINED)
declare_instruction(oos,      "P51",      0, "%oos", 0, INLINED)
declare_instruction(cip,      "P60",      0, "%cip", 1, INLINED)
declare_instruction(cop,      "P61",      0, "%cop", 1, INLINED)
declare_instruction(gos,      "P9",       0, "%gos", 1, INLINED)
declare_instruction(rdc,      "R0",       0, "%rdc", 1, INLINED)
declare_instruction(rdac,     "R1",       0, "%rdac", 1, INLINED)
declare_instruction(rdcr,     "R2",       0, "%rdcr", 1, INLINED)
declare_instruction(eofp,     "R8",       0, "%eofp", 1, INLINED)
declare_instruction(eof,      "R9",       0, "%eof", 0, INLINED)
declare_instruction(wrc,      "W0",       0, "%wrc", 2, INLINED)
declare_instruction(wrs,      "W1",       0, "%wrs", 2, INLINED)
declare_instruction(wrcd,     "W4",       0, "%wrcd", 2, INLINED)
declare_instruction(wrcw,     "W5",       0, "%wrcw", 2, INLINED)
declare_instruction(wrnl,     "W6",       0, "%wrnl", 1, INLINED)
declare_instruction(wrhw,     "W7",       0, "%wrhw", 2, INLINED)
declare_instruction(wriw,     "W8",       0, "%wriw", 2, INLINED)

/* serialization and deserialization instructions */
declare_instruction(igp,      "U0",       0, "integrable?",       '1', AUTOGL)
declare_instruction(fenc,     "U1",       0, "find-integrable-encoding", 2, AUTOGL)
declare_instruction(wrsi,     "U2",       0, "encode-integrable",  3, AUTOGL)
declare_instruction(rdsx,     "U3",       0, "deserialize-sexp",   1, AUTOGL)
declare_instruction(rdsc,     "U4",       0, "deserialize-code",   1, AUTOGL)
declare_instruction(iglk,     "U5",       0, "lookup-integrable", '1', AUTOGL)
declare_instruction(igty,     "U6",       0, "integrable-type",   '1', AUTOGL)
declare_instruction(iggl,     "U7",       0, "integrable-global", '1', AUTOGL)
declare_instruction(igco,     "U8",       0, "integrable-code",   '2', AUTOGL)

/* inlined integrables (no custom instructions) */
declare_integrable(NULL,      "aaa",      0, "caaar",             '1', AUTOGL)
declare_integrable(NULL,      "daa",      0, "caadr",             '1', AUTOGL)
declare_integrable(NULL,      "ada",      0, "cadar",             '1', AUTOGL)
declare_integrable(NULL,      "dda",      0, "caddr",             '1', AUTOGL)
declare_integrable(NULL,      "aad",      0, "cdaar",             '1', AUTOGL)
declare_integrable(NULL,      "dad",      0, "cdadr",             '1', AUTOGL)
declare_integrable(NULL,      "add",      0, "cddar",             '1', AUTOGL)
declare_integrable(NULL,      "ddd",      0, "cdddr",             '1', AUTOGL)
declare_integrable(NULL,      "aaaa",     0, "caaaar",            '1', AUTOGL)
declare_integrable(NULL,      "daaa",     0, "caaadr",            '1', AUTOGL)
declare_integrable(NULL,      "adaa",     0, "caadar",            '1', AUTOGL)
declare_integrable(NULL,      "ddaa",     0, "caaddr",            '1', AUTOGL)
declare_integrable(NULL,      "aada",     0, "cadaar",            '1', AUTOGL)
declare_integrable(NULL,      "dada",     0, "cadadr",            '1', AUTOGL)
declare_integrable(NULL,      "adda",     0, "caddar",            '1', AUTOGL)
declare_integrable(NULL,      "ddda",     0, "cadddr",            '1', AUTOGL)
declare_integrable(NULL,      "aaad",     0, "cdaaar",            '1', AUTOGL)
declare_integrable(NULL,      "daad",     0, "cdaadr",            '1', AUTOGL)
declare_integrable(NULL,      "adad",     0, "cdadar",            '1', AUTOGL)
declare_integrable(NULL,      "ddad",     0, "cdaddr",            '1', AUTOGL)
declare_integrable(NULL,      "aadd",     0, "cddaar",            '1', AUTOGL)
declare_integrable(NULL,      "dadd",     0, "cddadr",            '1', AUTOGL)
declare_integrable(NULL,      "addd",     0, "cdddar",            '1', AUTOGL)
declare_integrable(NULL,      "dddd",     0, "cddddr",            '1', AUTOGL)

/* globals */
declare_integrable(NULL,       NULL,   0, "%appl", 2, "%2_!K3")
declare_integrable(NULL,       NULL,   0, "%cwmv", 2, "%2_!K4")
declare_integrable(NULL,       NULL,   0, "%sdmv", -1, "K6")

#undef declare_instruction
#undef declare_instrshadow
#undef declare_integrable
