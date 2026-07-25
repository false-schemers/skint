/* i_tower.c -- generic math instructions for full numerical tower */

define_instruction(nump) {
  ac = bool_obj(is_number(ac));
  gonexti(); 
}

define_instruction(realp) {
  if (likely(is_fixnum(ac) || is_flonum(ac) || is_bignum(ac))) {
    ac = bool_obj(1);
  } else if (likely(is_fatnum(ac))) {
    ac = bool_obj(fnisreal(get_fatnum(ac)));
  } else {
    ac = bool_obj(0);
  }
  gonexti(); 
}

define_instruction(ratp) {
  if (likely(is_fixnum(ac) || is_bignum(ac))) {
    ac = bool_obj(1);
  } else if (likely(is_flonum(ac))) {
    double f = get_flonum(ac);
    ac = bool_obj(f > -HUGE_VAL && f < HUGE_VAL);
  } else if (likely(is_fatnum(ac))) {
    ac = bool_obj(fnisrat(get_fatnum(ac)));
  } else {
    ac = bool_obj(0);
  }
  gonexti(); 
}

define_instruction(intp) {
  if (likely(is_fixnum(ac) || is_bignum(ac))) {
    ac = bool_obj(1);
  } else if (likely(is_flonum(ac))) {
    ac = bool_obj(flisint(get_flonum(ac)));
  } else if (likely(is_fatnum(ac))) {
    ac = bool_obj(fnisint(get_fatnum(ac)));
  } else { /* accepts any object! */
    ac = bool_obj(0);
  }
  gonexti(); 
}

define_instruction(exip) {
  if (likely(is_fixnum(ac) || is_bignum(ac))) {
    ac = bool_obj(1);
  } else { 
    ac = bool_obj(0);
  }
  gonexti(); 
}

define_instruction(nanp) {
  if (likely(is_flonum(ac))) {
    double f = get_flonum(ac);
    ac = bool_obj(f != f);
  } else if (likely(is_fixnum(ac) || is_bignum(ac) || is_fatnum(ac))) {
    ac = bool_obj(0);
  } else failactype("number");
  gonexti(); 
}

define_instruction(finp) {
  if (likely(is_flonum(ac))) {
    double f = get_flonum(ac);
    ac = bool_obj(f > -HUGE_VAL && f < HUGE_VAL);
  } else if (likely(is_fixnum(ac) || is_bignum(ac) || is_fatnum(ac))) {
    ac = bool_obj(1);
  } else failactype("number");
  gonexti(); 
}

define_instruction(infp) {
  if (likely(is_flonum(ac))) {
    double f = get_flonum(ac);
    ac = bool_obj(f <= -HUGE_VAL || f >= HUGE_VAL);
  } else if (likely(is_fixnum(ac) || is_bignum(ac) || is_fatnum(ac))) {
    ac = bool_obj(0);
  } else failactype("number");
  gonexti(); 
}

define_instruction(zerop) {
  obj x = ac;
  if (likely(is_fixnum(x))) {
    ac = bool_obj(x == fixnum_obj(0));
  } else if (likely(is_flonum(x))) {
    ac = bool_obj(get_flonum(x) == 0.0);
  } else if (likely(is_bignum(ac))) {
    ac = bool_obj(0); /* normalization */
  } else if (likely(is_fatnum(ac))) {
    ac = bool_obj(fnzero(get_fatnum(ac)));
  } else failactype("number");
  gonexti();
}

define_instruction(posp) {
  if (likely(is_fixnum(ac))) ac = bool_obj(get_fixnum(ac) > 0);
  else if (likely(is_flonum(ac))) ac = bool_obj(get_flonum(ac) > 0.0);
  else if (likely(is_bignum(ac))) ac = bool_obj(bnsign(get_bignum(ac)) > 0.0);
  else if (likely(is_fatnum(ac) && fnisreal(get_fatnum(ac)))) 
    ac = bool_obj(fnispos(get_fatnum(ac)));
  else failactype("real number");
  gonexti(); 
}

define_instruction(negp) {
  if (likely(is_fixnum(ac))) ac = bool_obj(get_fixnum(ac) < 0);
  else if (likely(is_flonum(ac))) ac = bool_obj(get_flonum(ac) < 0.0);
  else if (likely(is_bignum(ac))) ac = bool_obj(bnsign(get_bignum(ac)) < 0.0);
  else if (likely(is_fatnum(ac) && fnisreal(get_fatnum(ac)))) 
    ac = bool_obj(fnisneg(get_fatnum(ac)));
  else failactype("real number");
  gonexti(); 
}

define_instruction(evnp) {
  if (likely(is_fixnum(ac))) {
    ac = bool_obj((get_fixnum(ac) & 1) == 0);
  } else if (likely(is_flonum(ac))) {
    double f = get_flonum(ac);
    ac = bool_obj(flisint(f / 2.0));
  } else if (likely(is_bignum(ac))) {
    ac = bool_obj(bneven(get_bignum(ac)));
  } else if (likely(is_fatnum(ac)) && fnisint(get_fatnum(ac))) {
    ac = bool_obj(fneven(get_fatnum(ac)));
  } else failactype("integer");
  gonexti(); 
}

define_instruction(oddp) {
  if (likely(is_fixnum(ac))) {
    ac = bool_obj((get_fixnum(ac) & 1) != 0);
  } else if (likely(is_flonum(ac))) {
    double f = get_flonum(ac);
    ac = bool_obj(flisint((f + 1.0) / 2.0));
  } else if (likely(is_bignum(ac))) {
    ac = bool_obj(bnodd(get_bignum(ac)));
  } else if (likely(is_fatnum(ac)) && fnisint(get_fatnum(ac))) {
    ac = bool_obj(fnodd(get_fatnum(ac)));
  } else failactype("integer");
  gonexti(); 
}

