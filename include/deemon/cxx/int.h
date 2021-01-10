/* Copyright (c) 2018-2021 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2021 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_CXX_INT_H
#define GUARD_DEEMON_CXX_INT_H 1

#include "api.h"

#include "../int.h"
#include "object.h"

DEE_CXX_BEGIN

class int_: public Object {
public:
	static DeeTypeObject *classtype() DEE_CXX_NOTHROW {
		return &DeeInt_Type;
	}
	static bool check(DeeObject *__restrict ob) DEE_CXX_NOTHROW {
		return DeeInt_Check(ob);
	}
	static bool checkexact(DeeObject *__restrict ob) DEE_CXX_NOTHROW {
		return DeeInt_CheckExact(ob);
	}

public:
	DEE_CXX_DEFINE_OBJECT_CONSTRUCTORS(int_, Object)
	int_() DEE_CXX_NOTHROW: Object(nonnull((DeeObject *)&DeeInt_Zero)) {}
	int_(char value) DEE_CXX_NOTHROW: Object(inherit(DeeInt_NewChar(value))) {}
	int_(signed char value) DEE_CXX_NOTHROW: Object(inherit(DeeInt_NewSChar(value))) {}
	int_(unsigned char value) DEE_CXX_NOTHROW: Object(inherit(DeeInt_NewUChar(value))) {}
	int_(short value) DEE_CXX_NOTHROW: Object(inherit(DeeInt_NewShort(value))) {}
	int_(unsigned short value) DEE_CXX_NOTHROW: Object(inherit(DeeInt_NewUShort(value))) {}
	int_(int value) DEE_CXX_NOTHROW: Object(inherit(DeeInt_NewInt(value))) {}
	int_(unsigned int value) DEE_CXX_NOTHROW: Object(inherit(DeeInt_NewUInt(value))) {}
	int_(long value) DEE_CXX_NOTHROW: Object(inherit(DeeInt_NewLong(value))) {}
	int_(unsigned long value) DEE_CXX_NOTHROW: Object(inherit(DeeInt_NewULong(value))) {}
#ifdef __COMPILER_HAVE_LONGLONG
	int_(__LONGLONG value) DEE_CXX_NOTHROW: Object(inherit(DeeInt_NewLLong(value))) {}
	int_(__ULONGLONG value) DEE_CXX_NOTHROW: Object(inherit(DeeInt_NewULLong(value))) {}
#endif /* __COMPILER_HAVE_LONGLONG */
	int_(Dee_int128_t value) DEE_CXX_NOTHROW: Object(inherit(DeeInt_NewS128(value))) {}
	int_(Dee_uint128_t value) DEE_CXX_NOTHROW: Object(inherit(DeeInt_NewU128(value))) {}

#ifndef __OPTIMIZE_SIZE__
	/* Optimized conversion operators (based on `DeeInt_AsS32()' and friends) */
#ifdef __CHAR_UNSIGNED__
	int_ const &(getval)(char &value) const {
		throw_if_nonzero(DeeInt_Check(this->ptr())
		                 ? DeeInt_AsU(__SIZEOF_CHAR__, *this, &value)
		                 : DeeObject_AsChar(*this, &value));
		return *this;
	}
#else /* __CHAR_UNSIGNED__ */
	int_ const &(getval)(char &value) const {
		throw_if_nonzero(DeeInt_Check(this->ptr())
		                 ? DeeInt_AsS(__SIZEOF_CHAR__, *this, &value)
		                 : DeeObject_AsChar(*this, &value));
		return *this;
	}
#endif /* !__CHAR_UNSIGNED__ */
	int_ const &(getval)(signed char &value) const {
		throw_if_nonzero(DeeInt_Check(this->ptr())
		                 ? DeeInt_AsS(__SIZEOF_CHAR__, *this, &value)
		                 : DeeObject_AsSChar(*this, &value));
		return *this;
	}
	int_ const &(getval)(unsigned char &value) const {
		throw_if_nonzero(DeeInt_Check(this->ptr())
		                 ? DeeInt_AsU(__SIZEOF_CHAR__, *this, &value)
		                 : DeeObject_AsUChar(*this, &value));
		return *this;
	}
	int_ const &(getval)(short &value) const {
		throw_if_nonzero(DeeInt_Check(this->ptr())
		                 ? DeeInt_AsS(__SIZEOF_SHORT__, *this, &value)
		                 : DeeObject_AsShort(*this, &value));
		return *this;
	}
	int_ const &(getval)(unsigned short &value) const {
		throw_if_nonzero(DeeInt_Check(this->ptr())
		                 ? DeeInt_AsU(__SIZEOF_SHORT__, *this, &value)
		                 : DeeObject_AsUShort(*this, &value));
		return *this;
	}
	int_ const &(getval)(int &value) const {
		throw_if_nonzero(DeeInt_Check(this->ptr())
		                 ? DeeInt_AsS(__SIZEOF_INT__, *this, &value)
		                 : DeeObject_AsInt(*this, &value));
		return *this;
	}
	int_ const &(getval)(unsigned int &value) const {
		throw_if_nonzero(DeeInt_Check(this->ptr())
		                 ? DeeInt_AsU(__SIZEOF_INT__, *this, &value)
		                 : DeeObject_AsUInt(*this, &value));
		return *this;
	}
	int_ const &(getval)(long &value) const {
		throw_if_nonzero(DeeInt_Check(this->ptr())
		                 ? DeeInt_AsS(__SIZEOF_LONG__, *this, &value)
		                 : DeeObject_AsLong(*this, &value));
		return *this;
	}
	int_ const &(getval)(unsigned long &value) const {
		throw_if_nonzero(DeeInt_Check(this->ptr())
		                 ? DeeInt_AsU(__SIZEOF_LONG__, *this, &value)
		                 : DeeObject_AsULong(*this, &value));
		return *this;
	}
