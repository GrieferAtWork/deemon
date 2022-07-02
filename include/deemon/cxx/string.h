/* Copyright (c) 2018-2022 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2022 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_CXX_STRING_H
#define GUARD_DEEMON_CXX_STRING_H 1

#include "api.h"
/**/

#include <initializer_list>

#include "../format.h" /* DEE_PCK* */
#include "../int.h"
#include "../string.h"
#include "../stringutils.h"
#include "../system-features.h" /* strlen(), wcslen() */
#include "object.h"
#include "sequence.h"
#include "tuple.h"

DEE_CXX_BEGIN

namespace detail {
#ifdef __native_wchar_t_defined
#ifndef _dee_wcslen
#ifdef CONFIG_HAVE_wcslen
#define _dee_wcslen(str) ::wcslen(str)
#else /* CONFIG_HAVE_wcslen */
#define _dee_wcslen(str) (::deemon::detail::_dee_wcslen)(str)
DeeSystem_DEFINE_wcslen(_dee_wcslen)
#endif /* !CONFIG_HAVE_wcslen */
#endif /* !_dee_wcslen */
#endif /* __native_wchar_t_defined */
#ifndef _dee_c16len
DeeSystem_DEFINE_XSTRLEN(_dee_c16len, uint16_t)
#define _dee_c16len(str) _dee_c16len(str)
#endif /* !_dee_c16len */
#ifndef _dee_c32len
DeeSystem_DEFINE_XSTRLEN(_dee_c32len, uint32_t)
#define _dee_c32len(str) _dee_c32len(str)
#endif /* !_dee_c32len */
}

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
	string() DEE_CXX_NOTHROW
	    : Sequence<string>(nonnull(Dee_EmptyString)) {}
	string(/*utf-8*/ char const *__restrict utf8_str)
	    : Sequence<string>(inherit(DeeString_NewUtf8(utf8_str, strlen(utf8_str), STRING_ERROR_FSTRICT))) {}
	string(/*utf-8*/ char const *__restrict utf8_str, size_t utf8_len, unsigned int error_mode = STRING_ERROR_FSTRICT)
	    : Sequence<string>(inherit(DeeString_NewUtf8(utf8_str, utf8_len, error_mode))) {}
	string(/*utf-16*/ uint16_t const *__restrict utf16_str)
	    : Sequence<string>(inherit(DeeString_NewUtf16(utf16_str, detail::_dee_c16len(utf16_str), STRING_ERROR_FSTRICT))) {}
	string(/*utf-16*/ uint16_t const *__restrict utf16_str, size_t utf16_len, unsigned int error_mode = STRING_ERROR_FSTRICT)
	    : Sequence<string>(inherit(DeeString_NewUtf16(utf16_str, utf16_len, error_mode))) {}
	string(/*utf-32*/ uint32_t const *__restrict utf32_str)
	    : Sequence<string>(inherit(DeeString_NewUtf32(utf32_str, detail::_dee_c32len(utf32_str), STRING_ERROR_FSTRICT))) {}
	string(/*utf-32*/ uint32_t const *__restrict utf32_str, size_t utf32_len, unsigned int error_mode = STRING_ERROR_FSTRICT)
	    : Sequence<string>(inherit(DeeString_NewUtf32(utf32_str, utf32_len, error_mode))) {}
#ifdef __native_char16_t_defined
	string(/*utf-16*/ char16_t const *__restrict utf16_str)
	    : Sequence<string>(inherit(DeeString_NewUtf16((uint16_t const *)utf16_str, detail::_dee_c16len((uint16_t const *)utf16_str), STRING_ERROR_FSTRICT))) {}
	string(/*utf-16*/ char16_t const *__restrict utf16_str, size_t utf16_len, unsigned int error_mode = STRING_ERROR_FSTRICT)
	    : Sequence<string>(inherit(DeeString_NewUtf16((uint16_t const *)utf16_str, utf16_len, error_mode))) {}
	string(/*utf-32*/ char32_t const *__restrict utf32_str)
	    : Sequence<string>(inherit(DeeString_NewUtf32((uint32_t const *)utf32_str, detail::_dee_c32len((uint32_t const *)utf32_str), STRING_ERROR_FSTRICT))) {}
	string(/*utf-32*/ char32_t const *__restrict utf32_str, size_t utf32_len, unsigned int error_mode = STRING_ERROR_FSTRICT)
	    : Sequence<string>(inherit(DeeString_NewUtf32((uint32_t const *)utf32_str, utf32_len, error_mode))) {}
#endif /* __native_char16_t_defined */
#ifdef __native_wchar_t_defined
	string(/*wide*/ wchar_t const *__restrict wide_str)
	    : Sequence<string>(inherit(DeeString_NewWide(wide_str, _dee_wcslen(wide_str), STRING_ERROR_FSTRICT))) {}
	string(/*wide*/ wchar_t const *__restrict wide_str, size_t wide_len, unsigned int error_mode = STRING_ERROR_FSTRICT)
	    : Sequence<string>(inherit(DeeString_NewWide(wide_str, wide_len, error_mode))) {}
#endif /* __native_wchar_t_defined */
	static WUNUSED string vcformat(/*utf-8*/ char const *__restrict format, va_list args) {
		return inherit(DeeString_VNewf(format, args));
	}
	static WUNUSED string cformat(/*utf-8*/ char const *__restrict format, ...) {
		va_list args;
		DREF DeeObject *result;
		va_start(args, format);
		result = DeeString_VNewf(format, args);
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
		return DeeString_Hash(this->ptr());
	}
