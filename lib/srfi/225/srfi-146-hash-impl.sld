(define-library
  (srfi 225 srfi-146-hash-impl)
  (import (scheme base)
          (srfi 146 hash)
          (srfi 225 core)
          (srfi 225 default-impl)
          (srfi 225 indexes))
  (export hash-mapping-dto)
  (begin

    (define (prep-dto-arg proc)
      (lambda (dto . args)
        (apply proc args)))

    (define (hashmap-map* dto proc dict)
      (hashmap-map (lambda (key value)
                     (values key (proc key value)))
                   (dict-comparator dto dict) dict))

    (define (hashmap-find-update* dto dict key failure success)
      (call/cc
        ;; escape from whole hashmap-search entirely, when success / failure
        ;; return something other than through passed in continuation procedures
        (lambda (k)
          (define-values
            (new-dict ignored)
            (hashmap-search dict key
                            (lambda (insert ignore)
                              ;; handle when continuation procedure is called
                              ;; and force it into tail call
                              (call/cc (lambda (k2)
                                         (define result
                                           ;; calls to insert / ignore / update / remove
                                           ;; can return unspecified amount of values,
                                           ;; hence call-with-values approach
                                           (failure (lambda (value) (call-with-values (lambda () (insert value #f)) k2))
                                                    (lambda () (call-with-values (lambda () (ignore #f)) k2))))
                                         ;; neither insert nor ignore called -- return result to top level escape
                                         (k result))))
                            (lambda (key value update remove)
                              (call/cc (lambda (k2)
                                         (define result
                                           (success
                                             key
                                             value
                                             (lambda (new-key new-value) (call-with-values (lambda () (update new-key new-value #f)) k2))
                                             (lambda () (call-with-values (lambda () (remove #f)) k2))))
                                         (k result))))))
          new-dict)))
    (define hash-mapping-dto
      (make-dto
        dictionary?-id (prep-dto-arg hashmap?)
        dict-pure?-id (lambda _ #t)
        dict-map-id hashmap-map*
        dict-empty?-id (prep-dto-arg hashmap-empty?)
        dict-contains?-id (prep-dto-arg hashmap-contains?)
        dict-ref-id (prep-dto-arg hashmap-ref)
        dict-ref/default-id (prep-dto-arg hashmap-ref/default)
        dict-set!-id (prep-dto-arg hashmap-set)
        dict-adjoin!-id (prep-dto-arg hashmap-adjoin)
        dict-delete!-id (prep-dto-arg hashmap-delete)
        dict-delete-all!-id (prep-dto-arg hashmap-delete-all)
        dict-replace!-id (prep-dto-arg hashmap-replace)
        dict-intern!-id (prep-dto-arg hashmap-intern)
        dict-update!-id (prep-dto-arg hashmap-update)
        dict-update/default!-id (prep-dto-arg hashmap-update/default)
        dict-pop!-id (prep-dto-arg hashmap-pop)
        dict-filter-id (prep-dto-arg hashmap-filter)
        dict-remove-id (prep-dto-arg hashmap-remove)
        dict-find-update!-id hashmap-find-update*
        dict-size-id (prep-dto-arg hashmap-size)
        dict-count-id (prep-dto-arg hashmap-count)
        dict-keys-id (prep-dto-arg hashmap-keys)
        dict-values-id (prep-dto-arg hashmap-values)
        dict-entries-id (prep-dto-arg hashmap-entries)
        dict-fold-id (prep-dto-arg hashmap-fold)
        dict-map->list-id (prep-dto-arg hashmap-map->list)
        dict->alist-id (prep-dto-arg hashmap->alist)
        dict-comparator-id (prep-dto-arg hashmap-key-comparator)))))