#ifdef __COMPILER_HAVE_LONGLONG
	int_ const &(getval)(__LONGLONG &value) const {
		throw_if_nonzero(DeeInt_Check(this->ptr())
		                 ? DeeInt_AsS(__SIZEOF_LONG_LONG__, *this, &value)
		                 : DeeObject_AsLLong(*this, &value));
		return *this;
	}
	int_ const &(getval)(__ULONGLONG &value) const {
		throw_if_nonzero(DeeInt_Check(this->ptr())
		                 ? DeeInt_AsU(__SIZEOF_LONG_LONG__, *this, &value)
		                 : DeeObject_AsULLong(*this, &value));
		return *this;
	}
#endif /* __COMPILER_HAVE_LONGLONG */
	int_ const &(getval)(Dee_int128_t &value) const {
		throw_if_nonzero(DeeInt_Check(this->ptr())
		                 ? DeeInt_AsS128(*this, &value)
		                 : DeeObject_AsInt128(*this, &value));
		return *this;
	}
	int_ const &(getval)(Dee_uint128_t &value) const {
		throw_if_nonzero(DeeInt_Check(this->ptr())
		                 ? DeeInt_AsU128(*this, &value)
		                 : DeeObject_AsUInt128(*this, &value));
		return *this;
	}

	WUNUSED short(asshort)() const {
		short result;
		getval(result);
		return result;
	}
	WUNUSED unsigned short(asushort)() const {
		unsigned short result;
		getval(result);
		return result;
	}
	WUNUSED int(asint)() const {
		int result;
		getval(result);
		return result;
	}
	WUNUSED unsigned int(asuint)() const {
		unsigned int result;
		getval(result);
		return result;
	}
	WUNUSED long(aslong)() const {
		long result;
		getval(result);
		return result;
	}
	WUNUSED unsigned long(asulong)() const {
		unsigned long result;
		getval(result);
		return result;
	}
#ifdef __COMPILER_HAVE_LONGLONG
	WUNUSED __LONGLONG(asllong)() const {
		__LONGLONG result;
		getval(result);
		return result;
	}
	WUNUSED __ULONGLONG(asullong)() const {
		__ULONGLONG result;
		getval(result);
		return result;
	}
#endif /* __COMPILER_HAVE_LONGLONG */

	WUNUSED int8_t(ass8)() const {
		int8_t result;
		getval(result);
		return result;
	}
	WUNUSED int16_t(ass16)() const {
		int16_t result;
		getval(result);
		return result;
	}
	WUNUSED int32_t(ass32)() const {
		int32_t result;
		getval(result);
		return result;
	}
	WUNUSED int64_t(ass64)() const {
		int64_t result;
		getval(result);
		return result;
	}
	WUNUSED Dee_int128_t(ass128)() const {
		Dee_int128_t result;
		getval(result);
		return result;
	}
	WUNUSED uint8_t(asu8)() const {
		uint8_t result;
		getval(result);
		return result;
	}
	WUNUSED uint16_t(asu16)() const {
		uint16_t result;
		getval(result);
		return result;
	}
	WUNUSED uint32_t(asu32)() const {
		uint32_t result;
		getval(result);
		return result;
	}
	WUNUSED uint64_t(asu64)() const {
		uint64_t result;
		getval(result);
		return result;
	}
	WUNUSED Dee_uint128_t(asu128)() const {
		Dee_uint128_t result;
		getval(result);
		return result;
	}
	WUNUSED size_t(assize)() const {
		size_t result;
		getval(result);
		return result;
	}
	WUNUSED Dee_ssize_t(asssize)() const {
		Dee_ssize_t result;
		getval(result);
		return result;
	}

	/* Integer conversion operators */
	explicit WUNUSED operator char() const {
		char result;
		getval(result);
		return result;
	}
	explicit WUNUSED operator signed char() const {
		signed char result;
		getval(result);
		return result;
	}
	explicit WUNUSED operator unsigned char() const {
		unsigned char result;
		getval(result);
		return result;
	}
	explicit WUNUSED operator short() const {
		short result;
		getval(result);
		return result;
	}
	explicit WUNUSED operator unsigned short() const {
		unsigned short result;
		getval(result);
		return result;
	}
	explicit WUNUSED operator int() const {
		int result;
		getval(result);
		return result;
	}
	explicit WUNUSED operator unsigned int() const {
		unsigned int result;
		getval(result);
		return result;
	}
	explicit WUNUSED operator long() const {
		long result;
		getval(result);
		return result;
	}
	explicit WUNUSED operator unsigned long() const {
		unsigned long result;
		getval(result);
		return result;
	}
