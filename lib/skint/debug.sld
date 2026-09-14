;; A SKETCH, not a finished library.  It exists to answer one question: can the
;; stack captured in a failure object be walked and printed frame by frame?
;;
;; On load it installs a handler into the current-exception-handler parameter,
;; keeping the previous value as the fall-back.  A failure object is walked and
;; printed, then handed on to the fall-back so that reporting and reset happen
;; as before; anything else goes straight to the fall-back.

(define-library (skint debug)

  (import (scheme base) (scheme read) (scheme write) (scheme case-lambda) (scheme cxr))
  (import (skint print))
  (import (only (skint disasm) da da-name da-prune-globals))
  (import (only (skint) failure-object? failure-object-message current-debugger
                        void
                        failure-object-irritants))
  (import (only (skint hidden) closure? closure->vector set-port-prompt!
                               note-error! repl-from-port repl-evaluate-top-form))

  (export failure-frames continuation-frames print-failure-frames
          print-frames debugger)

(begin

;; ---------------------------------------------------------------------------
;; Walking the captured stack
;; ---------------------------------------------------------------------------
;;
;; The failure object is a vector.  Slots [0 .. frames-end) are the captured vm
;; stack, OLDEST FIRST; above them sit the irritants, their count, and the
;; message.
;;
;; A call pushes exactly two words (i.c, save): the calling procedure, then
;; the offset at which it will resume inside its OWN code vector.  return
;; pops them in reverse.  Between two such pairs lie that frame working
;; values -- arguments being accumulated, saved temporaries.
;;
;; So a frame start is: a closure, followed by an exact non-negative fixnum
;; that is a legal offset into that closure own code vector.
;;
;; THIS IS A HEURISTIC, NOT A PARSE.  It never MISSES a real frame -- every
;; frame satisfies it by construction -- but it does report extra ones: a
;; procedure stored as DATA next to a small exact fixnum passes the same test.
;; Verified, not assumed: (boom my-proc 2 (list 1)) puts my-proc on the
;; stack beside a fixnum and the test accepts it.  Read a backtrace from here
;; as a superset of the truth.

(define (code-of p) (vector-ref (closure->vector p) 0))

(define (frame-start? v i)
  (and (< (+ i 1) (vector-length v))
       (closure? (vector-ref v i))
       (let ([n (vector-ref v (+ i 1))])
         (and (integer? n) (exact? n) (>= n 0)
              (<= n (vector-length (code-of (vector-ref v i))))))))

;; A failure object is a continuation: [0] the adapter code, [1] the dynamic
;; state, [2..] the stack image -- and above the image sit the irritants, their
;; count, the message, and the halt return frame that makes an accidental call
;; reset instead of resuming. So the stack runs from 2 up to
;; (len - 4 - #irritants), and the SAME walk serves a real continuation with a
;; hi of (len) instead.

(define (frame-starts v lo hi)
  (let loop ([i lo] [r '()])
    (cond [(>= i hi) (reverse r)]
          [(frame-start? v i) (loop (+ i 2) (cons i r))]
          [else (loop (+ i 1) r)])))

;; => a list of (procedure return-offset (working-value ...)), INNERMOST FIRST
(define (walk-frames v lo hi)
  (let loop ([ss (frame-starts v lo hi)] [r '()])
    (if (null? ss)
        r                                    ; consed while ascending => reversed
        (let* ([i (car ss)]
               [next (if (null? (cdr ss)) hi (cadr ss))]
               [vals (let g ([j (+ i 2)] [a '()])
                       (if (>= j next) (reverse a)
                           (g (+ j 1) (cons (vector-ref v j) a))))])
          (loop (cdr ss)
                (cons (list (vector-ref v i) (vector-ref v (+ i 1)) vals) r))))))

(define (failure-frames obj)
  (let ([v (closure->vector obj)])
    (walk-frames v 2 (- (vector-length v) 4 (length (failure-object-irritants obj))))))

;; the same walk over a live continuation, for comparison in a debugger
(define (continuation-frames k)
  (let ([v (closure->vector k)])
    (walk-frames v 2 (vector-length v))))

;; ---------------------------------------------------------------------------
;; Which frames are the program's
;; ---------------------------------------------------------------------------
;;
;; A stack captured at the REPL holds more than the program that failed.  Outside
;; it is the REPL itself, from the frame of repl-evaluate-top-form out.  Inside
;; it, for an error that was raised, is the way the error was noted: the body of
;; raise, the handler repl-from-port installs, and note-error!, where the stack
;; was captured.  A failure object's stack starts at the failing procedure, so it
;; has no such head.  The debugger shows neither end.
;;
;; A frame is known by its code: a procedure's code vector, or the code vector
;; of a lambda inside it, which is nested in its own.  A frame at offset 0 is not
;; a call waiting to return -- no call returns to the first instruction -- and is
;; the adapter that returns to a call-with-values or parameterize, which is left
;; out where it borders either cut.

;; the code vectors of P and of every lambda written inside it
(define (code-vectors p)
  (let walk ([cv (code-of p)] [acc '()])
    (let loop ([i 0] [acc (cons cv acc)])
      (if (= i (vector-length cv))
          acc
          (loop (+ i 1) (let ([x (vector-ref cv i)]) (if (vector? x) (walk x acc) acc)))))))

(define reporting-code
  (apply append (map code-vectors (list note-error! repl-from-port raise raise-continuable))))

(define (adapter-frame? f) (eqv? (cadr f) 0))
(define (reporting-frame? f)
  (or (adapter-frame? f) (memq (code-of (car f)) reporting-code)))

;; FS, innermost first, without the REPL outside the program or the reporting
;; inside it; the whole of FS when no REPL frame is found, as for a continuation
;; captured some other way
(define (program-frames fs)
  (let ([inner (let take ([l fs])
                 (cond [(null? l) '()]
                       [(eq? (car (car l)) repl-evaluate-top-form) '()]
                       [else (cons (car l) (take (cdr l)))]))])
    (if (= (length inner) (length fs))
        fs
        (let drop ([l (reverse inner)])       ; outermost first: the adapters by the REPL
          (if (and (pair? l) (adapter-frame? (car l)))
              (drop (cdr l))
              (let head ([l (reverse l)])     ; innermost first again: the reporting
                (if (and (pair? l) (reporting-frame? (car l)))
                    (head (cdr l))
                    l)))))))

;; ---------------------------------------------------------------------------
;; Printing
;; ---------------------------------------------------------------------------

;; values are printed with length and level capped at 3, so one deep or long
;; value cannot bury the trace; the code under a frame is printed whole, since
;; cutting it could cut out the call it marks

(define (put x port) (print x port print-length 3 print-level 3))

(define print-failure-frames
  (case-lambda
    [(obj) (print-failure-frames obj (current-error-port))]
    [(obj port)
     (print-failure obj port)
     (print-frames (failure-frames obj) port)]))

(define (print-failure obj port)
  (display "Failure: " port)
  (display (failure-object-message obj) port)
  (for-each (lambda (a) (display " " port) (put a port))
            (failure-object-irritants obj))
  (newline port))

;; ---------------------------------------------------------------------------
;; The debugger
;; ---------------------------------------------------------------------------
;;
;; Loading this library installs the procedure below as current-debugger, which
;; is what the ,db repl command invokes. It is handed whatever the last error left behind:
;; a failure object, or (continuation . error-object) for an ordinary error,
;; whose stack had to be captured because it was still live. Nothing here
;; touches current-exception-handler; reporting stays where it was.  Only the
;; program's own frames are shown; failure-frames and continuation-frames still
;; give the whole stack.

(define (print-frames fs port)
  (let loop ([fs fs] [n 0])
    (unless (null? fs)
      (print-frame-header (car fs) n port)
      (print-frame-code (car fs) port)
      (loop (cdr fs) (+ n 1)))))

;; one line for frame F, numbered N: its procedure, its offset, its working values;
;; with a true CURRENT?, marked as the frame the debugger is on
(define (print-frame-header f n port . current?)
  (display (if (and (pair? current?) (car current?)) "* " "  ") port)
  (display n port) (display ": " port)
  (put (procedure-label (car f)) port)
  (display " @" port) (display (cadr f) port)
  (for-each (lambda (v) (display " " port) (put v port)) (caddr f))
  (newline port))

;; A procedure is shown by the name the store files it under, where it has one:
;; the one thing to say about a procedure with no readable code, such as the
;; wrapper a built-in like car gets when it is passed as a value.
(define (procedure-label p)
  (or (guard (x (#t #f)) (da-name p)) p))

;; Under a frame, the code it is running, as Scheme, with the expression it is
;; stopped at marked.  The offset a frame holds is the cursor (skint disasm)
;; takes: for a frame waiting on a call it is where the call returns, and for the
;; frame a failure pushed it is just past the instruction that failed, so the
;; mark is on the call or on the operation that failed.  A frame whose offset
;; points at nothing gets no code, which leaves out procedures with no readable
;; form and most of the false frames the walk above can report.  A disassembly
;; that fails shows nothing rather than cutting the trace short.  #t if there was
;; code to show.
(define (print-frame-code f port)
  (let* ([where #f]
         [form (guard (x (#t #f))
                 (da (car f) (cadr f) (lambda (w) (set! where w))))])
    (and (pair? form) where
         (begin (display "     " port)
                (pretty-print form port print-cursor where print-indent 5)
                #t))))

;; ---------------------------------------------------------------------------
;; The command loop
;; ---------------------------------------------------------------------------
;;
;; A disassembly can run to many lines, and a stack to many frames, so the
;; debugger lists the frames with the code of the first only, and then takes
;; commands to show the others.  A much smaller cousin of Chez Scheme's debug>
;; prompt and the inspector behind it, borrowing their letters.

(define debug-help
  '("Type <n>  to show frame n: its code, with the expression it is stopped at marked"
    "     d    to show the next frame down, the caller of this one"
    "     u    to show the next frame up, the one this one called"
    "     s    to show this frame again"
    "     sf   to list the frames"
    "     g    to switch between pruned and full global names, for this debugger only"
    "     e    or eof to exit the debugger"))

;; list the frames FS, with the code of the first, then take commands
(define (debug-frames fs port)
  (let loop ([l fs] [n 0])
    (unless (null? l)
      (print-frame-header (car l) n port)
      (when (= n 0) (print-frame-code (car l) port))
      (loop (cdr l) (+ n 1))))
  (debug-loop fs port))

(define (debug-loop fs port)
  (define ip (current-input-port))
  (define count (length fs))
  (define current 0)
  ;; what da-prune-globals is while a command runs; g flips it here and only here
  (define prune (da-prune-globals))
  (define (say . xs) (for-each (lambda (x) (display x port)) xs) (newline port))
  (define (show n)
    (cond [(zero? count) (say "There are no frames.")]
          [(and (>= n 0) (< n count))
           (set! current n)
           (print-frame-header (list-ref fs n) n port)
           (unless (print-frame-code (list-ref fs n) port)
             (say "     (no code to show)"))]
          [else (say "There is no frame " n ".")]))
  (define (list-frames)
    (when (zero? count) (say "There are no frames."))
    (let loop ([l fs] [n 0])
      (unless (null? l)
        (print-frame-header (car l) n port (= n current))
        (loop (cdr l) (+ n 1)))))
  (let loop ()
    (or (set-port-prompt! ip "debug> ")
        (begin (display "debug> " port) (flush-output-port port)))
    (let ([line (read-line ip)])
      (unless (eof-object? line)
        (let ([cmd (guard (x (#t #f)) (read (open-input-string line)))])
          (cond [(eof-object? cmd) (loop)]
                [(eq? cmd 'e)]
                [else
                 (parameterize ([da-prune-globals prune])
                   (cond [(and (integer? cmd) (exact? cmd)) (show cmd)]
                         [(eq? cmd 'd) (show (+ current 1))]
                         [(eq? cmd 'u) (show (- current 1))]
                         [(eq? cmd 's) (show current)]
                         [(eq? cmd 'sf) (list-frames)]
                         [(eq? cmd 'g)
                          (set! prune (not prune))
                          (say (if prune
                                   "Global names are pruned to their identifiers."
                                   "Global names are shown in full."))]
                         [(eq? cmd '?) (for-each say debug-help)]
                         [else (say "Invalid command.  Type ? for options.")]))
                 (loop)]))))))

(define debugger
  (case-lambda
    [() (debugger #f)]
    [(e)
     (let ([p (current-error-port)])
       (cond
         [(not e)
          (display "No error to debug." p) (newline p)]
         [(failure-object? e)
          (print-failure e p)
          (debug-frames (program-frames (failure-frames e)) p)]
         [(pair? e)
          (display "Error: " p) (put (cdr e) p) (newline p)
          (debug-frames (program-frames (continuation-frames (car e))) p)]
         [else
          (display "Unrecognized last error: " p) (put e p) (newline p)]))
     (void)]))

(current-debugger debugger)

))
