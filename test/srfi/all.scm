(import (skint))
(import (only (skint hidden) file-resolve-relative-to-current %system %command-line))

(define *tests* '(
1
2
4
5
6
8
9
11
13
14
16
17
22
23
25
26
27 ;*
28
29
30
31
34
35
37
38
39
41
42
43
45
46
48
51
54 ;*
55
60
61
62
63
64
66
67
69
71
74 ;*
78
86
87
95
98
111
113
115 ;*
116
117
121
125
127
128
130
131
132
133
134
135
136
137
141
143
144
145
146
151
152
153
156
158
160
162
165
171
172
173
175
176
189
193
194 ;*
196
197
214
217
219
221
222
223
224
225
227
228
232
234
235
236
239
244
251
252 ;*
257
264
))

(define *results* '())

(define (cmdline-base)
  (let loop ([l (%command-line)] [b ""])
    (if (or (null? l) (null? (cdr l)))
        b 
        (loop (cdr l) (string-append b (car l) " ")))))

(define (run-test n)
  ; assumes skint is in the current directory 
  (define fname (format "~a.scm" n))
  (define cmd (format "~a ~a" (cmdline-base) (file-resolve-relative-to-current fname)))
  (format #t "~%~%------------------------------------------------------~%")
  (format #t "RUNNING TESTS FOR SRFI ~a: ~a~%" n cmd)
  (format #t "------------------------------------------------------~%")
  (let ([res (%system cmd)])
    (set! *results* (cons (cons n res) *results*))))

(for-each run-test *tests*)

(format #t "~%~%------------------------------------------------------~%")
(format #t "CUMULATIVE RESULTS:~%")
(format #t "------------------------------------------------------~%~%")

(define *total-fail-count* 0)

(define (show-result n&r)
  (if (zero? (cdr n&r))
      (format #t "SRFI-~a: \tok~%" (car n&r))
      (format #t "SRFI-~a: \t~a failed~%" (car n&r) (cdr n&r)))
  (set! *total-fail-count* (+ *total-fail-count* (cdr n&r))) 
)
 
(for-each show-result (reverse *results*))

(format #t "~%~%SRFIs TESTED: ~a~%" (length *tests*))
(format #t "CUMULATIVE FAILURES: ~a~%" *total-fail-count*)

(exit *total-fail-count*)
