; gencsets14.scm -- script to generate ivs charsets for SRFI-14

(import 
 (scheme base) (scheme read) (scheme write)
 (skint ivset) 
 (srfi 257) (srfi 257 rx)
 (srfi 272 advanced))
 
; download standard Unicode char property DB before use
(define ud-path "tmp/UnicodeData.txt") 

(define table
(let ([p (open-input-file ud-path)])
  (define (add-cp cp flags l)
    (let ([prev (car l)])
      (if (= cp (+ (cadr prev) 1))
          (cond [(eqv? flags (caddr prev)) 
                 (set-car! (cdr prev) cp) l]
                [(and (eqv? (caddr prev) 'l) (= cp (+ (car prev) 1)) (eqv? flags 'u))
                 (set-car! (cddr prev) 'lu) (set-car! (cdr prev) cp) l]
                [(and (eqv? (caddr prev) 'u) (= cp (+ (car prev) 1)) (eqv? flags 'l))
                 (set-car! (cddr prev) 'ul) (set-car! (cdr prev) cp) l]
                [(and (eqv? (caddr prev) 'lu) (even? (- cp (car prev))) (eqv? flags 'l))
                 (set-car! (cdr prev) cp) l]
                [(and (eqv? (caddr prev) 'lu) (odd? (- cp (car prev))) (eqv? flags 'u))
                 (set-car! (cdr prev) cp) l]
                [(and (eqv? (caddr prev) 'ul) (even? (- cp (car prev))) (eqv? flags 'u))
                 (set-car! (cdr prev) cp) l]
                [(and (eqv? (caddr prev) 'ul) (odd? (- cp (car prev))) (eqv? flags 'l))
                 (set-car! (cdr prev) cp) l]
                [(and (eqv? (caddr prev) 0) (= cp (+ (car prev) 1)) (eqv? flags 1))
                 (set-car! (cddr prev) 'd) (set-car! (cdr prev) cp) l]
                [(and (eqv? (caddr prev) 'd) (eqv? (- cp (car prev)) flags))
                 (set-car! (cdr prev) cp) l]
                [else 
                 (cons (list cp cp flags) l)])
          (cons (list cp cp flags)
                (cons (list (+ (cadr prev) 1) (- cp 1) '?) l)))))
  (let loop ([l '((0 0 ?))])
    (match (read-line p)
      [(~? eof-object?) (reverse l)]
      [(~/split ";" (~list* (~number->string cp 16) _ (~symbol->string cat) _ _ _ (~number->string dv 10) _ _ _ _ _))
       ;(write cp) (display " ") (write cat) (newline)
       (case cat
         [(Lu) (loop (add-cp cp 'u l))]
         [(Ll) (loop (add-cp cp 'l l))]
         [(Lt) (loop (add-cp cp 't l))]
         [(Nd) (loop (add-cp cp dv l))]
         ;[(Zs Zl Zp) (loop (add-cp cp 's l))] ; all whitespace
         [(Zs) (loop (add-cp cp 'h l))] ; horizontal whitespace (add \t later)
         [(Zl Zp) (loop (add-cp cp 'w l))] ; other whitespace
         [(Sm Sc Sk So) (loop (add-cp cp 'y l))] ; symbolic
         [(Pc Pd Ps Pe Pi Pf Po) (loop (add-cp cp 'p l))] ; punctuation
         [else (loop l)])]
      [else (loop l)]))))

;(write table)

(define (write-ivs name ivs)
  (define ivs8 (ivs-intersection ivs #(0 255)))
  (if (ivs=? ivs ivs8)
      (pp `(define ,name ,ivs) 
        pp-code #t pp-radix 16 pp-width 100)
      (pp `(define ,name
              (cond-expand 
              [full-unicode ,ivs]
              [else ,ivs8]))
        pp-code #t pp-radix 16 pp-width 100 pp-brackets #t))
  (newline))

(define (write-selected name sym)
  (let loop ([t table] [l '()])
    (cond [(and (null? t) (eq? sym 'h)) ; add #\t
           (write-ivs name (list->vector (cons 9 (cons 9 (reverse l)))))]
          [(null? t) 
           (write-ivs name (list->vector (reverse l)))]
          [(eq? (caddr (car t)) sym)
           (loop (cdr t) (cons (cadr (car t)) (cons (car (car t)) l)))]
          [else (loop (cdr t) l)])))

(define codepoint-count 
  (cond-expand 
    [full-unicode #x110000] 
    [else #x100]))

(define ivs:full
  (cond-expand 
    [full-unicode  
     (let () ; all code points except non-characters
       (define bmp-ranges ; Plane 0 (BMP)
         (list (cons #x0000 #xD7FF)   ; ASCII, controls, symbols, up to surrogates
               (cons #xE000 #xFDCF)   ; Surrogates omitted; ends before BMP non-characters
               (cons #xFDF0 #xFEFE)   ; BMP non-characters omitted; ends before BOM
               (cons #xFF00 #xFFFD))) ; BOM omitted; ends before plane 0 non-characters
       (define (make-plane-ranges plane) ; valid ranges for planes 1 through 16 (#x10)
         (if (> plane #x10) '()
             (let* ([start (* plane #x10000)]
                   [end (+ start #xFFFD)]) ; Last 2 code points of a plane are non-chars
               (cons (cons start end)
                     (make-plane-ranges (+ plane 1))))))
       (ivl->ivs (append bmp-ranges (make-plane-ranges 1))))]
    [else #(#x00 #xFF)]))

(define (char-filter char-pred)
  (lambda (i) (and (ivs-contains? ivs:full i) (char-pred (integer->char i)))))

(define (write-calculated name pred)
  (define ivs (predicate->ivs (char-filter pred) 0 codepoint-count))
  (write-ivs name ivs))

(write-ivs 'ivs:empty #())
(write-ivs 'ivs:full ivs:full)
(write-calculated 'ivs:lower-case char-lower-case?)
(write-calculated 'ivs:upper-case char-upper-case?)
(write-selected 'ivs:title-case 't)
(write-calculated 'ivs:letter char-alphabetic?)
(write-calculated 'ivs:digit char-numeric?)
(write-ivs 'ivs:hex-digit #(#x30 #x39 #x41 #x46 #x61 #x66))
(pp '(define ivs:letter+digit (ivs-union ivs:letter ivs:digit)) 
    pp-code #t pp-radix 16 pp-width 80) (newline)
(write-ivs 'ivs:hex-digit #(#x30 #x39 #x41 #x46 #x61 #x66))
(write-selected 'ivs:blank 'h)
(write-calculated 'ivs:whitespace char-whitespace?)
(write-selected 'ivs:punctuation 'p)
(write-selected 'ivs:symbol 'y)
(pp '(define ivs:graphic
       (ivs-union ivs:letter+digit (ivs-union ivs:punctuation ivs:symbol)))
     pp-code #t pp-radix 16 pp-width 80) (newline)
(pp '(define ivs:printing
       (ivs-union ivs:graphic ivs:whitespace))
     pp-code #t pp-radix 16 pp-width 80) (newline)
(write-ivs 'ivs:iso-control #(#x00 #x1F #x7F #x9F))
(write-ivs 'ivs:ascii #(#x00 #x7F))

