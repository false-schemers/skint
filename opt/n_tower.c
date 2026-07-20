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

/* helper to copy sign of y to x, correctly handling signed zero */
static double c90_copysign(double x, double y) 
{
  if (y < 0.0) return -fabs(x);
  if (y > 0.0) return fabs(x);
  return (1.0/y < 0.0) ? -fabs(x): fabs(x);
}

/* C90 log1p replacement, good near zero.  Requires x > -1. */
static double c90_log1p(double x)
{
  double u;
  if (x == 0.0) return x;
  u = 1.0 + x;
  if (u == 1.0) return x;
  return log(u) * (x / (u - 1.0));
}

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
    r = c90_log1p(ax + (ax * ax) / (1.0 + s));
  }
  return c90_copysign(r, x);
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
  m = c90_hypot(rx, ix);
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
    *pre = c90_copysign(M_PI_2, x);
    *pim = c90_copysign(0.0, y);
    return;
  }
  /* re part */
  if (x == 0.0) {
    if (ay > 1.0) re = c90_copysign(M_PI_2, x);
    else re = x; /* preserve +0.0 / -0.0 */
  } else if (ax >= C90_BIG || ay >= C90_BIG) {
    re = c90_copysign(M_PI_2, x);
  } else {
    double den;
    if (ax < ay) den = (1.0 - ay) * (1.0 + ay) - x * x;
    else den = (1.0 - ax) * (1.0 + ax) - y * y;
    re = 0.5 * atan2(2.0 * x, den);
  }
  /* im part */
  if (fabs(ay - 1.0) < 0.5) { /* avoid losing x*x */
    double hp = c90_hypot(x, y + 1.0);
    double hm = c90_hypot(x, y - 1.0);
    if (hp == 0.0) im = -HUGE_VAL; /* z = -i */
    else if (hm == 0.0) im = HUGE_VAL;  /* z = +i */
    else im = 0.5 * (log(hp) - log(hm));
  } else { /* 1/4 log1p(4y / (x^2 + (y-1)^2)) */
    double ym = y - 1.0, h = c90_hypot(x, ym), t;
    if (h == 0.0) im = HUGE_VAL;  /* z = +i */
    else {
      if (h >= C90_BIG) { /* avoid h*h overflow */
        t = (4.0 / h) * (y / h);
      } else {
        double den = x * x + ym * ym;
        t = 4.0 * y / den;
      }
      im = 0.25 * c90_log1p(t);
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
  return c90_hypot(x, y);
}

#endif /* HAVE_C99_COMPLEX */


/* number <-> string conversions */

/* convert char to integer for any radix up to 16 */
static int char_to_val(int c, unsigned radix) 
{
  int d = -1;
  if (c >= '0' && c <= '9') d = c - '0';
  else if (c >= 'a' && c <= 'f') d = c - 'a' + 10;
  else if (c >= 'A' && c <= 'F') d = c - 'A' + 10;
  return (d >= 0 && (unsigned)d < radix) ? d : -1;
}

/* convert bin/qua/oct/hex floating-point notation to double
 * bbits: bits per mantissa digit (1=bin, 2=qua, 3=oct, 4=hex)
 * 'e' (ebase = eradix = 1<<bbits) or 'p' (ebase = 2, eradix = 10) */
double po2b_atod(const char *s, char **endp, unsigned bbits) 
{
  const char *p = s; uint64_t sig = 0; double d;
  int neg, dotpos = -1, ndigits = 0, hxcount = 0, echar; 
  int first_digit_idx = -1, first_digit_val = 0, sticky = 0;
  long pexp = 0; unsigned base = 1 << bbits;

  while (isspace((unsigned char)*p)) ++p;
  neg = (*p == '-');
  if (*p == '-' || *p == '+') ++p;

  /* parse mantissa */
  for (;; ++p) {
    int c = (unsigned char)*p;
    int d = (c == '.' && dotpos < 0) ? -2 : char_to_val(c, base);
    if (d == -1) break; /* not a valid digit */
    if (d == -2) { dotpos = ndigits; continue; }
    if (d != 0 || first_digit_idx >= 0) {
      if (first_digit_idx < 0) {
        first_digit_idx = ndigits;
        first_digit_val = d;
      }
      /* keep as many digits as fits in 60 bits to prevent uint64_t overflow */
      if ((hxcount + 1) * bbits <= 60) {
        sig = (sig << bbits) | (unsigned)d;
        hxcount++;
      } else if (d > 0) {
        sticky = 1;
      }
    }
    ++ndigits;
  }

  if (ndigits == 0) {
    if (endp) *endp = (char *)s;
    errno = EDOM; return 0.0;
  }
  if (dotpos < 0) dotpos = ndigits;

  /* parse exponent */
  echar = tolower((unsigned char)*p);
  if (echar == 'e' || echar == 'p') {
    const char *r = p + 1;
    int pneg = (*r == '-');
    unsigned eradix = (echar == 'p') ? 10 : base;
    if (*r == '-' || *r == '+') r++;
    if (char_to_val(*r, eradix) >= 0) {
      pexp = 0;
      while (1) {
        int d = char_to_val(*r, eradix);
        if (d < 0) break;
        if (pexp < 1000000) pexp = pexp * eradix + d;
        ++r;
      }
      if (pneg) pexp = -pexp;
      p = r;
    }
  }
  if (endp) *endp = (char *)p;

  if (first_digit_idx < 0) { 
    return neg ? -0.0 : 0.0;
  } else {
    int msb_offset = 0;
    int temp_v = first_digit_val;
    int target_bits, shift, round_bit;
    long ldx, exp_msb, binary_pexp;

    /* MSB offset within the first digit (e.g., octal '7' is 2, binary '1' is 0) */
    while (temp_v >>= 1) ++msb_offset;
    /* calculate binary weight */
    binary_pexp = (echar == 'p') ? pexp : pexp * (long)bbits;
    ldx = binary_pexp + (long)(dotpos - 1 - first_digit_idx) * (long)bbits;
    ldx -= (long)(hxcount - 1) * (long)bbits;
    /* binary weight of the MSB */
    exp_msb = ldx + (long)(hxcount - 1) * (long)bbits + msb_offset;

    /* determine precision (IEEE 754 double) */
    if (exp_msb >= (long)DBL_MIN_EXP - 1) {
      target_bits = DBL_MANT_DIG;
    } else {
      target_bits = (int)(exp_msb - (long)(DBL_MIN_EXP - DBL_MANT_DIG - 1));
      if (target_bits < 0) target_bits = 0;
    }

    /* rounding logic (RNE) */
    /* total bits currently in sig: (digits - 1)*bbits + (msb_offset + 1) */
    shift = ((hxcount - 1) * bbits + msb_offset + 1) - target_bits;
    if (shift > 0) {
      if (shift >= 64) {
        sticky |= (sig != 0);
        sig = 0; round_bit = 0;
      } else {
        sticky |= ((sig & ((UINT64_C(1) << (shift - 1)) - 1)) != 0);
        round_bit = (int)((sig >> (shift - 1)) & 1);
        sig >>= shift;
      }
      if (round_bit && (sticky || (sig & 1))) {
        sig++;
        if (target_bits > 0 && (sig >= (UINT64_C(1) << target_bits))) {
          sig >>= 1; ldx++;
        }
      }
      ldx += shift;
    } else if (shift < 0) {
      sig <<= -shift;
      ldx += shift;
    }

    /* result */
    d = ldexp((double)sig, (int)ldx);
    if (d <= -HUGE_VAL || d >= HUGE_VAL) errno = ERANGE;
    else if (d == 0.0) errno = ERANGE; /* subnormal underflow */
    return neg ? -d : d;
  }
}

#define bin_strtod(s, endp) po2b_atod(s, endp, 1)
#define oct_strtod(s, endp) po2b_atod(s, endp, 3)
#define hex_strtod(s, endp) po2b_atod(s, endp, 4)

/* derived constants for buffer safety */
/* binary (bbits=1) is the worst case for digit count: DBL_MANT_DIG = 53 */
#define GN_DIG_MAX (1 + (DBL_MANT_DIG - 1) + 4)
/* buffer must handle the worst-case binary shift: ~1024 digits + digits + margin */
#define GN_BUFSIZE (DBL_MAX_EXP + GN_DIG_MAX + 64)

int gn_digit(int x) { return (char)(x < 10 ? ('0' + x) : ('A' + (x - 10))); }

/* render the exponent in a specific radix (2 to 16) */
static char *render_exp(long val, unsigned radix, char *out) 
{
  char tmp[64];
  int i = 0, j = 0, neg = 0;
  if (val < 0) { neg = 1; val = -val; }
  do { tmp[i++] = gn_digit(val % radix); val /= radix; } while (val > 0);
  if (neg) out[j++] = '-';
  while (i > 0) out[j++] = tmp[--i];
  out[j] = '\0';
  return out;
}

/* convert double to bin/qua/oct/hex floating-point notation
 * bbits: 1 (bin), 2 (qua), 3 (oct), 4 (hex)
 * echar: 'e' (base^exp) or 'p' (2^exp)
 * eradix: 2, 8, 10, or 16 (radix to print the exponent part) */
char *po2b_dtoa(double x, char *buf, size_t buflen, unsigned bbits, int echar) 
{
  double f, adj;
  int neg, e, rem, i, pos, ndig;
  unsigned base = 1 << bbits;
  int dig[GN_DIG_MAX], last_nz;
  int len_std, len_shift, shift;
  long bin_exp_at_dot;
  char exp_part[64], val_part[64];

  assert(buflen >= GN_BUFSIZE);
  assert(bbits >= 1 && bbits <= 4);

  if (x != x || x >= HUGE_VAL || x <= -HUGE_VAL) return NULL;
  if (x < 0.0 || (x == 0.0 && 1.0/x < 0.0)) { neg = 1; x = -x; } 
  else neg = 0;
  if (x == 0.0) { strcpy(buf, neg ? "-0.0" : "0.0"); return buf; }

  f = frexp(x, &e);

  /* normalize leading digit to be in [1, 2^bbits) */
  rem = e % (int)bbits;
  if (rem <= 0) rem += bbits;

  adj = ldexp(f, rem);
  bin_exp_at_dot = (long)e - rem; /* guaranteed multiple of bbits */

  /* NB: leading dig holds 'rem' bits, calc total count */
  ndig = 1 + (DBL_MANT_DIG - rem + bbits - 1) / bbits;
  assert(ndig <= GN_DIG_MAX);

  /* extract digits */
  for (i = 0; i < GN_DIG_MAX; ++i) dig[i] = 0;
  for (i = 0; i < ndig; ++i) {
    int d = (int)adj;
    dig[i] = d;
    adj = ldexp(adj - (double)d, bbits);
  }

  last_nz = ndig - 1;
  while (last_nz > 0 && dig[last_nz] == 0) --last_nz;

  /* prepare exponent string */
  exp_part[0] = echar;
  if (echar == 'p') {
    /* 'p' format always uses decimal binary exponent */
    sprintf(exp_part + 1, "%ld", bin_exp_at_dot); /* drop + for consistency */
  } else {
    /* 'e' format uses base-radix exponent */
    long base_exp = bin_exp_at_dot / (long)bbits;
    render_exp(base_exp, base, val_part); /* does not print '+' */
    strcpy(exp_part + 1, val_part);
  }

  /* determine shortest representation */
  shift = (int)(bin_exp_at_dot / (long)bbits);

  /* standard form length: "D.FFFF" + exp_part */
  if (last_nz == 0) len_std = 1 + (int)strlen(exp_part);
  else len_std = 1 + 1 + last_nz + (int)strlen(exp_part);
  /* we 'pessimize' len_std to widen the 'fixed' range */
  len_std += (bin_exp_at_dot >= 0) ? 3 : 2; /* as if '+' is there */

  /* shifted form length */
  if (shift >= 0) {
    int digits_before_dot = shift + 1;
    int digits_after_dot = (last_nz > shift) ? (last_nz - shift) : 1;
    len_shift = digits_before_dot + 1 + digits_after_dot;
  } else {
    int abs_shift = -shift;
    len_shift = 2 + (abs_shift - 1) + (last_nz + 1);
  }

  /* formatting */
  pos = 0;
  if (neg) buf[pos++] = '-';

  if (len_shift <= len_std) {
    if (shift >= 0) {
      for (i = 0; i <= shift || i <= last_nz; i++) {
        buf[pos++] = gn_digit(i < GN_DIG_MAX ? dig[i] : 0);
        if (i == shift) buf[pos++] = '.';
      }
      if (buf[pos-1] == '.') buf[pos++] = '0';
    } else {
      int abs_shift = -shift;
      buf[pos++] = '0'; buf[pos++] = '.';
      for (i = 0; i < abs_shift - 1; i++) buf[pos++] = '0';
      for (i = 0; i <= last_nz; i++) buf[pos++] = gn_digit(dig[i]);
    }
    buf[pos] = '\0';
  } else {
    buf[pos++] = gn_digit(dig[0]);
    if (last_nz > 0) {
      buf[pos++] = '.';
      for (i = 1; i <= last_nz; i++) buf[pos++] = gn_digit(dig[i]);
    }
    buf[pos] = '\0';
    strcat(buf, exp_part);
  }

  return buf;
}

#define FN_BIN_BUFSIZE GN_BUFSIZE
#define dtobin(x, buf, buflen) po2b_dtoa(x, buf, buflen, 1, 'e')
#define dtooct(x, buf, buflen) po2b_dtoa(x, buf, buflen, 3, 'e')
#define dtohex(x, buf, buflen) po2b_dtoa(x, buf, buflen, 4, 'p')


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
  if (radix == 2 || radix == 8 || radix == 10) {
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


/* double <-> string conversions */

/* clears errno and sets it to EDOM or ERANGE on error */
double strtofn(const char *str, int radix, char **ep)
{
  double x; char *e = (char *)str; errno = 0;
  switch (radix) {
    case 2:  x = bin_strtod(str, &e); break;
    case 8:  x = oct_strtod(str, &e); break;
    case 10: x = strtod(str, &e); break; // dec_strtod(str, &e); break;
    case 16: x = hex_strtod(str, &e); break;
    default: x = HUGE_VAL-HUGE_VAL; assert(0);
  }
  if (str == e) errno = EDOM;
  else if (errno == ERANGE) errno = 0; /* allow over/underflow */
  else if (errno) errno = EDOM; 
  if (ep) *ep = e;
  return x;
}

/* check if x is not normally printed with a sign */
int fnsignless(double x)
{
  if (x != x || x <= -HUGE_VAL || HUGE_VAL <= x) return 0;
  if (x == 0.0 && 1.0/x < 0.0) return 0;
  return x >= 0.0;
}

/* add safety margin for "0.", "e-", etc. */
#define FN_DEC_BUFSIZE (DBL_DIG+20)

/* measure max. size of the buffer needed for printing */
size_t fnfmtsize(int radix)
{
  assert(radix == 2 || radix == 8 || radix == 10 || radix == 16);
  if (radix == 10) return FN_DEC_BUFSIZE; /* estimated up */
  else return FN_BIN_BUFSIZE; /* largest of 2/8/16 estimated up */
}

/* format finite x into buffer; len should be at least as calculated by 
 * fnfmtsize; returns buffer (prints left-to-right) or NULL on wrong radix */
char *fntostr(char *buf, size_t len, double x, int radix)
{
  char *res = buf;
  assert(len >= fnfmtsize(radix));
  switch (radix) {
    case 10: {
      /* make sure either . or e is always there */
      int eordot = 0;
      sprintf(buf, "%.*g", DBL_DIG+2, x);
      for (; *buf != 0; buf++) if (*buf == 'e' || *buf == 'E' || *buf == '.') eordot = 1;
      if (!eordot) *buf++ = '.', *buf++ = '0';
      *buf = 0;
    } break;
    case 2:  res = dtobin(x, buf, len); break;
    case 8:  res = dtooct(x, buf, len); break;
    case 16: res = dtohex(x, buf, len); break;
    default: return NULL;
  }
  assert(res == buf);
  return res;
}

static void fnprint(FILE *fp, double x, int radix)
{
  switch (radix) {
    case 10: {
      /* make sure either . or e is always there */
      char buffer[FN_DEC_BUFSIZE], *buf = buffer; int eordot = 0;
      sprintf(buffer, "%.*g", DBL_DIG+2, x);
      for (; *buf != 0; buf++) if (*buf == 'e' || *buf == 'E' || *buf == '.') eordot = 1;
      if (!eordot) *buf++ = '.', *buf++ = '0';
      *buf = 0;
      fputs(buffer, fp); 
    } break;
    case 2: {
      char buffer[FN_BIN_BUFSIZE], *buf = dtobin(x, buffer, FN_BIN_BUFSIZE);
      assert(buffer == buf);
      fputs(buf, fp); 
    } break;
    case 8: {
      char buffer[FN_BIN_BUFSIZE], *buf = dtooct(x, buffer, FN_BIN_BUFSIZE);
      assert(buffer == buf);
      fputs(buf, fp); 
    } break;
    case 16: {
      char buffer[FN_BIN_BUFSIZE], *buf = dtohex(x, buffer, FN_BIN_BUFSIZE);
      assert(buffer == buf);
      fputs(buf, fp); 
    } break;
    default: assert(0);
  }
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
  limb_t  limb[1];
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

static void bnfree(bignum_t *pn);

static bignum_t *bny_dupll(const bignum_t *lla);
static bignum_t *bndup(const bignum_t *a);
bignum_t *lltobn(long long n);

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
      w = (((uint64_t)INT64_MIN) >> LIMB_BITS) + 1;
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
  *r = (limb_t)(long long)h; 

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
  ptr = realloc(ptr, size);
  assert(ptr != NULL);
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

int bncmpl(const bignum_t *n, long long fix)
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

bignum_t *bnaddl(const bignum_t *n, long long incr)
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
  return_NORMALIZE(r, "bnaddl");
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

static bignum_t *bnashl(const bignum_t *a, long long sh)
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
      NEWBN(r, rs, "bnashl");
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

    NEWBN(r, rs, "bnashl");
    r->size = bnx_shleft(r->limb, rs, a->limb, aw, ap, sw, sp);
    r->isneg = a->isneg;
  } else {
    return bndup(a);
  }
  return_NORMALIZE(r, "bnashl");
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

bignum_t *bnmull(const bignum_t *n, long long v)
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
        bnx_failure("bnmull");
      NEWBN(r, rs, "bnmull");
      r->size = bnx_mul(r->limb, rs,
        n->limb, n->size,
        bx->limb, bx->size);
    }
    r->isneg = n->isneg;
  } else if (v == LONG_MIN) {
    bignum_t *bx = bnx_makell(&b, v);
    size_t rs = n->size + bx->size;
    if (rs < n->size)
      bnx_failure("bnmull");
    NEWBN(r, rs, "bnmull");
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
        bnx_failure("bnmull");
      NEWBN(r, rs, "bnmull");
      r->size = bnx_mul(r->limb, rs,
        n->limb, n->size,
        bx->limb, bx->size);
    }
    r->isneg = !n->isneg;
  }
  return_NORMALIZE(r, "bnmull");
}

