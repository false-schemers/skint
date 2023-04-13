/* n.h -- natives */

/* standard includes */
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <assert.h>
/* extra includes */
#include <math.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>
#include <time.h>

/* standard definitions */
#ifdef NAN_BOXING
#include <stdint.h>
typedef int64_t obj;          /* pointers are this size, higher 16 bits and lower bit zero */
typedef int64_t cxoint_t;     /* same thing, used as integer */
typedef struct {              /* type descriptor */
  const char *tname;          /* name (debug) */
  void (*free)(void*);        /* deallocator */
} cxtype_t;

#define notobjptr(o)          (((cxoint_t)(o) - (cxoint_t)cxg_heap) & cxg_hmask)
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

#define notobjptr(o)          (((char*)(o) - (char*)cxg_heap) & cxg_hmask)
#define isobjptr(o)           (!notobjptr(o))
#define notaptr(o)            ((o) & 1)
#define isaptr(o)             (!notaptr(o))
#endif

#define obj_from_obj(o)       (o)
#define obj_from_objptr(p)    ((obj)(p))
#define obj_from_size(n)      (((cxoint_t)(n) << 1) | 1)

#define objptr_from_objptr(p) (p)
#define objptr_from_obj(o)    ((obj*)(o))

#define size_from_obj(o)      ((int)((o) >> 1))

#define obj_from_case(n)      obj_from_objptr(cases+(n))
#define case_from_obj(o)      (objptr_from_obj(o)-cases)
#define obj_from_ktrap()      obj_from_size(0x5D56F806)
#define obj_from_void(v)      ((void)(v), obj_from_size(0x6F56DF77))

#define bool_from_obj(o)      (o)
#define bool_from_bool(b)     (b)
#define bool_from_size(s)     (s)

#define void_from_void(v)     (void)(v)
#define void_from_obj(o)      (void)(o)

#define rreserve(m)           if (r + (m) >= cxg_rend) r = cxm_rgc(r, m)
#define hpushptr(p, pt, l)    (hreserve(2, l), *--hp = (obj)(p), *--hp = (obj)(pt), (obj)(hp+1))   
#define hbsz(s)               ((s) + 1) /* 1 extra word to store block size */
#define hreserve(n, l)        ((hp < cxg_heap + (n)) ? hp = cxm_hgc(r, r+(l), hp, n) : hp)
#define hendblk(n)            (*--hp = obj_from_size(n), (obj)(hp+1))
#define hblklen(p)            size_from_obj(((obj*)(p))[-1])
#define hblkref(p, i)         (((obj*)(p))[i])

typedef obj (*cxhost_t)(obj);
typedef struct cxroot_tag {
  int globc; obj **globv;
  struct cxroot_tag *next;
} cxroot_t;

extern obj *cxg_heap;
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
#define isim0(o)    (((o) & 0xffff000000000003ULL) == 3)
#define isimm(o, t) (((o) & 0xffff0000000000ffULL) == (((t) << 2) | 1))
#ifdef NDEBUG
  #define getim0s(o) (long)((((o >> 2) & 0x3fffffff) ^ 0x20000000) - 0x20000000)
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
  #define getim0s(o) (long)((((o >> 2) & 0x3fffffff) ^ 0x20000000) - 0x20000000)
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
   static int isnative(obj o, cxtype_t *tp) 
     { return isobjptr(o) && objptr_from_obj(o)[-1] == (obj)tp;  }
   #define getnative(o, t) ((void*)(*objptr_from_obj(o)))
#else
  extern int isnative(obj o, cxtype_t *tp);
  extern void *getnative(obj o, cxtype_t *tp);
#endif
extern int istagged(obj o, int t);
#ifdef NDEBUG
  #define cktagged(o, t) (o)
  #define taggedlen(o, t) (hblklen(o)-1) 
  #define taggedref(o, t, i) (&hblkref(o, (i)+1))
#else
  extern obj cktagged(obj o, int t);
  extern int taggedlen(obj o, int t);
  extern obj* taggedref(obj o, int t, int i); 
#endif
extern int istyped(obj o);
#ifdef NDEBUG
  #define cktyped(o, t) (o)
  #define typedtype(o) (&hblkref(o, 0))
  #define typedlen(o) (hblklen(o)-1) 
  #define typedref(o, i) (&hblkref(o, (i)+1))
#else
  extern obj cktyped(obj o);
  extern obj* typedtype(obj o); 
  extern int typedlen(obj o);
  extern obj* typedref(obj o, int i); 
