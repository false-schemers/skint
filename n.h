/* n.h -- native/platform interfaces */

#ifndef SKINT_S_H_SEEN
#error "s.h must be included before n.h -- feature-test macros are selected there"
#endif

#if defined(__clang__)
#pragma GCC diagnostic ignored "-Wparentheses-equality"
#pragma GCC diagnostic ignored "-Wignored-attributes"
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wunused-value"
#pragma GCC diagnostic ignored "-Woverlength-strings"
#pragma GCC diagnostic ignored "-Wunused-variable"
#if __has_warning("-Wunused-but-set-variable")
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#endif
#endif
/* this is for MS headers; shouldn't affect others */
#define _CRT_SECURE_NO_WARNINGS 1

/* standard includes */
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <assert.h>
/* extra includes */
#include <stdint.h>
#include <math.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>
#include <float.h>
#include <limits.h>
#include <time.h>
/* old lib patches */
#if defined(_MSC_VER) && (_MSC_VER <= 1700)
#define strtoll  _strtoi64
#define strtoull _strtoui64
#define PRId64 "I64d"
#define PRIu64 "I64u"
#else
#include <inttypes.h>
#endif
#ifndef DBL_DECIMAL_DIG
#ifdef DECIMAL_DIG
#define DBL_DECIMAL_DIG DECIMAL_DIG
#else
#define DBL_DECIMAL_DIG 17
#endif
#endif
#ifndef FLT_DECIMAL_DIG
#define FLT_DECIMAL_DIG 9
#endif
/* useful math constants */
#ifndef M_PI
#define M_PI 3.141592653589793238462643383279502884
#endif
#ifndef M_PI_2
#define M_PI_2 1.570796326794896619231321691639751442
#endif
#ifndef M_PI_4
#define M_PI_4 0.785398163397448309615660845819875721
#endif
#ifndef M_SQRT2
#define M_SQRT2 1.414213562373095048801688724209698079
#endif
#ifndef M_SQRT1_2
#define M_SQRT1_2 0.707106781186547524400844362104849039
#endif
#ifndef M_LN2
#define M_LN2 0.693147180559945309417232121458176568
#endif
#ifndef M_PI_LN2
#define M_PI_LN2 4.53236014182719380962768294571666681
#endif
#ifndef M_PI_LN10
#define M_PI_LN10 1.36437635384184134748578362543135577
#endif
#ifndef M_LOG2E
#define M_LOG2E 1.442695040888963407359924681001892137
#endif

/* standard definitions */
#ifdef NAN_BOXING
typedef int64_t obj;          /* pointers are this size, higher 16 bits and lower bit zero */
typedef int64_t cxoint_t;     /* same thing, used as integer */
typedef struct {              /* type descriptor */
  const char *tname;          /* name (debug) */
  void (*free)(void*);        /* deallocator */
} cxtype_t;

#define notobjptr(o)          (((cxoint_t)(o) - (cxoint_t)cxg_heap_plus1) & cxg_hmask)
#define isobjptr(o)           (!notobjptr(o))
#define notaptr(o)            ((o) & 0xffff000000000001ULL)
#define isaptr(o)             (!notaptr(o))
#else
typedef ptrdiff_t obj;        /* pointers are this size, lower bit zero */
typedef ptrdiff_t cxoint_t;   /* same thing, used as integer */
typedef struct {              /* type descriptor */
  const char *tname;          /* name (debug) */
  void (*free)(void*);        /* deallocator */
} cxtype_t;

#define notobjptr(o)          (((char*)(o) - (char*)cxg_heap_plus1) & cxg_hmask)
#define isobjptr(o)           (!notobjptr(o))
#define notaptr(o)            ((o) & 1)
#define isaptr(o)             (!notaptr(o))
#endif

#define obj_from_objptr(p)    ((obj)(p))

/* block header cells carry block size and a 2-bit microtag (MTAG) */
#define TYPED_MTAG            0 /* cell 0 is rtd / #f for tuples */
#define PACKED_MTAG           1 /* size defines type: 1=box, 2=pair */
#define CLOSURE_MTAG          2 /* closures with code vec in cell 0 */
#define VECTOR_MTAG           3 /* regular vectors, including #() */
#define obj_from_sztag(n, m)  (((cxoint_t)(n) << 3) | ((m) << 1) | 1)
#define obj_from_packed(n)    obj_from_sztag(n, PACKED_MTAG)
#define size_from_obj(o)      ((int)((cxoint_t)(o) >> 3))
#define mtag_from_obj(o)      ((int)(((cxoint_t)(o) >> 1) & 3))
#define obj_is_blkhdr(o, m)   ((((cxoint_t)(o)) & 7) == (((m) << 1) | 1))

