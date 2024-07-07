
;---------------------------------------------------------------------------------------------
; Transformer and Compiler
;---------------------------------------------------------------------------------------------

(load "s.scm")

;---------------------------------------------------------------------------------------------
; Utils
;---------------------------------------------------------------------------------------------

(define set-member?
  (lambda (x s)
    (cond
      [(null? s) #f]
      [(eq? x (car s)) #t]
      [else (set-member? x (cdr s))])))

(define set-cons
  (lambda (x s) 
    (if (set-member? x s) 
        s 
        (cons x s))))

(define set-union
  (lambda (s1 s2)
    (if (null? s1) 
        s2 
        (set-union (cdr s1) (set-cons (car s1) s2)))))

(define set-minus
  (lambda (s1 s2)
    (if (null? s1)
        '()
        (if (set-member? (car s1) s2)
            (set-minus (cdr s1) s2)
            (cons (car s1) (set-minus (cdr s1) s2))))))

(define set-intersect
  (lambda (s1 s2)
    (if (null? s1)
        '()
        (if (set-member? (car s1) s2)
            (cons (car s1) (set-intersect (cdr s1) s2))
            (set-intersect (cdr s1) s2)))))

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
      (and (eq? pat '<string>) (string? x))
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

(define symbol-append
  (lambda syms (string->symbol (apply string-append (map symbol->string syms)))))

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

(define posq
  (lambda (x l)
    (let loop ([l l] [n 0])
      (cond [(null? l) #f]
            [(eq? x (car l)) n]
            [else (loop (cdr l) (fx+ n 1))]))))

(define rassq
  (lambda (x al)
    (and (pair? al)
         (let ([a (car al)])
           (if (eq? x (cdr a)) a (rassq x (cdr al)))))))

(define list-diff
  (lambda (l t)
    (if (or (null? l) (eq? l t))
        '()
        (cons (car l) (list-diff (cdr l) t)))))

(define (pair* x . more)
  (let loop ([x x] [rest more])
    (if (null? rest) x 
        (cons x (loop (car rest) (cdr rest))))))

(define (append* lst)
  (cond [(null? lst) '()]
        [(null? (cdr lst)) (car lst)]
        [else (append (car lst) (append* (cdr lst)))]))

(define (string-append* l) 
  (apply string-append l)) 

(define (andmap p l)
  (if (pair? l) (and (p (car l)) (andmap p (cdr l))) #t))

(define (ormap p l)
  (if (pair? l) (or (p (car l)) (ormap p (cdr l))) #f))

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


;---------------------------------------------------------------------------------------------
; Syntax of the Scheme Core language
;---------------------------------------------------------------------------------------------

;  <core> -> (quote <object>)
;  <core> -> (const <id>) ; immutable variant of ref 
;  <core> -> (ref <id>)
;  <core> -> (set! <id> <core>)
;  <core> -> (set& <id>)
;  <core> -> (gref <global>)
;  <core> -> (gset! <global> <core>)
;  <core> -> (lambda <ids> <core>) where <ids> -> (<id> ...) | (<id> ... . <id>) | <id>
;  <core> -> (lambda* (<arity> <core>) ...) where <arity> -> (<cnt> <rest?>) 
;  <core> -> (letcc <id> <core>) 
;  <core> -> (withcc <core> <core>) 
;  <core> -> (begin <core> ...)
;  <core> -> (if <core> <core> <core>)
;  <core> -> (call <core> <core> ...) 
;  <core> -> (integrable <ig> <core> ...) where <ig> is an index in the integrables table
;  <core> -> (asm <igs>) where <igs> is ig string leaving result in ac, e.g. "'2,'1+" 
;  <core> -> (once <gid> <core>) where gid is always resolved as global

;  NB: (begin) is legit, returns unspecified value
;  on top level, these four extra core forms are legal:

;  <core> -> (define <id> <core>)
;  <core> -> (define-syntax <id> <transformer>)
;  <core> -> (define-library <listname> <library>)
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

;---------------------------------------------------------------------------------------------
; Macro transformer (from Scheme to Scheme Core) derived from Al Petrofsky's EIOD 1.17
;---------------------------------------------------------------------------------------------

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
; <library>     ->  <vector of init-code and export-alist>

(define-syntax  location?            box?)
(define-syntax  make-location        box)
(define-syntax  location-val         unbox)
(define-syntax  location-set-val!    set-box!)
(define-syntax  val-core?            pair?)
(define-syntax  val-transformer?     procedure?)
(define-syntax  val-integrable?      integrable?)
(define-syntax  val-builtin?         symbol?)
(define-syntax  val-library?         vector?)
(define-syntax  make-library         vector)
(define-syntax  library-code         (syntax-rules () [(_ l) (vector-ref l 0)]))
(define-syntax  library-exports      (syntax-rules () [(_ l) (vector-ref l 1)]))
(define-syntax  library-set-exports! (syntax-rules () [(_ l v) (vector-set! l 1 v)]))

(define (location-special? l)        (not (pair? (unbox l))))
(define (new-id sym den getlits)     (define p (list sym den getlits)) (lambda () p))
(define (old-sym id)                 (car (id)))
(define (old-den id)                 (cadr (id)))
(define (old-literals id)            ((or (caddr (id)) (lambda () '()))))
(define (id? x)                      (or (symbol? x) (procedure? x)))
(define (id->sym id)                 (if (symbol? id) id (old-sym id)))

; take a possibly renamed target id, and find image for nid
(define (id-rename-as id nid)
  (let loop ([id id])
    (if (symbol? id) nid
        (let* ([lits (old-literals id)] [oid->id (rassq id lits)])
          (unless oid->id (x-error "id-rename-as failed: not found in its own lits" id))
          (let ([renamed-nid (loop (car oid->id))])
            (cond [(assq renamed-nid lits) => cdr]
                  [else renamed-nid]))))))

; Expand-time environments map identifiers (symbolic or thunked) to denotations, i.e. locations
; containing either a <special> or a <core> value. In normal case, <core> value is (ref <gid>),
; where <gid> is a key in run-time store, aka *globals*. Environments should allocate new locations
; as needed, so every identifier gets mapped to one. Expand-time environments are represented as
; two-argument procedures, where the second argument is an access type symbol.

(define (extend-xenv-local id val env)
  (let ([loc (make-location val)])
    (lambda (i at) 
      (if (eq? id i)
          (case at [(ref set!) loc] [else #f]) 
          (env i at)))))

(define (add-local-var id gid env)
  (extend-xenv-local id (list 'ref gid) env))

(define (xenv-lookup env id at)
  (or (env id at)
      (error* "transformer: invalid identifier access" (list id (id->sym id) at))))

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

(define lambda-id (new-id 'lambda (make-location 'lambda) #f))
(define begin-id (new-id 'begin (make-location 'begin) #f))
(define define-id (new-id 'define (make-location 'define) #f))
(define define-syntax-id (new-id 'define-syntax (make-location 'define-syntax) #f))
(define syntax-quote-id (new-id 'syntax-quote (make-location 'syntax-quote) #f))

; xform receives Scheme s-expressions and returns either Core Scheme <core>
; (always a pair) or special-form, which is either a builtin (a symbol) or
; a transformer (a procedure). Appos? flag is true when the context can
; allow xform to return a special; otherwise, only <core> is returned. 

(define (xform appos? sexp env)
  (cond [(id? sexp) 
         (let ([hval (xform-ref sexp env)])
           (cond [appos? hval]
                 [(val-integrable? hval) ; integrable id-syntax
                  (list 'ref (integrable-global hval))]
                 [(val-transformer? hval) ; id-syntax
                  (xform appos? (hval sexp env) env)]
                 [(not (val-core? hval)) ; other special used out of context 
                  (x-error "improper use of syntax form" hval)]
                 [else hval]))] ; core 
        [(not (pair? sexp))
         (xform-quote (list sexp) env)]
        [else 
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
             [(define-library) (xform-define-library head tail env appos?)]
             [(import)         (xform-import         head tail env appos?)]
             [else (cond [(val-integrable? hval)  (xform-integrable hval tail env)]
                         [(val-transformer? hval) (xform appos? (hval sexp env) env)]
                         [(val-library? hval)     (x-error "improper use of library" hval sexp)]
                         [else                    (xform-call hval tail env)])]))]))

(define (xform-quote tail env)
  (if (list1? tail)
      (list 'quote (xform-sexp->datum (car tail)))
      (x-error "improper quote form" (cons 'quote tail))))

(define (xform-ref id env)
  (let ([den (xenv-ref env id)])
    (cond [(eq? (location-val den) '...) (x-error "improper use of ...")]
          [else (location-val den)])))

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
                  (cond [(and (list2? tail) (null? (car tail))) ; idless
                         (let ([init (cadr tail)])
                           (loop env (cons #f ids) (cons init inits) (cons #f nids) rest))]
                        [(and (list2? tail) (id? (car tail)))
                         (let* ([id (car tail)] [init (cadr tail)]
                                [nid (gensym (id->sym id))] [env (add-local-var id nid env)])
                           (loop env (cons id ids) (cons init inits) (cons nid nids) rest))]
                        [(and (list2+? tail) (pair? (car tail)) (id? (caar tail)) (idslist? (cdar tail)))
                         (let* ([id (caar tail)] [init (cons lambda-id (cons (cdar tail) (cdr tail)))]
                                [nid (gensym (id->sym id))] [env (add-local-var id nid env)])
                           (loop env (cons id ids) (cons init inits) (cons nid nids) rest))]
                        [else (x-error "improper define form" first)])]
                 [(define-syntax) ; internal
                  (if (and (list2? tail) (id? (car tail))) 
                      (let* ([id (car tail)] [init (cadr tail)]
                             [env (extend-xenv-local id '(undefined) env)])
                        (loop env (cons id ids) (cons init inits) (cons #t nids) rest))
                      (x-error "improper define-syntax form" first))]
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

(define (xform-begin tail env appos?) ; non-internal
  (if (list? tail)
      (if (list1? tail)
          (xform appos? (car tail) env) ; (begin x) == x
          (cons 'begin (map (lambda (sexp) (xform #f sexp env)) tail)))
      (x-error "improper begin form" (cons 'begin tail))))

(define (xform-define tail env) ; non-internal
  (cond [(and (list2? tail) (null? (car tail))) ; idless
         (xform #f (cadr tail) env)]
        [(and (list2? tail) (id? (car tail)))
         (list 'define (id->sym (car tail)) 
           (xform #f (cadr tail) env))]
        [(and (list2+? tail) (pair? (car tail)) (id? (caar tail)) (idslist? (cdar tail)))
         (list 'define (id->sym (caar tail))
           (xform-lambda (cons (cdar tail) (cdr tail)) env))] 
        [else 
         (x-error "improper define form" (cons 'define tail))]))

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
  (define (ellipsis-denotation? den)
    (eq? (location-val den) '...)) ; fixme: need eq? with correct #&...
  (define (ellipsis? x)
    (if ellipsis
        (eq? x ellipsis)
        (and (id? x) (ellipsis-denotation? (xenv-ref mac-env x)))))

  ; List-ids returns a list of the non-ellipsis ids in a
  ; pattern or template for which (pred? id) is true.  If
  ; include-scalars is false, we only include ids that are
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

  ; Returns #f or an alist mapping each pattern var to a part of
  ; the input.  Ellipsis vars are mapped to lists of parts (or
  ; lists of lists ...).
  (define (match-pattern pat use use-env)
    (call-with-current-continuation
      (lambda (return)
        (define (fail) (return #f))
        (let match ([pat pat] [sexp use] [bindings '()])
          (define (continue-if condition)
            (if condition bindings (fail)))
          (cond
            [(id? pat)
              (if (pat-literal? pat)
                  (continue-if 
                    (and (id? sexp) (eq? (xenv-ref use-env sexp) (xenv-ref mac-env pat))))
                  (cons (cons pat sexp) bindings))]
            [(vector? pat)
             (or (vector? sexp) (fail))
             (match (vector->list pat) (vector->list sexp) bindings)]
            [(not (pair? pat))
             (continue-if (equal? pat sexp))]
            [(ellipsis-pair? (cdr pat))
             (let* ([tail-len (length (cddr pat))]
                    [sexp-len (if (list? sexp) (length sexp) (fail))]
                    [seq-len (fx- sexp-len tail-len)]
                    [sexp-tail (begin (if (negative? seq-len) (fail)) (list-tail sexp seq-len))]
                    [seq (reverse (list-tail (reverse sexp) tail-len))]
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
    ; fresh id, assq will always retrieve the first one.
    (define new-literals
      (body
        (define nl
          (map (lambda (id) (cons id (new-id (id->sym id) (xenv-ref mac-env id) (lambda () nl))))
               (list-ids tmpl #t (lambda (id) (not (assq id top-bindings))))))
        nl))

    (define ellipsis-vars
      (list-ids pat #f not-pat-literal?))

    (define (list-ellipsis-vars subtmpl)
      (list-ids subtmpl #t 
        (lambda (id) (memq id ellipsis-vars))))

    (let expand ([tmpl tmpl] [bindings top-bindings])
      (let expand-part ([tmpl tmpl])
        (cond
          [(id? tmpl)
           (cdr (or (assq tmpl bindings)
                    (assq tmpl top-bindings)
                    (assq tmpl new-literals)))]
          [(vector? tmpl)
           (list->vector (expand-part (vector->list tmpl)))]
          [(and (pair? tmpl) (ellipsis-pair? (cdr tmpl)))
           (let ([vars-to-iterate (list-ellipsis-vars (car tmpl))])
             (define (lookup var)
               (cdr (assq var bindings)))
             (define (expand-using-vals . vals)
               (expand (car tmpl)
                 (map cons vars-to-iterate vals)))
             (if (null? vars-to-iterate)
                 ; ellipsis following non-repeatable part is an error, but we don't care
                 (cons (expand-part (car tmpl)) (expand-part (cddr tmpl))) ; repeat once
                 ; correct use of ellipsis
                 (let ([val-lists (map lookup vars-to-iterate)])
                   (append
                     (apply map (cons expand-using-vals val-lists))
                     (expand-part (cddr tmpl))))))]
          [(pair? tmpl)
           (cons (expand-part (car tmpl)) (expand-part (cdr tmpl)))]
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
(define (preprocess-cond-expand lit=? sexp) ;=> (sexp ...)
  (define (pp freq con alt)
    (cond [(lit=? freq 'else) (con)]
          [(id? freq) (if (feature-available? (id->sym freq)) (con) (alt))]
          [(and (list2? freq) (lit=? (car freq) 'library))
           (if (library-available? (xform-sexp->datum (cadr freq))) (con) (alt))]
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
    (define (lit=? id sym) ; match literal using free-id=? -like match
      (and (id? id) (eq? (xenv-ref env id) (xenv-ref root-environment sym))))
    (cons begin-id (preprocess-cond-expand lit=? sexp))))

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
         ; NB: this is 1/3 of listname->library interface
         (let* ([listname (xform-sexp->datum s)] [sym (listname->symbol listname)]
                [id (id-rename-as sid sym)] [val (xform-ref id env)]) ; or should id be just sym?
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
            [(id? (car ee)) (let ([s (id->sym (car ee))]) (toesps (cdr ee) (adjoin-esps (list (cons s s)) esps)))]
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
               (let ([new-eal (list (cons 'import (make-location import-transformer)))])
                 (loop decls code (adjoin-eals new-eal eal) esps forms))]
              [(eq? (car decl) ld-import-id)
               (let ([ic&ex (preprocess-import-sets decl env)])
                 (let ([new-code (car ic&ex)] [new-eal (cdr ic&ex)])
                   (loop decls (adjoin-code code new-code) (adjoin-eals new-eal eal) esps forms)))]
              [(eq? (car decl) ld-library-declarations-id) ; splice
               (loop (append (cdr decl) decls) code eal esps forms)]
              [(eq? (car decl) ld-cond-expand-id) ; flatten and splice
               (let ([lit=? (lambda (id sym) (and (id? id) (eq? id (id-rename-as sid sym))))])
                 (loop (append (preprocess-cond-expand lit=? (cdr decl)) decls) code eal esps forms))]
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
        
; make functional read-only environment from import al,
; allowing fall-through to env for lib://foo/bar ids
(define (ial->controlled-environment ial make-nid env)
  (let ([v (make-vector 1 '())]) ; new ids go here
    (lambda (id at)
      (cond [(procedure? id) 
             (and (memq at '(ref const)) (old-den id))]
            [(assq id (vector-ref v 0)) => 
             cdr] ; full access to new locations
            [(assq id ial) => ; read-only access to imports, no shadowing?
             (lambda (b) (and (memq at '(ref const)) (cdr b)))] 
            [(symbol-libname? id) ; read-only acess to libs 
             (and (memq at '(ref const)) (env id at))]
            [(memq at '(ref const set! define))
             (let ([loc (make-location (list 'ref (make-nid id)))])
               (vector-set! v 0 (cons (cons id loc) (vector-ref v 0)))
               loc)]
            [(memq at '(define-syntax))
             (let ([loc (make-location '(undefined))])
               (vector-set! v 0 (cons (cons id loc) (vector-ref v 0)))
               loc)]
            [else #f]))))

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
           [cenv (ial->controlled-environment ial make-nid env)] [eal '()])
      (define (scan body code*) ;=> extended with side-effect on cenv
        (if (null? body) 
            code*
            (let ([first (car body)] [rest (cdr body)])
              (if (pair? first)
                  (let* ([head (car first)] [tail (cdr first)] [hval (xform #t head cenv)])
                    (cond
                      [(eq? hval 'begin)
                       (unless (list? tail) (x-error "improper begin form" first))
                       (scan (append tail rest) code*)]
                      [(and (eq? hval 'define) (list2? tail) (null? (car tail))) ; special idless define
                       (scan (append (cadr tail) rest) code*)]
                      [(eq? hval 'define)
                       (let* ([core (xform-define tail cenv)] 
                              [loc (xenv-lookup cenv (cadr core) 'define)])
                         (unless (location? loc) (x-error "unexpected define for id" (cadr core) first))
                         (scan rest (cons (list 'set! (cadr (location-val loc)) (caddr core)) code*)))] 
                      [(eq? hval 'define-syntax)
                       (let* ([core (xform-define-syntax tail cenv)]
                              [loc (xenv-lookup cenv (cadr core) 'define-syntax)])
                         (unless (location? loc) (x-error "unexpected define-syntax for id" (cadr core) first))
                         (location-set-val! loc (caddr core))
                         (scan rest code*))]
                      [(eq? hval 'define-library)
                       (x-error "NYI: define-library inside library code" first)]
                      [(eq? hval 'import)
                       (x-error "NYI: import inside library code" first)]
                      ; TODO: check for built-in (export) and modify eal!
                      [(val-transformer? hval) ; apply transformer and loop
                       (scan (cons (hval first cenv) rest) code*)]
                      [(val-integrable? hval) ; integrable application
                       (scan rest (cons (xform-integrable hval tail cenv) code*))]
                      [else ; other specials and calls (xform does not return libraries)
                       (scan rest (cons (xform #f first cenv) code*))]))
                  (scan rest (cons (xform #f first cenv) code*)))))) 
      (let* ([code* (scan forms '())] [forms-code (cons 'begin (reverse! code*))] 
             [combined-code (adjoin-code code (if lid (list 'once lid forms-code) forms-code))])
        ; walk through esps, fetching locations from cenv
        (let loop ([esps esps] [eal eal])
          (if (null? esps)
              (cons combined-code (reverse! eal))
              (let* ([lid (caar esps)] [eid (cdar esps)] [loc (cenv lid 'const)])
                (cond [(not loc) (x-error "cannot export id" lid)]
                      [(location-special? loc) (loop (cdr esps) (cons (cons eid loc) eal))]
                      [else (let ([val (location-val loc)])
                              (if (memq (car val) '(ref const))
                                  (loop (cdr esps) (cons (cons eid (make-location (list 'const (cadr val)))) eal))
                                  (x-error "cannot export code alias id" lid val)))]))))))))

; note: define-library semantics does not depend on lexical context, and, as a syntax definition,
; it should become available in local env immediately, even at definition-scanning phase -- so we
; introduce new special <core> form define-library

(define (xform-define-library head tail env appos?) ; non-internal
  (if (and (list2+? tail) (list1+? (car tail)))
      (let* ([name (xform-sexp->datum (car tail))] [sym (if (symbol? name) name (listname->symbol name))]
             [libform (cons head (cons sym (cdr tail)))] ; head is used as seed id for renamings
             [ic&ex (preprocess-library libform env)] [lid (id-rename-as head sym)])
        ; NB: this is 1/3 of listname->library interface
        (list 'define-library lid (make-library (car ic&ex) (cdr ic&ex))))
      (x-error "improper define-library form" (cons head tail))))

; for now, we have no clear idea of how to process import in all possible contexts, so we will also
; introduce new special <core> form import

(define (xform-import head tail env appos?) ; non-internal
  (if (list? tail)
      (let ([ic&ex (preprocess-import-sets (cons head tail) env)])
        ; NB: this is 1/3 of listname->library interface
        (list 'import (make-library (car ic&ex) (cdr ic&ex))))
      (x-error "improper import form" (cons head tail))))


;---------------------------------------------------------------------------------------------
; String representation of S-expressions and code arguments
;---------------------------------------------------------------------------------------------

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


;---------------------------------------------------------------------------------------------
; Compiler producing serialized code
;---------------------------------------------------------------------------------------------

(define (c-error msg . args)
  (error* (string-append "compiler: " msg) args))

(define (c-warning msg . args)
  (warning* (string-append "compiler: " msg) args))

(define find-free*
  (lambda (x* b)
    (if (null? x*)
        '()
        (set-union 
          (find-free (car x*) b) 
          (find-free* (cdr x*) b)))))

(define find-free
  (lambda (x b)
    (record-case x
      [quote (obj) 
       '()]
      [gref (gid)
       '()]
      [gset! (gid exp)
       (find-free exp b)]
      [(ref const) (id)
       (if (set-member? id b) '() (list id))]
      [set! (id exp)
       (set-union
         (if (set-member? id b) '() (list id))
         (find-free exp b))]
      [set& (id)
       (if (set-member? id b) '() (list id))]
      [lambda (idsi exp)
       (find-free exp (set-union (flatten-idslist idsi) b))]
      [lambda* clauses
       (find-free* (map cadr clauses) b)]
      [letcc (kid exp)
       (find-free exp (set-union (list kid) b))]
      [withcc (kexp exp)
       (set-union (find-free kexp b) (find-free exp b))]
      [if (test then else)
       (set-union
         (find-free test b)
         (set-union (find-free then b) (find-free else b)))]
      [begin exps
       (find-free* exps b)]
      [integrable (ig . args)
       (find-free* args b)]
      [call (exp . args)
       (set-union (find-free exp b) (find-free* args b))]
      [asm (cstr)
       '()]
      [once (gid exp)
       (find-free exp b)]
      [(define define-syntax define-library import) tail
       (c-error "misplaced definition form" x)])))

(define find-sets*
  (lambda (x* v)
    (if (null? x*)
        '()
        (set-union 
          (find-sets (car x*) v) 
          (find-sets* (cdr x*) v)))))

(define find-sets
  (lambda (x v)
    (record-case x
      [quote (obj) 
       '()]
      [gref (gid)
       '()]
      [gset! (gid exp)
       (find-sets exp v)]
      [(ref const) (id)
       '()]
      [set! (id exp)
       (set-union
         (if (set-member? id v) (list id) '())
         (find-sets exp v))]
      [set& (id)
       (if (set-member? id v) (list id) '())]
      [lambda (idsi exp)
       (find-sets exp (set-minus v (flatten-idslist idsi)))]
      [lambda* clauses
       (find-sets* (map cadr clauses) v)]
      [letcc (kid exp)
       (find-sets exp (set-minus v (list kid)))]
      [withcc (kexp exp)
       (set-union (find-sets kexp v) (find-sets exp v))]
      [begin exps
       (find-sets* exps v)]
      [if (test then else)
       (set-union
         (find-sets test v)
         (set-union (find-sets then v) (find-sets else v)))]
      [integrable (ig . args)
       (find-sets* args v)]
      [call (exp . args)
       (set-union (find-sets exp v) (find-sets* args v))]
      [asm (cstr)
       '()]
      [once (gid exp)
       (find-sets exp v)]
      [(define define-syntax define-library import) tail
       (c-error "misplaced definition form" x)])))

(define codegen
  ; x: Scheme Core expression to compile
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
      [if (test then else)
       (codegen test l f s g #f port)
       (write-char #\? port)
       (write-char #\{ port)
       (codegen then l f s g k port)
       (write-char #\} port)
       (cond [k ; tail call: 'then' arm exits, so br around is not needed
              (codegen else l f s g k port)]
             [(equal? else '(begin)) ; non-tail with void 'else' arm
              ] ; no code needed -- ac retains #f from failed test
             [else ; non-tail with 'else' expression; needs br 
              (write-char #\{ port)
              (codegen else l f s g k port)
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
       (c-error "misplaced definition form" x)])))

(define (compile-to-string x)
  (let ([p (open-output-string)])
    (codegen x '() '() '() (find-free x '()) #f p)                   
    (get-output-string p)))

(define (compile-to-thunk-code x)
  (let ([p (open-output-string)])
    (codegen x '() '() '() (find-free x '()) 0 p)                   
    (get-output-string p)))


;---------------------------------------------------------------------------------------------
; Path and file name resolution
;---------------------------------------------------------------------------------------------

(define (path-directory filename)
  (let loop ([l (reverse (string->list filename))])
    (cond [(null? l) ""]
          [(memv (car l) '(#\\ #\/ #\:)) (list->string (reverse l))]
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
   (x-error "circularity in include file chain" filename))
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


;---------------------------------------------------------------------------------------------
; Library names and library file lookup
;---------------------------------------------------------------------------------------------

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

(define (symbol-libname? sym) ; FIXME: shouldn't exist! 
  (let* ([str (symbol->string sym)] [sl (string-length str)])
    (and (< 6 sl) 
         (char=? (string-ref str 0) #\l)
         (char=? (string-ref str 1) #\i)
         (char=? (string-ref str 2) #\b)
         (char=? (string-ref str 3) #\:)
         (char=? (string-ref str 4) #\/)
         (char=? (string-ref str 5) #\/)
         (substring str 6 sl))))


; hacks for locating library files

(define *library-path-list* '("./"))

(define (add-library-path! path)
  (if (base-path-separator path)
      (set! *library-path-list* (append *library-path-list* (list path)))
      (c-error "library path should end in directory separator" path))) 

(define (find-library-path listname) ;=> name of existing .sld file or #f
  (let loop ([l *library-path-list*])
    (and (pair? l)
         (let ([p (listname->path listname (car l) ".sld")]) 
           (if (and p (file-exists? p)) p (loop (cdr l)))))))

(define (read-file-sexps filepath ci?)
  (call-with-input-file filepath
    (lambda (port) 
      (when ci? (set-port-fold-case! port #t))
      (let loop ([sexps '()])
        (let ([s (read-code-sexp port)])
          (if (eof-object? s)
              (reverse! sexps)
              (loop (cons s sexps))))))))

(define (library-available? lib) ;=> #f | filepath (external) | (code . eal) (loaded)
  (cond [(string? lib) (file-resolve-relative-to-current lib)]
        [(library-info lib #f)] ; builtin or preloaded
        [else (and (list1+? lib) (find-library-path lib))]))  ;(or (symbol? lib) (list1+? lib))

; name prefixes

(define (fully-qualified-library-prefixed-name lib id)
  (symbol-append (if (symbol? lib) lib (listname->symbol lib)) '? id))


;---------------------------------------------------------------------------------------------
; Environments
;---------------------------------------------------------------------------------------------

; new lookup procedure for explicit macro environments

(define (env-lookup id env at) ;=> location (| #f)
  (if (procedure? id)
      ; nonsymbolic ids can't be (re)defined
      (case at [(ref set!) (old-den id)] [else #f])
      (let loop ([env env])
        (cond [(pair? env) ; imported: ref-only
               (if (eq? (caar env) id)
                   (case at [(ref) (cdar env)] [else #f])
                   (loop (cdr env)))]
              [(vector? env) ; extendable, keeps imported? flags
               (let* ([n (vector-length env)] [i (immediate-hash id n)]
                      [al (vector-ref env i)] [p (assq id al)])
                 (if p ; p is (key loc imported?)
                     (case at 
                       [(ref) (cadr p)] 
                       [else (if (caddr p) #f (cadr p))]) ; imported can be ref-d only 
                     ; implicitly/on demand append integrables and "naked" globals
                     (let ([loc (make-location (or (lookup-integrable id) (list 'ref id)))])
                       (vector-set! env i (cons (list id loc #f) al)) ; not imported
                       loc)))]
              [(string? env) ; module prefix = module internals: full access
               (and (memq at '(ref set! define define-syntax))
                 (let ([gid (string->symbol (string-append env (symbol->string id)))])
                   (env-lookup gid *root-environment* 'ref)))]
              [(procedure? env)
               (env id at)]
              [else ; finite env 
               #f]))))


; make explicit root environment (a vector) and fill it

(define *root-environment*
  (make-vector 101 '())) ; use prime number

(define (define-in-root-environment! name loc imported?)
  (let* ([env *root-environment*] [n (vector-length env)]
         [i (immediate-hash name n)] [al (vector-ref env i)] 
         [p (assq name al)])
    (if p 
        (begin (set-car! (cdr p) loc) (set-car! (cddr p) imported?))
        (vector-set! env i (cons (list name loc imported?) al)))))

; put handmade ones first! 

(define-in-root-environment! 'include 
  (make-location (make-include-transformer #f)) #t)

(define-in-root-environment! 'include-ci 
  (make-location (make-include-transformer #t)) #t)

(define-in-root-environment! 'cond-expand 
  (make-location (make-cond-expand-transformer)) #t)

; now put the builtins (lazily) and others

(let ([put! (lambda (k loc) (define-in-root-environment! k loc #t))]) 
  (let loop ([l (initial-transformers)])
    (if (null? l) 'ok
        (let ([p (car l)] [l (cdr l)])
          (let ([k (car p)] [v (cdr p)])
            (cond
              [(or (symbol? v) (number? v)) 
               (put! k (make-location v))
               (loop l)]
              [(and (pair? v) (eq? (car v) 'syntax-rules))
               (body
                 (define (sr-env id at) 
                   (env-lookup id *root-environment* at))
                 (define sr-v
                   (if (id? (cadr v))
                       (syntax-rules* sr-env (cadr v) (caddr v) (cdddr v))
                       (syntax-rules* sr-env #f (cadr v) (cddr v))))
                 (put! k (make-location sr-v))
                 (loop l))]))))))

(define (root-environment id at)
  (env-lookup id *root-environment* at))

;---------------------------------------------------------------------------------------------
; Symbol-name identifiers registry and built-in syntax values
;---------------------------------------------------------------------------------------------

; name registries are htables (vectors of prime+1 length) of alists ((sym . <location>) ...)
(define *root-name-registry* (make-vector 102 '())) 

(define (name-lookup nr name mkdefval) ;=> loc | #f
  (let* ([n-1 (- (vector-length nr) 1)] [i (if (pair? name) n-1 (immediate-hash name n-1))] 
         [al (vector-ref nr i)] [p (if (pair? name) (assoc name al) (assq name al))])
    (cond [p (cdr p)]
          [mkdefval
           (let ([loc (make-location (mkdefval name))]) 
             (vector-set! nr i (cons (cons name loc) al))
             loc)]
          [else #f])))

; register standard libraries as well as all-encompassing (repl) library
(for-each
  (lambda (r)
    (define (key->listname k)
      (case k
        [(w) '(scheme write)]   [(t) '(scheme time)]  [(p) '(scheme repl)]
        [(r) '(scheme read)]    [(v) '(scheme r5rs)]  [(u) '(scheme r5rs-null)] 
        [(d) '(scheme load)]    [(z) '(scheme lazy)]  [(s) '(scheme process-context)]
        [(i) '(scheme inexact)] [(f) '(scheme file)]  [(e) '(scheme eval)]
        [(o) '(scheme complex)] [(h) '(scheme char)]  [(l) '(scheme case-lambda)]
        [(x) '(scheme cxr)]     [(b) '(scheme base)]))
    (define (get-library! listname) ;=> <library> 
      (location-val 
        (name-lookup *root-name-registry* listname 
          (lambda (ln) 
            (let ([l (make-library '(begin) '())])
              ; for now, mirror libraries in old registry too... FIXME!
              (define-in-root-environment! (listname->symbol listname) (make-location l) #t)
              l)))))
    (define (put-loc! library k loc)
      (let* ([eal (library-exports library)] [p (assq k eal)])
        (cond [p (set-cdr! p loc)] 
              [else (library-set-exports! library (cons (cons k loc) eal))])))
    (let loop ([name (car r)] [keys (cdr r)])
      (cond [(null? keys) ; all go to (repl)
             (put-loc! (get-library! '(repl)) name (root-environment name 'ref))]
            [else
             (put-loc! (get-library! (key->listname (car keys))) name (root-environment name 'ref))
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
    (eof-object b) (eof-object? v b) (eq? v b) (equal? v b) (eqv? v b) (error b)
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
    (make-rectangular v o) (real-part v o) (caaar v x) (caadr v x) (cadar v x) (caddr v x) (cdaar v x)
    (cdadr v x) (cddar v x) (cdddr v x) (caaaar v x) (caaadr v x) (caadar v x) (caaddr v x) (cadaar v x)
    (cadadr v x) (caddar v x) (cadddr v x) (cdaaar v x) (cdaadr v x) (cdadar v x) (cdaddr v x)
    (cddaar v x) (cddadr v x) (cdddar v x) (cddddr v x) (environment e) (eval v e)
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
    ; skint extras go into (repl) only - not to be confused with (scheme repl)
    (box?) (box) (unbox) (set-box!)
    ))


;---------------------------------------------------------------------------------------------
; Evaluation
;---------------------------------------------------------------------------------------------


;---------------------------------------------------------------------------------------------
; REPL
;---------------------------------------------------------------------------------------------

(define *verbose* #f)

(define (repl-environment id at) ; FIXME: need to happen in a "repl." namespace
  (env-lookup id *root-environment* at))

(define (repl-compile-and-run-core-expr core)
  (when *verbose* (display "TRANSFORM =>") (newline) (write core) (newline))
  (unless (val-core? core) (x-error "unexpected transformed output" core))
  (let ([code (compile-to-thunk-code core)] [start #f])
    (when *verbose* 
      (display "COMPILE-TO-STRING =>") (newline) (display code) (newline)
      (display "DECODE+EXECUTE =>") (newline)
      (set! start (current-jiffy)))
    (let* ([cl (closure (deserialize-code code))] [res (cl)])
      (when *verbose* 
        (display "Elapsed time: ") (write (* 1000 (/ (- (current-jiffy) start) (jiffies-per-second))))
        (display " ms.") (newline))
      (unless (eq? res (void)) (write res) (newline)))))      

(define (repl-eval-top-form x env)
  (if (pair? x)
      (let ([hval (xform #t (car x) env)]) ; returns <core>
        (cond
          [(eq? hval 'begin) ; splice
           (let loop ([x* (cdr x)])
             (when (pair? x*) 
               (repl-eval-top-form (car x*) env)
               (loop (cdr x*))))]
          [(and (eq? hval 'define) (null? (cadr x))) ; special idless define
           (repl-eval-top-form (caddr x) env)]
          [(eq? hval 'define) ; use new protocol for top-level envs
           (let* ([core (xform-define (cdr x) env)]
                  [loc (xenv-lookup env (cadr core) 'define)])
             (if (and loc (sexp-match? '(ref *) (location-val loc)))
                 (repl-compile-and-run-core-expr 
                   (list 'set! (cadr (location-val loc)) (caddr core)))
                 (x-error "identifier cannot be (re)defined as variable in env:" 
                   (cadr core) env)))]
          [(eq? hval 'define-syntax) ; use new protocol for top-level envs
           (let* ([core (xform-define-syntax (cdr x) env)]
                  [loc (xenv-lookup env (cadr core) 'define-syntax)])
             (if loc ; location or #f
                 (location-set-val! loc (caddr core))
                 (x-error "identifier cannot be (re)defined as syntax in env:"
                   (cadr core) env))
             (when *verbose* (display "SYNTAX INSTALLED: ") (write (cadr core)) (newline)))]
          [(eq? hval 'define-library) ; use new protocol for top-level envs
           (let* ([core (xform-define-library (car x) (cdr x) env #f)]
                  [loc (xenv-lookup env (cadr core) 'define-syntax)])
             (if loc ; location or #f
                 (let ([l (caddr core)]) (location-set-val! loc l))
                 (x-error "identifier cannot be (re)defined as syntax in env:"
                   (cadr core) env))
             (when *verbose* (display "LIBRARY INSTALLED: ") (write (cadr core)) (newline)))]
          [(eq? hval 'import) ; splice as definitions
           (let* ([core (xform-import (car x) (cdr x) env #f)] ; core is (import <library>)
                  [l (cadr core)] [code (library-code l)] [eal (library-exports l)])
             (define (define-alias p) 
               (repl-eval-top-form ; FIXME: this is not optimal -- too much fuss
                 (list define-syntax-id (car p) (list syntax-quote-id (location-val (cdr p)))) env))
             (repl-compile-and-run-core-expr code)
             (for-each define-alias eal))]
          [(val-transformer? hval) ; apply transformer and loop
           (repl-eval-top-form (hval x env) env)]
          [(val-integrable? hval) ; integrable application
           (repl-compile-and-run-core-expr (xform-integrable hval (cdr x) env))]
          [(val-builtin? hval) ; other builtins
           (repl-compile-and-run-core-expr (xform #f x env))]
          [else ; regular call
           (repl-compile-and-run-core-expr (xform-call hval (cdr x) env))]))
      ; var refs and literals
      (repl-compile-and-run-core-expr (xform #f x env))))

(define (repl-read iport prompt)
  (when prompt (newline) (display prompt) (display " "))
  (read-code-sexp iport))

(define (repl-from-port iport env prompt)
  (define cfs (current-file-stack))
  (guard (err 
          [(error-object? err)
           (let ([p (current-error-port)])
            (display (error-object-message err) p) (newline p)
            (for-each (lambda (arg) (write arg p) (newline p)) 
              (error-object-irritants err)))
           (set-current-file-stack! cfs)
           (when prompt (repl-from-port iport env prompt))]
          [else 
           (let ([p (current-error-port)])
             (display "Unknown error:" p) (newline p)
             (write err p) (newline p))
           (set-current-file-stack! cfs)
           (when prompt (repl-from-port iport env prompt))])
    (let loop ([x (repl-read iport prompt)])
      (unless (eof-object? x)
        (repl-eval-top-form x env)
        (loop (repl-read iport prompt))))))

(define (repl-file fname env)
  (define iport (open-input-file fname))
  (repl-from-port iport env #f)
  (close-input-port iport))

(define (benchmark-file fname)
  (define iport (open-input-file fname))
  (unless (sexp-match? '(load "libl.sf") (read-code-sexp iport))
    (error "unexpected benchmark file format" fname))
  (repl-from-port iport repl-environment #f)
  (repl-eval-top-form '(main #f) repl-environment)  
  (close-input-port iport))

(define (run-repl)
  (set-current-file-stack! '())
  (repl-from-port 
    (current-input-port) 
    repl-environment
    "skint]"))


