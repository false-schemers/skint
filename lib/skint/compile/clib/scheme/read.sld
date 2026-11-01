;;; read.sld -- (scheme read) as a real library, for compiled programs.
;;;
;;; Shadow counterpart of Skint's predefined (scheme read): the predefined
;;; library exports bare bindings to Skint-initialized globals, which a
;;; standalone executable does not have.  Here every procedure name is
;;; either
;;;   * a cover macro: a call in matching position expands to the Skint
;;;     builtin (imported renamed as %builtin-<name>), a bare identifier
;;;     expands to %residual-<name>, an ordinary definition in this
;;;     library (so the compiled program owns a real closure); or
;;;   * a definition copied from Skint's pre/s.scm (the macro/procedure
;;;     library the interpreter itself is built from), adapted only to
;;;     refer to this library's own bindings; or
;;;   * core syntax / an auxiliary identifier re-exported unchanged --
;;;     expansion-time machinery, not a runtime global.
;;; Only builtin denotations are taken from Skint, via (skint) and, for
;;; %-prefixed builtin operations (skint) does not export, (skint hidden).
;;; A real Scheme reader: Skint's %read from pre/s.scm (the\n;;; pure-Scheme, get-token based reader), adapted to the compiled\n;;; runtime:\n;;;   * fold-case is tracked in a local variable -- compiled\n;;;     ports have no fold-case slot (Skint uses port slots).\n;;;   * the \\x escape caps come from current s.scm: at most\n;;;     max-char-hex-digits (6) digits, value <= max-char-code\n;;;     (#x10FFFF); the shared-alist lookup uses assoc.\n;;;   * only #u8(...) constructs a vector (a bytevector); the\n;;;     other numerical vector types and #e/#i exactness have no\n;;;     compiled-runtime objects and raise a read error.\n;;;   * unknown #!name directives are skipped like shebangs;\n;;;     read errors raise via (scheme base)'s error (kind #f --\n;;;     the error-object constructor is not exported there).\n;;;   * string->number is the compiled 2-argument conversion,\n;;;     so ratios/complex literals read back as symbols.\n
(define-library (scheme read)
  (import (only (skint) define lambda define-syntax syntax-rules if _ ...)
          (only (skint) fixnum? flonum? fx+ fx<=?)
          (only (scheme base)
            let let* letrec begin cond case else when unless and or quote set!
            define-record-type apply error
            car cdr cons list pair? null? not eq? eqv? memv assoc
            set-car! set-cdr! vector-length vector-ref vector-set!
            list->vector list->string string-length string-ref
            string-append string=? string? string->symbol symbol->string symbol?
            string->number exact inexact
            char? char=? char<=?
            integer->char char->integer bytevector
            input-port? eof-object? eof-object
            read-char peek-char read-line current-input-port
            vector? + - < = >)
          (only (scheme box) box box? unbox set-box!)
          (only (scheme char) string-foldcase
            char-whitespace? char-numeric? char-alphabetic?))
  (export read)
  (begin

    ;; -- read errors: raise a base error object (see header note).
    (define (read-error p msg . args) (apply error msg args))
    (define-syntax r-error
      (syntax-rules () [(_ p msg a ...) (read-error p msg a ...)]))

    ;; reader tokens: (marker . description) pairs, as in current s.scm
    (define reader-token-marker (list 'reader-token))
    (define close-paren (cons reader-token-marker "right parenthesis"))
    (define close-bracket (cons reader-token-marker "right bracket"))
    (define dot (cons reader-token-marker "\" . \""))
    (define (reader-token? form)
      (and (pair? form) (eq? (car form) reader-token-marker)))

    ;; a shared (#n=) reference: resolved by the nmr pass once the
    ;; whole datum is read; loc is a box holding the target datum.
    (define-record-type <shared-ref>
      (make-shared-ref loc)
      shared-ref?
      (loc shared-ref-loc))

    (define opt-fold-case #f)         ; no number tower in this runtime
    (define max-char-code #x10FFFF)   ; Unicode range, as in current s.scm
    (define max-char-hex-digits 6)

    (define (%reverse! l)
      (let loop ([l l] [r '()])
        (if (null? l) r
            (let ([t (cdr l)]) (set-cdr! l r) (loop t l)))))

    (define (%read p simple? ci?)
      (unless (input-port? p) (r-error #f "input port expected" p))
      (let ([shared '()]
            [fold-case? (or ci? opt-fold-case)])

        (define (char-delimiter? c)
          (or (eof-object? c) (char-whitespace? c)
              (memv c '(#\( #\) #\[ #\] #\" #\; #\# #\|))))

        (define (char-hexdigit? c)
          (or (char-numeric? c)
              (and (char<=? #\a c) (char<=? c #\f))
              (and (char<=? #\A c) (char<=? c #\F))))

        (define (char-constituent? c)
          (and (char? c) (not (char-whitespace? c))
               (not (memv c '(#\( #\) #\[ #\] #\" #\' #\` #\, #\; #\# #\| #\\)))))

        (define (token-start-number? c)
          (or (char-numeric? c) (memv c '(#\+ #\- #\.))))

        (define (%string->fixnum s radix)
          (let ([n (string->number s radix)])
            (if (fixnum? n) n (r-error p "number is not a fixnum" s))))

        ;; first char already consumed; returns a string, or the dot token
        (define (get-token first)
          (let loop ([l (list first)] [c (peek-char p)])
            (cond [(char-constituent? c)
                   (read-char p) (loop (cons c l) (peek-char p))]
                  [else (let ([t (list->string (%reverse! l))])
                          (if (string=? t ".") dot t))])))

        (define (get-symbol-from-token t)
          (string->symbol (if fold-case? (string-foldcase t) t)))

        (define (get-symbol)
          (let ([t (get-token (read-char p))])
            (if (eq? t dot) dot (get-symbol-from-token t))))

        (define (sub-read-carefully)
          (let ([form (sub-read #f)])
            (cond [(eof-object? form) (r-error p "unexpected end of file")]
                  [(reader-token? form) (r-error p "unexpected token:" (cdr form))]
                  [else form])))

        (define (sub-read-line-comment)
          (let loop ()
            (let ([ch (read-char p)])
              (unless (or (eof-object? ch) (char=? ch #\newline)) (loop)))))

        (define (sub-read-block-comment)
          (let loop ([depth 1])
            (let ([c (read-char p)])
              (cond [(eof-object? c) (r-error p "end of file in #| comment")]
                    [(and (char=? c #\#) (eqv? (peek-char p) #\|))
                     (read-char p) (loop (+ depth 1))]
                    [(and (char=? c #\|) (eqv? (peek-char p) #\#))
                     (read-char p) (when (> depth 1) (loop (- depth 1)))]
                    [else (loop depth)]))))

        (define (sub-read-x-char-escape in-string?)
          (define (rev-digits->char l)
            (if (null? l) (r-error p "\\x escape sequence is too short")
                (let ([n (%string->fixnum (list->string (%reverse! l)) 16)])
                  (if (> n max-char-code)
                      (r-error p "\\x escape sequence is too large" n)
                      (integer->char n)))))
          (let loop ([l '()] [cc 0])
            (let ([c (peek-char p)])
              (cond [(and in-string? (eqv? c #\;))
                     (read-char p) (rev-digits->char l)]
                    [(and (char? c) (char-hexdigit? c) (< cc max-char-hex-digits))
                     (read-char p) (loop (cons c l) (+ cc 1))]
                    [(and (not in-string?) (or (eof-object? c) (char-delimiter? c)))
                     (rev-digits->char l)]
                    [else (r-error p "invalid char in \\x escape sequence" c)]))))

        (define (sub-read-strsym-char-escape what)
          (let ([c (read-char p)])
            (when (eof-object? c) (r-error p "end of file within a" what))
            (cond [(or (char=? c #\\) (char=? c #\") (char=? c #\|)) c]
                  [(char=? c #\a) #\alarm]
                  [(char=? c #\b) #\backspace]
                  [(char=? c #\t) #\tab]
                  [(char=? c #\n) #\newline]
                  [(char=? c #\v) #\vtab]
                  [(char=? c #\f) #\page]
                  [(char=? c #\r) #\return]
                  [(char=? c #\x) (sub-read-x-char-escape #t)]
                  [(and (eq? what 'string) (char-whitespace? c))
                   (let loop ([gotnl (char=? c #\newline)] [nc (peek-char p)])
                     (cond [(or (eof-object? nc) (not (char-whitespace? nc)))
                            (if gotnl #f (r-error p "no newline in line ending escape"))]
                           [(and gotnl (char=? nc #\newline)) #f]
                           [else (read-char p)
                                 (loop (or gotnl (char=? nc #\newline)) (peek-char p))]))]
                  [else (r-error p "invalid char escape in" what ': c)])))

        (define (sub-read-string)
          (let loop ([l '()])
            (let ([c (read-char p)])
              (cond [(eof-object? c) (r-error p "end of file within a string")]
                    [(char=? c #\\)
                     (let ([e (sub-read-strsym-char-escape 'string)])
                       (loop (if e (cons e l) l)))]
                    [(char=? c #\") (list->string (%reverse! l))]
                    [else (loop (cons c l))]))))

        (define (sub-read-barsym)
          (let loop ([l '()])
            (let ([c (read-char p)])
              (cond [(eof-object? c) (r-error p "end of file within a |symbol|")]
                    [(char=? c #\\)
                     (let ([e (sub-read-strsym-char-escape 'symbol)])
                       (loop (if e (cons e l) l)))]
                    [(char=? c #\|) (string->symbol (list->string (%reverse! l)))]
                    [else (loop (cons c l))]))))

        (define (sub-read-char-literal)
          (let ([c (peek-char p)])
            (cond [(eof-object? c) (r-error p "end of file after #\\")]
                  [(char=? #\x c)
                   (read-char p)
                   (if (char-delimiter? (peek-char p)) c (sub-read-x-char-escape #f))]
                  [(char-alphabetic? c)
                   (let ([name (sub-read-carefully)])
                     (if (= (string-length (symbol->string name)) 1)
                         c
                         (case name
                           [(null) (integer->char 0)]
                           [(space) #\space]
                           [(alarm) #\alarm]
                           [(backspace) #\backspace]
                           [(delete) (integer->char #x7F)]
                           [(escape) (integer->char #x1B)]
                           [(tab) #\tab]
                           [(newline linefeed) #\newline]
                           [(vtab) #\vtab]
                           [(page) #\page]
                           [(return) #\return]
                           [else (r-error p "unknown #\\ name" name)])))]
                  [else (read-char p) c])))

        (define (sub-read-list c close-token dot?)
          (let ([form (sub-read #f)])
            (if (eq? form dot)
                (r-error p "missing car -- ( immediately followed by .")
                (let recur ([form form])
                  (cond [(eof-object? form)
                         (r-error p "eof inside list -- unbalanced parentheses")]
                        [(eq? form close-token) (read-char p) '()]
                        [(eq? form dot)
                         (if dot?
                             (let* ([last-form (sub-read-carefully)]
                                    [another-form (sub-read #f)])
                               (if (eq? another-form close-token)
                                   (begin (read-char p) last-form)
                                   (r-error p "randomness after form after dot" another-form)))
                             (r-error p "dot in #(...)"))]
                        [(reader-token? form)
                         (r-error p "error inside list --" (cdr form))]
                        [else (cons form (recur (sub-read #f)))])))))

        (define (sub-read-numerical-list ts)
          (unless (eq? ts 'u8)
            (r-error p "unsupported numerical vector type in compiled programs" ts))
          (unless (eq? (read-char p) #\()
            (r-error p (string-append "invalid " (symbol->string ts) "vector syntax")))
          (let recur ([form (sub-read #f)])
            (cond [(eof-object? form) (r-error p "eof inside #u8 vector")]
                  [(eq? form close-paren) (read-char p) '()]
                  [(reader-token? form) (r-error p "error inside #u8 vector --" (cdr form))]
                  [(and (fixnum? form) (fx<=? 0 form) (fx<=? form 255))
                   (cons form (recur (sub-read #f)))]
                  [else (r-error p "invalid element inside #u8 vector --" form)])))

        (define (list->numvector lst ts)
          (if (eq? ts 'u8) (apply bytevector lst)
              (r-error p "unsupported numerical vector type in compiled programs" ts)))

        (define (sub-read-exactness-marker ec)
          (let ([t (get-token (read-char p))])
            (unless (string? t) (r-error p "random token after number prefix --" t))
            (let* ([radix (case ec [(#\b) 2] [(#\o) 8] [(#\d) 10] [(#\x) 16] [else #f])]
                   [n (string->number t radix)])
              (cond [(not n) (r-error p "invalid number syntax" t)]
                    [(char=? ec #\e) (if (flonum? n) (exact n) n)]
                    [(char=? ec #\i) (if (fixnum? n) (inexact n) n)]
                    [else n]))))

        (define (sub-read-datum-label)
          (when simple? (r-error p "#N=/#N# notation is not allowed in this mode"))
          (let loop ([l '()])
            (let ([c (read-char p)])
              (cond [(eof-object? c) (r-error p "end of file within a #N notation")]
                    [(char-numeric? c)
                     (loop (if (and (null? l) (char=? c #\0)) l (cons c l)))]
                    [(char=? c #\=)
                     (let ([s (if (null? l) "0" (list->string (%reverse! l)))])
                       (when (assoc s shared) (r-error p "duplicate #n= tag:" s))
                       (let* ([loc (box #f)] [ref (make-shared-ref loc)])
                         (set! shared (cons (cons s ref) shared))
                         (let ([form (sub-read-carefully)])
                           (cond [(eq? form ref) (r-error p "#n= has itself as target" s)]
                                 [else (set-box! loc form) form]))))]
                    [(char=? c #\#)
                     (let ([s (if (null? l) "0" (list->string (%reverse! l)))])
                       (let ([entry (assoc s shared)])
                         (if entry (cdr entry)
                             (r-error p "undefined #n# tag:" s))))]
                    [else (r-error p "invalid terminator for #N notation")]))))

        (define (sub-read-settings top)
          (cond [(memv (peek-char p) '(#\space #\/))
                 (read-line p) (sub-read top)]
                [else
                 (let ([name (get-symbol)])
                   (case name
                     [(fold-case) (set! fold-case? #t) (sub-read top)]
                     [(no-fold-case) (set! fold-case? #f) (sub-read top)]
                     [else (sub-read top)]))]))

        (define (sub-read-sharp top)
          (let ([c (peek-char p)])
            (cond [(eof-object? c) (r-error p "end of file after #")]
                  [(char=? c #\!) (read-char p) (sub-read-settings top)]
                  [(char=? c #\() (read-char p) (list->vector (sub-read-list c close-paren #f))]
                  [(char=? c #\\) (read-char p) (sub-read-char-literal)]
                  [(char=? c #\;) (read-char p) (sub-read-carefully) (sub-read top)]
                  [(char=? c #\|) (read-char p) (sub-read-block-comment) (sub-read top)]
                  [(char=? c #\') (read-char p) (list 'syntax (sub-read-carefully))]
                  [(char=? c #\`) (read-char p) (list 'quasisyntax (sub-read-carefully))]
                  [(char=? c #\,) (read-char p)
                   (if (eqv? (peek-char p) #\@)
                       (begin (read-char p) (list 'unsyntax-splicing (sub-read-carefully)))
                       (list 'unsyntax (sub-read-carefully)))]
                  [(char=? c #\&) (read-char p) (box (sub-read-carefully))]
                  [(or (char=? c #\e) (char=? c #\i) (char=? c #\b)
                       (char=? c #\o) (char=? c #\d) (char=? c #\x))
                   (read-char p) (sub-read-exactness-marker c)]
                  [(char-numeric? c) (sub-read-datum-label)]
                  [else
                   (let* ([name0 (get-symbol)]
                          [name (if (symbol? name0)
                                    (string->symbol
                                      (string-foldcase (symbol->string name0)))
                                    name0)])
                     (case name
                       [(t true) #t]
                       [(f false) #f]
                       [(u8) (list->numvector (sub-read-numerical-list 'u8) 'u8)]
                       [(s8 u16 s16 u32 s32 u64 s64 f32 f64 c64 c128)
                        (r-error p "unsupported numerical vector type in compiled programs" name)]
                       [else (r-error p "unknown # syntax" name)]))])))

        (define (sub-read top)
          (let ([c (peek-char p)])
            (cond [(eof-object? c) (eof-object)]
                  [(char-whitespace? c) (read-char p) (sub-read top)]
                  [(char=? c #\() (read-char p) (sub-read-list c close-paren #t)]
                  [(char=? c #\)) close-paren]
                  [(char=? c #\[) (read-char p) (sub-read-list c close-bracket #t)]
                  [(char=? c #\]) close-bracket]
                  [(char=? c #\;) (sub-read-line-comment) (sub-read top)]
                  [(char=? c #\') (read-char p) (list 'quote (sub-read-carefully))]
                  [(char=? c #\`) (read-char p) (list 'quasiquote (sub-read-carefully))]
                  [(char=? c #\,) (read-char p)
                   (let ([next (peek-char p)])
                     (cond [(eof-object? next) (r-error p "end of file after ,")]
                           [(char=? next #\@)
                            (read-char p) (list 'unquote-splicing (sub-read-carefully))]
                           [else (list 'unquote (sub-read-carefully))]))]
                  [(char=? c #\") (read-char p) (sub-read-string)]
                  [(char=? c #\|) (read-char p) (sub-read-barsym)]
                  [(char=? c #\#) (read-char p) (sub-read-sharp top)]
                  [(token-start-number? c)
                   (let ([t (get-token (read-char p))])
                     (if (string? t)
                         (or (string->number t)
                             (and (char=? (string-ref t 0) #\.)
                                  (string->number (string-append "0" t)))
                             (get-symbol-from-token t))
                         t))]
                  [else (get-symbol)])))

        ;; resolve shared refs in place (structure may be cyclic)
        (define (nmr form)
          (cond [(shared-ref? form) (unbox (shared-ref-loc form))]
                [(pair? form)
                 (let loop ([f form])
                   (set-car! f (nmr (car f)))
                   (let ([d (cdr f)])
                     (cond [(pair? d) (loop d)]
                           [else (set-cdr! f (nmr d))])))
                 form]
                [(vector? form)
                 (let loop ([i 0])
                   (when (< i (vector-length form))
                     (vector-set! form i (nmr (vector-ref form i)))
                     (loop (+ i 1))))
                 form]
                [(box? form) (set-box! form (nmr (unbox form))) form]
                [else form]))

        (let ([form (sub-read #t)])
          (cond [(reader-token? form) (r-error p "unexpected token:" (cdr form))]
                [else (nmr form)]))))

    (define read
      (lambda args
        (if (null? args) (%read (current-input-port) #f #f)
            (%read (car args) #f #f))))

    (define read-simple
      (lambda args
        (if (null? args) (%read (current-input-port) #t #f)
            (%read (car args) #t #f))))

    (define read-simple-ci
      (lambda args
        (if (null? args) (%read (current-input-port) #t #t)
            (%read (car args) #t #t))))
))
