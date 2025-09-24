;
; factorial.scm - compute the factorial function, recursively.
;
; This implements a classic textbook algorithm for computing the
; factorial. It defines the factorial function as `f(n) = n * f(n-1)`
; and using an if statement (conditional) to terminate recursion when
; `n` reaches 1.  This is tail-recursive: no stack in either C++ or
; in scheme (or in "Atomese") will increase in size.
;
; This is an imperfect demo: although it does show that classic
; recursive algorithms can be implemented in Atomese, this is not
; really recommended programming practice, for the following reasons:
;
;  * The AtomSpace is a database. Whatever atoms get created as a result
;    of the algorithm will continue to live in the AtomSpace,
;    indefinitely. Is this something you really want? Badly-written
;    algos risk filling the AtomSpace with junk nodes - be careful!
;
;  * Running algorithms in Atomese is slow. It's not intended to be
;    a high-performance numerical computing platform; it is meant to
;    be a general-purpose knowledge-representation and reasoning system.
;
;  * A better example of numerical computation in the AtomSpace is the
;    processing of Values. Values are inherently fleeting, and are much
;    more numerically-oriented. Other examples show how.
;
(use-modules (opencog) (opencog exec))

; Compute the factorial of a NumberNode, using the classic
; recursive algorithm.
;
(Define
	(DefinedProcedure "factorial")
	(Lambda
		; A single argument; it must be a number
		(TypedVariable (Variable "$n") (Type "NumberNode"))

		; Conditional: if the first term (the conditional)
		; evaluates to "true", then the second term (the
		; consequent) will be executed; else the third term
		; (the alternative) will be executed
		(Cond
			; The condition: `n>0`
			(GreaterThan (Variable "$n") (Number 0))

			; The consequent: `f(n) = n * f(n-1)`
			(Times
				(Variable "$n")
				(ExecutionOutput
					(DefinedProcedure "factorial")
					(Minus (Variable "$n") (Number 1))))

			; The alternative: `f(1) = 1`
			(Number 1)))
)

; Call the above-defined factorial function, computing the
; factorial of five. Should return 120.
; (cog-execute! (ExecutionOutput (DefinedProcedure "factorial") (Number 5)))
;
#! ----------
How fast is this? Well, its slowwwww, but still, you can find out:
Just cut-n-paste the below. It takes about 16 seconds on my cheap
Intel Celeron laptop.

(define nrep 5000)
(define start (get-internal-real-time))
(for-each
	(lambda (x)
		(cog-execute! (ExecutionOutput (DefinedProcedure "factorial") (Number 100))))
	(iota nrep))
(define end (get-internal-real-time))

(define elapsed
	(exact->inexact
		(/ (- end start) internal-time-units-per-second)))

(format #t "Total run time=~6F seconds.  Each call took ~6F millisecs\n"
	elapsed (* 1000 (/ elapsed nrep)))

Note that the run-time does not depend on the AtomSpace size:
Lets create 400K Atoms:

(for-each (lambda (n)
   (Times (Number n) (Plus (Number (* n 3.14) (Number 1.57)))))
   (iota 100000))

and run the measurement again; there should be no change.

---- !#

; Also, lets take a look at the atomspace, after execution:
; (cog-report-counts)

; Notice the number of Atoms that were created. Effectively, the code
; is creating a tree that is N levels high, and then evaluating that
; tree, producing one NumberNode, holding the result of the evaluation.
;
; ---------------------------------------------------------------
; The End! That's all, folks!