static bignum_t *bnexpt(const bignum_t *a, unsigned long long n)
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
    if (n >= LLONG_MAX) bnx_failure("bnexpt");  
    return bnashl(bn1, n);
  }

  if (BNMINUSTWO(a)) { /* esl++ */
    if (n >= LLONG_MAX) bnx_failure("bnexpt");
    b = bnashl(bn1, n);
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
    long long v;
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
    *rem = -(long long)rl;
  else
    *rem = rl;

  if (!BNZERO(d))
    d->isneg = isneg != num->isneg;
  return_NORMALIZE(d, "bndmodl");

from_limb:
#if (ULONG_MAX > LIMB_MAX)
  {
    size_t i;
    long long v;
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
    return -(long long)reml;
  else
    return reml;
from_limb:
#if (ULONG_MAX > LIMB_MAX)
  {
    size_t i;
    long long v;
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
#if 0
    fputs("a = ", stdout); bnprint(stdout, a, 10); fputs("\n", stdout);
    fputs("b = ", stdout); bnprint(stdout, b, 10); fputs("\n", stdout);
#endif    
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
static size_t bnwidthu(const bignum_t *n)
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

bignum_t *lltobn(long long n)
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

static bignum_t *dtobn(double x)
{
  bignum_t *v = bn0;
  double f;
  int i, e;

  if (x != x || fabs(x) > DBL_MAX) /* exclude nans, infinities */
    return NULL;

  f = frexp(x, &e);

  for (i = 0; e > 0 && i < sizeof(double) * CHAR_BIT; i++, e--) {
    bignum_t *v2 = bnashl(v, 1);

    bnfree(v);
    f = f * 2.0;
    if (f >= 1.0) {
      bignum_t *s = bnaddl(v2, 1);
      bnfree(v2);
      v2 = s;
      f = f - 1.0;
    }
    v = v2;
  }
  if (e > 0) {
    bignum_t *s = bnashl(v, e);
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

bignum_t *strtobn(const char *str, char **endptr, int radix)
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
  case 0: if (endptr) *endptr = (char *)str; return (errno = EDOM, NULL);
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
  if (endptr)
    *endptr = (char *)str;

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

int bnprint(FILE *f, const bignum_t *n, int radix)
{
  size_t len;
  char *buffer;
  char *ptr;
  int size;
  int status;

  assert(n != NULL);
  CHECKSIGN(n);
  assert(f != NULL);

  len = bnfmtsize(n, radix);
  if (len == 0)
    return 0;
  buffer = bnrealloc(NULL, len); // use regular alloc???
  ptr = bntostr(buffer, len, n, radix);
  size = (int)strlen(ptr);
  status = fputs(ptr, f);
  bnrealloc(buffer, 0);
  if (status != 0)
    return 0;
  else
    return size;
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
  t = bnashl(a, (long)(DBL_MANT_DIG + 2) + (long)wb - (long)wa);
  q = bndmod(&r, t, b);
  bnfree(t); bnfree(a); bnfree(b);

  /* GRS rounding: look at bottom 2 bits of q and sticky bit (r) */
  low = bnmodl(q, 4);
  if (low > 2) { /* 11x: round up */
    t = bnaddl(q, 1); bnfree(q); q = t;
  } else if (low == 2) {
    /* 10x: half-way, check sticky */
    if (!bnzero(r)) { /* 101: round up */
      t = bnaddl(q, 1); bnfree(q); q = t;
    } else { /* 100: tie - round to even (check bit 2) */
      t = bnashl(q, -2);
      if (bnodd(t)) {
        bignum_t *p = bnaddl(q, 1); bnfree(q); q = p;
      }
      bnfree(t);
    }
  } /* else 0xx or 01x: truncate */

  bnfree(r);

  /* drop guard+round bits */
  t = bnashl(q, -2); bnfree(q); q = t;

  /* handle overflow from rounding (now has DBL_MANT_DIG+1 bits) */
  if (bnwidthu(q) > DBL_MANT_DIG) {
    ++e; t = bnashl(q, -1); bnfree(q); q = t;
  }

  v = s * ldexp(bntod(q), e - DBL_MANT_DIG);
  bnfree(q);
  return v;
}

/* ignores sign of the input, returns nonnegative result */
double bnsqrttod(const bignum_t *n0)
{
  bignum_t *n, *m; size_t w, keep;
  long sh; double md, r;

  n = bnabs(n0);
  if (bnzero(n)) { bnfree(n); return 0.0; }
  w = bnwidthu(n);

  /* see if we are getting infinity anyway */
  if (w > (size_t)2 * (size_t)DBL_MAX_EXP) { bnfree(n); return HUGE_VAL; }

  /* Keep enough high bits so that conversion is finite and accurate. */
  keep = (size_t)(DBL_MAX_EXP - 2);
  if (keep < (size_t)DBL_MANT_DIG) keep = (size_t)DBL_MANT_DIG;
  /* see if regular conversion is ok */
  if (w <= keep) { 
    md = bntod(n); bnfree(n); 
    return sqrt(md);
  }

  /* lop off useless bits, but keep shift even */
  sh = (long)(w - keep); 
  if (sh & 1L) ++sh;
  m = bnashl(n, -sh);
  md = bntod(m);

  /* sh/2 is exact, adjust exponent */
  r = ldexp(sqrt(md), (int)(sh / 2));

  bnfree(m);
  bnfree(n);
  return r;
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
    /* same as bnashl */
    return bnashl(a, (long long)cnt);
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

    r = bnashl(a, (long long)cnt);
    if (sticky) { bignum_t *t = bnaddl(r, -1); bnfree(r); r = t; }

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
numt_t numdup(nump_t *yp, numt_t xt, const nump_t *xp)
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

/* move number in memory (ownership goes with it) */
numt_t nummove(nump_t *yp, numt_t xt, const nump_t *xp)
{
  numt_t yt = xt;
  assert(xt && "NONE number");
  *yp = *xp; xt >>= 2; if (!xt) return yt; else ++xp, ++yp;
  *yp = *xp; xt >>= 2; if (!xt) return yt; else ++xp, ++yp;
  *yp = *xp; xt >>= 2; if (!xt) return yt; else ++xp, ++yp;
  *yp = *xp;
  return yt;
}

/* numerical comparisons (needed for inexacts) */
typedef enum { NCMP_LT, NCMP_LE, NCMP_EQ, NCMP_GE, NCMP_GT } ncmp_t;


/* generic integer arithmetics */

/* NB: in normal form, only integers out of fixnum range are represented as bignums */

#define NUMT_IS_INTNUM(nt) ((nt) == NUMT_FIX || (nt) == NUMT_BIG)

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
int intless(numt_t xt, const nump_t *xp, numt_t yt, const nump_t *yp)
{
  assert(NUMT_IS_INTNUM(xt) && "non-integer number");
  assert(NUMT_IS_INTNUM(yt) && "non-integer number");
  if (isfix(xt)) {
    if (isfix(yt)) return getfix(xp) < getfix(yp);
    else return bnsign(getbig(yp)) > 0; /* disjoint */
  } else {
    if (isfix(yt)) return bnsign(getbig(xp)) < 0; /* disjoint */
    else return bncmp(getbig(xp), getbig(yp)) < 0;
  }
}

#define bneql(b, l) (0 == bncmpl(b, l))

/* odd(x) */
int intodd(numt_t xt, const nump_t *xp)
{
  assert(NUMT_IS_INTNUM(xt) && "non-integer number");
  if (isfix(xt)) return (getfix(xp) & 1) != 0;
  else return bnodd(getbig(xp));
}

/* z = -x */
numt_t intneg(nump_t *zp, numt_t xt, const nump_t *xp)
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
numt_t intabs(nump_t *zp, numt_t xt, const nump_t *xp)
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
int intsign(numt_t xt, const nump_t *xp)
{
  assert(NUMT_IS_INTNUM(xt) && "non-integer number");
  if (isfix(xt)) return getfix(xp) ? (getfix(xp) < 0 ? -1 : 1) : 0;
  else return bnsign(getbig(xp));
}

/* z = x + y */
numt_t intadd(nump_t *zp, numt_t xt, const nump_t *xp, numt_t yt, const nump_t *yp)
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
      bz = bnaddl(getbig(yp), getfix(xp));
    }
  } else {
    if (isfix(yt)) bz = bnaddl(getbig(xp), getfix(yp));
    else bz = bnadd(getbig(xp), getbig(yp));
  }
  if (bnwidths(bz) > FIXNUM_WIDTH) zt = setbig(zp, bz);
  else (zt = setfix(zp, bntol(bz))), bnfree(bz);
  return zt;
}

/* z = x - y */
numt_t intsub(nump_t *zp, numt_t xt, const nump_t *xp, numt_t yt, const nump_t *yp)
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
      bignum_t *bt = bnaddl(getbig(yp), -getfix(xp));  /* no bnsubl */
      bz = bnneg(bt); /* no in-place sign change */
      bnfree(bt);
    }
  } else {
    if (isfix(yt)) bz = bnaddl(getbig(xp), -getfix(yp));
    else bz = bnsub(getbig(xp), getbig(yp));
  }
  if (bnwidths(bz) > FIXNUM_WIDTH) zt = setbig(zp, bz);
  else (zt = setfix(zp, bntol(bz))), bnfree(bz);
  return zt;
}

