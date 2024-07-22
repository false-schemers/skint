/* n.c -- generated via skint nsf2c.ssc n.sf */

#include "s.h"
#include "n.h"

#ifdef NAN_BOXING

#ifndef NDEBUG
long getim0s(obj o) {
  assert(isim0(o));
  return (int32_t)(((((uint32_t)o >> 2) & 0x3fffffff) ^ 0x20000000) - 0x20000000);
}
long getimmu(obj o, int t) {
  assert(isimm((o), t));
  return (long)(((uint32_t)o >> 8) & 0xffffff);
}
#endif

#else

#ifndef NDEBUG
long getim0s(obj o) {
  assert(isim0(o));
  return (int)((((o >> 2) & 0x3fffffff) ^ 0x20000000) - 0x20000000);
}
long getimmu(obj o, int t) {
  assert(isimm(o, t));
  return (long)((o >> 8) & 0xffffff);
}
#endif

#endif

#ifndef NDEBUG
int isnative(obj o, cxtype_t *tp) {
  return isobjptr(o) && objptr_from_obj(o)[-1] == (obj)tp; 
}
void *getnative(obj o, cxtype_t *tp) {
  assert(isnative(o, tp));
  return (void*)(*objptr_from_obj(o));
}
#endif

int istagged(obj o, int t) {
  if (!isobjptr(o)) return 0;
  else { obj h = objptr_from_obj(o)[-1];
    return notaptr(h) && size_from_obj(h) >= 1 
      && hblkref(o, 0) == obj_from_size(t); }
}

#ifndef NDEBUG
obj cktagged(obj o, int t) {
  assert(istagged((o), t));
  return o;
}
int taggedlen(obj o, int t) {
  assert(istagged((o), t));
  return hblklen(o) - 1;
}
obj* taggedref(obj o, int t, int i) {
  int len; assert(istagged((o), t));
  len = hblklen(o);
  assert(i >= 0 && i < len-1);  
  return &hblkref(o, i+1);
}
#endif

int istyped(obj o) {
  if (!isobjptr(o)) return 0;
  else { obj h = objptr_from_obj(o)[-1];
    return notaptr(h) && size_from_obj(h) >= 1 
      /* FIXME: manual issymbol() check */
      && isimm(hblkref(o, 0), 4/*SYMBOL_ITAG*/); }
}

#ifndef NDEBUG
obj cktyped(obj o) {
  assert(istyped(o));
  return o;
}
obj* typedtype(obj o) {
  assert(istyped(o));
  return &hblkref(o, 0);
}
int typedlen(obj o) {
  assert(istyped(o));
  return hblklen(o) - 1;
}
obj* typedref(obj o, int i) {
  int len; assert(istyped(o));
  len = hblklen(o);
  assert(i >= 0 && i < len-1);  
  return &hblkref(o, i+1);
}
#endif

#ifndef NDEBUG
long fxneg(long x) { 
  assert(x != FIXNUM_MIN); 
  return -x; 
}
long fxabs(long x) { 
  assert(x != FIXNUM_MIN); 
  return labs(x); 
}
long fxadd(long x, long y) { 
  long z = x + y; 
  assert(z >= FIXNUM_MIN && z <= FIXNUM_MAX); 
  return z; 
}
long fxsub(long x, long y) { 
  long z = x - y; 
  assert(z >= FIXNUM_MIN && z <= FIXNUM_MAX); 
  return z; 
}
long fxmul(long x, long y) { 
  double z = (double)x * (double)y;
  assert(z >= FIXNUM_MIN && z <= FIXNUM_MAX);
  return x * y; 
}
/* exact integer division */
long fxdiv(long x, long y) { 
  assert(y); 
  assert(x != FIXNUM_MIN || y != -1);
  assert(x % y == 0);
  return x / y; 
}
/* truncated division (common/C99) */
long fxquo(long x, long y) { 
  assert(y); assert(x != FIXNUM_MIN || y != -1);
  return x / y; 
}
long fxrem(long x, long y) { 
  assert(y);
  return x % y; 
}
/* floor division */
long fxmqu(long x, long y) {
  long q; assert(y); assert(x != FIXNUM_MIN || y != -1);
  q = x / y;
  return ((x < 0 && y > 0) || (x > 0 && y < 0)) ? q - 1 : q;
}
long fxmlo(long x, long y) {
  long r; assert(y); r = x % y;
  return ((r < 0 && y > 0) || (r > 0 && y < 0)) ? r + y : r;
}
/* euclidean division */
long fxeuq(long x, long y) { 
  long q, r; assert(y); assert(x != FIXNUM_MIN || y != -1);
  q = x / y, r = x % y; 
  return (r < 0) ? ((y > 0) ? q - 1 : q + 1) : q;
}
long fxeur(long x, long y) {
  long r; assert(y); r = x % y; 
  return (r < 0) ? ((y > 0) ? r + y : r - y) : r;
} 
long fxgcd(long x, long y) {
  long a = labs(x), b = labs(y), c; 
  while (b) c = a%b, a = b, b = c; 
  assert(a <= FIXNUM_MAX);
  return a;
} 
long fxasl(long x, long y) {
  assert(y >= 0 && y < FIXNUM_BIT); 
  return x << y;
} 
long fxasr(long x, long y) {
  assert(y >= 0 && y < FIXNUM_BIT);
  assert(!y || x >= 0); /* >> of negative x is undefined */ 
  return x >> y;
} 
long fxflo(double f) {
  long l = (long)f; assert((double)l == f);
  assert(l >= FIXNUM_MIN && l <= FIXNUM_MAX);
  return l;
}
#endif

long fxpow(long x, long y) { 
  assert(y >= 0);
  retry: if (y == 0) return 1; if (y == 1) return x;
  if (y % 2 == 1) x *= fxpow(x, y-1); 
  else { x *= x; y /= 2; assert(FIXNUM_MIN <= x && x <= FIXNUM_MAX); goto retry; }
  assert(FIXNUM_MIN <= x && x <= FIXNUM_MAX); return x;
}

