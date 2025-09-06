(define-library
  (srfi 225 srfi-126-impl)
  (import (scheme base)
          (prefix (srfi 126) t126-)
          (srfi 225 default-impl)
          (srfi 225 indexes))
  (export srfi-126-dto)
  (begin
    (define (prep-dto-arg proc)
      (lambda (dto . args)
        (apply proc args)))

    (define (t126-hashtable-pure?* dto table)
      #f)

    (define (t126-hashtable-ref* dto table key fail success)
      (define-values (value found?) (t126-hashtable-lookup table key))
      (if found?
        (success value)
        (fail)))

    (define (t126-hashtable-ref/default* dto table key default)
      (t126-hashtable-ref table key default))

    (define (t126-hashtable-set* dto table . obj)
      (let loop ((obj obj))
        (if (null? obj)
          #t
          (begin
            (t126-hashtable-set! table (car obj) (cadr obj))
            (loop (cddr obj)))))
      table)

    (define (t126-hashtable-delete-all* dto table keys)
      (for-each
        (lambda (key)
          (t126-hashtable-delete! table key))
        keys)
      table)

    (define (t126-hashtable-intern* dto table key default)
      (values table (t126-hashtable-intern! table key default)))

    (define (t126-hashtable-update/default* dto table key updater default)
      (t126-hashtable-update! table key updater default)
      table)

    (define (t126-hashtable-pop* dto table)
      (if (t126-hashtable-empty? table)
        (error "popped empty dictionary")
        (call-with-values (lambda () (t126-hashtable-pop! table))
                          (lambda (key value) (values table key value)))))

    (define (t126-hashtable-update-all* dto proc table)
      (t126-hashtable-update-all! table proc)
      table)

    (define (t126-hashtable-filter* dto proc table)
      (t126-hashtable-prune! table
                             (lambda (key value)
                               (not (proc key value))))
      table)

    (define (t126-hashtable-remove* dto proc table)
      (t126-hashtable-prune! table proc)
      table)

    (define (t126-hashtable-find-update* dto table key fail success)
      (define (handle-success value)
        (define (update new-key new-value)
          (unless (eq? new-key key)
            (t126-hashtable-delete! table key))
          (t126-hashtable-set! table new-key new-value)
          table)
        (define (remove)
          (t126-hashtable-delete! table key)
          table)
        (success key value update remove))
      (define (handle-fail)
        (define (ignore)
          table)
        (define (insert value)
          (t126-hashtable-set! table key value)
          table)
        (fail insert ignore))

      (define default (cons #f #f))
      (define found (t126-hashtable-ref table key default))
      (if (eq? default found)
        (handle-fail)
        (handle-success found)))

    (define (t126-hashtable-map->lset* dto proc table)
      (t126-hashtable-map->lset table proc))

    (define (t126-hashtable-keys* dto table)
      (vector->list (t126-hashtable-keys table)))

    (define (t126-hashtable-values* dto table)
      (vector->list (t126-hashtable-values table)))

    (define (t126-hashtable-entries* dto table)
      (call-with-values
        (lambda () (t126-hashtable-entries table))
        (lambda (keys vals)
          (values
            (vector->list keys)
            (vector->list vals)))))

    (define (t126-hashtable-comparator* dto table)
      #f)

    (define srfi-126-dto
      (make-dto
        dictionary?-id (prep-dto-arg t126-hashtable?)
        dict-pure?-id t126-hashtable-pure?*
        dict-empty?-id (prep-dto-arg t126-hashtable-empty?)
        dict-contains?-id (prep-dto-arg t126-hashtable-contains?)
        dict-ref-id t126-hashtable-ref*
        dict-ref/default-id t126-hashtable-ref/default*
        dict-set!-id t126-hashtable-set*
        dict-delete-all!-id t126-hashtable-delete-all*
        dict-intern!-id t126-hashtable-intern*
        dict-update/default!-id t126-hashtable-update/default*
        dict-pop!-id t126-hashtable-pop*
        dict-map-id t126-hashtable-update-all*
        dict-filter-id t126-hashtable-filter*
        dict-remove-id t126-hashtable-remove*
        dict-find-update!-id t126-hashtable-find-update*
        dict-size-id (prep-dto-arg t126-hashtable-size)
        dict-keys-id t126-hashtable-keys*
        dict-values-id t126-hashtable-values*
        dict-entries-id t126-hashtable-entries*
        dict-map->list-id t126-hashtable-map->lset*
        dict-comparator-id t126-hashtable-comparator*))))
