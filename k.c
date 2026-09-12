/* k.c -- kernel: gc runtime, kernel globals, startup */

/* This file used to be generated from pre/k.sf by the sfc compiler and
 * post-processed by pre/ksf2c.ssc; it is hand-coded now. The startup code
 * below is a direct analog of what k.sf contained, and the basic runtime
 * at the end of this file (the copying gc) is still sfc's, kept as-is. */

#include "s.h"
#include "n.h"
#include "k.h"

/* kernel globals (imported by i.c) */
obj cx_current_error;
obj cx_current_input;
obj cx_current_output;
obj cx_dynamic_state;
obj cx_global_store;
obj cx_tansformers;
obj cx_callmv_adapter_closure;
obj cx_continuation_adapter_code;

/* gc roots */
static obj *globv[] = {
  &cx_current_error,
  &cx_current_input,
  &cx_current_output,
  &cx_dynamic_state,
  &cx_global_store,
  &cx_tansformers,
  &cx_callmv_adapter_closure,
  &cx_continuation_adapter_code,
};

static cxroot_t root = {
  sizeof(globv)/sizeof(obj *), globv, NULL
};

/* startup */

/* serialized code decoded at startup (cf. k.sf) */
static const char callmv_adapter_code[] = "K5";
static const char repl_code[] = "${@(y4:repl)[00}";

/* protects registers from r to sp; no args/values; returns new hp */
static obj *init_kernel_globals(obj *r, obj *sp, obj *hp)
{
  { /* (define *globals* (make-vector 991 '())) */
  obj o; int i = 0, c = 991;
  hreserve(hbsz(c+1), sp-r);
  o = mknull(); /* gc-safe */
  while (i++ < c) *--hp = o;
  *--hp = obj_from_size(VECTOR_BTAG);
  cx_global_store = hendblk(c+1); }
  { /* (define *dynamic-state* (cons #f '())) */
  hreserve(hbsz(3), sp-r);
  *--hp = mknull();
  *--hp = obj_from_bool(0);
  *--hp = obj_from_size(PAIR_BTAG);
  cx_dynamic_state = hendblk(3); }
  cx_current_input = obj_from_bool(0);
  cx_current_output = obj_from_bool(0);
  cx_current_error = obj_from_bool(0);
  cx_tansformers = mknull();
  cx_continuation_adapter_code = obj_from_bool(0);
  return hp;
}

/* protects registers from r to sp, in: ra=string, out: ra=closure;
 * this is k.sf's (make-closure (decode str)) */
static obj *decode_closure(obj *r, obj *sp, obj *hp)
{
  hp = vm_decode(r, sp, hp);       /* ra=string => ra=code */
  hp = vm_make_closure(r, sp, hp); /* ra=code => ra=closure */
  return hp;
}

/* protects registers from r to sp; no args/values; returns new hp */
static obj *run_kernel(obj *r, obj *sp, obj *hp)
{
  hp = init_kernel_globals(r, sp, hp);
  /* (define callmv-adapter-closure (make-closure (decode "K5"))) */
  ra = hpushstr(sp-r, newsdata(callmv_adapter_code));
  hp = decode_closure(r, sp, hp);
  cx_callmv_adapter_closure = ra;
  /* (install-global-lambdas) */
  hp = vm_install_global_lambdas(r, sp, hp);
  /* (initialize-modules) */
  hp = vm_initialize_modules(r, sp, hp);
  /* (define (main) (if (eq? (tcode-repl) #t) #f (main))) -- the repl
   * returns #t when it is done, anything else on an error exit */
  do { /* (define (tcode-repl) (execute-thunk-closure ...)) */
    ra = hpushstr(sp-r, newsdata(repl_code));
    hp = decode_closure(r, sp, hp);
    hp = vm_execute_thunk_closure(r, sp, hp); /* ra=closure => ra=result */
  } while (ra != obj_from_bool(1));
  return hp;
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
  if (((cxoint_t)(o) - (cxoint_t)h1) & m1) return hp;
  fo = (op = objptr_from_obj(o))[-1]; assert(fo);
  if (notaptr(fo)) {
    fop = op + size_from_obj(fo); while (fop >= op) *--hp = *--fop;
    *p = *fop = obj_from_objptr(hp+1);
  } else if (((cxoint_t)(fo) - (cxoint_t)h2) & m2) {
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
    m2 = 1 | ~((cxoint_t)s*sizeof(obj)-1);
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
int main(int argc, char **argv)
{
  obj *r, *sp, *hp; int i;
  cxg_argv = argv;
  /* the register file is allocated at its final size once and never
   * reallocated, so r stays valid: the vm uses its tail as a stack */
  r = cxm_rgc(NULL, VM_REGC + VM_STACK_LEN);
  for (i = 0; i < VM_REGC; ++i) r[i] = 0; /* all of them are gc-scanned */
  rz = (obj)(r + VM_STACK_GSZ); /* sp red zone */
  sp = r + VM_REGC;
  root.next = cxg_rootp; cxg_rootp = &root;
  hp = cxg_hp; /* NULL: the first hreserve() allocates the heap */
  hp = run_kernel(r, sp, hp);
  cxg_hp = hp;
  return 0;
}
