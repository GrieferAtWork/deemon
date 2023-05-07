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
#ifndef GUARD_DEEMON_CXX_TUPLE_H
#define GUARD_DEEMON_CXX_TUPLE_H 1

#include "api.h"
/**/

#include "object.h"
#include "sequence.h"
/**/

#include "../format.h"
#include "../tuple.h"
/**/

DEE_CXX_BEGIN

namespace detail {
template<size_t Index, class ...Types>
struct _AbstractTupleType {};
template<class Type, class ...MoreTypes>
struct _AbstractTupleType<0, Type, MoreTypes...> {
	enum { valid = true };
	typedef Type type;
};
template<size_t Index, class Type, class ...MoreTypes>
struct _AbstractTupleType<Index, Type, MoreTypes...>
	: public _AbstractTupleType<Index - 1, MoreTypes...> {};


template<size_t Index, class ...Types>
class _AbstractTupleItemWrapper
	: public _AbstractTupleType<Index, Types...>
	, public deemon::detail::ConstGetAndSetRefProxyWithDefault<
		_AbstractTupleItemWrapper<Index, Types...>,
		typename detail::_AbstractTupleType<Index, Types...>::type>
{
private:
	DeeObject *m_self; /* [1..1] Linked object */
public:
	_AbstractTupleItemWrapper(DeeObject *self) DEE_CXX_NOTHROW
		: m_self(self) {}
	WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
		return DeeObject_GetItemIndex(m_self, Index);
	}
	WUNUSED DREF DeeObject *_getref(DeeObject *def) const DEE_CXX_NOTHROW {
		DREF DeeObject *index_ob, *result = NULL;
		index_ob = DeeInt_NewSize(Index);
		if likely(index_ob) {
			result = DeeObject_GetItemDef(m_self, index_ob, def);
			Dee_Decref(index_ob);
		}
		return result;
	}
	WUNUSED int _setref(DeeObject *value) const DEE_CXX_NOTHROW {
		return DeeObject_SetItemIndex(m_self, Index, value);
	}
	bool has() const {
		return throw_if_negative(DeeObject_HasItemIndex(m_self, Index)) != 0;
	}
	bool bound(bool allow_missing = true) const {
		return throw_if_minusone(DeeObject_BoundItemIndex(m_self, Index, allow_missing)) > 0;
	}
	void del() const {
		throw_if_nonzero(DeeObject_DelItemIndex(m_self, Index));
	}
};


template<class TupleType>
class _TupleComponentFunctionsBase {
public:
	ATTR_RETNONNULL WUNUSED DeeObject *_citem(size_t index) const DEE_CXX_NOTHROW {
		DeeObject_AssertTypeExact(((TupleType *)this)->ptr(), &DeeTuple_Type);
		Dee_ASSERT(index < DeeTuple_SIZE(((TupleType *)this)->ptr()));
		return DeeTuple_GET(((TupleType *)this)->ptr(), index);
	}
};


template<class TupleType, class ...Types>
class _AbstractTupleComponentFunctions;
template<class TupleType, class ...Types>
class _TupleComponentFunctions;

