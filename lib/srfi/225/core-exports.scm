(export
    ;; predicates
    dictionary?
    dict-empty?
    dict-contains?
    dict=?
    dict-pure?

    ;; lookup
    dict-ref
    dict-ref/default
    dict-comparator

    ;; mutation
    dict-set!
    dict-adjoin!
    dict-delete!
    dict-delete-all!
    dict-replace!
    dict-intern!
    dict-update!
    dict-update/default!
    dict-pop!
    dict-map
    dict-filter
    dict-remove
    dict-find-update!

    ;; whole dictionary
    dict-size
    dict-count
    dict-any
    dict-every
    dict-keys
    dict-values
    dict-entries
    dict-fold
    dict-map->list
    dict->alist

    ;; iteration
    dict-for-each
    dict->generator
    dict-set!-accumulator
    dict-adjoin!-accumulator

    ;; dictionary type descriptors
    dto?
    dto-ref

    ;; exceptions
    dictionary-error
    dictionary-error?
    dictionary-message
    dictionary-irritants)