#endif /* CONFIG_BUILDING_DEEMON */
	WUNUSED Dee_hash_t hashcase() const DEE_CXX_NOTHROW {
		return DeeString_HashCase(this->ptr());
	}
	WUNUSED ATTR_RETNONNULL uint8_t *cbytes(bool allow_invalid = false) const {
		return (uint8_t *)throw_if_null(DeeString_AsBytes(this->ptr(), allow_invalid));
	}
	inline WUNUSED Bytes bytes() const;
	inline WUNUSED Bytes bytes(bool allow_invalid) const;
	inline WUNUSED Bytes bytes(size_t start, size_t end) const;
	inline WUNUSED Bytes bytes(size_t start, size_t end, bool allow_invalid) const;
	WUNUSED ATTR_RETNONNULL char *asutf8() const {
		return (char *)throw_if_null((void *)DeeString_AsUtf8(this->ptr()));
	}
	WUNUSED ATTR_RETNONNULL uint16_t *asutf16(unsigned int error_mode = STRING_ERROR_FSTRICT) const {
		return (uint16_t *)throw_if_null((void *)DeeString_AsUtf16(this->ptr(), error_mode));
	}
	WUNUSED ATTR_RETNONNULL uint32_t *asutf32() const {
		return (uint32_t *)throw_if_null((void *)DeeString_AsUtf32(this->ptr()));
	}
	WUNUSED ATTR_RETNONNULL Dee_wchar_t *aswide() const {
		return (Dee_wchar_t *)throw_if_null((void *)DeeString_AsWide(this->ptr()));
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
#ifdef __native_char16_t_defined
	explicit ATTR_RETNONNULL operator char16_t *() const {
		return (char16_t *)asutf16();
	}
	explicit ATTR_RETNONNULL operator char32_t *() const {
		return (char32_t *)asutf32();
	}
#endif /* __native_char16_t_defined */
#ifdef __native_wchar_t_defined
	explicit ATTR_RETNONNULL operator wchar_t *() const {
		return aswide();
	}
#endif /* __native_wchar_t_defined */
	WUNUSED Object decode(/*utf-8*/ char const *__restrict codec_name) const {
		return inherit(DeeObject_CallAttrStringf(this->ptr(), "decode", "s", codec_name));
	}
	WUNUSED Object decode(obj_string codec_name) const {
		return inherit(DeeObject_CallAttrStringf(this->ptr(), "decode", "o", (DeeObject *)codec_name));
	}
	WUNUSED Object decode(/*utf-8*/ char const *__restrict codec_name, /*utf-8*/ char const *__restrict errors) const {
		return inherit(DeeObject_CallAttrStringf(this->ptr(), "decode", "ss", codec_name, errors));
	}
	WUNUSED Object decode(/*utf-8*/ char const *__restrict codec_name, obj_string errors) const {
		return inherit(DeeObject_CallAttrStringf(this->ptr(), "decode", "so", codec_name, (DeeObject *)errors));
	}
	WUNUSED Object decode(obj_string codec_name, /*utf-8*/ char const *__restrict errors) const {
		return inherit(DeeObject_CallAttrStringf(this->ptr(), "decode", "os", (DeeObject *)codec_name, errors));
	}
	WUNUSED Object decode(obj_string codec_name, obj_string errors) const {
		return inherit(DeeObject_CallAttrStringf(this->ptr(), "decode", "oo", (DeeObject *)codec_name, (DeeObject *)errors));
	}
	inline WUNUSED Bytes encode(/*utf-8*/ char const *__restrict codec_name) const;
	inline WUNUSED Bytes encode(obj_string codec_name) const;
	inline WUNUSED Bytes encode(/*utf-8*/ char const *__restrict codec_name, /*utf-8*/ char const *__restrict errors) const;
	inline WUNUSED Bytes encode(/*utf-8*/ char const *__restrict codec_name, obj_string errors) const;
	inline WUNUSED Bytes encode(obj_string codec_name, /*utf-8*/ char const *__restrict errors) const;
	inline WUNUSED Bytes encode(obj_string codec_name, obj_string errors) const;
	WUNUSED Object encodeob(/*utf-8*/ char const *__restrict codec_name) const {
		return inherit(DeeObject_CallAttrStringf(this->ptr(), "encode", "s", codec_name));
	}
	WUNUSED Object encodeob(obj_string codec_name) const {
		return inherit(DeeObject_CallAttrStringf(this->ptr(), "encode", "o", (DeeObject *)codec_name));
	}
	WUNUSED Object encodeob(/*utf-8*/ char const *__restrict codec_name, /*utf-8*/ char const *__restrict errors) const {
		return inherit(DeeObject_CallAttrStringf(this->ptr(), "encode", "ss", codec_name, errors));
	}
	WUNUSED Object encodeob(/*utf-8*/ char const *__restrict codec_name, obj_string errors) const {
		return inherit(DeeObject_CallAttrStringf(this->ptr(), "encode", "so", codec_name, (DeeObject *)errors));
	}
	WUNUSED Object encodeob(obj_string codec_name, /*utf-8*/ char const *__restrict errors) const {
		return inherit(DeeObject_CallAttrStringf(this->ptr(), "encode", "os", (DeeObject *)codec_name, errors));
	}
	WUNUSED Object encodeob(obj_string codec_name, obj_string errors) const {
		return inherit(DeeObject_CallAttrStringf(this->ptr(), "encode", "oo", (DeeObject *)codec_name, (DeeObject *)errors));
	}

	WUNUSED string format(obj_sequence args) const {
		return inherit(DeeObject_CallAttrStringf(this->ptr(), "format", "o", (DeeObject *)args));
	}
	WUNUSED string format(std::initializer_list<DeeObject *> const &args) const {
		Tuple<Object> ob_args(args);
		return inherit(DeeObject_CallAttrStringf(this->ptr(), "format", "o", (DeeObject *)ob_args));
	}
	WUNUSED Sequence scanf(/*utf-8*/ char const *__restrict format) const {
		return inherit(DeeObject_CallAttrStringf(this->ptr(), "scanf", "s", format));
	}
	WUNUSED Sequence scanf(obj_string format) const {
		return inherit(DeeObject_CallAttrStringf(this->ptr(), "scanf", "o", (DeeObject *)format));
	}
#define DEFINE_CHARACTER_TRAIT(name)                                                                                        \
	WUNUSED bool(name)() const {                                                                                            \
		return Object(inherit(DeeObject_CallAttrString(this->ptr(), #name, 0, NULL))).bool_();                              \
	}                                                                                                                       \
	WUNUSED bool(name)(size_t index) const {                                                                                \
		return Object(inherit(DeeObject_CallAttrStringf(this->ptr(), #name, DEE_PCKuSIZ, index))).bool_();                  \
	}                                                                                                                       \
	WUNUSED bool(name)(size_t start, size_t end) const {                                                                    \
		return Object(inherit(DeeObject_CallAttrStringf(this->ptr(), #name, DEE_PCKuSIZ DEE_PCKuSIZ, start, end))).bool_(); \
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
#define DEFINE_CHARACTER_TRAIT(name)                                                                                        \
	WUNUSED bool(name)() const {                                                                                            \
		return Object(inherit(DeeObject_CallAttrString(this->ptr(), #name, 0, NULL))).bool_();                              \
	}                                                                                                                       \
	WUNUSED bool(name)(size_t start) const {                                                                                \
		return Object(inherit(DeeObject_CallAttrStringf(this->ptr(), #name, DEE_PCKuSIZ, start))).bool_();                  \
	}                                                                                                                       \
	WUNUSED bool(name)(size_t start, size_t end) const {                                                                    \
		return Object(inherit(DeeObject_CallAttrStringf(this->ptr(), #name, DEE_PCKuSIZ DEE_PCKuSIZ, start, end))).bool_(); \
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
#define DEFINE_STRING_TRANSFORMATION(name)                                                                  \
	WUNUSED string(name)() const {                                                                          \
		return inherit(DeeObject_CallAttrString(this->ptr(), #name, 0, NULL));                              \
	}                                                                                                       \
	WUNUSED string(name)(size_t start) const {                                                              \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, DEE_PCKuSIZ, start));                  \
	}                                                                                                       \
	WUNUSED string(name)(size_t start, size_t end) const {                                                  \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, DEE_PCKuSIZ DEE_PCKuSIZ, start, end)); \
	}
	DEFINE_STRING_TRANSFORMATION(lower)
	DEFINE_STRING_TRANSFORMATION(upper)
	DEFINE_STRING_TRANSFORMATION(title)
	DEFINE_STRING_TRANSFORMATION(capitalize)
	DEFINE_STRING_TRANSFORMATION(swapcase)
	DEFINE_STRING_TRANSFORMATION(casefold)
	DEFINE_STRING_TRANSFORMATION(reversed)
#undef DEFINE_STRING_TRANSFORMATION
	inline WUNUSED deemon::int_ asnumber() const;
	inline WUNUSED deemon::int_ asnumber(size_t index) const;
	inline WUNUSED deemon::int_ asnumber(size_t index, int defl) const;
	inline WUNUSED deemon::int_ asnumber(DeeObject *index) const;
	inline WUNUSED deemon::int_ asnumber(DeeObject *index, int defl) const;
	inline WUNUSED deemon::int_ asdigit() const;
	inline WUNUSED deemon::int_ asdigit(size_t index) const;
	inline WUNUSED deemon::int_ asdigit(size_t index, int defl) const;
	inline WUNUSED deemon::int_ asdigit(DeeObject *index) const;
	inline WUNUSED deemon::int_ asdigit(DeeObject *index, int defl) const;
	inline WUNUSED deemon::int_ asdecimal() const;
	inline WUNUSED deemon::int_ asdecimal(size_t index) const;
	inline WUNUSED deemon::int_ asdecimal(size_t index, int defl) const;
	inline WUNUSED deemon::int_ asdecimal(DeeObject *index) const;
	inline WUNUSED deemon::int_ asdecimal(DeeObject *index, int defl) const;
	WUNUSED Object asnumber(size_t index, DeeObject *defl) const {
		return inherit(DeeObject_CallAttrStringf(this->ptr(), "asnumber", DEE_PCKuSIZ "o", index, defl));
	}
	WUNUSED Object asnumber(DeeObject *index, DeeObject *defl) const {
		return inherit(DeeObject_CallAttrStringf(this->ptr(), "asnumber", "oo", index, defl));
	}
	WUNUSED Object asdigit(size_t index, DeeObject *defl) const {
		return inherit(DeeObject_CallAttrStringf(this->ptr(), "asdigit", DEE_PCKuSIZ "o", index, defl));
	}
	WUNUSED Object asdigit(DeeObject *index, DeeObject *defl) const {
		return inherit(DeeObject_CallAttrStringf(this->ptr(), "asdigit", "oo", index, defl));
	}
	WUNUSED Object asdecimal(size_t index, DeeObject *defl) const {
		return inherit(DeeObject_CallAttrStringf(this->ptr(), "asdecimal", DEE_PCKuSIZ "o", index, defl));
	}
	WUNUSED Object asdecimal(DeeObject *index, DeeObject *defl) const {
		return inherit(DeeObject_CallAttrStringf(this->ptr(), "asdecimal", "oo", index, defl));
	}
#define DEFINE_REPLACE_FUNCTION(name)                                                                                           \
	WUNUSED string(name)(DeeObject * find, DeeObject * repl) const {                                                            \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, "oo", find, repl));                                        \
	}                                                                                                                           \
	WUNUSED string(name)(DeeObject * find, DeeObject * repl, size_t maxcount) const {                                           \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, "oo" DEE_PCKuSIZ, find, repl, maxcount));                  \
	}                                                                                                                           \
	WUNUSED string(name)(DeeObject * find, /*utf-8*/ char const *__restrict repl) const {                                       \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, "os", find, repl));                                        \
	}                                                                                                                           \
	WUNUSED string(name)(DeeObject * find, /*utf-8*/ char const *__restrict repl, size_t maxcount) const {                      \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, "os" DEE_PCKuSIZ, find, repl, maxcount));                  \
	}                                                                                                                           \
	WUNUSED string(name)(/*utf-8*/ char const *__restrict find, DeeObject *repl) const {                                        \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, "so", find, repl));                                        \
	}                                                                                                                           \
	WUNUSED string(name)(/*utf-8*/ char const *__restrict find, DeeObject *repl, size_t maxcount) const {                       \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, "so" DEE_PCKuSIZ, find, repl, maxcount));                  \
	}                                                                                                                           \
	WUNUSED string(name)(/*utf-8*/ char const *__restrict find, /*utf-8*/ char const *__restrict repl) const {                  \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, "ss", find, repl));                                        \
	}                                                                                                                           \
	WUNUSED string(name)(/*utf-8*/ char const *__restrict find, /*utf-8*/ char const *__restrict repl, size_t maxcount) const { \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, "ss" DEE_PCKuSIZ, find, repl, maxcount));                  \
	}
	DEFINE_REPLACE_FUNCTION(replace)
	DEFINE_REPLACE_FUNCTION(casereplace)
#undef DEFINE_REPLACE_FUNCTION
#define DEFINE_FIND_FUNCTION(Treturn, name, needle)                                                        \
	inline WUNUSED Treturn(name)(/*utf-8*/ char const *__restrict needle) const;                           \
	inline WUNUSED Treturn(name)(/*utf-8*/ char const *__restrict needle, size_t start) const;             \
	inline WUNUSED Treturn(name)(/*utf-8*/ char const *__restrict needle, size_t start, size_t end) const; \
	inline WUNUSED Treturn(name)(DeeObject *needle) const;                                                 \
	inline WUNUSED Treturn(name)(DeeObject *needle, size_t start) const;                                   \
	inline WUNUSED Treturn(name)(DeeObject *needle, size_t start, size_t end) const;
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
#define DEFINE_FIND_FUNCTION(Treturn, name, needle)                                                                     \
	WUNUSED Treturn(name)(/*utf-8*/ char const *__restrict needle) const {                                              \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, "s", needle));                                     \
	}                                                                                                                   \
	WUNUSED Treturn(name)(/*utf-8*/ char const *__restrict needle, size_t start) const {                                \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, "s" DEE_PCKuSIZ, needle, start));                  \
	}                                                                                                                   \
	WUNUSED Treturn(name)(/*utf-8*/ char const *__restrict needle, size_t start, size_t end) const {                    \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, "s" DEE_PCKuSIZ DEE_PCKuSIZ, needle, start, end)); \
	}                                                                                                                   \
	WUNUSED Treturn(name)(DeeObject *needle) const {                                                                    \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, "o", needle));                                     \
	}                                                                                                                   \
	WUNUSED Treturn(name)(DeeObject *needle, size_t start) const {                                                      \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, "o" DEE_PCKuSIZ, needle, start));                  \
	}                                                                                                                   \
	WUNUSED Treturn(name)(DeeObject *needle, size_t start, size_t end) const {                                          \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, "o" DEE_PCKuSIZ DEE_PCKuSIZ, needle, start, end)); \
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
		return inherit(DeeObject_CallAttrStringf(this->ptr(), "substr", DEE_PCKuSIZ, start));
	}
	WUNUSED string substr(size_t start, size_t end) const {
		return inherit(DeeObject_CallAttrStringf(this->ptr(), "substr", DEE_PCKuSIZ DEE_PCKuSIZ, start, end));
	}
