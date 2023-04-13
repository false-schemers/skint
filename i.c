/* i.c -- instructions */

#include "n.h"
#include "i.h"

/* imports */
extern obj cx__2Aglobals_2A;
extern obj cx__2Atransformers_2A;
extern obj cx__2Adynamic_2Dstate_2A;
extern obj cx_continuation_2Dadapter_2Dcode;
extern obj cx_callmv_2Dadapter_2Dclosure;
extern obj cx__2Acurrent_2Dinput_2A;
extern obj cx__2Acurrent_2Doutput_2A;
extern obj cx__2Acurrent_2Derror_2A;

//#define istagged(o, t) istagged_inlined(o, t) 

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

/* copying objects */
#ifndef OBJCOPY_LOOP
#define objcpy(dst, src, cnt) \
  memcpy((dst), (src), (cnt)*sizeof(obj))  
#define objmove_left(dst, src, cnt) \
  memmove((dst), (src), (cnt)*sizeof(obj))  
#define objmove(dst, src, cnt) \
  memmove((dst), (src), (cnt)*sizeof(obj))  
#else
#define objcpy(dst, src, cnt) \
  do { obj *noalias dp__ = (dst), *noalias sp__ = (src); int n__ = (cnt); \
       while (n__-- > 0) *dp__++ = *sp__++; } while(0)
#define objmove_left(dst, src, cnt) \
  do { int n__ = (cnt); obj *dp__ = (dst), *sp__ = (src); \
       while (n__-- > 0) *dp__++ = *sp__++; } while(0)
#define objmove_right(dst, src, cnt) \
  do { int n__ = (cnt); obj *dp__ = (dst)+n__, *sp__ = (src)+n__; \
       while (n__-- > 0) *--dp__ = *--sp__; } while(0)
#define objmove(dst, src, cnt) \
  do { int n__ = (cnt); obj *dp__ = (dst), *sp__ = (src); \
       if (dp__ > sp__) { dp__ += n__, sp__ += n; while (n__-- > 0) *--dp__ = *--sp__; } \
       else if (dp__ < sp__) while (n__-- > 0) *dp__++ = *sp__++; } while(0)
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
#define VM_REGC       6      /* r[0] ... r[5] */
#define VM_STACK_LEN  256000 /* r[6] ... r[256005] */
#define VM_STACK_RSZ  256    /* red zone for overflow checks */
#define VM_STACK_GSZ  (VM_STACK_LEN-VM_STACK_RSZ)

/* faster non-debug type testing */
#ifdef NDEBUG /* quick */
static int istagged_inline(obj o, int t) { return isobjptr(o) && hblkref(o, 0) == obj_from_size(t); }
#define istagged(o, t) istagged_inline(o, t)
#endif

/* box representation extras */
#define boxbsz()      hbsz(1+1)
#define hend_box()    (*--hp = obj_from_size(BOX_BTAG), hendblk(1+1))

/* pair representation extras */
#define pairbsz()     hbsz(2+1)
#define hend_pair()   (*--hp = obj_from_size(PAIR_BTAG), hendblk(2+1))

/* vector representation extras */
#define vecbsz(n)     hbsz((n)+1)
#define hend_vec(n)   (*--hp = obj_from_size(VECTOR_BTAG), hendblk((n)+1))

/* record representation extras  */
#define recbsz(c)     hbsz((c)+1)
#define hend_rec(rtd, c) (*--hp = rtd, hendblk((c)+1))

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
#define istuple(x)    istagged(x, 0)
#define tupleref(x,i) *taggedref(x, 0, i)
#define tuplelen(x)   taggedlen(x, 0)
#define tuplebsz(c)   hbsz((c)+1)
#define hend_tuple(c) (*--hp = obj_from_size(0), hendblk((c)+1))

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

/* object representation extras */
#define bool_obj(b) obj_from_bool(b)
#define is_bool(o) is_bool_obj(o)
#define get_bool(o) bool_from_obj(o)
#define char_obj(b) obj_from_char(b)
#define is_char(o) is_char_obj(o)
#define get_char(o) char_from_obj(o)
#define void_obj() obj_from_void(0)
#define is_void(o) (o == obj_from_void(0))
#define unit_obj() obj_from_unit()
#define is_unit(o) (o == obj_from_unit())
#define null_obj() mknull()
#define is_null(o) isnull(o)
#define eof_obj() mkeof()
#define is_eof(o) ((o) == mkeof())
#define fixnum_obj(x) obj_from_fixnum(x)
#define is_fixnum(o) is_fixnum_obj(o)
#define are_fixnums(o1, o2) (is_fixnum(o1) && is_fixnum(o2))
#define get_fixnum(o) fixnum_from_obj(o)
#define is_byte(o) is_byte_obj(o)
#define byte_obj(x) obj_from_fixnum((unsigned char)(x))
#define get_byte(o) ((unsigned char)fixnum_from_obj(o))
#ifdef FLONUMS_BOXED
#define flonum_obj(x) hp_pushptr(dupflonum(x), FLONUM_NTAG)
#define is_flonum(o) is_flonum_obj(o)
#define get_flonum(o) flonum_from_obj(o)
#else
#define flonum_obj(x) obj_from_flonum(0, x)
#define is_flonum(o) is_flonum_obj(o)
#define get_flonum(o) flonum_from_obj(o)
#endif
#define is_symbol(o) issymbol(o)
#define get_symbol(o) getsymbol(o)
#define is_pair(o) ispair(o)
#define pair_car(o) car(o)
#define pair_cdr(o) cdr(o)
#define is_list(o) islist(o)
#define is_circular(o) iscircular(o)
#define is_noncircular(o) (!iscircular(o))
#define is_vector(o) isvector(o)
#define vector_len(o) vectorlen(o)
#define vector_ref(o, i) vectorref(o, i)
#define string_obj(s) hp_pushptr((s), STRING_NTAG)
#define is_string(o) isstring(o)
#define string_len(o) stringlen(o) 
#define string_ref(o, i) (*stringref(o, i))
#define bytevector_obj(s) hp_pushptr((s), BYTEVECTOR_NTAG)
#define is_bytevector(o) isbytevector(o)
#define bytevector_len(o) bytevectorlen(o)
#define bytevector_ref(o, i) (*bytevectorref(o, i))
#define iport_file_obj(fp) hp_pushptr((fp), IPORT_FILE_NTAG)
#define oport_file_obj(fp) hp_pushptr((fp), OPORT_FILE_NTAG)
#define iport_string_obj(fp) hp_pushptr((fp), IPORT_STRING_NTAG)
#define oport_string_obj(fp) hp_pushptr((fp), OPORT_STRING_NTAG)
#define iport_bytevector_obj(fp) hp_pushptr((fp), IPORT_BYTEVECTOR_NTAG)
#define oport_bytevector_obj(fp) hp_pushptr((fp), OPORT_BYTEVECTOR_NTAG)
#define is_iport(o) isiport(o)
#define is_oport(o) isoport(o)
#define is_box(o) isbox(o)
#define box_ref(o) boxref(o)
#define is_proc(o) isvmclo(o)
#define is_tuple(o) (isrecord(o) && recordrtd(o) == 0)
#define tuple_len(o) tuplelen(o)
#define tuple_ref(o, i) tupleref(o, i)
#define is_record(o) (isrecord(o) && recordrtd(o) != 0)
#define record_rtd(o) recordrtd(o)
#define record_len(o) recordlen(o)
#define record_ref(o, i) recordref(o, i)


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
    if (isintegrable(r[2]) && is_fixnum(r[3])) {
      const char *cs = integrable_code(integrabledata(r[2]), get_fixnum(r[3]));
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

#define ckp(x) do { obj _x = (x); if (unlikely(!is_pair(_x))) \
  { ac = _x; spush((obj)"pair"); musttail return cxi_failactype(IARGS); } } while (0)
#define ckl(x) do { obj _x = (x); if (unlikely(!is_list(_x))) \
  { ac = _x; spush((obj)"list"); musttail return cxi_failactype(IARGS); } } while (0)
#define cku(x) do { obj _x = (x); if (unlikely(!is_null(_x))) \
  { ac = _x; spush((obj)"proper list"); musttail return cxi_failactype(IARGS); } } while (0)
#define cko(x) do { obj _x = (x); if (unlikely(!is_record(_x))) \
  { ac = _x; spush((obj)"record"); musttail return cxi_failactype(IARGS); } } while (0)
#define ckv(x) do { obj _x = (x); if (unlikely(!is_vector(_x))) \
  { ac = _x; spush((obj)"vector"); musttail return cxi_failactype(IARGS); } } while (0)
#define ckc(x) do { obj _x = (x); if (unlikely(!is_char(_x))) \
  { ac = _x; spush((obj)"char"); musttail return cxi_failactype(IARGS); } } while (0)
#define cks(x) do { obj _x = (x); if (unlikely(!is_string(_x))) \
  { ac = _x; spush((obj)"string"); musttail return cxi_failactype(IARGS); } } while (0)
#define ckb(x) do { obj _x = (x); if (unlikely(!is_bytevector(_x))) \
  { ac = _x; spush((obj)"bytevector"); musttail return cxi_failactype(IARGS); } } while (0)
#define cki(x) do { obj _x = (x); if (unlikely(!is_fixnum(_x))) \
  { ac = _x; spush((obj)"fixnum"); musttail return cxi_failactype(IARGS); } } while (0)
#define ckj(x) do { obj _x = (x); if (unlikely(!is_flonum(_x))) \
  { ac = _x; spush((obj)"flonum"); musttail return cxi_failactype(IARGS); } } while (0)
#define ckn(x) do { obj _x = (x); if (unlikely(!is_fixnum(_x) && !is_flonum(_x))) \
  { ac = _x; spush((obj)"number"); musttail return cxi_failactype(IARGS); } } while (0)
#define ckk(x) do { obj _x = (x); if (unlikely(!is_fixnum(_x) || get_fixnum(_x) < 0)) \
  { ac = _x; spush((obj)"nonnegative fixnum"); musttail return cxi_failactype(IARGS); } } while (0)
#define ck8(x) do { obj _x = (x); if (unlikely(!is_byte(_x))) \
  { ac = _x; spush((obj)"byte"); musttail return cxi_failactype(IARGS); } } while (0)
#define cky(x) do { obj _x = (x); if (unlikely(!is_symbol(_x))) \
  { ac = _x; spush((obj)"symbol"); musttail return cxi_failactype(IARGS); } } while (0)
#define ckr(x) do { obj _x = (x); if (unlikely(!is_iport(_x))) \
  { ac = _x; spush((obj)"input port"); musttail return cxi_failactype(IARGS); } } while (0)
#define ckw(x) do { obj _x = (x); if (unlikely(!is_oport(_x))) \
  { ac = _x; spush((obj)"output port"); musttail return cxi_failactype(IARGS); } } while (0)
#define ckx(x) do { obj _x = (x); if (unlikely(!is_proc(_x))) \
  { ac = _x; spush((obj)"procedure"); musttail return cxi_failactype(IARGS); } } while (0)
#define ckz(x) do { obj _x = (x); if (unlikely(!is_box(_x))) \
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
  if (is_pair(sref(0))) fputs(":\n", stderr); 
  else fputs("\n", stderr);
  for (l = sref(0); is_pair(l); l = pair_cdr(l)) {
    oportputcircular(pair_car(l), p, 0);
    fputc('\n', stderr);
  }
  sdrop(1);
  ac = void_obj();
  unwindi(0); 
}

define_instruction(abort) {
  /* exit code ignored */
  abort();
  unwindi(0);
}

