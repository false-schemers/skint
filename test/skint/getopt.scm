(import (skint getopt))

(include "test.scm")

;; ---------------------------------------------------------------------------
;; Fixtures and helpers
;; ---------------------------------------------------------------------------

;; flags and argument-taking options, both forms present
(define opts
  '([verbose "-v" "--verbose" #f     "Increase output verbosity"]
    [quiet   "-q" "--quiet"   #f     "Suppress nonessential messages"]
    [output  "-o" "--output"  "FILE" "Write output to FILE"]
    [count   "-n" "--count"   "N"    "Repeat N times"]))

;; options with only one of the two forms
(define one-sided-opts
  '([verbose "-v" "--verbose" #f     "Increase output verbosity"]
    [output  "-o" "--output"  "FILE" "Write output to FILE"]
    [name    #f   "--name"    "NAME" "Set the name"]
    [count   "-n" #f          "N"    "Repeat N times"]
    [help    "-h" "--help"    #f     "Display this help"]))

;; parse the whole argument list; => ((keysym . optarg) ...)
(define (parse-opts args . ?optmap)
  (define optmap (if (pair? ?optmap) (car ?optmap) opts))
  (let loop ([args args] [found '()])
    (get-next-command-line-option args optmap
      (lambda (keysym optarg restargs)
        (if keysym
            (loop restargs (cons (cons keysym optarg) found))
            (reverse found))))))

;; parse the whole argument list; => the operands left over
(define (parse-rest args . ?optmap)
  (define optmap (if (pair? ?optmap) (car ?optmap) opts))
  (let loop ([args args])
    (get-next-command-line-option args optmap
      (lambda (keysym optarg restargs)
        (if keysym (loop restargs) restargs)))))

;; just the option keysyms, in order
(define (parse-keys args . ?optmap)
  (map car (apply parse-opts args ?optmap)))

;; render the help listing into a string
(define (help-string optmap)
  (let ([p (open-output-string)])
    (print-command-line-options optmap p)
    (get-output-string p)))


(display "\n--- Argument syntax ---\n")

(test "long option, no argument"
  '((verbose . #f))
  (parse-opts '("--verbose")))

(test "long option, argument attached with ="
  '((output . "f.txt"))
  (parse-opts '("--output=f.txt")))

(test "long option, argument in next element"
  '((output . "f.txt"))
  (parse-opts '("--output" "f.txt")))

(test "short option, no argument"
  '((verbose . #f))
  (parse-opts '("-v")))

(test "short option, argument attached"
  '((output . "f.txt"))
  (parse-opts '("-of.txt")))

(test "short option, argument in next element"
  '((output . "f.txt"))
  (parse-opts '("-o" "f.txt")))

(test "several options in sequence"
  '((verbose . #f) (output . "f.txt") (quiet . #f))
  (parse-opts '("-v" "--output=f.txt" "--quiet")))

(test "long and short forms name the same option"
  (parse-opts '("--verbose"))
  (parse-opts '("-v")))


(display "\n--- Clustered short options ---\n")

(test "two flags in one cluster"
  '(verbose quiet)
  (parse-keys '("-vq")))

(test "repeated flag in one cluster"
  '(verbose verbose verbose)
  (parse-keys '("-vvv")))

(test "cluster ending in an argument-taking option, argument separate"
  '((verbose . #f) (output . "f.txt"))
  (parse-opts '("-vo" "f.txt")))

(test "cluster ending in an argument-taking option, argument attached"
  '((verbose . #f) (output . "f.txt"))
  (parse-opts '("-vof.txt")))

(test "cluster leaves operands alone"
  '("a.scm")
  (parse-rest '("-vq" "a.scm")))


(display "\n--- Where parsing stops ---\n")

(test "empty argument list yields no options"
  '()
  (parse-opts '()))

(test "empty argument list yields no operands"
  '()
  (parse-rest '()))

(test "-- terminates options"
  '((verbose . #f))
  (parse-opts '("-v" "--" "-q")))

(test "-- is consumed"
  '("-q")
  (parse-rest '("-v" "--" "-q")))

(test "-- protects option-shaped operands"
  '("-notanopt" "y")
  (parse-rest '("-v" "--" "-notanopt" "y")))

(test "bare - terminates options"
  '((verbose . #f))
  (parse-opts '("-v" "-" "y")))

(test "bare - is kept as an operand"
  '("-" "y")
  (parse-rest '("-v" "-" "y")))

(test "first operand terminates options"
  '((verbose . #f))
  (parse-opts '("-v" "file.scm")))

(test "first operand is kept"
  '("file.scm")
  (parse-rest '("-v" "file.scm")))

(test "empty string is an operand, not an option"
  '("")
  (parse-rest '("")))

(test "-- alone leaves nothing"
  '()
  (parse-rest '("--")))


(display "\n--- Options are not permuted (POSIX, not GNU) ---\n")

(test "options after the first operand are not parsed"
  '((verbose . #f))
  (parse-opts '("-v" "file.scm" "-q")))

(test "options after the first operand are left in place"
  '("file.scm" "-q")
  (parse-rest '("-v" "file.scm" "-q")))

(test "an operand shields a following argument-taking option"
  '("script.scm" "-o" "f.txt")
  (parse-rest '("-v" "script.scm" "-o" "f.txt")))


(display "\n--- Option arguments are taken verbatim ---\n")

(test "negative number as a separate argument"
  '((count . "-5"))
  (parse-opts '("-n" "-5")))

(test "negative number attached to a short option"
  '((count . "-5"))
  (parse-opts '("-n-5")))

(test "negative number after = on a long option"
  '((count . "-5"))
  (parse-opts '("--count=-5")))

(test "option-shaped argument is not re-parsed"
  '((output . "--quiet"))
  (parse-opts '("-o" "--quiet")))

(test "empty argument attached with ="
  '((output . ""))
  (parse-opts '("--output=")))


(display "\n--- One-sided option records ---\n")

(test "long-only option is matched by its long form"
  '((name . "gwendolyn"))
  (parse-opts '("--name=gwendolyn") one-sided-opts))

(test "short-only option is matched by its short form"
  '((count . "3"))
  (parse-opts '("-n" "3") one-sided-opts))

(test-error "long-only option has no short form"
  (parse-opts '("-N" "gwendolyn") one-sided-opts))

(test-error "short-only option has no long form"
  (parse-opts '("--count" "3") one-sided-opts))


(display "\n--- Errors ---\n")

(test-error "unknown long option"
  (parse-opts '("--nope")))

(test-error "unknown short option"
  (parse-opts '("-z")))

(test-error "bare negative number is option-shaped"
  (parse-opts '("-5")))

(test-error "argument-taking long option at end of input"
  (parse-opts '("--output")))

(test-error "argument-taking short option at end of input"
  (parse-opts '("-o")))

(test-error "argument given to a long flag"
  (parse-opts '("--verbose=x")))


(display "\n--- Help formatting ---\n")

(test "both forms, no argument"
  "  -v, --verbose    Increase output verbosity\n"
  (help-string '([verbose "-v" "--verbose" #f "Increase output verbosity"])))

(test "both forms, argument attaches to the long form with ="
  "  -o, --output=FILE    Write output to FILE\n"
  (help-string '([output "-o" "--output" "FILE" "Write output to FILE"])))

(test "long form only"
  "  --name=NAME    Set the name\n"
  (help-string '([name #f "--name" "NAME" "Set the name"])))

(test "short form only, argument attaches with a space"
  "  -n N    Repeat N times\n"
  (help-string '([count "-n" #f "N" "Repeat N times"])))

(test "columns are aligned across a whole option map"
  (string-append
    "  -v, --verbose        Increase output verbosity\n"
    "  -o, --output=FILE    Write output to FILE\n"
    "  --name=NAME          Set the name\n"
    "  -n N                 Repeat N times\n"
    "  -h, --help           Display this help\n")
  (help-string one-sided-opts))

(test "empty option map prints nothing"
  ""
  (help-string '()))

(test "returned offset is where the help column starts"
  23
  (print-command-line-options one-sided-opts (open-output-string)))

;; the port argument defaults to the current output port, so the listing
;; follows parameterize rather than going straight to stdout
(test "port defaults to the current output port"
  "  -v, --verbose    Increase output verbosity\n"
  (let ([p (open-output-string)])
    (parameterize ([current-output-port p])
      (print-command-line-options '([verbose "-v" "--verbose" #f "Increase output verbosity"])))
    (get-output-string p)))

;; 2 spaces of indent + the widest entry + 4 spaces of gap
(test "returned offset tracks the widest entry"
  8
  (print-command-line-options
    '([verbose "-v" #f #f "Increase output verbosity"])
    (open-output-string)))

(test "widest entry alone determines the offset"
  (print-command-line-options one-sided-opts (open-output-string))
  (print-command-line-options
    '([output "-o" "--output" "FILE" "Write output to FILE"])
    (open-output-string)))

(test-error "record with neither short nor long form is rejected"
  (print-command-line-options '([bad #f #f #f "no forms"]) (open-output-string)))


(display "\n--- Round trip: the offset lines up with the rendered text ---\n")

;; the reported offset should equal the column at which help text begins
(let* ([optmap one-sided-opts]
       [offset (print-command-line-options optmap (open-output-string))]
       [line   (let* ([s (help-string optmap)]
                      [nl (string-position #\newline s)])
                 (substring s 0 nl))]
       [help   "Increase output verbosity"])
  (test "help text starts at the reported offset"
    help
    (substring line offset (string-length line))))


(display "\n--- All tests complete. ---\n")

(test-end)
