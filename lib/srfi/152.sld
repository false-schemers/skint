
(define-library (srfi 152)
  (import
    (scheme base) (scheme cxr) (scheme char) (scheme case-lambda) (srfi 8)
    (only (skint) string-position))
  (export
    string? string-null? string-every string-any
    make-string string string-tabulate string-unfold string-unfold-right
    string->vector string->list vector->string list->string reverse-list->string
    string-length string-ref substring string-copy
    string-take string-take-right string-drop string-drop-right
    string-pad string-pad-right string-trim string-trim-right string-trim-both
    string-replace
    string=? string-ci=? string<? string-ci<? string>? string-ci>?
    string<=? string-ci<=? string>=? string-ci>=?
    string-prefix-length string-suffix-length string-prefix? string-suffix?
    string-index string-index-right string-skip string-skip-right
    string-contains string-contains-right
    string-take-while string-take-while-right string-drop-while string-drop-while-right
    string-break string-span string-append string-concatenate string-concatenate-reverse
    string-join string-fold string-fold-right string-map string-for-each string-count
    string-filter string-remove string-replicate string-segment string-split
    read-string write-string
    string-set! string-fill! string-copy!)

(begin
;;;
;;; Author: Francisco Solsona <solsona@acm.org>

;; NOTE: This is the `less-efficient version of LET-OPTIONALS*'.
;;       Once I understand the more efficient one, as for to
;;       adapt it to PLT Scheme, I will.  Sorry, Olin Shivers,
;;       wrote a far to complex thing for me to grasp. :-{

;; (LET-OPTIONALS* arg-list (opt-clause1 ... opt-clauseN [rest]) body ...)
;; where
;;     <opt-clause> ::= (var default [arg-check supplied?])
;;                  |   ((var1 ... varN) external-arg-parser)
;;
;; LET-OPTIONALS* has LET* scope -- each arg clause sees the bindings of
;; the previous clauses. LET-OPTIONALS has LET scope -- each arg clause
;; sees the outer scope (an ARG-CHECK expression sees the outer scope
;; *plus* the variable being bound by that clause, by necessity).

(define-syntax let-optionals*
  (syntax-rules ()
    ((let-optionals* arg (opt-clause ...) body ...)
     (let ((rest arg))
       (%let-optionals* rest (opt-clause ...) body ...)))))

(define-syntax %let-optionals*
  (syntax-rules ()
    ((%let-optionals* arg (((var ...) xparser) opt-clause ...) body ...)
     (call-with-values (lambda () (xparser arg))
       (lambda (rest var ...)
         (%let-optionals* rest (opt-clause ...) body ...))))
                        
    ((%let-optionals* arg ((var default) opt-clause ...) body ...)
     (call-with-values (lambda () (if (null? arg) (values default '())
                                      (values (car arg) (cdr arg))))
       (lambda (var rest)
         (%let-optionals* rest (opt-clause ...) body ...))))

    ((%let-optionals* arg ((var default test) opt-clause ...) body ...)
     (call-with-values (lambda ()
                         (if (null? arg) (values default '())
                             (let ((var (car arg)))
                               (if test (values var (cdr arg))
                                   (error "arg failed LET-OPT test" var)))))
       (lambda (var rest)
         (%let-optionals* rest (opt-clause ...) body ...))))

    ((%let-optionals* arg ((var default test supplied?) opt-clause ...) body ...)
     (call-with-values (lambda ()
                         (if (null? arg) (values default #f '())
                             (let ((var (car arg)))
                               (if test (values var #t (cdr arg))
                                   (error "arg failed LET-OPT test" var)))))
       (lambda (var supplied? rest)
         (%let-optionals* rest (opt-clause ...) body ...))))

    ((%let-optionals* arg (rest) body ...)
     (let ((rest arg)) body ...))

    ((%let-optionals* arg () body ...)
     (if (null? arg) (begin body ...)
         (error "Too many arguments in let-opt" arg)))))

; end LET-OPTIONALS* code

; [esl] selected extracts from:
;;; SRFI 130 string library reference implementation    -*- Scheme -*-
;;; Olin Shivers 7/2000
;;; John Cowan 4/2016
;;;
;;; Copyright (c) 1988-1994 Massachusetts Institute of Technology.
;;; Copyright (c) 1998, 1999, 2000 Olin Shivers. 
;;; Copyright (c) 2016 John Cowan.
;;;   The details of the copyrights appear at the end of the file. Short
;;;   summary: BSD-style open source.

(define check-arg
   (lambda (pred val proc) 
     (if (pred val) val (error "Bad arg" val pred proc))))

(define (substring-spec-ok? s start end)
  (and (string? s)
       (integer? start)
       (exact? start)
       (integer? end)
       (exact? end)
       (<= 0 start)
       (<= start end)
       (<= end (string-length s))))

(define (check-substring-spec proc s start end)
  (if (not (substring-spec-ok? s start end))
      (error "Illegal substring spec." proc s start end)))

(define (string-parse-start+end proc s args)
  (if (not (string? s)) (error "Non-string value" proc s))
  (let ((slen (string-length s)))
    (if (pair? args)
        (let ((start (car args))
              (args (cdr args)))
          (if (and (integer? start) (exact? start) (>= start 0))
              (receive (end args)
                  (if (pair? args)
                      (let ((end (car args))
                            (args (cdr args)))
                        (if (and (integer? end) (exact? end) (<= end slen))
                            (values end args)
                            (error "Illegal substring END spec" proc end s)))
                      (values slen args))
                (if (<= start end) (values args start end)
                    (error "Illegal substring START/END spec"
                           proc start end s)))
              (error "Illegal substring START spec" proc start s)))
        (values '() 0 slen))))

(define (string-parse-final-start+end proc s args)
  (receive (rest start end) (string-parse-start+end proc s args)
    (if (pair? rest) (error "Extra arguments to procedure" proc rest)
        (values start end))))

(define-syntax let-string-start+end
  (syntax-rules ()
    ((let-string-start+end (start end) proc s-exp args-exp body ...)
     (receive (start end) (string-parse-final-start+end proc s-exp args-exp)
       body ...))
    ((let-string-start+end (start end rest) proc s-exp args-exp body ...)
     (receive (rest start end) (string-parse-start+end proc s-exp args-exp)
       body ...))))

(define-syntax let-string-start+end2
  (syntax-rules ()
    ((l-s-s+e2 (start1 end1 start2 end2) proc s1 s2 args body ...)
     (let ((procv proc)) ; Make sure PROC is only evaluated once.
       (let-string-start+end (start1 end1 rest) procv s1 args
         (let-string-start+end (start2 end2) procv s2 rest
           body ...))))))

(define-syntax %string-copy! string-copy!) ; NB: not a builtin!
(define-syntax %substring substring)
(define-syntax add1 (syntax-rules () ((_ x) (+ 1 x))))

; (string-null? s)
(define (string-null? s) (zero? (string-length s)))

; (string-every pred s [start end])
(define (string-every criterion s . maybe-start+end)
  (let-string-start+end (start end) string-every s maybe-start+end
    (or (= start end) ; final (PRED S[END-1]) call
        (let lp ((i start)) ; is a tail call.
          (let ((c (string-ref s i))
                (i1 (+ i 1)))
            (if (= i1 end) (criterion c)  ; Tail call.
                (and (criterion c) (lp i1))))))))

; (string-any pred s [start end])
(define (string-any criterion s . maybe-start+end)
  (let-string-start+end (start end) string-any s maybe-start+end
    (and (< start end) ; final (PRED S[END-1]) call
         (let lp ((i start)) ; is a tail call.
           (let ((c (string-ref s i))
                 (i1 (+ i 1)))
             (if (= i1 end) (criterion c)  ; Tail call
                 (or (criterion c) (lp i1))))))))

; (string-tabulate proc len)
(define (string-tabulate proc len)
  (check-arg procedure? proc string-tabulate)
  (check-arg (lambda (val) (and (integer? val) (exact? val) (<= 0 val)))
             len string-tabulate)
  (let ((s (make-string len)))
    (do ((i (- len 1) (- i 1)))
        ((< i 0))
      (string-set! s i (proc i)))
    s))

; (string-unfold stop? mapper successor seed [base make-final])
(define (string-unfold p f g seed . base+make-final)
  (check-arg procedure? p string-unfold)
  (check-arg procedure? f string-unfold)
  (check-arg procedure? g string-unfold)
  (let-optionals* base+make-final
                  ((base       ""              (string? base))
                   (make-final (lambda (x) "") (procedure? make-final)))
    (let lp ((chunks '())                ; Previously filled chunks
             (nchars 0)                        ; Number of chars in CHUNKS
             (chunk (make-string 40))        ; Current chunk into which we write
             (chunk-len 40)
             (i 0)                        ; Number of chars written into CHUNK
             (seed seed))
      (let lp2 ((i i) (seed seed))
        (if (not (p seed))
            (let ((c (f seed))
                  (seed (g seed)))
              (if (< i chunk-len)
                  (begin (string-set! chunk i c)
                         (lp2 (+ i 1) seed))

                  (let* ((nchars2 (+ chunk-len nchars))
                         (chunk-len2 (min 4096 nchars2))
                         (new-chunk (make-string chunk-len2)))
                    (string-set! new-chunk 0 c)
                    (lp (cons chunk chunks) (+ nchars chunk-len)
                        new-chunk chunk-len2 1 seed))))

            ;; We're done. Make the answer string & install the bits.
            (let* ((final (make-final seed))
                   (flen (string-length final))
                   (base-len (string-length base))
                   (j (+ base-len nchars i))
                   (ans (make-string (+ j flen))))
              (%string-copy! ans j final 0 flen)        ; Install FINAL.
              (let ((j (- j i)))
                (%string-copy! ans j chunk 0 i)                ; Install CHUNK[0,I).
                (let lp ((j j) (chunks chunks))                ; Install CHUNKS.
                  (if (pair? chunks)
                      (let* ((chunk  (car chunks))
                             (chunks (cdr chunks))
                             (chunk-len (string-length chunk))
                             (j (- j chunk-len)))
                        (%string-copy! ans j chunk 0 chunk-len)
                        (lp j chunks)))))
              (%string-copy! ans 0 base 0 base-len)        ; Install BASE.
              ans))))))

; (string-unfold-right stop? mapper successor seed [base make-final])
(define (string-unfold-right p f g seed . base+make-final)
  (let-optionals* base+make-final
                  ((base       ""              (string? base))
                   (make-final (lambda (x) "") (procedure? make-final)))
    (let lp ((chunks '())                ; Previously filled chunks
             (nchars 0)                        ; Number of chars in CHUNKS
             (chunk (make-string 40))        ; Current chunk into which we write
             (chunk-len 40)
             (i 40)                        ; Number of chars available in CHUNK
             (seed seed))
      (let lp2 ((i i) (seed seed))        ; Fill up CHUNK from right
        (if (not (p seed))                ; to left.
            (let ((c (f seed))
                  (seed (g seed)))
              (if (> i 0)
                  (let ((i (- i 1)))
                    (string-set! chunk i c)
                    (lp2 i seed))

                  (let* ((nchars2 (+ chunk-len nchars))
                         (chunk-len2 (min 4096 nchars2))
                         (new-chunk (make-string chunk-len2))
                         (i (- chunk-len2 1)))
                    (string-set! new-chunk i c)
                    (lp (cons chunk chunks) (+ nchars chunk-len)
                        new-chunk chunk-len2 i seed))))

            ;; We're done. Make the answer string & install the bits.
            (let* ((final (make-final seed))
                   (flen (string-length final))
                   (base-len (string-length base))
                   (chunk-used (- chunk-len i))
                   (j (+ base-len nchars chunk-used))
                   (ans (make-string (+ j flen))))
              (%string-copy! ans 0 final 0 flen)        ; Install FINAL.
              (%string-copy! ans flen chunk i chunk-len); Install CHUNK[I,).
              (let lp ((j (+ flen chunk-used))                ; Install CHUNKS.
                       (chunks chunks))                
                  (if (pair? chunks)
                      (let* ((chunk  (car chunks))
                             (chunks (cdr chunks))
                             (chunk-len (string-length chunk)))
                        (%string-copy! ans j chunk 0 chunk-len)
                        (lp (+ j chunk-len) chunks))
                      (%string-copy! ans j base 0 base-len))); Install BASE.
              ans))))))

; (reverse-list->string char-list)
(define (reverse-list->string clist)
  (let* ((len (length clist))
         (s (make-string len)))
    (do ((i (- len 1) (- i 1))   (clist clist (cdr clist)))
        ((not (pair? clist)))
      (string-set! s i (car clist)))
    s))

; (string-take string nchars)
(define (string-take s n)
  (check-arg string? s string-take)
  (check-arg (lambda (val) (and (integer? n) (exact? n)
                                (<= 0 n (string-length s))))
             n string-take)
  (%substring s 0 n))

; (string-take-right string nchars)
(define (string-take-right s n)
  (check-arg string? s string-take-right)
  (let ((len (string-length s)))
    (check-arg (lambda (val) (and (integer? n) (exact? n) (<= 0 n len)))
               n string-take-right)
    (%substring s (- len n) len)))

; (string-drop string nchars)
(define (string-drop s n)
  (check-arg string? s string-drop)
  (let ((len (string-length s)))
    (check-arg (lambda (val) (and (integer? n) (exact? n) (<= 0 n len)))
               n string-drop)
  (%substring s n len)))

; (string-drop-right string nchars)
(define (string-drop-right s n)
  (check-arg string? s string-drop-right)
  (let ((len (string-length s)))
    (check-arg (lambda (val) (and (integer? n) (exact? n) (<= 0 n len)))
               n string-drop-right)
    (%substring s 0 (- len n))))


; (string-pad string len [char start end])
(define (string-pad s n . char+start+end)
  (let-optionals* char+start+end ((char #\space (char? char)) rest)
    (let-string-start+end (start end) string-pad s rest
      (check-arg (lambda (n) (and (integer? n) (exact? n) (<= 0 n)))
                 n string-pad)
      (let ((len (- end start)))
        (if (<= n len)
            (%substring s (- end n) end)
            (let ((ans (make-string n char)))
              (%string-copy! ans (- n len) s start end)
              ans))))))

; (string-pad-right string len [char start end])
(define (string-pad-right s n . char+start+end)
  (let-optionals* char+start+end ((char #\space (char? char)) rest)
    (let-string-start+end (start end) string-pad-right s rest
      (check-arg (lambda (n) (and (integer? n) (exact? n) (<= 0 n)))
                 n string-pad-right)
      (let ((len (- end start)))
        (if (<= n len)
            (%substring s start (+ start n))
            (let ((ans (make-string n char)))
              (%string-copy! ans 0 s start end)
              ans))))))

; (string-trim string [pred start end])
(define (string-trim s . criterion+start+end)
  (let-optionals* criterion+start+end ((criterion char-whitespace?) rest)
    (let-string-start+end (start end) string-trim s rest
      (cond ((string-skip s criterion start end) =>
             (lambda (i) (%substring s i end)))
            (else "")))))

; (string-trim-right string [pred start end])
(define (string-trim-right s . criterion+start+end)
  (let-optionals* criterion+start+end ((criterion char-whitespace?) rest)
    (let-string-start+end (start end) string-trim-right s rest
      (cond ((string-skip-right s criterion start end) =>
             (lambda (i) (%substring s start (+ 1 i))))
            (else "")))))

; (string-trim-both string [pred start end])
(define (string-trim-both s . criterion+start+end)
  (let-optionals* criterion+start+end ((criterion char-whitespace?) rest)
    (let-string-start+end (start end) string-trim-both s rest
      (cond ((string-skip s criterion start end) =>
             (lambda (i)
               (%substring s i (+ 1 (string-skip-right s criterion i end)))))
            (else "")))))

; (string-replace string1 string2 start1 end1 [start2 end2])
(define (string-replace s1 s2 start1 end1 . maybe-start+end)
  (check-substring-spec string-replace s1 start1 end1)
  (let-string-start+end (start2 end2) string-replace s2 maybe-start+end
    (let* ((slen1 (string-length s1))
           (sublen2 (- end2 start2))
           (alen (+ (- slen1 (- end1 start1)) sublen2))
           (ans (make-string alen)))
      (%string-copy! ans 0 s1 0 start1)
      (%string-copy! ans start1 s2 start2 end2)
      (%string-copy! ans (+ start1 sublen2) s1 end1 slen1)
      ans)))

;;; Find the length of the common prefix/suffix.
;;; It is not required that the two substrings passed be of equal length.
;;; This was microcode in MIT Scheme -- a very tightly bummed primitive.
;;; %STRING-PREFIX-LENGTH is the core routine of all string-comparisons,
;;; so should be as tense as possible.
(define (%string-prefix-length s1 start1 end1 s2 start2 end2)
  (let* ((delta (min (- end1 start1) (- end2 start2)))
         (end1 (+ start1 delta)))
    (if (and (eq? s1 s2) (= start1 start2))        ; EQ fast path
        delta
        (let lp ((i start1) (j start2))                ; Regular path
          (if (or (>= i end1)
                  (not (char=? (string-ref s1 i)
                               (string-ref s2 j))))
              (- i start1)
              (lp (+ i 1) (+ j 1)))))))
(define (%string-suffix-length s1 start1 end1 s2 start2 end2)
  (let* ((delta (min (- end1 start1) (- end2 start2)))
         (start1 (- end1 delta)))
    (if (and (eq? s1 s2) (= end1 end2))                ; EQ fast path
        delta
        (let lp ((i (- end1 1)) (j (- end2 1)))        ; Regular path
          (if (or (< i start1)
                  (not (char=? (string-ref s1 i)
                               (string-ref s2 j))))
              (- (- end1 i) 1)
              (lp (- i 1) (- j 1)))))))
(define (%string-prefix? s1 start1 end1 s2 start2 end2)
  (let ((len1 (- end1 start1)))
    (and (<= len1 (- end2 start2))        ; Quick check
         (= (%string-prefix-length s1 start1 end1
                                   s2 start2 end2)
            len1))))
(define (%string-suffix? s1 start1 end1 s2 start2 end2)
  (let ((len1 (- end1 start1)))
    (and (<= len1 (- end2 start2))        ; Quick check
         (= len1 (%string-suffix-length s1 start1 end1
                                        s2 start2 end2)))))

; (string-prefix-length string1 string2 [start1 end1 start2 end2])
(define (string-prefix-length s1 s2 . maybe-starts+ends)
  (let-string-start+end2 (start1 end1 start2 end2) 
                         string-prefix-length s1 s2 maybe-starts+ends
    (%string-prefix-length s1 start1 end1 s2 start2 end2)))

; (string-suffix-length string1 string2 [start1 end1 start2 end2])
(define (string-suffix-length s1 s2 . maybe-starts+ends)
  (let-string-start+end2 (start1 end1 start2 end2) 
                         string-suffix-length s1 s2 maybe-starts+ends
    (%string-suffix-length s1 start1 end1 s2 start2 end2)))

; (string-prefix? string1 string2 [start1 end1 start2 end2])
(define (string-prefix? s1 s2 . maybe-starts+ends)
  (let-string-start+end2 (start1 end1 start2 end2) 
                         string-prefix? s1 s2 maybe-starts+ends
    (%string-prefix? s1 start1 end1 s2 start2 end2)))

; (string-suffix? string1 string2 [start1 end1 start2 end2])
(define (string-suffix? s1 s2 . maybe-starts+ends)
  (let-string-start+end2 (start1 end1 start2 end2) 
                         string-suffix? s1 s2 maybe-starts+ends
    (%string-suffix? s1 start1 end1 s2 start2 end2)))


; (string-index string pred [start end])
(define (string-index str criterion . maybe-start+end)
  (let-string-start+end (start end) string-index str maybe-start+end
    (let lp ((i start))
      (and (< i end)
          (if (criterion (string-ref str i)) i
              (lp (+ i 1)))))))

; (string-index-right string pred [start end])
(define (string-index-right str criterion . maybe-start+end)
  (let-string-start+end (start end) string-index-right str maybe-start+end
    (let lp ((i (- end 1)))
      (and (>= i start)
          (if (criterion (string-ref str i)) i
              (lp (- i 1)))))))

; (string-skip string pred [start end])
(define (string-skip str criterion . maybe-start+end)
  (let-string-start+end (start end) string-skip str maybe-start+end
    (let lp ((i start))
      (and (< i end)
          (if (criterion (string-ref str i)) (lp (+ i 1))
              i)))))

; (string-skip-right string pred [start end])
(define (string-skip-right str criterion . maybe-start+end)
  (let-string-start+end (start end) string-skip-right str maybe-start+end
    (let lp ((i (- end 1)))
      (and (>= i start)
          (if (criterion (string-ref str i)) (lp (- i 1))
              i)))))


; (string-contains text pattern [start1 end1 start2 end2])
(define (string-contains text pattern . maybe-starts+ends)
  ; [esl*] use fast builtin in the common case
  (if (null? maybe-starts+ends) ; [esl+]
      (string-position pattern text) ; [esl+]
      (let-string-start+end2 (t-start t-end p-start p-end)
                             string-contains text pattern maybe-starts+ends
        (%kmp-search pattern text char=? p-start p-end t-start t-end))))

; (string-contains-right text pattern [start1 end1 start2 end2])
(define (string-contains-right text pattern . maybe-starts+ends)
  (let-string-start+end2 (t-start t-end p-start p-end)
                         string-contains-right text pattern maybe-starts+ends
    (let* ((t-len (string-length text))
           (p-len (string-length pattern))
           (p-size (- p-end p-start))
           (rt-start (- t-len t-end))
           (rt-end (- t-len t-start))
           (rp-start (- p-len p-end))
           (rp-end (- p-len p-start))
           (res (%kmp-search (string-reverse pattern)
                             (string-reverse text)
                             char=? rp-start rp-end rt-start rt-end)))
      (if res
        (- t-len res p-size)
        #f))))

;;; Knuth-Morris-Pratt string searching
(define (%kmp-search pattern text c= p-start p-end t-start t-end)
  (let ((plen (- p-end p-start))
        (rv (make-kmp-restart-vector pattern c= p-start p-end)))
    ;; The search loop. TJ & PJ are redundant state.
    (let lp ((ti t-start) (pi 0)
             (tj (- t-end t-start)) ; (- tlen ti) -- how many chars left.
             (pj plen))                 ; (- plen pi) -- how many chars left.
      (if (= pi plen)
          (- ti plen)                        ; Win.
          (and (<= pj tj)                ; Lose.
               (if (c= (string-ref text ti) ; Search.
                       (string-ref pattern (+ p-start pi)))
                   (lp (+ 1 ti) (+ 1 pi) (- tj 1) (- pj 1)) ; Advance.
                   
                   (let ((pi (vector-ref rv pi))) ; Retreat.
                     (if (= pi -1)
                         (lp (+ ti 1) 0  (- tj 1) plen) ; Punt.
                         (lp ti       pi tj       (- plen pi))))))))))

(define (make-kmp-restart-vector pattern . maybe-c=+start+end)
  (let-optionals* maybe-c=+start+end
                  ((c= char=?) rest) ; (procedure? c=))
     (receive (rest2 start end) (string-parse-start+end make-kmp-restart-vector pattern rest)
       (let* ((rvlen (- end start))
              (rv (make-vector rvlen -1)))
      (if (> rvlen 0)
          (let ((rvlen-1 (- rvlen 1))
                (c0 (string-ref pattern start)))
            ;; Here's the main loop. We have set rv[0] ... rv[i].
            ;; K = I + START -- it is the corresponding index into PATTERN.
            (let lp1 ((i 0) (j -1) (k start))
              (if (< i rvlen-1)
                  ;; lp2 invariant:
                  ;;   pat[(k-j) .. k-1] matches pat[start .. start+j-1]
                  ;;   or j = -1.
                  (let lp2 ((j j))
                    (cond ((= j -1)
                           (let ((i1 (+ i 1))
                                 (ck+1 (string-ref pattern (add1 k))))
                             (vector-set! rv i1 (if (c= ck+1 c0) -1 0))
                             (lp1 i1 0 (+ k 1))))
                          ;; pat[(k-j) .. k] matches pat[start..start+j].
                          ((c= (string-ref pattern k)
                               (string-ref pattern (+ j start)))
                           (let* ((i1 (+ 1 i))
                                  (j1 (+ 1 j)))
                             (vector-set! rv i1 j1)
                             (lp1 i1 j1 (+ k 1))))
                          (else (lp2 (vector-ref rv j)))))))))
      rv))))

(define (string-reverse s . maybe-start+end)
  (let-string-start+end (start end) string-reverse s maybe-start+end
    (let* ((len (- end start))
           (ans (make-string len)))
      (do ((i start (+ i 1))
           (j (- len 1) (- j 1)))
          ((< j 0))
        (string-set! ans j (string-ref s i)))
      ans)))


; (string-take-while string pred [start end])
(define (string-take-while s criterion . maybe-start+end)
  (let-string-start+end (start end) string-take-while s maybe-start+end
    (let ((idx (string-skip s criterion start end)))
      (if idx
          (%substring s 0 idx)
          ""))))

; (string-take-while-right string pred [start end])
(define (string-take-while-right s criterion . maybe-start+end)
  (let-string-start+end (start end) string-take-while s maybe-start+end
    (let ((idx (string-skip-right s criterion start end)))
      (if idx
          (%substring s (+ idx 1) (string-length s))
          ""))))

; (string-drop-while string pred [start end])
(define (string-drop-while s criterion . maybe-start+end)
  (let-string-start+end (start end) string-drop-while s maybe-start+end
    (let ((idx (string-skip s criterion start end)))
      (if idx
          (%substring s idx (string-length s))
          s))))

; (string-drop-while-right string pred [start end])
(define (string-drop-while-right s criterion . maybe-start+end)
  (let-string-start+end (start end) string-drop-while s maybe-start+end
    (let ((idx (string-skip-right s criterion start end)))
      (if idx
          (%substring s 0 (+ idx 1))
          s))))

; (string-span string pred [start end])
(define (string-span s criterion . maybe-start+end)
  (let-string-start+end (start end) string-span s maybe-start+end
    (let ((idx (string-skip s criterion start end)))
      (if idx
        (values (%substring s 0 idx) (%substring s idx (string-length s)))
        (values "" s)))))

; (string-break string pred [start end])
(define (string-break s criterion . maybe-start+end)
  (let-string-start+end (start end) string-break s maybe-start+end
    (let ((idx (string-index s criterion start end)))
      (if idx
        (values (%substring s 0 idx) (%substring s idx (string-length s)))
        (values s "")))))


; (string-concatenate string-list)
(define (string-concatenate strings)
  (let* ((total (do ((strings strings (cdr strings))
                     (i 0 (+ i (string-length (car strings)))))
                    ((not (pair? strings)) i)))
         (ans (make-string total)))
    (let lp ((i 0) (strings strings))
      (if (pair? strings)
          (let* ((s (car strings))
                 (slen (string-length s)))
            (%string-copy! ans i s 0 slen)
            (lp (+ i slen) (cdr strings)))))
    ans))

; (string-concatenate-reverse string-list [final-string end]) 
(define (string-concatenate-reverse string-list . maybe-final+end)
  (let-optionals* maybe-final+end ((final "" (string? final))
                                   (end (string-length final)
                                        (and (integer? end)
                                             (exact? end)
                                             (<= 0 end (string-length final)))))
    (let ((len (let lp ((sum 0) (lis string-list))
                 (if (pair? lis)
                     (lp (+ sum (string-length (car lis))) (cdr lis))
                     sum))))
      (%finish-string-concatenate-reverse len string-list final end))))

(define (%finish-string-concatenate-reverse len string-list final end)
  (let ((ans (make-string (+ end len))))
    (%string-copy! ans len final 0 end)
    (let lp ((i len) (lis string-list))
      (if (pair? lis)
          (let* ((s   (car lis))
                 (lis (cdr lis))
                 (slen (string-length s))
                 (i (- i slen)))
            (%string-copy! ans i s 0 slen)
            (lp i lis))))
    ans))

; (string-join string-list [delimiter grammar])
(define (string-join strings . delim+grammar)
  (let-optionals* delim+grammar ((delim " " (string? delim))
                                 (grammar 'infix))
    (let ((buildit (lambda (lis final)
                     (let recur ((lis lis))
                       (if (pair? lis)
                           (cons delim (cons (car lis) (recur (cdr lis))))
                           final)))))
      (cond ((pair? strings)
             (string-concatenate
              (case grammar
                ((infix strict-infix)
                 (cons (car strings) (buildit (cdr strings) '())))
                ((prefix) (buildit strings '()))
                ((suffix)
                 (cons (car strings) (buildit (cdr strings) (list delim))))
                (else (error "Illegal join grammar"
                             grammar string-join)))))
             ((not (null? strings))
              (error "STRINGS parameter not list." strings string-join))
             ;; STRINGS is ()
             ((eq? grammar 'strict-infix)
              (error "Empty list cannot be joined with STRICT-INFIX grammar."
                     string-join))
             (else "")))))                ; Special-cased for infix grammar.



; (string-fold kons knil string [start end])
(define (string-fold kons knil s . maybe-start+end)
  (check-arg procedure? kons string-fold)
  (let-string-start+end (start end) string-fold s maybe-start+end
    (let lp ((v knil) (i start))
      (if (< i end) (lp (kons (string-ref s i) v) (+ i 1))
          v))))

; (string-fold-right kons knil string [start end])
(define (string-fold-right kons knil s . maybe-start+end)
  (check-arg procedure? kons string-fold-right)
  (let-string-start+end (start end) string-fold-right s maybe-start+end
    (let lp ((v knil) (i (- end 1)))
      (if (>= i start) (lp (kons (string-ref s i) v) (- i 1))
          v))))

; (string-count string pred [start end])
(define (string-count s criterion . maybe-start+end)
  (let-string-start+end (start end) string-count s maybe-start+end
           (do ((i start (+ i 1))
                (count 0 (if (criterion (string-ref s i)) (+ count 1) count)))
               ((>= i end) count))))

; (string-filter pred string [start end])
(define (string-filter criterion s . maybe-start+end)
  (let-string-start+end (start end) string-filter s maybe-start+end
        (let* ((slen (- end start))
               (temp (make-string slen))
               (ans-len (string-fold (lambda (c i)
                                       (if (criterion c)
                                           (begin (string-set! temp i c)
                                                  (+ i 1))
                                           i))
                                     0 s start end)))
          (if (= ans-len slen) temp (substring temp 0 ans-len)))))

; (string-remove pred string [start end])
(define (string-remove criterion s . maybe-start+end)
  (let-string-start+end (start end) string-remove s maybe-start+end
        (let* ((slen (- end start))
               (temp (make-string slen))
               (ans-len (string-fold (lambda (c i)
                                       (if (criterion c) i
                                           (begin (string-set! temp i c)
                                                  (+ i 1))))
                                     0 s start end)))
          (if (= ans-len slen) temp (substring temp 0 ans-len)))))

; (string-replicate string from to [start end])
(define (string-replicate s from . maybe-to+start+end)
  (check-arg (lambda (val) (and (integer? val) (exact? val)))
             from string-replicate)
  (receive (to start end)
           (if (pair? maybe-to+start+end)
               (let-string-start+end (start end) string-replicate s (cdr maybe-to+start+end)
                 (let ((to (car maybe-to+start+end)))
                   (check-arg (lambda (val) (and (integer? val)
                                                 (exact? val)
                                                 (<= from val)))
                              to string-replicate)
                   (values to start end)))
               (let ((slen (string-length (check-arg string? s string-replicate))))
                 (values (+ from slen) 0 slen)))
    (let ((slen   (- end start))
          (anslen (- to  from)))
      (cond ((zero? anslen) "")
            ((zero? slen) (error "Cannot replicate empty (sub)string"
                                  string-replicate s from to start end))
            ((= 1 slen)                ; Fast path for 1-char replication.
             (make-string anslen (string-ref s start)))
            ;; Selected text falls entirely within one span.
            ((= (floor (/ from slen)) (floor (/ to slen)))
             (substring s (+ start (modulo from slen))
                          (+ start (modulo to   slen))))
            ;; Selected text requires multiple spans.
            (else (let ((ans (make-string anslen)))
                    (%multispan-repcopy! ans 0 s from to start end)
                    ans))))))

(define (%multispan-repcopy! target tstart s sfrom sto start end)
  (let* ((slen (- end start))
         (i0 (+ start (modulo sfrom slen)))
         (total-chars (- sto sfrom)))
    ;; Copy the partial span ! the beginning
    (%string-copy! target tstart s i0 end)
    (let* ((ncopied (- end i0))                        ; We've copied this many.
           (nleft (- total-chars ncopied))        ; # chars left to copy.
           (nspans (quotient nleft slen)))        ; # whole spans to copy
      ;; Copy the whole spans in the middle.
      (do ((i (+ tstart ncopied) (+ i slen))        ; Current target index.
           (nspans nspans (- nspans 1)))        ; # spans to copy
          ((zero? nspans)
           ;; Copy the partial-span ! the end & we're done.
           (%string-copy! target i s start (+ start (- total-chars (- i tstart)))))
        (%string-copy! target i s start end))))); Copy a whole span.

; (string-segment string k)
(define (string-segment str k)
  (if (< k 1) (error "minimum segment size is 1" k))
  (let ((len (string-length str)))
    (let loop ((start 0)
               (result '()))
      (if (= start len)
        (reverse result)
        (let ((end (min (+ start k) len)))
          (loop end (cons (%substring str start end) result)))))))

;;; Thanks to Shiro Kawai for the following code.

; (string-split string delimiter [grammar limit start end])
(define (string-split s delimiter . args)
  ;; The argument checking part might be refactored with other srfi-130
  ;; routines.
  (if (not (string? s)) (error "string expected" s))
  (if (not (string? delimiter)) (error "string expected" delimiter))
  (let ((slen (string-length s)))
    (receive (grammar limit no-limit start end)
        (if (pair? args)
          (if (pair? (cdr args))
            (if (pair? (cddr args))
              (if (pair? (cdddr args))
                (values (car args) (cadr args) #f (caddr args) (cadddr args))
                (values (car args) (cadr args) #f (caddr args) slen))
              (values (car args) (cadr args) #f 0 slen))
            (values (car args) #f #t 0 slen))
          (values 'infix #f #t 0 slen))
      (if (not (memq grammar '(infix strict-infix prefix suffix)))
        (error "grammar must be one of (infix strict-infix prefix suffix)" grammar))
      (if (not limit) (set! no-limit #t))
      (if (not (or no-limit
                  (and (integer? limit) (exact? limit) (>= limit 0))))
        (error "limit must be exact nonnegative integer or #f" limit))
      (if (not (and (integer? start) (exact? start)))
        (error "start argument must be exact integer" start))
      (if (not (<= 0 start slen))
        (error "start argument out of range" start))
      (if (not (<= 0 end slen))
        (error "end argument out of range" end))
      (if (not (<= start end))
        (error "start argument is greater than end argument" (list start end)))
      (cond ((= start end)
             (if (eq? grammar 'strict-infix)
               (error "empty string cannot be spilt with strict-infix grammar")
               '()))
            ((string-null? delimiter)
             (%string-split-chars s start end limit))
            (else (%string-split s start end delimiter grammar limit))))))

(define (%string-split-chars s start end limit)
  (if (not limit)
    (map string (string->list s start end))
    (let loop ((r '()) (c start) (n 0))
      (cond ((= c end) (reverse r))
            ((>= n limit) (reverse (cons (substring s c end) r)))
            (else (loop (cons (string (string-ref s c)) r)
                        (+ c 1)
                        (+ n 1)))))))

(define (%string-split s start end delimiter grammar limit)
  (let ((dlen (string-length delimiter)))
    (define (finish r c)
      (let ((rest (substring s c end)))
        (if (and (eq? grammar 'suffix) (string-null? rest))
          (reverse r)
          (reverse (cons rest r)))))
    (define (scan r c n)
      (if (and limit (>= n limit))
        (finish r c)
        (let ((i (string-contains s delimiter c end)))
          (if i
            (let ((fragment (substring s c i)))
              (if (and (= n 0) (eq? grammar 'prefix) (string-null? fragment))
                (scan r (+ i dlen) (+ n 1))
                (scan (cons fragment r) 
                      (+ i dlen)
                      (+ n 1))))
            (finish r c)))))
    (scan '() start 0)))

))