define_instruction(exit) {
  int excode;
  if (ac == bool_obj(0)) excode = 1;
  else if (ac == bool_obj(1)) excode = 0;
  else if (is_fixnum(ac)) excode = (int)get_fixnum(ac);
  else excode = 1;
  exit(excode);
  unwindi(0);
}


define_instruction(lit) { 
  ac = *ip++; 
  gonexti();
}

define_instruction(sref) {
  int i = get_fixnum(*ip++);
  ac = sref(i);
  gonexti();
}  

define_instruction(dref) {
  int i = get_fixnum(*ip++);
  ac = dref(i); 
  gonexti();
}

define_instruction(gref) {
  obj p = *ip++; 
  ac = gref(p);
  gonexti();
}

define_instruction(iref) {
  ac = box_ref(ac);
  gonexti();
}

define_instruction(iset) {
  box_ref(ac) = spop();
  gonexti();
}

define_instruction(dclose) {
  int i, n = get_fixnum(*ip++), c = n+1; 
  hp_reserve(vmclobsz(c));
  for (i = n-1; i >= 0; --i) *--hp = sref(i); /* display */
  *--hp = *ip++; /* code */
  ac = hend_vmclo(c); /* closure */
  sdrop(n);   
  gonexti();
}

define_instruction(sbox) {
  int i = get_fixnum(*ip++); 
  hp_reserve(boxbsz());
  *--hp = sref(i);  
  sref(i) = hend_box();
  gonexti();
}

define_instruction(br) { 
  int dx = get_fixnum(*ip++); 
  ip += dx; 
  gonexti(); 
}

define_instruction(brt) { 
  int dx = get_fixnum(*ip++);
  if (ac) ip += dx; 
  gonexti(); 
}

define_instruction(brnot) { 
  int dx = get_fixnum(*ip++);
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
  int i = get_fixnum(*ip++);
  box_ref(sref(i)) = ac;
  gonexti();
}

define_instruction(dseti) {
  int i = get_fixnum(*ip++);
  box_ref(dref(i)) = ac;
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
  for (n = 0; is_pair(t); t = pair_cdr(t)) ++n; sgrow(n);
  for (i = 0; i < n; ++i, l = pair_cdr(l)) sref(i) = pair_car(l);
  ckx(ac); rd = ac; rx = fixnum_obj(0);
  ac = fixnum_obj(n); /* argc */
  callsubi();
}