define_instrhelper(tower_unary) {
  fatnum4r_t z4; fatnum4_t x4; fatnum_t *fx = NULL;
  int (*fnop)(fatnum4r_t *, const fatnum_t *) =
  (int (*)(fatnum4r_t *, const fatnum_t *)) ac;
  obj x = spop();
  if (is_fixnum(x)) { x4.t = NUMT_FIX; x4.p[0].fix = get_fixnum(x); fx = (fatnum_t*)&x4; } 
  else if (is_flonum(x)) { x4.t = NUMT_FLO; x4.p[0].flo = get_flonum(x); fx = (fatnum_t*)&x4; } 
  else if (is_bignum(x)) { x4.t = NUMT_BIG; x4.p[0].big = get_bignum(x); fx = (fatnum_t*)&x4; } 
  else if (is_fatnum(x)) { fx = get_fatnum(x); } 
  if (!fx) failtype(x, "number");
  if (fnop(&z4, fx)) {
    switch (z4.t) {
      case NUMT_NONE: assert(0); break; 
      case NUMT_FIX: ac = fixnum_obj(z4.u.p[0].fix); break;
      case NUMT_FLO: ac = flonum_obj(z4.u.p[0].flo); break;
      case NUMT_BIG: ac = bignum_obj(z4.u.p[0].big); break;
      default: ac = fatnum_obj(dupfatnum((fatnum_t*)&z4));
    }
    gonexti();
  } else {
    fail(z4.u.msg);
  }
}

define_instrhelper(tower_binary) {
  fatnum4r_t z4; fatnum4_t x4, y4; fatnum_t *fx = NULL, *fy = NULL;
  int (*fnop)(fatnum4r_t *, const fatnum_t *, const fatnum_t *) =
  (int (*)(fatnum4r_t *, const fatnum_t *, const fatnum_t *)) ac;
  obj y = spop(), x = spop();
  if (is_fixnum(x)) { x4.t = NUMT_FIX; x4.p[0].fix = get_fixnum(x); fx = (fatnum_t*)&x4; } 
  else if (is_flonum(x)) { x4.t = NUMT_FLO; x4.p[0].flo = get_flonum(x); fx = (fatnum_t*)&x4; } 
  else if (is_bignum(x)) { x4.t = NUMT_BIG; x4.p[0].big = get_bignum(x); fx = (fatnum_t*)&x4; } 
  else if (is_fatnum(x)) { fx = get_fatnum(x); } 
  if (is_fixnum(y)) { y4.t = NUMT_FIX; y4.p[0].fix = get_fixnum(y); fy = (fatnum_t*)&y4; } 
  else if (is_flonum(y)) { y4.t = NUMT_FLO; y4.p[0].flo = get_flonum(y); fy = (fatnum_t*)&y4; } 
  else if (is_bignum(y)) { y4.t = NUMT_BIG; y4.p[0].big = get_bignum(y); fy = (fatnum_t*)&y4; } 
  else if (is_fatnum(y)) { fy = get_fatnum(y); } 
  if (!fx) failtype(x, "number");
  if (!fy) failtype(y, "number");
  if (fnop(&z4, fx, fy)) {
    switch (z4.t) {
      case NUMT_NONE: assert(0); break; 
      case NUMT_FIX: ac = fixnum_obj(z4.u.p[0].fix); break;
      case NUMT_FLO: ac = flonum_obj(z4.u.p[0].flo); break;
      case NUMT_BIG: ac = bignum_obj(z4.u.p[0].big); break;
      default: ac = fatnum_obj(dupfatnum((fatnum_t*)&z4));
    }
    gonexti();
  } else {
    fail(z4.u.msg);
  }
}

define_instruction(add) {
  obj x = ac, y = spop();
  if (likely(are_fixnums(x, y))) {
    long lx = get_fixnum(x), ly = get_fixnum(y);   
    long long llz = (long long)lx + (long long)ly;
    if (likely(llz >= FIXNUM_MIN && llz <= FIXNUM_MAX)) ac = fixnum_obj((long)llz);
    else { spush(x); spush(y); ac = (obj)&fnadd; goih(tower_binary); }
  } else {
    double dx, dy;
    if (likely(is_flonum(x))) dx = get_flonum(x);
    else if (likely(is_fixnum(x))) dx = (double)get_fixnum(x);
    else { spush(x); spush(y); ac = (obj)&fnadd; goih(tower_binary); }
    if (likely(is_flonum(y))) dy = get_flonum(y);
    else if (likely(is_fixnum(y))) dy = (double)get_fixnum(y);
    else { spush(x); spush(y); ac = (obj)&fnadd; goih(tower_binary); }
    ac = flonum_obj(dx + dy);
  }
  gonexti(); 
}

define_instruction(sub) {
  obj x = ac, y = spop();
  if (likely(are_fixnums(x, y))) {
    long lx = get_fixnum(x), ly = get_fixnum(y);   
    long long llz = (long long)lx - (long long)ly;
    if (likely(llz >= FIXNUM_MIN && llz <= FIXNUM_MAX)) ac = fixnum_obj((long)llz);
    else { spush(x); spush(y); ac = (obj)&fnsub; goih(tower_binary); }
  } else {
    double dx, dy;
    if (likely(is_flonum(x))) dx = get_flonum(x);
    else if (likely(is_fixnum(x))) dx = (double)get_fixnum(x);
    else { spush(x); spush(y); ac = (obj)&fnsub; goih(tower_binary); }
    if (likely(is_flonum(y))) dy = get_flonum(y);
    else if (likely(is_fixnum(y))) dy = (double)get_fixnum(y);
    else { spush(x); spush(y); ac = (obj)&fnsub; goih(tower_binary); }
    ac = flonum_obj(dx - dy);
  }
  gonexti(); 
}