long fxsqrt(long x) { 
  assert(x >= 0); if (x < 2) return x;
  else { long s = fxsqrt(x >> 2) << 1, l = s + 1; return l*l > x ? s : l; }
}

int fxifdv(long x, long y, long *pi, double *pd) { 
  assert(y); assert(x != FIXNUM_MIN || y != -1);
  if (x % y == 0) { *pi = x / y; return 1; }
  else { *pd = (double)x / (double)y; return 0; }  
}

double flquo(double x, double y) {
  double z; assert(y != 0.0 && flisint(x) && flisint(y));
  modf(x / y,  &z);
  return z;
}

double flrem(double x, double y) {
  assert(y != 0.0 && flisint(x) && flisint(y));
  return fmod(x, y);
}

double flmqu(double x, double y) {
  assert(y != 0.0 && flisint(x) && flisint(y));
  return floor(x / y);
}

double flmlo(double x, double y) {
  assert(y != 0.0 && flisint(x) && flisint(y));
  return x - y * floor(x / y);
}

double flgcd(double x, double y) {
  double a = fabs(x), b = fabs(y), c; 
  assert(flisint(a) && flisint(b));
  while (b > 0.0) c = fmod(a, b), a = b, b = c; 
  return a;
}

double flround(double x) {
  double f = floor(x), c = ceil(x), d = x-f, u = c-x; 
  if (d == u) return fmod(f, 2.0) == 0.0 ? f : c;
  else return (d < u) ? f : c;
}

int strtofxfl(char *s, int radix, long *pl, double *pd) {
  extern int strcmp_ci(char *s1, char *s2); /* defined below */
  char *e; int conv = 0, eno = errno; long l; double d;
  for (; s[0] == '#'; s += 2) {
    switch (s[1]) {
      case 'b': case 'B': radix = 2; break;
      case 'o': case 'O': radix = 8; break;
      case 'd': case 'D': radix = 10; break;
      case 'x': case 'X': radix = 16; break;
      case 'e': case 'E': conv = 'e'; break;
      case 'i': case 'I': conv = 'i'; break;
      default: return 0;
    }
  }
  if (isspace(*s)) return 0;
  for (e = s; *e; ++e) { if (strchr(".eEiInN", *e)) break; }
  if (!*e || radix != 10) { /* s is not a syntax for an inexact number */
    l = (errno = 0, strtol(s, &e, radix));
    if (errno || *e || e == s) { if (conv == 'i') goto fl; return (errno = eno, 0); }
    if (conv == 'i') return (errno = eno, *pd = (double)l, 'i');
    if (FIXNUM_MIN <= l && l <= FIXNUM_MAX) return (errno = eno, *pl = l, 'e');
    return (errno = eno, 0); /* can't represent as an exact */
  } 
  fl: if (radix != 10) return (errno = eno, 0); 
  e = "", errno = 0; if (*s != '+' && *s != '-') d = strtod(s, &e);
  else if (strcmp_ci(s+1, "inf.0") == 0) d = (*s == '-' ? -HUGE_VAL : HUGE_VAL); 
  else if (strcmp_ci(s+1, "nan.0") == 0) d = HUGE_VAL - HUGE_VAL;
  else d = strtod(s, &e);
  if (errno || *e || e == s) return (errno = eno, 0);
  if ((conv == 'e') && ((l=(long)d) < FIXNUM_MIN || l > FIXNUM_MAX || (double)l != d))
    return (errno = eno, 0); /* can't be converted to an exact number */
  return (errno = eno, (conv == 'e') ? (*pl = fxflo(d), 'e') : (*pd = d, 'i'));
}

#ifndef FLONUMS_BOXED

#else

static cxtype_t cxt_flonum = { "flonum", free };

cxtype_t *FLONUM_NTAG = &cxt_flonum;

flonum_t *dupflonum(flonum_t f) {
  flonum_t *pf = cxm_cknull(malloc(sizeof(flonum_t)), "malloc(flonum)");
  *pf = f; return pf;
}

#endif

static cxtype_t cxt_string = { "string", free };

cxtype_t *STRING_NTAG = &cxt_string;

#ifndef NDEBUG
char* stringref(obj o, int i) {
  int *d = stringdata(o); assert(i >= 0 && i < *d);  
  return sdatachars(d)+i;
}
#endif

int *newstring(char *s) {
  int l, *d; assert(s); l = (int)strlen(s); 
  d = cxm_cknull(malloc(sizeof(int)+l+1), "malloc(string)");
  *d = l; strcpy(sdatachars(d), s); return d;
}

int *newstringn(char *s, int n) {
  int *d; char *ns; assert(s); assert(n >= 0);
  d = cxm_cknull(malloc(sizeof(int)+n+1), "malloc(stringn)");
  *d = n; memcpy((ns = sdatachars(d)), s, n); ns[n] = 0; return d;
}

int *allocstring(int n, int c) {
  int *d; char *s; assert(n+1 > 0); 
  d = cxm_cknull(malloc(sizeof(int)+n+1), "malloc(string)");
  *d = n; s = sdatachars(d); memset(s, c, n); s[n] = 0;
  return d;
}

int *substring(int *d0, int from, int to) {
  int n = to-from, *d1; char *s0, *s1; assert(d0);
  assert(0 <= from && from <= to && to <= *d0); 
  d1 = cxm_cknull(malloc(sizeof(int)+n+1), "malloc(string)");
  *d1 = n; s0 = sdatachars(d0); s1 = sdatachars(d1); 
  memcpy(s1, s0+from, n); s1[n] = 0;
  return d1;
}

int *stringcat(int *d0, int *d1) {
  int l0 = *d0, l1 = *d1, n = l0+l1; char *s0, *s1, *s;
  int *d = cxm_cknull(malloc(sizeof(int)+n+1), "malloc(string)");
  *d = n; s = sdatachars(d); s0 = sdatachars(d0); s1 = sdatachars(d1);
  memcpy(s, s0, l0); memcpy(s+l0, s1, l1); s[n] = 0;
  return d;
}

int *dupstring(int *d0) {
  int n = *d0, *d1 = cxm_cknull(malloc(sizeof(int)+n+1), "malloc(string)");
  memcpy(d1, d0, sizeof(int)+n+1);
  return d1;
}

