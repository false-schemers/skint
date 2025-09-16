
;;; [esl] adapted from the reference implementation by Wolfgang Corcoran-Mathe

;;; SPDX-FileCopyrightText: 2020 Wolfgang Corcoran-Mathe <wcm@sigwinch.xyz>
;;;
;;; SPDX-License-Identifier: MIT

(define-library (srfi 178)
  (import (scheme base))
  (import (scheme case-lambda))
  (import (scheme cxr))
  (import (only (srfi 143) 
    fxlength fxbit-set? fxarithmetic-shift))
  (import (only (skint)
    fixnum? fxnegative? fxzero? fx=? fx<=? fx>=? fx<? fx>? fx+ fx- fxmin fxand fxior fxxor
    bytevector=? bytevector->list make-numvector numvector? numvector-length 
    numvector-ref numvector-set! list->numvector))

  (export 
    ;; Bit conversion
    bit->integer bit->boolean  
    
    ;; Constructors
    make-bitvector bitvector bitvector-unfold
    bitvector-unfold-right bitvector-copy
    bitvector-reverse-copy bitvector-append bitvector-concatenate
    bitvector-append-subbitvectors

    ;; Predicates
    bitvector? bitvector-empty? bitvector=?

    ;; Selectors
    bitvector-ref/int bitvector-ref/bool bitvector-length

    ;; Iteration
    bitvector-take bitvector-take-right
    bitvector-drop bitvector-drop-right bitvector-segment
    bitvector-fold/int bitvector-fold/bool bitvector-fold-right/int
    bitvector-fold-right/bool bitvector-map/int bitvector-map/bool
    bitvector-map!/int bitvector-map!/bool bitvector-map->list/int
    bitvector-map->list/bool bitvector-for-each/int
    bitvector-for-each/bool

    ;; Prefixes, suffixes, trimming, padding
    bitvector-prefix-length
    bitvector-suffix-length bitvector-prefix?  bitvector-suffix?
    bitvector-pad bitvector-pad-right bitvector-trim
    bitvector-trim-right bitvector-trim-both

    ;; Mutators
    bitvector-set!
    bitvector-swap! bitvector-reverse!
    bitvector-copy! bitvector-reverse-copy!

    ;; Conversion
    bitvector->list/int
    bitvector->list/bool reverse-bitvector->list/int
    reverse-bitvector->list/bool list->bitvector
    reverse-list->bitvector bitvector->vector/int
    bitvector->vector/bool vector->bitvector bitvector->string
    string->bitvector bitvector->integer integer->bitvector
    reverse-vector->bitvector reverse-bitvector->vector/int
    reverse-bitvector->vector/bool

    ;; Generators and accumulators
    make-bitvector/int-generator make-bitvector/bool-generator
    make-bitvector-accumulator

    ;; Basic operations
    bitvector-not bitvector-not!
    bitvector-and bitvector-and!  bitvector-ior bitvector-ior!
    bitvector-xor bitvector-xor!  bitvector-eqv bitvector-eqv!
    bitvector-nand bitvector-nand!  bitvector-nor bitvector-nor!
    bitvector-andc1 bitvector-andc1!  bitvector-andc2
    bitvector-andc2!  bitvector-orc1 bitvector-orc1!
    bitvector-orc2 bitvector-orc2!

    ;; Quasi-integer operations
    bitvector-logical-shift
    bitvector-count bitvector-if
    bitvector-first-bit bitvector-count-run

    ;; Bit field operations
    bitvector-field-any?  bitvector-field-every?
    bitvector-field-clear bitvector-field-clear!
    bitvector-field-set bitvector-field-set!
    bitvector-field-replace-same bitvector-field-replace-same!
    bitvector-field-rotate bitvector-field-flip
    bitvector-field-flip!
    bitvector-field-replace bitvector-field-replace!)

(begin

;;; [esl] all ref. imp's u8-vector-based wrappers are replaced with
;;; Skint-specific numvector-based variants; some algorithms come from
;;; John Cowan's SRFI 160 (MIT-licensed, see 160.sld)

(define-syntax u1 32) ; internal skint's numvec type for bitvectors

(define-syntax bitvector-check
  (syntax-rules ()
    [(_ exp)
     (let ([t u1] [x exp]) 
       (cond [(numvector? x t) x]
             [else (error "invalid bitvector" x)]))]))

(define (bitvector? x) (numvector? x u1))
(define (bitvector-length x) (numvector-length (bitvector-check x)))
(define (bitvector-empty? x) (fxzero? (numvector-length (bitvector-check x))))
(define (bitvector-ref x i) (numvector-ref (bitvector-check x) i))
(define (bitvector-set! x i n) (numvector-set! (bitvector-check x) i n))
(define (list->bitvector l) (list->numvector l u1))
(define (bitvector . l) (list->numvector l u1))

(define (bitvector-swap! vec i j)
  (let ((ival (bitvector-ref vec i))
        (jval (bitvector-ref vec j)))
    (bitvector-set! vec i jval)
    (bitvector-set! vec j ival)))

(define (bitvector=? . bvecs)
  ; todo: make sure all bvecs are bitvectors!
  (apply bytevector=? bvecs)) ; Skint's bytevector=? compares subtypes and contents

(define make-bitvector
  (case-lambda
    [(len) (make-numvector u1 len)]
    [(len init)
     (let ([v (make-numvector u1 len)] [init (bit->integer init)])
       (if (zero? init)
           v ; initialized by zeroes by default
           (do ([i 0 (+ i 1)]) [(>= i len) v] (numvector-set! v i init))))]))

(define (numvector->list v start end)
  (let loop ([i (- end 1)] [l '()])
    (if (< i start) l (loop (- i 1) (cons (numvector-ref v i) l)))))

(define bitvector->list
  (case-lambda
    [(v) (numvector->list v 0 (numvector-length (bitvector-check v)))]
    [(v start) (numvector->list v start (numvector-length (bitvector-check v)))]
    [(v start end) (numvector->list (bitvector-check v) start end)]))

(define (%bitvector-unfold f len) ; seedless variant
  (let ([v (make-bitvector len)])
    (let loop ([i 0])
      (cond [(= i len) v]
            [else (bitvector-set! v i (f i)) (loop (+ i 1))]))))

(define bitvector-ref/int 
  bitvector-ref)

(define (bitvector-ref/bool v i)
  (= (bitvector-ref v i) 1))

(define (subbitvector-copy! to at from start end)
  (let ([limit (fxmin end (fx+ start (fx- (bitvector-length to) at)))])
    (if (fx<=? at start)
        (do ([i at (fx+ i 1)] [j start (fx+ j 1)]) 
          [(fx>=? j limit)]
          (bitvector-set! to i (bitvector-ref from j)))
        (do ([i (fx+ at (fx- (fx- end start) 1)) (fx- i 1)] [j (fx- limit 1) (fx- j 1)])
          [(fx<? j start)]
          (bitvector-set! to i (bitvector-ref from j))))))

(define bitvector-copy!
  (case-lambda
     [(to at from) (subbitvector-copy! to at from 0 (bitvector-length from))]
     [(to at from start) (subbitvector-copy! to at from start (bitvector-length from))]
     [(to at from start end) (subbitvector-copy! to at from start end)]))

(define (subbitvector vec start end)  ; TODO: %vsub?
  (let ([v (make-bitvector (fx- end start))])
    (subbitvector-copy! v 0 vec start end)
    v))

(define bitvector-copy
  (case-lambda
     [(vec) (subbitvector vec 0 (bitvector-length vec))] ; TODO: %vcpy ?
     [(vec start) (subbitvector vec start (bitvector-length vec))]
     [(vec start end) (subbitvector vec start end)]))

(define (subbitvector-fill! vec x start end)
  (do ([i start (fx+ i 1)]) [(fx>=? i end)] (bitvector-set! vec i x)))

(define bitvector-fill!
  (case-lambda
     [(vec x) (subbitvector-fill! vec x 0 (bitvector-length vec))]
     [(vec x start) (subbitvector-fill! vec x start (bitvector-length vec))]
     [(vec x start end) (subbitvector-fill! vec x start end)]))

(define (%bitvectors-sum-length vecs)
  (let loop ([vecs vecs] [l 0])
    (if (null? vecs) l (loop (cdr vecs) (fx+ l (bitvector-length (car vecs)))))))

(define (%bitvectors-copy-into! to vecs)
  (let loop ([vecs vecs] [i 0])
    (if (null? vecs)
        to
        (let ([vec (car vecs)] [vecs (cdr vecs)])
          (let ([len (bitvector-length vec)])
            (subbitvector-copy! to i vec 0 len)
            (loop vecs (fx+ i len)))))))  

(define (bitvector-append . vecs)
  (%bitvectors-copy-into! (make-bitvector (%bitvectors-sum-length vecs)) vecs))

(define (bitvector-concatenate vecs)
  (%bitvectors-copy-into! (make-bitvector (%bitvectors-sum-length vecs)) vecs))

(define (%subbitvectors-sum-length args)
  (if (null? args) 0
    (+ (- (caddr args) (cadr args))
       (%subbitvectors-sum-length (cdddr args)))))

(define (bitvector-append-subbitvectors . args)
  (let ([v (make-bitvector (%subbitvectors-sum-length args))])
    (let loop ([args args] [at 0])
      (unless (null? args)
        (let ([vec (car args)] [start (cadr args)] [end (caddr args)])
          (bitvector-copy! v at vec start end)
          (loop (cdddr args) (+ at (- end start))))))
    v))

(define bitvector-reverse-copy
  (case-lambda
    ((vec) (bitvector-reverse-copy vec 0 (bitvector-length vec)))
    ((vec start) (bitvector-reverse-copy vec start (bitvector-length vec)))
    ((vec start end) 
     (let ((v (make-bitvector (- end start))))
      (bitvector-reverse-copy! v 0 vec start end)
      v))))

(define bitvector-reverse-copy!
  (case-lambda
    ((to at from)
     (bitvector-reverse-copy! to at from 0 (bitvector-length from)))
    ((to at from start)
     (bitvector-reverse-copy! to at from start (bitvector-length from)))
    ((to at from start end) 
     (let loop ((at at) (i (- end 1)))
       (unless (< i start)
         (bitvector-set! to at (bitvector-ref from i))
         (loop (+ at 1) (- i 1)))))))
    
(define (bitvector-take vec n)
  (let ((v (make-bitvector n)))
    (bitvector-copy! v 0 vec 0 n)
    v))

(define (bitvector-take-right vec n)
  (let ((v (make-bitvector n))
        (len (bitvector-length vec)))
    (bitvector-copy! v 0 vec (- len n) len)
    v))

(define (bitvector-drop vec n)
 (let* ((len (bitvector-length vec))
        (vlen (- len n))
        (v (make-bitvector vlen)))
    (bitvector-copy! v 0 vec n len)
    v))

(define (bitvector-drop-right vec n)
  (let* ((len (bitvector-length vec))
         (rlen (- len n))
         (v (make-bitvector rlen)))
    (bitvector-copy! v 0 vec 0 rlen)
    v))

(define (bitvector-segment vec n)
  (unless (and (fixnum? n) (positive? n))
    (error "length must be a positive integer" n))
  (let loop ((r '()) (i 0) (remain (bitvector-length vec)))
    (if (<= remain 0)
        (reverse r)
        (let ((size (min n remain)))
          (loop
            (cons (bitvector-copy vec i (+ i size)) r)
            (+ i size)
            (- remain size))))))

(define (%bitvectors-ref/int vecs i)
  (map (lambda (v) (bitvector-ref/int v i)) vecs))

(define (bitvector-fold/int kons knil vec . vecs)
  (if (null? vecs) ; fast path
      (let ((len (bitvector-length vec)))
        (let loop ((r knil) (i 0))
          (if (= i len)
              r
              (loop (kons r (bitvector-ref/int vec i)) (+ i 1)))))
      (let* ((vecs (cons vec vecs))
             (len (apply min (map bitvector-length vecs))))
        (let loop ((r knil) (i 0))
          (if (= i len)
              r
              (loop (apply kons r (%bitvectors-ref/int vecs i))
                    (+ i 1)))))))

(define (bitvector-fold-right/int kons knil vec . vecs)
  (if (null? vecs) ; fast path
      (let ((len (bitvector-length vec)))
        (let loop ((r knil) (i (- (bitvector-length vec) 1)))
          (if (fxnegative? i)
              r
              (loop (kons r (bitvector-ref/int vec i)) (- i 1)))))
      (let* ((vecs (cons vec vecs))
             (len (apply min (map bitvector-length vecs))))
        (let loop ((r knil) (i (- len 1)))
          (if (fxnegative? i)
              r
              (loop (apply kons r (%bitvectors-ref/int vecs i))
                    (- i 1)))))))

(define (%bitvectors-ref/bool vecs i)
  (map (lambda (v) (bitvector-ref/bool v i)) vecs))

(define (bitvector-fold/bool kons knil vec . vecs)
  (if (null? vecs) ; fast path
      (let ((len (bitvector-length vec)))
        (let loop ((r knil) (i 0))
          (if (= i len)
              r
              (loop (kons r (bitvector-ref/bool vec i)) (+ i 1)))))
      (let* ((vecs (cons vec vecs))
             (len (apply min (map bitvector-length vecs))))
        (let loop ((r knil) (i 0))
          (if (= i len)
              r
              (loop (apply kons r (%bitvectors-ref/bool vecs i))
                    (+ i 1)))))))

(define (bitvector-fold-right/bool kons knil vec . vecs)
  (if (null? vecs) ; fast path
      (let ((len (bitvector-length vec)))
        (let loop ((r knil) (i (- (bitvector-length vec) 1)))
          (if (fxnegative? i)
              r
              (loop (kons r (bitvector-ref/bool vec i)) (- i 1)))))
      (let* ((vecs (cons vec vecs))
             (len (apply min (map bitvector-length vecs))))
        (let loop ((r knil) (i (- len 1)))
          (if (fxnegative? i)
              r
              (loop (apply kons r (%bitvectors-ref/bool vecs i))
                    (- i 1)))))))

(define (bitvector-map/int f vec . vecs)
  (if (null? vecs) ; fast path
      (let* ((len (bitvector-length vec))
             (v (make-bitvector len)))
        (let loop ((i 0))
          (unless (= i len)
            (bitvector-set! v i (f (bitvector-ref/int vec i)))
            (loop (+ i 1))))
        v)
      (let* ((vecs (cons vec vecs))
             (len (apply min (map bitvector-length vecs)))
             (v (make-bitvector len)))
        (let loop ((i 0))
          (unless (= i len)
            (bitvector-set! v i (apply f (%bitvectors-ref/int vecs i)))
            (loop (+ i 1))))
        v)))

(define (bitvector-map!/int f vec . vecs)
  (if (null? vecs) ; fast path
      (let ((len (bitvector-length vec)))
        (let loop ((i 0))
          (unless (= i len)
            (bitvector-set! vec i (f (bitvector-ref/int vec i)))
            (loop (+ i 1)))))
      (let* ((vecs (cons vec vecs))
            (len (apply min (map bitvector-length vecs))))
        (let loop ((i 0))
          (unless (= i len)
            (bitvector-set! vec i (apply f (%bitvectors-ref/int vecs i)))
            (loop (+ i 1)))))))

(define (bitvector-for-each/int f vec . vecs)
  (if (null? vecs) ; fast path
      (let ((len (bitvector-length vec)))
        (let loop ((i 0))
          (unless (= i len)
            (f (bitvector-ref/int vec i))
            (loop (+ i 1)))))
      (let* ((vecs (cons vec vecs))
            (len (apply min (map bitvector-length vecs))))
        (let loop ((i 0))
          (unless (= i len)
            (apply f (%bitvectors-ref/int vecs i))
            (loop (+ i 1)))))))

(define (bitvector-map/bool f vec . vecs)
  (if (null? vecs) ; fast path
      (let* ((len (bitvector-length vec))
             (v (make-bitvector len)))
        (let loop ((i 0))
          (unless (= i len)
            (bitvector-set! v i (f (bitvector-ref/bool vec i)))
            (loop (+ i 1))))
        v)
      (let* ((vecs (cons vec vecs))
             (len (apply min (map bitvector-length vecs)))
             (v (make-bitvector len)))
        (let loop ((i 0))
          (unless (= i len)
            (bitvector-set! v i (apply f (%bitvectors-ref/bool vecs i)))
            (loop (+ i 1))))
        v)))

(define (bitvector-map!/bool f vec . vecs)
  (if (null? vecs) ; fast path
      (let ((len (bitvector-length vec)))
        (let loop ((i 0))
          (unless (= i len)
            (bitvector-set! vec i (f (bitvector-ref/bool vec i)))
            (loop (+ i 1)))))
      (let* ((vecs (cons vec vecs))
            (len (apply min (map bitvector-length vecs))))
        (let loop ((i 0))
          (unless (= i len)
            (bitvector-set! vec i (apply f (%bitvectors-ref/bool vecs i)))
            (loop (+ i 1)))))))

(define (bitvector-for-each/bool f vec . vecs)
  (if (null? vecs) ; fast path
      (let ((len (bitvector-length vec)))
        (let loop ((i 0))
          (unless (= i len)
            (f (bitvector-ref/bool vec i))
            (loop (+ i 1)))))
      (let* ((vecs (cons vec vecs))
            (len (apply min (map bitvector-length vecs))))
        (let loop ((i 0))
          (unless (= i len)
            (apply f (%bitvectors-ref/bool vecs i))
            (loop (+ i 1)))))))

(define bitvector-reverse!
  (case-lambda
    ((vec) (bitvector-reverse! vec 0 (bitvector-length vec)))
    ((vec start) (bitvector-reverse! vec start (bitvector-length vec)))
    ((vec start end)
     (let loop ((i start) (j (- end 1)))
       (when (< i j)
         (bitvector-swap! vec i j)
         (loop (+ i 1) (- j 1)))))))

(define bitvector-reverse-copy!
  (case-lambda
    ((to at from)
     (bitvector-reverse-copy! to at from 0 (bitvector-length from)))
    ((to at from start)
     (bitvector-reverse-copy! to at from start (bitvector-length from)))
    ((to at from start end) 
     (let loop ((at at) (i (- end 1)))
       (unless (< i start)
         (bitvector-set! to at (bitvector-ref from i))
         (loop (+ at 1) (- i 1)))))))

#;(define bitvector-reverse-copy
  (case-lambda
    ((vec) (bitvector-reverse-copy vec 0 (bitvector-length vec)))
    ((vec start) (bitvector-reverse-copy vec start (bitvector-length vec)))
    ((vec start end) 
     (let ((v (make-bitvector (- end start))))
       (bitvector-reverse-copy! v 0 vec start end)
       v))))

(define bitvector->list/int
  (case-lambda
    [(v) (numvector->list v 0 (numvector-length (bitvector-check v)))]
    [(v start) (numvector->list v start (numvector-length (bitvector-check v)))]
    [(v start end) (numvector->list (bitvector-check v) start end)]))

(define bitvector->list/bool
  (case-lambda
    [(v) (bitvector->list/bool v 0 (bitvector-length v))]
    [(v start) (bitvector->list/bool v start (bitvector-length v))]
    [(v start end) (map bit->boolean (numvector->list (bitvector-check v) start end))]))

(define reverse-bitvector->list/int
  (case-lambda
    ((vec) (reverse-bitvector->list/int vec 0 (bitvector-length vec)))
    ((vec start) (reverse-bitvector->list/int vec start (bitvector-length vec)))
    ((vec start end) 
     (let loop ((i start) (r '()))
       (if (= i end)
           r
           (loop (+ 1 i) (cons (bitvector-ref/int vec i) r)))))))

(define (reverse-list->bitvector list)
  (let* ((len (length list))
         (r (make-bitvector len)))
    (let loop ((i 0) (list list))
      (cond
        ((= i len) r)
        (else
          (bitvector-set! r (- len i 1) (car list))
          (loop (+ i 1) (cdr list)))))))

(define reverse-bitvector->list/bool
  (case-lambda
    ((vec) (reverse-bitvector->list/bool vec 0 (bitvector-length vec)))
    ((vec start) (reverse-bitvector->list/bool vec start (bitvector-length vec)))
    ((vec start end) 
     (let loop ((i start) (r '()))
       (if (= i end)
           r
           (loop (+ 1 i) (cons (bitvector-ref/bool vec i) r)))))))

(define bitvector->vector/int
  (case-lambda
    ((vec) (bitvector->vector/int vec 0 (bitvector-length vec)))
    ((vec start) (bitvector->vector/int vec start (bitvector-length vec)))
    ((vec start end) 
     (let* ((len (- end start)) (r (make-vector len)))
       (let loop ((i start) (o 0))
         (cond ((= i end) r)
               (else (vector-set! r o (bitvector-ref/int vec i))
                     (loop (+ i 1) (+ o 1)))))))))

(define bitvector->vector/bool
  (case-lambda
    ((vec) (bitvector->vector/bool vec 0 (bitvector-length vec)))
    ((vec start) (bitvector->vector/bool vec start (bitvector-length vec)))
    ((vec start end) 
     (let* ((len (- end start)) (r (make-vector len)))
       (let loop ((i start) (o 0))
         (cond ((= i end) r)
               (else (vector-set! r o (bitvector-ref/bool vec i))
                     (loop (+ i 1) (+ o 1)))))))))

(define vector->bitvector
  (case-lambda
    ((vec) (vector->bitvector vec 0 (vector-length vec)))
    ((vec start) (vector->bitvector vec start (vector-length vec)))
    ((vec start end) 
     (let* ((len (- end start)) (r (make-bitvector len)))
      (let loop ((i start) (o 0))
        (cond ((= i end) r)
              (else
                (bitvector-set! r o (vector-ref vec i))
                (loop (+ i 1) (+ o 1)))))))))


;; [esl] ref. imp's code continues from here; real bitvectors are used

;; The "seedless" case is all we need.
(define (vector-unfold f len)
  (let ((v (make-vector len)))
    (let lp ((i 0))
      (cond ((= i len) v)
            (else (vector-set! v i (f i))
                  (lp (+ i 1)))))))

;;;; Bit conversions

(define (bit->integer bit) 
  (case bit ((0 #f) 0) (else 1)))

(define (bit->boolean bit)
  (case bit ((0 #f) #f) (else #t)))

(define (bitvector->string bvec)
  (let loop ((i (- (bitvector-length bvec) 1))
             (r '()))
    (if (< i 0)
      (list->string (cons #\# (cons #\* r)))
      (loop (- i 1)
            (cons (if (bitvector-ref/bool bvec i) #\1 #\0) r)))))

(define (string->bitvector str)
  (call/cc
   (lambda (return)
     (and
       (> (string-length str) 1)
       (char=? (string-ref str 0) #\#)
       (char=? (string-ref str 1) #\*)
       (bitvector-unfold
        (lambda (ri si)
          (case (string-ref str si)
            ((#\0) (values 0 (+ si 1)))
            ((#\1) (values 1 (+ si 1)))
            (else (return #f))))
        (- (string-length str) 2)
        2)))))

;;;; Bitvector/integer conversions

; [esl] may overflow into flonums!
(define (bitvector->integer bvec) 
  (bitvector-fold-right/int (lambda (r b) (+ (* r 2) b)) 0 bvec))

; [esl] input limited to fixnums!
(define integer->bitvector 
  (case-lambda
    ((int) (integer->bitvector int (fxlength int)))
    ((int len)
     (bitvector-unfold
       (lambda (_ int) (values (fxbit-set? 0 int) (fxarithmetic-shift int -1)))
       len
       int))))

;;; Additional vector conversions

(define reverse-vector->bitvector
  (case-lambda
    ((vec) (reverse-vector->bitvector vec 0 (vector-length vec)))
    ((vec start) (reverse-vector->bitvector vec start (vector-length vec)))
    ((vec start end)
     (%bitvector-unfold (lambda (i) (vector-ref vec (- end 1 i)))
       (- end start)))))

(define reverse-bitvector->vector/int
  (case-lambda
    ((bvec)
     (reverse-bitvector->vector/int bvec 0 (bitvector-length bvec)))
    ((bvec start)
     (reverse-bitvector->vector/int bvec start (bitvector-length bvec)))
    ((bvec start end)
     (vector-unfold (lambda (i) (bitvector-ref bvec (- end 1 i)))
                    (- end start)))))

(define reverse-bitvector->vector/bool
  (case-lambda
    ((bvec)
     (reverse-bitvector->vector/bool bvec 0 (bitvector-length bvec)))
    ((bvec start)
     (reverse-bitvector->vector/bool bvec start (bitvector-length bvec)))
    ((bvec start end)
     (vector-unfold (lambda (i) (bitvector-ref/bool bvec (- end 1 i)))
                    (- end start)))))

;;; Fields

(define (bitvector-field-any? bvec start end)
  (let lp ((i start))
    (and (< i end)
         (or (bitvector-ref/bool bvec i)
             (lp (+ i 1))))))

(define (bitvector-field-every? bvec start end)
  (let lp ((i start))
    (or (>= i end)
        (and (bitvector-ref/bool bvec i)
             (lp (+ i 1))))))

(define (%bitvector-field-modify bvec bit start end)
  (bitvector-unfold
    (lambda (i)
      (if (and (>= i start) (< i end))
          bit
          (bitvector-ref/int bvec i)))
    (bitvector-length bvec)))

(define (bitvector-field-clear bvec start end)
  (%bitvector-field-modify bvec 0 start end))

(define (%bitvector-fill!/int bvec int start end)
  (do ([i start (fx+ i 1)]) [(fx>=? i end)] (bitvector-set! bvec i int))) ;[esl*]

(define (bitvector-field-clear! bvec start end)
  (%bitvector-fill!/int bvec 0 start end))

(define (bitvector-field-set bvec start end)
  (%bitvector-field-modify bvec 1 start end))

(define (bitvector-field-set! bvec start end)
  (%bitvector-fill!/int bvec 1 start end))

(define (bitvector-field-replace dest source start end)
  (bitvector-unfold
    (lambda (i)
      (if (and (>= i start) (< i end))
          (bitvector-ref/int source (- i start))
          (bitvector-ref/int dest i)))
    (bitvector-length dest)))

(define (bitvector-field-replace! dest source start end)
  (bitvector-copy! dest start source 0 (- end start)))

(define (bitvector-field-replace-same dest source start end)
  (bitvector-unfold
    (lambda (i) (bitvector-ref/int (if (and (>= i start) (< i end)) source dest) i))
    (bitvector-length dest)))

(define (bitvector-field-replace-same! dest source start end)
  (bitvector-copy! dest start source start end))

(define (bitvector-field-rotate bvec count start end)
  (if (zero? count)
      bvec
      (let ((field-len (- end start)))
        (bitvector-unfold
          (lambda (i)
            (if (and (>= i start) (< i end))
                (bitvector-ref/int bvec (+ start (floor-remainder (+ (- i start) count) field-len)))
                (bitvector-ref/int bvec i)))
         (bitvector-length bvec)))))

(define (bitvector-field-flip bvec start end)
  (bitvector-unfold
    (lambda (i)
       (if (and (>= i start) (< i end))
           (fxxor (bitvector-ref/int bvec i) 1)
           (bitvector-ref/int bvec i)))
    (bitvector-length bvec)))

(define (bitvector-field-flip! bvec start end)
  (let lp ((i start))
    (unless (>= i end)
      (bitvector-set! bvec i (fxxor (bitvector-ref/int bvec i) 1))
      (lp (+ i 1)))))


;;; Generators and Accumulators

(define (%make-bitvector-generator bvec ref-proc)
  (let ((len (bitvector-length bvec))
        (i 0))
    (lambda ()
      (if (= i len)
          (eof-object)
          (let ((r (ref-proc bvec i)))
            (set! i (+ i 1))
            r)))))

(define (make-bitvector/int-generator bvec)
  (%make-bitvector-generator bvec bitvector-ref/int))

(define (make-bitvector/bool-generator bvec)
  (%make-bitvector-generator bvec bitvector-ref/bool))

(define (make-bitvector-accumulator)
  (let ((r '()))
    (lambda (x)
      (if (eof-object? x)
        (reverse-list->bitvector r)
        (set! r (cons x r))))))

;;; Logic ops

(define (u1-not a)
  (fx- 1 a))

(define (bitvector-not avec)
  (bitvector-map/int u1-not avec))

(define (bitvector-not! avec)
  (bitvector-map!/int u1-not avec))

(define-syntax u1-and fxand) ;[esl*]

(define (bitvector-and  . vecs)
  (apply bitvector-map/int u1-and vecs))

(define (bitvector-and!  . vecs)
  (apply bitvector-map!/int u1-and vecs))

(define-syntax u1-ior fxior) ;[esl*]

(define (bitvector-ior . vecs)
  (apply bitvector-map/int u1-ior vecs))

(define (bitvector-ior! . vecs)
  (apply bitvector-map!/int u1-ior vecs))

(define-syntax u1-xor fxxor) ;[esl*]

(define (bitvector-xor . vecs)
  (apply bitvector-map/int u1-xor vecs))

(define (bitvector-xor! . vecs)
  (apply bitvector-map!/int u1-xor vecs))

(define (u1-eqv . args)
  (let ((xor-value (apply u1-xor args)))
    (if (odd? (length args))
      xor-value
      (u1-not xor-value))))

(define (bitvector-eqv . vecs)
  (apply bitvector-map/int u1-eqv vecs))

(define (bitvector-eqv! . vecs)
  (apply bitvector-map!/int u1-eqv vecs))

(define (u1-nand a b)
  (u1-not (u1-and a b)))

(define (bitvector-nand a b)
  (bitvector-map/int u1-nand a b))

(define (bitvector-nand! a b)
  (bitvector-map!/int u1-nand a b))

(define (u1-nor a b)
  (u1-not (u1-ior a b)))

(define (bitvector-nor a b)
  (bitvector-map/int u1-nor a b))

(define (bitvector-nor! a b)
  (bitvector-map!/int u1-nor a b))

(define (u1-andc1 a b)
  (u1-and (u1-not a) b))

(define (bitvector-andc1 a b)
  (bitvector-map/int u1-andc1 a b))

(define (bitvector-andc1! a b)
  (bitvector-map!/int u1-andc1 a b))

(define (u1-andc2 a b)
  (u1-and a (u1-not b)))

(define (bitvector-andc2 a b)
  (bitvector-map/int u1-andc2 a b))

(define (bitvector-andc2! a b)
  (bitvector-map!/int u1-andc2 a b))

(define (u1-orc1 a b)
  (u1-ior (u1-not a) b))

(define (bitvector-orc1 a b)
  (bitvector-map/int u1-orc1 a b))

(define (bitvector-orc1! a b)
  (bitvector-map!/int u1-orc1 a b))

(define (u1-orc2 a b)
  (u1-ior a (u1-not b)))

(define (bitvector-orc2 a b)
  (bitvector-map/int u1-orc2 a b))

(define (bitvector-orc2! a b)
  (bitvector-map!/int u1-orc2 a b))

;;; Map-to-List

(define bitvector-map->list/int
  (case-lambda
    ((f bvec) ; fast path
     (bitvector-fold-right/int (lambda (xs b) (cons (f b) xs)) '() bvec))
    ((f . bvecs)
     (apply bitvector-fold-right/int (lambda (xs . bs) (cons (apply f bs) xs)) '() bvecs))))

(define bitvector-map->list/bool
  (case-lambda
    ((f bvec) ; fast path
     (bitvector-fold-right/bool (lambda (xs b) (cons (f b) xs)) '()  bvec))
    ((f . bvecs)
     (apply bitvector-fold-right/bool (lambda (xs . bs) (cons (apply f bs) xs)) '() bvecs))))

;;; Quasi-ints

(define (bitvector-logical-shift bvec count bit)
  (cond ((positive? count)
         (%bitvector-left-shift bvec count (bit->integer bit)))
        ((negative? count)
         (%bitvector-right-shift bvec (- count) (bit->integer bit)))
        (else bvec)))

(define (%bitvector-left-shift bvec count bit)
  (let ((len (bitvector-length bvec)))
    (bitvector-unfold
      (lambda (i)
        (let ((i* (+ i count)))
          (if (< i* len) (bitvector-ref/int bvec i*) bit)))
      len)))

(define (%bitvector-right-shift bvec count bit)
  (bitvector-unfold
    (lambda (i)
      (if (< i count)
          bit
          (bitvector-ref/int bvec (- i count))))
    (bitvector-length bvec)))

(define (bitvector-count bit bvec)
  (let ((int (bit->integer bit)))
    (bitvector-fold/int (lambda (n b) (if (= b int) (+ n 1) n)) 0 bvec)))

(define (bitvector-count-run bit bvec index)
  (let ((int (bit->integer bit)) (len (bitvector-length bvec)))
    (let lp ((i index) (c 0))
      (if (or (>= i len) (not (= int (bitvector-ref/int bvec i))))
          c
          (lp (+ i 1) (+ c 1))))))

(define (bitvector-if if-bvec then-bvec else-bvec)
  (bitvector-map/bool 
    (lambda (bit then-bit else-bit) (if bit then-bit else-bit))
    if-bvec then-bvec else-bvec))

(define (bitvector-first-bit bit bvec)
  (let ((int (bit->integer bit)) (len (bitvector-length bvec)))
    (let lp ((i 0))
      (cond ((>= i len) -1)
            ((= int (bitvector-ref/int bvec i)) i)
            (else (lp (+ i 1)))))))


;;; Quasi-strings

(define (bitvector-prefix-length bvec1 bvec2)
  (let ((end (min (bitvector-length bvec1) (bitvector-length bvec2))))
    (if (eqv? bvec1 bvec2)
        end
        (let lp ((i 0))
          (if (or (>= i end)
                  (not (= (bitvector-ref/int bvec1 i)
                          (bitvector-ref/int bvec2 i))))
              i
              (lp (+ i 1)))))))

(define (bitvector-suffix-length bvec1 bvec2)
  (let ((end1 (bitvector-length bvec1))
        (end2 (bitvector-length bvec2)))
    (let* ((delta (min end1 end2))
           (start (- end1 delta)))
      (if (eqv? bvec1 bvec2)
          delta
          (let lp ((i (- end1 1)) (j (- end2 1)))
            (if (or (< i start)
                    (not (= (bitvector-ref/int bvec1 i)
                            (bitvector-ref/int bvec2 j))))
                (- (- end1 i) 1)
                (lp (- i 1) (- j 1))))))))

(define (bitvector-prefix? bvec1 bvec2)
  (let ((len1 (bitvector-length bvec1)))
    (and (<= len1 (bitvector-length bvec2))
         (= (bitvector-prefix-length bvec1 bvec2) len1))))

(define (bitvector-suffix? bvec1 bvec2)
  (let ((len1 (bitvector-length bvec1)))
    (and (<= len1 (bitvector-length bvec2))
         (= (bitvector-suffix-length bvec1 bvec2) len1))))

(define (bitvector-pad bit bvec len)
  (let ((old-len (bitvector-length bvec)))
    (if (<= len old-len)
        bvec
        (let ((result (make-bitvector len bit)))
          (bitvector-copy! result (- len old-len) bvec)
          result))))

(define (bitvector-pad-right bit bvec len)
  (if (<= len (bitvector-length bvec))
      bvec
      (let ((result (make-bitvector len bit)))
        (bitvector-copy! result 0 bvec)
        result)))

(define (%bitvector-skip bvec bit)
  (let ((len (bitvector-length bvec))
        (int (bit->integer bit)))
    (let lp ((i 0))
      (and (< i len)
           (if (= int (bitvector-ref/int bvec i))
               (lp (+ i 1))
               i)))))

(define (%bitvector-skip-right bvec bit)
  (let ((len (bitvector-length bvec))
        (int (bit->integer bit)))
    (let lp ((i (- len 1)))
      (and (>= i 0)
           (if (= int (bitvector-ref/int bvec i))
               (lp (- i 1))
               i)))))

(define (bitvector-trim bit bvec)
  (cond ((%bitvector-skip bvec bit) =>
         (lambda (skip)
           (bitvector-copy bvec skip (bitvector-length bvec))))
        (else (bitvector))))

(define (bitvector-trim-right bit bvec)
  (cond ((%bitvector-skip-right bvec bit) =>
         (lambda (skip)
           (bitvector-copy bvec 0 (+ skip 1))))
        (else (bitvector))))

(define (bitvector-trim-both bit bvec)
  (cond ((%bitvector-skip bvec bit) =>
         (lambda (skip)
           (bitvector-copy bvec skip (+ 1 (%bitvector-skip-right bvec bit)))))
        (else (bitvector))))

;;;; Unfolds

;;; These procedures work by building temporary lists, then converting
;;; them to vectors. This uses more space than pre-allocating a bitvector
;;; and filling it, but it's referentially transparent: there's no way
;;; to capture a partially-filled bitvector through continuation tricks.

;; Unfold a list. f is passed the current index and list of seeds
;; on each step, and must return a bit and new seeds on each step.
(define (%unfold/index f len seeds)
  (letrec
    ((build
      (lambda (i seeds)
        (if (= i len)
            '()
            (let-values (((b . seeds*) (apply f i seeds)))
              (cons b (build (+ i 1) seeds*)))))))
    (build 0 seeds)))

(define (bitvector-unfold f len . seeds)
  (if (null? seeds) 
      (%bitvector-unfold f len) ;[esl+] fast path
      (list->bitvector (%unfold/index f len seeds))))

;; Unfold a list from the right. f is passed the current index and
;; list of seeds on each step, and must return a bit and new seeds
;; on each step.
(define (%unfold-right/index f len seeds)
  (letrec
    ((build
      (lambda (i seeds res)
        (if (< i 0)
            res
            (let-values (((b . seeds*) (apply f i seeds)))
              (build (- i 1) seeds* (cons b res)))))))
    (build (- len 1) seeds '())))

(define (bitvector-unfold-right f len . seeds)
  (list->bitvector (%unfold-right/index f len seeds)))
  
))
