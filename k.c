/* k.c -- generated via skint ksf2c.ssc k.sf */

#include "n.h"
#include "i.h"

#define MODULE module_k
#define LOAD()

/* cx globals */
obj cx__2Acurrent_2Derror_2A; /* *current-error* */
obj cx__2Acurrent_2Dinput_2A; /* *current-input* */
obj cx__2Acurrent_2Doutput_2A; /* *current-output* */
obj cx__2Adynamic_2Dstate_2A; /* *dynamic-state* */
obj cx__2Aglobals_2A; /* *globals* */
obj cx__2Atransformers_2A; /* *transformers* */
obj cx_callmv_2Dadapter_2Dclosure; /* callmv-adapter-closure */
obj cx_continuation_2Dadapter_2Dcode; /* continuation-adapter-code */
obj cx_decode; /* decode */
obj cx_decode_2Dsexp; /* decode-sexp */
obj cx_execute_2Dthunk_2Dclosure; /* execute-thunk-closure */
obj cx_initialize_2Dmodules; /* initialize-modules */
obj cx_install_2Dglobal_2Dlambdas; /* install-global-lambdas */
obj cx_main; /* main */
obj cx_make_2Dclosure; /* make-closure */
obj cx_tcode_2Drepl; /* tcode-repl */
static obj cx__2312; /* constant #12 */
static obj cx__2316; /* constant #16 */

/* gc roots */
static obj *globv[] = {
  &cx__2Acurrent_2Derror_2A,
  &cx__2Acurrent_2Dinput_2A,
  &cx__2Acurrent_2Doutput_2A,
  &cx__2Adynamic_2Dstate_2A,
  &cx__2Aglobals_2A,
  &cx__2Atransformers_2A,
  &cx_callmv_2Dadapter_2Dclosure,
  &cx_continuation_2Dadapter_2Dcode,
  &cx_decode,
  &cx_decode_2Dsexp,
  &cx_execute_2Dthunk_2Dclosure,
  &cx_initialize_2Dmodules,
  &cx_install_2Dglobal_2Dlambdas,
  &cx_make_2Dclosure,
  &cx__2312,
  &cx__2316,
};

static cxroot_t root = {
  sizeof(globv)/sizeof(obj *), globv, NULL
};

/* entry points */
static obj host(obj);
static obj cases[10] = {
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
  (obj)host,  (obj)host,  (obj)host,  (obj)host,  (obj)host,
};

