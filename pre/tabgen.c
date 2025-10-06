/* Build minimal 32-bit (cp<<8)|mask table from UnicodeData.txt
 * see https://unicode.org/Public/15.1.0/ucd/UnicodeData.txt
 * produces uniprops.c */
#ifdef _WIN32 /* works as _WIN64 too */
#define _CRT_SECURE_NO_WARNINGS
#endif
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

int verbose = 1;

#define MAX_CP  0x10FFFF
#define LINE_LEN 512
#define MAX_TABLE 16384

#define FLAG_ALPHA   1
#define FLAG_DECIMAL 2
#define FLAG_WHITE   4
#define FLAG_UPPER   8
#define FLAG_LOWER   16

/* main property table (populated from all sources) */
static struct pte { int fromcp; int tocp; int typ; } prop_table[MAX_TABLE];

/* flags table (populated from addditional prop files) */
static uint8_t flags_table[MAX_CP+1];

/* parse flags from the additional prop files */
static int parse_flags(const char *path)
{
  /* We accept any prop files, but they need to be in order. */
  struct { const char *name; uint8_t mask; } props[] = {
    { "Alphabetic",  FLAG_ALPHA  },
    { "Uppercase",   FLAG_UPPER  },
    { "Lowercase",   FLAG_LOWER  },
    { "White_Space", FLAG_WHITE  },
    /* { "Decimal",     FLAG_DECIMAL}, -- comes from main data, field 6 */
    { NULL, 0 }
  };
  char line[LINE_LEN];
  FILE *f = fopen(path, "r");
  if (!f) return 1;

  while (fgets(line, LINE_LEN, f)) {
    unsigned x, y, cp; int i; char *prop; uint8_t mask;
    char *p = strchr(line, ';');
    if (!p) continue;
    *p = 0;
    prop = p + 1;
    while (*prop == ' ' || *prop == '\t') ++prop;

    /* strip trailing comment/CR/LF */
    if ((p = strchr(prop, '#'))) *p = 0;
    while (p > prop && (p[-1] == ' ' || p[-1] == '\t' || p[-1] == '\r' || p[-1] == '\n')) *--p = 0;

    mask = 0;
    for (i = 0; props[i].name; ++i) {
      if (strcmp(prop, props[i].name) == 0) { mask = props[i].mask; break; }
    }
    if (!mask) continue;

    /* range or single */
    if (sscanf(line, "%X..%X", &x, &y) == 2) {
      for (cp = x; cp <= y && cp <= MAX_CP; ++cp) flags_table[cp] |= mask;
    } else if (sscanf(line, "%X", &x) == 1) {
      if (x <= MAX_CP) flags_table[x] |= mask;
    }
  }

  fclose(f);
  return 0;
}

/* case mapping */

/* special offsets for oscillating groups */
#define CASEIDENTITY  0x77777700 /* 0, but no fall-thru */
#define CASEPLUS_01   0x77777701 /* oscillating  0 / 1  */
#define CASEPLUS_10   0x77777710 /* oscillating  1 / 0  */
#define CASEMINUS_01  0x7777770F /* oscillating  0 / -1 */
#define CASEMINUS_10  0x777777F0 /* oscillating -1 / 0  */

/* main case tables (populated from all sources) */
static struct cte { int fromcp; int tocp; int off; } 
  lower_table[MAX_TABLE], upper_table[MAX_TABLE], title_table[MAX_TABLE], fold_table[MAX_TABLE];

/* extra lc map to calc differences with fold info */
static int lower_map[MAX_CP+1];

