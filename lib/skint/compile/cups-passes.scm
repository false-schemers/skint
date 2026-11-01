;;; cups-passes.scm -- CUPS part 3: middle/CPS passes, CUPS validator,
;;; .cps serializer and reader.  Pass order (driver in cups-main.scm):
;;;   fix-letrecs -> remove-assignments -> analyze-globals -> constant-fold
;;;   -> beta-substitute #f #f -> lift-quoted-constants -> cps-convert
;;;   -> beta-substitute #t #f -> lambda-lift -> beta-substitute #t #t
;;;   -> convert-global-calls -> unbox-values -> validate-cups

;; variant-case binds fields by calling <variant>-><field> accessors,
;; so the short field names used in several clauses below need aliases
;; for the canonical accessors defined in cups-base.scm.
(define if-exp->t if-exp->test-exp)
(define if-exp->h if-exp->then-exp)
(define if-exp->a if-exp->else-exp)
(define if-exp->e if-exp->else-exp)
(define gvarassign-exp->x gvarassign-exp->exp)
(define gvarassign-exp->e gvarassign-exp->exp)
(define gvarassign-exp->i gvarassign-exp->id)
(define varassign-exp->x varassign-exp->exp)
(define varassign-exp->e varassign-exp->exp)
(define varassign-exp->i varassign-exp->id)
(define var-exp->i var-exp->id)
(define letcc-exp->i letcc-exp->id)
(define ilambda-exp->i ilambda-exp->id)
(define withcc-exp->c withcc-exp->cont-exp)
(define withcc-exp->x withcc-exp->exp)
(define withcc-exp->e withcc-exp->exp)
(define once-exp->x once-exp->exp)
(define once-exp->e once-exp->exp)
(define labelapp-exp->tid labelapp-exp->id)
(define begin-exp->x begin-exp->exp1)
(define begin-exp->y begin-exp->exp2)

