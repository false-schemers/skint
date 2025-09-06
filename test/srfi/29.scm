(import (srfi 29))
(include "test.scm")

(test-begin "srfi-29")

; [esl] adapted from ref. imp. by Scott G. Miller
; (C) 2002 Scott G. Miller

(let ((translations
       '(((en) . ((time . "Its ~a, ~a.")
                (goodbye . "Goodbye, ~a.")))
         ((fr) . ((time . "~1@*~a, c'est ~0@*~a.") ; [esl*] Skint's ~N@* implemented in CL-like form!
                (goodbye . "Au revoir, ~a."))))))
  (for-each (lambda (translation)
              (let ((bundle-name (cons 'hello-program (car translation))))
                (if (not (load-bundle! bundle-name))
                    (begin
                     (declare-bundle! bundle-name (cdr translation))
                     (store-bundle! bundle-name)))))
             translations))

(define localized-message
  (lambda (message-name . args)
    (apply format (cons (localized-template 'hello-program
                                            message-name)
                        args))))

(define (print-it port)
  (let ((myname "Fred"))
    (display (localized-message 'time "12:00" myname) port)
    (display #\newline port)
    (display (localized-message 'goodbye myname) port)
    (display #\newline port)))

(test-equal "Its 12:00, Fred.\nGoodbye, Fred.\nFred, c'est 12:00.\nAu revoir, Fred.\n"
  (let ([port (open-output-string)])
    (parameterize ([current-language 'en])
      (print-it port))
    (parameterize ([current-language 'fr])
      (print-it port))
    (get-output-string port)))

(test-end)

