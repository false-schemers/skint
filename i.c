/* i.c -- instructions */

#include "n.h"
#include "i.h"

/* imports */
extern obj cx_continuation_2Dclosure_2Dcode;
extern obj cx__2Aglobals_2A;
extern obj cx__2Atransformers_2A;

#define istagged(o, t) istagged_inlined(o, t) 

/* forwards */
static struct intgtab_entry *intgtab_find_encoding(int sym, int arity);
static void wrs_integrable(int argc, struct intgtab_entry *pe, obj port);
static obj *rds_intgtab(obj *r, obj *sp, obj *hp);
static obj *rds_stox(obj *r, obj *sp, obj *hp);
static obj *rds_stoc(obj *r, obj *sp, obj *hp);
static obj *init_modules(obj *r, obj *sp, obj *hp);

/* platform-dependent optimizations */
#if defined(__clang__)
#define unlikely(x) __builtin_expect(x, 0)
#define likely(x)   __builtin_expect(x, 1)
#if __clang_major__ >= 13
#define VM_AC_IN_REG
#define outofline   __attribute__((noinline))
#define VM_MUSTTAIL_GUARANTEE
#define musttail    __attribute__((musttail))
#define regcall     __regcall        
#define noalias     restrict
#else
#define outofline
#define musttail    
#define regcall
#define noalias
#endif
/* #pragma GCC optimize ("align-functions=16") */
#define nochecks    __attribute__((no_stack_protector, aligned(8)))
/* __attribute__((nocf_check)) */
#define VM_INS_CODE_ALIGNED
#else
#define unlikely(x) (x)
#define likely(x)   (x)
#define outofline
#define musttail
#define regcall
#define noalias
#define nochecks
#endif

#ifdef VM_INS_CODE_ALIGNED /* direct representation */
#define ins_from_obj(x) ((ins_t)(x))
#else /* indirect representation (alignment needed) */
#define ins_from_obj(x) (*(ins_t*)(x))
#endif

#define fail(msg) do { ac = (obj)msg; musttail return cxi_fail(IARGS); } while (0)
#define failactype(msg) do { spush((obj)msg); musttail return cxi_failactype(IARGS); } while (0) 

/* instruction calling convention (platform-dependent) */
#ifdef VM_AC_IN_REG
#define ILOCALS       obj ac, *r, *ip, *sp, *hp
#define IPARAMS       obj ac, obj *noalias r, obj *noalias ip, obj *noalias sp, obj *noalias hp
#define IARGS         ac, r, ip, sp, hp
#define IARGS1        ac, r, ip+1, sp, hp
#define unload_ac()   (ra = ac)
#define reload_ac()   (ac = ra)
#else
#define ILOCALS       obj *r, *ip, *sp, *hp
#define IPARAMS       obj *r, obj *noalias ip, obj *noalias sp, obj *noalias hp
#define IARGS         r, ip, sp, hp
#define IARGS1        r, ip+1, sp, hp
#define ac ra
#define unload_ac()   (0)
#define reload_ac()   (0)
#endif

/* gc-safe obj registers; some of them used for saving/restoring during gc */
#define rk   (r[0])   /* #f or any non-false value for unwindi; run result */
#define ra   (r[1])   /* shadow reg for the accumulator (argc on call) */
#define rx   (r[2])   /* next instruction (index in closure's code) */
#define rd   (r[3])   /* current closure/display (vector of [0]=code, display) */
#define rs   (r[4])   /* shadow reg for stack pointer */

/* the rest of the register file is used as a stack */
#define VM_REGC       5    /* r[0] ... r[4] */
#define VM_STACK_LEN  1000 /* r[5] ... r[1004] */

/* vm closure representation */
#ifdef VMCLO_AS_VECTOR
#define isvmclo       isvector
#define vmcloref      vectorref
#define vmclolen      vectorlen
#define vmclobsz(c)   hbsz((c)+1)
#define hpushvmclo(c) (*--hp = obj_from_size(VECTOR_BTAG), hendblk((c)+1))
#else /* as procedure with code vector */
#define isvmclo       isprocedure
#define vmcloref      *procedureref
#define vmclolen      procedurelen
#define vmclobsz(c)   hbsz(c)
#define hpushvmclo(c) hendblk(c)
#endif

#define unload_ip()   (rx = obj_from_fixnum(ip - &vectorref(vmcloref(rd, 0), 0)))
#define reload_ip()   (ip = &vectorref(vmcloref(rd, 0), fixnum_from_obj(rx)))
#define unload_sp()   (rs = obj_from_fixnum(sp - r))
#define reload_sp()   (sp = r + fixnum_from_obj(rs))

#define sref(i)       (sp[-(i)-1])
#define dref(i)       (vmcloref(rd, (i)+1))
#define gref(p)       (cdr(p))
#define spush(o)      (*sp++ = o) /* todo: overflow check */
#define spop()        (*--sp)     /* todo: underflow check */
#define sdrop(n)      (sp -= (n)) /* todo: underflow check */
#define sgrow(n)      (sp += (n)) /* todo: overflow check */

#define hp_reserve(n) do { \
 if (unlikely(hp < cxg_heap + (n))) { \
   unload_ac(); unload_ip();   \
   hp = cxm_hgc(r, sp, hp, n); \
   reload_ac(); reload_ip();   \
 } } while (0)

/* cxi instructions protocol; retval is new hp: */
typedef obj* regcall (*ins_t)(IPARAMS);

/* calling next instruction and returning from instruction */
#define goih(ih)      musttail return ((ih)(IARGS))
#define goi(name)     musttail return (glue(cxi_, name)(IARGS))
#define gonexti()     musttail return ((ins_from_obj(*ip))(IARGS1))
#define unwindi(c)    return (unload_ac(), unload_ip(), rd = (c), hp)
#ifdef VM_MUSTTAIL_GUARANTEE
  #define callsubi()  reload_ip(); gonexti()
  #define retfromi()  reload_ip(); gonexti()
  #define trampcnd()  (0)
#else
  #define callsubi()  unload_sp(); return hp
  #define retfromi()  unload_sp(); return hp
  #define trampcnd()  (rd)
#endif


/* defining instruction helpers */
#define define_instrhelper(name) \
  static obj* regcall outofline nochecks name(IPARAMS)

/* defining and binding instructions */
#define define_instruction(name) \
  obj* regcall nochecks glue(cxi_, name)(IPARAMS)
#ifdef VM_INS_CODE_ALIGNED
#define declare_instruction_global(name) \
  obj glue(cx_ins_2D, name) = (obj)(&glue(cxi_, name));
#else
#define declare_instruction_global(name) \
  static ins_t glue(cxib_, name)[1] = { &glue(cxi_, name) }; \
  obj glue(cx_ins_2D, name) = (obj)(&glue(cxib_, name)[0]);
#endif

static obj vmhost(obj);
obj vmcases[8] = { 
  (obj)vmhost, (obj)vmhost, (obj)vmhost, (obj)vmhost, 
  (obj)vmhost, (obj)vmhost, (obj)vmhost, (obj)vmhost 
};
/* vmhost procedure */
static obj vmhost(obj pc)
{
  ILOCALS;
  int rc = cxg_rc;
  r = cxg_regs; hp = cxg_hp;
jump: 
  switch (objptr_from_obj(pc)-vmcases) {

  case 0: /* execute-thunk-closure */    
    /* r[0] = self, r[1] = k, r[2] = closure */
    { obj k, arg;
    assert(rc == 3);
    k = r[1]; arg = r[2];
    r = cxm_rgc(NULL, VM_REGC + VM_STACK_LEN);
    rk = k; /* continuation, kept there */
    ra = obj_from_fixnum(0); /* argc, shadow ac */
    rx = obj_from_fixnum(0); /* shadow ip */ 
    rd = arg; /* thunk closure to execute */
    rs = obj_from_fixnum(VM_REGC); /* sp */
    do { /* unwindi trampoline */
      reload_ac(); /* ra => ac */
      reload_ip(); /* rd/rx => ip */
      reload_sp(); /* rs => sp */
      hp = (ins_from_obj(*ip))(IARGS1);
    } while (trampcnd());
    /* r[0] = k, r[1] = result */
    r[2] = r[1];
    r[1] = obj_from_ktrap();
    pc = objptr_from_obj(r[0])[0];
    rc = 3;
    goto jump; }

  case 1: /* make-closure */    
    /* r[0] = clo, r[1] = k, r[2] = code */
    { assert(rc == 3);
    r[0] = r[1]; r[1] = r[2]; 
    /* r[0] = k; r[1] = code */
    hreserve(vmclobsz(1), 2); /* 2 live regs */
    *--hp = r[1];
    r[2] = hpushvmclo(1);
    r[1] = obj_from_ktrap();
    pc = objptr_from_obj(r[0])[0];
    rc = 3;
    goto jump; }

  case 2: /* decode-sexp */    
    /* r[0] = clo, r[1] = k, r[2] = xstr */
    { assert(rc == 3);
    r[0] = r[1]; r[1] = r[2]; 
    /* r[0] = k; r[1] = xstr */
    hp = rds_stox(r, r+2, hp); /* r[1] -> r[1] */
    r[2] = r[1]; r[1] = obj_from_ktrap();
    /* r[0] = k; r[1] = ek; r[2] = code */
    pc = objptr_from_obj(r[0])[0];
    rc = 3;
    goto jump; }

  case 3: /* decode-code */    
    /* r[0] = clo, r[1] = k, r[2] = cstr */
    { assert(rc == 3);
    r[0] = r[1]; r[1] = r[2]; 
    /* r[0] = k; r[1] = cstr */
    hp = rds_stoc(r, r+2, hp); /* r[1] -> r[1] */
    r[2] = r[1]; r[1] = obj_from_ktrap();
    /* r[0] = k; r[1] = ek; r[2] = code */
    pc = objptr_from_obj(r[0])[0];
    rc = 3;
    goto jump; }

  case 4: /* find-integrable-encoding */
    /* r[0] = clo, r[1] = k, r[2] = id, r[3] = argc */
    { assert(rc == 4);
    if (issymbol(r[2]) && is_fixnum_obj(r[3])) {
      int sym = getsymbol(r[2]), argc = fixnum_from_obj(r[3]);
      struct intgtab_entry *pe = intgtab_find_encoding(sym, argc);
      r[2] = (obj)pe;
    } else r[2] = 0;
    r[0] = r[1]; r[1] = obj_from_ktrap();
    pc = objptr_from_obj(r[0])[0];
    rc = 3;
    goto jump; }
      
  case 5: /* encode-integrable */
    /* r[0] = clo, r[1] = k, r[2] = argc, r[3] = pe, r[4] = port */
    { assert(rc == 5);
    if (is_fixnum_obj(r[2]) && isaptr(r[3]) && notobjptr(r[3]) && isoport(r[4])) {
      int argc = fixnum_from_obj(r[2]);
      wrs_integrable(argc, (struct intgtab_entry *)r[3], r[4]);
    } else assert(0);
    r[0] = r[1]; r[1] = obj_from_ktrap();
    pc = objptr_from_obj(r[0])[0];
    rc = 3;
    goto jump; }

  case 6: /* install-global-lambdas */
    /* r[0] = clo, r[1] = k */
    { assert(rc == 2);
    r[0] = r[1];
    hp = rds_intgtab(r, r+2, hp);
    r[1] = obj_from_ktrap();
    r[2] = obj_from_void(0);
    pc = objptr_from_obj(r[0])[0];
    rc = 3;
    goto jump; }

  case 7: /* initialize-modules */
    /* r[0] = clo, r[1] = k */
    { assert(rc == 2);
    r[0] = r[1];
    r = cxm_rgc(NULL, VM_REGC + VM_STACK_LEN);
    hp = init_modules(r, r+2, hp);
    r[1] = obj_from_ktrap();
    r[2] = obj_from_void(0);
    pc = objptr_from_obj(r[0])[0];
    rc = 3;
    goto jump; }
      
  default: /* inter-host call */
    cxg_hp = hp;
    cxm_rgc(r, 1);
    cxg_rc = rc;
    return pc;
  }
}


/* instructions for basic vm machinery */

define_instrhelper(cxi_fail) { 
  fprintf(stderr, "run-time failure: %s\n", (char*)ac); 
  unwindi(0); 
}

define_instrhelper(cxi_failactype) { 
  char *msg = (char*)spop(); obj p;
  fprintf(stderr, "run-time failure: argument is not a %s:\n", msg); 
  p = mkoport_file(sp-r, stderr); spush(p);
  oportputcircular(ac, p, 0);
  fputc('\n', stderr);
  spop();
  unwindi(0); 
}

define_instruction(halt) { unwindi(0); }

define_instruction(lit) { ac = *ip++; gonexti(); }

define_instruction(sref) { int i = fixnum_from_obj(*ip++); ac = sref(i); gonexti(); }  

define_instruction(dref) { int i = fixnum_from_obj(*ip++); ac = dref(i); gonexti(); }

define_instruction(gref) { obj p = *ip++; ac = gref(p); gonexti(); }

define_instruction(indirect) { ac = boxref(ac); gonexti(); }

define_instruction(dclose) {
  int i, n = fixnum_from_obj(*ip++), c = n+1; 
  hp_reserve(vmclobsz(c));
  for (i = n-1; i >= 0; --i) *--hp = sref(i); /* display */
  *--hp = *ip++; /* code */
  ac = hpushvmclo(c); /* closure */
  sdrop(n);   
  gonexti();
}

define_instruction(sbox) {
  int i = fixnum_from_obj(*ip++); 
  hp_reserve(hbsz(1+1));
  *--hp = sref(i);  
  *--hp = obj_from_size(BOX_BTAG); 
  sref(i) = hendblk(1+1);
  gonexti();
}

define_instruction(br) { int dx = fixnum_from_obj(*ip++); ip += dx; gonexti(); }

define_instruction(brt) { int dx = fixnum_from_obj(*ip++); if (ac) ip += dx; gonexti(); }

define_instruction(brnot) { int dx = fixnum_from_obj(*ip++); if (!ac) ip += dx; gonexti(); }