/* z = x * y */
numt_t intmul(nump_t *zp, numt_t xt, const nump_t *xp, numt_t yt, const nump_t *yp)
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
      bz = bnmull(getbig(yp), getfix(xp));
    }
  } else {
    if (isfix(yt)) bz = bnmull(getbig(xp), getfix(yp));
    else bz = bnmul(getbig(xp), getbig(yp));
  }
  if (bnwidths(bz) > FIXNUM_WIDTH) zt = setbig(zp, bz);
  else (zt = setfix(zp, bntol(bz))), bnfree(bz);
  return zt;
}

/* z = truncate(x/y) (truncate-quotient, a.k.a. quotient) */
numt_t intquo(nump_t *zp, numt_t xt, const nump_t *xp, numt_t yt, const nump_t *yp)
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
numt_t intrem(nump_t *zp, numt_t xt, const nump_t *xp, numt_t yt, const nump_t *yp)
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

/* z = floor(x/y)  (floor-quotient, pair for modulo) */
numt_t intfquo(nump_t *zp, numt_t xt, const nump_t *xp, numt_t yt, const nump_t *yp)
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
numt_t intfrem(nump_t *zp, numt_t xt, const nump_t *xp, numt_t yt, const nump_t *yp)
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

/* q = floor(sqrt(x)), r = x-floor(sqrt(x)) x >= 0 */
void intsqrt(numt_t *pqt, nump_t *qp, numt_t *prt, nump_t *rp, numt_t xt, const nump_t *xp)
{
  assert(NUMT_IS_INTNUM(xt) && getfix(xp) >= 0 && "non-natural number");
  if (isfix(xt)) {
    long x = getfix(xp), q;
    for (q = 1; q*q > x || x > q*(q+2); q = (q + x/q)/2);
    *pqt = setfix(qp, q), *prt = setfix(rp, x-q*q);
  } else {
    bignum_t *bx = getbig(xp), *bq = bn1, *bq2, *bl, *bu;
    bignum_t *bn2 = lltobn(2), *br;
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
    if (bnwidths(bq) > FIXNUM_WIDTH) *pqt = setbig(qp, bq);
    else (*pqt = setfix(qp, bntol(bq))), bnfree(bq);
    if (bnwidths(br) > FIXNUM_WIDTH) *prt = setbig(rp, br);
    else (*prt = setfix(rp, bntol(br))), bnfree(br);
  }  
}

/* z = gcd(x, y) */
numt_t intgcd(nump_t *zp, numt_t xt, const nump_t *xp, numt_t yt, const nump_t *yp)
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
numt_t intexptu(nump_t *zp, numt_t xt, const nump_t *xp, unsigned long long y)
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
    return setbig(zp, bnexpt(getbig(xp), y));
  }
}

/* z = ~x */
numt_t intnot(nump_t *zp, numt_t xt, const nump_t *xp)
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
numt_t intand(nump_t *zp, numt_t xt, const nump_t *xp, numt_t yt, const nump_t *yp)
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
numt_t intior(nump_t *zp, numt_t xt, const nump_t *xp, numt_t yt, const nump_t *yp)
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
numt_t intxor(nump_t *zp, numt_t xt, const nump_t *xp, numt_t yt, const nump_t *yp)
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
numt_t intash(nump_t *zp, numt_t xt, const nump_t *xp, numt_t yt, const nump_t *yp)
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
          long z = x << y; if ((z >> y) == x) return setfix(zp, z); 
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
numt_t intlen(nump_t *zp, numt_t xt, const nump_t *xp)
{
  assert(NUMT_IS_INTNUM(xt) && "non-integer number");

  if (isfix(xt)) {
    unsigned long v = (unsigned long)(getfix(xp) >= 0 ? getfix(xp) : ~getfix(xp));
    long len = 0;
    while (v) { ++len; v >>= 1; }
    return setfix(zp, len);
  } else {
    long len = bnintlen(getbig(xp));
    return setfix(zp, len);
  }
}

/* z = bit-count(x); always returns fixnum */
numt_t intbitc(nump_t *zp, numt_t xt, const nump_t *xp)
{
  long cnt = 0;
  assert(NUMT_IS_INTNUM(xt) && "non-integer number");

  if (isfix(xt)) {
    unsigned long v = (unsigned long)(getfix(xp) >= 0 ? getfix(xp) : ~getfix(xp));
    while (v) { ++cnt; v &= v - 1; } /* Kernighan popcount */
  } else {
    cnt = bnbitc(getbig(xp));
  }
  return (cnt <= FIXNUM_MAX) ? setfix(zp, cnt) : setfail(EDOM);
}


/* returns NUMT_NONE and sets errno on failure */
numt_t strtoint(nump_t *zp, const char *str, char **endptr, int radix)
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
  if (endptr) *endptr = ep;
  return zt;
}


