;; Copyright (C) Marc Nieper-Wiﬂkirchen (2016).  All Rights Reserved. 

;; Permission is hereby granted, free of charge, to any person
;; obtaining a copy of this software and associated documentation
;; files (the "Software"), to deal in the Software without
;; restriction, including without limitation the rights to use, copy,
;; modify, merge, publish, distribute, sublicense, and/or sell copies
;; of the Software, and to permit persons to whom the Software is
;; furnished to do so, subject to the following conditions:

;; The above copyright notice and this permission notice shall be
;; included in all copies or substantial portions of the Software.

;; THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
;; EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
;; MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
;; NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
;; BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
;; ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
;; CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
;; SOFTWARE.

(define-library (srfi 146)
  (export mapping mapping-unfold
          mapping/ordered mapping-unfold/ordered
          mapping? mapping-contains? mapping-empty? mapping-disjoint?
          mapping-ref mapping-ref/default mapping-key-comparator
          mapping-adjoin mapping-adjoin!
          mapping-set mapping-set!
          mapping-replace mapping-replace!
          mapping-delete mapping-delete! mapping-delete-all mapping-delete-all!
          mapping-intern mapping-intern!
          mapping-update mapping-update! mapping-update/default mapping-update!/default
          mapping-pop mapping-pop!
          mapping-search mapping-search!
          mapping-size mapping-find mapping-count mapping-any? mapping-every?
          mapping-keys mapping-values mapping-entries
          mapping-map mapping-map->list mapping-for-each mapping-fold
          mapping-filter mapping-filter!
          mapping-remove mapping-remove!
          mapping-partition mapping-partition!
          mapping-copy mapping->alist alist->mapping alist->mapping!
          alist->mapping/ordered alist->mapping/ordered!
          mapping=? mapping<? mapping>? mapping<=? mapping>=?
          mapping-union mapping-intersection mapping-difference mapping-xor
          mapping-union! mapping-intersection! mapping-difference! mapping-xor!
          make-mapping-comparator
          mapping-comparator
          mapping-min-key mapping-max-key
          mapping-min-value mapping-max-value
          mapping-key-predecessor mapping-key-successor
          mapping-range= mapping-range< mapping-range> mapping-range<= mapping-range>=
          mapping-range=! mapping-range<! mapping-range>! mapping-range<=! mapping-range>=!
          mapping-split
          mapping-catenate mapping-catenate!
          mapping-map/monotone mapping-map/monotone!
          mapping-fold/reverse
          comparator?)
  (import (scheme base)
          (scheme case-lambda)
          (srfi 1)
          (srfi 2)
          (srfi 8)
          (srfi 121)
          (srfi 128)
          (srfi 145))

(begin

;; rbtree

;; Copyright (C) Marc Nieper-Wiﬂkirchen (2016).  All Rights Reserved. 

;; Permission is hereby granted, free of charge, to any person
;; obtaining a copy of this software and associated documentation
;; files (the "Software"), to deal in the Software without
;; restriction, including without limitation the rights to use, copy,
;; modify, merge, publish, distribute, sublicense, and/or sell copies
;; of the Software, and to permit persons to whom the Software is
;; furnished to do so, subject to the following conditions:

;; The above copyright notice and this permission notice shall be
;; included in all copies or substantial portions of the Software.

;; THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
;; EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
;; MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
;; NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
;; BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
;; ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
;; CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
;; SOFTWARE.

;; Concrete data types

(define (make-item key value) (vector key value))
(define (item-key item) (vector-ref item 0))
(define (item-value item) (vector-ref item 1))

(define (node color left item right) (vector color left item right))
(define (color node) (vector-ref node 0))
(define (left node) (vector-ref node 1))
(define (item node) (vector-ref node 2))
(define (right node) (vector-ref node 3))
(define (key node) (item-key (item node)))
(define (value node) (item-value (item node)))
(define (red left item right) (node 'red left item right))
(define (black left item right)
  (node 'black left item right))
(define (black-leaf) (black #f #f #f))
(define (white left item right)
  (node 'white left item right))
(define (white-leaf) (white #f #f #f))
(define (red? node) (eq? (color node) 'red))
(define (black? node) (eq? (color node) 'black))
(define (white? node) (eq? (color node) 'white))

;;; Tree matcher macros

(define-syntax tree-match
  (syntax-rules ()
    ((tree-match tree (pattern . expression*) ...)
     (compile-patterns (expression* ...) tree () (pattern ...)))))

(define-syntax compile-patterns
  (syntax-rules ()

    ((compile-patterns (expression* ...) tree (clauses ...) ())
     (call-with-current-continuation
      (lambda (return)
    (or (and-let* clauses
          (call-with-values
          (lambda () . expression*)
        return))
        ...
        (error "tree does not match any pattern" tree)))))
    
    ((compile-patterns e tree clauses* (pattern . pattern*))
     (compile-pattern tree pattern
              (add-pattern e tree clauses* pattern*)))))

(define-syntax add-pattern
  (syntax-rules ()
    ((add-pattern e tree (clauses ...) pattern* new-clauses)
     (compile-patterns e tree (clauses ... new-clauses) pattern*))))

(define-syntax compile-pattern
  (syntax-rules (_ and red? black? white? ? node red black white)

    ((compile-pattern tree (red? x) (k ...))
     (k ... (((red? tree)) (x tree))))

    ((compile-pattern tree (black? x) (k ...))
     (k ... (((black? tree)) (x tree))))

    ((compile-pattern tree (white? x) (k ...))
     (k ... (((white? tree)) (x tree))))

    ((compile-pattern tree (black) (k ...))
     (k ... (((black? tree)) ((not (item tree))))))

    ((compile-pattern tree (white) (k ...))
     (k ... (((white? tree)) ((not (item tree))))))

    ((compile-pattern tree (and pt ...) k*)
     (compile-subpatterns () ((t pt) ...)
              (compile-and-pattern tree t k*)))

    ((compile-pattern tree (node pc pa px pb) k*)
     (compile-subpatterns () ((c pc) (a pa) (x px) (b pb))
              (compile-node-pattern tree c a x b k*)))

    ((compile-pattern tree (red pa px pb) k*)
     (compile-subpatterns () ((a pa) (x px) (b pb))
              (compile-color-pattern red? tree a x b k*)))

    ((compile-pattern tree (black pa px pb) k*)
     (compile-subpatterns () ((a pa) (x px) (b pb))
              (compile-color-pattern black? tree a x b k*)))

    ((compile-pattern tree (white pa px pb) k*)
     (compile-subpatterns () ((a pa) (x px) (b pb))
              (compile-color-pattern white? tree a x b k*)))

    ((compile-pattern tree _ (k ...))
     (k ... ()))

    ((compile-pattern tree x (k ...))
     (k ... ((x tree))))))

(define-syntax compile-and-pattern
  (syntax-rules ()    
    ((compile-and-pattern tree t (k ...) clauses)
     (k ... ((t tree) . clauses)))))

(define-syntax compile-node-pattern
  (syntax-rules ()
    ((compile-node-pattern tree c a x b (k ...) clauses)
     (k ... (((item tree))
         (c (color tree))
         (a (left tree))
         (x (item tree))
         (b (right tree)) . clauses)))))

(define-syntax compile-color-pattern
  (syntax-rules ()
    ((compile-color-pattern pred? tree a x b (k ...) clauses)
     (k ... (((item tree))
         ((pred? tree))
         (a (left tree))
         (x (item tree))
         (b (right tree)) . clauses)))))

(define-syntax compile-subpatterns
  (syntax-rules ()
    
    ((compile-subpatterns clauses () (k ...))
     (k ... clauses))

    ((compile-subpatterns clauses ((tree pattern) . rest) k*)
     (compile-pattern tree pattern (add-subpattern clauses rest k*)))))

(define-syntax add-subpattern
  (syntax-rules ()
    ((add-subpattern (clause ...) rest k* clauses)
     (compile-subpatterns (clause ... . clauses) rest k*))))
    
;;; Tree recolouring procedures

(define (blacken tree)
  (tree-match tree
    ((red a x b)
     (black a x b))
    (t t)))

(define (redden tree)
  (tree-match tree
    ((black (black? a) x (black? b))
     (red a x b))
    (t t)))

(define (white->black tree)
  (tree-match tree
    ((white)
     (black-leaf))
    ((white a x b)
     (black a x b))))

;;; Exported identifiers

(define (make-tree) (black-leaf))

(define (tree-fold proc seed tree)
  (let loop ((acc seed) (tree tree))
    (tree-match tree
      ((black)
       acc)
      ((node _ a x b)
       (let*
       ((acc (loop acc a))
        (acc (proc (item-key x) (item-value x) acc))
        (acc (loop acc b)))
     acc)))))

(define (tree-fold/reverse proc seed tree)
  (let loop ((acc seed) (tree tree))
    (tree-match tree
      ((black)
       acc)
      ((node _ a x b)
       (let*
       ((acc (loop acc b))
        (acc (proc (item-key x) (item-value x) acc))
        (acc (loop acc a)))
     acc)))))

(define (tree-for-each proc tree)
  (tree-fold (lambda (key value acc)
           (proc key value))
         #f tree))

(define (tree-generator tree)
  (make-coroutine-generator
   (lambda (yield)
     (tree-for-each (lambda item (yield item)) tree))))

(define (identity obj) obj)

(define (tree-search comparator tree obj failure success)
  (receive (tree ret op)
      (let search ((tree (redden tree)))
    (tree-match tree      
      ((black)
       (failure
        ;; insert
        (lambda (new-key new-value ret)
          (values (red (black-leaf) (make-item new-key new-value) (black-leaf))
              ret
              balance))
        ;; ignore
        (lambda (ret)
          (values (black-leaf) ret identity))))
      
      ((and t (node c a x b))
       (let ((key (item-key x)))
         (comparator-if<=> comparator obj key
           
           (receive (a ret op) (search a)
         (values (op (node c a x b)) ret op))
           
           (success
        key
        (item-value x)
        ;; update
        (lambda (new-key new-value ret)
          (values (node c a (make-item new-key new-value) b)
              ret
              identity))
        ;; remove
        (lambda (ret)
          (values
           (tree-match t
             ((red (black) x (black))
              (black-leaf))
             ((black (red a x b) _ (black))
              (black a x b))
             ((black (black) _ (black))
              (white-leaf))
             (_
              (receive (x b) (min+delete b)
            (rotate (node c a x b)))))
           ret
           rotate)))
           
           (receive (b ret op) (search b)
         (values (op (node c a x b)) ret op)))))))
      
    (values (blacken tree) ret)))

(define (tree-key-successor comparator tree obj failure)
  (let loop ((return failure) (tree tree))
    (tree-match tree
      ((black)
       (return))
      ((node _ a x b)
       (let ((key (item-key x)))
     (comparator-if<=> comparator key obj
               (loop return b)
               (loop return b)
               (loop (lambda () key) a)))))))

(define (tree-key-predecessor comparator tree obj failure)
  (let loop ((return failure) (tree tree))
    (tree-match tree
      ((black)
       (return))
      ((node _ a x b)
       (let ((key (item-key x)))
     (comparator-if<=> comparator key obj
               (loop (lambda () key) b)
               (loop return a)
               (loop return a)))))))

(define (tree-map proc tree)
  (let loop ((tree tree))
    (tree-match tree
      ((black)
       (black-leaf))
      ((node c a x b)
       (receive (key value)
       (proc (item-key x) (item-value x))
     (node c (loop a) (make-item key value) (loop b)))))))


(define (tree-catenate tree1 pivot-key pivot-value tree2)
  (let ((pivot (make-item pivot-key pivot-value))
    (height1 (black-height tree1))
    (height2 (black-height tree2)))
    (cond
     ((= height1 height2)
      (black tree1 pivot tree2))
     ((< height1 height2)
      (blacken
       (let loop ((tree tree2) (depth (- height2 height1)))
     (if (zero? depth)
         (balance (red tree1 pivot tree))
         (balance
          (node (color tree) (loop (left tree) (- depth 1)) (item tree) (right tree)))))))
     (else
      (blacken
       (let loop ((tree tree1) (depth (- height1 height2)))
     (if (zero? depth)
         (balance (red tree pivot tree2))
         (balance
          (node (color tree) (left tree) (item tree) (loop (right tree) (- depth 1)))))))))))

(define (tree-split comparator tree obj)
  (let loop ((tree1 (black-leaf))
         (tree2 (black-leaf))
         (pivot1 #f)
         (pivot2 #f)
         (tree tree))
    (tree-match tree
      ((black)
       (let ((tree1 (catenate-left tree1 pivot1 (black-leaf)))
         (tree2 (catenate-right (black-leaf) pivot2 tree2)))
     (values tree1 tree1 (black-leaf) tree2 tree2)))
      ((node _ a x b)
       (comparator-if<=> comparator obj (item-key x)
             (loop tree1
                   (catenate-right (blacken b) pivot2 tree2)
                   pivot1
                   x
                   (blacken a))
             (let* ((tree1 (catenate-left tree1 pivot1 (blacken a)))
                (tree1+ (catenate-left tree1 x (black-leaf)))
                (tree2 (catenate-right (blacken b) pivot2 tree2))
                (tree2+ (catenate-right (black-leaf) x tree2)))
               (values tree1
                   tree1+
                   (black (black-leaf) x (black-leaf))
                   tree2+
                   tree2))
             (loop (catenate-left tree1 pivot1 (blacken a))
                   tree2
                   x
                   pivot2
                   (blacken b)))))))

(define (catenate-left tree1 item tree2)
  (if item
      (tree-catenate tree1 (item-key item) (item-value item) tree2)
      tree2))

(define (catenate-right tree1 item tree2)
  (if item
      (tree-catenate tree1 (item-key item) (item-value item) tree2)
      tree1))

(define (black-height tree)
  (let loop ((tree tree))
    (tree-match tree
      ((black)
       0)
      ((node red a x b)
       (loop b))
      ((node black a x b)
       (+ 1 (loop b))))))

(define (left-tree tree depth)
  (let loop ((parent #f) (tree tree) (depth depth))
    (if (zero? depth)
    (values parent tree)
    (loop tree (left tree) (- depth 1)))))

(define (right-tree tree depth)
  (let loop ((parent #f) (tree tree) (depth depth))
    (if (zero? depth)
    (values parent tree)
    (loop tree (right tree) (- depth 1)))))

;;; Helper procedures for deleting and balancing

(define (min+delete tree)
  (tree-match tree
    ((red (black) x (black))
     (values x (black-leaf)))
    ((black (black) x (black))
     (values x (white-leaf)))
    ((black (black) x (red a y b))
     (values x (black a y b)))
    ((node c a x b)
     (receive (v a) (min+delete a)
       (values v (rotate (node c a x b)))))))

(define (balance tree)
  (tree-match tree
    ((black (red (red a x b) y c) z d)
     (red (black a x b) y (black c z d)))
    ((black (red a x (red b y c)) z d)
     (red (black a x b) y (black c z d)))
    ((black a x (red (red b y c) z d))
     (red (black a x b) y (black c z d)))
    ((black a x (red b y (red c z d)))
     (red (black a x b) y (black c z d)))
    ((white (red a x (red b y c)) z d)
     (black (black a x b) y (black c z d)))
    ((white a x (red (red b y c) z d))
     (black (black a x b) y (black c z d)))   
    (t t)))

(define (rotate tree)
  (tree-match tree
    ((red (white? a+x+b) y (black c z d))
     (balance (black (red (white->black a+x+b) y c) z d)))
    ((red (black a x b) y (white? c+z+d))
     (balance (black a x (red b y (white->black c+z+d)))))
    ((black (white? a+x+b) y (black c z d))
     (balance (white (red (white->black a+x+b) y c) z d)))
    ((black (black a x b) y (white? c+z+d))
     (balance (white a x (red b y (white->black c+z+d)))))
    ((black (white? a+w+b) x (red (black c y d) z e))
     (black (balance (black (red (white->black a+w+b) x c) y d)) z e))
    ((black (red a w (black b x c)) y (white? d+z+e))
     (black a w (balance (black b x (red c y (white->black d+z+e))))))
    (t t)))

;; mapping

;; Copyright (C) Marc Nieper-Wiﬂkirchen (2016, 2017).  All Rights
;; Reserved.

;; Permission is hereby granted, free of charge, to any person
;; obtaining a copy of this software and associated documentation
;; files (the "Software"), to deal in the Software without
;; restriction, including without limitation the rights to use, copy,
;; modify, merge, publish, distribute, sublicense, and/or sell copies
;; of the Software, and to permit persons to whom the Software is
;; furnished to do so, subject to the following conditions:

;; The above copyright notice and this permission notice shall be
;; included in all copies or substantial portions of the Software.

;; THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
;; EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
;; MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
;; NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
;; BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
;; ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
;; CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
;; SOFTWARE.

;;; New types

(define-record-type <mapping>
  (%make-mapping comparator tree)
  mapping?
  (comparator mapping-key-comparator)
  (tree mapping-tree))

(define (make-empty-mapping comparator)
  (assume (comparator? comparator))
  (%make-mapping comparator (make-tree)))

;;; Exported procedures

;; Constructors

(define (mapping comparator . args)
  (assume (comparator? comparator))
  (mapping-unfold null?
          (lambda (args)
        (values (car args)
            (cadr args)))
          cddr
          args
          comparator))

(define (mapping-unfold stop? mapper successor seed comparator)
  (assume (procedure? stop?))
  (assume (procedure? mapper))
  (assume (procedure? successor))
  (assume (comparator? comparator))
  (let loop ((mapping (make-empty-mapping comparator))
         (seed seed))
    (if (stop? seed)
    mapping
    (receive (key value)
        (mapper seed)
      (loop (mapping-adjoin mapping key value)
        (successor seed))))))

(define mapping/ordered mapping)
(define mapping-unfold/ordered mapping-unfold)

;; Predicates

(define (mapping-empty? mapping)
  (assume (mapping? mapping))
  (not (mapping-any? (lambda (key value) #t) mapping)))

(define (mapping-contains? mapping key)
  (assume (mapping? mapping))
  (call/cc
   (lambda (return)
     (mapping-search mapping
         key
         (lambda (insert ignore)
           (return #f))
         (lambda (key value update remove)
           (return #t))))))

(define (mapping-disjoint? mapping1 mapping2)
  (assume (mapping? mapping1))
  (assume (mapping? mapping2))
  (call/cc
   (lambda (return)
     (mapping-for-each (lambda (key value)
             (when (mapping-contains? mapping2 key)
               (return #f)))
           mapping1)
     #t)))

;; Accessors

(define mapping-ref
  (case-lambda
    ((mapping key)
     (assume (mapping? mapping))
     (mapping-ref mapping key (lambda ()
            (error "mapping-ref: key not in mapping" key))))
    ((mapping key failure)
     (assume (mapping? mapping))
     (assume (procedure? failure))
     (mapping-ref mapping key failure (lambda (value)
                value)))
    ((mapping key failure success)
     (assume (mapping? mapping))
     (assume (procedure? failure))
     (assume (procedure? success))
     ((call/cc
       (lambda (return-thunk)
     (mapping-search mapping
             key
             (lambda (insert ignore)
               (return-thunk failure))
             (lambda (key value update remove)
               (return-thunk (lambda () (success value)))))))))))

(define (mapping-ref/default mapping key default)
  (assume (mapping? mapping))
  (mapping-ref mapping key (lambda () default)))

;; Updaters

(define (mapping-adjoin mapping . args)
  (assume (mapping? mapping))
  (let loop ((args args)
         (mapping mapping))
    (if (null? args)
    mapping
    (receive (mapping value)
        (mapping-intern mapping (car args) (lambda () (cadr args)))
      (loop (cddr args) mapping)))))

(define mapping-adjoin! mapping-adjoin)

(define (mapping-set mapping . args)
  (assume (mapping? mapping))
  (let loop ((args args)
         (mapping mapping))
    (if (null? args)
    mapping
    (receive (mapping)
        (mapping-update mapping (car args) (lambda (value) (cadr args)) (lambda () #f))    
      (loop (cddr args)
        mapping)))))

(define mapping-set! mapping-set)

(define (mapping-replace mapping key value)
  (assume (mapping? mapping))
  (receive (mapping obj)
      (mapping-search mapping
          key
          (lambda (insert ignore)
            (ignore #f))
          (lambda (old-key old-value update remove)
            (update key value #f)))
    mapping))

(define mapping-replace! mapping-replace)

(define (mapping-delete mapping . keys)
  (assume (mapping? mapping))
  (mapping-delete-all mapping keys))

(define mapping-delete! mapping-delete)

(define (mapping-delete-all mapping keys)
  (assume (mapping? mapping))
  (assume (list? keys))
  (fold (lambda (key mapping)
      (receive (mapping obj)
          (mapping-search mapping
              key
              (lambda (insert ignore)
                (ignore #f))
              (lambda (old-key old-value update remove)
                (remove #f)))
        mapping))
    mapping keys))

(define mapping-delete-all! mapping-delete-all)

(define (mapping-intern mapping key failure)
  (assume (mapping? mapping))
  (assume (procedure? failure))
  (call/cc
   (lambda (return)
     (mapping-search mapping
         key
         (lambda (insert ignore)
           (receive (value)
               (failure)
             (insert value value)))
         (lambda (old-key old-value update remove)
           (return mapping old-value))))))

(define mapping-intern! mapping-intern)

(define mapping-update 
  (case-lambda
   ((mapping key updater)
    (mapping-update mapping key updater (lambda ()
                  (error "mapping-update: key not found in mapping" key))))
   ((mapping key updater failure)
    (mapping-update mapping key updater failure (lambda (value)
                      value)))
   ((mapping key updater failure success)
    (assume (mapping? mapping))
    (assume (procedure? updater))
    (assume (procedure? failure))
    (assume (procedure? success))
    (receive (mapping obj)
    (mapping-search mapping
            key
            (lambda (insert ignore)
              (insert (updater (failure)) #f))
            (lambda (old-key old-value update remove)
              (update key (updater (success old-value)) #f)))
      mapping))))

(define mapping-update! mapping-update)

(define (mapping-update/default mapping key updater default)
  (mapping-update mapping key updater (lambda () default)))

(define mapping-update!/default mapping-update/default)

(define mapping-pop
  (case-lambda
    ((mapping)
     (mapping-pop mapping (lambda ()
                (error "mapping-pop: mapping has no association"))))
    ((mapping failure)
     (assume (mapping? mapping))
     (assume (procedure? failure))
     ((call/cc
       (lambda (return-thunk)
     (receive (key value)
         (mapping-find (lambda (key value) #t) mapping (lambda () (return-thunk failure)))
       (lambda ()
         (values (mapping-delete mapping key) key value)))))))))

(define mapping-pop! mapping-pop)

(define (mapping-search mapping key failure success)
  (assume (mapping? mapping))
  (assume (procedure? failure))
  (assume (procedure? success))
  (call/cc
   (lambda (return)
     (let*-values
     (((comparator)
       (mapping-key-comparator mapping))
      ((tree obj)
       (tree-search comparator
            (mapping-tree mapping)
            key
            (lambda (insert ignore)
              (failure (lambda (value obj)
                     (insert key value obj))
                   (lambda (obj)
                     (return mapping obj))))
            success)))
       (values (%make-mapping comparator tree)
           obj)))))

(define mapping-search! mapping-search)

;; The whole mapping

(define (mapping-size mapping)
  (assume (mapping? mapping))
  (mapping-count (lambda (key value)
           #t)
         mapping))

(define (mapping-find predicate mapping failure)
  (assume (procedure? predicate))
  (assume (mapping? mapping))
  (assume (procedure? failure))
  (call/cc
   (lambda (return)
     (mapping-for-each (lambda (key value)
             (when (predicate key value)
               (return key value)))
           mapping)
     (failure))))

(define (mapping-count predicate mapping)
  (assume (procedure? predicate))
  (assume (mapping? mapping))
  (mapping-fold (lambda (key value count)
          (if (predicate key value)
          (+ 1 count)
          count))
        0 mapping))

(define (mapping-any? predicate mapping)
  (assume (procedure? predicate))
  (assume (mapping? mapping))
  (call/cc
   (lambda (return)
     (mapping-for-each (lambda (key value)
             (when (predicate key value)
               (return #t)))
           mapping)
     #f)))

(define (mapping-every? predicate mapping)
  (assume (procedure? predicate))
  (assume (mapping? mapping))
  (not (mapping-any? (lambda (key value)
           (not (predicate key value)))
         mapping)))

(define (mapping-keys mapping)
  (assume (mapping? mapping))
  (mapping-fold/reverse (lambda (key value keys)
              (cons key keys))
            '() mapping))

(define (mapping-values mapping)
  (assume (mapping? mapping))
  (mapping-fold/reverse (lambda (key value values)
              (cons value values))
            '() mapping))

(define (mapping-entries mapping)
  (assume (mapping? mapping))
  (values (mapping-keys mapping)
      (mapping-values mapping)))

;; Mapping and folding

(define (mapping-map proc comparator mapping)
  (assume (procedure? proc))
  (assume (comparator? comparator))
  (assume (mapping? mapping))
  (mapping-fold (lambda (key value mapping)
          (receive (key value)
          (proc key value)
        (mapping-set mapping key value)))
        (make-empty-mapping comparator)
        mapping))

(define (mapping-for-each proc mapping)
  (assume (procedure? proc))
  (assume (mapping? mapping))
  (tree-for-each proc (mapping-tree mapping)))

(define (mapping-fold proc acc mapping)
  (assume (procedure? proc))
  (assume (mapping? mapping))
  (tree-fold proc acc (mapping-tree mapping)))

(define (mapping-map->list proc mapping)
  (assume (procedure? proc))
  (assume (mapping? mapping))
  (mapping-fold/reverse (lambda (key value lst)
              (cons (proc key value) lst))
            '()
            mapping))

(define (mapping-filter predicate mapping)
  (assume (procedure? predicate))
  (assume (mapping? mapping))
  (mapping-fold (lambda (key value mapping)
          (if (predicate key value)
          (mapping-set mapping key value)
          mapping))
        (make-empty-mapping (mapping-key-comparator mapping))
        mapping))

(define mapping-filter! mapping-filter)

(define (mapping-remove predicate mapping)
  (assume (procedure? predicate))
  (assume (mapping? mapping))
  (mapping-filter (lambda (key value)
        (not (predicate key value)))
          mapping))

(define mapping-remove! mapping-remove)

(define (mapping-partition predicate mapping)
  (assume (procedure? predicate))
  (assume (mapping? mapping))
  (values (mapping-filter predicate mapping)
      (mapping-remove predicate mapping)))

(define mapping-partition! mapping-partition)

;; Copying and conversion

(define (mapping-copy mapping)
  (assume (mapping? mapping))
  mapping)

(define (mapping->alist mapping)
  (assume (mapping? mapping))
  (reverse
   (mapping-fold (lambda (key value alist)
           (cons (cons key value) alist))
         '() mapping)))

(define (alist->mapping comparator alist)
  (assume (comparator? comparator))
  (assume (list? alist))
  (mapping-unfold null?
          (lambda (alist)
        (let ((key (caar alist))
              (value (cdar alist)))
          (values key value)))
          cdr
          alist
          comparator))

(define (alist->mapping! mapping alist)
  (assume (mapping? mapping))
  (assume (list? alist))
  (fold (lambda (association mapping)
      (let ((key (car association))
        (value (cdr association)))
        (mapping-set mapping key value)))
    mapping
    alist))

(define alist->mapping/ordered alist->mapping)
(define alist->mapping/ordered! alist->mapping!)

;; Submappings

(define mapping=?
  (case-lambda
    ((comparator mapping)
     (assume (mapping? mapping))
     #t)
    ((comparator mapping1 mapping2) (%mapping=? comparator mapping1 mapping2))
    ((comparator mapping1 mapping2 . mappings)
     (and (%mapping=? comparator mapping1 mapping2)
          (apply mapping=? comparator mapping2 mappings)))))
(define (%mapping=? comparator mapping1 mapping2)
  (and (eq? (mapping-key-comparator mapping1) (mapping-key-comparator mapping2))
       (%mapping<=? comparator mapping1 mapping2)
       (%mapping<=? comparator mapping2 mapping1)))

(define mapping<=?
  (case-lambda
    ((comparator mapping)
     (assume (mapping? mapping))
     #t)
    ((comparator mapping1 mapping2)
     (assume (comparator? comparator))
     (assume (mapping? mapping1))
     (assume (mapping? mapping2))
     (%mapping<=? comparator mapping1 mapping2))
    ((comparator mapping1 mapping2 . mappings)
     (assume (comparator? comparator))
     (assume (mapping? mapping1))
     (assume (mapping? mapping2))
     (and (%mapping<=? comparator mapping1 mapping2)
          (apply mapping<=? comparator mapping2 mappings)))))

(define (%mapping<=? comparator mapping1 mapping2)
  (assume (comparator? comparator))
  (assume (mapping? mapping1))
  (assume (mapping? mapping2))
  (let ((less? (comparator-ordering-predicate (mapping-key-comparator mapping1)))
    (equality-predicate (comparator-equality-predicate comparator))
    (gen1 (tree-generator (mapping-tree mapping1)))
    (gen2 (tree-generator (mapping-tree mapping2))))    
    (let loop ((item1 (gen1))
           (item2 (gen2)))
      (cond
       ((eof-object? item1)
    #t)
       ((eof-object? item2)
    #f)
       (else
    (let ((key1 (car item1)) (value1 (cadr item1))
          (key2 (car item2)) (value2 (cadr item2)))
      (cond
       ((less? key1 key2)
        #f)
       ((less? key2 key1)
        (loop item1 (gen2)))
       ((equality-predicate value1 value2)
        (loop (gen1) (gen2)))
       (else
        #f))))))))

(define mapping>?
  (case-lambda
    ((comparator mapping)
     (assume (mapping? mapping))
     #t)
    ((comparator mapping1 mapping2)
     (assume (comparator? comparator))
     (assume (mapping? mapping1))
     (assume (mapping? mapping2))
     (%mapping>? comparator mapping1 mapping2))
    ((comparator mapping1 mapping2 . mappings)
     (assume (comparator? comparator))
     (assume (mapping? mapping1))
     (assume (mapping? mapping2))
     (and (%mapping>? comparator  mapping1 mapping2)
          (apply mapping>? comparator mapping2 mappings)))))

(define (%mapping>? comparator mapping1 mapping2)
  (assume (comparator? comparator))
  (assume (mapping? mapping1))
  (assume (mapping? mapping2))
  (not (%mapping<=? comparator mapping1 mapping2)))

(define mapping<?
  (case-lambda
    ((comparator mapping)
     (assume (mapping? mapping))
     #t)
    ((comparator mapping1 mapping2)
     (assume (comparator? comparator))
     (assume (mapping? mapping1))
     (assume (mapping? mapping2))
     (%mapping<? comparator mapping1 mapping2))
    ((comparator mapping1 mapping2 . mappings)
     (assume (comparator? comparator))
     (assume (mapping? mapping1))
     (assume (mapping? mapping2))
     (and (%mapping<? comparator  mapping1 mapping2)
          (apply mapping<? comparator mapping2 mappings)))))

(define (%mapping<? comparator mapping1 mapping2)
     (assume (comparator? comparator))
     (assume (mapping? mapping1))
     (assume (mapping? mapping2))
     (%mapping>? comparator mapping2 mapping1))

(define mapping>=?
  (case-lambda
    ((comparator mapping)
     (assume (mapping? mapping))
     #t)
    ((comparator mapping1 mapping2)
     (assume (comparator? comparator))
     (assume (mapping? mapping1))
     (assume (mapping? mapping2))
     (%mapping>=? comparator mapping1 mapping2))
    ((comparator mapping1 mapping2 . mappings)
     (assume (comparator? comparator))
     (assume (mapping? mapping1))
     (assume (mapping? mapping2))
     (and (%mapping>=? comparator mapping1 mapping2)
          (apply mapping>=? comparator mapping2 mappings)))))

(define (%mapping>=? comparator mapping1 mapping2)
  (assume (comparator? comparator))
  (assume (mapping? mapping1))
  (assume (mapping? mapping2))
  (not (%mapping<? comparator mapping1 mapping2)))

;; Set theory operations

(define (%mapping-union mapping1 mapping2)
  (mapping-fold (lambda (key2 value2 mapping)
          (receive (mapping obj)
              (mapping-search mapping
                      key2
                      (lambda (insert ignore)
                    (insert value2 #f))
                      (lambda (key1 value1 update remove)
                    (update key1 value1 #f)))
            mapping))
        mapping1 mapping2))

(define (%mapping-intersection mapping1 mapping2)
  (mapping-filter (lambda (key1 value1)
        (mapping-contains? mapping2 key1))
          mapping1))

(define (%mapping-difference mapping1 mapping2)
  (mapping-fold (lambda (key2 value2 mapping)
          (receive (mapping obj)
          (mapping-search mapping
                  key2
                  (lambda (insert ignore)
                (ignore #f))
                  (lambda (key1 value1 update remove)
                (remove #f)))
        mapping))
        mapping1 mapping2))

(define (%mapping-xor mapping1 mapping2)
  (mapping-fold (lambda (key2 value2 mapping)
          (receive (mapping obj)
          (mapping-search mapping
                  key2
                  (lambda (insert ignore)
                (insert value2 #f))
                  (lambda (key1 value1 update remove)
                (remove #f)))
        mapping))
        mapping1 mapping2))

(define mapping-union
  (case-lambda
    ((mapping)
     (assume (mapping? mapping))
     mapping)
    ((mapping1 mapping2)
     (assume (mapping? mapping1))
     (assume (mapping? mapping2))
     (%mapping-union mapping1 mapping2))
    ((mapping1 mapping2 . mappings)
     (assume (mapping? mapping1))
     (assume (mapping? mapping2))
     (apply mapping-union (%mapping-union mapping1 mapping2) mappings))))
(define mapping-union! mapping-union)

(define mapping-intersection
  (case-lambda
    ((mapping)
     (assume (mapping? mapping))
     mapping)
    ((mapping1 mapping2)
     (assume (mapping? mapping1))
     (assume (mapping? mapping2))
     (%mapping-intersection mapping1 mapping2))
    ((mapping1 mapping2 . mappings)
     (assume (mapping? mapping1))
     (assume (mapping? mapping2))
     (apply mapping-intersection (%mapping-intersection mapping1 mapping2) mappings))))
(define mapping-intersection! mapping-intersection)

(define mapping-difference
  (case-lambda
    ((mapping)
     (assume (mapping? mapping))
     mapping)
    ((mapping1 mapping2)
     (assume (mapping? mapping1))
     (assume (mapping? mapping2))
     (%mapping-difference mapping1 mapping2))
    ((mapping1 mapping2 . mappings)
     (assume (mapping? mapping1))
     (assume (mapping? mapping2))
     (apply mapping-difference (%mapping-difference mapping1 mapping2) mappings))))
(define mapping-difference! mapping-difference)

(define mapping-xor
  (case-lambda
    ((mapping)
     (assume (mapping? mapping))
     mapping)
    ((mapping1 mapping2)
     (assume (mapping? mapping1))
     (assume (mapping? mapping2))
     (%mapping-xor mapping1 mapping2))
    ((mapping1 mapping2 . mappings)
     (assume (mapping? mapping1))
     (assume (mapping? mapping2))
     (apply mapping-xor (%mapping-xor mapping1 mapping2) mappings))))
(define mapping-xor! mapping-xor)

;; Additional procedures for mappings with ordererd keys

(define (mapping-min-key mapping)
  (assume (mapping? mapping))
  (call/cc
   (lambda (return)
     (mapping-fold (lambda (key value acc)
             (return key))
           #f mapping)
     (error "mapping-min-key: empty map"))))

(define (mapping-max-key mapping)
  (assume (mapping? mapping))
  (call/cc
   (lambda (return)
     (mapping-fold/reverse (lambda (key value acc)
                 (return key))
               #f mapping)
     (error "mapping-max-key: empty map"))))

(define (mapping-min-value mapping)
  (assume (mapping? mapping))
  (call/cc
   (lambda (return)
     (mapping-fold (lambda (key value acc)
             (return value))
           #f mapping)
     (error "mapping-min-value: empty map"))))

(define (mapping-max-value mapping)
  (assume (mapping? mapping))
  (call/cc
   (lambda (return)
     (mapping-fold/reverse (lambda (key value acc)
                 (return value))
               #f mapping)
     (error "mapping-max-value: empty map"))))

(define (mapping-key-predecessor mapping obj failure)
  (assume (mapping? mapping))
  (assume (procedure? failure))
  (tree-key-predecessor (mapping-key-comparator mapping) (mapping-tree mapping) obj failure))

(define (mapping-key-successor mapping obj failure)
  (assume (mapping? mapping))
  (assume (procedure? failure))
  (tree-key-successor (mapping-key-comparator mapping) (mapping-tree mapping) obj failure))

(define (mapping-range= mapping obj)
  (assume (mapping? mapping))
  (let ((comparator (mapping-key-comparator mapping)))
    (receive (tree< tree<= tree= tree>= tree>)
    (tree-split comparator (mapping-tree mapping) obj)
      (%make-mapping comparator tree=))))

(define (mapping-range< mapping obj)
  (assume (mapping? mapping))
  (let ((comparator (mapping-key-comparator mapping)))
    (receive (tree< tree<= tree= tree>= tree>)
    (tree-split comparator (mapping-tree mapping) obj)
      (%make-mapping comparator tree<))))

(define (mapping-range<= mapping obj)
  (assume (mapping? mapping))
  (let ((comparator (mapping-key-comparator mapping)))
    (receive (tree< tree<= tree= tree>= tree>)
    (tree-split comparator (mapping-tree mapping) obj)
      (%make-mapping comparator tree<=))))

(define (mapping-range> mapping obj)
  (assume (mapping? mapping))
  (let ((comparator (mapping-key-comparator mapping)))
    (receive (tree< tree<= tree= tree>= tree>)
    (tree-split comparator (mapping-tree mapping) obj)
      (%make-mapping comparator tree>))))

(define (mapping-range>= mapping obj)
  (assume (mapping? mapping))
  (assume (mapping? mapping))
  (let ((comparator (mapping-key-comparator mapping)))
    (receive (tree< tree<= tree= tree>= tree>)
    (tree-split comparator (mapping-tree mapping) obj)
      (%make-mapping comparator tree>=))))

(define mapping-range=! mapping-range=)
(define mapping-range<! mapping-range<)
(define mapping-range>! mapping-range>)
(define mapping-range<=! mapping-range<=)
(define mapping-range>=! mapping-range>=)

(define (mapping-split mapping obj)
  (assume (mapping? mapping))
  (let ((comparator (mapping-key-comparator mapping)))
    (receive (tree< tree<= tree= tree>= tree>)
    (tree-split comparator (mapping-tree mapping) obj)
      (values (%make-mapping comparator tree<)
          (%make-mapping comparator tree<=)
          (%make-mapping comparator tree=)
          (%make-mapping comparator tree>=)
          (%make-mapping comparator tree>)))))

(define (mapping-catenate comparator mapping1 pivot-key pivot-value mapping2)
  (assume (comparator? comparator))
  (assume (mapping? mapping1))
  (assume (mapping? mapping2))
  (%make-mapping comparator (tree-catenate (mapping-tree mapping1)
                       pivot-key
                       pivot-value
                       (mapping-tree mapping2))))

(define mapping-catenate! mapping-catenate)

(define (mapping-map/monotone proc comparator mapping)
  (assume (procedure? proc))
  (assume (comparator? comparator))
  (assume (mapping? mapping))
  (%make-mapping comparator (tree-map proc (mapping-tree mapping))))

(define mapping-map/monotone! mapping-map/monotone)

(define (mapping-fold/reverse proc acc mapping)
  (assume (procedure? proc))
  (assume (mapping? mapping))
  (tree-fold/reverse proc acc (mapping-tree mapping)))

;; Comparators

(define (mapping-equality comparator)
  (assume (comparator? comparator))
  (lambda (mapping1 mapping2)
    (mapping=? comparator mapping1 mapping2)))

(define (mapping-ordering comparator)
  (assume (comparator? comparator))
  (let ((value-equality (comparator-equality-predicate comparator))
    (value-ordering (comparator-ordering-predicate comparator)))
    (lambda (mapping1 mapping2)
      (let* ((key-comparator (mapping-key-comparator mapping1))
         (equality (comparator-equality-predicate key-comparator))
         (ordering (comparator-ordering-predicate key-comparator))
         (gen1 (tree-generator (mapping-tree mapping1)))
         (gen2 (tree-generator (mapping-tree mapping2))))
    (let loop ()
      (let ((item1 (gen1)) (item2 (gen2)))
        (cond
         ((eof-object? item1)
          (not (eof-object? item2)))
         ((eof-object? item2)
          #f)
         (else
          (let ((key1 (car item1)) (value1 (cadr item1))
            (key2 (car item2)) (value2 (cadr item2)))
        (cond
         ((equality key1 key2)
          (if (value-equality value1 value2)
              (loop)
              (value-ordering value1 value2)))
         (else
          (ordering key1 key2))))))))))))

(define (make-mapping-comparator comparator)
  (make-comparator mapping? (mapping-equality comparator) (mapping-ordering comparator) #f))

(define mapping-comparator (make-mapping-comparator (make-default-comparator)))

(comparator-register-default! mapping-comparator)

))