define_instruction(sseti) {
  int i = fixnum_from_obj(*ip++);
  boxref(sref(i)) = ac;
  gonexti();
}

define_instruction(dseti) {
  int i = fixnum_from_obj(*ip++);
  boxref(dref(i)) = ac;
  gonexti();
}

define_instruction(gset) {
  obj p = *ip++;
  gref(p) = ac;
  gonexti();
}

define_instruction(conti) {
  int n = (int)(sp - (r + VM_REGC));
  hp_reserve(vmclobsz(2)+hbsz(n+1));
  hp -= n; memcpy(hp, sp-n, n*sizeof(obj));
  *--hp = obj_from_size(VECTOR_BTAG);
  ac = hendblk(n+1); /* stack copy */   
  *--hp = ac;
  *--hp = cx_continuation_2Dclosure_2Dcode;
  ac = hpushvmclo(2); /* closure */   
  gonexti();
}

define_instruction(nuate) {
  obj v = dref(0); int n = vectorlen(v);
  assert((cxg_rend - cxg_regs - VM_REGC) > n);
  sp = r + VM_REGC; /* stack is empty */
  memcpy(sp, &vectorref(v, 0), n*sizeof(obj));
  sp += n; /* contains n elements now */
  gonexti();
}

define_instruction(appl) {
  int n, i; obj l = spop(), t = l;
  for (n = 0; ispair(t); t = cdr(t)) ++n; sgrow(n);
  for (i = 0; i < n; ++i, l = cdr(l)) sref(i) = car(l);
  rd = ac; rx = obj_from_fixnum(0);
  ac = obj_from_fixnum(n); /* argc */
  callsubi();
}

define_instruction(save) {
  int dx = fixnum_from_obj(*ip++); 
  spush(rd);
  spush(obj_from_fixnum(ip + dx - &vectorref(vmcloref(rd, 0), 0)));  
  gonexti();
}

define_instruction(push) { spush(ac); gonexti(); }

define_instruction(call) {
  int n = fixnum_from_obj(*ip++); 
  rd = ac; rx = obj_from_fixnum(0); 
  ac = obj_from_fixnum(n); /* argc */
  callsubi();
}

define_instruction(scall) {
  int m = fixnum_from_obj(*ip++), n = fixnum_from_obj(*ip++);
  rd = ac; rx = obj_from_fixnum(0); 
  ac = obj_from_fixnum(n); /* argc */
  memmove((void*)(sp-n-m), (void*)(sp-n), (size_t)n*sizeof(obj));
  sdrop(m);
  callsubi();
}

define_instruction(return) {
  rx = spop();
  rd = spop();
  retfromi();
}

define_instruction(sreturn) {
  int n = fixnum_from_obj(*ip++); 
  sdrop(n);
  rx = spop();
  rd = spop();
  retfromi();
}

define_instruction(adrop) {
  int n = fixnum_from_obj(*ip++); 
  sdrop(n);
  gonexti();
}

define_instruction(pop) { ac = spop(); gonexti(); }

define_instruction(atest) {
  obj no = *ip++;
  if (unlikely(ac != no)) fail("argument count error on entry");
  gonexti();
}

define_instruction(shrarg) {
  int n = fixnum_from_obj(*ip++), c = fixnum_from_obj(ac), m = c-n; 
  /* rest arg ends up on top */
  if (unlikely(m < 0)) {
    fail("argument count error on entry (<)");
  } else if (unlikely(m == 0)) {
    spush(mknull());
  } else {
    obj l = mknull();
    hp_reserve(hbsz(2+1)*m);
    while (m > 0) { 
      *--hp = l; *--hp = sref(n + m - 1);
      *--hp = obj_from_size(PAIR_BTAG); 
      l = hendblk(2+1); 
      --m;
    }
    memmove((void*)(sp-c), (void*)(sp-n), (size_t)n*sizeof(obj));
    sdrop(c-n);
    spush(l); 
  }
  /* ac = obj_from_fixnum(n+1); */
  gonexti();
}

define_instruction(shlit) { spush(ac); ac = *ip++; gonexti(); }
define_instruction(shi0)  { spush(ac); ac = obj_from_fixnum(0); gonexti(); }


/* type checks */

define_instruction(ckp) {
  if (likely(ispair(ac))) gonexti();
  failactype("pair");
}

define_instruction(ckl) {
  if (likely(islist(ac))) gonexti();
  failactype("list");
}

define_instruction(ckv) {
  if (likely(isvector(ac))) gonexti();
  failactype("vector");
}

define_instruction(ckc) {
  if (likely(ischar(ac))) gonexti();
  failactype("char");
}

define_instruction(cks) {
  if (likely(isstring(ac))) gonexti();
  failactype("string");
}

define_instruction(cki) {
  if (likely(is_fixnum_obj(ac))) gonexti();
  failactype("fixnum");
}

define_instruction(ckj) {
  if (likely(is_flonum_obj(ac))) gonexti();
  failactype("flonum");
}

define_instruction(ckn) {
  if (likely(is_fixnum_obj(ac) || is_flonum_obj(ac))) gonexti();
  failactype("number");
}

define_instruction(ckk) {
  if (likely(is_fixnum_obj(ac) && get_fixnum_unchecked(ac) >= 0)) gonexti();
  failactype("nonnegative fixnum");
}

define_instruction(cky) {
  if (likely(issymbol(ac))) gonexti();
  failactype("symbol");
}

define_instruction(ckr) {
  if (likely(isiport(ac))) gonexti();
  failactype("input port");
}

define_instruction(ckw) {
  if (likely(isoport(ac))) gonexti();
  failactype("output port");
}

define_instruction(ckx) {
  if (likely(isprocedure(ac))) gonexti(); // fixme?
  failactype("procedure");
}

define_instruction(cknj) {
  if (likely(is_flonum_obj(ac))) gonexti();
  if (!is_fixnum_obj(ac)) failactype("number");
  ac = obj_from_flonum(sp-r, (flonum_t)fixnum_from_obj(ac));
  gonexti();
}


/* integrable instructions */

define_instruction(isq) {
  obj x = ac, y = spop();
  ac = obj_from_bool(x == y);
  gonexti(); 
}

define_instruction(isv) {
  obj x = ac, y = spop();
  ac = obj_from_bool(iseqv(x,y));
  gonexti(); 
}

define_instruction(ise) {
  obj x = ac, y = spop();
  ac = obj_from_bool(isequal(x,y));
  gonexti(); 
}

define_instruction(car) { ac = car(ac); gonexti(); }
define_instruction(cdr) { ac = cdr(ac); gonexti(); }
define_instruction(caar) { ac = car(car(ac)); gonexti(); }
define_instruction(cadr) { ac = car(cdr(ac)); gonexti(); }
define_instruction(cdar) { ac = cdr(car(ac)); gonexti(); }
define_instruction(cddr) { ac = cdr(cdr(ac)); gonexti(); }

define_instruction(setcar) {
  car(ac) = spop();
  gonexti();
}

define_instruction(setcdr) {
  cdr(ac) = spop();
  gonexti();
}

define_instruction(nullp) {
  ac = obj_from_bool(isnull(ac));
  gonexti();
}

define_instruction(pairp) {
  ac = obj_from_bool(ispair(ac));
  gonexti();
}

define_instruction(cons) {
  hp_reserve(hbsz(2+1));
  *--hp = spop(); /* cdr */
  *--hp = ac;     /* car */
  *--hp = obj_from_size(PAIR_BTAG);
  ac = hendblk(2+1);
  gonexti();
}

define_instruction(listp) {
  ac = obj_from_bool(islist(ac));
  gonexti();
}

define_instruction(list) {
  int i, n = fixnum_from_obj(*ip++);
  hp_reserve(hbsz((2+1)*n));
  for (ac = mknull(), i = n-1; i >= 0; --i) {
    *--hp = ac;      /* cdr */
    *--hp = sref(i); /* car */
    *--hp = obj_from_size(PAIR_BTAG);
    ac = hendblk(2+1);
  }
  sdrop(n);
  gonexti();
}

define_instruction(llen) {
  int n = 0;
  while (ispair(ac)) { ac = cdr(ac); ++n; }
  ac = obj_from_fixnum(n);
  gonexti();
}

define_instruction(lget) {
  int c = fixnum_from_obj(spop());
  while (c-- > 0) ac = cdr(ac);
  ac = car(ac);
  gonexti();
}

define_instruction(lput) {
  int c = fixnum_from_obj(spop());
  while (c-- > 0) ac = cdr(ac);
  car(ac) = spop();
  gonexti();
}


define_instruction(memq) {
  obj l = spop(); 
  for (; l != mknull(); l = cdr(l)) {
    if (car(l) == ac) { ac = l; gonexti(); }
  }
  ac = 0;
  gonexti();
}

define_instruction(memv) {
  ac = ismemv(ac, spop());
  gonexti();
}

define_instruction(meme) {
  ac = ismember(ac, spop());
  gonexti();
}

define_instruction(assq) {
  obj l = spop(); 
  for (; l != mknull(); l = cdr(l)) {
    obj p = car(l); if (car(p) == ac) { ac = p; gonexti(); }
  }
  ac = 0;
  gonexti();
}

define_instruction(assv) {
  ac = isassv(ac, spop());
  gonexti();
}

define_instruction(asse) {
  ac = isassoc(ac, spop());
  gonexti();
}

define_instruction(ltail) {
  int c = fixnum_from_obj(spop());
  while (c-- > 0) ac = cdr(ac);
  gonexti();
}

define_instruction(lpair) {
  obj l = 0;
  while (ispair(ac)) { l = ac; ac = cdr(ac); }
  ac = l;
  gonexti();
}

define_instruction(lrev) {
  obj l = ac, o = mknull(); int n = 0;
  while (ispair(ac)) { ac = cdr(ac); ++n; }
  hp_reserve(hbsz(2+1)*n);
  for (ac = l; ac != mknull(); ac = cdr(ac)) { 
    *--hp = o; *--hp = car(ac);
    *--hp = obj_from_size(PAIR_BTAG); o = hendblk(2+1); 
  }
  ac = o;
  gonexti();
}

define_instruction(lrevi) {
  obj t, v = mknull();
  while (ac != mknull()) t = cdr(ac), cdr(ac) = v, v = ac, ac = t;
  ac = v;
  gonexti();
}

define_instruction(charp) {
  ac = obj_from_bool(ischar(ac));
  gonexti();
}

define_instruction(strp) {
  ac = obj_from_bool(isstring(ac));
  gonexti();
}

define_instruction(str) {
  int i, n = fixnum_from_obj(*ip++);
  obj o = hpushstr(sp-r, allocstring(n, ' '));
  unsigned char *s = (unsigned char *)stringchars(o);
  for (i = 0; i < n; ++i) s[i] = char_from_obj(sref(i));
  sdrop(n); ac = o;
  gonexti();
}

define_instruction(smk) {
  int n = fixnum_from_obj(ac), c = char_from_obj(spop());
  ac = hpushstr(sp-r, allocstring(n, c)); 
  gonexti();
}

define_instruction(slen) {
  ac = obj_from_fixnum(stringlen(ac));
  gonexti();
}

define_instruction(sget) {
  int i = fixnum_from_obj(spop());
  ac = obj_from_char(*stringref(ac, i));
  gonexti();
}

define_instruction(sput) {
  int i = fixnum_from_obj(spop());
  *stringref(ac, i) = char_from_obj(spop());
  gonexti();
}

define_instruction(scat) {
  obj x = ac, y = spop();
  int *d = stringcat(stringdata(x), stringdata(y));
  ac = hpushstr(sp-r, d);
  gonexti();
}

define_instruction(ssub) {
  int is = fixnum_from_obj(spop());
  int ie = fixnum_from_obj(spop());
  int *d = substring(stringdata(ac), is, ie);
  ac = hpushstr(sp-r, d);
  gonexti();
}

define_instruction(vecp) {
  ac = obj_from_bool(isvector(ac));
  gonexti();
}

define_instruction(vec) {
  int i, n = fixnum_from_obj(*ip++);
  hp_reserve(hbsz(n+1));
  for (i = n-1; i >= 0; --i) *--hp = sref(i);
  *--hp = obj_from_size(VECTOR_BTAG);
  ac = hendblk(n+1);
  sdrop(n);
  gonexti();
}

define_instruction(vmk) {
  int i, n = fixnum_from_obj(ac); obj v = sref(0);
  hp_reserve(hbsz(n+1));
  for (i = 0; i < n; ++i) *--hp = v;
  *--hp = obj_from_size(VECTOR_BTAG);
  ac = hendblk(n+1);
  sdrop(1);
  gonexti();
}

define_instruction(vlen) {
  ac = obj_from_fixnum(vectorlen(ac));
  gonexti();
}

define_instruction(vget) {
  int i = fixnum_from_obj(spop());
  ac = vectorref(ac, i);
  gonexti();
}

define_instruction(vput) {
  int i = fixnum_from_obj(spop());
  vectorref(ac, i) = spop();
  gonexti();
}

define_instruction(vcat) {
  int n1 = vectorlen(ac), n2 = vectorlen(sref(0)), n = n1 + n2;
  hp_reserve(hbsz(n+1));
  hp -= n2; memcpy(hp, &vectorref(spop(), 0), n2*sizeof(obj));
  hp -= n1; memcpy(hp, &vectorref(ac, 0), n1*sizeof(obj));
  *--hp = obj_from_size(VECTOR_BTAG);
  ac = hendblk(n+1);
  gonexti();
}

define_instruction(vtol) {
  int n = vectorlen(ac);
  obj l = mknull();
  hp_reserve(hbsz((2+1)*n));
  while (n > 0) {
    *--hp = l; *--hp = vectorref(ac, n-1);
    *--hp = obj_from_size(PAIR_BTAG);
    l = hendblk(2+1);
    --n;
  }
  ac = l;
  gonexti();
}

define_instruction(ltov) {
  obj l = ac; int n = 0, i;
  while (ispair(l)) { l = cdr(l); ++n; }
  hp_reserve(hbsz(n+1));
  for (l = ac, i = 0, hp -= n; i < n; ++i, l = cdr(l)) hp[i] = car(l);
  *--hp = obj_from_size(VECTOR_BTAG);
  ac = hendblk(n+1);
  gonexti();
}

