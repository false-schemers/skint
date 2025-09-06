;[esl] adapted from ref. imp. by Arvydas Silanskas

(define-library (srfi 221)

  (import
    (scheme base)
    (scheme case-lambda)
    (srfi 1) ;; lists
    (srfi 41) ;; streams
    (srfi 158)) ;; generators

  (export
    accumulate-generated-values
    gdelete-duplicates
    genumerate
    gcompose-left
    gcompose-right
    gchoice
    generator->stream
    stream->generator)

(begin

(define (accumulate-generated-values acc gen)
  (let ((value (gen)))
   (if (eof-object? value)
       (acc value)
       (begin
         (acc value)
         (accumulate-generated-values acc gen)))))

(define gdelete-duplicates
  (case-lambda
    ((gen) (gdelete-duplicates* gen equal?))
    ((gen =) (gdelete-duplicates* gen =))))

(define (gdelete-duplicates* gen =)
  (define seen '())
  ;; first parameter should be older value than second. However in `member` it's other way around.
  ;; as such function is changed to switch parameters in places
  (define (=* a b) (= b a))
  (define (seen? value)
    (member value seen =*))
  (lambda ()
    (let loop ((value (gen)))
     (cond
       ((eof-object? value)
        value)
       ((seen? value)
        (loop (gen)))
       (else
         (begin
           (set! seen (cons value seen))
           value))))))

(define (genumerate gen)
  (gmap
    cons
    (make-range-generator 0)
    gen))

(define (gcompose-left constr . ops)
  (let loop ((gen (constr))
             (ops ops))
   (if (null? ops)
       gen
       (let* ((op (car ops))
              (new-gen (op gen)))
         (loop new-gen (cdr ops))))))

(define (gcompose-right . args)
  (apply gcompose-left (reverse args)))

(define (gchoice choice-gen . source-gens)
  (define source-gens-v (list->vector source-gens))
  (define l (vector-length source-gens-v))
  (define exhausted-count 0)
  (unless (procedure? choice-gen)
    (error "choice-gen must be a generator"))
  (for-each
    (lambda (g)
      (unless (procedure? g)
        (error "source-gens must be generators")))
    source-gens)
  (lambda ()
    (let loop ((i (choice-gen)))
     (cond
      ;; all source-gens have been exhausted
      ((= exhausted-count l) (eof-object))
      ;; choice-gen have been exhausted
      ((eof-object? i) (eof-object))
      ;; source-gen returned bad value
      ((or (not (integer? i))
           (< i 0)
           (>= i l))
       (error (string-append "choice-gen didn't return an integer in range 0 to "
                             (number->string (- l 1)))))
      (else
        (let ((gen (vector-ref source-gens-v i)))
         (if (not gen)
             ;; we picked exhausted generator -- pick again
             (loop (choice-gen))
             (let ((value (gen)))
              (if (eof-object? value)
                  ;; picked generator was exhausted on this iteration -- mark it and pick again
                  (begin
                    (vector-set! source-gens-v i #f)
                    (set! exhausted-count (+ 1 exhausted-count))
                    (loop (choice-gen)))
                  value)))))))))

(define (generator->stream gen)
  (define gen-stream
    (stream-lambda()
      (stream-cons (gen) (gen-stream))))
  (stream-take-while
    (lambda (value) (not (eof-object? value)))
    (gen-stream)))

(define (stream->generator stream)
  (lambda ()
    (if (stream-null? stream)
        (eof-object)
        (let ((value (stream-car stream)))
         (set! stream (stream-cdr stream))
         value))))
))