define_instruction(cwmv) {
  obj t = ac, x = spop();
  ckx(t); ckx(x);
  /* we can run in constant space in some situations */
  if (vmcloref(x, 0) == cx_continuation_2Dadapter_2Dcode 
   && vmcloref(x, 1) ==  cx__2Adynamic_2Dstate_2A) {
    /* arrange call of t with x as continuation */
    /* [0] adapter_code, [1] dynamic_state */
    int n = vmclolen(x) - 2; 
    assert((cxg_rend - cxg_regs - VM_REGC) > n);
    sp = r + VM_REGC; /* stack is empty */
    objcpy(sp, &vmcloref(x, 2), n);
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
  /* tail-call the consumer with the returned value(s) */
  if (is_unit(val)) { /* (values) in improper context */
    ac = fixnum_obj(0);
  } else if (is_tuple(val)) { /* (values a1 a2 a ...) in improper context */
    int n = tuple_len(val), i;
    for (i = n-1; i >= 0; --i) spush(tuple_ref(val, i));
    ac = fixnum_obj(n);
  } else { /* regular single value */ 
    spush(val); ac = fixnum_obj(1);
  }
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
    int n = get_fixnum(ac), m = 3, i;
    if (sref(n) == fixnum_obj(0) && sref(n+1) == cx_callmv_2Dadapter_2Dclosure) {
      /* tail-call the consumer with the produced values */
      rd = sref(n+2); rx = fixnum_obj(0); /* cns */
      /* NB: can be sped up for popular cases: n == 0, n == 2 */
      objmove_left(sp-n-m, sp-n, n);
      sdrop(m); callsubi();
    } else if (n == 0) { /* return unit (n = 0) */
      ac = unit_obj();
      rx = spop();
      rd = spop();
      retfromi();
    } else { /* return args as tuple (n > 1) */
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
  int m = get_fixnum(*ip++);
  int n; cki(sref(m)); ckx(sref(m+1));
  n = (int)(sp-m-(r+VM_REGC));
  hp_reserve(vmclobsz(n+2));
  hp -= n; objcpy(hp, sp-n-m, n);
  /* [0] adapter_code, [1] dynamic_state */
  *--hp = cx__2Adynamic_2Dstate_2A;
  *--hp = cx_continuation_2Dadapter_2Dcode;
  ac = hend_vmclo(n+2);
  gonexti();
}

define_instruction(lck0) {
  int n; cki(sref(0)); ckx(sref(1));
  n = (int)(sp-(r+VM_REGC));
  hp_reserve(vmclobsz(n+2));
  hp -= n; objcpy(hp, sp-n, n);
  /* [0] adapter_code, [1] dynamic_state */
  *--hp = cx__2Adynamic_2Dstate_2A;
  *--hp = cx_continuation_2Dadapter_2Dcode;
  ac = hend_vmclo(n+2);
  gonexti();
}

define_instruction(wck) {
  obj x = ac, t = spop(); ckx(t); ckx(x);
  if (vmcloref(x, 0) != cx_continuation_2Dadapter_2Dcode) 
    failactype("continuation");
  /* [0] adapter_code, [1] dynamic_state */
  if (vmcloref(x, 1) == cx__2Adynamic_2Dstate_2A) {
    /* restore cont stack and invoke t there */
    int n = vmclolen(x) - 2; 
    assert((cxg_rend - cxg_regs - VM_REGC) > n);
    sp = r + VM_REGC; /* stack is empty */
    objcpy(sp, &vmcloref(x, 2), n);
    sp += n; /* contains n elements now */
    rd = t; rx = fixnum_obj(0); 
    ac = fixnum_obj(0);
    callsubi();
  } else {
    /* have to arrange call of cont adapter */
    spush(x);
    spush(cx_callmv_2Dadapter_2Dclosure); 
    spush(fixnum_obj(0));
    /* call the thunk as producer */
    rd = t; rx = fixnum_obj(0); 
    ac = fixnum_obj(0); 
    callsubi(); 
  }
}

define_instruction(wckr) {
  obj x = ac, o = spop(); ckx(x);
  if (vmcloref(x, 0) != cx_continuation_2Dadapter_2Dcode) 
    failactype("continuation");
  /* [0] adapter_code, [1] dynamic_state */
  if (vmcloref(x, 1) == cx__2Adynamic_2Dstate_2A) {
    /* restore cont stack and return o there */
    int n = vmclolen(x) - 2;
    assert((cxg_rend - cxg_regs - VM_REGC) > n);
    sp = r + VM_REGC; /* stack is empty */
    objcpy(sp, &vmcloref(x, 2), n);
    sp += n;
    ac = o;
    rx = spop();
    rd = spop();
    retfromi();
  } else {
    /* have to arrange call of cont adapter */
    spush(o);
    rd = x; rx = fixnum_obj(0); 
    ac = fixnum_obj(1); 
    callsubi(); 
  }
}


define_instruction(rck) {
  /* called with continuation as rd: 
   * in: ac:argc, args on stack, rd display is dys, saved stack */
  if (vmcloref(rd, 1) != cx__2Adynamic_2Dstate_2A) {
    /* need to run the rest of the code to unwind/rewind on the
     * old stack; rck will be called again when done */
    gonexti(); 
  } else if (ac == fixnum_obj(1)) { /* easy, popular case */
    ac = rd; 
    goi(wckr);
  } else { /* multiple results case */
    /* rd[0] adapter_code, rd[1] dynamic_state */
    int c = get_fixnum(ac), n = vmclolen(rd) - 2, i;
    obj *ks = &vmcloref(rd, 2), *ke = ks + n;
    if (ke-ks > 3 && *--ke == fixnum_obj(0) && *--ke == cx_callmv_2Dadapter_2Dclosure) {
      obj *sb = r + VM_REGC;
      rd = *--ke; rx = fixnum_obj(0); n = (int)(ke - ks); /* cns */
      /* arrange stack as follows: [ks..ke] [arg ...] */
      assert((cxg_rend - cxg_regs - VM_REGC) > n + c);
      if (c) objmove(sb+n, sp-c, c);
      objcpy(sb, ks, n);
      sp = sb+n+c; callsubi();
    } else if (c == 0) { /* return unit (n = 0) */
      spush(unit_obj());
      ac = rd;
      goi(wckr);
    } else { /* return args as tuple (n > 1) */
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

define_instruction(dys) {
  ac = cx__2Adynamic_2Dstate_2A;
  gonexti();
}

define_instruction(setdys) {
  cx__2Adynamic_2Dstate_2A = ac;
  gonexti();
}

define_instruction(save) {
  int dx = get_fixnum(*ip++); 
  spush(rd);
  spush(fixnum_obj(ip + dx - &vector_ref(vmcloref(rd, 0), 0)));  
  gonexti();
}

define_instruction(push) { spush(ac); gonexti(); }

define_instruction(jdceq) {
  obj v = *ip++, i = *ip++;
  if (ac == v) {
    rd = dref(get_fixnum(i));
    rx = fixnum_obj(0);
    callsubi();
  }
  gonexti();
}

define_instruction(jdcge) {
  obj v = *ip++, i = *ip++;
  if (ac >= v) { /* unsigned tagged fixnums can be compared as-is */
    rd = dref(get_fixnum(i));
    rx = fixnum_obj(0);
    callsubi();
  }
  gonexti();
}

define_instruction(jdref) {
  int i = get_fixnum(*ip++); 
  rd = dref(i); 
  rx = fixnum_obj(0);
  callsubi();
}

define_instruction(call) {
  int n = get_fixnum(*ip++); 
  ckx(ac); rd = ac; rx = fixnum_obj(0); 
  ac = fixnum_obj(n); /* argc */
  callsubi();
}

define_instruction(scall) {
  int m = get_fixnum(*ip++), n = get_fixnum(*ip++);
  ckx(ac); rd = ac; rx = fixnum_obj(0); 
  ac = fixnum_obj(n); /* argc */
  objmove_left(sp-n-m, sp-n, n);
  sdrop(m);
  callsubi();
}

define_instruction(return) {
  rx = spop();
  rd = spop();
  retfromi();
}

define_instruction(sreturn) {
  int n = get_fixnum(*ip++); 
  sdrop(n);
  rx = spop();
  rd = spop();
  retfromi();
}

define_instruction(adrop) {
  int n = get_fixnum(*ip++); 
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
  int n = get_fixnum(*ip++), c = get_fixnum(ac), m = c-n; 
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
    objmove_left(sp-c, sp-n, n);
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

define_instruction(unbox) { 
  ckz(ac); 
  ac = box_ref(ac); 
  gonexti(); 
}

define_instruction(setbox) { 
  ckz(ac); 
  box_ref(ac) = spop(); 
  gonexti(); 
}

define_instruction(box) {
  hp_reserve(boxbsz());
  *--hp = ac;
  ac = hend_box();
  gonexti();
}

define_instruction(car) { ckp(ac); ac = pair_car(ac); gonexti(); }
define_instruction(cdr) { ckp(ac); ac = pair_cdr(ac); gonexti(); }
define_instruction(setcar) { ckp(ac); pair_car(ac) = spop(); gonexti(); }
define_instruction(setcdr) { ckp(ac); pair_cdr(ac) = spop(); gonexti(); }

define_instruction(caar) { ckp(ac); ac = pair_car(ac); ckp(ac); ac = pair_car(ac); gonexti(); }
define_instruction(cadr) { ckp(ac); ac = pair_cdr(ac); ckp(ac); ac = pair_car(ac); gonexti(); }
define_instruction(cdar) { ckp(ac); ac = pair_car(ac); ckp(ac); ac = pair_cdr(ac); gonexti(); }
define_instruction(cddr) { ckp(ac); ac = pair_cdr(ac); ckp(ac); ac = pair_cdr(ac); gonexti(); }


define_instruction(nullp) {
  ac = bool_obj(is_null(ac));
  gonexti();
}

define_instruction(pairp) {
  ac = bool_obj(is_pair(ac));
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
  ac = bool_obj(is_list(ac));
  gonexti();
}

define_instruction(list) {
  int i, n = get_fixnum(*ip++);
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
  n = get_fixnum(ac); 
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
  while (is_pair(ac)) { ac = pair_cdr(ac); ++n; }
  ac = fixnum_obj(n);
  gonexti();
}

define_instruction(lget) {
  obj x = spop(); int c;
  ckk(x); c = get_fixnum(x);
  while (c-- > 0) { ckp(ac); ac = pair_cdr(ac); }
  ckp(ac); ac = pair_car(ac);
  gonexti();
}

define_instruction(lput) {
  obj x = spop(); int c;
  ckk(x); c = get_fixnum(x);
  while (c-- > 0) { ckp(ac); ac = pair_cdr(ac); }
  ckp(ac); pair_car(ac) = spop();
  gonexti();
}


define_instruction(memq) {
  obj l = spop(); 
  for (; is_pair(l); l = pair_cdr(l)) {
    if (pair_car(l) == ac) { ac = l; gonexti(); }
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
  for (; is_pair(l); l = pair_cdr(l)) {
    obj p = pair_car(l); if (is_pair(p) && pair_car(p) == ac) { ac = p; gonexti(); }
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
  ckk(x); c = get_fixnum(x);
  while (c-- > 0) { ckp(ac); ac = pair_cdr(ac); }
  gonexti();
}

define_instruction(lpair) {
  obj l = bool_obj(0);
  while (is_pair(ac)) { l = ac; ac = pair_cdr(ac); }
  ac = l;
  gonexti();
}

define_instruction(lrev) {
  obj l = ac, o = null_obj(); int n = 0;
  while (is_pair(ac)) { ac = pair_cdr(ac); ++n; }
  cku(ac); ac = l;
  hp_reserve(pairbsz()*n);
  for (; ac != null_obj(); ac = pair_cdr(ac)) { 
    *--hp = o; *--hp = pair_car(ac);
    o = hend_pair(); 
  }
  ac = o;
  gonexti();
}

define_instruction(lrevi) {
  obj t, v = null_obj();
  while (is_pair(ac)) t = pair_cdr(ac), pair_cdr(ac) = v, v = ac, ac = t;
  ac = v;
  gonexti();
}

define_instruction(circp) {
  ac = bool_obj(is_circular(ac));
  gonexti();
}

define_instruction(charp) {
  ac = bool_obj(is_char(ac));
  gonexti();
}

define_instruction(strp) {
  ac = bool_obj(is_string(ac));
  gonexti();
}

define_instruction(str) {
  int i, n = get_fixnum(*ip++);
  obj o = string_obj(allocstring(n, ' '));
  unsigned char *s = (unsigned char *)stringchars(o);
  for (i = 0; i < n; ++i) {
    obj x = sref(i); ckc(x); s[i] = get_char(x);
  }
  sdrop(n); ac = o;
  gonexti();
}

define_instruction(smk) {
  int n, c; obj x = spop(); 
  ckk(ac); ckc(x);
  n = get_fixnum(ac), c = get_char(x);
  ac = string_obj(allocstring(n, c)); 
  gonexti();
}

define_instruction(slen) {
  cks(ac);
  ac = fixnum_obj(string_len(ac));
  gonexti();
}

define_instruction(sget) {
  obj x = spop(); int i; 
  cks(ac); ckk(x); 
  i = get_fixnum(x); 
  if (i >= string_len(ac)) failtype(x, "valid string index");
  ac = char_obj(string_ref(ac, i));
  gonexti();
}

define_instruction(sput) {
  obj x = spop(), y = spop(); int i; 
  cks(ac); ckk(x); ckc(y); 
  i = get_fixnum(x); 
  if (i >= string_len(ac)) failtype(x, "valid string index");
  string_ref(ac, i) = get_char(y);
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
  is = get_fixnum(x), ie = get_fixnum(y);
  if (is > ie) failtype(x, "valid start string index");
  if (ie > string_len(ac)) failtype(y, "valid end string index");
  d = substring(stringdata(ac), is, ie);
  ac = string_obj(d);
  gonexti();
}

define_instruction(spos) {
  obj x = ac, y = spop(); char *s, *p;
  ckc(x); cks(y);
  s = stringchars(y), p = strchr(s, get_char(x));
  ac = p ? fixnum_obj(p-s) : bool_obj(0);
  gonexti();
}

define_instruction(supc) {
  int *d; char *s; cks(ac);
  d = dupstring(stringdata(ac)); 
  for (s = sdatachars(d); *s; ++s) *s = toupper(*s);
  ac = string_obj(d);
  gonexti();
}

define_instruction(sdnc) {
  int *d; char *s; cks(ac);
  d = dupstring(stringdata(ac)); 
  for (s = sdatachars(d); *s; ++s) *s = tolower(*s);
  ac = string_obj(d);
  gonexti();
}

define_instruction(sflc) {
  int *d; char *s; cks(ac);
  d = dupstring(stringdata(ac)); 
  for (s = sdatachars(d); *s; ++s) *s = tolower(*s); /* stub */
  ac = string_obj(d);
  gonexti();
}


define_instruction(bvecp) {
  ac = bool_obj(is_bytevector(ac));
  gonexti();
}

define_instruction(bvec) {
  int i, n = get_fixnum(*ip++);
  obj o = bytevector_obj(allocbytevector(n));
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
  n = get_fixnum(ac), b = byte_from_obj(x);
  ac = bytevector_obj(makebytevector(n, b)); 
  gonexti();
}

define_instruction(blen) {
  ckb(ac);
  ac = fixnum_obj(bytevector_len(ac));
  gonexti();
}

define_instruction(bget) {
  obj x = spop(); int i; 
  ckb(ac); ckk(x); 
  i = get_fixnum(x); 
  if (i >= bytevector_len(ac)) failtype(x, "valid bytevector index");
  ac = fixnum_obj(bytevector_ref(ac, i));
  gonexti();
}

define_instruction(bput) {
  obj x = spop(), y = spop(); int i; 
  ckb(ac); ckk(x); ck8(y); 
  i = get_fixnum(x); 
  if (i >= bytevector_len(ac)) failtype(x, "valid bytevector index");
  bytevector_ref(ac, i) = byte_from_obj(y);
  gonexti();
}

define_instruction(bsub) {
  obj x = spop(), y = spop(); int is, ie, *d; 
  ckb(ac); ckk(x); ckk(y); 
  is = get_fixnum(x), ie = get_fixnum(y);
  if (is > ie) failtype(x, "valid start bytevector index");
  if (ie > bytevector_len(ac)) failtype(y, "valid end bytevector index");
  d = subbytevector(bytevectordata(ac), is, ie);
  ac = bytevector_obj(d);
  gonexti();
}

define_instruction(beq) {
  obj x = ac, y = spop(); ckb(x); ckb(y);
  ac = bool_obj(bytevectoreq(bytevectordata(x), bytevectordata(y)));
  gonexti(); 
}


define_instruction(recp) {
  obj x = ac, y = spop();
  if (is_void(y)) {
    ac = bool_obj(is_record(ac));
  } else {
    ac = bool_obj(is_record(ac) && record_rtd(ac) == y);
  }
  gonexti();
}

define_instruction(rmk) {
  int i, n; obj v; ckk(sref(0));
  n = get_fixnum(sref(0)); 
  hp_reserve(recbsz(n)); v = sref(1);
  for (i = 0; i < n; ++i) *--hp = v;
  ac = hend_rec(ac, n);
  sdrop(2);
  gonexti();
}

define_instruction(rlen) {
  cko(ac);
  ac = fixnum_obj(record_len(ac));
  gonexti();
}

define_instruction(rget) {
  obj x = spop(); int i; 
  cko(ac); ckk(x); 
  i = get_fixnum(x);
  if (i >= record_len(ac)) failtype(x, "valid record index");
  ac = record_ref(ac, i);
  gonexti();
}

define_instruction(rput) {
  obj x = spop(), y = spop(); int i; 
  cko(ac); ckk(x);
  i = get_fixnum(x);
  if (i >= record_len(ac)) failtype(x, "valid record index");
  record_ref(ac, i) = y;
  gonexti();
}

define_instruction(rrtd) {
  cko(ac);
  ac = record_rtd(ac);
  gonexti();
}


define_instruction(vecp) {
  ac = bool_obj(is_vector(ac));
  gonexti();
}

define_instruction(vec) {
  int i, n = get_fixnum(*ip++);
  hp_reserve(vecbsz(n));
  for (i = n-1; i >= 0; --i) *--hp = sref(i);
  ac = hend_vec(n);
  sdrop(n);
  gonexti();
}

define_instruction(vmk) {
  int i, n; obj v; ckk(ac);
  n = get_fixnum(ac); 
  hp_reserve(vecbsz(n)); v = sref(0);
  for (i = 0; i < n; ++i) *--hp = v;
  ac = hend_vec(n);
  sdrop(1);
  gonexti();
}

define_instruction(vlen) {
  ckv(ac);
  ac = fixnum_obj(vector_len(ac));
  gonexti();
}

define_instruction(vget) {
  obj x = spop(); int i; 
  ckv(ac); ckk(x); 
  i = get_fixnum(x);
  if (i >= vector_len(ac)) failtype(x, "valid vector index");
  ac = vector_ref(ac, i);
  gonexti();
}

define_instruction(vput) {
  obj x = spop(), y = spop(); int i; 
  ckv(ac); ckk(x);
  i = get_fixnum(x);
  if (i >= vector_len(ac)) failtype(x, "valid vector index");
  vector_ref(ac, i) = y;
  gonexti();
}

define_instruction(vcat) {
  obj x = ac, y = sref(0); int n1, n2, n;
  ckv(x); ckv(y);
  n1 = vector_len(x), n2 = vector_len(y), n = n1 + n2;
  hp_reserve(vecbsz(n));
  /* NB: vector_ref fails to return pointer to empty vector's start */
  hp -= n2; if (n2) objcpy(hp, &vector_ref(y, 0), n2);
  hp -= n1; if (n1) objcpy(hp, &vector_ref(x, 0), n1);
  ac = hend_vec(n);
  sdrop(1);
  gonexti();
}

define_instruction(vtol) {
  obj l = null_obj(); int n;
  ckv(ac); n = vector_len(ac);
  hp_reserve(pairbsz()*n);
  while (n > 0) {
    *--hp = l; *--hp = vector_ref(ac, n-1);
    l = hend_pair();
    --n;
  }
  ac = l;
  gonexti();
}

define_instruction(ltov) {
  obj l = ac; int n = 0, i;
  while (is_pair(l)) { l = pair_cdr(l); ++n; } cku(l);
  hp_reserve(vecbsz(n));
  for (l = ac, i = 0, hp -= n; i < n; ++i, l = pair_cdr(l)) hp[i] = pair_car(l);
  ac = hend_vec(n);
  gonexti();
}

define_instruction(ltob) {
  obj l = ac, o; int n = 0, i; unsigned char *s;
  while (is_pair(l)) { l = pair_cdr(l); ++n; } cku(l);
  o = bytevector_obj(allocbytevector(n));
  s = bytevectorbytes(o);
  for (i = 0, l = ac; i < n; ++i, l = pair_cdr(l)) {
    obj x = pair_car(l); ck8(x);
    s[i] = get_byte(x);
  } 
  ac = o;
  gonexti();
}

define_instruction(stol) {
  obj l = null_obj(); int n;
  cks(ac); n = string_len(ac);
  hp_reserve(pairbsz()*n);
  while (n > 0) {
    *--hp = l; *--hp = char_obj(string_ref(ac, n-1));
    l = hend_pair();
    --n;
  }
  ac = l;
  gonexti();
}

define_instruction(ltos) {
  obj l = ac; int n = 0, i, *d;
  while (is_pair(l)) { l = pair_cdr(l); ++n; }
  d = allocstring(n, ' ');
  for (i = 0; i < n; ac = pair_cdr(ac), ++i)
    sdatachars(d)[i] = get_char(pair_car(ac));
  ac = string_obj(d);
  gonexti();
}

define_instruction(ytos) {
  cky(ac);
  ac = string_obj(newstring(symbolname(get_symbol(ac))));
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
  num = get_fixnum(x), radix = get_fixnum(y);
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
  s = stringchars(x); radix = get_fixnum(y);
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
  ac = fixnum_obj(get_char(ac));
  gonexti();
}

define_instruction(itoc) {
  cki(ac); /* TODO: range check? */
  ac = char_obj(get_fixnum(ac));
  gonexti();
}

define_instruction(jtos) {
  char buf[30], *s; double d; ckj(ac);
  d = get_flonum(ac); sprintf(buf, "%.15g", d);
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
  if (is_fixnum(ac)) goi(itos);
  if (unlikely(spop() != fixnum_obj(10))) fail("non-10 radix in flonum conversion");
  goi(jtos);
}

define_instruction(ston) {
  char *s; int radix; long l; double d;
  obj x = ac, y = spop(); cks(x); ckk(y);
  s = stringchars(x); radix = get_fixnum(y);
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
  ac = bool_obj(get_fixnum(ac) > 0);
  gonexti(); 
}

define_instruction(inegp) {
  cki(ac);
  ac = bool_obj(get_fixnum(ac) < 0);
  gonexti(); 
}

define_instruction(ievnp) {
  cki(ac);
  ac = bool_obj((get_fixnum(ac) & 1) == 0);
  gonexti(); 
}

define_instruction(ioddp) {
  cki(ac);
  ac = bool_obj((get_fixnum(ac) & 1) != 0);
  gonexti(); 
}

define_instruction(iadd) {
  obj x = ac, y = spop(); cki(x); cki(y);
  ac = fixnum_obj(fxadd(get_fixnum(x), get_fixnum(y)));
  gonexti(); 
}

define_instruction(isub) {
  obj x = ac, y = spop(); cki(x); cki(y);
  ac = fixnum_obj(fxsub(get_fixnum(x), get_fixnum(y)));
  gonexti(); 
}

define_instruction(imul) {
  obj x = ac, y = spop(); cki(x); cki(y);
  ac = fixnum_obj(fxmul(get_fixnum(x), get_fixnum(y)));
  gonexti();
}

define_instruction(iquo) {
  obj x = ac, y = spop(); cki(x); cki(y);
  if (y == fixnum_obj(0)) fail("division by zero");
  ac = fixnum_obj(fxquo(get_fixnum(x), get_fixnum(y)));
  gonexti();
}

define_instruction(irem) {
  obj x = ac, y = spop(); cki(x); cki(y);
  if (y == fixnum_obj(0)) fail("division by zero");
  ac = fixnum_obj(fxrem(get_fixnum(x), get_fixnum(y)));
  gonexti();
}

define_instruction(imin) {
  obj x = ac, y = spop(); cki(x); cki(y);
  ac = get_fixnum(x) < get_fixnum(y) ? x : y;
  gonexti(); 
}

define_instruction(imax) {
  obj x = ac, y = spop(); cki(x); cki(y);
  ac = get_fixnum(x) > get_fixnum(y) ? x : y;
  gonexti(); 
}

define_instruction(ineg) {
  cki(ac);
  ac = fixnum_obj(fxneg(get_fixnum(ac)));
  gonexti();
}

define_instruction(iabs) {
  cki(ac);
  ac = fixnum_obj(fxabs(get_fixnum(ac)));
  gonexti();
}

define_instruction(itoj) {
  cki(ac);
  ac = flonum_obj((double)get_fixnum(ac));
  gonexti();
}

define_instruction(idiv) {
  obj x = ac, y = spop(); cki(x); cki(y);
  if (y == fixnum_obj(0)) fail("division by zero");
  ac = fixnum_obj(fxdiv(get_fixnum(x), get_fixnum(y)));
  gonexti();
}

define_instruction(imqu) {
  obj x = ac, y = spop(); cki(x); cki(y);
  if (y == fixnum_obj(0)) fail("division by zero");
  ac = fixnum_obj(fxmqu(get_fixnum(x), get_fixnum(y)));
  gonexti();
}

define_instruction(imlo) {
  obj x = ac, y = spop(); cki(x); cki(y);
  if (y == fixnum_obj(0)) fail("division by zero");
  ac = fixnum_obj(fxmlo(get_fixnum(x), get_fixnum(y)));
  gonexti();
}

define_instruction(ieuq) {
  obj x = ac, y = spop(); cki(x); cki(y);
  if (y == fixnum_obj(0)) fail("division by zero");
  ac = fixnum_obj(fxeuq(get_fixnum(x), get_fixnum(y)));
  gonexti();
}

define_instruction(ieur) {
  obj x = ac, y = spop(); cki(x); cki(y);
  if (y == fixnum_obj(0)) fail("division by zero");
  ac = fixnum_obj(fxeur(get_fixnum(x), get_fixnum(y)));
  gonexti();
}

define_instruction(igcd) {
  obj x = ac, y = spop(); cki(x); cki(y);
  ac = fixnum_obj(fxgcd(get_fixnum(x), get_fixnum(y)));
  gonexti();
}

define_instruction(ipow) {
  obj x = ac, y = spop(); cki(x); cki(y);
  ac = fixnum_obj(fxpow(get_fixnum(x), get_fixnum(y)));
  gonexti();
}

define_instruction(isqrt) {
  cki(ac); /* TODO: check for negative */
  ac = fixnum_obj(fxsqrt(get_fixnum(ac)));
  gonexti();
}

define_instruction(inot) {
  cki(ac);
  ac = fixnum_obj(~get_fixnum(ac));
  gonexti();
}

define_instruction(iand) {
  obj x = ac, y = spop(); cki(x); cki(y);
  ac = fixnum_obj(get_fixnum(x) & get_fixnum(y));
  gonexti();
}

define_instruction(iior) {
  obj x = ac, y = spop(); cki(x); cki(y);
  ac = fixnum_obj(get_fixnum(x) | get_fixnum(y));
  gonexti();
}

define_instruction(ixor) {
  obj x = ac, y = spop(); cki(x); cki(y);
  ac = fixnum_obj(get_fixnum(x) ^ get_fixnum(y));
  gonexti();
}

define_instruction(iasl) {
  obj x = ac, y = spop(); cki(x); cki(y);
  ac = fixnum_obj(fxasl(get_fixnum(x), get_fixnum(y)));
  gonexti();
}

define_instruction(iasr) {
  obj x = ac, y = spop(); cki(x); cki(y);
  ac = fixnum_obj(fxasr(get_fixnum(x), get_fixnum(y)));
  gonexti();
}

define_instruction(ilt) {
  obj x = ac, y = spop(); cki(x); cki(y);
  ac = bool_obj(get_fixnum(x) < get_fixnum(y));
  gonexti(); 
}

define_instruction(igt) {
  obj x = ac, y = spop(); cki(x); cki(y);
  ac = bool_obj(get_fixnum(x) > get_fixnum(y));
  gonexti(); 
}

define_instruction(ile) {
  obj x = ac, y = spop(); cki(x); cki(y);
  ac = bool_obj(get_fixnum(x) <= get_fixnum(y));
  gonexti(); 
}

define_instruction(ige) {
  obj x = ac, y = spop(); cki(x); cki(y);
  ac = bool_obj(get_fixnum(x) >= get_fixnum(y));
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
  ac = bool_obj(get_flonum(ac) == 0.0);
  gonexti();
}

define_instruction(jposp) {
  ckj(ac);
  ac = bool_obj(get_flonum(ac) > 0.0);
  gonexti(); 
}

define_instruction(jnegp) {
  ckj(ac);
  ac = bool_obj(get_flonum(ac) < 0.0);
  gonexti(); 
}

define_instruction(jevnp) {
  double f; ckj(ac);
  f = get_flonum(ac);
  ac = bool_obj(flisint(f / 2.0));
  gonexti(); 
}

define_instruction(joddp) {
  double f; ckj(ac);
  f = get_flonum(ac);
  ac = bool_obj(flisint((f + 1.0) / 2.0));
  gonexti(); 
}

define_instruction(jintp) {
  double f; ckj(ac);
  f = get_flonum(ac);
  ac = bool_obj(f > -HUGE_VAL && f < HUGE_VAL && f == floor(f));
  gonexti(); 
}

define_instruction(jnanp) {
  double f; ckj(ac);
  f = get_flonum(ac);
  ac = bool_obj(f != f);
  gonexti(); 
}

define_instruction(jfinp) {
  double f; ckj(ac);
  f = get_flonum(ac);
  ac = bool_obj(f > -HUGE_VAL && f < HUGE_VAL);
  gonexti(); 
}

define_instruction(jinfp) {
  double f; ckj(ac);
  f = get_flonum(ac);
  ac = bool_obj(f <= -HUGE_VAL || f >= HUGE_VAL);
  gonexti(); 
}

define_instruction(jadd) {
  obj x = ac, y = spop(); ckj(x); ckj(y);
  ac = flonum_obj(get_flonum(x) + get_flonum(y));
  gonexti(); 
}

define_instruction(jsub) {
  obj x = ac, y = spop(); ckj(x); ckj(y);
  ac = flonum_obj(get_flonum(x) - get_flonum(y));
  gonexti(); 
}

define_instruction(jmul) {
  obj x = ac, y = spop(); ckj(x); ckj(y);
  ac = flonum_obj(get_flonum(x) * get_flonum(y));
  gonexti();
}

define_instruction(jdiv) {
  obj x = ac, y = spop(); ckj(x); ckj(y);
  ac = flonum_obj(get_flonum(x) / get_flonum(y));
  gonexti();
}

define_instruction(jlt) {
  obj x = ac, y = spop(); ckj(x); ckj(y);
  ac = bool_obj(get_flonum(x) < get_flonum(y));
  gonexti(); 
}

define_instruction(jgt) {
  obj x = ac, y = spop(); ckj(x); ckj(y);
  ac = bool_obj(get_flonum(x) > get_flonum(y));
  gonexti(); 
}

define_instruction(jle) {
  obj x = ac, y = spop(); ckj(x); ckj(y);
  ac = bool_obj(get_flonum(x) <= get_flonum(y));
  gonexti(); 
}

define_instruction(jge) {
  obj x = ac, y = spop(); ckj(x); ckj(y);
  ac = bool_obj(get_flonum(x) >= get_flonum(y));
  gonexti(); 
}

define_instruction(jeq) {
  obj x = ac, y = spop(); ckj(x); ckj(y);
  ac = bool_obj(get_flonum(x) == get_flonum(y));
  gonexti(); 
}

define_instruction(jne) {
  obj x = ac, y = spop(); ckj(x); ckj(y);
  ac = bool_obj(get_flonum(x) != get_flonum(y));
  gonexti(); 
}

define_instruction(jmin) {
  obj x = ac, y = spop(); ckj(x); ckj(y);
  ac = get_flonum(x) < get_flonum(y) ? x : y;
  gonexti(); 
}

define_instruction(jmax) {
  obj x = ac, y = spop(); ckj(x); ckj(y);
  ac = get_flonum(x) > get_flonum(y) ? x : y;
  gonexti(); 
}

define_instruction(jneg) {
  ckj(ac);
  ac = flonum_obj(-get_flonum(ac));
  gonexti(); 
}

define_instruction(jabs) {
  ckj(ac);
  ac = flonum_obj(fabs(get_flonum(ac)));
  gonexti(); 
}

define_instruction(jgcd) {
  obj x = ac, y = spop(); ckj(x); ckj(y);
  ac = flonum_obj(flgcd(get_flonum(x), get_flonum(y)));
  gonexti(); 
}

define_instruction(jpow) {
  obj x = ac, y = spop(); ckj(x); ckj(y);
  ac = flonum_obj(pow(get_flonum(x), get_flonum(y)));
  gonexti(); 
}

define_instruction(jsqrt) {
  ckj(ac);
  ac = flonum_obj(sqrt(get_flonum(ac)));
  gonexti(); 
}

define_instruction(jtoi) {
  ckj(ac);
  ac = fixnum_obj(fxflo(get_flonum(ac)));
  gonexti();
}

define_instruction(jquo) {
  obj x = ac, y = spop(); double n, d, i; 
  ckj(x); ckj(y);
  n = get_flonum(x), d = get_flonum(y); modf(n/d,  &i);
  ac = flonum_obj(i);
  gonexti(); 
}

define_instruction(jrem) {
  /* NB: we keep sign: (flremainder -10.0 2.0) => -0.0 */
  obj x = ac, y = spop(); ckj(x); ckj(y);
  ac = flonum_obj(fmod(get_flonum(x), get_flonum(y)));
  gonexti();
}


define_instruction(jmqu) {
  obj x = ac, y = spop(); ckj(x); ckj(y);
  ac = flonum_obj(flmqu(get_flonum(x), get_flonum(y)));
  gonexti(); 
}

define_instruction(jmlo) {
  obj x = ac, y = spop(); ckj(x); ckj(y);
  ac = flonum_obj(flmlo(get_flonum(x), get_flonum(y)));
  gonexti(); 
}

define_instruction(jfloor) {
  ckj(ac);
  ac = flonum_obj(floor(get_flonum(ac)));
  gonexti(); 
}

define_instruction(jceil) {
  ckj(ac);
  ac = flonum_obj(ceil(get_flonum(ac)));
  gonexti(); 
}

define_instruction(jtrunc) {
  double i; ckj(ac);
  modf(get_flonum(ac), &i);
  ac = flonum_obj(i);
  gonexti(); 
}

define_instruction(jround) {
  ckj(ac);
  ac = flonum_obj(flround(get_flonum(ac)));
  gonexti(); 
}

define_instruction(jexp) {
  ckj(ac);
  ac = flonum_obj(exp(get_flonum(ac)));
  gonexti(); 
}

define_instruction(jlog) {
  obj x = ac, y = spop(); ckj(ac);
  if (likely(!y)) {
    ac = flonum_obj(log(get_flonum(ac)));
  } else {
    double b; ckj(y); b = get_flonum(y);
    if (likely(b == 10.0)) ac = flonum_obj(log10(get_flonum(ac)));
    else ac = flonum_obj(log(get_flonum(ac))/log(b));
  }
  gonexti(); 
}

define_instruction(jsin) {
  ckj(ac);
  ac = flonum_obj(sin(get_flonum(ac)));
  gonexti(); 
}

define_instruction(jcos) {
  ckj(ac);
  ac = flonum_obj(cos(get_flonum(ac)));
  gonexti(); 
}

define_instruction(jtan) {
  ckj(ac);
  ac = flonum_obj(tan(get_flonum(ac)));
  gonexti(); 
}

define_instruction(jasin) {
  ckj(ac);
  ac = flonum_obj(asin(get_flonum(ac)));
  gonexti(); 
}

define_instruction(jacos) {
  ckj(ac);
  ac = flonum_obj(acos(get_flonum(ac)));
  gonexti(); 
}

define_instruction(jatan) {
  obj x = ac, y = spop(); ckj(ac);
  if (likely(!y)) {
    ac = flonum_obj(atan(get_flonum(x)));
  } else {
    ckj(y); 
    ac = flonum_obj(atan2(get_flonum(x), get_flonum(y)));
  }
  gonexti(); 
}


define_instruction(zerop) {
  obj x = ac;
  if (likely(is_fixnum(x))) {
    ac = bool_obj(x == fixnum_obj(0));
  } else if (likely(is_flonum(x))) {
    ac = bool_obj(get_flonum(x) == 0.0);
  } else failactype("number");
  gonexti();
}

define_instruction(posp) {
  if (likely(is_fixnum(ac))) ac = bool_obj(get_fixnum(ac) > 0);
  else if (likely(is_flonum(ac))) ac = bool_obj(get_flonum(ac) > 0.0);
  else failactype("number");
  gonexti(); 
}

define_instruction(negp) {
  if (likely(is_fixnum(ac))) ac = bool_obj(get_fixnum(ac) < 0);
  else if (likely(is_flonum(ac))) ac = bool_obj(get_flonum(ac) < 0.0);
  else failactype("number");
  gonexti(); 
}

define_instruction(add) {
  obj x = ac, y = spop();
  if (likely(are_fixnums(x, y))) {
    long lx = get_fixnum(x), ly = get_fixnum(y);   
    long long llz = (long long)lx + (long long)ly;
    if (likely(llz >= FIXNUM_MIN && llz <= FIXNUM_MAX)) ac = fixnum_obj((long)llz);
    else ac = flonum_obj((double)lx + (double)ly);
  } else {
    double dx, dy;
    if (likely(is_flonum(x))) dx = get_flonum(x);
    else if (likely(is_fixnum(x))) dx = (double)get_fixnum(x);
    else failtype(x, "number");
    if (likely(is_flonum(y))) dy = get_flonum(y);
    else if (likely(is_fixnum(y))) dy = (double)get_fixnum(y);
    else failtype(y, "number");
    ac = flonum_obj(dx + dy);
  }
  gonexti(); 
}

define_instruction(sub) {
  obj x = ac, y = spop();
  if (likely(are_fixnums(x, y))) {
    long lx = get_fixnum(x), ly = get_fixnum(y);   
    long long llz = (long long)lx - (long long)ly;
    if (likely(llz >= FIXNUM_MIN && llz <= FIXNUM_MAX)) ac = fixnum_obj((long)llz);
    else ac = flonum_obj((double)lx - (double)ly);
  } else {
    double dx, dy;
    if (likely(is_flonum(x))) dx = get_flonum(x);
    else if (likely(is_fixnum(x))) dx = (double)get_fixnum(x);
    else failtype(x, "number");
    if (likely(is_flonum(y))) dy = get_flonum(y);
    else if (likely(is_fixnum(y))) dy = (double)get_fixnum(y);
    else failtype(y, "number");
    ac = flonum_obj(dx - dy);
  }
  gonexti(); 
}

define_instruction(mul) {
  obj x = ac, y = spop();
  if (likely(are_fixnums(x, y))) {
    long lx = get_fixnum(x), ly = get_fixnum(y);   
    long long llz = (long long)lx * (long long)ly;
    if (likely(llz >= FIXNUM_MIN && llz <= FIXNUM_MAX)) ac = fixnum_obj((long)llz);
    else ac = flonum_obj((double)lx * (double)ly);
  } else {
    double dx, dy;
    if (likely(is_flonum(x))) dx = get_flonum(x);
    else if (likely(is_fixnum(x))) dx = (double)get_fixnum(x);
    else failtype(x, "number");
    if (likely(is_flonum(y))) dy = get_flonum(y);
    else if (likely(is_fixnum(y))) dy = (double)get_fixnum(y);
    else failtype(y, "number");
    ac = flonum_obj(dx * dy);
  }
  gonexti(); 
}

define_instruction(div) {
  obj x = ac, y = spop();
  if (likely(are_fixnums(x, y))) {
    long lx, ly; long long llz, llr;
    if (unlikely(y == fixnum_obj(0))) fail("division by zero");
    lx = get_fixnum(x), ly = get_fixnum(y);   
    llz = (long long)lx / (long long)ly, llr = (long long)lx % (long long)ly;
    if (likely(!llr && llz >= FIXNUM_MIN && llz <= FIXNUM_MAX)) ac = fixnum_obj((long)llz);
    else ac = flonum_obj((double)lx / (double)ly);
  } else {
    double dx, dy;
    if (likely(is_flonum(x))) dx = get_flonum(x);
    else if (likely(is_fixnum(x))) dx = (double)get_fixnum(x);
    else failtype(x, "number");
    if (likely(is_flonum(y))) dy = get_flonum(y);
    else if (likely(is_fixnum(y))) dy = (double)get_fixnum(y);
    else failtype(y, "number");
    ac = flonum_obj(dx / dy);
  }
  gonexti(); 
}

define_instruction(quo) {
  obj x = ac, y = spop();
  if (likely(are_fixnums(x, y))) {
    if (unlikely(y == fixnum_obj(0))) fail("division by zero");
    ac = fixnum_obj(fxquo(get_fixnum(x), get_fixnum(y)));
  } else {
    double dx, dy, dz;
    if (likely(is_flonum(x) && flisint(dx = get_flonum(x)))) /* ok */;
    else if (likely(is_fixnum(x))) dx = (double)get_fixnum(x);
    else failtype(x, "integer");
    if (likely(is_flonum(y) && flisint(dy = get_flonum(y)))) /* ok */;
    else if (likely(is_fixnum(y))) dy = (double)get_fixnum(y);
    else failtype(y, "integer");
    modf(dx / dy,  &dz);
    ac = flonum_obj(dz);
  }
  gonexti(); 
}

define_instruction(rem) {
  obj x = ac, y = spop();
  if (likely(are_fixnums(x, y))) {
    if (unlikely(y == fixnum_obj(0))) fail("division by zero");
    ac = fixnum_obj(fxrem(get_fixnum(x), get_fixnum(y)));
  } else {
    double dx, dy, dz;
    if (likely(is_flonum(x) && flisint(dx = get_flonum(x)))) /* ok */;
    else if (likely(is_fixnum(x))) dx = (double)get_fixnum(x);
    else failtype(x, "integer");
    if (likely(is_flonum(y) && flisint(dy = get_flonum(y)))) /* ok */;
    else if (likely(is_fixnum(y))) dy = (double)get_fixnum(y);
    else failtype(y, "integer");
    dz = fmod(dx, dy);
    /* keep zero positive: (remainder -10.0 2.0) => 0.0, not -0.0 */
    ac = flonum_obj((dz == 0.0) ? 0.0 : dz);
  }
  gonexti(); 
}

define_instruction(mqu) {
  obj x = ac, y = spop();
  if (likely(are_fixnums(x, y))) {
    if (unlikely(y == fixnum_obj(0))) fail("division by zero");
    ac = fixnum_obj(fxmqu(get_fixnum(x), get_fixnum(y)));
  } else {
    double dx, dy;
    if (likely(is_flonum(x))) dx = get_flonum(x);
    else if (likely(is_fixnum(x))) dx = (double)get_fixnum(x);
    else failtype(x, "number");
    if (likely(is_flonum(y))) dy = get_flonum(y);
    else if (likely(is_fixnum(y))) dy = (double)get_fixnum(y);
    else failtype(y, "number");
    ac = flonum_obj(flmqu(dx, dy));
  }
  gonexti(); 
}

define_instruction(mlo) {
  obj x = ac, y = spop();
  if (likely(are_fixnums(x, y))) {
    if (unlikely(y == fixnum_obj(0))) fail("division by zero");
    ac = fixnum_obj(fxmlo(get_fixnum(x), get_fixnum(y)));
  } else {
    double dx, dy;
    if (likely(is_flonum(x))) dx = get_flonum(x);
    else if (likely(is_fixnum(x))) dx = (double)get_fixnum(x);
    else failtype(x, "number");
    if (likely(is_flonum(y))) dy = get_flonum(y);
    else if (likely(is_fixnum(y))) dy = (double)get_fixnum(y);
    else failtype(y, "number");
    ac = flonum_obj(flmlo(dx, dy));
  }
  gonexti(); 
}


define_instruction(lt) {
  obj x = ac, y = spop();
  if (likely(are_fixnums(x, y))) {
    ac = bool_obj(get_fixnum(x) < get_fixnum(y));
  } else {
    double dx, dy;
    if (likely(is_flonum(x))) dx = get_flonum(x);
    else if (likely(is_fixnum(x))) dx = (double)get_fixnum(x);
    else failtype(x, "number");
    if (likely(is_flonum(y))) dy = get_flonum(y);
    else if (likely(is_fixnum(y))) dy = (double)get_fixnum(y);
    else failtype(y, "number");
    ac = bool_obj(dx < dy);
  }
  gonexti(); 
}

define_instruction(gt) {
  obj x = ac, y = spop();
  if (likely(are_fixnums(x, y))) {
    ac = bool_obj(get_fixnum(x) > get_fixnum(y));
  } else {
    double dx, dy;
    if (likely(is_flonum(x))) dx = get_flonum(x);
    else if (likely(is_fixnum(x))) dx = (double)get_fixnum(x);
    else failtype(x, "number");
    if (likely(is_flonum(y))) dy = get_flonum(y);
    else if (likely(is_fixnum(y))) dy = (double)get_fixnum(y);
    else failtype(y, "number");
    ac = bool_obj(dx > dy);
  }
  gonexti(); 
}

define_instruction(le) {
  obj x = ac, y = spop();
  if (likely(are_fixnums(x, y))) {
    ac = bool_obj(get_fixnum(x) <= get_fixnum(y));
  } else {
    double dx, dy;
    if (likely(is_flonum(x))) dx = get_flonum(x);
    else if (likely(is_fixnum(x))) dx = (double)get_fixnum(x);
    else failtype(x, "number");
    if (likely(is_flonum(y))) dy = get_flonum(y);
    else if (likely(is_fixnum(y))) dy = (double)get_fixnum(y);
    else failtype(y, "number");
    ac = bool_obj(dx <= dy);
  }
  gonexti(); 
}

define_instruction(ge) {
  obj x = ac, y = spop();
  if (likely(are_fixnums(x, y))) {
    ac = bool_obj(get_fixnum(x) >= get_fixnum(y));
  } else {
    double dx, dy;
    if (likely(is_flonum(x))) dx = get_flonum(x);
    else if (likely(is_fixnum(x))) dx = (double)get_fixnum(x);
    else failtype(x, "number");
    if (likely(is_flonum(y))) dy = get_flonum(y);
    else if (likely(is_fixnum(y))) dy = (double)get_fixnum(y);
    else failtype(y, "number");
    ac = bool_obj(dx >= dy);
  }
  gonexti(); 
}

define_instruction(eq) {
  obj x = ac, y = spop();
  if (likely(are_fixnums(x, y))) {
    ac = bool_obj(x == y);
  } else if (is_flonum(x) || is_flonum(y)) {
    double dx, dy;
    if (likely(is_flonum(x))) dx = get_flonum(x);
    else if (likely(is_fixnum(x))) dx = (double)get_fixnum(x);
    else failtype(x, "number");
    if (likely(is_flonum(y))) dy = get_flonum(y);
    else if (likely(is_fixnum(y))) dy = (double)get_fixnum(y);
    else failtype(y, "number");
    ac = bool_obj(dx == dy);
  } else ac = bool_obj(0);
  gonexti(); 
}

define_instruction(ne) {
  obj x = ac, y = spop();
  if (likely(are_fixnums(x, y))) {
    ac = bool_obj(x != y);
  } else if (is_flonum(x) || is_flonum(y)) {
    double dx, dy;
    if (likely(is_flonum(x))) dx = get_flonum(x);
    else if (likely(is_fixnum(x))) dx = (double)get_fixnum(x);
    else failtype(x, "number");
    if (likely(is_flonum(y))) dy = get_flonum(y);
    else if (likely(is_fixnum(y))) dy = (double)get_fixnum(y);
    else failtype(y, "number");
    ac = bool_obj(dx != dy);
  } else ac = bool_obj(1);
  gonexti(); 
}

define_instruction(min) {
  obj x = ac, y = spop();
  if (likely(are_fixnums(x, y))) {
    ac = (get_fixnum(x) < get_fixnum(y)) ? x : y;
  } else {
    double dx, dy;
    if (likely(is_flonum(x))) dx = get_flonum(x);
    else if (likely(is_fixnum(x))) dx = (double)get_fixnum(x);
    else failtype(x, "number");
    if (likely(is_flonum(y))) dy = get_flonum(y);
    else if (likely(is_fixnum(y))) dy = (double)get_fixnum(y);
    else failtype(y, "number");
    ac = dx < dy ? flonum_obj(dx) : flonum_obj(dy);
  }
  gonexti(); 
}

define_instruction(max) {
  obj x = ac, y = spop();
  if (likely(are_fixnums(x, y))) {
    ac = (get_fixnum(x) > get_fixnum(y)) ? x : y;
  } else {
    double dx, dy;
    if (likely(is_flonum(x))) dx = get_flonum(x);
    else if (likely(is_fixnum(x))) dx = (double)get_fixnum(x);
    else failtype(x, "number");
    if (likely(is_flonum(y))) dy = get_flonum(y);
    else if (likely(is_fixnum(y))) dy = (double)get_fixnum(y);
    else failtype(y, "number");
    ac = dx > dy ? flonum_obj(dx) : flonum_obj(dy);
  }
  gonexti(); 
}

define_instruction(gcd) {
  obj x = ac, y = spop();
  if (likely(are_fixnums(x, y))) {
    ac = fixnum_obj(fxgcd(get_fixnum(x), get_fixnum(y)));
  } else {
    double dx, dy;
    if (likely(is_flonum(x))) dx = get_flonum(x);
    else if (likely(is_fixnum(x))) dx = (double)get_fixnum(x);
    else failtype(x, "number");
    if (likely(is_flonum(y))) dy = get_flonum(y);
    else if (likely(is_fixnum(y))) dy = (double)get_fixnum(y);
    else failtype(y, "number");
    ac = flonum_obj(flgcd(dx, dy));
  }
  gonexti(); 
}

define_instruction(pow) {
  obj x = ac, y = spop();
  if (likely(are_fixnums(x, y))) {
    /* fixme: this will either overflow, or fail on negative y */
    ac = fixnum_obj(fxpow(get_fixnum(x), get_fixnum(y)));
  } else {
    double dx, dy;
    if (likely(is_flonum(x))) dx = get_flonum(x);
    else if (likely(is_fixnum(x))) dx = (double)get_fixnum(x);
    else failtype(x, "number");
    if (likely(is_flonum(y))) dy = get_flonum(y);
    else if (likely(is_fixnum(y))) dy = (double)get_fixnum(y);
    else failtype(y, "number");
    ac = flonum_obj(pow(dx, dy));
  }
  gonexti(); 
}

define_instruction(sqrt) {
  if (likely(is_flonum(ac))) {
    ac = flonum_obj(sqrt(get_flonum(ac)));
  } else if (likely(is_fixnum(ac))) {
    long x = get_fixnum(ac), y;
    if (x < 0) ac = flonum_obj((HUGE_VAL - HUGE_VAL));   
    else if (y = fxsqrt(x), y*y == x) ac = fixnum_obj(y);
    else ac = flonum_obj(sqrt((double)x));
  } else failactype("number");
  gonexti(); 
}

define_instruction(exp) {
  double x;
  if (unlikely(is_fixnum(ac))) x = (double)get_fixnum(ac);
  else if (likely(is_flonum(ac))) x = get_flonum(ac);
  else failactype("number");
  ac = flonum_obj(exp(x));
  gonexti(); 
}

define_instruction(log) {
  double x; obj y = spop();
  if (unlikely(is_fixnum(ac))) x = (double)get_fixnum(ac);
  else if (likely(is_flonum(ac))) x = get_flonum(ac);
  else failactype("number");
  if (likely(!y)) {
    ac = flonum_obj(log(x));
  } else if (likely(y == fixnum_obj(10))) {
    ac = flonum_obj(log10(x));
  } else {
    double b; 
    if (unlikely(is_fixnum(y))) b = (double)get_fixnum(y);
    else if (likely(is_flonum(y))) b = get_flonum(y);
    else failtype(y, "number");
    if (likely(b == 10.0)) ac = flonum_obj(log10(x));
    else ac = flonum_obj(log(x)/log(b));
  }
  gonexti(); 
}

define_instruction(sin) {
  double x;
  if (unlikely(is_fixnum(ac))) {
    x = (double)get_fixnum(ac);
  } else if (likely(is_flonum(ac))) {
    x = get_flonum(ac);
  } else failactype("number");
  ac = flonum_obj(sin(x));
  gonexti(); 
}

define_instruction(cos) {
  double x;
  if (unlikely(is_fixnum(ac))) {
    x = (double)get_fixnum(ac);
  } else if (likely(is_flonum(ac))) {
    x = get_flonum(ac);
  } else failactype("number");
  ac = flonum_obj(cos(x));
  gonexti(); 
}

define_instruction(tan) {
  double x;
  if (unlikely(is_fixnum(ac))) {
    x = (double)get_fixnum(ac);
  } else if (likely(is_flonum(ac))) {
    x = get_flonum(ac);
  } else failactype("number");
  ac = flonum_obj(tan(x));
  gonexti(); 
}

define_instruction(asin) {
  double x;
  if (unlikely(is_fixnum(ac))) {
    x = (double)get_fixnum(ac);
  } else if (likely(is_flonum(ac))) {
    x = get_flonum(ac);
  } else failactype("number");
  ac = flonum_obj(asin(x));
  gonexti(); 
}

define_instruction(acos) {
  double x;
  if (unlikely(is_fixnum(ac))) {
    x = (double)get_fixnum(ac);
  } else if (likely(is_flonum(ac))) {
    x = get_flonum(ac);
  } else failactype("number");
  ac = flonum_obj(acos(x));
  gonexti(); 
}

define_instruction(atan) {
  double x; obj y = spop();
  if (unlikely(is_fixnum(ac))) x = (double)get_fixnum(ac);
  else if (likely(is_flonum(ac))) x = get_flonum(ac);
  else failactype("number");
  if (likely(!y)) {
    ac = flonum_obj(atan(x));
  } else {
    double b; 
    if (unlikely(is_fixnum(y))) b = (double)get_fixnum(y);
    else if (likely(is_flonum(y))) b = get_flonum(y);
    else failtype(y, "number");
    ac = flonum_obj(atan2(x, b));
  }
  gonexti(); 
}

define_instruction(neg) {
  if (likely(is_fixnum(ac))) {
    ac = fixnum_obj(-get_fixnum(ac));
  } else if (likely(is_flonum(ac))) {
    ac = flonum_obj(-get_flonum(ac));
  } else failactype("number");
  gonexti(); 
}

define_instruction(abs) {
  if (likely(is_fixnum(ac))) {
    ac = fixnum_obj(fxabs(get_fixnum(ac)));
  } else if (likely(is_flonum(ac))) {
    ac = flonum_obj(fabs(get_flonum(ac)));
  } else failactype("number");
  gonexti(); 
}

define_instruction(floor) {
  if (likely(is_flonum(ac))) {
    ac = flonum_obj(floor(get_flonum(ac)));
  } else if (unlikely(!is_fixnum(ac))) {
    failactype("number");
  }
  gonexti(); 
}

define_instruction(ceil) {
  if (likely(is_flonum(ac))) {
    ac = flonum_obj(ceil(get_flonum(ac)));
  } else if (unlikely(!is_fixnum(ac))) {
    failactype("number");
  }
  gonexti(); 
}

define_instruction(trunc) {
  if (likely(is_flonum(ac))) {
    double x = get_flonum(ac);
    double i; modf(x,  &i);
    ac = flonum_obj(i);
  } else if (unlikely(!is_fixnum(ac))) {
    failactype("number");
  }
  gonexti(); 
}

define_instruction(round) {
  if (likely(is_flonum(ac))) {
    ac = flonum_obj(flround(get_flonum(ac)));
  } else if (unlikely(!is_fixnum(ac))) {
    failactype("number");
  }
  gonexti(); 
}


define_instruction(nump) {
  ac = bool_obj(is_fixnum(ac) || is_flonum(ac));
  gonexti(); 
}

define_instruction(fixp) {
  ac = bool_obj(is_fixnum(ac));
  gonexti(); 
}

define_instruction(flop) {
  ac = bool_obj(is_flonum(ac));
  gonexti(); 
}

define_instruction(intp) {
  if (likely(is_fixnum(ac))) {
    ac = bool_obj(1);
  } else { /* accepts any object! */
    ac = bool_obj(is_flonum(ac) && flisint(get_flonum(ac)));
  }
  gonexti(); 
}

define_instruction(ratp) {
  if (likely(is_fixnum(ac))) {
    ac = bool_obj(1);
  } else if (likely(is_flonum(ac))) {
    double f = get_flonum(ac);
    ac = bool_obj(f > -HUGE_VAL && f < HUGE_VAL);
  } else {
    ac = bool_obj(0);
  }
  gonexti(); 
}


define_instruction(nanp) {
  if (unlikely(is_fixnum(ac))) {
    ac = bool_obj(0);
  } else if (likely(is_flonum(ac))) {
    double f = get_flonum(ac);
    ac = bool_obj(f != f);
  } else failactype("number");
  gonexti(); 
}

define_instruction(finp) {
  if (unlikely(is_fixnum(ac))) {
    ac = bool_obj(1);
  } else if (likely(is_flonum(ac))) {
    double f = get_flonum(ac);
    ac = bool_obj(f > -HUGE_VAL && f < HUGE_VAL);
  } else failactype("number");
  gonexti(); 
}

define_instruction(infp) {
  if (unlikely(is_fixnum(ac))) {
    ac = bool_obj(0);
  } else if (likely(is_flonum(ac))) {
    double f = get_flonum(ac);
    ac = bool_obj(f <= -HUGE_VAL || f >= HUGE_VAL);
  } else failactype("number");
  gonexti(); 
}

define_instruction(evnp) {
  if (likely(is_fixnum(ac))) {
    ac = bool_obj((get_fixnum(ac) & 1) == 0);
  } else if (likely(is_flonum(ac))) {
    double f = get_flonum(ac);
    ac = bool_obj(flisint(f / 2.0));
  } else failactype("number");
  gonexti(); 
}

define_instruction(oddp) {
  if (likely(is_fixnum(ac))) {
    ac = bool_obj((get_fixnum(ac) & 1) != 0);
  } else if (likely(is_flonum(ac))) {
    double f = get_flonum(ac);
    ac = bool_obj(flisint((f + 1.0) / 2.0));
  } else failactype("number");
  gonexti(); 
}

define_instruction(ntoi) {
  if (likely(is_flonum(ac))) ac = fixnum_obj(fxflo(get_flonum(ac)));
  else if (likely(is_fixnum(ac))) /* keep ac as-is */ ;
  else failactype("number");
  gonexti(); 
}

define_instruction(ntoj) {
  if (likely(is_fixnum(ac))) ac = flonum_obj((flonum_t)get_fixnum(ac));
  else if (likely(is_flonum(ac))) /* keep ac as-is */ ;
  else failactype("number");
  gonexti(); 
}


define_instruction(lcat) {
  obj t, l, *p, *d; int c;
  for (l = ac, c = 0; is_pair(l); l = pair_cdr(l)) ++c;
  hp_reserve(pairbsz()*c);
  p = --sp; t = *p; /* pop & take addr */
  for (l = ac; is_pair(l); l = pair_cdr(l)) {
    *--hp = t; d = hp; *--hp = pair_car(l);
    *p = hend_pair(); p = d;
  }
  ac = *sp;
  gonexti(); 
}

define_instruction(lcpy) {
  obj t, l, *p, *d; int c;
  for (l = ac, c = 0; is_pair(l); l = pair_cdr(l)) ++c;
  hp_reserve(pairbsz()*c);
  p = sp; *p = t = l; /* tail of last pair */
  for (l = ac; is_pair(l); l = pair_cdr(l)) {
    *--hp = t; d = hp; *--hp = pair_car(l);
    *p = hend_pair(); p = d;
  }
  ac = *sp;
  gonexti(); 
}

define_instruction(ccmp) {
  obj x = ac, y = spop(); int cmp; ckc(x); ckc(y);
  cmp = get_char(x) - get_char(y);
  ac = fixnum_obj(cmp);
  gonexti(); 
}

define_instruction(ceq) {
  obj x = ac, y = spop(); ckc(x); ckc(y);
  ac = bool_obj(get_char(x) == get_char(y));
  gonexti(); 
}

define_instruction(clt) {
  obj x = ac, y = spop(); ckc(x); ckc(y);
  ac = bool_obj(get_char(x) < get_char(y));
  gonexti(); 
}

define_instruction(cgt) {
  obj x = ac, y = spop(); ckc(x); ckc(y);
  ac = bool_obj(get_char(x) > get_char(y));
  gonexti(); 
}

define_instruction(cle) {
  obj x = ac, y = spop(); ckc(x); ckc(y);
  ac = bool_obj(get_char(x) <= get_char(y));
  gonexti(); 
}

define_instruction(cge) {
  obj x = ac, y = spop(); ckc(x); ckc(y);
  ac = bool_obj(get_char(x) >= get_char(y));
  gonexti(); 
}

define_instruction(cicmp) {
  obj x = ac, y = spop(); int cmp; ckc(x); ckc(y);
  cmp = tolower(get_char(x)) - tolower(get_char(y));
  ac = fixnum_obj(cmp);
  gonexti(); 
}

define_instruction(cieq) {
  obj x = ac, y = spop(); ckc(x); ckc(y);
  ac = bool_obj(tolower(get_char(x)) == tolower(get_char(y)));
  gonexti(); 
}

define_instruction(cilt) {
  obj x = ac, y = spop(); ckc(x); ckc(y);
  ac = bool_obj(tolower(get_char(x)) < tolower(get_char(y)));
  gonexti(); 
}

define_instruction(cigt) {
  obj x = ac, y = spop(); ckc(x); ckc(y);
  ac = bool_obj(tolower(get_char(x)) > tolower(get_char(y)));
  gonexti(); 
}

define_instruction(cile) {
  obj x = ac, y = spop(); ckc(x); ckc(y);
  ac = bool_obj(tolower(get_char(x)) <= tolower(get_char(y)));
  gonexti(); 
}

define_instruction(cige) {
  obj x = ac, y = spop(); ckc(x); ckc(y);
  ac = bool_obj(tolower(get_char(x)) >= tolower(get_char(y)));
  gonexti(); 
}

define_instruction(cwsp) {
  ckc(ac);
  ac = bool_obj(isspace(get_char(ac)));
  gonexti(); 
}

define_instruction(clcp) {
  ckc(ac);
  ac = bool_obj(islower(get_char(ac)));
  gonexti(); 
}

define_instruction(cucp) {
  ckc(ac);
  ac = bool_obj(isupper(get_char(ac)));
  gonexti(); 
}

define_instruction(calp) {
  ckc(ac);
  ac = bool_obj(isalpha(get_char(ac)));
  gonexti(); 
}

define_instruction(cnup) {
  ckc(ac);
  ac = bool_obj(isdigit(get_char(ac)));
  gonexti(); 
}

define_instruction(cupc) {
  ckc(ac);
  ac = char_obj(toupper(get_char(ac)));
  gonexti(); 
}

define_instruction(cdnc) {
  ckc(ac);
  ac = char_obj(tolower(get_char(ac)));
  gonexti(); 
}

define_instruction(cflc) {
  ckc(ac);
  ac = char_obj(tolower(get_char(ac))); /* stub */
  gonexti(); 
}

define_instruction(cdgv) {
  int ch; ckc(ac);
  ch = get_char(ac);
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
  ac = bool_obj(is_symbol(ac));
  gonexti();
}

define_instruction(boolp) {
  ac = bool_obj(is_bool(ac));
  gonexti();
}

define_instruction(boxp) {
  ac = bool_obj(is_box(ac));
  gonexti();
}

define_instruction(funp) {
  ac = bool_obj(is_proc(ac));
  gonexti();
}

define_instruction(voidp) {
  ac = bool_obj(is_void(ac));
  gonexti();
}

define_instruction(void) {
  ac = void_obj();
  gonexti();
}

define_instruction(ipp) {
  ac = bool_obj(is_iport(ac));
  gonexti();
}

define_instruction(opp) {
  ac = bool_obj(is_oport(ac));
  gonexti();
}

define_instruction(cin) {
  ac = cx__2Acurrent_2Dinput_2A;
  assert(is_iport(ac));
  gonexti();
}

define_instruction(cout) {
  ac = cx__2Acurrent_2Doutput_2A;
  assert(is_oport(ac));
  gonexti();
}

define_instruction(cerr) {
  ac = cx__2Acurrent_2Derror_2A;
  assert(is_oport(ac));
  gonexti();
}

define_instruction(setcin) {
  ckr(ac);
  cx__2Acurrent_2Dinput_2A = ac;
  gonexti();
}

define_instruction(setcout) {
  ckw(ac);
  cx__2Acurrent_2Doutput_2A = ac;
  gonexti();
}

define_instruction(setcerr) {
  ckw(ac);
  cx__2Acurrent_2Derror_2A = ac;
  gonexti();
  gonexti();
}

define_instruction(sip) {
  ac = iport_file_obj(stdin);
  gonexti();
}

define_instruction(sop) {
  ac = oport_file_obj(stdout);
  gonexti();
}

define_instruction(sep) {
  ac = oport_file_obj(stderr);
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

define_instruction(oif) {
  FILE *fp = fopen(stringchars(ac), "r");
  ac = (fp == NULL) ? bool_obj(0) : iport_file_obj(fp);
  gonexti();
}

define_instruction(oof) {
  FILE *fp = fopen(stringchars(ac), "w");
  ac = (fp == NULL) ? bool_obj(0) : oport_file_obj(fp);
  gonexti();
}

define_instruction(obif) {
  FILE *fp = fopen(stringchars(ac), "rb");
  ac = (fp == NULL) ? bool_obj(0) : iport_file_obj(fp);
  gonexti();
}

define_instruction(obof) {
  FILE *fp = fopen(stringchars(ac), "wb");
  ac = (fp == NULL) ? bool_obj(0) : oport_file_obj(fp);
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

define_instruction(oib) {
  int *d; unsigned char *p, *e; ckb(ac);
  d = dupbytevector(bytevectordata(ac));
  p = bvdatabytes(d), e = p + *d;
  ac = iport_bytevector_obj(bvialloc(p, e, d));
  gonexti();
}

define_instruction(oob) {
  ac = oport_bytevector_obj(newcb());
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

define_instruction(gob) {
  cxtype_oport_t *vt; ckw(ac);
  vt = ckoportvt(ac);
  if (vt != (cxtype_oport_t *)OPORT_BYTEVECTOR_NTAG &&
      vt != (cxtype_oport_t *)OPORT_STRING_NTAG) {
    ac = eof_obj();
  } else {
    cbuf_t *pcb = oportdata(ac);
    int len = (int)(pcb->fill - pcb->buf);
    ac = bytevector_obj(newbytevector((unsigned char *)pcb->buf, len));
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

define_instruction(rd8) {
  int c; ckr(ac);
  c = iportgetc(ac);
  if (unlikely(c == EOF)) ac = eof_obj();
  else ac = byte_obj(c);
  gonexti();
}

define_instruction(rda8) {
  int c; ckr(ac);
  c = iportpeekc(ac);
  if (unlikely(c == EOF)) ac = eof_obj();
  else ac = byte_obj(c);
  gonexti();
}

define_instruction(rd8r) {
  ckr(ac);
  ac = bool_obj(1); /* no portable way to detect hanging? */
  gonexti();
}


define_instruction(eofp) {
  ac = bool_obj(is_eof(ac));
  gonexti();
}

define_instruction(eof) {
  ac = eof_obj();
  gonexti();
}


define_instruction(wrc) {
  obj x = ac, y = spop(); ckc(x); ckw(y);
  oportputc(get_char(x), y);
  ac = void_obj();
  gonexti();
}

define_instruction(wrs) {
  obj x = ac, y = spop(); cks(x); ckw(y);
  oportputs(stringchars(x), y);
  ac = void_obj();
  gonexti();
}

define_instruction(wr8) {
  obj x = ac, y = spop(); ck8(x); ckw(y);
  oportputc(get_byte(x), y);
  ac = void_obj();
  gonexti();
}

define_instruction(wrb) {
  obj x = ac, y = spop(); int *d; ckb(x); ckw(y);
  d = bytevectordata(x);
  oportwrite((char *)bvdatabytes(d), *d, y);
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

define_instruction(itrs) {
  ac = cx__2Atransformers_2A;
  gonexti(); 
}

define_instruction(igp) {
  ac = bool_obj(isintegrable(ac));
  gonexti(); 
}

define_instruction(iglk) {
  struct intgtab_entry *pe; cky(ac);
  pe = lookup_integrable(get_symbol(ac));
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
  n = get_fixnum(spop());
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

define_instruction(srefi0) { ac = box_ref(sref(0)); gonexti(); }
define_instruction(srefi1) { ac = box_ref(sref(1)); gonexti(); }
define_instruction(srefi2) { ac = box_ref(sref(2)); gonexti(); }
define_instruction(srefi3) { ac = box_ref(sref(3)); gonexti(); }
define_instruction(srefi4) { ac = box_ref(sref(4)); gonexti(); }
define_instruction(pushsrefi0) { ac = box_ref(sref(0)); spush(ac); gonexti(); }
define_instruction(pushsrefi1) { ac = box_ref(sref(1)); spush(ac); gonexti(); }
define_instruction(pushsrefi2) { ac = box_ref(sref(2)); spush(ac); gonexti(); }
define_instruction(pushsrefi3) { ac = box_ref(sref(3)); spush(ac); gonexti(); }
define_instruction(pushsrefi4) { ac = box_ref(sref(4)); spush(ac); gonexti(); }

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

define_instruction(drefi0) { ac = box_ref(dref(0)); gonexti(); }
define_instruction(drefi1) { ac = box_ref(dref(1)); gonexti(); }
define_instruction(drefi2) { ac = box_ref(dref(2)); gonexti(); }
define_instruction(drefi3) { ac = box_ref(dref(3)); gonexti(); }
define_instruction(drefi4) { ac = box_ref(dref(4)); gonexti(); }
define_instruction(pushdrefi0) { ac = box_ref(dref(0)); spush(ac); gonexti(); }
define_instruction(pushdrefi1) { ac = box_ref(dref(1)); spush(ac); gonexti(); }
define_instruction(pushdrefi2) { ac = box_ref(dref(2)); spush(ac); gonexti(); }
define_instruction(pushdrefi3) { ac = box_ref(dref(3)); spush(ac); gonexti(); }
define_instruction(pushdrefi4) { ac = box_ref(dref(4)); spush(ac); gonexti(); }

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
  int m = 1, n = get_fixnum(*ip++);
  ckx(ac); rd = ac; rx = fixnum_obj(0); 
  ac = fixnum_obj(n); /* argc */
  objmove_left(sp-n-m, sp-n, n);
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
  int m = 2, n = get_fixnum(*ip++);
  ckx(ac); rd = ac; rx = fixnum_obj(0); 
  ac = fixnum_obj(n);
  objmove_left(sp-n-m, sp-n, n);
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
  int m = 3, n = get_fixnum(*ip++);
  ckx(ac); rd = ac; rx = fixnum_obj(0); 
  ac = fixnum_obj(n);
  objmove_left(sp-n-m, sp-n, n);
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
  int m = 4, n = get_fixnum(*ip++);
  ckx(ac); rd = ac; rx = fixnum_obj(0); 
  ac = fixnum_obj(n);
  objmove_left(sp-n-m, sp-n, n);
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
  if (likely(are_fixnums(x, y))) {
    int dx = get_fixnum(*ip++);
    ac = bool_obj(get_fixnum(x) < get_fixnum(y));
    ip = ac ? ip : ip + dx;
    gonexti(); 
  } else {
    int dx = get_fixnum(*ip++);
    double fx, fy;
    if (likely(is_flonum(x))) fx = get_flonum(x);
    else { cki(x); fx = (double)get_fixnum(x); }
    if (likely(is_flonum(y))) fy = get_flonum(y);
    else { cki(y); fy = (double)get_fixnum(y); }
    ac = bool_obj(fx < fy);
    ip = ac ? ip : ip + dx;
    gonexti(); 
  }
}

define_instruction(pushsub) {
  obj x = ac, y = spop();
  if (likely(are_fixnums(x, y))) {
    long lx = get_fixnum(x), ly = get_fixnum(y);   
    long long llz = (long long)lx - (long long)ly;
    if (likely(llz >= FIXNUM_MIN && llz <= FIXNUM_MAX)) ac = fixnum_obj((long)llz);
    else ac = flonum_obj((double)lx - (double)ly);
  } else {
    double dx, dy;
    if (likely(is_flonum(x))) dx = get_flonum(x);
    else if (likely(is_fixnum(x))) dx = (double)get_fixnum(x);
    else failtype(x, "number");
    if (likely(is_flonum(y))) dy = get_flonum(y);
    else if (likely(is_fixnum(y))) dy = (double)get_fixnum(y);
    else failtype(y, "number");
    ac = flonum_obj(dx - dy);
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

define_instruction(argvref) {
  int i; char *s; ckk(ac);
  i = get_fixnum(ac); /* todo: range-check */
  s = cxg_argv[i];
  if (s) ac = string_obj(newstring(s));
  else ac = bool_obj(0); 
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
  for (l = ra, i = 0; i < n; ++i, l = cdr(l)) *--hp = pair_car(l);
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
        n = get_fixnum(ra);
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
      case 't': {
        lcode = lbuf; assert(pe->enc); 
        pe0 = pe->enc; pe1 = pe0 + strlen(pe0) + 1; assert(*pe1);
        sprintf(lbuf, "%%!2.0u?{%s,.3,.3%s]3}.0du?{.0a,.3,.3%s]3}%%%%", pe1, pe0, pe0);
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
    } else if (name != 0 && name[0] == 'K' && name[1] == 0) {
      /* special entry for cx_continuation_2Dadapter_2Dcode */
      ent += 1; name = ent[0], data = ent[1];
      assert(name == 0); assert(data != 0);
      ra = mkiport_string(sp-r, sialloc((char*)data, NULL));
      hp = rds_seq(r, sp, hp);  /* ra=port => ra=revcodelist/eof */
      if (!iseof(ra)) hp = revlist2vec(r, sp, hp); /* ra => ra */
      assert(!iseof(ra));
      cx_continuation_2Dadapter_2Dcode = ra;
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

/* partially hand-coded module (prototyped in i.scm) */
char *i_code[] = {

  /* initialize current port variables */
  "C", 0,
  "P10Psi" "P11Pso" "P12Pse",

  /* internal continuation switch code */
  "P", "%dynamic-state-reroot!",
  "%1.0,,#0.0,&1{%1.0,yq~?{${.2d,:0^[01}.0ad,.1aa,y,.1,.3c,.1sa.3,.1sdf,."
  "4san,.4sd.3sy_1.0[30}]1}.!0.0^_1[11",

  /* code for dynamic-wind's internal lambda is modified as follows:
   * ,    save argc by pushing it on top of args in stack
   * ${   push new frame for return from %dynamic-state-reroot!
   * :0   get 'here' dynamic state from internal lambda's display
   * ,    put it on the stack for dynamic-state-reroot!
   * @(y22:%25dynamic-state-reroot!) get the d-s-r! procedure
   * [01  call it with 1 argument ('here' dynamic state)
   * }    we will return here when d-s-r! is finished 
   * _!   pop saved argc from stack into ac register
   * K6   use sdmv opcode to return args from the lambda
   * also, %x procedure checks inserted for early error detection
  */
  "P", "dynamic-wind",
  "%3y,${.2,.6%x,.5%xcc,@(y22:%25dynamic-state-reroot!)[01}.0,&1{,${:0,@("
  "y22:%25dynamic-state-reroot!)[01}_!K6},.3,@(y16:call-with-values)[42",

  /* code for the continuation adapter:
   * k!   first attempt; does not return if nothing to un/re-wind
   * ,    save argc by pushing it on top of args in stack
   * ${   push new frame for return from %dynamic-state-reroot!
   * :0   get old dynamic state from continuation's display
   * ,    put it on the stack for dynamic-state-reroot!
   * @(y22:%25dynamic-state-reroot!) get the d-s-r! procedure
   * [01  call it with 1 argument (old dynamic state)
   * }    we will return here when d-s-r! is finished 
   * _!   pop saved argc from stack; we are ready to retry
   * k!   retry; should not return this time
   * %%   signal an (argument?) error if we return ?? */
  "K", 0, 
  "k!,${:0,@(y22:%25dynamic-state-reroot!)[01}_!k!%%",

  0, 0, 0
};

/* protects registers from r to sp=r+2; returns new hp */
static obj *init_modules(obj *r, obj *sp, obj *hp)
{
  extern char* i_code[]; /* i.c */
  extern char* s_code[]; /* s.c */
  extern char* t_code[]; /* t.c */
  hp = init_module(r, sp, hp, (const char **)i_code);
  hp = init_module(r, sp, hp, (const char **)s_code);
  hp = init_module(r, sp, hp, (const char **)t_code);
  return hp;
}