/* # of chars needed for x in radix, including '-' for negs and '\0' */
size_t intfmtsize(numt_t xt, const nump_t *xp, int radix)
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
char *inttostr(char *buffer, size_t len, numt_t xt, const nump_t *xp, int radix)
{
  assert(radix >= 2 && radix <= 36);
  assert(NUMT_IS_INTNUM(xt) && "non-integer number");
  if (isfix(xt)) {
    long lx = labs(getfix(xp));
    char *pc = buffer + len;
    for (*--pc = 0; lx > 0; lx /= radix) {
      int d = (int)(lx % radix);
      *--pc = d < 10 ? '0' + d : 'A' + d;
    }
    if (getfix(xp) == 0) *--pc = '0';
    else if (getfix(xp) < 0 && pc > buffer) *--pc = '-';
    assert(pc >= buffer);
    return pc;
  } else { /* print bignum right-to-left */
    return bntostr(buffer, len, getbig(xp), radix);
  }
}

/* print x in radix; use a-z if radix > 10; add - if negative */
void intprint(FILE *fp, numt_t xt, const nump_t *xp, int radix)
{
  assert(radix >= 2 && radix <= 36);
  assert(NUMT_IS_INTNUM(xt) && "non-integer number");
  if (isfix(xt)) {
    long lx = labs(getfix(xp));
    if (lx == 0) {
      fputc('0', fp);
      return;
    } else {
      char buf[FIXNUM_WIDTH+2], *pc = buf + sizeof(buf);
      for (*--pc = 0; lx > 0; lx /= radix) {
        int d = (int)(lx % radix);
        *--pc = d < 10 ? '0' + d : 'a' + d;
      }
      if (getfix(xp) < 0) *--pc = '-';
      fputs(pc, fp);
    }
  } else {
    bnprint(fp, getbig(xp), radix);
  }
}

/* (double)x */
double inttod(numt_t xt, const nump_t *xp)
{
  assert(NUMT_IS_INTNUM(xt) && "non-integer number");
  if (isfix(xt)) return (double)getfix(xp);
  return bntod(getbig(xp));
}

/* |x| < |y| */
int intlessabs(numt_t xt, const nump_t *xp, numt_t yt, const nump_t *yp)
{
  if (isfix(xt)) {
    if (isfix(yt)) return labs(getfix(xp)) < labs(getfix(yp));
    else if (getfix(xp) == FIXNUM_MIN && bneql(getbig(yp), FIXNUM_MAX+1)) return 0;
    else return 1;
  } else {
    if (isbig(yt)) return bncmpabs(getbig(xp), getbig(yp)) < 0;
    else return 0;
  }
}

/* generic rational arithmetics */

/* in normal form, only non-0 ratios with non-1 denominator are represented as 2-slot numbers;
 * in those, denominator is always positive and has no common non-1 factors with the numerator */

#define NUMT_IS_INTORNONE(nt) ((nt) == NUMT_FIX || (nt) == NUMT_BIG || (nt) == NUMT_NONE)
#define NUMT_IS_RATNUM(nt) (((nt) & NUMT_DS_MASK) == (nt) && NUMT_IS_INTNUM(NUMT_RAT_N(nt)) && NUMT_IS_INTORNONE(NUMT_RAT_D(nt)))

/* these macros assume RATNUM arguments */
#define isint(xt)     (NUMT_IS_INTNUM(xt))

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
int ratless(numt_t xt, const nump_t *xp, numt_t yt, const nump_t *yp)
{
  int xs, ys;
  assert(NUMT_IS_RATNUM(xt) && "non-rational number");
  assert(NUMT_IS_RATNUM(yt) && "non-rational number");
  if (isint(xt) && isint(yt)) {
    return intless(xt, xp, yt, yp);
  } else if ((xs = intsign(NUMT_RAT_N(xt), xp)) != (ys = intsign(NUMT_RAT_N(yt), yp))) {
    return xs < ys;
  } else if (!isint(xt) && !isint(yt) && inteq(NUMT_RAT_D(xt), xp+1, NUMT_RAT_D(yt), yp+1)) {
    return intless(NUMT_RAT_N(xt), xp, NUMT_RAT_N(yt), yp);
  } else {
    /* have to compare n(x)*d(y) < n(y)*d(x) */
    int res;
    numt_t nxt; nump_t nxp[1]; numt_t dxt; nump_t dxp[1]; /* owned by caller */
    numt_t nyt; nump_t nyp[1]; numt_t dyt; nump_t dyp[1]; /* owned by caller */
    numt_t mxt; nump_t mxp[1]; numt_t myt; nump_t myp[1]; /* new */
    nxt = NUMT_RAT_N(xt), *nxp = *xp;
    if (!(dxt = NUMT_RAT_D(xt))) dxt = setfix(dxp, 1); else dxp[0] = xp[1];
    nyt = NUMT_RAT_N(yt), *nyp = *yp;
    if (!(dyt = NUMT_RAT_D(yt))) dyt = setfix(dyp, 1); else dyp[0] = yp[1];
    mxt = intmul(mxp, nxt, nxp, dyt, dyp), myt = intmul(myp, nyt, nyp, dxt, dxp);
    res = intless(mxt, mxp, myt, myp);
    numfini(mxt, mxp), numfini(myt, myp);
    return res;
  }
}

/* z = -x */
numt_t ratneg(nump_t *zp, numt_t xt, const nump_t *xp)
{
  assert(NUMT_IS_RATNUM(xt) && "non-rational number");
  if (isint(xt)) {
    return intneg(zp, xt, xp);
  } else {
    return NUMT_MKRAT(intneg(zp, NUMT_RAT_N(xt), xp), numdup(zp+1, NUMT_RAT_D(xt), xp+1));
  }
}

/* z = 1/x */
numt_t ratrcp(nump_t *zp, numt_t xt, const nump_t *xp)
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
numt_t ratabs(nump_t *zp, numt_t xt, const nump_t *xp)
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
int ratsign(numt_t xt, const nump_t *xp)
{
  assert(NUMT_IS_RATNUM(xt) && "non-rational number");
  return intsign(NUMT_RAT_N(xt), xp);
}

/* z = x + y */
numt_t ratadd(nump_t *zp, numt_t xt, const nump_t *xp, numt_t yt, const nump_t *yp)
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
numt_t ratsub(nump_t *zp, numt_t xt, const nump_t *xp, numt_t yt, const nump_t *yp)
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
numt_t ratmul(nump_t *zp, numt_t xt, const nump_t *xp, numt_t yt, const nump_t *yp)
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
numt_t ratdiv(nump_t *zp, numt_t xt, const nump_t *xp, numt_t yt, const nump_t *yp)
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
numt_t ratfloor(nump_t *zp, numt_t xt, const nump_t *xp)
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
numt_t ratceil(nump_t *zp, numt_t xt, const nump_t *xp)
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
numt_t rattrunc(nump_t *zp, numt_t xt, const nump_t *xp)
{
  assert(NUMT_IS_RATNUM(xt) && "non-rational number");
  if (isint(xt)) {
    return numdup(zp, xt, xp); /* must be fresh! */
  } else {
    return intquo(zp, NUMT_RAT_N(xt), xp, NUMT_RAT_D(xt), xp+1);
  }
}

/* z = round(x) */
numt_t ratround(nump_t *zp, numt_t xt, const nump_t *xp)
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
numt_t ratscale(nump_t *zp, numt_t xt, const nump_t *xp, long b, long e)
{
  assert(b > 0 && b <= FIXNUM_MAX && e < FIXNUM_MAX);
  if (!e) {
    return numdup(zp, xt, xp);
  } else {
    nump_t bp[1]; numt_t bt = setfix(bp, b);
    nump_t sp[1]; numt_t st = intexptu(sp, bt, bp, (unsigned long long)labs(e));
    numt_t zt;
    if (e < 0) zt = ratdiv(zp, xt, xp, st, sp);
    else zt = ratmul(zp, xt, xp, st, sp);
    numfini(st, sp);
    numfini(bt, bp);
    return zt;
  }
}

/* returns NUMT_NONE and sets errno on failure */
numt_t strtorat(nump_t *zp, const char *str, char **endptr, int radix)
{
  numt_t zt = NUMT_NONE; char *ep = NULL;
  int decimal = (radix == 2 || radix == 8 || radix == 10 || radix == 16);
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
  if (endptr) *endptr = ep;
  return zt;
}

/* # of chars needed for x in radix, including '/', '-' for negs and '\0' */
size_t ratfmtsize(numt_t xt, const nump_t *xp, int radix)
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
char *rattostr(char *buffer, size_t len, numt_t xt, const nump_t *xp, int radix)
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

/* print x in radix; use a-z if radix > 10; add - if negative */
void ratprint(FILE *fp, numt_t xt, const nump_t *xp, int radix)
{
  assert(radix >= 2 && radix <= 36);
  assert(NUMT_IS_RATNUM(xt) && "non-rational number");
  if (isint(xt)) {
    intprint(fp, xt, xp, radix);
  } else {
    intprint(fp, NUMT_RAT_N(xt), xp, radix);
    fputc('/', fp);
    intprint(fp, NUMT_RAT_D(xt), xp+1, radix);
  }  
}

/* (double)x */
double rattod(numt_t xt, const nump_t *xp)
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

/* strtod via strtorat, rattod */
double strtoratd(const char *str, char **endptr, int radix)
{
  double res;
  nump_t tp[2]; numt_t tt = strtorat(tp, str, endptr, radix);
  if (!tt || errno) res = HUGE_VAL-HUGE_VAL; /* NaN */
  else res = rattod(tt, tp);
  numfini(tt, tp);
  return res;
}

/* (rat)d -- returns NUMT_NONE and sets errno on failure */
numt_t dtorat(nump_t *zp, double x)
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
    else nt = setbig(np, bnashl(bn1, ame));
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
int recteq(numt_t xt, const nump_t *xp, numt_t yt, const nump_t *yp)
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
numt_t rectneg(nump_t *zp, numt_t xt, const nump_t *xp)
{
  assert(NUMT_IS_RECTNUM(xt) && "non-exact-complex number");
  if (israt(xt)) {
    return ratneg(zp, xt, xp);
  } else {
    return NUMT_MKCOM(ratneg(zp, NUMT_COM_R(xt), xp), ratneg(zp+2, NUMT_COM_I(xt), xp+2));
  }
}

/* z = conjugate(x) */
numt_t rectconj(nump_t *zp, numt_t xt, const nump_t *xp)
{
  assert(NUMT_IS_RECTNUM(xt) && "non-exact-complex number");
  if (israt(xt)) {
    return numdup(zp, xt, xp); /* must be fresh! */
  } else {
    return NUMT_MKCOM(numdup(zp, NUMT_COM_R(xt), xp), ratneg(zp+2, NUMT_COM_I(xt), xp+2));
  }
}

/* z = x + y */
numt_t rectadd(nump_t *zp, numt_t xt, const nump_t *xp, numt_t yt, const nump_t *yp)
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
numt_t rectsub(nump_t *zp, numt_t xt, const nump_t *xp, numt_t yt, const nump_t *yp)
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
numt_t rectmul(nump_t *zp, numt_t xt, const nump_t *xp, numt_t yt, const nump_t *yp)
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
numt_t rectdiv(nump_t *zp, numt_t xt, const nump_t *xp, numt_t yt, const nump_t *yp)
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
numt_t strtorect(nump_t *zp, const char *str, char **endptr, int radix)
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
    if (isflo(zt) && errno == ERANGE) errno = 0; /* ok here */
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
        double a = isflo(at) ? getflo(ap) : rattod(at, ap);
        numfini(zt, zp); numfini(at, ap);
        zt = NUMT_MKCOM(setflo(zp, m*cos(m)), setflo(zp+2, m*sin(a)));
      }
    } else { 
      /* real */
    }
  }
  if (endptr) *endptr = ep;
  if (!isrect(zt)) errno = ERANGE;
  return zt;
}

