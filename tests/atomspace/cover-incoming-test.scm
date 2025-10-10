;
; cover-incoming-test.scm
; Verify that incoming sets are correctly computed in nested
; atomspaces.
;
(use-modules (srfi srfi-1))
(use-modules (opencog) (opencog test-runner))

(opencog-test-runner)

(define (get-cnt ATOM) (inexact->exact (cog-count ATOM)))

; -------------------------------------------------------------------
; Common setup, used by all tests.

(define base-space (cog-atomspace))
(define mid1-space (cog-new-atomspace base-space))
(define mid2-space (cog-new-atomspace mid1-space))
(define mid3-space (cog-new-atomspace mid2-space))
(define mid4-space (cog-new-atomspace mid3-space))
(define mid5-space (cog-new-atomspace mid4-space))
(define mid6-space (cog-new-atomspace mid5-space))
(define top-space (cog-new-atomspace mid6-space))

; Splatter some atoms into the various spaces.
(cog-set-atomspace! base-space)
(Concept "foo" (FloatValue 1 0 3))
(Concept "bar" (FloatValue 1 0 4))

(cog-set-atomspace! mid1-space)
(ListLink (Concept "foo") (Concept "bar") (FloatValue 1 0 5))

(cog-set-atomspace! mid2-space)
(cog-extract-recursive! (Concept "foo"))

(cog-set-atomspace! mid3-space)
(Concept "foo" (FloatValue 1 0 6))
(List (Concept "foo") (Concept "x"))
(Set (Concept "foo") (Concept "s"))

(cog-set-atomspace! mid4-space)
(ListLink (Concept "foo") (Concept "bar") (FloatValue 1 0 7))

(cog-set-atomspace! mid5-space)
(ListLink (Concept "foo") (Concept "bar") (FloatValue 1 0 8))

(cog-set-atomspace! mid6-space)
(Concept "foo" (FloatValue 1 0 9))

(cog-set-atomspace! top-space)

; -------------------------------------------------------------------
; Test that incoming sets in complex situations.

(define complex-inco "test complex incoming")
(test-begin complex-inco)

(define foo (Concept "foo"))
(define bar (Concept "bar"))

; ------------------------------------
; top space
(test-equal "foo-inset-sz" 3 (cog-incoming-size foo))
(test-equal "foo-inset" 3 (length (cog-incoming-set foo)))

