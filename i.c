/* i.c -- instructions */

#include "n.h"
#include "i.h"

/* imports */
extern obj cx__2Aglobals_2A;
extern obj cx__2Atransformers_2A;
extern obj cx_continuation_2Dadapter_2Dcode;
extern obj cx_callmv_2Dadapter_2Dclosure;

#define istagged(o, t) istagged_inlined(o, t) 

/* forwards */
static struct intgtab_entry *lookup_integrable(int sym);
static int isintegrable(obj x);
static struct intgtab_entry *integrabledata(obj x);
static obj mkintegrable(struct intgtab_entry *);
static int integrable_type(struct intgtab_entry *pi);
static const char *integrable_global(struct intgtab_entry *pi);
static const char *integrable_code(struct intgtab_entry *pi, int n);
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
#define rz   (r[5])   /* red zone for stack pointer (r + len - rsz) */

/* the rest of the register file is used as a stack */
#define VM_REGC       6    /* r[0] ... r[5] */
#define VM_STACK_LEN  1000 /* r[6] ... r[1005] */
#define VM_STACK_RSZ  64   /* red zone for overflow checks */
#define VM_STACK_GSZ  (VM_STACK_LEN-VM_STACK_RSZ)

/* vm closure representation */
#ifdef NDEBUG /* quick */
#define isvmclo(x)    (isobjptr(x) && isobjptr(hblkref(x, 0)))
#define vmcloref(x,i) hblkref(x, i)
#define vmclolen(x)   hblklen(x)
#define vmclobsz(c)   hbsz(c)
#define hpushvmclo(c) hendblk(c)
#else /* slow but thorough */
#define isvmclo       isprocedure
#define vmcloref      *procedureref
#define vmclolen      procedurelen
#define vmclobsz(c)   hbsz(c)
#define hpushvmclo(c) hendblk(c)
#endif

/* vm tuple representation (c != 1) */
#define istuple(x)    (isrecord(x) && recordrtd(x) == 0)  
#define tupleref      recordref
#define tuplelen      recordlen
#define tuplebsz(c)   hbsz((c)+2)
#define hpushtuple(c) (*--hp = 0, *--hp = obj_from_size(RECORD_BTAG), hendblk((c)+2))

/* in/re-loading gc-save shadow registers */
#define unload_ip()   (rx = obj_from_fixnum(ip - &vectorref(vmcloref(rd, 0), 0)))
#define reload_ip()   (ip = &vectorref(vmcloref(rd, 0), fixnum_from_obj(rx)))
#define unload_sp()   (rs = obj_from_fixnum(sp - r))
#define reload_sp()   (sp = r + fixnum_from_obj(rs))

/* access to stack, display, global cells */
#define sref(i)       (sp[-(i)-1])
#define dref(i)       (vmcloref(rd, (i)+1))
#define gref(p)       (boxref(p))
#ifdef _DEBUG
static void _sck(obj *s) { 
  assert(s != NULL);
  assert(s >= cxg_regs + VM_REGC);
  assert(s < cxg_rend); 
}
#define spush(o)      (_sck(sp), *sp++ = o)
#define spop()        (_sck(sp), *--sp)
#define sdrop(n)      (_sck(sp), sp -= (n))
#define sgrow(n)      (_sck(sp), sp += (n))
#else
#define spush(o)      (*sp++ = o)
#define spop()        (*--sp)
#define sdrop(n)      (sp -= (n))
#define sgrow(n)      (sp += (n))
#endif

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
#define check_sp()    if (unlikely(sp > (obj*)rz)) fail("stack overflow")
#ifdef VM_MUSTTAIL_GUARANTEE
  #define callsubi()  check_sp(); reload_ip(); gonexti()
  #define retfromi()  reload_ip(); gonexti()
  #define trampcnd()  (0)
#else
  #define callsubi()  check_sp(); unload_sp(); return hp
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
obj vmcases[13] = { 
  (obj)vmhost, (obj)vmhost, (obj)vmhost, (obj)vmhost, 
  (obj)vmhost, (obj)vmhost, (obj)vmhost, (obj)vmhost,
  (obj)vmhost, (obj)vmhost, (obj)vmhost, (obj)vmhost,
  (obj)vmhost
};
/* vmhost procedure */
static obj vmhost(obj pc)
{
  ILOCALS;
  int rc = cxg_rc, i;
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
    rz = (obj)(r + VM_STACK_GSZ); /* sp red zone */
    do { /* unwindi trampoline */
      reload_ac(); /* ra => ac */
      reload_ip(); /* rd/rx => ip */
      reload_sp(); /* rs => sp */
      hp = (ins_from_obj(*ip))(IARGS1);
    } while (likely(trampcnd()));
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
    for (i = 3; i < VM_REGC; ++i) r[i] = 0; 
    rz = (obj)(r + VM_STACK_GSZ); /* sp red zone */
    hp = rds_stox(r, r+VM_REGC, hp); /* r[1] -> r[1] */
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
    for (i = 3; i < VM_REGC; ++i) r[i] = 0;
    rz = (obj)(r + VM_STACK_GSZ); /* sp red zone */
    hp = rds_stoc(r, r+VM_REGC, hp); /* r[1] -> r[1] */
    r[2] = r[1]; r[1] = obj_from_ktrap();
    /* r[0] = k; r[1] = ek; r[2] = code */
    pc = objptr_from_obj(r[0])[0];
    rc = 3;
    goto jump; }

  case 4: /* find-integrable-encoding */
    /* r[0] = clo, r[1] = k, r[2] = id, r[3] = argc */
    { assert(rc == 4);
    r[2] = obj_from_bool(0);;
    r[0] = r[1]; r[1] = obj_from_ktrap();
    pc = objptr_from_obj(r[0])[0];
    rc = 3;
    goto jump; }
      
  case 5: /* encode-integrable */
    /* r[0] = clo, r[1] = k, r[2] = argc, r[3] = pe, r[4] = port */
    { assert(rc == 5);
    assert(0);
    r[0] = r[1]; r[1] = obj_from_ktrap();
    pc = objptr_from_obj(r[0])[0];
    rc = 3;
    goto jump; }

  case 6: /* install-global-lambdas */
    /* r[0] = clo, r[1] = k */
    { assert(rc == 2);
    r[0] = r[1];
    for (i = 2; i < VM_REGC; ++i) r[i] = 0;
    rz = (obj)(r + VM_STACK_GSZ); /* sp red zone */
    hp = rds_intgtab(r, r+VM_REGC, hp);
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
    for (i = 2; i < VM_REGC; ++i) r[i] = 0;
    rz = (obj)(r + VM_STACK_GSZ); /* sp red zone */
    hp = init_modules(r, r+VM_REGC, hp);
    r[1] = obj_from_ktrap();
    r[2] = obj_from_void(0);
    pc = objptr_from_obj(r[0])[0];
    rc = 3;
    goto jump; }

  case 8: /* integrable? */
    /* r[0] = clo, r[1] = k, r[2] = obj */
    { assert(rc == 3);
    r[2] = obj_from_bool(isintegrable(r[2]));
    r[0] = r[1]; r[1] = obj_from_ktrap();
    pc = objptr_from_obj(r[0])[0];
    rc = 3;
    goto jump; }

  case 9: /* lookup-integrable */
    /* r[0] = clo, r[1] = k, r[2] = id */
    { assert(rc == 3);
    if (issymbol(r[2])) {
      int sym = getsymbol(r[2]);
      struct intgtab_entry *pe = lookup_integrable(sym);
      r[2] = pe ? mkintegrable(pe) : obj_from_bool(0);
    } else r[2] = obj_from_bool(0);
    r[0] = r[1]; r[1] = obj_from_ktrap();
    pc = objptr_from_obj(r[0])[0];
    rc = 3;
    goto jump; }

  case 10: /* integrable-type */
    /* r[0] = clo, r[1] = k, r[2] = ig */
    { assert(rc == 3);
    if (isintegrable(r[2])) {
      int it = integrable_type(integrabledata(r[2]));
      r[2] = it ? obj_from_char(it) : obj_from_bool(0);
    } else r[2] = obj_from_bool(0);
    r[0] = r[1]; r[1] = obj_from_ktrap();
    pc = objptr_from_obj(r[0])[0];
    rc = 3;
    goto jump; }

  case 11: /* integrable-global */
    /* r[0] = clo, r[1] = k, r[2] = ig */
    { assert(rc == 3);
    if (isintegrable(r[2])) {
      const char *igs = integrable_global(integrabledata(r[2]));
      r[2] = igs ? mksymbol(internsym((char*)igs)) : obj_from_bool(0);
    } else r[2] = obj_from_bool(0);
    r[0] = r[1]; r[1] = obj_from_ktrap();
    pc = objptr_from_obj(r[0])[0];
    rc = 3;
    goto jump; }

  case 12: /* integrable-code */
    /* r[0] = clo, r[1] = k, r[2] = ig, r[3] = i */
    { assert(rc == 4);
    if (isintegrable(r[2]) && is_fixnum_obj(r[3])) {
      const char *cs = integrable_code(integrabledata(r[2]), fixnum_from_obj(r[3]));
      r[2] = cs ? hpushstr(3, newstring((char*)cs)) : obj_from_bool(0);
    } else r[2] = obj_from_bool(0);
    r[0] = r[1]; r[1] = obj_from_ktrap();
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

#define fail(msg) do { ac = (obj)msg; musttail return cxi_fail(IARGS); } while (0)
#define failtype(x, msg) do { ac = (x); spush((obj)msg); musttail return cxi_failactype(IARGS); } while (0) 
#define failactype(msg) do { spush((obj)msg); musttail return cxi_failactype(IARGS); } while (0) 

#define ckp(x) do { obj _x = (x); if (unlikely(!ispair(_x))) \
  { ac = _x; spush((obj)"pair"); musttail return cxi_failactype(IARGS); } } while (0)
#define ckl(x) do { obj _x = (x); if (unlikely(!islist(_x))) \
  { ac = _x; spush((obj)"list"); musttail return cxi_failactype(IARGS); } } while (0)
#define ckv(x) do { obj _x = (x); if (unlikely(!isvector(_x))) \
  { ac = _x; spush((obj)"vector"); musttail return cxi_failactype(IARGS); } } while (0)
#define ckc(x) do { obj _x = (x); if (unlikely(!ischar(_x))) \
  { ac = _x; spush((obj)"char"); musttail return cxi_failactype(IARGS); } } while (0)
#define cks(x) do { obj _x = (x); if (unlikely(!isstring(_x))) \
  { ac = _x; spush((obj)"string"); musttail return cxi_failactype(IARGS); } } while (0)
#define cki(x) do { obj _x = (x); if (unlikely(!is_fixnum_obj(_x))) \
  { ac = _x; spush((obj)"fixnum"); musttail return cxi_failactype(IARGS); } } while (0)
#define ckj(x) do { obj _x = (x); if (unlikely(!is_flonum_obj(_x))) \
  { ac = _x; spush((obj)"flonum"); musttail return cxi_failactype(IARGS); } } while (0)
#define ckn(x) do { obj _x = (x); if (unlikely(!is_fixnum_obj(_x) && !is_flonum_obj(_x))) \
  { ac = _x; spush((obj)"number"); musttail return cxi_failactype(IARGS); } } while (0)
#define ckk(x) do { obj _x = (x); if (unlikely(!is_fixnum_obj(_x) || fixnum_from_obj(_x) < 0)) \
  { ac = _x; spush((obj)"nonnegative fixnum"); musttail return cxi_failactype(IARGS); } } while (0)
#define cky(x) do { obj _x = (x); if (unlikely(!issymbol(_x))) \
  { ac = _x; spush((obj)"symbol"); musttail return cxi_failactype(IARGS); } } while (0)
#define ckr(x) do { obj _x = (x); if (unlikely(!isiport(_x))) \
  { ac = _x; spush((obj)"input port"); musttail return cxi_failactype(IARGS); } } while (0)
#define ckw(x) do { obj _x = (x); if (unlikely(!isoport(_x))) \
  { ac = _x; spush((obj)"output port"); musttail return cxi_failactype(IARGS); } } while (0)
#define ckx(x) do { obj _x = (x); if (unlikely(!isvmclo(_x))) \
  { ac = _x; spush((obj)"procedure"); musttail return cxi_failactype(IARGS); } } while (0)
#define ckz(x) do { obj _x = (x); if (unlikely(!isbox(_x))) \
  { ac = _x; spush((obj)"box, cell, or promise"); musttail return cxi_failactype(IARGS); } } while (0)
#define ckg(x) do { obj _x = (x); if (unlikely(!isintegrable(_x))) \
  { ac = _x; spush((obj)"integrable entry"); musttail return cxi_failactype(IARGS); } } while (0)