define_instruction(mul) {
  obj x = ac, y = spop();
  if (likely(are_fixnums(x, y))) {
    long lx = get_fixnum(x), ly = get_fixnum(y);   
    long long llz = (long long)lx * (long long)ly;
    if (likely(llz >= FIXNUM_MIN && llz <= FIXNUM_MAX)) ac = fixnum_obj((long)llz);
    else { spush(x); spush(y); ac = (obj)&fnmul; goih(tower_binary); }
  } else {
    double dx, dy;
    if (likely(is_flonum(x))) dx = get_flonum(x);
    else if (likely(is_fixnum(x))) dx = (double)get_fixnum(x);
    else { spush(x); spush(y); ac = (obj)&fnmul; goih(tower_binary); }
    if (likely(is_flonum(y))) dy = get_flonum(y);
    else if (likely(is_fixnum(y))) dy = (double)get_fixnum(y);
    else { spush(x); spush(y); ac = (obj)&fnmul; goih(tower_binary); }
    ac = flonum_obj(dx * dy);
  }
  gonexti(); 
}

define_instruction(div) {
  obj x = ac, y = spop();
  if (likely(are_fixnums(x, y))) {
    long lx, ly; long long llz, llr;
    if (unlikely(y == fixnum_obj(0))) fail("division by zero");
    lx = get_fixnum(x), ly = get_fixnum(y);   
    llz = (long long)lx / (long long)ly, llr = (long long)lx % (long long)ly;
    if (likely(!llr && llz >= FIXNUM_MIN && llz <= FIXNUM_MAX)) ac = fixnum_obj((long)llz);
    else { spush(x); spush(y); ac = (obj)&fndiv; goih(tower_binary); }
  } else {
    double dx, dy;
    if (likely(is_flonum(x))) dx = get_flonum(x);
    else if (likely(is_fixnum(x))) dx = (double)get_fixnum(x);
    else { spush(x); spush(y); ac = (obj)&fndiv; goih(tower_binary); }
    if (likely(is_flonum(y))) dy = get_flonum(y);
    else if (likely(is_fixnum(y))) dy = (double)get_fixnum(y);
    else { spush(x); spush(y); ac = (obj)&fndiv; goih(tower_binary); }
    ac = flonum_obj(dx / dy);
  }
  gonexti(); 
}

define_instruction(quo) {
  obj x = ac, y = spop();
  if (likely(are_fixnums(x, y))) {
    if (unlikely(y == fixnum_obj(0))) fail("division by zero");
    if (unlikely(x == fixnum_obj(FIXNUM_MIN) && y == fixnum_obj(-1)))
      { spush(x); spush(y); ac = (obj)&fntquo; goih(tower_binary); }
    ac = fixnum_obj(fxquo(get_fixnum(x), get_fixnum(y)));
  } else {
    double dx, dy, dz;
    if (likely(is_flonum(x) && flisint(dx = get_flonum(x)))) /* ok */;
    else if (likely(is_fixnum(x))) dx = (double)get_fixnum(x);
    else { spush(x); spush(y); ac = (obj)&fntquo; goih(tower_binary); }
    if (likely(is_flonum(y) && flisint(dy = get_flonum(y)))) /* ok */;
    else if (likely(is_fixnum(y))) dy = (double)get_fixnum(y);
    else { spush(x); spush(y); ac = (obj)&fntquo; goih(tower_binary); }
    modf(dx / dy,  &dz);
    ac = flonum_obj(dz);
  }
  gonexti(); 
}

define_instruction(rem) {
  obj x = ac, y = spop();
  if (likely(are_fixnums(x, y))) {
    if (unlikely(y == fixnum_obj(0))) fail("division by zero");
    ac = fixnum_obj(fxrem(get_fixnum(x), get_fixnum(y)));
  } else {
    double dx, dy, dz;
    if (likely(is_flonum(x) && flisint(dx = get_flonum(x)))) /* ok */;
    else if (likely(is_fixnum(x))) dx = (double)get_fixnum(x);
    else { spush(x); spush(y); ac = (obj)&fntrem; goih(tower_binary); }
    if (likely(is_flonum(y) && flisint(dy = get_flonum(y)))) /* ok */;
    else if (likely(is_fixnum(y))) dy = (double)get_fixnum(y);
    else { spush(x); spush(y); ac = (obj)&fntrem; goih(tower_binary); }
    dz = fmod(dx, dy);
    /* keep zero positive: (remainder -10.0 2.0) => 0.0, not -0.0 */
    ac = flonum_obj((dz == 0.0) ? 0.0 : dz);
  }
  gonexti(); 
}

define_instruction(mqu) {
  obj x = ac, y = spop();
  if (likely(are_fixnums(x, y))) {
    if (unlikely(y == fixnum_obj(0))) fail("division by zero");
    if (unlikely(x == fixnum_obj(FIXNUM_MIN) && y == fixnum_obj(-1)))
     { spush(x); spush(y); ac = (obj)&fnfquo; goih(tower_binary); }
    ac = fixnum_obj(fxmqu(get_fixnum(x), get_fixnum(y)));
  } else {
    double dx, dy;
    if (likely(is_flonum(x))) dx = get_flonum(x);
    else if (likely(is_fixnum(x))) dx = (double)get_fixnum(x);
    else { spush(x); spush(y); ac = (obj)&fnfquo; goih(tower_binary); }
    if (likely(is_flonum(y))) dy = get_flonum(y);
    else if (likely(is_fixnum(y))) dy = (double)get_fixnum(y);
    else { spush(x); spush(y); ac = (obj)&fnfquo; goih(tower_binary); }
    ac = flonum_obj(flmqu(dx, dy));
  }
  gonexti(); 
}

define_instruction(mlo) {
  obj x = ac, y = spop();
  if (likely(are_fixnums(x, y))) {
    if (unlikely(y == fixnum_obj(0))) fail("division by zero");
    ac = fixnum_obj(fxmlo(get_fixnum(x), get_fixnum(y)));
  } else {
    double dx, dy;
    if (likely(is_flonum(x))) dx = get_flonum(x);
    else if (likely(is_fixnum(x))) dx = (double)get_fixnum(x);
    else { spush(x); spush(y); ac = (obj)&fnfrem; goih(tower_binary); }
    if (likely(is_flonum(y))) dy = get_flonum(y);
    else if (likely(is_fixnum(y))) dy = (double)get_fixnum(y);
    else { spush(x); spush(y); ac = (obj)&fnfrem; goih(tower_binary); }
    ac = flonum_obj(flmlo(dx, dy));
  }
  gonexti(); 
}