define_instruction(stol) {
  int n = stringlen(ac);
  obj l = mknull();
  hp_reserve(hbsz((2+1)*n));
  while (n > 0) {
    *--hp = l; *--hp = obj_from_char(*stringref(ac, n-1));
    *--hp = obj_from_size(PAIR_BTAG);
    l = hendblk(2+1);
    --n;
  }
  ac = l;
  gonexti();
}

define_instruction(ltos) {
  obj l = ac; int n = 0, i, *d;
  while (ispair(l)) { l = cdr(l); ++n; }
  d = allocstring(n, ' ');
  for (i = 0; i < n; ac = cdr(ac), ++i)
    sdatachars(d)[i] = char_from_obj(car(ac));
  ac = hpushstr(sp-r, d);
  gonexti();
}

define_instruction(ytos) {
  ac = hpushstr(sp-r, newstring(symbolname(getsymbol(ac))));
  gonexti();
}

define_instruction(stoy) {
  ac = mksymbol(internsym(stringchars(ac)));
  gonexti();
}

define_instruction(itos) {
  char buf[35], *s = buf + sizeof(buf) - 1; 
  int neg = 0;
  long num = fixnum_from_obj(ac);
  long radix = fixnum_from_obj(spop());
  if (num < 0) { neg = 1; num = -num; }
  *s = 0;
  do { int d = num % radix; *--s = d < 10 ? d + '0' : d - 10 + 'a'; }
  while (num /= radix);
  if (neg) *--s = '-';
  ac = hpushstr(sp-r, newstring(s));  
  gonexti();
}

define_instruction(stoi) {
  char *e, *s = stringchars(ac);
  int radix = fixnum_from_obj(spop()); long l;
  if (s[0] == '#' && (s[1] == 'b' || s[1] == 'B')) s += 2, radix = 2;
  else if (s[0] == '#' && (s[1] == 'o' || s[1] == 'O')) s += 2, radix = 8;
  else if (s[0] == '#' && (s[1] == 'd' || s[1] == 'D')) s += 2, radix = 10;
  else if (s[0] == '#' && (s[1] == 'x' || s[1] == 'X')) s += 2, radix = 16;
  l = (errno = 0, strtol(s, &e, radix));
  if (errno || l < FIXNUM_MIN || l > FIXNUM_MAX || e == s || *e) ac = obj_from_bool(0);
  else ac = obj_from_fixnum(l);
  gonexti();
}

define_instruction(ctoi) {
  ac = obj_from_fixnum(char_from_obj(ac));
  gonexti();
}

define_instruction(itoc) {
  ac = obj_from_char(fixnum_from_obj(ac));
  gonexti();
}

define_instruction(jtos) {
  char buf[30], *s; double d = flonum_from_obj(ac); sprintf(buf, "%.15g", d);
  for (s = buf; *s != 0; s++) if (strchr(".eE", *s)) break;
  if (d != d) strcpy(buf, "+nan.0"); else if (d <= -HUGE_VAL) strcpy(buf, "-inf.0");
  else if (d >= HUGE_VAL) strcpy(buf, "+inf.0"); else if (*s == 'E') *s = 'e'; 
  else if (*s == 0) {  *s++ = '.'; *s++ = '0'; *s = 0; }
  ac = hpushstr(sp-r, newstring(buf));
  gonexti();
}

define_instruction(stoj) {
  char *e = "", *s = stringchars(ac); double d; errno = 0;
  if (*s != '+' && *s != '-') d = strtod(s, &e);
  else if (strcmp_ci(s+1, "inf.0") == 0) d = (*s == '-' ? -HUGE_VAL : HUGE_VAL); 
  else if (strcmp_ci(s+1, "nan.0") == 0) d = HUGE_VAL - HUGE_VAL; 
  else d = strtod(s, &e);
  if (errno || e == s || *e) ac = obj_from_bool(0);
  else ac = obj_from_flonum(sp-r, d);
  gonexti();
}

define_instruction(ntos) {
  if (is_fixnum_obj(ac)) goi(itos);
  if (unlikely(spop() != obj_from_fixnum(10))) fail("non-10 radix in flonum conversion");
  goi(jtos);
}

define_instruction(ston) {
  char *s = stringchars(ac);
  int radix = fixnum_from_obj(spop()); long l; double d;
  switch (strtofxfl(s, radix, &l, &d)) {
    case 'e': ac = obj_from_fixnum(l); break;
    case 'i': ac = obj_from_flonum(sp-r, d); break;
    default : ac = obj_from_bool(0); break;
  }
  gonexti();
}

define_instruction(not) {
  ac = obj_from_bool(ac == 0);
  gonexti();
}

define_instruction(izerop) {
  ac = obj_from_bool(ac == obj_from_fixnum(0));
  gonexti();
}

define_instruction(iposp) {
  ac = obj_from_bool(fixnum_from_obj(ac) > 0);
  gonexti(); 
}

define_instruction(inegp) {
  ac = obj_from_bool(fixnum_from_obj(ac) < 0);
  gonexti(); 
}

define_instruction(ievnp) {
  ac = obj_from_bool((fixnum_from_obj(ac) & 1) == 0);
  gonexti(); 
}

define_instruction(ioddp) {
  ac = obj_from_bool((fixnum_from_obj(ac) & 1) != 0);
  gonexti(); 
}

define_instruction(iadd) {
  obj x = ac, y = spop();
  ac = obj_from_fixnum(fxadd(fixnum_from_obj(x), fixnum_from_obj(y)));
  gonexti(); 
}

define_instruction(isub) {
  obj x = ac, y = spop();
  ac = obj_from_fixnum(fxsub(fixnum_from_obj(x), fixnum_from_obj(y)));
  gonexti(); 
}

define_instruction(imul) {
  obj x = ac, y = spop();
  ac = obj_from_fixnum(fxmul(fixnum_from_obj(x), fixnum_from_obj(y)));
  gonexti();
}

define_instruction(iquo) {
  obj x = ac, y = spop();
  if (y == obj_from_fixnum(0)) fail("division by zero");
  ac = obj_from_fixnum(fxquo(fixnum_from_obj(x), fixnum_from_obj(y)));
  gonexti();
}

define_instruction(irem) {
  obj x = ac, y = spop();
  if (y == obj_from_fixnum(0)) fail("division by zero");
  ac = obj_from_fixnum(fxrem(fixnum_from_obj(x), fixnum_from_obj(y)));
  gonexti();
}

define_instruction(imin) {
  obj x = ac, y = spop();
  ac = fixnum_from_obj(x) < fixnum_from_obj(y) ? x : y;
  gonexti(); 
}

define_instruction(imax) {
  obj x = ac, y = spop();
  ac = fixnum_from_obj(x) > fixnum_from_obj(y) ? x : y;
  gonexti(); 
}

define_instruction(ineg) {
  ac = obj_from_fixnum(fxneg(fixnum_from_obj(ac)));
  gonexti();
}

define_instruction(iabs) {
  ac = obj_from_fixnum(fxabs(fixnum_from_obj(ac)));
  gonexti();
}

define_instruction(itoj) {
  ac = obj_from_flonum(sp-r, (double)fixnum_from_obj(ac));
  gonexti();
}

define_instruction(idiv) {
  obj x = ac, y = spop();
  if (y == obj_from_fixnum(0)) fail("division by zero");
  ac = obj_from_fixnum(fxdiv(fixnum_from_obj(x), fixnum_from_obj(y)));
  gonexti();
}

define_instruction(imqu) {
  obj x = ac, y = spop();
  if (y == obj_from_fixnum(0)) fail("division by zero");
  ac = obj_from_fixnum(fxmqu(fixnum_from_obj(x), fixnum_from_obj(y)));
  gonexti();
}

define_instruction(imlo) {
  obj x = ac, y = spop();
  if (y == obj_from_fixnum(0)) fail("division by zero");
  ac = obj_from_fixnum(fxmlo(fixnum_from_obj(x), fixnum_from_obj(y)));
  gonexti();
}

define_instruction(ieuq) {
  obj x = ac, y = spop();
  if (y == obj_from_fixnum(0)) fail("division by zero");
  ac = obj_from_fixnum(fxeuq(fixnum_from_obj(x), fixnum_from_obj(y)));
  gonexti();
}

define_instruction(ieur) {
  obj x = ac, y = spop();
  if (y == obj_from_fixnum(0)) fail("division by zero");
  ac = obj_from_fixnum(fxeur(fixnum_from_obj(x), fixnum_from_obj(y)));
  gonexti();
}

define_instruction(igcd) {
  obj x = ac, y = spop();
  ac = obj_from_fixnum(fxgcd(fixnum_from_obj(x), fixnum_from_obj(y)));
  gonexti();
}

define_instruction(ipow) {
  obj x = ac, y = spop();
  ac = obj_from_fixnum(fxpow(fixnum_from_obj(x), fixnum_from_obj(y)));
  gonexti();
}

define_instruction(isqrt) {
  ac = obj_from_fixnum(fxsqrt(fixnum_from_obj(ac)));
  gonexti();
}

define_instruction(inot) {
  ac = obj_from_fixnum(~fixnum_from_obj(ac));
  gonexti();
}

define_instruction(iand) {
  obj x = ac, y = spop();
  ac = obj_from_fixnum(fixnum_from_obj(x) & fixnum_from_obj(y));
  gonexti();
}

define_instruction(iior) {
  obj x = ac, y = spop();
  ac = obj_from_fixnum(fixnum_from_obj(x) | fixnum_from_obj(y));
  gonexti();
}

define_instruction(ixor) {
  obj x = ac, y = spop();
  ac = obj_from_fixnum(fixnum_from_obj(x) ^ fixnum_from_obj(y));
  gonexti();
}

define_instruction(iasl) {
  obj x = ac, y = spop();
  ac = obj_from_fixnum(fxasl(fixnum_from_obj(x), fixnum_from_obj(y)));
  gonexti();
}

define_instruction(iasr) {
  obj x = ac, y = spop();
  ac = obj_from_fixnum(fxasr(fixnum_from_obj(x), fixnum_from_obj(y)));
  gonexti();
}

define_instruction(ilt) {
  obj x = ac, y = spop();
  ac = obj_from_bool(fixnum_from_obj(x) < fixnum_from_obj(y));
  gonexti(); 
}

define_instruction(igt) {
  obj x = ac, y = spop();
  ac = obj_from_bool(fixnum_from_obj(x) > fixnum_from_obj(y));
  gonexti(); 
}

define_instruction(ile) {
  obj x = ac, y = spop();
  ac = obj_from_bool(fixnum_from_obj(x) <= fixnum_from_obj(y));
  gonexti(); 
}

define_instruction(ige) {
  obj x = ac, y = spop();
  ac = obj_from_bool(fixnum_from_obj(x) >= fixnum_from_obj(y));
  gonexti(); 
}

define_instruction(ieq) {
  obj x = ac, y = spop();
  ac = obj_from_bool(x == y);
  gonexti(); 
}

define_instruction(ine) {
  obj x = ac, y = spop();
  ac = obj_from_bool(x != y);
  gonexti(); 
}


define_instruction(jzerop) {
  ac = obj_from_bool(flonum_from_obj(ac) == 0.0);
  gonexti();
}

define_instruction(jposp) {
  ac = obj_from_bool(flonum_from_obj(ac) > 0.0);
  gonexti(); 
}

define_instruction(jnegp) {
  ac = obj_from_bool(flonum_from_obj(ac) < 0.0);
  gonexti(); 
}

define_instruction(jevnp) {
  double f = flonum_from_obj(ac);
  ac = obj_from_bool(flisint(f / 2.0));
  gonexti(); 
}

define_instruction(joddp) {
  double f = flonum_from_obj(ac);
  ac = obj_from_bool(flisint((f + 1.0) / 2.0));
  gonexti(); 
}

define_instruction(jintp) {
  double f = flonum_from_obj(ac);
  ac = obj_from_bool(f > -HUGE_VAL && f < HUGE_VAL && f == floor(f));
  gonexti(); 
}

define_instruction(jnanp) {
  double f = flonum_from_obj(ac);
  ac = obj_from_bool(f != f);
  gonexti(); 
}

define_instruction(jfinp) {
  double f = flonum_from_obj(ac);
  ac = obj_from_bool(f > -HUGE_VAL && f < HUGE_VAL);
  gonexti(); 
}

define_instruction(jinfp) {
  double f = flonum_from_obj(ac);
  ac = obj_from_bool(f <= -HUGE_VAL || f >= HUGE_VAL);
  gonexti(); 
}

define_instruction(jadd) {
  obj x = ac, y = spop();
  ac = obj_from_flonum(sp-r, flonum_from_obj(x) + flonum_from_obj(y));
  gonexti(); 
}

define_instruction(jsub) {
  obj x = ac, y = spop();
  ac = obj_from_flonum(sp-r, flonum_from_obj(x) - flonum_from_obj(y));
  gonexti(); 
}

define_instruction(jmul) {
  obj x = ac, y = spop();
  ac = obj_from_flonum(sp-r, flonum_from_obj(x) * flonum_from_obj(y));
  gonexti();
}

define_instruction(jdiv) {
  obj x = ac, y = spop();
  ac = obj_from_flonum(sp-r, flonum_from_obj(x) / flonum_from_obj(y));
  gonexti();
}

define_instruction(jrem) {
  obj x = ac, y = spop();
  ac = obj_from_flonum(sp-r, flrem(flonum_from_obj(x), flonum_from_obj(y)));
  gonexti();
}

define_instruction(jlt) {
  obj x = ac, y = spop();
  ac = obj_from_bool(flonum_from_obj(x) < flonum_from_obj(y));
  gonexti(); 
}

define_instruction(jgt) {
  obj x = ac, y = spop();
  ac = obj_from_bool(flonum_from_obj(x) > flonum_from_obj(y));
  gonexti(); 
}

