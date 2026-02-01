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
#ifndef GUARD_DEEMON_CXX_TUPLE_H
#define GUARD_DEEMON_CXX_TUPLE_H 1

#include "../api.h"
#include "api.h"

#include "../format.h" /* Dee_PCKdSIZ, Dee_PCKuSIZ */
#include "../object.h" /* DeeObject_* */
#include "../tuple.h"  /* DeeTuple* */
#include "../types.h"  /* DREF, DeeObject, Dee_ssize_t, _Dee_HashSelectC */
#include "object.h"
#include "sequence.h"

#include <stdbool.h>   /* bool, true */
#include <stddef.h>    /* NULL, size_t */
#include <type_traits> /* std::enable_if */

DEE_CXX_BEGIN

class Type;
class int_;

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
	WUNUSED DREF DeeObject *_trygetref() const DEE_CXX_NOTHROW {
		return DeeObject_TryGetItemIndex(m_self, Index);
	}
	WUNUSED int _setref(DeeObject *value) const DEE_CXX_NOTHROW {
		return DeeObject_SetItemIndex(m_self, Index, value);
	}
	bool has() const {
		return throw_if_negative(DeeObject_HasItemIndex(m_self, Index)) != 0;
	}
	bool bound() const {
		return throw_if_minusone(DeeObject_BoundItemIndex(m_self, Index)) > 0;
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
		return inherit(DeeObject_CallAttrStringHashf(this, "find", _Dee_HashSelectC(0x9e66372, 0x2b65fe03bbdde5b2), "oo" Dee_PCKdSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 2, 4)) Ref<deemon::int_> (find)(DeeObject *item, DeeObject *start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "find", _Dee_HashSelectC(0x9e66372, 0x2b65fe03bbdde5b2), "oo" Dee_PCKdSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::int_> (find)(DeeObject *item, DeeObject *start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "find", _Dee_HashSelectC(0x9e66372, 0x2b65fe03bbdde5b2), "oo" Dee_PCKuSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 2, 4)) Ref<deemon::int_> (find)(DeeObject *item, DeeObject *start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "find", _Dee_HashSelectC(0x9e66372, 0x2b65fe03bbdde5b2), "oo" Dee_PCKuSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (find)(DeeObject *item, Dee_ssize_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "find", _Dee_HashSelectC(0x9e66372, 0x2b65fe03bbdde5b2), "o" Dee_PCKdSIZ, item, start));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::int_> (find)(DeeObject *item, Dee_ssize_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "find", _Dee_HashSelectC(0x9e66372, 0x2b65fe03bbdde5b2), "o" Dee_PCKdSIZ "o", item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 3, 4)) Ref<deemon::int_> (find)(DeeObject *item, Dee_ssize_t start, DeeObject *end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "find", _Dee_HashSelectC(0x9e66372, 0x2b65fe03bbdde5b2), "o" Dee_PCKdSIZ "oo", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (find)(DeeObject *item, Dee_ssize_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "find", _Dee_HashSelectC(0x9e66372, 0x2b65fe03bbdde5b2), "o" Dee_PCKdSIZ Dee_PCKdSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::int_> (find)(DeeObject *item, Dee_ssize_t start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "find", _Dee_HashSelectC(0x9e66372, 0x2b65fe03bbdde5b2), "o" Dee_PCKdSIZ Dee_PCKdSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (find)(DeeObject *item, Dee_ssize_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "find", _Dee_HashSelectC(0x9e66372, 0x2b65fe03bbdde5b2), "o" Dee_PCKdSIZ Dee_PCKuSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::int_> (find)(DeeObject *item, Dee_ssize_t start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "find", _Dee_HashSelectC(0x9e66372, 0x2b65fe03bbdde5b2), "o" Dee_PCKdSIZ Dee_PCKuSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (find)(DeeObject *item, size_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "find", _Dee_HashSelectC(0x9e66372, 0x2b65fe03bbdde5b2), "o" Dee_PCKuSIZ, item, start));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::int_> (find)(DeeObject *item, size_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "find", _Dee_HashSelectC(0x9e66372, 0x2b65fe03bbdde5b2), "o" Dee_PCKuSIZ "o", item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 3, 4)) Ref<deemon::int_> (find)(DeeObject *item, size_t start, DeeObject *end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "find", _Dee_HashSelectC(0x9e66372, 0x2b65fe03bbdde5b2), "o" Dee_PCKuSIZ "oo", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (find)(DeeObject *item, size_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "find", _Dee_HashSelectC(0x9e66372, 0x2b65fe03bbdde5b2), "o" Dee_PCKuSIZ Dee_PCKdSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::int_> (find)(DeeObject *item, size_t start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "find", _Dee_HashSelectC(0x9e66372, 0x2b65fe03bbdde5b2), "o" Dee_PCKuSIZ Dee_PCKdSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (find)(DeeObject *item, size_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "find", _Dee_HashSelectC(0x9e66372, 0x2b65fe03bbdde5b2), "o" Dee_PCKuSIZ Dee_PCKuSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::int_> (find)(DeeObject *item, size_t start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "find", _Dee_HashSelectC(0x9e66372, 0x2b65fe03bbdde5b2), "o" Dee_PCKuSIZ Dee_PCKuSIZ "o", item, start, end, key));
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
		return inherit(DeeObject_CallAttrStringHashf(this, "rfind", _Dee_HashSelectC(0xfb368ca, 0x8b40ffa7172dc59d), "oo" Dee_PCKdSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 2, 4)) Ref<deemon::int_> (rfind)(DeeObject *item, DeeObject *start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rfind", _Dee_HashSelectC(0xfb368ca, 0x8b40ffa7172dc59d), "oo" Dee_PCKdSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<deemon::int_> (rfind)(DeeObject *item, DeeObject *start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rfind", _Dee_HashSelectC(0xfb368ca, 0x8b40ffa7172dc59d), "oo" Dee_PCKuSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 2, 4)) Ref<deemon::int_> (rfind)(DeeObject *item, DeeObject *start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rfind", _Dee_HashSelectC(0xfb368ca, 0x8b40ffa7172dc59d), "oo" Dee_PCKuSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (rfind)(DeeObject *item, Dee_ssize_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rfind", _Dee_HashSelectC(0xfb368ca, 0x8b40ffa7172dc59d), "o" Dee_PCKdSIZ, item, start));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::int_> (rfind)(DeeObject *item, Dee_ssize_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rfind", _Dee_HashSelectC(0xfb368ca, 0x8b40ffa7172dc59d), "o" Dee_PCKdSIZ "o", item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 3, 4)) Ref<deemon::int_> (rfind)(DeeObject *item, Dee_ssize_t start, DeeObject *end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rfind", _Dee_HashSelectC(0xfb368ca, 0x8b40ffa7172dc59d), "o" Dee_PCKdSIZ "oo", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (rfind)(DeeObject *item, Dee_ssize_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rfind", _Dee_HashSelectC(0xfb368ca, 0x8b40ffa7172dc59d), "o" Dee_PCKdSIZ Dee_PCKdSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::int_> (rfind)(DeeObject *item, Dee_ssize_t start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rfind", _Dee_HashSelectC(0xfb368ca, 0x8b40ffa7172dc59d), "o" Dee_PCKdSIZ Dee_PCKdSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (rfind)(DeeObject *item, Dee_ssize_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rfind", _Dee_HashSelectC(0xfb368ca, 0x8b40ffa7172dc59d), "o" Dee_PCKdSIZ Dee_PCKuSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::int_> (rfind)(DeeObject *item, Dee_ssize_t start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rfind", _Dee_HashSelectC(0xfb368ca, 0x8b40ffa7172dc59d), "o" Dee_PCKdSIZ Dee_PCKuSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (rfind)(DeeObject *item, size_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rfind", _Dee_HashSelectC(0xfb368ca, 0x8b40ffa7172dc59d), "o" Dee_PCKuSIZ, item, start));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<deemon::int_> (rfind)(DeeObject *item, size_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rfind", _Dee_HashSelectC(0xfb368ca, 0x8b40ffa7172dc59d), "o" Dee_PCKuSIZ "o", item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 3, 4)) Ref<deemon::int_> (rfind)(DeeObject *item, size_t start, DeeObject *end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rfind", _Dee_HashSelectC(0xfb368ca, 0x8b40ffa7172dc59d), "o" Dee_PCKuSIZ "oo", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (rfind)(DeeObject *item, size_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rfind", _Dee_HashSelectC(0xfb368ca, 0x8b40ffa7172dc59d), "o" Dee_PCKuSIZ Dee_PCKdSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::int_> (rfind)(DeeObject *item, size_t start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rfind", _Dee_HashSelectC(0xfb368ca, 0x8b40ffa7172dc59d), "o" Dee_PCKuSIZ Dee_PCKdSIZ "o", item, start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<deemon::int_> (rfind)(DeeObject *item, size_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rfind", _Dee_HashSelectC(0xfb368ca, 0x8b40ffa7172dc59d), "o" Dee_PCKuSIZ Dee_PCKuSIZ, item, start, end));
	}
	WUNUSED NONNULL_CXX((1, 4)) Ref<deemon::int_> (rfind)(DeeObject *item, size_t start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "rfind", _Dee_HashSelectC(0xfb368ca, 0x8b40ffa7172dc59d), "o" Dee_PCKuSIZ Dee_PCKuSIZ "o", item, start, end, key));
	}
	WUNUSED Ref<Sequence<> > (sorted)() {
		return inherit(DeeObject_CallAttrStringHash(this, "sorted", _Dee_HashSelectC(0x93fb6d4, 0x2fc60c43cfaf0860), 0, NULL));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Sequence<> > (sorted)(DeeObject *start) {
		DeeObject *args[1];
		args[0] = start;
		return inherit(DeeObject_CallAttrStringHash(this, "sorted", _Dee_HashSelectC(0x93fb6d4, 0x2fc60c43cfaf0860), 1, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<Sequence<> > (sorted)(DeeObject *start, DeeObject *end) {
		DeeObject *args[2];
		args[0] = start;
		args[1] = end;
		return inherit(DeeObject_CallAttrStringHash(this, "sorted", _Dee_HashSelectC(0x93fb6d4, 0x2fc60c43cfaf0860), 2, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3)) Ref<Sequence<> > (sorted)(DeeObject *start, DeeObject *end, DeeObject *key) {
		DeeObject *args[3];
		args[0] = start;
		args[1] = end;
		args[2] = key;
		return inherit(DeeObject_CallAttrStringHash(this, "sorted", _Dee_HashSelectC(0x93fb6d4, 0x2fc60c43cfaf0860), 3, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Sequence<> > (sorted)(DeeObject *start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "sorted", _Dee_HashSelectC(0x93fb6d4, 0x2fc60c43cfaf0860), "o" Dee_PCKdSIZ, start, end));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<Sequence<> > (sorted)(DeeObject *start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "sorted", _Dee_HashSelectC(0x93fb6d4, 0x2fc60c43cfaf0860), "o" Dee_PCKdSIZ "o", start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Sequence<> > (sorted)(DeeObject *start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "sorted", _Dee_HashSelectC(0x93fb6d4, 0x2fc60c43cfaf0860), "o" Dee_PCKuSIZ, start, end));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<Sequence<> > (sorted)(DeeObject *start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "sorted", _Dee_HashSelectC(0x93fb6d4, 0x2fc60c43cfaf0860), "o" Dee_PCKuSIZ "o", start, end, key));
	}
	WUNUSED Ref<Sequence<> > (sorted)(Dee_ssize_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "sorted", _Dee_HashSelectC(0x93fb6d4, 0x2fc60c43cfaf0860), Dee_PCKdSIZ, start));
	}
	WUNUSED NONNULL_CXX((2)) Ref<Sequence<> > (sorted)(Dee_ssize_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "sorted", _Dee_HashSelectC(0x93fb6d4, 0x2fc60c43cfaf0860), Dee_PCKdSIZ "o", start, end));
	}
	WUNUSED NONNULL_CXX((2, 3)) Ref<Sequence<> > (sorted)(Dee_ssize_t start, DeeObject *end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "sorted", _Dee_HashSelectC(0x93fb6d4, 0x2fc60c43cfaf0860), Dee_PCKdSIZ "oo", start, end, key));
	}
	WUNUSED Ref<Sequence<> > (sorted)(Dee_ssize_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "sorted", _Dee_HashSelectC(0x93fb6d4, 0x2fc60c43cfaf0860), Dee_PCKdSIZ Dee_PCKdSIZ, start, end));
	}
	WUNUSED NONNULL_CXX((3)) Ref<Sequence<> > (sorted)(Dee_ssize_t start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "sorted", _Dee_HashSelectC(0x93fb6d4, 0x2fc60c43cfaf0860), Dee_PCKdSIZ Dee_PCKdSIZ "o", start, end, key));
	}
	WUNUSED Ref<Sequence<> > (sorted)(Dee_ssize_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "sorted", _Dee_HashSelectC(0x93fb6d4, 0x2fc60c43cfaf0860), Dee_PCKdSIZ Dee_PCKuSIZ, start, end));
	}
	WUNUSED NONNULL_CXX((3)) Ref<Sequence<> > (sorted)(Dee_ssize_t start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "sorted", _Dee_HashSelectC(0x93fb6d4, 0x2fc60c43cfaf0860), Dee_PCKdSIZ Dee_PCKuSIZ "o", start, end, key));
	}
	WUNUSED Ref<Sequence<> > (sorted)(size_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "sorted", _Dee_HashSelectC(0x93fb6d4, 0x2fc60c43cfaf0860), Dee_PCKuSIZ, start));
	}
	WUNUSED NONNULL_CXX((2)) Ref<Sequence<> > (sorted)(size_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "sorted", _Dee_HashSelectC(0x93fb6d4, 0x2fc60c43cfaf0860), Dee_PCKuSIZ "o", start, end));
	}
	WUNUSED NONNULL_CXX((2, 3)) Ref<Sequence<> > (sorted)(size_t start, DeeObject *end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "sorted", _Dee_HashSelectC(0x93fb6d4, 0x2fc60c43cfaf0860), Dee_PCKuSIZ "oo", start, end, key));
	}
	WUNUSED Ref<Sequence<> > (sorted)(size_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "sorted", _Dee_HashSelectC(0x93fb6d4, 0x2fc60c43cfaf0860), Dee_PCKuSIZ Dee_PCKdSIZ, start, end));
	}
	WUNUSED NONNULL_CXX((3)) Ref<Sequence<> > (sorted)(size_t start, Dee_ssize_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "sorted", _Dee_HashSelectC(0x93fb6d4, 0x2fc60c43cfaf0860), Dee_PCKuSIZ Dee_PCKdSIZ "o", start, end, key));
	}
	WUNUSED Ref<Sequence<> > (sorted)(size_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "sorted", _Dee_HashSelectC(0x93fb6d4, 0x2fc60c43cfaf0860), Dee_PCKuSIZ Dee_PCKuSIZ, start, end));
	}
	WUNUSED NONNULL_CXX((3)) Ref<Sequence<> > (sorted)(size_t start, size_t end, DeeObject *key) {
		return inherit(DeeObject_CallAttrStringHashf(this, "sorted", _Dee_HashSelectC(0x93fb6d4, 0x2fc60c43cfaf0860), Dee_PCKuSIZ Dee_PCKuSIZ "o", start, end, key));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Sequence<> > (unpack)(DeeObject *length) {
		DeeObject *args[1];
		args[0] = length;
		return inherit(DeeObject_CallAttrStringHash(this, "unpack", _Dee_HashSelectC(0x579a82d1, 0x2714bac446bf9c9), 1, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<Sequence<> > (unpack)(DeeObject *min, DeeObject *max) {
		DeeObject *args[2];
		args[0] = min;
		args[1] = max;
		return inherit(DeeObject_CallAttrStringHash(this, "unpack", _Dee_HashSelectC(0x579a82d1, 0x2714bac446bf9c9), 2, args));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Sequence<> > (unpack)(DeeObject *min, Dee_ssize_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "unpack", _Dee_HashSelectC(0x579a82d1, 0x2714bac446bf9c9), "o" Dee_PCKdSIZ, min, max));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Sequence<> > (unpack)(DeeObject *min, size_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "unpack", _Dee_HashSelectC(0x579a82d1, 0x2714bac446bf9c9), "o" Dee_PCKuSIZ, min, max));
	}
	WUNUSED Ref<Sequence<> > (unpack)(Dee_ssize_t length) {
		return inherit(DeeObject_CallAttrStringHashf(this, "unpack", _Dee_HashSelectC(0x579a82d1, 0x2714bac446bf9c9), Dee_PCKdSIZ, length));
	}
	WUNUSED NONNULL_CXX((2)) Ref<Sequence<> > (unpack)(Dee_ssize_t min, DeeObject *max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "unpack", _Dee_HashSelectC(0x579a82d1, 0x2714bac446bf9c9), Dee_PCKdSIZ "o", min, max));
	}
	WUNUSED Ref<Sequence<> > (unpack)(Dee_ssize_t min, Dee_ssize_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "unpack", _Dee_HashSelectC(0x579a82d1, 0x2714bac446bf9c9), Dee_PCKdSIZ Dee_PCKdSIZ, min, max));
	}
	WUNUSED Ref<Sequence<> > (unpack)(Dee_ssize_t min, size_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "unpack", _Dee_HashSelectC(0x579a82d1, 0x2714bac446bf9c9), Dee_PCKdSIZ Dee_PCKuSIZ, min, max));
	}
	WUNUSED Ref<Sequence<> > (unpack)(size_t length) {
		return inherit(DeeObject_CallAttrStringHashf(this, "unpack", _Dee_HashSelectC(0x579a82d1, 0x2714bac446bf9c9), Dee_PCKuSIZ, length));
	}
	WUNUSED NONNULL_CXX((2)) Ref<Sequence<> > (unpack)(size_t min, DeeObject *max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "unpack", _Dee_HashSelectC(0x579a82d1, 0x2714bac446bf9c9), Dee_PCKuSIZ "o", min, max));
	}
	WUNUSED Ref<Sequence<> > (unpack)(size_t min, Dee_ssize_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "unpack", _Dee_HashSelectC(0x579a82d1, 0x2714bac446bf9c9), Dee_PCKuSIZ Dee_PCKdSIZ, min, max));
	}
	WUNUSED Ref<Sequence<> > (unpack)(size_t min, size_t max) {
		return inherit(DeeObject_CallAttrStringHashf(this, "unpack", _Dee_HashSelectC(0x579a82d1, 0x2714bac446bf9c9), Dee_PCKuSIZ Dee_PCKuSIZ, min, max));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Object> (__seq_enumerate__)(DeeObject *cb) {
		DeeObject *args[1];
		args[0] = cb;
		return inherit(DeeObject_CallAttrStringHash(this, "__seq_enumerate__", _Dee_HashSelectC(0x2daa9e99, 0xca96f5a312eda247), 1, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<Object> (__seq_enumerate__)(DeeObject *cb, DeeObject *start) {
		DeeObject *args[2];
		args[0] = cb;
		args[1] = start;
		return inherit(DeeObject_CallAttrStringHash(this, "__seq_enumerate__", _Dee_HashSelectC(0x2daa9e99, 0xca96f5a312eda247), 2, args));
	}
	WUNUSED NONNULL_CXX((1, 2, 3)) Ref<Object> (__seq_enumerate__)(DeeObject *cb, DeeObject *start, DeeObject *end) {
		DeeObject *args[3];
		args[0] = cb;
		args[1] = start;
		args[2] = end;
		return inherit(DeeObject_CallAttrStringHash(this, "__seq_enumerate__", _Dee_HashSelectC(0x2daa9e99, 0xca96f5a312eda247), 3, args));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<Object> (__seq_enumerate__)(DeeObject *cb, DeeObject *start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__seq_enumerate__", _Dee_HashSelectC(0x2daa9e99, 0xca96f5a312eda247), "oo" Dee_PCKdSIZ, cb, start, end));
	}
	WUNUSED NONNULL_CXX((1, 2)) Ref<Object> (__seq_enumerate__)(DeeObject *cb, DeeObject *start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__seq_enumerate__", _Dee_HashSelectC(0x2daa9e99, 0xca96f5a312eda247), "oo" Dee_PCKuSIZ, cb, start, end));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Object> (__seq_enumerate__)(DeeObject *cb, Dee_ssize_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__seq_enumerate__", _Dee_HashSelectC(0x2daa9e99, 0xca96f5a312eda247), "o" Dee_PCKdSIZ, cb, start));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<Object> (__seq_enumerate__)(DeeObject *cb, Dee_ssize_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__seq_enumerate__", _Dee_HashSelectC(0x2daa9e99, 0xca96f5a312eda247), "o" Dee_PCKdSIZ "o", cb, start, end));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Object> (__seq_enumerate__)(DeeObject *cb, Dee_ssize_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__seq_enumerate__", _Dee_HashSelectC(0x2daa9e99, 0xca96f5a312eda247), "o" Dee_PCKdSIZ Dee_PCKdSIZ, cb, start, end));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Object> (__seq_enumerate__)(DeeObject *cb, Dee_ssize_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__seq_enumerate__", _Dee_HashSelectC(0x2daa9e99, 0xca96f5a312eda247), "o" Dee_PCKdSIZ Dee_PCKuSIZ, cb, start, end));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Object> (__seq_enumerate__)(DeeObject *cb, size_t start) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__seq_enumerate__", _Dee_HashSelectC(0x2daa9e99, 0xca96f5a312eda247), "o" Dee_PCKuSIZ, cb, start));
	}
	WUNUSED NONNULL_CXX((1, 3)) Ref<Object> (__seq_enumerate__)(DeeObject *cb, size_t start, DeeObject *end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__seq_enumerate__", _Dee_HashSelectC(0x2daa9e99, 0xca96f5a312eda247), "o" Dee_PCKuSIZ "o", cb, start, end));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Object> (__seq_enumerate__)(DeeObject *cb, size_t start, Dee_ssize_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__seq_enumerate__", _Dee_HashSelectC(0x2daa9e99, 0xca96f5a312eda247), "o" Dee_PCKuSIZ Dee_PCKdSIZ, cb, start, end));
	}
	WUNUSED NONNULL_CXX((1)) Ref<Object> (__seq_enumerate__)(DeeObject *cb, size_t start, size_t end) {
		return inherit(DeeObject_CallAttrStringHashf(this, "__seq_enumerate__", _Dee_HashSelectC(0x2daa9e99, 0xca96f5a312eda247), "o" Dee_PCKuSIZ Dee_PCKuSIZ, cb, start, end));
	}
	class _Wrap_first
		: public deemon::detail::ConstGetRefProxy<_Wrap_first, Object> {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap_first(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "first", _Dee_HashSelectC(0xa9f0e818, 0x9d12a485470a29a7));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "first", _Dee_HashSelectC(0xa9f0e818, 0x9d12a485470a29a7)));
		}
	};
	WUNUSED _Wrap_first (first)() DEE_CXX_NOTHROW {
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
			return DeeObject_GetAttrStringHash(m_self, "last", _Dee_HashSelectC(0x185a4f9a, 0x760894ca6d41e4dc));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "last", _Dee_HashSelectC(0x185a4f9a, 0x760894ca6d41e4dc)));
		}
	};
	WUNUSED _Wrap_last (last)() DEE_CXX_NOTHROW {
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
			return DeeObject_GetAttrStringHash(m_self, "frozen", _Dee_HashSelectC(0x82311b77, 0x7b55e2e6e642b6fd));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "frozen", _Dee_HashSelectC(0x82311b77, 0x7b55e2e6e642b6fd)));
		}
	};
	WUNUSED _Wrap_frozen (frozen)() DEE_CXX_NOTHROW {
		return this;
	}
	class _Wrap_cached
		: public deemon::detail::ConstGetRefProxy<_Wrap_cached, Sequence<> > {
	private:
		DeeObject *m_self; /* [1..1] Linked object */
	public:
		_Wrap_cached(DeeObject *self) DEE_CXX_NOTHROW
			: m_self(self) {}
		WUNUSED DREF DeeObject *_getref() const DEE_CXX_NOTHROW {
			return DeeObject_GetAttrStringHash(m_self, "cached", _Dee_HashSelectC(0x915e175e, 0xddfd408a14eae4b4));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "cached", _Dee_HashSelectC(0x915e175e, 0xddfd408a14eae4b4)));
		}
	};
	WUNUSED _Wrap_cached (cached)() DEE_CXX_NOTHROW {
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
			return DeeObject_GetAttrStringHash(m_self, "__sizeof__", _Dee_HashSelectC(0x422f56f1, 0x4240f7a183278760));
		}
		WUNUSED bool bound() const {
			return throw_if_minusone(DeeObject_BoundAttrStringHash(m_self, "__sizeof__", _Dee_HashSelectC(0x422f56f1, 0x4240f7a183278760)));
		}
	};
	WUNUSED _Wrap___sizeof__ (__sizeof__)() DEE_CXX_NOTHROW {
		return this;
	}
/*[[[end]]]*/
};

DEE_CXX_END

#endif /* !GUARD_DEEMON_CXX_TUPLE_H */
