;;; SPDX-FileCopyrightText: 2024 Sergei Egorov
;;; SPDX-License-Identifier: MIT

(define-library (srfi 257 misc)
  (import (scheme base) (srfi 257))
  (export 
    ~etc+ ~etc= ~etc**
    cm-match ->
    sr-match)

(begin

(define-match-pattern ~etc+ ()
  ((~etc+ p) (~pair? (~etc p))))

(define-match-pattern ~etc= ()
  ((~etc= k p)
   (~and (~list? (~prop length => k))
         (~etc p))))

(define-match-pattern ~etc** ()
  ((~etc** k j p)
   (~and (~list? (~prop length => (~and (~test >= (k)) (~test <= (j)))))
         (~etc p))))

(define-match-pattern ~cmp->p (<...> <_> unquote ->)
  ((_ l ,(x)) (~prop l => x))
  ((_ l ,(f -> x ...)) (~prop f => x ...))
  ((_ l ,(x ...)) (~prop l => x ...))
  ((_ l ,<_>) _)
  ((_ l ,x) x)
  ((_ l ()) '())
  ((_ l (x <...>)) (~etc (~cmp->p l x))) ; optimization
  ((_ l (x <...> . y)) (~append/t y (~etc (~cmp->p l x)) (~cmp->p l y)))
  ((_ l (x . y)) (~cons (~cmp->p l x) (~cmp->p l y)))
  ((_ l #(x ...)) (~list->vector (~cmp->p l (x ...))))
  ((_ l other) 'other))

(define-syntax cm-match
  (syntax-rules (guard)
    ((_ () l x () (c ...))
     (match x c ... (_ (error "cm-match error" x))))
    ((_ () l x ((cmp (guard . e*) . b) cmc ...) (c ...))
     (cm-match () l x (cmc ...)
       (c ... ((~replace-specials <...> <_> (~cmp->p l cmp))
                 (=> n) (if (and . e*) (let () . b) (n))))))
    ((_ () l x ((cmp . b) cmc ...) (c ...))
     (cm-match () l x (cmc ...)
       (c ... ((~replace-specials <...> <_> (~cmp->p l cmp))
                 (let () . b)))))
    ((_ x cmc ...)
     (let l ((v x)) (cm-match () l v (cmc ...) ())))))

(define-match-pattern ~srp->p (<...> <_>)
  ((_ l* ()) '())
  ((_ l* (x <...>)) (~etc (~srp->p l* x))) ; optimization
  ((_ l* (x <...> . y)) (~append/t y (~etc (~srp->p l* x)) (~srp->p l* y)))
  ((_ l* (x . y)) (~cons (~srp->p l* x) (~srp->p l* y)))
  ((_ l* #(x ...)) (~list->vector (~srp->p l* (x ...))))
  ((_ l* <_>) _)
  ((_ l* a) (~if-id-member a l* 'a a)))

(define-syntax sr-match
  (syntax-rules (=>)
    ((_ () l* v () (c ...))
     (match v c ... (_ (error "sr-match error" v))))
    ((_ () l* v ((srp . b) src ...) (c ...))
     (sr-match () l* v (src ...) (c ...
       ((~replace-specials <...> <_> (~srp->p l* srp)) . b))))
    ((_ x (l ...) src ...)
     (let ((v x)) (sr-match () (l ...) v (src ...) ())))))

))
