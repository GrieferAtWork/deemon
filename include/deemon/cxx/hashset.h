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
#ifndef GUARD_DEEMON_CXX_HASHSET_H
#define GUARD_DEEMON_CXX_HASHSET_H 1

#include "api.h"
/**/

#include "object.h"
#include "set.h"
/**/

#include "../hashset.h"

DEE_CXX_BEGIN

template<class T = Object>
class HashSet: public Set<T> {
public:
	static WUNUSED Type &classtype() DEE_CXX_NOTHROW {
		return *(Type *)&DeeHashSet_Type;
	}
	static WUNUSED NONNULL_CXX((1)) bool check(DeeObject *ob) DEE_CXX_NOTHROW {
		return DeeHashSet_Check(ob);
	}
	static WUNUSED NONNULL_CXX((1)) bool checkexact(DeeObject *ob) DEE_CXX_NOTHROW {
		return DeeHashSet_CheckExact(ob);
	}

public:
	static Ref<HashSet<T> > of() DEE_CXX_NOTHROW {
		return inherit(DeeHashSet_New());
	}
	static NONNULL_CXX((1)) Ref<HashSet<T> > ofseq(DeeObject *seq) DEE_CXX_NOTHROW {
		return inherit(DeeHashSet_FromSequence(seq));
	}
	static NONNULL_CXX((1)) Ref<HashSet<T> > ofiter(DeeObject *iter) DEE_CXX_NOTHROW {
		return inherit(DeeHashSet_FromIterator(iter));
	}

public:
	NONNULL_CXX((1)) bool cinsert(DeeObject *item) {
		return throw_if_negative(DeeHashSet_Insert(this, item)) > 0;
	}
	NONNULL_CXX((1)) bool cinsert(char const *item) {
		return cinsert(item, strlen(item));
	}
	NONNULL_CXX((1)) bool cinsert(char const *item, size_t item_length) {
		return throw_if_negative(DeeHashSet_InsertString(this, item, item_length)) > 0;
	}
	NONNULL_CXX((1)) bool cremove(DeeObject *item) {
		return throw_if_negative(DeeHashSet_Remove(this, item)) > 0;
	}
	NONNULL_CXX((1)) bool cremove(char const *item) {
		return cremove(item, strlen(item));
	}
	NONNULL_CXX((1)) bool cremove(char const *item, size_t item_length) {
		return throw_if_negative(DeeHashSet_RemoveString(this, item, item_length)) > 0;
	}
	NONNULL_CXX((1)) bool ccontains(DeeObject *item) {
		return throw_if_negative(DeeHashSet_Contains(this, item)) > 0;
	}
	NONNULL_CXX((1)) bool ccontains(char const *item) {
		return ccontains(item, strlen(item));
	}
	NONNULL_CXX((1)) bool ccontains(char const *item, size_t item_length) {
		return throw_if_negative(DeeHashSet_ContainsString(this, item, item_length)) > 0;
	}
	NONNULL_CXX((1)) Ref<T> cunify(DeeObject *item) {
		return throw_if_negative(DeeHashSet_Unify(this, item)) > 0;
	}
	NONNULL_CXX((1)) Ref<string> cunify(char const *item) {
		return throw_if_negative(DeeHashSet_UnifyString(this, item, strlen(item))) > 0;
	}
	NONNULL_CXX((1)) Ref<string> cunify(char const *item, size_t item_length) {
		return throw_if_negative(DeeHashSet_UnifyString(this, item, item_length)) > 0;
	}

public:
/*[[[deemon (CxxType from rt.gen.cxxapi)(HashSet from deemon).printCxxApi(templateParameters: { "T" });]]]*/
	WUNUSED Ref<T> (pop)() {
		return inherit(DeeObject_CallAttrStringHash(this, "pop", _Dee_HashSelect(UINT32_C(0x960361ff), UINT64_C(0x666fb01461b0a0eb)), 0, NULL));
	}
	void (clear)() {
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "clear", _Dee_HashSelect(UINT32_C(0x7857faae), UINT64_C(0x22a34b6f82b3b83c)), 0, NULL)));
	}
	WUNUSED Ref<T> (popitem)() {
		return inherit(DeeObject_CallAttrStringHash(this, "popitem", _Dee_HashSelect(UINT32_C(0x40b249f3), UINT64_C(0x131a404a88439bc0)), 0, NULL));
	}
	WUNUSED NONNULL_CXX((1)) Ref<T> (unify)(DeeObject *ob) {
		DeeObject *args[1];
		args[0] = ob;
		return inherit(DeeObject_CallAttrStringHash(this, "unify", _Dee_HashSelect(UINT32_C(0x3cce686e), UINT64_C(0x4c0c9bdcc8d95cc7)), 1, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (insert)(DeeObject *ob) {
		DeeObject *args[1];
		args[0] = ob;
		return inherit(DeeObject_CallAttrStringHash(this, "insert", _Dee_HashSelect(UINT32_C(0x71d74a66), UINT64_C(0x5e168c86241590d7)), 1, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (update)(DeeObject *items) {
		DeeObject *args[1];
		args[0] = items;
		return inherit(DeeObject_CallAttrStringHash(this, "update", _Dee_HashSelect(UINT32_C(0xdf8e9237), UINT64_C(0x41c79529f2460018)), 1, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (insertall)(DeeObject *items) {
		DeeObject *args[1];
		args[0] = items;
		return inherit(DeeObject_CallAttrStringHash(this, "insertall", _Dee_HashSelect(UINT32_C(0xbf9bc3a9), UINT64_C(0x4f85971d093a27f2)), 1, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (remove)(DeeObject *ob) {
		DeeObject *args[1];
		args[0] = ob;
		return inherit(DeeObject_CallAttrStringHash(this, "remove", _Dee_HashSelect(UINT32_C(0x3d2727dd), UINT64_C(0xe9f313a03e2051a)), 1, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (add)(DeeObject *ob) {
		DeeObject *args[1];
		args[0] = ob;
		return inherit(DeeObject_CallAttrStringHash(this, "add", _Dee_HashSelect(UINT32_C(0xc72c9b19), UINT64_C(0x6fcfa13f924a5deb)), 1, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (discard)(DeeObject *ob) {
		DeeObject *args[1];
		args[0] = ob;
		return inherit(DeeObject_CallAttrStringHash(this, "discard", _Dee_HashSelect(UINT32_C(0x60249e06), UINT64_C(0xc8fe11c3fc0b52e6)), 1, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (insert_all)(DeeObject *ob) {
		DeeObject *args[1];
		args[0] = ob;
		return inherit(DeeObject_CallAttrStringHash(this, "insert_all", _Dee_HashSelect(UINT32_C(0xc6123bae), UINT64_C(0x6d5810c3bb999f3c)), 1, args));
	}
	class _Wrap_frozen
		: public deemon::detail::ConstGetRefProxy<_Wrap_frozen, Set<T> > {
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
	class _Wrap_max_load_factor
		: public deemon::detail::ConstGetRefProxy<_Wrap_max_load_factor, deemon::float_>
		, public deemon::detail::ConstSetRefProxy<_Wrap_max_load_factor, deemon::float_> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		using deemon::detail::ConstSetRefProxy<_Wrap_max_load_factor, deemon::float_>::operator =;
		_Wrap_max_load_factor(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "max_load_factor", _Dee_HashSelect(UINT32_C(0xae8f7ef7), UINT64_C(0xa0a4981439b4c20f)));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "max_load_factor", _Dee_HashSelect(UINT32_C(0xae8f7ef7), UINT64_C(0xa0a4981439b4c20f))));
		}
		void del() const {
			throw_if_nonzero(DeeObject_DelAttrStringHash(m_self, "max_load_factor", _Dee_HashSelect(UINT32_C(0xae8f7ef7), UINT64_C(0xa0a4981439b4c20f))));
		}
		int _setref(DeeObject *value) const DEE_CXX_NOTHROW {
			return DeeObject_SetAttrStringHash(m_self, "max_load_factor", _Dee_HashSelect(UINT32_C(0xae8f7ef7), UINT64_C(0xa0a4981439b4c20f)), value);
		}
	};
	WUNUSED _Wrap_max_load_factor (max_load_factor)() {
		return this;
	}
	class _Wrap___sizeof__
		: public deemon::detail::ConstGetRefProxy<_Wrap___sizeof__, deemon::int_> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap___sizeof__(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "__sizeof__", _Dee_HashSelect(UINT32_C(0x422f56f1), UINT64_C(0x4240f7a183278760)));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "__sizeof__", _Dee_HashSelect(UINT32_C(0x422f56f1), UINT64_C(0x4240f7a183278760))));
		}
	};
	WUNUSED _Wrap___sizeof__ (__sizeof__)() {
		return this;
	}
/*[[[end]]]*/
};

DEE_CXX_END

#endif /* !GUARD_DEEMON_CXX_HASHSET_H */