#ifdef __COMPILER_HAVE_LONGLONG
	explicit WUNUSED operator __LONGLONG() const {
		__LONGLONG result;
		getval(result);
		return result;
	}
	explicit WUNUSED operator __ULONGLONG() const {
		__ULONGLONG result;
		getval(result);
		return result;
	}
#endif /* __COMPILER_HAVE_LONGLONG */
	explicit WUNUSED operator Dee_int128_t() const {
		Dee_int128_t result;
		getval(result);
		return result;
	}
	explicit WUNUSED operator Dee_uint128_t() const {
		Dee_uint128_t result;
		getval(result);
		return result;
	}
#endif /* !__OPTIMIZE_SIZE__ */
};
inline int_ Object::int_() const {
	return inherit(DeeObject_Int(*this));
}

#ifdef GUARD_DEEMON_CXX_SEQUENCE_H
inline deemon::int_ detail::sequence_base::erase(size_t index) const {
	return inherit(DeeObject_CallAttrStringf(*this, "erase", "Iu", index));
}
inline deemon::int_ detail::sequence_base::erase(size_t index, size_t count) const {
	return inherit(DeeObject_CallAttrStringf(*this, "erase", "IuIu", index, count));
}
inline deemon::int_ detail::sequence_base::erase(DeeObject *__restrict index) const {
	return inherit(DeeObject_CallAttrStringf(*this, "erase", "o", index));
}
inline deemon::int_ detail::sequence_base::erase(DeeObject *__restrict index, size_t count) const {
	return inherit(DeeObject_CallAttrStringf(*this, "erase", "oIu", index, count));
}
inline deemon::int_ detail::sequence_base::erase(DeeObject *__restrict index, DeeObject *__restrict count) const {
	return inherit(DeeObject_CallAttrStringf(*this, "erase", "oo", index, count));
}
inline Sequence<deemon::int_> detail::sequence_base::makerange(Dee_ssize_t end) {
	return inherit(DeeObject_CallAttrStringf((DeeObject *)&DeeSeq_Type, "range", "Id", end));
}
inline Sequence<deemon::int_> detail::sequence_base::makerange(Dee_ssize_t start, Dee_ssize_t end) {
	return inherit(DeeObject_CallAttrStringf((DeeObject *)&DeeSeq_Type, "range", "IdId", start, end));
}
inline Sequence<deemon::int_> detail::sequence_base::makerange(Dee_ssize_t start, Dee_ssize_t end, Dee_ssize_t step) {
	return inherit(DeeObject_CallAttrStringf((DeeObject *)&DeeSeq_Type, "range", "IdIdId", start, end, step));
}
inline Sequence<deemon::int_> detail::sequence_base::makerange(Dee_ssize_t start, Dee_ssize_t end, DeeObject *__restrict step) {
	return inherit(DeeObject_CallAttrStringf((DeeObject *)&DeeSeq_Type, "range", "IdIdo", start, end, step));
}
inline Sequence<deemon::int_> detail::sequence_base::makerange(Dee_ssize_t start, DeeObject *__restrict end) {
	return inherit(DeeObject_CallAttrStringf((DeeObject *)&DeeSeq_Type, "range", "Ido", start, end));
}
inline Sequence<deemon::int_> detail::sequence_base::makerange(Dee_ssize_t start, DeeObject *__restrict end, Dee_ssize_t step) {
	return inherit(DeeObject_CallAttrStringf((DeeObject *)&DeeSeq_Type, "range", "IdoId", start, end, step));
}
inline Sequence<deemon::int_> detail::sequence_base::makerange(Dee_ssize_t start, DeeObject *__restrict end, DeeObject *__restrict step) {
	return inherit(DeeObject_CallAttrStringf((DeeObject *)&DeeSeq_Type, "range", "Idoo", start, end, step));
}
inline Sequence<deemon::int_> detail::sequence_base::makerange(DeeObject *__restrict start, Dee_ssize_t end) {
	return inherit(DeeObject_CallAttrStringf((DeeObject *)&DeeSeq_Type, "range", "oId", start, end));
}
inline Sequence<deemon::int_> detail::sequence_base::makerange(DeeObject *__restrict start, Dee_ssize_t end, Dee_ssize_t step) {
	return inherit(DeeObject_CallAttrStringf((DeeObject *)&DeeSeq_Type, "range", "oIdId", start, end, step));
}
inline Sequence<deemon::int_> detail::sequence_base::makerange(DeeObject *__restrict start, Dee_ssize_t end, DeeObject *__restrict step) {
	return inherit(DeeObject_CallAttrStringf((DeeObject *)&DeeSeq_Type, "range", "oIdo", start, end, step));
}
inline Sequence<deemon::int_> detail::sequence_base::makerange(DeeObject *__restrict start, DeeObject *__restrict end, Dee_ssize_t step) {
	return inherit(DeeObject_CallAttrStringf((DeeObject *)&DeeSeq_Type, "range", "ooId", start, end, step));
}
#define DEFINE_ELEM_SEARCH_FUNCTION(Treturn, name)                                                             \
	inline Treturn(detail::sequence_base::name)(DeeObject *__restrict elem) const {                            \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "o", elem));                                    \
	}                                                                                                          \
	inline Treturn(detail::sequence_base::name)(DeeObject *__restrict elem, DeeObject *__restrict key) const { \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "oo", elem, key));                              \
	}
