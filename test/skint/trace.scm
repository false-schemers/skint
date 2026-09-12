(import (skint trace))

(include "test.scm")

;; ---------------------------------------------------------------------------
;; Helpers
;; ---------------------------------------------------------------------------

;; run thunk with the trace output captured, and return that output
(define (out thunk)
  (let ([p (open-output-string)])
    (parameterize ([trace-output-port p]) (thunk))
    (get-output-string p)))

;; run thunk with the trace output captured, and return its value instead
(define (val thunk)
  (let ([p (open-output-string)])
    (parameterize ([trace-output-port p]) (thunk))))


(display "\n--- trace-lambda ---\n")

(define double (trace-lambda double (x) (* x 2)))

(test "a call prints the form, then the result" "|(double 3)\n|6\n"
  (out (lambda () (double 3))))

(test "the value still comes back" 6 (val (lambda () (double 3))))

(test "the name is the one given, not the variable it is bound to"
  "|(double 3)\n|6\n"
  (let ([other double]) (out (lambda () (other 3)))))

(test "several arguments" "|(add 1 2 3)\n|6\n"
  (out (lambda () ((trace-lambda add (a b c) (+ a b c)) 1 2 3))))

(test "no arguments" "|(thunk)\n|7\n"
  (out (lambda () ((trace-lambda thunk () 7)))))

(test "a rest argument" "|(rest 1 2 3)\n|(1 2 3)\n"
  (out (lambda () ((trace-lambda rest args args) 1 2 3))))

(test "an improper formals list" "|(mixed 1 2 3)\n|(1 (2 3))\n"
  (out (lambda () ((trace-lambda mixed (a . b) (list a b)) 1 2 3))))

(test "internal defines are allowed in the body, as in skint's lambda"
  "|(withdef 3)\n|7\n"
  (out (lambda () ((trace-lambda withdef (x) (define y (* x 2)) (+ y 1)) 3))))


(display "\n--- nesting and indentation ---\n")

(define (nest n)
  ;; a chain of n non-tail traced calls
  (let loop ([i 0])
    (if (>= i n) 0 (+ 1 ((trace-lambda d (k) (loop k)) (+ i 1))))))

(test "depth 0 is a bar, depth 1 adds a space, depth 2 adds a bar"
  "|(d 1)\n| (d 2)\n| |(d 3)\n| |0\n| 1\n|2\n"
  (out (lambda () (nest 3))))