void stringfill(int *d, int c) {
  int l = *d, i; char *s = sdatachars(d);
  for (i = 0; i < l; ++i) s[i] = c;
}

int strcmp_ci(char *s1, char *s2) {
  int c1, c2, d;
  do { c1 = *s1++; c2 = *s2++; d = (unsigned)tolower(c1) - (unsigned)tolower(c2); } while (!d && c1 && c2);
  return d;
}

static cxtype_t cxt_bytevector = { "bytevector", free };

cxtype_t *BYTEVECTOR_NTAG = &cxt_bytevector;

#define mallocbvdata(n) cxm_cknull(malloc(sizeof(int)+(n)), "malloc(bytevector)")

#ifndef NDEBUG
unsigned char* bytevectorref(obj o, int i) {
  int *d = bytevectordata(o); assert(i >= 0 && i < *d); return (bvdatabytes(d))+i;
}
#endif

int *newbytevector(unsigned char *s, int n) {
  int *d; assert(s); assert(n >= 0); 
  d = mallocbvdata(n); *d = n; memcpy(bvdatabytes(d), s, n); return d;
}

int *makebytevector(int n, int c) {
  int *d; assert(n >= 0); 
  d = mallocbvdata(n); *d = n; memset(bvdatabytes(d), c, n);
  return d;
}

int *allocbytevector(int n) {
  int *d = mallocbvdata(n); *d = n; return d;
}

int *dupbytevector(int *d0) {
  int *d1 = mallocbvdata(*d0); *d1 = *d0; 
  memcpy(bvdatabytes(d1), bvdatabytes(d0), *d0); 
  return d1;
}

int bytevectoreq(int *d0, int *d1) {
  int l0 = *d0, l1 = *d1;
  return (l0 != l1) ? 0 : memcmp(bvdatabytes(d0), bvdatabytes(d1), l0) == 0; 
}

int *subbytevector(int *d0, int from, int to) {
  int n = to-from, *d1; unsigned char *s0, *s1; assert(d0);
  assert(0 <= from && from <= to && to <= *d0); 
  d1 = mallocbvdata(n); *d1 = n; s0 = bvdatabytes(d0); s1 = bvdatabytes(d1); 
  memcpy(s1, s0+from, n); return d1;
}

int islist(obj l) {
  obj s = l;
  for (;;) {
    if (isnull(l)) return 1;
    else if (!ispair(l)) return 0;
    else if ((l = cdr(l)) == s) return 0;
    else if (isnull(l)) return 1;
    else if (!ispair(l)) return 0;
    else if ((l = cdr(l)) == s) return 0;
    else s = cdr(s); 
  }
}

static struct { char **a; char ***v; size_t sz; size_t u; size_t maxu; } symt;

static unsigned long hashs(char *s) {
  unsigned long i = 0, l = (unsigned long)strlen(s), h = l;
  while (i < l) h = (h << 4) ^ (h >> 28) ^ s[i++];
  return h ^ (h  >> 10) ^ (h >> 20);
}

char *symbolname(int sym) {
  assert(sym >= 0); assert(sym < (int)symt.u);
  return symt.a[sym];
}

int internsym(char *name) {
  size_t i, j; /* based on a code (C) 1998, 1999 by James Clark. */
  if (symt.sz == 0) { /* init */
    symt.a = cxm_cknull(calloc(64, sizeof(char*)), "symtab[0]");
    symt.v = cxm_cknull(calloc(64, sizeof(char**)), "symtab[1]");
    symt.sz = 64, symt.maxu = 64 / 2;
    i = hashs(name) & (symt.sz-1);
  } else {
    unsigned long h = hashs(name);
    for (i = h & (symt.sz-1); symt.v[i]; i = (i-1) & (symt.sz-1))
      if (strcmp(name, *symt.v[i]) == 0) return (int)(symt.v[i] - symt.a);
    if (symt.u == symt.maxu) { /* rehash */
      size_t nsz = symt.sz * 2;
      char **na = cxm_cknull(calloc(nsz, sizeof(char*)), "symtab[2]");
      char ***nv = cxm_cknull(calloc(nsz, sizeof(char**)), "symtab[3]");
      for (i = 0; i < symt.sz; i++)
        if (symt.v[i]) {
          for (j = hashs(*symt.v[i]) & (nsz-1); nv[j]; j = (j-1) & (nsz-1)) ;
          nv[j] = symt.v[i] - symt.a + na;
        }
      free(symt.v); symt.v = nv; symt.sz = nsz; symt.maxu = nsz / 2;
      memcpy(na, symt.a, symt.u * sizeof(char*)); free(symt.a); symt.a = na; 
      for (i = h & (symt.sz-1); symt.v[i]; i = (i-1) & (symt.sz-1)) ;
    }
  }
  *(symt.v[i] = symt.a + symt.u) = 
    strcpy(cxm_cknull(malloc(strlen(name)+1), "symtab[4]"), name);
  return (int)((symt.u)++);
}

int isprocedure(obj o) {
  if (!o) return 0;
  else if (isaptr(o) && !isobjptr(o)) return 1;
  else if (!isobjptr(o)) return 0;
  else { obj h = objptr_from_obj(o)[-1];
    return notaptr(h) && size_from_obj(h) >= 1 
      && isaptr(hblkref(o, 0)); }
}

int procedurelen(obj o) {
  assert(isprocedure(o));
  return isobjptr(o) ? hblklen(o) : 1;
}

obj* procedureref(obj o, int i) {
  int len; assert(isprocedure(o));
  len = isobjptr(o) ? hblklen(o) : 1;
  assert(i >= 0 && i < len);
  return &hblkref(o, i);   
}

/* shared generic methods */

static void nofree(void *p) {}

static int noclose(void *p) { return 0; }

static int nogetch(void *p) { return EOF; }

static int noungetch(int c) { return c; }

static int noputch(int c, void *p) { return EOF; }

static int noflush(void *p) { return EOF; }

static int noctl(const char *cmd, void *p, ...) { return -1; }

static void ffree(void *vp) {
  /* FILE *fp = vp; assert(fp); cannot fclose(fp) here because of FILE reuse! */ }