#define DEFINE_STRIP_FUNCTION(name)                                                           \
	WUNUSED string(name)() const {                                                            \
		return inherit(DeeObject_CallAttrString(this->ptr(), #name, 0, NULL));                \
	}                                                                                         \
	WUNUSED string(name)(/*utf-8*/ char const *__restrict mask) const {                       \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, "s", mask));             \
	}                                                                                         \
	WUNUSED string(name)(/*utf-8*/ char const *__restrict mask, size_t mask_size) const {     \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, "$s", mask_size, mask)); \
	}                                                                                         \
	WUNUSED string(name)(DeeObject *mask) const {                                             \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, "o", mask));             \
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
#define DEFINE_COMPARE_FUNCTION(Treturn, name)                                                                                     \
	inline WUNUSED Treturn(name)(/*utf-8*/ char const *__restrict other) const;                                                    \
	inline WUNUSED Treturn(name)(/*utf-8*/ char const *__restrict other, size_t other_size) const;                                 \
	inline WUNUSED Treturn(name)(size_t my_start, /*utf-8*/ char const *__restrict other) const;                                   \
	inline WUNUSED Treturn(name)(size_t my_start, /*utf-8*/ char const *__restrict other, size_t other_size) const;                \
	inline WUNUSED Treturn(name)(size_t my_start, size_t my_end, /*utf-8*/ char const *__restrict other) const;                    \
	inline WUNUSED Treturn(name)(size_t my_start, size_t my_end, /*utf-8*/ char const *__restrict other, size_t other_size) const; \
	inline WUNUSED Treturn(name)(DeeObject *other) const;                                                                          \
	inline WUNUSED Treturn(name)(DeeObject *other, size_t other_start) const;                                                      \
	inline WUNUSED Treturn(name)(DeeObject *other, size_t other_start, size_t other_end) const;                                    \
	inline WUNUSED Treturn(name)(size_t my_start, DeeObject *other) const;                                                         \
	inline WUNUSED Treturn(name)(size_t my_start, DeeObject *other, size_t other_start) const;                                     \
	inline WUNUSED Treturn(name)(size_t my_start, DeeObject *other, size_t other_start, size_t other_end) const;                   \
	inline WUNUSED Treturn(name)(size_t my_start, size_t my_end, DeeObject *other) const;                                          \
	inline WUNUSED Treturn(name)(size_t my_start, size_t my_end, DeeObject *other, size_t other_start) const;                      \
	inline WUNUSED Treturn(name)(size_t my_start, size_t my_end, DeeObject *other, size_t other_start, size_t other_end) const;
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
#define DEFINE_COMPARE_FUNCTION(Treturn, name)                                                                                                                               \
	WUNUSED Treturn(name)(/*utf-8*/ char const *__restrict other) const {                                                                                                    \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, "s", other));                                                                                           \
	}                                                                                                                                                                        \
	WUNUSED Treturn(name)(/*utf-8*/ char const *__restrict other, size_t other_size) const {                                                                                 \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, "$s", other_size, other));                                                                              \
	}                                                                                                                                                                        \
	WUNUSED Treturn(name)(size_t my_start, /*utf-8*/ char const *__restrict other) const {                                                                                   \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, DEE_PCKuSIZ "s", my_start, other));                                                                     \
	}                                                                                                                                                                        \
	WUNUSED Treturn(name)(size_t my_start, /*utf-8*/ char const *__restrict other, size_t other_size) const {                                                                \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, DEE_PCKuSIZ "$s", my_start, other_size, other));                                                        \
	}                                                                                                                                                                        \
	WUNUSED Treturn(name)(size_t my_start, size_t my_end, /*utf-8*/ char const *__restrict other) const {                                                                    \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, DEE_PCKuSIZ DEE_PCKuSIZ "s", my_start, my_end, other));                                                 \
	}                                                                                                                                                                        \
	WUNUSED Treturn(name)(size_t my_start, size_t my_end, /*utf-8*/ char const *__restrict other, size_t other_size) const {                                                 \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, DEE_PCKuSIZ DEE_PCKuSIZ "$s", my_start, my_end, other_size, other));                                    \
	}                                                                                                                                                                        \
	WUNUSED Treturn(name)(DeeObject *other) const {                                                                                                                          \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, "o", other));                                                                                           \
	}                                                                                                                                                                        \
	WUNUSED Treturn(name)(DeeObject *other, size_t other_start) const {                                                                                                      \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, "o" DEE_PCKuSIZ, other, other_start));                                                                  \
	}                                                                                                                                                                        \
	WUNUSED Treturn(name)(DeeObject *other, size_t other_start, size_t other_end) const {                                                                                    \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, "o" DEE_PCKuSIZ DEE_PCKuSIZ, other, other_start, other_end));                                           \
	}                                                                                                                                                                        \
	WUNUSED Treturn(name)(size_t my_start, DeeObject *other) const {                                                                                                         \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, DEE_PCKuSIZ "o", my_start, other));                                                                     \
	}                                                                                                                                                                        \
	WUNUSED Treturn(name)(size_t my_start, DeeObject *other, size_t other_start) const {                                                                                     \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, DEE_PCKuSIZ "o" DEE_PCKuSIZ, my_start, other, other_start));                                            \
	}                                                                                                                                                                        \
	WUNUSED Treturn(name)(size_t my_start, DeeObject *other, size_t other_start, size_t other_end) const {                                                                   \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, DEE_PCKuSIZ "o" DEE_PCKuSIZ DEE_PCKuSIZ, my_start, other, other_start, other_end));                     \
	}                                                                                                                                                                        \
	WUNUSED Treturn(name)(size_t my_start, size_t my_end, DeeObject *other) const {                                                                                          \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, DEE_PCKuSIZ DEE_PCKuSIZ "o", my_start, my_end, other));                                                 \
	}                                                                                                                                                                        \
	WUNUSED Treturn(name)(size_t my_start, size_t my_end, DeeObject *other, size_t other_start) const {                                                                      \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, DEE_PCKuSIZ DEE_PCKuSIZ "o" DEE_PCKuSIZ, my_start, my_end, other, other_start));                        \
	}                                                                                                                                                                        \
	WUNUSED Treturn(name)(size_t my_start, size_t my_end, DeeObject *other, size_t other_start, size_t other_end) const {                                                    \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, DEE_PCKuSIZ DEE_PCKuSIZ "o" DEE_PCKuSIZ DEE_PCKuSIZ, my_start, my_end, other, other_start, other_end)); \
	}
	DEFINE_COMPARE_FUNCTION(deemon::bool_, wmatch)
	DEFINE_COMPARE_FUNCTION(deemon::bool_, casewmatch)
#undef DEFINE_COMPARE_FUNCTION
#define DEFINE_CENTER_FUNCTION(name)                                                                                 \
	WUNUSED string(name)(size_t width) const {                                                                       \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, DEE_PCKuSIZ, width));                           \
	}                                                                                                                \
	WUNUSED string(name)(size_t width, DeeObject *filler) const {                                                    \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, DEE_PCKuSIZ "o", width, filler));               \
	}                                                                                                                \
	WUNUSED string(name)(size_t width, /*utf-8*/ char const *__restrict filler) const {                              \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, DEE_PCKuSIZ "s", width, filler));               \
	}                                                                                                                \
	WUNUSED string(name)(size_t width, /*utf-8*/ char const *__restrict filler, size_t filler_size) const {          \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, DEE_PCKuSIZ "$s", width, filler_size, filler)); \
	}
	DEFINE_CENTER_FUNCTION(center)
	DEFINE_CENTER_FUNCTION(ljust)
	DEFINE_CENTER_FUNCTION(rjust)
	DEFINE_CENTER_FUNCTION(zfill)
#undef DEFINE_CENTER_FUNCTION
	WUNUSED string expandtabs() const {
		return inherit(DeeObject_CallAttrString(this->ptr(), "expandtabs", 0, NULL));
	}
	WUNUSED string expandtabs(size_t tab_width) const {
		return inherit(DeeObject_CallAttrStringf(this->ptr(), "expandtabs", DEE_PCKuSIZ, tab_width));
	}
#define DEFINE_UNIFYLINES(name, replacement)                                                                \
	WUNUSED string(name)() const {                                                                          \
		return inherit(DeeObject_CallAttrString(this->ptr(), #name, 0, NULL));                              \
	}                                                                                                       \
	WUNUSED string(name)(DeeObject *replacement) const {                                                    \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, "o", replacement));                    \
	}                                                                                                       \
	WUNUSED string(name)(/*utf-8*/ char const *__restrict replacement) const {                              \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, "s", replacement));                    \
	}                                                                                                       \
	WUNUSED string(name)(/*utf-8*/ char const *__restrict replacement, size_t replacement_size) const {     \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, "$s", replacement_size, replacement)); \
	}
	DEFINE_UNIFYLINES(unifylines, replacement)
	DEFINE_UNIFYLINES(indent, filler)
#undef DEFINE_UNIFYLINES
	WUNUSED string join(DeeObject *seq) const {
		return inherit(DeeObject_CallAttrStringf(this->ptr(), "join", "o", seq));
	}
	WUNUSED string join(std::initializer_list<DeeObject *> const &seq) const {
		Tuple<Object> seq_obj(seq);
		return inherit(DeeObject_CallAttrStringf(this->ptr(), "join", "o", seq_obj.ptr()));
	}
#define DEFINE_SPLIT_FUNCTION(name)                                                               \
	WUNUSED Sequence<string>(name)(DeeObject *sep) const {                                        \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, "o", sep));                  \
	}                                                                                             \
	WUNUSED Sequence<string>(name)(/*utf-8*/ char const *__restrict sep) const {                  \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, "s", sep));                  \
	}                                                                                             \
	WUNUSED Sequence<string>(name)(/*utf-8*/ char const *__restrict sep, size_t sep_size) const { \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, "$s", sep_size, sep));       \
	}
	DEFINE_SPLIT_FUNCTION(split)
	DEFINE_SPLIT_FUNCTION(casesplit)
