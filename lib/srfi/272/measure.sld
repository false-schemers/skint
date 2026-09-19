; SPDX-FileCopyrightText: 2026 Sergei Egorov
;
; SPDX-License-Identifier: MIT

; Char-width library: scheme analog of wcwidth()
; returns #f, 0, 1, 2

(define-library (srfi 272 measure)
  (import (scheme base) (scheme char))

  (export char-width-procedure)

  (begin
    (cond-expand
      (unicode
       ; char-width: portable Scheme approximation of traditional wcwidth()
       ; assumes some (?) UTF-8 locale; returns #f (=not ptintable), 0, 1, 2
       ; if your system has access to the real wcwidth, better use it
       (define width-0
         '#(768 879 1155 1161 1425 1479 1552 1562 1611 1631 1648 1648
            6832 6911 7616 7679 8203 8207 8234 8238 8288 8303 8400 8447
            65024 65039 65056 65071 65279 65279 917505 917631 917760
            917999))
       (define width-2
         '#(4352 4447 8986 8987 9001 9002 9193 9196 9200 9200 9203 9203
            9725 9726 9748 9749 9776 9783 9800 9811 9855 9855 9866 9871
            9875 9875 9889 9889 9898 9899 9917 9918 9924 9925 9934 9934
            9940 9940 9962 9962 9970 9971 9973 9973 9978 9978 9981 9981
            9989 9989 9994 9995 10024 10024 10060 10060 10062 10062
            10067 10069 10071 10071 10133 10135 10160 10160 10175 10175
            11035 11036 11088 11088 11093 11093 11904 11929 11931 12019
            12032 12245 12272 12350 12353 12438 12441 12543 12549 12591
            12593 12686 12688 12773 12783 12830 12832 12871 12880 42124
            42128 42182 43360 43388 44032 55203 63744 64255 65040 65049
            65072 65106 65108 65126 65128 65131 65281 65376 65504 65510
            94176 94180 94192 94198 94208 101589 101631 101662 101760
            101874 110576 110579 110581 110587 110589 110590 110592
            110882 110898 110898 110928 110930 110933 110933 110948
            110951 110960 111355 119552 119638 119648 119670 126980
            126980 127183 127183 127374 127374 127377 127386 127488
            127490 127504 127547 127552 127560 127568 127569 127584
            127589 127744 127776 127789 127797 127799 127868 127870
            127891 127904 127946 127951 127955 127968 127984 127988
            127988 127992 128062 128064 128064 128066 128252 128255
            128317 128331 128334 128336 128359 128378 128378 128405
            128406 128420 128420 128507 128591 128640 128709 128716
            128716 128720 128722 128725 128728 128732 128735 128747
            128748 128756 128764 128992 129003 129008 129008 129292
            129338 129340 129349 129351 129535 129648 129660 129664
            129674 129678 129734 129736 129736 129741 129756 129759
            129770 129775 129784 131072 196605 196608 262141))
       (define (has-width? table cp)
         (let ((n (/ (vector-length table) 2)))
           (let loop ((lo 0) (hi (- n 1)))
             (and (<= lo hi)
                  (let* ((mid (quotient (+ lo hi) 2))
                         (start (vector-ref table (* 2 mid)))
                         (end (vector-ref table (+ (* 2 mid) 1))))
                    (cond ((< cp start) (loop lo (- mid 1)))
                          ((> cp end) (loop (+ mid 1) hi))
                          (else #t)))))))
       ; note that we only check real chars, not surrogates
       (define (char-width ch)
         (let ((cp (char->integer ch)))
           (cond ((<= 32 cp 126) 1) ; fast track
                 ((< cp 32) #f) ; C0
                 ((<= 127 cp 159) #f) ; DEL/C1
                 ((has-width? width-0 cp) 0)
                 ((has-width? width-2 cp) 2)
                 (else 1)))))
      (else
       (define (char-width ch) (if (char<=? #\space ch #\~) 1 #f))))

    ; users can plug in their custom procedures
    (define char-width-procedure (make-parameter char-width))))