/* push new case info to the property table */
static int push_case(struct cte *table, int ti, int cp, int off)
{
  struct cte *ptp = &table[ti-1], *pti = &table[ti];

retry:  
  /* try to extend the previous record */
  assert((!cp && ti == 1) || cp > ptp->tocp);
  if (!cp) {
    ptp->fromcp = ptp->tocp = 0; ptp->off = 0;
  } else if (cp == ptp->tocp + 1) {
    if (off == ptp->off) { 
      ptp->tocp = cp;
    } else if (ptp->off == 1 && cp == ptp->fromcp + 1 && off == 0) {
      ptp->off = CASEPLUS_10; ptp->tocp = cp; 
    } else if (ptp->off == 0 && cp == ptp->fromcp + 1 && off == 1) {
      ptp->off = CASEPLUS_01; ptp->tocp = cp; 
    } else if (ptp->off == CASEPLUS_10 && (cp - ptp->fromcp) % 2 == 0 && off == 1) {
      ptp->tocp = cp;
    } else if (ptp->off == CASEPLUS_10 && (cp - ptp->fromcp) % 2 == 1 && off == 0) {
      ptp->tocp = cp;
    } else if (ptp->off == CASEPLUS_01 && (cp - ptp->fromcp) % 2 == 0 && off == 0) {
      ptp->tocp = cp;
    } else if (ptp->off == CASEPLUS_01 && (cp - ptp->fromcp) % 2 == 1 && off == 1) {
      ptp->tocp = cp;
    } else if (ptp->off == -1 && cp == ptp->fromcp + 1 && off == 0) {
      ptp->off = CASEMINUS_10; ptp->tocp = cp; 
    } else if (ptp->off == 0 && cp == ptp->fromcp + 1 && off == -1) {
      ptp->off = CASEMINUS_01; ptp->tocp = cp; 
    } else if (ptp->off == CASEMINUS_10 && (cp - ptp->fromcp) % 2 == 0 && off == -1) {
      ptp->tocp = cp;
    } else if (ptp->off == CASEMINUS_10 && (cp - ptp->fromcp) % 2 == 1 && off == 0) {
      ptp->tocp = cp;
    } else if (ptp->off == CASEMINUS_01 && (cp - ptp->fromcp) % 2 == 0 && off == 0) {
      ptp->tocp = cp;
    } else if (ptp->off == CASEMINUS_01 && (cp - ptp->fromcp) % 2 == 1 && off == -1) {
      ptp->tocp = cp;
    } else { /* got to ship previous record and insert new one */
      if (verbose > 1) fprintf(stderr, "CASE RECORD #%d: { %.4X..%.4X %d }\n", ti-1, ptp->fromcp, ptp->tocp, ptp->off);
      pti->fromcp = pti->tocp = cp; pti->off = off;
      ti += 1;
    }
  } else { /* gap */
    if (ptp->off == 0) { 
      ptp->tocp = cp-1; /* absorb gap */
      goto retry;
    } else if (cp == ptp->tocp + 2 && ptp->fromcp == ptp->tocp && ptp->off == 1) { /* special gap */
      ptp->off = CASEPLUS_10; ptp->tocp += 1; goto retry;
    } else if (ptp->off == CASEPLUS_10 && cp == ptp->tocp + 2 && (ptp->tocp+1 - ptp->fromcp) % 2 == 1) { /* special gap 2 */
      ptp->tocp += 1; goto retry;
    } else if (ptp->off == CASEPLUS_01 && cp == ptp->tocp + 2 && (ptp->tocp+1 - ptp->fromcp) % 2 == 0) { /* special gap 2 */
      ptp->tocp += 1; goto retry;
    } else if (cp == ptp->tocp + 2 && ptp->fromcp == ptp->tocp && ptp->off == -1) { /* special gap */
      ptp->off = CASEMINUS_10; ptp->tocp += 1; goto retry;
    } else if (ptp->off == CASEMINUS_10 && cp == ptp->tocp + 2 && (ptp->tocp+1 - ptp->fromcp) % 2 == 1) { /* special gap 2 */
      ptp->tocp += 1; goto retry;
    } else if (ptp->off == CASEMINUS_01 && cp == ptp->tocp + 2 && (ptp->tocp+1 - ptp->fromcp) % 2 == 0) { /* special gap 2 */
      ptp->tocp += 1; goto retry;
    } else {
      if (verbose > 1) fprintf(stderr, "CASE RECORD #%d: { %.4X..%.4X %d }\n", ti-1, ptp->fromcp, ptp->tocp, ptp->off);
      pti->fromcp = ptp->tocp+1; pti->tocp = cp-1; pti->off = 0;
      ti += 1;
      ptp = pti; pti = ptp+1;
      goto retry;
    }
  }

  return ti;
}