(test "from depth 10 on, a bracketed number replaces the bars"
  #t
  (let ([s (out (lambda () (nest 11)))])
    (and (string? s)
         ;; the eleventh entry is the first one at depth 10
         (let loop ([i 0] [found #f])
           (cond [(>= i (- (string-length s) 4)) found]
                 [(and (char=? (string-ref s i) #\[)
                       (char=? (string-ref s (+ i 1)) #\1)
                       (char=? (string-ref s (+ i 2)) #\0)
                       (char=? (string-ref s (+ i 3)) #\]))
                  #t]
                 [else (loop (+ i 1) found)])))))

(test "a tail call nests, unlike Chez, because it cannot be told from a
       non-tail call -- this pins the documented behaviour"
  "|(t 2)\n| (t 1)\n| |(t 0)\n| |done\n| done\n|done\n"
  (out (lambda ()
         (define t (trace-lambda t (n) (if (= n 0) 'done (t (- n 1)))))
         (t 2))))


(display "\n--- values ---\n")

(test "several values print space separated" "|(two 4)\n|4 8\n"
  (out (lambda () ((trace-lambda two (x) (values x (* x 2))) 4))))

(test-values (values 4 8)
  (val (lambda () ((trace-lambda two (x) (values x (* x 2))) 4))))

(test "no values at all" "|(none)\n|\n"
  (out (lambda () ((trace-lambda none () (values))))))


(display "\n--- trace-let ---\n")

(test "it is a named let" 120
  (val (lambda () (trace-let fact ([n 5]) (if (= n 0) 1 (* n (fact (- n 1))))))))

(test "each entry is printed" "|(fact 2)\n| (fact 1)\n| |(fact 0)\n| |1\n| 1\n|2\n"
  (out (lambda () (trace-let fact ([n 2]) (if (= n 0) 1 (* n (fact (- n 1))))))))

(test "no bindings at all, which Chez uses to show an expression's value"
  "|(here)\n|42\n"
  (out (lambda () (trace-let here () 42))))

(test "internal defines are allowed in the body"
  20
  (val (lambda () (trace-let g ([n 2]) (define m (* n 10)) m))))

(test "bindings may use brackets, as everywhere else in skint"
  6 (val (lambda () (trace-let g ([a 1] [b 2] [c 3]) (+ a b c)))))


(display "\n--- trace-do ---\n")

;; the do loop is the macro's own, so it is known to be a tail call: iterations
;; stay at one depth and only one result line is printed, exactly as in Chez

(test "Chez's own example, verbatim"
  "|(do (a b c) ())\n|(do (b c) (a))\n|(do (c) (b a))\n|(do () (c b a))\n|(c b a)\n"
  (out (lambda ()
         (trace-do ([old '(a b c) (cdr old)]
                    [new '() (cons (car old) new)])
           ((null? old) new)))))

(test "and it returns what do would" '(c b a)
  (val (lambda ()
         (trace-do ([old '(a b c) (cdr old)]
                    [new '() (cons (car old) new)])
           ((null? old) new)))))

(test "a binding may omit its step, as in skint's do"
  "|(do 0 9)\n|(do 1 9)\n|(do 2 9)\n|9\n"
  (out (lambda () (trace-do ([i 0 (+ i 1)] [fixed 9]) [(= i 2) fixed]))))

(test "commands run each iteration" 3
  (val (lambda ()
         (let ([n 0])
           (trace-do ([i 0 (+ i 1)]) [(= i 3) n] (set! n (+ n 1)))))))

(test "the result sequence is a body, so it may hold defines" 6
  (val (lambda () (trace-do ([i 0 (+ i 1)]) [(= i 3) (define r (* i 2)) r]))))

;; an empty result sequence is allowed, as it is in skint's do; the value it
;; yields is unspecified there and here, so only the iteration lines are checked
(test "an empty result sequence is allowed"
  "|(do 0)\n|(do 1)\n"
  (let ([s (out (lambda () (trace-do ([i 0 (+ i 1)]) [(= i 1)])))])
    (substring s 0 16)))

(test "the loop stays flat however many iterations it runs"
  "|(do 0)\n|(do 1)\n|(do 2)\n|(do 3)\n|(do 4)\n|done\n"
  (out (lambda () (trace-do ([i 0 (+ i 1)]) [(= i 4) 'done]))))


(display "\n--- trace-define ---\n")

(trace-define (plus x y) (+ x y))

(test "the procedure form traces under its own name" "|(plus 3 4)\n|7\n"
  (out (lambda () (plus 3 4))))

(test "and returns normally" 7 (val (lambda () (plus 3 4))))

(trace-define (variadic a . rest) (cons a rest))

(test "the procedure form accepts skint's full formals syntax"
  "|(variadic 1 2 3)\n|(1 2 3)\n"
  (out (lambda () (variadic 1 2 3))))

(trace-define timesthree (lambda (x) (* x 3)))

(test "the expression form wraps a procedure the expression produced"
  "|(timesthree 5)\n|15\n"
  (out (lambda () (timesthree 5))))

(test "Chez's example: it works in an internal body too" '(7 11)
  (val (lambda ()
         (let ()
           (trace-define inner (lambda (x y) (+ x y)))
           (list (inner 3 4) (+ 5 6))))))


(display "\n--- trace-case-lambda ---\n")

(define pick
  (trace-case-lambda pick
    [(x) (list 'one x)]
    [(x y) (list 'two x y)]
    [args (list 'many args)]))

(test "|(pick 7)\n|(one 7)\n" (out (lambda () (pick 7))))
(test "|(pick 7 8)\n|(two 7 8)\n" (out (lambda () (pick 7 8))))
(test "|(pick 7 8 9)\n|(many (7 8 9))\n" (out (lambda () (pick 7 8 9))))

(test "clause bodies may hold defines, as in skint's case-lambda"
  '(one 14)
  (val (lambda ()
         ((trace-case-lambda cl [(x) (define d (* x 2)) (list 'one d)]) 7))))


(display "\n--- trace and untrace ---\n")

(define (sq x) (* x x))
(define (cube x) (* x x x))

(test "trace returns the names it was given" '(sq) (trace sq))

(test "and the variable now traces" "|(sq 3)\n|9\n"
  (out (lambda () (sq 3))))

(test "trace with no arguments lists what is traced" '(sq) (trace))

(test "tracing a second one" '(cube) (trace cube))

(test "the list is most recently traced first" '(cube sq) (trace))

(test "untrace returns what it actually untraced" '(sq) (untrace sq))

(test "and the variable is back to normal" "" (out (lambda () (sq 3))))

(test 9 (val (lambda () (sq 3))))

(test "untracing something already untraced reports nothing" '() (untrace sq))

(test "untrace with no arguments untraces everything left" '(cube) (untrace))

(test "nothing is traced now" '() (trace))

(test "re-assigning a traced variable silently drops it, as in Chez"
  '()
  (begin
    (trace sq)
    (set! sq (lambda (x) 0))
    (trace)))

(test "so untracing it reports nothing and leaves the new value alone"
  '(() 0)
  (list (untrace sq) (sq 5)))

(test "tracing the same variable twice does not stack wrappers"
  "|(cube 2)\n|8\n"
  (begin
    (set! cube (lambda (x) (* x x x)))
    (trace cube)
    (trace cube)
    (out (lambda () (cube 2)))))

(test '(cube) (untrace))


(display "\n--- trace-output-port and trace-print ---\n")

(test "the default output port is a port" #t (port? (trace-output-port)))

(test "the default printer is write" #t (eq? (trace-print) write))

(test "trace-print is consulted for both lines"
  "|<(double 3)>\n|<6>\n"
  (out (lambda ()
         (parameterize ([trace-print
                         (lambda (x port)
                           (display "<" port) (display x port) (display ">" port))])
           (double 3)))))

(test "write is used by default, so strings are quoted"
  "|(id \"hi\")\n|\"hi\"\n"
  (out (lambda () ((trace-lambda id (s) s) "hi"))))

(test "nothing reaches the default port while it is rebound" ""
  (let ([p (open-output-string)])
    (parameterize ([trace-output-port p]) (double 1))
    (let ([q (open-output-string)])
      (parameterize ([trace-output-port q]) #f)
      (get-output-string q))))


(display "\n--- All tests complete. ---\n")

(test-end)
