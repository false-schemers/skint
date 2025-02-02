;;
;; R7RS and Skint tests from various sources
;; 


;; tests from chibi scheme
;; 

(define *tests-run* 0)
(define *tests-passed* 0)

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
         ;(flush-output)
         (cond
          ((equal? res expect)
           (set! *tests-passed* (+ *tests-passed* 1))
           (display " [PASS]\n"))
          (else
           (display " [FAIL]\n")
           (display "********** expected ") (write expect)
           (display " but got ") (write res) (newline))))))))

(define-syntax test-assert
  (syntax-rules ()
    ((test-assert expr) (test #t expr))
    ((test-assert name expr) (test name #t expr))))

(define-syntax test-values
  (syntax-rules ()
    ((test-values vals expr) 
     (test (call-with-values (lambda () vals) list) (call-with-values (lambda () expr) list)))))

(define-syntax test~=
  (syntax-rules ()
    ((test~= val expr) (test (number->string val) (number->string expr)))))


(define (test-begin . name)
  #f)

(define (test-end)
  (write *tests-passed*)
  (display " out of ")
  (write *tests-run*)
  (display " passed (")
  (write (/ (floor (* (/ *tests-passed* *tests-run*) 10000)) 100))
  (display "%)")
  (newline))

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(test-begin "r5rs")

;(define (square x) (* x x)) -- standard in r7rs

(test 1764 (square 42))

(test 4 (square 2))

(test 8 ((lambda (x) (+ x x)) 4))

(test '(3 4 5 6) ((lambda x x) 3 4 5 6))

(test '(5 6) ((lambda (x y . z) z) 3 4 5 6))

(test 'yes (if (> 3 2) 'yes 'no))

(test 'no (if (> 2 3) 'yes 'no))

(test 1 (if (> 3 2) (- 3 2) (+ 3 2)))

(test 'greater (cond ((> 3 2) 'greater) ((< 3 2) 'less)))

(test 'equal (cond ((> 3 3) 'greater) ((< 3 3) 'less) (else 'equal)))

(test 'composite (case (* 2 3) ((2 3 5 7) 'prime) ((1 4 6 8 9) 'composite)))

(test 'consonant
    (case (car '(c d))
      ((a e i o u) 'vowel)
      ((w y) 'semivowel)
      (else 'consonant)))

(test #t (and (= 2 2) (> 2 1)))

(test #f (and (= 2 2) (< 2 1)))

(test '(f g) (and 1 2 'c '(f g)))

(test #t (and))

(test #t (or (= 2 2) (> 2 1)))

(test #t (or (= 2 2) (< 2 1)))

(test '(b c) (or (memq 'b '(a b c)) (/ 3 0)))

(test 6 (let ((x 2) (y 3)) (* x y)))

(test 35 (let ((x 2) (y 3)) (let ((x 7) (z (+ x y))) (* z x))))

(test 70 (let ((x 2) (y 3)) (let* ((x 7) (z (+ x y))) (* z x))))

(test -2 (let ()
           (define x 2)
           (define f (lambda () (- x)))
           (f)))

(define let*-def 1)
(let* () (define let*-def 2) #f)
(test 1 let*-def)

(test '#(0 1 2 3 4)
 (do ((vec (make-vector 5))
      (i 0 (+ i 1)))
     ((= i 5) vec)
   (vector-set! vec i i)))

(test 25
    (let ((x '(1 3 5 7 9)))
      (do ((x x (cdr x))
           (sum 0 (+ sum (car x))))
          ((null? x)
           sum))))

(test '((6 1 3) (-5 -2))
    (let loop ((numbers '(3 -2 1 6 -5)) (nonneg '()) (neg '()))
      (cond
       ((null? numbers)
        (list nonneg neg))
       ((>= (car numbers) 0)
        (loop (cdr numbers) (cons (car numbers) nonneg) neg))
       ((< (car numbers) 0)
        (loop (cdr numbers) nonneg (cons (car numbers) neg))))))

(test '(list 3 4) `(list ,(+ 1 2) 4))

(test '(list a 'a) (let ((name 'a)) `(list ,name ',name)))

(test '(a 3 4 5 6 b)
    `(a ,(+ 1 2) ,@(map abs '(4 -5 6)) b))

(test '(10 5 4 16 9 8)
    `(10 5 ,(expt 2 2) ,@(map (lambda (n) (expt n 2)) '(4 3)) 8))

(test '(a `(b ,(+ 1 2) ,(foo 4 d) e) f)
    `(a `(b ,(+ 1 2) ,(foo ,(+ 1 3) d) e) f))

(test '(a `(b ,x ,'y d) e)
    (let ((name1 'x)
          (name2 'y))
      `(a `(b ,,name1 ,',name2 d) e)))

(test '(list 3 4)
 (quasiquote (list (unquote (+ 1 2)) 4)))

(test #t (eqv? 'a 'a))

(test #f (eqv? 'a 'b))

(test #t (eqv? '() '()))

(test #f (eqv? (cons 1 2) (cons 1 2)))

(test #f (eqv? (lambda () 1) (lambda () 2)))

(test #t (let ((p (lambda (x) x))) (eqv? p p)))

(test #t (eq? 'a 'a))

(test #f (eq? (list 'a) (list 'a)))

(test #t (eq? '() '()))

(test #t (eq? car car))

(test #t (let ((x '(a))) (eq? x x)))

(test #t (let ((p (lambda (x) x))) (eq? p p)))

(test #t (equal? 'a 'a))

(test #t (equal? '(a) '(a)))

(test #t (equal? '(a (b) c) '(a (b) c)))

(test #t (equal? "abc" "abc"))

(test #f (equal? "abc" "abcd"))

(test #f (equal? "a" "b"))

(test #t (equal? 2 2))

(test #f (eqv? 2 2.0))

(test #f (equal? 2.0 2))

(test #t (equal? (make-vector 5 'a) (make-vector 5 'a)))

(test 4 (max 3 4))

(test 4.0 (max 3.9 4))

(test 7 (+ 3 4))

(test 3 (+ 3))

(test 0 (+))

(test 4 (* 4))

(test 1 (*))

(test -1 (- 3 4))

(test -6 (- 3 4 5))

(test -3 (- 3))

(test -1.0 (- 3.0 4))

(test #t (< -1 0))

(test 7 (abs -7))

(test 1 (modulo 13 4))

(test 1 (remainder 13 4))

(test 3 (modulo -13 4))

(test -1 (remainder -13 4))

(test -3 (modulo 13 -4))

(test 0 (modulo 3333333 -3))

(test 1 (remainder 13 -4))

(test -1 (modulo -13 -4))

(test -1 (remainder -13 -4))

(test 4 (gcd 32 -36))

(test 288 (lcm 32 -36))

(test 100 (string->number "100"))

(test 256 (string->number "100" 16))

(test 127 (string->number "177" 8))

(test 5 (string->number "101" 2))

(test 100.0 (string->number "1e2"))

(test "100" (number->string 100))

(test "100" (number->string 256 16))

(test "ff" (number->string 255 16))

(test "177" (number->string 127 8))

(test "101" (number->string 5 2))

(test #f (not 3))

(test #f (not (list 3)))

(test #f (not '()))

(test #f (not (list)))

(test #f (not '()))

(test #f (boolean? 0))

(test #f (boolean? '()))

(test #t (boolean=? #t #t))

(test #f (boolean=? #f #t))

(test #t (boolean=? #f #f #f))

(test #f (boolean=? #f #f #f #t #f))

(test #t (pair? '(a . b)))

(test #t (pair? '(a b c)))

(test '(a) (cons 'a '()))

(test '((a) b c d) (cons '(a) '(b c d)))

(test '("a" b c) (cons "a" '(b c)))

(test '(a . 3) (cons 'a 3))

(test '((a b) . c) (cons '(a b) 'c))

(test 'a (car '(a b c)))

(test '(a) (car '((a) b c d)))

(test 1 (car '(1 . 2)))

(test '(b c d) (cdr '((a) b c d)))

(test 2 (cdr '(1 . 2)))

(test #t (list? '(a b c)))

(test #t (list? '()))

(test #f (list? '(a . b)))

(test #f
    (let ((x (list 'a)))
      (set-cdr! x x)
      (list? x)))

(test '(a 7 c) (list 'a (+ 3 4) 'c))

(test '() (list))

(test 3 (length '(a b c)))

(test 3 (length '(a (b) (c d e))))

(test 0 (length '()))

(test '(x y) (append '(x) '(y)))

(test '(a b c d) (append '(a) '(b c d)))

(test '(a (b) (c)) (append '(a (b)) '((c))))

(test '(a b c . d) (append '(a b) '(c . d)))

(test 'a (append '() 'a))

(test '(c b a) (reverse '(a b c)))

(test '((e (f)) d (b c) a) (reverse '(a (b c) d (e (f)))))

(test 'c (list-ref '(a b c d) 2))

(test '(a b c) (memq 'a '(a b c)))

(test '(b c) (memq 'b '(a b c)))

(test #f (memq 'a '(b c d)))

(test #f (memq (list 'a) '(b (a) c)))

(test '((a) c) (member (list 'a) '(b (a) c)))

(test '(101 102) (memv 101 '(100 101 102)))

(test #f (assq (list 'a) '(((a)) ((b)) ((c)))))

(test '((a)) (assoc (list 'a) '(((a)) ((b)) ((c)))))

(test '(5 7) (assv 5 '((2 3) (5 7) (11 13))))

(let* ((x (list 'a 'b 'c))
       (y x))
  (test '(a b c) (values y))
  (test #t (list? y))
  (set-cdr! x 4)
  (test '(a . 4) (values x))
  (test #t (eqv? x y))
  (test #f (list? y))
  (set-cdr! x x)
  (test #f (list? x)))

(test #t (pair? '(a . b)))
(test #t (pair? '(a b c)))
(test #f (pair? '()))
(test #f (pair? '#(a b)))

(test '(a) (cons 'a '()))
(test '((a) b c d) (cons '(a) '(b c d)))
(test '("a" b c) (cons "a" '(b c)))
(test '(a . 3) (cons 'a 3))
(test '((a b) . c) (cons '(a b) 'c))

(test 'a (car '(a b c)))
(test '(a) (car '((a) b c d)))
(test 1 (car '(1 . 2)))

(test '(b c d) (cdr '((a) b c d)))
(test 2 (cdr '(1 . 2)))

(test #t (list? '(a b c)))
(test #t (list? '()))
(test #f (list? '(a . b)))
(test #f (let ((x (list 'a))) (set-cdr! x x) (list? x)))

(test '() (make-list 0 3))
(test '(3 3) (make-list 2 3))
(test 42 (length (make-list 42)))

(test '(a 7 c) (list 'a (+ 3 4) 'c))
(test '() (list))

(test 3 (length '(a b c)))
(test 3 (length '(a (b) (c d e))))
(test 0 (length '()))

(test '(x y) (append '(x) '(y)))
(test '(a b c d) (append '(a) '(b c d)))
(test '(a (b) (c)) (append '(a (b)) '((c))))

(test '(a b c . d) (append '(a b) '(c . d)))
(test 'a (append '() 'a))

(test '(c b a) (reverse '(a b c)))
(test '((e (f)) d (b c) a) (reverse '(a (b c) d (e (f)))))

(test '(d e) (list-tail '(a b c d e) 3))

(test 'c (list-ref '(a b c d) 2))
(test 'c (list-ref '(a b c d)
          (exact (round 1.8))))

(test '(0 ("Sue" "Sue") "Anna")
    (let ((lst (list 0 '(2 2 2 2) "Anna")))
      (list-set! lst 1 '("Sue" "Sue"))
      lst))

(test '(a b c) (memq 'a '(a b c)))
(test '(b c) (memq 'b '(a b c)))
(test #f (memq 'a '(b c d)))
(test #f (memq (list 'a) '(b (a) c)))
(test '((a) c) (member (list 'a) '(b (a) c)))
(test '("b" "c") (member "B" '("a" "b" "c") string-ci=?))
(test '(101 102) (memv 101 '(100 101 102)))

(let ()
  (define e '((a 1) (b 2) (c 3)))
  (test '(a 1) (assq 'a e))
  (test '(b 2) (assq 'b e))
  (test #f (assq 'd e)))

(test #f (assq (list 'a) '(((a)) ((b)) ((c)))))
(test '((a)) (assoc (list 'a) '(((a)) ((b)) ((c)))))
(test '(2 4) (assoc 2.0 '((1 1) (2 4) (3 9)) =))
(test '(5 7) (assv 5 '((2 3) (5 7) (11 13))))

(test '(1 2 3) (list-copy '(1 2 3)))
(test "foo" (list-copy "foo"))
(test '() (list-copy '()))
(test '(3 . 4) (list-copy '(3 . 4)))
(test '(6 7 8 . 9) (list-copy '(6 7 8 . 9)))
(let* ((l1 '((a b) (c d) e))
       (l2 (list-copy l1)))
  (test l2 '((a b) (c d) e))
  (test #t (eq? (car l1) (car l2)))
  (test #t (eq? (cadr l1) (cadr l2)))
  (test #f (eq? (cdr l1) (cdr l2)))
  (test #f (eq? (cddr l1) (cddr l2))))

(test #t (char? #\a))
(test #f (char? "a"))
(test #f (char? 'a))
(test #f (char? 0))

(test #t (char=? #\a #\a #\a))
(test #f (char=? #\a #\A))
(test #t (char<? #\a #\b #\c))
(test #f (char<? #\a #\a))
(test #f (char<? #\b #\a))
(test #f (char>? #\a #\b))
(test #f (char>? #\a #\a))
(test #t (char>? #\c #\b #\a))
(test #t (char<=? #\a #\b #\b))
(test #t (char<=? #\a #\a))
(test #f (char<=? #\b #\a))
(test #f (char>=? #\a #\b))
(test #t (char>=? #\a #\a))
(test #t (char>=? #\b #\b #\a))

(test #t (char-ci=? #\a #\a))
(test #t (char-ci=? #\a #\A #\a))
(test #f (char-ci=? #\a #\b))
(test #t (char-ci<? #\a #\B #\c))
(test #f (char-ci<? #\A #\a))
(test #f (char-ci<? #\b #\A))
(test #f (char-ci>? #\A #\b))
(test #f (char-ci>? #\a #\A))
(test #t (char-ci>? #\c #\B #\a))
(test #t (char-ci<=? #\a #\B #\b))
(test #t (char-ci<=? #\A #\a))
(test #f (char-ci<=? #\b #\A))
(test #f (char-ci>=? #\A #\b))
(test #t (char-ci>=? #\a #\A))
(test #t (char-ci>=? #\b #\B #\a))

(test #t (char-alphabetic? #\a))
(test #f (char-alphabetic? #\space))
(test #t (char-numeric? #\0))
(test #f (char-numeric? #\.))
(test #f (char-numeric? #\a))
(test #t (char-whitespace? #\space))
(test #t (char-whitespace? #\tab))
(test #t (char-whitespace? #\newline))
(test #f (char-whitespace? #\_))
(test #f (char-whitespace? #\a))
(test #t (char-upper-case? #\A))
(test #f (char-upper-case? #\a))
(test #f (char-upper-case? #\3))
(test #t (char-lower-case? #\a))
(test #f (char-lower-case? #\A))
(test #f (char-lower-case? #\3))

(test #\A (char-upcase #\a))
(test #\A (char-upcase #\A))
(test #\3 (char-upcase #\3))
(test #\a (char-downcase #\A))
(test #\a (char-downcase #\a))
(test #\3 (char-downcase #\3))
(test #\a (char-foldcase #\A))
(test #\a (char-foldcase #\a))
(test #\3 (char-foldcase #\3))

(test #f (digit-value #\space))
(test 0 (digit-value #\0))
(test 3 (digit-value #\3))
(test 7 (digit-value #\7))
(test 9 (digit-value #\9))
(test #f (digit-value #\A))

(test #t (symbol? 'foo))

(test #t (symbol? (car '(a b))))

(test #f (symbol? "bar"))

(test #t (symbol? 'nil))

(test #f (symbol? '()))

(test #t (symbol=? 'a 'a))

(test #f (symbol=? 'a 'b))

(test #t (symbol=? 'b 'b 'b 'b))

(test #f (symbol=? 'b 'b 'b 'b 'a))

(test "flying-fish" (symbol->string 'flying-fish))

(test "Martin" (symbol->string 'Martin))

(test "Malvina" (symbol->string (string->symbol "Malvina")))

(test #t (string? ""))
(test #t (string? " "))
(test #f (string? 'a))
(test #f (string? #\a))

(test 3 (string-length (make-string 3)))
(test "---" (make-string 3 #\-))

(test "" (string))
(test "---" (string #\- #\- #\-))
(test "kitten" (string #\k #\i #\t #\t #\e #\n))

(test 0 (string-length ""))
(test 1 (string-length "a"))
(test 3 (string-length "abc"))

(test #\a (string-ref "abc" 0))
(test #\b (string-ref "abc" 1))
(test #\c (string-ref "abc" 2))

(test "a-c" (let ((str (string #\a #\b #\c))) (string-set! str 1 #\-) str))

;(test (string #\a #\x1F700 #\c)
;    (let ((s (string #\a #\b #\c)))
;      (string-set! s 1 #\x1F700)
;      s))

(test #t (string=? "" ""))
(test #t (string=? "abc" "abc" "abc"))
(test #f (string=? "" "abc"))
(test #f (string=? "abc" "aBc"))

(test #f (string<? "" ""))
(test #f (string<? "abc" "abc"))
(test #t (string<? "abc" "abcd" "acd"))
(test #f (string<? "abcd" "abc"))
(test #t (string<? "abc" "bbc"))

(test #f (string>? "" ""))
(test #f (string>? "abc" "abc"))
(test #f (string>? "abc" "abcd"))
(test #t (string>? "acd" "abcd" "abc"))
(test #f (string>? "abc" "bbc"))

(test #t (string<=? "" ""))
(test #t (string<=? "abc" "abc"))
(test #t (string<=? "abc" "abcd" "abcd"))
(test #f (string<=? "abcd" "abc"))
(test #t (string<=? "abc" "bbc"))

(test #t (string>=? "" ""))
(test #t (string>=? "abc" "abc"))
(test #f (string>=? "abc" "abcd"))
(test #t (string>=? "abcd" "abcd" "abc"))
(test #f (string>=? "abc" "bbc"))

(test #t (string-ci=? "" ""))
(test #t (string-ci=? "abc" "abc"))
(test #f (string-ci=? "" "abc"))
(test #t (string-ci=? "abc" "aBc"))
(test #f (string-ci=? "abc" "aBcD"))
(test #t (string-ci=? "abc" "aBc" "Abc"))

(test #f (string-ci<? "abc" "aBc"))
(test #t (string-ci<? "abc" "aBcD"))
(test #f (string-ci<? "ABCd" "aBc"))
(test #t (string-ci<? "abc" "aBcD" "AbCE"))

(test #f (string-ci>? "abc" "aBc"))
(test #f (string-ci>? "abc" "aBcD"))
(test #t (string-ci>? "ABCd" "aBc"))
(test #t (string-ci>? "ABCE" "ABCd" "aBc"))

(test #t (string-ci<=? "abc" "aBc"))
(test #t (string-ci<=? "abc" "aBcD"))
(test #f (string-ci<=? "ABCd" "aBc"))
(test #t (string-ci<=? "abc" "aBc" "aBcD"))

(test #t (string-ci>=? "abc" "aBc"))
(test #f (string-ci>=? "abc" "aBcD"))
(test #t (string-ci>=? "ABCd" "aBc"))
(test #t (string-ci>=? "ABCd" "aBc" "AbC"))

(test "ABC" (string-upcase "abc"))
(test "ABC" (string-upcase "ABC"))
(test "abc" (string-downcase "abc"))
(test "abc" (string-downcase "ABC"))
(test "abc" (string-foldcase "abc"))
(test "abc" (string-foldcase "ABC"))

(test "" (substring "" 0 0))
(test "" (substring "a" 0 0))
(test "" (substring "abc" 1 1))
(test "ab" (substring "abc" 0 2))
(test "bc" (substring "abc" 1 3))

(test "" (string-append ""))
(test "" (string-append "" ""))
(test "abc" (string-append "" "abc"))
(test "abc" (string-append "abc" ""))
(test "abcde" (string-append "abc" "de"))
(test "abcdef" (string-append "abc" "de" "f"))

(test '() (string->list ""))
(test '(#\a) (string->list "a"))
(test '(#\a #\b #\c) (string->list "abc"))
(test '(#\a #\b #\c) (string->list "abc" 0))
(test '(#\b #\c) (string->list "abc" 1))
(test '(#\b #\c) (string->list "abc" 1 3))

(test "" (list->string '()))
(test "abc" (list->string '(#\a #\b #\c)))

(test "" (string-copy ""))
(test "" (string-copy "" 0))
(test "" (string-copy "" 0 0))
(test "abc" (string-copy "abc"))
(test "abc" (string-copy "abc" 0))
(test "bc" (string-copy "abc" 1))
(test "b" (string-copy "abc" 1 2))
(test "bc" (string-copy "abc" 1 3))

(test "-----"
    (let ((str (make-string 5 #\x))) (string-fill! str #\-) str))
(test "xx---"
    (let ((str (make-string 5 #\x))) (string-fill! str #\- 2) str))
(test "xx-xx"
    (let ((str (make-string 5 #\x))) (string-fill! str #\- 2 3) str))

(test "a12de"
    (let ((str (string-copy "abcde"))) (string-copy! str 1 "12345" 0 2) str))
(test "-----"
    (let ((str (make-string 5 #\x))) (string-copy! str 0 "-----") str))
(test "---xx"
    (let ((str (make-string 5 #\x))) (string-copy! str 0 "-----" 2) str))
(test "xx---"
    (let ((str (make-string 5 #\x))) (string-copy! str 2 "-----" 0 3) str))
(test "xx-xx"
    (let ((str (make-string 5 #\x))) (string-copy! str 2 "-----" 2 3) str))

;; same source and dest
(test "aabde"
    (let ((str (string-copy "abcde"))) (string-copy! str 1 str 0 2) str))
(test "abcab"
    (let ((str (string-copy "abcde"))) (string-copy! str 3 str 0 2) str))

(test #t (string? "a"))

(test #f (string? 'a))

(test 0 (string-length ""))

(test 3 (string-length "abc"))

(test #\a (string-ref "abc" 0))

(test #\c (string-ref "abc" 2))

(test #t (string=? "a" (string #\a)))

(test #f (string=? "a" (string #\b)))

(test #t (string<? "a" "aa"))

(test #f (string<? "aa" "a"))

(test #f (string<? "a" "a"))

(test #t (string<=? "a" "aa"))

(test #t (string<=? "a" "a"))

(test #t (string=? "a" (make-string 1 #\a)))

(test #f (string=? "a" (make-string 1 #\b)))

(test "" (substring "abc" 0 0))

(test "a" (substring "abc" 0 1))

(test "bc" (substring "abc" 1 3))

(test "abc" (string-append "abc" ""))

(test "abc" (string-append "" "abc"))

(test "abc" (string-append "a" "bc"))


(test #t (vector? '#()))
(test #t (vector? '#(1 2 3)))

(test 0 (vector-length (make-vector 0)))
(test 1000 (vector-length (make-vector 1000)))

(test '#(0 (2 2 2 2) "Anna") '#(0 (2 2 2 2) "Anna"))

(test '#(a b c) (vector 'a 'b 'c))

(test 8 (vector-ref '#(1 1 2 3 5 8 13 21) 5))
(test 13 (vector-ref '#(1 1 2 3 5 8 13 21)
            (let ((i (round (* 2 (acos -1)))))
              (if (inexact? i)
                  (exact i)
                  i))))

(test '#(0 ("Sue" "Sue") "Anna") (let ((vec (vector 0 '(2 2 2 2) "Anna")))
  (vector-set! vec 1 '("Sue" "Sue"))
  vec))

(test '(dah dah didah) (vector->list '#(dah dah didah)))
(test '(dah didah) (vector->list '#(dah dah didah) 1))
(test '(dah) (vector->list '#(dah dah didah) 1 2))
(test '#(dididit dah) (list->vector '(dididit dah)))

(test '#() (string->vector ""))
(test '#(#\A #\B #\C) (string->vector "ABC"))
(test '#(#\B #\C) (string->vector "ABC" 1))
(test '#(#\B) (string->vector "ABC" 1 2))

(test "" (vector->string #()))
(test "123" (vector->string '#(#\1 #\2 #\3)))
(test "23" (vector->string '#(#\1 #\2 #\3) 1))
(test "2" (vector->string '#(#\1 #\2 #\3) 1 2))

(test '#() (vector-copy '#()))
(test '#(a b c) (vector-copy '#(a b c)))
(test '#(b c) (vector-copy '#(a b c) 1))
(test '#(b) (vector-copy '#(a b c) 1 2))

(test '#() (vector-append))
(test '#() (vector-append '#()))
(test '#() (vector-append '#() '#()))
(test '#(a b c) (vector-append '#() '#(a b c)))
(test '#(a b c) (vector-append '#(a b c) '#()))
(test '#(a b c d e) (vector-append '#(a b c) '#(d e)))
(test '#(a b c d e f) (vector-append '#(a b c) '#(d e) '#(f)))

(test '#(1 2 smash smash 5)
    (let ((vec (vector 1 2 3 4 5))) (vector-fill! vec 'smash 2 4) vec))
(test '#(x x x x x)
    (let ((vec (vector 1 2 3 4 5))) (vector-fill! vec 'x) vec))
(test '#(1 2 x x x)
    (let ((vec (vector 1 2 3 4 5))) (vector-fill! vec 'x 2) vec))
(test '#(1 2 x 4 5)
    (let ((vec (vector 1 2 3 4 5))) (vector-fill! vec 'x 2 3) vec))

(test '#(1 a b 4 5)
    (let ((vec (vector 1 2 3 4 5))) (vector-copy! vec 1 #(a b c d e) 0 2) vec))
(test '#(a b c d e)
    (let ((vec (vector 1 2 3 4 5))) (vector-copy! vec 0 #(a b c d e)) vec))
(test '#(c d e 4 5)
    (let ((vec (vector 1 2 3 4 5))) (vector-copy! vec 0 #(a b c d e) 2) vec))
(test '#(1 2 a b c)
    (let ((vec (vector 1 2 3 4 5))) (vector-copy! vec 2 #(a b c d e) 0 3) vec))
(test '#(1 2 c 4 5)
    (let ((vec (vector 1 2 3 4 5))) (vector-copy! vec 2 #(a b c d e) 2 3) vec))

;; same source and dest
(test '#(1 1 2 4 5)
    (let ((vec (vector 1 2 3 4 5))) (vector-copy! vec 1 vec 0 2) vec))
(test '#(1 2 3 1 2)
    (let ((vec (vector 1 2 3 4 5))) (vector-copy! vec 3 vec 0 2) vec))


(test '#(0 ("Sue" "Sue") "Anna")
 (let ((vec (vector 0 '(2 2 2 2) "Anna")))
   (vector-set! vec 1 '("Sue" "Sue"))
   vec))

(test '(dah dah didah) (vector->list '#(dah dah didah)))

(test '#(dididit dah) (list->vector '(dididit dah)))

(test #t (procedure? car))

(test #f (procedure? 'car))

(test #t (procedure? (lambda (x) (* x x))))

(test #f (procedure? '(lambda (x) (* x x))))

(test #t (call-with-current-continuation procedure?))

(test 7 (call-with-current-continuation (lambda (k) (+ 2 5))))

(test 3 (call-with-current-continuation (lambda (k) (+ 2 5 (k 3)))))

(test 7 (apply + (list 3 4)))

(test '(b e h) (map cadr '((a b) (d e) (g h))))

(test '(1 4 27 256 3125) (map (lambda (n) (expt n n)) '(1 2 3 4 5)))

(test '(5 7 9) (map + '(1 2 3) '(4 5 6)))

(test '#(0 1 4 9 16)
    (let ((v (make-vector 5)))
      (for-each
       (lambda (i) (vector-set! v i (* i i)))
       '(0 1 2 3 4))
      v))

(test 3 (force (delay (+ 1 2))))

(test '(3 3) (let ((p (delay (+ 1 2)))) (list (force p) (force p))))

(test 'ok (let ((else 1)) (cond (else 'ok) (#t 'bad))))

(test 'ok (let ((=> 1)) (cond (#t => 'ok))))

(test '(,foo) (let ((unquote 1)) `(,foo)))

(test '(,@foo) (let ((unquote-splicing 1)) `(,@foo)))

(test '(list 3 4) `(list ,(+ 1 2) 4))

(let ((name 'a)) (test '(list a (quote a)) `(list ,name ',name)))

(test '(a 3 4 5 6 b) `(a ,(+ 1 2) ,@(map abs '(4 -5 6)) b))

(test '#(10 5 4 16 9 8)
    `#(10 5 ,(square 2) ,@(map square '(4 3)) 8))

(test '(a `(b ,(+ 1 2) ,(foo 4 d) e) f)
    `(a `(b ,(+ 1 2) ,(foo ,(+ 1 3) d) e) f) )

(let ((name1 'x)
      (name2 'y))
   (test '(a `(b ,x ,'y d) e) `(a `(b ,,name1 ,',name2 d) e)))

(test '(list 3 4) (quasiquote (list (unquote (+ 1 2)) 4)) )

(test `(list ,(+ 1 2) 4) (quasiquote (list (unquote (+ 1 2)) 4)))

(test 'greater (when (> 3 2) 'greater))
(test '(greater) (let ((x '())) (when (> 3 2) (set! x (cons 'greater x)) x)))

(test 'less (unless (< 3 2) 'less))
(test '(less) (let ((x '())) (unless (< 3 2) (set! x (cons 'less x)) x)))

(define plus
  (case-lambda 
   (() 0)
   ((x) x)
   ((x y) (+ x y))
   ((x y z) (+ (+ x y) z))
   (args (apply + args))))

(test 0 (plus))

(test 1 (plus 1))

(test 3 (plus 1 2))

(test 6 (plus 1 2 3))

(test 10 (plus 1 2 3 4))

(define mult
  (case-lambda 
   (() 1)
   ((x) x)
   ((x y) (* x y))
   ((x y . z) (apply mult (* x y) z))))

(test 1 (mult))

(test 1 (mult 1))

(test 2 (mult 1 2))

(test 6 (mult 1 2 3))

(test 24 (mult 1 2 3 4))

(test 'now (let-syntax
               ((when (syntax-rules ()
                        ((when test stmt1 stmt2 ...)
                         (if test
                             (begin stmt1
                                    stmt2 ...))))))
             (let ((if #t))
               (when if (set! if 'now))
               if)))

(test 'outer (let ((x 'outer))
  (let-syntax ((m (syntax-rules () ((m) x))))
    (let ((x 'inner))
      (m)))))

(test 7 (letrec-syntax
  ((my-or (syntax-rules ()
            ((my-or) #f)
            ((my-or e) e)
            ((my-or e1 e2 ...)
             (let ((temp e1))
               (if temp
                   temp
                   (my-or e2 ...)))))))
  (let ((x #f)
        (y 7)
        (temp 8)
        (let odd?)
        (if even?))
    (my-or x
           (let temp)
           (if y)
           y))))

(define-syntax be-like-begin1
  (syntax-rules ()
    ((be-like-begin1 name)
     (define-syntax name
       (syntax-rules ()
         ((name expr (... ...))
          (begin expr (... ...))))))))
(be-like-begin1 sequence1)
(test 3 (sequence1 0 1 2 3))

(define-syntax be-like-begin2
  (syntax-rules ()
    ((be-like-begin2 name)
     (define-syntax name
       (... (syntax-rules ()
              ((name expr ...)
               (begin expr ...))))))))
(be-like-begin2 sequence2)
(test 4 (sequence2 1 2 3 4))

(define-syntax be-like-begin3
  (syntax-rules ()
    ((be-like-begin3 name)
     (define-syntax name
       (syntax-rules dots ()
         ((name expr dots)
          (begin expr dots)))))))
(be-like-begin3 sequence3)
(test 5 (sequence3 2 3 4 5))

(define-syntax jabberwocky
  (syntax-rules ()
    ((_ hatter)
     (begin
       (define march-hare 42)
       (define-syntax hatter
         (syntax-rules ()
           ((_) march-hare)))))))
(jabberwocky mad-hatter)
(test 42 (mad-hatter))

(test 'ok (let ((=> #f)) (cond (#t => 'ok))))

(let ()
  (define x 1)
  (let-syntax ()
    (define x 2)
    #f)
  (test 1 x))

; this one is commented out because it breaks R7RS identifier
; discovery protocol
#;(let ()
 (define-syntax foo
   (syntax-rules ()
     ((foo bar y)
      (define-syntax bar
        (syntax-rules ()
          ((bar x) 'y))))))
 (foo bar x)
 (test 'x (bar 1)))

(begin
  (define-syntax ffoo
    (syntax-rules ()
      ((ffoo ff)
       (begin
         (define (ff x)
           (gg x))
         (define (gg x)
           (* x x))))))
  (ffoo ff)
  (test 100 (ff 10)))

(let-syntax ((vector-lit
               (syntax-rules ()
                 ((vector-lit)
                  '#(b)))))
  (test '#(b) (vector-lit)))

(let ()
  ;; forward hygienic refs
  (define-syntax foo399
    (syntax-rules () ((foo399) (bar399))))
  (define (quux399)
    (foo399))
  (define (bar399)
    42)
  (test 42 (quux399)))

(let-syntax
    ((m (syntax-rules ()
          ((m x) (let-syntax
                     ((n (syntax-rules (k)
                           ((n x) 'bound-identifier=?)
                           ((n y) 'free-identifier=?))))
                   (n z))))))
  (test 'bound-identifier=? (m k)))


(test 'ok
    (let ((... 2))
      (let-syntax ((s (syntax-rules ()
                        ((_ x ...) 'bad)
                        ((_ . r) 'ok))))
        (s a b c))))

; esl+ -- test if we can 'zip' lists coming from separate enumerations
(test '(10 20) (let-syntax ((zlet (syntax-rules () ((_ (i ...) (v ...) . b) (let ((i v) ...) . b)))))
                 (zlet (a b) (10 20) (list a b))))

; esl+ -- test for non-final ... pattern
(test '(2 3 4 5 6) (let-syntax ((gotmid (syntax-rules () ((_ a b ... c) (list b ...))))) (gotmid 1 2 3 4 5 6 7)))

; esl+ -- test for greedy dot-final ... pattern
(test '(2 3 4 5 6 7 ()) (let-syntax ((greedy (syntax-rules () ((_ a b ... . c) (list b ... c))))) (greedy 1 2 3 4 5 6 7)))

; esl+ -- test for greedy dot-final ... pattern
(test '(2 3 4 5 6 7) (let-syntax ((greed.y (syntax-rules () ((_ a b ... . c) (list b ... c))))) (greed.y 1 2 3 4 5 6 . 7)))

; esl+ -- nested underscores
(test '(_) (let-syntax ((foo (syntax-rules () ((_)
              (let-syntax ((bar (syntax-rules () ((_) (list '_)))))
                (bar))))))
              (foo)))

; esl+ -- r7rs rule head pre-match test
(test '(x 123) (let-syntax ([ttt (syntax-rules () [(x y) '(x y)])]) (ttt 123)))

; esl+ -- skint head pattern escape test
(test '(ttt 123) (let-syntax ([ttt (syntax-rules () [((... x) y) '(x y)])]) (ttt 123)))

; esl+ -- similar to above -- but with template escape for pattern escape!
(test '(ttt 123) 
  ((syntax-rules () [(_) (let-syntax ([ttt (syntax-rules () [(((... ...) x) y) '(x y)])]) (ttt 123))])))

; esl+ -- generated globals
(define-syntax foo1
  (syntax-rules ()
    ((_ x) 
     (begin ; all returned lists should be equal!
       (define x (lambda () (list x y z))) 
       (define y (lambda () (list x y z)))
       (define-syntax define-thunk (syntax-rules () ((_ v e) (define v (lambda () e)))))
       (define-thunk z (list x y z))))))
(foo1 bar)
(test #t (let ((l3 (bar))) (and (equal? (bar) ((cadr l3))) (equal? (bar) ((caddr l3))))))

; esl+ -- boxen
(define-syntax unbox-rebox
  (syntax-rules ()
    ((_ #&x ...) '(x ...))
    ((_ x ...) '(#&x ...))))
(test '(#&1 #&2 #&3 #&4 #&5) (unbox-rebox 1 2 3 4 5))
(test '(1 2 3 4 5) (unbox-rebox #&1 #&2 #&3 #&4 #&5))

; esl+ -- simple pattern escape
(let ()
  (define-syntax underscored
    (syntax-rules ()
      ((_ (... _) (... ...)) (list (... ...) (... _)))))
  (test '(2 1) (underscored 1 2)))

; esl+ -- named pattern escapes
(define-syntax wrap-by-type
  (syntax-rules ()
    ((_ (... string? x)) '#&x)
    ((_ (... number? x)) '#(x))
    ((_ x) 'x)))
(test '(#(42) #&"yes" #\c)
  (list (wrap-by-type 42) (wrap-by-type "yes") (wrap-by-type #\c))) 

; esl+ -- named template escape: string->id 
(let ()
  (define-syntax pi-e-example 
    (syntax-rules () 
      [(_)  
       (let ([(... string->id "pi" e) 3.14] [e 2.72])
         (+ pi (... string->id "e" pi)))]))
  (test 5.86 (pi-e-example)))

; esl+ -- simple pattern escape, string->id 
(let-syntax 
  ([define-math-constants
    (syntax-rules () 
      [((... ref-id))
       (begin (define (... string->id "pi" ref-id) 3.14)
              (define (... string->id "e" ref-id) 2.72))])])
  (define-math-constants)
  (test 5.86 (+ pi e)))


; esl+ -- named template escape: string->id
(define-syntax loop-until-break
  (syntax-rules ()
    [((... ref-id) e ...)
     (call/cc 
       (lambda ((... string->id "break" ref-id))
         (let loop () e ... (loop))))]))
(test '6
  (let ((n 10) (steps 0) (break write))
    (loop-until-break
      (when (= n 4) (break steps))
      (set! n (- n 1))
      (set! steps (+ steps 1)))))

; esl+ -- named template escape: string->list
(let ()
  (define-syntax unstringed
    (syntax-rules ()
      ((_ (... string? s)) '(... string->list s))))
  (test '(#\F #\o #\o) (unstringed "Foo")))


; these 2 tests presume that let-syntax is splicing, which is not a requirenment in r7rs
#;(test 'ok (let ()
            (let-syntax ()
              (define internal-def 'ok))
            internal-def))

#;(test 'ok (let ()
            (letrec-syntax ()
              (define internal-def 'ok))
            internal-def))

(test '(2 1)
    ((lambda () (let ((x 1)) (let ((y x)) (set! x 2) (list x y))))))

(test '(2 2)
    ((lambda () (let ((x 1)) (set! x 2) (let ((y x)) (list x y))))))

(test '(1 2)
    ((lambda () (let ((x 1)) (let ((y x)) (set! y 2) (list x y))))))

(test '(2 3)
    ((lambda () (let ((x 1)) (let ((y x)) (set! x 2) (set! y 3) (list x y))))))

(test '(a b c)
    (let* ((path '())
           (add (lambda (s) (set! path (cons s path)))))
      (dynamic-wind (lambda () (add 'a)) (lambda () (add 'b)) (lambda () (add 'c)))
      (reverse path)))

(test '(connect talk1 disconnect connect talk2 disconnect)
    (let ((path '())
          (c #f))
      (let ((add (lambda (s)
                   (set! path (cons s path)))))
        (dynamic-wind
            (lambda () (add 'connect))
            (lambda ()
              (add (call-with-current-continuation
                    (lambda (c0)
                      (set! c c0)
                      'talk1))))
            (lambda () (add 'disconnect)))
        (if (< (length path) 4)
            (c 'talk2)
            (reverse path)))))

(test 2 (let-syntax
            ((foo (syntax-rules ::: ()
                    ((foo ... args :::)
                     (args ::: ...)))))
          (foo 3 - 5)))

(test '(5 4 1 2 3)
    (let-syntax
        ((foo (syntax-rules ()
                ((foo args ... penultimate ultimate)
                 (list ultimate penultimate args ...)))))
      (foo 1 2 3 4 5)))

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(define-syntax M
  (syntax-rules E1 ()
    ((M x E1) (quote (x E1)))))

(test '(1 2 3) (M 1 2 3))

(let ((E2 2))
  (define-syntax N
    (syntax-rules E2 ()
      ((N y E2) (quote (y E2)))))
  (test '(1 2 3) (N 1 2 3)))

;(define-syntax ell
;  (syntax-rules ()
;   ((ell body)
;    (define-syntax emm
;      (syntax-rules ...1 ()
;        ((emm) body))))))

;(ell
; (define-syntax enn
;   (syntax-rules ...1 () ((enn args ...1) (quote (args ...1))))))

(let ((... 'local))
  (define-syntax asd
    (syntax-rules ()
      ((asd x ...) (quote (... x)))))
  (test '(2 1) (asd 1 2)))


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; r7rs extras
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;(test #t (complex? 3+4i))
(test #t (complex? 3))
(test #t (real? 3))
;(test #t (real? -2.5+0i))
;(test #f (real? -2.5+0.0i))
(test #t (real? #e1e5))
;(test #t (real? #e1e10)) ;this one can't be made exact because 10^10 is outside fixnum range
(test #t (real? +inf.0))
(test #f (rational? -inf.0))
;(test #t (rational? 6/10))
;(test #t (rational? 6/3))
;(test #t (integer? 3+0i))
(test #t (integer? 3.0))
;(test #t (integer? 8/4))  ;this one should try to produce fixnum, otherwise flonum as if it were (/ 8 4)

(test #f (exact? 3.0))
(test #t (exact? #e3.0))
(test #t (inexact? 3.))

(test #t (exact-integer? 32))
(test #f (exact-integer? 32.0))
;(test #f (exact-integer? 32/5))

(test #t (finite? 3))
(test #f (finite? +inf.0))
;(test #f (finite? 3.0+inf.0i))

(test #f (infinite? 3))
(test #t (infinite? +inf.0))
(test #f (infinite? +nan.0))
;(test #t (infinite? 3.0+inf.0i))

(test #t (nan? +nan.0))
(test #f (nan? 32))
;; (test #t (nan? +nan.0+5.0i))
;(test #f (nan? 1+2i))

;(test #t (= 1 1.0 1.0+0.0i))
;(test #f (= 1.0 1.0+1.0i))
(test #t (< 1 2 3))
(test #f (< 1 1 2))
(test #t (> 3.0 2.0 1.0))
(test #f (> -3.0 2.0 1.0))
(test #t (<= 1 1 2))
(test #f (<= 1 2 1))
(test #t (>= 2 1 1))
(test #f (>= 1 2 1))
(test '(#t #f) (list (<= 1 1 2) (<= 2 1 3)))


(define single-float-epsilon
  (do ((eps 1.0 (* eps 2.0)))
      ((= eps (+ eps 1.0)) eps)))

; this one overflows fixnum range on (exact a)
#;(let* ((a (/ 10.0 single-float-epsilon))
       (j (exact a)))
  (test #t (if (and (<= a j) (< j (+ j 1)))
               (not (<= (+ j 1) a))
               #t)))

(test #t (zero? 0))
(test #t (zero? 0.0))
;(test #t (zero? 0.0+0.0i))
(test #f (zero? 1))
(test #f (zero? -1))

(test #f (positive? 0))
(test #f (positive? 0.0))
(test #t (positive? 1))
(test #t (positive? 1.0))
(test #f (positive? -1))
(test #f (positive? -1.0))
(test #t (positive? +inf.0))
(test #f (positive? -inf.0))

(test #f (negative? 0))
(test #f (negative? 0.0))
(test #f (negative? 1))
(test #f (negative? 1.0))
(test #t (negative? -1))
(test #t (negative? -1.0))
(test #f (negative? +inf.0))
(test #t (negative? -inf.0))

(test #f (odd? 0))
(test #t (odd? 1))
(test #t (odd? -1))
(test #f (odd? 102))

(test #t (even? 0))
(test #f (even? 1))
(test #t (even? -2))
(test #t (even? 102))

(test 3 (max 3))
(test 4 (max 3 4))
(test 4.0 (max 3.9 4))
(test 5.0 (max 5 3.9 4))
(test +inf.0 (max 100 +inf.0))
(test 3 (min 3))
(test 3 (min 3 4))
(test 3.0 (min 3 3.1))
(test -inf.0 (min -inf.0 -100))

(test 7 (+ 3 4))
(test 3 (+ 3))
(test 0 (+))
(test 4 (* 4))
(test 1 (*))

(test -1 (- 3 4))
(test -6 (- 3 4 5))
(test -3 (- 3))
;(test 3/20 (/ 3 4 5))
;(test 1/3 (/ 3))

(test 7 (abs -7))
(test 7 (abs 7))

(test-values (values 2 1) (floor/ 5 2))
(test-values (values -3 1) (floor/ -5 2))
(test-values (values -3 -1) (floor/ 5 -2))
(test-values (values 2 -1) (floor/ -5 -2))
(test-values (values 2 1) (truncate/ 5 2))
(test-values (values -2 -1) (truncate/ -5 2))
(test-values (values -2 1) (truncate/ 5 -2))
(test-values (values 2 -1) (truncate/ -5 -2))
(test-values (values 2.0 -1.0) (truncate/ -5.0 -2))
(test-values (values 1111111 0) (truncate/ 3333333 3))
(test-values (values 1111111.0 0.0) (truncate/ 3333333 3.0))
(test-values (values -1111111 0) (truncate/ 3333333 -3))
(test-values (values -1111111.0 0.0) (truncate/ 3333333 -3.0))
(test-values (values -1111111 0) (truncate/ -3333333 3))
(test-values (values -1111111.0 0.0) (truncate/ -3333333 3.0))
(test-values (values 1111111 0) (truncate/ -3333333 -3))
(test-values (values 1111111.0 0.0) (truncate/ -3333333 -3.0))

(test 1 (modulo 13 4))
(test 1 (remainder 13 4))

(test 3 (modulo -13 4))
(test -1 (remainder -13 4))

(test -3 (modulo 13 -4))
(test 1 (remainder 13 -4))

(test -1 (modulo -13 -4))
(test -1 (remainder -13 -4))

(test -1.0 (remainder -13 -4.0))

(test 4 (gcd 32 -36))
(test 0 (gcd))
(test 288 (lcm 32 -36))
(test 288.0 (lcm 32.0 -36))
(test 1 (lcm))

;(test 3 (numerator (/ 6 4)))
;(test 2 (denominator (/ 6 4)))
;(test 2.0 (denominator (inexact (/ 6 4))))
;(test 11.0 (numerator 5.5))
;(test 2.0 (denominator 5.5))
;(test 5.0 (numerator 5.0))
;(test 1.0 (denominator 5.0))

(test -5.0 (floor -4.3))
(test -4.0 (ceiling -4.3))
(test -4.0 (truncate -4.3))
(test -4.0 (round -4.3))

(test 3.0 (floor 3.5))
(test 4.0 (ceiling 3.5))
(test 3.0 (truncate 3.5))
(test 4.0 (round 3.5))

;(test 4 (round 7/2))
;(test 7 (round 7))
;(test 1 (round 7/10))
;(test -4 (round -7/2))
;(test -7 (round -7))
;(test -1 (round -7/10))

;(test 1/3 (rationalize (exact .3) 1/10))
;(test #i1/3 (rationalize .3 1/10))

(test 1.0 (inexact (exp 0))) ;; may return exact number
(test~= 20.08553692318767 (exp 3))

(test 0.0 (inexact (log 1))) ;; may return exact number
(test 1.0 (log (exp 1)))
(test 42.0 (log (exp 42)))
(test 2.0 (log 100 10)) 
(test 12.0 (log 4096 2))

(test 0.0 (inexact (sin 0))) ;; may return exact number
(test 1.0 (sin 1.5707963267949))
(test 1.0 (inexact (cos 0))) ;; may return exact number
(test -1.0 (cos 3.14159265358979))
(test 0.0 (inexact (tan 0))) ;; may return exact number
(test~= 1.557407724654902 (tan 1))

(test 0.0 (inexact (asin 0))) ;; may return exact number
(test~= 1.570796326794897 (asin 1))
(test 0.0 (inexact (acos 1))) ;; may return exact number
(test~= 3.141592653589793 (acos -1))

;; (test 0.0-0.0i (asin 0+0.0i))
;; (test 1.5707963267948966+0.0i (acos 0+0.0i))

(test 0.0 (atan 0.0 1.0))
(test -0.0 (atan -0.0 1.0))
(test~= 0.7853981633974483 (atan 1.0 1.0))
(test~= 1.570796326794897 (atan 1.0 0.0))
(test~= 2.356194490192345 (atan 1.0 -1.0))
(test~= 3.141592653589793 (atan 0.0 -1.0))
(test~= -3.141592653589793 (atan -0.0 -1.0)) ;
(test~= -2.356194490192345 (atan -1.0 -1.0))
(test~= -1.570796326794897 (atan -1.0 0.0))
(test~= -0.7853981633974483 (atan -1.0 1.0))
;; (test undefined (atan 0.0 0.0))

(test 1764 (square 42))
(test 4 (square 2))

(test 3.0 (inexact (sqrt 9)))
(test~= 1.414213562373095 (sqrt 2))
;(test 0.0+1.0i (inexact (sqrt -1)))

(test '(0 0) (call-with-values (lambda () (exact-integer-sqrt 0)) list))
(test '(1 0) (call-with-values (lambda () (exact-integer-sqrt 1)) list))
(test '(1 1) (call-with-values (lambda () (exact-integer-sqrt 2)) list))
(test '(1 2) (call-with-values (lambda () (exact-integer-sqrt 3)) list))
(test '(2 0) (call-with-values (lambda () (exact-integer-sqrt 4)) list))
(test '(2 1) (call-with-values (lambda () (exact-integer-sqrt 5)) list))
(test '(2 1) (call-with-values (lambda () (exact-integer-sqrt 5)) list))
(test '(6 6) (call-with-values (lambda () (exact-integer-sqrt 42)) list))
(test '(2896 1791) (call-with-values (lambda () (exact-integer-sqrt 8388607)) list))

(test 27 (expt 3 3))
(test 1 (expt 0 0))
(test 0 (expt 0 1))
(test 1.0 (expt 0.0 0))
(test 0.0 (expt 0 1.0))

;(test 1+2i (make-rectangular 1 2))

;(test 0.54030230586814+0.841470984807897i (make-polar 1 1))

;(test 1 (real-part 1+2i))

;(test 2 (imag-part 1+2i))

;(test 2.23606797749979 (magnitude 1+2i))

;(test 1.10714871779409 (angle 1+2i))

(test 1.0 (inexact 1))
(test #t (inexact? (inexact 1)))
(test 1 (exact 1.0))
(test #t (exact? (exact 1.0)))

(test 100 (string->number "100"))
(test 256 (string->number "100" 16))
(test 100.0 (string->number "1e2"))


(define radix
  (make-parameter
   10
   (lambda (x)
     (if (and (integer? x) (<= 2 x 16))
         x
         (error "invalid radix")))))
(define (f n) (number->string n (radix)))
(test "12" (f 12))
(test "1100" (parameterize ((radix 2))
  (f 12)))
(test "12" (f 12))

(test '(x y x y) (let ((a 'a) (b 'b) (x 'x) (y 'y))
  (let*-values (((a b) (values x y))
                ((x y) (values a b)))
    (list a b x y))))

(test 'ok (let-values () 'ok))

(test 1 (let ((x 1))
	  (let*-values ()
	    (define x 2)
	    #f)
	  x))

(test 'ok
    (let ()
      (define-values () (values))
      'ok))
(test 1
    (let ()
      (define-values (x) (values 1))
      x))
(test 3
    (let ()
      (define-values x (values 1 2))
      (apply + x)))
(test 3
    (let ()
      (define-values (x y) (values 1 2))
      (+ x y)))
(test 6
    (let ()
      (define-values (x y z) (values 1 2 3))
      (+ x y z)))
(test 10
    (let ()
      (define-values (x y . z) (values 1 2 3 4))
      (+ x y (car z) (cadr z))))

(test 5
    (call-with-values (lambda () (values 4 5))
      (lambda (a b) b)))

(test -1 (call-with-values * -))

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(test 65
    (with-exception-handler
     (lambda (con) 42)
     (lambda ()
       (+ (raise-continuable "should be a number")
          23))))

(test #t
    (error-object? (guard (exn (else exn)) (error "BOOM!" 1 2 3))))
(test "BOOM!"
    (error-object-message (guard (exn (else exn)) (error "BOOM!" 1 2 3))))
(test '(1 2 3)
    (error-object-irritants (guard (exn (else exn)) (error "BOOM!" 1 2 3))))

(test #f
    (file-error? (guard (exn (else exn)) (error "BOOM!"))))
(test #t
    (file-error? (guard (exn (else exn)) (open-input-file " no such file "))))

(test #f
    (read-error? (guard (exn (else exn)) (error "BOOM!"))))
(test #t
    (read-error? (guard (exn (else exn)) (read (open-input-string ")")))))
(test #t
    (read-error? (guard (exn (else exn)) (read (open-input-string "\"")))))

(define something-went-wrong #f)
(define (test-exception-handler-1 v)
  (call-with-current-continuation
   (lambda (k)
     (with-exception-handler
      (lambda (x)
        (set! something-went-wrong (list "condition: " x))
        (k 'exception))
      (lambda ()
        (+ 1 (if (> v 0) (+ v 100) (raise 'an-error))))))))
(test 106 (test-exception-handler-1 5))
(test #f something-went-wrong)
(test 'exception (test-exception-handler-1 -1))
(test '("condition: " an-error) something-went-wrong)

(set! something-went-wrong #f)
(define (test-exception-handler-2 v)
  (guard (ex (else 'caught-another-exception))
    (with-exception-handler
     (lambda (x)
       (set! something-went-wrong #t)
       (list "exception:" x))
     (lambda ()
       (+ 1 (if (> v 0) (+ v 100) (raise 'an-error)))))))
(test 106 (test-exception-handler-2 5))
(test #f something-went-wrong)
(test 'caught-another-exception (test-exception-handler-2 -1))
(test #t something-went-wrong)

;; Based on an example from R6RS-lib section 7.1 Exceptions.
;; R7RS section 6.11 Exceptions has a simplified version.
(let* ((out (open-output-string))
       (value (with-exception-handler
               (lambda (con)
                 (cond
                  ((not (list? con))
                   (raise con))
                  ((list? con)
                   (display (car con) out))
                  (else
                   (display "a warning has been issued" out)))
                 42)
               (lambda ()
                 (+ (raise-continuable
                     (list "should be a number"))
                    23)))))
  (test "should be a number" (get-output-string out))
  (test 65 value))

;; From SRFI-34 "Examples" section - #3
(define (test-exception-handler-3 v out)
  (guard (condition
          (else
           (display "condition: " out)
           (write condition out)
           (display #\! out)
           'exception))
         (+ 1 (if (= v 0) (raise 'an-error) (/ 10 v)))))
(let* ((out (open-output-string))
       (value (test-exception-handler-3 0 out)))
  (test 'exception value)
  (test "condition: an-error!" (get-output-string out)))

(define (test-exception-handler-4 v out)
  (call-with-current-continuation
   (lambda (k)
     (with-exception-handler
      (lambda (x)
        (display "reraised " out)
        (write x out) (display #\! out)
        (k 'zero))
      (lambda ()
        (guard (condition
                ((positive? condition)
                 'positive)
                ((negative? condition)
                 'negative))
          (raise v)))))))

;; From SRFI-34 "Examples" section - #5
(let* ((out (open-output-string))
       (value (test-exception-handler-4 1 out)))
  (test "" (get-output-string out))
  (test 'positive value))
;; From SRFI-34 "Examples" section - #6
(let* ((out (open-output-string))
       (value (test-exception-handler-4 -1 out)))
  (test "" (get-output-string out))
  (test 'negative value))
;; From SRFI-34 "Examples" section - #7
(let* ((out (open-output-string))
       (value (test-exception-handler-4 0 out)))
  (test "reraised 0!" (get-output-string out))
  (test 'zero value))

;; From SRFI-34 "Examples" section - #8
(test 42
    (guard (condition
            ((assq 'a condition) => cdr)
            ((assq 'b condition)))
      (raise (list (cons 'a 42)))))

;; From SRFI-34 "Examples" section - #9
(test '(b . 23)
    (guard (condition
            ((assq 'a condition) => cdr)
            ((assq 'b condition)))
      (raise (list (cons 'b 23)))))

(test 'caught-d
    (guard (condition
            ((assq 'c condition) 'caught-c)
            ((assq 'd condition) 'caught-d))
      (list
       (sqrt 8)
       (guard (condition
               ((assq 'a condition) => cdr)
               ((assq 'b condition)))
         (raise (list (cons 'd 24)))))))


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(test-begin "6.9 Bytevectors")

(test #t (bytevector? #u8()))
(test #t (bytevector? #u8(0 1 2)))
(test #f (bytevector? #()))
(test #f (bytevector? #(0 1 2)))
(test #f (bytevector? '()))
(test #t (bytevector? (make-bytevector 0)))

(test 0 (bytevector-length (make-bytevector 0)))
(test 1024 (bytevector-length (make-bytevector 1024)))
(test 1024 (bytevector-length (make-bytevector 1024 255)))

(test 3 (bytevector-length (bytevector 0 1 2)))

(test 0 (bytevector-u8-ref (bytevector 0 1 2) 0))
(test 1 (bytevector-u8-ref (bytevector 0 1 2) 1))
(test 2 (bytevector-u8-ref (bytevector 0 1 2) 2))

(test #u8(0 255 2)
    (let ((bv (bytevector 0 1 2))) (bytevector-u8-set! bv 1 255) bv))

;; NB: bytevector->list and list->bytevector are NOT in r7rs!
(test '() (bytevector->list #u8()))
(test '(0 1 2) (bytevector->list #u8(0 1 2)))
(test '(0 1 2) (bytevector->list #u8(0 1 2) 0))
(test '(0 1 2) (bytevector->list #u8(0 1 2) 0 3))
(test '(1 2) (bytevector->list #u8(0 1 2) 1))
(test '(1) (bytevector->list #u8(0 1 2) 1 2))
(test '() (bytevector->list #u8(0 1 2) 2 2))
(test #u8() (list->bytevector '()))
(test #u8(24 42) (list->bytevector '(24 42)))

(test #u8() (bytevector-copy #u8()))
(test #u8(0 1 2) (bytevector-copy #u8(0 1 2)))
(test #u8(1 2) (bytevector-copy #u8(0 1 2) 1))
(test #u8(1) (bytevector-copy #u8(0 1 2) 1 2))

(test #u8(1 6 7 4 5)
    (let ((bv (bytevector 1 2 3 4 5)))
      (bytevector-copy! bv 1 #u8(6 7 8 9 10) 0 2)
      bv))
(test #u8(6 7 8 9 10)
    (let ((bv (bytevector 1 2 3 4 5)))
      (bytevector-copy! bv 0 #u8(6 7 8 9 10))
      bv))
(test #u8(8 9 10 4 5)
    (let ((bv (bytevector 1 2 3 4 5)))
      (bytevector-copy! bv 0 #u8(6 7 8 9 10) 2)
      bv))
(test #u8(1 2 6 7 8)
    (let ((bv (bytevector 1 2 3 4 5)))
      (bytevector-copy! bv 2 #u8(6 7 8 9 10) 0 3)
      bv))
(test #u8(1 2 8 4 5)
    (let ((bv (bytevector 1 2 3 4 5)))
      (bytevector-copy! bv 2 #u8(6 7 8 9 10) 2 3)
      bv))

;; same source and dest
(test #u8(1 1 2 4 5)
    (let ((bv (bytevector 1 2 3 4 5)))
      (bytevector-copy! bv 1 bv 0 2)
      bv))
(test #u8(1 2 3 1 2)
    (let ((bv (bytevector 1 2 3 4 5)))
      (bytevector-copy! bv 3 bv 0 2)
      bv))

(test #u8() (bytevector-append))
(test #u8() (bytevector-append #u8()))
(test #u8() (bytevector-append #u8() #u8()))
(test #u8(0 1 2) (bytevector-append #u8() #u8(0 1 2)))
(test #u8(0 1 2) (bytevector-append #u8(0 1 2) #u8()))
(test #u8(0 1 2 3 4) (bytevector-append #u8(0 1 2) #u8(3 4)))
(test #u8(0 1 2 3 4 5) (bytevector-append #u8(0 1 2) #u8(3 4) #u8(5)))

(test "" (utf8->string #u8()))
(test "ABC" (utf8->string #u8(#x41 #x42 #x43)))
(test "ABC" (utf8->string #u8(0 #x41 #x42 #x43) 1))
(test "ABC" (utf8->string #u8(0 #x41  #x42 #x43 0) 1 4))
;(test "λ" (utf8->string #u8(0 #xCE #xBB 0) 1 3))
(test #u8() (string->utf8 ""))
(test #u8(#x41 #x42 #x43) (string->utf8 "ABC"))
(test #u8(#x42 #x43) (string->utf8 "ABC" 1))
(test #u8(#x42) (string->utf8 "ABC" 1 2))
;(test #u8(#xCE #xBB) (string->utf8 "λ"))


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(test-begin "6.10 Control Features")

(test #t (procedure? car))
(test #f (procedure? 'car))
(test #t (procedure? (lambda (x) (* x x))))
(test #f (procedure? '(lambda (x) (* x x))))
(test #t (call-with-current-continuation procedure?))

(test 7 (apply + (list 3 4)))
(test 7 (apply + 3 4 (list)))
;(test-error (apply +)) ;; not enough args
;(test-error (apply + 3)) ;; final arg not a list
;(test-error (apply + 3 4)) ;; final arg not a list
;(test-error (apply + '(2 3 . 4))) ;; final arg is improper


(define compose
  (lambda (f g)
    (lambda args
      (f (apply g args)))))
(test '(30 0)
    (call-with-values (lambda () ((compose exact-integer-sqrt *) 12 75))
      list))

(test '(b e h) (map cadr '((a b) (d e) (g h))))

(test '(1 4 27 256 3125) (map (lambda (n) (expt n n)) '(1 2 3 4 5)))

(test '(5 7 9) (map + '(1 2 3) '(4 5 6 7)))

(test #t
    (let ((res (let ((count 0))
                 (map (lambda (ignored)
                        (set! count (+ count 1))
                        count)
                      '(a b)))))
      (or (equal? res '(1 2))
          (equal? res '(2 1)))))

; map should terminate on the shortest list!
(test '(10 200 3000 40 500 6000)
    (let ((ls1 (list 10 100 1000))
          (ls2 (list 1 2 3 4 5 6)))
      (set-cdr! (cddr ls1) ls1)
      (map * ls1 ls2)))

(test "abdegh" (string-map char-foldcase "AbdEgH"))

(test "IBM" (string-map
 (lambda (c)
   (integer->char (+ 1 (char->integer c))))
 "HAL"))

(test "StUdLyCaPs"
    (string-map
     (lambda (c k) (if (eqv? k #\u) (char-upcase c) (char-downcase c)))
     "studlycaps xxx"
     "ululululul"))

(test #(b e h) (vector-map cadr '#((a b) (d e) (g h))))

(test #(1 4 27 256 3125)
    (vector-map (lambda (n) (expt n n))
                '#(1 2 3 4 5)))

(test #(5 7 9) (vector-map + '#(1 2 3) '#(4 5 6 7)))

(test #t
    (let ((res (let ((count 0))
                 (vector-map
                  (lambda (ignored)
                    (set! count (+ count 1))
                    count)
                  '#(a b)))))
      (or (equal? res #(1 2))
          (equal? res #(2 1)))))

(test #(0 1 4 9 16)
    (let ((v (make-vector 5)))
      (for-each (lambda (i)
                  (vector-set! v i (* i i)))
                '(0 1 2 3 4))
      v))

(test 9750
    (let ((ls1 (list 10 100 1000))
          (ls2 (list 1 2 3 4 5 6))
          (count 0))
      (set-cdr! (cddr ls1) ls1)
      (for-each (lambda (x y) (set! count (+ count (* x y)))) ls2 ls1)
      count))

(test '(101 100 99 98 97)
    (let ((v '()))
      (string-for-each
       (lambda (c) (set! v (cons (char->integer c) v)))
       "abcde")
      v))

(test '(0 1 4 9 16) (let ((v (make-list 5)))
  (vector-for-each
   (lambda (i) (list-set! v i (* i i)))
   '#(0 1 2 3 4))
  v))

(test -3 (call-with-current-continuation
  (lambda (exit)
    (for-each (lambda (x)
                (if (negative? x)
                    (exit x)))
              '(54 0 37 -3 245 19))
    #t)))

(define list-length
  (lambda (obj)
    (call-with-current-continuation
      (lambda (return)
        (letrec ((r
                  (lambda (obj)
                    (cond ((null? obj) 0)
                          ((pair? obj)
                           (+ (r (cdr obj)) 1))
                          (else (return #f))))))
          (r obj))))))

(test 4 (list-length '(1 2 3 4)))

(test #f (list-length '(a b . c)))

(test 5
    (call-with-values (lambda () (values 4 5))
      (lambda (a b) b)))

(test -1 (call-with-values * -))


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(define-record-type <pare>
  (kons x y)
  pare?
  (x kar set-kar!)
  (y kdr))

(test #t (pare? (kons 1 2)))
(test #f (pare? (cons 1 2)))
(test 1 (kar (kons 1 2)))
(test 2 (kdr (kons 1 2)))
(test 3 (let ((k (kons 1 2)))
          (set-kar! k 3)
          (kar k)))

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


(test-begin "6.13 Input and output")

(test #t (port? (current-input-port)))
(test #t (input-port? (current-input-port)))
(test #t (output-port? (current-output-port)))
(test #t (output-port? (current-error-port)))
(test #t (input-port? (open-input-string "abc")))
(test #t (output-port? (open-output-string)))

(test #t (textual-port? (open-input-string "abc")))
(test #t (textual-port? (open-output-string)))
(test #t (binary-port? (open-input-bytevector #u8(0 1 2))))
(test #t (binary-port? (open-output-bytevector)))

(test #t (input-port-open? (open-input-string "abc")))
(test #t (output-port-open? (open-output-string)))

(test #f
    (let ((in (open-input-string "abc")))
      (close-input-port in)
      (input-port-open? in)))

(test #f
    (let ((out (open-output-string)))
      (close-output-port out)
      (output-port-open? out)))

(test #f
    (let ((out (open-output-string)))
      (close-port out)
      (output-port-open? out)))

; r7rs does not require signalling an error here
#;(test 'error
    (let ((in (open-input-string "abc")))
      (close-input-port in)
      (guard (exn (else 'error)) (read-char in))))

; r7rs does not require signalling an error here
#;(test 'error
    (let ((out (open-output-string)))
      (close-output-port out)
      (guard (exn (else 'error)) (write-char #\c out))))

(test #t (eof-object? (eof-object)))
(test #t (eof-object? (read (open-input-string ""))))
(test #t (char-ready? (open-input-string "42")))
(test 42 (read (open-input-string " 42 ")))

(test #t (eof-object? (read-char (open-input-string ""))))
(test #\a (read-char (open-input-string "abc")))

(test #t (eof-object? (read-line (open-input-string ""))))
(test "abc" (read-line (open-input-string "abc")))
(test "abc" (read-line (open-input-string "abc\ndef\n")))

(test #t (eof-object? (read-string 3 (open-input-string ""))))
(test "abc" (read-string 3 (open-input-string "abcd")))
(test "abc" (read-string 3 (open-input-string "abc\ndef\n")))

#|(let ((in (open-input-string (string #\x10F700 #\x10F701 #\x10F702))))
  (let* ((c0 (peek-char in))
         (c1 (read-char in))
         (c2 (read-char in))
         (c3 (read-char in)))
    (test #\x10F700 c0)
    (test #\x10F700 c1)
    (test #\x10F701 c2)
    (test #\x10F702 c3)))

(test (string #\x10F700)
    (let ((out (open-output-string)))
      (write-char #\x10F700 out)
      (get-output-string out)))|#

(test "abc"
    (let ((out (open-output-string)))
      (write 'abc out)
      (get-output-string out)))

(test "abc def"
    (let ((out (open-output-string)))
      (display "abc def" out)
      (get-output-string out)))

(test "abc"
    (let ((out (open-output-string)))
      (display #\a out)
      (display "b" out)
      (display #\c out)
      (get-output-string out)))

(test #t
      (let* ((out (open-output-string))
             (r (begin (newline out) (get-output-string out))))
        (or (equal? r "\n") (equal? r "\r\n"))))

(test "abc def"
    (let ((out (open-output-string)))
      (write-string "abc def" out)
      (get-output-string out)))

(test "def"
    (let ((out (open-output-string)))
      (write-string "abc def" out 4)
      (get-output-string out)))

(test "c d"
    (let ((out (open-output-string)))
      (write-string "abc def" out 2 5)
      (get-output-string out)))

(test ""
  (let ((out (open-output-string)))
    (flush-output-port out)
    (get-output-string out)))

(test #t (eof-object? (read-u8 (open-input-bytevector #u8()))))
(test 1 (read-u8 (open-input-bytevector #u8(1 2 3))))

(test #t (eof-object? (read-bytevector 3 (open-input-bytevector #u8()))))
(test #t (u8-ready? (open-input-bytevector #u8(1))))
(test #u8(1) (read-bytevector 3 (open-input-bytevector #u8(1))))
(test #u8(1 2) (read-bytevector 3 (open-input-bytevector #u8(1 2))))
(test #u8(1 2 3) (read-bytevector 3 (open-input-bytevector #u8(1 2 3))))
(test #u8(1 2 3) (read-bytevector 3 (open-input-bytevector #u8(1 2 3 4))))

(test #t
    (let ((bv (bytevector 1 2 3 4 5)))
      (eof-object? (read-bytevector! bv (open-input-bytevector #u8())))))

(test #u8(6 7 8 9 10)
  (let ((bv (bytevector 1 2 3 4 5)))
    (read-bytevector! bv (open-input-bytevector #u8(6 7 8 9 10)) 0 5)
    bv))

(test #u8(6 7 8 4 5)
  (let ((bv (bytevector 1 2 3 4 5)))
    (read-bytevector! bv (open-input-bytevector #u8(6 7 8 9 10)) 0 3)
    bv))

(test #u8(1 2 3 6 5)
  (let ((bv (bytevector 1 2 3 4 5)))
    (read-bytevector! bv (open-input-bytevector #u8(6 7 8 9 10)) 3 4)
    bv))

(test 2
  (let ((bv (bytevector 1 2 3 4 5)))
    (read-bytevector! bv (open-input-bytevector #u8(6 7)))))

(test #t
  (let ((bv (bytevector 1 2 3 4 5)))
    (eof-object? (read-bytevector! bv (open-input-bytevector #u8())))))

(test #u8(1 2 3)
  (let ((out (open-output-bytevector)))
    (write-u8 1 out)
    (write-u8 2 out)
    (write-u8 3 out)
    (get-output-bytevector out)))

(test #u8(1 2 3 4 5)
  (let ((out (open-output-bytevector)))
    (write-bytevector #u8(1 2 3 4 5) out)
    (get-output-bytevector out)))

(test #u8(3 4 5)
  (let ((out (open-output-bytevector)))
    (write-bytevector #u8(1 2 3 4 5) out 2)
    (get-output-bytevector out)))

(test #u8(3 4)
  (let ((out (open-output-bytevector)))
    (write-bytevector #u8(1 2 3 4 5) out 2 4)
    (get-output-bytevector out)))

(test #u8()
  (let ((out (open-output-bytevector)))
    (flush-output-port out)
    (get-output-bytevector out)))

(test #t
    (and (member
          (let ((out (open-output-string))
                (x (list 1)))
            (set-cdr! x x)
            (write x out)
            (get-output-string out))
          ;; labels not guaranteed to be 0 indexed, spacing may differ
          '("#0=(1 . #0#)" "#1=(1 . #1#)"))
         #t))

(test "((1 2 3) (1 2 3))"
    (let ((out (open-output-string))
          (x (list 1 2 3)))
      (write (list x x) out)
      (get-output-string out)))

(test "((1 2 3) (1 2 3))"
    (let ((out (open-output-string))
          (x (list 1 2 3)))
      (write-simple (list x x) out)
      (get-output-string out)))

(test #t
    (and (member (let ((out (open-output-string))
                       (x (list 1 2 3)))
                   (write-shared (list x x) out)
                   (get-output-string out))
                 '("(#0=(1 2 3) #0#)" "(#1=(1 2 3) #1#)"))
         #t))

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(test-begin "Read syntax")

;; check reading boolean followed by eof
(test #t (read (open-input-string "#t")))
(test #t (read (open-input-string "#true")))
(test #f (read (open-input-string "#f")))
(test #f (read (open-input-string "#false")))
(define (read2 port)
  (let* ((o1 (read port)) (o2 (read port)))
    (cons o1 o2)))
;; check reading boolean followed by delimiter
(test '(#t . (5)) (read2 (open-input-string "#t(5)")))
(test '(#t . 6) (read2 (open-input-string "#true 6 ")))
(test '(#f . 7) (read2 (open-input-string "#f 7")))
(test '(#f . "8") (read2 (open-input-string "#false\"8\"")))

(test '() (read (open-input-string "()")))
(test '(1 2) (read (open-input-string "(1 2)")))
(test '(1 . 2) (read (open-input-string "(1 . 2)")))
(test '(1 2) (read (open-input-string "(1 . (2))")))
(test '(1 2 3 4 5) (read (open-input-string "(1 . (2 3 4 . (5)))")))
(test '1 (cadr (read (open-input-string "#0=(1 . #0#)"))))
(test '(1 2 3) (cadr (read (open-input-string "(#0=(1 2 3) #0#)"))))

(test '(quote (1 2)) (read (open-input-string "'(1 2)")))
(test '(quote (1 (unquote 2))) (read (open-input-string "'(1 ,2)")))
(test '(quote (1 (unquote-splicing 2))) (read (open-input-string "'(1 ,@2)")))
(test '(quasiquote (1 (unquote 2))) (read (open-input-string "`(1 ,2)")))

(test #() (read (open-input-string "#()")))
(test #(a b) (read (open-input-string "#(a b)")))

(test #u8() (read (open-input-string "#u8()")))
(test #u8(0 1) (read (open-input-string "#u8(0 1)")))

(test 'abc (read (open-input-string "abc")))
(test 'abc (read (open-input-string "abc def")))
(test 'ABC (read (open-input-string "ABC")))
(test 'Hello (read (open-input-string "|H\\x65;llo|")))

;(test 'abc (read (open-input-string "#!fold-case ABC")))
;(test 'ABC (read (open-input-string "#!fold-case #!no-fold-case ABC")))

(test 'def (read (open-input-string "#; abc def")))
(test 'def (read (open-input-string "; abc \ndef")))
(test 'def (read (open-input-string "#| abc |# def")))
(test 'ghi (read (open-input-string "#| abc #| def |# |# ghi")))
(test 'ghi (read (open-input-string "#; ; abc\n def ghi")))
(test '(abs -16) (read (open-input-string "(#;sqrt abs -16)")))
(test '(a d) (read (open-input-string "(a #; #;b c d)")))
(test '(a e) (read (open-input-string "(a #;(b #;c d) e)")))
(test '(a . c) (read (open-input-string "(a . #;b c)")))
(test '(a . b) (read (open-input-string "(a . b #;c)")))

(define (test-read-error str)
  (test-assert ;str
      (guard (exn (else #t))
        (read (open-input-string str))
        #f)))

(test-read-error "(#;a . b)")
(test-read-error "(a . #;b)")
(test-read-error "(a #;. b)")
(test-read-error "(#;x #;y . z)")
(test-read-error "(#; #;x #;y . z)")
(test-read-error "(#; #;x . z)")
(test-read-error "(#0#)")
(test-read-error "(#0=#0#)")

(test #\a (read (open-input-string "#\\a")))
(test #\space (read (open-input-string "#\\space")))
(test 0 (char->integer (read (open-input-string "#\\null"))))
(test 7 (char->integer (read (open-input-string "#\\alarm"))))
(test 8 (char->integer (read (open-input-string "#\\backspace"))))
(test 9 (char->integer (read (open-input-string "#\\tab"))))
(test 10 (char->integer (read (open-input-string "#\\newline"))))
(test 13 (char->integer (read (open-input-string "#\\return"))))
(test #x7F (char->integer (read (open-input-string "#\\delete"))))
(test #x1B (char->integer (read (open-input-string "#\\escape"))))
(test #x1C (char->integer (read (open-input-string "#\\x1C"))))
;(test #x03BB (char->integer (read (open-input-string "#\\λ"))))
;(test #x03BB (char->integer (read (open-input-string "#\\x03BB"))))

(test "abc" (read (open-input-string "\"abc\"")))
(test "abc" (read (open-input-string "\"abc\" \"def\"")))
(test "ABC" (read (open-input-string "\"ABC\"")))
(test "Hello" (read (open-input-string "\"H\\x65;llo\"")))
(test 7 (char->integer (string-ref (read (open-input-string "\"\\a\"")) 0)))
(test 8 (char->integer (string-ref (read (open-input-string "\"\\b\"")) 0)))
(test 9 (char->integer (string-ref (read (open-input-string "\"\\t\"")) 0)))
(test 10 (char->integer (string-ref (read (open-input-string "\"\\n\"")) 0)))
(test 13 (char->integer (string-ref (read (open-input-string "\"\\r\"")) 0)))
(test #x22 (char->integer (string-ref (read (open-input-string "\"\\\"\"")) 0)))
(test #x7C (char->integer (string-ref (read (open-input-string "\"\\|\"")) 0)))
(test #x1C (char->integer (string-ref (read (open-input-string "\"\\x1C;\"")) 0)))
(test "line 1\nline 2\n" (read (open-input-string "\"line 1\nline 2\n\"")))
(test "line 1continued\n" (read (open-input-string "\"line 1\\\ncontinued\n\"")))
(test "line 1continued\n" (read (open-input-string "\"line 1\\ \ncontinued\n\"")))
(test "line 1continued\n" (read (open-input-string "\"line 1\\\n continued\n\"")))
(test "line 1continued\n" (read (open-input-string "\"line 1\\ \t \n \t continued\n\"")))
(test "line 1\n\nline 3\n" (read (open-input-string "\"line 1\\ \t \n \t \n\nline 3\n\"")))
;(test #x03BB (char->integer (string-ref (read (open-input-string "\"\\x03BB;\"")) 0)))

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


;; Numeric syntax adapted from Peter Bex's tests.
;;
;; These are updated to R7RS, using string ports instead of
;; string->number, and "error" tests removed because implementations
;; are free to provide their own numeric extensions.  Currently all
;; tests are run by default - need to cond-expand and test for
;; infinities and -0.0.

(define-syntax test-numeric-syntax
  (syntax-rules ()
    ((test-numeric-syntax str expect strs ...)
     (let* ((z (string->number str))
            (z-str (if z (number->string z) "?")))
       (test~= expect (values z))
       (test #t (and (member z-str '(str strs ...)) #t))))))

;; Each test is of the form:
;;
;;   (test-numeric-syntax input-str expected-value expected-write-values ...)
;;
;; where the input should be eqv? to the expected-value, and the
;; written output the same as any of the expected-write-values.  The
;; form
;;
;;   (test-numeric-syntax input-str expected-value)
;;
;; is a shorthand for
;;
;;   (test-numeric-syntax input-str expected-value (input-str))
;; Simple
(test-numeric-syntax "1" 1)
(test-numeric-syntax "+1" 1 "1")
(test-numeric-syntax "-1" -1)
(test-numeric-syntax "#i1" 1.0 "1.0" "1.")
(test-numeric-syntax "#I1" 1.0 "1.0" "1.")
(test-numeric-syntax "#i-1" -1.0 "-1.0" "-1.")
;; Decimal
(test-numeric-syntax "1.0" 1.0 "1.0" "1.")
(test-numeric-syntax "1." 1.0 "1.0" "1.")
(test-numeric-syntax ".1" 0.1 "0.1" "100.0e-3")
(test-numeric-syntax "-.1" -0.1 "-0.1" "-100.0e-3")
;; Some Schemes don't allow negative zero. This is okay with the standard
(test-numeric-syntax "-.0" -0.0 "-0." "-0.0" "0.0" "0." ".0")
(test-numeric-syntax "-0." -0.0 "-.0" "-0.0" "0.0" "0." ".0")
(test-numeric-syntax "#i1.0" 1.0 "1.0" "1.")
(test-numeric-syntax "#e1.0" 1 "1")
(test-numeric-syntax "#e-.0" 0 "0")
(test-numeric-syntax "#e-0." 0 "0")
(test-numeric-syntax "#E1.0" 1 "1")
(test-numeric-syntax "#E-.0" 0 "0")
(test-numeric-syntax "#E-0." 0 "0")
;; Decimal notation with suffix
(test-numeric-syntax "1e2" 100.0 "100.0" "100.")
(test-numeric-syntax "1E2" 100.0 "100.0" "100.")
;(test-numeric-syntax "1s2" 100.0 "100.0" "100.")
;(test-numeric-syntax "1S2" 100.0 "100.0" "100.")
;(test-numeric-syntax "1f2" 100.0 "100.0" "100.")
;(test-numeric-syntax "1F2" 100.0 "100.0" "100.")
;(test-numeric-syntax "1d2" 100.0 "100.0" "100.")
;(test-numeric-syntax "1D2" 100.0 "100.0" "100.")
;(test-numeric-syntax "1l2" 100.0 "100.0" "100.")
;(test-numeric-syntax "1L2" 100.0 "100.0" "100.")
;; NaN, Inf
(test-numeric-syntax "+nan.0" +nan.0 "+nan.0" "+NaN.0")
(test-numeric-syntax "+NAN.0" +nan.0 "+nan.0" "+NaN.0")
(test-numeric-syntax "+inf.0" +inf.0 "+inf.0" "+Inf.0")
(test-numeric-syntax "+InF.0" +inf.0 "+inf.0" "+Inf.0")
(test-numeric-syntax "-inf.0" -inf.0 "-inf.0" "-Inf.0")
(test-numeric-syntax "-iNF.0" -inf.0 "-inf.0" "-Inf.0")
(test-numeric-syntax "#i+nan.0" +nan.0 "+nan.0" "+NaN.0")
(test-numeric-syntax "#i+inf.0" +inf.0 "+inf.0" "+Inf.0")
(test-numeric-syntax "#i-inf.0" -inf.0 "-inf.0" "-Inf.0")
#|
;; Exact ratios
(test-numeric-syntax "1/2" (/ 1 2))
(test-numeric-syntax "#e1/2" (/ 1 2) "1/2")
(test-numeric-syntax "10/2" 5 "5")
(test-numeric-syntax "-1/2" (- (/ 1 2)))
(test-numeric-syntax "0/10" 0 "0")
(test-numeric-syntax "#e0/10" 0 "0")
(test-numeric-syntax "#i3/2" (/ 3.0 2.0) "1.5")
;; Exact complex
(test-numeric-syntax "1+2i" (make-rectangular 1 2))
(test-numeric-syntax "1+2I" (make-rectangular 1 2) "1+2i")
(test-numeric-syntax "1-2i" (make-rectangular 1 -2))
(test-numeric-syntax "-1+2i" (make-rectangular -1 2))
(test-numeric-syntax "-1-2i" (make-rectangular -1 -2))
(test-numeric-syntax "+i" (make-rectangular 0 1) "+i" "+1i" "0+i" "0+1i")
(test-numeric-syntax "0+i" (make-rectangular 0 1) "+i" "+1i" "0+i" "0+1i")
(test-numeric-syntax "0+1i" (make-rectangular 0 1) "+i" "+1i" "0+i" "0+1i")
(test-numeric-syntax "-i" (make-rectangular 0 -1) "-i" "-1i" "0-i" "0-1i")
(test-numeric-syntax "0-i" (make-rectangular 0 -1) "-i" "-1i" "0-i" "0-1i")
(test-numeric-syntax "0-1i" (make-rectangular 0 -1) "-i" "-1i" "0-i" "0-1i")
(test-numeric-syntax "+2i" (make-rectangular 0 2) "2i" "+2i" "0+2i")
(test-numeric-syntax "-2i" (make-rectangular 0 -2) "-2i" "0-2i")
;; Decimal-notation complex numbers (rectangular notation)
(test-numeric-syntax "1.0+2i" (make-rectangular 1.0 2) "1.0+2.0i" "1.0+2i" "1.+2i" "1.+2.i")
(test-numeric-syntax "1+2.0i" (make-rectangular 1 2.0) "1.0+2.0i" "1+2.0i" "1.+2.i" "1+2.i")
(test-numeric-syntax "1e2+1.0i" (make-rectangular 100.0 1.0) "100.0+1.0i" "100.+1.i")
(test-numeric-syntax "1s2+1.0i" (make-rectangular 100.0 1.0) "100.0+1.0i" "100.+1.i")
(test-numeric-syntax "1.0+1e2i" (make-rectangular 1.0 100.0) "1.0+100.0i" "1.+100.i")
(test-numeric-syntax "1.0+1s2i" (make-rectangular 1.0 100.0) "1.0+100.0i" "1.+100.i")
;; Fractional complex numbers (rectangular notation)
(test-numeric-syntax "1/2+3/4i" (make-rectangular (/ 1 2) (/ 3 4)))
;; Mixed fractional/decimal notation complex numbers (rectangular notation)
(test-numeric-syntax "0.5+3/4i" (make-rectangular 0.5 (/ 3 4))
  "0.5+0.75i" ".5+.75i" "0.5+3/4i" ".5+3/4i" "500.0e-3+750.0e-3i")
;; Complex NaN, Inf (rectangular notation)
;;(test-numeric-syntax "+nan.0+nan.0i" (make-rectangular the-nan the-nan) "+NaN.0+NaN.0i") 
(test-numeric-syntax "+inf.0+inf.0i" (make-rectangular +inf.0 +inf.0) "+Inf.0+Inf.0i")
(test-numeric-syntax "-inf.0+inf.0i" (make-rectangular -inf.0 +inf.0) "-Inf.0+Inf.0i")
(test-numeric-syntax "-inf.0-inf.0i" (make-rectangular -inf.0 -inf.0) "-Inf.0-Inf.0i")
(test-numeric-syntax "+inf.0-inf.0i" (make-rectangular +inf.0 -inf.0) "+Inf.0-Inf.0i")
;; Complex numbers (polar notation)
;; Need to account for imprecision in write output.
;;(test-numeric-syntax "1@2" -0.416146836547142+0.909297426825682i "-0.416146836547142+0.909297426825682i")
|#
;; Base prefixes
(test-numeric-syntax "#x11" 17 "17")
(test-numeric-syntax "#X11" 17 "17")
(test-numeric-syntax "#d11" 11 "11")
(test-numeric-syntax "#D11" 11 "11")
(test-numeric-syntax "#o11" 9 "9")
(test-numeric-syntax "#O11" 9 "9")
(test-numeric-syntax "#b11" 3 "3")
(test-numeric-syntax "#B11" 3 "3")
(test-numeric-syntax "#o7" 7 "7")
(test-numeric-syntax "#xa" 10 "10")
(test-numeric-syntax "#xA" 10 "10")
(test-numeric-syntax "#xf" 15 "15")
(test-numeric-syntax "#x-10" -16 "-16")
(test-numeric-syntax "#d-10" -10 "-10")
(test-numeric-syntax "#o-10" -8 "-8")
(test-numeric-syntax "#b-10" -2 "-2")
;; Combination of prefixes
(test-numeric-syntax "#e#x10" 16 "16")
(test-numeric-syntax "#E#X10" 16 "16")
(test-numeric-syntax "#X#E10" 16 "16")
(test-numeric-syntax "#i#x10" 16.0 "16.0" "16.")
;; (Attempted) decimal notation with base prefixes
(test-numeric-syntax "#d1." 1.0 "1.0" "1.")
(test-numeric-syntax "#d.1" 0.1 "0.1" ".1" "100.0e-3")
(test-numeric-syntax "#x1e2" 482 "482")
(test-numeric-syntax "#d1e2" 100.0 "100.0" "100.")
#|
;; Fractions with prefixes
(test-numeric-syntax "#x10/2" 8 "8")
(test-numeric-syntax "#x11/2" (/ 17 2) "17/2")
(test-numeric-syntax "#d11/2" (/ 11 2) "11/2")
(test-numeric-syntax "#o11/2" (/ 9 2) "9/2")
(test-numeric-syntax "#b11/10" (/ 3 2) "3/2")
;; Complex numbers with prefixes
;;(test-numeric-syntax "#x10+11i" (make-rectangular 16 17) "16+17i")
(test-numeric-syntax "#d1.0+1.0i" (make-rectangular 1.0 1.0) "1.0+1.0i" "1.+1.i")
(test-numeric-syntax "#d10+11i" (make-rectangular 10 11) "10+11i")
;;(test-numeric-syntax "#o10+11i" (make-rectangular 8 9) "8+9i")
;;(test-numeric-syntax "#b10+11i" (make-rectangular 2 3) "2+3i")
;;(test-numeric-syntax "#e1.0+1.0i" (make-rectangular 1 1) "1+1i" "1+i")
;;(test-numeric-syntax "#i1.0+1.0i" (make-rectangular 1.0 1.0) "1.0+1.0i" "1.+1.i")
|#
(define-syntax test-precision
  (syntax-rules ()
    ((test-round-trip str alt ...)
     (let* ((n (string->number str))
            (str2 (number->string n))
            (accepted (list str alt ...))
            (ls (member str2 accepted)))
       (test-assert (string-append "(member? " str2 " "
                                   (let ((out (open-output-string)))
                                     (write accepted out)
                                     (get-output-string out))
                                   ")")
         (pair? ls))
       (when (pair? ls)
         (test-assert (string-append "(eqv?: " str " " str2 ")")
           (eqv? n (string->number (car ls)))))))))
;(test-precision "-1.7976931348623157e+308" "-inf.0")
;(test-precision "4.940656458412465e-324" "4.94065645841247e-324" "5.0e-324" "0.0")
;(test-precision "9.881312916824931e-324" "9.88131291682493e-324" "1.0e-323" "0.0")
;(test-precision "1.48219693752374e-323" "1.5e-323" "0.0")
;(test-precision "1.976262583364986e-323" "1.97626258336499e-323" "2.0e-323" "0.0")
;(test-precision "2.470328229206233e-323" "2.47032822920623e-323" "2.5e-323" "0.0")
;(test-precision "2.420921664622108e-322" "2.42092166462211e-322" "2.4e-322" "0.0")
;(test-precision "2.420921664622108e-320" "2.42092166462211e-320" "2.421e-320" "0.0")
;(test-precision "1.4489974452386991" "1.4489975")
;(test-precision "0.14285714285714282" "0.14285714285714288" "0.14285715")
;(test-precision "1.7976931348623157e+308" "+inf.0")


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; 
;; read-write tests from chibi and larceny
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


; equal? tests to make sure it can deal with circular structures

(test (equal? 'a 'a) #t)
(test (equal? '(a) '(a)) #t)
(test (equal? '(a (b) c) '(a (b) c)) #t)
(test (equal? "abc" "abc") #t)
(test (equal? 2 2) #t)
(test (equal? (make-vector 5 'a)
               (make-vector 5 'a)) 
          #t)

; circular constants are legal in r7rs, but macroexpander hangs on them
;(test (equal? '#1=(a b . #1#)
;              '#2=(a b a b . #2#))
;           #t)

(test (equal? '#u8(1 2 3 4 5)
               (bytevector 1 2 3 4 5))
          #t)

(test (let* ((x (list 'a))
               (y (list 'a))
               (z (list x y)))
          (list (equal? z (list y x))
               (equal? z (list x x))))
          '(#t #t))

(let ()
  (define x
     (let ([x1 (vector 'h)]
               [x2 (let ([x (list #f)]) (set-car! x x) x)])
          (vector x1 (vector 'h) x1 (vector 'h) x1 x2)))
  (define y
     (let ([y1 (vector 'h)]
               [y2 (vector 'h)]
               [y3 (let ([x (list #f)]) (set-car! x x) x)])
          (vector (vector 'h) y1 y1 y2 y2 y3)))
     (test (equal? x y) #t))

 (let ()
   (define x
     (let ([x (cons (cons #f 'a) 'a)])
       (set-car! (car x) x)
       x))
   (define y
     (let ([y (cons (cons #f 'a) 'a)])
       (set-car! (car y) (car y))
       y))
   (test (equal? x y) #t))

(let ([k 100])
  (define x
    (let ([x1 (cons
                (let f ([n k])
                  (if (= n 0)
                      (let ([x0 (cons #f #f)])
                        (set-car! x0 x0)
                        (set-cdr! x0 x0)
                        x0)
                      (let ([xi (cons #f (f (- n 1)))])
                        (set-car! xi xi)
                        xi)))
                #f)])
      (set-cdr! x1 x1)
      x1))
  (define y
    (let* ([y2 (cons #f #f)] [y1 (cons y2 y2)])
      (set-car! y2 y1)
      (set-cdr! y2 y1)
      y1))
  (test (equal? x y) #t))

(test (let ((x (list 'a 'b 'c 'a))
               (y (list 'a 'b 'c 'a 'b 'c 'a)))
          (set-cdr! (list-tail x 2) x)
          (set-cdr! (list-tail y 5) y)
          (list
          (equal? x x)
          (equal? x y)
          (equal? (list x y 'a) (list y x 'b))))
          '(#t #t #f))

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(define-syntax test-io
  (syntax-rules ()
    [(_ str expr)
     (let ([p (open-output-string)])
       (write-shared expr p)  
       (test str (get-output-string p)))]))

(define-syntax test-cyclic-io
  (syntax-rules ()
    [(_ str expr)
     (let ([p (open-output-string)])
       (write expr p)  
       (test str (get-output-string p)))]))

; defined in libl
#;(define (last-pair ls) 
  (if (null? (cdr ls)) ls (last-pair (cdr ls))))

; defined in libl
#;(define (make-list n . o)
  (let ((default (if (pair? o) (car o))))
    (let lp ((n n) (res '()))
      (if (<= n 0) res (lp (- n 1) (cons default res))))))  

(define (circular-list x . args)
  (let ((res (cons x args)))
    (set-cdr! (last-pair res) res)
    res))

; (let ([l (make-list 1000000 1)]) (set-cdr! (last-pair l) l) (write-shared l) (newline))
; (let* ([x (list 1)] [l (make-list 100000 x)]) (set-cdr! (last-pair l) l) (write-shared l) (newline))

(test-io "(1)" (list 1))
(test-io "(1 2)" (list 1 2))
(test-io "(1 . 2)" (cons 1 2))

(test-io "#0=(1 . #0#)" (circular-list 1))
(test-io "#0=(1 2 . #0#)" (circular-list 1 2))
(test-io "(1 . #0=(2 . #0#))" (cons 1 (circular-list 2)))
(test-io "#0=(1 #0# 3)"
         (let ((x (list 1 2 3))) (set-car! (cdr x) x) x))
(test-io "(#0=(1 #0# 3))"
         (let ((x (list 1 2 3))) (set-car! (cdr x) x) (list x)))
(test-io "(#0=(1 #0# 3) #0#)"
         (let ((x (list 1 2 3))) (set-car! (cdr x) x) (list x x)))
(test-io "(#0=(1 . #0#) #1=(1 . #1#))"
         (list (circular-list 1) (circular-list 1)))

(test-io "(#0=(1 . 2) #1=(1 . 2) #2=(3 . 4) #0# #1# #2#)"
         (let ((a (cons 1 2)) (b (cons 1 2)) (c (cons 3 4)))
           (list a b c a b c)))
(test-cyclic-io "((1 . 2) (1 . 2) (3 . 4) (1 . 2) (1 . 2) (3 . 4))"
                (let ((a (cons 1 2)) (b (cons 1 2)) (c (cons 3 4)))
                  (list a b c a b c)))
(test-cyclic-io "#0=((1 . 2) (1 . 2) (3 . 4) . #0#)"
                (let* ((a (cons 1 2))
                       (b (cons 1 2))
                       (c (cons 3 4))
                       (ls (list a b c)))
                  (set-cdr! (cddr ls) ls)
                  ls))

(test-io "#0=#(#0#)"
         (let ((x (vector 1))) (vector-set! x 0 x) x))
(test-io "#0=#(1 #0#)"
         (let ((x (vector 1 2))) (vector-set! x 1 x) x))
(test-io "#0=#(1 #0# 3)"
         (let ((x (vector 1 2 3))) (vector-set! x 1 x) x))
(test-io "(#0=#(1 #0# 3))"
         (let ((x (vector 1 2 3))) (vector-set! x 1 x) (list x)))
(test-io "#0=#(#0# 2 #0#)"
         (let ((x (vector 1 2 3)))
           (vector-set! x 0 x)
           (vector-set! x 2 x)
           x))

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; tests from Larceny ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(define (write-datum writer x)
  (let ((q (open-output-string)))
    (writer x q) (get-output-string q)))

(define (write-data writer data)
  (map (lambda (datum) (write-datum writer datum)) data))    

(define (write-then-read-help writer datum)
  (let* ((s (write-datum writer datum))
         (p (open-input-string s))
         (x (read p)))
    (and (eof-object? (read p))
         x)))

(define (write-then-read datum)
 (let ((x1 (write-then-read-help write datum))
       (x2 (write-then-read-help write-shared datum))
       (x3 (write-then-read-help write-simple datum)))
   (if (and (equal? x1 x2) (equal? x2 x3))
       x1
       (list x1 x2 x3))))

(define (read-from-string s)
 (let ((p (open-input-string s)))
   (read p)))

;(define (string-downcase s)
;  (list->string (map char-downcase (string->list s))))

;; Strings

; skint can't read \| yet (legal in r7rs)
;(test (write-then-read "\r\n\t\b\a\|\"\\")
;     (list->string
;      (map integer->char
;           '(13 10 9 8 7 124 34 92))))

(test (write-then-read
      "\x7f;\x4c;\x61;\x72;\x63;\x65;\x6e;\x79;\x0;#x21;")
     (list->string
      (map integer->char
           '(127 76 97 114 99 101 110 121 0 35 120 50 49 59))))

;; Characters
#|
(test (write-then-read (map integer->char '(32 9 10 13)))
     '(#\space #\tab #\newline #\return))

; skint can't read \delete yet (legal in r7rs)
;(test (write-then-read '(#\alarm #\backspace #\delete #\escape #\newline))
;     (map integer->char '(7 8 127 27 10)))

(test (write-then-read '(#\null #\return #\space #\tab))
     (map integer->char '(0 13 32 9)))

(test (write-then-read '(#\return #\space #\tab))
     (map integer->char '(13 32 9)))

(test (write-then-read '(#\x0 #\x00 #\x1 #\x20 #\x5f #\x7c #\x7f))
     (map integer->char '(0 0 1 32 95 124 127)))
|#

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; <identifier>
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


;; <identifier> ::= <initial> <subsequent>*

(test (write-then-read (map string->symbol
                           '("a" "!" "$" "%" "&" "*" "/" ":"
                             "<" "=" ">" "?" "^" "_" "~" "@")))
     '(a ! $ % & * / : < = > ? ^ _ ~ @))

;; <subsequent> ::= <initial> | <digit> | <special subsequent>

(test (write-then-read
      '(Z: !z $0 %/ &? *^ /~ :@ <9 =+ >- ?@ ^+- _-+ ~@- @@@@@))
     (map string->symbol
          '("Z:" "!z" "$0" "%/" "&?" "*^" "/~" ":@"
            "<9" "=+" ">-" "?@" "^+-" "_-+" "~@-" "@@@@@")))

;; <identifier> ::= <vertical line> <symbol element>* <vertical line>


(test (write-then-read '|;alskjdf;aqwjepojq-1945apgf ;bna]as|)
     (string->symbol ";alskjdf;aqwjepojq-1945apgf ;bna]as"))

;; |\a\b\t\n\r\|\"\\|

(test (write-then-read '|\a\b\t\n\r\|\"\\|)
     (string->symbol
      (string #\alarm #\backspace #\tab #\newline #\return
              #\| #\" #\\)))
(test (write-then-read '(|\\\|\" a| |\"\|\\ b|))
     (map string->symbol
          (list (string #\\ #\| #\" #\space #\a)
                (string #\" #\| #\\ #\space #\b))))
(test (write-then-read '|\x000;\x01;\x2;\t\r\x41;\n\t\x7e;\x7f;|)
     (string->symbol
      (string #\null #\x1 #\x2 #\tab #\return #\A
              #\newline #\tab #\~ #\delete)))

;; <peculiar identifier>
;; <explicit sign>
(test (write-then-read '(+ -)) '(+ -))

;; <explicit sign> <sign subsequent> <subsequent>*
(test (write-then-read '(+: -@ +- -- +@ -@ +$$ -@3 +-4 --5 +@_ -@.))
     (map string->symbol
          '("+:" "-@" "+-" "--" "+@" "-@" "+$$"
            "-@3" "+-4" "--5" "+@_" "-@.")))
;; <explicit sign> . <dot subsequent> <subsequent>*
(test (write-then-read '(+.! -.+ +.. -.. +.@ -.@))
     (map string->symbol
          '("+.!" "-.+" "+.." "-.." "+.@" "-.@")))
(test (write-then-read '(+.<.+ -.++. +..:? -..- +.@&. -.@24))
     (map string->symbol
          '("+.<.+" "-.++." "+..:?" "-..-" "+.@&." "-.@24")))
;; . <dot subsequent> <subsequent>*
(test (write-then-read '(._ .+ .- .@ .. ._. .+. .-. .@. ...))
     (map string->symbol
          '("._" ".+" ".-" ".@" ".."
            "._." ".+." ".-." ".@." "...")))

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Miscellaneous.
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(test (read-from-string
      (write-datum display '(( ) #( ) #u8( ) 'x `x ,x ,@x (a . b))))
     '(() #() #u8() 'x `x ,x ,@x (a . b)))

(test (write-then-read '(( ) #( ) #u8( ) 'x `x ,x ,@x (a . b)))
     '(() #() #u8() 'x `x ,x ,@x (a . b)))

;; Shared data.
(test (write-then-read (let* ((x (list 1 2))
                             (y (list x x)))
                        (list y y)))
     '(((1 2) (1 2)) ((1 2) (1 2))))

(test (let ((z (read-from-string
               (write-datum write-shared (let* ((x (list 1 2))
                                                (y (list x x)))
                                           (list y y))))))
       (list (eq? (car z) (cadr z))
             (eq? (car (car z)) (cadr (car z)))))
     '(#t #t))

;; Circular data.
(test (read-from-string
      (write-datum write (let* ((x (list 1 2 3 4 5))
                                (v (vector x x x)))
                           (set-car! (cddr x) v)
                           (set-cdr! (cdr (cdr (cdr x))) x)
                           (vector-set! v 1 (list v))
                           v)))
     (let* ((x (list 1 2 3 4 5))
            (v (vector x x x)))
       (set-car! (cddr x) v)
       (set-cdr! (cdr (cdr (cdr x))) x)
       (vector-set! v 1 (list v))
       v))

(test (read-from-string
      (write-datum write-shared (let* ((x (list 1 2 3 4 5))
                                       (v (vector x x x)))
                                       (set-car! (cddr x) v)
                                       (set-cdr! (cdr (cdr (cdr x))) x)
                                       (vector-set! v 1 (list v))
                                       v)))
     (let* ((x (list 1 2 3 4 5))
            (v (vector x x x)))
       (set-car! (cddr x) v)
       (set-cdr! (cdr (cdr (cdr x))) x)
       (vector-set! v 1 (list v))
       v))

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;
;;; Some of these tests enforce a conventional representation
;;; when unconventional representations might be legal ouputs.
;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;; <boolean>
(test (and (member (string-downcase (write-datum display #t))
                  '("#t" "#true"))
          #t)
     #t)
(test (and (member (string-downcase (write-datum write #t))
                  '("#t" "#true"))
          #t)
     #t)
(test (and (member (string-downcase (write-datum write-shared #t))
                  '("#t" "#true"))
          #t)
     #t)
(test (and (member (string-downcase (write-datum write-simple #t))
                  '("#t" "#true"))
          #t)
     #t)
(test (and (member (string-downcase (write-datum display #f))
                  '("#f" "#false"))
          #t)
     #t)
;; <string>
(test (and (member (string-downcase (write-datum write #f))
                  '("#f" "#false"))
          #t)
     #t)
;; <string>
(test (and (member (string-downcase (write-datum write-shared #f))
                  '("#f" "#false"))
          #t)
     #t)
;; <string>
(test (and (member (string-downcase (write-datum write-simple #f))
                  '("#f" "#false"))
          #t)
     #t)
;; <string>
(test (write-datum display
                  "The quick red fox jumped over the lazy dog.")
     "The quick red fox jumped over the lazy dog.")
;; <character>
(test (write-datum write
                  "The quick red fox jumped over the lazy dog.")
     "\"The quick red fox jumped over the lazy dog.\"")

;; <character>

(test (write-datum write-shared
                  "The quick red fox jumped over the lazy dog.")
     "\"The quick red fox jumped over the lazy dog.\"")

;; <character>

(test (write-datum write-simple
                  "The quick red fox jumped over the lazy dog.")
     "\"The quick red fox jumped over the lazy dog.\"")

;; <character>

(test (write-data display
                 '(#\a  #\Z  #\0  #\9  #\`  #\'   #\"  #\~  #\!  #\=))
     '("a" "Z" "0" "9" "`" "'" "\"" "~" "!" "="))

(test (write-data write
                 '(#\a  #\Z  #\0  #\9  #\`  #\'   #\"  #\~  #\!  #\=))
     '("#\\a" "#\\Z" "#\\0" "#\\9" "#\\`" "#\\'"
       "#\\\"" "#\\~" "#\\!" "#\\="))

(test (write-data write-shared
                 '(#\a  #\Z  #\0  #\9  #\`  #\'   #\"  #\~  #\!  #\=))
     '("#\\a" "#\\Z" "#\\0" "#\\9" "#\\`" "#\\'"
       "#\\\"" "#\\~" "#\\!" "#\\="))

(test (write-data write-simple
                 '(#\a  #\Z  #\0  #\9  #\`  #\'   #\"  #\~  #\!  #\=))
     '("#\\a" "#\\Z" "#\\0" "#\\9" "#\\`" "#\\'"
       "#\\\"" "#\\~" "#\\!" "#\\="))

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; <identifier>
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;; <identifier> ::= <initial> <subsequent>*

(test (write-data display (map string->symbol
                            '("a" "!" "$" "%" "&" "*" "/" ":"
                              "<" "=" ">" "?" "^" "_" "~" "@")))
     '("a" "!" "$" "%" "&" "*" "/" ":" "<" "=" ">" "?" "^" "_" "~" "@"))

(test (write-data write (map string->symbol
                            '("a" "!" "$" "%" "&" "*" "/" ":"
                              "<" "=" ">" "?" "^" "_" "~" "@")))
     '("a" "!" "$" "%" "&" "*" "/" ":" "<" "=" ">" "?" "^" "_" "~" "@"))

(test (write-data write-shared (map string->symbol
                            '("a" "!" "$" "%" "&" "*" "/" ":"
                              "<" "=" ">" "?" "^" "_" "~" "@")))
     '("a" "!" "$" "%" "&" "*" "/" ":" "<" "=" ">" "?" "^" "_" "~" "@"))

(test (write-data write-simple (map string->symbol
                            '("a" "!" "$" "%" "&" "*" "/" ":"
                              "<" "=" ">" "?" "^" "_" "~" "@")))
     '("a" "!" "$" "%" "&" "*" "/" ":" "<" "=" ">" "?" "^" "_" "~" "@"))

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; <number>
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;; small exact integers

(test (write-data display
                 '(0 +0 -0 00 +0000 -00000 001 2 -003 007 97 1001))
     '("0" "0" "0" "0" "0" "0" "1" "2" "-3" "7" "97" "1001"))

(test (write-data write
                 '(0 +0 -0 00 +0000 -00000 001 2 -003 007 97 1001))
     '("0" "0" "0" "0" "0" "0" "1" "2" "-3" "7" "97" "1001"))

(test (write-data write-shared
                 '(0 +0 -0 00 +0000 -00000 001 2 -003 007 97 1001))
     '("0" "0" "0" "0" "0" "0" "1" "2" "-3" "7" "97" "1001"))

(test (write-data write-simple
                 '(0 +0 -0 00 +0000 -00000 001 2 -003 007 97 1001))
     '("0" "0" "0" "0" "0" "0" "1" "2" "-3" "7" "97" "1001"))

(test (write-data display
                 '(#e0 #e+0 #e-0 #e00 #e+0000 #e-00000
                   #e001 #e2 #e-003 #e007 #e97 #e1001))
     '("0" "0" "0" "0" "0" "0" "1" "2" "-3" "7" "97" "1001"))

(test (write-data write
                 '(#e0 #e+0 #e-0 #e00 #e+0000 #e-00000
                   #e001 #e2 #e-003 #e007 #e97 #e1001))
     '("0" "0" "0" "0" "0" "0" "1" "2" "-3" "7" "97" "1001"))

(test (write-data write-shared
                 '(#e0 #e+0 #e-0 #e00 #e+0000 #e-00000
                   #e001 #e2 #e-003 #e007 #e97 #e1001))
     '("0" "0" "0" "0" "0" "0" "1" "2" "-3" "7" "97" "1001"))

(test (write-data write-simple
                 '(#e0 #e+0 #e-0 #e00 #e+0000 #e-00000
                   #e001 #e2 #e-003 #e007 #e97 #e1001))
     '("0" "0" "0" "0" "0" "0" "1" "2" "-3" "7" "97" "1001"))

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; External representations of data.
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(test (write-data display '(#u8() #u8(0 1 2 3 255)))
     '("#u8()" "#u8(0 1 2 3 255)"))

(test (write-data write '(#u8() #u8(0 1 2 3 255)))
     '("#u8()" "#u8(0 1 2 3 255)"))

(test (write-data write-shared '(#u8() #u8(0 1 2 3 255)))
     '("#u8()" "#u8(0 1 2 3 255)"))

(test (write-data write-simple '(#u8() #u8(0 1 2 3 255)))
     '("#u8()" "#u8(0 1 2 3 255)"))

(test (write-data display '(() (1 . 2) (3 . (4 . (5 . ())))))
     '("()" "(1 . 2)" "(3 4 5)"))

(test (write-data write '(() (1 . 2) (3 . (4 . (5 . ())))))
     '("()" "(1 . 2)" "(3 4 5)"))

(test (write-data write-shared '(() (1 . 2) (3 . (4 . (5 . ())))))
     '("()" "(1 . 2)" "(3 4 5)"))

(test (write-data write-simple '(() (1 . 2) (3 . (4 . (5 . ())))))
     '("()" "(1 . 2)" "(3 4 5)"))

(test (write-data display '(#() #(a) #(19 21 c)))
     '("#()" "#(a)" "#(19 21 c)"))

(test (write-data write '(#() #(a) #(19 21 c)))
     '("#()" "#(a)" "#(19 21 c)"))

(test (write-data write-shared '(#() #(a) #(19 21 c)))
     '("#()" "#(a)" "#(19 21 c)"))

(test (write-data write-simple '(#() #(a) #(19 21 c)))
     '("#()" "#(a)" "#(19 21 c)"))

;; Skint extras

(define s "foo")
(define v #(foo))
(test #f (eq? (string-append s) s))
(test #f (eq? (vector-append v) v))

; expt should overflow
(test 1 (expt 0 0))
(test 0.1 (expt 10 -1))
(test 0.0009765625 (expt 2 -10))
(test -32768 (expt -2 15))
(test -536870912 (expt -2 29))
(test 536870912.0 (expt 2 29))
(test 6.80564733841877e+038 (expt 2 129))

; fixnum ops shouldn't fail on overflow, but may return whatever
(test #t (fixnum? (fx- -536870912 1)))
(test #t (fixnum? (fx* -536870912 -1)))
(test #t (fixnum? (fx* 536870911 536870911)))

; floor division


; _ and ... as literals:
(define-syntax test-specials (syntax-rules (_ ...) ((_ _ ...) '(_ ...)) ((_ x y) (vector x y))))
(test (list (test-specials _ ...) (test-specials 1 2))
     '((_ ...) #(1 2)))


(test-end)
