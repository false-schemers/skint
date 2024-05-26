
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

;  These names are bound to specials never returned by xform:

;  (syntax <value>)
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

; An environment is a procedure that accepts any identifier and returns a denotation.
; The denotation of an identifier is its macro location, which is a cell storing the 
; identifier's current syntactic value. Location's value can be changed later.

; Special forms are either a symbol naming a builtin, or a transformer procedure 
; that takes two arguments: a macro use and the environment of the macro use.

; <identifier>  ->  <symbol> | <thunk returning (sym . den)>
; <denotation>  ->  <location>
; <location>    ->  #&<value>
; <value>       ->  <special> | <core>
; <special>     ->  <builtin> | <integrable> | <transformer>
; <builtin>     ->  syntax | quote | set! | set& | if | lambda | lambda* |
;                   letcc | withcc | body | begin | define | define-syntax |
;                   syntax-lambda | syntax-rules | syntax-length | syntax-error
; <integrable>  ->  <fixnum serving as index in internal integrables table>
; <transformer> ->  <procedure of exp and env returning exp>

(define-syntax  val-core?          pair?)

(define-syntax  make-location      box)
(define-syntax  location-val       unbox)
(define-syntax  location-set-val!  set-box!)

(define (location-special? l)      (not (pair? (unbox l))))
(define (new-id sym den)           (define p (cons sym den)) (lambda () p))
(define (old-sym id)               (car (id)))
(define (old-den id)               (cdr (id)))
(define (id? x)                    (or (symbol? x) (procedure? x)))
(define (id->sym id)               (if (symbol? id) id (old-sym id)))

; Expand-time environments map identifiers (symbolic or thunked) to denotations, i.e. locations
; containing either a <special> or a <core> value. In normal case, <core> value is (ref <gid>),
; where <gid> is a key in run-time store, aka *globals*. Environments should allocate new locations
; as needed, so every identifier gets mapped to one. Expand-time environments are represented as
; one-argument procedures.

(define (extend-xenv env id bnd)   (lambda (i) (if (eq? id i) bnd (env i))))

(define (add-location key val env) ; adds as-is     
  (extend-xenv env key (make-location val)))

