/* k.h -- kernel interfaces: vm registers and vm entry points */

#ifndef SKINT_S_H_SEEN
#error "s.h must be included before k.h -- feature-test macros are selected there"
#endif

#ifndef SKINT_K_H_SEEN
#define SKINT_K_H_SEEN 1

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

/* vm entry points for hand-coded kernel code; each of them protects
 * registers from r to sp, takes and returns hp, and passes values
 * in and out via ra -- see vm_* functions in i.c */
extern obj *vm_execute_thunk_closure(obj *r, obj *sp, obj *hp); /* in: ra=closure; out: ra=result */
extern obj *vm_make_closure(obj *r, obj *sp, obj *hp);          /* in: ra=code; out: ra=closure */
extern obj *vm_decode(obj *r, obj *sp, obj *hp);                /* in: ra=string; out: ra=code */
extern obj *vm_decode_sexp(obj *r, obj *sp, obj *hp);           /* in: ra=string; out: ra=sexp */
extern obj *vm_install_global_lambdas(obj *r, obj *sp, obj *hp); /* no args/values; returns new hp */
extern obj *vm_initialize_modules(obj *r, obj *sp, obj *hp);     /* no args/values; returns new hp */

#endif /* SKINT_K_H_SEEN */