/* the vocabulary the kinds below are written in */
#define blkhdr(o)             (objptr_from_obj(o)[-1])
#define is_tagged(o, m)       (isobjptr(o) && obj_is_blkhdr(blkhdr(o), m))
#define is_packed(o, n)       (isobjptr(o) && blkhdr(o) == obj_from_packed(n))
#define hend_tagged(n, m)     (*--hp = obj_from_sztag(n, m), (obj)(hp+1))
#define hend_packed(n)        hend_tagged(n, PACKED_MTAG)
#define packed_bsz(n)         block_bsz(n)
#define packed_ref(o, i)      block_ref(o, i)

#define objptr_from_obj(o)    ((obj*)(o))
#define get_bool(o)           (o)

#define hpushptr(p, pt, l)    (hreserve(2, l), *--hp = (obj)(p), *--hp = (obj)(pt), (obj)(hp+1))   
#define block_bsz(s)          ((s) + 1) /* 1 extra word to store block size and mtag */
#define hreserve(n, l)        ((hp < cxg_heap + (n)) ? hp = cxm_hgc(r, r+(l), hp, n) : hp)
#define block_len(p)          size_from_obj(((obj*)(p))[-1])
#define block_ref(p, i)       (((obj*)(p))[i])

typedef struct cxroot_tag {
  int globc; obj **globv;
  struct cxroot_tag *next;
} cxroot_t;

extern obj *cxg_heap, *cxg_heap_plus1;
extern obj *cxg_hp;
extern cxoint_t cxg_hmask;
extern cxroot_t *cxg_rootp;
extern obj *cxm_rgc(obj *regs, size_t needs);
extern obj *cxm_hgc(obj *regs, obj *regp, obj *hp, size_t needs);
extern obj *cxg_regs, *cxg_rend;
extern void cxm_check(int x, char *msg);
extern void *cxm_cknull(void *p, char *msg);
extern int cxg_rc;
extern char **cxg_argv;

/* extra definitions */
/* basic object representation */
#ifdef NAN_BOXING
#define isim0(o)    (((o) & 0xffffffff00000003ULL) == 3) /* 30 bits of payload */
#define isimm(o, t) (((o) & 0xffffffff000000ffULL) == (((t) << 2) | 1)) /* 24 */
#ifdef NDEBUG
  #define getim0s(o) (long)(((((int32_t)(o) >> 2) & 0x3fffffff) ^ 0x20000000) - 0x20000000)
  #define getimmu(o, t) (long)(((o) >> 8) & 0xffffff)
#else
  extern long getim0s(obj o);
  extern long getimmu(obj o, int t);
#endif
#define mkim0(v) ((obj)((((v) & 0x000000003fffffffULL) << 2) | 3))
#define mkimm(v, t) ((obj)((((v) & 0x0000000000ffffffULL) << 8) | ((t) << 2) | 1))
#else
#define isim0(o)    (((o) & 3) == 3)
#define isimm(o, t) (((o) & 0xff) == (((t) << 2) | 1))
#ifdef NDEBUG
  #define getim0s(o) (long)(((((int)(o) >> 2) & 0x3fffffff) ^ 0x20000000) - 0x20000000)
  #define getimmu(o, t) (long)(((o) >> 8) & 0xffffff)
#else
  extern long getim0s(obj o);
  extern long getimmu(obj o, int t);
#endif
#define mkim0(o) (obj)((((o) & 0x3fffffff) << 2) | 3)
#define mkimm(o, t) (obj)((((o) & 0xffffff) << 8) | ((t) << 2) | 1)
#define FLONUMS_BOXED
#endif
#ifdef NDEBUG
   static int is_native(obj o, cxtype_t *tp) 
     { return isobjptr(o) && objptr_from_obj(o)[-1] == (obj)tp; }
   #define get_native(o, t) ((void*)(*objptr_from_obj(o)))
   static void set_native(obj o, cxtype_t *tp, void *v) 
     { *objptr_from_obj(o) = (obj)v; }
#else
  extern int is_native(obj o, cxtype_t *tp);
  extern void *get_native(obj o, cxtype_t *tp);
  extern void set_native(obj o, cxtype_t *tp, void *v);
#endif
/* TYPED blocks keep their rtd in cell 0 and their fields from cell 1 on; the
 * kind test reads the header alone, so the rtd itself can be any object --
 * records are told apart by eq? on it.  The debug versions in n.c give the
 * same answers and assert the rest. */
