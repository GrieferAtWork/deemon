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
#ifndef GUARD_DEEMON_CXX_SET_H
#define GUARD_DEEMON_CXX_SET_H 1

#include "api.h"
/**/

#include "object.h"
#include "sequence.h"
/**/

#include "../set.h"

DEE_CXX_BEGIN

template<class T = Object>
class Set: public Sequence<T> {
public:
	static WUNUSED Type &classtype() DEE_CXX_NOTHROW {
		return *(Type *)&DeeSet_Type;
	}
	static WUNUSED NONNULL_CXX((1)) bool check(DeeObject *ob) DEE_CXX_NOTHROW {
		return DeeObject_InstanceOf(ob, &DeeSet_Type);
	}
	static WUNUSED NONNULL_CXX((1)) bool checkexact(DeeObject *ob) DEE_CXX_NOTHROW {
		return DeeObject_InstanceOfExact(ob, &DeeSet_Type);
	}

public:
	static NONNULL_CXX((1)) Ref<Set<T> > of() DEE_CXX_NOTHROW {
		return nonnull(Dee_EmptySet);
	}

public:
/*[[[deemon (CxxType from rt.gen.cxxapi)(Set from deemon).printCxxApi(templateParameters: { "T" });]]]*/
	WUNUSED NONNULL_CXX((1)) Ref<Set<T> > (difference)(DeeObject *to) {
		DeeObject *args[1];
		args[0] = to;
		return inherit(DeeObject_CallAttrStringHash(this, "difference", _Dee_HashSelectC(0xe944baff, 0x6add28d83d2e1f6e), 1, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Set<T> > (intersection)(DeeObject *with_) {
		DeeObject *args[1];
		args[0] = with_;
		return inherit(DeeObject_CallAttrStringHash(this, "intersection", _Dee_HashSelectC(0xabaa0afa, 0xc72d025e185198b7), 1, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (isdisjoint)(DeeObject *with_) {
		DeeObject *args[1];
		args[0] = with_;
		return inherit(DeeObject_CallAttrStringHash(this, "isdisjoint", _Dee_HashSelectC(0x4b97e75a, 0x14d2b48b4b9da607), 1, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Set<T> > (union_)(DeeObject *with_) {
		DeeObject *args[1];
		args[0] = with_;
		return inherit(DeeObject_CallAttrStringHash(this, "union", _Dee_HashSelectC(0x23b88b9b, 0x3b416e7d690babb2), 1, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Set<T> > (symmetric_difference)(DeeObject *with_) {
		DeeObject *args[1];
		args[0] = with_;
		return inherit(DeeObject_CallAttrStringHash(this, "symmetric_difference", _Dee_HashSelectC(0x9a1e5057, 0x17b1425a414674d3), 1, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (issubset)(DeeObject *of) {
		DeeObject *args[1];
		args[0] = of;
		return inherit(DeeObject_CallAttrStringHash(this, "issubset", _Dee_HashSelectC(0xac6aa1c0, 0x49ece9bed26428cf), 1, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (issuperset)(DeeObject *of) {
		DeeObject *args[1];
		args[0] = of;
		return inherit(DeeObject_CallAttrStringHash(this, "issuperset", _Dee_HashSelectC(0x55780f5f, 0x7f578be05d081a7f), 1, args));
	}
	class _Wrap_frozen
		: public deemon::detail::ConstGetRefProxy<_Wrap_frozen, Set<T> > {
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
	WUNUSED _Wrap_frozen (frozen)() {
		return this;
	}
/*[[[end]]]*/
};

DEE_CXX_END

#endif /* !GUARD_DEEMON_CXX_SET_H */
