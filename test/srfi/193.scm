(import (srfi 152) (srfi 193))
(include "test.scm")

(test-begin "srfi-193")

(test-assert (let ([cl (command-line)]) (and (pair? cl) (null? (cdr cl)) (string-suffix? "193.scm" (car cl)))))
(test-equal "193" (command-name))
(test-equal '() (command-args))
(test-assert (string-suffix? "193.scm" (script-file)))
(test-assert (string-prefix? (script-directory) (script-file)))
(test-equal (script-file) (string-append (script-directory) "193.scm"))

(test-end)

