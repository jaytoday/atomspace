;
; formulas.scm -- Declaring formulas that compute truth values.
;
; This is a modified copy of an example program. It helps verify that
; the example actually works.
;
(use-modules (opencog) (opencog exec))

(define tvkey (Predicate "*-TruthValueKey-*"))

(define atom-a (Concept "A")
(define atom-b (Concept "B")
(cog-set-value! atom-a tvkey (SimpleTruthValue  0.8 1.0))
(cog-set-value! atom-b tvkey (SimpleTruthValue  0.6 0.9))

(define (strength-of ATOM) (ElementOf (Number 0) (ValueOf ATOM tvkey)))
(define (confidence-of ATOM) (ElementOf (Number 1) (ValueOf ATOM tvkey)))

; Multiply the strength of the TV's of two atoms.
(define prod
	(Times (strength-of (Concept "A")) (strength-of (Concept "B"))))

(define stv-const (FormulaPredicate (Number 0.7) (Number 0.314)))

(define formula-stv
	(FormulaPredicate
		(Minus
			(Number 1)
			(Times (strength-of (Concept "A")) (strength-of (Concept "B"))))
		(Times (confidence-of (Concept "A")) (confidence-of (Concept "B")))))

; The below computes a truth value, and attaches it to the
; EvaluationLink.
(define my-ev-link
	(Evaluation
		(FormulaPredicate (Number 0.75) (Number 0.628))
		(List
			(Concept "A")
			(Concept "B"))))

; Formula with variables
(define eval-formula
	(Evaluation
		; Compute TV = (1-sA*sB, cA*cB)
		(FormulaPredicate
			(Minus
				(Number 1)
				(Times
					(strength-of (Variable "$X"))
					(strength-of (Variable "$Y"))))
			(Times
				(confidence-of (Variable "$X"))
				(confidence-of (Variable "$Y"))))
		(List
			(Concept "A")
			(Concept "B"))))

; Optionally, you can wrap formulas with LambdaLinks. This doesn't
; really change anything; formulas work fine without LambdaLinks.
(define eval-lambda
	(Evaluation
		; Compute TV = (1-sA*sB, cA*cB)
		(FormulaPredicate
			(Lambda
				; Lambda without a decl, intentionally so.
				; (NopeVariableList (Variable "$X") (Variable "$Y"))
				(Minus
					(Number 1)
					(Times
						(strength-of (Variable "$X"))
						(strength-of (Variable "$Y")))))
			(Lambda
				(VariableList (Variable "$X") (Variable "$Y"))
				(Times
					(confidence-of (Variable "$X"))
					(confidence-of (Variable "$Y")))))
		(List
			(Concept "A")
			(Concept "B"))))


; Beta-reducation works as normal. The below will create an
; EvaluationLink with ConceptNode A and B in it, and will set the
; truth value according to the formula.
(define put-link
		(PutLink
			(VariableList (Variable "$VA") (Variable "$VB"))
			(Evaluation
				; Compute TV = (1-sA*sB, cA*cB)
				(FormulaPredicate
					(Minus
						(Number 1)
						(Times
							(strength-of (Variable "$VA"))
							(strength-of (Variable "$VB"))))
					(Times
						(confidence-of (Variable "$VA"))
						(confidence-of (Variable "$VB"))))
				(List
					(Variable "$VA") (Variable "$VB")))
		(Set (List (Concept "A") (Concept "B")))))


; One can also use DefinedPredicates, to give the formula a name.
(DefineLink
	(DefinedPredicate "has a reddish color")
	(FormulaPredicate
		(Minus
			(Number 1)
			(Times
				(strength-of (Variable "$X"))
				(strength-of (Variable "$Y"))))
		(Times
			(confidence-of (Variable "$X"))
			(confidence-of (Variable "$Y")))))

(cog-set-value! (Concept "A") tvkey (SimpleTruthValue 0.9 0.98))
(cog-set-value! (Concept "B") tvkey (SimpleTruthValue 0.9 0.98))

; The will cause the formula to evaluate.
(define red-form
	(Evaluation
		(DefinedPredicate "has a reddish color")
		(List
			(Concept "A")
			(Concept "B"))))

; --------------------------------------------------

(cog-set-value! atom-a tvkey (SimpleTruthValue 0.8 1.0))
(cog-set-value! atom-b tvkey (SimpleTruthValue 0.6 0.9))
(define atom-c (Concept "C"))


(define key (Predicate "key"))

(define iab (Inheritance atom-a atom-b))
(define ibc (Inheritance atom-b atom-c))
(cog-set-value! iab tvkey (SimpleTruthValue  0.8 0.8))
(cog-set-value! ibc tvkey (SimpleTruthValue  0.3 0.3))

(cog-set-value! iab key (FloatValue 1 2 3))
(cog-set-value! ibc key (FloatValue 4 5 6))

; The InheritanceLink is not necessarily in any atomspace.
(Define
	(DefinedPredicate "its-about-one")
	(Lambda
		(VariableList (Variable "$x") (Variable "$y"))
		(SequentialAnd
			(GreaterThan
				(FloatValueOf (Inheritance (Variable "$x") (Variable "$y"))  key)
				(Number 0.99))
			(GreaterThan
				(Number 1.01)
				(FloatValueOf (Inheritance (Variable "$x") (Variable "$y")) key))
		)))

; Expect (its-one atom-a atom-b) to be true,
; and (its-one atom-b atom-c) to be false.
(define (its-one a b)
	(Evaluation (DefinedPredicate "its-about-one") (List a b)))

(Define
	(DefinedPredicate "mostly-confident")
	(Lambda
		(VariableList (Variable "$x") (Variable "$y"))
		(SequentialAnd
			(GreaterThan
				(confidence-of (Inheritance (Variable "$x") (Variable "$y")))
				(Number 0.75))
			(GreaterThan
				(Number 0.85)
				(confidence-of (Inheritance (Variable "$x") (Variable "$y"))))
		)))

; Expect (its-conf atom-a atom-b) to be true,
; and (its-conf atom-b atom-c) to be false.
(define (its-conf a b)
	(Evaluation (DefinedPredicate "mostly-confident") (List a b)))

; --------------------------------------------------
; Testing naked predicate formulas (issue #2218).

(define naked-pred1
  (FormulaPredicate
    (Number 1)
    (Number 1)
  )
)

(define naked-pred2
  (FormulaPredicate
    (Times
      (Number 0.5)
      (Number 1)
    )
    (Number 1)
  )
)

(define naked-pred3
  (FormulaPredicate
    (Number 1)
    (Times
      (Number 0.5)
      (Number 1)
    )
  )
)

(define apple-is-green (Concept "apple-is-green" (stv 1 0.5)))
(define apple-is-red (Concept "apple-is-red" (stv 0.9 0.6)))

(define naked-pred4
  (FormulaPredicate
    (Number 1)
    (Times
      (Number 1)
      (Number 0.5)
      (strength-of apple-is-green)
      (confidence-of apple-is-red)
    )
  )
)

(define (times x y)
  (cog-execute! (Times x y))
)

(define naked-pred5
  (FormulaPredicate
    (Number 1)
    (ExecutionOutput
      (GroundedSchema "scm:times")
      (List (Number 0.9) (Number 0.5))
    )
  )
)

(define naked-pred-crash1
  (FormulaPredicate
    (Concept "blabla")
    (Number 1)
  )
)

(define naked-pred-crash2
  (FormulaPredicate
    (Number 1)
    (ExecutionOutput
      (Lambda (Concept "blabla"))
      (List)
    )
  )
)

; -------------------------------------------------
; Testing defined predicate formulas (issue #2218).

(Define
  (DefinedPredicate "defined-pred1")
  (FormulaPredicate
    (Number 1)
    (Number 1)
  )
)

(Define
  (DefinedPredicate "defined-pred2")
  (FormulaPredicate
    (Times
      (Number 1)
      (Number 0.5)
    )
    (Number 1)
  )
)

(Define
  (DefinedPredicate "defined-pred3")
  (FormulaPredicate
    (Number 1)
    (Times
      (Number 1)
      (Number 0.5)
    )
  )
)

(Define
  (DefinedPredicate "defined-pred4")
  (FormulaPredicate
    (Number 1)
    (Times
      (Number 1)
      (Number 0.5)
      (strength-of apple-is-green)
      (confidence-of apple-is-red)
    )
  )
)

(Define
  (DefinedPredicate "defined-pred-crash1")
  (FormulaPredicate
    (ExecutionOutput
      (Lambda (Concept "ahaha"))
      (List)
    )
    (Times
      (Number 1)
      (Number 0.5)
      (strength-of apple-is-green)
      (confidence-of apple-is-red)
    )
  )
)

(Define
  (DefinedPredicate "defined-pred-crash2")
  (FormulaPredicate
    (Number 1)
    (Concept "saboteur")
  )
)

(define (eval-nullary name)
  (Evaluation
    (DefinedPredicate name)
    (List)
  )
)

; -------------------------------------------------
; Testing defined predicate execution (issue #2312).

; Initialize count to zero
(State (Anchor "sum") (Number 0))

; Define increment "function"
(Define
	(DefinedPredicate "inc")
	(True
		(Put
			(State (Anchor "sum") (Variable "$x"))
			(Plus (Number 1) (Get (State (Anchor "sum") (Variable "$y")))))))

; GetLink returns a SetLink. Unwrap it to get the NumberNode.
(define (get-sum)
	(cog-outgoing-atom (cog-execute!
		(Get (State (Anchor "sum") (Variable "$x"))))
		0))

(*unspecified*)
