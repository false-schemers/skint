/* Enhanced TTY interfaces */

#if defined(_MSC_VER)
/* =========================== Windows TTY ================================= */

static UINT original_in_cp;
static UINT original_out_cp;
const char* enhterm_lib = NULL;

void ttyfini(void) 
{
  SetConsoleCP(original_in_cp);
  SetConsoleOutputCP(original_out_cp);
}

void ttyinit(void) 
{
  HANDLE hin = GetStdHandle(STD_INPUT_HANDLE);
  HANDLE hout = GetStdHandle(STD_OUTPUT_HANDLE);
  original_in_cp = GetConsoleCP();
  original_out_cp = GetConsoleOutputCP();
  SetConsoleCP(CP_UTF8);
  SetConsoleOutputCP(CP_UTF8);
  enhterm_lib = "conhost.exe";
  atexit(ttyfini);
}

int ttyisansi(void)
{
#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif
  const char *term; DWORD mode = 0;
  HANDLE hout = GetStdHandle(STD_OUTPUT_HANDLE);
  if (hout == INVALID_HANDLE_VALUE) return 0;
  if (GetConsoleMode(hout, &mode) && (mode & ENABLE_VIRTUAL_TERMINAL_PROCESSING)) return 1;
  if (getenv("ANSICON")) return 1; /* ANSICON, ConEmuANSI */
  term = getenv("TERM"); /* check for Cygwin, MSYS2, Git-Bash, mintty */
  return !!(term && (strstr(term, "xterm") || strstr(term, "cygwin") || strstr(term, "rxvt")));
}

char* ttygetln(const char *prompt, cbuf_t *pcb) 
{
  char *buf = cballoc(cbclear(pcb), 256); int bufsz = 256;
  HANDLE hin; wchar_t wbuf[256]; DWORD rc, wc; char *res;
  if (prompt) ttyputs(prompt);
  hin = GetStdHandle(STD_INPUT_HANDLE);
  rc = 0; res = NULL;
  if (ReadConsoleW(hin, wbuf, bufsz-1, &rc, NULL)) {
    if (rc > 0 && wbuf[rc-1] == L'\n') {
      wbuf[rc-1] = L'\0';
      if (rc > 1 && wbuf[rc-2] == L'\r') wbuf[rc-2] = L'\0';
    } else wbuf[rc] = L'\0';
    wcscat(wbuf, L"\n");
    wc = WideCharToMultiByte(CP_UTF8, 0, wbuf, -1, buf, bufsz, NULL, NULL);
    if (!wc) return NULL; /* no input or conversion failure */
    pcb->fill = pcb->buf + wc - 1; /* wc includes terminating 0 */
    res = cbdata(pcb);
  }
  return res;
}

void ttyputs(const char *str) 
{
  wchar_t buf[120], *wbuf; HANDLE hout; DWORD wc;
  int wlen = MultiByteToWideChar(CP_UTF8, 0, str, -1, NULL, 0);
  if (wlen == 0) return; /* empty string or conversion error */
  if (wlen <= sizeof(buf)/sizeof(wchar_t)) wbuf = buf; 
  else wbuf = cxm_cknull(malloc(wlen*sizeof(wchar_t)), "malloc(tty_puts)");
  MultiByteToWideChar(CP_UTF8, 0, str, -1, wbuf, wlen);
  hout = GetStdHandle(STD_OUTPUT_HANDLE);
  WriteConsoleW(hout, wbuf, (DWORD)wcslen(wbuf), &wc, NULL);
  if (wbuf != buf) free(wbuf);
}

#else
/* ========================= Linux/MacOS TTY ============================== */

typedef char* (*rl_compentry_func_t)(const char*, int);
typedef char** (*rl_attempted_func_t)(const char*, int, int);

