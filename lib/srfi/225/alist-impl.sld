(define-library
  (srfi 225 alist-impl)
  (import (scheme base)
          (srfi 1)
          (srfi 128)
          (srfi 225 core)
          (srfi 225 default-impl)
          (srfi 225 indexes))
  (export
    make-alist-dto
    eqv-alist-dto
    equal-alist-dto)

  (begin

    (define (alist? dto l)
      (and (list? l)
           (or (null? l)
               (pair? (car l)))))

    (define (alist-pure? dto alist)
      #t)

    (define (alist-map dto proc alist)
      (map
        (lambda (e)
          (define key (car e))
          (define value (cdr e))
          (cons key (proc key value)))
        alist))

    (define (alist-filter dto pred alist)
      (filter
        (lambda (e)
          (pred (car e) (cdr e)))
        alist))

    (define (make-alist-delete key=)
      (lambda (dto key alist)
        (filter
          (lambda (entry)
            (not (key= (car entry) key)))
          alist)))

    (define (make-alist-find-update key=)
      (define alist-delete (make-alist-delete key=))
      (lambda (dto alist key failure success)
        (define (handle-success pair)
          (define old-key (car pair))
          (define old-value (cdr pair))
          (define (update new-key new-value)
            (cond
              ((and (eq? old-key
                         new-key)
                    (eq? old-value
                         new-value))
               alist)
              (else
                (let ((new-list
                        (alist-cons
                          new-key new-value
                          (alist-delete dto old-key alist))))
                  new-list))))
          (define (remove)
            (alist-delete dto old-key alist))
          (success old-key old-value update remove))

        (define (handle-failure)
          (define (insert value)
            (alist-cons key value alist))
          (define (ignore)
            alist)
          (failure insert ignore))
        (cond
          ((assoc key alist key=) => handle-success)
          (else (handle-failure)))))


    (define (alist-size dto alist)
      (length alist))

    (define (alist->alist dto alist)
      alist)

    (define (alist-comparator dto dictionary)
      #f)

    (define (make-alist-dto key=)
      (make-dto
        dictionary?-id alist?
        dict-pure?-id alist-pure?
        dict-map-id alist-map
        dict-filter-id alist-filter
        dict-find-update!-id (make-alist-find-update key=)
        dict-size-id alist-size
        dict->alist-id alist->alist
        dict-comparator-id alist-comparator))

    (define eqv-alist-dto (make-alist-dto eqv?))
    (define equal-alist-dto (make-alist-dto equal?))))
