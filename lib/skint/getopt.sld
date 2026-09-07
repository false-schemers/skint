
(define-library (skint getopt)

  (import
    (only (skint hidden)
      get-next-command-line-option
      print-command-line-options))

  (export
    get-next-command-line-option
    print-command-line-options))
