;;; base.sld -- (scheme base) as a real library, for compiled programs.
;;;
;;; Shadow counterpart of Skint's predefined (scheme base): the predefined
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
;;; (scheme base) is the canonical shadow: satellite libraries
;;; re-export its bindings for shared names instead of defining
;;; their own, so importing several libraries never produces
;;; duplicate-binding conflicts.  The depth-2 cxr names are
;;; re-exported from (scheme cxr) for the same reason.

(define-library (scheme base)
  (import (only (skint)
            define define-syntax lambda lambda* body letcc if begin
            quote set! set& syntax-rules syntax-lambda syntax-error
            syntax-length include include-ci cond-expand
            ;; and/or are re-exported unchanged, NOT redefined below:
            ;; cond-expand's compound requirements are recognised by
            ;; free-identifier comparison against the root bindings,
            ;; so a library-local macro named and/or/not would make
            ;; (and ...) / (or ...) / (not ...) feature requirements
            ;; invalid in every importing program or library.
            and or not
            else => unquote unquote-splicing library _ ...
            box fixnum? asse meme list-cat
            fx+ fx- fx=? fx<? fx<=? fx>? fx>=?)
          (only (skint hidden)
            %port?
            %current-input-port %current-output-port %current-error-port
            %set-current-input-port! %set-current-output-port!
            %set-current-error-port! apply-to-list
            %exit)
          (rename (only (skint) make-record record?)
            (make-record %builtin-make-record)
            (record? %builtin-record?))
          (only (scheme cxr) caar cadr cdar cddr)
          (rename (only (skint) zero? write-char vector? vector-set! vector-ref vector-length truncate-remainder truncate-quotient truncate symbol? symbol->string substring string? string=? string-set! string-ref string-length string->symbol string->number set-cdr! set-car! round reverse remainder read-char quotient procedure? positive? peek-char pair? output-port? output-port-open? open-output-string open-output-bytevector open-input-string open-input-bytevector number? number->string null? not newline negative? modulo min memv memq max make-vector make-string make-list make-bytevector list? list-tail list-ref list->string length integer? integer->char input-port? input-port-open? inexact? inexact get-output-string get-output-bytevector flush-output-port floor-remainder floor-quotient floor expt exact? exact-integer? exact eqv? equal? eq? eof-object? eof-object cons close-output-port close-input-port char? char>? char>=? char=? char<? char<=? char->integer ceiling cdr car bytevector? bytevector-u8-set! bytevector-u8-ref bytevector-length boolean? boolean=? assv assq abs >= > = <= < / - + * values call-with-values)
            (zero? %builtin-zero?)
            (write-char %builtin-write-char)
            (vector? %builtin-vector?)
            (vector-set! %builtin-vector-set!)
            (vector-ref %builtin-vector-ref)
            (vector-length %builtin-vector-length)
            (truncate-remainder %builtin-truncate-remainder)
            (truncate-quotient %builtin-truncate-quotient)
            (truncate %builtin-truncate)
            (symbol? %builtin-symbol?)
            (symbol->string %builtin-symbol->string)
            (substring %builtin-substring)
            (string? %builtin-string?)
            (string=? %builtin-string=?)
            (string-set! %builtin-string-set!)
            (string-ref %builtin-string-ref)
            (string-length %builtin-string-length)
            (string->symbol %builtin-string->symbol)
            (string->number %builtin-string->number)
            (set-cdr! %builtin-set-cdr!)
            (set-car! %builtin-set-car!)
            (round %builtin-round)
            (reverse %builtin-reverse)
            (remainder %builtin-remainder)
            (read-char %builtin-read-char)
            (quotient %builtin-quotient)
            (procedure? %builtin-procedure?)
            (positive? %builtin-positive?)
            (peek-char %builtin-peek-char)
            (pair? %builtin-pair?)
            (output-port? %builtin-output-port?)
            (output-port-open? %builtin-output-port-open?)
            (open-output-string %builtin-open-output-string)
            (open-output-bytevector %builtin-open-output-bytevector)
            (open-input-string %builtin-open-input-string)
            (open-input-bytevector %builtin-open-input-bytevector)
            (number? %builtin-number?)
            (number->string %builtin-number->string)
            (null? %builtin-null?)
            (not %builtin-not)
            (newline %builtin-newline)
            (negative? %builtin-negative?)
            (modulo %builtin-modulo)
            (min %builtin-min)
            (memv %builtin-memv)
            (memq %builtin-memq)
            (max %builtin-max)
            (make-vector %builtin-make-vector)
            (make-string %builtin-make-string)
            (make-list %builtin-make-list)
            (make-bytevector %builtin-make-bytevector)
            (list? %builtin-list?)
            (list-tail %builtin-list-tail)
            (list-ref %builtin-list-ref)
            (list->string %builtin-list->string)
            (length %builtin-length)
            (integer? %builtin-integer?)
            (integer->char %builtin-integer->char)
            (input-port? %builtin-input-port?)
            (input-port-open? %builtin-input-port-open?)
            (inexact? %builtin-inexact?)
            (inexact %builtin-inexact)
            (get-output-string %builtin-get-output-string)
            (get-output-bytevector %builtin-get-output-bytevector)
            (flush-output-port %builtin-flush-output-port)
            (floor-remainder %builtin-floor-remainder)
            (floor-quotient %builtin-floor-quotient)
            (floor %builtin-floor)
            (expt %builtin-expt)
            (exact? %builtin-exact?)
            (exact-integer? %builtin-exact-integer?)
            (exact %builtin-exact)
            (eqv? %builtin-eqv?)
            (equal? %builtin-equal?)
            (eq? %builtin-eq?)
            (eof-object? %builtin-eof-object?)
            (eof-object %builtin-eof-object)
            (cons %builtin-cons)
            (close-output-port %builtin-close-output-port)
            (close-input-port %builtin-close-input-port)
            (char? %builtin-char?)
            (char>? %builtin-char>?)
            (char>=? %builtin-char>=?)
            (char=? %builtin-char=?)
            (char<? %builtin-char<?)
            (char<=? %builtin-char<=?)
            (char->integer %builtin-char->integer)
            (ceiling %builtin-ceiling)
            (cdr %builtin-cdr)
            (car %builtin-car)
            (bytevector? %builtin-bytevector?)
            (bytevector-u8-set! %builtin-bytevector-u8-set!)
            (bytevector-u8-ref %builtin-bytevector-u8-ref)
            (bytevector-length %builtin-bytevector-length)
            (boolean? %builtin-boolean?)
            (boolean=? %builtin-boolean=?)
            (assv %builtin-assv)
            (assq %builtin-assq)
            (abs %builtin-abs)
            (>= %builtin->=)
            (> %builtin->)
            (= %builtin-=)
            (<= %builtin-<=)
            (< %builtin-<)
            (/ %builtin-/)
            (- %builtin--)
            (+ %builtin-+)
            (* %builtin-*)
            (values %builtin-values)
            (call-with-values %builtin-call-with-values))
          (rename (only (skint) apply) (apply %builtin-apply)))

  (export
    syntax-rules
    syntax-error
    set!
    quote
    lambda
    if
    define-syntax
    define
    begin
    _
    ...
    (rename %cover-zero? zero?)
    (rename %cover-write-char write-char)
    (rename %cover-vector? vector?)
    (rename %cover-vector-set! vector-set!)
    (rename %cover-vector-ref vector-ref)
    (rename %cover-vector-length vector-length)
    (rename %cover-truncate-remainder truncate-remainder)
    (rename %cover-truncate-quotient truncate-quotient)
    (rename %cover-truncate truncate)
    (rename %cover-symbol? symbol?)
    (rename %cover-symbol->string symbol->string)
    (rename %cover-substring substring)
    (rename %cover-string? string?)
    (rename %cover-string=? string=?)
    (rename %cover-string-set! string-set!)
    (rename %cover-string-ref string-ref)
    (rename %cover-string-length string-length)
    (rename %cover-string->symbol string->symbol)
    (rename %cover-string->number string->number)
    (rename %cover-set-cdr! set-cdr!)
    (rename %cover-set-car! set-car!)
    (rename %cover-round round)
    (rename %cover-reverse reverse)
    (rename %cover-remainder remainder)
    (rename %cover-read-char read-char)
    (rename %cover-quotient quotient)
    (rename %cover-procedure? procedure?)
    (rename %cover-positive? positive?)
    (rename %cover-peek-char peek-char)
    (rename %cover-pair? pair?)
    (rename %cover-output-port? output-port?)
    (rename %cover-output-port-open? output-port-open?)
    (rename %cover-open-output-string open-output-string)
    (rename %cover-open-output-bytevector open-output-bytevector)
    (rename %cover-open-input-string open-input-string)
    (rename %cover-open-input-bytevector open-input-bytevector)
    (rename %cover-number? number?)
    (rename %cover-number->string number->string)
    (rename %cover-null? null?)
    not
    (rename %cover-newline newline)
    (rename %cover-negative? negative?)
    (rename %cover-modulo modulo)
    (rename %cover-min min)
    (rename %cover-memv memv)
    (rename %cover-memq memq)
    (rename %cover-max max)
    (rename %cover-make-vector make-vector)
    (rename %cover-make-string make-string)
    (rename %cover-make-list make-list)
    (rename %cover-make-bytevector make-bytevector)
    (rename %cover-list? list?)
    (rename %cover-list-tail list-tail)
    (rename %cover-list-ref list-ref)
    (rename %cover-list->string list->string)
    (rename %cover-length length)
    (rename %cover-integer? integer?)
    (rename %cover-integer->char integer->char)
    (rename %cover-input-port? input-port?)
    (rename %cover-input-port-open? input-port-open?)
    (rename %cover-inexact? inexact?)
    (rename %cover-inexact inexact)
    (rename %cover-get-output-string get-output-string)
    (rename %cover-get-output-bytevector get-output-bytevector)
    (rename %cover-flush-output-port flush-output-port)
    (rename %cover-floor-remainder floor-remainder)
    (rename %cover-floor-quotient floor-quotient)
    (rename %cover-floor floor)
    (rename %cover-expt expt)
    (rename %cover-exact? exact?)
    (rename %cover-exact-integer? exact-integer?)
    (rename %cover-exact exact)
    (rename %cover-eqv? eqv?)
    (rename %cover-equal? equal?)
    (rename %cover-eq? eq?)
    (rename %cover-eof-object? eof-object?)
    (rename %cover-eof-object eof-object)
    (rename %cover-cons cons)
    (rename %cover-close-output-port close-output-port)
    (rename %cover-close-input-port close-input-port)
    (rename %cover-char? char?)
    (rename %cover-char>? char>?)
    (rename %cover-char>=? char>=?)
    (rename %cover-char=? char=?)
    (rename %cover-char<? char<?)
    (rename %cover-char<=? char<=?)
    (rename %cover-char->integer char->integer)
    (rename %cover-ceiling ceiling)
    (rename %cover-cdr cdr)
    (rename %cover-car car)
    (rename %cover-bytevector? bytevector?)
    (rename %cover-bytevector-u8-set! bytevector-u8-set!)
    (rename %cover-bytevector-u8-ref bytevector-u8-ref)
    (rename %cover-bytevector-length bytevector-length)
    (rename %cover-boolean? boolean?)
    (rename %cover-boolean=? boolean=?)
    (rename %cover-assv assv)
    (rename %cover-assq assq)
    (rename %cover-abs abs)
    (rename %cover->= >=)
    (rename %cover-> >)
    (rename %cover-= =)
    (rename %cover-<= <=)
    (rename %cover-< <)
    (rename %cover-/ /)
    (rename %cover-- -)
    (rename %cover-+ +)
    (rename %cover-* *)
    bytevector
    bytevector-append
    char-ready?
    complex?
    denominator
    even?
    gcd
    list
    list->vector
    list-copy
    list-set!
    numerator
    odd?
    peek-u8
    rational?
    read-line
    read-u8
    real?
    string
    string-append
    string<=?
    string<?
    string>=?
    string>?
    symbol=?
    u8-ready?
    vector
    vector-append
    write-u8
    caar
    cadr
    cdar
    cddr
    and
    append
    apply
    assoc
    call-with-current-continuation
    call/cc
    case
    cond
    current-error-port
    current-input-port
    current-output-port
    define-record-type
    define-values
    do
    for-each
    guard
    let
    let*
    let*-values
    let-syntax
    let-values
    letrec
    letrec*
    letrec-syntax
    map
    member
    or
    parameterize
    quasiquote
    string->list
    unless
    vector->list
    when
    include
    include-ci
    cond-expand
    binary-port?
    bytevector-copy
    bytevector-copy!
    call-with-port
    close-port
    exact-integer-sqrt
    file-error?
    floor/
    lcm
    make-parameter
    port?
    raise
    raise-continuable
    rationalize
    read-bytevector
    read-bytevector!
    read-error?
    read-string
    square
    string->utf8
    string->vector
    string-copy
    string-copy!
    string-fill!
    string-for-each
    string-map
    textual-port?
    truncate/
    utf8->string
    vector->string
    vector-copy
    vector-copy!
    vector-fill!
    vector-for-each
    vector-map
    with-exception-handler
    write-bytevector
    write-string
    error-object?
    error-object-message
    error-object-irritants
    (rename %cover-values values)
    (rename %cover-call-with-values call-with-values)
    features
    =>
    else
    library
    unquote
    unquote-splicing
    error
    dynamic-wind)

  (begin
    ;; the variadic constructors are ordinary Scheme, so no
    ;; backend ever has to materialize them as host procedures
    (define (list . args) args)

    (define (vector . args) (list->vector args))

    (define (string . args) (list->string args))

    (define (string-append . args)
      (let ([s (make-string
                (let loop ([n 0] [args args])
                  (if (null? args) n
                      (loop (+ n (string-length (car args)))
                            (cdr args)))))])
        (let loop ([pos 0] [args args])
          (cond [(null? args) s]
                [else (string-copy! s pos (car args))
                      (loop (+ pos (string-length (car args)))
                            (cdr args))]))))

(define-syntax let-syntax
  (syntax-rules ()
    [(_ ([kw init] ...))
     (begin)]
    [(_ ([kw init] ...) . forms)
     ((syntax-lambda (kw ...) . forms)
      init ...)]))

(define-syntax letrec-syntax
  (syntax-rules ()
    [(_ ([key trans] ...) . forms) 
     (body (define-syntax key trans) ... . forms)]))

(define-syntax letrec
  (syntax-rules ()
    [(_ ([var init] ...) . forms)
     (body (define var init) ... . forms)]))

(define-syntax letrec*
  (syntax-rules ()
    [(_ ([var expr] ...) . forms)
     (let ([var #f] ...)
       (set! var expr)
       ...
       (body . forms))]))

(define-syntax let
  (syntax-rules ()
    [(_ () . forms) (body . forms)]
    [(_ ([var init] ...) . forms)
     ((lambda (var ...) . forms) init ...)]
    [(_ name ([var init] ...) . forms)
     ((letrec ((name (lambda (var ...) . forms))) name) init ...)]))

(define-syntax let*
  (syntax-rules ()
    [(_ () . forms) 
     (body . forms)]
    [(_ (first . more) . forms)
     (let (first) (let* more . forms))]))

(define-syntax %let-values-loop
  (syntax-rules ()
    [(_ (new-b ...) new-aa x map-b* () () . forms)
     (let*-values (new-b ... [new-aa x]) (let map-b* . forms))]
    [(_ (new-b ...) new-aa old-x map-b* () ([aa x] . b*) . forms)
     (%let-values-loop (new-b ... [new-aa old-x]) () x map-b* aa b* . forms)]
    [(_ new-b* (new-a ...) x (map-b ...) (a . aa) b* . forms)
     (%let-values-loop new-b* (new-a ... tmp-a) x (map-b ... [a tmp-a]) aa b* . forms)]
    [(_ new-b* (new-a ...) x (map-b ...) a b* . forms) 
     (%let-values-loop new-b* (new-a ... . tmp-a) x (map-b ... [a tmp-a]) () b* . forms)]))

(define-syntax let*-values
  (syntax-rules ()
    [(_ () . forms) (body . forms)]
    [(_ ([(a) x] . b*) . forms) (let ([a x]) (let*-values b* . forms))]
    [(_ ([aa x] . b*) . forms) (call-with-values (lambda () x) (lambda aa (let*-values b* . forms)))]))

(define-syntax let-values
  (syntax-rules ()
    [(_ () . forms) (let () . forms)]
    [(_ ([aa x] . b*) . forms)
     (%let-values-loop () () x () aa b* . forms)]))

(define-syntax %define-values-loop
  (syntax-rules ()
    [(_ new-aa ([a tmp-a] ...) () x)
     (begin
       (define a (begin)) ...
       (define () (call-with-values (lambda () x) (lambda new-aa (set! a tmp-a) ...))))]
    [(_ (new-a ...) (map-a ...) (a . aa) x) 
     (%define-values-loop (new-a ... tmp-a) (map-a ... [a tmp-a]) aa x)]
    [(_ (new-a ...) (map-a ...) a x) 
     (%define-values-loop (new-a ... . tmp-a) (map-a ... [a tmp-a]) () x)]))

(define-syntax define-values
  (syntax-rules ()
    [(_ () x) (define () (call-with-values (lambda () x) (lambda ())))] ; use idless define
    [(_ aa x) (%define-values-loop () () aa x)]))

;; and/or: re-exported from (skint) at the top, deliberately not
;; redefined (a local binding would break cond-expand's compound
;; feature requirements in importing code; see the import note).

(define-syntax cond ; + body support
  (syntax-rules (else =>)
    [(_) #f]
    [(_ (else . exps)) (body . exps)]
    [(_ (x) . rest) (or x (cond . rest))]
    [(_ (x => proc) . rest) (let ([tmp x]) (cond [tmp (proc tmp)] . rest))]
    [(_ (x . exps) . rest) (if x (body . exps) (cond . rest))]))

(define-syntax %case-test
  (syntax-rules () 
    [(_ k ()) #f]
    [(_ k (datum)) (eqv? k 'datum)] 
    [(_ k data) (memv k 'data)]))

(define-syntax %case ; + body support
  (syntax-rules (else =>)
    [(_ key) (begin)]
    [(_ key (else => resproc))
     (resproc key)]
    [(_ key (else expr ...))
     (body expr ...)]
    [(_ key ((datum ...) => resproc) . clauses)
     (if (%case-test key (datum ...))
         (resproc key)
         (%case key . clauses))]
    [(_ key ((datum ...) expr ...) . clauses)
     (if (%case-test key (datum ...))
         (body expr ...)
         (%case key . clauses))]))

(define-syntax case ; + body support
  (syntax-rules ()
    [(_ x . clauses) (let ([key x]) (%case key . clauses))]))

(define-syntax %do-step 
  (syntax-rules () 
    [(_ x) x] [(_ x y) y]))

(define-syntax do ; + body support
  (syntax-rules ()
    [(_ ([var init step ...] ...)
       [test expr ...]
       command ...)
     (let loop ([var init] ...)
       (if test
           (body expr ...)
           (let () command ... 
             (loop (%do-step var step ...) ...))))]))

(define-syntax quasiquote
  (syntax-rules (unquote unquote-splicing quasiquote quote)
    [(_ x) (quasiquote 0 () x 7)]
    [(_ 0 () ,x s . a*) (quasiquote s x . a*)]
    [(_ 0 () (,@x) s . a*) (quasiquote s x . a*)] ; popular extension
    [(_ 0 () (,@x . y) s . a*) (quasiquote 0 () y 1 x s . a*)]
    [(_ 0 d `x s . a*) (quasiquote 0 (d) (x) 2 quasiquote s . a*)]
    [(_ 0 (d) ,x s . a*) (quasiquote 0 d (x) 2 unquote s . a*)]
    [(_ 0 (d) ,@x s . a*) (quasiquote 0 d (x) 2 unquote-splicing s . a*)]
    [(_ 0 d (x . y) s . a*) (quasiquote 0 d x 3 d y s . a*)]
    [(_ 0 d #(x ...) s . a*) (quasiquote 0 d (x ...) 5 s . a*)]
    [(_ 0 d #&x s . a*) (quasiquote 0 d x 6 s . a*)]
    [(_ 0 d x s . a*) (quasiquote s 'x . a*)]
    [(_ 1 'y '(x ...) s . a*) (quasiquote s '(x ... . y) . a*)]
    [(_ 1 y x s . a*) (quasiquote s (append x y) . a*)]
    [(_ 2 'y c s . a*) (quasiquote s '(c . y) . a*)]
    [(_ 2 y c s . a*) (quasiquote s (cons 'c y) . a*)]
    [(_ 3 x d y s . a*) (quasiquote 0 d y 4 x s . a*)]
    [(_ 4 'y 'x s . a*) (quasiquote s '(x . y) . a*)]
    [(_ 4 y x s . a*) (quasiquote s (cons x y) . a*)]
    [(_ 5 '(x ...) s . a*) (quasiquote s '#(x ...) . a*)]
    [(_ 5 x s . a*) (quasiquote s (list->vector x) . a*)]
    [(_ 6 'x s . a*) (quasiquote s '#&x . a*)]
    [(_ 6 x s . a*) (quasiquote s (box x) . a*)]
    [(_ 7 x) x]))

(define-syntax when ; + body support
  (syntax-rules ()
    [(_ test . rest) (if test (body . rest))]))

(define-syntax unless ; + body support
  (syntax-rules ()
    [(_ test . rest) (if (not test) (body . rest))]))

(define-syntax %parameterize-loop
  (syntax-rules ()
    [(_ ([param value p old new] ...) () body)
     (let ([p param] ...)
       (let ([old (p)] ... [new (p value #f)] ...)
         (dynamic-wind
           (lambda () (p new #t) ...)
           (lambda () . body)
           (lambda () (p old #t) ...))))]
    [(_ args ([param value] . rest) body)
     (%parameterize-loop ([param value p old new] . args) rest body)]))

(define-syntax parameterize
  (syntax-rules ()
    [(_ ([param value] ...) . body)
     (%parameterize-loop () ([param value] ...) body)]))

(define-syntax %id-eq?? 
  (syntax-rules ()
    [(_ id b kt kf)
     ((syntax-lambda (id ok) ((syntax-rules () [(_ b) (id)]) ok))
      (syntax-rules () [(_) kf]) (syntax-rules () [(_) kt]))]))

(define-syntax %id-assq??
  (syntax-rules ()
    [(_ id () kt kf) 
     kf]
    [(_ id ([id0 . r0] . idr*) kt kf) 
     (%id-eq?? id id0 (kt . r0) (%id-assq?? id idr* kt kf))]))

(define-syntax %drt-init
  (syntax-rules ()
    [(_  r () fi* (x ...)) 
     (begin x ... r)]
    [(_  r (id0 . id*) fi* (x ...))
     (%id-assq?? id0 fi* 
       (syntax-rules () [(_ i0) (%drt-init r id* fi* (x ... (record-set! r i0 id0)))]) 
       (syntax-error "id in define-record-type constructor is not a field:" id0))]))

(define-syntax %drt-unroll
  (syntax-rules ()
    [(_ rtn (consn id ...) predn () ([f i] ...) ([a ia] ...) ([m im] ...))
     (begin
       (define rtn 
         (new-record-type 'rtn '(f ...)))
       (define consn 
         (lambda (id ...) 
           (let ([r (make-record rtn (syntax-length (f ...)))])
             (%drt-init r (id ...) ([f i] ...) ()))))
       (define predn 
         (lambda (obj) (record? obj rtn)))
       (define a 
         (lambda (obj) (record-ref obj ia)))
       ...
       (define m 
         (lambda (obj val) (record-set! obj im val)))
       ...)]
    [(_ rtn cf* predn ([fn accn] fam ...) (fi ...) (ai ...) (mi ...))
     (%drt-unroll rtn cf* predn (fam ...) 
       (fi ... [fn (syntax-length (fi ...))]) 
       (ai ... [accn (syntax-length (fi ...))]) 
       (mi ...))]
    [(_  rtn cf* predn ([fn accn modn] fam ...) (fi ...) (ai ...) (mi ...))
     (%drt-unroll rtn cf* predn (fam ...) 
       (fi ... [fn (syntax-length (fi ...))]) 
       (ai ... [accn (syntax-length (fi ...))]) 
       (mi ... [modn (syntax-length (fi ...))]))]))

(define-syntax define-record-type
  (syntax-rules ()
    [(_ rtn (consn id ...) predn (fn . am) ...)
     (%drt-unroll rtn (consn id ...) predn ((fn . am) ...) () () ())]))

(define-syntax %guard-aux ; + body support
  (syntax-rules (else =>)
    [(_ reraise (else result1 result2 ...))
     (body result1 result2 ...)]
    [(_ reraise (test => result))
     (let ([temp test]) (if temp (result temp) reraise))]
    [(_ reraise (test => result) clause1 clause2 ...)
     (let ([temp test])
       (if temp
           (result temp)
           (%guard-aux reraise clause1 clause2 ...)))]
    [(_ reraise (test)) (or test reraise)]
    [(_ reraise (test) clause1 clause2 ...)
     (let ([temp test])
       (if temp temp (%guard-aux reraise clause1 clause2 ...)))]
    [(_ reraise (test result1 result2 ...))
     (if test (body result1 result2 ...) reraise)]
    [(_ reraise (test result1 result2 ...) clause1 clause2 ...)
     (if test
         (body result1 result2 ...)
         (%guard-aux reraise clause1 clause2 ...))]))

(define-syntax guard
  (syntax-rules ()
    [(guard (var clause ...) e1 e2 ...)
    ((call/cc
      (lambda (guard-k)
        (with-exception-handler
          (lambda (condition)
            ((call/cc
                (lambda (handler-k)
                  (guard-k
                    (lambda ()
                      (let ([var condition])
                        (%guard-aux
                          (handler-k
                            (lambda ()
                              (raise-continuable condition)))
                          clause
                          ...))))))))
          (lambda ()
            (call-with-values
              (lambda () e1 e2 ...)
              (lambda args
                (guard-k (lambda () (apply values args))))))))))]))

(define %current-input-port-parameter
  (case-lambda 
    [() (%current-input-port)]
    [(p) (%set-current-input-port! p)]
    [(p s) (if s (%set-current-input-port! p) p)]))

(define-syntax current-input-port
  (syntax-rules ()
    [(_) (%current-input-port)]
    [(_ p) (%set-current-input-port! p)]
    [(_ . r) (%current-input-port-parameter . r)]
    [_ %current-input-port-parameter]))

(define %current-output-port-parameter
  (case-lambda 
    [() (%current-output-port)]
    [(p) (%set-current-output-port! p)]
    [(p s) (if s (%set-current-output-port! p) p)]))

(define-syntax current-output-port
  (syntax-rules ()
    [(_) (%current-output-port)]
    [(_ p) (%set-current-output-port! p)]
    [(_ . r) (%current-output-port-parameter . r)]
    [_ %current-output-port-parameter]))

(define %current-error-port-parameter
  (case-lambda 
    [() (%current-error-port)]
    [(p) (%set-current-error-port! p)]
    [(p s) (if s (%set-current-error-port! p) p)]))

(define-syntax current-error-port
  (syntax-rules ()
    [(_) (%current-error-port)]
    [(_ p) (%set-current-error-port! p)]
    [(_ . r) (%current-error-port-parameter . r)]
    [_ %current-error-port-parameter]))

(define-syntax append
  (syntax-rules ()
    [(_) '()] [(_ x) x]
    [(_ x y) (list-cat x y)]
    [(_ x y z ...) (list-cat x (append y z ...))]
    [_ %append]))

(define-syntax member
  (syntax-rules ()
    [(_ v l) (meme v l)] 
    [(_ . args) (%member . args)]
    [_ %member]))

(define-syntax assoc
  (syntax-rules ()
    [(_ v al) (asse v al)] 
    [(_ . args) (%assoc . args)]
    [_ %assoc]))

(define-syntax apply
  (syntax-rules ()
    [(_ p l) (apply-to-list p l)]
    [(_ p a b ... l) (apply-to-list p (list* a b ... l))]
    [(_ . args) (%apply . args)]
    [_ %apply]))

;; call-with-current-continuation gets its own copy of the call/cc
;; cover macro below, not (define-syntax <name> call/cc): a syntax
;; alias does not survive DS library flattening -- uses of the
;; alias turn into a free global reference to call/cc.  The escape
;; procedure is variadic in both macros: a continuation may be
;; invoked with any number of values; one value is delivered as-is,
;; any other count is packed into the values tuple that
;; call-with-values consumers already unpack.  (Skint's SRFI-1,
;; %cars+cdrs, aborts with (abort '() '()) and relies on this.)
(define-syntax call-with-current-continuation
  (syntax-rules ()
    [(_ p) (let ([saved *winders*])
             (letcc k
               (p (lambda vals
                    (%do-wind saved)
                    (k (if (and (pair? vals) (null? (cdr vals)))
                           (car vals)
                           (apply values vals)))))))]
    [(_ . args) (%call/cc+wind . args)]
    [_ %call/cc+wind]))

(define-syntax map
  (syntax-rules ()
    [(_ p l) (%map1 p l)]
    [(_ p l1 l2) (%map2 p l1 l2)]
    [(_ . args) (%map . args)]
    [_ %map]))

(define-syntax for-each
  (syntax-rules ()
    [(_ p l) (%for-each1 p l)]
    [(_ p l1 l2) (%for-each2 p l1 l2)]
    [(_ . args) (%for-each . args)]
    [_ %for-each]))

(define-syntax string->list
  (syntax-rules ()
    [(_ x) (%string->list x)]
    [(_ . r) (%string->list . r)]
    [_ %string->list]))

(define-syntax vector->list
  (syntax-rules ()
    [(_ x) (%vector->list x)]
    [(_ . r) (%vector->list . r)]
    [_ %vector->list]))

(define-syntax list*
  (syntax-rules ()
    [(_ x) x]
    [(_ x y) (cons x y)]
    [(_ x y z ...) (cons x (list* y z ...))]
    [(_ . args) (%list* . args)]
    [_ %list*]))

    ;; case-lambda, spelled without lambda* (Skint's multi-arity
    ;; core form, which the compiled backend does not have): the
    ;; expansion dispatches on the shape of the argument list.
    (define-syntax %cl-try
      (syntax-rules ()
        [(_ () a (body ...) fb)
         (if (null? a) (let () body ...) fb)]
        [(_ (p . rest) a (body ...) fb)
         (if (pair? a)
             (let ([p (car a)]) (%cl-try rest (cdr a) (body ...) fb))
             fb)]
        [(_ p a (body ...) fb)
         (let ([p a]) body ...)]))

    (define-syntax %cl-chain
      (syntax-rules ()
        [(_ a) (error "case-lambda: no matching clause" a)]
        [(_ a (fmls . body) . rest)
         (%cl-try fmls a body (%cl-chain a . rest))]))

    (define-syntax case-lambda
      (syntax-rules ()
        [(_ (fmls . body) ...)
         (lambda args (%cl-chain args (fmls . body) ...))]))

(define make-parameter
  (case-lambda
    [(value) 
     (case-lambda 
       [() value]
       [(x) (set! value x)]
       [(x s) (if s (set! value x) x)])]
    [(init converter)
     (let ([value (converter init)])
       (case-lambda 
         [() value]
         [(x) (set! value (converter x))]
         [(x s) (if s (set! value x) (converter x))]))]))

(define *rtd-count* 0)

(define (new-record-type name fields)
  ; an rtd may be any object but #f -- records are told apart by eq? on it -- so this
  ; could be (cons name fields); a symbol is kept because it prints readably, and the
  ; :count in it shows which of several record types of that name this one is
  ; NOTE: R7RS requires d-r-t records to be generative, so we have to "gensym" rtds
  (set! *rtd-count* (+ *rtd-count* 1))
  (string->symbol (string-append "rtd://" (symbol->string name) 
                                 ":" (number->string *rtd-count*))))

(define (floor/ x y)
  (values (floor-quotient x y) (floor-remainder x y)))

(define (truncate/ x y)
  (values (truncate-quotient x y) (truncate-remainder x y)))

(define (lcm . args)
  (if (null? args) 1
      (let loop ([x (abs (car args))] [args (cdr args)])
        (if (null? args) x
            (let* ([y (abs (car args))] [g (gcd x y)])
              (loop (if (zero? g) g (* (quotient x g) y)) (cdr args)))))))

(define rationalize ; Alan Bawden's algorithm
  (letrec
    ([rat1 (lambda (x y)
             (cond [(> x 0) (rat2 x y)]
                   [(< y 0) (- (rat2 (- y) (- x)))]
                   [else (if (and (exact? x) (exact? y)) 0 0.0)]))]
     [rat2 (lambda (x y)
             (let ([fx (floor x)] [fy (floor y)])
               (cond [(= fx x) fx]
                     [(= fx fy) (+ fx (/ (rat2 (/ (- y fy)) (/ (- x fx)))))]
                     [else (+ fx 1)])))])
    (lambda (x e)
      (unless (real? x) (error "rationalize: not a real number" x))
      (unless (real? e) (error "rationalize: not a real number" e))
      (let ([x (- x e)] [y (+ x e)])
        (cond [(< x y) (rat1 x y)] [(< y x) (rat1 y x)] [else x])))))

(define (square x) (* x x))

(define (exact-integer-sqrt x)
  (cond [(< x 2) (values x 0)]
        [else
         (let loop ([s x])
           (let ([t (quotient (+ s (quotient x s)) 2)])
             (if (>= t s)
                 (values s (- x (* s s)))
                 (loop t))))]))

(define (%append . args)
  (let loop ([args args])
    (cond [(null? args) '()]
          [(null? (cdr args)) (car args)]
          [else (list-cat (car args) (loop (cdr args)))])))

(define (%member v l . ?eq)
  (if (null? ?eq) 
      (meme v l)
      (let loop ([v v] [l l] [eq (car ?eq)])
        (and (pair? l) 
             (if (eq v (car l)) 
                 l 
                 (loop v (cdr l) eq))))))

(define (%assoc v al . ?eq)
  (if (null? ?eq) 
      (asse v al)
      (let loop ([v v] [al al] [eq (car ?eq)])
        (and (pair? al) 
             (if (eq v (caar al)) 
                 (car al) 
                 (loop v (cdr al) eq))))))

(define (%list* x . l)
  (let loop ([x x] [l l])
    (if (null? l) x (cons x (loop (car l) (cdr l))))))

(define (substring->list str start end)
  (let loop ([i (fx- end 1)] [l '()])
    (if (fx<? i start) l (loop (fx- i 1) (cons (string-ref str i) l)))))

(define %string->list
  (case-lambda
     [(str) (substring->list str 0 (string-length str))]
     [(str start) (substring->list str start (string-length str))]
     [(str start end) (substring->list str start end)]))

(define (substring-copy! to at from start end)
  (let ([limit (fxmin end (fx+ start (fx- (string-length to) at)))])
    (if (fx<=? at start)
        (do ([i at (fx+ i 1)] [j start (fx+ j 1)]) 
          [(fx>=? j limit)]
          (string-set! to i (string-ref from j)))
        (do ([i (fx+ at (fx- (fx- end start) 1)) (fx- i 1)] [j (fx- limit 1) (fx- j 1)])
          [(fx<? j start)]
          (string-set! to i (string-ref from j))))))

(define string-copy!
  (case-lambda
     [(to at from) (substring-copy! to at from 0 (string-length from))]
     [(to at from start) (substring-copy! to at from start (string-length from))]
     [(to at from start end) (substring-copy! to at from start end)]))

(define string-copy 
  (case-lambda
     [(str) (substring str 0 (string-length str))]  ; TODO: %scpy ?
     [(str start) (substring str start (string-length str))]
     [(str start end) (substring str start end)]))

(define (substring-fill! str c start end)
  (do ([i start (fx+ i 1)]) [(fx>=? i end)] (string-set! str i c)))

(define string-fill!
  (case-lambda
     [(str c) (substring-fill! str c 0 (string-length str))]
     [(str c start) (substring-fill! str c start (string-length str))]
     [(str c start end) (substring-fill! str c start end)]))

(define (substring-vector-copy! to at from start end)
  (let ([limit (fxmin end (fx+ start (fx- (vector-length to) at)))])
    (do ([i at (fx+ i 1)] [j start (fx+ j 1)]) 
      [(fx>=? j limit) to]
      (vector-set! to i (string-ref from j)))))

(define (substring->vector str start end)
  (substring-vector-copy! (make-vector (fx- end start)) 0 str start end))

(define (subvector->list vec start end)
  (let loop ([i (fx- end 1)] [l '()])
    (if (fx<? i start) l (loop (fx- i 1) (cons (vector-ref vec i) l)))))

(define %vector->list
  (case-lambda
     [(vec) (subvector->list vec 0 (vector-length vec))]
     [(vec start) (subvector->list vec start (vector-length vec))]
     [(vec start end) (subvector->list vec start end)]))

(define (subvector-copy! to at from start end)
  (let ([limit (fxmin end (fx+ start (fx- (vector-length to) at)))])
    (if (fx<=? at start)
        (do ([i at (fx+ i 1)] [j start (fx+ j 1)]) 
          [(fx>=? j limit)]
          (vector-set! to i (vector-ref from j)))
        (do ([i (fx+ at (fx- (fx- end start) 1)) (fx- i 1)] [j (fx- limit 1) (fx- j 1)])
          [(fx<? j start)]
          (vector-set! to i (vector-ref from j))))))

(define vector-copy!
  (case-lambda
     [(to at from) (subvector-copy! to at from 0 (vector-length from))]
     [(to at from start) (subvector-copy! to at from start (vector-length from))]
     [(to at from start end) (subvector-copy! to at from start end)]))

(define (subvector vec start end)  ; TODO: %vsub?
  (let ([v (make-vector (fx- end start))])
    (subvector-copy! v 0 vec start end)
    v))

(define vector-copy
  (case-lambda
     [(vec) (subvector vec 0 (vector-length vec))] ; TODO: %vcpy ?
     [(vec start) (subvector vec start (vector-length vec))]
     [(vec start end) (subvector vec start end)]))

(define (subvector-fill! vec x start end)
  (do ([i start (fx+ i 1)]) [(fx>=? i end)] (vector-set! vec i x)))

(define vector-fill!
  (case-lambda
     [(vec x) (subvector-fill! vec x 0 (vector-length vec))]
     [(vec x start) (subvector-fill! vec x start (vector-length vec))]
     [(vec x start end) (subvector-fill! vec x start end)]))

(define (subvector-string-copy! to at from start end)
  (let ([limit (fxmin end (fx+ start (fx- (string-length to) at)))])
    (do ([i at (fx+ i 1)] [j start (fx+ j 1)]) 
      [(fx>=? j limit) to]
      (string-set! to i (vector-ref from j)))))

(define (subvector->string vec start end)
  (subvector-string-copy! (make-string (fx- end start)) 0 vec start end))

(define vector->string
  (case-lambda
     [(vec) (subvector->string vec 0 (vector-length vec))]
     [(vec start) (subvector->string vec start (vector-length vec))]
     [(vec start end) (subvector->string vec start end)]))

(define (subbytevector->list bvec start end)
  (let loop ([i (fx- end 1)] [l '()])
    (if (fx<? i start) l (loop (fx- i 1) (cons (bytevector-u8-ref bvec i) l)))))

(define (subbytevector-copy! to at from start end)
  (let ([limit (fxmin end (fx+ start (fx- (bytevector-length to) at)))])
    (if (fx<=? at start)
        (do ([i at (fx+ i 1)] [j start (fx+ j 1)]) 
          [(fx>=? j limit)]
          (bytevector-u8-set! to i (bytevector-u8-ref from j)))
        (do ([i (fx+ at (fx- end start 1)) (fx- i 1)] [j (fx- limit 1) (fx- j 1)])
          [(fx<? j start)]
          (bytevector-u8-set! to i (bytevector-u8-ref from j))))))

(define bytevector-copy!
  (case-lambda
     [(to at from) (subbytevector-copy! to at from 0 (bytevector-length from))]
     [(to at from start) (subbytevector-copy! to at from start (bytevector-length from))]
     [(to at from start end) (subbytevector-copy! to at from start end)]))

(define (subbytevector-fill! bvec b start end)
  (do ([i start (fx+ i 1)]) [(fx>=? i end)] (bytevector-u8-set! bvec i b)))

(define bytevector-copy
  (case-lambda
     [(bvec) (subbytevector bvec 0 (bytevector-length bvec))]
     [(bvec start) (subbytevector bvec start (bytevector-length bvec))]
     [(bvec start end) (subbytevector bvec start end)]))

(define utf8->string
  (case-lambda
    [(bvec) (subutf8->string bvec 0 (bytevector-length bvec))]
    [(bvec start) (subutf8->string bvec start (bytevector-length bvec))]
    [(bvec start end) (subutf8->string bvec start end)]))

(define string->utf8
  (case-lambda
    [(str) (substring->utf8 str 0 (string-length str))]
    [(str start) (substring->utf8 str start (string-length str))]
    [(str start end) (substring->utf8 str start end)]))

(define string->vector
  (case-lambda
     [(str) (substring->vector str 0 (string-length str))]
     [(str start) (substring->vector str start (string-length str))]
     [(str start end) (substring->vector str start end)]))

(define (%apply p x . l)
  (apply-to-list p 
    (let loop ([x x] [l l])
      (if (null? l) x (cons x (loop (car l) (cdr l)))))))

(define (%call/cc p) (letcc k (p k)))

(define (%map1 p l)
  (let loop ([l l] [r '()])
    (if (pair? l) 
        (loop (cdr l) (cons (p (car l)) r)) 
        (reverse! r))))

(define (%map2 p l1 l2)
  (let loop ([l1 l1] [l2 l2] [r '()])
    (if (and (pair? l1) (pair? l2))
        (loop (cdr l1) (cdr l2) (cons (p (car l1) (car l2)) r))
        (reverse! r))))

(define (%map p l . l*)
  (cond [(null? l*) (%map1 p l)]
        [(null? (cdr l*)) (%map2 p l (car l*))] 
        [else
         (let loop ([l* (cons l l*)] [r '()])
           (if (let lp ([l* l*]) 
                 (or (null? l*) (and (pair? (car l*)) (lp (cdr l*)))))
               (loop (%map1 cdr l*) (cons (apply p (%map1 car l*)) r))
               (reverse! r)))]))

(define (%for-each1 p l)
  (let loop ([l l]) 
    (if (pair? l) 
        (begin (p (car l))
               (loop (cdr l))))))

(define (%for-each2 p l1 l2)
  (let loop ([l1 l1] [l2 l2]) 
    (if (and (pair? l1) (pair? l2)) 
        (begin (p (car l1) (car l2)) 
               (loop (cdr l1) (cdr l2))))))

(define (%for-each p l . l*)
  (cond [(null? l*) (%for-each1 p l)]
        [(null? (cdr l*)) (%for-each2 p l (car l*))]
        [else
         (let loop ([l* (cons l l*)])
           (if (let lp ([l* l*]) 
                 (or (null? l*) (and (pair? (car l*)) (lp (cdr l*)))))
               (begin (apply p (map car l*)) (loop (map cdr l*)))))]))

(define (string-map p s . s*)
  (if (null? s*)
      (let* ([len (string-length s)] [res (make-string len)])
        (do ([i 0 (fx+ i 1)]) [(fx>=? i len) res]
           (string-set! res i (p (string-ref s i)))))
      (list->string (apply map p (map string->list (cons s s*))))))

(define (vector-map p v . v*)
  (if (null? v*)
      (let* ([len (vector-length v)] [res (make-vector len)])
        (do ([i 0 (fx+ i 1)]) [(fx>=? i len) res]
          (vector-set! res i (p (vector-ref v i)))))
      (list->vector (apply map p (map vector->list (cons v v*))))))

(define (string-for-each p s . s*)
  (if (null? s*) 
      (let ([len (string-length s)])
        (do ([i 0 (fx+ i 1)]) [(fx>=? i len)] (p (string-ref s i))))
      (apply for-each p (map string->list (cons s s*)))))

(define (vector-for-each p v . v*)
  (if (null? v*)
      (let ([len (vector-length v)])
        (do ([i 0 (fx+ i 1)]) [(fx>=? i len)] (p (vector-ref v i))))
      (apply for-each p (map vector->list (cons v v*)))))

(define (port? x) (fixnum? (%port? x)))

(define (textual-port? x) (eqv? (%port? x 4) 0))

(define (binary-port? x) (eqv? (%port? x 4) 4))

(define (close-port p)
  (if (input-port? p) (close-input-port p))
  (if (output-port? p) (close-output-port p)))

(define (call-with-port port proc)
  (call-with-values (lambda () (proc port))
    (lambda vals (close-port port) (apply values vals))))

(define (read-substring! str start end p)
  (let loop ([i start])
    (if (fx>=? i end) (fx- i start)
        (let ([c (read-char p)])
          (cond [(eof-object? c) (if (fx=? i start) c (fx- i start))]
                [else (string-set! str i c) (loop (fx+ i 1))])))))

(define (read-substring k p)
  (let ([str (make-string k)])
    (let ([r (read-substring! str 0 k p)])
      (if (eof-object? r) r
          (if (fx=? r k) str (substring str 0 r))))))

(define read-string!
  (case-lambda
    [(str) (read-substring! str 0 (string-length str) (current-input-port))]
    [(str p) (read-substring! str 0 (string-length str) p)]
    [(str p start) (read-substring! str start (string-length str) p)]
    [(str p start end) (read-substring! str start end p)]))

(define read-string
  (case-lambda
    [(k) (read-substring k (current-input-port))]
    [(k p) (read-substring k p)]))

(define (read-subbytevector! bvec start end p)
  (let loop ([i start])
    (if (fx>=? i end) (fx- i start)
        (let ([u8 (read-u8 p)])
          (cond [(eof-object? u8) (if (fx=? i start) u8 (fx- i start))]
                [else (bytevector-u8-set! bvec i u8) (loop (fx+ i 1))])))))

(define (read-subbytevector k p)
  (let ([bvec (make-bytevector k)])
    (let ([r (read-subbytevector! bvec 0 k p)])
      (if (eof-object? r) r
          (if (fx=? r k) bvec (subbytevector bvec 0 r))))))

(define read-bytevector!
  (case-lambda
    [(bvec) (read-subbytevector! bvec 0 (bytevector-length bvec) (current-input-port))]
    [(bvec p) (read-subbytevector! bvec 0 (bytevector-length bvec) p)]
    [(bvec p start) (read-subbytevector! bvec start (bytevector-length bvec) p)]
    [(bvec p start end) (read-subbytevector! bvec start end p)]))

(define read-bytevector
  (case-lambda
    [(k) (read-subbytevector k (current-input-port))]
    [(k p) (read-subbytevector k p)]))

(define (write-substring from start end p)
  (do ([i start (fx+ i 1)]) [(fx>=? i end)] (write-char (string-ref from i) p)))

(define write-string
  (case-lambda
    [(str) (write-substring str 0 (string-length str) (current-output-port))]
    [(str p) (write-substring str 0 (string-length str) p)]
    [(str p start) (write-substring str start (string-length str) p)]
    [(str p start end) (write-substring str start end p)]))

(define (write-subbytevector from start end p)
  (do ([i start (fx+ i 1)]) [(fx>=? i end)] (write-u8 (bytevector-u8-ref from i) p)))

(define write-bytevector
  (case-lambda
    [(bvec) (write-subbytevector bvec 0 (bytevector-length bvec) (current-output-port))]
    [(bvec p) (write-subbytevector bvec 0 (bytevector-length bvec) p)]
    [(bvec p start) (write-subbytevector bvec start (bytevector-length bvec) p)]
    [(bvec p start end) (write-subbytevector bvec start end p)]))

(define (read-error? obj)
  (and (error-object? obj) (eq? (error-object-kind obj) 'read)))

(define (file-error? obj)
  (and (error-object? obj) (eq? (error-object-kind obj) 'file)))

    ;; fxmin is used by the copied substring/subvector copy loops;
    ;; it is not imported as a builtin, so provide the obvious local
    ;; fixnum definition in terms of fx<?.
    (define (fxmin a b) (if (fx<? a b) a b))

    ;; reverse! is destructive list reversal; the compiled runtime
    ;; has no primitive for it, so it is spelled out here.
    (define (reverse! l)
      (let loop ([l l] [r '()])
        (if (null? l) r
            (let ([t (cdr l)])
              (set-cdr! l r)
              (loop t l)))))

    ;; -- records: a record is a PACKED block of at least three
    ;; cells (see the make-record case in the compiler), its rtd in
    ;; slot 0 and field i in slot i+1.  Pairs are PACKED blocks of
    ;; exactly two cells and a box has one cell, so records are
    ;; disjoint from pairs, boxes and vectors: vector?, pair? and
    ;; box? of a record are all #f -- an observable distinction
    ;; (SRFI-115's regexp-match-convert dispatches on vector?).
    ;; define-record-type (from s.scm, above) tells records apart
    ;; by eq? on the rtd.  make-record is backed by the compiler's
    ;; allocator and record? by the $scmi:recp prim; the accessors
    ;; are ordinary block accesses, which ignore the microtag.
    ;; record-length counts block cells minus the rtd slot, so it
    ;; reads 2 for the padded zero/one-field records.
    (define-syntax %cover-make-record
      (syntax-rules ()
        [(_ . args) (%builtin-make-record . args)]
        [_ %residual-make-record]))
    (define %residual-make-record
      (lambda (a0 a1) (%builtin-make-record a0 a1)))
    (define-syntax make-record %cover-make-record)
    (define-syntax %cover-record?
      (syntax-rules ()
        [(_ o) (%builtin-record? o)]
        [(_ o r) (and (%builtin-record? o) (eq? (vector-ref o 0) r))]
        [_ %residual-record?]))
    (define %residual-record?
      (lambda (obj . rtd)
        (and (%builtin-record? obj)
             (or (null? rtd) (eq? (vector-ref obj 0) (car rtd))))))
    (define-syntax record? %cover-record?)
    (define (record-length r) (- (vector-length r) 1))
    (define (record-ref r i) (vector-ref r (+ i 1)))
    (define (record-set! r i val) (vector-set! r (+ i 1) val))

    ;; -- dynamic-wind: the compiled runtime has no winder
    ;; machinery, so it is implemented here in Scheme over %call/cc
    ;; (the textbook winder-list implementation).  call/cc therefore
    ;; cannot be the raw letcc cover: every escape procedure it hands
    ;; out is wrapped so that invoking it first runs the out/in
    ;; thunks between the current winder state and the captured one.
    ;; parameterize (from s.scm, above) is spelled with dynamic-wind,
    ;; so the exception machinery below depends on this being right.
    (define *winders* '())

    (define (%common-tail x y)
      (let ([lx (length x)] [ly (length y)])
        (let loop ([x (if (> lx ly) (list-tail x (- lx ly)) x)]
                   [y (if (> ly lx) (list-tail y (- ly lx)) y)])
          (if (eq? x y) x (loop (cdr x) (cdr y))))))

    (define (%do-wind new)
      (let ([tail (%common-tail new *winders*)])
        (let loop ([l *winders*])
          (unless (eq? l tail)
            (set! *winders* (cdr l))
            ((cdr (car l)))
            (loop (cdr l))))
        (let loop ([l new])
          (unless (eq? l tail)
            (loop (cdr l))
            ((car (car l)))
            (set! *winders* (cons (car l) *winders*))))))

    (define (%call/cc+wind p)
      (%call/cc
        (lambda (k)
          (let ([saved *winders*])
            (p (lambda vals
                 (%do-wind saved)
                 (k (if (and (pair? vals) (null? (cdr vals)))
                        (car vals)
                        (apply values vals)))))))))

    ;; call/cc in call position inlines the letcc at the call site:
    ;; the continuation handed to p is a wrapped escape, but the
    ;; escape itself is applied through the letcc binding, so the
    ;; expander still rewrites its applications to withcc -- the only
    ;; continuation-application form the CPS backend supports.  The
    ;; procedure %call/cc+wind serves value positions.
    (define-syntax call/cc
      (syntax-rules ()
        [(_ p) (let ([saved *winders*])
                 (letcc k
                   (p (lambda vals
                        (%do-wind saved)
                        (k (if (and (pair? vals) (null? (cdr vals)))
                               (car vals)
                               (apply values vals)))))))]
        [(_ . args) (%call/cc+wind . args)]
        [_ %call/cc+wind]))

    (define (dynamic-wind in body out)
      (in)
      (set! *winders* (cons (cons in out) *winders*))
      (call-with-values body
        (lambda results
          (set! *winders* (cdr *winders*))
          (out)
          (apply values results))))

    ;; -- exceptions: <error-object> machinery from s.scm, minus the
    ;; debugger/failure-object integration (that part reads Skint's
    ;; closure representation and belongs to the interpreter; the
    ;; default handler bottoms out in the builtin error).
    (define-record-type <error-object>
      (error-object kind message irritants)
      error-object?
      (kind error-object-kind)
      (message error-object-message)
      (irritants error-object-irritants))

    (define (error msg . args)
      (raise (error-object #f msg args)))

    ;; the default handler is the end of the line: report the
    ;; message on the error port and leave through the %exit
    ;; prim -- it must not re-enter raise (that would loop), and
    ;; no backend has to supply a host `error` procedure for it.
    ;; NB: handlers in this machinery are case-lambdas whose
    ;; 0-argument call returns the previous handler (see raise),
    ;; so the default handler must be one too, its own parent.
    (define current-exception-handler
      (make-parameter
        (letrec ([default-handler
                  (case-lambda
                    [() default-handler]
                    [(obj)
                     (when (and (error-object? obj)
                                (string? (error-object-message obj)))
                       (let ([p (current-error-port)])
                         (write-string "Error: " p)
                         (write-string (error-object-message obj) p)
                         (newline p)))
                     (%exit #f)])])
          default-handler)))

    (define (with-exception-handler handler thunk)
      (let ([eh (current-exception-handler)])
        (parameterize ([current-exception-handler
                        (case-lambda [() eh] [(obj) (handler obj)])])
          (thunk))))

    (define (raise obj)
      (let ([eh (current-exception-handler)])
        (parameterize ([current-exception-handler (eh)])
          (eh obj)
          (raise (error-object 'raise
                   "exception handler returned" (list eh obj))))))

    (define (raise-continuable obj)
      (let ([eh (current-exception-handler)])
        (parameterize ([current-exception-handler (eh)])
          (eh obj))))

    ;; gcd and the string orderings: no compiled-runtime
    ;; primitive exists for them, and none is needed -- they are
    ;; ordinary Scheme over modulo/abs and the char orderings.
    (define (gcd . args)
      (let loop ([a 0] [args args])
        (if (null? args) a
            (loop (let ([b (abs (car args))])
                    (let loop2 ([x a] [y b])
                      (if (zero? y) x (loop2 y (modulo x y)))))
                  (cdr args)))))

    (define (%string-cmp a b)
      ;; -1, 0 or 1 by lexicographic char order; a proper prefix
      ;; of the other string is the smaller one
      (let ([la (string-length a)] [lb (string-length b)])
        (let loop ([i 0])
          (cond [(= i la) (if (= i lb) 0 -1)]
                [(= i lb) 1]
                [(char<? (string-ref a i) (string-ref b i)) -1]
                [(char>? (string-ref a i) (string-ref b i)) 1]
                [else (loop (+ i 1))]))))

    (define (%string-order pred)
      (lambda (a b . rest)
        (let loop ([a a] [b b] [rest rest])
          (and (pred (%string-cmp a b))
               (or (null? rest) (loop b (car rest) (cdr rest)))))))

    (define string<? (%string-order (lambda (c) (< c 0))))
    (define string>? (%string-order (lambda (c) (> c 0))))
    (define string<=? (%string-order (lambda (c) (<= c 0))))
    (define string>=? (%string-order (lambda (c) (>= c 0))))

    ;; the bytevector and list->vector constructors: ordinary
    ;; Scheme over make-bytevector/make-vector and the setters
    (define (bytevector . args)
      (let ([bv (make-bytevector (length args))])
        (let loop ([i 0] [args args])
          (cond [(null? args) bv]
                [else (bytevector-u8-set! bv i (car args))
                      (loop (+ i 1) (cdr args))]))))

    (define (list->vector l)
      (let ([v (make-vector (length l))])
        (let loop ([i 0] [l l])
          (cond [(null? l) v]
                [else (vector-set! v i (car l))
                      (loop (+ i 1) (cdr l))]))))

    ;; a compiled program's feature list: R7RS on a little-endian
    ;; unix host, with no ratios/complex in the numeric tower
    (define (features) '(r7rs linux little-endian))

    (define (bytevector-append . bvs)
      (let ([bv (make-bytevector
                 (let loop ([n 0] [bvs bvs])
                   (if (null? bvs) n
                       (loop (+ n (bytevector-length (car bvs)))
                             (cdr bvs)))))])
        (let loop ([pos 0] [bvs bvs])
          (cond [(null? bvs) bv]
                [else (bytevector-copy! bv pos (car bvs))
                      (loop (+ pos (bytevector-length (car bvs)))
                            (cdr bvs))]))))

    (define (vector-append . vs)
      (let ([v (make-vector
                (let loop ([n 0] [vs vs])
                  (if (null? vs) n
                      (loop (+ n (vector-length (car vs))) (cdr vs)))))])
        (let loop ([pos 0] [vs vs])
          (cond [(null? vs) v]
                [else
                 (let ([src (car vs)] [n (vector-length (car vs))])
                   (let loop2 ([i 0])
                     (cond [(= i n) (loop (+ pos n) (cdr vs))]
                           [else (vector-set! v (+ pos i)
                                              (vector-ref src i))
                                 (loop2 (+ i 1))])))]))))

    (define (list-copy l)
      (if (pair? l) (cons (car l) (list-copy (cdr l))) l))

    (define (list-set! l k obj)
      (set-car! (list-tail l k) obj))

    (define (even? n) (zero? (modulo n 2)))
    (define (odd? n) (not (zero? (modulo n 2))))

    ;; this runtime has no non-real numbers: every number is
    ;; complex and real; a number is rational iff it is finite
    (define (complex? x) (number? x))
    (define (real? x) (number? x))
    (define (rational? x)
      (and (number? x)
           (or (exact? x)
               (and (= x x) (< (abs x) (/ 1.0 0.0))))))

    (define (symbol=? a b . rest)
      (let loop ([a a] [b b] [rest rest])
        (and (eq? a b)
             (or (null? rest) (loop b (car rest) (cdr rest))))))

    ;; exact binary decomposition of a finite flonum: doubling
    ;; reaches an integer-valued flonum in at most ~1100 steps
    (define (%num-den q)
      (let loop ([x q] [d 1.0])
        (if (= x (truncate x)) (cons x d) (loop (* x 2.0) (* d 2.0)))))
    (define (numerator q)
      (if (exact? q) q (car (%num-den q))))
    (define (denominator q)
      (if (exact? q) 1 (cdr (%num-den q))))

    (define (read-line . args)
      (let ([p (if (pair? args) (car args) (current-input-port))])
        (let loop ([cs '()])
          (let ([c (read-char p)])
            (cond [(eof-object? c)
                   (if (null? cs) c (list->string (reverse cs)))]
                  [(char=? c #\newline) (list->string (reverse cs))]
                  [else (loop (cons c cs))])))))

    ;; byte io over the character ports: on bytevector ports and
    ;; (for ASCII) on text ports a byte is a char below 256
    (define (read-u8 . args)
      (let ([c (read-char (if (pair? args) (car args)
                              (current-input-port)))])
        (if (eof-object? c) c (char->integer c))))
    (define (peek-u8 . args)
      (let ([c (peek-char (if (pair? args) (car args)
                              (current-input-port)))])
        (if (eof-object? c) c (char->integer c))))
    (define (write-u8 b . args)
      (write-char (integer->char b)
                  (if (pair? args) (car args) (current-output-port))))
    (define (char-ready? . args)
      (not (eof-object?
            (peek-char (if (pair? args) (car args)
                           (current-input-port))))))
    (define (u8-ready? . args)
      (not (eof-object?
            (peek-u8 (if (pair? args) (car args)
                         (current-input-port))))))

    ;; subbytevector is a Skint builtin the compiled runtime has
    ;; no primitive for; the bytevector-copy case-lambda and the
    ;; bytevector readers below use it, so it is ordinary Scheme
    ;; here (not exported: it is not an R7RS name)
    (define (subbytevector bvec start end)
      (let ([bv (make-bytevector (- end start))])
        (subbytevector-copy! bv 0 bvec start end)
        bv))

    ;; UTF-8 <-> string conversion (utf8->string / string->utf8
    ;; below are case-lambdas over these); pure Scheme, since the
    ;; compiled runtime has no subutf8->string prim and needs none
    (define (subutf8->string bvec start end)
      (let loop ([i start] [cs '()])
        (if (>= i end)
            (list->string (reverse cs))
            (let ([b (bytevector-u8-ref bvec i)])
              (cond
                [(< b 128)
                 (loop (+ i 1) (cons (integer->char b) cs))]
                [(< b 224)
                 (loop (+ i 2)
                       (cons (integer->char
                              (+ (* (- b 192) 64)
                                 (- (bytevector-u8-ref bvec (+ i 1)) 128)))
                             cs))]
                [(< b 240)
                 (loop (+ i 3)
                       (cons (integer->char
                              (+ (* (- b 224) 4096)
                                 (* (- (bytevector-u8-ref bvec (+ i 1)) 128) 64)
                                 (- (bytevector-u8-ref bvec (+ i 2)) 128)))
                             cs))]
                [else
                 (loop (+ i 4)
                       (cons (integer->char
                              (+ (* (- b 240) 262144)
                                 (* (- (bytevector-u8-ref bvec (+ i 1)) 128) 4096)
                                 (* (- (bytevector-u8-ref bvec (+ i 2)) 128) 64)
                                 (- (bytevector-u8-ref bvec (+ i 3)) 128)))
                             cs))])))))

    (define (%utf8-clen c)
      (cond [(< c 128) 1] [(< c 2048) 2] [(< c 65536) 3] [else 4]))

    (define (substring->utf8 str start end)
      (let ([total (let loop ([i start] [n 0])
                     (if (>= i end) n
                         (loop (+ i 1)
                               (+ n (%utf8-clen
                                     (char->integer (string-ref str i)))))))])
        (let ([bv (make-bytevector total)])
          (let loop ([i start] [j 0])
            (if (>= i end) bv
                (let ([c (char->integer (string-ref str i))])
                  (cond
                    [(< c 128)
                     (bytevector-u8-set! bv j c)
                     (loop (+ i 1) (+ j 1))]
                    [(< c 2048)
                     (bytevector-u8-set! bv j (+ 192 (quotient c 64)))
                     (bytevector-u8-set! bv (+ j 1) (+ 128 (modulo c 64)))
                     (loop (+ i 1) (+ j 2))]
                    [(< c 65536)
                     (bytevector-u8-set! bv j (+ 224 (quotient c 4096)))
                     (bytevector-u8-set! bv (+ j 1)
                                         (+ 128 (modulo (quotient c 64) 64)))
                     (bytevector-u8-set! bv (+ j 2) (+ 128 (modulo c 64)))
                     (loop (+ i 1) (+ j 3))]
                    [else
                     (bytevector-u8-set! bv j (+ 240 (quotient c 262144)))
                     (bytevector-u8-set! bv (+ j 1)
                                         (+ 128 (modulo (quotient c 4096) 64)))
                     (bytevector-u8-set! bv (+ j 2)
                                         (+ 128 (modulo (quotient c 64) 64)))
                     (bytevector-u8-set! bv (+ j 3) (+ 128 (modulo c 64)))
                     (loop (+ i 1) (+ j 4))])))))))

    (define-syntax %cover-zero?
      (syntax-rules ()
        [(_ . args) (%builtin-zero? . args)]
        [_ %residual-zero?]))
    (define %residual-zero? (lambda (a0) (%builtin-zero? a0)))
    (define-syntax zero? %cover-zero?)
    (define-syntax %cover-write-char
      (syntax-rules ()
        [(_ . args) (%builtin-write-char . args)]
        [_ %residual-write-char]))
    (define %residual-write-char
      (lambda (a0 . args)
        (if (pair? args) (%builtin-write-char a0 (car args)) (%builtin-write-char a0))))
    (define-syntax write-char %cover-write-char)
    (define-syntax %cover-vector?
      (syntax-rules ()
        [(_ . args) (%builtin-vector? . args)]
        [_ %residual-vector?]))
    (define %residual-vector? (lambda (a0) (%builtin-vector? a0)))
    (define-syntax vector? %cover-vector?)
    (define-syntax %cover-vector-set!
      (syntax-rules ()
        [(_ . args) (%builtin-vector-set! . args)]
        [_ %residual-vector-set!]))
    (define %residual-vector-set! (lambda (a0 a1 a2) (%builtin-vector-set! a0 a1 a2)))
    (define-syntax vector-set! %cover-vector-set!)
    (define-syntax %cover-vector-ref
      (syntax-rules ()
        [(_ . args) (%builtin-vector-ref . args)]
        [_ %residual-vector-ref]))
    (define %residual-vector-ref (lambda (a0 a1) (%builtin-vector-ref a0 a1)))
    (define-syntax vector-ref %cover-vector-ref)
    (define-syntax %cover-vector-length
      (syntax-rules ()
        [(_ . args) (%builtin-vector-length . args)]
        [_ %residual-vector-length]))
    (define %residual-vector-length (lambda (a0) (%builtin-vector-length a0)))
    (define-syntax vector-length %cover-vector-length)
    (define-syntax %cover-truncate-remainder
      (syntax-rules ()
        [(_ . args) (%builtin-truncate-remainder . args)]
        [_ %residual-truncate-remainder]))
    (define %residual-truncate-remainder (lambda (a0 a1) (%builtin-truncate-remainder a0 a1)))
    (define-syntax truncate-remainder %cover-truncate-remainder)
    (define-syntax %cover-truncate-quotient
      (syntax-rules ()
        [(_ . args) (%builtin-truncate-quotient . args)]
        [_ %residual-truncate-quotient]))
    (define %residual-truncate-quotient (lambda (a0 a1) (%builtin-truncate-quotient a0 a1)))
    (define-syntax truncate-quotient %cover-truncate-quotient)
    (define-syntax %cover-truncate
      (syntax-rules ()
        [(_ . args) (%builtin-truncate . args)]
        [_ %residual-truncate]))
    (define %residual-truncate (lambda (a0) (%builtin-truncate a0)))
    (define-syntax truncate %cover-truncate)
    (define-syntax %cover-symbol?
      (syntax-rules ()
        [(_ . args) (%builtin-symbol? . args)]
        [_ %residual-symbol?]))
    (define %residual-symbol? (lambda (a0) (%builtin-symbol? a0)))
    (define-syntax symbol? %cover-symbol?)
    (define-syntax %cover-symbol->string
      (syntax-rules ()
        [(_ . args) (%builtin-symbol->string . args)]
        [_ %residual-symbol->string]))
    (define %residual-symbol->string (lambda (a0) (%builtin-symbol->string a0)))
    (define-syntax symbol->string %cover-symbol->string)
    (define-syntax %cover-substring
      (syntax-rules ()
        [(_ . args) (%builtin-substring . args)]
        [_ %residual-substring]))
    (define %residual-substring (lambda (a0 a1 a2) (%builtin-substring a0 a1 a2)))
    (define-syntax substring %cover-substring)
    (define-syntax %cover-string?
      (syntax-rules ()
        [(_ . args) (%builtin-string? . args)]
        [_ %residual-string?]))
    (define %residual-string? (lambda (a0) (%builtin-string? a0)))
    (define-syntax string? %cover-string?)
    (define-syntax %cover-string=?
      (syntax-rules ()
        [(_ . args) (%builtin-string=? . args)]
        [_ %residual-string=?]))
    (define %residual-string=?
      (lambda (a0 a1 . args) (%builtin-apply %builtin-string=? (cons a0 (cons a1 args)))))
    (define-syntax string=? %cover-string=?)
    (define-syntax %cover-string-set!
      (syntax-rules ()
        [(_ . args) (%builtin-string-set! . args)]
        [_ %residual-string-set!]))
    (define %residual-string-set! (lambda (a0 a1 a2) (%builtin-string-set! a0 a1 a2)))
    (define-syntax string-set! %cover-string-set!)
    (define-syntax %cover-string-ref
      (syntax-rules ()
        [(_ . args) (%builtin-string-ref . args)]
        [_ %residual-string-ref]))
    (define %residual-string-ref (lambda (a0 a1) (%builtin-string-ref a0 a1)))
    (define-syntax string-ref %cover-string-ref)
    (define-syntax %cover-string-length
      (syntax-rules ()
        [(_ . args) (%builtin-string-length . args)]
        [_ %residual-string-length]))
    (define %residual-string-length (lambda (a0) (%builtin-string-length a0)))
    (define-syntax string-length %cover-string-length)
    (define-syntax %cover-string->symbol
      (syntax-rules ()
        [(_ . args) (%builtin-string->symbol . args)]
        [_ %residual-string->symbol]))
    (define %residual-string->symbol (lambda (a0) (%builtin-string->symbol a0)))
    (define-syntax string->symbol %cover-string->symbol)
    (define-syntax %cover-string->number
      (syntax-rules ()
        [(_ . args) (%builtin-string->number . args)]
        [_ %residual-string->number]))
    (define %residual-string->number
      (lambda (a0 . args)
        (if (pair? args) (%builtin-string->number a0 (car args)) (%builtin-string->number a0))))
    (define-syntax string->number %cover-string->number)
    (define-syntax %cover-set-cdr!
      (syntax-rules ()
        [(_ . args) (%builtin-set-cdr! . args)]
        [_ %residual-set-cdr!]))
    (define %residual-set-cdr! (lambda (a0 a1) (%builtin-set-cdr! a0 a1)))
    (define-syntax set-cdr! %cover-set-cdr!)
    (define-syntax %cover-set-car!
      (syntax-rules ()
        [(_ . args) (%builtin-set-car! . args)]
        [_ %residual-set-car!]))
    (define %residual-set-car! (lambda (a0 a1) (%builtin-set-car! a0 a1)))
    (define-syntax set-car! %cover-set-car!)
    (define-syntax %cover-round
      (syntax-rules ()
        [(_ . args) (%builtin-round . args)]
        [_ %residual-round]))
    (define %residual-round (lambda (a0) (%builtin-round a0)))
    (define-syntax round %cover-round)
    (define-syntax %cover-reverse
      (syntax-rules ()
        [(_ . args) (%builtin-reverse . args)]
        [_ %residual-reverse]))
    (define %residual-reverse (lambda (a0) (%builtin-reverse a0)))
    (define-syntax reverse %cover-reverse)
    (define-syntax %cover-remainder
      (syntax-rules ()
        [(_ . args) (%builtin-remainder . args)]
        [_ %residual-remainder]))
    (define %residual-remainder (lambda (a0 a1) (%builtin-remainder a0 a1)))
    (define-syntax remainder %cover-remainder)
    (define-syntax %cover-read-char
      (syntax-rules ()
        [(_ . args) (%builtin-read-char . args)]
        [_ %residual-read-char]))
    (define %residual-read-char
      (lambda args
        (if (pair? args) (%builtin-read-char (car args)) (%builtin-read-char))))
    (define-syntax read-char %cover-read-char)
    (define-syntax %cover-quotient
      (syntax-rules ()
        [(_ . args) (%builtin-quotient . args)]
        [_ %residual-quotient]))
    (define %residual-quotient (lambda (a0 a1) (%builtin-quotient a0 a1)))
    (define-syntax quotient %cover-quotient)
    (define-syntax %cover-procedure?
      (syntax-rules ()
        [(_ . args) (%builtin-procedure? . args)]
        [_ %residual-procedure?]))
    (define %residual-procedure? (lambda (a0) (%builtin-procedure? a0)))
    (define-syntax procedure? %cover-procedure?)
    (define-syntax %cover-positive?
      (syntax-rules ()
        [(_ . args) (%builtin-positive? . args)]
        [_ %residual-positive?]))
    (define %residual-positive? (lambda (a0) (%builtin-positive? a0)))
    (define-syntax positive? %cover-positive?)
    (define-syntax %cover-peek-char
      (syntax-rules ()
        [(_ . args) (%builtin-peek-char . args)]
        [_ %residual-peek-char]))
    (define %residual-peek-char
      (lambda args
        (if (pair? args) (%builtin-peek-char (car args)) (%builtin-peek-char))))
    (define-syntax peek-char %cover-peek-char)
    (define-syntax %cover-pair?
      (syntax-rules ()
        [(_ . args) (%builtin-pair? . args)]
        [_ %residual-pair?]))
    (define %residual-pair? (lambda (a0) (%builtin-pair? a0)))
    (define-syntax pair? %cover-pair?)
    (define-syntax %cover-output-port?
      (syntax-rules ()
        [(_ . args) (%builtin-output-port? . args)]
        [_ %residual-output-port?]))
    (define %residual-output-port? (lambda (a0) (%builtin-output-port? a0)))
    (define-syntax output-port? %cover-output-port?)
    (define-syntax %cover-output-port-open?
      (syntax-rules ()
        [(_ . args) (%builtin-output-port-open? . args)]
        [_ %residual-output-port-open?]))
    (define %residual-output-port-open? (lambda (a0) (%builtin-output-port-open? a0)))
    (define-syntax output-port-open? %cover-output-port-open?)
    (define-syntax %cover-open-output-string
      (syntax-rules ()
        [(_ . args) (%builtin-open-output-string . args)]
        [_ %residual-open-output-string]))
    (define %residual-open-output-string (lambda () (%builtin-open-output-string)))
    (define-syntax open-output-string %cover-open-output-string)
    (define-syntax %cover-open-output-bytevector
      (syntax-rules ()
        [(_ . args) (%builtin-open-output-bytevector . args)]
        [_ %residual-open-output-bytevector]))
    (define %residual-open-output-bytevector (lambda () (%builtin-open-output-bytevector)))
    (define-syntax open-output-bytevector %cover-open-output-bytevector)
    (define-syntax %cover-open-input-string
      (syntax-rules ()
        [(_ . args) (%builtin-open-input-string . args)]
        [_ %residual-open-input-string]))
    (define %residual-open-input-string (lambda (a0) (%builtin-open-input-string a0)))
    (define-syntax open-input-string %cover-open-input-string)
    (define-syntax %cover-open-input-bytevector
      (syntax-rules ()
        [(_ . args) (%builtin-open-input-bytevector . args)]
        [_ %residual-open-input-bytevector]))
    (define %residual-open-input-bytevector (lambda (a0) (%builtin-open-input-bytevector a0)))
    (define-syntax open-input-bytevector %cover-open-input-bytevector)
    (define-syntax %cover-number?
      (syntax-rules ()
        [(_ . args) (%builtin-number? . args)]
        [_ %residual-number?]))
    (define %residual-number? (lambda (a0) (%builtin-number? a0)))
    (define-syntax number? %cover-number?)
    (define-syntax %cover-number->string
      (syntax-rules ()
        [(_ . args) (%builtin-number->string . args)]
        [_ %residual-number->string]))
    (define %residual-number->string
      (lambda (a0 . args)
        (if (pair? args) (%builtin-number->string a0 (car args)) (%builtin-number->string a0))))
    (define-syntax number->string %cover-number->string)
    (define-syntax %cover-null?
      (syntax-rules ()
        [(_ . args) (%builtin-null? . args)]
        [_ %residual-null?]))
    (define %residual-null? (lambda (a0) (%builtin-null? a0)))
    (define-syntax null? %cover-null?)
    ;; no %cover-not: `not` is the root binding re-exported above
    ;; (see the import note); in call position it is the DS prim,
    ;; in value position the compiler's prim wrapper.
    (define-syntax %cover-newline
      (syntax-rules ()
        [(_ . args) (%builtin-newline . args)]
        [_ %residual-newline]))
    (define %residual-newline
      (lambda args
        (if (pair? args) (%builtin-newline (car args)) (%builtin-newline))))
    (define-syntax newline %cover-newline)
    (define-syntax %cover-negative?
      (syntax-rules ()
        [(_ . args) (%builtin-negative? . args)]
        [_ %residual-negative?]))
    (define %residual-negative? (lambda (a0) (%builtin-negative? a0)))
    (define-syntax negative? %cover-negative?)
    (define-syntax %cover-modulo
      (syntax-rules ()
        [(_ . args) (%builtin-modulo . args)]
        [_ %residual-modulo]))
    (define %residual-modulo (lambda (a0 a1) (%builtin-modulo a0 a1)))
    (define-syntax modulo %cover-modulo)
    (define-syntax %cover-min
      (syntax-rules ()
        [(_ . args) (%builtin-min . args)]
        [_ %residual-min]))
    (define %residual-min
      (lambda (a0 . args) (%builtin-apply %builtin-min (cons a0 args))))
    (define-syntax min %cover-min)
    (define-syntax %cover-memv
      (syntax-rules ()
        [(_ . args) (%builtin-memv . args)]
        [_ %residual-memv]))
    (define %residual-memv (lambda (a0 a1) (%builtin-memv a0 a1)))
    (define-syntax memv %cover-memv)
    (define-syntax %cover-memq
      (syntax-rules ()
        [(_ . args) (%builtin-memq . args)]
        [_ %residual-memq]))
    (define %residual-memq (lambda (a0 a1) (%builtin-memq a0 a1)))
    (define-syntax memq %cover-memq)
    (define-syntax %cover-max
      (syntax-rules ()
        [(_ . args) (%builtin-max . args)]
        [_ %residual-max]))
    (define %residual-max
      (lambda (a0 . args) (%builtin-apply %builtin-max (cons a0 args))))
    (define-syntax max %cover-max)
    (define-syntax %cover-make-vector
      (syntax-rules ()
        [(_ . args) (%builtin-make-vector . args)]
        [_ %residual-make-vector]))
    (define %residual-make-vector
      (lambda (a0 . args)
        (if (pair? args) (%builtin-make-vector a0 (car args)) (%builtin-make-vector a0))))
    (define-syntax make-vector %cover-make-vector)
    (define-syntax %cover-make-string
      (syntax-rules ()
        [(_ . args) (%builtin-make-string . args)]
        [_ %residual-make-string]))
    (define %residual-make-string
      (lambda (a0 . args)
        (if (pair? args) (%builtin-make-string a0 (car args)) (%builtin-make-string a0))))
    (define-syntax make-string %cover-make-string)
    (define-syntax %cover-make-list
      (syntax-rules ()
        [(_ . args) (%builtin-make-list . args)]
        [_ %residual-make-list]))
    (define %residual-make-list
      (lambda (a0 . args)
        (if (pair? args) (%builtin-make-list a0 (car args)) (%builtin-make-list a0))))
    (define-syntax make-list %cover-make-list)
    (define-syntax %cover-make-bytevector
      (syntax-rules ()
        [(_ . args) (%builtin-make-bytevector . args)]
        [_ %residual-make-bytevector]))
    (define %residual-make-bytevector
      (lambda (a0 . args)
        (if (pair? args) (%builtin-make-bytevector a0 (car args)) (%builtin-make-bytevector a0))))
    (define-syntax make-bytevector %cover-make-bytevector)
    (define-syntax %cover-list?
      (syntax-rules ()
        [(_ . args) (%builtin-list? . args)]
        [_ %residual-list?]))
    (define %residual-list? (lambda (a0) (%builtin-list? a0)))
    (define-syntax list? %cover-list?)
    (define-syntax %cover-list-tail
      (syntax-rules ()
        [(_ . args) (%builtin-list-tail . args)]
        [_ %residual-list-tail]))
    (define %residual-list-tail (lambda (a0 a1) (%builtin-list-tail a0 a1)))
    (define-syntax list-tail %cover-list-tail)
    (define-syntax %cover-list-ref
      (syntax-rules ()
        [(_ . args) (%builtin-list-ref . args)]
        [_ %residual-list-ref]))
    (define %residual-list-ref (lambda (a0 a1) (%builtin-list-ref a0 a1)))
    (define-syntax list-ref %cover-list-ref)
    (define-syntax %cover-list->string
      (syntax-rules ()
        [(_ . args) (%builtin-list->string . args)]
        [_ %residual-list->string]))
    (define %residual-list->string (lambda (a0) (%builtin-list->string a0)))
    (define-syntax list->string %cover-list->string)
    (define-syntax %cover-length
      (syntax-rules ()
        [(_ . args) (%builtin-length . args)]
        [_ %residual-length]))
    (define %residual-length (lambda (a0) (%builtin-length a0)))
    (define-syntax length %cover-length)
    (define-syntax %cover-integer?
      (syntax-rules ()
        [(_ . args) (%builtin-integer? . args)]
        [_ %residual-integer?]))
    (define %residual-integer? (lambda (a0) (%builtin-integer? a0)))
    (define-syntax integer? %cover-integer?)
    (define-syntax %cover-integer->char
      (syntax-rules ()
        [(_ . args) (%builtin-integer->char . args)]
        [_ %residual-integer->char]))
    (define %residual-integer->char (lambda (a0) (%builtin-integer->char a0)))
    (define-syntax integer->char %cover-integer->char)
    (define-syntax %cover-input-port?
      (syntax-rules ()
        [(_ . args) (%builtin-input-port? . args)]
        [_ %residual-input-port?]))
    (define %residual-input-port? (lambda (a0) (%builtin-input-port? a0)))
    (define-syntax input-port? %cover-input-port?)
    (define-syntax %cover-input-port-open?
      (syntax-rules ()
        [(_ . args) (%builtin-input-port-open? . args)]
        [_ %residual-input-port-open?]))
    (define %residual-input-port-open? (lambda (a0) (%builtin-input-port-open? a0)))
    (define-syntax input-port-open? %cover-input-port-open?)
    (define-syntax %cover-inexact?
      (syntax-rules ()
        [(_ . args) (%builtin-inexact? . args)]
        [_ %residual-inexact?]))
    (define %residual-inexact? (lambda (a0) (%builtin-inexact? a0)))
    (define-syntax inexact? %cover-inexact?)
    (define-syntax %cover-inexact
      (syntax-rules ()
        [(_ . args) (%builtin-inexact . args)]
        [_ %residual-inexact]))
    (define %residual-inexact (lambda (a0) (%builtin-inexact a0)))
    (define-syntax inexact %cover-inexact)
    (define-syntax %cover-get-output-string
      (syntax-rules ()
        [(_ . args) (%builtin-get-output-string . args)]
        [_ %residual-get-output-string]))
    (define %residual-get-output-string (lambda (a0) (%builtin-get-output-string a0)))
    (define-syntax get-output-string %cover-get-output-string)
    (define-syntax %cover-get-output-bytevector
      (syntax-rules ()
        [(_ . args) (%builtin-get-output-bytevector . args)]
        [_ %residual-get-output-bytevector]))
    (define %residual-get-output-bytevector (lambda (a0) (%builtin-get-output-bytevector a0)))
    (define-syntax get-output-bytevector %cover-get-output-bytevector)
    (define-syntax %cover-flush-output-port
      (syntax-rules ()
        [(_ . args) (%builtin-flush-output-port . args)]
        [_ %residual-flush-output-port]))
    (define %residual-flush-output-port
      (lambda args
        (if (pair? args) (%builtin-flush-output-port (car args)) (%builtin-flush-output-port))))
    (define-syntax flush-output-port %cover-flush-output-port)
    (define-syntax %cover-floor-remainder
      (syntax-rules ()
        [(_ . args) (%builtin-floor-remainder . args)]
        [_ %residual-floor-remainder]))
    (define %residual-floor-remainder (lambda (a0 a1) (%builtin-floor-remainder a0 a1)))
    (define-syntax floor-remainder %cover-floor-remainder)
    (define-syntax %cover-floor-quotient
      (syntax-rules ()
        [(_ . args) (%builtin-floor-quotient . args)]
        [_ %residual-floor-quotient]))
    (define %residual-floor-quotient (lambda (a0 a1) (%builtin-floor-quotient a0 a1)))
    (define-syntax floor-quotient %cover-floor-quotient)
    (define-syntax %cover-floor
      (syntax-rules ()
        [(_ . args) (%builtin-floor . args)]
        [_ %residual-floor]))
    (define %residual-floor (lambda (a0) (%builtin-floor a0)))
    (define-syntax floor %cover-floor)
    (define-syntax %cover-expt
      (syntax-rules ()
        [(_ . args) (%builtin-expt . args)]
        [_ %residual-expt]))
    (define %residual-expt (lambda (a0 a1) (%builtin-expt a0 a1)))
    (define-syntax expt %cover-expt)
    (define-syntax %cover-exact?
      (syntax-rules ()
        [(_ . args) (%builtin-exact? . args)]
        [_ %residual-exact?]))
    (define %residual-exact? (lambda (a0) (%builtin-exact? a0)))
    (define-syntax exact? %cover-exact?)
    (define-syntax %cover-exact-integer?
      (syntax-rules ()
        [(_ . args) (%builtin-exact-integer? . args)]
        [_ %residual-exact-integer?]))
    (define %residual-exact-integer? (lambda (a0) (%builtin-exact-integer? a0)))
    (define-syntax exact-integer? %cover-exact-integer?)
    (define-syntax %cover-exact
      (syntax-rules ()
        [(_ . args) (%builtin-exact . args)]
        [_ %residual-exact]))
    (define %residual-exact (lambda (a0) (%builtin-exact a0)))
    (define-syntax exact %cover-exact)
    (define-syntax %cover-eqv?
      (syntax-rules ()
        [(_ . args) (%builtin-eqv? . args)]
        [_ %residual-eqv?]))
    (define %residual-eqv? (lambda (a0 a1) (%builtin-eqv? a0 a1)))
    (define-syntax eqv? %cover-eqv?)
    (define-syntax %cover-equal?
      (syntax-rules ()
        [(_ . args) (%builtin-equal? . args)]
        [_ %residual-equal?]))
    (define %residual-equal? (lambda (a0 a1) (%builtin-equal? a0 a1)))
    (define-syntax equal? %cover-equal?)
    (define-syntax %cover-eq?
      (syntax-rules ()
        [(_ . args) (%builtin-eq? . args)]
        [_ %residual-eq?]))
    (define %residual-eq? (lambda (a0 a1) (%builtin-eq? a0 a1)))
    (define-syntax eq? %cover-eq?)
    (define-syntax %cover-eof-object?
      (syntax-rules ()
        [(_ . args) (%builtin-eof-object? . args)]
        [_ %residual-eof-object?]))
    (define %residual-eof-object? (lambda (a0) (%builtin-eof-object? a0)))
    (define-syntax eof-object? %cover-eof-object?)
    (define-syntax %cover-eof-object
      (syntax-rules ()
        [(_ . args) (%builtin-eof-object . args)]
        [_ %residual-eof-object]))
    (define %residual-eof-object (lambda () (%builtin-eof-object)))
    (define-syntax eof-object %cover-eof-object)
    (define-syntax %cover-cons
      (syntax-rules ()
        [(_ . args) (%builtin-cons . args)]
        [_ %residual-cons]))
    (define %residual-cons (lambda (a0 a1) (%builtin-cons a0 a1)))
    (define-syntax cons %cover-cons)
    (define-syntax %cover-close-output-port
      (syntax-rules ()
        [(_ . args) (%builtin-close-output-port . args)]
        [_ %residual-close-output-port]))
    (define %residual-close-output-port (lambda (a0) (%builtin-close-output-port a0)))
    (define-syntax close-output-port %cover-close-output-port)
    (define-syntax %cover-close-input-port
      (syntax-rules ()
        [(_ . args) (%builtin-close-input-port . args)]
        [_ %residual-close-input-port]))
    (define %residual-close-input-port (lambda (a0) (%builtin-close-input-port a0)))
    (define-syntax close-input-port %cover-close-input-port)
    (define-syntax %cover-char?
      (syntax-rules ()
        [(_ . args) (%builtin-char? . args)]
        [_ %residual-char?]))
    (define %residual-char? (lambda (a0) (%builtin-char? a0)))
    (define-syntax char? %cover-char?)
    (define-syntax %cover-char>?
      (syntax-rules ()
        [(_ . args) (%builtin-char>? . args)]
        [_ %residual-char>?]))
    (define %residual-char>?
      (lambda (a0 a1 . args) (%builtin-apply %builtin-char>? (cons a0 (cons a1 args)))))
    (define-syntax char>? %cover-char>?)
    (define-syntax %cover-char>=?
      (syntax-rules ()
        [(_ . args) (%builtin-char>=? . args)]
        [_ %residual-char>=?]))
    (define %residual-char>=?
      (lambda (a0 a1 . args) (%builtin-apply %builtin-char>=? (cons a0 (cons a1 args)))))
    (define-syntax char>=? %cover-char>=?)
    (define-syntax %cover-char=?
      (syntax-rules ()
        [(_ . args) (%builtin-char=? . args)]
        [_ %residual-char=?]))
    (define %residual-char=?
      (lambda (a0 a1 . args) (%builtin-apply %builtin-char=? (cons a0 (cons a1 args)))))
    (define-syntax char=? %cover-char=?)
    (define-syntax %cover-char<?
      (syntax-rules ()
        [(_ . args) (%builtin-char<? . args)]
        [_ %residual-char<?]))
    (define %residual-char<?
      (lambda (a0 a1 . args) (%builtin-apply %builtin-char<? (cons a0 (cons a1 args)))))
    (define-syntax char<? %cover-char<?)
    (define-syntax %cover-char<=?
      (syntax-rules ()
        [(_ . args) (%builtin-char<=? . args)]
        [_ %residual-char<=?]))
    (define %residual-char<=?
      (lambda (a0 a1 . args) (%builtin-apply %builtin-char<=? (cons a0 (cons a1 args)))))
    (define-syntax char<=? %cover-char<=?)
    (define-syntax %cover-char->integer
      (syntax-rules ()
        [(_ . args) (%builtin-char->integer . args)]
        [_ %residual-char->integer]))
    (define %residual-char->integer (lambda (a0) (%builtin-char->integer a0)))
    (define-syntax char->integer %cover-char->integer)
    (define-syntax %cover-ceiling
      (syntax-rules ()
        [(_ . args) (%builtin-ceiling . args)]
        [_ %residual-ceiling]))
    (define %residual-ceiling (lambda (a0) (%builtin-ceiling a0)))
    (define-syntax ceiling %cover-ceiling)
    (define-syntax %cover-cdr
      (syntax-rules ()
        [(_ . args) (%builtin-cdr . args)]
        [_ %residual-cdr]))
    (define %residual-cdr (lambda (a0) (%builtin-cdr a0)))
    (define-syntax cdr %cover-cdr)
    (define-syntax %cover-car
      (syntax-rules ()
        [(_ . args) (%builtin-car . args)]
        [_ %residual-car]))
    (define %residual-car (lambda (a0) (%builtin-car a0)))
    (define-syntax car %cover-car)
    (define-syntax %cover-bytevector?
      (syntax-rules ()
        [(_ . args) (%builtin-bytevector? . args)]
        [_ %residual-bytevector?]))
    (define %residual-bytevector? (lambda (a0) (%builtin-bytevector? a0)))
    (define-syntax bytevector? %cover-bytevector?)
    (define-syntax %cover-bytevector-u8-set!
      (syntax-rules ()
        [(_ . args) (%builtin-bytevector-u8-set! . args)]
        [_ %residual-bytevector-u8-set!]))
    (define %residual-bytevector-u8-set! (lambda (a0 a1 a2) (%builtin-bytevector-u8-set! a0 a1 a2)))
    (define-syntax bytevector-u8-set! %cover-bytevector-u8-set!)
    (define-syntax %cover-bytevector-u8-ref
      (syntax-rules ()
        [(_ . args) (%builtin-bytevector-u8-ref . args)]
        [_ %residual-bytevector-u8-ref]))
    (define %residual-bytevector-u8-ref (lambda (a0 a1) (%builtin-bytevector-u8-ref a0 a1)))
    (define-syntax bytevector-u8-ref %cover-bytevector-u8-ref)
    (define-syntax %cover-bytevector-length
      (syntax-rules ()
        [(_ . args) (%builtin-bytevector-length . args)]
        [_ %residual-bytevector-length]))
    (define %residual-bytevector-length (lambda (a0) (%builtin-bytevector-length a0)))
    (define-syntax bytevector-length %cover-bytevector-length)
    (define-syntax %cover-boolean?
      (syntax-rules ()
        [(_ . args) (%builtin-boolean? . args)]
        [_ %residual-boolean?]))
    (define %residual-boolean? (lambda (a0) (%builtin-boolean? a0)))
    (define-syntax boolean? %cover-boolean?)
    (define-syntax %cover-boolean=?
      (syntax-rules ()
        [(_ . args) (%builtin-boolean=? . args)]
        [_ %residual-boolean=?]))
    (define %residual-boolean=?
      (lambda (a0 a1 . args) (%builtin-apply %builtin-boolean=? (cons a0 (cons a1 args)))))
    (define-syntax boolean=? %cover-boolean=?)
    (define-syntax %cover-assv
      (syntax-rules ()
        [(_ . args) (%builtin-assv . args)]
        [_ %residual-assv]))
    (define %residual-assv (lambda (a0 a1) (%builtin-assv a0 a1)))
    (define-syntax assv %cover-assv)
    (define-syntax %cover-assq
      (syntax-rules ()
        [(_ . args) (%builtin-assq . args)]
        [_ %residual-assq]))
    (define %residual-assq (lambda (a0 a1) (%builtin-assq a0 a1)))
    (define-syntax assq %cover-assq)
    (define-syntax %cover-abs
      (syntax-rules ()
        [(_ . args) (%builtin-abs . args)]
        [_ %residual-abs]))
    (define %residual-abs (lambda (a0) (%builtin-abs a0)))
    (define-syntax abs %cover-abs)
    (define-syntax %cover->=
      (syntax-rules ()
        [(_ . args) (%builtin->= . args)]
        [_ %residual->=]))
    (define %residual->=
      (lambda (a0 a1 . args) (%builtin-apply %builtin->= (cons a0 (cons a1 args)))))
    (define-syntax >= %cover->=)
    (define-syntax %cover->
      (syntax-rules ()
        [(_ . args) (%builtin-> . args)]
        [_ %residual->]))
    (define %residual->
      (lambda (a0 a1 . args) (%builtin-apply %builtin-> (cons a0 (cons a1 args)))))
    (define-syntax > %cover->)
    (define-syntax %cover-=
      (syntax-rules ()
        [(_ . args) (%builtin-= . args)]
        [_ %residual-=]))
    (define %residual-=
      (lambda (a0 a1 . args) (%builtin-apply %builtin-= (cons a0 (cons a1 args)))))
    (define-syntax = %cover-=)
    (define-syntax %cover-<=
      (syntax-rules ()
        [(_ . args) (%builtin-<= . args)]
        [_ %residual-<=]))
    (define %residual-<=
      (lambda (a0 a1 . args) (%builtin-apply %builtin-<= (cons a0 (cons a1 args)))))
    (define-syntax <= %cover-<=)
    (define-syntax %cover-<
      (syntax-rules ()
        [(_ . args) (%builtin-< . args)]
        [_ %residual-<]))
    (define %residual-<
      (lambda (a0 a1 . args) (%builtin-apply %builtin-< (cons a0 (cons a1 args)))))
    (define-syntax < %cover-<)
    (define-syntax %cover-/
      (syntax-rules ()
        [(_ . args) (%builtin-/ . args)]
        [_ %residual-/]))
    (define %residual-/
      (lambda (a0 . args) (%builtin-apply %builtin-/ (cons a0 args))))
    (define-syntax / %cover-/)
    (define-syntax %cover--
      (syntax-rules ()
        [(_ . args) (%builtin-- . args)]
        [_ %residual--]))
    (define %residual--
      (lambda (a0 . args) (%builtin-apply %builtin-- (cons a0 args))))
    (define-syntax - %cover--)
    (define-syntax %cover-+
      (syntax-rules ()
        [(_ . args) (%builtin-+ . args)]
        [_ %residual-+]))
    (define %residual-+ (lambda args (%builtin-apply %builtin-+ args)))
    (define-syntax + %cover-+)
    (define-syntax %cover-*
      (syntax-rules ()
        [(_ . args) (%builtin-* . args)]
        [_ %residual-*]))
    (define %residual-* (lambda args (%builtin-apply %builtin-* args)))
    (define-syntax * %cover-*)
    (define-syntax %cover-values
      (syntax-rules ()
        [(_ . args) (%builtin-values . args)]
        [_ %residual-values]))
    (define %residual-values (lambda args (%builtin-apply %builtin-values args)))
    (define-syntax values %cover-values)
    (define-syntax %cover-call-with-values
      (syntax-rules ()
        [(_ . args) (%builtin-call-with-values . args)]
        [_ %residual-call-with-values]))
    (define %residual-call-with-values (lambda args (%builtin-apply %builtin-call-with-values args)))
    (define-syntax call-with-values %cover-call-with-values)
))