(test-equal "foo-lk-inset-sz" 2 (cog-incoming-size-by-type foo 'List))
(test-equal "foo-lk-inset" 2 (length (cog-incoming-by-type foo 'List)))

(test-equal "foo-st-inset-sz" 1 (cog-incoming-size-by-type foo 'Set))
(test-equal "foo-st-inset" 1 (length (cog-incoming-by-type foo 'Set)))

(test-equal "top-foo-tv" 9 (get-cnt (Concept "foo")))
(test-equal "top-lk-tv" 8 (get-cnt (List (Concept "foo") (Concept "bar"))))

; ------------------------------------
; mid6 space
(test-equal "6foo-inset-sz" 3 (cog-incoming-size foo mid6-space))
(test-equal "6foo-inset" 3 (length (cog-incoming-set foo mid6-space)))

(test-equal "6foo-lk-inset-sz" 2 (cog-incoming-size-by-type foo 'List mid6-space))
(test-equal "6foo-lk-inset" 2 (length (cog-incoming-by-type foo 'List mid6-space)))

(test-equal "6foo-st-inset-sz" 1 (cog-incoming-size-by-type foo 'Set mid6-space))
(test-equal "6foo-st-inset" 1 (length (cog-incoming-by-type foo 'Set mid6-space)))

(test-equal "6bar-lk-inset-sz" 1 (cog-incoming-size-by-type bar 'List mid6-space))
(test-equal "6bar-lk-inset-cnt" 8
	(get-cnt (car (cog-incoming-by-type bar 'List mid6-space))))

; This Link still has the lower foo in it, not the top foo
(define lofo (gar (car (cog-incoming-by-type bar 'List mid6-space))))
(test-equal "6bar-lk-in-foo-cnt" 6 (get-cnt lofo))
(test-assert "6foo" (not (equal? lofo foo)))
(test-assert "6foo-eq" (cog-equal? lofo foo))

; ------------------------------------
; mid5 space
(test-equal "5foo-inset-sz" 3 (cog-incoming-size foo mid5-space))
(test-equal "5foo-inset" 3 (length (cog-incoming-set foo mid5-space)))

(test-equal "5foo-lk-inset-sz" 2 (cog-incoming-size-by-type foo 'List mid5-space))
(test-equal "5foo-lk-inset" 2 (length (cog-incoming-by-type foo 'List mid5-space)))

(test-equal "5foo-st-inset-sz" 1 (cog-incoming-size-by-type foo 'Set mid5-space))
(test-equal "5foo-st-inset" 1 (length (cog-incoming-by-type foo 'Set mid5-space)))

(test-equal "5bar-lk-inset-sz" 1 (cog-incoming-size-by-type bar 'List mid5-space))
(test-equal "5bar-lk-inset-cnt" 8
	(get-cnt (car (cog-incoming-by-type bar 'List mid5-space))))

; ------------------------------------
; mid4 space
(test-equal "4foo-inset-sz" 3 (cog-incoming-size foo mid4-space))
(test-equal "4foo-inset" 3 (length (cog-incoming-set foo mid4-space)))

(test-equal "4foo-lk-inset-sz" 2 (cog-incoming-size-by-type foo 'List mid4-space))
(test-equal "4foo-lk-inset" 2 (length (cog-incoming-by-type foo 'List mid4-space)))

(test-equal "4foo-st-inset-sz" 1 (cog-incoming-size-by-type foo 'Set mid4-space))
(test-equal "4foo-st-inset" 1 (length (cog-incoming-by-type foo 'Set mid4-space)))

(test-equal "4bar-lk-inset-sz" 1 (cog-incoming-size-by-type bar 'List mid4-space))
(test-equal "4bar-lk-inset-cnt" 7
	(get-cnt (car (cog-incoming-by-type bar 'List mid4-space))))

; ------------------------------------
; mid3 space
(test-equal "3foo-inset-sz" 2 (cog-incoming-size foo mid3-space))
(test-equal "3foo-inset" 2 (length (cog-incoming-set foo mid3-space)))

(test-equal "3foo-lk-inset-sz" 1 (cog-incoming-size-by-type foo 'List mid3-space))
(test-equal "3foo-lk-inset" 1 (length (cog-incoming-by-type foo 'List mid3-space)))

(test-equal "3foo-st-inset-sz" 1 (cog-incoming-size-by-type foo 'Set mid3-space))
(test-equal "3foo-st-inset" 1 (length (cog-incoming-by-type foo 'Set mid3-space)))

(test-equal "set-space" mid3-space
	(cog-atomspace (car (cog-incoming-by-type foo 'Set))))

; ------------------------------------
; mid2 space
(test-equal "2foo-inset-sz" 0 (cog-incoming-size foo mid2-space))
(test-equal "2foo-inset" 0 (length (cog-incoming-set foo mid2-space)))

(test-equal "2foo-lk-inset-sz" 0 (cog-incoming-size-by-type foo 'List mid2-space))
(test-equal "2foo-lk-inset" 0 (length (cog-incoming-by-type foo 'List mid2-space)))

(test-equal "2foo-st-inset-sz" 0 (cog-incoming-size-by-type foo 'Set mid2-space))
(test-equal "2foo-st-inset" 0 (length (cog-incoming-by-type foo 'Set mid2-space)))

; ------------------------------------
; mid1 space
(test-equal "1foo-inset-sz" 1 (cog-incoming-size foo mid1-space))
(test-equal "1foo-inset" 1 (length (cog-incoming-set foo mid1-space)))

(test-equal "1foo-lk-inset-sz" 1 (cog-incoming-size-by-type foo 'List mid1-space))
(test-equal "1foo-lk-inset" 1 (length (cog-incoming-by-type foo 'List mid1-space)))

(test-equal "1foo-st-inset-sz" 0 (cog-incoming-size-by-type foo 'Set mid1-space))
(test-equal "1foo-st-inset" 0 (length (cog-incoming-by-type foo 'Set mid1-space)))

(test-equal "1bar-lk-inset-cnt" 5
	(get-cnt (car (cog-incoming-by-type foo 'List mid1-space))))

; ------------------------------------
; base space
(test-equal "0foo-inset-sz" 0 (cog-incoming-size foo base-space))
(test-equal "0foo-inset" 0 (length (cog-incoming-set foo base-space)))

(test-equal "0foo-lk-inset-sz" 0 (cog-incoming-size-by-type foo 'List base-space))
(test-equal "0foo-lk-inset" 0 (length (cog-incoming-by-type foo 'List base-space)))

(test-equal "0foo-st-inset-sz" 0 (cog-incoming-size-by-type foo 'Set base-space))
(test-equal "0foo-st-inset" 0 (length (cog-incoming-by-type foo 'Set base-space)))

(test-end complex-inco)

; -------------------------------------------------------------------
(opencog-test-end)