#endif
/* booleans */
#define TRUE_ITAG 0
typedef int bool_t;
#define is_bool_obj(o) (!((o) & ~(obj)1))
#define is_bool_bool(b) ((void)(b), 1)
#define void_from_bool(b) (void)(b)
#define obj_from_bool(b) ((b) ? mkimm(0, TRUE_ITAG) : 0)
/* void */
#define VOID_ITAG 1
#define mkvoid() mkimm(0, VOID_ITAG)
#define isvoid(o) ((o) == mkimm(0, VOID_ITAG))
#undef obj_from_void
#define obj_from_void(v) ((void)(v), mkimm(0, VOID_ITAG))
/* unit */
#define obj_from_unit() (obj_from_size(0x6DF6F577))
/* numbers */
#define FIXNUM_BIT 30
#define FIXNUM_MIN -536870912
#define FIXNUM_MAX 536870911
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
  long q = x / y; return ((x < 0 && y > 0) || (x > 0 && y < 0)) ? q - 1 : q;
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
static int flisint(double f) { return f > -HUGE_VAL && f < HUGE_VAL && f == floor(f); }
extern long fxpow(long x, long y);
extern long fxsqrt(long x);
extern int fxifdv(long x, long y, long *pi, double *pd);
extern double flquo(double x, double y);
extern double flrem(double x, double y);
extern double flmqu(double x, double y);
extern double flmlo(double x, double y);
extern double flgcd(double x, double y);
extern double flround(double x);
extern int strtofxfl(char *s, int radix, long *pl, double *pd);
/* fixnums */
typedef long fixnum_t;
#define is_fixnum_obj(o) (isim0(o))
#define is_fixnum_fixnum(i) ((void)(i), 1)
#define is_bool_fixnum(i) ((void)(i), 0)
#define is_fixnum_bool(i) ((void)(i), 0)
#define fixnum_from_obj(o) (getim0s(o))
#define fixnum_from_fixnum(i) (i)
#define fixnum_from_flonum(l,x) ((fixnum_t)(x))
#define bool_from_fixnum(i) ((void)(i), 1)
#define void_from_fixnum(i) (void)(i)
#define obj_from_fixnum(i) mkim0((fixnum_t)(i))
/* flonums */
#ifndef FLONUMS_BOXED
typedef double flonum_t;
#define is_flonum_obj(o) (((o) & 0xffff000000000000ULL) != 0ULL)
#define is_flonum_flonum(f) ((void)(f), 1)
#define is_flonum_bool(f) ((void)(f), 0)
#define is_bool_flonum(f) ((void)(f), 0)
#define is_fixnum_flonum(i) ((void)(i), 0)
#define is_flonum_fixnum(i) ((void)(i), 0)
#define flonum_from_flonum(l, f) (f)
#define flonum_from_fixnum(x) ((flonum_t)(x))
#define bool_from_flonum(f) ((void)(f), 0)
#define void_from_flonum(l, f) (void)(f)
union iod { cxoint_t i; double d; };
static double flonum_from_obj(obj o) { 
  union iod u; 
  assert(is_flonum_obj(o));
  u.i = ~o; 
  return u.d; 
}
static obj obj_from_flonum(int rc, double d) { 
  union iod u; 
  u.d = d; 
  assert(is_flonum_obj(~u.i));
  return ~u.i; 
}
#else /* FLONUMS_BOXED */
extern cxtype_t *FLONUM_NTAG;
typedef double flonum_t;
#define is_flonum_obj(o) (isnative(o, FLONUM_NTAG))
#define is_flonum_flonum(f) ((void)(f), 1)
#define is_flonum_bool(f) ((void)(f), 0)
#define is_bool_flonum(f) ((void)(f), 0)
#define is_fixnum_flonum(i) ((void)(i), 0)
#define is_flonum_fixnum(i) ((void)(i), 0)
#define flonum_from_obj(o) (*(flonum_t*)getnative(o, FLONUM_NTAG))
#define flonum_from_flonum(l, f) (f)
#define flonum_from_fixnum(x) ((flonum_t)(x))
#define bool_from_flonum(f) ((void)(f), 0)
#define void_from_flonum(l, f) (void)(f)
#define obj_from_flonum(l, f) hpushptr(dupflonum(f), FLONUM_NTAG, l)
extern flonum_t *dupflonum(flonum_t f);
#endif
/* characters */
#define CHAR_ITAG 2
typedef int char_t;
#define ischar(o) (isimm(o, CHAR_ITAG))
#define is_char_obj(o) (isimm(o, CHAR_ITAG))
#define is_char_char(i) ((void)(i), 1)
#define is_char_bool(i) ((void)(i), 0)
#define is_bool_char(i) ((void)(i), 0)
#define is_char_fixnum(i) ((void)(i), 0)
#define is_fixnum_char(i) ((void)(i), 0)
#define is_char_flonum(i) ((void)(i), 0)
#define is_flonum_char(i) ((void)(i), 0)
#define char_from_obj(o) ((int)getimmu(o, CHAR_ITAG))
#define char_from_char(i) (i)
#define bool_from_char(i) ((void)(i), 1)
#define void_from_char(i) (void)(i)
#define obj_from_char(i) mkimm(i, CHAR_ITAG)
/* strings */
extern cxtype_t *STRING_NTAG;
#define isstring(o) (isnative(o, STRING_NTAG))
#define stringdata(o) ((int*)getnative(o, STRING_NTAG))
#define sdatachars(d) ((char*)((d)+1))
#define stringlen(o) (*stringdata(o))
#define stringchars(o) ((char*)(stringdata(o)+1))
#define hpushstr(l, s) hpushptr(s, STRING_NTAG, l)
#ifdef NDEBUG
  #define stringref(o, i) (stringchars(o)+(i))