define_instruction(jle) {
  obj x = ac, y = spop();
  ac = obj_from_bool(flonum_from_obj(x) <= flonum_from_obj(y));
  gonexti(); 
}

define_instruction(jge) {
  obj x = ac, y = spop();
  ac = obj_from_bool(flonum_from_obj(x) >= flonum_from_obj(y));
  gonexti(); 
}

define_instruction(jeq) {
  obj x = ac, y = spop();
  ac = obj_from_bool(flonum_from_obj(x) == flonum_from_obj(y));
  gonexti(); 
}

define_instruction(jne) {
  obj x = ac, y = spop();
  ac = obj_from_bool(flonum_from_obj(x) != flonum_from_obj(y));
  gonexti(); 
}

define_instruction(jmin) {
  obj x = ac, y = spop();
  ac = flonum_from_obj(x) < flonum_from_obj(y) ? x : y;
  gonexti(); 
}

define_instruction(jmax) {
  obj x = ac, y = spop();
  ac = flonum_from_obj(x) > flonum_from_obj(y) ? x : y;
  gonexti(); 
}

define_instruction(jneg) {
  ac = obj_from_flonum(sp-r, -flonum_from_obj(ac));
  gonexti(); 
}

define_instruction(jabs) {
  ac = obj_from_flonum(sp-r, fabs(flonum_from_obj(ac)));
  gonexti(); 
}

define_instruction(jtoi) {
  ac = obj_from_fixnum(fxflo(flonum_from_obj(ac)));
  gonexti();
}

define_instruction(jquo) {
  obj x = ac, y = spop();
  ac = obj_from_flonum(sp-r, flquo(flonum_from_obj(x), flonum_from_obj(y)));
  gonexti(); 
}

define_instruction(jmqu) {
  obj x = ac, y = spop();
  ac = obj_from_flonum(sp-r, flmqu(flonum_from_obj(x), flonum_from_obj(y)));
  gonexti(); 
}

define_instruction(jmlo) {
  obj x = ac, y = spop();
  ac = obj_from_flonum(sp-r, flmlo(flonum_from_obj(x), flonum_from_obj(y)));
  gonexti(); 
}

define_instruction(jfloor) {
  ac = obj_from_flonum(sp-r, floor(flonum_from_obj(ac)));
  gonexti(); 
}

define_instruction(jceil) {
  ac = obj_from_flonum(sp-r, ceil(flonum_from_obj(ac)));
  gonexti(); 
}

define_instruction(jtrunc) {
  double f = flonum_from_obj(ac), i;
  modf(f,  &i);
  ac = obj_from_flonum(sp-r, i);
  gonexti(); 
}

define_instruction(jround) {
  ac = obj_from_flonum(sp-r, flround(flonum_from_obj(ac)));
  gonexti(); 
}


define_instruction(zerop) {
  obj x = ac;
  if (likely(is_fixnum_obj(x))) {
    ac = obj_from_bool(x == obj_from_fixnum(0));
  } else {
    ac = obj_from_bool(flonum_from_obj(x) == 0.0);
  }
  gonexti();
}

define_instruction(posp) {
  if (likely(is_fixnum_obj(ac))) ac = obj_from_bool(get_fixnum_unchecked(ac) > 0);
  else ac = obj_from_bool(flonum_from_obj(ac) > 0.0);
  gonexti(); 
}

define_instruction(negp) {
  if (likely(is_fixnum_obj(ac))) ac = obj_from_bool(get_fixnum_unchecked(ac) < 0);
  else ac = obj_from_bool(flonum_from_obj(ac) < 0.0);
  gonexti(); 
}

define_instruction(add) {
  obj x = ac, y = spop();
  if (likely(are_fixnum_objs(x, y))) {
    ac = obj_from_fixnum(fxadd(get_fixnum_unchecked(x), get_fixnum_unchecked(y)));
  } else {
    double dx, dy;
    if (likely(is_flonum_obj(x))) dx = flonum_from_obj(x);
    else dx = (double)fixnum_from_obj(x);
    if (likely(is_flonum_obj(y))) dy = flonum_from_obj(y);
    else dy = (double)fixnum_from_obj(y);
    ac = obj_from_flonum(sp-r, dx + dy);
  }
  gonexti(); 
}

define_instruction(sub) {
  obj x = ac, y = spop();
  if (likely(are_fixnum_objs(x, y))) {
    ac = obj_from_fixnum(fxsub(get_fixnum_unchecked(x), get_fixnum_unchecked(y)));
  } else {
    double dx, dy;
    if (likely(is_flonum_obj(x))) dx = flonum_from_obj(x);
    else dx = (double)fixnum_from_obj(x);
    if (likely(is_flonum_obj(y))) dy = flonum_from_obj(y);
    else dy = (double)fixnum_from_obj(y);
    ac = obj_from_flonum(sp-r, dx - dy);
  }
  gonexti(); 
}

define_instruction(mul) {
  obj x = ac, y = spop();
  if (likely(are_fixnum_objs(x, y))) {
    ac = obj_from_fixnum(fxmul(get_fixnum_unchecked(x), get_fixnum_unchecked(y)));
  } else {
    double dx, dy;
    if (likely(is_flonum_obj(x))) dx = flonum_from_obj(x);
    else dx = (double)fixnum_from_obj(x);
    if (likely(is_flonum_obj(y))) dy = flonum_from_obj(y);
    else dy = (double)fixnum_from_obj(y);
    ac = obj_from_flonum(sp-r, dx * dy);
  }
  gonexti(); 
}

define_instruction(div) {
  obj x = ac, y = spop();
  if (likely(are_fixnum_objs(x, y))) {
    long i; double d;
    if (unlikely(y == obj_from_fixnum(0))) fail("division by zero");
    if (fxifdv(get_fixnum_unchecked(x), get_fixnum_unchecked(y), &i, &d)) ac = obj_from_fixnum(i);
    else obj_from_flonum(sp-r, d);
  } else {
    double dx, dy;
    if (likely(is_flonum_obj(x))) dx = flonum_from_obj(x);
    else dx = (double)fixnum_from_obj(x);
    if (likely(is_flonum_obj(y))) dy = flonum_from_obj(y);
    else dy = (double)fixnum_from_obj(y);
    ac = obj_from_flonum(sp-r, dx / dy);
  }
  gonexti(); 
}

define_instruction(quo) {
  obj x = ac, y = spop();
  if (likely(are_fixnum_objs(x, y))) {
    if (unlikely(y == obj_from_fixnum(0))) fail("division by zero");
    ac = obj_from_fixnum(fxquo(get_fixnum_unchecked(x), get_fixnum_unchecked(y)));
  } else {
    double dx, dy;
    if (likely(is_flonum_obj(x))) dx = flonum_from_obj(x);
    else dx = (double)fixnum_from_obj(x);
    if (likely(is_flonum_obj(y))) dy = flonum_from_obj(y);
    else dy = (double)fixnum_from_obj(y);
    ac = obj_from_flonum(sp-r, flquo(dx, dy));
  }
  gonexti(); 
}

define_instruction(rem) {
  obj x = ac, y = spop();
  if (likely(are_fixnum_objs(x, y))) {
    if (unlikely(y == obj_from_fixnum(0))) fail("division by zero");
    ac = obj_from_fixnum(fxrem(get_fixnum_unchecked(x), get_fixnum_unchecked(y)));
  } else {
    double dx, dy;
    if (likely(is_flonum_obj(x))) dx = flonum_from_obj(x);
    else dx = (double)fixnum_from_obj(x);
    if (likely(is_flonum_obj(y))) dy = flonum_from_obj(y);
    else dy = (double)fixnum_from_obj(y);
    ac = obj_from_flonum(sp-r, flrem(dx, dy));
  }
  gonexti(); 
}

define_instruction(mqu) {
  obj x = ac, y = spop();
  if (likely(are_fixnum_objs(x, y))) {
    if (unlikely(y == obj_from_fixnum(0))) fail("division by zero");
    ac = obj_from_fixnum(fxmqu(get_fixnum_unchecked(x), get_fixnum_unchecked(y)));
  } else {
    double dx, dy;
    if (likely(is_flonum_obj(x))) dx = flonum_from_obj(x);
    else dx = (double)fixnum_from_obj(x);
    if (likely(is_flonum_obj(y))) dy = flonum_from_obj(y);
    else dy = (double)fixnum_from_obj(y);
    ac = obj_from_flonum(sp-r, flmqu(dx, dy));
  }
  gonexti(); 
}

define_instruction(mlo) {
  obj x = ac, y = spop();
  if (likely(are_fixnum_objs(x, y))) {
    if (unlikely(y == obj_from_fixnum(0))) fail("division by zero");
    ac = obj_from_fixnum(fxmlo(get_fixnum_unchecked(x), get_fixnum_unchecked(y)));
  } else {
    double dx, dy;
    if (likely(is_flonum_obj(x))) dx = flonum_from_obj(x);
    else dx = (double)fixnum_from_obj(x);
    if (likely(is_flonum_obj(y))) dy = flonum_from_obj(y);
    else dy = (double)fixnum_from_obj(y);
    ac = obj_from_flonum(sp-r, flmlo(dx, dy));
  }
  gonexti(); 
}


define_instruction(lt) {
  obj x = ac, y = spop();
  if (likely(are_fixnum_objs(x, y))) {
    ac = obj_from_bool(get_fixnum_unchecked(x) < get_fixnum_unchecked(y));
  } else {
    double dx, dy;
    if (likely(is_flonum_obj(x))) dx = flonum_from_obj(x);
    else dx = (double)fixnum_from_obj(x);
    if (likely(is_flonum_obj(y))) dy = flonum_from_obj(y);
    else dy = (double)fixnum_from_obj(y);
    ac = obj_from_bool(dx < dy);
  }
  gonexti(); 
}

define_instruction(gt) {
  obj x = ac, y = spop();
  if (likely(are_fixnum_objs(x, y))) {
    ac = obj_from_bool(get_fixnum_unchecked(x) > get_fixnum_unchecked(y));
  } else {
    double dx, dy;
    if (likely(is_flonum_obj(x))) dx = flonum_from_obj(x);
    else dx = (double)fixnum_from_obj(x);
    if (likely(is_flonum_obj(y))) dy = flonum_from_obj(y);
    else dy = (double)fixnum_from_obj(y);
    ac = obj_from_bool(dx > dy);
  }
  gonexti(); 
}

define_instruction(le) {
  obj x = ac, y = spop();
  if (likely(are_fixnum_objs(x, y))) {
    ac = obj_from_bool(get_fixnum_unchecked(x) <= get_fixnum_unchecked(y));
  } else {
    double dx, dy;
    if (likely(is_flonum_obj(x))) dx = flonum_from_obj(x);
    else dx = (double)fixnum_from_obj(x);
    if (likely(is_flonum_obj(y))) dy = flonum_from_obj(y);
    else dy = (double)fixnum_from_obj(y);
    ac = obj_from_bool(dx <= dy);
  }
  gonexti(); 
}

define_instruction(ge) {
  obj x = ac, y = spop();
  if (likely(are_fixnum_objs(x, y))) {
    ac = obj_from_bool(get_fixnum_unchecked(x) >= get_fixnum_unchecked(y));
  } else {
    double dx, dy;
    if (likely(is_flonum_obj(x))) dx = flonum_from_obj(x);
    else dx = (double)fixnum_from_obj(x);
    if (likely(is_flonum_obj(y))) dy = flonum_from_obj(y);
    else dy = (double)fixnum_from_obj(y);
    ac = obj_from_bool(dx >= dy);
  }
  gonexti(); 
}

define_instruction(eq) {
  obj x = ac, y = spop();
  if (x == y) ac = obj_from_bool(1);
  else if (is_flonum_obj(x) || is_flonum_obj(y)) {
    double dx, dy;
    if (likely(is_flonum_obj(x))) dx = flonum_from_obj(x);
    else dx = (double)fixnum_from_obj(x);
    if (likely(is_flonum_obj(y))) dy = flonum_from_obj(y);
    else dy = (double)fixnum_from_obj(y);
    ac = obj_from_bool(dx == dy);
  } else ac = obj_from_bool(0);
  gonexti(); 
}

define_instruction(ne) {
  obj x = ac, y = spop();
  if (x == y) ac = obj_from_bool(0);
  else if (is_flonum_obj(x) || is_flonum_obj(y)) {
    double dx, dy;
    if (likely(is_flonum_obj(x))) dx = flonum_from_obj(x);
    else dx = (double)fixnum_from_obj(x);
    if (likely(is_flonum_obj(y))) dy = flonum_from_obj(y);
    else dy = (double)fixnum_from_obj(y);
    ac = obj_from_bool(dx != dy);
  } else ac = obj_from_bool(1);
  gonexti(); 
}

define_instruction(min) {
  obj x = ac, y = spop();
  if (likely(are_fixnum_objs(x, y))) {
    ac = (get_fixnum_unchecked(x) < get_fixnum_unchecked(y)) ? x : y;
  } else {
    double dx, dy;
    if (likely(is_flonum_obj(x))) dx = flonum_from_obj(x);
    else dx = (double)fixnum_from_obj(x);
    if (likely(is_flonum_obj(y))) dy = flonum_from_obj(y);
    else dy = (double)fixnum_from_obj(y);
    ac = dx < dy ? obj_from_flonum(sp-r, dx) : obj_from_flonum(sp-r, dy);
  }
  gonexti(); 
}

define_instruction(max) {
  obj x = ac, y = spop();
  if (likely(are_fixnum_objs(x, y))) {
    ac = (get_fixnum_unchecked(x) > get_fixnum_unchecked(y)) ? x : y;
  } else {
    double dx, dy;
    if (likely(is_flonum_obj(x))) dx = flonum_from_obj(x);
    else dx = (double)fixnum_from_obj(x);
    if (likely(is_flonum_obj(y))) dy = flonum_from_obj(y);
    else dy = (double)fixnum_from_obj(y);
    ac = dx > dy ? obj_from_flonum(sp-r, dx) : obj_from_flonum(sp-r, dy);
  }
  gonexti(); 
}