/* host procedure */
#define MAX_HOSTREGS 16
static obj host(obj pc)
{
  register obj *r = cxg_regs;
  register obj *hp = cxg_hp;
  register int rc = cxg_rc;
  rreserve(MAX_HOSTREGS); 
jump: 
  switch (case_from_obj(pc)) {

case 0: /* load module */
    cx__2312 = (hpushstr(0, newstring("K5")));
    { static char s[] = { 36, 123, 64, 40, 121, 52, 58, 114, 101, 112, 108, 41, 91, 48, 48, 125, 0 };
    cx__2316 = (hpushstr(0, newstring(s))); }
    { /* make-vector */
    obj o; int i = 0, c = (+991);
    hreserve(hbsz(c+1), 0); /* 0 live regs */
    o = (mknull()); /* gc-safe */
    while (i++ < c) *--hp = o;
    *--hp = obj_from_size(VECTOR_BTAG);
    cx__2Aglobals_2A = (hendblk(c+1)); }
    { /* cons */ 
    hreserve(hbsz(3), 0); /* 0 live regs */
    *--hp = (mknull());
    *--hp = obj_from_bool(0);
    *--hp = obj_from_size(PAIR_BTAG); 
    cx__2Adynamic_2Dstate_2A = (hendblk(3)); }
    cx__2Acurrent_2Dinput_2A = obj_from_bool(0);
    cx__2Acurrent_2Doutput_2A = obj_from_bool(0);
    cx__2Acurrent_2Derror_2A = obj_from_bool(0);
    { /* define execute-thunk-closure */
    static obj c[] = { obj_from_objptr(vmcases+0) }; 
    cx_execute_2Dthunk_2Dclosure = obj_from_objptr(c); }
    { /* define make-closure */
    static obj c[] = { obj_from_objptr(vmcases+1) };
    cx_make_2Dclosure = obj_from_objptr(c); }
    { /* define decode-sexp */
    static obj c[] = { obj_from_objptr(vmcases+2) };
    cx_decode_2Dsexp = obj_from_objptr(c); }
    { /* define decode */
    static obj c[] = { obj_from_objptr(vmcases+3) };
    cx_decode = obj_from_objptr(c); }
    cx__2Atransformers_2A = (mknull());
    cx_continuation_2Dadapter_2Dcode = obj_from_bool(0);
    { /* define decode */
    static obj c[] = { obj_from_objptr(vmcases+3) };
    r[0] = obj_from_objptr(c); }
    hreserve(hbsz(0+1), 1); /* 1 live regs */
    *--hp = obj_from_case(1);
    r[1] = (hendblk(0+1));
    r[2+0] = r[0];  
    pc = objptr_from_obj(r[2+0])[0];
    r[2+1] = r[1];  
    r[2+2] = (cx__2312);
    r += 2; /* shift reg wnd */
    rreserve(MAX_HOSTREGS);
    rc = 3;
    goto jump;

case 1: /* clo ek r */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
    /* ek r */
    { /* define make-closure */
    static obj c[] = { obj_from_objptr(vmcases+1) };
    r[2] = obj_from_objptr(c); }
    hreserve(hbsz(0+1), 3); /* 3 live regs */
    *--hp = obj_from_case(2);
    r[3] = (hendblk(0+1));
    r[4+0] = r[2];  
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = r[3];  
    r[4+2] = r[1];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_HOSTREGS);
    rc = 3;
    goto jump;

case 2: /* clo ek r */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
    /* ek r */
    cx_callmv_2Dadapter_2Dclosure = r[1];  
    { /* define install-global-lambdas */
    static obj c[] = { obj_from_objptr(vmcases+6) };
    cx_install_2Dglobal_2Dlambdas = obj_from_objptr(c); }
    { /* define install-global-lambdas */
    static obj c[] = { obj_from_objptr(vmcases+6) };
    r[2] = obj_from_objptr(c); }
    hreserve(hbsz(0+1), 3); /* 3 live regs */
    *--hp = obj_from_case(3);
    r[3] = (hendblk(0+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[3];  
    rreserve(MAX_HOSTREGS);
    rc = 2;
    goto jump;

case 3: /* clo ek . */
    assert(rc >= 2);
    r[2] = obj_from_void(0); /* ignored */
    r += 1; /* shift reg. wnd */
    /* ek . */
    { /* define initialize-modules */
    static obj c[] = { obj_from_objptr(vmcases+7) };
    cx_initialize_2Dmodules = obj_from_objptr(c); }
    { /* define initialize-modules */
    static obj c[] = { obj_from_objptr(vmcases+7) };
    r[2] = obj_from_objptr(c); }
    hreserve(hbsz(0+1), 3); /* 3 live regs */
    *--hp = obj_from_case(4);
    r[3] = (hendblk(0+1));
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = r[3];  
    rreserve(MAX_HOSTREGS);
    rc = 2;
    goto jump;

case 4: /* clo ek . */
    assert(rc >= 2);
    r[2] = obj_from_void(0); /* ignored */
    r += 1; /* shift reg. wnd */
    /* ek . */
    { static obj c[] = { obj_from_case(5) }; cx_tcode_2Drepl = (obj)c; }
    { static obj c[] = { obj_from_case(8) }; cx_main = (obj)c; }
    r[2] = obj_from_void(0);
    r[3+0] = r[0];
    pc = 0; /* exit from module init */
    r[3+1] = r[2];  
    r += 3; /* shift reg wnd */
    rc = 2;
    goto jump;

case 5: /* tcode-repl k */
    assert(rc == 2);
    r += 1; /* shift reg. wnd */
gs_tcode_2Drepl: /* k */
    { /* define decode */
    static obj c[] = { obj_from_objptr(vmcases+3) };
    r[1] = obj_from_objptr(c); }
    hreserve(hbsz(1+1), 2); /* 2 live regs */
    *--hp = r[0];  
    *--hp = obj_from_case(6);
    r[2] = (hendblk(1+1));
    r[3+0] = r[1];  
    pc = objptr_from_obj(r[3+0])[0];
    r[3+1] = r[2];  
    r[3+2] = (cx__2316);
    r += 3; /* shift reg wnd */
    rreserve(MAX_HOSTREGS);
    rc = 3;
    goto jump;

case 6: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* ek r k */
    { /* define make-closure */
    static obj c[] = { obj_from_objptr(vmcases+1) };
    r[3] = obj_from_objptr(c); }
    hreserve(hbsz(1+1), 4); /* 4 live regs */
    *--hp = r[2];  
    *--hp = obj_from_case(7);
    r[4] = (hendblk(1+1));
    r[5+0] = r[3];  
    pc = objptr_from_obj(r[5+0])[0];
    r[5+1] = r[4];  
    r[5+2] = r[1];  
    r += 5; /* shift reg wnd */
    rreserve(MAX_HOSTREGS);
    rc = 3;
    goto jump;

case 7: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* ek r k */
    { /* define execute-thunk-closure */
    static obj c[] = { obj_from_objptr(vmcases+0) }; 
    r[3] = obj_from_objptr(c); }
    r[4+0] = r[3];  
    pc = objptr_from_obj(r[4+0])[0];
    r[4+1] = r[2];  
    r[4+2] = r[1];  
    r += 4; /* shift reg wnd */
    rreserve(MAX_HOSTREGS);
    rc = 3;
    goto jump;

case 8: /* main k argv */
    assert(rc == 3);
    r += 1; /* shift reg. wnd */
gs_main: /* k argv */
    hreserve(hbsz(1+1), 2); /* 2 live regs */
    *--hp = r[0];  
    *--hp = obj_from_case(9);
    r[2] = (hendblk(1+1));
    r[0] = r[2];  
    goto gs_tcode_2Drepl;

case 9: /* clo ek r */
    assert(rc == 3);
    { obj* p = objptr_from_obj(r[0]);
    r[1+2] = p[1]; }
    r += 1; /* shift reg. wnd */
    /* ek r k */
  if (((r[1]) == obj_from_bool(1))) {
    r[0] = r[2];  
    pc = objptr_from_obj(r[0])[0];
    r[1] = obj_from_ktrap();
    r[2] = obj_from_bool(0);
    rreserve(MAX_HOSTREGS);
    rc = 3;
    goto jump;
  } else {
    r[0] = r[2];  
    r[1] = obj_from_bool(0);
    goto gs_main;
  }

default: /* inter-host call */
    cxg_hp = hp;
    cxm_rgc(r, MAX_HOSTREGS);
    cxg_rc = rc;
    return pc;
  }
}

/* module load */
void MODULE(void)
{
  obj pc;
  if (!root.next) {
    root.next = cxg_rootp;
    cxg_rootp = &root;
    LOAD();
    pc = obj_from_case(0);
    cxg_rc = 0;
    while (pc) pc = (*(cxhost_t*)pc)(pc); 
    assert(cxg_rc == 2);
  }
}

/* basic runtime */
#define HEAP_SIZE 131072 /* 2^17 */
#define REGS_SIZE 4092

obj *cxg_heap = NULL;
cxoint_t cxg_hmask = 0;
obj *cxg_hp = NULL;
static cxroot_t cxg_root = { 0, NULL, NULL };
cxroot_t *cxg_rootp = &cxg_root;
obj *cxg_regs = NULL, *cxg_rend = NULL;
int cxg_rc = 0;
char **cxg_argv = NULL;

static obj *cxg_heap2 = NULL;
size_t cxg_hsize = 0;
static cxoint_t cxg_hmask2 = 0;
int cxg_gccount = 0, cxg_bumpcount = 0;

static obj *toheap2(obj* p, obj *hp, obj *h1, cxoint_t m1, obj *h2, cxoint_t m2)
{
  obj o = *p, *op, fo, *fop;
  if (((char*)(o) - (char*)h1) & m1) return hp;
  fo = (op = objptr_from_obj(o))[-1]; assert(fo);
  if (notaptr(fo)) {
    fop = op + size_from_obj(fo); while (fop >= op) *--hp = *--fop;
    *p = *fop = obj_from_objptr(hp+1);
  } else if (((char*)(fo) - (char*)h2) & m2) {
    *--hp = *op--; *--hp = *op;
    *p = *op = obj_from_objptr(hp+1);
  } else *p = fo;
  return hp;
}

static void finalize(obj *hp1, obj *he1, obj *h2, cxoint_t m2)
{
  while (hp1 < he1) {
    obj fo = *hp1++; assert(fo);
    if (notaptr(fo)) hp1 += size_from_obj(fo);
    else if (((char*)(fo) - (char*)h2) & m2) ((cxtype_t*)fo)->free((void*)*hp1++);
    else if (notaptr(fo = objptr_from_obj(fo)[-1])) hp1 += size_from_obj(fo);
    else ++hp1;
  } assert(hp1 == he1);
}

static obj *relocate(cxroot_t *pr, obj *regs, obj *regp, 
obj *he2, obj *he1, obj *hp, obj *h1, cxoint_t m1, obj *h2, cxoint_t m2) 
{
  obj *p, *hp1 = hp; hp = he2;
  for (p = regs; p < regp; ++p) hp = toheap2(p, hp, h1, m1, h2, m2);
  for (; pr; pr = pr->next) {
    obj **pp = pr->globv; int c = pr->globc;
    while (c-- > 0) hp = toheap2(*pp++, hp, h1, m1, h2, m2);
  }
  for (p = he2; p > hp; --p) hp = toheap2(p-1, hp, h1, m1, h2, m2);
  if (he1) finalize(hp1, he1, h2, m2);
  return hp;
}

obj *cxm_hgc(obj *regs, obj *regp, obj *hp, size_t needs) 
{
  obj *h1 = cxg_heap, *h2 = cxg_heap2; cxoint_t m1 = cxg_hmask, m2 = cxg_hmask2;
  size_t hs = cxg_hsize; cxroot_t *pr = cxg_rootp;

  obj *h = h1, *he1 = h1 + hs, *he2 = h2 + hs; 
  ++cxg_gccount;
  if (h1) hp = relocate(pr, regs, regp, he2, he1, hp, h1, m1, h2, m2), 
    needs += (h2 + hs - hp)*2; /* make heap half empty */
  else hp = h2 + hs;
  if (hs < needs) {
    size_t s = HEAP_SIZE; while (s < needs) s *= 2;
    m2 = 1 | ~(s*sizeof(obj)-1);
    if (!(h = realloc(h1, s*sizeof(obj)))) { perror("alloc[h]"); exit(2); }
    h1 = h2; h2 = h; he2 = h2 + s; he1 = 0; /* no finalize flag */
    if (h1) hp = relocate(pr, regs, regp, he2, he1, hp, h1, m1, h2, m2);
    else hp = h2 + s;
    if (!(h = realloc(h1, s*sizeof(obj)))) { perror("alloc[h]"); exit(2); }
    hs = s; m1 = m2; ++cxg_bumpcount;
  }
  h1 = h2; h2 = h;

  cxg_heap = h1; cxg_hmask = m1; cxg_heap2 = h2; cxg_hmask2 = m2;
  cxg_hsize = hs; return cxg_hp = hp;
}

obj *cxm_rgc(obj *regs, size_t needs) 
{
  obj *p = cxg_regs; assert(needs > 0);
  if (!p || cxg_rend < p + needs) {
    size_t roff = regs ? regs - p : 0;
    if (!(p = realloc(p, needs*sizeof(obj)))) { perror("alloc[r]"); exit(2); }
    cxg_regs = p; cxg_rend = p + needs;
    regs = p + roff;
  }
  if (regs && regs > p) while (needs--) *p++ = *regs++;
  return cxg_regs;
}

void cxm_check(int x, char *msg)
{
  if (!x) { 
    perror(msg); exit(2); 
  }
}

void *cxm_cknull(void *p, char *msg)
{
  cxm_check(p != NULL, msg); 
  return p;
}

/* os entry point */
int main(int argc, char **argv) {
  int res; obj pc;
  obj retcl[1] = { 0 };
  cxm_rgc(NULL, REGS_SIZE);
  cxg_argv = argv;
  MODULE();
  cxg_regs[0] = cx_main;
  cxg_regs[1] = (obj)retcl;
  cxg_regs[2] = (obj)argv;
  cxg_rc = 3;
  pc = objptr_from_obj(cx_main)[0];
  while (pc) pc = (*(cxhost_t*)pc)(pc); 
  assert(cxg_rc == 3);
  res = (cxg_regs[2] != 0); 
  return res; 
}