sifile_t *sialloc(char *p, void *base) { 
  sifile_t *fp = cxm_cknull(malloc(sizeof(sifile_t)), "malloc(sifile)");
  fp->p = p; fp->base = base; return fp; }

static void sifree(sifile_t *fp) { 
  assert(fp); if (fp->base) free(fp->base); free(fp); }

static int siclose(sifile_t *fp) { 
  assert(fp); if (fp->base) free(fp->base); fp->base = NULL; fp->p = ""; return 0; }

static int sigetch(sifile_t *fp) {
  int c; assert(fp && fp->p); if (!(c = *(fp->p))) return EOF; ++(fp->p); return c; }

static int siungetch(int c, sifile_t *fp) {
  assert(fp && fp->p); --(fp->p); assert(c == *(fp->p)); return c; }

bvifile_t *bvialloc(unsigned char *p, unsigned char *e, void *base) { 
  bvifile_t *fp = cxm_cknull(malloc(sizeof(bvifile_t)), "malloc(bvifile)");
  fp->p = p; fp->e = e; fp->base = base; return fp; }

static void bvifree(bvifile_t *fp) { 
  assert(fp); if (fp->base) free(fp->base); free(fp); }

static int bviclose(bvifile_t *fp) { 
  assert(fp); if (fp->base) free(fp->base); fp->base = NULL; 
  fp->p = fp->e = (unsigned char *)""; return 0; }

static int bvigetch(bvifile_t *fp) {
  assert(fp && fp->p && fp->e); return (fp->p >= fp->e) ? EOF : (0xff & *(fp->p)++); }

static int bviungetch(int c, bvifile_t *fp) {
  assert(fp && fp->p && fp->e); --(fp->p); assert(c == *(fp->p)); return c; }

cbuf_t* newcb(void) {
  cbuf_t* pcb = cxm_cknull(malloc(sizeof(cbuf_t)), "malloc(cbuf)");
  pcb->fill = pcb->buf = cxm_cknull(malloc(64), "malloc(cbdata)");
  pcb->end = pcb->buf + 64; return pcb;
}

void freecb(cbuf_t* pcb) { if (pcb) { free(pcb->buf); free(pcb); } }

static void cbgrow(cbuf_t* pcb, size_t n) {
  size_t oldsz = pcb->end - pcb->buf, newsz = oldsz*2;
  size_t cnt = pcb->fill - pcb->buf;
  if (oldsz + n > newsz) newsz += n;
  pcb->buf = cxm_cknull(realloc(pcb->buf, newsz), "realloc(cbdata)");
  pcb->fill = pcb->buf + cnt, pcb->end = pcb->buf + newsz;
}

int cbputc(int c, cbuf_t* pcb) {
  if ((pcb)->fill == (pcb)->end) cbgrow(pcb, 1); *((pcb)->fill)++ = c; return c;
}

static int cbflush(cbuf_t* pcb) { return 0; }

static int cbclose(cbuf_t* pcb) { free(pcb->buf); pcb->buf = NULL; return 0; }

size_t cblen(cbuf_t* pcb) { return pcb->fill - pcb->buf; }

char* cbdata(cbuf_t* pcb) {
  if (pcb->fill == pcb->end) cbgrow(pcb, 1); *(pcb->fill) = 0; return pcb->buf; 
}

/* port type array */

#define PORTTYPES_MAX 8

static cxtype_port_t cxt_port_types[PORTTYPES_MAX] = {
#define IPORT_CLOSED_PTINDEX     0
  { "closed-input-port", (void (*)(void*))nofree, 
    SPT_CLOSED, (int (*)(void*))noclose,
    (int (*)(void*))nogetch, (int (*)(int, void*))noungetch,
    (int (*)(int, void*))noputch, (int (*)(void*))noflush, 
    (int (*)(const char *, void *, ...))noctl },
#define IPORT_FILE_PTINDEX       1
  { "file-input-port", ffree, 
    SPT_INPUT, (int (*)(void*))fclose, 
    (int (*)(void*))(fgetc), (int (*)(int, void*))(ungetc),
    (int (*)(int, void*))noputch, (int (*)(void*))noflush,
    (int (*)(const char *, void *, ...))noctl },
#define IPORT_STRING_PTINDEX     2
  { "string-input-port", (void (*)(void*))sifree, 
    SPT_INPUT, (int (*)(void*))siclose,
    (int (*)(void*))sigetch, (int (*)(int, void*))siungetch,
    (int (*)(int, void*))noputch, (int (*)(void*))noflush,
    (int (*)(const char *, void *, ...))noctl },
#define IPORT_BYTEVECTOR_PTINDEX 3
  { "bytevector-input-port", (void (*)(void*))bvifree, 
    SPT_INPUT, (int (*)(void*))bviclose,
    (int (*)(void*))bvigetch, (int (*)(int, void*))bviungetch,
    (int (*)(int, void*))noputch, (int (*)(void*))noflush,
    (int (*)(const char *, void *, ...))noctl },
#define OPORT_CLOSED_PTINDEX     4
  { "closed-output-port", (void (*)(void*))nofree, 
    SPT_OUTPUT, (int (*)(void*))noclose, 
    (int (*)(void*))nogetch, (int (*)(int, void*))noungetch,
    (int (*)(int, void*))noputch, (int (*)(void*))noflush,
    (int (*)(const char *, void *, ...))noctl },
#define OPORT_FILE_PTINDEX       5
  { "file-output-port", ffree, 
    SPT_OUTPUT, (int (*)(void*))fclose, 
    (int (*)(void*))nogetch, (int (*)(int, void*))noungetch,
    (int (*)(int, void*))(fputc), (int (*)(void*))fflush,
    (int (*)(const char *, void *, ...))noctl },
#define OPORT_STRING_PTINDEX     6
  { "string-output-port", (void (*)(void*))freecb, 
    SPT_OUTPUT, (int (*)(void*))cbclose,
    (int (*)(void*))nogetch, (int (*)(int, void*))noungetch,
    (int (*)(int, void*))cbputc, (int (*)(void*))cbflush,
    (int (*)(const char *, void *, ...))noctl },
#define OPORT_BYTEVECTOR_PTINDEX 7
  { "bytevector-output-port", (void (*)(void*))freecb, 
    SPT_OUTPUT, (int (*)(void*))cbclose,
    (int (*)(void*))nogetch, (int (*)(int, void*))noungetch,
    (int (*)(int, void*))cbputc, (int (*)(void*))cbflush,
    (int (*)(const char *, void *, ...))noctl }    
};

