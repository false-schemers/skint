/* scmi.c -- callable implementations behind the $scmi instruction
 * macros (lib/scmi.mvh), the MARV counterpart of the slow paths in
 * Skint's i.c generic arithmetic.
 *
 * The macros keep only what inlines well: the both-fixnums fast
 * path (a tag test, one i64 operation, a range check) and the
 * convention adaptation (first operand in $ac, rest popped from
 * the operand stack, result back in $ac).  Everything else --
 * mixed fixnum/flonum dispatch, overflow promotion, floored
 * division adjustment, exact power, min/max contagion -- lives
 * here, once, instead of being substituted at every call site.
 *
 * This file is written in the WCPL C dialect, in the same
 * single-source style as the other adapter libraries: compiled
 * with wcpl -c it becomes the $scmi module of a WCPL binary;
 * compiled with a plain C compiler the exported names are
 * prefixed scmi_ (via the #define block below), matching what the
 * MARV C target expects ($scmi:add_slow -> scmi_add_slow, and so
 * on).  In compile-and-link mode MARV links this file
 * automatically as the companion of lib/scmi.mvh.
 *
 * Object representation (see lib/scmn.mvh): fixnums are the
 * 30-bit payload in the low word tagged 3, flonums are the
 * bitwise complement of a double (NAN_BOXING; the two classes
 * are disjoint by construction).  The generic-numeric functions
 * allocate nothing: every value in play is an immediate, so no
 * heap or root discipline is needed there.  The allocating
 * instructions at the end of the file follow the
 * xxx_slow_hreserved convention documented there (caller
 * pre-reserves, result into the cell the caller points to,
 * updated $hp returned).  Operands are unchecked, exactly like
 * the macros: a non-number is dispatched as if it were a
 * flonum.
 */

#if defined(__WCPL__)
#pragma module "scmi"
#include "scmrt.h"
#endif

/* this is for MS headers; shouldn't affect others, as skint's s.h does it */
#define _CRT_SECURE_NO_WARNINGS 1

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#if !defined(__WCPL__)
/* native build: present the scmi_ prefixed names the MARV C target
   imports; the WCPL build gets $scmi:<name> from the pragma above */
#define add_slow  scmi_add_slow
#define sub_slow  scmi_sub_slow
#define mul_slow  scmi_mul_slow
#define div_slow  scmi_div_slow
#define quo_slow  scmi_quo_slow
#define rem_slow  scmi_rem_slow
#define mod_slow  scmi_mod_slow
#define fquo_slow scmi_fquo_slow
#define lt_slow   scmi_lt_slow
#define gt_slow   scmi_gt_slow
#define le_slow   scmi_le_slow
#define ge_slow   scmi_ge_slow
#define eq_slow   scmi_eq_slow
#define ne_slow   scmi_ne_slow
#define gmin_slow scmi_gmin_slow
#define gmax_slow scmi_gmax_slow
#define gabs_slow scmi_gabs_slow
#define expt_slow scmi_expt_slow
#define exact_slow scmi_exact_slow
#define sin_slow  scmi_sin_slow
#define cos_slow  scmi_cos_slow
#define tan_slow  scmi_tan_slow
#define asin_slow scmi_asin_slow
#define acos_slow scmi_acos_slow
#define atan_slow scmi_atan_slow
#define atan2_slow scmi_atan2_slow
#define exp_slow  scmi_exp_slow
#define log_slow  scmi_log_slow
#define sqrt_slow scmi_sqrt_slow
#define fxsqrt_slow scmi_fxsqrt_slow
#define fxsqrtb_slow scmi_fxsqrtb_slow
#define fxaddc_slow scmi_fxaddc_slow
#define fxsubc_slow scmi_fxsubc_slow
#define fxmulc_slow scmi_fxmulc_slow
#define flmodfb_slow scmi_flmodfb_slow
#define flfrexpb_slow scmi_flfrexpb_slow
#define sinh_slow scmi_sinh_slow
#define cosh_slow scmi_cosh_slow
#define tanh_slow scmi_tanh_slow
#define ldexp_slow scmi_ldexp_slow
#define fxgcd_slow scmi_fxgcd_slow
#define flgcd_slow scmi_flgcd_slow
#define fxexpt_slow scmi_fxexpt_slow
#define log10_slow scmi_log10_slow
#define cons_slow_hreserved scmi_cons_slow_hreserved
#define ac_scratch scmi_ac_scratch
#define msg_not_pair scmi_msg_not_pair
#define msg_not_proc scmi_msg_not_proc
#define msg_not_nonzero scmi_msg_not_nonzero
#define msg_not_number scmi_msg_not_number
#define msg_not_string scmi_msg_not_string
#define msg_not_vector scmi_msg_not_vector
#define msg_not_bytevector scmi_msg_not_bytevector
#define msg_not_index scmi_msg_not_index
#define box_slow_hreserved scmi_box_slow_hreserved
#define lmk_slow_hreserved scmi_lmk_slow_hreserved
#define vmk_slow_hreserved scmi_vmk_slow_hreserved
#define lrev_slow_hreserved scmi_lrev_slow_hreserved
#define lcat_slow_hreserved scmi_lcat_slow_hreserved
#define slist_slow_hreserved scmi_slist_slow_hreserved
#define smk_slow_hreserved scmi_smk_slow_hreserved
#define ssub_slow_hreserved scmi_ssub_slow_hreserved
#define scat_slow_hreserved scmi_scat_slow_hreserved
#define sappn_slow_hreserved scmi_sappn_slow_hreserved
#define strn_slow_hreserved scmi_strn_slow_hreserved
#define lstr_slow_hreserved scmi_lstr_slow_hreserved
#define sdwn_slow_hreserved scmi_sdwn_slow_hreserved
#define s8tos_slow_hreserved scmi_s8tos_slow_hreserved
#define ssto8_slow_hreserved scmi_ssto8_slow_hreserved
#define bvnew_slow_hreserved scmi_bvnew_slow_hreserved
#define bvlist_slow_hreserved scmi_bvlist_slow_hreserved
#define symstr_slow_hreserved scmi_symstr_slow_hreserved
#define n2s_slow_hreserved scmi_n2s_slow_hreserved
#define openin_slow_hreserved scmi_openin_slow_hreserved
#define openout_slow_hreserved scmi_openout_slow_hreserved
#define openin2_slow_hreserved scmi_openin2_slow_hreserved
#define openout2_slow_hreserved scmi_openout2_slow_hreserved
#define sinport_slow_hreserved scmi_sinport_slow_hreserved
#define soutport_slow_hreserved scmi_soutport_slow_hreserved
#define getostr_slow_hreserved scmi_getostr_slow_hreserved
#define rdline_slow_hreserved scmi_rdline_slow_hreserved
#define binport_slow_hreserved scmi_binport_slow_hreserved
#define boutport_slow_hreserved scmi_boutport_slow_hreserved
#define getbvo_slow_hreserved scmi_getbvo_slow_hreserved
#define getenv_slow_hreserved scmi_getenv_slow_hreserved
#define envvref_slow_hreserved scmi_envvref_slow_hreserved
/* $scmrt payload/wrapper services used by the allocating
   instructions below (lib/scmrt.h is the WCPL-side declaration) */