/* # of chars needed for x in radix, including '+', '-', 'i', and '\0' */
size_t rectfmtsize(numt_t xt, const nump_t *xp, int radix)
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
char *recttostr(char *buffer, size_t len, numt_t xt, const nump_t *xp, int radix)
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

/* print x in radix; use a-z if radix > 10; add - if negative */
void rectprint(FILE *fp, numt_t xt, const nump_t *xp, int radix)
{
  assert(radix >= 2 && radix <= 36);
  assert(NUMT_IS_RECTNUM(xt) && "non-exact-complex number");
  if (israt(xt)) {
    ratprint(fp, xt, xp, radix);
  } else if (isfix(NUMT_COM_R(xt)) && getfix(xp) == 0) {
    if (ratsign(NUMT_COM_I(xt), xp+2) < 0) ; /* - added automatically */
    else fputc('+', fp); /* + won't be added, so we do it here */
    if (isfix(NUMT_COM_I(xt)) && getfix(xp+2) == 1) ; /* skip 1 */
    else if (isfix(NUMT_COM_I(xt)) && getfix(xp+2) == -1) fputc('-', fp); /* skip 1 */
    else ratprint(fp, NUMT_COM_I(xt), xp+2, radix);
    fputc('i', fp);
  } else {
    ratprint(fp, NUMT_COM_R(xt), xp, radix);
    if (ratsign(NUMT_COM_I(xt), xp+2) < 0) ; /* - added automatically */
    else fputc('+', fp); /* + won't be added, so we do it here */
    if (isfix(NUMT_COM_I(xt)) && getfix(xp+2) == 1) ; /* skip 1 */
    else if (isfix(NUMT_COM_I(xt)) && getfix(xp+2) == -1) fputc('-', fp); /* skip 1 */
    else ratprint(fp, NUMT_COM_I(xt), xp+2, radix);
    fputc('i', fp);
  }  
}

/* (double,double)x */
void recttodd(numt_t xt, const nump_t *xp, double *prd, double *pid)
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
int compeq(numt_t xt, const nump_t *xp, numt_t yt, const nump_t *yp)
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
numt_t compneg(nump_t *zp, numt_t xt, const nump_t *xp)
{
  assert(NUMT_IS_COMPNUM(xt) && "non-inexact-complex number");
  if (isflo(xt)) {
    return setflo(zp, -getflo(xp));
  } else {
    return NUMT_MKCOM(setflo(zp, -getflo(xp)), setflo(zp+2, -getflo(xp+2)));
  }
}

/* z = conjugate(x) */
numt_t compconj(nump_t *zp, numt_t xt, const nump_t *xp)
{
  assert(NUMT_IS_COMPNUM(xt) && "non-exact-complex number");
  if (isflo(xt)) {
    return NUMT_MKCOM(setflo(zp, getflo(xp)), setflo(zp+2, -0.0));
  } else {
    return NUMT_MKCOM(setflo(zp, getflo(xp)), setflo(zp+2, -getflo(xp+2)));
  }
}

/* z = x + y */
numt_t compadd(nump_t *zp, numt_t xt, const nump_t *xp, numt_t yt, const nump_t *yp)
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
numt_t compsub(nump_t *zp, numt_t xt, const nump_t *xp, numt_t yt, const nump_t *yp)
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
numt_t compmul(nump_t *zp, numt_t xt, const nump_t *xp, numt_t yt, const nump_t *yp)
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
numt_t compdiv(nump_t *zp, numt_t xt, const nump_t *xp, numt_t yt, const nump_t *yp)
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


/* returns NUMT_NONE and sets errno on failure */
numt_t strtoflo(nump_t *zp, const char *str, char **endptr, int radix)
{
  double d; char *e, *t; numt_t zt = NUMT_NONE;
  errno = 0; 
  /* check for infnans first */
  if (endptr) *endptr = (char*)str + 6;
  if (0 == strncmp_ci(str, "+inf.0", 6)) return setflo(zp, HUGE_VAL);
  if (0 == strncmp_ci(str, "-inf.0", 6)) return setflo(zp, -HUGE_VAL);
  if (0 == strncmp_ci(str, "+nan.0", 6)) return setflo(zp, HUGE_VAL-HUGE_VAL);
  if (0 == strncmp_ci(str, "-nan.0", 6)) return setflo(zp, HUGE_VAL-HUGE_VAL);
  /* check for a ratio (in radix) */
  e = (char*)str; if (*e == '-' || *e == '+') ++e;
  if ((t = skip_uinteger(e, radix))[0] == '/') {
    zt = strtorat(zp, str, endptr, radix);
    if (isflo(zt) && errno == ERANGE) return errno = 0, zt;
    else if (!zt || errno) return setfail(EDOM);
    d = rattod(zt, zp);
    numfini(zt, zp);
    return setflo(zp, d);
  }
  /* not a ratio: either intnum or inexact (b,o,x)decimal */
  if (radix == 2 || radix == 8 || radix == 10 || radix == 16) {
    d = strtofn(str, radix, endptr); /* will handle ints too */
    if (errno == ERANGE) errno = 0; /* overflows and underflows are ok */
    if (errno) return setfail(EDOM); 
  } else { /* intnum only, arbitrary radix */
    double zero = (*str == '-') ? -0.0 : 0.0; /* NB: -0! */ 
    zt = strtoint(zp, str, endptr, radix);
    if (!zt) return setfail(EDOM);
    if (isfix(zt) && getfix(zp) == 0) d = zero;
    else d = inttod(zt, zp); /* may overflow, but it's ok */
  }
  numfini(zt, zp);
  return setflo(zp, d);
}

/* returns NUMT_NONE and sets errno on failure */
numt_t strtocomp(nump_t *zp, const char *str, char **endptr, int radix)
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
    } else if (ep && *ep == '@') {
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
  if (endptr) *endptr = ep;
  return zt;
}

/* # of chars needed for x in radix; return 0 if radix != 10  */
size_t compfmtsize(numt_t xt, const nump_t *xp, int radix)
{
  assert(radix >= 2 && radix <= 36);
  assert(NUMT_IS_COMPNUM(xt) && "non-exact-complex number");
  return isflo(xt) ? fnfmtsize(radix) : fnfmtsize(radix)*2 + 2;
}

/* returns buf on success, NULL on wrong radix */
char *flotostr(char *buf, size_t len, double x, int radix)
{
  assert(len >= 7);
  if (x != x) return strcpy(buf, "+nan.0");
  if (x > DBL_MAX) return strcpy(buf, "+inf.0");
  if (x < -DBL_MAX) return strcpy(buf, "-inf.0");
  return fntostr(buf, len, x, radix);  
}

/* format x into buffer; len should be as calculated by compfmtsize;
 * returns ptr to first char of zero-terminated result in buffer
 * or NULL if an inexact number is printed in non-10 radix. */
char *comptostr(char *buffer, size_t len, numt_t xt, const nump_t *xp, int radix)
{
  assert(NUMT_IS_COMPNUM(xt) && "non-exact-complex number");
  if (isflo(xt)) { /* may return NULL on wrong radix */
    return flotostr(buffer, len, getflo(xp), radix);
  } else { /* flotostr prints left-to-right */
    char *buf = flotostr(buffer, len/2, getflo(xp), radix);
    char *sep = buf ? buf + strlen(buf) : NULL;
    assert(!buf || buf == buffer);
    if (!buf) return NULL;
    if (fnsignless(getflo(xp+2))) *sep++ = '+';
    buf = flotostr(sep, len/2, getflo(xp+2), radix);
    assert(!buf || buf == sep);
    if (!buf) return NULL;
    sep = buf + strlen(buf);
    *sep++ = 'i'; *sep = 0;
    assert(sep-buffer < (int)len);
    return buffer;
  }
}

void floprint(FILE *fp, double x, int radix)
{
  if (x != x) fputs("+nan.0", fp);
  else if (x > DBL_MAX) fputs("+inf.0", fp);
  else if (x < -DBL_MAX) fputs("-inf.0", fp);
  else fnprint(fp, x, radix);  
}

/* print x in radix; use a-z if radix > 10; return -1 if radix != 10 */
void compprint(FILE *fp, numt_t xt, const nump_t *xp, int radix)
{
  assert(radix == 10 || radix == 16);
  assert(NUMT_IS_COMPNUM(xt) && "non-exact-complex number");
  if (isflo(xt)) {
    floprint(fp, getflo(xp), radix);
  } else {
    floprint(fp, getflo(xp), radix);
    if (fnsignless(getflo(xp+2))) fputc('+', fp);
    floprint(fp, getflo(xp+2), radix);
    fputc('i', fp);
  }
}

/* (double,double)x */
void comptodd(numt_t xt, const nump_t *xp, double *prd, double *pid)
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
  long lx, ly; double dx, dy; nump_t xp1[2], yp1[2];
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
      int res;
      if (isflo(xt)) xt = dtorat(xp1, getflo(xp)), xp = xp1;
      if (isflo(yt)) yt = dtorat(yp1, getflo(yp)), yp = yp1;
      res = rateq(xt, xp, yt, yp);
      if (xp == xp1) numfini(xt, xp1);
      if (yp == yp1) numfini(yt, yp1);
      return res;
    }
  } else {
    return rateq(xt, xp, yt, yp);
  }
}

/* x < y */
int realless(numt_t xt, const nump_t *xp, numt_t yt, const nump_t *yp)
{
  long lx, ly; double dx, dy; nump_t xp1[2], yp1[2];
  assert(NUMT_IS_REALNUM(xt) && "non-real number");
  assert(NUMT_IS_REALNUM(yt) && "non-real number");
  if (isfix(xt) && isfix(yt)) { /* fast track */
    lx = getfix(xp), ly = getfix(yp); 
    return lx < ly;
  } else if (isflo(xt) && isflo(yt)) { /* fast track */
    dx = getflo(xp), dy = getflo(yp); 
    return dx < dy;
  } else if (isflo(xt) || isflo(yt)) {
    if (isfix(xt) || isfix(yt)) { /* fast track */
      dx = isfix(xt) ? getfix(xp) : getflo(xp); 
      dy = isfix(yt) ? getfix(yp) : getflo(yp);
      return dx < dy;
    } else { /* slow track */
      int res;
      if (isflo(xt)) xt = dtorat(xp1, getflo(xp)), xp = xp1;
      if (isflo(yt)) yt = dtorat(yp1, getflo(yp)), yp = yp1;
      res = ratless(xt, xp, yt, yp);
      if (xp == xp1) numfini(xt, xp1);
      if (yp == yp1) numfini(yt, yp1);
      return res;
    }
  } else {
    return ratless(xt, xp, yt, yp);
  }
}