/*[[[deemon
global final N = 10;
for (local n: [:N+1]) {
	print("template<class TupleType"),;
	for (local i: [:n])
		print(", class T", i + 1),;
	print(">");
	print("class _AbstractTupleComponentFunctions<TupleType"),;
	for (local i: [:n])
		print(", T", i + 1),;
	print(">"),;
	if (n) {
		print;
		print("	: public _AbstractTupleComponentFunctions<TupleType"),;
		for (local i: [:n - 1])
			print(", T", i + 1),;
		print(">");
	} else {
		print(" "),;
	}
	print("{");
	if (n) {
		print("public:");
		print("	_AbstractTupleItemWrapper<", n - 1),;
		for (local i: [:n])
			print(", T", i + 1),;
		print("> component", n, "() DEE_CXX_NOTHROW {");
		print("		return ((TupleType *)this)->ptr();");
		print("	}");
	}
	print("};");
}
print("template<class TupleType, class ...Types>");
print("class _AbstractTupleComponentFunctions");
print("	: public _AbstractTupleComponentFunctions<TupleType");
for (local n: [:N])
	print("		, typename _AbstractTupleType<", n, ", Types...>::type");
print("	>");
print("{");
print("};");

for (local n: [:N+1]) {
	print("template<class TupleType"),;
	for (local i: [:n])
		print(", class T", i + 1),;
	print(">");
	print("class _TupleComponentFunctions<TupleType"),;
	for (local i: [:n])
		print(", T", i + 1),;
	print(">");
	if (n) {
		print("	: public _TupleComponentFunctions<TupleType"),;
		for (local i: [:n - 1])
			print(", T", i + 1),;
		print(">");
	} else {
		print("	: public _TupleComponentFunctionsBase<TupleType>");
	}
	print("{");
	if (n) {
		print("public:");
		print("	T", n, " &ccomponent", n, "() const DEE_CXX_NOTHROW {");
		print("		return *(T", n, " *)this->_citem(", n - 1, ");");
		print("	}");
	}
	print("};");
}
print("template<class TupleType, class ...Types>");
print("class _TupleComponentFunctions");
print("	: public _TupleComponentFunctions<TupleType");
for (local n: [:N])
	print("		, typename _AbstractTupleType<", n, ", Types...>::type");
print("	>");
print("{");
print("};");
]]]*/
template<class TupleType>
class _AbstractTupleComponentFunctions<TupleType> {
};
template<class TupleType, class T1>
class _AbstractTupleComponentFunctions<TupleType, T1>
	: public _AbstractTupleComponentFunctions<TupleType>
{
public:
	_AbstractTupleItemWrapper<0, T1> component1() DEE_CXX_NOTHROW {
		return ((TupleType *)this)->ptr();
	}
};
template<class TupleType, class T1, class T2>
class _AbstractTupleComponentFunctions<TupleType, T1, T2>
	: public _AbstractTupleComponentFunctions<TupleType, T1>
{
public:
	_AbstractTupleItemWrapper<1, T1, T2> component2() DEE_CXX_NOTHROW {
		return ((TupleType *)this)->ptr();
	}
};
template<class TupleType, class T1, class T2, class T3>
class _AbstractTupleComponentFunctions<TupleType, T1, T2, T3>
	: public _AbstractTupleComponentFunctions<TupleType, T1, T2>
{
public:
	_AbstractTupleItemWrapper<2, T1, T2, T3> component3() DEE_CXX_NOTHROW {
		return ((TupleType *)this)->ptr();
	}
};
template<class TupleType, class T1, class T2, class T3, class T4>
class _AbstractTupleComponentFunctions<TupleType, T1, T2, T3, T4>
	: public _AbstractTupleComponentFunctions<TupleType, T1, T2, T3>
{
public:
	_AbstractTupleItemWrapper<3, T1, T2, T3, T4> component4() DEE_CXX_NOTHROW {
		return ((TupleType *)this)->ptr();
	}
};
template<class TupleType, class T1, class T2, class T3, class T4, class T5>
class _AbstractTupleComponentFunctions<TupleType, T1, T2, T3, T4, T5>
	: public _AbstractTupleComponentFunctions<TupleType, T1, T2, T3, T4>
{
public:
	_AbstractTupleItemWrapper<4, T1, T2, T3, T4, T5> component5() DEE_CXX_NOTHROW {
		return ((TupleType *)this)->ptr();
	}
};
template<class TupleType, class T1, class T2, class T3, class T4, class T5, class T6>
class _AbstractTupleComponentFunctions<TupleType, T1, T2, T3, T4, T5, T6>
	: public _AbstractTupleComponentFunctions<TupleType, T1, T2, T3, T4, T5>
{
public:
	_AbstractTupleItemWrapper<5, T1, T2, T3, T4, T5, T6> component6() DEE_CXX_NOTHROW {
		return ((TupleType *)this)->ptr();
	}
};
template<class TupleType, class T1, class T2, class T3, class T4, class T5, class T6, class T7>
class _AbstractTupleComponentFunctions<TupleType, T1, T2, T3, T4, T5, T6, T7>
	: public _AbstractTupleComponentFunctions<TupleType, T1, T2, T3, T4, T5, T6>
{
public:
	_AbstractTupleItemWrapper<6, T1, T2, T3, T4, T5, T6, T7> component7() DEE_CXX_NOTHROW {
		return ((TupleType *)this)->ptr();
	}
};
template<class TupleType, class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8>
class _AbstractTupleComponentFunctions<TupleType, T1, T2, T3, T4, T5, T6, T7, T8>
	: public _AbstractTupleComponentFunctions<TupleType, T1, T2, T3, T4, T5, T6, T7>
{
public:
	_AbstractTupleItemWrapper<7, T1, T2, T3, T4, T5, T6, T7, T8> component8() DEE_CXX_NOTHROW {
		return ((TupleType *)this)->ptr();
	}
};
template<class TupleType, class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9>
class _AbstractTupleComponentFunctions<TupleType, T1, T2, T3, T4, T5, T6, T7, T8, T9>
	: public _AbstractTupleComponentFunctions<TupleType, T1, T2, T3, T4, T5, T6, T7, T8>
{
public:
	_AbstractTupleItemWrapper<8, T1, T2, T3, T4, T5, T6, T7, T8, T9> component9() DEE_CXX_NOTHROW {
		return ((TupleType *)this)->ptr();
	}
};
template<class TupleType, class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10>
class _AbstractTupleComponentFunctions<TupleType, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10>
	: public _AbstractTupleComponentFunctions<TupleType, T1, T2, T3, T4, T5, T6, T7, T8, T9>
{
public:
	_AbstractTupleItemWrapper<9, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10> component10() DEE_CXX_NOTHROW {
		return ((TupleType *)this)->ptr();
	}
};
template<class TupleType, class ...Types>
class _AbstractTupleComponentFunctions
	: public _AbstractTupleComponentFunctions<TupleType
		, typename _AbstractTupleType<0, Types...>::type
		, typename _AbstractTupleType<1, Types...>::type
		, typename _AbstractTupleType<2, Types...>::type
		, typename _AbstractTupleType<3, Types...>::type
		, typename _AbstractTupleType<4, Types...>::type
		, typename _AbstractTupleType<5, Types...>::type
		, typename _AbstractTupleType<6, Types...>::type
		, typename _AbstractTupleType<7, Types...>::type
		, typename _AbstractTupleType<8, Types...>::type
		, typename _AbstractTupleType<9, Types...>::type
	>
{
};
template<class TupleType>
class _TupleComponentFunctions<TupleType>
	: public _TupleComponentFunctionsBase<TupleType>
{
};
template<class TupleType, class T1>
class _TupleComponentFunctions<TupleType, T1>
	: public _TupleComponentFunctions<TupleType>
{
public:
	T1 &ccomponent1() const DEE_CXX_NOTHROW {
		return *(T1 *)this->_citem(0);
	}
};
template<class TupleType, class T1, class T2>
class _TupleComponentFunctions<TupleType, T1, T2>
	: public _TupleComponentFunctions<TupleType, T1>
{
public:
	T2 &ccomponent2() const DEE_CXX_NOTHROW {
		return *(T2 *)this->_citem(1);
	}
};
template<class TupleType, class T1, class T2, class T3>
class _TupleComponentFunctions<TupleType, T1, T2, T3>
	: public _TupleComponentFunctions<TupleType, T1, T2>
{
public:
	T3 &ccomponent3() const DEE_CXX_NOTHROW {
		return *(T3 *)this->_citem(2);
	}
};
template<class TupleType, class T1, class T2, class T3, class T4>
class _TupleComponentFunctions<TupleType, T1, T2, T3, T4>
	: public _TupleComponentFunctions<TupleType, T1, T2, T3>
{
public:
	T4 &ccomponent4() const DEE_CXX_NOTHROW {
		return *(T4 *)this->_citem(3);
	}
};
template<class TupleType, class T1, class T2, class T3, class T4, class T5>
class _TupleComponentFunctions<TupleType, T1, T2, T3, T4, T5>
	: public _TupleComponentFunctions<TupleType, T1, T2, T3, T4>
{
public:
	T5 &ccomponent5() const DEE_CXX_NOTHROW {
		return *(T5 *)this->_citem(4);
	}
};
template<class TupleType, class T1, class T2, class T3, class T4, class T5, class T6>
class _TupleComponentFunctions<TupleType, T1, T2, T3, T4, T5, T6>
	: public _TupleComponentFunctions<TupleType, T1, T2, T3, T4, T5>
{
public:
	T6 &ccomponent6() const DEE_CXX_NOTHROW {
		return *(T6 *)this->_citem(5);
	}
};
template<class TupleType, class T1, class T2, class T3, class T4, class T5, class T6, class T7>
class _TupleComponentFunctions<TupleType, T1, T2, T3, T4, T5, T6, T7>
	: public _TupleComponentFunctions<TupleType, T1, T2, T3, T4, T5, T6>
{
public:
	T7 &ccomponent7() const DEE_CXX_NOTHROW {
		return *(T7 *)this->_citem(6);
	}
};
template<class TupleType, class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8>
class _TupleComponentFunctions<TupleType, T1, T2, T3, T4, T5, T6, T7, T8>
	: public _TupleComponentFunctions<TupleType, T1, T2, T3, T4, T5, T6, T7>
{
public:
	T8 &ccomponent8() const DEE_CXX_NOTHROW {
		return *(T8 *)this->_citem(7);
	}
};
template<class TupleType, class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9>
class _TupleComponentFunctions<TupleType, T1, T2, T3, T4, T5, T6, T7, T8, T9>
	: public _TupleComponentFunctions<TupleType, T1, T2, T3, T4, T5, T6, T7, T8>
{
public:
	T9 &ccomponent9() const DEE_CXX_NOTHROW {
		return *(T9 *)this->_citem(8);
	}
};
template<class TupleType, class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10>
class _TupleComponentFunctions<TupleType, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10>
	: public _TupleComponentFunctions<TupleType, T1, T2, T3, T4, T5, T6, T7, T8, T9>
{
public:
	T10 &ccomponent10() const DEE_CXX_NOTHROW {
		return *(T10 *)this->_citem(9);
	}
};
template<class TupleType, class ...Types>
class _TupleComponentFunctions
	: public _TupleComponentFunctions<TupleType
		, typename _AbstractTupleType<0, Types...>::type
		, typename _AbstractTupleType<1, Types...>::type
		, typename _AbstractTupleType<2, Types...>::type
		, typename _AbstractTupleType<3, Types...>::type
		, typename _AbstractTupleType<4, Types...>::type
		, typename _AbstractTupleType<5, Types...>::type
		, typename _AbstractTupleType<6, Types...>::type
		, typename _AbstractTupleType<7, Types...>::type
		, typename _AbstractTupleType<8, Types...>::type
		, typename _AbstractTupleType<9, Types...>::type
	>
{
};
/*[[[end]]]*/

} /* namespace detail */

