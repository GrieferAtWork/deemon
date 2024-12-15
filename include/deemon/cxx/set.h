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
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (insert)(DeeObject *key) {
		DeeObject *args[1];
		args[0] = key;
		return inherit(DeeObject_CallAttrStringHash(this, "insert", _Dee_HashSelectC(0x71d74a66, 0x5e168c86241590d7), 1, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (remove)(DeeObject *key) {
		DeeObject *args[1];
		args[0] = key;
		return inherit(DeeObject_CallAttrStringHash(this, "remove", _Dee_HashSelectC(0x3d2727dd, 0xe9f313a03e2051a), 1, args));
	}
	NONNULL_CXX((1)) void (insertall)(DeeObject *keys) {
		DeeObject *args[1];
		args[0] = keys;
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "insertall", _Dee_HashSelectC(0xbf9bc3a9, 0x4f85971d093a27f2), 1, args)));
	}
	NONNULL_CXX((1)) void (removeall)(DeeObject *keys) {
		DeeObject *args[1];
		args[0] = keys;
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "removeall", _Dee_HashSelectC(0x902407ed, 0x97879af70abc9349), 1, args)));
	}
	WUNUSED NONNULL_CXX((1)) Ref<T> (unify)(DeeObject *key) {
		DeeObject *args[1];
		args[0] = key;
		return inherit(DeeObject_CallAttrStringHash(this, "unify", _Dee_HashSelectC(0x3cce686e, 0x4c0c9bdcc8d95cc7), 1, args));
	}
	WUNUSED Ref<T> (pop)() {
		return inherit(DeeObject_CallAttrStringHash(this, "pop", _Dee_HashSelectC(0x960361ff, 0x666fb01461b0a0eb), 0, NULL));
	}
	WUNUSED NONNULL_CXX((1)) Ref<T> (pop)(DeeObject *def) {
		DeeObject *args[1];
		args[0] = def;
		return inherit(DeeObject_CallAttrStringHash(this, "pop", _Dee_HashSelectC(0x960361ff, 0x666fb01461b0a0eb), 1, args));
	}
	WUNUSED Ref<Iterator<T> > (__iter__)() {
		return inherit(DeeObject_CallAttrStringHash(this, "__iter__", _Dee_HashSelectC(0x4ae49b3, 0x29df7f8a609cead7), 0, NULL));
	}
	WUNUSED Ref<deemon::int_> (__size__)() {
		return inherit(DeeObject_CallAttrStringHash(this, "__size__", _Dee_HashSelectC(0x543ba3b5, 0xd416117435cce357), 0, NULL));
	}
	WUNUSED NONNULL_CXX((1)) Ref<T> (__foreach__)(DeeObject *cb) {
		DeeObject *args[1];
		args[0] = cb;
		return inherit(DeeObject_CallAttrStringHash(this, "__foreach__", _Dee_HashSelectC(0xeb324f7f, 0xa61da94d66bf93e9), 1, args));
	}
	WUNUSED Ref<deemon::int_> (__hash__)() {
		return inherit(DeeObject_CallAttrStringHash(this, "__hash__", _Dee_HashSelectC(0xc088645e, 0xbc5b5b1504b9d2d8), 0, NULL));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (__compare_eq__)(DeeObject *rhs) {
		DeeObject *args[1];
		args[0] = rhs;
		return inherit(DeeObject_CallAttrStringHash(this, "__compare_eq__", _Dee_HashSelectC(0xe8a4d608, 0x1e72244b6194ba57), 1, args));
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
	WUNUSED Ref<Set<T> > (__inv__)() {
		return inherit(DeeObject_CallAttrStringHash(this, "__inv__", _Dee_HashSelectC(0x63d81258, 0xef0370cd96d93e7e), 0, NULL));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Set<T> > (__add__)(DeeObject *rhs) {
		DeeObject *args[1];
		args[0] = rhs;
		return inherit(DeeObject_CallAttrStringHash(this, "__add__", _Dee_HashSelectC(0x5cb4d11a, 0x6f33a2bc44b51c54), 1, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Set<T> > (__sub__)(DeeObject *rhs) {
		DeeObject *args[1];
		args[0] = rhs;
		return inherit(DeeObject_CallAttrStringHash(this, "__sub__", _Dee_HashSelectC(0xc2239a1e, 0xd91dc2370225ae2f), 1, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Set<T> > (__and__)(DeeObject *rhs) {
		DeeObject *args[1];
		args[0] = rhs;
		return inherit(DeeObject_CallAttrStringHash(this, "__and__", _Dee_HashSelectC(0xac39cb48, 0x2b28cb619a45a71e), 1, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Set<T> > (__xor__)(DeeObject *rhs) {
		DeeObject *args[1];
		args[0] = rhs;
		return inherit(DeeObject_CallAttrStringHash(this, "__xor__", _Dee_HashSelectC(0x7378854c, 0x4a8410f65a74106f), 1, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Set<T> > (__inplace_add__)(DeeObject *rhs) {
		DeeObject *args[1];
		args[0] = rhs;
		return inherit(DeeObject_CallAttrStringHash(this, "__inplace_add__", _Dee_HashSelectC(0x28e8178a, 0x4185ae98a0cae056), 1, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Set<T> > (__inplace_sub__)(DeeObject *rhs) {
		DeeObject *args[1];
		args[0] = rhs;
		return inherit(DeeObject_CallAttrStringHash(this, "__inplace_sub__", _Dee_HashSelectC(0x6df12ae4, 0x8d0a0a61a536e34c), 1, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Set<T> > (__inplace_and__)(DeeObject *rhs) {
		DeeObject *args[1];
		args[0] = rhs;
		return inherit(DeeObject_CallAttrStringHash(this, "__inplace_and__", _Dee_HashSelectC(0x346d8846, 0x338c9c23ba4fea0e), 1, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Set<T> > (__inplace_xor__)(DeeObject *rhs) {
		DeeObject *args[1];
		args[0] = rhs;
		return inherit(DeeObject_CallAttrStringHash(this, "__inplace_xor__", _Dee_HashSelectC(0xea50ca60, 0xd8f62f1d57ea2d1d), 1, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Set<T> > (__or__)(DeeObject *rhs) {
		DeeObject *args[1];
		args[0] = rhs;
		return inherit(DeeObject_CallAttrStringHash(this, "__or__", _Dee_HashSelectC(0xf95e054c, 0x2bc6caacde6c129e), 1, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Set<T> > (__inplace_or__)(DeeObject *rhs) {
		DeeObject *args[1];
		args[0] = rhs;
		return inherit(DeeObject_CallAttrStringHash(this, "__inplace_or__", _Dee_HashSelectC(0x2ebbbb19, 0xe07a9de59b5dfa47), 1, args));
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
	WUNUSED _Wrap_frozen (frozen)() DEE_CXX_NOTHROW {
		return this;
	}
/*[[[end]]]*/
};

DEE_CXX_END

#endif /* !GUARD_DEEMON_CXX_SET_H */
