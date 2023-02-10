/* Copyright (c) 2018-2023 Griefer@Work                                       *
 *                                                                            *
 * This software is provided 'as-is', without any express or implied          *
 * warranty. In no event will the authors be held liable for any damages      *
 * arising from the use of this software.                                     *
 *                                                                            *
 * Permission is granted to anyone to use this software for any purpose,      *
 * including commercial applications, and to alter it and redistribute it     *
 * freely, subject to the following restrictions:                             *
 *                                                                            *
 * 1. The origin of this software must not be misrepresented; you must not    *
 *    claim that you wrote the original software. If you use this software    *
 *    in a product, an acknowledgement (see the following) in the product     *
 *    documentation is required:                                              *
 *    Portions Copyright (c) 2018-2023 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_CXX_SEQUENCE_H
#define GUARD_DEEMON_CXX_SEQUENCE_H 1

#include "api.h"
/**/

#include "object.h"
/**/

#include "../numeric.h"

DEE_CXX_BEGIN

template<class T>
class Sequence
	: public Object
	, public detail::ItemProxyAccessor<Sequence<T>, T>
	, public detail::RangeProxyAccessor<Sequence<T>, Sequence<T> >
{
public:
	using detail::ItemProxyAccessor<Sequence<T>, T>::iterator;
	using detail::ItemProxyAccessor<Sequence<T>, T>::begin;
	using detail::ItemProxyAccessor<Sequence<T>, T>::end;
	using detail::ItemProxyAccessor<Sequence<T>, T>::item;
	using detail::ItemProxyAccessor<Sequence<T>, T>::getitem;
	using detail::ItemProxyAccessor<Sequence<T>, T>::delitem;
	using detail::ItemProxyAccessor<Sequence<T>, T>::setitem;
	using detail::ItemProxyAccessor<Sequence<T>, T>::hasitem;
	using detail::ItemProxyAccessor<Sequence<T>, T>::bounditem;
	using detail::RangeProxyAccessor<Sequence<T>, Sequence<T> >::range;
	using detail::RangeProxyAccessor<Sequence<T>, Sequence<T> >::getrange;
	using detail::RangeProxyAccessor<Sequence<T>, Sequence<T> >::delrange;
	using detail::RangeProxyAccessor<Sequence<T>, Sequence<T> >::setrange;

	/* TODO: Deemon doc text needs a way to expression generic types, so we're
	 *       somehow able to encode RTTI information such that the runtime will
	 *       know that (e.g.) `Sequence<T>.frozen' returns `Sequence<T>'
	 * This also goes for deemon code, where `{string...}' is `Sequence<T>'.
	 * Also, get rid of the "Cell with int" syntax. - Instead, the syntax
	 * used should be "Cell<int>" (which simply feels much more natural).
	 *
	 * Also: it should be possible for user-defined deemon types to be written
	 *       such that they take template parameters:
	 * >> // "List<T>" here is parsed like "List", but the parameter is stored in RTTI and may be used by an IDE
	 * >> class MyList<T>: List<T> {
	 * >>     public operator [](index: int): T {
	 * >>         return super[index];
	 * >>     }
	 * >> }
	 *
	 * The existence of the template parameter "T" must then be stored within
	 * the type's doc-string, and use of "T" in doc strings of members must
	 * reference the template parameter of the surrounding class. The same must
	 * then also be possible for user-defined functions:
	 * >> function getitem<T>(seq: {T...}, index: int): T {
	 * >>     return seq[index];
	 * >> }
	 *
	 * When a template parameter list is specified alongside a type, it must
	 * always be complete (i.e. have the same # of parameters as are used by
	 * that type). If a template parameter list is omitted, behavior is the
	 * same as though all parameters were "Object from deemon".
	 *
	 * Anonymous classes and functions can't have template parameter lists.
	 */
/*[[[deemon (CxxType from rt.gen.cxxapi)(Sequence from deemon).printCxxApi();]]]*/
	// TODO: function reduce
	// TODO: function filter
	// TODO: function sum
	// TODO: function any
	// TODO: function all
	// TODO: function parity
	// TODO: function min
	// TODO: function max
	// TODO: function count
	// TODO: function locate
	// TODO: function rlocate
	// TODO: function locateall
	// TODO: function transform
	// TODO: function contains
	// TODO: function startswith
	// TODO: function endswith
	// TODO: function find
	// TODO: function rfind
	// TODO: function index
	// TODO: function rindex
	// TODO: function reversed
	// TODO: function sorted
	// TODO: function segments
	// TODO: function distribute
	// TODO: function combinations
	// TODO: function repeatcombinations
	// TODO: function permutations
	// TODO: function insert
	// TODO: function insertall
	// TODO: function append
	// TODO: function extend
	// TODO: function erase
	// TODO: function xch
	// TODO: function pop
	// TODO: function popfront
	// TODO: function popback
	// TODO: function pushfront
	// TODO: function pushback
	// TODO: function remove
	// TODO: function rremove
	// TODO: function removeall
	// TODO: function removeif
	// TODO: function clear
	// TODO: function resize
	// TODO: function fill
	// TODO: function reverse
	// TODO: function sort
	// TODO: function byhash
	// TODO: function bfind
	// TODO: function bcontains
	// TODO: function bindex
	// TODO: function bposition
	// TODO: function brange
	// TODO: function blocate
	// TODO: function blocateall
	// TODO: function binsert
	// TODO: function front
	// TODO: function back
	// TODO: function empty
	// TODO: function non_empty
	// TODO: function at
	// TODO: function get
	WUNUSED deemon::Ref<deemon::int_> length() {
		return inherit(DeeObject_GetAttrStringHash(this, "length", _Dee_HashSelect(UINT32_C(0xecef0c1), UINT64_C(0x2993e8eb119cab21))));
	}
	WUNUSED deemon::Ref<deemon::Object> first() {
		return inherit(DeeObject_GetAttrStringHash(this, "first", _Dee_HashSelect(UINT32_C(0xa9f0e818), UINT64_C(0x9d12a485470a29a7))));
	}
	WUNUSED deemon::Ref<deemon::Object> last() {
		return inherit(DeeObject_GetAttrStringHash(this, "last", _Dee_HashSelect(UINT32_C(0x185a4f9a), UINT64_C(0x760894ca6d41e4dc))));
	}
	WUNUSED deemon::Ref<deemon::bool_> ismutable() {
		return inherit(DeeObject_GetAttrStringHash(this, "ismutable", _Dee_HashSelect(UINT32_C(0x503e0af), UINT64_C(0x218befd96d70f7d4))));
	}
	WUNUSED deemon::Ref<deemon::bool_> isresizable() {
		return inherit(DeeObject_GetAttrStringHash(this, "isresizable", _Dee_HashSelect(UINT32_C(0x8e5e51ee), UINT64_C(0x11d0a53385154152))));
	}
	WUNUSED deemon::Ref<Sequence<deemon::Object> > each() {
		return inherit(DeeObject_GetAttrStringHash(this, "each", _Dee_HashSelect(UINT32_C(0x9de8b13d), UINT64_C(0x374e052f37a5e158))));
	}
	WUNUSED deemon::Ref<Sequence<deemon::int_> > ids() {
		return inherit(DeeObject_GetAttrStringHash(this, "ids", _Dee_HashSelect(UINT32_C(0x3173a48f), UINT64_C(0x7cd9fae6cf17bb9f))));
	}
	WUNUSED deemon::Ref<Sequence<deemon::Type> > types() {
		return inherit(DeeObject_GetAttrStringHash(this, "types", _Dee_HashSelect(UINT32_C(0x871b2836), UINT64_C(0xde8693a2d24930))));
	}
	WUNUSED deemon::Ref<Sequence<deemon::Type> > classes() {
		return inherit(DeeObject_GetAttrStringHash(this, "classes", _Dee_HashSelect(UINT32_C(0x75e5899b), UINT64_C(0xc75d2d970415e4a0))));
	}
	WUNUSED deemon::Ref<deemon::bool_> isempty() {
		return inherit(DeeObject_GetAttrStringHash(this, "isempty", _Dee_HashSelect(UINT32_C(0x693ca74b), UINT64_C(0xbdd6a8314c184088))));
	}
	WUNUSED deemon::Ref<deemon::bool_> isnonempty() {
		return inherit(DeeObject_GetAttrStringHash(this, "isnonempty", _Dee_HashSelect(UINT32_C(0xde7e1cb1), UINT64_C(0x837d8f16bac4b317))));
	}
	WUNUSED deemon::Ref<deemon::bool_> isfrozen() {
		return inherit(DeeObject_GetAttrStringHash(this, "isfrozen", _Dee_HashSelect(UINT32_C(0xb418a8ca), UINT64_C(0xa1babf165e1b1733))));
	}
	WUNUSED deemon::Ref<deemon::Sequence<> > frozen() {
		return inherit(DeeObject_GetAttrStringHash(this, "frozen", _Dee_HashSelect(UINT32_C(0x82311b77), UINT64_C(0x7b55e2e6e642b6fd))));
	}
/*[[[end]]]*/
};

DEE_CXX_END

#endif /* !GUARD_DEEMON_CXX_SEQUENCE_H */
