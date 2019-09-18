/* Copyright (c) 2019 Griefer@Work                                            *
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
 *    in a product, an acknowledgement in the product documentation would be  *
 *    appreciated but is not required.                                        *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_CXX_STRING_H
#define GUARD_DEEMON_CXX_STRING_H 1

#include "api.h"

#include <string.h>

#include "../int.h"
#include "../string.h"
#include "../stringutils.h"
#include "object.h"
#include "sequence.h"
#include "tuple.h"
#ifdef CONFIG_DEEMON_HAVE_NATIVE_WCHAR_T
#include <wchar.h>
#endif /* CONFIG_DEEMON_HAVE_NATIVE_WCHAR_T */

DEE_CXX_BEGIN


class string: public Sequence<string> {
public:
	static DeeTypeObject *classtype() DEE_CXX_NOTHROW {
		return &DeeString_Type;
	}
	static bool check(DeeObject *__restrict ob) DEE_CXX_NOTHROW {
		return DeeString_Check(ob);
	}
	static bool checkexact(DeeObject *__restrict ob) DEE_CXX_NOTHROW {
		return DeeString_CheckExact(ob);
	}

public: /* string from deemon */
	DEE_CXX_DEFINE_OBJECT_CONSTRUCTORS(string, Sequence<string>)
	string() DEE_CXX_NOTHROW: Sequence<string>(nonnull(Dee_EmptyString)) {}
	string(/*utf-8*/ char const *__restrict utf8_str)
	    : Sequence<string>(inherit(DeeString_NewUtf8(utf8_str, strlen(utf8_str), STRING_ERROR_FSTRICT))) {}
	string(/*utf-8*/ char const *__restrict utf8_str, size_t utf8_len, unsigned int error_mode = STRING_ERROR_FSTRICT)
	    : Sequence<string>(inherit(DeeString_NewUtf8(utf8_str, utf8_len, error_mode))) {}
	string(/*utf-16*/ uint16_t const *__restrict utf16_str, size_t utf16_len, unsigned int error_mode = STRING_ERROR_FSTRICT)
	    : Sequence<string>(inherit(DeeString_NewUtf16(utf16_str, utf16_len, error_mode))) {}
	string(/*utf-32*/ uint32_t const *__restrict utf32_str, size_t utf32_len, unsigned int error_mode = STRING_ERROR_FSTRICT)
	    : Sequence<string>(inherit(DeeString_NewUtf32(utf32_str, utf32_len, error_mode))) {}
#ifdef CONFIG_DEEMON_HAVE_NATIVE_WCHAR_T
	string(/*wide*/ dwchar_t const *__restrict wide_str)
	    : Sequence<string>(inherit(DeeString_NewWide(wide_str, wcslen(wide_str), STRING_ERROR_FSTRICT))) {}
	string(/*wide*/ dwchar_t const *__restrict wide_str, size_t wide_len, unsigned int error_mode = STRING_ERROR_FSTRICT)
	    : Sequence<string>(inherit(DeeString_NewWide(wide_str, wide_len, error_mode))) {}
#endif /* CONFIG_DEEMON_HAVE_NATIVE_WCHAR_T */
	static WUNUSED string vcformat(/*utf-8*/ char const *__restrict str, va_list args) {
		return inherit(DeeString_VNewf(str, args));
	}
	static WUNUSED string cformat(/*utf-8*/ char const *__restrict str, ...) {
		va_list args;
		DREF DeeObject *result;
		va_start(args, str);
		result = DeeString_VNewf(str, args);
		va_end(args);
		return inherit(result);
	}
	static WUNUSED string from1byte(/*1-byte*/ uint8_t const *__restrict str, size_t len) {
		return inherit(DeeString_New1Byte(str, len));
	}
	static WUNUSED string from2byte(/*2-byte*/ uint16_t const *__restrict str, size_t len) {
		return inherit(DeeString_New2Byte(str, len));
	}
	static WUNUSED string from4byte(/*4-byte*/ uint32_t const *__restrict str, size_t len) {
		return inherit(DeeString_New4Byte(str, len));
	}
	static WUNUSED string chr(uint8_t ch) {
		return inherit(DeeString_Chr(ch));
	}
	static WUNUSED string chr(uint16_t ch) {
		return inherit(DeeString_Chr(ch));
	}
	static WUNUSED string chr(uint32_t ch) {
		return inherit(DeeString_Chr(ch));
	}
	WUNUSED bool is1byte() const DEE_CXX_NOTHROW {
		return DeeString_Is1Byte(this->ptr());
	}
	WUNUSED bool is2byte() const DEE_CXX_NOTHROW {
		return DeeString_Is2Byte(this->ptr());
	}
	WUNUSED bool is4byte() const DEE_CXX_NOTHROW {
		return DeeString_Is4Byte(this->ptr());
	}
	operator obj_string() const DEE_CXX_NOTHROW {
		return obj_string(this->ptr());
	}
	WUNUSED bool bool_() const DEE_CXX_NOTHROW {
		return !DeeString_IsEmpty(this->ptr());
	}
	WUNUSED operator bool() const DEE_CXX_NOTHROW {
		return !DeeString_IsEmpty(this->ptr());
	}
	WUNUSED bool operator!() const DEE_CXX_NOTHROW {
		return DeeString_IsEmpty(this->ptr());
	}
	WUNUSED size_t size() const DEE_CXX_NOTHROW {
		return DeeString_WLEN(this->ptr());
	}
	WUNUSED uint32_t getchar(size_t index) const DEE_CXX_NOTHROW {
		return DeeString_GetChar(this->ptr(), index);
	}
#ifdef CONFIG_BUILDING_DEEMON
	WUNUSED Dee_hash_t hash() const DEE_CXX_NOTHROW {
		return DeeString_Hash(*this);
	}
#endif /* CONFIG_BUILDING_DEEMON */
	WUNUSED Dee_hash_t hashcase() const DEE_CXX_NOTHROW {
		return DeeString_HashCase(*this);
	}
	WUNUSED ATTR_RETNONNULL uint8_t *bytes(bool allow_invalid = false) const {
		return (uint8_t *)throw_if_null(DeeString_AsBytes(*this, allow_invalid));
	}
	WUNUSED ATTR_RETNONNULL char *asutf8() const {
		return (char *)throw_if_null((void *)DeeString_AsUtf8(*this));
	}
	WUNUSED ATTR_RETNONNULL uint16_t *asutf16(unsigned int error_mode = STRING_ERROR_FSTRICT) const {
		return (uint16_t *)throw_if_null((void *)DeeString_AsUtf16(*this, error_mode));
	}
	WUNUSED ATTR_RETNONNULL uint32_t *asutf32() const {
		return (uint32_t *)throw_if_null((void *)DeeString_AsUtf32(this->ptr()));
	}
	WUNUSED ATTR_RETNONNULL dwchar_t *aswide() const {
		return (dwchar_t *)throw_if_null((void *)DeeString_AsWide(this->ptr()));
	}
	WUNUSED ATTR_RETNONNULL uint8_t *as1byte() const DEE_CXX_NOTHROW {
		return DeeString_As1Byte(this->ptr());
	}
	WUNUSED ATTR_RETNONNULL uint16_t *as2byte() const {
		return (uint16_t *)throw_if_null((void *)DeeString_As2Byte(this->ptr()));
	}
	WUNUSED ATTR_RETNONNULL uint32_t *as4byte() const {
		return (uint32_t *)throw_if_null((void *)DeeString_As4Byte(this->ptr()));
	}
	WUNUSED ATTR_RETNONNULL uint8_t *get1byte() const DEE_CXX_NOTHROW {
		return DeeString_Get1Byte(this->ptr());
	}
	WUNUSED ATTR_RETNONNULL uint16_t *get2byte() const DEE_CXX_NOTHROW {
		return DeeString_Get2Byte(this->ptr());
	}
	WUNUSED ATTR_RETNONNULL uint32_t *get4byte() const DEE_CXX_NOTHROW {
		return DeeString_Get4Byte(this->ptr());
	}
	explicit ATTR_RETNONNULL operator char *() const {
		return asutf8();
	}
#ifdef CONFIG_DEEMON_HAVE_NATIVE_WCHAR_T
	explicit ATTR_RETNONNULL operator dwchar_t *() const {
		return aswide();
	}
#endif /* CONFIG_DEEMON_HAVE_NATIVE_WCHAR_T */
	Object decode(/*utf-8*/ char const *__restrict codec_name) const {
		return inherit(DeeObject_CallAttrStringf(*this, "decode", "s", codec_name));
	}
	Object decode(/*utf-8*/ char const *__restrict codec_name, /*utf-8*/ char const *__restrict errors) const {
		return inherit(DeeObject_CallAttrStringf(*this, "decode", "ss", codec_name, errors));
	}
	Object encode(/*utf-8*/ char const *__restrict codec_name) const {
		return inherit(DeeObject_CallAttrStringf(*this, "encode", "s", codec_name));
	}
	Object encode(/*utf-8*/ char const *__restrict codec_name, /*utf-8*/ char const *__restrict errors) const {
		return inherit(DeeObject_CallAttrStringf(*this, "encode", "ss", codec_name, errors));
	}
	WUNUSED string format(obj_sequence args) const {
		return inherit(DeeObject_CallAttrStringf(*this, "format", "o", (DeeObject *)args));
	}
	WUNUSED string format(std::initializer_list<DeeObject *> const &args) const {
		Tuple<Object> ob_args(args);
		return inherit(DeeObject_CallAttrStringf(*this, "format", "o", (DeeObject *)ob_args));
	}
	WUNUSED Sequence scanf(/*utf-8*/ char const *__restrict format) const {
		return inherit(DeeObject_CallAttrStringf(*this, "scanf", "s", format));
	}
	WUNUSED Sequence scanf(obj_string format) const {
		return inherit(DeeObject_CallAttrStringf(*this, "scanf", "o", (DeeObject *)format));
	}
#define DEFINE_CHARACTER_TRAIT(name)                                                                 \
	WUNUSED bool(name)() const {                                                                     \
		return Object(inherit(DeeObject_CallAttrString(*this, #name, 0, NULL))).bool_();             \
	}                                                                                                \
	WUNUSED bool(name)(size_t index) const {                                                         \
		return Object(inherit(DeeObject_CallAttrStringf(*this, #name, "Iu", index))).bool_();        \
	}                                                                                                \
	WUNUSED bool(name)(size_t start, size_t end) const {                                             \
		return Object(inherit(DeeObject_CallAttrStringf(*this, #name, "IuIu", start, end))).bool_(); \
	}
	DEFINE_CHARACTER_TRAIT(isprint)
	DEFINE_CHARACTER_TRAIT(isalpha)
	DEFINE_CHARACTER_TRAIT(isspace)
	DEFINE_CHARACTER_TRAIT(islf)
	DEFINE_CHARACTER_TRAIT(islower)
	DEFINE_CHARACTER_TRAIT(isupper)
	DEFINE_CHARACTER_TRAIT(iscntrl)
	DEFINE_CHARACTER_TRAIT(isdigit)
	DEFINE_CHARACTER_TRAIT(isdecimal)
	DEFINE_CHARACTER_TRAIT(issymstrt)
	DEFINE_CHARACTER_TRAIT(issymcont)
	DEFINE_CHARACTER_TRAIT(isalnum)
	DEFINE_CHARACTER_TRAIT(isnumeric)
	DEFINE_CHARACTER_TRAIT(istitle)
	DEFINE_CHARACTER_TRAIT(issymbol)
	DEFINE_CHARACTER_TRAIT(isascii)
#undef DEFINE_CHARACTER_TRAIT
#define DEFINE_CHARACTER_TRAIT(name)                                                                 \
	WUNUSED bool(name)() const {                                                                     \
		return Object(inherit(DeeObject_CallAttrString(*this, #name, 0, NULL))).bool_();             \
	}                                                                                                \
	WUNUSED bool(name)(size_t start) const {                                                         \
		return Object(inherit(DeeObject_CallAttrStringf(*this, #name, "Iu", start))).bool_();        \
	}                                                                                                \
	WUNUSED bool(name)(size_t start, size_t end) const {                                             \
		return Object(inherit(DeeObject_CallAttrStringf(*this, #name, "IuIu", start, end))).bool_(); \
	}
	DEFINE_CHARACTER_TRAIT(isanyprint)
	DEFINE_CHARACTER_TRAIT(isanyalpha)
	DEFINE_CHARACTER_TRAIT(isanyspace)
	DEFINE_CHARACTER_TRAIT(isanylf)
	DEFINE_CHARACTER_TRAIT(isanylower)
	DEFINE_CHARACTER_TRAIT(isanyupper)
	DEFINE_CHARACTER_TRAIT(isanycntrl)
	DEFINE_CHARACTER_TRAIT(isanydigit)
	DEFINE_CHARACTER_TRAIT(isanydecimal)
	DEFINE_CHARACTER_TRAIT(isanysymstrt)
	DEFINE_CHARACTER_TRAIT(isanysymcont)
	DEFINE_CHARACTER_TRAIT(isanyalnum)
	DEFINE_CHARACTER_TRAIT(isanynumeric)
	DEFINE_CHARACTER_TRAIT(isanytitle)
	DEFINE_CHARACTER_TRAIT(isanyascii)
#undef DEFINE_CHARACTER_TRAIT
#define DEFINE_STRING_TRANSFORMATION(name)                                           \
	WUNUSED string(name)() const {                                                   \
		return inherit(DeeObject_CallAttrString(*this, #name, 0, NULL));             \
	}                                                                                \
	WUNUSED string(name)(size_t start) const {                                       \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "Iu", start));        \
	}                                                                                \
	WUNUSED string(name)(size_t start, size_t end) const {                           \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "IuIu", start, end)); \
	}
	DEFINE_STRING_TRANSFORMATION(lower)
	DEFINE_STRING_TRANSFORMATION(upper)
	DEFINE_STRING_TRANSFORMATION(title)
	DEFINE_STRING_TRANSFORMATION(capitalize)
	DEFINE_STRING_TRANSFORMATION(swapcase)
	DEFINE_STRING_TRANSFORMATION(casefold)
	DEFINE_STRING_TRANSFORMATION(reversed)
#undef DEFINE_STRING_TRANSFORMATION
	WUNUSED deemon::int_ asnumber() const;
	WUNUSED deemon::int_ asnumber(size_t index) const;
	WUNUSED deemon::int_ asnumber(size_t index, int defl) const;
	WUNUSED deemon::int_ asnumber(DeeObject *__restrict index) const;
	WUNUSED deemon::int_ asnumber(DeeObject *__restrict index, int defl) const;
	WUNUSED deemon::int_ asdigit() const;
	WUNUSED deemon::int_ asdigit(size_t index) const;
	WUNUSED deemon::int_ asdigit(size_t index, int defl) const;
	WUNUSED deemon::int_ asdigit(DeeObject *__restrict index) const;
	WUNUSED deemon::int_ asdigit(DeeObject *__restrict index, int defl) const;
	WUNUSED deemon::int_ asdecimal() const;
	WUNUSED deemon::int_ asdecimal(size_t index) const;
	WUNUSED deemon::int_ asdecimal(size_t index, int defl) const;
	WUNUSED deemon::int_ asdecimal(DeeObject *__restrict index) const;
	WUNUSED deemon::int_ asdecimal(DeeObject *__restrict index, int defl) const;
	WUNUSED Object asnumber(size_t index, DeeObject *__restrict defl) const {
		return inherit(DeeObject_CallAttrStringf(*this, "asnumber", "Iuo", index, defl));
	}
	WUNUSED Object asnumber(DeeObject *__restrict index, DeeObject *__restrict defl) const {
		return inherit(DeeObject_CallAttrStringf(*this, "asnumber", "oo", index, defl));
	}
	WUNUSED Object asdigit(size_t index, DeeObject *__restrict defl) const {
		return inherit(DeeObject_CallAttrStringf(*this, "asdigit", "Iuo", index, defl));
	}
	WUNUSED Object asdigit(DeeObject *__restrict index, DeeObject *__restrict defl) const {
		return inherit(DeeObject_CallAttrStringf(*this, "asdigit", "oo", index, defl));
	}
	WUNUSED Object asdecimal(size_t index, DeeObject *__restrict defl) const {
		return inherit(DeeObject_CallAttrStringf(*this, "asdecimal", "Iuo", index, defl));
	}
	WUNUSED Object asdecimal(DeeObject *__restrict index, DeeObject *__restrict defl) const {
		return inherit(DeeObject_CallAttrStringf(*this, "asdecimal", "oo", index, defl));
	}
#define DEFINE_REPLACE_FUNCTION(name)                                                                                           \
	WUNUSED string(name)(DeeObject * __restrict find, DeeObject * __restrict repl) const {                                      \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "oo", find, repl));                                              \
	}                                                                                                                           \
	WUNUSED string(name)(DeeObject * __restrict find, DeeObject * __restrict repl, size_t maxcount) const {                     \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "ooIu", find, repl, maxcount));                                  \
	}                                                                                                                           \
	WUNUSED string(name)(DeeObject * __restrict find, /*utf-8*/ char const *__restrict repl) const {                            \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "os", find, repl));                                              \
	}                                                                                                                           \
	WUNUSED string(name)(DeeObject * __restrict find, /*utf-8*/ char const *__restrict repl, size_t maxcount) const {           \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "osIu", find, repl, maxcount));                                  \
	}                                                                                                                           \
	WUNUSED string(name)(/*utf-8*/ char const *__restrict find, DeeObject *__restrict repl) const {                             \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "so", find, repl));                                              \
	}                                                                                                                           \
	WUNUSED string(name)(/*utf-8*/ char const *__restrict find, DeeObject *__restrict repl, size_t maxcount) const {            \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "soIu", find, repl, maxcount));                                  \
	}                                                                                                                           \
	WUNUSED string(name)(/*utf-8*/ char const *__restrict find, /*utf-8*/ char const *__restrict repl) const {                  \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "ss", find, repl));                                              \
	}                                                                                                                           \
	WUNUSED string(name)(/*utf-8*/ char const *__restrict find, /*utf-8*/ char const *__restrict repl, size_t maxcount) const { \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "ssIu", find, repl, maxcount));                                  \
	}
	DEFINE_REPLACE_FUNCTION(replace)
	DEFINE_REPLACE_FUNCTION(casereplace)
#undef DEFINE_REPLACE_FUNCTION
#define DEFINE_FIND_FUNCTION(Treturn, name, needle)                                                 \
	WUNUSED Treturn(name)(/*utf-8*/ char const *__restrict needle) const;                           \
	WUNUSED Treturn(name)(/*utf-8*/ char const *__restrict needle, size_t start) const;             \
	WUNUSED Treturn(name)(/*utf-8*/ char const *__restrict needle, size_t start, size_t end) const; \
	WUNUSED Treturn(name)(DeeObject * __restrict needle) const;                                     \
	WUNUSED Treturn(name)(DeeObject * __restrict needle, size_t start) const;                       \
	WUNUSED Treturn(name)(DeeObject * __restrict needle, size_t start, size_t end) const;
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
#define DEFINE_FIND_FUNCTION(Treturn, name, needle)                                                  \
	WUNUSED Treturn(name)(/*utf-8*/ char const *__restrict needle) const {                           \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "s", needle));                        \
	}                                                                                                \
	WUNUSED Treturn(name)(/*utf-8*/ char const *__restrict needle, size_t start) const {             \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "sIu", needle, start));               \
	}                                                                                                \
	WUNUSED Treturn(name)(/*utf-8*/ char const *__restrict needle, size_t start, size_t end) const { \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "sIuIu", needle, start, end));        \
	}                                                                                                \
	WUNUSED Treturn(name)(DeeObject * __restrict needle) const {                                     \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "o", needle));                        \
	}                                                                                                \
	WUNUSED Treturn(name)(DeeObject * __restrict needle, size_t start) const {                       \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "oIu", needle, start));               \
	}                                                                                                \
	WUNUSED Treturn(name)(DeeObject * __restrict needle, size_t start, size_t end) const {           \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "oIuIu", needle, start, end));        \
	}
	DEFINE_FIND_FUNCTION(deemon::bool_, contains, needle)
	DEFINE_FIND_FUNCTION(deemon::bool_, startswith, needle)
	DEFINE_FIND_FUNCTION(deemon::bool_, endswith, needle)
	DEFINE_FIND_FUNCTION(Tuple<string>, partition, needle)
	DEFINE_FIND_FUNCTION(Tuple<string>, rpartition, needle)
	DEFINE_FIND_FUNCTION(deemon::bool_, casecontains, needle)
	DEFINE_FIND_FUNCTION(deemon::bool_, casestartswith, needle)
	DEFINE_FIND_FUNCTION(deemon::bool_, caseendswith, needle)
	DEFINE_FIND_FUNCTION(Tuple<string>, casepartition, needle)
	DEFINE_FIND_FUNCTION(Tuple<string>, caserpartition, needle)
