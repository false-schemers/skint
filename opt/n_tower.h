/* n_tower.h -- numerical tower interfaces */

/* big/fat numbers */
#define NUMT_BIG  (3)
#define NUMT_MKRAT(nt, dt) ((dt)<<2|(nt))   /* if denominator is NONE, bit-compatible with integer */
#define NUMT_RAT_N(rt)((rt) & 3)            /* numerator type part of the rational type */
#define NUMT_RAT_D(rt)(((rt)>>2) & 3)       /* denominator type part of the rational type */
#define NUMT_MKCOM(rt, it) ((it)<<4|(rt))   /* if imag. part type is NONE, bit-compatible with real */
#define NUMT_COM_R(ct)((ct) & 15)           /* real type part of the complex type */
#define NUMT_COM_I(ct)(((ct)>>4) & 15)      /* imaginary type part of the rational type */
#define NUMT_SS_MASK  (3)                   /* types of single-slot numbers (or parts) */
#define NUMT_DS_MASK  (15)                  /* types of double-slot numbers (or parts) */

/* numerical comparisons (needed for inexacts) */
typedef enum { NCMP_LT, NCMP_LE, NCMP_EQ, NCMP_GE, NCMP_GT } ncmp_t;

/* 'generic' writer for flonums; returns 0 or -1 on invalid radix */
extern int wrdn(double n, int radix, int mode, int prc, int (*pf)(int, void*), void *pd);

/* bignums (avp) */
typedef struct bignum bignum_t;

extern bignum_t *lltobn(int64_t n);
extern bignum_t *ulltobn(uint64_t n);
extern double bntod(const bignum_t *n);
extern long bntol(const bignum_t *n);
extern int64_t bntoll(const bignum_t *n);
extern uint64_t bntoull(const bignum_t *n);
/* # of bits needed for 2-complement signed representation of n */
extern size_t bnwidths(const bignum_t *n);
/* # of bits needed for unsigned representation of |n| */
extern size_t bnwidthu(const bignum_t *n);
extern bignum_t *bngcd(const bignum_t *x, const bignum_t *y);
extern long bnmodl(const bignum_t *num, long den);
extern bignum_t *bnmod(const bignum_t *num, const bignum_t *den);
extern bignum_t *bndivl(const bignum_t *num, long den);
extern bignum_t *bndiv(const bignum_t *num, const bignum_t *den);
extern bignum_t *bndmodl(long *rem, const bignum_t *num, long den);
extern bignum_t *bndmod(bignum_t **rem, const bignum_t *num, const bignum_t *den);
extern bignum_t *bnexptull(const bignum_t *a, uint64_t n);
extern bignum_t *bnmulll(const bignum_t *n, int64_t v);
extern bignum_t *bnmul(const bignum_t *a, const bignum_t *b);
extern bignum_t *bnashll(const bignum_t *a, int64_t sh); /* shifts right if sh < 0 */
extern bignum_t *bnsub(const bignum_t *a, const bignum_t *b);
extern bignum_t *bnaddll(const bignum_t *n, int64_t incr);
extern bignum_t *bnadd(const bignum_t *a, const bignum_t *b);
extern int bnodd(const bignum_t *a); /* lsb bit is 1 */
extern int bneven(const bignum_t *a); /* lsb bit is 0 */
extern int bneq(const bignum_t *a, const bignum_t *b); /* a == b ? */
extern int bncmpll(const bignum_t *n, int64_t fix); /* returns -1,0,1 */
extern int bncmpabs(const bignum_t *a, const bignum_t *b); /* compares |a| and |b|, returns -1,0,1 */
extern int bncmp(const bignum_t *a, const bignum_t *b); /* returns -1,0,1 */
extern int bnzero(const bignum_t *a); /* a == 0 ? */
extern int bnsign(const bignum_t *a); /* returns -1,0,1 */
extern bignum_t *bnabs(const bignum_t *n); /* |n| */
extern bignum_t *bnneg(const bignum_t *n); /* -n */
extern bignum_t *bnbitnot(const bignum_t *n);
extern bignum_t *bnbitand(const bignum_t *a, const bignum_t *b);
extern bignum_t *bnbitior(const bignum_t *a, const bignum_t *b);
extern bignum_t *bnbitxor(const bignum_t *a, const bignum_t *b);
extern bignum_t *bnbitash(const bignum_t *a, long cnt);
extern long bnintlen(const bignum_t *n);
extern long bnbitc(const bignum_t *n);
extern void bnfree(bignum_t *n);

