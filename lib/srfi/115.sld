;;;; SPDX-FileCopyrightText: 2013 - 2016 Alex Shinn
;;;;
;;;; SPDX-License-Identifier: BSD-3-Clause

(define-library (srfi 115)
  (export regexp regexp? valid-sre? rx regexp->sre char-set->sre
          regexp-matches regexp-matches? regexp-search
          regexp-replace regexp-replace-all regexp-match->list
          regexp-fold regexp-extract regexp-split regexp-partition
          regexp-match? regexp-match-count
          regexp-match-submatch
          regexp-match-submatch-start regexp-match-submatch-end)
  (import (scheme base) (scheme char) (srfi 1) (srfi 14) (srfi 69))
  (begin
    (define %char-set:letter
      (char-set-intersection char-set:ascii char-set:letter))
    (define %char-set:lower-case
      (char-set-intersection char-set:ascii char-set:lower-case))
    (define %char-set:upper-case
      (char-set-intersection char-set:ascii char-set:upper-case))
    (define %char-set:digit
      (char-set-intersection char-set:ascii char-set:digit))
    (define %char-set:letter+digit
      (char-set-intersection char-set:ascii char-set:letter+digit))
    (define %char-set:punctuation
      (char-set-intersection char-set:ascii char-set:punctuation))
    (define %char-set:symbol
      (char-set-intersection char-set:ascii char-set:symbol))
    (define %char-set:graphic
      (char-set-intersection char-set:ascii char-set:graphic))
    (define %char-set:whitespace
      (char-set-intersection char-set:ascii char-set:whitespace))
    (define %char-set:printing
      (char-set-intersection char-set:ascii char-set:printing))
    (define %char-set:iso-control
      (char-set-intersection char-set:ascii char-set:iso-control)))
  (cond-expand
   ((library (srfi 151)) (import (srfi 151)))
   ((library (srfi 33)) (import (srfi 33)))
   (else (import (srfi 60))))
  ;;[esl-] (import (srfi 115 boundary)) -- see below
  ;; Use string-cursors where available.
  (cond-expand
   (chibi
    (begin
      (define (string-start-arg s o)
        (if (pair? o) (string-index->cursor s (car o)) (string-cursor-start s)))
      (define (string-end-arg s o)
        (if (pair? o) (string-index->cursor s (car o)) (string-cursor-end s)))
      (define (string-concatenate-reverse ls)
        (string-concatenate (reverse ls)))))
   ((library (srfi 130))
    (import (srfi 130))
    (begin
      (define (string-start-arg s o)
        (if (pair? o) (string-index->cursor s (car o)) (string-cursor-start s)))
      (define (string-end-arg s o)
        (if (pair? o) (string-index->cursor s (car o)) (string-cursor-end s)))
      (define string-cursor-ref string-ref/cursor)
      (define substring-cursor substring/cursors)))
   (else
    (begin
      (define (string-start-arg s o)
        (if (pair? o) (string-index->cursor s (car o)) 0))
      (define (string-end-arg s o)
        (if (pair? o) (string-index->cursor s (car o)) (string-length s)))
      (define string-cursor? integer?)
      (define string-cursor=? =)
      (define string-cursor<? <)
      (define string-cursor<=? <=)
      (define string-cursor>? >)
      (define string-cursor>=? >=)
      (define string-cursor-ref string-ref)
      (define (string-cursor-next s i) (+ i 1))
      (define (string-cursor-prev s i) (- i 1))
      (define substring-cursor substring)
      (define (string-cursor->index str off) off)
      (define (string-index->cursor str i) i)
      (define (string-concatenate ls) (apply string-append ls))
      (define (string-concatenate-reverse ls)
        (string-concatenate (reverse ls))))))

(begin
;;[esl**] boundaries depend on Unicode support
(cond-expand 
  [(and skint full-unicode) 
   ; full Unicode support, uses Skint's extended ->char-set
   (begin ; charsets generated via Chibi-derived pre/boundary.scm
    (define char-set:control 
      (->char-set
       #(#x0 #x9 #xb #xc #xe #x1f #x7f #x9f #x600 #x604 #x200e #x200f
         #x202a #x202e #x2060 #x2064 #x2066 #x206f #xfff0 #xfffb
         #x1d173 #x1d17a #xe0002 #xe00ff #xe01f0 #xe0fff)))
    (define char-set:extend-or-spacing-mark
      (->char-set
       #(#x300 #x36f #x483 #x489 #x591 #x5bd #x5c1 #x5c2 #x5c4 #x5c5
         #x610 #x61a #x64b #x65f #x6d6 #x6dc #x6df #x6e4 #x6e7 #x6e8
         #x6ea #x6ed #x730 #x74a #x7a6 #x7b0 #x7eb #x7f3 #x816 #x819
         #x81b #x823 #x825 #x827 #x829 #x82d #x859 #x85b #x8e4 #x8fe
         #x900 #x902 #x93e #x94c #x94e #x94f #x951 #x957 #x962 #x963
         #x982 #x983 #x9bf #x9c4 #x9c7 #x9c8 #x9cb #x9cc #x9e2 #x9e3
         #xa01 #xa02 #xa3e #xa42 #xa47 #xa48 #xa4b #xa4d #xa70 #xa71
         #xa81 #xa82 #xabe #xac5 #xac7 #xac8 #xacb #xacc #xae2 #xae3
         #xb02 #xb03 #xb41 #xb44 #xb47 #xb48 #xb4b #xb4c #xb62 #xb63
         #xbc1 #xbc2 #xbc6 #xbc8 #xbca #xbcc #xc01 #xc03 #xc3e #xc44
         #xc46 #xc48 #xc4a #xc4d #xc55 #xc56 #xc62 #xc63 #xc82 #xc83
         #xcc0 #xcc1 #xcc3 #xcc4 #xcc7 #xcc8 #xcca #xccd #xcd5 #xcd6
         #xce2 #xce3 #xd02 #xd03 #xd3f #xd44 #xd46 #xd48 #xd4a #xd4c
         #xd62 #xd63 #xd82 #xd83 #xdd0 #xdd4 #xdd8 #xdde #xdf2 #xdf3
         #xe34 #xe3a #xe47 #xe4e #xeb4 #xeb9 #xebb #xebc #xec8 #xecd
         #xf18 #xf19 #xf3e #xf3f #xf71 #xf7e #xf80 #xf84 #xf86 #xf87
         #xf8d #xf97 #xf99 #xfbc #x102d #x1030 #x1032 #x1037 #x1039
         #x103e #x1056 #x1059 #x105e #x1060 #x1071 #x1074 #x1085
         #x1086 #x135d #x135f #x1712 #x1714 #x1732 #x1734 #x1752
         #x1753 #x1772 #x1773 #x17b4 #x17b5 #x17b7 #x17c5 #x17c7
         #x17d3 #x180b #x180d #x1920 #x192b #x1930 #x1931 #x1933
         #x193b #x19b5 #x19b7 #x1a17 #x1a1a #x1a58 #x1a5e #x1a65
         #x1a7c #x1b00 #x1b03 #x1b36 #x1b3a #x1b3d #x1b41 #x1b43
         #x1b44 #x1b6b #x1b73 #x1b80 #x1b81 #x1ba2 #x1ba9 #x1bac
         #x1bad #x1be8 #x1bec #x1bef #x1bf3 #x1c24 #x1c37 #x1cd0
         #x1cd2 #x1cd4 #x1ce0 #x1ce2 #x1ce8 #x1cf2 #x1cf3 #x1dc0
         #x1de6 #x1dfc #x1dff #x200c #x200d #x20d0 #x20e0 #x20e2
         #x20f0 #x2cef #x2cf1 #x2de0 #x2dff #x302a #x302f #x3099
         #x309a #xa670 #xa672 #xa674 #xa67d #xa6f0 #xa6f1 #xa823
         #xa826 #xa880 #xa881 #xa8b4 #xa8c3 #xa8e0 #xa8f1 #xa926
         #xa92d #xa947 #xa953 #xa980 #xa982 #xa9b4 #xa9bb #xa9bd
         #xa9c0 #xaa29 #xaa36 #xaab2 #xaab4 #xaab7 #xaab8 #xaabe
         #xaabf #xaaec #xaaef #xabe3 #xabe4 #xabe6 #xabe7 #xabe9
         #xabea #xfe00 #xfe0f #xfe20 #xfe26 #xff9e #xff9f #x10a01
         #x10a03 #x10a05 #x10a06 #x10a0c #x10a0f #x10a38 #x10a3a
         #x11038 #x11046 #x11080 #x11081 #x110b0 #x110ba #x11100
         #x11102 #x11127 #x1112b #x1112d #x11134 #x11180 #x11181
         #x111b3 #x111c0 #x116ae #x116b5 #x16f51 #x16f7e #x16f8f
         #x16f92 #x1d167 #x1d169 #x1d16e #x1d172 #x1d17b #x1d182
         #x1d185 #x1d18b #x1d1aa #x1d1ad #x1d242 #x1d244 #xe0100
         #xe01ef)))
    (define char-set:regional-indicator (->char-set #(#x1f1e6 #x1f1ff)))
    (define char-set:hangul-l (->char-set #(#x1100 #x115f #xa960 #xa97c)))
    (define char-set:hangul-v (->char-set #(#x1160 #x11a7 #xd7b0 #xd7c6)))
    (define char-set:hangul-t (->char-set #(#x11a8 #x11ff #xd7cb #xd7fb)))
    (define char-set:hangul-lv (->char-set #()))
    (define char-set:hangul-lvt 
      (->char-set
       #(#xac01 #xac1b #xac1d #xac37 #xac39 #xac53 #xac55 #xac6f
         #xac71 #xac8b #xac8d #xaca7 #xaca9 #xacc3 #xacc5 #xacdf
         #xace1 #xacfb #xacfd #xad17 #xad19 #xad33 #xad35 #xad4f
         #xad51 #xad6b #xad6d #xad87 #xad89 #xada3 #xada5 #xadbf
         #xadc1 #xaddb #xaddd #xadf7 #xadf9 #xae13 #xae15 #xae2f
         #xae31 #xae4b #xae4d #xae67 #xae69 #xae83 #xae85 #xae9f
         #xaea1 #xaebb #xaebd #xaed7 #xaed9 #xaef3 #xaef5 #xaf0f
         #xaf11 #xaf2b #xaf2d #xaf47 #xaf49 #xaf63 #xaf65 #xaf7f
         #xaf81 #xaf9b #xaf9d #xafb7 #xafb9 #xafd3 #xafd5 #xafef
         #xaff1 #xb00b #xb00d #xb027 #xb029 #xb043 #xb045 #xb05f
         #xb061 #xb07b #xb07d #xb097 #xb099 #xb0b3 #xb0b5 #xb0cf
         #xb0d1 #xb0eb #xb0ed #xb107 #xb109 #xb123 #xb125 #xb13f
         #xb141 #xb15b #xb15d #xb177 #xb179 #xb193 #xb195 #xb1af
         #xb1b1 #xb1cb #xb1cd #xb1e7 #xb1e9 #xb203 #xb205 #xb21f
         #xb221 #xb23b #xb23d #xb257 #xb259 #xb273 #xb275 #xb28f
         #xb291 #xb2ab #xb2ad #xb2c7 #xb2c9 #xb2e3 #xb2e5 #xb2ff
         #xb301 #xb31b #xb31d #xb337 #xb339 #xb353 #xb355 #xb36f
         #xb371 #xb38b #xb38d #xb3a7 #xb3a9 #xb3c3 #xb3c5 #xb3df
         #xb3e1 #xb3fb #xb3fd #xb417 #xb419 #xb433 #xb435 #xb44f
         #xb451 #xb46b #xb46d #xb487 #xb489 #xb4a3 #xb4a5 #xb4bf
         #xb4c1 #xb4db #xb4dd #xb4f7 #xb4f9 #xb513 #xb515 #xb52f
         #xb531 #xb54b #xb54d #xb567 #xb569 #xb583 #xb585 #xb59f
         #xb5a1 #xb5bb #xb5bd #xb5d7 #xb5d9 #xb5f3 #xb5f5 #xb60f
         #xb611 #xb62b #xb62d #xb647 #xb649 #xb663 #xb665 #xb67f
         #xb681 #xb69b #xb69d #xb6b7 #xb6b9 #xb6d3 #xb6d5 #xb6ef
         #xb6f1 #xb70b #xb70d #xb727 #xb729 #xb743 #xb745 #xb75f
         #xb761 #xb77b #xb77d #xb797 #xb799 #xb7b3 #xb7b5 #xb7cf
         #xb7d1 #xb7eb #xb7ed #xb807 #xb809 #xb823 #xb825 #xb83f
         #xb841 #xb85b #xb85d #xb877 #xb879 #xb893 #xb895 #xb8af
         #xb8b1 #xb8cb #xb8cd #xb8e7 #xb8e9 #xb903 #xb905 #xb91f
         #xb921 #xb93b #xb93d #xb957 #xb959 #xb973 #xb975 #xb98f
         #xb991 #xb9ab #xb9ad #xb9c7 #xb9c9 #xb9e3 #xb9e5 #xb9ff
         #xba01 #xba1b #xba1d #xba37 #xba39 #xba53 #xba55 #xba6f
         #xba71 #xba8b #xba8d #xbaa7 #xbaa9 #xbac3 #xbac5 #xbadf
         #xbae1 #xbafb #xbafd #xbb17 #xbb19 #xbb33 #xbb35 #xbb4f
         #xbb51 #xbb6b #xbb6d #xbb87 #xbb89 #xbba3 #xbba5 #xbbbf
         #xbbc1 #xbbdb #xbbdd #xbbf7 #xbbf9 #xbc13 #xbc15 #xbc2f
         #xbc31 #xbc4b #xbc4d #xbc67 #xbc69 #xbc83 #xbc85 #xbc9f
         #xbca1 #xbcbb #xbcbd #xbcd7 #xbcd9 #xbcf3 #xbcf5 #xbd0f
         #xbd11 #xbd2b #xbd2d #xbd47 #xbd49 #xbd63 #xbd65 #xbd7f
         #xbd81 #xbd9b #xbd9d #xbdb7 #xbdb9 #xbdd3 #xbdd5 #xbdef
         #xbdf1 #xbe0b #xbe0d #xbe27 #xbe29 #xbe43 #xbe45 #xbe5f
         #xbe61 #xbe7b #xbe7d #xbe97 #xbe99 #xbeb3 #xbeb5 #xbecf
         #xbed1 #xbeeb #xbeed #xbf07 #xbf09 #xbf23 #xbf25 #xbf3f
         #xbf41 #xbf5b #xbf5d #xbf77 #xbf79 #xbf93 #xbf95 #xbfaf
         #xbfb1 #xbfcb #xbfcd #xbfe7 #xbfe9 #xc003 #xc005 #xc01f
         #xc021 #xc03b #xc03d #xc057 #xc059 #xc073 #xc075 #xc08f
         #xc091 #xc0ab #xc0ad #xc0c7 #xc0c9 #xc0e3 #xc0e5 #xc0ff
         #xc101 #xc11b #xc11d #xc137 #xc139 #xc153 #xc155 #xc16f
         #xc171 #xc18b #xc18d #xc1a7 #xc1a9 #xc1c3 #xc1c5 #xc1df
         #xc1e1 #xc1fb #xc1fd #xc217 #xc219 #xc233 #xc235 #xc24f
         #xc251 #xc26b #xc26d #xc287 #xc289 #xc2a3 #xc2a5 #xc2bf
         #xc2c1 #xc2db #xc2dd #xc2f7 #xc2f9 #xc313 #xc315 #xc32f
         #xc331 #xc34b #xc34d #xc367 #xc369 #xc383 #xc385 #xc39f
         #xc3a1 #xc3bb #xc3bd #xc3d7 #xc3d9 #xc3f3 #xc3f5 #xc40f
         #xc411 #xc42b #xc42d #xc447 #xc449 #xc463 #xc465 #xc47f
         #xc481 #xc49b #xc49d #xc4b7 #xc4b9 #xc4d3 #xc4d5 #xc4ef
         #xc4f1 #xc50b #xc50d #xc527 #xc529 #xc543 #xc545 #xc55f
         #xc561 #xc57b #xc57d #xc597 #xc599 #xc5b3 #xc5b5 #xc5cf
         #xc5d1 #xc5eb #xc5ed #xc607 #xc609 #xc623 #xc625 #xc63f
         #xc641 #xc65b #xc65d #xc677 #xc679 #xc693 #xc695 #xc6af
         #xc6b1 #xc6cb #xc6cd #xc6e7 #xc6e9 #xc703 #xc705 #xc71f
         #xc721 #xc73b #xc73d #xc757 #xc759 #xc773 #xc775 #xc78f
         #xc791 #xc7ab #xc7ad #xc7c7 #xc7c9 #xc7e3 #xc7e5 #xc7ff
         #xc801 #xc81b #xc81d #xc837 #xc839 #xc853 #xc855 #xc86f
         #xc871 #xc88b #xc88d #xc8a7 #xc8a9 #xc8c3 #xc8c5 #xc8df
         #xc8e1 #xc8fb #xc8fd #xc917 #xc919 #xc933 #xc935 #xc94f
         #xc951 #xc96b #xc96d #xc987 #xc989 #xc9a3 #xc9a5 #xc9bf
         #xc9c1 #xc9db #xc9dd #xc9f7 #xc9f9 #xca13 #xca15 #xca2f
         #xca31 #xca4b #xca4d #xca67 #xca69 #xca83 #xca85 #xca9f
         #xcaa1 #xcabb #xcabd #xcad7 #xcad9 #xcaf3 #xcaf5 #xcb0f
         #xcb11 #xcb2b #xcb2d #xcb47 #xcb49 #xcb63 #xcb65 #xcb7f
         #xcb81 #xcb9b #xcb9d #xcbb7 #xcbb9 #xcbd3 #xcbd5 #xcbef
         #xcbf1 #xcc0b #xcc0d #xcc27 #xcc29 #xcc43 #xcc45 #xcc5f
         #xcc61 #xcc7b #xcc7d #xcc97 #xcc99 #xccb3 #xccb5 #xcccf
         #xccd1 #xcceb #xcced #xcd07 #xcd09 #xcd23 #xcd25 #xcd3f
         #xcd41 #xcd5b #xcd5d #xcd77 #xcd79 #xcd93 #xcd95 #xcdaf
         #xcdb1 #xcdcb #xcdcd #xcde7 #xcde9 #xce03 #xce05 #xce1f
         #xce21 #xce3b #xce3d #xce57 #xce59 #xce73 #xce75 #xce8f
         #xce91 #xceab #xcead #xcec7 #xcec9 #xcee3 #xcee5 #xceff
         #xcf01 #xcf1b #xcf1d #xcf37 #xcf39 #xcf53 #xcf55 #xcf6f
         #xcf71 #xcf8b #xcf8d #xcfa7 #xcfa9 #xcfc3 #xcfc5 #xcfdf
         #xcfe1 #xcffb #xcffd #xd017 #xd019 #xd033 #xd035 #xd04f
         #xd051 #xd06b #xd06d #xd087 #xd089 #xd0a3 #xd0a5 #xd0bf
         #xd0c1 #xd0db #xd0dd #xd0f7 #xd0f9 #xd113 #xd115 #xd12f
         #xd131 #xd14b #xd14d #xd167 #xd169 #xd183 #xd185 #xd19f
         #xd1a1 #xd1bb #xd1bd #xd1d7 #xd1d9 #xd1f3 #xd1f5 #xd20f
         #xd211 #xd22b #xd22d #xd247 #xd249 #xd263 #xd265 #xd27f
         #xd281 #xd29b #xd29d #xd2b7 #xd2b9 #xd2d3 #xd2d5 #xd2ef
         #xd2f1 #xd30b #xd30d #xd327 #xd329 #xd343 #xd345 #xd35f
         #xd361 #xd37b #xd37d #xd397 #xd399 #xd3b3 #xd3b5 #xd3cf
         #xd3d1 #xd3eb #xd3ed #xd407 #xd409 #xd423 #xd425 #xd43f
         #xd441 #xd45b #xd45d #xd477 #xd479 #xd493 #xd495 #xd4af
         #xd4b1 #xd4cb #xd4cd #xd4e7 #xd4e9 #xd503 #xd505 #xd51f
         #xd521 #xd53b #xd53d #xd557 #xd559 #xd573 #xd575 #xd58f
         #xd591 #xd5ab #xd5ad #xd5c7 #xd5c9 #xd5e3 #xd5e5 #xd5ff
         #xd601 #xd61b #xd61d #xd637 #xd639 #xd653 #xd655 #xd66f
         #xd671 #xd68b #xd68d #xd6a7 #xd6a9 #xd6c3 #xd6c5 #xd6df
         #xd6e1 #xd6fb #xd6fd #xd717 #xd719 #xd733 #xd735 #xd74f
         #xd751 #xd76b #xd76d #xd787 #xd789 #xd7a3))))]
  [else ; no Unicode support
   (begin
    (define char-set:control 
      (char-set-union 
        (ucs-range->char-set 0 10) (ucs-range->char-set 11 13) (ucs-range->char-set 14 32) 
        (ucs-range->char-set 127 127)))
    (define char-set:extend-or-spacing-mark (char-set-union))
    (define char-set:regional-indicator (char-set-union))
    (define char-set:hangul-l (char-set-union))
    (define char-set:hangul-v (char-set-union))
    (define char-set:hangul-t (char-set-union))
    (define char-set:hangul-lv (char-set-union))
    (define char-set:hangul-lvt (char-set-union)))])

;;;; SPDX-FileCopyrightText: 2013 - 2016 Alex Shinn
;;;;
;;;; SPDX-License-Identifier: BSD-3-Clause

;; regexp.scm -- simple non-bactracking NFA implementation

;;; An rx represents a start state and meta-info such as the number
;;; and names of submatches.
(define-record-type Rx
  (make-rx start-state num-matches num-save-indexes non-greedy-indexes
           match-rules match-names sre)
  regexp?
  (start-state rx-start-state rx-start-state-set!)
  (num-matches rx-num-matches rx-num-matches-set!)
  (num-save-indexes rx-num-save-indexes rx-num-save-indexes-set!)
  (non-greedy-indexes rx-non-greedy-indexes rx-non-greedy-indexes-set!)
  (match-rules rx-rules rx-rules-set!)
  (match-names rx-names rx-names-set!)
  (sre regexp->sre))

;; Syntactic sugar.
(define-syntax rx
  (syntax-rules ()
    ((rx sre ...)
     (regexp `(: sre ...)))))

;;; A state is a single nfa state with transition rules.
(define-record-type State
  (%make-state accept? chars match match-rule next1 next2 id)
  state?
  ;; A boolean indicating if this is an accepting state.
  (accept? state-accept? state-accept?-set!)
  ;; A char or char-set indicating when we can transition.
  ;; Alternately, #f indicates an epsilon transition, while a
  ;; procedure of the form (lambda (ch i matches) ...) is a predicate
  ;; which should return #t if the char matches.
  (chars state-chars state-chars-set!)
  ;; A single integer indicating the match position to record.
  (match state-match state-match-set!)
  ;; The rule for merging ambiguous matches.  Can be any of: left,
  ;; right, (list i j).  Posix semantics are equivalent to using left
  ;; for the beginning of a submatch and right for the end.  List is
  ;; used to capture a list of submatch data in the current match.
  (match-rule state-match-rule state-match-rule-set!)
  ;; The destination if the char match succeeds.
  (next1 state-next1 state-next1-set!)
  ;; An optional additional transition used for forking to two states.
  (next2 state-next2 state-next2-set!)
  ;; A unique (per regexp) id for debugging.
  (id state-id))

(define (make-state accept? chars match match-rule next1 next2 id)
  (if (and next1 (not (state? next1)))
      (error "expected a state" next1))
  (if (and next2 (not (state? next2)))
      (error "expected a state" next2))
  (%make-state accept? chars match match-rule next1 next2 id))

(define ~none 0)
(define ~ci? 1)
(define ~ascii? 2)
(define ~nocapture? 4)

(define (flag-set? flags i) (= i (bitwise-and flags i)))
(define (flag-join a b) (if b (bitwise-ior a b) a))
(define (flag-clear a b) (bitwise-and a (bitwise-not b)))

(define (char-set-ci cset)
  (char-set-fold
   (lambda (ch res)
     (char-set-adjoin! (char-set-adjoin! res (char-upcase ch))
                       (char-downcase ch)))
   (char-set)
   cset))

(define (make-char-state ch flags next id)
  (if (flag-set? flags ~ci?)
      (let ((cset (cond ((char? ch) (char-set-ci (char-set ch)))
                        ((char-set? ch) (char-set-ci ch))
                        (else ch))))
        (make-state #f cset #f #f next #f id))
      (make-state #f ch #f #f next #f id)))
(define (make-fork-state next1 next2 id)
  (make-state #f #f #f #f next1 next2 id))
(define (make-epsilon-state next id)
  (make-fork-state next #f id))
(define (make-accept-state id)
  (make-state #t #f #f #f #f #f id))

;; A record holding the current match data - essentially a wrapper
;; around a vector, plus a reference to the RX for meta-info.
(define-record-type Regexp-Match
  (%make-regexp-match matches rx string)
  regexp-match?
  (matches regexp-match-matches regexp-match-matches-set!)
  (rx regexp-match-rx)
  (string regexp-match-string))

(define (regexp-match-rules md)
  (rx-rules (regexp-match-rx md)))
(define (regexp-match-names md)
  (rx-names (regexp-match-rx md)))
(define (make-regexp-match len rx str)
  (%make-regexp-match (make-vector len #f) rx str))
(define (make-regexp-match-for-rx rx str)
  (make-regexp-match (rx-num-save-indexes rx) rx str))
(define (regexp-match-count md)
  (- (quotient (vector-length (regexp-match-matches md)) 2) 1))

(define (regexp-match-name-offset md name)
  (let lp ((ls (regexp-match-names md)) (first #f))
    (cond
     ((null? ls) (or first (error "unknown match name" md name)))
     ((eq? name (caar ls))
      (if (regexp-match-submatch-start+end md (cdar ls))
          (cdar ls)
          (lp (cdr ls) (or first (cdar ls)))))
     (else (lp (cdr ls) first)))))

(define (regexp-match-ref md n)
  (vector-ref (regexp-match-matches md)
              (if (integer? n)
                  n
                  (regexp-match-name-offset md n))))

(define (regexp-match-set! md n val)
  (vector-set! (regexp-match-matches md) n val))

(define (copy-regexp-match md)
  (let* ((src (regexp-match-matches md))
         (len (vector-length src))
         (dst (make-vector len #f)))
    (do ((i 0 (+ i 1)))
        ((= i len)
         (%make-regexp-match dst (regexp-match-rx md) (regexp-match-string md)))
      (vector-set! dst i (vector-ref src i)))))

;;> Returns the matching result for the given named or indexed
;;> submatch \var{n}, possibly as a list for a submatch-list, or
;;> \scheme{#f} if not matched.

(define (regexp-match-submatch/list md n)
  (let ((n (if (integer? n) n (regexp-match-name-offset md n))))
    (cond
     ((>= n (vector-length (regexp-match-rules md)))
      #f)
     (else
      (let ((rule (vector-ref (regexp-match-rules md) n)))
        (cond
         ((pair? rule)
          (let ((start (regexp-match-ref md (car rule)))
                (end (regexp-match-ref md (cdr rule)))
                (str (regexp-match-string md)))
            (and start end (substring-cursor str start end))))
         (else
          (let ((res (regexp-match-ref md rule)))
            (if (pair? res)
                (reverse res)
                res)))))))))

;;> Returns the matching substring for the given named or indexed
;;> submatch \var{n}, or \scheme{#f} if not matched.

(define (regexp-match-submatch md n)
  (let ((res (regexp-match-submatch/list md n)))
    (if (pair? res) (car res) res)))

(define (regexp-match-submatch-start+end md n)
  (let ((n (if (integer? n) n (regexp-match-name-offset md n))))
    (and (< n (vector-length (regexp-match-rules md)))
         (let ((rule (vector-ref (regexp-match-rules md) n)))
           (if (pair? rule)
               (let ((start (regexp-match-ref md (car rule)))
                     (end (regexp-match-ref md (cdr rule)))
                     (str (regexp-match-string md)))
                 (and start end
                      (cons (string-cursor->index str start)
                            (string-cursor->index str end))))
               #f)))))

;;> Returns the start index for the given named or indexed submatch
;;> \var{n}, or \scheme{#f} if not matched.

(define (regexp-match-submatch-start md n)
  (cond ((regexp-match-submatch-start+end md n) => car) (else #f)))

;;> Returns the end index for the given named or indexed submatch
;;> \var{n}, or \scheme{#f} if not matched.

(define (regexp-match-submatch-end md n)
  (cond ((regexp-match-submatch-start+end md n) => cdr) (else #f)))

(define (regexp-match-convert recurse? md str)
  (cond
   ((vector? md)
    (let lp ((i 0) (res '()))
      (cond
       ((>= i (vector-length md))
        (reverse res))
       ((string-cursor? (vector-ref md i))
        (lp (+ i 2)
            (cons (substring-cursor str
                                    (vector-ref md i)
                                    (vector-ref md (+ i 1)))
                  res)))
       (else
        (lp (+ i 1)
            (cons (regexp-match-convert recurse? (vector-ref md i) str)
                  res))))))
   ((list? md)
    (if recurse?
        (map (lambda (x) (regexp-match-convert recurse? x str)) (reverse md))
        (regexp-match-convert recurse? (car md) str)))
   ((and (pair? md) (string-cursor? (car md)) (string-cursor? (cdr md)))
    (substring-cursor str (car md) (cdr md)))
   ((regexp-match? md)
    (regexp-match-convert
     recurse? (regexp-match-matches md) (regexp-match-string md)))
   (else
    md)))

;;> Convert an regexp-match result to a list of submatches, beginning
;;> with the full match, using \scheme{#f} for unmatched submatches.

(define (regexp-match->list md)
  (regexp-match-convert #f md #f))

;;> Convert an regexp-match result to a forest of submatches, beginning
;;> with the full match, using \scheme{#f} for unmatched submatches.

(define (regexp-match->sexp md)
  (regexp-match-convert #t md #f))

;; Collect results from a list match.
(define (match-collect md spec)
  (define (match-extract md n)
    (let* ((vec (regexp-match-matches md))
           (rules (regexp-match-rules md))
           (n-rule (vector-ref rules n))
           (rule (vector-ref rules n-rule)))
      (if (pair? rule)
          (let ((start (regexp-match-ref md (car rule)))
                (end (regexp-match-ref md (cdr rule))))
            (and start end (cons start end)))
          (regexp-match-ref md rule))))
  (let ((end (cadr spec))
        (vec (regexp-match-matches md)))
    (let lp ((i (+ 1 (car spec)))
             (ls '()))
      (if (>= i end)
          (reverse ls)
          (lp (+ i 1) (cons (match-extract md i) ls))))))

;; A searcher represents a single rx state and match information.
(define-record-type Searcher
  (make-searcher state matches)
  searcher?
  (state searcher-state searcher-state-set!)
  (matches searcher-matches searcher-matches-set!))

;; Merge two regexp-matches, preferring the leftmost-longest of their
;; matches, or shortest for non-greedy matches.
(define (regexp-match>=? m1 m2)
  (let ((non-greedy-indexes (rx-non-greedy-indexes (regexp-match-rx m1)))
        (end (- (vector-length (regexp-match-matches m1)) 1)))
    (let lp ((i 0))
      (cond
       ((>= i end)
        #t)
       ((and (eqv? (regexp-match-ref m1 i)
                   (regexp-match-ref m2 i))
             (eqv? (regexp-match-ref m1 (+ i 1))
                   (regexp-match-ref m2 (+ i 1))))
        (lp (+ i 2)))
       (else
        (not
         (and (string-cursor? (regexp-match-ref m2 i))
              (or (not (string-cursor? (regexp-match-ref m1 i)))
                  (string-cursor<? (regexp-match-ref m2 i)
                                   (regexp-match-ref m1 i))
                  ;; sanity check for incompletely advanced epsilons
                  (and (string-cursor? (regexp-match-ref m1 (+ i 1)))
                       (string-cursor<? (regexp-match-ref m1 (+ i 1))
                                        (regexp-match-ref m1 i)))
                  ((if (memq (+ i 1) non-greedy-indexes) not values)
                   (and
                    (string-cursor=? (regexp-match-ref m2 i)
                                     (regexp-match-ref m1 i))
                    (or (not (string-cursor? (regexp-match-ref m2 (+ i 1))))
                        (and (string-cursor? (regexp-match-ref m1 (+ i 1)))
                             (string-cursor>?
                              (regexp-match-ref m2 (+ i 1))
                              (regexp-match-ref m1 (+ i 1)))))))))))))))

(define (regexp-match-max m1 m2)
  (if (regexp-match>=? m1 m2) m1 m2))

;; Merge match data from sr2 into sr1, preferring the leftmost-longest
;; match in the event of a conflict.
(define (searcher-merge! sr1 sr2)
  (let ((m (regexp-match-max (searcher-matches sr1) (searcher-matches sr2))))
    (if (not (eq? m (searcher-matches sr1)))
        (searcher-matches-set! sr1 (copy-regexp-match m)))))

(define (searcher-max sr1 sr2)
  (if (or (not (searcher? sr2))
          (regexp-match>=? (searcher-matches sr1) (searcher-matches sr2)))
      sr1
      sr2))

(define (searcher-start-match sr)
  (regexp-match-ref (searcher-matches sr) 0))

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; A posse is a group of searchers.

(define (make-posse . o)
  (make-hash-table eq?))

(define posse? hash-table?)
(define (posse-empty? posse) (zero? (hash-table-size posse)))

(define (posse-ref posse sr)
  (hash-table-ref/default posse (searcher-state sr) #f))
(define (posse-add! posse sr)
  (hash-table-set! posse (searcher-state sr) sr))
(define (posse-clear! posse)
  (hash-table-walk posse (lambda (key val) (hash-table-delete! posse key))))
(define (posse-for-each proc posse)
  (hash-table-walk posse (lambda (key val) (proc val))))
(define (posse-every pred posse)
  (hash-table-fold posse (lambda (key val acc) (and acc (pred val))) #t))

(define (posse->list posse)
  (hash-table-values posse))
(define (list->posse ls)
  (let ((searchers (make-posse)))
    (for-each (lambda (sr) (posse-add! searchers sr)) ls)
    searchers))
(define (posse . args)
  (list->posse args))

(define (make-start-searcher rx str)
  (make-searcher (rx-start-state rx) (make-regexp-match-for-rx rx str)))

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Execution

;; A transition which doesn't advance the index.

(define (epsilon-state? st)
  (or (not (state-chars st))
      (procedure? (state-chars st))))

;; Match the state against a char and index.

(define (state-matches? st str i ch start end matches)
  (let ((matcher (state-chars st)))
    (cond
     ((char? matcher)
      (eqv? matcher ch))
     ((char-set? matcher)
      (char-set-contains? matcher ch))
     ((pair? matcher)
      (and (char<=? (car matcher) ch) (char<=? ch (cdr matcher))))
     ((procedure? matcher)
      (matcher str i ch start end matches))
     ((not matcher))
     (else
      (error "unknown state matcher" (state-chars st))))))

;; Advance epsilons together - if the State is newly added to the
;; group and is an epsilon state, recursively add the transition.

(define (posse-advance! new seen accept sr str i start end)
  (let advance! ((sr sr))
    (let ((st (searcher-state sr)))
      ;; Update match data.
      (cond
       ((state-match st)
        (let* ((index (state-match st))
               (matches (searcher-matches sr))
               (before (copy-regexp-match matches)))
          (cond
           ((pair? index)
            ;; Submatch list, accumulate and push.
            (let* ((prev (regexp-match-ref matches (car index)))
                   (new (cons (match-collect matches (cdr index))
                              (if (pair? prev) prev '()))))
              (regexp-match-set! matches (car index) new)))
           ((not (and (eq? 'non-greedy-left (state-match-rule st))
                      (regexp-match-ref matches index)
                      (string-cursor>=? (regexp-match-ref matches index)
                                        (regexp-match-ref matches (- index 1)))))
            (regexp-match-set! matches index i))))))
      ;; Follow transitions.
      (cond
       ((state-accept? st)
        (set-cdr! accept (searcher-max sr (cdr accept))))
       ((posse-ref seen sr)
        => (lambda (sr-prev) (searcher-merge! sr-prev sr)))
       ((epsilon-state? st)
        (let ((ch (and (string-cursor<? i end) (string-cursor-ref str i))))
          ;; Epsilon transition.  If there is a procedure matcher,
          ;; it's a guarded epsilon and needs to be checked.
          (cond
           ((state-matches? st str i ch start end (searcher-matches sr))
            (posse-add! seen sr)
            (let* ((next1 (state-next1 st))
                   (next2 (state-next2 st))
                   (matches
                    (and next2 (searcher-matches sr))))
              (cond
               (next1
                (searcher-state-set! sr next1)
                (advance! (make-searcher next1 (copy-regexp-match (searcher-matches sr))))))
              (cond
               (next2
                (let ((sr2 (make-searcher next2 (copy-regexp-match matches))))
                  (advance! sr2)))))))))
       ;; Non-special, non-epsilon searcher, add to posse.
       ((posse-ref new sr)
        ;; Merge regexp-match for existing searcher.
        => (lambda (sr-prev) (searcher-merge! sr-prev sr)))
       (else
        ;; Add new searcher.
        (posse-add! new sr))))))

;; Run so long as there is more to match.

(define (regexp-run-offsets search? rx str start end)
  (let ((rx (regexp rx))
        (epsilons (posse))
        (accept (list #f)))
    (let lp ((i start)
             (searchers1 (posse))
             (searchers2 (posse)))
      ;; Advance initial epsilons once from the first index, or every
      ;; time when searching.
      (cond
       ((or search? (string-cursor=? i start))
        (posse-advance! searchers1 epsilons accept (make-start-searcher rx str)
                        str i start end)
        (posse-clear! epsilons)))
      (cond
       ((or (string-cursor>=? i end)
            (and search?
                 (searcher? (cdr accept))
                 (let ((accept-start (searcher-start-match (cdr accept))))
                   (posse-every
                    (lambda (searcher)
                      (string-cursor>? (searcher-start-match searcher)
                                       accept-start))
                    searchers1)))
            (and (not search?)
                 (posse-empty? searchers1)))
        ;; Terminate when the string is done or there are no more
        ;; searchers.  If we terminate prematurely and are not
        ;; searching, return false.
        (and (searcher? (cdr accept))
             (let ((matches (searcher-matches (cdr accept))))
               (and (or search? (string-cursor>=? (regexp-match-ref matches 1)
                                                  end))
                    (searcher-matches (cdr accept))))))
       (else
        ;; Otherwise advance normally.
        (let ((ch (string-cursor-ref str i))
              (i2 (string-cursor-next str i)))
          (posse-for-each  ;; NOTE: non-deterministic from hash order
           (lambda (sr)
             (cond
              ((state-matches? (searcher-state sr) str i ch
                               start end (searcher-matches sr))
               (searcher-state-set! sr (state-next1 (searcher-state sr)))
               ;; Epsilons are considered at the next position.
               (posse-advance! searchers2 epsilons accept sr str i2 start end)
               (posse-clear! epsilons))))
           searchers1)
          (posse-clear! searchers1)
          (lp i2 searchers2 searchers1)))))))

;; Wrapper to determine start and end offsets.

(define (regexp-run search? rx str . o)
  (let ((start (string-start-arg str o))
        (end (string-end-arg str (if (pair? o) (cdr o) o))))
    (regexp-run-offsets search? rx str start end)))

;;> Match the given regexp or SRE against the entire string and return
;;> the match data on success.  Returns \scheme{#f} on failure.

(define (regexp-matches rx str . o)
  (apply regexp-run #f rx str o))

;;> Match the given regexp or SRE against the entire string and return
;;> the \scheme{#t} on success.  Returns \scheme{#f} on failure.

(define (regexp-matches? rx str . o)
  (and (apply regexp-matches rx str o) #t))

;;> Search for the given regexp or SRE within string and return
;;> the match data on success.  Returns \scheme{#f} on failure.

(define (regexp-search rx str . o)
  (apply regexp-run #t rx str o))

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Compiling

(define (parse-flags ls)
  (define (symbol->flag s)
    (case s ((i ci case-insensitive) ~ci?) (else ~none)))
  (let lp ((ls ls) (res ~none))
    (if (not (pair? ls))
        res
        (lp (cdr ls) (flag-join res (symbol->flag (car ls)))))))

(define char-set:nonl
  (char-set-difference char-set:full (char-set #\newline)))
(define char-set:control (ucs-range->char-set 0 32))
(define char-set:word-constituent
  (char-set-union char-set:letter char-set:digit (char-set #\_)))
(define %char-set:word-constituent
  (char-set-union %char-set:letter %char-set:digit (char-set #\_)))
(define (char-word-constituent? ch)
  (char-set-contains? char-set:word-constituent ch))
(define get-char-set:cased
  (let ((char-set:cased #f))
    (lambda ()
      (if (not char-set:cased)
          (set! char-set:cased
                (char-set-union char-set:upper-case
                                char-set:lower-case
                                char-set:title-case)))
      char-set:cased)))

(define (match/bos str i ch start end matches)
  (string-cursor=? i start))
(define (match/eos str i ch start end matches)
  (string-cursor>=? i end))
(define (match/bol str i ch start end matches)
  (or (string-cursor=? i start)
      (eqv? #\newline (string-cursor-ref str (string-cursor-prev str i)))))
(define (match/eol str i ch start end matches)
  (or (string-cursor>=? i end)
      (eqv? #\newline (string-cursor-ref str i))))
(define (match/bow str i ch start end matches)
  (and (string-cursor<? i end)
       (or (string-cursor=? i start)
           (not (char-word-constituent?
                 (string-cursor-ref str (string-cursor-prev str i)))))
       (char-word-constituent? ch)))
(define (match/eow str i ch start end matches)
  (and (or (string-cursor>=? i end)
           (not (char-word-constituent? ch)))
       (string-cursor>? i start)
       (char-word-constituent?
        (string-cursor-ref str (string-cursor-prev str i)))))
(define (match/nwb str i ch start end matches)
  (and (not (match/bow str i ch start end matches))
       (not (match/eow str i ch start end matches))))
(define (match/bog str i ch start end matches)
  (and (string-cursor<? i end)
       (or (string-cursor=? i start)
           (match/eog str (string-cursor-prev str i) ch start end matches))))
(define (match/eog str i ch start end matches)
  (and (string-cursor>? i start)
       (or (string-cursor>=? i end)
           (let* ((sci (string-cursor->index str i))
                  (sce (string-cursor->index str end))
                  (m (regexp-search re:grapheme str sci sce)))
             (and m (<= (regexp-match-submatch-end m 0) sci))))))

(define (lookup-char-set name flags)
  (cond
   ((flag-set? flags ~ascii?)
    (case name
      ((any) char-set:full)
      ((nonl) char-set:nonl)
      ((lower-case lower)
       (if (flag-set? flags ~ci?) %char-set:letter %char-set:lower-case))
      ((upper-case upper)
       (if (flag-set? flags ~ci?) %char-set:letter %char-set:upper-case))
      ((title-case title)
       (if (flag-set? flags ~ci?) %char-set:letter (char-set)))
      ((alphabetic alpha) %char-set:letter)
      ((numeric num) %char-set:digit)
      ((alphanumeric alphanum alnum) %char-set:letter+digit)
      ((punctuation punct) %char-set:punctuation)
      ((symbol) %char-set:symbol)
      ((graphic graph) %char-set:graphic)
      ((word-constituent) %char-set:word-constituent)
      ((whitespace white space) %char-set:whitespace)
      ((printing print) %char-set:printing)
      ((control cntrl) %char-set:iso-control)
      ((hex-digit xdigit hex) char-set:hex-digit)
      ((ascii) char-set:ascii)
      (else #f)))
   (else
    (case name
      ((any) char-set:full)
      ((nonl) char-set:nonl)
      ((lower-case lower)
       (if (flag-set? flags ~ci?) (get-char-set:cased) char-set:lower-case))
      ((upper-case upper)
       (if (flag-set? flags ~ci?) (get-char-set:cased) char-set:upper-case))
      ((title-case title)
       (if (flag-set? flags ~ci?) (get-char-set:cased) char-set:title-case))
      ((alphabetic alpha) char-set:letter)
      ((numeric num digit) char-set:digit)
      ((alphanumeric alphanum alnum) char-set:letter+digit)
      ((punctuation punct) char-set:punctuation)
      ((symbol) char-set:symbol)
      ((graphic graph) char-set:graphic)
      ((word-constituent) char-set:word-constituent)
      ((whitespace white space) char-set:whitespace)
      ((printing print) char-set:printing)
      ((control cntrl) char-set:control)
      ((hex-digit xdigit hex) char-set:hex-digit)
      ((ascii) char-set:ascii)
      (else #f)))))

(define (sre-flatten-ranges orig-ls)
  (let lp ((ls orig-ls) (res '()))
    (cond
     ((null? ls)
      (reverse res))
     ((string? (car ls))
      (lp (append (string->list (car ls)) (cdr ls)) res))
     ((null? (cdr ls))
      (error "unbalanced cset / range" orig-ls))
     ((string? (cadr ls))
      (lp (cons (car ls) (append (string->list (cadr ls)) (cddr ls))) res))
     (else
      (lp (cddr ls) (cons (cons (car ls) (cadr ls)) res))))))

(define (char-set-sre? sre)
  (or (char? sre)
      (and (string? sre) (= 1 (string-length sre)))
      (lookup-char-set sre ~none)
      (and (pair? sre)
           (or (string? (car sre))
               (memq (car sre)
                     '(char-set / char-range & and ~ complement - difference))
               (and (memq (car sre)
                          '(|\|| or w/case w/nocase w/unicode w/ascii))
                    (every char-set-sre? (cdr sre)))))))

(define (non-greedy-sre? sre)
  (and (pair? sre)
       (or (memq (car sre) '(?? *? **? non-greedy-optional
                             non-greedy-zero-or-more non-greedy-repeated))
           (and (memq (car sre) '(: seq w/case w/nocase w/unicode w/ascii))
                (non-greedy-sre? (car (reverse sre))))
           (and (eq? (car sre) 'or)
                (any non-greedy-sre? (cdr sre))))))

(define (valid-sre? x)
  (guard (exn (else #f)) (regexp x) #t))

(define (sre->char-set sre . o)
  (let ((flags (if (pair? o) (car o) ~none)))
    (define (->cs sre) (sre->char-set sre flags))
    (define (maybe-ci sre)
      (if (flag-set? flags ~ci?) (char-set-ci sre) sre))
    (cond
     ((lookup-char-set sre flags))
     ((char-set? sre) (maybe-ci sre))
     ((char? sre) (maybe-ci (char-set sre)))
     ((string? sre)
      (if (= 1 (string-length sre))
          (maybe-ci (string->char-set sre))
          (error "only single char strings can be char-sets")))
     ((pair? sre)
      (if (string? (car sre))
          (maybe-ci (string->char-set (car sre)))
          (case (car sre)
            ((char-set) (if (null? (cddr sre))
                            (maybe-ci (string->char-set (cadr sre)))
                            (error "(char-set) takes only one char-set" sre)))
            ((/ char-range)
             (->cs
              `(or ,@(map (lambda (x)
                            (ucs-range->char-set
                             (char->integer (car x))
                             (+ 1 (char->integer (cdr x)))))
                          (sre-flatten-ranges (cdr sre))))))
            ((& and) (apply char-set-intersection (map ->cs (cdr sre))))
            ((|\|| or) (apply char-set-union (map ->cs (cdr sre))))
            ((~ complement) (char-set-complement (->cs `(or ,@(cdr sre)))))
            ((- difference) (char-set-difference (->cs (cadr sre))
                                                 (->cs `(or ,@(cddr sre)))))
            ((w/case) (if (null? (cddr sre))
                          (sre->char-set (cadr sre) (flag-clear flags ~ci?))
                          (error "w/case takes only one char-set" sre)))
            ((w/nocase) (if (null? (cddr sre))
                            (sre->char-set (cadr sre) (flag-join flags ~ci?))
                            (error "w/nocase takes only one char-set" sre)))
            ((w/ascii) (if (null? (cddr sre))
                           (sre->char-set (cadr sre) (flag-join flags ~ascii?))
                           (error "w/ascii takes only one char-set" sre)))
            ((w/unicode) (if (null? (cddr sre))
                             (sre->char-set (cadr sre) (flag-clear flags ~ascii?))
                             (error "w/unicode takes only one char-set" sre)))
            (else (error "invalid sre char-set" sre)))))
     (else (error "invalid sre char-set" sre)))))

(define (char-set->sre cset)
  (list (char-set->string cset)))

(define (strip-submatches sre)
  (if (pair? sre)
      (case (car sre)
        (($ submatch) (strip-submatches (cons ': (cdr sre))))
        ((-> => submatch-named) (strip-submatches (cons ': (cddr sre))))
        (else (cons (strip-submatches (car sre))
                    (strip-submatches (cdr sre)))))
      sre))

(define (sre-expand-reps from to sre)
  (let ((sre0 (strip-submatches sre)))
    (let lp ((i 0) (res '(:)))
      (if (= i from)
          (cond
           ((not to)
            (reverse (cons `(* ,sre) res)))
           ((= from to)
            (reverse (cons sre (cdr res))))
           (else
            (let lp ((i (+ i 1)) (res res))
              (if (>= i to)
                  (reverse (cons `(? ,sre) res))
                  (lp (+ i 1) (cons `(? ,sre0) res))))))
          (lp (+ i 1) (cons sre0 res))))))

;;> Compile an \var{sre} into a regexp.

(define (regexp sre . o)
  (define current-index 2)
  (define current-match 0)
  (define current-id 0)
  (define match-names '())
  (define match-rules (list (cons 0 1)))
  (define non-greedy-indexes '())
  (define (next-id)
    (let ((res current-id)) (set! current-id (+ current-id 1)) res))
  (define (make-submatch-state sre flags next index)
    (let* ((n3 (make-epsilon-state next (next-id)))
           (n2 (->rx sre flags n3))
           (n1 (make-epsilon-state n2 (next-id)))
           (non-greedy? (non-greedy-sre? sre)))
      (state-match-set! n1 index)
      (state-match-rule-set! n1 'left)
      (state-match-set! n3 (+ index 1))
      (state-match-rule-set! n3 (if non-greedy? 'non-greedy-left 'right))
      (if non-greedy?
          (set! non-greedy-indexes (cons (+ index 1) non-greedy-indexes)))
      n1))
  (define (->rx sre flags next)
    (cond
     ;; The base cases chars and strings match literally.
     ((char? sre)
      (make-char-state sre flags next (next-id)))
     ((char-set? sre)
      (make-char-state sre flags next (next-id)))
     ((string? sre)
      (->rx (cons 'seq (string->list sre)) flags next))
     ((and (symbol? sre) (lookup-char-set sre flags))
      => (lambda (cset) (make-char-state cset ~none next (next-id))))
     ((symbol? sre)
      (case sre
        ((epsilon) next)
        ((bos) (make-char-state match/bos flags next (next-id)))
        ((eos) (make-char-state match/eos flags next (next-id)))
        ((bol) (make-char-state match/bol flags next (next-id)))
        ((eol) (make-char-state match/eol flags next (next-id)))
        ((bow) (make-char-state match/bow flags next (next-id)))
        ((eow) (make-char-state match/eow flags next (next-id)))
        ((nwb) (make-char-state match/nwb flags next (next-id)))
        ((bog) (make-char-state match/bog flags next (next-id)))
        ((eog) (make-char-state match/eog flags next (next-id)))
        ((grapheme)
         (->rx
          `(or (: (* ,char-set:hangul-l) (+ ,char-set:hangul-v)
                  (* ,char-set:hangul-t))
               (: (* ,char-set:hangul-l) ,char-set:hangul-v
                  (* ,char-set:hangul-v) (* ,char-set:hangul-t))
               (: (* ,char-set:hangul-l) ,char-set:hangul-lvt
                  (* ,char-set:hangul-t))
               (+ ,char-set:hangul-l)
               (+ ,char-set:hangul-t)
               (+ ,char-set:regional-indicator)
               (: "\r\n")
               (: (~ control ("\r\n"))
                  (* ,char-set:extend-or-spacing-mark))
               control)
          flags
          next))
        ((word) (->rx '(word+ any) flags next))
        (else (error "unknown sre" sre))))
     ((pair? sre)
      (case (car sre)
        ((seq :)
         ;; Sequencing.  An empty sequence jumps directly to next,
         ;; otherwise we join the first element to the sequence formed
         ;; of the remaining elements followed by next.
         (if (null? (cdr sre))
             next
             ;; Make a dummy intermediate to join the states so that
             ;; we can generate n1 first, preserving the submatch order.
             (let* ((n2 (make-epsilon-state #f (next-id)))
                    (n1 (->rx (cadr sre) flags n2))
                    (n3 (->rx (cons 'seq (cddr sre)) flags next)))
               (state-next1-set! n2 n3)
               n1)))
        ((or |\||)
         ;; Alternation.  An empty alternation always fails.
         ;; Otherwise we fork between any of the alternations, each
         ;; continuing to next.
         (cond
          ((null? (cdr sre))
           #f)
          ((char-set-sre? sre)
           (make-char-state (sre->char-set sre) flags next (next-id)))
          ((null? (cddr sre))
           (->rx (cadr sre) flags next))
          (else
           (let* ((n1 (->rx (cadr sre) flags next))
                  (n2 (->rx (cons 'or (cddr sre)) flags next)))
             (make-fork-state n1 n2 (next-id))))))
        ((? optional ?? non-greedy-optional)
         ;; Optionality.  Either match the body or fork to the next
         ;; state directly.
         (make-fork-state (->rx (cons 'seq (cdr sre)) flags next)
                          next (next-id)))
        ((* zero-or-more *? non-greedy-zero-or-more)
         ;; Repetition.  Introduce two fork states which can jump from
         ;; the end of the loop to the beginning and from the
         ;; beginning to the end (to skip the first iteration).
         (let* ((n2 (make-fork-state next #f (next-id)))
                (n1 (make-fork-state (->rx (cons 'seq (cdr sre)) flags n2)
                                     n2 (next-id))))
           (state-next2-set! n2 n1)
           n1))
        ((+ one-or-more)
         ;; One-or-more repetition.  Same as above but the first
         ;; transition is required so the rx is simpler - we only
         ;; need one fork from the end of the loop to the beginning.
         (let* ((n2 (make-fork-state next #f (next-id)))
                (n1 (->rx (cons 'seq (cdr sre)) flags n2)))
           (state-next2-set! n2 n1)
           n1))
        ((= exactly)
         ;; Exact repetition.
         (->rx (sre-expand-reps (cadr sre) (cadr sre) (cons 'seq (cddr sre)))
               flags next))
        ((>= at-least)
         ;; n-or-more repetition.
         (->rx (sre-expand-reps (cadr sre) #f (cons 'seq (cddr sre)))
               flags next))
        ((** repeated **? non-greedy-repeated)
         ;; n-to-m repetition.
         (->rx (sre-expand-reps (cadr sre) (car (cddr sre))
                                (cons 'seq (cdr (cddr sre))))
               flags next))
        ((-> submatch-named)
         ;; Named submatches just record the name for the current
         ;; match and rewrite as a non-named submatch.
         (cond
          ((flag-set? flags ~nocapture?)
           (->rx (cons 'seq (cddr sre)) flags next))
          (else
           (set! match-names
                 (cons (cons (cadr sre) (+ 1 current-match)) match-names))
           (->rx (cons 'submatch (cddr sre)) flags next))))
        (($ submatch)
         ;; A submatch wraps next with an epsilon transition before
         ;; next, setting the start and end index on the result and
         ;; wrapped next respectively.
         (cond
          ((flag-set? flags ~nocapture?)
           (->rx (cons 'seq (cdr sre)) flags next))
          (else
           (let ((num current-match)
                 (index current-index))
             (set! current-match (+ current-match 1))
             (set! current-index (+ current-index 2))
             (set! match-rules `((,index . ,(+ index 1)) ,@match-rules))
             (make-submatch-state (cons 'seq (cdr sre)) flags next index)))))
        ((~ - & / complement difference and char-range char-set)
         (make-char-state (sre->char-set sre flags) ~none next (next-id)))
        ((word)
         (->rx `(: bow ,@(cdr sre) eow) flags next))
        ((word+)
         (->rx `(word (+ ,(if (equal? '(any) (cdr sre))
                              'word-constituent
                              (char-set-intersection
                               char-set:word-constituent
                               (sre->char-set `(or ,@(cdr sre)) flags)))))
               flags
               next))
        ((w/case)
         (->rx `(: ,@(cdr sre)) (flag-clear flags ~ci?) next))
        ((w/nocase)
         (->rx `(: ,@(cdr sre)) (flag-join flags ~ci?) next))
        ((w/unicode)
         (->rx `(: ,@(cdr sre)) (flag-clear flags ~ascii?) next))
        ((w/ascii)
         (->rx `(: ,@(cdr sre)) (flag-join flags ~ascii?) next))
        ((w/nocapture)
         (->rx `(: ,@(cdr sre)) (flag-join flags ~nocapture?) next))
        (else
         (if (string? (car sre))
             (make-char-state (sre->char-set sre flags) ~none next (next-id))
             (error "unknown sre" sre)))))))
  (let ((flags (parse-flags (and (pair? o) (car o)))))
    (if (regexp? sre)
        sre
        (let ((start (make-submatch-state
                      sre flags (make-accept-state (next-id)) 0)))
          ;; (define (state->list st)
          ;;   (let ((seen (make-hash-table eq?))
          ;;         (count 0))
          ;;     (reverse
          ;;      (let lp ((st st) (res '()))
          ;;        (cond
          ;;         ((not (state? st)) res)
          ;;         ((hash-table-ref/default seen st #f) res)
          ;;         (else
          ;;          (hash-table-set! seen st count)
          ;;          (let ((orig-count count))
          ;;            (set! count (+ count 1))
          ;;            (let* ((next1 (lp (state-next1 st) '()))
          ;;                   (next2 (lp (state-next2 st) '()))
          ;;                   (this (append
          ;;                          (list (state-id st) ;;orig-count
          ;;                                (cond
          ;;                                 ((epsilon-state? st)
          ;;                                  (if (state-chars st) '? '-))
          ;;                                 ((and (char-set? (state-chars st))
          ;;                                       (< (char-set-size (state-chars st)) 5))
          ;;                                  (char-set->string (state-chars st)))
          ;;                                 ((char? (state-chars st))
          ;;                                  (string (state-chars st)))
          ;;                                 (else '+))
          ;;                                (cond
          ;;                                 ((state-next1 st) => state-id)
          ;;                                 (else #f)))
          ;;                          (if (state-next2 st)
          ;;                              (list (state-id (state-next2 st)))
          ;;                              '())
          ;;                          (if (state-match st)
          ;;                              (list (list 'm (state-match st)))
          ;;                              '()))))
          ;;              (append next2 next1 (cons this res))))))))))
          ;;(for-each (lambda (x) (write x) (newline)) (state->list start))
          (make-rx start current-match current-index non-greedy-indexes
                   (list->vector (reverse match-rules)) match-names sre)))))

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Utilities

;;> The fundamental regexp matching iterator.  Repeatedly searches
;;> \var{str} for the regexp \var{re} so long as a match can be found.
;;> On each successful match, applies \scheme{(\var{kons} \var{i}
;;> \var{regexp-match} \var{str} \var{acc})} where \var{i} is the
;;> index since the last match (beginning with
;;> \var{start}),\var{regexp-match} is the resulting match, and
;;> \var{acc} is the result of the previous \var{kons} application,
;;> beginning with \var{knil}.  When no more matches can be found,
;;> calls \var{finish} with the same arguments, except that
;;> \var{regexp-match} is \scheme{#f}.
;;>
;;> By default \var{finish} just returns \var{acc}.

(define (regexp-fold rx kons knil str . o)
  (let* ((rx (regexp rx))
         (finish (if (pair? o) (car o) (lambda (from md str acc) acc)))
         (o (if (pair? o) (cdr o) o))
         (start (string-start-arg str o))
         (end (string-end-arg str (if (pair? o) (cdr o) o))))
    (let lp ((i start)
             (from start)
             (acc knil))
      (cond
       ((and (string-cursor<? i end) (regexp-run-offsets #t rx str i end))
        => (lambda (md)
             (let ((j (regexp-match-ref md 1)))
               (lp (if (and (string-cursor=? i j) (string-cursor<? j end))
                       (string-cursor-next str j)
                       j)
                   j
                   (kons (string-cursor->index str from) md str acc)))))
       (else
        (finish (string-cursor->index str from) #f str acc))))))

;;> Extracts all non-empty substrings of \var{str} which match
;;> \var{re} between \var{start} and \var{end} as a list of strings.

(define (regexp-extract rx str . o)
  (apply regexp-fold
         rx
         (lambda (from md str a)
           (let ((s (regexp-match-submatch md 0)))
             (if (equal? s "") a (cons s a))))
         '()
         str
         (lambda (from md str a) (reverse a))
         o))

;;> Splits \var{str} into a list of strings separated by matches of
;;> \var{re}.

(define (regexp-split rx str . o)
  ;; start and end in indices passed to regexp-fold
  (let ((start (if (pair? o) (car o) 0))
        (end (if (and (pair? o) (pair? (cdr o))) (cadr o) (string-length str))))
    (regexp-fold
     rx
     (lambda (from md str a)
       (let ((i (regexp-match-submatch-start md 0))
             (j (regexp-match-submatch-end md 0)))
         (if (eqv? i j)
             a
             (cons j
                   (cons (substring str (car a) i) (cdr a))))))
     (cons start '())
     str
     (lambda (from md str a)
       (reverse (cons (substring str (car a) end) (cdr a))))
     start
     end)))

;;> Partitions \var{str} into a list of non-empty strings
;;> matching \var{re}, interspersed with the unmatched portions
;;> of the string.  The first and every odd element is an unmatched
;;> substring, which will be the empty string if \var{re} matches
;;> at the beginning of the string or end of the previous match.  The
;;> second and every even element will be a substring matching
;;> \var{re}.  If the final match ends at the end of the string,
;;> no trailing empty string will be included.  Thus, in the
;;> degenerate case where \var{str} is the empty string, the
;;> result is \scheme{("")}.

(define (regexp-partition rx str . o)
  (let ((start (if (pair? o) (car o) 0))
        (end (if (and (pair? o) (pair? (cdr o))) (cadr o) (string-length str))))
    (define (kons from md str a)
      (let ((i (regexp-match-submatch-start md 0))
            (j (regexp-match-submatch-end md 0)))
        (if (eqv? i j)
            a
            (let ((left (substring str (car a) i)))
              (cons j
                    (cons (regexp-match-submatch md 0)
                          (cons left (cdr a))))))))
    (define (final from md str a)
      (if (or (< from end) (null? (cdr a)))
          (cons (substring str (car a) end) (cdr a))
          (cdr a)))
    (reverse (regexp-fold rx kons (cons start '()) str final start end))))

;;> Returns a new string replacing the \var{count}th match of \var{re}
;;> in \var{str} the \var{subst}, where the zero-indexed \var{count}
;;> defaults to zero (i.e. the first match).  If there are not
;;> \var{count} matches, returns the selected substring unmodified.

;;> \var{subst} can be a string, an integer or symbol indicating the
;;> contents of a numbered or named submatch of \var{re},\scheme{'pre}
;;> for the substring to the left of the match, or \scheme{'post} for
;;> the substring to the right of the match.

;;> The optional parameters \var{start} and \var{end} restrict both
;;> the matching and the substitution, to the given indices, such that
;;> the result is equivalent to omitting these parameters and
;;> replacing on \scheme{(substring str start end)}. As a convenience,
;;> a value of \scheme{#f} for \var{end} is equivalent to
;;> \scheme{(string-length str)}.

(define (regexp-replace rx str subst . o)
  (let* ((start (if (and (pair? o) (car o)) (car o) 0))
         (o (if (pair? o) (cdr o) '()))
         (end (if (and (pair? o) (car o)) (car o) (string-length str)))
         (o (if (pair? o) (cdr o) '()))
         (count (if (pair? o) (car o) 0)))
    (let lp ((i start) (count count))
      (let ((m (regexp-search rx str i end)))
        (cond
         ((not m) str)
         ((positive? count)
          (lp (regexp-match-submatch-end m 0) (- count 1)))
         (else
          (string-concatenate
           (cons
            (substring str start (regexp-match-submatch-start m 0))
            (append
             (reverse (regexp-apply-match m str subst start end))
             (list (substring str (regexp-match-submatch-end m 0) end)))))))))))

;;> Equivalent to \var{regexp-replace}, but replaces all occurrences
;;> of \var{re} in \var{str}.

(define (regexp-replace-all rx str subst . o)
  (let* ((start (if (and (pair? o) (car o)) (car o) 0))
         (o (if (pair? o) (cdr o) '()))
         (end (if (and (pair? o) (car o)) (car o) (string-length str))))
    (regexp-fold
     rx
     (lambda (i m str acc)
       (let ((m-start (regexp-match-submatch-start m 0)))
         (append (regexp-apply-match m str subst start end)
                 (if (>= i m-start)
                     acc
                     (cons (substring str i m-start) acc)))))
     '()
     str
     (lambda (i m str acc)
       (let ((end (string-length str)))
         (string-concatenate-reverse
          (if (>= i end)
              acc
              (cons (substring str i end) acc)))))
     start end)))

(define (regexp-apply-match m str ls start end)
  (let lp ((ls ls) (res '()))
    (cond
     ((null? ls)
      res)
     ((not (pair? ls))
      (lp (list ls) res))
     ((integer? (car ls))
      (lp (cdr ls) (cons (or (regexp-match-submatch m (car ls)) "") res)))
     ((procedure? (car ls))
      (lp (cdr ls) (cons ((car ls) m) res)))
     ((symbol? (car ls))
      (case (car ls)
        ((pre)
         (lp (cdr ls)
             (cons (substring str start (regexp-match-submatch-start m 0))
                   res)))
        ((post)
         (lp (cdr ls)
             (cons (substring str (regexp-match-submatch-end m 0) end)
                   res)))
        (else
         (cond
          ((assq (car ls) (regexp-match-names m))
           => (lambda (x) (lp (cons (cdr x) (cdr ls)) res)))
          (else
           (error "unknown match replacement" (car ls)))))))
     (else
      (lp (cdr ls) (cons (car ls) res))))))

(define re:grapheme (regexp 'grapheme))

))
