(define-syntax assume
  (syntax-rules ()
    ((assume expression message ...)
     (or expression
         (error "invalid assumption" (quote expression) (list message ...))))
    ((assume . _)
     (syntax-error "invalid assume syntax"))))