/* 'generic' writer for bignums; returns 0 or -1 on invalid radix */
extern int wrbn(const bignum_t *n, int radix, int (*pf)(int, void*), void *pd);
extern bignum_t *strtobn(const char *str, char **endptr, int radix);

extern cxtype_t *BIGNUM_NTAG;
#define is_bignum_obj(o) (isnative(o, BIGNUM_NTAG))
#define bignum_from_obj(o) ((bignum_t*)getnative(o, BIGNUM_NTAG))
#define obj_from_bignum(l, b) hpushptr(b, BIGNUM_NTAG, l)


/* fatnums -- compound tower numbers (ratnums, compnums, rectnums) */
/* make sure fatnum_t is compatible with fatnum4_t! */
typedef struct fatnum { numt_t t; nump_t p[1]; } fatnum_t; /* alloc 1..4 parts */
/* result struct with space for error message, compatible with fatnum_t */
typedef struct fatnum4r { numt_t t; union { nump_t p[4]; char msg[80]; } u; } fatnum4r_t; 

/* return values: 1 = true, 0 = false */
extern int fneqn(const fatnum_t *fx, const fatnum_t *fy);
extern int fneqv(const fatnum_t *fx, const fatnum_t *fy);
extern int fncmp(const fatnum_t *fx, const fatnum_t *fy, ncmp_t c);
extern int fnodd(const fatnum_t *fx);
extern int fneven(const fatnum_t *fx);
extern int fnisex(const fatnum_t *fx);
extern int fnisin(const fatnum_t *fx);
extern int fnzero(const fatnum_t *fx);
extern int fnispos(const fatnum_t *fx);
extern int fnisneg(const fatnum_t *fx);
extern int fnisint(const fatnum_t *fx);
extern int fnisrat(const fatnum_t *fx);
extern int fnisreal(const fatnum_t *fx);
extern int fnisran(const fatnum_t *fx);
extern int fniscon(const fatnum_t *fx);
extern int fnisren(const fatnum_t *fx);
extern int fnisfin(const fatnum_t *fx);
extern int fnisinf(const fatnum_t *fx);
extern int fnisnan(const fatnum_t *fx);
/* return values: 1 = success, 0 = domain/range error */
extern int fnabs(fatnum4r_t *fz, const fatnum_t *fx);
extern int fnneg(fatnum4r_t *fz, const fatnum_t *fx);
extern int fntoex(fatnum4r_t *fz, const fatnum_t *fx);
extern int fntoin(fatnum4r_t *fz, const fatnum_t *fx);
extern int fnfloor(fatnum4r_t *fz, const fatnum_t *fx);
extern int fnceil(fatnum4r_t *fz, const fatnum_t *fx);
extern int fntrunc(fatnum4r_t *fz, const fatnum_t *fx);
extern int fnround(fatnum4r_t *fz, const fatnum_t *fx);
extern int fnnumer(fatnum4r_t *fz, const fatnum_t *fx);
extern int fndenom(fatnum4r_t *fz, const fatnum_t *fx);
extern int fnrpart(fatnum4r_t *fz, const fatnum_t *fx);
extern int fnipart(fatnum4r_t *fz, const fatnum_t *fx);
extern int fnmagn(fatnum4r_t *fz, const fatnum_t *fx);
extern int fnangl(fatnum4r_t *fz, const fatnum_t *fx);
extern int fnexp(fatnum4r_t *fz, const fatnum_t *fx);
extern int fnlog(fatnum4r_t *fz, const fatnum_t *fx);
extern int fnsin(fatnum4r_t *fz, const fatnum_t *fx);
extern int fncos(fatnum4r_t *fz, const fatnum_t *fx);
extern int fntan(fatnum4r_t *fz, const fatnum_t *fx);
extern int fnasin(fatnum4r_t *fz, const fatnum_t *fx);
extern int fnacos(fatnum4r_t *fz, const fatnum_t *fx);
extern int fnatan(fatnum4r_t *fz, const fatnum_t *fx);
extern int fnsqrt(fatnum4r_t *fz, const fatnum_t *fx);
extern int fnisqrt(fatnum4r_t *fz, fatnum4r_t *fr, const fatnum_t *fx);
extern int fnmax(fatnum4r_t *fz, const fatnum_t *fx, const fatnum_t *fy);
extern int fnmin(fatnum4r_t *fz, const fatnum_t *fx, const fatnum_t *fy);
extern int fnadd(fatnum4r_t *fz, const fatnum_t *fx, const fatnum_t *fy);
extern int fnsub(fatnum4r_t *fz, const fatnum_t *fx, const fatnum_t *fy);
extern int fnmul(fatnum4r_t *fz, const fatnum_t *fx, const fatnum_t *fy);
extern int fndiv(fatnum4r_t *fz, const fatnum_t *fx, const fatnum_t *fy);
extern int fngcd(fatnum4r_t *fz, const fatnum_t *fx, const fatnum_t *fy);
extern int fnpow(fatnum4r_t *fz, const fatnum_t *fx, const fatnum_t *fy);
extern int fnquo(fatnum4r_t *fz, const fatnum_t *fx, const fatnum_t *fy);
extern int fnrem(fatnum4r_t *fz, const fatnum_t *fx, const fatnum_t *fy);
extern int fnmqu(fatnum4r_t *fz, const fatnum_t *fx, const fatnum_t *fy);
extern int fnmlo(fatnum4r_t *fz, const fatnum_t *fx, const fatnum_t *fy);
extern int fnlogn(fatnum4r_t *fz, const fatnum_t *fx, const fatnum_t *fy);
extern int fnatan2(fatnum4r_t *fz, const fatnum_t *fy, const fatnum_t *fx);
extern int fnmkrec(fatnum4r_t *fz, const fatnum_t *fx, const fatnum_t *fy);
extern int fnmkpol(fatnum4r_t *fz, const fatnum_t *fx, const fatnum_t *fy);
/* bitwise ops; return values: 1 = success, 0 = domain/range error */
extern int fnnot(fatnum4r_t *fz, const fatnum_t *fx);
extern int fnand(fatnum4r_t *fz, const fatnum_t *fx, const fatnum_t *fy);
extern int fnior(fatnum4r_t *fz, const fatnum_t *fx, const fatnum_t *fy);
extern int fnxor(fatnum4r_t *fz, const fatnum_t *fx, const fatnum_t *fy);
extern int fnash(fatnum4r_t *fz, const fatnum_t *fx, const fatnum_t *fy);
extern int fnlen(fatnum4r_t *fz, const fatnum_t *fx);
extern int fnbtc(fatnum4r_t *fz, const fatnum_t *fx);


/* 'generic' writer for fatnums; returns 0 or -1 on invalid radix */
extern int wrfn(const fatnum_t *n, int radix, int mode, int prc, int (*pf)(int, void*), void *pd);

extern cxtype_t *FATNUM_NTAG;
#define is_fatnum_obj(o) (isnative(o, FATNUM_NTAG))
#define fatnum_from_obj(o) ((fatnum_t*)getnative(o, FATNUM_NTAG))
#define obj_from_fatnum(l, f) hpushptr(f, FATNUM_NTAG, l)
#define fntype(o) (fatnum_from_obj(o)->type)
extern fatnum_t *dupfatnum(fatnum_t *fn); /* shallow copy! */
extern void fnfini(fatnum_t *fx);