define_instruction(halt) { 
  unwindi(0); 
}

define_instruction(lit) { 
  ac = *ip++; 
  gonexti();
}

define_instruction(sref) {
  int i = fixnum_from_obj(*ip++);
  ac = sref(i);
  gonexti();
}  

define_instruction(dref) {
  int i = fixnum_from_obj(*ip++);
  ac = dref(i); 
  gonexti();
}

define_instruction(gref) {
  obj p = *ip++; 
  ac = gref(p);
  gonexti();
}

define_instruction(iref) {
  ac = boxref(ac);
  gonexti();
}

define_instruction(iset) {
  boxref(ac) = spop();
  gonexti();
}

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

define_instruction(br) { 
  int dx = fixnum_from_obj(*ip++); 
  ip += dx; 
  gonexti(); 
}

define_instruction(brt) { 
  int dx = fixnum_from_obj(*ip++);
  if (ac) ip += dx; 
  gonexti(); 
}

define_instruction(brnot) { 
  int dx = fixnum_from_obj(*ip++);
  if (!ac) ip += dx; 
  gonexti(); 
}

define_instruction(andbo) {
  if (ac) { /* go to next binary instruction w/2 args */
    ac = spop();
  } else { /* skip the next instruction, drop its args */
    sdrop(2);
    ip += 1;
  }
  gonexti(); 
}

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

define_instruction(gloc) {
  ac = *ip++;
  gonexti();
}

define_instruction(gset) {
  obj p = *ip++;
  gref(p) = ac;
  gonexti();
}

define_instruction(appl) {
  int n, i; obj l = spop(), t = l;
  for (n = 0; ispair(t); t = cdr(t)) ++n; sgrow(n);
  for (i = 0; i < n; ++i, l = cdr(l)) sref(i) = car(l);
  ckx(ac); rd = ac; rx = obj_from_fixnum(0);
  ac = obj_from_fixnum(n); /* argc */
  callsubi();
}

define_instruction(cwmv) {
  obj t = ac, x = spop();
  ckx(t); ckx(x);
  if (vmcloref(x, 0) == cx_continuation_2Dadapter_2Dcode) {
    /* arrange call of t with x as continuation */
    int n = vmclolen(x) - 1;
    assert((cxg_rend - cxg_regs - VM_REGC) > n);
    sp = r + VM_REGC; /* stack is empty */
    memcpy(sp, &vmcloref(x, 1), n*sizeof(obj));
    sp += n; /* contains n elements now */
    rd = t; rx = obj_from_fixnum(0); 
    ac = obj_from_fixnum(0);
    callsubi();
  } else { 
    /* arrange return to cwmv code w/x */
    spush(x);
    spush(cx_callmv_2Dadapter_2Dclosure); 
    spush(obj_from_fixnum(0));
    /* call the producer */
    rd = t; rx = obj_from_fixnum(0); ac = obj_from_fixnum(0); 
    callsubi(); 
  }
}

define_instruction(rcmv) {
  /* single-value producer call returns here with result in ac, cns on stack */
  obj val = ac, x = spop();
  /* tail-call the consumer with the returned value */
  spush(val); ac = obj_from_fixnum(1);
  rd = x; rx = obj_from_fixnum(0); 
  callsubi();
}

define_instruction(sdmv) {
  /* sending values on stack, ac contains argc */
  if (ac == obj_from_fixnum(1)) {
    /* can return anywhere, including rcmv */
    ac = spop();
    rx = spop();
    rd = spop();
    retfromi();
  } else {
    /* can only pseudo-return to rcmv */
    int n = fixnum_from_obj(ac), m = 3, i;
    if (sref(n) == obj_from_fixnum(0) && sref(n+1) == cx_callmv_2Dadapter_2Dclosure) {
      /* tail-call the consumer with the produced values */
      rd = sref(n+2); rx = obj_from_fixnum(0); /* cns */
      /* NB: can be sped up for popular cases: n == 0, n == 2 */
      memmove((void*)(sp-n-m), (void*)(sp-n), (size_t)n*sizeof(obj));
      sdrop(m); callsubi();
    } else { /* return args as a tuple (n != 1) */
      hp_reserve(tuplebsz(n));
      for (i = n-1; i >= 0; --i) *--hp = sref(i);
      ac = hpushtuple(n);
      sdrop(n);
      rx = spop();
      rd = spop();
      retfromi();
    }
  }
}

define_instruction(lck) {
  int m = fixnum_from_obj(*ip++);
  int n; cki(sref(m)); ckx(sref(m+1));
  n = (int)(sp-m-(r+VM_REGC));
  hp_reserve(vmclobsz(n+1));
  hp -= n; memcpy(hp, sp-n-m, n*sizeof(obj));
  *--hp = cx_continuation_2Dadapter_2Dcode;
  ac = hpushvmclo(n+1);
  gonexti();
}

define_instruction(lck0) {
  int n; cki(sref(0)); ckx(sref(1));
  n = (int)(sp-(r+VM_REGC));
  hp_reserve(vmclobsz(n+1));
  hp -= n; memcpy(hp, sp-n, n*sizeof(obj));
  *--hp = cx_continuation_2Dadapter_2Dcode;
  ac = hendblk(n+1);
  gonexti();
}

define_instruction(wck) {
  obj x = ac, t = spop(); int n; ckx(t); ckx(x);
  if (vmcloref(x, 0) != cx_continuation_2Dadapter_2Dcode) 
    failactype("continuation");
  n = vmclolen(x) - 1;
  assert((cxg_rend - cxg_regs - VM_REGC) > n);
  sp = r + VM_REGC; /* stack is empty */
  memcpy(sp, &vmcloref(x, 1), n*sizeof(obj));
  sp += n; /* contains n elements now */
  rd = t; rx = obj_from_fixnum(0); 
  ac = obj_from_fixnum(0);
  callsubi();
}

define_instruction(wckr) {
  obj x = ac, o = spop(); int n; ckx(x);
  if (vmcloref(x, 0) != cx_continuation_2Dadapter_2Dcode) 
    failactype("continuation");
  n = vmclolen(x) - 1;
  assert((cxg_rend - cxg_regs - VM_REGC) > n);
  sp = r + VM_REGC; /* stack is empty */
  memcpy(sp, &vmcloref(x, 1), n*sizeof(obj));
  sp += n;
  ac = o;
  rx = spop();
  rd = spop();
  retfromi();
}

define_instruction(rck) {
  /* in: ac:argc, args on stack, rd display is saved stack */
  if (ac == obj_from_fixnum(1)) { /* easy, popular case */
    ac = rd; 
    goi(wckr);
  } else { /* multiple results case */
    int c = fixnum_from_obj(ac), n = vmclolen(rd) - 1, i;
    obj *ks = &vmcloref(rd, 1), *ke = ks + n;
    if (ke-ks > 3 && *--ke == obj_from_fixnum(0) && *--ke == cx_callmv_2Dadapter_2Dclosure) {
      obj *sb = r + VM_REGC;
      rd = *--ke; rx = obj_from_fixnum(0); n = ke - ks; /* cns */
      /* arrange stack as follows: [ks..ke] [arg ...] */
      assert((cxg_rend - cxg_regs - VM_REGC) > n + c);
      if (c) memmove(sb+n, sp-c, c*sizeof(obj));
      memcpy(sb, ks, n*sizeof(obj));
      sp = sb+n+c; callsubi();
    } else { /* return args as a tuple (c != 1) */
      hp_reserve(tuplebsz(c));
      for (i = c-1; i >= 0; --i) *--hp = sref(i);
      ac = hpushtuple(c);
      sdrop(c);
      spush(ac);
      ac = rd;
      goi(wckr);
    }
  } 
}

define_instruction(save) {
  int dx = fixnum_from_obj(*ip++); 
  spush(rd);
  spush(obj_from_fixnum(ip + dx - &vectorref(vmcloref(rd, 0), 0)));  
  gonexti();
}

define_instruction(push) { spush(ac); gonexti(); }

define_instruction(jdceq) {
  obj v = *ip++, i = *ip++;
  if (ac == v) {
    rd = dref(fixnum_from_obj(i));
    rx = obj_from_fixnum(0);
    callsubi();
  }
  gonexti();
}

define_instruction(jdcge) {
  obj v = *ip++, i = *ip++;
  if (ac >= v) { /* unsigned tagged fixnums can be compared as-is */
    rd = dref(fixnum_from_obj(i));
    rx = obj_from_fixnum(0);
    callsubi();
  }
  gonexti();
}

define_instruction(jdref) {
  int i = fixnum_from_obj(*ip++); 
  rd = dref(i); 
  rx = obj_from_fixnum(0);
  callsubi();
}

define_instruction(call) {
  int n = fixnum_from_obj(*ip++); 
  ckx(ac); rd = ac; rx = obj_from_fixnum(0); 
  ac = obj_from_fixnum(n); /* argc */
  callsubi();
}

