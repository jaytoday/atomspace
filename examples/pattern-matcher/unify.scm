;
; unify.scm -- Term unification demo
;
; The query engine is able to perform conventional, classical
; term unification. The below is a very short, simple demo of
; this ability.
;
(use-modules (opencog) (opencog exec))

; Populate the AtomSpace with some initial data. This includes
; the data we want to match, and also some confounding data,
; that should not be found.
(Inheritance (Concept "A") (Concept "B"))
(Inheritance (Concept "A") (Concept "C"))
(Inheritance (Concept "B") (Concept "C"))

; Define a basic unifier. It uses the conventional MeetLink to
; do the work.
(define unifier
	(Meet
		(VariableList (Variable "$X") (Variable "$Y"))
		(Identical
			(Inheritance (Concept "A") (Variable "$Y"))
			(Inheritance (Variable "$X") (Concept "B")))))

; Run it.
(cog-execute! unifier)

; The variable declaration is not explicitly required; the variables
; will be automatically extracted in the order that they are found.
; Caution: this reverses the variable order from the above! So $Y
; comes first, so the results will be reversed.
(define implicit-vars
	(Meet
		(Identical
			(Inheritance (Concept "A") (Variable "$Y"))
			(Inheritance (Variable "$X") (Concept "B")))))

; Run it.
(cog-execute! implicit-vars)

; Lets try something more complex, a three-way unification.
; We'll declare the variables explicitly, to avoid confusion.

(define three-way
	(Meet
		(VariableList (Variable "$X") (Variable "$Y") (Variable "$Z"))
		(And
			(Identical
				(Inheritance (Concept "A") (Variable "$Y"))
				(Inheritance (Variable "$X") (Concept "B")))
			(Identical
				(Inheritance (Concept "B") (Variable "$Z"))
				(Inheritance (Variable "$Y") (Concept "C")))
)))

; Run it.
(cog-execute! three-way)

; The End. That's all, folks!