#undef DEFINE_FIND_FUNCTION
	WUNUSED string substr(size_t start) const {
		return inherit(DeeObject_CallAttrStringf(*this, "substr", "Iu", start));
	}
	WUNUSED string substr(size_t start, size_t end) const {
		return inherit(DeeObject_CallAttrStringf(*this, "substr", "IuIu", start, end));
	}
#define DEFINE_STRIP_FUNCTION(name)                                                       \
	WUNUSED string(name)() const {                                                        \
		return inherit(DeeObject_CallAttrString(*this, #name, 0, NULL));                  \
	}                                                                                     \
	WUNUSED string(name)(/*utf-8*/ char const *__restrict mask) const {                   \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "s", mask));               \
	}                                                                                     \
	WUNUSED string(name)(/*utf-8*/ char const *__restrict mask, size_t mask_size) const { \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "$s", mask_size, mask));   \
	}                                                                                     \
	WUNUSED string(name)(DeeObject * __restrict mask) const {                             \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "o", mask));               \
	}
	DEFINE_STRIP_FUNCTION(strip)
	DEFINE_STRIP_FUNCTION(lstrip)
	DEFINE_STRIP_FUNCTION(rstrip)
	DEFINE_STRIP_FUNCTION(sstrip)
	DEFINE_STRIP_FUNCTION(lsstrip)
	DEFINE_STRIP_FUNCTION(rsstrip)
	DEFINE_STRIP_FUNCTION(casestrip)
	DEFINE_STRIP_FUNCTION(caselstrip)
	DEFINE_STRIP_FUNCTION(caserstrip)
	DEFINE_STRIP_FUNCTION(casesstrip)
	DEFINE_STRIP_FUNCTION(caselsstrip)
	DEFINE_STRIP_FUNCTION(casersstrip)