define_instruction(neg) {
  if (likely(is_fixnum_obj(ac))) {
    ac = obj_from_fixnum(-fixnum_from_obj(ac));
  } else {
    ac = obj_from_flonum(sp-r, -flonum_from_obj(ac));
  }
  gonexti(); 
}

define_instruction(abs) {
  if (likely(is_fixnum_obj(ac))) {
    ac = obj_from_fixnum(fxabs(fixnum_from_obj(ac)));
  } else {
    ac = obj_from_flonum(sp-r, fabs(flonum_from_obj(ac)));
  }
  gonexti(); 
}


define_instruction(nump) {
  ac = obj_from_bool(is_fixnum_obj(ac) || is_flonum_obj(ac));
  gonexti(); 
}

define_instruction(fixp) {
  ac = obj_from_bool(is_fixnum_obj(ac));
  gonexti(); 
}

define_instruction(flop) {
  ac = obj_from_bool(is_flonum_obj(ac));
  gonexti(); 
}

define_instruction(intp) {
  if (likely(is_fixnum_obj(ac))) {
    ac = obj_from_bool(1);
  } else { /* accepts any object! */
    ac = obj_from_bool(is_flonum_obj(ac) && flisint(flonum_from_obj(ac)));
  }
  gonexti(); 
}

define_instruction(nanp) {
  if (unlikely(is_fixnum_obj(ac))) {
    ac = obj_from_bool(0);
  } else {
    double f = flonum_from_obj(ac);
    ac = obj_from_bool(f != f);
  }
  gonexti(); 
}

define_instruction(finp) {
  if (unlikely(is_fixnum_obj(ac))) {
    ac = obj_from_bool(1);
  } else {
    double f = flonum_from_obj(ac);
    ac = obj_from_bool(f > -HUGE_VAL && f < HUGE_VAL);
  }
  gonexti(); 
}

define_instruction(infp) {
  if (unlikely(is_fixnum_obj(ac))) {
    ac = obj_from_bool(0);
  } else {
    double f = flonum_from_obj(ac);
    ac = obj_from_bool(f <= -HUGE_VAL || f >= HUGE_VAL);
  }
  gonexti(); 
}

define_instruction(evnp) {
  if (likely(is_fixnum_obj(ac))) {
    ac = obj_from_bool((fixnum_from_obj(ac) & 1) == 0);
  } else {
    double f = flonum_from_obj(ac);
    ac = obj_from_bool(flisint(f / 2.0));
  }
  gonexti(); 
}

define_instruction(oddp) {
  if (likely(is_fixnum_obj(ac))) {
    ac = obj_from_bool((fixnum_from_obj(ac) & 1) != 0);
  } else {
    double f = flonum_from_obj(ac);
    ac = obj_from_bool(flisint((f + 1.0) / 2.0));
  }
  gonexti(); 
}

define_instruction(ntoi) {
  if (is_flonum_obj(ac)) ac = obj_from_fixnum(fxflo(flonum_from_obj(ac)));
  gonexti(); 
}

define_instruction(ntoj) {
  if (is_fixnum_obj(ac)) ac = obj_from_flonum(sp-r, (flonum_t)flonum_from_obj(ac));
  gonexti(); 
}


define_instruction(lcat) {
  obj t, l, *p, *d; int c;
  for (l = ac, c = 0; ispair(l); l = cdr(l)) ++c;
  hp_reserve(hbsz(2+1)*c);
  p = --sp; t = *p; /* pop & take addr */
  for (l = ac; ispair(l); l = cdr(l)) {
    *--hp = t; d = hp; *--hp = car(l);
    *--hp = obj_from_size(PAIR_BTAG); 
    *p = hendblk(2+1); p = d;
  }
  ac = *sp;
  gonexti(); 
}  

define_instruction(ccmp) {
  obj x = ac, y = spop();
  int cmp = char_from_obj(x) - char_from_obj(y);
  ac = obj_from_fixnum(cmp);
  gonexti(); 
}

define_instruction(ceq) {
  obj x = ac, y = spop();
  ac = obj_from_bool(char_from_obj(x) == char_from_obj(y));
  gonexti(); 
}

define_instruction(clt) {
  obj x = ac, y = spop();
  ac = obj_from_bool(char_from_obj(x) < char_from_obj(y));
  gonexti(); 
}

define_instruction(cgt) {
  obj x = ac, y = spop();
  ac = obj_from_bool(char_from_obj(x) > char_from_obj(y));
  gonexti(); 
}

define_instruction(cle) {
  obj x = ac, y = spop();
  ac = obj_from_bool(char_from_obj(x) <= char_from_obj(y));
  gonexti(); 
}

define_instruction(cge) {
  obj x = ac, y = spop();
  ac = obj_from_bool(char_from_obj(x) >= char_from_obj(y));
  gonexti(); 
}

define_instruction(cicmp) {
  obj x = ac, y = spop();
  int cmp = tolower(char_from_obj(x)) - tolower(char_from_obj(y));
  ac = obj_from_fixnum(cmp);
  gonexti(); 
}

define_instruction(cieq) {
  obj x = ac, y = spop();
  ac = obj_from_bool(tolower(char_from_obj(x)) == tolower(char_from_obj(y)));
  gonexti(); 
}

define_instruction(cilt) {
  obj x = ac, y = spop();
  ac = obj_from_bool(tolower(char_from_obj(x)) < tolower(char_from_obj(y)));
  gonexti(); 
}

define_instruction(cigt) {
  obj x = ac, y = spop();
  ac = obj_from_bool(tolower(char_from_obj(x)) > tolower(char_from_obj(y)));
  gonexti(); 
}

define_instruction(cile) {
  obj x = ac, y = spop();
  ac = obj_from_bool(tolower(char_from_obj(x)) <= tolower(char_from_obj(y)));
  gonexti(); 
}

define_instruction(cige) {
  obj x = ac, y = spop();
  ac = obj_from_bool(tolower(char_from_obj(x)) >= tolower(char_from_obj(y)));
  gonexti(); 
}

define_instruction(cwsp) {
  ac = obj_from_bool(isspace(char_from_obj(ac)));
  gonexti(); 
}

define_instruction(clcp) {
  ac = obj_from_bool(islower(char_from_obj(ac)));
  gonexti(); 
}

define_instruction(cucp) {
  ac = obj_from_bool(isupper(char_from_obj(ac)));
  gonexti(); 
}

define_instruction(calp) {
  ac = obj_from_bool(isalpha(char_from_obj(ac)));
  gonexti(); 
}

define_instruction(cnup) {
  ac = obj_from_bool(isdigit(char_from_obj(ac)));
  gonexti(); 
}

define_instruction(cupc) {
  ac = obj_from_char(toupper(char_from_obj(ac)));
  gonexti(); 
}

define_instruction(cdnc) {
  ac = obj_from_char(tolower(char_from_obj(ac)));
  gonexti(); 
}

define_instruction(scmp) {
  obj x = ac, y = spop();
  int cmp = strcmp(stringchars(x), stringchars(y));
  ac = obj_from_fixnum(cmp);
  gonexti(); 
}

define_instruction(seq) {
  obj x = ac, y = spop();
  ac = obj_from_bool(strcmp(stringchars(x), stringchars(y)) == 0);
  gonexti(); 
}

define_instruction(slt) {
  obj x = ac, y = spop();
  ac = obj_from_bool(strcmp(stringchars(x), stringchars(y)) < 0);
  gonexti(); 
}

define_instruction(sgt) {
  obj x = ac, y = spop();
  ac = obj_from_bool(strcmp(stringchars(x), stringchars(y)) > 0);
  gonexti(); 
}

define_instruction(sle) {
  obj x = ac, y = spop();
  ac = obj_from_bool(strcmp(stringchars(x), stringchars(y)) <= 0);
  gonexti(); 
}

define_instruction(sge) {
  obj x = ac, y = spop();
  ac = obj_from_bool(strcmp(stringchars(x), stringchars(y)) >= 0);
  gonexti(); 
}

define_instruction(sicmp) {
  obj x = ac, y = spop();
  int cmp = strcmp_ci(stringchars(x), stringchars(y));
  ac = obj_from_fixnum(cmp);
  gonexti(); 
}

define_instruction(sieq) {
  obj x = ac, y = spop();
  ac = obj_from_bool(strcmp_ci(stringchars(x), stringchars(y)) == 0);
  gonexti(); 
}

define_instruction(silt) {
  obj x = ac, y = spop();
  ac = obj_from_bool(strcmp_ci(stringchars(x), stringchars(y)) < 0);
  gonexti(); 
}

define_instruction(sigt) {
  obj x = ac, y = spop();
  ac = obj_from_bool(strcmp_ci(stringchars(x), stringchars(y)) > 0);
  gonexti(); 
}

define_instruction(sile) {
  obj x = ac, y = spop();
  ac = obj_from_bool(strcmp_ci(stringchars(x), stringchars(y)) <= 0);
  gonexti(); 
}

define_instruction(sige) {
  obj x = ac, y = spop();
  ac = obj_from_bool(strcmp_ci(stringchars(x), stringchars(y)) >= 0);
  gonexti(); 
}


define_instruction(symp) {
  ac = obj_from_bool(issymbol(ac));
  gonexti();
}

define_instruction(boolp) {
  ac = obj_from_bool(is_bool_obj(ac));
  gonexti();
}

define_instruction(eofp) {
  ac = obj_from_bool(iseof(ac));
  gonexti();
}

define_instruction(funp) {
  ac = obj_from_bool(isprocedure(ac)); // FIXME/TODO: make VM closure recognizeable as procedure
  gonexti();
}

define_instruction(ipp) {
  ac = obj_from_bool(isiport(ac));
  gonexti();
}

define_instruction(opp) {
  ac = obj_from_bool(isoport(ac));
  gonexti();
}

define_instruction(sip) {
  ac = mkiport_file(sp-r, stdin);
  gonexti();
}

define_instruction(sop) {
  ac = mkoport_file(sp-r, stdout);
  gonexti();
}

define_instruction(sep) {
  ac = mkoport_file(sp-r, stderr);
  gonexti();
}

define_instruction(otip) {
  FILE *fp = fopen(stringchars(ac), "r");
  if (fp == NULL) fail("can't open input file");
  ac = mkiport_file(sp-r, fp);
  gonexti();
}

define_instruction(otop) {
  FILE *fp = fopen(stringchars(ac), "w");
  if (fp == NULL) fail("can't open output file");
  ac = mkoport_file(sp-r, fp);
  gonexti();
}

define_instruction(ois) {
  int *d = dupstring(stringdata(ac));
  ac = mkiport_string(sp-r, sialloc(sdatachars(d), d));
  gonexti();
}

define_instruction(oos) {
  ac = mkoport_string(sp-r, newcb());
  gonexti();
}

define_instruction(cip) {
  cxtype_iport_t *vt = iportvt(ac); assert(vt);
  vt->close(iportdata(ac)); vt->free(iportdata(ac));
  objptr_from_obj(ac)[-1] = (obj)IPORT_CLOSED_NTAG;
  gonexti();
}

define_instruction(cop) {
  cxtype_oport_t *vt = oportvt(ac); assert(vt);
  vt->close(oportdata(ac)); vt->free(oportdata(ac));
  objptr_from_obj(ac)[-1] = (obj)OPORT_CLOSED_NTAG;
  gonexti();
}

define_instruction(gos) {
  cxtype_oport_t *vt = ckoportvt(ac);
  if (vt != (cxtype_oport_t *)OPORT_STRING_NTAG &&
      vt != (cxtype_oport_t *)OPORT_BYTEVECTOR_NTAG) {
    ac = mkeof();
  } else {
    cbuf_t *pcb = oportdata(ac);
    ac = hpushstr(sp-r, newstring(cbdata(pcb)));
  }
  gonexti();
}

define_instruction(wrc) {
  oportputc(char_from_obj(ac), spop());
  gonexti();
}

define_instruction(wrs) {
  oportputs(stringchars(ac), spop());
  gonexti();
}

define_instruction(wrcd) {
  oportputcircular(ac, spop(), 1);
  gonexti();
}

define_instruction(wrcw) {
  oportputcircular(ac, spop(), 0);
  gonexti();
}

define_instruction(wrnl) {
  oportputc('\n', ac);
  gonexti();
}

define_instruction(wrhw) {
  oportputshared(ac, spop(), 0);
  gonexti();
}

define_instruction(wriw) {
  oportputsimple(ac, spop(), 0);
  gonexti();
}

define_instruction(fenc) {
  obj y = ac, c = spop();
  assert(issymbol(y) && is_fixnum_obj(c));
  ac = (obj)intgtab_find_encoding(getsymbol(y), fixnum_from_obj(c));
  gonexti(); 
}

define_instruction(wrsi) {
  obj c = ac, e = spop(), p = spop();
  assert(is_fixnum_obj(c) && isaptr(e) && notobjptr(e) && isoport(p));
  wrs_integrable(fixnum_from_obj(c), (struct intgtab_entry *)e, p);
  gonexti(); 
}

define_instruction(rdsx) {
  unload_ac(); /* ac->ra (string) */
  hp = rds_stox(r, sp, hp);
  reload_ac(); /* ra->ac (sexp or eof) */
  /* if (ac == mkeof()) { set ac to error object; unwindi(0); }  */
  gonexti(); 
}

define_instruction(rdsc) {
  unload_ac(); /* ac->ra (port) */
  hp = rds_stoc(r, sp, hp);
  reload_ac(); /* ra->ac (codevec or eof) */
  /* if (ac == mkeof()) { set ac to error object; unwindi(0); } */
  gonexti(); 
}

define_instruction(litf) { ac = obj_from_bool(0); gonexti(); }  
define_instruction(litt) { ac = obj_from_bool(1); gonexti(); }  
define_instruction(litn) { ac = mknull(); gonexti(); }  
define_instruction(pushlitf) { ac = obj_from_bool(0); spush(ac); gonexti(); }  
define_instruction(pushlitt) { ac = obj_from_bool(1); spush(ac); gonexti(); }  
define_instruction(pushlitn) { ac = mknull(); spush(ac); gonexti(); }  