cxtype_t *IPORT_CLOSED_NTAG = (cxtype_t *)&cxt_port_types[IPORT_CLOSED_PTINDEX];

cxtype_t *IPORT_FILE_NTAG = (cxtype_t *)&cxt_port_types[IPORT_FILE_PTINDEX];

cxtype_t *IPORT_STRING_NTAG = (cxtype_t *)&cxt_port_types[IPORT_STRING_PTINDEX];

cxtype_t *IPORT_BYTEVECTOR_NTAG = (cxtype_t *)&cxt_port_types[IPORT_BYTEVECTOR_PTINDEX];

cxtype_t *OPORT_CLOSED_NTAG = (cxtype_t *)&cxt_port_types[OPORT_CLOSED_PTINDEX];

cxtype_t *OPORT_FILE_NTAG = (cxtype_t *)&cxt_port_types[OPORT_FILE_PTINDEX];

cxtype_t *OPORT_STRING_NTAG = (cxtype_t *)&cxt_port_types[OPORT_STRING_PTINDEX];

cxtype_t *OPORT_BYTEVECTOR_NTAG = (cxtype_t *)&cxt_port_types[OPORT_BYTEVECTOR_PTINDEX];

/* eq hash table for circular/sharing checks and safe equal? */
typedef struct { obj *v; obj *r; size_t sz; size_t u, maxu, c; } stab_t;
static stab_t *staballoc(void) {
  stab_t *p = cxm_cknull(calloc(1, sizeof(stab_t)), "newstab");
  p->v = cxm_cknull(calloc(64, sizeof(obj)), "newstab[1]");
  p->sz = 64, p->maxu = 64 / 2; return p;
}
static stab_t *stabfree(stab_t *p) { 
  if (p) { free(p->v); free(p->r); free(p); }
  return NULL; 
}
static int stabnew(obj o, stab_t *p, int circ) {
  if (!o || notaptr(o) || notobjptr(o) || (circ && isaptr(objptr_from_obj(o)[-1]))) return 0;
  else if (circ && isaptr(objptr_from_obj(o)[0])) return 0; /* opaque */ 
  else { /* v[i] is 0 or heap obj, possibly with lower bit set if it's not new */
    unsigned long h = (unsigned long)o; size_t sz = p->sz, i, j;
    for (i = h & (sz-1); p->v[i]; i = (i-1) & (sz-1))
      if ((p->v[i] & ~1) == o) { p->v[i] |= 1; return 0; }
    if (p->u == p->maxu) { /* rehash */
      size_t nsz = sz * 2; obj *nv = cxm_cknull(calloc(nsz, sizeof(obj)), "stabnew");
      for (i = 0; i < sz; ++i) if (p->v[i] & ~1) {
        for (j = (unsigned long)(p->v[i] & ~1) & (nsz-1); nv[j]; j = (j-1) & (nsz-1)) ;
        nv[j] = p->v[i];
      }
      free(p->v); p->v = nv; sz = p->sz = nsz; p->maxu = nsz / 2;
      for (i = h & (sz-1); p->v[i]; i = (i-1) & (sz-1)) ;
    }
    p->v[i] = o; p->u += 1; return 1; 
  }
}
static void stabdelifu(obj o, stab_t *p) { 
  unsigned long h = (unsigned long)o; size_t sz = p->sz, i;
  for (i = h & (sz-1); p->v[i]; i = (i-1) & (sz-1)) if ((p->v[i] & ~1) == o) { 
    if (p->v[i] & 1) /* keep */; else p->v[i] = 1; /* del */
    return;
  }
}
static void stabpushp(obj o, stab_t *p) {
  obj *r = p->r; if (!r) { p->r = r = cxm_cknull(calloc(sizeof(obj), 12), "stabpushp"); r[1] = 10; }
  else if (r[0] == r[1]) { p->r = r = cxm_cknull(realloc(r, sizeof(obj)*(2+(size_t)r[1]*2)), "stabpushp"); r[1] *= 2; }
  r[2 + r[0]++] = o; 
}
static void stabpopp(stab_t *p) {
  obj *r = p->r; assert(r && r[0] > 0); r[0] -= 1;
}
static void stabcircular(obj o, stab_t *p) {
  tail: if (stabnew(o, p, 1)) {
    obj *op = objptr_from_obj(o), fo = op[-1];
    if (notaptr(fo)) { 
      obj *fop = op + size_from_obj(fo);
      stabpushp(0, p); while (op+1 < fop) stabcircular(*op++, p); stabpopp(p); 
      if (op+1 == fop) { stabpushp(o, p); o = *op; goto tail; }
    } 
  } else {
    obj *r = p->r; if (r) {
      obj *op = r+2, *fop = op+r[0];
      while (fop > op && fop[-1] != 0) stabdelifu(*--fop, p); 
      r[0] = fop - op;
    }
  }
}
static void stabshared(obj o, stab_t *p) {
  tail: if (stabnew(o, p, 0)) {
    obj *op = objptr_from_obj(o), fo = op[-1];
    if (notaptr(fo)) { 
      obj *fop = op + size_from_obj(fo); 
      while (op+1 < fop) stabshared(*op++, p);
      if (op+1 == fop) { o = *op; goto tail; }
    } 
  }
}
static stab_t *stabend(stab_t *p) {
  size_t nz, i, sz = p->sz;
  for (nz = i = 0; i < sz; ++i) if ((p->v[i] & ~1) && (p->v[i] & 1)) ++nz;
  if (nz) {
    size_t nsz, j; obj *nv; for (nsz = 8; nsz < nz*2; nsz *= 2) ;
    nv = cxm_cknull(calloc(nsz, sizeof(obj)), "stabend");
    for (i = 0; i < sz; ++i) if ((p->v[i] & ~1) && (p->v[i] & 1)) {
      for (j = (unsigned long)(p->v[i] & ~1) & (nsz-1); nv[j]; j = (j-1) & (nsz-1)) ;
      nv[j] = p->v[i];
    }
    free(p->v); p->v = nv; sz = p->sz = nsz; p->maxu = nsz / 2;
    free(p->r); p->r = NULL;
  } else p = stabfree(p);
  return p;
}
static long stabri(size_t i, stab_t *p, int upd) {
  obj *pri, ri; if (!p->r) p->r = cxm_cknull(calloc(p->sz, sizeof(obj)), "stabri");
  pri = p->r + i; ri = *pri; if (!ri) *pri = ri = ++(p->c);
  if (upd && ri > 0) *pri = -ri; return (long)ri;
}
static long stabref(obj o, stab_t *p, int upd) {
  if (!p || !o || notaptr(o) || notobjptr(o)) return 0; else {
    unsigned long h = (unsigned long)o; size_t sz = p->sz, i;
    for (i = h & (sz-1); p->v[i]; i = (i-1) & (sz-1))
      if ((p->v[i] & ~1) == o) return (p->v[i] & 1) ? stabri(i, p, upd) : 0; 
    return 0;
  }
}
static int stabufind(obj x, obj y, stab_t *p) {
  size_t sz = p->sz, i, ix=0, iy=0; /* bogus 0 inits to silence gcc */ obj *r = p->r;
  for (i = (unsigned long)x & (sz-1); p->v[i]; i = (i-1) & (sz-1)) if ((p->v[i] & ~1) == x) { ix = i; break; }
  for (i = ix; r[i] >= 0; ) i = (size_t)r[i]; if (i != ix) ix = (size_t)(r[ix] = i);  
  for (i = (unsigned long)y & (sz-1); p->v[i]; i = (i-1) & (sz-1)) if ((p->v[i] & ~1) == y) { iy = i; break; }
  for (i = iy; r[i] >= 0; ) i = (size_t)r[i]; if (i != iy) iy = (size_t)(r[iy] = i);
  if (ix == iy) return 1; /* same class, assumed to be equal */ 
  if (r[ix] < r[iy]) { r[ix] += r[iy]; r[iy] = ix; } else { r[iy] += r[ix]; r[ix] = iy; } return 0;
}
static int stabequal(obj x, obj y, stab_t *p) {
  obj h; int i, n; loop: if (x == y) return 1;
  if (!x || !y || notaptr(x) || notaptr(y) || notobjptr(x) || notobjptr(y)) return 0;
  if ((h = objptr_from_obj(x)[-1]) != objptr_from_obj(y)[-1]) return 0;
#ifdef FLONUMS_BOXED
  if (h == (obj)FLONUM_NTAG) return flonum_from_obj(x) == flonum_from_obj(y); 
#endif
  if (h == (obj)STRING_NTAG) return strcmp(stringchars(x), stringchars(y)) == 0;
  if (h == (obj)BYTEVECTOR_NTAG) return bytevectoreq(bytevectordata(x), bytevectordata(y)); 
  if (isaptr(h) || !(n = size_from_obj(h)) || hblkref(x, 0) != hblkref(y, 0)) return 0;
  if (stabufind(x, y, p)) return 1; /* seen before and decided to be equal */
  for (i = 1; i < n-1; ++i) if (!stabequal(hblkref(x, i), hblkref(y, i), p)) return 0;
  if (i == n-1) { x = hblkref(x, i); y = hblkref(y, i); goto loop; } else return 1; 
}
static int boundequal(obj x, obj y, int fuel) { /* => remaining fuel or <0 on failure */
  obj h; int i, n; loop: assert(fuel > 0); if (x == y) return fuel-1;
  if (!x || !y || notaptr(x) || notaptr(y) || notobjptr(x) || notobjptr(y)) return -1;
  if ((h = objptr_from_obj(x)[-1]) != objptr_from_obj(y)[-1]) return -1;
#ifdef FLONUMS_BOXED
  if (h == (obj)FLONUM_NTAG) return flonum_from_obj(x) == flonum_from_obj(y) ? fuel-1 : -1; 
#endif
  if (h == (obj)STRING_NTAG) return strcmp(stringchars(x), stringchars(y)) == 0 ? fuel-1 : -1;
  if (h == (obj)BYTEVECTOR_NTAG) return bytevectoreq(bytevectordata(x), bytevectordata(y)) ? fuel-1 : -1;
  if (isaptr(h) || !(n = size_from_obj(h)) || hblkref(x, 0) != hblkref(y, 0)) return -1;
  if (--fuel == 0) return 0; /* we must spend fuel while comparing objects themselves */
  for (i = 1; i < n-1; ++i) if ((fuel = boundequal(hblkref(x, i), hblkref(y, i), fuel)) <= 0) return fuel;
  if (i == n-1) { x = hblkref(x, i); y = hblkref(y, i); goto loop; } else return fuel;
}