#undef DEFINE_STRIP_FUNCTION
#define DEFINE_COMPARE_FUNCTION(Treturn, name)                                                                              \
	WUNUSED Treturn(name)(/*utf-8*/ char const *__restrict other) const;                                                    \
	WUNUSED Treturn(name)(/*utf-8*/ char const *__restrict other, size_t other_size) const;                                 \
	WUNUSED Treturn(name)(size_t my_start, /*utf-8*/ char const *__restrict other) const;                                   \
	WUNUSED Treturn(name)(size_t my_start, /*utf-8*/ char const *__restrict other, size_t other_size) const;                \
	WUNUSED Treturn(name)(size_t my_start, size_t my_end, /*utf-8*/ char const *__restrict other) const;                    \
	WUNUSED Treturn(name)(size_t my_start, size_t my_end, /*utf-8*/ char const *__restrict other, size_t other_size) const; \
	WUNUSED Treturn(name)(DeeObject * __restrict other) const;                                                              \
	WUNUSED Treturn(name)(DeeObject * __restrict other, size_t other_start) const;                                          \
	WUNUSED Treturn(name)(DeeObject * __restrict other, size_t other_start, size_t other_end) const;                        \
	WUNUSED Treturn(name)(size_t my_start, DeeObject * __restrict other) const;                                             \
	WUNUSED Treturn(name)(size_t my_start, DeeObject * __restrict other, size_t other_start) const;                         \
	WUNUSED Treturn(name)(size_t my_start, DeeObject * __restrict other, size_t other_start, size_t other_end) const;       \
	WUNUSED Treturn(name)(size_t my_start, size_t my_end, DeeObject * __restrict other) const;                              \
	WUNUSED Treturn(name)(size_t my_start, size_t my_end, DeeObject * __restrict other, size_t other_start) const;          \
	WUNUSED Treturn(name)(size_t my_start, size_t my_end, DeeObject * __restrict other, size_t other_start, size_t other_end) const;
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
#define DEFINE_COMPARE_FUNCTION(Treturn, name)                                                                                        \
	WUNUSED Treturn(name)(/*utf-8*/ char const *__restrict other) const {                                                             \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "s", other));                                                          \
	}                                                                                                                                 \
	WUNUSED Treturn(name)(/*utf-8*/ char const *__restrict other, size_t other_size) const {                                          \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "$s", other_size, other));                                             \
	}                                                                                                                                 \
	WUNUSED Treturn(name)(size_t my_start, /*utf-8*/ char const *__restrict other) const {                                            \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "Ius", my_start, other));                                              \
	}                                                                                                                                 \
	WUNUSED Treturn(name)(size_t my_start, /*utf-8*/ char const *__restrict other, size_t other_size) const {                         \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "Iu$s", my_start, other_size, other));                                 \
	}                                                                                                                                 \
	WUNUSED Treturn(name)(size_t my_start, size_t my_end, /*utf-8*/ char const *__restrict other) const {                             \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "IuIus", my_start, my_end, other));                                    \
	}                                                                                                                                 \
	WUNUSED Treturn(name)(size_t my_start, size_t my_end, /*utf-8*/ char const *__restrict other, size_t other_size) const {          \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "IuIu$s", my_start, my_end, other_size, other));                       \
	}                                                                                                                                 \
	WUNUSED Treturn(name)(DeeObject * __restrict other) const {                                                                       \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "o", other));                                                          \
	}                                                                                                                                 \
	WUNUSED Treturn(name)(DeeObject * __restrict other, size_t other_start) const {                                                   \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "oIu", other, other_start));                                           \
	}                                                                                                                                 \
	WUNUSED Treturn(name)(DeeObject * __restrict other, size_t other_start, size_t other_end) const {                                 \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "oIuIu", other, other_start, other_end));                              \
	}                                                                                                                                 \
	WUNUSED Treturn(name)(size_t my_start, DeeObject * __restrict other) const {                                                      \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "Iuo", my_start, other));                                              \
	}                                                                                                                                 \
	WUNUSED Treturn(name)(size_t my_start, DeeObject * __restrict other, size_t other_start) const {                                  \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "IuoIu", my_start, other, other_start));                               \
	}                                                                                                                                 \
	WUNUSED Treturn(name)(size_t my_start, DeeObject * __restrict other, size_t other_start, size_t other_end) const {                \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "IuoIuIu", my_start, other, other_start, other_end));                  \
	}                                                                                                                                 \
	WUNUSED Treturn(name)(size_t my_start, size_t my_end, DeeObject * __restrict other) const {                                       \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "IuIuo", my_start, my_end, other));                                    \
	}                                                                                                                                 \
	WUNUSED Treturn(name)(size_t my_start, size_t my_end, DeeObject * __restrict other, size_t other_start) const {                   \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "IuIuoIu", my_start, my_end, other, other_start));                     \
	}                                                                                                                                 \
	WUNUSED Treturn(name)(size_t my_start, size_t my_end, DeeObject * __restrict other, size_t other_start, size_t other_end) const { \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "IuIuoIuIu", my_start, my_end, other, other_start, other_end));        \
	}
	DEFINE_COMPARE_FUNCTION(deemon::bool_, wmatch)
	DEFINE_COMPARE_FUNCTION(deemon::bool_, casewmatch)
#undef DEFINE_COMPARE_FUNCTION
#define DEFINE_CENTER_FUNCTION(name)                                                                        \
	WUNUSED string(name)(size_t width) const {                                                              \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "Iu", width));                               \
	}                                                                                                       \
	WUNUSED string(name)(size_t width, DeeObject * __restrict filler) const {                               \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "Iuo", width, filler));                      \
	}                                                                                                       \
	WUNUSED string(name)(size_t width, /*utf-8*/ char const *__restrict filler) const {                     \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "Ius", width, filler));                      \
	}                                                                                                       \
	WUNUSED string(name)(size_t width, /*utf-8*/ char const *__restrict filler, size_t filler_size) const { \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "Iu$s", width, filler_size, filler));        \
	}
	DEFINE_CENTER_FUNCTION(center)
	DEFINE_CENTER_FUNCTION(ljust)
	DEFINE_CENTER_FUNCTION(rjust)
	DEFINE_CENTER_FUNCTION(zfill)
#undef DEFINE_CENTER_FUNCTION
	WUNUSED string expandtabs() const {
		return inherit(DeeObject_CallAttrString(*this, "expandtabs", 0, NULL));
	}
	WUNUSED string expandtabs(size_t tab_width) const {
		return inherit(DeeObject_CallAttrStringf(*this, "expandtabs", "Iu", tab_width));
	}
