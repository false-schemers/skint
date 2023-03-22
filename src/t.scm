
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
    [(record-case id [key ids exp ...] clause ...)
     (if (eq? (car id) 'key)
         (apply (lambda ids exp ...) (cdr id))
         (record-case id clause ...))]))

(define syntax-match?
  (lambda (pat exp)
    (or (eq? pat '*)
        (equal? exp pat)
        (and (pair? pat)
             (cond
              [(and (eq? (car pat) '$)
                    (pair? (cdr pat))
                    (null? (cddr pat)))
               (eq? exp (cadr pat))]
              [(and (pair? (cdr pat))
                    (eq? (cadr pat) '...)
                    (null? (cddr pat)))
               (let ([pat (car pat)])
                 (define (f lst)
                   (or (null? lst)
                       (and (pair? lst)
                            (syntax-match? pat (car lst))
                            (f (cdr lst)))))  
                 (f exp))]
              [else
               (and (pair? exp)
                    (syntax-match? (car pat) (car exp))
                    (syntax-match? (cdr pat) (cdr exp)))])))))

; unique symbol generator (poor man's version)
(define gensym
  (let ([gsc 0])
    (lambda args ; (), (symbol), or (#f) for gsc reset
      (set! gsc (fx+ gsc 1))
      (if (null? args)
          (string->symbol 
            (string-append "#" (fixnum->string gsc 10)))
          (if (symbol? (car args))
              (string->symbol 
                (string-append (symbol->string (car args))
                  (string-append "#" (fixnum->string gsc 10))))
              (set! gsc 0))))))

(define posq
  (lambda (x l)
    (let loop ([l l] [n 0])
      (cond [(null? l) #f]
            [(eq? x (car l)) n]
            [else (loop (cdr l) (fx+ n 1))]))))

(define list-diff
  (lambda (l t)
    (if (or (null? l) (eq? l t))
        '()
        (cons (car l) (list-diff (cdr l) t)))))

(define (pair* x . more)
  (let loop ([x x] [rest more])
    (if (null? rest) x 
        (cons x (loop (car rest) (cdr rest))))))

(define (andmap p l)
  (if (pair? l) (and (p (car l)) (andmap p (cdr l))) #t))

(define (list1? x) (and (pair? x) (null? (cdr x))))
(define (list1+? x) (and (pair? x) (list? (cdr x))))
(define (list2? x) (and (pair? x) (list1? (cdr x))))
(define (list2+? x) (and (pair? x) (list1+? (cdr x))))


;---------------------------------------------------------------------------------------------
; Syntax of the Scheme Core language
;---------------------------------------------------------------------------------------------

;  <core> -> (quote <object>)
;  <core> -> (ref <id>)
;  <core> -> (set! <id> <core>)
;  <core> -> (set& <id>)
;  <core> -> (lambda <ids> <core>) where <ids> -> (<id> ...) | (<id> ... . <id>) | <id>
;  <core> -> (lambda* (<arity> <core>) ...) where <arity> -> (<cnt> <rest?>) 
;  <core> -> (syntax-lambda (<id> ...) <core>)
;  <core> -> (letcc <id> <core>) 
;  <core> -> (withcc <core> <core>) 
;  <core> -> (begin <core> ...)
;  <core> -> (if <core> <core> <core>)
;  <core> -> (call <core> <core> ...) 
;  <core> -> (integrable <ig> <core> ...) where <ig> is an index in the integrables table

;  NB: (begin) is legit, returns unspecified value
;  on top level, these two extra core forms are legal:

;  <core> -> (define <id> <core>)
;  <core> -> (define-syntax <id> <transformer>)

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

; An environment is a procedure that accepts any identifier and
; returns a denotation.  The denotation of an unbound identifier is
; its name (as a symbol).  A bound identifier's denotation is its
; binding, which is a pair of the current value and the identifier's 
; name (needed by quote). Biding's value can be changed later.

; Special forms are either a symbol naming a builtin, or a transformer procedure 
; that takes two arguments: a macro use and the environment of the macro use.

; <identifier>  ->  <symbol> | <thunk returning den>
; <denotation>  ->  <symbol> | <binding>
; <binding>     ->  (<symbol> . <value>)
; <value>       ->  <special> | <core>
; <special>     ->  <builtin> | <transformer>
; <builtin>     ->  syntax | quote | set! | set& | begin | if | lambda | 
;                   lambda* | syntax-lambda | letcc | withcc | body | 
;                   define | define-syntax ; top-level only
; <transformer> ->  <procedure of exp and env returning exp>

(define        val-core?                  pair?)
(define        (val-special? val)         (not (pair? val)))

(define        binding?                   pair?)
(define        make-binding               cons)
(define        binding-val                cdr)
(define        (binding-special? bnd)     (val-special? (cdr bnd)))
(define        binding-sym                car)
(define        binding-set-val!           set-cdr!)
(define        find-top-binding           assq)

(define (new-id den)               (define p (list den)) (lambda () p))
(define (old-den id)               (car (id)))
(define (id? x)                    (or (symbol? x) (procedure? x)))
(define (id->sym id)               (if (symbol? id) id (den->sym (old-den id))))
(define (den->sym den)             (if (symbol? den) den (binding-sym den)))

(define (empty-xenv id)            (if (symbol? id) id (old-den id)))
(define (extend-xenv env id bnd)   (lambda (i) (if (eq? id i) bnd (env i))))

(define (add-binding key val env) ; adds as-is     
  (extend-xenv env key (make-binding (id->sym key) val)))

(define (add-var var val env) ; adds renamed var as <core>
  (extend-xenv env var (make-binding (id->sym var) (list 'ref val))))

(define (x-error msg . args)
  (error* (string-append "transformer: " msg) args))

; xform receives Scheme s-expressions and returns either Core Scheme <core>
; (always a pair) or special-form, which is either a builtin (a symbol) or
; a transformer (a procedure). Appos? flag is true when the context can
; allow xform to return a transformer; otherwise, only <core> is accepted. 

(define (xform appos? sexp env)
  (cond [(id? sexp) 
         (let ([hval (xform-ref sexp env)])
           (cond [appos? hval]
                 [(integrable? hval) ; integrable id-syntax
                  (list 'ref (integrable-global hval))]
                 [(procedure? hval) ; id-syntax
                  (xform appos? (hval sexp env) env)]
                 [(not (pair? hval)) 
                  (x-error "improper use of syntax form" hval)]
                 [else hval]))]
        [(not (pair? sexp))
         (xform-quote (list sexp) env)]
        [else 
         (let* ([head (car sexp)] [tail (cdr sexp)] [hval (xform #t head env)])
           (case hval
             [(syntax)        (car tail)] ; internal use only
             [(quote)         (xform-quote         tail env)]
             [(set!)          (xform-set!          tail env)]
             [(set&)          (xform-set&          tail env)]
             [(begin)         (xform-begin         tail env)]
             [(if)            (xform-if            tail env)]
             [(lambda)        (xform-lambda        tail env)]
             [(lambda*)       (xform-lambda*       tail env)]
             [(syntax-lambda) (xform-syntax-lambda tail env)]
             [(letcc)         (xform-letcc         tail env)]
             [(withcc)        (xform-withcc        tail env)]
             [(body)          (xform-body          tail env)]
             [(define)        (xform-define        tail env)]
             [(define-syntax) (xform-define-syntax tail env)]
             [else            (if (integrable? hval)
                                  (xform-integrable hval tail env)
                                  (if (procedure? hval)
                                      (xform appos? (hval sexp env) env)
                                      (xform-call hval tail env)))]))]))

(define (xform-ref id env)
  (let ([den (env id)])
    (cond [(symbol? den) (list 'ref den)]
          [else (binding-val den)])))

(define (xform-quote tail env)
  (if (list1? tail)
      (list 'quote 
        (let conv ([sexp (car tail)])
          (cond [(id? sexp) (id->sym sexp)]
                [(pair? sexp) (cons (conv (car sexp)) (conv (cdr sexp)))]
                [(vector? sexp) (list->vector (map conv (vector->list sexp)))]
                [else sexp])))
      (x-error "improper quote form" (cons 'quote tail))))

(define (xform-set! tail env)
  (if (and (list2? tail) (id? (car tail)))
      (let ([den (env (car tail))] [xexp (xform #f (cadr tail) env)])
        (cond [(symbol? den) (list 'set! den xexp)]
              [(binding-special? den) (binding-set-val! den xexp) '(begin)]
              [else (let ([val (binding-val den)])
                      (if (eq? (car val) 'ref)
                          (list 'set! (cadr val) xexp)
                          (x-error "set! to non-identifier form")))]))
      (x-error "improper set! form" (cons 'set! tail))))

(define (xform-set& tail env)
  (if (list1? tail)
      (let ([den (env (car tail))])      
        (cond [(symbol? den) (list 'set& den)]
              [(binding-special? den) (x-error "set& of a non-variable")]
              [else (let ([val (binding-val den)])
                      (if (eq? (car val) 'ref)
                          (list 'set& (cadr val))
                          (x-error "set& of a non-variable")))]))
      (x-error "improper set& form" (cons 'set& tail))))

(define (xform-begin tail env)
  (if (list? tail)
      (let ([xexps (map (lambda (sexp) (xform #f sexp env)) tail)])
        (if (and (pair? xexps) (null? (cdr xexps)))
            (car xexps) ; (begin x) => x
            (cons 'begin xexps)))
      (x-error "improper begin form" (cons 'begin! tail))))

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
    [(#\u) (<= 0 n 1)] [(#\b) (<= 1 n 2)]
    [(#\#) (>= n 0)]   [(#\@) #f]
    [else #f])) 

(define (xform-integrable ig tail env)
  (if (integrable-argc-match? (integrable-type ig) (length tail))
      (cons 'integrable (cons ig (map (lambda (sexp) (xform #f sexp env)) tail)))
      (xform-call (list 'ref (integrable-global ig)) tail env)))

(define (xform-lambda tail env)
  (if (and (list1+? tail) (idslist? (car tail)))
      (let loop ([vars (car tail)] [ienv env] [ipars '()])
        (cond [(pair? vars)
               (let* ([var (car vars)] [nvar (gensym (id->sym var))])
                 (loop (cdr vars) (add-var var nvar ienv) (cons nvar ipars)))]
              [(null? vars)
               (list 'lambda (reverse ipars) (xform-body (cdr tail) ienv))]
              [else ; improper 
               (let* ([var vars] [nvar (gensym (id->sym var))] 
                      [ienv (add-var var nvar ienv)])
                 (list 'lambda (append (reverse ipars) nvar)
                   (xform-body (cdr tail) ienv)))]))
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

(define (xform-syntax-lambda tail env)
  (if (and (list2+? tail) (andmap id? (car tail)))
      (let ([vars (car tail)] [macenv env] [forms (cdr tail)])
        ; return a transformer that wraps xformed body in (syntax ...)
        (lambda (use useenv)
          (if (and (list1+? use) (fx=? (length vars) (length (cdr use))))
              (let loop ([vars vars] [exps (cdr use)] [env macenv])
                (if (null? vars)
                    (list 'syntax (xform-body forms env))
                    (loop (cdr vars) (cdr exps)
                      (add-binding (car vars) 
                        (xform #t (car exps) useenv) env))))  
              (x-error "invalif syntax-lambda application" use))))  
      (x-error "improper syntax-lambda body" (cons 'syntax-lambda tail))))

(define (xform-letcc tail env)
  (if (and (list2+? tail) (id? (car tail)))
      (let* ([var (car tail)] [nvar (gensym (id->sym var))])
        (list 'letcc nvar 
          (xform-body (cdr tail) (add-var var nvar env))))
      (x-error "improper letcc form" (cons 'letcc tail))))

(define (xform-withcc tail env)
  (if (list2+? tail)
      (list 'withcc (xform #f (car tail) env)
        (xform-body (cdr tail) env))
      (x-error "improper withcc form" (cons 'withcc tail))))

(define (xform-body tail env)
  (if (null? tail)
      (list 'begin)
      (let loop ([env env] [ids '()] [inits '()] [nids '()] [body tail])
        (if (and (pair? body) (pair? (car body)))
            (let ([first (car body)] [rest (cdr body)])
              (let* ([head (car first)] [hval (xform #t head env)])
                (case hval
                  [(begin)
                   (loop env ids inits nids (append (cdr first) rest))]
                  [(define)
                   (let* ([id (cadr first)] [init (caddr first)]
                          [nid (gensym (id->sym id))] [env (add-var id nid env)])
                     (loop env (cons id ids) (cons init inits) (cons nid nids) rest))]
                  [(define-syntax)
                   (let* ([id (cadr first)] [init (caddr first)]
                          [env (add-binding id '(undefined) env)])
                     (loop env (cons id ids) (cons init inits) (cons #t nids) rest))]
                  [else
                   (if (procedure? hval)
                       (loop env ids inits nids (cons (hval first env) rest))
                       (xform-labels (reverse ids) (reverse inits) (reverse nids) body env))])))
            (xform-labels (reverse ids) (reverse inits) (reverse nids) body env)))))

(define (xform-labels ids inits nids body env)
  (let loop ([ids ids] [inits inits] [nids nids] [sets '()] [lids '()])
    (cond [(null? ids) 
           (let* ([xexps (append (reverse sets)
                           (map (lambda (sexp) (xform #f sexp env)) body))]
                  [xexp (if (and (pair? xexps) (null? (cdr xexps)))
                            (car xexps)
                            (cons 'begin xexps))])
             (if (null? lids)
                 xexp
                 (pair* 'call (list 'lambda (reverse lids) xexp)
                   (map (lambda (lid) '(begin)) lids))))]
          [(symbol? (car nids)) ; define
           (loop (cdr ids) (cdr inits) (cdr nids)
             (cons (xform-set! (list (car ids) (car inits)) env) sets)
             (cons (car nids) lids))]
          [else ; define-syntax
           (binding-set-val! (env (car ids)) (xform #t (car inits) env))
           (loop (cdr ids) (cdr inits) (cdr nids) sets lids)])))

(define (xform-define tail env) ; top-level only
  (if (and (list2? tail) (id? (car tail)))
      (list 'define (id->sym (car tail)) (xform #f (cadr tail) env))
      (x-error "improper define form" (cons 'define tail))))

(define (xform-define-syntax tail env) ; top-level only
  (if (and (list2? tail) (id? (car tail)))
      (list 'define-syntax (id->sym (car tail)) (xform #t (cadr tail) env))
      (x-error "improper define-syntax form" (cons 'define-syntax tail))))


; ellipsis denotation is used for comparisons only

(define denotation-of-default-ellipsis 
  (make-binding '... (lambda (sexp env) (x-error "improper use of ..." sexp))))

(define *transformers* 
  (list
    (make-binding 'syntax 'syntax) 
    (make-binding 'define 'define)
    (make-binding 'define-syntax 'define-syntax)
    (make-binding 'quote 'quote)
    (make-binding 'set! 'set!)
    (make-binding 'set& 'set&)
    (make-binding 'lambda 'lambda)
    (make-binding 'lambda* 'lambda*)
    (make-binding 'syntax-lambda 'syntax-lambda)
    (make-binding 'letcc 'letcc)
    (make-binding 'withcc 'withcc)
    (make-binding 'begin 'begin)
    (make-binding 'if 'if)
    (make-binding 'body 'body)
    denotation-of-default-ellipsis))

(define (top-transformer-env id)
  (let ([bnd (find-top-binding id *transformers*)])
    (cond [(binding? bnd)
           ; special case: syntax-rules in sexp form (left by init)
           (let ([val (binding-val bnd)])
             (if (and (pair? val) (eq? (car val) 'syntax-rules))  
                 (binding-set-val! bnd (transform #t val))))
           bnd]
          [(symbol? id)
           (let ([bnd (make-binding id (or (lookup-integrable id) (list 'ref id)))])
             (set! *transformers* (cons bnd *transformers*))
             bnd)]
          [else (old-den id)])))

(define (install-transformer! s t)
  (binding-set-val! (top-transformer-env s) t))

(define (install-transformer-rules! s ell lits rules)
  (install-transformer! s
    (syntax-rules* top-transformer-env ell lits rules)))

(define (transform appos? sexp . optenv)
  ; (gensym #f) ; reset gs counter to make results reproducible
  (xform appos? sexp (if (null? optenv) top-transformer-env (car optenv))))


; 'syntax-rules' transformer produces another transformer from the rules

(define (syntax-rules* mac-env ellipsis pat-literals rules)
  (define (pat-literal? id) (memq id pat-literals))
  (define (not-pat-literal? id) (not (pat-literal? id)))
  (define (ellipsis-pair? x)
    (and (pair? x) (ellipsis? (car x))))
  (define (ellipsis? x)
    (if ellipsis
        (eq? x ellipsis)
        (and (id? x) (eq? (mac-env x) denotation-of-default-ellipsis))))

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
                  (continue-if (and (id? sexp) (eq? (use-env sexp) (mac-env pat))))
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
      (map (lambda (id) (cons id (new-id (mac-env id))))
           (list-ids tmpl #t
             (lambda (id) (not (assq id top-bindings))))))

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

(install-transformer! 'syntax-rules
  (lambda (sexp env)
    (define syntax-id (new-id (make-binding 'syntax 'syntax)))
    ; sexp can be either
    (if (id? (cadr sexp))
        ; (_ ellipsis (litname ...) . rules)
        (list syntax-id (syntax-rules* env (cadr sexp) (caddr sexp) (cdddr sexp)))
        ; or (_ (litname ...) . rules)
        (list syntax-id (syntax-rules* env #f (cadr sexp) (cddr sexp))))))

; non-recursive transformer for define relies on old definition

(install-transformer! 'define
  (let ([env (add-binding 'define 'define top-transformer-env)])
    (syntax-rules* env #f '() '(
      [(_ (name . args) . forms)
       (define name (lambda args . forms))]
      [(_ name exp)
       (define name exp)]))))

; Remaining transformers are made with the help of syntax-rules*
; NB: order of installation is important -- each transformer can
; be self-recursive but can't use transformers defined later!

(define-syntax install-sr-transformer!
  (syntax-rules (quote syntax-rules)
    [(_ 'name (syntax-rules (lit ...) . rules))
     (install-transformer-rules! 'name #f '(lit ...) 'rules)]
    [(_ 'name (syntax-rules ellipsis (lit ...) . rules))
     (install-transformer-rules! 'name 'ellipsis '(lit ...) 'rules)]))    

(install-sr-transformer! 'letrec-syntax
  (syntax-rules ()
    [(_ ([key trans] ...) . forms) ; non-splicing!
     (body (define-syntax key trans) ... . forms)]))

(install-sr-transformer! 'let-syntax
  (syntax-rules ()
    [(_ () . forms) 
     (body . forms)]
    [(_ ([key trans] ...) . forms)
     ((syntax-lambda (key ...) . forms) trans ...)]))

(install-sr-transformer! 'letrec
  (syntax-rules ()
    [(_ ([var init] ...) . forms)
     (body (define var init) ... . forms)]))

(install-sr-transformer! 'let
  (syntax-rules ()
    [(_ ([var init] ...) . forms)
     ((lambda (var ...) . forms) init ...)]
    [(_ name ([var init] ...) . forms)
     ((letrec ((name (lambda (var ...) . forms))) name) init ...)]))

(install-sr-transformer! 'let*
  (syntax-rules ()
    [(_ () . forms) 
     (body . forms)]
    [(_ (first . more) . forms)
     (let (first) (let* more . forms))]))

(install-sr-transformer! 'and
  (syntax-rules ()
    [(_) #t]
    [(_ test) test]
    [(_ test . tests) (if test (and . tests) #f)]))

(install-sr-transformer! 'or
  (syntax-rules ()
    [(_) #f]
    [(_ test) test]
    [(_ test . tests) (let ([x test]) (if x x (or . tests)))]))

(install-sr-transformer! 'cond
  (syntax-rules (else =>)
    [(_) #f]
    [(_ (else . exps)) (begin . exps)]
    [(_ (x) . rest) (or x (cond . rest))]
    [(_ (x => proc) . rest) (let ([tmp x]) (cond [tmp (proc tmp)] . rest))]
    [(_ (x . exps) . rest) (if x (begin . exps) (cond . rest))]))

(install-sr-transformer! 'case-test
  (syntax-rules (else) 
    [(_ k else) #t]
    [(_ k atoms) (memv k 'atoms)]))

(install-sr-transformer! 'case
  (syntax-rules ()
    [(_ x (test . exprs) ...)
     (let ([key x]) (cond ((case-test key test) . exprs) ...))]))

(install-sr-transformer! 'do
  (syntax-rules ()
    [(_ ((var init . step) ...) ending expr ...)
     (let loop ([var init] ...)
       (cond ending [else expr ... (loop (begin var . step) ...)]))]))

(install-sr-transformer! 'quasiquote
  (syntax-rules (unquote unquote-splicing quasiquote)
    [(_ ,x) x]
    [(_ (,@x . y)) (append x `y)]
    [(_ `x . d) (cons 'quasiquote (quasiquote (x) d))]
    [(_ ,x   d) (cons 'unquote (quasiquote (x) . d))]
    [(_ ,@x  d) (cons 'unquote-splicing (quasiquote (x) . d))]
    [(_ (x . y) . d) (cons (quasiquote x . d) (quasiquote y . d))]
    [(_ #(x ...) . d) (list->vector (quasiquote (x ...) . d))]
    [(_ x . d) 'x]))

(install-sr-transformer! 'when
  (syntax-rules ()
    [(_ test . rest) (if test (begin . rest))]))

(install-sr-transformer! 'unless
  (syntax-rules ()
    [(_ test . rest) (if (not test) (begin . rest))]))

(install-sr-transformer! 'case-lambda
  (syntax-rules ()
    [(_ [args . body] ...) (lambda* [args (lambda args . body)] ...)]))