int iscircular(obj x) {
  if (!x || notaptr(x) || notobjptr(x)) return 0;
  else { stab_t *p = staballoc(); stabcircular(x, p); p = stabend(p); stabfree(p); return p != NULL; }
}

int iseqv(obj x, obj y) {
  obj h; if (x == y) return 1;
  if (!x || !y || notaptr(x) || notaptr(y) || notobjptr(x) || notobjptr(y)) return 0;
  if ((h = objptr_from_obj(x)[-1]) != objptr_from_obj(y)[-1]) return 0;
#ifdef FLONUMS_BOXED
  if (h == (obj)FLONUM_NTAG) return *(flonum_t*)objptr_from_obj(x)[0] == *(flonum_t*)objptr_from_obj(y)[0]; 
#endif
  return 0;
}

obj ismemv(obj x, obj l) {
  if (!x || notaptr(x) || notobjptr(x)) {
    for (; l != mknull(); l = cdr(l)) 
      { if (car(l) == x) return l; }
  } else if (is_flonum_obj(x)) {
    flonum_t fx = flonum_from_obj(x); 
    for (; l != mknull(); l = cdr(l)) 
      { obj y = car(l); if (is_flonum_obj(y) && fx == flonum_from_obj(y)) return l; }
  } else { /* for others, memv == memq */
    for (; l != mknull(); l = cdr(l)) 
      { if (car(l) == x) return l; }
  } return 0;
}