#define DEFINE_UNIFYLINES(name, replacement)                                                            \
	WUNUSED string(name)() const {                                                                      \
		return inherit(DeeObject_CallAttrString(*this, #name, 0, NULL));                                \
	}                                                                                                   \
	WUNUSED string(name)(DeeObject * __restrict replacement) const {                                    \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "o", replacement));                      \
	}                                                                                                   \
	WUNUSED string(name)(/*utf-8*/ char const *__restrict replacement) const {                          \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "s", replacement));                      \
	}                                                                                                   \
	WUNUSED string(name)(/*utf-8*/ char const *__restrict replacement, size_t replacement_size) const { \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "$s", replacement_size, replacement));   \
	}
	DEFINE_UNIFYLINES(unifylines, replacement)
	DEFINE_UNIFYLINES(indent, filler)
#undef DEFINE_UNIFYLINES
	WUNUSED string join(DeeObject *__restrict seq) const {
		return inherit(DeeObject_CallAttrStringf(*this, "join", "o", seq));
	}
	WUNUSED string join(std::initializer_list<DeeObject *> const &seq) const {
		Tuple<Object> seq_obj(seq);
		return inherit(DeeObject_CallAttrStringf(*this, "join", "o", seq_obj.ptr()));
	}
#define DEFINE_SPLIT_FUNCTION(name)                                                               \
	WUNUSED Sequence<string>(name)(DeeObject * __restrict sep) const {                            \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "o", sep));                        \
	}                                                                                             \
	WUNUSED Sequence<string>(name)(/*utf-8*/ char const *__restrict sep) const {                  \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "s", sep));                        \
	}                                                                                             \
	WUNUSED Sequence<string>(name)(/*utf-8*/ char const *__restrict sep, size_t sep_size) const { \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "$s", sep_size, sep));             \
	}
	DEFINE_SPLIT_FUNCTION(split)
	DEFINE_SPLIT_FUNCTION(casesplit)
#undef DEFINE_SPLIT_FUNCTION
	WUNUSED Sequence<string> splitlines() const {
		return inherit(DeeObject_CallAttrString(*this, "splitlines", 0, NULL));
	}
	WUNUSED Sequence<string> splitlines(bool keepends) const {
		return inherit(DeeObject_CallAttrStringf(*this, "join", "b", keepends));
	}
	WUNUSED string dedent() const {
		return inherit(DeeObject_CallAttrString(*this, "dedent", 0, NULL));
	}
	WUNUSED string dedent(size_t max_chars) const {
		return inherit(DeeObject_CallAttrStringf(*this, "dedent", "Iu", max_chars));
	}
	WUNUSED string dedent(size_t max_chars, DeeObject *__restrict mask) const {
		return inherit(DeeObject_CallAttrStringf(*this, "dedent", "Iuo", max_chars, mask));
	}
	WUNUSED string dedent(size_t max_chars, /*utf-8*/ char const *__restrict mask) const {
		return inherit(DeeObject_CallAttrStringf(*this, "dedent", "Ius", max_chars, mask));
	}
	WUNUSED string dedent(size_t max_chars, /*utf-8*/ char const *__restrict mask, size_t mask_size) const {
		return inherit(DeeObject_CallAttrStringf(*this, "dedent", "Iu$s", max_chars, mask_size, mask));
	}
#define DEFINE_FINDMATCH_FUNCTION(Treturn, name)                                                                                \
	WUNUSED Treturn(name)(DeeObject * __restrict open, DeeObject * __restrict close) const;                                     \
	WUNUSED Treturn(name)(DeeObject * __restrict open, DeeObject * __restrict close, size_t start) const;                       \
	WUNUSED Treturn(name)(DeeObject * __restrict open, DeeObject * __restrict close, size_t start, size_t end) const;           \
	WUNUSED Treturn(name)(DeeObject * __restrict open, /*utf-8*/ char const *__restrict close) const;                           \
	WUNUSED Treturn(name)(DeeObject * __restrict open, /*utf-8*/ char const *__restrict close, size_t start) const;             \
	WUNUSED Treturn(name)(DeeObject * __restrict open, /*utf-8*/ char const *__restrict close, size_t start, size_t end) const; \
	WUNUSED Treturn(name)(/*utf-8*/ char const *__restrict open, DeeObject *__restrict close) const;                            \
	WUNUSED Treturn(name)(/*utf-8*/ char const *__restrict open, DeeObject *__restrict close, size_t start) const;              \
	WUNUSED Treturn(name)(/*utf-8*/ char const *__restrict open, DeeObject *__restrict close, size_t start, size_t end) const;  \
	WUNUSED Treturn(name)(/*utf-8*/ char const *__restrict open, /*utf-8*/ char const *__restrict close) const;                 \
	WUNUSED Treturn(name)(/*utf-8*/ char const *__restrict open, /*utf-8*/ char const *__restrict close, size_t start) const;   \
	WUNUSED Treturn(name)(/*utf-8*/ char const *__restrict open, /*utf-8*/ char const *__restrict close, size_t start, size_t end) const;
	DEFINE_FINDMATCH_FUNCTION(deemon::int_, findmatch)
	DEFINE_FINDMATCH_FUNCTION(deemon::int_, indexmatch)
	DEFINE_FINDMATCH_FUNCTION(deemon::int_, rfindmatch)
	DEFINE_FINDMATCH_FUNCTION(deemon::int_, rindexmatch)
	DEFINE_FINDMATCH_FUNCTION(Sequence<deemon::int_>, casefindmatch)
	DEFINE_FINDMATCH_FUNCTION(Sequence<deemon::int_>, caseindexmatch)
	DEFINE_FINDMATCH_FUNCTION(Sequence<deemon::int_>, caserfindmatch)
	DEFINE_FINDMATCH_FUNCTION(Sequence<deemon::int_>, caserindexmatch)
#undef DEFINE_FINDMATCH_FUNCTION
#define DEFINE_FINDMATCH_FUNCTION(Treturn, name)                                                                                           \
	WUNUSED Treturn(name)(DeeObject * __restrict open, DeeObject * __restrict close) const {                                               \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "oo", open, close));                                                        \
	}                                                                                                                                      \
	WUNUSED Treturn(name)(DeeObject * __restrict open, DeeObject * __restrict close, size_t start) const {                                 \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "ooIu", open, close, start));                                               \
	}                                                                                                                                      \
	WUNUSED Treturn(name)(DeeObject * __restrict open, DeeObject * __restrict close, size_t start, size_t end) const {                     \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "ooIuIu", open, close, start, end));                                        \
	}                                                                                                                                      \
	WUNUSED Treturn(name)(DeeObject * __restrict open, /*utf-8*/ char const *__restrict close) const {                                     \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "os", open, close));                                                        \
	}                                                                                                                                      \
	WUNUSED Treturn(name)(DeeObject * __restrict open, /*utf-8*/ char const *__restrict close, size_t start) const {                       \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "osIu", open, close, start));                                               \
	}                                                                                                                                      \
	WUNUSED Treturn(name)(DeeObject * __restrict open, /*utf-8*/ char const *__restrict close, size_t start, size_t end) const {           \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "osIuIu", open, close, start, end));                                        \
	}                                                                                                                                      \
	WUNUSED Treturn(name)(/*utf-8*/ char const *__restrict open, DeeObject *__restrict close) const {                                      \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "so", open, close));                                                        \
	}                                                                                                                                      \
	WUNUSED Treturn(name)(/*utf-8*/ char const *__restrict open, DeeObject *__restrict close, size_t start) const {                        \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "soIu", open, close, start));                                               \
	}                                                                                                                                      \
	WUNUSED Treturn(name)(/*utf-8*/ char const *__restrict open, DeeObject *__restrict close, size_t start, size_t end) const {            \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "soIuIu", open, close, start, end));                                        \
	}                                                                                                                                      \
	WUNUSED Treturn(name)(/*utf-8*/ char const *__restrict open, /*utf-8*/ char const *__restrict close) const {                           \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "ss", open, close));                                                        \
	}                                                                                                                                      \
	WUNUSED Treturn(name)(/*utf-8*/ char const *__restrict open, /*utf-8*/ char const *__restrict close, size_t start) const {             \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "ssIu", open, close, start));                                               \
	}                                                                                                                                      \
	WUNUSED Treturn(name)(/*utf-8*/ char const *__restrict open, /*utf-8*/ char const *__restrict close, size_t start, size_t end) const { \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "ssIuIu", open, close, start, end));                                        \
	}
	DEFINE_FINDMATCH_FUNCTION(Sequence<string>, partitionmatch)
	DEFINE_FINDMATCH_FUNCTION(Sequence<string>, rpartitionmatch)
	DEFINE_FINDMATCH_FUNCTION(Sequence<string>, casepartitionmatch)
	DEFINE_FINDMATCH_FUNCTION(Sequence<string>, caserpartitionmatch)
#undef DEFINE_FINDMATCH_FUNCTION
	WUNUSED Sequence<string> segments(size_t substring_length) const {
		return inherit(DeeObject_CallAttrStringf(*this, "segments", "Iu", substring_length));
	}
	WUNUSED Sequence<string> segments(DeeObject *__restrict substring_length) const {
		return inherit(DeeObject_CallAttrStringf(*this, "segments", "o", substring_length));
	}
	WUNUSED Sequence<string> distribute(size_t substring_count) const {
		return inherit(DeeObject_CallAttrStringf(*this, "distribute", "Iu", substring_count));
	}
	WUNUSED Sequence<string> distribute(DeeObject *__restrict substring_count) const {
		return inherit(DeeObject_CallAttrStringf(*this, "distribute", "o", substring_count));
	}