#ifdef NDEBUG
  #define is_typed(o) is_tagged(o, TYPED_MTAG)
  #define ck_typed(o, t) (o)
  #define typed_type(o) (&block_ref(o, 0))
  #define typed_len(o) (block_len(o)-1) 
  #define typed_ref(o, i) (&block_ref(o, (i)+1))
#else
  extern int is_typed(obj o);
  extern obj ck_typed(obj o);
  extern obj* typed_type(obj o); 
  extern int typed_len(obj o);
  extern obj* typed_ref(obj o, int i); 
#endif

/* booleans */
#define TRUE_ITAG 0
typedef int bool_t;
#define is_bool(o) (!((o) & ~(obj)1))
#define bool_obj(b) ((b) ? mkimm(0, TRUE_ITAG) : 0)

/* numbers */
#define FIXNUM_WIDTH 30
#define FIXNUM_MASK 0x3FFFFFFF
#define FIXNUM_SIGN 0x20000000
#define FIXNUM_MIN -536870912
#define FIXNUM_MAX 536870911
#define ASSERT(x) (void)(0)
#ifdef NDEBUG
#define fxneg(x) (-(x))
#define fxabs(x) (labs(x))
#define fxadd(x, y) ((x) + (y))
#define fxsub(x, y) ((x) - (y))
#define fxmul(x, y) ((x) * (y))
/* exact integer division */
#define fxdiv(x, y) ((x) / (y))  
/* truncated division (common/C99) */
#define fxquo(x, y) ((x) / (y))  
#define fxrem(x, y) ((x) % (y))
/* floor division */
static long fxmqu(long x, long y) {
  long q = x / y, r = x % y; return ((r < 0 && y > 0) || (r > 0 && y < 0)) ? q - 1 : q;
}
static long fxmlo(long x, long y) {
  long r = x % y; return ((r < 0 && y > 0) || (r > 0 && y < 0)) ? r + y : r;
}
/* euclidean division */
static long fxeuq(long x, long y) { 
  long q = x / y, r = x % y; return (r < 0) ? ((y > 0) ? q - 1 : q + 1) : q;
}
static long fxeur(long x, long y) {
  long r = x % y; return (r < 0) ? ((y > 0) ? r + y : r - y) : r;
} 
static long fxgcd(long x, long y) {
  long a = labs(x), b = labs(y), c; while (b) c = a%b, a = b, b = c; 
  return a;
} 
#define fxasl(x, y) ((x) << (y))
#define fxasr(x, y) ((x) >> (y))
#define fxflo(f) ((long)(f))
#else
extern long fxneg(long x);
extern long fxabs(long x);
extern long fxadd(long x, long y);
extern long fxsub(long x, long y);
extern long fxmul(long x, long y);
extern long fxdiv(long x, long y);
extern long fxquo(long x, long y);
extern long fxrem(long x, long y);
extern long fxmqu(long x, long y);
extern long fxmlo(long x, long y);
extern long fxeuq(long x, long y);
extern long fxeur(long x, long y);
extern long fxgcd(long x, long y);
extern long fxasl(long x, long y);
extern long fxasr(long x, long y);
extern long fxflo(double f);
#endif
extern long fxaddc(long x, long y, long *pc);
extern long fxsubc(long x, long y, long *pc);
extern long fxmulc(long x, long y, long *pc);
static int flisint(double f) { return f > -HUGE_VAL && f < HUGE_VAL && f == floor(f); }
/* returns 0 if result is not representable as a fixnum */
extern long fxpow(long x, long y);
/* returns 0 if result is not representable as a fixnum */
extern long fxsqrt(long x);
extern int fxifdv(long x, long y, long *pi, double *pd);
extern int fxlen(long x); /* srfi-143 fxlength */
extern int fxbtc(long x); /* srfi-143 fxfxbit-count */
extern double flquo(double x, double y);
extern double flrem(double x, double y);
extern double flmqu(double x, double y);
extern double flmlo(double x, double y);
extern double flgcd(double x, double y);
extern double flround(double x);

/* fixnums */
typedef long fixnum_t;
#define is_fixnum(o) (isim0(o))
#define get_fixnum(o) (getim0s(o))
#define fixnum_obj(i) mkim0((fixnum_t)(i))

