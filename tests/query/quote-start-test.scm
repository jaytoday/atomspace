;
; quote-start-test.scm -- Verify a Quote appearing at the top level works.
;

(use-modules (opencog) (opencog exec))
(use-modules (opencog test-runner))

(opencog-test-runner)
(define tname "quote-start-test")
(test-begin tname)

(Evaluation (Predicate "foo") (Concept "bar"))

(define qry
	(CollectionOf (Meet (TypedVariable (Variable "X") (Type 'Concept))
	(Quote (Evaluation
		(Unquote (Predicate "foo"))
		(Unquote (Variable "X")))))))

(test-assert "query for both"
	(equal? (cog-execute! qry) (Set (Concept "bar"))))

(test-end tname)

(opencog-test-end)