#define make_string_payload scmrt_make_string_payload
#define make_string_payload_raw scmrt_make_string_payload_raw
#define string_type scmrt_string_type
#define make_bytevector_payload scmrt_make_bytevector_payload
#define make_bytevector_payload_raw scmrt_make_bytevector_payload_raw
#define char_to_lower    scmrt_char_to_lower
#define bytevector_type scmrt_bytevector_type
#define symbol_to_string scmrt_symbol_to_string
#define number_to_string scmrt_number_to_string
#define open_input_file scmrt_open_input_file
#define open_output_file scmrt_open_output_file
#define try_open_input_file scmrt_try_open_input_file
#define try_open_output_file scmrt_try_open_output_file
#define make_input_string_port scmrt_make_input_string_port
#define make_output_string_port scmrt_make_output_string_port
#define get_output_string_payload scmrt_get_output_string_payload
#define port_read_line scmrt_port_read_line
#define make_input_bytevector_port scmrt_make_input_bytevector_port
#define get_output_bytevector_payload scmrt_get_output_bytevector_payload
#define make_output_bytevector_port scmrt_make_output_bytevector_port
#define get_env_payload scmrt_get_env_payload
#define env_entry_payload scmrt_env_entry_payload
#define arg_error scmrt_arg_error
#define port_type scmrt_port_type
#endif

typedef uint64_t obj_t;

/* The $ac-convention macros' result cell, declared to MARV as
 * (data $scmi:ac_scratch ...): the $NAME_to destination macros
 * are the general form, and the plain $NAME macros delegate
 * with this cell as the destination, then reload $ac from it.
 * It is deliberately NOT a GC root: an hreserved call and the
 * reload that follows it are separated by no allocation, so
 * the cell never holds a live pointer at a collection -- it is
 * a register that happens to live in memory, nothing more.
 * Owning it here is what frees every caller's $r layout from
 * reserving a shadow cell: $r is pure root array again. */
obj_t ac_scratch[1];

/* static message fragment for $scmrt:arg_error, declared as
   (data $scmi:msg_not_pair ...) in scmi.mvh; storage lives here
   in the companion C file, like ac_scratch above */
const char msg_not_pair[] = "a pair";
const char msg_not_proc[] = "a procedure";
const char msg_not_nonzero[] = "a nonzero divisor";
const char msg_not_number[] = "a number";
const char msg_not_string[] = "a string";
const char msg_not_vector[] = "a vector";
const char msg_not_bytevector[] = "a bytevector";
const char msg_not_index[] = "an index in range";

#if !defined(__WCPL__)
/* $scmrt services (see above; renamed by the #define block) */
void *make_string_payload(int32_t len, int32_t fill);
void *make_string_payload_raw(int32_t len);
obj_t string_type(void);
void *make_bytevector_payload(int32_t len);
void *make_bytevector_payload_raw(int32_t len);
int32_t char_to_lower(int32_t c);
obj_t bytevector_type(void);
void *symbol_to_string(obj_t o);
void *number_to_string(obj_t o, obj_t radix);
void *open_input_file(obj_t o);
void *open_output_file(obj_t o);
void *try_open_input_file(obj_t o);
void *try_open_output_file(obj_t o);
void *arg_error(obj_t bad, const char *what);
void *make_input_string_port(obj_t o);
void *make_output_string_port(void);
void *get_output_string_payload(obj_t o);
void *port_read_line(obj_t o);
void *make_input_bytevector_port(obj_t o);
void *get_output_bytevector_payload(obj_t o);
void *make_output_bytevector_port(void);
void *get_env_payload(obj_t o);
void *env_entry_payload(int32_t i);
obj_t port_type(void);
#endif

#define SCM_FALSE ((obj_t)0)
#define SCM_TRUE  ((obj_t)1)

/* block header: (size<<3) | (mtag<<1) | 1; pairs are size 2, MTAG_PACKED */
#define SCM_PAIR_HDR ((obj_t)((2 << 3) | (1 << 1) | 1))

#define FIXNUM_MIN (-536870912LL)
#define FIXNUM_MAX (536870911LL)

/* ------------------------------------------------------------------ */
/* small helpers (mirroring lib/scmrt.c and Skint's n.h)               */

static int is_flonum(obj_t o)
{
  return (o & (obj_t)0xffff000000000000ULL) != 0;
}

static int is_fixnum(obj_t o)
{
  return (o & (obj_t)0xffffffff00000003ULL) == 3;   /* isim0 */
}

static obj_t mk_fixnum(int64_t v)
{
  return (obj_t)(((uint64_t)v & 0x3fffffffULL) << 2) | (obj_t)3;
}

static int64_t fx_val(obj_t o)
{
  return (int64_t)(((((int32_t)o >> 2) & 0x3fffffff) ^ 0x20000000)
                   - 0x20000000);
}

/* flonum <-> obj bit conversion; under WCPL the intrinsics from
   <sys/intrs.h> (pulled in by <math.h>) compile to single
   i64.reinterpret_f64 / f64.reinterpret_i64 instructions */
