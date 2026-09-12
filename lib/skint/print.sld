(define-library (skint print)

  (import (scheme base) (scheme char) (scheme cxr) 
          (scheme case-lambda) (scheme inexact)
          (scheme file) (scheme read) (scheme write))

  (import (only (skint) 
           box? box unbox 
           numvector? numvector-length numvector-ref))

  ; procedures
  (export print pretty-print)

  ; configuration
  (export pretty-style)

  ; parameters
  (export print-width
          print-circle
          print-graph
          print-radix
          print-length
          print-level
          print-indent
          print-brackets)

(begin

; parameters

; default values are chosen for print to follow write
; (single-line, no newline at the end, nothing fancy)

; number of columns to print within (ideally) unless
; we are printing inline (print-indent is #f)
(define (cv-width x)
  (if (and (number? x) (exact? x) (> x 0)) x
      (error "invalid value for print-width" x)))
(define print-width (make-parameter 80 cv-width))

; detect and mark cyclic substructure
(define (cv-boolean x) (not (not x)))
(define print-circle (make-parameter #t cv-boolean))

; detect and mark shared AND cyclic substructure
(define print-graph (make-parameter #f cv-boolean))

; initial indent (first line assumption) or #f for inline printing
(define (cv-indent x)
  (if (or (not x) (and (number? x) (exact? x) (>= x 0))) x
      (error "invalid value for print-indent" x)))
(define print-indent (make-parameter #f cv-indent)) ; inline

; print argument obj as code, opposed to data (not public)
(define print-code (make-parameter #f cv-boolean))

; radix for (at least) exact integers; 2, 8, 10, 16 are supported
; if not 10, prefix is printed when needed to keep numbers readable
; inexact numbers printed in a system-dependent machine-readable way
(define (cv-radix x)
  (case x [(2 8 10 16) x] [else (error "invalid value for print-radix" x)]))
(define print-radix (make-parameter 10 cv-radix))

; #f or max numbers of subitems to display in a sequence
(define (cv-length x)
  (if (or (not x) (and (number? x) (exact? x) (>= x 0))) x
      (error "invalid value for print-length" x)))
(define print-length (make-parameter #f cv-length)) ; no limit

; #f or max depth of subitems to display in a sequence
(define (cv-level x)
  (if (or (not x) (and (number? x) (exact? x) (>= x 0))) x
      (error "invalid value for print-level" x)))
(define print-level (make-parameter #f cv-level)) ; no limit

; print r6rs-style square brackets if printing code
(define print-brackets (make-parameter #f cv-boolean))

; interface to formatting style registry
(define (pretty-style sym . args)
  (let ([tab (print-styles)])
    (if (null? args)
        (table-ref tab sym #f)
        (begin
          (table-set! tab sym (car args))
          #f)))) 

; predicate-based hooks for nonstandard data
(define print-hooks (make-parameter '()))

; adding a hook to the explicit hook registry
(define (add-print-hook hooks pred . opt-hook)
  (define hook (if (null? opt-hook) #f (car opt-hook)))
  (if (assv pred hooks)
      (alist-addv pred hook hooks) ; replaces at its original pos
      (cons (cons pred hook) hooks))) ; adds to front

; generalized read macro hook constructor
(define (rmac-print-hook pfx reff tomf)
  (list 'rm pfx reff tomf))

; generalized list hook constructor
(define (glist-print-hook pfx tolf toxf sfx)
  (list 'gl pfx tolf toxf sfx))

; binary vector hook constructor
(define (bvec-print-hook pfx lenf reff sfx)
  (list 'bv pfx lenf reff sfx))

; atomic hook constructor
(define (atom-print-hook sh? widf wrtf)
  (list 'at sh? widf wrtf))

; portable eq table, similar to R6RS(?) eq-hashtable   

(define (make-eq-table)
  (cons 'table '()))

(define (table-ref ht key default)
  (cond [(assq key (cdr ht)) => cdr]
        [else default]))

(define (table-set! ht key val)
  (let ([pair (assq key (cdr ht))])
    (if pair
        (set-cdr! pair val)
        (set-cdr! ht (cons (cons key val) (cdr ht))))))


; formatting style registry (private)
(define print-styles (make-parameter (make-eq-table)))

; functional modifications of alists 
(define (alist-addv key val alist) ; adds new to the end!
  (let loop ([l alist])
    (cond [(null? l) (list (cons key val))]
          [(eqv? key (caar l))
           (if (equal? val (cdar l)) l (cons (cons key val) (cdr l)))]
          [else (let ([rest (loop (cdr l))])
                  (if (eq? rest (cdr l)) l (cons (car l) rest)))])))

; char-width, returning #f 0 1 2
(define (char-width ch)
  (if (char<=? #\space ch #\~) 1 #f))

; TODO: in Unicode setting may takes 0-wide and 2-wide chars into account
(define (string-width s)
  (define n (string-length s))
  (let loop ([i 0] [w 0])
    (if (= i n) w ; fixme: check for \\ \" ??
        (let* ([ch (string-ref s i)] [cw (char-width ch)])
          (loop (+ i 1) (+ w (or cw (if (char<? ch #\space) 2 7))))))))

(define (quoted-string-width s) ; add "" or || quotes
  (+ 2 (string-width s)))

; width of written representation (slow but exact)
(define (written-width x)
  (let* ([p (open-output-string)] ; slow but exact
         [s (begin (write x p) (get-output-string p))])
    (close-output-port p) (string-width s)))

; this list should contain rmacs supported by the reader by default
; fixme: add a way to space ,@ if followed by a symbol that starts with @
(define builtin-read-macros 
 '((quote "'" d)
   (quasiquote "`" e)
   (unquote "," e)
   (unquote-splicing ",@" e)))

; the body of the formatter is embeded into pp to allow direct
; access to the external parameters through the local environment
; instead of threading them through the code
(define (pp sexp . rest)
  ; port is always distinguishable from a param procedure
  (define-values (*port* kv*) 
    (if (and (pair? rest) (output-port? (car rest))) 
        (values (car rest) (cdr rest))
        ; if port is not given as optional, look for the kw
        (values (current-output-port) rest)))

  ; we use parameters themselves as keys: they are unique procedures
  ; note: pp only searches for and calls parameters it *knows*, not 
  ; arbitrary args!!
  (define (kval args param . oconv)
    (define conv (if (pair? oconv) (car oconv) (lambda (x) x)))
    (let loop ([a args])
      (cond [(null? a) (param)] ; already converted, no need for conv
            [(and (pair? (cdr a)) (eq? (car a) param)) (conv (cadr a))]
            [(and (pair? (cdr a)) (procedure? (car a))) (loop (cddr a))]
            [else (error "invalid printer parameter list" a)])))

  ; bring in all external parameters as lexical vars
  (define *width*            (kval kv* print-width cv-width))
  (define *circle*           (kval kv* print-circle cv-boolean))
  (define *graph*            (kval kv* print-graph cv-boolean))
  (define *radix*            (kval kv* print-radix cv-radix))
  (define *length*           (kval kv* print-length cv-length))
  (define *level*            (kval kv* print-level cv-level))
  (define *indent*           (kval kv* print-indent cv-indent))
  (define *code*             (kval kv* print-code cv-boolean))
  (define *brackets*         (and *code* (kval kv* print-brackets cv-boolean)))
  (define *tab*              1)
  (define *alt-tab*          5)
  (define *miser-width*      #f)
  (define *inline-width*     60)
  (define *length-stub*      "...")
  (define *level-stub*       #t) ; string (e.g. "#") or #t for Chez "shells"

  ; params to be zeroed if we are within *miser-width* from width
  (define (std-indent-miser ind) 
    (if (and ind (>= (+ ind *miser-width*) *width*)) 0 *tab*))
  (define std-indent
    (if *miser-width* std-indent-miser (lambda (ind) *tab*)))
  (define (alt-indent-miser ind) 
    (if (and ind (>= (+ ind *miser-width*) *width*)) 0 *alt-tab*))
  (define alt-indent
    (if *miser-width* alt-indent-miser (lambda (ind) *alt-tab*)))

  ; indentation ind is either an exact nonnegative integer or #f meaning 
  ; "we are printing inline code, so it doesn't matter"
  ; safe increment for indentation (handles #f)
  (define (ind+ i n) (and i (+ i n)))

  ; initial environment and cutoff machinery
  ; level/length environment is #f or a pair (cur-index . cur-level)
  (define nop (lambda (v) v))
  (define non (if *length* (lambda (v) (cons 0 (cdr v))) nop))
  (define step (if *length* (lambda (v) (cons (+ (car v) 1) (cdr v))) nop))
  (define nest (if *level*  (lambda (v) (cons 0 (+ (cdr v) 1))) non))
  ; improper tail is not a step ahead of the previous element!
  (define back (lambda (v) (cons (- (car v) 1) (cdr v))))
  (define nocut (lambda (v) #f))
  (define cuti? (if *length* (lambda (v) (>= (car v) *length*)) nocut))
  (define cutd? (if *level*  (lambda (v) (>= (cdr v) *level*)) nocut))
  (define env (cons 0 0)) ; ignored if no cutoffs are set
  (define cuti-wid (string-width *length-stub*))
  (define cutd-widf 
    (if (string? *level-stub*) 
        (let ([w (string-width *level-stub*)])
          (lambda (x) ; CL-like model
            (dispatch-on-type x
              (lambda (pfx reff tomf) w) ; #
              (lambda (pfx tolf toxf sfx) w) ; #
              (lambda (pfx lenf reff sfx) w) ; #
              (lambda (sh? widf wrtf) (widf x))))) ; atom
        (lambda (x) ; Chez-like model, fancy!
          (dispatch-on-type x
            (lambda (pfx reff tomf) ; '...
              (+ (string-width pfx) cuti-wid))
            (lambda (pfx tolf toxf sfx) ; (...)
              (+ (string-width pfx) cuti-wid (string-width sfx)))
            (lambda (pfx lenf reff sfx) ; #u8(...)
              (+ (string-width pfx) cuti-wid (string-width sfx)))
            (lambda (sh? widf wrtf) (widf x)))))) ; atom

  ; shortcut output routines used below
  (define (emit s) (display s *port*))
  (define (emit-atom x) (write-atom x *port*))
  ; inserts single space if ind is #f; otherwise prints newline and indents to ind
  (define (space ind v)
    (cond [(not ind) (write-char #\space *port*)]
          [else (newline *port*)
                (do ([i 0 (+ i 1)]) [(>= i ind)] 
                  (write-char #\space *port*))]))
  (define (emit-lpar) (write-char #\( *port*))
  (define (emit-rpar) (write-char #\) *port*))
  (define (emit-lbra) (write-char (if *brackets* #\[ #\() *port*))
  (define (emit-rbra) (write-char (if *brackets* #\] #\)) *port*))
  (define (emit-cuti) (display *length-stub* *port*))
  (define print-cutd 
    (if (string? *level-stub*) ; CL-like model
        (lambda (x bk?) ; bk? is brackets flag, ignored
          (dispatch-on-type x
            (lambda (pfx reff tomf) (emit *level-stub*)) ; #
            (lambda (pfx tolf toxf sfx) (emit *level-stub*)) ; #
            (lambda (pfx lenf reff sfx) (emit *level-stub*)) ; #
            (lambda (sh? widf wrtf) (wrtf x *port*)))) ; atoms
        ; Chez-like model: print "shell" of the object
        (lambda (x bk?) ; bk? is brackets flag
          (cond [(and bk? (pair? x))
                 (emit-lbra) (emit-cuti) (emit-rbra)]
                [else (dispatch-on-type x
                        (lambda (pfx reff tomf) ; '...
                          (emit pfx) (emit-cuti))
                        (lambda (pfx tolf toxf sfx) ; (...)
                          (emit pfx) (emit-cuti) (emit sfx))
                        (lambda (pfx lenf reff sfx) ; #u8(...)
                          (emit pfx) (emit-cuti) (emit sfx))
                        (lambda (sh? widf wrtf) ; atoms
                          (wrtf x *port*)))]))))

  ; locating hooks and calling handlers
  ; 'normalizes' input objects to simplify and sync all phases
  (define (dispatch-on-type x retm retl retv reta)
    (let loop ([al (print-hooks)])
      (cond [(null? al) ; dispatch on builtins
             ; the reader's abbreviations are for code: as data, (quote x) is
             ; printed as the list it is, which is what write does too
             (cond [(and *code* (pair? x) (symbol? (car x)) (pair? (cdr x)) (null? (cddr x))
                         (assq (car x) builtin-read-macros)) =>
                    (lambda (l) (retm (cadr l) cadr (lambda (x) (list (car l) x))))]
                   [(pair? x) 
                    (retl "(" (lambda (x) x) (lambda (x) x) ")")]
                   [(vector? x) ; NB: contents markable!
                    (retl "#(" vector->list list->vector ")")]
                   [(string? x) ; shareable atomic
                    (reta #t quoted-string-width write)]
                   [(bytevector? x) ; NB: contents non-markable!
                    (retv "#u8(" bytevector-length bytevector-u8-ref ")")]
                   [else (reta #f atom-width write-atom)])]
            [((caar al) x) => ; match!
             (lambda (res) ; test can return hook
               (let ([hk (or (cdar al) res)])
                 (unless (pair? hk) (error "invalid hook!"))
                 (case (car hk)
                   ; read macro: rmac-print-hook pfx reff tomf
                   [(rm) (apply retm (cdr hk))]
                   ; list-like: glist-print-hook pfx tolf toxf sfx
                   [(gl) (apply retl (cdr hk))]
                   ; binary vector: bvec-print-hook pfx lenf reff sfx
                   [(bv) (apply retv (cdr hk))]
                   ; atomic: atom-print-hook sh? widf wrtf
                   [(at) (apply reta (cdr hk))]
                   ; TODO: pre-check, this shouldn't happen! 
                   [else (error "invalid hook!")])))]
            [else (loop (cdr al))])))

  ; graph sharing/cycles detection
  ; NB: if print-graph is off, print-circle determines if it is called with cycles-only? #t
  ; or not at all. If print-graph is on, mark-shared is called with cycles-only? #f
  (define unique (list 'shared-mark))
  (define (shared-mark? x) 
     (and (vector? x) (= (vector-length x) 4) (eq? (vector-ref x 0) unique)))
  (define (shared-mark first? count x)
    (vector unique first? count x))
  (define (shared-unmark m)
    (values (vector-ref m 1) (vector-ref m 2) (vector-ref m 3)))
  (define (not-shareable? x) ; definitely not shareable
    (or (symbol? x) (number? x) (boolean? x) (char? x)))
  (define (mark-shared sexp env cycles-only?)
    (let ([counts (make-eq-table)] [marks? #f])
      ; calculate ref counts in O(N)
      (let scan ([x sexp] [v env])
        (unless (not-shareable? x)
          (dispatch-on-type x ;=>
            (lambda (pfx reff tomf) ; read-macro
              (unless (cutd? v)
                (let ([c (table-ref counts x 0)])
                  (table-set! counts x (+ c 1))
                  ; Caveat: surprisingly, no v nesting is done here!
                  ; That's how CL and Chez treat their abbreviations
                  (if (= c 0) (scan (reff x) v) (set! marks? #t)))))
            (lambda (pfx tolf toxf sfx) ; list-like
              (unless (cutd? v)
                (let ([c (table-ref counts x 0)])
                  (table-set! counts x (+ c 1))
                  (if (= c 0) 
                      (let ([l (tolf x)]) ; beware: l could be eq x
                        (when (pair? l) 
                          (scan (car l) (nest v))
                          (let ([l (cdr l)] [v (step v)])
                            (unless (and (cuti? v) (pair? l))
                              (scan l v)))))
                      (set! marks? #t)))))
            (lambda (pfx lenf reff sfx) ; bytevector-like
              (let ([c (table-ref counts x 0)])
                (table-set! counts x (+ c 1))
                (if (> c 0) (set! marks? #t))))
            (lambda (sh? widf wrtf) ; atom
              (when sh? ; printable, store-allocated
                (let ([c (table-ref counts x 0)])
                  (table-set! counts x (+ c 1))
                  (if (> c 0) (set! marks? #t))))))))
      ; optionally detect cycles in O(N)
      (when (and marks? cycles-only?)
        (set! marks? #f) 
        (let find-cycles ([x sexp] [v env] [up '()])
          (unless (not-shareable? x)
            (let ([c (table-ref counts x 0)])
              (cond [(eq? c 'cycle)]
                    [(memq x up) 
                     (table-set! counts x 'cycle) (set! marks? #t)]
                    [(eq? c 'visited)]
                    [else (let ([up (if (> c 1) (cons x up) up)])
                            (table-set! counts x 'visited)
                            (dispatch-on-type x ;=>
                              (lambda (pfx reff tomf) ; read-macro
                                (unless (cutd? v) ; Caveat: no nesting!
                                  (find-cycles (reff x) v up)))
                              (lambda (pfx tolf toxf sfx) ; list-like
                                (unless (cutd? v)
                                  (let ([l (tolf x)]) ; beware: l could be eq x
                                    (when (pair? l) 
                                      (find-cycles (car l) (nest v) up)
                                      (let ([l (cdr l)] [v (step v)])
                                        (unless (and (cuti? v) (pair? l))
                                          (find-cycles l v up)))))))
                              ; bytevector-like and atoms can't contain cycles!
                              (lambda (pfx lenf reff sfx) 42)
                              (lambda (sh? widf wrtf) 42)))])))))
      ; rebuild x with sharing marks as needed in O(N)
      (if marks?
          (let ([ids (make-eq-table)] [next-id 0])
            (define (rebuild x v)
              (if (not-shareable? x) x
                  (let ([c (table-ref counts x 0)])
                    (if (if cycles-only? (eq? c 'cycle) (> c 1))
                        (cond [(table-ref ids x #f) => 
                               (lambda (id) (shared-mark #f id x))]
                              [else
                               (let ([id next-id])
                                 (set! next-id (+ next-id 1))
                                 (table-set! ids x id)
                                 (shared-mark #t id (recur x v)))])
                        (recur x v)))))
            (define (recur x v)
              (dispatch-on-type x ;=>
                (lambda (pfx reff tomf) ; read-macro
                  (if (cutd? v) x ; Caveat: no nesting below! 
                      (let* ([e (reff x)] [ne (rebuild e v)])
                        (if (eq? e ne) x (tomf ne)))))
                (lambda (pfx tolf toxf sfx) ; list-like
                  (if (cutd? v) x
                      (let ([l (tolf x)]) ; beware: l could be eq x
                        (if (pair? l)
                            (let* ([h (rebuild (car l) (nest v))] 
                                   [t (let ([l (cdr l)] [v (step v)])
                                        (if (and (cuti? v) (pair? l))
                                             l (rebuild l v)))])
                              (if (and (eq? h (car l)) (eq? t (cdr l))) x (toxf (cons h t))))))))
                ; bytevector-like and atoms can't contain cycles!
                (lambda (pfx lenf reff sfx) x)
                (lambda (sh? widf wrtf) x)))
            (rebuild sexp env))
          sexp)))

  ; guess print length of atoms -- better be fast than exact
  ; call directly only in cases that don't need override
  ; TODO: we can cache some non-trivial cases
  (define log10-of-2 2.302585092994046)
  (define (atom-width x)
    (cond [(or (null? x) (boolean? x)) 2]
          [(symbol? x) (string-width (symbol->string x))] ; inexact
          [(string? x) (quoted-string-width x)] ; may be inexact
          [(and (char? x) (char<=? #\! x #\~)) 3]
          [(and (= *radix* 10) (integer? x) (exact? x)) ; don't print bignums
           (cond [(<= 0 x 9) 1] [(<= -9 x 99) 2] [(<= -99 x 999) 3]
                 [(> x 0) (exact (ceiling (/ (log (+ x 0.1)) log10-of-2)))]
                 [else (+ 1 (exact (ceiling (/ (log (- 0.1 x)) log10-of-2))))])]
          [(and (= *radix* 10) (rational? x) (exact? x))
           (+ (atom-width (numerator x)) 1 (atom-width (denominator x)))]
          [(memv x '((#\tab . 5) (#\newline . 9) (#\space . 7))) => cdr]
          [(and (number? x) (exact? x) (not (= *radix* 10)))
           (+ 2 (string-length (number->string x *radix*)))]
          [else (written-width x)]))

  (define (write-atom x port)
    (if (and (number? x) (exact? x) (not (= *radix* 10)))
        (let ([s (number->string x *radix*)])
          (write-char #\# port)
          (write-char (case *radix* [(2) #\b] [(8) #\o] [(16) #\x]) port)
          (display s port))
        (write x port)))

  ; we calculate width on S-exps directly; this is far from exact,
  ; but should do for our purposes. Stops early if cap is reached,
  ; returning a value larger than cap. Doing it this way helps to
  ; keep fit-ind width calculations O(1) by input size
  ; make counter from an indent, considering width and inline limit
  (define cmake 
    (let ([il (if *inline-width* *inline-width* *width*)])
      (lambda (ind) (list (if ind (min (- *width* ind) il) +inf.0))))) 
  ; subtract n from c in-place, returning false if we felll below 0
  (define (csub c n) (>= (begin (set-car! c (- (car c) n)) (car c)) 0))
  (define (cdup c) (list (car c)))
  (define (fits-mark? x c v)
    (let-values ([(first? id val) (shared-unmark x)])
      (and (csub c (+ (atom-width id) 2))
           (if first? (fits? val c v) (csub c 0)))))
  (define (fits-read-macro? x c v pfx elt)
    (and (csub c (string-width pfx))
         (fits-tail? elt c v))) ; caveat: no nesting here!
  (define (fits-tail? lst c v)
    (let loop ([sep 0] [l lst] [v v])
      (cond [(null? l) #t]
            [(not (pair? l)) (and (csub c 3) (fits? l c (back v)))]
            [(cuti? v) (csub c (+ sep cuti-wid))]
            [else (and (csub c sep) (fits? (car l) c v)
                       (loop 1 (cdr l) (step v)))])))
  (define (fits-list-like? x c v pfx lst sfx)
    (and (csub c (+ (string-width pfx) (string-width sfx)))
         (fits-tail? lst c (nest v))))
  (define (fits-vector-like? x c v pfx lenf reff sfx)
    (and (csub c (+ (string-width pfx) (string-width sfx)))
         (let loop ([sep 0] [i 0] [v (nest v)] [n (lenf x)])
           (cond [(= i n) #t]
                 [(cuti? v) (csub c (+ sep cuti-wid))]
                 [else (and (csub c sep) (fits? (reff x i) c v)
                            (loop 1 (+ i 1) (step v) n))]))))
  (define (fits? x c v) ;=> #t if x fits into space provided by c
    (cond [(shared-mark? x) (fits-mark? x c v)]
          [(cutd? v) (csub c (cutd-widf x))]
          [else (dispatch-on-type x ;=>
                  (lambda (pfx reff tomf) ; read-macro
                    (fits-read-macro? x c v pfx (reff x)))
                  (lambda (pfx tolf toxf sfx) ; list-like
                    (fits-list-like? x c v pfx (tolf x) sfx))
                  (lambda (pfx lenf reff sfx) ; bytevector-like
                    (fits-vector-like? x c v pfx lenf reff sfx)) 
                  (lambda (sh? widf wrtf) ; atom
                    (csub c (widf x))))]))
  (define (fits-flat-or-stacked? x c v sr) ;=> #f | #t | sr
    (define c0 (cdup c))
    (define (fits*/stack? l v)
      (cond [(null? l) (set-car! c 0) sr]
            [(fits? (car l) (cdup c0) v)
             (fits*/stack? (cdr l) (step v))] 
            [else #f]))
    (or (fits? x c v) ; fits flat? return #t
        (and (not (cutd? v)) (list? x) 
             (csub c0 1) (fits*/stack? x (nest v)))))
  (define (fit-ind x ind v)
    (if (or (not ind) (fits? x (cmake ind) v)) #f ind))
  (define (fit-ind* x* ind v)
    (if (or (not ind) (fits-tail? x* (cmake ind) v)) #f ind))

  (define (print-mark x ind v print)
    (let-values ([(first? id x) (shared-unmark x)])
      (emit "#") (emit id) 
      (emit (if first? "=" "#"))
      (when first?
        (let ([ilen (atom-width id)])
          (print x (ind+ ind (+ ilen 2)) v)))))
  
  (define (print-read-macro x ind v pfx elt)
    (emit pfx)
    (let ([ind (ind+ ind (string-width pfx))])
      (print-datum elt ind v))) ; Caveat: no nesting!

  ; fill-style printing of (possibly improper) list contents
  ; if lst is atom, prints dot before it; v is env for lst head
  ; NB: adjacent closers may cause last line overflow!
  (define (print*/fill lst ind v restoff prin1 print)
    (define (fitsi? e c v) (if (cuti? v) (csub c cuti-wid) (fits? e c v)))
    (define (cari l v) (if (cuti? v) 42 (car l)))
    (define (prest x ind c v)
      (if (or (not ind) (and (csub c 3) (fitsi? x c v)))
          (begin (emit " . ") (print x #f v))
          (begin (space ind v) (emit ".")
                 (let ([ind1 (fit-ind x (ind+ ind 2) v)])
                   (space (and ind1 ind) v) (print x (and ind1 ind) v)))))
    (define (ploop l e v ind ind1 indi c prt)
      ; invariant: e is (car l) or ..., v is e's env
      (if (cuti? v) (emit-cuti) (prt e indi v))
      (cond [(cuti? v)] [(null? (cdr l))]
            [(not (pair? (cdr l))) (prest (cdr l) ind c (back v))]
            [else (let* ([l (cdr l)] [v (step v)] [e (cari l v)])
                    (cond [(and (csub c 1) (fitsi? e c v)) 
                           (space #f v) (ploop l e v ind ind1 #f c print)]
                          [else (let ([c (cmake ind1)]) ; bumped below
                                  (space ind1 v) (fitsi? e c v) 
                                  (ploop l e v ind ind1 ind1 c print))]))]))
    (cond [(null? lst)]
          [(not (pair? lst)) (prest lst ind (cmake ind) (back v))]
          [else (let ([e (cari lst v)] [c (cmake ind)])
                  (fitsi? e c v) ; need to bump c!
                  (ploop lst e v ind (ind+ ind restoff) ind c prin1))]))

  ; regular one-per-line-on-overflow printing of list contents
  ; if lst is atom, prints dot before it; v is env for lst head
  ; prin1 is used to print first subexpression, print for the rest
  (define (print*/body lst ind v prin1 print)
    (let loop ([first? #t] [lst lst] [v v] [pr prin1])
      (cond [(null? lst)]
            [(not (pair? lst)) ; TODO: better miser wrap?
             (emit " . ") (print lst (ind+ ind 3) (back v))]
            [(cuti? v) (unless first? (space ind v)) (emit-cuti)]
            [else (unless first? (space ind v))
                  (print (car lst) ind v)
                  (loop #f (cdr lst) (step v) print)])))

  (define (print-list-like x ind v pfx lst sfx)
    (let ([ind (fit-ind x ind v)])
      (emit pfx)
      (let ([ind (ind+ ind (string-width pfx))] [v (nest v)])
        (if (vector? x) ; hack: fill-print vectors 
            (print*/fill lst ind v 0 print-datum print-datum)
            (print*/body lst ind v print-datum print-datum)))
      (emit sfx)))
  
  ; fill-style binary vector printing: saves verical space
  ; NB: adjacent closers may cause last-line overflow!
  (define (print-vector-like x ind v pfx lenf reff sfx)
    (define (fitsi? e c v) (if (cuti? v) (csub c cuti-wid) (fits? e c v)))
    (define (refi x i v) (if (cuti? v) 42 (reff x i)))
    (emit pfx)
    (let ([n (lenf x)] [ind (ind+ ind (string-width pfx))] [v (nest v)])
      (unless (= 0 n) ; nonempty, but may be cut by length
        (let ([e (refi x 0 v)] [c (cmake ind)])
          (fitsi? e c v) ; we'll print anyway, just bump c!
          ; loop invariant: i is e's index, v is e's env
          (let loop ([i 0] [e e] [v v] [indi ind] [c c])
            (if (cuti? v) (emit-cuti) (print-datum e indi v))
            (unless (or (= i (- n 1)) (cuti? v)) ; got next elt
              (let* ([i (+ i 1)] [v (step v)] [e (refi x i v)])
                (cond [(and (csub c 1) (fitsi? e c v)) 
                       (space #f v) (loop i e v #f c)]
                      [else (let ([c (cmake ind)]) ; bumped below
                              (space ind v) (fitsi? e c v) 
                              (loop i e v ind c))])))))))
    (emit sfx))

  (define (print-datum x ind v)
    (cond [(shared-mark? x) (print-mark x ind v print-datum)]
          [(cutd? v) (print-cutd x #f)] ; no brackets w/o fmt!
          [else (dispatch-on-type x ;=>
                  (lambda (pfx reff tomf) ; read-macro
                    (print-read-macro x ind v pfx (reff x)))
                  (lambda (pfx tolf toxf sfx) ; list-like
                    (print-list-like x ind v pfx (tolf x) sfx))
                  (lambda (pfx lenf reff sfx) ; bytevector-like
                    (print-vector-like x ind v pfx lenf reff sfx))
                  (lambda (sh? widf wrtf) ; atom
                    (wrtf x *port*)))]))

  ; default list exp printer; precondition: x is a pair
  (define (print-app x ind v) 
    (let ([ind (fit-ind x ind v)])
      (emit-lpar)
      (let ([ind (ind+ ind 1)] [v (nest v)])
        (if (and (symbol? (car x)) (pair? (cdr x))  (not (cuti? v)))
            (let ([oplen (atom-width (car x))])
              (if (< oplen (alt-indent ind)) ; ind = len + 1 space
                  (begin (emit-atom (car x)) (emit " ")
                         (print*/fill (cdr x) (ind+ ind (+ 1 oplen))
                           (step v) 0 print-exp print-exp))
                  (print*/fill x ind v (std-indent ind) 
                    print-exp print-exp)))
            (print*/fill x ind v (std-indent ind) 
              print-exp print-exp)))
      (emit-rpar)))

  ; clause printer, prin1 is used for the head
  (define (print-clause x ind v prin1)
    (let ([ind (fit-ind x ind v)])
      (cond [(shared-mark? x) (print-mark x ind v print-exp)]
            [(cutd? v) (print-cutd x #t)] ; brackets!
            [(and (list? x) (= (length x) 3) (eq? (cadr x) '=>)) ; (* => *)
             (emit-lbra)
             (print*/fill x (ind+ ind 1) (nest v) 0 prin1 print-exp)
             (emit-rbra)] 
            [(pair? x)
             (emit-lbra)
             (print*/body x (ind+ ind 1) (nest v) prin1 print-exp)
             (emit-rbra)]
            [else (print-datum x ind v)])))

  (define (print-datum-clause x ind v) (print-clause x ind v print-datum))
  (define (print-exp-clause x ind v) (print-clause x ind v print-exp))

  ; clause block printer, prin1 is used for each clause's head
  (define (print-clauses x ind v prin1)
    (define (print x ind v) (print-clause x ind v prin1))
    (let ([ind (fit-ind x ind v)])
      (cond [(shared-mark? x) (print-mark x ind v print-exp)]
            [(cutd? v) (print-cutd x #f)] ; no brackets!
            [(pair? x)
             (emit-lpar)
             (print*/body x (ind+ ind 1) (nest v) print print)
             (emit-rpar)]
            [else (print-datum x ind v)])))

  (define (print-exp x ind v)
    (cond [(shared-mark? x) (print-mark x ind v print-exp)]
          [(cutd? v) (print-cutd x #f)] ; no brackets w/o fmt!
          [else (dispatch-on-type x ;=>
                  (lambda (pfx reff tomf) ; read-macro
                    (print-read-macro x ind v pfx (reff x)))
                  (lambda (pfx tolf toxf sfx) ; list-like
                    (let ([l (tolf x)])
                      (if (and (eq? l x) (pair? l)) ; regular pair
                          (print-list-exp l ind v)
                          (print-list-like x ind v pfx l sfx))))
                  (lambda (pfx lenf reff sfx) ; bytevector-like
                    (print-vector-like x ind v pfx lenf reff sfx))
                  (lambda (sh? widf wrtf) ; atom
                    (wrtf x *port*)))]))

  (define (print-list-exp x ind v) ; assumes x is a pair
    (cond [(not (symbol? (car x))) (print-app x ind v)]
          [(pretty-style (car x)) => ; NB: car of fmt is _
           (lambda (fmt)
             (if (eq? (cdr fmt) 'fill)
                 (print-app x ind v) ; special case
                 (print/fmt (cons 'i (cdr fmt)) x ind v)))]
          [else (print-app x ind v)]))
 
  (define (print/fmt fmt x ind v) ; x is a pair, no cutd/mark
    (let ([ind (fit-ind x ind v)])
      (cond [(eq? fmt 'e) (print-exp x ind v)]
            [(memq fmt '(d i)) (print-datum x ind v)]
            [(eq? fmt 'dc) (print-clause x ind v print-datum)]
            [(eq? fmt 'ec) (print-clause x ind v print-exp)]
            [(eq? fmt 'ec*) (print-clauses x ind v print-exp)]
            [(eq? fmt 'dc*) (print-clauses x ind v print-datum)]
            [else (emit-lpar) (print/fmt* x (ind+ ind 1) (nest v) fmt) (emit-rpar)])))

  ; print via atomic tail format
  ; if lst is atom, prints dot before it; v is env for lst head
  (define (print/fmt-tail lst ind v fmt)
    (define print 
      (case fmt 
        [(dc*) print-datum-clause]
        [(ec*) print-exp-clause]
        [else print-exp]))
    (print*/body lst ind v print print))

  ; format-controlled printing of (possibly improper) list contents
  ; if lst is atom, prints dot before it; v is env for lst head
  ; NB: adjacent closers may cause last line overflow!
  (define (print/fmt* lst ind v fmt*)
    (define roff (std-indent ind))
    (define (fitsi? e c v) 
      (if (cuti? v) (csub c cuti-wid) (fits? e c v)))
    (define (fitsfsi? e c v sr) ; sr is a result to return if stacked
      (if (cuti? v) (csub c cuti-wid) (fits-flat-or-stacked? e c v sr)))
    (define (cari l v) (if (cuti? v) 42 (car l)))
    (define (fmcdr fmt*) (if (pair? fmt*) (cdr fmt*) fmt*))
    (define (prcar fmt* x ind v)
      (cond [(not (pair? fmt*)) (print-exp x ind v)]
            [(shared-mark? x) (print-mark x ind v print-exp)]
            [(cutd? v) (print-cutd x (memq (car fmt*) '(dc ec)))]
            [else (print/fmt (car fmt*) x ind v)]))
    (define (prest x ind c v)
      (if (or (not ind) (and (csub c 3) (fitsi? x c v)))
          (begin (emit " . ") (print-datum x #f v))
          (begin (space ind v) (emit ".")
                 (let ([ind1 (fit-ind x (ind+ ind 2) v)])
                   (space (and ind1 ind) v) (print-datum x (and ind1 ind) v)))))
    (define (pind l e v ind ind1)
      (define aoff ; potential alt offset
        (and (eq? l lst) (symbol? e) ; alt indent?
             (let ([oplen (atom-width e)])
               (and (< oplen (alt-indent ind)) (+ 1 oplen)))))
      (space (if aoff #f ind1) v) ; just 1 space if alt
      (if aoff (ind+ ind aoff) ind1))
    (define (ploop l e v ind ind1 indi c fmt*)
      (if (and (pair? fmt*) (eq? (car fmt*) 'i?))
          (if (symbol? e) ; optional id is present, use it
              (pnext l e v ind ind1 indi c (cons 'i (cdr fmt*)))
              (ploop l e v ind ind1 indi c (cdr fmt*)))
          (pnext l e v ind ind1 indi c fmt*)))
    (define (pnext l e v ind ind1 indi c fmt*)
      ; invariant: e is (car l) or ..., v is e's env
      (if (cuti? v) (emit-cuti) (prcar fmt* e indi v))
      (cond [(cuti? v)] [(null? (cdr l))]
            [(not (pair? (cdr l))) (prest (cdr l) ind c (back v))]
            [(memq (fmcdr fmt*) '(dc* ec* body)) ; tail part ahead
             (let ([ind2 (pind l e v ind ind1)]) ; pind calls space!
               (print/fmt-tail (cdr l) ind2 (step v) (fmcdr fmt*)))]
            [else (let* ([ll l] [le e] [l (cdr l)] [v (step v)] [e (cari l v)])
                    (cond [(and ind (eq? ll lst) (symbol? le) (equal? '(i dc* . body) fmt*)
                             (let ([oplen (atom-width le)])
                               (and (< oplen (alt-indent ind))
                                 (csub c 1) (fitsfsi? e c v (+ oplen 1))))) =>
                           (lambda (r) ; r here is #t or head offset
                             (define indr (if (integer? r) (ind+ ind r) #f))
                             (space #f v) (ploop l e v ind ind1 indr c (fmcdr fmt*)))]
                          [(and (csub c 1) (fitsi? e c v)) 
                           (space #f v) (ploop l e v ind ind1 #f c (fmcdr fmt*))]
                          [else (let ([c (cmake ind1)]) ; bumped below
                                  (space ind1 v) (fitsi? e c v) 
                                  (ploop l e v ind ind1 ind1 c (fmcdr fmt*)))]))]))
    (cond [(null? lst)]
          [(not (pair? lst)) (prest lst ind (cmake ind) v)]
          [else (let ([e (cari lst v)] [c (cmake ind)])
                  (fitsi? e c v) ; need to bump c!
                  (ploop lst e v ind (ind+ ind roff) ind c fmt*))]))

  (let* ([pg (if *graph* 2 (if *circle* 1 0))] 
         [x (if (> pg 0) (mark-shared sexp env (= pg 1)) sexp)])
    (cond [*code* (print-exp x *indent* env)]
          [else   (print-datum x *indent* env)])
    ; do not add newline if we are printing inline
    (when *indent* (newline *port*))))

; accepts a keyword-value list as last argument
(define (pp* obj arg . args)
  (define (cons* arg . args)
    (let loop ([xs (cons arg args)])
      (if (null? (cdr xs))
          (car xs) (cons (car xs) (loop (cdr xs))))))
  (apply pp obj (apply cons* arg args)))

; code-oriented multiline printing via preset parameters
(define (pretty-print obj . rest)
  (define-values (port kv*) 
    (if (and (pair? rest) (output-port? (car rest))) 
        (values (car rest) (cdr rest))
        (values (current-output-port) rest)))
  (pp* obj port (append kv* (list print-indent 0 print-code #t print-brackets #t))))

; prints like write by default; no preset parameters
(define (print obj . rest)
  (define-values (port kv*) 
    (if (and (pair? rest) (output-port? (car rest))) 
        (values (car rest) (cdr rest))
        (values (current-output-port) rest)))
  (pp* obj port kv*))


; initialize format pattern registry

(for-each (lambda (x) (pretty-style (car x) (cdr x))) 
'((lambda             . (_ d . body))
  (define             . (_ d . body))
  (define-values      . (_ d . body))
  (define-syntax      . (_ d . body))
  (define-library     . (_ d . body))
  (define-record-type . (_ d . body))
  (let                . (_ i? dc* . body))
  (letrec             . (_ dc* . body))
  (letrec*            . (_ dc* . body))
  (let*               . (_ dc* . body))
  (let-values         . (_ dc* . body))
  (let*-values        . (_ dc* . body))
  (let-syntax         . (_ dc* . body))
  (letrec-syntax      . (_ dc* . body))
  (parameterize       . (_ dc* . body))
  (with-syntax        . (_ dc* . body))
  (syntax-rules       . (_ i? d . ec*))
  (do                 . (_ dc* ec . body))
  (begin              . (_ . body))
  (if                 . (_ . body))
  (cond               . (_ . ec*))
  (case               . (_ e . dc*))
  (when               . (_ e . body))
  (unless             . (_ e . body))
  (set!               . (_ e . fill))
  (and                . (_ . fill))
  (or                 . (_ . fill))
  (import             . (_ . fill))
  (delay              . (_ e))
  (guard              . (_ (e . ec*) . body))
  (case-lambda        . (_ . dc*))
  (cond-expand        . (_ . dc*))
))


; initialize pp hook registry

(print-hooks 
  (add-print-hook (print-hooks)
    box? ; boxes, printed as #&x
    (glist-print-hook 
      "#&" (lambda (x) (list (unbox x)))  
          (lambda (x) (box (car x))) "")))
(print-hooks
  (add-print-hook (print-hooks)
    (lambda (x) ; supported homogenous num vectors
      (case (numvector? x)
        [(#f 0) #f]
        [(1) (bvec-print-hook "#s8(" numvector-length numvector-ref ")")]
        [(2) (bvec-print-hook "#u16(" numvector-length numvector-ref ")")] 
        [(3) (bvec-print-hook "#s16(" numvector-length numvector-ref ")")]
        [(10) (bvec-print-hook "#f32(" numvector-length numvector-ref ")")]
        [(11) (bvec-print-hook "#f64(" numvector-length numvector-ref ")")]
        ; TODO: add 2 to numvector-length for #*0101... bitvec notation
        [else (atom-print-hook #t written-width write)]))))

))