/* properties */

/* push new property to the property table */
static int push_prop(int ti, int cp, int typ)
{
  struct pte *ptp = &prop_table[ti-1], *pti = &prop_table[ti];
 
  /* try to extend the previous record */
  assert((!cp && ti == 1) || cp > ptp->tocp);
  if (!cp) {
    ptp->fromcp = ptp->tocp = 0; ptp->typ = '?';
  } else if (cp == ptp->tocp + 1) {
    if (typ == ptp->typ) { 
      ptp->tocp = cp;
    } else if (ptp->typ == 'l' && cp == ptp->fromcp + 1 && typ == 'u') {
      ptp->typ = 'L'; ptp->tocp = cp; 
    } else if (ptp->typ == 'u' && cp == ptp->fromcp + 1 && typ == 'l') {
      ptp->typ = 'U'; ptp->tocp = cp; 
    } else if (ptp->typ == 'L' && (cp - ptp->fromcp) % 2 == 0 && typ == 'l') {
      ptp->tocp = cp;
    } else if (ptp->typ == 'L' && (cp - ptp->fromcp) % 2 == 1 && typ == 'u') {
      ptp->tocp = cp;
    } else if (ptp->typ == 'U' && (cp - ptp->fromcp) % 2 == 0 && typ == 'u') {
      ptp->tocp = cp;
    } else if (ptp->typ == 'U' && (cp - ptp->fromcp) % 2 == 1 && typ == 'l') {
      ptp->tocp = cp;
    } else if (ptp->typ == '0' && cp == ptp->fromcp + 1 && typ == '1') {
      ptp->typ = 'd'; ptp->tocp = cp;
    } else if (ptp->typ == 'd' && cp - ptp->fromcp + '0' == typ && typ <= '9') {
      ptp->tocp = cp;
    } else { /* got to ship previous record and insert new one */
      if (verbose > 1) fprintf(stderr, "RECORD #%d: { %.4X..%.4X %c }\n", ti-1, ptp->fromcp, ptp->tocp, ptp->typ);
      pti->fromcp = pti->tocp = cp; pti->typ = typ;
      ti += 1;
    }
  } else { /* gap */
    assert(0);
  }

  return ti;
}

/* properties and case */

static int process_prop(int cp, char *cat, char *dv, char *upper, char *lower, char *title, int ti, int *plti, int *puti, int *ptti)
{
  int typ, lcp = -1, ucp = -1, tcp = -1, uti = *puti, lti = *plti, tti = *ptti;

  /* process case mapping first */
  if (lower[0] && (lcp = (int)strtol(lower, NULL, 16)) > 0 && lcp <= MAX_CP) /* full */
    lti = push_case(lower_table, lti, cp, lcp-cp); else lcp = -1;
  if (upper[0] && (ucp = (int)strtol(upper, NULL, 16)) > 0 && ucp <= MAX_CP) /* full */
    uti = push_case(upper_table, uti, cp, ucp-cp); else ucp = -1;
  if (title[0] && (tcp = (int)strtol(title, NULL, 16)) > 0 && tcp <= MAX_CP && tcp != ucp) /* delta w/uc */
    tti = push_case(title_table, tti, cp, tcp==cp ? CASEIDENTITY : tcp-cp); else tcp = -1;
  /* save lc info for calculating differences with fold */
  if (lcp > 0) lower_map[cp] = lcp;

  /* type of the current entry */
  if (!strcmp(cat, "Lu")) 
    typ = 'u';  /* uppercase */
  else if (!strcmp(cat, "Ll")) 
    typ = 'l';  /* lowercase */
  else if (cat[0] == 'L')
    typ = 'a'; /* alphabetic */
  else if (!strcmp(cat, "Nd") && '0' <= dv[0] && dv[0] <= '9' && dv[1] == 0)
    typ = dv[0]; /* decimal digit */
  else if ((cp >= 0x80 && !strcmp(cat, "Zs")) || (cp < 0x80 && strchr(" \t\n\v\f\r", cp)))
    typ = 's'; /* whitespace */
  else if (flags_table[cp] & FLAG_UPPER) 
    typ = 'u'; /* additional uppercase */
  else if (flags_table[cp] & FLAG_LOWER)
    typ = 'l'; /* additional lowercase */
  else if (flags_table[cp] & FLAG_ALPHA)
    typ = 'a'; /* additional alphabetic */
  else if (flags_table[cp] & FLAG_WHITE) 
    typ = 's'; /* additional whitespace */
  else
    typ = '?';

  ti = push_prop(ti, cp, typ);

  *plti = lti; *puti = uti; *ptti = tti; 
  return ti;
}