(define (add-var var val env) ; adds renamed var as <core>
  (extend-xenv env var (make-location (list 'ref val))))

(define (xform-sexp->datum sexp)
  (let conv ([sexp sexp])
    (cond [(id? sexp) (id->sym sexp)]
          [(pair? sexp) (cons (conv (car sexp)) (conv (cdr sexp)))]
          [(vector? sexp) (list->vector (map conv (vector->list sexp)))]
          [else sexp])))

(define (x-error msg . args)
  (error* (string-append "transformer: " msg) args))

; xform receives Scheme s-expressions and returns either Core Scheme <core>
; (always a pair) or special-form, which is either a builtin (a symbol) or
; a transformer (a procedure). Appos? flag is true when the context can
; allow xform to return a special; otherwise, only <core> is returned. 

(define (xform appos? sexp env)
  (cond [(id? sexp) 
         (let ([hval (xform-ref sexp env)])
           (cond [appos? hval]
                 [(integrable? hval) ; integrable id-syntax
                  (list 'ref (integrable-global hval))]
                 [(procedure? hval) ; id-syntax
                  (xform appos? (hval sexp env) env)]
                 [(not (pair? hval)) ; special used out of context 
                  (x-error "improper use of syntax form" hval)]
                 [else hval]))] ; core 
        [(not (pair? sexp))
         (xform-quote (list sexp) env)]
        [else 
         (let* ([head (car sexp)] [tail (cdr sexp)] [hval (xform #t head env)])
           (case hval
             [(syntax)        (car tail)] ; internal use only
             [(quote)         (xform-quote         tail env)]
             [(set!)          (xform-set!          tail env)]
             [(set&)          (xform-set&          tail env)]
             [(if)            (xform-if            tail env)]
             [(lambda)        (xform-lambda        tail env)]
             [(lambda*)       (xform-lambda*       tail env)]
             [(letcc)         (xform-letcc         tail env)]
             [(withcc)        (xform-withcc        tail env)]
             [(body)          (xform-body          tail env)]
             [(begin)         (xform-begin         tail env)]
             [(define)        (xform-define        tail env)]
             [(define-syntax) (xform-define-syntax tail env)]
             [(syntax-lambda) (xform-syntax-lambda tail env)]
             [(syntax-rules)  (xform-syntax-rules  tail env)]
             [(syntax-length) (xform-syntax-length tail env)]
             [(syntax-error)  (xform-syntax-error  tail env)]
             [else            (if (integrable? hval)
                                  (xform-integrable hval tail env)
                                  (if (procedure? hval)
                                      (xform appos? (hval sexp env) env)
                                      (xform-call hval tail env)))]))]))

(define (xform-ref id env)
  (let ([den (env id)])
    (cond [(eq? (location-val den) '...) (x-error "improper use of ...")]
          [else (location-val den)])))

(define (xform-quote tail env)
  (if (list1? tail)
      (list 'quote (xform-sexp->datum (car tail)))
      (x-error "improper quote form" (cons 'quote tail))))

(define (xform-set! tail env)
  (if (and (list2? tail) (id? (car tail)))
      (let ([den (env (car tail))] [xexp (xform #f (cadr tail) env)])
        (cond [(location-special? den) (location-set-val! den xexp) '(begin)]
              [else (let ([val (location-val den)])
                      (if (eq? (car val) 'ref)
                          (list 'set! (cadr val) xexp)
                          (x-error "set! to non-identifier form")))]))
      (x-error "improper set! form" (cons 'set! tail))))

(define (xform-set& tail env)
  (if (list1? tail)
      (let ([den (env (car tail))])      
        (cond [(location-special? den) (x-error "set& of a non-variable")]
              [else (let ([val (location-val den)])
                      (if (eq? (car val) 'ref)
                          (list 'set& (cadr val))
                          (x-error "set& of a non-variable")))]))
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
  (cond
    [(null? tail) 
     (list 'begin)]
    [(list1? tail) ; can't have defines there
     (xform #f (car tail) env)]
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
                                [nid (gensym (id->sym id))] [env (add-var id nid env)])
                           (loop env (cons id ids) (cons init inits) (cons nid nids) rest))]
                        [(and (list2+? tail) (pair? (car tail)) (id? (caar tail)) (idslist? (cdar tail)))
                         (let* ([id (caar tail)] [lambda-id (new-id 'lambda (make-location 'lambda))] 
                                [init (cons lambda-id (cons (cdar tail) (cdr tail)))]
                                [nid (gensym (id->sym id))] [env (add-var id nid env)])
                           (loop env (cons id ids) (cons init inits) (cons nid nids) rest))]
                        [else (x-error "improper define form" first)])]
                 [(define-syntax) ; internal
                  (if (and (list2? tail) (id? (car tail))) 
                      (let* ([id (car tail)] [init (cadr tail)]
                             [env (add-location id '(undefined) env)])
                        (loop env (cons id ids) (cons init inits) (cons #t nids) rest))
                      (x-error "improper define-syntax form" first))]
                 [else
                  (if (procedure? hval)
                      (loop env ids inits nids (cons (hval first env) rest))
                      (xform-labels (reverse ids) (reverse inits) (reverse nids) body env))])))
           (xform-labels (reverse ids) (reverse inits) (reverse nids) body env)))]))

(define (xform-labels ids inits nids body env)
  (let loop ([ids ids] [inits inits] [nids nids] [sets '()] [lids '()])
    (cond [(null? ids) 
           (let* ([xexps (append (reverse sets) (map (lambda (x) (xform #f x env)) body))]
                  [xexp (if (list1? xexps) (car xexps) (cons 'begin xexps))])
             (if (null? lids) xexp
                 (pair* 'call (list 'lambda (reverse lids) xexp)
                   (map (lambda (lid) '(begin)) lids))))]
          [(not (car ids)) ; idless define
           (loop (cdr ids) (cdr inits) (cdr nids)
             (cons (xform #f (car inits) env) sets) lids)]
          [(symbol? (car nids)) ; define
           (loop (cdr ids) (cdr inits) (cdr nids)
             (cons (xform-set! (list (car ids) (car inits)) env) sets)
             (cons (car nids) lids))]
          [else ; define-syntax
           (location-set-val! (env (car ids)) (xform #t (car inits) env))
           (loop (cdr ids) (cdr inits) (cdr nids) sets lids)])))

(define (xform-begin tail env) ; non-internal
  (if (list? tail)
      (let ([xexps (map (lambda (sexp) (xform #f sexp env)) tail)])
        (if (and (pair? xexps) (null? (cdr xexps)))
            (car xexps) ; (begin x) => x
            (cons 'begin xexps)))
      (x-error "improper begin form" (cons 'begin! tail))))

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

(define (xform-syntax-lambda tail env)
  (if (and (list2+? tail) (andmap id? (car tail)))
      (let ([vars (car tail)] [macenv env] [forms (cdr tail)])
        ; return a transformer that wraps xformed body in (syntax ...)
        ; to make sure xform treats it as final <core> form and exits the loop
        (lambda (use useenv)
          (if (and (list1+? use) (fx=? (length vars) (length (cdr use))))
              (let loop ([vars vars] [exps (cdr use)] [env macenv])
                (if (null? vars)
                    (list 'syntax (xform-body forms env))
                    (loop (cdr vars) (cdr exps)
                      (add-location (car vars) 
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
        (and (id? x) (ellipsis-denotation? (mac-env x)))))

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
      (map (lambda (id) (cons id (new-id (id->sym id) (mac-env id))))
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
      [ref (id)
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
      [define tail
       (c-error "misplaced define form" x)])))

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
      [ref (id)
       '()]
      [set! (id x)
       (set-union
         (if (set-member? id v) (list id) '())
         (find-sets x v))]
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
      [define tail
       (c-error "misplaced define form" x)])))

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
      [ref (id)
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
      [define tail
       (c-error "misplaced define form" x)])))

(define (compile-to-string x)
  (let ([p (open-output-string)])
    (codegen x '() '() '() (find-free x '()) #f p)                   
    (get-output-string p)))

(define (compile-to-thunk-code x)
  (let ([p (open-output-string)])
    (codegen x '() '() '() (find-free x '()) 0 p)                   
    (get-output-string p)))


;---------------------------------------------------------------------------------------------
; Code deserialization and execution
;---------------------------------------------------------------------------------------------

;(define (execute-thunk-closure t) (t))

; (define (make-closure code) ...) -- need builtin?

;(define execute
;  (lambda (code)
;    (execute-thunk-closure (make-closure code))))

;(define decode-sexp deserialize-sexp)

;(define decode deserialize-code)

;(define (evaluate x)
;  (execute (decode (compile-to-string (transform #f x)))))


;---------------------------------------------------------------------------------------------
; Environments
;---------------------------------------------------------------------------------------------

; new lookup procedure for alist-like macro environments

(define (env-lookup id env full?) ;=> location (| #f)
  (if (procedure? id)
      (old-den id) ; nonsymbolic ids can't be globally bound
      (let loop ([env env])
        (cond [(pair? env)
               (if (eq? (caar env) id)
                   (cdar env) ; location
                   (loop (cdr env)))]
              [(vector? env) ; root
               (let* ([n (vector-length env)] [i (immediate-hash id n)]
                      [al (vector-ref env i)] [p (assq id al)])
                 (if p (cdr p)
                     ; implicitly append integrables and "naked" globals
                     (let ([loc (make-location (or (lookup-integrable id) (list 'ref id)))])
                       (vector-set! env i (cons (cons id loc) al))
                       loc)))]
              [(string? env) ; module prefix
               (and full?
                 (let ([gid (string->symbol (string-append env (symbol->string id)))])
                   (env-lookup gid *root-environment* #t)))]
              [else ; finite env 
               #f]))))


; make root environment from the list of initial transformers

(define *root-environment*
  (let* ([n 101] ; use prime number
         [env (make-vector n '())]) 
    (define (put! k loc)
      (let* ([i (immediate-hash k n)] [al (vector-ref env i)] [p (assq k al)])
        (cond [p (set-cdr! p loc)]
              [else (vector-set! env i (cons (cons k loc) al))])))
    (let loop ([l (initial-transformers)])
      (if (null? l) env
          (let ([p (car l)] [l (cdr l)])
            (let ([k (car p)] [v (cdr p)])
              (cond
                [(or (symbol? v) (number? v)) 
                 (put! k (make-location v))
                 (loop l)]
                [(and (pair? v) (eq? (car v) 'syntax-rules))
                 (body
                   (define (sr-env id) 
                     (env-lookup id *root-environment* #t))
                   (define sr-v
                     (if (id? (cadr v))
                         (syntax-rules* sr-env (cadr v) (caddr v) (cdddr v))
                         (syntax-rules* sr-env #f (cadr v) (cddr v))))
                   (put! k (make-location sr-v))
                   (loop l))])))))))

(define (root-environment id)
  ; new protocol for top-level envs
  (if (pair? id)
      (record-case id
        [define (i) i]
        [define-syntax (i) (env-lookup i *root-environment* #t)])
      (env-lookup id *root-environment* #t)))


;---------------------------------------------------------------------------------------------
; Evaluation
;---------------------------------------------------------------------------------------------

(define (error* msg args)
  (apply error (cons msg args)))

; transformation of top-level form should process begin, define, and define-syntax
; explicitly, so that they can produce and observe side effects on env

(define (eval-top-form x env)
  (if (pair? x)
      (let ([hval (xform #t (car x) env)])
        (cond
          [(eq? hval 'begin)
           (let loop ([x* (cdr x)])
             (when (pair? x*) 
               (eval-top-form (car x*) env)
               (loop (cdr x*))))]
          [(eq? hval 'define)
           ; new protocol for top-level envs
           (let* ([core (xform-define (cdr x) env)]
                  [res (env (list 'define (cadr core)))])
             (if res ; symbol (runtime store key) or #f
                 (compile-and-run-core-expr (list 'set! res (caddr core)))
                 (x-error "identifier cannot be (re)defined in env"
                   (cadr core) env)))]
          [(eq? hval 'define-syntax)
           ; new protocol for top-level envs
           (let* ([core (xform-define-syntax (cdr x) env)]
                  [res (env (list 'define-syntax (cadr core)))])
             (if res ; macro location or #f
                 (location-set-val! res (caddr core))
                 (x-error "identifier cannot be (re)defined as syntax in env"
                   (cadr core) env)))]
          [(procedure? hval)
           (eval-top-form (hval x env) env)]
          [(integrable? hval)
           (compile-and-run-core-expr 
             (xform-integrable hval (cdr x) env))]
          [else
           (compile-and-run-core-expr 
             (xform #f x env))]))
      (compile-and-run-core-expr 
        (xform #f x env))))

(define *verbose* #f)

(define (compile-and-run-core-expr core)
  (unless (pair? core) (x-error "unexpected transformed output" core))
  (when *verbose* (write core) (newline))
  (when (eq? (car core) 'define) (set-car! core 'set!))
  (let ([code (compile-to-thunk-code core)])
    (when *verbose* (write code) (newline))
    (let* ([cl (closure (deserialize-code code))] [r (cl)])
      (when *verbose* (write r) (newline)))))
  
#|
(define (transform! x)
  (let ([t (xform #t x root-environment)])
    (when (and (syntax-match? '(define-syntax * *) t) (id? (cadr t))) ; (procedure? (caddr t))
        (let ([loc (root-environment (cadr t))])
          (when loc (location-set-val! loc (caddr t)))))
    t)) 
  
(define (visit f) 
  (define p (open-input-file f)) 
  (let loop ([x (read p)]) 
    (unless (eof-object? x)
      (let ([t (transform! x)])
        (write t) 
        (newline))
      (loop (read p)))) 
  (close-input-port p))

(define (visit/c f) 
  (define p (open-input-file f)) 
  (let loop ([x (read p)]) 
    (unless (eof-object? x)
      (let ([t (transform! x)])
        (write t) (newline)
        (let exec ([x t]) 
          (record-case x
            [begin x* (for-each exec x*)]
            [define (i v) (exec (list 'set! i v))]
            [define-syntax (i m)]
            [else (write (compile-to-thunk-code x)) (newline)])))
      (loop (read p)))) 
  (close-input-port p))
|#


(define (visit/x f) 
  (define p (open-input-file f)) 
  (let loop ([x (read p)]) 
    (unless (eof-object? x)
      (when *verbose* (write x) (newline))
      (eval-top-form x root-environment)
      (when *verbose* (newline))
      (loop (read p)))) 
  (close-input-port p))