/* flonums */
#ifndef FLONUMS_BOXED
typedef double flonum_t;
typedef cxoint_t flobits_t;
#define is_flonum(o) (((o) & 0xffff000000000000ULL) != 0ULL)
#define get_flobits(o) (~(o))
union iod { cxoint_t i; double d; };
static double get_flonum(obj o) { 
  union iod u; 
  assert(is_flonum(o));
  u.i = ~o; 
  return u.d; 
}
static obj hflonum_obj(int rc, double d) { 
  union iod u; 
  u.d = d; 
  assert(is_flonum(~u.i));
  return ~u.i; 
}
#else
extern cxtype_t *FLONUM_NTAG;
typedef double flonum_t;
typedef int64_t flobits_t; /* has to be the same size as flonum_t! */
#define is_flonum(o) (is_native(o, FLONUM_NTAG))
#define get_flonum(o) (*(flonum_t*)get_native(o, FLONUM_NTAG))
#define get_flobits(o) (*(flobits_t*)get_native(o, FLONUM_NTAG))
#define hflonum_obj(l, f) hpushptr(dupflonum(f), FLONUM_NTAG, l)
extern flonum_t *dupflonum(flonum_t f);
#endif

/* part of a number */
typedef union nump { 
  long fix; 
  double flo;
#ifdef OPT_TOWER
  struct bignum *big;
#endif
} nump_t;

/* numerical type */
typedef unsigned short numt_t;
#define NUMT_NONE (0)
#define NUMT_FIX  (1)
#define NUMT_FLO  (2)
/* .. extended in n_tower.h" */

typedef struct fatnum4 { 
  numt_t t; 
  nump_t p[4]; 
} fatnum4_t;

/* returns NUMT_NONE and sets errno on failure */
extern numt_t strtonum4(fatnum4_t *f4, const char *s, char **endp, int radix);

/* other numbers */
#ifdef OPT_TOWER
#include "opt/n_tower.h"
#endif

/* characters */
#define CHAR_ITAG 2
typedef int char_t;
#define is_char(o) (isimm(o, CHAR_ITAG))
#define get_char(o) ((int)getimmu(o, CHAR_ITAG))
#define char_obj(i) mkimm(i, CHAR_ITAG)

/* common helper data types */
typedef struct { char *buf; char *fill; char *end; } cbuf_t;
extern cbuf_t* newcb(void);
extern void freecb(cbuf_t* pcb);
extern void cbgrow(cbuf_t* pcb, size_t n);
extern char* cballoc(cbuf_t* pcb, size_t n);
extern int cbputc(int c, cbuf_t* pcb);
static int cbempty(cbuf_t* pcb) { return pcb->fill == pcb->buf; }
extern size_t cblen(cbuf_t* pcb);
extern char* cbdata(cbuf_t* pcb);
extern cbuf_t* cbclear(cbuf_t *pcb);

/* strings */
extern cxtype_t *STRING_NTAG;
#define is_string(o) (is_native(o, STRING_NTAG))
#define string_data(o) ((const int*)get_native(o, STRING_NTAG))
#define sdatalen(d) ((d)[0])
#define string_len(o) sdatalen(string_data(o))
#define string_chars(o) ((const char*)(sdatachars(string_data(o))))
#ifdef OPT_UNICODE
#include "opt/n_unicode.h"
#else /* ascii representation block */
#define sdatachars(d) ((char*)((d)+1))
#define sdatacspan(d) ((d)[0])
#define sdataget(d, i) (sdatachars(d)[i])
#define sdataput(d, i, c) (sdatachars(d)[i] = (c), d)
extern int *newsdata(const char *s);
extern int *newsdatan(const char *s, int n);
extern int *makesdata(int n, int c);
extern int *subsdata(const int *d, int from, int to);
extern int *catsdata(const int *d0, const int *d1);
extern int *dupsdata(const int *d);
extern int *mapsdata(const int *d, int (*f)(int));
extern int sdatacmp(const int *d1, const int *d2);
extern int sdatacmp_ci(const int *d1, const int *d2);
extern unsigned long sdatahash(const int *d);
/* string procedures */
#ifdef NDEBUG
  #define string_refp(o, i) (string_chars(o)+(i))
#else
  extern char* string_refp(obj o, int i);
