/*
 * TransposeColumn.cc
 *
 * Copyright (C) 2015, 2022, 2025 Linas Vepstas
 *
 * Author: Linas Vepstas <linasvepstas@gmail.com>  January 2009
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License v3 as
 * published by the Free Software Foundation and including the
 * exceptions at http://opencog.org/wiki/Licenses
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public
 * License along with this program; if not, write to:
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <opencog/atoms/core/FunctionLink.h>
#include <opencog/atoms/core/NumberNode.h>
#include <opencog/atoms/value/FloatValue.h>
#include <opencog/atoms/value/LinkValue.h>
#include <opencog/atoms/value/StringValue.h>

#include "TransposeColumn.h"

using namespace opencog;

TransposeColumn::TransposeColumn(const HandleSeq&& oset, Type t)
	: Link(std::move(oset), t)
{
	if (not nameserver().isA(t, TRANSPOSE_COLUMN))
	{
		const std::string& tname = nameserver().getTypeName(t);
		throw InvalidParamException(TRACE_INFO,
			"Expecting a TransposeColumn, got %s", tname.c_str());
	}
}

// ---------------------------------------------------------------

/// Return a FloatValue vector.
ValuePtr TransposeColumn::do_handle_loop(AtomSpace* as, bool silent,
                                         const HandleSeq& hrows)
{
	ValueSeq vrows;
	vrows.reserve(hrows.size());
	for (const Handle& h : hrows)
		vrows.push_back(h);

	return do_value_loop(as, silent, vrows);
}

// ---------------------------------------------------------------

#define CHKSZ(ROW) \
	if (ROW.size() < ncols) \
		throw RuntimeException(TRACE_INFO, \
			"Short row! Got %lu want %lu\n", ROW.size(), ncols);

/// Return a FloatValue vector.
ValuePtr TransposeColumn::do_value_loop(AtomSpace* as, bool silent,
                                        const ValueSeq& vrows)
{
	if (0 == vrows.size())
		return createLinkValue();

	// On transposition, rows become columns. The number of rows becomes
	// the number of columns, and the type of the row becomes the type of
	// the column. ... except when the columns are packaged into either a
	// ListValue or a Link, in which case, the columns show up packed
	// individually in rows. These are two distinct cases; each gets
	// different unpacking.
	Type rtype = vrows[0]->get_type();
	for (ValuePtr vp: vrows)
	{
		if (vp->get_type() != rtype)
			return do_direct_loop(as, silent, vrows);
	}

	// If we are here, then the first ListValue row holds the columns
	// that we will be extracting. That is, the first row provides all
	// the columns and column types.
	size_t ncols = 0;
	ValueSeq vcols;
	for (ValuePtr vp: vrows)
	{
		if (vp->is_atom() and HandleCast(vp)->is_executable())
			vp = FunctionLink::get_value(as, silent, vp);

		if (0 == ncols)
		{
			ncols = vp->size();
			vcols.reserve(ncols);

			// The if-statemens below are ordered in the sequence
			// of most-likely to least likely. I think transposing
			// FloatValues will be the most common case, and then
			// the LinkValues...
			if (vp->is_type(FLOAT_VALUE))
			{
				const std::vector<double>& vals = FloatValueCast(vp)->value();
				for (double d : vals)
					vcols.emplace_back(createFloatValue(d));
			}
			else if (vp->is_type(LINK_VALUE))
			{
				const ValueSeq& vrow = LinkValueCast(vp)->value();
				for (const ValuePtr& v : vrow)
				{
					if (1 == v->size())
					{
						if (v->is_type(FLOAT_VALUE))
						{
							double d = FloatValueCast(v)->value()[0];
							vcols.emplace_back(createFloatValue(d));
						}
						else if (v->is_type(STRING_VALUE))
						{
							const std::string& s = StringValueCast(v)->value()[0];
							vcols.emplace_back(createStringValue(s));
						}
						else
							vcols.emplace_back(createLinkValue(v));
					}
					else
						vcols.emplace_back(createLinkValue(v));
				}
			}
			else if (vp->is_type(NUMBER_NODE))
			{
				const std::vector<double>& vals = NumberNodeCast(vp)->value();
				for (double d : vals)
					vcols.emplace_back(createFloatValue(d));
			}
			else if (vp->is_type(STRING_VALUE))
			{
				const std::vector<std::string>& vals = StringValueCast(vp)->value();
				for (const std::string& s : vals)
					vcols.emplace_back(createStringValue(s));
			}
			else if (vp->is_link())
			{
				const HandleSeq& hrow = HandleCast(vp)->getOutgoingSet();
				for (const Handle& h : hrow)
					vcols.emplace_back(createLinkValue(h));
			}
			else
				throw RuntimeException(TRACE_INFO,
					"I don't know what to do with %s\n", vp->to_string().c_str());

			continue;
		}

		// If we are here, this is not the first row, and we know how
		// many columns there are, and what their types should be.
		if (vp->is_type(FLOAT_VALUE))
		{
			const std::vector<double>& vals = FloatValueCast(vp)->value();
			CHKSZ(vals);
			for (size_t i=0; i<ncols; i++)
				FloatValueCast(vcols[i]) -> _value.push_back(vals[i]);
		}
		else if (vp->is_type(LINK_VALUE))
		{
			const ValueSeq& vrow = LinkValueCast(vp)->value();
			CHKSZ(vrow);
			for (size_t i=0; i< ncols; i++)
			{
				const ValuePtr& v(vrow[i]);
				if (1 == v->size())
				{
					if (v->is_type(FLOAT_VALUE))
					{
						double d = FloatValueCast(v)->value()[0];
						FloatValueCast(vcols[i]) -> _value.emplace_back(d);
					}
					else if (v->is_type(STRING_VALUE))
					{
						const std::string& s = StringValueCast(v)->value()[0];
						StringValueCast(vcols[i]) -> _value.emplace_back(s);
					}
					else
						LinkValueCast(vcols[i]) -> _value.push_back(vrow[i]);
				}
				else
					LinkValueCast(vcols[i]) -> _value.push_back(vrow[i]);
			}
		}
		else if (vp->is_type(NUMBER_NODE))
		{
			const std::vector<double>& vals = NumberNodeCast(vp)->value();
			CHKSZ(vals);
			for (size_t i=0; i< ncols; i++)
				FloatValueCast(vcols[i]) -> _value.push_back(vals[i]);
		}
		else if (vp->is_type(STRING_VALUE))
		{
			const std::vector<std::string>& vals = StringValueCast(vp)->value();
			CHKSZ(vals);
			for (size_t i=0; i< ncols; i++)
				StringValueCast(vcols[i]) -> _value.push_back(vals[i]);
		}
		else if (vp->is_link())
		{
			const HandleSeq& hrow = HandleCast(vp)->getOutgoingSet();
			CHKSZ(hrow);
			for (size_t i=0; i< ncols; i++)
				LinkValueCast(vcols[i]) -> _value.push_back(hrow[i]);
		}
	}

	return createLinkValue(std::move(vcols));
}

// ---------------------------------------------------------------

/// Return a FloatValue vector.
ValuePtr TransposeColumn::do_direct_loop(AtomSpace* as, bool silent,
                                         const ValueSeq& vrows)
{
	// Convert rows to columns. The number of columns output will
	// equal the length of the first row (and all rows should have
	// this same length.) The type of the column will be the type
	// of the corresponding row.
	size_t ncols = vrows[0]->size();
	ValueSeq vcols;
	vcols.reserve(ncols);
	for (size_t i=0; i<ncols; i++)
		vcols.emplace_back(createLinkValue());

	for (ValuePtr vp: vrows)
	{
		if (vp->is_type(FLOAT_VALUE))
		{
			const std::vector<double>& vals = FloatValueCast(vp)->value();
			CHKSZ(vals);
			for (size_t i=0; i< ncols; i++)
				LinkValueCast(vcols[i]) -> _value.emplace_back(
					createFloatValue(vals[i]));
		}
		else if (vp->is_type(STRING_VALUE))
		{
			const std::vector<std::string>& vals = StringValueCast(vp)->value();
			CHKSZ(vals);
			for (size_t i=0; i< ncols; i++)
				LinkValueCast(vcols[i]) -> _value.emplace_back(
					createStringValue(vals[i]));
		}
		else if (vp->is_type(NUMBER_NODE))
		{
			const std::vector<double>& vals = NumberNodeCast(vp)->value();
			CHKSZ(vals);
			for (size_t i=0; i< ncols; i++)
				LinkValueCast(vcols[i]) -> _value.emplace_back(
					createFloatValue(vals[i]));
		}
		else if (vp->is_link())
		{
			const HandleSeq& vals = HandleCast(vp)->getOutgoingSet();
			CHKSZ(vals);
			for (size_t i=0; i< ncols; i++)
				LinkValueCast(vcols[i]) -> _value.push_back(vals[i]);
		}
	}
	return createLinkValue(std::move(vcols));
}

// ---------------------------------------------------------------

/// Return a FloatValue vector.
ValuePtr TransposeColumn::do_execute(AtomSpace* as, bool silent)
{
	// If the given Atom is executable, then execute it.
	Handle base(_outgoing[0]);
	if (base->is_executable())
	{
		ValuePtr vpe(base->execute(as, silent));
		if (vpe->is_atom())
			base = HandleCast(vpe);
		else
		{
			if (vpe->is_type(LINK_VALUE))
				return do_value_loop(as, silent, LinkValueCast(vpe)->value());

			ValueSeq vsq;
			vsq.emplace_back(vpe);
			return do_value_loop(as, silent, vsq);
		}
	}

	// If we are here, then base is an link. Expect
	// it to contain things that evaluate to a double
	return do_handle_loop(as, silent, base->getOutgoingSet());
}

// ---------------------------------------------------------------

/// Return a FloatValue vector.
ValuePtr TransposeColumn::execute(AtomSpace* as, bool silent)
{
	if (1 == _outgoing.size())
		return do_execute(as, silent);

	return do_handle_loop(as, silent, _outgoing);
}

DEFINE_LINK_FACTORY(TransposeColumn, TRANSPOSE_COLUMN)

/* ===================== END OF FILE ===================== */
