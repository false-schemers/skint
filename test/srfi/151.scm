(import (srfi 151))
(include "test.scm")

(test-begin "srfi-151")
(define-syntax check
  (syntax-rules (=>)
    [(_) #t]
    [(_ e => r . more) (begin (test 'r e) (check . more))]))

;; tests from the SRFI doc (some modified for range)
(check
(bitwise-ior 3  10) =>  11
(bitwise-and 11 26) =>  10
(bitwise-xor 3 10) =>   9
(bitwise-eqv 37 12) => -42
(bitwise-and 37 12) =>   4
(bitwise-nand 11 26) =>  -11
(bitwise-nor  11 26) => -28
(bitwise-andc1 11 26) => 16
(bitwise-andc2 11 26) => 1
(bitwise-orc1 11 26) => -2
(bitwise-orc2 11 26) => -17
(arithmetic-shift 8 2) => 32
(bit-count 0) =>  0
(bit-count -1) =>  0
(bit-count 7) =>  3
(bit-count  13) =>  3 ;Two's-complement binary: ...0001101
(bit-count -13) =>  2 ;Two's-complement binary: ...1110011
(bit-count  30) =>  4 ;Two's-complement binary: ...0011110
(bit-count -30) =>  4 ;Two's-complement binary: ...1100010
(bit-count (expt 2 10)) =>  1
(bit-count (- (expt 2 10))) =>  10
(bit-count (- (+ 1 (expt 2 10)))) =>  1
(integer-length  0) => 0
(integer-length  1) => 1
(integer-length -1) => 0
(integer-length  7) => 3
(integer-length -7) => 3
(integer-length  8) => 4
(integer-length -8) => 3
(bitwise-if 3 1 8) => 9
(bitwise-if 3 8 1) => 0
(bitwise-if 1 1 2) => 3
(bitwise-if #b00111100 #b11110000 #b00001111) => #b00110011
(bit-set? 1 1) =>  #false
(bit-set? 0 1) =>  #true
(bit-set? 3 10) =>  #true
(bit-set? 1000000 -1) =>  #true
(bit-set? 2 6) =>  #true
(bit-set? 0 6) =>  #false
(copy-bit 0 0 #t) => #b1
(copy-bit 2 0 #t) => #b100
(copy-bit 2 #b1111 #f) => #b1011
(bit-swap 0 2 4) => #b1
(first-set-bit 1) => 0
(first-set-bit 2) => 1
(first-set-bit 0) => -1
(first-set-bit 40) => 3
(first-set-bit -28) => 2
(first-set-bit (expt  2 19)) => 19
(first-set-bit (expt -2 19)) => 19
(bit-field #b1101101010 0 4) => #b1010
(bit-field #b1101101010 3 9) => #b101101
(bit-field #b1101101010 4 9) => #b10110
(bit-field #b1101101010 4 10) => #b110110
(bit-field 6 0 1) => 0
(bit-field 6 1 3) => 3
(bit-field 6 2 999) => 1
(bit-field #b1000000000000000000000000 24 25) => 1
(bit-field-any? #b1001001 1 6) => #t
(bit-field-any? #b1000001 1 6) => #f
(bit-field-every? #b1011110 1 5) => #t
(bit-field-every? #b1011010 1 5) => #f
(bit-field-clear #b101010 1 4) => #b100000
(bit-field-set #b101010 1 4) => #b101110
(bit-field-replace #b101010 #b010 1 4) => #b100100
(bit-field-replace #b110 1 0 1) => #b111
(bit-field-replace #b110 1 1 2) => #b110
(bit-field-replace-same #b1111 #b0000 1 3) => #b1001
(bit-field-rotate #b110 0 0 10) => #b110
(bit-field-rotate #b110 0 0 256) => #b110
(bit-field-rotate #b110 1 1 2) => #b110
(bit-field-rotate #b110 1 2 4) => #b1010
(bit-field-rotate #b0111 -1 1 4) => #b1011
(bit-field-reverse 6 1 3) => 6
(bit-field-reverse 6 1 4) => 12
(bit-field-reverse 1 0 16) => #x8000
(bit-field-reverse 1 0 15) => #x4000
(bit-field-reverse 1 0 14) => #x2000
(bits->list #b1110101) => (#t #f #t #f #t #t #t)
(bits->list 3 5) => (#t #t #f #f #f)
(bits->list 6 4) => (#f #t #t #f)
(bits->vector #b1110101) => #(#t #f #t #f #t #t #t)
(list->bits '(#t #f #t #f #t #t #t)) => #b1110101
(list->bits '(#f #f #t #f #t #f #t #t #t)) => #b111010100
(list->bits '(#f #t #t)) => 6
(list->bits '(#f #t #t #f)) => 6
(list->bits '(#f #f #t #t)) => 12
(vector->bits '#(#t #f #t #f #t #t #t)) => #b1110101
(vector->bits '#(#f #f #t #f #t #f #t #t #t)) => #b111010100
(vector->bits '#(#f #t #t)) => 6
(vector->bits '#(#f #t #t #f)) => 6
(vector->bits '#(#f #f #t #t)) => 12
(bits #t #f #t #f #t #t #t) => #b1110101
(bits #f #f #t #f #t #f #t #t #t) => #b111010100
(bitwise-fold cons '() #b1010111) => (#t #f #t #f #t #t #t)
)

(let ((g (make-bitwise-generator #b110)))
    (test #f (g))
    (test #t (g))
    (test #t (g))
    (test #f (g)))

;; [esl] special tests for access outside fixnum range
(test #t (bit-set? 1000000 -1))
(test #f (bit-set? 1000000 1))
(test -1 (copy-bit 1000000 -1 #t))
(test 1 (copy-bit 1000000 1 #f))
(test -1 (arithmetic-shift -42 -1000000))
(test 0 (arithmetic-shift 42 -1000000))
(test 0 (arithmetic-shift 0 1000000))
(test-error (arithmetic-shift -42 1000000))
(test-error (arithmetic-shift 42 1000000))
(test -1 (bit-field -42 1000000 1000100))
(test 0 (bit-field 42 1000000 1000100))
(test #t (bit-field-any? -42 1000000 1000100))
(test #t (bit-field-every? -42 1000000 1000100))
(test #f (bit-field-any? 42 1000000 1000100))
(test #f (bit-field-every? 42 1000000 1000100))
(test -42 (bit-field-replace -42 -1 1000000 1000100)) 
(test 42 (bit-field-replace 42 0 1000000 1000100))
(test-error (bit-field-replace -42 0 1000000 1000100)) 
(test-error (bit-field-replace 42 -1 1000000 1000100))
(test -42 (bit-field-replace-same -42 -1 1000000 1000100))
(test 42 (bit-field-replace-same 42 1 1000000 1000100))
(test-error (bit-field-replace-same -42 0 1000000 1000100)) 
(test-error (bit-field-replace-same 42 -1 1000000 1000100))
(test -42 (bit-field-set -42 1000000 1000100))
(test 42 (bit-field-clear 42 1000000 1000100))
(test-error (bit-field-set 42 1000000 1000100)) 
(test-error (bit-field-clear -42 1000000 1000100))
(test -42 (bit-swap 1000000 1000100 -42))
(test 42 (bit-swap 1000000 1000100 42))
(test -1 (bit-swap 1000000 0 -1))
(test 0 (bit-swap 1000000 0 0))
(test-error (bit-swap 1000000 0 1))
(test -42 (bit-field-rotate -42 123 1000000 1000100))
(test 42 (bit-field-rotate 42 123 1000000 1000100))
(test -42 (bit-field-reverse -42 1000000 1000100))
(test 42 (bit-field-reverse 42 1000000 1000100))

;; Chibi test, adapted to Skint limitations

(test 0 (integer-length #b0))
(test 1 (integer-length #b1))
(test 2 (integer-length #b10))
(test 3 (integer-length #b100))
(test 4 (integer-length #b1000))
(test 5 (integer-length #b10000))
(test 6 (integer-length #b110000))
(test 0 (bitwise-and #b0 #b1))
(test 1 (bitwise-and #b1 #b1))
(test 0 (bitwise-and #b1 #b10))
(test #b10 (bitwise-and #b11 #b10))
(test #b101 (bitwise-and #b101 #b111))
(test #b111 (bitwise-and -1 #b111))
(test #b110 (bitwise-and -2 #b111))

(test 1 (arithmetic-shift 1 0))
(test 2 (arithmetic-shift 1 1))
(test 4 (arithmetic-shift 1 2))
(test 8 (arithmetic-shift 1 3))
(test 16 (arithmetic-shift 1 4))
(test (expt 2 21) (arithmetic-shift 1 21))
(test (expt 2 22) (arithmetic-shift 1 22))
(test (expt 2 23) (arithmetic-shift 1 23))

(test -1 (arithmetic-shift -1 0))
(test -2 (arithmetic-shift -1 1))
(test -4 (arithmetic-shift -1 2))
(test -8 (arithmetic-shift -1 3))
(test -16 (arithmetic-shift -1 4))
(test (- (expt 2 21)) (arithmetic-shift -1 21))
(test (- (expt 2 22)) (arithmetic-shift -1 22))
(test (- (expt 2 23)) (arithmetic-shift -1 23))

(test 0 (arithmetic-shift 1 -31))

(test #x79e5 (arithmetic-shift #x79e5a0 -8))

(test-not (bit-set? 14 1))
(test-assert (bit-set? 16 #x10000))
(test-assert (bit-set? 1000000 -1))
(test-assert (bit-set? 1000 -1))

(test #b1010 (bit-field #b1101101010 0 4))
(test #b101101 (bit-field #b1101101010 3 9))
(test #b10110 (bit-field #b1101101010 4 9))
(test #b110110 (bit-field #b1101101010 4 10))

(test 0 (bitwise-if 1 2 1))
(test 3 (bitwise-if 1 1 2))
(test 9 (bitwise-if 3 1 8))
(test 0 (bitwise-if 3 8 1))
(test #b00110011 (bitwise-if #b00111100 #b11110000 #b00001111))

(test #b1 (copy-bit 0 0 #t))
(test #b100 (copy-bit 2 0 #t))
(test #b1011 (copy-bit 2 #b1111 #f))

(test #b1110 (bit-swap 0 1 #b1101))
(test #b1011 (bit-swap 1 2 #b1101))
(test #b1011 (bit-swap 2 1 #b1101))
(test #b10000000101 (bit-swap 3 10 #b1101))

(test '(#t #t #t #f #t #f #t) (bits->list #b1010111))
(test '(#t #t #t #f #t) (bits->list #b1010111 5))
(test '(#t #t #t #f #t #f #t #f #f) (bits->list #b1010111 9))
(test '#(#t #t #t #f #t #f #t) (bits->vector #b1010111))
(test '#(#t #t #t #f #t #f #t #f #f) (bits->vector #b1010111 9))

(test #b1010111 (list->bits '(#t #t #t #f #t #f #t)))
(test #b1010111 (list->bits '(#t #t #t #f #t #f #t #f #f)))
(test #b1010111 (vector->bits '#(#t #t #t #f #t #f #t)))
(test #b1010111 (vector->bits '#(#t #t #t #f #t #f #t #f #f)))
(test #b1010111 (bits #t #t #t #f #t #f #t))
(test #b1010111 (bits #t #t #t #f #t #f #t #f #f))

(test 0 (first-set-bit 1))
(test 1 (first-set-bit 2))
(test -1 (first-set-bit 0))
(test 3 (first-set-bit 40))
(test 2 (first-set-bit -28))
(test 19 (first-set-bit (expt  2 19)))
(test 19 (first-set-bit (expt -2 19)))

(test '(#t #f #t #f #t #t #t) (bitwise-fold cons '() #b1010111))

(test 5
    (let ((count 0))
    (bitwise-for-each (lambda (b) (if b (set! count (+ count 1))))
                        #b1010111)
    count))

(test #b101010101
    (bitwise-unfold (lambda (i) (= i 10)) even? (lambda (i) (+ i 1)) 0))

(test #b110  (bit-field-rotate #b110 1 1 2))
(test #b1010 (bit-field-rotate #b110 1 2 4))
(test #b1011 (bit-field-rotate #b0111 -1 1 4))
(test #b0  (bit-field-rotate #b0 128 0 256))
(test #b1  (bit-field-rotate #b1 128 1 256))
(test #b110 (bit-field-rotate #b110 0 0 10))
(test #b110 (bit-field-rotate #b110 0 0 256))

(test 6 (bit-field-reverse 6 1 3))
(test 12 (bit-field-reverse 6 1 4))
(test #x8000 (bit-field-reverse 1 0 16))
(test #x4000 (bit-field-reverse 1 0 15))
(test #x2000 (bit-field-reverse 1 0 14))
(test (bitwise-ior (arithmetic-shift -1 16) #xFBFF) 
(bit-field-reverse -2 0 11))
(test (bitwise-ior (arithmetic-shift -1 16) #xF7FF) 
(bit-field-reverse -2 0 12))
(test (bitwise-ior (arithmetic-shift -1 16) #xEFFF) 
(bit-field-reverse -2 0 13))
(test (bitwise-ior (arithmetic-shift -1 16) #xDFFF) 
(bit-field-reverse -2 0 14))
(test (bitwise-ior (arithmetic-shift -1 16) #xBFFF) 
(bit-field-reverse -2 0 15))
(test (bitwise-ior (arithmetic-shift -1 16) #x7FFF) 
(bit-field-reverse -2 0 16))

(test-assert (bit-field-any? #b1001001 1 6))
(test-not    (bit-field-any? #b1000001 1 6))
(test-assert (bit-field-every? 45 2 4))
(test-assert (bit-field-every? 45 0 1))
(test-assert (bit-field-every? #b1011110 1 5))
(test-not    (bit-field-every? #b1011010 1 5))

(test-end)

