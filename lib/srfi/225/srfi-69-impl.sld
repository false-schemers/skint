(define-library
  (srfi 225 srfi-69-impl)
  (import (scheme base)
          (srfi 128)
          (prefix (srfi 69) t69-)
          (srfi 225 default-impl)
          (srfi 225 indexes))
  (export srfi-69-dto)
  (begin
    (define (prep-dto-arg proc)
      (lambda (dto . args)
        (apply proc args)))

    (define (t69-hash-table-pure?* dto table)
      #f)

    (define (t69-hash-table-ref* dto table key fail success)
      (define default (cons #f #f))
      (define found (t69-hash-table-ref/default table key default))
      (if (eq? found default)
        (fail)
        (success found)))

    (define (t69-hash-table-set!* dto table . obj)
      (let loop ((obj obj))
        (if (null? obj)
          table
          (begin
            (t69-hash-table-set! table (car obj) (cadr obj))
            (loop (cddr obj)))))
      table)

    (define (t69-hash-table-update!/default* dto table key proc default)
      (t69-hash-table-update!/default table key proc default)
      table)

    (define (t69-hash-table-delete-all!* dto table keys)
      (for-each
        (lambda (key)
          (t69-hash-table-delete! table key))
        keys)
      table)

    (define (t69-hash-table-map!* dto proc table)
      (t69-hash-table-walk table (lambda (key value)
                                   (t69-hash-table-set! table key (proc key value))))
      table)

    (define (t69-hash-table-filter!* dto proc table)
      (t69-hash-table-walk table
                           (lambda (key value)
                             (unless (proc key value)
                               (t69-hash-table-delete! table key))))
      table)

    (define (t69-hash-table-fold* dto proc knil table)
      (t69-hash-table-fold table proc knil))

    (define (t69-hash-table-find-update!* dto table key fail success)
      (define (handle-success value)
        (define (update new-key new-value)
          (unless (eq? new-key key)
            (t69-hash-table-delete! table key))
          (t69-hash-table-set! table new-key new-value)
          table)
        (define (remove)
          (t69-hash-table-delete! table key)
          table)
        (success key value update remove))
      (define (handle-fail)
        (define (ignore)
          table)
        (define (insert value)
          (t69-hash-table-set! table key value)
          table)
        (fail insert ignore))

      (define default (cons #f #f))
      (define found (t69-hash-table-ref/default table key default))
      (if (eq? default found)
        (handle-fail)
        (handle-success found)))

    (define (t69-hash-table-comparator* dto table)
      (make-comparator (lambda args #t)
                       (or (t69-hash-table-equivalence-function table)
                           equal?)
                       #f
                       (t69-hash-table-hash-function table)))

    (define srfi-69-dto
      (make-dto
        dictionary?-id (prep-dto-arg t69-hash-table?)
        dict-pure?-id t69-hash-table-pure?*
        dict-ref-id t69-hash-table-ref*
        dict-ref/default-id (prep-dto-arg t69-hash-table-ref/default)
        dict-set!-id t69-hash-table-set!*
        dict-delete-all!-id t69-hash-table-delete-all!*
        dict-contains?-id (prep-dto-arg t69-hash-table-exists?)
        dict-update/default!-id t69-hash-table-update!/default*
        dict-size-id (prep-dto-arg t69-hash-table-size)
        dict-keys-id (prep-dto-arg t69-hash-table-keys)
        dict-values-id (prep-dto-arg t69-hash-table-values)
        dict-map-id t69-hash-table-map!*
        dict-filter-id t69-hash-table-filter!*
        dict-fold-id t69-hash-table-fold*
        dict->alist-id (prep-dto-arg t69-hash-table->alist)
        dict-find-update!-id t69-hash-table-find-update!*
        dict-comparator-id t69-hash-table-comparator*))))
