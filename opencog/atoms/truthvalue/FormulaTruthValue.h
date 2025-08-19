/*
 * opencog/atoms/truthvalue/FormulaTruthValue.h
 *
 * Copyright (C) 2020 Linas Vepstas
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

#ifndef _OPENCOG_FORMULA_TRUTH_VALUE_H_
#define _OPENCOG_FORMULA_TRUTH_VALUE_H_

#include <opencog/atoms/base/Handle.h>
#include <opencog/atomspace/AtomSpace.h>
#include <opencog/atoms/truthvalue/SimpleTruthValue.h>

namespace opencog
{
/** \addtogroup grp_atomspace
 *  @{
 */

//! A TruthValue that recomputes the TV from a stored formula.
//! This can be either a single stored predicate, or a pair of
//! formulas that each return a single number.
class FormulaTruthValue : public SimpleTruthValue
{
protected:
	void init(void);
	virtual void update(void) const;
	HandleSeq _formula;
	AtomSpace* _as;

public:
	FormulaTruthValue(const Handle&);
	FormulaTruthValue(const Handle&, const Handle&);
	FormulaTruthValue(const HandleSeq&&);
	virtual ~FormulaTruthValue();

	std::string to_string(const std::string&) const;

	/// Update the values. The get_confidence() method is NOT
	/// overloaded because only one update is needed/desired.
	virtual strength_t get_mean() const;
};

VALUE_PTR_DECL(FormulaTruthValue);
CREATE_VALUE_DECL(FormulaTruthValue);

/** @}*/
} // namespace opencog

#endif // _OPENCOG_FORMULA_TRUTH_VALUE_H_