typedef struct RL {
  void* h_rl;   // handle for libreadline/libedit
  void* h_hist; // handle for libhistory (GNU readline; optional)

  // readline API
  char* (*readline)(const char* prompt);
  int   (*rl_initialize)(void);
  int   (*rl_variable_bind)(const char* var, const char* value);
  void  (*rl_enable_bracketed_paste)(int enable); // optional (GNU readline >= 8)
  int   (*stifle_history)(int max);               // optional

  // history API
  void  (*using_history)(void);
  int   (*read_history)(const char* filename);
  int   (*write_history)(const char* filename);
  void  (*add_history)(const char* line);

  // global variables (addresses)
  char** rl_readline_name; // pointer to char*
  int* rl_catch_signals; // pointer to int

  // completion API
  rl_attempted_func_t* rl_attempted_completion_function; // address of var
  char** (*rl_completion_matches)(const char*, rl_compentry_func_t);
  rl_compentry_func_t rl_filename_completion_function;

  // optional tuning knobs (may be NULL if not present)
  char** rl_completer_word_break_characters; // pointer to char*
  int* rl_completion_append_character;       // pointer to int

} RL;

static RL g_rl = { 0 };
static char* g_history_path = NULL;
const char* enhterm_lib = NULL;

static void* try_dlopen_many(const char* const* names)
{
  for (size_t i = 0; names[i]; ++i) {
    void* h = dlopen(names[i], RTLD_LAZY | RTLD_LOCAL);
    if (h) { enhterm_lib = (const char *)(names[i]); return h; }
  }
  enhterm_lib = NULL;
  return NULL;
}

static void* getsym_from_handles(RL* rl, const char* sym) {
  void* p = NULL;
  if (rl->h_rl)   p = dlsym(rl->h_rl, sym);
  if (!p && rl->h_hist) p = dlsym(rl->h_hist, sym);
  return p;
}

static bool load_readline(RL* rl) {
#ifdef __APPLE__
  static const char* const cand_rl[] = {
    // Homebrew/MacPorts first (common installs):
    "/opt/homebrew/opt/readline/lib/libreadline.dylib",
    "/usr/local/opt/readline/lib/libreadline.dylib",
    "/opt/local/lib/libreadline.dylib",
    // System names:
    "libreadline.9.dylib", "libreadline.8.dylib", "libreadline.7.dylib", 
    "libreadline.dylib",
    "libedit.3.dylib", "libedit.dylib",
    NULL
  };
  static const char* const cand_hist[] = {
    "/opt/homebrew/opt/readline/lib/libhistory.dylib",
    "/usr/local/opt/readline/lib/libhistory.dylib",
    "/opt/local/lib/libhistory.dylib",
    "libhistory.9.dylib", "libhistory.8.dylib", "libhistory.7.dylib", 
    "libhistory.dylib",
    NULL
  };
#else
  static const char* const cand_rl[] = {
    "libreadline.so.9", "libreadline.so.8", "libreadline.so.7", "libreadline.so.6", 
    "libreadline.so",
    "libedit.so.0", "libedit.so",
    NULL
  };
  static const char* const cand_hist[] = {
    "libhistory.so.9", "libhistory.so.8", "libhistory.so.7", "libhistory.so.6", 
    "libhistory.so",
    NULL
  };
#endif

  memset(rl, 0, sizeof(*rl));

  rl->h_rl = try_dlopen_many(cand_rl);
  if (!rl->h_rl) return false;

  // History may be inside libreadline/libedit or in a separate libhistory
  // Try to resolve from rl first; dlopen libhistory only if needed.
#define LOAD_SYM(ptr, name) do { \
  rl->ptr = (void*)getsym_from_handles(rl, name); \
} while (0)

  rl->readline = (char* (*)(const char*)) getsym_from_handles(rl, "readline");
  rl->rl_initialize = (int(*)(void)) getsym_from_handles(rl, "rl_initialize");
  rl->rl_variable_bind = (int(*)(const char*, const char*))getsym_from_handles(rl, "rl_variable_bind");
  rl->rl_enable_bracketed_paste = (void(*)(int)) getsym_from_handles(rl, "rl_enable_bracketed_paste");
  rl->stifle_history = (int(*)(int)) getsym_from_handles(rl, "stifle_history");

  rl->using_history = (void(*)(void)) getsym_from_handles(rl, "using_history");
  rl->read_history = (int(*)(const char*)) getsym_from_handles(rl, "read_history");
  rl->write_history = (int(*)(const char*)) getsym_from_handles(rl, "write_history");
  rl->add_history = (void(*)(const char*)) getsym_from_handles(rl, "add_history");

  rl->rl_readline_name = (char**)getsym_from_handles(rl, "rl_readline_name");
  rl->rl_catch_signals = (int*)getsym_from_handles(rl, "rl_catch_signals");

  if (!rl->readline) {
    dlclose(rl->h_rl);
    memset(rl, 0, sizeof(*rl));
    return false;
  }

  // If any of the history functions are missing, try libhistory
  if (!rl->using_history || !rl->read_history || !rl->write_history || !rl->add_history) {
    rl->h_hist = try_dlopen_many(cand_hist);
    if (rl->h_hist) {
      rl->using_history = (void(*)(void)) getsym_from_handles(rl, "using_history");
      rl->read_history = (int(*)(const char*)) getsym_from_handles(rl, "read_history");
      rl->write_history = (int(*)(const char*)) getsym_from_handles(rl, "write_history");
      rl->add_history = (void(*)(const char*)) getsym_from_handles(rl, "add_history");
      if (!rl->stifle_history)
        rl->stifle_history = (int(*)(int)) getsym_from_handles(rl, "stifle_history");
    }
  }

  // Attempted completion hook (variable)
  rl->rl_attempted_completion_function =
    (rl_attempted_func_t*)getsym_from_handles(rl, "rl_attempted_completion_function");

  // completion_matches (GNU Readline) or completion_matches (libedit alias)
  rl->rl_completion_matches =
    (char** (*)(const char*, rl_compentry_func_t)) getsym_from_handles(rl, "rl_completion_matches");
  if (!rl->rl_completion_matches) {
    rl->rl_completion_matches =
      (char** (*)(const char*, rl_compentry_func_t)) getsym_from_handles(rl, "completion_matches");
  }

  // filename completion generator
  rl->rl_filename_completion_function =
    (rl_compentry_func_t)getsym_from_handles(rl, "rl_filename_completion_function");
  if (!rl->rl_filename_completion_function) {
    rl->rl_filename_completion_function =
      (rl_compentry_func_t)getsym_from_handles(rl, "filename_completion_function");
  }

  // optional variables (might be absent; that's fine)
  rl->rl_completer_word_break_characters =
    (char**)getsym_from_handles(rl, "rl_completer_word_break_characters");
  rl->rl_completion_append_character =
    (int*)getsym_from_handles(rl, "rl_completion_append_character");

  return true;
}

