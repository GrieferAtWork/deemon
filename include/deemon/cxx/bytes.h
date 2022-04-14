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
#ifndef GUARD_DEEMON_CXX_BYTES_H
#define GUARD_DEEMON_CXX_BYTES_H 1

#include "api.h"
/**/

#include <hybrid/typecore.h>

#include <initializer_list>

#include "../format.h" /* DEE_PCK* */
#include "../bytes.h"
#include "../system-features.h" /* strlen() */
#include "int.h"
#include "object.h"
#include "sequence.h"
#include "tuple.h"

DEE_CXX_BEGIN

class Bytes: public Sequence<int_> {
public:
	static DeeTypeObject *classtype() DEE_CXX_NOTHROW {
		return &DeeBytes_Type;
	}
	static bool check(DeeObject *__restrict ob) DEE_CXX_NOTHROW {
		return DeeBytes_Check(ob);
	}
	static bool checkexact(DeeObject *__restrict ob) DEE_CXX_NOTHROW {
		return DeeBytes_CheckExact(ob);
	}

public: /* Raw data access */
	explicit ATTR_RETNONNULL operator char *() const DEE_CXX_NOTHROW {
		return str();
	}
	WUNUSED ATTR_RETNONNULL char *str() const DEE_CXX_NOTHROW {
		return (char *)DeeBytes_DATA(this->ptr());
	}
	WUNUSED ATTR_RETNONNULL void *data() const DEE_CXX_NOTHROW {
		return DeeBytes_DATA(this->ptr());
	}
	WUNUSED size_t size() const DEE_CXX_NOTHROW {
		return DeeBytes_SIZE(this->ptr());
	}

public: /* Constructors */
	DEE_CXX_DEFINE_OBJECT_CONSTRUCTORS(Bytes, Sequence)
	Bytes() DEE_CXX_NOTHROW: Sequence(nonnull(Dee_EmptyBytes)) {}
	Bytes(std::initializer_list<uint8_t> const &data)
	    : Sequence(inherit(DeeBytes_NewBufferData(data.begin(), data.size()))) {}
	Bytes(DeeObject *owner, void *base, size_t num_bytes, unsigned int flags)
	    : Sequence(inherit(DeeBytes_NewView(owner, base, num_bytes, flags))) {}
	Bytes(DeeObject *owner, void *base, size_t num_bytes)
	    : Sequence(inherit(DeeBytes_NewView(owner, base, num_bytes, Dee_BUFFER_FWRITABLE))) {}
	Bytes(DeeObject *owner, void const *base, size_t num_bytes)
	    : Sequence(inherit(DeeBytes_NewView(owner, (void *)base, num_bytes, Dee_BUFFER_FREADONLY))) {}
	Bytes(void const *base, size_t num_bytes)
	    : Sequence(inherit(DeeBytes_NewBufferData(base, num_bytes))) {}
	Bytes(char const *str)
	    : Sequence(inherit(DeeBytes_NewBufferData(str, strlen(str)))) {}
	static WUNUSED Bytes of(DeeObject *__restrict buffer_ob,
	                        unsigned int flags = Dee_BUFFER_FWRITABLE,
	                        size_t start = 0, size_t end = (size_t)-1) {
		return inherit(DeeObject_Bytes(buffer_ob, flags, start, end));
	}
	static WUNUSED Bytes ofro(DeeObject *__restrict buffer_ob, size_t start = 0, size_t end = (size_t)-1) {
		return inherit(DeeObject_Bytes(buffer_ob, Dee_BUFFER_FREADONLY, start, end));
	}
	static WUNUSED Bytes ofwr(DeeObject *__restrict buffer_ob, size_t start = 0, size_t end = (size_t)-1) {
		return inherit(DeeObject_Bytes(buffer_ob, Dee_BUFFER_FWRITABLE, start, end));
	}
	static WUNUSED Bytes fromseq(DeeObject *__restrict byteseq) {
		return inherit(DeeBytes_FromSequence(byteseq));
	}
	static WUNUSED Bytes buffer(size_t num_bytes) {
		return inherit(DeeBytes_NewBufferUninitialized(num_bytes));
	}
	static WUNUSED Bytes buffer(size_t num_bytes, uint8_t init) {
		return inherit(DeeBytes_NewBuffer(num_bytes, init));
	}
	static WUNUSED Bytes buffer(void const *__restrict data, size_t num_bytes) {
		return inherit(DeeBytes_NewBufferData(data, num_bytes));
	}
	static WUNUSED Bytes view(DeeObject *owner, void *base, size_t num_bytes, unsigned int flags) {
		return inherit(DeeBytes_NewView(owner, base, num_bytes, flags));
	}
	static WUNUSED Bytes view(DeeObject *owner, void *base, size_t num_bytes) {
		return inherit(DeeBytes_NewView(owner, base, num_bytes, Dee_BUFFER_FWRITABLE));
	}
	static WUNUSED Bytes view(DeeObject *owner, void const *base, size_t num_bytes) {
		return inherit(DeeBytes_NewView(owner, (void *)base, num_bytes, Dee_BUFFER_FREADONLY));
	}

public: /* Helper functions */
	WUNUSED Bytes subview(void *base, size_t num_bytes) const {
		return inherit(DeeBytes_NewSubView(this->ptr(), base, num_bytes));
	}
	WUNUSED Bytes subviewro(void const *base, size_t num_bytes) const {
		return inherit(DeeBytes_NewSubViewRo(this->ptr(), base, num_bytes));
	}
	void resize(size_t num_bytes) {
		if likely(DeeBytes_Check(this->ptr()))
			m_ptr = throw_if_null(DeeBytes_ResizeBuffer(*this, num_bytes));
		else {
			Sequence::resize(num_bytes);
		}
	}
	void truncate(size_t num_bytes) {
		if likely(DeeBytes_Check(this->ptr()))
			m_ptr = DeeBytes_TruncateBuffer(*this, num_bytes);
		else {
			Sequence::resize(num_bytes);
		}
	}

public: /* API functions */
	static WUNUSED Bytes fromhex(char const *__restrict hexstring) {
		return inherit(DeeObject_CallAttrStringf((DeeObject *)&DeeBytes_Type, "fromhex", "s", hexstring));
	}
	static WUNUSED Bytes fromhex(obj_string hexstring) {
		return inherit(DeeObject_CallAttrStringf((DeeObject *)&DeeBytes_Type, "fromhex", "o", (DeeObject *)hexstring));
	}
	WUNUSED bool isreadonly() const DEE_CXX_NOTHROW {
		return !DeeBytes_WRITABLE(this->ptr());
	}
	WUNUSED bool iswritable() const DEE_CXX_NOTHROW {
		return DeeBytes_WRITABLE(this->ptr());
	}
	WUNUSED bool ismutable() const DEE_CXX_NOTHROW {
		return DeeBytes_WRITABLE(this->ptr());
	}
	bool empty() const DEE_CXX_NOTHROW {
		return DeeBytes_IsEmpty(this->ptr());
	}
	bool nonempty() const DEE_CXX_NOTHROW {
		return !DeeBytes_IsEmpty(this->ptr());
	}
	WUNUSED Bytes &bytes() {
		return *this;
	}
	WUNUSED Bytes const &bytes() const {
		return *this;
	}
	WUNUSED Bytes bytes(size_t start, size_t end) const {
		size_t len = size();
		if (end > len)
			end = len;
		if (start > end)
			start = end;
		return inherit(DeeBytes_NewSubView(this->ptr(),
		                                   (__BYTE_TYPE__ *)data() + end,
		                                   end - start));
	}
	inline WUNUSED string decode(/*utf-8*/ char const *__restrict codec_name) const;
	inline WUNUSED string decode(obj_string codec_name) const;
	inline WUNUSED string decode(/*utf-8*/ char const *__restrict codec_name, /*utf-8*/ char const *__restrict errors) const;
	inline WUNUSED string decode(/*utf-8*/ char const *__restrict codec_name, obj_string errors) const;
	inline WUNUSED string decode(obj_string codec_name, /*utf-8*/ char const *__restrict errors) const;
	inline WUNUSED string decode(obj_string codec_name, obj_string errors) const;
	WUNUSED Object decodeob(/*utf-8*/ char const *__restrict codec_name) const {
		return inherit(DeeObject_CallAttrStringf(*this, "decode", "s", codec_name));
	}
	WUNUSED Object decodeob(obj_string codec_name) const {
		return inherit(DeeObject_CallAttrStringf(*this, "decode", "o", (DeeObject *)codec_name));
	}
	WUNUSED Object decodeob(/*utf-8*/ char const *__restrict codec_name, /*utf-8*/ char const *__restrict errors) const {
		return inherit(DeeObject_CallAttrStringf(*this, "decode", "ss", codec_name, errors));
	}
	WUNUSED Object decodeob(/*utf-8*/ char const *__restrict codec_name, obj_string errors) const {
		return inherit(DeeObject_CallAttrStringf(*this, "decode", "so", codec_name, (DeeObject *)errors));
	}
	WUNUSED Object decodeob(obj_string codec_name, /*utf-8*/ char const *__restrict errors) const {
		return inherit(DeeObject_CallAttrStringf(*this, "decode", "os", (DeeObject *)codec_name, errors));
	}
	WUNUSED Object decodeob(obj_string codec_name, obj_string errors) const {
		return inherit(DeeObject_CallAttrStringf(*this, "decode", "oo", (DeeObject *)codec_name, (DeeObject *)errors));
	}
	WUNUSED Object encode(/*utf-8*/ char const *__restrict codec_name) const {
		return inherit(DeeObject_CallAttrStringf(*this, "encode", "s", codec_name));
	}
	WUNUSED Object encode(obj_string codec_name) const {
		return inherit(DeeObject_CallAttrStringf(*this, "encode", "o", (DeeObject *)codec_name));
	}
	WUNUSED Object encode(/*utf-8*/ char const *__restrict codec_name, /*utf-8*/ char const *__restrict errors) const {
		return inherit(DeeObject_CallAttrStringf(*this, "encode", "ss", codec_name, errors));
	}
	WUNUSED Object encode(/*utf-8*/ char const *__restrict codec_name, obj_string errors) const {
		return inherit(DeeObject_CallAttrStringf(*this, "encode", "so", codec_name, (DeeObject *)errors));
	}
	WUNUSED Object encode(obj_string codec_name, /*utf-8*/ char const *__restrict errors) const {
		return inherit(DeeObject_CallAttrStringf(*this, "encode", "os", (DeeObject *)codec_name, errors));
	}
	WUNUSED Object encode(obj_string codec_name, obj_string errors) const {
		return inherit(DeeObject_CallAttrStringf(*this, "encode", "oo", (DeeObject *)codec_name, (DeeObject *)errors));
	}
	WUNUSED Bytes &reverse() {
		decref(throw_if_null(DeeObject_CallAttrString(*this, "reverse", 0, NULL)));
		return *this;
	}
	WUNUSED Bytes &reverse(size_t start) {
		decref(throw_if_null(DeeObject_CallAttrStringf(*this, "reverse", DEE_PCKuSIZ, start)));
		return *this;
	}
	WUNUSED Bytes &reverse(size_t start, size_t end) {
		decref(throw_if_null(DeeObject_CallAttrStringf(*this, "reverse", DEE_PCKuSIZ DEE_PCKuSIZ, start, end)));
		return *this;
	}
	WUNUSED Bytes makereadonly() const {
		return inherit(DeeObject_CallAttrString(*this, "makereadonly", 0, NULL));
	}
	WUNUSED Bytes makewritable() const {
		return inherit(DeeObject_CallAttrString(*this, "makewritable", 0, NULL));
	}
	inline WUNUSED string hex() const;
	inline WUNUSED string hex(size_t start) const;
	inline WUNUSED string hex(size_t start, size_t end) const;
	inline WUNUSED Bytes format(obj_sequence args) const {
		return inherit(DeeObject_CallAttrStringf(*this, "format", "o", (DeeObject *)args));
	}
	inline WUNUSED Bytes format(std::initializer_list<DeeObject *> const &args) const {
		Tuple<Object> ob_args(args);
		return inherit(DeeObject_CallAttrStringf(*this, "format", "o", (DeeObject *)ob_args));
	}


#define DEFINE_CHARACTER_TRAIT(name)                                                                                  \
	WUNUSED bool(name)() const {                                                                                      \
		return Object(inherit(DeeObject_CallAttrString(*this, #name, 0, NULL))).bool_();                              \
	}                                                                                                                 \
	WUNUSED bool(name)(size_t index) const {                                                                          \
		return Object(inherit(DeeObject_CallAttrStringf(*this, #name, DEE_PCKuSIZ, index))).bool_();                  \
	}                                                                                                                 \
	WUNUSED bool(name)(size_t start, size_t end) const {                                                              \
		return Object(inherit(DeeObject_CallAttrStringf(*this, #name, DEE_PCKuSIZ DEE_PCKuSIZ, start, end))).bool_(); \
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
#define DEFINE_CHARACTER_TRAIT(name)                                                                                  \
	WUNUSED bool(name)() const {                                                                                      \
		return Object(inherit(DeeObject_CallAttrString(*this, #name, 0, NULL))).bool_();                              \
	}                                                                                                                 \
	WUNUSED bool(name)(size_t start) const {                                                                          \
		return Object(inherit(DeeObject_CallAttrStringf(*this, #name, DEE_PCKuSIZ, start))).bool_();                  \
	}                                                                                                                 \
	WUNUSED bool(name)(size_t start, size_t end) const {                                                              \
		return Object(inherit(DeeObject_CallAttrStringf(*this, #name, DEE_PCKuSIZ DEE_PCKuSIZ, start, end))).bool_(); \
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
#define DEFINE_STRING_TRANSFORMATION(name)                                                            \
	WUNUSED Bytes(name)() const {                                                                     \
		return inherit(DeeObject_CallAttrString(*this, #name, 0, NULL));                              \
	}                                                                                                 \
	WUNUSED Bytes(name)(size_t start) const {                                                         \
		return inherit(DeeObject_CallAttrStringf(*this, #name, DEE_PCKuSIZ, start));                  \
	}                                                                                                 \
	WUNUSED Bytes(name)(size_t start, size_t end) const {                                             \
		return inherit(DeeObject_CallAttrStringf(*this, #name, DEE_PCKuSIZ DEE_PCKuSIZ, start, end)); \
	}
	DEFINE_STRING_TRANSFORMATION(lower)
	DEFINE_STRING_TRANSFORMATION(upper)
	DEFINE_STRING_TRANSFORMATION(title)
	DEFINE_STRING_TRANSFORMATION(capitalize)
	DEFINE_STRING_TRANSFORMATION(swapcase)
	DEFINE_STRING_TRANSFORMATION(casefold)
	DEFINE_STRING_TRANSFORMATION(reversed)
#undef DEFINE_STRING_TRANSFORMATION
	WUNUSED deemon::int_ asnumber() const {
		return inherit(DeeObject_CallAttrString(*this, "asnumber", 0, NULL));
	}
	WUNUSED deemon::int_ asnumber(size_t index) const {
		return inherit(DeeObject_CallAttrStringf(*this, "asnumber", DEE_PCKuSIZ, index));
	}
	WUNUSED deemon::int_ asnumber(size_t index, int defl) const {
		return inherit(DeeObject_CallAttrStringf(*this, "asnumber", DEE_PCKuSIZ "d", index, defl));
	}
	WUNUSED deemon::int_ asnumber(DeeObject *__restrict index) const {
		return inherit(DeeObject_CallAttrStringf(*this, "asnumber", "o", index));
	}
	WUNUSED deemon::int_ asnumber(DeeObject *__restrict index, int defl) const {
		return inherit(DeeObject_CallAttrStringf(*this, "asnumber", "od", index, defl));
	}
	WUNUSED deemon::int_ asdigit() const {
		return inherit(DeeObject_CallAttrString(*this, "asdigit", 0, NULL));
	}
	WUNUSED deemon::int_ asdigit(size_t index) const {
		return inherit(DeeObject_CallAttrStringf(*this, "asdigit", DEE_PCKuSIZ, index));
	}
	WUNUSED deemon::int_ asdigit(size_t index, int defl) const {
		return inherit(DeeObject_CallAttrStringf(*this, "asdigit", DEE_PCKuSIZ "d", index, defl));
	}
	WUNUSED deemon::int_ asdigit(DeeObject *__restrict index) const {
		return inherit(DeeObject_CallAttrStringf(*this, "asdigit", "o", index));
	}
	WUNUSED deemon::int_ asdigit(DeeObject *__restrict index, int defl) const {
		return inherit(DeeObject_CallAttrStringf(*this, "asdigit", "od", index, defl));
	}
	WUNUSED deemon::int_ asdecimal() const {
		return inherit(DeeObject_CallAttrString(*this, "asdecimal", 0, NULL));
	}
	WUNUSED deemon::int_ asdecimal(size_t index) const {
		return inherit(DeeObject_CallAttrStringf(*this, "asdecimal", DEE_PCKuSIZ, index));
	}
	WUNUSED deemon::int_ asdecimal(size_t index, int defl) const {
		return inherit(DeeObject_CallAttrStringf(*this, "asdecimal", DEE_PCKuSIZ "d", index, defl));
	}
	WUNUSED deemon::int_ asdecimal(DeeObject *__restrict index) const {
		return inherit(DeeObject_CallAttrStringf(*this, "asdecimal", "o", index));
	}
	WUNUSED deemon::int_ asdecimal(DeeObject *__restrict index, int defl) const {
		return inherit(DeeObject_CallAttrStringf(*this, "asdecimal", "od", index, defl));
	}
	WUNUSED Object asnumber(size_t index, DeeObject *defl) const {
		return inherit(DeeObject_CallAttrStringf(*this, "asnumber", DEE_PCKuSIZ "o", index, defl));
	}
	WUNUSED Object asnumber(DeeObject *index, DeeObject *defl) const {
		return inherit(DeeObject_CallAttrStringf(*this, "asnumber", "oo", index, defl));
	}
	WUNUSED Object asdigit(size_t index, DeeObject *defl) const {
		return inherit(DeeObject_CallAttrStringf(*this, "asdigit", DEE_PCKuSIZ "o", index, defl));
	}
	WUNUSED Object asdigit(DeeObject *index, DeeObject *defl) const {
		return inherit(DeeObject_CallAttrStringf(*this, "asdigit", "oo", index, defl));
	}
	WUNUSED Object asdecimal(size_t index, DeeObject *defl) const {
		return inherit(DeeObject_CallAttrStringf(*this, "asdecimal", DEE_PCKuSIZ "o", index, defl));
	}
	WUNUSED Object asdecimal(DeeObject *index, DeeObject *defl) const {
		return inherit(DeeObject_CallAttrStringf(*this, "asdecimal", "oo", index, defl));
	}
#define DEFINE_REPLACE_FUNCTION(name)                                                                                          \
	WUNUSED Bytes(name)(DeeObject * find, DeeObject * repl) const {                                                            \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "oo", find, repl));                                             \
	}                                                                                                                          \
	WUNUSED Bytes(name)(DeeObject * find, DeeObject * repl, size_t maxcount) const {                                           \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "oo" DEE_PCKuSIZ, find, repl, maxcount));                       \
	}                                                                                                                          \
	WUNUSED Bytes(name)(DeeObject * find, /*utf-8*/ char const *__restrict repl) const {                                       \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "os", find, repl));                                             \
	}                                                                                                                          \
	WUNUSED Bytes(name)(DeeObject * find, /*utf-8*/ char const *__restrict repl, size_t maxcount) const {                      \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "os" DEE_PCKuSIZ, find, repl, maxcount));                       \
	}                                                                                                                          \
	WUNUSED Bytes(name)(/*utf-8*/ char const *__restrict find, DeeObject *repl) const {                                        \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "so", find, repl));                                             \
	}                                                                                                                          \
	WUNUSED Bytes(name)(/*utf-8*/ char const *__restrict find, DeeObject *repl, size_t maxcount) const {                       \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "so" DEE_PCKuSIZ, find, repl, maxcount));                       \
	}                                                                                                                          \
	WUNUSED Bytes(name)(/*utf-8*/ char const *__restrict find, /*utf-8*/ char const *__restrict repl) const {                  \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "ss", find, repl));                                             \
	}                                                                                                                          \
	WUNUSED Bytes(name)(/*utf-8*/ char const *__restrict find, /*utf-8*/ char const *__restrict repl, size_t maxcount) const { \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "ss" DEE_PCKuSIZ, find, repl, maxcount));                       \
	}
	DEFINE_REPLACE_FUNCTION(replace)
	DEFINE_REPLACE_FUNCTION(casereplace)
#undef DEFINE_REPLACE_FUNCTION
#define DEFINE_FIND_FUNCTION(Treturn, name, needle)                                                               \
	inline WUNUSED Treturn(name)(/*utf-8*/ char const *__restrict needle) const {                                 \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "s", needle));                                     \
	}                                                                                                             \
	inline WUNUSED Treturn(name)(/*utf-8*/ char const *__restrict needle, size_t start) const {                   \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "s" DEE_PCKuSIZ, needle, start));                  \
	}                                                                                                             \
	inline WUNUSED Treturn(name)(/*utf-8*/ char const *__restrict needle, size_t start, size_t end) const {       \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "s" DEE_PCKuSIZ DEE_PCKuSIZ, needle, start, end)); \
	}                                                                                                             \
	inline WUNUSED Treturn(name)(DeeObject *__restrict needle) const {                                            \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "o", needle));                                     \
	}                                                                                                             \
	inline WUNUSED Treturn(name)(DeeObject *__restrict needle, size_t start) const {                              \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "o" DEE_PCKuSIZ, needle, start));                  \
	}                                                                                                             \
	inline WUNUSED Treturn(name)(DeeObject *__restrict needle, size_t start, size_t end) const {                  \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "o" DEE_PCKuSIZ DEE_PCKuSIZ, needle, start, end)); \
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
#define DEFINE_FIND_FUNCTION(Treturn, name, needle)                                                               \
	WUNUSED Treturn(name)(/*utf-8*/ char const *__restrict needle) const {                                        \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "s", needle));                                     \
	}                                                                                                             \
	WUNUSED Treturn(name)(/*utf-8*/ char const *__restrict needle, size_t start) const {                          \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "s" DEE_PCKuSIZ, needle, start));                  \
	}                                                                                                             \
	WUNUSED Treturn(name)(/*utf-8*/ char const *__restrict needle, size_t start, size_t end) const {              \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "s" DEE_PCKuSIZ DEE_PCKuSIZ, needle, start, end)); \
	}                                                                                                             \
	WUNUSED Treturn(name)(DeeObject *needle) const {                                                              \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "o", needle));                                     \
	}                                                                                                             \
	WUNUSED Treturn(name)(DeeObject *needle, size_t start) const {                                                \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "o" DEE_PCKuSIZ, needle, start));                  \
	}                                                                                                             \
	WUNUSED Treturn(name)(DeeObject *needle, size_t start, size_t end) const {                                    \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "o" DEE_PCKuSIZ DEE_PCKuSIZ, needle, start, end)); \
	}
	DEFINE_FIND_FUNCTION(deemon::bool_, contains, needle)
	DEFINE_FIND_FUNCTION(deemon::bool_, startswith, needle)
	DEFINE_FIND_FUNCTION(deemon::bool_, endswith, needle)
	DEFINE_FIND_FUNCTION(Tuple<Bytes>, partition, needle)
	DEFINE_FIND_FUNCTION(Tuple<Bytes>, rpartition, needle)
	DEFINE_FIND_FUNCTION(deemon::bool_, casecontains, needle)
	DEFINE_FIND_FUNCTION(deemon::bool_, casestartswith, needle)
	DEFINE_FIND_FUNCTION(deemon::bool_, caseendswith, needle)
	DEFINE_FIND_FUNCTION(Tuple<Bytes>, casepartition, needle)
	DEFINE_FIND_FUNCTION(Tuple<Bytes>, caserpartition, needle)
