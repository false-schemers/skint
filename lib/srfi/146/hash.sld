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

(define-library (srfi 146 hash)
  (export
   hashmap hashmap-unfold
   hashmap? hashmap-contains? hashmap-empty? hashmap-disjoint?
   hashmap-ref hashmap-ref/default hashmap-key-comparator
   hashmap-adjoin hashmap-adjoin!
   hashmap-set hashmap-set!
   hashmap-replace hashmap-replace!
   hashmap-delete hashmap-delete! hashmap-delete-all hashmap-delete-all!
   hashmap-intern hashmap-intern!
   hashmap-update hashmap-update! hashmap-update/default hashmap-update!/default
   hashmap-pop hashmap-pop!
   hashmap-search hashmap-search!
   hashmap-size hashmap-find hashmap-count hashmap-any? hashmap-every?
   hashmap-keys hashmap-values hashmap-entries
   hashmap-map hashmap-map->list hashmap-for-each hashmap-fold
   hashmap-filter hashmap-filter!
   hashmap-remove hashmap-remove!
   hashmap-partition hashmap-partition!
   hashmap-copy hashmap->alist alist->hashmap alist->hashmap!
   hashmap=? hashmap<? hashmap>? hashmap<=? hashmap>=?
   hashmap-union hashmap-intersection hashmap-difference hashmap-xor
   hashmap-union! hashmap-intersection! hashmap-difference! hashmap-xor!
   make-hashmap-comparator
   hashmap-comparator
   comparator?)
  (import (scheme base)
          (scheme case-lambda)
          (srfi 1)
          (srfi 8)
          (srfi 121)
;          (only (skint) fxand fxnot fxsll fxsrl fx-)
          (only (srfi 125) make-hash-table string-hash)
          (except (srfi 128) string-hash string-comparator)
          (srfi 143)
          (srfi 145)
          (srfi 151))

(begin


;;;; Utilities used by HAMT

;;; Copyright MMIV-MMXVII Arthur A. Gleckler.  All rights reserved.

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
;; NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
;; HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
;; WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
;; OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
;; DEALINGS IN THE SOFTWARE.

(define-syntax assert
  (syntax-rules ()
    ((_ (operator argument ...))
     (unless (operator argument ...)
       (error "Assertion failed:"
	      '(operator argument ...)
	      (list 'operator argument ...))))
    ((_ expression)
     (unless expression
       (error "Assertion failed:" 'expression)))))

(define-syntax do-list
  (syntax-rules ()
    ((_ (variable list) body ...)
     (do ((remaining list (cdr remaining)))
	 ((null? remaining))
       (let ((variable (car remaining)))
	 body ...)))
    ((_ (element-variable index-variable list) body ...)
     (do ((remaining list (cdr remaining))
	  (index-variable 0 (+ index-variable 1)))
	 ((null? remaining))
       (let ((element-variable (car remaining)))
	 body ...)))))

(define string-comparator
  (make-comparator string? string=? #f string-hash))

(define (make-string-hash-table)
  (make-hash-table string-comparator))

(define (with-output-to-string thunk)
  (parameterize ((current-output-port (open-output-string)))
    (thunk)
    (get-output-string (current-output-port))))


;;; Copyright MMIV-MMXV Arthur A. Gleckler.  All rights reserved.

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
;; NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
;; HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
;; WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
;; OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
;; DEALINGS IN THE SOFTWARE.

;;; `vector-edit' adds and/or removes elements from a vector
;;; non-destructively, i.e. by returning a new vector.  It maps
;;; offsets in the original vector to offsets in the new vector so
;;; that the caller doesn't have to perform these error-prone
;;; calculations itself.

(define (vector-without v start end)
  "Return a copy of vector `v' without the elements with indices [start, end)."
  (let* ((size (vector-length v))
	 (gap-size (- end start))
	 (new-size (- size gap-size))
	 (result (make-vector new-size)))
    (vector-copy! result 0 v 0 start)
    (vector-copy! result start v end size)
    result))

(define (vector-replace-one v i e)
  "Return a copy of vector `v' with the `i'th element replaced by `e'."
  (let ((result (vector-copy v)))
    (vector-set! result i e)
    result))

(define-syntax vector-edit-total-skew
  (syntax-rules (add drop)
    ((_ s) s)
    ((_ s (add i e) . rest)
     (vector-edit-total-skew (+ s 1) . rest))
    ((_ s (drop i c) . rest)
     (vector-edit-total-skew (- s c) . rest))))

(define-syntax vector-edit-code
  (syntax-rules (add drop)
    ((_ v r o s)
     (let ((index (vector-length v)))
       (vector-copy! r (+ o s) v o index)
       r))
    ((_ v r o s (add i e) . rest)
     (let ((index i))
       (vector-copy! r (+ o s) v o index)
       (vector-set! r (+ s index) e)
       (let ((skew (+ s 1)))
	 (vector-edit-code v r index skew . rest))))
    ((_ v r o s (drop i c) . rest)
     (let ((index i))
       (vector-copy! r (+ o s) v o index)
       (let* ((dropped c)
	      (offset (+ index dropped))
	      (skew (- s dropped)))
	 (vector-edit-code v r offset skew . rest))))))

;; <> Optimize this by allowing one to supply more than one value in
;; `add' sub-expressions so that adjacent values can be inserted
;; without extra computation.

;; Given a vector `v' and a set of `(add i e)' and `(drop i c)' forms,
;; return a new vector that is the result of applying insertions to
;; and deletions from `v'.  Interpret each `i' as an index into `v',
;; each `e' as an element to be inserted into the resulting vector at
;; the index corresponding to `i', and each `c' as a count of elements
;; of `v' to be dropped starting at index `i'.  The `i' values in the
;; `add' and `drop' forms must never decrease from left to right.
;; This is useful for doing insertions and deletions without
;; constructing an intermediate vector.
(define-syntax vector-edit
  (syntax-rules ()
    ((_ v . rest)
     (let ((result (make-vector (+ (vector-length v)
				   (vector-edit-total-skew 0 . rest)))))
       (vector-edit-code v result 0 0 . rest)))))


;;;; Persistent Hash Map

;;; Copyright MMXV-MMXVII Arthur A. Gleckler.  All rights reserved.

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
;; NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
;; HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
;; WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
;; OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
;; DEALINGS IN THE SOFTWARE.

;;; Naming conventions:

;;    =: procedure that compares keys
;;    c: bit string representing the non-leaf children present
;;       immediately below a sparse node
;;    d: datum, or `hamt-null' to represent absence or deletion
;;   dp: procedure that takes an existing datum and returns the datum
;;       that should replace it.  Either may be `hamt-null'.  When
;;       there is no payload, `hamt-null' is passed.
;;    h: hash
;;   hp: procedure that computes hash
;;    k: key that maps to a particular datum
;;    l: bit string representing the leaves present below a sparse node
;;    n: node (of type `collision', `narrow', or `wide')

;;; Background

;; See these papers:

;; - Ideal Hash Trees, Phil Bagwell, 2000,
;;   <https://infoscience.epfl.ch/record/64398/files/idealhashtrees.pdf>

;; - Optimizing Hash-Array Mapped Tries for Fast and Lean Immutable
;;   JVM Collections, Steinforder & Vinju, 2015,
;;   <http://michael.steindorfer.name/publications/oopsla15.pdf>

;; Also, see Clojure's persistent hash maps, which support both
;; mutable ("transient") and persistent modes.

;;; Design

;; According to Phil Bagwell's paper, "Occasionally an entire 32 bit
;; hash may be consumed and a new one must be computed to
;; differentiate the two keys."  Later, he says "The hash function was
;; tailored to give a 32 bit hash.  The algorithm requires that the
;; hash can be extended to an arbitrary number of bits.  This was
;; accomplished by rehashing the key combined with an integer
;; representing the trie level, zero being the root.  Hence if two
;; keys do give the same initial hash then the rehash has a
;; probability of 1 in 2^32 of a further collision."  However, I
;; implement collision lists instead because they will be rarely used
;; when hash functions are good, but work well when they're not, as in
;; the case of MIT Scheme's `string-hash'.

;; [esl]* If fx-width is a multiple of hamt-hash-slice-size (originally 5),
;; then hamt-hash-size == fx-width and hamt-hash-modulus is not representable
;; as a fixnum (promoted to flonum in Skint). Lowering it to 4 allows us to
;; use less fixnum bits (28 out of 30), but guarantees we stay in fixnum range

(define hamt-hash-slice-size 4) ;[esl]* : was 5
(define hamt-hash-size
  (let ((word-size fx-width))
    (- word-size
       (remainder word-size hamt-hash-slice-size))))
(define hamt-hash-modulus (expt 2 hamt-hash-size))
(define hamt-bucket-size (expt 2 hamt-hash-slice-size))
(define hamt-null (cons 'hamt 'null))

(define-record-type hash-array-mapped-trie
    (%make-hamt = count hash mutable? payload? root)
    hash-array-mapped-trie?
  (=        hamt/=)
  (count    hamt/count set-hamt/count!)
  (hash     hamt/hash)
  (mutable? hamt/mutable?)
  (payload? hamt/payload?)
  (root     hamt/root  set-hamt/root!))

(define (make-hamt = hash payload?)
  (%make-hamt = 0 hash #f payload? (make-empty-narrow)))

(define-record-type collision
    (make-collision entries hash)
    collision?
  (entries collision/entries)
  (hash  collision/hash))

(define-record-type narrow
    (make-narrow array children leaves)
    narrow?
  (array    narrow/array)
  (children narrow/children)
  (leaves   narrow/leaves))

(define-record-type wide
    (make-wide array children leaves)
    wide?
  (array    wide/array)
  (children wide/children set-wide/children!)
  (leaves   wide/leaves   set-wide/leaves!))

(define (hamt/empty? hamt)
  (zero? (hamt/count hamt)))

(define (hamt/immutable-inner hamt replace)
  "Return a HAMT equivalent to `hamt', but that is immutable.  Even if
`hamt' is mutable, no change to it will affect the returned HAMT.  If
`hamt' has payloads, replace each datum in a wide node with what
`replace' returns when passed the key and corresponding datum.  This
is useful for converting HAMT sets stored as values in a HAMT map back
to immutable ones when the containing map is made immutable.  (Only
data in wide nodes will have been modified since the change to mutable
happened.)"
  (if (hamt/mutable? hamt)
      (let ((payload? (hamt/payload? hamt)))
	(%make-hamt (hamt/= hamt)
		    (hamt/count hamt)
		    (hamt/hash hamt)
		    #f
		    payload?
		    (->immutable (hamt/root hamt) payload? replace)))
      hamt))

(define hamt/immutable
  (case-lambda
   ((hamt) (hamt/immutable-inner hamt (lambda (k d) d)))
   ((hamt replace) (hamt/immutable-inner hamt replace))))

(define (hamt/mutable hamt)
  (if (hamt/mutable? hamt)
      hamt
      (%make-hamt (hamt/= hamt)
		  (hamt/count hamt)
		  (hamt/hash hamt)
		  #t
		  (hamt/payload? hamt)
		  (hamt/root hamt))))

(define (hamt/replace hamt key dp)
  (assert (not (hamt/mutable? hamt)))
  (let*-values (((payload?) (hamt/payload? hamt))
		((root) (hamt/root hamt))
		((==) (hamt/= hamt))
		((hp) (hamt/hash hamt))
		((hash) (hash-bits hp key))
		((change node) (modify-pure hamt root 0 dp hash key)))
    (if (eq? node root)
	hamt
	(let ((count (+ (hamt/count hamt) change)))
	  (%make-hamt == count hp #f payload? node)))))

(define (hamt/put hamt key datum)
  (hamt/replace hamt key (lambda (x) datum)))

(define (hamt/replace! hamt key dp)
  (assert (hamt/mutable? hamt))
  (let*-values (((root) (hamt/root hamt))
		((hp) (hamt/hash hamt))
		((hash) (hash-bits hp key))
		((change node) (mutate hamt root 0 dp hash key)))
    (unless (zero? change)
      (set-hamt/count! hamt (+ (hamt/count hamt) change)))
    (unless (eq? node root)
      (set-hamt/root! hamt node))
    hamt))

(define (hamt/put! hamt key datum)
  (hamt/replace! hamt key (lambda (x) datum)))

(define (make-empty-narrow)
  (make-narrow (vector) 0 0))

(define (hamt-null? n)
  (eq? n hamt-null))

(define (collision-single-leaf? n)
  (let ((elements (collision/entries n)))
    (and (not (null? elements))
	 (null? (cdr elements)))))

(define (narrow-single-leaf? n)
  (and (zero? (narrow/children n))
       (= 1 (bit-count (narrow/leaves n)))))

(define (wide-single-leaf? n)
  (and (zero? (wide/children n))
       (= 1 (bit-count (wide/leaves n)))))

(define (hash-bits hp key)
  (remainder (hp key) hamt-hash-modulus))

(define (next-set-bit i start end)
  (let ((index (first-set-bit (bit-field i start end))))
    (and (not (= index -1))
	 (+ index start))))

(define (narrow->wide n payload?)
  (let* ((c (narrow/children n))
	 (l (narrow/leaves n))
	 (stride (leaf-stride payload?))
	 (a-in (narrow/array n))
	 (a-out (make-vector (* stride hamt-bucket-size))))
    (let next-leaf ((start 0) (count 0))
      (let ((i (next-set-bit l start hamt-bucket-size)))
	(when i
	  (let ((j (* stride i)))
	    (vector-set! a-out j (vector-ref a-in count))
	    (when payload?
	      (vector-set! a-out (+ j 1) (vector-ref a-in (+ count 1)))))
	  (next-leaf (+ i 1) (+ stride count)))))
    (let next-child ((start 0) (offset (* stride (bit-count l))))
      (let ((i (next-set-bit c start hamt-bucket-size)))
	(when i
	  (vector-set! a-out (* stride i) (vector-ref a-in offset))
	  (next-child (+ i 1) (+ offset 1)))))
    (make-wide a-out c l)))

(define (->immutable n payload? replace)
  "Convert `n' and its descendants into `collision' or `narrow' nodes.
Stop at the first `collision' node or `narrow' node on each path.  If
`payload?' is true, then expect data, not just keys, and replace each
datum in a wide node with what `replace' returns when passed the key
and corresponding datum."
  (cond ((collision? n) n)
	((narrow? n) n)
	((wide? n)
	 (let* ((c (wide/children n))
		(l (wide/leaves n))
		(stride (leaf-stride payload?))
		(l-count (bit-count l))
		(a-in (wide/array n))
		(a-out (make-vector
			(+ (* stride l-count) (bit-count c)))))
	   (let next-leaf ((start 0) (count 0))
	     (let ((i (next-set-bit l
				    start
				    hamt-bucket-size)))
	       (when i
		 (let* ((j (* stride i))
			(key (vector-ref a-in j)))
		   (vector-set! a-out count key)
		   (when payload?
		     (vector-set! a-out
				  (+ count 1)
				  (replace
				   key
				   (vector-ref a-in (+ j 1))))))
		 (next-leaf (+ i 1) (+ stride count)))))
	   (let next-child ((start 0) (offset (* stride l-count)))
	     (let ((i (next-set-bit c
				    start
				    hamt-bucket-size)))
	       (when i
		 (vector-set! a-out
			      offset
			      (->immutable (vector-ref a-in (* stride i))
					  payload?
					  replace))
		 (next-child (+ i 1) (+ offset 1)))))
	   (make-narrow a-out c l)))
	(else (error "Unexpected type of node."))))

(define (hash-fragment shift hash)
  (bit-field hash shift (+ shift hamt-hash-slice-size)))

(define (fragment->mask fragment)
  (- (expt 2 fragment) 1))

(define (mutate hamt n shift dp h k)
  (cond ((collision? n) (modify-collision hamt n shift dp h k))
	((narrow? n)
	 (modify-wide hamt
		      (narrow->wide n (hamt/payload? hamt))
		      shift
		      dp
		      h
		      k))
	((wide? n) (modify-wide hamt n shift dp h k))
	(else (error "Unknown HAMT node type." n))))

(define (modify-wide hamt n shift dp h k)
  (let ((fragment (hash-fragment shift h)))
    (cond ((bit-set? fragment (wide/children n))
	   (modify-wide-child hamt n shift dp h k))
	  ((bit-set? fragment (wide/leaves n))
	   (modify-wide-leaf hamt n shift dp h k))
	  (else
	   (let ((d (dp hamt-null)))
	     (if (hamt-null? d)
		 (values 0 n)
		 (modify-wide-new hamt n shift d h k)))))))

(define (modify-wide-child hamt n shift dp h k)
  (let*-values (((fragment) (hash-fragment shift h))
		((array) (wide/array n))
		((payload?) (hamt/payload? hamt))
		((stride) (leaf-stride payload?))
		((i) (* stride fragment))
		((child) (vector-ref array i))
		((change new-child)
		 (mutate hamt
			 child
			 (+ shift hamt-hash-slice-size)
			 dp
			 h
			 k)))
    (define (coalesce key datum)
      (vector-set! array i key)
      (when payload?
	(vector-set! array (+ i 1) datum))
      (set-wide/children! n (copy-bit fragment (wide/children n) #f))
      (set-wide/leaves! n (copy-bit fragment (wide/leaves n) #t))
      (values change n))
    (define (replace)
      (vector-set! array i new-child)
      (values change n))
    (cond ((eq? new-child child) (values change n))
	  ((hamt-null? new-child)
	   (error "Child cannot become null." n))
	  ((collision? new-child)
	   (if (collision-single-leaf? new-child)
	       (let ((a (car (collision/entries new-child))))
		 (if payload?
		     (coalesce (car a) (cdr a))
		     (coalesce a #f)))
	       (replace)))
	  ((wide? new-child)
	   (if (wide-single-leaf? new-child)
	       (let ((a (wide/array new-child))
		     (j (* stride (next-set-bit (wide/leaves new-child)
						0
						hamt-bucket-size))))
		 (coalesce (vector-ref a j)
			   (and payload? (vector-ref a (+ j 1)))))
	       (replace)))
	  ((narrow? new-child)
	   (replace))
	  (else (error "Unexpected type of child node.")))))

(define (modify-wide-leaf hamt n shift dp h k)
  (let* ((fragment (hash-fragment shift h))
	 (array (wide/array n))
	 (payload? (hamt/payload? hamt))
	 (stride (leaf-stride payload?))
	 (i (* stride fragment))
	 (key (vector-ref array i)))
    (if ((hamt/= hamt) k key)
	(let* ((existing (if payload? (vector-ref array (+ i 1)) hamt-null))
	       (d (dp existing)))
	  (cond ((hamt-null? d)
		 (vector-set! array i #f)
		 (when payload? (vector-set! array (+ i 1) #f))
		 (set-wide/leaves! n (copy-bit fragment (wide/leaves n) #f))
		 (values -1 n))
		(else
		 (when payload? (vector-set! array (+ i 1) d))
		 (values 0 n))))
	(let ((d (dp hamt-null)))
	  (if (hamt-null? d)
	      (values 0 n)
	      (add-wide-leaf-key hamt n shift d h k))))))

(define (add-wide-leaf-key hamt n shift d h k)
  (define payload? (hamt/payload? hamt))
  (define make-entry
    (if payload? cons (lambda (k d) k)))
  (let* ((fragment (hash-fragment shift h))
	 (array (wide/array n))
	 (stride (leaf-stride payload?))
	 (i (* stride fragment))
	 (key (vector-ref array i))
	 (hash (hash-bits (hamt/hash hamt) key))
	 (datum (and payload? (vector-ref array (+ i 1)))))
    (vector-set! array
		 i
		 (if (= h hash)
		     (make-collision (list (make-entry k d)
					   (make-entry key datum))
				     h)
		     (make-narrow-with-two-keys
		      payload?
		      (+ shift hamt-hash-slice-size)
		      h
		      k
		      d
		      hash
		      key
		      datum)))
    (when payload?
      (vector-set! array (+ i 1) #f))
    (set-wide/children! n (copy-bit fragment (wide/children n) #t))
    (set-wide/leaves! n (copy-bit fragment (wide/leaves n) #f))
    (values 1 n)))

(define (modify-wide-new hamt n shift d h k)
  (let* ((fragment (hash-fragment shift h))
	 (array (wide/array n))
	 (payload? (hamt/payload? hamt))
	 (stride (leaf-stride payload?))
	 (i (* stride fragment)))
    (vector-set! array i k)
    (when payload?
      (vector-set! array (+ i 1) d))
    (set-wide/leaves! n (copy-bit fragment (wide/leaves n) #t))
    (values 1 n)))

(define (make-narrow-with-two-keys payload? shift h1 k1 d1 h2 k2 d2)
  (define (two-leaves f1 k1 d1 f2 k2 d2)
    (make-narrow
     (if payload?
	 (vector k1 d1 k2 d2)
	 (vector k1 k2))
     0
     (copy-bit f2 (copy-bit f1 0 #t) #t)))
  (assert (not (= h1 h2)))
  (let ((f1 (hash-fragment shift h1))
	(f2 (hash-fragment shift h2)))
    (cond ((= f1 f2)
	   (make-narrow
	    (vector (make-narrow-with-two-keys payload?
					       (+ shift hamt-hash-slice-size)
					       h1
					       k1
					       d1
					       h2
					       k2
					       d2))
	    (copy-bit f1 0 #t)
	    0))
	  ((< f1 f2)
	   (two-leaves f1 k1 d1 f2 k2 d2))
	  (else
	   (two-leaves f2 k2 d2 f1 k1 d1)))))

(define (modify-pure hamt n shift dp h k)
  (cond ((collision? n) (modify-collision hamt n shift dp h k))
	((narrow? n) (modify-narrow hamt n shift dp h k))
	((wide? n) (error "Should have been converted to narrow before here."))
	(else (error "Unknown HAMT node type." n))))

(define (lower-collision hamt n shift dp h k)
  "If we try to add a key to a collision but it has a different hash
than the collision's elements, add it to a narrow above the collision
instead.  Add as many levels of child-only narrows as needed to reach
the point where the hash fragments differ.  This is guaranteed to
happen at some level because we're only called when the full hashes
differ."
  (let ((collision-hash (collision/hash n))
	(d (dp hamt-null)))
    (if (hamt-null? d)
	(values 0 n)
	(values
	 1
	 (let descend ((shift shift))
	   (let ((collision-fragment (hash-fragment shift collision-hash))
		 (leaf-fragment (hash-fragment shift h)))
	     (if (= collision-fragment leaf-fragment)
		 (let ((child (descend (+ shift hamt-hash-slice-size))))
		   (make-narrow
		    (vector child)
		    (copy-bit collision-fragment 0 #t)
		    0))
		 (make-narrow
		  (if (hamt/payload? hamt)
		      (vector k d n)
		      (vector k n))
		  (copy-bit collision-fragment 0 #t)
		  (copy-bit leaf-fragment 0 #t)))))))))

(define (modify-collision hamt n shift dp h k)
  (if (= h (collision/hash n))
      (let ((payload? (hamt/payload? hamt)))
	(let next ((entries (collision/entries n))
		   (checked '()))
	  (if (null? entries)
	      (let ((d (dp hamt-null)))
		(if (hamt-null? d)
		    (values 0 n)
		    (values 1
			    (make-collision (if payload?
						(cons (cons k d) checked)
						(cons k checked))
					    h))))
	      (let* ((entry (car entries))
		     (key (if payload? (car entry) entry)))
		(if ((hamt/= hamt) k key)
		    (let* ((existing (if payload? (cdr entry) hamt-null))
			   (d (dp existing))
			   (delete? (hamt-null? d))
			   (others (append checked (cdr entries))))
		      (values
		       (if delete? -1 0)
		       (make-collision (cond (delete? others)
					     (payload? (cons (cons k d) others))
					     (else (cons k others)))
				       h)))
		    (next (cdr entries)
			  (cons (car entries) checked)))))))
      (lower-collision hamt n shift dp h k)))

;; If we're storing "payloads," i.e. a datum to go with each key, we
;; must reserve two spots for each key in each vector.  Otherwise, we
;; need only one.
(define (leaf-stride payload?)
  (if payload? 2 1))

(define (narrow-child-index l c mask payload?)
  (+ (* (leaf-stride payload?) (bit-count l))
     (bit-count (bitwise-and c mask))))

(define (narrow-leaf-index l mask payload?)
  (* (leaf-stride payload?) (bit-count (bitwise-and l mask))))

(define (modify-narrow hamt n shift dp h k)
  (let ((fragment (hash-fragment shift h)))
    (cond ((bit-set? fragment (narrow/children n))
	   (modify-narrow-child hamt n shift dp h k))
	  ((bit-set? fragment (narrow/leaves n))
	   (modify-narrow-leaf hamt n shift dp h k))
	  (else
	   (let ((d (dp hamt-null)))
	     (if (hamt-null? d)
		 (values 0 n)
		 (modify-narrow-new hamt n shift d h k)))))))

(define (modify-narrow-child hamt n shift dp h k)
  (let*-values (((fragment) (hash-fragment shift h))
		((mask) (fragment->mask fragment))
		((c) (narrow/children n))
		((l) (narrow/leaves n))
		((array) (narrow/array n))
		((payload?) (hamt/payload? hamt))
		((child-index)
		 (narrow-child-index l c mask payload?))
		((child) (vector-ref array child-index))
		((change new-child)
		 (modify-pure hamt
			      child
			      (+ shift hamt-hash-slice-size)
			      dp
			      h
			      k)))
    (define (coalesce key datum)
      (let ((leaf-index (narrow-leaf-index l mask payload?)))
	(values change
		(make-narrow (if payload?
				 (vector-edit array
					      (add leaf-index key)
					      (add leaf-index datum)
					      (drop child-index 1))
				 (vector-edit array
					      (add leaf-index key)
					      (drop child-index 1)))
			     (copy-bit fragment c #f)
			     (copy-bit fragment l #t)))))
    (define (replace)
      (values change
	      (make-narrow (vector-replace-one array child-index new-child)
			   c
			   l)))
    (cond ((eq? new-child child) (values 0 n))
	  ((hamt-null? new-child)
	   (error "Child cannot become null." n))
	  ((collision? new-child)
	   (if (collision-single-leaf? new-child)
	       (let ((a (car (collision/entries new-child))))
		 (if payload?
		     (coalesce (car a) (cdr a))
		     (coalesce a #f)))
	       (replace)))
	  ((narrow? new-child)
	   (if (narrow-single-leaf? new-child)
	       (let ((a (narrow/array new-child)))
		 (coalesce (vector-ref a 0)
			   (and payload? (vector-ref a 1))))
	       (replace)))
	  ((wide? new-child)
	   (error "New child should be collision or narrow."))
	  (else (error "Unexpected type of child node.")))))

(define (modify-narrow-leaf hamt n shift dp h k)
  (let* ((fragment (hash-fragment shift h))
	 (mask (fragment->mask fragment))
	 (c (narrow/children n))
	 (l (narrow/leaves n))
	 (array (narrow/array n))
	 (payload? (hamt/payload? hamt))
	 (stride (leaf-stride payload?))
	 (leaf-index (narrow-leaf-index l mask payload?))
	 (key (vector-ref array leaf-index)))
    (if ((hamt/= hamt) k key)
	(let* ((existing (if payload?
			     (vector-ref array (+ leaf-index 1))
			     hamt-null))
	       (d (dp existing)))
	  (cond ((hamt-null? d)
		 (values -1
			 (make-narrow (vector-without array
						      leaf-index
						      (+ leaf-index stride))
				      c
				      (copy-bit fragment l #f))))
		(payload?
		 (values
		  0
		  (make-narrow (vector-replace-one array (+ leaf-index 1) d)
			       c
			       l)))
		(else (values 0 n))))
	(let ((d (dp hamt-null)))
	  (if (hamt-null? d)
	      (values 0 n)
	      (add-narrow-leaf-key hamt n shift d h k))))))

(define (add-narrow-leaf-key hamt n shift d h k)
  (define payload? (hamt/payload? hamt))
  (define make-entry
    (if payload? cons (lambda (k d) k)))
  (let* ((fragment (hash-fragment shift h))
	 (mask (fragment->mask fragment))
	 (c (narrow/children n))
	 (l (narrow/leaves n))
	 (array (narrow/array n))
	 (payload? (hamt/payload? hamt))
	 (stride (leaf-stride payload?))
	 (leaf-index (narrow-leaf-index l mask payload?))
	 (key (vector-ref array leaf-index))
	 (child-index (narrow-child-index l c mask payload?))
	 (hash (hash-bits (hamt/hash hamt) key))
	 (datum (and payload? (vector-ref array (+ leaf-index 1)))))
    (values 1
	    (make-narrow (if (= h hash)
			     (vector-edit
			      array
			      (drop leaf-index stride)
			      (add child-index
				   (make-collision (list (make-entry k d)
							 (make-entry key datum))
						   h)))
			     (vector-edit
			      array
			      (drop leaf-index stride)
			      (add child-index
				   (make-narrow-with-two-keys
				    payload?
				    (+ shift hamt-hash-slice-size)
				    h
				    k
				    d
				    hash
				    key
				    datum))))
			 (copy-bit fragment c #t)
			 (copy-bit fragment l #f)))))

(define (modify-narrow-new hamt n shift d h k)
  (let* ((fragment (hash-fragment shift h))
	 (mask (fragment->mask fragment))
	 (c (narrow/children n))
	 (l (narrow/leaves n))
	 (array (narrow/array n))
	 (payload? (hamt/payload? hamt))
	 (leaf-index (narrow-leaf-index l mask payload?))
	 (delete? (hamt-null? d)))
    (values 1
	    (make-narrow (if payload?
			     (vector-edit array
					  (add leaf-index k)
					  (add leaf-index d))
			     (vector-edit array
					  (add leaf-index k)))
			 c
			 (copy-bit fragment l #t)))))

(define (hamt-fetch hamt key)
  "Fetch datum from `hamt' at `key'.  Return `hamt-null' if the key is
not present.  If `hamt' stores no payloads, return the symbol
`present' if the key is present."
  (let ((h (hash-bits (hamt/hash hamt) key))
	(payload? (hamt/payload? hamt)))
    (let descend ((n (hamt/root hamt))
		  (shift 0))
      (cond ((collision? n)
	     (let ((entries (collision/entries n))
		   (key= (hamt/= hamt)))
	       (if payload?
		   (cond ((assoc key entries key=) => cdr)
			 (else hamt-null))
		   (if (find-tail (lambda (e) (key= key e)) entries)
		       'present
		       hamt-null))))
	    ((narrow? n)
	     (let ((array (narrow/array n))
		   (c (narrow/children n))
		   (l (narrow/leaves n))
		   (fragment (hash-fragment shift h)))
	       (cond ((bit-set? fragment c)
		      (let* ((mask (fragment->mask fragment))
			     (child-index (narrow-child-index
					   l
					   c
					   mask
					   (hamt/payload? hamt))))
			(descend (vector-ref array child-index)
				 (+ shift hamt-hash-slice-size))))
		     ((bit-set? fragment l)
		      (let* ((mask (fragment->mask fragment))
			     (leaf-index
			      (narrow-leaf-index l mask (hamt/payload? hamt)))
			     (k (vector-ref array leaf-index)))
			(if ((hamt/= hamt) k key)
			    (if payload?
				(vector-ref array (+ leaf-index 1))
				'present)
			    hamt-null)))
		     (else hamt-null))))
	    ((wide? n)
	     (let ((array (wide/array n))
		   (stride (leaf-stride (hamt/payload? hamt)))
		   (c (wide/children n))
		   (l (wide/leaves n))
		   (i (hash-fragment shift h)))
	       (cond ((bit-set? i c)
		      (descend (vector-ref array (* stride i))
			       (+ shift hamt-hash-slice-size)))
		     ((bit-set? i l)
		      (let* ((j (* stride i))
			     (k (vector-ref array j)))
			(if ((hamt/= hamt) k key)
			    (if payload?
				(vector-ref array (+ j 1))
				'present)
			    hamt-null)))
		     (else hamt-null))))
	    (else (error "Unexpected type of child node."))))))

(define (collision/for-each procedure node payload?)
  (if payload?
      (do-list (e (collision/entries node))
	(procedure (car e) (cdr e)))
      (do-list (e (collision/entries node))
	(procedure e #f))))

(define (narrow/for-each procedure node payload?)
  (let ((array (narrow/array node))
	(stride (leaf-stride payload?))
	(c (narrow/children node))
	(l (narrow/leaves node)))
    (let next-leaf ((count 0)
		    (start 0))
      (let ((i (next-set-bit l start hamt-bucket-size)))
	(if i
	    (let* ((j (* stride count))
		   (k (vector-ref array j))
		   (d (and payload? (vector-ref array (+ j 1)))))
	      (procedure k d)
	      (next-leaf (+ count 1) (+ i 1)))
	    (let next-child ((start 0)
			     (offset (* stride count)))
	      (let ((i (next-set-bit c start hamt-bucket-size)))
		(when i
		  (let ((child (vector-ref array offset)))
		    (hamt-node/for-each child payload? procedure)
		    (next-child (+ i 1) (+ offset 1)))))))))))

(define (wide/for-each procedure node payload?)
  (let ((array (wide/array node))
	(stride (leaf-stride payload?))
	(c (wide/children node))
	(l (wide/leaves node)))
    (do ((i 0 (+ i 1)))
	((= i hamt-bucket-size))
      (let ((j (* stride i)))
	(cond ((bit-set? i l)
	       (let ((k (vector-ref array j))
		     (d (and payload? (vector-ref array (+ j 1)))))
		 (procedure k d)))
	      ((bit-set? i c)
	       (let ((child (vector-ref array j)))
		 (hamt-node/for-each child payload? procedure))))))))

(define (hamt-node/for-each node payload? procedure)
  (cond ((collision? node) (collision/for-each procedure node payload?))
	((narrow? node) (narrow/for-each procedure node payload?))
	((wide? node) (wide/for-each procedure node payload?))
	(else (error "Invalid type of node." node))))

(define (hamt/for-each procedure hamt)
  (hamt-node/for-each (hamt/root hamt)
		      (hamt/payload? hamt)
		      procedure))

(define (hamt->list hamt procedure)
  (let ((accumulator '()))
    (hamt/for-each (lambda (k v)
		     (set! accumulator
			   (cons (procedure k v)
				 accumulator)))
		   hamt)
    accumulator))

;;; Debugging

(define (assert-collision-valid node hp payload?)
  "Do sanity checks on a collision.  Return the list of all keys
present."
  (let ((entries (collision/entries node))
	(hash (collision/hash node))
	(extract (if payload? car (lambda (x) x))))
    (do-list (a entries)
      (assert (= hash (hash-bits hp (extract a)))))
    (if payload?
	(map car entries)
	entries)))

(define (assert-narrow-valid node hp payload? shift)
  "Do sanity checks on a narrow and all its children.  Return the list
of all keys present."
  (let ((array (narrow/array node))
	(stride (leaf-stride payload?))
	(c (narrow/children node))
	(l (narrow/leaves node)))
    (assert (zero? (bitwise-and c l)))
    (let next-leaf ((count 0)
		    (i 0)
		    (keys '()))
      (if (< i hamt-bucket-size)
	  (cond ((bit-set? i l)
		 (let ((k (vector-ref array (* stride count))))
		   (assert (= i (hash-fragment shift (hash-bits hp k))))
		   (next-leaf (+ count 1) (+ i 1) (cons k keys))))
		(else (next-leaf count (+ i 1) keys)))
	  (let next-child ((i 0)
			   (key-groups (list keys))
			   (offset (* stride count)))
	    (if (= i hamt-bucket-size)
		(apply append key-groups)
		(cond ((bit-set? i c)
		       (let* ((child (vector-ref array offset))
			      (child-keys (assert-hamt-node-valid
					   child
					   hp
					   payload?
					   (+ shift hamt-hash-slice-size))))
			 (do-list (k child-keys)
			   (assert (= i
				      (hash-fragment shift (hash-bits hp k)))))
			 (next-child (+ i 1)
				     (cons child-keys key-groups)
				     (+ offset 1))))
		      (else (next-child (+ i 1) key-groups offset)))))))))

(define (assert-wide-valid node hp payload? shift)
  "Do sanity checks on a wide and all its children.  Return the list
of all keys present."
  (let ((array (wide/array node))
	(stride (leaf-stride payload?))
	(c (wide/children node))
	(l (wide/leaves node)))
    (assert (zero? (bitwise-and c l)))
    (let next-fragment ((i 0)
			(key-groups '()))
      (if (= i hamt-bucket-size)
	  (apply append key-groups)
	  (let ((j (* stride i)))
	    (cond ((bit-set? i l)
		   (let ((k (vector-ref array j)))
		     (assert (= i (hash-fragment shift (hash-bits hp k))))
		     (next-fragment (+ i 1) (cons (list k) key-groups))))
		  ((bit-set? i c)
		   (let* ((child (vector-ref array j))
			  (child-keys (assert-hamt-node-valid
				       child
				       hp
				       payload?
				       (+ shift hamt-hash-slice-size))))
		     (do-list (k child-keys)
		       (assert (= i
				  (hash-fragment shift (hash-bits hp k)))))
		     (next-fragment (+ i 1)
				    (cons child-keys key-groups))))
		  (else
		   (assert (not (vector-ref array j)))
		   (when payload?
		     (assert (not (vector-ref array (+ j 1)))))
		   (next-fragment (+ i 1) key-groups))))))))

(define (assert-hamt-node-valid node hp payload? shift)
  "Do sanity checks on a HAMT node and all its children.  Return the
list of all keys present."
  (cond ((collision? node) (assert-collision-valid node hp payload?))
	((narrow? node) (assert-narrow-valid node hp payload? shift))
	((wide? node) (assert-wide-valid node hp payload? shift))
	(else (error "Invalid type of node." node))))

(define (assert-hamt-valid hamt)
  "Do sanity checks on `hamt'."
  (let ((hp (hamt/hash hamt)))
    (assert (procedure? (hamt/= hamt)))
    (assert (procedure? hp))
    (assert (memq (hamt/mutable? hamt) '(#t #f)))
    (let* ((payload? (hamt/payload? hamt))
	   (keys (assert-hamt-node-valid (hamt/root hamt) hp payload? 0)))
      (assert (= (hamt/count hamt) (length keys))))))


;;; Copyright MMXV-MMXVII Arthur A. Gleckler.  All rights reserved.

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
;; NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
;; HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
;; WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
;; OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
;; DEALINGS IN THE SOFTWARE.

;;; Public protocol (API)

;; (phm? datum)

;;   Return true iff `datum' is a persistent hash map.

;; (make-phm hash = [alist])

;;   Return a new immutable persistent hash map that uses the `hash'
;;   procedure to hash its keys and `=' to compare them.  If `alist'
;;   is supplied, include all its keys and data in the result.  Later
;;   occurrences of the same key override earlier ones.

;; (phm/count phm)

;;   Return the number of elements in `phm'.

;; (phm/empty? phm)

;;   Return true iff `phm' is empty.

;; (phm/immutable phm)
;; (phm/immutable phm replace)

;;   Return a PHM equivalent to `phm', but that is immutable.  Even if
;;   `phm' is mutable, no change to it will affect the returned one.
;;   If `replace' is supplied, replace the datum associated with each
;;   key whose value has been modified since the PHM was made mutable
;;   with the result of calling `replace' on that key and datum.  This
;;   is useful for converting PHSs (sets) stored as values in a PHM
;;   back into immutable ones when the containing PHM is made
;;   immutable.

;; (phm/mutable phm)

;;   Return a PHM equivalent to `phm', but that is mutable.  If `phm'
;;   was immutable, no change to the returned PHM will affect `phm'.

;; (phm/mutable? phm)

;;   Return true iff `phm' is mutable.

;; (phm/put phm key datum)

;;   Return a PHM equivalent to `phm' except that `datum' is at `key'.

;; (phm/put! phm key datum)

;;   Return a PHM equivalent to `phm' except that `datum' is at `key'.
;;   Modify `phm', which must be mutable, in the process.

;; (phm/replace phm key replace)

;;   Return a PHM equivalent to `phm' except that whatever value is at
;;   `key' has been replaced by `(replace datum)', or `(replace
;;   hamt-null)' if there was no value there already.  If `replace'
;;   returns `hamt-null', the value is removed.

;; (phm/replace! phm key replace)

;;   Return a PHM equivalent to `phm' except that whatever value is at
;;   `key' has been replaced by `(replace datum)', or `(replace
;;   hamt-null)' if there was no value there already.  If `replace'
;;   returns `hamt-null', the value is removed.  Modify `phm', which
;;   must be mutable, in the process.

;; (phm/get phm key [default])

;;   Return the datum stored at `key' in `phm'.  If none is present,
;;   return `default' if it was supplied, or #f if it was not.

;; (phm/contains? phm key)

;;   Return true iff `phm' has a datum at `key'.

;; (phm/remove phm key)

;;   Return a PHM equivalent to `phm' except that there is no datum at
;;   `key'.

;; (phm/remove! phm key)

;;   Return a PHM equivalent to `phm' except that there is no datum at
;;   `key'.  Modify `phm', which must be mutable, in the process.

;; (phm/add-alist phm alist)

;;   Return a PHM equivalent to `phm' except that, for every pair in
;;   `alist', the datum in its cdr is stored in the new PHM at the key
;;   in its car.  Later occurrences of the same key override earlier
;;   ones.

;; (phm/add-alist! phm alist)

;;   Return a PHM equivalent to `phm' except that, for every pair in
;;   `alist', the datum in its cdr is stored in the new PHM at the key
;;   in its car.  Later occurrences of the same key override earlier
;;   ones.  Modify `phm', which must be mutable, in the process.

;; (phm->alist phm)

;;   Return an alist mapping the keys in `phm' to their values.

;; (phm/keys phm)

;;   Return a list of the keys in `phm'.

;; (phm/for-each procedure phm)

;;   Run `procedure' on each key and datum in `phm'.

;;; Implementation of public protocol (API)

(define (phm? datum)
  (and (hash-array-mapped-trie? datum)
       (hamt/payload? datum)))

(define (make-phm-inner hash = alist)
  (let ((phm (make-hamt = hash #t)))
    (if (null? alist)
    phm
    (let ((phm-1 (phm/mutable phm)))
      (phm/add-alist! phm-1 alist)
      (phm/immutable phm-1)))))

(define make-phm
  (case-lambda
   ((hash =) (make-phm-inner hash = '()))
   ((hash = alist) (make-phm-inner hash = alist))))

(define (phm/count phm)
  (assert (phm? phm))
  (hamt/count phm))

(define (phm/empty? phm)
  (assert (phm? phm))
  (hamt/empty? phm))

(define phm/immutable
  (case-lambda
   ((phm)
    (assert (phm? phm))
    (hamt/immutable phm))
   ((phm replace)
    (assert (phm? phm))
    (hamt/immutable phm replace))))

(define (phm/mutable phm)
  (assert (phm? phm))
  (hamt/mutable phm))

(define (phm/mutable? phm)
  (assert (phm? phm))
  (hamt/mutable? phm))

(define (phm/put phm key datum)
  (assert (phm? phm))
  (hamt/put phm key datum))

(define (phm/put! phm key datum)
  (assert (phm? phm))
  (hamt/put! phm key datum))

(define (phm/replace phm key replace)
  (assert (phm? phm))
  (hamt/replace phm key replace))

(define (phm/replace! phm key replace)
  (assert (phm? phm))
  (hamt/replace! phm key replace))

(define (phm/get-inner phm key default)
  (assert (phm? phm))
  (let ((result (hamt-fetch phm key)))
    (if (hamt-null? result)
    default
    result)))

(define phm/get
  (case-lambda
   ((phm key) (phm/get-inner phm key #f))
   ((phm key default) (phm/get-inner phm key default))))

(define (phm/contains? phm key)
  (assert (phm? phm))
  (not (hamt-null? (hamt-fetch phm key))))

(define (phm/remove phm key)
  (assert (phm? phm))
  (phm/put phm key hamt-null))

(define (phm/remove! phm key)
  (assert (phm? phm))
  (assert (hamt/mutable? phm))
  (phm/put! phm key hamt-null))

(define (phm/add-alist phm alist)
  (assert (phm? phm))
  (fold (lambda (a phm) (phm/put phm (car a) (cdr a))) phm alist))

(define (phm/add-alist! phm alist)
  (assert (phm? phm))
  (do-list (a alist)
    (phm/put! phm (car a) (cdr a)))
  phm)

(define (phm->alist phm)
  (assert (phm? phm))
  (hamt->list phm cons))

(define (phm/data phm)
  (assert (phm? phm))
  (hamt->list phm (lambda (k d) d)))

(define (phm/keys phm)
  (assert (phm? phm))
  (hamt->list phm (lambda (k d) k)))

(define (phm/for-each procedure phm)
  (assert (phm? phm))
  (hamt/for-each procedure phm))
 

;; hashmap

;; Copyright (C) Marc Nieper-Wiﬂkirchen (2018).  All Rights
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

;;; Implementation layer

(define (tree-search comparator tree obj failure success)
  (let ((entry (phm/get tree obj)))
    (if entry
    (success (car entry) (cdr entry)
         (lambda (new-key new-datum ret)
           (let ((tree (phm/remove tree obj)))
             (values (phm/put tree new-key (cons new-key new-datum))
                 ret)))
         (lambda (ret)
           (values (phm/remove tree obj) ret)))
    (failure (lambda (new-key new-datum ret)
           (values (phm/put tree new-key (cons new-key new-datum))
               ret))
         (lambda (ret)
           (values tree ret))))))

(define (tree-fold proc seed tree)
  (phm/for-each (lambda (key entry)
          (set! seed (proc (car entry) (cdr entry) seed)))
        tree)
  seed)

(define (tree-for-each proc tree)
  (phm/for-each (lambda (key entry)
          (proc (car entry) (cdr entry)))
        tree))

(define (tree-generator tree)
  (make-coroutine-generator
   (lambda (yield)
     (tree-for-each (lambda item (yield item))
            tree))))

;;; New types

(define-record-type <hashmap>
  (%make-hashmap comparator tree)
  hashmap?
  (comparator hashmap-key-comparator)
  (tree hashmap-tree))

(define (make-empty-hashmap comparator)
  (assume (comparator? comparator))
  (%make-hashmap comparator
         (make-phm (comparator-hash-function comparator)
               (comparator-equality-predicate comparator))))

;;; Exported procedures

;; Constructors

(define (hashmap comparator . args)
  (assume (comparator? comparator))
  (hashmap-unfold null?
          (lambda (args)
        (values (car args)
            (cadr args)))
          cddr
          args
          comparator))

(define (hashmap-unfold stop? mapper successor seed comparator)
  (assume (procedure? stop?))
  (assume (procedure? mapper))
  (assume (procedure? successor))
  (assume (comparator? comparator))
  (let loop ((hashmap (make-empty-hashmap comparator))
         (seed seed))
    (if (stop? seed)
    hashmap
    (receive (key value)
        (mapper seed)
      (loop (hashmap-adjoin hashmap key value)
        (successor seed))))))

;; Predicates

(define (hashmap-empty? hashmap)
  (assume (hashmap? hashmap))
  (not (hashmap-any? (lambda (key value) #t) hashmap)))

(define (hashmap-contains? hashmap key)
  (assume (hashmap? hashmap))
  (call/cc
   (lambda (return)
     (hashmap-search hashmap
         key
         (lambda (insert ignore)
           (return #f))
         (lambda (key value update remove)
           (return #t))))))

(define (hashmap-disjoint? hashmap1 hashmap2)
  (assume (hashmap? hashmap1))
  (assume (hashmap? hashmap2))
  (call/cc
   (lambda (return)
     (hashmap-for-each (lambda (key value)
             (when (hashmap-contains? hashmap2 key)
               (return #f)))
           hashmap1)
     #t)))

;; Accessors

(define hashmap-ref
  (case-lambda
    ((hashmap key)
     (assume (hashmap? hashmap))
     (hashmap-ref hashmap key (lambda ()
            (error "hashmap-ref: key not in hashmap" key))))
    ((hashmap key failure)
     (assume (hashmap? hashmap))
     (assume (procedure? failure))
     (hashmap-ref hashmap key failure (lambda (value)
                value)))
    ((hashmap key failure success)
     (assume (hashmap? hashmap))
     (assume (procedure? failure))
     (assume (procedure? success))
     ((call/cc
       (lambda (return-thunk)
     (hashmap-search hashmap
             key
             (lambda (insert ignore)
               (return-thunk failure))
             (lambda (key value update remove)
               (return-thunk (lambda () (success value)))))))))))

(define (hashmap-ref/default hashmap key default)
  (assume (hashmap? hashmap))
  (hashmap-ref hashmap key (lambda () default)))

;; Updaters

(define (hashmap-adjoin hashmap . args)
  (assume (hashmap? hashmap))
  (let loop ((args args)
         (hashmap hashmap))
    (if (null? args)
    hashmap
    (receive (hashmap value)
        (hashmap-intern hashmap (car args) (lambda () (cadr args)))
      (loop (cddr args) hashmap)))))

(define hashmap-adjoin! hashmap-adjoin)

(define (hashmap-set hashmap . args)
  (assume (hashmap? hashmap))
  (let loop ((args args)
         (hashmap hashmap))
    (if (null? args)
    hashmap
    (receive (hashmap)
        (hashmap-update hashmap (car args) (lambda (value) (cadr args)) (lambda () #f))
      (loop (cddr args)
        hashmap)))))

(define hashmap-set! hashmap-set)

(define (hashmap-replace hashmap key value)
  (assume (hashmap? hashmap))
  (receive (hashmap obj)
      (hashmap-search hashmap
          key
          (lambda (insert ignore)
            (ignore #f))
          (lambda (old-key old-value update remove)
            (update key value #f)))
    hashmap))

(define hashmap-replace! hashmap-replace)

(define (hashmap-delete hashmap . keys)
  (assume (hashmap? hashmap))
  (hashmap-delete-all hashmap keys))

(define hashmap-delete! hashmap-delete)

(define (hashmap-delete-all hashmap keys)
  (assume (hashmap? hashmap))
  (assume (list? keys))
  (fold (lambda (key hashmap)
      (receive (hashmap obj)
          (hashmap-search hashmap
              key
              (lambda (insert ignore)
                (ignore #f))
              (lambda (old-key old-value update remove)
                (remove #f)))
        hashmap))
    hashmap keys))

(define hashmap-delete-all! hashmap-delete-all)

(define (hashmap-intern hashmap key failure)
  (assume (hashmap? hashmap))
  (assume (procedure? failure))
  (call/cc
   (lambda (return)
     (hashmap-search hashmap
         key
         (lambda (insert ignore)
           (receive (value)
               (failure)
             (insert value value)))
         (lambda (old-key old-value update remove)
           (return hashmap old-value))))))

(define hashmap-intern! hashmap-intern)

(define hashmap-update
  (case-lambda
   ((hashmap key updater)
    (hashmap-update hashmap key updater (lambda ()
                  (error "hashmap-update: key not found in hashmap" key))))
   ((hashmap key updater failure)
    (hashmap-update hashmap key updater failure (lambda (value)
                      value)))
   ((hashmap key updater failure success)
    (assume (hashmap? hashmap))
    (assume (procedure? updater))
    (assume (procedure? failure))
    (assume (procedure? success))
    (receive (hashmap obj)
    (hashmap-search hashmap
            key
            (lambda (insert ignore)
              (insert (updater (failure)) #f))
            (lambda (old-key old-value update remove)
              (update key (updater (success old-value)) #f)))
      hashmap))))

(define hashmap-update! hashmap-update)

(define (hashmap-update/default hashmap key updater default)
  (hashmap-update hashmap key updater (lambda () default)))

(define hashmap-update!/default hashmap-update/default)

(define hashmap-pop
  (case-lambda
    ((hashmap)
     (hashmap-pop hashmap (lambda ()
                (error "hashmap-pop: hashmap has no association"))))
    ((hashmap failure)
     (assume (hashmap? hashmap))
     (assume (procedure? failure))
     ((call/cc
       (lambda (return-thunk)
     (receive (key value)
         (hashmap-find (lambda (key value) #t) hashmap (lambda () (return-thunk failure)))
       (lambda ()
         (values (hashmap-delete hashmap key) key value)))))))))

(define hashmap-pop! hashmap-pop)

(define (hashmap-search hashmap key failure success)
  (assume (hashmap? hashmap))
  (assume (procedure? failure))
  (assume (procedure? success))
  (call/cc
   (lambda (return)
     (let*-values
     (((comparator)
       (hashmap-key-comparator hashmap))
      ((tree obj)
       (tree-search comparator
            (hashmap-tree hashmap)
            key
            (lambda (insert ignore)
              (failure (lambda (value obj)
                     (insert key value obj))
                   (lambda (obj)
                     (return hashmap obj))))
            success)))
       (values (%make-hashmap comparator tree)
           obj)))))

(define hashmap-search! hashmap-search)

;; The whole hashmap

(define (hashmap-size hashmap)
  (assume (hashmap? hashmap))
  (hashmap-count (lambda (key value)
           #t)
         hashmap))

(define (hashmap-find predicate hashmap failure)
  (assume (procedure? predicate))
  (assume (hashmap? hashmap))
  (assume (procedure? failure))
  (call/cc
   (lambda (return)
     (hashmap-for-each (lambda (key value)
             (when (predicate key value)
               (return key value)))
           hashmap)
     (failure))))

(define (hashmap-count predicate hashmap)
  (assume (procedure? predicate))
  (assume (hashmap? hashmap))
  (hashmap-fold (lambda (key value count)
          (if (predicate key value)
          (+ 1 count)
          count))
        0 hashmap))

(define (hashmap-any? predicate hashmap)
  (assume (procedure? predicate))
  (assume (hashmap? hashmap))
  (call/cc
   (lambda (return)
     (hashmap-for-each (lambda (key value)
             (when (predicate key value)
               (return #t)))
           hashmap)
     #f)))

(define (hashmap-every? predicate hashmap)
  (assume (procedure? predicate))
  (assume (hashmap? hashmap))
  (not (hashmap-any? (lambda (key value)
           (not (predicate key value)))
         hashmap)))

(define (hashmap-keys hashmap)
  (assume (hashmap? hashmap))
  (hashmap-fold (lambda (key value keys)
          (cons key keys))
        '() hashmap))

(define (hashmap-values hashmap)
  (assume (hashmap? hashmap))
  (hashmap-fold (lambda (key value values)
          (cons value values))
        '() hashmap))

(define (hashmap-entries hashmap)
  (assume (hashmap? hashmap))
  (values (hashmap-keys hashmap)
      (hashmap-values hashmap)))

;; Hashmap and folding

(define (hashmap-map proc comparator hashmap)
  (assume (procedure? proc))
  (assume (comparator? comparator))
  (assume (hashmap? hashmap))
  (hashmap-fold (lambda (key value hashmap)
          (receive (key value)
          (proc key value)
        (hashmap-set hashmap key value)))
        (make-empty-hashmap comparator)
        hashmap))

(define (hashmap-for-each proc hashmap)
  (assume (procedure? proc))
  (assume (hashmap? hashmap))
  (tree-for-each proc (hashmap-tree hashmap)))

(define (hashmap-fold proc acc hashmap)
  (assume (procedure? proc))
  (assume (hashmap? hashmap))
  (tree-fold proc acc (hashmap-tree hashmap)))

(define (hashmap-map->list proc hashmap)
  (assume (procedure? proc))
  (assume (hashmap? hashmap))
  (hashmap-fold (lambda (key value lst)
          (cons (proc key value) lst))
        '()
        hashmap))

(define (hashmap-filter predicate hashmap)
  (assume (procedure? predicate))
  (assume (hashmap? hashmap))
  (hashmap-fold (lambda (key value hashmap)
          (if (predicate key value)
          (hashmap-set hashmap key value)
          hashmap))
        (make-empty-hashmap (hashmap-key-comparator hashmap))
        hashmap))

(define hashmap-filter! hashmap-filter)

(define (hashmap-remove predicate hashmap)
  (assume (procedure? predicate))
  (assume (hashmap? hashmap))
  (hashmap-filter (lambda (key value)
        (not (predicate key value)))
          hashmap))

(define hashmap-remove! hashmap-remove)

(define (hashmap-partition predicate hashmap)
  (assume (procedure? predicate))
  (assume (hashmap? hashmap))
  (values (hashmap-filter predicate hashmap)
      (hashmap-remove predicate hashmap)))

(define hashmap-partition! hashmap-partition)

;; Copying and conversion

(define (hashmap-copy hashmap)
  (assume (hashmap? hashmap))
  hashmap)

(define (hashmap->alist hashmap)
  (assume (hashmap? hashmap))
  (hashmap-fold (lambda (key value alist)
          (cons (cons key value) alist))
        '() hashmap))

(define (alist->hashmap comparator alist)
  (assume (comparator? comparator))
  (assume (list? alist))
  (hashmap-unfold null?
          (lambda (alist)
        (let ((key (caar alist))
              (value (cdar alist)))
          (values key value)))
          cdr
          alist
          comparator))

(define (alist->hashmap! hashmap alist)
  (assume (hashmap? hashmap))
  (assume (list? alist))
  (fold (lambda (association hashmap)
      (let ((key (car association))
        (value (cdr association)))
        (hashmap-set hashmap key value)))
    hashmap
    alist))

;; Subhashmaps

(define hashmap=?
  (case-lambda
    ((comparator hashmap)
     (assume (hashmap? hashmap))
     #t)
    ((comparator hashmap1 hashmap2) (%hashmap=? comparator hashmap1 hashmap2))
    ((comparator hashmap1 hashmap2 . hashmaps)
     (and (%hashmap=? comparator hashmap1 hashmap2)
          (apply hashmap=? comparator hashmap2 hashmaps)))))
(define (%hashmap=? comparator hashmap1 hashmap2)
  (and (eq? (hashmap-key-comparator hashmap1) (hashmap-key-comparator hashmap2))
       (%hashmap<=? comparator hashmap1 hashmap2)
       (%hashmap<=? comparator hashmap2 hashmap1)))

(define hashmap<=?
  (case-lambda
    ((comparator hashmap)
     (assume (hashmap? hashmap))
     #t)
    ((comparator hashmap1 hashmap2)
     (assume (comparator? comparator))
     (assume (hashmap? hashmap1))
     (assume (hashmap? hashmap2))
     (%hashmap<=? comparator hashmap1 hashmap2))
    ((comparator hashmap1 hashmap2 . hashmaps)
     (assume (comparator? comparator))
     (assume (hashmap? hashmap1))
     (assume (hashmap? hashmap2))
     (and (%hashmap<=? comparator hashmap1 hashmap2)
          (apply hashmap<=? comparator hashmap2 hashmaps)))))

(define (%hashmap<=? comparator hashmap1 hashmap2)
  (assume (comparator? comparator))
  (assume (hashmap? hashmap1))
  (assume (hashmap? hashmap2))
  (hashmap-every? (lambda (key value)
            (hashmap-ref hashmap2 key
                 (lambda ()
                   #f)
                 (lambda (stored-value)
                   (=? comparator value stored-value))))
          hashmap1))

(define hashmap>?
  (case-lambda
    ((comparator hashmap)
     (assume (hashmap? hashmap))
     #t)
    ((comparator hashmap1 hashmap2)
     (assume (comparator? comparator))
     (assume (hashmap? hashmap1))
     (assume (hashmap? hashmap2))
     (%hashmap>? comparator hashmap1 hashmap2))
    ((comparator hashmap1 hashmap2 . hashmaps)
     (assume (comparator? comparator))
     (assume (hashmap? hashmap1))
     (assume (hashmap? hashmap2))
     (and (%hashmap>? comparator  hashmap1 hashmap2)
          (apply hashmap>? comparator hashmap2 hashmaps)))))

(define (%hashmap>? comparator hashmap1 hashmap2)
  (assume (comparator? comparator))
  (assume (hashmap? hashmap1))
  (assume (hashmap? hashmap2))
  (not (%hashmap<=? comparator hashmap1 hashmap2)))

(define hashmap<?
  (case-lambda
    ((comparator hashmap)
     (assume (hashmap? hashmap))
     #t)
    ((comparator hashmap1 hashmap2)
     (assume (comparator? comparator))
     (assume (hashmap? hashmap1))
     (assume (hashmap? hashmap2))
     (%hashmap<? comparator hashmap1 hashmap2))
    ((comparator hashmap1 hashmap2 . hashmaps)
     (assume (comparator? comparator))
     (assume (hashmap? hashmap1))
     (assume (hashmap? hashmap2))
     (and (%hashmap<? comparator  hashmap1 hashmap2)
          (apply hashmap<? comparator hashmap2 hashmaps)))))

(define (%hashmap<? comparator hashmap1 hashmap2)
     (assume (comparator? comparator))
     (assume (hashmap? hashmap1))
     (assume (hashmap? hashmap2))
     (%hashmap>? comparator hashmap2 hashmap1))

(define hashmap>=?
  (case-lambda
    ((comparator hashmap)
     (assume (hashmap? hashmap))
     #t)
    ((comparator hashmap1 hashmap2)
     (assume (comparator? comparator))
     (assume (hashmap? hashmap1))
     (assume (hashmap? hashmap2))
     (%hashmap>=? comparator hashmap1 hashmap2))
    ((comparator hashmap1 hashmap2 . hashmaps)
     (assume (comparator? comparator))
     (assume (hashmap? hashmap1))
     (assume (hashmap? hashmap2))
     (and (%hashmap>=? comparator hashmap1 hashmap2)
          (apply hashmap>=? comparator hashmap2 hashmaps)))))

(define (%hashmap>=? comparator hashmap1 hashmap2)
  (assume (comparator? comparator))
  (assume (hashmap? hashmap1))
  (assume (hashmap? hashmap2))
  (not (%hashmap<? comparator hashmap1 hashmap2)))

;; Set theory operations

(define (%hashmap-union hashmap1 hashmap2)
  (hashmap-fold (lambda (key2 value2 hashmap)
          (receive (hashmap obj)
              (hashmap-search hashmap
                      key2
                      (lambda (insert ignore)
                    (insert value2 #f))
                      (lambda (key1 value1 update remove)
                    (update key1 value1 #f)))
            hashmap))
        hashmap1 hashmap2))

(define (%hashmap-intersection hashmap1 hashmap2)
  (hashmap-filter (lambda (key1 value1)
        (hashmap-contains? hashmap2 key1))
          hashmap1))

(define (%hashmap-difference hashmap1 hashmap2)
  (hashmap-fold (lambda (key2 value2 hashmap)
          (receive (hashmap obj)
          (hashmap-search hashmap
                  key2
                  (lambda (insert ignore)
                (ignore #f))
                  (lambda (key1 value1 update remove)
                (remove #f)))
        hashmap))
        hashmap1 hashmap2))

(define (%hashmap-xor hashmap1 hashmap2)
  (hashmap-fold (lambda (key2 value2 hashmap)
          (receive (hashmap obj)
          (hashmap-search hashmap
                  key2
                  (lambda (insert ignore)
                (insert value2 #f))
                  (lambda (key1 value1 update remove)
                (remove #f)))
        hashmap))
        hashmap1 hashmap2))

(define hashmap-union
  (case-lambda
    ((hashmap)
     (assume (hashmap? hashmap))
     hashmap)
    ((hashmap1 hashmap2)
     (assume (hashmap? hashmap1))
     (assume (hashmap? hashmap2))
     (%hashmap-union hashmap1 hashmap2))
    ((hashmap1 hashmap2 . hashmaps)
     (assume (hashmap? hashmap1))
     (assume (hashmap? hashmap2))
     (apply hashmap-union (%hashmap-union hashmap1 hashmap2) hashmaps))))
(define hashmap-union! hashmap-union)

(define hashmap-intersection
  (case-lambda
    ((hashmap)
     (assume (hashmap? hashmap))
     hashmap)
    ((hashmap1 hashmap2)
     (assume (hashmap? hashmap1))
     (assume (hashmap? hashmap2))
     (%hashmap-intersection hashmap1 hashmap2))
    ((hashmap1 hashmap2 . hashmaps)
     (assume (hashmap? hashmap1))
     (assume (hashmap? hashmap2))
     (apply hashmap-intersection (%hashmap-intersection hashmap1 hashmap2) hashmaps))))
(define hashmap-intersection! hashmap-intersection)

(define hashmap-difference
  (case-lambda
    ((hashmap)
     (assume (hashmap? hashmap))
     hashmap)
    ((hashmap1 hashmap2)
     (assume (hashmap? hashmap1))
     (assume (hashmap? hashmap2))
     (%hashmap-difference hashmap1 hashmap2))
    ((hashmap1 hashmap2 . hashmaps)
     (assume (hashmap? hashmap1))
     (assume (hashmap? hashmap2))
     (apply hashmap-difference (%hashmap-difference hashmap1 hashmap2) hashmaps))))
(define hashmap-difference! hashmap-difference)

(define hashmap-xor
  (case-lambda
    ((hashmap)
     (assume (hashmap? hashmap))
     hashmap)
    ((hashmap1 hashmap2)
     (assume (hashmap? hashmap1))
     (assume (hashmap? hashmap2))
     (%hashmap-xor hashmap1 hashmap2))
    ((hashmap1 hashmap2 . hashmaps)
     (assume (hashmap? hashmap1))
     (assume (hashmap? hashmap2))
     (apply hashmap-xor (%hashmap-xor hashmap1 hashmap2) hashmaps))))
(define hashmap-xor! hashmap-xor)

;; Comparators

(define (hashmap-equality comparator)
  (assume (comparator? comparator))
  (lambda (hashmap1 hashmap2)
    (hashmap=? comparator hashmap1 hashmap2)))

(define (hashmap-hash-function comparator)
  (assume (comparator? comparator))
  (lambda (hashmap)
    0 ;; TODO
    #;
    (default-hash (hashmap->alist hashmap))))

(define (make-hashmap-comparator comparator)
  (make-comparator hashmap?
           (hashmap-equality comparator)
           #f
           (hashmap-hash-function comparator)))

(define hashmap-comparator (make-hashmap-comparator (make-default-comparator)))

(comparator-register-default! hashmap-comparator)

))