#if defined(__WCPL__)
static obj_t mk_flonum(double d)
{
  return ~__builtin_asuint64(d);
}

static double flo_val(obj_t o)
{
  return __builtin_asdouble(~o);
}
#else
static obj_t mk_flonum(double d)
{
  union { double d; uint64_t u; } cvt;
  cvt.d = d;
  return ~cvt.u;
}

static double flo_val(obj_t o)
{
  union { double d; uint64_t u; } cvt;
  cvt.u = ~o;
  return cvt.d;
}
#endif

/* a generic number (fixnum or flonum) as a double: each operand
   is dispatched on its own tag, so mixed fixnum/flonum pairs
   convert exactly (a 30-bit fixnum is always exact in a double) */
/* Every generic arithmetic slow path converts through here, so this is
   the one place a value that is not a number has to be caught.  It used
   to fall into flo_val, which reinterprets the bits as a double: that is
   why (+ 1 (quote a)) returned the symbol instead of reporting.  The
   fixnum fast paths in the macros never reach this, so the test costs
   nothing on the common path.  [cco] */
static double num_f64(obj_t o)
{
  if (is_fixnum(o)) return (double)fx_val(o);
  if (!is_flonum(o)) arg_error(o, msg_not_number);
  return flo_val(o);
}

static int fixnum_range(int64_t v)
{
  return v >= FIXNUM_MIN && v <= FIXNUM_MAX;
}

/* the macros trap on a fixnum zero divisor before calling, so a
   zero here would mean the caller bypassed the fast path */
static void zero_divisor(void)
{
  fprintf(stderr, "scmi: zero divisor\n");
  abort();
}

/* ------------------------------------------------------------------ */
/* generic binary arithmetic: a fixnum result iff both operands    */
/* are fixnums and the exact result fits, else a flonum            */

obj_t add_slow(obj_t a, obj_t b)
{
  if (is_fixnum(a) && is_fixnum(b)) {
    int64_t z = fx_val(a) + fx_val(b);
    if (fixnum_range(z)) return mk_fixnum(z);
  }
  return mk_flonum(num_f64(a) + num_f64(b));
}

obj_t sub_slow(obj_t a, obj_t b)
{
  if (is_fixnum(a) && is_fixnum(b)) {
    int64_t z = fx_val(a) - fx_val(b);
    if (fixnum_range(z)) return mk_fixnum(z);
  }
  return mk_flonum(num_f64(a) - num_f64(b));
}

obj_t mul_slow(obj_t a, obj_t b)
{
  if (is_fixnum(a) && is_fixnum(b)) {
    int64_t z = fx_val(a) * fx_val(b);
    if (fixnum_range(z)) return mk_fixnum(z);
  }
  return mk_flonum(num_f64(a) * num_f64(b));
}

/* generic / : a fixnum iff the fixnum quotient is exact and in
   range, else a flonum (as Skint's div, minus its fail() call) */
obj_t div_slow(obj_t a, obj_t b)
{
  if (is_fixnum(a) && is_fixnum(b)) {
    int64_t x = fx_val(a), y = fx_val(b), z;
    if (y == 0) zero_divisor();
    z = x / y;
    if (x % y == 0 && fixnum_range(z)) return mk_fixnum(z);
  }
  return mk_flonum(num_f64(a) / num_f64(b));
}

/* truncating quotient (R7RS quotient) */
obj_t quo_slow(obj_t a, obj_t b)
{
  if (is_fixnum(a) && is_fixnum(b)) {
    int64_t x = fx_val(a), y = fx_val(b), z;
    if (y == 0) zero_divisor();
    z = x / y;
    if (fixnum_range(z)) return mk_fixnum(z);
  }
  return mk_flonum(trunc(num_f64(a) / num_f64(b)));
}

/* truncating remainder (R7RS remainder, sign of the dividend) */
obj_t rem_slow(obj_t a, obj_t b)
{
  double x, y;
  if (is_fixnum(a) && is_fixnum(b)) {
    int64_t fx = fx_val(a), fy = fx_val(b);
    if (fy == 0) zero_divisor();
    return mk_fixnum(fx % fy);
  }
  x = num_f64(a); y = num_f64(b);
  return mk_flonum(x - trunc(x / y) * y);
}

/* floored remainder (R7RS modulo, sign of the divisor) */
obj_t mod_slow(obj_t a, obj_t b)
{
  double x, y, z;
  if (is_fixnum(a) && is_fixnum(b)) {
    int64_t fx = fx_val(a), fy = fx_val(b), fz;
    if (fy == 0) zero_divisor();
    fz = fx % fy;
    if (fz != 0 && ((fz < 0) != (fy < 0))) fz += fy;
    return mk_fixnum(fz);
  }
  x = num_f64(a); y = num_f64(b);
  z = x - trunc(x / y) * y;
  if (z != 0.0 && ((z < 0.0) != (y < 0.0))) z += y;
  return mk_flonum(z);
}

/* floored quotient: the truncating quotient adjusted down when a
   nonzero remainder's sign differs from the divisor's */
obj_t fquo_slow(obj_t a, obj_t b)
{
  if (is_fixnum(a) && is_fixnum(b)) {
    int64_t x = fx_val(a), y = fx_val(b), z;
    if (y == 0) zero_divisor();
    z = x / y;
    if (x % y != 0 && ((x % y < 0) != (y < 0))) z -= 1;
    if (fixnum_range(z)) return mk_fixnum(z);
  }
  return mk_flonum(floor(num_f64(a) / num_f64(b)));
}

/* ------------------------------------------------------------------ */
/* generic comparisons: two fixnums compare in the macros; here    */
/* each operand converts on its own tag (mixed pairs included)     */

obj_t lt_slow(obj_t a, obj_t b)
{
  return num_f64(a) < num_f64(b) ? SCM_TRUE : SCM_FALSE;
}

obj_t gt_slow(obj_t a, obj_t b)
{
  return num_f64(a) > num_f64(b) ? SCM_TRUE : SCM_FALSE;
}