define_instruction(lit0) { ac = obj_from_fixnum(0); gonexti(); }  
define_instruction(lit1) { ac = obj_from_fixnum(1); gonexti(); }  
define_instruction(lit2) { ac = obj_from_fixnum(2); gonexti(); }  
define_instruction(lit3) { ac = obj_from_fixnum(3); gonexti(); }  
define_instruction(lit4) { ac = obj_from_fixnum(4); gonexti(); }  
define_instruction(lit5) { ac = obj_from_fixnum(5); gonexti(); }  
define_instruction(lit6) { ac = obj_from_fixnum(6); gonexti(); }  
define_instruction(lit7) { ac = obj_from_fixnum(7); gonexti(); }  
define_instruction(lit8) { ac = obj_from_fixnum(8); gonexti(); }  
define_instruction(lit9) { ac = obj_from_fixnum(9); gonexti(); }  
define_instruction(pushlit0) { ac = obj_from_fixnum(0); spush(ac); gonexti(); }  
define_instruction(pushlit1) { ac = obj_from_fixnum(1); spush(ac); gonexti(); }  
define_instruction(pushlit2) { ac = obj_from_fixnum(2); spush(ac); gonexti(); }  
define_instruction(pushlit3) { ac = obj_from_fixnum(3); spush(ac); gonexti(); }  
define_instruction(pushlit4) { ac = obj_from_fixnum(4); spush(ac); gonexti(); }  
define_instruction(pushlit5) { ac = obj_from_fixnum(5); spush(ac); gonexti(); }  
define_instruction(pushlit6) { ac = obj_from_fixnum(6); spush(ac); gonexti(); }  
define_instruction(pushlit7) { ac = obj_from_fixnum(7); spush(ac); gonexti(); }  
define_instruction(pushlit8) { ac = obj_from_fixnum(8); spush(ac); gonexti(); }  
define_instruction(pushlit9) { ac = obj_from_fixnum(9); spush(ac); gonexti(); }  

define_instruction(sref0) { ac = sref(0); gonexti(); }  
define_instruction(sref1) { ac = sref(1); gonexti(); }  
define_instruction(sref2) { ac = sref(2); gonexti(); }  
define_instruction(sref3) { ac = sref(3); gonexti(); }  
define_instruction(sref4) { ac = sref(4); gonexti(); }  
define_instruction(sref5) { ac = sref(5); gonexti(); }  
define_instruction(sref6) { ac = sref(6); gonexti(); }  
define_instruction(sref7) { ac = sref(7); gonexti(); }  
define_instruction(sref8) { ac = sref(8); gonexti(); }  
define_instruction(sref9) { ac = sref(9); gonexti(); }  
define_instruction(pushsref0) { ac = sref(0); spush(ac); gonexti(); }  
define_instruction(pushsref1) { ac = sref(1); spush(ac); gonexti(); }  
define_instruction(pushsref2) { ac = sref(2); spush(ac); gonexti(); }  
define_instruction(pushsref3) { ac = sref(3); spush(ac); gonexti(); }  
define_instruction(pushsref4) { ac = sref(4); spush(ac); gonexti(); }  
define_instruction(pushsref5) { ac = sref(5); spush(ac); gonexti(); }  
define_instruction(pushsref6) { ac = sref(6); spush(ac); gonexti(); }  
define_instruction(pushsref7) { ac = sref(7); spush(ac); gonexti(); }  
define_instruction(pushsref8) { ac = sref(8); spush(ac); gonexti(); }  
define_instruction(pushsref9) { ac = sref(9); spush(ac); gonexti(); }  

define_instruction(srefi0) { ac = boxref(sref(0)); gonexti(); }
define_instruction(srefi1) { ac = boxref(sref(1)); gonexti(); }
define_instruction(srefi2) { ac = boxref(sref(2)); gonexti(); }
define_instruction(srefi3) { ac = boxref(sref(3)); gonexti(); }
define_instruction(srefi4) { ac = boxref(sref(4)); gonexti(); }
define_instruction(pushsrefi0) { ac = boxref(sref(0)); spush(ac); gonexti(); }
define_instruction(pushsrefi1) { ac = boxref(sref(1)); spush(ac); gonexti(); }
define_instruction(pushsrefi2) { ac = boxref(sref(2)); spush(ac); gonexti(); }
define_instruction(pushsrefi3) { ac = boxref(sref(3)); spush(ac); gonexti(); }
define_instruction(pushsrefi4) { ac = boxref(sref(4)); spush(ac); gonexti(); }

define_instruction(dref0) { ac = dref(0); gonexti(); }
define_instruction(dref1) { ac = dref(1); gonexti(); }
define_instruction(dref2) { ac = dref(2); gonexti(); }
define_instruction(dref3) { ac = dref(3); gonexti(); }
define_instruction(dref4) { ac = dref(4); gonexti(); }
define_instruction(pushdref0) { ac = dref(0); spush(ac); gonexti(); }  
define_instruction(pushdref1) { ac = dref(1); spush(ac); gonexti(); }  
define_instruction(pushdref2) { ac = dref(2); spush(ac); gonexti(); }  
define_instruction(pushdref3) { ac = dref(3); spush(ac); gonexti(); }  
define_instruction(pushdref4) { ac = dref(4); spush(ac); gonexti(); }  

define_instruction(drefi0) { ac = boxref(dref(0)); gonexti(); }
define_instruction(drefi1) { ac = boxref(dref(1)); gonexti(); }
define_instruction(drefi2) { ac = boxref(dref(2)); gonexti(); }
define_instruction(drefi3) { ac = boxref(dref(3)); gonexti(); }
define_instruction(drefi4) { ac = boxref(dref(4)); gonexti(); }
define_instruction(pushdrefi0) { ac = boxref(dref(0)); spush(ac); gonexti(); }
define_instruction(pushdrefi1) { ac = boxref(dref(1)); spush(ac); gonexti(); }
define_instruction(pushdrefi2) { ac = boxref(dref(2)); spush(ac); gonexti(); }
define_instruction(pushdrefi3) { ac = boxref(dref(3)); spush(ac); gonexti(); }
define_instruction(pushdrefi4) { ac = boxref(dref(4)); spush(ac); gonexti(); }

define_instruction(call0) { 
  rd = ac; rx = obj_from_fixnum(0); ac = obj_from_fixnum(0); 
  callsubi(); 
}
define_instruction(call1) { 
  rd = ac; rx = obj_from_fixnum(0); ac = obj_from_fixnum(1); 
  callsubi(); 
}
define_instruction(call2) { 
  rd = ac; rx = obj_from_fixnum(0); ac = obj_from_fixnum(2); 
  callsubi(); 
}
define_instruction(call3) { 
  rd = ac; rx = obj_from_fixnum(0); ac = obj_from_fixnum(3); 
  callsubi(); 
}
define_instruction(call4) { 
  rd = ac; rx = obj_from_fixnum(0); ac = obj_from_fixnum(4); 
  callsubi(); 
}

define_instruction(sreturn1) { 
  sdrop(1); rx = spop(); rd = spop(); 
  retfromi(); 
}
define_instruction(sreturn2) { 
  sdrop(2); rx = spop(); rd = spop(); 
  retfromi(); 
}
define_instruction(sreturn3) { 
  sdrop(3); rx = spop(); rd = spop(); 
  retfromi(); 
}
define_instruction(sreturn4) { 
  sdrop(4); rx = spop(); rd = spop(); 
  retfromi(); 
}

define_instruction(atest0) { if (unlikely(ac != obj_from_fixnum(0))) fail("argument count error on entry"); gonexti(); }
define_instruction(atest1) { if (unlikely(ac != obj_from_fixnum(1))) fail("argument count error on entry"); gonexti(); }
define_instruction(atest2) { if (unlikely(ac != obj_from_fixnum(2))) fail("argument count error on entry"); gonexti(); }
define_instruction(atest3) { if (unlikely(ac != obj_from_fixnum(3))) fail("argument count error on entry"); gonexti(); }
define_instruction(atest4) { if (unlikely(ac != obj_from_fixnum(4))) fail("argument count error on entry"); gonexti(); }


define_instruction(scall1) {
  int m = 1, n = fixnum_from_obj(*ip++);
  rd = ac; rx = obj_from_fixnum(0); 
  ac = obj_from_fixnum(n); /* argc */
  memmove((void*)(sp-n-m), (void*)(sp-n), (size_t)n*sizeof(obj));
  sdrop(m); callsubi();
}

define_instruction(scall10) {
  rd = ac; rx = obj_from_fixnum(0); ac = obj_from_fixnum(0); /* argc */
  sdrop(1); callsubi();
}

define_instruction(scall11) {
  rd = ac; rx = obj_from_fixnum(0); ac = obj_from_fixnum(1); /* argc */
  sref(1) = sref(0);
  sdrop(1); callsubi();
}

define_instruction(scall12) {
  rd = ac; rx = obj_from_fixnum(0); ac = obj_from_fixnum(2); /* argc */
  sref(2) = sref(1); sref(1) = sref(0);
  sdrop(1); callsubi();
}

define_instruction(scall13) {
  rd = ac; rx = obj_from_fixnum(0); ac = obj_from_fixnum(3); /* argc */
  sref(3) = sref(2); sref(2) = sref(1); sref(1) = sref(0);
  sdrop(1); callsubi();
}

define_instruction(scall14) {
  rd = ac; rx = obj_from_fixnum(0); ac = obj_from_fixnum(4); /* argc */
  sref(4) = sref(3); sref(3) = sref(2); sref(2) = sref(1); sref(1) = sref(0);
  sdrop(1); callsubi();
}

define_instruction(scall2) {
  int m = 2, n = fixnum_from_obj(*ip++);
  rd = ac; rx = obj_from_fixnum(0); 
  ac = obj_from_fixnum(n); /* argc */
  memmove((void*)(sp-n-m), (void*)(sp-n), (size_t)n*sizeof(obj));
  sdrop(m); callsubi();
}

define_instruction(scall20) {
  rd = ac; rx = obj_from_fixnum(0); ac = obj_from_fixnum(0); /* argc */
  sdrop(2); callsubi();
}

define_instruction(scall21) {
  rd = ac; rx = obj_from_fixnum(0); ac = obj_from_fixnum(1); /* argc */
  sref(2) = sref(0);
  sdrop(2); callsubi();
}

define_instruction(scall22) {
  rd = ac; rx = obj_from_fixnum(0); ac = obj_from_fixnum(2); /* argc */
  sref(3) = sref(1); sref(2) = sref(0);
  sdrop(2); callsubi();
}

define_instruction(scall23) {
  rd = ac; rx = obj_from_fixnum(0); ac = obj_from_fixnum(3); /* argc */
  sref(4) = sref(2); sref(3) = sref(1); sref(2) = sref(0);
  sdrop(2); callsubi();
}

define_instruction(scall24) {
  rd = ac; rx = obj_from_fixnum(0); ac = obj_from_fixnum(4); /* argc */
  sref(5) = sref(3); sref(4) = sref(2); sref(3) = sref(1); sref(2) = sref(0);
  sdrop(2); callsubi();
}

define_instruction(scall3) {
  int m = 3, n = fixnum_from_obj(*ip++);
  rd = ac; rx = obj_from_fixnum(0); 
  ac = obj_from_fixnum(n); /* argc */
  memmove((void*)(sp-n-m), (void*)(sp-n), (size_t)n*sizeof(obj));
  sdrop(m); callsubi();
}

define_instruction(scall30) {
  rd = ac; rx = obj_from_fixnum(0); ac = obj_from_fixnum(0); /* argc */
  sdrop(3); callsubi();
}

define_instruction(scall31) {
  rd = ac; rx = obj_from_fixnum(0); ac = obj_from_fixnum(1); /* argc */
  sref(3) = sref(0);
  sdrop(3); callsubi();
}

define_instruction(scall32) {
  rd = ac; rx = obj_from_fixnum(0); ac = obj_from_fixnum(2); /* argc */
  sref(4) = sref(1); sref(3) = sref(0);
  sdrop(3); callsubi();
}

define_instruction(scall33) {
  rd = ac; rx = obj_from_fixnum(0); ac = obj_from_fixnum(3); /* argc */
  sref(5) = sref(2); sref(4) = sref(1); sref(3) = sref(0);
  sdrop(3); callsubi();
}

define_instruction(scall34) {
  rd = ac; rx = obj_from_fixnum(0); ac = obj_from_fixnum(4); /* argc */
  sref(6) = sref(3); sref(5) = sref(2); sref(4) = sref(1); sref(3) = sref(0);
  sdrop(3); callsubi();
}

define_instruction(scall4) {
  int m = 3, n = fixnum_from_obj(*ip++);
  rd = ac; rx = obj_from_fixnum(0); 
  ac = obj_from_fixnum(n); /* argc */
  memmove((void*)(sp-n-m), (void*)(sp-n), (size_t)n*sizeof(obj));
  sdrop(m); callsubi();
}

define_instruction(scall40) {
  rd = ac; rx = obj_from_fixnum(0); ac = obj_from_fixnum(0); /* argc */
  sdrop(4); callsubi();
}

define_instruction(scall41) {
  rd = ac; rx = obj_from_fixnum(0); ac = obj_from_fixnum(1); /* argc */
  sref(4) = sref(0);
  sdrop(4); callsubi();
}

define_instruction(scall42) {
  rd = ac; rx = obj_from_fixnum(0); ac = obj_from_fixnum(2); /* argc */
  sref(5) = sref(1); sref(4) = sref(0);
  sdrop(4); callsubi();
}

define_instruction(scall43) {
  rd = ac; rx = obj_from_fixnum(0); ac = obj_from_fixnum(3); /* argc */
  sref(6) = sref(2); sref(5) = sref(1); sref(4) = sref(0);
  sdrop(4); callsubi();
}

define_instruction(scall44) {
  rd = ac; rx = obj_from_fixnum(0); ac = obj_from_fixnum(4); /* argc */
  sref(7) = sref(3); sref(6) = sref(2); sref(5) = sref(1); sref(4) = sref(0);
  sdrop(4); callsubi();
}

