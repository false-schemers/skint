;;; SRFI 192: Port Positioning
;;;
;;; A position is an integer, not necessarily an exact one: it comes back exact
;;; whenever the configuration can hold it exactly, and as a whole flonum when
;;; it cannot, which puts the ceiling at 2^53 in a build with no numeric tower.
;;; Any integer form is accepted going the other way.
;;;
;;; Skint has no condition types, so the errors here are file errors, and
;;; i/o-invalid-position-error? tells its own apart from the rest by the message
;;; it raised them with. That means the message is part of the interface: change
;;; the string in one place and the predicate follows, but do not give two
;;; different failures the same one.
;;;
;;; Only binary ports have positions so far.

(define-library (srfi 192)
  (import (scheme base)
          (only (skint hidden)
                %port-poscaps %port-tell %port-seek file-error error-object))
  (export port-has-port-position? port-position
          port-has-set-port-position!? set-port-position!
          i/o-invalid-position-error? make-i/o-invalid-position-error)
  (begin

    ;; the position the port would not take; see the header
    (define invalid-position-message "port position cannot be set")

    ;; %port-poscaps answers 1 for a port that can tell its position, 2 for one
    ;; that can be moved, 3 for both, 0 for neither

    (define (port-has-port-position? port)
      (let ((caps (%port-poscaps port)))
        (or (= caps 1) (= caps 3))))

    (define (port-has-set-port-position!? port)
      (let ((caps (%port-poscaps port)))
        (or (= caps 2) (= caps 3))))

    (define (port-position port)
      (let ((pos (%port-tell port)))
        (if pos
            pos
            (file-error "port has no position" port))))

    (define (set-port-position! port pos)
      ;; the srfi asks for an output port to be flushed first, even when the
      ;; position does not end up moving
      (if (output-port? port) (flush-output-port port))
      (if (%port-seek port pos 0)
          (if #f #f)
          (file-error invalid-position-message port pos)))

    ;; true of whatever set-port-position! raises -- a position out of range, or
    ;; a port that cannot be positioned at all -- and of nothing else: asking a
    ;; port for a position it does not keep is a different failure, and reads as
    ;; false here
    (define (i/o-invalid-position-error? obj)
      (and (file-error? obj)
           (equal? (error-object-message obj) invalid-position-message)))

    ;; builds one without raising it, for a caller with its own port to refuse.
    ;; The srfi gives it the position alone, so it carries just that; the one
    ;; set-port-position! raises names the port as well, which is worth more to
    ;; whoever reads it and costs the predicate nothing
    (define (make-i/o-invalid-position-error pos)
      (error-object 'file invalid-position-message (list pos)))

    ))
