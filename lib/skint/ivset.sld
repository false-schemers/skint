(define-library (skint ivset)

  (import (scheme base))

  (export 
    ivs->ivl ivl->ivs
    ivs-size ivs-contains?
    ivs=? ivs<=? ivs-disjoint?
    ivs-union ivs-intersection
    ivs-difference
    ivs->list list->ivs
    predicate->ivs
    ivs-for-each)

(begin

;; list helpers

(define (sort! l) 
  (cond [(null? l) '()]
        [(null? (cdr l)) l]
        [else (let ([l2 (split! l)]) (merge! (sort! l) (sort! l2)))]))

(define (merge! l1 l2) 
  (cond [(null? l1) l2] [(null? l2) l1]
        [(< (car l2) (car l1)) (set-cdr! l2 (merge! l1 (cdr l2))) l2]
        [else (set-cdr! l1 (merge! (cdr l1) l2)) l1]))

(define (split! l)
  (if (null? l) l
      (let loop ([hd l] [tl (cdr l)])
        (if (or (null? tl) (null? (cdr tl)))
            (let ([x (cdr hd)]) (set-cdr! hd '()) x)
            (loop (cdr hd) (cddr tl))))))

(define (list-sort/remove-duplicates l)
  (if (null? l)
      '()
      (let loop ([sl (sort! (list-copy l))] [rl '()])
        (cond [(null? sl) (reverse rl)]
              [(and (not (null? rl)) (= (car sl) (car rl)))
               (loop (cdr sl) rl)]
              [else (loop (cdr sl) (cons (car sl) rl))]))))


;; Interval vectors

(define-syntax first (syntax-rules () [(_ iv) (car iv)]))

(define-syntax last (syntax-rules () [(_ iv) (cdr iv)]))

(define-syntax make-iv (syntax-rules () [(_ f l) (cons f l)]))

(define (iv-valid? iv) (<= (first iv) (last iv)))

(define (iv-before? iv1 iv2) (< (last iv1) (- (first iv2) 1))) ; does not touch!

;; place of iv1 compared to iv2: 0=intersect, -1=before(-touch), 1=after(-touch)
(define (iv-compare iv1 iv2)
  (if (<= (max (first iv1) (first iv2)) (min (last iv1) (last iv2)))
      0
      (if (< (last iv1) (first iv2)) -1 1)))

;; check that iv1 is included in iv2
(define (subiv-of? iv1 iv2)
  (and (<= (first iv2) (first iv1)) (>= (last iv2) (last iv1))))

;; shortest interval that includes both iv1 and iv2 (always valid)
(define (iv-join iv1 iv2)
  (make-iv (min (first iv1) (first iv2)) (max (last iv1) (last iv2))))

;; longest interval that is included in both iv1 and iv2 (may be invalid)
(define (iv-meet i1 i2)
  (make-iv (max (first i1) (first i2)) (min (last i1) (last i2))))

;; union interval list of interval iv and interval list ivl
;; precondition: ivl is a valid interval list (disjoint, sorted, with gaps)
(define (iv-adjoin iv ivl)
  (let loop ([iv iv] [ivl ivl] [rl '()])
    (cond [(null? ivl) (reverse (cons iv rl))]
          [(iv-before? iv (car ivl))
           (append (reverse (cons iv rl)) ivl)]
          [(iv-before? (car ivl) iv)
           (loop iv (cdr ivl) (cons (car ivl) rl))]
          [else (loop (iv-join iv (car ivl)) (cdr ivl) rl)])))

;; intersection interval list of interval iv and interval list ivl
;; precondition: ivl is a valid interval list (disjoint, sorted, with gaps)
(define (iv-disjoin iv ivl)
  (let loop ([ivl ivl] [rl '()])
    (cond [(null? ivl) (reverse rl)]
          [(iv-before? iv (car ivl)) (reverse rl)]
          [(iv-before? (car ivl) iv) (loop (cdr ivl) rl)]
          [(subiv-of? iv (car ivl)) (reverse (cons iv rl))]
          [(subiv-of? (car ivl) iv) (loop (cdr ivl) (cons (car ivl) rl))]
          [else (let ([r (iv-meet iv (car ivl))])
                  (loop (cdr ivl) (if (iv-valid? r) (cons r rl) rl)))])))

;; subtract iv from iv0 (they should intersect or touch)
(define (iv-subtract iv iv0)
  (define ivl1 
    (if (< (first iv0) (first iv)) (list (make-iv (first iv0) (- (first iv) 1))) '()))
  (define ivl2 
    (if (< (last iv) (last iv0)) (list (make-iv (+ (last iv) 1) (last iv0))) '()))
  (reverse (append ivl2 ivl1)))

;; subtract interval iv from interval list ivl
(define (iv-subtract* iv ivl)
  (let loop ([ivl ivl] [rl '()])
    (cond [(null? ivl) (reverse rl)]
          [(iv-before? iv (car ivl)) (append (reverse rl) ivl)]
          [(iv-before? (car ivl) iv) (loop (cdr ivl) (cons (car ivl) rl))]
          [(subiv-of? (car ivl) iv) (loop (cdr ivl) rl)]
          [else (loop (cdr ivl) (append (reverse (iv-subtract iv (car ivl))) rl))])))

;; convert interval list to ivs (a vector)
(define (ivl->ivs ivl)
  (let* ([len (* 2 (length ivl))] [vec (make-vector len)])
    (let loop ([i 0] [ivl ivl])
      (if (null? ivl)
          vec
          (begin
            (vector-set! vec i (first (car ivl)))
            (vector-set! vec (+ i 1) (last (car ivl)))
            (loop (+ i 2) (cdr ivl)))))))

;; convert ivs (a vector) to interval list
(define (ivs->ivl vec)
  (let ([n (quotient (vector-length vec) 2)])
    (let loop ([i 0] [rl '()])
      (if (>= i n)
          (reverse rl)
          (loop (+ i 1)
                (cons (make-iv (vector-ref vec (* 2 i))
                        (vector-ref vec (+ (* 2 i) 1)))
                      rl))))))

;; set cardinality
(define (ivs-size ivs)
  (let ([ivl (ivs->ivl ivs)])
    (let loop ([ivl ivl] [cnt 0])
      (if (null? ivl)
          cnt
          (loop (cdr ivl)
                (+ cnt (- (last (car ivl)) (first (car ivl))) 1))))))

;; set membership test (binary search)
(define (ivs-contains? ivs e)
  (let ([n (quotient (vector-length ivs) 2)])
    (let loop ([lo 0] [hi (- n 1)])
      (and (<= lo hi)
           (let ([mid (quotient (+ lo hi) 2)])
             (cond [(< e (vector-ref ivs (* 2 mid))) (loop lo (- mid 1))]
                   [(> e (vector-ref ivs (+ (* 2 mid) 1))) (loop (+ mid 1) hi)]
                   [else #t]))))))

;; set equality test
(define (ivs=? ivs1 ivs2) (equal? ivs1 ivs2))

;; subset test: s1 <= s2
(define (ivs<=? ivs1 ivs2)
  (let loop ([ivl1 (ivs->ivl ivs1)] [ivl2 (ivs->ivl ivs2)])
    (cond [(null? ivl1) #t] [(null? ivl2) #f]
          [else (case (iv-compare (car ivl1) (car ivl2))
                  [(0) ; intersect
                   (if (subiv-of? (car ivl1) (car ivl2))
                       (loop (cdr ivl1) ivl2)
                       #f)]
                  [(-1) #f] ; ci1 is lower
                  [(1) (loop ivl1 (cdr ivl2))])])))

;; set disjointness test
(define (ivs-disjoint? ivs1 ivs2)
  (let loop ([ivl1 (ivs->ivl ivs1)] [ivl2 (ivs->ivl ivs2)])
    (cond [(null? ivl1) #t] [(null? ivl2) #t]
          [else (case (iv-compare (car ivl1) (car ivl2))
                  [(0) #f] ; intersect
                  [(-1) (loop (cdr ivl1) ivl2)]
                  [(1) (loop ivl1 (cdr ivl2))])])))

;; union of two sets
(define (ivs-union ivs1 ivs2)
  (let loop ([ivl1 (ivs->ivl ivs1)] [ivl2 (ivs->ivl ivs2)] [rl '()])
    (cond [(null? ivl1) (ivl->ivs (reverse (append (reverse ivl2) rl)))]
          [(null? ivl2) (ivl->ivs (reverse (append (reverse ivl1) rl)))]
          [(iv-before? (car ivl1) (car ivl2))
           (loop (cdr ivl1) ivl2 (cons (car ivl1) rl))]
          [(iv-before? (car ivl2) (car ivl1))
           (loop ivl1 (cdr ivl2) (cons (car ivl2) rl))]
          [else (loop (cdr ivl1) (iv-adjoin (car ivl1) ivl2) rl)])))

;; intersection of two sets
(define (ivs-intersection ivs1 ivs2)
  (let loop ([ivl1 (ivs->ivl ivs1)] [ivl2 (ivs->ivl ivs2)] [rl '()])
    (cond [(null? ivl1) (ivl->ivs (reverse rl))]
          [(null? ivl2) (ivl->ivs (reverse rl))]
          [(iv-before? (car ivl1) (car ivl2))
           (loop (cdr ivl1) ivl2 rl)]
          [(iv-before? (car ivl2) (car ivl1))
           (loop ivl1 (cdr ivl2) rl)]
          [(subiv-of? (car ivl1) (car ivl2))
           (loop (cdr ivl1) ivl2 (cons (car ivl1) rl))]
          [(subiv-of? (car ivl2) (car ivl1))
           (loop ivl1 (cdr ivl2) (cons (car ivl2) rl))]
          [else ; intervals touch or intersect 
           (let ([dsj (iv-disjoin (car ivl1) ivl2)])
             (loop (cdr ivl1) ivl2 (append (reverse dsj) rl)))])))

;; difference of two sets (s1 - s2)
(define (ivs-difference ivs1 ivs2)
  (let loop ([ivl1 (ivs->ivl ivs1)] [ivl2 (ivs->ivl ivs2)] [rl '()])
    (cond [(null? ivl1) (ivl->ivs (reverse rl))]
          [(null? ivl2) (ivl->ivs (reverse (append (reverse ivl1) rl)))]
          [(iv-before? (car ivl1) (car ivl2))
           (loop (cdr ivl1) ivl2 (cons (car ivl1) rl))]
          [(iv-before? (car ivl2) (car ivl1))
           (loop ivl1 (cdr ivl2) rl)]
          [else ; intervals touch or intersect
           (let ([subbed (iv-subtract* (car ivl2) ivl1)])
             (loop subbed (cdr ivl2) rl))])))

;; convert to list
(define (ivs->list ivs)
  (let ([n (quotient (vector-length ivs) 2)])
    (let loop ([i (- n 1)] [rl '()])
      (if (< i 0) rl
          (let* ([start (vector-ref ivs (* 2 i))]
                 [end (vector-ref ivs (+ (* 2 i) 1))])
            (let lp ([e end] [acc rl])
              (if (< e start)
                  (loop (- i 1) acc)
                  (lp (- e 1) (cons e acc)))))))))

;; convert from list
(define (list->ivs lst)
  (if (null? lst)
      (vector)
      (let ([sorted (list-sort/remove-duplicates lst)])
        (let loop ([lst (cdr sorted)]
                   [start (car sorted)]
                   [end (car sorted)]
                   [ranges '()])
          (cond [(null? lst)
                 (ivl->ivs (reverse (cons (make-iv start end) ranges)))]
                [(= (car lst) (+ end 1))
                 (loop (cdr lst) start (car lst) ranges)]
                [else (loop (cdr lst) (car lst) (car lst)
                        (cons (make-iv start end) ranges))])))))

(define (predicate->ivs pred start end)
  (let loop ([i start] [cstart #f] [cend #f]  [ranges '()])
    (cond [(>= i end)
           (if cstart
               (ivl->ivs (reverse (cons (make-iv cstart cend) ranges)))
               (ivl->ivs (reverse ranges)))]
          [(pred i)
           (if cstart
               (loop (+ i 1) cstart i ranges)
               (loop (+ i 1) i i ranges))]
          [else
           (if cstart
               (loop (+ i 1) #f #f (cons (make-iv cstart cend) ranges))
               (loop (+ i 1) #f #f ranges))])))

(define (ivs-for-each proc ivs)
  (let ([n (quotient (vector-length ivs) 2)])
    (let loop ([i 0])
      (when (< i n)
        (let* ([idx (* 2 i)]
               [start (vector-ref ivs idx)]
               [end (vector-ref ivs (+ idx 1))])
          (let lp ([e start])
            (when (<= e end) (proc e) (lp (+ e 1))))
          (loop (+ i 1)))))))

))
