(import (srfi 62))
(include "test.scm")

(test-begin "srfi-62")

(test #t (read-error? (guard (exn (else exn)) (read (open-input-string " #;")))))

(test '(1 2 3) (read (open-input-string "#;foo (1 #;bar 2 3 #;baz)")))

(test '(1 2 3) (read (open-input-string "#;(f o o) (1 #;(bar #;baz foobar) 2 3 #;#(quux))")))

(test-end)