#else
  extern char* stringref(obj o, int i);
#endif
extern int *newstring(char *s);
extern int *newstringn(char *s, int n);
extern int *allocstring(int n, int c);
extern int *substring(int *d, int from, int to);
extern int *stringcat(int *d0, int *d1);
extern int *dupstring(int *d);
extern void stringfill(int *d, int c);
extern int strcmp_ci(char *s1, char *s2);
/* vectors */
#define VECTOR_BTAG 1
#define isvector(o) istagged(o, VECTOR_BTAG)
#define vectorref(v, i) *taggedref(v, VECTOR_BTAG, i)
#define vectorlen(v) taggedlen(v, VECTOR_BTAG)
/* bytevectors */
extern cxtype_t *BYTEVECTOR_NTAG;
#define isbytevector(o) (isnative(o, BYTEVECTOR_NTAG))
#define bytevectordata(o) ((int*)getnative(o, BYTEVECTOR_NTAG))
#define bvdatabytes(d) ((unsigned char*)((d)+1))
#define bytevectorlen(o) (*bytevectordata(o))
#define bytevectorbytes(o) (bvdatabytes(bytevectordata(o)))
#define hpushu8v(l, s) hpushptr(s, BYTEVECTOR_NTAG, l)
static int is_byte_obj(obj o) { return (obj_from_fixnum(0) <= o && o <= obj_from_fixnum(255)); } 
#define byte_from_obj(o) byte_from_fixnum(fixnum_from_obj(o))
#ifdef NDEBUG
  #define byte_from_fixnum(n) ((unsigned char)(n))
#else
  static unsigned char byte_from_fixnum(int n) { assert(0 <= n && n <= 255); return n; } 
#endif
#define byte_from_obj(o) byte_from_fixnum(fixnum_from_obj(o))
#ifdef NDEBUG
  #define bytevectorref(o, i) (bytevectorbytes(o)+(i))
#else
  extern unsigned char* bytevectorref(obj o, int i);
#endif
extern int *newbytevector(unsigned char *s, int n);
extern int *makebytevector(int n, int c);
extern int *allocbytevector(int n);
extern int *dupbytevector(int *d);
extern int bytevectoreq(int *d0, int *d1);
extern int *subbytevector(int *d, int from, int to);
/* boxes */
#define BOX_BTAG 2
#define isbox(o) istagged(o, BOX_BTAG)
#define boxref(o) *taggedref(o, BOX_BTAG, 0)
/* null */
#define NULL_ITAG 3
#define mknull() mkimm(0, NULL_ITAG)
#define isnull(o) ((o) == mkimm(0, NULL_ITAG))
/* pairs and lists */
#define PAIR_BTAG 3
#define ispair(o) istagged(o, PAIR_BTAG)
#define car(o) *taggedref(o, PAIR_BTAG, 0)
#define cdr(o) *taggedref(o, PAIR_BTAG, 1)
extern int islist(obj l);
/* symbols */
#define SYMBOL_ITAG 4
#define issymbol(o) (isimm(o, SYMBOL_ITAG))
#define mksymbol(i) mkimm(i, SYMBOL_ITAG)
#define getsymbol(o) getimmu(o, SYMBOL_ITAG)
extern char *symbolname(int sym);
extern int internsym(char *name);
/* records */
#define isrecord(o) istyped(o)
#define recordrtd(r) *typedtype(r)
#define recordlen(r) typedlen(r)
#define recordref(r, i) *typedref(r, i)
/* procedures */
extern int isprocedure(obj o);
extern int procedurelen(obj o);
extern obj* procedureref(obj o, int i);
/* apply and dotted lambda list */
extern obj appcases[];
/* eof */
#define EOF_ITAG 7
#define mkeof() mkimm(0, EOF_ITAG)
#define iseof(o) ((o) == mkimm(0, EOF_ITAG))
/* input ports */
typedef struct { /* extends cxtype_t */
  const char *tname;
  void (*free)(void*);
  int (*close)(void*);
  int (*getch)(void*);
  int (*ungetch)(int, void*);
} cxtype_iport_t;
extern cxtype_t *IPORT_CLOSED_NTAG;
extern cxtype_t *IPORT_FILE_NTAG;
extern cxtype_t *IPORT_STRING_NTAG;
extern cxtype_t *IPORT_BYTEVECTOR_NTAG;
static cxtype_iport_t *iportvt(obj o) { 
  cxtype_t *pt; if (!isobjptr(o)) return NULL;
  pt = (cxtype_t*)objptr_from_obj(o)[-1];
  if (pt != IPORT_CLOSED_NTAG && pt != IPORT_FILE_NTAG &&
      pt != IPORT_STRING_NTAG && pt != IPORT_BYTEVECTOR_NTAG) return NULL; 
  else return (cxtype_iport_t*)pt; }