/* parse properties and case from UnicodeData.txt */
static int parse_props(const char *path, int *plti, int *puti, int *ptti)
{
  char line[LINE_LEN]; int lno = 1, ti, uti, lti, tti, prevcp = -1; 
  FILE *f = fopen(path, "r");
  if (!f) { return -1; }

  /* init casing records */
  lower_table[0].fromcp = lower_table[0].tocp = 0; 
  lower_table[0].off = 0; lti = 1;
  upper_table[0].fromcp = upper_table[0].tocp = 0; 
  upper_table[0].off = 0; uti = 1;
  title_table[0].fromcp = title_table[0].tocp = 0; 
  title_table[0].off = 0; tti = 1;

  /* init the first property record */
  prop_table[0].fromcp = prop_table[0].tocp = 0; 
  prop_table[0].typ = '?'; ti = 1;

  /* parse the file */
  for (; fgets(line, LINE_LEN, f) != NULL; ++lno) {
    char *s, *field[15], *name, *cat, *dv, *upper, *lower, *title; 
    int cp, i, lcp = -1, ucp = -1, tcp = -1; 

    /* split line by semicolon */
    for (s = line, i = 0; i <= 14; ++i) {
      char *ns = (i < 14) ? strchr(s, ';') : strchr(s, '\n');
      if (ns) { field[i] = s; *ns = 0; s = ns+1; }
      else if (i == 14) { field[i] = s; }
      else break;
    }

    if (i < 14) {
      fprintf(stderr, "line %d failed to parse [%d]: %s\n", lno, i, line);
      continue;
    }
    if (ti+1 >= MAX_TABLE) { 
      fprintf(stderr, "table overflow while processing line %d: increase MAX_TABLE\n", lno);
      exit(1);
    }
       
    /* important fields */
    cp = strtol(field[0], NULL, 16); name = field[1]; cat = field[2]; 
    dv = field[6]; upper = field[12]; lower = field[13]; title = field[14];

    /* fill in lines missing in a gap */
    while (prevcp+1 < cp) { /* fill gap with fake info */
      ti = process_prop(++prevcp, "Xx", "", "", "", "", ti, &lti, &uti, &tti);
    }
    if (name[0] == '<' && strstr(name, "First>")) { 
      /* repeat lines in ranges */
      int cp_first = cp, cp_last; ++lno;
      if (fgets(line, LINE_LEN, f) != NULL 
          && (cp_last = (int)strtol(line, &s, 16)) > cp_first
          && s && s[0] == ';' && s[1] == '<' && strstr(s+1, "Last>")) {
        /* just roll a loop with typ from <first> */
        for (cp = cp_first; cp <= cp_last; ++cp) { 
          ti = process_prop(cp, cat, "", "", "", "", ti, &lti, &uti, &tti);
        }
        prevcp = cp_last;
      } else {
        fprintf(stderr, "range error on line %d: %s\n", lno, line);
        exit(1);
      }
    } else {
      /* singular line */
      ti = process_prop(cp, cat, dv, upper, lower, title, ti, &lti, &uti, &tti);
      prevcp = cp;
    }
  }

  /* fill in the last gap*/
  while (++prevcp <= MAX_CP) { 
    ti = process_prop(prevcp, "Xx", "", "", "", "", ti, &lti, &uti, &tti);
  }

  if (verbose > 0) fprintf(stderr, "Total number of records: %d\n", ti);

  fclose(f);

  *plti = lti; *puti = uti; *ptti = tti; 
  return ti;
}

