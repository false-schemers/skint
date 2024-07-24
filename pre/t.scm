
;--------------------------------------------------------------------------------------------------
; Transformer and Compiler
;--------------------------------------------------------------------------------------------------

(load "s.scm")


;--------------------------------------------------------------------------------------------------
; Utils
;--------------------------------------------------------------------------------------------------

(define (set-member? x s)
  (cond [(null? s) #f]
        [(eq? x (car s)) #t]
        [else (set-member? x (cdr s))]))

(define (set-cons x s) 
  (if (set-member? x s) 
      s 
      (cons x s)))

(define (set-union s1 s2)
  (if (null? s1) 
      s2 
      (set-union (cdr s1) (set-cons (car s1) s2))))

(define (set-minus s1 s2)
  (if (null? s1)
      '()
      (if (set-member? (car s1) s2)
          (set-minus (cdr s1) s2)
          (cons (car s1) (set-minus (cdr s1) s2)))))

(define (set-intersect s1 s2)
  (if (null? s1)
      '()
      (if (set-member? (car s1) s2)
          (cons (car s1) (set-intersect (cdr s1) s2))
          (set-intersect (cdr s1) s2))))

(define-syntax record-case
  (syntax-rules (else)
    [(record-case (pa . ir) clause ...)
     (let ([id (pa . ir)]) 
       (record-case id clause ...))]
    [(record-case id)
     'record-case-miss]
    [(record-case id [else exp ...])
     (begin exp ...)]
    [(record-case id [(key ...) ids exp ...] clause ...)
     (if (memq (car id) '(key ...))
         (apply (lambda ids exp ...) (cdr id))
         (record-case id clause ...))] 
    [(record-case id [key ids exp ...] clause ...)
     (if (eq? (car id) 'key)
         (apply (lambda ids exp ...) (cdr id))
         (record-case id clause ...))]))

(define (sexp-match? pat x)
  (or (eq? pat '*)
      (and (eq? pat '<id>) (or (symbol? x) (procedure? x)))
      (and (eq? pat '<symbol>) (symbol? x))
      (and (eq? pat '<number>) (number? x))
      (and (eq? pat '<string>) (string? x))
      (and (eq? pat '<vector>) (vector? x))
      (and (eq? pat '<box>) (box? x))
      (eqv? x pat)
      (and (pair? pat)
           (cond [(and (eq? (car pat) '...)
                       (pair? (cdr pat))
                       (null? (cddr pat)))
                  (eqv? x (cadr pat))]
                 [(and (pair? (cdr pat))
                       (eq? (cadr pat) '...)
                       (null? (cddr pat)))
                  (let ([pat (car pat)])
                    (if (eq? pat '*)
                        (list? x)
                        (let loop ([lst x])
                          (or (null? lst)
                              (and (pair? lst)
                                   (sexp-match? pat (car lst))
                                   (loop (cdr lst)))))))]
                 [else
                  (and (pair? x)
                       (sexp-match? (car pat) (car x))
                       (sexp-match? (cdr pat) (cdr x)))]))
      (and (vector? pat) (vector? x) 
           (sexp-match? (vector->list pat) (vector->list x)))
      (and (box? pat) (box? x) 
           (sexp-match? (unbox pat) (unbox x)))))

(define-syntax sexp-case
  (syntax-rules (else)
    [(_ (key ...) clauses ...)
     (let ([atom-key (key ...)])
       (sexp-case atom-key clauses ...))]
    [(_ key (else result1 result2 ...))
     (begin result1 result2 ...)]
    [(_ key (pat result1 result2 ...))
     (if (sexp-match? 'pat key)
         (begin result1 result2 ...))]
    [(_ key (pat result1 result2 ...) clause clauses ...)
     (if (sexp-match? 'pat key)
         (begin result1 result2 ...)
         (sexp-case key clause clauses ...))]))

(define (symbol-append . syms)
 (string->symbol (apply string-append (map symbol->string syms))))

; unique symbol generator (poor man's version)
(define gensym
  (let ([gsc 0]) ; never goes down! FIXME: extend fixnum range
    (lambda args ; (), (symbol), or (#f) for gsc reset
      (set! gsc (fx+ gsc 1))
      (if (or (null? args) (not (symbol? (car args))))
          (string->symbol 
            (string-append "#" (fixnum->string gsc 10)))
          (string->symbol 
            (string-append (symbol->string (car args))
               (string-append "#" (fixnum->string gsc 10))))))))

(define (remove! x l pred?) ; applies (pred? x (car l))
  (let loop ([f #f] [l #f] [r l])
    (cond [(not (pair? r)) (if l (begin (set-cdr! l r) f) r)]
          [(pred? x (car r)) (loop f l (cdr r))]
          [l (set-cdr! l r) (loop f r (cdr r))]
          [else (loop r r (cdr r))])))

(define (pair* x . more)
  (let loop ([x x] [r more])
    (if (null? r) x (cons x (loop (car r) (cdr r))))))

(define (append* lst)
  (cond [(null? lst) '()]
        [(null? (cdr lst)) (car lst)]
        [else (append (car lst) (append* (cdr lst)))]))

(define (string-append* l) 
  (apply string-append l))

(define (list1? x) (and (pair? x) (null? (cdr x))))
(define (list1+? x) (and (pair? x) (list? (cdr x))))
(define (list2? x) (and (pair? x) (list1? (cdr x))))
(define (list2+? x) (and (pair? x) (list1+? (cdr x))))
(define (list3? x) (and (pair? x) (list2? (cdr x))))

(define (read-code-sexp port)
  ; for now, we will just use read with no support for circular structures
  (read-simple port))

(define (error* msg args)
  (raise (error-object #f msg args)))

(define (warning* msg args)
  (print-error-message (string-append "Warning: " msg) args (current-error-port)))


;--------------------------------------------------------------------------------------------------
; Syntax of the 'Scheme Core' language
;--------------------------------------------------------------------------------------------------

;  <core> -> (quote <object>)
;  <core> -> (const <id>) ; immutable variant of ref 
;  <core> -> (ref <id>)
;  <core> -> (set! <id> <core>)
;  <core> -> (set& <id>)
;  <core> -> (gref <gid>) where <gid> is a symbolic index in a global store
;  <core> -> (gset! <gid> <core>)
;  <core> -> (lambda <ids> <core>) where <ids> -> (<id> ...) | (<id> ... . <id>) | <id>
;  <core> -> (lambda* (<arity> <core>) ...) where <arity> -> (<cnt> <rest?>) 
;  <core> -> (letcc <id> <core>) 
;  <core> -> (withcc <core> <core>) 
;  <core> -> (begin <core> ...)
;  <core> -> (if <core> <core> <core>)
;  <core> -> (call <core> <core> ...) 
;  <core> -> (integrable <ig> <core> ...) where <ig> is an index in the integrables table
;  <core> -> (asm <igs>) where <igs> is ig string leaving result in ac, e.g. "'2,'1+" 
;  <core> -> (once <gid> <core>) 

;  NB: (begin) is legit, returns unspecified value
;  on top level, these four extra core forms are legal:

;  <core> -> (define <id> <core>)
;  <core> -> (define-syntax <id> <transformer>)
;  <core> -> (define-library <listname> <library>) where <library> is a vector (see below)
;  <core> -> (import <library>)

;  These names are bound to specials never returned by xform:

;  (syntax-quote <value>)
;  (body <expr or def> ...)
;  (syntax-lambda (<id> ...) <expr>)
;  (syntax-rules (<id> ...) <rule> ...)
;  (syntax-length <form>)
;  (syntax-error <msg> <arg> ...)


(define idslist?
  (lambda (x)
    (cond [(null? x) #t]
          [(pair? x) (and (id? (car x)) (idslist? (cdr x)))]
          [else (id? x)])))

(define normalize-arity
  (lambda (arity)
    (if (and (list2? arity) (fixnum? (car arity)) (boolean? (cadr arity)))
        arity
        (let loop ([cnt 0] [l arity])
          (cond [(pair? l) (loop (fx+ 1 cnt) (cdr l))]
                [(null? l) (list cnt #f)]
                [else (list cnt #t)])))))

; convention for 'flattened' <ids> is to put rest arg if any at the front
(define flatten-idslist
  (lambda (ilist)
    (if (list? ilist) ilist
        (let loop ([l ilist] [r '()])
          (cond [(pair? l) (loop (cdr l) (cons (car l) r))]
                [else (if (null? l) (reverse! r) (cons l (reverse! r)))])))))

(define idslist-req-count
  (lambda (ilist)
    (if (pair? ilist)
        (fx+ 1 (idslist-req-count (cdr ilist)))
        0)))

;--------------------------------------------------------------------------------------------------
; Macro transformer (from Scheme to Scheme Core) derived from Al Petrofsky's EIOD 1.17
;--------------------------------------------------------------------------------------------------

; EIOD Copyright notice (amended with the author's permission): 
; Copyright 2002, 2004, 2005 Al Petrofsky <al@petrofsky.org>
; LICENSING (3-clause BSD or GNU GPL 2 and up)
; <the text of the 3-clause BSD license is in the LICENSE file>

; An environment is a procedure that accepts any identifier and access type and returns a 
; denotation. Access type is one of these symbols: ref, set!, define, define-syntax.
; The denotation of an identifier is its macro location, which is a cell storing the 
; identifier's current syntactic value. Location's value can be changed later.

; Special forms are either a symbol naming a builtin, or a transformer procedure 
; that takes two arguments: a macro use and the environment of the macro use.

; <identifier>  ->  <symbol> | <thunk returning (sym . den)>
; <denotation>  ->  <location>
; <location>    ->  #&<value>
; <value>       ->  <special> | <core>
; <special>     ->  <builtin> | <integrable> | <transformer> | <library> | <void>
; <builtin>     ->  syntax-quote | quote | set! | set& | if | lambda | lambda* |
;                   letcc | withcc | body | begin | define | define-syntax |
;                   syntax-lambda | syntax-rules | syntax-length | syntax-error |
;                   define-library | import
; <integrable>  ->  <fixnum serving as index in internal integrables table>
; <transformer> ->  <procedure of exp and env returning exp>
; <library>     ->  <vector of init-code and export-alist>
; <void>        ->  <void value used to initialize uninitialized locations>

(define-syntax  location?            box?)
(define-syntax  make-location        box)
(define-syntax  location-val         unbox)
(define-syntax  location-set-val!    set-box!)
(define-syntax  val-void?            void?)
(define-syntax  val-core?            pair?)
(define-syntax  val-transformer?     procedure?)
(define-syntax  val-integrable?      integrable?)
(define-syntax  val-builtin?         symbol?)
(define-syntax  val-library?         vector?)
(define-syntax  make-library         vector)
(define-syntax  library-code         (syntax-rules () [(_ l) (vector-ref l 0)]))
(define-syntax  library-exports      (syntax-rules () [(_ l) (vector-ref l 1)]))
(define-syntax  library-set-exports! (syntax-rules () [(_ l v) (vector-set! l 1 v)]))

(define (location-special? l)        (not (val-core? (location-val l))))
(define (new-id sym peek getlits)    (define p (list sym peek getlits)) (lambda () p))
(define-syntax new-id?               procedure?)
(define (old-sym id)                 (car (id)))
(define (old-peek id)                (cadr (id)))
(define (old-literals id)            ((or (caddr (id)) (lambda () '()))))
(define (id? x)                      (or (symbol? x) (new-id? x)))
(define (id->sym id)                 (if (symbol? id) id (old-sym id)))

; take a possibly renamed target id, and find image for nid in the same context
(define (id-rename-as id nid)
  (let loop ([id id])
    (if (symbol? id) nid
        (let* ([lits (old-literals id)] [oid->id (rassq id lits)])
          (unless oid->id (x-error "id-rename-as failed: not found in its own lits" id))
          (let ([renamed-nid (loop (car oid->id))])
            (cond [(assq renamed-nid lits) => cdr]
                  [else renamed-nid]))))))

; make a fresh (renamed) id for a template literal, delaying allocation of its binding to
; the moment it is used free -- in hope that such moment will never happen
(define (new-literal-id id env getlits)
  (define peek (env id 'peek))
  (unless peek (x-error "env peek failed!" id env (id->sym id) (and (new-id? id) (new-id-lookup id 'peek))))
  (new-id (id->sym id) peek getlits))

; common code for consistency between two procedures below
; precondition: peek part of spg is a name registry; it is replaced by location
; containing (ref gs) value, where gs is a gensym derived from spg name part
(define (gensym-ref-value-helper def? spg)
  ; on top level, make sure the def? calling path should reach this procedure first!
  (name-lookup (cadr spg) (car spg) (lambda (n) (list 'ref (if def? (gensym n) n)))))

; look up denotation of renamed identifier, trying to delay allocation up to the moment
; when actual location is needed -- peeks don't have to go all the way (see )
(define (new-id-lookup id at)
  (let* ([spg (id)] [peek (cadr spg)])
    (if (or (eq? at 'peek) (location? peek))
        peek ; delay binding allocation until absolutely necessary
        (gensym-ref-value-helper #f spg))))

; this operation should be consistent with new-id-lookup, but comes with a twist: if it is
; called with new-id and at=define/define-syntax, (env id at) is guaranteed to fail because
; renamed ids cannot be interned into name registries. 
(define (top-defined-id-lookup env id at) ;=> loc | #f
  (and (memq at '(define define-syntax))
       (if (symbol? id) (xenv-lookup env id at)
           (let* ([spg (id)] [peek (cadr spg)])
             (if (location? peek) 
                 peek ; loc is already there, use it
                 (gensym-ref-value-helper #t spg))))))


; Expand-time environments map names (identifiers or listnames) to denotations, i.e. locations
; containing either a <special> or a <core> value. In normal case, <core> value is (ref <gid>),
; where <gid> is a key in run-time store, aka *globals*. Environments should allocate new locations
; as needed, so every name gets mapped to one. Expand-time environments are represented as
; two-argument procedures, where the second argument (at) is an access type symbol, one of the
; these values: peek, ref, set!, define, define-syntax, import (last 3 are requests to allocate)

(define (extend-xenv-local id val env)
  (let ([loc (make-location val)])
    (if (pair? id) 
        (lambda (i at) ; listname binding 
          (if (equal? id i) 
              (case at [(ref set! peek) loc] [else #f]) 
              (env i at)))
        (lambda (i at) ; symname binding
          (if (eq? id i)
              (case at [(ref set! peek) loc] [else #f]) 
              (env i at))))))

(define (add-local-var id gid env)
  (extend-xenv-local id (list 'ref gid) env))

(define (xenv-lookup env id at)
  (or (env id at)
      (error* "transformer: invalid identifier access" (list id (xform-sexp->datum id) at))))

(define (xenv-ref env id) (xenv-lookup env id 'ref))

(define (xform-sexp->datum sexp)
  (let conv ([sexp sexp])
    (cond [(id? sexp) (id->sym sexp)]
          [(pair? sexp) (cons (conv (car sexp)) (conv (cdr sexp)))]
          [(vector? sexp) (list->vector (map conv (vector->list sexp)))]
          [else sexp])))

(define (x-error msg . args)
  (error* (string-append "transformer: " msg) args))

(define (check-syntax sexp pat msg)
  (unless (sexp-match? pat sexp) (x-error msg sexp)))

; popular context-independent ids retaining their original meaning (not for id=?)
(define lambda-id        (new-id 'lambda        (make-location 'lambda) #f))
(define begin-id         (new-id 'begin         (make-location 'begin) #f))
(define define-id        (new-id 'define        (make-location 'define) #f))
(define define-syntax-id (new-id 'define-syntax (make-location 'define-syntax) #f))
(define syntax-quote-id  (new-id 'syntax-quote  (make-location 'syntax-quote) #f))

; standard way of comparing identifiers used as keywords and such; details below
(define (free-id=? id1 env1 id2 env2)
  (let ([p1 (env1 id1 'peek)] [p2 (env2 id2 'peek)])
    (and p1 p2 ; both envs should be supported by name registries
      (if (and (name-registry? p1) (name-registry? p2))
          (and (eq? p1 p2) (eq? id1 id2)) ; would end w/same loc if alloced
          (eq? p1 p2))))) ; nrs and locs are distinct, so this means "same loc" 

; xform receives Scheme s-expressions and returns either Core Scheme <core> form
; (always a pair) or so-called 'special', which is either a builtin (a symbol), a
; a transformer (a procedure), or an integrable (an integer). Appos? flag is true 
; when the context allows xform to return a special; otherwise, only <core> can 
; be returned. 

(define (xform appos? sexp env)
  (cond [(id? sexp) 
         (let ([hval (xform-ref sexp env)])
           (cond [appos?                  hval] ; anything goes in app position
                 [(val-integrable? hval)  (list 'const (integrable-global hval))]
                 [(val-transformer? hval) (xform appos? (hval sexp env) env)] ; id-syntax
                 [(val-library? hval)     (x-error "improper use of library" hval sexp)]
                 [(val-void? hval)        (x-error "id has no value" hval sexp (id->sym sexp))]
                 [(not (val-core? hval))  (x-error "improper use of syntax form" hval)]
                 [else                    hval]))]
        [(not (pair? sexp))
         (xform-quote (list sexp) env)]
        [else ; note: these transformations are made in 'expression' context
         (let* ([head (car sexp)] [tail (cdr sexp)] [hval (xform #t head env)])
           (case hval
             [(quote)          (xform-quote          tail env)]
             [(set!)           (xform-set!           tail env)]
             [(set&)           (xform-set&           tail env)]
             [(if)             (xform-if             tail env)]
             [(lambda)         (xform-lambda         tail env)]
             [(lambda*)        (xform-lambda*        tail env)]
             [(letcc)          (xform-letcc          tail env)]
             [(withcc)         (xform-withcc         tail env)]
             [(body)           (xform-body           tail env appos?)]
             [(begin)          (xform-begin          tail env appos?)]
             [(define)         (xform-define         tail env)]
             [(define-syntax)  (xform-define-syntax  tail env)]
             [(syntax-quote)   (xform-syntax-quote   tail env)]
             [(syntax-lambda)  (xform-syntax-lambda  tail env appos?)]
             [(syntax-rules)   (xform-syntax-rules   tail env)]
             [(syntax-length)  (xform-syntax-length  tail env)]
             [(syntax-error)   (xform-syntax-error   tail env)]
             [(define-library) (xform-define-library head tail env #f)]
             [(import)         (xform-import         head tail env #f)]
             [(export program) (x-error "FIXME: not yet implemented" hval sexp)]
             [(... _)          (x-error "improper use of auxiliary syntax" hval sexp)]
             [else (cond [(val-integrable? hval)  (xform-integrable hval tail env)]
                         [(val-transformer? hval) (xform appos? (hval sexp env) env)]
                         [(val-library? hval)     (x-error "improper use of library" hval sexp)]
                         [(val-void? hval)        (x-error "use of uninitialized val" hval sexp)]
                         [(not (val-core? hval))  (x-error "improper use of syntax form" hval)]
                         [else                    (xform-call hval tail env)])]))]))

(define (xform-quote tail env)
  (if (list1? tail)
      (list 'quote (xform-sexp->datum (car tail)))
      (x-error "improper quote form" (cons 'quote tail))))

(define (xform-ref id env)
  (location-val (xenv-ref env id))) 

(define (xform-set! tail env)
  (if (and (list2? tail) (id? (car tail)))
      (let ([den (xenv-lookup env (car tail) 'set!)] [xexp (xform #f (cadr tail) env)])
        (if (location-special? den) ;; was (location-set-val! den xexp) '(begin)
            (x-error "set! to macro or integrable identifier" (cons 'set! tail))
            (let ([val (location-val den)])
              (if (eq? (car val) 'ref)
                  (list 'set! (cadr val) xexp)
                  (x-error "set! is not allowed" (cons 'set! tail))))))
      (x-error "improper set! form" (cons 'set! tail))))

(define (xform-set& tail env)
  (if (list1? tail)
      (let ([den (xenv-lookup env (car tail) 'set!)])      
        (if (location-special? den) 
            (x-error "set& of macro or integrable identifier" (cons 'set& tail))
            (let ([val (location-val den)])
              (if (eq? (car val) 'ref)
                  (list 'set& (cadr val))
                  (x-error "set& is not allowed" (cons 'set! tail))))))
      (x-error "improper set& form" (cons 'set& tail))))

(define (xform-if tail env)
  (if (list? tail)
      (let ([xexps (map (lambda (sexp) (xform #f sexp env)) tail)])
        (case (length xexps)
          [(2) (cons 'if (append xexps '((begin))))]
          [(3) (cons 'if xexps)]
          [else (x-error "malformed if form" (cons 'if tail))]))
      (x-error "improper if form" (cons 'if tail))))

(define (xform-call xexp tail env)
  (if (list? tail)
      (let ([xexps (map (lambda (sexp) (xform #f sexp env)) tail)])
        (if (and (null? xexps) (eq? (car xexp) 'lambda) (null? (cadr xexp)))
            (caddr xexp) ; ((let () x)) => x
            (pair* 'call xexp xexps)))
      (x-error "improper application" (cons xexp tail))))

(define (integrable-argc-match? igt n)
  (case igt
    [(#\0) (=  n 0)]   [(#\1) (=  n 1)]   [(#\2) (=  n 2)]   [(#\3) (=  n 3)] 
    [(#\p) (>= n 0)]   [(#\m) (>= n 1)]   [(#\c) (>= n 2)]   [(#\x) (>= n 1)]
    [(#\u) (<= 0 n 1)] [(#\b) (<= 1 n 2)] [(#\t) (<= 2 n 3)]
    [(#\#) (>= n 0)]   [(#\@) #f]
    [else  #f])) 

(define (xform-integrable ig tail env)
  (if (integrable-argc-match? (integrable-type ig) (length tail))
      (cons 'integrable (cons ig (map (lambda (sexp) (xform #f sexp env)) tail)))
      (xform-call (list 'ref (integrable-global ig)) tail env)))

(define (xform-lambda tail env)
  (if (and (list1+? tail) (idslist? (car tail)))
      (let loop ([vars (car tail)] [ienv env] [ipars '()])
        (cond [(pair? vars)
               (let* ([var (car vars)] [nvar (gensym (id->sym var))])
                 (loop (cdr vars) (add-local-var var nvar ienv) (cons nvar ipars)))]
              [(null? vars)
               (list 'lambda (reverse ipars) (xform-body (cdr tail) ienv #f))]
              [else ; improper 
               (let* ([var vars] [nvar (gensym (id->sym var))] 
                      [ienv (add-local-var var nvar ienv)])
                 (list 'lambda (append (reverse ipars) nvar)
                   (xform-body (cdr tail) ienv #f)))]))
      (x-error "improper lambda body" (cons 'lambda tail))))

(define (xform-lambda* tail env)
  (if (list? tail)
      (cons 'lambda*
         (map (lambda (aexp)
                 (if (and (list2? aexp) 
                          (or (and (list2? (car aexp)) 
                                   (fixnum? (caar aexp)) 
                                   (boolean? (cadar aexp)))
                              (idslist? (car aexp))))
                     (list (normalize-arity (car aexp))
                           (xform #f (cadr aexp) env))
                     (x-error "improper lambda* clause" aexp)))
              tail))
      (x-error "improper lambda* form" (cons 'lambda* tail))))

(define (xform-letcc tail env)
  (if (and (list2+? tail) (id? (car tail)))
      (let* ([var (car tail)] [nvar (gensym (id->sym var))])
        (list 'letcc nvar 
          (xform-body (cdr tail) (add-local-var var nvar env) #f)))
      (x-error "improper letcc form" (cons 'letcc tail))))

(define (xform-withcc tail env)
  (if (list2+? tail)
      (list 'withcc (xform #f (car tail) env)
        (xform-body (cdr tail) env) #f)
      (x-error "improper withcc form" (cons 'withcc tail))))

(define (preprocess-define head tail) ;=> (id sexp) or (sexp) for idless
  (cond [(and (list2? tail) (null? (car tail))) (cdr tail)] ; idless
        [(and (list2? tail) (id? (car tail))) tail] ; simple
        [(and (list2+? tail) (pair? (car tail)) (id? (caar tail)) (idslist? (cdar tail)))
         (list (caar tail) (cons lambda-id (cons (cdar tail) (cdr tail))))]
        ; TODO? here we can do full MIT-style define (arbitrarily nested)
        [else (x-error "improper define form" (cons head tail))]))

(define (preprocess-define-syntax head tail) ;=> (id sexp)
  (cond [(and (list2? tail) (id? (car tail))) tail] ; simple
        ; TODO? here we can do some fancy shortcuts or extensions
        [else (x-error "improper define-syntax form" (cons head tail))]))

(define (xform-body tail env appos?)
  (cond
    [(null? tail) 
     (list 'begin)]
    [(list1? tail) ; can't have defines there
     (xform appos? (car tail) env)]
    [(not (list? tail))
     (x-error "improper body form" (cons 'body tail))]
    [else
     (let loop ([env env] [ids '()] [inits '()] [nids '()] [body tail])
       (if (and (pair? body) (pair? (car body)))
           (let ([first (car body)] [rest (cdr body)])
             (let* ([head (car first)] [tail (cdr first)] [hval (xform #t head env)])
               (case hval
                 [(begin) ; internal
                  (if (list? tail) 
                      (loop env ids inits nids (append tail rest))
                      (x-error "improper begin form" first))]
                 [(define) ; internal
                  (let ([tail (preprocess-define head tail)])
                    (cond [(list1? tail) ; idless
                           (let ([init (car tail)])
                             (loop env (cons #f ids) (cons init inits) (cons #f nids) rest))]
                          [else ; (id sexp)
                           (let* ([id (car tail)] [init (cadr tail)]
                                  [nid (gensym (id->sym id))] [env (add-local-var id nid env)])
                             (loop env (cons id ids) (cons init inits) (cons nid nids) rest))]))]
                 [(define-syntax) ; internal
                  (let ([tail (preprocess-define-syntax head tail)])
                    (let* ([id (car tail)] [init (cadr tail)]
                           [env (extend-xenv-local id '(undefined) env)]) ; placeholder val
                      (loop env (cons id ids) (cons init inits) (cons #t nids) rest)))]
                 [(define-library) ; internal
                  (if (and (list2+? tail) (listname? (car tail)))
                      ; note: library is fully expanded in incomplete env, to make it
                      ; immediately available for import; it ignores lexical scope anyway 
                      (let* ([core (xform-define-library head tail env #f)] 
                             ; core is (define-library <listname> <library>)
                             [listname (cadr core)] [library (caddr core)]
                             [env (extend-xenv-local listname library env)])
                        (loop env ids inits nids rest)) ; no trace for xform-labels
                      (x-error "improper define-library form" first))]
                 [(import) ; internal
                  (if (list? tail)
                      ; note: import is fully expanded in incomplete env, right now! 
                      (let* ([core (xform-import head tail env #f)] ; core is (import <library>)
                             [l (cadr core)] [code (library-code l)] [eal (library-exports l)])
                        (let scan ([eal eal] [env env])
                          (if (null? eal) ; add init code as if it were idless define
                              (let ([init (list syntax-quote-id code)])
                                (loop env (cons #f ids) (cons init inits) (cons #f nids) rest))
                              (let ([id (id-rename-as head (caar eal))] [loc (cdar eal)])
                                (scan (cdr eal) 
                                  (lambda (i at) ; make env sharing loc, but for peek/ref only!
                                    (if (eq? i id) (and (memq at '(ref peek)) loc) (env i at))))))))
                      (x-error "improper import form" first))] 
                 [else
                  (if (val-transformer? hval)
                      (loop env ids inits nids (cons (hval first env) rest))
                      (xform-labels (reverse ids) (reverse inits) (reverse nids) body env appos?))])))
           (xform-labels (reverse ids) (reverse inits) (reverse nids) body env appos?)))]))

(define (xform-labels ids inits nids body env appos?)
  (define no-defines? (andmap (lambda (nid) (eq? nid #t)) nids))
  (let loop ([ids ids] [inits inits] [nids nids] [sets '()] [lids '()])
    (cond [(null? ids)
           (if (and no-defines? (list1? body))
               ; special case: expand body using current appos?
               (xform appos? (car body) env)
               ; general case: produce expression    
               (let* ([xexps (append (reverse sets) (map (lambda (x) (xform #f x env)) body))]
                      [xexp (if (list1? xexps) (car xexps) (cons 'begin xexps))])
                 (if (null? lids) xexp
                     (pair* 'call (list 'lambda (reverse lids) xexp)
                       (map (lambda (lid) '(begin)) lids)))))]
          [(not (car ids)) ; idless define, nid is #f
           (loop (cdr ids) (cdr inits) (cdr nids)
             (cons (xform #f (car inits) env) sets) lids)]
          [(symbol? (car nids)) ; define
           (loop (cdr ids) (cdr inits) (cdr nids)
             (cons (xform-set! (list (car ids) (car inits)) env) sets)
             (cons (car nids) lids))]
          [else ; define-syntax, nid is #t
           (location-set-val! (xenv-lookup env (car ids) 'set!) (xform #t (car inits) env))
           (loop (cdr ids) (cdr inits) (cdr nids) sets lids)])))

; FIXME: make sure that (begin (begin) x (begin)) == x !! (tail-rec includes hack)
(define (xform-begin tail env appos?) ; non-internal
  (if (list? tail) 
      (if (list1? tail)
          (xform appos? (car tail) env) ; (begin x) == x
          (cons 'begin (map (lambda (sexp) (xform #f sexp env)) tail)))
      (x-error "improper begin form" (cons 'begin tail))))

; used in simplistic transformer in scm2c.ssc only!
(define (xform-define tail env) 
  (cond [(and (list2? tail) (null? (car tail))) ; idless
         (xform #f (cadr tail) env)]
        [(and (list2? tail) (id? (car tail)))
         (list 'define (id->sym (car tail)) 
           (xform #f (cadr tail) env))]
        [(and (list2+? tail) (pair? (car tail)) (id? (caar tail)) (idslist? (cdar tail)))
         (list 'define (id->sym (caar tail))
           (xform-lambda (cons (cdar tail) (cdr tail)) env))] 
        [else (x-error "improper define form" (cons 'define tail))]))

; used in simplistic transformer in scm2c.ssc only!
(define (xform-define-syntax tail env) ; non-internal
  (if (and (list2? tail) (id? (car tail)))
      (list 'define-syntax (id->sym (car tail)) (xform #t (cadr tail) env))
      (x-error "improper define-syntax form" (cons 'define-syntax tail))))

(define (xform-syntax-quote tail env)
  (if (list1? tail)
      (car tail) ; must be <core>, todo: check?
      (x-error "improper syntax-quote form" (cons 'syntax-quote tail))))

(define (xform-syntax-lambda tail env appos?)
  (if (and (list2+? tail) (andmap id? (car tail)))
      (let ([vars (car tail)] [macenv env] [forms (cdr tail)])
        ; return a transformer that wraps xformed body in (syntax-quote ...)
        ; to make sure xform treats it as final <core> form and exits the loop
        (lambda (use useenv)
          (if (and (list1+? use) (fx=? (length vars) (length (cdr use))))
              (let loop ([vars vars] [exps (cdr use)] [env macenv])
                (if (null? vars)
                    (list syntax-quote-id (xform-body forms env appos?))
                    (loop (cdr vars) (cdr exps)
                      (extend-xenv-local (car vars) 
                        (xform #t (car exps) useenv) env))))  
              (x-error "invalid syntax-lambda application" use))))  
      (x-error "improper syntax-lambda body" (cons 'syntax-lambda tail))))

(define (xform-syntax-rules tail env)
  (cond [(and (list2+? tail) (id? (car tail)) (andmap id? (cadr tail)))
         (syntax-rules* env (car tail) (cadr tail) (cddr tail))]
        [(and (list1+? tail) (andmap id? (car tail)))
         (syntax-rules* env #f (car tail) (cdr tail))]
        [else
         (x-error "improper syntax-rules form" (cons 'syntax-rules tail))]))

(define (xform-syntax-length tail env)
  (if (and (list1? tail) (list? (car tail)))
      (list 'quote (length (car tail)))
      (x-error "improper syntax-length form" (cons 'syntax-length tail))))

(define (xform-syntax-error tail env)
  (let ([args (map xform-sexp->datum tail)])
    (if (and (list1+? args) (string? (car args)))
        (apply x-error args)
        (x-error "improper syntax-error form" (cons 'syntax-error tail)))))


; make transformer procedure from the rules

(define (syntax-rules* mac-env ellipsis pat-literals rules)

  (define (pat-literal? id) (memq id pat-literals))

  (define (not-pat-literal? id) (not (pat-literal? id)))

  (define (ellipsis-pair? x)
    (and (pair? x) (ellipsis? (car x))))

  ; root-environment may be not yet defined, so instead of this test:
  ; (free-id=? x mac-env '... root-environment) we will do it manually;
  ; nb: 'real' ... is a builtin, at this time possibly registered in rnr
  (define ellipsis-den ; we may need to be first to alloc ... binding!
    (name-lookup *root-name-registry* '... (lambda (n) '...)))
  ; now we need just peek x in maro env to compare with the above
  (define (ellipsis? x)
    (if ellipsis (eq? x ellipsis) ; custom one is given
        (and (id? x) (eq? (mac-env x 'peek) ellipsis-den))))

  ; ditto for underscore
  (define underscore-den ; we may need to be first to alloc _ binding!
    (name-lookup *root-name-registry* '_ (lambda (n) '_)))
  (define (underscore? x)
    (and (id? x) (eq? (mac-env x 'peek) underscore-den)))

  ; List-ids returns a list of the non-ellipsis ids in a pattern or template for which 
  ; (pred? id) is true.  If include-scalars is false, we only include ids that are
  ; within the scope of at least one ellipsis.
  (define (list-ids x include-scalars pred?)
    (let collect ([x x] [inc include-scalars] [l '()])
      (cond [(id? x) (if (and inc (pred? x)) (cons x l) l)]
            [(vector? x) (collect (vector->list x) inc l)]
            [(pair? x)
             (if (ellipsis-pair? (cdr x))
                 (collect (car x) #t (collect (cddr x) inc l))
                 (collect (car x) inc (collect (cdr x) inc l)))]
            [else l])))

  ; calculate the length of the post-... part of the pattern / input sexp
  (define (proper-head-length lst check-ellipsis?)
    (let loop ([p lst] [len 0])
      (cond [(not (pair? p)) len]
            [(and check-ellipsis? (ellipsis? (car p)))
             (x-error "misplaced ellipsis in syntax-case pattern" lst)]
            [else (loop (cdr p) (fx+ len 1))])))

  ; Returns #f or an alist mapping each pattern var to a part of the input. Ellipsis vars
  ; are mapped to lists of parts (or lists of lists ...). There is no mapping for underscore
  (define (match-pattern pat use use-env)
    (call-with-current-continuation
      (lambda (return)
        (define (fail) (return #f))
        (let match ([pat pat] [sexp use] [bindings '()])
          (define (continue-if condition)
            (if condition bindings (fail)))
          (cond
            [(underscore? pat) bindings]
            [(id? pat)
             (if (pat-literal? pat)
                 (continue-if (and (id? sexp) (free-id=? sexp use-env pat mac-env)))
                 (cons (cons pat sexp) bindings))]
            [(vector? pat)
             (or (vector? sexp) (fail))
             (match (vector->list pat) (vector->list sexp) bindings)]
            [(not (pair? pat))
             (continue-if (equal? pat sexp))]
            [(ellipsis-pair? (cdr pat))
             (let* ([tail-len (proper-head-length (cddr pat) #t)] 
                    [sexp-len (proper-head-length sexp #f)]
                    [seq-len (fx- sexp-len tail-len)]
                    [sexp-tail (if (negative? seq-len) (fail) (list-tail sexp seq-len))]
                    [seq (list-head sexp seq-len)]
                    [vars (list-ids (car pat) #t not-pat-literal?)])
               (define (match1 sexp)
                 (map cdr (match (car pat) sexp '())))
               (append
                 (apply map (cons list (cons vars (map match1 seq))))
                 (match (cddr pat) sexp-tail bindings)))]
            [(pair? sexp)
             (match (car pat) (car sexp)
               (match (cdr pat) (cdr sexp) bindings))]
            [else (fail)])))))

  (define (expand-template pat tmpl top-bindings)
    ; New-literals is an alist mapping each literal id in the
    ; template to a fresh id for inserting into the output.  It
    ; might have duplicate entries mapping an id to two different
    ; fresh ids, but that's okay because when we go to retrieve a
    ; fresh id, assq will always retrieve the same (first) one.
    (define new-literals
      (body
        (define nl
          (map (lambda (id) (cons id (new-literal-id id mac-env (lambda () nl))))
               (list-ids tmpl #t (lambda (id) (not (assq id top-bindings))))))
        nl))

    (define ellipsis-vars
      (list-ids pat #f not-pat-literal?))

    (define (list-ellipsis-vars subtmpl)
      (list-ids subtmpl #t (lambda (id) (memq id ellipsis-vars))))

    (let expand ([tmpl tmpl] [bindings top-bindings] [esc? #f])
      (let expand-part ([tmpl tmpl] [esc? esc?])
        (cond
          [(id? tmpl)
           (cdr (or (assq tmpl bindings)
                    (assq tmpl top-bindings)
                    (assq tmpl new-literals)))]
          [(vector? tmpl)
           (list->vector (expand-part (vector->list tmpl) esc?))]
          [(and (not esc?) (pair? tmpl) (ellipsis? (car tmpl))) ; r7rs
           (if (pair? (cdr tmpl)) (expand-part (cadr tmpl) #t)
               (x-error "invalid escaped template syntax" tmpl))]
          [(and (not esc?) (pair? tmpl) (ellipsis-pair? (cdr tmpl)))
           (let ([vars-to-iterate (list-ellipsis-vars (car tmpl))])
             (define (lookup var)
               (cdr (assq var bindings)))
             (define (expand-using-vals esc? . vals)
               (expand (car tmpl) (map cons vars-to-iterate vals) esc?))
             (if (null? vars-to-iterate)
                 ; ellipsis following non-repeatable part is an error, but we don't care
                 (cons (expand-part (car tmpl) esc?) 
                       (expand-part (cddr tmpl) esc?)) ; 'repeat' once
                 ; correct use of ellipsis
                 (let ([val-lists (map lookup vars-to-iterate)]
                       [euv (lambda v* (apply expand-using-vals esc? v*))])
                   (append (apply map (cons euv val-lists)) (expand-part (cddr tmpl) esc?)))))]
          [(pair? tmpl)
           (cons (expand-part (car tmpl) esc?) (expand-part (cdr tmpl) esc?))]
          [else tmpl]))))

  (lambda (use use-env)
    (let loop ([rules rules])
      (if (null? rules) (x-error "invalid syntax" use))
      (let* ([rule (car rules)] [pat (car rule)] [tmpl (cadr rule)])
        (cond [(match-pattern pat use use-env) =>
               (lambda (bindings) (expand-template pat tmpl bindings))]
              [else (loop (cdr rules))])))))


; hand-made transformers (use functionality defined below)

(define (make-include-transformer ci?)
  (define (push-current-file-transformer sexp env)
    (unless (and (list2? sexp) (string? (cadr sexp))) (x-error "invalid syntax" sexp))
    (push-current-file! (cadr sexp)) (list begin-id))
  (define (pop-current-file-transformer sexp env)
    (unless (list1? sexp) (x-error "invalid syntax" sexp))
    (pop-current-file!) (list begin-id))
  (define push-cf-id (new-id 'push-cf (make-location push-current-file-transformer) #f))
  (define pop-cf-id (new-id 'pop-cf (make-location pop-current-file-transformer) #f))
  (lambda (sexp env)
    (unless (list1+? sexp) (x-error "invalid syntax" sexp))
    (let loop ([files (cdr sexp)] [exp-lists '()])
      (if (null? files)
          (cons begin-id (apply append (reverse! exp-lists)))
          (let* ([filepath (file-resolve-relative-to-current (car files))]
                 [fileok? (and (string? filepath) (file-exists? filepath))]
                 [test (if fileok? #t (x-error "cannot include" (car files) sexp))]
                 [sexps (read-file-sexps filepath ci?)]
                 [wrapped-sexps `((,push-cf-id ,filepath) ,@sexps (,pop-cf-id))])
            (loop (cdr files) (cons wrapped-sexps exp-lists)))))))

; return the right ce branch using (lit=? id sym) for literal match
(define (preprocess-cond-expand lit=? sexp env) ;=> (sexp ...)
  (define (pp freq con alt)
    (cond [(lit=? freq 'else) (con)]
          [(id? freq) (if (feature-available? (id->sym freq)) (con) (alt))]
          [(and (list2? freq) (lit=? (car freq) 'library))
           (if (library-available? (xform-sexp->datum (cadr freq)) env) (con) (alt))]
          [(and (list1+? freq) (lit=? (car freq) 'and))
           (cond [(null? (cdr freq)) (con)] [(null? (cddr freq)) (pp (cadr freq) con alt)]
                 [else (pp (cadr freq) (lambda () (pp (cons (car freq) (cddr freq)) con alt)) alt)])]
          [(and (list1+? freq) (lit=? (car freq) 'or))
           (cond [(null? (cdr freq)) (alt)] [(null? (cddr freq)) (pp (cadr freq) con alt)]
                 [else (pp (cadr freq) con (lambda () (pp (cons (car freq) (cddr freq)) con alt)))])]
          [(and (list2? freq) (lit=? (car freq) 'not)) (pp (cadr freq) alt con)]
          [else (x-error "invalid cond-expand feature requirement" freq)]))
  (check-syntax sexp '(<id> (* * ...) ...) "invalid cond-expand syntax")
  (let loop ([clauses (cdr sexp)])
    (if (null? clauses) '()
        (pp (caar clauses) (lambda () (cdar clauses)) (lambda () (loop (cdr clauses)))))))

(define (make-cond-expand-transformer)
  (lambda (sexp env)
    (define (lit=? id sym) ; match literal as free identifier
      (and (id? id) (free-id=? id env sym root-environment sym)))
    (cons begin-id (preprocess-cond-expand lit=? sexp env))))

; library transformers

; code is a <core> scheme expression as produced by the expander
(define (adjoin-code code1 code2) ;=> code12, in original order
  (cond [(equal? code1 '(begin)) code2]
        [(equal? code2 '(begin)) code1]
        [(and (sexp-match? '(begin * ...) code1) (sexp-match? '(begin * ...) code2))
         (cons 'begin (append (cdr code1) (cdr code2)))] 
        [(sexp-match? '(begin * ...) code1) (cons 'begin (append (cdr code1) (list code2)))] 
        [(sexp-match? '(begin * ...) code2) (cons 'begin (cons code1 (cdr code2)))] 
        [else (list 'begin code1 code2)]))

; eals are alist-like export lists ((<sym> . <den>) ...)
(define (adjoin-eals eal1 eal2) ;=> eal12
  (if (null? eal1) eal2
      (let ([eal2 (adjoin-eals (cdr eal1) eal2)])
        (cond [(assq (caar eal1) eal2) => 
               (lambda (p)
                  (if (eq? (cdar eal1) (cdr p))
                      eal2 ; repeat of same id with same denotation is allowed
                      (x-error "multiple identifier bindings on import" (car eal1) p)))]
              [else (cons (car eal1) eal2)]))))

; esps is a list of export specs, each spec is (<old-id> . <new-id>)
(define (adjoin-esps esps1 esps2) ;=> esps12
  (if (null? esps1) esps2 ; assume esps2 is already checked
      (let ([esp (car esps1)] [esps (adjoin-esps (cdr esps1) esps2)])
        (cond [(member esp esps) esps] ; duplicate, but same rename -- already checked, ok
              [(assq (car esp) esps) => (lambda (p) (x-error "duplicate identifier exports" esp p))]
              [(rassq (cdr esp) esps) => (lambda (p) (x-error "conflicting identifier exports" esp p))]
              [else (cons esp esps)]))))

(define (preprocess-import-sets sexp env) ;=> (init-core . exports-eal)
  (define (twoids? x) (and (list2? x) (id? (car x)) (id? (cadr x)))) 
  (define (libpart? x) (or (id? x) (exact-integer? x)))
  (check-syntax sexp '(<id> * ...) "invalid import syntax")
  (let* ([sid (car sexp)] ; reference id to capture names entered by user
         [is-only-id (id-rename-as sid 'only)] [is-except-id (id-rename-as sid 'except)]
         [is-rename-id (id-rename-as sid 'rename)] [is-prefix-id (id-rename-as sid 'prefix)]
         [is-library-id (id-rename-as sid 'library)])
    (define (pp s return)
      (define special (and (list2+? s) (pair? (cadr s))))
      (cond
        [(and special (eq? (car s) is-only-id) (andmap id? (cddr s)))
         (pp (cadr s) ;=>
           (lambda (code al) 
             (return code            
               (let loop ([al al] [ids (map id->sym (cddr s))])
                 (cond [(null? al) al]
                       [(memq (caar al) ids) (cons (car al) (loop (cdr al) ids))]
                       [else (loop (cdr al) ids)])))))]
        [(and special (eq? (car s) is-except-id) (andmap id? (cddr s)))
         (pp (cadr s) ;=>
           (lambda (code al) 
             (return code            
               (let loop ([al al] [ids (map id->sym (cddr s))])
                 (cond [(null? al) al]
                       [(memq (caar al) ids) (loop (cdr al) ids)]
                       [else (cons (car al) (loop (cdr al) ids))])))))]
        [(and special (eq? (car s) is-prefix-id) (list2? (cdr s)) (id? (caddr s)))
         (pp (cadr s) ;=>
           (lambda (code al) 
             (return code            
               (let loop ([al al] [pfx (id->sym (caddr s))])
                 (cond [(null? al) al]
                       [else (let ([nn (symbol-append pfx (caar al))])
                               (cons (cons nn (cdar al)) (loop (cdr al) pfx)))])))))]
        [(and special (eq? (car s) is-rename-id) (andmap twoids? (cddr s)))
         (pp (cadr s) ;=>
           (lambda (code al) 
             (return code            
               (let loop ([al al] [idpairs (xform-sexp->datum (cddr s))])
                 (cond [(null? al) al]
                       [(assq (caar al) idpairs) => 
                        (lambda (idpair) (cons (cons (cadr idpair) (cdar al)) (loop (cdr al) idpairs)))]
                       [else (cons (car al) (loop (cdr al) idpairs))])))))]
        [(and (list2+? s) (eq? (car s) is-library-id))
         (let ([ic&ex (preprocess-library s env)])
           (return (car ic&ex) (cdr ic&ex)))]
        [(and (list1+? s) (andmap libpart? s))
         ; NB: this is part 1/4 of listname <-> library interaction
         (let* ([listname (xform-sexp->datum s)] [val (xform-ref listname env)])
           (unless (val-library? val) (x-error "invalid library" listname val))
           (return (library-code val) (library-exports val)))]
        [else
         (x-error "invalid import set in import" s)]))
    (let loop ([isets (cdr sexp)] [code '(begin)] [eal '()])
      (if (null? isets)
          (cons code eal)
          (pp (car isets) ;=>
            (lambda (new-code new-eal)
              (loop (cdr isets) 
                (adjoin-code code new-code)
                (adjoin-eals new-eal eal))))))))

(define (preprocess-library-declarations sexp env)  ;=> (init-code import-eal esps forms)
  (check-syntax sexp '(<id> (<id> * ...) ...) "invalid library declarations syntax")
  (let* ([sid (car sexp)] ; reference id to capture names entered by user
         [ld-export-id (id-rename-as sid 'export)] [ld-import-id (id-rename-as sid 'import)]
         [ld-include-id (id-rename-as sid 'include)] [ld-include-ci-id (id-rename-as sid 'include-ci)]         
         [ld-begin-id (id-rename-as sid 'begin)] [ld-rename-id (id-rename-as sid 'rename)] 
         [ld-cond-expand-id (id-rename-as sid 'cond-expand)]
         [ld-push-cf-id (id-rename-as sid 'push-cf)] [ld-pop-cf-id (id-rename-as sid 'pop-cf)]
         [ld-library-declarations-id (id-rename-as sid 'library-declarations)]
         [ld-include-library-declarations-id (id-rename-as sid 'include-library-declarations)]
         [include-id (new-id 'include (make-location (make-include-transformer #f)) #f)]
         [include-ci-id (new-id 'include-ci (make-location (make-include-transformer #t)) #f)])
    (define (toesps ee esps) ;=> ((<old-id> . <new-id>) ...)
      (cond [(null? ee) (reverse! esps)]
            [(id? (car ee)) (let ([s (id->sym (car ee))]) 
             (toesps (cdr ee) (adjoin-esps (list (cons s s)) esps)))]
            [(and (sexp-match? '(<id> <id> <id>) (car ee)) (eq? (caar ee) ld-rename-id))
             (toesps (cdr ee) (adjoin-esps (list (cons (id->sym (cadar ee)) (id->sym (caddar ee)))) esps))]
            [else (x-error "invalid export spec element" (xform-sexp->datum (car ee)))]))
    (let loop ([decls (cdr sexp)] [code '(begin)] [eal '()] [esps '()] [forms '()])
      (if (null? decls)
          (list code eal esps forms)
          (let ([decl (car decls)] [decls (cdr decls)])
            (cond ; precondition: decl matches (<id> * ...) 
              [(eq? (car decl) ld-export-id)
               (loop decls code eal (adjoin-esps (toesps (cdr decl) '()) esps) forms)]
              [(and (list2? decl) (eq? (car decl) ld-import-id) (eq? (cadr decl) ld-import-id))
               (let ([new-eal (list (cons 'import (make-location 'import)))])
                 (loop decls code (adjoin-eals new-eal eal) esps forms))]
              [(eq? (car decl) ld-import-id)
               (let ([ic&ex (preprocess-import-sets decl env)])
                 (let ([new-code (car ic&ex)] [new-eal (cdr ic&ex)])
                   (loop decls (adjoin-code code new-code) (adjoin-eals new-eal eal) esps forms)))]
              [(eq? (car decl) ld-library-declarations-id) ; splice
               (loop (append (cdr decl) decls) code eal esps forms)]
              [(eq? (car decl) ld-cond-expand-id) ; flatten and splice
               (let ([lit=? (lambda (id sym) (and (id? id) (eq? id (id-rename-as sid sym))))])
                 (loop (append (preprocess-cond-expand lit=? decl env) decls) code eal esps forms))]
              [(eq? (car decl) ld-push-cf-id) ; internal
               (check-syntax decl '(<id> <string>) "invalid library declarations syntax")
               (push-current-file! (cadr decl))
               (loop decls code eal esps forms)]
              [(eq? (car decl) ld-pop-cf-id) ; internal
               (check-syntax decl '(<id>) "invalid library declarations syntax")
               (pop-current-file!)
               (loop decls code eal esps forms)]
              [(eq? (car decl) ld-include-library-declarations-id) ; splice
               (check-syntax decl '(<id> <string> ...) "invalid include-library-declarations syntax")
               (let lp ([files (reverse (cdr decl))] [decls decls])
                 (if (null? files) (loop decls code eal esps forms)
                     (let* ([filepath (file-resolve-relative-to-current (car files))]
                            [fileok? (and (string? filepath) (file-exists? filepath))]
                            [test (if fileok? #t (x-error "cannot include declarations" (car files) sexp))]
                            [decl* (read-file-sexps filepath #f)]) ; no ci?
                       (lp (cdr files) `((,ld-push-cf-id ,filepath) ,@decl* (,ld-pop-cf-id) . ,decls)))))]
              [(eq? (car decl) ld-include-id)
               (check-syntax decl '(<id> <string> ...) "invalid include library declaration syntax")
               (loop decls code eal esps `(,@forms (,include-id . ,(cdr decl))))]
              [(eq? (car decl) ld-include-ci-id)
               (check-syntax decl '(<id> <string> ...) "invalid include-ci library declaration syntax")
               (loop decls code eal esps `(,@forms (,include-ci-id . ,(cdr decl))))]
              [(eq? (car decl) ld-begin-id)
               (loop decls code eal esps (append forms (xform-sexp->datum (cdr decl))))]))))))

; scan forms and return reversed list of core forms, interspersed with (define gs exp)
; forms that need further processing (each one will become (set! gs core) form
(define (preprocess-top-forms-scan forms cenv env) ;=> code* in reverse order
  (define (scan body code*) ;=> code* extended, with side-effect on cenv
    (if (null? body) 
        code*
        (let ([first (car body)] [rest (cdr body)])
          (if (pair? first)
              (let* ([head (car first)] [tail (cdr first)] [hval (xform #t head cenv)])
                (cond
                  [(eq? hval 'begin)
                   (unless (list? tail) (x-error "improper begin form" first))
                   (scan (append tail rest) code*)] ; splice
                  [(eq? hval 'define)
                   (let ([tail (preprocess-define head tail)])
                     (if (list1? tail) ; tail is either (sexp) or (id sexp)
                         (scan (append tail rest) code*) ; idless, splice
                         (let ([loc (top-defined-id-lookup cenv (car tail) 'define)])
                           (unless (and (location? loc) (sexp-match? '(ref *) (location-val loc)))
                             (x-error "unexpected define for id" (car tail) first))
                           (let ([gs (cadr (location-val loc))] [exp (cadr tail)])
                             ; exp expansiom is delayed until all defined ids are ready
                             (scan rest (cons (list 'define gs exp) code*))))))] 
                  [(eq? hval 'define-syntax)
                   (let* ([tail (preprocess-define-syntax head tail)]
                          [loc (top-defined-id-lookup cenv (car tail) 'define-syntax)])
                     (unless (location? loc) 
                       (x-error "unexpected define-syntax for id" (car tail) first))
                     (location-set-val! loc (xform #t (cadr tail) cenv))
                     (scan rest code*))]
                  [(eq? hval 'define-library)
                   (let* ([core (xform-define-library head tail env #f)]
                          ; core is (define-library <listname> <library>)
                          [loc (xenv-lookup env (cadr core) 'define-syntax)])
                     (unless (location? loc) 
                       (x-error "unexpected define-library for id" (cadr core) first))
                     (location-set-val! loc (caddr core))
                     (scan rest code*))]
                  [(eq? hval 'import) ; support, in case there is an internal import
                   (let* ([core (xform-import head tail cenv #f)] 
                          ; core is (import <library>)
                          [l (cadr core)] [code (library-code l)] [eal (library-exports l)])
                     (unless (cenv eal 'import) ; adjoins eal to cenv's imports
                       (x-error "broken import inside library code" first))
                     (scan rest (cons code code*)))] ; adds library init code 
                  [(val-transformer? hval) ; apply transformer and loop
                   (scan (cons (hval first cenv) rest) code*)]
                  [else ; form expansion may need to be delayed
                   (scan rest (cons (list 'define '() first) code*))]))
              ; form expansion may need to be delayed
              (scan rest (cons (list 'define '() first) code*)))))) 
  (scan forms '()))

; scan returns underprocessed defines; this fn fixes that
(define (preprocess-top-form-fix! code cenv) ;=> core
  (if (and (pair? code) (eq? (car code) 'define) (list3? code))
      (let* ([gs (cadr code)] [exp (caddr code)] [core (xform #f exp cenv)])
        (if (null? gs) core (list 'set! gs core)))
      code))

(define (preprocess-library sexp env) ;=> (init-core . exports-eal)
  ; generator of globals: use prefix or temporary if no prefix is given 
  (define (make-nid id) 
    (if (and (list2+? sexp) (id? (cadr sexp))) ; got library id
        (fully-qualified-library-prefixed-name (id->sym (cadr sexp)) (id->sym id))
        (gensym (id->sym id))))
  ; (library lib? library-declaration ...)
  (check-syntax sexp '(* * ...) "invalid library syntax")
  (let* ([lid (if (and (list2+? sexp) (id? (cadr sexp))) (id->sym (cadr sexp)) #f)]
         [decls (if lid (cddr sexp) (cdr sexp))] ; NB: mac env is used below to resolve lib names!
         [icimesfs (preprocess-library-declarations (cons (car sexp) decls) env)])
    (let* ([code (car icimesfs)] [ial (cadr icimesfs)] [esps (caddr icimesfs)] [forms (cadddr icimesfs)] 
           [cenv (make-controlled-environment ial make-nid env)] [eal '()]) ; m-c-e is defined below
      (let* ([code* (preprocess-top-forms-scan forms cenv env)]
             [fix! (lambda (code) (preprocess-top-form-fix! code cenv))] 
             [forms-code (cons 'begin (map fix! (reverse! code*)))] 
             [combined-code (adjoin-code code (if lid (list 'once lid forms-code) forms-code))])
        ; walk through esps, fetching locations from cenv
        (let loop ([esps esps] [eal eal])
          (if (null? esps)
              (cons combined-code (reverse! eal))
              (let* ([lid (caar esps)] [eid (cdar esps)] [loc (cenv lid 'ref)])
                (cond [(not loc) (x-error "cannot export id" lid)]
                      [(location-special? loc) (loop (cdr esps) (cons (cons eid loc) eal))]
                      [else (let ([val (location-val loc)])
                              (if (or (not (val-core? val)) (memq (car val) '(ref const)))
                                  (loop (cdr esps) (cons (cons eid loc) eal))
                                  (x-error "cannot export code alias id" lid val)))]))))))))

; Note: define-library semantics does not depend on lexical context, and, as a syntax definition,
; it should become available in local env immediately, even at definition-scanning phase -- so we
; introduce new special <core> form define-library

(define (xform-define-library head tail env top?) ; non-internal
  (if (and (list2+? tail) (listname? (car tail)))
      (let* ([listname (xform-sexp->datum (car tail))]
             [prefix (and top? (listname->symbol listname))]
             ; NB: head is used as seed id for renamings; fixed prefix used on top only
             [libform (cons head (if prefix (cons prefix (cdr tail)) (cdr tail)))]
             [ic&ex (preprocess-library libform env)])
        ; NB: this is part 2/4 of listname <-> library interaction
        (list 'define-library listname (make-library (car ic&ex) (cdr ic&ex))))
      (x-error "improper define-library form" (cons head tail))))

; for now, we have no clear idea of how to process import in all possible contexts, so we will also
; introduce new special <core> form import

(define (xform-import head tail env top?)
  (if (list? tail)
      (let ([ic&ex (preprocess-import-sets (cons head tail) env)])
        ; NB: this is part 3/4 of listname <-> library interaction
        (list 'import (make-library (car ic&ex) (cdr ic&ex))))
      (x-error "improper import form" (cons head tail))))


;--------------------------------------------------------------------------------------------------
; String representation of S-expressions and code arguments
;--------------------------------------------------------------------------------------------------

(define (write-serialized-char x port)
  (cond [(or (char=? x #\%) (char=? x #\") (char=? x #\\) (char<? x #\space) (char>? x #\~))
         (write-char #\% port)
         (let ([s (fixnum->string (char->integer x) 16)])
           (if (fx=? (string-length s) 1) (write-char #\0 port))
           (write-string s port))]
        [else (write-char x port)]))

(define (write-serialized-byte x port)
  (let ([s (fixnum->string x 16)])
    (if (fx=? (string-length s) 1) (write-char #\0 port))
    (write-string s port)))

(define (write-serialized-size n port)         
  (write-string (fixnum->string n 10) port)
  (write-char #\: port))

(define (write-serialized-element x port)
  (write-serialized-sexp x port)
  (write-char #\; port))

(define (write-serialized-sexp x port)
  (cond [(eq? x #f) 
         (write-char #\f port)]
        [(eq? x #t) 
         (write-char #\t port)]
        [(eq? x '()) 
         (write-char #\n port)]
        [(char? x)
         (write-char #\c port)
         (write-serialized-char x port)]
        [(number? x)
         (write-char (if (exact? x) #\i #\j) port)
         (write-string (number->string x 10) port)]
        [(list? x)
         (write-char #\l port)
         (write-serialized-size (length x) port)
         (do ([x x (cdr x)]) [(null? x)]
           (write-serialized-element (car x) port))]
        [(pair? x)
         (write-char #\p port)
         (write-serialized-element (car x) port) 
         (write-serialized-element (cdr x) port)]
        [(vector? x)
         (write-char #\v port)
         (write-serialized-size (vector-length x) port)
         (do ([i 0 (fx+ i 1)]) [(fx=? i (vector-length x))]
           (write-serialized-element (vector-ref x i) port))]
        [(string? x)
         (write-char #\s port)
         (write-serialized-size (string-length x) port)
         (do ([i 0 (fx+ i 1)]) [(fx=? i (string-length x))]
           (write-serialized-char (string-ref x i) port))]
        [(bytevector? x)
         (write-char #\b port)
         (write-serialized-size (bytevector-length x) port)
         (do ([i 0 (fx+ i 1)]) [(fx=? i (bytevector-length x))]
           (write-serialized-byte (bytevector-u8-ref x i) port))]
        [(symbol? x)
         (write-char #\y port)
         (let ([x (symbol->string x)])
           (write-serialized-size (string-length x) port)
           (do ([i 0 (fx+ i 1)]) [(fx=? i (string-length x))]
             (write-serialized-char (string-ref x i) port)))]
        [(box? x)
         (write-char #\z port)
         (write-serialized-element (unbox x) port)]
        [else (c-error "cannot encode literal" x)]))

(define (write-serialized-arg arg port)
  (if (and (number? arg) (exact? arg) (fx<=? 0 arg) (fx<=? arg 9))
      (write-char (string-ref "0123456789" arg) port)
      (begin (write-char #\( port)
            (write-serialized-sexp arg port)
            (write-char #\) port))))


;--------------------------------------------------------------------------------------------------
; Compiler producing serialized code
;--------------------------------------------------------------------------------------------------

(define (c-error msg . args)
  (error* (string-append "compiler: " msg) args))

(define (c-warning msg . args)
  (warning* (string-append "compiler: " msg) args))

(define (find-free* x* b)
  (if (null? x*)
      '()
      (set-union 
        (find-free (car x*) b) 
        (find-free* (cdr x*) b))))

(define (find-free x b)
  (record-case x
    [quote (obj)         '()]
    [gref (gid)          '()]
    [gset! (gid exp)     (find-free exp b)]
    [(ref const) (id)    (if (set-member? id b) '() (list id))]
    [set! (id exp)       (set-union (if (set-member? id b) '() (list id)) (find-free exp b))]
    [set& (id)           (if (set-member? id b) '() (list id))]
    [lambda (idsi exp)   (find-free exp (set-union (flatten-idslist idsi) b))]
    [lambda* clauses     (find-free* (map cadr clauses) b)]
    [letcc (kid exp)     (find-free exp (set-union (list kid) b))]
    [withcc (kexp exp)   (set-union (find-free kexp b) (find-free exp b))]
    [if (ce te ee)       (set-union (find-free ce b) (set-union (find-free te b) (find-free ee b)))]
    [begin exps          (find-free* exps b)]
    [integrable (i . as) (find-free* as b)]
    [call (exp . args)   (set-union (find-free exp b) (find-free* args b))]
    [asm (cstr)          '()]
    [once (gid exp)      (find-free exp b)]
    [(define define-syntax define-library import) tail (c-error "misplaced definition form" x)]
    [else (c-error "unexpected <core> form" x)]))

(define (find-sets* x* v)
  (if (null? x*)
      '()
      (set-union 
        (find-sets (car x*) v) 
        (find-sets* (cdr x*) v))))

(define (find-sets x v)
  (record-case x
    [quote (obj)         '()]
    [gref (gid)          '()]
    [gset! (gid exp)     (find-sets exp v)]
    [(ref const) (id)    '()]
    [set! (id exp)       (set-union (if (set-member? id v) (list id) '()) (find-sets exp v))]
    [set& (id)           (if (set-member? id v) (list id) '())]
    [lambda (idsi exp)   (find-sets exp (set-minus v (flatten-idslist idsi)))]
    [lambda* clauses     (find-sets* (map cadr clauses) v)]
    [letcc (kid exp)     (find-sets exp (set-minus v (list kid)))]
    [withcc (kexp exp)   (set-union (find-sets kexp v) (find-sets exp v))]
    [begin exps          (find-sets* exps v)]
    [if (ce te ee)       (set-union (find-sets ce v) (set-union (find-sets te v) (find-sets ee v)))]
    [integrable (i . as) (find-sets* as v)]
    [call (exp . args)   (set-union (find-sets exp v) (find-sets* args v))]
    [asm (cstr)          '()]
    [once (gid exp)      (find-sets exp v)]
    [(define define-syntax define-library import) tail (c-error "misplaced definition form" x)]
    [else (c-error "unexpected <core> form" x)]))

(define codegen
  ; x: <core> expression to compile
  ; l: local var list (with #f placeholders for nonvar slots)
  ; f: free var list
  ; s: set! var set
  ; g: global var set
  ; k: #f: x goes to ac, N: x is to be returned after (sdrop n)
  ; port: output code goes here
  (lambda (x l f s g k port)
    (record-case x
      [quote (obj)
       (case obj
          [(#t) (write-char #\t port)]
          [(#f) (write-char #\f port)]
          [(()) (write-char #\n port)]
          [else (write-char #\' port) (write-serialized-arg obj port)])
       (when k (write-char #\] port) (write-serialized-arg k port))]
      [gref (gid)
       (write-char #\@ port)
       (write-serialized-arg gid port)
       (when k (write-char #\] port) (write-serialized-arg k port))]
      [gset! (gid x)
       (codegen x l f s g #f port)
       (write-char #\@ port) (write-char #\! port)
       (write-serialized-arg gid port)
       (when k (write-char #\] port) (write-serialized-arg k port))]
      [(ref const) (id)
       (cond [(posq id l) => ; local
              (lambda (n) 
                (write-char #\. port)
                (write-serialized-arg n port)
                (if (set-member? id s) (write-char #\^ port)))]
             [(posq id f) => ; free
              (lambda (n) 
                (write-char #\: port)
                (write-serialized-arg n port)
                (if (set-member? id s) (write-char #\^ port)))]
             [else ; global
              (write-char #\@ port)
              (write-serialized-arg id port)])
       (when k (write-char #\] port) (write-serialized-arg k port))]
      [set! (id x)
       (codegen x l f s g #f port)
       (cond [(posq id l) => ; local
              (lambda (n) 
                (write-char #\. port) (write-char #\! port)
                (write-serialized-arg n port))]
             [(posq id f) => ; free
              (lambda (n) 
                (write-char #\: port) (write-char #\! port)
                (write-serialized-arg n port))]
             [else ; global
              (write-char #\@ port) (write-char #\! port)
              (write-serialized-arg id port)])
       (when k (write-char #\] port) (write-serialized-arg k port))]
      [set& (id)
       (cond [(posq id l) => ; local
              (lambda (n) 
                (write-char #\. port)
                (write-serialized-arg n port))]
             [(posq id f) => ; free
              (lambda (n) 
                (write-char #\: port)
                (write-serialized-arg n port))]
             [else ; global
              (write-char #\` port)
              (write-serialized-arg id port)])
       (when k (write-char #\] port) (write-serialized-arg k port))]
      [begin exps
       (let loop ([xl exps])
         (when (pair? xl)
           (let ([k (if (pair? (cdr xl)) #f k)])
             (codegen (car xl) l f s g k port)
             (loop (cdr xl)))))
       (when (and k (null? exps)) (write-char #\] port) (write-serialized-arg k port))]
      [if (cexp texp eexp)
       (codegen cexp l f s g #f port)
       (write-char #\? port)
       (write-char #\{ port)
       (codegen texp l f s g k port)
       (write-char #\} port)
       (cond [k ; tail call: 'then' arm exits, so br around is not needed
              (codegen eexp l f s g k port)]
             [(equal? eexp '(begin)) ; non-tail with void 'else' arm
              ] ; no code needed -- ac retains #f from failed test
             [else ; non-tail with 'else' expression; needs br 
              (write-char #\{ port)
              (codegen eexp l f s g k port)
              (write-char #\} port)])]              
      [lambda (idsi exp)
       (let* ([ids (flatten-idslist idsi)]
              [free (set-minus (find-free exp ids) g)]
              [sets (find-sets exp ids)])
         (do ([free (reverse free) (cdr free)] [l l (cons #f l)]) [(null? free)]
           ; note: called with empty set! var list
           ; to make sure no dereferences are generated
           (codegen (list 'ref (car free)) l f '() g #f port)
           (write-char #\, port))
         (write-char #\& port)
         (write-serialized-arg (length free) port)
         (write-char #\{ port)
         (cond [(list? idsi)
                (write-char #\% port)
                (write-serialized-arg (length idsi) port)]
               [else
                (write-char #\% port) (write-char #\! port)
                (write-serialized-arg (idslist-req-count idsi) port)])
         (do ([ids ids (cdr ids)] [n 0 (fx+ n 1)]) [(null? ids)]
           (when (set-member? (car ids) sets)
             (write-char #\# port)
             (write-serialized-arg n port)))
         (codegen exp ids free 
           (set-union sets (set-intersect s free))
           g (length ids) port)
         (write-char #\} port))
       (when k (write-char #\] port) (write-serialized-arg k port))]
      [lambda* clauses
       (do ([clauses (reverse clauses) (cdr clauses)] [l l (cons #f l)]) 
            [(null? clauses)]
         (codegen (cadr (car clauses)) l f s g #f port)
         (write-char #\% port) (write-char #\x port)
         (write-char #\, port))
       (write-char #\& port)
       (write-serialized-arg (length clauses) port)
       (write-char #\{ port)
       (do ([clauses clauses (cdr clauses)] [i 0 (fx+ i 1)]) 
            [(null? clauses)]
         (let* ([arity (caar clauses)] [cnt (car arity)] [rest? (cadr arity)])
           (write-char #\| port)
           (if rest? (write-char #\! port))
           (write-serialized-arg cnt port)
           (write-serialized-arg i port)))
       (write-char #\% port) (write-char #\% port)
       (write-char #\} port)
       (when k (write-char #\] port) (write-serialized-arg k port))]
      [letcc (kid exp)
       (let* ([ids (list kid)] [sets (find-sets exp ids)]
              [news (set-union (set-minus s ids) sets)]) 
         (cond [k ; tail position with k locals on stack to be disposed of
                (write-char #\k port) (write-serialized-arg k port)
                (write-char #\, port)
                (when (set-member? kid sets) 
                  (write-char #\# port) (write-char #\0 port))
                ; stack map here: kid on top
                (codegen exp (cons kid l) f news g (fx+ k 1) port)]
               [else ; non-tail position 
                (write-char #\$ port) (write-char #\{ port)
                (write-char #\k port) (write-char #\0 port)
                (write-char #\, port)
                (when (set-member? kid sets) 
                  (write-char #\# port) (write-char #\0 port))
                ; stack map here: kid on top, two-slot frame under it
                (codegen exp (cons kid (cons #f (cons #f l))) f news g #f port)
                (write-char #\_ port) (write-serialized-arg 3 port)
                (write-char #\} port)]))]
      [withcc (kexp exp)
       (cond [(memq (car exp) '(quote ref lambda)) ; exp is a constant, return it
              (codegen exp l f s g #f port) 
              (write-char #\, port) ; stack map after: k on top
              (codegen kexp (cons #f l) f s g #f port)
              (write-char #\w port) (write-char #\! port)]
             [else ; exp is not a constant, thunk it and call it from k
              (codegen (list 'lambda '() exp) l f s g #f port) 
              (write-char #\, port) ; stack map after: k on top
              (codegen kexp (cons #f l) f s g #f port)
              (write-char #\w port)])]
      [integrable (ig . args)
       (let ([igty (integrable-type ig)] [igc0 (integrable-code ig 0)])
         (case igty
            [(#\0 #\1 #\2 #\3) ; 1st arg in a, others on stack
             (do ([args (reverse args) (cdr args)] [l l (cons #f l)]) 
               [(null? args)]
               (codegen (car args) l f s g #f port)
               (unless (null? (cdr args)) (write-char #\, port)))
             (write-string igc0 port)]
            [(#\p) ; (length args) >= 0
             (if (null? args)
                 (let ([igc1 (integrable-code ig 1)])
                   (write-string igc1 port))
                 (let ([opc (fx- (length args) 1)])
                   (do ([args (reverse args) (cdr args)] [l l (cons #f l)]) 
                     [(null? args)]
                     (codegen (car args) l f s g #f port)
                     (unless (null? (cdr args)) (write-char #\, port)))
                   (do ([i 0 (fx+ i 1)]) [(fx>=? i opc)]
                     (write-string igc0 port))))]
            [(#\m) ; (length args) >= 1
             (if (null? (cdr args))
                 (let ([igc1 (integrable-code ig 1)])
                   (codegen (car args) l f s g #f port)
                   (write-string igc1 port))
                 (let ([opc (fx- (length args) 1)])
                   (do ([args (reverse args) (cdr args)] [l l (cons #f l)]) 
                     [(null? args)]
                     (codegen (car args) l f s g #f port)
                     (unless (null? (cdr args)) (write-char #\, port)))
                   (do ([i 0 (fx+ i 1)]) [(fx>=? i opc)]
                     (write-string igc0 port))))]
            [(#\c) ; (length args) >= 2
             (let ([opc (fx- (length args) 1)] [args (reverse args)])
               (codegen (car args) l f s g #f port)
               (write-char #\, port)
               (do ([args (cdr args) (cdr args)] [l (cons #f l) (cons #f (cons #f l))]) 
                 [(null? args)]
                 (codegen (car args) l f s g #f port)
                 (unless (null? (cdr args)) (write-char #\, port) (write-char #\, port)))
               (do ([i 0 (fx+ i 1)]) [(fx>=? i opc)]
                 (unless (fxzero? i) (write-char #\; port)) 
                 (write-string igc0 port)))]
            [(#\x) ; (length args) >= 1
             (let ([opc (fx- (length args) 1)])
               (do ([args (reverse args) (cdr args)] [l l (cons #f l)]) 
                 [(null? args)]
                 (codegen (car args) l f s g #f port)
                 (unless (null? (cdr args)) (write-char #\, port)))
               (do ([i 0 (fx+ i 1)]) [(fx>=? i opc)]
                 (write-string igc0 port)))]
            [(#\u) ; 0 <= (length args) <= 1
             (if (null? args)
                 (write-string (integrable-code ig 1) port)
                 (codegen (car args) l f s g #f port))
             (write-string igc0 port)]
            [(#\b) ; 1 <= (length args) <= 2
             (if (null? (cdr args))
                 (write-string (integrable-code ig 1) port)
                 (codegen (cadr args) l f s g #f port))
             (write-char #\, port)
             (codegen (car args) (cons #f l) f s g #f port)
             (write-string igc0 port)]
            [(#\t) ; 2 <= (length args) <= 3
             (if (null? (cddr args))
                 (write-string (integrable-code ig 1) port)
                 (codegen (caddr args) l f s g #f port))
             (write-char #\, port)
             (codegen (cadr args) (cons #f l) f s g #f port)
             (write-char #\, port)
             (codegen (car args) (cons #f (cons #f l)) f s g #f port)
             (write-string igc0 port)]
            [(#\#) ; (length args) >= 0
             (do ([args (reverse args) (cdr args)] [l l (cons #f l)]) 
               [(null? args)]
               (codegen (car args) l f s g #f port)
               (write-char #\, port))
             (write-string igc0 port)
             (write-serialized-arg (length args) port)]
            [else (c-error "unsupported integrable type" igty)]))
       (when k (write-char #\] port) (write-serialized-arg k port))]
      [call (exp . args)
       (cond [(and (eq? (car exp) 'lambda) (list? (cadr exp))
                   (fx=? (length args) (length (cadr exp))))
              ; let-like call; compile as special lambda + call combo
              (do ([args (reverse args) (cdr args)] [l l (cons #f l)]) 
                [(null? args)]
                (codegen (car args) l f s g #f port)
                (write-char #\, port))
              (let* ([ids (cadr exp)] [exp (caddr exp)]
                     [sets (find-sets exp ids)]
                     [news (set-union (set-minus s ids) sets)]
                     [newl (append ids l)]) ; with real names
                (do ([ids ids (cdr ids)] [n 0 (fx+ n 1)]) [(null? ids)]
                  (when (set-member? (car ids) sets)
                    (write-char #\# port)
                    (write-serialized-arg n port)))
                (if k 
                    (codegen exp newl f news g (fx+ k (length args)) port)
                    (begin 
                      (codegen exp newl f news g #f port)
                      (write-char #\_ port)
                      (write-serialized-arg (length args) port))))]
             [k ; tail call with k elements under arguments
              (do ([args (reverse args) (cdr args)] [l l (cons #f l)]) 
                [(null? args) (codegen exp l f s g #f port)]
                (codegen (car args) l f s g #f port)
                (write-char #\, port))
              (write-char #\[ port)
              (write-serialized-arg k port)
              (write-serialized-arg (length args) port)]
             [else ; non-tail call; 'save' puts 2 extra elements on the stack!
              (write-char #\$ port) (write-char #\{ port)
              (do ([args (reverse args) (cdr args)] [l (cons #f (cons #f l)) (cons #f l)]) 
                [(null? args) (codegen exp l f s g #f port)]
                (codegen (car args) l f s g #f port)
                (write-char #\, port))
              (write-char #\[ port)
              (write-serialized-arg 0 port)
              (write-serialized-arg (length args) port)
              (write-char #\} port)])]
      [asm (cstr)
       (write-string cstr port)
       (when k (write-char #\] port) (write-serialized-arg k port))]
      [once (gid exp)
       (codegen `(if (integrable ,(lookup-integrable 'eq?) (gref ,gid) (quote #t)) 
                     (begin)
                     (begin (gset! ,gid (quote #t)) ,exp))
          l f s g k port)]
      [(define define-syntax define-library import) tail
       (c-error "misplaced definition form" x)]
      [else (c-error "unexpected <core> form" x)])))

(define (compile-to-string x)
  (let ([p (open-output-string)])
    (codegen x '() '() '() (find-free x '()) #f p)                   
    (get-output-string p)))

(define (compile-to-thunk-code x)
  (let ([p (open-output-string)])
    (codegen x '() '() '() (find-free x '()) 0 p)                   
    (get-output-string p)))


;--------------------------------------------------------------------------------------------------
; Path and file name resolution
;--------------------------------------------------------------------------------------------------

(define (path-strip-directory filename)
  (let loop ([l (reverse (string->list filename))] [r '()])
    (cond [(null? l) (list->string r)]
          [(memv (car l) '(#\\ #\/ #\:)) (list->string r)]
          [else (loop (cdr l) (cons (car l) r))])))

(define (path-directory filename)
  (let loop ([l (reverse (string->list filename))])
    (cond [(null? l) ""]
          [(memv (car l) '(#\\ #\/ #\:)) (list->string (reverse l))]
          [else (loop (cdr l))])))

(define (path-strip-extension filename) ; improved
  (let loop ([l (reverse (string->list filename))])
    (cond [(null? l) filename]
          [(eqv? (car l) #\.) (list->string (reverse (cdr l)))]
          [(memv (car l) '(#\\ #\/ #\:)) filename]
          [else (loop (cdr l))])))

(define (base-path-separator basepath)
  (let ([l (reverse (string->list basepath))])
    (cond [(null? l) #f]
          [(memv (car l) '(#\\ #\/)) (car l)]
          [else #f])))

(define (path-relative? filename)
  (let ([l (string->list filename)])
    (cond [(null? l) #f]
          [(memv (car l) '(#\\ #\/)) #f]
          [(and (> (length l) 3) (char-alphabetic? (car l)) (eqv? (cadr l) #\:) (eqv? (caddr l) #\\)) #f]
          [else #t])))

(define (file-resolve-relative-to-base-path filename basepath)
  (if (and (path-relative? filename) (base-path-separator basepath))
      (string-append basepath filename) ; leading . and .. to be resolved by OS
      filename))

; hacks for relative file name resolution

(define *current-file-stack* '())

(define (current-file-stack) *current-file-stack*)
(define (set-current-file-stack! s) (set! *current-file-stack* s))

(define (current-file) ;=> filename of #f
  (and (pair? *current-file-stack*) (car *current-file-stack*)))

(define (push-current-file! filename)
 (when (member filename *current-file-stack* string=?)
   (x-error "circularity in nested file chain" filename))
 (set! *current-file-stack* (cons filename *current-file-stack*)))

(define (pop-current-file!)
 (unless (null? *current-file-stack*)
   (set! *current-file-stack* (cdr *current-file-stack*))))

(define (with-current-file filename thunk)
  (dynamic-wind
    (lambda () (push-current-file! filename))
    thunk
    (lambda () (pop-current-file!))))

(define (file-resolve-relative-to-current filename) ; => resolved or original filename 
  (if (path-relative? filename)
      (let ([cf (current-file)])
        (if cf (file-resolve-relative-to-base-path filename (path-directory cf)) filename))
      filename))

(define (call-with-current-input-file filename portproc)
  (let* ([filepath (and (string? filename) (file-resolve-relative-to-current filename))]
         [fileok? (and (string? filepath) (file-exists? filepath))])
    (unless fileok? (error "cannot open file" filename filepath)) 
    (with-current-file filepath
      (lambda () (call-with-input-file filepath portproc)))))


;--------------------------------------------------------------------------------------------------
; Library names and library file lookup
;--------------------------------------------------------------------------------------------------

(define (lnpart? x) (or (id? x) (exact-integer? x)))
(define (listname? x) (and (list1+? x) (andmap lnpart? x))) 

(define (mangle-symbol->string sym)
  (define safe '(#\! #\$ #\- #\_ #\=))
  (let loop ([lst (string->list (symbol->string sym))] [text '()])
    (cond [(null? lst) 
            (list->string (reverse text))]
          [(or (char-lower-case? (car lst)) (char-numeric? (car lst)))
            (loop (cdr lst) (cons (car lst) text))]
          [(memv (car lst) safe)
            (loop (cdr lst) (cons (car lst) text))]
          [else ; use % encoding for everything else
            (let* ([s (number->string (char->integer (car lst)) 16)]
                  [s (if (< (string-length s) 2) (string-append "0" s) s)]
                  [l (cons #\% (string->list (string-downcase s)))])
              (loop (cdr lst) (append (reverse l) text)))])))

(define (listname->symbol lib)
  (define postfix "")
  (define prefix "lib:/")
  (define symbol-prefix "/")
  (define number-prefix "/")
  (unless (list? lib) (x-error "invalid library name" lib))
  (let loop ([lst lib] [parts (list prefix)])
    (if (null? lst)
        (string->symbol (apply string-append (reverse (cons postfix parts))))
        (cond [(symbol? (car lst))
               (loop (cdr lst) (cons (mangle-symbol->string (car lst)) (cons symbol-prefix parts)))]
              [(exact-integer? (car lst))
               (loop (cdr lst) (cons (number->string (car lst)) (cons number-prefix parts)))]
              [else (x-error "invalid library name" lib)]))))

(define (listname-segment->string s)
  (cond [(symbol? s) (mangle-symbol->string s)]
        [(exact-integer? s) (number->string s)]
        [else (c-error "invalid library name name element" s)]))

(define (listname->path listname basepath ext)
  (define sep 
    (let ([sc (base-path-separator basepath)])
      (if sc (string sc) (c-error "library path does not end in separator" basepath))))
  (let loop ([l listname] [r '()])
    (if (pair? l)
        (loop (cdr l) 
              (if (null? r) 
                  (cons (listname-segment->string (car l)) r) 
                  (cons (listname-segment->string (car l)) (cons sep r))))
        (file-resolve-relative-to-base-path (string-append* (reverse (cons ext r))) basepath))))

; hacks for locating library files

(define *library-path-list* '("./")) ; will do for now; FIXME: get access to real separator!

(define (append-library-path! path)
  (if (base-path-separator path)
      (set! *library-path-list* (append *library-path-list* (list path)))
      (c-error "library path should end in directory separator" path))) 

(define (prepend-library-path! path)
  (if (base-path-separator path)
      (set! *library-path-list* (append (list path) *library-path-list*))
      (c-error "library path should end in directory separator" path))) 

(define (find-library-path listname) ;=> name of existing .sld file or #f
  (let loop ([l *library-path-list*])
    (and (pair? l)
         (let ([p (listname->path listname (car l) ".sld")]) 
           (if (and p (file-exists? p)) p (loop (cdr l)))))))

(define (read-port-sexps port)
  (let loop ([sexps '()])
    (let ([s (read-code-sexp port)])
      (if (eof-object? s)
          (reverse! sexps)
          (loop (cons s sexps))))))

(define (read-file-sexps filepath ci?)
  (call-with-input-file filepath
    (lambda (port) 
      (when ci? (set-port-fold-case! port #t))
      (read-port-sexps port))))

(define (library-available? lib env) ;=> #f | filepath (external) | <library> (loaded)
  (cond [(not (listname? lib)) #f]
        [(find-library-in-env lib env)] ; defined below
        [else (find-library-path lib)]))

; name prefixes

(define (fully-qualified-library-prefixed-name lib id)
  (symbol-append (if (symbol? lib) lib (listname->symbol lib)) '? id))

; used as autoload action supplying default value for list names
(define (fetch-library listname sld-env) ;=> <library> | #f
  ; note: any part of the actual fetch/process may fail. In that case,
  ; fetch-library escapes and environment will still lack binding for
  ; listname -- so the library can be fixed and reloaded afterwards 
  (define (fetch filepath)
    (unless *quiet*
      (let ([p (current-error-port)])
        (display "; fetching " p) (write listname p) (display " library from " p) 
        (display filepath p) (newline p)))
    (with-current-file filepath
      (lambda () 
        (let ([sexps (read-file-sexps filepath #f)])
          (if (sexp-match? '((define-library * * ...)) sexps)
              ; invoke xform-define-library in 'top' context (for lib:// globals)
              (let ([core (xform-define-library (caar sexps) (cdar sexps) sld-env #t)])
                (if (and (sexp-match? '(define-library * *) core)
                         (equal? (cadr core) listname) (val-library? (caddr core)))
                    (caddr core) ;=> <library>
                    (x-error "library autoloader: internal transformer error" 
                      listname filepath sexps core)))
              (x-error "library autoloader: unexpected forms in .sld file" 
                listname filepath sexps))))))
  (define filepath (find-library-path listname))
  ; return #f if .sld file is not found; otherwise expect it to be in shape
  (and filepath (fetch filepath))) 


;--------------------------------------------------------------------------------------------------
; Expand-time name registries
;--------------------------------------------------------------------------------------------------

; name registries are htables (vectors with one extra slot) of alists ((sym . <location>) ...)
; last slot is used for list names (library names), the rest for regular symbolic names

(define (name-registry? x) (vector? x)) ; should be distinguishable from locs, which are boxes

(define (make-name-registry size) ; size is approx. count of buckets
  (define primes ; some nice-looking primes in 1 - 1000 range
   '(1 11 31 41 61 71 101 131 151 181 191 211 241 251 271 281 311 331 401 421 431 
     461 491 521 541 571 601 631 641 661 691 701 751 761 811 821 881 911 941 971))
  (define prime (cond [(member size primes <=) => car] [else 991]))
  (make-vector (+ prime 1) '())) ; last bucket used for listnames

(define (eal->name-registry eal n) 
  (if (= n 1) 
      (vector eal '())
      (let ([nr (make-name-registry n)])
        (for-each (lambda (p) (name-install! nr (car p) (cdr p))) eal)
        nr)))
 
(define (eal-name-registry-import! ir ial)
  (if (= (vector-length ir) 2)
      (vector-set! ir 0 (adjoin-eals (vector-ref ir 0) ial)) ; may end in x-error on conflict
      (let loop ([ial ial])
        (unless (null? ial)
          (case (name-install! ir (caar ial) (cdar ial))
            [(modified) (x-error "import conflict on importing binding" (caar ial) (cdar ial))])
          (loop (cdr ial))))))

(define (name-lookup nr name mkdefval) ;=> loc | #f
  (let* ([n-1 (- (vector-length nr) 1)] [i (if (pair? name) n-1 (immediate-hash name n-1))] 
         [al (vector-ref nr i)] [p (if (pair? name) (assoc name al) (assq name al))])
    (cond [p (cdr p)]
          [mkdefval ; got callback for missing bindings? use it:
           (let ([val (mkdefval name)]) ; check if it didn't fail:
             (cond [(not val) #f] ; mkdefval rejected the idea
                   [(location? val) val] ; found good location elsewhere
                   [else (let ([al (vector-ref nr i)] [p (if (pair? name) (assoc name al) (assq name al))])
                           ; note: have to refetch both cause mkdefval call could've change them!
                           (if p (x-error "recursive library dependence on" name)
                               (let ([loc (make-location val)]) ; ok, put it in:
                                 (vector-set! nr i (cons (cons name loc) al))
                                 loc)))]))]
          [else #f])))

(define (name-install! nr name loc) ;=> same|modified|added
  (let* ([n-1 (- (vector-length nr) 1)] [i (if (pair? name) n-1 (immediate-hash name n-1))]
         [al (vector-ref nr i)] [p (if (pair? name) (assoc name al) (assq name al))])
    (cond [(and p (eq? (cdr p) loc)) 'same] ; nothing changed
          [p (set-cdr! p loc) 'modified]
          [else (vector-set! nr i (cons (cons name loc) al)) 'added])))

(define (name-remove! nr name)
  (let* ([n-1 (- (vector-length nr) 1)] [i (if (pair? name) n-1 (immediate-hash name n-1))])
    (vector-set! nr i (remove! name (vector-ref nr i) (lambda (name p) (equal? (car p) name))))))

; public registry for all non-hidden skint names
(define *root-name-registry* (make-name-registry 300))

; nonpublic registry for all hidden skint names (used by built-in macros)
(define *hidden-name-registry* (make-name-registry 1)) ; 1 to share bindings w/(skint hidden)

(define (builtin-sr-environment id at)
  (cond [(new-id? id) (new-id-lookup id at)]
        [(eq? at 'peek) ; for free-id=?
         (or (name-lookup *hidden-name-registry* id #f)
             (name-lookup *root-name-registry* id #f)
             *root-name-registry*)] 
        [else 
         (name-lookup *hidden-name-registry* id 
           (lambda (n) ; create new bindings in rnr:
             (name-lookup *root-name-registry* id 
               (lambda (n) (list 'ref n)))))]))

; register integrable procedures
(let loop ([i 0])
  (let ([li (lookup-integrable i)]) ;=> #f, #<void>, or integrable (li == i)
    (when li ; in range: void or integrable
      (when (integrable? i)
        (let ([name (integrable-global i)])
          (when (symbol? name) (name-lookup *root-name-registry* name (lambda (name) i)))))
      (loop (+ i 1))))) 

; register initial define-syntax transformers coming from s.scm and this file
(let loop ([l (initial-transformers)])
  (unless (null? l)
    (let* ([p (car l)] [l (cdr l)] [k (car p)] [v (cdr p)])
      (cond [(or (symbol? v) (integrable? v))
             ; integrables and builtins like begin and ... (sic!)
             (name-lookup *root-name-registry* k (lambda (n) v)) 
             (loop l)]
            [(and (pair? v) (eq? (car v) 'syntax-rules))
             (let ([sr-env builtin-sr-environment])
               (define sr-v
                 (if (id? (cadr v))
                     (syntax-rules* sr-env (cadr v) (caddr v) (cdddr v))
                     (syntax-rules* sr-env #f (cadr v) (cddr v))))
               (name-lookup *root-name-registry* k (lambda (name) sr-v))
               (loop l))]))))

; register handcoded transformers
(name-lookup *root-name-registry* 'include     (lambda (n) (make-include-transformer #f)))
(name-lookup *root-name-registry* 'include-ci  (lambda (n) (make-include-transformer #t)))
(name-lookup *root-name-registry* 'cond-expand (lambda (n) (make-cond-expand-transformer)))

; register standard libraries as well as (skint) library for interactive environment
; ... while doing that, bind missing standard names as refs to constant globals
(for-each
  (lambda (r)
    (define (key->listname k)
      (case k
        [(w) '(scheme write)]   [(t) '(scheme time)]  [(p) '(scheme repl)]
        [(r) '(scheme read)]    [(v) '(scheme r5rs)]  [(u) '(scheme r5rs-null)] 
        [(d) '(scheme load)]    [(z) '(scheme lazy)]  [(s) '(scheme process-context)]
        [(i) '(scheme inexact)] [(f) '(scheme file)]  [(e) '(scheme eval)]
        [(o) '(scheme complex)] [(h) '(scheme char)]  [(l) '(scheme case-lambda)]
        [(a) '(scheme cxr)]     [(b) '(scheme base)]  [(x) '(scheme box)]
        [else (if (exact-integer? k) (list 'srfi k) (list k))]))
    (define (get-library! listname) ;=> <library> 
      (location-val 
        (name-lookup *root-name-registry* listname 
          (lambda (ln) (make-library '(begin) '()))))) 
    (define (put-loc! library k loc)
      (let* ([eal (library-exports library)] [p (assq k eal)])
        (cond [p (set-cdr! p loc)] 
              [else (library-set-exports! library (cons (cons k loc) eal))])))
    (define (get-loc name)
      (name-lookup *root-name-registry* name (lambda (name) (list 'const name))))
    (let loop ([name (car r)] [keys (cdr r)])
      (cond [(null? keys) (put-loc! (get-library! '(skint)) name (get-loc name))]
            [(not (pair? keys)) (put-loc! (get-library! `(skint ,keys)) name (get-loc name))] 
            [else (put-loc! (get-library! (key->listname (car keys))) name (get-loc name))
                  (loop name (cdr keys))])))
  '((* v b) (+ v b) (- v b) (... v u b) (/ v b) (< v b) (<= v b) (= v b) (=> v u b) (> v b) (>= v b)
    (_ b) (abs v b) (and v u b) (append v b) (apply v b) (assoc v b) (assq v b) (assv v b) (begin v u b)
    (binary-port? b) (boolean=? b) (boolean? v b) (bytevector b) (bytevector-append b)
    (bytevector-copy b) (bytevector-copy! b) (bytevector-length b) (bytevector-u8-ref b)
    (bytevector-u8-set! b) (bytevector? b) (caar v b) (cadr v b) (call-with-current-continuation v b)
    (call-with-port b) (call-with-values v b) (call/cc b) (car v b) (case v u b) (cdar v b) (cddr v b)
    (cdr v b) (ceiling v b) (char->integer v b) (char-ready? v b) (char<=? v b) (char<? v b)
    (char=? v b) (char>=? v b) (char>? v b) (char? b) (close-input-port v b) (close-output-port v b)
    (close-port b) (complex? v b) (cond v u b) (cond-expand b) (cons v b) (current-error-port b)
    (current-input-port v b) (current-output-port v b) (define v u b) (define-record-type b)
    (define-syntax v u b) (define-values b) (denominator v b) (do v u b) (dynamic-wind v b) (else v u b)
    (eof-object b) (eof-object? v b) (eq? v b) (equal? v b) (eqv? v b) (error b) (library b)
    (error-object-irritants b) (error-object-message b) (error-object? b) (even? v b) (exact b)
    (exact-integer-sqrt b) (exact-integer? b) (exact? v b) (expt v b) (features b) (file-error? b)
    (floor v b) (floor-quotient b) (floor-remainder b) (floor/ b) (flush-output-port b) (for-each v b)
    (gcd v b) (get-output-bytevector b) (get-output-string b) (guard b) (if v u b) (include b)
    (include-ci b) (inexact b) (inexact? v b) (input-port-open? b) (input-port? v b) (integer->char v b)
    (integer? v b) (lambda v u b) (lcm v b) (length v b) (let v u b) (let* v u b) (let*-values b)
    (let-syntax v u b) (let-values b) (letrec v u b) (letrec* b) (letrec-syntax v u b) (list v b)
    (list->string v b) (list->vector v b) (list-copy b) (list-ref v b) (list-set! b) (list-tail v b)
    (list? v b) (make-bytevector b) (make-list b) (make-parameter b) (make-string v b) (make-vector v b)
    (map v b) (max v b) (member v b) (memq v b) (memv v b) (min v b) (modulo v b) (negative? v b)
    (newline v b) (not v b) (null? v b) (number->string v b) (number? v b) (numerator v b) (odd? v b)
    (open-input-bytevector b) (open-input-string b) (open-output-bytevector b) (open-output-string b)
    (or v u b) (output-port-open? b) (output-port? v b) (pair? v b) (parameterize b) (peek-char v b)
    (peek-u8 b) (port? b) (positive? v b) (procedure? v b) (quasiquote v u b) (quote v u b)
    (quotient v b) (raise b) (raise-continuable b) (rational? v b) (rationalize v b) (read-bytevector b)
    (read-bytevector! b) (read-char v b) (read-error? b) (read-line b) (read-string b) (read-u8 b)
    (real? v b) (remainder v b) (reverse v b) (round v b) (set! v b) (set-car! v b) (set-cdr! v b)
    (square b) (string v b) (string->list v b) (string->number v b) (string->symbol v b)
    (string->utf8 b) (string->vector b) (string-append v b) (string-copy v b) (string-copy! b)
    (string-fill! v b) (string-for-each b) (string-length v b) (string-map b) (string-ref v b)
    (string-set! v b) (string<=? v b) (string<? v b) (string=? v b) (string>=? v b) (string>? v b)
    (string? v b) (substring v b) (symbol->string v b) (symbol=? b) (symbol? v b) (syntax-error b)
    (syntax-rules v u b) (textual-port? b) (truncate v b) (truncate-quotient b) (truncate-remainder b)
    (truncate/ b) (u8-ready? b) (unless b) (unquote v u b) (unquote-splicing v u b) (utf8->string b)
    (values v b) (vector v b) (vector->list v b) (vector->string b) (vector-append b) (vector-copy b)
    (vector-copy! b) (vector-fill! v b) (vector-for-each b) (vector-length v b) (vector-map b)
    (vector-ref v b) (vector-set! v b) (vector? v b) (when b) (with-exception-handler b)
    (write-bytevector b) (write-char v b) (write-string b) (write-u8 b) (zero? v b) (case-lambda l)
    (char-alphabetic? v h) (char-ci<=? v h) (char-ci<? v h) (char-ci=? v h) (char-ci>=? v h)
    (char-ci>? v h) (char-downcase v h) (char-foldcase h) (char-lower-case? v h) (char-numeric? v h)
    (char-upcase v h) (char-upper-case? v h) (char-whitespace? v h) (digit-value h) (string-ci<=? v h)
    (string-ci<? v h) (string-ci=? v h) (string-ci>=? v h) (string-ci>? v h) (string-downcase h)
    (string-foldcase h) (string-upcase h) (angle v o) (imag-part v o) (magnitude v o) (make-polar v o)
    (make-rectangular v o) (real-part v o) (caaar v a) (caadr v a) (cadar v a) (caddr v a) (cdaar v a)
    (cdadr v a) (cddar v a) (cdddr v a) (caaaar v a) (caaadr v a) (caadar v a) (caaddr v a) (cadaar v a)
    (cadadr v a) (caddar v a) (cadddr v a) (cdaaar v a) (cdaadr v a) (cdadar v a) (cdaddr v a)
    (cddaar v a) (cddadr v a) (cdddar v a) (cddddr v a) (environment e) (eval v e)
    (call-with-input-file v f) (call-with-output-file v f) (delete-file f) (file-exists? f)
    (open-binary-input-file f) (open-binary-output-file f) (open-input-file v f) (open-output-file v f)
    (with-input-from-file v f) (with-output-to-file v f) (acos v z i) (asin v z i) (atan v z i)
    (cos v z i) (exp v z i) (finite? z i) (infinite? i) (log v i) (nan? i) (sin v i) (sqrt v i)
    (tan v i) (delay v u z) (delay-force z) (force v z) (make-promise z) (promise? z) (load v d)
    (command-line s) (emergency-exit s) (exit s) (get-environment-variable s)
    (get-environment-variables s) (display w v) (exact->inexact v) (inexact->exact v)
    (interaction-environment p v) (null-environment v) (read r v) (scheme-report-environment v)
    (write w v) (current-jiffy t) (current-second t) (jiffies-per-second t) (write-shared w)
    (write-simple w)
    ; these are special forms in skint!
    (define-library) (import) (export) (program) 
    ; selected extracts from r7rs-large and srfis
    (box? x 111) (box x 111) (unbox x 111) (set-box! x 111) (format 28 48) 
    (fprintf) (format-pretty-print) (format-fixed-print) (format-fresh-line) (format-help-string)
    ; skint extras go into (skint); the rest goes to (skint hidden)
    (set&) (lambda*) (body) (letcc) (withcc) (syntax-lambda) (syntax-length)
    (record?) (make-record) (record-length) (record-ref) (record-set!) 
    (fixnum?) (fxpositive?) (fxnegative?) (fxeven?) (fxodd?) (fxzero?) (fx+) (fx*) (fx-) (fx/) 
    (fxquotient) (fxremainder) (fxmodquo) (fxmodulo) (fxeucquo) (fxeucrem) (fxneg)
    (fxabs) (fx<?) (fx<=?) (fx>?) (fx>=?) (fx=?) (fx!=?) (fxmin) (fxmax) (fxneg) (fxabs) (fxgcd) 
    (fxexpt) (fxsqrt) (fxnot) (fxand) (fxior) (fxxor) (fxsll) (fxsrl) (fixnum->flonum) (fixnum->string)
    (string->fixnum) (flonum?) (flzero?) (flpositive?) (flnegative?) (flinteger?) (flnan?)
    (flinfinite?) (flfinite?) (fleven?) (flodd?) (fl+) (fl*) (fl-) (fl/) (flneg) (flabs) (flgcd) 
    (flexpt) (flsqrt) (flfloor) (flceiling) (fltruncate) (flround) (flexp) (fllog) (flsin) (flcos) 
    (fltan) (flasin) (flacos) (flatan) (fl<?) (fl<=?) (fl>?) (fl>=?) (fl=?) (fl!=?) (flmin) 
    (flmax) (flremainder) (flmodulo) (flquotient) (flmodquo) (flonum->fixnum) (flonum->string) 
    (string->flonum) (list-cat) (last-pair) (list-head) (meme) (asse) (reverse!) (circular?) (cons*)
    (list*) (char-cmp) (char-ci-cmp) (string-cat) (string-position) (string-cmp) (string-ci-cmp) 
    (vector-cat) (bytevector=?) (bytevector->list) (list->bytevector) (subbytevector) 
    (standard-input-port) (standard-output-port) (standard-error-port) (tty-port?)
    (port-fold-case?) (set-port-fold-case!) (rename-file) (current-directory) (directory-separator)
    (void) (void?)
    ; (repl hidden) library entries below the auto-adder need to be added explicitly 
    (*user-name-registry* . hidden) (make-readonly-environment . hidden) 
    (make-controlled-environment . hidden) (make-sld-environment . hidden) 
    (make-repl-environment . hidden) (find-library-in-env . hidden) (root-environment . hidden)
    (repl-environment . hidden) (empty-environment . hidden) (make-historic-report-environment . hidden)
    (r5rs-environment . hidden) (r5rs-null-environment . hidden) (*verbose*  . hidden) (*quiet*  . hidden)
    (compile-and-run-core-expr . hidden) (evaluate-top-form . hidden) (run-script . hidden)
    (run-program . hidden) (repl-evaluate-top-form . hidden) (repl-read . hidden) 
    (repl-exec-command . hidden) (repl-from-port . hidden) (run-benchmark . hidden) (repl . hidden) 
    ))

; clean up root environment by moving all symbolic bindings not in (skint) library
; to hidden name registry (so built-in macros can still work properly)
(let* ([rr *root-name-registry*] [l (name-lookup rr '(skint) #f)] 
       [eal (library-exports (location-val l))]
       [n-1 (- (vector-length rr) 1)]) ; sans listnames
  (do ([i 0 (+ i 1)]) [(= i n-1)]
    (let loop ([prev #f] [lst (vector-ref rr i)])
      (cond [(null? lst)]
            [(assq (caar lst) eal) (loop lst (cdr lst))]
            [else (if prev (set-cdr! prev (cdr lst)) (vector-set! rr i (cdr lst)))
                  (name-install! *hidden-name-registry* (caar lst) (cdar lst))
                  (loop prev (cdr lst))]))))

; make hidden bindings available via (skint hidden) library
; note: definitions below this expression are not yet in (global-store), so one
; has to add them explicitly via (foo . hidden) mechanism above
(let* ([mklib (lambda (ln) (make-library '(begin) '()))]
       [loc (name-lookup *root-name-registry* '(skint hidden) mklib)]
       [lib (location-val loc)] [eal (vector-ref *hidden-name-registry* 0)]
       [combeal (adjoin-eals eal (library-exports lib))]
       [skintloc (name-lookup *root-name-registry* '(skint) #f)]
       [skintlib (and (location? skintloc) (location-val skintloc))]
       [skinteal (and (val-library? skintlib) (library-exports skintlib))])
  (define (add-hidden-ref! p)
    (let* ([id (car p)] [in-sk? (and skinteal (assq id skinteal))])
      (unless (or in-sk? (assq id combeal))
        (set! combeal (cons (cons id (make-location (list 'const id))) combeal)))))
  (let* ([gsv (global-store)] [n (vector-length gsv)])
    (do ([i 0 (+ i 1)]) [(>= i n)] (for-each add-hidden-ref! (vector-ref gsv i)))) 
  (library-set-exports! lib combeal))

; private registry for names introduced in repl 
(define *user-name-registry* (make-name-registry 200)) 


;--------------------------------------------------------------------------------------------------
; Environments
;--------------------------------------------------------------------------------------------------

; make read-only environment from a registry
(define (make-readonly-environment rr)
  (lambda (name at)
    (cond [(new-id? name) ; nonsymbolic ids can't be (re)bound here
           (case at [(ref set! peek) (new-id-lookup name at)] [else #f])]
          [(eq? at 'peek) (or (name-lookup rr name #f) rr)] ; for free-id=? 
          [(eq? at 'ref) (name-lookup rr name #f)]  ; no allocation callback
          [else #f])))

; makes controlled environments for libraries and programs using import al, global name generator, 
; and env allowing fall-through for list names (so libraries can still be fetched by list name)
(define (make-controlled-environment ial global use-env)
  (define ir (eal->name-registry ial 100)) ; handmade import registry from ial
  (define lr (make-name-registry 100)) ; local registry for new names
  (lambda (name at)
    (cond [(new-id? name) ; nonsymbolic ids can't be (re)bound here
           (case at [(ref set! peek) (new-id-lookup name at)] [else #f])]
          [(eq? at 'ref) ; for reference only: try not to alloc
           (name-lookup lr name ; return if in local registry
             (lambda (n) ; ok, not in lr: check ir
               (or (name-lookup ir name #f) ; if in ir, return it as-is
                   (if (symbol? name)
                       (list 'ref (global name)) ; alloc in repl store
                       (use-env name 'ref)))))]  ; listnames looked up in use-env
          [(eq? at 'peek) ; for free-id=? : see where id resides or would reside if alloced
           (or (name-lookup lr name #f) (name-lookup ir name #f)
               (if (symbol? name) lr (use-env name 'peek)))] 
          [(memq at '(set! define)) ; same action for both operations
           ; works only for symbolic names; auto-allocates but does not shadow
           (and (symbol? name) ; no set!/define to list names
                (name-lookup lr name ; return if in local registry
                  (lambda (n) ; ok, not in lr: check ir and fail if it's there 
                    (and (not (name-lookup ir name #f)) ; not imported? alloc:
                         (list 'ref (global name))))))]
          [(eq? at 'define-syntax) ; for introducing new syntax definition
           ; works for all names; auto-allocates but does not shadow
           (name-lookup lr name ; return if in local registry
             (lambda (n) ; not in lr: check ir and fail if it's there
               (and (not (name-lookup ir name #f)) ; not imported? alloc:
                    (void))))]
          [(and (eq? at 'import) (sexp-match? '((<symbol> . #&*) ...) name))
           ; someone trues to add new imports: allow if there are no conflicts
           (let ([ial name])
             (define (check p)
               (cond [(name-lookup lr (car p) #f) => (lambda (loc) 
                      (x-error "imported name shadows local name" (car p) (cdr p) loc))]))
             (for-each check ial)
             (eal-name-registry-import! ir ial)
             #t)] ; don't bother returning counts, they are useful in repl only
          [else #f])))

; makes environments for .sld files, automatically extending root registry with list names
(define (make-sld-environment rr)
  ; note: lookup in sld environment can cause recursive calls to fetch-library
  ; if upstream dependencies are not yet loaded; loops are detected inside f-l
  (define (sld-env id at)
    (cond [(not (memq at '(ref peek))) #f]
          [(new-id? id) (new-id-lookup id at)]
          [(eq? at 'peek) (or (name-lookup rr name #f) rr)] ; for free-id=? purposes
          [(eq? id 'define-library) (make-location 'define-library)]
          [(not (listname? id)) #f]
          [else (name-lookup rr id
                  (lambda (n) ; no library? see if we can fetch it recursively
                    (fetch-library id sld-env)))])) ;=> <library> or #f
  sld-env)

; makes mutable environments from two registries; new bindings go to user registry
(define (make-repl-environment rr ur gpref) ; gpref is prefix for allocated globals
  (define (global name) (fully-qualified-library-prefixed-name gpref name))
  (lambda (name at)
    (cond [(new-id? name) ; nonsymbolic ids can't be (re)bound here
           (case at [(ref set! peek) (new-id-lookup name at)] [else #f])]
          [(eq? at 'ref) ; for reference only: try not to alloc
           (name-lookup ur name ; return if in user registry
             (lambda (n) ; ok, not in ur: check rr
               (or (name-lookup rr name ; if in rr, return it as-is
                     (lambda (n) ; not in rr: see if it is a library to autoload
                        (and (listname? name) ; default value: autoload from .sld
                             (let ([sld-env (make-sld-environment rr)]) 
                               (fetch-library name sld-env))))) ;=> <library> or #f
                   (and (symbol? name) (list 'ref (global name))))))] ; alloc in repl store
          [(eq? at 'peek) ; for free-id=? : see where id resides or would reside if alloced
           (or (name-lookup ur name #f) (name-lookup rr name #f)
               (if (symbol? name) ur rr))] ; symbol ids: would be alloced in ur if not found 
          [(eq? at 'set!) ; for assigning new values to variables
           ; works only for symbolic names; auto-allocates but does not shadow
           (and (symbol? name) ; no set! to list names
                (name-lookup ur name ; return if in user registry
                  (lambda (n) ; ok, not in ur: check rr and fail if it's there 
                    (and (not (name-lookup rr name #f)) ; not there? alloc:
                         (list 'ref (global name))))))]
          [(eq? at 'define) ; for introducing new definition
           ; works only for symbolic names; auto-allocates and shadows
           (and (symbol? name) ; no define for list names
                (name-lookup ur name ; return if in user registry
                  (lambda (n) ; ok, not in ur: alloc 
                    (list 'ref (global name)))))]
          [(eq? at 'define-syntax) ; for introducing new syntax definition
           ; works for all names; auto-allocates and shadows
           (name-lookup ur name ; return if in user registry
             (lambda (n) ; ok, not in ur: alloc 
               (void)))]
          [(and (eq? at 'import) (sexp-match? '((<symbol> . #&*) ...) name))
           ; special request for repl environment only: mass import
           (let loop ([eal name] [samc 0] [modc 0] [addc 0])
             (if (null? eal)
                 (list samc modc addc) ; return counts for verbosity
                 (let ([id (caar eal)] [loc (cdar eal)] [eal (cdr eal)])
                   (name-remove! ur id) ; user binding isn't changed, but no longer visible 
                   (case (name-install! rr id loc) ; root binding possibly changed
                     [(same)     (loop eal (+ samc 1) modc addc)]
                     [(modified) (loop eal samc (+ modc 1) addc)]
                     [(added)    (loop eal samc modc (+ addc 1))]))))] 
          [else #f])))

(define (find-library-in-env listname env) ;=> library | #f
  (let ([loc (env listname 'ref)])
    (and loc (let ([val (location-val loc)]) (and (val-library? val) val)))))

(define root-environment
  (make-readonly-environment *root-name-registry*))

(define repl-environment 
  (make-repl-environment *root-name-registry* *user-name-registry* 'repl://))

(define (empty-environment id at)
  (cond [(new-id? id) (new-id-lookup id at)]
        [else #f]))
 
(define (make-historic-report-environment listname prefix)
  (let* ([loc (name-lookup *root-name-registry* listname #f)]
         [l (and loc (location-val loc))] [l (and (val-library? l) l)]
         [ial (and l (library-exports l))] [global (lambda (n) (symbol-append prefix n))])
    (and (list? ial) (make-controlled-environment ial global empty-environment))))

(define r5rs-environment 
  (make-historic-report-environment '(scheme r5rs) 'r5rs://))

(define r5rs-null-environment 
  (make-historic-report-environment '(scheme r5rs-null) 'r5rs://))


; public interface to environments as per r7rs

(define (interaction-environment) repl-environment)

(define (scheme-report-environment v)
  (let ([e (and (eqv? v 5) r5rs-environment)])
    (or e (error "cannot create r5rs environment"))))

(define (null-environment v)
  (let ([e (and (eqv? v 5) r5rs-null-environment)])
    (or e (error "cannot create r5rs null environment"))))

; this freaking thing breaks expand-time -- run-time barrier!
(define (environment . isets)
  (define iform (cons 'import isets)) ; incredibly stupid, but whatcha gonna do
  (define ienv root-environment) ; it's a procedure, so there you go...
  (define ic&eal (preprocess-import-sets iform ienv)) ;=> (init-core . exports-eal)
  (define ir (eal->name-registry (cdr ic&eal) 1)) ; handmade import registry from ial
  ; initialization code got to be run, so we may as well do it right now
  (compile-and-run-core-expr (car ic&eal)) ; defined below, value(s) ignored
  ; now just wrap the regisry in read-only env and be done with it
  ; note: lookup of listnames is disabled -- this env is not for imports/d-ls
  (make-readonly-environment ir))


;--------------------------------------------------------------------------------------------------
; Eval
;--------------------------------------------------------------------------------------------------

(define *verbose* #f)
(define *quiet* #f)

(define (compile-and-run-core-expr core) ; => returns value(s) of core run
  (define (compile-and-run core)
    (define code (compile-to-thunk-code core))
    (define cl (closure (deserialize-code code)))
    (cl)) ; tail call possibly returning multiple values
  (when *verbose* (display "TRANSFORM =>") (newline) (write core) (newline))
  (unless (val-core? core) (x-error "unexpected transformed output" core))
  (let loop ([cores (list core)])
    (if (null? cores) 
        (void)
        (let ([first (car cores)] [rest (cdr cores)])
          (record-case first
            [begin exps 
             (loop (append exps rest))]
            [once (gid exp)
             (compile-and-run first) ; ignore value(s) of init code
             ; this 'once' is done and there is no need to keep it around
             (set-car! first 'begin) (set-cdr! first '()) ; mucho kluge!
             (loop rest)]
            [else
             (if (null? rest)
                 (compile-and-run first) ; tail call
                 (begin (compile-and-run first) (loop rest)))])))))      

(define (evaluate-top-form x env)
  (if (pair? x)
      (let ([hval (xform #t (car x) env)]) ; returns <core>
        (cond
          [(eq? hval 'begin) ; splice
           (let loop ([x* (cdr x)])
             (cond [(null? x*) (void)]
                   [(not (pair? x*)) (x-error "invalid begin form:" x)]
                   [(null? (cdr x*)) (evaluate-top-form (car x*) env)] ; tail call
                   [else (evaluate-top-form (car x*) env) (loop (cdr x*))]))]
          [(eq? hval 'define) ; use new protocol for top-level envs
           (let ([tail (preprocess-define (car x) (cdr x))])
             (if (list1? tail) ; tail is either (sexp) or (id sexp)
                 (begin (evaluate-top-form (car tail) env) (void))
                 (let ([loc (top-defined-id-lookup env (car tail) 'define)])
                   (unless (and (location? loc) (sexp-match? '(ref *) (location-val loc)))
                     (x-error "identifier cannot be (re)defined as variable" (car tail) x))
                   (let ([g (cadr (location-val loc))] [core (xform #f (cadr tail) env)])
                     (compile-and-run-core-expr (list 'set! g core)) (void)))))]
          [(eq? hval 'define-syntax) ; use new protocol for top-level envs
           (let* ([tail (preprocess-define-syntax (car x) (cdr x))]
                  [loc (top-defined-id-lookup env (car tail) 'define-syntax)])
             (unless (location? loc) 
               (x-error "unexpected define-syntax for id" (car tail) x))
             (location-set-val! loc (xform #t (cadr tail) env))
             (when *verbose* (display "SYNTAX INSTALLED: ") (write (car tail)) (newline))
             (void))]
          [(eq? hval 'define-library) ; use new protocol for top-level envs
           (let* ([core (xform-define-library (car x) (cdr x) env #t)]
                  ; core is (define-library <listname> <library>)
                  [loc (xenv-lookup env (cadr core) 'define-syntax)])
             (unless (location? loc) 
               (x-error "unexpected define-library for id" (cadr core) x))
             (location-set-val! loc (caddr core))
             (when *verbose* (display "LIBRARY INSTALLED: ") (write (cadr core)) (newline)))]
          [(eq? hval 'import) ; splice as definitions
           (let* ([core (xform-import (car x) (cdr x) env #t)] 
                  ; core is (import <library>)
                  [l (cadr core)] [code (library-code l)] [eal (library-exports l)])
             ; note: try to use env's import protocol
             (let ([res (env eal 'import)]) 
               (unless res ; this env does not support import 
                 (x-error "failed to import to env, import is not supported:" env eal))
               (when (and (or (not *quiet*) *verbose*) 
                          (sexp-match? '(<number> <number> <number>) res))
                 (if *verbose* (display "IMPORT: ") (display "; import: "))
                 (write (car res)) (display " bindings are the same, ")
                 (write (cadr res)) (display " modified, ")
                 (write (caddr res)) (display " added\n")))
             (compile-and-run-core-expr code))]
          [(val-transformer? hval) ; apply transformer and loop
           ; NOTE: if transformer output is a begin, it needs to be scanned for defines 
           ; in case some of them use generated names that need to be gensym'd via pp pass
           (let* ([x (hval x env)] [hv (and (pair? x) (xform #t (car x) env))])
             (if (and (eq? hv 'begin) (list2+? x))
                 (let* ([code* (preprocess-top-forms-scan (cdr x) env env)]
                        [fix! (lambda (code) (preprocess-top-form-fix! code env))]
                        [code (cons 'begin (map fix! (reverse! code*)))])
                   (compile-and-run-core-expr code)) ; tail
                 (evaluate-top-form x env)))] ; tail
          [(val-integrable? hval) ; integrable application
           (compile-and-run-core-expr (xform-integrable hval (cdr x) env))]
          [(val-builtin? hval) ; other builtins
           (compile-and-run-core-expr (xform #f x env))]
          [else ; regular call
           (compile-and-run-core-expr (xform-call hval (cdr x) env))]))
      ; var refs and literals
      (compile-and-run-core-expr (xform #f x env))))


; public interface to eval as per r7rs

; another disgusting breaker of expand-time -- run-time barrier
(define (eval expr . ?env)
  (define env (if (pair? ?env) (car ?env) (interaction-environment)))
  (evaluate-top-form expr env))

; and its dirty cousin
(define (load filename . ?env)
  (define env (if (pair? ?env) (car ?env) (interaction-environment)))
  (define ci? #f) ; do not bother setting this unless told by the specification
  (call-with-current-input-file filename ;=>
    (lambda (port) 
      (when ci? (set-port-fold-case! port #t))
      (let loop ([x (read-code-sexp port)])
        (unless (eof-object? x)
          (eval x env)
          (loop (read-code-sexp port))))))
  ; we aren't asked by the spec to call last expr tail-recursively, so this
  (void)) 

; srfi-22 - like script processor (args is list of strings)
(define (run-script filename args)
  (define env (interaction-environment))
  (define ci? #f) ; normal load-like behavior is the default
  (define callmain #f) ; got changed via first #! line
  (define main-args (cons filename args))
  (call-with-current-input-file filename ;=>
    (lambda (port) 
      (let ([x0 (read-code-sexp port)])
        (when (shebang? x0)
          (let ([shs (symbol->string (shebang->symbol x0))])
            (cond [(string-position "r7rs" shs)] ; skint env will do
                  [(string-position "skint" shs)] ; ditto
                  [(string-position "r5rs" shs)
                   (set! env (scheme-report-environment 5))
                   (set! ci? #t)]
                  [else (error "only scheme-r[57]rs scripts are supported" shs)]) 
            (when ci? (set-port-fold-case! port #t))
            (set! callmain #t)
            (set! x0 (read-code-sexp port))))
        (parameterize ([command-line main-args])
          (let loop ([x x0])
            (unless (eof-object? x)
              (eval x env)
              (loop (read-code-sexp port)))))
        (if callmain
          ; if it is a real script, call main and return its value
          (eval `(main (quote ,main-args)) env)
          ; otherwise return #t -- it will be used as exit value
          #t)))))

; r7rs scheme program processor (args is list of strings)
(define (run-program filename args)
  (define modname (string->symbol (path-strip-extension (path-strip-directory filename))))
  (define global (lambda (n) (symbol-append 'prog:// modname '? n)))
  (define ial (list (cons 'import (make-location 'import))))
  (define env (make-controlled-environment ial global root-environment))
  (define ci? #f) ; normal load-like behavior is the default
  (define main-args (cons filename args))
  (call-with-current-input-file filename ;=>
    (lambda (port) 
      (command-line main-args)
      (let loop ([x (read-code-sexp port)])
        (unless (eof-object? x)
          (eval x env)
          (loop (read-code-sexp port))))))
    ; return #t -- it will be used as exit value
    #t)


;--------------------------------------------------------------------------------------------------
; REPL
;--------------------------------------------------------------------------------------------------

(define (repl-evaluate-top-form x env op)
  (define-values vals (evaluate-top-form x env))
  (define (print val) (write val op) (newline op))
  (unless (and (list1? vals) (void? (car vals)))
    (for-each print vals)))

(define (repl-read ip prompt op)
  (when prompt  (format op "~%~a ~!" prompt))
  (read-code-sexp ip))

(define (repl-exec-command cmd argstr op)
  (define args
    (if (memq cmd '(load cd sh)) ; do not expect s-exps!
        (list (string-trim-whitespace argstr))
        (guard (err [else (void)]) (read-port-sexps (open-input-string argstr)))))
  (let retry ([cmd+args (cons cmd args)])
    (sexp-case cmd+args
      [(ref <symbol>) (write (repl-environment (car args) 'ref) op) (newline op)]
      [(ref (* * ...)) (let ([x (repl-environment (car args) 'peek)]) 
                         (write (and (location? x) x) op) (newline op))]
      [(rnr) (write *root-name-registry* op) (newline op)]
      [(rref *) (write (name-lookup *root-name-registry* (car args) #f) op) (newline op)]
      [(rrem! *) (cond [(name-lookup *root-name-registry* (car args) #f)
                        (name-remove! *root-name-registry* (car args)) (display "done!\n" op)]
                      [else (display "name not found: " op) (write name op) (newline op)])]
      [(unr) (write *user-name-registry* op) (newline op)]
      [(uref *) (write (name-lookup *user-name-registry* (car args) #f) op) (newline op)]
      [(urem! *) (cond [(name-lookup *user-name-registry* (car args) #f)
                        (name-remove! *user-name-registry* (car args)) (display "done!\n" op)]
                      [else (display "name not found: " op) (write name op) (newline op)])]
      [(gs) (write (global-store) op) (newline op)]
      [(gs <symbol>) 
       (let* ([k (car args)] [v (global-store)] [i (immediate-hash k (vector-length v))]) 
         (write (cond [(assq k (vector-ref v i)) => cdr] [else #f]) op) (newline op))]
      [(load <string>) (load (car args))]
      [(v)  (set! *verbose* #t) (format #t "verbosity is on~%")]
      [(v-) (set! *verbose* #f) (format #t "verbosity is off~%")]
      [(q)  (set! *quiet* #t) (format #t "quiet is on~%")]
      [(q-) (set! *quiet* #f) (format #t "quiet is off~%")]
      [(time *) (let ([start (current-jiffy)])
                  (repl-evaluate-top-form (car args) repl-environment op)
                  (format #t "; elapsed time: ~s ms.~%"
                    (* 1000 (/ (- (current-jiffy) start) (jiffies-per-second)))))]
      [(pwd) (display (current-directory) op) (newline op)]
      [(cd <string>) (current-directory (car args))]
      [(sh <string>) (%system (car args))]
      [(si) (format #t "~d collections, ~d reallocs, heap size ~d words~%" 
               (%gc-count) (%bump-count) (%heap-size))]
      [(gc) (%gc) (retry '(si))]
      [(help)
       (display "\nREPL commands (,load ,cd ,sh arguments need no quotes):\n" op)
       (display " ,load <fname>       load <fname> into REPL\n" op)
       (display " ,q                  quiet: disable informational messages\n" op)
       (display " ,q-                 enable informational messages\n" op)
       (display " ,v                  turn verbosity on\n" op)
       (display " ,v-                 turn verbosity off\n" op)
       (display " ,ref <name>         show denotation for <name>\n" op)
       (display " ,rnr                show root name registry\n" op)
       (display " ,rref <name>        lookup name in root registry\n" op)
       (display " ,rrem! <name>       remove name from root registry\n" op)
       (display " ,unr                show user name registry\n" op)
       (display " ,uref <name>        lookup name in user registry\n" op)
       (display " ,urem! <name>       remove name from user registry\n" op)
       (display " ,gs                 show global store (big!)\n" op)
       (display " ,gs <name>          lookup global location for <name>\n" op)
       (display " ,time <expr>        time single-line expression <expr>\n" op)
       (display " ,pwd                show skint's current working directory\n" op)
       (display " ,cd <dir>           change skint's current working directory\n" op)
       (display " ,sh <cmdline>       send <cmdline> to local shell\n" op)
       (display " ,si                 display system info\n" op)
       (display " ,gc                 force gc to finalize lost objects\n" op)
       (display " ,help               this help\n" op)]
      [(h) (retry '(help))]
      [else
       (display "syntax error in repl command\n" op)
       (display "type ,help to see available commands\n" op)])))

(define (repl-from-port ip env prompt op)
  (define cfs (current-file-stack))
  (guard (err 
          [(error-object? err)
           (let ([p (current-error-port)])
            (display (error-object-message err) p) (newline p)
            (for-each (lambda (arg) (write arg p) (newline p)) 
              (error-object-irritants err)))
           (set-current-file-stack! cfs)
           (when prompt (repl-from-port ip env prompt op))]
          [else 
           (let ([p (current-error-port)])
             (display "Unknown error:" p) (newline p)
             (write err p) (newline p))
           (set-current-file-stack! cfs)
           (when prompt (repl-from-port ip env prompt op))])
    (let loop ([x (repl-read ip prompt op)])
      (unless (eof-object? x)
        (if (and prompt (sexp-match? '(unquote *) x))
            (repl-exec-command (cadr x) (read-line ip) op)
            (repl-evaluate-top-form x env op))
        (loop (repl-read ip prompt op))))))

(define (run-benchmark fname args) ; for debug purposes only
  (define ip (open-input-file fname))
  (define op (current-output-port))
  (unless (sexp-match? '(load "libl.sf") (read-code-sexp ip))
    (error "unexpected benchmark file format" fname))
  (let ([start (current-jiffy)])
    (repl-from-port ip repl-environment #f op)
    (repl-evaluate-top-form '(main) repl-environment op)  
    (format #t "; elapsed time: ~s ms.~%"
      (* 1000 (/ (- (current-jiffy) start) (jiffies-per-second)))))
  (close-input-port ip))

(define *repl-first-time* #t)

(define (repl)
  (define ip (current-input-port))
  (define op (current-output-port))
  (define prompt (and (tty-port? ip) "skint]")) 
  (set-current-file-stack! '())
  (when *repl-first-time*
    (set! *repl-first-time* #f)
    (skint-main))
  ; capture cc to handle unhandled exceptions
  (letcc k (set-reset-handler! k)
    (repl-from-port ip repl-environment prompt op))
  #t) ; exited normally via end-of-input


;--------------------------------------------------------------------------------------------------
; Main
;--------------------------------------------------------------------------------------------------

(define *skint-options*
 '([verbose        "-v" "--verbose" #f            "Increase output verbosity"]
   [quiet          "-q" "--quiet" #f              "Suppress nonessential messages"]
   [append-libdir  "-A" "--append-libdir" "DIR"   "Append a library search directory"] 
   [prepend-libdir "-I" "--prepend-libdir" "DIR"  "Prepend a library search directory"] 
   [define-feature "-D" "--define-feature" "NAME" "Add name to the list of features"] 
   [eval           "-e" "--eval" "SEXP"           "Evaluate and print an expression"] 
   [script         "-s" "--script" "FILE"         "Run file as a Scheme script"]
   [program        "-p" "--program" "FILE"        "Run file as a Scheme program"]
   ;[benchmark       #f  "--benchmark" "FILE"     "Run .sf benchmark file (internal)"]
   [version        "-V" "--version" #f            "Display version info"]
   [help           "-h" "--help" #f               "Display this help"]
))

(define *skint-version* "0.3.9")

(define (skint-main)
  ; see if command line asks for special processing
  (define (eval! str print?)
    (let ([obj (read-from-string str)])
      (for-each (lambda (val) (when print? (write val) (newline)))
        (call-with-values (lambda () (eval obj)) list))))
  (define (add-feature! f)
    (features (set-cons (string->symbol f) (features))))
  (define (print-version!)
    (format #t "(version ~s)~%(scheme.id skint)~%" *skint-version*))
  (define (print-help!)
    (format #t "SKINT Scheme Interpreter v~a~%" *skint-version*)
    (format #t "usage: skint [OPTION]... [FILE] [ARG]...~%")
    (format #t "~%")
    (format #t "Options:~%")
    (print-command-line-options *skint-options* (current-output-port))
    (format #t "~%")
    (format #t "'--' ends options processing. Standalone FILE argument is treated as a script.~%")
    (format #t "If no FILE is given, skint enters Read-Eval-Print loop (stdin>eval-print>stdout)~%")
    (format #t "~%"))
  (define cl (command-line))
  (let loop ([args (cdr cl)] [repl? #t])
    (get-next-command-line-option args *skint-options* ;=>
      (lambda (keysym optarg restargs)
        (sexp-case (if optarg (list keysym optarg) (list keysym))
          [(verbose) (set! *verbose* #t) (loop restargs #t)]
          [(quiet) (set! *quiet* #t) (loop restargs #t)]
          [(append-libdir *) (append-library-path! optarg) (loop restargs #t)]
          [(prepend-libdir *) (prepend-library-path! optarg) (loop restargs #t)]
          [(define-feature *) (add-feature! optarg) (loop restargs #t)]
          [(eval *) (eval! optarg #t) (loop restargs #f)]
          [(script *) (set! *quiet* #t) (exit (run-script optarg restargs))]
          [(program *) (set! *quiet* #t) (exit (run-program optarg restargs))]
          [(benchmark *) (exit (run-script optarg restargs))] ; FIXME
          [(version) (print-version!) (loop '() #f)]
          [(help) (print-help!) (loop '() #f)]
          [(#f) (cond [(pair? restargs) 
                       (set! *quiet* #t) (exit (run-script (car restargs) (cdr restargs)))]
                      [(not repl?) (exit #t)])])))) ; all done -- no need to continue
  ; falling through to interactive mode
  (when (and (tty-port? (current-input-port)) (tty-port? (current-output-port)))
    ; quick check for non-interactive use failed, greet
    (format #t "SKINT Scheme Interpreter v~a~%" *skint-version*)
    (format #t "Copyright (c) 2024 False Schemers~%"))
  #t) ; exited normally