define_instrhelper(tower_cmp) {
  fatnum4_t x4, y4; fatnum_t *fx = NULL, *fy = NULL;
  ncmp_t ncmp = (ncmp_t)get_fixnum(ac); int res;
  obj y = spop(), x = spop();
  if (is_fixnum(x)) { x4.t = NUMT_FIX; x4.p[0].fix = get_fixnum(x); fx = (fatnum_t*)&x4; } 
  else if (is_flonum(x)) { x4.t = NUMT_FLO; x4.p[0].flo = get_flonum(x); fx = (fatnum_t*)&x4; } 
  else if (is_bignum(x)) { x4.t = NUMT_BIG; x4.p[0].big = get_bignum(x); fx = (fatnum_t*)&x4; } 
  else if (is_fatnum(x)) { fx = get_fatnum(x); if (!fnisreal(fx)) fx = NULL; } 
  if (is_fixnum(y)) { y4.t = NUMT_FIX; y4.p[0].fix = get_fixnum(y); fy = (fatnum_t*)&y4; } 
  else if (is_flonum(y)) { y4.t = NUMT_FLO; y4.p[0].flo = get_flonum(y); fy = (fatnum_t*)&y4; } 
  else if (is_bignum(y)) { y4.t = NUMT_BIG; y4.p[0].big = get_bignum(y); fy = (fatnum_t*)&y4; } 
  else if (is_fatnum(y)) { fy = get_fatnum(y); if (!fnisreal(fy)) fy = NULL; } 
  if (!fx) failtype(x, "real number");
  if (!fy) failtype(y, "real number");
  res = fncmp(fx, fy, ncmp); assert(!res || res == 1);
  ac = bool_obj(res);
  gonexti();
}

define_instruction(lt) {
  obj x = ac, y = spop();
  if (likely(are_fixnums(x, y))) {
    ac = bool_obj(get_fixnum(x) < get_fixnum(y));
  } else {
    double dx, dy;
    if (likely(is_flonum(x))) dx = get_flonum(x);
    else if (likely(is_fixnum(x))) dx = (double)get_fixnum(x);
    else { spush(x); spush(y); ac = (obj)NCMP_LT; goih(tower_cmp); }
    if (likely(is_flonum(y))) dy = get_flonum(y);
    else if (likely(is_fixnum(y))) dy = (double)get_fixnum(y);
    else { spush(x); spush(y); ac = (obj)NCMP_LT; goih(tower_cmp); }
    ac = bool_obj(dx < dy);
  }
  gonexti(); 
}

define_instruction(gt) {
  obj x = ac, y = spop();
  if (likely(are_fixnums(x, y))) {
    ac = bool_obj(get_fixnum(x) > get_fixnum(y));
  } else {
    double dx, dy;
    if (likely(is_flonum(x))) dx = get_flonum(x);
    else if (likely(is_fixnum(x))) dx = (double)get_fixnum(x);
    else { spush(x); spush(y); ac = (obj)NCMP_GT; goih(tower_cmp); }
    if (likely(is_flonum(y))) dy = get_flonum(y);
    else if (likely(is_fixnum(y))) dy = (double)get_fixnum(y);
    else { spush(x); spush(y); ac = (obj)NCMP_GT; goih(tower_cmp); }
    ac = bool_obj(dx > dy);
  }
  gonexti(); 
}

define_instruction(le) {
  obj x = ac, y = spop();
  if (likely(are_fixnums(x, y))) {
    ac = bool_obj(get_fixnum(x) <= get_fixnum(y));
  } else {
    double dx, dy;
    if (likely(is_flonum(x))) dx = get_flonum(x);
    else if (likely(is_fixnum(x))) dx = (double)get_fixnum(x);
    else { spush(x); spush(y); ac = (obj)NCMP_LE; goih(tower_cmp); }
    if (likely(is_flonum(y))) dy = get_flonum(y);
    else if (likely(is_fixnum(y))) dy = (double)get_fixnum(y);
    else { spush(x); spush(y); ac = (obj)NCMP_LE; goih(tower_cmp); }
    ac = bool_obj(dx <= dy);
  }
  gonexti(); 
}

define_instruction(ge) {
  obj x = ac, y = spop();
  if (likely(are_fixnums(x, y))) {
    ac = bool_obj(get_fixnum(x) >= get_fixnum(y));
  } else {
    double dx, dy;
    if (likely(is_flonum(x))) dx = get_flonum(x);
    else if (likely(is_fixnum(x))) dx = (double)get_fixnum(x);
    else { spush(x); spush(y); ac = (obj)NCMP_GE; goih(tower_cmp); }
    if (likely(is_flonum(y))) dy = get_flonum(y);
    else if (likely(is_fixnum(y))) dy = (double)get_fixnum(y);
    else { spush(x); spush(y); ac = (obj)NCMP_GE; goih(tower_cmp); }
    ac = bool_obj(dx >= dy);
  }
  gonexti(); 
}

define_instrhelper(tower_eq) {
  fatnum4_t x4, y4; fatnum_t *fx = NULL, *fy = NULL;
  int ok = (ac == bool_obj(1)) ? 1 : 0;
  obj y = spop(), x = spop();
  if (is_fixnum(x)) { x4.t = NUMT_FIX; x4.p[0].fix = get_fixnum(x); fx = (fatnum_t*)&x4; } 
  else if (is_flonum(x)) { x4.t = NUMT_FLO; x4.p[0].flo = get_flonum(x); fx = (fatnum_t*)&x4; } 
  else if (is_bignum(x)) { x4.t = NUMT_BIG; x4.p[0].big = get_bignum(x); fx = (fatnum_t*)&x4; } 
  else if (is_fatnum(x)) { fx = get_fatnum(x); } 
  if (is_fixnum(y)) { y4.t = NUMT_FIX; y4.p[0].fix = get_fixnum(y); fy = (fatnum_t*)&y4; } 
  else if (is_flonum(y)) { y4.t = NUMT_FLO; y4.p[0].flo = get_flonum(y); fy = (fatnum_t*)&y4; } 
  else if (is_bignum(y)) { y4.t = NUMT_BIG; y4.p[0].big = get_bignum(y); fy = (fatnum_t*)&y4; } 
  else if (is_fatnum(y)) { fy = get_fatnum(y); } 
  if (!fx) failtype(x, "number");
  if (!fy) failtype(y, "number");
  ac = bool_obj(fneqn(fx, fy) ? ok : !ok);
  gonexti();
}

