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

/* bignums (avp) */
typedef struct bignum bignum_t;

extern bignum_t *lltobn(long long n);
extern double bntod(const bignum_t *n);
extern long bntol(const bignum_t *n);
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
extern bignum_t *bnexpt(const bignum_t *a, unsigned long long n);
extern bignum_t *bnmull(const bignum_t *n, long long v);
extern bignum_t *bnmul(const bignum_t *a, const bignum_t *b);
extern bignum_t *bnashl(const bignum_t *a, long long sh); /* shifts right if sh < 0 */
extern bignum_t *bnsub(const bignum_t *a, const bignum_t *b);
extern bignum_t *bnaddl(const bignum_t *n, long long incr);
extern bignum_t *bnadd(const bignum_t *a, const bignum_t *b);
extern int bnodd(const bignum_t *a); /* lsb bit is 1 */
extern int bneven(const bignum_t *a); /* lsb bit is 0 */
extern int bneq(const bignum_t *a, const bignum_t *b); /* a == b ? */
extern int bncmpl(const bignum_t *n, long long fix); /* returns -1,0,1 */
extern int bncmpabs(const bignum_t *a, const bignum_t *b); /* compares |a| and |b|, returns -1,0,1 */
extern int bncmp(const bignum_t *a, const bignum_t *b); /* returns -1,0,1 */
extern int bnzero(const bignum_t *a); /* a == 0 ? */
extern int bnsign(const bignum_t *a); /* returns -1,0,1 */
extern bignum_t *bnabs(const bignum_t *n); /* |n| */
extern bignum_t *bnneg(const bignum_t *n); /* -n */
extern void bnfree(bignum_t *n);

extern cxtype_t *BIGNUM_NTAG;
#define is_bignum_obj(o) (isnative(o, BIGNUM_NTAG))
#define bignum_from_obj(o) ((bignum_t*)getnative(o, BIGNUM_NTAG))
#define obj_from_bignum(l, b) hpushptr(b, BIGNUM_NTAG, l)

/* fatnums -- compound tower numbers (ratnums, compnums, rectnums) */
typedef struct fatnum {
  numt_t t;
  nump_t p[1]; /* alloc 1..4 parts */
} fatnum_t;

/* return values for predicates: 1 = true, 0 = false */
/* non-predicates: 1 = success, 0 = domain/range error */
extern int fneqn(const fatnum_t *fx, const fatnum_t *fy);
extern int fneqv(const fatnum_t *fx, const fatnum_t *fy);
extern int fnless(const fatnum_t *fx, const fatnum_t *fy);
extern int fncmp(const fatnum_t *fx, const fatnum_t *fy, ncmp_t c);
extern int fnisodd(const fatnum_t *fx);
extern int fniseven(const fatnum_t *fx);
extern int fniszero(const fatnum_t *fx);
extern int fnispos(const fatnum_t *fx);
extern int fnisneg(const fatnum_t *fx);
extern int fnisex(const fatnum_t *fx);
extern int fnisinex(const fatnum_t *fx);
extern int fnisint(const fatnum_t *fx);
extern int fnisrat(const fatnum_t *fx);
extern int fnisreal(const fatnum_t *fx);
extern int fniscomp(const fatnum_t *fx);
extern int fnisfin(const fatnum_t *fx);
extern int fnisinf(const fatnum_t *fx);
extern int fnisnan(const fatnum_t *fx);
extern int fnabs(fatnum_t *fz, const fatnum_t *fx);
extern int fnneg(fatnum_t *fz, const fatnum_t *fx);
extern int fntoe(fatnum_t *fz, const fatnum_t *fx);
extern int fntoi(fatnum_t *fz, const fatnum_t *fx);
extern int fnfloor(fatnum_t *fz, const fatnum_t *fx);
extern int fnceil(fatnum_t *fz, const fatnum_t *fx);
extern int fntrunc(fatnum_t *fz, const fatnum_t *fx);
extern int fnround(fatnum_t *fz, const fatnum_t *fx);
extern int fnnumer(fatnum_t *fz, const fatnum_t *fx);
extern int fndenom(fatnum_t *fz, const fatnum_t *fx);
extern int fnreal(fatnum_t *fz, const fatnum_t *fx);
extern int fnimag(fatnum_t *fz, const fatnum_t *fx);
extern int fnmagn(fatnum_t *fz, const fatnum_t *fx);
extern int fnangle(fatnum_t *fz, const fatnum_t *fx);
extern int fnexp(fatnum_t *fz, const fatnum_t *fx);
extern int fnlog(fatnum_t *fz, const fatnum_t *fx);
extern int fnsin(fatnum_t *fz, const fatnum_t *fx);
extern int fncos(fatnum_t *fz, const fatnum_t *fx);
extern int fntan(fatnum_t *fz, const fatnum_t *fx);
extern int fnasin(fatnum_t *fz, const fatnum_t *fx);
extern int fnacos(fatnum_t *fz, const fatnum_t *fx);
extern int fnatan(fatnum_t *fz, const fatnum_t *fx);
extern int fnsqrt(fatnum_t *fz, const fatnum_t *fx);
extern int fnmax(fatnum_t *fz, const fatnum_t *fx, const fatnum_t *fy);
extern int fnmin(fatnum_t *fz, const fatnum_t *fx, const fatnum_t *fy);
extern int fnadd(fatnum_t *fz, const fatnum_t *fx, const fatnum_t *fy);
extern int fnsub(fatnum_t *fz, const fatnum_t *fx, const fatnum_t *fy);
extern int fnmul(fatnum_t *fz, const fatnum_t *fx, const fatnum_t *fy);
extern int fndiv(fatnum_t *fz, const fatnum_t *fx, const fatnum_t *fy);
extern int fngcd(fatnum_t *fz, const fatnum_t *fx, const fatnum_t *fy);
extern int fntquo(fatnum_t *fz, const fatnum_t *fx, const fatnum_t *fy);
extern int fntrem(fatnum_t *fz, const fatnum_t *fx, const fatnum_t *fy);
extern int fnfquo(fatnum_t *fz, const fatnum_t *fx, const fatnum_t *fy);
extern int fnfrem(fatnum_t *fz, const fatnum_t *fx, const fatnum_t *fy);
extern int fnlogn(fatnum_t *fz, const fatnum_t *fx, const fatnum_t *fy);
extern int fnexpt(fatnum_t *fz, const fatnum_t *fx, const fatnum_t *fy);
extern int fnatan2(fatnum_t *fz, const fatnum_t *fy, const fatnum_t *fx);
extern int fnmaker(fatnum_t *fz, const fatnum_t *fx, const fatnum_t *fy);
extern int fnmakep(fatnum_t *fz, const fatnum_t *fx, const fatnum_t *fy);
extern int fnsqrti(fatnum_t *fq, fatnum_t *fr, const fatnum_t *fx);

extern cxtype_t *FATNUM_NTAG;
#define is_fatnum_obj(o) (isnative(o, FATNUM_NTAG))
#define fatnum_from_obj(o) ((fatnum_t*)getnative(o, FATNUM_NTAG))
#define obj_from_fatnum(l, f) hpushptr(f, BIGNUM_NTAG, l)
#define fntype(o) (fatnum_from_obj(o)->type)

