/* n_enhtty.h -- enhanced tty interfaces */

extern void ttyinit(void);
extern char* ttygets(const char *prompt, char *buf, int bufsz);
extern void ttyputs(const char *str);

typedef enum { TTF_NONE = 0, TTF_EOF = 1, TTF_CI = 2 } ttflags_t;
typedef struct ttfile { cbuf_t cb; char prbuf[64]; char *next; int lno; ttflags_t flags; } ttfile_t;
extern ttfile_t *ttalloc(int in);

static void ttfree(ttfile_t *tp);
static int ttgetch(ttfile_t *tp);
static int ttungetch(int c, ttfile_t *tp);
static int ttctl(ctlop_t op, ttfile_t *tp, ...);
static int ttputch(int c, ttfile_t *tp);
static int ttflush(ttfile_t *tp);