define_instruction(eq) {
  obj x = ac, y = spop();
  if (likely(are_fixnums(x, y))) {
    ac = bool_obj(x == y);
  } else if (is_flonum(x) || is_flonum(y)) {
    double dx, dy;
    if (likely(is_flonum(x))) dx = get_flonum(x);
    else if (likely(is_fixnum(x))) dx = (double)get_fixnum(x);
    else { spush(x); spush(y); ac = bool_obj(1); goih(tower_eq); }
    if (likely(is_flonum(y))) dy = get_flonum(y);
    else if (likely(is_fixnum(y))) dy = (double)get_fixnum(y);
    else { spush(x); spush(y); ac = bool_obj(1); goih(tower_eq); }
    ac = bool_obj(dx == dy);
  } else ac = bool_obj(0);
  gonexti(); 
}

define_instruction(ne) {
  obj x = ac, y = spop();
  if (likely(are_fixnums(x, y))) {
    ac = bool_obj(x != y);
  } else if (is_flonum(x) || is_flonum(y)) {
    double dx, dy;
    if (likely(is_flonum(x))) dx = get_flonum(x);
    else if (likely(is_fixnum(x))) dx = (double)get_fixnum(x);
    else { spush(x); spush(y); ac = bool_obj(0); goih(tower_eq); }
    if (likely(is_flonum(y))) dy = get_flonum(y);
    else if (likely(is_fixnum(y))) dy = (double)get_fixnum(y);
    else { spush(x); spush(y); ac = bool_obj(0); goih(tower_eq); }
    ac = bool_obj(dx != dy);
  } else ac = bool_obj(1);
  gonexti(); 
}

define_instruction(min) {
  obj x = ac, y = spop();
  if (likely(are_fixnums(x, y))) {
    ac = (get_fixnum(x) < get_fixnum(y)) ? x : y;
  } else {
    double dx, dy;
    if (likely(is_flonum(x))) dx = get_flonum(x);
    else if (likely(is_fixnum(x))) dx = (double)get_fixnum(x);
    else { spush(x); spush(y); ac = (obj)&fnmin; goih(tower_binary); }
    if (likely(is_flonum(y))) dy = get_flonum(y);
    else if (likely(is_fixnum(y))) dy = (double)get_fixnum(y);
    else { spush(x); spush(y); ac = (obj)&fnmin; goih(tower_binary); }
    ac = dx < dy ? flonum_obj(dx) : flonum_obj(dy);
  }
  gonexti(); 
}

define_instruction(max) {
  obj x = ac, y = spop();
  if (likely(are_fixnums(x, y))) {
    ac = (get_fixnum(x) > get_fixnum(y)) ? x : y;
  } else {
    double dx, dy;
    if (likely(is_flonum(x))) dx = get_flonum(x);
    else if (likely(is_fixnum(x))) dx = (double)get_fixnum(x);
    else { spush(x); spush(y); ac = (obj)&fnmax; goih(tower_binary); }
    if (likely(is_flonum(y))) dy = get_flonum(y);
    else if (likely(is_fixnum(y))) dy = (double)get_fixnum(y);
    else { spush(x); spush(y); ac = (obj)&fnmax; goih(tower_binary); }
    ac = dx > dy ? flonum_obj(dx) : flonum_obj(dy);
  }
  gonexti(); 
}

define_instruction(neg) {
  if (likely(is_fixnum(ac))) {
    ac = fixnum_obj(-get_fixnum(ac));
  } else if (likely(is_flonum(ac))) {
    ac = flonum_obj(-get_flonum(ac));
  } else { spush(ac); ac = (obj)&fnneg; goih(tower_unary); }
  gonexti(); 
}

define_instruction(abs) {
  if (likely(is_fixnum(ac))) {
    ac = fixnum_obj(fxabs(get_fixnum(ac)));
  } else if (likely(is_flonum(ac))) {
    ac = flonum_obj(fabs(get_flonum(ac)));
  } else { spush(ac); ac = (obj)&fnabs; goih(tower_unary); }
  gonexti(); 
}

///////////////////////// TODO:

define_instruction(gcd) {
  obj x = ac, y = spop();
  if (likely(are_fixnums(x, y))) {
    ac = fixnum_obj(fxgcd(get_fixnum(x), get_fixnum(y)));
  } else {
    double dx, dy;
    if (likely(is_flonum(x))) dx = get_flonum(x);
    else if (likely(is_fixnum(x))) dx = (double)get_fixnum(x);
    else failtype(x, "number");
    if (likely(is_flonum(y))) dy = get_flonum(y);
    else if (likely(is_fixnum(y))) dy = (double)get_fixnum(y);
    else failtype(y, "number");
    ac = flonum_obj(flgcd(dx, dy));
  }
  gonexti(); 
}

define_instruction(pow) {
  obj x = ac, y = spop();
  if (likely(are_fixnums(x, y))) {
    /* if fxpow fails or overflows, it returns 0 */
    long fx = get_fixnum(x), fy = get_fixnum(y), fz;
    if (unlikely(fx == 0 && fy < 0)) fail("division by zero");
    fz = ((fx | fy)) ? fxpow(fx, fy) : 1; /* 0^0 == 1! */
    ac = (!fz && fx) ? flonum_obj(pow((double)fx, (double)fy)) : fixnum_obj(fz); // fixme
  } else {
    double dx, dy;
    if (likely(is_flonum(x))) dx = get_flonum(x);
    else if (likely(is_fixnum(x))) dx = (double)get_fixnum(x);
    else failtype(x, "number"); // fixme
    if (likely(is_flonum(y))) dy = get_flonum(y);
    else if (likely(is_fixnum(y))) dy = (double)get_fixnum(y);
    else failtype(y, "number"); // fixme
    ac = flonum_obj(pow(dx, dy));
  }
  gonexti(); 
}