obj_t le_slow(obj_t a, obj_t b)
{
  return num_f64(a) <= num_f64(b) ? SCM_TRUE : SCM_FALSE;
}

obj_t ge_slow(obj_t a, obj_t b)
{
  return num_f64(a) >= num_f64(b) ? SCM_TRUE : SCM_FALSE;
}

/* generic = and /= on numbers */
obj_t eq_slow(obj_t a, obj_t b)
{
  return num_f64(a) == num_f64(b) ? SCM_TRUE : SCM_FALSE;
}

obj_t ne_slow(obj_t a, obj_t b)
{
  return num_f64(a) != num_f64(b) ? SCM_TRUE : SCM_FALSE;
}

/* ------------------------------------------------------------------ */
/* generic min/max: when either operand is a flonum the result is  */
/* inexact (contagion, as in Skint)                                */

obj_t gmin_slow(obj_t a, obj_t b)
{
  obj_t r = num_f64(a) <= num_f64(b) ? a : b;
  if ((is_flonum(a) || is_flonum(b)) && is_fixnum(r))
    r = mk_flonum((double)fx_val(r));
  return r;
}

obj_t gmax_slow(obj_t a, obj_t b)
{
  obj_t r = num_f64(a) >= num_f64(b) ? a : b;
  if ((is_flonum(a) || is_flonum(b)) && is_fixnum(r))
    r = mk_flonum((double)fx_val(r));
  return r;
}

/* generic abs: fixnum abs (FIXNUM_MIN promotes), flonum abs */
obj_t gabs_slow(obj_t a)
{
  if (is_fixnum(a)) {
    int64_t v = fx_val(a);
    if (v == FIXNUM_MIN) return mk_flonum(536870912.0);
    return mk_fixnum(v < 0 ? -v : v);
  }
  return mk_flonum(fabs(flo_val(a)));
}

/* expt: an exact fixnum power that fits a fixnum stays exact
   (binary exponentiation; |factors| <= 2^29 keeps every partial
   product inside i64, and magnitudes grow monotonically, so the
   range check is exact); anything else goes through C pow and
   is a flonum (no bignums in this runtime) */
obj_t expt_slow(obj_t a, obj_t b)
{
  if (is_fixnum(a) && is_fixnum(b) && fx_val(b) >= 0) {
    int64_t base = fx_val(a), e = fx_val(b), z = 1;
    /* every factor in play is range-checked before it can be
       multiplied in, so partial products stay within 2^58 and
       never overflow i64; a factor out of fixnum range with
       exponent bits still set means the result is out of range
       too (integer magnitudes never cancel) */
    while (e > 0) {
      if (e & 1) {
        z *= base;
        if (!fixnum_range(z)) return mk_flonum(pow(num_f64(a), num_f64(b)));
      }
      e >>= 1;
      if (e > 0) {
        base *= base;
        if (!fixnum_range(base)) return mk_flonum(pow(num_f64(a), num_f64(b)));
      }
    }
    return mk_fixnum(z);
  }
  return mk_flonum(pow(num_f64(a), num_f64(b)));
}

/* exact: fixnum identity; an integral flonum in fixnum range
   becomes a fixnum, a larger integral flonum stays a flonum
   (its value is still exact below 2^53; this runtime has no
   bignums, and whole-mantissa conversion would be wrong) */
obj_t exact_slow(obj_t a)
{
  if (is_flonum(a)) {
    double d = flo_val(a), t = trunc(d);
    if (t == d && t >= -536870912.0 && t <= 536870911.0)
      return mk_fixnum((int64_t)t);
  }
  return a;
}

/* generic transcendentals (Skint's i.c sin/cos/... instructions):
   the argument is read as a double whatever its tag, the result is
   a flonum */
obj_t sin_slow(obj_t a)  { return mk_flonum(sin(num_f64(a))); }
obj_t cos_slow(obj_t a)  { return mk_flonum(cos(num_f64(a))); }
obj_t tan_slow(obj_t a)  { return mk_flonum(tan(num_f64(a))); }
obj_t asin_slow(obj_t a) { return mk_flonum(asin(num_f64(a))); }
obj_t acos_slow(obj_t a) { return mk_flonum(acos(num_f64(a))); }
obj_t atan_slow(obj_t a) { return mk_flonum(atan(num_f64(a))); }
obj_t exp_slow(obj_t a)  { return mk_flonum(exp(num_f64(a))); }
obj_t log_slow(obj_t a)  { return mk_flonum(log(num_f64(a))); }
obj_t sqrt_slow(obj_t a) { return mk_flonum(sqrt(num_f64(a))); }
obj_t atan2_slow(obj_t a, obj_t b)
{
  return mk_flonum(atan2(num_f64(a), num_f64(b)));
}

obj_t fxgcd_slow(obj_t a, obj_t b)
{
  /* Skint's walk: gcd(a, b) = gcd(b, a mod b) with floored mod,
     so the sign follows the last nonzero remainder (fxgcd -6 = -6) */
  int64_t x = fx_val(a), y = fx_val(b);
  while (y) {
    int64_t r = x % y;
    if (r != 0 && ((r < 0) != (y < 0))) r += y;
    x = y; y = r;
  }
  return mk_fixnum(x);
}

obj_t flgcd_slow(obj_t a, obj_t b)
{
  double x = fabs(num_f64(a)), y = fabs(num_f64(b));
  while (y != 0.0) { double t = fmod(x, y); x = y; y = t; }
  return mk_flonum(x);
}

obj_t fxexpt_slow(obj_t a, obj_t b)
{
  double r;
  if (fx_val(a) == 0 && fx_val(b) == 0) return mk_fixnum(0);
  r = pow((double)fx_val(a), (double)fx_val(b));
  if (r == trunc(r) && r >= -536870912.0 && r <= 536870911.0)
    return mk_fixnum((int64_t)r);
  return mk_fixnum(0);
}