static char* build_history_path(const char* fname)
{
  const char* home = getenv("HOME");
  if (!home || !*home) {
    // Fallback: current directory
    size_t len = strlen("./") + strlen(fname) + 1;
    char* p = (char*)malloc(len);
    if (!p) return NULL;
    snprintf(p, len, "./%s", fname);
    return p;
  }
  size_t len = strlen(home) + 1 + strlen(fname) + 1;
  char* p = (char*)malloc(len);
  if (!p) return NULL;
  snprintf(p, len, "%s/%s", home, fname);
  return p;
}

// Loaded identifiers (if any)
static char** g_user_identifiers = NULL;
static size_t g_user_identifiers_count = 0;
static bool   g_user_identifiers_loaded = false;

static void free_user_identifiers(void)
{
  if (!g_user_identifiers) return;
  for (size_t i = 0; i < g_user_identifiers_count; ++i) {
    free(g_user_identifiers[i]);
  }
  free(g_user_identifiers);
  g_user_identifiers = NULL;
  g_user_identifiers_count = 0;
  g_user_identifiers_loaded = false;
}

static char* trim_and_dup(const char* s)
{
  if (!s) return NULL;
  const unsigned char* p = (const unsigned char*)s;
  while (*p && isspace(*p)) p++; // skip leading ws
  const unsigned char* q = p;
  while (*q) q++;  // go to end
  while (q > p && isspace(*(q - 1))) q--; // trim trailing ws
  size_t len = (size_t)(q - p);
  char* out = (char*)malloc(len + 1);
  if (!out) return NULL;
  if (len) memcpy(out, p, len);
  out[len] = '\0';
  return out;
}

static bool str_array_contains(char* const* arr, size_t n, const char* s)
{
  for (size_t i = 0; i < n; ++i) {
    if (strcmp(arr[i], s) == 0) return true;
  }
  return false;
}

