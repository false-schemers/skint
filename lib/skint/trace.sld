
(define-library (skint trace)

  (import
    (scheme base)
    (scheme write)
    (scheme case-lambda)
    (only (skint) body))

  (export
    trace-lambda
    trace-case-lambda
    trace-let
    trace-do
    trace-define
    trace
    untrace
    trace-output-port
    trace-print)

(begin

;; --------------------------------------------------------------------------
;; Output
;; --------------------------------------------------------------------------

(define trace-output-port (make-parameter (current-output-port)))

(define trace-print (make-parameter write))

;; current nesting depth; a parameter so that an escape out of a traced call
;; unwinds it correctly
(define %depth (make-parameter 0))

;; Chez's scheme: "| " repeated, one character per level, so depth 0 is "|",
;; depth 1 is "| ", depth 2 is "| |"; from depth 10 on, a bracketed number
(define (%indent d port)
  (if (>= d 10)
      (begin
        (write-char #\[ port)
        (write-string (number->string d) port)
        (write-char #\] port))
      (let loop ([i 0])
        (when (<= i d)
          (write-char (if (even? i) #\| #\space) port)
          (loop (+ i 1))))))

(define (%show-call d name args port)
  (%indent d port)
  ((trace-print) (cons name args) port)
  (newline port))

(define (%show-values d vals port)
  (%indent d port)
  (let loop ([v vals] [sep ""])
    (unless (null? v)
      (write-string sep port)
      ((trace-print) (car v) port)
      (loop (cdr v) " ")))
  (newline port))

;; --------------------------------------------------------------------------
;; The traced-call protocol
;; --------------------------------------------------------------------------

;; Print the call, run the procedure one level deeper, print what came back.
;; NB: the call is not a tail call -- see the documentation.
(define (%trace-call name proc args)
  (let ([d (%depth)] [port (trace-output-port)])
    (%show-call d name args port)
    (call-with-values
      (lambda () (parameterize ([%depth (+ d 1)]) (apply proc args)))
      (lambda vals
        (%show-values d vals port)
        (apply values vals)))))

(define (%traced name proc)
  (lambda args (%trace-call name proc args)))

;; --------------------------------------------------------------------------
;; Registry behind trace and untrace
;; --------------------------------------------------------------------------

(define-record-type <entry>
  (%make-entry name get set original traced)
  %entry?
  (name %entry-name)
  (get %entry-get)
  (set %entry-set)
  (original %entry-original)
  (traced %entry-traced))

;; most recently traced first, which is the order Chez reports them in
(define %registry '())

;; a variable counts as traced only while it still holds the procedure we
;; installed; assigning it anything else silently drops it from the list
(define (%live? e) (eq? ((%entry-get e)) (%entry-traced e)))

(define (%without name entries)
  (let loop ([l entries] [r '()])
    (cond [(null? l) (reverse r)]
          [(eq? (%entry-name (car l)) name) (loop (cdr l) r)]
          [else (loop (cdr l) (cons (car l) r))])))

(define (%trace-one! name get set)
  (let ([e (%find name)])
    (unless (and e (%live? e))
      (let* ([original (get)] [traced (%traced name original)])
        (set! %registry (cons (%make-entry name get set original traced)
                              (%without name %registry)))
        (set traced)))))

(define (%find name)
  (let loop ([l %registry])
    (cond [(null? l) #f]
          [(eq? (%entry-name (car l)) name) (car l)]
          [else (loop (cdr l))])))

(define (%prune!)
  (let loop ([l %registry] [r '()])
    (cond [(null? l) (set! %registry (reverse r)) %registry]
          [(%live? (car l)) (loop (cdr l) (cons (car l) r))]
          [else (loop (cdr l) r)])))

(define (%traced-names)
  (map %entry-name (%prune!)))

(define (%untrace! keep?)
  (let loop ([l %registry] [kept '()] [gone '()])
    (cond [(null? l)
           (set! %registry (reverse kept))
           (reverse gone)]
          [(not (%live? (car l)))
           (loop (cdr l) kept gone)]
          [(keep? (%entry-name (car l)))
           (loop (cdr l) (cons (car l) kept) gone)]
          [else
           ((%entry-set (car l)) (%entry-original (car l)))
           (loop (cdr l) kept (cons (%entry-name (car l)) gone))])))

(define (%untrace-names! names)
  (%untrace! (lambda (n) (not (memq n names)))))

(define (%untrace-all!)
  (%untrace! (lambda (n) #f)))

)

;; --------------------------------------------------------------------------
;; Syntax
;; --------------------------------------------------------------------------

(begin

;; the step of a do binding is optional, as it is in skint's own do
(define-syntax %step
  (syntax-rules ()
    [(_ x) x]
    [(_ x y) y]))

(define-syntax trace-lambda
  (syntax-rules ()
    [(_ name formals . forms)
     (%traced 'name (lambda formals . forms))]))

(define-syntax trace-case-lambda
  (syntax-rules ()
    [(_ name clause ...)
     (%traced 'name (case-lambda clause ...))]))

(define-syntax trace-let
  (syntax-rules ()
    [(_ name ([var init] ...) . forms)
     ((letrec ([name (trace-lambda name (var ...) . forms)]) name) init ...)]))

;; Unlike the others this one keeps its loop flat and its tail call intact:
;; the recursion belongs to the macro, so it is known to be a tail call and
;; every iteration can be printed at the same depth, as Chez does.
(define-syntax trace-do
  (syntax-rules ()
    [(_ ([var init step ...] ...) [test expr ...] command ...)
     (let ([port (trace-output-port)] [d (%depth)])
       (let loop ([var init] ...)
         (%show-call d 'do (list var ...) port)
         (if test
             (call-with-values
               (lambda () (parameterize ([%depth (+ d 1)]) (body expr ...)))
               (lambda vals
                 (%show-values d vals port)
                 (apply values vals)))
             (begin
               (parameterize ([%depth (+ d 1)]) (body command ... #f))
               (loop (%step var step ...) ...)))))]))

(define-syntax trace-define
  (syntax-rules ()
    [(_ (var . idspec) . forms)
     (define var (trace-lambda var idspec . forms))]
    [(_ var expr)
     (define var (let ([p expr]) (%traced 'var p)))]))

(define-syntax trace
  (syntax-rules ()
    [(_) (%traced-names)]
    [(_ var ...)
     (begin
       (%trace-one! 'var (lambda () var) (lambda (v) (set! var v)))
       ...
       (list 'var ...))]))

(define-syntax untrace
  (syntax-rules ()
    [(_) (%untrace-all!)]
    [(_ var ...) (%untrace-names! (list 'var ...))]))

))