define_instruction(sqrt) {
  if (likely(is_flonum(ac))) {
    ac = flonum_obj(sqrt(get_flonum(ac)));
  } else if (likely(is_fixnum(ac))) {
    long x = get_fixnum(ac), y;
    if (x < 0) ac = flonum_obj((HUGE_VAL - HUGE_VAL));   
    else if (y = fxsqrt(x), y*y == x) ac = fixnum_obj(y);
    else ac = flonum_obj(sqrt((double)x));
  } else failactype("number");
  gonexti(); 
}

define_instruction(exp) {
  double x;
  if (unlikely(is_fixnum(ac))) x = (double)get_fixnum(ac);
  else if (likely(is_flonum(ac))) x = get_flonum(ac);
  else failactype("number");
  ac = flonum_obj(exp(x));
  gonexti(); 
}

define_instruction(log) {
  double x; obj y = spop();
  if (unlikely(is_fixnum(ac))) x = (double)get_fixnum(ac);
  else if (likely(is_flonum(ac))) x = get_flonum(ac);
  else failactype("number");
  if (likely(!y)) {
    ac = flonum_obj(log(x));
  } else if (likely(y == fixnum_obj(10))) {
    ac = flonum_obj(log10(x));
  } else {
    double b; 
    if (unlikely(is_fixnum(y))) b = (double)get_fixnum(y);
    else if (likely(is_flonum(y))) b = get_flonum(y);
    else failtype(y, "number");
    if (likely(b == 10.0)) ac = flonum_obj(log10(x));
    else ac = flonum_obj(log(x)/log(b));
  }
  gonexti(); 
}

define_instruction(sin) {
  double x;
  if (unlikely(is_fixnum(ac))) {
    x = (double)get_fixnum(ac);
  } else if (likely(is_flonum(ac))) {
    x = get_flonum(ac);
  } else failactype("number");
  ac = flonum_obj(sin(x));
  gonexti(); 
}

define_instruction(cos) {
  double x;
  if (unlikely(is_fixnum(ac))) {
    x = (double)get_fixnum(ac);
  } else if (likely(is_flonum(ac))) {
    x = get_flonum(ac);
  } else failactype("number");
  ac = flonum_obj(cos(x));
  gonexti(); 
}

define_instruction(tan) {
  double x;
  if (unlikely(is_fixnum(ac))) {
    x = (double)get_fixnum(ac);
  } else if (likely(is_flonum(ac))) {
    x = get_flonum(ac);
  } else failactype("number");
  ac = flonum_obj(tan(x));
  gonexti(); 
}

define_instruction(asin) {
  double x;
  if (unlikely(is_fixnum(ac))) {
    x = (double)get_fixnum(ac);
  } else if (likely(is_flonum(ac))) {
    x = get_flonum(ac);
  } else failactype("number");
  ac = flonum_obj(asin(x));
  gonexti(); 
}

define_instruction(acos) {
  double x;
  if (unlikely(is_fixnum(ac))) {
    x = (double)get_fixnum(ac);
  } else if (likely(is_flonum(ac))) {
    x = get_flonum(ac);
  } else failactype("number");
  ac = flonum_obj(acos(x));
  gonexti(); 
}

define_instruction(atan) {
  double x; obj y = spop();
  if (unlikely(is_fixnum(ac))) x = (double)get_fixnum(ac);
  else if (likely(is_flonum(ac))) x = get_flonum(ac);
  else failactype("number");
  if (likely(!y)) {
    ac = flonum_obj(atan(x));
  } else {
    double b; 
    if (unlikely(is_fixnum(y))) b = (double)get_fixnum(y);
    else if (likely(is_flonum(y))) b = get_flonum(y);
    else failtype(y, "number");
    ac = flonum_obj(atan2(x, b));
  }
  gonexti(); 
}

define_instruction(floor) {
  if (likely(is_flonum(ac))) {
    ac = flonum_obj(floor(get_flonum(ac)));
  } else if (unlikely(!is_fixnum(ac))) {
    failactype("number");
  }
  gonexti(); 
}

define_instruction(ceil) {
  if (likely(is_flonum(ac))) {
    ac = flonum_obj(ceil(get_flonum(ac)));
  } else if (unlikely(!is_fixnum(ac))) {
    failactype("number");
  }
  gonexti(); 
}

define_instruction(trunc) {
  if (likely(is_flonum(ac))) {
    double x = get_flonum(ac);
    double i; modf(x,  &i);
    ac = flonum_obj(i);
  } else if (unlikely(!is_fixnum(ac))) {
    failactype("number");
  }
  gonexti(); 
}

define_instruction(round) {
  if (likely(is_flonum(ac))) {
    ac = flonum_obj(flround(get_flonum(ac)));
  } else if (unlikely(!is_fixnum(ac))) {
    failactype("number");
  }
  gonexti(); 
}

define_instruction(ntoi) {
  if (likely(is_flonum(ac))) {
    double d = get_flonum(ac); long l;
    if (flisint(d) && (l = fxflo(d)) >= FIXNUM_MIN && l <= FIXNUM_MAX) ac = fixnum_obj(l);
    else failactype("flonum integer in fixnum range");  
  } else if (likely(is_fixnum(ac))) /* keep ac as-is */ ;
  else failactype("number");
  gonexti(); 
}

define_instruction(ntoj) {
  if (likely(is_fixnum(ac))) ac = flonum_obj((flonum_t)get_fixnum(ac));
  else if (likely(is_flonum(ac))) /* keep ac as-is */ ;
  else failactype("number");
  gonexti(); 
}

