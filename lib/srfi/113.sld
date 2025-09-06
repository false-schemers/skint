
(define-library (srfi 113)
  (import 
   (scheme base) (srfi 1) (srfi 125) 
   (except (srfi 128) string-hash string-ci-hash))
  (export
   set set-contains? set-unfold
   set? set-empty? set-disjoint?
   set-member set-element-comparator
   set-adjoin set-adjoin! set-replace set-replace!
   set-delete set-delete! set-delete-all set-delete-all!
   set-search!
   set-size set-find set-count set-any? set-every?
   set-map set-for-each set-fold set-filter set-filter!
   set-remove set-remove! set-partition set-partition!
   set-copy set->list list->set list->set!
   set=? set<? set>? set<=? set>=?
   set-union set-intersection set-difference set-xor
   set-union! set-intersection! set-difference! set-xor!
   (rename the-set-comparator set-comparator)
   bag bag-contains? bag-unfold
   bag? bag-empty? bag-disjoint?
   bag-member bag-element-comparator
   bag-adjoin bag-adjoin! bag-replace bag-replace!
   bag-delete bag-delete! bag-delete-all bag-delete-all!
   bag-search!
   bag-size bag-find bag-count bag-any? bag-every?
   bag-map bag-for-each bag-fold bag-filter bag-filter!
   bag-remove bag-remove! bag-partition bag-partition!
   bag-copy bag->list list->bag list->bag!
   bag=? bag<? bag>? bag<=? bag>=?
   bag-union bag-intersection bag-difference bag-xor
   bag-union! bag-intersection! bag-difference! bag-xor!
   (rename the-bag-comparator bag-comparator)
   bag-unique-size
   bag-sum bag-sum! bag-product bag-product! bag-element-count
   bag-for-each-unique bag-fold-unique bag-increment! bag-decrement!
   bag->set set->bag set->bag!
   bag->alist alist->bag)

(begin
;;; Alex Shinn after John Cowan

;; sets

(define-record-type Set (make-set table comparator) set?
  (table set-table)
  (comparator set-comparator))

(define (set comparator . elts)
  (let ((res (make-hash-table comparator)))
    (for-each (lambda (x) (hash-table-set! res x x)) elts)
    (make-set res comparator)))

(define (set-unfold comparator stop? mapper successor seed)
  (let ((mapper (lambda (acc) (let ((elt (mapper acc))) (values elt elt)))))
    (make-set (hash-table-unfold stop? mapper successor seed comparator)
              comparator)))

(define (set-contains? set element)
  (hash-table-contains? (set-table set) element))

(define (set-empty? set)
  (zero? (set-size set)))

(define (set-disjoint? set1 set2)
  (if (< (hash-table-size (set-table set2))
         (hash-table-size (set-table set1)))
      (set-disjoint? set2 set1)
      (let ((ht (set-table set2)))
        (not (hash-table-find (lambda (key value) (hash-table-contains? ht key))
                              (set-table set1)
                              (lambda () #f))))))

(define (set-member set element default)
  (hash-table-ref/default (set-table set) element default))

(define (set-element-comparator set)
  (set-comparator set))

(define (set-adjoin set . elts)
  (apply set-adjoin! (set-copy set) elts))

(define (set-adjoin! set . elts)
  (for-each (lambda (elt) (hash-table-set! (set-table set) elt elt)) elts)
  set)

(define (set-replace set element)
  (set-replace! (set-copy set) element))

(define (set-replace! set element)
  (when (hash-table-contains? (set-table set) element)
    (hash-table-delete! (set-table set) element)
    (hash-table-set! (set-table set) element element))
  set)

(define (set-delete set . elts)
  (set-delete-all set elts))

(define (set-delete! set . elts)
  (set-delete-all! set elts))

(define (set-delete-all set element-list)
  (set-delete-all! (set-copy set) element-list))

(define (set-delete-all! set element-list)
  (for-each (lambda (elt) (hash-table-delete! (set-table set) elt))
            element-list)
  set)

(define set-search!
  (let ((not-found (list 'not-found)))
    (lambda (set element failure success)
      (let ((elt (hash-table-ref/default (set-table set) element not-found)))
        (if (eq? elt not-found)
            (failure (lambda (obj)
                       (hash-table-set! (set-table set) element element)
                       (values set obj))
                     (lambda (obj)
                       (values set obj)))
            (success elt
                     (lambda (new-element obj)
                       (hash-table-delete! (set-table set) elt)
                       (hash-table-set! (set-table set) new-element new-element)
                       (values set obj))
                     (lambda (obj)
                       (hash-table-delete! (set-table set) element)
                       (values set obj))))))))

(define (set-size set)
  (hash-table-size (set-table set)))

(define (set-find predicate set failure)
  (call-with-current-continuation
   (lambda (return)
     (hash-table-for-each
      (lambda (elt _) (if (predicate elt) (return elt)))
      (set-table set))
     (failure))))

(define (set-count predicate set)
  (hash-table-count (lambda (key value) (predicate key)) (set-table set)))

(define (set-any? predicate set)
  (and (hash-table-find (lambda (key value) (predicate key))
                        (set-table set)
                        (lambda () #f))
       #t))

(define (set-every? predicate set)
  (not (set-any? (lambda (x) (not (predicate x))) set)))

(define (set-map comparator proc s)
  (set-fold (lambda (elt res) (set-adjoin! res (proc elt)))
            (set comparator)
            s))

(define (set-for-each proc set)
  (hash-table-for-each (lambda (elt _) (proc elt)) (set-table set)))

(define (set-fold proc nil set)
  (hash-table-fold (lambda (elt _ acc) (proc elt acc)) nil (set-table set)))

(define (set-filter predicate st)
  (set-fold (lambda (elt res)
              (if (predicate elt) (set-adjoin! res elt) res))
            (set (set-comparator st))
            st))

(define set-filter! set-filter)

(define (set-remove predicate set)
  (set-filter (lambda (elt) (not (predicate elt))) set))

(define set-remove! set-remove)

(define (set-partition predicate set)
  (values (set-filter predicate set)
          (set-remove predicate set)))

(define set-partition! set-partition)

(define (set-copy set)
  (make-set (hash-table-copy (set-table set))
            (set-comparator set)))

(define (set->list set)
  (hash-table-keys (set-table set)))

(define (list->set comparator list)
  (fold (lambda (elt set) (set-adjoin! set elt)) (set comparator) list))

(define (list->set! set list)
  (fold (lambda (elt set) (set-adjoin! set elt)) set list))

(define (comparable-sets? set1 set2)
  (or (eq? (set-comparator set1) (set-comparator set2))
      (error "can't compare sets with different comparators" set1 set2)))

(define (set=? set1 . sets)
  (or (null? sets)
      (and (comparable-sets? set1 (car sets))
           (= (set-size set1) (set-size (car sets)))
           (set-every? (lambda (elt) (set-contains? set1 elt)) (car sets))
           (apply set=? sets))))

(define (set<? set1 . sets)
  (or (null? sets)
      (and (comparable-sets? set1 (car sets))
           (< (set-size set1) (set-size (car sets)))
           (set-every? (lambda (elt) (set-contains? (car sets) elt)) set1)
           (apply set<? sets))))

(define (set>? . sets)
  (apply set<? (reverse sets)))

(define (set<=? set1 . sets)
  (or (null? sets)
      (and (comparable-sets? set1 (car sets))
           (<= (set-size set1) (set-size (car sets)))
           (set-every? (lambda (elt) (set-contains? (car sets) elt)) set1)
           (apply set<=? sets))))

(define (set>=? . sets)
  (apply set<=? (reverse sets)))

(define (set-union set1 . sets)
  (apply set-union! (set-copy set1) sets))

(define (set-intersection set1 . sets)
  (apply set-intersection! (set-copy set1) sets))

(define (set-difference set1 . sets)
  (apply set-difference! (set-copy set1) sets))

(define (set-xor set1 set2)
  (set-xor! (set-copy set1) set2))

(define (set-union! set1 . sets)
  (if (null? sets)
      set1
      (and (comparable-sets? set1 (car sets))
           (apply set-union!
                  (set-fold (lambda (elt set) (set-adjoin! set elt)) set1 (car sets))
                  (cdr sets)))))

(define (set-intersection! set1 . sets)
  (if (null? sets)
      set1
      (and (comparable-sets? set1 (car sets))
           (apply set-intersection!
                  (set-filter! (lambda (elt) (set-contains? (car sets) elt)) set1)
                  (cdr sets)))))

(define (set-difference! set1 . sets)
  (if (null? sets)
      set1
      (and (comparable-sets? set1 (car sets))
           (apply set-difference!
                  (set-remove! (lambda (elt) (set-contains? (car sets) elt)) set1)
                  (cdr sets)))))

(define (set-xor! set1 set2)
  (and (comparable-sets? set1 set2)
       (set-union (set-remove (lambda (elt) (set-contains? set2 elt)) set1)
                  (set-remove (lambda (elt) (set-contains? set1 elt)) set2))))

(define the-set-comparator
  (make-comparator set? set=? set<? hash))

;; bags

(define-record-type Bag (make-bag table comparator) bag?
  (table bag-table)
  (comparator bag-comparator))

(define (bag comparator . elts)
  (let ((res (make-bag (make-hash-table comparator) comparator)))
    (for-each (lambda (x) (bag-adjoin! res x)) elts)
    res))

(define (bag-unfold comparator stop? mapper successor seed)
  (let ((mapper (lambda (acc) (let ((elt (mapper acc))) (values elt 1)))))
    (make-bag (hash-table-unfold stop? mapper successor seed comparator)
              comparator)))

(define (bag-contains? bag element)
  (hash-table-contains? (bag-table bag) element))

(define (bag-empty? bag)
  (zero? (bag-size bag)))

(define (bag-disjoint? bag1 bag2)
  (if (< (hash-table-size (bag-table bag2))
         (hash-table-size (bag-table bag1)))
      (bag-disjoint? bag2 bag1)
      (let ((ht (bag-table bag2)))
        (not (hash-table-find (lambda (key value) (hash-table-contains? ht key))
                              (bag-table bag1)
                              (lambda () #f))))))

(define (bag-member bag element default)
  ;; (let ((cell (hash-table-cell (bag-table bag) element #f)))
  ;;   (if cell (car cell) default))
  (if (hash-table-contains? (bag-table bag) element)
      element
      default))

(define (bag-element-comparator bag)
  (bag-comparator bag))

(define (bag-adjoin bag . elts)
  (apply bag-adjoin! (bag-copy bag) elts))

(define (bag-adjoin! bag . elts)
  (for-each (lambda (elt)
              (hash-table-update!/default (bag-table bag)
                                          elt
                                          (lambda (count) (+ 1 count))
                                          0))
            elts)
  bag)

(define (bag-replace bag element)
  (bag-replace! (bag-copy bag) element))

(define (bag-replace! bag element)
  (when (hash-table-contains? (bag-table bag) element)
    (hash-table-delete! (bag-table bag) element)
    (hash-table-set! (bag-table bag) element 1))
  bag)

(define (bag-delete bag . elts)
  (bag-delete-all bag elts))

(define (bag-delete! bag . elts)
  (bag-delete-all! bag elts))

(define (bag-delete-all bag element-list)
  (bag-delete-all! (bag-copy bag) element-list))

(define (bag-delete-all! bag element-list)
  (let ((ht (bag-table bag)))
    (for-each (lambda (elt)
                (let ((count (- (hash-table-ref/default ht elt 0) 1)))
                  (cond
                   ((positive? count) (hash-table-set! ht elt count))
                   ((zero? count) (hash-table-delete! ht elt)))))
              element-list))
  bag)

(define bag-search!
  (let ((not-found (list 'not-found)))
    (lambda (bag element failure success)
      (let ((elt (hash-table-ref/default (bag-table bag) element not-found)))
        (if (eq? elt not-found)
            (failure (lambda (obj)
                       (hash-table-set! (bag-table bag) element 1)
                       (values bag obj))
                     (lambda (obj)
                       (values bag obj)))
            (success elt
                     (lambda (new-element obj)
                       (hash-table-delete! (bag-table bag) element)
                       (bag-adjoin! bag new-element)
                       (values bag obj))
                     (lambda (obj)
                       (hash-table-delete! (bag-table bag) element)
                       (values bag obj))))))))

(define (bag-size bag)
  (hash-table-fold (bag-table bag) (lambda (elt count acc) (+ count acc)) 0))

(define (bag-find predicate bag failure)
  (call-with-current-continuation
   (lambda (return)
     (hash-table-for-each
      (lambda (elt count) (if (predicate elt) (return elt)))
      (bag-table bag))
     (failure))))

(define (bag-count predicate bag)
  (hash-table-fold (lambda (elt count acc) (+ acc (if (predicate elt) count 0)))
                   0
                   (bag-table bag)))

(define (bag-any? predicate bag)
  (and (hash-table-find (lambda (key value) (predicate key))
                        (bag-table bag)
                        (lambda () #f))
       #t))

(define (bag-every? predicate bag)
  (not (bag-any? (lambda (x) (not (predicate x))) bag)))

(define (bag-map comparator proc s)
  (bag-fold (lambda (elt res) (bag-adjoin! res (proc elt)))
            (bag comparator)
            s))

(define (bag-for-each proc bag)
  (hash-table-for-each (lambda (elt count)
                         (let lp ((i count))
                           (when (positive? i)
                             (proc elt)
                             (lp (- i 1)))))
                       (bag-table bag)))

(define (bag-fold proc nil bag)
  (hash-table-fold (lambda (elt count acc)
                     (let lp ((i count) (acc acc))
                       (if (zero? i)
                           acc
                           (lp (- i 1) (proc elt acc)))))
                   nil
                   (bag-table bag)))

(define (bag-filter predicate st)
  (bag-fold (lambda (elt res)
              (if (predicate elt) (bag-adjoin! res elt) res))
            (bag (bag-comparator st))
            st))

(define bag-filter! bag-filter)

(define (bag-remove predicate bag)
  (bag-filter (lambda (elt) (not (predicate elt))) bag))

(define bag-remove! bag-remove)

(define (bag-partition predicate bag)
  (values (bag-filter predicate bag)
          (bag-remove predicate bag)))

(define bag-partition! bag-partition)

(define (bag-copy bag)
  (make-bag (hash-table-copy (bag-table bag))
            (bag-comparator bag)))

(define (bag->list bag)
  (hash-table-keys (bag-table bag)))

(define (list->bag comparator list)
  (fold (lambda (elt bag) (bag-adjoin! bag elt)) (bag comparator) list))

(define (list->bag! bag list)
  (fold (lambda (elt bag) (bag-adjoin! bag elt)) bag list))

(define (comparable-bags? bag1 bag2)
  (or (eq? (bag-comparator bag1) (bag-comparator bag2))
      (error "can't compare bags with different comparators" bag1 bag2)))

(define (bag=? bag1 . bags)
  (or (null? bags)
      (and (comparable-bags? bag1 (car bags))
           (= (bag-size bag1) (bag-size (car bags)))
           (bag-every? (lambda (elt) (bag-contains? bag1 elt)) (car bags))
           (apply bag=? bags))))

(define (bag<? bag1 . bags)
  (or (null? bags)
      (and (comparable-bags? bag1 (car bags))
           (< (bag-size bag1) (bag-size (car bags)))
           (bag-every? (lambda (elt) (bag-contains? (car bags) elt)) bag1)
           (apply bag<? bags))))

(define (bag>? . bags)
  (apply bag<? (reverse bags)))

(define (bag<=? bag1 . bags)
  (or (null? bags)
      (and (comparable-bags? bag1 (car bags))
           (<= (bag-size bag1) (bag-size (car bags)))
           (bag-every? (lambda (elt) (bag-contains? (car bags) elt)) bag1)
           (apply bag<=? bags))))

(define (bag>=? . bags)
  (apply bag<=? (reverse bags)))

(define (bag-union bag1 . bags)
  (apply bag-union! (bag-copy bag1) bags))

(define (bag-intersection bag1 . bags)
  (apply bag-intersection! (bag-copy bag1) bags))

(define (bag-difference bag1 . bags)
  (apply bag-difference! (bag-copy bag1) bags))

(define (bag-xor bag1 bag2)
  (bag-xor! (bag-copy bag1) bag2))

(define (bag-union! bag1 . bags)
  (if (null? bags)
      bag1
      (and (comparable-bags? bag1 (car bags))
           (begin
             (hash-table-for-each
              (lambda (elt count)
                (hash-table-update!/default (bag-table bag1)
                                            elt
                                            (lambda (c) (max c count))
                                            count))
              (bag-table (car bags)))
             (apply bag-union! bag1 (cdr bags))))))

(define (bag-intersection! bag1 . bags)
  (if (null? bags)
      bag1
      (and (comparable-bags? bag1 (car bags))
           (let ((ht (bag-table (car bags))))
             (hash-table-for-each
              (lambda (elt count)
                (let ((count2 (min count (hash-table-ref/default ht elt 0))))
                  (if (positive? count2)
                      (hash-table-set! (bag-table bag1) elt count2)
                      (hash-table-delete! (bag-table bag1) elt))))
              (bag-table bag1))
             (apply bag-intersection! bag1 (cdr bags))))))

(define (bag-difference! bag1 . bags)
  (if (null? bags)
      bag1
      (and (comparable-bags? bag1 (car bags))
           (let ((ht (bag-table (car bags))))
             (hash-table-for-each
              (lambda (elt count)
                (let ((count2 (- count (hash-table-ref/default ht elt 0))))
                  (if (positive? count2)
                      (hash-table-set! (bag-table bag1) elt count2)
                      (hash-table-delete! (bag-table bag1) elt))))
              (bag-table bag1))
             (apply bag-difference! bag1 (cdr bags))))))

(define (bag-xor! bag1 bag2)
  (and (comparable-bags? bag1 bag2)
       (let ((ht1 (bag-table bag1))
             (ht2 (bag-table bag2)))
         (hash-table-for-each
          (lambda (elt count)
            (let ((count2 (abs (- count (hash-table-ref/default ht1 elt 0)))))
              (if (positive? count2)
                  (hash-table-set! ht1 elt count2)
                  (hash-table-delete! ht1 elt))))
          ht2)
         bag1)))

(define (bag-sum bag1 . bags)
  (apply bag-sum! (bag-copy bag1) bags))

(define (bag-sum! bag1 . bags)
  (if (null? bags)
      bag1
      (and (comparable-bags? bag1 (car bags))
           (begin
             (hash-table-for-each
              (lambda (elt count)
                (hash-table-update!/default (bag-table bag1)
                                            elt
                                            (lambda (c) (+ c count))
                                            count))
              (bag-table (car bags)))
             (apply bag-sum! bag1 (cdr bags))))))

(define (bag-product n bag)
  (bag-product! n (bag-copy bag)))

(define (bag-product! n bag)
  (for-each
   (lambda (elt)
     (hash-table-update! (bag-table bag) elt (lambda (count) (* n count))))
   (hash-table-keys (bag-table bag)))
  bag)

(define (bag-unique-size bag)
  (hash-table-size (bag-table bag)))

(define (bag-element-count bag element)
  (hash-table-ref/default (bag-table bag) element 0))

(define (bag-for-each-unique proc bag)
  (hash-table-for-each proc (bag-table bag)))

(define (bag-fold-unique proc nil bag)
  (hash-table-fold proc nil (bag-table bag)))

(define (bag-increment! bag element count)
  (let* ((ht (bag-table bag))
         (count2 (+ count (hash-table-ref/default ht element 0))))
    (if (positive? count2)
        (hash-table-set! ht element count2)
        (hash-table-delete! ht element))
    bag)) ;[esl] return the modified bag!

(define (bag-decrement! bag element count)
  (bag-increment! bag element (- count)))

(define (bag->set bag)
  (let ((ht (hash-table-copy (bag-table bag))))
    (hash-table-map! (lambda (key count) key) ht)
    (make-set ht (bag-comparator bag))))

(define (set->bag set)
  (set->bag! (bag (set-comparator set)) set))

(define (set->bag! bag set)
  (set-for-each (lambda (elt) (bag-adjoin! bag elt)) set)
  bag)

(define (bag->alist bag)
  (hash-table->alist (bag-table bag)))

(define (alist->bag comparator alist)
  (let ((res (bag comparator)))
    (for-each (lambda (x) (bag-increment! res (car x) (cdr x))) alist)
    res))

(define the-bag-comparator
  (make-comparator bag? bag=? bag<? hash))

))