#define DEFINE_RE_FUNCTION(Treturn, name)                                                                                                    \
	WUNUSED Treturn(name)(DeeObject * __restrict pattern) const;                                                                             \
	WUNUSED Treturn(name)(DeeObject * __restrict pattern, DeeObject * __restrict rules) const;                                               \
	WUNUSED Treturn(name)(DeeObject * __restrict pattern, /*utf-8*/ char const *__restrict rules) const;                                     \
	WUNUSED Treturn(name)(DeeObject * __restrict pattern, size_t start) const;                                                               \
	WUNUSED Treturn(name)(DeeObject * __restrict pattern, DeeObject * __restrict rules, size_t start) const;                                 \
	WUNUSED Treturn(name)(DeeObject * __restrict pattern, /*utf-8*/ char const *__restrict rules, size_t start) const;                       \
	WUNUSED Treturn(name)(DeeObject * __restrict pattern, size_t start, size_t end) const;                                                   \
	WUNUSED Treturn(name)(DeeObject * __restrict pattern, DeeObject * __restrict rules, size_t start, size_t end) const;                     \
	WUNUSED Treturn(name)(DeeObject * __restrict pattern, /*utf-8*/ char const *__restrict rules, size_t start, size_t end) const;           \
	WUNUSED Treturn(name)(DeeObject * __restrict pattern, size_t start, DeeObject * __restrict rules) const;                                 \
	WUNUSED Treturn(name)(DeeObject * __restrict pattern, size_t start, size_t end, DeeObject * __restrict rules) const;                     \
	WUNUSED Treturn(name)(DeeObject * __restrict pattern, size_t start, /*utf-8*/ char const *__restrict rules) const;                       \
	WUNUSED Treturn(name)(DeeObject * __restrict pattern, size_t start, size_t end, /*utf-8*/ char const *__restrict rules) const;           \
	WUNUSED Treturn(name)(/*utf-8*/ char const *__restrict pattern) const;                                                                   \
	WUNUSED Treturn(name)(/*utf-8*/ char const *__restrict pattern, DeeObject *__restrict rules) const;                                      \
	WUNUSED Treturn(name)(/*utf-8*/ char const *__restrict pattern, /*utf-8*/ char const *__restrict rules) const;                           \
	WUNUSED Treturn(name)(/*utf-8*/ char const *__restrict pattern, size_t start) const;                                                     \
	WUNUSED Treturn(name)(/*utf-8*/ char const *__restrict pattern, DeeObject *__restrict rules, size_t start) const;                        \
	WUNUSED Treturn(name)(/*utf-8*/ char const *__restrict pattern, /*utf-8*/ char const *__restrict rules, size_t start) const;             \
	WUNUSED Treturn(name)(/*utf-8*/ char const *__restrict pattern, size_t start, size_t end) const;                                         \
	WUNUSED Treturn(name)(/*utf-8*/ char const *__restrict pattern, DeeObject *__restrict rules, size_t start, size_t end) const;            \
	WUNUSED Treturn(name)(/*utf-8*/ char const *__restrict pattern, /*utf-8*/ char const *__restrict rules, size_t start, size_t end) const; \
	WUNUSED Treturn(name)(/*utf-8*/ char const *__restrict pattern, size_t start, DeeObject *__restrict rules) const;                        \
	WUNUSED Treturn(name)(/*utf-8*/ char const *__restrict pattern, size_t start, size_t end, DeeObject *__restrict rules) const;            \
	WUNUSED Treturn(name)(/*utf-8*/ char const *__restrict pattern, size_t start, /*utf-8*/ char const *__restrict rules) const;             \
	WUNUSED Treturn(name)(/*utf-8*/ char const *__restrict pattern, size_t start, size_t end, /*utf-8*/ char const *__restrict rules) const;
	DEFINE_RE_FUNCTION(deemon::int_, rematch)
	DEFINE_RE_FUNCTION(Sequence<deemon::int_>, refind)
	DEFINE_RE_FUNCTION(Sequence<deemon::int_>, rerfind)
	DEFINE_RE_FUNCTION(Sequence<deemon::int_>, reindex)
	DEFINE_RE_FUNCTION(Sequence<deemon::int_>, rerindex)
	DEFINE_RE_FUNCTION(Sequence<Sequence<deemon::int_> >, refindall)
	DEFINE_RE_FUNCTION(deemon::int_, recount)
#undef DEFINE_RE_FUNCTION
#define DEFINE_RE_FUNCTION(Treturn, name)                                                                                                     \
	WUNUSED Treturn(name)(DeeObject * __restrict pattern) const {                                                                             \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "o", pattern));                                                                \
	}                                                                                                                                         \
	WUNUSED Treturn(name)(DeeObject * __restrict pattern, DeeObject * __restrict rules) const {                                               \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "oo", pattern, rules));                                                        \
	}                                                                                                                                         \
	WUNUSED Treturn(name)(DeeObject * __restrict pattern, /*utf-8*/ char const *__restrict rules) const {                                     \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "os", pattern, rules));                                                        \
	}                                                                                                                                         \
	WUNUSED Treturn(name)(DeeObject * __restrict pattern, size_t start) const {                                                               \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "oIu", pattern, start));                                                       \
	}                                                                                                                                         \
	WUNUSED Treturn(name)(DeeObject * __restrict pattern, DeeObject * __restrict rules, size_t start) const {                                 \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "ooIu", pattern, rules, start));                                               \
	}                                                                                                                                         \
	WUNUSED Treturn(name)(DeeObject * __restrict pattern, /*utf-8*/ char const *__restrict rules, size_t start) const {                       \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "osIu", pattern, rules, start));                                               \
	}                                                                                                                                         \
	WUNUSED Treturn(name)(DeeObject * __restrict pattern, size_t start, size_t end) const {                                                   \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "oIuIu", pattern, start, end));                                                \
	}                                                                                                                                         \
	WUNUSED Treturn(name)(DeeObject * __restrict pattern, DeeObject * __restrict rules, size_t start, size_t end) const {                     \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "ooIuIu", pattern, rules, start, end));                                        \
	}                                                                                                                                         \
	WUNUSED Treturn(name)(DeeObject * __restrict pattern, /*utf-8*/ char const *__restrict rules, size_t start, size_t end) const {           \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "osIuIu", pattern, rules, start, end));                                        \
	}                                                                                                                                         \
	WUNUSED Treturn(name)(DeeObject * __restrict pattern, size_t start, DeeObject * __restrict rules) const {                                 \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "oIuo", pattern, start, rules));                                               \
	}                                                                                                                                         \
	WUNUSED Treturn(name)(DeeObject * __restrict pattern, size_t start, size_t end, DeeObject * __restrict rules) const {                     \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "oIuIuo", pattern, start, end, rules));                                        \
	}                                                                                                                                         \
	WUNUSED Treturn(name)(DeeObject * __restrict pattern, size_t start, /*utf-8*/ char const *__restrict rules) const {                       \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "oIus", pattern, start, rules));                                               \
	}                                                                                                                                         \
	WUNUSED Treturn(name)(DeeObject * __restrict pattern, size_t start, size_t end, /*utf-8*/ char const *__restrict rules) const {           \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "oIuIus", pattern, start, end, rules));                                        \
	}                                                                                                                                         \
	WUNUSED Treturn(name)(/*utf-8*/ char const *__restrict pattern) const {                                                                   \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "s", pattern));                                                                \
	}                                                                                                                                         \
	WUNUSED Treturn(name)(/*utf-8*/ char const *__restrict pattern, DeeObject *__restrict rules) const {                                      \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "so", pattern, rules));                                                        \
	}                                                                                                                                         \
	WUNUSED Treturn(name)(/*utf-8*/ char const *__restrict pattern, /*utf-8*/ char const *__restrict rules) const {                           \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "ss", pattern, rules));                                                        \
	}                                                                                                                                         \
	WUNUSED Treturn(name)(/*utf-8*/ char const *__restrict pattern, size_t start) const {                                                     \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "sIu", pattern, start));                                                       \
	}                                                                                                                                         \
	WUNUSED Treturn(name)(/*utf-8*/ char const *__restrict pattern, DeeObject *__restrict rules, size_t start) const {                        \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "soIu", pattern, rules, start));                                               \
	}                                                                                                                                         \
	WUNUSED Treturn(name)(/*utf-8*/ char const *__restrict pattern, /*utf-8*/ char const *__restrict rules, size_t start) const {             \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "ssIu", pattern, rules, start));                                               \
	}                                                                                                                                         \
	WUNUSED Treturn(name)(/*utf-8*/ char const *__restrict pattern, size_t start, size_t end) const {                                         \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "sIuIu", pattern, start, end));                                                \
	}                                                                                                                                         \
	WUNUSED Treturn(name)(/*utf-8*/ char const *__restrict pattern, DeeObject *__restrict rules, size_t start, size_t end) const {            \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "soIuIu", pattern, rules, start, end));                                        \
	}                                                                                                                                         \
	WUNUSED Treturn(name)(/*utf-8*/ char const *__restrict pattern, /*utf-8*/ char const *__restrict rules, size_t start, size_t end) const { \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "ssIuIu", pattern, rules, start, end));                                        \
	}                                                                                                                                         \
	WUNUSED Treturn(name)(/*utf-8*/ char const *__restrict pattern, size_t start, DeeObject *__restrict rules) const {                        \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "sIuo", pattern, start, rules));                                               \
	}                                                                                                                                         \
	WUNUSED Treturn(name)(/*utf-8*/ char const *__restrict pattern, size_t start, size_t end, DeeObject *__restrict rules) const {            \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "sIuIuo", pattern, start, end, rules));                                        \
	}                                                                                                                                         \
	WUNUSED Treturn(name)(/*utf-8*/ char const *__restrict pattern, size_t start, /*utf-8*/ char const *__restrict rules) const {             \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "sIus", pattern, start, rules));                                               \
	}                                                                                                                                         \
	WUNUSED Treturn(name)(/*utf-8*/ char const *__restrict pattern, size_t start, size_t end, /*utf-8*/ char const *__restrict rules) const { \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "sIuIus", pattern, start, end, rules));                                        \
	}
	DEFINE_RE_FUNCTION(string, relocate)
	DEFINE_RE_FUNCTION(string, rerlocate)
	DEFINE_RE_FUNCTION(Sequence<string>, repartition)
	DEFINE_RE_FUNCTION(Sequence<string>, rerpartition)
	DEFINE_RE_FUNCTION(Sequence<string>, relocateall)
	DEFINE_RE_FUNCTION(Sequence<string>, resplit)
	DEFINE_RE_FUNCTION(deemon::bool_, restartswith)
	DEFINE_RE_FUNCTION(deemon::bool_, reendswith)
	DEFINE_RE_FUNCTION(string, restrip)
	DEFINE_RE_FUNCTION(string, relstrip)
	DEFINE_RE_FUNCTION(string, rerstrip)
	DEFINE_RE_FUNCTION(deemon::bool_, recontains)
