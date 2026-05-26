;; SRFI 14: clean room implementation for SKINT [esl++]

(define-library (srfi 14)
  (import 
   (scheme base) (scheme char) (scheme case-lambda)
   (skint ivset) (only (skint hidden) immediate-hash)
   (only (skint) fx+ fxmodulo fx<=? fx>=? fxfmar))
  
  (export
   char-set? char-set= char-set<=
   char-set-hash
   char-set-cursor char-set-ref char-set-cursor-next end-of-char-set?
   char-set-fold char-set-unfold char-set-unfold!
   char-set-for-each char-set-map
   char-set-copy char-set
   list->char-set  string->char-set 
   list->char-set! string->char-set! 
   char-set-filter  ucs-range->char-set ->char-set
   char-set-filter! ucs-range->char-set!
   char-set->list char-set->string
   char-set-size char-set-count char-set-contains?
   char-set-every char-set-any
   char-set-adjoin  char-set-delete 
   char-set-adjoin! char-set-delete!
   char-set-complement   char-set-union  char-set-intersection  
   char-set-complement!  char-set-union! char-set-intersection! 
   char-set-difference   char-set-xor    char-set-diff+intersection
   char-set-difference!  char-set-xor!   char-set-diff+intersection!
   char-set:lower-case   char-set:upper-case    char-set:title-case
   char-set:letter       char-set:digit         char-set:letter+digit
   char-set:graphic      char-set:printing      char-set:whitespace
   char-set:iso-control  char-set:punctuation   char-set:symbol
   char-set:hex-digit    char-set:blank         char-set:ascii
   char-set:empty        char-set:full)

(begin

(define codepoint-count 
  (cond-expand 
    [full-unicode #x110000] 
    [else #x100]))

;; basic ivsets (generated via pre/gencsets14.scm)

(define ivs:empty #())

(define ivs:full
  (cond-expand
    [full-unicode
     #(#x0 #xd7ff #xe000 #xfdcf #xfdf0 #xfefe #xff00 #xfffd #x10000
       #x1fffd #x20000 #x2fffd #x30000 #x3fffd #x40000 #x4fffd
       #x50000 #x5fffd #x60000 #x6fffd #x70000 #x7fffd #x80000
       #x8fffd #x90000 #x9fffd #xa0000 #xafffd #xb0000 #xbfffd
       #xc0000 #xcfffd #xd0000 #xdfffd #xe0000 #xefffd #xf0000
       #xffffd #x100000 #x10fffd)]
    [else #(#x0 #xff)]))

(define ivs:lower-case
  (cond-expand
    [full-unicode
     #(#x61 #x7a #xb5 #xb5 #xdf #xf6 #xf8 #xff #x101 #x101 #x103
       #x103 #x105 #x105 #x107 #x107 #x109 #x109 #x10b #x10b #x10d
       #x10d #x10f #x10f #x111 #x111 #x113 #x113 #x115 #x115 #x117
       #x117 #x119 #x119 #x11b #x11b #x11d #x11d #x11f #x11f #x121
       #x121 #x123 #x123 #x125 #x125 #x127 #x127 #x129 #x129 #x12b
       #x12b #x12d #x12d #x12f #x12f #x131 #x131 #x133 #x133 #x135
       #x135 #x137 #x138 #x13a #x13a #x13c #x13c #x13e #x13e #x140
       #x140 #x142 #x142 #x144 #x144 #x146 #x146 #x148 #x149 #x14b
       #x14b #x14d #x14d #x14f #x14f #x151 #x151 #x153 #x153 #x155
       #x155 #x157 #x157 #x159 #x159 #x15b #x15b #x15d #x15d #x15f
       #x15f #x161 #x161 #x163 #x163 #x165 #x165 #x167 #x167 #x169
       #x169 #x16b #x16b #x16d #x16d #x16f #x16f #x171 #x171 #x173
       #x173 #x175 #x175 #x177 #x177 #x17a #x17a #x17c #x17c #x17e
       #x180 #x183 #x183 #x185 #x185 #x188 #x188 #x18c #x18d #x192
       #x192 #x195 #x195 #x199 #x19b #x19e #x19e #x1a1 #x1a1 #x1a3
       #x1a3 #x1a5 #x1a5 #x1a8 #x1a8 #x1aa #x1ab #x1ad #x1ad #x1b0
       #x1b0 #x1b4 #x1b4 #x1b6 #x1b6 #x1b9 #x1ba #x1bd #x1bf #x1c6
       #x1c6 #x1c9 #x1c9 #x1cc #x1cc #x1ce #x1ce #x1d0 #x1d0 #x1d2
       #x1d2 #x1d4 #x1d4 #x1d6 #x1d6 #x1d8 #x1d8 #x1da #x1da #x1dc
       #x1dd #x1df #x1df #x1e1 #x1e1 #x1e3 #x1e3 #x1e5 #x1e5 #x1e7
       #x1e7 #x1e9 #x1e9 #x1eb #x1eb #x1ed #x1ed #x1ef #x1f0 #x1f3
       #x1f3 #x1f5 #x1f5 #x1f9 #x1f9 #x1fb #x1fb #x1fd #x1fd #x1ff
       #x1ff #x201 #x201 #x203 #x203 #x205 #x205 #x207 #x207 #x209
       #x209 #x20b #x20b #x20d #x20d #x20f #x20f #x211 #x211 #x213
       #x213 #x215 #x215 #x217 #x217 #x219 #x219 #x21b #x21b #x21d
       #x21d #x21f #x21f #x221 #x221 #x223 #x223 #x225 #x225 #x227
       #x227 #x229 #x229 #x22b #x22b #x22d #x22d #x22f #x22f #x231
       #x231 #x233 #x239 #x23c #x23c #x23f #x240 #x242 #x242 #x247
       #x247 #x249 #x249 #x24b #x24b #x24d #x24d #x24f #x293 #x295
       #x2af #x345 #x345 #x371 #x371 #x373 #x373 #x377 #x377 #x37b
       #x37d #x390 #x390 #x3ac #x3ce #x3d0 #x3d1 #x3d5 #x3d7 #x3d9
       #x3d9 #x3db #x3db #x3dd #x3dd #x3df #x3df #x3e1 #x3e1 #x3e3
       #x3e3 #x3e5 #x3e5 #x3e7 #x3e7 #x3e9 #x3e9 #x3eb #x3eb #x3ed
       #x3ed #x3ef #x3f3 #x3f5 #x3f5 #x3f8 #x3f8 #x3fb #x3fc #x430
       #x45f #x461 #x461 #x463 #x463 #x465 #x465 #x467 #x467 #x469
       #x469 #x46b #x46b #x46d #x46d #x46f #x46f #x471 #x471 #x473
       #x473 #x475 #x475 #x477 #x477 #x479 #x479 #x47b #x47b #x47d
       #x47d #x47f #x47f #x481 #x481 #x48b #x48b #x48d #x48d #x48f
       #x48f #x491 #x491 #x493 #x493 #x495 #x495 #x497 #x497 #x499
       #x499 #x49b #x49b #x49d #x49d #x49f #x49f #x4a1 #x4a1 #x4a3
       #x4a3 #x4a5 #x4a5 #x4a7 #x4a7 #x4a9 #x4a9 #x4ab #x4ab #x4ad
       #x4ad #x4af #x4af #x4b1 #x4b1 #x4b3 #x4b3 #x4b5 #x4b5 #x4b7
       #x4b7 #x4b9 #x4b9 #x4bb #x4bb #x4bd #x4bd #x4bf #x4bf #x4c2
       #x4c2 #x4c4 #x4c4 #x4c6 #x4c6 #x4c8 #x4c8 #x4ca #x4ca #x4cc
       #x4cc #x4ce #x4cf #x4d1 #x4d1 #x4d3 #x4d3 #x4d5 #x4d5 #x4d7
       #x4d7 #x4d9 #x4d9 #x4db #x4db #x4dd #x4dd #x4df #x4df #x4e1
       #x4e1 #x4e3 #x4e3 #x4e5 #x4e5 #x4e7 #x4e7 #x4e9 #x4e9 #x4eb
       #x4eb #x4ed #x4ed #x4ef #x4ef #x4f1 #x4f1 #x4f3 #x4f3 #x4f5
       #x4f5 #x4f7 #x4f7 #x4f9 #x4f9 #x4fb #x4fb #x4fd #x4fd #x4ff
       #x4ff #x501 #x501 #x503 #x503 #x505 #x505 #x507 #x507 #x509
       #x509 #x50b #x50b #x50d #x50d #x50f #x50f #x511 #x511 #x513
       #x513 #x515 #x515 #x517 #x517 #x519 #x519 #x51b #x51b #x51d
       #x51d #x51f #x51f #x521 #x521 #x523 #x523 #x525 #x525 #x527
       #x527 #x529 #x529 #x52b #x52b #x52d #x52d #x52f #x52f #x560
       #x588 #x10d0 #x10fa #x10fd #x10ff #x13f8 #x13fd #x1c80
       #x1c88 #x1d00 #x1d2b #x1d6b #x1d77 #x1d79 #x1d9a #x1e01
       #x1e01 #x1e03 #x1e03 #x1e05 #x1e05 #x1e07 #x1e07 #x1e09
       #x1e09 #x1e0b #x1e0b #x1e0d #x1e0d #x1e0f #x1e0f #x1e11
       #x1e11 #x1e13 #x1e13 #x1e15 #x1e15 #x1e17 #x1e17 #x1e19
       #x1e19 #x1e1b #x1e1b #x1e1d #x1e1d #x1e1f #x1e1f #x1e21
       #x1e21 #x1e23 #x1e23 #x1e25 #x1e25 #x1e27 #x1e27 #x1e29
       #x1e29 #x1e2b #x1e2b #x1e2d #x1e2d #x1e2f #x1e2f #x1e31
       #x1e31 #x1e33 #x1e33 #x1e35 #x1e35 #x1e37 #x1e37 #x1e39
       #x1e39 #x1e3b #x1e3b #x1e3d #x1e3d #x1e3f #x1e3f #x1e41
       #x1e41 #x1e43 #x1e43 #x1e45 #x1e45 #x1e47 #x1e47 #x1e49
       #x1e49 #x1e4b #x1e4b #x1e4d #x1e4d #x1e4f #x1e4f #x1e51
       #x1e51 #x1e53 #x1e53 #x1e55 #x1e55 #x1e57 #x1e57 #x1e59
       #x1e59 #x1e5b #x1e5b #x1e5d #x1e5d #x1e5f #x1e5f #x1e61
       #x1e61 #x1e63 #x1e63 #x1e65 #x1e65 #x1e67 #x1e67 #x1e69
       #x1e69 #x1e6b #x1e6b #x1e6d #x1e6d #x1e6f #x1e6f #x1e71
       #x1e71 #x1e73 #x1e73 #x1e75 #x1e75 #x1e77 #x1e77 #x1e79
       #x1e79 #x1e7b #x1e7b #x1e7d #x1e7d #x1e7f #x1e7f #x1e81
       #x1e81 #x1e83 #x1e83 #x1e85 #x1e85 #x1e87 #x1e87 #x1e89
       #x1e89 #x1e8b #x1e8b #x1e8d #x1e8d #x1e8f #x1e8f #x1e91
       #x1e91 #x1e93 #x1e93 #x1e95 #x1e9d #x1e9f #x1e9f #x1ea1
       #x1ea1 #x1ea3 #x1ea3 #x1ea5 #x1ea5 #x1ea7 #x1ea7 #x1ea9
       #x1ea9 #x1eab #x1eab #x1ead #x1ead #x1eaf #x1eaf #x1eb1
       #x1eb1 #x1eb3 #x1eb3 #x1eb5 #x1eb5 #x1eb7 #x1eb7 #x1eb9
       #x1eb9 #x1ebb #x1ebb #x1ebd #x1ebd #x1ebf #x1ebf #x1ec1
       #x1ec1 #x1ec3 #x1ec3 #x1ec5 #x1ec5 #x1ec7 #x1ec7 #x1ec9
       #x1ec9 #x1ecb #x1ecb #x1ecd #x1ecd #x1ecf #x1ecf #x1ed1
       #x1ed1 #x1ed3 #x1ed3 #x1ed5 #x1ed5 #x1ed7 #x1ed7 #x1ed9
       #x1ed9 #x1edb #x1edb #x1edd #x1edd #x1edf #x1edf #x1ee1
       #x1ee1 #x1ee3 #x1ee3 #x1ee5 #x1ee5 #x1ee7 #x1ee7 #x1ee9
       #x1ee9 #x1eeb #x1eeb #x1eed #x1eed #x1eef #x1eef #x1ef1
       #x1ef1 #x1ef3 #x1ef3 #x1ef5 #x1ef5 #x1ef7 #x1ef7 #x1ef9
       #x1ef9 #x1efb #x1efb #x1efd #x1efd #x1eff #x1f07 #x1f10
       #x1f15 #x1f20 #x1f27 #x1f30 #x1f37 #x1f40 #x1f45 #x1f50
       #x1f57 #x1f60 #x1f67 #x1f70 #x1f7d #x1f80 #x1f87 #x1f90
       #x1f97 #x1fa0 #x1fa7 #x1fb0 #x1fb4 #x1fb6 #x1fb7 #x1fbe
       #x1fbe #x1fc2 #x1fc4 #x1fc6 #x1fc7 #x1fd0 #x1fd3 #x1fd6
       #x1fd7 #x1fe0 #x1fe7 #x1ff2 #x1ff4 #x1ff6 #x1ff7 #x210a
       #x210a #x210e #x210f #x2113 #x2113 #x212f #x212f #x2134
       #x2134 #x2139 #x2139 #x213c #x213d #x2146 #x2149 #x214e
       #x214e #x2170 #x217f #x2184 #x2184 #x24d0 #x24e9 #x2c30
       #x2c5f #x2c61 #x2c61 #x2c65 #x2c66 #x2c68 #x2c68 #x2c6a
       #x2c6a #x2c6c #x2c6c #x2c71 #x2c71 #x2c73 #x2c74 #x2c76
       #x2c7b #x2c81 #x2c81 #x2c83 #x2c83 #x2c85 #x2c85 #x2c87
       #x2c87 #x2c89 #x2c89 #x2c8b #x2c8b #x2c8d #x2c8d #x2c8f
       #x2c8f #x2c91 #x2c91 #x2c93 #x2c93 #x2c95 #x2c95 #x2c97
       #x2c97 #x2c99 #x2c99 #x2c9b #x2c9b #x2c9d #x2c9d #x2c9f
       #x2c9f #x2ca1 #x2ca1 #x2ca3 #x2ca3 #x2ca5 #x2ca5 #x2ca7
       #x2ca7 #x2ca9 #x2ca9 #x2cab #x2cab #x2cad #x2cad #x2caf
       #x2caf #x2cb1 #x2cb1 #x2cb3 #x2cb3 #x2cb5 #x2cb5 #x2cb7
       #x2cb7 #x2cb9 #x2cb9 #x2cbb #x2cbb #x2cbd #x2cbd #x2cbf
       #x2cbf #x2cc1 #x2cc1 #x2cc3 #x2cc3 #x2cc5 #x2cc5 #x2cc7
       #x2cc7 #x2cc9 #x2cc9 #x2ccb #x2ccb #x2ccd #x2ccd #x2ccf
       #x2ccf #x2cd1 #x2cd1 #x2cd3 #x2cd3 #x2cd5 #x2cd5 #x2cd7
       #x2cd7 #x2cd9 #x2cd9 #x2cdb #x2cdb #x2cdd #x2cdd #x2cdf
       #x2cdf #x2ce1 #x2ce1 #x2ce3 #x2ce4 #x2cec #x2cec #x2cee
       #x2cee #x2cf3 #x2cf3 #x2d00 #x2d25 #x2d27 #x2d27 #x2d2d
       #x2d2d #xa641 #xa641 #xa643 #xa643 #xa645 #xa645 #xa647
       #xa647 #xa649 #xa649 #xa64b #xa64b #xa64d #xa64d #xa64f
       #xa64f #xa651 #xa651 #xa653 #xa653 #xa655 #xa655 #xa657
       #xa657 #xa659 #xa659 #xa65b #xa65b #xa65d #xa65d #xa65f
       #xa65f #xa661 #xa661 #xa663 #xa663 #xa665 #xa665 #xa667
       #xa667 #xa669 #xa669 #xa66b #xa66b #xa66d #xa66d #xa681
       #xa681 #xa683 #xa683 #xa685 #xa685 #xa687 #xa687 #xa689
       #xa689 #xa68b #xa68b #xa68d #xa68d #xa68f #xa68f #xa691
       #xa691 #xa693 #xa693 #xa695 #xa695 #xa697 #xa697 #xa699
       #xa699 #xa69b #xa69b #xa723 #xa723 #xa725 #xa725 #xa727
       #xa727 #xa729 #xa729 #xa72b #xa72b #xa72d #xa72d #xa72f
       #xa731 #xa733 #xa733 #xa735 #xa735 #xa737 #xa737 #xa739
       #xa739 #xa73b #xa73b #xa73d #xa73d #xa73f #xa73f #xa741
       #xa741 #xa743 #xa743 #xa745 #xa745 #xa747 #xa747 #xa749
       #xa749 #xa74b #xa74b #xa74d #xa74d #xa74f #xa74f #xa751
       #xa751 #xa753 #xa753 #xa755 #xa755 #xa757 #xa757 #xa759
       #xa759 #xa75b #xa75b #xa75d #xa75d #xa75f #xa75f #xa761
       #xa761 #xa763 #xa763 #xa765 #xa765 #xa767 #xa767 #xa769
       #xa769 #xa76b #xa76b #xa76d #xa76d #xa76f #xa76f #xa771
       #xa778 #xa77a #xa77a #xa77c #xa77c #xa77f #xa77f #xa781
       #xa781 #xa783 #xa783 #xa785 #xa785 #xa787 #xa787 #xa78c
       #xa78c #xa78e #xa78e #xa791 #xa791 #xa793 #xa795 #xa797
       #xa797 #xa799 #xa799 #xa79b #xa79b #xa79d #xa79d #xa79f
       #xa79f #xa7a1 #xa7a1 #xa7a3 #xa7a3 #xa7a5 #xa7a5 #xa7a7
       #xa7a7 #xa7a9 #xa7a9 #xa7af #xa7af #xa7b5 #xa7b5 #xa7b7
       #xa7b7 #xa7b9 #xa7b9 #xa7bb #xa7bb #xa7bd #xa7bd #xa7bf
       #xa7bf #xa7c1 #xa7c1 #xa7c3 #xa7c3 #xa7c8 #xa7c8 #xa7ca
       #xa7ca #xa7d1 #xa7d1 #xa7d3 #xa7d3 #xa7d5 #xa7d5 #xa7d7
       #xa7d7 #xa7d9 #xa7d9 #xa7f6 #xa7f6 #xa7fa #xa7fa #xab30
       #xab5a #xab60 #xab68 #xab70 #xabbf #xfb00 #xfb06 #xfb13
       #xfb17 #xff41 #xff5a #x10428 #x1044f #x104d8 #x104fb #x10597
       #x105a1 #x105a3 #x105b1 #x105b3 #x105b9 #x105bb #x105bc
       #x10cc0 #x10cf2 #x118c0 #x118df #x16e60 #x16e7f #x1d41a
       #x1d433 #x1d44e #x1d454 #x1d456 #x1d467 #x1d482 #x1d49b
       #x1d4b6 #x1d4b9 #x1d4bb #x1d4bb #x1d4bd #x1d4c3 #x1d4c5
       #x1d4cf #x1d4ea #x1d503 #x1d51e #x1d537 #x1d552 #x1d56b
       #x1d586 #x1d59f #x1d5ba #x1d5d3 #x1d5ee #x1d607 #x1d622
       #x1d63b #x1d656 #x1d66f #x1d68a #x1d6a5 #x1d6c2 #x1d6da
       #x1d6dc #x1d6e1 #x1d6fc #x1d714 #x1d716 #x1d71b #x1d736
       #x1d74e #x1d750 #x1d755 #x1d770 #x1d788 #x1d78a #x1d78f
       #x1d7aa #x1d7c2 #x1d7c4 #x1d7c9 #x1d7cb #x1d7cb #x1df00
       #x1df09 #x1df0b #x1df1e #x1df25 #x1df2a #x1e922 #x1e943)]
    [else #(#x61 #x7a #xb5 #xb5 #xdf #xf6 #xf8 #xff)]))

(define ivs:upper-case
  (cond-expand
    [full-unicode
     #(#x41 #x5a #xc0 #xd6 #xd8 #xde #x100 #x100 #x102 #x102 #x104
       #x104 #x106 #x106 #x108 #x108 #x10a #x10a #x10c #x10c #x10e
       #x10e #x110 #x110 #x112 #x112 #x114 #x114 #x116 #x116 #x118
       #x118 #x11a #x11a #x11c #x11c #x11e #x11e #x120 #x120 #x122
       #x122 #x124 #x124 #x126 #x126 #x128 #x128 #x12a #x12a #x12c
       #x12c #x12e #x12e #x130 #x130 #x132 #x132 #x134 #x134 #x136
       #x136 #x139 #x139 #x13b #x13b #x13d #x13d #x13f #x13f #x141
       #x141 #x143 #x143 #x145 #x145 #x147 #x147 #x14a #x14a #x14c
       #x14c #x14e #x14e #x150 #x150 #x152 #x152 #x154 #x154 #x156
       #x156 #x158 #x158 #x15a #x15a #x15c #x15c #x15e #x15e #x160
       #x160 #x162 #x162 #x164 #x164 #x166 #x166 #x168 #x168 #x16a
       #x16a #x16c #x16c #x16e #x16e #x170 #x170 #x172 #x172 #x174
       #x174 #x176 #x176 #x178 #x179 #x17b #x17b #x17d #x17d #x181
       #x182 #x184 #x184 #x186 #x187 #x189 #x18b #x18e #x191 #x193
       #x194 #x196 #x198 #x19c #x19d #x19f #x1a0 #x1a2 #x1a2 #x1a4
       #x1a4 #x1a6 #x1a7 #x1a9 #x1a9 #x1ac #x1ac #x1ae #x1af #x1b1
       #x1b3 #x1b5 #x1b5 #x1b7 #x1b8 #x1bc #x1bc #x1c4 #x1c4 #x1c7
       #x1c7 #x1ca #x1ca #x1cd #x1cd #x1cf #x1cf #x1d1 #x1d1 #x1d3
       #x1d3 #x1d5 #x1d5 #x1d7 #x1d7 #x1d9 #x1d9 #x1db #x1db #x1de
       #x1de #x1e0 #x1e0 #x1e2 #x1e2 #x1e4 #x1e4 #x1e6 #x1e6 #x1e8
       #x1e8 #x1ea #x1ea #x1ec #x1ec #x1ee #x1ee #x1f1 #x1f1 #x1f4
       #x1f4 #x1f6 #x1f8 #x1fa #x1fa #x1fc #x1fc #x1fe #x1fe #x200
       #x200 #x202 #x202 #x204 #x204 #x206 #x206 #x208 #x208 #x20a
       #x20a #x20c #x20c #x20e #x20e #x210 #x210 #x212 #x212 #x214
       #x214 #x216 #x216 #x218 #x218 #x21a #x21a #x21c #x21c #x21e
       #x21e #x220 #x220 #x222 #x222 #x224 #x224 #x226 #x226 #x228
       #x228 #x22a #x22a #x22c #x22c #x22e #x22e #x230 #x230 #x232
       #x232 #x23a #x23b #x23d #x23e #x241 #x241 #x243 #x246 #x248
       #x248 #x24a #x24a #x24c #x24c #x24e #x24e #x370 #x370 #x372
       #x372 #x376 #x376 #x37f #x37f #x386 #x386 #x388 #x38a #x38c
       #x38c #x38e #x38f #x391 #x3a1 #x3a3 #x3ab #x3cf #x3cf #x3d2
       #x3d4 #x3d8 #x3d8 #x3da #x3da #x3dc #x3dc #x3de #x3de #x3e0
       #x3e0 #x3e2 #x3e2 #x3e4 #x3e4 #x3e6 #x3e6 #x3e8 #x3e8 #x3ea
       #x3ea #x3ec #x3ec #x3ee #x3ee #x3f4 #x3f4 #x3f7 #x3f7 #x3f9
       #x3fa #x3fd #x42f #x460 #x460 #x462 #x462 #x464 #x464 #x466
       #x466 #x468 #x468 #x46a #x46a #x46c #x46c #x46e #x46e #x470
       #x470 #x472 #x472 #x474 #x474 #x476 #x476 #x478 #x478 #x47a
       #x47a #x47c #x47c #x47e #x47e #x480 #x480 #x48a #x48a #x48c
       #x48c #x48e #x48e #x490 #x490 #x492 #x492 #x494 #x494 #x496
       #x496 #x498 #x498 #x49a #x49a #x49c #x49c #x49e #x49e #x4a0
       #x4a0 #x4a2 #x4a2 #x4a4 #x4a4 #x4a6 #x4a6 #x4a8 #x4a8 #x4aa
       #x4aa #x4ac #x4ac #x4ae #x4ae #x4b0 #x4b0 #x4b2 #x4b2 #x4b4
       #x4b4 #x4b6 #x4b6 #x4b8 #x4b8 #x4ba #x4ba #x4bc #x4bc #x4be
       #x4be #x4c0 #x4c1 #x4c3 #x4c3 #x4c5 #x4c5 #x4c7 #x4c7 #x4c9
       #x4c9 #x4cb #x4cb #x4cd #x4cd #x4d0 #x4d0 #x4d2 #x4d2 #x4d4
       #x4d4 #x4d6 #x4d6 #x4d8 #x4d8 #x4da #x4da #x4dc #x4dc #x4de
       #x4de #x4e0 #x4e0 #x4e2 #x4e2 #x4e4 #x4e4 #x4e6 #x4e6 #x4e8
       #x4e8 #x4ea #x4ea #x4ec #x4ec #x4ee #x4ee #x4f0 #x4f0 #x4f2
       #x4f2 #x4f4 #x4f4 #x4f6 #x4f6 #x4f8 #x4f8 #x4fa #x4fa #x4fc
       #x4fc #x4fe #x4fe #x500 #x500 #x502 #x502 #x504 #x504 #x506
       #x506 #x508 #x508 #x50a #x50a #x50c #x50c #x50e #x50e #x510
       #x510 #x512 #x512 #x514 #x514 #x516 #x516 #x518 #x518 #x51a
       #x51a #x51c #x51c #x51e #x51e #x520 #x520 #x522 #x522 #x524
       #x524 #x526 #x526 #x528 #x528 #x52a #x52a #x52c #x52c #x52e
       #x52e #x531 #x556 #x10a0 #x10c5 #x10c7 #x10c7 #x10cd #x10cd
       #x13a0 #x13f5 #x1c90 #x1cba #x1cbd #x1cbf #x1e00 #x1e00
       #x1e02 #x1e02 #x1e04 #x1e04 #x1e06 #x1e06 #x1e08 #x1e08
       #x1e0a #x1e0a #x1e0c #x1e0c #x1e0e #x1e0e #x1e10 #x1e10
       #x1e12 #x1e12 #x1e14 #x1e14 #x1e16 #x1e16 #x1e18 #x1e18
       #x1e1a #x1e1a #x1e1c #x1e1c #x1e1e #x1e1e #x1e20 #x1e20
       #x1e22 #x1e22 #x1e24 #x1e24 #x1e26 #x1e26 #x1e28 #x1e28
       #x1e2a #x1e2a #x1e2c #x1e2c #x1e2e #x1e2e #x1e30 #x1e30
       #x1e32 #x1e32 #x1e34 #x1e34 #x1e36 #x1e36 #x1e38 #x1e38
       #x1e3a #x1e3a #x1e3c #x1e3c #x1e3e #x1e3e #x1e40 #x1e40
       #x1e42 #x1e42 #x1e44 #x1e44 #x1e46 #x1e46 #x1e48 #x1e48
       #x1e4a #x1e4a #x1e4c #x1e4c #x1e4e #x1e4e #x1e50 #x1e50
       #x1e52 #x1e52 #x1e54 #x1e54 #x1e56 #x1e56 #x1e58 #x1e58
       #x1e5a #x1e5a #x1e5c #x1e5c #x1e5e #x1e5e #x1e60 #x1e60
       #x1e62 #x1e62 #x1e64 #x1e64 #x1e66 #x1e66 #x1e68 #x1e68
       #x1e6a #x1e6a #x1e6c #x1e6c #x1e6e #x1e6e #x1e70 #x1e70
       #x1e72 #x1e72 #x1e74 #x1e74 #x1e76 #x1e76 #x1e78 #x1e78
       #x1e7a #x1e7a #x1e7c #x1e7c #x1e7e #x1e7e #x1e80 #x1e80
       #x1e82 #x1e82 #x1e84 #x1e84 #x1e86 #x1e86 #x1e88 #x1e88
       #x1e8a #x1e8a #x1e8c #x1e8c #x1e8e #x1e8e #x1e90 #x1e90
       #x1e92 #x1e92 #x1e94 #x1e94 #x1e9e #x1e9e #x1ea0 #x1ea0
       #x1ea2 #x1ea2 #x1ea4 #x1ea4 #x1ea6 #x1ea6 #x1ea8 #x1ea8
       #x1eaa #x1eaa #x1eac #x1eac #x1eae #x1eae #x1eb0 #x1eb0
       #x1eb2 #x1eb2 #x1eb4 #x1eb4 #x1eb6 #x1eb6 #x1eb8 #x1eb8
       #x1eba #x1eba #x1ebc #x1ebc #x1ebe #x1ebe #x1ec0 #x1ec0
       #x1ec2 #x1ec2 #x1ec4 #x1ec4 #x1ec6 #x1ec6 #x1ec8 #x1ec8
       #x1eca #x1eca #x1ecc #x1ecc #x1ece #x1ece #x1ed0 #x1ed0
       #x1ed2 #x1ed2 #x1ed4 #x1ed4 #x1ed6 #x1ed6 #x1ed8 #x1ed8
       #x1eda #x1eda #x1edc #x1edc #x1ede #x1ede #x1ee0 #x1ee0
       #x1ee2 #x1ee2 #x1ee4 #x1ee4 #x1ee6 #x1ee6 #x1ee8 #x1ee8
       #x1eea #x1eea #x1eec #x1eec #x1eee #x1eee #x1ef0 #x1ef0
       #x1ef2 #x1ef2 #x1ef4 #x1ef4 #x1ef6 #x1ef6 #x1ef8 #x1ef8
       #x1efa #x1efa #x1efc #x1efc #x1efe #x1efe #x1f08 #x1f0f
       #x1f18 #x1f1d #x1f28 #x1f2f #x1f38 #x1f3f #x1f48 #x1f4d
       #x1f59 #x1f59 #x1f5b #x1f5b #x1f5d #x1f5d #x1f5f #x1f5f
       #x1f68 #x1f6f #x1fb8 #x1fbb #x1fc8 #x1fcb #x1fd8 #x1fdb
       #x1fe8 #x1fec #x1ff8 #x1ffb #x2102 #x2102 #x2107 #x2107
       #x210b #x210d #x2110 #x2112 #x2115 #x2115 #x2119 #x211d
       #x2124 #x2124 #x2126 #x2126 #x2128 #x2128 #x212a #x212d
       #x2130 #x2133 #x213e #x213f #x2145 #x2145 #x2160 #x216f
       #x2183 #x2183 #x24b6 #x24cf #x2c00 #x2c2f #x2c60 #x2c60
       #x2c62 #x2c64 #x2c67 #x2c67 #x2c69 #x2c69 #x2c6b #x2c6b
       #x2c6d #x2c70 #x2c72 #x2c72 #x2c75 #x2c75 #x2c7e #x2c80
       #x2c82 #x2c82 #x2c84 #x2c84 #x2c86 #x2c86 #x2c88 #x2c88
       #x2c8a #x2c8a #x2c8c #x2c8c #x2c8e #x2c8e #x2c90 #x2c90
       #x2c92 #x2c92 #x2c94 #x2c94 #x2c96 #x2c96 #x2c98 #x2c98
       #x2c9a #x2c9a #x2c9c #x2c9c #x2c9e #x2c9e #x2ca0 #x2ca0
       #x2ca2 #x2ca2 #x2ca4 #x2ca4 #x2ca6 #x2ca6 #x2ca8 #x2ca8
       #x2caa #x2caa #x2cac #x2cac #x2cae #x2cae #x2cb0 #x2cb0
       #x2cb2 #x2cb2 #x2cb4 #x2cb4 #x2cb6 #x2cb6 #x2cb8 #x2cb8
       #x2cba #x2cba #x2cbc #x2cbc #x2cbe #x2cbe #x2cc0 #x2cc0
       #x2cc2 #x2cc2 #x2cc4 #x2cc4 #x2cc6 #x2cc6 #x2cc8 #x2cc8
       #x2cca #x2cca #x2ccc #x2ccc #x2cce #x2cce #x2cd0 #x2cd0
       #x2cd2 #x2cd2 #x2cd4 #x2cd4 #x2cd6 #x2cd6 #x2cd8 #x2cd8
       #x2cda #x2cda #x2cdc #x2cdc #x2cde #x2cde #x2ce0 #x2ce0
       #x2ce2 #x2ce2 #x2ceb #x2ceb #x2ced #x2ced #x2cf2 #x2cf2
       #xa640 #xa640 #xa642 #xa642 #xa644 #xa644 #xa646 #xa646
       #xa648 #xa648 #xa64a #xa64a #xa64c #xa64c #xa64e #xa64e
       #xa650 #xa650 #xa652 #xa652 #xa654 #xa654 #xa656 #xa656
       #xa658 #xa658 #xa65a #xa65a #xa65c #xa65c #xa65e #xa65e
       #xa660 #xa660 #xa662 #xa662 #xa664 #xa664 #xa666 #xa666
       #xa668 #xa668 #xa66a #xa66a #xa66c #xa66c #xa680 #xa680
       #xa682 #xa682 #xa684 #xa684 #xa686 #xa686 #xa688 #xa688
       #xa68a #xa68a #xa68c #xa68c #xa68e #xa68e #xa690 #xa690
       #xa692 #xa692 #xa694 #xa694 #xa696 #xa696 #xa698 #xa698
       #xa69a #xa69a #xa722 #xa722 #xa724 #xa724 #xa726 #xa726
       #xa728 #xa728 #xa72a #xa72a #xa72c #xa72c #xa72e #xa72e
       #xa732 #xa732 #xa734 #xa734 #xa736 #xa736 #xa738 #xa738
       #xa73a #xa73a #xa73c #xa73c #xa73e #xa73e #xa740 #xa740
       #xa742 #xa742 #xa744 #xa744 #xa746 #xa746 #xa748 #xa748
       #xa74a #xa74a #xa74c #xa74c #xa74e #xa74e #xa750 #xa750
       #xa752 #xa752 #xa754 #xa754 #xa756 #xa756 #xa758 #xa758
       #xa75a #xa75a #xa75c #xa75c #xa75e #xa75e #xa760 #xa760
       #xa762 #xa762 #xa764 #xa764 #xa766 #xa766 #xa768 #xa768
       #xa76a #xa76a #xa76c #xa76c #xa76e #xa76e #xa779 #xa779
       #xa77b #xa77b #xa77d #xa77e #xa780 #xa780 #xa782 #xa782
       #xa784 #xa784 #xa786 #xa786 #xa78b #xa78b #xa78d #xa78d
       #xa790 #xa790 #xa792 #xa792 #xa796 #xa796 #xa798 #xa798
       #xa79a #xa79a #xa79c #xa79c #xa79e #xa79e #xa7a0 #xa7a0
       #xa7a2 #xa7a2 #xa7a4 #xa7a4 #xa7a6 #xa7a6 #xa7a8 #xa7a8
       #xa7aa #xa7ae #xa7b0 #xa7b4 #xa7b6 #xa7b6 #xa7b8 #xa7b8
       #xa7ba #xa7ba #xa7bc #xa7bc #xa7be #xa7be #xa7c0 #xa7c0
       #xa7c2 #xa7c2 #xa7c4 #xa7c7 #xa7c9 #xa7c9 #xa7d0 #xa7d0
       #xa7d6 #xa7d6 #xa7d8 #xa7d8 #xa7f5 #xa7f5 #xff21 #xff3a
       #x10400 #x10427 #x104b0 #x104d3 #x10570 #x1057a #x1057c
       #x1058a #x1058c #x10592 #x10594 #x10595 #x10c80 #x10cb2
       #x118a0 #x118bf #x16e40 #x16e5f #x1d400 #x1d419 #x1d434
       #x1d44d #x1d468 #x1d481 #x1d49c #x1d49c #x1d49e #x1d49f
       #x1d4a2 #x1d4a2 #x1d4a5 #x1d4a6 #x1d4a9 #x1d4ac #x1d4ae
       #x1d4b5 #x1d4d0 #x1d4e9 #x1d504 #x1d505 #x1d507 #x1d50a
       #x1d50d #x1d514 #x1d516 #x1d51c #x1d538 #x1d539 #x1d53b
       #x1d53e #x1d540 #x1d544 #x1d546 #x1d546 #x1d54a #x1d550
       #x1d56c #x1d585 #x1d5a0 #x1d5b9 #x1d5d4 #x1d5ed #x1d608
       #x1d621 #x1d63c #x1d655 #x1d670 #x1d689 #x1d6a8 #x1d6c0
       #x1d6e2 #x1d6fa #x1d71c #x1d734 #x1d756 #x1d76e #x1d790
       #x1d7a8 #x1d7ca #x1d7ca #x1e900 #x1e921 #x1f130 #x1f149
       #x1f150 #x1f169 #x1f170 #x1f189)]
    [else #(#x41 #x5a #xc0 #xd6 #xd8 #xde)]))

(define ivs:title-case
  (cond-expand
    [full-unicode
     #(#x1c5 #x1c5 #x1c8 #x1c8 #x1cb #x1cb #x1f2 #x1f2 #x1f88
       #x1f8f #x1f98 #x1f9f #x1fa8 #x1faf #x1fbc #x1fbc #x1fcc
       #x1fcc #x1ffc #x1ffc)]
    [else #()]))

(define ivs:letter
  (cond-expand
    [full-unicode
     #(#x41 #x5a #x61 #x7a #xaa #xaa #xb5 #xb5 #xba #xba #xc0 #xd6
       #xd8 #xf6 #xf8 #x2c1 #x2c6 #x2d1 #x2e0 #x2e4 #x2ec #x2ec
       #x2ee #x2ee #x345 #x345 #x370 #x374 #x376 #x377 #x37a #x37d
       #x37f #x37f #x386 #x386 #x388 #x38a #x38c #x38c #x38e #x3a1
       #x3a3 #x3f5 #x3f7 #x481 #x48a #x52f #x531 #x556 #x559 #x559
       #x560 #x588 #x5b0 #x5bd #x5bf #x5bf #x5c1 #x5c2 #x5c4 #x5c5
       #x5c7 #x5c7 #x5d0 #x5ea #x5ef #x5f2 #x610 #x61a #x620 #x657
       #x659 #x65f #x66e #x6d3 #x6d5 #x6dc #x6e1 #x6e8 #x6ed #x6ef
       #x6fa #x6fc #x6ff #x6ff #x710 #x73f #x74d #x7b1 #x7ca #x7ea
       #x7f4 #x7f5 #x7fa #x7fa #x800 #x817 #x81a #x82c #x840 #x858
       #x860 #x86a #x870 #x887 #x889 #x88e #x8a0 #x8c9 #x8d4 #x8df
       #x8e3 #x8e9 #x8f0 #x93b #x93d #x94c #x94e #x950 #x955 #x963
       #x971 #x983 #x985 #x98c #x98f #x990 #x993 #x9a8 #x9aa #x9b0
       #x9b2 #x9b2 #x9b6 #x9b9 #x9bd #x9c4 #x9c7 #x9c8 #x9cb #x9cc
       #x9ce #x9ce #x9d7 #x9d7 #x9dc #x9dd #x9df #x9e3 #x9f0 #x9f1
       #x9fc #x9fc #xa01 #xa03 #xa05 #xa0a #xa0f #xa10 #xa13 #xa28
       #xa2a #xa30 #xa32 #xa33 #xa35 #xa36 #xa38 #xa39 #xa3e #xa42
       #xa47 #xa48 #xa4b #xa4c #xa51 #xa51 #xa59 #xa5c #xa5e #xa5e
       #xa70 #xa75 #xa81 #xa83 #xa85 #xa8d #xa8f #xa91 #xa93 #xaa8
       #xaaa #xab0 #xab2 #xab3 #xab5 #xab9 #xabd #xac5 #xac7 #xac9
       #xacb #xacc #xad0 #xad0 #xae0 #xae3 #xaf9 #xafc #xb01 #xb03
       #xb05 #xb0c #xb0f #xb10 #xb13 #xb28 #xb2a #xb30 #xb32 #xb33
       #xb35 #xb39 #xb3d #xb44 #xb47 #xb48 #xb4b #xb4c #xb56 #xb57
       #xb5c #xb5d #xb5f #xb63 #xb71 #xb71 #xb82 #xb83 #xb85 #xb8a
       #xb8e #xb90 #xb92 #xb95 #xb99 #xb9a #xb9c #xb9c #xb9e #xb9f
       #xba3 #xba4 #xba8 #xbaa #xbae #xbb9 #xbbe #xbc2 #xbc6 #xbc8
       #xbca #xbcc #xbd0 #xbd0 #xbd7 #xbd7 #xc00 #xc0c #xc0e #xc10
       #xc12 #xc28 #xc2a #xc39 #xc3d #xc44 #xc46 #xc48 #xc4a #xc4c
       #xc55 #xc56 #xc58 #xc5a #xc5d #xc5d #xc60 #xc63 #xc80 #xc83
       #xc85 #xc8c #xc8e #xc90 #xc92 #xca8 #xcaa #xcb3 #xcb5 #xcb9
       #xcbd #xcc4 #xcc6 #xcc8 #xcca #xccc #xcd5 #xcd6 #xcdd #xcde
       #xce0 #xce3 #xcf1 #xcf3 #xd00 #xd0c #xd0e #xd10 #xd12 #xd3a
       #xd3d #xd44 #xd46 #xd48 #xd4a #xd4c #xd4e #xd4e #xd54 #xd57
       #xd5f #xd63 #xd7a #xd7f #xd81 #xd83 #xd85 #xd96 #xd9a #xdb1
       #xdb3 #xdbb #xdbd #xdbd #xdc0 #xdc6 #xdcf #xdd4 #xdd6 #xdd6
       #xdd8 #xddf #xdf2 #xdf3 #xe01 #xe3a #xe40 #xe46 #xe4d #xe4d
       #xe81 #xe82 #xe84 #xe84 #xe86 #xe8a #xe8c #xea3 #xea5 #xea5
       #xea7 #xeb9 #xebb #xebd #xec0 #xec4 #xec6 #xec6 #xecd #xecd
       #xedc #xedf #xf00 #xf00 #xf40 #xf47 #xf49 #xf6c #xf71 #xf83
       #xf88 #xf97 #xf99 #xfbc #x1000 #x1036 #x1038 #x1038 #x103b
       #x103f #x1050 #x108f #x109a #x109d #x10a0 #x10c5 #x10c7
       #x10c7 #x10cd #x10cd #x10d0 #x10fa #x10fc #x1248 #x124a
       #x124d #x1250 #x1256 #x1258 #x1258 #x125a #x125d #x1260
       #x1288 #x128a #x128d #x1290 #x12b0 #x12b2 #x12b5 #x12b8
       #x12be #x12c0 #x12c0 #x12c2 #x12c5 #x12c8 #x12d6 #x12d8
       #x1310 #x1312 #x1315 #x1318 #x135a #x1380 #x138f #x13a0
       #x13f5 #x13f8 #x13fd #x1401 #x166c #x166f #x167f #x1681
       #x169a #x16a0 #x16ea #x16ee #x16f8 #x1700 #x1713 #x171f
       #x1733 #x1740 #x1753 #x1760 #x176c #x176e #x1770 #x1772
       #x1773 #x1780 #x17b3 #x17b6 #x17c8 #x17d7 #x17d7 #x17dc
       #x17dc #x1820 #x1878 #x1880 #x18aa #x18b0 #x18f5 #x1900
       #x191e #x1920 #x192b #x1930 #x1938 #x1950 #x196d #x1970
       #x1974 #x1980 #x19ab #x19b0 #x19c9 #x1a00 #x1a1b #x1a20
       #x1a5e #x1a61 #x1a74 #x1aa7 #x1aa7 #x1abf #x1ac0 #x1acc
       #x1ace #x1b00 #x1b33 #x1b35 #x1b43 #x1b45 #x1b4c #x1b80
       #x1ba9 #x1bac #x1baf #x1bba #x1be5 #x1be7 #x1bf1 #x1c00
       #x1c36 #x1c4d #x1c4f #x1c5a #x1c7d #x1c80 #x1c88 #x1c90
       #x1cba #x1cbd #x1cbf #x1ce9 #x1cec #x1cee #x1cf3 #x1cf5
       #x1cf6 #x1cfa #x1cfa #x1d00 #x1dbf #x1de7 #x1df4 #x1e00
       #x1f15 #x1f18 #x1f1d #x1f20 #x1f45 #x1f48 #x1f4d #x1f50
       #x1f57 #x1f59 #x1f59 #x1f5b #x1f5b #x1f5d #x1f5d #x1f5f
       #x1f7d #x1f80 #x1fb4 #x1fb6 #x1fbc #x1fbe #x1fbe #x1fc2
       #x1fc4 #x1fc6 #x1fcc #x1fd0 #x1fd3 #x1fd6 #x1fdb #x1fe0
       #x1fec #x1ff2 #x1ff4 #x1ff6 #x1ffc #x2071 #x2071 #x207f
       #x207f #x2090 #x209c #x2102 #x2102 #x2107 #x2107 #x210a
       #x2113 #x2115 #x2115 #x2119 #x211d #x2124 #x2124 #x2126
       #x2126 #x2128 #x2128 #x212a #x212d #x212f #x2139 #x213c
       #x213f #x2145 #x2149 #x214e #x214e #x2160 #x2188 #x24b6
       #x24e9 #x2c00 #x2ce4 #x2ceb #x2cee #x2cf2 #x2cf3 #x2d00
       #x2d25 #x2d27 #x2d27 #x2d2d #x2d2d #x2d30 #x2d67 #x2d6f
       #x2d6f #x2d80 #x2d96 #x2da0 #x2da6 #x2da8 #x2dae #x2db0
       #x2db6 #x2db8 #x2dbe #x2dc0 #x2dc6 #x2dc8 #x2dce #x2dd0
       #x2dd6 #x2dd8 #x2dde #x2de0 #x2dff #x2e2f #x2e2f #x3005
       #x3007 #x3021 #x3029 #x3031 #x3035 #x3038 #x303c #x3041
       #x3096 #x309d #x309f #x30a1 #x30fa #x30fc #x30ff #x3105
       #x312f #x3131 #x318e #x31a0 #x31bf #x31f0 #x31ff #x3400
       #x4dbf #x4e00 #xa48c #xa4d0 #xa4fd #xa500 #xa60c #xa610
       #xa61f #xa62a #xa62b #xa640 #xa66e #xa674 #xa67b #xa67f
       #xa6ef #xa717 #xa71f #xa722 #xa788 #xa78b #xa7ca #xa7d0
       #xa7d1 #xa7d3 #xa7d3 #xa7d5 #xa7d9 #xa7f2 #xa805 #xa807
       #xa827 #xa840 #xa873 #xa880 #xa8c3 #xa8c5 #xa8c5 #xa8f2
       #xa8f7 #xa8fb #xa8fb #xa8fd #xa8ff #xa90a #xa92a #xa930
       #xa952 #xa960 #xa97c #xa980 #xa9b2 #xa9b4 #xa9bf #xa9cf
       #xa9cf #xa9e0 #xa9ef #xa9fa #xa9fe #xaa00 #xaa36 #xaa40
       #xaa4d #xaa60 #xaa76 #xaa7a #xaabe #xaac0 #xaac0 #xaac2
       #xaac2 #xaadb #xaadd #xaae0 #xaaef #xaaf2 #xaaf5 #xab01
       #xab06 #xab09 #xab0e #xab11 #xab16 #xab20 #xab26 #xab28
       #xab2e #xab30 #xab5a #xab5c #xab69 #xab70 #xabea #xac00
       #xd7a3 #xd7b0 #xd7c6 #xd7cb #xd7fb #xf900 #xfa6d #xfa70
       #xfad9 #xfb00 #xfb06 #xfb13 #xfb17 #xfb1d #xfb28 #xfb2a
       #xfb36 #xfb38 #xfb3c #xfb3e #xfb3e #xfb40 #xfb41 #xfb43
       #xfb44 #xfb46 #xfbb1 #xfbd3 #xfd3d #xfd50 #xfd8f #xfd92
       #xfdc7 #xfdf0 #xfdfb #xfe70 #xfe74 #xfe76 #xfefc #xff21
       #xff3a #xff41 #xff5a #xff66 #xffbe #xffc2 #xffc7 #xffca
       #xffcf #xffd2 #xffd7 #xffda #xffdc #x10000 #x1000b #x1000d
       #x10026 #x10028 #x1003a #x1003c #x1003d #x1003f #x1004d
       #x10050 #x1005d #x10080 #x100fa #x10140 #x10174 #x10280
       #x1029c #x102a0 #x102d0 #x10300 #x1031f #x1032d #x1034a
       #x10350 #x1037a #x10380 #x1039d #x103a0 #x103c3 #x103c8
       #x103cf #x103d1 #x103d5 #x10400 #x1049d #x104b0 #x104d3
       #x104d8 #x104fb #x10500 #x10527 #x10530 #x10563 #x10570
       #x1057a #x1057c #x1058a #x1058c #x10592 #x10594 #x10595
       #x10597 #x105a1 #x105a3 #x105b1 #x105b3 #x105b9 #x105bb
       #x105bc #x10600 #x10736 #x10740 #x10755 #x10760 #x10767
       #x10780 #x10785 #x10787 #x107b0 #x107b2 #x107ba #x10800
       #x10805 #x10808 #x10808 #x1080a #x10835 #x10837 #x10838
       #x1083c #x1083c #x1083f #x10855 #x10860 #x10876 #x10880
       #x1089e #x108e0 #x108f2 #x108f4 #x108f5 #x10900 #x10915
       #x10920 #x10939 #x10980 #x109b7 #x109be #x109bf #x10a00
       #x10a03 #x10a05 #x10a06 #x10a0c #x10a13 #x10a15 #x10a17
       #x10a19 #x10a35 #x10a60 #x10a7c #x10a80 #x10a9c #x10ac0
       #x10ac7 #x10ac9 #x10ae4 #x10b00 #x10b35 #x10b40 #x10b55
       #x10b60 #x10b72 #x10b80 #x10b91 #x10c00 #x10c48 #x10c80
       #x10cb2 #x10cc0 #x10cf2 #x10d00 #x10d27 #x10e80 #x10ea9
       #x10eab #x10eac #x10eb0 #x10eb1 #x10f00 #x10f1c #x10f27
       #x10f27 #x10f30 #x10f45 #x10f70 #x10f81 #x10fb0 #x10fc4
       #x10fe0 #x10ff6 #x11000 #x11045 #x11071 #x11075 #x11080
       #x110b8 #x110c2 #x110c2 #x110d0 #x110e8 #x11100 #x11132
       #x11144 #x11147 #x11150 #x11172 #x11176 #x11176 #x11180
       #x111bf #x111c1 #x111c4 #x111ce #x111cf #x111da #x111da
       #x111dc #x111dc #x11200 #x11211 #x11213 #x11234 #x11237
       #x11237 #x1123e #x11241 #x11280 #x11286 #x11288 #x11288
       #x1128a #x1128d #x1128f #x1129d #x1129f #x112a8 #x112b0
       #x112e8 #x11300 #x11303 #x11305 #x1130c #x1130f #x11310
       #x11313 #x11328 #x1132a #x11330 #x11332 #x11333 #x11335
       #x11339 #x1133d #x11344 #x11347 #x11348 #x1134b #x1134c
       #x11350 #x11350 #x11357 #x11357 #x1135d #x11363 #x11400
       #x11441 #x11443 #x11445 #x11447 #x1144a #x1145f #x11461
       #x11480 #x114c1 #x114c4 #x114c5 #x114c7 #x114c7 #x11580
       #x115b5 #x115b8 #x115be #x115d8 #x115dd #x11600 #x1163e
       #x11640 #x11640 #x11644 #x11644 #x11680 #x116b5 #x116b8
       #x116b8 #x11700 #x1171a #x1171d #x1172a #x11740 #x11746
       #x11800 #x11838 #x118a0 #x118df #x118ff #x11906 #x11909
       #x11909 #x1190c #x11913 #x11915 #x11916 #x11918 #x11935
       #x11937 #x11938 #x1193b #x1193c #x1193f #x11942 #x119a0
       #x119a7 #x119aa #x119d7 #x119da #x119df #x119e1 #x119e1
       #x119e3 #x119e4 #x11a00 #x11a32 #x11a35 #x11a3e #x11a50
       #x11a97 #x11a9d #x11a9d #x11ab0 #x11af8 #x11c00 #x11c08
       #x11c0a #x11c36 #x11c38 #x11c3e #x11c40 #x11c40 #x11c72
       #x11c8f #x11c92 #x11ca7 #x11ca9 #x11cb6 #x11d00 #x11d06
       #x11d08 #x11d09 #x11d0b #x11d36 #x11d3a #x11d3a #x11d3c
       #x11d3d #x11d3f #x11d41 #x11d43 #x11d43 #x11d46 #x11d47
       #x11d60 #x11d65 #x11d67 #x11d68 #x11d6a #x11d8e #x11d90
       #x11d91 #x11d93 #x11d96 #x11d98 #x11d98 #x11ee0 #x11ef6
       #x11f00 #x11f10 #x11f12 #x11f3a #x11f3e #x11f40 #x11fb0
       #x11fb0 #x12000 #x12399 #x12400 #x1246e #x12480 #x12543
       #x12f90 #x12ff0 #x13000 #x1342f #x13441 #x13446 #x14400
       #x14646 #x16800 #x16a38 #x16a40 #x16a5e #x16a70 #x16abe
       #x16ad0 #x16aed #x16b00 #x16b2f #x16b40 #x16b43 #x16b63
       #x16b77 #x16b7d #x16b8f #x16e40 #x16e7f #x16f00 #x16f4a
       #x16f4f #x16f87 #x16f8f #x16f9f #x16fe0 #x16fe1 #x16fe3
       #x16fe3 #x16ff0 #x16ff1 #x17000 #x187f7 #x18800 #x18cd5
       #x18d00 #x18d08 #x1aff0 #x1aff3 #x1aff5 #x1affb #x1affd
       #x1affe #x1b000 #x1b122 #x1b132 #x1b132 #x1b150 #x1b152
       #x1b155 #x1b155 #x1b164 #x1b167 #x1b170 #x1b2fb #x1bc00
       #x1bc6a #x1bc70 #x1bc7c #x1bc80 #x1bc88 #x1bc90 #x1bc99
       #x1bc9e #x1bc9e #x1d400 #x1d454 #x1d456 #x1d49c #x1d49e
       #x1d49f #x1d4a2 #x1d4a2 #x1d4a5 #x1d4a6 #x1d4a9 #x1d4ac
       #x1d4ae #x1d4b9 #x1d4bb #x1d4bb #x1d4bd #x1d4c3 #x1d4c5
       #x1d505 #x1d507 #x1d50a #x1d50d #x1d514 #x1d516 #x1d51c
       #x1d51e #x1d539 #x1d53b #x1d53e #x1d540 #x1d544 #x1d546
       #x1d546 #x1d54a #x1d550 #x1d552 #x1d6a5 #x1d6a8 #x1d6c0
       #x1d6c2 #x1d6da #x1d6dc #x1d6fa #x1d6fc #x1d714 #x1d716
       #x1d734 #x1d736 #x1d74e #x1d750 #x1d76e #x1d770 #x1d788
       #x1d78a #x1d7a8 #x1d7aa #x1d7c2 #x1d7c4 #x1d7cb #x1df00
       #x1df1e #x1df25 #x1df2a #x1e000 #x1e006 #x1e008 #x1e018
       #x1e01b #x1e021 #x1e023 #x1e024 #x1e026 #x1e02a #x1e030
       #x1e06d #x1e08f #x1e08f #x1e100 #x1e12c #x1e137 #x1e13d
       #x1e14e #x1e14e #x1e290 #x1e2ad #x1e2c0 #x1e2eb #x1e4d0
       #x1e4eb #x1e7e0 #x1e7e6 #x1e7e8 #x1e7eb #x1e7ed #x1e7ee
       #x1e7f0 #x1e7fe #x1e800 #x1e8c4 #x1e900 #x1e943 #x1e947
       #x1e947 #x1e94b #x1e94b #x1ee00 #x1ee03 #x1ee05 #x1ee1f
       #x1ee21 #x1ee22 #x1ee24 #x1ee24 #x1ee27 #x1ee27 #x1ee29
       #x1ee32 #x1ee34 #x1ee37 #x1ee39 #x1ee39 #x1ee3b #x1ee3b
       #x1ee42 #x1ee42 #x1ee47 #x1ee47 #x1ee49 #x1ee49 #x1ee4b
       #x1ee4b #x1ee4d #x1ee4f #x1ee51 #x1ee52 #x1ee54 #x1ee54
       #x1ee57 #x1ee57 #x1ee59 #x1ee59 #x1ee5b #x1ee5b #x1ee5d
       #x1ee5d #x1ee5f #x1ee5f #x1ee61 #x1ee62 #x1ee64 #x1ee64
       #x1ee67 #x1ee6a #x1ee6c #x1ee72 #x1ee74 #x1ee77 #x1ee79
       #x1ee7c #x1ee7e #x1ee7e #x1ee80 #x1ee89 #x1ee8b #x1ee9b
       #x1eea1 #x1eea3 #x1eea5 #x1eea9 #x1eeab #x1eebb #x1f130
       #x1f149 #x1f150 #x1f169 #x1f170 #x1f189 #x20000 #x2a6df
       #x2a700 #x2b739 #x2b740 #x2b81d #x2b820 #x2cea1 #x2ceb0
       #x2ebe0 #x2ebf0 #x2ee5d #x2f800 #x2fa1d #x30000 #x3134a
       #x31350 #x323af)]
    [else
     #(#x41 #x5a #x61 #x7a #xaa #xaa #xb5 #xb5 #xba #xba #xc0 #xd6
       #xd8 #xf6 #xf8 #xff)]))

(define ivs:digit
  (cond-expand
    [full-unicode
     #(#x30 #x39 #x660 #x669 #x6f0 #x6f9 #x7c0 #x7c9 #x966 #x96f
       #x9e6 #x9ef #xa66 #xa6f #xae6 #xaef #xb66 #xb6f #xbe6 #xbef
       #xc66 #xc6f #xce6 #xcef #xd66 #xd6f #xde6 #xdef #xe50 #xe59
       #xed0 #xed9 #xf20 #xf29 #x1040 #x1049 #x1090 #x1099 #x17e0
       #x17e9 #x1810 #x1819 #x1946 #x194f #x19d0 #x19d9 #x1a80
       #x1a89 #x1a90 #x1a99 #x1b50 #x1b59 #x1bb0 #x1bb9 #x1c40
       #x1c49 #x1c50 #x1c59 #xa620 #xa629 #xa8d0 #xa8d9 #xa900
       #xa909 #xa9d0 #xa9d9 #xa9f0 #xa9f9 #xaa50 #xaa59 #xabf0
       #xabf9 #xff10 #xff19 #x104a0 #x104a9 #x10d30 #x10d39 #x11066
       #x1106f #x110f0 #x110f9 #x11136 #x1113f #x111d0 #x111d9
       #x112f0 #x112f9 #x11450 #x11459 #x114d0 #x114d9 #x11650
       #x11659 #x116c0 #x116c9 #x11730 #x11739 #x118e0 #x118e9
       #x11950 #x11959 #x11c50 #x11c59 #x11d50 #x11d59 #x11da0
       #x11da9 #x11f50 #x11f59 #x16a60 #x16a69 #x16ac0 #x16ac9
       #x16b50 #x16b59 #x1d7ce #x1d7ff #x1e140 #x1e149 #x1e2f0
       #x1e2f9 #x1e4f0 #x1e4f9 #x1e950 #x1e959 #x1fbf0 #x1fbf9)]
    [else #(#x30 #x39)]))

(define ivs:hex-digit #(#x30 #x39 #x41 #x46 #x61 #x66))

(define ivs:letter+digit (ivs-union ivs:letter ivs:digit))

(define ivs:hex-digit #(#x30 #x39 #x41 #x46 #x61 #x66))

(define ivs:blank
  (cond-expand
    [full-unicode
     #(#x9 #x9 #x20 #x20 #xa0 #xa0 #x1680 #x1680 #x2000 #x200a
       #x202f #x202f #x205f #x205f #x3000 #x3000)]
    [else #(#x9 #x9 #x20 #x20 #xa0 #xa0)]))

(define ivs:whitespace
  (cond-expand
    [full-unicode
     #(#x9 #xd #x20 #x20 #x85 #x85 #xa0 #xa0 #x1680 #x1680 #x2000
       #x200a #x2028 #x2029 #x202f #x202f #x205f #x205f #x3000
       #x3000)]
    [else #(#x9 #xd #x20 #x20 #x85 #x85 #xa0 #xa0)]))

(define ivs:punctuation
  (cond-expand
    [full-unicode
     #(#x21 #x23 #x25 #x2a #x2c #x2f #x3a #x3b #x3f #x40 #x5b #x5d
       #x5f #x5f #x7b #x7b #x7d #x7d #xa1 #xa1 #xa7 #xa7 #xab #xab
       #xb6 #xb7 #xbb #xbb #xbf #xbf #x37e #x37e #x387 #x387 #x55a
       #x55f #x589 #x58a #x5be #x5be #x5c0 #x5c0 #x5c3 #x5c3 #x5c6
       #x5c6 #x5f3 #x5f4 #x609 #x60a #x60c #x60d #x61b #x61b #x61d
       #x61f #x66a #x66d #x6d4 #x6d4 #x700 #x70d #x7f7 #x7f9 #x830
       #x83e #x85e #x85e #x964 #x965 #x970 #x970 #x9fd #x9fd #xa76
       #xa76 #xaf0 #xaf0 #xc77 #xc77 #xc84 #xc84 #xdf4 #xdf4 #xe4f
       #xe4f #xe5a #xe5b #xf04 #xf12 #xf14 #xf14 #xf3a #xf3d #xf85
       #xf85 #xfd0 #xfd4 #xfd9 #xfda #x104a #x104f #x10fb #x10fb
       #x1360 #x1368 #x1400 #x1400 #x166e #x166e #x169b #x169c
       #x16eb #x16ed #x1735 #x1736 #x17d4 #x17d6 #x17d8 #x17da
       #x1800 #x180a #x1944 #x1945 #x1a1e #x1a1f #x1aa0 #x1aa6
       #x1aa8 #x1aad #x1b5a #x1b60 #x1b7d #x1b7e #x1bfc #x1bff
       #x1c3b #x1c3f #x1c7e #x1c7f #x1cc0 #x1cc7 #x1cd3 #x1cd3
       #x2010 #x2027 #x2030 #x2043 #x2045 #x2051 #x2053 #x205e
       #x207d #x207e #x208d #x208e #x2308 #x230b #x2329 #x232a
       #x2768 #x2775 #x27c5 #x27c6 #x27e6 #x27ef #x2983 #x2998
       #x29d8 #x29db #x29fc #x29fd #x2cf9 #x2cfc #x2cfe #x2cff
       #x2d70 #x2d70 #x2e00 #x2e2e #x2e30 #x2e4f #x2e52 #x2e5d
       #x3001 #x3003 #x3008 #x3011 #x3014 #x301f #x3030 #x3030
       #x303d #x303d #x30a0 #x30a0 #x30fb #x30fb #xa4fe #xa4ff
       #xa60d #xa60f #xa673 #xa673 #xa67e #xa67e #xa6f2 #xa6f7
       #xa874 #xa877 #xa8ce #xa8cf #xa8f8 #xa8fa #xa8fc #xa8fc
       #xa92e #xa92f #xa95f #xa95f #xa9c1 #xa9cd #xa9de #xa9df
       #xaa5c #xaa5f #xaade #xaadf #xaaf0 #xaaf1 #xabeb #xabeb
       #xfd3e #xfd3f #xfe10 #xfe19 #xfe30 #xfe52 #xfe54 #xfe61
       #xfe63 #xfe63 #xfe68 #xfe68 #xfe6a #xfe6b #xff01 #xff03
       #xff05 #xff0a #xff0c #xff0f #xff1a #xff1b #xff1f #xff20
       #xff3b #xff3d #xff3f #xff3f #xff5b #xff5b #xff5d #xff5d
       #xff5f #xff65 #x10100 #x10102 #x1039f #x1039f #x103d0
       #x103d0 #x1056f #x1056f #x10857 #x10857 #x1091f #x1091f
       #x1093f #x1093f #x10a50 #x10a58 #x10a7f #x10a7f #x10af0
       #x10af6 #x10b39 #x10b3f #x10b99 #x10b9c #x10ead #x10ead
       #x10f55 #x10f59 #x10f86 #x10f89 #x11047 #x1104d #x110bb
       #x110bc #x110be #x110c1 #x11140 #x11143 #x11174 #x11175
       #x111c5 #x111c8 #x111cd #x111cd #x111db #x111db #x111dd
       #x111df #x11238 #x1123d #x112a9 #x112a9 #x1144b #x1144f
       #x1145a #x1145b #x1145d #x1145d #x114c6 #x114c6 #x115c1
       #x115d7 #x11641 #x11643 #x11660 #x1166c #x116b9 #x116b9
       #x1173c #x1173e #x1183b #x1183b #x11944 #x11946 #x119e2
       #x119e2 #x11a3f #x11a46 #x11a9a #x11a9c #x11a9e #x11aa2
       #x11b00 #x11b09 #x11c41 #x11c45 #x11c70 #x11c71 #x11ef7
       #x11ef8 #x11f43 #x11f4f #x11fff #x11fff #x12470 #x12474
       #x12ff1 #x12ff2 #x16a6e #x16a6f #x16af5 #x16af5 #x16b37
       #x16b3b #x16b44 #x16b44 #x16e97 #x16e9a #x16fe2 #x16fe2
       #x1bc9f #x1bc9f #x1da87 #x1da8b #x1e95e #x1e95f)]
    [else
     #(#x21 #x23 #x25 #x2a #x2c #x2f #x3a #x3b #x3f #x40 #x5b #x5d
       #x5f #x5f #x7b #x7b #x7d #x7d #xa1 #xa1 #xa7 #xa7 #xab #xab
       #xb6 #xb7 #xbb #xbb #xbf #xbf)]))

(define ivs:symbol
  (cond-expand
    [full-unicode
     #(#x24 #x24 #x2b #x2b #x3c #x3e #x5e #x5e #x60 #x60 #x7c #x7c
       #x7e #x7e #xa2 #xa6 #xa8 #xa9 #xac #xac #xae #xb1 #xb4 #xb4
       #xb8 #xb8 #xd7 #xd7 #xf7 #xf7 #x2c2 #x2c5 #x2d2 #x2df #x2e5
       #x2eb #x2ed #x2ed #x2ef #x2ff #x375 #x375 #x384 #x385 #x3f6
       #x3f6 #x482 #x482 #x58d #x58f #x606 #x608 #x60b #x60b #x60e
       #x60f #x6de #x6de #x6e9 #x6e9 #x6fd #x6fe #x7f6 #x7f6 #x7fe
       #x7ff #x888 #x888 #x9f2 #x9f3 #x9fa #x9fb #xaf1 #xaf1 #xb70
       #xb70 #xbf3 #xbfa #xc7f #xc7f #xd4f #xd4f #xd79 #xd79 #xe3f
       #xe3f #xf01 #xf03 #xf13 #xf13 #xf15 #xf17 #xf1a #xf1f #xf34
       #xf34 #xf36 #xf36 #xf38 #xf38 #xfbe #xfc5 #xfc7 #xfcc #xfce
       #xfcf #xfd5 #xfd8 #x109e #x109f #x1390 #x1399 #x166d #x166d
       #x17db #x17db #x1940 #x1940 #x19de #x19ff #x1b61 #x1b6a
       #x1b74 #x1b7c #x1fbd #x1fbd #x1fbf #x1fc1 #x1fcd #x1fcf
       #x1fdd #x1fdf #x1fed #x1fef #x1ffd #x1ffe #x2044 #x2044
       #x2052 #x2052 #x207a #x207c #x208a #x208c #x20a0 #x20c0
       #x2100 #x2101 #x2103 #x2106 #x2108 #x2109 #x2114 #x2114
       #x2116 #x2118 #x211e #x2123 #x2125 #x2125 #x2127 #x2127
       #x2129 #x2129 #x212e #x212e #x213a #x213b #x2140 #x2144
       #x214a #x214d #x214f #x214f #x218a #x218b #x2190 #x2307
       #x230c #x2328 #x232b #x2426 #x2440 #x244a #x249c #x24e9
       #x2500 #x2767 #x2794 #x27c4 #x27c7 #x27e5 #x27f0 #x2982
       #x2999 #x29d7 #x29dc #x29fb #x29fe #x2b73 #x2b76 #x2b95
       #x2b97 #x2bff #x2ce5 #x2cea #x2e50 #x2e51 #x2e80 #x2e99
       #x2e9b #x2ef3 #x2f00 #x2fd5 #x2ff0 #x2fff #x3004 #x3004
       #x3012 #x3013 #x3020 #x3020 #x3036 #x3037 #x303e #x303f
       #x309b #x309c #x3190 #x3191 #x3196 #x319f #x31c0 #x31e3
       #x31ef #x31ef #x3200 #x321e #x322a #x3247 #x3250 #x3250
       #x3260 #x327f #x328a #x32b0 #x32c0 #x33ff #x4dc0 #x4dff
       #xa490 #xa4c6 #xa700 #xa716 #xa720 #xa721 #xa789 #xa78a
       #xa828 #xa82b #xa836 #xa839 #xaa77 #xaa79 #xab5b #xab5b
       #xab6a #xab6b #xfb29 #xfb29 #xfbb2 #xfbc2 #xfd40 #xfd4f
       #xfdcf #xfdcf #xfdfc #xfdff #xfe62 #xfe62 #xfe64 #xfe66
       #xfe69 #xfe69 #xff04 #xff04 #xff0b #xff0b #xff1c #xff1e
       #xff3e #xff3e #xff40 #xff40 #xff5c #xff5c #xff5e #xff5e
       #xffe0 #xffe6 #xffe8 #xffee #xfffc #xfffd #x10137 #x1013f
       #x10179 #x10189 #x1018c #x1018e #x10190 #x1019c #x101a0
       #x101a0 #x101d0 #x101fc #x10877 #x10878 #x10ac8 #x10ac8
       #x1173f #x1173f #x11fd5 #x11ff1 #x16b3c #x16b3f #x16b45
       #x16b45 #x1bc9c #x1bc9c #x1cf50 #x1cfc3 #x1d000 #x1d0f5
       #x1d100 #x1d126 #x1d129 #x1d164 #x1d16a #x1d16c #x1d183
       #x1d184 #x1d18c #x1d1a9 #x1d1ae #x1d1ea #x1d200 #x1d241
       #x1d245 #x1d245 #x1d300 #x1d356 #x1d6c1 #x1d6c1 #x1d6db
       #x1d6db #x1d6fb #x1d6fb #x1d715 #x1d715 #x1d735 #x1d735
       #x1d74f #x1d74f #x1d76f #x1d76f #x1d789 #x1d789 #x1d7a9
       #x1d7a9 #x1d7c3 #x1d7c3 #x1d800 #x1d9ff #x1da37 #x1da3a
       #x1da6d #x1da74 #x1da76 #x1da83 #x1da85 #x1da86 #x1e14f
       #x1e14f #x1e2ff #x1e2ff #x1ecac #x1ecac #x1ecb0 #x1ecb0
       #x1ed2e #x1ed2e #x1eef0 #x1eef1 #x1f000 #x1f02b #x1f030
       #x1f093 #x1f0a0 #x1f0ae #x1f0b1 #x1f0bf #x1f0c1 #x1f0cf
       #x1f0d1 #x1f0f5 #x1f10d #x1f1ad #x1f1e6 #x1f202 #x1f210
       #x1f23b #x1f240 #x1f248 #x1f250 #x1f251 #x1f260 #x1f265
       #x1f300 #x1f6d7 #x1f6dc #x1f6ec #x1f6f0 #x1f6fc #x1f700
       #x1f776 #x1f77b #x1f7d9 #x1f7e0 #x1f7eb #x1f7f0 #x1f7f0
       #x1f800 #x1f80b #x1f810 #x1f847 #x1f850 #x1f859 #x1f860
       #x1f887 #x1f890 #x1f8ad #x1f8b0 #x1f8b1 #x1f900 #x1fa53
       #x1fa60 #x1fa6d #x1fa70 #x1fa7c #x1fa80 #x1fa88 #x1fa90
       #x1fabd #x1fabf #x1fac5 #x1face #x1fadb #x1fae0 #x1fae8
       #x1faf0 #x1faf8 #x1fb00 #x1fb92 #x1fb94 #x1fbca)]
    [else
     #(#x24 #x24 #x2b #x2b #x3c #x3e #x5e #x5e #x60 #x60 #x7c #x7c
       #x7e #x7e #xa2 #xa6 #xa8 #xa9 #xac #xac #xae #xb1 #xb4 #xb4
       #xb8 #xb8 #xd7 #xd7 #xf7 #xf7)]))

(define ivs:graphic
  (ivs-union ivs:letter+digit
    (ivs-union ivs:punctuation ivs:symbol)))

(define ivs:printing (ivs-union ivs:graphic ivs:whitespace))

(define ivs:iso-control #(#x0 #x1f #x7f #x9f))

(define ivs:ascii #(#x0 #x7f))


;; Record type representing a character set
(define-record-type <char-set>
  (ivs->char-set ivs)
  char-set?
  (ivs char-set->ivs))

;; Set equality: comparing multiple character sets
(define (char-set= . css)
  (or (null? css)
      (let loop ([first (car css)] [rest (cdr css)])
        (or (null? rest)
            (and (ivs=? (char-set->ivs first) (char-set->ivs (car rest)))
                 (loop (car rest) (cdr rest)))))))

;; Subset relation: cs1 <= cs2 <= ...
(define (char-set<= . css)
  (or (null? css)
      (let loop ([first (car css)] [rest (cdr css)])
        (or (null? rest)
            (and (ivs<=? (char-set->ivs first) (char-set->ivs (car rest)))
                 (loop (car rest) (cdr rest)))))))

(define char-set-hash 
  (case-lambda 
    [(cs) (char-set-hash cs 0)]
    [(cs bound)
     (let* ([v (char-set->ivs cs)] [len (vector-length v)]
            [r (if (fx<=? bound 0) 15760399 bound)] [h (fxmodulo 571 r)])
       ; use Skint's fixnum fused-multiply-add-remainder builtin
       (do ([i 0 (fx+ i 1)] [h h (fxfmar h 33 (immediate-hash (vector-ref v i)) r)]) 
         [(fx>=? i len) h]))]))


;; Cursor interface
;; A cursor is represented as an interval list (ivl): ((first . last) ...)
(define (char-set-cursor cset)
  (ivs->ivl (char-set->ivs cset)))
(define (char-set-ref cset cursor)
  (integer->char (caar cursor)))
(define (char-set-cursor-next cset cursor)
  (let* ([rng (car cursor)] [first (car rng)] [last (cdr rng)])
    (if (< first last)
        (cons (cons (+ first 1) last) (cdr cursor))
        (cdr cursor))))
(define (end-of-char-set? cursor)
  (null? cursor))

;; Fundamental iterator
(define (char-set-fold kons knil cs)
  (let loop ([cur (char-set-cursor cs)] [val knil])
    (if (end-of-char-set? cur)
        val
        (loop (char-set-cursor-next cs cur)
              (kons (char-set-ref cs cur) val)))))

;; Fundamental constructor
(define char-set-unfold
  (case-lambda
    [(p f g seed)
     (char-set-unfold p f g seed char-set:empty)]
    [(p f g seed base-cs)
     (let loop ([seed seed] [acc '()])
       (if (p seed)
           (let ([added-ivs (list->ivs acc)])
             (ivs->char-set (ivs-union (char-set->ivs base-cs) added-ivs)))
           (loop (g seed) (cons (char->integer (f seed)) acc))))]))

(define char-set-unfold! char-set-unfold)

;; Iteration
(define (char-set-for-each proc cs)
  (let loop ([cur (char-set-cursor cs)])
    (unless (end-of-char-set? cur)
      (proc (char-set-ref cs cur))
      (loop (char-set-cursor-next cs cur)))))

;; Mapping
;; fixme: not optimized for memory; may produce huge intermediate lists!
#;(define (char-set-map proc cs)
  (let ([ints '()])
    (char-set-for-each
      (lambda (c) (set! ints (cons (char->integer (proc c)) ints)))
      cs)
    (ivs->char-set (list->ivs ints))))
(define (char-set-map proc cs)
  (let ([chunk-limit 1000]) ; do it in chunks to have a chance to save cons memory
    (let loop ([cur (char-set-cursor cs)] [acc-ivs ivs:empty] [chunk '()] [count 0])
      (if (end-of-char-set? cur)
          (ivs->char-set (if (null? chunk) acc-ivs (ivs-union acc-ivs (list->ivs chunk))))        
          (let* ([c (char-set-ref cs cur)] [mc (char->integer (proc c))]
                 [new-chunk (cons mc chunk)] [new-count (+ count 1)]
                 [next-cur (char-set-cursor-next cs cur)])
            (if (= new-count chunk-limit)
                (loop next-cur (ivs-union acc-ivs (list->ivs new-chunk)) '() 0)
                (loop next-cur acc-ivs new-chunk new-count)))))))

;; Since our char-sets are immutable, copy is an identity operation
(define (char-set-copy cs)
  cs)


;; Constructors

(define (char-set . chars)
  (ivs->char-set (list->ivs (map char->integer chars))))

(define list->char-set
  (case-lambda
    [(chars)
     (ivs->char-set (list->ivs (map char->integer chars)))]
    [(chars base-cs)
     (ivs->char-set 
       (ivs-union (char-set->ivs base-cs)
                  (list->ivs (map char->integer chars))))]))

(define list->char-set! list->char-set)

(define string->char-set
  (case-lambda
    [(s) (list->char-set (string->list s))]
    [(s base-cs) (list->char-set (string->list s) base-cs)]))

(define string->char-set! string->char-set)


;; Filtering ranges list without intermediate list generation

(define (filter-ivl pred ivl)
  (let loop ([ivl ivl] [cstart #f] [cend #f] [acc '()])
    (cond
      [(null? ivl)
       (if cstart
           (reverse (cons (cons cstart cend) acc))
           (reverse acc))]
      [else
       (let* ([rng (car ivl)] [first (car rng)] [last (cdr rng)])
         (let lp ([i first] [cs cstart] [ce cend] [a acc])
           (cond [(> i last)
                  (loop (cdr ivl) cs ce a)]
                 [(pred (integer->char i))
                  (if cs
                      (lp (+ i 1) cs i a)
                      (lp (+ i 1) i i a))]
                 [else
                  (if cs
                      (lp (+ i 1) #f #f (cons (cons cs ce) a))
                      (lp (+ i 1) #f #f a))])))])))

(define char-set-filter
  (case-lambda
    [(pred cs)
     (let ([ivl (ivs->ivl (char-set->ivs cs))])
       (ivs->char-set (ivl->ivs (filter-ivl pred ivl))))]
    [(pred cs base-cs)
     (let ([ivl (ivs->ivl (char-set->ivs cs))])
       (ivs->char-set 
         (ivs-union (char-set->ivs base-cs)
           (ivl->ivs (filter-ivl pred ivl)))))]))

(define char-set-filter! char-set-filter)

(define ucs-range->char-set
  (case-lambda
    [(lower upper)
     (ucs-range->char-set lower upper #f char-set:empty)]
    [(lower upper error?)
     (ucs-range->char-set lower upper error? char-set:empty)]
    [(lower upper error? base-cs)
     (let* ([range-ivs (if (< lower upper)
                           (ivl->ivs (list (cons lower (- upper 1))))
                           ivs:empty)])
       (when (and error? (> (ivs-size (ivs-difference range-ivs ivs:full)) 0))
         (error "ucs-range->char-set: requested range contains invalid characters for this implementation"))
       (let ([valid-ivs (ivs-intersection range-ivs ivs:full)])
         (ivs->char-set (ivs-union (char-set->ivs base-cs) valid-ivs))))]))

(define ucs-range->char-set! ucs-range->char-set)

;; Coercion helper
(define (->char-set x)
  (cond [(char-set? x) x]
        [(char? x) (char-set x)]
        [(string? x) (string->char-set x)]
        ; extended to accept ivs sets (vectors)
        ; this hack is used in other Skint SRFI implementations
        [(vector? x) (ivs->char-set x)]
        [else (error "->char-set: coercion not supported for" x)]))

;; Set size
(define (char-set-size cs)
  (ivs-size (char-set->ivs cs)))

;; Element counting with predicate
(define (char-set-count pred cs)
  (char-set-fold (lambda (c acc) (if (pred c) (+ acc 1) acc)) 0 cs))

;; Convert to list of characters
(define (char-set->list cs)
  (char-set-fold cons '() cs))

;; Convert to string
(define (char-set->string cs)
  (list->string (char-set->list cs)))

;; Membership query
(define (char-set-contains? cs char)
  (ivs-contains? (char-set->ivs cs) (char->integer char)))

;; Universal quantifier
(define (char-set-every pred cs)
  (let loop ([cur (char-set-cursor cs)])
    (or (end-of-char-set? cur)
        (and (pred (char-set-ref cs cur))
             (loop (char-set-cursor-next cs cur))))))

;; Existential quantifier
(define (char-set-any pred cs)
  (let loop ([cur (char-set-cursor cs)])
    (and (not (end-of-char-set? cur))
         (or (pred (char-set-ref cs cur))
             (loop (char-set-cursor-next cs cur))))))

;; Adding elements
(define (char-set-adjoin cs . chars)
  (ivs->char-set 
    (ivs-union (char-set->ivs cs)
      (list->ivs (map char->integer chars)))))

(define char-set-adjoin! char-set-adjoin)

;; Deleting elements
(define (char-set-delete cs . chars)
  (ivs->char-set 
    (ivs-difference (char-set->ivs cs)
       (list->ivs (map char->integer chars)))))

(define char-set-delete! char-set-delete)

;; Set complement
(define (char-set-complement cs)
  (char-set-difference char-set:full cs))

(define char-set-complement! char-set-complement)

;; Set union
(define (char-set-union . css)
  (if (null? css)
      char-set:empty
      (let loop ([res (char-set->ivs (car css))] [rest (cdr css)])
        (if (null? rest)
            (ivs->char-set res)
            (loop (ivs-union res (char-set->ivs (car rest))) (cdr rest))))))

(define char-set-union! char-set-union)

;; Set intersection
(define (char-set-intersection . css)
  (if (null? css)
      char-set:full
      (let loop ([res (char-set->ivs (car css))] [rest (cdr css)])
        (if (null? rest)
            (ivs->char-set res)
            (loop (ivs-intersection res (char-set->ivs (car rest))) (cdr rest))))))

(define char-set-intersection! char-set-intersection)

;; Set difference
(define (char-set-difference cs1 . css)
  (if (null? css)
      cs1
      (let loop ([res (char-set->ivs cs1)] [rest css])
        (if (null? rest)
            (ivs->char-set res)
            (loop (ivs-difference res (char-set->ivs (car rest))) (cdr rest))))))

(define char-set-difference! char-set-difference)

;; Set symmetric difference (xor)
(define (char-set-xor . css)
  (if (null? css)
      char-set:empty
      (let loop ([res (char-set->ivs (car css))] [rest (cdr css)])
        (if (null? rest)
            (ivs->char-set res)
            (loop (ivs-difference (ivs-union res (char-set->ivs (car rest)))
                                  (ivs-intersection res (char-set->ivs (car rest))))
                  (cdr rest))))))

(define char-set-xor! char-set-xor)

;; Partitioning difference and intersection
(define (char-set-diff+intersection cs1 . css)
  (if (null? css)
      (values cs1 (ivs->char-set ivs:empty))
      (let* ([others (apply char-set-union css)]
             [diff (char-set-difference cs1 others)]
             [inter (char-set-intersection cs1 others)])
        (values diff inter))))

(define char-set-diff+intersection! char-set-diff+intersection)


;; exported char sets

(define char-set:lower-case (ivs->char-set ivs:lower-case))
(define char-set:upper-case (ivs->char-set ivs:upper-case))
(define char-set:title-case (ivs->char-set ivs:title-case))
(define char-set:letter (ivs->char-set ivs:letter))
(define char-set:digit (ivs->char-set ivs:digit))
(define char-set:letter+digit (ivs->char-set ivs:letter+digit))
(define char-set:graphic (ivs->char-set ivs:graphic))
(define char-set:printing (ivs->char-set ivs:printing))
(define char-set:whitespace (ivs->char-set ivs:whitespace))
(define char-set:iso-control (ivs->char-set ivs:iso-control))
(define char-set:punctuation (ivs->char-set ivs:punctuation))
(define char-set:symbol (ivs->char-set ivs:symbol))
(define char-set:hex-digit (ivs->char-set ivs:hex-digit))
(define char-set:blank (ivs->char-set ivs:blank))
(define char-set:ascii (ivs->char-set ivs:ascii))
(define char-set:empty (ivs->char-set ivs:empty))
(define char-set:full (ivs->char-set ivs:full))

))