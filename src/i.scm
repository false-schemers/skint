;---------------------------------------------------------------------------------------------
; Interpreter bootstrap helper code  
;---------------------------------------------------------------------------------------------

(load "s.scm")


; NB: this can be compiled into an entry in i_code module (see i.c), but doesn't have to
; The code in there was produced while the following opcodes had been exposed as integrables:
;
; declare_instruction(dys,        "y",            0,  "%dynamic-state",           '0', AUTOGL)
; declare_instruction(setdys,     "sy",           0,  "%set-dynamic-state!",      '1', AUTOGL)
 
(define (%dynamic-state-reroot! there)
  (let loop ([there there])
    (unless (eq? (%dynamic-state) there)
      (loop (cdr there))
      (let ([before (caar there)] [after (cdar there)])
        (let ([here (%dynamic-state)])
          (set-car! here (cons after before))
          (set-cdr! here there)
          (set-car! there #f)
          (set-cdr! there '())
          (%set-dynamic-state! there))
        (before)))))