// Load from ~/.skint_completion; returns true if at least one identifier loaded
static bool load_completion_identifiers(void)
{
  const char* home = getenv("HOME");
  if (!home || !*home) return false;

  size_t path_len = strlen(home) + 1 + strlen(".skint_completion") + 1;
  char* path = (char*)malloc(path_len);
  if (!path) return false;
  snprintf(path, path_len, "%s/%s", home, ".skint_completion");

  FILE* fp = fopen(path, "r");
  free(path);
  if (!fp) return false; // file absent or unreadable

  // Start fresh if reloading
  free_user_identifiers();

  char* line = NULL;
  size_t cap = 0;
  ssize_t nread;
  bool any = false;

  while ((nread = getline(&line, &cap, fp)) != -1) {
    // Strip CRLF and trim
    while (nread > 0 && (line[nread - 1] == '\n' || line[nread - 1] == '\r')) {
      line[--nread] = '\0';
    }
    char* id = trim_and_dup(line);
    if (!id) continue;
    if (id[0] == '\0' || id[0] == '#') { free(id); continue; }

    // Deduplicate
    if (str_array_contains(g_user_identifiers, g_user_identifiers_count, id)) {
      free(id);
      continue;
    }

    char** tmp = (char**)realloc(g_user_identifiers, (g_user_identifiers_count + 1) * sizeof(char*));
    if (!tmp) {
      free(id);
      // If OOM, keep what we already loaded and stop
      break;
    }
    g_user_identifiers = tmp;
    g_user_identifiers[g_user_identifiers_count++] = id;
    any = true;
  }
  free(line);
  fclose(fp);

  g_user_identifiers_loaded = any;
  return any;
}

// Sample identifiers to complete if no completion file
static const char* const k_identifiers[] = {
  "define", "define-syntax", "define-record-type", 
  "define-library", "define-values", "import",
  "syntax-rules", "call-with-current-continuation",
  "call-with-input-file", "call-with-output-file",
  "call-with-valies", "make-parameter", "parameterize",
  "current-input-file", "current-output-file",
  "lambda", "case-lambda",
  NULL
};

static char* completion_generator(const char* text, int state)
{
  static size_t idx;
  static size_t len;

  if (state == 0) {
    idx = 0;
    len = strlen(text);
  }

  if (g_user_identifiers_loaded) {
    while (idx < g_user_identifiers_count) {
      const char* cand = g_user_identifiers[idx++];
      if (strncmp(cand, text, len) == 0) {
        return strdup(cand);
      }
    }
    return NULL;
  } else {
    // Fall back to built-in list
    while (k_identifiers[idx]) {
      const char* cand = k_identifiers[idx++];
      if (strncmp(cand, text, len) == 0) {
        return strdup(cand);
      }
    }
    return NULL;
  }
}

static char** attempted_completion(const char* text, int start, int end)
{
  (void)start; (void)end;
  if (!text) return NULL;

  // If it looks like a path, let the default filename completer handle it
  if (strchr(text, '/') || text[0] == '~' || (text[0] == '.' && text[1] == '/')) {
    return NULL; // NULL => use default (filename) completion
  }

  // Try our identifier list first
  if (g_rl.rl_completion_matches) {
    char** matches = g_rl.rl_completion_matches(text, completion_generator);
    if (matches && matches[0]) {
      // Nice UX: append a space after completing an identifier
      if (g_rl.rl_completion_append_character) {
        *g_rl.rl_completion_append_character = ' ';
      }
      return matches;
    }
  }

  // No identifier matches: fall back explicitly to filenames if available
  if (g_rl.rl_completion_matches && g_rl.rl_filename_completion_function) {
    // Let readline decide append behavior for files/dirs
    if (g_rl.rl_completion_append_character) {
      *g_rl.rl_completion_append_character = 0;
    }
    return g_rl.rl_completion_matches(text, g_rl.rl_filename_completion_function);
  }

  // As a final fallback, return NULL to let readline’s default handler run
  return NULL;
}

