/* Copyright (c) 2018-2024 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2024 Griefer@Work                           *
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

#include "../format.h"
#include "../seq.h"

DEE_CXX_BEGIN

template<class T>
class Sequence
	: public Object
	, public detail::ItemProxyAccessor<Sequence<T>, T>
	, public detail::RangeProxyAccessor<Sequence<T>, Sequence<T> >
{
public:
	static WUNUSED Type &classtype() DEE_CXX_NOTHROW {
		return *(Type *)&DeeSeq_Type;
	}
	static WUNUSED NONNULL_CXX((1)) bool check(DeeObject *ob) DEE_CXX_NOTHROW {
		return DeeObject_InstanceOf(ob, &DeeSeq_Type);
	}
	static WUNUSED NONNULL_CXX((1)) bool checkexact(DeeObject *ob) DEE_CXX_NOTHROW {
		return DeeObject_InstanceOfExact(ob, &DeeSeq_Type);
	}

public:
	static Ref<Sequence<T> > of() {
		return nonnull(Dee_EmptySeq);
	}
	static Ref<Sequence<T> > range(DeeObject *start, DeeObject *end, DeeObject *step = NULL) {
		return inherit(DeeRange_New(start, end, step));
	}
	static Ref<Sequence<T> > range(Dee_ssize_t start, Dee_ssize_t end, Dee_ssize_t step = 1) {
		return inherit(DeeRange_NewInt(start, end, step));
	}

public:
	using detail::ItemProxyAccessor<Sequence<T>, T>::iterator;
	using detail::ItemProxyAccessor<Sequence<T>, T>::iter;
	using detail::ItemProxyAccessor<Sequence<T>, T>::begin;
	using detail::ItemProxyAccessor<Sequence<T>, T>::end;
	using detail::ItemProxyAccessor<Sequence<T>, T>::operator[];
	using detail::ItemProxyAccessor<Sequence<T>, T>::item;
	using detail::ItemProxyAccessor<Sequence<T>, T>::hasitem;
	using detail::ItemProxyAccessor<Sequence<T>, T>::getitem;
	using detail::ItemProxyAccessor<Sequence<T>, T>::bounditem;
	using detail::ItemProxyAccessor<Sequence<T>, T>::delitem;
	using detail::ItemProxyAccessor<Sequence<T>, T>::setitem;
	using detail::RangeProxyAccessor<Sequence<T>, Sequence<T> >::range;
	using detail::RangeProxyAccessor<Sequence<T>, Sequence<T> >::getrange;
	using detail::RangeProxyAccessor<Sequence<T>, Sequence<T> >::delrange;
	using detail::RangeProxyAccessor<Sequence<T>, Sequence<T> >::setrange;

public:
	Ref<T> csum() {
		return inherit(DeeSeq_Sum(this));
	}
	bool cany() {
		return throw_if_negative(DeeSeq_Any(this)) > 0;
	}
	bool call() {
		return throw_if_negative(DeeSeq_All(this)) > 0;
	}
	Ref<T> cmin() {
		return inherit(DeeSeq_Min(this));
	}
	Ref<T> cmax() {
		return inherit(DeeSeq_Max(this));
	}

public:
/*[[[deemon (CxxType from rt.gen.cxxapi)(Sequence from deemon).printCxxApi(templateParameters: { "T" });]]]*/
	WUNUSED NONNULL_CXX((1)) Ref<T> (reduce)(DeeObject *combine) {
		DeeObject *args[1];
		args[0] = combine;
		return inherit(DeeObject_CallAttrStringHash(this, "reduce", _Dee_HashSelectC(0x907b2992, 0xb7e66094a8a9409d), 1, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<T> (reduce)(DeeObject *combine, DeeObject *start) {
		DeeObject *args[2];
		args[0] = combine;
		args[1] = start;
		return inherit(DeeObject_CallAttrStringHash(this, "reduce", _Dee_HashSelectC(0x907b2992, 0xb7e66094a8a9409d), 2, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3)) Ref<T> (reduce)(DeeObject *combine, DeeObject *start, DeeObject *end) {
		DeeObject *args[3];
		args[0] = combine;
		args[1] = start;
		args[2] = end;
		return inherit(DeeObject_CallAttrStringHash(this, "reduce", _Dee_HashSelectC(0x907b2992, 0xb7e66094a8a9409d), 3, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3, 4)) Ref<T> (reduce)(DeeObject *combine, DeeObject *start, DeeObject *end, DeeObject *init) {
		DeeObject *args[4];
		args[0] = combine;
		args[1] = start;
		args[2] = end;
		args[3] = init;
		return inherit(DeeObject_CallAttrStringHash(this, "reduce", _Dee_HashSelectC(0x907b2992, 0xb7e66094a8a9409d), 4, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<T> (reduce)(DeeObject *combine, DeeObject *start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "reduce", _Dee_HashSelectC(0x907b2992, 0xb7e66094a8a9409d), "oo" DEE_PCKdSIZ, combine, start, end));
	}
	WUNUSED NONNULL_CXX((1, 2, 4)) Ref<T> (reduce)(DeeObject *combine, DeeObject *start, Dee_ssize_t end, DeeObject *init) {
		return inherit(DeeObject_CallAttrStringHashf(this, "reduce", _Dee_HashSelectC(0x907b2992, 0xb7e66094a8a9409d), "oo" DEE_PCKdSIZ "o", combine, start, end, init));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<T> (reduce)(DeeObject *combine, DeeObject *start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "reduce", _Dee_HashSelectC(0x907b2992, 0xb7e66094a8a9409d), "oo" DEE_PCKuSIZ, combine, start, end));
	}
	WUNUSED NONNULL_CXX((1, 2, 4)) Ref<T> (reduce)(DeeObject *combine, DeeObject *start, size_t end, DeeObject *init) {
		return inherit(DeeObject_CallAttrStringHashf(this, "reduce", _Dee_HashSelectC(0x907b2992, 0xb7e66094a8a9409d), "oo" DEE_PCKuSIZ "o", combine, start, end, init));
	}
	WUNUSED NONNULL_CXX((1)) Ref<T> (reduce)(DeeObject *combine, Dee_ssize_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "reduce", _Dee_HashSelectC(0x907b2992, 0xb7e66094a8a9409d), "o" DEE_PCKdSIZ, combine, start));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<T> (reduce)(DeeObject *combine, Dee_ssize_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "reduce", _Dee_HashSelectC(0x907b2992, 0xb7e66094a8a9409d), "o" DEE_PCKdSIZ "o", combine, start, end));
	}
	WUNUSED NONNULL_CXX((1, 3, 4)) Ref<T> (reduce)(DeeObject *combine, Dee_ssize_t start, DeeObject *end, DeeObject *init) {
		return inherit(DeeObject_CallAttrStringHashf(this, "reduce", _Dee_HashSelectC(0x907b2992, 0xb7e66094a8a9409d), "o" DEE_PCKdSIZ "oo", combine, start, end, init));
	}
	WUNUSED NONNULL_CXX((1)) Ref<T> (reduce)(DeeObject *combine, Dee_ssize_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "reduce", _Dee_HashSelectC(0x907b2992, 0xb7e66094a8a9409d), "o" DEE_PCKdSIZ DEE_PCKdSIZ, combine, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<T> (reduce)(DeeObject *combine, Dee_ssize_t start, Dee_ssize_t end, DeeObject *init) {
		return inherit(DeeObject_CallAttrStringHashf(this, "reduce", _Dee_HashSelectC(0x907b2992, 0xb7e66094a8a9409d), "o" DEE_PCKdSIZ DEE_PCKdSIZ "o", combine, start, end, init));
	}
	WUNUSED NONNULL_CXX((1)) Ref<T> (reduce)(DeeObject *combine, Dee_ssize_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "reduce", _Dee_HashSelectC(0x907b2992, 0xb7e66094a8a9409d), "o" DEE_PCKdSIZ DEE_PCKuSIZ, combine, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<T> (reduce)(DeeObject *combine, Dee_ssize_t start, size_t end, DeeObject *init) {
		return inherit(DeeObject_CallAttrStringHashf(this, "reduce", _Dee_HashSelectC(0x907b2992, 0xb7e66094a8a9409d), "o" DEE_PCKdSIZ DEE_PCKuSIZ "o", combine, start, end, init));
	}
	WUNUSED NONNULL_CXX((1)) Ref<T> (reduce)(DeeObject *combine, size_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "reduce", _Dee_HashSelectC(0x907b2992, 0xb7e66094a8a9409d), "o" DEE_PCKuSIZ, combine, start));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<T> (reduce)(DeeObject *combine, size_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "reduce", _Dee_HashSelectC(0x907b2992, 0xb7e66094a8a9409d), "o" DEE_PCKuSIZ "o", combine, start, end));
	}
	WUNUSED NONNULL_CXX((1, 3, 4)) Ref<T> (reduce)(DeeObject *combine, size_t start, DeeObject *end, DeeObject *init) {
		return inherit(DeeObject_CallAttrStringHashf(this, "reduce", _Dee_HashSelectC(0x907b2992, 0xb7e66094a8a9409d), "o" DEE_PCKuSIZ "oo", combine, start, end, init));
	}
	WUNUSED NONNULL_CXX((1)) Ref<T> (reduce)(DeeObject *combine, size_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "reduce", _Dee_HashSelectC(0x907b2992, 0xb7e66094a8a9409d), "o" DEE_PCKuSIZ DEE_PCKdSIZ, combine, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<T> (reduce)(DeeObject *combine, size_t start, Dee_ssize_t end, DeeObject *init) {
		return inherit(DeeObject_CallAttrStringHashf(this, "reduce", _Dee_HashSelectC(0x907b2992, 0xb7e66094a8a9409d), "o" DEE_PCKuSIZ DEE_PCKdSIZ "o", combine, start, end, init));
	}
	WUNUSED NONNULL_CXX((1)) Ref<T> (reduce)(DeeObject *combine, size_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "reduce", _Dee_HashSelectC(0x907b2992, 0xb7e66094a8a9409d), "o" DEE_PCKuSIZ DEE_PCKuSIZ, combine, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<T> (reduce)(DeeObject *combine, size_t start, size_t end, DeeObject *init) {
		return inherit(DeeObject_CallAttrStringHashf(this, "reduce", _Dee_HashSelectC(0x907b2992, 0xb7e66094a8a9409d), "o" DEE_PCKuSIZ DEE_PCKuSIZ "o", combine, start, end, init));
	}
	WUNUSED Ref<Sequence<_AbstractTuple<deemon::int_, T> > > (enumerate)() {
		return inherit(DeeObject_CallAttrStringHash(this, "enumerate", _Dee_HashSelectC(0x990a48c9, 0x8514809a12261fe3), 0, NULL));
	}
	WUNUSED NONNULL_CXX((1)) Ref<T> (enumerate)(DeeObject *start_or_cb) {
		DeeObject *args[1];
		args[0] = start_or_cb;
		return inherit(DeeObject_CallAttrStringHash(this, "enumerate", _Dee_HashSelectC(0x990a48c9, 0x8514809a12261fe3), 1, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<T> (enumerate)(DeeObject *start_or_cb, DeeObject *end_or_start) {
		DeeObject *args[2];
		args[0] = start_or_cb;
		args[1] = end_or_start;
		return inherit(DeeObject_CallAttrStringHash(this, "enumerate", _Dee_HashSelectC(0x990a48c9, 0x8514809a12261fe3), 2, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3)) Ref<T> (enumerate)(DeeObject *cb, DeeObject *start, DeeObject *end) {
		DeeObject *args[3];
		args[0] = cb;
		args[1] = start;
		args[2] = end;
		return inherit(DeeObject_CallAttrStringHash(this, "enumerate", _Dee_HashSelectC(0x990a48c9, 0x8514809a12261fe3), 3, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<T> (enumerate)(DeeObject *cb, DeeObject *start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "enumerate", _Dee_HashSelectC(0x990a48c9, 0x8514809a12261fe3), "oo" DEE_PCKdSIZ, cb, start, end));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<T> (enumerate)(DeeObject *cb, DeeObject *start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "enumerate", _Dee_HashSelectC(0x990a48c9, 0x8514809a12261fe3), "oo" DEE_PCKuSIZ, cb, start, end));
	}
	WUNUSED NONNULL_CXX((1)) Ref<T> (enumerate)(DeeObject *start_or_cb, Dee_ssize_t end_or_start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "enumerate", _Dee_HashSelectC(0x990a48c9, 0x8514809a12261fe3), "o" DEE_PCKdSIZ, start_or_cb, end_or_start));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<T> (enumerate)(DeeObject *cb, Dee_ssize_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "enumerate", _Dee_HashSelectC(0x990a48c9, 0x8514809a12261fe3), "o" DEE_PCKdSIZ "o", cb, start, end));
	}
	WUNUSED NONNULL_CXX((1)) Ref<T> (enumerate)(DeeObject *cb, Dee_ssize_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "enumerate", _Dee_HashSelectC(0x990a48c9, 0x8514809a12261fe3), "o" DEE_PCKdSIZ DEE_PCKdSIZ, cb, start, end));
	}
	WUNUSED NONNULL_CXX((1)) Ref<T> (enumerate)(DeeObject *cb, Dee_ssize_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "enumerate", _Dee_HashSelectC(0x990a48c9, 0x8514809a12261fe3), "o" DEE_PCKdSIZ DEE_PCKuSIZ, cb, start, end));
	}
	WUNUSED NONNULL_CXX((1)) Ref<T> (enumerate)(DeeObject *start_or_cb, size_t end_or_start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "enumerate", _Dee_HashSelectC(0x990a48c9, 0x8514809a12261fe3), "o" DEE_PCKuSIZ, start_or_cb, end_or_start));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<T> (enumerate)(DeeObject *cb, size_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "enumerate", _Dee_HashSelectC(0x990a48c9, 0x8514809a12261fe3), "o" DEE_PCKuSIZ "o", cb, start, end));
	}
	WUNUSED NONNULL_CXX((1)) Ref<T> (enumerate)(DeeObject *cb, size_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "enumerate", _Dee_HashSelectC(0x990a48c9, 0x8514809a12261fe3), "o" DEE_PCKuSIZ DEE_PCKdSIZ, cb, start, end));
	}
	WUNUSED NONNULL_CXX((1)) Ref<T> (enumerate)(DeeObject *cb, size_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "enumerate", _Dee_HashSelectC(0x990a48c9, 0x8514809a12261fe3), "o" DEE_PCKuSIZ DEE_PCKuSIZ, cb, start, end));
	}
	WUNUSED Ref<Sequence<_AbstractTuple<deemon::int_, T> > > (enumerate)(Dee_ssize_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "enumerate", _Dee_HashSelectC(0x990a48c9, 0x8514809a12261fe3),  DEE_PCKdSIZ, start));
	}
	WUNUSED NONNULL_CXX((2)) Ref<Sequence<_AbstractTuple<deemon::int_, T> > > (enumerate)(Dee_ssize_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "enumerate", _Dee_HashSelectC(0x990a48c9, 0x8514809a12261fe3),  DEE_PCKdSIZ "o", start, end));
	}
	WUNUSED Ref<Sequence<_AbstractTuple<deemon::int_, T> > > (enumerate)(Dee_ssize_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "enumerate", _Dee_HashSelectC(0x990a48c9, 0x8514809a12261fe3),  DEE_PCKdSIZ DEE_PCKdSIZ, start, end));
	}
	WUNUSED Ref<Sequence<_AbstractTuple<deemon::int_, T> > > (enumerate)(Dee_ssize_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "enumerate", _Dee_HashSelectC(0x990a48c9, 0x8514809a12261fe3),  DEE_PCKdSIZ DEE_PCKuSIZ, start, end));
	}
	WUNUSED Ref<Sequence<_AbstractTuple<deemon::int_, T> > > (enumerate)(size_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "enumerate", _Dee_HashSelectC(0x990a48c9, 0x8514809a12261fe3),  DEE_PCKuSIZ, start));
	}
	WUNUSED NONNULL_CXX((2)) Ref<Sequence<_AbstractTuple<deemon::int_, T> > > (enumerate)(size_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "enumerate", _Dee_HashSelectC(0x990a48c9, 0x8514809a12261fe3),  DEE_PCKuSIZ "o", start, end));
	}
	WUNUSED Ref<Sequence<_AbstractTuple<deemon::int_, T> > > (enumerate)(size_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "enumerate", _Dee_HashSelectC(0x990a48c9, 0x8514809a12261fe3),  DEE_PCKuSIZ DEE_PCKdSIZ, start, end));
	}
	WUNUSED Ref<Sequence<_AbstractTuple<deemon::int_, T> > > (enumerate)(size_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "enumerate", _Dee_HashSelectC(0x990a48c9, 0x8514809a12261fe3),  DEE_PCKuSIZ DEE_PCKuSIZ, start, end));
	}
	WUNUSED Ref<T> (sum)() {
		return inherit(DeeObject_CallAttrStringHash(this, "sum", _Dee_HashSelectC(0xfdc548bb, 0x69e7b5a62f6dbc17), 0, NULL));
	}
	WUNUSED NONNULL_CXX((1)) Ref<T> (sum)(DeeObject *start) {
		DeeObject *args[1];
		args[0] = start;
		return inherit(DeeObject_CallAttrStringHash(this, "sum", _Dee_HashSelectC(0xfdc548bb, 0x69e7b5a62f6dbc17), 1, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<T> (sum)(DeeObject *start, DeeObject *end) {
		DeeObject *args[2];
		args[0] = start;
		args[1] = end;
		return inherit(DeeObject_CallAttrStringHash(this, "sum", _Dee_HashSelectC(0xfdc548bb, 0x69e7b5a62f6dbc17), 2, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<T> (sum)(DeeObject *start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "sum", _Dee_HashSelectC(0xfdc548bb, 0x69e7b5a62f6dbc17), "o" DEE_PCKdSIZ, start, end));
	}
	WUNUSED NONNULL_CXX((1)) Ref<T> (sum)(DeeObject *start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "sum", _Dee_HashSelectC(0xfdc548bb, 0x69e7b5a62f6dbc17), "o" DEE_PCKuSIZ, start, end));
	}
	WUNUSED Ref<T> (sum)(Dee_ssize_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "sum", _Dee_HashSelectC(0xfdc548bb, 0x69e7b5a62f6dbc17),  DEE_PCKdSIZ, start));
	}
	WUNUSED NONNULL_CXX((2)) Ref<T> (sum)(Dee_ssize_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "sum", _Dee_HashSelectC(0xfdc548bb, 0x69e7b5a62f6dbc17),  DEE_PCKdSIZ "o", start, end));
	}
	WUNUSED Ref<T> (sum)(Dee_ssize_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "sum", _Dee_HashSelectC(0xfdc548bb, 0x69e7b5a62f6dbc17),  DEE_PCKdSIZ DEE_PCKdSIZ, start, end));
	}
	WUNUSED Ref<T> (sum)(Dee_ssize_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "sum", _Dee_HashSelectC(0xfdc548bb, 0x69e7b5a62f6dbc17),  DEE_PCKdSIZ DEE_PCKuSIZ, start, end));
	}
	WUNUSED Ref<T> (sum)(size_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "sum", _Dee_HashSelectC(0xfdc548bb, 0x69e7b5a62f6dbc17),  DEE_PCKuSIZ, start));
	}
	WUNUSED NONNULL_CXX((2)) Ref<T> (sum)(size_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "sum", _Dee_HashSelectC(0xfdc548bb, 0x69e7b5a62f6dbc17),  DEE_PCKuSIZ "o", start, end));
	}
	WUNUSED Ref<T> (sum)(size_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "sum", _Dee_HashSelectC(0xfdc548bb, 0x69e7b5a62f6dbc17),  DEE_PCKuSIZ DEE_PCKdSIZ, start, end));
	}
	WUNUSED Ref<T> (sum)(size_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "sum", _Dee_HashSelectC(0xfdc548bb, 0x69e7b5a62f6dbc17),  DEE_PCKuSIZ DEE_PCKuSIZ, start, end));
	}
	WUNUSED Ref<deemon::bool_> (any)() {
		return inherit(DeeObject_CallAttrStringHash(this, "any", _Dee_HashSelectC(0x8125d3c6, 0xb4b817103f9d84ef), 0, NULL));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (any)(DeeObject *start) {
		DeeObject *args[1];
		args[0] = start;
		return inherit(DeeObject_CallAttrStringHash(this, "any", _Dee_HashSelectC(0x8125d3c6, 0xb4b817103f9d84ef), 1, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::bool_> (any)(DeeObject *start, DeeObject *end) {
		DeeObject *args[2];
		args[0] = start;
		args[1] = end;
		return inherit(DeeObject_CallAttrStringHash(this, "any", _Dee_HashSelectC(0x8125d3c6, 0xb4b817103f9d84ef), 2, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3)) Ref<deemon::bool_> (any)(DeeObject *start, DeeObject *end, DeeObject *key) {
		DeeObject *args[3];
		args[0] = start;
		args[1] = end;
		args[2] = key;
		return inherit(DeeObject_CallAttrStringHash(this, "any", _Dee_HashSelectC(0x8125d3c6, 0xb4b817103f9d84ef), 3, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (any)(DeeObject *start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "any", _Dee_HashSelectC(0x8125d3c6, 0xb4b817103f9d84ef), "o" DEE_PCKdSIZ, start, end));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::bool_> (any)(DeeObject *start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "any", _Dee_HashSelectC(0x8125d3c6, 0xb4b817103f9d84ef), "o" DEE_PCKdSIZ "o", start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (any)(DeeObject *start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "any", _Dee_HashSelectC(0x8125d3c6, 0xb4b817103f9d84ef), "o" DEE_PCKuSIZ, start, end));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::bool_> (any)(DeeObject *start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "any", _Dee_HashSelectC(0x8125d3c6, 0xb4b817103f9d84ef), "o" DEE_PCKuSIZ "o", start, end, key));
	}
	WUNUSED Ref<deemon::bool_> (any)(Dee_ssize_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "any", _Dee_HashSelectC(0x8125d3c6, 0xb4b817103f9d84ef),  DEE_PCKdSIZ, start));
	}
	WUNUSED NONNULL_CXX((2)) Ref<deemon::bool_> (any)(Dee_ssize_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "any", _Dee_HashSelectC(0x8125d3c6, 0xb4b817103f9d84ef),  DEE_PCKdSIZ "o", start, end));
	}
	WUNUSED NONNULL_CXX((2, 3)) Ref<deemon::bool_> (any)(Dee_ssize_t start, DeeObject *end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "any", _Dee_HashSelectC(0x8125d3c6, 0xb4b817103f9d84ef),  DEE_PCKdSIZ "oo", start, end, key));
	}
	WUNUSED Ref<deemon::bool_> (any)(Dee_ssize_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "any", _Dee_HashSelectC(0x8125d3c6, 0xb4b817103f9d84ef),  DEE_PCKdSIZ DEE_PCKdSIZ, start, end));
	}
	WUNUSED NONNULL_CXX((3)) Ref<deemon::bool_> (any)(Dee_ssize_t start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "any", _Dee_HashSelectC(0x8125d3c6, 0xb4b817103f9d84ef),  DEE_PCKdSIZ DEE_PCKdSIZ "o", start, end, key));
	}
	WUNUSED Ref<deemon::bool_> (any)(Dee_ssize_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "any", _Dee_HashSelectC(0x8125d3c6, 0xb4b817103f9d84ef),  DEE_PCKdSIZ DEE_PCKuSIZ, start, end));
	}
	WUNUSED NONNULL_CXX((3)) Ref<deemon::bool_> (any)(Dee_ssize_t start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "any", _Dee_HashSelectC(0x8125d3c6, 0xb4b817103f9d84ef),  DEE_PCKdSIZ DEE_PCKuSIZ "o", start, end, key));
	}
	WUNUSED Ref<deemon::bool_> (any)(size_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "any", _Dee_HashSelectC(0x8125d3c6, 0xb4b817103f9d84ef),  DEE_PCKuSIZ, start));
	}
	WUNUSED NONNULL_CXX((2)) Ref<deemon::bool_> (any)(size_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "any", _Dee_HashSelectC(0x8125d3c6, 0xb4b817103f9d84ef),  DEE_PCKuSIZ "o", start, end));
	}
	WUNUSED NONNULL_CXX((2, 3)) Ref<deemon::bool_> (any)(size_t start, DeeObject *end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "any", _Dee_HashSelectC(0x8125d3c6, 0xb4b817103f9d84ef),  DEE_PCKuSIZ "oo", start, end, key));
	}
	WUNUSED Ref<deemon::bool_> (any)(size_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "any", _Dee_HashSelectC(0x8125d3c6, 0xb4b817103f9d84ef),  DEE_PCKuSIZ DEE_PCKdSIZ, start, end));
	}
	WUNUSED NONNULL_CXX((3)) Ref<deemon::bool_> (any)(size_t start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "any", _Dee_HashSelectC(0x8125d3c6, 0xb4b817103f9d84ef),  DEE_PCKuSIZ DEE_PCKdSIZ "o", start, end, key));
	}
	WUNUSED Ref<deemon::bool_> (any)(size_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "any", _Dee_HashSelectC(0x8125d3c6, 0xb4b817103f9d84ef),  DEE_PCKuSIZ DEE_PCKuSIZ, start, end));
	}
	WUNUSED NONNULL_CXX((3)) Ref<deemon::bool_> (any)(size_t start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "any", _Dee_HashSelectC(0x8125d3c6, 0xb4b817103f9d84ef),  DEE_PCKuSIZ DEE_PCKuSIZ "o", start, end, key));
	}
	WUNUSED Ref<deemon::bool_> (all)() {
		return inherit(DeeObject_CallAttrStringHash(this, "all", _Dee_HashSelectC(0xb7e328b, 0xa812dcfcacc34ebc), 0, NULL));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (all)(DeeObject *start) {
		DeeObject *args[1];
		args[0] = start;
		return inherit(DeeObject_CallAttrStringHash(this, "all", _Dee_HashSelectC(0xb7e328b, 0xa812dcfcacc34ebc), 1, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::bool_> (all)(DeeObject *start, DeeObject *end) {
		DeeObject *args[2];
		args[0] = start;
		args[1] = end;
		return inherit(DeeObject_CallAttrStringHash(this, "all", _Dee_HashSelectC(0xb7e328b, 0xa812dcfcacc34ebc), 2, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3)) Ref<deemon::bool_> (all)(DeeObject *start, DeeObject *end, DeeObject *key) {
		DeeObject *args[3];
		args[0] = start;
		args[1] = end;
		args[2] = key;
		return inherit(DeeObject_CallAttrStringHash(this, "all", _Dee_HashSelectC(0xb7e328b, 0xa812dcfcacc34ebc), 3, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (all)(DeeObject *start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "all", _Dee_HashSelectC(0xb7e328b, 0xa812dcfcacc34ebc), "o" DEE_PCKdSIZ, start, end));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::bool_> (all)(DeeObject *start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "all", _Dee_HashSelectC(0xb7e328b, 0xa812dcfcacc34ebc), "o" DEE_PCKdSIZ "o", start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (all)(DeeObject *start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "all", _Dee_HashSelectC(0xb7e328b, 0xa812dcfcacc34ebc), "o" DEE_PCKuSIZ, start, end));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::bool_> (all)(DeeObject *start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "all", _Dee_HashSelectC(0xb7e328b, 0xa812dcfcacc34ebc), "o" DEE_PCKuSIZ "o", start, end, key));
	}
	WUNUSED Ref<deemon::bool_> (all)(Dee_ssize_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "all", _Dee_HashSelectC(0xb7e328b, 0xa812dcfcacc34ebc),  DEE_PCKdSIZ, start));
	}
	WUNUSED NONNULL_CXX((2)) Ref<deemon::bool_> (all)(Dee_ssize_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "all", _Dee_HashSelectC(0xb7e328b, 0xa812dcfcacc34ebc),  DEE_PCKdSIZ "o", start, end));
	}
	WUNUSED NONNULL_CXX((2, 3)) Ref<deemon::bool_> (all)(Dee_ssize_t start, DeeObject *end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "all", _Dee_HashSelectC(0xb7e328b, 0xa812dcfcacc34ebc),  DEE_PCKdSIZ "oo", start, end, key));
	}
	WUNUSED Ref<deemon::bool_> (all)(Dee_ssize_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "all", _Dee_HashSelectC(0xb7e328b, 0xa812dcfcacc34ebc),  DEE_PCKdSIZ DEE_PCKdSIZ, start, end));
	}
	WUNUSED NONNULL_CXX((3)) Ref<deemon::bool_> (all)(Dee_ssize_t start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "all", _Dee_HashSelectC(0xb7e328b, 0xa812dcfcacc34ebc),  DEE_PCKdSIZ DEE_PCKdSIZ "o", start, end, key));
	}
	WUNUSED Ref<deemon::bool_> (all)(Dee_ssize_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "all", _Dee_HashSelectC(0xb7e328b, 0xa812dcfcacc34ebc),  DEE_PCKdSIZ DEE_PCKuSIZ, start, end));
	}
	WUNUSED NONNULL_CXX((3)) Ref<deemon::bool_> (all)(Dee_ssize_t start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "all", _Dee_HashSelectC(0xb7e328b, 0xa812dcfcacc34ebc),  DEE_PCKdSIZ DEE_PCKuSIZ "o", start, end, key));
	}
	WUNUSED Ref<deemon::bool_> (all)(size_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "all", _Dee_HashSelectC(0xb7e328b, 0xa812dcfcacc34ebc),  DEE_PCKuSIZ, start));
	}
	WUNUSED NONNULL_CXX((2)) Ref<deemon::bool_> (all)(size_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "all", _Dee_HashSelectC(0xb7e328b, 0xa812dcfcacc34ebc),  DEE_PCKuSIZ "o", start, end));
	}
	WUNUSED NONNULL_CXX((2, 3)) Ref<deemon::bool_> (all)(size_t start, DeeObject *end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "all", _Dee_HashSelectC(0xb7e328b, 0xa812dcfcacc34ebc),  DEE_PCKuSIZ "oo", start, end, key));
	}
	WUNUSED Ref<deemon::bool_> (all)(size_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "all", _Dee_HashSelectC(0xb7e328b, 0xa812dcfcacc34ebc),  DEE_PCKuSIZ DEE_PCKdSIZ, start, end));
	}
	WUNUSED NONNULL_CXX((3)) Ref<deemon::bool_> (all)(size_t start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "all", _Dee_HashSelectC(0xb7e328b, 0xa812dcfcacc34ebc),  DEE_PCKuSIZ DEE_PCKdSIZ "o", start, end, key));
	}
	WUNUSED Ref<deemon::bool_> (all)(size_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "all", _Dee_HashSelectC(0xb7e328b, 0xa812dcfcacc34ebc),  DEE_PCKuSIZ DEE_PCKuSIZ, start, end));
	}
	WUNUSED NONNULL_CXX((3)) Ref<deemon::bool_> (all)(size_t start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "all", _Dee_HashSelectC(0xb7e328b, 0xa812dcfcacc34ebc),  DEE_PCKuSIZ DEE_PCKuSIZ "o", start, end, key));
	}
	WUNUSED Ref<deemon::bool_> (parity)() {
		return inherit(DeeObject_CallAttrStringHash(this, "parity", _Dee_HashSelectC(0x92f3da0a, 0x254564db48674f28), 0, NULL));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (parity)(DeeObject *start) {
		DeeObject *args[1];
		args[0] = start;
		return inherit(DeeObject_CallAttrStringHash(this, "parity", _Dee_HashSelectC(0x92f3da0a, 0x254564db48674f28), 1, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::bool_> (parity)(DeeObject *start, DeeObject *end) {
		DeeObject *args[2];
		args[0] = start;
		args[1] = end;
		return inherit(DeeObject_CallAttrStringHash(this, "parity", _Dee_HashSelectC(0x92f3da0a, 0x254564db48674f28), 2, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3)) Ref<deemon::bool_> (parity)(DeeObject *start, DeeObject *end, DeeObject *key) {
		DeeObject *args[3];
		args[0] = start;
		args[1] = end;
		args[2] = key;
		return inherit(DeeObject_CallAttrStringHash(this, "parity", _Dee_HashSelectC(0x92f3da0a, 0x254564db48674f28), 3, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (parity)(DeeObject *start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "parity", _Dee_HashSelectC(0x92f3da0a, 0x254564db48674f28), "o" DEE_PCKdSIZ, start, end));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::bool_> (parity)(DeeObject *start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "parity", _Dee_HashSelectC(0x92f3da0a, 0x254564db48674f28), "o" DEE_PCKdSIZ "o", start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (parity)(DeeObject *start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "parity", _Dee_HashSelectC(0x92f3da0a, 0x254564db48674f28), "o" DEE_PCKuSIZ, start, end));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::bool_> (parity)(DeeObject *start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "parity", _Dee_HashSelectC(0x92f3da0a, 0x254564db48674f28), "o" DEE_PCKuSIZ "o", start, end, key));
	}
	WUNUSED Ref<deemon::bool_> (parity)(Dee_ssize_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "parity", _Dee_HashSelectC(0x92f3da0a, 0x254564db48674f28),  DEE_PCKdSIZ, start));
	}
	WUNUSED NONNULL_CXX((2)) Ref<deemon::bool_> (parity)(Dee_ssize_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "parity", _Dee_HashSelectC(0x92f3da0a, 0x254564db48674f28),  DEE_PCKdSIZ "o", start, end));
	}
	WUNUSED NONNULL_CXX((2, 3)) Ref<deemon::bool_> (parity)(Dee_ssize_t start, DeeObject *end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "parity", _Dee_HashSelectC(0x92f3da0a, 0x254564db48674f28),  DEE_PCKdSIZ "oo", start, end, key));
	}
	WUNUSED Ref<deemon::bool_> (parity)(Dee_ssize_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "parity", _Dee_HashSelectC(0x92f3da0a, 0x254564db48674f28),  DEE_PCKdSIZ DEE_PCKdSIZ, start, end));
	}
	WUNUSED NONNULL_CXX((3)) Ref<deemon::bool_> (parity)(Dee_ssize_t start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "parity", _Dee_HashSelectC(0x92f3da0a, 0x254564db48674f28),  DEE_PCKdSIZ DEE_PCKdSIZ "o", start, end, key));
	}
	WUNUSED Ref<deemon::bool_> (parity)(Dee_ssize_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "parity", _Dee_HashSelectC(0x92f3da0a, 0x254564db48674f28),  DEE_PCKdSIZ DEE_PCKuSIZ, start, end));
	}
	WUNUSED NONNULL_CXX((3)) Ref<deemon::bool_> (parity)(Dee_ssize_t start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "parity", _Dee_HashSelectC(0x92f3da0a, 0x254564db48674f28),  DEE_PCKdSIZ DEE_PCKuSIZ "o", start, end, key));
	}
	WUNUSED Ref<deemon::bool_> (parity)(size_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "parity", _Dee_HashSelectC(0x92f3da0a, 0x254564db48674f28),  DEE_PCKuSIZ, start));
	}
	WUNUSED NONNULL_CXX((2)) Ref<deemon::bool_> (parity)(size_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "parity", _Dee_HashSelectC(0x92f3da0a, 0x254564db48674f28),  DEE_PCKuSIZ "o", start, end));
	}
	WUNUSED NONNULL_CXX((2, 3)) Ref<deemon::bool_> (parity)(size_t start, DeeObject *end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "parity", _Dee_HashSelectC(0x92f3da0a, 0x254564db48674f28),  DEE_PCKuSIZ "oo", start, end, key));
	}
	WUNUSED Ref<deemon::bool_> (parity)(size_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "parity", _Dee_HashSelectC(0x92f3da0a, 0x254564db48674f28),  DEE_PCKuSIZ DEE_PCKdSIZ, start, end));
	}
	WUNUSED NONNULL_CXX((3)) Ref<deemon::bool_> (parity)(size_t start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "parity", _Dee_HashSelectC(0x92f3da0a, 0x254564db48674f28),  DEE_PCKuSIZ DEE_PCKdSIZ "o", start, end, key));
	}
	WUNUSED Ref<deemon::bool_> (parity)(size_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "parity", _Dee_HashSelectC(0x92f3da0a, 0x254564db48674f28),  DEE_PCKuSIZ DEE_PCKuSIZ, start, end));
	}
	WUNUSED NONNULL_CXX((3)) Ref<deemon::bool_> (parity)(size_t start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "parity", _Dee_HashSelectC(0x92f3da0a, 0x254564db48674f28),  DEE_PCKuSIZ DEE_PCKuSIZ "o", start, end, key));
	}
	WUNUSED Ref<T> (min)() {
		return inherit(DeeObject_CallAttrStringHash(this, "min", _Dee_HashSelectC(0xde957cfa, 0xe6ffc7365e2d00ca), 0, NULL));
	}
	WUNUSED NONNULL_CXX((1)) Ref<T> (min)(DeeObject *start) {
		DeeObject *args[1];
		args[0] = start;
		return inherit(DeeObject_CallAttrStringHash(this, "min", _Dee_HashSelectC(0xde957cfa, 0xe6ffc7365e2d00ca), 1, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<T> (min)(DeeObject *start, DeeObject *end) {
		DeeObject *args[2];
		args[0] = start;
		args[1] = end;
		return inherit(DeeObject_CallAttrStringHash(this, "min", _Dee_HashSelectC(0xde957cfa, 0xe6ffc7365e2d00ca), 2, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3)) Ref<T> (min)(DeeObject *start, DeeObject *end, DeeObject *key) {
		DeeObject *args[3];
		args[0] = start;
		args[1] = end;
		args[2] = key;
		return inherit(DeeObject_CallAttrStringHash(this, "min", _Dee_HashSelectC(0xde957cfa, 0xe6ffc7365e2d00ca), 3, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<T> (min)(DeeObject *start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "min", _Dee_HashSelectC(0xde957cfa, 0xe6ffc7365e2d00ca), "o" DEE_PCKdSIZ, start, end));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<T> (min)(DeeObject *start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "min", _Dee_HashSelectC(0xde957cfa, 0xe6ffc7365e2d00ca), "o" DEE_PCKdSIZ "o", start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<T> (min)(DeeObject *start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "min", _Dee_HashSelectC(0xde957cfa, 0xe6ffc7365e2d00ca), "o" DEE_PCKuSIZ, start, end));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<T> (min)(DeeObject *start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "min", _Dee_HashSelectC(0xde957cfa, 0xe6ffc7365e2d00ca), "o" DEE_PCKuSIZ "o", start, end, key));
	}
	WUNUSED Ref<T> (min)(Dee_ssize_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "min", _Dee_HashSelectC(0xde957cfa, 0xe6ffc7365e2d00ca),  DEE_PCKdSIZ, start));
	}
	WUNUSED NONNULL_CXX((2)) Ref<T> (min)(Dee_ssize_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "min", _Dee_HashSelectC(0xde957cfa, 0xe6ffc7365e2d00ca),  DEE_PCKdSIZ "o", start, end));
	}
	WUNUSED NONNULL_CXX((2, 3)) Ref<T> (min)(Dee_ssize_t start, DeeObject *end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "min", _Dee_HashSelectC(0xde957cfa, 0xe6ffc7365e2d00ca),  DEE_PCKdSIZ "oo", start, end, key));
	}
	WUNUSED Ref<T> (min)(Dee_ssize_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "min", _Dee_HashSelectC(0xde957cfa, 0xe6ffc7365e2d00ca),  DEE_PCKdSIZ DEE_PCKdSIZ, start, end));
	}
	WUNUSED NONNULL_CXX((3)) Ref<T> (min)(Dee_ssize_t start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "min", _Dee_HashSelectC(0xde957cfa, 0xe6ffc7365e2d00ca),  DEE_PCKdSIZ DEE_PCKdSIZ "o", start, end, key));
	}
	WUNUSED Ref<T> (min)(Dee_ssize_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "min", _Dee_HashSelectC(0xde957cfa, 0xe6ffc7365e2d00ca),  DEE_PCKdSIZ DEE_PCKuSIZ, start, end));
	}
	WUNUSED NONNULL_CXX((3)) Ref<T> (min)(Dee_ssize_t start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "min", _Dee_HashSelectC(0xde957cfa, 0xe6ffc7365e2d00ca),  DEE_PCKdSIZ DEE_PCKuSIZ "o", start, end, key));
	}
	WUNUSED Ref<T> (min)(size_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "min", _Dee_HashSelectC(0xde957cfa, 0xe6ffc7365e2d00ca),  DEE_PCKuSIZ, start));
	}
	WUNUSED NONNULL_CXX((2)) Ref<T> (min)(size_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "min", _Dee_HashSelectC(0xde957cfa, 0xe6ffc7365e2d00ca),  DEE_PCKuSIZ "o", start, end));
	}
	WUNUSED NONNULL_CXX((2, 3)) Ref<T> (min)(size_t start, DeeObject *end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "min", _Dee_HashSelectC(0xde957cfa, 0xe6ffc7365e2d00ca),  DEE_PCKuSIZ "oo", start, end, key));
	}
	WUNUSED Ref<T> (min)(size_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "min", _Dee_HashSelectC(0xde957cfa, 0xe6ffc7365e2d00ca),  DEE_PCKuSIZ DEE_PCKdSIZ, start, end));
	}
	WUNUSED NONNULL_CXX((3)) Ref<T> (min)(size_t start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "min", _Dee_HashSelectC(0xde957cfa, 0xe6ffc7365e2d00ca),  DEE_PCKuSIZ DEE_PCKdSIZ "o", start, end, key));
	}
	WUNUSED Ref<T> (min)(size_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "min", _Dee_HashSelectC(0xde957cfa, 0xe6ffc7365e2d00ca),  DEE_PCKuSIZ DEE_PCKuSIZ, start, end));
	}
	WUNUSED NONNULL_CXX((3)) Ref<T> (min)(size_t start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "min", _Dee_HashSelectC(0xde957cfa, 0xe6ffc7365e2d00ca),  DEE_PCKuSIZ DEE_PCKuSIZ "o", start, end, key));
	}
	WUNUSED Ref<T> (max)() {
		return inherit(DeeObject_CallAttrStringHash(this, "max", _Dee_HashSelectC(0xc293979b, 0x822bd5c706bd9850), 0, NULL));
	}
	WUNUSED NONNULL_CXX((1)) Ref<T> (max)(DeeObject *start) {
		DeeObject *args[1];
		args[0] = start;
		return inherit(DeeObject_CallAttrStringHash(this, "max", _Dee_HashSelectC(0xc293979b, 0x822bd5c706bd9850), 1, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<T> (max)(DeeObject *start, DeeObject *end) {
		DeeObject *args[2];
		args[0] = start;
		args[1] = end;
		return inherit(DeeObject_CallAttrStringHash(this, "max", _Dee_HashSelectC(0xc293979b, 0x822bd5c706bd9850), 2, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3)) Ref<T> (max)(DeeObject *start, DeeObject *end, DeeObject *key) {
		DeeObject *args[3];
		args[0] = start;
		args[1] = end;
		args[2] = key;
		return inherit(DeeObject_CallAttrStringHash(this, "max", _Dee_HashSelectC(0xc293979b, 0x822bd5c706bd9850), 3, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<T> (max)(DeeObject *start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "max", _Dee_HashSelectC(0xc293979b, 0x822bd5c706bd9850), "o" DEE_PCKdSIZ, start, end));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<T> (max)(DeeObject *start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "max", _Dee_HashSelectC(0xc293979b, 0x822bd5c706bd9850), "o" DEE_PCKdSIZ "o", start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<T> (max)(DeeObject *start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "max", _Dee_HashSelectC(0xc293979b, 0x822bd5c706bd9850), "o" DEE_PCKuSIZ, start, end));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<T> (max)(DeeObject *start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "max", _Dee_HashSelectC(0xc293979b, 0x822bd5c706bd9850), "o" DEE_PCKuSIZ "o", start, end, key));
	}
	WUNUSED Ref<T> (max)(Dee_ssize_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "max", _Dee_HashSelectC(0xc293979b, 0x822bd5c706bd9850),  DEE_PCKdSIZ, start));
	}
	WUNUSED NONNULL_CXX((2)) Ref<T> (max)(Dee_ssize_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "max", _Dee_HashSelectC(0xc293979b, 0x822bd5c706bd9850),  DEE_PCKdSIZ "o", start, end));
	}
	WUNUSED NONNULL_CXX((2, 3)) Ref<T> (max)(Dee_ssize_t start, DeeObject *end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "max", _Dee_HashSelectC(0xc293979b, 0x822bd5c706bd9850),  DEE_PCKdSIZ "oo", start, end, key));
	}
	WUNUSED Ref<T> (max)(Dee_ssize_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "max", _Dee_HashSelectC(0xc293979b, 0x822bd5c706bd9850),  DEE_PCKdSIZ DEE_PCKdSIZ, start, end));
	}
	WUNUSED NONNULL_CXX((3)) Ref<T> (max)(Dee_ssize_t start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "max", _Dee_HashSelectC(0xc293979b, 0x822bd5c706bd9850),  DEE_PCKdSIZ DEE_PCKdSIZ "o", start, end, key));
	}
	WUNUSED Ref<T> (max)(Dee_ssize_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "max", _Dee_HashSelectC(0xc293979b, 0x822bd5c706bd9850),  DEE_PCKdSIZ DEE_PCKuSIZ, start, end));
	}
	WUNUSED NONNULL_CXX((3)) Ref<T> (max)(Dee_ssize_t start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "max", _Dee_HashSelectC(0xc293979b, 0x822bd5c706bd9850),  DEE_PCKdSIZ DEE_PCKuSIZ "o", start, end, key));
	}
	WUNUSED Ref<T> (max)(size_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "max", _Dee_HashSelectC(0xc293979b, 0x822bd5c706bd9850),  DEE_PCKuSIZ, start));
	}
	WUNUSED NONNULL_CXX((2)) Ref<T> (max)(size_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "max", _Dee_HashSelectC(0xc293979b, 0x822bd5c706bd9850),  DEE_PCKuSIZ "o", start, end));
	}
	WUNUSED NONNULL_CXX((2, 3)) Ref<T> (max)(size_t start, DeeObject *end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "max", _Dee_HashSelectC(0xc293979b, 0x822bd5c706bd9850),  DEE_PCKuSIZ "oo", start, end, key));
	}
	WUNUSED Ref<T> (max)(size_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "max", _Dee_HashSelectC(0xc293979b, 0x822bd5c706bd9850),  DEE_PCKuSIZ DEE_PCKdSIZ, start, end));
	}
	WUNUSED NONNULL_CXX((3)) Ref<T> (max)(size_t start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "max", _Dee_HashSelectC(0xc293979b, 0x822bd5c706bd9850),  DEE_PCKuSIZ DEE_PCKdSIZ "o", start, end, key));
	}
	WUNUSED Ref<T> (max)(size_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "max", _Dee_HashSelectC(0xc293979b, 0x822bd5c706bd9850),  DEE_PCKuSIZ DEE_PCKuSIZ, start, end));
	}
	WUNUSED NONNULL_CXX((3)) Ref<T> (max)(size_t start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "max", _Dee_HashSelectC(0xc293979b, 0x822bd5c706bd9850),  DEE_PCKuSIZ DEE_PCKuSIZ "o", start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (count)(DeeObject *item) {
		DeeObject *args[1];
		args[0] = item;
		return inherit(DeeObject_CallAttrStringHash(this, "count", _Dee_HashSelectC(0x54eac164, 0xbd66b5980d54babb), 1, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::int_> (count)(DeeObject *item, DeeObject *start) {
		DeeObject *args[2];
		args[0] = item;
		args[1] = start;
		return inherit(DeeObject_CallAttrStringHash(this, "count", _Dee_HashSelectC(0x54eac164, 0xbd66b5980d54babb), 2, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3)) Ref<deemon::int_> (count)(DeeObject *item, DeeObject *start, DeeObject *end) {
		DeeObject *args[3];
		args[0] = item;
		args[1] = start;
		args[2] = end;
		return inherit(DeeObject_CallAttrStringHash(this, "count", _Dee_HashSelectC(0x54eac164, 0xbd66b5980d54babb), 3, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3, 4)) Ref<deemon::int_> (count)(DeeObject *item, DeeObject *start, DeeObject *end, DeeObject *key) {
		DeeObject *args[4];
		args[0] = item;
		args[1] = start;
		args[2] = end;
		args[3] = key;
		return inherit(DeeObject_CallAttrStringHash(this, "count", _Dee_HashSelectC(0x54eac164, 0xbd66b5980d54babb), 4, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::int_> (count)(DeeObject *item, DeeObject *start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "count", _Dee_HashSelectC(0x54eac164, 0xbd66b5980d54babb), "oo" DEE_PCKdSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 2, 4)) Ref<deemon::int_> (count)(DeeObject *item, DeeObject *start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "count", _Dee_HashSelectC(0x54eac164, 0xbd66b5980d54babb), "oo" DEE_PCKdSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::int_> (count)(DeeObject *item, DeeObject *start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "count", _Dee_HashSelectC(0x54eac164, 0xbd66b5980d54babb), "oo" DEE_PCKuSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 2, 4)) Ref<deemon::int_> (count)(DeeObject *item, DeeObject *start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "count", _Dee_HashSelectC(0x54eac164, 0xbd66b5980d54babb), "oo" DEE_PCKuSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (count)(DeeObject *item, Dee_ssize_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "count", _Dee_HashSelectC(0x54eac164, 0xbd66b5980d54babb), "o" DEE_PCKdSIZ, item, start));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::int_> (count)(DeeObject *item, Dee_ssize_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "count", _Dee_HashSelectC(0x54eac164, 0xbd66b5980d54babb), "o" DEE_PCKdSIZ "o", item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 3, 4)) Ref<deemon::int_> (count)(DeeObject *item, Dee_ssize_t start, DeeObject *end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "count", _Dee_HashSelectC(0x54eac164, 0xbd66b5980d54babb), "o" DEE_PCKdSIZ "oo", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (count)(DeeObject *item, Dee_ssize_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "count", _Dee_HashSelectC(0x54eac164, 0xbd66b5980d54babb), "o" DEE_PCKdSIZ DEE_PCKdSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::int_> (count)(DeeObject *item, Dee_ssize_t start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "count", _Dee_HashSelectC(0x54eac164, 0xbd66b5980d54babb), "o" DEE_PCKdSIZ DEE_PCKdSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (count)(DeeObject *item, Dee_ssize_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "count", _Dee_HashSelectC(0x54eac164, 0xbd66b5980d54babb), "o" DEE_PCKdSIZ DEE_PCKuSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::int_> (count)(DeeObject *item, Dee_ssize_t start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "count", _Dee_HashSelectC(0x54eac164, 0xbd66b5980d54babb), "o" DEE_PCKdSIZ DEE_PCKuSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (count)(DeeObject *item, size_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "count", _Dee_HashSelectC(0x54eac164, 0xbd66b5980d54babb), "o" DEE_PCKuSIZ, item, start));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::int_> (count)(DeeObject *item, size_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "count", _Dee_HashSelectC(0x54eac164, 0xbd66b5980d54babb), "o" DEE_PCKuSIZ "o", item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 3, 4)) Ref<deemon::int_> (count)(DeeObject *item, size_t start, DeeObject *end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "count", _Dee_HashSelectC(0x54eac164, 0xbd66b5980d54babb), "o" DEE_PCKuSIZ "oo", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (count)(DeeObject *item, size_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "count", _Dee_HashSelectC(0x54eac164, 0xbd66b5980d54babb), "o" DEE_PCKuSIZ DEE_PCKdSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::int_> (count)(DeeObject *item, size_t start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "count", _Dee_HashSelectC(0x54eac164, 0xbd66b5980d54babb), "o" DEE_PCKuSIZ DEE_PCKdSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (count)(DeeObject *item, size_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "count", _Dee_HashSelectC(0x54eac164, 0xbd66b5980d54babb), "o" DEE_PCKuSIZ DEE_PCKuSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::int_> (count)(DeeObject *item, size_t start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "count", _Dee_HashSelectC(0x54eac164, 0xbd66b5980d54babb), "o" DEE_PCKuSIZ DEE_PCKuSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<T> (locate)(DeeObject *item) {
		DeeObject *args[1];
		args[0] = item;
		return inherit(DeeObject_CallAttrStringHash(this, "locate", _Dee_HashSelectC(0x72fc7691, 0xa3c641c56f3d6258), 1, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<T> (locate)(DeeObject *item, DeeObject *start) {
		DeeObject *args[2];
		args[0] = item;
		args[1] = start;
		return inherit(DeeObject_CallAttrStringHash(this, "locate", _Dee_HashSelectC(0x72fc7691, 0xa3c641c56f3d6258), 2, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3)) Ref<T> (locate)(DeeObject *item, DeeObject *start, DeeObject *end) {
		DeeObject *args[3];
		args[0] = item;
		args[1] = start;
		args[2] = end;
		return inherit(DeeObject_CallAttrStringHash(this, "locate", _Dee_HashSelectC(0x72fc7691, 0xa3c641c56f3d6258), 3, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3, 4)) Ref<T> (locate)(DeeObject *item, DeeObject *start, DeeObject *end, DeeObject *key) {
		DeeObject *args[4];
		args[0] = item;
		args[1] = start;
		args[2] = end;
		args[3] = key;
		return inherit(DeeObject_CallAttrStringHash(this, "locate", _Dee_HashSelectC(0x72fc7691, 0xa3c641c56f3d6258), 4, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<T> (locate)(DeeObject *item, DeeObject *start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "locate", _Dee_HashSelectC(0x72fc7691, 0xa3c641c56f3d6258), "oo" DEE_PCKdSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 2, 4)) Ref<T> (locate)(DeeObject *item, DeeObject *start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "locate", _Dee_HashSelectC(0x72fc7691, 0xa3c641c56f3d6258), "oo" DEE_PCKdSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<T> (locate)(DeeObject *item, DeeObject *start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "locate", _Dee_HashSelectC(0x72fc7691, 0xa3c641c56f3d6258), "oo" DEE_PCKuSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 2, 4)) Ref<T> (locate)(DeeObject *item, DeeObject *start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "locate", _Dee_HashSelectC(0x72fc7691, 0xa3c641c56f3d6258), "oo" DEE_PCKuSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<T> (locate)(DeeObject *item, Dee_ssize_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "locate", _Dee_HashSelectC(0x72fc7691, 0xa3c641c56f3d6258), "o" DEE_PCKdSIZ, item, start));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<T> (locate)(DeeObject *item, Dee_ssize_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "locate", _Dee_HashSelectC(0x72fc7691, 0xa3c641c56f3d6258), "o" DEE_PCKdSIZ "o", item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 3, 4)) Ref<T> (locate)(DeeObject *item, Dee_ssize_t start, DeeObject *end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "locate", _Dee_HashSelectC(0x72fc7691, 0xa3c641c56f3d6258), "o" DEE_PCKdSIZ "oo", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<T> (locate)(DeeObject *item, Dee_ssize_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "locate", _Dee_HashSelectC(0x72fc7691, 0xa3c641c56f3d6258), "o" DEE_PCKdSIZ DEE_PCKdSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<T> (locate)(DeeObject *item, Dee_ssize_t start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "locate", _Dee_HashSelectC(0x72fc7691, 0xa3c641c56f3d6258), "o" DEE_PCKdSIZ DEE_PCKdSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<T> (locate)(DeeObject *item, Dee_ssize_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "locate", _Dee_HashSelectC(0x72fc7691, 0xa3c641c56f3d6258), "o" DEE_PCKdSIZ DEE_PCKuSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<T> (locate)(DeeObject *item, Dee_ssize_t start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "locate", _Dee_HashSelectC(0x72fc7691, 0xa3c641c56f3d6258), "o" DEE_PCKdSIZ DEE_PCKuSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<T> (locate)(DeeObject *item, size_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "locate", _Dee_HashSelectC(0x72fc7691, 0xa3c641c56f3d6258), "o" DEE_PCKuSIZ, item, start));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<T> (locate)(DeeObject *item, size_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "locate", _Dee_HashSelectC(0x72fc7691, 0xa3c641c56f3d6258), "o" DEE_PCKuSIZ "o", item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 3, 4)) Ref<T> (locate)(DeeObject *item, size_t start, DeeObject *end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "locate", _Dee_HashSelectC(0x72fc7691, 0xa3c641c56f3d6258), "o" DEE_PCKuSIZ "oo", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<T> (locate)(DeeObject *item, size_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "locate", _Dee_HashSelectC(0x72fc7691, 0xa3c641c56f3d6258), "o" DEE_PCKuSIZ DEE_PCKdSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<T> (locate)(DeeObject *item, size_t start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "locate", _Dee_HashSelectC(0x72fc7691, 0xa3c641c56f3d6258), "o" DEE_PCKuSIZ DEE_PCKdSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<T> (locate)(DeeObject *item, size_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "locate", _Dee_HashSelectC(0x72fc7691, 0xa3c641c56f3d6258), "o" DEE_PCKuSIZ DEE_PCKuSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<T> (locate)(DeeObject *item, size_t start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "locate", _Dee_HashSelectC(0x72fc7691, 0xa3c641c56f3d6258), "o" DEE_PCKuSIZ DEE_PCKuSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<T> (rlocate)(DeeObject *item) {
		DeeObject *args[1];
		args[0] = item;
		return inherit(DeeObject_CallAttrStringHash(this, "rlocate", _Dee_HashSelectC(0xe233056a, 0xf4759389157e74b), 1, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<T> (rlocate)(DeeObject *item, DeeObject *start) {
		DeeObject *args[2];
		args[0] = item;
		args[1] = start;
		return inherit(DeeObject_CallAttrStringHash(this, "rlocate", _Dee_HashSelectC(0xe233056a, 0xf4759389157e74b), 2, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3)) Ref<T> (rlocate)(DeeObject *item, DeeObject *start, DeeObject *end) {
		DeeObject *args[3];
		args[0] = item;
		args[1] = start;
		args[2] = end;
		return inherit(DeeObject_CallAttrStringHash(this, "rlocate", _Dee_HashSelectC(0xe233056a, 0xf4759389157e74b), 3, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3, 4)) Ref<T> (rlocate)(DeeObject *item, DeeObject *start, DeeObject *end, DeeObject *key) {
		DeeObject *args[4];
		args[0] = item;
		args[1] = start;
		args[2] = end;
		args[3] = key;
		return inherit(DeeObject_CallAttrStringHash(this, "rlocate", _Dee_HashSelectC(0xe233056a, 0xf4759389157e74b), 4, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<T> (rlocate)(DeeObject *item, DeeObject *start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rlocate", _Dee_HashSelectC(0xe233056a, 0xf4759389157e74b), "oo" DEE_PCKdSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 2, 4)) Ref<T> (rlocate)(DeeObject *item, DeeObject *start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rlocate", _Dee_HashSelectC(0xe233056a, 0xf4759389157e74b), "oo" DEE_PCKdSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<T> (rlocate)(DeeObject *item, DeeObject *start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rlocate", _Dee_HashSelectC(0xe233056a, 0xf4759389157e74b), "oo" DEE_PCKuSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 2, 4)) Ref<T> (rlocate)(DeeObject *item, DeeObject *start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rlocate", _Dee_HashSelectC(0xe233056a, 0xf4759389157e74b), "oo" DEE_PCKuSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<T> (rlocate)(DeeObject *item, Dee_ssize_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rlocate", _Dee_HashSelectC(0xe233056a, 0xf4759389157e74b), "o" DEE_PCKdSIZ, item, start));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<T> (rlocate)(DeeObject *item, Dee_ssize_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rlocate", _Dee_HashSelectC(0xe233056a, 0xf4759389157e74b), "o" DEE_PCKdSIZ "o", item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 3, 4)) Ref<T> (rlocate)(DeeObject *item, Dee_ssize_t start, DeeObject *end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rlocate", _Dee_HashSelectC(0xe233056a, 0xf4759389157e74b), "o" DEE_PCKdSIZ "oo", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<T> (rlocate)(DeeObject *item, Dee_ssize_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rlocate", _Dee_HashSelectC(0xe233056a, 0xf4759389157e74b), "o" DEE_PCKdSIZ DEE_PCKdSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<T> (rlocate)(DeeObject *item, Dee_ssize_t start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rlocate", _Dee_HashSelectC(0xe233056a, 0xf4759389157e74b), "o" DEE_PCKdSIZ DEE_PCKdSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<T> (rlocate)(DeeObject *item, Dee_ssize_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rlocate", _Dee_HashSelectC(0xe233056a, 0xf4759389157e74b), "o" DEE_PCKdSIZ DEE_PCKuSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<T> (rlocate)(DeeObject *item, Dee_ssize_t start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rlocate", _Dee_HashSelectC(0xe233056a, 0xf4759389157e74b), "o" DEE_PCKdSIZ DEE_PCKuSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<T> (rlocate)(DeeObject *item, size_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rlocate", _Dee_HashSelectC(0xe233056a, 0xf4759389157e74b), "o" DEE_PCKuSIZ, item, start));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<T> (rlocate)(DeeObject *item, size_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rlocate", _Dee_HashSelectC(0xe233056a, 0xf4759389157e74b), "o" DEE_PCKuSIZ "o", item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 3, 4)) Ref<T> (rlocate)(DeeObject *item, size_t start, DeeObject *end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rlocate", _Dee_HashSelectC(0xe233056a, 0xf4759389157e74b), "o" DEE_PCKuSIZ "oo", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<T> (rlocate)(DeeObject *item, size_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rlocate", _Dee_HashSelectC(0xe233056a, 0xf4759389157e74b), "o" DEE_PCKuSIZ DEE_PCKdSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<T> (rlocate)(DeeObject *item, size_t start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rlocate", _Dee_HashSelectC(0xe233056a, 0xf4759389157e74b), "o" DEE_PCKuSIZ DEE_PCKdSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<T> (rlocate)(DeeObject *item, size_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rlocate", _Dee_HashSelectC(0xe233056a, 0xf4759389157e74b), "o" DEE_PCKuSIZ DEE_PCKuSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<T> (rlocate)(DeeObject *item, size_t start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rlocate", _Dee_HashSelectC(0xe233056a, 0xf4759389157e74b), "o" DEE_PCKuSIZ DEE_PCKuSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Sequence<T> > (filter)(DeeObject *keep) {
		DeeObject *args[1];
		args[0] = keep;
		return inherit(DeeObject_CallAttrStringHash(this, "filter", _Dee_HashSelectC(0x3110088a, 0x32e04884df75b1c1), 1, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Sequence<T> > (ubfilter)(DeeObject *keep) {
		DeeObject *args[1];
		args[0] = keep;
		return inherit(DeeObject_CallAttrStringHash(this, "ubfilter", _Dee_HashSelectC(0x9f55cd0c, 0xa457507f0faa4d80), 1, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Sequence<T> > (map)(DeeObject *mapper) {
		DeeObject *args[1];
		args[0] = mapper;
		return inherit(DeeObject_CallAttrStringHash(this, "map", _Dee_HashSelectC(0xeb1d32c8, 0x6ed228005fef6a3), 1, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Sequence<T> > (locateall)(DeeObject *item) {
		DeeObject *args[1];
		args[0] = item;
		return inherit(DeeObject_CallAttrStringHash(this, "locateall", _Dee_HashSelectC(0xd447ec, 0xc6a682da9d9f8345), 1, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<Sequence<T> > (locateall)(DeeObject *item, DeeObject *key) {
		DeeObject *args[2];
		args[0] = item;
		args[1] = key;
		return inherit(DeeObject_CallAttrStringHash(this, "locateall", _Dee_HashSelectC(0xd447ec, 0xc6a682da9d9f8345), 2, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (contains)(DeeObject *item) {
		DeeObject *args[1];
		args[0] = item;
		return inherit(DeeObject_CallAttrStringHash(this, "contains", _Dee_HashSelectC(0x9338eec0, 0x1e0128373382a209), 1, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::bool_> (contains)(DeeObject *item, DeeObject *start) {
		DeeObject *args[2];
		args[0] = item;
		args[1] = start;
		return inherit(DeeObject_CallAttrStringHash(this, "contains", _Dee_HashSelectC(0x9338eec0, 0x1e0128373382a209), 2, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3)) Ref<deemon::bool_> (contains)(DeeObject *item, DeeObject *start, DeeObject *end) {
		DeeObject *args[3];
		args[0] = item;
		args[1] = start;
		args[2] = end;
		return inherit(DeeObject_CallAttrStringHash(this, "contains", _Dee_HashSelectC(0x9338eec0, 0x1e0128373382a209), 3, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3, 4)) Ref<deemon::bool_> (contains)(DeeObject *item, DeeObject *start, DeeObject *end, DeeObject *key) {
		DeeObject *args[4];
		args[0] = item;
		args[1] = start;
		args[2] = end;
		args[3] = key;
		return inherit(DeeObject_CallAttrStringHash(this, "contains", _Dee_HashSelectC(0x9338eec0, 0x1e0128373382a209), 4, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::bool_> (contains)(DeeObject *item, DeeObject *start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "contains", _Dee_HashSelectC(0x9338eec0, 0x1e0128373382a209), "oo" DEE_PCKdSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 2, 4)) Ref<deemon::bool_> (contains)(DeeObject *item, DeeObject *start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "contains", _Dee_HashSelectC(0x9338eec0, 0x1e0128373382a209), "oo" DEE_PCKdSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::bool_> (contains)(DeeObject *item, DeeObject *start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "contains", _Dee_HashSelectC(0x9338eec0, 0x1e0128373382a209), "oo" DEE_PCKuSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 2, 4)) Ref<deemon::bool_> (contains)(DeeObject *item, DeeObject *start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "contains", _Dee_HashSelectC(0x9338eec0, 0x1e0128373382a209), "oo" DEE_PCKuSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (contains)(DeeObject *item, Dee_ssize_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "contains", _Dee_HashSelectC(0x9338eec0, 0x1e0128373382a209), "o" DEE_PCKdSIZ, item, start));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::bool_> (contains)(DeeObject *item, Dee_ssize_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "contains", _Dee_HashSelectC(0x9338eec0, 0x1e0128373382a209), "o" DEE_PCKdSIZ "o", item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 3, 4)) Ref<deemon::bool_> (contains)(DeeObject *item, Dee_ssize_t start, DeeObject *end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "contains", _Dee_HashSelectC(0x9338eec0, 0x1e0128373382a209), "o" DEE_PCKdSIZ "oo", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (contains)(DeeObject *item, Dee_ssize_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "contains", _Dee_HashSelectC(0x9338eec0, 0x1e0128373382a209), "o" DEE_PCKdSIZ DEE_PCKdSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::bool_> (contains)(DeeObject *item, Dee_ssize_t start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "contains", _Dee_HashSelectC(0x9338eec0, 0x1e0128373382a209), "o" DEE_PCKdSIZ DEE_PCKdSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (contains)(DeeObject *item, Dee_ssize_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "contains", _Dee_HashSelectC(0x9338eec0, 0x1e0128373382a209), "o" DEE_PCKdSIZ DEE_PCKuSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::bool_> (contains)(DeeObject *item, Dee_ssize_t start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "contains", _Dee_HashSelectC(0x9338eec0, 0x1e0128373382a209), "o" DEE_PCKdSIZ DEE_PCKuSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (contains)(DeeObject *item, size_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "contains", _Dee_HashSelectC(0x9338eec0, 0x1e0128373382a209), "o" DEE_PCKuSIZ, item, start));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::bool_> (contains)(DeeObject *item, size_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "contains", _Dee_HashSelectC(0x9338eec0, 0x1e0128373382a209), "o" DEE_PCKuSIZ "o", item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 3, 4)) Ref<deemon::bool_> (contains)(DeeObject *item, size_t start, DeeObject *end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "contains", _Dee_HashSelectC(0x9338eec0, 0x1e0128373382a209), "o" DEE_PCKuSIZ "oo", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (contains)(DeeObject *item, size_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "contains", _Dee_HashSelectC(0x9338eec0, 0x1e0128373382a209), "o" DEE_PCKuSIZ DEE_PCKdSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::bool_> (contains)(DeeObject *item, size_t start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "contains", _Dee_HashSelectC(0x9338eec0, 0x1e0128373382a209), "o" DEE_PCKuSIZ DEE_PCKdSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (contains)(DeeObject *item, size_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "contains", _Dee_HashSelectC(0x9338eec0, 0x1e0128373382a209), "o" DEE_PCKuSIZ DEE_PCKuSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::bool_> (contains)(DeeObject *item, size_t start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "contains", _Dee_HashSelectC(0x9338eec0, 0x1e0128373382a209), "o" DEE_PCKuSIZ DEE_PCKuSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (startswith)(DeeObject *item) {
		DeeObject *args[1];
		args[0] = item;
		return inherit(DeeObject_CallAttrStringHash(this, "startswith", _Dee_HashSelectC(0x58e22b, 0x6251da2c5cdb654d), 1, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::bool_> (startswith)(DeeObject *item, DeeObject *start) {
		DeeObject *args[2];
		args[0] = item;
		args[1] = start;
		return inherit(DeeObject_CallAttrStringHash(this, "startswith", _Dee_HashSelectC(0x58e22b, 0x6251da2c5cdb654d), 2, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3)) Ref<deemon::bool_> (startswith)(DeeObject *item, DeeObject *start, DeeObject *end) {
		DeeObject *args[3];
		args[0] = item;
		args[1] = start;
		args[2] = end;
		return inherit(DeeObject_CallAttrStringHash(this, "startswith", _Dee_HashSelectC(0x58e22b, 0x6251da2c5cdb654d), 3, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3, 4)) Ref<deemon::bool_> (startswith)(DeeObject *item, DeeObject *start, DeeObject *end, DeeObject *key) {
		DeeObject *args[4];
		args[0] = item;
		args[1] = start;
		args[2] = end;
		args[3] = key;
		return inherit(DeeObject_CallAttrStringHash(this, "startswith", _Dee_HashSelectC(0x58e22b, 0x6251da2c5cdb654d), 4, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::bool_> (startswith)(DeeObject *item, DeeObject *start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "startswith", _Dee_HashSelectC(0x58e22b, 0x6251da2c5cdb654d), "oo" DEE_PCKdSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 2, 4)) Ref<deemon::bool_> (startswith)(DeeObject *item, DeeObject *start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "startswith", _Dee_HashSelectC(0x58e22b, 0x6251da2c5cdb654d), "oo" DEE_PCKdSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::bool_> (startswith)(DeeObject *item, DeeObject *start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "startswith", _Dee_HashSelectC(0x58e22b, 0x6251da2c5cdb654d), "oo" DEE_PCKuSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 2, 4)) Ref<deemon::bool_> (startswith)(DeeObject *item, DeeObject *start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "startswith", _Dee_HashSelectC(0x58e22b, 0x6251da2c5cdb654d), "oo" DEE_PCKuSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (startswith)(DeeObject *item, Dee_ssize_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "startswith", _Dee_HashSelectC(0x58e22b, 0x6251da2c5cdb654d), "o" DEE_PCKdSIZ, item, start));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::bool_> (startswith)(DeeObject *item, Dee_ssize_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "startswith", _Dee_HashSelectC(0x58e22b, 0x6251da2c5cdb654d), "o" DEE_PCKdSIZ "o", item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 3, 4)) Ref<deemon::bool_> (startswith)(DeeObject *item, Dee_ssize_t start, DeeObject *end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "startswith", _Dee_HashSelectC(0x58e22b, 0x6251da2c5cdb654d), "o" DEE_PCKdSIZ "oo", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (startswith)(DeeObject *item, Dee_ssize_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "startswith", _Dee_HashSelectC(0x58e22b, 0x6251da2c5cdb654d), "o" DEE_PCKdSIZ DEE_PCKdSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::bool_> (startswith)(DeeObject *item, Dee_ssize_t start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "startswith", _Dee_HashSelectC(0x58e22b, 0x6251da2c5cdb654d), "o" DEE_PCKdSIZ DEE_PCKdSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (startswith)(DeeObject *item, Dee_ssize_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "startswith", _Dee_HashSelectC(0x58e22b, 0x6251da2c5cdb654d), "o" DEE_PCKdSIZ DEE_PCKuSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::bool_> (startswith)(DeeObject *item, Dee_ssize_t start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "startswith", _Dee_HashSelectC(0x58e22b, 0x6251da2c5cdb654d), "o" DEE_PCKdSIZ DEE_PCKuSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (startswith)(DeeObject *item, size_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "startswith", _Dee_HashSelectC(0x58e22b, 0x6251da2c5cdb654d), "o" DEE_PCKuSIZ, item, start));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::bool_> (startswith)(DeeObject *item, size_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "startswith", _Dee_HashSelectC(0x58e22b, 0x6251da2c5cdb654d), "o" DEE_PCKuSIZ "o", item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 3, 4)) Ref<deemon::bool_> (startswith)(DeeObject *item, size_t start, DeeObject *end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "startswith", _Dee_HashSelectC(0x58e22b, 0x6251da2c5cdb654d), "o" DEE_PCKuSIZ "oo", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (startswith)(DeeObject *item, size_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "startswith", _Dee_HashSelectC(0x58e22b, 0x6251da2c5cdb654d), "o" DEE_PCKuSIZ DEE_PCKdSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::bool_> (startswith)(DeeObject *item, size_t start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "startswith", _Dee_HashSelectC(0x58e22b, 0x6251da2c5cdb654d), "o" DEE_PCKuSIZ DEE_PCKdSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (startswith)(DeeObject *item, size_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "startswith", _Dee_HashSelectC(0x58e22b, 0x6251da2c5cdb654d), "o" DEE_PCKuSIZ DEE_PCKuSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::bool_> (startswith)(DeeObject *item, size_t start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "startswith", _Dee_HashSelectC(0x58e22b, 0x6251da2c5cdb654d), "o" DEE_PCKuSIZ DEE_PCKuSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (endswith)(DeeObject *item) {
		DeeObject *args[1];
		args[0] = item;
		return inherit(DeeObject_CallAttrStringHash(this, "endswith", _Dee_HashSelectC(0x8bdeff05, 0xca6abed3214345e1), 1, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::bool_> (endswith)(DeeObject *item, DeeObject *start) {
		DeeObject *args[2];
		args[0] = item;
		args[1] = start;
		return inherit(DeeObject_CallAttrStringHash(this, "endswith", _Dee_HashSelectC(0x8bdeff05, 0xca6abed3214345e1), 2, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3)) Ref<deemon::bool_> (endswith)(DeeObject *item, DeeObject *start, DeeObject *end) {
		DeeObject *args[3];
		args[0] = item;
		args[1] = start;
		args[2] = end;
		return inherit(DeeObject_CallAttrStringHash(this, "endswith", _Dee_HashSelectC(0x8bdeff05, 0xca6abed3214345e1), 3, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3, 4)) Ref<deemon::bool_> (endswith)(DeeObject *item, DeeObject *start, DeeObject *end, DeeObject *key) {
		DeeObject *args[4];
		args[0] = item;
		args[1] = start;
		args[2] = end;
		args[3] = key;
		return inherit(DeeObject_CallAttrStringHash(this, "endswith", _Dee_HashSelectC(0x8bdeff05, 0xca6abed3214345e1), 4, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::bool_> (endswith)(DeeObject *item, DeeObject *start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "endswith", _Dee_HashSelectC(0x8bdeff05, 0xca6abed3214345e1), "oo" DEE_PCKdSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 2, 4)) Ref<deemon::bool_> (endswith)(DeeObject *item, DeeObject *start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "endswith", _Dee_HashSelectC(0x8bdeff05, 0xca6abed3214345e1), "oo" DEE_PCKdSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::bool_> (endswith)(DeeObject *item, DeeObject *start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "endswith", _Dee_HashSelectC(0x8bdeff05, 0xca6abed3214345e1), "oo" DEE_PCKuSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 2, 4)) Ref<deemon::bool_> (endswith)(DeeObject *item, DeeObject *start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "endswith", _Dee_HashSelectC(0x8bdeff05, 0xca6abed3214345e1), "oo" DEE_PCKuSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (endswith)(DeeObject *item, Dee_ssize_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "endswith", _Dee_HashSelectC(0x8bdeff05, 0xca6abed3214345e1), "o" DEE_PCKdSIZ, item, start));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::bool_> (endswith)(DeeObject *item, Dee_ssize_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "endswith", _Dee_HashSelectC(0x8bdeff05, 0xca6abed3214345e1), "o" DEE_PCKdSIZ "o", item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 3, 4)) Ref<deemon::bool_> (endswith)(DeeObject *item, Dee_ssize_t start, DeeObject *end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "endswith", _Dee_HashSelectC(0x8bdeff05, 0xca6abed3214345e1), "o" DEE_PCKdSIZ "oo", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (endswith)(DeeObject *item, Dee_ssize_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "endswith", _Dee_HashSelectC(0x8bdeff05, 0xca6abed3214345e1), "o" DEE_PCKdSIZ DEE_PCKdSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::bool_> (endswith)(DeeObject *item, Dee_ssize_t start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "endswith", _Dee_HashSelectC(0x8bdeff05, 0xca6abed3214345e1), "o" DEE_PCKdSIZ DEE_PCKdSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (endswith)(DeeObject *item, Dee_ssize_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "endswith", _Dee_HashSelectC(0x8bdeff05, 0xca6abed3214345e1), "o" DEE_PCKdSIZ DEE_PCKuSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::bool_> (endswith)(DeeObject *item, Dee_ssize_t start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "endswith", _Dee_HashSelectC(0x8bdeff05, 0xca6abed3214345e1), "o" DEE_PCKdSIZ DEE_PCKuSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (endswith)(DeeObject *item, size_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "endswith", _Dee_HashSelectC(0x8bdeff05, 0xca6abed3214345e1), "o" DEE_PCKuSIZ, item, start));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::bool_> (endswith)(DeeObject *item, size_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "endswith", _Dee_HashSelectC(0x8bdeff05, 0xca6abed3214345e1), "o" DEE_PCKuSIZ "o", item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 3, 4)) Ref<deemon::bool_> (endswith)(DeeObject *item, size_t start, DeeObject *end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "endswith", _Dee_HashSelectC(0x8bdeff05, 0xca6abed3214345e1), "o" DEE_PCKuSIZ "oo", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (endswith)(DeeObject *item, size_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "endswith", _Dee_HashSelectC(0x8bdeff05, 0xca6abed3214345e1), "o" DEE_PCKuSIZ DEE_PCKdSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::bool_> (endswith)(DeeObject *item, size_t start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "endswith", _Dee_HashSelectC(0x8bdeff05, 0xca6abed3214345e1), "o" DEE_PCKuSIZ DEE_PCKdSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (endswith)(DeeObject *item, size_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "endswith", _Dee_HashSelectC(0x8bdeff05, 0xca6abed3214345e1), "o" DEE_PCKuSIZ DEE_PCKuSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::bool_> (endswith)(DeeObject *item, size_t start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "endswith", _Dee_HashSelectC(0x8bdeff05, 0xca6abed3214345e1), "o" DEE_PCKuSIZ DEE_PCKuSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (find)(DeeObject *item) {
		DeeObject *args[1];
		args[0] = item;
		return inherit(DeeObject_CallAttrStringHash(this, "find", _Dee_HashSelectC(0x9e66372, 0x2b65fe03bbdde5b2), 1, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::int_> (find)(DeeObject *item, DeeObject *start) {
		DeeObject *args[2];
		args[0] = item;
		args[1] = start;
		return inherit(DeeObject_CallAttrStringHash(this, "find", _Dee_HashSelectC(0x9e66372, 0x2b65fe03bbdde5b2), 2, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3)) Ref<deemon::int_> (find)(DeeObject *item, DeeObject *start, DeeObject *end) {
		DeeObject *args[3];
		args[0] = item;
		args[1] = start;
		args[2] = end;
		return inherit(DeeObject_CallAttrStringHash(this, "find", _Dee_HashSelectC(0x9e66372, 0x2b65fe03bbdde5b2), 3, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3, 4)) Ref<deemon::int_> (find)(DeeObject *item, DeeObject *start, DeeObject *end, DeeObject *key) {
		DeeObject *args[4];
		args[0] = item;
		args[1] = start;
		args[2] = end;
		args[3] = key;
		return inherit(DeeObject_CallAttrStringHash(this, "find", _Dee_HashSelectC(0x9e66372, 0x2b65fe03bbdde5b2), 4, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::int_> (find)(DeeObject *item, DeeObject *start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "find", _Dee_HashSelectC(0x9e66372, 0x2b65fe03bbdde5b2), "oo" DEE_PCKdSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 2, 4)) Ref<deemon::int_> (find)(DeeObject *item, DeeObject *start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "find", _Dee_HashSelectC(0x9e66372, 0x2b65fe03bbdde5b2), "oo" DEE_PCKdSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::int_> (find)(DeeObject *item, DeeObject *start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "find", _Dee_HashSelectC(0x9e66372, 0x2b65fe03bbdde5b2), "oo" DEE_PCKuSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 2, 4)) Ref<deemon::int_> (find)(DeeObject *item, DeeObject *start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "find", _Dee_HashSelectC(0x9e66372, 0x2b65fe03bbdde5b2), "oo" DEE_PCKuSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (find)(DeeObject *item, Dee_ssize_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "find", _Dee_HashSelectC(0x9e66372, 0x2b65fe03bbdde5b2), "o" DEE_PCKdSIZ, item, start));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::int_> (find)(DeeObject *item, Dee_ssize_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "find", _Dee_HashSelectC(0x9e66372, 0x2b65fe03bbdde5b2), "o" DEE_PCKdSIZ "o", item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 3, 4)) Ref<deemon::int_> (find)(DeeObject *item, Dee_ssize_t start, DeeObject *end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "find", _Dee_HashSelectC(0x9e66372, 0x2b65fe03bbdde5b2), "o" DEE_PCKdSIZ "oo", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (find)(DeeObject *item, Dee_ssize_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "find", _Dee_HashSelectC(0x9e66372, 0x2b65fe03bbdde5b2), "o" DEE_PCKdSIZ DEE_PCKdSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::int_> (find)(DeeObject *item, Dee_ssize_t start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "find", _Dee_HashSelectC(0x9e66372, 0x2b65fe03bbdde5b2), "o" DEE_PCKdSIZ DEE_PCKdSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (find)(DeeObject *item, Dee_ssize_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "find", _Dee_HashSelectC(0x9e66372, 0x2b65fe03bbdde5b2), "o" DEE_PCKdSIZ DEE_PCKuSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::int_> (find)(DeeObject *item, Dee_ssize_t start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "find", _Dee_HashSelectC(0x9e66372, 0x2b65fe03bbdde5b2), "o" DEE_PCKdSIZ DEE_PCKuSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (find)(DeeObject *item, size_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "find", _Dee_HashSelectC(0x9e66372, 0x2b65fe03bbdde5b2), "o" DEE_PCKuSIZ, item, start));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::int_> (find)(DeeObject *item, size_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "find", _Dee_HashSelectC(0x9e66372, 0x2b65fe03bbdde5b2), "o" DEE_PCKuSIZ "o", item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 3, 4)) Ref<deemon::int_> (find)(DeeObject *item, size_t start, DeeObject *end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "find", _Dee_HashSelectC(0x9e66372, 0x2b65fe03bbdde5b2), "o" DEE_PCKuSIZ "oo", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (find)(DeeObject *item, size_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "find", _Dee_HashSelectC(0x9e66372, 0x2b65fe03bbdde5b2), "o" DEE_PCKuSIZ DEE_PCKdSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::int_> (find)(DeeObject *item, size_t start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "find", _Dee_HashSelectC(0x9e66372, 0x2b65fe03bbdde5b2), "o" DEE_PCKuSIZ DEE_PCKdSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (find)(DeeObject *item, size_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "find", _Dee_HashSelectC(0x9e66372, 0x2b65fe03bbdde5b2), "o" DEE_PCKuSIZ DEE_PCKuSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::int_> (find)(DeeObject *item, size_t start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "find", _Dee_HashSelectC(0x9e66372, 0x2b65fe03bbdde5b2), "o" DEE_PCKuSIZ DEE_PCKuSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (rfind)(DeeObject *item) {
		DeeObject *args[1];
		args[0] = item;
		return inherit(DeeObject_CallAttrStringHash(this, "rfind", _Dee_HashSelectC(0xfb368ca, 0x8b40ffa7172dc59d), 1, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::int_> (rfind)(DeeObject *item, DeeObject *start) {
		DeeObject *args[2];
		args[0] = item;
		args[1] = start;
		return inherit(DeeObject_CallAttrStringHash(this, "rfind", _Dee_HashSelectC(0xfb368ca, 0x8b40ffa7172dc59d), 2, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3)) Ref<deemon::int_> (rfind)(DeeObject *item, DeeObject *start, DeeObject *end) {
		DeeObject *args[3];
		args[0] = item;
		args[1] = start;
		args[2] = end;
		return inherit(DeeObject_CallAttrStringHash(this, "rfind", _Dee_HashSelectC(0xfb368ca, 0x8b40ffa7172dc59d), 3, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3, 4)) Ref<deemon::int_> (rfind)(DeeObject *item, DeeObject *start, DeeObject *end, DeeObject *key) {
		DeeObject *args[4];
		args[0] = item;
		args[1] = start;
		args[2] = end;
		args[3] = key;
		return inherit(DeeObject_CallAttrStringHash(this, "rfind", _Dee_HashSelectC(0xfb368ca, 0x8b40ffa7172dc59d), 4, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::int_> (rfind)(DeeObject *item, DeeObject *start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rfind", _Dee_HashSelectC(0xfb368ca, 0x8b40ffa7172dc59d), "oo" DEE_PCKdSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 2, 4)) Ref<deemon::int_> (rfind)(DeeObject *item, DeeObject *start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rfind", _Dee_HashSelectC(0xfb368ca, 0x8b40ffa7172dc59d), "oo" DEE_PCKdSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::int_> (rfind)(DeeObject *item, DeeObject *start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rfind", _Dee_HashSelectC(0xfb368ca, 0x8b40ffa7172dc59d), "oo" DEE_PCKuSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 2, 4)) Ref<deemon::int_> (rfind)(DeeObject *item, DeeObject *start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rfind", _Dee_HashSelectC(0xfb368ca, 0x8b40ffa7172dc59d), "oo" DEE_PCKuSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (rfind)(DeeObject *item, Dee_ssize_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rfind", _Dee_HashSelectC(0xfb368ca, 0x8b40ffa7172dc59d), "o" DEE_PCKdSIZ, item, start));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::int_> (rfind)(DeeObject *item, Dee_ssize_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rfind", _Dee_HashSelectC(0xfb368ca, 0x8b40ffa7172dc59d), "o" DEE_PCKdSIZ "o", item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 3, 4)) Ref<deemon::int_> (rfind)(DeeObject *item, Dee_ssize_t start, DeeObject *end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rfind", _Dee_HashSelectC(0xfb368ca, 0x8b40ffa7172dc59d), "o" DEE_PCKdSIZ "oo", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (rfind)(DeeObject *item, Dee_ssize_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rfind", _Dee_HashSelectC(0xfb368ca, 0x8b40ffa7172dc59d), "o" DEE_PCKdSIZ DEE_PCKdSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::int_> (rfind)(DeeObject *item, Dee_ssize_t start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rfind", _Dee_HashSelectC(0xfb368ca, 0x8b40ffa7172dc59d), "o" DEE_PCKdSIZ DEE_PCKdSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (rfind)(DeeObject *item, Dee_ssize_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rfind", _Dee_HashSelectC(0xfb368ca, 0x8b40ffa7172dc59d), "o" DEE_PCKdSIZ DEE_PCKuSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::int_> (rfind)(DeeObject *item, Dee_ssize_t start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rfind", _Dee_HashSelectC(0xfb368ca, 0x8b40ffa7172dc59d), "o" DEE_PCKdSIZ DEE_PCKuSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (rfind)(DeeObject *item, size_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rfind", _Dee_HashSelectC(0xfb368ca, 0x8b40ffa7172dc59d), "o" DEE_PCKuSIZ, item, start));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::int_> (rfind)(DeeObject *item, size_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rfind", _Dee_HashSelectC(0xfb368ca, 0x8b40ffa7172dc59d), "o" DEE_PCKuSIZ "o", item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 3, 4)) Ref<deemon::int_> (rfind)(DeeObject *item, size_t start, DeeObject *end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rfind", _Dee_HashSelectC(0xfb368ca, 0x8b40ffa7172dc59d), "o" DEE_PCKuSIZ "oo", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (rfind)(DeeObject *item, size_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rfind", _Dee_HashSelectC(0xfb368ca, 0x8b40ffa7172dc59d), "o" DEE_PCKuSIZ DEE_PCKdSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::int_> (rfind)(DeeObject *item, size_t start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rfind", _Dee_HashSelectC(0xfb368ca, 0x8b40ffa7172dc59d), "o" DEE_PCKuSIZ DEE_PCKdSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (rfind)(DeeObject *item, size_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rfind", _Dee_HashSelectC(0xfb368ca, 0x8b40ffa7172dc59d), "o" DEE_PCKuSIZ DEE_PCKuSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::int_> (rfind)(DeeObject *item, size_t start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rfind", _Dee_HashSelectC(0xfb368ca, 0x8b40ffa7172dc59d), "o" DEE_PCKuSIZ DEE_PCKuSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1, 2, 3)) Ref<deemon::int_> (index)(DeeObject *item, DeeObject *start, DeeObject *end) {
		DeeObject *args[3];
		args[0] = item;
		args[1] = start;
		args[2] = end;
		return inherit(DeeObject_CallAttrStringHash(this, "index", _Dee_HashSelectC(0x77f34f0, 0x440d5888c0ff3081), 3, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3, 4)) Ref<deemon::int_> (index)(DeeObject *item, DeeObject *start, DeeObject *end, DeeObject *key) {
		DeeObject *args[4];
		args[0] = item;
		args[1] = start;
		args[2] = end;
		args[3] = key;
		return inherit(DeeObject_CallAttrStringHash(this, "index", _Dee_HashSelectC(0x77f34f0, 0x440d5888c0ff3081), 4, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::int_> (index)(DeeObject *item, DeeObject *start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "index", _Dee_HashSelectC(0x77f34f0, 0x440d5888c0ff3081), "oo" DEE_PCKdSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 2, 4)) Ref<deemon::int_> (index)(DeeObject *item, DeeObject *start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "index", _Dee_HashSelectC(0x77f34f0, 0x440d5888c0ff3081), "oo" DEE_PCKdSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::int_> (index)(DeeObject *item, DeeObject *start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "index", _Dee_HashSelectC(0x77f34f0, 0x440d5888c0ff3081), "oo" DEE_PCKuSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 2, 4)) Ref<deemon::int_> (index)(DeeObject *item, DeeObject *start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "index", _Dee_HashSelectC(0x77f34f0, 0x440d5888c0ff3081), "oo" DEE_PCKuSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::int_> (index)(DeeObject *item, Dee_ssize_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "index", _Dee_HashSelectC(0x77f34f0, 0x440d5888c0ff3081), "o" DEE_PCKdSIZ "o", item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 3, 4)) Ref<deemon::int_> (index)(DeeObject *item, Dee_ssize_t start, DeeObject *end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "index", _Dee_HashSelectC(0x77f34f0, 0x440d5888c0ff3081), "o" DEE_PCKdSIZ "oo", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (index)(DeeObject *item, Dee_ssize_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "index", _Dee_HashSelectC(0x77f34f0, 0x440d5888c0ff3081), "o" DEE_PCKdSIZ DEE_PCKdSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::int_> (index)(DeeObject *item, Dee_ssize_t start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "index", _Dee_HashSelectC(0x77f34f0, 0x440d5888c0ff3081), "o" DEE_PCKdSIZ DEE_PCKdSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (index)(DeeObject *item, Dee_ssize_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "index", _Dee_HashSelectC(0x77f34f0, 0x440d5888c0ff3081), "o" DEE_PCKdSIZ DEE_PCKuSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::int_> (index)(DeeObject *item, Dee_ssize_t start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "index", _Dee_HashSelectC(0x77f34f0, 0x440d5888c0ff3081), "o" DEE_PCKdSIZ DEE_PCKuSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::int_> (index)(DeeObject *item, size_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "index", _Dee_HashSelectC(0x77f34f0, 0x440d5888c0ff3081), "o" DEE_PCKuSIZ "o", item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 3, 4)) Ref<deemon::int_> (index)(DeeObject *item, size_t start, DeeObject *end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "index", _Dee_HashSelectC(0x77f34f0, 0x440d5888c0ff3081), "o" DEE_PCKuSIZ "oo", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (index)(DeeObject *item, size_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "index", _Dee_HashSelectC(0x77f34f0, 0x440d5888c0ff3081), "o" DEE_PCKuSIZ DEE_PCKdSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::int_> (index)(DeeObject *item, size_t start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "index", _Dee_HashSelectC(0x77f34f0, 0x440d5888c0ff3081), "o" DEE_PCKuSIZ DEE_PCKdSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (index)(DeeObject *item, size_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "index", _Dee_HashSelectC(0x77f34f0, 0x440d5888c0ff3081), "o" DEE_PCKuSIZ DEE_PCKuSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::int_> (index)(DeeObject *item, size_t start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "index", _Dee_HashSelectC(0x77f34f0, 0x440d5888c0ff3081), "o" DEE_PCKuSIZ DEE_PCKuSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (rindex)(DeeObject *item) {
		DeeObject *args[1];
		args[0] = item;
		return inherit(DeeObject_CallAttrStringHash(this, "rindex", _Dee_HashSelectC(0x1eb52bf1, 0xbce198a5867b343c), 1, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::int_> (rindex)(DeeObject *item, DeeObject *start) {
		DeeObject *args[2];
		args[0] = item;
		args[1] = start;
		return inherit(DeeObject_CallAttrStringHash(this, "rindex", _Dee_HashSelectC(0x1eb52bf1, 0xbce198a5867b343c), 2, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3)) Ref<deemon::int_> (rindex)(DeeObject *item, DeeObject *start, DeeObject *end) {
		DeeObject *args[3];
		args[0] = item;
		args[1] = start;
		args[2] = end;
		return inherit(DeeObject_CallAttrStringHash(this, "rindex", _Dee_HashSelectC(0x1eb52bf1, 0xbce198a5867b343c), 3, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3, 4)) Ref<deemon::int_> (rindex)(DeeObject *item, DeeObject *start, DeeObject *end, DeeObject *key) {
		DeeObject *args[4];
		args[0] = item;
		args[1] = start;
		args[2] = end;
		args[3] = key;
		return inherit(DeeObject_CallAttrStringHash(this, "rindex", _Dee_HashSelectC(0x1eb52bf1, 0xbce198a5867b343c), 4, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::int_> (rindex)(DeeObject *item, DeeObject *start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rindex", _Dee_HashSelectC(0x1eb52bf1, 0xbce198a5867b343c), "oo" DEE_PCKdSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 2, 4)) Ref<deemon::int_> (rindex)(DeeObject *item, DeeObject *start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rindex", _Dee_HashSelectC(0x1eb52bf1, 0xbce198a5867b343c), "oo" DEE_PCKdSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::int_> (rindex)(DeeObject *item, DeeObject *start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rindex", _Dee_HashSelectC(0x1eb52bf1, 0xbce198a5867b343c), "oo" DEE_PCKuSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 2, 4)) Ref<deemon::int_> (rindex)(DeeObject *item, DeeObject *start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rindex", _Dee_HashSelectC(0x1eb52bf1, 0xbce198a5867b343c), "oo" DEE_PCKuSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (rindex)(DeeObject *item, Dee_ssize_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rindex", _Dee_HashSelectC(0x1eb52bf1, 0xbce198a5867b343c), "o" DEE_PCKdSIZ, item, start));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::int_> (rindex)(DeeObject *item, Dee_ssize_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rindex", _Dee_HashSelectC(0x1eb52bf1, 0xbce198a5867b343c), "o" DEE_PCKdSIZ "o", item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 3, 4)) Ref<deemon::int_> (rindex)(DeeObject *item, Dee_ssize_t start, DeeObject *end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rindex", _Dee_HashSelectC(0x1eb52bf1, 0xbce198a5867b343c), "o" DEE_PCKdSIZ "oo", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (rindex)(DeeObject *item, Dee_ssize_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rindex", _Dee_HashSelectC(0x1eb52bf1, 0xbce198a5867b343c), "o" DEE_PCKdSIZ DEE_PCKdSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::int_> (rindex)(DeeObject *item, Dee_ssize_t start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rindex", _Dee_HashSelectC(0x1eb52bf1, 0xbce198a5867b343c), "o" DEE_PCKdSIZ DEE_PCKdSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (rindex)(DeeObject *item, Dee_ssize_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rindex", _Dee_HashSelectC(0x1eb52bf1, 0xbce198a5867b343c), "o" DEE_PCKdSIZ DEE_PCKuSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::int_> (rindex)(DeeObject *item, Dee_ssize_t start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rindex", _Dee_HashSelectC(0x1eb52bf1, 0xbce198a5867b343c), "o" DEE_PCKdSIZ DEE_PCKuSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (rindex)(DeeObject *item, size_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rindex", _Dee_HashSelectC(0x1eb52bf1, 0xbce198a5867b343c), "o" DEE_PCKuSIZ, item, start));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::int_> (rindex)(DeeObject *item, size_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rindex", _Dee_HashSelectC(0x1eb52bf1, 0xbce198a5867b343c), "o" DEE_PCKuSIZ "o", item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 3, 4)) Ref<deemon::int_> (rindex)(DeeObject *item, size_t start, DeeObject *end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rindex", _Dee_HashSelectC(0x1eb52bf1, 0xbce198a5867b343c), "o" DEE_PCKuSIZ "oo", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (rindex)(DeeObject *item, size_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rindex", _Dee_HashSelectC(0x1eb52bf1, 0xbce198a5867b343c), "o" DEE_PCKuSIZ DEE_PCKdSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::int_> (rindex)(DeeObject *item, size_t start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rindex", _Dee_HashSelectC(0x1eb52bf1, 0xbce198a5867b343c), "o" DEE_PCKuSIZ DEE_PCKdSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (rindex)(DeeObject *item, size_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rindex", _Dee_HashSelectC(0x1eb52bf1, 0xbce198a5867b343c), "o" DEE_PCKuSIZ DEE_PCKuSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::int_> (rindex)(DeeObject *item, size_t start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rindex", _Dee_HashSelectC(0x1eb52bf1, 0xbce198a5867b343c), "o" DEE_PCKuSIZ DEE_PCKuSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Sequence<Sequence<T> > > (segments)(DeeObject *segmentSize) {
		DeeObject *args[1];
		args[0] = segmentSize;
		return inherit(DeeObject_CallAttrStringHash(this, "segments", _Dee_HashSelectC(0x20acb0bd, 0x8554d160c212a46a), 1, args));
	}
	WUNUSED Ref<Sequence<Sequence<T> > > (segments)(Dee_ssize_t segmentSize) {
		return inherit(DeeObject_CallAttrStringHashf(this, "segments", _Dee_HashSelectC(0x20acb0bd, 0x8554d160c212a46a),  DEE_PCKdSIZ, segmentSize));
	}
	WUNUSED Ref<Sequence<Sequence<T> > > (segments)(size_t segmentSize) {
		return inherit(DeeObject_CallAttrStringHashf(this, "segments", _Dee_HashSelectC(0x20acb0bd, 0x8554d160c212a46a),  DEE_PCKuSIZ, segmentSize));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Sequence<Sequence<T> > > (distribute)(DeeObject *bucketCount) {
		DeeObject *args[1];
		args[0] = bucketCount;
		return inherit(DeeObject_CallAttrStringHash(this, "distribute", _Dee_HashSelectC(0xfe7253a7, 0x63150bb7d5354c6f), 1, args));
	}
	WUNUSED Ref<Sequence<Sequence<T> > > (distribute)(Dee_ssize_t bucketCount) {
		return inherit(DeeObject_CallAttrStringHashf(this, "distribute", _Dee_HashSelectC(0xfe7253a7, 0x63150bb7d5354c6f),  DEE_PCKdSIZ, bucketCount));
	}
	WUNUSED Ref<Sequence<Sequence<T> > > (distribute)(size_t bucketCount) {
		return inherit(DeeObject_CallAttrStringHashf(this, "distribute", _Dee_HashSelectC(0xfe7253a7, 0x63150bb7d5354c6f),  DEE_PCKuSIZ, bucketCount));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Sequence<Sequence<T> > > (combinations)(DeeObject *r) {
		DeeObject *args[1];
		args[0] = r;
		return inherit(DeeObject_CallAttrStringHash(this, "combinations", _Dee_HashSelectC(0x184d9b51, 0x3e5802b7656c4900), 1, args));
	}
	WUNUSED Ref<Sequence<Sequence<T> > > (combinations)(Dee_ssize_t r) {
		return inherit(DeeObject_CallAttrStringHashf(this, "combinations", _Dee_HashSelectC(0x184d9b51, 0x3e5802b7656c4900),  DEE_PCKdSIZ, r));
	}
	WUNUSED Ref<Sequence<Sequence<T> > > (combinations)(size_t r) {
		return inherit(DeeObject_CallAttrStringHashf(this, "combinations", _Dee_HashSelectC(0x184d9b51, 0x3e5802b7656c4900),  DEE_PCKuSIZ, r));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Sequence<Sequence<T> > > (repeatcombinations)(DeeObject *r) {
		DeeObject *args[1];
		args[0] = r;
		return inherit(DeeObject_CallAttrStringHash(this, "repeatcombinations", _Dee_HashSelectC(0xa3bc4ae1, 0x7ef1d21507ad27f5), 1, args));
	}
	WUNUSED Ref<Sequence<Sequence<T> > > (repeatcombinations)(Dee_ssize_t r) {
		return inherit(DeeObject_CallAttrStringHashf(this, "repeatcombinations", _Dee_HashSelectC(0xa3bc4ae1, 0x7ef1d21507ad27f5),  DEE_PCKdSIZ, r));
	}
	WUNUSED Ref<Sequence<Sequence<T> > > (repeatcombinations)(size_t r) {
		return inherit(DeeObject_CallAttrStringHashf(this, "repeatcombinations", _Dee_HashSelectC(0xa3bc4ae1, 0x7ef1d21507ad27f5),  DEE_PCKuSIZ, r));
	}
	WUNUSED Ref<Sequence<Sequence<T> > > (permutations)() {
		return inherit(DeeObject_CallAttrStringHash(this, "permutations", _Dee_HashSelectC(0x923fbd44, 0x498779abec4910f6), 0, NULL));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Sequence<Sequence<T> > > (permutations)(DeeObject *r) {
		DeeObject *args[1];
		args[0] = r;
		return inherit(DeeObject_CallAttrStringHash(this, "permutations", _Dee_HashSelectC(0x923fbd44, 0x498779abec4910f6), 1, args));
	}
	WUNUSED Ref<Sequence<Sequence<T> > > (permutations)(Dee_ssize_t r) {
		return inherit(DeeObject_CallAttrStringHashf(this, "permutations", _Dee_HashSelectC(0x923fbd44, 0x498779abec4910f6),  DEE_PCKdSIZ, r));
	}
	WUNUSED Ref<Sequence<Sequence<T> > > (permutations)(size_t r) {
		return inherit(DeeObject_CallAttrStringHashf(this, "permutations", _Dee_HashSelectC(0x923fbd44, 0x498779abec4910f6),  DEE_PCKuSIZ, r));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Sequence<T> > (byhash)(DeeObject *template_) {
		DeeObject *args[1];
		args[0] = template_;
		return inherit(DeeObject_CallAttrStringHash(this, "byhash", _Dee_HashSelectC(0x7b5277ce, 0x773c8074445a28d9), 1, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Sequence<T> > (unpack)(DeeObject *size) {
		DeeObject *args[1];
		args[0] = size;
		return inherit(DeeObject_CallAttrStringHash(this, "unpack", _Dee_HashSelectC(0x579a82d1, 0x2714bac446bf9c9), 1, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<Sequence<T> > (unpack)(DeeObject *minsize, DeeObject *maxsize) {
		DeeObject *args[2];
		args[0] = minsize;
		args[1] = maxsize;
		return inherit(DeeObject_CallAttrStringHash(this, "unpack", _Dee_HashSelectC(0x579a82d1, 0x2714bac446bf9c9), 2, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Sequence<T> > (unpack)(DeeObject *minsize, Dee_ssize_t maxsize) {
		return inherit(DeeObject_CallAttrStringHashf(this, "unpack", _Dee_HashSelectC(0x579a82d1, 0x2714bac446bf9c9), "o" DEE_PCKdSIZ, minsize, maxsize));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Sequence<T> > (unpack)(DeeObject *minsize, size_t maxsize) {
		return inherit(DeeObject_CallAttrStringHashf(this, "unpack", _Dee_HashSelectC(0x579a82d1, 0x2714bac446bf9c9), "o" DEE_PCKuSIZ, minsize, maxsize));
	}
	WUNUSED Ref<Sequence<T> > (unpack)(Dee_ssize_t size) {
		return inherit(DeeObject_CallAttrStringHashf(this, "unpack", _Dee_HashSelectC(0x579a82d1, 0x2714bac446bf9c9),  DEE_PCKdSIZ, size));
	}
	WUNUSED NONNULL_CXX((2)) Ref<Sequence<T> > (unpack)(Dee_ssize_t minsize, DeeObject *maxsize) {
		return inherit(DeeObject_CallAttrStringHashf(this, "unpack", _Dee_HashSelectC(0x579a82d1, 0x2714bac446bf9c9),  DEE_PCKdSIZ "o", minsize, maxsize));
	}
	WUNUSED Ref<Sequence<T> > (unpack)(Dee_ssize_t minsize, Dee_ssize_t maxsize) {
		return inherit(DeeObject_CallAttrStringHashf(this, "unpack", _Dee_HashSelectC(0x579a82d1, 0x2714bac446bf9c9),  DEE_PCKdSIZ DEE_PCKdSIZ, minsize, maxsize));
	}
	WUNUSED Ref<Sequence<T> > (unpack)(Dee_ssize_t minsize, size_t maxsize) {
		return inherit(DeeObject_CallAttrStringHashf(this, "unpack", _Dee_HashSelectC(0x579a82d1, 0x2714bac446bf9c9),  DEE_PCKdSIZ DEE_PCKuSIZ, minsize, maxsize));
	}
	WUNUSED Ref<Sequence<T> > (unpack)(size_t size) {
		return inherit(DeeObject_CallAttrStringHashf(this, "unpack", _Dee_HashSelectC(0x579a82d1, 0x2714bac446bf9c9),  DEE_PCKuSIZ, size));
	}
	WUNUSED NONNULL_CXX((2)) Ref<Sequence<T> > (unpack)(size_t minsize, DeeObject *maxsize) {
		return inherit(DeeObject_CallAttrStringHashf(this, "unpack", _Dee_HashSelectC(0x579a82d1, 0x2714bac446bf9c9),  DEE_PCKuSIZ "o", minsize, maxsize));
	}
	WUNUSED Ref<Sequence<T> > (unpack)(size_t minsize, Dee_ssize_t maxsize) {
		return inherit(DeeObject_CallAttrStringHashf(this, "unpack", _Dee_HashSelectC(0x579a82d1, 0x2714bac446bf9c9),  DEE_PCKuSIZ DEE_PCKdSIZ, minsize, maxsize));
	}
	WUNUSED Ref<Sequence<T> > (unpack)(size_t minsize, size_t maxsize) {
		return inherit(DeeObject_CallAttrStringHashf(this, "unpack", _Dee_HashSelectC(0x579a82d1, 0x2714bac446bf9c9),  DEE_PCKuSIZ DEE_PCKuSIZ, minsize, maxsize));
	}
	WUNUSED Ref<Set<T> > (distinct)() {
		return inherit(DeeObject_CallAttrStringHash(this, "distinct", _Dee_HashSelectC(0xe1eb56d, 0x9c50bb058e287b02), 0, NULL));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Set<T> > (distinct)(DeeObject *key) {
		DeeObject *args[1];
		args[0] = key;
		return inherit(DeeObject_CallAttrStringHash(this, "distinct", _Dee_HashSelectC(0xe1eb56d, 0x9c50bb058e287b02), 1, args));
	}
	WUNUSED Ref<Sequence<T> > (reversed)() {
		return inherit(DeeObject_CallAttrStringHash(this, "reversed", _Dee_HashSelectC(0x78e8d950, 0xde2071a9b2ca405f), 0, NULL));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Sequence<T> > (reversed)(DeeObject *start) {
		DeeObject *args[1];
		args[0] = start;
		return inherit(DeeObject_CallAttrStringHash(this, "reversed", _Dee_HashSelectC(0x78e8d950, 0xde2071a9b2ca405f), 1, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<Sequence<T> > (reversed)(DeeObject *start, DeeObject *end) {
		DeeObject *args[2];
		args[0] = start;
		args[1] = end;
		return inherit(DeeObject_CallAttrStringHash(this, "reversed", _Dee_HashSelectC(0x78e8d950, 0xde2071a9b2ca405f), 2, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Sequence<T> > (reversed)(DeeObject *start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "reversed", _Dee_HashSelectC(0x78e8d950, 0xde2071a9b2ca405f), "o" DEE_PCKdSIZ, start, end));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Sequence<T> > (reversed)(DeeObject *start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "reversed", _Dee_HashSelectC(0x78e8d950, 0xde2071a9b2ca405f), "o" DEE_PCKuSIZ, start, end));
	}
	WUNUSED Ref<Sequence<T> > (reversed)(Dee_ssize_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "reversed", _Dee_HashSelectC(0x78e8d950, 0xde2071a9b2ca405f),  DEE_PCKdSIZ, start));
	}
	WUNUSED NONNULL_CXX((2)) Ref<Sequence<T> > (reversed)(Dee_ssize_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "reversed", _Dee_HashSelectC(0x78e8d950, 0xde2071a9b2ca405f),  DEE_PCKdSIZ "o", start, end));
	}
	WUNUSED Ref<Sequence<T> > (reversed)(Dee_ssize_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "reversed", _Dee_HashSelectC(0x78e8d950, 0xde2071a9b2ca405f),  DEE_PCKdSIZ DEE_PCKdSIZ, start, end));
	}
	WUNUSED Ref<Sequence<T> > (reversed)(Dee_ssize_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "reversed", _Dee_HashSelectC(0x78e8d950, 0xde2071a9b2ca405f),  DEE_PCKdSIZ DEE_PCKuSIZ, start, end));
	}
	WUNUSED Ref<Sequence<T> > (reversed)(size_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "reversed", _Dee_HashSelectC(0x78e8d950, 0xde2071a9b2ca405f),  DEE_PCKuSIZ, start));
	}
	WUNUSED NONNULL_CXX((2)) Ref<Sequence<T> > (reversed)(size_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "reversed", _Dee_HashSelectC(0x78e8d950, 0xde2071a9b2ca405f),  DEE_PCKuSIZ "o", start, end));
	}
	WUNUSED Ref<Sequence<T> > (reversed)(size_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "reversed", _Dee_HashSelectC(0x78e8d950, 0xde2071a9b2ca405f),  DEE_PCKuSIZ DEE_PCKdSIZ, start, end));
	}
	WUNUSED Ref<Sequence<T> > (reversed)(size_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "reversed", _Dee_HashSelectC(0x78e8d950, 0xde2071a9b2ca405f),  DEE_PCKuSIZ DEE_PCKuSIZ, start, end));
	}
	WUNUSED Ref<Sequence<T> > (sorted)() {
		return inherit(DeeObject_CallAttrStringHash(this, "sorted", _Dee_HashSelectC(0x93fb6d4, 0x2fc60c43cfaf0860), 0, NULL));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Sequence<T> > (sorted)(DeeObject *start) {
		DeeObject *args[1];
		args[0] = start;
		return inherit(DeeObject_CallAttrStringHash(this, "sorted", _Dee_HashSelectC(0x93fb6d4, 0x2fc60c43cfaf0860), 1, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<Sequence<T> > (sorted)(DeeObject *start, DeeObject *end) {
		DeeObject *args[2];
		args[0] = start;
		args[1] = end;
		return inherit(DeeObject_CallAttrStringHash(this, "sorted", _Dee_HashSelectC(0x93fb6d4, 0x2fc60c43cfaf0860), 2, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3)) Ref<Sequence<T> > (sorted)(DeeObject *start, DeeObject *end, DeeObject *key) {
		DeeObject *args[3];
		args[0] = start;
		args[1] = end;
		args[2] = key;
		return inherit(DeeObject_CallAttrStringHash(this, "sorted", _Dee_HashSelectC(0x93fb6d4, 0x2fc60c43cfaf0860), 3, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Sequence<T> > (sorted)(DeeObject *start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "sorted", _Dee_HashSelectC(0x93fb6d4, 0x2fc60c43cfaf0860), "o" DEE_PCKdSIZ, start, end));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<Sequence<T> > (sorted)(DeeObject *start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "sorted", _Dee_HashSelectC(0x93fb6d4, 0x2fc60c43cfaf0860), "o" DEE_PCKdSIZ "o", start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Sequence<T> > (sorted)(DeeObject *start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "sorted", _Dee_HashSelectC(0x93fb6d4, 0x2fc60c43cfaf0860), "o" DEE_PCKuSIZ, start, end));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<Sequence<T> > (sorted)(DeeObject *start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "sorted", _Dee_HashSelectC(0x93fb6d4, 0x2fc60c43cfaf0860), "o" DEE_PCKuSIZ "o", start, end, key));
	}
	WUNUSED Ref<Sequence<T> > (sorted)(Dee_ssize_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "sorted", _Dee_HashSelectC(0x93fb6d4, 0x2fc60c43cfaf0860),  DEE_PCKdSIZ, start));
	}
	WUNUSED NONNULL_CXX((2)) Ref<Sequence<T> > (sorted)(Dee_ssize_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "sorted", _Dee_HashSelectC(0x93fb6d4, 0x2fc60c43cfaf0860),  DEE_PCKdSIZ "o", start, end));
	}
	WUNUSED NONNULL_CXX((2, 3)) Ref<Sequence<T> > (sorted)(Dee_ssize_t start, DeeObject *end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "sorted", _Dee_HashSelectC(0x93fb6d4, 0x2fc60c43cfaf0860),  DEE_PCKdSIZ "oo", start, end, key));
	}
	WUNUSED Ref<Sequence<T> > (sorted)(Dee_ssize_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "sorted", _Dee_HashSelectC(0x93fb6d4, 0x2fc60c43cfaf0860),  DEE_PCKdSIZ DEE_PCKdSIZ, start, end));
	}
	WUNUSED NONNULL_CXX((3)) Ref<Sequence<T> > (sorted)(Dee_ssize_t start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "sorted", _Dee_HashSelectC(0x93fb6d4, 0x2fc60c43cfaf0860),  DEE_PCKdSIZ DEE_PCKdSIZ "o", start, end, key));
	}
	WUNUSED Ref<Sequence<T> > (sorted)(Dee_ssize_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "sorted", _Dee_HashSelectC(0x93fb6d4, 0x2fc60c43cfaf0860),  DEE_PCKdSIZ DEE_PCKuSIZ, start, end));
	}
	WUNUSED NONNULL_CXX((3)) Ref<Sequence<T> > (sorted)(Dee_ssize_t start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "sorted", _Dee_HashSelectC(0x93fb6d4, 0x2fc60c43cfaf0860),  DEE_PCKdSIZ DEE_PCKuSIZ "o", start, end, key));
	}
	WUNUSED Ref<Sequence<T> > (sorted)(size_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "sorted", _Dee_HashSelectC(0x93fb6d4, 0x2fc60c43cfaf0860),  DEE_PCKuSIZ, start));
	}
	WUNUSED NONNULL_CXX((2)) Ref<Sequence<T> > (sorted)(size_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "sorted", _Dee_HashSelectC(0x93fb6d4, 0x2fc60c43cfaf0860),  DEE_PCKuSIZ "o", start, end));
	}
	WUNUSED NONNULL_CXX((2, 3)) Ref<Sequence<T> > (sorted)(size_t start, DeeObject *end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "sorted", _Dee_HashSelectC(0x93fb6d4, 0x2fc60c43cfaf0860),  DEE_PCKuSIZ "oo", start, end, key));
	}
	WUNUSED Ref<Sequence<T> > (sorted)(size_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "sorted", _Dee_HashSelectC(0x93fb6d4, 0x2fc60c43cfaf0860),  DEE_PCKuSIZ DEE_PCKdSIZ, start, end));
	}
	WUNUSED NONNULL_CXX((3)) Ref<Sequence<T> > (sorted)(size_t start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "sorted", _Dee_HashSelectC(0x93fb6d4, 0x2fc60c43cfaf0860),  DEE_PCKuSIZ DEE_PCKdSIZ "o", start, end, key));
	}
	WUNUSED Ref<Sequence<T> > (sorted)(size_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "sorted", _Dee_HashSelectC(0x93fb6d4, 0x2fc60c43cfaf0860),  DEE_PCKuSIZ DEE_PCKuSIZ, start, end));
	}
	WUNUSED NONNULL_CXX((3)) Ref<Sequence<T> > (sorted)(size_t start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "sorted", _Dee_HashSelectC(0x93fb6d4, 0x2fc60c43cfaf0860),  DEE_PCKuSIZ DEE_PCKuSIZ "o", start, end, key));
	}
	NONNULL_CXX((1, 2)) void (insert)(DeeObject *index, DeeObject *item) {
		DeeObject *args[2];
		args[0] = index;
		args[1] = item;
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "insert", _Dee_HashSelectC(0x71d74a66, 0x5e168c86241590d7), 2, args)));
	}
	NONNULL_CXX((2)) void (insert)(Dee_ssize_t index, DeeObject *item) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "insert", _Dee_HashSelectC(0x71d74a66, 0x5e168c86241590d7),  DEE_PCKdSIZ "o", index, item)));
	}
	NONNULL_CXX((2)) void (insert)(size_t index, DeeObject *item) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "insert", _Dee_HashSelectC(0x71d74a66, 0x5e168c86241590d7),  DEE_PCKuSIZ "o", index, item)));
	}
	NONNULL_CXX((1, 2)) void (insertall)(DeeObject *index, DeeObject *items) {
		DeeObject *args[2];
		args[0] = index;
		args[1] = items;
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "insertall", _Dee_HashSelectC(0xbf9bc3a9, 0x4f85971d093a27f2), 2, args)));
	}
	NONNULL_CXX((2)) void (insertall)(Dee_ssize_t index, DeeObject *items) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "insertall", _Dee_HashSelectC(0xbf9bc3a9, 0x4f85971d093a27f2),  DEE_PCKdSIZ "o", index, items)));
	}
	NONNULL_CXX((2)) void (insertall)(size_t index, DeeObject *items) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "insertall", _Dee_HashSelectC(0xbf9bc3a9, 0x4f85971d093a27f2),  DEE_PCKuSIZ "o", index, items)));
	}
	NONNULL_CXX((1)) void (append)(DeeObject *item) {
		DeeObject *args[1];
		args[0] = item;
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "append", _Dee_HashSelectC(0x5f19594f, 0x8c2b7c1aba65d5ee), 1, args)));
	}
	NONNULL_CXX((1)) void (extend)(DeeObject *items) {
		DeeObject *args[1];
		args[0] = items;
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "extend", _Dee_HashSelectC(0x960b75e7, 0xba076858e3adb055), 1, args)));
	}
	NONNULL_CXX((1)) void (erase)(DeeObject *index) {
		DeeObject *args[1];
		args[0] = index;
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "erase", _Dee_HashSelectC(0x6f5916cf, 0x65f9c8b6514af4e5), 1, args)));
	}
	NONNULL_CXX((1, 2)) void (erase)(DeeObject *index, DeeObject *count) {
		DeeObject *args[2];
		args[0] = index;
		args[1] = count;
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "erase", _Dee_HashSelectC(0x6f5916cf, 0x65f9c8b6514af4e5), 2, args)));
	}
	NONNULL_CXX((1)) void (erase)(DeeObject *index, Dee_ssize_t count) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "erase", _Dee_HashSelectC(0x6f5916cf, 0x65f9c8b6514af4e5), "o" DEE_PCKdSIZ, index, count)));
	}
	NONNULL_CXX((1)) void (erase)(DeeObject *index, size_t count) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "erase", _Dee_HashSelectC(0x6f5916cf, 0x65f9c8b6514af4e5), "o" DEE_PCKuSIZ, index, count)));
	}
	void (erase)(Dee_ssize_t index) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "erase", _Dee_HashSelectC(0x6f5916cf, 0x65f9c8b6514af4e5),  DEE_PCKdSIZ, index)));
	}
	NONNULL_CXX((2)) void (erase)(Dee_ssize_t index, DeeObject *count) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "erase", _Dee_HashSelectC(0x6f5916cf, 0x65f9c8b6514af4e5),  DEE_PCKdSIZ "o", index, count)));
	}
	void (erase)(Dee_ssize_t index, Dee_ssize_t count) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "erase", _Dee_HashSelectC(0x6f5916cf, 0x65f9c8b6514af4e5),  DEE_PCKdSIZ DEE_PCKdSIZ, index, count)));
	}
	void (erase)(Dee_ssize_t index, size_t count) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "erase", _Dee_HashSelectC(0x6f5916cf, 0x65f9c8b6514af4e5),  DEE_PCKdSIZ DEE_PCKuSIZ, index, count)));
	}
	void (erase)(size_t index) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "erase", _Dee_HashSelectC(0x6f5916cf, 0x65f9c8b6514af4e5),  DEE_PCKuSIZ, index)));
	}
	NONNULL_CXX((2)) void (erase)(size_t index, DeeObject *count) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "erase", _Dee_HashSelectC(0x6f5916cf, 0x65f9c8b6514af4e5),  DEE_PCKuSIZ "o", index, count)));
	}
	void (erase)(size_t index, Dee_ssize_t count) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "erase", _Dee_HashSelectC(0x6f5916cf, 0x65f9c8b6514af4e5),  DEE_PCKuSIZ DEE_PCKdSIZ, index, count)));
	}
	void (erase)(size_t index, size_t count) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "erase", _Dee_HashSelectC(0x6f5916cf, 0x65f9c8b6514af4e5),  DEE_PCKuSIZ DEE_PCKuSIZ, index, count)));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<T> (xchitem)(DeeObject *index, DeeObject *value) {
		DeeObject *args[2];
		args[0] = index;
		args[1] = value;
		return inherit(DeeObject_CallAttrStringHash(this, "xchitem", _Dee_HashSelectC(0xc89decce, 0x16e81f00d8d95d57), 2, args));
	}
	WUNUSED NONNULL_CXX((2)) Ref<T> (xchitem)(Dee_ssize_t index, DeeObject *value) {
		return inherit(DeeObject_CallAttrStringHashf(this, "xchitem", _Dee_HashSelectC(0xc89decce, 0x16e81f00d8d95d57),  DEE_PCKdSIZ "o", index, value));
	}
	WUNUSED NONNULL_CXX((2)) Ref<T> (xchitem)(size_t index, DeeObject *value) {
		return inherit(DeeObject_CallAttrStringHashf(this, "xchitem", _Dee_HashSelectC(0xc89decce, 0x16e81f00d8d95d57),  DEE_PCKuSIZ "o", index, value));
	}
	WUNUSED Ref<T> (pop)() {
		return inherit(DeeObject_CallAttrStringHash(this, "pop", _Dee_HashSelectC(0x960361ff, 0x666fb01461b0a0eb), 0, NULL));
	}
	WUNUSED NONNULL_CXX((1)) Ref<T> (pop)(DeeObject *index) {
		DeeObject *args[1];
		args[0] = index;
		return inherit(DeeObject_CallAttrStringHash(this, "pop", _Dee_HashSelectC(0x960361ff, 0x666fb01461b0a0eb), 1, args));
	}
	WUNUSED Ref<T> (pop)(Dee_ssize_t index) {
		return inherit(DeeObject_CallAttrStringHashf(this, "pop", _Dee_HashSelectC(0x960361ff, 0x666fb01461b0a0eb),  DEE_PCKdSIZ, index));
	}
	WUNUSED Ref<T> (pop)(size_t index) {
		return inherit(DeeObject_CallAttrStringHashf(this, "pop", _Dee_HashSelectC(0x960361ff, 0x666fb01461b0a0eb),  DEE_PCKuSIZ, index));
	}
	WUNUSED Ref<T> (popfront)() {
		return inherit(DeeObject_CallAttrStringHash(this, "popfront", _Dee_HashSelectC(0x46523911, 0x22a469cc52318bba), 0, NULL));
	}
	WUNUSED Ref<T> (popback)() {
		return inherit(DeeObject_CallAttrStringHash(this, "popback", _Dee_HashSelectC(0xd84577aa, 0xb77f74a49a9cc289), 0, NULL));
	}
	NONNULL_CXX((1)) void (pushfront)(DeeObject *item) {
		DeeObject *args[1];
		args[0] = item;
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "pushfront", _Dee_HashSelectC(0xc682cfdf, 0x5933eb9a387ff882), 1, args)));
	}
	NONNULL_CXX((1)) void (pushback)(DeeObject *item) {
		DeeObject *args[1];
		args[0] = item;
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "pushback", _Dee_HashSelectC(0xad1e1509, 0x4cfafd84a12923bd), 1, args)));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (remove)(DeeObject *item) {
		DeeObject *args[1];
		args[0] = item;
		return inherit(DeeObject_CallAttrStringHash(this, "remove", _Dee_HashSelectC(0x3d2727dd, 0xe9f313a03e2051a), 1, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::bool_> (remove)(DeeObject *item, DeeObject *start) {
		DeeObject *args[2];
		args[0] = item;
		args[1] = start;
		return inherit(DeeObject_CallAttrStringHash(this, "remove", _Dee_HashSelectC(0x3d2727dd, 0xe9f313a03e2051a), 2, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3)) Ref<deemon::bool_> (remove)(DeeObject *item, DeeObject *start, DeeObject *end) {
		DeeObject *args[3];
		args[0] = item;
		args[1] = start;
		args[2] = end;
		return inherit(DeeObject_CallAttrStringHash(this, "remove", _Dee_HashSelectC(0x3d2727dd, 0xe9f313a03e2051a), 3, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3, 4)) Ref<deemon::bool_> (remove)(DeeObject *item, DeeObject *start, DeeObject *end, DeeObject *key) {
		DeeObject *args[4];
		args[0] = item;
		args[1] = start;
		args[2] = end;
		args[3] = key;
		return inherit(DeeObject_CallAttrStringHash(this, "remove", _Dee_HashSelectC(0x3d2727dd, 0xe9f313a03e2051a), 4, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::bool_> (remove)(DeeObject *item, DeeObject *start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "remove", _Dee_HashSelectC(0x3d2727dd, 0xe9f313a03e2051a), "oo" DEE_PCKdSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 2, 4)) Ref<deemon::bool_> (remove)(DeeObject *item, DeeObject *start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "remove", _Dee_HashSelectC(0x3d2727dd, 0xe9f313a03e2051a), "oo" DEE_PCKdSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::bool_> (remove)(DeeObject *item, DeeObject *start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "remove", _Dee_HashSelectC(0x3d2727dd, 0xe9f313a03e2051a), "oo" DEE_PCKuSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 2, 4)) Ref<deemon::bool_> (remove)(DeeObject *item, DeeObject *start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "remove", _Dee_HashSelectC(0x3d2727dd, 0xe9f313a03e2051a), "oo" DEE_PCKuSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (remove)(DeeObject *item, Dee_ssize_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "remove", _Dee_HashSelectC(0x3d2727dd, 0xe9f313a03e2051a), "o" DEE_PCKdSIZ, item, start));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::bool_> (remove)(DeeObject *item, Dee_ssize_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "remove", _Dee_HashSelectC(0x3d2727dd, 0xe9f313a03e2051a), "o" DEE_PCKdSIZ "o", item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 3, 4)) Ref<deemon::bool_> (remove)(DeeObject *item, Dee_ssize_t start, DeeObject *end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "remove", _Dee_HashSelectC(0x3d2727dd, 0xe9f313a03e2051a), "o" DEE_PCKdSIZ "oo", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (remove)(DeeObject *item, Dee_ssize_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "remove", _Dee_HashSelectC(0x3d2727dd, 0xe9f313a03e2051a), "o" DEE_PCKdSIZ DEE_PCKdSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::bool_> (remove)(DeeObject *item, Dee_ssize_t start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "remove", _Dee_HashSelectC(0x3d2727dd, 0xe9f313a03e2051a), "o" DEE_PCKdSIZ DEE_PCKdSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (remove)(DeeObject *item, Dee_ssize_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "remove", _Dee_HashSelectC(0x3d2727dd, 0xe9f313a03e2051a), "o" DEE_PCKdSIZ DEE_PCKuSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::bool_> (remove)(DeeObject *item, Dee_ssize_t start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "remove", _Dee_HashSelectC(0x3d2727dd, 0xe9f313a03e2051a), "o" DEE_PCKdSIZ DEE_PCKuSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (remove)(DeeObject *item, size_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "remove", _Dee_HashSelectC(0x3d2727dd, 0xe9f313a03e2051a), "o" DEE_PCKuSIZ, item, start));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::bool_> (remove)(DeeObject *item, size_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "remove", _Dee_HashSelectC(0x3d2727dd, 0xe9f313a03e2051a), "o" DEE_PCKuSIZ "o", item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 3, 4)) Ref<deemon::bool_> (remove)(DeeObject *item, size_t start, DeeObject *end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "remove", _Dee_HashSelectC(0x3d2727dd, 0xe9f313a03e2051a), "o" DEE_PCKuSIZ "oo", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (remove)(DeeObject *item, size_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "remove", _Dee_HashSelectC(0x3d2727dd, 0xe9f313a03e2051a), "o" DEE_PCKuSIZ DEE_PCKdSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::bool_> (remove)(DeeObject *item, size_t start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "remove", _Dee_HashSelectC(0x3d2727dd, 0xe9f313a03e2051a), "o" DEE_PCKuSIZ DEE_PCKdSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (remove)(DeeObject *item, size_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "remove", _Dee_HashSelectC(0x3d2727dd, 0xe9f313a03e2051a), "o" DEE_PCKuSIZ DEE_PCKuSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::bool_> (remove)(DeeObject *item, size_t start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "remove", _Dee_HashSelectC(0x3d2727dd, 0xe9f313a03e2051a), "o" DEE_PCKuSIZ DEE_PCKuSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (rremove)(DeeObject *item) {
		DeeObject *args[1];
		args[0] = item;
		return inherit(DeeObject_CallAttrStringHash(this, "rremove", _Dee_HashSelectC(0x37ef1152, 0x199975a7908f6d6), 1, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::bool_> (rremove)(DeeObject *item, DeeObject *start) {
		DeeObject *args[2];
		args[0] = item;
		args[1] = start;
		return inherit(DeeObject_CallAttrStringHash(this, "rremove", _Dee_HashSelectC(0x37ef1152, 0x199975a7908f6d6), 2, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3)) Ref<deemon::bool_> (rremove)(DeeObject *item, DeeObject *start, DeeObject *end) {
		DeeObject *args[3];
		args[0] = item;
		args[1] = start;
		args[2] = end;
		return inherit(DeeObject_CallAttrStringHash(this, "rremove", _Dee_HashSelectC(0x37ef1152, 0x199975a7908f6d6), 3, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3, 4)) Ref<deemon::bool_> (rremove)(DeeObject *item, DeeObject *start, DeeObject *end, DeeObject *key) {
		DeeObject *args[4];
		args[0] = item;
		args[1] = start;
		args[2] = end;
		args[3] = key;
		return inherit(DeeObject_CallAttrStringHash(this, "rremove", _Dee_HashSelectC(0x37ef1152, 0x199975a7908f6d6), 4, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::bool_> (rremove)(DeeObject *item, DeeObject *start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rremove", _Dee_HashSelectC(0x37ef1152, 0x199975a7908f6d6), "oo" DEE_PCKdSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 2, 4)) Ref<deemon::bool_> (rremove)(DeeObject *item, DeeObject *start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rremove", _Dee_HashSelectC(0x37ef1152, 0x199975a7908f6d6), "oo" DEE_PCKdSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::bool_> (rremove)(DeeObject *item, DeeObject *start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rremove", _Dee_HashSelectC(0x37ef1152, 0x199975a7908f6d6), "oo" DEE_PCKuSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 2, 4)) Ref<deemon::bool_> (rremove)(DeeObject *item, DeeObject *start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rremove", _Dee_HashSelectC(0x37ef1152, 0x199975a7908f6d6), "oo" DEE_PCKuSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (rremove)(DeeObject *item, Dee_ssize_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rremove", _Dee_HashSelectC(0x37ef1152, 0x199975a7908f6d6), "o" DEE_PCKdSIZ, item, start));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::bool_> (rremove)(DeeObject *item, Dee_ssize_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rremove", _Dee_HashSelectC(0x37ef1152, 0x199975a7908f6d6), "o" DEE_PCKdSIZ "o", item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 3, 4)) Ref<deemon::bool_> (rremove)(DeeObject *item, Dee_ssize_t start, DeeObject *end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rremove", _Dee_HashSelectC(0x37ef1152, 0x199975a7908f6d6), "o" DEE_PCKdSIZ "oo", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (rremove)(DeeObject *item, Dee_ssize_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rremove", _Dee_HashSelectC(0x37ef1152, 0x199975a7908f6d6), "o" DEE_PCKdSIZ DEE_PCKdSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::bool_> (rremove)(DeeObject *item, Dee_ssize_t start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rremove", _Dee_HashSelectC(0x37ef1152, 0x199975a7908f6d6), "o" DEE_PCKdSIZ DEE_PCKdSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (rremove)(DeeObject *item, Dee_ssize_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rremove", _Dee_HashSelectC(0x37ef1152, 0x199975a7908f6d6), "o" DEE_PCKdSIZ DEE_PCKuSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::bool_> (rremove)(DeeObject *item, Dee_ssize_t start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rremove", _Dee_HashSelectC(0x37ef1152, 0x199975a7908f6d6), "o" DEE_PCKdSIZ DEE_PCKuSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (rremove)(DeeObject *item, size_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rremove", _Dee_HashSelectC(0x37ef1152, 0x199975a7908f6d6), "o" DEE_PCKuSIZ, item, start));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::bool_> (rremove)(DeeObject *item, size_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rremove", _Dee_HashSelectC(0x37ef1152, 0x199975a7908f6d6), "o" DEE_PCKuSIZ "o", item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 3, 4)) Ref<deemon::bool_> (rremove)(DeeObject *item, size_t start, DeeObject *end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rremove", _Dee_HashSelectC(0x37ef1152, 0x199975a7908f6d6), "o" DEE_PCKuSIZ "oo", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (rremove)(DeeObject *item, size_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rremove", _Dee_HashSelectC(0x37ef1152, 0x199975a7908f6d6), "o" DEE_PCKuSIZ DEE_PCKdSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::bool_> (rremove)(DeeObject *item, size_t start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rremove", _Dee_HashSelectC(0x37ef1152, 0x199975a7908f6d6), "o" DEE_PCKuSIZ DEE_PCKdSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (rremove)(DeeObject *item, size_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rremove", _Dee_HashSelectC(0x37ef1152, 0x199975a7908f6d6), "o" DEE_PCKuSIZ DEE_PCKuSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::bool_> (rremove)(DeeObject *item, size_t start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rremove", _Dee_HashSelectC(0x37ef1152, 0x199975a7908f6d6), "o" DEE_PCKuSIZ DEE_PCKuSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (removeall)(DeeObject *item) {
		DeeObject *args[1];
		args[0] = item;
		return inherit(DeeObject_CallAttrStringHash(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), 1, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::int_> (removeall)(DeeObject *item, DeeObject *start) {
		DeeObject *args[2];
		args[0] = item;
		args[1] = start;
		return inherit(DeeObject_CallAttrStringHash(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), 2, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3)) Ref<deemon::int_> (removeall)(DeeObject *item, DeeObject *start, DeeObject *end) {
		DeeObject *args[3];
		args[0] = item;
		args[1] = start;
		args[2] = end;
		return inherit(DeeObject_CallAttrStringHash(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), 3, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3, 4)) Ref<deemon::int_> (removeall)(DeeObject *item, DeeObject *start, DeeObject *end, DeeObject *max) {
		DeeObject *args[4];
		args[0] = item;
		args[1] = start;
		args[2] = end;
		args[3] = max;
		return inherit(DeeObject_CallAttrStringHash(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), 4, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3, 4, 5)) Ref<deemon::int_> (removeall)(DeeObject *item, DeeObject *start, DeeObject *end, DeeObject *max, DeeObject *key) {
		DeeObject *args[5];
		args[0] = item;
		args[1] = start;
		args[2] = end;
		args[3] = max;
		args[4] = key;
		return inherit(DeeObject_CallAttrStringHash(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), 5, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3)) Ref<deemon::int_> (removeall)(DeeObject *item, DeeObject *start, DeeObject *end, Dee_ssize_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "ooo" DEE_PCKdSIZ, item, start, end, max));
	}
	WUNUSED NONNULL_CXX((1, 2, 3, 5)) Ref<deemon::int_> (removeall)(DeeObject *item, DeeObject *start, DeeObject *end, Dee_ssize_t max, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "ooo" DEE_PCKdSIZ "o", item, start, end, max, key));
	}
	WUNUSED NONNULL_CXX((1, 2, 3)) Ref<deemon::int_> (removeall)(DeeObject *item, DeeObject *start, DeeObject *end, size_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "ooo" DEE_PCKuSIZ, item, start, end, max));
	}
	WUNUSED NONNULL_CXX((1, 2, 3, 5)) Ref<deemon::int_> (removeall)(DeeObject *item, DeeObject *start, DeeObject *end, size_t max, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "ooo" DEE_PCKuSIZ "o", item, start, end, max, key));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::int_> (removeall)(DeeObject *item, DeeObject *start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "oo" DEE_PCKdSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 2, 4)) Ref<deemon::int_> (removeall)(DeeObject *item, DeeObject *start, Dee_ssize_t end, DeeObject *max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "oo" DEE_PCKdSIZ "o", item, start, end, max));
	}
	WUNUSED NONNULL_CXX((1, 2, 4, 5)) Ref<deemon::int_> (removeall)(DeeObject *item, DeeObject *start, Dee_ssize_t end, DeeObject *max, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "oo" DEE_PCKdSIZ "oo", item, start, end, max, key));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::int_> (removeall)(DeeObject *item, DeeObject *start, Dee_ssize_t end, Dee_ssize_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "oo" DEE_PCKdSIZ DEE_PCKdSIZ, item, start, end, max));
	}
	WUNUSED NONNULL_CXX((1, 2, 5)) Ref<deemon::int_> (removeall)(DeeObject *item, DeeObject *start, Dee_ssize_t end, Dee_ssize_t max, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "oo" DEE_PCKdSIZ DEE_PCKdSIZ "o", item, start, end, max, key));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::int_> (removeall)(DeeObject *item, DeeObject *start, Dee_ssize_t end, size_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "oo" DEE_PCKdSIZ DEE_PCKuSIZ, item, start, end, max));
	}
	WUNUSED NONNULL_CXX((1, 2, 5)) Ref<deemon::int_> (removeall)(DeeObject *item, DeeObject *start, Dee_ssize_t end, size_t max, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "oo" DEE_PCKdSIZ DEE_PCKuSIZ "o", item, start, end, max, key));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::int_> (removeall)(DeeObject *item, DeeObject *start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "oo" DEE_PCKuSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 2, 4)) Ref<deemon::int_> (removeall)(DeeObject *item, DeeObject *start, size_t end, DeeObject *max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "oo" DEE_PCKuSIZ "o", item, start, end, max));
	}
	WUNUSED NONNULL_CXX((1, 2, 4, 5)) Ref<deemon::int_> (removeall)(DeeObject *item, DeeObject *start, size_t end, DeeObject *max, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "oo" DEE_PCKuSIZ "oo", item, start, end, max, key));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::int_> (removeall)(DeeObject *item, DeeObject *start, size_t end, Dee_ssize_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "oo" DEE_PCKuSIZ DEE_PCKdSIZ, item, start, end, max));
	}
	WUNUSED NONNULL_CXX((1, 2, 5)) Ref<deemon::int_> (removeall)(DeeObject *item, DeeObject *start, size_t end, Dee_ssize_t max, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "oo" DEE_PCKuSIZ DEE_PCKdSIZ "o", item, start, end, max, key));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::int_> (removeall)(DeeObject *item, DeeObject *start, size_t end, size_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "oo" DEE_PCKuSIZ DEE_PCKuSIZ, item, start, end, max));
	}
	WUNUSED NONNULL_CXX((1, 2, 5)) Ref<deemon::int_> (removeall)(DeeObject *item, DeeObject *start, size_t end, size_t max, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "oo" DEE_PCKuSIZ DEE_PCKuSIZ "o", item, start, end, max, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (removeall)(DeeObject *item, Dee_ssize_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" DEE_PCKdSIZ, item, start));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::int_> (removeall)(DeeObject *item, Dee_ssize_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" DEE_PCKdSIZ "o", item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 3, 4)) Ref<deemon::int_> (removeall)(DeeObject *item, Dee_ssize_t start, DeeObject *end, DeeObject *max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" DEE_PCKdSIZ "oo", item, start, end, max));
	}
	WUNUSED NONNULL_CXX((1, 3, 4, 5)) Ref<deemon::int_> (removeall)(DeeObject *item, Dee_ssize_t start, DeeObject *end, DeeObject *max, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" DEE_PCKdSIZ "ooo", item, start, end, max, key));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::int_> (removeall)(DeeObject *item, Dee_ssize_t start, DeeObject *end, Dee_ssize_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" DEE_PCKdSIZ "o" DEE_PCKdSIZ, item, start, end, max));
	}
	WUNUSED NONNULL_CXX((1, 3, 5)) Ref<deemon::int_> (removeall)(DeeObject *item, Dee_ssize_t start, DeeObject *end, Dee_ssize_t max, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" DEE_PCKdSIZ "o" DEE_PCKdSIZ "o", item, start, end, max, key));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::int_> (removeall)(DeeObject *item, Dee_ssize_t start, DeeObject *end, size_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" DEE_PCKdSIZ "o" DEE_PCKuSIZ, item, start, end, max));
	}
	WUNUSED NONNULL_CXX((1, 3, 5)) Ref<deemon::int_> (removeall)(DeeObject *item, Dee_ssize_t start, DeeObject *end, size_t max, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" DEE_PCKdSIZ "o" DEE_PCKuSIZ "o", item, start, end, max, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (removeall)(DeeObject *item, Dee_ssize_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" DEE_PCKdSIZ DEE_PCKdSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::int_> (removeall)(DeeObject *item, Dee_ssize_t start, Dee_ssize_t end, DeeObject *max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" DEE_PCKdSIZ DEE_PCKdSIZ "o", item, start, end, max));
	}
	WUNUSED NONNULL_CXX((1, 4, 5)) Ref<deemon::int_> (removeall)(DeeObject *item, Dee_ssize_t start, Dee_ssize_t end, DeeObject *max, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" DEE_PCKdSIZ DEE_PCKdSIZ "oo", item, start, end, max, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (removeall)(DeeObject *item, Dee_ssize_t start, Dee_ssize_t end, Dee_ssize_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" DEE_PCKdSIZ DEE_PCKdSIZ DEE_PCKdSIZ, item, start, end, max));
	}
	WUNUSED NONNULL_CXX((1, 5)) Ref<deemon::int_> (removeall)(DeeObject *item, Dee_ssize_t start, Dee_ssize_t end, Dee_ssize_t max, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" DEE_PCKdSIZ DEE_PCKdSIZ DEE_PCKdSIZ "o", item, start, end, max, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (removeall)(DeeObject *item, Dee_ssize_t start, Dee_ssize_t end, size_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" DEE_PCKdSIZ DEE_PCKdSIZ DEE_PCKuSIZ, item, start, end, max));
	}
	WUNUSED NONNULL_CXX((1, 5)) Ref<deemon::int_> (removeall)(DeeObject *item, Dee_ssize_t start, Dee_ssize_t end, size_t max, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" DEE_PCKdSIZ DEE_PCKdSIZ DEE_PCKuSIZ "o", item, start, end, max, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (removeall)(DeeObject *item, Dee_ssize_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" DEE_PCKdSIZ DEE_PCKuSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::int_> (removeall)(DeeObject *item, Dee_ssize_t start, size_t end, DeeObject *max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" DEE_PCKdSIZ DEE_PCKuSIZ "o", item, start, end, max));
	}
	WUNUSED NONNULL_CXX((1, 4, 5)) Ref<deemon::int_> (removeall)(DeeObject *item, Dee_ssize_t start, size_t end, DeeObject *max, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" DEE_PCKdSIZ DEE_PCKuSIZ "oo", item, start, end, max, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (removeall)(DeeObject *item, Dee_ssize_t start, size_t end, Dee_ssize_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" DEE_PCKdSIZ DEE_PCKuSIZ DEE_PCKdSIZ, item, start, end, max));
	}
	WUNUSED NONNULL_CXX((1, 5)) Ref<deemon::int_> (removeall)(DeeObject *item, Dee_ssize_t start, size_t end, Dee_ssize_t max, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" DEE_PCKdSIZ DEE_PCKuSIZ DEE_PCKdSIZ "o", item, start, end, max, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (removeall)(DeeObject *item, Dee_ssize_t start, size_t end, size_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" DEE_PCKdSIZ DEE_PCKuSIZ DEE_PCKuSIZ, item, start, end, max));
	}
	WUNUSED NONNULL_CXX((1, 5)) Ref<deemon::int_> (removeall)(DeeObject *item, Dee_ssize_t start, size_t end, size_t max, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" DEE_PCKdSIZ DEE_PCKuSIZ DEE_PCKuSIZ "o", item, start, end, max, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (removeall)(DeeObject *item, size_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" DEE_PCKuSIZ, item, start));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::int_> (removeall)(DeeObject *item, size_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" DEE_PCKuSIZ "o", item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 3, 4)) Ref<deemon::int_> (removeall)(DeeObject *item, size_t start, DeeObject *end, DeeObject *max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" DEE_PCKuSIZ "oo", item, start, end, max));
	}
	WUNUSED NONNULL_CXX((1, 3, 4, 5)) Ref<deemon::int_> (removeall)(DeeObject *item, size_t start, DeeObject *end, DeeObject *max, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" DEE_PCKuSIZ "ooo", item, start, end, max, key));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::int_> (removeall)(DeeObject *item, size_t start, DeeObject *end, Dee_ssize_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" DEE_PCKuSIZ "o" DEE_PCKdSIZ, item, start, end, max));
	}
	WUNUSED NONNULL_CXX((1, 3, 5)) Ref<deemon::int_> (removeall)(DeeObject *item, size_t start, DeeObject *end, Dee_ssize_t max, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" DEE_PCKuSIZ "o" DEE_PCKdSIZ "o", item, start, end, max, key));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::int_> (removeall)(DeeObject *item, size_t start, DeeObject *end, size_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" DEE_PCKuSIZ "o" DEE_PCKuSIZ, item, start, end, max));
	}
	WUNUSED NONNULL_CXX((1, 3, 5)) Ref<deemon::int_> (removeall)(DeeObject *item, size_t start, DeeObject *end, size_t max, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" DEE_PCKuSIZ "o" DEE_PCKuSIZ "o", item, start, end, max, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (removeall)(DeeObject *item, size_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" DEE_PCKuSIZ DEE_PCKdSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::int_> (removeall)(DeeObject *item, size_t start, Dee_ssize_t end, DeeObject *max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" DEE_PCKuSIZ DEE_PCKdSIZ "o", item, start, end, max));
	}
	WUNUSED NONNULL_CXX((1, 4, 5)) Ref<deemon::int_> (removeall)(DeeObject *item, size_t start, Dee_ssize_t end, DeeObject *max, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" DEE_PCKuSIZ DEE_PCKdSIZ "oo", item, start, end, max, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (removeall)(DeeObject *item, size_t start, Dee_ssize_t end, Dee_ssize_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" DEE_PCKuSIZ DEE_PCKdSIZ DEE_PCKdSIZ, item, start, end, max));
	}
	WUNUSED NONNULL_CXX((1, 5)) Ref<deemon::int_> (removeall)(DeeObject *item, size_t start, Dee_ssize_t end, Dee_ssize_t max, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" DEE_PCKuSIZ DEE_PCKdSIZ DEE_PCKdSIZ "o", item, start, end, max, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (removeall)(DeeObject *item, size_t start, Dee_ssize_t end, size_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" DEE_PCKuSIZ DEE_PCKdSIZ DEE_PCKuSIZ, item, start, end, max));
	}
	WUNUSED NONNULL_CXX((1, 5)) Ref<deemon::int_> (removeall)(DeeObject *item, size_t start, Dee_ssize_t end, size_t max, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" DEE_PCKuSIZ DEE_PCKdSIZ DEE_PCKuSIZ "o", item, start, end, max, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (removeall)(DeeObject *item, size_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" DEE_PCKuSIZ DEE_PCKuSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::int_> (removeall)(DeeObject *item, size_t start, size_t end, DeeObject *max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" DEE_PCKuSIZ DEE_PCKuSIZ "o", item, start, end, max));
	}
	WUNUSED NONNULL_CXX((1, 4, 5)) Ref<deemon::int_> (removeall)(DeeObject *item, size_t start, size_t end, DeeObject *max, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" DEE_PCKuSIZ DEE_PCKuSIZ "oo", item, start, end, max, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (removeall)(DeeObject *item, size_t start, size_t end, Dee_ssize_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" DEE_PCKuSIZ DEE_PCKuSIZ DEE_PCKdSIZ, item, start, end, max));
	}
	WUNUSED NONNULL_CXX((1, 5)) Ref<deemon::int_> (removeall)(DeeObject *item, size_t start, size_t end, Dee_ssize_t max, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" DEE_PCKuSIZ DEE_PCKuSIZ DEE_PCKdSIZ "o", item, start, end, max, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (removeall)(DeeObject *item, size_t start, size_t end, size_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" DEE_PCKuSIZ DEE_PCKuSIZ DEE_PCKuSIZ, item, start, end, max));
	}
	WUNUSED NONNULL_CXX((1, 5)) Ref<deemon::int_> (removeall)(DeeObject *item, size_t start, size_t end, size_t max, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), "o" DEE_PCKuSIZ DEE_PCKuSIZ DEE_PCKuSIZ "o", item, start, end, max, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (removeif)(DeeObject *should) {
		DeeObject *args[1];
		args[0] = should;
		return inherit(DeeObject_CallAttrStringHash(this, "removeif", _Dee_HashSelectC(0x156aa732, 0x96ad85f728d8a11e), 1, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::int_> (removeif)(DeeObject *should, DeeObject *start) {
		DeeObject *args[2];
		args[0] = should;
		args[1] = start;
		return inherit(DeeObject_CallAttrStringHash(this, "removeif", _Dee_HashSelectC(0x156aa732, 0x96ad85f728d8a11e), 2, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3)) Ref<deemon::int_> (removeif)(DeeObject *should, DeeObject *start, DeeObject *end) {
		DeeObject *args[3];
		args[0] = should;
		args[1] = start;
		args[2] = end;
		return inherit(DeeObject_CallAttrStringHash(this, "removeif", _Dee_HashSelectC(0x156aa732, 0x96ad85f728d8a11e), 3, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3, 4)) Ref<deemon::int_> (removeif)(DeeObject *should, DeeObject *start, DeeObject *end, DeeObject *max) {
		DeeObject *args[4];
		args[0] = should;
		args[1] = start;
		args[2] = end;
		args[3] = max;
		return inherit(DeeObject_CallAttrStringHash(this, "removeif", _Dee_HashSelectC(0x156aa732, 0x96ad85f728d8a11e), 4, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3)) Ref<deemon::int_> (removeif)(DeeObject *should, DeeObject *start, DeeObject *end, Dee_ssize_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeif", _Dee_HashSelectC(0x156aa732, 0x96ad85f728d8a11e), "ooo" DEE_PCKdSIZ, should, start, end, max));
	}
	WUNUSED NONNULL_CXX((1, 2, 3)) Ref<deemon::int_> (removeif)(DeeObject *should, DeeObject *start, DeeObject *end, size_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeif", _Dee_HashSelectC(0x156aa732, 0x96ad85f728d8a11e), "ooo" DEE_PCKuSIZ, should, start, end, max));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::int_> (removeif)(DeeObject *should, DeeObject *start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeif", _Dee_HashSelectC(0x156aa732, 0x96ad85f728d8a11e), "oo" DEE_PCKdSIZ, should, start, end));
	}
	WUNUSED NONNULL_CXX((1, 2, 4)) Ref<deemon::int_> (removeif)(DeeObject *should, DeeObject *start, Dee_ssize_t end, DeeObject *max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeif", _Dee_HashSelectC(0x156aa732, 0x96ad85f728d8a11e), "oo" DEE_PCKdSIZ "o", should, start, end, max));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::int_> (removeif)(DeeObject *should, DeeObject *start, Dee_ssize_t end, Dee_ssize_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeif", _Dee_HashSelectC(0x156aa732, 0x96ad85f728d8a11e), "oo" DEE_PCKdSIZ DEE_PCKdSIZ, should, start, end, max));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::int_> (removeif)(DeeObject *should, DeeObject *start, Dee_ssize_t end, size_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeif", _Dee_HashSelectC(0x156aa732, 0x96ad85f728d8a11e), "oo" DEE_PCKdSIZ DEE_PCKuSIZ, should, start, end, max));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::int_> (removeif)(DeeObject *should, DeeObject *start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeif", _Dee_HashSelectC(0x156aa732, 0x96ad85f728d8a11e), "oo" DEE_PCKuSIZ, should, start, end));
	}
	WUNUSED NONNULL_CXX((1, 2, 4)) Ref<deemon::int_> (removeif)(DeeObject *should, DeeObject *start, size_t end, DeeObject *max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeif", _Dee_HashSelectC(0x156aa732, 0x96ad85f728d8a11e), "oo" DEE_PCKuSIZ "o", should, start, end, max));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::int_> (removeif)(DeeObject *should, DeeObject *start, size_t end, Dee_ssize_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeif", _Dee_HashSelectC(0x156aa732, 0x96ad85f728d8a11e), "oo" DEE_PCKuSIZ DEE_PCKdSIZ, should, start, end, max));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::int_> (removeif)(DeeObject *should, DeeObject *start, size_t end, size_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeif", _Dee_HashSelectC(0x156aa732, 0x96ad85f728d8a11e), "oo" DEE_PCKuSIZ DEE_PCKuSIZ, should, start, end, max));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (removeif)(DeeObject *should, Dee_ssize_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeif", _Dee_HashSelectC(0x156aa732, 0x96ad85f728d8a11e), "o" DEE_PCKdSIZ, should, start));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::int_> (removeif)(DeeObject *should, Dee_ssize_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeif", _Dee_HashSelectC(0x156aa732, 0x96ad85f728d8a11e), "o" DEE_PCKdSIZ "o", should, start, end));
	}
	WUNUSED NONNULL_CXX((1, 3, 4)) Ref<deemon::int_> (removeif)(DeeObject *should, Dee_ssize_t start, DeeObject *end, DeeObject *max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeif", _Dee_HashSelectC(0x156aa732, 0x96ad85f728d8a11e), "o" DEE_PCKdSIZ "oo", should, start, end, max));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::int_> (removeif)(DeeObject *should, Dee_ssize_t start, DeeObject *end, Dee_ssize_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeif", _Dee_HashSelectC(0x156aa732, 0x96ad85f728d8a11e), "o" DEE_PCKdSIZ "o" DEE_PCKdSIZ, should, start, end, max));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::int_> (removeif)(DeeObject *should, Dee_ssize_t start, DeeObject *end, size_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeif", _Dee_HashSelectC(0x156aa732, 0x96ad85f728d8a11e), "o" DEE_PCKdSIZ "o" DEE_PCKuSIZ, should, start, end, max));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (removeif)(DeeObject *should, Dee_ssize_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeif", _Dee_HashSelectC(0x156aa732, 0x96ad85f728d8a11e), "o" DEE_PCKdSIZ DEE_PCKdSIZ, should, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::int_> (removeif)(DeeObject *should, Dee_ssize_t start, Dee_ssize_t end, DeeObject *max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeif", _Dee_HashSelectC(0x156aa732, 0x96ad85f728d8a11e), "o" DEE_PCKdSIZ DEE_PCKdSIZ "o", should, start, end, max));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (removeif)(DeeObject *should, Dee_ssize_t start, Dee_ssize_t end, Dee_ssize_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeif", _Dee_HashSelectC(0x156aa732, 0x96ad85f728d8a11e), "o" DEE_PCKdSIZ DEE_PCKdSIZ DEE_PCKdSIZ, should, start, end, max));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (removeif)(DeeObject *should, Dee_ssize_t start, Dee_ssize_t end, size_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeif", _Dee_HashSelectC(0x156aa732, 0x96ad85f728d8a11e), "o" DEE_PCKdSIZ DEE_PCKdSIZ DEE_PCKuSIZ, should, start, end, max));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (removeif)(DeeObject *should, Dee_ssize_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeif", _Dee_HashSelectC(0x156aa732, 0x96ad85f728d8a11e), "o" DEE_PCKdSIZ DEE_PCKuSIZ, should, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::int_> (removeif)(DeeObject *should, Dee_ssize_t start, size_t end, DeeObject *max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeif", _Dee_HashSelectC(0x156aa732, 0x96ad85f728d8a11e), "o" DEE_PCKdSIZ DEE_PCKuSIZ "o", should, start, end, max));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (removeif)(DeeObject *should, Dee_ssize_t start, size_t end, Dee_ssize_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeif", _Dee_HashSelectC(0x156aa732, 0x96ad85f728d8a11e), "o" DEE_PCKdSIZ DEE_PCKuSIZ DEE_PCKdSIZ, should, start, end, max));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (removeif)(DeeObject *should, Dee_ssize_t start, size_t end, size_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeif", _Dee_HashSelectC(0x156aa732, 0x96ad85f728d8a11e), "o" DEE_PCKdSIZ DEE_PCKuSIZ DEE_PCKuSIZ, should, start, end, max));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (removeif)(DeeObject *should, size_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeif", _Dee_HashSelectC(0x156aa732, 0x96ad85f728d8a11e), "o" DEE_PCKuSIZ, should, start));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::int_> (removeif)(DeeObject *should, size_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeif", _Dee_HashSelectC(0x156aa732, 0x96ad85f728d8a11e), "o" DEE_PCKuSIZ "o", should, start, end));
	}
	WUNUSED NONNULL_CXX((1, 3, 4)) Ref<deemon::int_> (removeif)(DeeObject *should, size_t start, DeeObject *end, DeeObject *max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeif", _Dee_HashSelectC(0x156aa732, 0x96ad85f728d8a11e), "o" DEE_PCKuSIZ "oo", should, start, end, max));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::int_> (removeif)(DeeObject *should, size_t start, DeeObject *end, Dee_ssize_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeif", _Dee_HashSelectC(0x156aa732, 0x96ad85f728d8a11e), "o" DEE_PCKuSIZ "o" DEE_PCKdSIZ, should, start, end, max));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::int_> (removeif)(DeeObject *should, size_t start, DeeObject *end, size_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeif", _Dee_HashSelectC(0x156aa732, 0x96ad85f728d8a11e), "o" DEE_PCKuSIZ "o" DEE_PCKuSIZ, should, start, end, max));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (removeif)(DeeObject *should, size_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeif", _Dee_HashSelectC(0x156aa732, 0x96ad85f728d8a11e), "o" DEE_PCKuSIZ DEE_PCKdSIZ, should, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::int_> (removeif)(DeeObject *should, size_t start, Dee_ssize_t end, DeeObject *max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeif", _Dee_HashSelectC(0x156aa732, 0x96ad85f728d8a11e), "o" DEE_PCKuSIZ DEE_PCKdSIZ "o", should, start, end, max));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (removeif)(DeeObject *should, size_t start, Dee_ssize_t end, Dee_ssize_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeif", _Dee_HashSelectC(0x156aa732, 0x96ad85f728d8a11e), "o" DEE_PCKuSIZ DEE_PCKdSIZ DEE_PCKdSIZ, should, start, end, max));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (removeif)(DeeObject *should, size_t start, Dee_ssize_t end, size_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeif", _Dee_HashSelectC(0x156aa732, 0x96ad85f728d8a11e), "o" DEE_PCKuSIZ DEE_PCKdSIZ DEE_PCKuSIZ, should, start, end, max));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (removeif)(DeeObject *should, size_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeif", _Dee_HashSelectC(0x156aa732, 0x96ad85f728d8a11e), "o" DEE_PCKuSIZ DEE_PCKuSIZ, should, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::int_> (removeif)(DeeObject *should, size_t start, size_t end, DeeObject *max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeif", _Dee_HashSelectC(0x156aa732, 0x96ad85f728d8a11e), "o" DEE_PCKuSIZ DEE_PCKuSIZ "o", should, start, end, max));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (removeif)(DeeObject *should, size_t start, size_t end, Dee_ssize_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeif", _Dee_HashSelectC(0x156aa732, 0x96ad85f728d8a11e), "o" DEE_PCKuSIZ DEE_PCKuSIZ DEE_PCKdSIZ, should, start, end, max));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (removeif)(DeeObject *should, size_t start, size_t end, size_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeif", _Dee_HashSelectC(0x156aa732, 0x96ad85f728d8a11e), "o" DEE_PCKuSIZ DEE_PCKuSIZ DEE_PCKuSIZ, should, start, end, max));
	}
	void (clear)() {
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "clear", _Dee_HashSelectC(0x7857faae, 0x22a34b6f82b3b83c), 0, NULL)));
	}
	NONNULL_CXX((1)) void (resize)(DeeObject *size) {
		DeeObject *args[1];
		args[0] = size;
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "resize", _Dee_HashSelectC(0x36fcb308, 0x573f3d2e97212b34), 1, args)));
	}
	NONNULL_CXX((1, 2)) void (resize)(DeeObject *size, DeeObject *filler) {
		DeeObject *args[2];
		args[0] = size;
		args[1] = filler;
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "resize", _Dee_HashSelectC(0x36fcb308, 0x573f3d2e97212b34), 2, args)));
	}
	void (resize)(Dee_ssize_t size) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "resize", _Dee_HashSelectC(0x36fcb308, 0x573f3d2e97212b34),  DEE_PCKdSIZ, size)));
	}
	NONNULL_CXX((2)) void (resize)(Dee_ssize_t size, DeeObject *filler) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "resize", _Dee_HashSelectC(0x36fcb308, 0x573f3d2e97212b34),  DEE_PCKdSIZ "o", size, filler)));
	}
	void (resize)(size_t size) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "resize", _Dee_HashSelectC(0x36fcb308, 0x573f3d2e97212b34),  DEE_PCKuSIZ, size)));
	}
	NONNULL_CXX((2)) void (resize)(size_t size, DeeObject *filler) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "resize", _Dee_HashSelectC(0x36fcb308, 0x573f3d2e97212b34),  DEE_PCKuSIZ "o", size, filler)));
	}
	void (fill)() {
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "fill", _Dee_HashSelectC(0xbd501461, 0x7b3ed649c1abacf4), 0, NULL)));
	}
	NONNULL_CXX((1)) void (fill)(DeeObject *start) {
		DeeObject *args[1];
		args[0] = start;
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "fill", _Dee_HashSelectC(0xbd501461, 0x7b3ed649c1abacf4), 1, args)));
	}
	NONNULL_CXX((1, 2)) void (fill)(DeeObject *start, DeeObject *end) {
		DeeObject *args[2];
		args[0] = start;
		args[1] = end;
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "fill", _Dee_HashSelectC(0xbd501461, 0x7b3ed649c1abacf4), 2, args)));
	}
	NONNULL_CXX((1, 2, 3)) void (fill)(DeeObject *start, DeeObject *end, DeeObject *filler) {
		DeeObject *args[3];
		args[0] = start;
		args[1] = end;
		args[2] = filler;
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "fill", _Dee_HashSelectC(0xbd501461, 0x7b3ed649c1abacf4), 3, args)));
	}
	NONNULL_CXX((1)) void (fill)(DeeObject *start, Dee_ssize_t end) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "fill", _Dee_HashSelectC(0xbd501461, 0x7b3ed649c1abacf4), "o" DEE_PCKdSIZ, start, end)));
	}
	NONNULL_CXX((1, 3)) void (fill)(DeeObject *start, Dee_ssize_t end, DeeObject *filler) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "fill", _Dee_HashSelectC(0xbd501461, 0x7b3ed649c1abacf4), "o" DEE_PCKdSIZ "o", start, end, filler)));
	}
	NONNULL_CXX((1)) void (fill)(DeeObject *start, size_t end) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "fill", _Dee_HashSelectC(0xbd501461, 0x7b3ed649c1abacf4), "o" DEE_PCKuSIZ, start, end)));
	}
	NONNULL_CXX((1, 3)) void (fill)(DeeObject *start, size_t end, DeeObject *filler) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "fill", _Dee_HashSelectC(0xbd501461, 0x7b3ed649c1abacf4), "o" DEE_PCKuSIZ "o", start, end, filler)));
	}
	void (fill)(Dee_ssize_t start) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "fill", _Dee_HashSelectC(0xbd501461, 0x7b3ed649c1abacf4),  DEE_PCKdSIZ, start)));
	}
	NONNULL_CXX((2)) void (fill)(Dee_ssize_t start, DeeObject *end) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "fill", _Dee_HashSelectC(0xbd501461, 0x7b3ed649c1abacf4),  DEE_PCKdSIZ "o", start, end)));
	}
	NONNULL_CXX((2, 3)) void (fill)(Dee_ssize_t start, DeeObject *end, DeeObject *filler) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "fill", _Dee_HashSelectC(0xbd501461, 0x7b3ed649c1abacf4),  DEE_PCKdSIZ "oo", start, end, filler)));
	}
	void (fill)(Dee_ssize_t start, Dee_ssize_t end) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "fill", _Dee_HashSelectC(0xbd501461, 0x7b3ed649c1abacf4),  DEE_PCKdSIZ DEE_PCKdSIZ, start, end)));
	}
	NONNULL_CXX((3)) void (fill)(Dee_ssize_t start, Dee_ssize_t end, DeeObject *filler) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "fill", _Dee_HashSelectC(0xbd501461, 0x7b3ed649c1abacf4),  DEE_PCKdSIZ DEE_PCKdSIZ "o", start, end, filler)));
	}
	void (fill)(Dee_ssize_t start, size_t end) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "fill", _Dee_HashSelectC(0xbd501461, 0x7b3ed649c1abacf4),  DEE_PCKdSIZ DEE_PCKuSIZ, start, end)));
	}
	NONNULL_CXX((3)) void (fill)(Dee_ssize_t start, size_t end, DeeObject *filler) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "fill", _Dee_HashSelectC(0xbd501461, 0x7b3ed649c1abacf4),  DEE_PCKdSIZ DEE_PCKuSIZ "o", start, end, filler)));
	}
	void (fill)(size_t start) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "fill", _Dee_HashSelectC(0xbd501461, 0x7b3ed649c1abacf4),  DEE_PCKuSIZ, start)));
	}
	NONNULL_CXX((2)) void (fill)(size_t start, DeeObject *end) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "fill", _Dee_HashSelectC(0xbd501461, 0x7b3ed649c1abacf4),  DEE_PCKuSIZ "o", start, end)));
	}
	NONNULL_CXX((2, 3)) void (fill)(size_t start, DeeObject *end, DeeObject *filler) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "fill", _Dee_HashSelectC(0xbd501461, 0x7b3ed649c1abacf4),  DEE_PCKuSIZ "oo", start, end, filler)));
	}
	void (fill)(size_t start, Dee_ssize_t end) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "fill", _Dee_HashSelectC(0xbd501461, 0x7b3ed649c1abacf4),  DEE_PCKuSIZ DEE_PCKdSIZ, start, end)));
	}
	NONNULL_CXX((3)) void (fill)(size_t start, Dee_ssize_t end, DeeObject *filler) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "fill", _Dee_HashSelectC(0xbd501461, 0x7b3ed649c1abacf4),  DEE_PCKuSIZ DEE_PCKdSIZ "o", start, end, filler)));
	}
	void (fill)(size_t start, size_t end) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "fill", _Dee_HashSelectC(0xbd501461, 0x7b3ed649c1abacf4),  DEE_PCKuSIZ DEE_PCKuSIZ, start, end)));
	}
	NONNULL_CXX((3)) void (fill)(size_t start, size_t end, DeeObject *filler) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "fill", _Dee_HashSelectC(0xbd501461, 0x7b3ed649c1abacf4),  DEE_PCKuSIZ DEE_PCKuSIZ "o", start, end, filler)));
	}
	void (reverse)() {
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "reverse", _Dee_HashSelectC(0xd8d820f4, 0x278cbcd3a230313a), 0, NULL)));
	}
	NONNULL_CXX((1)) void (reverse)(DeeObject *start) {
		DeeObject *args[1];
		args[0] = start;
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "reverse", _Dee_HashSelectC(0xd8d820f4, 0x278cbcd3a230313a), 1, args)));
	}
	NONNULL_CXX((1, 2)) void (reverse)(DeeObject *start, DeeObject *end) {
		DeeObject *args[2];
		args[0] = start;
		args[1] = end;
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "reverse", _Dee_HashSelectC(0xd8d820f4, 0x278cbcd3a230313a), 2, args)));
	}
	NONNULL_CXX((1)) void (reverse)(DeeObject *start, Dee_ssize_t end) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "reverse", _Dee_HashSelectC(0xd8d820f4, 0x278cbcd3a230313a), "o" DEE_PCKdSIZ, start, end)));
	}
	NONNULL_CXX((1)) void (reverse)(DeeObject *start, size_t end) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "reverse", _Dee_HashSelectC(0xd8d820f4, 0x278cbcd3a230313a), "o" DEE_PCKuSIZ, start, end)));
	}
	void (reverse)(Dee_ssize_t start) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "reverse", _Dee_HashSelectC(0xd8d820f4, 0x278cbcd3a230313a),  DEE_PCKdSIZ, start)));
	}
	NONNULL_CXX((2)) void (reverse)(Dee_ssize_t start, DeeObject *end) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "reverse", _Dee_HashSelectC(0xd8d820f4, 0x278cbcd3a230313a),  DEE_PCKdSIZ "o", start, end)));
	}
	void (reverse)(Dee_ssize_t start, Dee_ssize_t end) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "reverse", _Dee_HashSelectC(0xd8d820f4, 0x278cbcd3a230313a),  DEE_PCKdSIZ DEE_PCKdSIZ, start, end)));
	}
	void (reverse)(Dee_ssize_t start, size_t end) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "reverse", _Dee_HashSelectC(0xd8d820f4, 0x278cbcd3a230313a),  DEE_PCKdSIZ DEE_PCKuSIZ, start, end)));
	}
	void (reverse)(size_t start) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "reverse", _Dee_HashSelectC(0xd8d820f4, 0x278cbcd3a230313a),  DEE_PCKuSIZ, start)));
	}
	NONNULL_CXX((2)) void (reverse)(size_t start, DeeObject *end) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "reverse", _Dee_HashSelectC(0xd8d820f4, 0x278cbcd3a230313a),  DEE_PCKuSIZ "o", start, end)));
	}
	void (reverse)(size_t start, Dee_ssize_t end) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "reverse", _Dee_HashSelectC(0xd8d820f4, 0x278cbcd3a230313a),  DEE_PCKuSIZ DEE_PCKdSIZ, start, end)));
	}
	void (reverse)(size_t start, size_t end) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "reverse", _Dee_HashSelectC(0xd8d820f4, 0x278cbcd3a230313a),  DEE_PCKuSIZ DEE_PCKuSIZ, start, end)));
	}
	void (sort)() {
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "sort", _Dee_HashSelectC(0xde7868af, 0x58835b3b7416f7f1), 0, NULL)));
	}
	NONNULL_CXX((1)) void (sort)(DeeObject *start) {
		DeeObject *args[1];
		args[0] = start;
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "sort", _Dee_HashSelectC(0xde7868af, 0x58835b3b7416f7f1), 1, args)));
	}
	NONNULL_CXX((1, 2)) void (sort)(DeeObject *start, DeeObject *end) {
		DeeObject *args[2];
		args[0] = start;
		args[1] = end;
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "sort", _Dee_HashSelectC(0xde7868af, 0x58835b3b7416f7f1), 2, args)));
	}
	NONNULL_CXX((1, 2, 3)) void (sort)(DeeObject *start, DeeObject *end, DeeObject *key) {
		DeeObject *args[3];
		args[0] = start;
		args[1] = end;
		args[2] = key;
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "sort", _Dee_HashSelectC(0xde7868af, 0x58835b3b7416f7f1), 3, args)));
	}
	NONNULL_CXX((1)) void (sort)(DeeObject *start, Dee_ssize_t end) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "sort", _Dee_HashSelectC(0xde7868af, 0x58835b3b7416f7f1), "o" DEE_PCKdSIZ, start, end)));
	}
	NONNULL_CXX((1, 3)) void (sort)(DeeObject *start, Dee_ssize_t end, DeeObject *key) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "sort", _Dee_HashSelectC(0xde7868af, 0x58835b3b7416f7f1), "o" DEE_PCKdSIZ "o", start, end, key)));
	}
	NONNULL_CXX((1)) void (sort)(DeeObject *start, size_t end) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "sort", _Dee_HashSelectC(0xde7868af, 0x58835b3b7416f7f1), "o" DEE_PCKuSIZ, start, end)));
	}
	NONNULL_CXX((1, 3)) void (sort)(DeeObject *start, size_t end, DeeObject *key) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "sort", _Dee_HashSelectC(0xde7868af, 0x58835b3b7416f7f1), "o" DEE_PCKuSIZ "o", start, end, key)));
	}
	void (sort)(Dee_ssize_t start) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "sort", _Dee_HashSelectC(0xde7868af, 0x58835b3b7416f7f1),  DEE_PCKdSIZ, start)));
	}
	NONNULL_CXX((2)) void (sort)(Dee_ssize_t start, DeeObject *end) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "sort", _Dee_HashSelectC(0xde7868af, 0x58835b3b7416f7f1),  DEE_PCKdSIZ "o", start, end)));
	}
	NONNULL_CXX((2, 3)) void (sort)(Dee_ssize_t start, DeeObject *end, DeeObject *key) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "sort", _Dee_HashSelectC(0xde7868af, 0x58835b3b7416f7f1),  DEE_PCKdSIZ "oo", start, end, key)));
	}
	void (sort)(Dee_ssize_t start, Dee_ssize_t end) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "sort", _Dee_HashSelectC(0xde7868af, 0x58835b3b7416f7f1),  DEE_PCKdSIZ DEE_PCKdSIZ, start, end)));
	}
	NONNULL_CXX((3)) void (sort)(Dee_ssize_t start, Dee_ssize_t end, DeeObject *key) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "sort", _Dee_HashSelectC(0xde7868af, 0x58835b3b7416f7f1),  DEE_PCKdSIZ DEE_PCKdSIZ "o", start, end, key)));
	}
	void (sort)(Dee_ssize_t start, size_t end) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "sort", _Dee_HashSelectC(0xde7868af, 0x58835b3b7416f7f1),  DEE_PCKdSIZ DEE_PCKuSIZ, start, end)));
	}
	NONNULL_CXX((3)) void (sort)(Dee_ssize_t start, size_t end, DeeObject *key) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "sort", _Dee_HashSelectC(0xde7868af, 0x58835b3b7416f7f1),  DEE_PCKdSIZ DEE_PCKuSIZ "o", start, end, key)));
	}
	void (sort)(size_t start) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "sort", _Dee_HashSelectC(0xde7868af, 0x58835b3b7416f7f1),  DEE_PCKuSIZ, start)));
	}
	NONNULL_CXX((2)) void (sort)(size_t start, DeeObject *end) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "sort", _Dee_HashSelectC(0xde7868af, 0x58835b3b7416f7f1),  DEE_PCKuSIZ "o", start, end)));
	}
	NONNULL_CXX((2, 3)) void (sort)(size_t start, DeeObject *end, DeeObject *key) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "sort", _Dee_HashSelectC(0xde7868af, 0x58835b3b7416f7f1),  DEE_PCKuSIZ "oo", start, end, key)));
	}
	void (sort)(size_t start, Dee_ssize_t end) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "sort", _Dee_HashSelectC(0xde7868af, 0x58835b3b7416f7f1),  DEE_PCKuSIZ DEE_PCKdSIZ, start, end)));
	}
	NONNULL_CXX((3)) void (sort)(size_t start, Dee_ssize_t end, DeeObject *key) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "sort", _Dee_HashSelectC(0xde7868af, 0x58835b3b7416f7f1),  DEE_PCKuSIZ DEE_PCKdSIZ "o", start, end, key)));
	}
	void (sort)(size_t start, size_t end) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "sort", _Dee_HashSelectC(0xde7868af, 0x58835b3b7416f7f1),  DEE_PCKuSIZ DEE_PCKuSIZ, start, end)));
	}
	NONNULL_CXX((3)) void (sort)(size_t start, size_t end, DeeObject *key) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "sort", _Dee_HashSelectC(0xde7868af, 0x58835b3b7416f7f1),  DEE_PCKuSIZ DEE_PCKuSIZ "o", start, end, key)));
	}
	WUNUSED NONNULL_CXX((1)) Ref<T> (bfind)(DeeObject *item) {
		DeeObject *args[1];
		args[0] = item;
		return inherit(DeeObject_CallAttrStringHash(this, "bfind", _Dee_HashSelectC(0xdb39cc6c, 0x5ec07aef149314c7), 1, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<T> (bfind)(DeeObject *item, DeeObject *start) {
		DeeObject *args[2];
		args[0] = item;
		args[1] = start;
		return inherit(DeeObject_CallAttrStringHash(this, "bfind", _Dee_HashSelectC(0xdb39cc6c, 0x5ec07aef149314c7), 2, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3)) Ref<T> (bfind)(DeeObject *item, DeeObject *start, DeeObject *end) {
		DeeObject *args[3];
		args[0] = item;
		args[1] = start;
		args[2] = end;
		return inherit(DeeObject_CallAttrStringHash(this, "bfind", _Dee_HashSelectC(0xdb39cc6c, 0x5ec07aef149314c7), 3, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3, 4)) Ref<T> (bfind)(DeeObject *item, DeeObject *start, DeeObject *end, DeeObject *key) {
		DeeObject *args[4];
		args[0] = item;
		args[1] = start;
		args[2] = end;
		args[3] = key;
		return inherit(DeeObject_CallAttrStringHash(this, "bfind", _Dee_HashSelectC(0xdb39cc6c, 0x5ec07aef149314c7), 4, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<T> (bfind)(DeeObject *item, DeeObject *start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bfind", _Dee_HashSelectC(0xdb39cc6c, 0x5ec07aef149314c7), "oo" DEE_PCKdSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 2, 4)) Ref<T> (bfind)(DeeObject *item, DeeObject *start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bfind", _Dee_HashSelectC(0xdb39cc6c, 0x5ec07aef149314c7), "oo" DEE_PCKdSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<T> (bfind)(DeeObject *item, DeeObject *start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bfind", _Dee_HashSelectC(0xdb39cc6c, 0x5ec07aef149314c7), "oo" DEE_PCKuSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 2, 4)) Ref<T> (bfind)(DeeObject *item, DeeObject *start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bfind", _Dee_HashSelectC(0xdb39cc6c, 0x5ec07aef149314c7), "oo" DEE_PCKuSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<T> (bfind)(DeeObject *item, Dee_ssize_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bfind", _Dee_HashSelectC(0xdb39cc6c, 0x5ec07aef149314c7), "o" DEE_PCKdSIZ, item, start));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<T> (bfind)(DeeObject *item, Dee_ssize_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bfind", _Dee_HashSelectC(0xdb39cc6c, 0x5ec07aef149314c7), "o" DEE_PCKdSIZ "o", item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 3, 4)) Ref<T> (bfind)(DeeObject *item, Dee_ssize_t start, DeeObject *end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bfind", _Dee_HashSelectC(0xdb39cc6c, 0x5ec07aef149314c7), "o" DEE_PCKdSIZ "oo", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<T> (bfind)(DeeObject *item, Dee_ssize_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bfind", _Dee_HashSelectC(0xdb39cc6c, 0x5ec07aef149314c7), "o" DEE_PCKdSIZ DEE_PCKdSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<T> (bfind)(DeeObject *item, Dee_ssize_t start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bfind", _Dee_HashSelectC(0xdb39cc6c, 0x5ec07aef149314c7), "o" DEE_PCKdSIZ DEE_PCKdSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<T> (bfind)(DeeObject *item, Dee_ssize_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bfind", _Dee_HashSelectC(0xdb39cc6c, 0x5ec07aef149314c7), "o" DEE_PCKdSIZ DEE_PCKuSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<T> (bfind)(DeeObject *item, Dee_ssize_t start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bfind", _Dee_HashSelectC(0xdb39cc6c, 0x5ec07aef149314c7), "o" DEE_PCKdSIZ DEE_PCKuSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<T> (bfind)(DeeObject *item, size_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bfind", _Dee_HashSelectC(0xdb39cc6c, 0x5ec07aef149314c7), "o" DEE_PCKuSIZ, item, start));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<T> (bfind)(DeeObject *item, size_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bfind", _Dee_HashSelectC(0xdb39cc6c, 0x5ec07aef149314c7), "o" DEE_PCKuSIZ "o", item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 3, 4)) Ref<T> (bfind)(DeeObject *item, size_t start, DeeObject *end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bfind", _Dee_HashSelectC(0xdb39cc6c, 0x5ec07aef149314c7), "o" DEE_PCKuSIZ "oo", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<T> (bfind)(DeeObject *item, size_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bfind", _Dee_HashSelectC(0xdb39cc6c, 0x5ec07aef149314c7), "o" DEE_PCKuSIZ DEE_PCKdSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<T> (bfind)(DeeObject *item, size_t start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bfind", _Dee_HashSelectC(0xdb39cc6c, 0x5ec07aef149314c7), "o" DEE_PCKuSIZ DEE_PCKdSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<T> (bfind)(DeeObject *item, size_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bfind", _Dee_HashSelectC(0xdb39cc6c, 0x5ec07aef149314c7), "o" DEE_PCKuSIZ DEE_PCKuSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<T> (bfind)(DeeObject *item, size_t start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bfind", _Dee_HashSelectC(0xdb39cc6c, 0x5ec07aef149314c7), "o" DEE_PCKuSIZ DEE_PCKuSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (bcontains)(DeeObject *item) {
		DeeObject *args[1];
		args[0] = item;
		return inherit(DeeObject_CallAttrStringHash(this, "bcontains", _Dee_HashSelectC(0x2a030b92, 0x545f84c22975fbfc), 1, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::bool_> (bcontains)(DeeObject *item, DeeObject *start) {
		DeeObject *args[2];
		args[0] = item;
		args[1] = start;
		return inherit(DeeObject_CallAttrStringHash(this, "bcontains", _Dee_HashSelectC(0x2a030b92, 0x545f84c22975fbfc), 2, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3)) Ref<deemon::bool_> (bcontains)(DeeObject *item, DeeObject *start, DeeObject *end) {
		DeeObject *args[3];
		args[0] = item;
		args[1] = start;
		args[2] = end;
		return inherit(DeeObject_CallAttrStringHash(this, "bcontains", _Dee_HashSelectC(0x2a030b92, 0x545f84c22975fbfc), 3, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3, 4)) Ref<deemon::bool_> (bcontains)(DeeObject *item, DeeObject *start, DeeObject *end, DeeObject *key) {
		DeeObject *args[4];
		args[0] = item;
		args[1] = start;
		args[2] = end;
		args[3] = key;
		return inherit(DeeObject_CallAttrStringHash(this, "bcontains", _Dee_HashSelectC(0x2a030b92, 0x545f84c22975fbfc), 4, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::bool_> (bcontains)(DeeObject *item, DeeObject *start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bcontains", _Dee_HashSelectC(0x2a030b92, 0x545f84c22975fbfc), "oo" DEE_PCKdSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 2, 4)) Ref<deemon::bool_> (bcontains)(DeeObject *item, DeeObject *start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bcontains", _Dee_HashSelectC(0x2a030b92, 0x545f84c22975fbfc), "oo" DEE_PCKdSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::bool_> (bcontains)(DeeObject *item, DeeObject *start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bcontains", _Dee_HashSelectC(0x2a030b92, 0x545f84c22975fbfc), "oo" DEE_PCKuSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 2, 4)) Ref<deemon::bool_> (bcontains)(DeeObject *item, DeeObject *start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bcontains", _Dee_HashSelectC(0x2a030b92, 0x545f84c22975fbfc), "oo" DEE_PCKuSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (bcontains)(DeeObject *item, Dee_ssize_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bcontains", _Dee_HashSelectC(0x2a030b92, 0x545f84c22975fbfc), "o" DEE_PCKdSIZ, item, start));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::bool_> (bcontains)(DeeObject *item, Dee_ssize_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bcontains", _Dee_HashSelectC(0x2a030b92, 0x545f84c22975fbfc), "o" DEE_PCKdSIZ "o", item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 3, 4)) Ref<deemon::bool_> (bcontains)(DeeObject *item, Dee_ssize_t start, DeeObject *end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bcontains", _Dee_HashSelectC(0x2a030b92, 0x545f84c22975fbfc), "o" DEE_PCKdSIZ "oo", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (bcontains)(DeeObject *item, Dee_ssize_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bcontains", _Dee_HashSelectC(0x2a030b92, 0x545f84c22975fbfc), "o" DEE_PCKdSIZ DEE_PCKdSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::bool_> (bcontains)(DeeObject *item, Dee_ssize_t start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bcontains", _Dee_HashSelectC(0x2a030b92, 0x545f84c22975fbfc), "o" DEE_PCKdSIZ DEE_PCKdSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (bcontains)(DeeObject *item, Dee_ssize_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bcontains", _Dee_HashSelectC(0x2a030b92, 0x545f84c22975fbfc), "o" DEE_PCKdSIZ DEE_PCKuSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::bool_> (bcontains)(DeeObject *item, Dee_ssize_t start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bcontains", _Dee_HashSelectC(0x2a030b92, 0x545f84c22975fbfc), "o" DEE_PCKdSIZ DEE_PCKuSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (bcontains)(DeeObject *item, size_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bcontains", _Dee_HashSelectC(0x2a030b92, 0x545f84c22975fbfc), "o" DEE_PCKuSIZ, item, start));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::bool_> (bcontains)(DeeObject *item, size_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bcontains", _Dee_HashSelectC(0x2a030b92, 0x545f84c22975fbfc), "o" DEE_PCKuSIZ "o", item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 3, 4)) Ref<deemon::bool_> (bcontains)(DeeObject *item, size_t start, DeeObject *end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bcontains", _Dee_HashSelectC(0x2a030b92, 0x545f84c22975fbfc), "o" DEE_PCKuSIZ "oo", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (bcontains)(DeeObject *item, size_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bcontains", _Dee_HashSelectC(0x2a030b92, 0x545f84c22975fbfc), "o" DEE_PCKuSIZ DEE_PCKdSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::bool_> (bcontains)(DeeObject *item, size_t start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bcontains", _Dee_HashSelectC(0x2a030b92, 0x545f84c22975fbfc), "o" DEE_PCKuSIZ DEE_PCKdSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (bcontains)(DeeObject *item, size_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bcontains", _Dee_HashSelectC(0x2a030b92, 0x545f84c22975fbfc), "o" DEE_PCKuSIZ DEE_PCKuSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::bool_> (bcontains)(DeeObject *item, size_t start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bcontains", _Dee_HashSelectC(0x2a030b92, 0x545f84c22975fbfc), "o" DEE_PCKuSIZ DEE_PCKuSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (bindex)(DeeObject *item) {
		DeeObject *args[1];
		args[0] = item;
		return inherit(DeeObject_CallAttrStringHash(this, "bindex", _Dee_HashSelectC(0x2c8be478, 0xffcf13ce8c346f38), 1, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::int_> (bindex)(DeeObject *item, DeeObject *start) {
		DeeObject *args[2];
		args[0] = item;
		args[1] = start;
		return inherit(DeeObject_CallAttrStringHash(this, "bindex", _Dee_HashSelectC(0x2c8be478, 0xffcf13ce8c346f38), 2, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3)) Ref<deemon::int_> (bindex)(DeeObject *item, DeeObject *start, DeeObject *end) {
		DeeObject *args[3];
		args[0] = item;
		args[1] = start;
		args[2] = end;
		return inherit(DeeObject_CallAttrStringHash(this, "bindex", _Dee_HashSelectC(0x2c8be478, 0xffcf13ce8c346f38), 3, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3, 4)) Ref<deemon::int_> (bindex)(DeeObject *item, DeeObject *start, DeeObject *end, DeeObject *key) {
		DeeObject *args[4];
		args[0] = item;
		args[1] = start;
		args[2] = end;
		args[3] = key;
		return inherit(DeeObject_CallAttrStringHash(this, "bindex", _Dee_HashSelectC(0x2c8be478, 0xffcf13ce8c346f38), 4, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::int_> (bindex)(DeeObject *item, DeeObject *start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bindex", _Dee_HashSelectC(0x2c8be478, 0xffcf13ce8c346f38), "oo" DEE_PCKdSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 2, 4)) Ref<deemon::int_> (bindex)(DeeObject *item, DeeObject *start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bindex", _Dee_HashSelectC(0x2c8be478, 0xffcf13ce8c346f38), "oo" DEE_PCKdSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::int_> (bindex)(DeeObject *item, DeeObject *start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bindex", _Dee_HashSelectC(0x2c8be478, 0xffcf13ce8c346f38), "oo" DEE_PCKuSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 2, 4)) Ref<deemon::int_> (bindex)(DeeObject *item, DeeObject *start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bindex", _Dee_HashSelectC(0x2c8be478, 0xffcf13ce8c346f38), "oo" DEE_PCKuSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (bindex)(DeeObject *item, Dee_ssize_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bindex", _Dee_HashSelectC(0x2c8be478, 0xffcf13ce8c346f38), "o" DEE_PCKdSIZ, item, start));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::int_> (bindex)(DeeObject *item, Dee_ssize_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bindex", _Dee_HashSelectC(0x2c8be478, 0xffcf13ce8c346f38), "o" DEE_PCKdSIZ "o", item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 3, 4)) Ref<deemon::int_> (bindex)(DeeObject *item, Dee_ssize_t start, DeeObject *end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bindex", _Dee_HashSelectC(0x2c8be478, 0xffcf13ce8c346f38), "o" DEE_PCKdSIZ "oo", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (bindex)(DeeObject *item, Dee_ssize_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bindex", _Dee_HashSelectC(0x2c8be478, 0xffcf13ce8c346f38), "o" DEE_PCKdSIZ DEE_PCKdSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::int_> (bindex)(DeeObject *item, Dee_ssize_t start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bindex", _Dee_HashSelectC(0x2c8be478, 0xffcf13ce8c346f38), "o" DEE_PCKdSIZ DEE_PCKdSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (bindex)(DeeObject *item, Dee_ssize_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bindex", _Dee_HashSelectC(0x2c8be478, 0xffcf13ce8c346f38), "o" DEE_PCKdSIZ DEE_PCKuSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::int_> (bindex)(DeeObject *item, Dee_ssize_t start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bindex", _Dee_HashSelectC(0x2c8be478, 0xffcf13ce8c346f38), "o" DEE_PCKdSIZ DEE_PCKuSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (bindex)(DeeObject *item, size_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bindex", _Dee_HashSelectC(0x2c8be478, 0xffcf13ce8c346f38), "o" DEE_PCKuSIZ, item, start));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::int_> (bindex)(DeeObject *item, size_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bindex", _Dee_HashSelectC(0x2c8be478, 0xffcf13ce8c346f38), "o" DEE_PCKuSIZ "o", item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 3, 4)) Ref<deemon::int_> (bindex)(DeeObject *item, size_t start, DeeObject *end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bindex", _Dee_HashSelectC(0x2c8be478, 0xffcf13ce8c346f38), "o" DEE_PCKuSIZ "oo", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (bindex)(DeeObject *item, size_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bindex", _Dee_HashSelectC(0x2c8be478, 0xffcf13ce8c346f38), "o" DEE_PCKuSIZ DEE_PCKdSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::int_> (bindex)(DeeObject *item, size_t start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bindex", _Dee_HashSelectC(0x2c8be478, 0xffcf13ce8c346f38), "o" DEE_PCKuSIZ DEE_PCKdSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (bindex)(DeeObject *item, size_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bindex", _Dee_HashSelectC(0x2c8be478, 0xffcf13ce8c346f38), "o" DEE_PCKuSIZ DEE_PCKuSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::int_> (bindex)(DeeObject *item, size_t start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bindex", _Dee_HashSelectC(0x2c8be478, 0xffcf13ce8c346f38), "o" DEE_PCKuSIZ DEE_PCKuSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (bposition)(DeeObject *item) {
		DeeObject *args[1];
		args[0] = item;
		return inherit(DeeObject_CallAttrStringHash(this, "bposition", _Dee_HashSelectC(0xba99f013, 0xc8f6389c9f293cb2), 1, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::int_> (bposition)(DeeObject *item, DeeObject *start) {
		DeeObject *args[2];
		args[0] = item;
		args[1] = start;
		return inherit(DeeObject_CallAttrStringHash(this, "bposition", _Dee_HashSelectC(0xba99f013, 0xc8f6389c9f293cb2), 2, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3)) Ref<deemon::int_> (bposition)(DeeObject *item, DeeObject *start, DeeObject *end) {
		DeeObject *args[3];
		args[0] = item;
		args[1] = start;
		args[2] = end;
		return inherit(DeeObject_CallAttrStringHash(this, "bposition", _Dee_HashSelectC(0xba99f013, 0xc8f6389c9f293cb2), 3, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3, 4)) Ref<deemon::int_> (bposition)(DeeObject *item, DeeObject *start, DeeObject *end, DeeObject *key) {
		DeeObject *args[4];
		args[0] = item;
		args[1] = start;
		args[2] = end;
		args[3] = key;
		return inherit(DeeObject_CallAttrStringHash(this, "bposition", _Dee_HashSelectC(0xba99f013, 0xc8f6389c9f293cb2), 4, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::int_> (bposition)(DeeObject *item, DeeObject *start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bposition", _Dee_HashSelectC(0xba99f013, 0xc8f6389c9f293cb2), "oo" DEE_PCKdSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 2, 4)) Ref<deemon::int_> (bposition)(DeeObject *item, DeeObject *start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bposition", _Dee_HashSelectC(0xba99f013, 0xc8f6389c9f293cb2), "oo" DEE_PCKdSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::int_> (bposition)(DeeObject *item, DeeObject *start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bposition", _Dee_HashSelectC(0xba99f013, 0xc8f6389c9f293cb2), "oo" DEE_PCKuSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 2, 4)) Ref<deemon::int_> (bposition)(DeeObject *item, DeeObject *start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bposition", _Dee_HashSelectC(0xba99f013, 0xc8f6389c9f293cb2), "oo" DEE_PCKuSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (bposition)(DeeObject *item, Dee_ssize_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bposition", _Dee_HashSelectC(0xba99f013, 0xc8f6389c9f293cb2), "o" DEE_PCKdSIZ, item, start));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::int_> (bposition)(DeeObject *item, Dee_ssize_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bposition", _Dee_HashSelectC(0xba99f013, 0xc8f6389c9f293cb2), "o" DEE_PCKdSIZ "o", item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 3, 4)) Ref<deemon::int_> (bposition)(DeeObject *item, Dee_ssize_t start, DeeObject *end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bposition", _Dee_HashSelectC(0xba99f013, 0xc8f6389c9f293cb2), "o" DEE_PCKdSIZ "oo", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (bposition)(DeeObject *item, Dee_ssize_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bposition", _Dee_HashSelectC(0xba99f013, 0xc8f6389c9f293cb2), "o" DEE_PCKdSIZ DEE_PCKdSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::int_> (bposition)(DeeObject *item, Dee_ssize_t start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bposition", _Dee_HashSelectC(0xba99f013, 0xc8f6389c9f293cb2), "o" DEE_PCKdSIZ DEE_PCKdSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (bposition)(DeeObject *item, Dee_ssize_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bposition", _Dee_HashSelectC(0xba99f013, 0xc8f6389c9f293cb2), "o" DEE_PCKdSIZ DEE_PCKuSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::int_> (bposition)(DeeObject *item, Dee_ssize_t start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bposition", _Dee_HashSelectC(0xba99f013, 0xc8f6389c9f293cb2), "o" DEE_PCKdSIZ DEE_PCKuSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (bposition)(DeeObject *item, size_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bposition", _Dee_HashSelectC(0xba99f013, 0xc8f6389c9f293cb2), "o" DEE_PCKuSIZ, item, start));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::int_> (bposition)(DeeObject *item, size_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bposition", _Dee_HashSelectC(0xba99f013, 0xc8f6389c9f293cb2), "o" DEE_PCKuSIZ "o", item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 3, 4)) Ref<deemon::int_> (bposition)(DeeObject *item, size_t start, DeeObject *end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bposition", _Dee_HashSelectC(0xba99f013, 0xc8f6389c9f293cb2), "o" DEE_PCKuSIZ "oo", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (bposition)(DeeObject *item, size_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bposition", _Dee_HashSelectC(0xba99f013, 0xc8f6389c9f293cb2), "o" DEE_PCKuSIZ DEE_PCKdSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::int_> (bposition)(DeeObject *item, size_t start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bposition", _Dee_HashSelectC(0xba99f013, 0xc8f6389c9f293cb2), "o" DEE_PCKuSIZ DEE_PCKdSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (bposition)(DeeObject *item, size_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bposition", _Dee_HashSelectC(0xba99f013, 0xc8f6389c9f293cb2), "o" DEE_PCKuSIZ DEE_PCKuSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::int_> (bposition)(DeeObject *item, size_t start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bposition", _Dee_HashSelectC(0xba99f013, 0xc8f6389c9f293cb2), "o" DEE_PCKuSIZ DEE_PCKuSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<_AbstractTuple<deemon::int_, deemon::int_> > (brange)(DeeObject *item) {
		DeeObject *args[1];
		args[0] = item;
		return inherit(DeeObject_CallAttrStringHash(this, "brange", _Dee_HashSelectC(0xb132222e, 0xfed8bb16d0ac0dd2), 1, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<_AbstractTuple<deemon::int_, deemon::int_> > (brange)(DeeObject *item, DeeObject *start) {
		DeeObject *args[2];
		args[0] = item;
		args[1] = start;
		return inherit(DeeObject_CallAttrStringHash(this, "brange", _Dee_HashSelectC(0xb132222e, 0xfed8bb16d0ac0dd2), 2, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3)) Ref<_AbstractTuple<deemon::int_, deemon::int_> > (brange)(DeeObject *item, DeeObject *start, DeeObject *end) {
		DeeObject *args[3];
		args[0] = item;
		args[1] = start;
		args[2] = end;
		return inherit(DeeObject_CallAttrStringHash(this, "brange", _Dee_HashSelectC(0xb132222e, 0xfed8bb16d0ac0dd2), 3, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3, 4)) Ref<_AbstractTuple<deemon::int_, deemon::int_> > (brange)(DeeObject *item, DeeObject *start, DeeObject *end, DeeObject *key) {
		DeeObject *args[4];
		args[0] = item;
		args[1] = start;
		args[2] = end;
		args[3] = key;
		return inherit(DeeObject_CallAttrStringHash(this, "brange", _Dee_HashSelectC(0xb132222e, 0xfed8bb16d0ac0dd2), 4, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<_AbstractTuple<deemon::int_, deemon::int_> > (brange)(DeeObject *item, DeeObject *start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "brange", _Dee_HashSelectC(0xb132222e, 0xfed8bb16d0ac0dd2), "oo" DEE_PCKdSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 2, 4)) Ref<_AbstractTuple<deemon::int_, deemon::int_> > (brange)(DeeObject *item, DeeObject *start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "brange", _Dee_HashSelectC(0xb132222e, 0xfed8bb16d0ac0dd2), "oo" DEE_PCKdSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<_AbstractTuple<deemon::int_, deemon::int_> > (brange)(DeeObject *item, DeeObject *start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "brange", _Dee_HashSelectC(0xb132222e, 0xfed8bb16d0ac0dd2), "oo" DEE_PCKuSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 2, 4)) Ref<_AbstractTuple<deemon::int_, deemon::int_> > (brange)(DeeObject *item, DeeObject *start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "brange", _Dee_HashSelectC(0xb132222e, 0xfed8bb16d0ac0dd2), "oo" DEE_PCKuSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<_AbstractTuple<deemon::int_, deemon::int_> > (brange)(DeeObject *item, Dee_ssize_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "brange", _Dee_HashSelectC(0xb132222e, 0xfed8bb16d0ac0dd2), "o" DEE_PCKdSIZ, item, start));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<_AbstractTuple<deemon::int_, deemon::int_> > (brange)(DeeObject *item, Dee_ssize_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "brange", _Dee_HashSelectC(0xb132222e, 0xfed8bb16d0ac0dd2), "o" DEE_PCKdSIZ "o", item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 3, 4)) Ref<_AbstractTuple<deemon::int_, deemon::int_> > (brange)(DeeObject *item, Dee_ssize_t start, DeeObject *end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "brange", _Dee_HashSelectC(0xb132222e, 0xfed8bb16d0ac0dd2), "o" DEE_PCKdSIZ "oo", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<_AbstractTuple<deemon::int_, deemon::int_> > (brange)(DeeObject *item, Dee_ssize_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "brange", _Dee_HashSelectC(0xb132222e, 0xfed8bb16d0ac0dd2), "o" DEE_PCKdSIZ DEE_PCKdSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<_AbstractTuple<deemon::int_, deemon::int_> > (brange)(DeeObject *item, Dee_ssize_t start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "brange", _Dee_HashSelectC(0xb132222e, 0xfed8bb16d0ac0dd2), "o" DEE_PCKdSIZ DEE_PCKdSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<_AbstractTuple<deemon::int_, deemon::int_> > (brange)(DeeObject *item, Dee_ssize_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "brange", _Dee_HashSelectC(0xb132222e, 0xfed8bb16d0ac0dd2), "o" DEE_PCKdSIZ DEE_PCKuSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<_AbstractTuple<deemon::int_, deemon::int_> > (brange)(DeeObject *item, Dee_ssize_t start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "brange", _Dee_HashSelectC(0xb132222e, 0xfed8bb16d0ac0dd2), "o" DEE_PCKdSIZ DEE_PCKuSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<_AbstractTuple<deemon::int_, deemon::int_> > (brange)(DeeObject *item, size_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "brange", _Dee_HashSelectC(0xb132222e, 0xfed8bb16d0ac0dd2), "o" DEE_PCKuSIZ, item, start));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<_AbstractTuple<deemon::int_, deemon::int_> > (brange)(DeeObject *item, size_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "brange", _Dee_HashSelectC(0xb132222e, 0xfed8bb16d0ac0dd2), "o" DEE_PCKuSIZ "o", item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 3, 4)) Ref<_AbstractTuple<deemon::int_, deemon::int_> > (brange)(DeeObject *item, size_t start, DeeObject *end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "brange", _Dee_HashSelectC(0xb132222e, 0xfed8bb16d0ac0dd2), "o" DEE_PCKuSIZ "oo", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<_AbstractTuple<deemon::int_, deemon::int_> > (brange)(DeeObject *item, size_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "brange", _Dee_HashSelectC(0xb132222e, 0xfed8bb16d0ac0dd2), "o" DEE_PCKuSIZ DEE_PCKdSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<_AbstractTuple<deemon::int_, deemon::int_> > (brange)(DeeObject *item, size_t start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "brange", _Dee_HashSelectC(0xb132222e, 0xfed8bb16d0ac0dd2), "o" DEE_PCKuSIZ DEE_PCKdSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<_AbstractTuple<deemon::int_, deemon::int_> > (brange)(DeeObject *item, size_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "brange", _Dee_HashSelectC(0xb132222e, 0xfed8bb16d0ac0dd2), "o" DEE_PCKuSIZ DEE_PCKuSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<_AbstractTuple<deemon::int_, deemon::int_> > (brange)(DeeObject *item, size_t start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "brange", _Dee_HashSelectC(0xb132222e, 0xfed8bb16d0ac0dd2), "o" DEE_PCKuSIZ DEE_PCKuSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<T> (blocate)(DeeObject *item) {
		DeeObject *args[1];
		args[0] = item;
		return inherit(DeeObject_CallAttrStringHash(this, "blocate", _Dee_HashSelectC(0x7aa979d3, 0xbda91c237d69489e), 1, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<T> (blocate)(DeeObject *item, DeeObject *start) {
		DeeObject *args[2];
		args[0] = item;
		args[1] = start;
		return inherit(DeeObject_CallAttrStringHash(this, "blocate", _Dee_HashSelectC(0x7aa979d3, 0xbda91c237d69489e), 2, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3)) Ref<T> (blocate)(DeeObject *item, DeeObject *start, DeeObject *end) {
		DeeObject *args[3];
		args[0] = item;
		args[1] = start;
		args[2] = end;
		return inherit(DeeObject_CallAttrStringHash(this, "blocate", _Dee_HashSelectC(0x7aa979d3, 0xbda91c237d69489e), 3, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3, 4)) Ref<T> (blocate)(DeeObject *item, DeeObject *start, DeeObject *end, DeeObject *key) {
		DeeObject *args[4];
		args[0] = item;
		args[1] = start;
		args[2] = end;
		args[3] = key;
		return inherit(DeeObject_CallAttrStringHash(this, "blocate", _Dee_HashSelectC(0x7aa979d3, 0xbda91c237d69489e), 4, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3, 4, 5)) Ref<T> (blocate)(DeeObject *item, DeeObject *start, DeeObject *end, DeeObject *key, DeeObject *defl) {
		DeeObject *args[5];
		args[0] = item;
		args[1] = start;
		args[2] = end;
		args[3] = key;
		args[4] = defl;
		return inherit(DeeObject_CallAttrStringHash(this, "blocate", _Dee_HashSelectC(0x7aa979d3, 0xbda91c237d69489e), 5, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<T> (blocate)(DeeObject *item, DeeObject *start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "blocate", _Dee_HashSelectC(0x7aa979d3, 0xbda91c237d69489e), "oo" DEE_PCKdSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 2, 4)) Ref<T> (blocate)(DeeObject *item, DeeObject *start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "blocate", _Dee_HashSelectC(0x7aa979d3, 0xbda91c237d69489e), "oo" DEE_PCKdSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1, 2, 4, 5)) Ref<T> (blocate)(DeeObject *item, DeeObject *start, Dee_ssize_t end, DeeObject *key, DeeObject *defl) {
		return inherit(DeeObject_CallAttrStringHashf(this, "blocate", _Dee_HashSelectC(0x7aa979d3, 0xbda91c237d69489e), "oo" DEE_PCKdSIZ "oo", item, start, end, key, defl));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<T> (blocate)(DeeObject *item, DeeObject *start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "blocate", _Dee_HashSelectC(0x7aa979d3, 0xbda91c237d69489e), "oo" DEE_PCKuSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 2, 4)) Ref<T> (blocate)(DeeObject *item, DeeObject *start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "blocate", _Dee_HashSelectC(0x7aa979d3, 0xbda91c237d69489e), "oo" DEE_PCKuSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1, 2, 4, 5)) Ref<T> (blocate)(DeeObject *item, DeeObject *start, size_t end, DeeObject *key, DeeObject *defl) {
		return inherit(DeeObject_CallAttrStringHashf(this, "blocate", _Dee_HashSelectC(0x7aa979d3, 0xbda91c237d69489e), "oo" DEE_PCKuSIZ "oo", item, start, end, key, defl));
	}
	WUNUSED NONNULL_CXX((1)) Ref<T> (blocate)(DeeObject *item, Dee_ssize_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "blocate", _Dee_HashSelectC(0x7aa979d3, 0xbda91c237d69489e), "o" DEE_PCKdSIZ, item, start));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<T> (blocate)(DeeObject *item, Dee_ssize_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "blocate", _Dee_HashSelectC(0x7aa979d3, 0xbda91c237d69489e), "o" DEE_PCKdSIZ "o", item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 3, 4)) Ref<T> (blocate)(DeeObject *item, Dee_ssize_t start, DeeObject *end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "blocate", _Dee_HashSelectC(0x7aa979d3, 0xbda91c237d69489e), "o" DEE_PCKdSIZ "oo", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1, 3, 4, 5)) Ref<T> (blocate)(DeeObject *item, Dee_ssize_t start, DeeObject *end, DeeObject *key, DeeObject *defl) {
		return inherit(DeeObject_CallAttrStringHashf(this, "blocate", _Dee_HashSelectC(0x7aa979d3, 0xbda91c237d69489e), "o" DEE_PCKdSIZ "ooo", item, start, end, key, defl));
	}
	WUNUSED NONNULL_CXX((1)) Ref<T> (blocate)(DeeObject *item, Dee_ssize_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "blocate", _Dee_HashSelectC(0x7aa979d3, 0xbda91c237d69489e), "o" DEE_PCKdSIZ DEE_PCKdSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<T> (blocate)(DeeObject *item, Dee_ssize_t start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "blocate", _Dee_HashSelectC(0x7aa979d3, 0xbda91c237d69489e), "o" DEE_PCKdSIZ DEE_PCKdSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1, 4, 5)) Ref<T> (blocate)(DeeObject *item, Dee_ssize_t start, Dee_ssize_t end, DeeObject *key, DeeObject *defl) {
		return inherit(DeeObject_CallAttrStringHashf(this, "blocate", _Dee_HashSelectC(0x7aa979d3, 0xbda91c237d69489e), "o" DEE_PCKdSIZ DEE_PCKdSIZ "oo", item, start, end, key, defl));
	}
	WUNUSED NONNULL_CXX((1)) Ref<T> (blocate)(DeeObject *item, Dee_ssize_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "blocate", _Dee_HashSelectC(0x7aa979d3, 0xbda91c237d69489e), "o" DEE_PCKdSIZ DEE_PCKuSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<T> (blocate)(DeeObject *item, Dee_ssize_t start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "blocate", _Dee_HashSelectC(0x7aa979d3, 0xbda91c237d69489e), "o" DEE_PCKdSIZ DEE_PCKuSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1, 4, 5)) Ref<T> (blocate)(DeeObject *item, Dee_ssize_t start, size_t end, DeeObject *key, DeeObject *defl) {
		return inherit(DeeObject_CallAttrStringHashf(this, "blocate", _Dee_HashSelectC(0x7aa979d3, 0xbda91c237d69489e), "o" DEE_PCKdSIZ DEE_PCKuSIZ "oo", item, start, end, key, defl));
	}
	WUNUSED NONNULL_CXX((1)) Ref<T> (blocate)(DeeObject *item, size_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "blocate", _Dee_HashSelectC(0x7aa979d3, 0xbda91c237d69489e), "o" DEE_PCKuSIZ, item, start));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<T> (blocate)(DeeObject *item, size_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "blocate", _Dee_HashSelectC(0x7aa979d3, 0xbda91c237d69489e), "o" DEE_PCKuSIZ "o", item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 3, 4)) Ref<T> (blocate)(DeeObject *item, size_t start, DeeObject *end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "blocate", _Dee_HashSelectC(0x7aa979d3, 0xbda91c237d69489e), "o" DEE_PCKuSIZ "oo", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1, 3, 4, 5)) Ref<T> (blocate)(DeeObject *item, size_t start, DeeObject *end, DeeObject *key, DeeObject *defl) {
		return inherit(DeeObject_CallAttrStringHashf(this, "blocate", _Dee_HashSelectC(0x7aa979d3, 0xbda91c237d69489e), "o" DEE_PCKuSIZ "ooo", item, start, end, key, defl));
	}
	WUNUSED NONNULL_CXX((1)) Ref<T> (blocate)(DeeObject *item, size_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "blocate", _Dee_HashSelectC(0x7aa979d3, 0xbda91c237d69489e), "o" DEE_PCKuSIZ DEE_PCKdSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<T> (blocate)(DeeObject *item, size_t start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "blocate", _Dee_HashSelectC(0x7aa979d3, 0xbda91c237d69489e), "o" DEE_PCKuSIZ DEE_PCKdSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1, 4, 5)) Ref<T> (blocate)(DeeObject *item, size_t start, Dee_ssize_t end, DeeObject *key, DeeObject *defl) {
		return inherit(DeeObject_CallAttrStringHashf(this, "blocate", _Dee_HashSelectC(0x7aa979d3, 0xbda91c237d69489e), "o" DEE_PCKuSIZ DEE_PCKdSIZ "oo", item, start, end, key, defl));
	}
	WUNUSED NONNULL_CXX((1)) Ref<T> (blocate)(DeeObject *item, size_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "blocate", _Dee_HashSelectC(0x7aa979d3, 0xbda91c237d69489e), "o" DEE_PCKuSIZ DEE_PCKuSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<T> (blocate)(DeeObject *item, size_t start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "blocate", _Dee_HashSelectC(0x7aa979d3, 0xbda91c237d69489e), "o" DEE_PCKuSIZ DEE_PCKuSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1, 4, 5)) Ref<T> (blocate)(DeeObject *item, size_t start, size_t end, DeeObject *key, DeeObject *defl) {
		return inherit(DeeObject_CallAttrStringHashf(this, "blocate", _Dee_HashSelectC(0x7aa979d3, 0xbda91c237d69489e), "o" DEE_PCKuSIZ DEE_PCKuSIZ "oo", item, start, end, key, defl));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Sequence<T> > (blocateall)(DeeObject *item) {
		DeeObject *args[1];
		args[0] = item;
		return inherit(DeeObject_CallAttrStringHash(this, "blocateall", _Dee_HashSelectC(0x6b34ef10, 0xa9049a4c0519981c), 1, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<Sequence<T> > (blocateall)(DeeObject *item, DeeObject *start) {
		DeeObject *args[2];
		args[0] = item;
		args[1] = start;
		return inherit(DeeObject_CallAttrStringHash(this, "blocateall", _Dee_HashSelectC(0x6b34ef10, 0xa9049a4c0519981c), 2, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3)) Ref<Sequence<T> > (blocateall)(DeeObject *item, DeeObject *start, DeeObject *end) {
		DeeObject *args[3];
		args[0] = item;
		args[1] = start;
		args[2] = end;
		return inherit(DeeObject_CallAttrStringHash(this, "blocateall", _Dee_HashSelectC(0x6b34ef10, 0xa9049a4c0519981c), 3, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3, 4)) Ref<Sequence<T> > (blocateall)(DeeObject *item, DeeObject *start, DeeObject *end, DeeObject *key) {
		DeeObject *args[4];
		args[0] = item;
		args[1] = start;
		args[2] = end;
		args[3] = key;
		return inherit(DeeObject_CallAttrStringHash(this, "blocateall", _Dee_HashSelectC(0x6b34ef10, 0xa9049a4c0519981c), 4, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<Sequence<T> > (blocateall)(DeeObject *item, DeeObject *start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "blocateall", _Dee_HashSelectC(0x6b34ef10, 0xa9049a4c0519981c), "oo" DEE_PCKdSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 2, 4)) Ref<Sequence<T> > (blocateall)(DeeObject *item, DeeObject *start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "blocateall", _Dee_HashSelectC(0x6b34ef10, 0xa9049a4c0519981c), "oo" DEE_PCKdSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<Sequence<T> > (blocateall)(DeeObject *item, DeeObject *start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "blocateall", _Dee_HashSelectC(0x6b34ef10, 0xa9049a4c0519981c), "oo" DEE_PCKuSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 2, 4)) Ref<Sequence<T> > (blocateall)(DeeObject *item, DeeObject *start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "blocateall", _Dee_HashSelectC(0x6b34ef10, 0xa9049a4c0519981c), "oo" DEE_PCKuSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Sequence<T> > (blocateall)(DeeObject *item, Dee_ssize_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "blocateall", _Dee_HashSelectC(0x6b34ef10, 0xa9049a4c0519981c), "o" DEE_PCKdSIZ, item, start));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<Sequence<T> > (blocateall)(DeeObject *item, Dee_ssize_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "blocateall", _Dee_HashSelectC(0x6b34ef10, 0xa9049a4c0519981c), "o" DEE_PCKdSIZ "o", item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 3, 4)) Ref<Sequence<T> > (blocateall)(DeeObject *item, Dee_ssize_t start, DeeObject *end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "blocateall", _Dee_HashSelectC(0x6b34ef10, 0xa9049a4c0519981c), "o" DEE_PCKdSIZ "oo", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Sequence<T> > (blocateall)(DeeObject *item, Dee_ssize_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "blocateall", _Dee_HashSelectC(0x6b34ef10, 0xa9049a4c0519981c), "o" DEE_PCKdSIZ DEE_PCKdSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<Sequence<T> > (blocateall)(DeeObject *item, Dee_ssize_t start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "blocateall", _Dee_HashSelectC(0x6b34ef10, 0xa9049a4c0519981c), "o" DEE_PCKdSIZ DEE_PCKdSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Sequence<T> > (blocateall)(DeeObject *item, Dee_ssize_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "blocateall", _Dee_HashSelectC(0x6b34ef10, 0xa9049a4c0519981c), "o" DEE_PCKdSIZ DEE_PCKuSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<Sequence<T> > (blocateall)(DeeObject *item, Dee_ssize_t start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "blocateall", _Dee_HashSelectC(0x6b34ef10, 0xa9049a4c0519981c), "o" DEE_PCKdSIZ DEE_PCKuSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Sequence<T> > (blocateall)(DeeObject *item, size_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "blocateall", _Dee_HashSelectC(0x6b34ef10, 0xa9049a4c0519981c), "o" DEE_PCKuSIZ, item, start));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<Sequence<T> > (blocateall)(DeeObject *item, size_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "blocateall", _Dee_HashSelectC(0x6b34ef10, 0xa9049a4c0519981c), "o" DEE_PCKuSIZ "o", item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 3, 4)) Ref<Sequence<T> > (blocateall)(DeeObject *item, size_t start, DeeObject *end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "blocateall", _Dee_HashSelectC(0x6b34ef10, 0xa9049a4c0519981c), "o" DEE_PCKuSIZ "oo", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Sequence<T> > (blocateall)(DeeObject *item, size_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "blocateall", _Dee_HashSelectC(0x6b34ef10, 0xa9049a4c0519981c), "o" DEE_PCKuSIZ DEE_PCKdSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<Sequence<T> > (blocateall)(DeeObject *item, size_t start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "blocateall", _Dee_HashSelectC(0x6b34ef10, 0xa9049a4c0519981c), "o" DEE_PCKuSIZ DEE_PCKdSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Sequence<T> > (blocateall)(DeeObject *item, size_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "blocateall", _Dee_HashSelectC(0x6b34ef10, 0xa9049a4c0519981c), "o" DEE_PCKuSIZ DEE_PCKuSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<Sequence<T> > (blocateall)(DeeObject *item, size_t start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "blocateall", _Dee_HashSelectC(0x6b34ef10, 0xa9049a4c0519981c), "o" DEE_PCKuSIZ DEE_PCKuSIZ "o", item, start, end, key));
	}
	NONNULL_CXX((1)) void (binsert)(DeeObject *item) {
		DeeObject *args[1];
		args[0] = item;
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "binsert", _Dee_HashSelectC(0xcad2e09b, 0xdd69512251d4a3f7), 1, args)));
	}
	NONNULL_CXX((1, 2)) void (binsert)(DeeObject *item, DeeObject *start) {
		DeeObject *args[2];
		args[0] = item;
		args[1] = start;
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "binsert", _Dee_HashSelectC(0xcad2e09b, 0xdd69512251d4a3f7), 2, args)));
	}
	NONNULL_CXX((1, 2, 3)) void (binsert)(DeeObject *item, DeeObject *start, DeeObject *end) {
		DeeObject *args[3];
		args[0] = item;
		args[1] = start;
		args[2] = end;
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "binsert", _Dee_HashSelectC(0xcad2e09b, 0xdd69512251d4a3f7), 3, args)));
	}
	NONNULL_CXX((1, 2)) void (binsert)(DeeObject *item, DeeObject *start, Dee_ssize_t end) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "binsert", _Dee_HashSelectC(0xcad2e09b, 0xdd69512251d4a3f7), "oo" DEE_PCKdSIZ, item, start, end)));
	}
	NONNULL_CXX((1, 2)) void (binsert)(DeeObject *item, DeeObject *start, size_t end) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "binsert", _Dee_HashSelectC(0xcad2e09b, 0xdd69512251d4a3f7), "oo" DEE_PCKuSIZ, item, start, end)));
	}
	NONNULL_CXX((1)) void (binsert)(DeeObject *item, Dee_ssize_t start) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "binsert", _Dee_HashSelectC(0xcad2e09b, 0xdd69512251d4a3f7), "o" DEE_PCKdSIZ, item, start)));
	}
	NONNULL_CXX((1, 3)) void (binsert)(DeeObject *item, Dee_ssize_t start, DeeObject *end) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "binsert", _Dee_HashSelectC(0xcad2e09b, 0xdd69512251d4a3f7), "o" DEE_PCKdSIZ "o", item, start, end)));
	}
	NONNULL_CXX((1)) void (binsert)(DeeObject *item, Dee_ssize_t start, Dee_ssize_t end) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "binsert", _Dee_HashSelectC(0xcad2e09b, 0xdd69512251d4a3f7), "o" DEE_PCKdSIZ DEE_PCKdSIZ, item, start, end)));
	}
	NONNULL_CXX((1)) void (binsert)(DeeObject *item, Dee_ssize_t start, size_t end) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "binsert", _Dee_HashSelectC(0xcad2e09b, 0xdd69512251d4a3f7), "o" DEE_PCKdSIZ DEE_PCKuSIZ, item, start, end)));
	}
	NONNULL_CXX((1)) void (binsert)(DeeObject *item, size_t start) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "binsert", _Dee_HashSelectC(0xcad2e09b, 0xdd69512251d4a3f7), "o" DEE_PCKuSIZ, item, start)));
	}
	NONNULL_CXX((1, 3)) void (binsert)(DeeObject *item, size_t start, DeeObject *end) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "binsert", _Dee_HashSelectC(0xcad2e09b, 0xdd69512251d4a3f7), "o" DEE_PCKuSIZ "o", item, start, end)));
	}
	NONNULL_CXX((1)) void (binsert)(DeeObject *item, size_t start, Dee_ssize_t end) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "binsert", _Dee_HashSelectC(0xcad2e09b, 0xdd69512251d4a3f7), "o" DEE_PCKuSIZ DEE_PCKdSIZ, item, start, end)));
	}
	NONNULL_CXX((1)) void (binsert)(DeeObject *item, size_t start, size_t end) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "binsert", _Dee_HashSelectC(0xcad2e09b, 0xdd69512251d4a3f7), "o" DEE_PCKuSIZ DEE_PCKuSIZ, item, start, end)));
	}
	WUNUSED Ref<deemon::bool_> (__bool__)() {
		return inherit(DeeObject_CallAttrStringHash(this, "__bool__", _Dee_HashSelectC(0x1d6e29c8, 0x7d5655cb5b8aa88b), 0, NULL));
	}
	WUNUSED Ref<Iterator<T> > (__iter__)() {
		return inherit(DeeObject_CallAttrStringHash(this, "__iter__", _Dee_HashSelectC(0x4ae49b3, 0x29df7f8a609cead7), 0, NULL));
	}
	WUNUSED Ref<deemon::int_> (__size__)() {
		return inherit(DeeObject_CallAttrStringHash(this, "__size__", _Dee_HashSelectC(0x543ba3b5, 0xd416117435cce357), 0, NULL));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (__contains__)(DeeObject *item) {
		DeeObject *args[1];
		args[0] = item;
		return inherit(DeeObject_CallAttrStringHash(this, "__contains__", _Dee_HashSelectC(0x769af591, 0x80f9234f8000b556), 1, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<T> (__getitem__)(DeeObject *index) {
		DeeObject *args[1];
		args[0] = index;
		return inherit(DeeObject_CallAttrStringHash(this, "__getitem__", _Dee_HashSelectC(0x2796c7b1, 0x326672bfc335fb3d), 1, args));
	}
	WUNUSED Ref<T> (__getitem__)(Dee_ssize_t index) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__getitem__", _Dee_HashSelectC(0x2796c7b1, 0x326672bfc335fb3d),  DEE_PCKdSIZ, index));
	}
	WUNUSED Ref<T> (__getitem__)(size_t index) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__getitem__", _Dee_HashSelectC(0x2796c7b1, 0x326672bfc335fb3d),  DEE_PCKuSIZ, index));
	}
	NONNULL_CXX((1)) void (__delitem__)(DeeObject *index) {
		DeeObject *args[1];
		args[0] = index;
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "__delitem__", _Dee_HashSelectC(0x20ba3d50, 0x477c6001247177f), 1, args)));
	}
	void (__delitem__)(Dee_ssize_t index) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "__delitem__", _Dee_HashSelectC(0x20ba3d50, 0x477c6001247177f),  DEE_PCKdSIZ, index)));
	}
	void (__delitem__)(size_t index) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "__delitem__", _Dee_HashSelectC(0x20ba3d50, 0x477c6001247177f),  DEE_PCKuSIZ, index)));
	}
	NONNULL_CXX((1, 2)) void (__setitem__)(DeeObject *index, DeeObject *value) {
		DeeObject *args[2];
		args[0] = index;
		args[1] = value;
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "__setitem__", _Dee_HashSelectC(0xa12b6584, 0x4f2c202e4a8ee77a), 2, args)));
	}
	NONNULL_CXX((2)) void (__setitem__)(Dee_ssize_t index, DeeObject *value) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "__setitem__", _Dee_HashSelectC(0xa12b6584, 0x4f2c202e4a8ee77a),  DEE_PCKdSIZ "o", index, value)));
	}
	NONNULL_CXX((2)) void (__setitem__)(size_t index, DeeObject *value) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "__setitem__", _Dee_HashSelectC(0xa12b6584, 0x4f2c202e4a8ee77a),  DEE_PCKuSIZ "o", index, value)));
	}
	WUNUSED Ref<Sequence<T> > (__getrange__)() {
		return inherit(DeeObject_CallAttrStringHash(this, "__getrange__", _Dee_HashSelectC(0x7f22541, 0x53d4d4259a06a055), 0, NULL));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Sequence<T> > (__getrange__)(DeeObject *start) {
		DeeObject *args[1];
		args[0] = start;
		return inherit(DeeObject_CallAttrStringHash(this, "__getrange__", _Dee_HashSelectC(0x7f22541, 0x53d4d4259a06a055), 1, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<Sequence<T> > (__getrange__)(DeeObject *start, DeeObject *end) {
		DeeObject *args[2];
		args[0] = start;
		args[1] = end;
		return inherit(DeeObject_CallAttrStringHash(this, "__getrange__", _Dee_HashSelectC(0x7f22541, 0x53d4d4259a06a055), 2, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Sequence<T> > (__getrange__)(DeeObject *start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__getrange__", _Dee_HashSelectC(0x7f22541, 0x53d4d4259a06a055), "o" DEE_PCKdSIZ, start, end));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Sequence<T> > (__getrange__)(DeeObject *start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__getrange__", _Dee_HashSelectC(0x7f22541, 0x53d4d4259a06a055), "o" DEE_PCKuSIZ, start, end));
	}
	WUNUSED Ref<Sequence<T> > (__getrange__)(Dee_ssize_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__getrange__", _Dee_HashSelectC(0x7f22541, 0x53d4d4259a06a055),  DEE_PCKdSIZ, start));
	}
	WUNUSED NONNULL_CXX((2)) Ref<Sequence<T> > (__getrange__)(Dee_ssize_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__getrange__", _Dee_HashSelectC(0x7f22541, 0x53d4d4259a06a055),  DEE_PCKdSIZ "o", start, end));
	}
	WUNUSED Ref<Sequence<T> > (__getrange__)(Dee_ssize_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__getrange__", _Dee_HashSelectC(0x7f22541, 0x53d4d4259a06a055),  DEE_PCKdSIZ DEE_PCKdSIZ, start, end));
	}
	WUNUSED Ref<Sequence<T> > (__getrange__)(Dee_ssize_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__getrange__", _Dee_HashSelectC(0x7f22541, 0x53d4d4259a06a055),  DEE_PCKdSIZ DEE_PCKuSIZ, start, end));
	}
	WUNUSED Ref<Sequence<T> > (__getrange__)(size_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__getrange__", _Dee_HashSelectC(0x7f22541, 0x53d4d4259a06a055),  DEE_PCKuSIZ, start));
	}
	WUNUSED NONNULL_CXX((2)) Ref<Sequence<T> > (__getrange__)(size_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__getrange__", _Dee_HashSelectC(0x7f22541, 0x53d4d4259a06a055),  DEE_PCKuSIZ "o", start, end));
	}
	WUNUSED Ref<Sequence<T> > (__getrange__)(size_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__getrange__", _Dee_HashSelectC(0x7f22541, 0x53d4d4259a06a055),  DEE_PCKuSIZ DEE_PCKdSIZ, start, end));
	}
	WUNUSED Ref<Sequence<T> > (__getrange__)(size_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__getrange__", _Dee_HashSelectC(0x7f22541, 0x53d4d4259a06a055),  DEE_PCKuSIZ DEE_PCKuSIZ, start, end));
	}
	void (__delrange__)() {
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "__delrange__", _Dee_HashSelectC(0x685a6ec8, 0xbd7df74412129f9d), 0, NULL)));
	}
	NONNULL_CXX((1)) void (__delrange__)(DeeObject *start) {
		DeeObject *args[1];
		args[0] = start;
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "__delrange__", _Dee_HashSelectC(0x685a6ec8, 0xbd7df74412129f9d), 1, args)));
	}
	NONNULL_CXX((1, 2)) void (__delrange__)(DeeObject *start, DeeObject *end) {
		DeeObject *args[2];
		args[0] = start;
		args[1] = end;
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "__delrange__", _Dee_HashSelectC(0x685a6ec8, 0xbd7df74412129f9d), 2, args)));
	}
	NONNULL_CXX((1)) void (__delrange__)(DeeObject *start, Dee_ssize_t end) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "__delrange__", _Dee_HashSelectC(0x685a6ec8, 0xbd7df74412129f9d), "o" DEE_PCKdSIZ, start, end)));
	}
	NONNULL_CXX((1)) void (__delrange__)(DeeObject *start, size_t end) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "__delrange__", _Dee_HashSelectC(0x685a6ec8, 0xbd7df74412129f9d), "o" DEE_PCKuSIZ, start, end)));
	}
	void (__delrange__)(Dee_ssize_t start) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "__delrange__", _Dee_HashSelectC(0x685a6ec8, 0xbd7df74412129f9d),  DEE_PCKdSIZ, start)));
	}
	NONNULL_CXX((2)) void (__delrange__)(Dee_ssize_t start, DeeObject *end) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "__delrange__", _Dee_HashSelectC(0x685a6ec8, 0xbd7df74412129f9d),  DEE_PCKdSIZ "o", start, end)));
	}
	void (__delrange__)(Dee_ssize_t start, Dee_ssize_t end) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "__delrange__", _Dee_HashSelectC(0x685a6ec8, 0xbd7df74412129f9d),  DEE_PCKdSIZ DEE_PCKdSIZ, start, end)));
	}
	void (__delrange__)(Dee_ssize_t start, size_t end) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "__delrange__", _Dee_HashSelectC(0x685a6ec8, 0xbd7df74412129f9d),  DEE_PCKdSIZ DEE_PCKuSIZ, start, end)));
	}
	void (__delrange__)(size_t start) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "__delrange__", _Dee_HashSelectC(0x685a6ec8, 0xbd7df74412129f9d),  DEE_PCKuSIZ, start)));
	}
	NONNULL_CXX((2)) void (__delrange__)(size_t start, DeeObject *end) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "__delrange__", _Dee_HashSelectC(0x685a6ec8, 0xbd7df74412129f9d),  DEE_PCKuSIZ "o", start, end)));
	}
	void (__delrange__)(size_t start, Dee_ssize_t end) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "__delrange__", _Dee_HashSelectC(0x685a6ec8, 0xbd7df74412129f9d),  DEE_PCKuSIZ DEE_PCKdSIZ, start, end)));
	}
	void (__delrange__)(size_t start, size_t end) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "__delrange__", _Dee_HashSelectC(0x685a6ec8, 0xbd7df74412129f9d),  DEE_PCKuSIZ DEE_PCKuSIZ, start, end)));
	}
	NONNULL_CXX((1, 2, 3)) void (__setrange__)(DeeObject *start, DeeObject *end, DeeObject *values) {
		DeeObject *args[3];
		args[0] = start;
		args[1] = end;
		args[2] = values;
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "__setrange__", _Dee_HashSelectC(0x7f9874f9, 0x9da9fdb9a7e37ce8), 3, args)));
	}
	NONNULL_CXX((1, 3)) void (__setrange__)(DeeObject *start, Dee_ssize_t end, DeeObject *values) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "__setrange__", _Dee_HashSelectC(0x7f9874f9, 0x9da9fdb9a7e37ce8), "o" DEE_PCKdSIZ "o", start, end, values)));
	}
	NONNULL_CXX((1, 3)) void (__setrange__)(DeeObject *start, size_t end, DeeObject *values) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "__setrange__", _Dee_HashSelectC(0x7f9874f9, 0x9da9fdb9a7e37ce8), "o" DEE_PCKuSIZ "o", start, end, values)));
	}
	NONNULL_CXX((2, 3)) void (__setrange__)(Dee_ssize_t start, DeeObject *end, DeeObject *values) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "__setrange__", _Dee_HashSelectC(0x7f9874f9, 0x9da9fdb9a7e37ce8),  DEE_PCKdSIZ "oo", start, end, values)));
	}
	NONNULL_CXX((3)) void (__setrange__)(Dee_ssize_t start, Dee_ssize_t end, DeeObject *values) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "__setrange__", _Dee_HashSelectC(0x7f9874f9, 0x9da9fdb9a7e37ce8),  DEE_PCKdSIZ DEE_PCKdSIZ "o", start, end, values)));
	}
	NONNULL_CXX((3)) void (__setrange__)(Dee_ssize_t start, size_t end, DeeObject *values) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "__setrange__", _Dee_HashSelectC(0x7f9874f9, 0x9da9fdb9a7e37ce8),  DEE_PCKdSIZ DEE_PCKuSIZ "o", start, end, values)));
	}
	NONNULL_CXX((2, 3)) void (__setrange__)(size_t start, DeeObject *end, DeeObject *values) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "__setrange__", _Dee_HashSelectC(0x7f9874f9, 0x9da9fdb9a7e37ce8),  DEE_PCKuSIZ "oo", start, end, values)));
	}
	NONNULL_CXX((3)) void (__setrange__)(size_t start, Dee_ssize_t end, DeeObject *values) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "__setrange__", _Dee_HashSelectC(0x7f9874f9, 0x9da9fdb9a7e37ce8),  DEE_PCKuSIZ DEE_PCKdSIZ "o", start, end, values)));
	}
	NONNULL_CXX((3)) void (__setrange__)(size_t start, size_t end, DeeObject *values) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "__setrange__", _Dee_HashSelectC(0x7f9874f9, 0x9da9fdb9a7e37ce8),  DEE_PCKuSIZ DEE_PCKuSIZ "o", start, end, values)));
	}
	WUNUSED NONNULL_CXX((1)) Ref<T> (__foreach__)(DeeObject *cb) {
		DeeObject *args[1];
		args[0] = cb;
		return inherit(DeeObject_CallAttrStringHash(this, "__foreach__", _Dee_HashSelectC(0xeb324f7f, 0xa61da94d66bf93e9), 1, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<T> (__foreach_pair__)(DeeObject *cb) {
		DeeObject *args[1];
		args[0] = cb;
		return inherit(DeeObject_CallAttrStringHash(this, "__foreach_pair__", _Dee_HashSelectC(0x8b6283dd, 0x4bae3fff578a7438), 1, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<T> (__enumerate__)(DeeObject *cb) {
		DeeObject *args[1];
		args[0] = cb;
		return inherit(DeeObject_CallAttrStringHash(this, "__enumerate__", _Dee_HashSelectC(0xa424715d, 0xcc3185355ab5e636), 1, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<T> (__enumerate_index__)(DeeObject *cb) {
		DeeObject *args[1];
		args[0] = cb;
		return inherit(DeeObject_CallAttrStringHash(this, "__enumerate_index__", _Dee_HashSelectC(0xef24b4cd, 0xd4a294a07e266334), 1, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<T> (__enumerate_index__)(DeeObject *cb, DeeObject *start) {
		DeeObject *args[2];
		args[0] = cb;
		args[1] = start;
		return inherit(DeeObject_CallAttrStringHash(this, "__enumerate_index__", _Dee_HashSelectC(0xef24b4cd, 0xd4a294a07e266334), 2, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3)) Ref<T> (__enumerate_index__)(DeeObject *cb, DeeObject *start, DeeObject *end) {
		DeeObject *args[3];
		args[0] = cb;
		args[1] = start;
		args[2] = end;
		return inherit(DeeObject_CallAttrStringHash(this, "__enumerate_index__", _Dee_HashSelectC(0xef24b4cd, 0xd4a294a07e266334), 3, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<T> (__enumerate_index__)(DeeObject *cb, DeeObject *start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__enumerate_index__", _Dee_HashSelectC(0xef24b4cd, 0xd4a294a07e266334), "oo" DEE_PCKdSIZ, cb, start, end));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<T> (__enumerate_index__)(DeeObject *cb, DeeObject *start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__enumerate_index__", _Dee_HashSelectC(0xef24b4cd, 0xd4a294a07e266334), "oo" DEE_PCKuSIZ, cb, start, end));
	}
	WUNUSED NONNULL_CXX((1)) Ref<T> (__enumerate_index__)(DeeObject *cb, Dee_ssize_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__enumerate_index__", _Dee_HashSelectC(0xef24b4cd, 0xd4a294a07e266334), "o" DEE_PCKdSIZ, cb, start));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<T> (__enumerate_index__)(DeeObject *cb, Dee_ssize_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__enumerate_index__", _Dee_HashSelectC(0xef24b4cd, 0xd4a294a07e266334), "o" DEE_PCKdSIZ "o", cb, start, end));
	}
	WUNUSED NONNULL_CXX((1)) Ref<T> (__enumerate_index__)(DeeObject *cb, Dee_ssize_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__enumerate_index__", _Dee_HashSelectC(0xef24b4cd, 0xd4a294a07e266334), "o" DEE_PCKdSIZ DEE_PCKdSIZ, cb, start, end));
	}
	WUNUSED NONNULL_CXX((1)) Ref<T> (__enumerate_index__)(DeeObject *cb, Dee_ssize_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__enumerate_index__", _Dee_HashSelectC(0xef24b4cd, 0xd4a294a07e266334), "o" DEE_PCKdSIZ DEE_PCKuSIZ, cb, start, end));
	}
	WUNUSED NONNULL_CXX((1)) Ref<T> (__enumerate_index__)(DeeObject *cb, size_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__enumerate_index__", _Dee_HashSelectC(0xef24b4cd, 0xd4a294a07e266334), "o" DEE_PCKuSIZ, cb, start));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<T> (__enumerate_index__)(DeeObject *cb, size_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__enumerate_index__", _Dee_HashSelectC(0xef24b4cd, 0xd4a294a07e266334), "o" DEE_PCKuSIZ "o", cb, start, end));
	}
	WUNUSED NONNULL_CXX((1)) Ref<T> (__enumerate_index__)(DeeObject *cb, size_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__enumerate_index__", _Dee_HashSelectC(0xef24b4cd, 0xd4a294a07e266334), "o" DEE_PCKuSIZ DEE_PCKdSIZ, cb, start, end));
	}
	WUNUSED NONNULL_CXX((1)) Ref<T> (__enumerate_index__)(DeeObject *cb, size_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__enumerate_index__", _Dee_HashSelectC(0xef24b4cd, 0xd4a294a07e266334), "o" DEE_PCKuSIZ DEE_PCKuSIZ, cb, start, end));
	}
	WUNUSED Ref<Iterator<T> > (__iterkeys__)() {
		return inherit(DeeObject_CallAttrStringHash(this, "__iterkeys__", _Dee_HashSelectC(0x1cb7a03a, 0x911435bb7713e2b6), 0, NULL));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (__bounditem__)(DeeObject *index) {
		DeeObject *args[1];
		args[0] = index;
		return inherit(DeeObject_CallAttrStringHash(this, "__bounditem__", _Dee_HashSelectC(0xca532c4c, 0x346633a500ba801c), 1, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::bool_> (__bounditem__)(DeeObject *index, DeeObject *allow_missing) {
		DeeObject *args[2];
		args[0] = index;
		args[1] = allow_missing;
		return inherit(DeeObject_CallAttrStringHash(this, "__bounditem__", _Dee_HashSelectC(0xca532c4c, 0x346633a500ba801c), 2, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (__bounditem__)(DeeObject *index, bool allow_missing) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__bounditem__", _Dee_HashSelectC(0xca532c4c, 0x346633a500ba801c), "ob", index, allow_missing));
	}
	WUNUSED Ref<deemon::bool_> (__bounditem__)(Dee_ssize_t index) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__bounditem__", _Dee_HashSelectC(0xca532c4c, 0x346633a500ba801c),  DEE_PCKdSIZ, index));
	}
	WUNUSED NONNULL_CXX((2)) Ref<deemon::bool_> (__bounditem__)(Dee_ssize_t index, DeeObject *allow_missing) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__bounditem__", _Dee_HashSelectC(0xca532c4c, 0x346633a500ba801c),  DEE_PCKdSIZ "o", index, allow_missing));
	}
	WUNUSED Ref<deemon::bool_> (__bounditem__)(Dee_ssize_t index, bool allow_missing) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__bounditem__", _Dee_HashSelectC(0xca532c4c, 0x346633a500ba801c),  DEE_PCKdSIZ "b", index, allow_missing));
	}
	WUNUSED Ref<deemon::bool_> (__bounditem__)(size_t index) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__bounditem__", _Dee_HashSelectC(0xca532c4c, 0x346633a500ba801c),  DEE_PCKuSIZ, index));
	}
	WUNUSED NONNULL_CXX((2)) Ref<deemon::bool_> (__bounditem__)(size_t index, DeeObject *allow_missing) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__bounditem__", _Dee_HashSelectC(0xca532c4c, 0x346633a500ba801c),  DEE_PCKuSIZ "o", index, allow_missing));
	}
	WUNUSED Ref<deemon::bool_> (__bounditem__)(size_t index, bool allow_missing) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__bounditem__", _Dee_HashSelectC(0xca532c4c, 0x346633a500ba801c),  DEE_PCKuSIZ "b", index, allow_missing));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (__hasitem__)(DeeObject *index) {
		DeeObject *args[1];
		args[0] = index;
		return inherit(DeeObject_CallAttrStringHash(this, "__hasitem__", _Dee_HashSelectC(0x53c8b93c, 0x618a29a260a42a16), 1, args));
	}
	WUNUSED Ref<deemon::bool_> (__hasitem__)(Dee_ssize_t index) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__hasitem__", _Dee_HashSelectC(0x53c8b93c, 0x618a29a260a42a16),  DEE_PCKdSIZ, index));
	}
	WUNUSED Ref<deemon::bool_> (__hasitem__)(size_t index) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__hasitem__", _Dee_HashSelectC(0x53c8b93c, 0x618a29a260a42a16),  DEE_PCKuSIZ, index));
	}
	WUNUSED Ref<T> (__size_fast__)() {
		return inherit(DeeObject_CallAttrStringHash(this, "__size_fast__", _Dee_HashSelectC(0x691bc0e1, 0x2041fe796af51a80), 0, NULL));
	}
	WUNUSED NONNULL_CXX((1)) Ref<T> (__getitem_index__)(DeeObject *index) {
		DeeObject *args[1];
		args[0] = index;
		return inherit(DeeObject_CallAttrStringHash(this, "__getitem_index__", _Dee_HashSelectC(0x91cc2074, 0xf17dabe62fb9f4ce), 1, args));
	}
	WUNUSED Ref<T> (__getitem_index__)(Dee_ssize_t index) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__getitem_index__", _Dee_HashSelectC(0x91cc2074, 0xf17dabe62fb9f4ce),  DEE_PCKdSIZ, index));
	}
	WUNUSED Ref<T> (__getitem_index__)(size_t index) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__getitem_index__", _Dee_HashSelectC(0x91cc2074, 0xf17dabe62fb9f4ce),  DEE_PCKuSIZ, index));
	}
	NONNULL_CXX((1)) void (__delitem_index__)(DeeObject *index) {
		DeeObject *args[1];
		args[0] = index;
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "__delitem_index__", _Dee_HashSelectC(0x92b0848f, 0x436b9524d42c0f91), 1, args)));
	}
	void (__delitem_index__)(Dee_ssize_t index) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "__delitem_index__", _Dee_HashSelectC(0x92b0848f, 0x436b9524d42c0f91),  DEE_PCKdSIZ, index)));
	}
	void (__delitem_index__)(size_t index) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "__delitem_index__", _Dee_HashSelectC(0x92b0848f, 0x436b9524d42c0f91),  DEE_PCKuSIZ, index)));
	}
	NONNULL_CXX((1, 2)) void (__setitem_index__)(DeeObject *index, DeeObject *value) {
		DeeObject *args[2];
		args[0] = index;
		args[1] = value;
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "__setitem_index__", _Dee_HashSelectC(0x5f3dacf4, 0xf9e1066c6ed5b3af), 2, args)));
	}
	NONNULL_CXX((2)) void (__setitem_index__)(Dee_ssize_t index, DeeObject *value) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "__setitem_index__", _Dee_HashSelectC(0x5f3dacf4, 0xf9e1066c6ed5b3af),  DEE_PCKdSIZ "o", index, value)));
	}
	NONNULL_CXX((2)) void (__setitem_index__)(size_t index, DeeObject *value) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "__setitem_index__", _Dee_HashSelectC(0x5f3dacf4, 0xf9e1066c6ed5b3af),  DEE_PCKuSIZ "o", index, value)));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (__bounditem_index__)(DeeObject *index) {
		DeeObject *args[1];
		args[0] = index;
		return inherit(DeeObject_CallAttrStringHash(this, "__bounditem_index__", _Dee_HashSelectC(0xc93c6b80, 0x6cea097fcfc643a8), 1, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::bool_> (__bounditem_index__)(DeeObject *index, DeeObject *allow_missing) {
		DeeObject *args[2];
		args[0] = index;
		args[1] = allow_missing;
		return inherit(DeeObject_CallAttrStringHash(this, "__bounditem_index__", _Dee_HashSelectC(0xc93c6b80, 0x6cea097fcfc643a8), 2, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (__bounditem_index__)(DeeObject *index, bool allow_missing) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__bounditem_index__", _Dee_HashSelectC(0xc93c6b80, 0x6cea097fcfc643a8), "ob", index, allow_missing));
	}
	WUNUSED Ref<deemon::bool_> (__bounditem_index__)(Dee_ssize_t index) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__bounditem_index__", _Dee_HashSelectC(0xc93c6b80, 0x6cea097fcfc643a8),  DEE_PCKdSIZ, index));
	}
	WUNUSED NONNULL_CXX((2)) Ref<deemon::bool_> (__bounditem_index__)(Dee_ssize_t index, DeeObject *allow_missing) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__bounditem_index__", _Dee_HashSelectC(0xc93c6b80, 0x6cea097fcfc643a8),  DEE_PCKdSIZ "o", index, allow_missing));
	}
	WUNUSED Ref<deemon::bool_> (__bounditem_index__)(Dee_ssize_t index, bool allow_missing) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__bounditem_index__", _Dee_HashSelectC(0xc93c6b80, 0x6cea097fcfc643a8),  DEE_PCKdSIZ "b", index, allow_missing));
	}
	WUNUSED Ref<deemon::bool_> (__bounditem_index__)(size_t index) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__bounditem_index__", _Dee_HashSelectC(0xc93c6b80, 0x6cea097fcfc643a8),  DEE_PCKuSIZ, index));
	}
	WUNUSED NONNULL_CXX((2)) Ref<deemon::bool_> (__bounditem_index__)(size_t index, DeeObject *allow_missing) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__bounditem_index__", _Dee_HashSelectC(0xc93c6b80, 0x6cea097fcfc643a8),  DEE_PCKuSIZ "o", index, allow_missing));
	}
	WUNUSED Ref<deemon::bool_> (__bounditem_index__)(size_t index, bool allow_missing) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__bounditem_index__", _Dee_HashSelectC(0xc93c6b80, 0x6cea097fcfc643a8),  DEE_PCKuSIZ "b", index, allow_missing));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (__hasitem_index__)(DeeObject *index) {
		DeeObject *args[1];
		args[0] = index;
		return inherit(DeeObject_CallAttrStringHash(this, "__hasitem_index__", _Dee_HashSelectC(0xe3629815, 0xe761eca5403f1008), 1, args));
	}
	WUNUSED Ref<deemon::bool_> (__hasitem_index__)(Dee_ssize_t index) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__hasitem_index__", _Dee_HashSelectC(0xe3629815, 0xe761eca5403f1008),  DEE_PCKdSIZ, index));
	}
	WUNUSED Ref<deemon::bool_> (__hasitem_index__)(size_t index) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__hasitem_index__", _Dee_HashSelectC(0xe3629815, 0xe761eca5403f1008),  DEE_PCKuSIZ, index));
	}
	WUNUSED Ref<Sequence<T> > (__getrange_index__)() {
		return inherit(DeeObject_CallAttrStringHash(this, "__getrange_index__", _Dee_HashSelectC(0xd934483c, 0xeb5dabf913498601), 0, NULL));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Sequence<T> > (__getrange_index__)(DeeObject *start) {
		DeeObject *args[1];
		args[0] = start;
		return inherit(DeeObject_CallAttrStringHash(this, "__getrange_index__", _Dee_HashSelectC(0xd934483c, 0xeb5dabf913498601), 1, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<Sequence<T> > (__getrange_index__)(DeeObject *start, DeeObject *end) {
		DeeObject *args[2];
		args[0] = start;
		args[1] = end;
		return inherit(DeeObject_CallAttrStringHash(this, "__getrange_index__", _Dee_HashSelectC(0xd934483c, 0xeb5dabf913498601), 2, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Sequence<T> > (__getrange_index__)(DeeObject *start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__getrange_index__", _Dee_HashSelectC(0xd934483c, 0xeb5dabf913498601), "o" DEE_PCKdSIZ, start, end));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Sequence<T> > (__getrange_index__)(DeeObject *start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__getrange_index__", _Dee_HashSelectC(0xd934483c, 0xeb5dabf913498601), "o" DEE_PCKuSIZ, start, end));
	}
	WUNUSED Ref<Sequence<T> > (__getrange_index__)(Dee_ssize_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__getrange_index__", _Dee_HashSelectC(0xd934483c, 0xeb5dabf913498601),  DEE_PCKdSIZ, start));
	}
	WUNUSED NONNULL_CXX((2)) Ref<Sequence<T> > (__getrange_index__)(Dee_ssize_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__getrange_index__", _Dee_HashSelectC(0xd934483c, 0xeb5dabf913498601),  DEE_PCKdSIZ "o", start, end));
	}
	WUNUSED Ref<Sequence<T> > (__getrange_index__)(Dee_ssize_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__getrange_index__", _Dee_HashSelectC(0xd934483c, 0xeb5dabf913498601),  DEE_PCKdSIZ DEE_PCKdSIZ, start, end));
	}
	WUNUSED Ref<Sequence<T> > (__getrange_index__)(Dee_ssize_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__getrange_index__", _Dee_HashSelectC(0xd934483c, 0xeb5dabf913498601),  DEE_PCKdSIZ DEE_PCKuSIZ, start, end));
	}
	WUNUSED Ref<Sequence<T> > (__getrange_index__)(size_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__getrange_index__", _Dee_HashSelectC(0xd934483c, 0xeb5dabf913498601),  DEE_PCKuSIZ, start));
	}
	WUNUSED NONNULL_CXX((2)) Ref<Sequence<T> > (__getrange_index__)(size_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__getrange_index__", _Dee_HashSelectC(0xd934483c, 0xeb5dabf913498601),  DEE_PCKuSIZ "o", start, end));
	}
	WUNUSED Ref<Sequence<T> > (__getrange_index__)(size_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__getrange_index__", _Dee_HashSelectC(0xd934483c, 0xeb5dabf913498601),  DEE_PCKuSIZ DEE_PCKdSIZ, start, end));
	}
	WUNUSED Ref<Sequence<T> > (__getrange_index__)(size_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__getrange_index__", _Dee_HashSelectC(0xd934483c, 0xeb5dabf913498601),  DEE_PCKuSIZ DEE_PCKuSIZ, start, end));
	}
	void (__delrange_index__)() {
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "__delrange_index__", _Dee_HashSelectC(0x7fefc05, 0xedb527f3f0ccc1e9), 0, NULL)));
	}
	NONNULL_CXX((1)) void (__delrange_index__)(DeeObject *start) {
		DeeObject *args[1];
		args[0] = start;
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "__delrange_index__", _Dee_HashSelectC(0x7fefc05, 0xedb527f3f0ccc1e9), 1, args)));
	}
	NONNULL_CXX((1, 2)) void (__delrange_index__)(DeeObject *start, DeeObject *end) {
		DeeObject *args[2];
		args[0] = start;
		args[1] = end;
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "__delrange_index__", _Dee_HashSelectC(0x7fefc05, 0xedb527f3f0ccc1e9), 2, args)));
	}
	NONNULL_CXX((1)) void (__delrange_index__)(DeeObject *start, Dee_ssize_t end) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "__delrange_index__", _Dee_HashSelectC(0x7fefc05, 0xedb527f3f0ccc1e9), "o" DEE_PCKdSIZ, start, end)));
	}
	NONNULL_CXX((1)) void (__delrange_index__)(DeeObject *start, size_t end) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "__delrange_index__", _Dee_HashSelectC(0x7fefc05, 0xedb527f3f0ccc1e9), "o" DEE_PCKuSIZ, start, end)));
	}
	void (__delrange_index__)(Dee_ssize_t start) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "__delrange_index__", _Dee_HashSelectC(0x7fefc05, 0xedb527f3f0ccc1e9),  DEE_PCKdSIZ, start)));
	}
	NONNULL_CXX((2)) void (__delrange_index__)(Dee_ssize_t start, DeeObject *end) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "__delrange_index__", _Dee_HashSelectC(0x7fefc05, 0xedb527f3f0ccc1e9),  DEE_PCKdSIZ "o", start, end)));
	}
	void (__delrange_index__)(Dee_ssize_t start, Dee_ssize_t end) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "__delrange_index__", _Dee_HashSelectC(0x7fefc05, 0xedb527f3f0ccc1e9),  DEE_PCKdSIZ DEE_PCKdSIZ, start, end)));
	}
	void (__delrange_index__)(Dee_ssize_t start, size_t end) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "__delrange_index__", _Dee_HashSelectC(0x7fefc05, 0xedb527f3f0ccc1e9),  DEE_PCKdSIZ DEE_PCKuSIZ, start, end)));
	}
	void (__delrange_index__)(size_t start) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "__delrange_index__", _Dee_HashSelectC(0x7fefc05, 0xedb527f3f0ccc1e9),  DEE_PCKuSIZ, start)));
	}
	NONNULL_CXX((2)) void (__delrange_index__)(size_t start, DeeObject *end) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "__delrange_index__", _Dee_HashSelectC(0x7fefc05, 0xedb527f3f0ccc1e9),  DEE_PCKuSIZ "o", start, end)));
	}
	void (__delrange_index__)(size_t start, Dee_ssize_t end) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "__delrange_index__", _Dee_HashSelectC(0x7fefc05, 0xedb527f3f0ccc1e9),  DEE_PCKuSIZ DEE_PCKdSIZ, start, end)));
	}
	void (__delrange_index__)(size_t start, size_t end) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "__delrange_index__", _Dee_HashSelectC(0x7fefc05, 0xedb527f3f0ccc1e9),  DEE_PCKuSIZ DEE_PCKuSIZ, start, end)));
	}
	NONNULL_CXX((1, 2, 3)) void (__setrange_index__)(DeeObject *start, DeeObject *end, DeeObject *values) {
		DeeObject *args[3];
		args[0] = start;
		args[1] = end;
		args[2] = values;
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "__setrange_index__", _Dee_HashSelectC(0x66484220, 0x7924ae96a56da4ad), 3, args)));
	}
	NONNULL_CXX((1, 3)) void (__setrange_index__)(DeeObject *start, Dee_ssize_t end, DeeObject *values) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "__setrange_index__", _Dee_HashSelectC(0x66484220, 0x7924ae96a56da4ad), "o" DEE_PCKdSIZ "o", start, end, values)));
	}
	NONNULL_CXX((1, 3)) void (__setrange_index__)(DeeObject *start, size_t end, DeeObject *values) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "__setrange_index__", _Dee_HashSelectC(0x66484220, 0x7924ae96a56da4ad), "o" DEE_PCKuSIZ "o", start, end, values)));
	}
	NONNULL_CXX((2, 3)) void (__setrange_index__)(Dee_ssize_t start, DeeObject *end, DeeObject *values) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "__setrange_index__", _Dee_HashSelectC(0x66484220, 0x7924ae96a56da4ad),  DEE_PCKdSIZ "oo", start, end, values)));
	}
	NONNULL_CXX((3)) void (__setrange_index__)(Dee_ssize_t start, Dee_ssize_t end, DeeObject *values) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "__setrange_index__", _Dee_HashSelectC(0x66484220, 0x7924ae96a56da4ad),  DEE_PCKdSIZ DEE_PCKdSIZ "o", start, end, values)));
	}
	NONNULL_CXX((3)) void (__setrange_index__)(Dee_ssize_t start, size_t end, DeeObject *values) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "__setrange_index__", _Dee_HashSelectC(0x66484220, 0x7924ae96a56da4ad),  DEE_PCKdSIZ DEE_PCKuSIZ "o", start, end, values)));
	}
	NONNULL_CXX((2, 3)) void (__setrange_index__)(size_t start, DeeObject *end, DeeObject *values) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "__setrange_index__", _Dee_HashSelectC(0x66484220, 0x7924ae96a56da4ad),  DEE_PCKuSIZ "oo", start, end, values)));
	}
	NONNULL_CXX((3)) void (__setrange_index__)(size_t start, Dee_ssize_t end, DeeObject *values) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "__setrange_index__", _Dee_HashSelectC(0x66484220, 0x7924ae96a56da4ad),  DEE_PCKuSIZ DEE_PCKdSIZ "o", start, end, values)));
	}
	NONNULL_CXX((3)) void (__setrange_index__)(size_t start, size_t end, DeeObject *values) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "__setrange_index__", _Dee_HashSelectC(0x66484220, 0x7924ae96a56da4ad),  DEE_PCKuSIZ DEE_PCKuSIZ "o", start, end, values)));
	}
	WUNUSED NONNULL_CXX((1)) Ref<T> (__trygetitem__)(DeeObject *index) {
		DeeObject *args[1];
		args[0] = index;
		return inherit(DeeObject_CallAttrStringHash(this, "__trygetitem__", _Dee_HashSelectC(0x697faa5a, 0x9e668cedbada3131), 1, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<T> (__trygetitem__)(DeeObject *index, DeeObject *def) {
		DeeObject *args[2];
		args[0] = index;
		args[1] = def;
		return inherit(DeeObject_CallAttrStringHash(this, "__trygetitem__", _Dee_HashSelectC(0x697faa5a, 0x9e668cedbada3131), 2, args));
	}
	WUNUSED Ref<T> (__trygetitem__)(Dee_ssize_t index) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__trygetitem__", _Dee_HashSelectC(0x697faa5a, 0x9e668cedbada3131),  DEE_PCKdSIZ, index));
	}
	WUNUSED NONNULL_CXX((2)) Ref<T> (__trygetitem__)(Dee_ssize_t index, DeeObject *def) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__trygetitem__", _Dee_HashSelectC(0x697faa5a, 0x9e668cedbada3131),  DEE_PCKdSIZ "o", index, def));
	}
	WUNUSED Ref<T> (__trygetitem__)(size_t index) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__trygetitem__", _Dee_HashSelectC(0x697faa5a, 0x9e668cedbada3131),  DEE_PCKuSIZ, index));
	}
	WUNUSED NONNULL_CXX((2)) Ref<T> (__trygetitem__)(size_t index, DeeObject *def) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__trygetitem__", _Dee_HashSelectC(0x697faa5a, 0x9e668cedbada3131),  DEE_PCKuSIZ "o", index, def));
	}
	WUNUSED NONNULL_CXX((1)) Ref<T> (__trygetitem_index__)(DeeObject *index) {
		DeeObject *args[1];
		args[0] = index;
		return inherit(DeeObject_CallAttrStringHash(this, "__trygetitem_index__", _Dee_HashSelectC(0x7b9b3e4f, 0xb9cf2a9f61860a29), 1, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<T> (__trygetitem_index__)(DeeObject *index, DeeObject *def) {
		DeeObject *args[2];
		args[0] = index;
		args[1] = def;
		return inherit(DeeObject_CallAttrStringHash(this, "__trygetitem_index__", _Dee_HashSelectC(0x7b9b3e4f, 0xb9cf2a9f61860a29), 2, args));
	}
	WUNUSED Ref<T> (__trygetitem_index__)(Dee_ssize_t index) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__trygetitem_index__", _Dee_HashSelectC(0x7b9b3e4f, 0xb9cf2a9f61860a29),  DEE_PCKdSIZ, index));
	}
	WUNUSED NONNULL_CXX((2)) Ref<T> (__trygetitem_index__)(Dee_ssize_t index, DeeObject *def) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__trygetitem_index__", _Dee_HashSelectC(0x7b9b3e4f, 0xb9cf2a9f61860a29),  DEE_PCKdSIZ "o", index, def));
	}
	WUNUSED Ref<T> (__trygetitem_index__)(size_t index) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__trygetitem_index__", _Dee_HashSelectC(0x7b9b3e4f, 0xb9cf2a9f61860a29),  DEE_PCKuSIZ, index));
	}
	WUNUSED NONNULL_CXX((2)) Ref<T> (__trygetitem_index__)(size_t index, DeeObject *def) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__trygetitem_index__", _Dee_HashSelectC(0x7b9b3e4f, 0xb9cf2a9f61860a29),  DEE_PCKuSIZ "o", index, def));
	}
	WUNUSED Ref<deemon::int_> (__hash__)() {
		return inherit(DeeObject_CallAttrStringHash(this, "__hash__", _Dee_HashSelectC(0xc088645e, 0xbc5b5b1504b9d2d8), 0, NULL));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (__compare_eq__)(DeeObject *rhs) {
		DeeObject *args[1];
		args[0] = rhs;
		return inherit(DeeObject_CallAttrStringHash(this, "__compare_eq__", _Dee_HashSelectC(0xe8a4d608, 0x1e72244b6194ba57), 1, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (__compare__)(DeeObject *rhs) {
		DeeObject *args[1];
		args[0] = rhs;
		return inherit(DeeObject_CallAttrStringHash(this, "__compare__", _Dee_HashSelectC(0x6f1dbc07, 0x186c874121b03d6a), 1, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (__trycompare_eq__)(DeeObject *rhs) {
		DeeObject *args[1];
		args[0] = rhs;
		return inherit(DeeObject_CallAttrStringHash(this, "__trycompare_eq__", _Dee_HashSelectC(0x7e4181e0, 0x5e7888919a0cdc70), 1, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (__eq__)(DeeObject *rhs) {
		DeeObject *args[1];
		args[0] = rhs;
		return inherit(DeeObject_CallAttrStringHash(this, "__eq__", _Dee_HashSelectC(0x2e15aa28, 0x20311e6561792a00), 1, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (__ne__)(DeeObject *rhs) {
		DeeObject *args[1];
		args[0] = rhs;
		return inherit(DeeObject_CallAttrStringHash(this, "__ne__", _Dee_HashSelectC(0x485d961, 0xe9453f35f2aef187), 1, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (__lo__)(DeeObject *rhs) {
		DeeObject *args[1];
		args[0] = rhs;
		return inherit(DeeObject_CallAttrStringHash(this, "__lo__", _Dee_HashSelectC(0xbd689eba, 0xf2a5e28053b056c9), 1, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (__le__)(DeeObject *rhs) {
		DeeObject *args[1];
		args[0] = rhs;
		return inherit(DeeObject_CallAttrStringHash(this, "__le__", _Dee_HashSelectC(0xd4e31410, 0xe371879105557498), 1, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (__gr__)(DeeObject *rhs) {
		DeeObject *args[1];
		args[0] = rhs;
		return inherit(DeeObject_CallAttrStringHash(this, "__gr__", _Dee_HashSelectC(0x8af205e9, 0x3fe2793f689055e5), 1, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (__ge__)(DeeObject *rhs) {
		DeeObject *args[1];
		args[0] = rhs;
		return inherit(DeeObject_CallAttrStringHash(this, "__ge__", _Dee_HashSelectC(0xe467e452, 0xe5ad3ef5f6f17572), 1, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Sequence<T> > (__inplace_add__)(DeeObject *rhs) {
		DeeObject *args[1];
		args[0] = rhs;
		return inherit(DeeObject_CallAttrStringHash(this, "__inplace_add__", _Dee_HashSelectC(0x28e8178a, 0x4185ae98a0cae056), 1, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Sequence<T> > (__inplace_mul__)(DeeObject *factor) {
		DeeObject *args[1];
		args[0] = factor;
		return inherit(DeeObject_CallAttrStringHash(this, "__inplace_mul__", _Dee_HashSelectC(0x4a5fadc8, 0xb09538486fb7df18), 1, args));
	}
	WUNUSED Ref<Sequence<T> > (__inplace_mul__)(Dee_ssize_t factor) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__inplace_mul__", _Dee_HashSelectC(0x4a5fadc8, 0xb09538486fb7df18),  DEE_PCKdSIZ, factor));
	}
	WUNUSED Ref<Sequence<T> > (__inplace_mul__)(size_t factor) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__inplace_mul__", _Dee_HashSelectC(0x4a5fadc8, 0xb09538486fb7df18),  DEE_PCKuSIZ, factor));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Sequence<T> > (transform)(DeeObject *mapper) {
		DeeObject *args[1];
		args[0] = mapper;
		return inherit(DeeObject_CallAttrStringHash(this, "transform", _Dee_HashSelectC(0x1f489cc3, 0x9dc554137fbc6ef6), 1, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<T> (xch)(DeeObject *index, DeeObject *value) {
		DeeObject *args[2];
		args[0] = index;
		args[1] = value;
		return inherit(DeeObject_CallAttrStringHash(this, "xch", _Dee_HashSelectC(0x818ce38a, 0x6bb37305be1b0321), 2, args));
	}
	WUNUSED NONNULL_CXX((2)) Ref<T> (xch)(Dee_ssize_t index, DeeObject *value) {
		return inherit(DeeObject_CallAttrStringHashf(this, "xch", _Dee_HashSelectC(0x818ce38a, 0x6bb37305be1b0321),  DEE_PCKdSIZ "o", index, value));
	}
	WUNUSED NONNULL_CXX((2)) Ref<T> (xch)(size_t index, DeeObject *value) {
		return inherit(DeeObject_CallAttrStringHashf(this, "xch", _Dee_HashSelectC(0x818ce38a, 0x6bb37305be1b0321),  DEE_PCKuSIZ "o", index, value));
	}
	WUNUSED Ref<T> (front)() {
		return inherit(DeeObject_CallAttrStringHash(this, "front", _Dee_HashSelectC(0xcfb02b23, 0x52e2f82eb4cd4b29), 0, NULL));
	}
	WUNUSED Ref<T> (back)() {
		return inherit(DeeObject_CallAttrStringHash(this, "back", _Dee_HashSelectC(0xcee90855, 0x2b8e47c4c4dd02f6), 0, NULL));
	}
	WUNUSED Ref<deemon::bool_> (empty)() {
		return inherit(DeeObject_CallAttrStringHash(this, "empty", _Dee_HashSelectC(0x29cc2372, 0x88aaa44624e616f9), 0, NULL));
	}
	WUNUSED Ref<deemon::bool_> (non_empty)() {
		return inherit(DeeObject_CallAttrStringHash(this, "non_empty", _Dee_HashSelectC(0xaa839d50, 0x57830e42ce76bda3), 0, NULL));
	}
	WUNUSED NONNULL_CXX((1)) Ref<T> (at)(DeeObject *index) {
		DeeObject *args[1];
		args[0] = index;
		return inherit(DeeObject_CallAttrStringHash(this, "at", _Dee_HashSelectC(0xfe064fc3, 0x6375a48d1eff9d4e), 1, args));
	}
	WUNUSED Ref<T> (at)(Dee_ssize_t index) {
		return inherit(DeeObject_CallAttrStringHashf(this, "at", _Dee_HashSelectC(0xfe064fc3, 0x6375a48d1eff9d4e),  DEE_PCKdSIZ, index));
	}
	WUNUSED Ref<T> (at)(size_t index) {
		return inherit(DeeObject_CallAttrStringHashf(this, "at", _Dee_HashSelectC(0xfe064fc3, 0x6375a48d1eff9d4e),  DEE_PCKuSIZ, index));
	}
	WUNUSED NONNULL_CXX((1)) Ref<T> (get)(DeeObject *index) {
		DeeObject *args[1];
		args[0] = index;
		return inherit(DeeObject_CallAttrStringHash(this, "get", _Dee_HashSelectC(0x3b6d35a2, 0x7c8e1568eac4979f), 1, args));
	}
	WUNUSED Ref<T> (get)(Dee_ssize_t index) {
		return inherit(DeeObject_CallAttrStringHashf(this, "get", _Dee_HashSelectC(0x3b6d35a2, 0x7c8e1568eac4979f),  DEE_PCKdSIZ, index));
	}
	WUNUSED Ref<T> (get)(size_t index) {
		return inherit(DeeObject_CallAttrStringHashf(this, "get", _Dee_HashSelectC(0x3b6d35a2, 0x7c8e1568eac4979f),  DEE_PCKuSIZ, index));
	}
	class _Wrap_length
		: public deemon::detail::ConstGetRefProxy<_Wrap_length, deemon::int_> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap_length(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "length", _Dee_HashSelectC(0xecef0c1, 0x2993e8eb119cab21));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "length", _Dee_HashSelectC(0xecef0c1, 0x2993e8eb119cab21)));
		}
	};
	WUNUSED _Wrap_length (length)() DEE_CXX_NOTHROW {
		return this;
	}
	class _Wrap_first
		: public deemon::detail::ConstGetRefProxy<_Wrap_first, T>
		, public deemon::detail::ConstSetRefProxy<_Wrap_first, T> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		using deemon::detail::ConstSetRefProxy<_Wrap_first, T>::operator =;
		_Wrap_first(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "first", _Dee_HashSelectC(0xa9f0e818, 0x9d12a485470a29a7));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "first", _Dee_HashSelectC(0xa9f0e818, 0x9d12a485470a29a7)));
		}
		void del() const {
			throw_if_nonzero(DeeObject_DelAttrStringHash(m_self, "first", _Dee_HashSelectC(0xa9f0e818, 0x9d12a485470a29a7)));
		}
		int _setref(DeeObject *value) const DEE_CXX_NOTHROW {
			return DeeObject_SetAttrStringHash(m_self, "first", _Dee_HashSelectC(0xa9f0e818, 0x9d12a485470a29a7), value);
		}
	};
	WUNUSED _Wrap_first (first)() DEE_CXX_NOTHROW {
		return this;
	}
	class _Wrap_last
		: public deemon::detail::ConstGetRefProxy<_Wrap_last, T>
		, public deemon::detail::ConstSetRefProxy<_Wrap_last, T> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		using deemon::detail::ConstSetRefProxy<_Wrap_last, T>::operator =;
		_Wrap_last(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "last", _Dee_HashSelectC(0x185a4f9a, 0x760894ca6d41e4dc));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "last", _Dee_HashSelectC(0x185a4f9a, 0x760894ca6d41e4dc)));
		}
		void del() const {
			throw_if_nonzero(DeeObject_DelAttrStringHash(m_self, "last", _Dee_HashSelectC(0x185a4f9a, 0x760894ca6d41e4dc)));
		}
		int _setref(DeeObject *value) const DEE_CXX_NOTHROW {
			return DeeObject_SetAttrStringHash(m_self, "last", _Dee_HashSelectC(0x185a4f9a, 0x760894ca6d41e4dc), value);
		}
	};
	WUNUSED _Wrap_last (last)() DEE_CXX_NOTHROW {
		return this;
	}
	class _Wrap_each
		: public deemon::detail::ConstGetRefProxy<_Wrap_each, T> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap_each(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "each", _Dee_HashSelectC(0x9de8b13d, 0x374e052f37a5e158));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "each", _Dee_HashSelectC(0x9de8b13d, 0x374e052f37a5e158)));
		}
	};
	WUNUSED _Wrap_each (each)() DEE_CXX_NOTHROW {
		return this;
	}
	class _Wrap_ids
		: public deemon::detail::ConstGetRefProxy<_Wrap_ids, Sequence<deemon::int_> > {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap_ids(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "ids", _Dee_HashSelectC(0x3173a48f, 0x7cd9fae6cf17bb9f));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "ids", _Dee_HashSelectC(0x3173a48f, 0x7cd9fae6cf17bb9f)));
		}
	};
	WUNUSED _Wrap_ids (ids)() DEE_CXX_NOTHROW {
		return this;
	}
	class _Wrap_types
		: public deemon::detail::ConstGetRefProxy<_Wrap_types, Sequence<Type> > {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap_types(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "types", _Dee_HashSelectC(0x871b2836, 0xde8693a2d24930));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "types", _Dee_HashSelectC(0x871b2836, 0xde8693a2d24930)));
		}
	};
	WUNUSED _Wrap_types (types)() DEE_CXX_NOTHROW {
		return this;
	}
	class _Wrap_classes
		: public deemon::detail::ConstGetRefProxy<_Wrap_classes, Sequence<Type> > {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap_classes(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "classes", _Dee_HashSelectC(0x75e5899b, 0xc75d2d970415e4a0));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "classes", _Dee_HashSelectC(0x75e5899b, 0xc75d2d970415e4a0)));
		}
	};
	WUNUSED _Wrap_classes (classes)() DEE_CXX_NOTHROW {
		return this;
	}
	class _Wrap_isempty
		: public deemon::detail::ConstGetRefProxy<_Wrap_isempty, deemon::bool_> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap_isempty(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "isempty", _Dee_HashSelectC(0x693ca74b, 0xbdd6a8314c184088));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "isempty", _Dee_HashSelectC(0x693ca74b, 0xbdd6a8314c184088)));
		}
	};
	WUNUSED _Wrap_isempty (isempty)() DEE_CXX_NOTHROW {
		return this;
	}
	class _Wrap_isnonempty
		: public deemon::detail::ConstGetRefProxy<_Wrap_isnonempty, deemon::bool_> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap_isnonempty(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "isnonempty", _Dee_HashSelectC(0xde7e1cb1, 0x837d8f16bac4b317));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "isnonempty", _Dee_HashSelectC(0xde7e1cb1, 0x837d8f16bac4b317)));
		}
	};
	WUNUSED _Wrap_isnonempty (isnonempty)() DEE_CXX_NOTHROW {
		return this;
	}
	class _Wrap_isfrozen
		: public deemon::detail::ConstGetRefProxy<_Wrap_isfrozen, deemon::bool_> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap_isfrozen(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "isfrozen", _Dee_HashSelectC(0xb418a8ca, 0xa1babf165e1b1733));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "isfrozen", _Dee_HashSelectC(0xb418a8ca, 0xa1babf165e1b1733)));
		}
	};
	WUNUSED _Wrap_isfrozen (isfrozen)() DEE_CXX_NOTHROW {
		return this;
	}
	class _Wrap_frozen
		: public deemon::detail::ConstGetRefProxy<_Wrap_frozen, Sequence<T> > {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap_frozen(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "frozen", _Dee_HashSelectC(0x82311b77, 0x7b55e2e6e642b6fd));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "frozen", _Dee_HashSelectC(0x82311b77, 0x7b55e2e6e642b6fd)));
		}
	};
	WUNUSED _Wrap_frozen (frozen)() DEE_CXX_NOTHROW {
		return this;
	}
/*[[[end]]]*/
};

DEE_CXX_END

#endif /* !GUARD_DEEMON_CXX_SEQUENCE_H */