obj isassv(obj x, obj l) {
  if (!x || notaptr(x) || notobjptr(x)) {
    for (; l != mknull(); l = cdr(l)) 
      { obj p = car(l); if (car(p) == x) return p; }
  } else if (is_flonum_obj(x)) {
    flonum_t fx = flonum_from_obj(x); 
    for (; l != mknull(); l = cdr(l)) 
      { obj p = car(l), y = car(p); if (is_flonum_obj(y) && fx == flonum_from_obj(y)) return p; }
  } else { /* for others, assv == assq */
    for (; l != mknull(); l = cdr(l)) 
      { obj p = car(l); if (car(p) == x) return p; }
  } return 0;
}

int isequal(obj x, obj y) {
  stab_t *p; obj *r; size_t i; int res = boundequal(x, y, 500);
  if (res != 0) return res > 0; /* small/non-circular/easy */
  p = staballoc(); stabshared(x, p); stabshared(y, p);
  r = p->r = cxm_cknull(calloc(p->sz, sizeof(obj)), "isequal");
  for (i = 0; i < p->sz; ++i) if (p->v[i] & ~1) r[i] = -1; 
  res = stabequal(x, y, p); stabfree(p); return res;
}

obj ismember(obj x, obj l) {
  if (!x || notaptr(x) || notobjptr(x)) {
    for (; l != mknull(); l = cdr(l)) 
      { if (car(l) == x) return l; }
  } else if (is_flonum_obj(x)) {
    flonum_t fx = flonum_from_obj(x); 
    for (; l != mknull(); l = cdr(l)) 
      { obj y = car(l); if (is_flonum_obj(y) && fx == flonum_from_obj(y)) return l; }
  } else if (isstring(x)) {
    char *xs = stringchars(x);
    for (; l != mknull(); l = cdr(l)) 
      { obj y = car(l); if (isstring(y) && 0 == strcmp(xs, stringchars(y))) return l; }
  } else {
    for (; l != mknull(); l = cdr(l)) 
      { if (isequal(car(l), x)) return l; }
  } return 0;
}

obj isassoc(obj x, obj l) {
  if (!x || notaptr(x) || notobjptr(x)) {
    for (; l != mknull(); l = cdr(l)) 
      { obj p = car(l); if (car(p) == x) return p; }
  } else if (is_flonum_obj(x)) {
    flonum_t fx = flonum_from_obj(x); 
    for (; l != mknull(); l = cdr(l)) 
      { obj p = car(l), y = car(p); if (is_flonum_obj(y) && fx == flonum_from_obj(y)) return p; }
  } else if (isstring(x)) {
    char *xs = stringchars(x);
    for (; l != mknull(); l = cdr(l)) 
      { obj p = car(l), y = car(p); if (isstring(y) && 0 == strcmp(xs, stringchars(y))) return p; }
  } else {
    for (; l != mknull(); l = cdr(l)) 
      { obj p = car(l); if (isequal(car(p), x)) return p; }
  } return 0;
}

