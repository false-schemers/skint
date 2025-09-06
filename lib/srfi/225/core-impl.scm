;; procedure definitions that don't rely on concrete implementations

(define-record-type <dto>
  (make-dto-private procvec)
  dto?
  (procvec procvec))

(define-record-type <dto-err>
  (make-dictionary-error message irritants)
  dictionary-error?
  (message dictionary-message)
  (irritants dictionary-irritants))

;; shorthand access to dto procedure by index
(define-syntax dto-ref-stx
  (syntax-rules ()
    ((_ dto index)
     (begin
       (vector-ref (procvec dto) index)))))

;; shorthand to define proc with using proc index
(define-syntax define/dict-proc
  (syntax-rules ()
    ((_ proc index)
     (define (proc dto . args)
       (assume (dto? dto))
       (apply (dto-ref-stx dto index) dto args)))))

(define/dict-proc dictionary? dictionary?-id)
(define/dict-proc dict-empty? dict-empty?-id)
(define/dict-proc dict-contains? dict-contains?-id)
(define/dict-proc dict-pure? dict-pure?-id)
(define/dict-proc dict=? dict=?-id)

(define dict-ref
  (case-lambda
    ((dto dict key)
     (dict-ref dto dict key
               (lambda () (error "Key not found in dictionary" dict key))
               values))

    ((dto dict key failure)
     (dict-ref dto dict key failure values))

    ((dto dict key failure success)
     (assume (dto? dto))
     ((dto-ref-stx dto dict-ref-id) dto dict key failure success))))

(define/dict-proc dict-ref/default dict-ref/default-id)
(define/dict-proc dict-set! dict-set!-id)
(define/dict-proc dict-adjoin! dict-adjoin!-id)
(define/dict-proc dict-delete! dict-delete!-id)
(define/dict-proc dict-delete-all! dict-delete-all!-id)
(define/dict-proc dict-replace! dict-replace!-id)
(define/dict-proc dict-intern! dict-intern!-id)

(define dict-update!
  (case-lambda
    ((dto dict key updater)
     (dict-update! dto dict key updater
                  (lambda () (error "Key not found in dictionary" dict key))
                  values))

    ((dto dict key updater failure)
     (dict-update! dto dict key  updater failure values))

    ((dto dict key updater failure success)
     (assume (dto? dto))
     ((dto-ref-stx dto dict-update!-id) dto dict key updater failure success))))

(define/dict-proc dict-update/default! dict-update/default!-id)
(define/dict-proc dict-pop! dict-pop!-id)
(define/dict-proc dict-map dict-map-id)
(define/dict-proc dict-filter dict-filter-id)
(define/dict-proc dict-remove dict-remove-id)
(define/dict-proc dict-find-update! dict-find-update!-id)
(define/dict-proc dict-size dict-size-id)
(define/dict-proc dict-count dict-count-id)
(define/dict-proc dict-any dict-any-id)
(define/dict-proc dict-every dict-every-id)
(define/dict-proc dict-keys dict-keys-id)
(define/dict-proc dict-values dict-values-id)
(define/dict-proc dict-entries dict-entries-id)
(define/dict-proc dict-fold dict-fold-id)
(define/dict-proc dict-map->list dict-map->list-id)
(define/dict-proc dict->alist dict->alist-id)
(define/dict-proc dict-comparator dict-comparator-id)

(define dict-for-each
  (case-lambda
    ((dto proc dict) (dict-for-each dto proc dict #f #f))
    ((dto proc dict start) (dict-for-each dto proc dict start #f))
    ((dto proc dict start end) ((dto-ref-stx dto dict-for-each-id) dto proc dict start end))))

(define dict->generator
  (case-lambda
    ((dto dict) (dict->generator dto dict #f #f))
    ((dto dict start) (dict->generator dto dict start #f))
    ((dto dict start end) ((dto-ref-stx dto dict->generator-id) dto dict start end))))

(define/dict-proc dict-set!-accumulator dict-set!-accumulator-id)
(define/dict-proc dict-adjoin!-accumulator dict-adjoin!-accumulator-id)

(define (dto-ref dto procindex)
  (dto-ref-stx dto procindex))

(define (make-modified-dto dto lst)
  (define vec (vector-copy (procvec dto)))
  (do ((lst lst (cddr lst)))
      ((null? lst))
    (when (null? (cdr lst))
      (error "Uneven amount of arguments" lst))
    (let ((proc-id (car lst))
          (proc (cadr lst)))
      (unless (procedure? proc)
        (error "Not a procedure" proc))
      (vector-set! vec proc-id proc)))
  (make-dto-private vec))

(define-syntax dto-helper
  (syntax-rules ()
    ((_ (arg ...) (index proc) rest ...)
     (dto-helper (arg ... index proc) rest ...))
    ((_ (arg ...))
     (make-dto arg ...))))

(define-syntax dto
  (syntax-rules ()
    ((_ (index proc) ...)
     (dto-helper () (index proc) ...))))

(define (dictionary-error message . irritants)
  (make-dictionary-error message irritants))