/* cmp(x, y) == c */
int realcmp(numt_t xt, const nump_t *xp, numt_t yt, const nump_t *yp, ncmp_t c)
{
  long lx, ly; double dx, dy; int res;
  nump_t xp1[2], yp1[2]; numt_t xt1 = NUMT_NONE, yt1 = NUMT_NONE;
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
      if (isflo(xt)) xt = xt1 = dtorat(xp1, getflo(xp)), xp = xp1;
      if (isflo(yt)) yt = yt1 = dtorat(yp1, getflo(yp)), yp = yp1;
      goto cmpr;
    }
  } else {
    goto cmpr;
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
cmpr:
  switch (c) { 
    case NCMP_LT: res = ratless(xt, xp, yt, yp); break;
    case NCMP_LE: res = !ratless(yt, yp, xt, xp); break;
    case NCMP_EQ: res = rateq(xt, xp, yt, yp); break;
    case NCMP_GE: res = !ratless(xt, xp, yt, yp); break;
    case NCMP_GT: res = ratless(yt, yp, xt, xp); break;
    default: assert(0); res = 0; break;
  }
  numfini(xt1, xp1);
  numfini(yt1, yp1);
  return res;
}

/* cmp(x, 0) == c */
int realcmp0(numt_t xt, const nump_t *xp, ncmp_t c)
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
numt_t realneg(nump_t *zp, numt_t xt, const nump_t *xp)
{
  assert(NUMT_IS_REALNUM(xt) && "non-real number");
  if (isflo(xt)) return setflo(zp, -getflo(xp));
  return ratneg(zp, xt, xp);
}

/* z = |x| */
numt_t realabs(nump_t *zp, numt_t xt, const nump_t *xp)
{
  assert(NUMT_IS_REALNUM(xt) && "non-real number");
  if (isflo(xt)) return setflo(zp, fabs(getflo(xp)));
  return ratabs(zp, xt, xp);
}

/* sign(x) (-1 0 +1) */
int realsign(numt_t xt, const nump_t *xp)
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
numt_t realadd(nump_t *zp, numt_t xt, const nump_t *xp, numt_t yt, const nump_t *yp)
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
numt_t realsub(nump_t *zp, numt_t xt, const nump_t *xp, numt_t yt, const nump_t *yp)
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
numt_t realmul(nump_t *zp, numt_t xt, const nump_t *xp, numt_t yt, const nump_t *yp)
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
numt_t realdiv(nump_t *zp, numt_t xt, const nump_t *xp, numt_t yt, const nump_t *yp)
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
numt_t realfloor(nump_t *zp, numt_t xt, const nump_t *xp)
{
  assert(NUMT_IS_REALNUM(xt) && "non-real number");
  if (isflo(xt)) return setflo(zp, floor(getflo(xp)));
  else return ratfloor(zp, xt, xp);
}

/* z = ceiling(x) */
numt_t realceil(nump_t *zp, numt_t xt, const nump_t *xp)
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
numt_t realround(nump_t *zp, numt_t xt, const nump_t *xp)
{
  assert(NUMT_IS_REALNUM(xt) && "non-real number");
  if (isflo(xt)) {
    double x = getflo(xp), f = floor(x), c = ceil(x), d = x-f, u = c-x, r; 
    if (d == u) r = fmod(f, 2.0) == 0.0 ? f : c;
    else r = (d < u) ? f : c;
    return setflo(zp, r);
  } else return ratround(zp, xt, xp);
}

/* returns NUMT_NONE and sets errno on failure */
numt_t strtoreal(nump_t *zp, const char *str, char **endptr, int radix)
{
  numt_t zt = NUMT_NONE; char *cp;
  int forceie = 0; /* -1=#i 1=#e */
  int flags = 0; /* CNF_XXX|... */
  assert(str); assert(radix >= 2 && radix <= 36);
  /* returns start of complex part and sets radix/ie; returns NULL on errors */
  if ((cp = check_number(str, &radix, &forceie, &flags)) == NULL
   || (flags & (CNF_IMPART|CNF_POLAR)) != 0) { 
    /* invalid real syntax */
    if (endptr) *endptr = (char*)str;
    return setfail(EDOM);
  }
  /* positon at the start of complex */
  str = (const char *)cp;
  if (forceie < 0) { /* as inexact */
    zt = strtoflo(zp, str, endptr, radix);
  } else if (forceie > 0) { /* as exact */
    zt = strtorat(zp, str, endptr, radix);
    /* catch inexact, e.g. 1/0 -> +inf.0 */
    if (isflo(zt)) zt = setfail(ERANGE); 
  } else if (flags & CNF_DOTEXP) { /* has elements of inexact notation */
    zt = strtoflo(zp, str, endptr, radix);
  } else { /* no elements of inexact notation */
    zt = strtorat(zp, str, endptr, radix);
    /* catch inexact, e.g. 1/0 -> +inf.0 */
    if (isflo(zt)) zt = setfail(ERANGE); 
  }
  return zt;
}

/* # of chars needed for x in radix; assumes decimal if inexact by default */
size_t realfmtsize(numt_t xt, const nump_t *xp, int radix)
{
  assert(radix >= 2 && radix <= 36);
  assert(NUMT_IS_REALNUM(xt) && "non-real number");
  if (isflo(xt)) {
    if (radix == 10) return FN_DEC_BUFSIZE;
    return FN_BIN_BUFSIZE; /* 2, 8, 16 */
  } else {
    return ratfmtsize(xt, xp, radix);
  }
}

/* format x into buffer; len should be as calculated by realfmtsize;
 * returns ptr to first char of zero-terminated result in buffer
 * or NULL if an inexact number is printed in wrong radix. */
char *realtostr(char *buffer, size_t len, numt_t xt, const nump_t *xp, int radix)
{
  assert(radix >= 2 && radix <= 36);
  assert(NUMT_IS_REALNUM(xt) && "non-real number");
  if (isflo(xt)) {
    return flotostr(buffer, len, getflo(xp), radix);
  } else {
    return rattostr(buffer, len, xt, xp, radix);
  }
}

/* print x in radix; use a-z if radix > 10; return -1 if x is inexact and radix != 10 */
int realprint(FILE *fp, numt_t xt, const nump_t *xp, int radix)
{
  assert(radix >= 2 && radix <= 36);
  assert(NUMT_IS_REALNUM(xt) && "non-real number");
  if (isflo(xt)) {
    floprint(fp, getflo(xp), radix);
  } else {
    ratprint(fp, xt, xp, radix);
  }
  return 0;  
}


/* generic mixed-exactness numbers */

/* take from 1 to 4 slots */

#define NUMT_IS_VALID(xt) (NUMT_IS_COMPNUM(xt) || NUMT_IS_RECTNUM(xt))

/* x == y */
int gnumeqn(numt_t xt, const nump_t *xp, numt_t yt, const nump_t *yp)
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
int gnumeqv(numt_t xt, const nump_t *xp, numt_t yt, const nump_t *yp)
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

/* x < y  [real numbers only] */
int gnumless(numt_t xt, const nump_t *xp, numt_t yt, const nump_t *yp)
{
  assert(NUMT_IS_VALID(xt) && "unsupported number type");
  assert(NUMT_IS_VALID(yt) && "unsupported number type");
  if (!isreal(xt) || !isreal(yt)) return -1;
  return realless(xt, xp, yt, yp);
}

/* cmp(x, y) == c  [real numbers only] */
int gnumcmp(numt_t xt, const nump_t *xp, numt_t yt, const nump_t *yp, ncmp_t c)
{
  assert(NUMT_IS_VALID(xt) && "unsupported number type");
  assert(NUMT_IS_VALID(yt) && "unsupported number type");
  if (!isreal(xt) || !isreal(yt)) return -1;
  return realcmp(xt, xp, yt, yp, c);
}

/* cmp(x, 0) == c  [internal, real numbers only] */
int gnumcmp0(numt_t xt, const nump_t *xp, ncmp_t c)
{
  assert(NUMT_IS_VALID(xt) && "unsupported number type");
  if (!isreal(xt)) return -1;
  return realcmp0(xt, xp, c);
}

/* z = max(x, y)  [real numbers only; result is inexact if either arg is] */
numt_t gnummax(nump_t *zp, numt_t xt, const nump_t *xp, numt_t yt, const nump_t *yp)
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
numt_t gnummin(nump_t *zp, numt_t xt, const nump_t *xp, numt_t yt, const nump_t *yp)
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
numt_t gnumabs(nump_t *zp, numt_t xt, const nump_t *xp)
{
  assert(NUMT_IS_VALID(xt) && "unsupported number type");
  if (!isreal(xt)) return setfail(EDOM);
  return realabs(zp, xt, xp);
}

/* z = -x */
numt_t gnumneg(nump_t *zp, numt_t xt, const nump_t *xp)
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

/* z = conjugate(x) */
numt_t gnumconj(nump_t *zp, numt_t xt, const nump_t *xp)
{
  assert(NUMT_IS_VALID(xt) && "unsupported number type");
  if (isreal(xt)) {
    return numdup(zp, xt, xp);
  } else if (isrect(xt)) {
    return rectconj(zp, xt, xp);
  } else {
    return compconj(zp, xt, xp);
  }
}

/* z = x + y */
numt_t gnumadd(nump_t *zp, numt_t xt, const nump_t *xp, numt_t yt, const nump_t *yp)
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
numt_t gnumsub(nump_t *zp, numt_t xt, const nump_t *xp, numt_t yt, const nump_t *yp)
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
numt_t gnummul(nump_t *zp, numt_t xt, const nump_t *xp, numt_t yt, const nump_t *yp)
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
numt_t gnumdiv(nump_t *zp, numt_t xt, const nump_t *xp, numt_t yt, const nump_t *yp)
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
numt_t gnumtoe(nump_t *zp, numt_t xt, const nump_t *xp)
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
numt_t gnumtoi(nump_t *zp, numt_t xt, const nump_t *xp)
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
numt_t gnumfloor(nump_t *zp, numt_t xt, const nump_t *xp)
{
  assert(NUMT_IS_VALID(xt) && "unsupported number type");
  if (!isreal(xt)) return setfail(EDOM);
  return realfloor(zp, xt, xp);
}

/* z = ceiling(x)  [real numbers only] */
numt_t gnumceil(nump_t *zp, numt_t xt, const nump_t *xp)
{
  assert(NUMT_IS_VALID(xt) && "unsupported number type");
  if (!isreal(xt)) return setfail(EDOM);
  return realceil(zp, xt, xp);
}

/* z = truncate(x)  [real numbers only] */
numt_t gnumtrunc(nump_t *zp, numt_t xt, const nump_t *xp)
{
  assert(NUMT_IS_VALID(xt) && "unsupported number type");
  if (!isreal(xt)) return setfail(EDOM);
  return realtrunc(zp, xt, xp);
}

/* z = round(x)  [real numbers only] */
numt_t gnumround(nump_t *zp, numt_t xt, const nump_t *xp)
{
  assert(NUMT_IS_VALID(xt) && "unsupported number type");
  if (!isreal(xt)) return setfail(EDOM);
  return realround(zp, xt, xp);
}

