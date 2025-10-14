/* Enhanced TTY interfaces */

extern void ttyinit(void);
extern char* ttygets(const char *prompt, char *buf, int bufsz);
extern void ttyputs(const char *str);