#endif
#define string_get(o, i) (*(unsigned char *)string_refp(o, i))
#define string_put(o, i, c) (*(unsigned char *)string_refp(o, i) = (c))
extern int *stringr(int sc, obj pso[]);
extern int *stringrcat(int sc, obj pso[]);
/* basic parsing/unparsing */
#define uencode(buf, c) (buf[0] = (c), 1)
#define udecode(cpp) (*(*cpp)++)
#define unextc(cp) (*(cp)++)
#define unextc_check(cp) (*(cp)++)
#define udistance(sp, ep) ((ep) - (sp))
#define uadvance(sp, n) ((sp) + (n))
#define umemchr memchr
#define uungetch(c, pcb, next) ((next)-1)
#define ufputc fputc
#define ucbputc cbputc
/* char ops */
#define uisspace isspace
#define uislower islower
#define uisupper isupper
#define uisalpha isalpha
#define uisdigit isdigit
#define udigitval(c) ((c) - '0')
#define utolower tolower
#define utoupper toupper
#define utotitle toupper
#define utofold  tolower
#define ufopen fopen
#define uremove remove
#define urename rename
#define ugetenv getenv
#define usystem system
#endif /* end of !OPT_UNICODE block */
extern int strcmp_ci(const char *s1, const char *s2);
extern int strncmp_ci(const char *s1, const char *s2, size_t n);
#define hstring_obj(l, s) hpushptr(s, STRING_NTAG, l)

/* vectors */
/* V kind: every cell is an element */
#define is_vector(o) is_tagged(o, VECTOR_MTAG)
#define vector_ref(v, i) block_ref(v, i)
#define vector_len(v) block_len(v)

/* bytevectors */
extern cxtype_t *BYTEVECTOR_NTAG;
#define is_bytevector(o) (is_native(o, BYTEVECTOR_NTAG))
#define bytevector_data(o) ((int*)get_native(o, BYTEVECTOR_NTAG))
#define bvdatabytes(d) ((unsigned char*)((d)+2))
#define bvdatatype(d) ((d)[1])
#define bytevector_len(o) (bytevector_data(o)[0])
#define bytevector_bytes(o) (bvdatabytes(bytevector_data(o)))
#define bytevector_type(o) (bvdatatype(bytevector_data(o)))
#define hbytevector_obj(l, s) hpushptr(s, BYTEVECTOR_NTAG, l)
static int is_byte(obj o) { return (fixnum_obj(0) <= o && o <= fixnum_obj(255)); } 
#define byte_obj(x) fixnum_obj((unsigned char)(x))
#ifdef NDEBUG
  #define get_byte(o) ((unsigned char)get_fixnum(o))
#else /* every caller checks with ck8() first, so this only catches a missing one */
  static unsigned char get_byte(obj o) 
    { fixnum_t n = get_fixnum(o); assert(0 <= n && n <= 255); return (unsigned char)n; }
#endif
#ifdef NDEBUG
  #define bytevector_refp(o, i) (bytevector_bytes(o)+(i))
#else
  extern unsigned char* bytevector_refp(obj o, int i);
#endif
extern int *newbytevector(unsigned char *s, int n);
extern int *makebytevector(int n, int c);
extern int *allocbytevector(int n);
extern int *dupbytevector(int *d);
extern int bytevectoreq(int *d0, int *d1);
extern int *subbytevector(int *d, int from, int to);

/* boxes */
/* S kind, one cell: the header is the whole test and the cell is the value */
#define is_box(o) is_packed(o, 1)
#define box_ref(o) packed_ref(o, 0)

/* null */
#define NULL_ITAG 3
#define null_obj() mkimm(0, NULL_ITAG)
#define is_null(o) ((o) == mkimm(0, NULL_ITAG))

/* pairs and lists */
/* S kind, two cells: car and cdr, with no btag between them */
#define is_pair(o) is_packed(o, 2)
#define pair_car(o) packed_ref(o, 0)
#define pair_cdr(o) packed_ref(o, 1)
extern int is_list(obj l);

/* symbols */
#define SYMBOL_ITAG 4
#define is_symbol(o) (isimm(o, SYMBOL_ITAG))
#define symbol_obj(i) mkimm(i, SYMBOL_ITAG)
#define get_symbol(o) getimmu(o, SYMBOL_ITAG)
extern const char *symbolname(int sym);
extern int internsym(const char *name);
extern int internsdata(int *d, int dup);
extern const int *symsdata(int sym);

/* records: a TYPED block whose rtd is anything but TUPLE_RTD */
#define record_rtd(r) *typed_type(r)
#define record_len(r) typed_len(r)
#define record_ref(r, i) *typed_ref(r, i)

/* procedures (vm closures): recognised by cell 0 alone, and why is_procedure
 * must stay a macro -- see doc/internals/notes.md [1] */
#ifdef NDEBUG
   #define is_procedure(o) is_tagged(o, CLOSURE_MTAG)
   #define procedure_len(o) block_len(o)
   #define procedure_refp(o, i) (&block_ref(o, i))
#else
  extern int is_procedure(obj o);
  extern int procedure_len(obj o);
  extern obj* procedure_refp(obj o, int i);