/* z = numerator(x)  [rational/real numbers only] */
numt_t gnumnumer(nump_t *zp, numt_t xt, const nump_t *xp)
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
numt_t gnumdenom(nump_t *zp, numt_t xt, const nump_t *xp)
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
numt_t gnumgcd(nump_t *zp, numt_t xt, const nump_t *xp, numt_t yt, const nump_t *yp)
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

/* z = lcm(x, y)  [integer numbers only; result is always non-negative] */
/* NB: lcm(0,y)=lcm(x,0)=0; result is inexact if either arg is inexact */
numt_t gnumlcm(nump_t *zp, numt_t xt, const nump_t *xp, numt_t yt, const nump_t *yp)
{
  assert(NUMT_IS_VALID(xt) && "unsupported number type");
  assert(NUMT_IS_VALID(yt) && "unsupported number type");
  if (!isreal(xt) || !isreal(yt)) return setfail(EDOM);
  if (isflo(xt) || isflo(yt)) {
    double xd = isflo(xt) ? getflo(xp) : rattod(xt, xp);
    double yd = isflo(yt) ? getflo(yp) : rattod(yt, yp);
    double ax = fabs(xd), ay = fabs(yd), g, tmp;
    if (ax != floor(ax) || ay != floor(ay)) return setfail(EDOM);
    if (ax == 0.0 || ay == 0.0) return setflo(zp, 0.0);
    g = ax; tmp = ay; 
    while (tmp != 0.0) { double t = fmod(g, tmp); g = tmp; tmp = t; }
    return setflo(zp, (ax / g) * ay);
  } else {
    /* lcm(x,y) = |x*y| / gcd(x,y) */
    nump_t axp[1]; numt_t axt = intabs(axp, xt, xp);
    nump_t ayp[1]; numt_t ayt = intabs(ayp, yt, yp);
    nump_t gp[1], qp[1]; numt_t gt, qt, zt;
    if ((isfix(axt) && getfix(axp) == 0) || (isfix(ayt) && getfix(ayp) == 0)) {
      numfini(axt, axp); numfini(ayt, ayp);
      return setfix(zp, 0);
    }
    gt = intgcd(gp, axt, axp, ayt, ayp);
    qt = intquo(qp, axt, axp, gt, gp);
    zt = intmul(zp, qt, qp, ayt, ayp);
    numfini(axt, axp); numfini(ayt, ayp);
    numfini(gt, gp); numfini(qt, qp);
    return zt;
  }
}

/* z = truncate-quotient(x, y)  [integer real numbers only] */
numt_t gnumtquo(nump_t *zp, numt_t xt, const nump_t *xp, numt_t yt, const nump_t *yp)
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
numt_t gnumtrem(nump_t *zp, numt_t xt, const nump_t *xp, numt_t yt, const nump_t *yp)
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
numt_t gnumfquo(nump_t *zp, numt_t xt, const nump_t *xp, numt_t yt, const nump_t *yp)
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
numt_t gnumfrem(nump_t *zp, numt_t xt, const nump_t *xp, numt_t yt, const nump_t *yp)
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
void gnumsqrti(numt_t *pqt, nump_t *qp, numt_t *prt, nump_t *rp,
               numt_t xt, const nump_t *xp)
{
  assert(NUMT_IS_VALID(xt) && "unsupported number type");
  assert(isreal(xt) && "gnumsqrti: complex number");
  assert(gnumcmp0(xt, xp, NCMP_GE) && "gnumsqrti: negative number");
  if (isflo(xt)) {
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

/* q = integer-sqrt(x) [non-negative integer real numbers only] */
numt_t gnumsqrtiq(nump_t *qp, numt_t xt, const nump_t *xp)
{
  nump_t rp[4]; numt_t rt, qt;
  gnumsqrti(&qt, qp, &rt, rp, xt, xp);
  numfini(rt, rp);
  return qt;
}

/* r = x - integer-sqrt(x)^2 [non-negative integer real numbers only] */
numt_t gnumsqrtir(nump_t *rp, numt_t xt, const nump_t *xp)
{
  nump_t qp[4]; numt_t qt, rt;
  gnumsqrti(&qt, qp, &rt, rp, xt, xp);
  numfini(qt, qp);
  return rt;
}

/* z = sqrt(x)  [generic; result may be inexact complex] */
numt_t gnumsqrt(nump_t *zp, numt_t xt, const nump_t *xp)
{
  assert(NUMT_IS_VALID(xt) && "unsupported number type");
  if (isrect(xt)) {
    /* exact: try integer sqrt first, then fall back to inexact */
    if (isint(xt)) {
      /* exact integer: attempt exact square root */
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
    }
    if (!isint(xt) && israt(xt)) {
      nump_t np[4]; numt_t nt = gnumsqrt(np, NUMT_RAT_N(xt), xp);
      nump_t dp[4]; numt_t dt = gnumsqrt(dp, NUMT_RAT_D(xt), xp+1);
      numt_t zt = (nt == NUMT_NONE || dt == NUMT_NONE) ? NUMT_NONE : gnumdiv(zp, nt, np, dt, dp); 
      numfini(nt, np); numfini(dt, dp);
      return zt;
    }
    if (isreal(xt)) {
      double x = rattod(xt, xp);
      if (x >= 0.0) return setflo(zp, sqrt(x));
      return NUMT_MKCOM(setflo(zp, 0.0), setflo(zp+2, sqrt(-x)));
    } else {
      /* exact complex: convert to inexact and do complex sqrt */
      double rx, ix, re, im;
      recttodd(xt, xp, &rx, &ix);
      cmath_sqrt(rx, ix, &re, &im);
      return NUMT_MKCOM(setflo(zp, re), setflo(zp+2, im));
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

int gnumodd(numt_t xt, const nump_t *xp); /* see below */

/* z = expt(x, y)  [generic] */
/* exact base with exact integer exponent stays exact; may go inexact otherwise */
numt_t gnumexpt(nump_t *zp, numt_t xt, const nump_t *xp, numt_t yt, const nump_t *yp)
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
      if (x > 0) return setbig(zp, bnashl(bn1, ax));
      if (ax < FIXNUM_WIDTH-1) return NUMT_MKRAT(setfix(zp, 1), setfix(zp+1, 1L << ax));
      else return NUMT_MKRAT(setfix(zp, 1), setbig(zp+1, bnashl(bn1, ax)));
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
        bignum_t *bz = bnexpt(bx, (unsigned long long)getfix(yp));
        numt_t zt;
        if (isfix(xt)) bnfree(bx); 
        if (bnwidths(bz) > FIXNUM_WIDTH) zt = setbig(zp, bz);
        else (zt = setfix(zp, bntol(bz))), bnfree(bz);
        return zt;
      } else { /* do bignum expt, normalize, get reciprocal */
        bignum_t *bx = isfix(xt) ? lltobn(getfix(xp)) : getbig(xp);
        bignum_t *bd = bnexpt(bx, (unsigned long long)labs(getfix(yp)));
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
      return gnumtoi(zp, xt, xp);
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
  } else { /* general case: go inexact via polar form: (r,theta)^(c+di) */
    double rx, ix, ry, iy, rz, iz;
    if (isrect(xt)) recttodd(xt, xp, &rx, &ix); else comptodd(xt, xp, &rx, &ix);
    if (isrect(yt)) recttodd(yt, yp, &ry, &iy); else comptodd(yt, yp, &ry, &iy);
    cmath_pow(rx, ix, ry, iy, &rz, &iz);
    /* if inputs were purely real, and xt is positive, result is real */
    if (isrect(xt) && isreal(yt) && rx > 0.0 && iz == 0.0) return setflo(zp, rz); 
    return NUMT_MKCOM(setflo(zp, rz), setflo(zp+2, iz));
  }
}

/* z = real-part(x) */
numt_t gnumreal(nump_t *zp, numt_t xt, const nump_t *xp)
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
numt_t gnumimag(nump_t *zp, numt_t xt, const nump_t *xp)
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
numt_t gnummagn(nump_t *zp, numt_t xt, const nump_t *xp)
{
  assert(NUMT_IS_VALID(xt) && "unsupported number type");
  if (isreal(xt)) {
    return realabs(zp, xt, xp);
  } else if (isrect(xt)) {
    /* exact complex: do the math directly */
    nump_t rp[2]; numt_t rt = intmul(rp, NUMT_COM_R(xt), xp, NUMT_COM_R(xt), xp); 
    nump_t ip[2]; numt_t it = intmul(ip, NUMT_COM_I(xt), xp+2, NUMT_COM_I(xt), xp+2); 
    nump_t sp[2]; numt_t st = intadd(sp, rt, rp, it, ip);
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
numt_t gnumangle(nump_t *zp, numt_t xt, const nump_t *xp)
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
    double rx, ix;
    recttodd(xt, xp, &rx, &ix);
    return setflo(zp, atan2(ix, rx));
  } else {
    double rx = getflo(xp), ix = NUMT_COM_I(xt) ? getflo(xp+2) : 0.0;
    return setflo(zp, atan2(ix, rx));
  }
}

/* z = make-rectangular(x, y)  [x, y must be real] */
numt_t gnummakerect(nump_t *zp, numt_t xt, const nump_t *xp, numt_t yt, const nump_t *yp)
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
numt_t gnummakepolar(nump_t *zp, numt_t xt, const nump_t *xp, numt_t yt, const nump_t *yp)
{
  assert(NUMT_IS_VALID(xt) && "unsupported number type");
  assert(NUMT_IS_VALID(yt) && "unsupported number type");
  if (!isreal(xt) || !isreal(yt)) return setfail(EDOM);
  if ((isfix(yt) && getfix(yp) == 0) || (isfix(xt) && getfix(xp) == 0)) {
    return numdup(zp, xt, xp);
  } else {
    double r = isflo(xt) ? getflo(xp) : rattod(xt, xp);
    double theta = isflo(yt) ? getflo(yp) : rattod(yt, yp);
    return NUMT_MKCOM(setflo(zp, r*cos(theta)), setflo(zp+2, r*sin(theta)));
  }
}

/* inexact transcendental functions: all return inexact (compnum if needed) */

/* z = exp(x) */
numt_t gnumexp(nump_t *zp, numt_t xt, const nump_t *xp)
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

/* z = log(x)  [natural log; may produce complex for negative reals] */
numt_t gnumlog(nump_t *zp, numt_t xt, const nump_t *xp)
{
  assert(NUMT_IS_VALID(xt) && "unsupported number type");
  if (isfix(xt) && getfix(xp) == 0) {
    return setfail(EDOM); /* most Schemes fail */
  } else if (isfix(xt) && getfix(xp) == 1) {
    return setfix(zp, 0); /* consistent with exp */
  } else if (isreal(xt)) {
    double x = isflo(xt) ? getflo(xp) : rattod(xt, xp);
    /* if (x > 0.0 || x == 0.0 && 1.0/x > 0.0) return setflo(zp, log(x)); */
    if (x >= 0.0) return setflo(zp, log(x));
    /* negative real: log(x) = log|x| + i*pi */
    return NUMT_MKCOM(setflo(zp, log(-x)), setflo(zp+2, M_PI));
  } else {
    double rx, ix, re, im;
    if (isrect(xt)) recttodd(xt, xp, &rx, &ix); else comptodd(xt, xp, &rx, &ix);
    cmath_log(rx, ix, &re, &im);
    return NUMT_MKCOM(setflo(zp, re), setflo(zp+2, im));
  }
}

/* z = log(x, y)  [log base y] */
numt_t gnumlogn(nump_t *zp, numt_t xt, const nump_t *xp, numt_t yt, const nump_t *yp)
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
    } else { /* isreal(xt) */
      double x = isflo(xt) ? getflo(xp) : rattod(xt, xp);
      /* if (x > 0.0 || x == 0.0 && 1.0/x > 0.0) return setflo(zp, log10(x)); */
      if (x >= 0.0) return setflo(zp, log10(x));
      /* negative real: log10(x) = log10|x| + i*pi/log(10) */
      return NUMT_MKCOM(setflo(zp, log10(-x)), setflo(zp+2, M_PI_LN10));
    }
  } else {
    nump_t lxp[4]; numt_t lxt = gnumlog(lxp, xt, xp);
    nump_t lyp[4]; numt_t lyt = gnumlog(lyp, yt, yp);
    numt_t zt = gnumdiv(zp, lxt, lxp, lyt, lyp);
    numfini(lxt, lxp); numfini(lyt, lyp);
    return zt;
  }
}

