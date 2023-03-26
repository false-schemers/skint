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

/* box representation extras */
#define boxbsz()      hbsz(1+1)
#define hend_box()    (*--hp = obj_from_size(BOX_BTAG), hendblk(1+1))

/* pair representation extras */
#define pairbsz()     hbsz(2+1)
#define hend_pair()   (*--hp = obj_from_size(PAIR_BTAG), hendblk(2+1))

/* vector representation extras */
#define vecbsz(n)     hbsz((n)+1)
#define hend_vec(n)   (*--hp = obj_from_size(VECTOR_BTAG), hendblk((n)+1))

/* vm closure representation */
#ifdef NDEBUG /* quick */
#define isvmclo(x)    (isobjptr(x) && isobjptr(hblkref(x, 0)))
#define vmcloref(x,i) hblkref(x, i)
#define vmclolen(x)   hblklen(x)
#define vmclobsz(c)   hbsz(c)
#define hend_vmclo(c) hendblk(c)
#else /* slow but thorough */
#define isvmclo       isprocedure
#define vmcloref      *procedureref
#define vmclolen      procedurelen
#define vmclobsz(c)   hbsz(c)
#define hend_vmclo(c) hendblk(c)
#endif

/* vm tuple representation (c != 1) */
#define istuple(x)    (isrecord(x) && recordrtd(x) == 0)  
#define tupleref      recordref
#define tuplelen      recordlen
#define tuplebsz(c)   hbsz((c)+2)
#define hend_tuple(c) (*--hp = 0, *--hp = obj_from_size(RECORD_BTAG), hendblk((c)+2))

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

/* reserving heap memory inside instructions */
#define hp_reserve(n) do { \
 if (unlikely(hp < cxg_heap + (n))) { \
   unload_ac(); unload_ip();   \
   hp = cxm_hgc(r, sp, hp, n); \
   reload_ac(); reload_ip();   \
 } } while (0)
#define hp_reserve_inline(n) \
 ((hp < cxg_heap + (n)) \
 ? (unload_ac(), unload_ip(), hp = cxm_hgc(r, sp, hp, (n)), reload_ac(), reload_ip(), hp) \
 : hp)
#define hp_pushptr(p, pt) \
  (hp_reserve_inline(2), *--hp = (obj)(p), *--hp = (obj)(pt), (obj)(hp+1))   

/* small object representation extras */
#define bool_obj(b) obj_from_bool(b)
#define char_obj(b) obj_from_char(b)
#define void_obj() obj_from_void(0)
#define null_obj() mknull()
#define eof_obj() mkeof()
#define fixnum_obj(x) obj_from_fixnum(x)
#define flonum_obj(x) hp_pushptr(dupflonum(x), FLONUM_NTAG)
#define string_obj(s) hp_pushptr((s), STRING_NTAG)
#define bytevector_obj(s) hp_pushptr((s), BYTEVECTOR_NTAG)
#define iport_file_obj(fp) hp_pushptr((fp), IPORT_FILE_NTAG)
#define oport_file_obj(fp) hp_pushptr((fp), OPORT_FILE_NTAG)
#define iport_string_obj(fp) hp_pushptr((fp), IPORT_STRING_NTAG)
#define oport_string_obj(fp) hp_pushptr((fp), OPORT_STRING_NTAG)

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
    r[2] = hend_vmclo(1);
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
  ac = void_obj();
  unwindi(0); 
}

define_instrhelper(cxi_failactype) { 
  char *msg = (char*)spop(); obj p;
  fprintf(stderr, "run-time failure: argument is not a %s:\n", msg); 
  p = oport_file_obj(stderr); spush(p);
  oportputcircular(ac, p, 0);
  fputc('\n', stderr);
  spop();
  ac = void_obj();
  unwindi(0); 
}

#define fail(msg) do { ac = (obj)msg; musttail return cxi_fail(IARGS); } while (0)
#define failtype(x, msg) do { ac = (x); spush((obj)msg); musttail return cxi_failactype(IARGS); } while (0) 
#define failactype(msg) do { spush((obj)msg); musttail return cxi_failactype(IARGS); } while (0) 

#define ckp(x) do { obj _x = (x); if (unlikely(!ispair(_x))) \
  { ac = _x; spush((obj)"pair"); musttail return cxi_failactype(IARGS); } } while (0)
#define ckl(x) do { obj _x = (x); if (unlikely(!islist(_x))) \
  { ac = _x; spush((obj)"list"); musttail return cxi_failactype(IARGS); } } while (0)
#define cku(x) do { obj _x = (x); if (unlikely(!isnull(_x))) \
  { ac = _x; spush((obj)"proper list"); musttail return cxi_failactype(IARGS); } } while (0)
#define ckv(x) do { obj _x = (x); if (unlikely(!isvector(_x))) \
  { ac = _x; spush((obj)"vector"); musttail return cxi_failactype(IARGS); } } while (0)
#define ckc(x) do { obj _x = (x); if (unlikely(!is_char_obj(_x))) \
  { ac = _x; spush((obj)"char"); musttail return cxi_failactype(IARGS); } } while (0)
#define cks(x) do { obj _x = (x); if (unlikely(!isstring(_x))) \
  { ac = _x; spush((obj)"string"); musttail return cxi_failactype(IARGS); } } while (0)
#define ckb(x) do { obj _x = (x); if (unlikely(!isbytevector(_x))) \
  { ac = _x; spush((obj)"bytevector"); musttail return cxi_failactype(IARGS); } } while (0)
#define cki(x) do { obj _x = (x); if (unlikely(!is_fixnum_obj(_x))) \
  { ac = _x; spush((obj)"fixnum"); musttail return cxi_failactype(IARGS); } } while (0)
#define ckj(x) do { obj _x = (x); if (unlikely(!is_flonum_obj(_x))) \
  { ac = _x; spush((obj)"flonum"); musttail return cxi_failactype(IARGS); } } while (0)
#define ckn(x) do { obj _x = (x); if (unlikely(!is_fixnum_obj(_x) && !is_flonum_obj(_x))) \
  { ac = _x; spush((obj)"number"); musttail return cxi_failactype(IARGS); } } while (0)
#define ckk(x) do { obj _x = (x); if (unlikely(!is_fixnum_obj(_x) || fixnum_from_obj(_x) < 0)) \
  { ac = _x; spush((obj)"nonnegative fixnum"); musttail return cxi_failactype(IARGS); } } while (0)
#define ck8(x) do { obj _x = (x); if (unlikely(!is_byte_obj(_x))) \
  { ac = _x; spush((obj)"byte"); musttail return cxi_failactype(IARGS); } } while (0)
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

define_instruction(panic) {
  obj l, p; cks(ac); ckl(sref(0));
  p = oport_file_obj(stderr);  
  fprintf(stderr, "error: %s", stringchars(ac));
  if (ispair(sref(0))) fputs(":\n", stderr); 
  else fputs("\n", stderr);
  for (l = sref(0); ispair(l); l = cdr(l)) {
    oportputcircular(car(l), p, 0);
    fputc('\n', stderr);
  }
  sdrop(1);
  ac = void_obj();
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
  ac = hend_vmclo(c); /* closure */
  sdrop(n);   
  gonexti();
}

define_instruction(sbox) {
  int i = fixnum_from_obj(*ip++); 
  hp_reserve(boxbsz());
  *--hp = sref(i);  
  sref(i) = hend_box();
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
  ckx(ac); rd = ac; rx = fixnum_obj(0);
  ac = fixnum_obj(n); /* argc */
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
    rd = t; rx = fixnum_obj(0); 
    ac = fixnum_obj(0);
    callsubi();
  } else { 
    /* arrange return to cwmv code w/x */
    spush(x);
    spush(cx_callmv_2Dadapter_2Dclosure); 
    spush(fixnum_obj(0));
    /* call the producer */
    rd = t; rx = fixnum_obj(0); ac = fixnum_obj(0); 
    callsubi(); 
  }
}

define_instruction(rcmv) {
  /* single-value producer call returns here with result in ac, cns on stack */
  obj val = ac, x = spop();
  /* tail-call the consumer with the returned value */
  spush(val); ac = fixnum_obj(1);
  rd = x; rx = fixnum_obj(0); 
  callsubi();
}