#define ckiportvt(o) ((cxtype_iport_t*)cxm_cknull(iportvt(o), "iportvt"))
#define isiport(o) (iportvt(o) != NULL)
#define iportdata(o) ((void*)(*objptr_from_obj(o)))
static int iportgetc(obj o) {
  cxtype_iport_t *vt = iportvt(o); void *pp = iportdata(o);
  assert(vt); return vt->getch(pp);
}
static int iportpeekc(obj o) {
  cxtype_iport_t *vt = iportvt(o); void *pp = iportdata(o); int c;
  assert(vt); c = vt->getch(pp); if (c != EOF) vt->ungetch(c, pp); return c;
}
/* closed input ports */
#define mkiport_file(l, fp) hpushptr(fp, IPORT_FILE_NTAG, l)
/* string input ports */
typedef struct { char *p; void *base; } sifile_t;
extern sifile_t *sialloc(char *p, void *base);
#define mkiport_string(l, fp) hpushptr(fp, IPORT_STRING_NTAG, l)
/* bytevector input ports */
typedef struct { unsigned char *p, *e; void *base; } bvifile_t;
extern bvifile_t *bvialloc(unsigned char *p, unsigned char *e, void *base);
#define mkiport_bytevector(l, fp) hpushptr(fp, IPORT_BYTEVECTOR_NTAG, l)
/* output ports */
typedef struct { /* extends cxtype_t */
  const char *tname;
  void (*free)(void*);
  int (*close)(void*);
  int (*putch)(int, void*);
  int (*flush)(void*);
} cxtype_oport_t;
extern cxtype_t *OPORT_CLOSED_NTAG;
extern cxtype_t *OPORT_FILE_NTAG;
extern cxtype_t *OPORT_STRING_NTAG;
extern cxtype_t *OPORT_BYTEVECTOR_NTAG;
static cxtype_oport_t *oportvt(obj o) { 
  cxtype_t *pt; if (!isobjptr(o)) return NULL;
  pt = (cxtype_t*)objptr_from_obj(o)[-1];
  if (pt != OPORT_CLOSED_NTAG && pt != OPORT_FILE_NTAG && 
      pt != OPORT_STRING_NTAG && pt != OPORT_BYTEVECTOR_NTAG) return NULL; 
  else return (cxtype_oport_t*)pt; }
#define ckoportvt(o) ((cxtype_oport_t*)cxm_cknull(oportvt(o), "oportvt"))
#define isoport(o) (oportvt(o) != NULL)
#define oportdata(o) ((void*)(*objptr_from_obj(o)))
static void oportputc(int c, obj o) {
  cxtype_oport_t *vt = oportvt(o); void *pp = oportdata(o);
  assert(vt); vt->putch(c, pp);
}
static void oportputs(char *s, obj o) {
  cxtype_oport_t *vt = oportvt(o); void *pp = oportdata(o);
  assert(vt); while (*s) vt->putch(*s++, pp);
}
static void oportwrite(char *s, int n, obj o) {
  cxtype_oport_t *vt = oportvt(o); void *pp = oportdata(o);
  assert(vt); while (n-- > 0) vt->putch(*s++, pp);
}
static void oportflush(obj o) {
  cxtype_oport_t *vt = oportvt(o); void *pp = oportdata(o);
  assert(vt); vt->flush(pp);
}
/* closed output ports */
#define mkoport_file(l, fp) hpushptr(fp, OPORT_FILE_NTAG, l)
/* string output ports */
typedef struct cbuf_tag { char *buf; char *fill; char *end; } cbuf_t;
extern cbuf_t* newcb(void);
extern void freecb(cbuf_t* pcb);
extern int cbputc(int c, cbuf_t* pcb);
extern size_t cblen(cbuf_t* pcb);
extern char* cbdata(cbuf_t* pcb);
#define mkoport_string(l, fp) hpushptr(fp, OPORT_STRING_NTAG, l)
/* bytevector output ports */
#define mkoport_bytevector(l, fp) hpushptr(fp, OPORT_BYTEVECTOR_NTAG, l)
extern int iscircular(obj x);
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
