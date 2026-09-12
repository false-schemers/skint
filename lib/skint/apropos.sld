
(define-library (skint apropos)

  (import
    (scheme base)
    (scheme write)
    (scheme case-lambda)
    (scheme repl)
    (only (skint) string-position void)
    (only (skint hidden)
      *root-name-registry* *user-name-registry*
      library-exports val-library? location-val location?
      global-store))

  (export
    apropos
    apropos-list
    apropos-expose-hidden)

(begin

;; --------------------------------------------------------------------------
;; The pattern
;; --------------------------------------------------------------------------

(define (pattern-string s)
  (cond [(string? s) s]
        [(symbol? s) (symbol->string s)]
        [else (error "apropos: not a string or a symbol" s)]))

(define (matches? pat id)
  (and (string-position pat (symbol->string id)) #t))

(define (matching pat ids)
  (let loop ([l ids] [r '()])
    (cond [(null? l) r]
          [(matches? pat (car l)) (loop (cdr l) (cons (car l) r))]
          [else (loop (cdr l) r)])))

;; --------------------------------------------------------------------------
;; Sorting, and the two orders needed
;; --------------------------------------------------------------------------

(define (msort l less?)
  (define (merge a b)
    (cond [(null? a) b]
          [(null? b) a]
          [(less? (car b) (car a)) (cons (car b) (merge a (cdr b)))]
          [else (cons (car a) (merge (cdr a) b))]))
  (define (split l a b)
    (if (null? l) (values a b) (split (cdr l) b (cons (car l) a))))
  (if (or (null? l) (null? (cdr l)))
      l
      (let-values ([(a b) (split l '() '())])
        (merge (msort a less?) (msort b less?)))))

(define (dedup l)
  (cond [(or (null? l) (null? (cdr l))) l]
        [(eq? (car l) (cadr l)) (dedup (cdr l))]
        [else (cons (car l) (dedup (cdr l)))]))

(define (id<? a b) (string<? (symbol->string a) (symbol->string b)))

(define (sorted-ids l) (dedup (msort l id<?)))

;; A library name is a list of symbols and exact non-negative integers.  Order
;; the parts numbers first, then alphabetically; a name that is a prefix of
;; another comes first, so (rnrs) precedes (rnrs sorting).
(define (part<? x y)
  (cond [(and (number? x) (number? y)) (< x y)]
        [(number? x) #t]
        [(number? y) #f]
        [else (id<? x y)]))

(define (libname<? a b)
  (cond [(null? a) (pair? b)]
        [(null? b) #f]
        [(part<? (car a) (car b)) #t]
        [(part<? (car b) (car a)) #f]
        [else (libname<? (cdr a) (cdr b))]))

;; --------------------------------------------------------------------------
;; What the system knows about
;; --------------------------------------------------------------------------

;; Every library the system has predefined, defined or loaded, paired with its
;; export list -- which holds exactly that library's public identifiers.
;;
;; A registry keeps library names in its last bucket.  Predefined libraries and
;; the ones loaded from a .sld file are in the root registry; one defined by a
;; define-library form typed at the top level goes into the user registry, and
;; shadows a root entry of the same name, so the user registry is looked at
;; first and a name already seen is not taken again.
(define (registry-libraries nr r)
  (let loop ([l (vector-ref nr (- (vector-length nr) 1))] [r r])
    (cond [(null? l) r]
          [else
           (let ([name (caar l)] [val (location-val (cdar l))])
             (loop (cdr l)
                   (if (and (val-library? val) (not (assoc name r)))
                       (cons (cons name (library-exports val)) r)
                       r)))])))

(define (known-libraries)
  (registry-libraries *root-name-registry*
                      (registry-libraries *user-name-registry* '())))

;; The identifiers a registry binds.  Its last bucket holds library names and is
;; skipped.
(define (registry-ids nr)
  (let ([n-1 (- (vector-length nr) 1)])
    (let loop ([i 0] [r '()])
      (if (>= i n-1)
          r
          (let scan ([l (vector-ref nr i)] [r r])
            (cond [(null? l) (loop (+ i 1) r)]
                  [(symbol? (caar l)) (scan (cdr l) (cons (caar l) r))]
                  [else (scan (cdr l) r)]))))))

;; A name in the global store is <prefix>://<path>?<identifier> when some
;; environment allocated it and the bare identifier when it is a built-in.  The
;; prefix is %-encoded and so cannot hold a literal ?, which makes the first one
;; after the :// the end of it.
(define (store-identifier sym)
  (let* ([s (symbol->string sym)] [n (string-length s)])
    (let scan ([i 0])
      (cond [(> (+ i 3) n) sym]
            [(and (char=? (string-ref s i) #\:)
                  (char=? (string-ref s (+ i 1)) #\/)
                  (char=? (string-ref s (+ i 2)) #\/))
             (let mark ([j (+ i 3)])
               (cond [(>= j n) sym]
                     [(char=? (string-ref s j) #\?)
                      (if (>= (+ j 1) n) sym (string->symbol (substring s (+ j 1) n)))]
                     [else (mark (+ j 1))]))]
            [else (scan (+ i 1))]))))

(define (store-identifiers)
  (let* ([gs (global-store)] [n (vector-length gs)])
    (let loop ([i 0] [r '()])
      (if (>= i n)
          r
          (let scan ([l (vector-ref gs i)] [r r])
            (if (null? l)
                (loop (+ i 1) r)
                (scan (cdr l) (cons (store-identifier (caar l)) r))))))))

;; Every identifier that could be bound anywhere: the two registries the
;; interaction environment is built from, the identifier part of every name in
;; the global store, and every library's exports.  An environment is a
;; procedure, so it cannot be enumerated -- only asked about a name -- and this
;; is the set of names worth asking about.
(define (all-identifiers)
  (let loop ([l (known-libraries)]
             [r (append (registry-ids *root-name-registry*)
                        (registry-ids *user-name-registry*)
                        (store-identifiers))])
    (if (null? l)
        r
        (let scan ([eal (cdar l)] [r r])
          (if (null? eal)
              (loop (cdr l) r)
              (scan (cdr eal) (cons (caar eal) r)))))))

;; --------------------------------------------------------------------------
;; Asking an environment
;; --------------------------------------------------------------------------

;; `peek' is the one operation that says where a name resides without
;; allocating a location for it, which is what makes this safe to do for
;; thousands of names: it answers with the location when the name is bound and
;; with the registry the name would go into when it is not.
(define (bound-in? env id)
  (location? (env id 'peek)))

(define (environment-ids pat env)
  (let loop ([l (sorted-ids (matching pat (all-identifiers)))] [r '()])
    (cond [(null? l) (reverse r)]
          [(bound-in? env (car l)) (loop (cdr l) (cons (car l) r))]
          [else (loop (cdr l) r)])))

;; (skint hidden) is the interpreter's own inside: everything the expander and
;; the runtime need and no program should reach for.  Its contents change
;; between releases and are not part of any contract, so it is left out of the
;; answer unless it is asked for by name.
(define apropos-expose-hidden (make-parameter #f (lambda (x) (and x #t))))

(define (hidden-library? name)
  (and (not (apropos-expose-hidden)) (equal? name '(skint hidden))))

(define (library-rows pat)
  (let loop ([l (known-libraries)] [r '()])
    (cond
      [(null? l) (msort r (lambda (a b) (libname<? (car a) (car b))))]
      [(hidden-library? (caar l)) (loop (cdr l) r)]
      [else
       (let ([ids (sorted-ids (matching pat (map car (cdar l))))])
         (loop (cdr l) (if (null? ids) r (cons (cons (caar l) ids) r))))])))

;; --------------------------------------------------------------------------
;; apropos-list
;; --------------------------------------------------------------------------

(define (%apropos-list s env)
  (unless (procedure? env) (error "apropos: not an environment" env))
  (let ([pat (pattern-string s)])
    (append (environment-ids pat env) (library-rows pat))))

(define apropos-list
  (case-lambda
    [(s) (%apropos-list s (interaction-environment))]
    [(s env) (%apropos-list s env)]))

;; --------------------------------------------------------------------------
;; apropos
;; --------------------------------------------------------------------------

(define apropos-width 78)

;; one indented, comma-separated, filled line group
(define (show-ids ids)
  (let loop ([l ids] [col 0])
    (unless (null? l)
      (let* ([s (symbol->string (car l))]
             [more? (pair? (cdr l))]
             [w (+ (string-length s) (if more? 1 0))]
             [fresh? (or (= col 0) (> (+ col 1 w) apropos-width))])
        (cond [(= col 0) (display "  ")]
              [fresh? (newline) (display "  ")]
              [else (display " ")])
        (display s)
        (when more? (display ","))
        (loop (cdr l) (if fresh? (+ 2 w) (+ col 1 w))))))
  (newline))

(define (%apropos s env)
  (let ([l (%apropos-list s env)])
    (let split ([l l] [ids '()])
      (cond
        [(and (pair? l) (symbol? (car l))) (split (cdr l) (cons (car l) ids))]
        [else
         (unless (null? ids)
           (display (if (eq? env (interaction-environment))
                        "interaction environment:"
                        "environment:"))
           (newline)
           (show-ids (reverse ids)))
         (let rows ([l l])
           (unless (null? l)
             (write (caar l)) (display ":") (newline)
             (show-ids (cdar l))
             (rows (cdr l))))
         (void)]))))

(define apropos
  (case-lambda
    [(s) (%apropos s (interaction-environment))]
    [(s env) (%apropos s env)]))

))
