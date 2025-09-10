(import (srfi 30))
(include "test.scm")

(test-begin "srfi-30")

(test #t (read-error? (guard (exn (else exn)) (read (open-input-string " #| foo |# |# bar")))))

(test '(1 2 3) (read (open-input-string "#| foo |#  (1 #| bar |#  2 3 #| baz |#)")))

(test '(1 2 3) (read (open-input-string "#| foo |#  (1 #|bar #| baz |##| #### |||||| foo |#|#  2 3 #| baz |#)")))

(test-end)