/* z = sin(x) */
numt_t gnumsin(nump_t *zp, numt_t xt, const nump_t *xp)
{
  assert(NUMT_IS_VALID(xt) && "unsupported number type");
  if (isfix(xt) && getfix(xp) == 0) {
    return setfix(zp, 0); /* consistent with exp */
  } else if (isreal(xt)) {
    double x = isflo(xt) ? getflo(xp) : rattod(xt, xp);
    return setflo(zp, sin(x));
  } else {
    double rx, ix, re, im;
    if (isrect(xt)) recttodd(xt, xp, &rx, &ix); else comptodd(xt, xp, &rx, &ix);
    cmath_sin(rx, ix, &re, &im);
    return NUMT_MKCOM(setflo(zp, re), setflo(zp+2, im));
  }
}

/* z = cos(x) */
numt_t gnumcos(nump_t *zp, numt_t xt, const nump_t *xp)
{
  assert(NUMT_IS_VALID(xt) && "unsupported number type");
  if (isfix(xt) && getfix(xp) == 0) {
    return setfix(zp, 1); /* consistent with exp */
  } else if (isreal(xt)) {
    double x = isflo(xt) ? getflo(xp) : rattod(xt, xp);
    return setflo(zp, cos(x));
  } else {
    double rx, ix, re, im;
    if (isrect(xt)) recttodd(xt, xp, &rx, &ix); else comptodd(xt, xp, &rx, &ix);
    cmath_cos(rx, ix, &re, &im);
    return NUMT_MKCOM(setflo(zp, re), setflo(zp+2, im));
  }
}

/* z = tan(x) */
numt_t gnumtan(nump_t *zp, numt_t xt, const nump_t *xp)
{
  assert(NUMT_IS_VALID(xt) && "unsupported number type");
  if (isfix(xt) && getfix(xp) == 0) {
    return setfix(zp, 0); /* consistent with exp */
  } else if (isreal(xt)) {
    double x = isflo(xt) ? getflo(xp) : rattod(xt, xp);
    return setflo(zp, tan(x));
  } else {
    double rx, ix, re, im;
    if (isrect(xt)) recttodd(xt, xp, &rx, &ix); else comptodd(xt, xp, &rx, &ix);
    cmath_tan(rx, ix, &re, &im);
    return NUMT_MKCOM(setflo(zp, re), setflo(zp+2, im));
  }
}

/* z = asin(x) */
numt_t gnumasin(nump_t *zp, numt_t xt, const nump_t *xp)
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
numt_t gnumacos(nump_t *zp, numt_t xt, const nump_t *xp)
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
numt_t gnumatan(nump_t *zp, numt_t xt, const nump_t *xp)
{
  double rx, ix, re, im;
  assert(NUMT_IS_VALID(xt) && "unsupported number type");
  if (isfix(xt) && getfix(xp) == 0) {
    return setfix(zp, 0); /* consistent with tan */
  } else if (isreal(xt)) {
    rx = isflo(xt) ? getflo(xp) : rattod(xt, xp);
    ix = 0.0;
  } else {
    if (iscomp(xt)) comptodd(xt, xp, &rx, &ix); else recttodd(xt, xp, &rx, &ix);
  }
  if (ix == 0.0) {
    if (isreal(xt)) return setflo(zp, atan(rx));
    return NUMT_MKCOM(setflo(zp, atan(rx)), setflo(zp+2, 0.0));
  }
  cmath_atan(rx, ix, &re, &im);
  return NUMT_MKCOM(setflo(zp, re), setflo(zp+2, im));
}

/* z = atan(y, x) [2-argument, real only] */
numt_t gnumatan2(nump_t *zp, numt_t yt, const nump_t *yp, numt_t xt, const nump_t *xp)
{
  assert(NUMT_IS_VALID(xt) && "unsupported number type");
  assert(NUMT_IS_VALID(yt) && "unsupported number type");
  if (!isreal(xt) || !isreal(yt)) return setfail(EDOM);
  /* special corner case fixes */
  if (isfix(yt) && getfix(yp) == 0 && isfix(xt)) {
    if (getfix(xp) > 0) return setfix(zp, 0); 
    else if (getfix(xp) < 0) return setflo(zp, M_PI); 
    else return setfail(EDOM);
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
int gnumodd(numt_t xt, const nump_t *xp)
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

int gnumeven(numt_t xt, const nump_t *xp)
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

int gnumzero(numt_t xt, const nump_t *xp)
{
  assert(NUMT_IS_VALID(xt) && "unsupported number type");
  if (isfix(xt)) return getfix(xp) == 0;
  if (isflo(xt)) return getflo(xp) == 0.0;
  if (iscomp(xt)) return getflo(xp) == 0.0 && getflo(xp+2) == 0.0;
  return 0;
}

int gnumpositive(numt_t xt, const nump_t *xp)
{
  assert(NUMT_IS_VALID(xt) && "unsupported number type");
  if (!isreal(xt)) return -1;
  return realcmp0(xt, xp, NCMP_GT);
}

int gnumnegative(numt_t xt, const nump_t *xp)
{
  assert(NUMT_IS_VALID(xt) && "unsupported number type");
  if (!isreal(xt)) return -1;
  return realcmp0(xt, xp, NCMP_LT);
}

/* exact?(x) and inexact?(x) */
int gnumexactp(numt_t xt, const nump_t *xp)
{
  assert(NUMT_IS_VALID(xt) && "unsupported number type");
  (void)xp;
  return isrect(xt);
}

int gnuminexactp(numt_t xt, const nump_t *xp)
{
  assert(NUMT_IS_VALID(xt) && "unsupported number type");
  (void)xp;
  return !isrect(xt);
}

int gnumintegerp(numt_t xt, const nump_t *xp)
{
  assert(NUMT_IS_VALID(xt) && "unsupported number type");
  if (!isreal(xt)) return 0;
  return isflo(xt) ? flisint(getflo(xp)) : isint(xt);
}

int gnumrationalp(numt_t xt, const nump_t *xp)
{
  assert(NUMT_IS_VALID(xt) && "unsupported number type");
  if (!isreal(xt)) return 0;
  if (isflo(xt)) {
    double x = getflo(xp);
    return (x == x) && (x > -HUGE_VAL) && (x < HUGE_VAL);
  }
  return 1;  /* all exact reals are rational */
}

int gnumrealp(numt_t xt, const nump_t *xp)
{
  assert(NUMT_IS_VALID(xt) && "unsupported number type");
  return isreal(xt);
}

int gnumcomplexp(numt_t xt, const nump_t *xp)
{
  assert(NUMT_IS_VALID(xt) && "unsupported number type");
  (void)xp;
  return 1;  /* all generic numbers are complex */
}

int gnumnumberp(numt_t xt, const nump_t *xp)
{
  (void)xp;
  return NUMT_IS_VALID(xt); /* whatever */
}

int gnumfinitep(numt_t xt, const nump_t *xp)
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

int gnuminfinitep(numt_t xt, const nump_t *xp)
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

int gnumnanp(numt_t xt, const nump_t *xp)
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
numt_t strtognum(nump_t *zp, const char *str, char **endptr, int radix)
{
  numt_t zt = NUMT_NONE; char *cp;
  int forceie = 0; /* -1=#i 1=#e */
  int flags = 0; /* CNF_XXX|... */
  assert(str); assert(radix >= 2 && radix <= 36);
  /* returns start of complex part and sets radix/ie; returns NULL on errors */
  if ((cp = check_number(str, &radix, &forceie, &flags)) == NULL) { 
    /* invalid number syntax */
    if (endptr) *endptr = (char*)str;
    return setfail(EDOM);
  }
  /* positon at the start of complex */
  str = (const char *)cp;
  if (forceie < 0) { /* as inexact */
    zt = strtocomp(zp, str, endptr, radix);
  } else if (forceie > 0) { /* as exact */
    zt = strtorect(zp, str, endptr, radix);
    /* catch polar -> inexact */
    if (iscomp(zt)) zt = setfail(ERANGE); 
  } else if (flags & CNF_DOTEXP) { /* has elements of inexact notation */
    zt = strtocomp(zp, str, endptr, radix);
  } else { /* no elements of inexact notation */
    zt = strtorect(zp, str, endptr, radix);
    /* polar -> inexact is ok */
    if (iscomp(zt) && errno == ERANGE) errno = 0; 
  }
  return zt;
}

/* # of chars needed for x in radix; return 0 if x is inexact and radix != 10  */
size_t gnumfmtsize(numt_t xt, const nump_t *xp, int radix)
{
  assert(radix >= 2 && radix <= 36);
  assert(NUMT_IS_VALID(xt) && "unsupported number type");
  if (isrect(xt)) {
    return rectfmtsize(xt, xp, radix);
  } else {
    return compfmtsize(xt, xp, radix);
  }
}

/* format x into buffer; len should be as calculated by realfmtsize;
 * returns ptr to first char of zero-terminated result in buffer
 * or NULL if an inexact number is printed in wrong radix. */
char *gnumtostr(char *buffer, size_t len, numt_t xt, const nump_t *xp, int radix)
{
  assert(radix >= 2 && radix <= 36);
  assert(NUMT_IS_VALID(xt) && "unsupported number type");
  if (isrect(xt)) {
    return recttostr(buffer, len, xt, xp, radix);
  } else {
    return comptostr(buffer, len, xt, xp, radix);
  }
}

/* print x in radix; use a-z if radix > 10; return -1 if x is inexact and radix != 10 */
int gnumprint(FILE *fp, numt_t xt, const nump_t *xp, int radix)
{
  assert(radix >= 2 && radix <= 36);
  assert(NUMT_IS_VALID(xt) && "unsupported number type");
  if (isrect(xt)) {
    rectprint(fp, xt, xp, radix);
  } else {
    compprint(fp, xt, xp, radix);
  }
  return 0;  
}



/* bignums as skint objects */

static cxtype_t cxt_bignum = { "bignum", bnfree };
cxtype_t *BIGNUM_NTAG = &cxt_bignum;

