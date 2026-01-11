/* Copyright (c) 2018-2026 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2026 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_CXX_ITERATOR_H
#define GUARD_DEEMON_CXX_ITERATOR_H 1

#include "api.h"
/**/

#include "numeric.h"
#include "object.h"
/**/

#include "../format.h"
/**/

#include "../seq.h"

DEE_CXX_BEGIN

template<class T>
class Iterator
	: public Object
{
public:
	static WUNUSED Type &classtype() DEE_CXX_NOTHROW {
		return *(Type *)&DeeIterator_Type;
	}
	static WUNUSED NONNULL_CXX((1)) bool check(DeeObject *ob) DEE_CXX_NOTHROW {
		return DeeObject_InstanceOf(ob, &DeeIterator_Type);
	}
	static WUNUSED NONNULL_CXX((1)) bool checkexact(DeeObject *ob) DEE_CXX_NOTHROW {
		return DeeObject_InstanceOfExact(ob, &DeeIterator_Type);
	}

public:
/*[[[deemon (CxxType from rt.gen.cxxapi)(Iterator from deemon).printCxxApi(templateParameters: { "T" });]]]*/
	WUNUSED Ref<T> (next)() {
		return inherit(DeeObject_CallAttrStringHash(this, "next", _Dee_HashSelectC(0x7e5e1569, 0x2a31f6a650652d33), 0, NULL));
	}
	WUNUSED NONNULL_CXX((1)) Ref<T> (next)(DeeObject *defl) {
		DeeObject *args[1];
		args[0] = defl;
		return inherit(DeeObject_CallAttrStringHash(this, "next", _Dee_HashSelectC(0x7e5e1569, 0x2a31f6a650652d33), 1, args));
	}
	WUNUSED Ref<T> (peek)() {
		return inherit(DeeObject_CallAttrStringHash(this, "peek", _Dee_HashSelectC(0xb2ae48a2, 0xcc667a4d924a91f8), 0, NULL));
	}
	WUNUSED NONNULL_CXX((1)) Ref<T> (peek)(DeeObject *defl) {
		DeeObject *args[1];
		args[0] = defl;
		return inherit(DeeObject_CallAttrStringHash(this, "peek", _Dee_HashSelectC(0xb2ae48a2, 0xcc667a4d924a91f8), 1, args));
	}
	WUNUSED Ref<deemon::bool_> (prev)() {
		return inherit(DeeObject_CallAttrStringHash(this, "prev", _Dee_HashSelectC(0xeb31683d, 0x7487ec947044729e), 0, NULL));
	}
	void (rewind)() {
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "rewind", _Dee_HashSelectC(0x2ab1b235, 0xa35b8bb3941ca25f), 0, NULL)));
	}
	NONNULL_CXX((1)) void (revert)(DeeObject *step) {
		DeeObject *args[1];
		args[0] = step;
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "revert", _Dee_HashSelectC(0x98ca826, 0x626b4fca0d39dcf2), 1, args)));
	}
	void (revert)(Dee_ssize_t step) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "revert", _Dee_HashSelectC(0x98ca826, 0x626b4fca0d39dcf2), Dee_PCKdSIZ, step)));
	}
	void (revert)(size_t step) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "revert", _Dee_HashSelectC(0x98ca826, 0x626b4fca0d39dcf2), Dee_PCKuSIZ, step)));
	}
	NONNULL_CXX((1)) void (advance)(DeeObject *step) {
		DeeObject *args[1];
		args[0] = step;
		decref(throw_if_null(DeeObject_CallAttrStringHash(this, "advance", _Dee_HashSelectC(0xdd1157a0, 0x8667ad2c6ab8d35d), 1, args)));
	}
	void (advance)(Dee_ssize_t step) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "advance", _Dee_HashSelectC(0xdd1157a0, 0x8667ad2c6ab8d35d), Dee_PCKdSIZ, step)));
	}
	void (advance)(size_t step) {
		decref(throw_if_null(DeeObject_CallAttrStringHashf(this, "advance", _Dee_HashSelectC(0xdd1157a0, 0x8667ad2c6ab8d35d), Dee_PCKuSIZ, step)));
	}
	class _Wrap_seq
		: public deemon::detail::ConstGetRefProxy<_Wrap_seq, Sequence<T> > {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap_seq(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "seq", _Dee_HashSelectC(0x232af2b7, 0x80a0b0950a5a5251));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "seq", _Dee_HashSelectC(0x232af2b7, 0x80a0b0950a5a5251)));
		}
	};
	WUNUSED _Wrap_seq (seq)() DEE_CXX_NOTHROW {
		return this;
	}
	class _Wrap_future
		: public deemon::detail::ConstGetRefProxy<_Wrap_future, Sequence<T> > {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap_future(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "future", _Dee_HashSelectC(0x5ca3159c, 0x8ab2926ab5959525));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "future", _Dee_HashSelectC(0x5ca3159c, 0x8ab2926ab5959525)));
		}
	};
	WUNUSED _Wrap_future (future)() DEE_CXX_NOTHROW {
		return this;
	}
	class _Wrap_pending
		: public deemon::detail::ConstGetRefProxy<_Wrap_pending, Sequence<T> > {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap_pending(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "pending", _Dee_HashSelectC(0xa318502a, 0x9f3f699bf5a1e785));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "pending", _Dee_HashSelectC(0xa318502a, 0x9f3f699bf5a1e785)));
		}
	};
	WUNUSED _Wrap_pending (pending)() DEE_CXX_NOTHROW {
		return this;
	}
	class _Wrap_isbidirectional
		: public deemon::detail::ConstGetRefProxy<_Wrap_isbidirectional, deemon::bool_> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap_isbidirectional(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "isbidirectional", _Dee_HashSelectC(0x11d9fc36, 0x192134b5c30b96b7));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "isbidirectional", _Dee_HashSelectC(0x11d9fc36, 0x192134b5c30b96b7)));
		}
	};
	WUNUSED _Wrap_isbidirectional (isbidirectional)() DEE_CXX_NOTHROW {
		return this;
	}
	class _Wrap_hasprev
		: public deemon::detail::ConstGetRefProxy<_Wrap_hasprev, deemon::bool_> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap_hasprev(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "hasprev", _Dee_HashSelectC(0xe7e8f3c, 0x17b364986c9ecd3b));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "hasprev", _Dee_HashSelectC(0xe7e8f3c, 0x17b364986c9ecd3b)));
		}
	};
	WUNUSED _Wrap_hasprev (hasprev)() DEE_CXX_NOTHROW {
		return this;
	}
	class _Wrap_hasnext
		: public deemon::detail::ConstGetRefProxy<_Wrap_hasnext, deemon::bool_> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap_hasnext(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "hasnext", _Dee_HashSelectC(0xae2186a8, 0x19d7bd95854b765f));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "hasnext", _Dee_HashSelectC(0xae2186a8, 0x19d7bd95854b765f)));
		}
	};
	WUNUSED _Wrap_hasnext (hasnext)() DEE_CXX_NOTHROW {
		return this;
	}
	class _Wrap_index
		: public deemon::detail::ConstGetRefProxy<_Wrap_index, deemon::int_>
		, public deemon::detail::ConstSetRefProxy<_Wrap_index, deemon::int_> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		using deemon::detail::ConstSetRefProxy<_Wrap_index, deemon::int_>::operator =;
		_Wrap_index(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "index", _Dee_HashSelectC(0x77f34f0, 0x440d5888c0ff3081));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "index", _Dee_HashSelectC(0x77f34f0, 0x440d5888c0ff3081)));
		}
		int _setref(DeeObject *value) const DEE_CXX_NOTHROW {
			return DeeObject_SetAttrStringHash(m_self, "index", _Dee_HashSelectC(0x77f34f0, 0x440d5888c0ff3081), value);
		}
	};
	WUNUSED _Wrap_index (index)() DEE_CXX_NOTHROW {
		return this;
	}
/*[[[end]]]*/
};

DEE_CXX_END

#endif /* !GUARD_DEEMON_CXX_ITERATOR_H */