void emit_props(const char *out, int n)
{
  FILE *f = out ? fopen(out, "w") : stdout; int i;
  if (!f) { perror(out); exit(EXIT_FAILURE); }

  if (out) fputs("/* Generated by tabgen - do not edit */\n"
    "#include <stdint.h>\n\n"
    "static const uint32_t uprops[] = {\n ", f);

  for (i = 0; i < n; ++i) {
    struct pte *pti = &prop_table[i];
    uint32_t w = (((uint32_t)pti->fromcp) << 8) | (uint32_t)pti->typ;
    fprintf(f, " 0x%08X,", w);
    if ((i+1) % 8 == 0) fputs("\n ", f);
  }

  fprintf(f, " 0x10FFFF3F\n};\n\n");

  if (f != stdout) fclose(f);

  if (verbose > 0) fprintf(stderr, "Wrote %s : %d prop entries\n", out, n);
}

/* separate fold-case data */

static int parse_fold_case(const char *path)
{
  int ti; FILE *f = fopen(path, "r");
  if (!f) return -1;
  char line[LINE_LEN];

  /* init the first record */
  fold_table[0].fromcp = fold_table[0].tocp = 0; 
  fold_table[0].off = 0; ti = 1;

  while (fgets(line, LINE_LEN, f)) {
    unsigned cp, fold;
    char status;
    /* format: code ; status ; mapping ; # comment */
    if (sscanf(line, "%X ; %c ; %X", &cp, &status, &fold) != 3) continue;
    if (cp > MAX_CP || fold > MAX_CP) continue;
    /* keep only simple (S) or common (C) one-to-one mappings */
    if (status != 'C' && status != 'S') continue;
    /* store only differences with lc mapping! */
    if ((int)fold != lower_map[cp])
      ti = push_case(fold_table, ti, cp, fold==cp ? CASEIDENTITY : (int)fold - (int)cp);
  }

  if (fold_table[ti-1].tocp != MAX_CP) ti = push_case(fold_table, ti, MAX_CP, 0); 
  if (verbose > 1) fprintf(stderr, "CASE RECORD #%d: { %.4X..%.4X %d }\n", ti-1, 
    fold_table[ti-1].fromcp, fold_table[ti-1].tocp, fold_table[ti-1].off);

  if (verbose > 0) fprintf(stderr, "Total number of fold case records: %d\n", ti);

  fclose(f);

  return ti;
}

void emit_case(FILE *f, char *tname, struct cte *table, int n)
{
  int i;
  fprintf(f, "static const uint64_t %s[] = {\n ", tname);
  for (i = 0; i < n; ++i) {
    struct cte *pti = &table[i];
    uint64_t w = (((uint64_t)pti->fromcp) << 32) | (uint64_t)(uint32_t)(int32_t)pti->off;
    fprintf(f, " 0x%016llX,", w);
    if ((i+1) % 4 == 0) fputs("\n ", f);
  }
  fprintf(f, " 0x0010FFFF00000000\n};\n\n");

  if (verbose > 0) fprintf(stderr, "  %d %s entries\n", n, tname);
}