obj_t log10_slow(obj_t a) { return mk_flonum(log10(num_f64(a))); }
obj_t sinh_slow(obj_t a) { return mk_flonum(sinh(num_f64(a))); }
obj_t cosh_slow(obj_t a) { return mk_flonum(cosh(num_f64(a))); }
obj_t tanh_slow(obj_t a) { return mk_flonum(tanh(num_f64(a))); }
obj_t ldexp_slow(obj_t a, obj_t b)
{
  return mk_flonum(ldexp(num_f64(a), (int)fx_val(b)));
}

/* box-writing fixnum/flonum builtins (Skint's set& operations):
   the second (or third) operand is a box object whose cell the
   operation fills; no heap allocation happens here.  The carry
   operations split an exact i64 total into a high part (returned)
   and a balanced low part in [-2^29, 2^29) (stored in the box),
   at the 30-bit fixnum width: total = high * 2^30 + low. */
static void box_set(obj_t b, obj_t v) { ((obj_t *)(uintptr_t)b)[0] = v; }

obj_t fxsqrt_slow(obj_t a)
{
  uint64_t v = (uint64_t)fx_val(a), r = (uint64_t)sqrt((double)v);
  while ((r + 1) * (r + 1) <= v) r++;
  while (r * r > v) r--;
  return mk_fixnum((int64_t)r);
}

obj_t fxsqrtb_slow(obj_t a, obj_t b)
{
  uint64_t v = (uint64_t)fx_val(a), r = (uint64_t)sqrt((double)v);
  while ((r + 1) * (r + 1) <= v) r++;
  while (r * r > v) r--;
  box_set(b, mk_fixnum((int64_t)(v - r * r)));
  return mk_fixnum((int64_t)r);
}

static int64_t carry_split(int64_t total, obj_t b)
{
  const int64_t mod = (int64_t)1 << 30, half = (int64_t)1 << 29;
  int64_t t = total + half, high = t / mod;
  if (t % mod != 0 && t < 0) high--;
  box_set(b, mk_fixnum(total - high * mod));
  return high;
}

obj_t fxaddc_slow(obj_t a, obj_t b, obj_t box)
{
  int64_t carry = fx_val(((obj_t *)(uintptr_t)box)[0]);
  return mk_fixnum(carry_split(fx_val(a) + fx_val(b) + carry, box));
}

obj_t fxsubc_slow(obj_t a, obj_t b, obj_t box)
{
  int64_t carry = fx_val(((obj_t *)(uintptr_t)box)[0]);
  return mk_fixnum(carry_split(fx_val(a) - fx_val(b) - carry, box));
}

obj_t fxmulc_slow(obj_t a, obj_t b, obj_t box)
{
  int64_t carry = fx_val(((obj_t *)(uintptr_t)box)[0]);
  return mk_fixnum(carry_split(fx_val(a) * fx_val(b) + carry, box));
}

obj_t flmodfb_slow(obj_t a, obj_t b)
{
  double ip;
  double fr = modf(flo_val(a), &ip);
  box_set(b, mk_flonum(ip));
  return mk_flonum(fr);
}

obj_t flfrexpb_slow(obj_t a, obj_t b)
{
  int e;
  double fr = frexp(flo_val(a), &e);
  box_set(b, mk_fixnum(e));
  return mk_flonum(fr);
}

/* ------------------------------------------------------------------ */
/* Allocating instructions: the xxx_slow_hreserved convention.     */
/*                                                                 */
/* An allocating instruction cannot live in plain C alone: it      */
/* needs the caller's heap pointer and root set.  The convention   */
/* is:                                                               */
/*                                                                 */
/*   - the caller has already reserved the instruction's whole     */
/*     heap requirement ($scmn:hreserve) with its live objects     */
/*     rooted, so NO collection can happen inside the call;        */
/*   - the first two arguments are a result-cell pointer and the   */
/*     current $hp; any instruction operands follow;               */
/*   - the function returns the updated $hp directly;              */
/*   - the result object is NOT returned -- it is placed in the    */
/*     cell the first argument points to.  The $NAME_to            */
/*     destination macros pass the address of whatever memory      */
/*     cell is to receive the result, and it lands there           */
/*     directly; the $ac-convention macros pass the library's      */
/*     own $scmi:ac_scratch cell (below) and reload $ac from       */
/*     it, so no caller layout reserves anything for them.         */
/*                                                                 */
/* Returning $hp (rather than the result) is the future-proof      */
/* direction: allocation state is one value, while results may     */
/* grow to several values later, all of which fit the cell-array   */
/* convention without changing these signatures.                   */

/* cons: car/cdr arrive as ordinary operands; the pair goes to
   the result cell.  Pre-reserved: 3 words (pair_bsz). */
void *cons_slow_hreserved(void *r, void *hp, obj_t car, obj_t cdr)
{
  obj_t *w = (obj_t *)(void *)((unsigned char *)hp - 3 * sizeof(obj_t));
  w[0] = SCM_PAIR_HDR;
  w[1] = car;
  w[2] = cdr;
  ((obj_t *)r)[0] = (obj_t)(uint64_t)(void *)(w + 1);
  return (void *)w;
}

/* ------------------------------------------------------------------ */
/* the remaining allocating instructions, same convention.  Block   */
/* access mirrors lib/scmn.mvh: an obj points at its first cell,    */
/* the header word is the cell just below it.  String payloads are  */
/* the scmrt strpay_t layout {int32 len; int32 chars[]} behind the  */
/* wrapper's cell 0; payload memory itself is malloc'd in $scmrt    */
/* (no GC allocation), only the 2-word wrapper comes from $hp.      */

#define SCM_BOX_HDR ((obj_t)((1 << 3) | (1 << 1) | 1))
#define SCM_NULL_OBJ ((obj_t)13)
/* imm itag 7 (see scmn.mvh): the end-of-file object */
#define SCM_EOF_OBJ ((obj_t)29)

/* the obj for a block whose first cell is w+1 goes to $r[0] */
static obj_t *hreserved_block(void *hp, int32_t words)
{
  return (obj_t *)(void *)((unsigned char *)hp
                           - (size_t)words * sizeof(obj_t));
}

static void hreserved_result(void *r, obj_t *w)
{
  ((obj_t *)r)[0] = (obj_t)(uint64_t)(void *)(w + 1);
}