DEFINE_ELEM_SEARCH_FUNCTION(deemon::int_, count)
DEFINE_ELEM_SEARCH_FUNCTION(deemon::int_, locate)
DEFINE_ELEM_SEARCH_FUNCTION(deemon::int_, rlocate)
#undef DEFINE_ELEM_SEARCH_FUNCTION
#define DEFINE_ELEM_FIND_FUNCTION(Treturn, name)                                                                                         \
	inline Treturn(detail::sequence_base::name)(DeeObject *__restrict elem) const {                                                      \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "o", elem));                                                              \
	}                                                                                                                                    \
	inline Treturn(detail::sequence_base::name)(DeeObject *__restrict elem, DeeObject *__restrict key) const {                           \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "oo", elem, key));                                                        \
	}                                                                                                                                    \
	inline Treturn(detail::sequence_base::name)(DeeObject *__restrict elem, size_t start) const {                                        \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "oIu", elem, start));                                                     \
	}                                                                                                                                    \
	inline Treturn(detail::sequence_base::name)(DeeObject *__restrict elem, size_t start, DeeObject *__restrict key) const {             \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "oIuo", elem, start, key));                                               \
	}                                                                                                                                    \
	inline Treturn(detail::sequence_base::name)(DeeObject *__restrict elem, size_t start, size_t end) const {                            \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "oIuIu", elem, start, end));                                              \
	}                                                                                                                                    \
	inline Treturn(detail::sequence_base::name)(DeeObject *__restrict elem, size_t start, size_t end, DeeObject *__restrict key) const { \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "oIuIuo", elem, start, end, key));                                        \
	}
DEFINE_ELEM_FIND_FUNCTION(deemon::int_, find)
DEFINE_ELEM_FIND_FUNCTION(deemon::int_, rfind)
DEFINE_ELEM_FIND_FUNCTION(deemon::int_, index)
DEFINE_ELEM_FIND_FUNCTION(deemon::int_, rindex)
DEFINE_ELEM_FIND_FUNCTION(deemon::int_, removeall)
#undef DEFINE_ELEM_FIND_FUNCTION
#endif /* GUARD_DEEMON_CXX_SEQUENCE_H */