#endif
#define procedure_ref(o, i) (*procedure_refp(o, i))

/* box representation extras */
#define box_bsz() block_bsz(1)
#define hend_box() hend_packed(1)

/* pair representation extras */
#define pair_bsz() block_bsz(2)
#define hend_pair() hend_packed(2)

/* vector representation extras */
#define vector_bsz(n) block_bsz(n)
#define hend_vector(n) hend_tagged(n, VECTOR_MTAG)

/* record representation extras: a TYPED block whose rtd is a symbol */
#define record_bsz(c) block_bsz((c)+1)
#define hend_record(rtd, c) (*--hp = rtd, hend_tagged((c)+1, TYPED_MTAG))

/* vm closure representation; isprocedure and friends are the quick tests in
 * a release build and the same tests plus assertions in a debug one, so the
 * answers no longer depend on NDEBUG -- see n.h */
#define procedure_bsz(c) block_bsz(c)
#define hend_procedure(c) hend_tagged(c, CLOSURE_MTAG)

/* vm tuple representation (c != 1) */
/* a TYPED block too, with the one rtd that no record may use */
#define TUPLE_RTD      bool_obj(0)
#define is_tuple(x)    (is_typed(x) && record_rtd(x) == TUPLE_RTD)
#define tuple_ref(x,i) block_ref(x, (i)+1)
#define tuple_len(x)   (block_len(x)-1)
#define tuple_bsz(c)   block_bsz((c)+1)
#define hend_tuple(c)  (*--hp = TUPLE_RTD, hend_tagged((c)+1, TYPED_MTAG))

/* extras shared by every kind of code */
#define are_fixnums(o1, o2) (is_fixnum(o1) && is_fixnum(o2))
#define is_noncircular(o) (!is_circular(o))
#define bytevector_ref(o, i) (*bytevector_refp(o, i))
#define is_record(o) (is_typed(o) && record_rtd(o) != TUPLE_RTD)
#ifdef OPT_TOWER
#define is_number(o) (is_fixnum(o) || is_flonum(o) || is_bignum(o) || is_fatnum(o))
#else
#define is_number(o) (is_fixnum(o) || is_flonum(o))
#endif

/* void: the value of an expression that has nothing to return */
#define VOID_ITAG 5
#define void_obj() mkimm(0, VOID_ITAG)
#define is_void(o) ((o) == mkimm(0, VOID_ITAG))

/* unit: what (values) with no values becomes in a single-value context */
#define UNIT_ITAG 6
#define unit_obj() mkimm(0, UNIT_ITAG)
#define is_unit(o) ((o) == mkimm(0, UNIT_ITAG))

/* eof */
#define EOF_ITAG 7
#define eof_obj() mkimm(0, EOF_ITAG)
#define is_eof(o) ((o) == mkimm(0, EOF_ITAG))

/* shebangs (#! directives or script start lines) */
#define SHEBANG_ITAG 8
#define is_shebang(o) (isimm(o, SHEBANG_ITAG))
#define shebang_obj(i) mkimm(i, SHEBANG_ITAG)
#define get_shebang(o) getimmu(o, SHEBANG_ITAG)

/* input/output ports */
/* a ctl method answers -1 unimplemented, 0 ok, >0 failed; see notes.md [9] */
typedef enum { 
  CTLOP_OFL, CTLOP_ICL, 
  CTLOP_RDLN, 
  CTLOP_CI, CTLOP_SETCI, 
  CTLOP_SETPROMPT, 
  CTLOP_POS, CTLOP_SETPOS 
} ctlop_t;
typedef struct { /* extends cxtype_t */
  const char *tname;
  void (*free)(void*);
  enum { SPT_INPUT = 1, SPT_OUTPUT = 2, SPT_IO = 3, SPT_BINARY = 4 } spt;
  int  (*getch)(void*);
  int  (*ungetch)(int, void*);
  int  (*putch)(int, void*);
  int  (*ctl)(ctlop_t, void*, ...);
} cxtype_port_t, cxtype_iport_t, cxtype_oport_t;
#ifdef OPT_ENHTTY /* + tty */
#define PORTTYPES_MAX 12
#else /* no tty ports */
#define PORTTYPES_MAX 10
#endif
extern cxtype_port_t cxt_port_types[PORTTYPES_MAX];

