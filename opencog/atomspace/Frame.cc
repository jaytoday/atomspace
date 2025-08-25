/*
 * opencog/atomspace/Frame.cc
 *
 * Copyright (c) 2022 Linas Vepstas
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

#include <opencog/atoms/atom_types/NameServer.h>

#include "Frame.h"

using namespace opencog;

void Frame::init()
{
	if (not nameserver().isA(_type, FRAME))
		throw InvalidParamException(TRACE_INFO, "Not a Frame!");

	// Set up the incoming set.
	keep_incoming_set();

	// Cannot call shared_from_this() in the ctor, so cannot call
	// install() here.
	// install();
}

Frame::~Frame()
{
	// Cannot call this in the dtor, because cannot call
	// shared_from_this() in the dtor.
	// remove();

	// Because we cannot remove ourselves directly, via above,
	// we can at least remove other dead weak pointers.
	for (Handle& h : _outgoing)
		FrameCast(h)->scrub_incoming_set();
}

/// Place `this` into the incoming set of each outgoing frame.
void Frame::install()
{
	Handle llc(get_handle());
	for (Handle& h : _outgoing)
		h->insert_atom(llc);
}

void Frame::remove()
{
	Handle lll(get_handle());
	for (Handle& h : _outgoing)
		h->remove_atom(lll);
}

/// Remove all dead frames in the incoming set.
void Frame::scrub_incoming_set(void)
{
	if (not (_flags.load() & USE_ISET_FLAG)) return;
#if USE_BARE_BACKPOINTER
	// This won't work with bare pointers. Which means we have a
	// problem with the validity of the incoming set for Frames:
	// it will include Frames that have been deleted, and thus
	// pointing at freed memory. Oddly enough, no unit test seems
	// to trip on this. But .. well, I guess it's uhh.. maybe bad
	// luck, eh?
	#warning "Using AtomSpace frames with bare pointers is asking for trouble!"
#else
	INCOMING_UNIQUE_LOCK;

	// Iterate over all frame types
	std::vector<Type> framet;
	nameserver().getChildrenRecursive(FRAME, back_inserter(framet));
	for (Type t : framet)
	{
		auto bucket = _incoming_set._iset.find(t);
		for (auto bi = bucket->second.begin(); bi != bucket->second.end();)
		{
			if (0 == bi->use_count())
#if HAVE_SPARSEHASH
				// sparsehash erase does not invalidate iterators.
				bucket->second.erase(bi);
			bi++;
#else
				bi = bucket->second.erase(bi);
			else bi++;
#endif
		}
	}
#endif
}
