/* n_tower.c -- numerical tower */

/* complex number arithmetic */

#ifdef HAVE_C99_COMPLEX
#include <complex.h>

/* Convenience macro: pack two doubles into a _Complex double */
#define CMPLX2(r,i)  ((r) + (i)*_Complex_I)
#define UNPACK(z,pr,pi)  do { *(pr) = creal(z); *(pi) = cimag(z); } while(0)

void cmath_exp(double rx, double ix, double *prz, double *piz)
{ double complex z = cexp(CMPLX2(rx,ix)); UNPACK(z,prz,piz); }

void cmath_log(double rx, double ix, double *prz, double *piz)
{
  double complex z = clog(CMPLX2(rx, ix));
  double re = creal(z), im = cimag(z);
  /* clog(-0.0 +/- i*0.0) should have imag = +/i pi, not 0 */
  if (re <= -HUGE_VAL) {
    if (rx == 0.0 && 1.0/rx < 0.0)
      im = (ix == 0.0 && 1.0/ix < 0.0) ? -M_PI : M_PI;
    else if (rx == 0.0)
      im = (ix == 0.0 && 1.0/ix < 0.0) ? -0.0 : 0.0; 
  }
  *prz = re; *piz = im;
}

void cmath_sqrt(double rx, double ix, double *prz, double *piz)
{ double complex z = csqrt(CMPLX2(rx,ix)); UNPACK(z,prz,piz); }

void cmath_sin(double rx, double ix, double *prz, double *piz)
{ double complex z = csin(CMPLX2(rx,ix)); UNPACK(z,prz,piz); }

void cmath_cos(double rx, double ix, double *prz, double *piz)
{ double complex z = ccos(CMPLX2(rx,ix)); UNPACK(z,prz,piz); }

void cmath_tan(double rx, double ix, double *prz, double *piz)
{
  double complex z = ctan(CMPLX2(rx, ix));
  double re = creal(z), im = cimag(z);
  /* preserve signs of zero if incorrect */
  if (re == 0.0 && rx == 0.0 && 1.0/rx < 0.0) re = -0.0;
  if (im == 0.0 && ix == 0.0 && 1.0/ix < 0.0) im = -0.0;
  *prz = re; *piz = im;
}

void cmath_asin(double rx, double ix, double *prz, double *piz)
{ double complex z = casin(CMPLX2(rx,ix)); UNPACK(z,prz,piz); }

void cmath_acos(double rx, double ix, double *prz, double *piz)
{ double complex z = cacos(CMPLX2(rx,ix)); UNPACK(z,prz,piz); }

void cmath_atan(double rx, double ix, double *prz, double *piz)
{ 
  double complex z = catan(CMPLX2(rx,ix)); 
  double re = creal(z); double im = cimag(z);
  /* catan(-0.0 + y*i) should return -pi/2, not of pi/2 */
  if (rx == 0.0 && 1.0/rx < 0.0 && ix > 0.0 && re > 0.0) re = -re;
  *prz = re; *piz = im;
}

/* pow(z, w) = exp(w * log(z)) */
void cmath_pow(double rx, double ix, double ry, double iy, double *prz, double *piz)
{
  double lr, li, pr, pi;
  if (iy == 0.0) {
    if      (ry ==  0.0) { *prz = 1.0; *piz = 0.0; return; }
    else if (ry ==  0.5) { cmath_sqrt(rx,ix, prz,piz); return; }
    else if (ry ==  1.0) { *prz = rx;  *piz = ix;  return; }
    else if (ry ==  2.0) { *prz = rx*rx-ix*ix; *piz = 2.0*rx*ix; return; }
    else if (ry == -1.0) { double complex z = 1.0/CMPLX2(rx,ix); UNPACK(z,prz,piz); return; }
  }
  cmath_log(rx, ix, &lr, &li); /* log(z) */
  pr = ry*lr - iy*li; pi = ry*li + iy*lr; /* w * log(z) */
  cmath_exp(pr, pi, prz, piz); /* exp(...) */
}

#define cmath_hypot hypot

#else  /* C90 fallback */

#ifndef C99_MATH_LIB
/* helper to copy sign of y to x, correctly handling signed zero */
static double c90_copysign(double x, double y) 
{
  if (y < 0.0) return -fabs(x);
  if (y > 0.0) return fabs(x);
  return (1.0/y < 0.0) ? -fabs(x): fabs(x);
}
#define copysign(x, y) c90_copysign(x, y)
#endif

#ifndef C99_MATH_LIB
/* C90 log1p replacement, good near zero. Requires x > -1. */
static double c90_log1p(double x)
{ /* volatile to prevent optimizations */
  volatile double u;
  if (x == 0.0) return x;
  u = 1.0 + x;
  if (u == 1.0) return x;
  return log(u) * (x / (u - 1.0));
}
#define log1p(x) c90_log1p(x)
#endif

#ifndef C99_MATH_LIB
/* stable hypotenuse: sqrt(a^2+b^2) without overflow */
static double c90_hypot(double a, double b)
{
  double aa = fabs(a), bb = fabs(b), t;
  if (aa == 0.0) return bb;
  if (bb == 0.0) return aa;
  if (aa < bb) { t = aa; aa = bb; bb = t; } /* ensure aa >= bb */
  t = bb / aa;
  return aa * sqrt(1.0 + t*t);
}
#define hypot(a, b) c90_hypot(a, b)
#endif

/* complex divide: (a+ib)/(c+id) - Smith's method for stability */
static void c90_cdiv(double a, double b, double c, double d, double *pr, double *pi)
{
  double r, denom;
  if (fabs(c) >= fabs(d)) {
    r     = d / c;
    denom = c + r * d;
    *pr   = (a + r * b) / denom;
    *pi   = (b - r * a) / denom;
  } else {
    r     = c / d;
    denom = d + r * c;
    *pr   = (a * r + b) / denom;
    *pi   = (b * r - a) / denom;
  }
}

/* stable real asinh for C90 */
static double c90_asinh(double x)
{
  double ax, r, s;
  ax = fabs(x);
  if (ax == 0.0) return x;
  if (ax > 1.0e154) {
    r = log(ax) + M_LN2;
  } else {
    s = sqrt(1.0 + ax * ax);
    r = log1p(ax + (ax * ax) / (1.0 + s));
  }
  return copysign(r, x);
}

/* exp(z) = e^rx * (cos(ix) + i*sin(ix)) */
void cmath_exp(double rx, double ix, double *prz, double *piz)
{
  double er; int ix_neg = (ix < 0.0) || (ix == 0.0 && 1.0/ix < 0.0);
  /* handle ix = +/-0: exp(x +/- i0) = exp(x) +/- i0 (preserves sign of zero) */
  if (ix == 0.0) { *prz = exp(rx); *piz = ix; return; }
  /* handle rx = NaN: exp(NaN + iy) = NaN + i*NaN, unless y=0 (handled above) */
  if (rx != rx) { *prz = rx; *piz = ix; return; }
  er = exp(rx); /* overflow, but |ix| < 1: use logarithmic scaling */
  if (er == HUGE_VAL && fabs(ix) < 1.0) {
    double log_ix = log(fabs(ix)), scaled = rx + log_ix;
    *prz = HUGE_VAL; /* use log(DBL_MAX) =~ 709.0  */
    if (scaled < 709.0) *piz = (ix_neg ? -1.0 : 1.0) * exp(scaled);
    else *piz = ix_neg ? -HUGE_VAL : HUGE_VAL;
    return;
  }
  *prz = er * cos(ix);
  *piz = er * sin(ix);
}

/* log(z) = ln|z| + i*atan2(ix, rx) (principal branch) */
void cmath_log(double rx, double ix, double *prz, double *piz)
{
  double ar = fabs(rx), ai = fabs(ix), t;
  if (ar == 0.0 && ai == 0.0) {
    *prz = -HUGE_VAL;
  } else {
    if (ar < ai) { t = ar; ar = ai; ai = t; }
    t = ai / ar;
    *prz = log(ar) + 0.5 * log(1.0 + t*t);
  }
  *piz = atan2(ix, rx);
}

/* sqrt(z) - principal root, Re(result) >= 0 (Numerical Recipes 5.4) */
void cmath_sqrt(double rx, double ix, double *prz, double *piz)
{
  double m, re, im;
  int ix_neg = (ix < 0.0) || (ix == 0.0 && 1.0/ix < 0.0);
  int rx_neg = (rx < 0.0) || (rx == 0.0 && 1.0/rx < 0.0);
  if (rx != rx || ix != ix) {
    /* NaNs spoil both parts of the result */
    *prz = *piz = HUGE_VAL-HUGE_VAL;
    return;
  } 
  if (ix == HUGE_VAL || ix == -HUGE_VAL) {
    /* csqrt(x +/- i*inf) = inf +/- i*inf */
    *prz = HUGE_VAL; *piz = ix_neg ? -HUGE_VAL : HUGE_VAL;
    return;
  }
  if (rx == HUGE_VAL) {
    /* csqrt(+inf + i*y) = inf + i*0 (preserves sign of y) */
    *prz = HUGE_VAL; *piz = ix_neg ? -0.0 : 0.0;
    return;
  }
  if (rx == -HUGE_VAL) {
    /* csqrt(-inf + i*y) = 0 + i*inf (preserves sign of y) */
    *prz = 0.0; *piz = ix_neg ? -HUGE_VAL : HUGE_VAL;
    return;
  }
  m = hypot(rx, ix);
  if (!rx_neg) {
    re = sqrt((m + rx) * 0.5);
    im = (re == 0.0) ? (ix_neg ? -0.0 : 0.0) : ix / (2.0 * re);
  } else {
    im = sqrt((m - rx) * 0.5);
    if (ix_neg) im = -im;
    re = (im == 0.0) ? 0.0 : ix / (2.0 * im);
  }
  *prz = re;
  *piz = im;
}

/* cosh(t) = (e^t + e^-t) / 2, sinh(t) = (e^t - e^-t) / 2 */
static void c90_sinhcosh(double t, double *psh, double *pch)
{
  double ep = exp(t), em = exp(-t);
  *psh = (ep - em) * 0.5;
  *pch = (ep + em) * 0.5;
}

/* sin(z) = sin(rx)*cosh(ix) + i*cos(rx)*sinh(ix) */
void cmath_sin(double rx, double ix, double *prz, double *piz)
{
  double sh, ch;
  c90_sinhcosh(ix, &sh, &ch);
  *prz = sin(rx) * ch;
  *piz = cos(rx) * sh;
}

/* cos(z) = cos(rx)*cosh(ix) - i*sin(rx)*sinh(ix) */
void cmath_cos(double rx, double ix, double *prz, double *piz)
{
  double sh, ch;
  c90_sinhcosh(ix, &sh, &ch);
  *prz =  cos(rx) * ch;
  *piz = -sin(rx) * sh;
}

/* tan(z) = sin(z)/cos(z) */
void cmath_tan(double rx, double ix, double *prz, double *piz) 
{
  if (fabs(ix) > 20.0) { /* tan(x + iy) converges to sign(y) * i */
    *prz = 0.0; *piz = (ix > 0.0) ? 1.0 : -1.0;
  } else {
    double cos_rx = cos(rx), sinh_ix = sinh(ix);
    double d = cos_rx * cos_rx + sinh_ix * sinh_ix;
    if (d == 0.0) { /* singular pole at rx = pi/2 + k*pi, ix = 0 */
      *prz = 0.0; *piz = 0.0;
    } else {
      *prz = (sin(rx) * cos_rx) / d;
      *piz = (sinh_ix * cosh(ix)) / d;
    }
  }
}

/* asin(z) = -i * log(i*z + sqrt(1 - z^2)) */
void cmath_asin(double rx, double ix, double *prz, double *piz)
{
  double ar, ai, br, bi, den, arg;
  cmath_sqrt(1.0 - rx, -ix, &ar, &ai);
  cmath_sqrt(1.0 + rx, ix, &br, &bi);
  den = ar * br - ai * bi;
  arg = ar * bi - ai * br;
  *prz = atan2(rx, den);
  *piz = c90_asinh(arg);
}

/* acos(z) = -i * log(z + i*sqrt(1 - z^2)) */
void cmath_acos(double rx, double ix, double *prz, double *piz) 
{
  double temp_r, temp_i;
  cmath_asin(rx, ix, &temp_r, &temp_i);
  *prz = M_PI_2 - temp_r;
  *piz = -temp_i;
}

#define C90_BIG 1.0e154

/* atan(z) = (i/2) * log((1 - i*z) / (1 + i*z)) */
void cmath_atan(double rx, double ix, double *pre, double *pim)
{
  double x = rx, y = ix, ax = fabs(x), ay = fabs(y), re, im;
  /* corner case: isinf(x) */
  if (x <= -HUGE_VAL || x >= HUGE_VAL) {
    *pre = copysign(M_PI_2, x);
    *pim = copysign(0.0, y);
    return;
  }
  /* re part */
  if (x == 0.0) {
    if (ay > 1.0) re = copysign(M_PI_2, x);
    else re = x; /* preserve +0.0 / -0.0 */
  } else if (ax >= C90_BIG || ay >= C90_BIG) {
    re = copysign(M_PI_2, x);
  } else {
    double den;
    if (ax < ay) den = (1.0 - ay) * (1.0 + ay) - x * x;
    else den = (1.0 - ax) * (1.0 + ax) - y * y;
    re = 0.5 * atan2(2.0 * x, den);
  }
  /* im part */
  if (fabs(ay - 1.0) < 0.5) { /* avoid losing x*x */
    double hp = hypot(x, y + 1.0);
    double hm = hypot(x, y - 1.0);
    if (hp == 0.0) im = -HUGE_VAL; /* z = -i */
    else if (hm == 0.0) im = HUGE_VAL;  /* z = +i */
    else im = 0.5 * (log(hp) - log(hm));
  } else { /* 1/4 log1p(4y / (x^2 + (y-1)^2)) */
    double ym = y - 1.0, h = hypot(x, ym), t;
    if (h == 0.0) im = HUGE_VAL;  /* z = +i */
    else {
      if (h >= C90_BIG) { /* avoid h*h overflow */
        t = (4.0 / h) * (y / h);
      } else {
        double den = x * x + ym * ym;
        t = 4.0 * y / den;
      }
      im = 0.25 * log1p(t);
    }
  }
  *pre = re;
  *pim = im;
}

/* pow(z, w) = exp(w * log(z)) */
void cmath_pow(double rx, double ix, double ry, double iy, double *prz, double *piz)
{
  double lr, li, pr, pi;
  if (iy == 0.0) {
    if      (ry ==  0.0) { *prz = 1.0; *piz = 0.0; return; }
    else if (ry ==  0.5) { cmath_sqrt(rx,ix, prz,piz); return; }
    else if (ry ==  1.0) { *prz = rx;  *piz = ix;  return; }
    else if (ry ==  2.0) { *prz = rx*rx-ix*ix; *piz = 2.0*rx*ix; return; }
    else if (ry == -1.0) { c90_cdiv(1.0,0.0, rx,ix, prz,piz); return; }
  }
  cmath_log(rx, ix, &lr, &li); /* log(z) */
  pr = ry*lr - iy*li; pi = ry*li + iy*lr; /* w * log(z) */
  cmath_exp(pr, pi, prz, piz); /* exp(...) */
}


/* hypot(x, y) = sqrt(x*x + y*y) */
double cmath_hypot(double x, double y)
{
  double ax = fabs(x), ay = fabs(y), t;
  if (ax != ax) return ax; /* NaN */
  if (ay != ay) return ay; /* NaN */
  if (ax < ay) { t = ax; ax = ay; ay = t; }
  /* if ax is +inf, return +inf (even if ay is NaN).
  * if ax is 0.0, then ay is also 0.0 (since ax >= ay >= 0). */
  if (ax + ax == ax) return ax;
  /* if ay is +inf here, ax must be NaN
  * C99 requires hypot(NaN, inf) = inf. */
  if (ay + ay == ay && ay > 0.0) return ay;
  /* we can go with C90 version now */
  return hypot(x, y);
}

#endif /* HAVE_C99_COMPLEX */


/* validating generic number syntax */

/* skips return a pointer after the part they were able to 
 * match; int *pflags is set to a combination of CNF_XXX */

#define CNF_DOTEXP 1  /* dot or exponent */
#define CNF_IMPART 2  /* imaginary part  */
#define CNF_INFNAN 4  /* infinity or nan */
#define CNF_POLAR  8  /* polar notation  */
#define CNF_NONR7  16 /* extended syntax */

/* expects radix to be 2, 8, 10, or 16 */
static char *skip_uinteger(const char *s, int radix)
{
  assert(radix >= 2 && radix <= 36);
  if (radix > 10) {
    char limit_lc = 'a' + radix - 10;
    char limit_uc = 'A' + radix - 10;
    while ((*s >= '0' && *s <= '9') ||
           (*s >= 'a' && *s <= limit_lc) ||
           (*s >= 'A' && *s <= limit_uc)) ++s;
  } else {
    char limit = '0' + radix;
    while (*s >= '0' && *s < limit) ++s;
  }
  return (char *)s;
}

/* errors: stops at start or exponent if it is invalid */
static char *skip_decimal(const char *s, int radix, int *pflags)
{
  int flags = 0;
  const char *t, *e;
  if (*s == '.') {
    flags = CNF_DOTEXP;
    t = skip_uinteger(s + 1, radix);
    if (t == s + 1) return (char *)s;
    s = t;
  } else {
    t = skip_uinteger(s, radix);
    if (t == s) return (char *)s;
    s = t;
    if (*s == '.') {
      flags = CNF_DOTEXP;
      s = skip_uinteger(s + 1, radix);
    }
  }
  if (radix < 15 && tolower(*s) == 'e') {
    flags = (radix == 10) ? CNF_DOTEXP : CNF_DOTEXP|CNF_NONR7;
    e = s + 1;
    if (*e == '+' || *e == '-') ++e;
    t = skip_uinteger(e, radix);
    if (t > e) s = t;
  } else if (radix < 25 && tolower(*s) == 'p') {
    flags = CNF_DOTEXP|CNF_NONR7;
    e = s + 1;
    if (*e == '+' || *e == '-') ++e;
    t = skip_uinteger(e, 10);
    if (t > e) s = t;
  }  
  if (flags) *pflags |= flags;
  return (char *)s;
}

/* errors: stops at unparsed part or broken decimal exponent */
static char *skip_ureal(const char *s, int radix, int *pflags)
{
  const char *t; int flags = 0;
  if (radix == 2 || radix == 4 || radix == 8 || radix == 10) {
    t = skip_decimal(s, radix, &flags);
    if (t == s) return (char *)s;
    if (flags & CNF_DOTEXP) /* [.eEpP] */ 
      return *pflags |= flags, (char *)t;
  } else if (radix == 16) {
    t = skip_decimal(s, radix, &flags);
    if (t == s) return (char *)s;
    if (flags & CNF_DOTEXP) /* [.pP] */ 
      return *pflags |= flags, (char *)t;
  } else {
    t = skip_uinteger(s, radix);
    if (t == s) return (char *)s;
  }
  if (*t == '/') {
    const char *u = skip_uinteger(t + 1, radix);
    if (u > t + 1) return (char *)u;
  }
  return (char *)t;
}

/* errors: stops at start if no match */
static char *skip_infnan(const char *s, int *pflags)
{
  /* '+inf.0' | '-inf.0' | '+nan.0' | '-nan.0' */
  if (*s != '+' && *s != '-') return (char *)s;
  if (!((tolower((unsigned char)s[1]) == 'i' &&
         tolower((unsigned char)s[2]) == 'n' &&
         tolower((unsigned char)s[3]) == 'f') ||
        (tolower((unsigned char)s[1]) == 'n' &&
         tolower((unsigned char)s[2]) == 'a' &&
         tolower((unsigned char)s[3]) == 'n')))
    return (char *)s;
  if (s[4] != '.' || s[5] != '0') return (char *)s;
  *pflags |= CNF_INFNAN|CNF_DOTEXP;
  return (char *)(s + 6);
}

/* errors: stops at unparsed part or broken decimal exponent */
static char *skip_real(const char *s, int radix, int *pflags)
{
  const char *s0 = s, *t = skip_infnan(s, pflags);
  if (t > s) return (char *)t;
  if (*s == '+' || *s == '-') ++s;
  t = skip_ureal(s, radix, pflags);
  return (t > s) ? (char *)t : (char *)s0;
}

/* errors: stops at unparsed part or broken decimal exponent */
static char *skip_complex(const char *s, int radix, int *pflags)
{
  const char *t, *u;
  /* check for ifnans first */
  t = skip_infnan(s, pflags);
  if (t > s && tolower((unsigned char)*t) == 'i') 
    return *pflags |= CNF_IMPART, (char *)(t + 1);
  else if (t > s) goto gotreal;
  /* something else: purely imaginary: '+'/'-' ... 'i' */
  if (*s == '+' || *s == '-') {
    if (tolower((unsigned char)s[1]) == 'i') 
      return *pflags |= CNF_IMPART, (char *)(s + 2);
    t = skip_ureal(s + 1, radix, pflags);
    if (t > s + 1 && tolower((unsigned char)*t) == 'i') 
      return *pflags |= CNF_IMPART, (char*)(t + 1);
    else if (t > s + 1) goto gotreal;
  }
  /* none of the above: must start with real */
  t = skip_real(s, radix, pflags);
  if (t == s) return (char *)s; /* no match */
gotreal:  
  if (tolower((unsigned char)*t) == 'i') 
    return *pflags |= CNF_IMPART, (char*)(t + 1);
  if (*t == '@') {
    u = skip_real(t + 1, radix, pflags);
    if (u > t + 1) return *pflags |= CNF_POLAR, (char *)u;
  }
  /* no polar or failed polar match: can still be cartesian */
  u = skip_infnan(t, pflags);
  if (u > t && tolower((unsigned char)*u) == 'i')
    return *pflags |= CNF_IMPART, (char *)(u + 1);
  /* ok, could be a cartesian still */
  if (*t == '+' || *t == '-') {
    if (tolower((unsigned char)t[1]) == 'i') 
      return *pflags |= CNF_IMPART, (char *)(t + 2);
    u = skip_ureal(t + 1, radix, pflags);
    if (u > t + 1 && tolower((unsigned char)*u) == 'i') 
      return *pflags |= CNF_IMPART, (char *)(u + 1);
  }
  /* no (valid) imaginary part: stop after real */
  return (char *)t;
}

/* returns start of complex part and sets radix/ie; returns NULL on errors */
static char *check_number(const char *s, int *pradix, int *pie, int *pflags)
{
  const char *s0 = s, *t;  
  int radix = 0, forceie = 0; /* -1=#i 1=#e */
  /* no empty inputs! */
  if (*s == 0) return NULL;
  /* parse prefix */
  while (s[0] == '#') {
    switch (s[1]) {
      case 'e': case 'E': forceie = 1;  s += 2; continue;
      case 'i': case 'I': forceie = -1; s += 2; continue;
      case 'b': case 'B': radix = 2;    s += 2; continue;
      case 'o': case 'O': radix = 8;    s += 2; continue;
      case 'd': case 'D': radix = 10;   s += 2; continue;
      case 'x': case 'X': radix = 16;   s += 2; continue;
    }
    break;
  }
  /* there should be at most 2 prefixes of different kind */
  if (s > s0 + 4) return NULL;
  if (s == s0 + 4 && !(radix * forceie)) return NULL;
  /* ok, now the number itself ... */
  if (!radix && pradix) radix = *pradix;
  t = skip_complex(s, radix ? radix : 10, pflags);
  /* no unparsed parts! */
  if (t == s || *t != 0) return NULL;
  /* valid syntax, can proceed */
  if (pradix && radix) *pradix = radix;
  if (pie && forceie) *pie = forceie;
  return (char *)s; 
}

/* returns 1 and sets *ep, or 0 and does not change *ep */
static int parse_zero(const char *s, char **ep)
{
  const char *s0 = s; int res = 0;
  if (*s == '+' || *s == '-') ++s;
  while (*s == '0') { ++s; res = 1; }
  if (!res || *s != 0) return 0;
  *ep = (char *)s; return 1;
}


/* bignum implementation (avp) */

/* bignum version 0.6.0rc4 $Id: bnversion.c 382 2009-08-15 16:57:45Z avp $ */

/* esl** invariants and naming conventions
* internal procedures not returning bignums directly are named bnx_foo
* internal procedures always returning fresh new bignums are named bny_foo
* internal procedures returning new bignums or bn0 are named bnz_foo
* public interface procedures named bnfoo return bn0, bn1, or shared bignums
* all returned bignums are owned by the caller and should be bnfree()d */

/* pick 32-bit unsigned type for bignum 'limbs' */
#define LIMB_BITS 32
#define LIMB_MAX UINT32_MAX
typedef uint32_t limb_t;
typedef int32_t slimb_t;

/* pick 64-bit unsigned type for 2-limb math */
#define LIMBLIMB_BITS 64
#define LIMBLIMB_MAX UINT64_MAX
typedef uint64_t limblimb_t;

/* allocator:
*  if ptr == NULL then get fresh memory (size > 0 in this case)
*  if size == 0 then free ptr
*  otherwise reajust memory (possibly copying it) */
static void *bnrealloc(void *ptr, size_t size);

/* special values for dupcount */
enum { 
  DUP_FRESH = 0,
  DUP_STATIC = -1,
  DUP_AUTO = -2
};

/* The bignum type (typedef'd to bignum_t) */
struct bignum {
  size_t  size;
  int     dupcount;
  int     isneg;
  limb_t  limb[1]; /* alloc as many limbs as needed */
};

/* operations on limbs */
#define HILO(h,l) ((((limblimb_t)(h)) << LIMB_BITS)|(l))
#define HI_LIMB(w) ((limb_t)((w) >> LIMB_BITS))
#define LO_LIMB(w) ((limb_t)(w))
#define ZERO_LIMBS(d,c) memset((d), 0, (c) * sizeof(limb_t))
#define COPY_LIMBS(d,s,c) memcpy((d), (s), (c) * sizeof(limb_t))

#ifdef NDEBUG
#define NEWBN(loc, s, where)                                            \
  do { if ((s) == 0) (loc) = bn0; else {				\
      if ((s)-1 > (SIZE_MAX-sizeof(bignum_t)) / sizeof(limb_t))		\
          bnx_failure(where);                                           \
      (loc)=bnrealloc(NULL, sizeof(bignum_t)+((s)-1)*sizeof(limb_t));   \
      (loc)->dupcount = 0; (loc)->size = s; } } while(0)
#define return_NORMALIZE(n, where)                                      \
  do { if ((n)->size == 0) { bnfree(n); return bn0; }                   \
      if (BNPLUSONE(n)) { bnfree(n); return bn1; }                      \
      return (n); } while (0)
#define return_NEW(n) return (n)
#define CHECKNEW(b) 
#define CHECKSIGN(b)
#else /* debug */
#define NEWBN(loc, s, where)                                            \
  do { if ((s) == 0) (loc) = bn0; else {				\
      if ((s)-1 > (SIZE_MAX-sizeof(bignum_t)) / sizeof(limb_t))		\
          bnx_failure(where);                                           \
      (loc)=bnrealloc(NULL, sizeof(bignum_t) + ((s)-1)*sizeof(limb_t)); \
      (loc)->dupcount = 0; (loc)->size = s; (loc)->isneg = 2; } } while(0)
#define return_NORMALIZE(n, where)                                      \
  do { if ((n)->size == 0) { bnfree(n); return bn0; }                   \
      if (BNPLUSONE(n)) {bnfree(n); return bn1; }                       \
      CHECKSIGN(n);                                                     \
      return (n); } while (0)
#define return_NEW(n)                                                   \
  do { bignum_t *__b = n; CHECKNEW(__b); return __b; } while (0)
#define CHECKNEW(b) assert((b)->dupcount == 0)
#define CHECKSIGN(b) assert(((b)->isneg == 1) || ((b)->isneg == 0))
#endif /* defined(NDEBUG) */

#define RESIZE(b, s) do {                                               \
  if (b->size < s)                                                      \
      r = bnrealloc(r, sizeof(bignum_t)                                 \
                  + (r->size - 1) * sizeof(limb_t)); } while (0)

#define BNZERO(n) ((n)->size == 0)
#define BNONE(n, neg) (((n)->size==1) && ((n)->limb[0]==1) && ((n)->isneg==(neg)))
#define BNPLUSONE(n) (((n) == bn1) || BNONE(n,0))
#define BNMINUSONE(n) BNONE(n,1)
#define BNTWO(n, neg) (((n)->size==1) && ((n)->limb[0]==2) && ((n)->isneg==(neg)))
#define BNPLUSTWO(n) BNTWO(n,0)
#define BNMINUSTWO(n) BNONE(n,1)

#define INT64_LIMBS (1+(sizeof(int64_t)+sizeof(slimb_t)-1)/sizeof(limb_t))

typedef struct bignum_ll { /* DO NOT ALLOC */
  size_t   size;
  int      dupcount;
  int      isneg;
  limb_t limb[INT64_LIMBS];
} bignumll_t;

void bnfree(bignum_t *pn);

static bignum_t *bny_dupll(const bignum_t *lla);
static bignum_t *bndup(const bignum_t *a);
bignum_t *lltobn(int64_t n);
bignum_t *ulltobn(uint64_t n);
bignumll_t bnll(int64_t v);

static bignum_t zero = {0, DUP_STATIC, 0, {0}};
bignum_t *bn0 = &zero;

static bignum_t one =  {1, DUP_STATIC, 0, {1}};
bignum_t *bn1 = &one;

static void bnx_failure(char *msg)
{
  fprintf(stderr, "; bnx_failure(%s)\n", msg);
  cxm_check(0, msg);
}

static bignum_t *bnx_zdiv(void)
{
  fprintf(stderr, "; division by zero\n");
  cxm_check(0, "division by zero (bignum)");
  return bn0;
}

static bignum_t *bnx_makell(struct bignum_ll *b, int64_t v)
{
  size_t i;

  b->dupcount = DUP_AUTO;
  if (v >= 0) {
#if (UINT64_MAX > LIMB_MAX)
    uint64_t w;
    for (i = 0, w = v; w; i++, w >>= LIMB_BITS) {
      b->limb[i] = w & LIMB_MAX;
    }
#elif (UINT64_MAX == LIMB_MAX)
    i = 1;
    b->limb[0] = v;
#else
#error "Too bad: uint64_t is shorter than reasonable"
#endif
    b->size = i;
    b->isneg = 0;
  } else {
#if (UINT64_MAX > LIMB_MAX)
    uint64_t w;
    if (v == INT64_MIN) {
      b->limb[0] = 0;
      i = 1;
      w = (((uint64_t)INT64_MIN) >> LIMB_BITS); /* esl*, was + 1 */
    } else {
      w = -v;
      i = 0;
    }
    for (;w; i++, w >>= LIMB_BITS) {
      b->limb[i] = w & LIMB_MAX;
    }
#elif (UINT64_MAX == LIMB_MAX)
    if (v == INT64_MIN) {
      i = 2;
      b->limb[0] = 0;
      b->limb[1] = 1;
    } else {
      i = 1;
      b->limb[0] = -v;
    }
#else
#error "Too bad: uint64_t is shorter than reasonable"
#endif
    b->size = i;
    b->isneg = 1;
  }

  return (bignum_t *)(void *)b;
}

/* esl++ */
static bignum_t *bnx_makeull(struct bignum_ll *b, uint64_t v)
{
  size_t i;

  b->dupcount = DUP_AUTO;
#if (UINT64_MAX > LIMB_MAX)
  { uint64_t w;
    for (i = 0, w = v; w; i++, w >>= LIMB_BITS) {
      b->limb[i] = w & LIMB_MAX;
    }
  }
#elif (UINT64_MAX == LIMB_MAX)
  i = 1;
  b->limb[0] = v;
#else
#error "Too bad: uint64_t is shorter than reasonable"
#endif
  b->size = i;
  b->isneg = 0;

  return (bignum_t *)(void *)b;
}


/* if (a > b) return +1
 * if (a < b) return -1
 * if (a == b) return 0 */
static int bnx_cmp(const limb_t *a, size_t as, const limb_t *b, size_t bs)
{
  size_t i;

  if (as > bs)
    return +1;
  if (as < bs)
    return -1;

  for (i = as; i--;) {
    if (a[i] > b[i])
      return +1;
    if (a[i] < b[i])
      return -1;
  }
  return 0;
}

/* isneg not set, a != bn0 */
static bignum_t *bny_addl(const bignum_t *a, limb_t w) 
{
  bignum_t *r;
  size_t i;
  limb_t c;
  size_t rs = a->size + 1;

  if (rs == 0)
    bnx_failure("bnx_addl");

  NEWBN(r, rs, "bnx_addl");

  for (c = w, i = 0; i < a->size; i++) {
    limb_t x = a->limb[i];
    limb_t s = x + c;
    r->limb[i] = s;
    c = (x & ~s) >> (LIMB_BITS - 1); /* lesser magic */
  }
  r->limb[i] = c;
  r->size = i + c;

  return_NEW(r);
}


static size_t bnx_add(limb_t *r, size_t rs,
  const limb_t *a, size_t as, 
  const limb_t *b, size_t bs)
{
  size_t i;
  limb_t c;

  if (as < bs) {
    const limb_t *t = a;
    size_t ts = as;
    a = b;
    as = bs;
    b = t;
    bs = ts;
  }
  assert(rs >= (as + 1));

  for (i = 0, c = 0; i < bs; i++) {
    limb_t x = a[i];
    limb_t y = b[i];
    limb_t s = x + y + c;
    r[i] = s;
    c = ((x & y) | ((x | y) & ~s)) >> (LIMB_BITS - 1); /* magic */
  }
  for (; i < as; i++) {
    limb_t x = a[i];
    limb_t s = x + c;
    r[i] = s;
    c = (x & ~s) >> (LIMB_BITS - 1); /* lesser magic */
  }
  r[i] = c;

  return i + c;
}

/* only if a > w, a!=0, isneg not set */
static bignum_t *bny_subl(const bignum_t *a, limb_t w)
{
  bignum_t *r;
  size_t i;
  limb_t c;

  NEWBN(r, a->size, "bnx_subl");

  for (c = w, i = 0; i < a->size; i++) {
    limb_t x = a->limb[i];
    limb_t s = x - c;
    r->limb[i] = s;
    c = (~x & s) >> (LIMB_BITS - 1); /* magic */
  }
  for (i = a->size; i--;) {
    if (r->limb[i])
      break;
  }

  r->size = i + 1;

  return_NEW(r);
}

static size_t bnx_sub(limb_t *r, size_t rs,
  const limb_t *a, size_t as, 
  const limb_t *b, size_t bs) /* ignore isnegs, a > b */
{
  size_t i;
  limb_t c;

  assert(rs >= as);

  for (i = 0, c = 0; i < bs; i++) {
    limb_t x = a[i];
    limb_t y = b[i];
    limb_t s = x - y - c;
    r[i] = s;
    c = ((~x & y) | ((~x | y) & s)) >> (LIMB_BITS - 1); /* magic */
  }
  for (; i < as; i++) {
    limb_t x = a[i];
    limb_t s = x - c;
    r[i] = s;
    c = (~x & s) >> (LIMB_BITS - 1); /* magic */
  }
  for (i = as; i--;) {
    if (r[i])
      break;
  }

  return i + 1;
}

static bignum_t *bnz_addsub(int a_isneg, const bignum_t *a, int b_isneg, const bignum_t *b)
{
  bignum_t *r;

  if (a_isneg == b_isneg) {
    size_t as = a->size;
    size_t bs = b->size;
    size_t rs = ((as > bs)? as: bs) + 1;

    if (rs == 0)
      bnx_failure("bnx_addsub");
    NEWBN(r, rs, "bnx_addsub");
    r->size = bnx_add(r->limb, rs, a->limb, as, b->limb, bs);
    r->isneg = a_isneg;
  } else {
    size_t as = a->size;
    size_t bs = b->size;
    size_t rs = ((as > bs)? as: bs);

    NEWBN(r, rs, "bnx_addsub");

    switch (bnx_cmp(a->limb, a->size, b->limb, b->size)) {
    case +1:
      r->size = bnx_sub(r->limb, rs, a->limb, as, b->limb, bs);
      r->isneg = a_isneg;
      break;
    case 0:
      bnfree(r);
      return bn0;
    case -1:
      r->size = bnx_sub(r->limb, rs, b->limb, bs, a->limb, as);
      r->isneg = b_isneg;
      break;
    default:
      r = NULL;
      assert(r != NULL);
      break;
    }
    RESIZE(r, rs);
  }

  return_NEW(r);
}

static int bnx_bitsl(limb_t x)
{
  limb_t y;
  int n, c;

  n = 0;
  c = 16;
  do {
    y = x >> c;
    if (y != 0) { n += c; x = y; }
    c >>= 1;
  } while (c != 0);
  return n + x;
}

static size_t bnx_shleft(limb_t *r, size_t rs,
  const limb_t *a, size_t aw, int ap,
  size_t sw, int sp)
{
  if (sw > 0)
    ZERO_LIMBS(r, sw);

  if (sp == 0) {
    COPY_LIMBS(r + sw, a, aw + 1);
    return sw + aw + 1;
  } else {
    limb_t lo;
    limb_t hi;
    size_t i;

    for (i = 0, lo = 0; i < (aw + 1); i++, lo = hi) {
      hi = a[i];
      r[i + sw] = HI_LIMB(HILO(hi,lo) << sp);
    }
    lo = HI_LIMB(HILO(0, lo) << sp);
    if (lo) {
      r[aw + sw + 1] = lo;
      return aw + sw + 2;
    } else {
      return aw + sw + 1;
    }
  }
}

static size_t bnx_shright(limb_t *r, size_t rs,
  const limb_t *a, size_t aw, int ap,
  size_t sw, int sp)
{
  if (sp == 0) {
    assert(rs >= (aw + 1 - sw));

    COPY_LIMBS(r, a + sw, aw + 1 - sw);
    return aw + 1 - sw;
  } else if (sp < ap) {
    size_t i;
    limb_t lo;
    limb_t hi;

    assert(rs >= aw + 1 - sw);

    for (i = 0, lo = a[sw]; i < (aw - sw); i++, lo = hi) {
      hi = a[sw + i + 1];
      r[i] = LO_LIMB(HILO(hi, lo) >> sp);
    }
    r[aw - sw] = lo >> sp;
    return aw + (r[aw - sw]? 1: 0) - sw;
  } else {
    size_t i;
    limb_t lo;
    limb_t hi;

    assert(rs >= aw - sw);
    for (i = 0, lo = a[sw]; i < (aw - sw); i++, lo = hi) {
      hi = a[sw + i + 1];
      r[i] = LO_LIMB(HILO(hi, lo) >> sp);
    }
    return aw - sw;
  }
}

static bignum_t *bny_maddl(const bignum_t *n, limb_t w, limb_t c)
{
  bignum_t *r;
  size_t i;
  size_t rs = n->size + 1;

  if (rs == 0)
    bnx_failure("bny_maddl");

  NEWBN(r, rs, "bny_maddl");

  for (i = 0; i < n->size; i++) {
    limb_t x = n->limb[i];
    limblimb_t z = x * (limblimb_t)w + c;
    limb_t lo = LO_LIMB(z);
    limb_t hi = HI_LIMB(z);
    r->limb[i] = lo;
    c = hi;
  }
  r->limb[i] = c;
  r->size = i + ((c==0)? 0: 1);
  return_NEW(r);
}

/* IMPROV */
size_t bnx_mul(limb_t *r, size_t rs,
  const limb_t *a, size_t as, 
  const limb_t *b, size_t bs) /* ignores isneg */
{
  size_t i;
  size_t j;
  size_t k;

  assert(a != NULL);
  assert(b != NULL);
  assert(r != NULL);
  assert(as > 0);
  assert(bs > 0);
  assert(rs >= as + bs);

  ZERO_LIMBS(r, as + bs);

  for (k = i = 0; i < as; i++) {
    limb_t c;
    limb_t ai = a[i];

    for (k = i, c = 0, j = 0; j < bs; j++, k++) {
      limblimb_t w =  ai * (limblimb_t)b[j] + c;
      w += r[k];
      r[k] = LO_LIMB(w);
      c = HI_LIMB(w);
    }
    r[k] = c;
  }

  return k + (r[k]? 1: 0);
}

bignum_t *bnz_dmodl(limb_t *r, const bignum_t *n, limb_t d)
{
  bignum_t *x;
  size_t i;
  limblimb_t h;

  if (BNZERO(n)) {
    *r = 0;
    return bn0;
  }
  NEWBN(x, n->size, "bnz_dmodl");

  for (i = n->size, h = 0; i--;) {
    limblimb_t hn = HILO(h, n->limb[i]);
    x->limb[i] = LO_LIMB(hn / d);
    h = (hn % d);
  }
  *r = (limb_t)(int64_t)h; 

  if (x->limb[n->size - 1] != 0) {
    x->size = n->size;
  } else {
    x->size = n->size - 1;
  }

  return_NEW(x);
}

#define LIMBS_DEC(lb, size) do { size_t i; limb_t c;    \
  for (i = 0; i < size; i++) if (lb[i]) break; \
  for (c = 0;i < size; i++, c = LIMB_MAX) lb[i] = c - lb[i]; } while (0)

#define DEFINE_PTR(p)   size_t p##start; size_t p##size
/* size bits part is always positive and not greater than LIMB_BITS */
#define DEFINE_SIZE(p)  size_t p##size; int p##bits
#define SET_SIZE(n,w,p) ((n##bits=(p)), (n##size=(w)))
#define COPY_SIZE(a,b)  ((a##bits=b##bits), (a##size=b##size))

#define SIZE_ARG(n)     (n##size),(n##bits)
#define SIZE_ADD(r,a,b) \
  ((r##size=a##size+b##size+(((a##bits + b##bits) <= LIMB_BITS)? 0: 1)), \
   (r##bits=a##bits+b##bits), (r##bits -= (r##bits>LIMB_BITS)? LIMB_BITS: 0))
#define SIZE_ADD_LIMBS(r,a,l)  ((r##size=a##size+(l)), (r##bits=a##bits))
#define SIZE_SUB(r,a,b) \
  ((r##size=a##size-b##size-((a##bits >= b##bits)? 0: 1)), \
   (r##bits=a##bits-b##bits+((a##bits >= b##bits)? 0: LIMB_BITS)))
#define SIZE_SUB_LIMBS(r,a,l)  ((r##size=a##size-(l)), (r##bits=a##bits))
#define SIZE_LESS(a,b)  \
  ((a##size < b##size) ||                     \
   ((a##size == b##size) && (a##bits < b##bits)))
#define SIZE_EQUAL(a,b)  ((a##size == b##size) && (a##bits == b##bits))
#define FLIP_BLOCKS do { \
  t_block = a_block; a_block = b_block; b_block = t_block;     \
  t_size = a_size; a_size = b_size; b_size = t_size; } while (0)
#define FLIP_SIZES do { t = a; a = b; b = t; flipped = !flipped; } while (0)
#define CHECK_SIZE(condition) do { ok = (condition)? 0: ok; } while (0)

static limb_t *div_sizes(size_t *a_size, size_t *b_size,
  size_t num_size, int num_bits,
  size_t den_size, int den_bits,
  size_t div_size, int div_bits)
{
  int ok = 1;
  limb_t *block;
  size_t a;
  size_t b;
  size_t t;
  size_t c;
  DEFINE_SIZE(sh);
  DEFINE_SIZE(bit);
  DEFINE_SIZE(limb);
  DEFINE_SIZE(n);
  DEFINE_SIZE(d);
  DEFINE_SIZE(e);
  DEFINE_SIZE(f0);
  size_t di;
  size_t db;
  size_t fb;
  int shift = LIMB_BITS - den_bits;
  int flipped = 0;

  SET_SIZE(bit, 0, 1);
  SET_SIZE(limb, 0, LIMB_BITS);
  SET_SIZE(sh, 0, shift);
  /* shift */
  SIZE_ADD(d, den_, sh);
  SIZE_ADD(n, num_, sh); CHECK_SIZE(nsize < num_size);

  a = dsize;
  a += 1;        CHECK_SIZE(a < 1);
  a += nsize;    CHECK_SIZE(a < nsize);
  a += 1;        CHECK_SIZE(a < 1);

  /* multiply by f0 */
  nsize += 2; CHECK_SIZE(nsize < 2); nsize -= 2;
  SET_SIZE(f0,1,1);
  SIZE_ADD(n,n,f0);

  dsize += 2; CHECK_SIZE(dsize < 2); dsize -= 2;
  SIZE_ADD(d,d,f0); /* f0 * d */

  b = nsize + 1; CHECK_SIZE(b < 1);
  b += dsize;    CHECK_SIZE(b < dsize);
  b += 1;        CHECK_SIZE(b < 1);

  SET_SIZE(e, 0, LIMB_BITS - 2);
  SIZE_SUB(d,d,e);

  di = db = den_size + 2; CHECK_SIZE(db < 2);
  fb = 2;

  if (SIZE_LESS(e, div_)) {
    int i;
    size_t fd;
    size_t dj;
    size_t nj;
    size_t n;
    DEFINE_SIZE(e2);
    DEFINE_SIZE(dj);
    DEFINE_SIZE(djx);
    DEFINE_SIZE(f1);

    for (i = 0;; i++) {
      SIZE_ADD(e2, e, e);
      if (SIZE_LESS(div_, e2))
        break;

      fd = db + fb; CHECK_SIZE(fd < db);
      SET_SIZE(dj, fd - 1, LIMB_BITS);
      SIZE_SUB(djx, dj, e2);
      dj = djxsize + 1; CHECK_SIZE(dj < djxsize);

      SET_SIZE(f1, fb, 1);
      SIZE_ADD(n, n, f1);  CHECK_SIZE(nsize <= f1size);
      nj = nsize + 2; CHECK_SIZE(nj < 2);

      c = dj + nj; CHECK_SIZE(c < dj);
      if (a < c) a = c;
      db += fb; CHECK_SIZE(db < fb);
      fb += fb;
      di = dj;

      FLIP_SIZES;
      COPY_SIZE(e, e2);
    }

    /* last iteration is short */
    SIZE_ADD(div_, div_, bit);
    SIZE_SUB(div_, div_, e);
    n = 1 + div_size;
    fb = fb / 2;

    fb += n; CHECK_SIZE(fb < n);
    SET_SIZE(f1, fb, 1);
    SIZE_ADD(n,n,f1); CHECK_SIZE(nsize <= f1size);
    c = nsize + 2; CHECK_SIZE(c < nsize);
    if (a < c) a = c;
    FLIP_SIZES;
  } else {
    /* short division does not grow blocks */
  }

  /* polishing does not grow blocks */
  t = a + b;
  CHECK_SIZE(t < a);
  if (!ok || (t > SIZE_MAX / sizeof(limb_t)))
    bnx_failure("bnx_dmod");
  block = bnrealloc(NULL, t * sizeof(limb_t));
  if (flipped) {
    *a_size = b;
    *b_size = a;
  } else {
    *a_size = a;
    *b_size = b;
  }
  return block;
}

static bignum_t *bnz_dmod(bignum_t **rem, const bignum_t *num, const bignum_t *den)
{
  DEFINE_SIZE(n);
  DEFINE_SIZE(d);

  assert(num != NULL);
  assert(den != NULL);
  assert(rem != NULL);
  assert(!BNZERO(den));
  assert(den->size > 1);

  SET_SIZE(n, num->size - 1, bnx_bitsl(num->limb[num->size - 1]));
  SET_SIZE(d, den->size - 1, bnx_bitsl(den->limb[den->size - 1]));

  if (BNZERO(den)) {
    bnx_zdiv();
    return bn0;
  } else if (BNZERO(num)) {
    *rem = bn0;
    return bn0;
  } else if (SIZE_LESS(n, d)) {
    goto zero_quotient;
  } else if (SIZE_EQUAL(n, d)) {
    struct bignum_ll b;
    switch (bnx_cmp(num->limb, num->size, den->limb, den->size)) {
    case 0: /* num == den */
      *rem = bn0;
      return_NEW(bny_dupll(bnx_makell(&b, 1)));
    case +1: /* num > den */
      NEWBN(*rem, num->size, "bnx_dmod");
      (*rem)->size = bnx_sub((*rem)->limb, num->size,
        num->limb, num->size,
        den->limb, den->size);
      return_NEW(bny_dupll(bnx_makell(&b, 1)));
    default: /* num < den */
    zero_quotient:
      NEWBN(*rem, num->size, "bnx_dmod");
      COPY_LIMBS((*rem)->limb, num->limb, num->size);
      (*rem)->size = num->size;
      return bn0;
    }
  } else {
    bignum_t *div;
    limb_t *block;
    limb_t *a_block;
    size_t a_size;
    limb_t *b_block;
    size_t b_size;
    limb_t sd_hi;
    limb_t p;
    size_t db;
    size_t fb;
    DEFINE_SIZE(e);
    DEFINE_SIZE(r);
    DEFINE_SIZE(bit);
    DEFINE_PTR(sn);
    DEFINE_PTR(sd);

    SET_SIZE(bit, 0, 1);
    SIZE_SUB(r, n, d);
    SIZE_ADD(r, r, bit);

    block = div_sizes(&a_size, &b_size,
      SIZE_ARG(n),
      SIZE_ARG(d),
      SIZE_ARG(r));
    a_block = block;
    b_block = a_block + a_size;

    /* Shift num and den so that den[hi] >= 1/2 */
    sdstart = 0;
    sdsize = bnx_shleft(a_block + sdstart, a_size - sdstart,
      den->limb, dsize, dbits,
      0, LIMB_BITS - dbits);
    snstart = sdsize;
    snsize = bnx_shleft(a_block + snstart, a_size - snstart,
      num->limb, nsize, nbits,
      0, LIMB_BITS - dbits);
    sd_hi = a_block[sdstart + sdsize - 1];

    /* Multiply sd and sn by f0 */
    if (sd_hi == LIMB_MAX) {

      ZERO_LIMBS(b_block + 0, 1);
      COPY_LIMBS(b_block + 1, a_block + sdstart, sdsize);
      sdstart = 0;
      LIMBS_DEC(b_block, sdsize + 1);

      ZERO_LIMBS(b_block + sdsize, 1);
      COPY_LIMBS(b_block + sdsize + 1, a_block + snstart, snsize);
      snstart = sdsize;
      snsize += 1;
    } else {
      limb_t f0 = LO_LIMB(HILO(-(slimb_t)(sd_hi+1), 0) / (sd_hi+1));
      size_t ps;

      ps = bnx_mul(b_block, b_size,
        a_block, sdsize, &f0, 1);
      sdsize = 1 + bnx_add(b_block + 1, b_size - 1,
        b_block + 1, ps - 1, a_block, sdsize);
      LIMBS_DEC(b_block, sdsize);
      sdsize -= (b_block[sdsize -1]? 0: 1);

      ps = bnx_mul(b_block + sdsize, b_size - sdsize,
        a_block + snstart, snsize, &f0, 1);
      snsize = 1 + bnx_add(b_block + sdsize + 1, b_size - sdsize - 1,
        b_block + sdsize + 1, ps - 1,
        a_block + snstart, snsize);
      snstart = sdsize;
    }

    /* set all variables for interations*/
    SET_SIZE(e, 0, LIMB_BITS-2);
    db = den->size + 1;
    fb = 2;
    /* if not enough results produces do ... */
    if (SIZE_LESS(e, r)) {
      int i;
      size_t n;
      limb_t *t_block;
      size_t t_size;
      size_t njsize;
      size_t djsize;
      DEFINE_SIZE(e2);

      /* nx = (b_block, snstart, snsize)
      * dx = (b_block, 0, sdsize) */
      for (i = 0;; i++, db += fb, fb += fb) {
        SIZE_ADD(e2, e, e);
        if (SIZE_LESS(r, e2))
          break;

        /* advance dx and nx into a_block */
        djsize = bnx_mul(a_block, a_size,
          b_block, sdsize,
          b_block + db-fb, sdsize - (db-fb));
        djsize = fb + bnx_add(a_block + fb, a_size - fb,
          a_block + fb, djsize - fb,
          b_block, db-fb);

        njsize = bnx_mul(a_block + djsize, a_size - djsize,
          b_block + db-fb, sdsize - (db-fb),
          b_block + snstart, snsize);
        njsize = fb + bnx_add(a_block + djsize+fb, a_size - djsize-fb,
          a_block + djsize+fb, njsize - fb,
          b_block + snstart, snsize);

        sdsize = djsize;
        snstart = djsize;
        snsize = njsize;
        FLIP_BLOCKS;

        SET_SIZE(e, e2size, e2bits);
      }
      /* last iteration is short */
      SIZE_ADD(r, r, bit);
      SIZE_SUB(r, r, e);
      n = 1 + rsize;
      fb = fb / 2;

      njsize = bnx_mul(a_block, a_size,
        b_block + (db - fb - n), sdsize - (db - fb - n),
        b_block + snstart, snsize);

      njsize = fb + n + bnx_add(a_block + fb + n, a_size - (fb + n),
        a_block + fb + n, njsize - (fb + n),
        b_block + snstart, snsize);
      snstart = db + fb + n;
      snsize = njsize - snstart;
      FLIP_BLOCKS;
    } else {
      /* short answer (within -2 of the correct one) */
      snsize -= db;
      snstart += db;
    }
    sdsize = bnx_mul(a_block, a_size,
      b_block + snstart, snsize,
      den->limb, den->size);
    sdsize = bnx_sub(a_block, a_size,
      num->limb, num->size,
      a_block, sdsize);
    for (p = 0; bnx_cmp(a_block, sdsize, den->limb, den->size) >= 0; p++) {
      sdsize = bnx_sub(a_block, sdsize,
        a_block, sdsize,
        den->limb, den->size);
      assert(p <= 2);
    }
    snsize = bnx_add(b_block + snstart, b_size - snstart,
      b_block + snstart, snsize,
      &p, 1);

    if (sdsize) {
      NEWBN(*rem, sdsize, "bnx_dmod");
      COPY_LIMBS((*rem)->limb, a_block, sdsize);
    } else {
      *rem = bn0;
    }
    NEWBN(div, snsize, "bnx_dmod");
    COPY_LIMBS(div->limb, b_block + snstart, snsize);

    free(block);
    return_NEW(div);
  }
}

/* use for ll autos only; do not normalize! */
static bignum_t *bny_dupll(const bignum_t *n)
{
  bignum_t *nn;
  assert(n); 
  assert(n->dupcount == DUP_AUTO);
  NEWBN(nn, n->size, "bndup");
  COPY_LIMBS(nn->limb, n->limb, n->size);
  nn->isneg = n->isneg;
  return nn;
}


static bignum_t *bndup(const bignum_t *n)
{
  assert(n != NULL);
  CHECKSIGN(n);

  switch (n->dupcount) {
  case DUP_AUTO: {
    bignum_t *nn;
    NEWBN(nn, n->size, "bndup");
    COPY_LIMBS(nn->limb, n->limb, n->size);
    nn->isneg = n->isneg;
    return_NORMALIZE(nn, "bndup");
  }
  case DUP_STATIC:
    return (bignum_t *)n;
  default: {
    bignum_t *nn = (bignum_t *)n;
    nn->dupcount++;
    return nn;
  }
  }
}

void *bnrealloc(void *ptr, size_t size)
{
  /* NB: realloc is no longer defined for zero size! */
  if (!size) return free(ptr), NULL;
  ptr = cxm_cknull(realloc(ptr, size), "realloc(bignum)");
  return ptr;
}

void bnfree(bignum_t *n)
{
  if (n != NULL) {
    /* CHECKSIGN(n); */
    if (n->dupcount >= 0) {
      if (n->dupcount-- == 0)
        bnrealloc(n, 0);
    }
  }
}

bignum_t *bnneg(const bignum_t *a)
{
  bignum_t *n;

  assert(a != NULL);
  CHECKSIGN(a);

  if (BNZERO(a))
    return bn0;

  NEWBN(n, a->size, "bnneg");
  COPY_LIMBS(n->limb, a->limb, a->size);
  n->isneg = !a->isneg;
  return_NORMALIZE(n, "bnneg");
}


bignum_t *bnabs(const bignum_t *n)
{
  if (n->isneg)
    return bnneg(n);
  else
    return bndup(n);
}

int bnsign(const bignum_t *a)
{
  assert(a != 0);
  CHECKSIGN(a);

  if (a->isneg)
    return -1;
  if (BNZERO(a))
    return 0;
  else
    return +1;
}

int bnzero(const bignum_t *a)
{
  assert(a != NULL);
  return BNZERO(a) ? 1 : 0;
}

int bncmp(const bignum_t *a, const bignum_t *b)
{
  assert(a != NULL);
  CHECKSIGN(a);
  assert(b != NULL);
  CHECKSIGN(b);

  if (BNZERO(b))
    return bnsign(a);
  if (BNZERO(a))
    return -bnsign(b);

  if (a->isneg) {
    if (b->isneg) {
      return bnx_cmp(b->limb, b->size, a->limb, a->size);
    } else {
      return -1;
    }
  } else {
    if (b->isneg) {
      return +1;
    } else {
      return bnx_cmp(a->limb, a->size, b->limb, b->size);
    }
  }
}

/* compare absolute values */
int bncmpabs(const bignum_t *a, const bignum_t *b)
{
  assert(a != NULL);
  CHECKSIGN(a);
  assert(b != NULL);
  CHECKSIGN(b);

  if (BNZERO(b))
    return BNZERO(a) ? 0 : +1;
  if (BNZERO(a))
    return -1;

  return bnx_cmp(a->limb, a->size, b->limb, b->size);
}

int bncmpl(const bignum_t *n, int64_t fix)
{
  struct bignum_ll b;

  if (fix == 0)
    return bnsign(n);

  return bncmp(n, bnx_makell(&b, fix));
}

int bneq(const bignum_t *a, const bignum_t *b)
{
  return (bncmp(a, b) == 0);
}

#define bneql(b, l) (0 == bncmpl(b, l))

int bneven(const bignum_t *a)
{
  assert(a != NULL);
  CHECKSIGN(a);

  if (BNZERO(a))
    return 1;

  return (a->limb[0] & 1) == 0;
}

int bnodd(const bignum_t *a)
{
  assert(a != NULL);
  CHECKSIGN(a);

  if (BNZERO(a))
    return 0;

  return (a->limb[0] & 1) != 0;
}

bignum_t *bnadd(const bignum_t *a, const bignum_t *b)
{
  bignum_t *r;

  assert(a != NULL);
  CHECKSIGN(a);
  assert(b != NULL);
  CHECKSIGN(b);

  if (BNZERO(a))
    return bndup(b);
  if (BNZERO(b))
    return bndup(a);
  r = bnz_addsub(a->isneg, a, b->isneg, b);
  return_NORMALIZE(r, "bnadd");
}

bignum_t *bnaddll(const bignum_t *n, int64_t incr)
{
  struct bignum_ll b;
  bignum_t *r = bn0;

  assert(n != NULL);
  CHECKSIGN(n);

  if (BNZERO(n))
    return lltobn(incr);

  if (incr == 0) {
    return bndup(n);
  } else if (incr > 0) {
    if (incr <= LIMB_MAX) {
      if (n->isneg) {
        if ((n->size > 1) || (n->limb[0] > (limb_t)incr)) {
          r = bny_subl(n, (limb_t)incr);
          r->isneg = 1;
        } else if (n->limb[0] == (limb_t)incr)
          return bn0;
        else
          return bndup(bnx_makell(&b, incr - n->limb[0]));
      } else {
        r = bny_addl(n, (limb_t)incr);
        r->isneg = 0;
      }
    } else {
      r = bnz_addsub(n->isneg, n, 0, bnx_makell(&b, incr));
    }
  } else if (incr == LONG_MIN) {
    r = bnz_addsub(n->isneg, n, 1, bnx_makell(&b, incr));
  } else {
    incr = -incr;
    if (incr <= LIMB_MAX) {
      if (n->isneg) {
        r = bny_addl(n, (limb_t)incr);
        r->isneg = 1;
      } else {
        if ((n->size > 1) || (n->limb[0] > (limb_t)incr)) {
          r = bny_subl(n, (limb_t)incr);
          r->isneg = 0;
        } else if (n->limb[0] == (limb_t)incr)
          return bn0;
        else
          return bndup(bnx_makell(&b, n->limb[0] - incr));
      }
    } else {
      r = bnz_addsub(n->isneg, n, 1, bnx_makell(&b, incr));
    }
  }
  return_NORMALIZE(r, "bnaddll");
}

bignum_t *bnsub(const bignum_t *a, const bignum_t *b)
{
  bignum_t *r;
  assert(a != NULL);
  assert(b != NULL);
  CHECKSIGN(a);
  CHECKSIGN(b);

  if (BNZERO(a))
    return bnneg(b);
  if (BNZERO(b))
    return bndup(a);
  r = bnz_addsub(a->isneg, a, !b->isneg, b);
  return_NORMALIZE(r, "bnsub");
}

bignum_t *bnashll(const bignum_t *a, int64_t sh)
{
  bignum_t *r = bn0;
  assert(a != NULL);
  CHECKSIGN(a);

  if (BNZERO(a))
    return bn0;

  if (sh < 0) {
    size_t aw = a->size - 1;
    int    ap = bnx_bitsl(a->limb[aw]);
    size_t sw = (size_t)((-sh) / LIMB_BITS);
    int    sp = (size_t)((-sh) % LIMB_BITS);
    size_t rs = aw - sw + ((sp < ap)? 1: 0);

    if ((aw > sw) || ((aw == sw) && (ap > sp))) {
      NEWBN(r, rs, "bnashll");
      r->size = bnx_shright(r->limb, rs, a->limb, aw, ap, sw, sp);
      r->isneg = a->isneg;
    } else {
      return bn0;
    }
  } else if (sh > 0) {
    size_t aw = a->size - 1;
    int    ap = bnx_bitsl(a->limb[aw]);
    size_t sw = (size_t)(sh / LIMB_BITS);
    int    sp = sh % LIMB_BITS;
    size_t rs = aw + sw + (((sp + ap) >= LIMB_BITS)? 2: 1);

    assert(rs >= a->size);

    NEWBN(r, rs, "bnashll");
    r->size = bnx_shleft(r->limb, rs, a->limb, aw, ap, sw, sp);
    r->isneg = a->isneg;
  } else {
    return bndup(a);
  }
  return_NORMALIZE(r, "bnashll");
}

bignum_t *bnmul(const bignum_t *a, const bignum_t *b)
{
  size_t rs;
  bignum_t *r;

  assert(a != NULL);
  assert(b != NULL);
  CHECKSIGN(a);
  CHECKSIGN(b);

  if (BNZERO(a))
    return bn0;
  if (BNPLUSONE(a))
    return bndup(b);
  if (BNMINUSONE(a))
    return bnneg(b);

  if (BNZERO(b))
    return bn0;
  if (BNPLUSONE(b))
    return bndup(a);
  if (BNMINUSONE(b))
    return bnneg(a);

  rs = a->size + b->size;
  if (rs < a->size)
    bnx_failure("bnmul");
  NEWBN(r, rs, "bnmul");

  r->size = bnx_mul(r->limb, rs, a->limb, a->size, b->limb, b->size);
  r->isneg = a->isneg != b->isneg;
  return_NORMALIZE(r, "bnmul");
}

bignum_t *bnmulll(const bignum_t *n, int64_t v)
{
  struct bignum_ll b;
  bignum_t *r;

  assert(n != NULL);
  CHECKSIGN(n);

  if (BNZERO(n))
    return bn0;
  if (BNPLUSONE(n))
    return bndup(bnx_makell(&b, v));
  if (BNMINUSONE(n))
    return bnneg(bnx_makell(&b, v));

  if (v == 0)
    return bn0;
  if (v == 1)
    return bndup(n);
  if (v == -1)
    return bnneg(n);

  if (v > 0) {
    if (v <= LIMB_MAX) {
      r = bny_maddl(n, (limb_t)v, 0);
    } else {
      bignum_t *bx = bnx_makell(&b, v);
      size_t rs = n->size + bx->size;
      if (rs < n->size)
        bnx_failure("bnmulll");
      NEWBN(r, rs, "bnmulll");
      r->size = bnx_mul(r->limb, rs,
        n->limb, n->size,
        bx->limb, bx->size);
    }
    r->isneg = n->isneg;
  } else if (v == LONG_MIN) {
    bignum_t *bx = bnx_makell(&b, v);
    size_t rs = n->size + bx->size;
    if (rs < n->size)
      bnx_failure("bnmulll");
    NEWBN(r, rs, "bnmulll");
    r->size = bnx_mul(r->limb, rs,
      n->limb, n->size,
      bx->limb, bx->size);
    r->isneg = !n->isneg;
  } else {
    v = -v;
    if (v <= LIMB_MAX) {
      r = bny_maddl(n, (limb_t)v, 0);
    } else {
      bignum_t *bx = bnx_makell(&b, v);
      size_t rs = n->size + bx->size;
      if (rs < n->size)
        bnx_failure("bnmulll");
      NEWBN(r, rs, "bnmulll");
      r->size = bnx_mul(r->limb, rs,
        n->limb, n->size,
        bx->limb, bx->size);
    }
    r->isneg = !n->isneg;
  }
  return_NORMALIZE(r, "bnmulll");
}

bignum_t *bnexptull(const bignum_t *a, uint64_t n)
{
  bignum_t *b = bn1;
  bignum_t *an;

  assert(a != NULL);
  CHECKSIGN(a);

  if (BNZERO(a))
    return bn0;

  if (BNPLUSONE(a))
    return bn1;

  if (BNMINUSONE(a)) {
    if (n & 1)
      return bndup(a);
    else
      return bn1;
  }

  if (BNPLUSTWO(a)) { /* esl++ */
    if (n >= INT64_MAX) bnx_failure("bnexptull");  
    return bnashll(bn1, n);
  }

  if (BNMINUSTWO(a)) { /* esl++ */
    if (n >= INT64_MAX) bnx_failure("bnexptull");
    b = bnashll(bn1, n);
    if ((n & 1) == 0) return b;
    an = bnneg(b);
    bnfree(b);
    return an;
  }

  for (an = bndup(a); n != 0; n >>= 1) {
    if (n & 1) {
      bignum_t *t = bnmul(b, an);
      bnfree(b);
      b = t;
    }
    if (n > 1) {
      bignum_t *z = bnmul(an, an);
      bnfree(an);
      an = z;
    }
  }
  bnfree(an);
  return b;
}

bignum_t *bndmod(bignum_t **rem, const bignum_t *num, const bignum_t *den)
{
  bignum_t *d;
  assert(num != NULL);
  assert(den != NULL);
  assert(rem != NULL);
  CHECKSIGN(num);
  CHECKSIGN(den);

  if (BNZERO(den))
    bnx_zdiv();
  if (BNPLUSONE(den)) {
    *rem = bn0;
    return bndup(num);
  }
  if (BNMINUSONE(den)) {
    *rem = bn0;
    return bnneg(num);
  }

  if (den->size == 1) {
    limb_t r;
    int64_t v;
    struct bignum_ll b;
    d = bnz_dmodl(&r, num, den->limb[0]);
    v = r;
    if (num->isneg)
      v = -v;
    *rem = bndup(bnx_makell(&b, v));
    if (!BNZERO(d))
      d->isneg = num->isneg != den->isneg;
  } else {
    d = bnz_dmod(rem, num, den);
    if (!BNZERO(*rem))
      (*rem)->isneg = num->isneg;
    if (!BNZERO(d))
      d->isneg = num->isneg != den->isneg;
  }
  return_NORMALIZE(d, "bndmod");
}

bignum_t *bndmodl(long *rem, const bignum_t *num, long den)
{
  bignum_t *d;
  bignum_t *r = bn0;
  struct bignum_ll b;
  int isneg;
  limb_t rl;

  assert(num != NULL);
  assert(rem != NULL);
  CHECKSIGN(num);

  if (den >= LIMB_MAX) {
    d = bnz_dmod(&r, num, bnx_makell(&b, den));
    isneg = 0;
    goto from_limb;
  } else if (den > 1) {
    d = bnz_dmodl(&rl, num, (limb_t)den);
    isneg = 0;
  } else if (den == 1) {
    *rem = 0;
    return bndup(num);
  } else if (den == 0) {
    bnx_zdiv();
    return NULL; /* never happen */
  } else if (den == -1) {
    *rem = 0;
    return bnneg(num);
  } else if (den > -(slimb_t)LIMB_MAX) {
    d = bnz_dmodl(&rl, num, (limb_t)-den);
    isneg = 1;
  } else {
    d = bnz_dmod(&r, num, bnx_makell(&b, den));
    isneg = 1;
    goto from_limb;
  }

  if (num->isneg)
    *rem = -(int64_t)rl;
  else
    *rem = rl;

  if (!BNZERO(d))
    d->isneg = isneg != num->isneg;
  return_NORMALIZE(d, "bndmodl");

from_limb:
#if (ULONG_MAX > LIMB_MAX)
  {
    size_t i;
    int64_t v;
    for (i = r->size, v = 0; i--;)
      v += (v << LIMB_BITS) | r->limb[i];
    if (num->isneg)
      v = -v;
    *rem = v;
  }
#elif (ULONG_MAX == LIMB_MAX)
  if (BNZERO(r)) {
    *rem = 0;
  } else {
    *rem = ((long)(r->limb[0]))*(num->isneg? -1: +1);
  }
#else
#error "Long is unreasonably short"
#endif
  bnfree(r);
  if (!BNZERO(d))
    d->isneg = isneg != num->isneg;
  return_NORMALIZE(d, "bndmodl");
}

bignum_t *bndiv(const bignum_t *num, const bignum_t *den)
{
  bignum_t *rem;
  bignum_t *div = bndmod(&rem, num, den);

  bnfree(rem);
  return div;
}

bignum_t *bndivl(const bignum_t *num, long den)
{
  bignum_t *r;
  bignum_t *rem = bn0;
  struct bignum_ll b;
  limb_t reml;
  int isneg;

  assert(num != NULL);
  CHECKSIGN(num);

  if (den > 0 && den >= LIMB_MAX) {
    r = bnz_dmod(&rem, num, bnx_makell(&b, den));
    isneg = num->isneg;
  } else if (den > 1) {
    r = bnz_dmodl(&reml, num, den);
    isneg = num->isneg;
  } else if (den == 1) {
    return bndup(num);
  } else if (den == 0) {
    bnx_zdiv();
    return NULL; /* never happen */
  } else if (den == -1) {
    return bnneg(num);
  } else if (den > -(slimb_t)LIMB_MAX) {
    r = bnz_dmodl(&reml, num, -den);
    isneg = !num->isneg;
  } else {
    r = bnz_dmod(&rem, num, bnx_makell(&b, den));
    isneg = !num->isneg;
  }
  if (!BNZERO(r))
    r->isneg = isneg;
  bnfree(rem);
  return_NORMALIZE(r, "bndivl");
}

bignum_t *bnmod(const bignum_t *num, const bignum_t *den)
{
  bignum_t *rem;
  bignum_t *div = bndmod(&rem, num, den);

  bnfree(div);
  return rem;
}

long bnmodl(const bignum_t *num, long den)
{
  bignum_t *rem = bn0;
  struct bignum_ll b;
  limb_t reml;
  assert(num != NULL);
  CHECKSIGN(num);

  if (den >= LIMB_MAX) {
    bnfree(bnz_dmod(&rem, num, bnx_makell(&b, den)));
    goto from_limb;
  } else if (den > 1) {
    bnfree(bnz_dmodl(&reml, num, den));
  } else if (den == 1) {
    return 0;
  } else if (den == 0) {
    bnx_zdiv();
    return 0; /* never happen */
  } else if (den == -1) {
    return 0;
  } else if (den > -(slimb_t)LIMB_MAX) {
    bnfree(bnz_dmodl(&reml, num, -den));
  } else {
    bnfree(bnz_dmod(&rem, num, bnx_makell(&b, den)));
    goto from_limb;
  }
  if (num->isneg)
    return -(int64_t)reml;
  else
    return reml;
from_limb:
#if (ULONG_MAX > LIMB_MAX)
  {
    size_t i;
    int64_t v;
    for (i = rem->size, v = 0; i--;)
      v += (v << LIMB_BITS) | rem->limb[i];
    if (num->isneg)
      v = -v;
    bnfree(rem);
    return v;
  }
#elif (ULONG_MAX == LIMB_MAX)
  if (BNZERO(rem))
    return (bnfree(rem), 0);
  reml = rem->limb[0];
  bnfree(rem);
  return ((num->isneg)? -(slimb_t)reml: reml);
#else
#error "Long is unreasonably short"
#endif
}

bignum_t *bngcd(const bignum_t *x, const bignum_t *y)
{
  bignum_t *a, *b;
  bignum_t *r;
  assert(x != NULL);
  CHECKSIGN(x);
  assert(y != NULL);
  CHECKSIGN(y);

  a = bnabs(x);
  b = bnabs(y);

  while (b != bn0) {
    r = bnmod(a, b);
    bnfree(a);
    a = b;
    b = r;
  }
  return a;
}

/*  abs    plus    minus
*    0          0
*    1      01        1
*    2     010       10   
*    3     011      101
*    4    0100      100
*    5    0101     1011
*    6    0110     1010
*    7    0111     1001
*    8   01000     1000
*/

/* # of bits needed for unsigned representation of |n| */
size_t bnwidthu(const bignum_t *n)
{
  int64_t wd;
  limb_t h;

  assert(n != NULL);
  CHECKSIGN(n);

  if (BNZERO(n))
    return 0;

  assert(n->size > 0);
  assert(n->limb[n->size - 1] != 0);

  h = n->limb[n->size - 1];
  wd = ((int64_t)(n->size-1)*LIMB_BITS + bnx_bitsl(h));

  if (wd > SIZE_MAX)
    return (errno=ERANGE, 0);

  return (size_t)wd;
}

/* # of bits needed for 2-complement signed representation of n */
size_t bnwidths(const bignum_t *n)
{
  int64_t wd;
  limb_t h;

  assert(n != NULL);
  CHECKSIGN(n);

  if (BNZERO(n))
    return 1;

  assert(n->size > 0);
  assert(n->limb[n->size - 1] != 0);

  h = n->limb[n->size - 1];
  if (n->isneg == 0) {
    wd = ((int64_t)(n->size-1)*LIMB_BITS + (bnx_bitsl(h) + 1));
  } else {
    int i;
    limb_t w = 0;
    for (i = (int)n->size - 1; i--;) {
      w |= n->limb[i];
    }
    if ((w == 0) && ((h & (h - 1)) == 0)) {
      wd = (int64_t)(n->size-1)*LIMB_BITS + bnx_bitsl(h);
    } else {
      wd = (int64_t)(n->size-1)*LIMB_BITS + (bnx_bitsl(h) + 1);
    }
  }
  if (wd > SIZE_MAX)
    return (errno=ERANGE, 0);

  return (size_t)wd;
}

long bntol(const bignum_t *n)
{
  long v;

  assert(n != NULL);
  CHECKSIGN(n);

  if (bnwidths(n) > sizeof(long) * CHAR_BIT)
    return (errno = ERANGE, 0);

#if (ULONG_MAX > LIMB_MAX)
  {
    int i;
    for (v = 0, i = n->size; i--;)
      v = (v << (sizeof(limb_t) * CHAR_BIT)) + n->limb[i];
  }
#elif (ULONG_MAX == LIMB_MAX)
  v = n->limb[0]; 
#else
#error "Long is unreasonably short"
#endif

  if (n->isneg)
    return -v;
  else
    return v;
}

/* esl+ */
int64_t bntoll(const bignum_t *n)
{
  int64_t v;

  assert(n != NULL);
  CHECKSIGN(n);

  if (bnwidths(n) > sizeof(int64_t) * CHAR_BIT)
    return (errno = ERANGE, 0);

#if (UINT64_MAX > LIMB_MAX)
  {
    size_t i;
    for (v = 0, i = n->size; i--;)
      v = (v << (sizeof(limb_t) * CHAR_BIT)) + n->limb[i];
  }
#elif (UINT64_MAX == LIMB_MAX)
  v = n->limb[0]; 
#else
#error "Long is unreasonably short"
#endif

  if (n->isneg)
    return -v;
  else
    return v;
}

/* esl+ */
uint64_t bntoull(const bignum_t *n)
{
  uint64_t v;

  assert(n != NULL);
  CHECKSIGN(n);

  if (n->isneg || bnwidthu(n) > sizeof(uint64_t) * CHAR_BIT)
    return (errno = ERANGE, 0);

#if (UINT64_MAX > LIMB_MAX)
  {
    size_t i;
    for (v = 0, i = n->size; i--;)
      v = (v << (sizeof(limb_t) * CHAR_BIT)) + n->limb[i];
  }
#elif (UINT64_MAX == LIMB_MAX)
  v = n->limb[0]; 
#else
#error "Long is unreasonably short"
#endif

    return v;
}

double bntod(const bignum_t *n)
{
  double v, s;
  int i;

  assert(n != NULL);
  CHECKSIGN(n);

  for (s = 1, v = 0, i = 0; i < (int)n->size; i++) {
    if (s >= HUGE_VAL) { errno = ERANGE; v = HUGE_VAL; break; } /* [esl+] */
    v = v + s * n->limb[i];
    if (v >= HUGE_VAL) { errno = ERANGE; break; } /* [esl+] */
    s = s * (1.0 + LIMB_MAX);
  }
  if (n->isneg)
    return -v;
  else
    return v;
}

#ifndef COMPACT_BNTOSTR /* [esl*] big but fast print */
/* power-of-two fast printer (radices 2, 4, 8, 16, 32) */
static char *bntostr_po2(char *buffer, size_t len, const bignum_t *n, int bbits)
{
  size_t total_bits = bnwidthu(n);
  const size_t limb_bits = LIMB_BITS;
  const limb_t mask = ((limb_t)1 << bbits) - 1;
  char *ptr = buffer + len - 1;

  *ptr = '\0';

  if (total_bits == 0) {
    if (ptr == buffer) return NULL;
    *--ptr = '0';
    return ptr;
  } else {
    size_t bit_pos = 0;
    while (bit_pos < total_bits) {
      size_t limb_idx = bit_pos / limb_bits;
      size_t offset   = bit_pos % limb_bits;
      limb_t digit;
      if (ptr == buffer) return NULL;
      digit = n->limb[limb_idx] >> offset;
      if (offset + bbits > limb_bits && limb_idx + 1 < n->size) {
        digit |= (n->limb[limb_idx + 1] << (limb_bits - offset));
      }
      digit &= mask;
      *--ptr = gn_digit((int)digit);
      bit_pos += bbits;
    }
  }

  if (n->isneg) {
    if (ptr == buffer) return NULL;
    *--ptr = '-';
  }

  return ptr;
}

/* calc radix that is a multiple of the original radix fitting in limb */
static void get_super_radix(int radix, limb_t *out_chunk, int *out_digits)
{
  limb_t chunk = (limb_t)radix;
  int digits = 1;
  while (chunk <= LIMB_MAX / (limb_t)radix) {
    chunk *= (limb_t)radix;
    digits++;
  }
  *out_chunk = chunk;
  *out_digits = digits;
}

/* simple iterative printer for small bignums */
static size_t bntostr_small(char *out, const bignum_t *n, size_t req_digits, int is_top, int radix)
{
  limb_t big_chunk;
  int chunk_digits;
  limb_t chunks[128];
  int i, num_chunks = 0;
  char tmp[512];
  int tmp_pos = 512;
  bignumll_t bnll_chunk;
  bignum_t *bn_chunk, *curr;

  /* clean zero-handling for sub-blocks vs top-level zero */
  if (bnwidthu(n) == 0) {
    size_t total = is_top ? 1 : req_digits;
    if (total > 0) memset(out, '0', total);
    return total;
  }

  get_super_radix(radix, &big_chunk, &chunk_digits);
  bnll_chunk = bnll(big_chunk);
  bn_chunk = (bignum_t *)&bnll_chunk;
  curr = (bignum_t *)n;

  while (bnwidthu(curr) > 0) {
    bignum_t *rem = NULL;
    bignum_t *q = bndmod(&rem, curr, bn_chunk);
    if (num_chunks >= 128) break;
    chunks[num_chunks++] = (rem && rem->size > 0) ? rem->limb[0] : 0;
    if (rem) bnfree(rem);
    if (curr != n) bnfree(curr);
    curr = q;
  }
  if (curr != n) bnfree(curr);

  for (i = 0; i < num_chunks; i++) {
    limb_t val = chunks[i];
    int digits_to_write = (i == num_chunks - 1) ? 0 : chunk_digits;
    if (digits_to_write == 0) {
      do {
        if (tmp_pos == 0) break;
        tmp[--tmp_pos] = gn_digit((int)(val % radix));
        val /= radix;
      } while (val > 0);
    } else {
      int d;
      for (d = 0; d < chunk_digits; d++) {
        if (tmp_pos == 0) break;
        tmp[--tmp_pos] = gn_digit((int)(val % radix));
        val /= radix;
      }
    }
  }

  { size_t num_digits = 512 - tmp_pos;
    size_t total_len = (!is_top && req_digits > num_digits) ? req_digits : num_digits;
    size_t leading_zeros = total_len - num_digits;
    memset(out, '0', leading_zeros);
    memcpy(out + leading_zeros, tmp + tmp_pos, num_digits);
    return total_len;
  }
}

#define DC_THRESHOLD 32

/* divide-and-conquer printer for arbitrary radices */
static size_t bntostr_dc_rec(char *out, const bignum_t *n, size_t req_digits, int is_top, int radix)
{
  size_t bits = bnwidthu(n);
  size_t est_digits;
  bignumll_t bnll_radix;
  size_t d, req_q, q_len, r_len;
  bignum_t *p, *q, *r;

  est_digits = (size_t)((double)bits / log((double)radix) * log(2.0)) + 1;

  if (est_digits <= DC_THRESHOLD)
      return bntostr_small(out, n, req_digits, is_top, radix);

  d = est_digits / 2;
  bnll_radix = bnll(radix);
  p = bnexptull((bignum_t *)&bnll_radix, (uint64_t)d);
  r = NULL;
  q = bndmod(&r, n, p);
  bnfree(p);

  if (bnwidthu(q) == 0 && is_top) {
    size_t len = bntostr_dc_rec(out, r, req_digits, 1, radix);
    bnfree(q);
    bnfree(r);
    return len;
  }

  /* calculate required digits for q */
  req_q = (req_digits > d) ? (req_digits - d) : 0;
  q_len = bntostr_dc_rec(out, q, req_q, is_top, radix);
  bnfree(q);

  r_len = bntostr_dc_rec(out + q_len, r, d, 0, radix);
  bnfree(r);

  return q_len + r_len;
}

/* main entry: dispatch on radix and number size */
char *bntostr(char *buffer, size_t len, const bignum_t *n, int radix)
{
  size_t bits = bnwidthu(n);
  size_t est_digits;
  size_t min_buf_needed;
  char *tmp;
  size_t prefix_len = 0, body_len, total_len;
  char *right_ptr;

  if (radix == 2 || radix == 4 || radix == 8 || radix == 16 || radix == 32) {
    int bbits = radix == 2 ? 1 : radix == 4 ? 2 : radix == 8 ? 3 : radix == 16 ? 4 : 5;
    char *res = bntostr_po2(buffer, len, n, bbits);
    assert(res);
    return res;
  }

  est_digits = (size_t)((double)bits / log((double)radix) * log(2.0)) + 1;

  min_buf_needed = est_digits + (n->isneg ? 2 : 1);
  assert(len >= min_buf_needed);
  
  tmp = buffer;
  if (n->isneg && bits > 0) {
    *tmp++ = '-';
    prefix_len = 1;
  }

  if (est_digits <= DC_THRESHOLD) body_len = bntostr_small(tmp, n, 0, 1, radix);
  else body_len = bntostr_dc_rec(tmp, n, 0, 1, radix);

  /* make sure number is right-aligned in buffer */
  total_len = prefix_len + body_len;
  buffer[total_len] = '\0';
  right_ptr = buffer + (len - 1 - total_len);
  memmove(right_ptr, buffer, total_len + 1);

  return right_ptr;
}
#else /* simple but slow */
char *bntostr(char *buffer, size_t len, const bignum_t *n, int radix)
{
  /* IMPROV */
  char *ptr = buffer + len - 1;
  bignum_t *v;

  assert(buffer != NULL);
  assert(len > 1);
  assert(n != NULL);
  CHECKSIGN(n);
  assert(radix >= 2);
  assert(radix <= 36);

  *ptr = 0;
  if (BNZERO(n)) {
    *--ptr = '0';
    return ptr;
  }

  if (n->isneg)
    v = bnneg(n);
  else
    v = bndup(n);

  do {
    limb_t r;
    bignum_t *d = bnz_dmodl(&r, v, radix);
    bnfree(v);
    v = d;
    if (ptr == buffer) {
      bnfree(v);
      return NULL;
    }
    if (r < 10)
      *--ptr = r + '0';
    else
      *--ptr = r + 'a' - 10;
  } while (!BNZERO(v));
  bnfree(v);

  if (n->isneg) {
    if (ptr == buffer)
      return NULL;
    *--ptr = '-';
  }
  return ptr;
}
#endif

bignum_t *lltobn(int64_t n)
{
  struct bignum_ll bl;

  switch (n) {
  case 0:
    return bn0;
  case 1:
    return bn1;
  default:
    return bndup(bnx_makell(&bl, n));
  }
}

bignum_t *ulltobn(uint64_t n)
{
  struct bignum_ll bl;

  switch (n) {
  case 0:
    return bn0;
  case 1:
    return bn1;
  default:
    return bndup(bnx_makeull(&bl, n));
  }
}

static bignum_t *dtobn(double x)
{
  bignum_t *v = bn0;
  double f;
  int i, e;

  if (x != x || fabs(x) > DBL_MAX) /* exclude nans, infinities */
    return NULL;

  f = frexp(x, &e);

  for (i = 0; e > 0 && i < sizeof(double) * CHAR_BIT; i++, e--) {
    bignum_t *v2 = bnashll(v, 1);

    bnfree(v);
    f = f * 2.0;
    if (f >= 1.0) {
      bignum_t *s = bnaddll(v2, 1);
      bnfree(v2);
      v2 = s;
      f = f - 1.0;
    }
    v = v2;
  }
  if (e > 0) {
    bignum_t *s = bnashll(v, e);
    bnfree(v);
    v = s;
  }
  if (x < 0) {
    bignum_t *x = bnneg(v);
    bnfree(v);
    return x;
  } else {
    return v;
  }
}

bignum_t *strtobn(const char *str, char **endp, int radix)
{
  /* IMPROV */
  int isneg; int has_bn, ndigs;
  bignum_t *v = NULL;

  assert(str != NULL);
  assert(radix >= 2);
  assert(radix <= 36);

  while (isspace(*str))
    str++;

  switch (*str) {
  case '+': isneg = 0; str++; break;
  case '-': isneg = 1; str++; break;
  case 0: if (endp) *endp = (char *)str; return (errno = EDOM, NULL);
  default: isneg = 0; break;
  }

  for (v = bn0, has_bn = ndigs = 0;; str++) {
    char ch = *str;
    int git = (((ch >= '0') && (ch <= '9'))? ch - '0':
      ((ch >= 'a') && (ch <= 'z')) ? ch - 'a' + 10:
      ((ch >= 'A') && (ch <= 'Z')) ? ch - 'A' + 10 : 36);
    bignum_t *r;
    if (git >= radix)
      break;
    if (git > 0) has_bn = 1;
    ++ndigs;
    r = bny_maddl(v, radix, git);
    bnfree(v);
    v = r;
  }
  if (endp)
    *endp = (char *)str;

  if (ndigs) {
    if (has_bn)
      v->isneg = isneg;
    return_NORMALIZE(v, "strtobn");
  } else {
    bnfree(v);
    return (errno = EDOM, NULL);
  }
}

/* log(radix) * 65536 */
static double log_radix[] = {
  45426,  /*  2 */  71998, /*  3 */  90852, /*  4 */ 105476, /*  5 */
  117424, /*  6 */ 127527, /*  7 */ 136278, /*  8 */ 143997, /*  9 */
  150902, /* 10 */ 157148, /* 11 */ 162850, /* 12 */ 168096, /* 13 */
  172953, /* 14 */ 177474, /* 15 */ 181704, /* 16 */ 185677, /* 17 */
  189423, /* 18 */ 192966, /* 19 */ 196328, /* 20 */ 199525, /* 21 */
  202574, /* 22 */ 205487, /* 23 */ 208276, /* 24 */ 210952, /* 25 */
  213522, /* 26 */ 215995, /* 27 */ 218379, /* 28 */ 220679, /* 29 */
  222900, /* 30 */ 225049, /* 31 */ 227130, /* 32 */ 229147, /* 33 */
  231103, /* 34 */ 233003, /* 35 */ 234849  /* 36 */
};

#define LOG_BASE (2 * 726817. + 1) /* log(*W*) * 65536 */

size_t bnfmtsize(const bignum_t *n, int radix)
{
  double x;
  size_t xr;

  assert(n != NULL);
  CHECKSIGN(n);
  assert(radix >= 2);
  assert(radix <= 36);

  /* include space for the sign and the traling zero */
  x = 4 + (n->size * (LOG_BASE + 2) / (log_radix[radix - 2] - 2));
  xr = (size_t)x;

  if (xr < x)
    xr++;
  if (xr < x)
    return (errno = ERANGE, 0);

  return xr;
}

/* [esl++] 'generic' writer for bignums */
int wrbn(const bignum_t *n, int radix, int (*pf)(int, void*), void *pd)
{
  size_t len;
  char *buffer;
  char *ptr;

  assert(n != NULL);
  CHECKSIGN(n);
  
  if (radix < 2 || radix > 36) return -1;
  len = bnfmtsize(n, radix);
  if (len == 0) return 0; /* huh? */
  buffer = bnrealloc(NULL, len); // use regular alloc???
  ptr = bntostr(buffer, len, n, radix);
  for (; *ptr; ++ptr) (*pf)(*ptr, pd); 
  bnrealloc(buffer, 0);
  return 0;
}

/* [esl++] */
/* to simplify calculations, we limit the size of bignums to FIXNUM_MAX;
* for 30-bit fixnums, it is ~64MB, which is well in the degrading usability
* territory, outside of very special tasks. Having FIXNUM_MAX as the
* limit allows us not to deal with bignum exponents in expt */
#define BIGNUM_MAX_BITS FIXNUM_MAX

/* [esl++] convert ratio of two bignums to a double */
double bnrtod(const bignum_t *n, const bignum_t *d)
{
  bignum_t *a, *b, *q, *r, *t; 
  size_t wa, wb; long e, low; double v;
  int s = ((bnsign(n) < 0) ^ (bnsign(d) < 0)) ? -1 : 1;

  if (bnzero(n)) return s * 0.0;

  a = bnabs(n), b = bnabs(d);
  wa = bnwidthu(a), wb = bnwidthu(b); /* kept under BIGNUM_MAX_BITS */
  e = (long)wa - (long)wb;

  if (bnzero(d)) return bnfree(a), bnfree(b), s * HUGE_VAL;
  if (e > DBL_MAX_EXP - 1) return bnfree(a), bnfree(b), s * HUGE_VAL;
  if (e < DBL_MIN_EXP - DBL_MANT_DIG) return bnfree(a), bnfree(b), s * 0.0;

  /* scale to get DBL_MANT_DIG+2 bits (guard, round, sticky in remainder) */
  t = bnashll(a, (long)(DBL_MANT_DIG + 2) + (long)wb - (long)wa);
  q = bndmod(&r, t, b);
  bnfree(t); bnfree(a); bnfree(b);

  /* GRS rounding: look at bottom 2 bits of q and sticky bit (r) */
  low = bnmodl(q, 4);
  if (low > 2) { /* 11x: round up */
    t = bnaddll(q, 1); bnfree(q); q = t;
  } else if (low == 2) {
    /* 10x: half-way, check sticky */
    if (!bnzero(r)) { /* 101: round up */
      t = bnaddll(q, 1); bnfree(q); q = t;
    } else { /* 100: tie - round to even (check bit 2) */
      t = bnashll(q, -2);
      if (bnodd(t)) {
        bignum_t *p = bnaddll(q, 1); bnfree(q); q = p;
      }
      bnfree(t);
    }
  } /* else 0xx or 01x: truncate */

  bnfree(r);

  /* drop guard+round bits */
  t = bnashll(q, -2); bnfree(q); q = t;

  /* handle overflow from rounding (now has DBL_MANT_DIG+1 bits) */
  if (bnwidthu(q) > DBL_MANT_DIG) {
    ++e; t = bnashll(q, -1); bnfree(q); q = t;
  }

  v = s * ldexp(bntod(q), e - DBL_MANT_DIG);
  bnfree(q);
  return v;
}

/* [esl+] inexact sqrt bundle */

/* returns square root of a normalized |n|/d ratio as a double */
double bnrsqrttod(const bignum_t *n, const bignum_t *d)
{
  double dn, dd, m;
  long long sn, sd, s, k; int r;
  const double limb_base = (double)LIMB_MAX + 1.0; /* 2^32 = 4294967296.0 */

  assert(!BNZERO(d) && !d->isneg);
  if (BNZERO(n)) return 0.0;

  /* extract top two limbs of |n| */
  dn = (double)n->limb[n->size - 1];
  if (n->size > 1) {
    dn = dn * limb_base + (double)n->limb[n->size - 2];
    sn = (long long)(n->size - 2) * LIMB_BITS;
  } else {
    sn = (long long)(n->size - 1) * LIMB_BITS;
  }

  /* extract top two limbs of d */
  dd = (double)d->limb[d->size - 1];
  if (d->size > 1) {
      dd = dd * limb_base + (double)d->limb[d->size - 2];
      sd = (long long)(d->size - 2) * LIMB_BITS;
  } else {
      sd = (long long)(d->size - 1) * LIMB_BITS;
  }

  /* shift difference s = 2k + r, ensuring remainder r in {0, 1} */
  s = sn - sd;
  k = s / 2;
  r = (int)(s % 2);
  if (r < 0) { r += 2; k -= 1; }

  /* mantissa ratio m = (dn / dd) * 2^r */
  m = ldexp(dn / dd, r);

  /* range guard & final hardware scaling */
  if (k > DBL_MAX_EXP)  return HUGE_VAL;
  if (k < -DBL_MAX_EXP) return 0.0;

  return ldexp(sqrt(m), (int)k);
}

/* returns square root of a normalized |n|/d ratio as a double */
double bnsqrttod(const bignum_t *n)
{
  return bnrsqrttod(n, (const bignum_t *)bn1);
}

/* returns square root of normalized (rn/rd)+(in/id)i as doubles */
void bncsqrttodd(double *prd, double *pid, const bignum_t *rn, const bignum_t *rd, const bignum_t *in, const bignum_t *id)
{
  double sx, sy, smax, sx_p, sy_p, x_p, y_p, z_mag_p, u_p, v_p;
  double v_final, u_final;

  assert(prd != NULL && pid != NULL);

  /* get non-negative magnitude square roots: sx = sqrt(|x|), sy = sqrt(|y|) */
  sx = bnrsqrttod(rn, rd);
  sy = bnrsqrttod(in, id);

  smax = (sx > sy) ? sx : sy;
  if (smax == 0.0) {
    *prd = 0.0;
    *pid = 0.0;
    return;
  }

  /* scale components to <= 1.0 to guarantee zero intermediate overflow */
  sx_p = sx / smax; sy_p = sy / smax;
  x_p = rn->isneg ? -(sx_p * sx_p) : (sx_p * sx_p);
  y_p = in->isneg ? -(sy_p * sy_p) : (sy_p * sy_p);

  /* scaled magnitude |z'| = sqrt(x'^2 + y'^2) */
  z_mag_p = sqrt(x_p * x_p + y_p * y_p);

  /* numerically stable complex square root with direct-rescue path */
  if (x_p >= 0.0) {
    u_p = sqrt((z_mag_p + x_p) * 0.5);
    if (u_p != 0.0) {
      v_p = fabs(y_p) / (2.0 * u_p);
      /* rescue: y_p underflowed but sy != 0; compute unscaled v directly */
      if (v_p == 0.0 && sy != 0.0) {
        v_final = (sy / (2.0 * u_p)) * sy_p;
        *prd = u_p * smax;
        *pid = in->isneg ? -v_final : v_final;
        return;
      }
    } else {
      v_p = 0.0;
    }
  } else {
    v_p = sqrt((z_mag_p - x_p) * 0.5);
    if (v_p != 0.0) {
      u_p = fabs(y_p) / (2.0 * v_p);
      /* rescue: y_p underflowed but sy != 0; compute unscaled u directly. */
      if (u_p == 0.0 && sy != 0.0) {
        u_final = (sy / (2.0 * v_p)) * sy_p;
        *prd = u_final;
        *pid = in->isneg ? -(v_p * smax) : (v_p * smax);
        return;
      }
    } else {
      u_p = 0.0;
    }
  }

  /* rescale components back and assign correct imaginary branch sign */
  *prd = u_p * smax;
  *pid = in->isneg ? -(v_p * smax) : (v_p * smax);
}


/* [esl+] exact sqrt bundle */

/* fast approximate conversion of a bignum to a hardware double */
double bntod_approx(const bignum_t *n)
{
  double d; long long shift;
  const double limb_base = (double)LIMB_MAX + 1.0; /* 2^32 = 4294967296.0 */

  if (BNZERO(n)) return 0.0;

  /* the most significant limb */
  d = (double)n->limb[n->size - 1];

  /* blend in second limb if present to populate all mantissa bits */
  if (n->size > 1) {
    d = d * limb_base + (double)n->limb[n->size - 2];
    shift = (long long)(n->size - 2) * LIMB_BITS;
  } else {
    shift = (long long)(n->size - 1) * LIMB_BITS;
  }

  /* clamp shift to prevent ldexp exponent overflow */
  if (shift > 2 * DBL_MAX_EXP)  return n->isneg ? -HUGE_VAL : HUGE_VAL;
  if (shift < -2 * DBL_MAX_EXP) return 0.0;

  /* hardware floating-point scaling */
  d = ldexp(d, (int)shift);

  return n->isneg ? -d : d;
}

/* initial overestimate x0 for Newton-Raphson integer sqrt */
static bignum_t *bnisqrt_initial_seed(const bignum_t *n_abs)
{
  size_t w = bnwidthu(n_abs);
  bignum_t *x = NULL;
  const size_t MAX_EXACT_BITS = (size_t)(DBL_MANT_DIG - 1); /* 52 bits */

  if (w <= MAX_EXACT_BITS) {
    /* fits directly into double mantissa */
    double d = fabs(bntod_approx(n_abs));
    double s = sqrt(d);
    /* + 1.0 bias strictly guarantees x0 >= floor(sqrt(|n|)) */
    x = dtobn(s + 1.0);
  } else {
    /* large: shift right to extract top 52 bits
     * Ensure shift amount 'k' is even so sqrt(2^k) = 2^(k/2) is exact. */
    long long k = (long long)((w - MAX_EXACT_BITS) & ~(unsigned long long)1);
    bignum_t *n_top = bnashll(n_abs, -k); /* >> k */
    double d = fabs(bntod_approx(n_top));
    double s = sqrt(d);
    /* s + 1.0 is tiny (~2^26), so dtobn never sees Inf/NaN */
    bignum_t *x_top = dtobn(s + 1.0);
    if (x_top) {
      /* scale back up by k/2 bits */
      x = bnashll(x_top, (k / 2));
      bnfree(x_top);
    }
    bnfree(n_top);
  }

  return x;
}

/* exact integer square root and remainder on |n| = (*out_root)^2 + (*out_rem)
 * out_root should be non-NULL, but out_rem can be NULL; returns 1 if rem is 0  */
int bnisqrt(const bignum_t *n, bignum_t **out_root, bignum_t **out_rem)
{
  bignum_t *n_abs, *x, *next_x, *q, *sum, *sq, *rem; int res;

  assert(n != NULL);
  assert(out_root != NULL);

  if (BNZERO(n)) {
    *out_root = (bignum_t *)bn0;
    if (out_rem) *out_rem  = (bignum_t *)bn0;
    return 1;
  }

  /* work strictly on absolute magnitude |n| */
  n_abs = bnabs(n);

  /* get hardware-accelerated overestimate seed x0 */
  x = bnisqrt_initial_seed(n_abs);

  /* Newton-Raphson iteration: x_{k+1} = floor((x_k + floor(|n| / x_k)) / 2) */
  for (;;) {
    q = bndiv(n_abs, x); 
    sum = bnadd(x, q);
    bnfree(q);

    next_x = bnashll(sum, -1); /* >> 1 */
    bnfree(sum);

    /* convergence check: x_k decreases monotonically until hitting floor(sqrt(|n|)).
     * once next_x >= x, we have reached the exact integer root. */
    if (bncmp(next_x, x) >= 0) {
      bnfree(next_x);
      break;
    }

    bnfree(x);
    x = next_x;
  }

  *out_root = x;

  /* compute exact non-negative remainder |n| - x^2 */
  sq = bnmul(x, x); rem = bnsub(n_abs, sq);
  bnfree(sq); 
  bnfree(n_abs);

  res = BNZERO(rem);
  if (out_rem) *out_rem  = rem;
  else bnfree(rem);

  return res;
}

/* modular quadratic residue lookup tables */

static const unsigned char sq64[64] = {
  1,1,0,0,1,0,0,0, 0,1,0,0,0,0,0,0,
  1,1,0,0,0,0,0,0, 0,1,0,0,0,0,0,0,
  0,1,0,0,1,0,0,0, 0,1,0,0,0,0,0,0,
  0,1,0,0,0,0,0,0, 0,1,0,0,0,0,0,0
};

/* FIXED: Corrected residues for mod 63 (16 quadratic residues total) */
static const unsigned char sq63[63] = {
  1,1,0,0,1,0,0,1, 0,1,0,0,0,0,0,0,
  1,0,1,0,0,0,1,0, 0,1,0,0,1,0,0,0,
  0,0,0,0,1,1,0,0, 0,0,0,1,0,0,1,0,
  0,1,0,0,0,0,0,0, 0,0,1,0,0,0,0
};

static const unsigned char sq65[65] = {
  1,1,0,0,1,0,0,0, 0,1,1,0,0,0,1,0,
  1,0,0,0,0,0,0,0, 0,1,1,0,0,1,1,0,
  0,0,0,1,1,0,0,1, 1,0,0,0,0,0,0,0,
  0,1,0,1,0,0,0,1, 1,0,0,0,0,1,0,0,
  1
};

static const unsigned char sq11[11] = {
  1,1,0,1,1,1,0,0,0,1,0
};

static unsigned long bn_mag_modl(const bignum_t *n, long m)
{
  long r;
  assert(n->size > 0); /* never called on bn0 */
  if (m == 64) return (unsigned long)(n->limb[0] & 63UL);
  r = bnmodl(n, m);
  if (r < 0) r = -r;
  return (unsigned long)(r % m);
}


/* bignum square root of |n| */
int bntrysqrt(const bignum_t *n, bignum_t **out_root)
{
  bignum_t *root = NULL, *rem = NULL;

  assert(n != NULL);

  if (BNZERO(n)) {
    if (out_root) *out_root = (bignum_t *)bn0;
    return 1;
  }

  if (!sq64[bn_mag_modl(n, 64)]) return 0;
  if (!sq63[bn_mag_modl(n, 63)]) return 0;
  if (!sq65[bn_mag_modl(n, 65)]) return 0;
  if (!sq11[bn_mag_modl(n, 11)]) return 0;

  bnisqrt(n, &root, &rem);

  if (BNZERO(rem)) {
    bnfree(rem);
    if (out_root) *out_root = root;
    else bnfree(root);
    return 1;
  }

  bnfree(rem);
  bnfree(root);
  return 0;
}

/* exact rational square root, ratio is expected to be in reduced form */
int bnrtrysqrt_reduced(const bignum_t *num, const bignum_t *den, bignum_t **out_rn, bignum_t **out_rd)
{
  bignum_t *rn = NULL, *rd = NULL;

  assert(!BNZERO(den) && !den->isneg);

  if (!bntrysqrt(num, &rn)) return 0;
  if (!bntrysqrt(den, &rd)) { bnfree(rn); return 0; }

  if (out_rn) *out_rn = rn; else bnfree(rn);
  if (out_rd) *out_rd = rd; else bnfree(rd);
  return 1;
}

/* exact rational square root, ratio is not expected to be in reduced form */
int bnrtrysqrt(const bignum_t *num, const bignum_t *den, bignum_t **out_rn, bignum_t **out_rd)
{
  bignum_t *g, *num_red, *den_red;
  int res;

  assert(!BNZERO(den) && !den->isneg);

  g = bngcd(num, den);
  if (bncmp(g, bn1) == 0) {
    bnfree(g);
    return bnrtrysqrt_reduced(num, den, out_rn, out_rd);
  }

  num_red = bndiv(num, g); den_red = bndiv(den, g);
  bnfree(g);

  res = bnrtrysqrt_reduced(num_red, den_red, out_rn, out_rd);
  bnfree(num_red);
  bnfree(den_red);
  return res;
}

/* exact complex square root */
int bnctrysqrt(const bignum_t *rn, const bignum_t *rd, const bignum_t *in, const bignum_t *id,
               bignum_t **out_an, bignum_t **out_ad, bignum_t **out_bn, bignum_t **out_bd)
{
  bignum_t *g_den, *rd_prime, *id_prime;
  bignum_t *x, *y, *n_z2, *r_num;
  bignum_t *tmp1, *tmp2, *lcm_den, *a_num, *a_den;
  bignum_t *local_an = NULL, *local_ad = NULL;
  int is_sq;

  assert(!BNZERO(rd) && !rd->isneg);
  assert(!BNZERO(id) && !id->isneg);

  /* pure real case (y = 0) */
  if (BNZERO(in)) {
    if (bnsign(rn) >= 0) {
      if (bnrtrysqrt_reduced(rn, rd, out_an, out_ad)) {
        if (out_bn) *out_bn = (bignum_t *)bn0;
        if (out_bd) *out_bd = (bignum_t *)bn1;
        return 1;
      }
    } else {
      if (bnrtrysqrt_reduced(rn, rd, out_bn, out_bd)) {
        if (out_an) *out_an = (bignum_t *)bn0;
        if (out_ad) *out_ad = (bignum_t *)bn1;
        return 1;
      }
    }
    return 0;
  }

  /* reduce denominators upfront: rd' = rd/g, id' = id/g */
  g_den = bngcd(rd, id);
  rd_prime = bndiv(rd, g_den);
  id_prime = bndiv(id, g_den);

  /* x = rn * id', y = in * rd' */
  x = bnmul(rn, id_prime);
  y = bnmul(in, rd_prime);

  /* n_z2 = x^2 + y^2 */
  tmp1 = bnmul(x, x);
  tmp2 = bnmul(y, y);
  n_z2 = bnadd(tmp1, tmp2);
  bnfree(tmp1);
  bnfree(tmp2);

  /* test if numerator of |z|^2 is a square */
  is_sq = bntrysqrt(n_z2, &r_num);
  bnfree(n_z2);
  if (!is_sq) {
    bnfree(x); bnfree(y); bnfree(g_den);
    bnfree(rd_prime); bnfree(id_prime);
    return 0;
  }

  /* lcm_den = g_den * rd' * id' = lcm(rd, id) */
  tmp1 = bnmul(rd_prime, id_prime);
  lcm_den = bnmul(g_den, tmp1);
  bnfree(tmp1);
  bnfree(g_den);
  bnfree(rd_prime);
  bnfree(id_prime);

  /* a_num = r_num + x, a_den = 2 * lcm_den */
  a_num = bnadd(r_num, x);
  a_den = bnashll(lcm_den, 1);

  bnfree(r_num);
  bnfree(x); bnfree(y);
  bnfree(lcm_den);

  /* test if a is a rational square */
  is_sq = bnrtrysqrt(a_num, a_den, &local_an, &local_ad);
  bnfree(a_num);
  bnfree(a_den);

  if (!is_sq) return 0;

  /* compute b = y / (2 * a) */
  if (out_bn || out_bd) {
    bignum_t *nb_raw, *db_raw, *db_temp, *g;
    bignum_t *local_bn, *local_bd;

    nb_raw  = bnmul(in, local_ad);
    db_temp = bnmul(id, local_an);
    db_raw  = bnashll(db_temp, 1);
    bnfree(db_temp);

    g = bngcd(nb_raw, db_raw);
    local_bn = bndiv(nb_raw, g);
    local_bd = bndiv(db_raw, g);

    bnfree(nb_raw);
    bnfree(db_raw);
    bnfree(g);

    if (out_bn) *out_bn = local_bn; else bnfree(local_bn);
    if (out_bd) *out_bd = local_bd; else bnfree(local_bd);
  }

  if (out_an) *out_an = local_an; else bnfree(local_an);
  if (out_ad) *out_ad = local_ad; else bnfree(local_ad);

  return 1;
}

/* [esl] end of sqrt bundles */


#define BITMASK64(n) ((n) >= 64 ? ~(uint64_t)0 : (((uint64_t)1 << (n)) - 1))

/* log(|b|); returns -inf on b==0 */
double bnlogtod(const bignum_t *b)
{
  size_t i, s, s_limb, s_bit, w, b0;
  uint64_t top_bits;
  assert(b != NULL);

  w = bnwidthu(b);
  if (w == 0) return -HUGE_VAL;
  /* if (b->isneg) return NAN; */

  /* Small enough to convert directly */
  if (w <= DBL_MANT_DIG) {
    uint64_t val = 0;
    for (i = b->size; i > 0; i--) {
      val = (val << LIMB_BITS) | (uint64_t)b->limb[i - 1];
    }
    return log((double)val);
  }

  /* Extract top DBL_MANT_DIG bits */
  s      = w - DBL_MANT_DIG;
  s_limb = s / LIMB_BITS;
  s_bit  = s % LIMB_BITS;

  top_bits = 0;
  b0 = LIMB_BITS - s_bit; /* Bits available in starting limb */

  if (b0 >= DBL_MANT_DIG) {
    /* All 53 bits reside within s_limb */
    top_bits = ((uint64_t)b->limb[s_limb] >> s_bit) & BITMASK64(DBL_MANT_DIG);
  } else {
    /* Take remaining b0 bits from s_limb */
    uint64_t part0 = (uint64_t)b->limb[s_limb] >> s_bit;
    size_t rem1 = DBL_MANT_DIG - b0;

    if (rem1 <= LIMB_BITS) {
      /* Spans 2 limbs: take LEAST significant rem1 bits from s_limb + 1 */
      uint64_t part1 = (uint64_t)b->limb[s_limb + 1] & BITMASK64(rem1);
      top_bits = part0 | (part1 << b0);
    } else {
      /* Spans 3 limbs (e.g. 32-bit limbs with 64-bit mantissa) */
      uint64_t part1 = (uint64_t)b->limb[s_limb + 1];
      size_t rem2 = rem1 - LIMB_BITS;
      uint64_t part2 = (uint64_t)b->limb[s_limb + 2] & BITMASK64(rem2);
      top_bits = part0 | (part1 << b0) | (part2 << (b0 + LIMB_BITS));
    }
  }

  return log((double)top_bits) + ((double)s * log(2.0));
}

/* log(|n|/d); returns -inf if n==0 */
double bnrlogtod(const bignum_t *n, const bignum_t *d)
{
  long long diff_w;

  assert(!d->isneg);
  if (BNZERO(n)) return -HUGE_VAL;
  if (BNZERO(d)) return  HUGE_VAL;

  /* ratio close to 1? */
  diff_w = (long long)bnwidthu(n) - (long long)bnwidthu(d);
  if (diff_w >= -1 && diff_w <= 1) { 
    int cmp = bncmpabs(n, d);
    if (cmp == 0) return 0.0;
    else {
      bignum_t *delta;
      double r, ans;
      if (cmp > 0) {
        /* |n| > d */
        if (!n->isneg) delta = bnsub(n, d); /* n - d > 0 */
        else delta = bnadd(n, d); /* n + d = d - |n| < 0 */
        r = fabs(bnrtod(delta, d)); /* (|n| - d) / d */
        ans = log1p(r);
      } else { /* |n| < d */
        if (!n->isneg) delta = bnsub(d, n); /* d - n > 0 */
        else delta = bnadd(n, d); /* n + d = d - |n| > 0 */
        r = fabs(bnrtod(delta, d)); /* (d - |n|) / d */
        ans = log1p(-r);
      }
      bnfree(delta);
      return ans;
    }
  }

  /* ratio fits in double range (|diff_w| <= 900)? */
  if (diff_w >= -900 && diff_w <= 900) {
    double r = fabs(bnrtod(n, d));
    if (r > 0.0 && r < HUGE_VAL) return log(r);
  }

  /* extreme scales (|diff_w| > 900) */
  return bnlogtod(n) - bnlogtod(d);
}

#ifndef COMPACT_RATTRIG
/* hex string for 2*pi (integer part '6', followed by fract nibbles)
   python3 -c "from mpmath import mp, pi; mp.dps=1000; x=2*pi; n=512; 
   h=format(int(mp.floor(x*16**n)), 'X'); print(f'{h[0]}.{h[1:]}')" */
static const char *hex_2pi_str = "6"
  "487ED5110B4611A62633145C06E0E68948127044533E63A0105DF531D89CD912"
  "8A5043CC71A026EF7CA8CD9E69D218D98158536F92F8A1BA7F09AB6B6A8E122F"
  "242DABB312F3F637A262174D31BF6B585FFAE5B7A035BF6F71C35FDAD44CFD2D"
  "74F9208BE258FF324943328F6722D9EE1003E5C50B1DF82CC6D241B0E2AE9CD3"
  "48B1FD47E9267AFC1B2AE91EE51D6CB0E3179AB1042A95DCF6A9483B84B4B36B"
  "3861AA7255E4C0278BA3604650C10BE19482F23171B671DF1CF3B960C074301C"
  "D93C1D17603D147DAE2AEF837A62964EF15E5FB4AAC0B8C1CCAA4BE754AB5728"
  "AE9130C4C7D02880AB9472D45556216D6998B8682283D19D42A90D5EF8E5D327";

static bignum_t *g_2pi_max = NULL;
static size_t g_max_frac_bits = 0;
static long bnx_init_2pi(void)
{
  if (!g_2pi_max) {
    size_t len;
    g_2pi_max = strtobn(hex_2pi_str, NULL, 16);
    len = strlen(hex_2pi_str);
    g_max_frac_bits = (len > 1) ? (len - 1) * 4 : 0;
  }
  return (long)g_max_frac_bits;
}

/* Reduce rational n/d modulo 2*pi, returning *pnum, *pden */
void bnrmod2pi(const bignum_t *n, const bignum_t *d, bignum_t **pnum, bignum_t **pden)
{
  size_t w_n = bnwidthu(n), w_d = bnwidthu(d);
  size_t w = (w_n > w_d) ? (w_n - w_d) : 0;
  long m = bnx_init_2pi();
  bignum_t *nq = bnashll(n, m);
  bignum_t *dp = bnmul(d, g_2pi_max);
  bignum_t *r_den = bnashll(d, m);
  bignum_t *r_num, *k_bn = bndmod(&r_num, nq, dp);

  if (r_num->isneg) {
    bignum_t *adj_num = bnadd(r_num, dp);
    bnfree(r_num);
    r_num = adj_num;
  }

  *pnum = r_num; *pden = r_den;
  bnfree(nq); bnfree(dp); bnfree(k_bn);
}

/* compute correlated sin and cos of rational n/d */
void bnrsincostod(const bignum_t *n, const bignum_t *d, double *psin, double *pcos)
{
  bignum_t *num, *den;
  double x;

  bnrmod2pi(n, d, &num, &den);
  x = bnrtod(num, den);
#if defined(_gnu_source)
  /* gnu extension: sincos(x, &s, &c) */
  sincos(x, psin, pcos);
#else
  /* portable fallback */
  *psin = sin(x);
  *pcos = cos(x);
#endif

  bnfree(num); bnfree(den);
}
#endif

#define ATAN2_TRUNC_BITS (DBL_MANT_DIG + 7) /* guard bits for lost precision */
#define ATAN2_DOMINANCE  (DBL_MANT_DIG + 1) /* underflow */

double bnratan2tod(const bignum_t *ny, const bignum_t *dy, const bignum_t *nx, const bignum_t *dx)
{
  int y_is_neg, x_is_neg;
  size_t wny, wdy, wnx, wdx, sy, sx;
  size_t sh_ny, sh_dy, sh_nx, sh_dx;
  bignum_t *ny_t, *dy_t, *nx_t, *dx_t;
  bignum_t *n_ratio, *d_ratio;
  double r_trunc, r, y_val, x_val;
  int extra;

  if (BNZERO(dy) || BNZERO(dx)) return HUGE_VAL-HUGE_VAL;  /* NaN */

  /* signs of y = ny/dy, x = nx/dx */
  y_is_neg = (ny->isneg != dy->isneg);
  x_is_neg = (nx->isneg != dx->isneg);

  if (BNZERO(ny)) {
    if (x_is_neg) return y_is_neg ? -M_PI : M_PI;
    else return y_is_neg ? -0.0 : 0.0;
  } else if (BNZERO(nx)) {
    return y_is_neg ? -M_PI_2 : M_PI_2;
  }

  /* approximate log2 magnitudes */
  wny = bnwidthu(ny); wdy = bnwidthu(dy);
  wnx = bnwidthu(nx); wdx = bnwidthu(dx);
  sy = wny + wdx; /* approx log2(|ny * dx|) */
  sx = wdy + wnx; /* approx log2(|dy * nx|) */

  /* fast path: |y| >> |x| (angle approaches +/- pi/2) */
  if (sy > sx + ATAN2_DOMINANCE) {
    return y_is_neg ? -M_PI_2 : M_PI_2;
  }
  /* fast path: |x| >> |y|, x < 0 (angle approaches +/- pi) */
  if (sx > sy + ATAN2_DOMINANCE && x_is_neg) {
    return y_is_neg ? -M_PI : M_PI;
  }
  /* fast path: |x| >> |y|, x > 0 (angle approaches +/- 0) */
  if (sx > sy + ATAN2_DOMINANCE && !x_is_neg) {
    return y_is_neg ? -0.0 : 0.0;
  }

  /* comparable magnitudes: shift right via negative shift count */
  sh_ny = (wny > ATAN2_TRUNC_BITS) ? (wny - ATAN2_TRUNC_BITS) : 0;
  sh_dy = (wdy > ATAN2_TRUNC_BITS) ? (wdy - ATAN2_TRUNC_BITS) : 0;
  sh_nx = (wnx > ATAN2_TRUNC_BITS) ? (wnx - ATAN2_TRUNC_BITS) : 0;
  sh_dx = (wdx > ATAN2_TRUNC_BITS) ? (wdx - ATAN2_TRUNC_BITS) : 0;
  ny_t = sh_ny ? bnashll(ny, -(long)sh_ny) : bndup(ny);
  dy_t = sh_dy ? bnashll(dy, -(long)sh_dy) : bndup(dy);
  nx_t = sh_nx ? bnashll(nx, -(long)sh_nx) : bndup(nx);
  dx_t = sh_dx ? bnashll(dx, -(long)sh_dx) : bndup(dx);

  /* cross-multiply and free temps */
  n_ratio = bnmul(ny_t, dx_t);
  d_ratio = bnmul(dy_t, nx_t);
  bnfree(ny_t); bnfree(dy_t); bnfree(nx_t); bnfree(dx_t);

  /* convert ratio to abs double and free */
  r_trunc = fabs(bnrtod(n_ratio, d_ratio));
  bnfree(n_ratio); bnfree(d_ratio);

  /* compensate for truncation scale mismatch */
  extra = (int)(sh_ny + sh_dx) - (int)(sh_dy + sh_nx);
  r = ldexp(r_trunc, extra);

  /* apply signs and compute final angle */
  y_val = y_is_neg ? -r : r;
  x_val = x_is_neg ? -1.0 : 1.0;

  return atan2(y_val, x_val);
}

/* [esl+] complex arctangent: (rd + i*id) = atan((xn/xd) + i*(yn/yd)) */
void bncatantodd(double *prd, double *pid, const bignum_t *xn, const bignum_t *xd, const bignum_t *yn, const bignum_t *yd)
{
  bignum_t *x, *y, *d_xy, *d_xy2, *x2, *y2;
  bignum_t *tmp1, *n_2x, *tmp2, *n_1r2;
  bignum_t *sum_y, *diff_y, *sq_sum_y, *sq_diff_y, *n_v, *d_v;

  assert(!xd->isneg && !yd->isneg);

  /* compute basic cross-product terms */
  x = bnmul(xn, yd); y = bnmul(yn, xd);
  d_xy = bnmul(xd, yd);
  x2 = bnmul(x, x); y2 = bnmul(y, y);
  d_xy2 = bnmul(d_xy, d_xy);

  /* real part terms */
  tmp1 = bnmul(x, d_xy);
  n_2x = bnashll(tmp1, 1);
  bnfree(tmp1);
  tmp2 = bnadd(x2, y2);
  n_1r2 = bnsub(d_xy2, tmp2);
  bnfree(tmp2);

  *prd = 0.5 * bnratan2tod(n_2x, d_xy2, n_1r2, d_xy2);

  /* imaginary part terms */
  sum_y = bnadd(yd, yn);
  diff_y = bnsub(yd, yn);
  tmp1 = bnmul(xd, sum_y);
  sq_sum_y = bnmul(tmp1, tmp1);
  bnfree(tmp1);
  tmp2 = bnmul(xd, diff_y);
  sq_diff_y = bnmul(tmp2, tmp2);
  bnfree(tmp2);
  n_v = bnadd(x2, sq_sum_y);
  d_v = bnadd(x2, sq_diff_y);

  *pid = 0.25 * bnrlogtod(n_v, d_v);

  /* cleanup */
  bnfree(x); bnfree(y);
  bnfree(d_xy); bnfree(d_xy2);
  bnfree(x2); bnfree(y2);
  bnfree(n_2x); bnfree(n_1r2);
  bnfree(sum_y); bnfree(diff_y);
  bnfree(sq_sum_y); bnfree(sq_diff_y);
  bnfree(n_v); bnfree(d_v);
}

/* [esl+] fast(er) comparison of two normalized rationals n1/d1 n2/d2; returns -1/0/1 */
int bnrcmp(const bignum_t *n1, const bignum_t *d1, const bignum_t *n2, const bignum_t *d2)
{
  int z1, z2, isneg1, isneg2;
  size_t c1, c2, s1, s2;
  bignum_t *p1, *p2;
  int res;

  /* 0s amd signs */
  z1 = BNZERO(n1), z2 = BNZERO(n2);
  if (z1 && z2) return 0;
  isneg1 = n1->isneg, isneg2 = n2->isneg;
  if (z1) return isneg2 ? 1 : -1;
  if (z2) return isneg1 ? -1 : 1;
  if (isneg1 != isneg2) return isneg1 ? -1 : 1;

  /* limb count comparison; gap of 2 is large enough */
  c1 = n1->size + d2->size; c2 = d1->size + n2->size;
  if (c1 >= c2 + 2) return isneg1 ? -1 : 1;
  if (c2 >= c1 + 2) return isneg1 ? 1 : -1;

  /* bit width comparison; gap of 2 is large enough */
  s1 = bnwidthu(n1) + bnwidthu(d2);
  s2 = bnwidthu(d1) + bnwidthu(n2);
  if (s1 >= s2 + 2) return isneg1 ? -1 : 1;
  if (s2 >= s1 + 2) return isneg1 ? 1 : -1;

  /* cross-product fallnack; signs are the same */
  p1 = bnmul(n1, d2); p2 = bnmul(d1, n2);
  res = bncmpabs(p1, p2);
  bnfree(p1); bnfree(p2);
  
  return isneg1 ? -res : res;
}

/* [esl+] fast(er) comparison of n1/d1 and a non-NaN double */
int bnrdcmp(const bignum_t *n1, const bignum_t *d1, double d2)
{
  int isneg1, exp_d2, cmp;
  long long e1, e_d2, delta_l;
  bignum_t *p1, *p2, *tmp;
  bignumll_t bn_m;
  double m;

  assert(d2 == d2); /* no NaNs! */
  assert(DBL_MANT_DIG < 64);

  /* 0s amd signs */
  if (d2 >= HUGE_VAL)  return -1;
  if (d2 <= -HUGE_VAL) return  1;
  if (BNZERO(n1)) return (d2 == 0.0) ? 0 : (d2 > 0.0) ? -1 : 1;
  if (d2 == 0.0) return n1->isneg ? -1 : 1;
  isneg1 = n1->isneg;
  if (isneg1 != (d2 < 0.0)) return isneg1 ? -1 : 1;

  /* extract mantissa and exponent */
  m = frexp(fabs(d2), &exp_d2);

  /* limb count comparison */
  delta_l = (long long)n1->size - (long long)d1->size;
  if ((delta_l - 1) * LIMB_BITS >= (long long)exp_d2)
      return isneg1 ? -1 : 1;
  if ((long long)exp_d2 >= (delta_l + 1) * LIMB_BITS + 1)
      return isneg1 ? 1 : -1;

  /* bit width comparison */
  e1 = (long long)bnwidthu(n1) - (long long)bnwidthu(d1);
  if (e1 >= (long long)exp_d2 + 1) return isneg1 ? -1 : 1;
  if ((long long)exp_d2 >= e1 + 2) return isneg1 ? 1 : -1;

  /* cross-product fallback */
  e_d2 = (long long)exp_d2 - DBL_MANT_DIG;
  bn_m = bnll((int64_t)ldexp(m, DBL_MANT_DIG));
  p2 = bnmul(d1, (const bignum_t *)&bn_m);
  if (e_d2 > 0) {
    tmp = bnashll(p2, e_d2);
    bnfree(p2);
    p2 = tmp;
  }
  p1 = (e_d2 < 0) ? bnashll(n1, -e_d2) : bndup(n1);
  cmp = bncmpabs(p1, p2);
  bnfree(p1);
  bnfree(p2);

  return isneg1 ? -cmp : cmp;
}


bignumll_t bnll(int64_t v)
{
  bignumll_t b; bnx_makell(&b, v);
  return b;
}

/* bitwise-not: n -> -(n+1) = -1-n */
bignum_t *bnbitnot(const bignum_t *num)
{
  bignumll_t bl = bnll(-1);
  CHECKSIGN(num);
  return bnsub((bignum_t *)&bl, num);
}

/* 2-complement representation for the next limb [i] */
static limb_t bnx_2cget(const bignum_t *n, size_t i, limb_t *borrow)
{
  limb_t m = (i < n->size) ? n->limb[i] : (limb_t)0;
  if (!n->isneg) return m; /* positive: identity */
  /* negative: two's complement = ~m + borrow-in */
  m = ~m; m += *borrow;
  *borrow = (m < *borrow) ? 1 : 0; /* carry out */
  return m;
}

/* convert a two's-complement result limb back to sign+magnitude */
static limb_t bnx_2cput(limb_t v, int rneg, limb_t *carry)
{
  if (!rneg) return v;
  /* magnitude = ~v + *carry  (because -x = ~x+1, so |x| = ~tc+1) */
  v = ~v; v += *carry;
  *carry = (v < *carry) ? 1 : 0;
  return v;
}

static int bnx_andsign(int an, int bn) { return  an & bn; }
static int bnx_iorsign(int an, int bn) { return  an | bn; }
static int bnx_xorsign(int an, int bn) { return  an ^ bn; }

typedef limb_t (*limb_op_t)(limb_t a, limb_t b);
static limb_t bnx_andop(limb_t a, limb_t b) { return a & b; }
static limb_t bnx_iorop(limb_t a, limb_t b) { return a | b; }
static limb_t bnx_xorop(limb_t a, limb_t b) { return a ^ b; }

/* note: outn is a 'normal' size of the output; real output can be 1 longer */
static size_t bnx_bitwise(bignum_t *r, const bignum_t *a, const bignum_t *b, limb_op_t op, int rneg, size_t n)
{
  limb_t ba = a->isneg ? 1 : 0; /* borrow for a's 2c conversion */
  limb_t bb = b->isneg ? 1 : 0; /* borrow for b's 2c conversion */
  limb_t bc = rneg ? 1 : 0;     /* carry for result's 2c inversion */
  size_t i, last_nz = 0;

  for (i = 0; i < n; i++) {
    limb_t la = bnx_2cget(a, i, &ba);
    limb_t lb = bnx_2cget(b, i, &bb);
    limb_t lv = op(la, lb);
    r->limb[i] = bnx_2cput(lv, rneg, &bc);
    if (r->limb[i] != 0) last_nz = i;
  }

  /* if result is negative and bc still has a borrow, we need an extra limb */
  if (rneg && bc) {
    r->limb[n] = bc; /* bc == 1 here means we produced 2^(32n) */
    last_nz = n;
  }

  /* trim leading zeros */
  while (last_nz > 0 && r->limb[last_nz] == 0) last_nz--;
  if (r->limb[last_nz] == 0) return 0; /* result is zero */
  return last_nz + 1;
}

/* pre-compute the result size (allocate 1 more for overflow limb) */
static size_t bnx_out2csz(limb_op_t op, const bignum_t *a, const bignum_t *b)
{
  size_t na = a->size, nb = b->size;
  if (op == bnx_andop) {
    if (!a->isneg && na <= nb) return na;
    if (!b->isneg && nb <= na) return nb;
  } else if (op == bnx_iorop) {
    if (a->isneg && na <= nb) return na;
    if (b->isneg && nb <= na) return nb;
  }
  return na > nb ? na : nb;
}

bignum_t *bnbitand(const bignum_t *a, const bignum_t *b)
{
  bignum_t *r;
  size_t outsz = bnx_out2csz(bnx_andop, a, b);
  int rn = bnx_andsign(a->isneg, b->isneg);
  size_t actual;

  if (BNZERO(a) || BNZERO(b)) return bn0;

  NEWBN(r, outsz+1, "bnbitand");
  r->isneg = rn;
  ZERO_LIMBS(r->limb, outsz+1);

  actual = bnx_bitwise(r, a, b, bnx_andop, rn, outsz);
  assert(actual <= outsz+1);
  RESIZE(r, actual); r->size = actual;
  return_NORMALIZE(r, "bnbitand");
}

bignum_t *bnbitior(const bignum_t *a, const bignum_t *b)
{
  bignum_t *r;
  size_t outsz = bnx_out2csz(bnx_iorop, a, b);
  int rn = bnx_iorsign(a->isneg, b->isneg);
  size_t actual;

  if (BNZERO(a)) return bndup(b);
  if (BNZERO(b)) return bndup(a);

  NEWBN(r, outsz+1, "bnbitior");
  r->isneg = rn;
  ZERO_LIMBS(r->limb, outsz+1);

  actual = bnx_bitwise(r, a, b, bnx_iorop, rn, outsz);
  assert(actual <= outsz+1);
  RESIZE(r, actual); r->size = actual;
  return_NORMALIZE(r, "bnbitior");
}

bignum_t *bnbitxor(const bignum_t *a, const bignum_t *b)
{
  bignum_t *r;
  size_t outsz = bnx_out2csz(bnx_xorop, a, b);
  int rn = bnx_xorsign(a->isneg, b->isneg);
  size_t actual;

  if (BNZERO(a)) return bndup(b);
  if (BNZERO(b)) return bndup(a);

  NEWBN(r, outsz+1, "bnbitxor");
  r->isneg = rn;
  ZERO_LIMBS(r->limb, outsz+1);

  actual = bnx_bitwise(r, a, b, bnx_xorop, rn, outsz);
  RESIZE(r, actual); r->size = actual;
  assert(actual <= outsz+1);
  return_NORMALIZE(r, "bnbitxor");
}

bignum_t *bnbitash(const bignum_t *a, long cnt)
{
  /* left shift or any shift of a positive */
  if (cnt >= 0 || !a->isneg) {
    /* same as bnashll */
    return bnashll(a, (int64_t)cnt);
  } else { /* right shift of a negative */
    bignum_t *r; long acnt = -cnt;
    size_t wds = (size_t)(acnt / LIMB_BITS);
    int bts = (int)(acnt % LIMB_BITS);

    /* calc 'sticky bit' */ 
    int sticky = 0; size_t i;
    for (i = 0; i < wds && i < a->size; ++i)
      if (a->limb[i]) { sticky = 1; break; }
    if (!sticky && bts > 0 && wds < a->size)
      if (a->limb[wds] & (((limb_t)1 << bts) - 1)) sticky = 1;

    r = bnashll(a, (int64_t)cnt);
    if (sticky) { bignum_t *t = bnaddll(r, -1); bnfree(r); r = t; }

    return r;
  }
}

long bnintlen(const bignum_t *n)
{
  /* SRFI-142 integer-length = bnwidths(n) - 1 (excludes sign bit) */
  size_t w = bnwidths(n);
  return (w == 0) ? 0 : (long)(w - 1);
}

long bnbitc(const bignum_t *n)
{
  /* count 0-bits in negative, 1-bits otherwise */
  long cnt = 0; size_t i;

  if (BNZERO(n)) return 0;

  if (!n->isneg) {
    for (i = 0; i < n->size; i++) {
      limb_t v = n->limb[i];
      while (v) { ++cnt; v &= v-1; } /* Kernighan popcount */
    }
  } else {
    limb_t borrow = 1;
    for (i = 0; i < n->size; i++) {
      limb_t v = n->limb[i] - borrow;
      borrow = (v > n->limb[i]) ? 1 : 0;
      while (v) { ++cnt; v &= v-1; } /* Kernighan popcount */
    }
    /* borrow should be 0 since n != 0 */
  }

  return cnt;
}



/* fatnum implementation (esl) */

/* fatnums contain up to 4 parts, depending on type:
 * fixnum, flonum, bignum: nump_t[1]
 * ratnum: nump_t[2]
 * rectnum: nump_t[4]  (pair of ratnums or simple integers)
 * compnum: nump_t[4]  (pair of flonums in slots [0] and [2])
 * numerator and real parts precede denomonator/imaginary parts
 * parts of complex numbers always take two slots each */ 

/* reporting errors */
#define setfail(e) (errno = (e), NUMT_NONE)

/* working with elementary numbers */
#define isfix(xt)     ((xt) == NUMT_FIX)
#define getfix(xp)    ((xp)->fix)
#define setfix(pz, z) ((pz)->fix = (z), NUMT_FIX)
#define isbig(xt)     ((xt) == NUMT_BIG)
#define getbig(xp)    ((xp)->big)
#define setbig(pz, z) ((pz)->big = (z), NUMT_BIG) 
#define isflo(nt)     ((nt) == NUMT_FLO)
#define getflo(xp)    ((xp)->flo)
#define setflo(pz, z) ((pz)->flo = (z), NUMT_FLO)

nump_t numfix_0 = { 0 }, numfix_1 = { 1 };

/* free memory taken by a number */
static void numfini(numt_t xt, nump_t *xp)
{
  /* only bignums need freeing */
  if ((xt & NUMT_SS_MASK) == NUMT_BIG) bnfree(xp->big); 
  xt >>= 2; if (!xt) return; else ++xp;
  if ((xt & NUMT_SS_MASK) == NUMT_BIG) bnfree(xp->big);
  xt >>= 2; if (!xt) return; else ++xp;
  if ((xt & NUMT_SS_MASK) == NUMT_BIG) bnfree(xp->big);
  xt >>= 2; if (!xt) return; else ++xp;
  if ((xt & NUMT_SS_MASK) == NUMT_BIG) bnfree(xp->big);
}

/* calling conventions:
 * number type identifies what slots of a number are in use
 * input numbers are normalized and owned by the caller; there can be type restrictions
 * output number is not intitialized initially, will be fresh and normalized on return,
 * so the caller later should either take ownership of it or numfini() it */

/* duplicate a number and return a fresh copy owhed by the caller */
static numt_t numdup(nump_t *yp, numt_t xt, const nump_t *xp)
{
  numt_t yt = xt;
  assert(xt && "NONE number");
  /* only bignums need to ne duplicated */
  if ((xt & NUMT_SS_MASK) == NUMT_BIG) yp->big = bndup(xp->big); else *yp = *xp;
  xt >>= 2; if (!xt) return yt; else ++xp, ++yp;
  if ((xt & NUMT_SS_MASK) == NUMT_BIG) yp->big = bndup(xp->big); else *yp = *xp;
  xt >>= 2; if (!xt) return yt; else ++xp, ++yp;
  if ((xt & NUMT_SS_MASK) == NUMT_BIG) yp->big = bndup(xp->big); else *yp = *xp;
  xt >>= 2; if (!xt) return yt; else ++xp, ++yp;
  if ((xt & NUMT_SS_MASK) == NUMT_BIG) yp->big = bndup(xp->big); else *yp = *xp;
  return yt;
}

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4789)
#endif

/* move number in memory (ownership goes with it) */
static numt_t nummove(nump_t *yp, numt_t xt, const nump_t *xp)
{
  numt_t yt = xt;
  assert(xt && "NONE number");
  *yp = *xp; xt >>= 2; if (!xt) return yt; else ++xp, ++yp;
  *yp = *xp; xt >>= 2; if (!xt) return yt; else ++xp, ++yp;
  *yp = *xp; xt >>= 2; if (!xt) return yt; else ++xp, ++yp;
  *yp = *xp;
  return yt;
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

/* generic integer arithmetics */

/* NB: in normal form, only integers out of fixnum range are represented as bignums */

#define NUMT_IS_INTNUM(nt) ((nt) == NUMT_FIX || (nt) == NUMT_BIG)

/* x <=> y, returns -1/0/1 */
int intcmp(numt_t xt, const nump_t *xp, numt_t yt, const nump_t *yp)
{
  assert(NUMT_IS_INTNUM(xt) && "non-integer number");
  assert(NUMT_IS_INTNUM(yt) && "non-integer number");
  if (isfix(xt) && isfix(yt)) {
    long x = getfix(xp), y = getfix(yp);
    return (x < y) ? -1 : (x > y);
  } else if (isfix(xt)) { /* yt is big */
    return -bnsign(getbig(yp)); /* y is out of fixnum range */
  } else if (isfix(yt)) { /* xt is big */
    return bnsign(getbig(xp)); /* x is out of fixnum range */
  } else { /* both big */
    return bncmp(getbig(xp), getbig(yp));
  }
}

/* x == y */
int inteq(numt_t xt, const nump_t *xp, numt_t yt, const nump_t *yp)
{
  assert(NUMT_IS_INTNUM(xt) && "non-integer number");
  assert(NUMT_IS_INTNUM(yt) && "non-integer number");
  if (isfix(xt) && isfix(yt)) {
    return getfix(xp) == getfix(yp);
  } else if (isbig(xt) && isbig(yt)) {
    return bneq(getbig(xp), getbig(yp));
  } else { /* fixnums and bignums are disjoint */
    return 0;
  }
}

/* x < y */
#define intless(xt, xp, yt, yp) (intcmp(xt, xp, yt, yp) < 0)

/* odd(x) */
static int intodd(numt_t xt, const nump_t *xp)
{
  assert(NUMT_IS_INTNUM(xt) && "non-integer number");
  if (isfix(xt)) return (getfix(xp) & 1) != 0;
  else return bnodd(getbig(xp));
}

/* z = -x */
static numt_t intneg(nump_t *zp, numt_t xt, const nump_t *xp)
{
  numt_t zt = NUMT_NONE;
  assert(NUMT_IS_INTNUM(xt) && "non-integer number");
  if (isfix(xt)) {
    long z = -getfix(xp); /* NB: -FIXNUM_MIN > FIXNUM_MAX */
    if (z <= FIXNUM_MAX) zt = setfix(zp, z);
    else zt = setbig(zp, lltobn(z));
  } else if (bneql(getbig(xp), FIXNUM_MAX+1)) {
    zt = setfix(zp, FIXNUM_MIN);
  } else {
    zt = setbig(zp, bnneg(getbig(xp)));
  }
  return zt;
}

/* z = |x| */
static numt_t intabs(nump_t *zp, numt_t xt, const nump_t *xp)
{
  numt_t zt = NUMT_NONE;
  assert(NUMT_IS_INTNUM(xt) && "non-integer number");
  if (isfix(xt)) {
    long z = labs(getfix(xp)); /* NB: labs(FIXNUM_MIN) > FIXNUM_MAX */
    if (z <= FIXNUM_MAX) zt = setfix(zp, z);
    else zt = setbig(zp, lltobn(z));
  } else {
    zt = setbig(zp, bnabs(getbig(xp)));
  }
  return zt;
}

/* sign(x) (-1 0 +1) */
static int intsign(numt_t xt, const nump_t *xp)
{
  assert(NUMT_IS_INTNUM(xt) && "non-integer number");
  if (isfix(xt)) return getfix(xp) ? (getfix(xp) < 0 ? -1 : 1) : 0;
  else return bnsign(getbig(xp));
}

/* z = x + y */
static numt_t intadd(nump_t *zp, numt_t xt, const nump_t *xp, numt_t yt, const nump_t *yp)
{
  numt_t zt = NUMT_NONE;
  bignum_t *bz;
  assert(NUMT_IS_INTNUM(xt) && "non-integer number");
  assert(NUMT_IS_INTNUM(yt) && "non-integer number");
  if (isfix(xt)) {
    if (isfix(yt)) {
      long z = getfix(xp) + getfix(yp);
      if (z >= FIXNUM_MIN && z <= FIXNUM_MAX) zt = setfix(zp, z);
      else zt = setbig(zp, lltobn(z));
      return zt;
    } else {
      bz = bnaddll(getbig(yp), getfix(xp));
    }
  } else {
    if (isfix(yt)) bz = bnaddll(getbig(xp), getfix(yp));
    else bz = bnadd(getbig(xp), getbig(yp));
  }
  if (bnwidths(bz) > FIXNUM_WIDTH) zt = setbig(zp, bz);
  else (zt = setfix(zp, bntol(bz))), bnfree(bz);
  return zt;
}

/* z = x - y */
static numt_t intsub(nump_t *zp, numt_t xt, const nump_t *xp, numt_t yt, const nump_t *yp)
{
  numt_t zt = NUMT_NONE;
  bignum_t *bz;
  assert(NUMT_IS_INTNUM(xt) && "non-integer number");
  assert(NUMT_IS_INTNUM(yt) && "non-integer number");
  if (isfix(xt)) {
    if (isfix(yt)) {
      long z = getfix(xp) - getfix(yp);
      if (z >= FIXNUM_MIN && z <= FIXNUM_MAX) zt = setfix(zp, z);
      else zt = setbig(zp, lltobn(z));
      return zt;
    } else {
      bignum_t *bt = bnaddll(getbig(yp), -getfix(xp));  /* no bnsubl */
      bz = bnneg(bt); /* no in-place sign change */
      bnfree(bt);
    }
  } else {
    if (isfix(yt)) bz = bnaddll(getbig(xp), -getfix(yp));
    else bz = bnsub(getbig(xp), getbig(yp));
  }
  if (bnwidths(bz) > FIXNUM_WIDTH) zt = setbig(zp, bz);
  else (zt = setfix(zp, bntol(bz))), bnfree(bz);
  return zt;
}

/* z = x * y */
static numt_t intmul(nump_t *zp, numt_t xt, const nump_t *xp, numt_t yt, const nump_t *yp)
{
  numt_t zt = NUMT_NONE;
  bignum_t *bz;
  assert(NUMT_IS_INTNUM(xt) && "non-integer number");
  assert(NUMT_IS_INTNUM(yt) && "non-integer number");
  if (isfix(xt)) {
    if (isfix(yt)) {
      int64_t z = (int64_t)getfix(xp) * (int64_t)getfix(yp);
      if (z >= FIXNUM_MIN && z <= FIXNUM_MAX) zt = setfix(zp, (long)z);
      else zt = setbig(zp, lltobn(z));
      return zt;
    } else {
      bz = bnmulll(getbig(yp), getfix(xp));
    }
  } else {
    if (isfix(yt)) bz = bnmulll(getbig(xp), getfix(yp));
    else bz = bnmul(getbig(xp), getbig(yp));
  }
  if (bnwidths(bz) > FIXNUM_WIDTH) zt = setbig(zp, bz);
  else (zt = setfix(zp, bntol(bz))), bnfree(bz);
  return zt;
}

/* z = truncate(x/y) (truncate-quotient, a.k.a. quotient) */
static numt_t intquo(nump_t *zp, numt_t xt, const nump_t *xp, numt_t yt, const nump_t *yp)
{
  numt_t zt = NUMT_NONE;
  bignum_t *bz;
  assert(NUMT_IS_INTNUM(xt) && "non-integer number");
  assert(NUMT_IS_INTNUM(yt) && "non-integer number");
  if (isfix(xt)) {
    if (isfix(yt)) {
      if (!getfix(yp)) bnx_zdiv();
      else {
        long x = getfix(xp), y = getfix(yp), q = x / y;
        if (q <= FIXNUM_MAX) zt = setfix(zp, q);
        else zt = setbig(zp, lltobn(q)); /* FIXNUM_MIN/-1 > FIXNUM_MAX */
      }
    } else if (getfix(xp) == FIXNUM_MIN && bneql(getbig(yp), FIXNUM_MAX+1)) {
      zt = setfix(zp, -1);
    } else {
      zt = setfix(zp, 0);
    }
    return zt; 
  } else {
    if (isfix(yt)) bz = bndivl(getbig(xp), getfix(yp));
    else bz = bndiv(getbig(xp), getbig(yp));
  }
  if (bnwidths(bz) > FIXNUM_WIDTH) zt = setbig(zp, bz);
  else zt = (setfix(zp, bntol(bz))), bnfree(bz);
  return zt;
}

/* z = x-truncate(x/y) (truncate-remainder, a.k.a. remainder) */
static numt_t intrem(nump_t *zp, numt_t xt, const nump_t *xp, numt_t yt, const nump_t *yp)
{
  numt_t zt = NUMT_NONE;
  bignum_t *bz;
  assert(NUMT_IS_INTNUM(xt) && "non-integer number");
  assert(NUMT_IS_INTNUM(yt) && "non-integer number");
  if (isfix(xt)) {
    if (isfix(yt)) {
      if (!getfix(yp)) bnx_zdiv();
      else {
        long x = getfix(xp), y = getfix(yp), r = x % y;
        zt = setfix(zp, r);
      }
    } else if (getfix(xp) == FIXNUM_MIN && bneql(getbig(yp), FIXNUM_MAX+1)) {
      zt = setfix(zp, 0);
    } else {
      zt = setfix(zp, getfix(xp));
    }
    return zt;
  } else {
    if (isfix(yt)) {
      zt = setfix(zp, bnmodl(getbig(xp), getfix(yp)));
      return zt;
    } else {
      bz = bnmod(getbig(xp), getbig(yp));
    }
  }
  if (bnwidths(bz) > FIXNUM_WIDTH) zt = setbig(zp, bz);
  else (zt = setfix(zp, bntol(bz))), bnfree(bz);
  return zt;
}

#if 0 /* not used yet */
/* q = truncate(x/y), r = x-truncate(x/y) (truncate/, a.k.a. quotient&remainder) */
void intquorem(numt_t *pqt, nump_t *qp, numt_t *prt, nump_t *rp, numt_t xt, const nump_t *xp, numt_t yt, const nump_t *yp)
{
  assert(NUMT_IS_INTNUM(xt) && "non-integer number");
  assert(NUMT_IS_INTNUM(yt) && "non-integer number");
  if (isfix(xt)) {
    if (isfix(yt)) {
      if (!getfix(yp)) bnx_zdiv();
      else {
        long x = getfix(xp), y = getfix(yp), q = x / y, r = x % y;
        if ((r < 0 && y > 0) || (r > 0 && y < 0)) q -= 1, r += y;
        if (q >= FIXNUM_MIN && q <= FIXNUM_MAX) *pqt = setfix(qp, q);
        else *pqt = setbig(qp, lltobn(q)); /* FIXNUM_MIN/-1 > FIXNUM_MAX */
        assert(r >= FIXNUM_MIN && r <= FIXNUM_MAX);
        *prt = setfix(rp, r);
      }
    } else if (getfix(xp) == FIXNUM_MIN && bneql(getbig(yp), FIXNUM_MAX+1)) { 
      *pqt = setfix(qp, -1);
      *prt = setfix(rp, 0);
    } else {
      *pqt = setfix(qp, 0);
      *prt = setfix(rp, getfix(xp));
    }
  } else {
    if (isfix(yt)) {
      long r; 
      bignum_t *bq = bndmodl(&r, getbig(xp), getfix(yp));
      if (bnwidths(bq) > FIXNUM_WIDTH) *pqt = setbig(qp, bq);
      else (*pqt = setfix(qp, bntol(bq))), bnfree(bq);
      *prt = setfix(rp, r);
    } else {
      bignum_t *br, *bq = bndmod(&br, getbig(xp), getbig(yp));
      if (bnwidths(bq) > FIXNUM_WIDTH) *pqt = setbig(qp, bq);
      else (*pqt = setfix(qp, bntol(bq))), bnfree(bq);
      if (bnwidths(br) > FIXNUM_WIDTH) *prt = setbig(rp, br);
      else (*prt = setfix(rp, bntol(br))), bnfree(br);
    }
  }
}
#endif

/* z = floor(x/y)  (floor-quotient, pair for modulo) */
static numt_t intfquo(nump_t *zp, numt_t xt, const nump_t *xp, numt_t yt, const nump_t *yp)
{
  numt_t zt = NUMT_NONE;
  assert(NUMT_IS_INTNUM(xt) && "non-integer number");
  assert(NUMT_IS_INTNUM(yt) && "non-integer number");
  if (isfix(xt)) {
    if (isfix(yt)) {
      if (!getfix(yp)) bnx_zdiv();
      else {
        long x = getfix(xp), y = getfix(yp), q = x / y, r = x % y;
        if ((r < 0 && y > 0) || (r > 0 && y < 0)) q -= 1;
        if (q >= FIXNUM_MIN && q <= FIXNUM_MAX) zt = setfix(zp, q);
        else zt = setbig(zp, lltobn(q)); /* FIXNUM_MIN/-1 > FIXNUM_MAX */
      }
    } else if (getfix(xp) == FIXNUM_MIN && bneql(getbig(yp), FIXNUM_MAX+1)) {
      zt = setfix(zp, -1);
    } else {
      zt = setfix(zp, 0);
    }
    return zt; 
  } else {
    bignum_t *bq;
    if (isfix(yt)) {
      long y = getfix(yp), r; 
      bq = bndmodl(&r, getbig(xp), getfix(yp));
      if ((r < 0 && y > 0) || (r > 0 && y < 0)) {
        bignum_t *bt = bq; bq = bnsub(bt, bn1), bnfree(bt);
      }
    } else {
      bignum_t *bx = getbig(xp), *by = getbig(yp), *br;
      int sy = bnsign(by), sr;
      bq = bndmod(&br, bx, by); sr = bnsign(br);
      if ((sr < 0 && sy > 0) || (sr > 0 && sy < 0)) {
        bignum_t *bt = bq; bq = bnsub(bt, bn1), bnfree(bt);
      }      
    }
    if (bnwidths(bq) > FIXNUM_WIDTH) zt = setbig(zp, bq);
    else (zt = setfix(zp, bntol(bq))), bnfree(bq);
    return zt;
  }
}

/* z = x-floor(x/y)  (floor-remainder, a.k.a. modulo) */
static numt_t intfrem(nump_t *zp, numt_t xt, const nump_t *xp, numt_t yt, const nump_t *yp)
{
  numt_t zt = NUMT_NONE;
  assert(NUMT_IS_INTNUM(xt) && "non-integer number");
  assert(NUMT_IS_INTNUM(yt) && "non-integer number");
  if (isfix(xt)) {
    if (isfix(yt)) {
      if (!getfix(yp)) bnx_zdiv();
      else {
        long x = getfix(xp), y = getfix(yp), r = x % y;
        if ((r < 0 && y > 0) || (r > 0 && y < 0)) r += y;
        assert(r >= FIXNUM_MIN && r <= FIXNUM_MAX);
        zt = setfix(zp, r);
      }
    } else {
      zt = setfix(zp, 0);
    }
    return zt; 
  } else {
    int ysign, zsign;
    if (isfix(yt)) zt = setfix(zp, bnmodl(getbig(xp), getfix(yp)));
    else zt = setbig(zp, bnmod(getbig(xp), getbig(yp)));
    ysign = intsign(yt, yp), zsign = intsign(zt, zp);
    if (ysign != zsign) {
      numt_t tt; nump_t tp[1];
      tt = zt; tp[0] = zp[0];
      zt = intadd(zp, tt, tp, yt, yp);
      numfini(tt, tp);
    }
  }
  if (!isfix(zt) && bnwidths(getbig(zp)) <= FIXNUM_WIDTH) {
    bignum_t *bz = getbig(zp);
    zt = (setfix(zp, bntol(bz))), bnfree(bz);
  }
  return zt;
}

#if 0 /* not used yet */
/* q = floor(x/y), r = x-floor(x/y) (floor/, r a.k.a. modulo) */
void intfquorem(numt_t *pqt, nump_t *qp, numt_t *prt, nump_t *rp, numt_t xt, const nump_t *xp, numt_t yt, const nump_t *yp)
{
  assert(NUMT_IS_INTNUM(xt) && "non-integer number");
  assert(NUMT_IS_INTNUM(yt) && "non-integer number");
  if (isfix(xt)) {
    if (isfix(yt)) {
      if (!getfix(yp)) bnx_zdiv();
      else {
        long x = getfix(xp), y = getfix(yp), q = x / y, r = x % y;
        if ((r < 0 && y > 0) || (r > 0 && y < 0)) q -= 1, r += y;
        if (q >= FIXNUM_MIN && q <= FIXNUM_MAX) *pqt = setfix(qp, q);
        else *pqt = setbig(qp, lltobn(q)); /* FIXNUM_MIN/-1 > FIXNUM_MAX */
        assert(r >= FIXNUM_MIN && r <= FIXNUM_MAX);
        *prt = setfix(rp, r);
      }
    } else if (getfix(xp) == FIXNUM_MIN && bneql(getbig(yp), FIXNUM_MAX+1)) { 
      *pqt = setfix(qp, -1);
      *prt = setfix(rp, 0);
    } else {
      *pqt = setfix(qp, 0);
      *prt = setfix(rp, getfix(xp));
    }
  } else {
    if (isfix(yt)) {
      long y = getfix(yp), r; 
      bignum_t *bq = bndmodl(&r, getbig(xp), getfix(yp));
      if ((r < 0 && y > 0) || (r > 0 && y < 0)) {
        bignum_t *bt = bq; bq = bnsub(bt, bn1), bnfree(bt);
        r += y;
      }
      if (bnwidths(bq) > FIXNUM_WIDTH) *pqt = setbig(qp, bq);
      else (*pqt = setfix(qp, bntol(bq))), bnfree(bq);
      *prt = setfix(rp, r);
    } else {
      bignum_t *bx = getbig(xp), *by = getbig(yp), *br, *bq = bndmod(&br, bx, by);
      int sy = bnsign(by), sr = bnsign(br);
      if ((sr < 0 && sy > 0) || (sr > 0 && sy < 0)) {
        bignum_t *bt = bq; bq = bnsub(bt, bn1), bnfree(bt);
        bt = br; bq = bnadd(bt, by), bnfree(bt);
      }      
      if (bnwidths(bq) > FIXNUM_WIDTH) *pqt = setbig(qp, bq);
      else (*pqt = setfix(qp, bntol(bq))), bnfree(bq);
      if (bnwidths(br) > FIXNUM_WIDTH) *prt = setbig(rp, br);
      else (*prt = setfix(rp, bntol(br))), bnfree(br);
    }
  }
}
#endif

/* q = floor(sqrt(x)), r = x-floor(sqrt(x)) x >= 0 */
void intsqrt(numt_t *pqt, nump_t *qp, numt_t *prt, nump_t *rp, numt_t xt, const nump_t *xp)
{
  assert(NUMT_IS_INTNUM(xt) && getfix(xp) >= 0 && "non-natural number");
  if (isfix(xt)) {
    long x = getfix(xp), q;
    for (q = 1; q*q > x || x > q*(q+2); q = (q + x/q)/2);
    *pqt = setfix(qp, q), *prt = setfix(rp, x-q*q);
  } else {
#if 1
    bignum_t *bx = getbig(xp), *bq = NULL, *br = NULL;
    bnisqrt(bx, &bq, &br); assert(bq != NULL && br != NULL);
#else  
    bignum_t *bx = getbig(xp), *bq = bn1, *bq2, *bl, *bu;
    bignum_t *bn2 = ltobn(2), *br;
  loop:
    bl = bnmul(bq, bq), bq2 = bnmul(bq, bn2), bu = bnadd(bl, bq2);
    if (bncmp(bl, bx) <= 0 && bncmp(bx, bu) <= 0) {
      br = bnsub(bx, bl);
    } else {
      bignum_t *bt1 = bndiv(bx, bq), *bt2 = bnadd(bq, bt1);
      bignum_t *bq1 = bndiv(bt2, bn2);
      if (bq != bn1) bnfree(bq);
      bq = bq1;
      bnfree(bl), bnfree(bq2), bnfree(bu);
      bnfree(bt1), bnfree(bt2);
      goto loop;
    }
    bnfree(bl), bnfree(bq2), bnfree(bu), bnfree(bn2);
#endif
    if (bnwidths(bq) > FIXNUM_WIDTH) *pqt = setbig(qp, bq);
    else (*pqt = setfix(qp, bntol(bq))), bnfree(bq);
    if (bnwidths(br) > FIXNUM_WIDTH) *prt = setbig(rp, br);
    else (*prt = setfix(rp, bntol(br))), bnfree(br);
  }  
}

/* z = gcd(x, y) */
static numt_t intgcd(nump_t *zp, numt_t xt, const nump_t *xp, numt_t yt, const nump_t *yp)
{
  numt_t zt = NUMT_NONE;
  assert(NUMT_IS_INTNUM(xt) && "non-integer number");
  assert(NUMT_IS_INTNUM(yt) && "non-integer number");
  if (isfix(yt) && isbig(xt)) {
    numt_t tt = xt; const nump_t *tp = xp;
    xt = yt, yt = tt; xp = yp, yp = tp;
  }
  if (isfix(xt)) {
    long lx = labs(getfix(xp)), ly, lt;
    if (lx == 0) return intabs(zp, yt, yp);
    ly = isfix(yt) ? labs(getfix(yp)) : labs(bnmodl(getbig(yp), lx));
    if (lx < ly) lt = lx, lx = ly, ly = lt;
    while (ly) lt = ly, ly = lx % ly, lx = lt;
    if (lx <= FIXNUM_MAX) zt = setfix(zp, lx);
    else zt = setbig(zp, lltobn(lx));
  } else /* both are big */ {
    bignum_t *bz = bngcd(getbig(xp), getbig(yp));
    if (bnwidths(bz) > FIXNUM_WIDTH) zt = setbig(zp, bz);
    else (zt = setfix(zp, bntol(bz))), bnfree(bz);
  }
  return zt;
}

/* z = x^y, y >= 0 */
static numt_t intexptu(nump_t *zp, numt_t xt, const nump_t *xp, uint64_t y)
{
  assert(NUMT_IS_INTNUM(xt) && "non-integer number");
  /* x^0 = 1 (for all x, including 0) */
  if (y == 0) return setfix(zp, 1);
  /* x^1 = x for all x */
  if (y == 1) return numdup(zp, xt, xp);
  /* more special cases */
  if (isfix(xt)) {
    long x = getfix(xp);
    /* 0^y = 0 for y >= 0 */
    if (x == 0) return setfix(zp, 0); 
    /* 1^y = 1 */
    if (x == 1) return setfix(zp, 1); 
    /* -1^y = -1 if x is odd, 1 otherwise */
    if (x == -1) return (y & 1) ? setfix(zp, -1) : setfix(zp, 1);
    /* 2^y = ash(1, y) if y > 0, shortcut */
    if (x == 2 && y < FIXNUM_WIDTH-1) return setfix(zp, 1L << (long)y);
    { /* do it via intmul in hope we won't overflow into bignums */ 
      nump_t ap[1]; numt_t at = numdup(ap, xt, xp);
      numt_t zt = setfix(zp, 1);
      while (y > 0) {
        if (y & 1) {
          nump_t tp[1]; numt_t tt = intmul(tp, zt, zp, at, ap);
          numfini(zt, zp); zt = nummove(zp, tt, tp);
        }
        y >>= 1;
        if (y > 0) {
          nump_t tp[1]; numt_t tt = intmul(tp, at, ap, at, ap);
          numfini(at, ap); at = nummove(ap, tt, tp);
        }
      }
      numfini(at, ap);
      return zt;
    }
  } else { /* bignum case */    
    return setbig(zp, bnexptull(getbig(xp), y));
  }
}

/* z = ~x */
static numt_t intnot(nump_t *zp, numt_t xt, const nump_t *xp)
{
  assert(NUMT_IS_INTNUM(xt) && "non-integer number");
  if (isfix(xt)) {
    long x = getfix(xp), z = ~x;
    if (z >= FIXNUM_MIN && z <= FIXNUM_MAX) return setfix(zp, z);
    return setbig(zp, lltobn(z));
  } else {
    numt_t zt; bignum_t *bz = bnbitnot(getbig(xp)); 
    if (bnwidths(bz) > FIXNUM_WIDTH) zt = setbig(zp, bz);
    else (zt = setfix(zp, bntol(bz))), bnfree(bz);
    return zt;
  }
}

/* z = x & y */
static numt_t intand(nump_t *zp, numt_t xt, const nump_t *xp, numt_t yt, const nump_t *yp)
{
  assert(NUMT_IS_INTNUM(xt) && "non-integer number");
  assert(NUMT_IS_INTNUM(yt) && "non-integer number");

  if (isfix(xt) && isfix(yt)) return setfix(zp, getfix(xp) & getfix(yp));
  else if (isfix(xt) && getfix(xp) == 0) return setfix(zp, 0);
  else if (isfix(yt) && getfix(yp) == 0) return setfix(zp, 0);
  else if (isfix(xt) && getfix(xp) == -1) return numdup(zp, yt, yp);
  else if (isfix(xt) && getfix(xp) == -1) return numdup(zp, xt, xp);
  else {
    numt_t zt; bignumll_t blx, bly; 
    bignum_t *bx = isfix(xt) ? bnx_makell(&blx, getfix(xp)) : getbig(xp);
    bignum_t *by = isfix(yt) ? bnx_makell(&bly, getfix(yp)) : getbig(yp);
    bignum_t *bz = bnbitand(bx, by);
    if (bnwidths(bz) > FIXNUM_WIDTH) zt = setbig(zp, bz);
    else (zt = setfix(zp, bntol(bz))), bnfree(bz);
    return zt;
  }
}

/* z = x | y */
static numt_t intior(nump_t *zp, numt_t xt, const nump_t *xp, numt_t yt, const nump_t *yp)
{
  assert(NUMT_IS_INTNUM(xt) && "non-integer number");
  assert(NUMT_IS_INTNUM(yt) && "non-integer number");

  if (isfix(xt) && isfix(yt)) return setfix(zp, getfix(xp) | getfix(yp));
  else if (isfix(xt) && getfix(xp) == 0) return numdup(zp, yt, yp);
  else if (isfix(yt) && getfix(yp) == 0) return numdup(zp, xt, xp);
  else if (isfix(xt) && getfix(xp) == -1) return setfix(zp, -1);
  else if (isfix(yt) && getfix(yp) == -1) return setfix(zp, -1);
  else {
    numt_t zt; bignumll_t blx, bly; 
    bignum_t *bx = isfix(xt) ? bnx_makell(&blx, getfix(xp)) : getbig(xp);
    bignum_t *by = isfix(yt) ? bnx_makell(&bly, getfix(yp)) : getbig(yp);
    bignum_t *bz = bnbitior(bx, by);
    if (bnwidths(bz) > FIXNUM_WIDTH) zt = setbig(zp, bz);
    else (zt = setfix(zp, bntol(bz))), bnfree(bz);
    return zt;
  }
}

/* z = x ^ y */
static numt_t intxor(nump_t *zp, numt_t xt, const nump_t *xp, numt_t yt, const nump_t *yp)
{
  assert(NUMT_IS_INTNUM(xt) && "non-integer number");
  assert(NUMT_IS_INTNUM(yt) && "non-integer number");

  if (isfix(xt) && isfix(yt)) return setfix(zp, getfix(xp) ^ getfix(yp));
  else if (isfix(xt) && getfix(xp) == 0) return numdup(zp, yt, yp);
  else if (isfix(yt) && getfix(yp) == 0) return numdup(zp, xt, xp);
  else if (isfix(xt) && getfix(xp) == -1) return intnot(zp, yt, yp);
  else if (isfix(yt) && getfix(yp) == -1) return intnot(zp, xt, xp);
  else {
    numt_t zt; bignumll_t blx, bly; 
    bignum_t *bx = isfix(xt) ? bnx_makell(&blx, getfix(xp)) : getbig(xp);
    bignum_t *by = isfix(yt) ? bnx_makell(&bly, getfix(yp)) : getbig(yp);
    bignum_t *bz = bnbitxor(bx, by);
    if (bnwidths(bz) > FIXNUM_WIDTH) zt = setbig(zp, bz);
    else (zt = setfix(zp, bntol(bz))), bnfree(bz);
    return zt;
  }
}

/* z = x << y, right on negative y */
static numt_t intash(nump_t *zp, numt_t xt, const nump_t *xp, numt_t yt, const nump_t *yp)
{
  assert(NUMT_IS_INTNUM(xt) && "non-integer number");
  assert(NUMT_IS_INTNUM(yt) && "non-integer number");
  
  if (isfix(xt) && getfix(xp) == 0) return setfix(zp, 0);
  else if (isfix(yt) && getfix(yp) == 0) return numdup(zp, xt, xp);
  else if (isbig(yt)) {
    if (bnsign(getbig(yp)) < 0) return setfix(zp, intsign(xt, xp) < 0 ? -1 : 0);
    else return setfail(EDOM); /* out of memory */
  } else {
    numt_t zt; bignumll_t blx; bignum_t *bx, *bz;
    long y = getfix(yp);
    if (isfix(xt)) {
      long x = getfix(xp);
      if (y < 0) {
        long ay = -y;
        if (y >= FIXNUM_WIDTH) return setfix(zp, x < 0 ? -1 : 0);
        if (x >= 0) return setfix(zp, x >> ay);
        return setfix(zp, -(((-x-1) >> ay) + 1));
      } else { /* y > 0 */
        if (y >= BIGNUM_MAX_BITS) return setfail(EDOM); /* out of memory */
        if (y < FIXNUM_WIDTH - 1) { 
          long z = x << y; 
          if ((z >> y) == x && FIXNUM_MIN <= z && z <= FIXNUM_MAX) 
            return setfix(zp, z); 
        } /* else fall thru */
      }
    }
    bx = isfix(xt) ? bnx_makell(&blx, getfix(xp)) : getbig(xp);
    bz = bnbitash(bx, y);
    if (bnwidths(bz) > FIXNUM_WIDTH) zt = setbig(zp, bz);
    else (zt = setfix(zp, bntol(bz))), bnfree(bz);
    return zt;
  }
}

/* z = integer-length(x); always returns fixnum */
static numt_t intlen(nump_t *zp, numt_t xt, const nump_t *xp)
{
  assert(NUMT_IS_INTNUM(xt) && "non-integer number");

  if (isfix(xt)) {
    return setfix(zp, fxlen(getfix(xp)));
  } else {
    long len = bnintlen(getbig(xp));
    return setfix(zp, len);
  }
}

/* z = bit-count(x); always returns fixnum */
static numt_t intbtc(nump_t *zp, numt_t xt, const nump_t *xp)
{
  long cnt = 0;
  assert(NUMT_IS_INTNUM(xt) && "non-integer number");

  if (isfix(xt)) {
    return fxbtc(getfix(xp));
  } else {
    cnt = bnbitc(getbig(xp));
  }
  return (cnt <= FIXNUM_MAX) ? setfix(zp, cnt) : setfail(EDOM);
}


/* returns NUMT_NONE and sets errno on failure */
static numt_t strtoint(nump_t *zp, const char *str, char **endp, int radix)
{
  long l; char *ep = NULL;
  numt_t zt = NUMT_NONE;
  assert(str); assert(radix >= 2 && radix <= 36);
  errno = 0; 
  l = strtol(str, &ep, radix);
  if (errno == 0 && ep && ep > str && !isdigit(*ep)
      && l >= FIXNUM_MIN && l <= FIXNUM_MAX) {
    zt = setfix(zp, l);
  } else {
    bignum_t *bn;
    errno = 0; 
    bn = strtobn(str, &ep, radix);
    if (bn && errno == 0 && ep && !isdigit(*ep)) {
      zt = setbig(zp, bn);
    } else {
      if (bn) bnfree(bn);
      zt = setfail(EDOM);
    }
  }
  if (endp) *endp = ep;
  return zt;
}


/* # of chars needed for x in radix, including '-' for negs and '\0' */
static size_t intfmtsize(numt_t xt, const nump_t *xp, int radix)
{
  assert(radix >= 2 && radix <= 36);
  assert(NUMT_IS_INTNUM(xt) && "non-integer number");
  if (isfix(xt)) {
#if 1 /* quick estimate */
    if (radix < 4)       return FIXNUM_WIDTH/1+3;
    else if (radix < 8)  return FIXNUM_WIDTH/2+3;
    else if (radix < 16) return FIXNUM_WIDTH/3+3;
    else if (radix < 32) return FIXNUM_WIDTH/4+3;
    else                 return FIXNUM_WIDTH/5+3;
#else /* too slow */
    size_t cnt;
    long lx = labs(getfix(xp));
    for (cnt = 1; lx > 0; lx /= radix) ++cnt;
    if (getfix(xp) <= 0) ++cnt;
    return cnt;
#endif
  } else {
    return bnfmtsize(getbig(xp), radix);
  }
}

/* format x into buffer; len should be as calculated by intfmtsize or 1 
 * shorter for negative x if the sign needs to be omitted; returns first 
 * char of the zero-terminated result in buffer (prints right-to-left) */
static char *inttostr(char *buffer, size_t len, numt_t xt, const nump_t *xp, int radix)
{
  assert(radix >= 2 && radix <= 36);
  assert(NUMT_IS_INTNUM(xt) && "non-integer number");
  if (isfix(xt)) {
    long lx = labs(getfix(xp));
    char *pc = buffer + len;
    for (*--pc = 0; lx > 0; lx /= radix) {
      int d = (int)(lx % radix);
      *--pc = d < 10 ? '0' + d : 'A' + d-10;
    }
    if (getfix(xp) == 0) *--pc = '0';
    else if (getfix(xp) < 0 && pc > buffer) *--pc = '-';
    assert(pc >= buffer);
    return pc;
  } else { /* print bignum right-to-left */
    return bntostr(buffer, len, getbig(xp), radix);
  }
}

/* (double)x */
static double inttod(numt_t xt, const nump_t *xp)
{
  assert(NUMT_IS_INTNUM(xt) && "non-integer number");
  if (isfix(xt)) return (double)getfix(xp);
  return bntod(getbig(xp));
}


/* generic rational arithmetics */

/* in normal form, only non-0 ratios with non-1 denominator are represented as 2-slot numbers;
 * in those, denominator is always positive and has no common non-1 factors with the numerator */

#define NUMT_IS_INTORNONE(nt) ((nt) == NUMT_FIX || (nt) == NUMT_BIG || (nt) == NUMT_NONE)
#define NUMT_IS_RATNUM(nt) (((nt) & NUMT_DS_MASK) == (nt) && NUMT_IS_INTNUM(NUMT_RAT_N(nt)) && NUMT_IS_INTORNONE(NUMT_RAT_D(nt)))

/* these macros assume RATNUM arguments */
#define isint(xt)     (NUMT_IS_INTNUM(xt))

/* x <=> y, returns -1/0/1 */
int ratcmp(numt_t xt, const nump_t *xp, numt_t yt, const nump_t *yp)
{
  assert(NUMT_IS_RATNUM(xt) && "non-rational number");
  assert(NUMT_IS_RATNUM(yt) && "non-rational number");
  if (isint(xt) && isint(yt)) {
    return intcmp(xt, xp, yt, yp);
  } else {
    bignumll_t nxll, dxll, nyll, dyll;
    numt_t nxt = NUMT_RAT_N(xt), dxt = NUMT_RAT_D(xt), nyt = NUMT_RAT_N(yt), dyt = NUMT_RAT_D(yt);
    bignum_t *nx = isbig(nxt) ? getbig(xp)   : bnx_makell(&nxll, getfix(xp));
    bignum_t *dx = isbig(dxt) ? getbig(xp+1) : bnx_makell(&dxll, dxt ? getfix(xp+1) : 1);
    bignum_t *ny = isbig(nyt) ? getbig(yp)   : bnx_makell(&nyll, getfix(yp));
    bignum_t *dy = isbig(dyt) ? getbig(yp+1) : bnx_makell(&dyll, dyt ? getfix(yp+1) : 1);
    return bnrcmp(nx, dx, ny, dy);
  }
}

/* x == y */
int rateq(numt_t xt, const nump_t *xp, numt_t yt, const nump_t *yp)
{
  assert(NUMT_IS_RATNUM(xt) && "non-rational number");
  assert(NUMT_IS_RATNUM(yt) && "non-rational number");
  if (isint(xt) && isint(yt)) {
    return inteq(xt, xp, yt, yp);
  } else if (!isint(xt) && !isint(yt)) {
    return inteq(NUMT_RAT_N(xt), xp, NUMT_RAT_N(yt), yp) && inteq(NUMT_RAT_D(xt), xp+1, NUMT_RAT_D(yt), yp+1);
  } else { /* intnums and ratnums are disjoint */
    return 0;
  }
}

/* x < y */
#define ratless(xt, xp, yt, yp) (ratcmp(xt, xp, yt, yp) < 0)

/* z = -x */
static numt_t ratneg(nump_t *zp, numt_t xt, const nump_t *xp)
{
  assert(NUMT_IS_RATNUM(xt) && "non-rational number");
  if (isint(xt)) {
    return intneg(zp, xt, xp);
  } else {
    return NUMT_MKRAT(intneg(zp, NUMT_RAT_N(xt), xp), numdup(zp+1, NUMT_RAT_D(xt), xp+1));
  }
}

/* z = 1/x */
static numt_t ratrcp(nump_t *zp, numt_t xt, const nump_t *xp)
{
  assert(NUMT_IS_RATNUM(xt) && "non-rational number");
  /* shortcuts */
  if (isfix(xt)) {
    long x = getfix(xp);
    if (x == 0) return setfail(EDOM);
    if (x == 1 || x == -1) return setfix(zp, x);
    if (x > 0) return NUMT_MKRAT(setfix(zp, 1), setfix(zp+1, x));
    if (x > FIXNUM_MIN) return NUMT_MKRAT(setfix(zp, -1), setfix(zp+1, -x));
    else return NUMT_MKRAT(setfix(zp, -1), intneg(zp+1, xt, xp));  
  } else if (isint(xt)) {
    if (intsign(xt, xp) > 0) return NUMT_MKRAT(setfix(zp, 1), numdup(zp+1, xt, xp));
    else return NUMT_MKRAT(setfix(zp, -1), intabs(zp+1, xt, xp));
  } else if (isfix(NUMT_RAT_N(xt)) && getfix(xp) == 1) {
    return numdup(zp, NUMT_RAT_D(xt), xp+1);
  } else if (isfix(NUMT_RAT_N(xt)) && getfix(xp) == -1) {
    return intneg(zp, NUMT_RAT_D(xt), xp+1);
  } else if (intsign(NUMT_RAT_N(xt), xp) > 0) {
    return NUMT_MKRAT(numdup(zp, NUMT_RAT_D(xt), xp+1), numdup(zp+1, NUMT_RAT_N(xt), xp)); 
  } else {
    return NUMT_MKRAT(intneg(zp, NUMT_RAT_D(xt), xp+1), intneg(zp+1, NUMT_RAT_N(xt), xp));
  }
}

/* z = |x| */
static numt_t ratabs(nump_t *zp, numt_t xt, const nump_t *xp)
{
  assert(NUMT_IS_RATNUM(xt) && "non-rational number");
  if (isint(xt)) {
    return intabs(zp, xt, xp);
  } else if (intsign(NUMT_RAT_N(xt), xp) < 0) {
    return NUMT_MKRAT(intabs(zp, NUMT_RAT_N(xt), xp), numdup(zp+1, NUMT_RAT_D(xt), xp+1));
  } else {
    return numdup(zp, xt, xp); /* must be fresh! */
  }
}

/* sign(x) (-1 0 +1) */
static int ratsign(numt_t xt, const nump_t *xp)
{
  assert(NUMT_IS_RATNUM(xt) && "non-rational number");
  return intsign(NUMT_RAT_N(xt), xp);
}

/* z = x + y */
static numt_t ratadd(nump_t *zp, numt_t xt, const nump_t *xp, numt_t yt, const nump_t *yp)
{
  assert(NUMT_IS_RATNUM(xt) && "non-rational number");
  assert(NUMT_IS_RATNUM(yt) && "non-rational number");
  if (isint(xt) && isint(yt)) {
    return intadd(zp, xt, xp, yt, yp);
  } else if (isfix(xt) && getfix(xp) == 0) {
    return numdup(zp, yt, yp);
  } else if (isfix(yt) && getfix(yp) == 0) {
    return numdup(zp, xt, xp);
  } else {
    numt_t zt = NUMT_NONE;
    numt_t nxt; nump_t nxp[1]; numt_t dxt; nump_t dxp[1]; /* owned by caller */
    numt_t nyt; nump_t nyp[1]; numt_t dyt; nump_t dyp[1]; /* owned by caller */
    numt_t g1t; nump_t g1p[1]; /* new */
    nxt = NUMT_RAT_N(xt), *nxp = *xp;
    if (!(dxt = NUMT_RAT_D(xt))) dxt = setfix(dxp, 1); else dxp[0] = xp[1];
    nyt = NUMT_RAT_N(yt), *nyp = *yp;
    if (!(dyt = NUMT_RAT_D(yt))) dyt = setfix(dyp, 1); else dyp[0] = yp[1];
    g1t = intgcd(g1p, dxt, dxp, dyt, dyp);
    if (isfix(g1t) && getfix(g1p) == 1) {
      numt_t m1t; nump_t m1p[1]; numt_t m2t; nump_t m2p[1]; /* new */
      m1t = intmul(m1p, nxt, nxp, dyt, dyp), m2t = intmul(m2p, nyt, nyp, dxt, dxp);
      zt = NUMT_MKRAT(intadd(zp, m1t, m1p, m2t, m2p), intmul(zp+1, dxt, dxp, dyt, dyp));
      numfini(m1t, m1p), numfini(m2t, m2p);  
    } else {
      numt_t qxt; nump_t qxp[1]; numt_t qyt; nump_t qyp[1]; /* new */
      numt_t m1t; nump_t m1p[1]; numt_t m2t; nump_t m2p[1]; /* new */
      numt_t s1t; nump_t s1p[1]; numt_t g2t; nump_t g2p[1]; /* new */
      numt_t d1t; nump_t d1p[1]; numt_t d2t; nump_t d2p[1]; /* new */
      qxt = intquo(qxp, dxt, dxp, g1t, g1p), qyt = intquo(qyp, dyt, dyp, g1t, g1p);
      m1t = intmul(m1p, nxt, nxp, qyt, qyp), m2t = intmul(m2p, nyt, nyp, qxt, qxp);
      s1t = intadd(s1p, m1t, m1p, m2t, m2p), g2t = intgcd(g2p, s1t, s1p, g1t, g1p);
      d1t = intquo(d1p, dxt, dxp, g1t, g1p), d2t = intquo(d2p, dyt, dyp, g2t, g2p);
      zt = NUMT_MKRAT(intquo(zp, s1t, s1p, g2t, g2p), intmul(zp+1, d1t, d1p, d2t, d2p));
      if (isfix(NUMT_RAT_D(zt)) && getfix(zp+1) == 1) zt = NUMT_RAT_N(zt);
      numfini(qxt, qxp), numfini(qyt, qyp);
      numfini(m1t, m1p), numfini(m2t, m2p);  
      numfini(s1t, s1p), numfini(g2t, g2p);  
      numfini(d1t, d1p), numfini(d2t, d2p);  
    }
    numfini(g1t, g1p);
    return zt; 
  }
}

/* z = x - y */
static numt_t ratsub(nump_t *zp, numt_t xt, const nump_t *xp, numt_t yt, const nump_t *yp)
{
  assert(NUMT_IS_RATNUM(xt) && "non-rational number");
  assert(NUMT_IS_RATNUM(yt) && "non-rational number");
  if (isint(xt) && isint(yt)) {
    return intsub(zp, xt, xp, yt, yp);
  } else if (isfix(xt) && getfix(xp) == 0) {
    return ratneg(zp, yt, yp);
  } else if (isfix(yt) && getfix(yp) == 0) {
    return numdup(zp, xt, xp);
  } else {
    numt_t zt = NUMT_NONE;
    numt_t nxt; nump_t nxp[1]; numt_t dxt; nump_t dxp[1]; /* owned by caller */
    numt_t nyt; nump_t nyp[1]; numt_t dyt; nump_t dyp[1]; /* owned by caller */
    numt_t g1t; nump_t g1p[1]; /* new */
    nxt = NUMT_RAT_N(xt), *nxp = *xp;
    if (!(dxt = NUMT_RAT_D(xt))) dxt = setfix(dxp, 1); else dxp[0] = xp[1];
    nyt = NUMT_RAT_N(yt), *nyp = *yp;
    if (!(dyt = NUMT_RAT_D(yt))) dyt = setfix(dyp, 1); else dyp[0] = yp[1];
    g1t = intgcd(g1p, dxt, dxp, dyt, dyp);
    if (isfix(g1t) && getfix(g1p) == 1) {
      numt_t m1t; nump_t m1p[1]; numt_t m2t; nump_t m2p[1]; /* new */
      m1t = intmul(m1p, nxt, nxp, dyt, dyp), m2t = intmul(m2p, nyt, nyp, dxt, dxp);
      zt = NUMT_MKRAT(intsub(zp, m1t, m1p, m2t, m2p), intmul(zp+1, dxt, dxp, dyt, dyp));
      numfini(m1t, m1p), numfini(m2t, m2p);  
    } else {
      numt_t qxt; nump_t qxp[1]; numt_t qyt; nump_t qyp[1]; /* new */
      numt_t m1t; nump_t m1p[1]; numt_t m2t; nump_t m2p[1]; /* new */
      numt_t s1t; nump_t s1p[1]; numt_t g2t; nump_t g2p[1]; /* new */
      numt_t d1t; nump_t d1p[1]; numt_t d2t; nump_t d2p[1]; /* new */
      qxt = intquo(qxp, dxt, dxp, g1t, g1p), qyt = intquo(qyp, dyt, dyp, g1t, g1p);
      m1t = intmul(m1p, nxt, nxp, qyt, qyp), m2t = intmul(m2p, nyt, nyp, qxt, qxp);
      s1t = intsub(s1p, m1t, m1p, m2t, m2p), g2t = intgcd(g2p, s1t, s1p, g1t, g1p);
      d1t = intquo(d1p, dxt, dxp, g1t, g1p), d2t = intquo(d2p, dyt, dyp, g2t, g2p);
      zt = NUMT_MKRAT(intquo(zp, s1t, s1p, g2t, g2p), intmul(zp+1, d1t, d1p, d2t, d2p));
      if (isfix(NUMT_RAT_D(zt)) && getfix(zp+1) == 1) zt = NUMT_RAT_N(zt);
      numfini(qxt, qxp), numfini(qyt, qyp);
      numfini(m1t, m1p), numfini(m2t, m2p);  
      numfini(s1t, s1p), numfini(g2t, g2p);  
      numfini(d1t, d1p), numfini(d2t, d2p);  
    }
    numfini(g1t, g1p);
    return zt; 
  }
}

/* z = x * y */
static numt_t ratmul(nump_t *zp, numt_t xt, const nump_t *xp, numt_t yt, const nump_t *yp)
{
  assert(NUMT_IS_RATNUM(xt) && "non-rational number");
  assert(NUMT_IS_RATNUM(yt) && "non-rational number");
  if (isfix(xt) && getfix(xp) == 0) {
    return setfix(zp, 0);
  } else if (isfix(yt) && getfix(yp) == 0) {
    return setfix(zp, 0);
  } if (isfix(xt) && getfix(xp) == 1) {
    return numdup(zp, yt, yp);
  } else if (isfix(yt) && getfix(yp) == 1) {
    return numdup(zp, xt, xp);
  } else if (isint(xt) && isint(yt)) {
    return intmul(zp, xt, xp, yt, yp);
  } else {
    numt_t zt = NUMT_NONE;
    numt_t nxt; nump_t nxp[1]; numt_t dxt; nump_t dxp[1]; /* owned by caller */
    numt_t nyt; nump_t nyp[1]; numt_t dyt; nump_t dyp[1]; /* owned by caller */
    numt_t g1t; nump_t g1p[1]; numt_t g2t; nump_t g2p[1]; /* new */
    numt_t rnxt; nump_t rnxp[1]; numt_t rdxt; nump_t rdxp[1]; /* new */
    numt_t rnyt; nump_t rnyp[1]; numt_t rdyt; nump_t rdyp[1]; /* new */
    nxt = NUMT_RAT_N(xt), *nxp = *xp;
    if (!(dxt = NUMT_RAT_D(xt))) dxt = setfix(dxp, 1); else dxp[0] = xp[1];
    nyt = NUMT_RAT_N(yt), *nyp = *yp;
    if (!(dyt = NUMT_RAT_D(yt))) dyt = setfix(dyp, 1); else dyp[0] = yp[1];
    g1t = intgcd(g1p, nxt, nxp, dyt, dyp), g2t = intgcd(g2p, dxt, dxp, nyt, nyp);
    rnxt = intquo(rnxp, nxt, nxp, g1t, g1p), rdxt = intquo(rdxp, dxt, dxp, g2t, g2p);    
    rnyt = intquo(rnyp, nyt, nyp, g2t, g2p), rdyt = intquo(rdyp, dyt, dyp, g1t, g1p);
    zt = NUMT_MKRAT(intmul(zp, rnxt, rnxp, rnyt, rnyp), intmul(zp+1, rdxt, rdxp, rdyt, rdyp));
    if (isfix(NUMT_RAT_D(zt)) && getfix(zp+1) == 1) zt = NUMT_RAT_N(zt);
    numfini(rnxt, rnxp), numfini(rdxt, rdxp);
    numfini(rnyt, rnyp), numfini(rdyt, rdyp);
    numfini(g1t, g1p), numfini(g2t, g2p);
    return zt; 
  }
}

/* z = x / y */
static numt_t ratdiv(nump_t *zp, numt_t xt, const nump_t *xp, numt_t yt, const nump_t *yp)
{
  assert(NUMT_IS_RATNUM(xt) && "non-rational number");
  assert(NUMT_IS_RATNUM(yt) && "non-rational number");
  if (isfix(yt) && getfix(yp) == 0) {
    return setfail(EDOM);
  } else if (isfix(xt) && getfix(xp) == 1) {
    return ratrcp(zp, yt, yp);
  } else if (isfix(yt) && getfix(yp) == 1) {
    return numdup(zp, xt, xp);
  } else if (isfix(yt) && getfix(yp) == -1) {
    return ratneg(zp, xt, xp);
  } else {
    numt_t zt = NUMT_NONE; int nysign;
    numt_t nxt; nump_t nxp[1]; numt_t dxt; nump_t dxp[1]; /* owned by caller */
    numt_t nyt; nump_t nyp[1]; numt_t dyt; nump_t dyp[1]; /* owned by caller */
    numt_t gnt; nump_t gnp[1]; numt_t gdt; nump_t gdp[1]; /* new */
    numt_t rnxt; nump_t rnxp[1]; numt_t rdxt; nump_t rdxp[1]; /* new */
    numt_t rnyt; nump_t rnyp[1]; numt_t rdyt; nump_t rdyp[1]; /* new */
    nxt = NUMT_RAT_N(xt), *nxp = *xp;
    if (!(dxt = NUMT_RAT_D(xt))) dxt = setfix(dxp, 1); else dxp[0] = xp[1];
    nyt = NUMT_RAT_N(yt), *nyp = *yp;
    if (!(dyt = NUMT_RAT_D(yt))) dyt = setfix(dyp, 1); else dyp[0] = yp[1];
    gnt = intgcd(gnp, nxt, nxp, nyt, nyp), gdt = intgcd(gdp, dxt, dxp, dyt, dyp);
    rnxt = intquo(rnxp, nxt, nxp, gnt, gnp), rdxt = intquo(rdxp, dxt, dxp, gdt, gdp);    
    rnyt = intquo(rnyp, nyt, nyp, gnt, gnp), rdyt = intquo(rdyp, dyt, dyp, gdt, gdp);    
    nysign = intsign(nyt, nyp); assert(nysign);
    if (nysign < 0) {
      numt_t m1t; nump_t m1p[1]; numt_t m2t; nump_t m2p[1]; /* new */
      m1t = intmul(m1p, rnxt, rnxp, rdyt, rdyp), m2t = intmul(m2p, rdxt, rdxp, rnyt, rnyp);
      zt = NUMT_MKRAT(intneg(zp, m1t, m1p), intneg(zp+1, m2t, m2p));
      numfini(m1t, m1p), numfini(m2t, m2p);
    } else {
      zt = NUMT_MKRAT(intmul(zp, rnxt, rnxp, rdyt, rdyp), intmul(zp+1, rdxt, rdxp, rnyt, rnyp));
    }
    if (isfix(NUMT_RAT_D(zt)) && getfix(zp+1) == 1) zt = NUMT_RAT_N(zt);
    numfini(gnt, gnp), numfini(gdt, gdp);
    numfini(rnxt, rnxp), numfini(rdxt, rdxp);
    numfini(rnyt, rnyp), numfini(rdyt, rdyp);
    return zt; 
  }
}

/* z = floor(x) */
static numt_t ratfloor(nump_t *zp, numt_t xt, const nump_t *xp)
{
  assert(NUMT_IS_RATNUM(xt) && "non-rational number");
  if (isint(xt)) {
    return numdup(zp, xt, xp); /* must be fresh! */
  } else if (intsign(NUMT_RAT_N(xt), xp) > 0) {
    return intquo(zp, NUMT_RAT_N(xt), xp, NUMT_RAT_D(xt), xp+1);
  } else {
    numt_t zt; 
    numt_t qt; nump_t qp[1]; numt_t ot; nump_t op[1];
    qt = intquo(qp, NUMT_RAT_N(xt), xp, NUMT_RAT_D(xt), xp+1);
    ot = setfix(op, 1);
    zt = intsub(zp, qt, qp, ot, op);
    numfini(qt, qp), numfini(ot, op);
    return zt; 
  }
}

/* z = ceiling(x) */
static numt_t ratceil(nump_t *zp, numt_t xt, const nump_t *xp)
{
  assert(NUMT_IS_RATNUM(xt) && "non-rational number");
  if (isint(xt)) {
    return numdup(zp, xt, xp); /* must be fresh! */
  } else if (intsign(NUMT_RAT_N(xt), xp) < 0) {
    return intquo(zp, NUMT_RAT_N(xt), xp, NUMT_RAT_D(xt), xp+1);
  } else {
    numt_t zt; 
    numt_t qt; nump_t qp[1]; numt_t ot; nump_t op[1];
    qt = intquo(qp, NUMT_RAT_N(xt), xp, NUMT_RAT_D(xt), xp+1);
    ot = setfix(op, 1);
    zt = intadd(zp, qt, qp, ot, op);
    numfini(qt, qp), numfini(ot, op);
    return zt; 
  }
}

/* z = truncate(x) */
static numt_t rattrunc(nump_t *zp, numt_t xt, const nump_t *xp)
{
  assert(NUMT_IS_RATNUM(xt) && "non-rational number");
  if (isint(xt)) {
    return numdup(zp, xt, xp); /* must be fresh! */
  } else {
    return intquo(zp, NUMT_RAT_N(xt), xp, NUMT_RAT_D(xt), xp+1);
  }
}

/* z = round(x) */
static numt_t ratround(nump_t *zp, numt_t xt, const nump_t *xp)
{
  assert(NUMT_IS_RATNUM(xt) && "non-rational number");
  if (isint(xt)) {
    return numdup(zp, xt, xp); /* must be fresh! */
  } else {
    numt_t zt; 
    int s = intsign(NUMT_RAT_N(xt), xp);
    numt_t st; nump_t sp[2]; numt_t ht; nump_t hp[2];
    ht = NUMT_MKRAT(setfix(hp, s<0 ? -1 : 1), setfix(hp+1, 2));
    st = ratadd(sp, xt, xp, ht, hp);
    if (isint(st)) {
      /* either s or its closer-to-zero neighbor */
      if (intodd(st, sp)) {
        numfini(ht, hp); ht = setfix(hp, s<0 ? 1 : -1);
        zt = ratadd(zp, st, sp, ht, hp);
      } else {
        zt = numdup(zp, st, sp);
      }
    } else {
      zt = intquo(zp, NUMT_RAT_N(st), sp, NUMT_RAT_D(st), sp+1);
    }
    numfini(st, sp); numfini(ht, hp);
    return zt;
  }
}

/* z = x * (b ^ e) */
static numt_t ratscale(nump_t *zp, numt_t xt, const nump_t *xp, long b, long e)
{
  assert(b > 0 && b <= FIXNUM_MAX && e < FIXNUM_MAX);
  if (!e) {
    return numdup(zp, xt, xp);
  } else {
    nump_t bp[1]; numt_t bt = setfix(bp, b);
    nump_t sp[1]; numt_t st = intexptu(sp, bt, bp, (uint64_t)labs(e));
    numt_t zt;
    if (e < 0) zt = ratdiv(zp, xt, xp, st, sp);
    else zt = ratmul(zp, xt, xp, st, sp);
    numfini(st, sp);
    numfini(bt, bp);
    return zt;
  }
}

#ifndef COMPACT_RATTRIG
void ratsincos(numt_t xt, const nump_t *xp, double *psin, double *pcos)
{
  assert(NUMT_IS_RATNUM(xt) && "non-rational number");
  if (isfix(xt)) {
#if defined(_GNU_SOURCE)
    sincos(getfix(xp), psin, pcos);
#else
    long x = getfix(xp);
    *psin = sin(x), *pcos = cos(x);
#endif
  } else if (isbig(xt)) {
    bnrsincostod(getbig(xp), bn1, psin, pcos);
  } else { /* ratio */
    bignumll_t nll, dll; 
    bignum_t *n = isfix(NUMT_RAT_N(xt)) ? (nll = bnll(getfix(xp)), (bignum_t *)&nll) : getbig(xp);
    bignum_t *d = isfix(NUMT_RAT_D(xt)) ? (dll = bnll(getfix(xp+1)), (bignum_t *)&dll) : getbig(xp+1);
    bnrsincostod(n, d, psin, pcos);
  }
}
#endif

/* returns NUMT_NONE and sets errno on failure */
static numt_t strtorat(nump_t *zp, const char *str, char **endp, int radix)
{
  numt_t zt = NUMT_NONE; char *ep = NULL;
  int decimal = (radix == 2 || radix == 4 || radix == 8 || radix == 10 || radix == 16);
  assert(str); assert(radix >= 2 && radix <= 36);
  errno = 0;
  if (decimal && str[0] == '.' && char_to_val(str[1], radix) >= 0) 
    ep = (char*)str, zt = setfix(zp, 0);
  else if (decimal && str[0] == '-' && str[1] == '.' && char_to_val(str[2], radix) >= 0) 
    ep = (char*)str+1, zt = setfix(zp, 0);
  else if (decimal && str[0] == '+' && str[1] == '.' && char_to_val(str[2], radix) >= 0) 
    ep = (char*)str+1, zt = setfix(zp, 0);
  else zt = strtoint(zp, str, &ep, radix);
  if (errno) {
    numfini(zt, zp);
    zt = setfail(EDOM);
  } else if (ep && *ep == '/') {
    nump_t np[1]; numt_t nt = nummove(np, zt, zp);
    nump_t dp[1]; numt_t dt = strtoint(dp, ep+1, &ep, radix);
    if (errno) {
      zt = setfail(EDOM);
    } else if (isfix(dt) && getfix(dp) == 0) {
      switch (intsign(nt, np)) {
        case -1: zt = setflo(zp, -HUGE_VAL); break;
        case  0: zt = setflo(zp, HUGE_VAL-HUGE_VAL); break;
        case +1: zt = setflo(zp, HUGE_VAL); break;
        default: assert(0); zt = 0;
      }
      errno = ERANGE; /* z is useful but out of range */
    } else {
      zt = ratdiv(zp, nt, np, dt, dp);
    }
    numfini(nt, np); numfini(dt, dp);
  } else if (decimal && ep && (*ep == '.' || tolower(*ep) == 'e' || tolower(*ep) == 'p')) {
    long fdc = 0; 
    nump_t fp[1]; numt_t ft = setfix(fp, 0);
    if (ep[0] == '.') {
      char *fep = ++ep;
      if (char_to_val(*ep, radix) >= 0) ft = strtoint(fp, ep, &ep, radix);
      if (ep) fdc = (long)(ep-fep);
    }
    if (errno || !ep) {
      numfini(zt, zp);
      zt = setfail(EDOM);
    } else {
      nump_t np[2]; numt_t nt; /* used for rats too */
      nump_t tp[1]; numt_t tt = setfix(tp, radix);
      long i; int ebase = radix;
      for (i = 0; i < fdc; ++i) {
        nt = nummove(np, zt, zp);
        zt = intmul(zp, nt, np, tt, tp);
        numfini(nt, np);
      }
      nt = nummove(np, zt, zp);
      if (str[0] == '-') intsub(zp, nt, np, ft, fp);
      else intadd(zp, nt, np, ft, fp);
      numfini(nt, np);
      numfini(ft, fp);
      if (tolower(*ep) == 'p') ebase = 2;
      if (*ep && tolower(*ep) == 'e') ft = strtoint(fp, ep+1, &ep, radix);
      else if (*ep && tolower(*ep) == 'p') ft = strtoint(fp, ep+1, &ep, 10);
      else ft = setfix(fp, 0);
      if (errno || !ep || !isfix(ft)) {
        numfini(zt, zp);
        numfini(ft, fp);
        numfini(tt, tp);
        zt = setfail(EDOM);
      } else { /* valid exponent */
        long e = getfix(fp); 
        nt = nummove(np, zt, zp);
        if (ebase == radix) { /* e can be combined with fdc */
          zt = ratscale(zp, nt, np, ebase, e - fdc);
        } else { /* e and fdc need to be used separately */
          zt = ratscale(zp, nt, np, radix, -fdc);
          numfini(nt, np); nt = nummove(np, zt, zp);
          zt = ratscale(zp, nt, np, ebase, e);
        }
        numfini(nt, np);
        numfini(tt, tp);
        numfini(ft, fp);
      }
    }
  }
  if (endp) *endp = ep;
  return zt;
}

/* # of chars needed for x in radix, including '/', '-' for negs and '\0' */
static size_t ratfmtsize(numt_t xt, const nump_t *xp, int radix)
{
  assert(radix >= 2 && radix <= 36);
  assert(NUMT_IS_RATNUM(xt) && "non-rational number");
  if (isint(xt)) {
    return intfmtsize(xt, xp, radix);
  } else {
    /* note that first intfmtsize include trailing \0 that will be replaced by / */
    return intfmtsize(NUMT_RAT_N(xt), xp, radix) + intfmtsize(NUMT_RAT_D(xt), xp+1, radix);
  }
}

/* format x into buffer; len should be as calculated by ratfmtsize;
 * returns char of the zero-terminated result in buffer. */
static char *rattostr(char *buffer, size_t len, numt_t xt, const nump_t *xp, int radix)
{
  assert(radix >= 2 && radix <= 36);
  assert(NUMT_IS_RATNUM(xt) && "non-rational number");
  if (isint(xt)) {
    return inttostr(buffer, len, xt, xp, radix);
  } else { /* NB: inttostr prints right-to-left, so this: */
    char *sep = inttostr(buffer, len, NUMT_RAT_D(xt), xp+1, radix);
    char *res = inttostr(buffer, sep-buffer, NUMT_RAT_N(xt), xp, radix);
    assert(sep[-1] == 0); sep[-1] = '/';
    return res;
  }
}

/* (double)x */
static double rattod(numt_t xt, const nump_t *xp)
{
  assert(NUMT_IS_RATNUM(xt) && "non-rational number");
  if (isint(xt)) return inttod(xt, xp);
  /* if neither n nor d can overflow a double, do it the easy way */
  if ((isfix(NUMT_RAT_N(xt)) || bnwidthu(getbig(xp)) <= DBL_MANT_DIG) &&
      (isfix(NUMT_RAT_D(xt)) || bnwidthu(getbig(xp+1)) <= DBL_MANT_DIG)) {
    return inttod(NUMT_RAT_N(xt), xp) / inttod(NUMT_RAT_D(xt), xp+1);
  } else { /* use bignum algorithm */
    bignum_t *n = isfix(NUMT_RAT_N(xt)) ? lltobn(getfix(xp)) : getbig(xp);
    bignum_t *d = isfix(NUMT_RAT_D(xt)) ? lltobn(getfix(xp+1)) : getbig(xp+1);
    double r = bnrtod(n, d);
    if (isfix(NUMT_RAT_N(xt))) bnfree(n);
    if (isfix(NUMT_RAT_D(xt))) bnfree(d);
    return r;
  }
}

/* strtod via strtorat, rattod (not used?) */
static double strtoratd(const char *str, char **endp, int radix)
{
  double res;
  nump_t tp[2]; numt_t tt = strtorat(tp, str, endp, radix);
  if (!tt || errno) res = HUGE_VAL-HUGE_VAL; /* NaN */
  else res = rattod(tt, tp);
  numfini(tt, tp);
  return res;
}

/* (rat)d -- returns NUMT_NONE and sets errno on failure */
static numt_t dtorat(nump_t *zp, double x)
{
  if (x != x || fabs(x) >= HUGE_VAL) { /* exclude nans, infinities */
    return setfail(EDOM);
  } else if (x == 0.0) {
    return setfix(zp, 0);
  } else {
    numt_t zt; 
    numt_t mt; nump_t mp[1]; numt_t nt; nump_t np[1];
    double f; int64_t ml;
    int e, m = FLT_RADIX*DBL_MANT_DIG/2, ame; 
    assert(m < sizeof(int64_t)*CHAR_BIT);
    f = frexp(x, &e); ml = (int64_t)ldexp(f, m);
    if (ml >= FIXNUM_MIN && ml <= FIXNUM_MAX) mt = setfix(mp, (long)ml);
    else mt = setbig(mp, lltobn(ml));
    if (m > e) ame = m-e; else ame = e-m;
    if (ame < FIXNUM_WIDTH-1) nt = setfix(np, 1L << ame);
    else nt = setbig(np, bnashll(bn1, ame));
    if (m > e) zt = ratdiv(zp, mt, mp, nt, np);
    else zt = intmul(zp, mt, mp, nt, np);
    numfini(nt, np), numfini(mt, mp);
    return zt; 
  }
}


/* generic exact complex arithmetics */

/* in normal form, only numbers with non-0 imaginary paer are represented as 4-slot numbers */

#define NUMT_IS_RATORNONE(nt) (NUMT_IS_RATNUM(nt) || (nt) == NUMT_NONE)
#define NUMT_IS_RECTNUM(nt) (NUMT_IS_RATNUM(NUMT_COM_R(nt)) && NUMT_IS_RATORNONE(NUMT_COM_I(nt)))
#define isrect(nt) (NUMT_COM_R(nt) != NUMT_FLO && NUMT_COM_I(nt) != NUMT_FLO)

/* these macros assume RATNUM arguments */
#define israt(xt)     (NUMT_IS_RATNUM(xt))

/* x == y */
static int recteq(numt_t xt, const nump_t *xp, numt_t yt, const nump_t *yp)
{
  assert(NUMT_IS_RECTNUM(xt) && "non-exact-complex number");
  assert(NUMT_IS_RECTNUM(yt) && "non-exact-complex number");
  if (israt(xt) && israt(yt)) {
    return rateq(xt, xp, yt, yp);
  } else if (!israt(xt) && !israt(yt)) {
    return rateq(NUMT_COM_R(xt), xp, NUMT_COM_R(yt), yp) && rateq(NUMT_COM_I(xt), xp+2, NUMT_COM_I(yt), yp+2);
  } else { /* ratnums and rectnums are disjoint */
    return 0;
  }
}

/* z = -x */
static numt_t rectneg(nump_t *zp, numt_t xt, const nump_t *xp)
{
  assert(NUMT_IS_RECTNUM(xt) && "non-exact-complex number");
  if (israt(xt)) {
    return ratneg(zp, xt, xp);
  } else {
    return NUMT_MKCOM(ratneg(zp, NUMT_COM_R(xt), xp), ratneg(zp+2, NUMT_COM_I(xt), xp+2));
  }
}

/* z = x + y */
static numt_t rectadd(nump_t *zp, numt_t xt, const nump_t *xp, numt_t yt, const nump_t *yp)
{
  assert(NUMT_IS_RECTNUM(xt) && "non-exact-complex number");
  assert(NUMT_IS_RECTNUM(yt) && "non-exact-complex number");
  if (israt(xt) && israt(yt)) {
    return ratadd(zp, xt, xp, yt, yp);
  } else if (isfix(xt) && getfix(xp) == 0) {
    return numdup(zp, yt, yp);
  } else if (isfix(yt) && getfix(yp) == 0) {
    return numdup(zp, xt, xp);
  } else {
    nump_t zero; numt_t zt;
    numt_t rxt = NUMT_COM_R(xt); const nump_t *rxp;
    numt_t ixt = NUMT_COM_I(xt); const nump_t *ixp;
    numt_t ryt = NUMT_COM_R(yt); const nump_t *ryp;
    numt_t iyt = NUMT_COM_I(yt); const nump_t *iyp;
    setfix(&zero, 0);
    rxp = xp; if (ixt) ixp = xp+2; else ixt = NUMT_FIX, ixp = &zero; 
    ryp = yp; if (iyt) iyp = yp+2; else iyt = NUMT_FIX, iyp = &zero; 
    zt = NUMT_MKCOM(ratadd(zp, rxt, rxp, ryt, ryp), ratadd(zp+2, ixt, ixp, iyt, iyp));
    if (isfix(NUMT_COM_I(zt)) && getfix(zp+2) == 0) zt = NUMT_COM_R(zt);
    return zt;
  }
}

/* z = x - y */
static numt_t rectsub(nump_t *zp, numt_t xt, const nump_t *xp, numt_t yt, const nump_t *yp)
{
  assert(NUMT_IS_RECTNUM(xt) && "non-exact-complex number");
  assert(NUMT_IS_RECTNUM(yt) && "non-exact-complex number");
  if (israt(xt) && israt(yt)) {
    return ratsub(zp, xt, xp, yt, yp);
  } else if (isfix(xt) && getfix(xp) == 0) {
    return rectneg(zp, yt, yp);
  } else if (isfix(yt) && getfix(yp) == 0) {
    return numdup(zp, xt, xp);
  } else {
    nump_t zero; numt_t zt;
    numt_t rxt = NUMT_COM_R(xt); const nump_t *rxp;
    numt_t ixt = NUMT_COM_I(xt); const nump_t *ixp;
    numt_t ryt = NUMT_COM_R(yt); const nump_t *ryp;
    numt_t iyt = NUMT_COM_I(yt); const nump_t *iyp;
    setfix(&zero, 0);
    rxp = xp; if (ixt) ixp = xp+2; else ixt = NUMT_FIX, ixp = &zero; 
    ryp = yp; if (iyt) iyp = yp+2; else iyt = NUMT_FIX, iyp = &zero; 
    zt = NUMT_MKCOM(ratsub(zp, rxt, rxp, ryt, ryp), ratsub(zp+2, ixt, ixp, iyt, iyp));
    if (isfix(NUMT_COM_I(zt)) && getfix(zp+2) == 0) zt = NUMT_COM_R(zt);
    return zt;
  }
}

/* z = x * y */
static numt_t rectmul(nump_t *zp, numt_t xt, const nump_t *xp, numt_t yt, const nump_t *yp)
{
  assert(NUMT_IS_RECTNUM(xt) && "non-exact-complex number");
  assert(NUMT_IS_RECTNUM(yt) && "non-exact-complex number");
  if (isfix(xt) && getfix(xp) == 0) {
    return setfix(zp, 0);
  } else if (isfix(yt) && getfix(yp) == 0) {
    return setfix(zp, 0);
  } if (isfix(xt) && getfix(xp) == 1) {
    return numdup(zp, yt, yp);
  } else if (isfix(yt) && getfix(yp) == 1) {
    return numdup(zp, xt, xp);
  } else if (israt(xt) && israt(yt)) {
    return ratmul(zp, xt, xp, yt, yp);
  } else {
    numt_t zt = NUMT_NONE;
    numt_t rxt; nump_t rxp[2]; numt_t ixt; nump_t ixp[2]; /* owned by caller */
    numt_t ryt; nump_t ryp[2]; numt_t iyt; nump_t iyp[2]; /* owned by caller */
    numt_t m1t; nump_t m1p[2]; numt_t m2t; nump_t m2p[2]; /* new */
    numt_t m3t; nump_t m3p[2]; numt_t m4t; nump_t m4p[2]; /* new */
    rxt = NUMT_COM_R(xt), rxp[0] = xp[0], rxp[1] = xp[1];
    if (!(ixt = NUMT_COM_I(xt))) ixt = setfix(ixp, 0); else ixp[0] = xp[2], ixp[1] = xp[3];
    ryt = NUMT_COM_R(yt), ryp[0] = yp[0], ryp[1] = yp[1];
    if (!(iyt = NUMT_COM_I(yt))) iyt = setfix(iyp, 0); else iyp[0] = yp[2], iyp[1] = yp[3];
    m1t = ratmul(m1p, rxt, rxp, ryt, ryp), m2t = ratmul(m2p, ixt, ixp, iyt, iyp);
    m3t = ratmul(m3p, rxt, rxp, iyt, iyp), m4t = ratmul(m4p, ixt, ixp, ryt, ryp);
    zt = NUMT_MKCOM(ratsub(zp, m1t, m1p, m2t, m2p), ratadd(zp+2, m3t, m3p, m4t, m4p));
    if (isfix(NUMT_COM_I(zt)) && getfix(zp+2) == 0) zt = NUMT_COM_R(zt);
    numfini(m1t, m1p), numfini(m2t, m2p);
    numfini(m3t, m3p), numfini(m4t, m4p);
    return zt;
  }
}

/* z = x / y */
static numt_t rectdiv(nump_t *zp, numt_t xt, const nump_t *xp, numt_t yt, const nump_t *yp)
{
  assert(NUMT_IS_RECTNUM(xt) && "non-exact-complex number");
  assert(NUMT_IS_RECTNUM(yt) && "non-exact-complex number");
  if (isfix(yt) && getfix(yp) == 0) {
    return setfail(EDOM);
  } else if (isfix(yt) && getfix(yp) == 1) {
    return numdup(zp, xt, xp);
  } else if (israt(xt) && israt(yt)) {
    return ratdiv(zp, xt, xp, yt, yp);
  } else {
    numt_t zt = NUMT_NONE;
    numt_t rxt; nump_t rxp[2]; numt_t ixt; nump_t ixp[2]; /* owned by caller */
    numt_t ryt; nump_t ryp[2]; numt_t iyt; nump_t iyp[2]; /* owned by caller */
    numt_t art; nump_t arp[2]; numt_t ait; nump_t aip[2]; /* new */
    numt_t mrt; nump_t mrp[2]; numt_t mit; nump_t mip[2]; /* new */
    numt_t mdt; nump_t mdp[2]; /* new */
    numt_t mt; nump_t mp[2]; numt_t dt; nump_t dp[2]; /* new */
    numt_t rt; nump_t rp[2]; numt_t it; nump_t ip[2]; /* new */
    rxt = NUMT_COM_R(xt), rxp[0] = xp[0], rxp[1] = xp[1];
    if (!(ixt = NUMT_COM_I(xt))) ixt = setfix(ixp, 0); else ixp[0] = xp[2], ixp[1] = xp[3];
    ryt = NUMT_COM_R(yt), ryp[0] = yp[0], ryp[1] = yp[1];
    if (!(iyt = NUMT_COM_I(yt))) iyt = setfix(iyp, 0); else iyp[0] = yp[2], iyp[1] = yp[3];
    art = ratabs(arp, ryt, ryp), ait = ratabs(aip, iyt, iyp);
    if (ratless(ait, aip, art, arp)) {
      mt = ratdiv(mp, iyt, iyp, ryt, ryp);
      mdt = ratmul(mdp, mt, mp, iyt, iyp), dt = ratadd(dp, ryt, ryp, mdt, mdp);
      mrt = ratmul(mrp, mt, mp, ixt, ixp), rt = ratadd(rp, rxt, rxp, mrt, mrp);
      mit = ratmul(mip, mt, mp, rxt, rxp), it = ratsub(ip, ixt, ixp, mit, mip); 
    } else {
      mt = ratdiv(mp, ryt, ryp, iyt, iyp);
      mdt = ratmul(mdp, mt, mp, ryt, ryp), dt = ratadd(dp, iyt, iyp, mdt, mdp);
      mrt = ratmul(mrp, mt, mp, rxt, rxp), rt = ratadd(rp, ixt, ixp, mrt, mrp);
      mit = ratmul(mip, mt, mp, ixt, ixp), it = ratsub(ip, mit, mip, rxt, rxp); 
    }
    zt = NUMT_MKCOM(ratdiv(zp, rt, rp, dt, dp), ratdiv(zp+2, it, ip, dt, dp));
    if (isfix(NUMT_COM_I(zt)) && getfix(zp+2) == 0) zt = NUMT_COM_R(zt);
    numfini(art, arp), numfini(ait, aip);
    numfini(mrt, mrp), numfini(mit, mip);
    numfini(mdt, mdp);
    numfini(mt, mp), numfini(dt, dp);
    numfini(rt, rp), numfini(it, ip);
    return zt;
  }
}

/* returns NUMT_NONE and sets errno on failure (ERANGE is special!) */
static numt_t strtorect(nump_t *zp, const char *str, char **endp, int radix)
{
  numt_t zt = NUMT_NONE; char *ep = NULL;
  assert(str); assert(radix >= 2 && radix <= 36);
  errno = 0;
  if (str[0] == '+' && tolower(str[1]) == 'i' && tolower(str[2]) != 'n') {
    zt = NUMT_MKCOM(setfix(zp, 0), setfix(zp+2, 1));
    ep = (char*)str + 2; /* skip '+i' */
  } else if (str[0] == '-' && tolower(str[1]) == 'i' && tolower(str[2]) != 'n') {
    zt = NUMT_MKCOM(setfix(zp, 0), setfix(zp+2, -1));
    ep = (char*)str + 2; /* skip '-i' */
  } else {
    zt = strtorat(zp, str, &ep, radix);
    if (isflo(zt) && errno == ERANGE) errno = 0; 
    if (!zt || errno) {
      numfini(zt, zp);
      zt = setfail(EDOM);
    } else if (ep && tolower(*ep) == 'i') {
      numt_t it = nummove(zp+2, zt, zp);
      if (isflo(it)) zt = NUMT_MKCOM(setflo(zp, 0.0), it);
      else zt = NUMT_MKCOM(setfix(zp, 0), it);
      ep += 1; /* skip 'i' */
    } else if (ep && (*ep == '+' || *ep == '-')) {
      nump_t rp[2]; numt_t rt = nummove(rp, zt, zp);
      nump_t ip[2]; numt_t it;      
      if (tolower(ep[1]) == 'i' && tolower(ep[2]) != 'n') {
        nummove(zp, rt, rp);
        if (isflo(rt)) zt = NUMT_MKCOM(rt, setflo(zp+2, (*ep == '+') ? 1.0 : -1.0));
        else zt = NUMT_MKCOM(rt, setfix(zp+2, (*ep == '+') ? 1 : -1));
        ep += 2; /* skip '+i' or '-i' */
      } else { /* full imaginary part */
        it = strtorat(ip, ep, &ep, radix);
        if (isflo(it) && errno == ERANGE) errno = 0; /* ok here */
        if (errno || !ep || tolower(ep[0]) != 'i') {
          numfini(rt, rp); numfini(it, ip); 
          zt = NUMT_NONE;
          errno = EDOM;
        } else {  
          if (isflo(rt) && !isflo(it)) {
            zt = NUMT_MKCOM(setflo(zp, getflo(rp)), setflo(zp+2, rattod(it, ip)));
            numfini(it, ip);
          } else if (!isflo(rt) && isflo(it)) {
            zt = NUMT_MKCOM(setflo(zp, rattod(rt, rp)), setflo(zp+2, getflo(ip)));
            numfini(rt, rp);
          } else {
            zt = NUMT_MKCOM(nummove(zp, rt, rp), nummove(zp+2, it, ip));
          }
          if (isfix(NUMT_COM_I(zt)) && getfix(zp+2) == 0) zt = NUMT_COM_R(zt);
          ep += 1; /* skip 'i' */
        }
      }
    } else if (ep && *ep == '@') {
      nump_t ap[2]; numt_t at = strtorat(ap, ep+1, &ep, radix);
      if (isflo(at) && errno == ERANGE) errno = 0; /* ok here */
      if (!at || errno || !ep) {
        numfini(zt, zp); numfini(at, ap);
        zt = NUMT_NONE;
        errno = EDOM;
      } else if (isfix(at) && getfix(ap) == 0) {
        /* real */
      } else {
        double m = isflo(zt) ? getflo(zp) : rattod(zt, zp);
#ifndef COMPACT_RATTRIG
        numfini(zt, zp);
        if (isflo(at)) {
          double a = getflo(ap);
          zt = NUMT_MKCOM(setflo(zp, m*cos(a)), setflo(zp+2, m*sin(a)));
        } else {
          double sa, ca; ratsincos(at, ap, &sa, &ca);
          zt = NUMT_MKCOM(setflo(zp, m*ca), setflo(zp+2, m*sa));
        }
#else        
        double a = isflo(at) ? getflo(ap) : rattod(at, ap);
        numfini(zt, zp); numfini(at, ap);
        zt = NUMT_MKCOM(setflo(zp, m*cos(a)), setflo(zp+2, m*sin(a)));
#endif
      }
    } else { 
      /* real */
    }
  }
  if (endp) *endp = ep;
  if (!isrect(zt)) errno = ERANGE;
  return zt;
}

/* # of chars needed for x in radix, including '+', '-', 'i', and '\0' */
static size_t rectfmtsize(numt_t xt, const nump_t *xp, int radix)
{
  assert(radix >= 2 && radix <= 36);
  assert(NUMT_IS_RECTNUM(xt) && "non-exact-complex number");
  if (israt(xt)) {
    return ratfmtsize(xt, xp, radix);
  } else {
    /* note that first intfmtsize include trailing \0 that may be replaced by '+' sign */
    return ratfmtsize(NUMT_COM_R(xt), xp, radix) + ratfmtsize(NUMT_COM_I(xt), xp+2, radix) + 1;
  }
}

/* format x into buffer; len should be as calculated by rectfmtsize;
 * returns ptr to first char of zero-terminated result in buffer. */
static char *recttostr(char *buffer, size_t len, numt_t xt, const nump_t *xp, int radix)
{
  assert(radix >= 2 && radix <= 36);
  assert(NUMT_IS_RECTNUM(xt) && "non-exact-complex number");
  if (israt(xt)) {
    return rattostr(buffer, len, xt, xp, radix);
  } else { /* NB: rattostr prints right-to-left, so this */
    char *sep = rattostr(buffer, len-1, NUMT_COM_I(xt), xp+2, radix);
    assert(buffer[len-2] == 0); buffer[len-2] = 'i'; buffer[len-1] = 0;
    if (*sep != '-') *--sep = '+';
    if (sep[1] == '1' && sep[2] == 'i') sep[1] = 'i', sep[2] = 0;
    if (isfix(NUMT_COM_R(xt)) && getfix(xp) == 0) {
      return sep;
    } else {
      int sc = *sep;
      char *res = rattostr(buffer, sep+1-buffer, NUMT_COM_R(xt), xp, radix);
      assert(*sep == 0); *sep = sc;
      return res;
    }
  }
}

/* (double,double)x */
static void recttodd(numt_t xt, const nump_t *xp, double *prd, double *pid)
{
  assert(NUMT_IS_RECTNUM(xt) && "non-exact-complex number");
  assert(prd && pid);
  if (israt(xt)) {
    *prd = rattod(xt, xp), *pid = 0.0;
  } else {
    *prd = rattod(NUMT_COM_R(xt), xp), *pid = rattod(NUMT_COM_I(xt), xp+2);
  }
}


/* inexact complex arithmetics */

/* results of real-closed operations are real, otherwise complex (even if im part is 0.0) */

#define NUMT_IS_FLOORNONE(nt) ((nt) == NUMT_FLO || (nt) == NUMT_NONE)
#define NUMT_IS_COMPNUM(nt) (NUMT_COM_R(nt) == NUMT_FLO && NUMT_IS_FLOORNONE(NUMT_COM_I(nt)))
#define iscomp(nt) (NUMT_COM_R(nt) == NUMT_FLO && NUMT_COM_I(nt) == NUMT_FLO)

/* x == y */
static int compeq(numt_t xt, const nump_t *xp, numt_t yt, const nump_t *yp)
{
  assert(NUMT_IS_COMPNUM(xt) && "non-inexact-complex number");
  assert(NUMT_IS_COMPNUM(yt) && "non-inexact-complex number");
  if (isflo(xt) && isflo(yt)) {
    return getflo(xp) == getflo(yp);
  } else {
    double rx = getflo(xp), ix = NUMT_COM_I(xt) ? getflo(xp+2) : 0.0;
    double ry = getflo(yp), iy = NUMT_COM_I(yt) ? getflo(yp+2) : 0.0;
    return rx == ry && ix == iy;
  }
}

/* z = -x */
static numt_t compneg(nump_t *zp, numt_t xt, const nump_t *xp)
{
  assert(NUMT_IS_COMPNUM(xt) && "non-inexact-complex number");
  if (isflo(xt)) {
    return setflo(zp, -getflo(xp));
  } else {
    return NUMT_MKCOM(setflo(zp, -getflo(xp)), setflo(zp+2, -getflo(xp+2)));
  }
}

#if 0 /* cfl ops candidates */

/* z = x + y */
static numt_t compadd(nump_t *zp, numt_t xt, const nump_t *xp, numt_t yt, const nump_t *yp)
{
  assert(NUMT_IS_COMPNUM(xt) && "non-exact-complex number");
  assert(NUMT_IS_COMPNUM(yt) && "non-exact-complex number");
  if (isflo(xt) && isflo(yt)) {
    return setflo(zp, getflo(xp) + getflo(yp));
  } else {
    double rx = getflo(xp), ix = NUMT_COM_I(xt) ? getflo(xp+2) : 0.0;
    double ry = getflo(yp), iy = NUMT_COM_I(yt) ? getflo(yp+2) : 0.0;
    return NUMT_MKCOM(setflo(zp, rx+ry), setflo(zp+2, ix+iy));
  }
}

/* z = x - y */
static numt_t compsub(nump_t *zp, numt_t xt, const nump_t *xp, numt_t yt, const nump_t *yp)
{
  assert(NUMT_IS_COMPNUM(xt) && "non-exact-complex number");
  assert(NUMT_IS_COMPNUM(yt) && "non-exact-complex number");
  if (isflo(xt) && isflo(yt)) {
    return setflo(zp, getflo(xp) - getflo(yp));
  } else {
    double rx = getflo(xp), ix = NUMT_COM_I(xt) ? getflo(xp+2) : 0.0;
    double ry = getflo(yp), iy = NUMT_COM_I(yt) ? getflo(yp+2) : 0.0;
    return NUMT_MKCOM(setflo(zp, rx-ry), setflo(zp+2, ix-iy));
  }
}

/* z = x * y */
static numt_t compmul(nump_t *zp, numt_t xt, const nump_t *xp, numt_t yt, const nump_t *yp)
{
  assert(NUMT_IS_COMPNUM(xt) && "non-exact-complex number");
  assert(NUMT_IS_COMPNUM(yt) && "non-exact-complex number");
  if (isflo(xt) && isflo(yt)) {
    return setflo(zp, getflo(xp) * getflo(yp));
  } else {
    double rx = getflo(xp), ix = NUMT_COM_I(xt) ? getflo(xp+2) : 0.0;
    double ry = getflo(yp), iy = NUMT_COM_I(yt) ? getflo(yp+2) : 0.0;
    return NUMT_MKCOM(setflo(zp, rx*ry - ix*iy), setflo(zp+2, rx*iy + ix*ry));
  }
}

/* z = x / y */
static numt_t compdiv(nump_t *zp, numt_t xt, const nump_t *xp, numt_t yt, const nump_t *yp)
{
  assert(NUMT_IS_COMPNUM(xt) && "non-exact-complex number");
  assert(NUMT_IS_COMPNUM(yt) && "non-exact-complex number");
  if (isflo(xt) && isflo(yt)) {
    return setflo(zp, getflo(xp) / getflo(yp));
  } else {
    double rx = getflo(xp), ix = NUMT_COM_I(xt) ? getflo(xp+2) : 0.0;
    double ry = getflo(yp), iy = NUMT_COM_I(yt) ? getflo(yp+2) : 0.0;
    double d = ry*ry + iy*iy;
    return NUMT_MKCOM(setflo(zp, (rx*ry + ix*iy)/d), setflo(zp+2, (ix*ry - rx*iy)/d));
  }
}

#endif /* cfl ops candidates */

/* returns NUMT_NONE and sets errno on failure */
static numt_t strtoflo(nump_t *zp, const char *str, char **endp, int radix)
{
  double d; char *e, *t; numt_t zt = NUMT_NONE;
  errno = 0; 
  /* check for infnans first */
  if (endp) *endp = (char*)str + 6;
  if (0 == strncmp_ci(str, "+inf.0", 6)) return setflo(zp, HUGE_VAL);
  if (0 == strncmp_ci(str, "-inf.0", 6)) return setflo(zp, -HUGE_VAL);
  if (0 == strncmp_ci(str, "+nan.0", 6)) return setflo(zp, HUGE_VAL-HUGE_VAL);
  if (0 == strncmp_ci(str, "-nan.0", 6)) return setflo(zp, HUGE_VAL-HUGE_VAL);
  /* check for a ratio (in radix) */
  e = (char*)str; if (*e == '-' || *e == '+') ++e;
  if ((t = skip_uinteger(e, radix))[0] == '/') {
    zt = strtorat(zp, str, endp, radix);
    if (isflo(zt) && errno == ERANGE) return errno = 0, zt;
    else if (!zt || errno) return setfail(EDOM);
    d = rattod(zt, zp);
    numfini(zt, zp);
    return setflo(zp, d);
  }
  /* not a ratio: either intnum or inexact (b,o,x)decimal */
  if (radix == 2 || radix == 4 || radix == 8 || radix == 10 || radix == 16) {
    d = strtodn(str, radix, endp); /* will handle ints too */
    if (errno == ERANGE) errno = 0; /* overflows and underflows are ok */
    if (errno) return setfail(EDOM); 
  } else { /* intnum only, arbitrary radix */
    double zero = (*str == '-') ? -0.0 : 0.0; /* NB: -0! */ 
    zt = strtoint(zp, str, endp, radix);
    if (!zt) return setfail(EDOM);
    if (isfix(zt) && getfix(zp) == 0) d = zero;
    else d = inttod(zt, zp); /* may overflow, but it's ok */
  }
  numfini(zt, zp);
  return setflo(zp, d);
}

/* returns NUMT_NONE and sets errno on failure */
static numt_t strtocomp(nump_t *zp, const char *str, char **endp, int radix)
{
  numt_t zt = NUMT_NONE; char *ep = NULL;
  assert(str); assert(radix >= 2 && radix <= 36);
  errno = 0;
  if (str[0] == '+' && tolower(str[1]) == 'i' && tolower(str[2]) != 'n') {
    zt = NUMT_MKCOM(setflo(zp, 0.0), setflo(zp+2, 1.0));
    ep = (char*)str + 2; /* skip '+i' */
  } else if (str[0] == '-' && tolower(str[1]) == 'i' && tolower(str[2]) != 'n') {
    zt = NUMT_MKCOM(setflo(zp, 0.0), setflo(zp+2, -1.0));
    ep = (char*)str + 2; /* skip '-i' */
  } else {
    zt = strtoflo(zp, str, &ep, radix);
    if (!zt || errno) {
      if (zt) numfini(zt, zp);
      zt = setfail(EDOM);
    } else if (ep && tolower(*ep) == 'i') {
      numt_t it = nummove(zp+2, zt, zp);
      zt = NUMT_MKCOM(setflo(zp, 0.0), it);
      ep += 1; /* skip 'i' */
    } else if (ep && (*ep == '+' || *ep == '-')) {
      nump_t rp[2]; numt_t rt = nummove(rp, zt, zp);
      nump_t ip[2]; numt_t it; 
      if (tolower(ep[1]) == 'i' && tolower(ep[2]) != 'n') {
        nummove(zp, rt, rp);
        zt = NUMT_MKCOM(rt, setflo(zp+2, (*ep == '+') ? 1.0 : -1.0));
        ep += 2; /* skip '+i' or '-i' */
      } else {
        it = strtoflo(ip, ep, &ep, radix);
        if (errno || !ep || tolower(ep[0]) != 'i') {
          numfini(rt, rp); numfini(it, ip); 
          zt = setfail(EDOM);
        } else {
          zt = NUMT_MKCOM(nummove(zp, rt, rp), nummove(zp+2, it, ip));
          if (isfix(NUMT_COM_I(zt)) && getfix(zp+2) == 0) zt = NUMT_COM_R(zt);
          ep += 1; /* skip 'i' */
        }
      }
    } else if (ep && *ep == '@' && !parse_zero(ep+1, &ep)) {
      nump_t ap[2]; numt_t at = strtoflo(ap, ep+1, &ep, radix);
      if (!at || errno || !isflo(zt) || !isflo(at)) {
        numfini(zt, zp); numfini(at, ap);
        zt = setfail(errno || EDOM);
      } else {
        double m = getflo(zp), a = getflo(ap);
        numfini(zt, zp); numfini(at, ap);
        zt = NUMT_MKCOM(setflo(zp, m*cos(a)), setflo(zp+2, m*sin(a)));
      }
    } else { 
      /* real */
    }
  }
  if (endp) *endp = ep;
  return zt;
}

/* # of chars needed for x in radix; return 0 if radix != 10  */
static size_t compfmtsize(numt_t xt, const nump_t *xp, int radix, int prc)
{
  assert(radix >= 2 && radix <= 36);
  assert(NUMT_IS_COMPNUM(xt) && "non-exact-complex number");
  return isflo(xt) ? dnfmtsize(radix, prc) : dnfmtsize(radix, prc)*2 + 2;
}

/* format x into buffer; len should be as calculated by compfmtsize;
 * returns ptr to first char of zero-terminated result in buffer
 * or NULL if an inexact number is printed in non-10 radix. */
static char *comptostr(char *buffer, size_t len, numt_t xt, const nump_t *xp, int radix, int mode, int prc)
{
  assert(NUMT_IS_COMPNUM(xt) && "non-exact-complex number");
  if (isflo(xt)) { /* may return NULL on wrong radix */
    return dntostr(buffer, len, getflo(xp), radix, mode, prc);
  } else { /* dntostr prints left-to-right */
    char *buf = dntostr(buffer, len/2, getflo(xp), radix, mode, prc);
    char *sep = buf ? buf + strlen(buf) : NULL;
    assert(!buf || buf == buffer);
    if (!buf) return NULL;
    if (dnsignless(getflo(xp+2))) *sep++ = '+';
    buf = dntostr(sep, len/2, getflo(xp+2), radix, mode, prc);
    assert(!buf || buf == sep);
    if (!buf) return NULL;
    sep = buf + strlen(buf);
    *sep++ = 'i'; *sep = 0;
    assert(sep-buffer < (int)len);
    return buffer;
  }
}

/* (double,double)x */
static void comptodd(numt_t xt, const nump_t *xp, double *prd, double *pid)
{
  assert(NUMT_IS_COMPNUM(xt) && "non-exact-complex number");
  assert(prd && pid);
  if (isflo(xt)) {
    *prd = getflo(xp), *pid = 0.0;
  } else {
    *prd = getflo(xp), *pid = getflo(xp+2);
  }
}


/* mixed-exactness real numbers */

/* always in normal form, take 1 or 2 slots */

#define NUMT_IS_REALNUM(nt) ((nt) == NUMT_FLO || NUMT_IS_RATNUM(nt))
#define isreal(nt) NUMT_IS_REALNUM(nt) 

/* x == y */
int realeq(numt_t xt, const nump_t *xp, numt_t yt, const nump_t *yp)
{
  long lx, ly; double dx, dy; 
  assert(NUMT_IS_REALNUM(xt) && "non-real number");
  assert(NUMT_IS_REALNUM(yt) && "non-real number");
  if (isfix(xt) && isfix(yt)) { /* fast track */
    lx = getfix(xp), ly = getfix(yp); 
    return lx == ly;
  } else if (isflo(xt) && isflo(yt)) { /* fast track */
    dx = getflo(xp), dy = getflo(yp); 
    return dx == dy;
  } else if (isflo(xt) || isflo(yt)) {
    if (isfix(xt) || isfix(yt)) { /* fast track */
      dx = isfix(xt) ? getfix(xp) : getflo(xp); 
      dy = isfix(yt) ? getfix(yp) : getflo(yp);
      return dx == dy;
    } else { /* slow track */
      if (isflo(xt)) { /* yt is a ratio */
        bignumll_t nyll, dyll; double d = getflo(xp);
        numt_t nyt = NUMT_RAT_N(yt), dyt = NUMT_RAT_D(yt);
        bignum_t *ny = isbig(nyt) ? getbig(yp)   : bnx_makell(&nyll, getfix(yp));
        bignum_t *dy = isbig(dyt) ? getbig(yp+1) : bnx_makell(&dyll, dyt ? getfix(yp+1) : 1);
        return d == d && bnrdcmp(ny, dy, d) == 0;
      } else if (isflo(yt)) { /* xt is a ratio */
        bignumll_t nxll, dxll; double d = getflo(yp);
        numt_t nxt = NUMT_RAT_N(xt), dxt = NUMT_RAT_D(xt);
        bignum_t *nx = isbig(nxt) ? getbig(xp)   : bnx_makell(&nxll, getfix(xp));
        bignum_t *dx = isbig(dxt) ? getbig(xp+1) : bnx_makell(&dxll, dxt ? getfix(xp+1) : 1);
        return d == d && bnrdcmp(nx, dx, d) == 0;
      } else { /* can't happen */
        assert(0); return 0;
      }
    }
  } else {
    return rateq(xt, xp, yt, yp);
  }
}

/* cmp(x, y) == c */
int realcmpc(numt_t xt, const nump_t *xp, numt_t yt, const nump_t *yp, ncmp_t c)
{
  long lx, ly; double dx, dy; int cmp;
  assert(NUMT_IS_REALNUM(xt) && "non-real number");
  assert(NUMT_IS_REALNUM(yt) && "non-real number");
  if (isfix(xt) && isfix(yt)) { /* fast track */
    lx = getfix(xp), ly = getfix(yp); 
    goto cmpl;
  } else if (isflo(xt) && isflo(yt)) { /* fast track */
    dx = getflo(xp), dy = getflo(yp); 
    goto cmpd;
  } else if (isflo(xt) || isflo(yt)) {
    if (isfix(xt) || isfix(yt)) { /* fast track */
      dx = isfix(xt) ? getfix(xp) : getflo(xp); 
      dy = isfix(yt) ? getfix(yp) : getflo(yp);
      goto cmpd;
    } else { /* slow track */
      if (isflo(xt)) { /* yt is a ratio */
        bignumll_t nyll, dyll; double d = getflo(xp);
        numt_t nyt = NUMT_RAT_N(yt), dyt = NUMT_RAT_D(yt);
        bignum_t *ny = isbig(nyt) ? getbig(yp)   : bnx_makell(&nyll, getfix(yp));
        bignum_t *dy = isbig(dyt) ? getbig(yp+1) : bnx_makell(&dyll, dyt ? getfix(yp+1) : 1);
        if (d != d) return 0; /* oops -- NaN! */
        cmp = -bnrdcmp(ny, dy, d);
        goto cmp;
      } else if (isflo(yt)) { /* xt is a ratio */
        bignumll_t nxll, dxll; double d = getflo(yp);
        numt_t nxt = NUMT_RAT_N(xt), dxt = NUMT_RAT_D(xt);
        bignum_t *nx = isbig(nxt) ? getbig(xp)   : bnx_makell(&nxll, getfix(xp));
        bignum_t *dx = isbig(dxt) ? getbig(xp+1) : bnx_makell(&dxll, dxt ? getfix(xp+1) : 1);
        if (d != d) return 0; /* oops -- NaN! */
        cmp = bnrdcmp(nx, dx, d);
        goto cmp;
      }
    }
  } else {
    goto cmpr;
  }
cmpr:
  cmp = ratcmp(xt, xp, yt, yp);
cmp:
  switch (c) { 
    case NCMP_LT: return cmp < 0;
    case NCMP_LE: return cmp <= 0;
    case NCMP_EQ: return cmp == 0;
    case NCMP_GE: return cmp >= 0;
    case NCMP_GT: return cmp > 0;
    default: assert(0); return 0;
  }
cmpl:
  switch (c) { 
    case NCMP_LT: return lx < ly;
    case NCMP_LE: return lx <= ly;
    case NCMP_EQ: return lx == ly;
    case NCMP_GE: return lx >= ly;
    case NCMP_GT: return lx > ly;
    default: assert(0); return 0;
  }
cmpd:
  switch (c) { 
    case NCMP_LT: return dx < dy;
    case NCMP_LE: return dx <= dy;
    case NCMP_EQ: return dx == dy;
    case NCMP_GE: return dx >= dy;
    case NCMP_GT: return dx > dy;
    default: assert(0); return 0;
  }
}

/* cmp(x, 0) == c */
static int realcmp0(numt_t xt, const nump_t *xp, ncmp_t c)
{
  long lx; double dx;
  assert(NUMT_IS_REALNUM(xt) && "non-real number");
  if (isfix(xt)) {
    lx = getfix(xp); 
    goto cmpl;
  } else if (isflo(xt)) {
    dx = getflo(xp); 
    goto cmpd;
  } else {
    lx = ratsign(xt, xp);
    goto cmpl;
  }
cmpd:
  switch (c) { 
    case NCMP_LT: return dx < 0.0;
    case NCMP_LE: return dx <= 0.0;
    case NCMP_EQ: return dx == 0.0;
    case NCMP_GE: return dx >= 0.0;
    case NCMP_GT: return dx > 0.0;
    default: assert(0); return 0;
  }
cmpl:
  switch (c) {
    case NCMP_LT: return lx < 0;
    case NCMP_LE: return lx <= 0;
    case NCMP_EQ: return lx == 0;
    case NCMP_GE: return lx >= 0;
    case NCMP_GT: return lx > 0;
    default: assert(0); return 0;
  }
}

/* z = -x */
static numt_t realneg(nump_t *zp, numt_t xt, const nump_t *xp)
{
  assert(NUMT_IS_REALNUM(xt) && "non-real number");
  if (isflo(xt)) return setflo(zp, -getflo(xp));
  return ratneg(zp, xt, xp);
}

/* z = |x| */
static numt_t realabs(nump_t *zp, numt_t xt, const nump_t *xp)
{
  assert(NUMT_IS_REALNUM(xt) && "non-real number");
  if (isflo(xt)) return setflo(zp, fabs(getflo(xp)));
  return ratabs(zp, xt, xp);
}

/* sign(x) (-1 0 +1) */
static int realsign(numt_t xt, const nump_t *xp)
{
  assert(NUMT_IS_REALNUM(xt) && "non-real number");
  if (isflo(xt)) {
    double d = getflo(xp); 
    if (d > 0.0) return 1;
    if (d < 0.0 || 1.0/d < 0.0) return -1; /* -0 */
    return 0;
  }
  return ratsign(xt, xp);
}


/* z = x + y */
static numt_t realadd(nump_t *zp, numt_t xt, const nump_t *xp, numt_t yt, const nump_t *yp)
{
  assert(NUMT_IS_REALNUM(xt) && "non-real number");
  assert(NUMT_IS_REALNUM(yt) && "non-real number");
  if (isflo(xt) || isflo(yt)) {
    double x = isflo(xt) ? getflo(xp) : rattod(xt, xp);
    double y = isflo(yt) ? getflo(yp) : rattod(yt, yp);
    return setflo(zp, x+y);
  } else {
    return ratadd(zp, xt, xp, yt, yp);
  }
}

/* z = x - y */
static numt_t realsub(nump_t *zp, numt_t xt, const nump_t *xp, numt_t yt, const nump_t *yp)
{
  assert(NUMT_IS_REALNUM(xt) && "non-real number");
  assert(NUMT_IS_REALNUM(yt) && "non-real number");
  if (isflo(xt) || isflo(yt)) {
    double x = isflo(xt) ? getflo(xp) : rattod(xt, xp);
    double y = isflo(yt) ? getflo(yp) : rattod(yt, yp);
    return setflo(zp, x-y);
  } else {
    return ratsub(zp, xt, xp, yt, yp);
  }
}

/* z = x * y */
static numt_t realmul(nump_t *zp, numt_t xt, const nump_t *xp, numt_t yt, const nump_t *yp)
{
  assert(NUMT_IS_REALNUM(xt) && "non-real number");
  assert(NUMT_IS_REALNUM(yt) && "non-real number");
  if (isflo(xt) || isflo(yt)) {
    double x = isflo(xt) ? getflo(xp) : rattod(xt, xp);
    double y = isflo(yt) ? getflo(yp) : rattod(yt, yp);
    return setflo(zp, x*y);
  } else {
    return ratmul(zp, xt, xp, yt, yp);
  }
}

/* z = x / y */
static numt_t realdiv(nump_t *zp, numt_t xt, const nump_t *xp, numt_t yt, const nump_t *yp)
{
  assert(NUMT_IS_REALNUM(xt) && "non-real number");
  assert(NUMT_IS_REALNUM(yt) && "non-real number");
  if (isflo(xt) || isflo(yt)) {
    double x = isflo(xt) ? getflo(xp) : rattod(xt, xp);
    double y = isflo(yt) ? getflo(yp) : rattod(yt, yp);
    return setflo(zp, x/y);
  } else {
    return ratdiv(zp, xt, xp, yt, yp);
  }
}

/* z = floor(x) */
static numt_t realfloor(nump_t *zp, numt_t xt, const nump_t *xp)
{
  assert(NUMT_IS_REALNUM(xt) && "non-real number");
  if (isflo(xt)) return setflo(zp, floor(getflo(xp)));
  else return ratfloor(zp, xt, xp);
}

/* z = ceiling(x) */
static numt_t realceil(nump_t *zp, numt_t xt, const nump_t *xp)
{
  assert(NUMT_IS_REALNUM(xt) && "non-real number");
  if (isflo(xt)) return setflo(zp, ceil(getflo(xp)));
  else return ratceil(zp, xt, xp);
}

/* z = truncate(x) */
numt_t realtrunc(nump_t *zp, numt_t xt, const nump_t *xp)
{
  assert(NUMT_IS_REALNUM(xt) && "non-real number");
  if (isflo(xt)) {
    double i; modf(getflo(xp), &i);
    return setflo(zp, i);
  } else return rattrunc(zp, xt, xp);
}

/* z = round(x) */
static numt_t realround(nump_t *zp, numt_t xt, const nump_t *xp)
{
  assert(NUMT_IS_REALNUM(xt) && "non-real number");
  if (isflo(xt)) {
    double x = getflo(xp), f = floor(x), c = ceil(x), d = x-f, u = c-x, r; 
    if (d == u) r = fmod(f, 2.0) == 0.0 ? f : c;
    else r = (d < u) ? f : c;
    return setflo(zp, r);
  } else return ratround(zp, xt, xp);
}

/* generic mixed-exactness numbers */

/* take from 1 to 4 slots */

#define NUMT_IS_VALID(xt) (NUMT_IS_COMPNUM(xt) || NUMT_IS_RECTNUM(xt))

/* x == y */
static int gnumeqn(numt_t xt, const nump_t *xp, numt_t yt, const nump_t *yp)
{
  assert(NUMT_IS_VALID(xt) && "unsupported number type");
  assert(NUMT_IS_VALID(yt) && "unsupported number type");
  if (isreal(xt) && isreal(yt)) {
    return realeq(xt, xp, yt, yp);
  } else if (isrect(xt) && isrect(yt)) {
    return recteq(xt, xp, yt, yp);
  } else if (iscomp(xt) && iscomp(yt)) {
    return compeq(xt, xp, yt, yp);
  } else { /* mixed case; fixme: fail back on recteq! */
    nump_t xp1[1], yp1[1];
    /* compare real parts (always present) */
    if (!realeq(NUMT_COM_R(xt), xp, NUMT_COM_R(yt), yp)) return 0;
    /* compare imag parts (one but not both may be missing) */
    xt = NUMT_COM_I(xt); if (xt) xp += 2; else xt = setfix((nump_t*)(xp = xp1), 0);   
    yt = NUMT_COM_I(yt); if (yt) yp += 2; else yt = setfix((nump_t*)(yp = yp1), 0);
    return realeq(xt, xp, yt, yp);
  }
}

/* eqv(x,y) */
static int gnumeqv(numt_t xt, const nump_t *xp, numt_t yt, const nump_t *yp)
{
  assert(NUMT_IS_VALID(xt) && "unsupported number type");
  assert(NUMT_IS_VALID(yt) && "unsupported number type");
  if (xt != yt) return 0;
  while (xt) {
    switch (xt & NUMT_SS_MASK) {
      case NUMT_FIX:
        if (getfix(xp) != getfix(yp)) return 0; 
        break;
      case NUMT_BIG:
        if (!bneq(getbig(xp), getbig(yp))) return 0; 
        break;
      case NUMT_FLO: {
        double x = getflo(xp), y = getflo(yp);
        if (x != x && y != y) break; /* all NaNs are eqv? to e.o. */
        if (x != y) return 0; /* definitely different */
        if (!x && !y && 1.0/x != 1.0/y) return 0; /* 0 not eqv? to -0 */
      } break;
    }
    xt >>= 2; ++xp; ++yp;
  }
  return 1; 
}

/* cmp(x, y) == c  [real numbers only] */
static int gnumcmpc(numt_t xt, const nump_t *xp, numt_t yt, const nump_t *yp, ncmp_t c)
{
  assert(NUMT_IS_VALID(xt) && "unsupported number type");
  assert(NUMT_IS_VALID(yt) && "unsupported number type");
  if (!isreal(xt) || !isreal(yt)) return -1;
  return realcmpc(xt, xp, yt, yp, c);
}

/* cmp(x, 0) == c  [internal, real numbers only] */
static int gnumcmp0(numt_t xt, const nump_t *xp, ncmp_t c)
{
  assert(NUMT_IS_VALID(xt) && "unsupported number type");
  if (!isreal(xt)) return -1;
  return realcmp0(xt, xp, c);
}

/* z = max(x, y)  [real numbers only; result is inexact if either arg is] */
static numt_t gnummax(nump_t *zp, numt_t xt, const nump_t *xp, numt_t yt, const nump_t *yp)
{
  assert(NUMT_IS_VALID(xt) && "unsupported number type");
  assert(NUMT_IS_VALID(yt) && "unsupported number type");
  if (!isreal(xt) || !isreal(yt)) return setfail(EDOM);
  /* R7RS: if either arg is inexact, result is inexact */
  if (isflo(xt) || isflo(yt)) {
    double x = isflo(xt) ? getflo(xp) : rattod(xt, xp);
    double y = isflo(yt) ? getflo(yp) : rattod(yt, yp);
    return setflo(zp, x >= y ? x : y);
  } else { /* both exact */
    return ratless(xt, xp, yt, yp) ? numdup(zp, yt, yp) : numdup(zp, xt, xp);
  }
}

/* z = min(x, y)  [real numbers only; result is inexact if either arg is] */
static numt_t gnummin(nump_t *zp, numt_t xt, const nump_t *xp, numt_t yt, const nump_t *yp)
{
  assert(NUMT_IS_VALID(xt) && "unsupported number type");
  assert(NUMT_IS_VALID(yt) && "unsupported number type");
  if (!isreal(xt) || !isreal(yt)) return setfail(EDOM);
  /* R7RS: if either arg is inexact, result is inexact */
  if (isflo(xt) || isflo(yt)) {
    double x = isflo(xt) ? getflo(xp) : rattod(xt, xp);
    double y = isflo(yt) ? getflo(yp) : rattod(yt, yp);
    return setflo(zp, x <= y ? x : y);
  } else { /* both exact */
    return ratless(xt, xp, yt, yp) ? numdup(zp, xt, xp) : numdup(zp, yt, yp);
  }
}

/* z = |x| */
static numt_t gnumabs(nump_t *zp, numt_t xt, const nump_t *xp)
{
  assert(NUMT_IS_VALID(xt) && "unsupported number type");
  if (!isreal(xt)) return setfail(EDOM);
  return realabs(zp, xt, xp);
}

/* z = -x */
static numt_t gnumneg(nump_t *zp, numt_t xt, const nump_t *xp)
{
  assert(NUMT_IS_VALID(xt) && "unsupported number type");
  if (isrect(xt)) {
    return rectneg(zp, xt, xp);
  } else if (isflo(xt)) {
    return setflo(zp, -getflo(xp));
  } else {
    return compneg(zp, xt, xp);
  }
}

/* z = x + y */
static numt_t gnumadd(nump_t *zp, numt_t xt, const nump_t *xp, numt_t yt, const nump_t *yp)
{
  assert(NUMT_IS_VALID(xt) && "unsupported number type");
  assert(NUMT_IS_VALID(yt) && "unsupported number type");
  if (isrect(xt) && isrect(yt)) {
    return rectadd(zp, xt, xp, yt, yp);
  } else if (isreal(xt) && isreal(yt)) {
    return realadd(zp, xt, xp, yt, yp);
  } else {
    double rx, ix, ry, iy;
    if (isrect(xt)) recttodd(xt, xp, &rx, &ix); else comptodd(xt, xp, &rx, &ix);
    if (isrect(yt)) recttodd(yt, yp, &ry, &iy); else comptodd(yt, yp, &ry, &iy);
    return NUMT_MKCOM(setflo(zp, rx+ry), setflo(zp+2, ix+iy));
  }
}

/* z = x - y */
static numt_t gnumsub(nump_t *zp, numt_t xt, const nump_t *xp, numt_t yt, const nump_t *yp)
{
  assert(NUMT_IS_VALID(xt) && "unsupported number type");
  assert(NUMT_IS_VALID(yt) && "unsupported number type");
  if (isrect(xt) && isrect(yt)) {
    return rectsub(zp, xt, xp, yt, yp);
  } else if (isreal(xt) && isreal(yt)) {
    return realsub(zp, xt, xp, yt, yp);
  } else {
    double rx, ix, ry, iy;
    if (isrect(xt)) recttodd(xt, xp, &rx, &ix); else comptodd(xt, xp, &rx, &ix);
    if (isrect(yt)) recttodd(yt, yp, &ry, &iy); else comptodd(yt, yp, &ry, &iy);
    return NUMT_MKCOM(setflo(zp, rx-ry), setflo(zp+2, ix-iy));
  }
}

/* z = x * y */
static numt_t gnummul(nump_t *zp, numt_t xt, const nump_t *xp, numt_t yt, const nump_t *yp)
{
  assert(NUMT_IS_VALID(xt) && "unsupported number type");
  assert(NUMT_IS_VALID(yt) && "unsupported number type");
  if ((isfix(xt) && getfix(xp) == 0) || (isfix(yt) && getfix(yp) == 0)) {
    return setfix(zp, 0);
  } else if (isreal(xt) && isreal(yt)) {
    return realmul(zp, xt, xp, yt, yp);
  } else if (isrect(xt) && isrect(yt)) {
    return rectmul(zp, xt, xp, yt, yp);
  } else {
    double rx, ix, ry, iy;
    if (isrect(xt)) recttodd(xt, xp, &rx, &ix); else comptodd(xt, xp, &rx, &ix);
    if (isrect(yt)) recttodd(yt, yp, &ry, &iy); else comptodd(yt, yp, &ry, &iy);
    return NUMT_MKCOM(setflo(zp, rx*ry - ix*iy), setflo(zp+2, rx*iy + ix*ry));
  }
}

/* z = x / y */
static numt_t gnumdiv(nump_t *zp, numt_t xt, const nump_t *xp, numt_t yt, const nump_t *yp)
{
  assert(NUMT_IS_VALID(xt) && "unsupported number type");
  assert(NUMT_IS_VALID(yt) && "unsupported number type");
  if (isfix(yt) && getfix(yp) == 0) {
    if (isrect(xt)) return setfail(EDOM); /* exact */
    if (isreal(xt)) return setflo(zp, realsign(xt, xp) < 0 ? -HUGE_VAL : HUGE_VAL);
  }
  if (isrect(xt) && isrect(yt)) {
    return rectdiv(zp, xt, xp, yt, yp);
  } else if (isreal(xt) && isreal(yt)) {
    return realdiv(zp, xt, xp, yt, yp);
  } else {
    double rx, ix, ry, iy, d;
    if (isrect(xt)) recttodd(xt, xp, &rx, &ix); else comptodd(xt, xp, &rx, &ix);
    if (isrect(yt)) recttodd(yt, yp, &ry, &iy); else comptodd(yt, yp, &ry, &iy);
    d = ry*ry + iy*iy;
    return NUMT_MKCOM(setflo(zp, (rx*ry + ix*iy)/d), setflo(zp+2, (ix*ry - rx*iy)/d));
  }
}

/* z = (exact)x; may return NUMT_NONE if conversion fails */
static numt_t gnumtoex(nump_t *zp, numt_t xt, const nump_t *xp)
{
  numt_t zt;
  assert(NUMT_IS_VALID(xt) && "unsupported number type");
  if (isrect(xt)) {
    zt = numdup(zp, xt, xp);
  } else if (isflo(xt)) {
    zt = dtorat(zp, getflo(xp));
  } else {
    numt_t rt = dtorat(zp, getflo(xp)), it = dtorat(zp+2, getflo(xp+2));
    zt = (isfix(it) && getfix(zp+2) == 0) ? rt : NUMT_MKCOM(rt, it);
  }
  if (!NUMT_IS_RECTNUM(zt)) return setfail(EDOM);
  else return zt;  
}

/* z = (inexact)x */
static numt_t gnumtoin(nump_t *zp, numt_t xt, const nump_t *xp)
{
  numt_t zt;
  assert(NUMT_IS_VALID(xt) && "unsupported number type");
  if (israt(xt)) {
    zt = setflo(zp, rattod(xt, xp));
  } else if (isrect(xt)) {
    zt = NUMT_MKCOM(setflo(zp, rattod(NUMT_COM_R(xt), xp)), setflo(zp+2, rattod(NUMT_COM_I(xt), xp+2)));
  } else {
    zt = numdup(zp, xt, xp);
  }
  if (!NUMT_IS_COMPNUM(zt)) return setfail(EDOM);
  else return zt;  
}

/* z = floor(x)  [real numbers only] */
static numt_t gnumfloor(nump_t *zp, numt_t xt, const nump_t *xp)
{
  assert(NUMT_IS_VALID(xt) && "unsupported number type");
  if (!isreal(xt)) return setfail(EDOM);
  return realfloor(zp, xt, xp);
}

/* z = ceiling(x)  [real numbers only] */
static numt_t gnumceil(nump_t *zp, numt_t xt, const nump_t *xp)
{
  assert(NUMT_IS_VALID(xt) && "unsupported number type");
  if (!isreal(xt)) return setfail(EDOM);
  return realceil(zp, xt, xp);
}

/* z = truncate(x)  [real numbers only] */
static numt_t gnumtrunc(nump_t *zp, numt_t xt, const nump_t *xp)
{
  assert(NUMT_IS_VALID(xt) && "unsupported number type");
  if (!isreal(xt)) return setfail(EDOM);
  return realtrunc(zp, xt, xp);
}

/* z = round(x)  [real numbers only] */
static numt_t gnumround(nump_t *zp, numt_t xt, const nump_t *xp)
{
  assert(NUMT_IS_VALID(xt) && "unsupported number type");
  if (!isreal(xt)) return setfail(EDOM);
  return realround(zp, xt, xp);
}

/* z = numerator(x)  [rational/real numbers only] */
static numt_t gnumnumer(nump_t *zp, numt_t xt, const nump_t *xp)
{
  assert(NUMT_IS_VALID(xt) && "unsupported number type");
  if (!isreal(xt)) return setfail(EDOM);
  /* R7RS: numerator of inexact rational is inexact */
  if (isflo(xt)) {
    /* n/d form: integer flonums have numerator == self */
    nump_t tp[2]; numt_t tt; double n, x = getflo(xp); /* nan/inf? */
    if (x != x || x <= -HUGE_VAL || x >= HUGE_VAL) return setflo(zp, x); 
    /* rationalize then take numerator as double */
    tt = dtorat(tp, x);
    if (tt == NUMT_NONE) return setflo(zp, x);
    n = inttod(NUMT_RAT_N(tt), tp);
    numfini(tt, tp);
    return setflo(zp, n);
  } else {
    /* exact rational */
    return numdup(zp, NUMT_RAT_N(xt), xp);
  }
}

/* z = denominator(x)  [rational/real numbers only] */
static numt_t gnumdenom(nump_t *zp, numt_t xt, const nump_t *xp)
{
  assert(NUMT_IS_VALID(xt) && "unsupported number type");
  if (!isreal(xt)) return setfail(EDOM);
  /* R7RS: denominator of inexact rational is inexact */
  if (isflo(xt)) {
    nump_t tp[2]; numt_t tt; double d, x = getflo(xp); /* nan/inf? */
    if (x != x || x <= -HUGE_VAL || x >= HUGE_VAL) return setflo(zp, x); 
    tt = dtorat(tp, x);
    if (tt == NUMT_NONE) return setflo(zp, 1.0);
    d = NUMT_RAT_D(tt) ? inttod(NUMT_RAT_D(tt), tp+1) : 1.0;
    numfini(tt, tp);
    return setflo(zp, d);
  } else {
    /* exact rational */
    numt_t dt = NUMT_RAT_D(xt);
    if (!dt) return setfix(zp, 1);  /* integer: denominator is 1 */
    return numdup(zp, dt, xp+1);
  }
}

/* z = gcd(x, y)  [integer numbers only; result is always non-negative] */
/* NB: gnumgcd(0,0)=0; result is inexact if either arg is inexact */
static numt_t gnumgcd(nump_t *zp, numt_t xt, const nump_t *xp, numt_t yt, const nump_t *yp)
{
  assert(NUMT_IS_VALID(xt) && "unsupported number type");
  assert(NUMT_IS_VALID(yt) && "unsupported number type");
  if (!isreal(xt) || !isreal(yt)) return setfail(EDOM);
  if (isflo(xt) || isflo(yt)) {
    /* inexact: convert to exact, compute gcd, return as inexact */
    double xd = isflo(xt) ? getflo(xp) : rattod(xt, xp);
    double yd = isflo(yt) ? getflo(yp) : rattod(yt, yp);
    /* use floating-point gcd via fmod (only valid for integral floats) */
    xd = fabs(xd); yd = fabs(yd);
    if (xd != floor(xd) || yd != floor(yd)) return setfail(EDOM);
    if (xd == 0.0) return setflo(zp, yd);
    while (yd != 0.0) { double t = fmod(xd, yd); xd = yd; yd = t; }
    return setflo(zp, xd);
  } else {
    /* both exact integers */
    nump_t axp[1]; numt_t axt = intabs(axp, xt, xp);
    nump_t ayp[1]; numt_t ayt = intabs(ayp, yt, yp);
    numt_t zt = intgcd(zp, axt, axp, ayt, ayp);
    numfini(axt, axp); numfini(ayt, ayp);
    return zt;
  }
}

/* z = truncate-quotient(x, y)  [integer real numbers only] */
static numt_t gnumtquo(nump_t *zp, numt_t xt, const nump_t *xp, numt_t yt, const nump_t *yp)
{
  assert(NUMT_IS_VALID(xt) && "unsupported number type");
  assert(NUMT_IS_VALID(yt) && "unsupported number type");
  if (!isreal(xt) || !isreal(yt)) return setfail(EDOM);
  if (isflo(xt) || isflo(yt)) {
    double x = isflo(xt) ? getflo(xp) : rattod(xt, xp);
    double y = isflo(yt) ? getflo(yp) : rattod(yt, yp);
    double i; modf(x/y, &i);
    return setflo(zp, i);
  } else {
    return intquo(zp, xt, xp, yt, yp);
  }
}

/* z = truncate-remainder(x, y)  [integer real numbers only] */
static numt_t gnumtrem(nump_t *zp, numt_t xt, const nump_t *xp, numt_t yt, const nump_t *yp)
{
  assert(NUMT_IS_VALID(xt) && "unsupported number type");
  assert(NUMT_IS_VALID(yt) && "unsupported number type");
  if (!isreal(xt) || !isreal(yt)) return setfail(EDOM);
  if (isflo(xt) || isflo(yt)) {
    double x = isflo(xt) ? getflo(xp) : rattod(xt, xp);
    double y = isflo(yt) ? getflo(yp) : rattod(yt, yp);
    double qi; modf(x/y, &qi);
    return setflo(zp, x - qi*y);
  } else {
    return intrem(zp, xt, xp, yt, yp);
  }
}

/* z = floor-quotient(x, y)  [integer real numbers only] */
static numt_t gnumfquo(nump_t *zp, numt_t xt, const nump_t *xp, numt_t yt, const nump_t *yp)
{
  assert(NUMT_IS_VALID(xt) && "unsupported number type");
  assert(NUMT_IS_VALID(yt) && "unsupported number type");
  if (!isreal(xt) || !isreal(yt)) return setfail(EDOM);
  if (isflo(xt) || isflo(yt)) {
    double x = isflo(xt) ? getflo(xp) : rattod(xt, xp);
    double y = isflo(yt) ? getflo(yp) : rattod(yt, yp);
    return setflo(zp, floor(x/y));
  } else {
    return intfquo(zp, xt, xp, yt, yp);
  }
}

/* z = floor-remainder(x, y)  [integer real numbers only] */
static numt_t gnumfrem(nump_t *zp, numt_t xt, const nump_t *xp, numt_t yt, const nump_t *yp)
{
  assert(NUMT_IS_VALID(xt) && "unsupported number type");
  assert(NUMT_IS_VALID(yt) && "unsupported number type");
  if (!isreal(xt) || !isreal(yt)) return setfail(EDOM);
  if (isflo(xt) || isflo(yt)) {
    double x = isflo(xt) ? getflo(xp) : rattod(xt, xp);
    double y = isflo(yt) ? getflo(yp) : rattod(yt, yp);
    return setflo(zp, x - floor(x/y)*y);
  } else {
    return intfrem(zp, xt, xp, yt, yp);
  }
}

/* q = integer-sqrt(x), r = x - q*q  [non-negative integer real numbers only] */
static void gnumisqrt(numt_t *pqt, nump_t *qp, numt_t *prt, nump_t *rp, numt_t xt, const nump_t *xp)
{
  assert(NUMT_IS_VALID(xt) && "unsupported number type");
  if (!isreal(xt) || !realcmp0(xt, xp, NCMP_GE)) {
    *pqt = *prt = NUMT_NONE;
  } if (isflo(xt)) {
    double x = getflo(xp), q = floor(sqrt(x));
    /* adjust for floating-point rounding near perfect squares */
    while ((q+1)*(q+1) <= x) q += 1.0;
    while (q*q > x) q -= 1.0;
    *pqt = setflo(qp, q);
    *prt = setflo(rp, x - q*q);
  } else {
    intsqrt(pqt, qp, prt, rp, xt, xp);
  }
}

/* z = sqrt(x)  [generic; result may be inexact complex] */
numt_t gnumsqrt(nump_t *zp, numt_t xt, const nump_t *xp)
{
  assert(NUMT_IS_VALID(xt) && "unsupported number type");
  if (isrect(xt)) {
    /* exact: try integer sqrt first, then fall back to inexact */
    if (isint(xt)) {
#if 1
      if (isfix(xt)) {
        long x = getfix(xp), ax = labs(x), q;
        for (q = 1; q*q > ax || ax > q*(q+2); q = (q + ax/q)/2);
        if (q*q == ax) {
          if (x >= 0) return setfix(zp, q);
          else return NUMT_MKCOM(setfix(zp, 0), setfix(zp+2, q));
        } else {
          if (x >= 0) return setflo(zp, sqrt(x));
          else return NUMT_MKCOM(setflo(zp, 0.0), setflo(zp+2, sqrt(-x)));
        }
      } else { /* isbig(x) */
        bignum_t *x = getbig(xp), *q = NULL;
        if (bntrysqrt(x, &q)) { /* exact q? */
          numt_t qt; nump_t *qp = (bnsign(x) >= 0) ? zp : zp+2;
          if (bnwidths(q) > FIXNUM_WIDTH) qt = setbig(qp, q);
          else { qt = setfix(qp, bntol(q)); bnfree(q); }
          if (bnsign(x) >= 0) return qt;
          else return NUMT_MKCOM(setfix(zp, 0), qt);
        } else {
          double q = bnsqrttod(x); /* ignores x sign, returns nonnegative q */
          if (bnsign(x) >= 0) return setflo(zp, q);
          return NUMT_MKCOM(setflo(zp, 0.0), setflo(zp+2, q));
        }
      }
#else    
      nump_t qp[1]; numt_t qt; nump_t rp[1]; numt_t rt;
      if (intsign(xt, xp) < 0) {
        nump_t tp[1]; numt_t tt = intneg(tp, xt, xp);
        intsqrt(&qt, qp, &rt, rp, tt, tp);
        numfini(tt, tp);
      } else {
        intsqrt(&qt, qp, &rt, rp, xt, xp);
      }
      if (isfix(rt) && getfix(rp) == 0) {
        /* perfect square: return exact result */
        /* no need for numfini(rt, rp); */
        if (intsign(xt, xp) < 0) {
          zp[2] = qp[0]; return NUMT_MKCOM(setfix(zp, 0), qt);
        } else {
          zp[0] = qp[0]; return qt;
        }
      }
      numfini(qt, qp); numfini(rt, rp);
      /* no exact sqrt; handle bignum case here */
      if (isbig(xt)) {
        bignum_t *x = getbig(xp);
        double r = bnsqrttod(x); /* ignores x sign, returns nonnegative r */
        if (bnsign(x) >= 0) return setflo(zp, r);
        return NUMT_MKCOM(setflo(zp, 0.0), setflo(zp+2, r));
      } 
#endif
    }
    if (!isint(xt) && israt(xt)) {
#if 1
      bignumll_t nxll, dxll; bignum_t *nq = NULL, *dq = NULL;
      numt_t nxt = NUMT_RAT_N(xt), dxt = NUMT_RAT_D(xt);
      bignum_t *nx = isbig(nxt) ? getbig(xp)   : bnx_makell(&nxll, getfix(xp));
      bignum_t *dx = isbig(dxt) ? getbig(xp+1) : bnx_makell(&dxll, dxt ? getfix(xp+1) : 1);
      if (bnrtrysqrt(nx, dx, &nq, &dq)) { /* exact q? */
        numt_t nqt; nump_t *nqp = (bnsign(nx) >= 0) ? zp   : zp+2;
        numt_t dqt; nump_t *dqp = (bnsign(nx) >= 0) ? zp+1 : zp+3;
        if (bnwidths(nq) > FIXNUM_WIDTH) nqt = setbig(nqp, nq);
        else { nqt = setfix(nqp, bntol(nq)); bnfree(nq); }
        if (bnwidths(dq) > FIXNUM_WIDTH) dqt = setbig(dqp, dq);
        else { dqt = setfix(dqp, bntol(dq)); bnfree(dq); }
        if (bnsign(nx) >= 0) return NUMT_MKRAT(nqt, dqt);
        else return NUMT_MKCOM(setfix(zp, 0), NUMT_MKRAT(nqt, dqt));
      } else { /* inexact */
        double q = bnrsqrttod(nx, dx); /* sqrt(|nx|/dx) */
        if (bnsign(nx) >= 0) return setflo(zp, q);
        else return NUMT_MKCOM(setflo(zp, 0.0), setflo(zp+2, q));
      }   
#else
      nump_t np[4]; numt_t nt = gnumsqrt(np, NUMT_RAT_N(xt), xp);
      nump_t dp[4]; numt_t dt = gnumsqrt(dp, NUMT_RAT_D(xt), xp+1);
      numt_t zt = (nt == NUMT_NONE || dt == NUMT_NONE) ? NUMT_NONE : gnumdiv(zp, nt, np, dt, dp); 
      numfini(nt, np); numfini(dt, dp);
      return zt;
#endif
    }
    if (isreal(xt)) {
      double x = rattod(xt, xp);
      if (x >= 0.0) return setflo(zp, sqrt(x));
      return NUMT_MKCOM(setflo(zp, 0.0), setflo(zp+2, sqrt(-x)));
    } else {
#if 1
      /* exact complex, with nonzero imag part */
      bignumll_t nrll, drll, nill, dill; 
      bignum_t *nrz = NULL, *drz = NULL, *niz = NULL, *diz = NULL;
      numt_t rt = NUMT_COM_R(xt), nrt = NUMT_RAT_N(rt), drt = NUMT_RAT_D(rt); 
      bignum_t *nr = isbig(nrt) ? getbig(xp)   : bnx_makell(&nrll, getfix(xp));
      bignum_t *dr = isbig(drt) ? getbig(xp+1) : bnx_makell(&drll, drt ? getfix(xp+1) : 1);
      numt_t it = NUMT_COM_I(xt), nit = NUMT_RAT_N(it), dit = NUMT_RAT_D(it); 
      bignum_t *ni = isbig(nit) ? getbig(xp+2) : bnx_makell(&nill, getfix(xp+2));
      bignum_t *di = isbig(dit) ? getbig(xp+3) : bnx_makell(&dill, dit ? getfix(xp+3) : 1);
      if (bnctrysqrt(nr, dr, ni, di, &nrz, &drz, &niz, &diz)) { /* exact? */
        numt_t nrt, drt = NUMT_NONE, nit, dit = NUMT_NONE; long l;
        if (bnwidths(nrz) > FIXNUM_WIDTH) nrt = setbig(zp+0, nrz);
        else { nrt = setfix(zp+0, bntol(nrz)); bnfree(nrz); }
        if (bnwidths(drz) > FIXNUM_WIDTH) drt = setbig(zp+1, drz);
        else { l = bntol(drz); bnfree(drz); if (l != 1) drt = setfix(zp+1, l); }        
        if (bnwidths(niz) > FIXNUM_WIDTH) nit = setbig(zp+2, niz);
        else { nit = setfix(zp+2, bntol(niz)); bnfree(niz); }
        if (bnwidths(diz) > FIXNUM_WIDTH) dit = setbig(zp+3, diz);
        else { l = bntol(diz); bnfree(diz); if (l != 1) dit = setfix(zp+3, l); }        
        return NUMT_MKCOM(NUMT_MKRAT(nrt, drt), NUMT_MKRAT(nit, dit));
      } else {
        double zr, zi; bncsqrttodd(&zr, &zi, nr, dr, ni, di);
        return NUMT_MKCOM(setflo(zp, zr), setflo(zp+2, zi));
      }
#else
      /* exact complex: convert to inexact and do complex sqrt */
      double rx, ix, re, im;
      recttodd(xt, xp, &rx, &ix);
      cmath_sqrt(rx, ix, &re, &im);
      return NUMT_MKCOM(setflo(zp, re), setflo(zp+2, im));
#endif
    }
  } else if (isreal(xt) && getflo(xp) != getflo(xp)) { /* NaN */
    /* traditionally imag NaN is not added (although it makes sense) */
    return setflo(zp, HUGE_VAL-HUGE_VAL); /* just real NaN */
  } else if (isreal(xt) && getflo(xp) >= 0.0) { /* including -0.0 */
    /* NB: IEEE 754 requires sqrt(-0.0) => -0.0, so we follow it */
    return setflo(zp, sqrt(getflo(xp)));
  } else { /* inexact complex */
    double rx = getflo(xp), ix = NUMT_COM_I(xt) ? getflo(xp+2) : 0.0;
    if (ix == 0.0 && rx > 0.0) {
      return NUMT_MKCOM(setflo(zp, sqrt(rx)), setflo(zp+2, ix));
    } else {
      double re, im;
      cmath_sqrt(rx, ix, &re, &im);
      return NUMT_MKCOM(setflo(zp, re), setflo(zp+2, im));
    }
  }
}

/* see below */
static int gnumodd(numt_t xt, const nump_t *xp); 
static numt_t gnumexp(nump_t *zp, numt_t xt, const nump_t *xp);
static numt_t gnumlog(nump_t *zp, numt_t xt, const nump_t *xp);
static numt_t gnummagn(nump_t *zp, numt_t xt, const nump_t *xp);
static numt_t gnumangl(nump_t *zp, numt_t xt, const nump_t *xp);
static numt_t gnumatan2(nump_t *zp, numt_t yt, const nump_t *yp, numt_t xt, const nump_t *xp);
static numt_t gnummkrec(nump_t *zp, numt_t xt, const nump_t *xp, numt_t yt, const nump_t *yp);

/* z = expt(x, y)  [generic] */
/* exact base with exact integer exponent stays exact; may go inexact otherwise */
static numt_t gnumexpt(nump_t *zp, numt_t xt, const nump_t *xp, numt_t yt, const nump_t *yp)
{
  assert(NUMT_IS_VALID(xt) && "unsupported number type");
  assert(NUMT_IS_VALID(yt) && "unsupported number type");
  /* special cases as far as the eye can see */
  if (isfix(yt) || isbig(yt)) {
    /* x^0 = 1 (for all x, including 0) */
    if (isfix(yt) && getfix(yp) == 0) 
      return setfix(zp, 1);
    /* x^1 = x for all x */
    if (isfix(yt) && getfix(yp) == 1) 
      return numdup(zp, xt, xp);
    /* x^-1 = 1/x for all x */
    if (isfix(yt) && getfix(yp) == -1) 
      return gnumdiv(zp, NUMT_FIX, &numfix_1, xt, xp);
    /* 0^y = 0 for y >= 0; fail if y < 0 */
    if (isfix(xt) && getfix(xp) == 0) 
      return intsign(yt, yp) < 0 ? setfail(EDOM) : setfix(zp, 0);
    /* 1^y = 1 */
    if (isfix(xt) && getfix(xp) == 1) 
      return setfix(zp, 1);
    /* -1^y = -1 if x is odd, 1 otherwise */
    if (isfix(xt) && getfix(xp) == -1)
      return intodd(yt, yp) ? setfix(zp, -1) : setfix(zp, 1);
    /* 2^y = ash(1, y) if y > 0, 1/ash(1, -y) otherwise */
    if (isfix(xt) && getfix(xp) == 2) {
      long x = getfix(yp), ax = labs(x); /* legal even if bignum */
      if (isbig(yt) || ax >= BIGNUM_MAX_BITS) return setfail(ERANGE);
      if (x > 0 && ax < FIXNUM_WIDTH-1) return setfix(zp, 1L << ax);
      if (x > 0) return setbig(zp, bnashll(bn1, ax));
      if (ax < FIXNUM_WIDTH-1) return NUMT_MKRAT(setfix(zp, 1), setfix(zp+1, 1L << ax));
      else return NUMT_MKRAT(setfix(zp, 1), setbig(zp+1, bnashll(bn1, ax)));
    }
    /* flonum special cases */
    if (isflo(xt)) {
      double x = getflo(xp); double y; 
      /* if y fits into a double exactly, just use pow */
      if (isfix(yt) || bnwidthu(getbig(yp)) <= DBL_MANT_DIG)
        return setflo(zp, (y = inttod(yt, yp)) == 0.0 ? 1.0 : pow(x, y));
      /* nan x spoils the answer */
      if (x != x) return setflo(zp, x);
      /* 0.0^y or -0.0^y where y > 0 */
      if (x == 0.0 && intsign(yt, yp) > 0) 
        return intodd(yt, yp) ? setflo(zp, x) : setflo(zp, 0.0);
      /* 0.0^y or -0.0^y where y < 0 */
      if (x == 0.0 && intsign(yt, yp) > 0) 
        return intodd(yt, yp) ? setflo(zp, 1.0/x) : setflo(zp, HUGE_VAL);
      /* 1.0^y or -1.0^y */
      if (x == 1.0 || x == -1.0)
        return intodd(yt, yp) ? setflo(zp, x) : setflo(zp, 1.0);
      /* exponents that guarantee any x != 1.0 && x != 0.0 over/underflows */
      if (isbig(yt) && bnwidthu(getbig(yp)) >= sizeof(double)*CHAR_BIT - 1) {
        if ((1.0 < fabs(x)) == (bnsign(getbig(yp)) > 0))
          return setflo(zp, (bnodd(getbig(yp)) && x < 0.0) ? -HUGE_VAL : HUGE_VAL);
        else
          return setflo(zp, (bnodd(getbig(yp)) && x < 0.0) ? -0.0 : 0.0);
      }
      /* now we have a bignum y which is not too big to guarantee over/underflow
       * of pow() call but not small enough to fit into a double exactly. There
       * is a nice trick by Bradley Lucier (@gambiteer) of breaking y in two,
       * but we'll leave it for later... TODO */
      return setflo(zp, pow(x, inttod(yt, yp)));
    }
    /* x is a compnum, y fits into a double exactly */
    if (iscomp(xt) && (isfix(yt) || bnwidthu(getbig(yp)) <= DBL_MANT_DIG)) {
      double rx, ix, ry, iy, rz, iz;
      comptodd(xt, xp, &rx, &ix); ry = inttod(yt, yp), iy = 0.0;
      cmath_pow(rx, ix, ry, iy, &rz, &iz);
      return NUMT_MKCOM(setflo(zp, rz), setflo(zp+2, iz));
    }
    /* ratnum special cases */
    if (!isint(xt) && israt(xt) && intsign(yt, yp) > 0) {
      nump_t np[4]; numt_t nt = gnumexpt(np, NUMT_RAT_N(xt), xp, yt, yp);
      nump_t dp[4]; numt_t dt = gnumexpt(dp, NUMT_RAT_D(xt), xp+1, yt, yp);
      numt_t zt = (nt == NUMT_NONE || dt == NUMT_NONE) ? NUMT_NONE : gnumdiv(zp, nt, np, dt, dp); 
      numfini(nt, np); numfini(dt, dp);
      return zt;
    } else if (!isint(xt) && israt(xt) && intsign(yt, yp) < 0) {
      nump_t tp[2]; numt_t tt = intneg(tp, yt, yp);
      nump_t np[2]; numt_t nt = gnumexpt(np, NUMT_RAT_N(xt), xp, tt, tp);
      nump_t dp[2]; numt_t dt = gnumexpt(dp, NUMT_RAT_D(xt), xp+1, tt, tp);
      numt_t zt = (nt == NUMT_NONE || dt == NUMT_NONE) ? NUMT_NONE : gnumdiv(zp, dt, dp, nt, np); 
      numfini(tt, tp); numfini(nt, np); numfini(dt, dp);
      return zt;
    }
    /* now see if we can calc exact^exact without memory overflow */
    if (isfix(xt) || isbig(xt)) { /* NB: x is neither 0 nor +1/-1! */
      long xbcap = isfix(yt) ? BIGNUM_MAX_BITS/labs(getfix(yp)) : 0;
      int ok = isfix(xt) ? (xbcap >= FIXNUM_WIDTH-1 || labs(getfix(xp)) < (1<<xbcap))
                         : bnwidthu(getbig(xp)) < (size_t)xbcap;
      if (!ok) return setfail(ERANGE);
      assert(isfix(yt)); assert(labs(getfix(yp)) > 1);
      if (getfix(yp) > 0) { /* do bignum expt, normalize */
        bignum_t *bx = isfix(xt) ? lltobn(getfix(xp)) : getbig(xp);
        bignum_t *bz = bnexptull(bx, (uint64_t)getfix(yp));
        numt_t zt;
        if (isfix(xt)) bnfree(bx); 
        if (bnwidths(bz) > FIXNUM_WIDTH) zt = setbig(zp, bz);
        else (zt = setfix(zp, bntol(bz))), bnfree(bz);
        return zt;
      } else { /* do bignum expt, normalize, get reciprocal */
        bignum_t *bx = isfix(xt) ? lltobn(getfix(xp)) : getbig(xp);
        bignum_t *bd = bnexptull(bx, (uint64_t)labs(getfix(yp)));
        nump_t dp[1]; numt_t dt; numt_t zt; 
        if (isfix(xt)) bnfree(bx);
        if (bnwidths(bd) > FIXNUM_WIDTH) dt = setbig(dp, bd);
        else (dt = setfix(dp, bntol(bd))), bnfree(bd);
        zt = gnumdiv(zp, NUMT_FIX, &numfix_1, dt, dp);
        numfini(dt, dp);
        return zt;
      }
    }
  } else if (isflo(yt)) {
    /* nan to nan */
    if (getflo(yp) != getflo(yp)) return setflo(zp, getflo(yp));
    if (isflo(xt) && getflo(xp) != getflo(xp)) return setflo(zp, getflo(xp));
    /* x^0.0 = 1.0 (for all x, including 0.0) */
    if (getflo(yp) == 0.0) return setflo(zp, 1.0);
    /* x^1.0 = (inexact)x for all x */
    if (getflo(yp) == 1.0)
      return gnumtoin(zp, xt, xp);
    /* x^-1.0 = 1.0/x for all x */
    if (getflo(yp) == -1.0) {
      nump_t tp[1]; numt_t tt = setflo(tp, 1.0);  
      return gnumdiv(zp, tt, tp, xt, xp);
    }
    /* 0.0^y = 0.0 for y != 0.0 */
    if ((isfix(xt) && getfix(xp) == 0) || (isflo(xt) && getflo(xp) == 0.0)) {
      int neg = isflo(xt) && 1.0/getflo(xp) < 0.0 && gnumodd(yt, yp) > 0;
      if (getflo(yp) >= 0) return setflo(zp, neg ? -0.0 : 0.0);
      return setflo(zp, neg ? -HUGE_VAL : HUGE_VAL);
    }
    /* 1^y = 1.0^y = 1.0 */
    if ((isfix(xt) && getfix(xp) == 1) || (isflo(xt) && getflo(xp) == 1.0)) 
      return setflo(zp, 1.0);
    /* -1^y = -1.0^y = 1.0/-1.0 depending on eveness of y */
    if ((isfix(xt) && getfix(xp) == -1) || (isflo(xt) && getflo(xp) == -1.0)) 
      switch (gnumodd(yt, yp)) {
        case 0: return setflo(zp, 1.0); /* even */
        case 1: return setflo(zp, -1.0); /* odd */
        default: ; /* neither; fall thru */
      }
    /* +flonum^flonum: just call pow */
    if (isflo(xt) && getflo(xp) >= 0.0)
      return setflo(zp, pow(getflo(xp), getflo(yp)));
    /* else fall thru */
  } else if (israt(yt)) {
    /* 0^y = 0 for y >= 0; fail if y < 0 */
    if (isfix(xt) && getfix(xp) == 0) 
      return ratsign(yt, yp) < 0 ? setfail(EDOM) : setfix(zp, 0);
    /* 1^y = 1 */
    if (isfix(xt) && getfix(xp) == 1) 
      return setfix(zp, 1);
    /* x^1/2 = sqrt(x) */
    if (isfix(NUMT_RAT_N(yt)) && getfix(yp) == 1 && isfix(NUMT_RAT_D(yt)) && getfix(yp+1) == 2)
      return gnumsqrt(zp, xt, xp);
    /* else fall through */
  } else if (isrect(yt)) {
    /* 0^y = 0 for y >= 0; fail if y < 0 */
    if (isfix(xt) && getfix(xp) == 0) 
      return ratsign(yt, yp) < 0 ? setfail(EDOM) : setfix(zp, 0);
    /* 1^y = 1 */
    if (isfix(xt) && getfix(xp) == 1) 
      return setfix(zp, 1);
    /* else fall through */
  }
  /* exact base, exact non-negative integer exponent: exact result */
  if (isrect(xt) && isint(yt) && intsign(yt, yp) >= 0) {
    nump_t ap[4]; numt_t at = numdup(ap, xt, xp);
    nump_t ep[1]; numt_t et = numdup(ep, yt, yp);
    nump_t rp[4]; numt_t rt = setfix(rp, 1);
    while (!(isfix(et) && getfix(ep) == 0)) {
      if (intodd(et, ep)) { /* if odd(e) r := r*a */
        nump_t tp[4]; numt_t tt = gnummul(tp, rt, rp, at, ap);
        numfini(rt, rp); rt = nummove(rp, tt, tp);
      }
      { /* e := e/2 */
        nump_t twop[1]; numt_t two = setfix(twop, 2);
        nump_t tp[1]; numt_t tt = intquo(tp, et, ep, two, twop);
        numfini(et, ep); et = tt; ep[0] = tp[0];
      }
      if (!(isfix(et) && getfix(ep) == 0)) { /* if (e != 0) a := a*a */
        nump_t tp[4]; numt_t tt = gnummul(tp, at, ap, at, ap);
        numfini(at, ap); at = nummove(ap, tt, tp);
      }
    }
    numfini(at, ap); numfini(et, ep);
    return nummove(zp, rt, rp);
  }
  /* exact base, exact negative integer exponent: 1 / base^|exp| */
  if (isrect(xt) && isrect(yt) && isint(yt) && gnumcmp0(yt, yp, NCMP_LT)) {
    nump_t tp[4]; numt_t tt; nump_t nep[4]; numt_t net; numt_t zt;
    net = gnumneg(nep, yt, yp);
    tt = gnumexpt(tp, xt, xp, net, nep);
    numfini(net, nep);
    zt = gnumdiv(zp, NUMT_FIX, &numfix_1, tt, tp);
    numfini(tt, tp);
    return zt;
  } else if (israt(xt) && isreal(yt) && ratsign(xt, xp) > 0) {
    /* x^y = exp(log(x)*y) via inexacts, but w/lower chances of overflow */
    nump_t lp[4]; numt_t lt = gnumlog(lp, xt, xp);
    nump_t mp[4]; numt_t mt = gnummul(mp, lt, lp, yt, yp);
    numt_t zt = gnumexp(zp, mt, mp); 
    numfini(lt, lp); numfini(mt, mp);
    return zt;
  } else { /* general case: go inexact via polar form: (r,theta)^(c+di) */
    double rx, ix, ry, iy, rz, iz;
    if (isrect(xt)) recttodd(xt, xp, &rx, &ix); else comptodd(xt, xp, &rx, &ix);
    if (isrect(yt)) recttodd(yt, yp, &ry, &iy); else comptodd(yt, yp, &ry, &iy);
    cmath_pow(rx, ix, ry, iy, &rz, &iz);
    /* if inputs were purely real, and xt is positive, result is real */
    if (isreal(xt) && isreal(yt) && rx > 0.0 && iz == 0.0) return setflo(zp, rz); 
    return NUMT_MKCOM(setflo(zp, rz), setflo(zp+2, iz));
  }
}

/* z = real-part(x) */
static numt_t gnumreal(nump_t *zp, numt_t xt, const nump_t *xp)
{
  assert(NUMT_IS_VALID(xt) && "unsupported number type");
  if (isreal(xt)) {
    return numdup(zp, xt, xp);
  } else if (isrect(xt)) {
    return numdup(zp, NUMT_COM_R(xt), xp);
  } else {
    /* compnum */
    return setflo(zp, getflo(xp));
  }
}

/* z = imag-part(x) */
static numt_t gnumimag(nump_t *zp, numt_t xt, const nump_t *xp)
{
  assert(NUMT_IS_VALID(xt) && "unsupported number type");
  if (isreal(xt)) {
    /* real number: imaginary part is exact 0 */
    return setfix(zp, 0);
  } else if (isrect(xt)) {
    return numdup(zp, NUMT_COM_I(xt), xp+2);
  } else {
    /* compnum */
    return NUMT_COM_I(xt) ? setflo(zp, getflo(xp+2)) : setflo(zp, 0.0);
  }
}

/* z = magnitude(x) [a.k.a. abs for reals] */
static numt_t gnummagn(nump_t *zp, numt_t xt, const nump_t *xp)
{
  assert(NUMT_IS_VALID(xt) && "unsupported number type");
  if (isreal(xt)) {
    return realabs(zp, xt, xp);
  } else if (isrect(xt)) {
    /* exact complex: do the math directly */
    nump_t rp[2]; numt_t rt = ratmul(rp, NUMT_COM_R(xt), xp, NUMT_COM_R(xt), xp); 
    nump_t ip[2]; numt_t it = ratmul(ip, NUMT_COM_I(xt), xp+2, NUMT_COM_I(xt), xp+2); 
    nump_t sp[2]; numt_t st = ratadd(sp, rt, rp, it, ip);
    numt_t zt; numfini(rt, rp); numfini(it, ip);
    zt = gnumsqrt(zp, st, sp);
    numfini(st, sp);
    return zt;
  } else {
    /* inexact complex: use hypot */
    double rx, ix; comptodd(xt, xp, &rx, &ix);
    return setflo(zp, cmath_hypot(rx, ix));
  }
}

/* z = angle(x)  [always inexact] */
static numt_t gnumangl(nump_t *zp, numt_t xt, const nump_t *xp)
{
  assert(NUMT_IS_VALID(xt) && "unsupported number type");
  if (isfix(xt) && getfix(xp) == 0) return setfail(EDOM);
  if (isflo(xt)) {
    double x = getflo(xp);
    int neg = (x == 0.0) ? 1.0/x < 0.0 : x < 0.0; 
    return setflo(zp, neg ? M_PI : 0.0);
  } else if (israt(xt)) {
    return ratsign(xt, xp) < 0 ? setflo(zp, M_PI) : setfix(zp, 0);
  } else if (isrect(xt)) {
    return gnumatan2(zp, NUMT_COM_I(xt), xp+2, NUMT_COM_R(xt), xp);
  } else {
    double rx = getflo(xp), ix = NUMT_COM_I(xt) ? getflo(xp+2) : 0.0;
    return setflo(zp, atan2(ix, rx));
  }
}

/* z = make-rectangular(x, y)  [x, y must be real] */
static numt_t gnummkrec(nump_t *zp, numt_t xt, const nump_t *xp, numt_t yt, const nump_t *yp)
{
  assert(NUMT_IS_VALID(xt) && "unsupported number type");
  assert(NUMT_IS_VALID(yt) && "unsupported number type");
  if (!isreal(xt) || !isreal(yt)) return setfail(EDOM);
  if (isfix(yt) && getfix(yp) == 0) {
    /* stay real */
    return numdup(zp, xt, xp);
  } else if (isflo(xt) || isflo(yt)) {
    /* inexact: always produces compnum */
    double x = isflo(xt) ? getflo(xp) : rattod(xt, xp);
    double y = isflo(yt) ? getflo(yp) : rattod(yt, yp);
    return NUMT_MKCOM(setflo(zp, x), setflo(zp+2, y));
  } else {
    /* both exact */
    numt_t rt = numdup(zp, xt, xp);
    numt_t it = numdup(zp+2, yt, yp);
    return NUMT_MKCOM(rt, it);
  }
}

/* z = make-polar(r, theta)  [always inexact] */
static numt_t gnummkpol(nump_t *zp, numt_t xt, const nump_t *xp, numt_t yt, const nump_t *yp)
{
  assert(NUMT_IS_VALID(xt) && "unsupported number type");
  assert(NUMT_IS_VALID(yt) && "unsupported number type");
  if (!isreal(xt) || !isreal(yt)) return setfail(EDOM);
  if ((isfix(yt) && getfix(yp) == 0) || (isfix(xt) && getfix(xp) == 0)) {
    return numdup(zp, xt, xp);
  } else {
    double m = isflo(xt) ? getflo(xp) : rattod(xt, xp);
#ifndef COMPACT_RATTRIG
    if (isflo(yt)) {
      double a = getflo(yp);
      return NUMT_MKCOM(setflo(zp, m*cos(a)), setflo(zp+2, m*sin(a)));
    } else {
      double sa, ca; ratsincos(yt, yp, &sa, &ca);
      return NUMT_MKCOM(setflo(zp, m*ca), setflo(zp+2, m*sa));
    }
#else    
    double a = isflo(yt) ? getflo(yp) : rattod(yt, yp);
    return NUMT_MKCOM(setflo(zp, m*cos(a)), setflo(zp+2, m*sin(a)));
#endif
  }
}

/* transcendental functions: all return inexact (mostly) */

/* z = exp(x) */
static numt_t gnumexp(nump_t *zp, numt_t xt, const nump_t *xp)
{
  assert(NUMT_IS_VALID(xt) && "unsupported number type");
  if (isfix(xt) && getfix(xp) == 0) {
    return setfix(zp, 1); /* consistent with log, expt */
  } else if (isreal(xt)) {
    double x = isflo(xt) ? getflo(xp) : rattod(xt, xp);
    return setflo(zp, exp(x));
  } else {
    double rx, ix, re, im;
    if (isrect(xt)) recttodd(xt, xp, &rx, &ix); else comptodd(xt, xp, &rx, &ix);
    cmath_exp(rx, ix, &re, &im);
    return NUMT_MKCOM(setflo(zp, re), setflo(zp+2, im));
  }
}

/* z = log(x)  [natural log; complex result for negative reals] */
static numt_t gnumlog(nump_t *zp, numt_t xt, const nump_t *xp)
{
  assert(NUMT_IS_VALID(xt) && "unsupported number type");
  if (isfix(xt)) {
    long x = getfix(xp);
    if (x == 0) return setfail(EDOM); /* most Schemes fail */
    if (x == 1) return setfix(zp, 0); /* consistent with exp */
    if (x >= 0) return setflo(zp, log(x));
    return NUMT_MKCOM(setflo(zp, log(labs(x))), setflo(zp+2, M_PI));
  } else if (isbig(xt)) {
    bignum_t *x = getbig(xp);
    if (bnsign(x) >= 0) return setflo(zp, bnlogtod(x));
    return NUMT_MKCOM(setflo(zp, bnlogtod(x)), setflo(zp+2, M_PI));
  } else if (israt(xt)) {
    int sign = intsign(NUMT_RAT_N(xt), xp);
#if 1
    bignumll_t nxll, dxll;
    numt_t nxt = NUMT_RAT_N(xt), dxt = NUMT_RAT_D(xt); 
    bignum_t *nx = isbig(nxt) ? getbig(xp)   : bnx_makell(&nxll, getfix(xp));
    bignum_t *dx = isbig(dxt) ? getbig(xp+1) : bnx_makell(&dxll, dxt ? getfix(xp+1) : 1);
    double z = bnrlogtod(nx, dx); /* z = log(|nx|/dx), accurate around 1 */
    if (sign >= 0) return setflo(zp, z);
    return NUMT_MKCOM(setflo(zp, z), setflo(zp+2, M_PI));
#else
    double nlog = isfix(NUMT_RAT_N(xt)) ? log(labs(getfix(xp))) : bnlogtod(getbig(xp));
    double dlog = isfix(NUMT_RAT_D(xt)) ? log(labs(getfix(xp+1))) : bnlogtod(getbig(xp+1));
    if (sign >= 0) return setflo(zp, nlog-dlog);
    return NUMT_MKCOM(setflo(zp, nlog-dlog), setflo(zp+2, M_PI));
#endif
  } else if (isflo(xt)) {
    double x = getflo(xp);
    /* if (x > 0.0 || x == 0.0 && 1.0/x > 0.0) return setflo(zp, log(x)); */
    if (x >= 0.0) return setflo(zp, log(x));
    return NUMT_MKCOM(setflo(zp, log(-x)), setflo(zp+2, M_PI));
  } else if (isrect(xt)) {
    /* do the math directly so as not to convert to double prematurely */
    nump_t rp[2]; numt_t rt = ratmul(rp, NUMT_COM_R(xt), xp, NUMT_COM_R(xt), xp); 
    nump_t ip[2]; numt_t it = ratmul(ip, NUMT_COM_I(xt), xp+2, NUMT_COM_I(xt), xp+2); 
    nump_t sp[2]; numt_t st = ratadd(sp, rt, rp, it, ip); /* nonnegative! */
    nump_t lp[4]; numt_t lt = gnumlog(lp, st, sp); /* 4 slots just in case */
    nump_t tp[1]; numt_t tt = setfix(tp, 2), zt;
    numfini(rt, rp); numfini(it, ip); numfini(st, sp);
    rt = gnumdiv(rp, lt, lp, tt, tp); assert(!NUMT_COM_I(rt));
    it = gnumangl(ip, xt, xp); assert(!NUMT_COM_I(it));
    numfini(lt, lp); numfini(tt, tp); 
    zt = gnummkrec(zp, rt, rp, it, ip);
    numfini(rt, rp); numfini(it, ip); 
    return zt;
  } else {
    double rx, ix, re, im;
    if (isrect(xt)) recttodd(xt, xp, &rx, &ix); else comptodd(xt, xp, &rx, &ix);
    cmath_log(rx, ix, &re, &im);
    return NUMT_MKCOM(setflo(zp, re), setflo(zp+2, im));
  }
}

/* z = log(x, y)  [log base y] */
static numt_t gnumlogn(nump_t *zp, numt_t xt, const nump_t *xp, numt_t yt, const nump_t *yp)
{
  assert(NUMT_IS_VALID(xt) && "unsupported number type");
  assert(NUMT_IS_VALID(yt) && "unsupported number type");
  /* special case of y = 10 and real x using log10 */
  /* TODO: special case of y = 2 and real x using C99 log2 */
  if (isfix(yt) && getfix(yp) == 10 && isreal(xt)) {
    if (isfix(xt) && getfix(xp) == 0) {
      return setfail(EDOM); /* most Schemes fail */
    } else if (isfix(xt) && getfix(xp) == 1) {
      return setfix(zp, 0); /* consistent with exp */
    } else if (isflo(xt)) {
      double x = getflo(xp);
      /* if (x > 0.0 || x == 0.0 && 1.0/x > 0.0) return setflo(zp, log10(x)); */
      if (x >= 0.0) return setflo(zp, log10(x));
      /* negative real: log10(x) = log10|x| + i*pi/log(10) */
      return NUMT_MKCOM(setflo(zp, log10(-x)), setflo(zp+2, M_PI_LN10));
    }
  } 
  { /* fallback code; used ror int and rat x too */
    nump_t lxp[4]; numt_t lxt = gnumlog(lxp, xt, xp);
    nump_t lyp[4]; numt_t lyt = gnumlog(lyp, yt, yp);
    numt_t zt = gnumdiv(zp, lxt, lxp, lyt, lyp);
    numfini(lxt, lxp); numfini(lyt, lyp);
    return zt;
  }
}

/* z = sin(x) */
static numt_t gnumsin(nump_t *zp, numt_t xt, const nump_t *xp)
{
  assert(NUMT_IS_VALID(xt) && "unsupported number type");
  if (isfix(xt) && getfix(xp) == 0) {
    return setfix(zp, 0); /* consistent with exp */
  } else if (isreal(xt)) {
#ifndef COMPACT_RATTRIG
    if (isflo(xt)) return setflo(zp, sin(getflo(xp)));
    else { double s, c; ratsincos(xt, xp, &s, &c); return setflo(zp, s); }
#else  
    double x = isflo(xt) ? getflo(xp) : rattod(xt, xp);
    return setflo(zp, sin(x));
#endif
  } else {
    double rx, ix, re, im;
    if (isrect(xt)) recttodd(xt, xp, &rx, &ix); else comptodd(xt, xp, &rx, &ix);
    cmath_sin(rx, ix, &re, &im);
    return NUMT_MKCOM(setflo(zp, re), setflo(zp+2, im));
  }
}

/* z = cos(x) */
static numt_t gnumcos(nump_t *zp, numt_t xt, const nump_t *xp)
{
  assert(NUMT_IS_VALID(xt) && "unsupported number type");
  if (isfix(xt) && getfix(xp) == 0) {
    return setfix(zp, 1); /* consistent with exp */
  } else if (isreal(xt)) {
#ifndef COMPACT_RATTRIG
    if (isflo(xt)) return setflo(zp, cos(getflo(xp)));
    else { double s, c; ratsincos(xt, xp, &s, &c); return setflo(zp, c); }
#else  
    double x = isflo(xt) ? getflo(xp) : rattod(xt, xp);
    return setflo(zp, cos(x));
#endif
  } else {
    double rx, ix, re, im;
    if (isrect(xt)) recttodd(xt, xp, &rx, &ix); else comptodd(xt, xp, &rx, &ix);
    cmath_cos(rx, ix, &re, &im);
    return NUMT_MKCOM(setflo(zp, re), setflo(zp+2, im));
  }
}

/* z = tan(x) */
static numt_t gnumtan(nump_t *zp, numt_t xt, const nump_t *xp)
{
  assert(NUMT_IS_VALID(xt) && "unsupported number type");
  if (isfix(xt) && getfix(xp) == 0) {
    return setfix(zp, 0); /* consistent with exp */
  } else if (isreal(xt)) {
#ifndef COMPACT_RATTRIG
    if (isflo(xt)) return setflo(zp, tan(getflo(xp)));
    else { double s, c; ratsincos(xt, xp, &s, &c); return setflo(zp, s/c); }
#else  
    double x = isflo(xt) ? getflo(xp) : rattod(xt, xp);
    return setflo(zp, tan(x));
#endif
  } else {
    double rx, ix, re, im;
    if (isrect(xt)) recttodd(xt, xp, &rx, &ix); else comptodd(xt, xp, &rx, &ix);
    cmath_tan(rx, ix, &re, &im);
    return NUMT_MKCOM(setflo(zp, re), setflo(zp+2, im));
  }
}

/* z = asin(x) */
static numt_t gnumasin(nump_t *zp, numt_t xt, const nump_t *xp)
{
  double rx, ix, re, im;
  assert(NUMT_IS_VALID(xt) && "unsupported number type");
  if (isfix(xt) && getfix(xp) == 0) {
    return setfix(zp, 0); /* consistent with sin */
  } else if (isflo(xt) && getflo(xp) != getflo(xp)) { /* NaN */
    /* traditionally imag NaN is not added (although it makes sense) */
    return setflo(zp, HUGE_VAL-HUGE_VAL); /* just real NaN */
  } else if (isreal(xt)) {
    rx = isflo(xt) ? getflo(xp) : rattod(xt, xp);
    ix = 0.0;
  } else {
    if (iscomp(xt)) comptodd(xt, xp, &rx, &ix); else recttodd(xt, xp, &rx, &ix);
  }
  if (ix == 0.0 && rx >= -1.0 && rx <= 1.0) {
    if (isreal(xt)) return setflo(zp, asin(rx));
    return NUMT_MKCOM(setflo(zp, asin(rx)), setflo(zp+2, 0.0));
  }
  cmath_asin(rx, ix, &re, &im);
  /* C99 conventions => R7RS */
  if (isreal(xt)) { 
    if       (rx > 1.0) im = -fabs(im);
    else if (rx < -1.0) im =  fabs(im);
  } else if (ix == 0.0) {
    if   (1.0/ix < 0.0) im = -fabs(im);
    else                im = fabs(im);
  }
  return NUMT_MKCOM(setflo(zp, re), setflo(zp+2, im));
}

/* z = acos(x) */
static numt_t gnumacos(nump_t *zp, numt_t xt, const nump_t *xp)
{
  double rx, ix, re, im;
  assert(NUMT_IS_VALID(xt) && "unsupported number type");
  if (isfix(xt) && getfix(xp) == 1) {
    return setfix(zp, 0); /* consistent with cos */
  } else if (isflo(xt) && getflo(xp) != getflo(xp)) { /* NaN */
    /* traditionally imag NaN is not added (although it makes sense) */
    return setflo(zp, HUGE_VAL-HUGE_VAL); /* just real NaN */
  } else if (isreal(xt)) {
    rx = isflo(xt) ? getflo(xp) : rattod(xt, xp);
    ix = 0.0;
  } else {
    if (iscomp(xt)) comptodd(xt, xp, &rx, &ix); else recttodd(xt, xp, &rx, &ix);
  }
  if (ix == 0.0 && rx >= -1.0 && rx <= 1.0) {
    if (isreal(xt)) return setflo(zp, acos(rx));
    return NUMT_MKCOM(setflo(zp, acos(rx)), setflo(zp+2, 0.0));
  }
  cmath_acos(rx, ix, &re, &im);
  /* C99 conventions => R7RS */
  if (isreal(xt)) {
    if       (rx > 1.0) im = fabs(im);
    else if (rx < -1.0) im = -fabs(im);
  } else if (ix == 0.0) {
    if   (1.0/ix < 0.0) im = fabs(im);
    else                im = -fabs(im);
  }
  return NUMT_MKCOM(setflo(zp, re), setflo(zp+2, im));
}

/* z = atan(x) [1-argument] */
/* z = atan(x) [1-argument] */
numt_t gnumatan(nump_t *zp, numt_t xt, const nump_t *xp)
{
  double rx, ix, re, im;
  assert(NUMT_IS_VALID(xt) && "unsupported number type");
  if (isfix(xt) && getfix(xp) == 0) {
    return setfix(zp, 0); /* consistent with tan */
  } else if (isreal(xt)) {
    rx = isflo(xt) ? getflo(xp) : rattod(xt, xp);
    ix = 0.0;
#if 1
  } else if (isrect(xt)) {
    bignumll_t nrll, drll, nill, dill;
    numt_t rt = NUMT_COM_R(xt), nrt = NUMT_RAT_N(rt), drt = NUMT_RAT_D(rt); 
    bignum_t *nr = isbig(nrt) ? getbig(xp)   : bnx_makell(&nrll, getfix(xp));
    bignum_t *dr = isbig(drt) ? getbig(xp+1) : bnx_makell(&drll, drt ? getfix(xp+1) : 1);
    numt_t it = NUMT_COM_I(xt), nit = NUMT_RAT_N(it), dit = NUMT_RAT_D(it); 
    bignum_t *ni = isbig(nit) ? getbig(xp+2) : bnx_makell(&nill, getfix(xp+2));
    bignum_t *di = isbig(dit) ? getbig(xp+3) : bnx_makell(&dill, dit ? getfix(xp+3) : 1);
    double zr, zi; bncatantodd(&zr, &zi, nr, dr, ni, di);
    return NUMT_MKCOM(setflo(zp, zr), setflo(zp+2, zi));
  } else {
    comptodd(xt, xp, &rx, &ix);
  }
#else
  } else {
    if (iscomp(xt)) comptodd(xt, xp, &rx, &ix); else recttodd(xt, xp, &rx, &ix);
  }
#endif
  if (ix == 0.0) {
    if (isreal(xt)) return setflo(zp, atan(rx));
    return NUMT_MKCOM(setflo(zp, atan(rx)), setflo(zp+2, 0.0));
  }
  cmath_atan(rx, ix, &re, &im);
  return NUMT_MKCOM(setflo(zp, re), setflo(zp+2, im));
}

/* z = atan(y, x) [2-argument, real only] */
static numt_t gnumatan2(nump_t *zp, numt_t yt, const nump_t *yp, numt_t xt, const nump_t *xp)
{
  assert(NUMT_IS_VALID(xt) && "unsupported number type");
  assert(NUMT_IS_VALID(yt) && "unsupported number type");
  if (!isreal(xt) || !isreal(yt)) return setfail(EDOM);
  /* special corner case fixes */
  if (isfix(yt) && getfix(yp) == 0 && isfix(xt)) {
    if (getfix(xp) > 0) return setfix(zp, 0); 
    else if (getfix(xp) < 0) return setflo(zp, M_PI); 
    else return setfail(EDOM);
  } else if (israt(yt) && israt(xt)) {
    bignumll_t nyll, dyll, nxll, dxll; 
    numt_t nyt = NUMT_RAT_N(yt), dyt = NUMT_RAT_D(yt); 
    numt_t nxt = NUMT_RAT_N(xt), dxt = NUMT_RAT_D(xt); 
    bignum_t *ny = isbig(nyt) ? getbig(yp)   : bnx_makell(&nyll, getfix(yp));
    bignum_t *dy = isbig(dyt) ? getbig(yp+1) : bnx_makell(&dyll, dyt ? getfix(yp+1) : 1);
    bignum_t *nx = isbig(nxt) ? getbig(xp)   : bnx_makell(&nxll, getfix(xp));
    bignum_t *dx = isbig(dxt) ? getbig(xp+1) : bnx_makell(&dxll, dxt ? getfix(xp+1) : 1);
    return setflo(zp, bnratan2tod(ny, dy, nx, dx));
  } else {
    double y = isflo(yt) ? getflo(yp) : rattod(yt, yp);  /* first arg is y */
    double x = isflo(xt) ? getflo(xp) : rattod(xt, xp);  /* second arg is x */
    /* apparently, these atan2 fixes are needed */
    if (y >= HUGE_VAL) {
      /* atan2(+inf.0,+inf.0) => pi/4 */
      if (x >= HUGE_VAL) return setflo(zp, M_PI_4);
      /* atan2(+inf.0,-inf.0) => 3pi/4 */
      if (x <= -HUGE_VAL) return setflo(zp, M_PI-M_PI_4);
    } else if (y <= -HUGE_VAL) {
      /* atan2(-inf.0,+inf.0) => -pi/4 */
      if (x >= HUGE_VAL) return setflo(zp, -M_PI_4);
      /* atan2(-inf.0,-inf.0) => -pi/2 instead of -3pi/4 */
      if (x <= -HUGE_VAL) return setflo(zp, -M_PI+M_PI_4);
    } else if (y == 0.0) {
      /* atan2(-0.0,-1.0) => -pi */
      if (1.0/y < 0.0 && x == -1.0) return setflo(zp, -M_PI);
      /* atan2(-0.0,-0.0) => -pi */
      if (1.0/y < 0.0 && x == 0.0 && 1.0/x < 0.0) return setflo(zp, -M_PI);
      /* atan2(+0.0,-0.0) => +pi */
      if (1.0/y >= 0.0 && x == 0.0 && 1.0/x < 0.0) return setflo(zp, M_PI);
    }
    return setflo(zp, atan2(y, x));
  }
}

/* odd?(x) and even?(x)  [integer real numbers only] */
static int gnumodd(numt_t xt, const nump_t *xp)
{
  assert(NUMT_IS_VALID(xt) && "unsupported number type");
  if (!isreal(xt)) return -1;
  if (isflo(xt)) {
    double d = getflo(xp);
    return flisint(d) ? flisint((d + 1.0) / 2.0) : -1;
  } else if isint(xt) {
    return intodd(xt, xp);
  }
  return -1;
}

static int gnumeven(numt_t xt, const nump_t *xp)
{
  assert(NUMT_IS_VALID(xt) && "unsupported number type");
  if (!isreal(xt)) return -1;
  if (isflo(xt)) {
    double d = getflo(xp);
    return flisint(d) ? flisint(d / 2.0) : -1;
  } else if isint(xt) {
    return !intodd(xt, xp);
  }
  return -1;
}

static int gnumzero(numt_t xt, const nump_t *xp)
{
  assert(NUMT_IS_VALID(xt) && "unsupported number type");
  if (isfix(xt)) return getfix(xp) == 0;
  if (isflo(xt)) return getflo(xp) == 0.0;
  if (iscomp(xt)) return getflo(xp) == 0.0 && getflo(xp+2) == 0.0;
  return 0;
}

static int gnumpositive(numt_t xt, const nump_t *xp)
{
  assert(NUMT_IS_VALID(xt) && "unsupported number type");
  if (!isreal(xt)) return -1;
  return realcmp0(xt, xp, NCMP_GT);
}

static int gnumnegative(numt_t xt, const nump_t *xp)
{
  assert(NUMT_IS_VALID(xt) && "unsupported number type");
  if (!isreal(xt)) return -1;
  return realcmp0(xt, xp, NCMP_LT);
}

/* exact?(x) and inexact?(x) */
static int gnumexactp(numt_t xt, const nump_t *xp)
{
  assert(NUMT_IS_VALID(xt) && "unsupported number type");
  (void)xp;
  return isrect(xt);
}

static int gnuminexactp(numt_t xt, const nump_t *xp)
{
  assert(NUMT_IS_VALID(xt) && "unsupported number type");
  (void)xp;
  return !isrect(xt);
}

static int gnumintegerp(numt_t xt, const nump_t *xp)
{
  assert(NUMT_IS_VALID(xt) && "unsupported number type");
  if (!isreal(xt)) return 0;
  return isflo(xt) ? flisint(getflo(xp)) : isint(xt);
}

static int gnumrationalp(numt_t xt, const nump_t *xp)
{
  assert(NUMT_IS_VALID(xt) && "unsupported number type");
  if (!isreal(xt)) return 0;
  if (isflo(xt)) {
    double x = getflo(xp);
    return (x == x) && (x > -HUGE_VAL) && (x < HUGE_VAL);
  }
  return 1;  /* all exact reals are rational */
}

static int gnumrealp(numt_t xt, const nump_t *xp)
{
  assert(NUMT_IS_VALID(xt) && "unsupported number type");
  return isreal(xt);
}

static int gnumfinitep(numt_t xt, const nump_t *xp)
{
  if (isrect(xt)) return 1;  /* all exact numbers are finite */
  if (NUMT_COM_R(xt) == NUMT_FLO) {
    double x = getflo(xp);
    if (x != x || x <= -HUGE_VAL || x >= HUGE_VAL) return 0;
  }
  if (NUMT_COM_I(xt) == NUMT_FLO) {
    double y = getflo(xp+2);
    if (y != y || y <= -HUGE_VAL || y >= HUGE_VAL) return 0;
  }
  return 1;
}

static int gnuminfinitep(numt_t xt, const nump_t *xp)
{
  if (isrect(xt)) return 0;  /* all exact numbers are finite */
  if (NUMT_COM_R(xt) == NUMT_FLO) {
    double x = getflo(xp);
    if (x <= -HUGE_VAL || x >= HUGE_VAL) return 1;
  }
  if (NUMT_COM_I(xt) == NUMT_FLO) {
    double y = getflo(xp+2);
    if (y <= -HUGE_VAL || y >= HUGE_VAL) return 1;
  }
  return 0;
}

static int gnumnanp(numt_t xt, const nump_t *xp)
{
  assert(NUMT_IS_VALID(xt) && "unsupported number type");
  if (isrect(xt)) return 0;
  else {
    double x = getflo(xp);
    if (x != x) return 1;
    if (NUMT_COM_I(xt)) {
      double y = getflo(xp+2);
      if (y != y) return 1;
    }
    return 0;
  }
}


/* returns NUMT_NONE and sets errno on failure */
numt_t strtognum(nump_t *zp, const char *str, char **endp, int radix)
{
  numt_t zt = NUMT_NONE; char *cp;
  int forceie = 0; /* -1=#i 1=#e */
  int flags = 0; /* CNF_XXX|... */
  assert(str); 
  if (radix < 2 || radix > 36) return setfail(EDOM);
  /* returns start of complex part and sets radix/ie; returns NULL on errors */
  if ((cp = check_number(str, &radix, &forceie, &flags)) == NULL) { 
    /* invalid number syntax */
    if (endp) *endp = (char*)str;
    return setfail(EDOM);
  }
  if (flags & (CNF_DOTEXP|CNF_INFNAN|CNF_NONR7)) {
    if (radix != 2 && radix != 8 && radix != 10 && radix != 16) {
      /* invalid floating-point number syntax */
      if (endp) *endp = (char*)str;
      return setfail(EDOM); 
    }
  }
  /* positon at the start of complex */
  str = (const char *)cp;
  if (forceie < 0) { /* as inexact */
    zt = strtocomp(zp, str, endp, radix);
  } else if (forceie > 0) { /* as exact */
    zt = strtorect(zp, str, endp, radix);
    /* catch polar -> inexact */
    if (!isrect(zt)) zt = (numfini(zt, zp), setfail(EDOM)); 
  } else if (flags & CNF_DOTEXP) { /* has elements of inexact notation */
    zt = strtocomp(zp, str, endp, radix);
  } else { /* no elements of inexact notation */
    zt = strtorect(zp, str, endp, radix);
    /* polar -> inexact is ok */
    if (iscomp(zt) && errno == ERANGE) errno = 0;
    if (errno) zt = (numfini(zt, zp), setfail(EDOM));
  }
  return zt;
}

/* # of chars needed for x in radix; return 0 on invalid radix  */
size_t gnumfmtsize(numt_t xt, const nump_t *xp, int radix, int mode, int prc)
{
  assert(NUMT_IS_VALID(xt) && "unsupported number type");
  if (isrect(xt)) {
    if (radix < 2 || radix > 36) return 0; 
    return rectfmtsize(xt, xp, radix);
  } else {
    if (radix != 2 && radix != 8 && radix != 10 && radix != 16) return 0; 
    return compfmtsize(xt, xp, radix, prc);
  }
}

/* format x into buffer; len should be as calculated by gnumfmtsize;
 * returns ptr to first char of zero-terminated result in buffer
 * or NULL if an inexact number is printed in wrong radix. */
char *gnumtostr(char *buffer, size_t len, numt_t xt, const nump_t *xp, int radix, int mode, int prc)
{
  assert(NUMT_IS_VALID(xt) && "unsupported number type");
  if (isrect(xt)) {
    if (radix < 2 || radix > 36) return NULL; 
    return recttostr(buffer, len, xt, xp, radix);
  } else {
    if (radix != 2 && radix != 8 && radix != 10 && radix != 16) return NULL; 
    return comptostr(buffer, len, xt, xp, radix, mode, prc);
  }
}

/* 'generic' writer for gnums; returns 0 or -1 on invalid radix */
int gnumwrite(numt_t xt, const nump_t *xp, int radix, int mode, int prc, int (*pf)(int, void*), void *pd)
{
  size_t len; char *buf, *s;
  len = gnumfmtsize(xt, xp, radix, mode, prc);
  if (!len) return -1; 
  buf = cxm_cknull(malloc(len+1), "realloc(gnumwrite)");
  s = gnumtostr(buf, len, xt, xp, radix, mode, prc); assert(s);
  for (; *s; ++s) (*pf)(*s, pd);
  free(buf);
  return 0;
}


/* bignums as skint objects */

static cxtype_t cxt_bignum = { "bignum", (void (*)(void *))bnfree };
cxtype_t *BIGNUM_NTAG = &cxt_bignum;


/* fatnums as skint objects */

void fnfini(fatnum_t *n)
{
  if (n != NULL) numfini(n->t, n->p);
}
void fnfree(fatnum_t *n)
{
  if (n == NULL) return;
  numfini(n->t, n->p);
  free(n);
}

int fneqn(const fatnum_t *fx, const fatnum_t *fy)
  { return gnumeqn(fx->t, fx->p, fy->t, fy->p); }

int fneqv(const fatnum_t *fx, const fatnum_t *fy)
  { return gnumeqv(fx->t, fx->p, fy->t, fy->p); }

int fncmpc(const fatnum_t *fx, const fatnum_t *fy, ncmp_t c)
  { return gnumcmpc(fx->t, fx->p, fy->t, fy->p, c); }

int fnodd(const fatnum_t *fx)
  { return gnumodd(fx->t, fx->p); }

int fneven(const fatnum_t *fx)
  { return gnumeven(fx->t, fx->p); }

int fnisex(const fatnum_t *fx)
  { return gnumexactp(fx->t, fx->p); }

int fnisin(const fatnum_t *fx)
  { return gnuminexactp(fx->t, fx->p); }

int fnzero(const fatnum_t *fx)
  { return gnumzero(fx->t, fx->p); }

int fnispos(const fatnum_t *fx)
  { return gnumpositive(fx->t, fx->p); }

int fnisneg(const fatnum_t *fx)
  { return gnumnegative(fx->t, fx->p); }

int fnisint(const fatnum_t *fx)
  { return gnumintegerp(fx->t, fx->p); }

int fnisrat(const fatnum_t *fx)
  { return gnumrationalp(fx->t, fx->p); }

int fnisreal(const fatnum_t *fx)
  { return gnumrealp(fx->t, fx->p); }

int fnisran(const fatnum_t *fx)
  { return !NUMT_COM_I(fx->t) && NUMT_RAT_D(fx->t); }

int fniscon(const fatnum_t *fx)
  { return iscomp(fx->t); }

int fnisren(const fatnum_t *fx)
  { return NUMT_COM_I(fx->t) && !iscomp(fx->t); }

int fnisfin(const fatnum_t *fx)
  { return gnumfinitep(fx->t, fx->p); }

int fnisinf(const fatnum_t *fx)
  { return gnuminfinitep(fx->t, fx->p); }

int fnisnan(const fatnum_t *fx)
  { return gnumnanp(fx->t, fx->p); }

#define setmsg(fz, m) strncpy(fz->u.msg, m, sizeof(fz->u.msg))

int fnabs(fatnum4r_t *fz, const fatnum_t *fx)
  { numt_t zt = gnumabs(fz->u.p, fx->t, fx->p);
    if (zt == NUMT_NONE) setmsg(fz, "abs: domain error");
    fz->t = zt; return zt != NUMT_NONE; }

int fnneg(fatnum4r_t *fz, const fatnum_t *fx)
  { numt_t zt = gnumneg(fz->u.p, fx->t, fx->p);
    if (zt == NUMT_NONE) setmsg(fz, "neg: domain error");
    fz->t = zt; return zt != NUMT_NONE; }

int fntoex(fatnum4r_t *fz, const fatnum_t *fx)
  { numt_t zt = gnumtoex(fz->u.p, fx->t, fx->p);
    if (zt == NUMT_NONE) setmsg(fz, "exact: domain error");
    fz->t = zt; return zt != NUMT_NONE; }

int fntoin(fatnum4r_t *fz, const fatnum_t *fx)
  { numt_t zt = gnumtoin(fz->u.p, fx->t, fx->p);
    if (zt == NUMT_NONE) setmsg(fz, "inexact: domain error");
    fz->t = zt; return zt != NUMT_NONE; }

int fnfloor(fatnum4r_t *fz, const fatnum_t *fx)
  { numt_t zt = gnumfloor(fz->u.p, fx->t, fx->p);
    if (zt == NUMT_NONE) setmsg(fz, "floor: domain error");
    fz->t = zt; return zt != NUMT_NONE; }

int fnceil(fatnum4r_t *fz, const fatnum_t *fx)
  { numt_t zt = gnumceil(fz->u.p, fx->t, fx->p);
    if (zt == NUMT_NONE) setmsg(fz, "ceiling: domain error");
    fz->t = zt; return zt != NUMT_NONE; }

int fntrunc(fatnum4r_t *fz, const fatnum_t *fx)
  { numt_t zt = gnumtrunc(fz->u.p, fx->t, fx->p);
    if (zt == NUMT_NONE) setmsg(fz, "truncate: domain error");
    fz->t = zt; return zt != NUMT_NONE; }

int fnround(fatnum4r_t *fz, const fatnum_t *fx)
  { numt_t zt = gnumround(fz->u.p, fx->t, fx->p);
    if (zt == NUMT_NONE) setmsg(fz, "round: domain error");
    fz->t = zt; return zt != NUMT_NONE; }

int fnnumer(fatnum4r_t *fz, const fatnum_t *fx)
  { numt_t zt = gnumnumer(fz->u.p, fx->t, fx->p);
    if (zt == NUMT_NONE) setmsg(fz, "numerator: domain error");
    fz->t = zt; return zt != NUMT_NONE; }

int fndenom(fatnum4r_t *fz, const fatnum_t *fx)
  { numt_t zt = gnumdenom(fz->u.p, fx->t, fx->p);
    if (zt == NUMT_NONE) setmsg(fz, "denominator: domain error");
    fz->t = zt; return zt != NUMT_NONE; }

int fnrpart(fatnum4r_t *fz, const fatnum_t *fx)
  { numt_t zt = gnumreal(fz->u.p, fx->t, fx->p);
    if (zt == NUMT_NONE) setmsg(fz, "real-part: domain error");
    fz->t = zt; return zt != NUMT_NONE; }

int fnipart(fatnum4r_t *fz, const fatnum_t *fx)
  { numt_t zt = gnumimag(fz->u.p, fx->t, fx->p);
    if (zt == NUMT_NONE) setmsg(fz, "imag-part: domain error");
    fz->t = zt; return zt != NUMT_NONE; }

int fnmagn(fatnum4r_t *fz, const fatnum_t *fx)
  { numt_t zt = gnummagn(fz->u.p, fx->t, fx->p);
    if (zt == NUMT_NONE) setmsg(fz, "magnitude: domain error");
    fz->t = zt; return zt != NUMT_NONE; }

int fnangl(fatnum4r_t *fz, const fatnum_t *fx)
  { numt_t zt = gnumangl(fz->u.p, fx->t, fx->p);
    if (zt == NUMT_NONE) setmsg(fz, "angle: domain error");
    fz->t = zt; return zt != NUMT_NONE; }

int fnexp(fatnum4r_t *fz, const fatnum_t *fx)
  { numt_t zt = gnumexp(fz->u.p, fx->t, fx->p);
    if (zt == NUMT_NONE) setmsg(fz, "exp: domain error");
    fz->t = zt; return zt != NUMT_NONE; }

int fnlog(fatnum4r_t *fz, const fatnum_t *fx)
  { numt_t zt = gnumlog(fz->u.p, fx->t, fx->p);
    if (zt == NUMT_NONE) setmsg(fz, "log: domain error");
    fz->t = zt; return zt != NUMT_NONE; }

int fnsin(fatnum4r_t *fz, const fatnum_t *fx)
  { numt_t zt = gnumsin(fz->u.p, fx->t, fx->p);
    if (zt == NUMT_NONE) setmsg(fz, "sin: domain error");
    fz->t = zt; return zt != NUMT_NONE; }

int fncos(fatnum4r_t *fz, const fatnum_t *fx)
  { numt_t zt = gnumcos(fz->u.p, fx->t, fx->p);
    if (zt == NUMT_NONE) setmsg(fz, "cos: domain error");
    fz->t = zt; return zt != NUMT_NONE; }

int fntan(fatnum4r_t *fz, const fatnum_t *fx)
  { numt_t zt = gnumtan(fz->u.p, fx->t, fx->p);
    if (zt == NUMT_NONE) setmsg(fz, "tan: domain error");
    fz->t = zt; return zt != NUMT_NONE; }

int fnasin(fatnum4r_t *fz, const fatnum_t *fx)
  { numt_t zt = gnumasin(fz->u.p, fx->t, fx->p);
    if (zt == NUMT_NONE) setmsg(fz, "asin: domain error");
    fz->t = zt; return zt != NUMT_NONE; }

int fnacos(fatnum4r_t *fz, const fatnum_t *fx)
  { numt_t zt = gnumacos(fz->u.p, fx->t, fx->p);
    if (zt == NUMT_NONE) setmsg(fz, "acos: domain error");
    fz->t = zt; return zt != NUMT_NONE; }

int fnatan(fatnum4r_t *fz, const fatnum_t *fx)
  { numt_t zt = gnumatan(fz->u.p, fx->t, fx->p);
    if (zt == NUMT_NONE) setmsg(fz, "atan: domain error");
    fz->t = zt; return zt != NUMT_NONE; }

int fnsqrt(fatnum4r_t *fz, const fatnum_t *fx)
  { numt_t zt = gnumsqrt(fz->u.p, fx->t, fx->p);
    if (zt == NUMT_NONE) setmsg(fz, "sqrt: domain error");
    fz->t = zt; return zt != NUMT_NONE; }

int fnisqrt(fatnum4r_t *fz, fatnum4r_t *fr, const fatnum_t *fx)
  { numt_t zt, rt;
    gnumisqrt(&zt, fz->u.p, &rt, fr->u.p, fx->t, fx->p);
    fz->t = zt; fr->t = rt;
    if (zt == NUMT_NONE) setmsg(fz, "exact-integer-sqrt: domain error");
    return zt != NUMT_NONE; }

int fnmax(fatnum4r_t *fz, const fatnum_t *fx, const fatnum_t *fy)
  { numt_t zt = gnummax(fz->u.p, fx->t, fx->p, fy->t, fy->p);
    if (zt == NUMT_NONE) setmsg(fz, "max: domain error");
    fz->t = zt; return zt != NUMT_NONE; }

int fnmin(fatnum4r_t *fz, const fatnum_t *fx, const fatnum_t *fy)
  { numt_t zt = gnummin(fz->u.p, fx->t, fx->p, fy->t, fy->p);
    if (zt == NUMT_NONE) setmsg(fz, "min: domain error");
    fz->t = zt; return zt != NUMT_NONE; }

int fnadd(fatnum4r_t *fz, const fatnum_t *fx, const fatnum_t *fy)
  { numt_t zt = gnumadd(fz->u.p, fx->t, fx->p, fy->t, fy->p);
    if (zt == NUMT_NONE) setmsg(fz, "+: domain error");
    fz->t = zt; return zt != NUMT_NONE; }

int fnsub(fatnum4r_t *fz, const fatnum_t *fx, const fatnum_t *fy)
  { numt_t zt = gnumsub(fz->u.p, fx->t, fx->p, fy->t, fy->p);
    if (zt == NUMT_NONE) setmsg(fz, "-: domain error");
    fz->t = zt; return zt != NUMT_NONE; }

int fnmul(fatnum4r_t *fz, const fatnum_t *fx, const fatnum_t *fy)
  { numt_t zt = gnummul(fz->u.p, fx->t, fx->p, fy->t, fy->p);
    if (zt == NUMT_NONE) setmsg(fz, "*: domain error");
    fz->t = zt; return zt != NUMT_NONE; }

int fndiv(fatnum4r_t *fz, const fatnum_t *fx, const fatnum_t *fy)
  { numt_t zt = gnumdiv(fz->u.p, fx->t, fx->p, fy->t, fy->p);
    if (zt == NUMT_NONE) setmsg(fz, "/: domain error");
    fz->t = zt; return zt != NUMT_NONE; }

int fngcd(fatnum4r_t *fz, const fatnum_t *fx, const fatnum_t *fy)
  { numt_t zt = gnumgcd(fz->u.p, fx->t, fx->p, fy->t, fy->p);
    if (zt == NUMT_NONE) setmsg(fz, "gcd: domain error");
    fz->t = zt; return zt != NUMT_NONE; }

int fnquo(fatnum4r_t *fz, const fatnum_t *fx, const fatnum_t *fy)
  { numt_t zt = gnumtquo(fz->u.p, fx->t, fx->p, fy->t, fy->p);
    if (zt == NUMT_NONE) setmsg(fz, "truncate-quotient: domain error");
    fz->t = zt; return zt != NUMT_NONE; }

int fnrem(fatnum4r_t *fz, const fatnum_t *fx, const fatnum_t *fy)
  { numt_t zt = gnumtrem(fz->u.p, fx->t, fx->p, fy->t, fy->p);
    if (zt == NUMT_NONE) setmsg(fz, "truncate-remainder: domain error");
    fz->t = zt; return zt != NUMT_NONE; }

int fnmqu(fatnum4r_t *fz, const fatnum_t *fx, const fatnum_t *fy)
  { numt_t zt = gnumfquo(fz->u.p, fx->t, fx->p, fy->t, fy->p);
    if (zt == NUMT_NONE) setmsg(fz, "floor-quotient: domain error");
    fz->t = zt; return zt != NUMT_NONE; }

int fnmlo(fatnum4r_t *fz, const fatnum_t *fx, const fatnum_t *fy)
  { numt_t zt = gnumfrem(fz->u.p, fx->t, fx->p, fy->t, fy->p);
    if (zt == NUMT_NONE) setmsg(fz, "floor-remainder: domain error");
    fz->t = zt; return zt != NUMT_NONE; }

int fnlogn(fatnum4r_t *fz, const fatnum_t *fx, const fatnum_t *fy)
  { numt_t zt = gnumlogn(fz->u.p, fx->t, fx->p, fy->t, fy->p);
    if (zt == NUMT_NONE) setmsg(fz, "log: domain error");
    fz->t = zt; return zt != NUMT_NONE; }

int fnpow(fatnum4r_t *fz, const fatnum_t *fx, const fatnum_t *fy)
  { numt_t zt = gnumexpt(fz->u.p, fx->t, fx->p, fy->t, fy->p);
    if (zt == NUMT_NONE) setmsg(fz, "expt: domain error");
    fz->t = zt; return zt != NUMT_NONE; }

int fnatan2(fatnum4r_t *fz, const fatnum_t *fy, const fatnum_t *fx)
  { numt_t zt = gnumatan2(fz->u.p, fy->t, fy->p, fx->t, fx->p);
    if (zt == NUMT_NONE) setmsg(fz, "atan: domain error");
    fz->t = zt; return zt != NUMT_NONE; }

int fnmkrec(fatnum4r_t *fz, const fatnum_t *fx, const fatnum_t *fy)
  { numt_t zt = gnummkrec(fz->u.p, fx->t, fx->p, fy->t, fy->p);
    if (zt == NUMT_NONE) setmsg(fz, "make-rectangular: domain error");
    fz->t = zt; return zt != NUMT_NONE; }

int fnmkpol(fatnum4r_t *fz, const fatnum_t *fx, const fatnum_t *fy)
  { numt_t zt = gnummkpol(fz->u.p, fx->t, fx->p, fy->t, fy->p);
    if (zt == NUMT_NONE) setmsg(fz, "make-polar: domain error");
    fz->t = zt; return zt != NUMT_NONE; }
    
int fnnot(fatnum4r_t *fz, const fatnum_t *fx)
  { numt_t zt = NUMT_NONE;
    if (isint(fx->t)) zt = intnot(fz->u.p, fx->t, fx->p);
    if (zt == NUMT_NONE) setmsg(fz, "bitwise-not: domain error");
    fz->t = zt; return zt != NUMT_NONE; }

int fnand(fatnum4r_t *fz, const fatnum_t *fx, const fatnum_t *fy)
  { numt_t zt = NUMT_NONE;
    if (isint(fx->t) && isint(fy->t)) zt = intand(fz->u.p, fx->t, fx->p, fy->t, fy->p);
    if (zt == NUMT_NONE) setmsg(fz, "bitwise-and: domain error");
    fz->t = zt; return zt != NUMT_NONE; }

int fnior(fatnum4r_t *fz, const fatnum_t *fx, const fatnum_t *fy)
  { numt_t zt = NUMT_NONE;
    if (isint(fx->t) && isint(fy->t)) zt = intior(fz->u.p, fx->t, fx->p, fy->t, fy->p);
    if (zt == NUMT_NONE) setmsg(fz, "bitwise-ior: domain error");
    fz->t = zt; return zt != NUMT_NONE; }

int fnxor(fatnum4r_t *fz, const fatnum_t *fx, const fatnum_t *fy)
  { numt_t zt = NUMT_NONE;
    if (isint(fx->t) && isint(fy->t)) zt = intxor(fz->u.p, fx->t, fx->p, fy->t, fy->p);
    if (zt == NUMT_NONE) setmsg(fz, "bitwise-xor: domain error");
    fz->t = zt; return zt != NUMT_NONE; }

int fnash(fatnum4r_t *fz, const fatnum_t *fx, const fatnum_t *fy)
  { numt_t zt = NUMT_NONE;
    if (isint(fx->t) && isint(fy->t)) zt = intash(fz->u.p, fx->t, fx->p, fy->t, fy->p);
    if (zt == NUMT_NONE) setmsg(fz, "arithmetic-shift: domain error");
    fz->t = zt; return zt != NUMT_NONE; }

int fnlen(fatnum4r_t *fz, const fatnum_t *fx)
  { numt_t zt = NUMT_NONE;
    if (isint(fx->t)) zt = intlen(fz->u.p, fx->t, fx->p);
    if (zt == NUMT_NONE) setmsg(fz, "integer-length: domain error");
    fz->t = zt; return zt != NUMT_NONE; }
    
int fnbtc(fatnum4r_t *fz, const fatnum_t *fx)
  { numt_t zt = NUMT_NONE;
    if (isint(fx->t)) zt = intbtc(fz->u.p, fx->t, fx->p);
    if (zt == NUMT_NONE) setmsg(fz, "bit-count: domain error");
    fz->t = zt; return zt != NUMT_NONE; }


static cxtype_t cxt_fatnum = { "fatnum", (void (*)(void *))fnfree };
cxtype_t *FATNUM_NTAG = &cxt_fatnum;

fatnum_t *dupfatnum(fatnum_t *fn) /* shallow copy! */
{
  fatnum_t *r; size_t pc, sz; /* parts count */
  assert(fn); assert(fn->t);
  if ((fn->t & NUMT_SS_MASK) == fn->t) pc = 1; 
  else if ((fn->t & NUMT_DS_MASK) == fn->t) pc = 2; 
  else pc = 4; 
  sz = sizeof(fatnum_t) + sizeof(nump_t)*(pc-1);   
  r = cxm_cknull(calloc(sz, 1), "malloc(fatnum)");
  memcpy(r, fn, sz);
  return r;
}

/* 'generic' writer for fatnums */
int wrfn(const fatnum_t *n, int radix, int mode, int prc, int (*pf)(int, void*), void *pd)
{
  assert(n); assert(n->t);
  return gnumwrite(n->t, n->p, radix, mode, prc, pf, pd);
}


/* tower version of strtonum */
numt_t strtonum(fatnum4_t *f4, const char *s, char **endp, int radix)
{ 
  int eno = errno; errno = 0;
  f4->t = strtognum(&f4->p[0], s, endp, radix);
  if (errno == ERANGE) errno = 0; /* allow polar-to-inexact casting for now */ 
  if (errno) { numfini(f4->t, &f4->p[0]); f4->t = NUMT_NONE; } else errno = eno;
  return f4->t;
}
