/* n_tower.c -- numerical tower */

/* complex number arithmetic */
#define CMATH_LOG_DBL_MAX 709.782712893384
#define C90_BIG 1.0e154

/* cmath_cdiv: z/w via Smith's algorithm, handles inf/NaN */
void cmath_cdiv(double rx, double ix, double ry, double iy, double *prz, double *piz) 
{
  double ar, ai, den, t;
  if (ry == 0.0 && iy == 0.0) { *prz = *piz = HUGE_VAL - HUGE_VAL; return; }
  if (rx != rx || ix != ix || ry != ry || iy != iy) {
    *prz = *piz = HUGE_VAL - HUGE_VAL; return;
  }
  if (fabs(ry) >= fabs(iy)) {
    t = iy / ry;
    /* pure-real divisor: z / ry */
    if (t == 0.0) { *prz = rx / ry; *piz = ix / ry; return; }
    den = ry + iy * t; ar = rx + ix * t; ai = ix - rx * t;
  } else {
    t = ry / iy;
    /* pure-imag divisor: z/(i*iy) = (ix - i*rx)/iy */
    if (t == 0.0) { *prz = ix / iy; *piz = -rx / iy; return; }
    den = iy + ry * t; ar = rx * t + ix; ai = ix * t - rx;
  }
  *prz = ar / den;
  *piz = ai / den;
}

#ifndef C99_MATH_LIB
static double c90_copysign(double x, double y) 
{
  if (y < 0.0) return -fabs(x);
  if (y > 0.0) return fabs(x);
  return (1.0/y < 0.0) ? -fabs(x) : fabs(x);
}
#define copysign(x, y) c90_copysign(x, y)
#endif

#ifndef C99_MATH_LIB
static double c90_log1p(double x) 
{
  volatile double u;
  if (x == 0.0) return x;
  u = 1.0 + x;
  if (u == 1.0) return x;
  return log(u) * (x / (u - 1.0));
}
#define log1p(x) c90_log1p(x)
#endif

#ifndef C99_MATH_LIB
static double c90_hypot(double a, double b) 
{
  double aa = fabs(a), bb = fabs(b), t;
  if (aa == HUGE_VAL || bb == HUGE_VAL) return HUGE_VAL;
  if (aa != aa || bb == 0.0) return aa;
  if (bb != bb || aa == 0.0) return bb;
  if (aa < bb) { t = aa; aa = bb; bb = t; }
  t = bb / aa;
  return aa * sqrt(1.0 + t*t);
}
#define hypot(a, b) c90_hypot(a, b)
#endif

/* c90_asinh, stable for all magnitudes */
static double c90_asinh(double x) 
{
  double ax = fabs(x), r;
  if (ax == 0.0) return x;
  if (ax > 1.0e154) {
    r = log(ax) + M_LN2;
  } else if (ax > 1.0) {
    /* Correct identity: log(x) + log(1 + sqrt(1 + 1/x^2)) */
    r = log(ax) + log1p(sqrt(1.0 + 1.0/(ax*ax)));
  } else {
    r = log1p(ax + (ax*ax)/(1.0 + sqrt(1.0 + ax*ax)));
  }
  return copysign(r, x);
}

/* c90_sinhcosh: handles large arguments to avoid inf*0 = NaN */
static void c90_sinhcosh(double x, double *sh, double *ch) 
{
  double ax = fabs(x);
  if (ax != ax) { *sh = *ch = x + x; return; } /* NaN */
  if (ax < 0.25) {
    /* truncation after x^10/11! is < 1e-17 relative for |x| < 0.25. */
    double x2 = x*x;
    *sh = x * (1.0 + x2*(1.0/6.0
             + x2*(1.0/120.0
             + x2*(1.0/5040.0
             + x2*(1.0/362880.0
             + x2*(1.0/39916800.0))))));
    *ch = sqrt(1.0 + (*sh)*(*sh)); /* accurate: result is ~1 */
    return;
  }
  if (ax < 22.0) { /* cancellation now costs <2 bits */
    double e = exp(ax), r = 1.0/e;
    *sh = (x < 0.0 ? -0.5 : 0.5) * (e - r);
    *ch = 0.5 * (e + r);
    return;
  }
  /* e^-ax < e^-44 < eps/2: sinh == cosh == e^ax/2 to full precision.
     Compute as exp(ax - ln2) so we only overflow when the result really does. */
  { double t = exp(ax - M_LN2);
    *ch = t;
    *sh = (x < 0.0) ? -t : t; }
}

/* c90_cbrt: C90 replacement for C99 cbrt(), Halley-refined */
double c90_cbrt(double x) 
{
  double a, y, y3, r;
  int neg;
  if (x == 0.0 || x != x || x > DBL_MAX || x < -DBL_MAX) return x;
  neg = (x < 0.0);
  a = neg ? -x : x;
  y = pow(a, 1.0 / 3.0);
  if (y == 0.0) return neg ? -0.0 : 0.0;
  y3 = y * y * y;
  if (y3 == 0.0 || y3 > DBL_MAX || y3 < -DBL_MAX) r = a / y / y / y;
  else r = a / y3;
  y = y * (1.0 + 2.0 * r) / (2.0 + r);
  return neg ? -y : y;
}

/* cmath_exp: exp(rx + i*ix), with overflow scaling for small |ix| */
void cmath_exp(double rx, double ix, double *prz, double *piz) 
{
  double er;
  int ix_neg = (ix < 0.0) || (ix == 0.0 && 1.0/ix < 0.0);
  if (ix == 0.0) { *prz = exp(rx); *piz = ix; return; }
  if (rx != rx) { *prz = rx; *piz = ix; return; }
  er = exp(rx);
  if (er == HUGE_VAL && fabs(ix) < 1.0) {
    double log_ix = log(fabs(ix)), scaled = rx + log_ix;
    *prz = HUGE_VAL;
    if (scaled < CMATH_LOG_DBL_MAX) *piz = (ix_neg ? -1.0 : 1.0) * exp(scaled);
    else *piz = ix_neg ? -HUGE_VAL : HUGE_VAL;
    return;
  }
  *prz = er * cos(ix);
  *piz = er * sin(ix);
}

/* cmath_log: ln|z| + i*arg(z), principal branch, log1p near z=1 */
void cmath_log(double rx, double ix, double *prz, double *piz) 
{
  double ar = fabs(rx), ai = fabs(ix), t;
  if (ar == 0.0 && ai == 0.0) *prz = -HUGE_VAL;
  else if (ar < ai) { t = ar / ai; *prz = log(ai) + 0.5 * log1p(t*t); }
  else { t = ai / ar; *prz = log(ar) + 0.5 * log1p(t*t); }
  *piz = atan2(ix, rx);
}