define_instruction(brnotlt) {
  obj x = ac, y = spop();
  if (likely(are_fixnum_objs(x, y))) {
    int dx = fixnum_from_obj(*ip++);
    ac = obj_from_bool(get_fixnum_unchecked(x) < get_fixnum_unchecked(y));
    ip = ac ? ip : ip + dx;
    gonexti(); 
  } else {
    int dx = fixnum_from_obj(*ip++);
    double fx, fy;
    if (likely(is_flonum_obj(x))) fx = flonum_from_obj(x);
    else fx = (double)fixnum_from_obj(x);
    if (likely(is_flonum_obj(y))) fy = flonum_from_obj(y);
    else fy = (double)fixnum_from_obj(y);
    ac = obj_from_bool(fx < fy);
    ip = ac ? ip : ip + dx;
    gonexti(); 
  }
}

define_instruction(pushsub) {
  obj x = ac, y = spop();
  if (likely(are_fixnum_objs(x, y))) {
    ac = obj_from_fixnum(fxsub(get_fixnum_unchecked(x), get_fixnum_unchecked(y)));
  } else {
    double dx, dy;
    if (likely(is_flonum_obj(x))) dx = flonum_from_obj(x);
    else dx = (double)fixnum_from_obj(x);
    if (likely(is_flonum_obj(y))) dy = flonum_from_obj(y);
    else dy = (double)fixnum_from_obj(y);
    ac = obj_from_flonum(sp-r, dx - dy);
  }
  spush(ac);
  gonexti(); 
}


#define VM_GEN_DEFGLOBAL
#include "i.h"
#undef  VM_GEN_DEFGLOBAL

/* integrables table */
struct intgtab_entry { int sym; char *igname; int arity; char *enc; char *lcode; };
#define declare_intgtable_entry(enc, igname, arity, lcode) \
   { 0, igname, arity, enc, lcode },

static struct intgtab_entry intgtab[] = {
#define VM_GEN_INTGTABLE
#include "i.h"
#undef  VM_GEN_INTGTABLE
};

static int intgtab_sorted = 0;
static int intgtab_cmp(const void *p1, const void *p2)
{
  struct intgtab_entry *pe1 = (struct intgtab_entry *)p1;
  struct intgtab_entry *pe2 = (struct intgtab_entry *)p2;
  if (pe1->sym < pe2->sym) return -1;
  if (pe1->sym > pe2->sym) return 1;
  if (pe1->arity < pe2->arity) return -1;
  if (pe1->arity > pe2->arity) return 1;
  return 0;
}

static void sort_intgtab(int n)
{
  if (!intgtab_sorted) {
    int i;
    for (i = 0; i < n; ++i) {
      struct intgtab_entry *pe = &intgtab[i];
      pe->sym = pe->igname ? internsym(pe->igname) : 0;
    }
    qsort(&intgtab[0], n, sizeof(intgtab[0]), intgtab_cmp); 
    intgtab_sorted = 1;
  }
} 

static struct intgtab_entry *intgtab_find_encoding(int sym, int arity)
{
  struct intgtab_entry e, *pe;
  int n = sizeof(intgtab)/sizeof(intgtab[0]);
  if (!intgtab_sorted) sort_intgtab(n);
  e.sym = sym; e.arity = arity;
  pe = bsearch(&e, &intgtab[0], n, sizeof(intgtab[0]), intgtab_cmp);
  if (!pe) { e.arity = -1; pe = bsearch(&e, &intgtab[0], n, sizeof(intgtab[0]), intgtab_cmp); }
  return (pe && pe->enc) ? pe : NULL;
}


/* serialization machinery */
static void wrs_int_arg(int arg, obj port)
{
  if (0 <= arg && arg <= 9) {
    oportputc('0'+arg, port);
  } else {
    char buf[60];
    sprintf(buf, "(i%d)", arg);
    oportputs(buf, port);
  } 
}

static void wrs_integrable(int argc, struct intgtab_entry *pe, obj port)
{
  assert(pe); assert(pe->enc);
  if (pe->arity == -1 && argc > 0) oportputc(',', port);
  oportputs(pe->enc, port);
  if (pe->arity == -1) wrs_int_arg(argc, port);
}


/* deserialization machinery */

static int rds_char(obj port)
{
  int c = iportgetc(port);
  if (c == '%') {
    char buf[3];
    buf[0] = iportgetc(port); buf[1] = iportgetc(port); buf[2] = 0;
    c = (int)strtol(buf, NULL, 16);
  }
  return c;
}

static int rds_int(obj port)
{
  char buf[60], *p = buf, *e = p+59;
  while (p < e) {
    int c = iportpeekc(port);
    if (c == '-' || c == '+' || (c >= '0' && c <= '9')) {
      iportgetc(port);
      *p++ = c;
    } else break;
  } 
  *p = 0;
  return (int)strtol(buf, NULL, 10);
}

static double rds_real(obj port)
{
  char buf[60], *p = buf, *e = p+59;
  while (p < e) {
    int c = iportpeekc(port);
    if (c == '-' || c == '+' || c == '.' || c == 'e' || c == 'E' || (c >= '0' && c <= '9')) {
      iportgetc(port);
      *p++ = c;
    } else break;
  } 
  *p = 0;
  return strtod(buf, NULL);
}

static size_t rds_size(obj port)
{
  int n = rds_int(port);
  if ((n < 0) || iportgetc(port) != ':') return 0; /* postpone failure */
  return (size_t)n;
}

/* see below */
static obj *rds_sexp(obj *r, obj *sp, obj *hp);

/* protects registers from r to sp, in: ra=port, out: ra=sexp/eof */
static obj *rds_elt(obj *r, obj *sp, obj *hp)
{
  obj port = ra;
  spush(port);
  hp = rds_sexp(r, sp, hp);
  port = spop();
  if (!iseof(ra) && iportgetc(port) != ';') ra = mkeof();
  return hp;
}

/* protects registers from r to sp, in: ra=port, out: ra=sexp/eof */
static obj *rds_sexp(obj *r, obj *sp, obj *hp)
{
  obj port = ra;
  int c = iportgetc(port);
  switch (c) {
    default:  ra = mkeof(); break;
    case 'f': ra = obj_from_bool(0); break;
    case 't': ra = obj_from_bool(1); break;
    case 'n': ra = mknull(); break;
    case 'c': ra = obj_from_char(rds_char(port)); break;
    case 'i': ra = obj_from_fixnum(rds_int(port)); break; 
    case 'j': ra = obj_from_flonum(sp-r, rds_real(port)); break;
    case 'p': {
      spush(port);
      ra = sref(0); hp = rds_elt(r, sp, hp); 
      if (iseof(ra)) { sdrop(1); return hp; } else spush(ra);
      ra = sref(1); hp = rds_elt(r, sp, hp); 
      if (iseof(ra)) { sdrop(2); return hp; } else spush(ra);
      hreserve(hbsz(3), sp-r);
      *--hp = sref(0);  
      *--hp = sref(1);  
      *--hp = obj_from_size(PAIR_BTAG); 
      ra = hendblk(3);
      sdrop(3);      
    } break; 
    case 'l': {
      size_t n = rds_size(port), i;
      spush(port); /* sref(0)=port */
      for (i = 0; i < n; ++i) {
        ra = sref((int)i); hp = rds_elt(r, sp, hp); 
        if (iseof(ra)) { sdrop(i+1); return hp; } else spush(ra);
      }
      hreserve(hbsz(3)*n, sp-r);
      ra = mknull();
      for (i = 0; i < n; ++i) {
        *--hp = ra;  
        *--hp = sref((int)i);  
        *--hp = obj_from_size(PAIR_BTAG); 
        ra = hendblk(3);
      }      
      sdrop(n+1);
    } break; 
    case 'v': {
      size_t n = rds_size(port), i;
      spush(port);
      for (i = 0; i < n; ++i) {
        ra = sref((int)i); hp = rds_elt(r, sp, hp); 
        if (iseof(ra)) { sdrop(i+1); return hp; } else spush(ra);
      }
      hreserve(hbsz(n+1), sp-r);
      hp -= n; memcpy(hp, sp-n, n*sizeof(obj));      
      *--hp = obj_from_size(VECTOR_BTAG);
      ra = hendblk(n+1);
      sdrop(n+1);
    } break;
    case 's': case 'y': {
      cbuf_t *pcb = newcb();
      size_t n = rds_size(port), i;
      for (i = 0; i < n; ++i) {
        int x = rds_char(port);
        cbputc(x, pcb);
      }
      if (c == 's') ra = hpushstr(sp-r, newstring(cbdata(pcb)));
      else ra = mksymbol(internsym(cbdata(pcb)));
      freecb(pcb);
    }
  }  
  return hp;
}

/* protects registers from r to sp, in: ra=str, out: ra=sexp/eof */
static obj *rds_stox(obj *r, obj *sp, obj *hp)
{
  int *d = dupstring(stringdata(ra));
  ra = mkiport_string(sp-r, sialloc(sdatachars(d), d));
  hp = rds_sexp(r, sp, hp);  /* ra=port => ra=sexp/eof */
  return hp;
}

static struct { obj *pg; const char *enc; int etyp; } enctab[] = {
#define VM_GEN_ENCTABLE
#define declare_enctable_entry(name, enc, etyp) \
 { &glue(cx_ins_2D, name), enc, etyp },
#include "i.h"
#undef declare_enctable_entry
#undef VM_GEN_ENCTABLE
 { NULL, NULL, 0 }
};

struct emtrans { int c; struct embranch *pbr; struct emtrans *ptr; };
struct embranch { obj g; int etyp; struct emtrans *ptr; };
static struct embranch encmap = { 0, 0, NULL };

static void encmap_add(const char *enc, obj g, int etyp)
{
  const char *pc = enc; struct embranch *pbr = &encmap;
  down: while (*pc) {
    int c = *pc++; struct emtrans *ptr = pbr->ptr;
    while (ptr != NULL) {
      if (ptr->c != c) {
        if (ptr->ptr == NULL) break;
        ptr = ptr->ptr; continue;
      }
      pbr = ptr->pbr; goto down; 
    }
    if (!(ptr = calloc(1, sizeof(struct emtrans)))) 
      { perror("encmap[memt]"); exit(2); }
    ptr->c = c; ptr->ptr = pbr->ptr; pbr->ptr = ptr;
    if (!(ptr->pbr = calloc(1, sizeof(struct embranch)))) 
      { perror("encmap[memb]"); exit(2); }
    pbr = ptr->pbr;
  }
  if (pbr->g != 0) { 
    perror("encmap[dup]"); exit(2); 
  }
  pbr->g = g; pbr->etyp = etyp;
}

static struct embranch *get_encmap(void)
{
  if (encmap.ptr == NULL) {
    size_t i;
    for (i = 0; enctab[i].pg != NULL; ++i) {
      if (enctab[i].enc != NULL) {
        encmap_add(enctab[i].enc, *(enctab[i].pg), enctab[i].etyp);
      }
    }
    assert(encmap.g == 0);
    assert(encmap.ptr != NULL);
  }
  return &encmap;
} 

static struct embranch *rds_prefix(obj port)
{
  struct embranch *pbr = get_encmap(); int c;
  down: while ((c = iportpeekc(port)) != EOF) {
    struct emtrans *ptr = pbr->ptr;
    while (ptr != NULL) {
      if (ptr->c != c) {
        if (ptr->ptr == NULL) break;
        ptr = ptr->ptr; continue;
      }
      pbr = ptr->pbr; iportgetc(port); 
      goto down; 
    }
    break;
  }
  return pbr;
}

static obj lastpair(obj l)
{
  obj p = obj_from_bool(0);
  while (ispair(l)) { p = l; l = cdr(l); }
  return p;
}

static fixnum_t length(obj l)
{
  fixnum_t n = 0;
  while (ispair(l)) { ++n; l = cdr(l); }
  return n;
}

/* protects registers from r to sp, in: ra=list, out: ra=vec */
obj *revlist2vec(obj *r, obj *sp, obj *hp)
{
  obj l; fixnum_t n = length(ra), i;
  hreserve(hbsz(n+1), sp-r);
  for (l = ra, i = 0; i < n; ++i, l = cdr(l)) *--hp = car(l);
  *--hp = obj_from_size(VECTOR_BTAG);
  ra = hendblk(n+1);
  return hp;
}

/* protects registers from r to sp, in: ra=code, out: ra=closure */
obj *close0(obj *r, obj *sp, obj *hp)
{
  hreserve(vmclobsz(1), sp-r);
  *--hp = ra;
  ra = hpushvmclo(1);
  return hp;
}

/* protects registers from r to sp, in: ra=port, out: ra=sexp/eof */
static obj *rds_arg(obj *r, obj *sp, obj *hp)
{
  obj port = ra; int c = iportgetc(port);
  if ('0' <= c && c <= '9') {
    ra = obj_from_fixnum(c-'0');
  } else {
    if (c != '(') ra = mkeof(); else {
      spush(port);
      ra = sref(0); hp = rds_sexp(r, sp, hp); 
      port = sref(0);
      if (!iseof(ra) && iportgetc(port) != ')') ra = mkeof();
      sdrop(1); 
    }
  }
  return hp;
}

/* protects registers from r to sp, in: ra=sym, out: ra=loc/eof */
static obj *rds_global_loc(obj *r, obj *sp, obj *hp)
{
  if (issymbol(ra)) {
    obj p = isassv(ra, cx__2Aglobals_2A);
    if (ispair(p)) ra = p;
    else { /* prepend (sym . undefined) to *globals* */
      hreserve(hbsz(3)*2, sp-r);
      *--hp = mksymbol(internsym("undefined")); *--hp = ra;
      *--hp = obj_from_size(PAIR_BTAG); ra = hendblk(3);
      *--hp = cx__2Aglobals_2A; *--hp = ra;
      *--hp = obj_from_size(PAIR_BTAG); cx__2Aglobals_2A = hendblk(3);
    }
  } else {
    ra = mkeof();
  }
  return hp;
}


/* see below */
static obj *rds_seq(obj *r, obj *sp, obj *hp);