#undef DEFINE_SPLIT_FUNCTION
	WUNUSED Sequence<string> splitlines() const {
		return inherit(DeeObject_CallAttrString(this->ptr(), "splitlines", 0, NULL));
	}
	WUNUSED Sequence<string> splitlines(bool keepends) const {
		return inherit(DeeObject_CallAttrStringf(this->ptr(), "join", "b", keepends));
	}
	WUNUSED string dedent() const {
		return inherit(DeeObject_CallAttrString(this->ptr(), "dedent", 0, NULL));
	}
	WUNUSED string dedent(size_t max_chars) const {
		return inherit(DeeObject_CallAttrStringf(this->ptr(), "dedent", DEE_PCKuSIZ, max_chars));
	}
	WUNUSED string dedent(size_t max_chars, DeeObject *mask) const {
		return inherit(DeeObject_CallAttrStringf(this->ptr(), "dedent", DEE_PCKuSIZ "o", max_chars, mask));
	}
	WUNUSED string dedent(size_t max_chars, /*utf-8*/ char const *__restrict mask) const {
		return inherit(DeeObject_CallAttrStringf(this->ptr(), "dedent", DEE_PCKuSIZ "s", max_chars, mask));
	}
	WUNUSED string dedent(size_t max_chars, /*utf-8*/ char const *__restrict mask, size_t mask_size) const {
		return inherit(DeeObject_CallAttrStringf(this->ptr(), "dedent", DEE_PCKuSIZ "$s", max_chars, mask_size, mask));
	}
#define DEFINE_FINDMATCH_FUNCTION(Treturn, name)                                                                                     \
	inline WUNUSED Treturn(name)(DeeObject *open, DeeObject *close) const;                                                           \
	inline WUNUSED Treturn(name)(DeeObject *open, DeeObject *close, size_t start) const;                                             \
	inline WUNUSED Treturn(name)(DeeObject *open, DeeObject *close, size_t start, size_t end) const;                                 \
	inline WUNUSED Treturn(name)(DeeObject *open, /*utf-8*/ char const *__restrict close) const;                                     \
	inline WUNUSED Treturn(name)(DeeObject *open, /*utf-8*/ char const *__restrict close, size_t start) const;                       \
	inline WUNUSED Treturn(name)(DeeObject *open, /*utf-8*/ char const *__restrict close, size_t start, size_t end) const;           \
	inline WUNUSED Treturn(name)(/*utf-8*/ char const *__restrict open, DeeObject *close) const;                                     \
	inline WUNUSED Treturn(name)(/*utf-8*/ char const *__restrict open, DeeObject *close, size_t start) const;                       \
	inline WUNUSED Treturn(name)(/*utf-8*/ char const *__restrict open, DeeObject *close, size_t start, size_t end) const;           \
	inline WUNUSED Treturn(name)(/*utf-8*/ char const *__restrict open, /*utf-8*/ char const *__restrict close) const;               \
	inline WUNUSED Treturn(name)(/*utf-8*/ char const *__restrict open, /*utf-8*/ char const *__restrict close, size_t start) const; \
	inline WUNUSED Treturn(name)(/*utf-8*/ char const *__restrict open, /*utf-8*/ char const *__restrict close, size_t start, size_t end) const;
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
	WUNUSED Treturn(name)(DeeObject *open, DeeObject *close) const {                                                                       \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, "oo", open, close));                                                  \
	}                                                                                                                                      \
	WUNUSED Treturn(name)(DeeObject *open, DeeObject *close, size_t start) const {                                                         \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, "oo" DEE_PCKuSIZ, open, close, start));                               \
	}                                                                                                                                      \
	WUNUSED Treturn(name)(DeeObject *open, DeeObject *close, size_t start, size_t end) const {                                             \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, "oo" DEE_PCKuSIZ DEE_PCKuSIZ, open, close, start, end));              \
	}                                                                                                                                      \
	WUNUSED Treturn(name)(DeeObject *open, /*utf-8*/ char const *__restrict close) const {                                                 \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, "os", open, close));                                                  \
	}                                                                                                                                      \
	WUNUSED Treturn(name)(DeeObject *open, /*utf-8*/ char const *__restrict close, size_t start) const {                                   \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, "os" DEE_PCKuSIZ, open, close, start));                               \
	}                                                                                                                                      \
	WUNUSED Treturn(name)(DeeObject *open, /*utf-8*/ char const *__restrict close, size_t start, size_t end) const {                       \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, "os" DEE_PCKuSIZ DEE_PCKuSIZ, open, close, start, end));              \
	}                                                                                                                                      \
	WUNUSED Treturn(name)(/*utf-8*/ char const *__restrict open, DeeObject *close) const {                                                 \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, "so", open, close));                                                  \
	}                                                                                                                                      \
	WUNUSED Treturn(name)(/*utf-8*/ char const *__restrict open, DeeObject *close, size_t start) const {                                   \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, "so" DEE_PCKuSIZ, open, close, start));                               \
	}                                                                                                                                      \
	WUNUSED Treturn(name)(/*utf-8*/ char const *__restrict open, DeeObject *close, size_t start, size_t end) const {                       \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, "so" DEE_PCKuSIZ DEE_PCKuSIZ, open, close, start, end));              \
	}                                                                                                                                      \
	WUNUSED Treturn(name)(/*utf-8*/ char const *__restrict open, /*utf-8*/ char const *__restrict close) const {                           \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, "ss", open, close));                                                  \
	}                                                                                                                                      \
	WUNUSED Treturn(name)(/*utf-8*/ char const *__restrict open, /*utf-8*/ char const *__restrict close, size_t start) const {             \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, "ss" DEE_PCKuSIZ, open, close, start));                               \
	}                                                                                                                                      \
	WUNUSED Treturn(name)(/*utf-8*/ char const *__restrict open, /*utf-8*/ char const *__restrict close, size_t start, size_t end) const { \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, "ss" DEE_PCKuSIZ DEE_PCKuSIZ, open, close, start, end));              \
	}
	DEFINE_FINDMATCH_FUNCTION(Sequence<string>, partitionmatch)
	DEFINE_FINDMATCH_FUNCTION(Sequence<string>, rpartitionmatch)
	DEFINE_FINDMATCH_FUNCTION(Sequence<string>, casepartitionmatch)
	DEFINE_FINDMATCH_FUNCTION(Sequence<string>, caserpartitionmatch)
#undef DEFINE_FINDMATCH_FUNCTION
	WUNUSED Sequence<string> segments(size_t substring_length) const {
		return inherit(DeeObject_CallAttrStringf(this->ptr(), "segments", DEE_PCKuSIZ, substring_length));
	}
	WUNUSED Sequence<string> segments(DeeObject *substring_length) const {
		return inherit(DeeObject_CallAttrStringf(this->ptr(), "segments", "o", substring_length));
	}
	WUNUSED Sequence<string> distribute(size_t substring_count) const {
		return inherit(DeeObject_CallAttrStringf(this->ptr(), "distribute", DEE_PCKuSIZ, substring_count));
	}
	WUNUSED Sequence<string> distribute(DeeObject *substring_count) const {
		return inherit(DeeObject_CallAttrStringf(this->ptr(), "distribute", "o", substring_count));
	}
#define DEFINE_RE_FUNCTION(Treturn, name)                                                                                                           \
	inline WUNUSED Treturn(name)(DeeObject *pattern) const;                                                                                         \
	inline WUNUSED Treturn(name)(DeeObject *pattern, DeeObject *rules) const;                                                                       \
	inline WUNUSED Treturn(name)(DeeObject *pattern, /*utf-8*/ char const *__restrict rules) const;                                                 \
	inline WUNUSED Treturn(name)(DeeObject *pattern, size_t start) const;                                                                           \
	inline WUNUSED Treturn(name)(DeeObject *pattern, DeeObject *rules, size_t start) const;                                                         \
	inline WUNUSED Treturn(name)(DeeObject *pattern, /*utf-8*/ char const *__restrict rules, size_t start) const;                                   \
	inline WUNUSED Treturn(name)(DeeObject *pattern, size_t start, size_t end) const;                                                               \
	inline WUNUSED Treturn(name)(DeeObject *pattern, DeeObject *rules, size_t start, size_t end) const;                                             \
	inline WUNUSED Treturn(name)(DeeObject *pattern, /*utf-8*/ char const *__restrict rules, size_t start, size_t end) const;                       \
	inline WUNUSED Treturn(name)(DeeObject *pattern, size_t start, DeeObject *rules) const;                                                         \
	inline WUNUSED Treturn(name)(DeeObject *pattern, size_t start, size_t end, DeeObject *rules) const;                                             \
	inline WUNUSED Treturn(name)(DeeObject *pattern, size_t start, /*utf-8*/ char const *__restrict rules) const;                                   \
	inline WUNUSED Treturn(name)(DeeObject *pattern, size_t start, size_t end, /*utf-8*/ char const *__restrict rules) const;                       \
	inline WUNUSED Treturn(name)(/*utf-8*/ char const *__restrict pattern) const;                                                                   \
	inline WUNUSED Treturn(name)(/*utf-8*/ char const *__restrict pattern, DeeObject *rules) const;                                                 \
	inline WUNUSED Treturn(name)(/*utf-8*/ char const *__restrict pattern, /*utf-8*/ char const *__restrict rules) const;                           \
	inline WUNUSED Treturn(name)(/*utf-8*/ char const *__restrict pattern, size_t start) const;                                                     \
	inline WUNUSED Treturn(name)(/*utf-8*/ char const *__restrict pattern, DeeObject *rules, size_t start) const;                                   \
	inline WUNUSED Treturn(name)(/*utf-8*/ char const *__restrict pattern, /*utf-8*/ char const *__restrict rules, size_t start) const;             \
	inline WUNUSED Treturn(name)(/*utf-8*/ char const *__restrict pattern, size_t start, size_t end) const;                                         \
	inline WUNUSED Treturn(name)(/*utf-8*/ char const *__restrict pattern, DeeObject *rules, size_t start, size_t end) const;                       \
	inline WUNUSED Treturn(name)(/*utf-8*/ char const *__restrict pattern, /*utf-8*/ char const *__restrict rules, size_t start, size_t end) const; \
	inline WUNUSED Treturn(name)(/*utf-8*/ char const *__restrict pattern, size_t start, DeeObject *rules) const;                                   \
	inline WUNUSED Treturn(name)(/*utf-8*/ char const *__restrict pattern, size_t start, size_t end, DeeObject *rules) const;                       \
	inline WUNUSED Treturn(name)(/*utf-8*/ char const *__restrict pattern, size_t start, /*utf-8*/ char const *__restrict rules) const;             \
	inline WUNUSED Treturn(name)(/*utf-8*/ char const *__restrict pattern, size_t start, size_t end, /*utf-8*/ char const *__restrict rules) const;
	DEFINE_RE_FUNCTION(deemon::int_, rematch)
	DEFINE_RE_FUNCTION(Sequence<deemon::int_>, refind)
	DEFINE_RE_FUNCTION(Sequence<deemon::int_>, rerfind)
	DEFINE_RE_FUNCTION(Sequence<deemon::int_>, reindex)
	DEFINE_RE_FUNCTION(Sequence<deemon::int_>, rerindex)
	DEFINE_RE_FUNCTION(Sequence<Sequence<deemon::int_> >, refindall)
	DEFINE_RE_FUNCTION(deemon::int_, recount)
