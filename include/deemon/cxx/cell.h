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
#ifndef GUARD_DEEMON_CXX_CELL_H
#define GUARD_DEEMON_CXX_CELL_H 1

#include "api.h"
/**/

#include "object.h"
#include "sequence.h"
/**/

#include "../cell.h"

DEE_CXX_BEGIN

template<class T>
class Cell: public Sequence<T> {
public:
	static WUNUSED Type &classtype() DEE_CXX_NOTHROW {
		return *(Type *)&DeeCell_Type;
	}
	static WUNUSED NONNULL_CXX((1)) bool check(DeeObject *ob) DEE_CXX_NOTHROW {
		return DeeObject_InstanceOf(ob, &DeeCell_Type);
	}
	static WUNUSED NONNULL_CXX((1)) bool checkexact(DeeObject *ob) DEE_CXX_NOTHROW {
		return DeeObject_InstanceOfExact(ob, &DeeCell_Type);
	}

public:
	static NONNULL_CXX((1)) Ref<Cell<T> > of() DEE_CXX_NOTHROW {
		return inherit(DeeCell_NewEmpty());
	}
	static NONNULL_CXX((1)) Ref<Cell<T> > of(DeeObject *ob) DEE_CXX_NOTHROW {
		return inherit(DeeCell_New(ob));
	}

public:
	/*nullable*/ Ref<T> ctryget() DEE_CXX_NOTHROW {
		return inherit(maybenull(DeeCell_TryGet(this)));
	}
	Ref<T> cget() {
		return inherit(DeeCell_Get(this));
	}
	void cdel() {
		throw_if_nonzero(DeeCell_Del(this));
	}
	void cset(/*nullable*/ DeeObject *value) {
		throw_if_nonzero(DeeCell_Set(this, value));
	}
	/*nullable*/ Ref<T> cxch(/*nullable*/ DeeObject *value) {
		return inherit(maybenull(DeeCell_Xch(this, value)));
	}
	/*nullable*/ Ref<T> cxch_nonnull(/*nullable*/ DeeObject *value) {
		return inherit(maybenull(DeeCell_XchNonNull(this, value)));
	}
	/*nullable*/ Ref<T> ccmpxch(/*nullable*/ DeeObject *old_value,
	                            /*nullable*/ DeeObject *new_value) {
		return inherit(maybenull(DeeCell_CmpXch(this, old_value, DeeObject *new_value)));
	}

public:
/*[[[deemon (CxxType from rt.gen.cxxapi)(Cell from deemon).printCxxApi(templateParameters: { "T" });]]]*/
	WUNUSED NONNULL_CXX((1)) Ref<T> (get)(DeeObject *def) {
		DeeObject *args[1];
		args[0] = def;
		return inherit(DeeObject_CallAttrStringHash(this, "get", _Dee_HashSelect(UINT32_C(0x3b6d35a2), UINT64_C(0x7c8e1568eac4979f)), 1, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<T> (pop)(DeeObject *def) {
		DeeObject *args[1];
		args[0] = def;
		return inherit(DeeObject_CallAttrStringHash(this, "pop", _Dee_HashSelect(UINT32_C(0x960361ff), UINT64_C(0x666fb01461b0a0eb)), 1, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (set)(DeeObject *value) {
		DeeObject *args[1];
		args[0] = value;
		return inherit(DeeObject_CallAttrStringHash(this, "set", _Dee_HashSelect(UINT32_C(0x5ecc6fe8), UINT64_C(0xe706aa03fdbe04fa)), 1, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<T> (xch)(DeeObject *value) {
		DeeObject *args[1];
		args[0] = value;
		return inherit(DeeObject_CallAttrStringHash(this, "xch", _Dee_HashSelect(UINT32_C(0x818ce38a), UINT64_C(0x6bb37305be1b0321)), 1, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<T> (xch)(DeeObject *value, DeeObject *def) {
		DeeObject *args[2];
		args[0] = value;
		args[1] = def;
		return inherit(DeeObject_CallAttrStringHash(this, "xch", _Dee_HashSelect(UINT32_C(0x818ce38a), UINT64_C(0x6bb37305be1b0321)), 2, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (cmpdel)(DeeObject *old_value) {
		DeeObject *args[1];
		args[0] = old_value;
		return inherit(DeeObject_CallAttrStringHash(this, "cmpdel", _Dee_HashSelect(UINT32_C(0x9f2753d6), UINT64_C(0xb34dad8ca05ec81d)), 1, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (cmpxch)(DeeObject *new_value) {
		DeeObject *args[1];
		args[0] = new_value;
		return inherit(DeeObject_CallAttrStringHash(this, "cmpxch", _Dee_HashSelect(UINT32_C(0x3988d00b), UINT64_C(0xc8716ef5297eda10)), 1, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<T> (cmpxch)(DeeObject *old_value, DeeObject *new_value) {
		DeeObject *args[2];
		args[0] = old_value;
		args[1] = new_value;
		return inherit(DeeObject_CallAttrStringHash(this, "cmpxch", _Dee_HashSelect(UINT32_C(0x3988d00b), UINT64_C(0xc8716ef5297eda10)), 2, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3)) Ref<T> (cmpxch)(DeeObject *old_value, DeeObject *new_value, DeeObject *def) {
		DeeObject *args[3];
		args[0] = old_value;
		args[1] = new_value;
		args[2] = def;
		return inherit(DeeObject_CallAttrStringHash(this, "cmpxch", _Dee_HashSelect(UINT32_C(0x3988d00b), UINT64_C(0xc8716ef5297eda10)), 3, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::bool_> (cmpset)(DeeObject *old_value) {
		DeeObject *args[1];
		args[0] = old_value;
		return inherit(DeeObject_CallAttrStringHash(this, "cmpset", _Dee_HashSelect(UINT32_C(0xebdea141), UINT64_C(0xaa28c2c4dbba2ceb)), 1, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::bool_> (cmpset)(DeeObject *old_value, DeeObject *new_value) {
		DeeObject *args[2];
		args[0] = old_value;
		args[1] = new_value;
		return inherit(DeeObject_CallAttrStringHash(this, "cmpset", _Dee_HashSelect(UINT32_C(0xebdea141), UINT64_C(0xaa28c2c4dbba2ceb)), 2, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<T> (exchange)(DeeObject *value) {
		DeeObject *args[1];
		args[0] = value;
		return inherit(DeeObject_CallAttrStringHash(this, "exchange", _Dee_HashSelect(UINT32_C(0xc02f258), UINT64_C(0xf1932a0dd229fa54)), 1, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<T> (exchange)(DeeObject *value, DeeObject *def) {
		DeeObject *args[2];
		args[0] = value;
		args[1] = def;
		return inherit(DeeObject_CallAttrStringHash(this, "exchange", _Dee_HashSelect(UINT32_C(0xc02f258), UINT64_C(0xf1932a0dd229fa54)), 2, args));
	}
	class _Wrap_value
		: public deemon::detail::ConstGetRefProxy<_Wrap_value, T>
		, public deemon::detail::ConstSetRefProxy<_Wrap_value, T> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap_value(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "value", _Dee_HashSelect(UINT32_C(0xd9093f6e), UINT64_C(0x69e7413ae0c88471)));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "value", _Dee_HashSelect(UINT32_C(0xd9093f6e), UINT64_C(0x69e7413ae0c88471))));
		}
		void del() const {
			throw_if_nonzero(DeeObject_DelAttrStringHash(m_self, "value", _Dee_HashSelect(UINT32_C(0xd9093f6e), UINT64_C(0x69e7413ae0c88471))));
		}
		int _setref(DeeObject *value) const DEE_CXX_NOTHROW {
			return DeeObject_SetAttrStringHash(m_self, "value", _Dee_HashSelect(UINT32_C(0xd9093f6e), UINT64_C(0x69e7413ae0c88471)), value);
		}
	};
	WUNUSED _Wrap_value (value)() {
		return this;
	}
/*[[[end]]]*/
};

DEE_CXX_END

#endif /* !GUARD_DEEMON_CXX_CELL_H */
