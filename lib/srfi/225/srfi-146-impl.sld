(define-library
  (srfi 225 srfi-146-impl)
  (import (scheme base)
          (srfi 146)
          (srfi 225 core)
          (srfi 225 default-impl)
          (srfi 225 indexes))
  (export mapping-dto)
  (begin

    (define (prep-dto-arg proc)
      (lambda (dto . args)
        (apply proc args)))

    (define (mapping-map* dto proc dict)
      (mapping-map (lambda (key value)
                     (values key (proc key value)))
                   (dict-comparator dto dict) dict))

    (define (mapping-find-update* dto dict key failure success)
      (call/cc
        ;; escape from whole hashmap-search entirely, when success / failure
        ;; return something other than through passed in continuation procedures
        (lambda (k)
          (define-values
            (new-dict ignored)
            (mapping-search dict key
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

    (define mapping-dto
      (make-dto
        dictionary?-id (prep-dto-arg mapping?)
        dict-pure?-id (lambda _ #t)
        dict-map-id mapping-map*
        dict-empty?-id (prep-dto-arg mapping-empty?)
        dict-contains?-id (prep-dto-arg mapping-contains?)
        dict-ref-id (prep-dto-arg mapping-ref)
        dict-ref/default-id (prep-dto-arg mapping-ref/default)
        dict-set!-id (prep-dto-arg mapping-set)
        dict-adjoin!-id (prep-dto-arg mapping-adjoin)
        dict-delete!-id (prep-dto-arg mapping-delete)
        dict-delete-all!-id (prep-dto-arg mapping-delete-all)
        dict-replace!-id (prep-dto-arg mapping-replace)
        dict-intern!-id (prep-dto-arg mapping-intern)
        dict-update!-id (prep-dto-arg mapping-update)
        dict-update/default!-id (prep-dto-arg mapping-update/default)
        dict-pop!-id (prep-dto-arg mapping-pop)
        dict-filter-id (prep-dto-arg mapping-filter)
        dict-remove-id (prep-dto-arg mapping-remove)
        dict-find-update!-id mapping-find-update*
        dict-size-id (prep-dto-arg mapping-size)
        dict-count-id (prep-dto-arg mapping-count)
        dict-keys-id (prep-dto-arg mapping-keys)
        dict-values-id (prep-dto-arg mapping-values)
        dict-entries-id (prep-dto-arg mapping-entries)
        dict-fold-id (prep-dto-arg mapping-fold)
        dict-map->list-id (prep-dto-arg mapping-map->list)
        dict->alist-id (prep-dto-arg mapping->alist)
        dict-comparator-id (prep-dto-arg mapping-key-comparator)))))