define_instruction(sdmv) {
  /* sending values on stack, ac contains argc */
  if (ac == fixnum_obj(1)) {
    /* can return anywhere, including rcmv */
    ac = spop();
    rx = spop();
    rd = spop();
    retfromi();
  } else {
    /* can only pseudo-return to rcmv */
    int n = fixnum_from_obj(ac), m = 3, i;
    if (sref(n) == fixnum_obj(0) && sref(n+1) == cx_callmv_2Dadapter_2Dclosure) {
      /* tail-call the consumer with the produced values */
      rd = sref(n+2); rx = fixnum_obj(0); /* cns */
      /* NB: can be sped up for popular cases: n == 0, n == 2 */
      memmove((void*)(sp-n-m), (void*)(sp-n), (size_t)n*sizeof(obj));
      sdrop(m); callsubi();
    } else { /* return args as a tuple (n != 1) */
      hp_reserve(tuplebsz(n));
      for (i = n-1; i >= 0; --i) *--hp = sref(i);
      ac = hend_tuple(n);
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
  ac = hend_vmclo(n+1);
  gonexti();
}

define_instruction(lck0) {
  int n; cki(sref(0)); ckx(sref(1));
  n = (int)(sp-(r+VM_REGC));
  hp_reserve(vmclobsz(n+1));
  hp -= n; memcpy(hp, sp-n, n*sizeof(obj));
  *--hp = cx_continuation_2Dadapter_2Dcode;
  ac = hend_vmclo(n+1);
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
  rd = t; rx = fixnum_obj(0); 
  ac = fixnum_obj(0);
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
  if (ac == fixnum_obj(1)) { /* easy, popular case */
    ac = rd; 
    goi(wckr);
  } else { /* multiple results case */
    int c = fixnum_from_obj(ac), n = vmclolen(rd) - 1, i;
    obj *ks = &vmcloref(rd, 1), *ke = ks + n;
    if (ke-ks > 3 && *--ke == fixnum_obj(0) && *--ke == cx_callmv_2Dadapter_2Dclosure) {
      obj *sb = r + VM_REGC;
      rd = *--ke; rx = fixnum_obj(0); n = ke - ks; /* cns */
      /* arrange stack as follows: [ks..ke] [arg ...] */
      assert((cxg_rend - cxg_regs - VM_REGC) > n + c);
      if (c) memmove(sb+n, sp-c, c*sizeof(obj));
      memcpy(sb, ks, n*sizeof(obj));
      sp = sb+n+c; callsubi();
    } else { /* return args as a tuple (c != 1) */
      hp_reserve(tuplebsz(c));
      for (i = c-1; i >= 0; --i) *--hp = sref(i);
      ac = hend_tuple(c);
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
  spush(fixnum_obj(ip + dx - &vectorref(vmcloref(rd, 0), 0)));  
  gonexti();
}

define_instruction(push) { spush(ac); gonexti(); }

define_instruction(jdceq) {
  obj v = *ip++, i = *ip++;
  if (ac == v) {
    rd = dref(fixnum_from_obj(i));
    rx = fixnum_obj(0);
    callsubi();
  }
  gonexti();
}

define_instruction(jdcge) {
  obj v = *ip++, i = *ip++;
  if (ac >= v) { /* unsigned tagged fixnums can be compared as-is */
    rd = dref(fixnum_from_obj(i));
    rx = fixnum_obj(0);
    callsubi();
  }
  gonexti();
}

define_instruction(jdref) {
  int i = fixnum_from_obj(*ip++); 
  rd = dref(i); 
  rx = fixnum_obj(0);
  callsubi();
}

define_instruction(call) {
  int n = fixnum_from_obj(*ip++); 
  ckx(ac); rd = ac; rx = fixnum_obj(0); 
  ac = fixnum_obj(n); /* argc */
  callsubi();
}

define_instruction(scall) {
  int m = fixnum_from_obj(*ip++), n = fixnum_from_obj(*ip++);
  ckx(ac); rd = ac; rx = fixnum_obj(0); 
  ac = fixnum_obj(n); /* argc */
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
    spush(null_obj());
  } else {
    obj l = null_obj();
    hp_reserve(pairbsz()*m);
    while (m > 0) { 
      *--hp = l; *--hp = sref(n + m - 1);
      l = hend_pair(); 
      --m;
    }
    memmove((void*)(sp-c), (void*)(sp-n), (size_t)n*sizeof(obj));
    sdrop(c-n);
    spush(l); 
  }
  ac = fixnum_obj(n+1);
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
  ac = fixnum_obj(0);
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
  ac = bool_obj(x == y);
  gonexti(); 
}

define_instruction(isv) {
  obj x = ac, y = spop();
  ac = bool_obj(iseqv(x,y));
  gonexti(); 
}

define_instruction(ise) {
  obj x = ac, y = spop();
  ac = bool_obj(isequal(x,y));
  gonexti(); 
}

define_instruction(unbox) { ckz(ac); ac = boxref(ac); gonexti(); }
define_instruction(setbox) { ckz(ac); boxref(ac) = spop(); gonexti(); }

define_instruction(box) {
  hp_reserve(boxbsz());
  *--hp = ac;
  ac = hend_box();
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
  ac = bool_obj(isnull(ac));
  gonexti();
}

define_instruction(pairp) {
  ac = bool_obj(ispair(ac));
  gonexti();
}

define_instruction(cons) {
  hp_reserve(pairbsz());
  *--hp = spop(); /* cdr */
  *--hp = ac;     /* car */
  ac = hend_pair();
  gonexti();
}

define_instruction(listp) {
  ac = bool_obj(islist(ac));
  gonexti();
}

define_instruction(list) {
  int i, n = fixnum_from_obj(*ip++);
  hp_reserve(pairbsz()*n);
  for (ac = null_obj(), i = n-1; i >= 0; --i) {
    *--hp = ac;      /* cdr */
    *--hp = sref(i); /* car */
    ac = hend_pair();
  }
  sdrop(n);
  gonexti();
}

define_instruction(lmk) {
  int i, n; obj v; ckk(ac);
  n = fixnum_from_obj(ac); 
  hp_reserve(pairbsz()*n); v = sref(0);
  ac = null_obj();
  for (i = 0; i < n; ++i) {
    *--hp = ac; /* cdr */
    *--hp = v;  /* car */
    ac = hend_pair();
  }
  sdrop(1);
  gonexti();
}

define_instruction(llen) {
  int n = 0;
  while (ispair(ac)) { ac = cdr(ac); ++n; }
  ac = fixnum_obj(n);
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
  ac = bool_obj(0);
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
  ac = bool_obj(0);
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
  obj l = bool_obj(0);
  while (ispair(ac)) { l = ac; ac = cdr(ac); }
  ac = l;
  gonexti();
}

define_instruction(lrev) {
  obj l = ac, o = null_obj(); int n = 0;
  while (ispair(ac)) { ac = cdr(ac); ++n; }
  hp_reserve(pairbsz()*n);
  for (ac = l; ac != null_obj(); ac = cdr(ac)) { 
    *--hp = o; *--hp = car(ac);
    o = hend_pair(); 
  }
  ac = o;
  gonexti();
}

define_instruction(lrevi) {
  obj t, v = null_obj();
  while (ispair(ac)) t = cdr(ac), cdr(ac) = v, v = ac, ac = t;
  ac = v;
  gonexti();
}

define_instruction(charp) {
  ac = bool_obj(is_char_obj(ac));
  gonexti();
}

define_instruction(strp) {
  ac = bool_obj(isstring(ac));
  gonexti();
}

define_instruction(str) {
  int i, n = fixnum_from_obj(*ip++);
  obj o = string_obj(allocstring(n, ' '));
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
  ac = string_obj(allocstring(n, c)); 
  gonexti();
}

define_instruction(slen) {
  cks(ac);
  ac = fixnum_obj(stringlen(ac));
  gonexti();
}

define_instruction(sget) {
  obj x = spop(); int i; 
  cks(ac); ckk(x); 
  i = fixnum_from_obj(x); 
  if (i >= stringlen(ac)) failtype(x, "valid string index");
  ac = char_obj(*stringref(ac, i));
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
  ac = string_obj(d);
  gonexti();
}

define_instruction(ssub) {
  obj x = spop(), y = spop(); int is, ie, *d; 
  cks(ac); ckk(x); ckk(y); 
  is = fixnum_from_obj(x), ie = fixnum_from_obj(y);
  if (is > ie) failtype(x, "valid start string index");
  if (ie > stringlen(ac)) failtype(y, "valid end string index");
  d = substring(stringdata(ac), is, ie);
  ac = string_obj(d);
  gonexti();
}

define_instruction(spos) {
  obj x = ac, y = spop(); char *s, *p;
  ckc(x); cks(y);
  s = stringchars(y), p = strchr(s, char_from_obj(x));
  ac = p ? fixnum_obj(p-s) : bool_obj(0);
  gonexti();
}


define_instruction(bvecp) {
  ac = bool_obj(isbytevector(ac));
  gonexti();
}

define_instruction(bvec) {
  int i, n = fixnum_from_obj(*ip++);
  obj o = bytevector_obj(allocbytevector(n, 0));
  unsigned char *s = (unsigned char *)bytevectorbytes(o);
  for (i = 0; i < n; ++i) {
    obj x = sref(i); ck8(x); s[i] = byte_from_obj(x);
  }
  sdrop(n); ac = o;
  gonexti();
}

define_instruction(bmk) {
  int n, b; obj x = spop(); 
  ckk(ac); ck8(x);
  n = fixnum_from_obj(ac), b = byte_from_obj(x);
  ac = bytevector_obj(allocbytevector(n, b)); 
  gonexti();
}

define_instruction(blen) {
  ckb(ac);
  ac = fixnum_obj(bytevectorlen(ac));
  gonexti();
}

define_instruction(bget) {
  obj x = spop(); int i; 
  ckb(ac); ckk(x); 
  i = fixnum_from_obj(x); 
  if (i >= bytevectorlen(ac)) failtype(x, "valid bytevector index");
  ac = fixnum_obj(*bytevectorref(ac, i));
  gonexti();
}

define_instruction(bput) {
  obj x = spop(), y = spop(); int i; 
  ckb(ac); ckk(x); ck8(y); 
  i = fixnum_from_obj(x); 
  if (i >= bytevectorlen(ac)) failtype(x, "valid bytevector index");
  *bytevectorref(ac, i) = byte_from_obj(y);
  gonexti();
}

define_instruction(bsub) {
  obj x = spop(), y = spop(); int is, ie, *d; 
  ckb(ac); ckk(x); ckk(y); 
  is = fixnum_from_obj(x), ie = fixnum_from_obj(y);
  if (is > ie) failtype(x, "valid start bytevector index");
  if (ie > bytevectorlen(ac)) failtype(y, "valid end bytevector index");
  d = subbytevector(bytevectordata(ac), is, ie);
  ac = bytevector_obj(d);
  gonexti();
}

define_instruction(beq) {
  obj x = ac, y = spop(); ckb(x); ckb(y);
  ac = bool_obj(bytevectoreq(bytevectordata(x), bytevectordata(y)));
  gonexti(); 
}


define_instruction(vecp) {
  ac = bool_obj(isvector(ac));
  gonexti();
}

define_instruction(vec) {
  int i, n = fixnum_from_obj(*ip++);
  hp_reserve(vecbsz(n));
  for (i = n-1; i >= 0; --i) *--hp = sref(i);
  ac = hend_vec(n);
  sdrop(n);
  gonexti();
}

define_instruction(vmk) {
  int i, n; obj v; ckk(ac);
  n = fixnum_from_obj(ac); 
  hp_reserve(vecbsz(n)); v = sref(0);
  for (i = 0; i < n; ++i) *--hp = v;
  ac = hend_vec(n);
  sdrop(1);
  gonexti();
}

define_instruction(vlen) {
  ckv(ac);
  ac = fixnum_obj(vectorlen(ac));
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
  hp_reserve(vecbsz(n));
  /* NB: vectorref fails to return pointer to empty vector's start */
  hp -= n2; if (n2) memcpy(hp, &vectorref(y, 0), n2*sizeof(obj));
  hp -= n1; if (n1) memcpy(hp, &vectorref(x, 0), n1*sizeof(obj));
  ac = hend_vec(n);
  sdrop(1);
  gonexti();
}

define_instruction(vtol) {
  obj l = null_obj(); int n;
  ckv(ac); n = vectorlen(ac);
  hp_reserve(pairbsz()*n);
  while (n > 0) {
    *--hp = l; *--hp = vectorref(ac, n-1);
    l = hend_pair();
    --n;
  }
  ac = l;
  gonexti();
}

define_instruction(ltov) {
  obj l = ac; int n = 0, i;
  while (ispair(l)) { l = cdr(l); ++n; } cku(l);
  hp_reserve(vecbsz(n));
  for (l = ac, i = 0, hp -= n; i < n; ++i, l = cdr(l)) hp[i] = car(l);
  ac = hend_vec(n);
  gonexti();
}

define_instruction(ltob) {
  obj l = ac, o; int n = 0, i; unsigned char *s;
  while (ispair(l)) { l = cdr(l); ++n; } cku(l);
  o = bytevector_obj(allocbytevector(n, 0));
  s = bytevectorbytes(o);
  for (i = 0, l = ac; i < n; ++i, l = cdr(l)) {
    obj x = car(l); ck8(x);
    s[i] = byte_from_obj(x);
  } 
  ac = o;
  gonexti();
}

define_instruction(stol) {
  obj l = null_obj(); int n;
  cks(ac); n = stringlen(ac);
  hp_reserve(pairbsz()*n);
  while (n > 0) {
    *--hp = l; *--hp = char_obj(*stringref(ac, n-1));
    l = hend_pair();
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
  ac = string_obj(d);
  gonexti();
}

define_instruction(ytos) {
  cky(ac);
  ac = string_obj(newstring(symbolname(getsymbol(ac))));
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
  ac = string_obj(newstring(s));  
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
  if (errno || l < FIXNUM_MIN || l > FIXNUM_MAX || e == s || *e) ac = bool_obj(0);
  else ac = fixnum_obj(l);
  gonexti();
}

define_instruction(ctoi) {
  ckc(ac);
  ac = fixnum_obj(char_from_obj(ac));
  gonexti();
}

define_instruction(itoc) {
  cki(ac); /* TODO: range check? */
  ac = char_obj(fixnum_from_obj(ac));
  gonexti();
}

define_instruction(jtos) {
  char buf[30], *s; double d; ckj(ac);
  d = flonum_from_obj(ac); sprintf(buf, "%.15g", d);
  for (s = buf; *s != 0; s++) if (strchr(".eE", *s)) break;
  if (d != d) strcpy(buf, "+nan.0"); else if (d <= -HUGE_VAL) strcpy(buf, "-inf.0");
  else if (d >= HUGE_VAL) strcpy(buf, "+inf.0"); else if (*s == 'E') *s = 'e'; 
  else if (*s == 0) {  *s++ = '.'; *s++ = '0'; *s = 0; }
  ac = string_obj(newstring(buf));
  gonexti();
}

define_instruction(stoj) {
  char *e = "", *s; double d; cks(ac);
  s = stringchars(ac); errno = 0;
  if (*s != '+' && *s != '-') d = strtod(s, &e);
  else if (strcmp_ci(s+1, "inf.0") == 0) d = (*s == '-' ? -HUGE_VAL : HUGE_VAL); 
  else if (strcmp_ci(s+1, "nan.0") == 0) d = HUGE_VAL - HUGE_VAL; 
  else d = strtod(s, &e);
  if (errno || e == s || *e) ac = bool_obj(0);
  else ac = flonum_obj(d);
  gonexti();
}

define_instruction(ntos) {
  if (is_fixnum_obj(ac)) goi(itos);
  if (unlikely(spop() != fixnum_obj(10))) fail("non-10 radix in flonum conversion");
  goi(jtos);
}

define_instruction(ston) {
  char *s; int radix; long l; double d;
  obj x = ac, y = spop(); cks(x); ckk(y);
  s = stringchars(x); radix = fixnum_from_obj(y);
  if (radix < 2 || radix > 10 + 'z' - 'a') failtype(y, "valid radix");
  switch (strtofxfl(s, radix, &l, &d)) {
    case 'e': ac = fixnum_obj(l); break;
    case 'i': ac = flonum_obj(d); break;
    default : ac = bool_obj(0); break;
  }
  gonexti();
}

define_instruction(not) {
  ac = bool_obj(ac == 0);
  gonexti();
}

define_instruction(izerop) {
  cki(ac);
  ac = bool_obj(ac == fixnum_obj(0));
  gonexti();
}

define_instruction(iposp) {
  cki(ac);
  ac = bool_obj(fixnum_from_obj(ac) > 0);
  gonexti(); 
}

define_instruction(inegp) {
  cki(ac);
  ac = bool_obj(fixnum_from_obj(ac) < 0);
  gonexti(); 
}

define_instruction(ievnp) {
  cki(ac);
  ac = bool_obj((fixnum_from_obj(ac) & 1) == 0);
  gonexti(); 
}

define_instruction(ioddp) {
  cki(ac);
  ac = bool_obj((fixnum_from_obj(ac) & 1) != 0);
  gonexti(); 
}

define_instruction(iadd) {
  obj x = ac, y = spop(); cki(x); cki(y);
  ac = fixnum_obj(fxadd(fixnum_from_obj(x), fixnum_from_obj(y)));
  gonexti(); 
}

define_instruction(isub) {
  obj x = ac, y = spop(); cki(x); cki(y);
  ac = fixnum_obj(fxsub(fixnum_from_obj(x), fixnum_from_obj(y)));
  gonexti(); 
}

define_instruction(imul) {
  obj x = ac, y = spop(); cki(x); cki(y);
  ac = fixnum_obj(fxmul(fixnum_from_obj(x), fixnum_from_obj(y)));
  gonexti();
}

define_instruction(iquo) {
  obj x = ac, y = spop(); cki(x); cki(y);
  if (y == fixnum_obj(0)) fail("division by zero");
  ac = fixnum_obj(fxquo(fixnum_from_obj(x), fixnum_from_obj(y)));
  gonexti();
}

define_instruction(irem) {
  obj x = ac, y = spop(); cki(x); cki(y);
  if (y == fixnum_obj(0)) fail("division by zero");
  ac = fixnum_obj(fxrem(fixnum_from_obj(x), fixnum_from_obj(y)));
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
  ac = fixnum_obj(fxneg(fixnum_from_obj(ac)));
  gonexti();
}

define_instruction(iabs) {
  cki(ac);
  ac = fixnum_obj(fxabs(fixnum_from_obj(ac)));
  gonexti();
}

define_instruction(itoj) {
  cki(ac);
  ac = flonum_obj((double)fixnum_from_obj(ac));
  gonexti();
}

define_instruction(idiv) {
  obj x = ac, y = spop(); cki(x); cki(y);
  if (y == fixnum_obj(0)) fail("division by zero");
  ac = fixnum_obj(fxdiv(fixnum_from_obj(x), fixnum_from_obj(y)));
  gonexti();
}

define_instruction(imqu) {
  obj x = ac, y = spop(); cki(x); cki(y);
  if (y == fixnum_obj(0)) fail("division by zero");
  ac = fixnum_obj(fxmqu(fixnum_from_obj(x), fixnum_from_obj(y)));
  gonexti();
}

define_instruction(imlo) {
  obj x = ac, y = spop(); cki(x); cki(y);
  if (y == fixnum_obj(0)) fail("division by zero");
  ac = fixnum_obj(fxmlo(fixnum_from_obj(x), fixnum_from_obj(y)));
  gonexti();
}

define_instruction(ieuq) {
  obj x = ac, y = spop(); cki(x); cki(y);
  if (y == fixnum_obj(0)) fail("division by zero");
  ac = fixnum_obj(fxeuq(fixnum_from_obj(x), fixnum_from_obj(y)));
  gonexti();
}

define_instruction(ieur) {
  obj x = ac, y = spop(); cki(x); cki(y);
  if (y == fixnum_obj(0)) fail("division by zero");
  ac = fixnum_obj(fxeur(fixnum_from_obj(x), fixnum_from_obj(y)));
  gonexti();
}

define_instruction(igcd) {
  obj x = ac, y = spop(); cki(x); cki(y);
  ac = fixnum_obj(fxgcd(fixnum_from_obj(x), fixnum_from_obj(y)));
  gonexti();
}

define_instruction(ipow) {
  obj x = ac, y = spop(); cki(x); cki(y);
  ac = fixnum_obj(fxpow(fixnum_from_obj(x), fixnum_from_obj(y)));
  gonexti();
}

define_instruction(isqrt) {
  cki(ac); /* TODO: check for negative */
  ac = fixnum_obj(fxsqrt(fixnum_from_obj(ac)));
  gonexti();
}

define_instruction(inot) {
  cki(ac);
  ac = fixnum_obj(~fixnum_from_obj(ac));
  gonexti();
}

define_instruction(iand) {
  obj x = ac, y = spop(); cki(x); cki(y);
  ac = fixnum_obj(fixnum_from_obj(x) & fixnum_from_obj(y));
  gonexti();
}

define_instruction(iior) {
  obj x = ac, y = spop(); cki(x); cki(y);
  ac = fixnum_obj(fixnum_from_obj(x) | fixnum_from_obj(y));
  gonexti();
}

define_instruction(ixor) {
  obj x = ac, y = spop(); cki(x); cki(y);
  ac = fixnum_obj(fixnum_from_obj(x) ^ fixnum_from_obj(y));
  gonexti();
}

define_instruction(iasl) {
  obj x = ac, y = spop(); cki(x); cki(y);
  ac = fixnum_obj(fxasl(fixnum_from_obj(x), fixnum_from_obj(y)));
  gonexti();
}

define_instruction(iasr) {
  obj x = ac, y = spop(); cki(x); cki(y);
  ac = fixnum_obj(fxasr(fixnum_from_obj(x), fixnum_from_obj(y)));
  gonexti();
}

define_instruction(ilt) {
  obj x = ac, y = spop(); cki(x); cki(y);
  ac = bool_obj(fixnum_from_obj(x) < fixnum_from_obj(y));
  gonexti(); 
}

define_instruction(igt) {
  obj x = ac, y = spop(); cki(x); cki(y);
  ac = bool_obj(fixnum_from_obj(x) > fixnum_from_obj(y));
  gonexti(); 
}

define_instruction(ile) {
  obj x = ac, y = spop(); cki(x); cki(y);
  ac = bool_obj(fixnum_from_obj(x) <= fixnum_from_obj(y));
  gonexti(); 
}

define_instruction(ige) {
  obj x = ac, y = spop(); cki(x); cki(y);
  ac = bool_obj(fixnum_from_obj(x) >= fixnum_from_obj(y));
  gonexti(); 
}

define_instruction(ieq) {
  obj x = ac, y = spop(); cki(x); cki(y);
  ac = bool_obj(x == y);
  gonexti(); 
}

define_instruction(ine) {
  obj x = ac, y = spop(); cki(x); cki(y);
  ac = bool_obj(x != y);
  gonexti(); 
}


define_instruction(jzerop) {
  ckj(ac);
  ac = bool_obj(flonum_from_obj(ac) == 0.0);
  gonexti();
}

define_instruction(jposp) {
  ckj(ac);
  ac = bool_obj(flonum_from_obj(ac) > 0.0);
  gonexti(); 
}

define_instruction(jnegp) {
  ckj(ac);
  ac = bool_obj(flonum_from_obj(ac) < 0.0);
  gonexti(); 
}

define_instruction(jevnp) {
  double f; ckj(ac);
  f = flonum_from_obj(ac);
  ac = bool_obj(flisint(f / 2.0));
  gonexti(); 
}

define_instruction(joddp) {
  double f; ckj(ac);
  f = flonum_from_obj(ac);
  ac = bool_obj(flisint((f + 1.0) / 2.0));
  gonexti(); 
}

define_instruction(jintp) {
  double f; ckj(ac);
  f = flonum_from_obj(ac);
  ac = bool_obj(f > -HUGE_VAL && f < HUGE_VAL && f == floor(f));
  gonexti(); 
}

define_instruction(jnanp) {
  double f; ckj(ac);
  f = flonum_from_obj(ac);
  ac = bool_obj(f != f);
  gonexti(); 
}

define_instruction(jfinp) {
  double f; ckj(ac);
  f = flonum_from_obj(ac);
  ac = bool_obj(f > -HUGE_VAL && f < HUGE_VAL);
  gonexti(); 
}

define_instruction(jinfp) {
  double f; ckj(ac);
  f = flonum_from_obj(ac);
  ac = bool_obj(f <= -HUGE_VAL || f >= HUGE_VAL);
  gonexti(); 
}

define_instruction(jadd) {
  obj x = ac, y = spop(); ckj(x); ckj(y);
  ac = flonum_obj(flonum_from_obj(x) + flonum_from_obj(y));
  gonexti(); 
}

define_instruction(jsub) {
  obj x = ac, y = spop(); ckj(x); ckj(y);
  ac = flonum_obj(flonum_from_obj(x) - flonum_from_obj(y));
  gonexti(); 
}

define_instruction(jmul) {
  obj x = ac, y = spop(); ckj(x); ckj(y);
  ac = flonum_obj(flonum_from_obj(x) * flonum_from_obj(y));
  gonexti();
}

define_instruction(jdiv) {
  obj x = ac, y = spop(); ckj(x); ckj(y);
  ac = flonum_obj(flonum_from_obj(x) / flonum_from_obj(y));
  gonexti();
}

define_instruction(jrem) {
  obj x = ac, y = spop(); ckj(x); ckj(y);
  ac = flonum_obj(flrem(flonum_from_obj(x), flonum_from_obj(y)));
  gonexti();
}

define_instruction(jlt) {
  obj x = ac, y = spop(); ckj(x); ckj(y);
  ac = bool_obj(flonum_from_obj(x) < flonum_from_obj(y));
  gonexti(); 
}

define_instruction(jgt) {
  obj x = ac, y = spop(); ckj(x); ckj(y);
  ac = bool_obj(flonum_from_obj(x) > flonum_from_obj(y));
  gonexti(); 
}

define_instruction(jle) {
  obj x = ac, y = spop(); ckj(x); ckj(y);
  ac = bool_obj(flonum_from_obj(x) <= flonum_from_obj(y));
  gonexti(); 
}

define_instruction(jge) {
  obj x = ac, y = spop(); ckj(x); ckj(y);
  ac = bool_obj(flonum_from_obj(x) >= flonum_from_obj(y));
  gonexti(); 
}

define_instruction(jeq) {
  obj x = ac, y = spop(); ckj(x); ckj(y);
  ac = bool_obj(flonum_from_obj(x) == flonum_from_obj(y));
  gonexti(); 
}

define_instruction(jne) {
  obj x = ac, y = spop(); ckj(x); ckj(y);
  ac = bool_obj(flonum_from_obj(x) != flonum_from_obj(y));
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
  ac = flonum_obj(-flonum_from_obj(ac));
  gonexti(); 
}

define_instruction(jabs) {
  ckj(ac);
  ac = flonum_obj(fabs(flonum_from_obj(ac)));
  gonexti(); 
}

define_instruction(jgcd) {
  obj x = ac, y = spop(); ckj(x); ckj(y);
  ac = flonum_obj(flgcd(flonum_from_obj(x), flonum_from_obj(y)));
  gonexti(); 
}

define_instruction(jpow) {
  obj x = ac, y = spop(); ckj(x); ckj(y);
  ac = flonum_obj(pow(flonum_from_obj(x), flonum_from_obj(y)));
  gonexti(); 
}

define_instruction(jsqrt) {
  ckj(ac);
  ac = flonum_obj(sqrt(flonum_from_obj(ac)));
  gonexti(); 
}

define_instruction(jtoi) {
  ckj(ac);
  ac = fixnum_obj(fxflo(flonum_from_obj(ac)));
  gonexti();
}

define_instruction(jquo) {
  obj x = ac, y = spop(); ckj(x); ckj(y);
  ac = flonum_obj(flquo(flonum_from_obj(x), flonum_from_obj(y)));
  gonexti(); 
}

define_instruction(jmqu) {
  obj x = ac, y = spop(); ckj(x); ckj(y);
  ac = flonum_obj(flmqu(flonum_from_obj(x), flonum_from_obj(y)));
  gonexti(); 
}

define_instruction(jmlo) {
  obj x = ac, y = spop(); ckj(x); ckj(y);
  ac = flonum_obj(flmlo(flonum_from_obj(x), flonum_from_obj(y)));
  gonexti(); 
}

define_instruction(jfloor) {
  ckj(ac);
  ac = flonum_obj(floor(flonum_from_obj(ac)));
  gonexti(); 
}

define_instruction(jceil) {
  ckj(ac);
  ac = flonum_obj(ceil(flonum_from_obj(ac)));
  gonexti(); 
}

define_instruction(jtrunc) {
  double f, i; ckj(ac);
  f = flonum_from_obj(ac);
  modf(f,  &i);
  ac = flonum_obj(i);
  gonexti(); 
}

define_instruction(jround) {
  ckj(ac);
  ac = flonum_obj(flround(flonum_from_obj(ac)));
  gonexti(); 
}

define_instruction(jexp) {
  ckj(ac);
  ac = flonum_obj(exp(flonum_from_obj(ac)));
  gonexti(); 
}

define_instruction(jlog) {
  obj x = ac, y = spop(); ckj(ac);
  if (likely(!y)) {
    ac = flonum_obj(log(flonum_from_obj(ac)));
  } else {
    double b; ckj(y); b = flonum_from_obj(y);
    if (likely(b == 10.0)) ac = flonum_obj(log10(flonum_from_obj(ac)));
    else ac = flonum_obj(log(flonum_from_obj(ac))/log(b));
  }
  gonexti(); 
}

define_instruction(jsin) {
  ckj(ac);
  ac = flonum_obj(sin(flonum_from_obj(ac)));
  gonexti(); 
}

define_instruction(jcos) {
  ckj(ac);
  ac = flonum_obj(cos(flonum_from_obj(ac)));
  gonexti(); 
}

define_instruction(jtan) {
  ckj(ac);
  ac = flonum_obj(tan(flonum_from_obj(ac)));
  gonexti(); 
}

define_instruction(jasin) {
  ckj(ac);
  ac = flonum_obj(asin(flonum_from_obj(ac)));
  gonexti(); 
}

define_instruction(jacos) {
  ckj(ac);
  ac = flonum_obj(acos(flonum_from_obj(ac)));
  gonexti(); 
}

define_instruction(jatan) {
  obj x = ac, y = spop(); ckj(ac);
  if (likely(!y)) {
    ac = flonum_obj(atan(flonum_from_obj(x)));
  } else {
    ckj(y); 
    ac = flonum_obj(atan2(flonum_from_obj(x), flonum_from_obj(y)));
  }
  gonexti(); 
}


define_instruction(zerop) {
  obj x = ac;
  if (likely(is_fixnum_obj(x))) {
    ac = bool_obj(x == fixnum_obj(0));
  } else if (likely(is_flonum_obj(x))) {
    ac = bool_obj(flonum_from_obj(x) == 0.0);
  } else failactype("number");
  gonexti();
}

define_instruction(posp) {
  if (likely(is_fixnum_obj(ac))) ac = bool_obj(get_fixnum_unchecked(ac) > 0);
  else if (likely(is_flonum_obj(ac))) ac = bool_obj(flonum_from_obj(ac) > 0.0);
  else failactype("number");
  gonexti(); 
}

define_instruction(negp) {
  if (likely(is_fixnum_obj(ac))) ac = bool_obj(get_fixnum_unchecked(ac) < 0);
  else if (likely(is_flonum_obj(ac))) ac = bool_obj(flonum_from_obj(ac) < 0.0);
  else failactype("number");
  gonexti(); 
}

define_instruction(add) {
  obj x = ac, y = spop();
  if (likely(are_fixnum_objs(x, y))) {
    ac = fixnum_obj(fxadd(get_fixnum_unchecked(x), get_fixnum_unchecked(y)));
  } else {
    double dx, dy;
    if (likely(is_flonum_obj(x))) dx = flonum_from_obj(x);
    else if (likely(is_fixnum_obj(x))) dx = (double)fixnum_from_obj(x);
    else failtype(x, "number");
    if (likely(is_flonum_obj(y))) dy = flonum_from_obj(y);
    else if (likely(is_fixnum_obj(y))) dy = (double)fixnum_from_obj(y);
    else failtype(y, "number");
    ac = flonum_obj(dx + dy);
  }
  gonexti(); 
}

define_instruction(sub) {
  obj x = ac, y = spop();
  if (likely(are_fixnum_objs(x, y))) {
    ac = fixnum_obj(fxsub(get_fixnum_unchecked(x), get_fixnum_unchecked(y)));
  } else {
    double dx, dy;
    if (likely(is_flonum_obj(x))) dx = flonum_from_obj(x);
    else if (likely(is_fixnum_obj(x))) dx = (double)fixnum_from_obj(x);
    else failtype(x, "number");
    if (likely(is_flonum_obj(y))) dy = flonum_from_obj(y);
    else if (likely(is_fixnum_obj(y))) dy = (double)fixnum_from_obj(y);
    else failtype(y, "number");
    ac = flonum_obj(dx - dy);
  }
  gonexti(); 
}

define_instruction(mul) {
  obj x = ac, y = spop();
  if (likely(are_fixnum_objs(x, y))) {
    ac = fixnum_obj(fxmul(get_fixnum_unchecked(x), get_fixnum_unchecked(y)));
  } else {
    double dx, dy;
    if (likely(is_flonum_obj(x))) dx = flonum_from_obj(x);
    else if (likely(is_fixnum_obj(x))) dx = (double)fixnum_from_obj(x);
    else failtype(x, "number");
    if (likely(is_flonum_obj(y))) dy = flonum_from_obj(y);
    else if (likely(is_fixnum_obj(y))) dy = (double)fixnum_from_obj(y);
    else failtype(y, "number");
    ac = flonum_obj(dx * dy);
  }
  gonexti(); 
}

define_instruction(div) {
  obj x = ac, y = spop();
  if (likely(are_fixnum_objs(x, y))) {
    long i; double d;
    if (unlikely(y == fixnum_obj(0))) fail("division by zero");
    if (fxifdv(get_fixnum_unchecked(x), get_fixnum_unchecked(y), &i, &d)) ac = fixnum_obj(i);
    else ac = flonum_obj(d);
  } else {
    double dx, dy;
    if (likely(is_flonum_obj(x))) dx = flonum_from_obj(x);
    else if (likely(is_fixnum_obj(x))) dx = (double)fixnum_from_obj(x);
    else failtype(x, "number");
    if (likely(is_flonum_obj(y))) dy = flonum_from_obj(y);
    else if (likely(is_fixnum_obj(y))) dy = (double)fixnum_from_obj(y);
    else failtype(y, "number");
    ac = flonum_obj(dx / dy);
  }
  gonexti(); 
}

define_instruction(quo) {
  obj x = ac, y = spop();
  if (likely(are_fixnum_objs(x, y))) {
    if (unlikely(y == fixnum_obj(0))) fail("division by zero");
    ac = fixnum_obj(fxquo(get_fixnum_unchecked(x), get_fixnum_unchecked(y)));
  } else {
    double dx, dy;
    if (likely(is_flonum_obj(x))) dx = flonum_from_obj(x);
    else if (likely(is_fixnum_obj(x))) dx = (double)fixnum_from_obj(x);
    else failtype(x, "number");
    if (likely(is_flonum_obj(y))) dy = flonum_from_obj(y);
    else if (likely(is_fixnum_obj(y))) dy = (double)fixnum_from_obj(y);
    else failtype(y, "number");
    ac = flonum_obj(flquo(dx, dy));
  }
  gonexti(); 
}

define_instruction(rem) {
  obj x = ac, y = spop();
  if (likely(are_fixnum_objs(x, y))) {
    if (unlikely(y == fixnum_obj(0))) fail("division by zero");
    ac = fixnum_obj(fxrem(get_fixnum_unchecked(x), get_fixnum_unchecked(y)));
  } else {
    double dx, dy;
    if (likely(is_flonum_obj(x))) dx = flonum_from_obj(x);
    else if (likely(is_fixnum_obj(x))) dx = (double)fixnum_from_obj(x);
    else failtype(x, "number");
    if (likely(is_flonum_obj(y))) dy = flonum_from_obj(y);
    else if (likely(is_fixnum_obj(y))) dy = (double)fixnum_from_obj(y);
    else failtype(y, "number");
    ac = flonum_obj(flrem(dx, dy));
  }
  gonexti(); 
}

define_instruction(mqu) {
  obj x = ac, y = spop();
  if (likely(are_fixnum_objs(x, y))) {
    if (unlikely(y == fixnum_obj(0))) fail("division by zero");
    ac = fixnum_obj(fxmqu(get_fixnum_unchecked(x), get_fixnum_unchecked(y)));
  } else {
    double dx, dy;
    if (likely(is_flonum_obj(x))) dx = flonum_from_obj(x);
    else if (likely(is_fixnum_obj(x))) dx = (double)fixnum_from_obj(x);
    else failtype(x, "number");
    if (likely(is_flonum_obj(y))) dy = flonum_from_obj(y);
    else if (likely(is_fixnum_obj(y))) dy = (double)fixnum_from_obj(y);
    else failtype(y, "number");
    ac = flonum_obj(flmqu(dx, dy));
  }
  gonexti(); 
}

define_instruction(mlo) {
  obj x = ac, y = spop();
  if (likely(are_fixnum_objs(x, y))) {
    if (unlikely(y == fixnum_obj(0))) fail("division by zero");
    ac = fixnum_obj(fxmlo(get_fixnum_unchecked(x), get_fixnum_unchecked(y)));
  } else {
    double dx, dy;
    if (likely(is_flonum_obj(x))) dx = flonum_from_obj(x);
    else if (likely(is_fixnum_obj(x))) dx = (double)fixnum_from_obj(x);
    else failtype(x, "number");
    if (likely(is_flonum_obj(y))) dy = flonum_from_obj(y);
    else if (likely(is_fixnum_obj(y))) dy = (double)fixnum_from_obj(y);
    else failtype(y, "number");
    ac = flonum_obj(flmlo(dx, dy));
  }
  gonexti(); 
}


define_instruction(lt) {
  obj x = ac, y = spop();
  if (likely(are_fixnum_objs(x, y))) {
    ac = bool_obj(get_fixnum_unchecked(x) < get_fixnum_unchecked(y));
  } else {
    double dx, dy;
    if (likely(is_flonum_obj(x))) dx = flonum_from_obj(x);
    else if (likely(is_fixnum_obj(x))) dx = (double)fixnum_from_obj(x);
    else failtype(x, "number");
    if (likely(is_flonum_obj(y))) dy = flonum_from_obj(y);
    else if (likely(is_fixnum_obj(y))) dy = (double)fixnum_from_obj(y);
    else failtype(y, "number");
    ac = bool_obj(dx < dy);
  }
  gonexti(); 
}

define_instruction(gt) {
  obj x = ac, y = spop();
  if (likely(are_fixnum_objs(x, y))) {
    ac = bool_obj(get_fixnum_unchecked(x) > get_fixnum_unchecked(y));
  } else {
    double dx, dy;
    if (likely(is_flonum_obj(x))) dx = flonum_from_obj(x);
    else if (likely(is_fixnum_obj(x))) dx = (double)fixnum_from_obj(x);
    else failtype(x, "number");
    if (likely(is_flonum_obj(y))) dy = flonum_from_obj(y);
    else if (likely(is_fixnum_obj(y))) dy = (double)fixnum_from_obj(y);
    else failtype(y, "number");
    ac = bool_obj(dx > dy);
  }
  gonexti(); 
}

define_instruction(le) {
  obj x = ac, y = spop();
  if (likely(are_fixnum_objs(x, y))) {
    ac = bool_obj(get_fixnum_unchecked(x) <= get_fixnum_unchecked(y));
  } else {
    double dx, dy;
    if (likely(is_flonum_obj(x))) dx = flonum_from_obj(x);
    else if (likely(is_fixnum_obj(x))) dx = (double)fixnum_from_obj(x);
    else failtype(x, "number");
    if (likely(is_flonum_obj(y))) dy = flonum_from_obj(y);
    else if (likely(is_fixnum_obj(y))) dy = (double)fixnum_from_obj(y);
    else failtype(y, "number");
    ac = bool_obj(dx <= dy);
  }
  gonexti(); 
}

define_instruction(ge) {
  obj x = ac, y = spop();
  if (likely(are_fixnum_objs(x, y))) {
    ac = bool_obj(get_fixnum_unchecked(x) >= get_fixnum_unchecked(y));
  } else {
    double dx, dy;
    if (likely(is_flonum_obj(x))) dx = flonum_from_obj(x);
    else if (likely(is_fixnum_obj(x))) dx = (double)fixnum_from_obj(x);
    else failtype(x, "number");
    if (likely(is_flonum_obj(y))) dy = flonum_from_obj(y);
    else if (likely(is_fixnum_obj(y))) dy = (double)fixnum_from_obj(y);
    else failtype(y, "number");
    ac = bool_obj(dx >= dy);
  }
  gonexti(); 
}

define_instruction(eq) {
  obj x = ac, y = spop();
  if (x == y) ac = bool_obj(1);
  else if (is_flonum_obj(x) || is_flonum_obj(y)) {
    double dx, dy;
    if (likely(is_flonum_obj(x))) dx = flonum_from_obj(x);
    else if (likely(is_fixnum_obj(x))) dx = (double)fixnum_from_obj(x);
    else failtype(x, "number");
    if (likely(is_flonum_obj(y))) dy = flonum_from_obj(y);
    else if (likely(is_fixnum_obj(y))) dy = (double)fixnum_from_obj(y);
    else failtype(y, "number");
    ac = bool_obj(dx == dy);
  } else ac = bool_obj(0);
  gonexti(); 
}

define_instruction(ne) {
  obj x = ac, y = spop();
  if (x == y) ac = bool_obj(0);
  else if (is_flonum_obj(x) || is_flonum_obj(y)) {
    double dx, dy;
    if (likely(is_flonum_obj(x))) dx = flonum_from_obj(x);
    else if (likely(is_fixnum_obj(x))) dx = (double)fixnum_from_obj(x);
    else failtype(x, "number");
    if (likely(is_flonum_obj(y))) dy = flonum_from_obj(y);
    else if (likely(is_fixnum_obj(y))) dy = (double)fixnum_from_obj(y);
    else failtype(y, "number");
    ac = bool_obj(dx != dy);
  } else ac = bool_obj(1);
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
    ac = dx < dy ? flonum_obj(dx) : flonum_obj(dy);
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
    ac = dx > dy ? flonum_obj(dx) : flonum_obj(dy);
  }
  gonexti(); 
}

define_instruction(gcd) {
  obj x = ac, y = spop();
  if (likely(are_fixnum_objs(x, y))) {
    ac = fixnum_obj(fxgcd(fixnum_from_obj(x), fixnum_from_obj(y)));
  } else {
    double dx, dy;
    if (likely(is_flonum_obj(x))) dx = flonum_from_obj(x);
    else if (likely(is_fixnum_obj(x))) dx = (double)fixnum_from_obj(x);
    else failtype(x, "number");
    if (likely(is_flonum_obj(y))) dy = flonum_from_obj(y);
    else if (likely(is_fixnum_obj(y))) dy = (double)fixnum_from_obj(y);
    else failtype(y, "number");
    ac = flonum_obj(flgcd(dx, dy));
  }
  gonexti(); 
}

define_instruction(pow) {
  obj x = ac, y = spop();
  if (likely(are_fixnum_objs(x, y))) {
    /* fixme: this will either overflow, or fail on negative y */
    ac = fixnum_obj(fxpow(fixnum_from_obj(x), fixnum_from_obj(y)));
  } else {
    double dx, dy;
    if (likely(is_flonum_obj(x))) dx = flonum_from_obj(x);
    else if (likely(is_fixnum_obj(x))) dx = (double)fixnum_from_obj(x);
    else failtype(x, "number");
    if (likely(is_flonum_obj(y))) dy = flonum_from_obj(y);
    else if (likely(is_fixnum_obj(y))) dy = (double)fixnum_from_obj(y);
    else failtype(y, "number");
    ac = flonum_obj(pow(dx, dy));
  }
  gonexti(); 
}

define_instruction(sqrt) {
  if (likely(is_flonum_obj(ac))) {
    ac = flonum_obj(sqrt(flonum_from_obj(ac)));
  } else if (likely(is_fixnum_obj(ac))) {
    long x = fixnum_from_obj(ac), y;
    if (x < 0) ac = flonum_obj((HUGE_VAL - HUGE_VAL));   
    else if (y = fxsqrt(x), y*y == x) ac = fixnum_obj(y);
    else ac = flonum_obj(sqrt((double)x));
  } else failactype("number");
  gonexti(); 
}

define_instruction(exp) {
  double x;
  if (unlikely(is_fixnum_obj(ac))) x = (double)fixnum_from_obj(ac);
  else if (likely(is_flonum_obj(ac))) x = flonum_from_obj(ac);
  else failactype("number");
  ac = flonum_obj(exp(x));
  gonexti(); 
}

define_instruction(log) {
  double x; obj y = spop();
  if (unlikely(is_fixnum_obj(ac))) x = (double)fixnum_from_obj(ac);
  else if (likely(is_flonum_obj(ac))) x = flonum_from_obj(ac);
  else failactype("number");
  if (likely(!y)) {
    ac = flonum_obj(log(x));
  } else if (likely(y == fixnum_obj(10))) {
    ac = flonum_obj(log10(x));
  } else {
    double b; 
    if (unlikely(is_fixnum_obj(y))) b = (double)fixnum_from_obj(y);
    else if (likely(is_flonum_obj(y))) b = flonum_from_obj(y);
    else failtype(y, "number");
    if (likely(b == 10.0)) ac = flonum_obj(log10(x));
    else ac = flonum_obj(log(x)/log(b));
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
  ac = flonum_obj(sin(x));
  gonexti(); 
}

define_instruction(cos) {
  double x;
  if (unlikely(is_fixnum_obj(ac))) {
    x = (double)fixnum_from_obj(ac);
  } else if (likely(is_flonum_obj(ac))) {
    x = flonum_from_obj(ac);
  } else failactype("number");
  ac = flonum_obj(cos(x));
  gonexti(); 
}

define_instruction(tan) {
  double x;
  if (unlikely(is_fixnum_obj(ac))) {
    x = (double)fixnum_from_obj(ac);
  } else if (likely(is_flonum_obj(ac))) {
    x = flonum_from_obj(ac);
  } else failactype("number");
  ac = flonum_obj(tan(x));
  gonexti(); 
}

define_instruction(asin) {
  double x;
  if (unlikely(is_fixnum_obj(ac))) {
    x = (double)fixnum_from_obj(ac);
  } else if (likely(is_flonum_obj(ac))) {
    x = flonum_from_obj(ac);
  } else failactype("number");
  ac = flonum_obj(asin(x));
  gonexti(); 
}

define_instruction(acos) {
  double x;
  if (unlikely(is_fixnum_obj(ac))) {
    x = (double)fixnum_from_obj(ac);
  } else if (likely(is_flonum_obj(ac))) {
    x = flonum_from_obj(ac);
  } else failactype("number");
  ac = flonum_obj(acos(x));
  gonexti(); 
}

define_instruction(atan) {
  double x; obj y = spop();
  if (unlikely(is_fixnum_obj(ac))) x = (double)fixnum_from_obj(ac);
  else if (likely(is_flonum_obj(ac))) x = flonum_from_obj(ac);
  else failactype("number");
  if (likely(!y)) {
    ac = flonum_obj(atan(x));
  } else {
    double b; 
    if (unlikely(is_fixnum_obj(y))) b = (double)fixnum_from_obj(y);
    else if (likely(is_flonum_obj(y))) b = flonum_from_obj(y);
    else failtype(y, "number");
    ac = flonum_obj(atan2(x, b));
  }
  gonexti(); 
}

define_instruction(neg) {
  if (likely(is_fixnum_obj(ac))) {
    ac = fixnum_obj(-fixnum_from_obj(ac));
  } else if (likely(is_flonum_obj(ac))) {
    ac = flonum_obj(-flonum_from_obj(ac));
  } else failactype("number");
  gonexti(); 
}

define_instruction(abs) {
  if (likely(is_fixnum_obj(ac))) {
    ac = fixnum_obj(fxabs(fixnum_from_obj(ac)));
  } else if (likely(is_flonum_obj(ac))) {
    ac = flonum_obj(fabs(flonum_from_obj(ac)));
  } else failactype("number");
  gonexti(); 
}

define_instruction(floor) {
  if (likely(is_flonum_obj(ac))) {
    ac = flonum_obj(floor(flonum_from_obj(ac)));
  } else if (unlikely(!is_fixnum_obj(ac))) {
    failactype("number");
  }
  gonexti(); 
}

define_instruction(ceil) {
  if (likely(is_flonum_obj(ac))) {
    ac = flonum_obj(ceil(flonum_from_obj(ac)));
  } else if (unlikely(!is_fixnum_obj(ac))) {
    failactype("number");
  }
  gonexti(); 
}

define_instruction(trunc) {
  if (likely(is_flonum_obj(ac))) {
    double x = flonum_from_obj(ac);
    double i; modf(x,  &i);
    ac = flonum_obj(i);
  } else if (unlikely(!is_fixnum_obj(ac))) {
    failactype("number");
  }
  gonexti(); 
}

define_instruction(round) {
  if (likely(is_flonum_obj(ac))) {
    ac = flonum_obj(flround(flonum_from_obj(ac)));
  } else if (unlikely(!is_fixnum_obj(ac))) {
    failactype("number");
  }
  gonexti(); 
}


define_instruction(nump) {
  ac = bool_obj(is_fixnum_obj(ac) || is_flonum_obj(ac));
  gonexti(); 
}

define_instruction(fixp) {
  ac = bool_obj(is_fixnum_obj(ac));
  gonexti(); 
}

define_instruction(flop) {
  ac = bool_obj(is_flonum_obj(ac));
  gonexti(); 
}

define_instruction(intp) {
  if (likely(is_fixnum_obj(ac))) {
    ac = bool_obj(1);
  } else { /* accepts any object! */
    ac = bool_obj(is_flonum_obj(ac) && flisint(flonum_from_obj(ac)));
  }
  gonexti(); 
}

define_instruction(nanp) {
  if (unlikely(is_fixnum_obj(ac))) {
    ac = bool_obj(0);
  } else if (likely(is_flonum_obj(ac))) {
    double f = flonum_from_obj(ac);
    ac = bool_obj(f != f);
  } else failactype("number");
  gonexti(); 
}

define_instruction(finp) {
  if (unlikely(is_fixnum_obj(ac))) {
    ac = bool_obj(1);
  } else if (likely(is_flonum_obj(ac))) {
    double f = flonum_from_obj(ac);
    ac = bool_obj(f > -HUGE_VAL && f < HUGE_VAL);
  } else failactype("number");
  gonexti(); 
}

define_instruction(infp) {
  if (unlikely(is_fixnum_obj(ac))) {
    ac = bool_obj(0);
  } else if (likely(is_flonum_obj(ac))) {
    double f = flonum_from_obj(ac);
    ac = bool_obj(f <= -HUGE_VAL || f >= HUGE_VAL);
  } else failactype("number");
  gonexti(); 
}

define_instruction(evnp) {
  if (likely(is_fixnum_obj(ac))) {
    ac = bool_obj((fixnum_from_obj(ac) & 1) == 0);
  } else if (likely(is_flonum_obj(ac))) {
    double f = flonum_from_obj(ac);
    ac = bool_obj(flisint(f / 2.0));
  } else failactype("number");
  gonexti(); 
}

define_instruction(oddp) {
  if (likely(is_fixnum_obj(ac))) {
    ac = bool_obj((fixnum_from_obj(ac) & 1) != 0);
  } else if (likely(is_flonum_obj(ac))) {
    double f = flonum_from_obj(ac);
    ac = bool_obj(flisint((f + 1.0) / 2.0));
  } else failactype("number");
  gonexti(); 
}

define_instruction(ntoi) {
  if (likely(is_flonum_obj(ac))) ac = fixnum_obj(fxflo(flonum_from_obj(ac)));
  else if (likely(is_fixnum_obj(ac))) /* keep ac as-is */ ;
  else failactype("number");
  gonexti(); 
}

define_instruction(ntoj) {
  if (likely(is_fixnum_obj(ac))) ac = flonum_obj((flonum_t)fixnum_from_obj(ac));
  else if (likely(is_flonum_obj(ac))) /* keep ac as-is */ ;
  else failactype("number");
  gonexti(); 
}


define_instruction(lcat) {
  obj t, l, *p, *d; int c;
  for (l = ac, c = 0; ispair(l); l = cdr(l)) ++c;
  hp_reserve(pairbsz()*c);
  p = --sp; t = *p; /* pop & take addr */
  for (l = ac; ispair(l); l = cdr(l)) {
    *--hp = t; d = hp; *--hp = car(l);
    *p = hend_pair(); p = d;
  }
  ac = *sp;
  gonexti(); 
}  

define_instruction(ccmp) {
  obj x = ac, y = spop(); int cmp; ckc(x); ckc(y);
  cmp = char_from_obj(x) - char_from_obj(y);
  ac = fixnum_obj(cmp);
  gonexti(); 
}

define_instruction(ceq) {
  obj x = ac, y = spop(); ckc(x); ckc(y);
  ac = bool_obj(char_from_obj(x) == char_from_obj(y));
  gonexti(); 
}

define_instruction(clt) {
  obj x = ac, y = spop(); ckc(x); ckc(y);
  ac = bool_obj(char_from_obj(x) < char_from_obj(y));
  gonexti(); 
}

define_instruction(cgt) {
  obj x = ac, y = spop(); ckc(x); ckc(y);
  ac = bool_obj(char_from_obj(x) > char_from_obj(y));
  gonexti(); 
}

define_instruction(cle) {
  obj x = ac, y = spop(); ckc(x); ckc(y);
  ac = bool_obj(char_from_obj(x) <= char_from_obj(y));
  gonexti(); 
}

define_instruction(cge) {
  obj x = ac, y = spop(); ckc(x); ckc(y);
  ac = bool_obj(char_from_obj(x) >= char_from_obj(y));
  gonexti(); 
}

define_instruction(cicmp) {
  obj x = ac, y = spop(); int cmp; ckc(x); ckc(y);
  cmp = tolower(char_from_obj(x)) - tolower(char_from_obj(y));
  ac = fixnum_obj(cmp);
  gonexti(); 
}

define_instruction(cieq) {
  obj x = ac, y = spop(); ckc(x); ckc(y);
  ac = bool_obj(tolower(char_from_obj(x)) == tolower(char_from_obj(y)));
  gonexti(); 
}

define_instruction(cilt) {
  obj x = ac, y = spop(); ckc(x); ckc(y);
  ac = bool_obj(tolower(char_from_obj(x)) < tolower(char_from_obj(y)));
  gonexti(); 
}

define_instruction(cigt) {
  obj x = ac, y = spop(); ckc(x); ckc(y);
  ac = bool_obj(tolower(char_from_obj(x)) > tolower(char_from_obj(y)));
  gonexti(); 
}

define_instruction(cile) {
  obj x = ac, y = spop(); ckc(x); ckc(y);
  ac = bool_obj(tolower(char_from_obj(x)) <= tolower(char_from_obj(y)));
  gonexti(); 
}

define_instruction(cige) {
  obj x = ac, y = spop(); ckc(x); ckc(y);
  ac = bool_obj(tolower(char_from_obj(x)) >= tolower(char_from_obj(y)));
  gonexti(); 
}

define_instruction(cwsp) {
  ckc(ac);
  ac = bool_obj(isspace(char_from_obj(ac)));
  gonexti(); 
}

define_instruction(clcp) {
  ckc(ac);
  ac = bool_obj(islower(char_from_obj(ac)));
  gonexti(); 
}

define_instruction(cucp) {
  ckc(ac);
  ac = bool_obj(isupper(char_from_obj(ac)));
  gonexti(); 
}

define_instruction(calp) {
  ckc(ac);
  ac = bool_obj(isalpha(char_from_obj(ac)));
  gonexti(); 
}

define_instruction(cnup) {
  ckc(ac);
  ac = bool_obj(isdigit(char_from_obj(ac)));
  gonexti(); 
}

define_instruction(cupc) {
  ckc(ac);
  ac = char_obj(toupper(char_from_obj(ac)));
  gonexti(); 
}

define_instruction(cdnc) {
  ckc(ac);
  ac = char_obj(tolower(char_from_obj(ac)));
  gonexti(); 
}

define_instruction(cflc) {
  ckc(ac);
  ac = char_obj(tolower(char_from_obj(ac))); /* stub */
  gonexti(); 
}

define_instruction(cdgv) {
  int ch; ckc(ac);
  ch = char_from_obj(ac);
  if (likely('0' <= ch && ch <= '9')) ac = fixnum_obj(ch - '0');  
  /* R7RS won't allow hex and any larger radix digits
  else if (likely('a' <= ch && ch <= 'z')) ac = fixnum_obj(10 + ch - 'a');  
  else if (likely('A' <= ch && ch <= 'Z')) ac = fixnum_obj(10 + ch - 'A'); */
  else ac = bool_obj(0);
  gonexti(); 
}


define_instruction(scmp) {
  obj x = ac, y = spop(); int cmp; cks(x); cks(y);
  cmp = strcmp(stringchars(x), stringchars(y));
  ac = fixnum_obj(cmp);
  gonexti(); 
}

define_instruction(seq) {
  obj x = ac, y = spop(); cks(x); cks(y);
  ac = bool_obj(strcmp(stringchars(x), stringchars(y)) == 0);
  gonexti(); 
}

define_instruction(slt) {
  obj x = ac, y = spop(); cks(x); cks(y);
  ac = bool_obj(strcmp(stringchars(x), stringchars(y)) < 0);
  gonexti(); 
}

define_instruction(sgt) {
  obj x = ac, y = spop(); cks(x); cks(y);
  ac = bool_obj(strcmp(stringchars(x), stringchars(y)) > 0);
  gonexti(); 
}

define_instruction(sle) {
  obj x = ac, y = spop(); cks(x); cks(y);
  ac = bool_obj(strcmp(stringchars(x), stringchars(y)) <= 0);
  gonexti(); 
}

define_instruction(sge) {
  obj x = ac, y = spop(); cks(x); cks(y);
  ac = bool_obj(strcmp(stringchars(x), stringchars(y)) >= 0);
  gonexti(); 
}

define_instruction(sicmp) {
  obj x = ac, y = spop(); int cmp; cks(x); cks(y);
  cmp = strcmp_ci(stringchars(x), stringchars(y));
  ac = fixnum_obj(cmp);
  gonexti(); 
}

define_instruction(sieq) {
  obj x = ac, y = spop(); cks(x); cks(y);
  ac = bool_obj(strcmp_ci(stringchars(x), stringchars(y)) == 0);
  gonexti(); 
}

define_instruction(silt) {
  obj x = ac, y = spop(); cks(x); cks(y);
  ac = bool_obj(strcmp_ci(stringchars(x), stringchars(y)) < 0);
  gonexti(); 
}

define_instruction(sigt) {
  obj x = ac, y = spop(); cks(x); cks(y);
  ac = bool_obj(strcmp_ci(stringchars(x), stringchars(y)) > 0);
  gonexti(); 
}

define_instruction(sile) {
  obj x = ac, y = spop(); cks(x); cks(y);
  ac = bool_obj(strcmp_ci(stringchars(x), stringchars(y)) <= 0);
  gonexti(); 
}

define_instruction(sige) {
  obj x = ac, y = spop(); cks(x); cks(y);
  ac = bool_obj(strcmp_ci(stringchars(x), stringchars(y)) >= 0);
  gonexti(); 
}


define_instruction(symp) {
  ac = bool_obj(issymbol(ac));
  gonexti();
}

define_instruction(boolp) {
  ac = bool_obj(is_bool_obj(ac));
  gonexti();
}

define_instruction(boxp) {
  ac = bool_obj(isbox(ac));
  gonexti();
}

define_instruction(funp) {
  ac = bool_obj(isvmclo(ac));
  gonexti();
}

define_instruction(ipp) {
  ac = bool_obj(isiport(ac));
  gonexti();
}

define_instruction(opp) {
  ac = bool_obj(isoport(ac));
  gonexti();
}

define_instruction(sip) {
  ac = iport_file_obj(stdin); /* TODO: keep in global var -- in r7rs it is a parameter */
  gonexti();
}

define_instruction(sop) {
  ac = oport_file_obj(stdout); /* TODO: keep in global var -- in r7rs it is a parameter */
  gonexti();
}

define_instruction(sep) {
  ac = oport_file_obj(stderr); /* TODO: keep in global var -- in r7rs it is a parameter */
  gonexti();
}

define_instruction(ipop) {
  cxtype_iport_t *vt; ckr(ac); 
  vt = iportvt(ac); assert(vt);
  ac = bool_obj(vt != (cxtype_iport_t *)IPORT_CLOSED_NTAG);
  gonexti();
}

define_instruction(opop) {
  cxtype_oport_t *vt; ckw(ac); 
  vt = oportvt(ac); assert(vt);
  ac = bool_obj(vt != (cxtype_oport_t *)OPORT_CLOSED_NTAG);
  gonexti();
}

define_instruction(otip) {
  FILE *fp = fopen(stringchars(ac), "r");
  if (fp == NULL) fail("can't open input file");
  ac = iport_file_obj(fp);
  gonexti();
}

define_instruction(otop) {
  FILE *fp = fopen(stringchars(ac), "w");
  if (fp == NULL) fail("can't open output file");
  ac = oport_file_obj(fp);
  gonexti();
}

define_instruction(ois) {
  int *d; cks(ac);
  d = dupstring(stringdata(ac));
  ac = iport_string_obj(sialloc(sdatachars(d), d));
  gonexti();
}

define_instruction(oos) {
  ac = oport_string_obj(newcb());
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

define_instruction(fop) {
  ckw(ac);
  oportflush(ac);
  gonexti();
}

define_instruction(gos) {
  cxtype_oport_t *vt; ckw(ac);
  vt = ckoportvt(ac);
  if (vt != (cxtype_oport_t *)OPORT_STRING_NTAG &&
      vt != (cxtype_oport_t *)OPORT_BYTEVECTOR_NTAG) {
    ac = eof_obj();
  } else {
    cbuf_t *pcb = oportdata(ac);
    ac = string_obj(newstring(cbdata(pcb)));
  }
  gonexti();
}


define_instruction(rdc) {
  int c; ckr(ac);
  c = iportgetc(ac);
  if (unlikely(c == EOF)) ac = eof_obj();
  else ac = char_obj(c);
  gonexti();
}

define_instruction(rdac) {
  int c; ckr(ac);
  c = iportpeekc(ac);
  if (unlikely(c == EOF)) ac = eof_obj();
  else ac = char_obj(c);
  gonexti();
}

define_instruction(rdcr) {
  ckr(ac);
  ac = bool_obj(1); /* no portable way to detect hanging? */
  gonexti();
}

define_instruction(eofp) {
  ac = bool_obj(iseof(ac));
  gonexti();
}

define_instruction(eof) {
  ac = eof_obj();
  gonexti();
}


define_instruction(wrc) {
  obj x = ac, y = spop(); ckc(x); ckw(y);
  oportputc(char_from_obj(x), y);
  ac = void_obj();
  gonexti();
}

define_instruction(wrs) {
  obj x = ac, y = spop(); cks(x); ckw(y);
  oportputs(stringchars(x), y);
  ac = void_obj();
  gonexti();
}

define_instruction(wrcd) {
  obj x = ac, y = spop(); ckw(y);
  oportputcircular(x, y, 1);
  ac = void_obj();
  gonexti();
}

define_instruction(wrcw) {
  obj x = ac, y = spop(); ckw(y);
  oportputcircular(x, y, 0);
  ac = void_obj();
  gonexti();
}

define_instruction(wrnl) {
  ckw(ac);
  oportputc('\n', ac);
  ac = void_obj();
  gonexti();
}

define_instruction(wrhw) {
  obj x = ac, y = spop(); ckw(y);
  oportputshared(x, y, 0);
  ac = void_obj();
  gonexti();
}

define_instruction(wriw) {
  obj x = ac, y = spop(); ckw(y);
  oportputsimple(x, y, 0);
  ac = void_obj();
  gonexti();
}

define_instruction(igp) {
  ac = bool_obj(isintegrable(ac));
  gonexti(); 
}

define_instruction(iglk) {
  struct intgtab_entry *pe; cky(ac);
  pe = lookup_integrable(getsymbol(ac));
  ac = pe ? mkintegrable(pe) : bool_obj(0);
  gonexti(); 
}

define_instruction(igty) {
  int it; ckg(ac);
  it = integrable_type(integrabledata(ac));
  ac = it ? char_obj(it) : bool_obj(0);
  gonexti(); 
}

define_instruction(iggl) {
  const char *igs; ckg(ac);
  igs = integrable_global(integrabledata(ac));
  ac = igs ? mksymbol(internsym((char*)igs)) : bool_obj(0);
  gonexti(); 
}

define_instruction(igco) {
  int n; const char *cs; ckg(ac); ckk(sref(0));
  n = fixnum_from_obj(spop());
  cs = integrable_code(integrabledata(ac), n);
  ac = cs ? string_obj(newstring((char*)cs)) : bool_obj(0);
  gonexti(); 
}

define_instruction(rdsx) {
  cks(ac); 
  unload_ac(); /* ac->ra (string) */
  unload_ip(); /* ip->rx */
  hp = rds_stox(r, sp, hp);
  reload_ac(); /* ra->ac (sexp or eof) */
  reload_ip(); /* rx->ip */
  if (ac == eof_obj()) fail("failed to read serialized s-expression");
  gonexti(); 
}

define_instruction(rdsc) {
  cks(ac); unload_ac(); /* ac->ra (string) */
  unload_ac(); /* ac->ra (string) */
  unload_ip(); /* ip->rx */
  hp = rds_stoc(r, sp, hp);
  reload_ac(); /* ra->ac (codevec or eof) */
  reload_ip(); /* rx->ip */
  if (ac == eof_obj()) fail("failed to read serialized code");
  gonexti(); 
}

define_instruction(litf) { ac = bool_obj(0); gonexti(); }  
define_instruction(litt) { ac = bool_obj(1); gonexti(); }  
define_instruction(litn) { ac = null_obj(); gonexti(); }  
define_instruction(pushlitf) { ac = bool_obj(0); spush(ac); gonexti(); }  
define_instruction(pushlitt) { ac = bool_obj(1); spush(ac); gonexti(); }  
define_instruction(pushlitn) { ac = null_obj(); spush(ac); gonexti(); }  

define_instruction(lit0) { ac = fixnum_obj(0); gonexti(); }  
define_instruction(lit1) { ac = fixnum_obj(1); gonexti(); }  
define_instruction(lit2) { ac = fixnum_obj(2); gonexti(); }  
define_instruction(lit3) { ac = fixnum_obj(3); gonexti(); }  
define_instruction(lit4) { ac = fixnum_obj(4); gonexti(); }  
define_instruction(lit5) { ac = fixnum_obj(5); gonexti(); }  
define_instruction(lit6) { ac = fixnum_obj(6); gonexti(); }  
define_instruction(lit7) { ac = fixnum_obj(7); gonexti(); }  
define_instruction(lit8) { ac = fixnum_obj(8); gonexti(); }  
define_instruction(lit9) { ac = fixnum_obj(9); gonexti(); }  
define_instruction(pushlit0) { ac = fixnum_obj(0); spush(ac); gonexti(); }  
define_instruction(pushlit1) { ac = fixnum_obj(1); spush(ac); gonexti(); }  
define_instruction(pushlit2) { ac = fixnum_obj(2); spush(ac); gonexti(); }  
define_instruction(pushlit3) { ac = fixnum_obj(3); spush(ac); gonexti(); }  
define_instruction(pushlit4) { ac = fixnum_obj(4); spush(ac); gonexti(); }  
define_instruction(pushlit5) { ac = fixnum_obj(5); spush(ac); gonexti(); }  
define_instruction(pushlit6) { ac = fixnum_obj(6); spush(ac); gonexti(); }  
define_instruction(pushlit7) { ac = fixnum_obj(7); spush(ac); gonexti(); }  
define_instruction(pushlit8) { ac = fixnum_obj(8); spush(ac); gonexti(); }  
define_instruction(pushlit9) { ac = fixnum_obj(9); spush(ac); gonexti(); }  

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
  ckx(ac); rd = ac; rx = fixnum_obj(0); ac = fixnum_obj(0); 
  callsubi(); 
}
define_instruction(call1) { 
  ckx(ac); rd = ac; rx = fixnum_obj(0); ac = fixnum_obj(1); 
  callsubi(); 
}
define_instruction(call2) { 
  ckx(ac); rd = ac; rx = fixnum_obj(0); ac = fixnum_obj(2); 
  callsubi(); 
}
define_instruction(call3) { 
  ckx(ac); rd = ac; rx = fixnum_obj(0); ac = fixnum_obj(3); 
  callsubi(); 
}
define_instruction(call4) { 
  ckx(ac); rd = ac; rx = fixnum_obj(0); ac = fixnum_obj(4); 
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
  if (unlikely(ac != fixnum_obj(0))) fail("argument count error on entry");
  gonexti();
}

define_instruction(atest1) {
  if (unlikely(ac != fixnum_obj(1))) fail("argument count error on entry");
  gonexti();
}

define_instruction(atest2) {
  if (unlikely(ac != fixnum_obj(2))) fail("argument count error on entry");
  gonexti();
}

define_instruction(atest3) {
  if (unlikely(ac != fixnum_obj(3))) fail("argument count error on entry");
  gonexti();
}

define_instruction(atest4) {
  if (unlikely(ac != fixnum_obj(4))) fail("argument count error on entry");
  gonexti();
}

define_instruction(scall1) {
  int m = 1, n = fixnum_from_obj(*ip++);
  ckx(ac); rd = ac; rx = fixnum_obj(0); 
  ac = fixnum_obj(n); /* argc */
  memmove((void*)(sp-n-m), (void*)(sp-n), (size_t)n*sizeof(obj));
  sdrop(m); callsubi();
}

define_instruction(scall10) {
  ckx(ac); rd = ac; rx = fixnum_obj(0); ac = fixnum_obj(0);
  sdrop(1); callsubi();
}

define_instruction(scall11) {
  ckx(ac); rd = ac; rx = fixnum_obj(0); ac = fixnum_obj(1);
  sref(1) = sref(0);
  sdrop(1); callsubi();
}

define_instruction(scall12) {
  ckx(ac); rd = ac; rx = fixnum_obj(0); ac = fixnum_obj(2);
  sref(2) = sref(1); sref(1) = sref(0);
  sdrop(1); callsubi();
}

define_instruction(scall13) {
  ckx(ac); rd = ac; rx = fixnum_obj(0); ac = fixnum_obj(3);
  sref(3) = sref(2); sref(2) = sref(1); sref(1) = sref(0);
  sdrop(1); callsubi();
}

define_instruction(scall14) {
  ckx(ac); rd = ac; rx = fixnum_obj(0); ac = fixnum_obj(4);
  sref(4) = sref(3); sref(3) = sref(2); sref(2) = sref(1); sref(1) = sref(0);
  sdrop(1); callsubi();
}

define_instruction(scall2) {
  int m = 2, n = fixnum_from_obj(*ip++);
  ckx(ac); rd = ac; rx = fixnum_obj(0); 
  ac = fixnum_obj(n);
  memmove((void*)(sp-n-m), (void*)(sp-n), (size_t)n*sizeof(obj));
  sdrop(m); callsubi();
}

define_instruction(scall20) {
  ckx(ac); rd = ac; rx = fixnum_obj(0); ac = fixnum_obj(0);
  sdrop(2); callsubi();
}

define_instruction(scall21) {
  ckx(ac); rd = ac; rx = fixnum_obj(0); ac = fixnum_obj(1);
  sref(2) = sref(0);
  sdrop(2); callsubi();
}

define_instruction(scall22) {
  ckx(ac); rd = ac; rx = fixnum_obj(0); ac = fixnum_obj(2);
  sref(3) = sref(1); sref(2) = sref(0);
  sdrop(2); callsubi();
}

define_instruction(scall23) {
  ckx(ac); rd = ac; rx = fixnum_obj(0); ac = fixnum_obj(3);
  sref(4) = sref(2); sref(3) = sref(1); sref(2) = sref(0);
  sdrop(2); callsubi();
}

define_instruction(scall24) {
  ckx(ac); rd = ac; rx = fixnum_obj(0); ac = fixnum_obj(4);
  sref(5) = sref(3); sref(4) = sref(2); sref(3) = sref(1); sref(2) = sref(0);
  sdrop(2); callsubi();
}

define_instruction(scall3) {
  int m = 3, n = fixnum_from_obj(*ip++);
  ckx(ac); rd = ac; rx = fixnum_obj(0); 
  ac = fixnum_obj(n);
  memmove((void*)(sp-n-m), (void*)(sp-n), (size_t)n*sizeof(obj));
  sdrop(m); callsubi();
}

define_instruction(scall30) {
  ckx(ac); rd = ac; rx = fixnum_obj(0); ac = fixnum_obj(0);
  sdrop(3); callsubi();
}

define_instruction(scall31) {
  ckx(ac); rd = ac; rx = fixnum_obj(0); ac = fixnum_obj(1);
  sref(3) = sref(0);
  sdrop(3); callsubi();
}

define_instruction(scall32) {
  ckx(ac); rd = ac; rx = fixnum_obj(0); ac = fixnum_obj(2);
  sref(4) = sref(1); sref(3) = sref(0);
  sdrop(3); callsubi();
}

define_instruction(scall33) {
  ckx(ac); rd = ac; rx = fixnum_obj(0); ac = fixnum_obj(3);
  sref(5) = sref(2); sref(4) = sref(1); sref(3) = sref(0);
  sdrop(3); callsubi();
}

define_instruction(scall34) {
  ckx(ac); rd = ac; rx = fixnum_obj(0); ac = fixnum_obj(4);
  sref(6) = sref(3); sref(5) = sref(2); sref(4) = sref(1); sref(3) = sref(0);
  sdrop(3); callsubi();
}

define_instruction(scall4) {
  int m = 4, n = fixnum_from_obj(*ip++);
  ckx(ac); rd = ac; rx = fixnum_obj(0); 
  ac = fixnum_obj(n);
  memmove((void*)(sp-n-m), (void*)(sp-n), (size_t)n*sizeof(obj));
  sdrop(m); callsubi();
}

define_instruction(scall40) {
  ckx(ac); rd = ac; rx = fixnum_obj(0); ac = fixnum_obj(0);
  sdrop(4); callsubi();
}

define_instruction(scall41) {
  ckx(ac); rd = ac; rx = fixnum_obj(0); ac = fixnum_obj(1);
  sref(4) = sref(0);
  sdrop(4); callsubi();
}

define_instruction(scall42) {
  ckx(ac); rd = ac; rx = fixnum_obj(0); ac = fixnum_obj(2);
  sref(5) = sref(1); sref(4) = sref(0);
  sdrop(4); callsubi();
}

define_instruction(scall43) {
  ckx(ac); rd = ac; rx = fixnum_obj(0); ac = fixnum_obj(3);
  sref(6) = sref(2); sref(5) = sref(1); sref(4) = sref(0);
  sdrop(4); callsubi();
}

define_instruction(scall44) {
  ckx(ac); rd = ac; rx = fixnum_obj(0); ac = fixnum_obj(4);
  sref(7) = sref(3); sref(6) = sref(2); sref(5) = sref(1); sref(4) = sref(0);
  sdrop(4); callsubi();
}

define_instruction(brnotlt) {
  obj x = ac, y = spop();
  if (likely(are_fixnum_objs(x, y))) {
    int dx = fixnum_from_obj(*ip++);
    ac = bool_obj(get_fixnum_unchecked(x) < get_fixnum_unchecked(y));
    ip = ac ? ip : ip + dx;
    gonexti(); 
  } else {
    int dx = fixnum_from_obj(*ip++);
    double fx, fy;
    if (likely(is_flonum_obj(x))) fx = flonum_from_obj(x);
    else { cki(x); fx = (double)get_fixnum_unchecked(x); }
    if (likely(is_flonum_obj(y))) fy = flonum_from_obj(y);
    else { cki(y); fy = (double)get_fixnum_unchecked(y); }
    ac = bool_obj(fx < fy);
    ip = ac ? ip : ip + dx;
    gonexti(); 
  }
}

define_instruction(pushsub) {
  obj x = ac, y = spop();
  if (likely(are_fixnum_objs(x, y))) {
    ac = fixnum_obj(fxsub(get_fixnum_unchecked(x), get_fixnum_unchecked(y)));
  } else {
    double fx, fy;
    if (likely(is_flonum_obj(x))) fx = flonum_from_obj(x);
    else { cki(x); fx = (double)get_fixnum_unchecked(x); }
    if (likely(is_flonum_obj(y))) fy = flonum_from_obj(y);
    else { cki(y); fy = (double)get_fixnum_unchecked(y); }
    ac = flonum_obj(fx - fy);
  }
  spush(ac);
  gonexti(); 
}

define_instruction(fexis) {
  FILE *f; cks(ac);
  f = fopen(stringchars(ac), "r"); /* todo: pile #ifdefs here */
  if (f != NULL) fclose(f);
  ac = bool_obj(f != NULL);
  gonexti(); 
}

define_instruction(frem) {
  int res; cks(ac);
  res = remove(stringchars(ac));
  ac = bool_obj(res == 0);
  gonexti(); 
}

define_instruction(fren) {
  int res; cks(ac); cks(sref(0));
  res = rename(stringchars(ac), stringchars(sref(0)));
  spop();
  ac = bool_obj(res == 0);
  gonexti(); 
}

define_instruction(getenv) {
  char *v; cks(ac);
  v = getenv(stringchars(ac));
  if (v) ac = string_obj(newstring(v));
  else ac = bool_obj(0);
  gonexti(); 
}

define_instruction(clock) {
  double d = (double)clock();
  ac = flonum_obj(d);
  gonexti(); 
}

define_instruction(clops) {
  double d = (double)CLOCKS_PER_SEC;
  ac = flonum_obj(d);
  gonexti(); 
}

define_instruction(cursec) {
  double d = difftime(time(NULL), 0) + 37.0;
  ac = flonum_obj(d);
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

static int rds_byte(obj port)
{
  char buf[3]; int b;
  buf[0] = iportgetc(port); buf[1] = iportgetc(port); buf[2] = 0;
  b = (int)strtoul(buf, NULL, 16);
  return b;
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
      hreserve(pairbsz(), sp-r);
      *--hp = sref(0);  
      *--hp = sref(1);  
      ra = hend_pair();
      sdrop(3);      
    } break; 
    case 'l': {
      size_t n = rds_size(port), i;
      spush(port); /* sref(0)=port */
      for (i = 0; i < n; ++i) {
        ra = sref((int)i); hp = rds_elt(r, sp, hp); 
        if (iseof(ra)) { sdrop(i+1); return hp; } else spush(ra);
      }
      hreserve(pairbsz()*n, sp-r);
      ra = mknull();
      for (i = 0; i < n; ++i) {
        *--hp = ra;  
        *--hp = sref((int)i);  
        ra = hend_pair();
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
      hreserve(vecbsz(n), sp-r);
      hp -= n; memcpy(hp, sp-n, n*sizeof(obj));      
      ra = hend_vec(n);
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
    } break;
    case 'b': {
      cbuf_t *pcb = newcb();
      size_t n = rds_size(port), i;
      for (i = 0; i < n; ++i) {
        int x = rds_byte(port);
        cbputc(x, pcb);
      }
      ra = hpushu8v(sp-r, newbytevector((unsigned char *)cbdata(pcb), (int)cblen(pcb)));
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
  obj p = bool_obj(0);
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
  hreserve(vecbsz(n), sp-r);
  for (l = ra, i = 0; i < n; ++i, l = cdr(l)) *--hp = car(l);
  ra = hend_vec(n);
  return hp;
}

/* protects registers from r to sp, in: ra=code, out: ra=closure */
obj *close0(obj *r, obj *sp, obj *hp)
{
  hreserve(vmclobsz(1), sp-r);
  *--hp = ra;
  ra = hend_vmclo(1);
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
      hreserve(boxbsz()*1+pairbsz()*2, sp-r);
      *--hp = ra;
      box = hend_box();
      *--hp = box; *--hp = ra;
      ra = hend_pair();
      *--hp = cx__2Aglobals_2A; *--hp = ra;
      cx__2Aglobals_2A = hend_pair();
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
    hreserve(pairbsz(), sp-r);
    *--hp = sref(0); *--hp = glue(cx_ins_2D, halt);  
    ra = hend_pair();
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
        hreserve(pairbsz(), sp-r);
        *--hp = sref(0); *--hp = pbr->g;  
        sref(0) = hend_pair();
        goto more;
      } break;
      case 1: {
        ra = sref(1); hp = rds_arg(r, sp, hp);
        if (iseof(ra)) goto out;
        hreserve(pairbsz()*2, sp-r);
        *--hp = sref(0); *--hp = pbr->g;  
        sref(0) = hend_pair();
        *--hp = sref(0); *--hp = ra;  
        sref(0) = hend_pair();
        goto more;
      } break;
      case 2: {
        ra = sref(1); hp = rds_arg(r, sp, hp);
        if (iseof(ra)) goto out;
        hreserve(pairbsz()*2, sp-r);
        *--hp = sref(0); *--hp = pbr->g;  
        sref(0) = hend_pair();
        *--hp = sref(0); *--hp = ra;  
        sref(0) = hend_pair();
        ra = sref(1); hp = rds_arg(r, sp, hp);
        if (iseof(ra)) goto out;
        hreserve(pairbsz(), sp-r);
        *--hp = sref(0); *--hp = ra;  
        sref(0) = hend_pair();
        goto more;
      } break;
      case 'f': case 't': case 'n': {
        hreserve(pairbsz()*2, sp-r);
        *--hp = sref(0); *--hp = pbr->g;  
        sref(0) = hend_pair();
        *--hp = sref(0); *--hp = (pbr->etyp == 'n' ? mknull() : obj_from_bool(pbr->etyp == 't'));
        sref(0) = hend_pair();
        goto more;
      } break;
      case 'g': { /* gref/gset */
        ra = sref(1); hp = rds_arg(r, sp, hp);
        if (iseof(ra)) goto out;
        hp = rds_global_loc(r, sp, hp); /* ra => ra */
        if (iseof(ra)) goto out;
        hreserve(pairbsz()*2, sp-r); 
        *--hp = sref(0); *--hp = pbr->g;  
        sref(0) = hend_pair();
        *--hp = sref(0); *--hp = ra;
        sref(0) = hend_pair();
        goto more;
      } break;
      case 'a': { /* andbo */
        hreserve(pairbsz(), sp-r);
        *--hp = sref(0); *--hp = pbr->g;  
        sref(0) = hend_pair();
        c = iportpeekc(sref(1));
        if (c == EOF || c == '}') { ra = mkeof(); goto out; }
        pbr = rds_prefix(sref(1));
        if (pbr->g == 0 || pbr->etyp != 0) { ra = mkeof(); goto out; }
        hreserve(pairbsz(), sp-r);
        *--hp = sref(0); *--hp = pbr->g;  
        sref(0) = hend_pair();
        goto more;
      } break;
      case 's': { /* save */
        fixnum_t n;
        ra = sref(1); hp = rds_block(r, sp, hp);
        if (iseof(ra)) goto out;
        n = length(ra);
        hreserve(pairbsz()*2, sp-r); 
        *--hp = sref(0); *--hp = pbr->g;  
        sref(0) = hend_pair();
        *--hp = sref(0); *--hp = obj_from_fixnum(n);
        sref(0) = hend_pair();
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
        hreserve(pairbsz()*3, sp-r); 
        *--hp = sref(0); *--hp = pbr->g;  
        sref(0) = hend_pair();
        *--hp = sref(0); *--hp = obj_from_fixnum(n);
        sref(0) = hend_pair();
        *--hp = sref(0); *--hp = ra;  
        sref(0) = hend_pair();
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
          hreserve(pairbsz()*4, sp-r); 
          bseq1 = spop(); bseq2 = ra;
          n = length(bseq1);
          *--hp = sref(0); *--hp = pbr->g;  
          sref(0) = hend_pair();
          *--hp = sref(0); *--hp = obj_from_fixnum(n+2);
          sref(0) = hend_pair();
          if (n > 0) {
            obj lp = lastpair(bseq1); assert(ispair(lp));
            cdr(lp) = sref(0); sref(0) = bseq1;
          }
          *--hp = sref(0); *--hp = glue(cx_ins_2D, br);
          sref(0) = hend_pair();
          n = length(bseq2);
          *--hp = sref(0); *--hp = obj_from_fixnum(n);
          sref(0) = hend_pair();
          if (n > 0) {
            obj lp = lastpair(bseq2); assert(ispair(lp));
            cdr(lp) = sref(0); sref(0) = bseq2;
          }
        } else { /* regular 1-arm branch */
          n = length(ra);
          hreserve(pairbsz()*2, sp-r); 
          *--hp = sref(0); *--hp = pbr->g;  
          sref(0) = hend_pair();
          *--hp = sref(0); *--hp = obj_from_fixnum(n);
          sref(0) = hend_pair();
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
        hreserve(pairbsz()*2, sp-r);
        *--hp = obj_from_bool(0); *--hp = sym;
        bnd = hend_pair();
        *--hp = cx__2Atransformers_2A; *--hp = bnd;
        cx__2Atransformers_2A = hend_pair();
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
        hreserve(pairbsz()*2, sp-r);
        *--hp = obj_from_bool(0); *--hp = sym;
        bnd = hend_pair();
        *--hp = cx__2Atransformers_2A; *--hp = bnd;
        cx__2Atransformers_2A = hend_pair();
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