#ifdef GUARD_DEEMON_CXX_STRING_H
inline WUNUSED deemon::int_ string::asnumber() const {
	return inherit(DeeObject_CallAttrString(*this, "asnumber", 0, NULL));
}
inline WUNUSED deemon::int_ string::asnumber(size_t index) const {
	return inherit(DeeObject_CallAttrStringf(*this, "asnumber", "Iu", index));
}
inline WUNUSED deemon::int_ string::asnumber(size_t index, int defl) const {
	return inherit(DeeObject_CallAttrStringf(*this, "asnumber", "Iud", index, defl));
}
inline WUNUSED deemon::int_ string::asnumber(DeeObject *__restrict index) const {
	return inherit(DeeObject_CallAttrStringf(*this, "asnumber", "o", index));
}
inline WUNUSED deemon::int_ string::asnumber(DeeObject *__restrict index, int defl) const {
	return inherit(DeeObject_CallAttrStringf(*this, "asnumber", "od", index, defl));
}
inline WUNUSED deemon::int_ string::asdigit() const {
	return inherit(DeeObject_CallAttrString(*this, "asdigit", 0, NULL));
}
inline WUNUSED deemon::int_ string::asdigit(size_t index) const {
	return inherit(DeeObject_CallAttrStringf(*this, "asdigit", "Iu", index));
}
inline WUNUSED deemon::int_ string::asdigit(size_t index, int defl) const {
	return inherit(DeeObject_CallAttrStringf(*this, "asdigit", "Iud", index, defl));
}
inline WUNUSED deemon::int_ string::asdigit(DeeObject *__restrict index) const {
	return inherit(DeeObject_CallAttrStringf(*this, "asdigit", "o", index));
}
inline WUNUSED deemon::int_ string::asdigit(DeeObject *__restrict index, int defl) const {
	return inherit(DeeObject_CallAttrStringf(*this, "asdigit", "od", index, defl));
}
inline WUNUSED deemon::int_ string::asdecimal() const {
	return inherit(DeeObject_CallAttrString(*this, "asdecimal", 0, NULL));
}
inline WUNUSED deemon::int_ string::asdecimal(size_t index) const {
	return inherit(DeeObject_CallAttrStringf(*this, "asdecimal", "Iu", index));
}
inline WUNUSED deemon::int_ string::asdecimal(size_t index, int defl) const {
	return inherit(DeeObject_CallAttrStringf(*this, "asdecimal", "Iud", index, defl));
}
inline WUNUSED deemon::int_ string::asdecimal(DeeObject *__restrict index) const {
	return inherit(DeeObject_CallAttrStringf(*this, "asdecimal", "o", index));
}
inline WUNUSED deemon::int_ string::asdecimal(DeeObject *__restrict index, int defl) const {
	return inherit(DeeObject_CallAttrStringf(*this, "asdecimal", "od", index, defl));
}
#define DEFINE_FIND_FUNCTION(Treturn, name, needle)                                                                 \
	inline WUNUSED Treturn(string::name)(/*utf-8*/ char const *__restrict needle) const {                           \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "s", needle));                                       \
	}                                                                                                               \
	inline WUNUSED Treturn(string::name)(/*utf-8*/ char const *__restrict needle, size_t start) const {             \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "sIu", needle, start));                              \
	}                                                                                                               \
	inline WUNUSED Treturn(string::name)(/*utf-8*/ char const *__restrict needle, size_t start, size_t end) const { \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "sIuIu", needle, start, end));                       \
	}                                                                                                               \
	inline WUNUSED Treturn(string::name)(DeeObject *__restrict needle) const {                                      \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "o", needle));                                       \
	}                                                                                                               \
	inline WUNUSED Treturn(string::name)(DeeObject *__restrict needle, size_t start) const {                        \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "oIu", needle, start));                              \
	}                                                                                                               \
	inline WUNUSED Treturn(string::name)(DeeObject *__restrict needle, size_t start, size_t end) const {            \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "oIuIu", needle, start, end));                       \
	}