#undef DEFINE_RE_FUNCTION
	WUNUSED string rereplace(DeeObject *__restrict pattern, DeeObject *__restrict repl) const {
		return inherit(DeeObject_CallAttrStringf(*this, "rereplace", "oo", pattern, repl));
	}
	WUNUSED string rereplace(DeeObject *__restrict pattern, DeeObject *__restrict repl, size_t maxcount) const {
		return inherit(DeeObject_CallAttrStringf(*this, "rereplace", "ooIu", pattern, repl, maxcount));
	}
	WUNUSED string rereplace(DeeObject *__restrict pattern, /*utf-8*/ char const *__restrict repl) const {
		return inherit(DeeObject_CallAttrStringf(*this, "rereplace", "os", pattern, repl));
	}
	WUNUSED string rereplace(DeeObject *__restrict pattern, /*utf-8*/ char const *__restrict repl, size_t maxcount) const {
		return inherit(DeeObject_CallAttrStringf(*this, "rereplace", "osIu", pattern, repl, maxcount));
	}
	WUNUSED string rereplace(/*utf-8*/ char const *__restrict pattern, DeeObject *__restrict repl) const {
		return inherit(DeeObject_CallAttrStringf(*this, "rereplace", "so", pattern, repl));
	}
	WUNUSED string rereplace(/*utf-8*/ char const *__restrict pattern, DeeObject *__restrict repl, size_t maxcount) const {
		return inherit(DeeObject_CallAttrStringf(*this, "rereplace", "soIu", pattern, repl, maxcount));
	}
	WUNUSED string rereplace(/*utf-8*/ char const *__restrict pattern, /*utf-8*/ char const *__restrict repl) const {
		return inherit(DeeObject_CallAttrStringf(*this, "rereplace", "ss", pattern, repl));
	}
	WUNUSED string rereplace(/*utf-8*/ char const *__restrict pattern, /*utf-8*/ char const *__restrict repl, size_t maxcount) const {
		return inherit(DeeObject_CallAttrStringf(*this, "rereplace", "ssIu", pattern, repl, maxcount));
	}
	WUNUSED string rereplace(DeeObject *__restrict pattern, DeeObject *__restrict repl, DeeObject *__restrict rules) const {
		return inherit(DeeObject_CallAttrStringf(*this, "rereplace", "ooo", pattern, repl, rules));
	}
	WUNUSED string rereplace(DeeObject *__restrict pattern, DeeObject *__restrict repl, size_t maxcount, DeeObject *__restrict rules) const {
		return inherit(DeeObject_CallAttrStringf(*this, "rereplace", "ooIuo", pattern, repl, maxcount, rules));
	}
	WUNUSED string rereplace(DeeObject *__restrict pattern, DeeObject *__restrict repl, DeeObject *__restrict rules, size_t maxcount) const {
		return inherit(DeeObject_CallAttrStringf(*this, "rereplace", "oooIu", pattern, repl, rules, maxcount));
	}
	WUNUSED string rereplace(DeeObject *__restrict pattern, /*utf-8*/ char const *__restrict repl, DeeObject *__restrict rules) const {
		return inherit(DeeObject_CallAttrStringf(*this, "rereplace", "oso", pattern, repl, rules));
	}
	WUNUSED string rereplace(DeeObject *__restrict pattern, /*utf-8*/ char const *__restrict repl, size_t maxcount, DeeObject *__restrict rules) const {
		return inherit(DeeObject_CallAttrStringf(*this, "rereplace", "osIuo", pattern, repl, maxcount, rules));
	}
	WUNUSED string rereplace(DeeObject *__restrict pattern, /*utf-8*/ char const *__restrict repl, DeeObject *__restrict rules, size_t maxcount) const {
		return inherit(DeeObject_CallAttrStringf(*this, "rereplace", "osoIu", pattern, repl, rules, maxcount));
	}
	WUNUSED string rereplace(/*utf-8*/ char const *__restrict pattern, DeeObject *__restrict repl, DeeObject *__restrict rules) const {
		return inherit(DeeObject_CallAttrStringf(*this, "rereplace", "soo", pattern, repl, rules));
	}
	WUNUSED string rereplace(/*utf-8*/ char const *__restrict pattern, DeeObject *__restrict repl, size_t maxcount, DeeObject *__restrict rules) const {
		return inherit(DeeObject_CallAttrStringf(*this, "rereplace", "soIuo", pattern, repl, maxcount, rules));
	}
	WUNUSED string rereplace(/*utf-8*/ char const *__restrict pattern, DeeObject *__restrict repl, DeeObject *__restrict rules, size_t maxcount) const {
		return inherit(DeeObject_CallAttrStringf(*this, "rereplace", "sooIu", pattern, repl, rules, maxcount));
	}
	WUNUSED string rereplace(/*utf-8*/ char const *__restrict pattern, /*utf-8*/ char const *__restrict repl, DeeObject *__restrict rules) const {
		return inherit(DeeObject_CallAttrStringf(*this, "rereplace", "sso", pattern, repl, rules));
	}
	WUNUSED string rereplace(/*utf-8*/ char const *__restrict pattern, /*utf-8*/ char const *__restrict repl, size_t maxcount, DeeObject *__restrict rules) const {
		return inherit(DeeObject_CallAttrStringf(*this, "rereplace", "ssIuo", pattern, repl, maxcount, rules));
	}
	WUNUSED string rereplace(/*utf-8*/ char const *__restrict pattern, /*utf-8*/ char const *__restrict repl, DeeObject *__restrict rules, size_t maxcount) const {
		return inherit(DeeObject_CallAttrStringf(*this, "rereplace", "ssoIu", pattern, repl, rules, maxcount));
	}
	WUNUSED string rereplace(DeeObject *__restrict pattern, DeeObject *__restrict repl, /*utf-8*/ char const *__restrict rules) const {
		return inherit(DeeObject_CallAttrStringf(*this, "rereplace", "oos", pattern, repl, rules));
	}
	WUNUSED string rereplace(DeeObject *__restrict pattern, DeeObject *__restrict repl, size_t maxcount, /*utf-8*/ char const *__restrict rules) const {
		return inherit(DeeObject_CallAttrStringf(*this, "rereplace", "ooIus", pattern, repl, maxcount, rules));
	}
	WUNUSED string rereplace(DeeObject *__restrict pattern, DeeObject *__restrict repl, /*utf-8*/ char const *__restrict rules, size_t maxcount) const {
		return inherit(DeeObject_CallAttrStringf(*this, "rereplace", "oosIu", pattern, repl, rules, maxcount));
	}
	WUNUSED string rereplace(DeeObject *__restrict pattern, /*utf-8*/ char const *__restrict repl, /*utf-8*/ char const *__restrict rules) const {
		return inherit(DeeObject_CallAttrStringf(*this, "rereplace", "oss", pattern, repl, rules));
	}
	WUNUSED string rereplace(DeeObject *__restrict pattern, /*utf-8*/ char const *__restrict repl, size_t maxcount, /*utf-8*/ char const *__restrict rules) const {
		return inherit(DeeObject_CallAttrStringf(*this, "rereplace", "osIus", pattern, repl, maxcount, rules));
	}
	WUNUSED string rereplace(DeeObject *__restrict pattern, /*utf-8*/ char const *__restrict repl, /*utf-8*/ char const *__restrict rules, size_t maxcount) const {
		return inherit(DeeObject_CallAttrStringf(*this, "rereplace", "ossIu", pattern, repl, rules, maxcount));
	}
	WUNUSED string rereplace(/*utf-8*/ char const *__restrict pattern, DeeObject *__restrict repl, /*utf-8*/ char const *__restrict rules) const {
		return inherit(DeeObject_CallAttrStringf(*this, "rereplace", "sos", pattern, repl, rules));
	}
	WUNUSED string rereplace(/*utf-8*/ char const *__restrict pattern, DeeObject *__restrict repl, size_t maxcount, /*utf-8*/ char const *__restrict rules) const {
		return inherit(DeeObject_CallAttrStringf(*this, "rereplace", "soIus", pattern, repl, maxcount, rules));
	}
	WUNUSED string rereplace(/*utf-8*/ char const *__restrict pattern, DeeObject *__restrict repl, /*utf-8*/ char const *__restrict rules, size_t maxcount) const {
		return inherit(DeeObject_CallAttrStringf(*this, "rereplace", "sosIu", pattern, repl, rules, maxcount));
	}
	WUNUSED string rereplace(/*utf-8*/ char const *__restrict pattern, /*utf-8*/ char const *__restrict repl, /*utf-8*/ char const *__restrict rules) const {
		return inherit(DeeObject_CallAttrStringf(*this, "rereplace", "sss", pattern, repl, rules));
	}
	WUNUSED string rereplace(/*utf-8*/ char const *__restrict pattern, /*utf-8*/ char const *__restrict repl, size_t maxcount, /*utf-8*/ char const *__restrict rules) const {
		return inherit(DeeObject_CallAttrStringf(*this, "rereplace", "ssIus", pattern, repl, maxcount, rules));
	}
	WUNUSED string rereplace(/*utf-8*/ char const *__restrict pattern, /*utf-8*/ char const *__restrict repl, /*utf-8*/ char const *__restrict rules, size_t maxcount) const {
		return inherit(DeeObject_CallAttrStringf(*this, "rereplace", "sssIu", pattern, repl, rules, maxcount));
	}