define_instruction(scall) {
  int m = fixnum_from_obj(*ip++), n = fixnum_from_obj(*ip++);
  ckx(ac); rd = ac; rx = obj_from_fixnum(0); 
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

define_instruction(pop) { 
  ac = spop(); 
  gonexti();
}

define_instruction(atest) {
  obj no = *ip++;
  if (unlikely(ac != no)) fail("argument count error on entry");
  gonexti();
}

define_instruction(shrarg) {
  int n = fixnum_from_obj(*ip++), c = fixnum_from_obj(ac), m = c-n; 
  /* rest arg ends up on top */
  if (unlikely(m < 0)) {
    fail("argument count error on entry (not enough arguments)");
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
  ac = obj_from_fixnum(n+1);
  gonexti();
}

define_instruction(aerr) {
  fail("argument count error on entry");
  gonexti();
}

define_instruction(shlit) { 
  spush(ac); 
  ac = *ip++; 
  gonexti();
}

define_instruction(shi0) { 
  spush(ac);
  ac = obj_from_fixnum(0);
  gonexti();
}


/* type checks */

define_instruction(ckp) { ckp(ac); gonexti(); }
define_instruction(ckl) { ckl(ac); gonexti(); }
define_instruction(ckv) { ckv(ac); gonexti(); }
define_instruction(ckc) { ckc(ac); gonexti(); }
define_instruction(cks) { cks(ac); gonexti(); }
define_instruction(cki) { cki(ac); gonexti(); }
define_instruction(ckj) { ckj(ac); gonexti(); }
define_instruction(ckn) { ckn(ac); gonexti(); }
define_instruction(ckk) { ckk(ac); gonexti(); }
define_instruction(cky) { cky(ac); gonexti(); }
define_instruction(ckr) { ckr(ac); gonexti(); }
define_instruction(ckw) { ckw(ac); gonexti(); }
define_instruction(ckx) { ckx(ac); gonexti(); }
define_instruction(ckz) { ckz(ac); gonexti(); }


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

define_instruction(unbox) { ckz(ac); ac = boxref(ac); gonexti(); }
define_instruction(setbox) { ckz(ac); boxref(ac) = spop(); gonexti(); }

define_instruction(box) {
  hp_reserve(hbsz(1+1));
  *--hp = ac;
  *--hp = obj_from_size(BOX_BTAG);
  ac = hendblk(1+1);
  gonexti();
}

define_instruction(car) { ckp(ac); ac = car(ac); gonexti(); }
define_instruction(cdr) { ckp(ac); ac = cdr(ac); gonexti(); }
define_instruction(setcar) { ckp(ac); car(ac) = spop(); gonexti(); }
define_instruction(setcdr) { ckp(ac); cdr(ac) = spop(); gonexti(); }

define_instruction(caar) { ckp(ac); ac = car(ac); ckp(ac); ac = car(ac); gonexti(); }
define_instruction(cadr) { ckp(ac); ac = cdr(ac); ckp(ac); ac = car(ac); gonexti(); }
define_instruction(cdar) { ckp(ac); ac = car(ac); ckp(ac); ac = cdr(ac); gonexti(); }
define_instruction(cddr) { ckp(ac); ac = cdr(ac); ckp(ac); ac = cdr(ac); gonexti(); }


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
  hp_reserve(hbsz(2+1)*n);
  for (ac = mknull(), i = n-1; i >= 0; --i) {
    *--hp = ac;      /* cdr */
    *--hp = sref(i); /* car */
    *--hp = obj_from_size(PAIR_BTAG);
    ac = hendblk(2+1);
  }
  sdrop(n);
  gonexti();
}

define_instruction(lmk) {
  int i, n; obj v; ckk(ac);
  n = fixnum_from_obj(ac); 
  hp_reserve(hbsz(2+1)*n); v = sref(0);
  ac = mknull();
  for (i = 0; i < n; ++i) {
    *--hp = ac; /* cdr */
    *--hp = v;  /* car */
    *--hp = obj_from_size(PAIR_BTAG);
    ac = hendblk(2+1);
  }
  sdrop(1);
  gonexti();
}

define_instruction(llen) {
  int n = 0;
  while (ispair(ac)) { ac = cdr(ac); ++n; }
  ac = obj_from_fixnum(n);
  gonexti();
}

define_instruction(lget) {
  obj x = spop(); int c;
  ckk(x); c = fixnum_from_obj(x);
  while (c-- > 0) { ckp(ac); ac = cdr(ac); }
  ckp(ac); ac = car(ac);
  gonexti();
}

define_instruction(lput) {
  obj x = spop(); int c;
  ckk(x); c = fixnum_from_obj(x);
  while (c-- > 0) { ckp(ac); ac = cdr(ac); }
  ckp(ac); car(ac) = spop();
  gonexti();
}


define_instruction(memq) {
  obj l = spop(); 
  for (; ispair(l); l = cdr(l)) {
    if (car(l) == ac) { ac = l; gonexti(); }
  }
  ac = obj_from_bool(0);
  gonexti();
}

define_instruction(memv) {
  ac = ismemv(ac, spop()); /* FIXME: ckp() */
  gonexti();
}

define_instruction(meme) {
  ac = ismember(ac, spop()); /* FIXME: ckp() */
  gonexti();
}

define_instruction(assq) {
  obj l = spop(); 
  for (; ispair(l); l = cdr(l)) {
    obj p = car(l); if (ispair(p) && car(p) == ac) { ac = p; gonexti(); }
  }
  ac = obj_from_bool(0);
  gonexti();
}

define_instruction(assv) {
  ac = isassv(ac, spop()); /* FIXME: ckp() */
  gonexti();
}

define_instruction(asse) {
  ac = isassoc(ac, spop()); /* FIXME: ckp() */
  gonexti();
}

define_instruction(ltail) {
  obj x = spop(); int c;
  ckk(x); c = fixnum_from_obj(x);
  while (c-- > 0) { ckp(ac); ac = cdr(ac); }
  gonexti();
}

define_instruction(lpair) {
  obj l = obj_from_bool(0);
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
  while (ispair(ac)) t = cdr(ac), cdr(ac) = v, v = ac, ac = t;
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
  for (i = 0; i < n; ++i) {
    obj x = sref(i); ckc(x); s[i] = char_from_obj(x);
  }
  sdrop(n); ac = o;
  gonexti();
}

define_instruction(smk) {
  int n, c; obj x = spop(); 
  ckk(ac); ckc(x);
  n = fixnum_from_obj(ac), c = char_from_obj(x);
  ac = hpushstr(sp-r, allocstring(n, c)); 
  gonexti();
}

define_instruction(slen) {
  cks(ac);
  ac = obj_from_fixnum(stringlen(ac));
  gonexti();
}

define_instruction(sget) {
  obj x = spop(); int i; 
  cks(ac); ckk(x); 
  i = fixnum_from_obj(x); 
  if (i >= stringlen(ac)) failtype(x, "valid string index");
  ac = obj_from_char(*stringref(ac, i));
  gonexti();
}

define_instruction(sput) {
  obj x = spop(), y = spop(); int i; 
  cks(ac); ckk(x); ckc(y); 
  i = fixnum_from_obj(x); 
  if (i >= stringlen(ac)) failtype(x, "valid string index");
  *stringref(ac, i) = char_from_obj(y);
  gonexti();
}

define_instruction(scat) {
  obj x = ac, y = spop(); int *d; 
  cks(x); cks(y);
  d = stringcat(stringdata(x), stringdata(y));
  ac = hpushstr(sp-r, d);
  gonexti();
}

define_instruction(ssub) {
  obj x = spop(), y = spop(); int is, ie, *d; 
  cks(ac); ckk(x); ckk(y); 
  is = fixnum_from_obj(x), ie = fixnum_from_obj(y);
  if (is > ie) failtype(x, "valid start string index");
  if (ie > stringlen(ac)) failtype(y, "valid end string index");
  d = substring(stringdata(ac), is, ie);
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
  int i, n; obj v; ckk(ac);
  n = fixnum_from_obj(ac); 
  hp_reserve(hbsz(n+1)); v = sref(0);
  for (i = 0; i < n; ++i) *--hp = v;
  *--hp = obj_from_size(VECTOR_BTAG);
  ac = hendblk(n+1);
  sdrop(1);
  gonexti();
}

define_instruction(vlen) {
  ckv(ac);
  ac = obj_from_fixnum(vectorlen(ac));
  gonexti();
}

define_instruction(vget) {
  obj x = spop(); int i; 
  ckv(ac); ckk(x); 
  i = fixnum_from_obj(x);
  if (i >= vectorlen(ac)) failtype(x, "valid vector index");
  ac = vectorref(ac, i);
  gonexti();
}

define_instruction(vput) {
  obj x = spop(), y = spop(); int i; 
  ckv(ac); ckk(x);
  i = fixnum_from_obj(x);
  if (i >= vectorlen(ac)) failtype(x, "valid vector index");
  vectorref(ac, i) = y;
  gonexti();
}

define_instruction(vcat) {
  obj x = ac, y = sref(0); int n1, n2, n;
  ckv(x); ckv(y);
  n1 = vectorlen(x), n2 = vectorlen(y), n = n1 + n2;
  hp_reserve(hbsz(n+1));
  /* NB: vectorref fails to return pointer to empty vector's start */
  hp -= n2; if (n2) memcpy(hp, &vectorref(y, 0), n2*sizeof(obj));
  hp -= n1; if (n1) memcpy(hp, &vectorref(x, 0), n1*sizeof(obj));
  *--hp = obj_from_size(VECTOR_BTAG);
  ac = hendblk(n+1);
  sdrop(1);
  gonexti();
}

define_instruction(vtol) {
  obj l = mknull(); int n;
  ckv(ac); n = vectorlen(ac);
  hp_reserve(hbsz(2+1)*n);
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
  obj l = mknull(); int n;
  cks(ac); n = stringlen(ac);
  hp_reserve(hbsz(2+1)*n);
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
  cky(ac);
  ac = hpushstr(sp-r, newstring(symbolname(getsymbol(ac))));
  gonexti();
}

define_instruction(stoy) {
  cks(ac);
  ac = mksymbol(internsym(stringchars(ac)));
  gonexti();
}

define_instruction(itos) {
  char buf[35], *s = buf + sizeof(buf) - 1; 
  int neg = 0; long num, radix;
  obj x = ac, y = spop(); cki(x); ckk(y);
  num = fixnum_from_obj(x), radix = fixnum_from_obj(y);
  if (num < 0) { neg = 1; num = -num; }
  if (radix < 2 || radix > 10 + 'z' - 'a') failtype(y, "valid radix");
  *s = 0;
  do { int d = num % radix; *--s = d < 10 ? d + '0' : d - 10 + 'a'; }
  while (num /= radix);
  if (neg) *--s = '-';
  ac = hpushstr(sp-r, newstring(s));  
  gonexti();
}

define_instruction(stoi) {
  char *e, *s; long l, radix;
  obj x = ac, y = spop(); cks(x); ckk(y);
  s = stringchars(x); radix = fixnum_from_obj(y);
  if (radix < 2 || radix > 10 + 'z' - 'a') failtype(y, "valid radix");
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
  ckc(ac);
  ac = obj_from_fixnum(char_from_obj(ac));
  gonexti();
}

define_instruction(itoc) {
  cki(ac); /* TODO: range check? */
  ac = obj_from_char(fixnum_from_obj(ac));
  gonexti();
}

define_instruction(jtos) {
  char buf[30], *s; double d; ckj(ac);
  d = flonum_from_obj(ac); sprintf(buf, "%.15g", d);
  for (s = buf; *s != 0; s++) if (strchr(".eE", *s)) break;
  if (d != d) strcpy(buf, "+nan.0"); else if (d <= -HUGE_VAL) strcpy(buf, "-inf.0");
  else if (d >= HUGE_VAL) strcpy(buf, "+inf.0"); else if (*s == 'E') *s = 'e'; 
  else if (*s == 0) {  *s++ = '.'; *s++ = '0'; *s = 0; }
  ac = hpushstr(sp-r, newstring(buf));
  gonexti();
}

define_instruction(stoj) {
  char *e = "", *s; double d; cks(ac);
  s = stringchars(ac); errno = 0;
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
  char *s; int radix; long l; double d;
  obj x = ac, y = spop(); cks(x); ckk(y);
  s = stringchars(x); radix = fixnum_from_obj(y);
  if (radix < 2 || radix > 10 + 'z' - 'a') failtype(y, "valid radix");
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
  cki(ac);
  ac = obj_from_bool(ac == obj_from_fixnum(0));
  gonexti();
}

define_instruction(iposp) {
  cki(ac);
  ac = obj_from_bool(fixnum_from_obj(ac) > 0);
  gonexti(); 
}

define_instruction(inegp) {
  cki(ac);
  ac = obj_from_bool(fixnum_from_obj(ac) < 0);
  gonexti(); 
}

define_instruction(ievnp) {
  cki(ac);
  ac = obj_from_bool((fixnum_from_obj(ac) & 1) == 0);
  gonexti(); 
}

define_instruction(ioddp) {
  cki(ac);
  ac = obj_from_bool((fixnum_from_obj(ac) & 1) != 0);
  gonexti(); 
}

define_instruction(iadd) {
  obj x = ac, y = spop(); cki(x); cki(y);
  ac = obj_from_fixnum(fxadd(fixnum_from_obj(x), fixnum_from_obj(y)));
  gonexti(); 
}

define_instruction(isub) {
  obj x = ac, y = spop(); cki(x); cki(y);
  ac = obj_from_fixnum(fxsub(fixnum_from_obj(x), fixnum_from_obj(y)));
  gonexti(); 
}

define_instruction(imul) {
  obj x = ac, y = spop(); cki(x); cki(y);
  ac = obj_from_fixnum(fxmul(fixnum_from_obj(x), fixnum_from_obj(y)));
  gonexti();
}

define_instruction(iquo) {
  obj x = ac, y = spop(); cki(x); cki(y);
  if (y == obj_from_fixnum(0)) fail("division by zero");
  ac = obj_from_fixnum(fxquo(fixnum_from_obj(x), fixnum_from_obj(y)));
  gonexti();
}

define_instruction(irem) {
  obj x = ac, y = spop(); cki(x); cki(y);
  if (y == obj_from_fixnum(0)) fail("division by zero");
  ac = obj_from_fixnum(fxrem(fixnum_from_obj(x), fixnum_from_obj(y)));
  gonexti();
}

define_instruction(imin) {
  obj x = ac, y = spop(); cki(x); cki(y);
  ac = fixnum_from_obj(x) < fixnum_from_obj(y) ? x : y;
  gonexti(); 
}

define_instruction(imax) {
  obj x = ac, y = spop(); cki(x); cki(y);
  ac = fixnum_from_obj(x) > fixnum_from_obj(y) ? x : y;
  gonexti(); 
}

define_instruction(ineg) {
  cki(ac);
  ac = obj_from_fixnum(fxneg(fixnum_from_obj(ac)));
  gonexti();
}

define_instruction(iabs) {
  cki(ac);
  ac = obj_from_fixnum(fxabs(fixnum_from_obj(ac)));
  gonexti();
}

define_instruction(itoj) {
  cki(ac);
  ac = obj_from_flonum(sp-r, (double)fixnum_from_obj(ac));
  gonexti();
}

define_instruction(idiv) {
  obj x = ac, y = spop(); cki(x); cki(y);
  if (y == obj_from_fixnum(0)) fail("division by zero");
  ac = obj_from_fixnum(fxdiv(fixnum_from_obj(x), fixnum_from_obj(y)));
  gonexti();
}

define_instruction(imqu) {
  obj x = ac, y = spop(); cki(x); cki(y);
  if (y == obj_from_fixnum(0)) fail("division by zero");
  ac = obj_from_fixnum(fxmqu(fixnum_from_obj(x), fixnum_from_obj(y)));
  gonexti();
}

define_instruction(imlo) {
  obj x = ac, y = spop(); cki(x); cki(y);
  if (y == obj_from_fixnum(0)) fail("division by zero");
  ac = obj_from_fixnum(fxmlo(fixnum_from_obj(x), fixnum_from_obj(y)));
  gonexti();
}

define_instruction(ieuq) {
  obj x = ac, y = spop(); cki(x); cki(y);
  if (y == obj_from_fixnum(0)) fail("division by zero");
  ac = obj_from_fixnum(fxeuq(fixnum_from_obj(x), fixnum_from_obj(y)));
  gonexti();
}

define_instruction(ieur) {
  obj x = ac, y = spop(); cki(x); cki(y);
  if (y == obj_from_fixnum(0)) fail("division by zero");
  ac = obj_from_fixnum(fxeur(fixnum_from_obj(x), fixnum_from_obj(y)));
  gonexti();
}

define_instruction(igcd) {
  obj x = ac, y = spop(); cki(x); cki(y);
  ac = obj_from_fixnum(fxgcd(fixnum_from_obj(x), fixnum_from_obj(y)));
  gonexti();
}

define_instruction(ipow) {
  obj x = ac, y = spop(); cki(x); cki(y);
  ac = obj_from_fixnum(fxpow(fixnum_from_obj(x), fixnum_from_obj(y)));
  gonexti();
}

define_instruction(isqrt) {
  cki(ac); /* TODO: check for negative */
  ac = obj_from_fixnum(fxsqrt(fixnum_from_obj(ac)));
  gonexti();
}

define_instruction(inot) {
  cki(ac);
  ac = obj_from_fixnum(~fixnum_from_obj(ac));
  gonexti();
}

define_instruction(iand) {
  obj x = ac, y = spop(); cki(x); cki(y);
  ac = obj_from_fixnum(fixnum_from_obj(x) & fixnum_from_obj(y));
  gonexti();
}

define_instruction(iior) {
  obj x = ac, y = spop(); cki(x); cki(y);
  ac = obj_from_fixnum(fixnum_from_obj(x) | fixnum_from_obj(y));
  gonexti();
}

define_instruction(ixor) {
  obj x = ac, y = spop(); cki(x); cki(y);
  ac = obj_from_fixnum(fixnum_from_obj(x) ^ fixnum_from_obj(y));
  gonexti();
}

define_instruction(iasl) {
  obj x = ac, y = spop(); cki(x); cki(y);
  ac = obj_from_fixnum(fxasl(fixnum_from_obj(x), fixnum_from_obj(y)));
  gonexti();
}

define_instruction(iasr) {
  obj x = ac, y = spop(); cki(x); cki(y);
  ac = obj_from_fixnum(fxasr(fixnum_from_obj(x), fixnum_from_obj(y)));
  gonexti();
}

define_instruction(ilt) {
  obj x = ac, y = spop(); cki(x); cki(y);
  ac = obj_from_bool(fixnum_from_obj(x) < fixnum_from_obj(y));
  gonexti(); 
}

define_instruction(igt) {
  obj x = ac, y = spop(); cki(x); cki(y);
  ac = obj_from_bool(fixnum_from_obj(x) > fixnum_from_obj(y));
  gonexti(); 
}

define_instruction(ile) {
  obj x = ac, y = spop(); cki(x); cki(y);
  ac = obj_from_bool(fixnum_from_obj(x) <= fixnum_from_obj(y));
  gonexti(); 
}

define_instruction(ige) {
  obj x = ac, y = spop(); cki(x); cki(y);
  ac = obj_from_bool(fixnum_from_obj(x) >= fixnum_from_obj(y));
  gonexti(); 
}

define_instruction(ieq) {
  obj x = ac, y = spop(); cki(x); cki(y);
  ac = obj_from_bool(x == y);
  gonexti(); 
}

define_instruction(ine) {
  obj x = ac, y = spop(); cki(x); cki(y);
  ac = obj_from_bool(x != y);
  gonexti(); 
}


define_instruction(jzerop) {
  ckj(ac);
  ac = obj_from_bool(flonum_from_obj(ac) == 0.0);
  gonexti();
}

define_instruction(jposp) {
  ckj(ac);
  ac = obj_from_bool(flonum_from_obj(ac) > 0.0);
  gonexti(); 
}

define_instruction(jnegp) {
  ckj(ac);
  ac = obj_from_bool(flonum_from_obj(ac) < 0.0);
  gonexti(); 
}

define_instruction(jevnp) {
  double f; ckj(ac);
  f = flonum_from_obj(ac);
  ac = obj_from_bool(flisint(f / 2.0));
  gonexti(); 
}

define_instruction(joddp) {
  double f; ckj(ac);
  f = flonum_from_obj(ac);
  ac = obj_from_bool(flisint((f + 1.0) / 2.0));
  gonexti(); 
}

define_instruction(jintp) {
  double f; ckj(ac);
  f = flonum_from_obj(ac);
  ac = obj_from_bool(f > -HUGE_VAL && f < HUGE_VAL && f == floor(f));
  gonexti(); 
}

define_instruction(jnanp) {
  double f; ckj(ac);
  f = flonum_from_obj(ac);
  ac = obj_from_bool(f != f);
  gonexti(); 
}

define_instruction(jfinp) {
  double f; ckj(ac);
  f = flonum_from_obj(ac);
  ac = obj_from_bool(f > -HUGE_VAL && f < HUGE_VAL);
  gonexti(); 
}

define_instruction(jinfp) {
  double f; ckj(ac);
  f = flonum_from_obj(ac);
  ac = obj_from_bool(f <= -HUGE_VAL || f >= HUGE_VAL);
  gonexti(); 
}

define_instruction(jadd) {
  obj x = ac, y = spop(); ckj(x); ckj(y);
  ac = obj_from_flonum(sp-r, flonum_from_obj(x) + flonum_from_obj(y));
  gonexti(); 
}

define_instruction(jsub) {
  obj x = ac, y = spop(); ckj(x); ckj(y);
  ac = obj_from_flonum(sp-r, flonum_from_obj(x) - flonum_from_obj(y));
  gonexti(); 
}

define_instruction(jmul) {
  obj x = ac, y = spop(); ckj(x); ckj(y);
  ac = obj_from_flonum(sp-r, flonum_from_obj(x) * flonum_from_obj(y));
  gonexti();
}

define_instruction(jdiv) {
  obj x = ac, y = spop(); ckj(x); ckj(y);
  ac = obj_from_flonum(sp-r, flonum_from_obj(x) / flonum_from_obj(y));
  gonexti();
}

define_instruction(jrem) {
  obj x = ac, y = spop(); ckj(x); ckj(y);
  ac = obj_from_flonum(sp-r, flrem(flonum_from_obj(x), flonum_from_obj(y)));
  gonexti();
}

define_instruction(jlt) {
  obj x = ac, y = spop(); ckj(x); ckj(y);
  ac = obj_from_bool(flonum_from_obj(x) < flonum_from_obj(y));
  gonexti(); 
}

define_instruction(jgt) {
  obj x = ac, y = spop(); ckj(x); ckj(y);
  ac = obj_from_bool(flonum_from_obj(x) > flonum_from_obj(y));
  gonexti(); 
}

define_instruction(jle) {
  obj x = ac, y = spop(); ckj(x); ckj(y);
  ac = obj_from_bool(flonum_from_obj(x) <= flonum_from_obj(y));
  gonexti(); 
}

define_instruction(jge) {
  obj x = ac, y = spop(); ckj(x); ckj(y);
  ac = obj_from_bool(flonum_from_obj(x) >= flonum_from_obj(y));
  gonexti(); 
}

define_instruction(jeq) {
  obj x = ac, y = spop(); ckj(x); ckj(y);
  ac = obj_from_bool(flonum_from_obj(x) == flonum_from_obj(y));
  gonexti(); 
}

define_instruction(jne) {
  obj x = ac, y = spop(); ckj(x); ckj(y);
  ac = obj_from_bool(flonum_from_obj(x) != flonum_from_obj(y));
  gonexti(); 
}

define_instruction(jmin) {
  obj x = ac, y = spop(); ckj(x); ckj(y);
  ac = flonum_from_obj(x) < flonum_from_obj(y) ? x : y;
  gonexti(); 
}

define_instruction(jmax) {
  obj x = ac, y = spop(); ckj(x); ckj(y);
  ac = flonum_from_obj(x) > flonum_from_obj(y) ? x : y;
  gonexti(); 
}

define_instruction(jneg) {
  ckj(ac);
  ac = obj_from_flonum(sp-r, -flonum_from_obj(ac));
  gonexti(); 
}

define_instruction(jabs) {
  ckj(ac);
  ac = obj_from_flonum(sp-r, fabs(flonum_from_obj(ac)));
  gonexti(); 
}

define_instruction(jgcd) {
  obj x = ac, y = spop(); ckj(x); ckj(y);
  ac = obj_from_flonum(sp-r, flgcd(flonum_from_obj(x), flonum_from_obj(y)));
  gonexti(); 
}

define_instruction(jpow) {
  obj x = ac, y = spop(); ckj(x); ckj(y);
  ac = obj_from_flonum(sp-r, pow(flonum_from_obj(x), flonum_from_obj(y)));
  gonexti(); 
}

define_instruction(jsqrt) {
  ckj(ac);
  ac = obj_from_flonum(sp-r, sqrt(flonum_from_obj(ac)));
  gonexti(); 
}

define_instruction(jtoi) {
  ckj(ac);
  ac = obj_from_fixnum(fxflo(flonum_from_obj(ac)));
  gonexti();
}

define_instruction(jquo) {
  obj x = ac, y = spop(); ckj(x); ckj(y);
  ac = obj_from_flonum(sp-r, flquo(flonum_from_obj(x), flonum_from_obj(y)));
  gonexti(); 
}

define_instruction(jmqu) {
  obj x = ac, y = spop(); ckj(x); ckj(y);
  ac = obj_from_flonum(sp-r, flmqu(flonum_from_obj(x), flonum_from_obj(y)));
  gonexti(); 
}

define_instruction(jmlo) {
  obj x = ac, y = spop(); ckj(x); ckj(y);
  ac = obj_from_flonum(sp-r, flmlo(flonum_from_obj(x), flonum_from_obj(y)));
  gonexti(); 
}

define_instruction(jfloor) {
  ckj(ac);
  ac = obj_from_flonum(sp-r, floor(flonum_from_obj(ac)));
  gonexti(); 
}

define_instruction(jceil) {
  ckj(ac);
  ac = obj_from_flonum(sp-r, ceil(flonum_from_obj(ac)));
  gonexti(); 
}

define_instruction(jtrunc) {
  double f, i; ckj(ac);
  f = flonum_from_obj(ac);
  modf(f,  &i);
  ac = obj_from_flonum(sp-r, i);
  gonexti(); 
}

define_instruction(jround) {
  ckj(ac);
  ac = obj_from_flonum(sp-r, flround(flonum_from_obj(ac)));
  gonexti(); 
}

define_instruction(jexp) {
  ckj(ac);
  ac = obj_from_flonum(sp-r, exp(flonum_from_obj(ac)));
  gonexti(); 
}

define_instruction(jlog) {
  obj x = ac, y = spop(); ckj(ac);
  if (likely(!y)) {
    ac = obj_from_flonum(sp-r, log(flonum_from_obj(ac)));
  } else {
    double b; ckj(y); b = flonum_from_obj(y);
    if (likely(b == 10.0)) ac = obj_from_flonum(sp-r, log10(flonum_from_obj(ac)));
    else ac = obj_from_flonum(sp-r, log(flonum_from_obj(ac))/log(b));
  }
  gonexti(); 
}

define_instruction(jsin) {
  ckj(ac);
  ac = obj_from_flonum(sp-r, sin(flonum_from_obj(ac)));
  gonexti(); 
}

define_instruction(jcos) {
  ckj(ac);
  ac = obj_from_flonum(sp-r, cos(flonum_from_obj(ac)));
  gonexti(); 
}

define_instruction(jtan) {
  ckj(ac);
  ac = obj_from_flonum(sp-r, tan(flonum_from_obj(ac)));
  gonexti(); 
}

define_instruction(jasin) {
  ckj(ac);
  ac = obj_from_flonum(sp-r, asin(flonum_from_obj(ac)));
  gonexti(); 
}

define_instruction(jacos) {
  ckj(ac);
  ac = obj_from_flonum(sp-r, acos(flonum_from_obj(ac)));
  gonexti(); 
}

define_instruction(jatan) {
  obj x = ac, y = spop(); ckj(ac);
  if (likely(!y)) {
    ac = obj_from_flonum(sp-r, atan(flonum_from_obj(x)));
  } else {
    ckj(y); 
    ac = obj_from_flonum(sp-r, atan2(flonum_from_obj(x), flonum_from_obj(y)));
  }
  gonexti(); 
}


define_instruction(zerop) {
  obj x = ac;
  if (likely(is_fixnum_obj(x))) {
    ac = obj_from_bool(x == obj_from_fixnum(0));
  } else if (likely(is_flonum_obj(x))) {
    ac = obj_from_bool(flonum_from_obj(x) == 0.0);
  } else failactype("number");
  gonexti();
}

define_instruction(posp) {
  if (likely(is_fixnum_obj(ac))) ac = obj_from_bool(get_fixnum_unchecked(ac) > 0);
  else if (likely(is_flonum_obj(ac))) ac = obj_from_bool(flonum_from_obj(ac) > 0.0);
  else failactype("number");
  gonexti(); 
}

define_instruction(negp) {
  if (likely(is_fixnum_obj(ac))) ac = obj_from_bool(get_fixnum_unchecked(ac) < 0);
  else if (likely(is_flonum_obj(ac))) ac = obj_from_bool(flonum_from_obj(ac) < 0.0);
  else failactype("number");
  gonexti(); 
}

define_instruction(add) {
  obj x = ac, y = spop();
  if (likely(are_fixnum_objs(x, y))) {
    ac = obj_from_fixnum(fxadd(get_fixnum_unchecked(x), get_fixnum_unchecked(y)));
  } else {
    double dx, dy;
    if (likely(is_flonum_obj(x))) dx = flonum_from_obj(x);
    else if (likely(is_fixnum_obj(x))) dx = (double)fixnum_from_obj(x);
    else failtype(x, "number");
    if (likely(is_flonum_obj(y))) dy = flonum_from_obj(y);
    else if (likely(is_fixnum_obj(y))) dy = (double)fixnum_from_obj(y);
    else failtype(y, "number");
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
    else if (likely(is_fixnum_obj(x))) dx = (double)fixnum_from_obj(x);
    else failtype(x, "number");
    if (likely(is_flonum_obj(y))) dy = flonum_from_obj(y);
    else if (likely(is_fixnum_obj(y))) dy = (double)fixnum_from_obj(y);
    else failtype(y, "number");
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
    else if (likely(is_fixnum_obj(x))) dx = (double)fixnum_from_obj(x);
    else failtype(x, "number");
    if (likely(is_flonum_obj(y))) dy = flonum_from_obj(y);
    else if (likely(is_fixnum_obj(y))) dy = (double)fixnum_from_obj(y);
    else failtype(y, "number");
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
    else if (likely(is_fixnum_obj(x))) dx = (double)fixnum_from_obj(x);
    else failtype(x, "number");
    if (likely(is_flonum_obj(y))) dy = flonum_from_obj(y);
    else if (likely(is_fixnum_obj(y))) dy = (double)fixnum_from_obj(y);
    else failtype(y, "number");
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
    else if (likely(is_fixnum_obj(x))) dx = (double)fixnum_from_obj(x);
    else failtype(x, "number");
    if (likely(is_flonum_obj(y))) dy = flonum_from_obj(y);
    else if (likely(is_fixnum_obj(y))) dy = (double)fixnum_from_obj(y);
    else failtype(y, "number");
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
    else if (likely(is_fixnum_obj(x))) dx = (double)fixnum_from_obj(x);
    else failtype(x, "number");
    if (likely(is_flonum_obj(y))) dy = flonum_from_obj(y);
    else if (likely(is_fixnum_obj(y))) dy = (double)fixnum_from_obj(y);
    else failtype(y, "number");
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
    else if (likely(is_fixnum_obj(x))) dx = (double)fixnum_from_obj(x);
    else failtype(x, "number");
    if (likely(is_flonum_obj(y))) dy = flonum_from_obj(y);
    else if (likely(is_fixnum_obj(y))) dy = (double)fixnum_from_obj(y);
    else failtype(y, "number");
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
    else if (likely(is_fixnum_obj(x))) dx = (double)fixnum_from_obj(x);
    else failtype(x, "number");
    if (likely(is_flonum_obj(y))) dy = flonum_from_obj(y);
    else if (likely(is_fixnum_obj(y))) dy = (double)fixnum_from_obj(y);
    else failtype(y, "number");
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
    else if (likely(is_fixnum_obj(x))) dx = (double)fixnum_from_obj(x);
    else failtype(x, "number");
    if (likely(is_flonum_obj(y))) dy = flonum_from_obj(y);
    else if (likely(is_fixnum_obj(y))) dy = (double)fixnum_from_obj(y);
    else failtype(y, "number");
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
    else if (likely(is_fixnum_obj(x))) dx = (double)fixnum_from_obj(x);
    else failtype(x, "number");
    if (likely(is_flonum_obj(y))) dy = flonum_from_obj(y);
    else if (likely(is_fixnum_obj(y))) dy = (double)fixnum_from_obj(y);
    else failtype(y, "number");
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
    else if (likely(is_fixnum_obj(x))) dx = (double)fixnum_from_obj(x);
    else failtype(x, "number");
    if (likely(is_flonum_obj(y))) dy = flonum_from_obj(y);
    else if (likely(is_fixnum_obj(y))) dy = (double)fixnum_from_obj(y);
    else failtype(y, "number");
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
    else if (likely(is_fixnum_obj(x))) dx = (double)fixnum_from_obj(x);
    else failtype(x, "number");
    if (likely(is_flonum_obj(y))) dy = flonum_from_obj(y);
    else if (likely(is_fixnum_obj(y))) dy = (double)fixnum_from_obj(y);
    else failtype(y, "number");
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
    else if (likely(is_fixnum_obj(x))) dx = (double)fixnum_from_obj(x);
    else failtype(x, "number");
    if (likely(is_flonum_obj(y))) dy = flonum_from_obj(y);
    else if (likely(is_fixnum_obj(y))) dy = (double)fixnum_from_obj(y);
    else failtype(y, "number");
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
    else if (likely(is_fixnum_obj(x))) dx = (double)fixnum_from_obj(x);
    else failtype(x, "number");
    if (likely(is_flonum_obj(y))) dy = flonum_from_obj(y);
    else if (likely(is_fixnum_obj(y))) dy = (double)fixnum_from_obj(y);
    else failtype(y, "number");
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
    else if (likely(is_fixnum_obj(x))) dx = (double)fixnum_from_obj(x);
    else failtype(x, "number");
    if (likely(is_flonum_obj(y))) dy = flonum_from_obj(y);
    else if (likely(is_fixnum_obj(y))) dy = (double)fixnum_from_obj(y);
    else failtype(y, "number");
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
    else if (likely(is_fixnum_obj(x))) dx = (double)fixnum_from_obj(x);
    else failtype(x, "number");
    if (likely(is_flonum_obj(y))) dy = flonum_from_obj(y);
    else if (likely(is_fixnum_obj(y))) dy = (double)fixnum_from_obj(y);
    else failtype(y, "number");
    ac = dx > dy ? obj_from_flonum(sp-r, dx) : obj_from_flonum(sp-r, dy);
  }
  gonexti(); 
}

define_instruction(gcd) {
  obj x = ac, y = spop();
  if (likely(are_fixnum_objs(x, y))) {
    ac = obj_from_fixnum(fxgcd(fixnum_from_obj(x), fixnum_from_obj(y)));
  } else {
    double dx, dy;
    if (likely(is_flonum_obj(x))) dx = flonum_from_obj(x);
    else if (likely(is_fixnum_obj(x))) dx = (double)fixnum_from_obj(x);
    else failtype(x, "number");
    if (likely(is_flonum_obj(y))) dy = flonum_from_obj(y);
    else if (likely(is_fixnum_obj(y))) dy = (double)fixnum_from_obj(y);
    else failtype(y, "number");
    ac = obj_from_flonum(sp-r, flgcd(dx, dy));
  }
  gonexti(); 
}

define_instruction(pow) {
  obj x = ac, y = spop();
  if (likely(are_fixnum_objs(x, y))) {
    /* fixme: this will either overflow, or fail on negative y */
    ac = obj_from_fixnum(fxpow(fixnum_from_obj(x), fixnum_from_obj(y)));
  } else {
    double dx, dy;
    if (likely(is_flonum_obj(x))) dx = flonum_from_obj(x);
    else if (likely(is_fixnum_obj(x))) dx = (double)fixnum_from_obj(x);
    else failtype(x, "number");
    if (likely(is_flonum_obj(y))) dy = flonum_from_obj(y);
    else if (likely(is_fixnum_obj(y))) dy = (double)fixnum_from_obj(y);
    else failtype(y, "number");
    ac = obj_from_flonum(sp-r, pow(dx, dy));
  }
  gonexti(); 
}

define_instruction(sqrt) {
  if (likely(is_flonum_obj(ac))) {
    ac = obj_from_flonum(sp-r, sqrt(flonum_from_obj(ac)));
  } else if (likely(is_fixnum_obj(ac))) {
    long x = fixnum_from_obj(ac), y;
    if (x < 0) ac = obj_from_flonum(sp-r, (HUGE_VAL - HUGE_VAL));   
    else if (y = fxsqrt(x), y*y == x) ac = obj_from_fixnum(y);
    else ac = obj_from_flonum(sp-r, sqrt((double)x));
  } else failactype("number");
  gonexti(); 
}

define_instruction(exp) {
  double x;
  if (unlikely(is_fixnum_obj(ac))) x = (double)fixnum_from_obj(ac);
  else if (likely(is_flonum_obj(ac))) x = flonum_from_obj(ac);
  else failactype("number");
  ac = obj_from_flonum(sp-r, exp(x));
  gonexti(); 
}

define_instruction(log) {
  double x; obj y = spop();
  if (unlikely(is_fixnum_obj(ac))) x = (double)fixnum_from_obj(ac);
  else if (likely(is_flonum_obj(ac))) x = flonum_from_obj(ac);
  else failactype("number");
  if (likely(!y)) {
    ac = obj_from_flonum(sp-r, log(x));
  } else if (likely(y == obj_from_fixnum(10))) {
    ac = obj_from_flonum(sp-r, log10(x));
  } else {
    double b; 
    if (unlikely(is_fixnum_obj(y))) b = (double)fixnum_from_obj(y);
    else if (likely(is_flonum_obj(y))) b = flonum_from_obj(y);
    else failtype(y, "number");
    if (likely(b == 10.0)) ac = obj_from_flonum(sp-r, log10(x));
    else ac = obj_from_flonum(sp-r, log(x)/log(b));
  }
  gonexti(); 
}

define_instruction(sin) {
  double x;
  if (unlikely(is_fixnum_obj(ac))) {
    x = (double)fixnum_from_obj(ac);
  } else if (likely(is_flonum_obj(ac))) {
    x = flonum_from_obj(ac);
  } else failactype("number");
  ac = obj_from_flonum(sp-r, sin(x));
  gonexti(); 
}

define_instruction(cos) {
  double x;
  if (unlikely(is_fixnum_obj(ac))) {
    x = (double)fixnum_from_obj(ac);
  } else if (likely(is_flonum_obj(ac))) {
    x = flonum_from_obj(ac);
  } else failactype("number");
  ac = obj_from_flonum(sp-r, cos(x));
  gonexti(); 
}

define_instruction(tan) {
  double x;
  if (unlikely(is_fixnum_obj(ac))) {
    x = (double)fixnum_from_obj(ac);
  } else if (likely(is_flonum_obj(ac))) {
    x = flonum_from_obj(ac);
  } else failactype("number");
  ac = obj_from_flonum(sp-r, tan(x));
  gonexti(); 
}

define_instruction(asin) {
  double x;
  if (unlikely(is_fixnum_obj(ac))) {
    x = (double)fixnum_from_obj(ac);
  } else if (likely(is_flonum_obj(ac))) {
    x = flonum_from_obj(ac);
  } else failactype("number");
  ac = obj_from_flonum(sp-r, asin(x));
  gonexti(); 
}

define_instruction(acos) {
  double x;
  if (unlikely(is_fixnum_obj(ac))) {
    x = (double)fixnum_from_obj(ac);
  } else if (likely(is_flonum_obj(ac))) {
    x = flonum_from_obj(ac);
  } else failactype("number");
  ac = obj_from_flonum(sp-r, acos(x));
  gonexti(); 
}

define_instruction(atan) {
  double x; obj y = spop();
  if (unlikely(is_fixnum_obj(ac))) x = (double)fixnum_from_obj(ac);
  else if (likely(is_flonum_obj(ac))) x = flonum_from_obj(ac);
  else failactype("number");
  if (likely(!y)) {
    ac = obj_from_flonum(sp-r, atan(x));
  } else {
    double b; 
    if (unlikely(is_fixnum_obj(y))) b = (double)fixnum_from_obj(y);
    else if (likely(is_flonum_obj(y))) b = flonum_from_obj(y);
    else failtype(y, "number");
    ac = obj_from_flonum(sp-r, atan2(x, b));
  }
  gonexti(); 
}

define_instruction(neg) {
  if (likely(is_fixnum_obj(ac))) {
    ac = obj_from_fixnum(-fixnum_from_obj(ac));
  } else if (likely(is_flonum_obj(ac))) {
    ac = obj_from_flonum(sp-r, -flonum_from_obj(ac));
  } else failactype("number");
  gonexti(); 
}

define_instruction(abs) {
  if (likely(is_fixnum_obj(ac))) {
    ac = obj_from_fixnum(fxabs(fixnum_from_obj(ac)));
  } else if (likely(is_flonum_obj(ac))) {
    ac = obj_from_flonum(sp-r, fabs(flonum_from_obj(ac)));
  } else failactype("number");
  gonexti(); 
}

define_instruction(floor) {
  if (likely(is_flonum_obj(ac))) {
    ac = obj_from_flonum(sp-r, floor(flonum_from_obj(ac)));
  } else if (unlikely(!is_fixnum_obj(ac))) {
    failactype("number");
  }
  gonexti(); 
}

define_instruction(ceil) {
  if (likely(is_flonum_obj(ac))) {
    ac = obj_from_flonum(sp-r, ceil(flonum_from_obj(ac)));
  } else if (unlikely(!is_fixnum_obj(ac))) {
    failactype("number");
  }
  gonexti(); 
}

define_instruction(trunc) {
  if (likely(is_flonum_obj(ac))) {
    double x = flonum_from_obj(ac);
    double i; modf(x,  &i);
    ac = obj_from_flonum(sp-r, i);
  } else if (unlikely(!is_fixnum_obj(ac))) {
    failactype("number");
  }
  gonexti(); 
}

define_instruction(round) {
  if (likely(is_flonum_obj(ac))) {
    ac = obj_from_flonum(sp-r, flround(flonum_from_obj(ac)));
  } else if (unlikely(!is_fixnum_obj(ac))) {
    failactype("number");
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
  } else if (likely(is_flonum_obj(ac))) {
    double f = flonum_from_obj(ac);
    ac = obj_from_bool(f != f);
  } else failactype("number");
  gonexti(); 
}

define_instruction(finp) {
  if (unlikely(is_fixnum_obj(ac))) {
    ac = obj_from_bool(1);
  } else if (likely(is_flonum_obj(ac))) {
    double f = flonum_from_obj(ac);
    ac = obj_from_bool(f > -HUGE_VAL && f < HUGE_VAL);
  } else failactype("number");
  gonexti(); 
}

define_instruction(infp) {
  if (unlikely(is_fixnum_obj(ac))) {
    ac = obj_from_bool(0);
  } else if (likely(is_flonum_obj(ac))) {
    double f = flonum_from_obj(ac);
    ac = obj_from_bool(f <= -HUGE_VAL || f >= HUGE_VAL);
  } else failactype("number");
  gonexti(); 
}

define_instruction(evnp) {
  if (likely(is_fixnum_obj(ac))) {
    ac = obj_from_bool((fixnum_from_obj(ac) & 1) == 0);
  } else if (likely(is_flonum_obj(ac))) {
    double f = flonum_from_obj(ac);
    ac = obj_from_bool(flisint(f / 2.0));
  } else failactype("number");
  gonexti(); 
}

define_instruction(oddp) {
  if (likely(is_fixnum_obj(ac))) {
    ac = obj_from_bool((fixnum_from_obj(ac) & 1) != 0);
  } else if (likely(is_flonum_obj(ac))) {
    double f = flonum_from_obj(ac);
    ac = obj_from_bool(flisint((f + 1.0) / 2.0));
  } else failactype("number");
  gonexti(); 
}

define_instruction(ntoi) {
  if (likely(is_flonum_obj(ac))) ac = obj_from_fixnum(fxflo(flonum_from_obj(ac)));
  else if (likely(is_fixnum_obj(ac))) /* keep ac as-is */ ;
  else failactype("number");
  gonexti(); 
}

define_instruction(ntoj) {
  if (likely(is_fixnum_obj(ac))) ac = obj_from_flonum(sp-r, (flonum_t)fixnum_from_obj(ac));
  else if (likely(is_flonum_obj(ac))) /* keep ac as-is */ ;
  else failactype("number");
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
  obj x = ac, y = spop(); int cmp; ckc(x); ckc(y);
  cmp = char_from_obj(x) - char_from_obj(y);
  ac = obj_from_fixnum(cmp);
  gonexti(); 
}

define_instruction(ceq) {
  obj x = ac, y = spop(); ckc(x); ckc(y);
  ac = obj_from_bool(char_from_obj(x) == char_from_obj(y));
  gonexti(); 
}

define_instruction(clt) {
  obj x = ac, y = spop(); ckc(x); ckc(y);
  ac = obj_from_bool(char_from_obj(x) < char_from_obj(y));
  gonexti(); 
}

define_instruction(cgt) {
  obj x = ac, y = spop(); ckc(x); ckc(y);
  ac = obj_from_bool(char_from_obj(x) > char_from_obj(y));
  gonexti(); 
}

define_instruction(cle) {
  obj x = ac, y = spop(); ckc(x); ckc(y);
  ac = obj_from_bool(char_from_obj(x) <= char_from_obj(y));
  gonexti(); 
}

define_instruction(cge) {
  obj x = ac, y = spop(); ckc(x); ckc(y);
  ac = obj_from_bool(char_from_obj(x) >= char_from_obj(y));
  gonexti(); 
}

define_instruction(cicmp) {
  obj x = ac, y = spop(); int cmp; ckc(x); ckc(y);
  cmp = tolower(char_from_obj(x)) - tolower(char_from_obj(y));
  ac = obj_from_fixnum(cmp);
  gonexti(); 
}

define_instruction(cieq) {
  obj x = ac, y = spop(); ckc(x); ckc(y);
  ac = obj_from_bool(tolower(char_from_obj(x)) == tolower(char_from_obj(y)));
  gonexti(); 
}

define_instruction(cilt) {
  obj x = ac, y = spop(); ckc(x); ckc(y);
  ac = obj_from_bool(tolower(char_from_obj(x)) < tolower(char_from_obj(y)));
  gonexti(); 
}

define_instruction(cigt) {
  obj x = ac, y = spop(); ckc(x); ckc(y);
  ac = obj_from_bool(tolower(char_from_obj(x)) > tolower(char_from_obj(y)));
  gonexti(); 
}

define_instruction(cile) {
  obj x = ac, y = spop(); ckc(x); ckc(y);
  ac = obj_from_bool(tolower(char_from_obj(x)) <= tolower(char_from_obj(y)));
  gonexti(); 
}

define_instruction(cige) {
  obj x = ac, y = spop(); ckc(x); ckc(y);
  ac = obj_from_bool(tolower(char_from_obj(x)) >= tolower(char_from_obj(y)));
  gonexti(); 
}

define_instruction(cwsp) {
  ckc(ac);
  ac = obj_from_bool(isspace(char_from_obj(ac)));
  gonexti(); 
}

define_instruction(clcp) {
  ckc(ac);
  ac = obj_from_bool(islower(char_from_obj(ac)));
  gonexti(); 
}

define_instruction(cucp) {
  ckc(ac);
  ac = obj_from_bool(isupper(char_from_obj(ac)));
  gonexti(); 
}

define_instruction(calp) {
  ckc(ac);
  ac = obj_from_bool(isalpha(char_from_obj(ac)));
  gonexti(); 
}

define_instruction(cnup) {
  ckc(ac);
  ac = obj_from_bool(isdigit(char_from_obj(ac)));
  gonexti(); 
}

define_instruction(cupc) {
  ckc(ac);
  ac = obj_from_char(toupper(char_from_obj(ac)));
  gonexti(); 
}

define_instruction(cdnc) {
  ckc(ac);
  ac = obj_from_char(tolower(char_from_obj(ac)));
  gonexti(); 
}

define_instruction(cflc) {
  ckc(ac);
  ac = obj_from_char(tolower(char_from_obj(ac))); /* stub */
  gonexti(); 
}

define_instruction(cdgv) {
  int ch; ckc(ac);
  ch = char_from_obj(ac);
  if (likely('0' <= ch && ch <= '9')) ac = obj_from_fixnum(ch - '0');  
  /* R7RS won't allow hex and any larger radix digits
  else if (likely('a' <= ch && ch <= 'z')) ac = obj_from_fixnum(10 + ch - 'a');  
  else if (likely('A' <= ch && ch <= 'Z')) ac = obj_from_fixnum(10 + ch - 'A'); */
  else ac = obj_from_bool(0);
  gonexti(); 
}


define_instruction(scmp) {
  obj x = ac, y = spop(); int cmp; cks(x); cks(y);
  cmp = strcmp(stringchars(x), stringchars(y));
  ac = obj_from_fixnum(cmp);
  gonexti(); 
}

define_instruction(seq) {
  obj x = ac, y = spop(); cks(x); cks(y);
  ac = obj_from_bool(strcmp(stringchars(x), stringchars(y)) == 0);
  gonexti(); 
}

define_instruction(slt) {
  obj x = ac, y = spop(); cks(x); cks(y);
  ac = obj_from_bool(strcmp(stringchars(x), stringchars(y)) < 0);
  gonexti(); 
}

define_instruction(sgt) {
  obj x = ac, y = spop(); cks(x); cks(y);
  ac = obj_from_bool(strcmp(stringchars(x), stringchars(y)) > 0);
  gonexti(); 
}

define_instruction(sle) {
  obj x = ac, y = spop(); cks(x); cks(y);
  ac = obj_from_bool(strcmp(stringchars(x), stringchars(y)) <= 0);
  gonexti(); 
}

define_instruction(sge) {
  obj x = ac, y = spop(); cks(x); cks(y);
  ac = obj_from_bool(strcmp(stringchars(x), stringchars(y)) >= 0);
  gonexti(); 
}

define_instruction(sicmp) {
  obj x = ac, y = spop(); int cmp; cks(x); cks(y);
  cmp = strcmp_ci(stringchars(x), stringchars(y));
  ac = obj_from_fixnum(cmp);
  gonexti(); 
}

define_instruction(sieq) {
  obj x = ac, y = spop(); cks(x); cks(y);
  ac = obj_from_bool(strcmp_ci(stringchars(x), stringchars(y)) == 0);
  gonexti(); 
}

define_instruction(silt) {
  obj x = ac, y = spop(); cks(x); cks(y);
  ac = obj_from_bool(strcmp_ci(stringchars(x), stringchars(y)) < 0);
  gonexti(); 
}

define_instruction(sigt) {
  obj x = ac, y = spop(); cks(x); cks(y);
  ac = obj_from_bool(strcmp_ci(stringchars(x), stringchars(y)) > 0);
  gonexti(); 
}

define_instruction(sile) {
  obj x = ac, y = spop(); cks(x); cks(y);
  ac = obj_from_bool(strcmp_ci(stringchars(x), stringchars(y)) <= 0);
  gonexti(); 
}

define_instruction(sige) {
  obj x = ac, y = spop(); cks(x); cks(y);
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

define_instruction(boxp) {
  ac = obj_from_bool(isbox(ac));
  gonexti();
}

define_instruction(funp) {
  ac = obj_from_bool(isvmclo(ac));
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
  ac = mkiport_file(sp-r, stdin); /* TODO: keep in global var -- in r7rs it is a parameter */
  gonexti();
}

define_instruction(sop) {
  ac = mkoport_file(sp-r, stdout); /* TODO: keep in global var -- in r7rs it is a parameter */
  gonexti();
}

define_instruction(sep) {
  ac = mkoport_file(sp-r, stderr); /* TODO: keep in global var -- in r7rs it is a parameter */
  gonexti();
}

define_instruction(ipop) {
  cxtype_iport_t *vt; ckr(ac); 
  vt = iportvt(ac); assert(vt);
  ac = obj_from_bool(vt != (cxtype_iport_t *)IPORT_CLOSED_NTAG);
  gonexti();
}

define_instruction(opop) {
  cxtype_oport_t *vt; ckw(ac); 
  vt = oportvt(ac); assert(vt);
  ac = obj_from_bool(vt != (cxtype_oport_t *)OPORT_CLOSED_NTAG);
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
  int *d; cks(ac);
  d = dupstring(stringdata(ac));
  ac = mkiport_string(sp-r, sialloc(sdatachars(d), d));
  gonexti();
}

define_instruction(oos) {
  ac = mkoport_string(sp-r, newcb());
  gonexti();
}

define_instruction(cip) {
  cxtype_iport_t *vt; ckr(ac);
  vt = iportvt(ac); assert(vt);
  vt->close(iportdata(ac)); vt->free(iportdata(ac));
  objptr_from_obj(ac)[-1] = (obj)IPORT_CLOSED_NTAG;
  gonexti();
}

define_instruction(cop) {
  cxtype_oport_t *vt; ckw(ac);
  vt = oportvt(ac); assert(vt);
  vt->close(oportdata(ac)); vt->free(oportdata(ac));
  objptr_from_obj(ac)[-1] = (obj)OPORT_CLOSED_NTAG;
  gonexti();
}

define_instruction(gos) {
  cxtype_oport_t *vt; ckw(ac);
  vt = ckoportvt(ac);
  if (vt != (cxtype_oport_t *)OPORT_STRING_NTAG &&
      vt != (cxtype_oport_t *)OPORT_BYTEVECTOR_NTAG) {
    ac = mkeof();
  } else {
    cbuf_t *pcb = oportdata(ac);
    ac = hpushstr(sp-r, newstring(cbdata(pcb)));
  }
  gonexti();
}


define_instruction(rdc) {
  int c; ckr(ac);
  c = iportgetc(ac);
  if (unlikely(c == EOF)) ac = mkeof();
  else ac = obj_from_char(c);
  gonexti();
}

define_instruction(rdac) {
  int c; ckr(ac);
  c = iportpeekc(ac);
  if (unlikely(c == EOF)) ac = mkeof();
  else ac = obj_from_char(c);
  gonexti();
}

define_instruction(rdcr) {
  ckr(ac);
  ac = obj_from_bool(1); /* no portable way to detect hanging? */
  gonexti();
}

define_instruction(eofp) {
  ac = obj_from_bool(iseof(ac));
  gonexti();
}

define_instruction(eof) {
  ac = mkeof();
  gonexti();
}


define_instruction(wrc) {
  obj x = ac, y = spop(); ckc(x); ckw(y);
  oportputc(char_from_obj(x), y);
  gonexti();
}

define_instruction(wrs) {
  obj x = ac, y = spop(); cks(x); ckw(y);
  oportputs(stringchars(x), y);
  gonexti();
}

define_instruction(wrcd) {
  obj x = ac, y = spop(); ckw(y);
  oportputcircular(x, y, 1);
  gonexti();
}

define_instruction(wrcw) {
  obj x = ac, y = spop(); ckw(y);
  oportputcircular(x, y, 0);
  gonexti();
}

define_instruction(wrnl) {
  ckw(ac);
  oportputc('\n', ac);
  gonexti();
}

define_instruction(wrhw) {
  obj x = ac, y = spop(); ckw(y);
  oportputshared(x, y, 0);
  gonexti();
}

define_instruction(wriw) {
  obj x = ac, y = spop(); ckw(y);
  oportputsimple(x, y, 0);
  gonexti();
}

define_instruction(igp) {
  ac = obj_from_bool(isintegrable(ac));
  gonexti(); 
}

define_instruction(iglk) {
  struct intgtab_entry *pe; cky(ac);
  pe = lookup_integrable(getsymbol(ac));
  ac = pe ? mkintegrable(pe) : obj_from_bool(0);
  gonexti(); 
}

define_instruction(igty) {
  int it; ckg(ac);
  it = integrable_type(integrabledata(ac));
  ac = it ? obj_from_char(it) : obj_from_bool(0);
  gonexti(); 
}

define_instruction(iggl) {
  const char *igs; ckg(ac);
  igs = integrable_global(integrabledata(ac));
  ac = igs ? mksymbol(internsym((char*)igs)) : obj_from_bool(0);
  gonexti(); 
}

define_instruction(igco) {
  int n; const char *cs; ckg(ac); ckk(sref(0));
  n = fixnum_from_obj(spop());
  cs = integrable_code(integrabledata(ac), n);
  ac = cs ? hpushstr(sp-r, newstring((char*)cs)) : obj_from_bool(0);
  gonexti(); 
}

define_instruction(rdsx) {
  cks(ac); unload_ac(); /* ac->ra (string) */
  hp = rds_stox(r, sp, hp);
  reload_ac(); /* ra->ac (sexp or eof) */
  if (ac == mkeof()) fail("failed to read serialized s-expression");
  gonexti(); 
}

define_instruction(rdsc) {
  cks(ac); unload_ac(); /* ac->ra (string) */
  hp = rds_stoc(r, sp, hp);
  reload_ac(); /* ra->ac (codevec or eof) */
  if (ac == mkeof()) fail("failed to read serialized code");
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
  ckx(ac); rd = ac; rx = obj_from_fixnum(0); ac = obj_from_fixnum(0); 
  callsubi(); 
}
define_instruction(call1) { 
  ckx(ac); rd = ac; rx = obj_from_fixnum(0); ac = obj_from_fixnum(1); 
  callsubi(); 
}
define_instruction(call2) { 
  ckx(ac); rd = ac; rx = obj_from_fixnum(0); ac = obj_from_fixnum(2); 
  callsubi(); 
}
define_instruction(call3) { 
  ckx(ac); rd = ac; rx = obj_from_fixnum(0); ac = obj_from_fixnum(3); 
  callsubi(); 
}
define_instruction(call4) { 
  ckx(ac); rd = ac; rx = obj_from_fixnum(0); ac = obj_from_fixnum(4); 
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

define_instruction(atest0) {
  if (unlikely(ac != obj_from_fixnum(0))) fail("argument count error on entry");
  gonexti();
}

define_instruction(atest1) {
  if (unlikely(ac != obj_from_fixnum(1))) fail("argument count error on entry");
  gonexti();
}

define_instruction(atest2) {
  if (unlikely(ac != obj_from_fixnum(2))) fail("argument count error on entry");
  gonexti();
}

define_instruction(atest3) {
  if (unlikely(ac != obj_from_fixnum(3))) fail("argument count error on entry");
  gonexti();
}

define_instruction(atest4) {
  if (unlikely(ac != obj_from_fixnum(4))) fail("argument count error on entry");
  gonexti();
}

define_instruction(scall1) {
  int m = 1, n = fixnum_from_obj(*ip++);
  ckx(ac); rd = ac; rx = obj_from_fixnum(0); 
  ac = obj_from_fixnum(n); /* argc */
  memmove((void*)(sp-n-m), (void*)(sp-n), (size_t)n*sizeof(obj));
  sdrop(m); callsubi();
}

define_instruction(scall10) {
  ckx(ac); rd = ac; rx = obj_from_fixnum(0); ac = obj_from_fixnum(0);
  sdrop(1); callsubi();
}

define_instruction(scall11) {
  ckx(ac); rd = ac; rx = obj_from_fixnum(0); ac = obj_from_fixnum(1);
  sref(1) = sref(0);
  sdrop(1); callsubi();
}

define_instruction(scall12) {
  ckx(ac); rd = ac; rx = obj_from_fixnum(0); ac = obj_from_fixnum(2);
  sref(2) = sref(1); sref(1) = sref(0);
  sdrop(1); callsubi();
}

define_instruction(scall13) {
  ckx(ac); rd = ac; rx = obj_from_fixnum(0); ac = obj_from_fixnum(3);
  sref(3) = sref(2); sref(2) = sref(1); sref(1) = sref(0);
  sdrop(1); callsubi();
}

define_instruction(scall14) {
  ckx(ac); rd = ac; rx = obj_from_fixnum(0); ac = obj_from_fixnum(4);
  sref(4) = sref(3); sref(3) = sref(2); sref(2) = sref(1); sref(1) = sref(0);
  sdrop(1); callsubi();
}

define_instruction(scall2) {
  int m = 2, n = fixnum_from_obj(*ip++);
  ckx(ac); rd = ac; rx = obj_from_fixnum(0); 
  ac = obj_from_fixnum(n);
  memmove((void*)(sp-n-m), (void*)(sp-n), (size_t)n*sizeof(obj));
  sdrop(m); callsubi();
}

define_instruction(scall20) {
  ckx(ac); rd = ac; rx = obj_from_fixnum(0); ac = obj_from_fixnum(0);
  sdrop(2); callsubi();
}

define_instruction(scall21) {
  ckx(ac); rd = ac; rx = obj_from_fixnum(0); ac = obj_from_fixnum(1);
  sref(2) = sref(0);
  sdrop(2); callsubi();
}

define_instruction(scall22) {
  ckx(ac); rd = ac; rx = obj_from_fixnum(0); ac = obj_from_fixnum(2);
  sref(3) = sref(1); sref(2) = sref(0);
  sdrop(2); callsubi();
}

define_instruction(scall23) {
  ckx(ac); rd = ac; rx = obj_from_fixnum(0); ac = obj_from_fixnum(3);
  sref(4) = sref(2); sref(3) = sref(1); sref(2) = sref(0);
  sdrop(2); callsubi();
}

define_instruction(scall24) {
  ckx(ac); rd = ac; rx = obj_from_fixnum(0); ac = obj_from_fixnum(4);
  sref(5) = sref(3); sref(4) = sref(2); sref(3) = sref(1); sref(2) = sref(0);
  sdrop(2); callsubi();
}

define_instruction(scall3) {
  int m = 3, n = fixnum_from_obj(*ip++);
  ckx(ac); rd = ac; rx = obj_from_fixnum(0); 
  ac = obj_from_fixnum(n);
  memmove((void*)(sp-n-m), (void*)(sp-n), (size_t)n*sizeof(obj));
  sdrop(m); callsubi();
}

define_instruction(scall30) {
  ckx(ac); rd = ac; rx = obj_from_fixnum(0); ac = obj_from_fixnum(0);
  sdrop(3); callsubi();
}

define_instruction(scall31) {
  ckx(ac); rd = ac; rx = obj_from_fixnum(0); ac = obj_from_fixnum(1);
  sref(3) = sref(0);
  sdrop(3); callsubi();
}

define_instruction(scall32) {
  ckx(ac); rd = ac; rx = obj_from_fixnum(0); ac = obj_from_fixnum(2);
  sref(4) = sref(1); sref(3) = sref(0);
  sdrop(3); callsubi();
}

define_instruction(scall33) {
  ckx(ac); rd = ac; rx = obj_from_fixnum(0); ac = obj_from_fixnum(3);
  sref(5) = sref(2); sref(4) = sref(1); sref(3) = sref(0);
  sdrop(3); callsubi();
}

define_instruction(scall34) {
  ckx(ac); rd = ac; rx = obj_from_fixnum(0); ac = obj_from_fixnum(4);
  sref(6) = sref(3); sref(5) = sref(2); sref(4) = sref(1); sref(3) = sref(0);
  sdrop(3); callsubi();
}

define_instruction(scall4) {
  int m = 4, n = fixnum_from_obj(*ip++);
  ckx(ac); rd = ac; rx = obj_from_fixnum(0); 
  ac = obj_from_fixnum(n);
  memmove((void*)(sp-n-m), (void*)(sp-n), (size_t)n*sizeof(obj));
  sdrop(m); callsubi();
}

define_instruction(scall40) {
  ckx(ac); rd = ac; rx = obj_from_fixnum(0); ac = obj_from_fixnum(0);
  sdrop(4); callsubi();
}

define_instruction(scall41) {
  ckx(ac); rd = ac; rx = obj_from_fixnum(0); ac = obj_from_fixnum(1);
  sref(4) = sref(0);
  sdrop(4); callsubi();
}

define_instruction(scall42) {
  ckx(ac); rd = ac; rx = obj_from_fixnum(0); ac = obj_from_fixnum(2);
  sref(5) = sref(1); sref(4) = sref(0);
  sdrop(4); callsubi();
}

define_instruction(scall43) {
  ckx(ac); rd = ac; rx = obj_from_fixnum(0); ac = obj_from_fixnum(3);
  sref(6) = sref(2); sref(5) = sref(1); sref(4) = sref(0);
  sdrop(4); callsubi();
}

define_instruction(scall44) {
  ckx(ac); rd = ac; rx = obj_from_fixnum(0); ac = obj_from_fixnum(4);
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
    else { cki(x); fx = (double)get_fixnum_unchecked(x); }
    if (likely(is_flonum_obj(y))) fy = flonum_from_obj(y);
    else { cki(y); fy = (double)get_fixnum_unchecked(y); }
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
    double fx, fy;
    if (likely(is_flonum_obj(x))) fx = flonum_from_obj(x);
    else { cki(x); fx = (double)get_fixnum_unchecked(x); }
    if (likely(is_flonum_obj(y))) fy = flonum_from_obj(y);
    else { cki(y); fy = (double)get_fixnum_unchecked(y); }
    ac = obj_from_flonum(sp-r, fx - fy);
  }
  spush(ac);
  gonexti(); 
}


#define VM_GEN_DEFGLOBAL
#include "i.h"
#undef  VM_GEN_DEFGLOBAL

/* integrables table */
struct intgtab_entry { int sym; char *igname; int igtype; char *enc; char *lcode; };
#define declare_intgtable_entry(enc, igname, igtype, lcode) \
   { 0, igname, igtype, enc, lcode },

static struct intgtab_entry intgtab[] = {
#define VM_GEN_INTGTABLE
#include "i.h"
#undef  VM_GEN_INTGTABLE
};

static int intgtab_sorted = 0;
static int intgtab_cmp(const void *p1, const void *p2)
{
  int igtype1, igtype2;
  struct intgtab_entry *pe1 = (struct intgtab_entry *)p1;
  struct intgtab_entry *pe2 = (struct intgtab_entry *)p2;
  if (pe1->sym < pe2->sym) return -1;
  if (pe1->sym > pe2->sym) return 1;
  igtype1 = pe1->igtype; if (igtype1 >= ' ') igtype1 = ' ';
  igtype2 = pe2->igtype; if (igtype2 >= ' ') igtype2 = ' ';
  if (igtype1 < igtype2) return -1;
  if (igtype1 > igtype2) return 1;
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

static int isintegrable(obj o)
{
  int n = sizeof(intgtab)/sizeof(intgtab[0]);
  if (is_fixnum_obj(o)) {
    int i = fixnum_from_obj(o);
    if (i >= 0 && i < n) {  
      struct intgtab_entry *pe = &intgtab[i];
      return (pe && pe->igtype >= ' ' && pe->igname && pe->enc);
    }
  }
  return 0;
}

static struct intgtab_entry *integrabledata(obj o)
{
  int n = sizeof(intgtab)/sizeof(intgtab[0]);
  int i = fixnum_from_obj(o);
  struct intgtab_entry *pe = &intgtab[i];
  assert(i >= 0 && i < n);
  return pe;
}

static obj mkintegrable(struct intgtab_entry *pe)
{
  int n = sizeof(intgtab)/sizeof(intgtab[0]);
  assert(pe >= &intgtab[0] && pe < &intgtab[n]); 
  return obj_from_fixnum(pe-intgtab);
}

static struct intgtab_entry *lookup_integrable(int sym)
{
  struct intgtab_entry e, *pe;
  int n = sizeof(intgtab)/sizeof(intgtab[0]);
  if (!intgtab_sorted) sort_intgtab(n);
  e.sym = sym; e.igtype = ' ';
  pe = bsearch(&e, &intgtab[0], n, sizeof(intgtab[0]), intgtab_cmp);
  return (pe && pe->igtype >= ' ' && pe->igname && pe->enc) ? pe : NULL;
}

static int integrable_type(struct intgtab_entry *pi)
{
  int it = pi->igtype;
  return (it >= ' ') ? it : 0;
}

static const char *integrable_global(struct intgtab_entry *pi)
{
  return (pi->igtype >= ' ') ? pi->igname : NULL;
}

static const char *integrable_code(struct intgtab_entry *pi, int n)
{
  char *ps, *code = NULL; int it = pi->igtype;
  if (it >= ' ') {
    ps = pi->enc; assert(ps);
    while (ps && n-- > 0) {
      ps += strlen(ps) + 1; /* \0 terminates each field */
      assert(*ps);
    }
    code = ps;
  }
  return code;
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
  char buf[60], *p = buf, *e = p+59, *s; double d;
  while (p < e) {
    int c = iportpeekc(port);
    if (c == '-' || c == '+' || c == '.' || isalnum(c)) {
      iportgetc(port);
      *p++ = c;
    } else break;
  } 
  *p = 0;
  errno = 0; s = buf; e = "";
  if (*s != '+' && *s != '-') d = strtod(s, &e);
  else if (strcmp_ci(s+1, "inf.0") == 0) d = (*s == '-' ? -HUGE_VAL : HUGE_VAL); 
  else if (strcmp_ci(s+1, "nan.0") == 0) d = HUGE_VAL - HUGE_VAL; 
  else d = strtod(s, &e);
  if (errno || e == s || *e) assert(0);
  return d;
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
    if (ispair(p)) ra = cdr(p);
    else { /* prepend (sym . #&sym) to *globals* */
      obj box;
      hreserve(hbsz(2)*1+hbsz(3)*2, sp-r);
      *--hp = ra; /* mksymbol(internsym("undefined")); */
      *--hp = obj_from_size(BOX_BTAG); box = hendblk(2);
      *--hp = box; *--hp = ra;
      *--hp = obj_from_size(PAIR_BTAG); ra = hendblk(3);
      *--hp = cx__2Aglobals_2A; *--hp = ra;
      *--hp = obj_from_size(PAIR_BTAG); cx__2Aglobals_2A = hendblk(3);
      ra = box;
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
      case 'a': { /* andbo */
        hreserve(hbsz(3), sp-r);
        *--hp = sref(0); *--hp = pbr->g;  
        *--hp = obj_from_size(PAIR_BTAG); sref(0) = hendblk(3);
        c = iportpeekc(sref(1));
        if (c == EOF || c == '}') { ra = mkeof(); goto out; }
        pbr = rds_prefix(sref(1));
        if (pbr->g == 0 || pbr->etyp != 0) { ra = mkeof(); goto out; }
        hreserve(hbsz(3), sp-r);
        *--hp = sref(0); *--hp = pbr->g;  
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
        if (!is_fixnum_obj(ra)) { ra = mkeof(); goto out; }
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
  char lbuf[200], *lcode, *pe0, *pe1;
  if (!intgtab_sorted) sort_intgtab(n);
  for (i = 0; i < n; ++i) {
    struct intgtab_entry *pe = &intgtab[i];
    if (!pe->igname) continue; 
    lcode = pe->lcode;
    if (!lcode) switch (pe->igtype) {
      case '0': {
        lcode = lbuf; assert(pe->enc);
        sprintf(lbuf, "%%0%s]0", pe->enc); 
      } break; 
      case '1': {
        lcode = lbuf; assert(pe->enc);
        sprintf(lbuf, "%%1_!%s]0", pe->enc);
      } break;
      case '2': {
        lcode = lbuf; assert(pe->enc);
        sprintf(lbuf, "%%2_!%s]0", pe->enc);
      } break;
      case '3': {
        lcode = lbuf; assert(pe->enc); 
        sprintf(lbuf, "%%3_!%s]0", pe->enc);
      } break;
      case 'p': {
        lcode = lbuf; assert(pe->enc); 
        pe0 = pe->enc; pe1 = pe0 + strlen(pe0) + 1; assert(*pe1);
        sprintf(lbuf, "%%!0.0u?{%s]1}.0d,.1a,,#0.0,&1{%%2.1u?{.0]2}.1d,.2a,.2%s,:0^[22}.!0.0^_1[12", pe1, pe0);
      } break;
      case 'm': {
        lcode = lbuf; assert(pe->enc); 
        pe0 = pe->enc; pe1 = pe0 + strlen(pe0) + 1; assert(*pe1);
        sprintf(lbuf, "%%!1.0u?{.1%s]2}.0,.2,,#0.0,&1{%%2.1u?{.0]2}.1d,.2a,.2%s,:0^[22}.!0.0^_1[22", pe1, pe0);
      } break;
      case 'c': {
        lcode = lbuf; assert(pe->enc);
        sprintf(lbuf, "%%!0.0u,.0?{.0]2}.1d,.2a,,#0.0,&1{%%2.1u,.0?{.0]3}.2a,.0,.3%s?{.3d,.1,:0^[42}f]4}.!0.0^_1[22", pe->enc);
      } break;
      case 'x': {
        lcode = lbuf; assert(pe->enc);
        sprintf(lbuf, "%%!1.0,.2,,#0.0,&1{%%2.1u?{.0]2}.1d,.2a,.2%s,:0^[22}.!0.0^_1[22}", pe->enc);
      } break;
      case 'u': {
        lcode = lbuf; assert(pe->enc); 
        pe0 = pe->enc; pe1 = pe0 + strlen(pe0) + 1; assert(*pe1);
        sprintf(lbuf, "%%!0.0u?{%s%s]1}.0du?{.0a%s]1}%%%%", pe1, pe0, pe0);
      } break;
      case 'b': {
        lcode = lbuf; assert(pe->enc); 
        pe0 = pe->enc; pe1 = pe0 + strlen(pe0) + 1; assert(*pe1);
        sprintf(lbuf, "%%!1.0u?{%s,.2%s]2}.0du?{.0a,.2%s]2}%%%%", pe1, pe0, pe0);
      } break;
      case '#': /* must have explicit lcode */
        assert(0); 
      case '@': /* must have explicit lcode */
        assert(0); 
      default:
        assert(0); 
    }
    if (!lcode || *lcode == 0) continue;
    ra = mksymbol(internsym(pe->igname));
    hp = rds_global_loc(r, sp, hp); /* ra->ra */
    spush(ra); assert(isbox(ra));
    ra = mkiport_string(sp-r, sialloc(lcode, NULL));
    hp = rds_seq(r, sp, hp);  /* ra=port => ra=revcodelist/eof */
    if (!iseof(ra)) hp = revlist2vec(r, sp, hp); /* ra => ra */
    if (!iseof(ra)) hp = close0(r, sp, hp); /* ra => ra */
    if (!iseof(ra)) boxref(spop()) = ra;
  }
  return hp;
}

/* protects registers from r to sp=r+2; returns new hp */
static obj *init_module(obj *r, obj *sp, obj *hp, const char **mod)
{
  const char **ent;
  /* make sure we are called in a clean vm state */
  assert(r == cxg_regs); assert(sp-r == VM_REGC); /* k, ra (for temp use) */
  /* go over module entries and install/execute */
  for (ent = mod; ent[0] != NULL || ent[1] != NULL; ent += 2) {
    const char *name = ent[0], *data = ent[1];
    /* fprintf(stderr, "## initializing: %s\n%s\n", name?name:"NULL", data); */
    if (name != 0 && name[0] == 'S' && name[1] == 0) {
      /* 'syntax' entry: skip prefix */
      ent += 1; name = ent[0], data = ent[1];
      assert(name != 0); assert(data != 0);
    } else if (name != 0 && name[0] == 'C' && name[1] == 0) {
      /* 'command' entry: skip prefix */
      ent += 1; name = ent[0], data = ent[1];
      assert(name == 0); assert(data != 0);
    } else if (name != 0 && name[0] == 'P' && name[1] == 0) {
      /* 'procedure' entry: make closure and install */
      ent += 1; name = ent[0], data = ent[1];
      assert(name != 0); assert(data != 0);
      /* install code */
      ra = mksymbol(internsym((char*)name));
      hp = rds_global_loc(r, sp, hp); /* ra->ra */
      spush(ra); assert(isbox(ra));
      ra = mkiport_string(sp-r, sialloc((char*)data, NULL));
      hp = rds_seq(r, sp, hp);  /* ra=port => ra=revcodelist/eof */
      if (!iseof(ra)) hp = revlist2vec(r, sp, hp); /* ra => ra */
      if (!iseof(ra)) hp = close0(r, sp, hp); /* ra => ra */
      if (!iseof(ra)) boxref(spop()) = ra;
      continue;
    } else if (name != 0 && name[0] == 'A' && name[1] == 0) {
      /* 'alias' entry: copy transformer */
      obj oldsym, sym, oldbnd, oldden, bnd, al;
      struct intgtab_entry *pe;
      ent += 1; name = ent[0], data = ent[1];
      assert(name != 0); assert(data != 0);
      /* look for dst binding (we allow redefinition) */
      oldsym = mksymbol(internsym((char*)data));
      sym = mksymbol(internsym((char*)name));
      for (oldbnd = 0, al = cx__2Atransformers_2A; al != mknull(); al = cdr(al)) {
        obj ael = car(al);
        if (car(ael) != oldsym) continue;
        oldbnd = ael; break;
      }
      oldden = oldbnd ? cdr(oldbnd) : 0;
      /* missing binding could be an auto-installed integrable */
      if (!oldden && (pe = lookup_integrable(getsymbol(oldsym)))) {
        oldden = mkintegrable(pe);
      }
      /* we should have it now */
      assert(oldden); if (!oldden) continue;
      /* look for existing binding (we allow redefinition) */
      for (bnd = 0, al = cx__2Atransformers_2A; al != mknull(); al = cdr(al)) {
        obj ael = car(al);
        if (car(ael) != sym) continue; 
        bnd = ael; break;
      }
      /* or add new binding */
      spush(oldden); /* protect from gc */
      if (!bnd) { /* acons (sym . #f) */
        hreserve(hbsz(3)*2, sp-r);
        *--hp = obj_from_bool(0); *--hp = sym;
        *--hp = obj_from_size(PAIR_BTAG); bnd = hendblk(3);
        *--hp = cx__2Atransformers_2A; *--hp = bnd;
        *--hp = obj_from_size(PAIR_BTAG); cx__2Atransformers_2A = hendblk(3);
      }
      cdr(bnd) = spop(); /* oldden */
      continue;    
    }
    /* skipped prefix or no prefix */
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
      rz = (obj)(r + VM_STACK_GSZ); /* sp red zone */
      do { /* unwindi trampoline */
        reload_ac(); /* ra => ac */
        reload_ip(); /* rd/rx => ip */
        reload_sp(); /* rs => sp */
        hp = (ins_from_obj(*ip))(IARGS1);
      } while (trampcnd());
      /* r[0] = k, r[1] = random result */
      assert(r == cxg_regs);
      sp = r + VM_REGC;
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