DEFINE_FIND_FUNCTION(deemon::int_, find, needle)
DEFINE_FIND_FUNCTION(deemon::int_, rfind, needle)
DEFINE_FIND_FUNCTION(Sequence<deemon::int_>, findall, needle)
DEFINE_FIND_FUNCTION(deemon::int_, index, needle)
DEFINE_FIND_FUNCTION(deemon::int_, rindex, needle)
DEFINE_FIND_FUNCTION(deemon::int_, count, needle)
DEFINE_FIND_FUNCTION(deemon::int_, casefind, needle)
DEFINE_FIND_FUNCTION(deemon::int_, caserfind, needle)
DEFINE_FIND_FUNCTION(Sequence<deemon::int_>, casefindall, needle)
DEFINE_FIND_FUNCTION(deemon::int_, caseindex, needle)
DEFINE_FIND_FUNCTION(deemon::int_, caserindex, needle)
DEFINE_FIND_FUNCTION(deemon::int_, casecount, needle)
#undef DEFINE_FIND_FUNCTION
#define DEFINE_COMPARE_FUNCTION(Treturn, name)                                                                                                      \
	inline WUNUSED Treturn(string::name)(/*utf-8*/ char const *__restrict other) const {                                                            \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "s", other));                                                                        \
	}                                                                                                                                               \
	inline WUNUSED Treturn(string::name)(/*utf-8*/ char const *__restrict other, size_t other_size) const {                                         \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "$s", other_size, other));                                                           \
	}                                                                                                                                               \
	inline WUNUSED Treturn(string::name)(size_t my_start, /*utf-8*/ char const *__restrict other) const {                                           \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "Ius", my_start, other));                                                            \
	}                                                                                                                                               \
	inline WUNUSED Treturn(string::name)(size_t my_start, /*utf-8*/ char const *__restrict other, size_t other_size) const {                        \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "Iu$s", my_start, other_size, other));                                               \
	}                                                                                                                                               \
	inline WUNUSED Treturn(string::name)(size_t my_start, size_t my_end, /*utf-8*/ char const *__restrict other) const {                            \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "IuIus", my_start, my_end, other));                                                  \
	}                                                                                                                                               \
	inline WUNUSED Treturn(string::name)(size_t my_start, size_t my_end, /*utf-8*/ char const *__restrict other, size_t other_size) const {         \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "IuIu$s", my_start, my_end, other_size, other));                                     \
	}                                                                                                                                               \
	inline WUNUSED Treturn(string::name)(DeeObject *__restrict other) const {                                                                       \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "o", other));                                                                        \
	}                                                                                                                                               \
	inline WUNUSED Treturn(string::name)(DeeObject *__restrict other, size_t other_start) const {                                                   \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "oIu", other, other_start));                                                         \
	}                                                                                                                                               \
	inline WUNUSED Treturn(string::name)(DeeObject *__restrict other, size_t other_start, size_t other_end) const {                                 \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "oIuIu", other, other_start, other_end));                                            \
	}                                                                                                                                               \
	inline WUNUSED Treturn(string::name)(size_t my_start, DeeObject *__restrict other) const {                                                      \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "Iuo", my_start, other));                                                            \
	}                                                                                                                                               \
	inline WUNUSED Treturn(string::name)(size_t my_start, DeeObject *__restrict other, size_t other_start) const {                                  \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "IuoIu", my_start, other, other_start));                                             \
	}                                                                                                                                               \
	inline WUNUSED Treturn(string::name)(size_t my_start, DeeObject *__restrict other, size_t other_start, size_t other_end) const {                \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "IuoIuIu", my_start, other, other_start, other_end));                                \
	}                                                                                                                                               \
	inline WUNUSED Treturn(string::name)(size_t my_start, size_t my_end, DeeObject *__restrict other) const {                                       \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "IuIuo", my_start, my_end, other));                                                  \
	}                                                                                                                                               \
	inline WUNUSED Treturn(string::name)(size_t my_start, size_t my_end, DeeObject *__restrict other, size_t other_start) const {                   \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "IuIuoIu", my_start, my_end, other, other_start));                                   \
	}                                                                                                                                               \
	inline WUNUSED Treturn(string::name)(size_t my_start, size_t my_end, DeeObject *__restrict other, size_t other_start, size_t other_end) const { \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "IuIuoIuIu", my_start, my_end, other, other_start, other_end));                      \
	}
DEFINE_COMPARE_FUNCTION(deemon::int_, compare)
DEFINE_COMPARE_FUNCTION(deemon::int_, vercompare)
DEFINE_COMPARE_FUNCTION(deemon::int_, wildcompare)
DEFINE_COMPARE_FUNCTION(deemon::int_, fuzzycompare)
DEFINE_COMPARE_FUNCTION(deemon::int_, casecompare)
DEFINE_COMPARE_FUNCTION(deemon::int_, casevercompare)
DEFINE_COMPARE_FUNCTION(deemon::int_, casewildcompare)
DEFINE_COMPARE_FUNCTION(deemon::int_, casefuzzycompare)
DEFINE_COMPARE_FUNCTION(deemon::int_, common)
DEFINE_COMPARE_FUNCTION(deemon::int_, rcommon)
DEFINE_COMPARE_FUNCTION(deemon::int_, casecommon)
DEFINE_COMPARE_FUNCTION(deemon::int_, casercommon)
#undef DEFINE_COMPARE_FUNCTION
#define DEFINE_FINDMATCH_FUNCTION(Treturn, name)                                                                                                          \
	inline WUNUSED Treturn(string::name)(DeeObject *__restrict open, DeeObject *__restrict close) const {                                                 \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "oo", open, close));                                                                       \
	}                                                                                                                                                     \
	inline WUNUSED Treturn(string::name)(DeeObject *__restrict open, DeeObject *__restrict close, size_t start) const {                                   \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "ooIu", open, close, start));                                                              \
	}                                                                                                                                                     \
	inline WUNUSED Treturn(string::name)(DeeObject *__restrict open, DeeObject *__restrict close, size_t start, size_t end) const {                       \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "ooIuIu", open, close, start, end));                                                       \
	}                                                                                                                                                     \
	inline WUNUSED Treturn(string::name)(DeeObject *__restrict open, /*utf-8*/ char const *__restrict close) const {                                      \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "os", open, close));                                                                       \
	}                                                                                                                                                     \
	inline WUNUSED Treturn(string::name)(DeeObject *__restrict open, /*utf-8*/ char const *__restrict close, size_t start) const {                        \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "osIu", open, close, start));                                                              \
	}                                                                                                                                                     \
	inline WUNUSED Treturn(string::name)(DeeObject *__restrict open, /*utf-8*/ char const *__restrict close, size_t start, size_t end) const {            \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "osIuIu", open, close, start, end));                                                       \
	}                                                                                                                                                     \
	inline WUNUSED Treturn(string::name)(/*utf-8*/ char const *__restrict open, DeeObject *__restrict close) const {                                      \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "so", open, close));                                                                       \
	}                                                                                                                                                     \
	inline WUNUSED Treturn(string::name)(/*utf-8*/ char const *__restrict open, DeeObject *__restrict close, size_t start) const {                        \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "soIu", open, close, start));                                                              \
	}                                                                                                                                                     \
	inline WUNUSED Treturn(string::name)(/*utf-8*/ char const *__restrict open, DeeObject *__restrict close, size_t start, size_t end) const {            \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "soIuIu", open, close, start, end));                                                       \
	}                                                                                                                                                     \
	inline WUNUSED Treturn(string::name)(/*utf-8*/ char const *__restrict open, /*utf-8*/ char const *__restrict close) const {                           \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "ss", open, close));                                                                       \
	}                                                                                                                                                     \
	inline WUNUSED Treturn(string::name)(/*utf-8*/ char const *__restrict open, /*utf-8*/ char const *__restrict close, size_t start) const {             \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "ssIu", open, close, start));                                                              \
	}                                                                                                                                                     \
	inline WUNUSED Treturn(string::name)(/*utf-8*/ char const *__restrict open, /*utf-8*/ char const *__restrict close, size_t start, size_t end) const { \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "ssIuIu", open, close, start, end));                                                       \
	}
