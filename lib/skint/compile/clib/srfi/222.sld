(define-library (srfi 222)
  (import (scheme base))
  (export
   make-compound
   compound?
   compound-subobjects
   compound-length
   compound-ref
   compound-map
   compound-map->list
   compound-filter
   compound-predicate
   compound-access)

;[esl] adapted from ref. imp. by Arvydas Silanskas 

(begin

(define-record-type <compound-object>
    (raw-compound-object subobjs)
    compound?
  (subobjs raw-object-subobjects))

;; private
;; flatten list of objects and potentially other compounds
;; into simple list of objects without compounds
(define (assemble-subobjects in)
  (let loop ((in in)
             (out '()))
    (if (null? in)
	(reverse out)
	(loop (cdr in)
              (if (compound? (car in))
                  (append (reverse (compound-subobjects (car in))) out)
                  (cons (car in) out))))))

(define (make-compound . subobjs)
  (raw-compound-object (assemble-subobjects subobjs)))

(define (compound-subobjects obj)
  (if (compound? obj)
      (raw-object-subobjects obj)
      (list obj)))

(define (compound-length obj)
  (if (compound? obj)
      (length (raw-object-subobjects obj))
      1))

(define (compound-ref obj k)
  (if (compound? obj)
      (list-ref (compound-subobjects obj) k)
      obj))

(define (compound-map mapper obj)
  (if (compound? obj)
      (apply make-compound (compound-map->list mapper obj))
      (make-compound (mapper obj))))

(define (compound-map->list mapper obj)
  (map mapper (compound-subobjects obj)))

(define (filter pred list)
  (let loop ((list list) (result '()))
    (cond
     ((null? list)
      (reverse result))
     ((pred (car list))
      (loop (cdr list) (cons (car list) result)))
     (else
      (loop (cdr list) result)))))

(define (compound-filter pred obj)
  (define subobjs (filter pred (compound-subobjects obj)))
  (raw-compound-object subobjs))

(define (compound-predicate pred obj)
  (and
   (or
    ;; compound itself satisfies pred
    (pred obj)

    ;; compound has subobj that satisfies pred
    (let loop ((subobjs (compound-subobjects obj)))
      (cond
       ((null? subobjs) #f)
       ((pred (car subobjs)) #t)
       (else (loop (cdr subobjs))))))

   ;; if matched pred, convert result to #t
   #t))

(define (compound-access pred accessor default obj)
  (cond
   ((pred obj)
    (accessor obj))
   ((compound? obj)
    (let loop ((subobjs (compound-subobjects obj)))
      (cond
       ((null? subobjs) default)
       ((pred (car subobjs)) (accessor (car subobjs)))
       (else (loop (cdr subobjs))))))
   (else default)))

))
