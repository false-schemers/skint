/* n_tower.h -- numerical tower interfaces */

/* bignums (avp) */
typedef struct bignum bignum_t;

//extern bignum_t *ltobn(long n);
//extern bignum_t *lltobn(long long n);
extern double bntod(const bignum_t *n);
extern long bntol(const bignum_t n);
/* # of bits needed for 2-complement signed representation of n */
extern size_t bnwidths(const bignum_t n);
/* # of bits needed for unsigned representation of |n| */
extern size_t bnwidthu(const bignum_t *n);
extern bignum_t *bngcd(const bignum_t *x, const bignum_t *y);
extern long bnmodl(const bignum_t *num, long den);
extern bignum_t *bnmod(const bignum_t *num, const bignum_t *den);
extern bignum_t *bndivl(const bignum_t *num, long den);
extern bignum_t *bndiv(const bignum_t *num, const bignum_t *den);
extern bignum_t *bndmodl(long *rem, const bignum_t *num, long den);
extern bignum_t *bndmod(bignum_t **rem, const bignum_t *num, const bignum_t *den);
extern bignum_t *bnexpt(const bignum_t *a, unsigned long n);
extern bignum_t *bnmull(const bignum_t *n, long v);
extern bignum_t *bnmul(const bignum_t *a, const bignum_t *b);
extern bignum_t *bnashl(const bignum_t a, long sh); /* shifts right if sh < 0 */
extern bignum_t *bnsub(const bignum_t *a, const bignum_t *b);
extern bignum_t *bnaddl(const bignum_t *n, long incr);
extern bignum_t *bnadd(const bignum_t *a, const bignum_t *b);
extern int bnodd(const bignum_t a); /* lsb bit is 1 */
extern int bneven(const bignum_t a); /* lsb bit is 0 */
extern int bneq(const bignum_t *a, const bignum_t b); /* a == b ? */
extern int bncmpl(const bignum_t n, long fix); /* returns -1,0,1 */
extern int bncmpabs(const bignum_t *a, const bignum_t *b); /* compares |a| and |b|, returns -1,0,1 */
extern int bncmp(const bignum_t *a, const bignum_t b); /* returns -1,0,1 */
extern int bnzero(const bignum_t a); /* a == 0 ? */
extern int bnsign(const bignum_t a); /* returns -1,0,1 */
extern bignum_t *bnabs(const bignum_t n); /* |n| */
extern bignum_t *bnneg(const bignum_t n); /* -n */
extern void bnfree(bignum_t *n);

extern cxtype_t *BIGNUM_NTAG;
#define is_bignum_obj(o) (isnative(o, BIGNUM_NTAG))
#define bignum_from_obj(o) ((bignum_t*)getnative(o, BIGNUM_NTAG))
#define obj_from_bignum(l, b) hpushptr(b, BIGNUM_NTAG, l)

/* compound numbers */