(define (clear-cups-memoizations)
  (exp-vinfo #f) (var-uses-in-exp #f #f)
  (exp->free-vars #f) (exp->free-lexvars #f))

;;; ------------------------------------------------------------------
;;; Init timing of globals (the single-module / no-libraries model).
;;;
;;; The compiler never sees a library.  Expansion flattens a program
;;; and everything it imports into ONE module: an ordered init
;;; sequence whose steps assign values -- among them lambda
;;; expressions -- to global names.  Three facts make timing
;;; analysis possible:
;;;   1. An importing program/library cannot assign to an imported
;;;      (exported) identifier: the expander rejects it.  A global's
;;;      assignments are therefore exactly the ones visible in the
;;;      flattened module.
;;;   2. Init code of an importer cannot run before the init code of
;;;      everything it imports (dependencies are emitted first), so
;;;      the flattened sequence order IS the dynamic init order.
;;;   3. A procedure cannot be called before its own init has run,
;;;      and an imported procedure cannot be called before its
;;;      library's init has run (by 2, before any importer's init).
;;;
;;; A global is therefore promoted to a global-id-constant -- its
;;; value may be inlined and calls to it may become static direct
;;; calls across any former library border -- iff
;;;   (a) it is assigned exactly once in the whole module (fact 1:
;;;       it never changes after that assignment executes), and
;;;   (b) that assignment is guaranteed to execute during init, at
;;;       a known position T in the init sequence, and
;;;   (c) every use of the global is guaranteed to execute after T:
;;;       uses in the init sequence have a later position, and uses
;;;       inside a procedure body are timed by the body's earliest
;;;       possible invocation, computed by the fixpoint below
;;;       (calls and escapes lower a body's earliest invocation;
;;;       a body never invoked during init has time POST, after all
;;;       of init -- which is the common case for foo calling bar
;;;       in another library: both are assigned during init, and
;;;       foo's body can only run afterwards).
;;; Uses that would execute at or before T make the global fail the
;;; analysis: it stays an ordinary mutable global (still assigned
;;; once, but read through its box at run time).  This rejects the
;;; one dangerous shape: an init expression that actually invokes a
;;; procedure before a global it references has been assigned.
;;;
;;; *global-timing* records the result: ((id known-at status) ...)
;;; with status one of static | conditional | late-use | in-procedure.

(define *global-timing* '())

(define (flatten-init-sequence exp) ; begin spine -> (step ...)
  ;; remove-assignments wraps lambda bodies in null-lets, so the
  ;; sequence spine must see through degenerate lets as well.
  (cond [(begin-exp? exp)
         (append (flatten-init-sequence (begin-exp->exp1 exp))
                 (flatten-init-sequence (begin-exp->exp2 exp)))]
        [(degenerate-let-exp? exp)
         (flatten-init-sequence (degenerate-let-exp->body exp))]
        [(void-exp? exp) '()]
        [else (list exp)]))

(define (analyze-init-timing exp)
  (define steps (flatten-init-sequence exp))
  (define n-steps (length steps))
  (define post-time n-steps) ; "after init" sorts after every step
  ;; -- collect every globally assigned id ---------------------------
  (define assigned-ids '())
  (define assign-counts '()) ; ((id . n) ...) over the whole module
  (define (note-assignment! id)
    (unless (memq id assigned-ids) (set! assigned-ids (cons id assigned-ids)))
    (cond [(assq id assign-counts) => (lambda (a) (set-cdr! a (+ (cdr a) 1)))]
          [else (set! assign-counts (cons (cons id 1) assign-counts))]))
  (define (collect-assigned e)
    (cond [(gvarassign-exp? e)
           (note-assignment! (gvarassign-exp->id e))
           (collect-assigned (gvarassign-exp->exp e))]
          [(quote-exp? e) (void)]
          [(if-exp? e) (collect-assigned (if-exp->test-exp e))
                       (collect-assigned (if-exp->then-exp e))
                       (collect-assigned (if-exp->else-exp e))]
          [(primapp-exp? e) (for-each collect-assigned (primapp-exp->rands e))]
          [(app-exp? e) (collect-assigned (app-exp->rator e))
                        (for-each collect-assigned (app-exp->rands e))]
          [(lambda-exp? e) (collect-assigned (lambda-exp->body e))]
          [(ilambda-exp? e) (collect-assigned (ilambda-exp->body e))]
          [(lambda*-exp? e) (for-each collect-assigned (lambda*-exp->lambdas e))]
          [(fix-exp? e) (for-each collect-assigned (fix-exp->lams e))
                        (collect-assigned (fix-exp->body e))]
          [(let-exp? e) (for-each collect-assigned (let-exp->rands e))
                        (collect-assigned (let-exp->body e))]
          [(begin-exp? e) (collect-assigned (begin-exp->exp1 e))
                          (collect-assigned (begin-exp->exp2 e))]
          [(varassign-exp? e)
           (when (global-id? (varassign-exp->id e))
             (note-assignment! (varassign-exp->id e)))
           (collect-assigned (varassign-exp->exp e))]
          [(letcc-exp? e) (collect-assigned (letcc-exp->body e))]
          [(withcc-exp? e) (collect-assigned (withcc-exp->cont-exp e))
                           (collect-assigned (withcc-exp->exp e))]
          [else (void)]))
  (for-each collect-assigned steps)
  (define candidates '()) ; ((id pos guaranteed? rhs) ...)
  (define candidate-ids
    (keep (lambda (id) (= 1 (cond [(assq id assign-counts) => cdr] [else 0])))
          assigned-ids))
  ;; one pass over the init steps (not into procedure bodies)
  (define (record-assignments! e pos guaranteed?)
    (cond [(gvarassign-exp? e)
           (let ([id (gvarassign-exp->id e)])
             (when (and (memq id candidate-ids) (not (assq id candidates)))
               (set! candidates
                 (cons (list id pos guaranteed? (gvarassign-exp->exp e)) candidates))))
           (record-assignments! (gvarassign-exp->exp e) pos guaranteed?)]
          [(quote-exp? e) (void)]
          [(if-exp? e) (record-assignments! (if-exp->test-exp e) pos guaranteed?)
                       (record-assignments! (if-exp->then-exp e) pos #f)
                       (record-assignments! (if-exp->else-exp e) pos #f)]
          [(begin-exp? e) (record-assignments! (begin-exp->exp1 e) pos guaranteed?)
                          (record-assignments! (begin-exp->exp2 e) pos guaranteed?)]
          [(let-exp? e) (for-each (lambda (r) (record-assignments! r pos guaranteed?)) (let-exp->rands e))
                        (record-assignments! (let-exp->body e) pos guaranteed?)]
          [(fix-exp? e) (record-assignments! (fix-exp->body e) pos guaranteed?)]
          [(primapp-exp? e) (for-each (lambda (r) (record-assignments! r pos guaranteed?)) (primapp-exp->rands e))]
          [(app-exp? e) (record-assignments! (app-exp->rator e) pos guaranteed?)
                        (for-each (lambda (r) (record-assignments! r pos guaranteed?)) (app-exp->rands e))]
          [(varassign-exp? e) (record-assignments! (varassign-exp->exp e) pos guaranteed?)]
          [(letcc-exp? e) (record-assignments! (letcc-exp->body e) pos guaranteed?)]
          [(withcc-exp? e) (record-assignments! (withcc-exp->cont-exp e) pos guaranteed?)
                           (record-assignments! (withcc-exp->exp e) pos guaranteed?)]
          [else (void)]))
  (let loop ([ss steps] [pos 0])
    (unless (null? ss) (record-assignments! (car ss) pos #t) (loop (cdr ss) (+ pos 1))))
  ;; A statically known *procedure* must be an immediate lambda /
  ;; ilambda / lambda* (former case-lambda) assigned directly by a
  ;; step of the init-sequence begin.  A procedure value produced by
  ;; any other expression (let/if/alias/call ...), however constant,
  ;; does not participate in static procedure finding: in particular
  ;;   (define bar (let ((y 10)) (lambda (x) (+ x y))))
  ;; makes bar an ordinary first-class value -- that lambda closes
  ;; over y, has a nonempty closure display, is not a flat
  ;; closure-less procedure callable by name, and calls to bar by
  ;; name remain ordinary indirect calls.  (A let that is itself a
  ;; step of the init chain is a different matter entirely; see
  ;; name-init-chain-lets below.)
  (define direct-procedures '()) ; ((id . lambda|ilambda|lambda*) ...)
  (for-each
    (lambda (step)
      (when (gvarassign-exp? step)
        (let ([rhs (gvarassign-exp->exp step)] [id (gvarassign-exp->id step)])
          (when (memq id candidate-ids)
            (cond [(lambda-exp? rhs) (set! direct-procedures (cons (cons id 'lambda) direct-procedures))]
                  [(ilambda-exp? rhs) (set! direct-procedures (cons (cons id 'ilambda) direct-procedures))]
                  [(lambda*-exp? rhs) (set! direct-procedures (cons (cons id 'lambda*) direct-procedures))])))))
    steps)

  ;; -- earliest-invocation fixpoint ---------------------------------
  ;; lam-vals: lambda node -> earliest invocation time (starts POST)
  (define invoke-time '()) ; ((node . time) ...)
  (define (lam-time node)
    (cond [(assq node invoke-time) => cdr] [else post-time]))
  (define (remember-lam! node)
    (unless (assq node invoke-time)
      (set! invoke-time (cons (cons node post-time) invoke-time))))
  (define constraints '()) ; ((target-node . source) ...) source = pos | node
  (define (constrain! target source)
    (remember-lam! target)
    (set! constraints (cons (cons target source) constraints)))
  (define occurrences '()) ; ((candidate-id . source) ...)
  (define (candidate? id) (assq id candidates))
  (define (rhs-lambdas id) ; lambda node(s) a candidate is assigned, or ()
    (cond [(candidate? id) =>
           (lambda (c)
             (let ([rhs (cadddr c)])
               (cond [(or (lambda-exp? rhs) (ilambda-exp? rhs)) (list rhs)]
                     [(lambda*-exp? rhs) (lambda*-exp->lambdas rhs)]
                     [else '()])))]
          [else '()]))
  ;; walk: ctx is an init position (integer) or a lambda node;
  ;; env maps local ids to lambda node lists (() = unknown value).
  (define (lam-nodes-of e)
    (cond [(or (lambda-exp? e) (ilambda-exp? e)) (list e)]
          [(lambda*-exp? e) (lambda*-exp->lambdas e)]
          [else '()]))
  (define (walk-lambda-body lam ctx-env)
    (remember-lam! lam)
    (cond [(lambda-exp? lam)
           (walk (lambda-exp->body lam) lam
                 (append (map (lambda (i) (cons i '())) (lambda-exp->ids lam)) ctx-env))]
          [(ilambda-exp? lam)
           (walk (ilambda-exp->body lam) lam
                 (append (map (lambda (i) (cons i '()))
                              (cons (ilambda-exp->id lam) (ilambda-exp->ids lam))) ctx-env))]
          [else (void)]))
  (define (walk-clauses e ctx env) ; lambda* value in binding position
    (for-each (lambda (cl) (walk-lambda-body cl env)) (lambda*-exp->lambdas e)))
  (define (walk-value e ctx env) ; e in a position where it may escape
    (cond [(or (lambda-exp? e) (ilambda-exp? e))
           (remember-lam! e) (constrain! e ctx) (walk-lambda-body e env)]
          [(lambda*-exp? e)
           (for-each (lambda (cl) (remember-lam! cl) (constrain! cl ctx)
                                   (walk-lambda-body cl env))
                     (lambda*-exp->lambdas e))]
          [else (walk e ctx env)]))
  (define (walk e ctx env)
    (cond
      [(quote-exp? e) (void)]
      [(var-exp? e)
       (let ([id (var-exp->id e)])
         (cond [(candidate? id) (set! occurrences (cons (cons id ctx) occurrences))]
               [(assq id env) =>
                (lambda (b) (for-each (lambda (n) (remember-lam! n) (constrain! n ctx)) (cdr b)))]
               [else (void)]))]
      [(gvarassign-exp? e)
       (let ([rhs (gvarassign-exp->exp e)])
         (cond [(or (lambda-exp? rhs) (ilambda-exp? rhs)) (walk-lambda-body rhs env)]
               [(lambda*-exp? rhs) (walk-clauses rhs ctx env)]
               [else (walk rhs ctx env)]))]
      [(varassign-exp? e) (walk (varassign-exp->exp e) ctx env)]
      [(if-exp? e) (walk (if-exp->test-exp e) ctx env)
                   (walk (if-exp->then-exp e) ctx env)
                   (walk (if-exp->else-exp e) ctx env)]
      [(primapp-exp? e) (for-each (lambda (r) (walk-value r ctx env)) (primapp-exp->rands e))]
      [(app-exp? e)
       (let ([rator (app-exp->rator e)])
         (cond [(and (var-exp? rator) (pair? (rhs-lambdas (var-exp->id rator))))
                (set! occurrences
                      (cons (cons (var-exp->id rator) ctx) occurrences))
                (for-each (lambda (n) (remember-lam! n) (constrain! n ctx))
                          (rhs-lambdas (var-exp->id rator)))]
               [(and (var-exp? rator) (assq (var-exp->id rator) env))
                => (lambda (b)
                     (set! occurrences (cons (cons (var-exp->id rator) ctx) occurrences))
                     (for-each (lambda (n) (remember-lam! n) (constrain! n ctx)) (cdr b))
                     ;; fallthrough: also record var use below
                     )]
               [(or (lambda-exp? rator) (ilambda-exp? rator))
                (remember-lam! rator) (constrain! rator ctx) (walk-lambda-body rator env)]
               [else (walk-value rator ctx env)]))
       ;; a var rator that is a local lambda was constrained above via
       ;; the generic var case only in the non-lambda branch; rands:
       (for-each (lambda (r) (walk-value r ctx env)) (app-exp->rands e))]
      [(let-exp? e)
       (let ([new-env
              (append (map (lambda (id rand)
                             (cons id (begin (cond [(or (lambda-exp? rand) (ilambda-exp? rand))
                                                    (walk-lambda-body rand env)]
                                                   [(lambda*-exp? rand) (walk-clauses rand ctx env)]
                                                   [else (walk rand ctx env)])
                                             (lam-nodes-of rand))))
                           (let-exp->ids e) (let-exp->rands e))
                      env)])
         (walk (let-exp->body e) ctx new-env))]
      [(fix-exp? e)
       (let ([new-env
              (append (map (lambda (id lam) (cons id (lam-nodes-of lam)))
                           (fix-exp->ids e) (fix-exp->lams e)) env)])
         (for-each (lambda (lam)
                     (cond [(lambda*-exp? lam) (walk-clauses lam ctx new-env)]
                           [else (walk-lambda-body lam new-env)]))
                   (fix-exp->lams e))
         (walk (fix-exp->body e) ctx new-env))]
      [(or (lambda-exp? e) (ilambda-exp? e)) (walk-value e ctx env)]
      [(lambda*-exp? e) (walk-value e ctx env)]
      [(letcc-exp? e)
       (walk (letcc-exp->body e) ctx
             (cons (cons (letcc-exp->id e) '()) env))]
      [(withcc-exp? e) (walk-value (withcc-exp->cont-exp e) ctx env)
                       (walk (withcc-exp->exp e) ctx env)]
      [(once-exp? e) (walk (once-exp->exp e) ctx env)]
      [else (void)]))
  (let loop ([ss steps] [pos 0])
    (unless (null? ss) (walk (car ss) pos '()) (loop (cdr ss) (+ pos 1))))
  ;; solve: worklist propagation of earliest invocation times
  ;; (a body's time only ever lowers, towards its earliest caller)
  (define dependents '()) ; node -> (target ...)
  (define worklist '())
  (define (enqueue! node) (set! worklist (cons node worklist)))
  (for-each
    (lambda (c)
      (if (integer? (cdr c))
          (let ([cell (assq (car c) invoke-time)])
            (when (and cell (< (cdr c) (cdr cell)))
              (set-cdr! cell (cdr c)) (enqueue! (car c))))
          (cond [(assq (cdr c) dependents) =>
                 (lambda (d) (set-cdr! d (cons (car c) (cdr d))))]
                [else (set! dependents (cons (cons (cdr c) (list (car c))) dependents))])))
    constraints)
  (let loop ()
    (when (pair? worklist)
      (let ([node (car worklist)])
        (set! worklist (cdr worklist))
        (cond [(assq node dependents) =>
               (lambda (d)
                 (for-each
                   (lambda (target)
                     (let ([cell (assq target invoke-time)])
                       (when (and cell (< (cdr (assq node invoke-time)) (cdr cell)))
                         (set-cdr! cell (cdr (assq node invoke-time)))
                         (enqueue! target))))
                   (cdr d)))])
        (loop))))
  ;; -- verdicts -------------------------------------------------------
  (define (source-time src) (if (integer? src) src (lam-time src)))
  (define result
    (map (lambda (c)
           (let ([id (car c)] [pos (cadr c)] [guaranteed? (caddr c)])
             (define late?
               (ormap (lambda (o)
                        (and (eq? id (car o)) (<= (source-time (cdr o)) pos)))
                      occurrences))
             (list id pos
                   (cond [(not guaranteed?) 'conditional]
                         [late? 'late-use]
                         [else 'static])
                   (cond [(assq id direct-procedures) => cdr] [else #f]))))
         candidates))
  (set! *global-timing* result)
  result)

(define (global-timing-status sym) ; => (known-at status) or #f
  (let loop ([t *global-timing*])
    (cond [(null? t) #f]
          [(eq? sym (id->symbol (caar t))) (cdr (car t))]
          [else (loop (cdr t))])))

;;; ------------------------------------------------------------------
;;; analyze-globals: promote timing-verified globals to constants.
;;; (Adapted from sfc 4.sf; #F's stack functions are omitted -- in
;;; CUPS such a function is an ordinary lifted label, partitioned
;;; B1/B2/C by the S-MARV back end.)  Promotion is what lets beta and
;;; convert-global-calls inline a procedure value or turn a call to
;;; it into a static direct call, across any former library border.

;; Post-rename statically known procedures: ((global-id . kind) ...)
;; kind is lambda | ilambda | lambda*.  Only these globals may be
;; direct-called (convert-global-calls); a merely constant global
;; whose value is a procedure produced some other way may not.
(define *static-procedures* '())

(define (static-procedure-kind sym) ; => kind or #f
  (let loop ([s *static-procedures*])
    (cond [(null? s) #f]
          [(eq? sym (id->symbol (caar s))) (cdar s)]
          [else (loop (cdr s))])))

(define (analyze-globals exp)
  (analyze-init-timing exp)
  (define promoted
    (keep (lambda (t) (eq? 'static (caddr t))) *global-timing*))
  (define substs
    (map (lambda (t)
           (let ([id (car t)])
             (list id
                   (if (global-id-private-constant? id) id
                       (global-id-constant (id->symbol id)))
                   #f)))
         promoted))
  (set! *static-procedures*
    (map (lambda (t)
           (cons (cond [(assq (car t) substs) => cadr] [else (car t)])
                 (cadddr t)))
         (keep (lambda (t) (and (eq? 'static (caddr t)) (cadddr t)))
               *global-timing*)))
  (define (subst-globals e) ; substs: ((old cid #f) ...)
    (variant-case e
      [var-exp (id) (cond [(assq id substs) => (lambda (s) (var-exp (cadr s)))] [else e])]
      [gvarassign-exp (id exp)
       (cond [(assq id substs) => (lambda (s) (gvarassign-exp (cadr s) (subst-globals exp)))]
             [else (gvarassign-exp id (subst-globals exp))])]
      [varassign-exp (id exp) (varassign-exp id (subst-globals exp))]
      [if-exp (test-exp then-exp else-exp)
       (if-exp (subst-globals test-exp) (subst-globals then-exp) (subst-globals else-exp))]
      [primapp-exp (effect prim rands) (primapp-exp effect prim (map subst-globals rands))]
      [app-exp (rator rands) (app-exp (subst-globals rator) (map subst-globals rands))]
      [fix-exp (ids lams body) (fix-exp ids (map subst-globals lams) (subst-globals body))]
      [lambda-exp (ids body) (lambda-exp ids (subst-globals body))]
      [ilambda-exp (ids id body) (ilambda-exp ids id (subst-globals body))]
      [lambda*-exp (arities lambdas) (lambda*-exp arities (map subst-globals lambdas))]
      [letcc-exp (id body) (letcc-exp id (subst-globals body))]
      [withcc-exp (cont-exp exp) (withcc-exp (subst-globals cont-exp) (subst-globals exp))]
      [once-exp (id exp) (once-exp id (subst-globals exp))]
      [else e]))
  (if (null? substs) exp (subst-globals exp)))

;;; ------------------------------------------------------------------
;;; name-init-chain-lets: a let that survived beta-substitution and
;;; is still an immediate step of the init-sequence begin is, by
;;; definition, equivalent to a statically known procedure with a
;;; single call site:
;;;   (let ((x ex) (y ey)) body)
;;;     ==  (define tmpname<n> (lambda (x y) body))
;;;         (tmpname<n> ex ey)
;;; The generated name is a statically known, by-then-inited global
;;; (assigned immediately before its only call), and the single call
;;; has the right argument count by construction -- so only the
;;; non-argc-checking body variant of tmpname<n> will ever be
;;; needed: the call becomes a direct jump, no checking head, no
;;; closure materialisation.  Only lets on the begin/let spine of
;;; the init chain are rewritten (recursively, so a let's body may
;;; itself start with further chain lets, which become generated
;;; procedures invoked from within the enclosing one).  A let in
;;; VALUE position -- above all as the assigned value in
;;; (define bar (let ((y 10)) (lambda (x) (+ x y)))) -- is not an
;;; init-chain step and is untouched: that lambda closes over y and
;;; bar remains an ordinary first-class procedure value.
;;;
;;; This pass runs after analyze-globals (whose timing analysis
;;; already understands lets natively, so globals assigned inside a
;;; chain let's body keep their recorded positions and verdicts) and
;;; after the first beta-substitute (so only lets that genuinely
;;; survived appear here).  It registers each generated name in
;;; *static-procedures* directly; the generated assignment has the
;;; direct-define shape, so convert-global-calls sees it in
;;; *global-curry-table* and turns the single call into a jump.

(define (name-init-chain-lets exp)
  (define (chain e) ; e in init-chain position -> exp
    (cond [(begin-exp? e)
           (begin-exp (chain (begin-exp->exp1 e)) (chain (begin-exp->exp2 e)))]
          [(degenerate-let-exp? e)
           (if (null-let-exp? e)
               (null-let-exp (chain (degenerate-let-exp->body e)))
               (identity-let-exp (chain (degenerate-let-exp->body e))))]
          [(let-exp? e)
           (let ([tmp (global-id-constant (symbol 'tmpname "#" (timestamp)))]
                 [body (chain (let-exp->body e))])
             (set! *static-procedures* (cons (cons tmp 'lambda) *static-procedures*))
             (begin-exp (gvarassign-exp tmp (lambda-exp (let-exp->ids e) body))
                        (app-exp (var-exp tmp) (let-exp->rands e))))]
          [else e])) ; value positions (gset right-hand sides, calls,
                     ; branches, procedure bodies) are not init chain
  (chain exp))

;;; ------------------------------------------------------------------
;;; constant-fold (adapted from sfc 4.sf; ctypes are our symbols).
;;; Folds a type predicate whose argument's ctype is known, and drops
;;; a %ck* assertion whose argument already has the asserted ctype
;;; (the assertion is re-emitted implicitly by unbox consumers).

(define (constant-fold exp)
  (define (fold-check t-exp env h e)
    (variant-case t-exp
      [let-exp (ids rands body)
       (fold-check body
         (append (map (lambda (id r) (cons id (exp-ctype r env))) ids rands) env) h e)]
      [primapp-exp (effect prim rands)
       (and (= 1 (length rands))
            (let ([ct (typecheck-prim-ctype prim)]
                  [rt (exp-ctype (car rands) env)])
              (and ct rt (not (memq ct '(obj))) (not (memq rt '(obj)))
                   (if (eq? ct rt) h e))))]
      [else #f]))
  (define (cf exp env)
    (variant-case exp
      [if-exp (t h e)
       (let ([t2 (cf t env)] [h2 (cf h env)] [e2 (cf e env)])
         (cond [(fold-check t2 env h2 e2) => (lambda (x) (begin-exp t2 x))]
               [else (if-exp t2 h2 e2)]))]
      [let-exp (ids rands body)
       (let ([r2 (map (lambda (r) (cf r env)) rands)])
         (let-exp ids r2
           (cf body (append (map (lambda (id r) (cons id (exp-ctype r env)))
                                 ids r2) env))))]
      [primapp-exp (effect prim rands) (primapp-exp effect prim (map (lambda (r) (cf r env)) rands))]
      [app-exp (rator rands)
       (let ([x (app-exp (cf rator env) (map (lambda (r) (cf r env)) rands))])
         (if (let-exp? x) (cf x env) x))]
      [fix-exp (ids lams body) (fix-exp ids (map (lambda (l) (cf l env)) lams) (cf body env))]
      [lambda-exp (ids body) (lambda-exp ids (cf body env))]
      [ilambda-exp (ids id body) (ilambda-exp ids id (cf body env))]
      [lambda*-exp (arities lambdas) (lambda*-exp arities (map (lambda (l) (cf l env)) lambdas))]
      [gvarassign-exp (id e) (gvarassign-exp id (cf e env))]
      [letcc-exp (id body) (letcc-exp id (cf body env))]
      [withcc-exp (c e) (withcc-exp (cf c env) (cf e env))]
      [else exp]))
  (cf exp '()))

;;; ------------------------------------------------------------------
;;; beta-substitute (sfc 4.sf lineage + compile-x.txt folding).
;;; Modes: (#f #f) pre-CPS, (#t #f) post-CPS admin, (#t #t) label conv.
;;; CUPS additions: safe integrable folding (never folds a call that
;;; raises, nor a 0-arg host query into a non-literal), and direct
;;; reduction of ((ilambda ...) rands...) / ((lambda* ...) rands...).

(define (safe-apply-integrable prim objs)
  (define (literal? o)
    (or (boolean? o) (number? o) (char? o) (null? o) (symbol? o) (string? o)))
  (and (pair? objs)
       (guard (e (else #f))
         (let ([r (apply-integrable prim objs)])
           (and (literal? r) (list r)))))) ; boxed result / #f

;;; Whether an expression may be dropped when nobody wants its value.
;;;
;;; This is the proof obligation behind dropping a definition.  An
;;; expression that only allocates, or only reads, cannot change what the
;;; program computes or does once its value is dead; one that produces an
;;; effect, or that can raise, can.  The recursion is conservative by
;;; construction: an application of anything the compiler does not know
;;; about falls to the else arm and is not removable, and a primitive is
;;; judged by the effect its table entry records.
;;;
;;; beta-substitute keeps its own copy under the old name, so that pass is
;;; untouched; ds-shake! asks through this one.  A (begin ...) is a virtual
;;; variant over let-exp, so the let-exp arm already covers it.  [cco]
(define (exp-removable-if-dead? exp)
  (variant-case exp
    [quote-exp (obj) #t]
    [var-exp (id) #t]
    [if-exp (test-exp then-exp else-exp)
     (andapp exp-removable-if-dead? test-exp then-exp else-exp)]
    [primapp-exp (effect prim rands)
     (and (removable-if-dead-effect? effect)
          (not (integrable-may-raise? prim))
          (andmap exp-removable-if-dead? rands))]
    [lambda-exp (ids body) #t]
    [ilambda-exp (ids id body) #t]
    [lambda*-exp (arities lambdas) #t]
    [let-exp (ids rands body)
     (and (andmap exp-removable-if-dead? rands)
          (exp-removable-if-dead? body))]
    [letcc-exp (id body) (exp-removable-if-dead? body)]
    [withcc-exp (cont-exp exp) (andapp exp-removable-if-dead? cont-exp exp)]
    [else #f]))

(define (beta-substitute exp hold-cps-invariants? hold-lifting-invariants?)
  (define (bs-zero-cost-ref-transparent? exp)
    (variant-case exp
      [quote-exp (obj) (or (boolean? obj) (number? obj) (char? obj) (symbol? obj) (null? obj))]
      [var-exp (id) (or (not (global-id? id)) (global-id-constant? id))]
      [primapp-exp (effect prim rands)
       (and (ref-transparent-effect? effect) (null? rands) (integer? prim))]
      [else #f]))
  (define (bs-ref-transparent? exp)
    (variant-case exp
      [quote-exp (obj) #t]
      [var-exp (id) (or (not (global-id? id)) (global-id-constant? id))]
      [if-exp (t h e) (andapp bs-ref-transparent? t h e)]
      [primapp-exp (effect prim rands)
       (and (ref-transparent-effect? effect) (andmap bs-ref-transparent? rands))]
      [lambda-exp (ids body) #t]
      [ilambda-exp (ids id body) #t]
      [lambda*-exp (arities lambdas) #t]
      [let-exp (ids rands body)
       (and (andmap bs-ref-transparent? rands) (bs-ref-transparent? body))]
      [else #f]))
  (define (bs-removable-if-dead? exp)
    (variant-case exp
      [quote-exp (obj) #t]
      [var-exp (id) #t]
      [if-exp (t h e) (andapp bs-removable-if-dead? t h e)]
      [primapp-exp (effect prim rands)
       (and (removable-if-dead-effect? effect) (andmap bs-removable-if-dead? rands))]
      [lambda-exp (ids body) #t]
      [ilambda-exp (ids id body) #t]
      [lambda*-exp (arities lambdas) #t]
      [let-exp (ids rands body)
       (and (andmap bs-removable-if-dead? rands) (bs-removable-if-dead? body))]
      [letcc-exp (id body) (bs-removable-if-dead? body)]
      [withcc-exp (c e) (andapp bs-removable-if-dead? c e)]
      [else #f]))
  (define (bs-arg-of-application? id body)
    (define (one? e) (and (var-exp? e) (eq? id (var-exp->id e))))
    (variant-case body
      [primapp-exp (effect prim rands)
       (and (andmap bs-ref-transparent? rands) (ormap one? rands))]
      [app-exp (rator rands)
       (and (bs-ref-transparent? rator) (andmap bs-ref-transparent? rands)
            (or (one? rator) (ormap one? rands)))]
      [else #f]))
  (define (bs-cheap-ref-transparent? exp)
    (variant-case exp
      [primapp-exp (effect prim rands)
       (and (ref-transparent-effect? effect) (<= (length rands) 2)
            (andmap bs-zero-cost-ref-transparent? rands))]
      [else #f]))
  (define (bs-cheap-lambda? exp)
    (variant-case exp
      [lambda-exp (ids body)
       (variant-case body
         [primapp-exp (effect prim rands) (andmap bs-zero-cost-ref-transparent? rands)]
         [app-exp (rator rands)
          (and (bs-zero-cost-ref-transparent? rator) (andmap bs-zero-cost-ref-transparent? rands))]
         [else #f])]
      [else #f]))
  (define (make-list-exp exps)
    (if (null? exps)
        (quote-exp '())
        (primapp-exp (*-effect) (lookup-integrable 'list) exps)))
  (define (reduce-variadic rator rands) ; => exp or #f
    (variant-case rator
      [ilambda-exp (ids id body)
       (and (>= (length rands) (length ids))
            (let-exp (append ids (list id))
                      (append (list-head rands (length ids))
                              (list (make-list-exp (list-tail rands (length ids)))))
                      body))]
      [lambda*-exp (arities lambdas)
       (let ([n (length rands)])
         (let loop ([ar arities] [la lambdas])
           (and (pair? ar)
                (let ([cnt (caar ar)] [rest? (cadar ar)])
                  (if (if rest? (>= n cnt) (= n cnt))
                      (let ([clause (car la)])
                        (if rest?
                            (reduce-variadic clause rands)
                            (and (lambda-exp? clause)
                                 (= (length (lambda-exp->ids clause)) n)
                                 (let-exp (lambda-exp->ids clause) rands
                                          (lambda-exp->body clause)))))
                      (loop (cdr ar) (cdr la)))))))]
      [else #f]))
  (define (beta-subst exp substs)
    (define (bs-head exp)
      (variant-case exp
        [var-exp (id)
         (cond [(assq id substs) =>
                (lambda (s) (if (null? (cddr s)) (cadr s) (caddr s)))]
               [else exp])]
        [else (bs exp)]))
    (define (bs exp)
      (variant-case exp
        [quote-exp (obj) exp]
        [var-exp (id) (cond [(assq id substs) => cadr] [else exp])]
        [gvarassign-exp (id e) (gvarassign-exp id (bs e))]
        [varassign-exp (id e) (varassign-exp id (bs e))]
        [if-exp (t h e)
         (let ([t2 (bs t)])
           (if (quote-exp? t2)
               (if (quote-exp->obj t2) (bs h) (bs e))
               (if-exp t2 (bs h) (bs e))))]
        [degenerate-let-exp (body) (bs body)]
        [let-exp (ids rands body)
         (let loop ([in-ids ids] [in-rands (map bs rands)]
                    [out-ids '()] [out-rands '()] [substs substs])
           (define (pass id rand)
             (loop (cdr in-ids) (cdr in-rands) (cons id out-ids) (cons rand out-rands) substs))
           (define (skip ns)
             (loop (cdr in-ids) (cdr in-rands) out-ids out-rands ns))
           (if (null? in-ids)
               (if (null? out-ids) (beta-subst body substs)
                   (let-exp (reverse out-ids) (reverse out-rands) (beta-subst body substs)))
               (let ([id (car in-ids)] [rand (car in-rands)])
                 (let ([refc (var-reference-count id body)])
                   (cond [(= 0 refc)
                          (if (bs-removable-if-dead? rand) (skip substs) (pass id rand))]
                         [(= 1 refc)
                          (if (or (bs-ref-transparent? rand)
                                  (and (not hold-cps-invariants?)
                                       (bs-arg-of-application? id body)))
                              (skip (cons (list id rand) substs)) (pass id rand))]
                         [(and (= 2 refc) (not hold-cps-invariants?)
                               (bs-cheap-ref-transparent? rand))
                          (skip (cons (list id rand) substs))]
                         [(if hold-lifting-invariants?
                              (or (curry-exp? rand) (icurry-exp? rand) (curry*-exp? rand))
                              (bs-cheap-lambda? rand))
                          (if (var-only-applied-in-exp? id body)
                              (skip (cons (list id rand) substs))
                              (loop (cdr in-ids) (cdr in-rands)
                                    (cons id out-ids) (cons rand out-rands)
                                    (cons (list id (var-exp id) rand) substs)))]
                         [else (if (bs-zero-cost-ref-transparent? rand)
                                   (skip (cons (list id rand) substs)) (pass id rand))])))))]
        [primapp-exp (effect prim rands)
         (let ([r2 (map bs rands)])
           (cond [(and (not hold-cps-invariants?) (ref-transparent-effect? effect)
                       (andmap quote-exp? r2) (integer? prim)
                       (safe-apply-integrable prim (map quote-exp->obj r2))) =>
                  (lambda (boxed) (quote-exp (car boxed)))]
                 [else (primapp-exp effect prim r2)]))]
        [app-exp (rator rands)
         (let ([x (app-exp (bs-head rator) (map bs rands))])
           (cond [(let-exp? x) (bs x)]
                 ;; static dispatch: a curry* descriptor in rator
                 ;; position (a known procedure value, e.g. a
                 ;; substituted let-bound closure) with a known
                 ;; argument count goes directly to its case proc.
                 ;; This is the lambda* counterpart of the let-pattern
                 ;; reduction that turns applied curry lambdas into
                 ;; labelapps, and is what keeps known local calls
                 ;; (fix siblings, continuations) as direct jumps.
                 [(and (curry*-exp? (app-exp->rator x))
                       (curry*-static-dispatch (app-exp->rator x) (app-exp->rands x))) =>
                  (lambda (y) (bs y))]
                 [(and (not hold-cps-invariants?)
                       (reduce-variadic (app-exp->rator x) (app-exp->rands x))) =>
                  (lambda (y) (bs y))]
                 [else x]))]
        [fix-exp (ids lams body) (fix-exp ids (map bs lams) (bs body))]
        [lambda-exp (ids body) (lambda-exp ids (bs body))]
        [ilambda-exp (ids id body) (ilambda-exp ids id (bs body))]
        [lambda*-exp (arities lambdas) (lambda*-exp arities (map bs lambdas))]
        [asm-exp (cstr) exp]
        [once-exp (id e) (once-exp id (bs e))]
        [letcc-exp (id body) (letcc-exp id (bs body))]
        [withcc-exp (c e) (withcc-exp (bs c) (bs e))]
        [else exp]))
    (bs exp))
  (beta-subst exp '()))

;;; ------------------------------------------------------------------
;;; lift-quoted-constants: compound quote datums -> private constants.
;;; Immediate datums stay inline (they are S-MARV immediates).

(define (immediate-datum? o)
  (or (boolean? o) (number? o) (char? o) (null? o) (symbol? o)
      (eof-object? o)))

(define (lift-quoted-constants exp)
  (define table '()) ; ((datum . gid) ...)
  (define (intern o)
    (cond [(assoc o table) => cdr]
          [else (let ([g (global-id-private-constant 'lit)])
                  (set! table (cons (cons o g) table)) g)]))
  (define (lift e)
    (variant-case e
      [quote-exp (obj) (if (immediate-datum? obj) e (var-exp (intern obj)))]
      [varassign-exp (id x) (varassign-exp id (lift x))]
      [if-exp (t h a) (if-exp (lift t) (lift h) (lift a))]
      [primapp-exp (effect prim rands) (primapp-exp effect prim (map lift rands))]
      [app-exp (rator rands) (app-exp (lift rator) (map lift rands))]
      [lambda-exp (ids body) (lambda-exp ids (lift body))]
      [ilambda-exp (ids id body) (ilambda-exp ids id (lift body))]
      [lambda*-exp (arities lambdas) (lambda*-exp arities (map lift lambdas))]
      [fix-exp (ids lams body) (fix-exp ids (map lift lams) (lift body))]
      [letcc-exp (id body) (letcc-exp id (lift body))]
      [withcc-exp (c x) (withcc-exp (lift c) (lift x))]
      [once-exp (id x) (once-exp id (lift x))]
      [else e]))
  (let ([e2 (lift exp)])
    (reduce-left (lambda (entry e) (begin-exp (gvarassign-exp (cdr entry) (quote-exp (car entry))) e))
                 e2 table)))

;;; ------------------------------------------------------------------
;;; curry / icurry / curry* virtual patterns (over lambda/ilambda/
;;; lambda* whose body is a labelapp).  A curry's captured rands are
;;; the labelapp arguments beyond the closure's own parameters.

(define (curry-exp tid ids rands)
  (lambda-exp ids (labelapp-exp tid (append (map var-exp ids) rands))))
(define (curry-exp? exp)
  (variant-case exp
    [lambda-exp (ids body)
     (variant-case body
       [labelapp-exp (id rands)
        (let loop ([ids ids] [r rands])
          (or (null? ids)
              (and (pair? r) (var-exp? (car r)) (eq? (var-exp->id (car r)) (car ids))
                   (loop (cdr ids) (cdr r)))))]
       [else #f])]
    [else #f]))
(define (curry-exp->id exp) (labelapp-exp->id (lambda-exp->body exp)))
(define (curry-exp->ids exp) (lambda-exp->ids exp))
(define (curry-exp->rands exp)
  (list-tail (labelapp-exp->rands (lambda-exp->body exp))
             (length (lambda-exp->ids exp))))

(define (icurry-exp tid req-ids rest-id rands)
  (ilambda-exp req-ids rest-id
    (labelapp-exp tid (append (map var-exp (append req-ids (list rest-id))) rands))))
(define (icurry-exp? exp)
  (variant-case exp
    [ilambda-exp (ids id body)
     (variant-case body
       [labelapp-exp (tid rands)
        (let loop ([ids (append ids (list id))] [r rands])
          (or (null? ids)
              (and (pair? r) (var-exp? (car r)) (eq? (var-exp->id (car r)) (car ids))
                   (loop (cdr ids) (cdr r)))))]
       [else #f])]
    [else #f]))
(define (icurry-exp->id exp) (labelapp-exp->id (ilambda-exp->body exp)))
(define (icurry-exp->ids exp) (ilambda-exp->ids exp))
(define (icurry-exp->rest-id exp) (ilambda-exp->id exp))
(define (icurry-exp->rands exp)
  (list-tail (labelapp-exp->rands (ilambda-exp->body exp))
             (+ 1 (length (ilambda-exp->ids exp)))))

(define (curry*-exp arities/tids rands)
  (lambda*-exp (map car arities/tids)
    (map (lambda (a/t)
           (let ([arity (car a/t)] [tid (cadr a/t)])
             (if (cadr arity) ; rest? -> clause is an icurry with cnt params
                 (icurry-exp tid (map (lambda (_) (lexical-id 'a)) (iota 0 (car arity)))
                             (lexical-id 'rest) rands)
                 (curry-exp tid (map (lambda (_) (lexical-id 'a)) (iota 0 (car arity)))
                            rands))))
         arities/tids)))
;; NOTE: curry*-exp above cannot reconstruct clause parameter names;
;; lambda-lift therefore builds lambda*-exp directly (see lift-lams).
(define (curry*-exp? exp)
  (and (lambda*-exp? exp)
       (andmap (lambda (l) (or (curry-exp? l) (icurry-exp? l)))
               (lambda*-exp->lambdas exp))))
(define (curry*-exp->tid/clauses exp) ; => ((arity tid) ...)
  (map (lambda (a l) (list a (if (curry-exp? l) (curry-exp->id l) (icurry-exp->id l))))
       (lambda*-exp->arities exp) (lambda*-exp->lambdas exp)))
(define (curry*-exp->rands exp)
  (let ([l (car (lambda*-exp->lambdas exp))])
    (if (curry-exp? l) (curry-exp->rands l) (icurry-exp->rands l))))

(define (halt-exp e) (primapp-exp (*?!-effect) '%halt (list e)))
(define (halt-exp? e)
  (and (primapp-exp? e) (eq? '%halt (primapp-exp->prim e))))

;;; ------------------------------------------------------------------
;;; cps-convert: single-argument continuations (smarv.html 2b.3).
;;; Every continuation takes exactly one obj; (values) / tuples are
;;; the consumer's problem (call-with-values in the runtime library).


;;; ------------------------------------------------------------------
;;; normalize-lambdas: treat every lambda as a lambda* with a single
;;; case.  After this pass the compiler only ever deals with
;;; lambda*: a fixed-arity lambda is a one-case lambda* whose clause
;;; never checks its argument count, a variadic lambda likewise with
;;; a rest case, and case-lambda already is a lambda*.  The cases
;;; are the future body procs; the lambda* value itself (after
;;; lifting, a curry* closure descriptor listing (cnt rest? $case)
;;; per clause) is the dispatching head.  A call whose destination
;;; is statically known can then be statically dispatched by
;;; convert-global-calls straight to the case proc, while indirect
;;; calls go through the descriptor, which checks/dispatches.


;;; ------------------------------------------------------------------
;;; lambda-lift (sfc 5.sf, generalised to lambda/ilambda/lambda*).
;;; Every lifted body becomes a top-level lambda-exp over
;;; (own-params ... captured-free-vars ...); a lambda* binding becomes
;;; one lifted label per clause sharing the captured variables.


;;; ------------------------------------------------------------------
;;; convert-global-calls: (call (gref G) args...) where G is a global
;;; constant bound to a curry becomes a direct label jump.  The result
;;; is still an app-exp node; gclabelapp-exp? recognises it given the
;;; table built here (kept in *global-curry-table* for the printer /
;;; validator / S-MARV partitioner).

(define *global-curry-table* '()) ; ((gid . curry-exp) ...)

(define (collect-global-curries exp)
  (define table '())
  (define (walk e)
    (variant-case e
      [gvarassign-exp (id x)
       (when (and (global-id-constant? id)
                  (assq id *static-procedures*)
                  (or (curry-exp? x) (icurry-exp? x) (curry*-exp? x)))
         (set! table (cons (cons id x) table)))
       (walk x)]
      [if-exp (t h a) (walk t) (walk h) (walk a)]
      [primapp-exp (effect prim rands) (for-each walk rands)]
      [app-exp (rator rands) (walk rator) (for-each walk rands)]
      [lambda-exp (ids body) (walk body)]
      [ilambda-exp (ids id body) (walk body)]
      [lambda*-exp (arities lambdas) (for-each walk lambdas)]
      [fix-exp (ids lams body) (for-each walk lams) (walk body)]
      [letcc-exp (id body) (walk body)]
      [withcc-exp (c x) (walk c) (walk x)]
      [else (void)]))
  (walk exp) table)

;; Static dispatch: the first clause of a curry* descriptor that
;; accepts n arguments (exact for fixed clauses, at-least for a rest
;; clause), in case-lambda order.  This is the compile-time twin of
;; the run-time dispatcher the descriptor itself performs.
(define (curry*-clause-for exp n) ; => (arity tid) or #f
  (let loop ([a/ts (curry*-exp->tid/clauses exp)])
    (cond [(null? a/ts) #f]
          [(let ([cnt (caaar a/ts)] [rest? (cadaar a/ts)])
             (if rest? (>= n cnt) (= n cnt))) (car a/ts)]
          [else (loop (cdr a/ts))])))

(define (curry*-static-dispatch desc rands) ; descriptor + args -> labelapp or #f
  (let ([cl (curry*-clause-for desc (length rands))])
    (and cl
         (let ([cnt (caar cl)] [rest? (cadar cl)] [tid (cadr cl)])
           (if rest?
               (labelapp-exp tid
                 (append (list-head rands cnt)
                         (list (if (= (length rands) cnt)
                                   (quote-exp '())
                                   (primapp-exp (*-effect) (lookup-integrable 'list)
                                                (list-tail rands cnt))))
                         (curry*-exp->rands desc)))
               (labelapp-exp tid (append rands (curry*-exp->rands desc))))))))

(define (gclabelapp-exp? exp)
  (and (app-exp? exp) (var-exp? (app-exp->rator exp))
       (assq (var-exp->id (app-exp->rator exp)) *global-curry-table*) #t))
(define (gclabelapp-exp->id exp) ; => label id of the direct entry
  (let ([c (cdr (assq (var-exp->id (app-exp->rator exp)) *global-curry-table*))])
    (cond [(curry-exp? c) (curry-exp->id c)]
          [(icurry-exp? c) (icurry-exp->id c)]
          [else (let ([cl (curry*-clause-for c (length (app-exp->rands exp)))])
                  (and cl (cadr cl)))])))


;;; ------------------------------------------------------------------
;;; unbox-values (sfc 5.sf; declarative signatures instead of C text)

(define (id-ctype-in-exp id exp) ; => fx/fl/bool/char, 'none, or #f
  (define (merge a b)
    (cond [(eq? a 'none) b] [(eq? b 'none) a]
          [(not a) #f] [(not b) #f] [(eq? a b) a] [else #f]))
  (define (walk e)
    (variant-case e
      [var-exp (i) (if (eq? i id) #f 'none)] ; bare use: not unboxable
      [primapp-exp (effect prim rands)
       (let loop ([rs rands] [n 0] [acc 'none])
         (if (null? rs) acc
             (let ([r (car rs)])
               (loop (cdr rs) (+ n 1)
                     (merge acc (if (and (var-exp? r) (eq? id (var-exp->id r)))
                                    (or (prim-arg-ctype prim n) #f)
                                    (walk r)))))))]
      [app-exp (rator rands) (reduce-left merge 'none (map walk (cons rator rands)))]
      [if-exp (t h a) (merge (walk t) (merge (walk h) (walk a)))]
      [lambda-exp (ids body) (if (memq id ids) 'none (walk body))]
      [ilambda-exp (ids i body) (if (memq id (cons i ids)) 'none (walk body))]
      [lambda*-exp (arities lambdas) (reduce-left merge 'none (map walk lambdas))]
      [fix-exp (ids lams body)
       (reduce-left merge (walk body) (map walk lams))]
      [gvarassign-exp (i x) (walk x)]
      [varassign-exp (i x) (walk x)]
      [letcc-exp (i body) (if (eq? i id) 'none (walk body))]
      [withcc-exp (c x) (merge (walk c) (walk x))]
      [else 'none]))
  (walk exp))

(define (var-unboxed-ctype id body rand)
  (let ([b (id-ctype-in-exp id body)])
    (and b (not (eq? b 'none)) (memq b '(fx fl bool char))
         (let ([r (exp-ctype rand '())])
           (and r (eq? b r) b)))))


;;; ------------------------------------------------------------------
;;; flatten-nested-lets: canonicalise (let ((x (let ...))) ...) into
;;; sequential bindings, so CUPS let right-hand sides are flat value
;;; forms.  (let ((x (if t a b))) ...) and (let ((x (gset! g v))) ...)
;;; are kept: both are non-call value forms the S-MARV partitioner
;;; lowers directly (branching bind / effect then bind).


;;; ------------------------------------------------------------------
;;; CUPS validation + statistics

(define (cups-stats exp) ; => alist
  (define counts '())
  (define (bump k) (let ([a (assq k counts)])
                     (if a (set-cdr! a (+ 1 (cdr a))) (set! counts (cons (cons k 1) counts)))))
  (define (walk e)
    (cond [(quote-exp? e) (bump 'quote)]
          [(var-exp? e) (bump 'var)]
          [(halt-exp? e) (bump 'halt) (walk (car (primapp-exp->rands e)))]
          [(labelapp-exp? e) (bump 'labelapp) (for-each walk (labelapp-exp->rands e))]
          [(curry-exp? e) (bump 'curry) (for-each walk (curry-exp->rands e))]
          [(icurry-exp? e) (bump 'icurry) (for-each walk (icurry-exp->rands e))]
          [(curry*-exp? e) (bump 'curry*) (for-each walk (curry*-exp->rands e))]
          [(gclabelapp-exp? e) (bump 'gclabelapp) (for-each walk (app-exp->rands e))]
          [(primapp-exp? e) (bump 'primapp) (for-each walk (primapp-exp->rands e))]
          [(if-exp? e) (bump 'if) (walk (if-exp->test-exp e)) (walk (if-exp->then-exp e)) (walk (if-exp->else-exp e))]
          [(let-exp? e) (bump 'let) (for-each walk (let-exp->rands e)) (walk (let-exp->body e))]
          [(begin-exp? e) (bump 'begin) (walk (begin-exp->exp1 e)) (walk (begin-exp->exp2 e))]
          [(app-exp? e) (bump 'app) (walk (app-exp->rator e)) (for-each walk (app-exp->rands e))]
          [(gvarassign-exp? e) (bump 'gset) (walk (gvarassign-exp->exp e))]
          [(fix-exp? e) (bump 'fix) (for-each walk (fix-exp->lams e)) (walk (fix-exp->body e))]
          [(lambda-exp? e) (bump 'lambda) (walk (lambda-exp->body e))]
          [else (bump 'other)]))
  (walk exp) counts)


;;; ------------------------------------------------------------------
;;; .cps serializer.  Format:
;;;   (cups 1 (module <name>)
;;;     (literals (<uname> . <datum>) ...)
;;;     (globals  (<uname> mutable|constant) ...)
;;;     (labels   ($<uname> ((<param> <ctype|obj>) ...) <body>) ...)
;;;     (main <body>))
;;; <body> uses (quote d) (var u) (prim <sym> <effect> a...) (let ...)
;;; (if t h e) (begin ...) (jump $l a...) (gcall $g a...) (call f a...)
;;; (curry $l (p...) (fv...)) (icurry $l (p...) <rest> (fv...))
;;; (curry* ((<cnt> <rest?> $l) ...) (fv...)) (halt a) (void)

(define (cups-source exp)
  (define (body-list e)
    (cond [(begin-exp? e) (cons (src (begin-exp->exp1 e)) (body-list (begin-exp->exp2 e)))]
          [(degenerate-let-exp? e) (body-list (degenerate-let-exp->body e))]
          [else (list (src e))]))
  (define (src e)
    (cond [(void-exp? e) '(void)]
          [(halt-exp? e) (list 'halt (src (car (primapp-exp->rands e))))]
          [(quote-exp? e) (list 'quote (quote-exp->obj e))]
          [(var-exp? e) (list 'var (id->uname (var-exp->id e)))]
          [(gvarassign-exp? e) (list 'gset (id->uname (gvarassign-exp->id e)) (src (gvarassign-exp->exp e)))]
          [(varassign-exp? e) (list 'set (id->uname (varassign-exp->id e)) (src (varassign-exp->exp e)))]
          [(if-exp? e) (list 'if (src (if-exp->test-exp e)) (src (if-exp->then-exp e)) (src (if-exp->else-exp e)))]
          [(labelapp-exp? e) (cons 'jump (cons (id->uname (labelapp-exp->id e)) (map src (labelapp-exp->rands e))))]
          [(curry-exp? e) (list 'curry (id->uname (curry-exp->id e))
                                (map id->uname (curry-exp->ids e)) (map src (curry-exp->rands e)))]
          [(icurry-exp? e) (list 'icurry (id->uname (icurry-exp->id e))
                                 (map id->uname (icurry-exp->ids e)) (id->uname (icurry-exp->rest-id e))
                                 (map src (icurry-exp->rands e)))]
          [(curry*-exp? e) (list 'curry*
                                 (map (lambda (a/t) (list (caar a/t) (cadar a/t) (id->uname (cadr a/t))))
                                      (curry*-exp->tid/clauses e))
                                 (map src (curry*-exp->rands e)))]
          [(gclabelapp-exp? e) (cons 'gcall (cons (id->uname (var-exp->id (app-exp->rator e))) (map src (app-exp->rands e))))]
          [(primapp-exp? e) (list* 'prim (or (prim->sym (primapp-exp->prim e)) (primapp-exp->prim e))
                                   (primapp-exp->effect e) (map src (primapp-exp->rands e)))]
          [(let-exp? e) (list* 'let (map (lambda (id r) (list (id->uname id) (src r)))
                                         (let-exp->ids e) (let-exp->rands e))
                               (body-list (let-exp->body e)))]
          [(begin-exp? e) (cons 'begin (body-list e))]
          [(app-exp? e) (cons 'call (cons (src (app-exp->rator e)) (map src (app-exp->rands e))))]
          [(fix-exp? e) (list 'fix (map id->uname (fix-exp->ids e)) (src (fix-exp->body e)))]
          [(lambda-exp? e) (list* 'lambda (map id->uname (lambda-exp->ids e)) (body-list (lambda-exp->body e)))]
          [else (list 'unknown e)]))
  (src exp))

(define (collect-literals exp) ; ((uname . datum) ...) from private consts
  (define lits '())
  (define (walk e)
    (cond [(and (gvarassign-exp? e) (global-id-private-constant? (gvarassign-exp->id e))
                (quote-exp? (gvarassign-exp->exp e)))
           (set! lits (cons (cons (id->uname (gvarassign-exp->id e))
                                  (quote-exp->obj (gvarassign-exp->exp e))) lits))]
          [(fix-exp? e) (for-each walk (fix-exp->lams e)) (walk (fix-exp->body e))]
          [(lambda-exp? e) (walk (lambda-exp->body e))]
          [(let-exp? e) (for-each walk (let-exp->rands e)) (walk (let-exp->body e))]
          [(begin-exp? e) (walk (begin-exp->exp1 e)) (walk (begin-exp->exp2 e))]
          [(if-exp? e) (walk (if-exp->test-exp e)) (walk (if-exp->then-exp e)) (walk (if-exp->else-exp e))]
          [(app-exp? e) (walk (app-exp->rator e)) (for-each walk (app-exp->rands e))]
          [(primapp-exp? e) (for-each walk (primapp-exp->rands e))]))
  (walk exp) (reverse lits))

(define (collect-globals exp)
  (define gs '())
  (define (note id)
    (unless (assq (id->uname id) gs)
      (set! gs (cons (list (id->uname id)
                           (if (global-id-constant? id) 'constant
                               (if (global-id-private-constant? id) 'literal 'mutable))) gs))))
  (define (walk e)
    (cond [(var-exp? e) (when (global-id? (var-exp->id e)) (note (var-exp->id e)))]
          [(gvarassign-exp? e) (note (gvarassign-exp->id e)) (walk (gvarassign-exp->exp e))]
          [(fix-exp? e) (for-each walk (fix-exp->lams e)) (walk (fix-exp->body e))]
          [(lambda-exp? e) (walk (lambda-exp->body e))]
          [(ilambda-exp? e) (walk (ilambda-exp->body e))]
          [(lambda*-exp? e) (for-each walk (lambda*-exp->lambdas e))]
          [(let-exp? e) (for-each walk (let-exp->rands e)) (walk (let-exp->body e))]
          [(begin-exp? e) (walk (begin-exp->exp1 e)) (walk (begin-exp->exp2 e))]
          [(if-exp? e) (walk (if-exp->test-exp e)) (walk (if-exp->then-exp e)) (walk (if-exp->else-exp e))]
          [(app-exp? e) (walk (app-exp->rator e)) (for-each walk (app-exp->rands e))]
          [(primapp-exp? e) (for-each walk (primapp-exp->rands e))]
          [(letcc-exp? e) (walk (letcc-exp->body e))]
          [(withcc-exp? e) (walk (withcc-exp->cont-exp e)) (walk (withcc-exp->exp e))]))
  (walk exp) (reverse gs))

(define (write-cups exp modname oport)
  (unless (fix-exp? exp) (error "write-cups: not a CUPS fix-exp" exp))
  (display "(cups 1\n" oport)
  (display "  (module " oport) (write modname oport) (display ")\n" oport)
  (display "  (literals" oport)
  (for-each (lambda (l) (newline oport) (display "    (" oport) (write (car l) oport)
              (display " . " oport) (write (cdr l) oport) (display ")" oport))
            (collect-literals exp))
  (display ")\n  (globals" oport)
  (for-each (lambda (g) (newline oport) (display "    " oport) (write g oport)) (collect-globals exp))
  (display ")\n  (labels" oport)
  (for-each (lambda (id lam)
              (newline oport) (display "    (" oport) (write (id->uname id) oport)
              (display " (" oport)
              (for-each (lambda (p) (write (list (id->uname p) (or (and (cvar-id? p) (id->ctype p)) 'obj)) oport) (display " " oport))
                        (lambda-exp->ids lam))
              (display ")\n      " oport)
              (write (cups-source (lambda-exp->body lam)) oport)
              (display ")" oport))
            (fix-exp->ids exp) (fix-exp->lams exp))
  (display ")\n  (main " oport)
  (write (cups-source (fix-exp->body exp)) oport)
  (display "))\n" oport))