/* tower definitions */
define_instruction(exnp) { /* exact? */
  if (likely(is_fixnum(ac))) ac = bool_obj(1);
  else if (likely(is_flonum(ac))) ac = bool_obj(0);
  else if (likely(is_bignum(ac))) ac = bool_obj(1);
  else if (likely(is_fatnum(ac))) ac = bool_obj(fnisexn(get_fatnum(ac)));
  else failactype("number");
  gonexti(); 
}

define_instruction(innp) { /* inexact? */
  if (likely(is_fixnum(ac))) ac = bool_obj(0);
  else if (likely(is_flonum(ac))) ac = bool_obj(1);
  else if (likely(is_bignum(ac))) ac = bool_obj(0);
  else if (likely(is_fatnum(ac))) ac = bool_obj(fnisinn(get_fatnum(ac)));
  else failactype("number");
  gonexti(); 
}

define_instruction(bigp) { /* bignum? */
  if (likely(is_bignum(ac))) ac = bool_obj(1);
  else ac = bool_obj(0); 
  gonexti(); 
}
define_instruction(ranp) { /* ratnum? */
  if (likely(is_fatnum(ac))) ac = bool_obj(fnisran(get_fatnum(ac)));
  else ac = bool_obj(0); 
  gonexti(); 
}
define_instruction(conp) { /* compnum? */
  if (likely(is_fatnum(ac))) ac = bool_obj(fniscon(get_fatnum(ac)));
  else ac = bool_obj(0); 
  gonexti(); 
}
define_instruction(renp) { /* rectnum? */
  if (likely(is_fatnum(ac))) ac = bool_obj(fnisren(get_fatnum(ac)));
  else ac = bool_obj(0); 
  gonexti(); 
}

define_instruction(numer) {
  if (likely(is_fixnum(ac))) gonexti();
  else if (likely(is_bignum(ac))) gonexti();
  else { /* slow path */
    fatnum4r_t z4; fatnum4_t x4; fatnum_t *fx = NULL;
    if (is_flonum(ac)) { x4.t = NUMT_FLO; x4.p[0].flo = get_flonum(ac); fx = (fatnum_t*)&x4; } 
    else if (is_fatnum(ac)) { fx = get_fatnum(ac); } 
    if (fx && fnnumer(&z4, fx)) {
      switch (z4.t) {
        case NUMT_NONE: assert(0); break; 
        case NUMT_FIX: ac = fixnum_obj(z4.u.p[0].fix); break;
        case NUMT_FLO: ac = flonum_obj(z4.u.p[0].flo); break;
        case NUMT_BIG: ac = bignum_obj(z4.u.p[0].big); break;
        default: ac = fatnum_obj(dupfatnum((fatnum_t*)&z4));
      }
      gonexti();
    }
  } 
  failactype("rational number");
  gonexti(); 
}

define_instruction(denom) {
  if (likely(is_fixnum(ac))) { ac = fixnum_obj(1); gonexti(); }
  else if (likely(is_bignum(ac))) { ac = fixnum_obj(1); gonexti(); }
  else { /* slow path */
    fatnum4r_t z4; fatnum4_t x4; fatnum_t *fx = NULL;
    if (is_flonum(ac)) { x4.t = NUMT_FLO; x4.p[0].flo = get_flonum(ac); fx = (fatnum_t*)&x4; } 
    else if (is_fatnum(ac)) { fx = get_fatnum(ac); } 
    if (fx && fndenom(&z4, fx)) {
      switch (z4.t) {
        case NUMT_NONE: assert(0); break; 
        case NUMT_FIX: ac = fixnum_obj(z4.u.p[0].fix); break;
        case NUMT_FLO: ac = flonum_obj(z4.u.p[0].flo); break;
        case NUMT_BIG: ac = bignum_obj(z4.u.p[0].big); break;
        default: ac = fatnum_obj(dupfatnum((fatnum_t*)&z4));
      }
      gonexti();
    }
  } 
  failactype("rational number");
  gonexti(); 
}

define_instruction(rpart) { 
  if (likely(is_fixnum(ac))) gonexti();
  else if (likely(is_flonum(ac))) gonexti();
  else if (likely(is_bignum(ac))) gonexti();
  else { /* slow path */
    fatnum4r_t z4; fatnum_t *fx = NULL;
    if (is_fatnum(ac)) { fx = get_fatnum(ac); } 
    if (fx && fnrpart(&z4, fx)) {
      switch (z4.t) {
        case NUMT_NONE: assert(0); break; 
        case NUMT_FIX: ac = fixnum_obj(z4.u.p[0].fix); break;
        case NUMT_FLO: ac = flonum_obj(z4.u.p[0].flo); break;
        case NUMT_BIG: ac = bignum_obj(z4.u.p[0].big); break;
        default: ac = fatnum_obj(dupfatnum((fatnum_t*)&z4));
      }
      gonexti();
    }
  } 
  failactype("number");
  gonexti(); 
}

define_instruction(ipart) { 
  if (likely(is_fixnum(ac))) { ac = fixnum_obj(0); gonexti(); }
  else if (likely(is_flonum(ac))) { ac = fixnum_obj(0); gonexti(); }
  else if (likely(is_bignum(ac))) { ac = fixnum_obj(0); gonexti(); }
  else { /* slow path */
    fatnum4r_t z4; fatnum_t *fx = NULL;
    if (is_fatnum(ac)) { fx = get_fatnum(ac); } 
    if (fx && fnipart(&z4, fx)) {
      switch (z4.t) {
        case NUMT_NONE: assert(0); break; 
        case NUMT_FIX: ac = fixnum_obj(z4.u.p[0].fix); break;
        case NUMT_FLO: ac = flonum_obj(z4.u.p[0].flo); break;
        case NUMT_BIG: ac = bignum_obj(z4.u.p[0].big); break;
        default: ac = fatnum_obj(dupfatnum((fatnum_t*)&z4));
      }
      gonexti();
    }
  } 
  failactype("number");
  gonexti(); 
}