#undef DEFINE_RE_FUNCTION
#define DEFINE_RE_FUNCTION(Treturn, name)                                                                                                     \
	WUNUSED Treturn(name)(DeeObject *pattern) const {                                                                                         \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, "o", pattern));                                                          \
	}                                                                                                                                         \
	WUNUSED Treturn(name)(DeeObject *pattern, DeeObject *rules) const {                                                                       \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, "oo", pattern, rules));                                                  \
	}                                                                                                                                         \
	WUNUSED Treturn(name)(DeeObject *pattern, /*utf-8*/ char const *__restrict rules) const {                                                 \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, "os", pattern, rules));                                                  \
	}                                                                                                                                         \
	WUNUSED Treturn(name)(DeeObject *pattern, size_t start) const {                                                                           \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, "o" DEE_PCKuSIZ, pattern, start));                                       \
	}                                                                                                                                         \
	WUNUSED Treturn(name)(DeeObject *pattern, DeeObject *rules, size_t start) const {                                                         \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, "oo" DEE_PCKuSIZ, pattern, rules, start));                               \
	}                                                                                                                                         \
	WUNUSED Treturn(name)(DeeObject *pattern, /*utf-8*/ char const *__restrict rules, size_t start) const {                                   \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, "os" DEE_PCKuSIZ, pattern, rules, start));                               \
	}                                                                                                                                         \
	WUNUSED Treturn(name)(DeeObject *pattern, size_t start, size_t end) const {                                                               \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, "o" DEE_PCKuSIZ DEE_PCKuSIZ, pattern, start, end));                      \
	}                                                                                                                                         \
	WUNUSED Treturn(name)(DeeObject *pattern, DeeObject *rules, size_t start, size_t end) const {                                             \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, "oo" DEE_PCKuSIZ DEE_PCKuSIZ, pattern, rules, start, end));              \
	}                                                                                                                                         \
	WUNUSED Treturn(name)(DeeObject *pattern, /*utf-8*/ char const *__restrict rules, size_t start, size_t end) const {                       \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, "os" DEE_PCKuSIZ DEE_PCKuSIZ, pattern, rules, start, end));              \
	}                                                                                                                                         \
	WUNUSED Treturn(name)(DeeObject *pattern, size_t start, DeeObject *rules) const {                                                         \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, "o" DEE_PCKuSIZ "o", pattern, start, rules));                            \
	}                                                                                                                                         \
	WUNUSED Treturn(name)(DeeObject *pattern, size_t start, size_t end, DeeObject *rules) const {                                             \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, "o" DEE_PCKuSIZ DEE_PCKuSIZ "o", pattern, start, end, rules));           \
	}                                                                                                                                         \
	WUNUSED Treturn(name)(DeeObject *pattern, size_t start, /*utf-8*/ char const *__restrict rules) const {                                   \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, "o" DEE_PCKuSIZ "s", pattern, start, rules));                            \
	}                                                                                                                                         \
	WUNUSED Treturn(name)(DeeObject *pattern, size_t start, size_t end, /*utf-8*/ char const *__restrict rules) const {                       \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, "o" DEE_PCKuSIZ DEE_PCKuSIZ "s", pattern, start, end, rules));           \
	}                                                                                                                                         \
	WUNUSED Treturn(name)(/*utf-8*/ char const *__restrict pattern) const {                                                                   \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, "s", pattern));                                                          \
	}                                                                                                                                         \
	WUNUSED Treturn(name)(/*utf-8*/ char const *__restrict pattern, DeeObject *rules) const {                                                 \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, "so", pattern, rules));                                                  \
	}                                                                                                                                         \
	WUNUSED Treturn(name)(/*utf-8*/ char const *__restrict pattern, /*utf-8*/ char const *__restrict rules) const {                           \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, "ss", pattern, rules));                                                  \
	}                                                                                                                                         \
	WUNUSED Treturn(name)(/*utf-8*/ char const *__restrict pattern, size_t start) const {                                                     \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, "s" DEE_PCKuSIZ, pattern, start));                                       \
	}                                                                                                                                         \
	WUNUSED Treturn(name)(/*utf-8*/ char const *__restrict pattern, DeeObject *rules, size_t start) const {                                   \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, "so" DEE_PCKuSIZ, pattern, rules, start));                               \
	}                                                                                                                                         \
	WUNUSED Treturn(name)(/*utf-8*/ char const *__restrict pattern, /*utf-8*/ char const *__restrict rules, size_t start) const {             \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, "ss" DEE_PCKuSIZ, pattern, rules, start));                               \
	}                                                                                                                                         \
	WUNUSED Treturn(name)(/*utf-8*/ char const *__restrict pattern, size_t start, size_t end) const {                                         \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, "s" DEE_PCKuSIZ DEE_PCKuSIZ, pattern, start, end));                      \
	}                                                                                                                                         \
	WUNUSED Treturn(name)(/*utf-8*/ char const *__restrict pattern, DeeObject *rules, size_t start, size_t end) const {                       \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, "so" DEE_PCKuSIZ DEE_PCKuSIZ, pattern, rules, start, end));              \
	}                                                                                                                                         \
	WUNUSED Treturn(name)(/*utf-8*/ char const *__restrict pattern, /*utf-8*/ char const *__restrict rules, size_t start, size_t end) const { \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, "ss" DEE_PCKuSIZ DEE_PCKuSIZ, pattern, rules, start, end));              \
	}                                                                                                                                         \
	WUNUSED Treturn(name)(/*utf-8*/ char const *__restrict pattern, size_t start, DeeObject *rules) const {                                   \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, "s" DEE_PCKuSIZ "o", pattern, start, rules));                            \
	}                                                                                                                                         \
	WUNUSED Treturn(name)(/*utf-8*/ char const *__restrict pattern, size_t start, size_t end, DeeObject *rules) const {                       \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, "s" DEE_PCKuSIZ DEE_PCKuSIZ "o", pattern, start, end, rules));           \
	}                                                                                                                                         \
	WUNUSED Treturn(name)(/*utf-8*/ char const *__restrict pattern, size_t start, /*utf-8*/ char const *__restrict rules) const {             \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, "s" DEE_PCKuSIZ "s", pattern, start, rules));                            \
	}                                                                                                                                         \
	WUNUSED Treturn(name)(/*utf-8*/ char const *__restrict pattern, size_t start, size_t end, /*utf-8*/ char const *__restrict rules) const { \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, "s" DEE_PCKuSIZ DEE_PCKuSIZ "s", pattern, start, end, rules));           \
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
	WUNUSED string rereplace(DeeObject *pattern, DeeObject *repl) const {
		return inherit(DeeObject_CallAttrStringf(this->ptr(), "rereplace", "oo", pattern, repl));
	}
	WUNUSED string rereplace(DeeObject *pattern, DeeObject *repl, size_t maxcount) const {
		return inherit(DeeObject_CallAttrStringf(this->ptr(), "rereplace", "oo" DEE_PCKuSIZ, pattern, repl, maxcount));
	}
	WUNUSED string rereplace(DeeObject *pattern, /*utf-8*/ char const *__restrict repl) const {
		return inherit(DeeObject_CallAttrStringf(this->ptr(), "rereplace", "os", pattern, repl));
	}
	WUNUSED string rereplace(DeeObject *pattern, /*utf-8*/ char const *__restrict repl, size_t maxcount) const {
		return inherit(DeeObject_CallAttrStringf(this->ptr(), "rereplace", "os" DEE_PCKuSIZ, pattern, repl, maxcount));
	}
	WUNUSED string rereplace(/*utf-8*/ char const *__restrict pattern, DeeObject *repl) const {
		return inherit(DeeObject_CallAttrStringf(this->ptr(), "rereplace", "so", pattern, repl));
	}
	WUNUSED string rereplace(/*utf-8*/ char const *__restrict pattern, DeeObject *repl, size_t maxcount) const {
		return inherit(DeeObject_CallAttrStringf(this->ptr(), "rereplace", "so" DEE_PCKuSIZ, pattern, repl, maxcount));
	}
	WUNUSED string rereplace(/*utf-8*/ char const *__restrict pattern, /*utf-8*/ char const *__restrict repl) const {
		return inherit(DeeObject_CallAttrStringf(this->ptr(), "rereplace", "ss", pattern, repl));
	}
	WUNUSED string rereplace(/*utf-8*/ char const *__restrict pattern, /*utf-8*/ char const *__restrict repl, size_t maxcount) const {
		return inherit(DeeObject_CallAttrStringf(this->ptr(), "rereplace", "ss" DEE_PCKuSIZ, pattern, repl, maxcount));
	}
	WUNUSED string rereplace(DeeObject *pattern, DeeObject *repl, DeeObject *rules) const {
		return inherit(DeeObject_CallAttrStringf(this->ptr(), "rereplace", "ooo", pattern, repl, rules));
	}
	WUNUSED string rereplace(DeeObject *pattern, DeeObject *repl, size_t maxcount, DeeObject *rules) const {
		return inherit(DeeObject_CallAttrStringf(this->ptr(), "rereplace", "oo" DEE_PCKuSIZ "o", pattern, repl, maxcount, rules));
	}
	WUNUSED string rereplace(DeeObject *pattern, DeeObject *repl, DeeObject *rules, size_t maxcount) const {
		return inherit(DeeObject_CallAttrStringf(this->ptr(), "rereplace", "ooo" DEE_PCKuSIZ, pattern, repl, rules, maxcount));
	}
	WUNUSED string rereplace(DeeObject *pattern, /*utf-8*/ char const *__restrict repl, DeeObject *rules) const {
		return inherit(DeeObject_CallAttrStringf(this->ptr(), "rereplace", "oso", pattern, repl, rules));
	}
	WUNUSED string rereplace(DeeObject *pattern, /*utf-8*/ char const *__restrict repl, size_t maxcount, DeeObject *rules) const {
		return inherit(DeeObject_CallAttrStringf(this->ptr(), "rereplace", "os" DEE_PCKuSIZ "o", pattern, repl, maxcount, rules));
	}
	WUNUSED string rereplace(DeeObject *pattern, /*utf-8*/ char const *__restrict repl, DeeObject *rules, size_t maxcount) const {
		return inherit(DeeObject_CallAttrStringf(this->ptr(), "rereplace", "oso" DEE_PCKuSIZ, pattern, repl, rules, maxcount));
	}
	WUNUSED string rereplace(/*utf-8*/ char const *__restrict pattern, DeeObject *repl, DeeObject *rules) const {
		return inherit(DeeObject_CallAttrStringf(this->ptr(), "rereplace", "soo", pattern, repl, rules));
	}
	WUNUSED string rereplace(/*utf-8*/ char const *__restrict pattern, DeeObject *repl, size_t maxcount, DeeObject *rules) const {
		return inherit(DeeObject_CallAttrStringf(this->ptr(), "rereplace", "so" DEE_PCKuSIZ "o", pattern, repl, maxcount, rules));
	}
	WUNUSED string rereplace(/*utf-8*/ char const *__restrict pattern, DeeObject *repl, DeeObject *rules, size_t maxcount) const {
		return inherit(DeeObject_CallAttrStringf(this->ptr(), "rereplace", "soo" DEE_PCKuSIZ, pattern, repl, rules, maxcount));
	}
	WUNUSED string rereplace(/*utf-8*/ char const *__restrict pattern, /*utf-8*/ char const *__restrict repl, DeeObject *rules) const {
		return inherit(DeeObject_CallAttrStringf(this->ptr(), "rereplace", "sso", pattern, repl, rules));
	}
	WUNUSED string rereplace(/*utf-8*/ char const *__restrict pattern, /*utf-8*/ char const *__restrict repl, size_t maxcount, DeeObject *rules) const {
		return inherit(DeeObject_CallAttrStringf(this->ptr(), "rereplace", "ss" DEE_PCKuSIZ "o", pattern, repl, maxcount, rules));
	}
	WUNUSED string rereplace(/*utf-8*/ char const *__restrict pattern, /*utf-8*/ char const *__restrict repl, DeeObject *rules, size_t maxcount) const {
		return inherit(DeeObject_CallAttrStringf(this->ptr(), "rereplace", "sso" DEE_PCKuSIZ, pattern, repl, rules, maxcount));
	}
	WUNUSED string rereplace(DeeObject *pattern, DeeObject *repl, /*utf-8*/ char const *__restrict rules) const {
		return inherit(DeeObject_CallAttrStringf(this->ptr(), "rereplace", "oos", pattern, repl, rules));
	}
	WUNUSED string rereplace(DeeObject *pattern, DeeObject *repl, size_t maxcount, /*utf-8*/ char const *__restrict rules) const {
		return inherit(DeeObject_CallAttrStringf(this->ptr(), "rereplace", "oo" DEE_PCKuSIZ "s", pattern, repl, maxcount, rules));
	}
	WUNUSED string rereplace(DeeObject *pattern, DeeObject *repl, /*utf-8*/ char const *__restrict rules, size_t maxcount) const {
		return inherit(DeeObject_CallAttrStringf(this->ptr(), "rereplace", "oos" DEE_PCKuSIZ, pattern, repl, rules, maxcount));
	}
	WUNUSED string rereplace(DeeObject *pattern, /*utf-8*/ char const *__restrict repl, /*utf-8*/ char const *__restrict rules) const {
		return inherit(DeeObject_CallAttrStringf(this->ptr(), "rereplace", "oss", pattern, repl, rules));
	}
	WUNUSED string rereplace(DeeObject *pattern, /*utf-8*/ char const *__restrict repl, size_t maxcount, /*utf-8*/ char const *__restrict rules) const {
		return inherit(DeeObject_CallAttrStringf(this->ptr(), "rereplace", "os" DEE_PCKuSIZ "s", pattern, repl, maxcount, rules));
	}
	WUNUSED string rereplace(DeeObject *pattern, /*utf-8*/ char const *__restrict repl, /*utf-8*/ char const *__restrict rules, size_t maxcount) const {
		return inherit(DeeObject_CallAttrStringf(this->ptr(), "rereplace", "oss" DEE_PCKuSIZ, pattern, repl, rules, maxcount));
	}
	WUNUSED string rereplace(/*utf-8*/ char const *__restrict pattern, DeeObject *repl, /*utf-8*/ char const *__restrict rules) const {
		return inherit(DeeObject_CallAttrStringf(this->ptr(), "rereplace", "sos", pattern, repl, rules));
	}
	WUNUSED string rereplace(/*utf-8*/ char const *__restrict pattern, DeeObject *repl, size_t maxcount, /*utf-8*/ char const *__restrict rules) const {
		return inherit(DeeObject_CallAttrStringf(this->ptr(), "rereplace", "so" DEE_PCKuSIZ "s", pattern, repl, maxcount, rules));
	}
	WUNUSED string rereplace(/*utf-8*/ char const *__restrict pattern, DeeObject *repl, /*utf-8*/ char const *__restrict rules, size_t maxcount) const {
		return inherit(DeeObject_CallAttrStringf(this->ptr(), "rereplace", "sos" DEE_PCKuSIZ, pattern, repl, rules, maxcount));
	}
	WUNUSED string rereplace(/*utf-8*/ char const *__restrict pattern, /*utf-8*/ char const *__restrict repl, /*utf-8*/ char const *__restrict rules) const {
		return inherit(DeeObject_CallAttrStringf(this->ptr(), "rereplace", "sss", pattern, repl, rules));
	}
	WUNUSED string rereplace(/*utf-8*/ char const *__restrict pattern, /*utf-8*/ char const *__restrict repl, size_t maxcount, /*utf-8*/ char const *__restrict rules) const {
		return inherit(DeeObject_CallAttrStringf(this->ptr(), "rereplace", "ss" DEE_PCKuSIZ "s", pattern, repl, maxcount, rules));
	}
	WUNUSED string rereplace(/*utf-8*/ char const *__restrict pattern, /*utf-8*/ char const *__restrict repl, /*utf-8*/ char const *__restrict rules, size_t maxcount) const {
		return inherit(DeeObject_CallAttrStringf(this->ptr(), "rereplace", "sss" DEE_PCKuSIZ, pattern, repl, rules, maxcount));
	}

