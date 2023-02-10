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
	static Ref<Sequence<T> > range(DeeObject *begin, DeeObject *end, DeeObject *step = NULL) {
		return inherit(DeeRange_New(begin, end, step));
	}
	static Ref<Sequence<T> > range(Dee_ssize_t begin, Dee_ssize_t end, Dee_ssize_t step = 1) {
		return inherit(DeeRange_NewInt(begin, end, step));
	}

public:
	using detail::ItemProxyAccessor<Sequence<T>, T>::iterator;
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
	WUNUSED NONNULL_CXX((1)) Ref<T> (reduce)(DeeObject *merger) {
		DeeObject *args[1];
		args[0] = merger;
		return inherit(DeeObject_CallAttrStringHash(this, "reduce", _Dee_HashSelect(UINT32_C(0x907b2992), UINT64_C(0xb7e66094a8a9409d)), 1, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<T> (reduce)(DeeObject *merger, DeeObject *init) {
		DeeObject *args[2];
		args[0] = merger;
		args[1] = init;
		return inherit(DeeObject_CallAttrStringHash(this, "reduce", _Dee_HashSelect(UINT32_C(0x907b2992), UINT64_C(0xb7e66094a8a9409d)), 2, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Sequence<T> > (filter)(DeeObject *keep) {
		DeeObject *args[1];
		args[0] = keep;
		return inherit(DeeObject_CallAttrStringHash(this, "filter", _Dee_HashSelect(UINT32_C(0x3110088a), UINT64_C(0x32e04884df75b1c1)), 1, args));
	}
	WUNUSED Ref<T> (min)() {
		return inherit(DeeObject_CallAttrStringHash(this, "min", _Dee_HashSelect(UINT32_C(0xde957cfa), UINT64_C(0xe6ffc7365e2d00ca)), 0, NULL));
	}
	WUNUSED NONNULL_CXX((1)) Ref<T> (min)(DeeObject *key) {
		DeeObject *args[1];
		args[0] = key;
		return inherit(DeeObject_CallAttrStringHash(this, "min", _Dee_HashSelect(UINT32_C(0xde957cfa), UINT64_C(0xe6ffc7365e2d00ca)), 1, args));
	}
	WUNUSED Ref<T> (max)() {
		return inherit(DeeObject_CallAttrStringHash(this, "max", _Dee_HashSelect(UINT32_C(0xc293979b), UINT64_C(0x822bd5c706bd9850)), 0, NULL));
	}
	WUNUSED NONNULL_CXX((1)) Ref<T> (max)(DeeObject *key) {
		DeeObject *args[1];
		args[0] = key;
		return inherit(DeeObject_CallAttrStringHash(this, "max", _Dee_HashSelect(UINT32_C(0xc293979b), UINT64_C(0x822bd5c706bd9850)), 1, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (count)(DeeObject *elem) {
		DeeObject *args[1];
		args[0] = elem;
		return inherit(DeeObject_CallAttrStringHash(this, "count", _Dee_HashSelect(UINT32_C(0x54eac164), UINT64_C(0xbd66b5980d54babb)), 1, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::int_> (count)(DeeObject *elem, DeeObject *key) {
		DeeObject *args[2];
		args[0] = elem;
		args[1] = key;
		return inherit(DeeObject_CallAttrStringHash(this, "count", _Dee_HashSelect(UINT32_C(0x54eac164), UINT64_C(0xbd66b5980d54babb)), 2, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<T> (locate)(DeeObject *elem) {
		DeeObject *args[1];
		args[0] = elem;
		return inherit(DeeObject_CallAttrStringHash(this, "locate", _Dee_HashSelect(UINT32_C(0x72fc7691), UINT64_C(0xa3c641c56f3d6258)), 1, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<T> (locate)(DeeObject *elem, DeeObject *key) {
		DeeObject *args[2];
		args[0] = elem;
		args[1] = key;
		return inherit(DeeObject_CallAttrStringHash(this, "locate", _Dee_HashSelect(UINT32_C(0x72fc7691), UINT64_C(0xa3c641c56f3d6258)), 2, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<T> (rlocate)(DeeObject *elem) {
		DeeObject *args[1];
		args[0] = elem;
		return inherit(DeeObject_CallAttrStringHash(this, "rlocate", _Dee_HashSelect(UINT32_C(0xe233056a), UINT64_C(0xf4759389157e74b)), 1, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<T> (rlocate)(DeeObject *elem, DeeObject *key) {
		DeeObject *args[2];
		args[0] = elem;
		args[1] = key;
		return inherit(DeeObject_CallAttrStringHash(this, "rlocate", _Dee_HashSelect(UINT32_C(0xe233056a), UINT64_C(0xf4759389157e74b)), 2, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Sequence<T> > (locateall)(DeeObject *elem) {
		DeeObject *args[1];
		args[0] = elem;
		return inherit(DeeObject_CallAttrStringHash(this, "locateall", _Dee_HashSelect(UINT32_C(0xd447ec), UINT64_C(0xc6a682da9d9f8345)), 1, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<Sequence<T> > (locateall)(DeeObject *elem, DeeObject *key) {
		DeeObject *args[2];
		args[0] = elem;
		args[1] = key;
		return inherit(DeeObject_CallAttrStringHash(this, "locateall", _Dee_HashSelect(UINT32_C(0xd447ec), UINT64_C(0xc6a682da9d9f8345)), 2, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Sequence<T> > (transform)(DeeObject *transformation) {
		DeeObject *args[1];
		args[0] = transformation;
		return inherit(DeeObject_CallAttrStringHash(this, "transform", _Dee_HashSelect(UINT32_C(0x1f489cc3), UINT64_C(0x9dc554137fbc6ef6)), 1, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (contains)(DeeObject *elem) {
		DeeObject *args[1];
		args[0] = elem;
		return inherit(DeeObject_CallAttrStringHash(this, "contains", _Dee_HashSelect(UINT32_C(0x9338eec0), UINT64_C(0x1e0128373382a209)), 1, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::bool_> (contains)(DeeObject *elem, DeeObject *key) {
		DeeObject *args[2];
		args[0] = elem;
		args[1] = key;
		return inherit(DeeObject_CallAttrStringHash(this, "contains", _Dee_HashSelect(UINT32_C(0x9338eec0), UINT64_C(0x1e0128373382a209)), 2, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (startswith)(DeeObject *elem) {
		DeeObject *args[1];
		args[0] = elem;
		return inherit(DeeObject_CallAttrStringHash(this, "startswith", _Dee_HashSelect(UINT32_C(0x58e22b), UINT64_C(0x6251da2c5cdb654d)), 1, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::bool_> (startswith)(DeeObject *elem, DeeObject *key) {
		DeeObject *args[2];
		args[0] = elem;
		args[1] = key;
		return inherit(DeeObject_CallAttrStringHash(this, "startswith", _Dee_HashSelect(UINT32_C(0x58e22b), UINT64_C(0x6251da2c5cdb654d)), 2, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (endswith)(DeeObject *elem) {
		DeeObject *args[1];
		args[0] = elem;
		return inherit(DeeObject_CallAttrStringHash(this, "endswith", _Dee_HashSelect(UINT32_C(0x8bdeff05), UINT64_C(0xca6abed3214345e1)), 1, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::bool_> (endswith)(DeeObject *elem, DeeObject *key) {
		DeeObject *args[2];
		args[0] = elem;
		args[1] = key;
		return inherit(DeeObject_CallAttrStringHash(this, "endswith", _Dee_HashSelect(UINT32_C(0x8bdeff05), UINT64_C(0xca6abed3214345e1)), 2, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (find)(DeeObject *elem) {
		DeeObject *args[1];
		args[0] = elem;
		return inherit(DeeObject_CallAttrStringHash(this, "find", _Dee_HashSelect(UINT32_C(0x9e66372), UINT64_C(0x2b65fe03bbdde5b2)), 1, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::int_> (find)(DeeObject *elem, DeeObject *key_or_start) {
		DeeObject *args[2];
		args[0] = elem;
		args[1] = key_or_start;
		return inherit(DeeObject_CallAttrStringHash(this, "find", _Dee_HashSelect(UINT32_C(0x9e66372), UINT64_C(0x2b65fe03bbdde5b2)), 2, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3)) Ref<deemon::int_> (find)(DeeObject *elem, DeeObject *start, DeeObject *key_or_end) {
		DeeObject *args[3];
		args[0] = elem;
		args[1] = start;
		args[2] = key_or_end;
		return inherit(DeeObject_CallAttrStringHash(this, "find", _Dee_HashSelect(UINT32_C(0x9e66372), UINT64_C(0x2b65fe03bbdde5b2)), 3, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3, 4)) Ref<deemon::int_> (find)(DeeObject *elem, DeeObject *start, DeeObject *end, DeeObject *key) {
		DeeObject *args[4];
		args[0] = elem;
		args[1] = start;
		args[2] = end;
		args[3] = key;
		return inherit(DeeObject_CallAttrStringHash(this, "find", _Dee_HashSelect(UINT32_C(0x9e66372), UINT64_C(0x2b65fe03bbdde5b2)), 4, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::int_> (find)(DeeObject *elem, DeeObject *start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "find", _Dee_HashSelect(UINT32_C(0x9e66372), UINT64_C(0x2b65fe03bbdde5b2)), "oo" DEE_PCKdSIZ, elem, start, end));
	}
	WUNUSED NONNULL_CXX((1, 2, 4)) Ref<deemon::int_> (find)(DeeObject *elem, DeeObject *start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "find", _Dee_HashSelect(UINT32_C(0x9e66372), UINT64_C(0x2b65fe03bbdde5b2)), "oo" DEE_PCKdSIZ "o", elem, start, end, key));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::int_> (find)(DeeObject *elem, DeeObject *start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "find", _Dee_HashSelect(UINT32_C(0x9e66372), UINT64_C(0x2b65fe03bbdde5b2)), "oo" DEE_PCKuSIZ, elem, start, end));
	}
	WUNUSED NONNULL_CXX((1, 2, 4)) Ref<deemon::int_> (find)(DeeObject *elem, DeeObject *start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "find", _Dee_HashSelect(UINT32_C(0x9e66372), UINT64_C(0x2b65fe03bbdde5b2)), "oo" DEE_PCKuSIZ "o", elem, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (find)(DeeObject *elem, Dee_ssize_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "find", _Dee_HashSelect(UINT32_C(0x9e66372), UINT64_C(0x2b65fe03bbdde5b2)), "o" DEE_PCKdSIZ, elem, start));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::int_> (find)(DeeObject *elem, Dee_ssize_t start, DeeObject *key_or_end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "find", _Dee_HashSelect(UINT32_C(0x9e66372), UINT64_C(0x2b65fe03bbdde5b2)), "o" DEE_PCKdSIZ "o", elem, start, key_or_end));
	}
	WUNUSED NONNULL_CXX((1, 3, 4)) Ref<deemon::int_> (find)(DeeObject *elem, Dee_ssize_t start, DeeObject *end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "find", _Dee_HashSelect(UINT32_C(0x9e66372), UINT64_C(0x2b65fe03bbdde5b2)), "o" DEE_PCKdSIZ "oo", elem, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (find)(DeeObject *elem, Dee_ssize_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "find", _Dee_HashSelect(UINT32_C(0x9e66372), UINT64_C(0x2b65fe03bbdde5b2)), "o" DEE_PCKdSIZ DEE_PCKdSIZ, elem, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::int_> (find)(DeeObject *elem, Dee_ssize_t start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "find", _Dee_HashSelect(UINT32_C(0x9e66372), UINT64_C(0x2b65fe03bbdde5b2)), "o" DEE_PCKdSIZ DEE_PCKdSIZ "o", elem, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (find)(DeeObject *elem, Dee_ssize_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "find", _Dee_HashSelect(UINT32_C(0x9e66372), UINT64_C(0x2b65fe03bbdde5b2)), "o" DEE_PCKdSIZ DEE_PCKuSIZ, elem, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::int_> (find)(DeeObject *elem, Dee_ssize_t start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "find", _Dee_HashSelect(UINT32_C(0x9e66372), UINT64_C(0x2b65fe03bbdde5b2)), "o" DEE_PCKdSIZ DEE_PCKuSIZ "o", elem, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (find)(DeeObject *elem, size_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "find", _Dee_HashSelect(UINT32_C(0x9e66372), UINT64_C(0x2b65fe03bbdde5b2)), "o" DEE_PCKuSIZ, elem, start));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::int_> (find)(DeeObject *elem, size_t start, DeeObject *key_or_end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "find", _Dee_HashSelect(UINT32_C(0x9e66372), UINT64_C(0x2b65fe03bbdde5b2)), "o" DEE_PCKuSIZ "o", elem, start, key_or_end));
	}
	WUNUSED NONNULL_CXX((1, 3, 4)) Ref<deemon::int_> (find)(DeeObject *elem, size_t start, DeeObject *end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "find", _Dee_HashSelect(UINT32_C(0x9e66372), UINT64_C(0x2b65fe03bbdde5b2)), "o" DEE_PCKuSIZ "oo", elem, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (find)(DeeObject *elem, size_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "find", _Dee_HashSelect(UINT32_C(0x9e66372), UINT64_C(0x2b65fe03bbdde5b2)), "o" DEE_PCKuSIZ DEE_PCKdSIZ, elem, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::int_> (find)(DeeObject *elem, size_t start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "find", _Dee_HashSelect(UINT32_C(0x9e66372), UINT64_C(0x2b65fe03bbdde5b2)), "o" DEE_PCKuSIZ DEE_PCKdSIZ "o", elem, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (find)(DeeObject *elem, size_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "find", _Dee_HashSelect(UINT32_C(0x9e66372), UINT64_C(0x2b65fe03bbdde5b2)), "o" DEE_PCKuSIZ DEE_PCKuSIZ, elem, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::int_> (find)(DeeObject *elem, size_t start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "find", _Dee_HashSelect(UINT32_C(0x9e66372), UINT64_C(0x2b65fe03bbdde5b2)), "o" DEE_PCKuSIZ DEE_PCKuSIZ "o", elem, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (rfind)(DeeObject *elem) {
		DeeObject *args[1];
		args[0] = elem;
		return inherit(DeeObject_CallAttrStringHash(this, "rfind", _Dee_HashSelect(UINT32_C(0xfb368ca), UINT64_C(0x8b40ffa7172dc59d)), 1, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::int_> (rfind)(DeeObject *elem, DeeObject *key_or_start) {
		DeeObject *args[2];
		args[0] = elem;
		args[1] = key_or_start;
		return inherit(DeeObject_CallAttrStringHash(this, "rfind", _Dee_HashSelect(UINT32_C(0xfb368ca), UINT64_C(0x8b40ffa7172dc59d)), 2, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3)) Ref<deemon::int_> (rfind)(DeeObject *elem, DeeObject *start, DeeObject *key_or_end) {
		DeeObject *args[3];
		args[0] = elem;
		args[1] = start;
		args[2] = key_or_end;
		return inherit(DeeObject_CallAttrStringHash(this, "rfind", _Dee_HashSelect(UINT32_C(0xfb368ca), UINT64_C(0x8b40ffa7172dc59d)), 3, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3, 4)) Ref<deemon::int_> (rfind)(DeeObject *elem, DeeObject *start, DeeObject *end, DeeObject *key) {
		DeeObject *args[4];
		args[0] = elem;
		args[1] = start;
		args[2] = end;
		args[3] = key;
		return inherit(DeeObject_CallAttrStringHash(this, "rfind", _Dee_HashSelect(UINT32_C(0xfb368ca), UINT64_C(0x8b40ffa7172dc59d)), 4, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::int_> (rfind)(DeeObject *elem, DeeObject *start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rfind", _Dee_HashSelect(UINT32_C(0xfb368ca), UINT64_C(0x8b40ffa7172dc59d)), "oo" DEE_PCKdSIZ, elem, start, end));
	}
	WUNUSED NONNULL_CXX((1, 2, 4)) Ref<deemon::int_> (rfind)(DeeObject *elem, DeeObject *start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rfind", _Dee_HashSelect(UINT32_C(0xfb368ca), UINT64_C(0x8b40ffa7172dc59d)), "oo" DEE_PCKdSIZ "o", elem, start, end, key));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::int_> (rfind)(DeeObject *elem, DeeObject *start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rfind", _Dee_HashSelect(UINT32_C(0xfb368ca), UINT64_C(0x8b40ffa7172dc59d)), "oo" DEE_PCKuSIZ, elem, start, end));
	}
	WUNUSED NONNULL_CXX((1, 2, 4)) Ref<deemon::int_> (rfind)(DeeObject *elem, DeeObject *start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rfind", _Dee_HashSelect(UINT32_C(0xfb368ca), UINT64_C(0x8b40ffa7172dc59d)), "oo" DEE_PCKuSIZ "o", elem, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (rfind)(DeeObject *elem, Dee_ssize_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rfind", _Dee_HashSelect(UINT32_C(0xfb368ca), UINT64_C(0x8b40ffa7172dc59d)), "o" DEE_PCKdSIZ, elem, start));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::int_> (rfind)(DeeObject *elem, Dee_ssize_t start, DeeObject *key_or_end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rfind", _Dee_HashSelect(UINT32_C(0xfb368ca), UINT64_C(0x8b40ffa7172dc59d)), "o" DEE_PCKdSIZ "o", elem, start, key_or_end));
	}
	WUNUSED NONNULL_CXX((1, 3, 4)) Ref<deemon::int_> (rfind)(DeeObject *elem, Dee_ssize_t start, DeeObject *end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rfind", _Dee_HashSelect(UINT32_C(0xfb368ca), UINT64_C(0x8b40ffa7172dc59d)), "o" DEE_PCKdSIZ "oo", elem, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (rfind)(DeeObject *elem, Dee_ssize_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rfind", _Dee_HashSelect(UINT32_C(0xfb368ca), UINT64_C(0x8b40ffa7172dc59d)), "o" DEE_PCKdSIZ DEE_PCKdSIZ, elem, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::int_> (rfind)(DeeObject *elem, Dee_ssize_t start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rfind", _Dee_HashSelect(UINT32_C(0xfb368ca), UINT64_C(0x8b40ffa7172dc59d)), "o" DEE_PCKdSIZ DEE_PCKdSIZ "o", elem, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (rfind)(DeeObject *elem, Dee_ssize_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rfind", _Dee_HashSelect(UINT32_C(0xfb368ca), UINT64_C(0x8b40ffa7172dc59d)), "o" DEE_PCKdSIZ DEE_PCKuSIZ, elem, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::int_> (rfind)(DeeObject *elem, Dee_ssize_t start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rfind", _Dee_HashSelect(UINT32_C(0xfb368ca), UINT64_C(0x8b40ffa7172dc59d)), "o" DEE_PCKdSIZ DEE_PCKuSIZ "o", elem, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (rfind)(DeeObject *elem, size_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rfind", _Dee_HashSelect(UINT32_C(0xfb368ca), UINT64_C(0x8b40ffa7172dc59d)), "o" DEE_PCKuSIZ, elem, start));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::int_> (rfind)(DeeObject *elem, size_t start, DeeObject *key_or_end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rfind", _Dee_HashSelect(UINT32_C(0xfb368ca), UINT64_C(0x8b40ffa7172dc59d)), "o" DEE_PCKuSIZ "o", elem, start, key_or_end));
	}
	WUNUSED NONNULL_CXX((1, 3, 4)) Ref<deemon::int_> (rfind)(DeeObject *elem, size_t start, DeeObject *end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rfind", _Dee_HashSelect(UINT32_C(0xfb368ca), UINT64_C(0x8b40ffa7172dc59d)), "o" DEE_PCKuSIZ "oo", elem, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (rfind)(DeeObject *elem, size_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rfind", _Dee_HashSelect(UINT32_C(0xfb368ca), UINT64_C(0x8b40ffa7172dc59d)), "o" DEE_PCKuSIZ DEE_PCKdSIZ, elem, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::int_> (rfind)(DeeObject *elem, size_t start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rfind", _Dee_HashSelect(UINT32_C(0xfb368ca), UINT64_C(0x8b40ffa7172dc59d)), "o" DEE_PCKuSIZ DEE_PCKdSIZ "o", elem, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (rfind)(DeeObject *elem, size_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rfind", _Dee_HashSelect(UINT32_C(0xfb368ca), UINT64_C(0x8b40ffa7172dc59d)), "o" DEE_PCKuSIZ DEE_PCKuSIZ, elem, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::int_> (rfind)(DeeObject *elem, size_t start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rfind", _Dee_HashSelect(UINT32_C(0xfb368ca), UINT64_C(0x8b40ffa7172dc59d)), "o" DEE_PCKuSIZ DEE_PCKuSIZ "o", elem, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (index)(DeeObject *elem) {
		DeeObject *args[1];
		args[0] = elem;
		return inherit(DeeObject_CallAttrStringHash(this, "index", _Dee_HashSelect(UINT32_C(0x77f34f0), UINT64_C(0x440d5888c0ff3081)), 1, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::int_> (index)(DeeObject *elem, DeeObject *key_or_start) {
		DeeObject *args[2];
		args[0] = elem;
		args[1] = key_or_start;
		return inherit(DeeObject_CallAttrStringHash(this, "index", _Dee_HashSelect(UINT32_C(0x77f34f0), UINT64_C(0x440d5888c0ff3081)), 2, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3)) Ref<deemon::int_> (index)(DeeObject *elem, DeeObject *start, DeeObject *key_or_end) {
		DeeObject *args[3];
		args[0] = elem;
		args[1] = start;
		args[2] = key_or_end;
		return inherit(DeeObject_CallAttrStringHash(this, "index", _Dee_HashSelect(UINT32_C(0x77f34f0), UINT64_C(0x440d5888c0ff3081)), 3, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3, 4)) Ref<deemon::int_> (index)(DeeObject *elem, DeeObject *start, DeeObject *end, DeeObject *key) {
		DeeObject *args[4];
		args[0] = elem;
		args[1] = start;
		args[2] = end;
		args[3] = key;
		return inherit(DeeObject_CallAttrStringHash(this, "index", _Dee_HashSelect(UINT32_C(0x77f34f0), UINT64_C(0x440d5888c0ff3081)), 4, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::int_> (index)(DeeObject *elem, DeeObject *start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "index", _Dee_HashSelect(UINT32_C(0x77f34f0), UINT64_C(0x440d5888c0ff3081)), "oo" DEE_PCKdSIZ, elem, start, end));
	}
	WUNUSED NONNULL_CXX((1, 2, 4)) Ref<deemon::int_> (index)(DeeObject *elem, DeeObject *start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "index", _Dee_HashSelect(UINT32_C(0x77f34f0), UINT64_C(0x440d5888c0ff3081)), "oo" DEE_PCKdSIZ "o", elem, start, end, key));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::int_> (index)(DeeObject *elem, DeeObject *start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "index", _Dee_HashSelect(UINT32_C(0x77f34f0), UINT64_C(0x440d5888c0ff3081)), "oo" DEE_PCKuSIZ, elem, start, end));
	}
	WUNUSED NONNULL_CXX((1, 2, 4)) Ref<deemon::int_> (index)(DeeObject *elem, DeeObject *start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "index", _Dee_HashSelect(UINT32_C(0x77f34f0), UINT64_C(0x440d5888c0ff3081)), "oo" DEE_PCKuSIZ "o", elem, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (index)(DeeObject *elem, Dee_ssize_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "index", _Dee_HashSelect(UINT32_C(0x77f34f0), UINT64_C(0x440d5888c0ff3081)), "o" DEE_PCKdSIZ, elem, start));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::int_> (index)(DeeObject *elem, Dee_ssize_t start, DeeObject *key_or_end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "index", _Dee_HashSelect(UINT32_C(0x77f34f0), UINT64_C(0x440d5888c0ff3081)), "o" DEE_PCKdSIZ "o", elem, start, key_or_end));
	}
	WUNUSED NONNULL_CXX((1, 3, 4)) Ref<deemon::int_> (index)(DeeObject *elem, Dee_ssize_t start, DeeObject *end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "index", _Dee_HashSelect(UINT32_C(0x77f34f0), UINT64_C(0x440d5888c0ff3081)), "o" DEE_PCKdSIZ "oo", elem, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (index)(DeeObject *elem, Dee_ssize_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "index", _Dee_HashSelect(UINT32_C(0x77f34f0), UINT64_C(0x440d5888c0ff3081)), "o" DEE_PCKdSIZ DEE_PCKdSIZ, elem, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::int_> (index)(DeeObject *elem, Dee_ssize_t start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "index", _Dee_HashSelect(UINT32_C(0x77f34f0), UINT64_C(0x440d5888c0ff3081)), "o" DEE_PCKdSIZ DEE_PCKdSIZ "o", elem, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (index)(DeeObject *elem, Dee_ssize_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "index", _Dee_HashSelect(UINT32_C(0x77f34f0), UINT64_C(0x440d5888c0ff3081)), "o" DEE_PCKdSIZ DEE_PCKuSIZ, elem, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::int_> (index)(DeeObject *elem, Dee_ssize_t start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "index", _Dee_HashSelect(UINT32_C(0x77f34f0), UINT64_C(0x440d5888c0ff3081)), "o" DEE_PCKdSIZ DEE_PCKuSIZ "o", elem, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (index)(DeeObject *elem, size_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "index", _Dee_HashSelect(UINT32_C(0x77f34f0), UINT64_C(0x440d5888c0ff3081)), "o" DEE_PCKuSIZ, elem, start));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::int_> (index)(DeeObject *elem, size_t start, DeeObject *key_or_end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "index", _Dee_HashSelect(UINT32_C(0x77f34f0), UINT64_C(0x440d5888c0ff3081)), "o" DEE_PCKuSIZ "o", elem, start, key_or_end));
	}
	WUNUSED NONNULL_CXX((1, 3, 4)) Ref<deemon::int_> (index)(DeeObject *elem, size_t start, DeeObject *end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "index", _Dee_HashSelect(UINT32_C(0x77f34f0), UINT64_C(0x440d5888c0ff3081)), "o" DEE_PCKuSIZ "oo", elem, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (index)(DeeObject *elem, size_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "index", _Dee_HashSelect(UINT32_C(0x77f34f0), UINT64_C(0x440d5888c0ff3081)), "o" DEE_PCKuSIZ DEE_PCKdSIZ, elem, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::int_> (index)(DeeObject *elem, size_t start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "index", _Dee_HashSelect(UINT32_C(0x77f34f0), UINT64_C(0x440d5888c0ff3081)), "o" DEE_PCKuSIZ DEE_PCKdSIZ "o", elem, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (index)(DeeObject *elem, size_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "index", _Dee_HashSelect(UINT32_C(0x77f34f0), UINT64_C(0x440d5888c0ff3081)), "o" DEE_PCKuSIZ DEE_PCKuSIZ, elem, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::int_> (index)(DeeObject *elem, size_t start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "index", _Dee_HashSelect(UINT32_C(0x77f34f0), UINT64_C(0x440d5888c0ff3081)), "o" DEE_PCKuSIZ DEE_PCKuSIZ "o", elem, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (rindex)(DeeObject *elem) {
		DeeObject *args[1];
		args[0] = elem;
		return inherit(DeeObject_CallAttrStringHash(this, "rindex", _Dee_HashSelect(UINT32_C(0x1eb52bf1), UINT64_C(0xbce198a5867b343c)), 1, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::int_> (rindex)(DeeObject *elem, DeeObject *key_or_start) {
		DeeObject *args[2];
		args[0] = elem;
		args[1] = key_or_start;
		return inherit(DeeObject_CallAttrStringHash(this, "rindex", _Dee_HashSelect(UINT32_C(0x1eb52bf1), UINT64_C(0xbce198a5867b343c)), 2, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3)) Ref<deemon::int_> (rindex)(DeeObject *elem, DeeObject *start, DeeObject *key_or_end) {
		DeeObject *args[3];
		args[0] = elem;
		args[1] = start;
		args[2] = key_or_end;
		return inherit(DeeObject_CallAttrStringHash(this, "rindex", _Dee_HashSelect(UINT32_C(0x1eb52bf1), UINT64_C(0xbce198a5867b343c)), 3, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3, 4)) Ref<deemon::int_> (rindex)(DeeObject *elem, DeeObject *start, DeeObject *end, DeeObject *key) {
		DeeObject *args[4];
		args[0] = elem;
		args[1] = start;
		args[2] = end;
		args[3] = key;
		return inherit(DeeObject_CallAttrStringHash(this, "rindex", _Dee_HashSelect(UINT32_C(0x1eb52bf1), UINT64_C(0xbce198a5867b343c)), 4, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::int_> (rindex)(DeeObject *elem, DeeObject *start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rindex", _Dee_HashSelect(UINT32_C(0x1eb52bf1), UINT64_C(0xbce198a5867b343c)), "oo" DEE_PCKdSIZ, elem, start, end));
	}
	WUNUSED NONNULL_CXX((1, 2, 4)) Ref<deemon::int_> (rindex)(DeeObject *elem, DeeObject *start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rindex", _Dee_HashSelect(UINT32_C(0x1eb52bf1), UINT64_C(0xbce198a5867b343c)), "oo" DEE_PCKdSIZ "o", elem, start, end, key));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::int_> (rindex)(DeeObject *elem, DeeObject *start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rindex", _Dee_HashSelect(UINT32_C(0x1eb52bf1), UINT64_C(0xbce198a5867b343c)), "oo" DEE_PCKuSIZ, elem, start, end));
	}
	WUNUSED NONNULL_CXX((1, 2, 4)) Ref<deemon::int_> (rindex)(DeeObject *elem, DeeObject *start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rindex", _Dee_HashSelect(UINT32_C(0x1eb52bf1), UINT64_C(0xbce198a5867b343c)), "oo" DEE_PCKuSIZ "o", elem, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (rindex)(DeeObject *elem, Dee_ssize_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rindex", _Dee_HashSelect(UINT32_C(0x1eb52bf1), UINT64_C(0xbce198a5867b343c)), "o" DEE_PCKdSIZ, elem, start));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::int_> (rindex)(DeeObject *elem, Dee_ssize_t start, DeeObject *key_or_end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rindex", _Dee_HashSelect(UINT32_C(0x1eb52bf1), UINT64_C(0xbce198a5867b343c)), "o" DEE_PCKdSIZ "o", elem, start, key_or_end));
	}
	WUNUSED NONNULL_CXX((1, 3, 4)) Ref<deemon::int_> (rindex)(DeeObject *elem, Dee_ssize_t start, DeeObject *end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rindex", _Dee_HashSelect(UINT32_C(0x1eb52bf1), UINT64_C(0xbce198a5867b343c)), "o" DEE_PCKdSIZ "oo", elem, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (rindex)(DeeObject *elem, Dee_ssize_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rindex", _Dee_HashSelect(UINT32_C(0x1eb52bf1), UINT64_C(0xbce198a5867b343c)), "o" DEE_PCKdSIZ DEE_PCKdSIZ, elem, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::int_> (rindex)(DeeObject *elem, Dee_ssize_t start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rindex", _Dee_HashSelect(UINT32_C(0x1eb52bf1), UINT64_C(0xbce198a5867b343c)), "o" DEE_PCKdSIZ DEE_PCKdSIZ "o", elem, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (rindex)(DeeObject *elem, Dee_ssize_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rindex", _Dee_HashSelect(UINT32_C(0x1eb52bf1), UINT64_C(0xbce198a5867b343c)), "o" DEE_PCKdSIZ DEE_PCKuSIZ, elem, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::int_> (rindex)(DeeObject *elem, Dee_ssize_t start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rindex", _Dee_HashSelect(UINT32_C(0x1eb52bf1), UINT64_C(0xbce198a5867b343c)), "o" DEE_PCKdSIZ DEE_PCKuSIZ "o", elem, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (rindex)(DeeObject *elem, size_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rindex", _Dee_HashSelect(UINT32_C(0x1eb52bf1), UINT64_C(0xbce198a5867b343c)), "o" DEE_PCKuSIZ, elem, start));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::int_> (rindex)(DeeObject *elem, size_t start, DeeObject *key_or_end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rindex", _Dee_HashSelect(UINT32_C(0x1eb52bf1), UINT64_C(0xbce198a5867b343c)), "o" DEE_PCKuSIZ "o", elem, start, key_or_end));
	}
	WUNUSED NONNULL_CXX((1, 3, 4)) Ref<deemon::int_> (rindex)(DeeObject *elem, size_t start, DeeObject *end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rindex", _Dee_HashSelect(UINT32_C(0x1eb52bf1), UINT64_C(0xbce198a5867b343c)), "o" DEE_PCKuSIZ "oo", elem, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (rindex)(DeeObject *elem, size_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rindex", _Dee_HashSelect(UINT32_C(0x1eb52bf1), UINT64_C(0xbce198a5867b343c)), "o" DEE_PCKuSIZ DEE_PCKdSIZ, elem, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::int_> (rindex)(DeeObject *elem, size_t start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rindex", _Dee_HashSelect(UINT32_C(0x1eb52bf1), UINT64_C(0xbce198a5867b343c)), "o" DEE_PCKuSIZ DEE_PCKdSIZ "o", elem, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (rindex)(DeeObject *elem, size_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rindex", _Dee_HashSelect(UINT32_C(0x1eb52bf1), UINT64_C(0xbce198a5867b343c)), "o" DEE_PCKuSIZ DEE_PCKuSIZ, elem, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::int_> (rindex)(DeeObject *elem, size_t start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rindex", _Dee_HashSelect(UINT32_C(0x1eb52bf1), UINT64_C(0xbce198a5867b343c)), "o" DEE_PCKuSIZ DEE_PCKuSIZ "o", elem, start, end, key));
	}
	WUNUSED Ref<Sequence<T> > (sorted)() {
		return inherit(DeeObject_CallAttrStringHash(this, "sorted", _Dee_HashSelect(UINT32_C(0x93fb6d4), UINT64_C(0x2fc60c43cfaf0860)), 0, NULL));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Sequence<T> > (sorted)(DeeObject *key) {
		DeeObject *args[1];
		args[0] = key;
		return inherit(DeeObject_CallAttrStringHash(this, "sorted", _Dee_HashSelect(UINT32_C(0x93fb6d4), UINT64_C(0x2fc60c43cfaf0860)), 1, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Sequence<Sequence<T> > > (segments)(DeeObject *segment_size) {
		DeeObject *args[1];
		args[0] = segment_size;
		return inherit(DeeObject_CallAttrStringHash(this, "segments", _Dee_HashSelect(UINT32_C(0x20acb0bd), UINT64_C(0x8554d160c212a46a)), 1, args));
	}
	WUNUSED Ref<Sequence<Sequence<T> > > (segments)(Dee_ssize_t segment_size) {
		return inherit(DeeObject_CallAttrStringHashf(this, "segments", _Dee_HashSelect(UINT32_C(0x20acb0bd), UINT64_C(0x8554d160c212a46a)),  DEE_PCKdSIZ, segment_size));
	}
	WUNUSED Ref<Sequence<Sequence<T> > > (segments)(size_t segment_size) {
		return inherit(DeeObject_CallAttrStringHashf(this, "segments", _Dee_HashSelect(UINT32_C(0x20acb0bd), UINT64_C(0x8554d160c212a46a)),  DEE_PCKuSIZ, segment_size));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Sequence<Sequence<T> > > (distribute)(DeeObject *bucket_count) {
		DeeObject *args[1];
		args[0] = bucket_count;
		return inherit(DeeObject_CallAttrStringHash(this, "distribute", _Dee_HashSelect(UINT32_C(0xfe7253a7), UINT64_C(0x63150bb7d5354c6f)), 1, args));
	}
	WUNUSED Ref<Sequence<Sequence<T> > > (distribute)(Dee_ssize_t bucket_count) {
		return inherit(DeeObject_CallAttrStringHashf(this, "distribute", _Dee_HashSelect(UINT32_C(0xfe7253a7), UINT64_C(0x63150bb7d5354c6f)),  DEE_PCKdSIZ, bucket_count));
	}
	WUNUSED Ref<Sequence<Sequence<T> > > (distribute)(size_t bucket_count) {
		return inherit(DeeObject_CallAttrStringHashf(this, "distribute", _Dee_HashSelect(UINT32_C(0xfe7253a7), UINT64_C(0x63150bb7d5354c6f)),  DEE_PCKuSIZ, bucket_count));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Sequence<Sequence<T> > > (combinations)(DeeObject *r) {
		DeeObject *args[1];
		args[0] = r;
		return inherit(DeeObject_CallAttrStringHash(this, "combinations", _Dee_HashSelect(UINT32_C(0x184d9b51), UINT64_C(0x3e5802b7656c4900)), 1, args));
	}
	WUNUSED Ref<Sequence<Sequence<T> > > (combinations)(Dee_ssize_t r) {
		return inherit(DeeObject_CallAttrStringHashf(this, "combinations", _Dee_HashSelect(UINT32_C(0x184d9b51), UINT64_C(0x3e5802b7656c4900)),  DEE_PCKdSIZ, r));
	}
	WUNUSED Ref<Sequence<Sequence<T> > > (combinations)(size_t r) {
		return inherit(DeeObject_CallAttrStringHashf(this, "combinations", _Dee_HashSelect(UINT32_C(0x184d9b51), UINT64_C(0x3e5802b7656c4900)),  DEE_PCKuSIZ, r));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Sequence<Sequence<T> > > (repeatcombinations)(DeeObject *r) {
		DeeObject *args[1];
		args[0] = r;
		return inherit(DeeObject_CallAttrStringHash(this, "repeatcombinations", _Dee_HashSelect(UINT32_C(0xa3bc4ae1), UINT64_C(0x7ef1d21507ad27f5)), 1, args));
	}
	WUNUSED Ref<Sequence<Sequence<T> > > (repeatcombinations)(Dee_ssize_t r) {
		return inherit(DeeObject_CallAttrStringHashf(this, "repeatcombinations", _Dee_HashSelect(UINT32_C(0xa3bc4ae1), UINT64_C(0x7ef1d21507ad27f5)),  DEE_PCKdSIZ, r));
	}
	WUNUSED Ref<Sequence<Sequence<T> > > (repeatcombinations)(size_t r) {
		return inherit(DeeObject_CallAttrStringHashf(this, "repeatcombinations", _Dee_HashSelect(UINT32_C(0xa3bc4ae1), UINT64_C(0x7ef1d21507ad27f5)),  DEE_PCKuSIZ, r));
	}
	WUNUSED Ref<Sequence<Sequence<T> > > (permutations)() {
		return inherit(DeeObject_CallAttrStringHash(this, "permutations", _Dee_HashSelect(UINT32_C(0x923fbd44), UINT64_C(0x498779abec4910f6)), 0, NULL));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Sequence<Sequence<T> > > (permutations)(DeeObject *r) {
		DeeObject *args[1];
		args[0] = r;
		return inherit(DeeObject_CallAttrStringHash(this, "permutations", _Dee_HashSelect(UINT32_C(0x923fbd44), UINT64_C(0x498779abec4910f6)), 1, args));
	}
	WUNUSED Ref<Sequence<Sequence<T> > > (permutations)(Dee_ssize_t r) {
		return inherit(DeeObject_CallAttrStringHashf(this, "permutations", _Dee_HashSelect(UINT32_C(0x923fbd44), UINT64_C(0x498779abec4910f6)),  DEE_PCKdSIZ, r));
	}
	WUNUSED Ref<Sequence<Sequence<T> > > (permutations)(size_t r) {
		return inherit(DeeObject_CallAttrStringHashf(this, "permutations", _Dee_HashSelect(UINT32_C(0x923fbd44), UINT64_C(0x498779abec4910f6)),  DEE_PCKuSIZ, r));
	}
	NONNULL_CXX((1, 2)) void (insert)(DeeObject *index, DeeObject *item) {
		DeeObject *args[2];
		args[0] = index;
		args[1] = item;
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "insert", _Dee_HashSelect(UINT32_C(0x71d74a66), UINT64_C(0x5e168c86241590d7)), 2, args)));
	}
	NONNULL_CXX((2)) void (insert)(Dee_ssize_t index, DeeObject *item) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "insert", _Dee_HashSelect(UINT32_C(0x71d74a66), UINT64_C(0x5e168c86241590d7)),  DEE_PCKdSIZ "o", index, item)));
	}
	NONNULL_CXX((2)) void (insert)(size_t index, DeeObject *item) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "insert", _Dee_HashSelect(UINT32_C(0x71d74a66), UINT64_C(0x5e168c86241590d7)),  DEE_PCKuSIZ "o", index, item)));
	}
	NONNULL_CXX((1, 2)) void (insertall)(DeeObject *index, DeeObject *items) {
		DeeObject *args[2];
		args[0] = index;
		args[1] = items;
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "insertall", _Dee_HashSelect(UINT32_C(0xbf9bc3a9), UINT64_C(0x4f85971d093a27f2)), 2, args)));
	}
	NONNULL_CXX((2)) void (insertall)(Dee_ssize_t index, DeeObject *items) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "insertall", _Dee_HashSelect(UINT32_C(0xbf9bc3a9), UINT64_C(0x4f85971d093a27f2)),  DEE_PCKdSIZ "o", index, items)));
	}
	NONNULL_CXX((2)) void (insertall)(size_t index, DeeObject *items) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "insertall", _Dee_HashSelect(UINT32_C(0xbf9bc3a9), UINT64_C(0x4f85971d093a27f2)),  DEE_PCKuSIZ "o", index, items)));
	}
	NONNULL_CXX((1)) void (append)(DeeObject *item) {
		DeeObject *args[1];
		args[0] = item;
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "append", _Dee_HashSelect(UINT32_C(0x5f19594f), UINT64_C(0x8c2b7c1aba65d5ee)), 1, args)));
	}
	NONNULL_CXX((1)) void (extend)(DeeObject *items) {
		DeeObject *args[1];
		args[0] = items;
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "extend", _Dee_HashSelect(UINT32_C(0x960b75e7), UINT64_C(0xba076858e3adb055)), 1, args)));
	}
	NONNULL_CXX((1)) void (erase)(DeeObject *index) {
		DeeObject *args[1];
		args[0] = index;
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "erase", _Dee_HashSelect(UINT32_C(0x6f5916cf), UINT64_C(0x65f9c8b6514af4e5)), 1, args)));
	}
	NONNULL_CXX((1, 2)) void (erase)(DeeObject *index, DeeObject *count) {
		DeeObject *args[2];
		args[0] = index;
		args[1] = count;
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "erase", _Dee_HashSelect(UINT32_C(0x6f5916cf), UINT64_C(0x65f9c8b6514af4e5)), 2, args)));
	}
	NONNULL_CXX((1)) void (erase)(DeeObject *index, Dee_ssize_t count) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "erase", _Dee_HashSelect(UINT32_C(0x6f5916cf), UINT64_C(0x65f9c8b6514af4e5)), "o" DEE_PCKdSIZ, index, count)));
	}
	NONNULL_CXX((1)) void (erase)(DeeObject *index, size_t count) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "erase", _Dee_HashSelect(UINT32_C(0x6f5916cf), UINT64_C(0x65f9c8b6514af4e5)), "o" DEE_PCKuSIZ, index, count)));
	}
	void (erase)(Dee_ssize_t index) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "erase", _Dee_HashSelect(UINT32_C(0x6f5916cf), UINT64_C(0x65f9c8b6514af4e5)),  DEE_PCKdSIZ, index)));
	}
	NONNULL_CXX((2)) void (erase)(Dee_ssize_t index, DeeObject *count) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "erase", _Dee_HashSelect(UINT32_C(0x6f5916cf), UINT64_C(0x65f9c8b6514af4e5)),  DEE_PCKdSIZ "o", index, count)));
	}
	void (erase)(Dee_ssize_t index, Dee_ssize_t count) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "erase", _Dee_HashSelect(UINT32_C(0x6f5916cf), UINT64_C(0x65f9c8b6514af4e5)),  DEE_PCKdSIZ DEE_PCKdSIZ, index, count)));
	}
	void (erase)(Dee_ssize_t index, size_t count) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "erase", _Dee_HashSelect(UINT32_C(0x6f5916cf), UINT64_C(0x65f9c8b6514af4e5)),  DEE_PCKdSIZ DEE_PCKuSIZ, index, count)));
	}
	void (erase)(size_t index) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "erase", _Dee_HashSelect(UINT32_C(0x6f5916cf), UINT64_C(0x65f9c8b6514af4e5)),  DEE_PCKuSIZ, index)));
	}
	NONNULL_CXX((2)) void (erase)(size_t index, DeeObject *count) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "erase", _Dee_HashSelect(UINT32_C(0x6f5916cf), UINT64_C(0x65f9c8b6514af4e5)),  DEE_PCKuSIZ "o", index, count)));
	}
	void (erase)(size_t index, Dee_ssize_t count) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "erase", _Dee_HashSelect(UINT32_C(0x6f5916cf), UINT64_C(0x65f9c8b6514af4e5)),  DEE_PCKuSIZ DEE_PCKdSIZ, index, count)));
	}
	void (erase)(size_t index, size_t count) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "erase", _Dee_HashSelect(UINT32_C(0x6f5916cf), UINT64_C(0x65f9c8b6514af4e5)),  DEE_PCKuSIZ DEE_PCKuSIZ, index, count)));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<T> (xch)(DeeObject *index, DeeObject *value) {
		DeeObject *args[2];
		args[0] = index;
		args[1] = value;
		return inherit(DeeObject_CallAttrStringHash(this, "xch", _Dee_HashSelect(UINT32_C(0x818ce38a), UINT64_C(0x6bb37305be1b0321)), 2, args));
	}
	WUNUSED NONNULL_CXX((2)) Ref<T> (xch)(Dee_ssize_t index, DeeObject *value) {
		return inherit(DeeObject_CallAttrStringHashf(this, "xch", _Dee_HashSelect(UINT32_C(0x818ce38a), UINT64_C(0x6bb37305be1b0321)),  DEE_PCKdSIZ "o", index, value));
	}
	WUNUSED NONNULL_CXX((2)) Ref<T> (xch)(size_t index, DeeObject *value) {
		return inherit(DeeObject_CallAttrStringHashf(this, "xch", _Dee_HashSelect(UINT32_C(0x818ce38a), UINT64_C(0x6bb37305be1b0321)),  DEE_PCKuSIZ "o", index, value));
	}
	WUNUSED Ref<T> (pop)() {
		return inherit(DeeObject_CallAttrStringHash(this, "pop", _Dee_HashSelect(UINT32_C(0x960361ff), UINT64_C(0x666fb01461b0a0eb)), 0, NULL));
	}
	WUNUSED NONNULL_CXX((1)) Ref<T> (pop)(DeeObject *index) {
		DeeObject *args[1];
		args[0] = index;
		return inherit(DeeObject_CallAttrStringHash(this, "pop", _Dee_HashSelect(UINT32_C(0x960361ff), UINT64_C(0x666fb01461b0a0eb)), 1, args));
	}
	WUNUSED Ref<T> (pop)(Dee_ssize_t index) {
		return inherit(DeeObject_CallAttrStringHashf(this, "pop", _Dee_HashSelect(UINT32_C(0x960361ff), UINT64_C(0x666fb01461b0a0eb)),  DEE_PCKdSIZ, index));
	}
	WUNUSED Ref<T> (pop)(size_t index) {
		return inherit(DeeObject_CallAttrStringHashf(this, "pop", _Dee_HashSelect(UINT32_C(0x960361ff), UINT64_C(0x666fb01461b0a0eb)),  DEE_PCKuSIZ, index));
	}
	NONNULL_CXX((1)) void (pushfront)(DeeObject *item) {
		DeeObject *args[1];
		args[0] = item;
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "pushfront", _Dee_HashSelect(UINT32_C(0xc682cfdf), UINT64_C(0x5933eb9a387ff882)), 1, args)));
	}
	NONNULL_CXX((1)) void (pushback)(DeeObject *item) {
		DeeObject *args[1];
		args[0] = item;
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "pushback", _Dee_HashSelect(UINT32_C(0xad1e1509), UINT64_C(0x4cfafd84a12923bd)), 1, args)));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (remove)(DeeObject *elem) {
		DeeObject *args[1];
		args[0] = elem;
		return inherit(DeeObject_CallAttrStringHash(this, "remove", _Dee_HashSelect(UINT32_C(0x3d2727dd), UINT64_C(0xe9f313a03e2051a)), 1, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::bool_> (remove)(DeeObject *elem, DeeObject *key_or_start) {
		DeeObject *args[2];
		args[0] = elem;
		args[1] = key_or_start;
		return inherit(DeeObject_CallAttrStringHash(this, "remove", _Dee_HashSelect(UINT32_C(0x3d2727dd), UINT64_C(0xe9f313a03e2051a)), 2, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3)) Ref<deemon::bool_> (remove)(DeeObject *elem, DeeObject *start, DeeObject *key_or_end) {
		DeeObject *args[3];
		args[0] = elem;
		args[1] = start;
		args[2] = key_or_end;
		return inherit(DeeObject_CallAttrStringHash(this, "remove", _Dee_HashSelect(UINT32_C(0x3d2727dd), UINT64_C(0xe9f313a03e2051a)), 3, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3, 4)) Ref<deemon::bool_> (remove)(DeeObject *elem, DeeObject *start, DeeObject *end, DeeObject *key) {
		DeeObject *args[4];
		args[0] = elem;
		args[1] = start;
		args[2] = end;
		args[3] = key;
		return inherit(DeeObject_CallAttrStringHash(this, "remove", _Dee_HashSelect(UINT32_C(0x3d2727dd), UINT64_C(0xe9f313a03e2051a)), 4, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::bool_> (remove)(DeeObject *elem, DeeObject *start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "remove", _Dee_HashSelect(UINT32_C(0x3d2727dd), UINT64_C(0xe9f313a03e2051a)), "oo" DEE_PCKdSIZ, elem, start, end));
	}
	WUNUSED NONNULL_CXX((1, 2, 4)) Ref<deemon::bool_> (remove)(DeeObject *elem, DeeObject *start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "remove", _Dee_HashSelect(UINT32_C(0x3d2727dd), UINT64_C(0xe9f313a03e2051a)), "oo" DEE_PCKdSIZ "o", elem, start, end, key));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::bool_> (remove)(DeeObject *elem, DeeObject *start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "remove", _Dee_HashSelect(UINT32_C(0x3d2727dd), UINT64_C(0xe9f313a03e2051a)), "oo" DEE_PCKuSIZ, elem, start, end));
	}
	WUNUSED NONNULL_CXX((1, 2, 4)) Ref<deemon::bool_> (remove)(DeeObject *elem, DeeObject *start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "remove", _Dee_HashSelect(UINT32_C(0x3d2727dd), UINT64_C(0xe9f313a03e2051a)), "oo" DEE_PCKuSIZ "o", elem, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (remove)(DeeObject *elem, Dee_ssize_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "remove", _Dee_HashSelect(UINT32_C(0x3d2727dd), UINT64_C(0xe9f313a03e2051a)), "o" DEE_PCKdSIZ, elem, start));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::bool_> (remove)(DeeObject *elem, Dee_ssize_t start, DeeObject *key_or_end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "remove", _Dee_HashSelect(UINT32_C(0x3d2727dd), UINT64_C(0xe9f313a03e2051a)), "o" DEE_PCKdSIZ "o", elem, start, key_or_end));
	}
	WUNUSED NONNULL_CXX((1, 3, 4)) Ref<deemon::bool_> (remove)(DeeObject *elem, Dee_ssize_t start, DeeObject *end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "remove", _Dee_HashSelect(UINT32_C(0x3d2727dd), UINT64_C(0xe9f313a03e2051a)), "o" DEE_PCKdSIZ "oo", elem, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (remove)(DeeObject *elem, Dee_ssize_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "remove", _Dee_HashSelect(UINT32_C(0x3d2727dd), UINT64_C(0xe9f313a03e2051a)), "o" DEE_PCKdSIZ DEE_PCKdSIZ, elem, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::bool_> (remove)(DeeObject *elem, Dee_ssize_t start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "remove", _Dee_HashSelect(UINT32_C(0x3d2727dd), UINT64_C(0xe9f313a03e2051a)), "o" DEE_PCKdSIZ DEE_PCKdSIZ "o", elem, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (remove)(DeeObject *elem, Dee_ssize_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "remove", _Dee_HashSelect(UINT32_C(0x3d2727dd), UINT64_C(0xe9f313a03e2051a)), "o" DEE_PCKdSIZ DEE_PCKuSIZ, elem, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::bool_> (remove)(DeeObject *elem, Dee_ssize_t start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "remove", _Dee_HashSelect(UINT32_C(0x3d2727dd), UINT64_C(0xe9f313a03e2051a)), "o" DEE_PCKdSIZ DEE_PCKuSIZ "o", elem, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (remove)(DeeObject *elem, size_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "remove", _Dee_HashSelect(UINT32_C(0x3d2727dd), UINT64_C(0xe9f313a03e2051a)), "o" DEE_PCKuSIZ, elem, start));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::bool_> (remove)(DeeObject *elem, size_t start, DeeObject *key_or_end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "remove", _Dee_HashSelect(UINT32_C(0x3d2727dd), UINT64_C(0xe9f313a03e2051a)), "o" DEE_PCKuSIZ "o", elem, start, key_or_end));
	}
	WUNUSED NONNULL_CXX((1, 3, 4)) Ref<deemon::bool_> (remove)(DeeObject *elem, size_t start, DeeObject *end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "remove", _Dee_HashSelect(UINT32_C(0x3d2727dd), UINT64_C(0xe9f313a03e2051a)), "o" DEE_PCKuSIZ "oo", elem, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (remove)(DeeObject *elem, size_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "remove", _Dee_HashSelect(UINT32_C(0x3d2727dd), UINT64_C(0xe9f313a03e2051a)), "o" DEE_PCKuSIZ DEE_PCKdSIZ, elem, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::bool_> (remove)(DeeObject *elem, size_t start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "remove", _Dee_HashSelect(UINT32_C(0x3d2727dd), UINT64_C(0xe9f313a03e2051a)), "o" DEE_PCKuSIZ DEE_PCKdSIZ "o", elem, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (remove)(DeeObject *elem, size_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "remove", _Dee_HashSelect(UINT32_C(0x3d2727dd), UINT64_C(0xe9f313a03e2051a)), "o" DEE_PCKuSIZ DEE_PCKuSIZ, elem, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::bool_> (remove)(DeeObject *elem, size_t start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "remove", _Dee_HashSelect(UINT32_C(0x3d2727dd), UINT64_C(0xe9f313a03e2051a)), "o" DEE_PCKuSIZ DEE_PCKuSIZ "o", elem, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (rremove)(DeeObject *elem) {
		DeeObject *args[1];
		args[0] = elem;
		return inherit(DeeObject_CallAttrStringHash(this, "rremove", _Dee_HashSelect(UINT32_C(0x37ef1152), UINT64_C(0x199975a7908f6d6)), 1, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::bool_> (rremove)(DeeObject *elem, DeeObject *key_or_start) {
		DeeObject *args[2];
		args[0] = elem;
		args[1] = key_or_start;
		return inherit(DeeObject_CallAttrStringHash(this, "rremove", _Dee_HashSelect(UINT32_C(0x37ef1152), UINT64_C(0x199975a7908f6d6)), 2, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3)) Ref<deemon::bool_> (rremove)(DeeObject *elem, DeeObject *start, DeeObject *key_or_end) {
		DeeObject *args[3];
		args[0] = elem;
		args[1] = start;
		args[2] = key_or_end;
		return inherit(DeeObject_CallAttrStringHash(this, "rremove", _Dee_HashSelect(UINT32_C(0x37ef1152), UINT64_C(0x199975a7908f6d6)), 3, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3, 4)) Ref<deemon::bool_> (rremove)(DeeObject *elem, DeeObject *start, DeeObject *end, DeeObject *key) {
		DeeObject *args[4];
		args[0] = elem;
		args[1] = start;
		args[2] = end;
		args[3] = key;
		return inherit(DeeObject_CallAttrStringHash(this, "rremove", _Dee_HashSelect(UINT32_C(0x37ef1152), UINT64_C(0x199975a7908f6d6)), 4, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::bool_> (rremove)(DeeObject *elem, DeeObject *start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rremove", _Dee_HashSelect(UINT32_C(0x37ef1152), UINT64_C(0x199975a7908f6d6)), "oo" DEE_PCKdSIZ, elem, start, end));
	}
	WUNUSED NONNULL_CXX((1, 2, 4)) Ref<deemon::bool_> (rremove)(DeeObject *elem, DeeObject *start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rremove", _Dee_HashSelect(UINT32_C(0x37ef1152), UINT64_C(0x199975a7908f6d6)), "oo" DEE_PCKdSIZ "o", elem, start, end, key));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::bool_> (rremove)(DeeObject *elem, DeeObject *start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rremove", _Dee_HashSelect(UINT32_C(0x37ef1152), UINT64_C(0x199975a7908f6d6)), "oo" DEE_PCKuSIZ, elem, start, end));
	}
	WUNUSED NONNULL_CXX((1, 2, 4)) Ref<deemon::bool_> (rremove)(DeeObject *elem, DeeObject *start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rremove", _Dee_HashSelect(UINT32_C(0x37ef1152), UINT64_C(0x199975a7908f6d6)), "oo" DEE_PCKuSIZ "o", elem, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (rremove)(DeeObject *elem, Dee_ssize_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rremove", _Dee_HashSelect(UINT32_C(0x37ef1152), UINT64_C(0x199975a7908f6d6)), "o" DEE_PCKdSIZ, elem, start));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::bool_> (rremove)(DeeObject *elem, Dee_ssize_t start, DeeObject *key_or_end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rremove", _Dee_HashSelect(UINT32_C(0x37ef1152), UINT64_C(0x199975a7908f6d6)), "o" DEE_PCKdSIZ "o", elem, start, key_or_end));
	}
	WUNUSED NONNULL_CXX((1, 3, 4)) Ref<deemon::bool_> (rremove)(DeeObject *elem, Dee_ssize_t start, DeeObject *end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rremove", _Dee_HashSelect(UINT32_C(0x37ef1152), UINT64_C(0x199975a7908f6d6)), "o" DEE_PCKdSIZ "oo", elem, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (rremove)(DeeObject *elem, Dee_ssize_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rremove", _Dee_HashSelect(UINT32_C(0x37ef1152), UINT64_C(0x199975a7908f6d6)), "o" DEE_PCKdSIZ DEE_PCKdSIZ, elem, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::bool_> (rremove)(DeeObject *elem, Dee_ssize_t start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rremove", _Dee_HashSelect(UINT32_C(0x37ef1152), UINT64_C(0x199975a7908f6d6)), "o" DEE_PCKdSIZ DEE_PCKdSIZ "o", elem, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (rremove)(DeeObject *elem, Dee_ssize_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rremove", _Dee_HashSelect(UINT32_C(0x37ef1152), UINT64_C(0x199975a7908f6d6)), "o" DEE_PCKdSIZ DEE_PCKuSIZ, elem, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::bool_> (rremove)(DeeObject *elem, Dee_ssize_t start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rremove", _Dee_HashSelect(UINT32_C(0x37ef1152), UINT64_C(0x199975a7908f6d6)), "o" DEE_PCKdSIZ DEE_PCKuSIZ "o", elem, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (rremove)(DeeObject *elem, size_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rremove", _Dee_HashSelect(UINT32_C(0x37ef1152), UINT64_C(0x199975a7908f6d6)), "o" DEE_PCKuSIZ, elem, start));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::bool_> (rremove)(DeeObject *elem, size_t start, DeeObject *key_or_end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rremove", _Dee_HashSelect(UINT32_C(0x37ef1152), UINT64_C(0x199975a7908f6d6)), "o" DEE_PCKuSIZ "o", elem, start, key_or_end));
	}
	WUNUSED NONNULL_CXX((1, 3, 4)) Ref<deemon::bool_> (rremove)(DeeObject *elem, size_t start, DeeObject *end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rremove", _Dee_HashSelect(UINT32_C(0x37ef1152), UINT64_C(0x199975a7908f6d6)), "o" DEE_PCKuSIZ "oo", elem, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (rremove)(DeeObject *elem, size_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rremove", _Dee_HashSelect(UINT32_C(0x37ef1152), UINT64_C(0x199975a7908f6d6)), "o" DEE_PCKuSIZ DEE_PCKdSIZ, elem, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::bool_> (rremove)(DeeObject *elem, size_t start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rremove", _Dee_HashSelect(UINT32_C(0x37ef1152), UINT64_C(0x199975a7908f6d6)), "o" DEE_PCKuSIZ DEE_PCKdSIZ "o", elem, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (rremove)(DeeObject *elem, size_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rremove", _Dee_HashSelect(UINT32_C(0x37ef1152), UINT64_C(0x199975a7908f6d6)), "o" DEE_PCKuSIZ DEE_PCKuSIZ, elem, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::bool_> (rremove)(DeeObject *elem, size_t start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rremove", _Dee_HashSelect(UINT32_C(0x37ef1152), UINT64_C(0x199975a7908f6d6)), "o" DEE_PCKuSIZ DEE_PCKuSIZ "o", elem, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (removeall)(DeeObject *elem) {
		DeeObject *args[1];
		args[0] = elem;
		return inherit(DeeObject_CallAttrStringHash(this, "removeall", _Dee_HashSelect(UINT32_C(0x902407ed), UINT64_C(0x97879af70abc9349)), 1, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::int_> (removeall)(DeeObject *elem, DeeObject *key_or_start) {
		DeeObject *args[2];
		args[0] = elem;
		args[1] = key_or_start;
		return inherit(DeeObject_CallAttrStringHash(this, "removeall", _Dee_HashSelect(UINT32_C(0x902407ed), UINT64_C(0x97879af70abc9349)), 2, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3)) Ref<deemon::int_> (removeall)(DeeObject *elem, DeeObject *start, DeeObject *key_or_end) {
		DeeObject *args[3];
		args[0] = elem;
		args[1] = start;
		args[2] = key_or_end;
		return inherit(DeeObject_CallAttrStringHash(this, "removeall", _Dee_HashSelect(UINT32_C(0x902407ed), UINT64_C(0x97879af70abc9349)), 3, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3, 4)) Ref<deemon::int_> (removeall)(DeeObject *elem, DeeObject *start, DeeObject *end, DeeObject *key) {
		DeeObject *args[4];
		args[0] = elem;
		args[1] = start;
		args[2] = end;
		args[3] = key;
		return inherit(DeeObject_CallAttrStringHash(this, "removeall", _Dee_HashSelect(UINT32_C(0x902407ed), UINT64_C(0x97879af70abc9349)), 4, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::int_> (removeall)(DeeObject *elem, DeeObject *start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelect(UINT32_C(0x902407ed), UINT64_C(0x97879af70abc9349)), "oo" DEE_PCKdSIZ, elem, start, end));
	}
	WUNUSED NONNULL_CXX((1, 2, 4)) Ref<deemon::int_> (removeall)(DeeObject *elem, DeeObject *start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelect(UINT32_C(0x902407ed), UINT64_C(0x97879af70abc9349)), "oo" DEE_PCKdSIZ "o", elem, start, end, key));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::int_> (removeall)(DeeObject *elem, DeeObject *start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelect(UINT32_C(0x902407ed), UINT64_C(0x97879af70abc9349)), "oo" DEE_PCKuSIZ, elem, start, end));
	}
	WUNUSED NONNULL_CXX((1, 2, 4)) Ref<deemon::int_> (removeall)(DeeObject *elem, DeeObject *start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelect(UINT32_C(0x902407ed), UINT64_C(0x97879af70abc9349)), "oo" DEE_PCKuSIZ "o", elem, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (removeall)(DeeObject *elem, Dee_ssize_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelect(UINT32_C(0x902407ed), UINT64_C(0x97879af70abc9349)), "o" DEE_PCKdSIZ, elem, start));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::int_> (removeall)(DeeObject *elem, Dee_ssize_t start, DeeObject *key_or_end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelect(UINT32_C(0x902407ed), UINT64_C(0x97879af70abc9349)), "o" DEE_PCKdSIZ "o", elem, start, key_or_end));
	}
	WUNUSED NONNULL_CXX((1, 3, 4)) Ref<deemon::int_> (removeall)(DeeObject *elem, Dee_ssize_t start, DeeObject *end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelect(UINT32_C(0x902407ed), UINT64_C(0x97879af70abc9349)), "o" DEE_PCKdSIZ "oo", elem, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (removeall)(DeeObject *elem, Dee_ssize_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelect(UINT32_C(0x902407ed), UINT64_C(0x97879af70abc9349)), "o" DEE_PCKdSIZ DEE_PCKdSIZ, elem, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::int_> (removeall)(DeeObject *elem, Dee_ssize_t start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelect(UINT32_C(0x902407ed), UINT64_C(0x97879af70abc9349)), "o" DEE_PCKdSIZ DEE_PCKdSIZ "o", elem, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (removeall)(DeeObject *elem, Dee_ssize_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelect(UINT32_C(0x902407ed), UINT64_C(0x97879af70abc9349)), "o" DEE_PCKdSIZ DEE_PCKuSIZ, elem, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::int_> (removeall)(DeeObject *elem, Dee_ssize_t start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelect(UINT32_C(0x902407ed), UINT64_C(0x97879af70abc9349)), "o" DEE_PCKdSIZ DEE_PCKuSIZ "o", elem, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (removeall)(DeeObject *elem, size_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelect(UINT32_C(0x902407ed), UINT64_C(0x97879af70abc9349)), "o" DEE_PCKuSIZ, elem, start));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::int_> (removeall)(DeeObject *elem, size_t start, DeeObject *key_or_end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelect(UINT32_C(0x902407ed), UINT64_C(0x97879af70abc9349)), "o" DEE_PCKuSIZ "o", elem, start, key_or_end));
	}
	WUNUSED NONNULL_CXX((1, 3, 4)) Ref<deemon::int_> (removeall)(DeeObject *elem, size_t start, DeeObject *end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelect(UINT32_C(0x902407ed), UINT64_C(0x97879af70abc9349)), "o" DEE_PCKuSIZ "oo", elem, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (removeall)(DeeObject *elem, size_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelect(UINT32_C(0x902407ed), UINT64_C(0x97879af70abc9349)), "o" DEE_PCKuSIZ DEE_PCKdSIZ, elem, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::int_> (removeall)(DeeObject *elem, size_t start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelect(UINT32_C(0x902407ed), UINT64_C(0x97879af70abc9349)), "o" DEE_PCKuSIZ DEE_PCKdSIZ "o", elem, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (removeall)(DeeObject *elem, size_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelect(UINT32_C(0x902407ed), UINT64_C(0x97879af70abc9349)), "o" DEE_PCKuSIZ DEE_PCKuSIZ, elem, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::int_> (removeall)(DeeObject *elem, size_t start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeall", _Dee_HashSelect(UINT32_C(0x902407ed), UINT64_C(0x97879af70abc9349)), "o" DEE_PCKuSIZ DEE_PCKuSIZ "o", elem, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (removeif)(DeeObject *should) {
		DeeObject *args[1];
		args[0] = should;
		return inherit(DeeObject_CallAttrStringHash(this, "removeif", _Dee_HashSelect(UINT32_C(0x156aa732), UINT64_C(0x96ad85f728d8a11e)), 1, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::int_> (removeif)(DeeObject *should, DeeObject *start) {
		DeeObject *args[2];
		args[0] = should;
		args[1] = start;
		return inherit(DeeObject_CallAttrStringHash(this, "removeif", _Dee_HashSelect(UINT32_C(0x156aa732), UINT64_C(0x96ad85f728d8a11e)), 2, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3)) Ref<deemon::int_> (removeif)(DeeObject *should, DeeObject *start, DeeObject *end) {
		DeeObject *args[3];
		args[0] = should;
		args[1] = start;
		args[2] = end;
		return inherit(DeeObject_CallAttrStringHash(this, "removeif", _Dee_HashSelect(UINT32_C(0x156aa732), UINT64_C(0x96ad85f728d8a11e)), 3, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::int_> (removeif)(DeeObject *should, DeeObject *start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeif", _Dee_HashSelect(UINT32_C(0x156aa732), UINT64_C(0x96ad85f728d8a11e)), "oo" DEE_PCKdSIZ, should, start, end));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::int_> (removeif)(DeeObject *should, DeeObject *start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeif", _Dee_HashSelect(UINT32_C(0x156aa732), UINT64_C(0x96ad85f728d8a11e)), "oo" DEE_PCKuSIZ, should, start, end));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (removeif)(DeeObject *should, Dee_ssize_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeif", _Dee_HashSelect(UINT32_C(0x156aa732), UINT64_C(0x96ad85f728d8a11e)), "o" DEE_PCKdSIZ, should, start));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::int_> (removeif)(DeeObject *should, Dee_ssize_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeif", _Dee_HashSelect(UINT32_C(0x156aa732), UINT64_C(0x96ad85f728d8a11e)), "o" DEE_PCKdSIZ "o", should, start, end));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (removeif)(DeeObject *should, Dee_ssize_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeif", _Dee_HashSelect(UINT32_C(0x156aa732), UINT64_C(0x96ad85f728d8a11e)), "o" DEE_PCKdSIZ DEE_PCKdSIZ, should, start, end));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (removeif)(DeeObject *should, Dee_ssize_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeif", _Dee_HashSelect(UINT32_C(0x156aa732), UINT64_C(0x96ad85f728d8a11e)), "o" DEE_PCKdSIZ DEE_PCKuSIZ, should, start, end));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (removeif)(DeeObject *should, size_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeif", _Dee_HashSelect(UINT32_C(0x156aa732), UINT64_C(0x96ad85f728d8a11e)), "o" DEE_PCKuSIZ, should, start));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::int_> (removeif)(DeeObject *should, size_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeif", _Dee_HashSelect(UINT32_C(0x156aa732), UINT64_C(0x96ad85f728d8a11e)), "o" DEE_PCKuSIZ "o", should, start, end));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (removeif)(DeeObject *should, size_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeif", _Dee_HashSelect(UINT32_C(0x156aa732), UINT64_C(0x96ad85f728d8a11e)), "o" DEE_PCKuSIZ DEE_PCKdSIZ, should, start, end));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (removeif)(DeeObject *should, size_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "removeif", _Dee_HashSelect(UINT32_C(0x156aa732), UINT64_C(0x96ad85f728d8a11e)), "o" DEE_PCKuSIZ DEE_PCKuSIZ, should, start, end));
	}
	NONNULL_CXX((1)) void (resize)(DeeObject *int_) {
		DeeObject *args[1];
		args[0] = int_;
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "resize", _Dee_HashSelect(UINT32_C(0x36fcb308), UINT64_C(0x573f3d2e97212b34)), 1, args)));
	}
	NONNULL_CXX((1, 2)) void (resize)(DeeObject *int_, DeeObject *filler) {
		DeeObject *args[2];
		args[0] = int_;
		args[1] = filler;
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "resize", _Dee_HashSelect(UINT32_C(0x36fcb308), UINT64_C(0x573f3d2e97212b34)), 2, args)));
	}
	WUNUSED Ref<deemon::int_> (fill)() {
		return inherit(DeeObject_CallAttrStringHash(this, "fill", _Dee_HashSelect(UINT32_C(0xbd501461), UINT64_C(0x7b3ed649c1abacf4)), 0, NULL));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (fill)(DeeObject *start) {
		DeeObject *args[1];
		args[0] = start;
		return inherit(DeeObject_CallAttrStringHash(this, "fill", _Dee_HashSelect(UINT32_C(0xbd501461), UINT64_C(0x7b3ed649c1abacf4)), 1, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::int_> (fill)(DeeObject *start, DeeObject *end) {
		DeeObject *args[2];
		args[0] = start;
		args[1] = end;
		return inherit(DeeObject_CallAttrStringHash(this, "fill", _Dee_HashSelect(UINT32_C(0xbd501461), UINT64_C(0x7b3ed649c1abacf4)), 2, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3)) Ref<deemon::int_> (fill)(DeeObject *start, DeeObject *end, DeeObject *filler) {
		DeeObject *args[3];
		args[0] = start;
		args[1] = end;
		args[2] = filler;
		return inherit(DeeObject_CallAttrStringHash(this, "fill", _Dee_HashSelect(UINT32_C(0xbd501461), UINT64_C(0x7b3ed649c1abacf4)), 3, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (fill)(DeeObject *start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "fill", _Dee_HashSelect(UINT32_C(0xbd501461), UINT64_C(0x7b3ed649c1abacf4)), "o" DEE_PCKdSIZ, start, end));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::int_> (fill)(DeeObject *start, Dee_ssize_t end, DeeObject *filler) {
		return inherit(DeeObject_CallAttrStringHashf(this, "fill", _Dee_HashSelect(UINT32_C(0xbd501461), UINT64_C(0x7b3ed649c1abacf4)), "o" DEE_PCKdSIZ "o", start, end, filler));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (fill)(DeeObject *start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "fill", _Dee_HashSelect(UINT32_C(0xbd501461), UINT64_C(0x7b3ed649c1abacf4)), "o" DEE_PCKuSIZ, start, end));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::int_> (fill)(DeeObject *start, size_t end, DeeObject *filler) {
		return inherit(DeeObject_CallAttrStringHashf(this, "fill", _Dee_HashSelect(UINT32_C(0xbd501461), UINT64_C(0x7b3ed649c1abacf4)), "o" DEE_PCKuSIZ "o", start, end, filler));
	}
	WUNUSED Ref<deemon::int_> (fill)(Dee_ssize_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "fill", _Dee_HashSelect(UINT32_C(0xbd501461), UINT64_C(0x7b3ed649c1abacf4)),  DEE_PCKdSIZ, start));
	}
	WUNUSED NONNULL_CXX((2)) Ref<deemon::int_> (fill)(Dee_ssize_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "fill", _Dee_HashSelect(UINT32_C(0xbd501461), UINT64_C(0x7b3ed649c1abacf4)),  DEE_PCKdSIZ "o", start, end));
	}
	WUNUSED NONNULL_CXX((2, 3)) Ref<deemon::int_> (fill)(Dee_ssize_t start, DeeObject *end, DeeObject *filler) {
		return inherit(DeeObject_CallAttrStringHashf(this, "fill", _Dee_HashSelect(UINT32_C(0xbd501461), UINT64_C(0x7b3ed649c1abacf4)),  DEE_PCKdSIZ "oo", start, end, filler));
	}
	WUNUSED Ref<deemon::int_> (fill)(Dee_ssize_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "fill", _Dee_HashSelect(UINT32_C(0xbd501461), UINT64_C(0x7b3ed649c1abacf4)),  DEE_PCKdSIZ DEE_PCKdSIZ, start, end));
	}
	WUNUSED NONNULL_CXX((3)) Ref<deemon::int_> (fill)(Dee_ssize_t start, Dee_ssize_t end, DeeObject *filler) {
		return inherit(DeeObject_CallAttrStringHashf(this, "fill", _Dee_HashSelect(UINT32_C(0xbd501461), UINT64_C(0x7b3ed649c1abacf4)),  DEE_PCKdSIZ DEE_PCKdSIZ "o", start, end, filler));
	}
	WUNUSED Ref<deemon::int_> (fill)(Dee_ssize_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "fill", _Dee_HashSelect(UINT32_C(0xbd501461), UINT64_C(0x7b3ed649c1abacf4)),  DEE_PCKdSIZ DEE_PCKuSIZ, start, end));
	}
	WUNUSED NONNULL_CXX((3)) Ref<deemon::int_> (fill)(Dee_ssize_t start, size_t end, DeeObject *filler) {
		return inherit(DeeObject_CallAttrStringHashf(this, "fill", _Dee_HashSelect(UINT32_C(0xbd501461), UINT64_C(0x7b3ed649c1abacf4)),  DEE_PCKdSIZ DEE_PCKuSIZ "o", start, end, filler));
	}
	WUNUSED Ref<deemon::int_> (fill)(size_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "fill", _Dee_HashSelect(UINT32_C(0xbd501461), UINT64_C(0x7b3ed649c1abacf4)),  DEE_PCKuSIZ, start));
	}
	WUNUSED NONNULL_CXX((2)) Ref<deemon::int_> (fill)(size_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "fill", _Dee_HashSelect(UINT32_C(0xbd501461), UINT64_C(0x7b3ed649c1abacf4)),  DEE_PCKuSIZ "o", start, end));
	}
	WUNUSED NONNULL_CXX((2, 3)) Ref<deemon::int_> (fill)(size_t start, DeeObject *end, DeeObject *filler) {
		return inherit(DeeObject_CallAttrStringHashf(this, "fill", _Dee_HashSelect(UINT32_C(0xbd501461), UINT64_C(0x7b3ed649c1abacf4)),  DEE_PCKuSIZ "oo", start, end, filler));
	}
	WUNUSED Ref<deemon::int_> (fill)(size_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "fill", _Dee_HashSelect(UINT32_C(0xbd501461), UINT64_C(0x7b3ed649c1abacf4)),  DEE_PCKuSIZ DEE_PCKdSIZ, start, end));
	}
	WUNUSED NONNULL_CXX((3)) Ref<deemon::int_> (fill)(size_t start, Dee_ssize_t end, DeeObject *filler) {
		return inherit(DeeObject_CallAttrStringHashf(this, "fill", _Dee_HashSelect(UINT32_C(0xbd501461), UINT64_C(0x7b3ed649c1abacf4)),  DEE_PCKuSIZ DEE_PCKdSIZ "o", start, end, filler));
	}
	WUNUSED Ref<deemon::int_> (fill)(size_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "fill", _Dee_HashSelect(UINT32_C(0xbd501461), UINT64_C(0x7b3ed649c1abacf4)),  DEE_PCKuSIZ DEE_PCKuSIZ, start, end));
	}
	WUNUSED NONNULL_CXX((3)) Ref<deemon::int_> (fill)(size_t start, size_t end, DeeObject *filler) {
		return inherit(DeeObject_CallAttrStringHashf(this, "fill", _Dee_HashSelect(UINT32_C(0xbd501461), UINT64_C(0x7b3ed649c1abacf4)),  DEE_PCKuSIZ DEE_PCKuSIZ "o", start, end, filler));
	}
	void (sort)() {
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "sort", _Dee_HashSelect(UINT32_C(0xde7868af), UINT64_C(0x58835b3b7416f7f1)), 0, NULL)));
	}
	NONNULL_CXX((1)) void (sort)(DeeObject *key) {
		DeeObject *args[1];
		args[0] = key;
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "sort", _Dee_HashSelect(UINT32_C(0xde7868af), UINT64_C(0x58835b3b7416f7f1)), 1, args)));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Sequence<T> > (byhash)(DeeObject *template_) {
		DeeObject *args[1];
		args[0] = template_;
		return inherit(DeeObject_CallAttrStringHash(this, "byhash", _Dee_HashSelect(UINT32_C(0x7b5277ce), UINT64_C(0x773c8074445a28d9)), 1, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<T> (bfind)(DeeObject *elem) {
		DeeObject *args[1];
		args[0] = elem;
		return inherit(DeeObject_CallAttrStringHash(this, "bfind", _Dee_HashSelect(UINT32_C(0xdb39cc6c), UINT64_C(0x5ec07aef149314c7)), 1, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<T> (bfind)(DeeObject *elem, DeeObject *key_or_start) {
		DeeObject *args[2];
		args[0] = elem;
		args[1] = key_or_start;
		return inherit(DeeObject_CallAttrStringHash(this, "bfind", _Dee_HashSelect(UINT32_C(0xdb39cc6c), UINT64_C(0x5ec07aef149314c7)), 2, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3)) Ref<T> (bfind)(DeeObject *elem, DeeObject *start, DeeObject *key_or_end) {
		DeeObject *args[3];
		args[0] = elem;
		args[1] = start;
		args[2] = key_or_end;
		return inherit(DeeObject_CallAttrStringHash(this, "bfind", _Dee_HashSelect(UINT32_C(0xdb39cc6c), UINT64_C(0x5ec07aef149314c7)), 3, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3, 4)) Ref<T> (bfind)(DeeObject *elem, DeeObject *start, DeeObject *end, DeeObject *key) {
		DeeObject *args[4];
		args[0] = elem;
		args[1] = start;
		args[2] = end;
		args[3] = key;
		return inherit(DeeObject_CallAttrStringHash(this, "bfind", _Dee_HashSelect(UINT32_C(0xdb39cc6c), UINT64_C(0x5ec07aef149314c7)), 4, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<T> (bfind)(DeeObject *elem, DeeObject *start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bfind", _Dee_HashSelect(UINT32_C(0xdb39cc6c), UINT64_C(0x5ec07aef149314c7)), "oo" DEE_PCKdSIZ, elem, start, end));
	}
	WUNUSED NONNULL_CXX((1, 2, 4)) Ref<T> (bfind)(DeeObject *elem, DeeObject *start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bfind", _Dee_HashSelect(UINT32_C(0xdb39cc6c), UINT64_C(0x5ec07aef149314c7)), "oo" DEE_PCKdSIZ "o", elem, start, end, key));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<T> (bfind)(DeeObject *elem, DeeObject *start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bfind", _Dee_HashSelect(UINT32_C(0xdb39cc6c), UINT64_C(0x5ec07aef149314c7)), "oo" DEE_PCKuSIZ, elem, start, end));
	}
	WUNUSED NONNULL_CXX((1, 2, 4)) Ref<T> (bfind)(DeeObject *elem, DeeObject *start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bfind", _Dee_HashSelect(UINT32_C(0xdb39cc6c), UINT64_C(0x5ec07aef149314c7)), "oo" DEE_PCKuSIZ "o", elem, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<T> (bfind)(DeeObject *elem, Dee_ssize_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bfind", _Dee_HashSelect(UINT32_C(0xdb39cc6c), UINT64_C(0x5ec07aef149314c7)), "o" DEE_PCKdSIZ, elem, start));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<T> (bfind)(DeeObject *elem, Dee_ssize_t start, DeeObject *key_or_end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bfind", _Dee_HashSelect(UINT32_C(0xdb39cc6c), UINT64_C(0x5ec07aef149314c7)), "o" DEE_PCKdSIZ "o", elem, start, key_or_end));
	}
	WUNUSED NONNULL_CXX((1, 3, 4)) Ref<T> (bfind)(DeeObject *elem, Dee_ssize_t start, DeeObject *end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bfind", _Dee_HashSelect(UINT32_C(0xdb39cc6c), UINT64_C(0x5ec07aef149314c7)), "o" DEE_PCKdSIZ "oo", elem, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<T> (bfind)(DeeObject *elem, Dee_ssize_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bfind", _Dee_HashSelect(UINT32_C(0xdb39cc6c), UINT64_C(0x5ec07aef149314c7)), "o" DEE_PCKdSIZ DEE_PCKdSIZ, elem, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<T> (bfind)(DeeObject *elem, Dee_ssize_t start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bfind", _Dee_HashSelect(UINT32_C(0xdb39cc6c), UINT64_C(0x5ec07aef149314c7)), "o" DEE_PCKdSIZ DEE_PCKdSIZ "o", elem, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<T> (bfind)(DeeObject *elem, Dee_ssize_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bfind", _Dee_HashSelect(UINT32_C(0xdb39cc6c), UINT64_C(0x5ec07aef149314c7)), "o" DEE_PCKdSIZ DEE_PCKuSIZ, elem, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<T> (bfind)(DeeObject *elem, Dee_ssize_t start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bfind", _Dee_HashSelect(UINT32_C(0xdb39cc6c), UINT64_C(0x5ec07aef149314c7)), "o" DEE_PCKdSIZ DEE_PCKuSIZ "o", elem, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<T> (bfind)(DeeObject *elem, size_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bfind", _Dee_HashSelect(UINT32_C(0xdb39cc6c), UINT64_C(0x5ec07aef149314c7)), "o" DEE_PCKuSIZ, elem, start));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<T> (bfind)(DeeObject *elem, size_t start, DeeObject *key_or_end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bfind", _Dee_HashSelect(UINT32_C(0xdb39cc6c), UINT64_C(0x5ec07aef149314c7)), "o" DEE_PCKuSIZ "o", elem, start, key_or_end));
	}
	WUNUSED NONNULL_CXX((1, 3, 4)) Ref<T> (bfind)(DeeObject *elem, size_t start, DeeObject *end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bfind", _Dee_HashSelect(UINT32_C(0xdb39cc6c), UINT64_C(0x5ec07aef149314c7)), "o" DEE_PCKuSIZ "oo", elem, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<T> (bfind)(DeeObject *elem, size_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bfind", _Dee_HashSelect(UINT32_C(0xdb39cc6c), UINT64_C(0x5ec07aef149314c7)), "o" DEE_PCKuSIZ DEE_PCKdSIZ, elem, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<T> (bfind)(DeeObject *elem, size_t start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bfind", _Dee_HashSelect(UINT32_C(0xdb39cc6c), UINT64_C(0x5ec07aef149314c7)), "o" DEE_PCKuSIZ DEE_PCKdSIZ "o", elem, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<T> (bfind)(DeeObject *elem, size_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bfind", _Dee_HashSelect(UINT32_C(0xdb39cc6c), UINT64_C(0x5ec07aef149314c7)), "o" DEE_PCKuSIZ DEE_PCKuSIZ, elem, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<T> (bfind)(DeeObject *elem, size_t start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bfind", _Dee_HashSelect(UINT32_C(0xdb39cc6c), UINT64_C(0x5ec07aef149314c7)), "o" DEE_PCKuSIZ DEE_PCKuSIZ "o", elem, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (bcontains)(DeeObject *elem) {
		DeeObject *args[1];
		args[0] = elem;
		return inherit(DeeObject_CallAttrStringHash(this, "bcontains", _Dee_HashSelect(UINT32_C(0x2a030b92), UINT64_C(0x545f84c22975fbfc)), 1, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::bool_> (bcontains)(DeeObject *elem, DeeObject *key_or_start) {
		DeeObject *args[2];
		args[0] = elem;
		args[1] = key_or_start;
		return inherit(DeeObject_CallAttrStringHash(this, "bcontains", _Dee_HashSelect(UINT32_C(0x2a030b92), UINT64_C(0x545f84c22975fbfc)), 2, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3)) Ref<deemon::bool_> (bcontains)(DeeObject *elem, DeeObject *start, DeeObject *key_or_end) {
		DeeObject *args[3];
		args[0] = elem;
		args[1] = start;
		args[2] = key_or_end;
		return inherit(DeeObject_CallAttrStringHash(this, "bcontains", _Dee_HashSelect(UINT32_C(0x2a030b92), UINT64_C(0x545f84c22975fbfc)), 3, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3, 4)) Ref<deemon::bool_> (bcontains)(DeeObject *elem, DeeObject *start, DeeObject *end, DeeObject *key) {
		DeeObject *args[4];
		args[0] = elem;
		args[1] = start;
		args[2] = end;
		args[3] = key;
		return inherit(DeeObject_CallAttrStringHash(this, "bcontains", _Dee_HashSelect(UINT32_C(0x2a030b92), UINT64_C(0x545f84c22975fbfc)), 4, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::bool_> (bcontains)(DeeObject *elem, DeeObject *start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bcontains", _Dee_HashSelect(UINT32_C(0x2a030b92), UINT64_C(0x545f84c22975fbfc)), "oo" DEE_PCKdSIZ, elem, start, end));
	}
	WUNUSED NONNULL_CXX((1, 2, 4)) Ref<deemon::bool_> (bcontains)(DeeObject *elem, DeeObject *start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bcontains", _Dee_HashSelect(UINT32_C(0x2a030b92), UINT64_C(0x545f84c22975fbfc)), "oo" DEE_PCKdSIZ "o", elem, start, end, key));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::bool_> (bcontains)(DeeObject *elem, DeeObject *start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bcontains", _Dee_HashSelect(UINT32_C(0x2a030b92), UINT64_C(0x545f84c22975fbfc)), "oo" DEE_PCKuSIZ, elem, start, end));
	}
	WUNUSED NONNULL_CXX((1, 2, 4)) Ref<deemon::bool_> (bcontains)(DeeObject *elem, DeeObject *start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bcontains", _Dee_HashSelect(UINT32_C(0x2a030b92), UINT64_C(0x545f84c22975fbfc)), "oo" DEE_PCKuSIZ "o", elem, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (bcontains)(DeeObject *elem, Dee_ssize_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bcontains", _Dee_HashSelect(UINT32_C(0x2a030b92), UINT64_C(0x545f84c22975fbfc)), "o" DEE_PCKdSIZ, elem, start));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::bool_> (bcontains)(DeeObject *elem, Dee_ssize_t start, DeeObject *key_or_end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bcontains", _Dee_HashSelect(UINT32_C(0x2a030b92), UINT64_C(0x545f84c22975fbfc)), "o" DEE_PCKdSIZ "o", elem, start, key_or_end));
	}
	WUNUSED NONNULL_CXX((1, 3, 4)) Ref<deemon::bool_> (bcontains)(DeeObject *elem, Dee_ssize_t start, DeeObject *end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bcontains", _Dee_HashSelect(UINT32_C(0x2a030b92), UINT64_C(0x545f84c22975fbfc)), "o" DEE_PCKdSIZ "oo", elem, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (bcontains)(DeeObject *elem, Dee_ssize_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bcontains", _Dee_HashSelect(UINT32_C(0x2a030b92), UINT64_C(0x545f84c22975fbfc)), "o" DEE_PCKdSIZ DEE_PCKdSIZ, elem, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::bool_> (bcontains)(DeeObject *elem, Dee_ssize_t start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bcontains", _Dee_HashSelect(UINT32_C(0x2a030b92), UINT64_C(0x545f84c22975fbfc)), "o" DEE_PCKdSIZ DEE_PCKdSIZ "o", elem, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (bcontains)(DeeObject *elem, Dee_ssize_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bcontains", _Dee_HashSelect(UINT32_C(0x2a030b92), UINT64_C(0x545f84c22975fbfc)), "o" DEE_PCKdSIZ DEE_PCKuSIZ, elem, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::bool_> (bcontains)(DeeObject *elem, Dee_ssize_t start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bcontains", _Dee_HashSelect(UINT32_C(0x2a030b92), UINT64_C(0x545f84c22975fbfc)), "o" DEE_PCKdSIZ DEE_PCKuSIZ "o", elem, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (bcontains)(DeeObject *elem, size_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bcontains", _Dee_HashSelect(UINT32_C(0x2a030b92), UINT64_C(0x545f84c22975fbfc)), "o" DEE_PCKuSIZ, elem, start));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::bool_> (bcontains)(DeeObject *elem, size_t start, DeeObject *key_or_end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bcontains", _Dee_HashSelect(UINT32_C(0x2a030b92), UINT64_C(0x545f84c22975fbfc)), "o" DEE_PCKuSIZ "o", elem, start, key_or_end));
	}
	WUNUSED NONNULL_CXX((1, 3, 4)) Ref<deemon::bool_> (bcontains)(DeeObject *elem, size_t start, DeeObject *end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bcontains", _Dee_HashSelect(UINT32_C(0x2a030b92), UINT64_C(0x545f84c22975fbfc)), "o" DEE_PCKuSIZ "oo", elem, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (bcontains)(DeeObject *elem, size_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bcontains", _Dee_HashSelect(UINT32_C(0x2a030b92), UINT64_C(0x545f84c22975fbfc)), "o" DEE_PCKuSIZ DEE_PCKdSIZ, elem, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::bool_> (bcontains)(DeeObject *elem, size_t start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bcontains", _Dee_HashSelect(UINT32_C(0x2a030b92), UINT64_C(0x545f84c22975fbfc)), "o" DEE_PCKuSIZ DEE_PCKdSIZ "o", elem, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (bcontains)(DeeObject *elem, size_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bcontains", _Dee_HashSelect(UINT32_C(0x2a030b92), UINT64_C(0x545f84c22975fbfc)), "o" DEE_PCKuSIZ DEE_PCKuSIZ, elem, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::bool_> (bcontains)(DeeObject *elem, size_t start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bcontains", _Dee_HashSelect(UINT32_C(0x2a030b92), UINT64_C(0x545f84c22975fbfc)), "o" DEE_PCKuSIZ DEE_PCKuSIZ "o", elem, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (bindex)(DeeObject *elem) {
		DeeObject *args[1];
		args[0] = elem;
		return inherit(DeeObject_CallAttrStringHash(this, "bindex", _Dee_HashSelect(UINT32_C(0x2c8be478), UINT64_C(0xffcf13ce8c346f38)), 1, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::int_> (bindex)(DeeObject *elem, DeeObject *key_or_start) {
		DeeObject *args[2];
		args[0] = elem;
		args[1] = key_or_start;
		return inherit(DeeObject_CallAttrStringHash(this, "bindex", _Dee_HashSelect(UINT32_C(0x2c8be478), UINT64_C(0xffcf13ce8c346f38)), 2, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3)) Ref<deemon::int_> (bindex)(DeeObject *elem, DeeObject *start, DeeObject *key_or_end) {
		DeeObject *args[3];
		args[0] = elem;
		args[1] = start;
		args[2] = key_or_end;
		return inherit(DeeObject_CallAttrStringHash(this, "bindex", _Dee_HashSelect(UINT32_C(0x2c8be478), UINT64_C(0xffcf13ce8c346f38)), 3, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3, 4)) Ref<deemon::int_> (bindex)(DeeObject *elem, DeeObject *start, DeeObject *end, DeeObject *key) {
		DeeObject *args[4];
		args[0] = elem;
		args[1] = start;
		args[2] = end;
		args[3] = key;
		return inherit(DeeObject_CallAttrStringHash(this, "bindex", _Dee_HashSelect(UINT32_C(0x2c8be478), UINT64_C(0xffcf13ce8c346f38)), 4, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::int_> (bindex)(DeeObject *elem, DeeObject *start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bindex", _Dee_HashSelect(UINT32_C(0x2c8be478), UINT64_C(0xffcf13ce8c346f38)), "oo" DEE_PCKdSIZ, elem, start, end));
	}
	WUNUSED NONNULL_CXX((1, 2, 4)) Ref<deemon::int_> (bindex)(DeeObject *elem, DeeObject *start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bindex", _Dee_HashSelect(UINT32_C(0x2c8be478), UINT64_C(0xffcf13ce8c346f38)), "oo" DEE_PCKdSIZ "o", elem, start, end, key));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::int_> (bindex)(DeeObject *elem, DeeObject *start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bindex", _Dee_HashSelect(UINT32_C(0x2c8be478), UINT64_C(0xffcf13ce8c346f38)), "oo" DEE_PCKuSIZ, elem, start, end));
	}
	WUNUSED NONNULL_CXX((1, 2, 4)) Ref<deemon::int_> (bindex)(DeeObject *elem, DeeObject *start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bindex", _Dee_HashSelect(UINT32_C(0x2c8be478), UINT64_C(0xffcf13ce8c346f38)), "oo" DEE_PCKuSIZ "o", elem, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (bindex)(DeeObject *elem, Dee_ssize_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bindex", _Dee_HashSelect(UINT32_C(0x2c8be478), UINT64_C(0xffcf13ce8c346f38)), "o" DEE_PCKdSIZ, elem, start));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::int_> (bindex)(DeeObject *elem, Dee_ssize_t start, DeeObject *key_or_end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bindex", _Dee_HashSelect(UINT32_C(0x2c8be478), UINT64_C(0xffcf13ce8c346f38)), "o" DEE_PCKdSIZ "o", elem, start, key_or_end));
	}
	WUNUSED NONNULL_CXX((1, 3, 4)) Ref<deemon::int_> (bindex)(DeeObject *elem, Dee_ssize_t start, DeeObject *end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bindex", _Dee_HashSelect(UINT32_C(0x2c8be478), UINT64_C(0xffcf13ce8c346f38)), "o" DEE_PCKdSIZ "oo", elem, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (bindex)(DeeObject *elem, Dee_ssize_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bindex", _Dee_HashSelect(UINT32_C(0x2c8be478), UINT64_C(0xffcf13ce8c346f38)), "o" DEE_PCKdSIZ DEE_PCKdSIZ, elem, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::int_> (bindex)(DeeObject *elem, Dee_ssize_t start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bindex", _Dee_HashSelect(UINT32_C(0x2c8be478), UINT64_C(0xffcf13ce8c346f38)), "o" DEE_PCKdSIZ DEE_PCKdSIZ "o", elem, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (bindex)(DeeObject *elem, Dee_ssize_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bindex", _Dee_HashSelect(UINT32_C(0x2c8be478), UINT64_C(0xffcf13ce8c346f38)), "o" DEE_PCKdSIZ DEE_PCKuSIZ, elem, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::int_> (bindex)(DeeObject *elem, Dee_ssize_t start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bindex", _Dee_HashSelect(UINT32_C(0x2c8be478), UINT64_C(0xffcf13ce8c346f38)), "o" DEE_PCKdSIZ DEE_PCKuSIZ "o", elem, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (bindex)(DeeObject *elem, size_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bindex", _Dee_HashSelect(UINT32_C(0x2c8be478), UINT64_C(0xffcf13ce8c346f38)), "o" DEE_PCKuSIZ, elem, start));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::int_> (bindex)(DeeObject *elem, size_t start, DeeObject *key_or_end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bindex", _Dee_HashSelect(UINT32_C(0x2c8be478), UINT64_C(0xffcf13ce8c346f38)), "o" DEE_PCKuSIZ "o", elem, start, key_or_end));
	}
	WUNUSED NONNULL_CXX((1, 3, 4)) Ref<deemon::int_> (bindex)(DeeObject *elem, size_t start, DeeObject *end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bindex", _Dee_HashSelect(UINT32_C(0x2c8be478), UINT64_C(0xffcf13ce8c346f38)), "o" DEE_PCKuSIZ "oo", elem, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (bindex)(DeeObject *elem, size_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bindex", _Dee_HashSelect(UINT32_C(0x2c8be478), UINT64_C(0xffcf13ce8c346f38)), "o" DEE_PCKuSIZ DEE_PCKdSIZ, elem, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::int_> (bindex)(DeeObject *elem, size_t start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bindex", _Dee_HashSelect(UINT32_C(0x2c8be478), UINT64_C(0xffcf13ce8c346f38)), "o" DEE_PCKuSIZ DEE_PCKdSIZ "o", elem, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (bindex)(DeeObject *elem, size_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bindex", _Dee_HashSelect(UINT32_C(0x2c8be478), UINT64_C(0xffcf13ce8c346f38)), "o" DEE_PCKuSIZ DEE_PCKuSIZ, elem, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::int_> (bindex)(DeeObject *elem, size_t start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bindex", _Dee_HashSelect(UINT32_C(0x2c8be478), UINT64_C(0xffcf13ce8c346f38)), "o" DEE_PCKuSIZ DEE_PCKuSIZ "o", elem, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (bposition)(DeeObject *elem) {
		DeeObject *args[1];
		args[0] = elem;
		return inherit(DeeObject_CallAttrStringHash(this, "bposition", _Dee_HashSelect(UINT32_C(0xba99f013), UINT64_C(0xc8f6389c9f293cb2)), 1, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::int_> (bposition)(DeeObject *elem, DeeObject *key_or_start) {
		DeeObject *args[2];
		args[0] = elem;
		args[1] = key_or_start;
		return inherit(DeeObject_CallAttrStringHash(this, "bposition", _Dee_HashSelect(UINT32_C(0xba99f013), UINT64_C(0xc8f6389c9f293cb2)), 2, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3)) Ref<deemon::int_> (bposition)(DeeObject *elem, DeeObject *start, DeeObject *key_or_end) {
		DeeObject *args[3];
		args[0] = elem;
		args[1] = start;
		args[2] = key_or_end;
		return inherit(DeeObject_CallAttrStringHash(this, "bposition", _Dee_HashSelect(UINT32_C(0xba99f013), UINT64_C(0xc8f6389c9f293cb2)), 3, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3, 4)) Ref<deemon::int_> (bposition)(DeeObject *elem, DeeObject *start, DeeObject *end, DeeObject *key) {
		DeeObject *args[4];
		args[0] = elem;
		args[1] = start;
		args[2] = end;
		args[3] = key;
		return inherit(DeeObject_CallAttrStringHash(this, "bposition", _Dee_HashSelect(UINT32_C(0xba99f013), UINT64_C(0xc8f6389c9f293cb2)), 4, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::int_> (bposition)(DeeObject *elem, DeeObject *start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bposition", _Dee_HashSelect(UINT32_C(0xba99f013), UINT64_C(0xc8f6389c9f293cb2)), "oo" DEE_PCKdSIZ, elem, start, end));
	}
	WUNUSED NONNULL_CXX((1, 2, 4)) Ref<deemon::int_> (bposition)(DeeObject *elem, DeeObject *start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bposition", _Dee_HashSelect(UINT32_C(0xba99f013), UINT64_C(0xc8f6389c9f293cb2)), "oo" DEE_PCKdSIZ "o", elem, start, end, key));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::int_> (bposition)(DeeObject *elem, DeeObject *start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bposition", _Dee_HashSelect(UINT32_C(0xba99f013), UINT64_C(0xc8f6389c9f293cb2)), "oo" DEE_PCKuSIZ, elem, start, end));
	}
	WUNUSED NONNULL_CXX((1, 2, 4)) Ref<deemon::int_> (bposition)(DeeObject *elem, DeeObject *start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bposition", _Dee_HashSelect(UINT32_C(0xba99f013), UINT64_C(0xc8f6389c9f293cb2)), "oo" DEE_PCKuSIZ "o", elem, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (bposition)(DeeObject *elem, Dee_ssize_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bposition", _Dee_HashSelect(UINT32_C(0xba99f013), UINT64_C(0xc8f6389c9f293cb2)), "o" DEE_PCKdSIZ, elem, start));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::int_> (bposition)(DeeObject *elem, Dee_ssize_t start, DeeObject *key_or_end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bposition", _Dee_HashSelect(UINT32_C(0xba99f013), UINT64_C(0xc8f6389c9f293cb2)), "o" DEE_PCKdSIZ "o", elem, start, key_or_end));
	}
	WUNUSED NONNULL_CXX((1, 3, 4)) Ref<deemon::int_> (bposition)(DeeObject *elem, Dee_ssize_t start, DeeObject *end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bposition", _Dee_HashSelect(UINT32_C(0xba99f013), UINT64_C(0xc8f6389c9f293cb2)), "o" DEE_PCKdSIZ "oo", elem, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (bposition)(DeeObject *elem, Dee_ssize_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bposition", _Dee_HashSelect(UINT32_C(0xba99f013), UINT64_C(0xc8f6389c9f293cb2)), "o" DEE_PCKdSIZ DEE_PCKdSIZ, elem, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::int_> (bposition)(DeeObject *elem, Dee_ssize_t start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bposition", _Dee_HashSelect(UINT32_C(0xba99f013), UINT64_C(0xc8f6389c9f293cb2)), "o" DEE_PCKdSIZ DEE_PCKdSIZ "o", elem, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (bposition)(DeeObject *elem, Dee_ssize_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bposition", _Dee_HashSelect(UINT32_C(0xba99f013), UINT64_C(0xc8f6389c9f293cb2)), "o" DEE_PCKdSIZ DEE_PCKuSIZ, elem, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::int_> (bposition)(DeeObject *elem, Dee_ssize_t start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bposition", _Dee_HashSelect(UINT32_C(0xba99f013), UINT64_C(0xc8f6389c9f293cb2)), "o" DEE_PCKdSIZ DEE_PCKuSIZ "o", elem, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (bposition)(DeeObject *elem, size_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bposition", _Dee_HashSelect(UINT32_C(0xba99f013), UINT64_C(0xc8f6389c9f293cb2)), "o" DEE_PCKuSIZ, elem, start));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::int_> (bposition)(DeeObject *elem, size_t start, DeeObject *key_or_end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bposition", _Dee_HashSelect(UINT32_C(0xba99f013), UINT64_C(0xc8f6389c9f293cb2)), "o" DEE_PCKuSIZ "o", elem, start, key_or_end));
	}
	WUNUSED NONNULL_CXX((1, 3, 4)) Ref<deemon::int_> (bposition)(DeeObject *elem, size_t start, DeeObject *end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bposition", _Dee_HashSelect(UINT32_C(0xba99f013), UINT64_C(0xc8f6389c9f293cb2)), "o" DEE_PCKuSIZ "oo", elem, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (bposition)(DeeObject *elem, size_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bposition", _Dee_HashSelect(UINT32_C(0xba99f013), UINT64_C(0xc8f6389c9f293cb2)), "o" DEE_PCKuSIZ DEE_PCKdSIZ, elem, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::int_> (bposition)(DeeObject *elem, size_t start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bposition", _Dee_HashSelect(UINT32_C(0xba99f013), UINT64_C(0xc8f6389c9f293cb2)), "o" DEE_PCKuSIZ DEE_PCKdSIZ "o", elem, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (bposition)(DeeObject *elem, size_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bposition", _Dee_HashSelect(UINT32_C(0xba99f013), UINT64_C(0xc8f6389c9f293cb2)), "o" DEE_PCKuSIZ DEE_PCKuSIZ, elem, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::int_> (bposition)(DeeObject *elem, size_t start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "bposition", _Dee_HashSelect(UINT32_C(0xba99f013), UINT64_C(0xc8f6389c9f293cb2)), "o" DEE_PCKuSIZ DEE_PCKuSIZ "o", elem, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<_AbstractTuple<deemon::int_, deemon::int_> > (brange)(DeeObject *elem) {
		DeeObject *args[1];
		args[0] = elem;
		return inherit(DeeObject_CallAttrStringHash(this, "brange", _Dee_HashSelect(UINT32_C(0xb132222e), UINT64_C(0xfed8bb16d0ac0dd2)), 1, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<_AbstractTuple<deemon::int_, deemon::int_> > (brange)(DeeObject *elem, DeeObject *key_or_start) {
		DeeObject *args[2];
		args[0] = elem;
		args[1] = key_or_start;
		return inherit(DeeObject_CallAttrStringHash(this, "brange", _Dee_HashSelect(UINT32_C(0xb132222e), UINT64_C(0xfed8bb16d0ac0dd2)), 2, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3)) Ref<_AbstractTuple<deemon::int_, deemon::int_> > (brange)(DeeObject *elem, DeeObject *start, DeeObject *key_or_end) {
		DeeObject *args[3];
		args[0] = elem;
		args[1] = start;
		args[2] = key_or_end;
		return inherit(DeeObject_CallAttrStringHash(this, "brange", _Dee_HashSelect(UINT32_C(0xb132222e), UINT64_C(0xfed8bb16d0ac0dd2)), 3, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3, 4)) Ref<_AbstractTuple<deemon::int_, deemon::int_> > (brange)(DeeObject *elem, DeeObject *start, DeeObject *end, DeeObject *key) {
		DeeObject *args[4];
		args[0] = elem;
		args[1] = start;
		args[2] = end;
		args[3] = key;
		return inherit(DeeObject_CallAttrStringHash(this, "brange", _Dee_HashSelect(UINT32_C(0xb132222e), UINT64_C(0xfed8bb16d0ac0dd2)), 4, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<_AbstractTuple<deemon::int_, deemon::int_> > (brange)(DeeObject *elem, DeeObject *start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "brange", _Dee_HashSelect(UINT32_C(0xb132222e), UINT64_C(0xfed8bb16d0ac0dd2)), "oo" DEE_PCKdSIZ, elem, start, end));
	}
	WUNUSED NONNULL_CXX((1, 2, 4)) Ref<_AbstractTuple<deemon::int_, deemon::int_> > (brange)(DeeObject *elem, DeeObject *start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "brange", _Dee_HashSelect(UINT32_C(0xb132222e), UINT64_C(0xfed8bb16d0ac0dd2)), "oo" DEE_PCKdSIZ "o", elem, start, end, key));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<_AbstractTuple<deemon::int_, deemon::int_> > (brange)(DeeObject *elem, DeeObject *start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "brange", _Dee_HashSelect(UINT32_C(0xb132222e), UINT64_C(0xfed8bb16d0ac0dd2)), "oo" DEE_PCKuSIZ, elem, start, end));
	}
	WUNUSED NONNULL_CXX((1, 2, 4)) Ref<_AbstractTuple<deemon::int_, deemon::int_> > (brange)(DeeObject *elem, DeeObject *start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "brange", _Dee_HashSelect(UINT32_C(0xb132222e), UINT64_C(0xfed8bb16d0ac0dd2)), "oo" DEE_PCKuSIZ "o", elem, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<_AbstractTuple<deemon::int_, deemon::int_> > (brange)(DeeObject *elem, Dee_ssize_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "brange", _Dee_HashSelect(UINT32_C(0xb132222e), UINT64_C(0xfed8bb16d0ac0dd2)), "o" DEE_PCKdSIZ, elem, start));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<_AbstractTuple<deemon::int_, deemon::int_> > (brange)(DeeObject *elem, Dee_ssize_t start, DeeObject *key_or_end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "brange", _Dee_HashSelect(UINT32_C(0xb132222e), UINT64_C(0xfed8bb16d0ac0dd2)), "o" DEE_PCKdSIZ "o", elem, start, key_or_end));
	}
	WUNUSED NONNULL_CXX((1, 3, 4)) Ref<_AbstractTuple<deemon::int_, deemon::int_> > (brange)(DeeObject *elem, Dee_ssize_t start, DeeObject *end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "brange", _Dee_HashSelect(UINT32_C(0xb132222e), UINT64_C(0xfed8bb16d0ac0dd2)), "o" DEE_PCKdSIZ "oo", elem, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<_AbstractTuple<deemon::int_, deemon::int_> > (brange)(DeeObject *elem, Dee_ssize_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "brange", _Dee_HashSelect(UINT32_C(0xb132222e), UINT64_C(0xfed8bb16d0ac0dd2)), "o" DEE_PCKdSIZ DEE_PCKdSIZ, elem, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<_AbstractTuple<deemon::int_, deemon::int_> > (brange)(DeeObject *elem, Dee_ssize_t start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "brange", _Dee_HashSelect(UINT32_C(0xb132222e), UINT64_C(0xfed8bb16d0ac0dd2)), "o" DEE_PCKdSIZ DEE_PCKdSIZ "o", elem, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<_AbstractTuple<deemon::int_, deemon::int_> > (brange)(DeeObject *elem, Dee_ssize_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "brange", _Dee_HashSelect(UINT32_C(0xb132222e), UINT64_C(0xfed8bb16d0ac0dd2)), "o" DEE_PCKdSIZ DEE_PCKuSIZ, elem, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<_AbstractTuple<deemon::int_, deemon::int_> > (brange)(DeeObject *elem, Dee_ssize_t start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "brange", _Dee_HashSelect(UINT32_C(0xb132222e), UINT64_C(0xfed8bb16d0ac0dd2)), "o" DEE_PCKdSIZ DEE_PCKuSIZ "o", elem, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<_AbstractTuple<deemon::int_, deemon::int_> > (brange)(DeeObject *elem, size_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "brange", _Dee_HashSelect(UINT32_C(0xb132222e), UINT64_C(0xfed8bb16d0ac0dd2)), "o" DEE_PCKuSIZ, elem, start));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<_AbstractTuple<deemon::int_, deemon::int_> > (brange)(DeeObject *elem, size_t start, DeeObject *key_or_end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "brange", _Dee_HashSelect(UINT32_C(0xb132222e), UINT64_C(0xfed8bb16d0ac0dd2)), "o" DEE_PCKuSIZ "o", elem, start, key_or_end));
	}
	WUNUSED NONNULL_CXX((1, 3, 4)) Ref<_AbstractTuple<deemon::int_, deemon::int_> > (brange)(DeeObject *elem, size_t start, DeeObject *end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "brange", _Dee_HashSelect(UINT32_C(0xb132222e), UINT64_C(0xfed8bb16d0ac0dd2)), "o" DEE_PCKuSIZ "oo", elem, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<_AbstractTuple<deemon::int_, deemon::int_> > (brange)(DeeObject *elem, size_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "brange", _Dee_HashSelect(UINT32_C(0xb132222e), UINT64_C(0xfed8bb16d0ac0dd2)), "o" DEE_PCKuSIZ DEE_PCKdSIZ, elem, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<_AbstractTuple<deemon::int_, deemon::int_> > (brange)(DeeObject *elem, size_t start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "brange", _Dee_HashSelect(UINT32_C(0xb132222e), UINT64_C(0xfed8bb16d0ac0dd2)), "o" DEE_PCKuSIZ DEE_PCKdSIZ "o", elem, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<_AbstractTuple<deemon::int_, deemon::int_> > (brange)(DeeObject *elem, size_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "brange", _Dee_HashSelect(UINT32_C(0xb132222e), UINT64_C(0xfed8bb16d0ac0dd2)), "o" DEE_PCKuSIZ DEE_PCKuSIZ, elem, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<_AbstractTuple<deemon::int_, deemon::int_> > (brange)(DeeObject *elem, size_t start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "brange", _Dee_HashSelect(UINT32_C(0xb132222e), UINT64_C(0xfed8bb16d0ac0dd2)), "o" DEE_PCKuSIZ DEE_PCKuSIZ "o", elem, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<T> (blocate)(DeeObject *elem) {
		DeeObject *args[1];
		args[0] = elem;
		return inherit(DeeObject_CallAttrStringHash(this, "blocate", _Dee_HashSelect(UINT32_C(0x7aa979d3), UINT64_C(0xbda91c237d69489e)), 1, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<T> (blocate)(DeeObject *elem, DeeObject *key_or_start) {
		DeeObject *args[2];
		args[0] = elem;
		args[1] = key_or_start;
		return inherit(DeeObject_CallAttrStringHash(this, "blocate", _Dee_HashSelect(UINT32_C(0x7aa979d3), UINT64_C(0xbda91c237d69489e)), 2, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3)) Ref<T> (blocate)(DeeObject *elem, DeeObject *key_or_start_or_start_or_start, DeeObject *defl_or_key_or_key_or_end) {
		DeeObject *args[3];
		args[0] = elem;
		args[1] = key_or_start_or_start_or_start;
		args[2] = defl_or_key_or_key_or_end;
		return inherit(DeeObject_CallAttrStringHash(this, "blocate", _Dee_HashSelect(UINT32_C(0x7aa979d3), UINT64_C(0xbda91c237d69489e)), 3, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3, 4)) Ref<T> (blocate)(DeeObject *elem, DeeObject *start, DeeObject *key_or_end_or_end, DeeObject *defl_or_key_or_key) {
		DeeObject *args[4];
		args[0] = elem;
		args[1] = start;
		args[2] = key_or_end_or_end;
		args[3] = defl_or_key_or_key;
		return inherit(DeeObject_CallAttrStringHash(this, "blocate", _Dee_HashSelect(UINT32_C(0x7aa979d3), UINT64_C(0xbda91c237d69489e)), 4, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3, 4, 5)) Ref<T> (blocate)(DeeObject *elem, DeeObject *start, DeeObject *end, DeeObject *key, DeeObject *defl) {
		DeeObject *args[5];
		args[0] = elem;
		args[1] = start;
		args[2] = end;
		args[3] = key;
		args[4] = defl;
		return inherit(DeeObject_CallAttrStringHash(this, "blocate", _Dee_HashSelect(UINT32_C(0x7aa979d3), UINT64_C(0xbda91c237d69489e)), 5, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<T> (blocate)(DeeObject *elem, DeeObject *start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "blocate", _Dee_HashSelect(UINT32_C(0x7aa979d3), UINT64_C(0xbda91c237d69489e)), "oo" DEE_PCKdSIZ, elem, start, end));
	}
	WUNUSED NONNULL_CXX((1, 2, 4)) Ref<T> (blocate)(DeeObject *elem, DeeObject *start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "blocate", _Dee_HashSelect(UINT32_C(0x7aa979d3), UINT64_C(0xbda91c237d69489e)), "oo" DEE_PCKdSIZ "o", elem, start, end, key));
	}
	WUNUSED NONNULL_CXX((1, 2, 4, 5)) Ref<T> (blocate)(DeeObject *elem, DeeObject *start, Dee_ssize_t end, DeeObject *key, DeeObject *defl) {
		return inherit(DeeObject_CallAttrStringHashf(this, "blocate", _Dee_HashSelect(UINT32_C(0x7aa979d3), UINT64_C(0xbda91c237d69489e)), "oo" DEE_PCKdSIZ "oo", elem, start, end, key, defl));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<T> (blocate)(DeeObject *elem, DeeObject *start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "blocate", _Dee_HashSelect(UINT32_C(0x7aa979d3), UINT64_C(0xbda91c237d69489e)), "oo" DEE_PCKuSIZ, elem, start, end));
	}
	WUNUSED NONNULL_CXX((1, 2, 4)) Ref<T> (blocate)(DeeObject *elem, DeeObject *start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "blocate", _Dee_HashSelect(UINT32_C(0x7aa979d3), UINT64_C(0xbda91c237d69489e)), "oo" DEE_PCKuSIZ "o", elem, start, end, key));
	}
	WUNUSED NONNULL_CXX((1, 2, 4, 5)) Ref<T> (blocate)(DeeObject *elem, DeeObject *start, size_t end, DeeObject *key, DeeObject *defl) {
		return inherit(DeeObject_CallAttrStringHashf(this, "blocate", _Dee_HashSelect(UINT32_C(0x7aa979d3), UINT64_C(0xbda91c237d69489e)), "oo" DEE_PCKuSIZ "oo", elem, start, end, key, defl));
	}
	WUNUSED NONNULL_CXX((1)) Ref<T> (blocate)(DeeObject *elem, Dee_ssize_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "blocate", _Dee_HashSelect(UINT32_C(0x7aa979d3), UINT64_C(0xbda91c237d69489e)), "o" DEE_PCKdSIZ, elem, start));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<T> (blocate)(DeeObject *elem, Dee_ssize_t start, DeeObject *key_or_end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "blocate", _Dee_HashSelect(UINT32_C(0x7aa979d3), UINT64_C(0xbda91c237d69489e)), "o" DEE_PCKdSIZ "o", elem, start, key_or_end));
	}
	WUNUSED NONNULL_CXX((1, 3, 4)) Ref<T> (blocate)(DeeObject *elem, Dee_ssize_t start, DeeObject *key_or_end_or_end, DeeObject *defl_or_key_or_key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "blocate", _Dee_HashSelect(UINT32_C(0x7aa979d3), UINT64_C(0xbda91c237d69489e)), "o" DEE_PCKdSIZ "oo", elem, start, key_or_end_or_end, defl_or_key_or_key));
	}
	WUNUSED NONNULL_CXX((1, 3, 4, 5)) Ref<T> (blocate)(DeeObject *elem, Dee_ssize_t start, DeeObject *end, DeeObject *key, DeeObject *defl) {
		return inherit(DeeObject_CallAttrStringHashf(this, "blocate", _Dee_HashSelect(UINT32_C(0x7aa979d3), UINT64_C(0xbda91c237d69489e)), "o" DEE_PCKdSIZ "ooo", elem, start, end, key, defl));
	}
	WUNUSED NONNULL_CXX((1)) Ref<T> (blocate)(DeeObject *elem, Dee_ssize_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "blocate", _Dee_HashSelect(UINT32_C(0x7aa979d3), UINT64_C(0xbda91c237d69489e)), "o" DEE_PCKdSIZ DEE_PCKdSIZ, elem, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<T> (blocate)(DeeObject *elem, Dee_ssize_t start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "blocate", _Dee_HashSelect(UINT32_C(0x7aa979d3), UINT64_C(0xbda91c237d69489e)), "o" DEE_PCKdSIZ DEE_PCKdSIZ "o", elem, start, end, key));
	}
	WUNUSED NONNULL_CXX((1, 4, 5)) Ref<T> (blocate)(DeeObject *elem, Dee_ssize_t start, Dee_ssize_t end, DeeObject *key, DeeObject *defl) {
		return inherit(DeeObject_CallAttrStringHashf(this, "blocate", _Dee_HashSelect(UINT32_C(0x7aa979d3), UINT64_C(0xbda91c237d69489e)), "o" DEE_PCKdSIZ DEE_PCKdSIZ "oo", elem, start, end, key, defl));
	}
	WUNUSED NONNULL_CXX((1)) Ref<T> (blocate)(DeeObject *elem, Dee_ssize_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "blocate", _Dee_HashSelect(UINT32_C(0x7aa979d3), UINT64_C(0xbda91c237d69489e)), "o" DEE_PCKdSIZ DEE_PCKuSIZ, elem, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<T> (blocate)(DeeObject *elem, Dee_ssize_t start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "blocate", _Dee_HashSelect(UINT32_C(0x7aa979d3), UINT64_C(0xbda91c237d69489e)), "o" DEE_PCKdSIZ DEE_PCKuSIZ "o", elem, start, end, key));
	}
	WUNUSED NONNULL_CXX((1, 4, 5)) Ref<T> (blocate)(DeeObject *elem, Dee_ssize_t start, size_t end, DeeObject *key, DeeObject *defl) {
		return inherit(DeeObject_CallAttrStringHashf(this, "blocate", _Dee_HashSelect(UINT32_C(0x7aa979d3), UINT64_C(0xbda91c237d69489e)), "o" DEE_PCKdSIZ DEE_PCKuSIZ "oo", elem, start, end, key, defl));
	}
	WUNUSED NONNULL_CXX((1)) Ref<T> (blocate)(DeeObject *elem, size_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "blocate", _Dee_HashSelect(UINT32_C(0x7aa979d3), UINT64_C(0xbda91c237d69489e)), "o" DEE_PCKuSIZ, elem, start));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<T> (blocate)(DeeObject *elem, size_t start, DeeObject *key_or_end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "blocate", _Dee_HashSelect(UINT32_C(0x7aa979d3), UINT64_C(0xbda91c237d69489e)), "o" DEE_PCKuSIZ "o", elem, start, key_or_end));
	}
	WUNUSED NONNULL_CXX((1, 3, 4)) Ref<T> (blocate)(DeeObject *elem, size_t start, DeeObject *key_or_end_or_end, DeeObject *defl_or_key_or_key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "blocate", _Dee_HashSelect(UINT32_C(0x7aa979d3), UINT64_C(0xbda91c237d69489e)), "o" DEE_PCKuSIZ "oo", elem, start, key_or_end_or_end, defl_or_key_or_key));
	}
	WUNUSED NONNULL_CXX((1, 3, 4, 5)) Ref<T> (blocate)(DeeObject *elem, size_t start, DeeObject *end, DeeObject *key, DeeObject *defl) {
		return inherit(DeeObject_CallAttrStringHashf(this, "blocate", _Dee_HashSelect(UINT32_C(0x7aa979d3), UINT64_C(0xbda91c237d69489e)), "o" DEE_PCKuSIZ "ooo", elem, start, end, key, defl));
	}
	WUNUSED NONNULL_CXX((1)) Ref<T> (blocate)(DeeObject *elem, size_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "blocate", _Dee_HashSelect(UINT32_C(0x7aa979d3), UINT64_C(0xbda91c237d69489e)), "o" DEE_PCKuSIZ DEE_PCKdSIZ, elem, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<T> (blocate)(DeeObject *elem, size_t start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "blocate", _Dee_HashSelect(UINT32_C(0x7aa979d3), UINT64_C(0xbda91c237d69489e)), "o" DEE_PCKuSIZ DEE_PCKdSIZ "o", elem, start, end, key));
	}
	WUNUSED NONNULL_CXX((1, 4, 5)) Ref<T> (blocate)(DeeObject *elem, size_t start, Dee_ssize_t end, DeeObject *key, DeeObject *defl) {
		return inherit(DeeObject_CallAttrStringHashf(this, "blocate", _Dee_HashSelect(UINT32_C(0x7aa979d3), UINT64_C(0xbda91c237d69489e)), "o" DEE_PCKuSIZ DEE_PCKdSIZ "oo", elem, start, end, key, defl));
	}
	WUNUSED NONNULL_CXX((1)) Ref<T> (blocate)(DeeObject *elem, size_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "blocate", _Dee_HashSelect(UINT32_C(0x7aa979d3), UINT64_C(0xbda91c237d69489e)), "o" DEE_PCKuSIZ DEE_PCKuSIZ, elem, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<T> (blocate)(DeeObject *elem, size_t start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "blocate", _Dee_HashSelect(UINT32_C(0x7aa979d3), UINT64_C(0xbda91c237d69489e)), "o" DEE_PCKuSIZ DEE_PCKuSIZ "o", elem, start, end, key));
	}
	WUNUSED NONNULL_CXX((1, 4, 5)) Ref<T> (blocate)(DeeObject *elem, size_t start, size_t end, DeeObject *key, DeeObject *defl) {
		return inherit(DeeObject_CallAttrStringHashf(this, "blocate", _Dee_HashSelect(UINT32_C(0x7aa979d3), UINT64_C(0xbda91c237d69489e)), "o" DEE_PCKuSIZ DEE_PCKuSIZ "oo", elem, start, end, key, defl));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Sequence<T> > (blocateall)(DeeObject *elem) {
		DeeObject *args[1];
		args[0] = elem;
		return inherit(DeeObject_CallAttrStringHash(this, "blocateall", _Dee_HashSelect(UINT32_C(0x6b34ef10), UINT64_C(0xa9049a4c0519981c)), 1, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<Sequence<T> > (blocateall)(DeeObject *elem, DeeObject *key_or_start) {
		DeeObject *args[2];
		args[0] = elem;
		args[1] = key_or_start;
		return inherit(DeeObject_CallAttrStringHash(this, "blocateall", _Dee_HashSelect(UINT32_C(0x6b34ef10), UINT64_C(0xa9049a4c0519981c)), 2, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3)) Ref<Sequence<T> > (blocateall)(DeeObject *elem, DeeObject *start, DeeObject *key_or_end) {
		DeeObject *args[3];
		args[0] = elem;
		args[1] = start;
		args[2] = key_or_end;
		return inherit(DeeObject_CallAttrStringHash(this, "blocateall", _Dee_HashSelect(UINT32_C(0x6b34ef10), UINT64_C(0xa9049a4c0519981c)), 3, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3, 4)) Ref<Sequence<T> > (blocateall)(DeeObject *elem, DeeObject *start, DeeObject *end, DeeObject *key) {
		DeeObject *args[4];
		args[0] = elem;
		args[1] = start;
		args[2] = end;
		args[3] = key;
		return inherit(DeeObject_CallAttrStringHash(this, "blocateall", _Dee_HashSelect(UINT32_C(0x6b34ef10), UINT64_C(0xa9049a4c0519981c)), 4, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<Sequence<T> > (blocateall)(DeeObject *elem, DeeObject *start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "blocateall", _Dee_HashSelect(UINT32_C(0x6b34ef10), UINT64_C(0xa9049a4c0519981c)), "oo" DEE_PCKdSIZ, elem, start, end));
	}
	WUNUSED NONNULL_CXX((1, 2, 4)) Ref<Sequence<T> > (blocateall)(DeeObject *elem, DeeObject *start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "blocateall", _Dee_HashSelect(UINT32_C(0x6b34ef10), UINT64_C(0xa9049a4c0519981c)), "oo" DEE_PCKdSIZ "o", elem, start, end, key));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<Sequence<T> > (blocateall)(DeeObject *elem, DeeObject *start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "blocateall", _Dee_HashSelect(UINT32_C(0x6b34ef10), UINT64_C(0xa9049a4c0519981c)), "oo" DEE_PCKuSIZ, elem, start, end));
	}
	WUNUSED NONNULL_CXX((1, 2, 4)) Ref<Sequence<T> > (blocateall)(DeeObject *elem, DeeObject *start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "blocateall", _Dee_HashSelect(UINT32_C(0x6b34ef10), UINT64_C(0xa9049a4c0519981c)), "oo" DEE_PCKuSIZ "o", elem, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Sequence<T> > (blocateall)(DeeObject *elem, Dee_ssize_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "blocateall", _Dee_HashSelect(UINT32_C(0x6b34ef10), UINT64_C(0xa9049a4c0519981c)), "o" DEE_PCKdSIZ, elem, start));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<Sequence<T> > (blocateall)(DeeObject *elem, Dee_ssize_t start, DeeObject *key_or_end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "blocateall", _Dee_HashSelect(UINT32_C(0x6b34ef10), UINT64_C(0xa9049a4c0519981c)), "o" DEE_PCKdSIZ "o", elem, start, key_or_end));
	}
	WUNUSED NONNULL_CXX((1, 3, 4)) Ref<Sequence<T> > (blocateall)(DeeObject *elem, Dee_ssize_t start, DeeObject *end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "blocateall", _Dee_HashSelect(UINT32_C(0x6b34ef10), UINT64_C(0xa9049a4c0519981c)), "o" DEE_PCKdSIZ "oo", elem, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Sequence<T> > (blocateall)(DeeObject *elem, Dee_ssize_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "blocateall", _Dee_HashSelect(UINT32_C(0x6b34ef10), UINT64_C(0xa9049a4c0519981c)), "o" DEE_PCKdSIZ DEE_PCKdSIZ, elem, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<Sequence<T> > (blocateall)(DeeObject *elem, Dee_ssize_t start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "blocateall", _Dee_HashSelect(UINT32_C(0x6b34ef10), UINT64_C(0xa9049a4c0519981c)), "o" DEE_PCKdSIZ DEE_PCKdSIZ "o", elem, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Sequence<T> > (blocateall)(DeeObject *elem, Dee_ssize_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "blocateall", _Dee_HashSelect(UINT32_C(0x6b34ef10), UINT64_C(0xa9049a4c0519981c)), "o" DEE_PCKdSIZ DEE_PCKuSIZ, elem, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<Sequence<T> > (blocateall)(DeeObject *elem, Dee_ssize_t start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "blocateall", _Dee_HashSelect(UINT32_C(0x6b34ef10), UINT64_C(0xa9049a4c0519981c)), "o" DEE_PCKdSIZ DEE_PCKuSIZ "o", elem, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Sequence<T> > (blocateall)(DeeObject *elem, size_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "blocateall", _Dee_HashSelect(UINT32_C(0x6b34ef10), UINT64_C(0xa9049a4c0519981c)), "o" DEE_PCKuSIZ, elem, start));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<Sequence<T> > (blocateall)(DeeObject *elem, size_t start, DeeObject *key_or_end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "blocateall", _Dee_HashSelect(UINT32_C(0x6b34ef10), UINT64_C(0xa9049a4c0519981c)), "o" DEE_PCKuSIZ "o", elem, start, key_or_end));
	}
	WUNUSED NONNULL_CXX((1, 3, 4)) Ref<Sequence<T> > (blocateall)(DeeObject *elem, size_t start, DeeObject *end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "blocateall", _Dee_HashSelect(UINT32_C(0x6b34ef10), UINT64_C(0xa9049a4c0519981c)), "o" DEE_PCKuSIZ "oo", elem, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Sequence<T> > (blocateall)(DeeObject *elem, size_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "blocateall", _Dee_HashSelect(UINT32_C(0x6b34ef10), UINT64_C(0xa9049a4c0519981c)), "o" DEE_PCKuSIZ DEE_PCKdSIZ, elem, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<Sequence<T> > (blocateall)(DeeObject *elem, size_t start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "blocateall", _Dee_HashSelect(UINT32_C(0x6b34ef10), UINT64_C(0xa9049a4c0519981c)), "o" DEE_PCKuSIZ DEE_PCKdSIZ "o", elem, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Sequence<T> > (blocateall)(DeeObject *elem, size_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "blocateall", _Dee_HashSelect(UINT32_C(0x6b34ef10), UINT64_C(0xa9049a4c0519981c)), "o" DEE_PCKuSIZ DEE_PCKuSIZ, elem, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<Sequence<T> > (blocateall)(DeeObject *elem, size_t start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "blocateall", _Dee_HashSelect(UINT32_C(0x6b34ef10), UINT64_C(0xa9049a4c0519981c)), "o" DEE_PCKuSIZ DEE_PCKuSIZ "o", elem, start, end, key));
	}
	NONNULL_CXX((1)) void (binsert)(DeeObject *elem) {
		DeeObject *args[1];
		args[0] = elem;
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "binsert", _Dee_HashSelect(UINT32_C(0xcad2e09b), UINT64_C(0xdd69512251d4a3f7)), 1, args)));
	}
	NONNULL_CXX((1, 2)) void (binsert)(DeeObject *elem, DeeObject *key) {
		DeeObject *args[2];
		args[0] = elem;
		args[1] = key;
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "binsert", _Dee_HashSelect(UINT32_C(0xcad2e09b), UINT64_C(0xdd69512251d4a3f7)), 2, args)));
	}
	WUNUSED NONNULL_CXX((1)) Ref<T> (at)(DeeObject *index) {
		DeeObject *args[1];
		args[0] = index;
		return inherit(DeeObject_CallAttrStringHash(this, "at", _Dee_HashSelect(UINT32_C(0xfe064fc3), UINT64_C(0x6375a48d1eff9d4e)), 1, args));
	}
	WUNUSED Ref<T> (at)(Dee_ssize_t index) {
		return inherit(DeeObject_CallAttrStringHashf(this, "at", _Dee_HashSelect(UINT32_C(0xfe064fc3), UINT64_C(0x6375a48d1eff9d4e)),  DEE_PCKdSIZ, index));
	}
	WUNUSED Ref<T> (at)(size_t index) {
		return inherit(DeeObject_CallAttrStringHashf(this, "at", _Dee_HashSelect(UINT32_C(0xfe064fc3), UINT64_C(0x6375a48d1eff9d4e)),  DEE_PCKuSIZ, index));
	}
	WUNUSED NONNULL_CXX((1)) Ref<T> (get)(DeeObject *index) {
		DeeObject *args[1];
		args[0] = index;
		return inherit(DeeObject_CallAttrStringHash(this, "get", _Dee_HashSelect(UINT32_C(0x3b6d35a2), UINT64_C(0x7c8e1568eac4979f)), 1, args));
	}
	WUNUSED Ref<T> (get)(Dee_ssize_t index) {
		return inherit(DeeObject_CallAttrStringHashf(this, "get", _Dee_HashSelect(UINT32_C(0x3b6d35a2), UINT64_C(0x7c8e1568eac4979f)),  DEE_PCKdSIZ, index));
	}
	WUNUSED Ref<T> (get)(size_t index) {
		return inherit(DeeObject_CallAttrStringHashf(this, "get", _Dee_HashSelect(UINT32_C(0x3b6d35a2), UINT64_C(0x7c8e1568eac4979f)),  DEE_PCKuSIZ, index));
	}
	class _Wrap_length
		: public deemon::detail::ConstGetRefProxy<_Wrap_length, deemon::int_> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap_length(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "length", _Dee_HashSelect(UINT32_C(0xecef0c1), UINT64_C(0x2993e8eb119cab21)));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "length", _Dee_HashSelect(UINT32_C(0xecef0c1), UINT64_C(0x2993e8eb119cab21))));
		}
	};
	WUNUSED _Wrap_length (length)() {
		return this;
	}
	class _Wrap_first
		: public deemon::detail::ConstGetRefProxy<_Wrap_first, T>
		, public deemon::detail::ConstSetRefProxy<_Wrap_first, T> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap_first(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "first", _Dee_HashSelect(UINT32_C(0xa9f0e818), UINT64_C(0x9d12a485470a29a7)));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "first", _Dee_HashSelect(UINT32_C(0xa9f0e818), UINT64_C(0x9d12a485470a29a7))));
		}
		void del() const {
			throw_if_nonzero(DeeObject_DelAttrStringHash(m_self, "first", _Dee_HashSelect(UINT32_C(0xa9f0e818), UINT64_C(0x9d12a485470a29a7))));
		}
		int _setref(DeeObject *value) const DEE_CXX_NOTHROW {
			return DeeObject_SetAttrStringHash(m_self, "first", _Dee_HashSelect(UINT32_C(0xa9f0e818), UINT64_C(0x9d12a485470a29a7)), value);
		}
	};
	WUNUSED _Wrap_first (first)() {
		return this;
	}
	class _Wrap_last
		: public deemon::detail::ConstGetRefProxy<_Wrap_last, T>
		, public deemon::detail::ConstSetRefProxy<_Wrap_last, T> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap_last(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "last", _Dee_HashSelect(UINT32_C(0x185a4f9a), UINT64_C(0x760894ca6d41e4dc)));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "last", _Dee_HashSelect(UINT32_C(0x185a4f9a), UINT64_C(0x760894ca6d41e4dc))));
		}
		void del() const {
			throw_if_nonzero(DeeObject_DelAttrStringHash(m_self, "last", _Dee_HashSelect(UINT32_C(0x185a4f9a), UINT64_C(0x760894ca6d41e4dc))));
		}
		int _setref(DeeObject *value) const DEE_CXX_NOTHROW {
			return DeeObject_SetAttrStringHash(m_self, "last", _Dee_HashSelect(UINT32_C(0x185a4f9a), UINT64_C(0x760894ca6d41e4dc)), value);
		}
	};
	WUNUSED _Wrap_last (last)() {
		return this;
	}
	class _Wrap_ismutable
		: public deemon::detail::ConstGetRefProxy<_Wrap_ismutable, deemon::bool_> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap_ismutable(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "ismutable", _Dee_HashSelect(UINT32_C(0x503e0af), UINT64_C(0x218befd96d70f7d4)));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "ismutable", _Dee_HashSelect(UINT32_C(0x503e0af), UINT64_C(0x218befd96d70f7d4))));
		}
	};
	WUNUSED _Wrap_ismutable (ismutable)() {
		return this;
	}
	class _Wrap_isresizable
		: public deemon::detail::ConstGetRefProxy<_Wrap_isresizable, deemon::bool_> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap_isresizable(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "isresizable", _Dee_HashSelect(UINT32_C(0x8e5e51ee), UINT64_C(0x11d0a53385154152)));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "isresizable", _Dee_HashSelect(UINT32_C(0x8e5e51ee), UINT64_C(0x11d0a53385154152))));
		}
	};
	WUNUSED _Wrap_isresizable (isresizable)() {
		return this;
	}
	class _Wrap_each
		: public deemon::detail::ConstGetRefProxy<_Wrap_each, Sequence<T> > {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap_each(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "each", _Dee_HashSelect(UINT32_C(0x9de8b13d), UINT64_C(0x374e052f37a5e158)));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "each", _Dee_HashSelect(UINT32_C(0x9de8b13d), UINT64_C(0x374e052f37a5e158))));
		}
	};
	WUNUSED _Wrap_each (each)() {
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
			return DeeObject_GetAttrStringHash(m_self, "ids", _Dee_HashSelect(UINT32_C(0x3173a48f), UINT64_C(0x7cd9fae6cf17bb9f)));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "ids", _Dee_HashSelect(UINT32_C(0x3173a48f), UINT64_C(0x7cd9fae6cf17bb9f))));
		}
	};
	WUNUSED _Wrap_ids (ids)() {
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
			return DeeObject_GetAttrStringHash(m_self, "types", _Dee_HashSelect(UINT32_C(0x871b2836), UINT64_C(0xde8693a2d24930)));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "types", _Dee_HashSelect(UINT32_C(0x871b2836), UINT64_C(0xde8693a2d24930))));
		}
	};
	WUNUSED _Wrap_types (types)() {
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
			return DeeObject_GetAttrStringHash(m_self, "classes", _Dee_HashSelect(UINT32_C(0x75e5899b), UINT64_C(0xc75d2d970415e4a0)));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "classes", _Dee_HashSelect(UINT32_C(0x75e5899b), UINT64_C(0xc75d2d970415e4a0))));
		}
	};
	WUNUSED _Wrap_classes (classes)() {
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
			return DeeObject_GetAttrStringHash(m_self, "isempty", _Dee_HashSelect(UINT32_C(0x693ca74b), UINT64_C(0xbdd6a8314c184088)));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "isempty", _Dee_HashSelect(UINT32_C(0x693ca74b), UINT64_C(0xbdd6a8314c184088))));
		}
	};
	WUNUSED _Wrap_isempty (isempty)() {
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
			return DeeObject_GetAttrStringHash(m_self, "isnonempty", _Dee_HashSelect(UINT32_C(0xde7e1cb1), UINT64_C(0x837d8f16bac4b317)));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "isnonempty", _Dee_HashSelect(UINT32_C(0xde7e1cb1), UINT64_C(0x837d8f16bac4b317))));
		}
	};
	WUNUSED _Wrap_isnonempty (isnonempty)() {
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
			return DeeObject_GetAttrStringHash(m_self, "isfrozen", _Dee_HashSelect(UINT32_C(0xb418a8ca), UINT64_C(0xa1babf165e1b1733)));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "isfrozen", _Dee_HashSelect(UINT32_C(0xb418a8ca), UINT64_C(0xa1babf165e1b1733))));
		}
	};
	WUNUSED _Wrap_isfrozen (isfrozen)() {
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
			return DeeObject_GetAttrStringHash(m_self, "frozen", _Dee_HashSelect(UINT32_C(0x82311b77), UINT64_C(0x7b55e2e6e642b6fd)));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "frozen", _Dee_HashSelect(UINT32_C(0x82311b77), UINT64_C(0x7b55e2e6e642b6fd))));
		}
	};
	WUNUSED _Wrap_frozen (frozen)() {
		return this;
	}
/*[[[end]]]*/
};

DEE_CXX_END

#endif /* !GUARD_DEEMON_CXX_SEQUENCE_H */
