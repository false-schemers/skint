/* UNICODE replacements for standard string interfaces */

/* char (code point) */
extern int uisspace(int c);
extern int uislower(int c);
extern int uisupper(int c);
extern int uisalpha(int c);
extern int uisdigit(int c);
extern int udigitval(int c);
extern int utolower(int c);
extern int utoupper(int c);
extern int utotitle(int c);
extern int utofold(int c);

/* basic string parsing/unparsing */
extern int uencode(char *buf, int c);
extern int udecode(const char **sp);
extern int udecode_check(const char **sp);
#define unextc(cp) udecode((const char **)&(cp))
#define unextc_check(cp) udecode_check((const char **)&(cp))
extern int udistance(const char *u8s, const char *u8e);
extern void *umemchr(const void *s, int c, size_t spn);
extern char *uungetch(int c, cbuf_t *pcb, char *next);
extern int ufputc(int c, FILE *fp); 
extern int ucbputc(int c, cbuf_t* pcb); 

/* sdata (string data) */
#define sdatachars(d) ((char*)((d)+2))
#define sdatacspan(d) ((d)[1])
extern int *makesdata(int n, int c);
extern int sdataget(const int *d, int i);
extern int *sdataput(int *d, int i, int c);
extern int *newsdata(const char *u8s);
extern int *newsdatan(const char *u8s, int nbytes);
extern int *subsdata(const int *d, int fromc, int toc);
extern int *catsdata(const int *d0, const int *d1);
extern int *dupsdata(const int *d);
extern int *mapsdata(const int *d, int (*f)(int));
extern int sdatacmp(const int *d1, const int *d2);
extern int sdatacmp_ci(const int *d1, const int *d2);
extern unsigned long sdatahash(const int *d);

/* string procedures */
#define stringget(o, i) sdataget(stringdata(o), i)
static void stringput(obj o, int i, int c) {
  const int *d = stringdata(o); assert(i >= 0 && i < d[0]);
  if (d[0] == d[1] && (unsigned char)c < 0x80) sdatachars(d)[i] = c;
  else setnative(o, STRING_NTAG, sdataput((int *)d, i, c)); 
} 
extern int *stringr(int sc, obj pso[]);
extern int *stringrcat(int sc, obj pso[]);

/* system procedures */
extern FILE *ufopen(const char* fname, const char* mode);