define_instruction(magn) { 
  if (likely(is_fixnum(ac))) {
    ac = fixnum_obj(fxabs(get_fixnum(ac))); 
    gonexti();
  } else if (likely(is_flonum(ac))) {
    ac = flonum_obj(fabs(get_flonum(ac))); 
    gonexti();
  } else if (likely(is_bignum(ac))) {
    ac = bignum_obj(bnabs(get_bignum(ac))); 
    gonexti();
  } else { /* slow path */
    fatnum4r_t z4; fatnum_t *fx = NULL;
    if (is_fatnum(ac)) { fx = get_fatnum(ac); } 
    if (fx && fnmagn(&z4, fx)) {
      switch (z4.t) {
        case NUMT_NONE: assert(0); break; 
        case NUMT_FIX: ac = fixnum_obj(z4.u.p[0].fix); break;
        case NUMT_FLO: ac = flonum_obj(z4.u.p[0].flo); break;
        case NUMT_BIG: ac = bignum_obj(z4.u.p[0].big); break;
        default: ac = fatnum_obj(dupfatnum((fatnum_t*)&z4));
      }
      gonexti();
    }
  } 
  failactype("number");
  gonexti(); 
}

define_instruction(angl) {
  if (likely(is_fixnum(ac))) { 
    ac = (get_fixnum(ac) < 0) ? flonum_obj(M_PI) : fixnum_obj(0); 
    gonexti(); 
  } else if (likely(is_flonum(ac))) {
    double d = get_flonum(ac);
    ac = (d < 0.0 || (d == 0.0 && 1.0/d < 0)) ? flonum_obj(M_PI) : fixnum_obj(0); 
    gonexti(); 
  } else if (likely(is_bignum(ac))) { 
    ac = (bnsign(get_bignum(ac)) < 0) ? flonum_obj(M_PI) : fixnum_obj(0); 
    gonexti(); 
  } else { /* slow path */
    fatnum4r_t z4; fatnum_t *fx = NULL;
    if (is_fatnum(ac)) { fx = get_fatnum(ac); } 
    if (fx && fnangl(&z4, fx)) {
      switch (z4.t) {
        case NUMT_NONE: assert(0); break; 
        case NUMT_FIX: ac = fixnum_obj(z4.u.p[0].fix); break;
        case NUMT_FLO: ac = flonum_obj(z4.u.p[0].flo); break;
        case NUMT_BIG: ac = bignum_obj(z4.u.p[0].big); break;
        default: ac = fatnum_obj(dupfatnum((fatnum_t*)&z4));
      }
      gonexti();
    }
  } 
  failactype("number");
  gonexti(); 
}

define_instruction(mkrec) {
  obj i = spop();
  ckn(ac);
  if (!is_fixnum(i) || get_fixnum(i) != 0) 
    fail("result cannot be represented as a Skint number");
  gonexti();
}
define_instruction(mkpol) {
  obj a = spop();
  ckn(ac);
  if (!is_fixnum(a) || get_fixnum(a) != 0)
    fail("result cannot be represented as a Skint number");
  gonexti();
}

/* fixnum redirects in standard build */
define_instruction(gsqrt) { goi(isqrt); }
define_instruction(gnot) { goi(inot); }
define_instruction(gand) { goi(iand); }
define_instruction(gior) { goi(iior); }
define_instruction(gxor) { goi(ixor); }

define_instruction(gash) {
  long ix, iy, iz; 
  obj x = ac, y = spop(); cki(x); cki(y);
  ix = get_fixnum(x); iy = get_fixnum(y);
  if (iy < 0) { 
    if (-iy >= FIXNUM_WIDTH) iz = (ix >= 0 ? 0 : -1);
    else iz = fxasr(ix, -iy);
  } else {
    iz = (ix == 0 || iy < FIXNUM_WIDTH) ? fxasl(ix, iy) : FIXNUM_MAX+1;
    if (iz < FIXNUM_MIN || iz > FIXNUM_MAX || fxasr(iz, iy) != ix) {
      ac = flonum_obj((double)ix * pow(2.0, (double)iy));
      gonexti();
    }
  }
  ac = fixnum_obj(iz);
  gonexti();
}

define_instruction(geqv) { goi(ieqv); }
define_instruction(glen) { goi(ilen); }
define_instruction(gbtc) { goi(ibtc); }

/* generic number <-> string conversions */

define_instruction(ntos) {
  if (is_fixnum(ac)) {
    goi(itos);
  } else if (is_flonum(ac)) {
    if (unlikely(spop() != fixnum_obj(10))) fail("non-10 radix in flonum conversion");
    spush(0); /* #f: use default precision */
    goi(jtos);
  } else {
    int radix; cbuf_t *pcb; char *s;
    int (*pf)(int, void*) = (int (*)(int, void*))cbputc;
    obj x = ac, y = spop(); ckn(x); ckk(y);
    radix = get_fixnum(y);
    if (radix < 2 || radix > 10 + 'z' - 'a') failtype(y, "valid radix");
    pcb = newcb();
    if (is_bignum(x)) wrbn(get_bignum(x), radix, pf, pcb);
    else if (is_fatnum(x)) wrfn(get_fatnum(x), radix, pf, pcb);
    else failtype(x, "known number");
    s = cbdata(pcb);
    ac = string_obj(newsdata(s));
    gonexti();
  }
}

define_instruction(ston) {
  const char *s; int radix; fatnum4_t f4;
  obj x = ac, y = spop(); cks(x); ckk(y);
  s = stringchars(x); radix = get_fixnum(y);
  if (radix < 2 || radix > 10 + 'z' - 'a') failtype(y, "valid radix");
  errno = 0;
  switch (strtonum(&f4, s, NULL, radix)) {
    case NUMT_NONE: ac = bool_obj(0); break;
    case NUMT_FIX:  ac = fixnum_obj(f4.p[0].fix); break;
    case NUMT_FLO:  ac = flonum_obj(f4.p[0].flo); break;
    case NUMT_BIG:  ac = bignum_obj(f4.p[0].big); break;
    default: ac = fatnum_obj(dupfatnum((fatnum_t*)&f4)); break;
  }
  gonexti();
}