template<class ...Types>
class _AbstractTuple
	: public Sequence<deemon::Object>
	, public detail::_AbstractTupleComponentFunctions<_AbstractTuple<Types...>, Types...>
{
public:
	template<size_t Index>
	typename std::enable_if<detail::_AbstractTupleItemWrapper<Index, Types...>::valid,
	                        detail::_AbstractTupleItemWrapper<Index, Types...> >::type
	item() DEE_CXX_NOTHROW {
		return this;
	}
};

template<class ...Types>
class Tuple
	: public _AbstractTuple<Types...>
	, public detail::_TupleComponentFunctions<Tuple<Types...>, Types...>
{
public:
	static WUNUSED Type &classtype() DEE_CXX_NOTHROW {
		return *(Type *)&DeeTuple_Type;
	}
	static WUNUSED NONNULL_CXX((1)) bool check(DeeObject *ob) DEE_CXX_NOTHROW {
		return DeeTuple_Check(ob);
	}
	static WUNUSED NONNULL_CXX((1)) bool checkexact(DeeObject *ob) DEE_CXX_NOTHROW {
		return DeeTuple_CheckExact(ob);
	}
public:
	/* TODO: of() */

public:
	template<size_t Index>
	typename detail::_AbstractTupleType<Index, Types...>::type &
	citem() const DEE_CXX_NOTHROW {
		DeeObject_AssertTypeExact(this, &DeeTuple_Type);
		Dee_ASSERT(Index < DeeTuple_SIZE(this));
		return *(typename detail::_AbstractTupleType<Index, Types...>::type *)DeeTuple_GET(this, Index);
	}
	size_t csize() const DEE_CXX_NOTHROW {
		return DeeTuple_SIZE(this);
	}
	DeeObject *const *celem() const DEE_CXX_NOTHROW {
		return DeeTuple_ELEM(this);
	}
	DeeObject *cget(size_t index) const DEE_CXX_NOTHROW {
		return DeeTuple_GET(this, index);
	}
	void cset(size_t index, DeeObject *value) DEE_CXX_NOTHROW {
		DeeTuple_SET(this, index, value);
	}

public:
/*[[[deemon (CxxType from rt.gen.cxxapi)(Tuple from deemon).printCxxApi();]]]*/
	class _Wrap_first
		: public deemon::detail::ConstGetRefProxy<_Wrap_first, Object> {
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
	};
	WUNUSED _Wrap_first (first)() {
		return this;
	}
	class _Wrap_last
		: public deemon::detail::ConstGetRefProxy<_Wrap_last, Object> {
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
	};
	WUNUSED _Wrap_last (last)() {
		return this;
	}
	class _Wrap_frozen
		: public deemon::detail::ConstGetRefProxy<_Wrap_frozen, Sequence<> > {
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

#endif /* !GUARD_DEEMON_CXX_TUPLE_H */