/* one pair at the current hp; returns the new hp, pair in *out */
static void *hreserved_pair(void *hp, obj_t car, obj_t cdr, obj_t *out)
{
  obj_t *w = hreserved_block(hp, 3);
  w[0] = SCM_PAIR_HDR;
  w[1] = car;
  w[2] = cdr;
  *out = (obj_t)(uint64_t)(void *)(w + 1);
  return (void *)w;
}

static obj_t pair_car(obj_t o) { return ((obj_t *)(uintptr_t)o)[0]; }
static obj_t pair_cdr(obj_t o) { return ((obj_t *)(uintptr_t)o)[1]; }

static int32_t *string_payload(obj_t o)
{
  return (int32_t *)(uintptr_t)((obj_t *)(uintptr_t)o)[0];
}

/* a native wrapper (string, bytevector, port) around a malloc'd
   payload: 2 words, header is the payload kind's cxtype address */
static void *hreserved_wrap(void *r, void *hp, void *pay, obj_t type)
{
  obj_t *w = hreserved_block(hp, 2);
  w[0] = type;
  w[1] = (obj_t)(uintptr_t)pay;
  hreserved_result(r, w);
  return (void *)w;
}

/* box: Pre-reserved: 2 words (box_bsz). */
void *box_slow_hreserved(void *r, void *hp, obj_t v)
{
  obj_t *w = hreserved_block(hp, 2);
  w[0] = SCM_BOX_HDR;
  w[1] = v;
  hreserved_result(r, w);
  return (void *)w;
}

/* make-list: len (fixnum obj) and fill.  Pre-reserved: 3*len. */
void *lmk_slow_hreserved(void *r, void *hp, obj_t len, obj_t fill)
{
  int64_t n = fx_val(len), i;
  obj_t lst = SCM_NULL_OBJ;
  for (i = 0; i < n; i++)
    hp = hreserved_pair(hp, fill, lst, &lst);
  ((obj_t *)r)[0] = lst;
  return hp;
}

/* make-vector: len (fixnum obj) and fill.  Pre-reserved: len+1. */
void *vmk_slow_hreserved(void *r, void *hp, obj_t len, obj_t fill)
{
  int32_t n = (int32_t)fx_val(len), i;
  obj_t *w = hreserved_block(hp, n + 1);
  w[0] = (obj_t)(((uint64_t)n << 3) | 7);
  for (i = 0; i < n; i++) w[1 + i] = fill;
  hreserved_result(r, w);
  return (void *)w;
}

/* reverse: Pre-reserved: 3*length(list). */
void *lrev_slow_hreserved(void *r, void *hp, obj_t l)
{
  obj_t res = SCM_NULL_OBJ;
  while (l != SCM_NULL_OBJ) {
    obj_t p;
    hp = hreserved_pair(hp, pair_car(l), res, &p);
    res = p;
    l = pair_cdr(l);
  }
  ((obj_t *)r)[0] = res;
  return hp;
}

/* append (binary): copies the first list onto tail.
   Pre-reserved: 3*length(list). */
void *lcat_slow_hreserved(void *r, void *hp, obj_t l, obj_t tail)
{
  obj_t res = tail, prev = (obj_t)0;
  while (l != SCM_NULL_OBJ) {
    obj_t p;
    hp = hreserved_pair(hp, pair_car(l), tail, &p);
    if (prev == (obj_t)0) res = p;
    else ((obj_t *)(uintptr_t)prev)[1] = p;
    prev = p;
    l = pair_cdr(l);
  }
  ((obj_t *)r)[0] = res;
  return hp;
}

/* string->list: Pre-reserved: 3*length(string). */
void *slist_slow_hreserved(void *r, void *hp, obj_t s)
{
  int32_t *pay = string_payload(s);
  int32_t i = pay[0];
  obj_t res = SCM_NULL_OBJ;
  while (i > 0) {
    obj_t p;
    i--;
    hp = hreserved_pair(hp,
                        (obj_t)(((uint64_t)(uint32_t)pay[1 + i] << 8) | 9),
                        res, &p);
    res = p;
  }
  ((obj_t *)r)[0] = res;
  return hp;
}

/* bytevector->list: the wrapper's payload is an int32 length then
   the bytes, the same shape a string's payload has, so the string
   accessor serves here too.  Built back to front, like slist, so
   each pair's cdr is already in hand.  Pre-reserved: 3*length.
   [cco] */
void *bvlist_slow_hreserved(void *r, void *hp, obj_t b)
{
  int32_t *pay = string_payload(b);
  int32_t i = pay[0];
  unsigned char *bytes = (unsigned char *)(void *)(pay + 1);
  obj_t res = SCM_NULL_OBJ;
  while (i > 0) {
    obj_t p;
    i--;
    hp = hreserved_pair(hp, mk_fixnum((int64_t)bytes[i]), res, &p);
    res = p;
  }
  ((obj_t *)r)[0] = res;
  return hp;
}

/* make-string: len and fill char objs.  Pre-reserved: 2. */
void *smk_slow_hreserved(void *r, void *hp, obj_t len, obj_t fill)
{
  void *pay = make_string_payload((int32_t)fx_val(len),
                                  (int32_t)(fill >> 8));
  return hreserved_wrap(r, hp, pay, string_type());
}

/* substring: string, start and end (fixnum objs).  Pre-reserved: 2. */
void *ssub_slow_hreserved(void *r, void *hp, obj_t s, obj_t b, obj_t e)
{
  int32_t *src = string_payload(s);
  int32_t bb = (int32_t)fx_val(b), n = (int32_t)(fx_val(e) - fx_val(b));
  int32_t *pay = (int32_t *)make_string_payload_raw(n);
  if (n > 0) memcpy(pay + 1, src + 1 + bb, (size_t)n * 4);
  return hreserved_wrap(r, hp, (void *)pay, string_type());
}