public:
	WUNUSED string add(DeeObject *__restrict right) const {
		return inherit(DeeObject_Add(*this, right));
	}
	WUNUSED string operator+(DeeObject *__restrict right) const {
		return inherit(DeeObject_Add(*this, right));
	}
	WUNUSED string &inplace_add(DeeObject *__restrict right) {
		Sequence<string>::inplace_add(right);
		return *this;
	}
	WUNUSED string &operator+=(DeeObject *__restrict right) {
		Sequence<string>::operator+=(right);
		return *this;
	}
	WUNUSED string mul(DeeObject *__restrict n) const {
		return inherit(DeeObject_Mul(*this, n));
	}
	WUNUSED string mul(int8_t n) const {
		return inherit(DeeObject_MulInt(*this, n));
	}
	WUNUSED string mul(size_t n) const {
		return inherit(_mulref(n));
	}
	WUNUSED string operator*(DeeObject *__restrict n) const {
		return inherit(DeeObject_Mul(*this, n));
	}
	WUNUSED string operator*(int8_t n) const {
		return inherit(DeeObject_MulInt(*this, n));
	}
	WUNUSED string operator*(size_t n) const {
		return inherit(_mulref(n));
	}
	WUNUSED string &inplace_mul(DeeObject *__restrict n) {
		Sequence<string>::inplace_mul(n);
		return *this;
	}
	WUNUSED string &inplace_mul(int8_t n) {
		Sequence<string>::inplace_mul(n);
		return *this;
	}
	WUNUSED string &inplace_mul(size_t n) {
		Sequence<string>::inplace_mul(n);
		return *this;
	}
	WUNUSED string &operator*=(DeeObject *__restrict n) {
		Sequence<string>::inplace_mul(n);
		return *this;
	}
	WUNUSED string &operator*=(int8_t n) {
		Sequence<string>::inplace_mul(n);
		return *this;
	}
	WUNUSED string &operator*=(size_t n) {
		Sequence<string>::inplace_mul(n);
		return *this;
	}
};


#ifdef GUARD_DEEMON_CXX_INT_H
WUNUSED deemon::int_ string::asnumber() const {
	return inherit(DeeObject_CallAttrString(*this, "asnumber", 0, NULL));
}
WUNUSED deemon::int_ string::asnumber(size_t index) const {
	return inherit(DeeObject_CallAttrStringf(*this, "asnumber", "Iu", index));
}
WUNUSED deemon::int_ string::asnumber(size_t index, int defl) const {
	return inherit(DeeObject_CallAttrStringf(*this, "asnumber", "Iud", index, defl));
}
WUNUSED deemon::int_ string::asnumber(DeeObject *__restrict index) const {
	return inherit(DeeObject_CallAttrStringf(*this, "asnumber", "o", index));
}
WUNUSED deemon::int_ string::asnumber(DeeObject *__restrict index, int defl) const {
	return inherit(DeeObject_CallAttrStringf(*this, "asnumber", "od", index, defl));
}
WUNUSED deemon::int_ string::asdigit() const {
	return inherit(DeeObject_CallAttrString(*this, "asdigit", 0, NULL));
}
WUNUSED deemon::int_ string::asdigit(size_t index) const {
	return inherit(DeeObject_CallAttrStringf(*this, "asdigit", "Iu", index));
}
WUNUSED deemon::int_ string::asdigit(size_t index, int defl) const {
	return inherit(DeeObject_CallAttrStringf(*this, "asdigit", "Iud", index, defl));
}
WUNUSED deemon::int_ string::asdigit(DeeObject *__restrict index) const {
	return inherit(DeeObject_CallAttrStringf(*this, "asdigit", "o", index));
}
WUNUSED deemon::int_ string::asdigit(DeeObject *__restrict index, int defl) const {
	return inherit(DeeObject_CallAttrStringf(*this, "asdigit", "od", index, defl));
}
WUNUSED deemon::int_ string::asdecimal() const {
	return inherit(DeeObject_CallAttrString(*this, "asdecimal", 0, NULL));
}
WUNUSED deemon::int_ string::asdecimal(size_t index) const {
	return inherit(DeeObject_CallAttrStringf(*this, "asdecimal", "Iu", index));
}
WUNUSED deemon::int_ string::asdecimal(size_t index, int defl) const {
	return inherit(DeeObject_CallAttrStringf(*this, "asdecimal", "Iud", index, defl));
}
WUNUSED deemon::int_ string::asdecimal(DeeObject *__restrict index) const {
	return inherit(DeeObject_CallAttrStringf(*this, "asdecimal", "o", index));
}
WUNUSED deemon::int_ string::asdecimal(DeeObject *__restrict index, int defl) const {
	return inherit(DeeObject_CallAttrStringf(*this, "asdecimal", "od", index, defl));
}
#define DEFINE_FIND_FUNCTION(Treturn, name, needle)                                                          \
	WUNUSED Treturn(string::name)(/*utf-8*/ char const *__restrict needle) const {                           \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "s", needle));                                \
	}                                                                                                        \
	WUNUSED Treturn(string::name)(/*utf-8*/ char const *__restrict needle, size_t start) const {             \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "sIu", needle, start));                       \
	}                                                                                                        \
	WUNUSED Treturn(string::name)(/*utf-8*/ char const *__restrict needle, size_t start, size_t end) const { \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "sIuIu", needle, start, end));                \
	}                                                                                                        \
	WUNUSED Treturn(string::name)(DeeObject * __restrict needle) const {                                     \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "o", needle));                                \
	}                                                                                                        \
	WUNUSED Treturn(string::name)(DeeObject * __restrict needle, size_t start) const {                       \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "oIu", needle, start));                       \
	}                                                                                                        \
	WUNUSED Treturn(string::name)(DeeObject * __restrict needle, size_t start, size_t end) const {           \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "oIuIu", needle, start, end));                \
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
#define DEFINE_COMPARE_FUNCTION(Treturn, name)                                                                                                \
	WUNUSED Treturn(string::name)(/*utf-8*/ char const *__restrict other) const {                                                             \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "s", other));                                                                  \
	}                                                                                                                                         \
	WUNUSED Treturn(string::name)(/*utf-8*/ char const *__restrict other, size_t other_size) const {                                          \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "$s", other_size, other));                                                     \
	}                                                                                                                                         \
	WUNUSED Treturn(string::name)(size_t my_start, /*utf-8*/ char const *__restrict other) const {                                            \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "Ius", my_start, other));                                                      \
	}                                                                                                                                         \
	WUNUSED Treturn(string::name)(size_t my_start, /*utf-8*/ char const *__restrict other, size_t other_size) const {                         \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "Iu$s", my_start, other_size, other));                                         \
	}                                                                                                                                         \
	WUNUSED Treturn(string::name)(size_t my_start, size_t my_end, /*utf-8*/ char const *__restrict other) const {                             \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "IuIus", my_start, my_end, other));                                            \
	}                                                                                                                                         \
	WUNUSED Treturn(string::name)(size_t my_start, size_t my_end, /*utf-8*/ char const *__restrict other, size_t other_size) const {          \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "IuIu$s", my_start, my_end, other_size, other));                               \
	}                                                                                                                                         \
	WUNUSED Treturn(string::name)(DeeObject * __restrict other) const {                                                                       \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "o", other));                                                                  \
	}                                                                                                                                         \
	WUNUSED Treturn(string::name)(DeeObject * __restrict other, size_t other_start) const {                                                   \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "oIu", other, other_start));                                                   \
	}                                                                                                                                         \
	WUNUSED Treturn(string::name)(DeeObject * __restrict other, size_t other_start, size_t other_end) const {                                 \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "oIuIu", other, other_start, other_end));                                      \
	}                                                                                                                                         \
	WUNUSED Treturn(string::name)(size_t my_start, DeeObject * __restrict other) const {                                                      \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "Iuo", my_start, other));                                                      \
	}                                                                                                                                         \
	WUNUSED Treturn(string::name)(size_t my_start, DeeObject * __restrict other, size_t other_start) const {                                  \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "IuoIu", my_start, other, other_start));                                       \
	}                                                                                                                                         \
	WUNUSED Treturn(string::name)(size_t my_start, DeeObject * __restrict other, size_t other_start, size_t other_end) const {                \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "IuoIuIu", my_start, other, other_start, other_end));                          \
	}                                                                                                                                         \
	WUNUSED Treturn(string::name)(size_t my_start, size_t my_end, DeeObject * __restrict other) const {                                       \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "IuIuo", my_start, my_end, other));                                            \
	}                                                                                                                                         \
	WUNUSED Treturn(string::name)(size_t my_start, size_t my_end, DeeObject * __restrict other, size_t other_start) const {                   \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "IuIuoIu", my_start, my_end, other, other_start));                             \
	}                                                                                                                                         \
	WUNUSED Treturn(string::name)(size_t my_start, size_t my_end, DeeObject * __restrict other, size_t other_start, size_t other_end) const { \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "IuIuoIuIu", my_start, my_end, other, other_start, other_end));                \
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
#define DEFINE_FINDMATCH_FUNCTION(Treturn, name)                                                                                                   \
	WUNUSED Treturn(string::name)(DeeObject * __restrict open, DeeObject * __restrict close) const {                                               \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "oo", open, close));                                                                \
	}                                                                                                                                              \
	WUNUSED Treturn(string::name)(DeeObject * __restrict open, DeeObject * __restrict close, size_t start) const {                                 \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "ooIu", open, close, start));                                                       \
	}                                                                                                                                              \
	WUNUSED Treturn(string::name)(DeeObject * __restrict open, DeeObject * __restrict close, size_t start, size_t end) const {                     \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "ooIuIu", open, close, start, end));                                                \
	}                                                                                                                                              \
	WUNUSED Treturn(string::name)(DeeObject * __restrict open, /*utf-8*/ char const *__restrict close) const {                                     \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "os", open, close));                                                                \
	}                                                                                                                                              \
	WUNUSED Treturn(string::name)(DeeObject * __restrict open, /*utf-8*/ char const *__restrict close, size_t start) const {                       \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "osIu", open, close, start));                                                       \
	}                                                                                                                                              \
	WUNUSED Treturn(string::name)(DeeObject * __restrict open, /*utf-8*/ char const *__restrict close, size_t start, size_t end) const {           \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "osIuIu", open, close, start, end));                                                \
	}                                                                                                                                              \
	WUNUSED Treturn(string::name)(/*utf-8*/ char const *__restrict open, DeeObject *__restrict close) const {                                      \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "so", open, close));                                                                \
	}                                                                                                                                              \
	WUNUSED Treturn(string::name)(/*utf-8*/ char const *__restrict open, DeeObject *__restrict close, size_t start) const {                        \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "soIu", open, close, start));                                                       \
	}                                                                                                                                              \
	WUNUSED Treturn(string::name)(/*utf-8*/ char const *__restrict open, DeeObject *__restrict close, size_t start, size_t end) const {            \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "soIuIu", open, close, start, end));                                                \
	}                                                                                                                                              \
	WUNUSED Treturn(string::name)(/*utf-8*/ char const *__restrict open, /*utf-8*/ char const *__restrict close) const {                           \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "ss", open, close));                                                                \
	}                                                                                                                                              \
	WUNUSED Treturn(string::name)(/*utf-8*/ char const *__restrict open, /*utf-8*/ char const *__restrict close, size_t start) const {             \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "ssIu", open, close, start));                                                       \
	}                                                                                                                                              \
	WUNUSED Treturn(string::name)(/*utf-8*/ char const *__restrict open, /*utf-8*/ char const *__restrict close, size_t start, size_t end) const { \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "ssIuIu", open, close, start, end));                                                \
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
#define DEFINE_RE_FUNCTION(Treturn, name)                                                                                                             \
	WUNUSED Treturn(string::name)(DeeObject * __restrict pattern) const {                                                                             \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "o", pattern));                                                                        \
	}                                                                                                                                                 \
	WUNUSED Treturn(string::name)(DeeObject * __restrict pattern, DeeObject * __restrict rules) const {                                               \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "oo", pattern, rules));                                                                \
	}                                                                                                                                                 \
	WUNUSED Treturn(string::name)(DeeObject * __restrict pattern, /*utf-8*/ char const *__restrict rules) const {                                     \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "os", pattern, rules));                                                                \
	}                                                                                                                                                 \
	WUNUSED Treturn(string::name)(DeeObject * __restrict pattern, size_t start) const {                                                               \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "oIu", pattern, start));                                                               \
	}                                                                                                                                                 \
	WUNUSED Treturn(string::name)(DeeObject * __restrict pattern, DeeObject * __restrict rules, size_t start) const {                                 \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "ooIu", pattern, rules, start));                                                       \
	}                                                                                                                                                 \
	WUNUSED Treturn(string::name)(DeeObject * __restrict pattern, /*utf-8*/ char const *__restrict rules, size_t start) const {                       \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "osIu", pattern, rules, start));                                                       \
	}                                                                                                                                                 \
	WUNUSED Treturn(string::name)(DeeObject * __restrict pattern, size_t start, size_t end) const {                                                   \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "oIuIu", pattern, start, end));                                                        \
	}                                                                                                                                                 \
	WUNUSED Treturn(string::name)(DeeObject * __restrict pattern, DeeObject * __restrict rules, size_t start, size_t end) const {                     \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "ooIuIu", pattern, rules, start, end));                                                \
	}                                                                                                                                                 \
	WUNUSED Treturn(string::name)(DeeObject * __restrict pattern, /*utf-8*/ char const *__restrict rules, size_t start, size_t end) const {           \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "osIuIu", pattern, rules, start, end));                                                \
	}                                                                                                                                                 \
	WUNUSED Treturn(string::name)(DeeObject * __restrict pattern, size_t start, DeeObject * __restrict rules) const {                                 \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "oIuo", pattern, start, rules));                                                       \
	}                                                                                                                                                 \
	WUNUSED Treturn(string::name)(DeeObject * __restrict pattern, size_t start, size_t end, DeeObject * __restrict rules) const {                     \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "oIuIuo", pattern, start, end, rules));                                                \
	}                                                                                                                                                 \
	WUNUSED Treturn(string::name)(DeeObject * __restrict pattern, size_t start, /*utf-8*/ char const *__restrict rules) const {                       \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "oIus", pattern, start, rules));                                                       \
	}                                                                                                                                                 \
	WUNUSED Treturn(string::name)(DeeObject * __restrict pattern, size_t start, size_t end, /*utf-8*/ char const *__restrict rules) const {           \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "oIuIus", pattern, start, end, rules));                                                \
	}                                                                                                                                                 \
	WUNUSED Treturn(string::name)(/*utf-8*/ char const *__restrict pattern) const {                                                                   \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "s", pattern));                                                                        \
	}                                                                                                                                                 \
	WUNUSED Treturn(string::name)(/*utf-8*/ char const *__restrict pattern, DeeObject *__restrict rules) const {                                      \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "so", pattern, rules));                                                                \
	}                                                                                                                                                 \
	WUNUSED Treturn(string::name)(/*utf-8*/ char const *__restrict pattern, /*utf-8*/ char const *__restrict rules) const {                           \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "ss", pattern, rules));                                                                \
	}                                                                                                                                                 \
	WUNUSED Treturn(string::name)(/*utf-8*/ char const *__restrict pattern, size_t start) const {                                                     \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "sIu", pattern, start));                                                               \
	}                                                                                                                                                 \
	WUNUSED Treturn(string::name)(/*utf-8*/ char const *__restrict pattern, DeeObject *__restrict rules, size_t start) const {                        \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "soIu", pattern, rules, start));                                                       \
	}                                                                                                                                                 \
	WUNUSED Treturn(string::name)(/*utf-8*/ char const *__restrict pattern, /*utf-8*/ char const *__restrict rules, size_t start) const {             \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "ssIu", pattern, rules, start));                                                       \
	}                                                                                                                                                 \
	WUNUSED Treturn(string::name)(/*utf-8*/ char const *__restrict pattern, size_t start, size_t end) const {                                         \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "sIuIu", pattern, start, end));                                                        \
	}                                                                                                                                                 \
	WUNUSED Treturn(string::name)(/*utf-8*/ char const *__restrict pattern, DeeObject *__restrict rules, size_t start, size_t end) const {            \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "soIuIu", pattern, rules, start, end));                                                \
	}                                                                                                                                                 \
	WUNUSED Treturn(string::name)(/*utf-8*/ char const *__restrict pattern, /*utf-8*/ char const *__restrict rules, size_t start, size_t end) const { \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "ssIuIu", pattern, rules, start, end));                                                \
	}                                                                                                                                                 \
	WUNUSED Treturn(string::name)(/*utf-8*/ char const *__restrict pattern, size_t start, DeeObject *__restrict rules) const {                        \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "sIuo", pattern, start, rules));                                                       \
	}                                                                                                                                                 \
	WUNUSED Treturn(string::name)(/*utf-8*/ char const *__restrict pattern, size_t start, size_t end, DeeObject *__restrict rules) const {            \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "sIuIuo", pattern, start, end, rules));                                                \
	}                                                                                                                                                 \
	WUNUSED Treturn(string::name)(/*utf-8*/ char const *__restrict pattern, size_t start, /*utf-8*/ char const *__restrict rules) const {             \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "sIus", pattern, start, rules));                                                       \
	}                                                                                                                                                 \
	WUNUSED Treturn(string::name)(/*utf-8*/ char const *__restrict pattern, size_t start, size_t end, /*utf-8*/ char const *__restrict rules) const { \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "sIuIus", pattern, start, end, rules));                                                \
	}
