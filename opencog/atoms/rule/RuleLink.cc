/*
 * RuleLink.cc
 *
 * Copyright (C) 2009, 2014, 2015, 2019, 2022 Linas Vepstas
 *
 * Author: Linas Vepstas <linasvepstas@gmail.com>  January 2009
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License v3 as
 * published by the Free Software Foundation and including the
 * exceptions
 * at http://opencog.org/wiki/Licenses
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public
 * License
 * along with this program; if not, write to:
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <opencog/atoms/atom_types/NameServer.h>
#include <opencog/atoms/core/LambdaLink.h>
#include <opencog/atomspace/AtomSpace.h>

#include "RuleLink.h"

using namespace opencog;

void RuleLink::init(void)
{
	Type t = get_type();

	// If this is a PatternLink, bail out now. They have thier
	// own custom setup.
	if (nameserver().isA(t, PATTERN_LINK)) return;

	if (not nameserver().isA(t, RULE_LINK))
	{
		const std::string& tname = nameserver().getTypeName(t);
		throw InvalidParamException(TRACE_INFO,
			"Expecting a RuleLink, got %s", tname.c_str());
	}

	// If we are quoted, don't bother to try to do anything.
	if (_quoted) return;

	extract_variables(_outgoing);
}

RuleLink::RuleLink(const Handle& vardecl,
                   const Handle& body,
                   const Handle& rewrite)
	: RuleLink(HandleSeq{vardecl, body, rewrite})
{}

RuleLink::RuleLink(const Handle& body, const Handle& rewrite)
	: RuleLink(HandleSeq{body, rewrite})
{}

RuleLink::RuleLink(const HandleSeq&& hseq, Type t)
	: PrenexLink(std::move(hseq), t)
{
	// _quoted is true, that means we are inside a quote,
	// and so nothing to be done. Skip variable extraction.
	// This is what ScopeLink does, too.
	if (_quoted) return;
	init();
}

/* ================================================================= */
///
/// Find and unpack variable declarations, if any; otherwise, just
/// find all free variables in the body (but not the implicand).
///
/// On top of that, initialize _body and _implicand with the
/// clauses and the rewrite rule(s). (Multiple implicands are
/// allowed, this can save some CPU cycles when one search needs to
/// create several rewrites.)
///
void RuleLink::extract_variables(const HandleSeq& oset)
{
	size_t sz = oset.size();
	if (sz < 1)
		throw SyntaxException(TRACE_INFO,
			"Expecting a non-empty outgoing set");

	// Old-style declarations have variables in the first
	// slot. If they are there, then respect that.
	// Otherwise, the first slot holds the body.
	size_t boff = 0;
	Type vt = oset[0]->get_type();
	if (VARIABLE_LIST == vt or
	    VARIABLE_SET == vt or
	    TYPED_VARIABLE_LINK == vt or
	    VARIABLE_NODE == vt or
	    GLOB_NODE == vt)
	{
		_vardecl = oset[0];
		ScopeLink::init_scoped_variables(_vardecl);
		boff = 1;
	}
	else
	{
		// Hunt for variables in the main body, only.
		// Do not hunt for them in the implicand clauses that follow.
		// This is because those implicands might hold other free
		// variables not appearing in the body.
		_variables.find_variables(oset[0]);
	}

	// We already know that sz==1 or greater, so if boff is that oh no
	if (sz == boff)
		throw SyntaxException(TRACE_INFO,
			"Expecting a declaration of a body/premise!");

	_body = oset[boff];
	for (size_t i=boff+1; i < sz; i++)
		_implicand.push_back(oset[i]);

	// Remove any declared variables that are NOT in the body!
	// This is an "unusual" situation, except that the URE does
	// this regularly, when it constructs rules on the fly.
	// I don't know why.
	if (1 == boff)
	{
		_implicand.push_back(_body),
		trim(_implicand);
		_implicand.pop_back();
	}
}

/* ================================================================= */

// Execute h. It its a lambda, unwrap it.
static Handle maybe_exec(const Handle& h, Variables& redvars)
{
	Handle hred(h);

	// XXX FIXME: this is a weird hack that I do not understand.
	// ExecutableLinks can return anything, and not just Atoms.
	// So, here we execute it, and if a non-Atom Value is returned,
	// then pretend the execution never happened. Is this actually
	// correct? Won't there be weird side-effects? WTF??
	if (h->is_executable())
	{
		ValuePtr vp = h->execute();
		if (vp->is_atom())
			hred = HandleCast(vp);
	}

	if (hred->is_type(LAMBDA_LINK))
	{
		LambdaLinkPtr lmb(LambdaLinkCast(hred));
		redvars.extend_intersect(lmb->get_variables());
		hred = lmb->get_body();
	}
	return hred;
}

/// Reduce the link; i.e. call execute on everything that it wraps.
ValuePtr RuleLink::execute(AtomSpace* as, bool silent)
{
	Variables redvars = _variables;

	HandleSeq redbody;
	for (const Handle& h : _body->getOutgoingSet())
		redbody.emplace_back(maybe_exec(h, redvars));

	Handle rbdy(as->add_link(_body->get_type(), std::move(redbody)));

	HandleSeq redimpl;
	for (const Handle& h : _implicand)
		redimpl.emplace_back(maybe_exec(h, redvars));

	HandleSeq redset;
	redset.emplace_back(redvars.get_vardecl());
	redset.emplace_back(rbdy);
	redset.insert(redset.end(), redimpl.begin(), redimpl.end());

	return as->add_link(_type, std::move(redset));
}

DEFINE_LINK_FACTORY(RuleLink, RULE_LINK)

/* ===================== END OF FILE ===================== */