/* internal recursive write procedure */
typedef struct { stab_t *pst; int disp; cxtype_oport_t *vt; void *pp; } wenv_t;
static void wrc(int c, wenv_t *e) { e->vt->putch(c, e->pp); }
static void wrs(char *s, wenv_t *e) {
  cxtype_oport_t *vt = e->vt; void *pp = e->pp;
  assert(vt); while (*s) vt->putch(*s++, pp);
}
static int cleansymname(char *s) {
  char *inits = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!$%&*/:<=>?@^_~";
  char *subss = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!$%&*/:<=>?@^_~0123456789.@+-";
  if (s[0] == 0 || s[strspn(s, subss)] != 0) return 0; else if (strchr(inits, s[0])) return 1;
  else if (s[0] == '+' || s[0] == '-') return s[1] == 0 || (s[1] == '.' && s[2] && !isdigit(s[2])) || !isdigit(s[1]);
  else return s[0] == '.' && s[1] && !isdigit(s[1]); 
}
static void wrdatum(obj o, wenv_t *e) {
  long ref;
  tail: ref = stabref(o, e->pst, 1); /* update ref after access */
  if (ref < 0) { char buf[30]; sprintf(buf, "#%ld#", -ref-1); wrs(buf, e); return; }
  if (ref > 0) { char buf[30]; sprintf(buf, "#%ld=", +ref-1); wrs(buf, e); }
  if (is_bool_obj(o)) {
    wrs(bool_from_obj(o) ? "#t" : "#f", e);
  } else if (is_fixnum_obj(o)) {
    char buf[30]; sprintf(buf, "%ld", fixnum_from_obj(o)); wrs(buf, e);
  } else if (is_flonum_obj(o)) {
    char buf[30], *s; double d = flonum_from_obj(o); sprintf(buf, "%.15g", d);
    for (s = buf; *s != 0; s++) if (strchr(".eE", *s)) break;
    if (d != d) strcpy(buf, "+nan.0"); else if (d <= -HUGE_VAL) strcpy(buf, "-inf.0");
    else if (d >= HUGE_VAL) strcpy(buf, "+inf.0"); else if (*s == 'E') *s = 'e'; 
    else if (*s == 0) { *s++ = '.'; *s++ = '0'; *s = 0; }
    wrs(buf, e);
  } else if (iseof(o)) {
    wrs("#<eof>", e);
  } else if (isvoid(o)) {
    wrs("#<void>", e);
  } else if (isshebang(o)) {
    char *s = symbolname(getshebang(o));
    wrs("#<!", e); wrs(s, e); wrc('>', e);
  } else if (o == obj_from_unit()) {
    wrs("#<values>", e);
  } else if (isiport(o)) {
    char buf[60]; sprintf(buf, "#<%s>", ckiportvt(o)->tname); wrs(buf, e);
  } else if (isoport(o)) {
    char buf[60]; sprintf(buf, "#<%s>", ckoportvt(o)->tname); wrs(buf, e);
  } else if (issymbol(o)) {
    char *s = symbolname(getsymbol(o));
    if (e->disp || cleansymname(s)) wrs(s, e);
    else {
      wrc('|', e);
      while (*s) {
        int c = *s++;
        switch(c) {
          case '|': wrs("\\|", e); break;
          case '\\': wrs("\\\\", e); break;
          default: wrc(c, e); break;
        }
      }
      wrc('|', e);
    }

  } else if (isnull(o)) {
    wrs("()", e);
  } else if (ispair(o)) {
    wrc('(', e); wrdatum(car(o), e);
    while (ispair(cdr(o)) && !stabref(cdr(o), e->pst, 0)) { wrc(' ', e); o = cdr(o);  wrdatum(car(o), e); }
    if (!isnull(cdr(o))) { wrs(" . ", e); wrdatum(cdr(o), e); }
    wrc(')', e);
  } else if (is_char_obj(o)) {
    int c = char_from_obj(o);
    if (e->disp) wrc(c, e);
    else switch(c) {
      case 0x00: wrs("#\\null", e); break;
      case 0x07: wrs("#\\alarm", e); break;
      case 0x08: wrs("#\\backspace", e); break;
      case 0x7f: wrs("#\\delete", e); break;
      case 0x1b: wrs("#\\escape", e); break;
      case '\t': wrs("#\\tab", e); break;
      case '\n': wrs("#\\newline", e); break;
      case '\r': wrs("#\\return", e); break;
      case ' ': wrs("#\\space", e); break;
      default: wrs("#\\", e); wrc(c, e); break;
    }
  } else if (isstring(o)) {
    char *s = stringchars(o);
    if (e->disp) wrs(s, e);
    else {
      wrc('\"', e);
      while (*s) {
        int c = *s++;
        switch(c) {
          case '\"': wrs("\\\"", e); break;
          case '\\': wrs("\\\\", e); break;
          default: wrc(c, e); break;
        }
      }
      wrc('\"', e);
    }
  } else if (isvector(o)) {
    int i, n = vectorlen(o);
    wrs("#(", e);
    for (i = 0; i < n; ++i) { 
      if (i) wrc(' ', e); wrdatum(vectorref(o, i), e); 
    }
    wrc(')', e);
  } else if (isbytevector(o)) {
    int i, n = bytevectorlen(o);
    wrs("#u8(", e);
    for (i = 0; i < n; ++i) { 
      char buf[30]; sprintf(buf, "%d", *bytevectorref(o, i));
      if (i) wrc(' ', e); wrs(buf, e); 
    }
    wrc(')', e);
  } else if (isbox(o)) {
    wrs("#&", e); o = boxref(o); goto tail;
  } else if (istagged(o, 0)) {
    int i, n = taggedlen(o, 0);
    wrs("#<values", e);
    for (i = 0; i < n; ++i) { 
      wrc(' ', e); wrdatum(*taggedref(o, 0, i), e); 
    }
    wrc('>', e);
  } else if (isprocedure(o)) {
    char buf[60];
    if (isobjptr(hblkref(o, 0))) sprintf(buf, "#<vmclosure @%p>", objptr_from_obj(o));
    else sprintf(buf, "#<procedure @%p>", objptr_from_obj(o)); 
    wrs(buf, e);
  } else if (isrecord(o)) {
    int i, n = recordlen(o);
    wrs("#<record ", e);
    wrdatum(recordrtd(o), e); // TODO: no need to show as shared!
    for (i = 0; i < n; ++i) { 
      wrc(' ', e); wrdatum(recordref(o, i), e); 
    }
    wrc('>', e);
  } else {
    wrs("#<unknown>", e);
  }
}

/* S-expression writers */
void oportputsimple(obj x, obj p, int disp) {
  wenv_t e; e.pst = NULL; e.disp = disp; e.vt = oportvt(p); e.pp = oportdata(p);
  wrdatum(x, &e);
}
void oportputcircular(obj x, obj p, int disp) {
  wenv_t e; e.pst = staballoc(); e.disp = disp; e.vt = oportvt(p); e.pp = oportdata(p);
  stabcircular(x, e.pst); e.pst = stabend(e.pst);
  wrdatum(x, &e);
  stabfree(e.pst);
}
void oportputshared(obj x, obj p, int disp) {
  wenv_t e; e.pst = staballoc(); e.disp = disp; e.vt = oportvt(p); e.pp = oportdata(p);
  stabshared(x, e.pst); e.pst = stabend(e.pst);
  wrdatum(x, &e);
  stabfree(e.pst);
}

/* system-dependent extensions */


extern int is_tty_port(obj o)
{
  FILE *fp = NULL;
  if ((cxtype_t*)iportvt(o) == IPORT_FILE_NTAG) fp = (FILE*)iportdata(o);
  else if ((cxtype_t*)oportvt(o) == OPORT_FILE_NTAG) fp = (FILE*)oportdata(o); 
  if (!fp) return 0;
  return isatty(fileno(fp));
}

#ifdef WIN32
int dirsep = '\\';
#else
int dirsep = '/';
#endif

extern char *argv_ref(int idx)
{
  char **pv = cxg_argv;
  /* be careful with indexing! */
  if (idx < 0) return NULL;
  while (idx-- > 0) if (*pv++ == NULL) return NULL;
  return *pv;
}

#if defined(WIN32)
#define cxg_envv _environ
#elif defined(__linux) || defined(__APPLE__)
#define cxg_envv environ
#else /* add more systems? */
char **cxg_envv = { NULL };
#endif

extern char *envv_ref(int idx)
{
  char **pv = cxg_envv;
  /* be careful with indexing! */
  if (idx < 0) return NULL;
  while (idx-- > 0) if (*pv++ == NULL) return NULL;
  return *pv;
}

extern char *get_cwd(void)
{
  static char buf[FILENAME_MAX];
  if (getcwd(buf, FILENAME_MAX) == NULL) return NULL;
  return buf;
}

extern int set_cwd(char *cwd)
{
  return chdir(cwd);
}


