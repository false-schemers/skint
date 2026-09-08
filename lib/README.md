This directory contains additional external libraries in a form of .sld files. The `srfi` subtree contains SKINT ports of the following SRFIs:

0
1
2
4*
5
6
8
9
11
13
14
16
17
19+
22
23
25
26
27*
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
54*
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
74*
78
86
87
95
98
111
113
115+
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
160+
162
165
171
172
173
175
176
189
193
194+
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
252+
253
257
264
273

Note: SRFIs marked with an asterisk are implemented in abridged form: parts that require support for either full Unicode or a full numerical
tower (both features are not required by R7RS Small) are omitted. SRFIs marked with a plus are implemented in abridged form in basic 
configuration, but automatically provide full functionality if the corresponding option (`OPT_TOWER` / `OPT_UNICODE`) was configured
or selected at SKINT build time. SRFI-19 is exact-integer-closed under OPT_TOWER configurations, falling back to inexact integers in tower-less ones. 

The `skint` subtree contains SKINT-specific libraries:

| Library | Contents | Documentation |
|---|---|---|
| `(skint format)` | `format`, `fprintf`, `printf` and their control parameters | [doc/skint/format.md](../doc/skint/format.md) |
| `(skint getopt)` | AT&T/POSIX-style command line option parser and `--help` formatter | [doc/skint/getopt.md](../doc/skint/getopt.md) |
| `(skint ivset)` | Sets of exact integers as sorted vectors of disjoint intervals | [doc/skint/ivset.md](../doc/skint/ivset.md) |
| `(skint time)` | Points in time and durations in the style of SRFI 19 | [doc/skint/time.md](../doc/skint/time.md) |
| `(skint date)` | Calendar dates in the style of SRFI 19, ISO 8601 timestamps | [doc/skint/date.md](../doc/skint/date.md) |
