
(define a "hello")
(define b 'ola)

(define (go a b) 
    (if (and (or (> a b) (= a 0)) (not (= b 4)))
		(begin
            (print (+ a b))
			(print (* a b))
			)
    (print (- a b)))

	;(solid:block 0 0 0 1 1 1)
)
(define b (list 1 2 3))

(print (list-ref b 1))

;;; A define of nested lists
(define c (list (list 1 2) ;comment
(list 3 4) (list 5 6)))

; car, cdr cadr
(car (cdr c))

(car (cadr c))

(set! c '( ( 1 2) (3 4) ( 5 6)))

; A map
(map entity:delete (part:entities))

; A map with lambda
(map (lambda (x) (print "deleting")(entity:delete x)) (part:entities))

(let ((x 5)(y 2)) (display "result = ") (print (+ x y)))

(part:clear)
(define  smi0 (smi-shell))
(define  body0  (car (part:load "BODY_1.smt")))
(smi-shell:set-body  smi0  body0 )
(smi-shell:set-lop-options  smi0 'standard_full_rbi)
(smi-shell:set-default-in-offset  smi0 -0.5)
(smi-shell:set-default-out-offset  smi0 0)
(define body (smi-shell:get-body smi0 ))
(ut_expect_error 'LOP_TWK_TOPOL_CHANGE '(smi-shell:compute smi0 ))


(set! smi0 #f)

(define (vargs . args)
	(display args)
)

(list-ref '(1 2 3 4) 2)


(length '(the fat cat eats the thin canary))

(diplay '( (1 2) (3 4)))


