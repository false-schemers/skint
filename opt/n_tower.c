/* n_tower.c -- numerical tower */

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

#if 0
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

/* Derived constants for buffer safety */
/* Binary (bbits=1) is the worst case for digit count: DBL_MANT_DIG = 53 */
#define GN_DIG_MAX (1 + (DBL_MANT_DIG - 1) + 4)
/* Buffer must handle the worst-case binary shift: ~1024 digits + digits + margin */
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
#endif

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

/* types for platform-independent long its */
#define LONGT_MAX INT64_MAX
typedef int64_t long_t;
#define ULONGT_MAX UINT64_MAX
typedef uint64_t ulong_t;

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
bignum_t *ltobn(long_t n);

static bignum_t zero = {0, DUP_STATIC, 0, {0}};
bignum_t *bn0 = &zero;

static bignum_t one =  {1, DUP_STATIC, 0, {1}};
bignum_t *bn1 = &one;

static void bnx_failure(char *msg)
{
  fprintf(stderr, "; bnx_failure(%s)\n", msg);
  // exit(1); // abort(); // cxm_check(0, msg);
}

static bignum_t *bnx_zdiv(void)
{
  fprintf(stderr, "; division by zero\n");
  // exit(1); // abort(); // cxm_check(0, "division by zero (bignum)");
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
  *r = (limb_t)(long_t)h; 

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
      limb_t f0 = LO_LIMB(HILO(-(sd_hi+1), 0) / (sd_hi+1));
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

int bncmpl(const bignum_t *n, long_t fix)
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

bignum_t *bnaddl(const bignum_t *n, long_t incr)
{
  struct bignum_ll b;
  bignum_t *r = bn0;

  assert(n != NULL);
  CHECKSIGN(n);

  if (BNZERO(n))
    return ltobn(incr);

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

static bignum_t *bnashl(const bignum_t *a, long_t sh)
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

bignum_t *bnmull(const bignum_t *n, long_t v)
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

static bignum_t *bnexpt(const bignum_t *a, ulong_t n)
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
    if (n >= LONGT_MAX) bnx_failure("bnexpt");  
    return bnashl(bn1, n);
  }

  if (BNMINUSTWO(a)) { /* esl++ */
    if (n >= LONGT_MAX) bnx_failure("bnexpt");
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
    long_t v;
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
  } else if (den > -LIMB_MAX) {
    d = bnz_dmodl(&rl, num, (limb_t)-den);
    isneg = 1;
  } else {
    d = bnz_dmod(&r, num, bnx_makell(&b, den));
    isneg = 1;
    goto from_limb;
  }

  if (num->isneg)
    *rem = -(long_t)rl;
  else
    *rem = rl;

  if (!BNZERO(d))
    d->isneg = isneg != num->isneg;
  return_NORMALIZE(d, "bndmodl");

from_limb:
#if (ULONG_MAX > LIMB_MAX)
  {
    size_t i;
    long_t v;
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
  } else if (den > -LIMB_MAX) {
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
  } else if (den > -LIMB_MAX) {
    bnfree(bnz_dmodl(&reml, num, -den));
  } else {
    bnfree(bnz_dmod(&rem, num, bnx_makell(&b, den)));
    goto from_limb;
  }
  if (num->isneg)
    return -(long_t)reml;
  else
    return reml;
from_limb:
#if (ULONG_MAX > LIMB_MAX)
  {
    size_t i;
    long_t v;
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
  return ((num->isneg)? -reml: reml);
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
    for (i = n->size - 1; i--;) {
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

bignum_t *ltobn(long_t n)
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
  size = strlen(ptr);
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
    return bnashl(a, (long_t)cnt);
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

    r = bnashl(a, (long_t)cnt);
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


/* bignums as skint objects */

static cxtype_t cxt_bignum = { "bignum", bnfree };

cxtype_t *BIGNUM_NTAG = &cxt_bignum;

