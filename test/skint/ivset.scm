(import (srfi 95)) ; sort
(import (skint ivset))

(include "test.scm")

;; Helper to create IVS from ranges for easier test writing.
(define (ranges->ivs . ranges) (ivl->ivs ranges))

(define (list-sort < lst) (sort lst <))


#|
(display "--- Testing Internals ---\n")

(test "adjoin: Simple intersect"
  '((10 . 25))
  (iv-adjoin '(10 . 20) '((15 . 25))))

(test "adjoin: Joins multiple elements"
  '((10 . 40))
  (iv-adjoin '(10 . 35) '((15 . 25) (30 . 40))))

(test "adjoin: Touching at start"
  '((10 . 30))
  (iv-adjoin '(20 . 30) '((10 . 19))))

(test "adjoin: Input is contained"
  '((10 . 30))
  (iv-adjoin '(15 . 25) '((10 . 30))))

(test "adjoin: Stops at gap"
  '((10 . 25) (30 . 40))
  (iv-adjoin '(10 . 20) '((18 . 25) (30 . 40))))

;; Test 1: i and first interval touch (adjacent)
(test '((1 . 10))
      (iv-adjoin (make-iv 1 5) '((6 . 10))))

;; Test 2: i overlaps with first interval
(test '((1 . 10))
      (iv-adjoin (make-iv 1 7) '((5 . 10))))

;; Test 3: i overlaps first, then continues with non-touching intervals
(test '((1 . 10) (15 . 20))
      (iv-adjoin (make-iv 1 7) '((5 . 10) (15 . 20))))

;; Test 4: i touches/overlaps multiple intervals, merging them all
(test '((1 . 20))
      (iv-adjoin (make-iv 5 15) '((1 . 7) (10 . 13) (14 . 20))))

;; Test 5: i is entirely before remaining intervals after merging first
(test '((1 . 10) (20 . 25) (30 . 35))
      (iv-adjoin (make-iv 1 5) '((6 . 10) (20 . 25) (30 . 35))))


(test "disjoin: Simple intersect"
  '((15 . 20))
  (iv-disjoin '(10 . 20) '((15 . 25) (30 . 40))))

(test "disjoin: Input is a sub-interval"
  '((15 . 20))
  (iv-disjoin '(15 . 20) '((10 . 30))))

(test "disjoin: List element is a sub-interval"
  '((15 . 20))
  (iv-disjoin '(10 . 30) '((15 . 20) (40 . 50))))

(test "disjoin: Spans multiple elements"
  '((15 . 25) (30 . 35) (40 . 45))
  (iv-disjoin '(10 . 50) '((15 . 25) (30 . 35) (40 . 45))))

(test "disjoin: Touching without overlap"
  '()
  (iv-disjoin '(10 . 20) '((21 . 30))))

;; Test 1: i partially overlaps first interval from left
(test '((5 . 7))
      (iv-disjoin '(1 . 7) '((5 . 10))))

;; Test 2: i partially overlaps first interval from right
(test '((5 . 10))
      (iv-disjoin '(5 . 15) '((1 . 10))))

;; Test 3: i intersects multiple intervals
(test '((5 . 7) (12 . 15))
      (iv-disjoin '(1 . 15) '((5 . 7) (12 . 20))))

;; Test 4: i ends before later intervals (only intersects first)
(test '((5 . 7))
      (iv-disjoin '(1 . 7) '((5 . 10) (15 . 20))))

;; Test 5: i spans across gap, intersecting intervals on both sides
(test '((3 . 5) (8 . 10))
      (iv-disjoin '(1 . 12) '((3 . 5) (8 . 10) (20 . 25))))
|#

(display "\n--- Testing Construction and Basic Operations ---\n")

(test "list->ivs: Empty list"
  (ranges->ivs)
  (list->ivs '()))

(test "list->ivs: Single element"
  (ranges->ivs '(10 . 10))
  (list->ivs '(10)))

(test "list->ivs: Simple contiguous range"
  (ranges->ivs '(5 . 8))
  (list->ivs '(5 6 7 8)))

(test "list->ivs: With duplicates and unsorted"
  (ranges->ivs '(5 . 8))
  (list->ivs '(8 7 6 5 6 7)))

(test "list->ivs: Two disjoint ranges"
  (ranges->ivs '(5 . 6) '(10 . 11))
  (list->ivs '(5 6 10 11)))

(test "list->ivs: Adjacent ranges (should not merge)"
  (ranges->ivs '(5 . 6) '(8 . 9))
  (list->ivs '(5 6 8 9)))

(test "ivs-size: Empty set"
  0
  (ivs-size (list->ivs '())))

(test "ivs-size: Single range"
  5
  (ivs-size (list->ivs '(1 2 3 4 5))))

(test "ivs-size: Multiple ranges"
  7
  (ivs-size (list->ivs '(1 2 10 11 12 20 21))))

(test "ivs->list: Empty set"
  '()
  (ivs->list (list->ivs '())))

(test "ivs->list: Multiple ranges"
  '(1 2 10 11 12 20 21)
  (ivs->list (list->ivs '(1 2 10 11 12 20 21))))

(let ((s (list->ivs '(10 12 14))))
  (test "ivs-for-each"
    '(10 12 14)
    (let ((acc '()))
      (ivs-for-each (lambda (x) (set! acc (cons x acc))) s)
      (reverse acc))))

(display "\n--- Testing ivs-contains? ---\n")

(let ((s (ranges->ivs '(10 . 20) '(30 . 40))))
  (test "contains?: Start of first range"   #t (ivs-contains? s 10))
  (test "contains?: End of first range"     #t (ivs-contains? s 20))
  (test "contains?: Middle of first range"  #t (ivs-contains? s 15))
  (test "contains?: Start of second range"  #t (ivs-contains? s 30))
  (test "contains?: Middle of second range" #t (ivs-contains? s 35))
  (test "contains?: Before all ranges"      #f (ivs-contains? s 5))
  (test "contains?: After all ranges"       #f (ivs-contains? s 50))
  (test "contains?: In the gap"             #f (ivs-contains? s 25)))

(display "\n--- Testing Set Operations ---\n")

(let ((s1 (ranges->ivs '(10 . 20)))
      (s2 (ranges->ivs '(15 . 25)))
      (s3 (ranges->ivs '(100 . 110))))
  (test "union: Overlapping sets"
    (ranges->ivs '(10 . 25))
    (ivs-union s1 s2))
  (test "union: Disjoint sets"
    (ranges->ivs '(10 . 20) '(100 . 110))
    (ivs-union s1 s3))
  (test "union: Adjacent sets"
    (ranges->ivs '(10 . 25))
    (ivs-union (ranges->ivs '(10 . 17)) (ranges->ivs '(18 . 25))))
  (test "union: One set contains another"
    (ranges->ivs '(15 . 25))
    (ivs-union s2 (ranges->ivs '(18 . 22))))
  (test "union: With empty set"
    s1
    (ivs-union s1 (list->ivs '()))))

(let ((s1 (ranges->ivs '(10 . 20)))
      (s2 (ranges->ivs '(15 . 25)))
      (s3 (ranges->ivs '(100 . 110)))
      (s4 (ranges->ivs '(12 . 18))))
  (test "intersection: Overlapping sets"
    (ranges->ivs '(15 . 20))
    (ivs-intersection s1 s2))
  (test "intersection: Disjoint sets"
    (ranges->ivs)
    (ivs-intersection s1 s3))
  (test "intersection: One contains another"
    s4
    (ivs-intersection s1 s4))
  (test "intersection: With empty set"
    (ranges->ivs)
    (ivs-intersection s1 (list->ivs '()))))

(let ((s1 (ranges->ivs '(10 . 40)))
      (s2 (ranges->ivs '(20 . 30)))
      (s3 (ranges->ivs '(5 . 15)))
      (s4 (ranges->ivs '(35 . 45)))
      (s5 (ranges->ivs '(0 . 100))))
  (test "difference: Subtract from middle"
    (ranges->ivs '(10 . 19) '(31 . 40))
    (ivs-difference s1 s2))
  (test "difference: Subtract from start"
    (ranges->ivs '(16 . 40))
    (ivs-difference s1 s3))
  (test "difference: Subtract from end"
    (ranges->ivs '(10 . 34))
    (ivs-difference s1 s4))
  (test "difference: Subtract everything"
    (ranges->ivs)
    (ivs-difference s1 s5))
  (test "difference: Disjoint sets"
    s1
    (ivs-difference s1 (ranges->ivs '(100 . 200))))
  (test "difference: With empty set"
    s1
    (ivs-difference s1 (list->ivs '()))))

(display "\n--- Testing Comparison Operations ---\n")

(let ((s1 (ranges->ivs '(10 . 20)))
      (s2 (ranges->ivs '(10 . 20)))
      (s3 (ranges->ivs '(10 . 21)))
      (s4 (ranges->ivs '(12 . 18)))
      (s5 (ranges->ivs '(2 . 8)))
      (empty (list->ivs '())))
  (test "ivs=?: Equal sets"        #t (ivs=? s1 s2))
  (test "ivs=?: Unequal sets"      #f (ivs=? s1 s3))
  (test "ivs=?: Empty sets"        #t (ivs=? empty (list->ivs '())))

  (test "ivs<=?: Proper subset"    #t (ivs<=? s4 s1))
  (test "ivs<=?: Equal sets"       #t (ivs<=? s1 s2))
  (test "ivs<=?: Superset"         #f (ivs<=? s1 s4))
  (test "ivs<=?: Disjoint sets"    #f (ivs<=? s1 s5))
  (test "ivs<=?: Empty is subset"  #t (ivs<=? empty s1))
  (test "ivs<=?: Is subset of self" #t (ivs<=? s1 s1)))

;; Test adjacent ranges (should merge in union, stay separate otherwise)
(test (vector 1 10) (ivs-union (vector 1 5) (vector 6 10)))
(test (vector 1 5 7 10) (ivs-union (vector 1 5) (vector 7 10)))
(test (vector) (ivs-intersection (vector 1 5) (vector 6 10)))
(test (vector 1 5) (ivs-difference (vector 1 5) (vector 6 10)))

;; Test touching at boundaries
(test #t (ivs-contains? (vector 1 5) 1))
(test #t (ivs-contains? (vector 1 5) 5))
(test #f (ivs-contains? (vector 1 5) 0))
(test #f (ivs-contains? (vector 1 5) 6))

;; Test union with overlapping ranges
(test (vector 1 10) (ivs-union (vector 1 7) (vector 5 10)))
(test (vector 1 15) (ivs-union (vector 1 10) (vector 8 15)))
(test (vector 1 10) (ivs-union (vector 1 10) (vector 3 7)))
(test (vector 1 10) (ivs-union (vector 3 7) (vector 1 10)))

;; Test intersection with partial overlap
(test (vector 5 7) (ivs-intersection (vector 1 7) (vector 5 10)))
(test (vector 8 10) (ivs-intersection (vector 1 10) (vector 8 15)))
(test (vector 3 7) (ivs-intersection (vector 1 10) (vector 3 7)))
(test (vector 3 7) (ivs-intersection (vector 3 7) (vector 1 10)))

;; Test difference with partial overlap
(test (vector 1 4) (ivs-difference (vector 1 7) (vector 5 10)))
(test (vector 1 7) (ivs-difference (vector 1 10) (vector 8 15)))
(test (vector 1 2 8 10) (ivs-difference (vector 1 10) (vector 3 7)))
(test (vector) (ivs-difference (vector 3 7) (vector 1 10)))

;; Test with multiple non-overlapping ranges in both operands
(test (vector 1 3 5 7 10 12 15 17)
          (ivs-union (vector 1 3 10 12) (vector 5 7 15 17)))
(test (vector)
          (ivs-intersection (vector 1 3 10 12) (vector 5 7 15 17)))
(test (vector 1 3 10 12)
          (ivs-difference (vector 1 3 10 12) (vector 5 7 15 17)))

;; Test with ranges that share one endpoint
(test (vector 1 10) (ivs-union (vector 1 5) (vector 5 10)))
(test (vector 5 5) (ivs-intersection (vector 1 5) (vector 5 10)))
(test (vector 1 4) (ivs-difference (vector 1 5) (vector 5 10)))

;; Test subset relationships
(test #t (ivs<=? (vector) (vector)))
(test #t (ivs<=? (vector) (vector 1 10)))
(test #f (ivs<=? (vector 1 10) (vector)))
(test #t (ivs<=? (vector 3 7) (vector 1 10)))
(test #f (ivs<=? (vector 1 10) (vector 3 7)))
(test #t (ivs<=? (vector 3 5 7 9) (vector 1 10)))
(test #f (ivs<=? (vector 3 5 7 11) (vector 1 10)))

;; Test disjointness
(test #t (ivs-disjoint? (vector) (vector)))
(test #t (ivs-disjoint? (vector 1 5) (vector)))
(test #t (ivs-disjoint? (vector) (vector 1 5)))
(test #t (ivs-disjoint? (vector 1 5) (vector 7 10)))
(test #f (ivs-disjoint? (vector 1 5) (vector 5 10)))
(test #f (ivs-disjoint? (vector 1 7) (vector 5 10)))
(test #t (ivs-disjoint? (vector 1 3 7 9) (vector 4 5 11 15)))
(test #f (ivs-disjoint? (vector 1 3 7 9) (vector 3 5 11 15)))

;; Test multiple ranges with complex overlap patterns
(test (vector 1 7 10 20)
          (ivs-union (vector 1 5 10 15) (vector 3 7 12 20)))
(test (vector 3 5 12 15)
          (ivs-intersection (vector 1 5 10 15) (vector 3 7 12 20)))
(test (vector 1 2 10 11)
          (ivs-difference (vector 1 5 10 15) (vector 3 7 12 20)))

;; Test with ranges at extremes of valid range (0 to 2^24-1)
(let ((max-val 16777215))  ; 2^24 - 1
  (test #t (ivs-contains? (vector 0 max-val) 0))
  (test #t (ivs-contains? (vector 0 max-val) max-val))
  (test #t (ivs-contains? (vector 0 max-val) (quotient max-val 2)))
  (test (+ max-val 1) (ivs-size (vector 0 max-val)))
  (test (vector 0 max-val) (ivs-union (vector 0 100) (vector 100 max-val))))

;; Test difference creating gaps
(test (vector 1 4 6 10) (ivs-difference (vector 1 10) (vector 5 5)))
(test (vector 1 2 4 5 7 10) (ivs-difference (vector 1 10) (vector 3 3 6 6)))
(test (vector 2 9) (ivs-difference (vector 1 10) (vector 1 1 10 10)))

;; Test union that merges multiple ranges
(test (vector 1 10) (ivs-union (vector 1 3 5 7 9 10) (vector 4 4 8 8)))
(test (vector 1 15) (ivs-union (vector 1 5 10 15) (vector 5 10)))

;; Test intersection with one range spanning multiple in the other
(test (vector 2 4 6 8) (ivs-intersection (vector 1 10) (vector 2 4 6 8)))
(test (vector 5 5 10 10 15 15) 
          (ivs-intersection (vector 5 5 10 10 15 15) (vector 1 20)))

;; Test difference with second set having multiple small ranges
(test (vector 1 1 3 3 5 5 7 7 9 9)
          (ivs-difference (vector 1 10) (vector 2 2 4 4 6 6 8 8 10 10)))

;; Test symmetric difference scenarios via combination of operations
(let ((sd (lambda (s1 s2)
            (ivs-union (ivs-difference s1 s2)
                       (ivs-difference s2 s1)))))
  (test (vector 1 4 8 10) (sd (vector 1 7) (vector 5 10)))
  (test (vector) (sd (vector 1 10) (vector 1 10)))
  (test (vector 1 10) (sd (vector 1 10) (vector))))

;; Test operations preserving normalization (no adjacent ranges)
(let ((check-normalized
       (lambda (ivs)
         (let ((n (quotient (vector-length ivs) 2)))
           (let loop ((i 0))
             (if (>= i (- n 1))
                 #t
                 (and (< (+ (vector-ref ivs (+ (* 2 i) 1)) 1)
                        (vector-ref ivs (* 2 (+ i 1))))
                      (loop (+ i 1)))))))))
  (test #t (check-normalized (ivs-union (vector 1 5) (vector 10 15))))
  (test #t (check-normalized (ivs-union (vector 1 3 7 9) (vector 5 5))))
  (test #t (check-normalized (ivs-difference (vector 1 20) (vector 5 10)))))

;; Test that operations maintain sorted order
(let ((check-sorted
       (lambda (ivs)
         (let ((n (quotient (vector-length ivs) 2)))
           (let loop ((i 0))
             (if (>= i (- n 1))
                 #t
                 (and (< (vector-ref ivs (+ (* 2 i) 1))
                        (vector-ref ivs (* 2 (+ i 1))))
                      (loop (+ i 1)))))))))
  (test #t (check-sorted (ivs-union (vector 10 15) (vector 1 5))))
  (test #t (check-sorted (ivs-intersection (vector 5 10 20 25) (vector 1 8 22 30)))))

;; Test ivs-size with complex sets
(test 15 (ivs-size (vector 1 5 10 14 20 24)))
(test 100 (ivs-size (vector 0 49 100 149)))
(test 1 (ivs-size (vector 1000000 1000000)))


;; Test ivs-contains?
(let ((iv (vector 10 15 20 25 30 35)))
  (test #t (ivs-contains? iv 10))      ; start of first range
  (test #t (ivs-contains? iv 15))      ; end of first range
  (test #t (ivs-contains? iv 13))      ; middle of first range
  (test #t (ivs-contains? iv 20))      ; start of second range
  (test #t (ivs-contains? iv 25))      ; end of second range
  (test #t (ivs-contains? iv 22))      ; middle of second range
  (test #t (ivs-contains? iv 30))      ; start of third range
  (test #t (ivs-contains? iv 35))      ; end of third range
  (test #t (ivs-contains? iv 33))      ; middle of third range
  (test #f (ivs-contains? iv 9))       ; before first range
  (test #f (ivs-contains? iv 16))      ; gap between first and second
  (test #f (ivs-contains? iv 19))      ; gap between first and second
  (test #f (ivs-contains? iv 26))      ; gap between second and third
  (test #f (ivs-contains? iv 29))      ; gap between second and third
  (test #f (ivs-contains? iv 36))      ; after last range
  (test #f (ivs-contains? iv 100)))    ; far after last range

;; Test empty IVS
(let ((empty (vector)))
  (test #f (ivs-contains? empty 0))
  (test #f (ivs-contains? empty 10))
  (test 0 (ivs-size empty))
  (test '() (ivs->list empty)))

;; Test single element IVS
(let ((single (vector 42 42)))
  (test #t (ivs-contains? single 42))
  (test #f (ivs-contains? single 41))
  (test #f (ivs-contains? single 43))
  (test 1 (ivs-size single))
  (test '(42) (ivs->list single)))

;; Test single range IVS
(let ((range (vector 10 20)))
  (test #t (ivs-contains? range 10))
  (test #t (ivs-contains? range 15))
  (test #t (ivs-contains? range 20))
  (test #f (ivs-contains? range 9))
  (test #f (ivs-contains? range 21))
  (test 11 (ivs-size range)))

;; Test ivs-size
(test 0 (ivs-size (vector)))
(test 1 (ivs-size (vector 5 5)))
(test 11 (ivs-size (vector 10 20)))
(test 18 (ivs-size (vector 10 15 20 25 30 35)))  ; 6+6+6
(test 100 (ivs-size (vector 0 99)))

;; Test ivs->list
(test '() (ivs->list (vector)))
(test '(5) (ivs->list (vector 5 5)))
(test '(10 11 12) (ivs->list (vector 10 12)))
(test '(1 2 3 10 11 12 20 21)
          (ivs->list (vector 1 3 10 12 20 21)))

;; Test list->ivs
(test (vector) (list->ivs '()))
(test (vector 5 5) (list->ivs '(5)))
(test (vector 1 3) (list->ivs '(1 2 3)))
(test (vector 1 3 10 12 20 21) 
          (list->ivs '(1 2 3 10 11 12 20 21)))

;; Test list->ivs with unsorted input
(test (vector 1 3 10 12) (list->ivs '(12 2 1 11 10 3)))

;; Test list->ivs with duplicates
(test (vector 1 5) (list->ivs '(1 2 2 3 3 4 5)))

;; Test list->ivs round-trip
(let ((iv (vector 10 15 20 25 30 35)))
  (test iv (list->ivs (ivs->list iv))))

;; Test ivs=?
(test #t (ivs=? (vector) (vector)))
(test #t (ivs=? (vector 10 20) (vector 10 20)))
(test #t (ivs=? (vector 1 5 10 15) (vector 1 5 10 15)))
(test #f (ivs=? (vector 10 20) (vector 10 21)))
(test #f (ivs=? (vector 10 20) (vector 11 20)))
(test #f (ivs=? (vector 1 5 10 15) (vector 1 5 10 16)))
(test #f (ivs=? (vector 10 20) (vector)))

;; Test ivs<=? (subset)
(test #t (ivs<=? (vector) (vector)))
(test #t (ivs<=? (vector) (vector 1 10)))
(test #t (ivs<=? (vector 5 7) (vector 1 10)))
(test #t (ivs<=? (vector 5 7) (vector 5 7)))
(test #t (ivs<=? (vector 2 3 7 8) (vector 1 10)))
(test #f (ivs<=? (vector 1 10) (vector)))
(test #f (ivs<=? (vector 5 12) (vector 1 10)))
(test #f (ivs<=? (vector 0 5) (vector 1 10)))

;; Test ivs-for-each
(let ((collected '()))
  (ivs-for-each (lambda (x) (set! collected (cons x collected)))
                (vector 1 3 10 11))
  (test '(11 10 3 2 1) collected))

(let ((count 0))
  (ivs-for-each (lambda (x) (set! count (+ count 1)))
                (vector 0 99))
  (test 100 count))

;; Test ivs-union
(test (vector) (ivs-union (vector) (vector)))
(test (vector 1 10) (ivs-union (vector 1 10) (vector)))
(test (vector 1 10) (ivs-union (vector) (vector 1 10)))
(test (vector 1 10) (ivs-union (vector 1 5) (vector 6 10)))
(test (vector 1 5 7 10) (ivs-union (vector 1 5) (vector 7 10)))
(test (vector 1 15) (ivs-union (vector 1 10) (vector 5 15)))
(test (vector 1 20) (ivs-union (vector 1 10) (vector 1 20)))

;; Test ivs-intersection
(test (vector) (ivs-intersection (vector) (vector)))
(test (vector) (ivs-intersection (vector 1 10) (vector)))
(test (vector) (ivs-intersection (vector) (vector 1 10)))
(test (vector) (ivs-intersection (vector 1 5) (vector 6 10)))
(test (vector 5 10) (ivs-intersection (vector 1 10) (vector 5 15)))
(test (vector 1 10) (ivs-intersection (vector 1 10) (vector 1 20)))
(test (vector 5 7 12 14) 
          (ivs-intersection (vector 1 10 12 20) (vector 5 7 12 14)))

;; Test ivs-difference
(test (vector) (ivs-difference (vector) (vector)))
(test (vector) (ivs-difference (vector) (vector 1 10)))
(test (vector 1 10) (ivs-difference (vector 1 10) (vector)))
(test (vector 1 5) (ivs-difference (vector 1 5) (vector 6 10)))
(test (vector 1 4) (ivs-difference (vector 1 10) (vector 5 15)))
(test (vector) (ivs-difference (vector 1 10) (vector 1 20)))
(test (vector 1 4 8 10 15 20) 
          (ivs-difference (vector 1 10 15 20) (vector 5 7 12 14)))

;; Test with Unicode code points (realistic use case)
(let ((ascii-letters (list->ivs 
                      (append (list 65 66 67 68 69 70)  ; A-F
                              (list 97 98 99 100 101 102))))  ; a-f
      (hex-digits (list->ivs
                    (append (list 48 49 50 51 52 53 54 55 56 57)  ; 0-9
                            (list 65 66 67 68 69 70)  ; A-F
                            (list 97 98 99 100 101 102)))))  ; a-f
  (test #t (ivs-contains? ascii-letters 65))   ; 'A'
  (test #t (ivs-contains? ascii-letters 97))   ; 'a'
  (test #f (ivs-contains? ascii-letters 48))   ; '0'
  (test #t (ivs-contains? hex-digits 48))      ; '0'
  (test #t (ivs-contains? hex-digits 65))      ; 'A'
  (test 12 (ivs-size ascii-letters))
  (test 22 (ivs-size hex-digits))
  (test 12 (ivs-size (ivs-intersection ascii-letters hex-digits)))
  (test 22 (ivs-size (ivs-union ascii-letters hex-digits)))
  (test 10 (ivs-size (ivs-difference hex-digits ascii-letters))))

;; Edge cases: maximum values in range
(let ((max-range (vector 0 16777215)))  ; 2^24 - 1
  (test #t (ivs-contains? max-range 0))
  (test #t (ivs-contains? max-range 16777215))
  (test #t (ivs-contains? max-range 8388607))
  (test 16777216 (ivs-size max-range)))

(display "\n--- Testing Corner Cases for Construction ---\n")

(test "list->ivs: Single point repeated"
  (ranges->ivs '(5 . 5))
  (list->ivs '(5 5 5 5 5)))

(test "list->ivs: Merge three adjacent ranges"
  (ranges->ivs '(1 . 10))
  (list->ivs '(1 2 3 4 5 6 7 8 9 10)))

(test "list->ivs: Large numbers"
  (ranges->ivs '(1000000 . 1000001))
  (list->ivs '(1000000 1000001)))

(test "list->ivs: Zero handling"
  (ranges->ivs '(0 . 2))
  (list->ivs '(0 1 2)))

(display "\n--- Testing Corner Cases for Union ---\n")

(let ((s1 (ranges->ivs '(10 . 20) '(30 . 40)))
      (s2 (ranges->ivs '(21 . 29))))
  (test "union: Fills a gap exactly"
    (ranges->ivs '(10 . 40))
    (ivs-union s1 s2)))

(let ((s1 (ranges->ivs '(10 . 20)))
      (s2 (ranges->ivs '(20 . 30))))
  (test "union: Join at a single point (end to start)"
    (ranges->ivs '(10 . 30))
    (ivs-union s1 s2)))

(let ((s1 (ranges->ivs '(20 . 30)))
      (s2 (ranges->ivs '(10 . 20))))
  (test "union: Join at a single point (start to end)"
    (ranges->ivs '(10 . 30))
    (ivs-union s1 s2)))

(let ((s1 (ranges->ivs '(10 . 20) '(40 . 50)))
      (s2 (ranges->ivs '(30 . 39))))
  (test "union: Bridge two ranges"
    (ranges->ivs '(10 . 20) '(30 . 50))
    (ivs-union s1 s2)))

(let ((s1 (ranges->ivs '(10 . 20) '(41 . 50)))
      (s2 (ranges->ivs '(21 . 40))))
  (test "union: Bridge two ranges exactly"
    (ranges->ivs '(10 . 50))
    (ivs-union s1 s2)))

(test "union: Union of three overlapping ranges"
  (ranges->ivs '(10 . 40))
  (ivs-union (ranges->ivs '(10 . 20)) (ivs-union (ranges->ivs '(15 . 30)) (ranges->ivs '(25 . 40)))))

(test "union: One range completely containing two others"
  (ranges->ivs '(0 . 100))
  (ivs-union (ranges->ivs '(0 . 100)) (ranges->ivs '(10 . 20) '(80 . 90))))

(display "\n--- Testing Corner Cases for Intersection ---\n")

(let ((s1 (ranges->ivs '(10 . 20)))
      (s2 (ranges->ivs '(20 . 30))))
  (test "intersection: Overlap at a single point"
    (ranges->ivs '(20 . 20))
    (ivs-intersection s1 s2)))

(let ((s1 (ranges->ivs '(10 . 19)))
      (s2 (ranges->ivs '(20 . 30))))
  (test "intersection: Adjacent, non-overlapping"
    (ranges->ivs)
    (ivs-intersection s1 s2)))

(let ((s1 (ranges->ivs '(10 . 20) '(30 . 40)))
      (s2 (ranges->ivs '(0 . 100))))
  (test "intersection: One set completely contained"
    s1
    (ivs-intersection s1 s2)))

(let ((s1 (ranges->ivs '(10 . 20) '(30 . 40)))
      (s2 (ranges->ivs '(15 . 35))))
  (test "intersection: Span across a gap"
    (ranges->ivs '(15 . 20) '(30 . 35))
    (ivs-intersection s1 s2)))

(let ((s1 (ranges->ivs '(10 . 20) '(30 . 40) '(50 . 60)))
      (s2 (ranges->ivs '(15 . 15) '(55 . 55))))
  (test "intersection: Single points within ranges"
    (ranges->ivs '(15 . 15) '(55 . 55))
    (ivs-intersection s1 s2)))

(test "intersection: Empty with non-empty"
  (ranges->ivs)
  (ivs-intersection (ranges->ivs) (ranges->ivs '(1 . 100))))

(test "intersection: Empty with empty"
  (ranges->ivs)
  (ivs-intersection (ranges->ivs) (ranges->ivs)))

(display "\n--- Testing Corner Cases for Difference ---\n")

(let ((s1 (ranges->ivs '(10 . 40)))
      (s2 (ranges->ivs '(10 . 40))))
  (test "difference: Subtracting itself"
    (ranges->ivs)
    (ivs-difference s1 s2)))

(let ((s1 (ranges->ivs '(10 . 40)))
      (s2 (ranges->ivs '(20 . 20))))
  (test "difference: Subtracting a single point from middle"
    (ranges->ivs '(10 . 19) '(21 . 40))
    (ivs-difference s1 s2)))

(let ((s1 (ranges->ivs '(10 . 40)))
      (s2 (ranges->ivs '(10 . 10))))
  (test "difference: Subtracting the first point"
    (ranges->ivs '(11 . 40))
    (ivs-difference s1 s2)))

(let ((s1 (ranges->ivs '(10 . 40)))
      (s2 (ranges->ivs '(40 . 40))))
  (test "difference: Subtracting the last point"
    (ranges->ivs '(10 . 39))
    (ivs-difference s1 s2)))

(let ((s1 (ranges->ivs '(10 . 40)))
      (s2 (ranges->ivs '(0 . 9))))
  (test "difference: Subtractor is adjacent before"
    s1
    (ivs-difference s1 s2)))

(let ((s1 (ranges->ivs '(10 . 40)))
      (s2 (ranges->ivs '(41 . 50))))
  (test "difference: Subtractor is adjacent after"
    s1
    (ivs-difference s1 s2)))

(let ((s1 (ranges->ivs '(10 . 40)))
      (s2 (ranges->ivs '(0 . 10))))
  (test "difference: Subtractor overlaps at start"
    (ranges->ivs '(11 . 40))
    (ivs-difference s1 s2)))

(let ((s1 (ranges->ivs '(10 . 40)))
      (s2 (ranges->ivs '(40 . 50))))
  (test "difference: Subtractor overlaps at end"
    (ranges->ivs '(10 . 39))
    (ivs-difference s1 s2)))

(let ((s1 (ranges->ivs '(10 . 20) '(30 . 40)))
      (s2 (ranges->ivs '(15 . 35))))
  (test "difference: Subtractor spans a gap"
    (ranges->ivs '(10 . 14) '(36 . 40))
    (ivs-difference s1 s2)))

(let ((s1 (ranges->ivs '(10 . 20) '(30 . 40)))
      (s2 (ranges->ivs '(0 . 100))))
  (test "difference: Subtractor contains all ranges"
    (ranges->ivs)
    (ivs-difference s1 s2)))

(let ((s1 (ranges->ivs '(10 . 100)))
      (s2 (ranges->ivs '(20 . 30) '(50 . 60) '(80 . 90))))
  (test "difference: Creates multiple holes ('swiss cheese')"
    (ranges->ivs '(10 . 19) '(31 . 49) '(61 . 79) '(91 . 100))
    (ivs-difference s1 s2)))

(let ((s1 (ranges->ivs '(20 . 30) '(50 . 60)))
      (s2 (ranges->ivs '(10 . 100))))
  (test "difference: Subtract from a larger container"
    (ranges->ivs)
    (ivs-difference s1 s2)))

(display "\n--- Testing Corner Cases for Subset (ivs<=?) ---\n")

(let ((s1 (ranges->ivs '(10 . 20) '(30 . 40)))
      (s2 (ranges->ivs '(10 . 40))))
  (test "subset: Gapped set is not subset of filled gap"
    #f
    (ivs<=? s2 s1)))

(let ((s1 (ranges->ivs '(10 . 20)))
      (s2 (ranges->ivs '(10 . 19))))
  (test "subset: Off by one at end"
    #t
    (ivs<=? s2 s1)))

(let ((s1 (ranges->ivs '(10 . 20)))
      (s2 (ranges->ivs '(11 . 20))))
  (test "subset: Off by one at start"
    #t
    (ivs<=? s2 s1)))

(let ((s1 (ranges->ivs '(10 . 20) '(30 . 40)))
      (s2 (ranges->ivs '(30 . 40))))
  (test "subset: One of multiple ranges"
    #t
    (ivs<=? s2 s1)))

(test "subset: Non-empty is not subset of empty"
  #f
  (ivs<=? (ranges->ivs '(1 . 1)) (ranges->ivs)))

(test "subset: Both empty"
  #t
  (ivs<=? (ranges->ivs) (ranges->ivs)))

(display "\n--- Testing Mixed Operations ---\n")

(let* ((s_a (ranges->ivs '(0 . 10) '(20 . 30)))
       (s_b (ranges->ivs '(5 . 15) '(25 . 35)))
       (s_c (ranges->ivs '(8 . 28))))
  (test "mixed: (A union B) intersect C"
    (ranges->ivs '(8 . 15) '(20 . 28))
    (ivs-intersection (ivs-union s_a s_b) s_c)))

(let* ((s_a (ranges->ivs '(0 . 100)))
       (s_b (ranges->ivs '(10 . 20) '(80 . 90)))
       (s_c (ranges->ivs '(0 . 15) '(85 . 100))))
  (test "mixed: (A diff B) union (A diff C)"
    (ranges->ivs '(0 . 9) '(16 . 84) '(91 . 100))
    (ivs-union (ivs-difference s_a s_b) (ivs-difference s_a s_c))))

(let* ((s_a (ranges->ivs '(0 . 100)))
       (s_b (ranges->ivs '(10 . 20) '(80 . 90)))
       (s_c (ranges->ivs '(0 . 15) '(85 . 100))))
  (test "mixed: A diff (B intersect C)"
    (ranges->ivs '(0 . 9) '(16 . 84) '(91 . 100))
    (ivs-difference s_a (ivs-intersection s_b s_c))))

(test "mixed: De Morgan's Law (A union B) vs (A' intersect B')'"
  (let* ((universe (ranges->ivs '(0 . 1000)))
         (s_a (ranges->ivs '(100 . 200) '(500 . 600)))
         (s_b (ranges->ivs '(150 . 250) '(700 . 800)))
         (a_union_b (ivs-union s_a s_b))
         (a_prime (ivs-difference universe s_a))
         (b_prime (ivs-difference universe s_b))
         (primes_intersect (ivs-intersection a_prime b_prime)))
    (ivs-difference universe primes_intersect))
  (ivs-union (ranges->ivs '(100 . 200) '(500 . 600)) (ranges->ivs '(150 . 250) '(700 . 800))))

(display "\n--- Automated tests ---\n")

(define (run-fuzzy-test number-of-iterations)
  (let ([state 314159] [tap_mask 32] [high_bit 4194304])
    (define (next-random)
      (let* ([bit0 (modulo state 2)]
             [bit5 (modulo (quotient state tap_mask) 2)]
             [new-bit (modulo (+ bit0 bit5) 2)])
        (set! state (+ (quotient state 2) (* new-bit high_bit)))
        state))
    (define (random-int n) (modulo (next-random) n))
    
    ;; Generate a random IVS with up to max-ranges intervals
    (define (random-ivs max-val max-ranges)
      (let* ([num-ranges (+ 1 (random-int max-ranges))]
             [points (let loop ([n (* 2 num-ranges)] [acc '()])
                       (if (zero? n)
                           acc
                           (loop (- n 1) (cons (random-int max-val) acc))))]
             [sorted (list-sort < points)]
             [pairs (let pair-loop ([pts sorted] [acc '()])
                      (if (or (null? pts) (null? (cdr pts)))
                          (reverse acc)
                          (let ([f (car pts)]
                                [l (cadr pts)])
                            (if (<= f l)
                                (pair-loop (cddr pts) (cons (cons f l) acc))
                                (pair-loop (cddr pts) acc)))))])
        ;; Remove overlapping/touching intervals by converting to list and back
        (list->ivs (apply append (map (lambda (iv)
                                         (let loop ([i (car iv)] [acc '()])
                                           (if (> i (cdr iv))
                                               (reverse acc)
                                               (loop (+ i 1) (cons i acc)))))
                                       pairs)))))
    
    ;; Run tests
    (display "Running fuzzy tests...")
    (newline)
    
    (let test-loop ([i 0])
      (when (< i number-of-iterations)
        (let* ([max-val 1000]
               [max-ranges 10]
               [s1 (random-ivs max-val max-ranges)]
               [s2 (random-ivs max-val max-ranges)]
               [s3 (random-ivs max-val max-ranges)])
          
          ;; De Morgan's Laws
          ;; not(A ∪ B) = not(A) ∩ not(B)
          (let ([universe (vector 0 max-val)])
            (test (ivs-intersection (ivs-difference universe s1)
                                    (ivs-difference universe s2))
                  (ivs-difference universe (ivs-union s1 s2))))
          
          ;; not(A ∩ B) = not(A) ∪ not(B)
          (let ([universe (vector 0 max-val)])
            (test (ivs-union (ivs-difference universe s1)
                             (ivs-difference universe s2))
                  (ivs-difference universe (ivs-intersection s1 s2))))
          
          ;; Commutative Laws
          (test (ivs-union s1 s2)
                (ivs-union s2 s1))
          
          (test (ivs-intersection s1 s2)
                (ivs-intersection s2 s1))
          
          ;; Associative Laws
          (test (ivs-union (ivs-union s1 s2) s3)
                (ivs-union s1 (ivs-union s2 s3)))
          
          (test (ivs-intersection (ivs-intersection s1 s2) s3)
                (ivs-intersection s1 (ivs-intersection s2 s3)))
          
          ;; Distributive Laws
          (test (ivs-union s1 (ivs-intersection s2 s3))
                (ivs-intersection (ivs-union s1 s2) (ivs-union s1 s3)))
          
          (test (ivs-intersection s1 (ivs-union s2 s3))
                (ivs-union (ivs-intersection s1 s2) (ivs-intersection s1 s3)))
          
          ;; Identity Laws
          (test s1 (ivs-union s1 (vector)))
          (test s1 (ivs-union (vector) s1))
          (test (vector) (ivs-intersection s1 (vector)))
          (test (vector) (ivs-intersection (vector) s1))
          
          ;; Idempotent Laws
          (test s1 (ivs-union s1 s1))
          (test s1 (ivs-intersection s1 s1))
          
          ;; Complement Laws (with universe)
          (let ([universe (vector 0 max-val)])
            (test universe
                  (ivs-union s1 (ivs-difference universe s1)))
            (test (vector)
                  (ivs-intersection s1 (ivs-difference universe s1))))
          
          ;; Absorption Laws
          (test s1 (ivs-union s1 (ivs-intersection s1 s2)))
          (test s1 (ivs-intersection s1 (ivs-union s1 s2)))
          
          ;; Difference Laws
          (test (vector) (ivs-difference s1 s1))
          (test s1 (ivs-difference s1 (vector)))
          (test (ivs-intersection s1 (ivs-difference s2 s3))
                (ivs-difference (ivs-intersection s1 s2) s3))
          
          ;; Subset Laws
          (test #t (ivs<=? (vector) s1))
          (test #t (ivs<=? s1 s1))
          (test #t (ivs<=? (ivs-intersection s1 s2) s1))
          (test #t (ivs<=? (ivs-intersection s1 s2) s2))
          (test #t (ivs<=? s1 (ivs-union s1 s2)))
          (test #t (ivs<=? s2 (ivs-union s1 s2)))
          
          ;; If A <= B then A \/ B = B
          (when (ivs<=? s1 s2)
            (test s2 (ivs-union s1 s2)))
          
          ;; If A <= B then A /\ B = A
          (when (ivs<=? s1 s2)
            (test s1 (ivs-intersection s1 s2)))
          
          ;; Disjointness properties
          (test (ivs-disjoint? s1 s2)
                (ivs=? (ivs-intersection s1 s2) (vector)))
          
          (test (not (ivs=? (ivs-difference s1 s2) (vector)))
                (not (ivs<=? s1 s2)))
          
          ;; Size properties
          (test #t (<= (ivs-size (ivs-intersection s1 s2))
                       (min (ivs-size s1) (ivs-size s2))))
          
          (test #t (>= (ivs-size (ivs-union s1 s2))
                       (max (ivs-size s1) (ivs-size s2))))
          
          (test (+ (ivs-size s1) (ivs-size s2))
                (+ (ivs-size (ivs-union s1 s2))
                   (ivs-size (ivs-intersection s1 s2))))
          
          ;; Equality properties
          (test #t (ivs=? s1 s1))
          (test (ivs=? s1 s2) (ivs=? s2 s1))
          (when (and (ivs=? s1 s2) (ivs=? s2 s3))
            (test #t (ivs=? s1 s3)))
          
          ;; Round-trip property
          (test s1 (list->ivs (ivs->list s1)))
          
          ;; Membership consistency
          (let ([test-element (random-int max-val)])
            (test (ivs-contains? s1 test-element)
                  (not (not (member test-element (ivs->list s1)))))
            
            ;; If element in intersection, it's in both sets
            (when (ivs-contains? (ivs-intersection s1 s2) test-element)
              (test #t (ivs-contains? s1 test-element))
              (test #t (ivs-contains? s2 test-element)))
            
            ;; If element in union, it's in at least one set
            (when (ivs-contains? (ivs-union s1 s2) test-element)
              (test #t (or (ivs-contains? s1 test-element)
                           (ivs-contains? s2 test-element))))
            
            ;; If element in difference s1-s2, it's in s1 but not s2
            (when (ivs-contains? (ivs-difference s1 s2) test-element)
              (test #t (ivs-contains? s1 test-element))
              (test #f (ivs-contains? s2 test-element))))
          
          (test-loop (+ i 1))))))
  
  (display "Fuzzy tests completed.")
  (newline))

;; Run the fuzzy tester
(run-fuzzy-test 10)

(display "\n--- All tests complete. ---\n")

(test-end)