static void configure_readline(RL* rl)
{
  // Set locale for UTF-8/multibyte width calculations
  setlocale(LC_ALL, "");

  if (rl->rl_readline_name) {
    *rl->rl_readline_name = "dlreadline_echo";
  }

  // Allow library to initialize internal state if available
  if (rl->rl_initialize) {
    rl->rl_initialize();
  }

  // Enable good defaults for Unicode/UTF-8 usage
  if (rl->rl_variable_bind) {
    // These help with 8-bit/unicode I/O in readline (esp. older setups)
    rl->rl_variable_bind("input-meta", "on");
    rl->rl_variable_bind("convert-meta", "off");
    rl->rl_variable_bind("output-meta", "on");
    rl->rl_variable_bind("blink-matching-paren", "on");
  }

  // Enable bracketed paste (so pasted newlines don't instantly accept the line)
  if (rl->rl_enable_bracketed_paste) {
    rl->rl_enable_bracketed_paste(1);
  } else if (rl->rl_variable_bind) {
    // GNU Readline supports this variable; libedit may ignore it (fine).
    rl->rl_variable_bind("enable-bracketed-paste", "on");
  }

  // History setup
  if (rl->using_history) {
    rl->using_history();
  }
  if (rl->stifle_history) {
    rl->stifle_history(1000); // cap history to a reasonable size
  }

  g_history_path = build_history_path(".skint_history");
  if (g_history_path && rl->read_history) {
    // read_history returns nonzero if file doesn't exist; it's fine to ignore.
    rl->read_history(g_history_path);
  }

  // attempt to load completions from ~/.skint_completion (one if per line)
  load_completion_identifiers(); // generator checks global flag

  // Optional: friendlier completion defaults
  if (rl->rl_variable_bind) {
    rl->rl_variable_bind("completion-ignore-case", "on"); // optional
    // rl->rl_variable_bind("show-all-if-ambiguous", "on"); // optional
  }

  // Install our attempted-completion handler
  if (rl->rl_attempted_completion_function) {
    *rl->rl_attempted_completion_function = attempted_completion;
  }

  // Optional: tweak word breaks (keep '.' and '_' inside words)
  if (rl->rl_completer_word_break_characters && *rl->rl_completer_word_break_characters) {
    // Default is fine; uncomment to customize:
    // *rl->rl_completer_word_break_characters = strdup(" \t\n\"'`@$><=;|&{(");
  }
}

static void cleanup_at_exit(void)
{
  if (g_rl.write_history && g_history_path) {
    g_rl.write_history(g_history_path);
  }
  free(g_history_path);
  g_history_path = NULL;

  if (g_rl.h_hist) dlclose(g_rl.h_hist);
  if (g_rl.h_rl)   dlclose(g_rl.h_rl);
  memset(&g_rl, 0, sizeof(g_rl));

  free_user_identifiers();
}

/* ========================== External functions ============================= */

static int use_readline = 0;

void ttyinit(void) 
{
  if (!isatty(STDIN_FILENO) || !isatty(STDOUT_FILENO)) use_readline = 0;
  else use_readline = load_readline(&g_rl);
  if (use_readline) {
    configure_readline(&g_rl);
  } else {
    // Still set locale so printf handles UTF-8 reasonably
    setlocale(LC_ALL, "");
    //fprintf(stderr, "[info] Readline not found; falling back to fgets().\n");
  }
  atexit(cleanup_at_exit);
}

int ttyisansi(void)
{
  const char *term; struct winsize w;
  if (!isatty(STDOUT_FILENO)) return 0;
  term = getenv("TERM");
  if (!term || !*term || strcmp(term, "dumb") == 0) return 0;
  /* paranoia: ask for cursor position; if the terminal answers, it speaks ANSI */
  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0) return 1;
  return 0;
}

char *ttygetln(const char *prompt, cbuf_t *pcb)
{
  char *buf = cballoc(cbclear(pcb), 257); int bufsz = 256;
  int count; char *res; if (!prompt) prompt = "test}";
  if (!use_readline) {
    if (prompt) fputs(prompt, stdout);
    fflush(stdout);
    res = fgets(buf, bufsz, stdin);
    assert(buf == pcb->buf); assert(!res || res == buf);
    if (res) pcb->fill = pcb->buf + strlen(res);
    return res;
  } else {
    res = g_rl.readline(prompt);
    if (!res) return NULL;
    if (g_rl.add_history && res[0] != 0) g_rl.add_history(res);
    assert(buf == pcb->buf);
    strncpy(buf, res, bufsz); buf[bufsz] = 0;
    free(res);
    pcb->fill = pcb->buf + strlen(buf);
    assert(pcb->fill < pcb->end);
    cbputc('\n', pcb);
    return cbdata(pcb);
  }
}