/* string-append (binary).  Pre-reserved: 2. */
void *scat_slow_hreserved(void *r, void *hp, obj_t a, obj_t b)
{
  int32_t *pa = string_payload(a), *pb = string_payload(b);
  int32_t n = pa[0], m = pb[0];
  int32_t *pay = (int32_t *)make_string_payload_raw(n + m);
  if (n > 0) memcpy(pay + 1, pa + 1, (size_t)n * 4);
  if (m > 0) memcpy(pay + 1 + n, pb + 1, (size_t)m * 4);
  return hreserved_wrap(r, hp, (void *)pay, string_type());
}

/* n-ary string-append over a run of staged slots, after Skint's `sapp`
   instruction (i.c), which takes a count and a pointer to n strings and
   calls stringrcat: ONE pass to sum the lengths, ONE allocation, and
   each part copied ONCE.
   The compiler used to fold an n-ary append into n-1 pairwise scats,
   each copying the accumulated prefix, which is quadratic in the result
   -- the `string' benchmark does (string-append "123" s "456" s "789")
   with s growing into the hundreds of KB, so it copied the prefix four
   times per call and ran at 0.04x the interpreter.
   The slots are consecutive because ds-prim-args stages them that way,
   which is the same shape sapp wants of the Scheme stack.  Pre-reserved:
   2, as for scat. [cco] */
void *sappn_slow_hreserved(void *r, void *hp, void *slots, int32_t n)
{
  obj_t *sv = (obj_t *)slots;
  int32_t total = 0, at = 0, i;
  int32_t *pay;
  for (i = 0; i < n; i++) total += string_payload(sv[i])[0];
  pay = (int32_t *)make_string_payload_raw(total);
  for (i = 0; i < n; i++) {
    int32_t *ps = string_payload(sv[i]);
    int32_t m = ps[0];
    if (m > 0) memcpy(pay + 1 + at, ps + 1, (size_t)m * 4);
    at += m;
  }
  return hreserved_wrap(r, hp, (void *)pay, string_type());
}

/* list->string.  Pre-reserved: 2. */
void *lstr_slow_hreserved(void *r, void *hp, obj_t l)
{
  obj_t t = l;
  int32_t n = 0, i;
  int32_t *pay;
  while (t != SCM_NULL_OBJ) { n++; t = pair_cdr(t); }
  pay = (int32_t *)make_string_payload_raw(n);
  for (i = 0; i < n; i++) {
    pay[1 + i] = (int32_t)(pair_car(l) >> 8);
    l = pair_cdr(l);
  }
  return hreserved_wrap(r, hp, (void *)pay, string_type());
}

/* string-downcase.  Pre-reserved: 2. */
void *sdwn_slow_hreserved(void *r, void *hp, obj_t s)
{
  int32_t *src = string_payload(s);
  int32_t n = src[0], i;
  int32_t *pay = (int32_t *)make_string_payload_raw(n);
  /* the Unicode simple lowercase mapping, not an ASCII fold [cco] */
  for (i = 0; i < n; i++) pay[1 + i] = char_to_lower(src[1 + i]);
  return hreserved_wrap(r, hp, (void *)pay, string_type());
}

/* subutf8->string and substring->utf8, after Skint's s8tos and ssto8
   instructions.  The clib did both in Scheme, which is why `bv2string'
   -- which converts a 500-byte string three times per iteration -- ran
   at 0.39x the interpreter: it executed a compiled loop per BYTE
   against two C instructions.  Those two Scheme definitions counted,
   allocated and filled in exactly the shape below, so this is the same
   algorithm in C rather than a different one.

   The decode is unvalidated, which is what it replaces and what Skint
   does: its own udecode (n_unicode.c) is documented as detecting no
   errors whatsoever at all, and the clib classified a lead byte the
   same way, by the three thresholds 128/224/240.  The one difference
   is the bound below: a sequence truncated at the end index stops
   there instead of reading past it.  Pre-reserved: 2. [cco] */
void *s8tos_slow_hreserved(void *r, void *hp, obj_t b, obj_t s, obj_t e)
{
  const unsigned char *src =
    (const unsigned char *)(void *)(string_payload(b) + 1);
  int32_t bb = (int32_t)fx_val(s), ee = (int32_t)fx_val(e);
  int32_t i, n = 0, len;
  int32_t *pay;
  for (i = bb; i < ee; ) {
    uint32_t c = src[i];
    i += (c < 128) ? 1 : (c < 224) ? 2 : (c < 240) ? 3 : 4;
    n++;
  }
  pay = (int32_t *)make_string_payload_raw(n);
  i = bb;
  for (len = 0; len < n && i < ee; len++) {
    uint32_t c = src[i++];
    if (c < 128) {
      pay[1 + len] = (int32_t)c;
    } else {
      /* the lead byte's own bits, then six from each continuation
         byte -- the masks Skint's udecode uses, which agree with the
         subtractions the Scheme did on every well-formed sequence */
      int32_t k = (c < 224) ? 1 : (c < 240) ? 2 : 3;
      uint32_t v = (c < 224) ? (c & 0x1f) : (c < 240) ? (c & 0x0f) : (c & 7);
      while (k-- > 0 && i < ee) v = (v << 6) | (src[i++] & 0x3f);
      pay[1 + len] = (int32_t)v;
    }
  }
  pay[0] = len;                 /* equals n unless the input was bad */
  return hreserved_wrap(r, hp, (void *)pay, string_type());
}

void *ssto8_slow_hreserved(void *r, void *hp, obj_t s, obj_t b, obj_t e)
{
  int32_t *src = string_payload(s);
  int32_t bb = (int32_t)fx_val(b), ee = (int32_t)fx_val(e);
  int32_t i, total = 0, at = 0;
  int32_t *raw;
  unsigned char *pay;
  for (i = bb; i < ee; i++) {
    uint32_t c = (uint32_t)src[1 + i];
    total += (c < 0x80) ? 1 : (c < 0x800) ? 2 : (c < 0x10000) ? 3 : 4;
  }
  raw = (int32_t *)make_bytevector_payload_raw(total);
  pay = (unsigned char *)(void *)(raw + 1);  /* int32 length, then bytes */
  for (i = bb; i < ee; i++) {
    uint32_t c = (uint32_t)src[1 + i];
    if (c < 0x80) {
      pay[at++] = (unsigned char)c;
    } else if (c < 0x800) {
      pay[at++] = (unsigned char)(0xc0 | (c >> 6));
      pay[at++] = (unsigned char)(0x80 | (c & 63));
    } else if (c < 0x10000) {
      pay[at++] = (unsigned char)(0xe0 | (c >> 12));
      pay[at++] = (unsigned char)(0x80 | ((c >> 6) & 63));
      pay[at++] = (unsigned char)(0x80 | (c & 63));
    } else {
      pay[at++] = (unsigned char)(0xf0 | (c >> 18));
      pay[at++] = (unsigned char)(0x80 | ((c >> 12) & 63));
      pay[at++] = (unsigned char)(0x80 | ((c >> 6) & 63));
      pay[at++] = (unsigned char)(0x80 | (c & 63));
    }
  }
  return hreserved_wrap(r, hp, (void *)raw, bytevector_type());
}