DEFINE_RE_FUNCTION(deemon::int_, rematch)
DEFINE_RE_FUNCTION(Sequence<deemon::int_>, refind)
DEFINE_RE_FUNCTION(Sequence<deemon::int_>, rerfind)
DEFINE_RE_FUNCTION(Sequence<deemon::int_>, reindex)
DEFINE_RE_FUNCTION(Sequence<deemon::int_>, rerindex)
DEFINE_RE_FUNCTION(Sequence<Sequence<deemon::int_> >, refindall)
DEFINE_RE_FUNCTION(deemon::int_, recount)
#undef DEFINE_RE_FUNCTION
#endif /* GUARD_DEEMON_CXX_INT_H */

#ifdef GUARD_DEEMON_CXX_FILE_H
inline string(File::filename)() const {
	return inherit(DeeFile_Filename(*this));
}
inline string(File::readline)(size_t max_length, bool keep_lf) const {
	return inherit(DeeFile_ReadLine(*this, max_length, keep_lf));
}
inline string(File::read)(size_t max_length, bool readall) const {
	return inherit(DeeFile_ReadText(*this, max_length, readall));
}
inline string(File::pread)(dpos_t pos, size_t max_length, bool readall) const {
	return inherit(DeeFile_PReadText(*this, max_length, pos, readall));
}
inline deemon::string(File::Writer::string)() const {
	return inherit(DeeObject_InstanceOfExact(this->ptr(),
	                                         (DeeTypeObject *)&DeeFileWriter_Type)
	               ? DeeFileWriter_GetString(*this)
	               : DeeObject_GetAttrString(*this, "string"));
}
#endif /* GUARD_DEEMON_CXX_FILE_H */


inline string Object::str() const {
	return inherit(DeeObject_Str(*this));
}
inline string Object::repr() const {
	return inherit(DeeObject_Repr(*this));
}
inline string str(DeeObject *__restrict ob) {
	return inherit(DeeObject_Str(ob));
}
inline string repr(DeeObject *__restrict ob) {
	return inherit(DeeObject_Repr(ob));
}

DEE_CXX_END

#endif /* !GUARD_DEEMON_CXX_STRING_H */