/* common i/o utils */
extern char *msearch(const char *h, int hlen, const char *n, int nlen);
static cxtype_port_t *portvt(obj o) { 
  cxtype_t *pt; if (!isobjptr(o)) return NULL;
  pt = (cxtype_t*)objptr_from_obj(o)[-1];
  if (pt >= (cxtype_t*)&cxt_port_types[0] && 
      pt < (cxtype_t*)&cxt_port_types[PORTTYPES_MAX])
  return (cxtype_port_t*)pt; else return NULL; }
/* for the operations that do not care which way a port goes, such as its position */
#define is_port(o) (portvt(o) != NULL)
#define portdata(o) ((void*)(*objptr_from_obj(o)))

/* input ports */
extern cxtype_t *IPORT_CLOSED_NTAG;
extern cxtype_t *IPORT_FILE_NTAG;
extern cxtype_t *IPORT_BYTEFILE_NTAG;
extern cxtype_t *IPORT_STRING_NTAG;
extern cxtype_t *IPORT_BYTEVECTOR_NTAG;
static cxtype_iport_t *iportvt(obj o) { 
  cxtype_t *pt; if (!isobjptr(o)) return NULL;
  pt = (cxtype_t*)objptr_from_obj(o)[-1];
  if (pt >= (cxtype_t*)&cxt_port_types[0] && 
      pt < (cxtype_t*)&cxt_port_types[PORTTYPES_MAX] &&
      (((cxtype_port_t*)pt)->spt & SPT_INPUT))
  return (cxtype_iport_t*)pt; else return NULL; }
#define ckiportvt(o) ((cxtype_iport_t*)cxm_cknull(iportvt(o), "iportvt"))
#define is_iport(o) (iportvt(o) != NULL)
#define iportdata(o) ((void*)(*objptr_from_obj(o)))
static int iportgetc(obj o) {
  cxtype_iport_t *vt = iportvt(o); void *pp = iportdata(o);
  assert(vt); return vt->getch(pp);
}
static int iportpeekc(obj o) {
  cxtype_iport_t *vt = iportvt(o); void *pp = iportdata(o); int c;
  assert(vt); c = vt->getch(pp); if (c != EOF) vt->ungetch(c, pp); return c;
}
static void iportclear(obj o) {
  cxtype_iport_t *vt = iportvt(o); void *pp = iportdata(o);
  assert(vt); vt->ctl(CTLOP_ICL, pp);
}
extern char *rdns(int (*in_getc)(void*), int (*in_ungetc)(int, void*), void *in, cbuf_t *pcb);
extern int rdah(int fold, int (*in_getc)(void*), int (*in_ungetc)(int, void*), void *in, 
  obj *po, fatnum4_t *pf, int **pp);
/* file input ports */
typedef enum { TIF_NONE = 0, TIF_EOF = 1, TIF_CI = 2 } tiflags_t;
typedef struct tifile { cbuf_t cb; char *next; FILE *fp; int lno, fns; tiflags_t flags; } tifile_t;
extern tifile_t *tialloc(FILE *fp, int fns);
#define hiport_file_obj(l, fp) hpushptr(fp, IPORT_FILE_NTAG, l)
/* bytefile input ports */
#define hiport_bytefile_obj(l, fp) hpushptr(fp, IPORT_BYTEFILE_NTAG, l)
/* string input ports */
typedef enum { SIF_NONE = 0, SIF_CI = 2 } siflags_t;
typedef struct sifile { const char *p; const char *e; void *base; siflags_t flags; } sifile_t;
extern sifile_t *sialloc(const char *p, int span, void *base);
#define hiport_string_obj(l, fp) hpushptr(fp, IPORT_STRING_NTAG, l)
/* bytevector input ports */
/* s is where the data starts, so that p-s is the port's position */
typedef struct bvfile { unsigned char *s, *p, *e; void *base; } bvifile_t;
extern bvifile_t *bvialloc(unsigned char *p, unsigned char *e, void *base);
#define hiport_bytevector_obj(l, fp) hpushptr(fp, IPORT_BYTEVECTOR_NTAG, l)
/* optional enhanced tty ports */
#ifdef OPT_ENHTTY
extern struct ttfile *ttalloc(int in);
extern cxtype_t *IPORT_TTY_NTAG;
extern cxtype_t *OPORT_TTY_NTAG;
#define hiport_tty_obj(l) hpushptr(ttalloc(1), IPORT_TTY_NTAG, l)
#define hoport_tty_obj(l) hpushptr(ttalloc(0), OPORT_TTY_NTAG, l)
#endif

