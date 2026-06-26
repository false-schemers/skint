
(define *tests-run* 0)
(define *tests-passed* 0)

(define current-test-comparator (make-parameter equal?))

(define-syntax test
  (syntax-rules ()
    ((test name expect expr)
     (test expect expr))
    ((test expect expr)
     (begin
       (set! *tests-run* (+ *tests-run* 1))
       (let ((display-str
              (lambda ()
                (write *tests-run*)
                (display ". ")
                (write 'expr)))
             (res expr))
         (display-str)
         (write-char #\space)
         (display (make-string 10 #\.))
         (cond
          (((current-test-comparator) res expect)
           (set! *tests-passed* (+ *tests-passed* 1))
           (display " [PASS]\n"))
          (else
           (display " [FAIL]\n")
           (display "********** expected ") (write expect)
           (display " but got ") (write res) (newline))))))))

(define-syntax test-assert
  (syntax-rules ()
    ((test-assert expr) 
     (parameterize ((current-test-comparator equal?)) (test #f (not expr))))
    ((test-assert name expr) 
     (parameterize ((current-test-comparator equal?)) (test name #f (not expr))))))

(define-syntax test-values
  (syntax-rules ()
    ((test-values vals expr)
     (parameterize ((current-test-comparator equal?)) 
       (test (call-with-values (lambda () vals) list) (call-with-values (lambda () expr) list))))
    ((test-values name vals expr)
     (parameterize ((current-test-comparator equal?)) 
       (test (call-with-values (lambda () vals) list) (call-with-values (lambda () expr) list))))))

(define-syntax test~=
  (syntax-rules ()
    ((test~= val expr) 
     (parameterize ((current-test-comparator equal?))
       (test (number->string val) (number->string expr))))))

(define-syntax test-not
  (syntax-rules ()
    ((test-not name expr) (test-not expr))
    ((test-not expr) 
     (parameterize ((current-test-comparator equal?))
       (test #f expr)))))

(define-syntax test-eqv
  (syntax-rules ()
    ((test-eqv res expr) (test #t (eqv? res expr)))
    ((test-eqv name res expr) (test #t (eqv? res expr)))))

(define-syntax test-equal
  (syntax-rules ()
    ((test-equal res expr) (test res expr))
    ((test-equal = expr ...) (if (string? =) (test = #t (equal? expr ...)) (test #t (= expr ...))))))

(define-syntax test-error
  (syntax-rules ()
    ((test-error name expr) 
     (test-error expr))
    ((test-error expr) 
     (guard (condition (else (set! *tests-run* (- *tests-run* 1)) (test 'expr 'expr)))
       (test '<fail> expr)))))

(define-syntax test-output
  (syntax-rules ()
    ((test-output str expr ...) 
     (let ([p (open-output-string)])
       (parameterize ([current-output-port p]) expr ...)
       (test str (get-output-string p))))))

(define (test-begin . name)
  #f)

(define-syntax test-group
  (syntax-rules ()
    ((_ name . body)
     (begin
      (format #t "[Group ~a]\n" name)
      (let () . body)))))

(define (test-end . name)
  (write *tests-passed*)
  (display " out of ")
  (write *tests-run*)
  (display " passed (")
  (if (> *tests-run* 0) (display (flonum->string (* (/ *tests-passed* *tests-run*) 100.0) 2)))
  (display "%)")
  (newline)
  (exit (- *tests-run* *tests-passed*)))

(define (written x) (format #f "~s" x))

(define (call-with-false-on-error thunk)
  (guard (condition (else #f)) (thunk)))
