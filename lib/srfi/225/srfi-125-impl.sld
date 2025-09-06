(define-library
  (srfi 225 srfi-125-impl)
  (import (scheme base)
          (srfi 128)
          (prefix (srfi 125) t125-)
          (srfi 225 default-impl)
          (srfi 225 indexes))
  (export hash-table-dto)
  (begin

   (define (t125-hash-table-pure?* dto table)
     #f)

    (define (t125-hash-table-set* dto table . obj)
      (apply t125-hash-table-set! (cons table obj))
      table)

    (define (t125-hash-table-update* dto table key updater fail success)
      (t125-hash-table-update! table key updater fail success)
      table)

    (define (t125-hash-table-update/default* dto table key proc default)
      (t125-hash-table-update!/default table key proc default)
      table)

    (define (t125-hash-table-intern* dto table key failure)
      (values table (t125-hash-table-intern! table key failure)))

    (define (t125-hash-table-pop* dto table)
      (if (t125-hash-table-empty? table)
          (error "popped empty dictionary")
          (call-with-values (lambda () (t125-hash-table-pop! table))
                            (lambda (key value) (values table key value)))))

    (define (t125-hash-table-delete-all* dto table keys)
      (for-each
          (lambda (key)
            (t125-hash-table-delete! table key))
          keys)
      table)

    (define (t125-hash-table-map* dto proc table)
      (t125-hash-table-map! proc table))

    (define (t125-hash-table-filter* dto proc table)
      (t125-hash-table-prune!
          (lambda (key value)
            (not (proc key value)))
          table)
      table)

    (define (t125-hash-table-remove* dto proc table)
      (t125-hash-table-prune! proc table)
      table)

    (define (t125-hash-table-find-update* dto table key fail success)
      ;; instead of running immediately,
      ;; add an indirection through thunk
      ;; to guarantee call in tail position
      (define (make-success-thunk value)
        (define (update new-key new-value)
          (unless (eq? new-key key)
            (t125-hash-table-delete! table key))
          (t125-hash-table-set! table new-key new-value)
          table)
        (define (remove)
          (t125-hash-table-delete! table key)
          table)
        (lambda ()
          (success key value update remove) ))
      (define (make-failure-thunk)
        (define (ignore)
          table)
        (define (insert value)
          (t125-hash-table-set! table key value)
          table)
        (lambda ()
          (fail insert ignore)))
      (define thunk (t125-hash-table-ref table key make-failure-thunk make-success-thunk))
      (thunk))

    (define (t125-hash-table-comparator* dto table)
      (make-comparator (lambda args #t)
                       (t125-hash-table-equivalence-function table)
                       #f
                       (t125-hash-table-hash-function table)))

    (define (t125-hash-table-size* dto table)
      (t125-hash-table-size table))

    (define (t125-hash-table-keys* dto table)
      (t125-hash-table-keys table))

    (define (t125-hash-table-values* dto table)
      (t125-hash-table-values table))

    (define (t125-hash-table-entries* dto table)
      (t125-hash-table-entries table))

    (define (t125-hash-table-fold* dto proc knil table)
      (t125-hash-table-fold proc knil table))

    (define (t125-hash-table-map->list* dto proc table)
      (t125-hash-table-map->list proc table))

    (define (t125-hash-table->alist* dto table)
      (t125-hash-table->alist table))

    (define (t125-hash-table?* dto table)
      (t125-hash-table? table))

    (define (t125-hash-table-empty?* dto table)
      (t125-hash-table-empty? table))

    (define (t125-hash-table-contains?* dto table key)
      (t125-hash-table-contains? table key))

    (define (t125-hash-table-ref* dto table key failure success)
      (t125-hash-table-ref table key failure success))

    (define (t125-hash-table-ref/default* dto table key default)
      (t125-hash-table-ref/default table key default))

    (define hash-table-dto
      (make-dto
        dictionary?-id t125-hash-table?*
        dict-pure?-id t125-hash-table-pure?*
        dict-empty?-id t125-hash-table-empty?*
        dict-contains?-id t125-hash-table-contains?*
        dict-ref-id t125-hash-table-ref*
        dict-ref/default-id t125-hash-table-ref/default*
        dict-set!-id t125-hash-table-set*
        dict-delete-all!-id t125-hash-table-delete-all*
        dict-intern!-id t125-hash-table-intern*
        dict-update!-id t125-hash-table-update*
        dict-update/default!-id t125-hash-table-update/default*
        dict-pop!-id t125-hash-table-pop*
        dict-map-id t125-hash-table-map*
        dict-filter-id t125-hash-table-filter*
        dict-remove-id t125-hash-table-remove*
        dict-find-update!-id t125-hash-table-find-update*
        dict-size-id t125-hash-table-size*
        dict-keys-id t125-hash-table-keys*
        dict-values-id t125-hash-table-values*
        dict-entries-id t125-hash-table-entries*
        dict-fold-id t125-hash-table-fold*
        dict-map->list-id t125-hash-table-map->list*
        dict->alist-id t125-hash-table->alist*
        dict-comparator-id t125-hash-table-comparator*))))