#undef DEFINE_FIND_FUNCTION
	WUNUSED Bytes substr(size_t start) const {
		return inherit(DeeObject_CallAttrStringf(*this, "substr", DEE_PCKuSIZ, start));
	}
	WUNUSED Bytes substr(size_t start, size_t end) const {
		return inherit(DeeObject_CallAttrStringf(*this, "substr", DEE_PCKuSIZ DEE_PCKuSIZ, start, end));
	}
#define DEFINE_STRIP_FUNCTION(name)                                                      \
	WUNUSED Bytes(name)() const {                                                        \
		return inherit(DeeObject_CallAttrString(*this, #name, 0, NULL));                 \
	}                                                                                    \
	WUNUSED Bytes(name)(/*utf-8*/ char const *__restrict mask) const {                   \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "s", mask));              \
	}                                                                                    \
	WUNUSED Bytes(name)(/*utf-8*/ char const *__restrict mask, size_t mask_size) const { \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "$s", mask_size, mask));  \
	}                                                                                    \
	WUNUSED Bytes(name)(DeeObject *mask) const {                                         \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "o", mask));              \
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
#define DEFINE_COMPARE_FUNCTION(Treturn, name)                                                                                                                         \
	WUNUSED Treturn(name)(/*utf-8*/ char const *__restrict other) const {                                                                                              \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "s", other));                                                                                           \
	}                                                                                                                                                                  \
	WUNUSED Treturn(name)(/*utf-8*/ char const *__restrict other, size_t other_size) const {                                                                           \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "$s", other_size, other));                                                                              \
	}                                                                                                                                                                  \
	WUNUSED Treturn(name)(size_t my_start, /*utf-8*/ char const *__restrict other) const {                                                                             \
		return inherit(DeeObject_CallAttrStringf(*this, #name, DEE_PCKuSIZ "s", my_start, other));                                                                     \
	}                                                                                                                                                                  \
	WUNUSED Treturn(name)(size_t my_start, /*utf-8*/ char const *__restrict other, size_t other_size) const {                                                          \
		return inherit(DeeObject_CallAttrStringf(*this, #name, DEE_PCKuSIZ "$s", my_start, other_size, other));                                                        \
	}                                                                                                                                                                  \
	WUNUSED Treturn(name)(size_t my_start, size_t my_end, /*utf-8*/ char const *__restrict other) const {                                                              \
		return inherit(DeeObject_CallAttrStringf(*this, #name, DEE_PCKuSIZ DEE_PCKuSIZ "s", my_start, my_end, other));                                                 \
	}                                                                                                                                                                  \
	WUNUSED Treturn(name)(size_t my_start, size_t my_end, /*utf-8*/ char const *__restrict other, size_t other_size) const {                                           \
		return inherit(DeeObject_CallAttrStringf(*this, #name, DEE_PCKuSIZ DEE_PCKuSIZ "$s", my_start, my_end, other_size, other));                                    \
	}                                                                                                                                                                  \
	WUNUSED Treturn(name)(DeeObject *__restrict other) const {                                                                                                         \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "o", other));                                                                                           \
	}                                                                                                                                                                  \
	WUNUSED Treturn(name)(DeeObject *__restrict other, size_t other_start) const {                                                                                     \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "o" DEE_PCKuSIZ, other, other_start));                                                                  \
	}                                                                                                                                                                  \
	WUNUSED Treturn(name)(DeeObject *__restrict other, size_t other_start, size_t other_end) const {                                                                   \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "o" DEE_PCKuSIZ DEE_PCKuSIZ, other, other_start, other_end));                                           \
	}                                                                                                                                                                  \
	WUNUSED Treturn(name)(size_t my_start, DeeObject *__restrict other) const {                                                                                        \
		return inherit(DeeObject_CallAttrStringf(*this, #name, DEE_PCKuSIZ "o", my_start, other));                                                                     \
	}                                                                                                                                                                  \
	WUNUSED Treturn(name)(size_t my_start, DeeObject *__restrict other, size_t other_start) const {                                                                    \
		return inherit(DeeObject_CallAttrStringf(*this, #name, DEE_PCKuSIZ "o" DEE_PCKuSIZ, my_start, other, other_start));                                            \
	}                                                                                                                                                                  \
	WUNUSED Treturn(name)(size_t my_start, DeeObject *__restrict other, size_t other_start, size_t other_end) const {                                                  \
		return inherit(DeeObject_CallAttrStringf(*this, #name, DEE_PCKuSIZ "o" DEE_PCKuSIZ DEE_PCKuSIZ, my_start, other, other_start, other_end));                     \
	}                                                                                                                                                                  \
	WUNUSED Treturn(name)(size_t my_start, size_t my_end, DeeObject *__restrict other) const {                                                                         \
		return inherit(DeeObject_CallAttrStringf(*this, #name, DEE_PCKuSIZ DEE_PCKuSIZ "o", my_start, my_end, other));                                                 \
	}                                                                                                                                                                  \
	WUNUSED Treturn(name)(size_t my_start, size_t my_end, DeeObject *__restrict other, size_t other_start) const {                                                     \
		return inherit(DeeObject_CallAttrStringf(*this, #name, DEE_PCKuSIZ DEE_PCKuSIZ "o" DEE_PCKuSIZ, my_start, my_end, other, other_start));                        \
	}                                                                                                                                                                  \
	WUNUSED Treturn(name)(size_t my_start, size_t my_end, DeeObject *__restrict other, size_t other_start, size_t other_end) const {                                   \
		return inherit(DeeObject_CallAttrStringf(*this, #name, DEE_PCKuSIZ DEE_PCKuSIZ "o" DEE_PCKuSIZ DEE_PCKuSIZ, my_start, my_end, other, other_start, other_end)); \
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
#define DEFINE_COMPARE_FUNCTION(Treturn, name)                                                                                                                         \
	WUNUSED Treturn(name)(/*utf-8*/ char const *__restrict other) const {                                                                                              \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "s", other));                                                                                           \
	}                                                                                                                                                                  \
	WUNUSED Treturn(name)(/*utf-8*/ char const *__restrict other, size_t other_size) const {                                                                           \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "$s", other_size, other));                                                                              \
	}                                                                                                                                                                  \
	WUNUSED Treturn(name)(size_t my_start, /*utf-8*/ char const *__restrict other) const {                                                                             \
		return inherit(DeeObject_CallAttrStringf(*this, #name, DEE_PCKuSIZ "s", my_start, other));                                                                     \
	}                                                                                                                                                                  \
	WUNUSED Treturn(name)(size_t my_start, /*utf-8*/ char const *__restrict other, size_t other_size) const {                                                          \
		return inherit(DeeObject_CallAttrStringf(*this, #name, DEE_PCKuSIZ "$s", my_start, other_size, other));                                                        \
	}                                                                                                                                                                  \
	WUNUSED Treturn(name)(size_t my_start, size_t my_end, /*utf-8*/ char const *__restrict other) const {                                                              \
		return inherit(DeeObject_CallAttrStringf(*this, #name, DEE_PCKuSIZ DEE_PCKuSIZ "s", my_start, my_end, other));                                                 \
	}                                                                                                                                                                  \
	WUNUSED Treturn(name)(size_t my_start, size_t my_end, /*utf-8*/ char const *__restrict other, size_t other_size) const {                                           \
		return inherit(DeeObject_CallAttrStringf(*this, #name, DEE_PCKuSIZ DEE_PCKuSIZ "$s", my_start, my_end, other_size, other));                                    \
	}                                                                                                                                                                  \
	WUNUSED Treturn(name)(DeeObject *other) const {                                                                                                                    \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "o", other));                                                                                           \
	}                                                                                                                                                                  \
	WUNUSED Treturn(name)(DeeObject *other, size_t other_start) const {                                                                                                \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "o" DEE_PCKuSIZ, other, other_start));                                                                  \
	}                                                                                                                                                                  \
	WUNUSED Treturn(name)(DeeObject *other, size_t other_start, size_t other_end) const {                                                                              \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "o" DEE_PCKuSIZ DEE_PCKuSIZ, other, other_start, other_end));                                           \
	}                                                                                                                                                                  \
	WUNUSED Treturn(name)(size_t my_start, DeeObject *other) const {                                                                                                   \
		return inherit(DeeObject_CallAttrStringf(*this, #name, DEE_PCKuSIZ "o", my_start, other));                                                                     \
	}                                                                                                                                                                  \
	WUNUSED Treturn(name)(size_t my_start, DeeObject *other, size_t other_start) const {                                                                               \
		return inherit(DeeObject_CallAttrStringf(*this, #name, DEE_PCKuSIZ "o" DEE_PCKuSIZ, my_start, other, other_start));                                            \
	}                                                                                                                                                                  \
	WUNUSED Treturn(name)(size_t my_start, DeeObject *other, size_t other_start, size_t other_end) const {                                                             \
		return inherit(DeeObject_CallAttrStringf(*this, #name, DEE_PCKuSIZ "o" DEE_PCKuSIZ DEE_PCKuSIZ, my_start, other, other_start, other_end));                     \
	}                                                                                                                                                                  \
	WUNUSED Treturn(name)(size_t my_start, size_t my_end, DeeObject *other) const {                                                                                    \
		return inherit(DeeObject_CallAttrStringf(*this, #name, DEE_PCKuSIZ DEE_PCKuSIZ "o", my_start, my_end, other));                                                 \
	}                                                                                                                                                                  \
	WUNUSED Treturn(name)(size_t my_start, size_t my_end, DeeObject *other, size_t other_start) const {                                                                \
		return inherit(DeeObject_CallAttrStringf(*this, #name, DEE_PCKuSIZ DEE_PCKuSIZ "o" DEE_PCKuSIZ, my_start, my_end, other, other_start));                        \
	}                                                                                                                                                                  \
	WUNUSED Treturn(name)(size_t my_start, size_t my_end, DeeObject *other, size_t other_start, size_t other_end) const {                                              \
		return inherit(DeeObject_CallAttrStringf(*this, #name, DEE_PCKuSIZ DEE_PCKuSIZ "o" DEE_PCKuSIZ DEE_PCKuSIZ, my_start, my_end, other, other_start, other_end)); \
	}
	DEFINE_COMPARE_FUNCTION(deemon::bool_, wmatch)
	DEFINE_COMPARE_FUNCTION(deemon::bool_, casewmatch)
#undef DEFINE_COMPARE_FUNCTION
#define DEFINE_CENTER_FUNCTION(name)                                                                           \
	WUNUSED Bytes(name)(size_t width) const {                                                                  \
		return inherit(DeeObject_CallAttrStringf(*this, #name, DEE_PCKuSIZ, width));                           \
	}                                                                                                          \
	WUNUSED Bytes(name)(size_t width, DeeObject *filler) const {                                               \
		return inherit(DeeObject_CallAttrStringf(*this, #name, DEE_PCKuSIZ "o", width, filler));               \
	}                                                                                                          \
	WUNUSED Bytes(name)(size_t width, /*utf-8*/ char const *__restrict filler) const {                         \
		return inherit(DeeObject_CallAttrStringf(*this, #name, DEE_PCKuSIZ "s", width, filler));               \
	}                                                                                                          \
	WUNUSED Bytes(name)(size_t width, /*utf-8*/ char const *__restrict filler, size_t filler_size) const {     \
		return inherit(DeeObject_CallAttrStringf(*this, #name, DEE_PCKuSIZ "$s", width, filler_size, filler)); \
	}
	DEFINE_CENTER_FUNCTION(center)
	DEFINE_CENTER_FUNCTION(ljust)
	DEFINE_CENTER_FUNCTION(rjust)
	DEFINE_CENTER_FUNCTION(zfill)
#undef DEFINE_CENTER_FUNCTION
	WUNUSED Bytes expandtabs() const {
		return inherit(DeeObject_CallAttrString(*this, "expandtabs", 0, NULL));
	}
	WUNUSED Bytes expandtabs(size_t tab_width) const {
		return inherit(DeeObject_CallAttrStringf(*this, "expandtabs", DEE_PCKuSIZ, tab_width));
	}
#define DEFINE_UNIFYLINES(name, replacement)                                                           \
	WUNUSED Bytes(name)() const {                                                                      \
		return inherit(DeeObject_CallAttrString(*this, #name, 0, NULL));                               \
	}                                                                                                  \
	WUNUSED Bytes(name)(DeeObject *replacement) const {                                                \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "o", replacement));                     \
	}                                                                                                  \
	WUNUSED Bytes(name)(/*utf-8*/ char const *__restrict replacement) const {                          \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "s", replacement));                     \
	}                                                                                                  \
	WUNUSED Bytes(name)(/*utf-8*/ char const *__restrict replacement, size_t replacement_size) const { \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "$s", replacement_size, replacement));  \
	}
	DEFINE_UNIFYLINES(unifylines, replacement)
	DEFINE_UNIFYLINES(indent, filler)
#undef DEFINE_UNIFYLINES
	WUNUSED Bytes join(DeeObject *seq) const {
		return inherit(DeeObject_CallAttrStringf(*this, "join", "o", seq));
	}
	WUNUSED Bytes join(std::initializer_list<DeeObject *> const &seq) const {
		Tuple<Object> seq_obj(seq);
		return inherit(DeeObject_CallAttrStringf(*this, "join", "o", seq_obj.ptr()));
	}
#define DEFINE_SPLIT_FUNCTION(name)                                                              \
	WUNUSED Sequence<Bytes>(name)(DeeObject *sep) const {                                        \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "o", sep));                       \
	}                                                                                            \
	WUNUSED Sequence<Bytes>(name)(/*utf-8*/ char const *__restrict sep) const {                  \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "s", sep));                       \
	}                                                                                            \
	WUNUSED Sequence<Bytes>(name)(/*utf-8*/ char const *__restrict sep, size_t sep_size) const { \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "$s", sep_size, sep));            \
	}
	DEFINE_SPLIT_FUNCTION(split)
	DEFINE_SPLIT_FUNCTION(casesplit)
#undef DEFINE_SPLIT_FUNCTION
	WUNUSED Sequence<Bytes> splitlines() const {
		return inherit(DeeObject_CallAttrString(*this, "splitlines", 0, NULL));
	}
	WUNUSED Sequence<Bytes> splitlines(bool keepends) const {
		return inherit(DeeObject_CallAttrStringf(*this, "join", "b", keepends));
	}
	WUNUSED Bytes dedent() const {
		return inherit(DeeObject_CallAttrString(*this, "dedent", 0, NULL));
	}
	WUNUSED Bytes dedent(size_t max_chars) const {
		return inherit(DeeObject_CallAttrStringf(*this, "dedent", DEE_PCKuSIZ, max_chars));
	}
	WUNUSED Bytes dedent(size_t max_chars, DeeObject *mask) const {
		return inherit(DeeObject_CallAttrStringf(*this, "dedent", DEE_PCKuSIZ "o", max_chars, mask));
	}
	WUNUSED Bytes dedent(size_t max_chars, /*utf-8*/ char const *__restrict mask) const {
		return inherit(DeeObject_CallAttrStringf(*this, "dedent", DEE_PCKuSIZ "s", max_chars, mask));
	}
	WUNUSED Bytes dedent(size_t max_chars, /*utf-8*/ char const *__restrict mask, size_t mask_size) const {
		return inherit(DeeObject_CallAttrStringf(*this, "dedent", DEE_PCKuSIZ "$s", max_chars, mask_size, mask));
	}
#define DEFINE_FINDMATCH_FUNCTION(Treturn, name)                                                                                           \
	WUNUSED Treturn(name)(DeeObject *__restrict open, DeeObject *__restrict close) const {                                                 \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "oo", open, close));                                                        \
	}                                                                                                                                      \
	WUNUSED Treturn(name)(DeeObject *__restrict open, DeeObject *__restrict close, size_t start) const {                                   \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "oo" DEE_PCKuSIZ, open, close, start));                                     \
	}                                                                                                                                      \
	WUNUSED Treturn(name)(DeeObject *__restrict open, DeeObject *__restrict close, size_t start, size_t end) const {                       \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "oo" DEE_PCKuSIZ DEE_PCKuSIZ, open, close, start, end));                    \
	}                                                                                                                                      \
	WUNUSED Treturn(name)(DeeObject *__restrict open, /*utf-8*/ char const *__restrict close) const {                                      \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "os", open, close));                                                        \
	}                                                                                                                                      \
	WUNUSED Treturn(name)(DeeObject *__restrict open, /*utf-8*/ char const *__restrict close, size_t start) const {                        \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "os" DEE_PCKuSIZ, open, close, start));                                     \
	}                                                                                                                                      \
	WUNUSED Treturn(name)(DeeObject *__restrict open, /*utf-8*/ char const *__restrict close, size_t start, size_t end) const {            \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "os" DEE_PCKuSIZ DEE_PCKuSIZ, open, close, start, end));                    \
	}                                                                                                                                      \
	WUNUSED Treturn(name)(/*utf-8*/ char const *__restrict open, DeeObject *__restrict close) const {                                      \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "so", open, close));                                                        \
	}                                                                                                                                      \
	WUNUSED Treturn(name)(/*utf-8*/ char const *__restrict open, DeeObject *__restrict close, size_t start) const {                        \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "so" DEE_PCKuSIZ, open, close, start));                                     \
	}                                                                                                                                      \
	WUNUSED Treturn(name)(/*utf-8*/ char const *__restrict open, DeeObject *__restrict close, size_t start, size_t end) const {            \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "so" DEE_PCKuSIZ DEE_PCKuSIZ, open, close, start, end));                    \
	}                                                                                                                                      \
	WUNUSED Treturn(name)(/*utf-8*/ char const *__restrict open, /*utf-8*/ char const *__restrict close) const {                           \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "ss", open, close));                                                        \
	}                                                                                                                                      \
	WUNUSED Treturn(name)(/*utf-8*/ char const *__restrict open, /*utf-8*/ char const *__restrict close, size_t start) const {             \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "ss" DEE_PCKuSIZ, open, close, start));                                     \
	}                                                                                                                                      \
	WUNUSED Treturn(name)(/*utf-8*/ char const *__restrict open, /*utf-8*/ char const *__restrict close, size_t start, size_t end) const { \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "ss" DEE_PCKuSIZ DEE_PCKuSIZ, open, close, start, end));                    \
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
#define DEFINE_FINDMATCH_FUNCTION(Treturn, name)                                                                                           \
	WUNUSED Treturn(name)(DeeObject *open, DeeObject *close) const {                                                                       \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "oo", open, close));                                                        \
	}                                                                                                                                      \
	WUNUSED Treturn(name)(DeeObject *open, DeeObject *close, size_t start) const {                                                         \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "oo" DEE_PCKuSIZ, open, close, start));                                     \
	}                                                                                                                                      \
	WUNUSED Treturn(name)(DeeObject *open, DeeObject *close, size_t start, size_t end) const {                                             \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "oo" DEE_PCKuSIZ DEE_PCKuSIZ, open, close, start, end));                    \
	}                                                                                                                                      \
	WUNUSED Treturn(name)(DeeObject *open, /*utf-8*/ char const *__restrict close) const {                                                 \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "os", open, close));                                                        \
	}                                                                                                                                      \
	WUNUSED Treturn(name)(DeeObject *open, /*utf-8*/ char const *__restrict close, size_t start) const {                                   \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "os" DEE_PCKuSIZ, open, close, start));                                     \
	}                                                                                                                                      \
	WUNUSED Treturn(name)(DeeObject *open, /*utf-8*/ char const *__restrict close, size_t start, size_t end) const {                       \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "os" DEE_PCKuSIZ DEE_PCKuSIZ, open, close, start, end));                    \
	}                                                                                                                                      \
	WUNUSED Treturn(name)(/*utf-8*/ char const *__restrict open, DeeObject *close) const {                                                 \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "so", open, close));                                                        \
	}                                                                                                                                      \
	WUNUSED Treturn(name)(/*utf-8*/ char const *__restrict open, DeeObject *close, size_t start) const {                                   \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "so" DEE_PCKuSIZ, open, close, start));                                     \
	}                                                                                                                                      \
	WUNUSED Treturn(name)(/*utf-8*/ char const *__restrict open, DeeObject *close, size_t start, size_t end) const {                       \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "so" DEE_PCKuSIZ DEE_PCKuSIZ, open, close, start, end));                    \
	}                                                                                                                                      \
	WUNUSED Treturn(name)(/*utf-8*/ char const *__restrict open, /*utf-8*/ char const *__restrict close) const {                           \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "ss", open, close));                                                        \
	}                                                                                                                                      \
	WUNUSED Treturn(name)(/*utf-8*/ char const *__restrict open, /*utf-8*/ char const *__restrict close, size_t start) const {             \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "ss" DEE_PCKuSIZ, open, close, start));                                     \
	}                                                                                                                                      \
	WUNUSED Treturn(name)(/*utf-8*/ char const *__restrict open, /*utf-8*/ char const *__restrict close, size_t start, size_t end) const { \
		return inherit(DeeObject_CallAttrStringf(*this, #name, "ss" DEE_PCKuSIZ DEE_PCKuSIZ, open, close, start, end));                    \
	}
	DEFINE_FINDMATCH_FUNCTION(Sequence<Bytes>, partitionmatch)
	DEFINE_FINDMATCH_FUNCTION(Sequence<Bytes>, rpartitionmatch)
	DEFINE_FINDMATCH_FUNCTION(Sequence<Bytes>, casepartitionmatch)
	DEFINE_FINDMATCH_FUNCTION(Sequence<Bytes>, caserpartitionmatch)
#undef DEFINE_FINDMATCH_FUNCTION
	WUNUSED Sequence<Bytes> segments(size_t substring_length) const {
		return inherit(DeeObject_CallAttrStringf(*this, "segments", DEE_PCKuSIZ, substring_length));
	}
	WUNUSED Sequence<Bytes> segments(DeeObject *substring_length) const {
		return inherit(DeeObject_CallAttrStringf(*this, "segments", "o", substring_length));
	}
	WUNUSED Sequence<Bytes> distribute(size_t substring_count) const {
		return inherit(DeeObject_CallAttrStringf(*this, "distribute", DEE_PCKuSIZ, substring_count));
	}
	WUNUSED Sequence<Bytes> distribute(DeeObject *substring_count) const {
		return inherit(DeeObject_CallAttrStringf(*this, "distribute", "o", substring_count));
	}
public:
	WUNUSED Bytes add(DeeObject *right) const {
		return inherit(DeeObject_Add(*this, right));
	}
	WUNUSED Bytes operator+(DeeObject *right) const {
		return inherit(DeeObject_Add(*this, right));
	}
	WUNUSED Bytes &inplace_add(DeeObject *right) {
		Sequence::inplace_add(right);
		return *this;
	}
	WUNUSED Bytes &operator+=(DeeObject *right) {
		Sequence::operator+=(right);
		return *this;
	}
	WUNUSED Bytes mul(DeeObject *n) const {
		return inherit(DeeObject_Mul(*this, n));
	}
	WUNUSED Bytes mul(int8_t n) const {
		return inherit(DeeObject_MulInt(*this, n));
	}
	WUNUSED Bytes mul(size_t n) const {
		return inherit(_mulref(n));
	}
	WUNUSED Bytes operator*(DeeObject *n) const {
		return inherit(DeeObject_Mul(*this, n));
	}
	WUNUSED Bytes operator*(int8_t n) const {
		return inherit(DeeObject_MulInt(*this, n));
	}
	WUNUSED Bytes operator*(size_t n) const {
		return inherit(_mulref(n));
	}
	WUNUSED Bytes &inplace_mul(DeeObject *n) {
		Sequence::inplace_mul(n);
		return *this;
	}
	WUNUSED Bytes &inplace_mul(int8_t n) {
		Sequence::inplace_mul(n);
		return *this;
	}
	WUNUSED Bytes &inplace_mul(size_t n) {
		Sequence::inplace_mul(n);
		return *this;
	}
	WUNUSED Bytes &operator*=(DeeObject *n) {
		Sequence::inplace_mul(n);
		return *this;
	}
	WUNUSED Bytes &operator*=(int8_t n) {
		Sequence::inplace_mul(n);
		return *this;
	}
	WUNUSED Bytes &operator*=(size_t n) {
		Sequence::inplace_mul(n);
		return *this;
	}
};



#ifdef GUARD_DEEMON_CXX_FILE_H
inline Bytes(File::readline)(size_t max_length, bool keep_lf) const {
	return inherit(DeeFile_ReadLine(*this, max_length, keep_lf));
}
inline Bytes(File::read)(size_t max_length, bool readall) const {
	return inherit(DeeFile_ReadText(*this, max_length, readall));
}
inline Bytes(File::pread)(Dee_pos_t pos, size_t max_length, bool readall) const {
	return inherit(DeeFile_PReadText(*this, max_length, pos, readall));
}
#endif /* GUARD_DEEMON_CXX_FILE_H */

#ifdef GUARD_DEEMON_CXX_STRING_H
inline WUNUSED Bytes(string::bytes)() const {
	return inherit(DeeObject_CallAttrString(*this, "bytes", 0, NULL));
}
inline WUNUSED Bytes(string::bytes)(bool allow_invalid) const {
	return inherit(DeeObject_CallAttrStringf(*this, "bytes", "b", allow_invalid));
}
inline WUNUSED Bytes(string::bytes)(size_t start, size_t end) const {
	return inherit(DeeObject_CallAttrStringf(*this, "bytes", DEE_PCKuSIZ DEE_PCKuSIZ, start, end));
}
inline WUNUSED Bytes(string::bytes)(size_t start, size_t end, bool allow_invalid) const {
	return inherit(DeeObject_CallAttrStringf(*this, "bytes", DEE_PCKuSIZ DEE_PCKuSIZ "b", start, end, allow_invalid));
}
inline WUNUSED Bytes(string::encode)(/*utf-8*/ char const *__restrict codec_name) const {
	return inherit(DeeObject_CallAttrStringf(*this, "encode", "s", codec_name));
}
inline WUNUSED Bytes(string::encode)(obj_string codec_name) const {
	return inherit(DeeObject_CallAttrStringf(*this, "encode", "o", (DeeObject *)codec_name));
}
inline WUNUSED Bytes(string::encode)(/*utf-8*/ char const *__restrict codec_name, /*utf-8*/ char const *__restrict errors) const {
	return inherit(DeeObject_CallAttrStringf(*this, "encode", "ss", codec_name, errors));
}
inline WUNUSED Bytes(string::encode)(/*utf-8*/ char const *__restrict codec_name, obj_string errors) const {
	return inherit(DeeObject_CallAttrStringf(*this, "encode", "so", codec_name, (DeeObject *)errors));
}
inline WUNUSED Bytes(string::encode)(obj_string codec_name, /*utf-8*/ char const *__restrict errors) const {
	return inherit(DeeObject_CallAttrStringf(*this, "encode", "os", (DeeObject *)codec_name, errors));
}
inline WUNUSED Bytes(string::encode)(obj_string codec_name, obj_string errors) const {
	return inherit(DeeObject_CallAttrStringf(*this, "encode", "oo", (DeeObject *)codec_name, (DeeObject *)errors));
}
inline WUNUSED string(bytes::decode)(/*utf-8*/ char const *__restrict codec_name) const {
	return inherit(DeeObject_CallAttrStringf(*this, "decode", "s", codec_name));
}
inline WUNUSED string(bytes::decode)(obj_string codec_name) const {
	return inherit(DeeObject_CallAttrStringf(*this, "decode", "o", (DeeObject *)codec_name));
}
inline WUNUSED string(bytes::decode)(/*utf-8*/ char const *__restrict codec_name, /*utf-8*/ char const *__restrict errors) const {
	return inherit(DeeObject_CallAttrStringf(*this, "decode", "ss", codec_name, errors));
}
inline WUNUSED string(bytes::decode)(/*utf-8*/ char const *__restrict codec_name, obj_string errors) const {
	return inherit(DeeObject_CallAttrStringf(*this, "decode", "so", codec_name, (DeeObject *)errors));
}
inline WUNUSED string(bytes::decode)(obj_string codec_name, /*utf-8*/ char const *__restrict errors) const {
	return inherit(DeeObject_CallAttrStringf(*this, "decode", "os", (DeeObject *)codec_name, errors));
}
inline WUNUSED string(bytes::decode)(obj_string codec_name, obj_string errors) const {
	return inherit(DeeObject_CallAttrStringf(*this, "decode", "oo", (DeeObject *)codec_name, (DeeObject *)errors));
}
inline WUNUSED string(bytes::hex)() const {
	return inherit(DeeObject_CallAttrString(*this, "hex", 0, NULL));
}
inline WUNUSED string(bytes::hex)(size_t start) const {
	return inherit(DeeObject_CallAttrStringf(*this, "hex", DEE_PCKuSIZ, start));
}
inline WUNUSED string(bytes::hex)(size_t start, size_t end) const {
	return inherit(DeeObject_CallAttrStringf(*this, "hex", DEE_PCKuSIZ DEE_PCKuSIZ, start, end));
}
#endif /* GUARD_DEEMON_CXX_STRING_H */



DEE_CXX_END

#endif /* !GUARD_DEEMON_CXX_BYTES_H */
