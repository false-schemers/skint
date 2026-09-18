(import (srfi 192))
(import (scheme base) (scheme file))
(include "test.scm")

(test-begin "srfi-192")

;; a position past what a fixnum holds, written as a flonum so that the reader
;; accepts it in a build with no numeric tower
(define past-fixnum 600000000.0)

;; the tower is what decides whether a position that large comes back exact
(define tower? (and (memq 'full-numeric-tower (features)) #t))

(define (file-error-raised? thunk)
  (guard (e (#t (file-error? e))) (thunk) #f))

;; ---------------------------------------------------------------- which ports

(define (caps port) (list (port-has-port-position? port)
                          (port-has-set-port-position!? port)))

(test '(#t #t) (caps (open-input-bytevector (bytevector 1 2 3))))
(test '(#t #t) (caps (open-output-bytevector)))
(test '(#f #f) (caps (open-input-string "abc")))
(test '(#f #f) (caps (open-output-string)))

;; ------------------------------------------------------- bytevector input port

(define bi (open-input-bytevector (bytevector 0 1 2 3 4 5 6 7 8 9)))

(test 0 (port-position bi))
(test-assert (exact? (port-position bi)))
(read-u8 bi)
(read-u8 bi)
(test 2 (port-position bi))
(set-port-position! bi 7)
(test 7 (port-position bi))
(test 7 (read-u8 bi))
(test 8 (port-position bi))
(set-port-position! bi 0)
(test 0 (read-u8 bi))

;; the end of the data is a valid position; one past it is not
(set-port-position! bi 10)
(test 10 (port-position bi))
(test-assert (eof-object? (read-u8 bi)))
(test-assert (file-error-raised? (lambda () (set-port-position! bi 11))))
(test 10 (port-position bi))
(test-assert (file-error-raised? (lambda () (set-port-position! bi -1))))

;; peeking pushes a byte back, and the position has to follow
(set-port-position! bi 4)
(test 4 (peek-u8 bi))
(test 4 (port-position bi))

;; ------------------------------------------------------ bytevector output port

(define bo (open-output-bytevector))

(test 0 (port-position bo))
(write-u8 65 bo)
(write-u8 66 bo)
(write-u8 67 bo)
(test 3 (port-position bo))

;; seeking back and overwriting must not cut off what follows
(set-port-position! bo 1)
(test 1 (port-position bo))
(write-u8 90 bo)
(test 2 (port-position bo))
(test (bytevector 65 90 67) (get-output-bytevector bo))

;; and the end is still where it was
(set-port-position! bo 3)
(test 3 (port-position bo))
(write-u8 68 bo)
(test (bytevector 65 90 67 68) (get-output-bytevector bo))
(test-assert (file-error-raised? (lambda () (set-port-position! bo 5))))

;; ------------------------------------------------------------- binary file port

(define fname "t192.tmp")
(when (file-exists? fname) (delete-file fname))

(define fo (open-binary-output-file fname))
(test '(#t #t) (caps fo))
(write-u8 1 fo)
(write-u8 2 fo)
(write-u8 3 fo)
(test 3 (port-position fo))
(set-port-position! fo 1)
(write-u8 99 fo)
(test 2 (port-position fo))

;; a file can be positioned past its end; nothing is written, so it does not grow
(set-port-position! fo past-fixnum)
(test-assert (= past-fixnum (port-position fo)))
(test-equal tower? (exact? (port-position fo)))
(test-assert (integer? (port-position fo)))
;; whatever form it came back in, it is accepted going the other way
(set-port-position! fo (port-position fo))
(test-assert (= past-fixnum (port-position fo)))

;; Past 2^53 only an exact integer can still say where the port is, so this is
;; where the two configurations part company: the tower answers, and a build
;; without one has to decline rather than round.  Whether the port can be put
;; there at all is the platform's to say -- a file system whose largest file is
;; smaller than that refuses the seek, and is entitled to -- so the answer is
;; only worth checking once the seek has been allowed.
(define past-doubles 1e16)
(define seeked-far?
  (guard (e (#t #f)) (set-port-position! fo past-doubles) #t))
(cond ((not seeked-far?))
      (tower?
       (test-assert (= past-doubles (port-position fo)))
       (test-assert (exact? (port-position fo))))
      (else
       (test-assert (file-error-raised? (lambda () (port-position fo))))))
;; either way the port is still usable, and still where it was last put
(set-port-position! fo 2)
(test 2 (port-position fo))
(close-port fo)

(define fi (open-binary-input-file fname))
(test '(#t #t) (caps fi))
(test 0 (port-position fi))
(test 1 (read-u8 fi))
(test 99 (read-u8 fi))
(test 3 (read-u8 fi))
(test 3 (port-position fi))
(test-assert (eof-object? (read-u8 fi)))
(set-port-position! fi 2)
(test 3 (read-u8 fi))
;; peeking on a file port pushes back too
(set-port-position! fi 0)
(test 1 (peek-u8 fi))
(test 0 (port-position fi))
(close-port fi)
(delete-file fname)

;; ------------------------------------------------------------ position numbers

(define b2 (open-input-bytevector (bytevector 0 1 2 3 4)))

;; any integer form is a position, exact or not
(set-port-position! b2 3)
(test 3 (port-position b2))
(set-port-position! b2 1.0)
(test 1 (port-position b2))
(test-assert (exact? (port-position b2)))

;; a number with a fractional part is not a position at all
(test-error (set-port-position! b2 1.5))
(test-error (set-port-position! b2 'x))
(test 1 (port-position b2))

;; ------------------------------------------------------------------ closed port

(define bc (open-input-bytevector (bytevector 1 2)))
(close-port bc)
(test '(#f #f) (caps bc))
(test-assert (file-error-raised? (lambda () (port-position bc))))

;; -------------------------------------------------- i/o-invalid-position-error?

(define (raised thunk) (guard (e (#t e)) (thunk) #f))

(define b3 (open-input-bytevector (bytevector 1 2 3)))
(define s3 (open-input-string "abc"))

;; true of every way set-port-position! can refuse: out of range either side,
;; and a port that has no position to set
(test-assert (i/o-invalid-position-error? (raised (lambda () (set-port-position! b3 9)))))
(test-assert (i/o-invalid-position-error? (raised (lambda () (set-port-position! b3 -1)))))
(test-assert (i/o-invalid-position-error? (raised (lambda () (set-port-position! s3 0)))))

;; it is a file error, as everything raised here is
(test-assert (file-error? (raised (lambda () (set-port-position! b3 9)))))

;; but asking for a position that is not kept is a different failure
(test-assert (file-error? (raised (lambda () (port-position s3)))))
(test-not (i/o-invalid-position-error? (raised (lambda () (port-position s3)))))

;; and so is a file error from somewhere else entirely
(test-assert (file-error? (raised (lambda () (open-input-file "no-such-file-192")))))
(test-not (i/o-invalid-position-error? (raised (lambda () (open-input-file "no-such-file-192")))))

;; one can be built without being raised, and the predicate knows it
(test-assert (i/o-invalid-position-error? (make-i/o-invalid-position-error 42)))
(test-assert (file-error? (make-i/o-invalid-position-error 42)))
(test-equal '(42) (error-object-irritants (make-i/o-invalid-position-error 42)))

;; objects that are not conditions are simply false, including the message itself
(test-not (i/o-invalid-position-error? 42))
(test-not (i/o-invalid-position-error? "port position cannot be set"))
(test-not (i/o-invalid-position-error? b3))
(test-not (i/o-invalid-position-error? #f))

;; ----------------------------------------------------------- a port is required

(test-error (port-has-port-position? 'not-a-port))
(test-error (port-position 42))
(test-error (set-port-position! "nope" 0))

(test-end)