/* make-bytevector: len (fixnum obj), zeroed payload.
   Pre-reserved: 2. */
void *bvnew_slow_hreserved(void *r, void *hp, obj_t len)
{
  void *pay = make_bytevector_payload((int32_t)fx_val(len));
  return hreserved_wrap(r, hp, pay, bytevector_type());
}

/* symbol->string.  Pre-reserved: 2. */
void *symstr_slow_hreserved(void *r, void *hp, obj_t sym)
{
  return hreserved_wrap(r, hp, symbol_to_string(sym), string_type());
}

/* number->string: number and radix (fixnum) objs.  Pre-reserved: 2. */
void *n2s_slow_hreserved(void *r, void *hp, obj_t num, obj_t radix)
{
  return hreserved_wrap(r, hp, number_to_string(num, radix),
                        string_type());
}

/* open-input-file / open-output-file.  Pre-reserved: 2. */
void *openin_slow_hreserved(void *r, void *hp, obj_t name)
{
  return hreserved_wrap(r, hp, open_input_file(name), port_type());
}

void *openout_slow_hreserved(void *r, void *hp, obj_t name)
{
  return hreserved_wrap(r, hp, open_output_file(name), port_type());
}

/* %open-input-file / %open-output-file: #f instead of a wrapper
   when the file cannot be opened (no allocation then). */
void *openin2_slow_hreserved(void *r, void *hp, obj_t name)
{
  void *p = try_open_input_file(name);
  if (!p) { ((obj_t *)r)[0] = SCM_FALSE; return hp; }
  return hreserved_wrap(r, hp, p, port_type());
}

void *openout2_slow_hreserved(void *r, void *hp, obj_t name)
{
  void *p = try_open_output_file(name);
  if (!p) { ((obj_t *)r)[0] = SCM_FALSE; return hp; }
  return hreserved_wrap(r, hp, p, port_type());
}

/* open-input-string.  Pre-reserved: 2. */
void *sinport_slow_hreserved(void *r, void *hp, obj_t s)
{
  return hreserved_wrap(r, hp, make_input_string_port(s), port_type());
}

/* open-output-string.  Pre-reserved: 2. */
void *soutport_slow_hreserved(void *r, void *hp)
{
  return hreserved_wrap(r, hp, make_output_string_port(), port_type());
}

/* get-output-string.  Pre-reserved: 2. */
void *getostr_slow_hreserved(void *r, void *hp, obj_t port)
{
  return hreserved_wrap(r, hp, get_output_string_payload(port),
                        string_type());
}

/* The bytevector port trio, each a 2-word wrapper like the string
   trio above.  An output bytevector port IS a string output port: a
   byte is a codepoint below 256, so write-u8 reaches it through
   write-char with no byte-specific path, and only the narrowing at
   get-output-bytevector is particular to bytes.  [cco] */
void *binport_slow_hreserved(void *r, void *hp, obj_t bv)
{
  return hreserved_wrap(r, hp, make_input_bytevector_port(bv), port_type());
}

void *boutport_slow_hreserved(void *r, void *hp)
{
  return hreserved_wrap(r, hp, make_output_bytevector_port(), port_type());
}

void *getbvo_slow_hreserved(void *r, void *hp, obj_t port)
{
  return hreserved_wrap(r, hp, get_output_bytevector_payload(port),
                        bytevector_type());
}

/* get-environment-variable: #f instead of a wrapper when the
   variable is unset (no allocation then).  Pre-reserved: 2. */

/* read-line: one runtime call for the whole line.  Pre-reserved: 2
   (the wrapper), which is given back untouched at end of file --
   nothing is allocated then. [cco] */
void *rdline_slow_hreserved(void *r, void *hp, obj_t port)
{
  void *p = port_read_line(port);
  if (!p) { ((obj_t *)r)[0] = SCM_EOF_OBJ; return hp; }
  return hreserved_wrap(r, hp, p, string_type());
}

/* string: n characters, read from a consecutive run of staged slots.
   The compiler used to cons a list of the characters and call
   list->string, which threw away 3n words per call; this allocates the
   payload once and fills it.  Same shape as sappn above. [cco]
   Pre-reserved: 2 (the wrapper). */
void *strn_slow_hreserved(void *r, void *hp, void *slots, int32_t n)
{
  obj_t *sv = (obj_t *)slots;
  int32_t *pay = (int32_t *)make_string_payload_raw(n);
  int32_t i;
  for (i = 0; i < n; i++) pay[1 + i] = (int32_t)(sv[i] >> 8);
  return hreserved_wrap(r, hp, (void *)pay, string_type());
}

void *getenv_slow_hreserved(void *r, void *hp, obj_t name)
{
  void *p = get_env_payload(name);
  if (!p) { ((obj_t *)r)[0] = SCM_FALSE; return hp; }
  return hreserved_wrap(r, hp, p, string_type());
}

/* %envv-ref: the i-th environment entry as "KEY=VALUE", or #f past the
   end.  Pre-reserved: 2. */
void *envvref_slow_hreserved(void *r, void *hp, obj_t idx)
{
  void *p = env_entry_payload((int32_t)fx_val(idx));
  if (!p) { ((obj_t *)r)[0] = SCM_FALSE; return hp; }
  return hreserved_wrap(r, hp, p, string_type());
}