/* cmath_sqrt: principal sqrt, Re >= 0 (Numerical Recipes 5.4) */
void cmath_sqrt(double rx, double ix, double *prz, double *piz) 
{
  double m, re, im;
  int ix_neg = (ix < 0.0) || (ix == 0.0 && 1.0/ix < 0.0);
  int rx_neg = (rx < 0.0) || (rx == 0.0 && 1.0/rx < 0.0);
  if (rx != rx || ix != ix) { *prz = *piz = HUGE_VAL - HUGE_VAL; return; }
  if (ix == HUGE_VAL || ix == -HUGE_VAL) 
    { *prz = HUGE_VAL; *piz = ix_neg ? -HUGE_VAL : HUGE_VAL; return; }
  if (rx == HUGE_VAL) { *prz = HUGE_VAL; *piz = ix_neg ? -0.0 : 0.0; return; }
  if (rx == -HUGE_VAL) { *prz = 0.0; *piz = ix_neg ? -HUGE_VAL : HUGE_VAL; return; }
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

/* cmath_sin: sin(rx+ix) = sin(rx)cosh(ix) + i*cos(rx)sinh(ix) */
void cmath_sin(double rx, double ix, double *prz, double *piz) 
{
  double sh, ch, srx = sin(rx), crx = cos(rx);
  c90_sinhcosh(ix, &sh, &ch);
  *prz = (srx == 0.0) ? srx : (srx * ch);
  *piz = (crx == 0.0) ? crx * copysign(1.0, sh) : (crx * sh);
}

/* cmath_cos: cos(rx+ix) = cos(rx)cosh(ix) - i*sin(rx)sinh(ix) */
void cmath_cos(double rx, double ix, double *prz, double *piz) 
{
  double sh, ch, srx = sin(rx), crx = cos(rx);
  c90_sinhcosh(ix, &sh, &ch);
  *prz = (crx == 0.0) ? crx : (crx * ch);
  *piz = (srx == 0.0) ? (-srx) * copysign(1.0, sh) : -(srx * sh);
}

/* cmath_tan: tan(z), asymptotic for large |Im(z)|, handles inf/NaN */
void cmath_tan(double rx, double ix, double *prz, double *piz) 
{
  if (rx != rx || ix != ix) { *prz = *piz = HUGE_VAL - HUGE_VAL; return; }
  if (ix == HUGE_VAL || ix == -HUGE_VAL) {
    *prz = 0.0; *piz = (ix > 0.0) ? 1.0 : -1.0; return;
  }
  if (fabs(ix) > 20.0) { *prz = 0.0; *piz = (ix > 0.0) ? 1.0 : -1.0; return; }
  { double cos_rx = cos(rx), sinh_ix, cosh_ix, d;
    c90_sinhcosh(ix, &sinh_ix, &cosh_ix);
    d = cos_rx*cos_rx + sinh_ix*sinh_ix;
    if (d == 0.0) { *prz = HUGE_VAL; *piz = 0.0; }
    else { *prz = (sin(rx)*cos_rx)/d; *piz = (sinh_ix*cosh_ix)/d; }
  }
}

/* cmath_asin: -i*log(iz + sqrt(1-z^2)), principal branch */
void cmath_asin(double rx, double ix, double *prz, double *piz) 
{
  double ar, ai, br, bi, den, arg;
  cmath_sqrt(1.0 - rx, -ix, &ar, &ai);
  cmath_sqrt(1.0 + rx, ix, &br, &bi);
  den = ar*br - ai*bi;
  arg = ar*bi - ai*br;
  *prz = atan2(rx, den);
  *piz = c90_asinh(arg);
}

/* cmath_acos: pi/2 - asin(z), principal branch */
void cmath_acos(double rx, double ix, double *prz, double *piz) 
{
  double tr, ti;
  cmath_asin(rx, ix, &tr, &ti);
  *prz = M_PI_2 - tr;
  *piz = -ti;
}

/* cmath_atan: atan(z) = (i/2)*log((1-iz)/(1+iz)), Kahan-stable.
 * Re = 0.5  * atan2(2x, 1 - x^2 - y^2)
 * Im = 0.25 * log1p( 4|y| / (x^2 + (|y|-1)^2) ), sign taken from y
 * requires C90_BIG <= sqrt(DBL_MAX/2) ~ 9.5e153 so x*x+d*d cannot overflow */
void cmath_atan(double rx, double ix, double *pre, double *pim)
{
  double x = rx, y = ix, ax = fabs(x), ay = fabs(y), re, im;

  /* non-finite? */
  if (ay == HUGE_VAL) { /* x + i*(+-inf) */
    *pre = (x != x) ? x : copysign(M_PI_2, x);
    *pim = copysign(0.0, y);
    return;
  }
  if (ax == HUGE_VAL) { /* (+-inf) + iy, y may be NaN */
    *pre = copysign(M_PI_2, x);
    *pim = copysign(0.0, y);
    return;
  }
  if (x != x || y != y) { *pre = *pim = x + y; return; }   /* NaN */

  /* real part */
  if (x == 0.0) {
    re = (ay > 1.0) ? copysign(M_PI_2, x) : x;
  } else if (ax >= C90_BIG || ay >= C90_BIG) {
    re = copysign(M_PI_2, x);
  } else {
    double den = (ax < ay) ? (1.0 - ay)*(1.0 + ay) - x*x : (1.0 - ax)*(1.0 + ax) - y*y;
    re = 0.5 * atan2(2.0*x, den);
  }

  /* imaginary part: argument always >= 0 */
  if (ax >= C90_BIG || ay >= C90_BIG) {
    /* atan z ~ +-pi/2 - 1/z  =>  Im ~ y/(x^2+y^2); scaled, cannot overflow */
    double h = hypot(x, y);
    im = (y / h) / h;
  } else if (ay == 1.0 && ax < 1.0e-150) {
    /* at the pole x*x would underflow: 0.25*log(4/x^2) = (ln2 - ln|x|)/2 */
    im = copysign(0.5 * (M_LN2 - log(ax)), y);
  } else {
    double d = ay - 1.0;  /* exact for 0.5 <= ay <= 2 (Sterbenz) */
    double q = x*x + d*d; /* > 0 here, no cancellation possible  */
    im = copysign(0.25 * log1p(4.0*ay / q), y);
  }
  
  *pre = re;
  *pim = im;
}

/* inf/NaN-safe complex multiply: short-circuits when any component is zero */
static void cmath_cmul(double ar, double ai, double br, double bi, double *cr, double *ci) 
{
  if (ai == 0.0 && bi == 0.0) { *cr = ar*br;  *ci = 0.0;   return; }
  if (ai == 0.0)              { *cr = ar*br;  *ci = ar*bi;  return; }
  if (bi == 0.0)              { *cr = ar*br;  *ci = ai*br;  return; }
  if (ar == 0.0)              { *cr = -ai*bi; *ci = ai*br;  return; }
  if (br == 0.0)              { *cr = -ai*bi; *ci = ar*bi;  return; }
  { double r = ar*br - ai*bi, i = ar*bi + ai*br; *cr = r; *ci = i; }
}

/* cmath_pow: z^w, integer exponents via repeated squaring (bound |n|<=1024
 * prevents LONG_MIN overflow), else exp(w*log z). */
void cmath_pow(double rx, double ix, double ry, double iy, double *prz, double *piz) 
{
  if (iy == 0.0 && ry == floor(ry) && fabs(ry) <= 1024.0) {
    long n = (long)ry;
    double rr = 1.0, ri = 0.0, br = rx, bi = ix;
    int neg = (n < 0), have = 0;
    if (neg) n = -n;
    while (n > 0) {
      if (n & 1) {
        if (!have) { rr = br; ri = bi; have = 1; }
        else { cmath_cmul(rr, ri, br, bi, &rr, &ri); }
      }
      n >>= 1;
      if (n > 0) { cmath_cmul(br, bi, br, bi, &br, &bi); }
    }
    /* n == 0 leaves have == 0 ? rr,ri = 1,0 which is the correct z^0 */
    if (neg) cmath_cdiv(1.0, 0.0, rr, ri, &rr, &ri);
    *prz = rr; *piz = ri;
    return;
  }
  if (iy == 0.0) {
    if (ry == 0.0) { *prz = 1.0; *piz = 0.0; return; }
    if (ry == 0.5) { cmath_sqrt(rx, ix, prz, piz); return; }
    if (ry == 1.0) { *prz = rx; *piz = ix; return; }
    if (ry == -1.0) { cmath_cdiv(1.0, 0.0, rx, ix, prz, piz); return; }
  }
  { double lr, li, pr, pi;
    cmath_log(rx, ix, &lr, &li);
    pr = ry*lr - iy*li;
    pi = ry*li + iy*lr;
    cmath_exp(pr, pi, prz, piz);
  }
}

/* cmath_hypot: C99-compliant hypot with NaN/inf handling */
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

/* cmath_cbrt: principal complex cube root, overflow-safe via scaling */
void cmath_cbrt(double rx, double ix, double *prz, double *piz) 
{
  double m, r_cbrt, theta, max_val;
  int ix_neg = (ix < 0.0) || (ix == 0.0 && 1.0/ix < 0.0);
  if (rx != rx || ix != ix) { *prz = *piz = rx + ix; return; }
  if (rx == HUGE_VAL || rx == -HUGE_VAL || ix == HUGE_VAL || ix == -HUGE_VAL) {
    if (rx == -HUGE_VAL && fabs(ix) < HUGE_VAL) 
      { *prz = HUGE_VAL; *piz = ix_neg ? -HUGE_VAL : HUGE_VAL; return; }
    if (rx == HUGE_VAL && fabs(ix) < HUGE_VAL) 
      { *prz = HUGE_VAL; *piz = ix_neg ? -0.0 : 0.0; return; }
    *prz = HUGE_VAL; *piz = ix_neg ? -HUGE_VAL : HUGE_VAL; return;
  }
  if (rx == 0.0 && ix == 0.0) { *prz = 0.0; *piz = ix_neg ? -0.0 : 0.0; return; }
  max_val = fabs(rx) > fabs(ix) ? fabs(rx) : fabs(ix);
  if (max_val > 1.0e150) 
    { m = hypot(ldexp(rx, -600), ldexp(ix, -600)); r_cbrt = ldexp(c90_cbrt(m), 200); }
  else if (max_val < 1.0e-150) 
    { m = hypot(ldexp(rx, 600), ldexp(ix, 600)); r_cbrt = ldexp(c90_cbrt(m), -200); }
  else { m = hypot(rx, ix); r_cbrt = c90_cbrt(m); }
  theta = atan2(ix, rx);
  *prz = r_cbrt * cos(theta / 3.0);
  *piz = r_cbrt * sin(theta / 3.0);
}


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

static bignum_t plus1 =  {1, DUP_STATIC, 0, {1}};
bignum_t *bn1 = &plus1;

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

#define BIGNUM_MAX_LIMBS   ((size_t)(BIGNUM_MAX_BITS / LIMB_BITS))
#define BIGNUM_RZ_LIMBS    ((BIGNUM_MAX_LIMBS / 128) < 4 ? 4 : (BIGNUM_MAX_LIMBS / 128))

/* Input guards: bail if combined limb count leaves insufficient headroom */
#define IN_RZ2(a, b)       (((a)->size + (b)->size) >= BIGNUM_RZ_LIMBS)
#define IN_RZ4(a, b, c, d) (((a)->size + (b)->size + (c)->size + (d)->size) >= BIGNUM_RZ_LIMBS)

/* Loop/operation guards: tighter limits for repeated growth */
#define IN_RZ_LOOP2(a, b)  (((a)->size + (b)->size) >= BIGNUM_MAX_LIMBS / 4)
#define IN_RZ_SHIFT(base, shift) ((base)->size + (shift) / LIMB_BITS >= BIGNUM_MAX_LIMBS / 2)
#define IN_RZ_EXPT(base, exp)    ((base)->size > 0 && (exp) > BIGNUM_MAX_LIMBS / (base)->size)

/* additional static bignum constants */
static const bignum_t plus2 = {1, DUP_STATIC, 0, {2}}, *bn2 = &plus2; 
static const bignum_t plus3 = {1, DUP_STATIC, 0, {3}}, *bn3 = &plus3;

/* forward declarations */
double bnratan2tod(const bignum_t *ny, const bignum_t *dy, const bignum_t *nx, const bignum_t *dx);
void bnrsincostod(const bignum_t *n, const bignum_t *d, double *psin, double *pcos);
double bnrlogtod(const bignum_t *n, const bignum_t *d);

/* bnrfrexp: extract normalized mantissa m in [0.5, 1.0) and binary exponent *pe.
 * Mathematically: (n / d) == m * 2^(*pe).
 * Normalizes q based on actual width after rounding to ensure correct mantissa range. */
static double bnrfrexp(const bignum_t *n, const bignum_t *d, long *pe)
{
  bignum_t *a, *b, *q, *r, *t;
  size_t wa, wb, wq; long e, low, shift;
  double m; int s = ((bnsign(n) < 0) ^ (bnsign(d) < 0)) ? -1 : 1;
  int round_up = 0;

  if (bnzero(n)) { if (pe) *pe = 0; return s * 0.0; }
  a = bnabs(n); b = bnabs(d);
  if (bnzero(d)) { bnfree(a); bnfree(b); if (pe) *pe = 0; return s * HUGE_VAL; }
  wa = bnwidthu(a); wb = bnwidthu(b);
  e = (long)wa - (long)wb;

  shift = (long)(DBL_MANT_DIG + 2) + (long)wb - (long)wa;
  if (shift >= 0) { t = bnashll(a, shift); q = bndmod(&r, t, b); bnfree(t); } 
  else { t = bnashll(b, -shift); q = bndmod(&r, a, t); bnfree(t); }
  bnfree(a); bnfree(b);

  low = bnmodl(q, 4);
  if (low > 2) {
    round_up = 1;
  } else if (low == 2) {
    if (!bnzero(r)) { round_up = 1; } 
    else { t = bnashll(q, -2); if (bnodd(t)) round_up = 1; bnfree(t); }
  }
  bnfree(r);

  t = bnashll(q, -2); bnfree(q); q = t;
  if (round_up) { t = bnaddll(q, 1); bnfree(q); q = t; }

  /* normalize q to exactly DBL_MANT_DIG bits */
  wq = bnwidthu(q);
  if (wq > DBL_MANT_DIG) {
    ++e;
    t = bnashll(q, -1); bnfree(q); q = t;
  } else if (wq < DBL_MANT_DIG) {
    long diff = (long)DBL_MANT_DIG - (long)wq;
    e -= diff;
    t = bnashll(q, diff); bnfree(q); q = t;
  }

  m = s * ldexp(bntod(q), -(long)DBL_MANT_DIG);
  bnfree(q);

  if (pe) *pe = e;
  return m;
}

static double bnrldexp(double m, long e)
{
  const long max_e = DBL_MAX_EXP - DBL_MIN_EXP + DBL_MANT_DIG + 1;
  if (m == 0.0 || m != m) return m;
  if (e >  max_e) e =  max_e;
  if (e < -max_e) e = -max_e;
  return ldexp(m, (int)e);
}

double bnrtod(const bignum_t *n, const bignum_t *d)
{
  long e;
  double m = bnrfrexp(n, d, &e);
  return bnrldexp(m, e);
}

/* bnreduce: reduce *pp / *pq in place by their GCD */
static void bnreduce(bignum_t **pp, bignum_t **pq) 
{
  bignum_t *g = bngcd(*pp, *pq);
  if (!BNONE(g, 0)) {
    bignum_t *np = bndiv(*pp, g), *nq = bndiv(*pq, g);
    bnfree(*pp); bnfree(*pq);
    *pp = np; *pq = nq;
  }
  bnfree(g);
}


/* [esl+] exact sqrt bundle */

/* modular quadratic residue lookup tables for fast non-square rejection */
static const unsigned char sq64[64] = {
  1,1,0,0,1,0,0,0, 0,1,0,0,0,0,0,0,
  1,1,0,0,0,0,0,0, 0,1,0,0,0,0,0,0,
  0,1,0,0,1,0,0,0, 0,1,0,0,0,0,0,0,
  0,1,0,0,0,0,0,0, 0,1,0,0,0,0,0,0
};

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

/* bnx_mag_modl: compute |n| mod m, with fast path for m=64 */
static unsigned long bnx_mag_modl(const bignum_t *n, long m)
{
  long r;
  assert(n->size > 0);
  if (m == 64) return (unsigned long)(n->limb[0] & 63UL);
  r = bnmodl(n, m);
  if (r < 0) r = -r;
  return (unsigned long)(r % m);
}

/* bnx_maybe_square: quick filter using quadratic residue tables */
static int bnx_maybe_square(const bignum_t *x)
{
  if (BNZERO(x)) return 1;
  if (!sq64[bnx_mag_modl(x, 64)]) return 0;
  if (!sq63[bnx_mag_modl(x, 63)]) return 0;
  if (!sq65[bnx_mag_modl(x, 65)]) return 0;
  if (!sq11[bnx_mag_modl(x, 11)]) return 0;
  return 1;
}

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

/* bnisqrt_initial_seed: hardware-accelerated overestimate for Newton-Raphson */
static bignum_t *bnisqrt_initial_seed(const bignum_t *n_abs)
{
  size_t w = bnwidthu(n_abs);
  bignum_t *x = NULL;
  const size_t MAX_EXACT_BITS = (size_t)(DBL_MANT_DIG - 1);

  if (w <= MAX_EXACT_BITS) {
    double d = fabs(bntod_approx(n_abs));
    double s = sqrt(d);
    x = dtobn(s + 1.0);
  } else {
    long long k = (long long)((w - MAX_EXACT_BITS) & ~(unsigned long long)1);
    bignum_t *n_top = bnashll(n_abs, -k);
    double d = fabs(bntod_approx(n_top));
    double s = sqrt(d);
    bignum_t *x_top = dtobn(s + 1.0);
    if (x_top) {
      x = bnashll(x_top, (k / 2));
      bnfree(x_top);
    }
    bnfree(n_top);
  }
  return x;
}

/* bnisqrt: exact integer square root of |n|.
 * |n| = root^2 + rem, root >= 0, rem >= 0. Returns 1 if rem == 0. */
int bnisqrt(const bignum_t *n, bignum_t **out_root, bignum_t **out_rem)
{
  bignum_t *n_abs, *x, *next_x, *q, *sum, *sq, *rem;
  int res;

  assert(n != NULL);
  assert(out_root != NULL);

  if (BNZERO(n)) {
    *out_root = bn0;
    if (out_rem) *out_rem = bn0;
    return 1;
  }

  n_abs = bnabs(n);
  x = bnisqrt_initial_seed(n_abs);

  for (;;) {
    q = bndiv(n_abs, x);
    sum = bnadd(x, q);
    bnfree(q);

    next_x = bnashll(sum, -1);
    bnfree(sum);

    if (bncmp(next_x, x) >= 0) {
      bnfree(next_x);
      break;
    }
    bnfree(x);
    x = next_x;
  }

  *out_root = x;

  sq = bnmul(x, x);
  rem = bnsub(n_abs, sq);
  bnfree(sq);
  bnfree(n_abs);

  res = BNZERO(rem);
  if (out_rem) *out_rem = rem;
  else bnfree(rem);
  return res;
}

/* bntrysqrt: check if |n| has an exact integer square root */
int bntrysqrt(const bignum_t *n, bignum_t **out_root)
{
  bignum_t *root = NULL, *rem = NULL;

  assert(n != NULL);

  if (BNZERO(n)) {
    if (out_root) *out_root = bn0;
    return 1;
  }

  if (!bnx_maybe_square(n)) return 0;

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

/* bnrtrysqrt_reduced: exact rational sqrt of nx/dx (assumed reduced).
 * Outputs are guaranteed to be in lowest terms. */
int bnrtrysqrt_reduced(const bignum_t *nx, const bignum_t *dx,
                       bignum_t **pnr, bignum_t **pdr)
{
  bignum_t *rn = NULL, *rd = NULL;

  assert(!BNZERO(dx) && !dx->isneg);

  if (!bntrysqrt(nx, &rn)) return 0;
  if (!bntrysqrt(dx, &rd)) { bnfree(rn); return 0; }

  /* if nx/dx are reduced, their sqrts are in lowest terms automatically */
  if (pnr) *pnr = rn; else bnfree(rn);
  if (pdr) *pdr = rd; else bnfree(rd);
  return 1;
}

/* bnrtrysqrt: exact rational sqrt of nx/dx (not necessarily reduced) */
int bnrtrysqrt(const bignum_t *nx, const bignum_t *dx,
               bignum_t **pnr, bignum_t **pdr)
{
  bignum_t *g, *nx_red, *dx_red;
  int res;

  assert(!BNZERO(dx) && !dx->isneg);

  g = bngcd(nx, dx);
  if (bncmp(g, bn1) == 0) {
    bnfree(g);
    return bnrtrysqrt_reduced(nx, dx, pnr, pdr);
  }

  nx_red = bndiv(nx, g);
  dx_red = bndiv(dx, g);
  bnfree(g);

  res = bnrtrysqrt_reduced(nx_red, dx_red, pnr, pdr);
  bnfree(nx_red);
  bnfree(dx_red);
  return res;
}

/* bnctrysqrt: exact principal complex square root of nrx/drx + i*nix/dix.
 * Handles purely imaginary and purely real cases separately.
 * All four output pointers are required. Outputs are in lowest terms. */
int bnctrysqrt(const bignum_t *nrx, const bignum_t *drx,
               const bignum_t *nix, const bignum_t *dix,
               bignum_t **pnrr, bignum_t **pdrr,
               bignum_t **pnir, bignum_t **pdir)
{
  bignum_t *g_den, *rd_prime, *id_prime;
  bignum_t *x, *y, *n_z2, *r_num;
  bignum_t *tmp1, *tmp2, *lcm_den, *a_num, *a_den;
  bignum_t *local_an = NULL, *local_ad = NULL;
  int is_sq;

  assert(pnrr != NULL && pdrr != NULL && pnir != NULL && pdir != NULL);
  assert(!BNZERO(drx) && !drx->isneg);
  assert(!BNZERO(dix) && !dix->isneg);

  /* Purely real case */
  if (BNZERO(nix)) {
    if (bnsign(nrx) >= 0) {
      if (bnrtrysqrt_reduced(nrx, drx, pnrr, pdrr)) {
        *pnir = bn0; *pdir = bn1;
        return 1;
      }
    } else {
      if (bnrtrysqrt_reduced(nrx, drx, pnir, pdir)) {
        *pnrr = bn0; *pdrr = bn1;
        return 1;
      }
    }
    return 0;
  }

  /* purely imaginary case: sqrt(i*y) = sqrt(|y|/2) * (1 + i*sign(y)) */
  if (BNZERO(nrx)) {
    bignum_t *half_num, *half_den, *r;
    int y_neg = nix->isneg;
    bignum_t *abs_nix = bnabs(nix);

    /* Compute |nix| / (2 * dix) */
    half_num = bndup(abs_nix);
    half_den = bnashll(dix, 1);
    bnfree(abs_nix);

    /* Reduce */
    {
      bignum_t *g = bngcd(half_num, half_den);
      if (!BNONE(g, 0)) {
        bignum_t *new_num = bndiv(half_num, g);
        bignum_t *new_den = bndiv(half_den, g);
        bnfree(half_num); bnfree(half_den);
        half_num = new_num; half_den = new_den;
      }
      bnfree(g);
    }

    if (!bnrtrysqrt_reduced(half_num, half_den, &r, &local_ad)) {
      bnfree(half_num); bnfree(half_den);
      return 0;
    }
    bnfree(half_num); bnfree(half_den);

    *pnrr = bndup(r);
    *pdrr = bndup(local_ad);
    if (y_neg) {
      *pnir = bnneg(r);
      bnfree(r);
    } else {
      *pnir = r;
    }
    *pdir = local_ad;
    return 1;
  }

  /* general complex case */
  if (IN_RZ2(drx, dix)) return 0;

  g_den = bngcd(drx, dix);
  rd_prime = bndiv(drx, g_den);
  id_prime = bndiv(dix, g_den);

  if (IN_RZ2(nrx, id_prime) || IN_RZ2(nix, rd_prime)) {
    bnfree(g_den); bnfree(rd_prime); bnfree(id_prime);
    return 0;
  }

  x = bnmul(nrx, id_prime);
  y = bnmul(nix, rd_prime);

  if (IN_RZ2(x, x) || IN_RZ2(y, y)) {
    bnfree(x); bnfree(y); bnfree(g_den); bnfree(rd_prime); bnfree(id_prime);
    return 0;
  }

  tmp1 = bnmul(x, x);
  tmp2 = bnmul(y, y);

  if (IN_RZ2(tmp1, tmp2)) {
    bnfree(x); bnfree(y); bnfree(tmp1); bnfree(tmp2);
    bnfree(g_den); bnfree(rd_prime); bnfree(id_prime);
    return 0;
  }

  n_z2 = bnadd(tmp1, tmp2);
  bnfree(tmp1); bnfree(tmp2);

  is_sq = bntrysqrt(n_z2, &r_num);
  bnfree(n_z2);
  if (!is_sq) {
    bnfree(x); bnfree(y); bnfree(g_den);
    bnfree(rd_prime); bnfree(id_prime);
    return 0;
  }

  tmp1 = bnmul(rd_prime, id_prime);
  lcm_den = bnmul(g_den, tmp1);
  bnfree(tmp1); bnfree(g_den); bnfree(rd_prime); bnfree(id_prime);

  a_num = bnadd(r_num, x);
  a_den = bnashll(lcm_den, 1);
  bnfree(r_num); bnfree(x); bnfree(y); bnfree(lcm_den);

  is_sq = bnrtrysqrt(a_num, a_den, &local_an, &local_ad);
  bnfree(a_num); bnfree(a_den);
  if (!is_sq) return 0;
  
  { /* compute imaginary part: b = nix / (2 * local_an/local_ad * dix) */
    bignum_t *nb_raw = bnmul(nix, local_ad);
    bignum_t *db_temp = bnmul(dix, local_an);
    bignum_t *db_raw = bnashll(db_temp, 1);
    bignum_t *g, *local_bn, *local_bd;
    bnfree(db_temp);

    g = bngcd(nb_raw, db_raw);
    local_bn = bndiv(nb_raw, g);
    local_bd = bndiv(db_raw, g);
    bnfree(nb_raw); bnfree(db_raw); bnfree(g);

    *pnir = local_bn;
    *pdir = local_bd;
  }

  *pnrr = local_an;
  *pdrr = local_ad;
  return 1;
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

/* [esl] end of sqrt bundles */

/* [esl+] exact power bundle */

/* Helper: count trailing zero bits in unsigned bignum */
static size_t bnx_trailing_zeros(const bignum_t *x)
{
  size_t tz = 0, i;
  if (BNZERO(x)) return 0;
  for (i = 0; i < x->size; i++) {
    if (x->limb[i] == 0) {
      tz += sizeof(limb_t) * 8;
    } else {
      limb_t l = x->limb[i];
      while ((l & 1) == 0) {
        tz++;
        l >>= 1;
      }
      break;
    }
  }
  return tz;
}

#define LOG2_3_NUM 15849625ULL
#define LOG2_3_DEN 10000000ULL

/* bniroot: exact integer nth root of x (sign-preserving for odd n).
 * |x| = root^n + rem for even n; x = root^n + rem for odd n.
 * No internal resource-limit checks; callers must enforce size limits. */
int bniroot(const bignum_t *x, size_t n, bignum_t **out_root, bignum_t **out_rem)
{
  bignum_t *r = NULL, *rn = NULL, *rem = NULL;
  bignum_t *x_abs_alloc = NULL;
  const bignum_t *abs_x;
  size_t l;
  int is_exact, neg;

  assert(n >= 1);
  assert(out_root != NULL);

  if (BNZERO(x)) { *out_root = bn0; if (out_rem) *out_rem = bn0; return 1; }
  if (BNONE(x, 0)) {
    *out_root = (x->isneg && (n & 1)) ? bnneg(bn1) : bn1;
    if (out_rem) *out_rem = bn0;
    return 1;
  }
  if (n == 1) { *out_root = bndup(x); if (out_rem) *out_rem = bn0; return 1; }
  if (n == 2) { bignum_t *xa = bnabs(x); int res = bnisqrt(xa, out_root, out_rem); bnfree(xa); return res; }
  /////////////////if (n == 3) return bnicbrt(x, out_root, out_rem);

  neg = x->isneg;
  abs_x = neg ? (x_abs_alloc = bnabs(x)) : x;

  if ((n & 1) == 0) {
    bignum_t *sqrt_x = NULL;
    bnisqrt(abs_x, &sqrt_x, NULL);
    bniroot(sqrt_x, n / 2, &r, NULL);
    bnfree(sqrt_x);

    rn = bnexptull(r, (unsigned long long)n);
    rem = bnsub(abs_x, rn);
    bnfree(rn);

    while (rem->isneg) {
      bignum_t *r_next = bnsub(r, bn1);
      bnfree(r); bnfree(rem);
      r = r_next;
      rn = bnexptull(r, (unsigned long long)n);
      rem = bnsub(abs_x, rn);
      bnfree(rn);
    }

    is_exact = BNZERO(rem);
    *out_root = r;
    if (out_rem) *out_rem = rem; else bnfree(rem);
    if (x_abs_alloc) bnfree(x_abs_alloc);
    return is_exact;
  }

  /* odd n >= 5 */
  l = bnwidthu(abs_x);
  if (l <= n) r = bndup(bn1);
  else if (l <= 2 * n) {
    if ((unsigned long long)l * LOG2_3_DEN < (unsigned long long)n * LOG2_3_NUM) r = bndup(bn2);
    else {
      bignum_t *three_n = bnexptull(bn3, (unsigned long long)n);
      r = (bncmpabs(abs_x, three_n) < 0) ? bndup(bn2) : bndup(bn3);
      bnfree(three_n);
    }
  } else {
    size_t m = (l - 1) / (2 * n);
    bignum_t *x_top = bnashll(abs_x, -(long long)(m * n));
    bignum_t *r_top = NULL, *g = NULL;
    bniroot(x_top, n, &r_top, NULL);
    bnfree(x_top);
    { bignum_t *r_top_inc = bnadd(r_top, bn1);
      g = bnashll(r_top_inc, (long long)m);
      bnfree(r_top); bnfree(r_top_inc);
    }
    for (;;) {
      bignum_t *gn1 = bnexptull(g, (unsigned long long)(n - 1));
      bignum_t *gn = bnmul(gn1, g);
      bignum_t *c = bnmulll(gn, (long)(n - 1));
      bignum_t *num = bnadd(abs_x, c);
      bignum_t *den = bnmulll(gn1, (long)n);
      bignum_t *d;
      bnfree(c); bnfree(gn); bnfree(gn1);
      d = bndiv(num, den);
      bnfree(num); bnfree(den);
      if (bncmp(d, g) >= 0) { bnfree(d); break; }
      bnfree(g); g = d;
    }
    r = g;
  }

  rn = bnexptull(r, (unsigned long long)n);
  rem = bnsub(abs_x, rn);
  bnfree(rn);

  while (rem->isneg) {
    bignum_t *r_next = bnsub(r, bn1);
    bnfree(r); bnfree(rem);
    r = r_next;
    rn = bnexptull(r, (unsigned long long)n);
    rem = bnsub(abs_x, rn);
    bnfree(rn);
  }

  is_exact = BNZERO(rem);
  if (neg) {
    *out_root = bnneg(r); bnfree(r);
    if (out_rem) { *out_rem = bnneg(rem); bnfree(rem); }
    else bnfree(rem);
  } else {
    *out_root = r;
    if (out_rem) *out_rem = rem; else bnfree(rem);
  }
  if (x_abs_alloc) bnfree(x_abs_alloc);
  return is_exact;
}

/* bntryroot: check if |x| has an exact nth root. Returns non-negative root. */
int bntryroot(const bignum_t *x, size_t n, bignum_t **pbn)
{
  size_t l, tz;
  bignum_t *x_abs_alloc = NULL;
  const bignum_t *abs_x;

  assert(x != NULL);
  assert(n >= 1);

  if (x->size >= BIGNUM_RZ_LIMBS) return 0;
  if (BNZERO(x)) { if (pbn) *pbn = bn0; return 1; }
  if (BNONE(x, 0)) { if (pbn) *pbn = bn1; return 1; }
  if (n == 1) { if (pbn) *pbn = bnabs(x); return 1; }

  l = bnwidthu(x);
  if (l <= n) return 0;

  abs_x = x->isneg ? (x_abs_alloc = bnabs(x)) : x;

  if ((n & 1) == 0) {
    if (!bnx_maybe_square(abs_x)) { if (x_abs_alloc) bnfree(x_abs_alloc); return 0; }
  }
  tz = bnx_trailing_zeros(abs_x);
  if (tz > 0 && (tz % n) != 0) { if (x_abs_alloc) bnfree(x_abs_alloc); return 0; }

  if (l <= 53) {
    double x_d = fabs(bntod_approx(x));
    double g_d = pow(x_d, 1.0 / (double)n);
    long long g_lo = (long long)floor(g_d), g_hi = g_lo + 1;
    if (g_lo >= 2) {
      bignumll_t gll;
      bignum_t *g_bn = bnx_makell(&gll, g_lo), *gn;
      if (IN_RZ_EXPT(g_bn, n)) { if (x_abs_alloc) bnfree(x_abs_alloc); return 0; }
      gn = bnexptull(g_bn, (unsigned long long)n);
      if (bncmpabs(gn, x) == 0) { bnfree(gn); if (pbn) *pbn = bndup(g_bn); if (x_abs_alloc) bnfree(x_abs_alloc); return 1; }
      bnfree(gn);
    }
    { bignumll_t gll;
      bignum_t *g_bn = bnx_makell(&gll, g_hi), *gn;
      if (IN_RZ_EXPT(g_bn, n)) { if (x_abs_alloc) bnfree(x_abs_alloc); return 0; }
      gn = bnexptull(g_bn, (unsigned long long)n);
      if (bncmpabs(gn, x) == 0) { bnfree(gn); if (pbn) *pbn = bndup(g_bn); if (x_abs_alloc) bnfree(x_abs_alloc); return 1; }
      bnfree(gn);
    }
    if (x_abs_alloc) bnfree(x_abs_alloc);
    return 0;
  }

  if (n >= 3 && (n & 1) != 0) {
    static const long small_primes[] = { 7, 11, 13, 17, 19, 23, 29, 31 };
    size_t i;
    for (i = 0; i < sizeof(small_primes)/sizeof(small_primes[0]); i++) {
      long p = small_primes[i];
      long r_mod = (long)bnx_mag_modl(x, p);
      if (r_mod == 0) continue;
      { long pm1 = p - 1, g = (long)(n % (size_t)pm1), t = pm1, exp, base, result = 1;
        while (t != 0) { long tmp = t; t = g % t; g = tmp; }
        exp = pm1 / g; base = r_mod;
        while (exp > 0) { if (exp & 1) result = (result * base) % p; base = (base * base) % p; exp >>= 1; }
        if (result != 1) { if (x_abs_alloc) bnfree(x_abs_alloc); return 0; }
      }
    }
  }

  { bignum_t *root = NULL, *rem = NULL;
    int is_exact = bniroot(abs_x, n, &root, &rem);
    if (is_exact) { if (pbn) *pbn = root; else bnfree(root); }
    else bnfree(root);
    bnfree(rem);
    if (x_abs_alloc) bnfree(x_abs_alloc);
    return is_exact;
  }
}

/* pairs of (prime, C) where C/1e9 > cos(pi/prime), certified by
 * C = ceil(cos(pi/prime) * 1e9) with +1 when cos is exact (n=3) */
static const long prime_bound_table[] = {
    3, 500000001L,    5, 809016995L,    7, 900968868L,   11, 959492974L,
   13, 970941818L,   17, 982973100L,   19, 986361304L,   23, 990685947L,
   29, 994137958L,   31, 994869324L,   37, 996397489L,   41, 997065802L,
   43, 997332284L,   47, 997766879L,   53, 998243732L,   59, 998582696L,
   61, 998674090L,   67, 998900892L,   71, 999021227L,   73, 999074116L,
   79, 999209398L,   83, 999283755L,   89, 999377063L,   97, 999475570L,
  101, 999516283L,  103, 999534884L,  107, 999569007L,  109, 999584677L,
  113, 999613558L,  127, 999694058L,  131, 999712455L,  137, 999737089L,
  139, 999744600L,  149, 999777730L,  151, 999783579L,  157, 999799804L,
  163, 999814271L,  167, 999823061L,  173, 999835121L,  179, 999845989L,
  181, 999849374L,  191, 999864733L,  193, 999867522L,  197, 999872847L,
  199, 999875390L,  211, 999889160L,  223, 999900768L,  227, 999904235L,
  229, 999905900L,  233, 999909103L,  239, 999913610L,  241, 999915038L,
  251, 999921673L,  257, 999925287L,  263, 999928657L,  269, 999931804L,
  271, 999932807L,  277, 999935686L,  281, 999937504L,  283, 999938385L,
  293, 999942519L,  307, 999947642L,  311, 999948980L,  313, 999949630L,
  317, 999950893L,  331, 999954959L,  337, 999956549L,  347, 999959017L,
  349, 999959485L,  353, 999960398L,  359, 999961711L,  367, 999963362L,
  373, 999964531L,  379, 999965646L,  383, 999966359L,  389, 999967389L,
  397, 999968690L,  401, 999969312L,  409, 999970501L,  419, 999971892L,
  421, 999972158L,  431, 999973435L,  433, 999973680L,  439, 999974395L,
  443, 999974855L,  449, 999975523L,  457, 999976372L,  461, 999976780L,
  463, 999976980L,  467, 999977373L,  479, 999978493L,  487, 999979193L,
  491, 999979531L,  499, 999980182L,  503, 999980496L,  509, 999980953L,
  521, 999981821L,  523, 999981959L,  541, 999983140L,  547, 999983508L,
  557, 999984095L,  563, 999984432L,  569, 999984758L,  571, 999984865L,
  577, 999985178L,  587, 999985679L,  593, 999985967L,  599, 999986247L,
  601, 999986338L,  607, 999986607L,  613, 999986868L,  617, 999987038L,
  619, 999987121L,  631, 999987607L,  641, 999987990L,  643, 999988065L,
  647, 999988212L,  653, 999988428L,  659, 999988637L,  661, 999988706L,
  673, 999989105L,  677, 999989234L,  683, 999989422L,  691, 999989665L,
  701, 999989958L,  709, 999990184L,  719, 999990455L,  727, 999990664L,
  733, 999990816L,  739, 999990964L,  743, 999991061L,  751, 999991251L,
  757, 999991389L,  761, 999991479L,  769, 999991656L,  773, 999991742L,
  787, 999992033L,  797, 999992232L,  809, 999992460L,  811, 999992498L,
  821, 999992679L,  823, 999992715L,  827, 999992785L,  829, 999992820L,
  839, 999992990L,  853, 999993218L,  857, 999993281L,  859, 999993313L,
  863, 999993375L,  877, 999993584L,  881, 999993643L,  883, 999993671L,
  887, 999993728L,  907, 999994002L,  911, 999994054L,  919, 999994157L,
  929, 999994283L,  937, 999994380L,  941, 999994427L,  947, 999994498L,
  953, 999994567L,  967, 999994723L,  971, 999994767L,  977, 999994831L,
  983, 999994894L,  991, 999994976L,  997, 999995036L
};

#define NUM_SUPPORTED_PRIMES (sizeof(prime_bound_table) / (2 * sizeof(prime_bound_table[0])))

/* compurte Chebyshev polynomial P_n(p, u) via recurrence
 * P_0 = 1, P_1 = p, P_{k+1} = 2p*P_k - u*P_{k-1} */
static bignum_t *chebyshev_pn(const bignum_t *p, const bignum_t *u, unsigned long long n)
{
  bignum_t *prev2, *prev1, *cur;
  unsigned long long k;

  if (n == 0) return bndup(bn1);
  if (n == 1) return bndup(p);

  prev2 = bndup(bn1);
  prev1 = bndup(p);

  for (k = 2; k <= n; k++) {
    bignum_t *t1, *t2, *t3;
    if (IN_RZ2(prev1, p) || IN_RZ2(u, prev2)) {
      bnfree(prev2); bnfree(prev1);
      return NULL;
    }
    t1 = bnmul(p, prev1);
    t2 = bnashll(t1, 1);
    t3 = bnmul(u, prev2);
    cur = bnsub(t2, t3);
    bnfree(t1); bnfree(t2); bnfree(t3);
    bnfree(prev2);
    prev2 = prev1;
    prev1 = cur;
  }
  bnfree(prev2);
  return prev1;
}

/* compute Im((p + is)^n) via complex recurrence with size guard
 * P_0=1, Q_0=0; P_{k+1} = p*P_k - s*Q_k; Q_{k+1} = s*P_k + p*Q_k;
 * Returns Q_n or NULL on size overflow; sets *pprincipal = 1 iff 
 * the root p+is is in the principal sector */
static bignum_t *complex_pow_imag_chk(const bignum_t *p, const bignum_t *s, unsigned long long n, int *pprincipal)
{
  bignum_t *P, *Q;
  unsigned long long k;
  int s_is_zero = BNZERO(s);

  *pprincipal = 1;
  P = bndup(bn1);
  Q = bndup(bn0);

  for (k = 1; k <= n; k++) {
    bignum_t *pP, *sQ, *sP, *pQ, *newP, *newQ;

    if (IN_RZ2(p, P) || IN_RZ2(s, Q) || IN_RZ2(s, P) || IN_RZ2(p, Q)) {
      bnfree(P); bnfree(Q);
      *pprincipal = 0;
      return NULL;
    }

    pP = bnmul(p, P);
    sQ = bnmul(s, Q);
    sP = bnmul(s, P);
    pQ = bnmul(p, Q);
    newP = bnsub(pP, sQ);
    newQ = bnadd(sP, pQ);
    bnfree(pP); bnfree(sQ); bnfree(sP); bnfree(pQ);
    bnfree(P); bnfree(Q);
    P = newP;
    Q = newQ;

    /* Check principality: Q_k > 0 for all k=1..n (if s > 0) */
    if (!s_is_zero && (Q->isneg || BNZERO(Q))) {
      *pprincipal = 0;
    }
  }
  bnfree(P);
  return Q;
}


/* compute certified lower bound for principal sector
 * Returns mb_n = ceil(C * sqrt(u) / 1e9) where C/1e9 > cos(pi/n) */
static bignum_t *compute_principal_bound(const bignum_t *u, unsigned long long n)
{
  long C = 0;
  bignum_t *t, *rad, *root, *rem, *num, *mb;
  bignumll_t ll;
  bignum_t *D;
  size_t i;

  /* Linear search for the prime in the unified table */
  for (i = 0; i < NUM_SUPPORTED_PRIMES; i++) {
    if ((unsigned long long)prime_bound_table[2 * i] == n) {
      C = prime_bound_table[2 * i + 1];
      break;
    }
  }
  if (C == 0) return NULL;  /* unsupported prime */

  /* Conservative RZ guard for u * C^2 */
  if (IN_RZ2(u, u)) return NULL;

  /* rad = u * C^2 */
  t = bnmulll(u, C);
  rad = bnmulll(t, C);
  bnfree(t);

  /* root = floor(sqrt(u * C^2)) = floor(C * sqrt(u)) */
  bnisqrt(rad, &root, &rem);
  bnfree(rad);

  /* If C*sqrt(u) was not integral, use its ceiling */
  if (!BNZERO(rem)) {
    t = bnaddll(root, 1);
    bnfree(root);
    root = t;
  }
  bnfree(rem);

  /* mb = ceil(root / 1e9) */
  num = bnaddll(root, 999999999L);
  bnfree(root);

  D = bnx_makell(&ll, 1000000000L);
  mb = bndiv(num, D);
  bnfree(num);

  return mb;
}

/* generalized prime root solver; returns 1 and fills output params or 0 on failure */
static int bnctryprimeroot(unsigned long long n, /* n is prime >= 3 */
  const bignum_t *nrx, const bignum_t *drx, const bignum_t *nix, const bignum_t *dix,
  bignum_t **pnrr, bignum_t **pdrr, bignum_t **pnir, bignum_t **pdir)
{
  bignum_t *g_den, *L, *d, *Ln1;
  bignum_t *scale_r, *scale_i;
  bignum_t *X0, *Y0, *X, *Y;
  bignum_t *X2, *Y2, *norm, *u;
  bignum_t *p = NULL, *p2 = NULL, *s2 = NULL, *s = NULL;
  bignum_t *max_p, *mb_n;
  int found = 0, negate_s = 0, is_principal = 0;

  assert(pnrr != NULL && pdrr != NULL && pnir != NULL && pdir != NULL);
  assert(!BNZERO(drx) && !drx->isneg);
  assert(!BNZERO(dix) && !dix->isneg);
  assert(n >= 3 && (n & 1) != 0);

  if (BNZERO(nrx) && BNZERO(nix)) return 0;

  /* Step 1: Scale to Gaussian integer over L^n */
  if (IN_RZ2(drx, dix)) return 0;
  g_den = bngcd(drx, dix);
  { bignum_t *prod = bnmul(drx, dix);
    L = bndiv(prod, g_den);
    bnfree(prod);
  }
  bnfree(g_den);

  d = bndup(L);
  if (IN_RZ2(L, L)) { bnfree(L); bnfree(d); return 0; }

  scale_r = bndiv(L, drx);
  scale_i = bndiv(L, dix);

  if (IN_RZ2(nrx, scale_r) || IN_RZ2(nix, scale_i)) {
    bnfree(scale_r); bnfree(scale_i); bnfree(L); bnfree(d);
    return 0;
  }

  X0 = bnmul(nrx, scale_r);
  Y0 = bnmul(nix, scale_i);
  bnfree(scale_r); bnfree(scale_i); bnfree(L);

  if (d->size > 0 && (n - 1) > (BIGNUM_RZ_LIMBS - 1) / d->size) {
    bnfree(X0); bnfree(Y0); bnfree(d);
    return 0;
  }
  Ln1 = bnexptull(d, n - 1);
  if (IN_RZ2(X0, Ln1) || IN_RZ2(Y0, Ln1)) {
    bnfree(X0); bnfree(Y0); bnfree(Ln1); bnfree(d);
    return 0;
  }
  X = bnmul(X0, Ln1);
  Y = bnmul(Y0, Ln1);
  bnfree(X0); bnfree(Y0); bnfree(Ln1);

  if (IN_RZ2(X, X) || IN_RZ2(Y, Y)) {
    bnfree(X); bnfree(Y); bnfree(d);
    return 0;
  }
  X2 = bnmul(X, X);
  Y2 = bnmul(Y, Y);
  norm = bnadd(X2, Y2);
  bnfree(X2); bnfree(Y2);

  /* Step 2: |z|^2 must be a perfect nth power */
  if (!bntryroot(norm, (size_t)n, &u)) {
    bnfree(norm); bnfree(X); bnfree(Y); bnfree(d);
    return 0;
  }
  bnfree(norm);

  max_p = NULL;
  bnisqrt(u, &max_p, NULL);

  mb_n = compute_principal_bound(u, n);
  if (!mb_n) {
    bnfree(max_p);
    bnfree(d); bnfree(u); bnfree(X); bnfree(Y);
    return 0;
  }

  /* Step 3: Binary search for p in [mb_n, max_p]
   * P_n(p, u) is strictly increasing on this interval, so binary search is valid. */
  {
    bignum_t *lo = bndup(mb_n);
    bignum_t *hi = bndup(max_p);

    while (bncmp(lo, hi) <= 0) {
      bignum_t *sum = bnadd(lo, hi);
      bignum_t *mid = bnashll(sum, -1);
      bignum_t *pn_val;
      int cmp;
      bnfree(sum);

      pn_val = chebyshev_pn(mid, u, n);
      if (!pn_val) {
        /* Size overflow during evaluation   bail */
        bnfree(mid);
        bnfree(lo); bnfree(hi);
        goto cleanup_early;
      }

      cmp = bncmp(pn_val, X);
      bnfree(pn_val);

      if (cmp == 0) {
        /* Found the unique p. Transfer ownership of mid to p. */
        p = mid;
        bnfree(lo); bnfree(hi);
        goto verify;
      } else if (cmp < 0) {
        /* P_n(mid) < X, search right half */
        bignum_t *next = bnadd(mid, bn1);
        bnfree(mid);
        bnfree(lo);
        lo = next;
      } else {
        /* P_n(mid) > X, search left half */
        bignum_t *next = bnsub(mid, bn1);
        bnfree(mid);
        bnfree(hi);
        hi = next;
      }
    }

    /* No match found in the interval */
    bnfree(lo); bnfree(hi);
  }

  /* Cleanup and return failure */
  bnfree(mb_n); bnfree(max_p);
  bnfree(d); bnfree(u); bnfree(X); bnfree(Y);
  return 0;

verify:
  bnfree(mb_n); bnfree(max_p);

  /* Step 4 & 5: Verify imaginary part and principality */
  p2 = bnmul(p, p);
  s2 = bnsub(u, p2);

  if (!s2->isneg && bnisqrt(s2, &s, NULL)) {
    bignum_t *im_val = complex_pow_imag_chk(p, s, n, &is_principal);

    if (!im_val) {
      bnfree(s); s = NULL;
      found = 0;
    } else if (bncmp(im_val, Y) == 0) {
      found = 1;
      negate_s = 0;
      bnfree(im_val);
    } else {
      bignum_t *neg_im = bnneg(im_val);
      if (bncmp(neg_im, Y) == 0) {
        found = 1;
        negate_s = 1;
      }
      bnfree(neg_im);
      bnfree(im_val);
    }
  }

  /* Principal check: p > 0 and is_principal */
  if (found) {
    if (p->isneg || BNZERO(p)) {
      found = 0;
    } else if (!is_principal) {
      found = 0;
    }
  }

  bnfree(s2); bnfree(p2);
  if (!found) {
    bnfree(p); p = NULL;
    if (s) { bnfree(s); s = NULL; }
  }

  /* Step 6: Reduce and output */
  if (found && p && s) {
    bignum_t *gp = bngcd(p, d);
    bignum_t *p_out = bndiv(p, gp);
    bignum_t *pd_out = bndiv(d, gp);
    bignum_t *gs = bngcd(s, d);
    bignum_t *s_out = bndiv(s, gs);
    bignum_t *sd_out = bndiv(d, gs);
    bnfree(gp); bnfree(gs);

    *pnrr = p_out;
    *pdrr = pd_out;
    if (negate_s) { *pnir = bnneg(s_out); bnfree(s_out); }
    else *pnir = s_out;
    *pdir = sd_out;

    bnfree(p); bnfree(s); bnfree(d);
  } else {
    if (p) bnfree(p);
    if (s) bnfree(s);
    bnfree(d);
  }

  bnfree(u); bnfree(X); bnfree(Y);
  return found;

cleanup_early:
  bnfree(mb_n); bnfree(max_p);
  if (p) bnfree(p);
  if (s) bnfree(s);
  bnfree(d); bnfree(u); bnfree(X); bnfree(Y);
  return 0;
}

/* saturating cost model naturally limits feasibility for large primes or large inputs */
static unsigned long long compute_root_cost(unsigned long long pr, size_t limbs, size_t bits)
{
  unsigned long long ul_limbs = (unsigned long long)limbs;
  unsigned long long factor1, factor2;

  if (pr != 0 && ul_limbs > ULLONG_MAX / pr)
    return ULLONG_MAX;
  factor1 = pr * ul_limbs;

  if ((unsigned long long)bits > ULLONG_MAX - pr)
    return ULLONG_MAX;
  factor2 = pr + (unsigned long long)bits;

  if (factor1 != 0 && factor2 > ULLONG_MAX / factor1)
    return ULLONG_MAX;

  return factor1 * factor2;
}
static unsigned long long compute_root_budget(size_t limbs)
{
  unsigned long long ul_limbs = (unsigned long long)limbs;
  unsigned long long max_limbs = (unsigned long long)BIGNUM_MAX_LIMBS;
  unsigned long long base;

  if (ul_limbs != 0 && max_limbs > ULLONG_MAX / ul_limbs)
    return ULLONG_MAX;
  base = ul_limbs * max_limbs;

  if (base != 0 && 100ULL > ULLONG_MAX / base)
    return ULLONG_MAX;

  return base * 100ULL;
}

/* odd root driver: factor m into primes, chain solvers with cost budget */
static int bnctryoddroot(const bignum_t *nrx, const bignum_t *drx, const bignum_t *nix, const bignum_t *dix,
  unsigned long long m, bignum_t **pnrr, bignum_t **pdrr, bignum_t **pnir, bignum_t **pdir)
{
  unsigned long long temp_m = m;
  unsigned long long cost_budget;
  size_t cur_limbs, i;
  bignum_t *cur_rn, *cur_rd, *cur_in, *cur_id;
  bignum_t *next_rn, *next_rd, *next_in, *next_id;

  assert(pnrr != NULL && pdrr != NULL && pnir != NULL && pdir != NULL);
  assert(m & 1); /* not to be called with even m */

  if (m == 1) {
    *pnrr = bndup(nrx); *pdrr = bndup(drx);
    *pnir = bndup(nix); *pdir = bndup(dix);
    return 1;
  }

  cur_limbs = nrx->size + drx->size + nix->size + dix->size;
  cost_budget = compute_root_budget(cur_limbs);

  cur_rn = bndup(nrx); cur_rd = bndup(drx);
  cur_in = bndup(nix); cur_id = bndup(dix);

  /* Iterate through supported primes from the unified table */
  for (i = 0; i < NUM_SUPPORTED_PRIMES; i++) {
    unsigned long long pr = (unsigned long long)prime_bound_table[2 * i];
    int ok;

    while (temp_m % pr == 0) {
      unsigned long long cost;
      size_t approx_bits;

      /* Estimate cost using actual bit width with overflow protection */
      cur_limbs = cur_rn->size + cur_rd->size + cur_in->size + cur_id->size;
      approx_bits = bnwidthu(cur_rn) + bnwidthu(cur_rd) +
                    bnwidthu(cur_in) + bnwidthu(cur_id);
      cost = compute_root_cost(pr, cur_limbs, approx_bits);
      if (cost > cost_budget) {
        bnfree(cur_rn); bnfree(cur_rd);
        bnfree(cur_in); bnfree(cur_id);
        return 0;
      }
      cost_budget -= cost;

      /* Dispatch: cube root has a specialized fast path */
      ////if (pr == 3)
      ////  ok = bnctryoddroot_cbrt(cur_rn, cur_rd, cur_in, cur_id, &next_rn, &next_rd, &next_in, &next_id);
      ////else 
        ok = bnctryprimeroot(pr, cur_rn, cur_rd, cur_in, cur_id, &next_rn, &next_rd, &next_in, &next_id);

      bnfree(cur_rn); bnfree(cur_rd);
      bnfree(cur_in); bnfree(cur_id);
      if (!ok) return 0;

      cur_rn = next_rn; cur_rd = next_rd;
      cur_in = next_in; cur_id = next_id;
      temp_m /= pr;
    }
    if (temp_m == 1) break;
  }

  if (temp_m != 1) {
    /* Unsupported prime factor (not in table) */
    bnfree(cur_rn); bnfree(cur_rd);
    bnfree(cur_in); bnfree(cur_id);
    return 0;
  }

  *pnrr = cur_rn; *pdrr = cur_rd;
  *pnir = cur_in; *pdir = cur_id;
  return 1;
}

/* quick O(1) filter to check if x might be an exact nth power */
static int bnx_maybe_nth_power(const bignum_t *x, size_t n)
{
  size_t tz, l;

  if (n == 1 || BNZERO(x) || BNONE(x, 0)) return 1;
  /* -1 is an nth power iff n is odd: (-1)^n = -1 for odd n */
  if (BNONE(x, 1)) return (n & 1) != 0;

  /* Negative numbers cannot be even powers (in reals) */
  if (x->isneg && (n & 1) == 0) return 0;

  /* If bit length <= n, and |x| > 1, it cannot be an nth power of an integer >= 2 */
  l = bnwidthu(x);
  if (l <= n) return 0;

  /* trailing zeros must be a multiple of n */
  tz = bnx_trailing_zeros(x);
  if (tz > 0 && (tz % n) != 0) return 0;

  /* even n: must be a perfect square */
  if ((n & 1) == 0) { if (!bnx_maybe_square(x)) return 0; }

  /* small-prime modular checks (valid for all n >= 2, not just odd n).
   * catches cases like n=4, x=784 (a square but not a 4th power) */
  if (n >= 2) {
    static const long small_primes[] = { 7, 11, 13, 17, 19, 23, 29, 31 };
    size_t i;
    for (i = 0; i < sizeof(small_primes)/sizeof(small_primes[0]); i++) {
      long p = small_primes[i], r_mod = (long)bnx_mag_modl(x, p);
      long pm1, r, a1, b1, exp; unsigned long ubase, ures, up; 
      if (r_mod == 0) continue;
      pm1 = p - 1;
      /* compute gcd(n, p-1) cleanly with separate variables */
      r = (long)(n % (size_t)pm1);
      a1 = pm1; b1 = r;
      while (b1) { long t = a1 % b1; a1 = b1; b1 = t; }
      /* a1 = gcd(n, p-1) */
      exp = pm1 / a1;
      /* use unsigned long to prevent overflow in base*base for larger primes */
      ubase = (unsigned long)r_mod;
      ures = 1; up = (unsigned long)p;
      while (exp > 0) {
        if (exp & 1) ures = (ures * ubase) % up;
        ubase = (ubase * ubase) % up;
        exp >>= 1;
      }

      if (ures != 1) return 0;
    }
  }

  return 1;
}


/* try to calculate the exact principal power of a real rational.
 *
 * for positive base: returns the positive real root if it exists;
 * for negative base: returns the principal Gaussian rational root
 * if one exists, 0 otherwise. By the principal convention, this
 * only exists for b in {1, 2, 4} with appropriate a mod 2b.
 *
 * preconditions:
 *   - nx/dx in lowest terms, dx > 0
 *   - nn/dn in lowest terms, dn > 0
 *   - nn and dn each fit in a single limb
 *
 * postconditions:
 *  on success, all output fractions are in lowest terms with
 *  positive denominators. */
int bnrtrypow(const bignum_t *nx, const bignum_t *dx, const bignum_t *nn, const bignum_t *dn,
  bignum_t **pnrr, bignum_t **pdrr, bignum_t **pnir, bignum_t **pdir)
{
  unsigned long long a, b; int x_neg;
  bignum_t *abs_nx, *pb, *qb, *root_p, *root_q;

  assert(pnrr != NULL && pdrr != NULL && pnir != NULL && pdir != NULL);
  assert(!BNZERO(dx) && !dx->isneg);
  assert(!BNZERO(nn) && !nn->isneg);
  assert(!BNZERO(dn) && !dn->isneg);

  if (nn->size != 1 || dn->size != 1) return 0;

  a = (unsigned long long)nn->limb[0];
  b = (unsigned long long)dn->limb[0];

  x_neg = nx->isneg;

  if (BNZERO(nx)) {
    *pnrr = bn0;  *pdrr = bn1;
    *pnir = bn0;  *pdir = bn1;
    return 1;
  }

  if (a == b) {
    *pnrr = bndup(nx);  *pdrr = bndup(dx);
    *pnir = bn0;        *pdir = bn1;
    return 1;
  }

  /* for x^(a/b) to be rational (with gcd(a,b)=1), x must be a perfect b-th power;
   * we check this before computing x^a, avoiding massive allocations, since
   * bnx_maybe_nth_power is an O(1) filter that rejects non-powers instantly */
  if (b > 1) {
    if (!bnx_maybe_nth_power(nx, (size_t)b) || !bnx_maybe_nth_power(dx, (size_t)b)) {
      return 0;
    }
  }

  if (BNONE(nx, 0) && BNONE(dx, 0)) {
    /* |x| = 1, handle signs for principal root */
    if (x_neg) {
      /* Principal value of (-1)^(a/b) = e^{i pi a/b} */
      if ((a & 1) == 0) {
        /* Even a: e^{i pi a/b}. For Gaussian rational, need b | a, but gcd(a,b)=1 -> b=1 */
        if (b == 1) {
          *pnrr = bn1;  *pdrr = bn1;
          *pnir = bn0;  *pdir = bn1;
          return 1;
        }
        return 0;
      }
      /* Odd a: direction depends on a mod 2b */
      if (b == 1) {
        *pnrr = bnneg(bn1);  *pdrr = bn1;
        *pnir = bn0;         *pdir = bn1;
        return 1;
      }
      if (b == 2) {
        /* a mod 4: 1 -> +i, 3 -> -i */
        *pnrr = bn0;  *pdrr = bn1;
        if ((a & 3) == 1) {
          *pnir = bn1;  *pdir = bn1;
        } else {
          *pnir = bnneg(bn1);  *pdir = bn1;
        }
        return 1;
      }
      if (b == 4) {
        /* a mod 8: 1 -> (1+i)/sqrt(2)... wait, |x|=1 so r=1 */
        /* Principal 4th roots of -1: e^{i pi a/4} */
        /* a=1: e^{i pi/4} = (1+i)/sqrt(2) - not Gaussian rational */
        /* Actually for |x|=1, the magnitude is 1, so we need the direction to be Gaussian rational */
        /* e^{i pi/4} is not Gaussian rational (needs sqrt(2)) */
        return 0;
      }
      return 0;
    }
    /* Positive 1 */
    *pnrr = bn1;  *pdrr = bn1;
    *pnir = bn0;  *pdir = bn1;
    return 1;
  }

  abs_nx = bnabs(nx);

  if (a > 1) {
    if (IN_RZ_EXPT(abs_nx, a) || IN_RZ_EXPT(dx, a)) {
      bnfree(abs_nx);
      return 0;
    }
  }

  if (a == 1) {
    pb = bndup(abs_nx);
    qb = bndup(dx);
  } else {
    pb = bnexptull(abs_nx, a);
    qb = bnexptull(dx, a);
  }
  bnfree(abs_nx);

  if (!x_neg) {
    /* Positive base: positive real principal root */
    if (b == 1) {
      root_p = pb;
      root_q = qb;
    } else {
      if (!bntryroot(pb, (size_t)b, &root_p)) {
        bnfree(pb); bnfree(qb);
        return 0;
      }
      bnfree(pb);
      if (!bntryroot(qb, (size_t)b, &root_q)) {
        bnfree(root_p); bnfree(qb);
        return 0;
      }
      bnfree(qb);
    }

    bnreduce(&root_p, &root_q);
    *pnrr = root_p;  *pdrr = root_q;
    *pnir = bn0;     *pdir = bn1;
    return 1;
  }

  /* Negative base: principal Gaussian rational root */
  /* Principal arg = pi*a/b. For Gaussian rational, need tan(pi*a/b) in {0, +/-1, +/-inf} */
  /* This requires a/b mod 2 in {0, 1/4, 1/2, 3/4, 1, 5/4, 3/2, 7/4} */
  /* With gcd(a,b)=1, this only works for b in {1, 2, 4} */

  if ((a & 1) == 0) {
    /* Even a, negative base: principal arg = pi*a/b */
    /* For Gaussian rational with gcd(a,b)=1 and a even, b must be odd */
    /* The only odd b that works is b=1 */
    if (b != 1) {
      bnfree(pb); bnfree(qb);
      return 0;
    }
    /* b=1: result is |x|^a (positive real) */
    root_p = pb;
    root_q = qb;
    bnreduce(&root_p, &root_q);
    *pnrr = root_p;  *pdrr = root_q;
    *pnir = bn0;     *pdir = bn1;
    return 1;
  }

  /* Odd a, negative base */
  if (b == 1) {
    /* Principal arg = pi, result is -|x|^a */
    if (!bntryroot(pb, 1, &root_p)) {
      bnfree(pb); bnfree(qb);
      return 0;
    }
    bnfree(pb);
    if (!bntryroot(qb, 1, &root_q)) {
      bnfree(root_p); bnfree(qb);
      return 0;
    }
    bnfree(qb);

    bnreduce(&root_p, &root_q);
    *pnrr = bnneg(root_p);
    *pdrr = root_q;
    *pnir = bn0;
    *pdir = bn1;
    bnfree(root_p);
    return 1;
  }

  if (b == 2) {
    /* Principal arg = pi*a/2. a mod 4: 1 -> pi/2 (+i), 3 -> 3pi/2 (-i) */
    bignum_t *r_p = NULL, *r_q = NULL;
    int imag_neg;

    if (!bntryroot(pb, 2, &r_p)) {
      bnfree(pb); bnfree(qb);
      return 0;
    }
    bnfree(pb);
    if (!bntryroot(qb, 2, &r_q)) {
      bnfree(r_p); bnfree(qb);
      return 0;
    }
    bnfree(qb);

    bnreduce(&r_p, &r_q);
    imag_neg = ((a & 3) == 3);

    *pnrr = bn0;  *pdrr = bn1;
    if (imag_neg) {
      *pnir = bnneg(r_p);
      bnfree(r_p);
    } else {
      *pnir = r_p;
    }
    *pdir = r_q;
    return 1;
  }

  if (b == 4) {
    /* Principal arg = pi*a/4. a mod 8 determines quadrant */
    /* a=1: pi/4 -> (1+i)/sqrt(2) * r where r^4 = |x|^a/4 */
    /* a=3: 3pi/4 -> (-1+i)/sqrt(2) * r */
    /* a=5: 5pi/4 -> (-1-i)/sqrt(2) * r */
    /* a=7: 7pi/4 -> (1-i)/sqrt(2) * r */
    bignum_t *num, *den, *red_num, *red_den;
    bignum_t *r_p = NULL, *r_q = NULL;
    int re_neg, im_neg;

    if (IN_RZ_SHIFT(qb, (long long)(b / 2))) {
      bnfree(pb); bnfree(qb);
      return 0;
    }

    den = bnashll(qb, (long long)(b / 2));
    num = pb;
    bnfree(qb);

    {
      bignum_t *g = bngcd(num, den);
      red_num = bndiv(num, g);
      red_den = bndiv(den, g);
      bnfree(num); bnfree(den); bnfree(g);
    }

    if (!bntryroot(red_num, 4, &r_p)) {
      bnfree(red_num); bnfree(red_den);
      return 0;
    }
    bnfree(red_num);
    if (!bntryroot(red_den, 4, &r_q)) {
      bnfree(r_p); bnfree(red_den);
      return 0;
    }
    bnfree(red_den);

    bnreduce(&r_p, &r_q);

    /* Select signs based on a mod 8 */
    switch (a & 7) {
      case 1: re_neg = 0; im_neg = 0; break;
      case 3: re_neg = 1; im_neg = 0; break;
      case 5: re_neg = 1; im_neg = 1; break;
      case 7: re_neg = 0; im_neg = 1; break;
      default:
        bnfree(r_p); bnfree(r_q);
        return 0;
    }

    *pnrr = re_neg ? bnneg(r_p) : bndup(r_p);
    *pdrr = bndup(r_q);
    *pnir = im_neg ? bnneg(r_p) : bndup(r_p);
    *pdir = bndup(r_q);
    bnfree(r_p); bnfree(r_q);
    return 1;
  }

  /* b not in {1, 2, 4}: no principal Gaussian rational root */
  bnfree(pb); bnfree(qb);
  return 0;
}


/* multiply two complex rationals returns 0 on bignums getting too big */
static int bnx_cmul(const bignum_t *ar, const bignum_t *ad, const bignum_t *ai, const bignum_t *ae,
  const bignum_t *br, const bignum_t *bd, const bignum_t *bi, const bignum_t *be,
  bignum_t **cr, bignum_t **cd, bignum_t **ci, bignum_t **ce)
{
  bignum_t *rr, *ri, *ir, *ii;
  bignum_t *rr_n, *ri_n, *ir_n, *ii_n;
  bignum_t *real_num, *real_den, *imag_num, *imag_den;

  if (IN_RZ4(ar, ad, ai, ae) || IN_RZ4(br, bd, bi, be)) return 0;

  rr = bnmul(ar, br);   rr_n = bnmul(ad, bd);
  ii = bnmul(ai, bi);   ii_n = bnmul(ae, be);
  ri = bnmul(ar, bi);   ri_n = bnmul(ad, be);
  ir = bnmul(ai, br);   ir_n = bnmul(ae, bd);

  {
    bignum_t *t1 = bnmul(rr, ii_n);
    bignum_t *t2 = bnmul(ii, rr_n);
    real_num = bnsub(t1, t2);
    real_den = bnmul(rr_n, ii_n);
    bnfree(t1); bnfree(t2);
  }

  {
    bignum_t *t1 = bnmul(ri, ir_n);
    bignum_t *t2 = bnmul(ir, ri_n);
    imag_num = bnadd(t1, t2);
    imag_den = bnmul(ri_n, ir_n);
    bnfree(t1); bnfree(t2);
  }

  bnfree(rr); bnfree(rr_n);
  bnfree(ii); bnfree(ii_n);
  bnfree(ri); bnfree(ri_n);
  bnfree(ir); bnfree(ir_n);

  bnreduce(&real_num, &real_den);
  bnreduce(&imag_num, &imag_den);

  if (real_den->isneg) {
    bignum_t *t1 = bnneg(real_num);
    bignum_t *t2 = bnneg(real_den);
    bnfree(real_num); bnfree(real_den);
    real_num = t1; real_den = t2;
  }
  if (imag_den->isneg) {
    bignum_t *t1 = bnneg(imag_num);
    bignum_t *t2 = bnneg(imag_den);
    bnfree(imag_num); bnfree(imag_den);
    imag_num = t1; imag_den = t2;
  }

  *cr = real_num; *cd = real_den;
  *ci = imag_num; *ce = imag_den;
  return 1;
}

/* compute x^n for complex rational x, integer n >= 0; returns 0 on bignums getting too big */
static int bnx_cpow(const bignum_t *xr, const bignum_t *xd, const bignum_t *xi, const bignum_t *xe,
  unsigned long long n, bignum_t **yr, bignum_t **yd, bignum_t **yi, bignum_t **ye)
{
  bignum_t *rr, *rd, *ri, *re;
  bignum_t *br, *bd, *bi, *be;
  bignum_t *tr, *td, *ti, *te;

  if (n == 0) {
    *yr = bn1; *yd = bn1;
    *yi = bn0; *ye = bn1;
    return 1;
  }

  if (n == 1) {
    *yr = bndup(xr); *yd = bndup(xd);
    *yi = bndup(xi); *ye = bndup(xe);
    return 1;
  }

  if (IN_RZ4(xr, xd, xi, xe)) return 0;

  rr = bn1; rd = bn1;
  ri = bn0; re = bn1;

  br = bndup(xr); bd = bndup(xd);
  bi = bndup(xi); be = bndup(xe);

  while (n > 0) {
    if (IN_RZ_LOOP2(br, bd) || IN_RZ_LOOP2(bi, be)) {
      bnfree(br); bnfree(bd); bnfree(bi); bnfree(be);
      if (rr != bn1) bnfree(rr);
      if (rd != bn1) bnfree(rd);
      if (ri != bn0) bnfree(ri);
      if (re != bn1) bnfree(re);
      return 0;
    }

    if (n & 1) {
      if (!bnx_cmul(rr, rd, ri, re, br, bd, bi, be,
                    &tr, &td, &ti, &te)) {
        bnfree(br); bnfree(bd); bnfree(bi); bnfree(be);
        if (rr != bn1) bnfree(rr);
        if (rd != bn1) bnfree(rd);
        if (ri != bn0) bnfree(ri);
        if (re != bn1) bnfree(re);
        return 0;
      }
      if (rr != bn1) bnfree(rr);
      if (rd != bn1) bnfree(rd);
      if (ri != bn0) bnfree(ri);
      if (re != bn1) bnfree(re);
      rr = tr; rd = td; ri = ti; re = te;
    }

    n >>= 1;
    if (n > 0) {
      if (!bnx_cmul(br, bd, bi, be, br, bd, bi, be,
                    &tr, &td, &ti, &te)) {
        bnfree(br); bnfree(bd); bnfree(bi); bnfree(be);
        if (rr != bn1) bnfree(rr);
        if (rd != bn1) bnfree(rd);
        if (ri != bn0) bnfree(ri);
        if (re != bn1) bnfree(re);
        return 0;
      }
      bnfree(br); bnfree(bd); bnfree(bi); bnfree(be);
      br = tr; bd = td; bi = ti; be = te;
    }
  }

  bnfree(br); bnfree(bd); bnfree(bi); bnfree(be);

  *yr = rr; *yd = rd;
  *yi = ri; *ye = re;
  return 1;
}

/* try to get reciprocal of a complex rational; return 1 on success, 0 on zero division */
static int bnx_crecip(const bignum_t *yr, const bignum_t *yd, const bignum_t *yi, const bignum_t *ye,
  bignum_t **out_rr, bignum_t **out_rd, bignum_t **out_ri, bignum_t **out_re)
{
  bignum_t *a, *b, *d, *a2, *b2, *denom;
  bignum_t *num_r, *num_i;
  bignum_t *g_r, *g_i;
  bignum_t *tr, *td, *ti, *te;

  if (IN_RZ4(yr, yd, yi, ye)) return 0;

  if (BNZERO(yi)) {
    if (BNZERO(yr)) return 0;
    tr = bndup(yd);
    td = bndup(yr);
    if (td->isneg) {
      bignum_t *n1 = bnneg(tr);
      bignum_t *d1 = bnabs(td);
      bnfree(tr); bnfree(td);
      tr = n1; td = d1;
    }
    bnreduce(&tr, &td);
    *out_rr = tr; *out_rd = td;
    *out_ri = bn0; *out_re = bn1;
    return 1;
  }

  if (BNZERO(yr)) {
    tr = bn0; td = bn1;
    ti = bnneg(ye);
    te = bndup(yi);
    if (te->isneg) {
      bignum_t *n1 = bnneg(ti);
      bignum_t *d1 = bnabs(te);
      bnfree(ti); bnfree(te);
      ti = n1; te = d1;
    }
    bnreduce(&ti, &te);
    *out_rr = tr; *out_rd = td;
    *out_ri = ti; *out_re = te;
    return 1;
  }

  a = bnmul(yr, ye);
  b = bnmul(yi, yd);
  d = bnmul(yd, ye);

  a2 = bnmul(a, a);
  b2 = bnmul(b, b);
  denom = bnadd(a2, b2);
  bnfree(a2); bnfree(b2);

  num_r = bnmul(a, d);
  {
    bignum_t *bd_prod = bnmul(b, d);
    num_i = bnneg(bd_prod);
    bnfree(bd_prod);
  }
  bnfree(a); bnfree(b); bnfree(d);

  g_r = bngcd(num_r, denom);
  tr = bndiv(num_r, g_r);
  td = bndiv(denom, g_r);
  bnfree(num_r); bnfree(g_r);

  g_i = bngcd(num_i, denom);
  ti = bndiv(num_i, g_i);
  te = bndiv(denom, g_i);
  bnfree(num_i); bnfree(g_i);
  bnfree(denom);

  if (td->isneg) {
    bignum_t *n1 = bnneg(tr);
    bignum_t *d1 = bnabs(td);
    bnfree(tr); bnfree(td);
    tr = n1; td = d1;
  }
  if (te->isneg) {
    bignum_t *n1 = bnneg(ti);
    bignum_t *d1 = bnabs(te);
    bnfree(ti); bnfree(te);
    ti = n1; te = d1;
  }

  *out_rr = tr; *out_rd = td;
  *out_ri = ti; *out_re = te;
  return 1;
}

/* try to compute exact principal power of complex rational:
 * x^(nn/dn) where x = nrx/drx + i*nix/dix; supports negative exponents via reciprocal.
 * Returns 1 if exact principal Gaussian rational result found, 0 otherwise.
 *
 * Preconditions:
 *   - nrx/drx and nix/dix in lowest terms, denominators > 0
 *   - nn/dn in lowest terms, dn > 0
 *   - nn and dn each fit in a single limb
 */
int bnctrypow(const bignum_t *nrx, const bignum_t *drx, const bignum_t *nix, const bignum_t *dix,
  const bignum_t *nn, const bignum_t *dn, bignum_t **pnrr, bignum_t **pdrr, bignum_t **pnir, bignum_t **pdir)
{
  unsigned long long a, b, odd_part, k, i;
  int exp_is_neg;
  bignum_t *abs_nrn;
  bignum_t *yr, *yd, *yi, *ye;
  bignum_t *tr, *td, *ti, *te;

  assert(pnrr != NULL && pdrr != NULL && pnir != NULL && pdir != NULL);
  assert(!BNZERO(drx) && !drx->isneg);
  assert(!BNZERO(dix) && !dix->isneg);
  assert(!BNZERO(dn) && !dn->isneg);

  /* x^0 = 1 (checked before size restrictions, since bn0 has size 0) */
  if (BNZERO(nn)) {
    /* in Scheme 0^0 = 1 too */
    *pnrr = bn1; *pdrr = bn1;
    *pnir = bn0; *pdir = bn1;
    return 1;
  }

  /* nn/dn could not be longer that 1 limb */
  if (nn->size != 1 || dn->size != 1) return 0;

  exp_is_neg = nn->isneg;
  abs_nrn = bnabs(nn);
  a = (unsigned long long)dn->limb[0];     /* root index */
  b = (unsigned long long)abs_nrn->limb[0]; /* power magnitude */

  if (a == 0) {
    bnfree(abs_nrn);
    return 0;
  }

  /* zero base */
  if (BNZERO(nrx) && BNZERO(nix)) {
    bnfree(abs_nrn);
    if (exp_is_neg) return 0;
    *pnrr = bn0; *pdrr = bn1;
    *pnir = bn0; *pdir = bn1;
    return 1;
  }

  /* purely real base -> delegate to real solver */
  if (BNZERO(nix)) {
    int res;
    if (IN_RZ2(nrx, drx)) {
      bnfree(abs_nrn);
      return 0;
    }
    res = bnrtrypow(nrx, drx, abs_nrn, dn, &yr, &yd, &yi, &ye);
    bnfree(abs_nrn);
    if (!res) return 0;

    if (exp_is_neg) {
      int ok;
      if (IN_RZ4(yr, yd, yi, ye)) {
        bnfree(yr); bnfree(yd); bnfree(yi); bnfree(ye);
        return 0;
      }
      ok = bnx_crecip(yr, yd, yi, ye, pnrr, pdrr, pnir, pdir);
      bnfree(yr); bnfree(yd); bnfree(yi); bnfree(ye);
      return ok;
    }
    *pnrr = yr; *pdrr = yd;
    *pnir = yi; *pdir = ye;
    return 1;
  }

  /* for z^(b/a) to be rational, |z|^2 must be a perfect a-th power of a rational;
   * we check this before computing z^b, avoiding massive allocations. */
  if (a > 1) {
    bignum_t *nrx2 = bnmul(nrx, nrx), *dix2 = bnmul(dix, dix);
    bignum_t *nix2 = bnmul(nix, nix), *drx2 = bnmul(drx, drx);
    bignum_t *t1 = bnmul(nrx2, dix2), *t2 = bnmul(nix2, drx2);
    bignum_t *mag_num = bnadd(t1, t2), *mag_den; int mag_ok;
    bnfree(t1); bnfree(t2);
    mag_den = bnmul(drx2, dix2);
    bnfree(nrx2); bnfree(dix2); bnfree(nix2); bnfree(drx2);
    bnreduce(&mag_num, &mag_den);
    mag_ok = bnx_maybe_nth_power(mag_num, (size_t)a) &&
             bnx_maybe_nth_power(mag_den, (size_t)a);
    bnfree(mag_num); bnfree(mag_den);
    if (!mag_ok) {
      bnfree(abs_nrn);
      return 0;
    }
  }

  /* complex base: compute y = x^b first */
  if (b > 1) {
    size_t wr = bnwidthu(nrx) + bnwidthu(dix);
    size_t wi = bnwidthu(nix) + bnwidthu(drx);
    unsigned long long est = (wr > wi ? wr : wi) * b;
    if (est > BIGNUM_MAX_BITS) {
      bnfree(abs_nrn);
      return 0;
    }
  }

  if (!bnx_cpow(nrx, drx, nix, dix, b, &yr, &yd, &yi, &ye)) {
    bnfree(abs_nrn);
    return 0;
  }
  bnfree(abs_nrn);

  /* If y became purely real, delegate to real solver */
  if (BNZERO(yi)) {
    bignumll_t lla;
    bignum_t *bna = bnx_makell(&lla, (long long)a);
    int res;

    res = bnrtrypow(yr, yd, bn1, bna, &tr, &td, &ti, &te);
    bnfree(yr); bnfree(yd); bnfree(yi); bnfree(ye);
    if (!res) return 0;

    yr = tr; yd = td; yi = ti; ye = te;
    if (exp_is_neg) {
      int ok;
      if (IN_RZ4(yr, yd, yi, ye)) {
        bnfree(yr); bnfree(yd); bnfree(yi); bnfree(ye);
        return 0;
      }
      ok = bnx_crecip(yr, yd, yi, ye, pnrr, pdrr, pnir, pdir);
      bnfree(yr); bnfree(yd); bnfree(yi); bnfree(ye);
      return ok;
    }
    *pnrr = yr; *pdrr = yd;
    *pnir = yi; *pdir = ye;
    return 1;
  }

  /* Magnitude check: |y|^2 must be an exact a-th power */
  if (a >= 2) {
    bignum_t *ay, *by, *ay2, *by2, *mag_num, *dy, *mag_den, *g;
    bignum_t *red_num, *red_den;
    int mag_ok;

    if (IN_RZ4(yr, yd, yi, ye)) {
      bnfree(yr); bnfree(yd); bnfree(yi); bnfree(ye);
      return 0;
    }

    ay = bnmul(yr, ye);
    by = bnmul(yi, yd);
    ay2 = bnmul(ay, ay);
    by2 = bnmul(by, by);
    mag_num = bnadd(ay2, by2);
    dy = bnmul(yd, ye);
    mag_den = bnmul(dy, dy);
    g = bngcd(mag_num, mag_den);

    red_num = bndiv(mag_num, g);
    red_den = bndiv(mag_den, g);

    mag_ok = bntryroot(red_num, (size_t)a, NULL) &&
             bntryroot(red_den, (size_t)a, NULL);

    bnfree(ay); bnfree(by); bnfree(ay2); bnfree(by2);
    bnfree(dy); bnfree(mag_num); bnfree(mag_den);
    bnfree(g); bnfree(red_num); bnfree(red_den);

    if (!mag_ok) {
      bnfree(yr); bnfree(yd); bnfree(yi); bnfree(ye);
      return 0;
    }
  }

  /* Factor root index: a = 2^k * odd_part */
  odd_part = a;
  k = 0;
  while ((odd_part & 1) == 0) {
    odd_part >>= 1;
    k++;
  }

  /* Step 1: Handle odd root factor */
  if (odd_part > 1) {
    if (IN_RZ4(yr, yd, yi, ye)) {
      bnfree(yr); bnfree(yd); bnfree(yi); bnfree(ye);
      return 0;
    }

    if (!bnctryoddroot(yr, yd, yi, ye, odd_part, &tr, &td, &ti, &te)) {
      bnfree(yr); bnfree(yd); bnfree(yi); bnfree(ye);
      return 0;
    }
    bnfree(yr); bnfree(yd); bnfree(yi); bnfree(ye);
    yr = tr; yd = td; yi = ti; ye = te;
  }

  /* Step 2: Apply repeated square roots for 2^k factor */
  for (i = 0; i < k; i++) {
    if (IN_RZ4(yr, yd, yi, ye)) {
      bnfree(yr); bnfree(yd); bnfree(yi); bnfree(ye);
      return 0;
    }
    if (!bnctrysqrt(yr, yd, yi, ye, &tr, &td, &ti, &te)) {
      bnfree(yr); bnfree(yd); bnfree(yi); bnfree(ye);
      return 0;
    }
    bnfree(yr); bnfree(yd); bnfree(yi); bnfree(ye);
    yr = tr; yd = td; yi = ti; ye = te;
  }

  /* Step 3: Reciprocate if exponent was negative */
  if (exp_is_neg) {
    int ok;
    if (IN_RZ4(yr, yd, yi, ye)) {
      bnfree(yr); bnfree(yd); bnfree(yi); bnfree(ye);
      return 0;
    }
    ok = bnx_crecip(yr, yd, yi, ye, pnrr, pdrr, pnir, pdir);
    bnfree(yr); bnfree(yd); bnfree(yi); bnfree(ye);
    return ok;
  }

  *pnrr = yr; *pdrr = yd;
  *pnir = yi; *pdir = ye;
  return 1;
}


/* [esl+] inexact power bundle */

/* High/low Cody-Waite splitting constants for ln(2) to preserve precision. 
  Their sum gives ln2 in extended precision; to regenerate them, run
  python3 -c "import struct; from mpmath import mp; mp.dps=50; b=struct.unpack(
  '<Q', struct.pack('<d', float(mp.log(2))))[0] & ~((1<<25)-1); c1=struct.unpack(
  '<d', struct.pack('<Q', b))[0]; c2=float(mp.log(2)-c1); 
   print(f'c1 hex: {c1.hex()}\nc1 = {c1:.20e};\nc2 hex: {c2.hex()}\nc2 = {c2:.20e};')" */
#define M_LN2_CW_HI 6.93147178739309310913e-01 /* 0x1.62e42fe000000p-1, hi bits of ln2 */
#define M_LN2_CW_LO 1.82063599850414622404e-09 /* 0x1.f473de6af278fp-30, lo bits of ln2 */ 

/* computes e^x * v without premature over/underflow, or NaN generation. |v| <= 1.0 */
static double bnx_scale_expmul(double x, double v)
{
  double fk, r, er;
  int k, k1, k2;

  if (v == 0.0) return 0.0;
  if (x == 0.0) return v;

  /* 99.9% of inputs where exp(x) neither overflows nor underflows:
   * bounded inside [ln(DBL_MIN), ln(DBL_MAX)] ~= [-708.4, +709.8] */
  if (x >= -700.0 && x <= 700.0)  return exp(x) * v;

  /* underflow: e^-745 < 2^-1074 (absolute minimum subnormal double);
   * with |v| <= 1.0, e^x * v strictly underflows to 0.0. */
  if (x < -745.0) return 0.0;

  /* hard overflow: e^1455 * min_subnormal > DBL_MAX;
   * unconditional overflow to HUGE_VAL regardless of how small |v| is */
  if (x > 1455.0) return (v > 0.0) ? HUGE_VAL : -HUGE_VAL;

  /* Cody-Waite argument reduction for extreme x */
  fk = floor(x * M_LOG2E + 0.5);
  k = (int)fk;

  /* Reduced remainder r = x - k * ln2 */
  r = (x - fk * M_LN2_CW_HI) - fk * M_LN2_CW_LO;

  /* e^r is guaranteed to be in [0.707, 1.414] */
  er = exp(r);

  /* split exponent k to pull subnormal 'v's into normal range before multiplication */
  k1 = k / 2;
  k2 = k - k1;

  return ldexp(er * ldexp(v, k1), k2);
}

/* compute complex power (nrx/drx + i*nix/dix)^(nry/dry + i*niy/diy) */
void bncpowtodd(double *pre, double *pim,
  const bignum_t *nrx, const bignum_t *drx, const bignum_t *nix, const bignum_t *dix,
  const bignum_t *nry, const bignum_t *dry, const bignum_t *niy, const bignum_t *diy)
{
  double yr, yi, ln_rx, theta_x, m, phi;
  bignum_t *a, *b, *d, *a2, *b2, *num, *den;

  /* zero check for x = 0 + 0i */
  if (BNZERO(nrx) && BNZERO(nix)) {
    yr = bnrtod(nry, dry);
    yi = bnrtod(niy, diy);
    if (yr > 0.0) {
      if (pre) *pre = 0.0;
      if (pim) *pim = 0.0;
    } else if (yr == 0.0 && yi == 0.0) {
      if (pre) *pre = 1.0;
      if (pim) *pim = 0.0;
    } else {
      /* 0^(negative or complex) -> Inf/NaN */
      if (pre) *pre = HUGE_VAL;
      if (pim) *pim = 0.0;
    }
    return;
  }

  /* convert exponent y to double components */
  yr = bnrtod(nry, dry); yi = bnrtod(niy, diy);

  /* ln|x| in log-space: Re(ln x) = 0.5 * ln((a^2 + b^2) / d^2) */
  a   = bnmul(nix, drx);  /* a = nix * drx */
  b   = bnmul(nrx, dix);  /* b = nrx * dix */
  d   = bnmul(drx, dix);  /* d = drx * dix */
  a2  = bnmul(a, a);
  b2  = bnmul(b, b);
  num = bnadd(a2, b2);
  den = bnmul(d, d);

  ln_rx = 0.5 * bnrlogtod(num, den);

  bnfree(a);   bnfree(b);   bnfree(d);
  bnfree(a2);  bnfree(b2);  bnfree(num);  bnfree(den);

  /* phase angle arg(x) = atan2(x_i, x_r) */
  theta_x = bnratan2tod(nix, dix, nrx, drx);

  /* complex multiplication in log-space: y * ln(x)
   * m   = Re(y * ln(x)) = yr * ln|x| - yi * arg(x)
   * phi = Im(y * ln(x)) = yr * arg(x) + yi * ln|x| */
  m   = yr * ln_rx - yi * theta_x;
  phi = yr * theta_x + yi * ln_rx;

  /* scaled exponentiation preventing intermediate overflow/underflow */
  if (pre) *pre = bnx_scale_expmul(m, cos(phi));
  if (pim) *pim = bnx_scale_expmul(m, sin(phi));
}

/* [esl] end of power bundles */


/* [esl+] inexact and exact->inexact trigonometry bundle */

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

/*  ln(nr/dr + i * ni/di) -> *pre + i * *pim */
void bnclogtodd(double *pre, double *pim, const bignum_t *nr, const bignum_t *dr, const bignum_t *ni, const bignum_t *di)
{
  bignum_t *a, *b, *d, *a2, *b2, *num, *den;
  assert(pre && pim); 
  assert(!BNZERO(dr) && !di->isneg);
  assert(!BNZERO(di) && !di->isneg);

  /* Re(ln z) = 0.5 * ln(|z|^2) */
  a   = bnmul(ni, dr);  /* a = ni * dr */
  b   = bnmul(nr, di);  /* b = nr * di */
  d   = bnmul(dr, di);  /* d = dr * di */
  a2  = bnmul(a, a);
  b2  = bnmul(b, b);
  num = bnadd(a2, b2);
  den = bnmul(d, d);
  *pre = 0.5 * bnrlogtod(num, den);
  bnfree(a); bnfree(b); bnfree(d);
  bnfree(a2); bnfree(b2);
  bnfree(num); bnfree(den);

  /* Im(ln z) = arg(z) = atan2(y, x) */
  *pim = bnratan2tod(ni, di, nr, dr);
}

/* two-argument logarithm *pre+*pim*i  = log_y(x); returns complex? flag */
int bnclogntodd(double *pre, double *pim, /* use *pim only if return value is not 0 */
  const bignum_t *nrx, const bignum_t *drx, const bignum_t *nix, const bignum_t *dix,
  const bignum_t *nry, const bignum_t *dry, const bignum_t *niy, const bignum_t *diy)
{
  double ux, vx, uy, vy;

  /* structural analysis: log_y(x) is provably real iff
   * x is a positive real, y is a positive real, and y != 1 */
  int  result_is_real = BNZERO(nix) && BNZERO(niy) &&
                        !nrx->isneg && !BNZERO(nrx) &&
                        !nry->isneg && !BNZERO(nry) &&
                        (!BNONE(nry, 0) || !BNONE(dry, 0));

  /* near-unity cancellation: x and y round to 1+0i but aren't exactly 1 */
  { /* compute (x-1)/(y-1) using the Taylor expansion ln(1+e) ~= e for small e */
    double rx = bnrtod(nrx, drx), ix = bnrtod(nix, dix);
    double ry = bnrtod(nry, dry), iy = bnrtod(niy, diy);

    if (rx == 1.0 && ix == 0.0 && ry == 1.0 && iy == 0.0) {
      /* O(1) checks using BNONE for normalized inputs */
      int x_is_one = BNZERO(nix) && BNONE(nrx, 0) && BNONE(drx, 0);
      int y_is_one = BNZERO(niy) && BNONE(nry, 0) && BNONE(dry, 0);

      if (!x_is_one && !y_is_one) {
        /* fast path: purely real x and y near 1 */
        if (BNZERO(nix) && BNZERO(niy)) {
          bignum_t *dx1 = bnsub(nrx, drx), *dy1 = bnsub(nry, dry);
          bignum_t *num = bnmul(dx1, dry), *den = bnmul(dy1, drx);

          if (pre) *pre = bnrtod(num, den);
          if (pim) *pim = 0.0;

          bnfree(dx1); bnfree(dy1); bnfree(num); bnfree(den);
          return 0;
        } else {
          /* exact Gaussian rational division (x-1)/(y-1) */
          bignum_t *dx1 = bnsub(nrx, drx), *dy1 = bnsub(nry, dry);
          bignum_t *ux = bnmul(dx1, dix), *vx = bnmul(nix, drx), *dx = bnmul(drx, dix);
          bignum_t *uy = bnmul(dy1, diy), *vy = bnmul(niy, dry), *dy = bnmul(dry, diy);

          /* numerator and denominator of (ux + i vx) / (uy + i vy) */
          bignum_t *u1 = bnmul(ux, uy), *u2 = bnmul(vx, vy);
          bignum_t *v1 = bnmul(vx, uy), *v2 = bnmul(ux, vy);
          bignum_t *q1 = bnmul(uy, uy), *q2 = bnmul(vy, vy);

          bignum_t *m_re = bnadd(u1, u2), *m_im = bnsub(v1, v2);
          bignum_t *d_z2 = bnadd(q1, q2), *p_re, *p_im, *q;

          bnfree(u1); bnfree(u2); bnfree(v1); bnfree(v2); bnfree(q1); bnfree(q2);

          /* apply cross-denominators dy and dx */
          p_re = bnmul(m_re, dy), p_im = bnmul(m_im, dy);
          q = bnmul(d_z2, dx);

          bnfree(dx1); bnfree(dy1);
          bnfree(ux); bnfree(vx); bnfree(dx);
          bnfree(uy); bnfree(vy); bnfree(dy);
          bnfree(m_re); bnfree(m_im); bnfree(d_z2);

          *pre = bnrtod(p_re, q);
          *pim = bnrtod(p_im, q);

          bnfree(p_re); bnfree(p_im); bnfree(q);
          return !result_is_real;
        }
      }
    }
  }

  /* General path: ln(x) / ln(y) via bnclogtodd */
  bnclogtodd(&ux, &vx, nrx, drx, nix, dix);
  bnclogtodd(&uy, &vy, nry, dry, niy, diy);
  cmath_cdiv(ux, vx, uy, vy, pre, pim);

  return !result_is_real;
}

/* computes e^(z) = e^x * cos(y) + i * e^x * sin(y) for z = (rn/rd) + i*(in/id) */
void bncexptodd(double *prd, double *pid, const bignum_t *rn, const bignum_t *rd,  const bignum_t *in, const bignum_t *id)
{
  double x, sin_y, cos_y;

  assert(prd != NULL && pid != NULL);
  assert(!BNZERO(rd) && !rd->isneg);
  assert(!BNZERO(id) && !id->isneg);

  x = BNZERO(rn) ? 0.0 : bnrtod(rn, rd);

  if (BNZERO(in)) {
    sin_y = 0.0;
    cos_y = 1.0;
  } else {
    long long diff = (long long)bnwidthu(in) - (long long)bnwidthu(id);
    if (diff < -(DBL_MANT_DIG / 2)) {
      /* tiny diff: tigher-order Taylor terms underflow 53-bit ULP; cos(y)=1.0 and sin(y)=y are exact. */
      double y = bnrtod(in, id);
      sin_y = y;
      cos_y = 1.0;
    } else if (diff < 30) {
      /* arguments up to 2^30 (~10^9) with full libm range reduction precision */
      double y = bnrtod(in, id);
      sin_y = sin(y);
      cos_y = cos(y);
    } else {
      /* full bignum argument reduction required for massive imaginary exponents (|y| >= 10^9) */
      bnrsincostod(in, id, &sin_y, &cos_y);
    }
  }

  *prd = bnx_scale_expmul(x, cos_y);
  *pid = bnx_scale_expmul(x, sin_y);
}

/* hex string for pi/4 (512 fractional nibbles = 2048 fractional bits)
   python3 -c "from mpmath import mp, pi; mp.dps=1000; x=pi/4; n=512; 
   print(format(int(mp.floor(x*16**n)), 'X'))" */
static const char *hex_quadpi_str = /* as many digits as you like */
  "C90FDAA22168C234C4C6628B80DC1CD129024E088A67CC74020BBEA63B139B22"
  "514A08798E3404DDEF9519B3CD3A431B302B0A6DF25F14374FE1356D6D51C245"
  "E485B576625E7EC6F44C42E9A637ED6B0BFF5CB6F406B7EDEE386BFB5A899FA5"
  "AE9F24117C4B1FE649286651ECE45B3DC2007CB8A163BF0598DA48361C55D39A"
  "69163FA8FD24CF5F83655D23DCA3AD961C62F356208552BB9ED529077096966D"
  "670C354E4ABC9804F1746C08CA18217C32905E462E36CE3BE39E772C180E8603"
  "9B2783A2EC07A28FB5C55DF06F4C52C9DE2BCBF6955817183995497CEA956AE5"
  "15D2261898FA051015728E5A8AAAC42DAD33170D04507A33A85521ABDF1CBA64";

static bignum_t *g_quadpi_max = NULL;
static size_t g_max_frac_bits = 0;

static long bnx_init_quadpi(void)
{
  if (!g_quadpi_max) {
    g_quadpi_max = strtobn(hex_quadpi_str, NULL, 16);
    g_max_frac_bits = strlen(hex_quadpi_str) * 4; /* nibbles */
  }
  return (long)g_max_frac_bits;
}

/* reduce n/d modulo pi/4 into nearest octant k in [0..7] 
 * and remainder angle *pnum / *pden in [-pi/8, pi/8] */
long bnx_modoct(const bignum_t *n, const bignum_t *d, bignum_t **pnum, bignum_t **pden)
{
  long m = bnx_init_quadpi();
  bignum_t *nq = bnashll(n, m);
  bignum_t *dp_pi4 = bnmul(d, g_quadpi_max);
  bignum_t *r_den = bnashll(d, m);
  bignum_t *r_num, *two_r;
  bignum_t *k_bn = bndmod(&r_num, nq, dp_pi4);

  long oct = bnmodl(k_bn, 8);
  if (oct < 0) oct = (oct % 8 + 8) % 8;

  /* Ensure remainder is positive in [0, dp_pi4) */
  if (r_num->isneg) {
    bignum_t *adj_num = bnadd(r_num, dp_pi4);
    bnfree(r_num);
    r_num = adj_num;
    oct = (oct + 7) % 8;
  }

  /* Center remainder into [-dp_pi4/2, dp_pi4/2] */
  two_r = bnashll(r_num, 1);
  if (bncmp(two_r, dp_pi4) > 0) {
    bignum_t *adj_num = bnsub(r_num, dp_pi4);
    bnfree(r_num);
    r_num = adj_num;
    oct = (oct + 1) % 8;
  }

  *pnum = r_num;
  *pden = r_den;

  bnfree(nq);
  bnfree(dp_pi4);
  bnfree(k_bn);
  bnfree(two_r);

  return oct;
}

/* compute correlated sin and cos of rational n/d without underflow */
void bnrsincostod(const bignum_t *n, const bignum_t *d, double *psin, double *pcos)
{
  bignum_t *num, *den;
  long oct = bnx_modoct(n, d, &num, &den);
  double delta = bnrtod(num, den);
  double s, c;

#if defined(_GNU_SOURCE)
  sincos(delta, &s, &c);
#else
  s = sin(delta);
  c = cos(delta);
#endif
  
  if (oct & 1) { /* odd octants (pi/4, 3pi/4, etc.): rot by sqrt(2)/2 */
    double u = M_SQRT1_2 * (c + s);
    double v = M_SQRT1_2 * (c - s);
    switch (oct) {
      case 1: *psin =  u; *pcos =  v; break;  /*  pi/4 */
      case 3: *psin =  v; *pcos = -u; break;  /* 3pi/4 */
      case 5: *psin = -u; *pcos = -v; break;  /* 5pi/4 */
      case 7: *psin = -v; *pcos =  u; break;  /* 7pi/4 */
    }
  } else {
    /* for even octants (0, pi/2, pi, 3pi/2) */
    switch (oct) {
      case 0: *psin =  s; *pcos =  c; break;  /* 0     */
      case 2: *psin =  c; *pcos = -s; break;  /* pi/2  */
      case 4: *psin = -s; *pcos = -c; break;  /* pi    */
      case 6: *psin = -c; *pcos =  s; break;  /* 3pi/2 */
    }
  }

  bnfree(num);
  bnfree(den);
}

/* compute complex sine and cosine of rational z = nr/dr + i * ni/di */
void bncsincostodd(double *prsin, double *pisin, double *prcos, double *picos,
  const bignum_t *nr, const bignum_t *dr, const bignum_t *ni, const bignum_t *di)
{
  double sx, cx, y, sh, ch;
  bnrsincostod(nr, dr, &sx, &cx);

  y = bnrtod(ni, di);
  sh = sinh(y); ch = cosh(y);

  /* sin(z) = sin(x)cosh(y) + i * cos(x)sinh(y) */
  *prsin =  sx * ch; *pisin =  cx * sh;

  /* cos(z) = cos(x)cosh(y) - i * sin(x)sinh(y) */
  *prcos =  cx * ch; *picos = -sx * sh;
}

/* compute tan of rational n/d without underflow */
double bnrtantod(const bignum_t *n, const bignum_t *d)
{
  bignum_t *num, *den;
  long oct = bnx_modoct(n, d, &num, &den);
  double delta = bnrtod(num, den);
  double t = tan(delta); 

  bnfree(num);
  bnfree(den);

  /* tan(x) is pi-periodic: collapse 8 octants into 4 (oct & 3) */
  switch (oct & 3) {
    case 0: return t;                            /* 0,   pi   */
    case 1: return  1.0 + (2.0 * t) / (1.0 - t); /* pi/4, 5pi/4 */
    case 2: return -1.0 / t;                     /* pi/2, 3pi/2 */
    case 3: return -1.0 + (2.0 * t) / (1.0 + t); /* 3pi/4, 7pi/4 */
  }
  return 0.0;
}

/* compute complex tangent of rational z = nr/dr + i * ni/di */
void bnctantodd(double *prtan, double *pitan,
  const bignum_t *nr, const bignum_t *dr, const bignum_t *ni, const bignum_t *di)
{
  double sx, cx, y, abs_y;
  /* real trig values x = nr / dr via octant-reduced bnrsincostod */
  bnrsincostod(nr, dr, &sx, &cx);
  /* imaginary component y = ni / di */
  y = bnrtod(ni, di);
  abs_y = fabs(y);

  /* for large |y|, sinh^2(y) completely dominates cos^2(x) */
  if (abs_y > 20.0) {
    double exp_m2y = exp(-2.0 * abs_y);
    *prtan = 4.0 * sx * cx * exp_m2y;
    *pitan = (y > 0.0) ? 1.0 : -1.0;
  } else {
    double sh = sinh(y), ch = cosh(y);
    /* denominator: 0.5 * (cos(2x) + cosh(2y)) = cos^2(x) + sinh^2(y);
     * pure positive sum -> 0 subtraction cancellation near poles! */
    double den = cx * cx + sh * sh;
    *prtan = (sx * cx) / den;
    *pitan = (sh * ch) / den;
  }
}

/* compute complex asin(z) for rational z = nrx/drx + i * nix/dix */
int bncasintodd(double *pre, double *pim, /* use *pim only if return value is not 0 */
  const bignum_t *nrx, const bignum_t *drx, const bignum_t *nix, const bignum_t *dix)
{
  double rx, ix, re, im, eps_log;
  bignum_t *abs_nrx, *diff, *a;
  int is_comp = 1;

  /* fast path for zero */
  if (BNZERO(nrx) && BNZERO(nix)) {
    if (pre) *pre = 0.0;
    if (pim) *pim = 0.0;
    return 0;
  }

  /* near-unity rescue for purely real inputs (y = 0) */
  if (BNZERO(nix)) {
    int cmp_absnd = bncmpabs(nrx, drx);
    rx = bnrtod(nrx, drx);
    if (cmp_absnd <= 0) is_comp = 0;

    /* detect precision loss where x != +-1.0, but bnrtod rounded to +-1.0 */
    if ((rx == 1.0 || rx == -1.0) && cmp_absnd != 0) {
      abs_nrx = bnabs(nrx);

      if (cmp_absnd > 0) {
        /* |x| > 1: asin(1+eps) = pi/2 - i*sqrt(2*eps) */
        diff = bnsub(abs_nrx, drx);
        a = bnashll(diff, 1);
        eps_log = 0.5 * bnrlogtod(a, drx);
        bnfree(diff); bnfree(a); bnfree(abs_nrx);

        im = exp(eps_log);
        if (pre) *pre = nrx->isneg ? -M_PI_2 : M_PI_2;
        if (pim) *pim = nrx->isneg ? im : -im;
        return 1; /* complex */
      } else {
        /* |x| < 1: asin(1-eps) = pi/2 - sqrt(2*eps) */
        diff = bnsub(drx, abs_nrx);
        a = bnashll(diff, 1);
        eps_log = 0.5 * bnrlogtod(a, drx);
        bnfree(diff); bnfree(a); bnfree(abs_nrx);

        re = M_PI_2 - exp(eps_log);
        if (pre) *pre = nrx->isneg ? -re : re;
        if (pim) *pim = 0.0;
        return 0; /* real */
      }
    }
  }

  { /* large-magnitude rescue: |z| >> 1 */
    long long wr = BNZERO(nrx) ? 0 : (long long)bnwidthu(nrx) - (long long)bnwidthu(drx);
    long long wi = BNZERO(nix) ? 0 : (long long)bnwidthu(nix) - (long long)bnwidthu(dix);
    long long wmax = (wr > wi) ? wr : wi;

    /* asin(z) ~ atan2(x, |y|) + i * (ln2 + ln|z|) * sign */
    if (wmax >= DBL_MANT_DIG) {
      bignum_t *a, *b, *d, *a2, *b2, *num, *den;
      bignum_t *abs_nix;
      double ln_absz;

      /* Re(asin(z)) = atan2(x, |y|) */
      abs_nix = bnabs(nix);
      re = bnratan2tod(nrx, drx, abs_nix, dix);
      bnfree(abs_nix);

      /* ln|z| via bignum: 0.5 * ln(|z|^2) */
      a   = bnmul(nix, drx); b   = bnmul(nrx, dix);  d = bnmul(drx, dix); 
      a2  = bnmul(a, a);     b2  = bnmul(b, b);
      num = bnadd(a2, b2);   den = bnmul(d, d);

      ln_absz = 0.5 * bnrlogtod(num, den);

      bnfree(a);  bnfree(b);  bnfree(d);
      bnfree(a2); bnfree(b2); bnfree(num); bnfree(den);

      /* Im = ln(2) + ln|z|, with correct sign */
      im = M_LN2 + ln_absz;

      if (BNZERO(nix)) im = nrx->isneg ? im : -im;
      else im = nix->isneg ? -im : im;

      if (pre) *pre = re;
      if (pim) *pim = im;
      return 1; /* complex */
    }
  }

  /* fallback: convert to double and use C library */
  rx = bnrtod(nrx, drx);
  ix = bnrtod(nix, dix);
  cmath_asin(rx, ix, &re, &im);

  /* R7RS branch cuts for real rationals */
  if (BNZERO(nix)) {
    if      (rx >  1.0) im = -fabs(im);
    else if (rx < -1.0) im =  fabs(im);
  }

  if (pre) *pre = re;
  if (pim) *pim = im;

  return is_comp;
}

/* compute complex acos(z) via acos(z) = pi/2 - asin(z) */
int bncacostodd(double *pre, double *pim, /* use *pim only if return value is not 0 */
  const bignum_t *nrx, const bignum_t *drx, const bignum_t *nix, const bignum_t *dix)
{
  double re, im;
  int is_comp = bncasintodd(&re, &im, nrx, drx, nix, dix);

  /* R7RS branch cuts */
  *pre = M_PI_2 - re;
  if (pim) *pim = -im;

  return is_comp;
}

#define ATAN2_TRUNC_BITS (DBL_MANT_DIG + 7)  /* guard bits for lost precision */
#define ATAN2_DOMINANCE  (DBL_MANT_DIG + 1)  /* dominance relative to pi / (pi/2) */
#define ATAN2_UNDERFLOW  (DBL_MAX_EXP + DBL_MANT_DIG + 2) /* double underflow limit (~1079 bits) */

double bnratan2tod(const bignum_t *ny, const bignum_t *dy, const bignum_t *nx, const bignum_t *dx)
{
  int y_is_neg, x_is_neg;
  size_t wny, wdy, wnx, wdx, sy, sx;
  size_t sh_ny, sh_dy, sh_nx, sh_dx;
  bignum_t *ny_t, *dy_t, *nx_t, *dx_t;
  bignum_t *n_ratio, *d_ratio;
  double r_trunc, r, y_val, x_val;
  int extra;

  if (BNZERO(dy) || BNZERO(dx)) return HUGE_VAL - HUGE_VAL; /* NaN */

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
  /* fast path: |x| >> |y|, x > 0 (underflows double precision to +/- 0.0) */
  if (sx > sy + ATAN2_UNDERFLOW && !x_is_neg) {
    return y_is_neg ? -0.0 : 0.0;
  }

  /* comparable magnitudes or x > 0 with non-underflowing small angle */
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
    bignum_t *bx = getbig(xp), *bq = NULL, *br = NULL;
    bnisqrt(bx, &bq, &br); assert(bq != NULL && br != NULL);
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
    /* quick estimate */
    if (radix < 4)       return FIXNUM_WIDTH/1+3;
    else if (radix < 8)  return FIXNUM_WIDTH/2+3;
    else if (radix < 16) return FIXNUM_WIDTH/3+3;
    else if (radix < 32) return FIXNUM_WIDTH/4+3;
    else                 return FIXNUM_WIDTH/5+3;
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

double rattan(numt_t xt, const nump_t *xp)
{
  assert(NUMT_IS_RATNUM(xt) && "non-rational number");
  if (isfix(xt)) {
    return tan(getfix(xp));
  } else if (isbig(xt)) {
    return bnrtantod(getbig(xp), bn1);
  } else { /* ratio */
    bignumll_t nll, dll; 
    bignum_t *n = isfix(NUMT_RAT_N(xt)) ? (nll = bnll(getfix(xp)), (bignum_t *)&nll) : getbig(xp);
    bignum_t *d = isfix(NUMT_RAT_D(xt)) ? (dll = bnll(getfix(xp+1)), (bignum_t *)&dll) : getbig(xp+1);
    return bnrtantod(n, d);
  }
}

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
        numfini(zt, zp);
        if (isflo(at)) {
          double a = getflo(ap);
          zt = NUMT_MKCOM(setflo(zp, m*cos(a)), setflo(zp+2, m*sin(a)));
        } else {
          double sa, ca; ratsincos(at, ap, &sa, &ca);
          zt = NUMT_MKCOM(setflo(zp, m*ca), setflo(zp+2, m*sa));
        }
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

/* z = min(x, y)  [real numbers only; result is inexact if either arg is] */
numt_t gnummin(nump_t *zp, numt_t xt, const nump_t *xp, numt_t yt, const nump_t *yp)
{
  assert(NUMT_IS_VALID(xt) && "unsupported number type");
  assert(NUMT_IS_VALID(yt) && "unsupported number type");
  if (!isreal(xt) || !isreal(yt)) return setfail(EDOM);
  /* R7RS: if either arg is inexact, result is inexact */
  if (isfix(xt) && isfix(yt)) {
    long x = getfix(xp), y = getfix(yp);
    return setfix(zp, x <= y ? x : y);
  } else if (isflo(xt) && isflo(yt)) {
    double x = getflo(xp), y = getflo(yp);
    return setflo(zp, ieee_fminimum(x, y));
  } else if (isflo(xt)) { /* y is rat */
    double x = getflo(xp);
    if (x != x) { /* NaN! */
      return setflo(zp, x * 1.0); /* quiet it, just in case */
    } else if (isfix(yt)) { 
      double y = (double)getfix(yp); 
      return setflo(zp, ieee_fminimum(x, y));
    } else { /* y is rat */
      bignumll_t nyll, dyll; numt_t nyt = NUMT_RAT_N(yt), dyt = NUMT_RAT_D(yt);
      bignum_t *ny = isbig(nyt) ? getbig(yp)   : bnx_makell(&nyll, getfix(yp));
      bignum_t *dy = isbig(dyt) ? getbig(yp+1) : bnx_makell(&dyll, dyt ? getfix(yp+1) : 1);
      int cmp = -bnrdcmp(ny, dy, x);
      return setflo(zp, cmp <= 0 ? x : bnrtod(ny, dy)); 
    }
  } else if (isflo(yt)) { /* x is rat */
    double y = getflo(yp);
    if (y != y) { /* NaN! */
      return setflo(zp, y * 1.0); /* quiet it, just in case */
    } else if (isfix(xt)) { 
      double x = (double)getfix(xp); 
      return setflo(zp, ieee_fminimum(x, y));
    } else { /* x is rat */
      bignumll_t nxll, dxll; numt_t nxt = NUMT_RAT_N(xt), dxt = NUMT_RAT_D(xt);
      bignum_t *nx = isbig(nxt) ? getbig(xp)   : bnx_makell(&nxll, getfix(xp));
      bignum_t *dx = isbig(dxt) ? getbig(xp+1) : bnx_makell(&dxll, dxt ? getfix(xp+1) : 1);
      int cmp = bnrdcmp(nx, dx, y);
      return setflo(zp, cmp <= 0 ? bnrtod(nx, dx) : y); 
    }
  } else { /* both are rat */
    return ratcmp(xt, xp, yt, yp) <= 0 ? numdup(zp, xt, xp) : numdup(zp, yt, yp);
  }
}

/* z = max(x, y)  [real numbers only; result is inexact if either arg is] */
numt_t gnummax(nump_t *zp, numt_t xt, const nump_t *xp, numt_t yt, const nump_t *yp)
{
  assert(NUMT_IS_VALID(xt) && "unsupported number type");
  assert(NUMT_IS_VALID(yt) && "unsupported number type");
  if (!isreal(xt) || !isreal(yt)) return setfail(EDOM);
  /* R7RS: if either arg is inexact, result is inexact */
  if (isfix(xt) && isfix(yt)) {
    long x = getfix(xp), y = getfix(yp);
    return setfix(zp, x >= y ? x : y);
  } else if (isflo(xt) && isflo(yt)) {
    double x = getflo(xp), y = getflo(yp);
    return setflo(zp, ieee_fmaximum(x, y));
  } else if (isflo(xt)) { /* y is rat */
    double x = getflo(xp);
    if (x != x) { /* NaN! */
      return setflo(zp, x * 1.0); /* quiet it, just in case */
    } else if (isfix(yt)) { 
      double y = (double)getfix(yp); 
      return setflo(zp, ieee_fmaximum(x, y));
    } else { /* y is rat */
      bignumll_t nyll, dyll; numt_t nyt = NUMT_RAT_N(yt), dyt = NUMT_RAT_D(yt);
      bignum_t *ny = isbig(nyt) ? getbig(yp)   : bnx_makell(&nyll, getfix(yp));
      bignum_t *dy = isbig(dyt) ? getbig(yp+1) : bnx_makell(&dyll, dyt ? getfix(yp+1) : 1);
      int cmp = -bnrdcmp(ny, dy, x);
      return setflo(zp, cmp >= 0 ? x : bnrtod(ny, dy)); 
    }
  } else if (isflo(yt)) { /* x is rat */
    double y = getflo(yp);
    if (y != y) { /* NaN! */
      return setflo(zp, y * 1.0); /* quiet it, just in case */
    } else if (isfix(xt)) { 
      double x = (double)getfix(xp); 
      return setflo(zp, ieee_fmaximum(x, y));
    } else { /* x is rat */
      bignumll_t nxll, dxll; numt_t nxt = NUMT_RAT_N(xt), dxt = NUMT_RAT_D(xt);
      bignum_t *nx = isbig(nxt) ? getbig(xp)   : bnx_makell(&nxll, getfix(xp));
      bignum_t *dx = isbig(dxt) ? getbig(xp+1) : bnx_makell(&dxll, dxt ? getfix(xp+1) : 1);
      int cmp = bnrdcmp(nx, dx, y);
      return setflo(zp, cmp >= 0 ? bnrtod(nx, dx) : y); 
    }
  } else { /* both are rat */
    return ratcmp(xt, xp, yt, yp) >= 0 ? numdup(zp, xt, xp) : numdup(zp, yt, yp);
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
    }
    if (!isint(xt) && israt(xt)) {
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
    }
    if (isreal(xt)) {
      double x = rattod(xt, xp);
      if (x >= 0.0) return setflo(zp, sqrt(x));
      return NUMT_MKCOM(setflo(zp, 0.0), setflo(zp+2, sqrt(-x)));
    } else {
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
    /* 0^y = 0 for Re(y) >= 0; fail if Re(y) < 0 */
    if (isfix(xt) && getfix(xp) == 0) 
      return ratsign(NUMT_COM_R(yt), yp) <= 0 ? setfail(EDOM) : setfix(zp, 0);
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
#if 0
  } else if (israt(xt) && isreal(yt) && ratsign(xt, xp) > 0) {
    /* x^y = exp(log(x)*y) via inexacts, but w/lower chances of overflow */
    nump_t lp[4]; numt_t lt = gnumlog(lp, xt, xp);
    nump_t mp[4]; numt_t mt = gnummul(mp, lt, lp, yt, yp);
    numt_t zt = gnumexp(zp, mt, mp); 
    numfini(lt, lp); numfini(mt, mp);
    return zt;
#endif
  } else if (isrect(xt) && isrect(yt)) { /* try not to loose precision */
    bignumll_t nrxll, drxll, nixll, dixll, nryll, dryll, niyll, diyll;
    numt_t rxt = NUMT_COM_R(xt), nrxt = NUMT_RAT_N(rxt), drxt = NUMT_RAT_D(rxt); 
    bignum_t *nrx = isbig(nrxt) ? getbig(xp)   : bnx_makell(&nrxll, getfix(xp));
    bignum_t *drx = isbig(drxt) ? getbig(xp+1) : bnx_makell(&drxll, drxt ? getfix(xp+1) : 1);
    numt_t ixt = NUMT_COM_I(xt), nixt = NUMT_RAT_N(ixt), dixt = NUMT_RAT_D(ixt); 
    bignum_t *nix = isbig(nixt) ? getbig(xp+2) : bnx_makell(&nixll, nixt ? getfix(xp+2) : 0);
    bignum_t *dix = isbig(dixt) ? getbig(xp+3) : bnx_makell(&dixll, dixt ? getfix(xp+3) : 1);
    numt_t ryt = NUMT_COM_R(yt), nryt = NUMT_RAT_N(ryt), dryt = NUMT_RAT_D(ryt); 
    bignum_t *nry = isbig(nryt) ? getbig(yp)   : bnx_makell(&nryll, getfix(yp));
    bignum_t *dry = isbig(dryt) ? getbig(yp+1) : bnx_makell(&dryll, dryt ? getfix(yp+1) : 1);
    numt_t iyt = NUMT_COM_I(yt), niyt = NUMT_RAT_N(iyt), diyt = NUMT_RAT_D(iyt); 
    bignum_t *niy = isbig(niyt) ? getbig(yp+2) : bnx_makell(&niyll, niyt ? getfix(yp+2) : 0);
    bignum_t *diy = isbig(diyt) ? getbig(yp+3) : bnx_makell(&diyll, diyt ? getfix(yp+3) : 1);
    bignum_t *nrz = NULL, *drz = NULL, *niz = NULL, *diz = NULL;
    if (BNZERO(niy) && bnctrypow(nrx, drx, nix, dix, nry, dry, &nrz, &drz, &niz, &diz)) {
      numt_t nrt, drt = NUMT_NONE, nit, dit = NUMT_NONE; long l;
      assert(nrz != NULL && drz != NULL && niz != NULL && diz != NULL);
      if (bnwidths(nrz) > FIXNUM_WIDTH) nrt = setbig(zp+0, nrz);
      else { nrt = setfix(zp+0, bntol(nrz)); bnfree(nrz); }
      if (bnwidths(drz) > FIXNUM_WIDTH) drt = setbig(zp+1, drz);
      else { l = bntol(drz); bnfree(drz); if (l != 1) drt = setfix(zp+1, l); }        
      if (bnwidths(niz) > FIXNUM_WIDTH) nit = setbig(zp+2, niz);
      else { nit = setfix(zp+2, bntol(niz)); bnfree(niz); }
      if (bnwidths(diz) > FIXNUM_WIDTH) dit = setbig(zp+3, diz);
      else { l = bntol(diz); bnfree(diz); if (l != 1) dit = setfix(zp+3, l); }
      if (nit == NUMT_FIX && dit == NUMT_NONE && getfix(zp+2) == 0) return NUMT_MKRAT(nrt, drt);
      else return NUMT_MKCOM(NUMT_MKRAT(nrt, drt), NUMT_MKRAT(nit, dit));
    } else {
      double rz, iz; bncpowtodd(&rz, &iz, nrx, drx, nix, dix, nry, dry, niy, diy); 
      /* if inputs were purely real, and xt is positive, result is real */
      if (israt(xt) && israt(yt) && bnsign(nrx) > 0 && iz == 0.0) return setflo(zp, rz); 
      else return NUMT_MKCOM(setflo(zp, rz), setflo(zp+2, iz));
    }
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
    if (isflo(yt)) {
      double a = getflo(yp);
      return NUMT_MKCOM(setflo(zp, m*cos(a)), setflo(zp+2, m*sin(a)));
    } else {
      double sa, ca; ratsincos(yt, yp, &sa, &ca);
      return NUMT_MKCOM(setflo(zp, m*ca), setflo(zp+2, m*sa));
    }
  }
}

/* transcendental functions: all return inexact (mostly) */

/* z = exp(x) */
numt_t gnumexp(nump_t *zp, numt_t xt, const nump_t *xp)
{
  assert(NUMT_IS_VALID(xt) && "unsupported number type");
  if (isfix(xt) && getfix(xp) == 0) {
    return setfix(zp, 1); /* consistent with log, expt */
  } else if (isreal(xt)) {
    double x = isflo(xt) ? getflo(xp) : rattod(xt, xp);
    return setflo(zp, exp(x));
  } else if (isrect(xt)) {
    bignumll_t nrll, drll, nill, dill;
    numt_t rt = NUMT_COM_R(xt), nrt = NUMT_RAT_N(rt), drt = NUMT_RAT_D(rt); 
    bignum_t *nr = isbig(nrt) ? getbig(xp)   : bnx_makell(&nrll, getfix(xp));
    bignum_t *dr = isbig(drt) ? getbig(xp+1) : bnx_makell(&drll, drt ? getfix(xp+1) : 1);
    numt_t it = NUMT_COM_I(xt), nit = NUMT_RAT_N(it), dit = NUMT_RAT_D(it); 
    bignum_t *ni = isbig(nit) ? getbig(xp+2) : bnx_makell(&nill, getfix(xp+2));
    bignum_t *di = isbig(dit) ? getbig(xp+3) : bnx_makell(&dill, dit ? getfix(xp+3) : 1);
    double zr, zi; bncexptodd(&zr, &zi, nr, dr, ni, di);
    return NUMT_MKCOM(setflo(zp, zr), setflo(zp+2, zi));
  } else { /* compnum */
    double rx, ix, re, im;
    comptodd(xt, xp, &rx, &ix);
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
    bignumll_t nxll, dxll;
    numt_t nxt = NUMT_RAT_N(xt), dxt = NUMT_RAT_D(xt); 
    bignum_t *nx = isbig(nxt) ? getbig(xp)   : bnx_makell(&nxll, getfix(xp));
    bignum_t *dx = isbig(dxt) ? getbig(xp+1) : bnx_makell(&dxll, dxt ? getfix(xp+1) : 1);
    double z = bnrlogtod(nx, dx); /* z = log(|nx|/dx), accurate around 1 */
    if (sign >= 0) return setflo(zp, z);
    return NUMT_MKCOM(setflo(zp, z), setflo(zp+2, M_PI));
  } else if (isflo(xt)) {
    double x = getflo(xp);
    /* if (x > 0.0 || x == 0.0 && 1.0/x > 0.0) return setflo(zp, log(x)); */
    if (x >= 0.0) return setflo(zp, log(x));
    return NUMT_MKCOM(setflo(zp, log(-x)), setflo(zp+2, M_PI));
  } else if (isrect(xt)) {
    bignumll_t nrll, drll, nill, dill;
    numt_t rt = NUMT_COM_R(xt), nrt = NUMT_RAT_N(rt), drt = NUMT_RAT_D(rt); 
    bignum_t *nr = isbig(nrt) ? getbig(xp)   : bnx_makell(&nrll, getfix(xp));
    bignum_t *dr = isbig(drt) ? getbig(xp+1) : bnx_makell(&drll, drt ? getfix(xp+1) : 1);
    numt_t it = NUMT_COM_I(xt), nit = NUMT_RAT_N(it), dit = NUMT_RAT_D(it); 
    bignum_t *ni = isbig(nit) ? getbig(xp+2) : bnx_makell(&nill, getfix(xp+2));
    bignum_t *di = isbig(dit) ? getbig(xp+3) : bnx_makell(&dill, dit ? getfix(xp+3) : 1);
    double zr, zi; bnclogtodd(&zr, &zi, nr, dr, ni, di);
    return NUMT_MKCOM(setflo(zp, zr), setflo(zp+2, zi));
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
    } else { /* isreal(xt) */
      double x = isflo(xt) ? getflo(xp) : rattod(xt, xp);
      if (isflo(xt) || (x != 0.0 && fabs(x) < HUGE_VAL)) { /* rat is ok */
        if (x >= 0.0) return setflo(zp, log10(x));
        /* negative real: log10(x) = log10|x| + i*pi/log(10) */
        return NUMT_MKCOM(setflo(zp, log10(-x)), setflo(zp+2, M_PI_LN10));
      }
    } /* else fall thru */
  } 
  if (isrect(xt) && isrect(yt)) { /* try not to loose precision */
    bignumll_t nrxll, drxll, nixll, dixll, nryll, dryll, niyll, diyll;
    numt_t rxt = NUMT_COM_R(xt), nrxt = NUMT_RAT_N(rxt), drxt = NUMT_RAT_D(rxt); 
    bignum_t *nrx = isbig(nrxt) ? getbig(xp)   : bnx_makell(&nrxll, getfix(xp));
    bignum_t *drx = isbig(drxt) ? getbig(xp+1) : bnx_makell(&drxll, drxt ? getfix(xp+1) : 1);
    numt_t ixt = NUMT_COM_I(xt), nixt = NUMT_RAT_N(ixt), dixt = NUMT_RAT_D(ixt); 
    bignum_t *nix = isbig(nixt) ? getbig(xp+2) : bnx_makell(&nixll, nixt ? getfix(xp+2) : 0);
    bignum_t *dix = isbig(dixt) ? getbig(xp+3) : bnx_makell(&dixll, dixt ? getfix(xp+3) : 1);
    numt_t ryt = NUMT_COM_R(yt), nryt = NUMT_RAT_N(ryt), dryt = NUMT_RAT_D(ryt); 
    bignum_t *nry = isbig(nryt) ? getbig(yp)   : bnx_makell(&nryll, getfix(yp));
    bignum_t *dry = isbig(dryt) ? getbig(yp+1) : bnx_makell(&dryll, dryt ? getfix(yp+1) : 1);
    numt_t iyt = NUMT_COM_I(yt), niyt = NUMT_RAT_N(iyt), diyt = NUMT_RAT_D(iyt); 
    bignum_t *niy = isbig(niyt) ? getbig(yp+2) : bnx_makell(&niyll, niyt ? getfix(yp+2) : 0);
    bignum_t *diy = isbig(diyt) ? getbig(yp+3) : bnx_makell(&diyll, diyt ? getfix(yp+3) : 1);
    double rz, iz; int com = bnclogntodd(&rz, &iz, nrx, drx, nix, dix, nry, dry, niy, diy); 
    if (!com) return setflo(zp, rz); /* result is purely real */
    else return NUMT_MKCOM(setflo(zp, rz), setflo(zp+2, iz));
  } else { /* fallback code; used for int and rat x too */
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
    if (isflo(xt)) return setflo(zp, sin(getflo(xp)));
    else { double s, c; ratsincos(xt, xp, &s, &c); return setflo(zp, s); }
  } else if (isrect(xt)) {
    bignumll_t nrll, drll, nill, dill;
    numt_t rt = NUMT_COM_R(xt), nrt = NUMT_RAT_N(rt), drt = NUMT_RAT_D(rt); 
    bignum_t *nr = isbig(nrt) ? getbig(xp)   : bnx_makell(&nrll, getfix(xp));
    bignum_t *dr = isbig(drt) ? getbig(xp+1) : bnx_makell(&drll, drt ? getfix(xp+1) : 1);
    numt_t it = NUMT_COM_I(xt), nit = NUMT_RAT_N(it), dit = NUMT_RAT_D(it); 
    bignum_t *ni = isbig(nit) ? getbig(xp+2) : bnx_makell(&nill, getfix(xp+2));
    bignum_t *di = isbig(dit) ? getbig(xp+3) : bnx_makell(&dill, dit ? getfix(xp+3) : 1);
    double zr, zi, fr, fi; bncsincostodd(&zr, &zi, &fr, &fi, nr, dr, ni, di); 
    return NUMT_MKCOM(setflo(zp, zr), setflo(zp+2, zi));
  } else { /* compnum */
    double rx, ix, re, im;
    comptodd(xt, xp, &rx, &ix);
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
    if (isflo(xt)) return setflo(zp, cos(getflo(xp)));
    else { double s, c; ratsincos(xt, xp, &s, &c); return setflo(zp, c); }
  } else if (isrect(xt)) {
    bignumll_t nrll, drll, nill, dill;
    numt_t rt = NUMT_COM_R(xt), nrt = NUMT_RAT_N(rt), drt = NUMT_RAT_D(rt); 
    bignum_t *nr = isbig(nrt) ? getbig(xp)   : bnx_makell(&nrll, getfix(xp));
    bignum_t *dr = isbig(drt) ? getbig(xp+1) : bnx_makell(&drll, drt ? getfix(xp+1) : 1);
    numt_t it = NUMT_COM_I(xt), nit = NUMT_RAT_N(it), dit = NUMT_RAT_D(it); 
    bignum_t *ni = isbig(nit) ? getbig(xp+2) : bnx_makell(&nill, getfix(xp+2));
    bignum_t *di = isbig(dit) ? getbig(xp+3) : bnx_makell(&dill, dit ? getfix(xp+3) : 1);
    double zr, zi, fr, fi; bncsincostodd(&fr, &fi, &zr, &zi, nr, dr, ni, di); 
    return NUMT_MKCOM(setflo(zp, zr), setflo(zp+2, zi));
  } else { /* compnum */
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
    if (isflo(xt)) return setflo(zp, tan(getflo(xp)));
    else return setflo(zp, rattan(xt, xp));
  } else if (isrect(xt)) {
    bignumll_t nrll, drll, nill, dill;
    numt_t rt = NUMT_COM_R(xt), nrt = NUMT_RAT_N(rt), drt = NUMT_RAT_D(rt); 
    bignum_t *nr = isbig(nrt) ? getbig(xp)   : bnx_makell(&nrll, getfix(xp));
    bignum_t *dr = isbig(drt) ? getbig(xp+1) : bnx_makell(&drll, drt ? getfix(xp+1) : 1);
    numt_t it = NUMT_COM_I(xt), nit = NUMT_RAT_N(it), dit = NUMT_RAT_D(it); 
    bignum_t *ni = isbig(nit) ? getbig(xp+2) : bnx_makell(&nill, getfix(xp+2));
    bignum_t *di = isbig(dit) ? getbig(xp+3) : bnx_makell(&dill, dit ? getfix(xp+3) : 1);
    double zr, zi; bnctantodd(&zr, &zi, nr, dr, ni, di); 
    return NUMT_MKCOM(setflo(zp, zr), setflo(zp+2, zi));
  } else { /* compnum */
    double rx, ix, re, im;
    comptodd(xt, xp, &rx, &ix);
    cmath_tan(rx, ix, &re, &im);
    return NUMT_MKCOM(setflo(zp, re), setflo(zp+2, im));
  }
}

/* z = asin(x) */
numt_t gnumasin(nump_t *zp, numt_t xt, const nump_t *xp)
{
  assert(NUMT_IS_VALID(xt) && "unsupported number type");
  if (isfix(xt) && getfix(xp) == 0) {
    return setfix(zp, 0); /* consistent with sin */
  } else if (isflo(xt) && getflo(xp) != getflo(xp)) { /* NaN */
    /* traditionally imag NaN is not added (although it makes sense) */
    return setflo(zp, HUGE_VAL-HUGE_VAL); /* just real NaN */
  } else if (isrect(xt)) {
    bignumll_t nrll, drll, nill, dill; double zr, zi;
    numt_t rt = NUMT_COM_R(xt), nrt = NUMT_RAT_N(rt), drt = NUMT_RAT_D(rt); 
    bignum_t *nr = isbig(nrt) ? getbig(xp)   : bnx_makell(&nrll, getfix(xp));
    bignum_t *dr = isbig(drt) ? getbig(xp+1) : bnx_makell(&drll, drt ? getfix(xp+1) : 1);
    numt_t it = NUMT_COM_I(xt), nit = NUMT_RAT_N(it), dit = NUMT_RAT_D(it); 
    bignum_t *ni = isbig(nit) ? getbig(xp+2) : bnx_makell(&nill, nit ? getfix(xp+2) : 0);
    bignum_t *di = isbig(dit) ? getbig(xp+3) : bnx_makell(&dill, dit ? getfix(xp+3) : 1);
    if (!bncasintodd(&zr, &zi, nr, dr, ni, di)) return setflo(zp, zr);
    else return NUMT_MKCOM(setflo(zp, zr), setflo(zp+2, zi));
  } else { /* flonum or compnum */
    double rx, ix, re, im;
    if (isflo(xt)) rx = getflo(xp), ix = 0.0;
    else comptodd(xt, xp, &rx, &ix);
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
}

/* z = acos(x) */
numt_t gnumacos(nump_t *zp, numt_t xt, const nump_t *xp)
{
  assert(NUMT_IS_VALID(xt) && "unsupported number type");
  if (isfix(xt) && getfix(xp) == 1) {
    return setfix(zp, 0); /* consistent with cos */
  } else if (isflo(xt) && getflo(xp) != getflo(xp)) { /* NaN */
    /* traditionally imag NaN is not added (although it makes sense) */
    return setflo(zp, HUGE_VAL-HUGE_VAL); /* just real NaN */
  } else if (isrect(xt)) {
    bignumll_t nrll, drll, nill, dill; double zr, zi;
    numt_t rt = NUMT_COM_R(xt), nrt = NUMT_RAT_N(rt), drt = NUMT_RAT_D(rt); 
    bignum_t *nr = isbig(nrt) ? getbig(xp)   : bnx_makell(&nrll, getfix(xp));
    bignum_t *dr = isbig(drt) ? getbig(xp+1) : bnx_makell(&drll, drt ? getfix(xp+1) : 1);
    numt_t it = NUMT_COM_I(xt), nit = NUMT_RAT_N(it), dit = NUMT_RAT_D(it); 
    bignum_t *ni = isbig(nit) ? getbig(xp+2) : bnx_makell(&nill, nit ? getfix(xp+2) : 0);
    bignum_t *di = isbig(dit) ? getbig(xp+3) : bnx_makell(&dill, dit ? getfix(xp+3) : 1);
    if (!bncacostodd(&zr, &zi, nr, dr, ni, di)) return setflo(zp, zr);
    else return NUMT_MKCOM(setflo(zp, zr), setflo(zp+2, zi));
  } else { /* flonum or compnum */
    double rx, ix, re, im;
    if (isflo(xt)) rx = getflo(xp), ix = 0.0;
    else comptodd(xt, xp, &rx, &ix);
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
