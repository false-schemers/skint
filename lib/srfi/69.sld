
(define-library (srfi 69)
  (export ;[esl] provides extended functionality for SRFI-125
   make-hash-table hash-table? alist->hash-table 
   hash-table-equivalence-function hash-table-hash-function 
   hash-table-ref hash-table-ref/default hash-table-set! hash-table-delete!
   hash-table-exists? hash-table-update! hash-table-update!/default 
   hash-table-size hash-table-keys hash-table-values hash-table-walk
   hash-table-fold hash-table->alist hash-table-copy hash-table-merge! 
   hash string-hash string-ci-hash hash-by-identity)
  (import
   (only (skint) fixnum? flonum? fx+ fx* fxmodulo fx>=?)
   (only (skint hidden) immediate-hash)
   (scheme base)
   (scheme case-lambda)
   (scheme char)
   (srfi 1)
   (srfi 31))

(begin
;;; Copyright (C) Panu Kalliokoski (2005). All Rights Reserved.
;;; Made an R7RS library by Taylan Ulrich Bayirli/Kammer, Copyright (C) 2014.
;;; Modified by Sergei Egorov to work in Skint (modifications marked with [esl])

;;; Permission is hereby granted, free of charge, to any person obtaining a copy
;;; of this software and associated documentation files (the "Software"), to
;;; deal in the Software without restriction, including without limitation the
;;; rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
;;; sell copies of the Software, and to permit persons to whom the Software is
;;; furnished to do so, subject to the following conditions:

;;; The above copyright notice and this permission notice shall be included in
;;; all copies or substantial portions of the Software.

;;; THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
;;; IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
;;; FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
;;; AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
;;; LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
;;; FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
;;; IN THE SOFTWARE.

;[esl] lowered to the same value as in SRFI 128 to keep calculations in fixnum
;range when accumulating: (+ (* 15760399 33) 15760399) < fx-greatest
(define default-bound (make-parameter 15760399)) ; a prime

(define (%string-hash s ch-conv bound)
  (let ((hash 31)
        (len (string-length s)))
    (do ((index 0 (fx+ index 1)))
      ((fx>=? index len) (fxmodulo hash bound))
      (set! hash (fxmodulo (fx+ (fx* 37 hash)
                                (char->integer (ch-conv (string-ref s index))))
                           (default-bound))))))

(define string-hash
  (case-lambda
    ((s) (string-hash s (default-bound)))
    ((s bound)
     (%string-hash s (lambda (x) x) bound))))

(define string-ci-hash
  (case-lambda
    ((s) (string-ci-hash s (default-bound)))
    ((s bound)
     (%string-hash s char-downcase bound))))

;[esl] skint symbols are immediates
(define symbol-hash immediate-hash)
#;(define symbol-hash
  (case-lambda
    ((s) (symbol-hash s (default-bound)))
    ((s bound)
     (%string-hash (symbol->string s) (lambda (x) x) bound))))

;[esl] ported to skint by relying on immediate-hash for immediates
(define hash
  (case-lambda
    ((obj) (hash obj (default-bound)))
    ((obj bound)
     (cond ((fixnum? obj) (immediate-hash obj bound))
           ((string? obj) (string-hash obj bound))
           ((symbol? obj) (immediate-hash obj bound))
           ((flonum? obj) (immediate-hash obj bound))
           ((char? obj) (immediate-hash obj bound))
           ((vector? obj) (vector-hash obj bound))
           ((pair? obj) (fxmodulo (fx+ (hash (car obj)) (fx* 3 (hash (cdr obj)))) bound))
           ((procedure? obj) (error "hash: procedures cannot be hashed" obj))
           (else (immediate-hash obj bound))))))

(define hash-by-identity hash)

(define (vector-hash v bound)
  (let ((hashvalue 571) (len (vector-length v)))
    (do ((index 0 (fx+ index 1)))
      ((fx>=? index len) (fxmodulo hashvalue bound))
      (set! hashvalue (fxmodulo (fx+ (fx* 33 hashvalue) (hash (vector-ref v index)))
                                (default-bound))))))

(define %make-hash-node cons)
(define %hash-node-set-value! set-cdr!)
(define %hash-node-key car)
(define %hash-node-value cdr)

(define-record-type <srfi-hash-table>
  (%make-hash-table size hash compare associate entries)
  hash-table?
  (size hash-table-size hash-table-set-size!)
  (hash hash-table-hash-function)
  (compare hash-table-equivalence-function)
  (associate hash-table-association-function)
  (entries hash-table-entries hash-table-set-entries!))

(define default-table-size (make-parameter 64))

(define (appropriate-hash-function-for comparison)
  (or (and (eq? comparison eq?) hash-by-identity)
      (and (eq? comparison string=?) string-hash)
      (and (eq? comparison string-ci=?) string-ci-hash)
      hash))

(define make-hash-table
  (case-lambda
    (()
     (make-hash-table equal?
                      (appropriate-hash-function-for equal?)
                      (default-table-size)))
    ((comparison)
     (make-hash-table comparison
                      (appropriate-hash-function-for comparison)
                      (default-table-size)))
    ((comparison hash)
     (make-hash-table comparison
                      hash
                      (default-table-size)))
    ((comparison hash size)
     (let ((association (or (and (eq? comparison eq?) assq)
                            (and (eq? comparison eqv?) assv)
                            (and (eq? comparison equal?) assoc)
                            (rec (associate val alist)
                              (cond
                                ((null? alist) #f)
                                ((comparison val (caar alist)) (car alist))
                                (else (associate val (cdr alist))))))))
       (%make-hash-table
         0 hash comparison association (make-vector size '()))))))

(define (make-hash-table-maker comp hash)
  (lambda args (apply make-hash-table (cons comp (cons hash args)))))
(define make-symbol-hash-table
  (make-hash-table-maker eq? symbol-hash))
(define make-string-hash-table
  (make-hash-table-maker string=? string-hash))
(define make-string-ci-hash-table
  (make-hash-table-maker string-ci=? string-ci-hash))
(define make-integer-hash-table
  (make-hash-table-maker = modulo))

;[esl]+ support for override? optional flag to be forward-compatible with SRFI-125
(define (opt-override? rest)
  (if (pair? rest) (car rest) #t))

(define (%hash-table-hash hash-table key)
  (fxmodulo ; [esl] do not rely on hash functions accepting 2 args!
    ((hash-table-hash-function hash-table) key)
    (vector-length (hash-table-entries hash-table))))

(define (%hash-table-find entries associate hash key)
  (associate key (vector-ref entries hash)))

(define (%hash-table-add! entries hash key value)
  (vector-set! entries hash
               (cons (%make-hash-node key value)
                     (vector-ref entries hash))))

(define (%hash-table-delete! entries compare hash key)
  (let ((entrylist (vector-ref entries hash)))
    (cond ((null? entrylist) #f)
          ((compare key (caar entrylist))
           (vector-set! entries hash (cdr entrylist)) #t)
          (else
            (let loop ((current (cdr entrylist)) (previous entrylist))
              (cond ((null? current) #f)
                    ((compare key (caar current))
                     (set-cdr! previous (cdr current)) #t)
                    (else (loop (cdr current) current))))))))

(define (%hash-table-walk proc entries)
  (do ((index (- (vector-length entries) 1) (- index 1)))
    ((< index 0)) (for-each proc (vector-ref entries index))))

(define (%hash-table-maybe-resize! hash-table)
  (let* ((old-entries (hash-table-entries hash-table))
         (hash-length (vector-length old-entries)))
    (if (> (hash-table-size hash-table) hash-length)
      (let* ((new-length (* 2 hash-length))
             (new-entries (make-vector new-length '()))
             (hash (hash-table-hash-function hash-table)))
        (%hash-table-walk
          (lambda (node)
            (%hash-table-add! new-entries
               ; [esl] do not rely on hash functions accepting 2 args!
               (fxmodulo (hash (%hash-node-key node)) new-length)
               (%hash-node-key node) (%hash-node-value node)))
          old-entries)
        (hash-table-set-entries! hash-table new-entries)))))

(define (not-found-error key)
  (lambda ()
    (error "No value associated with key:" key)))

(define hash-table-ref
  (case-lambda
    ((hash-table key) (hash-table-ref hash-table key (not-found-error key)))
    ((hash-table key default-thunk)
     (cond ((%hash-table-find (hash-table-entries hash-table)
                              (hash-table-association-function hash-table)
                              (%hash-table-hash hash-table key) key)
            => %hash-node-value)
           (else (default-thunk))))
    ;[esl]+ support for success cont to be forward-compatible with SRFI-125
    ((hash-table key default-thunk success)
     (cond ((%hash-table-find (hash-table-entries hash-table)
                              (hash-table-association-function hash-table)
                              (%hash-table-hash hash-table key) key)
            => (lambda (n) (success (%hash-node-value n))))
           (else (default-thunk))))))

(define (hash-table-ref/default hash-table key default)
  (hash-table-ref hash-table key (lambda () default)))

;[esl]+ support for override? optional flag to be forward-compatible with SRFI-125
(define (hash-table-set! hash-table key value . rest)
  (define override? (opt-override? rest))
  (let ((hash (%hash-table-hash hash-table key))
        (entries (hash-table-entries hash-table)))
    (cond ((%hash-table-find entries
                             (hash-table-association-function hash-table)
                             hash key)
           => (lambda (node) (if override? (%hash-node-set-value! node value))))
          (else (%hash-table-add! entries hash key value)
                (hash-table-set-size! hash-table
                                       (+ 1 (hash-table-size hash-table)))
                (%hash-table-maybe-resize! hash-table)))))

(define hash-table-update!
  (case-lambda
    ((hash-table key function)
     (hash-table-update! hash-table key function (not-found-error key)))
    ;[esl]+ support for success cont to be forward-compatible with SRFI-125 
    ((hash-table key function default-thunk)
     (hash-table-update! hash-table key function default-thunk (lambda (x) x)))
    ((hash-table key function default-thunk success)
     (let ((hash (%hash-table-hash hash-table key))
           (entries (hash-table-entries hash-table)))
       (cond ((%hash-table-find entries
                                (hash-table-association-function hash-table)
                                hash key)
              => (lambda (node)
                   (%hash-node-set-value!
                    node (function (success (%hash-node-value node)))))) ;[esl+] wrap val in success call
             (else (%hash-table-add! entries hash key
                                     (function (default-thunk)))
                   (hash-table-set-size! hash-table
                                         (+ 1 (hash-table-size hash-table)))
                   (%hash-table-maybe-resize! hash-table)))))))

(define (hash-table-update!/default hash-table key function default)
  (hash-table-update! hash-table key function (lambda () default)))

(define (hash-table-delete! hash-table key)
  (if (%hash-table-delete! (hash-table-entries hash-table)
                           (hash-table-equivalence-function hash-table)
                           (%hash-table-hash hash-table key) key)
    (hash-table-set-size! hash-table (- (hash-table-size hash-table) 1))))

(define (hash-table-exists? hash-table key)
  (and (%hash-table-find (hash-table-entries hash-table)
                         (hash-table-association-function hash-table)
                         (%hash-table-hash hash-table key) key) #t))

(define (hash-table-walk hash-table proc)
  (%hash-table-walk
    (lambda (node) (proc (%hash-node-key node) (%hash-node-value node)))
    (hash-table-entries hash-table)))

(define (hash-table-fold hash-table f acc)
  (hash-table-walk hash-table 
    (lambda (key value) (set! acc (f key value acc))))
  acc)

(define (appropriate-size-for-alist alist)
  (max (default-table-size) (* 2 (length alist))))

(define alist->hash-table
  (case-lambda
    ((alist)
     (alist->hash-table alist
                        equal?
                        (appropriate-hash-function-for equal?)
                        (appropriate-size-for-alist alist)))
    ((alist comparison)
     (alist->hash-table alist
                        comparison
                        (appropriate-hash-function-for comparison)
                        (appropriate-size-for-alist alist)))
    ((alist comparison hash)
     (alist->hash-table alist
                        comparison
                        hash
                        (appropriate-size-for-alist alist)))
    ((alist comparison hash size)
     (let ((hash-table (make-hash-table comparison hash size)))
       (for-each
        (lambda (elem)
          (hash-table-update!/default
           hash-table (car elem) (lambda (x) x) (cdr elem)))
        alist)
       hash-table))))

(define (hash-table->alist hash-table)
  (hash-table-fold hash-table
                   (lambda (key val acc) (cons (cons key val) acc)) '()))

(define (hash-table-copy hash-table)
  (let ((new (make-hash-table (hash-table-equivalence-function hash-table)
                              (hash-table-hash-function hash-table)
                              (max (default-table-size)
                                   (* 2 (hash-table-size hash-table))))))
    (hash-table-walk hash-table
      (lambda (key value) (hash-table-set! new key value)))
    new))

;[esl]+ support for override? optional flag to be forward-compatible with SRFI-125
(define (hash-table-merge! hash-table1 hash-table2 . rest)
  (define override? (opt-override? rest))
  (hash-table-walk
    hash-table2
    (lambda (key value) (hash-table-set! hash-table1 key value override?)))
  hash-table1)

(define (hash-table-keys hash-table)
  (hash-table-fold hash-table (lambda (key val acc) (cons key acc)) '()))

(define (hash-table-values hash-table)
  (hash-table-fold hash-table (lambda (key val acc) (cons val acc)) '()))

))