void ttyputs(const char *line)
{
  if (!isatty(STDIN_FILENO) || !isatty(STDOUT_FILENO)) {
    /* Not a tty (pre-check via fistty!) */
    return;
  } else {
    fputs(line, stdout);  
  }
}
#endif

/* since all ttys are in fact synchronous, they share static copies of ttfile */
static ttfile_t ttyin, ttyout;

static void ttfini(void) {
  ttflush(&ttyout);
  cbfini(&ttyin.cb);
  cbfini(&ttyout.cb);
}

ttfile_t *ttalloc(int in) {
  static int done = 0;
  if (!done) {
    ttyinit();    
    cbinit(&ttyin.cb); ttyin.next = ttyin.cb.buf; *(ttyin.next) = 0;
    ttyin.lno = 0; ttyin.prbuf[0] = 0; ttyin.flags = TTF_NONE;
    cbinit(&ttyout.cb); ttyout.lno = 0; ttyout.flags = TTF_NONE;
    atexit(ttfini);
    done = 1;
  }
  return in ? &ttyin : &ttyout;
}

static void ttfree(ttfile_t *tp) {
  /* closed only at shutdown */
}

static int ttgetch(ttfile_t *tp) {
  int c; 
  assert(tp->next >= tp->cb.buf && tp->next <= tp->cb.fill);
  if (!cbempty(&ttyout.cb)) ttflush(&ttyout);
  retry: c = unextc_check(tp->next);
  if (c == 0x1A) goto eof; /* handle ^Z manually! */
  if (c != 0 || tp->next <= tp->cb.fill) return c; 
  else if (tp->flags & TTF_EOF) {
    goto eof; 
  } else { /* refill with next line from fp */
    cbuf_t *pcb = &tp->cb;
    char *line = ttygetln(tp->prbuf, pcb);
    assert(!line || line == pcb->buf);
    if (!line) { cbclear(pcb); tp->flags |= TTF_EOF; }
    tp->lno += 1; tp->next = line ? line : cbdata(pcb);
    strcpy(tp->prbuf, ""); /* continuation line prompt */
    goto retry;
  }
eof:
  tp->next = cbdata(cbclear(&tp->cb));
  tp->flags |= TTF_EOF;
  return EOF;
}

static int ttungetch(int c, ttfile_t *tp) {
  assert(tp->next > tp->cb.buf && tp->next <= tp->cb.fill);
  tp->next = uungetch(c, &tp->cb, tp->next);
  return c;
}

static int ttctl(ctlop_t op, ttfile_t *tp, ...) {
  switch (op) {
    case CTLOP_RDLN: {
      va_list args; int c, n, **pd;
      va_start(args, tp); 
      pd = va_arg(args, int **);
      c = ttgetch(tp);
      if (c == EOF) {
        *pd = NULL;
      } else {
        char *s; ttungetch(c, tp);
        s = tp->next; n = (int)(tp->cb.fill - s);
        if (n > 0 && s[n-1] == '\n') --n;
        *pd = newsdatan(s, n);
        tp->next = tp->cb.fill;
      }  
      va_end(args);
      return 0;
    } break;
    case CTLOP_CI: {
      va_list args; va_start(args, tp);
      va_end(args);
      return (tp->flags & TTF_CI) != 0;
    } break;
    case CTLOP_SETCI: {
      va_list args; int d; va_start(args, tp);
      d = va_arg(args, int);
      va_end(args);
      if (d) tp->flags |= TTF_CI; else tp->flags &= ~TTF_CI;
      return d != 0;
    } break;
    case CTLOP_SETPROMPT: {
      va_list args; char *s; va_start(args, tp);
      s = va_arg(args, char*);
      va_end(args);
      if (s) {
        strncpy(tp->prbuf, s, sizeof(tp->prbuf));
        tp->prbuf[sizeof(tp->prbuf)-1] = 0;
      } else {
        tp->prbuf[0] = 0;
      }
      return 0;
    } break;
  }
  return -1;
}

static int ttputch(int c, ttfile_t *tp) {
  /* note: there is no point in writing NULs to tty, 
   * so we just skip them */
  if (c) ucbputc(c, &tp->cb);
  if (c == '\n') ttflush(tp);
  return c;
}

static int ttflush(ttfile_t *tp) {
  if (!cbempty(&tp->cb)) ttyputs(cbdata(&tp->cb));
  cbclear(&tp->cb);
  return 0;
}