public:
	WUNUSED string add(DeeObject *right) const {
		return inherit(DeeObject_Add(this->ptr(), right));
	}
	WUNUSED string operator+(DeeObject *right) const {
		return inherit(DeeObject_Add(this->ptr(), right));
	}
	WUNUSED string &inplace_add(DeeObject *right) {
		Sequence<string>::inplace_add(right);
		return *this;
	}
	WUNUSED string &operator+=(DeeObject *right) {
		Sequence<string>::operator+=(right);
		return *this;
	}
	WUNUSED string mul(DeeObject *n) const {
		return inherit(DeeObject_Mul(this->ptr(), n));
	}
	WUNUSED string mul(int8_t n) const {
		return inherit(DeeObject_MulInt(this->ptr(), n));
	}
	WUNUSED string mul(size_t n) const {
		return inherit(_mulref(n));
	}
	WUNUSED string operator*(DeeObject *n) const {
		return inherit(DeeObject_Mul(this->ptr(), n));
	}
	WUNUSED string operator*(int8_t n) const {
		return inherit(DeeObject_MulInt(this->ptr(), n));
	}
	WUNUSED string operator*(size_t n) const {
		return inherit(_mulref(n));
	}
	WUNUSED string &inplace_mul(DeeObject *n) {
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
	WUNUSED string &operator*=(DeeObject *n) {
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
inline WUNUSED deemon::int_ string::asnumber() const {
	return inherit(DeeObject_CallAttrString(this->ptr(), "asnumber", 0, NULL));
}
inline WUNUSED deemon::int_ string::asnumber(size_t index) const {
	return inherit(DeeObject_CallAttrStringf(this->ptr(), "asnumber", DEE_PCKuSIZ, index));
}
inline WUNUSED deemon::int_ string::asnumber(size_t index, int defl) const {
	return inherit(DeeObject_CallAttrStringf(this->ptr(), "asnumber", DEE_PCKuSIZ "d", index, defl));
}
inline WUNUSED deemon::int_ string::asnumber(DeeObject *__restrict index) const {
	return inherit(DeeObject_CallAttrStringf(this->ptr(), "asnumber", "o", index));
}
inline WUNUSED deemon::int_ string::asnumber(DeeObject *__restrict index, int defl) const {
	return inherit(DeeObject_CallAttrStringf(this->ptr(), "asnumber", "od", index, defl));
}
inline WUNUSED deemon::int_ string::asdigit() const {
	return inherit(DeeObject_CallAttrString(this->ptr(), "asdigit", 0, NULL));
}
inline WUNUSED deemon::int_ string::asdigit(size_t index) const {
	return inherit(DeeObject_CallAttrStringf(this->ptr(), "asdigit", DEE_PCKuSIZ, index));
}
inline WUNUSED deemon::int_ string::asdigit(size_t index, int defl) const {
	return inherit(DeeObject_CallAttrStringf(this->ptr(), "asdigit", DEE_PCKuSIZ "d", index, defl));
}
inline WUNUSED deemon::int_ string::asdigit(DeeObject *__restrict index) const {
	return inherit(DeeObject_CallAttrStringf(this->ptr(), "asdigit", "o", index));
}
inline WUNUSED deemon::int_ string::asdigit(DeeObject *__restrict index, int defl) const {
	return inherit(DeeObject_CallAttrStringf(this->ptr(), "asdigit", "od", index, defl));
}
inline WUNUSED deemon::int_ string::asdecimal() const {
	return inherit(DeeObject_CallAttrString(this->ptr(), "asdecimal", 0, NULL));
}
inline WUNUSED deemon::int_ string::asdecimal(size_t index) const {
	return inherit(DeeObject_CallAttrStringf(this->ptr(), "asdecimal", DEE_PCKuSIZ, index));
}
inline WUNUSED deemon::int_ string::asdecimal(size_t index, int defl) const {
	return inherit(DeeObject_CallAttrStringf(this->ptr(), "asdecimal", DEE_PCKuSIZ "d", index, defl));
}
inline WUNUSED deemon::int_ string::asdecimal(DeeObject *__restrict index) const {
	return inherit(DeeObject_CallAttrStringf(this->ptr(), "asdecimal", "o", index));
}
inline WUNUSED deemon::int_ string::asdecimal(DeeObject *__restrict index, int defl) const {
	return inherit(DeeObject_CallAttrStringf(this->ptr(), "asdecimal", "od", index, defl));
}
#define DEFINE_FIND_FUNCTION(Treturn, name, needle)                                                                     \
	inline WUNUSED Treturn(string::name)(/*utf-8*/ char const *__restrict needle) const {                               \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, "s", needle));                                     \
	}                                                                                                                   \
	inline WUNUSED Treturn(string::name)(/*utf-8*/ char const *__restrict needle, size_t start) const {                 \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, "s" DEE_PCKuSIZ, needle, start));                  \
	}                                                                                                                   \
	inline WUNUSED Treturn(string::name)(/*utf-8*/ char const *__restrict needle, size_t start, size_t end) const {     \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, "s" DEE_PCKuSIZ DEE_PCKuSIZ, needle, start, end)); \
	}                                                                                                                   \
	inline WUNUSED Treturn(string::name)(DeeObject *__restrict needle) const {                                          \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, "o", needle));                                     \
	}                                                                                                                   \
	inline WUNUSED Treturn(string::name)(DeeObject *__restrict needle, size_t start) const {                            \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, "o" DEE_PCKuSIZ, needle, start));                  \
	}                                                                                                                   \
	inline WUNUSED Treturn(string::name)(DeeObject *__restrict needle, size_t start, size_t end) const {                \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, "o" DEE_PCKuSIZ DEE_PCKuSIZ, needle, start, end)); \
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
#define DEFINE_COMPARE_FUNCTION(Treturn, name)                                                                                                                               \
	inline WUNUSED Treturn(string::name)(/*utf-8*/ char const *__restrict other) const {                                                                                     \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, "s", other));                                                                                           \
	}                                                                                                                                                                        \
	inline WUNUSED Treturn(string::name)(/*utf-8*/ char const *__restrict other, size_t other_size) const {                                                                  \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, "$s", other_size, other));                                                                              \
	}                                                                                                                                                                        \
	inline WUNUSED Treturn(string::name)(size_t my_start, /*utf-8*/ char const *__restrict other) const {                                                                    \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, DEE_PCKuSIZ "s", my_start, other));                                                                     \
	}                                                                                                                                                                        \
	inline WUNUSED Treturn(string::name)(size_t my_start, /*utf-8*/ char const *__restrict other, size_t other_size) const {                                                 \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, DEE_PCKuSIZ "$s", my_start, other_size, other));                                                        \
	}                                                                                                                                                                        \
	inline WUNUSED Treturn(string::name)(size_t my_start, size_t my_end, /*utf-8*/ char const *__restrict other) const {                                                     \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, DEE_PCKuSIZ DEE_PCKuSIZ "s", my_start, my_end, other));                                                 \
	}                                                                                                                                                                        \
	inline WUNUSED Treturn(string::name)(size_t my_start, size_t my_end, /*utf-8*/ char const *__restrict other, size_t other_size) const {                                  \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, DEE_PCKuSIZ DEE_PCKuSIZ "$s", my_start, my_end, other_size, other));                                    \
	}                                                                                                                                                                        \
	inline WUNUSED Treturn(string::name)(DeeObject *__restrict other) const {                                                                                                \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, "o", other));                                                                                           \
	}                                                                                                                                                                        \
	inline WUNUSED Treturn(string::name)(DeeObject *__restrict other, size_t other_start) const {                                                                            \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, "o" DEE_PCKuSIZ, other, other_start));                                                                  \
	}                                                                                                                                                                        \
	inline WUNUSED Treturn(string::name)(DeeObject *__restrict other, size_t other_start, size_t other_end) const {                                                          \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, "o" DEE_PCKuSIZ DEE_PCKuSIZ, other, other_start, other_end));                                           \
	}                                                                                                                                                                        \
	inline WUNUSED Treturn(string::name)(size_t my_start, DeeObject *__restrict other) const {                                                                               \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, DEE_PCKuSIZ "o", my_start, other));                                                                     \
	}                                                                                                                                                                        \
	inline WUNUSED Treturn(string::name)(size_t my_start, DeeObject *__restrict other, size_t other_start) const {                                                           \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, DEE_PCKuSIZ "o" DEE_PCKuSIZ, my_start, other, other_start));                                            \
	}                                                                                                                                                                        \
	inline WUNUSED Treturn(string::name)(size_t my_start, DeeObject *__restrict other, size_t other_start, size_t other_end) const {                                         \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, DEE_PCKuSIZ "o" DEE_PCKuSIZ DEE_PCKuSIZ, my_start, other, other_start, other_end));                     \
	}                                                                                                                                                                        \
	inline WUNUSED Treturn(string::name)(size_t my_start, size_t my_end, DeeObject *__restrict other) const {                                                                \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, DEE_PCKuSIZ DEE_PCKuSIZ "o", my_start, my_end, other));                                                 \
	}                                                                                                                                                                        \
	inline WUNUSED Treturn(string::name)(size_t my_start, size_t my_end, DeeObject *__restrict other, size_t other_start) const {                                            \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, DEE_PCKuSIZ DEE_PCKuSIZ "o" DEE_PCKuSIZ, my_start, my_end, other, other_start));                        \
	}                                                                                                                                                                        \
	inline WUNUSED Treturn(string::name)(size_t my_start, size_t my_end, DeeObject *__restrict other, size_t other_start, size_t other_end) const {                          \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, DEE_PCKuSIZ DEE_PCKuSIZ "o" DEE_PCKuSIZ DEE_PCKuSIZ, my_start, my_end, other, other_start, other_end)); \
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
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, "oo", open, close));                                                                 \
	}                                                                                                                                                     \
	inline WUNUSED Treturn(string::name)(DeeObject *__restrict open, DeeObject *__restrict close, size_t start) const {                                   \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, "oo" DEE_PCKuSIZ, open, close, start));                                              \
	}                                                                                                                                                     \
	inline WUNUSED Treturn(string::name)(DeeObject *__restrict open, DeeObject *__restrict close, size_t start, size_t end) const {                       \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, "oo" DEE_PCKuSIZ DEE_PCKuSIZ, open, close, start, end));                             \
	}                                                                                                                                                     \
	inline WUNUSED Treturn(string::name)(DeeObject *__restrict open, /*utf-8*/ char const *__restrict close) const {                                      \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, "os", open, close));                                                                 \
	}                                                                                                                                                     \
	inline WUNUSED Treturn(string::name)(DeeObject *__restrict open, /*utf-8*/ char const *__restrict close, size_t start) const {                        \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, "os" DEE_PCKuSIZ, open, close, start));                                              \
	}                                                                                                                                                     \
	inline WUNUSED Treturn(string::name)(DeeObject *__restrict open, /*utf-8*/ char const *__restrict close, size_t start, size_t end) const {            \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, "os" DEE_PCKuSIZ DEE_PCKuSIZ, open, close, start, end));                             \
	}                                                                                                                                                     \
	inline WUNUSED Treturn(string::name)(/*utf-8*/ char const *__restrict open, DeeObject *__restrict close) const {                                      \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, "so", open, close));                                                                 \
	}                                                                                                                                                     \
	inline WUNUSED Treturn(string::name)(/*utf-8*/ char const *__restrict open, DeeObject *__restrict close, size_t start) const {                        \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, "so" DEE_PCKuSIZ, open, close, start));                                              \
	}                                                                                                                                                     \
	inline WUNUSED Treturn(string::name)(/*utf-8*/ char const *__restrict open, DeeObject *__restrict close, size_t start, size_t end) const {            \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, "so" DEE_PCKuSIZ DEE_PCKuSIZ, open, close, start, end));                             \
	}                                                                                                                                                     \
	inline WUNUSED Treturn(string::name)(/*utf-8*/ char const *__restrict open, /*utf-8*/ char const *__restrict close) const {                           \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, "ss", open, close));                                                                 \
	}                                                                                                                                                     \
	inline WUNUSED Treturn(string::name)(/*utf-8*/ char const *__restrict open, /*utf-8*/ char const *__restrict close, size_t start) const {             \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, "ss" DEE_PCKuSIZ, open, close, start));                                              \
	}                                                                                                                                                     \
	inline WUNUSED Treturn(string::name)(/*utf-8*/ char const *__restrict open, /*utf-8*/ char const *__restrict close, size_t start, size_t end) const { \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, "ss" DEE_PCKuSIZ DEE_PCKuSIZ, open, close, start, end));                             \
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
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, "o", pattern));                                                                         \
	}                                                                                                                                                        \
	inline WUNUSED Treturn(string::name)(DeeObject *__restrict pattern, DeeObject *__restrict rules) const {                                                 \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, "oo", pattern, rules));                                                                 \
	}                                                                                                                                                        \
	inline WUNUSED Treturn(string::name)(DeeObject *__restrict pattern, /*utf-8*/ char const *__restrict rules) const {                                      \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, "os", pattern, rules));                                                                 \
	}                                                                                                                                                        \
	inline WUNUSED Treturn(string::name)(DeeObject *__restrict pattern, size_t start) const {                                                                \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, "o" DEE_PCKuSIZ, pattern, start));                                                      \
	}                                                                                                                                                        \
	inline WUNUSED Treturn(string::name)(DeeObject *__restrict pattern, DeeObject *__restrict rules, size_t start) const {                                   \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, "oo" DEE_PCKuSIZ, pattern, rules, start));                                              \
	}                                                                                                                                                        \
	inline WUNUSED Treturn(string::name)(DeeObject *__restrict pattern, /*utf-8*/ char const *__restrict rules, size_t start) const {                        \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, "os" DEE_PCKuSIZ, pattern, rules, start));                                              \
	}                                                                                                                                                        \
	inline WUNUSED Treturn(string::name)(DeeObject *__restrict pattern, size_t start, size_t end) const {                                                    \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, "o" DEE_PCKuSIZ DEE_PCKuSIZ, pattern, start, end));                                     \
	}                                                                                                                                                        \
	inline WUNUSED Treturn(string::name)(DeeObject *__restrict pattern, DeeObject *__restrict rules, size_t start, size_t end) const {                       \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, "oo" DEE_PCKuSIZ DEE_PCKuSIZ, pattern, rules, start, end));                             \
	}                                                                                                                                                        \
	inline WUNUSED Treturn(string::name)(DeeObject *__restrict pattern, /*utf-8*/ char const *__restrict rules, size_t start, size_t end) const {            \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, "os" DEE_PCKuSIZ DEE_PCKuSIZ, pattern, rules, start, end));                             \
	}                                                                                                                                                        \
	inline WUNUSED Treturn(string::name)(DeeObject *__restrict pattern, size_t start, DeeObject *__restrict rules) const {                                   \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, "o" DEE_PCKuSIZ "o", pattern, start, rules));                                           \
	}                                                                                                                                                        \
	inline WUNUSED Treturn(string::name)(DeeObject *__restrict pattern, size_t start, size_t end, DeeObject *__restrict rules) const {                       \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, "o" DEE_PCKuSIZ DEE_PCKuSIZ "o", pattern, start, end, rules));                          \
	}                                                                                                                                                        \
	inline WUNUSED Treturn(string::name)(DeeObject *__restrict pattern, size_t start, /*utf-8*/ char const *__restrict rules) const {                        \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, "o" DEE_PCKuSIZ "s", pattern, start, rules));                                           \
	}                                                                                                                                                        \
	inline WUNUSED Treturn(string::name)(DeeObject *__restrict pattern, size_t start, size_t end, /*utf-8*/ char const *__restrict rules) const {            \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, "o" DEE_PCKuSIZ DEE_PCKuSIZ "s", pattern, start, end, rules));                          \
	}                                                                                                                                                        \
	inline WUNUSED Treturn(string::name)(/*utf-8*/ char const *__restrict pattern) const {                                                                   \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, "s", pattern));                                                                         \
	}                                                                                                                                                        \
	inline WUNUSED Treturn(string::name)(/*utf-8*/ char const *__restrict pattern, DeeObject *__restrict rules) const {                                      \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, "so", pattern, rules));                                                                 \
	}                                                                                                                                                        \
	inline WUNUSED Treturn(string::name)(/*utf-8*/ char const *__restrict pattern, /*utf-8*/ char const *__restrict rules) const {                           \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, "ss", pattern, rules));                                                                 \
	}                                                                                                                                                        \
	inline WUNUSED Treturn(string::name)(/*utf-8*/ char const *__restrict pattern, size_t start) const {                                                     \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, "s" DEE_PCKuSIZ, pattern, start));                                                      \
	}                                                                                                                                                        \
	inline WUNUSED Treturn(string::name)(/*utf-8*/ char const *__restrict pattern, DeeObject *__restrict rules, size_t start) const {                        \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, "so" DEE_PCKuSIZ, pattern, rules, start));                                              \
	}                                                                                                                                                        \
	inline WUNUSED Treturn(string::name)(/*utf-8*/ char const *__restrict pattern, /*utf-8*/ char const *__restrict rules, size_t start) const {             \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, "ss" DEE_PCKuSIZ, pattern, rules, start));                                              \
	}                                                                                                                                                        \
	inline WUNUSED Treturn(string::name)(/*utf-8*/ char const *__restrict pattern, size_t start, size_t end) const {                                         \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, "s" DEE_PCKuSIZ DEE_PCKuSIZ, pattern, start, end));                                     \
	}                                                                                                                                                        \
	inline WUNUSED Treturn(string::name)(/*utf-8*/ char const *__restrict pattern, DeeObject *__restrict rules, size_t start, size_t end) const {            \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, "so" DEE_PCKuSIZ DEE_PCKuSIZ, pattern, rules, start, end));                             \
	}                                                                                                                                                        \
	inline WUNUSED Treturn(string::name)(/*utf-8*/ char const *__restrict pattern, /*utf-8*/ char const *__restrict rules, size_t start, size_t end) const { \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, "ss" DEE_PCKuSIZ DEE_PCKuSIZ, pattern, rules, start, end));                             \
	}                                                                                                                                                        \
	inline WUNUSED Treturn(string::name)(/*utf-8*/ char const *__restrict pattern, size_t start, DeeObject *__restrict rules) const {                        \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, "s" DEE_PCKuSIZ "o", pattern, start, rules));                                           \
	}                                                                                                                                                        \
	inline WUNUSED Treturn(string::name)(/*utf-8*/ char const *__restrict pattern, size_t start, size_t end, DeeObject *__restrict rules) const {            \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, "s" DEE_PCKuSIZ DEE_PCKuSIZ "o", pattern, start, end, rules));                          \
	}                                                                                                                                                        \
	inline WUNUSED Treturn(string::name)(/*utf-8*/ char const *__restrict pattern, size_t start, /*utf-8*/ char const *__restrict rules) const {             \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, "s" DEE_PCKuSIZ "s", pattern, start, rules));                                           \
	}                                                                                                                                                        \
	inline WUNUSED Treturn(string::name)(/*utf-8*/ char const *__restrict pattern, size_t start, size_t end, /*utf-8*/ char const *__restrict rules) const { \
		return inherit(DeeObject_CallAttrStringf(this->ptr(), #name, "s" DEE_PCKuSIZ DEE_PCKuSIZ "s", pattern, start, end, rules));                          \
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
	return inherit(DeeFile_Filename(this->ptr()));
}
inline deemon::string(File::Writer::string)() const {
	return inherit(DeeObject_InstanceOfExact(this->ptr(),
	                                         (DeeTypeObject *)&DeeFileWriter_Type)
	               ? DeeFileWriter_GetString(this->ptr())
	               : DeeObject_GetAttrString(this->ptr(), "string"));
}
#endif /* GUARD_DEEMON_CXX_FILE_H */

#ifdef GUARD_DEEMON_CXX_BYTES_H
inline WUNUSED Bytes(string::bytes)() const {
	return inherit(DeeObject_CallAttrString(this->ptr(), "bytes", 0, NULL));
}
inline WUNUSED Bytes(string::bytes)(bool allow_invalid) const {
	return inherit(DeeObject_CallAttrStringf(this->ptr(), "bytes", "b", allow_invalid));
}
inline WUNUSED Bytes(string::bytes)(size_t start, size_t end) const {
	return inherit(DeeObject_CallAttrStringf(this->ptr(), "bytes", DEE_PCKuSIZ DEE_PCKuSIZ, start, end));
}
inline WUNUSED Bytes(string::bytes)(size_t start, size_t end, bool allow_invalid) const {
	return inherit(DeeObject_CallAttrStringf(this->ptr(), "bytes", DEE_PCKuSIZ DEE_PCKuSIZ "b", start, end, allow_invalid));
}
inline WUNUSED Bytes(string::encode)(/*utf-8*/ char const *__restrict codec_name) const {
	return inherit(DeeObject_CallAttrStringf(this->ptr(), "encode", "s", codec_name));
}
inline WUNUSED Bytes(string::encode)(obj_string codec_name) const {
	return inherit(DeeObject_CallAttrStringf(this->ptr(), "encode", "o", (DeeObject *)codec_name));
}
inline WUNUSED Bytes(string::encode)(/*utf-8*/ char const *__restrict codec_name, /*utf-8*/ char const *__restrict errors) const {
	return inherit(DeeObject_CallAttrStringf(this->ptr(), "encode", "ss", codec_name, errors));
}
inline WUNUSED Bytes(string::encode)(/*utf-8*/ char const *__restrict codec_name, obj_string errors) const {
	return inherit(DeeObject_CallAttrStringf(this->ptr(), "encode", "so", codec_name, (DeeObject *)errors));
}
inline WUNUSED Bytes(string::encode)(obj_string codec_name, /*utf-8*/ char const *__restrict errors) const {
	return inherit(DeeObject_CallAttrStringf(this->ptr(), "encode", "os", (DeeObject *)codec_name, errors));
}
inline WUNUSED Bytes(string::encode)(obj_string codec_name, obj_string errors) const {
	return inherit(DeeObject_CallAttrStringf(this->ptr(), "encode", "oo", (DeeObject *)codec_name, (DeeObject *)errors));
}
inline WUNUSED string(Bytes::decode)(/*utf-8*/ char const *__restrict codec_name) const {
	return inherit(DeeObject_CallAttrStringf(this->ptr(), "decode", "s", codec_name));
}
inline WUNUSED string(Bytes::decode)(obj_string codec_name) const {
	return inherit(DeeObject_CallAttrStringf(this->ptr(), "decode", "o", (DeeObject *)codec_name));
}
inline WUNUSED string(Bytes::decode)(/*utf-8*/ char const *__restrict codec_name, /*utf-8*/ char const *__restrict errors) const {
	return inherit(DeeObject_CallAttrStringf(this->ptr(), "decode", "ss", codec_name, errors));
}
inline WUNUSED string(Bytes::decode)(/*utf-8*/ char const *__restrict codec_name, obj_string errors) const {
	return inherit(DeeObject_CallAttrStringf(this->ptr(), "decode", "so", codec_name, (DeeObject *)errors));
}
inline WUNUSED string(Bytes::decode)(obj_string codec_name, /*utf-8*/ char const *__restrict errors) const {
	return inherit(DeeObject_CallAttrStringf(this->ptr(), "decode", "os", (DeeObject *)codec_name, errors));
}
inline WUNUSED string(Bytes::decode)(obj_string codec_name, obj_string errors) const {
	return inherit(DeeObject_CallAttrStringf(this->ptr(), "decode", "oo", (DeeObject *)codec_name, (DeeObject *)errors));
}
#endif /* GUARD_DEEMON_CXX_BYTES_H */


inline string Object::str() const {
	return inherit(DeeObject_Str(this->ptr()));
}
inline string Object::repr() const {
	return inherit(DeeObject_Repr(this->ptr()));
}
inline string str(DeeObject *__restrict ob) {
	return inherit(DeeObject_Str(ob));
}
inline string repr(DeeObject *__restrict ob) {
	return inherit(DeeObject_Repr(ob));
}

DEE_CXX_END

#endif /* !GUARD_DEEMON_CXX_STRING_H */