/* protects registers from r to sp, in: ra=port, out: ra=sexp/eof */
static obj *rds_block(obj *r, obj *sp, obj *hp)
{
  obj port = ra; int c = iportgetc(port);
  if (c != '{') ra = mkeof(); else {
    spush(port);
    ra = sref(0); hp = rds_seq(r, sp, hp); 
    port = sref(0);
    if (!iseof(ra) && iportgetc(port) != '}') ra = mkeof();
    sdrop(1); 
  }
  return hp; 
}

/* protects registers from r to sp, in: ra=port, out: ra=sexp/eof */
static obj *rds_seq(obj *r, obj *sp, obj *hp)
{
  obj port = ra; int c;
  spush(port);      /* sref[1] = port */
  spush(mknull());  /* sref[0] = l */
more:  
  c = iportpeekc(sref(1));
  if (c == EOF) {
    hreserve(hbsz(3), sp-r);
    *--hp = sref(0); *--hp = glue(cx_ins_2D, halt);  
    *--hp = obj_from_size(PAIR_BTAG); ra = hendblk(3);
  } else if (c == '}') { 
    ra = sref(0);
  } else {
    struct embranch *pbr = rds_prefix(sref(1));
    if (pbr->g == 0) {
      /* fprintf(stderr, "### rds_seq's remaining chars:\n");
      while (iportpeekc(sref(1)) != EOF) fputc(iportgetc(sref(1)), stderr);
      fputc('\n', stderr); */
      ra = mkeof();
    } else switch (pbr->etyp) {
      case 0: {
        hreserve(hbsz(3), sp-r);
        *--hp = sref(0); *--hp = pbr->g;  
        *--hp = obj_from_size(PAIR_BTAG); sref(0) = hendblk(3);
        goto more;
      } break;
      case 1: {
        ra = sref(1); hp = rds_arg(r, sp, hp);
        if (iseof(ra)) goto out;
        hreserve(hbsz(3)*2, sp-r);
        *--hp = sref(0); *--hp = pbr->g;  
        *--hp = obj_from_size(PAIR_BTAG); sref(0) = hendblk(3);
        *--hp = sref(0); *--hp = ra;  
        *--hp = obj_from_size(PAIR_BTAG); sref(0) = hendblk(3);
        goto more;
      } break;
      case 2: {
        ra = sref(1); hp = rds_arg(r, sp, hp);
        if (iseof(ra)) goto out;
        hreserve(hbsz(3)*2, sp-r);
        *--hp = sref(0); *--hp = pbr->g;  
        *--hp = obj_from_size(PAIR_BTAG); sref(0) = hendblk(3);
        *--hp = sref(0); *--hp = ra;  
        *--hp = obj_from_size(PAIR_BTAG); sref(0) = hendblk(3);
        ra = sref(1); hp = rds_arg(r, sp, hp);
        if (iseof(ra)) goto out;
        hreserve(hbsz(3), sp-r);
        *--hp = sref(0); *--hp = ra;  
        *--hp = obj_from_size(PAIR_BTAG); sref(0) = hendblk(3);
        goto more;
      } break;
      case 'f': case 't': case 'n': {
        hreserve(hbsz(3)*2, sp-r);
        *--hp = sref(0); *--hp = pbr->g;  
        *--hp = obj_from_size(PAIR_BTAG); sref(0) = hendblk(3);
        *--hp = sref(0); *--hp = (pbr->etyp == 'n' ? mknull() : obj_from_bool(pbr->etyp == 't'));
        *--hp = obj_from_size(PAIR_BTAG); sref(0) = hendblk(3);
        goto more;
      } break;
      case 'g': { /* gref/gset */
        ra = sref(1); hp = rds_arg(r, sp, hp);
        if (iseof(ra)) goto out;
        hp = rds_global_loc(r, sp, hp); /* ra => ra */
        if (iseof(ra)) goto out;
        hreserve(hbsz(3)*2, sp-r); 
        *--hp = sref(0); *--hp = pbr->g;  
        *--hp = obj_from_size(PAIR_BTAG); sref(0) = hendblk(3);
        *--hp = sref(0); *--hp = ra;
        *--hp = obj_from_size(PAIR_BTAG); sref(0) = hendblk(3);
        goto more;
      } break;
      case 's': { /* save */
        fixnum_t n;
        ra = sref(1); hp = rds_block(r, sp, hp);
        if (iseof(ra)) goto out;
        n = length(ra);
        hreserve(hbsz(3)*2, sp-r); 
        *--hp = sref(0); *--hp = pbr->g;  
        *--hp = obj_from_size(PAIR_BTAG); sref(0) = hendblk(3);
        *--hp = sref(0); *--hp = obj_from_fixnum(n);
        *--hp = obj_from_size(PAIR_BTAG); sref(0) = hendblk(3);
        if (n > 0) {
          obj lp = lastpair(ra); assert(ispair(lp));
          cdr(lp) = sref(0); sref(0) = ra;
        }
        goto more;
      } break;
      case 'd': { /* dclose */
        fixnum_t n;
        ra = sref(1); hp = rds_arg(r, sp, hp);
        if (!is_fixnum_obj(ra)) { 
          ra = mkeof(); goto out; 
        }
        n = fixnum_from_obj(ra);
        ra = sref(1); hp = rds_block(r, sp, hp);
        if (iseof(ra)) goto out;
        hp = revlist2vec(r, sp, hp); /* ra => ra */
        if (iseof(ra)) goto out;
        hreserve(hbsz(3)*3, sp-r); 
        *--hp = sref(0); *--hp = pbr->g;  
        *--hp = obj_from_size(PAIR_BTAG); sref(0) = hendblk(3);
        *--hp = sref(0); *--hp = obj_from_fixnum(n);
        *--hp = obj_from_size(PAIR_BTAG); sref(0) = hendblk(3);
        *--hp = sref(0); *--hp = ra;  
        *--hp = obj_from_size(PAIR_BTAG); sref(0) = hendblk(3);
        goto more;
      } break;
      case 'b': { /* branches */
        fixnum_t n; int c;
        ra = sref(1); hp = rds_block(r, sp, hp);
        if (iseof(ra)) goto out;
        c = iportpeekc(sref(1));
        if (c == '{') { /* 2-arm branch combo */
          obj bseq1 = ra, bseq2;
          spush(bseq1); /* ... port, l, bseq1 <= sp */
          ra = sref(2); hp = rds_block(r, sp, hp);
          if (iseof(ra)) { sdrop(1); goto out; }
          hreserve(hbsz(3)*4, sp-r); 
          bseq1 = spop(); bseq2 = ra;
          n = length(bseq1);
          *--hp = sref(0); *--hp = pbr->g;  
          *--hp = obj_from_size(PAIR_BTAG); sref(0) = hendblk(3);
          *--hp = sref(0); *--hp = obj_from_fixnum(n+2);
          *--hp = obj_from_size(PAIR_BTAG); sref(0) = hendblk(3);
          if (n > 0) {
            obj lp = lastpair(bseq1); assert(ispair(lp));
            cdr(lp) = sref(0); sref(0) = bseq1;
          }
          *--hp = sref(0); *--hp = glue(cx_ins_2D, br);
          *--hp = obj_from_size(PAIR_BTAG); sref(0) = hendblk(3);
          n = length(bseq2);
          *--hp = sref(0); *--hp = obj_from_fixnum(n);
          *--hp = obj_from_size(PAIR_BTAG); sref(0) = hendblk(3);
          if (n > 0) {
            obj lp = lastpair(bseq2); assert(ispair(lp));
            cdr(lp) = sref(0); sref(0) = bseq2;
          }
        } else { /* regular 1-arm branch */
          n = length(ra);
          hreserve(hbsz(3)*2, sp-r); 
          *--hp = sref(0); *--hp = pbr->g;  
          *--hp = obj_from_size(PAIR_BTAG); sref(0) = hendblk(3);
          *--hp = sref(0); *--hp = obj_from_fixnum(n);
          *--hp = obj_from_size(PAIR_BTAG); sref(0) = hendblk(3);
          if (n > 0) {
            obj lp = lastpair(ra); assert(ispair(lp));
            cdr(lp) = sref(0); sref(0) = ra;
          }
        }
        goto more;
      } break;
      default: assert(0);
    } 
  }
out:
  sdrop(2);
  return hp;
}

/* protects registers from r to sp, in: ra=str, out: ra=codevec/eof */
static obj *rds_stoc(obj *r, obj *sp, obj *hp)
{
  int *d = dupstring(stringdata(ra));
  /* fprintf(stderr, "** rds_stoc(%s)\n", sdatachars(d)); */
  ra = mkiport_string(sp-r, sialloc(sdatachars(d), d));
  hp = rds_seq(r, sp, hp);  /* ra=port => ra=revcodelist/eof */
  if (!iseof(ra)) hp = revlist2vec(r, sp, hp); /* ra => ra */
  return hp;
}


/* protects registers from r to sp */
static obj *rds_intgtab(obj *r, obj *sp, obj *hp)
{
  int i, n = sizeof(intgtab)/sizeof(intgtab[0]);
  char lbuf[60], *lcode;
  if (!intgtab_sorted) sort_intgtab(n);
  for (i = 0; i < n; ++i) {
    struct intgtab_entry *pe = &intgtab[i];
    if (!pe->igname) continue; 
    lcode = pe->lcode;
    if (!lcode) switch (pe->arity) {
      case 0:
        lcode = lbuf; assert(pe->enc);
        sprintf(lbuf, "%%0%s]0", pe->enc); 
        break; 
      case 1: 
        lcode = lbuf; assert(pe->enc);
        sprintf(lbuf, "%%1_!%s]0", pe->enc);   // "%%1.0%s]1"
        break;
      case 2: 
        lcode = lbuf; assert(pe->enc);
        sprintf(lbuf, "%%2_!%s]0", pe->enc);  // %%2.1,.1%s]2
        break;
      case 3: 
        lcode = lbuf; assert(pe->enc); 
        sprintf(lbuf, "%%3_!%s]0", pe->enc); // %%3.2,.2,.2%s]3
        break;
      default: assert(0); 
    }
    if (!lcode || *lcode == 0) continue;
    ra = mksymbol(internsym(pe->igname));
    hp = rds_global_loc(r, sp, hp); /* ra->ra */
    spush(ra);
    ra = mkiport_string(sp-r, sialloc(lcode, NULL));
    hp = rds_seq(r, sp, hp);  /* ra=port => ra=revcodelist/eof */
    if (!iseof(ra)) hp = revlist2vec(r, sp, hp); /* ra => ra */
    if (!iseof(ra)) hp = close0(r, sp, hp); /* ra => ra */
    if (!iseof(ra)) cdr(spop()) = ra;
  }
  return hp;
}

/* protects registers from r to sp=r+2; returns new hp */
static obj *init_module(obj *r, obj *sp, obj *hp, const char **mod)
{
  const char **ent;
  /* make sure we are called in a clean vm state */
  assert(r == cxg_regs); assert(sp-r == 2); /* k, ra (for temp use) */
  /* go over module entries and install/execute */
  for (ent = mod; ent[1] != NULL; ent += 2) {
    const char *name = ent[0], *data = ent[1];
    /* fprintf(stderr, "## initializing: %s\n%s\n", name?name:"NULL", data); */
    if (name != NULL) {
      /* install sexp-encoded syntax-rules as a transformer */
      obj sym = mksymbol(internsym((char*)name));
      obj al = cx__2Atransformers_2A, bnd = mknull();
      assert(ispair(al)); /* basic transformers already installed */
      /* look for existing binding (we allow redefinition) */
      while (al != mknull()) {
        obj ael = car(al);
        if (car(ael) != sym) { al = cdr(al); continue; }
        bnd = ael; break;
      }
      /* or add new binding */
      if (bnd == mknull()) { /* acons (sym . #f) */
        hreserve(hbsz(3)*2, sp-r);
        *--hp = obj_from_bool(0); *--hp = sym;
        *--hp = obj_from_size(PAIR_BTAG); bnd = hendblk(3);
        *--hp = cx__2Atransformers_2A; *--hp = bnd;
        *--hp = obj_from_size(PAIR_BTAG); cx__2Atransformers_2A = hendblk(3);
      }
      /* sexp-decode data into the cdr of the binding */
      spush(bnd); /* protect from gc */
      ra = mkiport_string(sp-r, sialloc((char*)data, NULL));
      hp = rds_sexp(r, sp, hp);  /* ra=port => ra=sexp/eof */      
      bnd = spop();
      assert(ispair(bnd) && (ispair(ra) || issymbol(ra)));
      cdr(bnd) = ra;
    } else {
      /* execute code-encoded thunk */
      obj *ip;
#ifdef VM_AC_IN_REG
      obj ac;
#endif      
      ra = mkiport_string(sp-r, sialloc((char*)data, NULL));
      hp = rds_seq(r, sp, hp);  /* ra=port => ra=revcodelist/eof */
      if (!iseof(ra)) hp = revlist2vec(r, sp, hp); /* ra => ra */
      if (!iseof(ra)) hp = close0(r, sp, hp); /* ra => ra */
      assert(!iseof(ra));
      /* ra is a thunk closure to execute */
      rd = ra;
      ra = obj_from_fixnum(0); /* argc, shadow ac */
      rx = obj_from_fixnum(0); /* shadow ip */ 
      rs = obj_from_fixnum(VM_REGC); /* sp */
      do { /* unwindi trampoline */
        reload_ac(); /* ra => ac */
        reload_ip(); /* rd/rx => ip */
        reload_sp(); /* rs => sp */
        hp = (ins_from_obj(*ip))(IARGS1);
      } while (trampcnd());
      /* r[0] = k, r[1] = random result */
    }
  }
  return hp;
}

/* protects registers from r to sp=r+2; returns new hp */
static obj *init_modules(obj *r, obj *sp, obj *hp)
{
  extern char* s_code[]; /* s.c */
  extern char* t_code[]; /* t.c */
  hp = init_module(r, sp, hp, (const char **)s_code);
  hp = init_module(r, sp, hp, (const char **)t_code);
  return hp;
}
