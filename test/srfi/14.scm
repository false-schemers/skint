(import (srfi 14))
(include "test.scm")

(define-syntax test-cs
  (syntax-rules ()
    ((test-cs . o)
     (test-equal char-set= . o))))

(define (vowel? c)
  (member c '(#\a #\e #\i #\o #\u)))

(test-begin "srfi-14: char-sets")

(test-not (char-set? 5))

(test-assert (char-set? (char-set #\a #\e #\i #\o #\u)))

(test-assert (char-set=))
(test-assert (char-set= (char-set)))

(test-cs (char-set #\a #\e #\i #\o #\u)
         (string->char-set "ioeauaiii"))

(test-not (char-set= (char-set #\e #\i #\o #\u)
                     (string->char-set "ioeauaiii")))

(test-assert (char-set<=))
(test-assert (char-set<= (char-set)))

(test-assert (char-set<= (char-set #\a #\e #\i #\o #\u)
                         (string->char-set "ioeauaiii")))

(test-assert (char-set<= (char-set #\e #\i #\o #\u)
                         (string->char-set "ioeauaiii")))

(test-assert (<= 0 (char-set-hash char-set:graphic 100) 99))

(test 4 (char-set-fold (lambda (c i) (+ i 1)) 0
                       (char-set #\e #\i #\o #\u #\e #\e)))

(test-cs (string->char-set "eiaou2468013579999")
         (char-set-unfold null? car cdr
                          '(#\a #\e #\i #\o #\u #\u #\u)
                          (char-set-intersection char-set:ascii
                                                 char-set:digit)))

(test-cs (string->char-set "eiaou246801357999")
         (char-set-unfold! null? car cdr '(#\a #\e #\i #\o #\u)
                           (string->char-set "0123456789")))

(test-not (char-set= (string->char-set "eiaou246801357")
                     (char-set-unfold! null? car cdr
                                       '(#\a #\e #\i #\o #\u)
                                       (string->char-set "0123456789"))))

(let ((cs (string->char-set "0123456789")))
  (char-set-for-each (lambda (c) (set! cs (char-set-delete cs c)))
                     (string->char-set "02468000"))
  (test-cs cs (string->char-set "97531")))

(test-not (let ((cs (string->char-set "0123456789")))
            (char-set-for-each (lambda (c) (set! cs (char-set-delete cs c)))
                               (string->char-set "02468"))
            (char-set= cs (string->char-set "7531"))))

(test-cs (string->char-set "IOUAEEEE")
         (char-set-map char-upcase (string->char-set "aeiou")))

(test-not (char-set= (char-set-map char-upcase (string->char-set "aeiou"))
                     (string->char-set "OUAEEEE")))

(test-cs (string->char-set "aeiou")
         (char-set-copy (string->char-set "aeiou")))

(test-cs (string->char-set "xy") (char-set #\x #\y))
(test-not (char-set= (char-set #\x #\y #\z) (string->char-set "xy")))

(test-cs (string->char-set "xy") (list->char-set '(#\x #\y)))
(test-not (char-set= (string->char-set "axy")
                     (list->char-set '(#\x #\y))))

(test-cs (string->char-set "xy12345")
         (list->char-set '(#\x #\y) (string->char-set "12345")))
(test-not (char-set= (string->char-set "y12345")
                     (list->char-set '(#\x #\y)
                                     (string->char-set "12345"))))

(test-cs (string->char-set "xy12345")
         (list->char-set! '(#\x #\y) (string->char-set "12345")))
(test-not (char-set= (string->char-set "y12345")
                     (list->char-set! '(#\x #\y)
                                      (string->char-set "12345"))))

(test-cs (string->char-set "aeiou12345")
         (char-set-filter vowel?
                          char-set:ascii
                          (string->char-set "12345")))
(test-not (char-set= (string->char-set "aeou12345")
                     (char-set-filter vowel?
                                      char-set:ascii
                                      (string->char-set "12345"))))

(test-cs (string->char-set "aeiou12345")
         (char-set-filter! vowel?
                           char-set:ascii
                           (string->char-set "12345")))
(test-not (char-set= (string->char-set "aeou12345")
                     (char-set-filter! vowel?
                                       char-set:ascii
                                       (string->char-set "12345"))))

(test-cs (string->char-set "abcdef12345")
         (ucs-range->char-set 97 103 #t (string->char-set "12345")))
(test-not (char-set=
           (string->char-set "abcef12345")
           (ucs-range->char-set 97 103 #t (string->char-set "12345"))))

(test-cs (string->char-set "abcdef12345")
         (ucs-range->char-set! 97 103 #t (string->char-set "12345")))
(test-not (char-set=
           (string->char-set "abcef12345")
           (ucs-range->char-set! 97 103 #t (string->char-set "12345"))))

(test-assert (char-set= (->char-set #\x)
                        (->char-set "x")
                        (->char-set (char-set #\x))))

(test-not (char-set= (->char-set #\x)
                     (->char-set "y")
                     (->char-set (char-set #\x))))

(test 10 (char-set-size
          (char-set-intersection char-set:ascii char-set:digit)))

(test 5 (char-set-count vowel? char-set:ascii))

(test '(#\x) (char-set->list (char-set #\x)))
(test-not (equal? '(#\X) (char-set->list (char-set #\x))))

(test "x" (char-set->string (char-set #\x)))
(test-not (equal? "X" (char-set->string (char-set #\x))))

(test-assert (char-set-contains? (->char-set "xyz") #\x))
(test-not (char-set-contains? (->char-set "xyz") #\a))

(test-assert (char-set-every char-lower-case? (->char-set "abcd")))
(test-not (char-set-every char-lower-case? (->char-set "abcD")))
(test-assert (char-set-any char-lower-case? (->char-set "abcd")))
(test-not (char-set-any char-lower-case? (->char-set "ABCD")))

(test-cs (->char-set "ABCD")
         (let ((cs (->char-set "abcd")))
           (let lp ((cur (char-set-cursor cs)) (ans '()))
             (if (end-of-char-set? cur) (list->char-set ans)
                 (lp (char-set-cursor-next cs cur)
                     (cons (char-upcase (char-set-ref cs cur)) ans))))))


(test-cs (->char-set "123xa")
         (char-set-adjoin (->char-set "123") #\x #\a))
(test-not (char-set= (char-set-adjoin (->char-set "123") #\x #\a)
                     (->char-set "123x")))
(test-cs (->char-set "123xa")
         (char-set-adjoin! (->char-set "123") #\x #\a))
(test-not (char-set= (char-set-adjoin! (->char-set "123") #\x #\a)
                     (->char-set "123x")))

(test-cs (->char-set "13")
         (char-set-delete (->char-set "123") #\2 #\a #\2))
(test-not (char-set= (char-set-delete (->char-set "123") #\2 #\a #\2)
                     (->char-set "13a")))
(test-cs (->char-set "13")
         (char-set-delete! (->char-set "123") #\2 #\a #\2))
(test-not (char-set= (char-set-delete! (->char-set "123") #\2 #\a #\2)
                     (->char-set "13a")))

(test-cs (->char-set "abcdefABCDEF")
         (char-set-intersection char-set:hex-digit
                                (char-set-complement char-set:digit)))
(test-cs (->char-set "abcdefABCDEF")
         (char-set-intersection!
          (char-set-complement! (->char-set "0123456789"))
          char-set:hex-digit))

(test-cs (->char-set "abcdefABCDEFghijkl0123456789")
         (char-set-union char-set:hex-digit
                         (->char-set "abcdefghijkl")))
(test-cs (->char-set "abcdefABCDEFghijkl0123456789")
         (char-set-union! (->char-set "abcdefghijkl")
                          char-set:hex-digit))

(test-cs (->char-set "ghijklmn")
         (char-set-difference (->char-set "abcdefghijklmn")
                              char-set:hex-digit))
(test-cs (->char-set "ghijklmn")
         (char-set-difference! (->char-set "abcdefghijklmn")
                               char-set:hex-digit))

(test-cs (->char-set "abcdefABCDEF")
         (char-set-xor (->char-set "0123456789")
                       char-set:hex-digit))
(test-cs (->char-set "abcdefABCDEF")
         (char-set-xor! (->char-set "0123456789")
                        char-set:hex-digit))

(call-with-values
    (lambda ()
      (char-set-diff+intersection char-set:hex-digit
                                  char-set:letter))
  (lambda (d i)
    (test-cs d (->char-set "0123456789"))
    (test-cs i (->char-set "abcdefABCDEF"))))

(call-with-values
    (lambda ()
      (char-set-diff+intersection! (char-set-copy char-set:hex-digit)
                                   (char-set-copy char-set:letter)))
  (lambda (d i)
    (test-cs d (->char-set "0123456789"))
    (test-cs i (->char-set "abcdefABCDEF"))))

;;; Extra tests

;;; === 1. Predicates and Comparisons ===

(test #t (char-set? char-set:empty))
(test #t (char-set? (char-set #\a #\b)))
(test #f (char-set? 'not-a-char-set))
(test #f (char-set? #\c))

;; Set equality: char-set=
(test #t (char-set=))
(test #t (char-set= char-set:empty))
(test #t (char-set= (char-set #\a #\b) (char-set #\b #\a)))
(test #f (char-set= (char-set #\a #\b) (char-set #\a)))
(test #t (char-set= (char-set #\x #\y) (char-set #\y #\x) (char-set #\x #\y)))

;; Subset relation: char-set<=
(test #t (char-set<=))
(test #t (char-set<= char-set:empty))
(test #t (char-set<= (char-set #\a) (char-set #\a #\b) (char-set #\a #\b #\c)))
(test #f (char-set<= (char-set #\a #\b) (char-set #\a)))
(test #t (char-set<= (char-set #\x) (char-set #\x)))


;;; === 2. Hashing ===

(test #t (integer? (char-set-hash (char-set #\a #\b))))
(test #t (exact-integer? (char-set-hash (char-set #\a #\b) 100)))
(test #t (let ([h (char-set-hash (char-set #\a #\b) 100)]) (and (>= h 0) (< h 100))))


;;; === 3. Cursor Interface ===

(test #t (end-of-char-set? (char-set-cursor char-set:empty)))
(test #f (end-of-char-set? (char-set-cursor (char-set #\a))))
(test #\a (char-set-ref (char-set #\a) (char-set-cursor (char-set #\a))))
(test #t (end-of-char-set? (char-set-cursor-next (char-set #\a) (char-set-cursor (char-set #\a)))))


;;; === 4. Iterators and Mapping ===

;; char-set-fold
(test 3 (char-set-fold (lambda (c acc) (+ acc 1)) 0 (char-set #\a #\b #\c)))
(test #t (char-set= (char-set #\a #\b #\c) 
                    (list->char-set (char-set-fold cons '() (char-set #\a #\b #\c)))))

;; char-set-unfold and char-set-unfold!
(test #t (char-set= (char-set #\a #\b #\c)
                    (char-set-unfold (lambda (x) (= x 3))
                                     (lambda (x) (vector-ref '#(#\a #\b #\c) x))
                                     (lambda (x) (+ x 1))
                                     0)))

(test #t (char-set= (char-set #\a #\b #\c #\d)
                    (char-set-unfold! (lambda (x) (= x 3))
                                      (lambda (x) (vector-ref '#(#\a #\b #\c) x))
                                      (lambda (x) (+ x 1))
                                      0
                                      (char-set #\d))))

;; char-set-for-each
(test '(#\a #\b) (let ([lst '()])
                   (char-set-for-each (lambda (c) (set! lst (cons c lst)))
                                      (char-set #\a #\b))
                   ;; Sorting since iteration order depends on internal structure
                   (if (char<? (car lst) (cadr lst)) lst (reverse lst))))

;; char-set-map
(test #t (char-set= (char-set #\b #\c #\d) 
                    (char-set-map (lambda (c) (integer->char (+ 1 (char->integer c)))) 
                                  (char-set #\a #\b #\c))))


;;; === 5. Copy and Constructors ===

(test #t (char-set= (char-set #\a) (char-set-copy (char-set #\a))))

(test #t (char-set= (char-set #\a #\b) (list->char-set '(#\a #\b))))
(test #t (char-set= (char-set #\a #\b #\c) (list->char-set! '(#\a #\b) (char-set #\c))))

(test #t (char-set= (char-set #\a #\b) (string->char-set "ab")))
(test #t (char-set= (char-set #\a #\b #\c) (string->char-set! "ab" (char-set #\c))))


;;; === 6. Filtering and Ranges ===

;; char-set-filter
(test #t (char-set= (char-set #\a #\c) 
                    (char-set-filter (lambda (c) (not (char=? c #\b))) 
                                     (char-set #\a #\b #\c))))
(test #t (char-set= (char-set #\a #\c #\d) 
                    (char-set-filter! (lambda (c) (not (char=? c #\b))) 
                                      (char-set #\a #\b #\c) 
                                      (char-set #\d))))

;; ucs-range->char-set
(test #t (char-set= (char-set #\a #\b #\c) (ucs-range->char-set 97 100)))
(test #t (char-set= (char-set #\a #\b #\c #\d) (ucs-range->char-set! 97 100 #f (char-set #\d))))

;; ->char-set coercion
(test #t (char-set= (char-set #\a) (->char-set (char-set #\a))))
(test #t (char-set= (char-set #\a) (->char-set #\a)))
(test #t (char-set= (char-set #\a #\b) (->char-set "ab")))


;;; === 7. Size, Count and Membership ===

(test 0 (char-set-size char-set:empty))
(test 3 (char-set-size (char-set #\a #\b #\c)))

(test 1 (char-set-count (lambda (c) (char=? c #\a)) (char-set #\a #\b #\a)))

(test #t (char-set-contains? (char-set #\a #\b) #\a))
(test #f (char-set-contains? (char-set #\a #\b) #\c))


;;; === 8. Quantifiers ===

(test #t (char-set-every char-lower-case? (char-set #\a #\b #\c)))
(test #f (char-set-every char-lower-case? (char-set #\a #\B #\c)))

(test #t (char-set-any char-upper-case? (char-set #\a #\B #\c)))
(test #f (char-set-any char-upper-case? (char-set #\a #\b #\c)))


;;; === 9. Conversion to List/String ===

(test #t (char-set= (char-set #\x #\y) (list->char-set (char-set->list (char-set #\x #\y)))))
(test #t (char-set= (char-set #\x #\y) (string->char-set (char-set->string (char-set #\x #\y)))))


;;; === 10. Element Modification ===

(test #t (char-set= (char-set #\a #\b #\c) (char-set-adjoin (char-set #\a #\b) #\c)))
(test #t (char-set= (char-set #\a #\b #\c) (char-set-adjoin! (char-set #\a #\b) #\c)))

(test #t (char-set= (char-set #\a #\b) (char-set-delete (char-set #\a #\b #\c) #\c)))
(test #t (char-set= (char-set #\a #\b) (char-set-delete! (char-set #\a #\b #\c) #\c)))


;;; === 11. Set Algebra ===

(test #t (char-set= char-set:empty (char-set-complement char-set:full)))
(test #t (char-set= char-set:empty (char-set-complement! char-set:full)))

(test #t (char-set= (char-set #\a #\b #\c) (char-set-union (char-set #\a) (char-set #\b) (char-set #\c))))
(test #t (char-set= (char-set #\a #\b #\c) (char-set-union! (char-set #\a) (char-set #\b) (char-set #\c))))

(test #t (char-set= (char-set #\b) (char-set-intersection (char-set #\a #\b) (char-set #\b #\c))))
(test #t (char-set= (char-set #\b) (char-set-intersection! (char-set #\a #\b) (char-set #\b #\c))))

(test #t (char-set= (char-set #\a) (char-set-difference (char-set #\a #\b) (char-set #\b #\c))))
(test #t (char-set= (char-set #\a) (char-set-difference! (char-set #\a #\b) (char-set #\b #\c))))

(test #t (char-set= (char-set #\a #\c) (char-set-xor (char-set #\a #\b) (char-set #\b #\c))))
(test #t (char-set= (char-set #\a #\c) (char-set-xor! (char-set #\a #\b) (char-set #\b #\c))))

(test #t (call-with-values 
           (lambda () (char-set-diff+intersection (char-set #\a #\b) (char-set #\b #\c)))
           (lambda (diff inter)
             (and (char-set= diff (char-set #\a))
                  (char-set= inter (char-set #\b))))))

(test #t (call-with-values 
           (lambda () (char-set-diff+intersection! (char-set #\a #\b) (char-set #\b #\c)))
           (lambda (diff inter)
             (and (char-set= diff (char-set #\a))
                  (char-set= inter (char-set #\b))))))


;;; === 12. Standard Predefined Sets (ASCII Boundaries) ===

(test #t (char-set-contains? char-set:lower-case #\a))
(test #f (char-set-contains? char-set:lower-case #\A))

(test #t (char-set-contains? char-set:upper-case #\A))
(test #f (char-set-contains? char-set:upper-case #\a))

(test #t (char-set-contains? char-set:digit #\3))
(test #f (char-set-contains? char-set:digit #\a))

(test #t (char-set-contains? char-set:hex-digit #\f))
(test #t (char-set-contains? char-set:hex-digit #\F))
(test #f (char-set-contains? char-set:hex-digit #\g))

(test #t (char-set-contains? char-set:whitespace #\space))
(test #t (char-set-contains? char-set:whitespace #\tab))

(test #t (char-set-contains? char-set:blank #\space))
(test #f (char-set-contains? char-set:blank #\newline))

(test #t (char-set-contains? char-set:iso-control (integer->char #x00)))
(test #t (char-set-contains? char-set:iso-control (integer->char #x7f)))

(test #t (char-set-contains? char-set:punctuation #\.))
(test #t (char-set-contains? char-set:symbol #\+))

(test #t (char-set-contains? char-set:ascii (integer->char 127)))
(test #f (char-set-contains? char-set:ascii (integer->char 128)))


;;; === 13. Unicode-Specific Tests ===

(cond-expand
  [full-unicode
   ;; Micro sign (U+00B5) is lower-case
   (test #t (char-set-contains? char-set:lower-case (integer->char #xb5)))
   
   ;; Latin Capital Letter A with Grave (U+00C0) is upper-case
   (test #t (char-set-contains? char-set:upper-case (integer->char #xc0)))
   
   ;; Latin Capital Letter Dz with Caron (U+01C5) is title-case
   (test #t (char-set-contains? char-set:title-case (integer->char #x1c5)))
   
   ;; Arabic-Indic Digit Zero (U+0660) is a digit
   (test #t (char-set-contains? char-set:digit (integer->char #x660)))
   
   ;; High range character check (U+10002) in ucs-range->char-set
   (test #t (char-set-contains? (ucs-range->char-set #x10000 #x10005) (integer->char #x10002)))
   
   ;; Ensure high plane character is present in char-set:full
   (test #t (char-set-contains? char-set:full (integer->char #x10000)))]
  [else])


(define (run-fuzzy-test number-of-iterations)
  (let ([state 314159] [tap_mask 32] [high_bit 4194304])
    (define (next-random)
      (let* ([bit0 (modulo state 2)]
             [bit5 (modulo (quotient state tap_mask) 2)]
             [new-bit (modulo (+ bit0 bit5) 2)])
        (set! state (+ (quotient state 2) (* new-bit high_bit)))
        state))
    (define (random-int n) (modulo (next-random) n))
    
    ;; Set bounds based on implementation capabilities
    (define max-val
      (cond-expand
        [full-unicode 5000]
        [else 256]))
        
    (define max-ranges 10)
    (define universe (ucs-range->char-set 0 max-val))

    ;; Generate a random char-set with up to max-ranges intervals
    (define (random-char-set max-val max-ranges)
      (let loop ([n (+ 1 (random-int max-ranges))] [acc char-set:empty])
        (if (zero? n)
            acc
            (let* ([p1 (random-int max-val)]
                   [p2 (random-int max-val)]
                   [low (min p1 p2)]
                   [high (max p1 p2)])
              ;; ucs-range->char-set is upper-bound exclusive, so we add 1
              (loop (- n 1)
                    (char-set-union acc (ucs-range->char-set low (+ high 1))))))))
    
    ;; Run tests
    (display "Running character set fuzzy tests...")
    (newline)
    
    (let test-loop ([i 0])
      (when (< i number-of-iterations)
        (let ([s1 (random-char-set max-val max-ranges)]
              [s2 (random-char-set max-val max-ranges)]
              [s3 (random-char-set max-val max-ranges)])
          
          ;; De Morgan's Laws
          ;; not(A ∪ B) = not(A) ∩ not(B)
          (test (char-set-intersection (char-set-difference universe s1)
                                       (char-set-difference universe s2))
                (char-set-difference universe (char-set-union s1 s2)))
          
          ;; not(A ∩ B) = not(A) ∪ not(B)
          (test (char-set-union (char-set-difference universe s1)
                                (char-set-difference universe s2))
                (char-set-difference universe (char-set-intersection s1 s2)))
          
          ;; Commutative Laws
          (test (char-set-union s1 s2)
                (char-set-union s2 s1))
          
          (test (char-set-intersection s1 s2)
                (char-set-intersection s2 s1))
          
          ;; Associative Laws
          (test (char-set-union (char-set-union s1 s2) s3)
                (char-set-union s1 (char-set-union s2 s3)))
          
          (test (char-set-intersection (char-set-intersection s1 s2) s3)
                (char-set-intersection s1 (char-set-intersection s2 s3)))
          
          ;; Distributive Laws
          (test (char-set-union s1 (char-set-intersection s2 s3))
                (char-set-intersection (char-set-union s1 s2) (char-set-union s1 s3)))
          
          (test (char-set-intersection s1 (char-set-union s2 s3))
                (char-set-union (char-set-intersection s1 s2) (char-set-intersection s1 s3)))
          
          ;; Identity Laws
          (test s1 (char-set-union s1 char-set:empty))
          (test s1 (char-set-union char-set:empty s1))
          (test char-set:empty (char-set-intersection s1 char-set:empty))
          (test char-set:empty (char-set-intersection char-set:empty s1))
          
          ;; Idempotent Laws
          (test s1 (char-set-union s1 s1))
          (test s1 (char-set-intersection s1 s1))
          
          ;; Complement Laws (with universe)
          (test universe
                (char-set-union s1 (char-set-difference universe s1)))
          (test char-set:empty
                (char-set-intersection s1 (char-set-difference universe s1)))
          
          ;; Absorption Laws
          (test s1 (char-set-union s1 (char-set-intersection s1 s2)))
          (test s1 (char-set-intersection s1 (char-set-union s1 s2)))
          
          ;; Difference Laws
          (test char-set:empty (char-set-difference s1 s1))
          (test s1 (char-set-difference s1 char-set:empty))
          (test (char-set-intersection s1 (char-set-difference s2 s3))
                (char-set-difference (char-set-intersection s1 s2) s3))
          
          ;; Subset Laws
          (test #t (char-set<= char-set:empty s1))
          (test #t (char-set<= s1 s1))
          (test #t (char-set<= (char-set-intersection s1 s2) s1))
          (test #t (char-set<= (char-set-intersection s1 s2) s2))
          (test #t (char-set<= s1 (char-set-union s1 s2)))
          (test #t (char-set<= s2 (char-set-union s1 s2)))
          
          ;; If A <= B then A \/ B = B
          (when (char-set<= s1 s2)
            (test s2 (char-set-union s1 s2)))
          
          ;; If A <= B then A /\ B = A
          (when (char-set<= s1 s2)
            (test s1 (char-set-intersection s1 s2)))
          
          ;; Disjointness properties
          (test (char-set= (char-set-intersection s1 s2) char-set:empty)
                (char-set= (char-set-intersection s1 s2) char-set:empty))
          
          (test (not (char-set= (char-set-difference s1 s2) char-set:empty))
                (not (char-set<= s1 s2)))
          
          ;; Size properties
          (test #t (<= (char-set-size (char-set-intersection s1 s2))
                       (min (char-set-size s1) (char-set-size s2))))
          
          (test #t (>= (char-set-size (char-set-union s1 s2))
                       (max (char-set-size s1) (char-set-size s2))))
          
          (test (+ (char-set-size s1) (char-set-size s2))
                (+ (char-set-size (char-set-union s1 s2))
                   (char-set-size (char-set-intersection s1 s2))))
          
          ;; Equality properties
          (test #t (char-set= s1 s1))
          (test (char-set= s1 s2) (char-set= s2 s1))
          (when (and (char-set= s1 s2) (char-set= s2 s3))
            (test #t (char-set= s1 s3)))
          
          ;; Round-trip property
          (test s1 (list->char-set (char-set->list s1)))
          
          ;; Membership consistency
          (let* ([test-int (random-int max-val)]
                 [test-char (integer->char test-int)])
            (test (char-set-contains? s1 test-char)
                  (not (not (member test-char (char-set->list s1)))))
            
            ;; If element in intersection, it's in both sets
            (when (char-set-contains? (char-set-intersection s1 s2) test-char)
              (test #t (char-set-contains? s1 test-char))
              (test #t (char-set-contains? s2 test-char)))
            
            ;; If element in union, it's in at least one set
            (when (char-set-contains? (char-set-union s1 s2) test-char)
              (test #t (or (char-set-contains? s1 test-char)
                           (char-set-contains? s2 test-char))))
            
            ;; If element in difference s1-s2, it's in s1 but not s2
            (when (char-set-contains? (char-set-difference s1 s2) test-char)
              (test #t (char-set-contains? s1 test-char))
              (test #f (char-set-contains? s2 test-char))))
          
          (test-loop (+ i 1))))))
  
  (display "Character set fuzzy tests completed.")
  (newline))

;; Run the fuzzy tester
(run-fuzzy-test 20)

(cond-expand
  [full-unicode
   (let* ([init-set char-set:letter]
          ;; Step 1: Filter characters in odd 256-code-point blocks (e.g., [256, 511], [768, 1023])
          [filtered (char-set-filter 
                     (lambda (c) 
                       (odd? (quotient (char->integer c) 256)))
                     init-set)]
          ;; Step 2: Map each character to the character right before it (code - 1)
          [mapped-prev (char-set-map 
                        (lambda (c) 
                          (integer->char (- (char->integer c) 1)))
                        filtered)]
          ;; Step 3: Map each character back to the next character (code + 1)
          [mapped-back (char-set-map 
                        (lambda (c) 
                          (integer->char (+ (char->integer c) 1)))
                        mapped-prev)])
     ;; Step 4: Compare results
     (test #t (char-set= filtered mapped-back)))]
  [else])

(test-end)
