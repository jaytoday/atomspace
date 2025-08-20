/*
 * opencog/atoms/parallel/PureExecLink.cc
 *
 * Copyright (C) 2009, 2013-2015, 2020, 2024, 2025 Linas Vepstas
 * SPDX-License-Identifier: AGPL-3.0-or-later
 * All Rights Reserved
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License v3 as
 * published by the Free Software Foundation and including the exceptions
 * at http://opencog.org/wiki/Licenses
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program; if not, write to:
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <opencog/atoms/parallel/PureExecLink.h>
#include <opencog/atoms/value/VoidValue.h>
#include <opencog/atomspace/AtomSpace.h>
#include <opencog/atomspace/Transient.h>

using namespace opencog;

/// PureExecLink
/// Perform execution in given AtomSpace, or a transient, if none given.
///
/// The general structure of this link is
///
///        PureExecLink
///            AtomSpace (optional)
///            ExecutableAtom
///            AnotherExecutableAtom
///            AnotherAtomSpace (optional)
///            MoreExecutableAtom
///
/// When this link is executed, all of the various `ExecutableAtoms`
/// are executed in the sequential order, in the most recent AtomSpace
/// that preceeded them. Thus, if execution has side effects, such
/// as creating new Atoms, they end up there, and not the current
/// AtomSpace. That's what make's it "Pure" -- no side-effects in the
/// current AtomSpace.
///
/// If no AtomSpace is given, a temporary transient is used.
/// The value returned by execution is the result of executing
/// the last Atom in the sequence. The result of executing a
/// non-executable atom is that Atom itself.

PureExecLink::PureExecLink(const HandleSeq&& oset, Type t)
    : Link(std::move(oset), t)
{
	if (0 == _outgoing.size())
		throw InvalidParamException(TRACE_INFO,
			"Expecting at least one argument!");
}

ValuePtr PureExecLink::execute(AtomSpace* as,
                               bool silent)
{
	ValuePtr result = createVoidValue();
	AtomSpace* ctxt = nullptr;
	for (const Handle& h : _outgoing)
	{
		if (h->is_type(ATOM_SPACE))
		{
			ctxt = AtomSpaceCast(h).get();
			continue;
		}
		if (not h->is_executable())
		{
			result = h;
			continue;
		}
		if (ctxt)
		{
			result = h->execute(ctxt, silent);
			continue;
		}

		// No AtomSpace provided. Use a temporary.
		// Avoid transient memory space leak. Well, there's no actual
		// leak, because the pool deals with it; it just prints a nasty
		// warning message, and we want to hide that message.
		AtomSpace* tas = grab_transient_atomspace(as);
		std::exception_ptr eptr;
		try {
			result = h->execute(tas, silent);
		}
		catch(...) {
			eptr = std::current_exception();
		}
		release_transient_atomspace(tas);
		if (eptr) std::rethrow_exception(eptr);
	}

	return result;
}

DEFINE_LINK_FACTORY(PureExecLink, PURE_EXEC_LINK)