DEFINE_FINDMATCH_FUNCTION(deemon::int_, findmatch)
DEFINE_FINDMATCH_FUNCTION(deemon::int_, indexmatch)
DEFINE_FINDMATCH_FUNCTION(deemon::int_, rfindmatch)
DEFINE_FINDMATCH_FUNCTION(deemon::int_, rindexmatch)
DEFINE_FINDMATCH_FUNCTION(Sequence<deemon::int_>, casefindmatch)
DEFINE_FINDMATCH_FUNCTION(Sequence<deemon::int_>, caseindexmatch)
DEFINE_FINDMATCH_FUNCTION(Sequence<deemon::int_>, caserfindmatch)
DEFINE_FINDMATCH_FUNCTION(Sequence<deemon::int_>, caserindexmatch)
#undef DEFINE_FINDMATCH_FUNCTION
#define DEFINE_RE_FUNCTION(Treturn, name)                                                                                                                    \
	inline WUNUSED Treturn(string::name)(DeeObject *__restrict pattern) const {                                                                              \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "o", pattern));                                                                               \
	}                                                                                                                                                        \
	inline WUNUSED Treturn(string::name)(DeeObject *__restrict pattern, DeeObject *__restrict rules) const {                                                 \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "oo", pattern, rules));                                                                       \
	}                                                                                                                                                        \
	inline WUNUSED Treturn(string::name)(DeeObject *__restrict pattern, /*utf-8*/ char const *__restrict rules) const {                                      \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "os", pattern, rules));                                                                       \
	}                                                                                                                                                        \
	inline WUNUSED Treturn(string::name)(DeeObject *__restrict pattern, size_t start) const {                                                                \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "oIu", pattern, start));                                                                      \
	}                                                                                                                                                        \
	inline WUNUSED Treturn(string::name)(DeeObject *__restrict pattern, DeeObject *__restrict rules, size_t start) const {                                   \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "ooIu", pattern, rules, start));                                                              \
	}                                                                                                                                                        \
	inline WUNUSED Treturn(string::name)(DeeObject *__restrict pattern, /*utf-8*/ char const *__restrict rules, size_t start) const {                        \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "osIu", pattern, rules, start));                                                              \
	}                                                                                                                                                        \
	inline WUNUSED Treturn(string::name)(DeeObject *__restrict pattern, size_t start, size_t end) const {                                                    \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "oIuIu", pattern, start, end));                                                               \
	}                                                                                                                                                        \
	inline WUNUSED Treturn(string::name)(DeeObject *__restrict pattern, DeeObject *__restrict rules, size_t start, size_t end) const {                       \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "ooIuIu", pattern, rules, start, end));                                                       \
	}                                                                                                                                                        \
	inline WUNUSED Treturn(string::name)(DeeObject *__restrict pattern, /*utf-8*/ char const *__restrict rules, size_t start, size_t end) const {            \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "osIuIu", pattern, rules, start, end));                                                       \
	}                                                                                                                                                        \
	inline WUNUSED Treturn(string::name)(DeeObject *__restrict pattern, size_t start, DeeObject *__restrict rules) const {                                   \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "oIuo", pattern, start, rules));                                                              \
	}                                                                                                                                                        \
	inline WUNUSED Treturn(string::name)(DeeObject *__restrict pattern, size_t start, size_t end, DeeObject *__restrict rules) const {                       \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "oIuIuo", pattern, start, end, rules));                                                       \
	}                                                                                                                                                        \
	inline WUNUSED Treturn(string::name)(DeeObject *__restrict pattern, size_t start, /*utf-8*/ char const *__restrict rules) const {                        \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "oIus", pattern, start, rules));                                                              \
	}                                                                                                                                                        \
	inline WUNUSED Treturn(string::name)(DeeObject *__restrict pattern, size_t start, size_t end, /*utf-8*/ char const *__restrict rules) const {            \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "oIuIus", pattern, start, end, rules));                                                       \
	}                                                                                                                                                        \
	inline WUNUSED Treturn(string::name)(/*utf-8*/ char const *__restrict pattern) const {                                                                   \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "s", pattern));                                                                               \
	}                                                                                                                                                        \
	inline WUNUSED Treturn(string::name)(/*utf-8*/ char const *__restrict pattern, DeeObject *__restrict rules) const {                                      \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "so", pattern, rules));                                                                       \
	}                                                                                                                                                        \
	inline WUNUSED Treturn(string::name)(/*utf-8*/ char const *__restrict pattern, /*utf-8*/ char const *__restrict rules) const {                           \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "ss", pattern, rules));                                                                       \
	}                                                                                                                                                        \
	inline WUNUSED Treturn(string::name)(/*utf-8*/ char const *__restrict pattern, size_t start) const {                                                     \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "sIu", pattern, start));                                                                      \
	}                                                                                                                                                        \
	inline WUNUSED Treturn(string::name)(/*utf-8*/ char const *__restrict pattern, DeeObject *__restrict rules, size_t start) const {                        \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "soIu", pattern, rules, start));                                                              \
	}                                                                                                                                                        \
	inline WUNUSED Treturn(string::name)(/*utf-8*/ char const *__restrict pattern, /*utf-8*/ char const *__restrict rules, size_t start) const {             \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "ssIu", pattern, rules, start));                                                              \
	}                                                                                                                                                        \
	inline WUNUSED Treturn(string::name)(/*utf-8*/ char const *__restrict pattern, size_t start, size_t end) const {                                         \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "sIuIu", pattern, start, end));                                                               \
	}                                                                                                                                                        \
	inline WUNUSED Treturn(string::name)(/*utf-8*/ char const *__restrict pattern, DeeObject *__restrict rules, size_t start, size_t end) const {            \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "soIuIu", pattern, rules, start, end));                                                       \
	}                                                                                                                                                        \
	inline WUNUSED Treturn(string::name)(/*utf-8*/ char const *__restrict pattern, /*utf-8*/ char const *__restrict rules, size_t start, size_t end) const { \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "ssIuIu", pattern, rules, start, end));                                                       \
	}                                                                                                                                                        \
	inline WUNUSED Treturn(string::name)(/*utf-8*/ char const *__restrict pattern, size_t start, DeeObject *__restrict rules) const {                        \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "sIuo", pattern, start, rules));                                                              \
	}                                                                                                                                                        \
	inline WUNUSED Treturn(string::name)(/*utf-8*/ char const *__restrict pattern, size_t start, size_t end, DeeObject *__restrict rules) const {            \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "sIuIuo", pattern, start, end, rules));                                                       \
	}                                                                                                                                                        \
	inline WUNUSED Treturn(string::name)(/*utf-8*/ char const *__restrict pattern, size_t start, /*utf-8*/ char const *__restrict rules) const {             \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "sIus", pattern, start, rules));                                                              \
	}                                                                                                                                                        \
	inline WUNUSED Treturn(string::name)(/*utf-8*/ char const *__restrict pattern, size_t start, size_t end, /*utf-8*/ char const *__restrict rules) const { \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "sIuIus", pattern, start, end, rules));                                                       \
	}
DEFINE_RE_FUNCTION(deemon::int_, rematch)
DEFINE_RE_FUNCTION(Sequence<deemon::int_>, refind)
DEFINE_RE_FUNCTION(Sequence<deemon::int_>, rerfind)
DEFINE_RE_FUNCTION(Sequence<deemon::int_>, reindex)
DEFINE_RE_FUNCTION(Sequence<deemon::int_>, rerindex)
DEFINE_RE_FUNCTION(Sequence<Sequence<deemon::int_> >, refindall)
DEFINE_RE_FUNCTION(deemon::int_, recount)
#undef DEFINE_RE_FUNCTION
#endif /* GUARD_DEEMON_CXX_STRING_H */


DEE_CXX_END

#endif /* !GUARD_DEEMON_CXX_INT_H */