/* output ports */
extern cxtype_t *OPORT_CLOSED_NTAG;
extern cxtype_t *OPORT_FILE_NTAG;
extern cxtype_t *OPORT_BYTEFILE_NTAG;
#define hoport_bytefile_obj(l, fp) hpushptr(fp, OPORT_BYTEFILE_NTAG, l)
extern cxtype_t *OPORT_STRING_NTAG;
extern cxtype_t *OPORT_BYTEVECTOR_NTAG;
static cxtype_oport_t *oportvt(obj o) { 
  cxtype_t *pt; if (!isobjptr(o)) return NULL;
  pt = (cxtype_t*)objptr_from_obj(o)[-1];
  if (pt >= (cxtype_t*)&cxt_port_types[0] && 
      pt < (cxtype_t*)&cxt_port_types[PORTTYPES_MAX] &&
    (((cxtype_port_t*)pt)->spt & SPT_OUTPUT))
    return (cxtype_oport_t*)pt; else return NULL; }
#define ckoportvt(o) ((cxtype_oport_t*)cxm_cknull(oportvt(o), "oportvt"))
#define is_oport(o) (oportvt(o) != NULL)
#define oportdata(o) ((void*)(*objptr_from_obj(o)))
static void oportputc(int c, obj o) {
  cxtype_oport_t *vt = oportvt(o); void *pp = oportdata(o);
  assert(vt); vt->putch(c, pp);
}
static void oportputs(const char *s, obj o) {
  cxtype_oport_t *vt = oportvt(o); void *pp = oportdata(o);
  assert(vt); while (*s) vt->putch(unextc(s), pp); /* chars to text port */
}
static void oportwrite(const unsigned char *s, int n, obj o) {
  cxtype_oport_t *vt = oportvt(o); void *pp = oportdata(o);
  assert(vt); while (n-- > 0) vt->putch(*s++, pp); /* bytes to bin port */
}
static void oportflush(obj o) {
  cxtype_oport_t *vt = oportvt(o); void *pp = oportdata(o);
  assert(vt); vt->ctl(CTLOP_OFL, pp);
}
/* file output ports */
#define hoport_file_obj(l, fp) hpushptr(fp, OPORT_FILE_NTAG, l)
/* string output ports */
#define hoport_string_obj(l, fp) hpushptr(fp, OPORT_STRING_NTAG, l)
/* bytevector output ports */
/* fill is the write position, hwl how far the data reaches; see notes.md [8] */
typedef struct bvofile { cbuf_t cb; size_t hwl; } bvofile_t;
extern bvofile_t *bvoalloc(void);
extern size_t bvolen(bvofile_t *fp);
#define hoport_bytevector_obj(l, fp) hpushptr(fp, OPORT_BYTEVECTOR_NTAG, l)

/* internal list functions */
extern int is_circular(obj x);
extern int iseqv(obj x, obj y);
extern obj ismemv(obj x, obj l);
extern obj isassv(obj x, obj l);
extern int isequal(obj x, obj y);
extern obj ismember(obj x, obj l);
extern obj isassoc(obj x, obj l);

/* S-expression writers */
extern void oportputsimple(obj x, obj p, int disp);
extern void oportputcircular(obj x, obj p, int disp);
extern void oportputshared(obj x, obj p, int disp);

/* 'generic' writer for flonums; returns 0 or -1 on invalid radix */
extern int wrdn(double n, int radix, int mode, int prc, int (*pf)(int, void*), void *pd);
/* floating-point formatter wit support for radices 10, 2, 4, 8, 16 */
extern char *dntostr(char *buf, size_t len, double x, int radix, int mode, int prc);
/* safe for fixed printing at the end of f range */
#define DN_DEC_BUFSIZE (16 + DBL_DECIMAL_DIG + 32)
/* safe for fixed binary printing at the end of f range */
#define DN_MAX_BUFSIZE (DBL_MANT_DIG + DBL_MANT_DIG + 40)

/* detecting math libraries */
#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
#define C99_MATH_LIB
#elif defined(_MSC_VER) && _MSC_VER >= 1900
#define C99_MATH_LIB
#endif
#if defined(C99_MATH_LIB) && defined(NOXMATH)
#undef C99_MATH_LIB
#endif
#if defined(_XOPEN_SOURCE) || defined(_BSD_SOURCE)
#define XSI_MATH_LIB
#elif defined(_MSC_VER) && _MSC_VER >= 1500
#define XSI_MATH_LIB
#define jn(x, y) _jn(x, y)
#define yn(x, y) _yn(x, y)
#endif
#if defined(XSI_MATH_LIB) && defined(NOXMATH)
#undef XSI_MATH_LIB
#endif

/* time */
extern double microtime(void);
extern double microclock(void);
extern long tzoffset(void);
