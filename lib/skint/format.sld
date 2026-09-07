
(define-library (skint format)

  (import
    (only (skint)
      format printf fprintf
      format-pretty-print
      format-fixed-print
      format-exponential-print
      format-general-print
      format-fresh-line
      format-help-string))

  (export
    format printf fprintf
    ;; parameters controlling the ~y ~e ~f ~g ~& ~h directives
    format-pretty-print
    format-fixed-print
    format-exponential-print
    format-general-print
    format-fresh-line
    format-help-string))
