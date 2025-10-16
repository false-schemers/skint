;;
;; R7RS and Skint tests (OPT_UNICODE)
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

(test-begin "r7rs-unicode")

(test 65 (char->integer #\A))
(test #\A (integer->char 65))
(test #\λ (integer->char 955))          ; Greek lambda

(test #t (char=? #\λ #\λ))
(test #f (char=? #\A #\λ))


(test "" (string))
(test 0 (string-length (string)))
(test 3 (string-length "λμν"))

(test "λλλ" (make-string 3 #\λ))
(test "" (make-string 0 #\λ))

(test '(#\H #\e #\λ #\λ #\o) (string->list "Heλλo"))
(test "Heλλo" (list->string '(#\H #\e #\λ #\λ #\o)))

(test "AλB" (string-append "A" "λ" "B"))
(test "" (string-append))

(test "Heλlo" (string-copy "Heλlo"))
(test "λ"   (string-copy "λμν" 0 1))

(let ((s (string-copy "cat")))
  (string-set! s 1 #\λ)
  (test "cλt" s))

(test #t (string=? "λμν" "λμν"))
(test #f (string=? "λμν" "abc"))

(test #t (string<? "abc" "λμν"))
(test #f (string<? "λμν" "abc"))
(test #t (string<=? "abc" "abc"))
(test #t (string<=? "abc" "λμν"))

(test #t (string>? "λμν" "abc"))
(test #f (string>? "abc" "λμν"))
(test #t (string>=? "λμν" "λμν"))
(test #t (string>=? "λμν" "abc"))

(test 'λ (string->symbol "λ"))
(test "λ" (symbol->string 'λ))

(test '(206 187) (string->utf8 "λ"))
(test "λ" (utf8->string #u8(206 187)))

(test #t (string<? "α" "β"))            ; Greek alpha < beta
(test #f (string<? "β" "α"))
(test #t (string<=? "α" "α"))
(test #t (string<=? "α" "β"))

(test #t (string<? "Б" "В"))            ; Cyrillic BE < VE
(test #f (string<? "В" "Б"))
(test #t (string>=? "Ж" "Б"))
(test #f (string>=? "Б" "Ж"))

(test #t (string<? "你" "好"))           ; CJK ideographs
(test #f (string<? "好" "你"))
(test #t (string<=? "你" "你"))

(test #t (string<? "🌈" "🌟"))          ; 4-byte emoji
(test #f (string<? "🌟" "🌈"))
(test #t (string>=? "🌟" "🌈"))
(test #t (string>? "🌟" "🌈"))

;; 2-byte UTF-8
(let ((s (string-copy "ab")))
  (string-set! s 1 #\λ)                 ; λ = CE BB
  (test "aλ" s))

;; 3-byte UTF-8
(let ((s (string-copy "xy")))
  (string-set! s 0 #\你)                ; 你 = E4 BD A0
  (test "你y" s))

;; 4-byte UTF-8
(let ((s (string-copy "12")))
  (string-set! s 1 #\🌈)                ; 🌈 = F0 9F 8C 88
  (test "1🌈" s))

;; overwrite in longer string
(let ((s (string-copy "AБВГ")))
  (string-set! s 2 #\Ж)                 ; replace В with Ж
  (test "AБЖГ" s))


;;; More string->utf8 / utf8->string + lexicographical order tests

;; ----- extra string->utf8 -----
(test #u8()        (string->utf8 ""))
(test #u8(65)      (string->utf8 "A"))
(test #u8(206 187) (string->utf8 "λ"))            ; 2-byte
(test #u8(228 189 160) (string->utf8 "你"))       ; 3-byte
(test #u8(240 159 140 136) (string->utf8 "🌈"))   ; 4-byte

(test '(65 206 187 240 159 140 136)
      (string->utf8 "Aλ🌈"))                   ; mixed

;; ----- extra utf8->string -----
(test ""  (utf8->string #u8()))
(test "A" (utf8->string #u8(65)))
(test "λ" (utf8->string #u8(206 187)))
(test "你" (utf8->string #u8(228 189 160)))
(test "🌈" (utf8->string #u8(240 159 140 136)))
(test "Aλ🌈" (utf8->string #u8(65 206 187 240 159 140 136)))

;; ----- lexicographical order (unequal-length strings) -----
(test #t (string<? "" "α"))             ; empty < anything
(test #f (string<? "α" ""))

(test #t (string<? "α" "αβ"))           ; proper prefix is smaller
(test #f (string<? "αβ" "α"))
(test #t (string<=? "α" "αβ"))
(test #f (string<=? "αβ" "α"))

(test #t (string<? "abc" "абв"))        ; Latin < Cyrillic
(test #f (string<? "абв" "abc"))

(test #t (string<? "你好" "你好吗"))     ; shorter CJK string first
(test #f (string<? "你好吗" "你好"))
(test #t (string>=? "你好吗" "你好"))
(test #f (string>=? "你好" "你好吗"))

(test #t (string<? "🌈" "🌈🌟"))        ; emoji prefix test
(test #f (string<? "🌈🌟" "🌈"))

(test-end)