void emit_cases(const char *out, int ln, int un, int tn, int fn)
{
  FILE *f = out ? fopen(out, "w") : stdout;
  if (!f) { perror(out); exit(EXIT_FAILURE); }

  if (out) fputs("/* Generated by tabgen - do not edit */\n", f);
  if (out) fputs("#include <stdint.h>\n\n", f);

  fputs("#define CASEIDENTITY  0x77777700 /* 0, but no fall-thru */\n", f);
  fputs("#define CASEPLUS_01   0x77777701 /* oscillating  0 / 1  */\n", f);
  fputs("#define CASEPLUS_10   0x77777710 /* oscillating  1 / 0  */\n", f);
  fputs("#define CASEMINUS_01  0x7777770F /* oscillating  0 / -1 */\n", f);
  fputs("#define CASEMINUS_10  0x777777F0 /* oscillating -1 / 0  */\n\n", f);

  if (verbose > 0) fprintf(stderr, "Wrote %s:\n", out);
  if (ln > 0) emit_case(f, "utolc", lower_table, ln);
  if (un > 0) emit_case(f, "utouc", upper_table, un);
  if (tn > 0) emit_case(f, "utotc", title_table, tn);
  if (fn > 0) emit_case(f, "ufold", fold_table, fn);

  if (f != stdout) fclose(f);
}

static char *uprops_start = "/* start of tabgen-generated property table */";
static char *uprops_end   = "/* end of tabgen-generated property table */";
static char *ucases_start = "/* start of tabgen-generated case tables */";
static char *ucases_end   = "/* end of tabgen-generated case tables */";

int main(int argc, char *argv[])
{
  int e, n, ln = 0, un = 0, tn = 0, fn = 0;
  char *tfname = NULL;

  if (argc > 2) { fprintf(stderr, "Usage: tabgen [TEMPLATE_FILE]\n"); exit(1); }
  else if (argc > 1) tfname = argv[1];

  if (tfname) verbose = 0;

  e = parse_flags("DerivedCoreProperties.txt");
  if (e) {
    fprintf(stderr, "Failed to open DerivedCoreProperties.txt\n"
      "You may download it via 'wget https://www.unicode.org/Public/UNIDATA/DerivedCoreProperties.txt'\n");
    return 1;
  }
  e = parse_flags("PropList.txt");
  if (e) {
    fprintf(stderr, "Failed to open PropList.txt\n"
      "You may download it via 'wget https://www.unicode.org/Public/UNIDATA/PropList.txt'\n");
    return 1;
  }
  n = parse_props("UnicodeData.txt", &ln, &un, &tn);
  if (n < 0) {
    fprintf(stderr, "Failed to open UnicodeData.txt\n"
      "You may download it via 'wget https://www.unicode.org/Public/UNIDATA/UnicodeData.txt'\n");
    return 1;
  }
  fn = parse_fold_case("CaseFolding.txt");
  if (fn < 0) {
    fprintf(stderr, "Failed to open CaseFolding.txt\n"
      "You may download it via 'wget https://www.unicode.org/Public/UNIDATA/CaseFolding.txt'\n");
    return 1;
  }

  if (tfname) {
    char line[LINE_LEN]; int state = 0;
    FILE *f = fopen(tfname, "r");
    if (!f) { perror("cannot open template file:"); exit(1); } 
    while (fgets(line, LINE_LEN, f) != NULL) {
      switch (state) {
        case 0: {
          if (strstr(line, uprops_start)) state = 1;
          fputs(line, stdout);
        } break;
        case 1: {
          if (strstr(line, uprops_end)) {
            emit_props(NULL, n);
            fputs(line, stdout);
            state = 2;
          }
        } break;
        case 2: {
          if (strstr(line, ucases_start)) state = 3;
          fputs(line, stdout);
        } break;
        case 3: {
          if (strstr(line, ucases_end)) {
            emit_cases(NULL, ln, un, tn, fn);
            fputs(line, stdout);
            state = 4;
          }
        } break;
        case 4: {
          fputs(line, stdout);
        } break;
      }
    }
    fclose(f);
  } else {
    emit_props("udata.c", n);
    emit_cases("ufold.c", ln, un, tn, fn);
  }

  return 0;
}
