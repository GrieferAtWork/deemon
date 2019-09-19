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
#ifndef GUARD_DEEMON_OBJECTS_UNICODE_STRING_FUNCTIONS_C
#define GUARD_DEEMON_OBJECTS_UNICODE_STRING_FUNCTIONS_C 1

#include "string_functions.h"

#include <deemon/alloc.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/bytes.h>
#include <deemon/error.h>
#include <deemon/int.h>
#include <deemon/none.h>
#include <deemon/seq.h>
#include <deemon/stringutils.h>
#include <deemon/thread.h>
#include <deemon/tuple.h>

#include <hybrid/byteswap.h>
#include <hybrid/overflow.h>

DECL_BEGIN

INTERN DREF DeeObject *DCALL
DeeString_StripSpc(DeeObject *__restrict self) {
	union dcharptr str, new_str;
	int width;
	size_t len, new_len;
	ASSERT_OBJECT_TYPE_EXACT(self, &DeeString_Type);
	width   = DeeString_WIDTH(self);
	str.ptr = new_str.ptr = DeeString_WSTR(self);
	len = new_len = WSTR_LENGTH(str.ptr);
	SWITCH_SIZEOF_WIDTH(width) {

	CASE_WIDTH_1BYTE:
		while (new_len && DeeUni_IsSpace(new_str.cp8[0]))
			++new_str.cp8, --new_len;
		while (new_len && DeeUni_IsSpace(new_str.cp8[new_len - 1]))
			--new_len;
		if (new_len == len)
			goto return_self;
		return DeeString_New1Byte(new_str.cp8, new_len);

	CASE_WIDTH_2BYTE:
		while (new_len && DeeUni_IsSpace(new_str.cp16[0]))
			++new_str.cp16, --new_len;
		while (new_len && DeeUni_IsSpace(new_str.cp16[new_len - 1]))
			--new_len;
		if (new_len == len)
			goto return_self;
		return DeeString_New2Byte(new_str.cp16, new_len);

	CASE_WIDTH_4BYTE:
		while (new_len && DeeUni_IsSpace(new_str.cp32[0]))
			++new_str.cp32, --new_len;
		while (new_len && DeeUni_IsSpace(new_str.cp32[new_len - 1]))
			--new_len;
		if (new_len == len)
			goto return_self;
		return DeeString_New4Byte(new_str.cp32, new_len);
	}
return_self:
	return_reference_(self);
}

INTERN DREF DeeObject *DCALL
DeeString_LStripSpc(DeeObject *__restrict self) {
	union dcharptr str, new_str;
	int width;
	size_t len, new_len;
	ASSERT_OBJECT_TYPE_EXACT(self, &DeeString_Type);
	width   = DeeString_WIDTH(self);
	str.ptr = new_str.ptr = DeeString_WSTR(self);
	len = new_len = WSTR_LENGTH(str.ptr);
	switch (width) {

	CASE_WIDTH_1BYTE:
		while (new_len && DeeUni_IsSpace(new_str.cp8[0]))
			++new_str.cp8, --new_len;
		if (new_len == len)
			goto return_self;
		return DeeString_New1Byte(new_str.cp8, new_len);

	CASE_WIDTH_2BYTE:
		while (new_len && DeeUni_IsSpace(new_str.cp16[0]))
			++new_str.cp16, --new_len;
		if (new_len == len)
			goto return_self;
		return DeeString_New2Byte(new_str.cp16, new_len);

	CASE_WIDTH_4BYTE:
		while (new_len && DeeUni_IsSpace(new_str.cp32[0]))
			++new_str.cp32, --new_len;
		if (new_len == len)
			goto return_self;
		return DeeString_New4Byte(new_str.cp32, new_len);
	}
return_self:
	return_reference_(self);
}

INTERN DREF DeeObject *DCALL
DeeString_RStripSpc(DeeObject *__restrict self) {
	union dcharptr str;
	int width;
	size_t len, new_len;
	ASSERT_OBJECT_TYPE_EXACT(self, &DeeString_Type);
	width   = DeeString_WIDTH(self);
	str.ptr = DeeString_WSTR(self);
	len = new_len = WSTR_LENGTH(str.ptr);
	SWITCH_SIZEOF_WIDTH(width) {

	CASE_WIDTH_1BYTE:
		while (new_len && DeeUni_IsSpace(str.cp8[new_len - 1]))
			--new_len;
		if (new_len == len)
			goto return_self;
		return DeeString_New1Byte(str.cp8, new_len);

	CASE_WIDTH_2BYTE:
		while (new_len && DeeUni_IsSpace(str.cp16[new_len - 1]))
			--new_len;
		if (new_len == len)
			goto return_self;
		return DeeString_New2Byte(str.cp16, new_len);

	CASE_WIDTH_4BYTE:
		while (new_len && DeeUni_IsSpace(str.cp32[new_len - 1]))
			--new_len;
		if (new_len == len)
			goto return_self;
		return DeeString_New4Byte(str.cp32, new_len);
	}
return_self:
	return_reference_(self);
}

INTERN DREF DeeObject *DCALL
DeeString_StripMask(DeeObject *__restrict self,
                    DeeObject *__restrict mask) {
	union dcharptr mystr, newstr, mskstr;
	size_t mylen, newlen, msklen;
	ASSERT_OBJECT_TYPE_EXACT(self, &DeeString_Type);
	ASSERT_OBJECT_TYPE_EXACT(mask, &DeeString_Type);
	SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON(DeeString_WIDTH(self),
	                                        DeeString_WIDTH(mask))) {

	CASE_WIDTH_1BYTE:
		mystr.cp8  = DeeString_As1Byte(self);
		mskstr.cp8 = DeeString_As1Byte(mask);
		mylen      = WSTR_LENGTH(mystr.cp8);
		msklen     = WSTR_LENGTH(mskstr.cp8);
		newstr.cp8 = mystr.cp8, newlen = mylen;
		while (newlen && memchrb(mskstr.cp8, newstr.cp8[0], msklen))
			++newstr.cp8, --newlen;
		while (newlen && memchrb(mskstr.cp8, newstr.cp8[newlen - 1], msklen))
			--newlen;
		if (newlen == mylen)
			goto retself;
		return DeeString_New1Byte(newstr.cp8, newlen);

	CASE_WIDTH_2BYTE:
		mystr.cp16 = DeeString_As2Byte(self);
		if unlikely(!mystr.cp16)
			goto err;
		mskstr.cp16 = DeeString_As2Byte(mask);
		if unlikely(!mskstr.cp16)
			goto err;
		mylen       = WSTR_LENGTH(mystr.cp16);
		msklen      = WSTR_LENGTH(mskstr.cp16);
		newstr.cp16 = mystr.cp16, newlen = mylen;
		while (newlen && memchrw(mskstr.cp16, newstr.cp16[0], msklen))
			++newstr.cp16, --newlen;
		while (newlen && memchrw(mskstr.cp16, newstr.cp16[newlen - 1], msklen))
			--newlen;
		if (newlen == mylen)
			goto retself;
		return DeeString_New2Byte(newstr.cp16, newlen);

	CASE_WIDTH_4BYTE:
		mystr.cp32 = DeeString_As4Byte(self);
		if unlikely(!mystr.cp32)
			goto err;
		mskstr.cp32 = DeeString_As4Byte(mask);
		if unlikely(!mskstr.cp32)
			goto err;
		mylen       = WSTR_LENGTH(mystr.cp32);
		msklen      = WSTR_LENGTH(mskstr.cp32);
		newstr.cp32 = mystr.cp32, newlen = mylen;
		while (newlen && memchrl(mskstr.cp32, newstr.cp32[0], msklen))
			++newstr.cp32, --newlen;
		while (newlen && memchrl(mskstr.cp32, newstr.cp32[newlen - 1], msklen))
			--newlen;
		if (newlen == mylen)
			goto retself;
		return DeeString_New4Byte(newstr.cp32, newlen);
	}
retself:
	return_reference_(self);
err:
	return NULL;
}

INTERN DREF DeeObject *DCALL
DeeString_LStripMask(DeeObject *__restrict self, DeeObject *__restrict mask) {
	union dcharptr mystr, newstr, mskstr;
	size_t mylen, newlen, msklen;
	ASSERT_OBJECT_TYPE_EXACT(self, &DeeString_Type);
	ASSERT_OBJECT_TYPE_EXACT(mask, &DeeString_Type);
	SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON(DeeString_WIDTH(self),
	                                        DeeString_WIDTH(mask))) {

	CASE_WIDTH_1BYTE:
		mystr.cp8  = DeeString_As1Byte(self);
		mskstr.cp8 = DeeString_As1Byte(mask);
		mylen      = WSTR_LENGTH(mystr.cp8);
		msklen     = WSTR_LENGTH(mskstr.cp8);
		newstr.cp8 = mystr.cp8, newlen = mylen;
		while (newlen && memchrb(mskstr.cp8, newstr.cp8[0], msklen))
			++newstr.cp8, --newlen;
		if (newlen == mylen)
			goto retself;
		return DeeString_New1Byte(newstr.cp8, newlen);

	CASE_WIDTH_2BYTE:
		mystr.cp16 = DeeString_As2Byte(self);
		if unlikely(!mystr.cp16)
			goto err;
		mskstr.cp16 = DeeString_As2Byte(mask);
		if unlikely(!mskstr.cp16)
			goto err;
		mylen       = WSTR_LENGTH(mystr.cp16);
		msklen      = WSTR_LENGTH(mskstr.cp16);
		newstr.cp16 = mystr.cp16, newlen = mylen;
		while (newlen && memchrw(mskstr.cp16, newstr.cp16[0], msklen))
			++newstr.cp16, --newlen;
		if (newlen == mylen)
			goto retself;
		return DeeString_New2Byte(newstr.cp16, newlen);

	CASE_WIDTH_4BYTE:
		mystr.cp32 = DeeString_As4Byte(self);
		if unlikely(!mystr.cp32)
			goto err;
		mskstr.cp32 = DeeString_As4Byte(mask);
		if unlikely(!mskstr.cp32)
			goto err;
		mylen       = WSTR_LENGTH(mystr.cp32);
		msklen      = WSTR_LENGTH(mskstr.cp32);
		newstr.cp32 = mystr.cp32, newlen = mylen;
		while (newlen && memchrl(mskstr.cp32, newstr.cp32[0], msklen))
			++newstr.cp32, --newlen;
		if (newlen == mylen)
			goto retself;
		return DeeString_New4Byte(newstr.cp32, newlen);
	}
retself:
	return_reference_(self);
err:
	return NULL;
}

INTERN DREF DeeObject *DCALL
DeeString_RStripMask(DeeObject *__restrict self, DeeObject *__restrict mask) {
	union dcharptr mystr, mskstr;
	size_t mylen, newlen, msklen;
	ASSERT_OBJECT_TYPE_EXACT(self, &DeeString_Type);
	ASSERT_OBJECT_TYPE_EXACT(mask, &DeeString_Type);
	SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON(DeeString_WIDTH(self),
	                                        DeeString_WIDTH(mask))) {

	CASE_WIDTH_1BYTE:
		mystr.cp8  = DeeString_As1Byte(self);
		mskstr.cp8 = DeeString_As1Byte(mask);
		mylen      = WSTR_LENGTH(mystr.cp8);
		msklen     = WSTR_LENGTH(mskstr.cp8);
		newlen     = mylen;
		while (newlen && memchrb(mskstr.cp8, mystr.cp8[newlen - 1], msklen))
			--newlen;
		if (newlen == mylen)
			goto retself;
		return DeeString_New1Byte(mystr.cp8, newlen);

	CASE_WIDTH_2BYTE:
		mystr.cp16 = DeeString_As2Byte(self);
		if unlikely(!mystr.cp16)
			goto err;
		mskstr.cp16 = DeeString_As2Byte(mask);
		if unlikely(!mskstr.cp16)
			goto err;
		mylen  = WSTR_LENGTH(mystr.cp16);
		msklen = WSTR_LENGTH(mskstr.cp16);
		newlen = mylen;
		while (newlen && memchrw(mskstr.cp16, mystr.cp16[newlen - 1], msklen))
			--newlen;
		if (newlen == mylen)
			goto retself;
		return DeeString_New2Byte(mystr.cp16, newlen);

	CASE_WIDTH_4BYTE:
		mystr.cp32 = DeeString_As4Byte(self);
		if unlikely(!mystr.cp32)
			goto err;
		mskstr.cp32 = DeeString_As4Byte(mask);
		if unlikely(!mskstr.cp32)
			goto err;
		mylen  = WSTR_LENGTH(mystr.cp32);
		msklen = WSTR_LENGTH(mskstr.cp32);
		newlen = mylen;
		while (newlen && memchrl(mskstr.cp32, mystr.cp32[newlen - 1], msklen))
			--newlen;
		if (newlen == mylen)
			goto retself;
		return DeeString_New4Byte(mystr.cp32, newlen);
	}
retself:
	return_reference_(self);
err:
	return NULL;
}

INTERN DREF DeeObject *DCALL
DeeString_CaseStripMask(DeeObject *__restrict self,
                        DeeObject *__restrict mask) {
	union dcharptr mystr, newstr, mskstr;
	size_t mylen, newlen, msklen;
	ASSERT_OBJECT_TYPE_EXACT(self, &DeeString_Type);
	ASSERT_OBJECT_TYPE_EXACT(mask, &DeeString_Type);
	SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON(DeeString_WIDTH(self),
	                                        DeeString_WIDTH(mask))) {

	CASE_WIDTH_1BYTE:
		mystr.cp8  = DeeString_As1Byte(self);
		mskstr.cp8 = DeeString_As1Byte(mask);
		mylen      = WSTR_LENGTH(mystr.cp8);
		msklen     = WSTR_LENGTH(mskstr.cp8);
		newstr.cp8 = mystr.cp8, newlen = mylen;
		while (newlen && memcasechrb(mskstr.cp8, newstr.cp8[0], msklen))
			++newstr.cp8, --newlen;
		while (newlen && memcasechrb(mskstr.cp8, newstr.cp8[newlen - 1], msklen))
			--newlen;
		if (newlen == mylen)
			goto retself;
		return DeeString_New1Byte(newstr.cp8, newlen);

	CASE_WIDTH_2BYTE:
		mystr.cp16 = DeeString_As2Byte(self);
		if unlikely(!mystr.cp16)
			goto err;
		mskstr.cp16 = DeeString_As2Byte(mask);
		if unlikely(!mskstr.cp16)
			goto err;
		mylen       = WSTR_LENGTH(mystr.cp16);
		msklen      = WSTR_LENGTH(mskstr.cp16);
		newstr.cp16 = mystr.cp16, newlen = mylen;
		while (newlen && memcasechrw(mskstr.cp16, newstr.cp16[0], msklen))
			++newstr.cp16, --newlen;
		while (newlen && memcasechrw(mskstr.cp16, newstr.cp16[newlen - 1], msklen))
			--newlen;
		if (newlen == mylen)
			goto retself;
		return DeeString_New2Byte(newstr.cp16, newlen);

	CASE_WIDTH_4BYTE:
		mystr.cp32 = DeeString_As4Byte(self);
		if unlikely(!mystr.cp32)
			goto err;
		mskstr.cp32 = DeeString_As4Byte(mask);
		if unlikely(!mskstr.cp32)
			goto err;
		mylen       = WSTR_LENGTH(mystr.cp32);
		msklen      = WSTR_LENGTH(mskstr.cp32);
		newstr.cp32 = mystr.cp32, newlen = mylen;
		while (newlen && memcasechrl(mskstr.cp32, newstr.cp32[0], msklen))
			++newstr.cp32, --newlen;
		while (newlen && memcasechrl(mskstr.cp32, newstr.cp32[newlen - 1], msklen))
			--newlen;
		if (newlen == mylen)
			goto retself;
		return DeeString_New4Byte(newstr.cp32, newlen);
	}
retself:
	return_reference_(self);
err:
	return NULL;
}

INTERN DREF DeeObject *DCALL
DeeString_CaseLStripMask(DeeObject *__restrict self, DeeObject *__restrict mask) {
	union dcharptr mystr, newstr, mskstr;
	size_t mylen, newlen, msklen;
	ASSERT_OBJECT_TYPE_EXACT(self, &DeeString_Type);
	ASSERT_OBJECT_TYPE_EXACT(mask, &DeeString_Type);
	SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON(DeeString_WIDTH(self),
	                                        DeeString_WIDTH(mask))) {

	CASE_WIDTH_1BYTE:
		mystr.cp8  = DeeString_As1Byte(self);
		mskstr.cp8 = DeeString_As1Byte(mask);
		mylen      = WSTR_LENGTH(mystr.cp8);
		msklen     = WSTR_LENGTH(mskstr.cp8);
		newstr.cp8 = mystr.cp8, newlen = mylen;
		while (newlen && memcasechrb(mskstr.cp8, newstr.cp8[0], msklen))
			++newstr.cp8, --newlen;
		if (newlen == mylen)
			goto retself;
		return DeeString_New1Byte(newstr.cp8, newlen);

	CASE_WIDTH_2BYTE:
		mystr.cp16 = DeeString_As2Byte(self);
		if unlikely(!mystr.cp16)
			goto err;
		mskstr.cp16 = DeeString_As2Byte(mask);
		if unlikely(!mskstr.cp16)
			goto err;
		mylen       = WSTR_LENGTH(mystr.cp16);
		msklen      = WSTR_LENGTH(mskstr.cp16);
		newstr.cp16 = mystr.cp16, newlen = mylen;
		while (newlen && memcasechrw(mskstr.cp16, newstr.cp16[0], msklen))
			++newstr.cp16, --newlen;
		if (newlen == mylen)
			goto retself;
		return DeeString_New2Byte(newstr.cp16, newlen);

	CASE_WIDTH_4BYTE:
		mystr.cp32 = DeeString_As4Byte(self);
		if unlikely(!mystr.cp32)
			goto err;
		mskstr.cp32 = DeeString_As4Byte(mask);
		if unlikely(!mskstr.cp32)
			goto err;
		mylen       = WSTR_LENGTH(mystr.cp32);
		msklen      = WSTR_LENGTH(mskstr.cp32);
		newstr.cp32 = mystr.cp32, newlen = mylen;
		while (newlen && memcasechrl(mskstr.cp32, newstr.cp32[0], msklen))
			++newstr.cp32, --newlen;
		if (newlen == mylen)
			goto retself;
		return DeeString_New4Byte(newstr.cp32, newlen);
	}
retself:
	return_reference_(self);
err:
	return NULL;
}

INTERN DREF DeeObject *DCALL
DeeString_CaseRStripMask(DeeObject *__restrict self, DeeObject *__restrict mask) {
	union dcharptr mystr, mskstr;
	size_t mylen, newlen, msklen;
	ASSERT_OBJECT_TYPE_EXACT(self, &DeeString_Type);
	ASSERT_OBJECT_TYPE_EXACT(mask, &DeeString_Type);
	SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON(DeeString_WIDTH(self),
	                                        DeeString_WIDTH(mask))) {

	CASE_WIDTH_1BYTE:
		mystr.cp8  = DeeString_As1Byte(self);
		mskstr.cp8 = DeeString_As1Byte(mask);
		mylen      = WSTR_LENGTH(mystr.cp8);
		msklen     = WSTR_LENGTH(mskstr.cp8);
		newlen     = mylen;
		while (newlen && memcasechrb(mskstr.cp8, mystr.cp8[newlen - 1], msklen))
			--newlen;
		if (newlen == mylen)
			goto retself;
		return DeeString_New1Byte(mystr.cp8, newlen);

	CASE_WIDTH_2BYTE:
		mystr.cp16 = DeeString_As2Byte(self);
		if unlikely(!mystr.cp16)
			goto err;
		mskstr.cp16 = DeeString_As2Byte(mask);
		if unlikely(!mskstr.cp16)
			goto err;
		mylen  = WSTR_LENGTH(mystr.cp16);
		msklen = WSTR_LENGTH(mskstr.cp16);
		newlen = mylen;
		while (newlen && memcasechrw(mskstr.cp16, mystr.cp16[newlen - 1], msklen))
			--newlen;
		if (newlen == mylen)
			goto retself;
		return DeeString_New2Byte(mystr.cp16, newlen);

	CASE_WIDTH_4BYTE:
		mystr.cp32 = DeeString_As4Byte(self);
		if unlikely(!mystr.cp32)
			goto err;
		mskstr.cp32 = DeeString_As4Byte(mask);
		if unlikely(!mskstr.cp32)
			goto err;
		mylen  = WSTR_LENGTH(mystr.cp32);
		msklen = WSTR_LENGTH(mskstr.cp32);
		newlen = mylen;
		while (newlen && memcasechrl(mskstr.cp32, mystr.cp32[newlen - 1], msklen))
			--newlen;
		if (newlen == mylen)
			goto retself;
		return DeeString_New4Byte(mystr.cp32, newlen);
	}
retself:
	return_reference_(self);
err:
	return NULL;
}

INTERN DREF DeeObject *DCALL
DeeString_Reversed(DeeObject *__restrict self,
                   size_t begin, size_t end) {
	DREF DeeObject *result;
	int width;
	uint8_t *my_str, *dst;
	size_t flip_size;
	ASSERT_OBJECT_TYPE_EXACT(self, &DeeString_Type);
	width  = DeeString_WIDTH(self);
	my_str = (uint8_t *)DeeString_WSTR(self);
	if (end > WSTR_LENGTH(my_str))
		end = WSTR_LENGTH(my_str);
	if (end <= begin)
		return_empty_string; /* Empty string area. */
	flip_size = (size_t)(end - begin);
	ASSERT(flip_size);
	/* Actually perform the search for the given string. */
	SWITCH_SIZEOF_WIDTH(width) {

	CASE_WIDTH_1BYTE:
		result = DeeString_NewBuffer(flip_size);
		if unlikely(!result)
			goto err;
		dst = (uint8_t *)DeeString_STR(result);
		do {
			--flip_size;
			*(uint8_t *)dst = ((uint8_t *)my_str)[flip_size];
			dst += 1;
		} while (flip_size);
		break;
		{
			uint16_t *buf;

		CASE_WIDTH_2BYTE:
			buf = DeeString_New2ByteBuffer(flip_size);
			if unlikely(!buf)
				goto err;
			dst = (uint8_t *)buf;
			do {
				--flip_size;
				*(uint16_t *)dst = ((uint16_t *)my_str)[flip_size];
				dst += 2;
			} while (flip_size);
			result = DeeString_Pack2ByteBuffer(buf);
		}
		break;
		{
			uint32_t *buf;

		CASE_WIDTH_4BYTE:
			buf = DeeString_New4ByteBuffer(flip_size);
			if unlikely(!buf)
				goto err;
			dst = (uint8_t *)buf;
			do {
				--flip_size;
				*(uint32_t *)dst = ((uint32_t *)my_str)[flip_size];
				dst += 4;
			} while (flip_size);
			result = DeeString_Pack4ByteBuffer(buf);
		}
		break;
	}
	return result;
err:
	return NULL;
}

INTERN DREF DeeObject *DCALL
DeeString_ExpandTabs(DeeObject *__restrict self, size_t tab_width) {
	union dcharptr iter, end, flush_start;
	size_t line_inset              = 0;
	struct unicode_printer printer = UNICODE_PRINTER_INIT;
	ASSERT_OBJECT_TYPE_EXACT(self, &DeeString_Type);
	SWITCH_SIZEOF_WIDTH(DeeString_WIDTH(self)) {

	CASE_WIDTH_1BYTE:
		iter.cp8        = DeeString_Get1Byte(self);
		end.cp8         = iter.cp8 + WSTR_LENGTH(iter.cp8);
		flush_start.cp8 = iter.cp8;
		unicode_printer_allocate(&printer,
		                         (size_t)(end.cp8 - iter.cp8),
		                         STRING_WIDTH_1BYTE);
		for (; iter.cp8 < end.cp8; ++iter.cp8) {
			uint8_t ch = *iter.cp8;
			if (!DeeUni_IsTab(ch)) {
				++line_inset;
				if (DeeUni_IsLF(ch))
					line_inset = 0; /* Reset insets at line starts. */
				continue;
			}
			if (unicode_printer_print8(&printer, flush_start.cp8,
			                           (size_t)(iter.cp8 - flush_start.cp8)) < 0)
				goto err;
			/* Replace with white-space. */
			if likely(tab_width) {
				line_inset = tab_width - (line_inset % tab_width);
				if (unicode_printer_repeatascii(&printer, UNICODE_SPACE, line_inset) < 0)
					goto err;
				line_inset = 0;
			}
			flush_start.cp8 = iter.cp8 + 1;
		}
		if (UNICODE_PRINTER_ISEMPTY(&printer))
			goto retself;
		if (unicode_printer_print8(&printer, flush_start.cp8,
		                           (size_t)(iter.cp8 - flush_start.cp8)) < 0)
			goto err;
		break;

	CASE_WIDTH_2BYTE:
		iter.cp16        = DeeString_Get2Byte(self);
		end.cp16         = iter.cp16 + WSTR_LENGTH(iter.cp16);
		flush_start.cp16 = iter.cp16;
		unicode_printer_allocate(&printer,
		                         (size_t)(end.cp16 - iter.cp16),
		                         STRING_WIDTH_2BYTE);
		for (; iter.cp16 < end.cp16; ++iter.cp16) {
			uint16_t ch = *iter.cp16;
			if (!DeeUni_IsTab(ch)) {
				++line_inset;
				if (DeeUni_IsLF(ch))
					line_inset = 0; /* Reset insets at line starts. */
				continue;
			}
			if (unicode_printer_print16(&printer, flush_start.cp16,
			                            (size_t)(iter.cp16 - flush_start.cp16)) < 0)
				goto err;
			/* Replace with white-space. */
			if likely(tab_width) {
				line_inset = tab_width - (line_inset % tab_width);
				if (unicode_printer_repeatascii(&printer, UNICODE_SPACE, line_inset) < 0)
					goto err;
				line_inset = 0;
			}
			flush_start.cp16 = iter.cp16 + 1;
		}
		if (UNICODE_PRINTER_ISEMPTY(&printer))
			goto retself;
		if (unicode_printer_print16(&printer, flush_start.cp16,
		                            (size_t)(iter.cp16 - flush_start.cp16)) < 0)
			goto err;
		break;

	CASE_WIDTH_4BYTE:
		iter.cp32        = DeeString_Get4Byte(self);
		end.cp32         = iter.cp32 + WSTR_LENGTH(iter.cp32);
		flush_start.cp32 = iter.cp32;
		unicode_printer_allocate(&printer,
		                         (size_t)(end.cp32 - iter.cp32),
		                         STRING_WIDTH_4BYTE);
		for (; iter.cp32 < end.cp32; ++iter.cp32) {
			uint32_t ch = *iter.cp32;
			if (!DeeUni_IsTab(ch)) {
				++line_inset;
				if (DeeUni_IsLF(ch))
					line_inset = 0; /* Reset insets at line starts. */
				continue;
			}
			if (unicode_printer_print32(&printer, flush_start.cp32,
			                            (size_t)(iter.cp32 - flush_start.cp32)) < 0)
				goto err;
			/* Replace with white-space. */
			if likely(tab_width) {
				line_inset = tab_width - (line_inset % tab_width);
				if (unicode_printer_repeatascii(&printer, UNICODE_SPACE, line_inset) < 0)
					goto err;
				line_inset = 0;
			}
			flush_start.cp32 = iter.cp32 + 1;
		}
		if (UNICODE_PRINTER_ISEMPTY(&printer))
			goto retself;
		if (unicode_printer_print32(&printer, flush_start.cp32,
		                            (size_t)(iter.cp32 - flush_start.cp32)) < 0)
			goto err;
		break;
	}
	return unicode_printer_pack(&printer);
retself:
	unicode_printer_fini(&printer);
	return_reference_(self);
err:
	unicode_printer_fini(&printer);
	return NULL;
}

INTERN DREF DeeObject *DCALL
DeeString_UnifyLines(DeeObject *__restrict self,
                     DeeObject *__restrict replacement) {
	union dcharptr iter, end, flush_start;
	struct unicode_printer printer = UNICODE_PRINTER_INIT;
	ASSERT_OBJECT_TYPE_EXACT(self, &DeeString_Type);
	SWITCH_SIZEOF_WIDTH(DeeString_WIDTH(self)) {

	CASE_WIDTH_1BYTE:
		iter.cp8        = DeeString_Get1Byte(self);
		end.cp8         = iter.cp8 + WSTR_LENGTH(iter.cp8);
		flush_start.cp8 = iter.cp8;
		unicode_printer_allocate(&printer,
		                         (size_t)(end.cp8 - iter.cp8),
		                         STRING_WIDTH_1BYTE);
		for (; iter.cp8 < end.cp8; ++iter.cp8) {
			uint8_t ch = *iter.cp8;
			if (!DeeUni_IsLF(ch))
				continue;
			if (unicode_printer_print8(&printer, flush_start.cp8,
			                           (size_t)(iter.cp8 - flush_start.cp8)) < 0 ||
			    unicode_printer_printstring(&printer, replacement) < 0)
				goto err;
			if (ch == UNICODE_CR && iter.cp8[1] == UNICODE_LF)
				++iter.cp8;
			flush_start.cp8 = iter.cp8 + 1;
		}
		if (UNICODE_PRINTER_ISEMPTY(&printer))
			goto retself;
		if (unicode_printer_print8(&printer, flush_start.cp8,
		                           (size_t)(iter.cp8 - flush_start.cp8)) < 0)
			goto err;
		break;

	CASE_WIDTH_2BYTE:
		iter.cp16        = DeeString_Get2Byte(self);
		end.cp16         = iter.cp16 + WSTR_LENGTH(iter.cp16);
		flush_start.cp16 = iter.cp16;
		unicode_printer_allocate(&printer,
		                         (size_t)(end.cp16 - iter.cp16),
		                         STRING_WIDTH_2BYTE);
		for (; iter.cp16 < end.cp16; ++iter.cp16) {
			uint16_t ch = *iter.cp16;
			if (!DeeUni_IsLF(ch))
				continue;
			if (unicode_printer_print16(&printer, flush_start.cp16,
			                            (size_t)(iter.cp16 - flush_start.cp16)) < 0 ||
			    unicode_printer_printstring(&printer, replacement) < 0)
				goto err;
			if (ch == UNICODE_CR && iter.cp16[1] == UNICODE_LF)
				++iter.cp16;
			flush_start.cp16 = iter.cp16 + 1;
		}
		if (UNICODE_PRINTER_ISEMPTY(&printer))
			goto retself;
		if (unicode_printer_print16(&printer, flush_start.cp16,
		                            (size_t)(iter.cp16 - flush_start.cp16)) < 0)
			goto err;
		break;

	CASE_WIDTH_4BYTE:
		iter.cp32        = DeeString_Get4Byte(self);
		end.cp32         = iter.cp32 + WSTR_LENGTH(iter.cp32);
		flush_start.cp32 = iter.cp32;
		unicode_printer_allocate(&printer,
		                         (size_t)(end.cp32 - iter.cp32),
		                         STRING_WIDTH_4BYTE);
		for (; iter.cp32 < end.cp32; ++iter.cp32) {
			uint32_t ch = *iter.cp32;
			if (!DeeUni_IsLF(ch))
				continue;
			if (unicode_printer_print32(&printer, flush_start.cp32,
			                            (size_t)(iter.cp32 - flush_start.cp32)) < 0 ||
			    unicode_printer_printstring(&printer, replacement) < 0)
				goto err;
			if (ch == UNICODE_CR && iter.cp32[1] == UNICODE_LF)
				++iter.cp32;
			flush_start.cp32 = iter.cp32 + 1;
		}
		if (UNICODE_PRINTER_ISEMPTY(&printer))
			goto retself;
		if (unicode_printer_print32(&printer, flush_start.cp32,
		                            (size_t)(iter.cp32 - flush_start.cp32)) < 0)
			goto err;
		break;
	}
	return unicode_printer_pack(&printer);
retself:
	unicode_printer_fini(&printer);
	return_reference_(self);
err:
	unicode_printer_fini(&printer);
	return NULL;
}

INTERN DREF DeeObject *DCALL
DeeString_UnifyLinesLf(DeeObject *__restrict self) {
	union dcharptr iter, end, flush_start;
	struct unicode_printer printer = UNICODE_PRINTER_INIT;
	ASSERT_OBJECT_TYPE_EXACT(self, &DeeString_Type);
	SWITCH_SIZEOF_WIDTH(DeeString_WIDTH(self)) {

	CASE_WIDTH_1BYTE:
		iter.cp8        = DeeString_Get1Byte(self);
		end.cp8         = iter.cp8 + WSTR_LENGTH(iter.cp8);
		flush_start.cp8 = iter.cp8;
		unicode_printer_allocate(&printer,
		                         (size_t)(end.cp8 - iter.cp8),
		                         STRING_WIDTH_1BYTE);
		for (; iter.cp8 < end.cp8; ++iter.cp8) {
			uint8_t ch = *iter.cp8;
			if (!DeeUni_IsLF(ch))
				continue;
			if (ch == UNICODE_LF)
				continue;
			if (unicode_printer_print8(&printer, flush_start.cp8,
			                           (size_t)(iter.cp8 - flush_start.cp8)) < 0 ||
			    unicode_printer_putascii(&printer, UNICODE_LF) < 0)
				goto err;
			if (ch == UNICODE_CR && iter.cp8[1] == UNICODE_LF)
				++iter.cp8;
			flush_start.cp8 = iter.cp8 + 1;
		}
		if (UNICODE_PRINTER_ISEMPTY(&printer))
			goto retself;
		if (unicode_printer_print8(&printer, flush_start.cp8,
		                           (size_t)(iter.cp8 - flush_start.cp8)) < 0)
			goto err;
		break;

	CASE_WIDTH_2BYTE:
		iter.cp16        = DeeString_Get2Byte(self);
		end.cp16         = iter.cp16 + WSTR_LENGTH(iter.cp16);
		flush_start.cp16 = iter.cp16;
		unicode_printer_allocate(&printer,
		                         (size_t)(end.cp16 - iter.cp16),
		                         STRING_WIDTH_2BYTE);
		for (; iter.cp16 < end.cp16; ++iter.cp16) {
			uint16_t ch = *iter.cp16;
			if (!DeeUni_IsLF(ch))
				continue;
			if (ch == UNICODE_LF)
				continue;
			if (unicode_printer_print16(&printer, flush_start.cp16,
			                            (size_t)(iter.cp16 - flush_start.cp16)) < 0 ||
			    unicode_printer_putascii(&printer, UNICODE_LF) < 0)
				goto err;
			if (ch == UNICODE_CR && iter.cp16[1] == UNICODE_LF)
				++iter.cp16;
			flush_start.cp16 = iter.cp16 + 1;
		}
		if (UNICODE_PRINTER_ISEMPTY(&printer))
			goto retself;
		if (unicode_printer_print16(&printer, flush_start.cp16,
		                            (size_t)(iter.cp16 - flush_start.cp16)) < 0)
			goto err;
		break;

	CASE_WIDTH_4BYTE:
		iter.cp32        = DeeString_Get4Byte(self);
		end.cp32         = iter.cp32 + WSTR_LENGTH(iter.cp32);
		flush_start.cp32 = iter.cp32;
		unicode_printer_allocate(&printer,
		                         (size_t)(end.cp32 - iter.cp32),
		                         STRING_WIDTH_4BYTE);
		for (; iter.cp32 < end.cp32; ++iter.cp32) {
			uint32_t ch = *iter.cp32;
			if (!DeeUni_IsLF(ch))
				continue;
			if (ch == UNICODE_LF)
				continue;
			if (unicode_printer_print32(&printer, flush_start.cp32,
			                            (size_t)(iter.cp32 - flush_start.cp32)) < 0 ||
			    unicode_printer_putascii(&printer, UNICODE_LF) < 0)
				goto err;
			if (ch == UNICODE_CR && iter.cp32[1] == UNICODE_LF)
				++iter.cp32;
			flush_start.cp32 = iter.cp32 + 1;
		}
		if (UNICODE_PRINTER_ISEMPTY(&printer))
			goto retself;
		if (unicode_printer_print32(&printer, flush_start.cp32,
		                            (size_t)(iter.cp32 - flush_start.cp32)) < 0)
			goto err;
		break;
	}
	return unicode_printer_pack(&printer);
retself:
	unicode_printer_fini(&printer);
	return_reference_(self);
err:
	unicode_printer_fini(&printer);
	return NULL;
}

INTERN DREF DeeObject *DCALL
DeeString_Join(DeeObject *__restrict self, DeeObject *__restrict seq) {
	DREF DeeObject *iter, *elem;
	bool is_first = true;
	size_t fast_size;
	struct unicode_printer printer = UNICODE_PRINTER_INIT;
	ASSERT_OBJECT_TYPE_EXACT(self, &DeeString_Type);
	fast_size = DeeFastSeq_GetSize(seq);
	if (fast_size != DEE_FASTSEQ_NOTFAST) {
		/* Fast-sequence optimizations. */
		size_t i;
		for (i = 0; i < fast_size; ++i) {
			/* Print `self' prior to every object, starting with the 2nd one. */
			if unlikely(!is_first &&
				         unicode_printer_printstring(&printer, self) < 0)
			goto err;
			elem = DeeFastSeq_GetItem(seq, i);
			if unlikely(!elem)
				goto err;
			if unlikely(unicode_printer_printobject(&printer, elem) < 0)
				goto err_elem_noiter;
			Dee_Decref(elem);
			is_first = false;
		}
	} else {
		iter = DeeObject_IterSelf(seq);
		if unlikely(!iter)
			goto err;
		while (ITER_ISOK(elem = DeeObject_IterNext(iter))) {
			/* Print `self' prior to every object, starting with the 2nd one. */
			if unlikely(!is_first &&
				         unicode_printer_printstring(&printer, self) < 0)
			goto err_elem;
			if unlikely(unicode_printer_printobject(&printer, elem) < 0)
				goto err_elem;
			Dee_Decref(elem);
			is_first = false;
			if (DeeThread_CheckInterrupt())
				goto err_iter;
		}
		if unlikely(!elem)
			goto err_iter;
		Dee_Decref(iter);
	}
	return unicode_printer_pack(&printer);
err_elem_noiter:
	Dee_Decref(elem);
	goto err;
err_elem:
	Dee_Decref(elem);
err_iter:
	Dee_Decref(iter);
err:
	unicode_printer_fini(&printer);
	return NULL;
}

INTERN DREF DeeObject *DCALL
DeeString_SStrip(DeeObject *__restrict self,
                 DeeObject *__restrict other) {
	union dcharptr mystr, newstr, otherstr;
	size_t mylen, newlen, otherlen;
	ASSERT_OBJECT_TYPE_EXACT(self, &DeeString_Type);
	ASSERT_OBJECT_TYPE_EXACT(other, &DeeString_Type);
	SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON(DeeString_WIDTH(self),
	                                        DeeString_WIDTH(other))) {

	CASE_WIDTH_1BYTE:
		mystr.cp8    = DeeString_As1Byte(self);
		otherstr.cp8 = DeeString_As1Byte(other);
		mylen        = WSTR_LENGTH(mystr.cp8);
		otherlen     = WSTR_LENGTH(otherstr.cp8);
		if unlikely(!otherlen)
			goto retself;
		newstr.cp8 = mystr.cp8, newlen = mylen;
		while (newlen >= otherlen &&
		       MEMEQB(newstr.cp8, otherstr.cp8, otherlen))
			newstr.cp8 += otherlen, newlen -= otherlen;
		while (newlen >= otherlen &&
		       MEMEQB(newstr.cp8 + (newlen - otherlen),
		              otherstr.cp8, otherlen))
			newlen -= otherlen;
		if (newlen == mylen)
			goto retself;
		return DeeString_New1Byte(newstr.cp8, newlen);

	CASE_WIDTH_2BYTE:
		mystr.cp16 = DeeString_As2Byte(self);
		if unlikely(!mystr.cp16)
			goto err;
		otherstr.cp16 = DeeString_As2Byte(other);
		if unlikely(!otherstr.cp16)
			goto err;
		mylen    = WSTR_LENGTH(mystr.cp16);
		otherlen = WSTR_LENGTH(otherstr.cp16);
		if unlikely(!otherlen)
			goto retself;
		newstr.cp16 = mystr.cp16, newlen = mylen;
		while (newlen >= otherlen &&
		       MEMEQB(newstr.cp16, otherstr.cp16, otherlen))
			newstr.cp16 += otherlen, newlen -= otherlen;
		while (newlen >= otherlen &&
		       MEMEQB(newstr.cp16 + (newlen - otherlen),
		              otherstr.cp16, otherlen))
			newlen -= otherlen;
		if (newlen == mylen)
			goto retself;
		if (DeeString_WIDTH(self) == STRING_WIDTH_1BYTE)
			return DeeString_New1Byte(DeeString_Get1Byte(self) + (newstr.cp16 - mystr.cp16), newlen);
		return DeeString_New2Byte(newstr.cp16, newlen);

	CASE_WIDTH_4BYTE:
		mystr.cp32 = DeeString_As4Byte(self);
		if unlikely(!mystr.cp32)
			goto err;
		otherstr.cp32 = DeeString_As4Byte(other);
		if unlikely(!otherstr.cp32)
			goto err;
		mylen    = WSTR_LENGTH(mystr.cp32);
		otherlen = WSTR_LENGTH(otherstr.cp32);
		if unlikely(!otherlen)
			goto retself;
		newstr.cp32 = mystr.cp32, newlen = mylen;
		while (newlen >= otherlen &&
		       MEMEQB(newstr.cp32, otherstr.cp32, otherlen))
			newstr.cp32 += otherlen, newlen -= otherlen;
		while (newlen >= otherlen &&
		       MEMEQB(newstr.cp32 + (newlen - otherlen),
		              otherstr.cp32, otherlen))
			newlen -= otherlen;
		if (newlen == mylen)
			goto retself;
		if (DeeString_WIDTH(self) == STRING_WIDTH_1BYTE)
			return DeeString_New1Byte(DeeString_Get1Byte(self) + (newstr.cp32 - mystr.cp32), newlen);
		if (DeeString_WIDTH(self) == STRING_WIDTH_2BYTE)
			return DeeString_New2Byte(DeeString_Get2Byte(self) + (newstr.cp32 - mystr.cp32), newlen);
		return DeeString_New4Byte(newstr.cp32, newlen);
	}
retself:
	return_reference_(self);
err:
	return NULL;
}

INTERN DREF DeeObject *DCALL
DeeString_LSStrip(DeeObject *__restrict self,
                  DeeObject *__restrict other) {
	union dcharptr mystr, newstr, otherstr;
	size_t mylen, newlen, otherlen;
	ASSERT_OBJECT_TYPE_EXACT(self, &DeeString_Type);
	ASSERT_OBJECT_TYPE_EXACT(other, &DeeString_Type);
	SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON(DeeString_WIDTH(self),
	                                        DeeString_WIDTH(other))) {

	CASE_WIDTH_1BYTE:
		mystr.cp8    = DeeString_As1Byte(self);
		otherstr.cp8 = DeeString_As1Byte(other);
		mylen        = WSTR_LENGTH(mystr.cp8);
		otherlen     = WSTR_LENGTH(otherstr.cp8);
		if unlikely(!otherlen)
			goto retself;
		newstr.cp8 = mystr.cp8, newlen = mylen;
		while (newlen >= otherlen &&
		       MEMEQB(newstr.cp8, otherstr.cp8, otherlen))
			newstr.cp8 += otherlen, newlen -= otherlen;
		if (newlen == mylen)
			goto retself;
		return DeeString_New1Byte(newstr.cp8, newlen);

	CASE_WIDTH_2BYTE:
		mystr.cp16 = DeeString_As2Byte(self);
		if unlikely(!mystr.cp16)
			goto err;
		otherstr.cp16 = DeeString_As2Byte(other);
		if unlikely(!otherstr.cp16)
			goto err;
		mylen    = WSTR_LENGTH(mystr.cp16);
		otherlen = WSTR_LENGTH(otherstr.cp16);
		if unlikely(!otherlen)
			goto retself;
		newstr.cp16 = mystr.cp16, newlen = mylen;
		while (newlen >= otherlen &&
		       MEMEQB(newstr.cp16, otherstr.cp16, otherlen))
			newstr.cp16 += otherlen, newlen -= otherlen;
		if (newlen == mylen)
			goto retself;
		if (DeeString_WIDTH(self) == STRING_WIDTH_1BYTE)
			return DeeString_New1Byte(DeeString_Get1Byte(self) + (newstr.cp16 - mystr.cp16), newlen);
		return DeeString_New2Byte(newstr.cp16, newlen);

	CASE_WIDTH_4BYTE:
		mystr.cp32 = DeeString_As4Byte(self);
		if unlikely(!mystr.cp32)
			goto err;
		otherstr.cp32 = DeeString_As4Byte(other);
		if unlikely(!otherstr.cp32)
			goto err;
		mylen    = WSTR_LENGTH(mystr.cp32);
		otherlen = WSTR_LENGTH(otherstr.cp32);
		if unlikely(!otherlen)
			goto retself;
		newstr.cp32 = mystr.cp32, newlen = mylen;
		while (newlen >= otherlen &&
		       MEMEQB(newstr.cp32, otherstr.cp32, otherlen))
			newstr.cp32 += otherlen, newlen -= otherlen;
		if (newlen == mylen)
			goto retself;
		if (DeeString_WIDTH(self) == STRING_WIDTH_1BYTE)
			return DeeString_New1Byte(DeeString_Get1Byte(self) + (newstr.cp32 - mystr.cp32), newlen);
		if (DeeString_WIDTH(self) == STRING_WIDTH_2BYTE)
			return DeeString_New2Byte(DeeString_Get2Byte(self) + (newstr.cp32 - mystr.cp32), newlen);
		return DeeString_New4Byte(newstr.cp32, newlen);
	}
retself:
	return_reference_(self);
err:
	return NULL;
}

INTERN DREF DeeObject *DCALL
DeeString_RSStrip(DeeObject *__restrict self,
                  DeeObject *__restrict other) {
	union dcharptr mystr, otherstr;
	size_t mylen, newlen, otherlen;
	ASSERT_OBJECT_TYPE_EXACT(self, &DeeString_Type);
	ASSERT_OBJECT_TYPE_EXACT(other, &DeeString_Type);
	SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON(DeeString_WIDTH(self),
	                                        DeeString_WIDTH(other))) {

	CASE_WIDTH_1BYTE:
		mystr.cp8    = DeeString_As1Byte(self);
		otherstr.cp8 = DeeString_As1Byte(other);
		mylen        = WSTR_LENGTH(mystr.cp8);
		otherlen     = WSTR_LENGTH(otherstr.cp8);
		if unlikely(!otherlen)
			goto retself;
		newlen = mylen;
		while (newlen >= otherlen &&
		       MEMEQB(mystr.cp8 + (newlen - otherlen),
		              otherstr.cp8, otherlen))
			newlen -= otherlen;
		if (newlen == mylen)
			goto retself;
		return DeeString_New1Byte(mystr.cp8, newlen);

	CASE_WIDTH_2BYTE:
		mystr.cp16 = DeeString_As2Byte(self);
		if unlikely(!mystr.cp16)
			goto err;
		otherstr.cp16 = DeeString_As2Byte(other);
		if unlikely(!otherstr.cp16)
			goto err;
		mylen    = WSTR_LENGTH(mystr.cp16);
		otherlen = WSTR_LENGTH(otherstr.cp16);
		if unlikely(!otherlen)
			goto retself;
		newlen = mylen;
		while (newlen >= otherlen &&
		       MEMEQW(mystr.cp16 + (newlen - otherlen),
		              otherstr.cp16, otherlen))
			newlen -= otherlen;
		if (newlen == mylen)
			goto retself;
		if (DeeString_WIDTH(self) == STRING_WIDTH_1BYTE)
			return DeeString_New1Byte(DeeString_Get1Byte(self) + (mystr.cp16 - mystr.cp16), newlen);
		return DeeString_New2Byte(mystr.cp16, newlen);

	CASE_WIDTH_4BYTE:
		mystr.cp32 = DeeString_As4Byte(self);
		if unlikely(!mystr.cp32)
			goto err;
		otherstr.cp32 = DeeString_As4Byte(other);
		if unlikely(!otherstr.cp32)
			goto err;
		mylen    = WSTR_LENGTH(mystr.cp32);
		otherlen = WSTR_LENGTH(otherstr.cp32);
		if unlikely(!otherlen)
			goto retself;
		newlen = mylen;
		while (newlen >= otherlen &&
		       MEMEQL(mystr.cp32 + (newlen - otherlen),
		              otherstr.cp32, otherlen))
			newlen -= otherlen;
		if (newlen == mylen)
			goto retself;
		if (DeeString_WIDTH(self) == STRING_WIDTH_1BYTE)
			return DeeString_New1Byte(DeeString_Get1Byte(self) + (mystr.cp32 - mystr.cp32), newlen);
		if (DeeString_WIDTH(self) == STRING_WIDTH_2BYTE)
			return DeeString_New2Byte(DeeString_Get2Byte(self) + (mystr.cp32 - mystr.cp32), newlen);
		return DeeString_New4Byte(mystr.cp32, newlen);
	}
retself:
	return_reference_(self);
err:
	return NULL;
}



INTERN DREF DeeObject *DCALL
DeeString_CaseSStrip(DeeObject *__restrict self,
                     DeeObject *__restrict other) {
	union dcharptr mystr, newstr, otherstr;
	size_t mylen, newlen, otherlen, matchlen;
	ASSERT_OBJECT_TYPE_EXACT(self, &DeeString_Type);
	ASSERT_OBJECT_TYPE_EXACT(other, &DeeString_Type);
	SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON(DeeString_WIDTH(self),
	                                        DeeString_WIDTH(other))) {

	CASE_WIDTH_1BYTE:
		mystr.cp8    = DeeString_As1Byte(self);
		otherstr.cp8 = DeeString_As1Byte(other);
		mylen        = WSTR_LENGTH(mystr.cp8);
		otherlen     = WSTR_LENGTH(otherstr.cp8);
		if unlikely(!otherlen)
			goto retself;
		newstr.cp8 = mystr.cp8, newlen = mylen;
		while ((matchlen = MEMCASESTARTSWITHB(newstr.cp8, newlen, otherstr.cp8, otherlen)) != 0)
			newstr.cp8 += matchlen, newlen -= matchlen;
		while ((matchlen = MEMCASEENDSWITHB(newstr.cp8, newlen, otherstr.cp8, otherlen)) != 0)
			newlen -= matchlen;
		if (newlen == mylen)
			goto retself;
		return DeeString_New1Byte(newstr.cp8, newlen);

	CASE_WIDTH_2BYTE:
		mystr.cp16 = DeeString_As2Byte(self);
		if unlikely(!mystr.cp16)
			goto err;
		otherstr.cp16 = DeeString_As2Byte(other);
		if unlikely(!otherstr.cp16)
			goto err;
		mylen    = WSTR_LENGTH(mystr.cp16);
		otherlen = WSTR_LENGTH(otherstr.cp16);
		if unlikely(!otherlen)
			goto retself;
		newstr.cp16 = mystr.cp16, newlen = mylen;
		while ((matchlen = MEMCASESTARTSWITHW(newstr.cp16, newlen, otherstr.cp16, otherlen)) != 0)
			newstr.cp16 += matchlen, newlen -= matchlen;
		while ((matchlen = MEMCASEENDSWITHW(newstr.cp16, newlen, otherstr.cp16, otherlen)) != 0)
			newlen -= matchlen;
		if (newlen == mylen)
			goto retself;
		if (DeeString_WIDTH(self) == STRING_WIDTH_1BYTE)
			return DeeString_New1Byte(DeeString_Get1Byte(self) + (newstr.cp16 - mystr.cp16), newlen);
		return DeeString_New2Byte(newstr.cp16, newlen);

	CASE_WIDTH_4BYTE:
		mystr.cp32 = DeeString_As4Byte(self);
		if unlikely(!mystr.cp32)
			goto err;
		otherstr.cp32 = DeeString_As4Byte(other);
		if unlikely(!otherstr.cp32)
			goto err;
		mylen    = WSTR_LENGTH(mystr.cp32);
		otherlen = WSTR_LENGTH(otherstr.cp32);
		if unlikely(!otherlen)
			goto retself;
		newstr.cp32 = mystr.cp32, newlen = mylen;
		while ((matchlen = MEMCASESTARTSWITHL(newstr.cp32, newlen, otherstr.cp32, otherlen)) != 0)
			newstr.cp32 += matchlen, newlen -= matchlen;
		while ((matchlen = MEMCASEENDSWITHL(newstr.cp32, newlen, otherstr.cp32, otherlen)) != 0)
			newlen -= matchlen;
		if (newlen == mylen)
			goto retself;
		if (DeeString_WIDTH(self) == STRING_WIDTH_1BYTE)
			return DeeString_New1Byte(DeeString_Get1Byte(self) + (newstr.cp32 - mystr.cp32), newlen);
		if (DeeString_WIDTH(self) == STRING_WIDTH_2BYTE)
			return DeeString_New2Byte(DeeString_Get2Byte(self) + (newstr.cp32 - mystr.cp32), newlen);
		return DeeString_New4Byte(newstr.cp32, newlen);
	}
retself:
	return_reference_(self);
err:
	return NULL;
}

INTERN DREF DeeObject *DCALL
DeeString_CaseLSStrip(DeeObject *__restrict self,
                      DeeObject *__restrict other) {
	union dcharptr mystr, newstr, otherstr;
	size_t mylen, newlen, otherlen, matchlen;
	ASSERT_OBJECT_TYPE_EXACT(self, &DeeString_Type);
	ASSERT_OBJECT_TYPE_EXACT(other, &DeeString_Type);
	SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON(DeeString_WIDTH(self),
	                                        DeeString_WIDTH(other))) {

	CASE_WIDTH_1BYTE:
		mystr.cp8    = DeeString_As1Byte(self);
		otherstr.cp8 = DeeString_As1Byte(other);
		mylen        = WSTR_LENGTH(mystr.cp8);
		otherlen     = WSTR_LENGTH(otherstr.cp8);
		if unlikely(!otherlen)
			goto retself;
		newstr.cp8 = mystr.cp8, newlen = mylen;
		while ((matchlen = MEMCASESTARTSWITHB(newstr.cp8, newlen, otherstr.cp8, otherlen)) != 0)
			newstr.cp8 += matchlen, newlen -= matchlen;
		if (newlen == mylen)
			goto retself;
		return DeeString_New1Byte(newstr.cp8, newlen);

	CASE_WIDTH_2BYTE:
		mystr.cp16 = DeeString_As2Byte(self);
		if unlikely(!mystr.cp16)
			goto err;
		otherstr.cp16 = DeeString_As2Byte(other);
		if unlikely(!otherstr.cp16)
			goto err;
		mylen    = WSTR_LENGTH(mystr.cp16);
		otherlen = WSTR_LENGTH(otherstr.cp16);
		if unlikely(!otherlen)
			goto retself;
		newstr.cp16 = mystr.cp16, newlen = mylen;
		while ((matchlen = MEMCASESTARTSWITHW(newstr.cp16, newlen, otherstr.cp16, otherlen)) != 0)
			newstr.cp16 += matchlen, newlen -= matchlen;
		if (newlen == mylen)
			goto retself;
		if (DeeString_WIDTH(self) == STRING_WIDTH_1BYTE)
			return DeeString_New1Byte(DeeString_Get1Byte(self) + (newstr.cp16 - mystr.cp16), newlen);
		return DeeString_New2Byte(newstr.cp16, newlen);

	CASE_WIDTH_4BYTE:
		mystr.cp32 = DeeString_As4Byte(self);
		if unlikely(!mystr.cp32)
			goto err;
		otherstr.cp32 = DeeString_As4Byte(other);
		if unlikely(!otherstr.cp32)
			goto err;
		mylen    = WSTR_LENGTH(mystr.cp32);
		otherlen = WSTR_LENGTH(otherstr.cp32);
		if unlikely(!otherlen)
			goto retself;
		newstr.cp32 = mystr.cp32, newlen = mylen;
		while ((matchlen = MEMCASESTARTSWITHL(newstr.cp32, newlen, otherstr.cp32, otherlen)) != 0)
			newstr.cp32 += matchlen, newlen -= matchlen;
		if (newlen == mylen)
			goto retself;
		if (DeeString_WIDTH(self) == STRING_WIDTH_1BYTE)
			return DeeString_New1Byte(DeeString_Get1Byte(self) + (newstr.cp32 - mystr.cp32), newlen);
		if (DeeString_WIDTH(self) == STRING_WIDTH_2BYTE)
			return DeeString_New2Byte(DeeString_Get2Byte(self) + (newstr.cp32 - mystr.cp32), newlen);
		return DeeString_New4Byte(newstr.cp32, newlen);
	}
retself:
	return_reference_(self);
err:
	return NULL;
}

INTERN DREF DeeObject *DCALL
DeeString_CaseRSStrip(DeeObject *__restrict self,
                      DeeObject *__restrict other) {
	union dcharptr mystr, otherstr;
	size_t mylen, newlen, otherlen, matchlen;
	ASSERT_OBJECT_TYPE_EXACT(self, &DeeString_Type);
	ASSERT_OBJECT_TYPE_EXACT(other, &DeeString_Type);
	SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON(DeeString_WIDTH(self),
	                                        DeeString_WIDTH(other))) {

	CASE_WIDTH_1BYTE:
		mystr.cp8    = DeeString_As1Byte(self);
		otherstr.cp8 = DeeString_As1Byte(other);
		mylen        = WSTR_LENGTH(mystr.cp8);
		otherlen     = WSTR_LENGTH(otherstr.cp8);
		if unlikely(!otherlen)
			goto retself;
		newlen = mylen;
		while ((matchlen = MEMCASEENDSWITHB(mystr.cp8, newlen, otherstr.cp8, otherlen)) != 0)
			newlen -= matchlen;
		if (newlen == mylen)
			goto retself;
		return DeeString_New1Byte(mystr.cp8, newlen);

	CASE_WIDTH_2BYTE:
		mystr.cp16 = DeeString_As2Byte(self);
		if unlikely(!mystr.cp16)
			goto err;
		otherstr.cp16 = DeeString_As2Byte(other);
		if unlikely(!otherstr.cp16)
			goto err;
		mylen    = WSTR_LENGTH(mystr.cp16);
		otherlen = WSTR_LENGTH(otherstr.cp16);
		if unlikely(!otherlen)
			goto retself;
		newlen = mylen;
		while ((matchlen = MEMCASEENDSWITHW(mystr.cp16, newlen, otherstr.cp16, otherlen)) != 0)
			newlen -= matchlen;
		if (newlen == mylen)
			goto retself;
		if (DeeString_WIDTH(self) == STRING_WIDTH_1BYTE)
			return DeeString_New1Byte(DeeString_Get1Byte(self) + (mystr.cp16 - mystr.cp16), newlen);
		return DeeString_New2Byte(mystr.cp16, newlen);

	CASE_WIDTH_4BYTE:
		mystr.cp32 = DeeString_As4Byte(self);
		if unlikely(!mystr.cp32)
			goto err;
		otherstr.cp32 = DeeString_As4Byte(other);
		if unlikely(!otherstr.cp32)
			goto err;
		mylen    = WSTR_LENGTH(mystr.cp32);
		otherlen = WSTR_LENGTH(otherstr.cp32);
		if unlikely(!otherlen)
			goto retself;
		newlen = mylen;
		while ((matchlen = MEMCASEENDSWITHL(mystr.cp32, newlen, otherstr.cp32, otherlen)) != 0)
			newlen -= matchlen;
		if (newlen == mylen)
			goto retself;
		if (DeeString_WIDTH(self) == STRING_WIDTH_1BYTE)
			return DeeString_New1Byte(DeeString_Get1Byte(self) + (mystr.cp32 - mystr.cp32), newlen);
		if (DeeString_WIDTH(self) == STRING_WIDTH_2BYTE)
			return DeeString_New2Byte(DeeString_Get2Byte(self) + (mystr.cp32 - mystr.cp32), newlen);
		return DeeString_New4Byte(mystr.cp32, newlen);
	}
retself:
	return_reference_(self);
err:
	return NULL;
}

#define DeeString_IsPrint(self, start, end)      DeeString_TestTrait(self, start, end, UNICODE_FPRINT)
#define DeeString_IsAlpha(self, start, end)      DeeString_TestTrait(self, start, end, UNICODE_FALPHA)
#define DeeString_IsSpace(self, start, end)      DeeString_TestTrait(self, start, end, UNICODE_FSPACE)
#define DeeString_IsLF(self, start, end)         DeeString_TestTrait(self, start, end, UNICODE_FLF)
#define DeeString_IsLower(self, start, end)      DeeString_TestTrait(self, start, end, UNICODE_FLOWER)
#define DeeString_IsUpper(self, start, end)      DeeString_TestTrait(self, start, end, UNICODE_FUPPER)
#define DeeString_IsCntrl(self, start, end)      DeeString_TestTrait(self, start, end, UNICODE_FCNTRL)
#define DeeString_IsDigit(self, start, end)      DeeString_TestTrait(self, start, end, UNICODE_FDIGIT)
#define DeeString_IsDecimal(self, start, end)    DeeString_TestTrait(self, start, end, UNICODE_FDECIMAL)
#define DeeString_IsSymStrt(self, start, end)    DeeString_TestTrait(self, start, end, UNICODE_FSYMSTRT)
#define DeeString_IsSymCont(self, start, end)    DeeString_TestTrait(self, start, end, UNICODE_FSYMCONT)
#define DeeString_IsAlnum(self, start, end)      DeeString_TestTrait(self, start, end, UNICODE_FALPHA | UNICODE_FDIGIT)
#define DeeString_IsNumeric(self, start, end)    DeeString_TestTrait(self, start, end, UNICODE_FDIGIT | UNICODE_FDECIMAL)
#define DeeString_IsAnyPrint(self, start, end)   DeeString_TestAnyTrait(self, start, end, UNICODE_FPRINT)
#define DeeString_IsAnyAlpha(self, start, end)   DeeString_TestAnyTrait(self, start, end, UNICODE_FALPHA)
#define DeeString_IsAnySpace(self, start, end)   DeeString_TestAnyTrait(self, start, end, UNICODE_FSPACE)
#define DeeString_IsAnyLF(self, start, end)      DeeString_TestAnyTrait(self, start, end, UNICODE_FLF)
#define DeeString_IsAnyLower(self, start, end)   DeeString_TestAnyTrait(self, start, end, UNICODE_FLOWER)
#define DeeString_IsAnyUpper(self, start, end)   DeeString_TestAnyTrait(self, start, end, UNICODE_FUPPER)
#define DeeString_IsAnyTitle(self, start, end)   DeeString_TestAnyTrait(self, start, end, UNICODE_FTITLE)
#define DeeString_IsAnyCntrl(self, start, end)   DeeString_TestAnyTrait(self, start, end, UNICODE_FCNTRL)
#define DeeString_IsAnyDigit(self, start, end)   DeeString_TestAnyTrait(self, start, end, UNICODE_FDIGIT)
#define DeeString_IsAnyDecimal(self, start, end) DeeString_TestAnyTrait(self, start, end, UNICODE_FDECIMAL)
#define DeeString_IsAnySymStrt(self, start, end) DeeString_TestAnyTrait(self, start, end, UNICODE_FSYMSTRT)
#define DeeString_IsAnySymCont(self, start, end) DeeString_TestAnyTrait(self, start, end, UNICODE_FSYMCONT)
#define DeeString_IsAnyAlnum(self, start, end)   DeeString_TestAnyTrait(self, start, end, UNICODE_FALPHA | UNICODE_FDECIMAL)
#define DeeString_IsAnyNumeric(self, start, end) DeeString_TestAnyTrait(self, start, end, UNICODE_FDIGIT | UNICODE_FDECIMAL)

INTERN bool DCALL
DeeString_TestTrait(DeeObject *__restrict self,
                    size_t start_index,
                    size_t end_index,
                    uniflag_t flags) {
	DeeString_Foreach(self, start_index, end_index, iter, end, {
		if (!(DeeUni_Flags(*iter) & flags))
			return false;
	});
	return true;
}

INTERN bool DCALL
DeeString_TestAnyTrait(DeeObject *__restrict self,
                       size_t start_index,
                       size_t end_index,
                       uniflag_t flags) {
	DeeString_Foreach(self, start_index, end_index, iter, end, {
		if (DeeUni_Flags(*iter) & flags)
			return true;
	});
	return false;
}

INTERN bool DCALL
DeeString_IsAscii(DeeObject *__restrict self,
                  size_t start_index,
                  size_t end_index) {
	struct string_utf *utf;
	utf = ((DeeStringObject *)self)->s_data;
	if (utf && utf->u_flags & STRING_UTF_FASCII)
		return true;
	DeeString_Foreach(self, start_index, end_index, iter, end, {
		if (*iter > 0x7f)
			return false;
	});
	/* Remember if the whole string is ASCII. */
	if (utf && start_index == 0 &&
	    end_index >= DeeString_WLEN(self))
		utf->u_flags |= STRING_UTF_FASCII;
	return true;
}

INTERN bool DCALL
DeeString_IsAnyAscii(DeeObject *__restrict self,
                     size_t start_index,
                     size_t end_index) {
	DeeString_Foreach(self, start_index, end_index, iter, end, {
		if (*iter <= 0x7f)
			return true;
	});
	return false;
}


INTERN bool DCALL
DeeString_IsTitle(DeeObject *__restrict self,
                  size_t start_index,
                  size_t end_index) {
	bool was_space = false;
	DeeString_Foreach(self, start_index, end_index, iter, end, {
		uniflag_t f = DeeUni_Flags(*iter);
		if (f & UNICODE_FSPACE) {
			was_space = true;
		} else if (was_space) {
			was_space = false;
			/* Space must be followed by title- or upper-case */
			if (!(f & (UNICODE_FTITLE | UNICODE_FUPPER)))
				return false;
		} else {
			/* Title- or upper-case anywhere else is illegal */
			if (f & (UNICODE_FTITLE | UNICODE_FUPPER))
				return false;
		}
	});
	return true;
}

INTERN bool DCALL
DeeString_IsSymbol(DeeObject *__restrict self,
                   size_t start_index,
                   size_t end_index) {
	uniflag_t flags = UNICODE_FSYMSTRT;
	DeeString_Foreach(self, start_index, end_index, iter, end, {
		if (!(DeeUni_Flags(*iter) & flags))
			return false;
		flags = UNICODE_FSYMCONT;
	});
	return true;
}



INTERN DREF DeeObject *DCALL
DeeString_Indent(DeeObject *__restrict self,
                 DeeObject *__restrict filler) {
	ASSERT_OBJECT_TYPE_EXACT(self, &DeeString_Type);
	ASSERT_OBJECT_TYPE_EXACT(filler, &DeeString_Type);
	/* Simple case: if the filler, or self-string are
	 *              empty, nothing would get inserted! */
	if unlikely(DeeString_IsEmpty(filler) || DeeString_IsEmpty(self))
		return_reference_(self);
	{
		struct unicode_printer printer = UNICODE_PRINTER_INIT;
		union dcharptr flush_start, iter, end;
		/* Start by inserting the initial, unconditional indentation at the start. */
		if (unicode_printer_printstring(&printer, filler) < 0)
			goto err;
		SWITCH_SIZEOF_WIDTH(DeeString_WIDTH(self)) {

		CASE_WIDTH_1BYTE:
			iter.cp8        = DeeString_Get1Byte(self);
			end.cp8         = iter.cp8 + WSTR_LENGTH(iter.cp8);
			flush_start.cp8 = iter.cp8;
			while (iter.cp8 < end.cp8) {
				uint8_t ch = *iter.cp8;
				if (DeeUni_IsLF(ch)) {
					++iter.cp8;
					/* Deal with windows-style linefeeds. */
					if (ch == UNICODE_CR && *iter.cp8 == UNICODE_LF)
						++iter.cp8;
					/* Flush all unwritten data up to this point. */
					if (unicode_printer_print8(&printer, flush_start.cp8,
					                           (size_t)(iter.cp8 - flush_start.cp8)) < 0)
						goto err;
					flush_start.cp8 = iter.cp8;
					/* Insert the filler just before the linefeed. */
					if (unicode_printer_printobject(&printer, filler) < 0)
						goto err;
					continue;
				}
				++iter.cp8;
			}
			if (iter.cp8 == flush_start.cp8) {
				/* Either the string is empty or ends with a line-feed.
				 * In either case, we must remove `filler' from its end,
				 * because we're not supposed to have the resulting
				 * string include it as trailing memory. */
				ASSERT(UNICODE_PRINTER_LENGTH(&printer) >= DeeString_WLEN(filler));
				unicode_printer_truncate(&printer,
				                         UNICODE_PRINTER_LENGTH(&printer) -
				                         DeeString_WLEN(filler));
			} else {
				/* Flush the remainder. */
				if (unicode_printer_print8(&printer, flush_start.cp8,
				                           (size_t)(iter.cp8 - flush_start.cp8)) < 0)
					goto err;
			}
			break;

		CASE_WIDTH_2BYTE:
			iter.cp16        = DeeString_Get2Byte(self);
			end.cp16         = iter.cp16 + WSTR_LENGTH(iter.cp16);
			flush_start.cp16 = iter.cp16;
			while (iter.cp16 < end.cp16) {
				uint16_t ch = *iter.cp16;
				if (DeeUni_IsLF(ch)) {
					++iter.cp16;
					if (ch == UNICODE_CR && *iter.cp16 == UNICODE_LF)
						++iter.cp16;
					if (unicode_printer_print16(&printer, flush_start.cp16,
					                            (size_t)(iter.cp16 - flush_start.cp16)) < 0)
						goto err;
					flush_start.cp16 = iter.cp16;
					if (unicode_printer_printobject(&printer, filler) < 0)
						goto err;
					continue;
				}
				++iter.cp16;
			}
			if (iter.cp16 == flush_start.cp16) {
				ASSERT(UNICODE_PRINTER_LENGTH(&printer) >= DeeString_WLEN(filler));
				unicode_printer_truncate(&printer,
				                         UNICODE_PRINTER_LENGTH(&printer) -
				                         DeeString_WLEN(filler));
			} else {
				if (unicode_printer_print16(&printer, flush_start.cp16,
				                            (size_t)(iter.cp16 - flush_start.cp16)) < 0)
					goto err;
			}
			break;

		CASE_WIDTH_4BYTE:
			iter.cp32        = DeeString_Get4Byte(self);
			end.cp32         = iter.cp32 + WSTR_LENGTH(iter.cp32);
			flush_start.cp32 = iter.cp32;
			while (iter.cp32 < end.cp32) {
				uint32_t ch = *iter.cp32;
				if (DeeUni_IsLF(ch)) {
					++iter.cp32;
					if (ch == UNICODE_CR && *iter.cp32 == UNICODE_LF)
						++iter.cp32;
					if (unicode_printer_print32(&printer, flush_start.cp32,
					                            (size_t)(iter.cp32 - flush_start.cp32)) < 0)
						goto err;
					flush_start.cp32 = iter.cp32;
					if (unicode_printer_printobject(&printer, filler) < 0)
						goto err;
					continue;
				}
				++iter.cp32;
			}
			if (iter.cp32 == flush_start.cp32) {
				ASSERT(UNICODE_PRINTER_LENGTH(&printer) >= DeeString_WLEN(filler));
				unicode_printer_truncate(&printer,
				                         UNICODE_PRINTER_LENGTH(&printer) -
				                         DeeString_WLEN(filler));
			} else {
				if (unicode_printer_print32(&printer, flush_start.cp32,
				                            (size_t)(iter.cp32 - flush_start.cp32)) < 0)
					goto err;
			}
			break;
		}
		return unicode_printer_pack(&printer);
err:
		unicode_printer_fini(&printer);
		return NULL;
	}
}

PRIVATE bool DCALL
mask_containsb(DeeObject *__restrict self, uint8_t ch) {
	union dcharptr str;
	SWITCH_SIZEOF_WIDTH(DeeString_WIDTH(self)) {

	CASE_WIDTH_1BYTE:
		str.cp8 = DeeString_Get1Byte(self);
		return memchrb(str.cp8, ch, WSTR_LENGTH(str.cp8)) != NULL;

	CASE_WIDTH_2BYTE:
		str.cp16 = DeeString_Get2Byte(self);
		return memchrw(str.cp16, ch, WSTR_LENGTH(str.cp16)) != NULL;

	CASE_WIDTH_4BYTE:
		str.cp32 = DeeString_Get4Byte(self);
		return memchrl(str.cp32, ch, WSTR_LENGTH(str.cp32)) != NULL;
	}
}

PRIVATE bool DCALL
mask_containsw(DeeObject *__restrict self, uint16_t ch) {
	union dcharptr str;
	SWITCH_SIZEOF_WIDTH(DeeString_WIDTH(self)) {

	CASE_WIDTH_1BYTE:
		if (ch > 0xff)
			return false;
		str.cp8 = DeeString_Get1Byte(self);
		return memchrb(str.cp8, (uint8_t)ch, WSTR_LENGTH(str.cp8)) != NULL;

	CASE_WIDTH_2BYTE:
		str.cp16 = DeeString_Get2Byte(self);
		return memchrw(str.cp16, ch, WSTR_LENGTH(str.cp16)) != NULL;

	CASE_WIDTH_4BYTE:
		str.cp32 = DeeString_Get4Byte(self);
		return memchrl(str.cp32, ch, WSTR_LENGTH(str.cp32)) != NULL;
	}
}

PRIVATE bool DCALL
mask_containsl(DeeObject *__restrict self, uint32_t ch) {
	union dcharptr str;
	SWITCH_SIZEOF_WIDTH(DeeString_WIDTH(self)) {

	CASE_WIDTH_1BYTE:
		if (ch > 0xff)
			return false;
		str.cp8 = DeeString_Get1Byte(self);
		return memchrb(str.cp8, (uint8_t)ch, WSTR_LENGTH(str.cp8)) != NULL;

	CASE_WIDTH_2BYTE:
		if (ch > 0xffff)
			return false;
		str.cp16 = DeeString_Get2Byte(self);
		return memchrw(str.cp16, (uint16_t)ch, WSTR_LENGTH(str.cp16)) != NULL;

	CASE_WIDTH_4BYTE:
		str.cp32 = DeeString_Get4Byte(self);
		return memchrl(str.cp32, ch, WSTR_LENGTH(str.cp32)) != NULL;
	}
}


INTERN DREF DeeObject *DCALL
DeeString_Dedent(DeeObject *__restrict self,
                 size_t max_chars,
                 DeeObject *__restrict mask) {
	struct unicode_printer printer = UNICODE_PRINTER_INIT;
	union dcharptr flush_start, iter, end;
	size_t i;
	ASSERT_OBJECT_TYPE_EXACT(self, &DeeString_Type);
	ASSERT_OBJECT_TYPE_EXACT(mask, &DeeString_Type);
	/* Simple case: Nothing should be removed. */
	if unlikely(!max_chars)
		return_reference_(self);
	SWITCH_SIZEOF_WIDTH(DeeString_WIDTH(self)) {

	CASE_WIDTH_1BYTE:
		iter.cp8 = DeeString_Get1Byte(self);
		end.cp8  = iter.cp8 + WSTR_LENGTH(iter.cp8);
		/* Remove leading characters. */
		for (i = 0; i < max_chars && mask_containsb(mask, *iter.cp8); ++i)
			++iter.cp8;
		flush_start.cp8 = iter.cp8;
		while (iter.cp8 < end.cp8) {
			uint8_t ch = *iter.cp8;
			if (DeeUni_IsLF(ch)) {
				++iter.cp8;
				if (ch == UNICODE_CR && *iter.cp8 == UNICODE_LF)
					++iter.cp8;
				/* Flush all unwritten data up to this point. */
				if (unicode_printer_print8(&printer, flush_start.cp8,
				                           (size_t)(iter.cp8 - flush_start.cp8)) < 0)
					goto err;
				/* Skip up to `max_chars' characters after a linefeed. */
				for (i = 0; i < max_chars && mask_containsb(mask, *iter.cp8); ++i)
					++iter.cp8;
				flush_start = iter;
				continue;
			}
			++iter.cp8;
		}
		/* Flush the remainder. */
		if (unicode_printer_print8(&printer, flush_start.cp8,
		                           (size_t)(iter.cp8 - flush_start.cp8)) < 0)
			goto err;
		break;

	CASE_WIDTH_2BYTE:
		iter.cp16 = DeeString_Get2Byte(self);
		end.cp16  = iter.cp16 + WSTR_LENGTH(iter.cp16);
		for (i = 0; i < max_chars && mask_containsw(mask, *iter.cp16); ++i)
			++iter.cp16;
		flush_start.cp16 = iter.cp16;
		while (iter.cp16 < end.cp16) {
			uint16_t ch = *iter.cp16;
			if (DeeUni_IsLF(ch)) {
				++iter.cp16;
				if (ch == UNICODE_CR && *iter.cp16 == UNICODE_LF)
					++iter.cp16;
				if (unicode_printer_print16(&printer, flush_start.cp16,
				                            (size_t)(iter.cp16 - flush_start.cp16)) < 0)
					goto err;
				for (i = 0; i < max_chars && mask_containsw(mask, *iter.cp16); ++i)
					++iter.cp16;
				flush_start = iter;
				continue;
			}
			++iter.cp16;
		}
		if (unicode_printer_print16(&printer, flush_start.cp16,
		                            (size_t)(iter.cp16 - flush_start.cp16)) < 0)
			goto err;
		break;

	CASE_WIDTH_4BYTE:
		iter.cp32 = DeeString_Get4Byte(self);
		end.cp32  = iter.cp32 + WSTR_LENGTH(iter.cp32);
		for (i = 0; i < max_chars && mask_containsl(mask, *iter.cp32); ++i)
			++iter.cp32;
		flush_start.cp32 = iter.cp32;
		while (iter.cp32 < end.cp32) {
			uint32_t ch = *iter.cp32;
			if (DeeUni_IsLF(ch)) {
				++iter.cp32;
				if (ch == UNICODE_CR && *iter.cp32 == UNICODE_LF)
					++iter.cp32;
				if (unicode_printer_print32(&printer, flush_start.cp32,
				                            (size_t)(iter.cp32 - flush_start.cp32)) < 0)
					goto err;
				for (i = 0; i < max_chars && mask_containsl(mask, *iter.cp32); ++i)
					++iter.cp32;
				flush_start = iter;
				continue;
			}
			++iter.cp32;
		}
		if (unicode_printer_print32(&printer, flush_start.cp32,
		                            (size_t)(iter.cp32 - flush_start.cp32)) < 0)
			goto err;
		break;
	}
	return unicode_printer_pack(&printer);
err:
	unicode_printer_fini(&printer);
	return NULL;
}

INTERN DREF DeeObject *DCALL
DeeString_DedentSpc(DeeObject *__restrict self,
                    size_t max_chars) {
	struct unicode_printer printer = UNICODE_PRINTER_INIT;
	union dcharptr flush_start, iter, end;
	size_t i;
	ASSERT_OBJECT_TYPE_EXACT(self, &DeeString_Type);
	/* Simple case: Nothing should be removed. */
	if unlikely(!max_chars)
		return_reference_(self);
	SWITCH_SIZEOF_WIDTH(DeeString_WIDTH(self)) {

	CASE_WIDTH_1BYTE:
		iter.cp8 = DeeString_Get1Byte(self);
		end.cp8  = iter.cp8 + WSTR_LENGTH(iter.cp8);
		/* Remove leading characters. */
		for (i = 0; i < max_chars && DeeUni_IsSpace(*iter.cp8); ++i)
			++iter.cp8;
		flush_start.cp8 = iter.cp8;
		while (iter.cp8 < end.cp8) {
			uint8_t ch = *iter.cp8;
			if (DeeUni_IsLF(ch)) {
				++iter.cp8;
				if (ch == UNICODE_CR && *iter.cp8 == UNICODE_LF)
					++iter.cp8;
				/* Flush all unwritten data up to this point. */
				if (unicode_printer_print8(&printer, flush_start.cp8,
				                           (size_t)(iter.cp8 - flush_start.cp8)) < 0)
					goto err;
				/* Skip up to `max_chars' characters after a linefeed. */
				for (i = 0; i < max_chars && DeeUni_IsSpace(*iter.cp8); ++i)
					++iter.cp8;
				flush_start = iter;
				continue;
			}
			++iter.cp8;
		}
		/* Flush the remainder. */
		if (unicode_printer_print8(&printer, flush_start.cp8,
		                           (size_t)(iter.cp8 - flush_start.cp8)) < 0)
			goto err;
		break;

	CASE_WIDTH_2BYTE:
		iter.cp16 = DeeString_Get2Byte(self);
		end.cp16  = iter.cp16 + WSTR_LENGTH(iter.cp16);
		for (i = 0; i < max_chars && DeeUni_IsSpace(*iter.cp16); ++i)
			++iter.cp16;
		flush_start.cp16 = iter.cp16;
		while (iter.cp16 < end.cp16) {
			uint16_t ch = *iter.cp16;
			if (DeeUni_IsLF(ch)) {
				++iter.cp16;
				if (ch == UNICODE_CR && *iter.cp16 == UNICODE_LF)
					++iter.cp16;
				if (unicode_printer_print16(&printer, flush_start.cp16,
				                            (size_t)(iter.cp16 - flush_start.cp16)) < 0)
					goto err;
				for (i = 0; i < max_chars && DeeUni_IsSpace(*iter.cp16); ++i)
					++iter.cp16;
				flush_start = iter;
				continue;
			}
			++iter.cp16;
		}
		if (unicode_printer_print16(&printer, flush_start.cp16,
		                            (size_t)(iter.cp16 - flush_start.cp16)) < 0)
			goto err;
		break;

	CASE_WIDTH_4BYTE:
		iter.cp32 = DeeString_Get4Byte(self);
		end.cp32  = iter.cp32 + WSTR_LENGTH(iter.cp32);
		for (i = 0; i < max_chars && DeeUni_IsSpace(*iter.cp32); ++i)
			++iter.cp32;
		flush_start.cp32 = iter.cp32;
		while (iter.cp32 < end.cp32) {
			uint32_t ch = *iter.cp32;
			if (DeeUni_IsLF(ch)) {
				++iter.cp32;
				if (ch == UNICODE_CR && *iter.cp32 == UNICODE_LF)
					++iter.cp32;
				if (unicode_printer_print32(&printer, flush_start.cp32,
				                            (size_t)(iter.cp32 - flush_start.cp32)) < 0)
					goto err;
				for (i = 0; i < max_chars && DeeUni_IsSpace(*iter.cp32); ++i)
					++iter.cp32;
				flush_start = iter;
				continue;
			}
			++iter.cp32;
		}
		if (unicode_printer_print32(&printer, flush_start.cp32,
		                            (size_t)(iter.cp32 - flush_start.cp32)) < 0)
			goto err;
		break;
	}
	return unicode_printer_pack(&printer);
err:
	unicode_printer_fini(&printer);
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeString_New2Byte(uint16_t const *__restrict str,
                   size_t length) {
	uint16_t *buffer;
	buffer = DeeString_New2ByteBuffer(length);
	if unlikely(!buffer)
		return NULL;
	memcpyw(buffer, str, length);
	return DeeString_Pack2ByteBuffer(buffer);
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeString_New4Byte(uint32_t const *__restrict str,
                   size_t length) {
	uint32_t *buffer;
	buffer = DeeString_New4ByteBuffer(length);
	if unlikely(!buffer)
		return NULL;
	memcpyl(buffer, str, length);
	return DeeString_Pack4ByteBuffer(buffer);
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeString_NewUtf16(uint16_t const *__restrict str,
                   size_t length,
                   unsigned int error_mode) {
	uint16_t *buffer;
	buffer = DeeString_New2ByteBuffer(length);
	if unlikely(!buffer)
		return NULL;
	memcpyw(buffer, str, length);
	return DeeString_PackUtf16Buffer(buffer, error_mode);
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeString_NewUtf32(uint32_t const *__restrict str,
                   size_t length,
                   unsigned int error_mode) {
	uint32_t *buffer;
	buffer = DeeString_New4ByteBuffer(length);
	if unlikely(!buffer)
		return NULL;
	memcpyl(buffer, str, length);
	return DeeString_PackUtf32Buffer(buffer, error_mode);
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeString_NewUtf16AltEndian(uint16_t const *__restrict str,
                            size_t length,
                            unsigned int error_mode) {
	uint16_t *buffer;
	size_t i;
	buffer = DeeString_New2ByteBuffer(length);
	if unlikely(!buffer)
		return NULL;
	for (i = 0; i < length; ++i)
		buffer[i] = BSWAP16(str[i]);
	return DeeString_PackUtf16Buffer(buffer, error_mode);
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeString_NewUtf32AltEndian(uint32_t const *__restrict str,
                            size_t length,
                            unsigned int error_mode) {
	uint32_t *buffer;
	size_t i;
	buffer = DeeString_New4ByteBuffer(length);
	if unlikely(!buffer)
		return NULL;
	for (i = 0; i < length; ++i)
		buffer[i] = BSWAP32(str[i]);
	return DeeString_PackUtf32Buffer(buffer, error_mode);
}



#ifndef PRIVATE_REPLACE_KWLIST_DEFINED
#define PRIVATE_REPLACE_KWLIST_DEFINED 1
PRIVATE struct keyword replace_kwlist[] = { K(find), K(replace), K(max), KEND };
#endif /* !PRIVATE_REPLACE_KWLIST_DEFINED */

PRIVATE DREF String *DCALL
string_replace(String *__restrict self, size_t argc,
               DeeObject **__restrict argv, DeeObject *Kw) {
	String *find, *replace;
	size_t max_count = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, Kw, replace_kwlist, "oo|Iu:replace", &find, &replace, &max_count) ||
	    DeeObject_AssertTypeExact((DeeObject *)find, &DeeString_Type) ||
	    DeeObject_AssertTypeExact((DeeObject *)replace, &DeeString_Type))
		goto err;
	if unlikely(!max_count)
		goto retself;
	{
		struct unicode_printer p = UNICODE_PRINTER_INIT;
		union dcharptr ptr, mystr, begin, end, findstr;
		size_t findlen;
		SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON(DeeString_WIDTH(self),
		                                        DeeString_WIDTH(find))) {

		CASE_WIDTH_1BYTE:
			mystr.cp8   = DeeString_As1Byte((DeeObject *)self);
			findstr.cp8 = DeeString_As1Byte((DeeObject *)find);
			findlen     = WSTR_LENGTH(findstr.cp8);
			/* Handle special cases. */
			if unlikely(findlen > WSTR_LENGTH(mystr.cp8))
				goto retself;
			if unlikely(!findlen)
				goto retrepl_if_self;
			begin.cp8 = mystr.cp8;
			end.cp8   = begin.cp8 + WSTR_LENGTH(mystr.cp8);
			while ((ptr.cp8 = memmemb(begin.cp8, end.cp8 - begin.cp8,
			                          findstr.cp8, findlen)) != NULL) {
				/* Found one */
				if (unlikely(unicode_printer_print8(&p, begin.cp8, (size_t)(ptr.cp8 - begin.cp8)) < 0) ||
				    unlikely(unicode_printer_printstring(&p, (DeeObject *)replace) < 0))
					goto err_printer;
				if unlikely(!--max_count)
					break;
				begin.cp8 = ptr.cp8 + findlen;
			}
			/* If we never found `find', our printer will still be empty.
			 * >> In that case we don't need to write the entire string to it,
			 *    but can simply return a reference to the original string,
			 *    saving on memory and speeding up the function by a lot. */
			if (UNICODE_PRINTER_ISEMPTY(&p) && begin.cp8 == mystr.cp8)
				goto retself;
			if unlikely(unicode_printer_print8(&p, begin.cp8, (size_t)(end.cp8 - begin.cp8)) < 0)
				goto err_printer;
			break;

		CASE_WIDTH_2BYTE:
			mystr.cp16 = DeeString_As2Byte((DeeObject *)self);
			if unlikely(!mystr.cp16)
				goto err_printer;
			findstr.cp16 = DeeString_As2Byte((DeeObject *)find);
			if unlikely(!findstr.cp16)
				goto err_printer;
			findlen = WSTR_LENGTH(findstr.cp16);
			if unlikely(findlen > WSTR_LENGTH(mystr.cp16))
				goto retself;
			if unlikely(!findlen)
				goto retrepl_if_self;
			begin.cp16 = mystr.cp16;
			end.cp16   = begin.cp16 + WSTR_LENGTH(mystr.cp16);
			while ((ptr.cp16 = memmemw(begin.cp16, end.cp16 - begin.cp16,
			                           findstr.cp16, findlen)) != NULL) {
				if (unlikely(unicode_printer_print16(&p, begin.cp16, (size_t)(ptr.cp16 - begin.cp16)) < 0) ||
				    unlikely(unicode_printer_printstring(&p, (DeeObject *)replace) < 0))
					goto err_printer;
				if unlikely(!--max_count)
					break;
				begin.cp16 = ptr.cp16 + findlen;
			}
			if (UNICODE_PRINTER_ISEMPTY(&p) && begin.cp16 == mystr.cp16)
				goto retself;
			if unlikely(unicode_printer_print16(&p, begin.cp16, (size_t)(end.cp16 - begin.cp16)) < 0)
				goto err_printer;
			break;

		CASE_WIDTH_4BYTE:
			mystr.cp32 = DeeString_As4Byte((DeeObject *)self);
			if unlikely(!mystr.cp32)
				goto err_printer;
			findstr.cp32 = DeeString_As4Byte((DeeObject *)find);
			if unlikely(!findstr.cp32)
				goto err_printer;
			findlen = WSTR_LENGTH(findstr.cp32);
			if unlikely(findlen > WSTR_LENGTH(mystr.cp32))
				goto retself;
			if unlikely(!findlen)
				goto retrepl_if_self;
			begin.cp32 = mystr.cp32;
			end.cp32   = begin.cp32 + WSTR_LENGTH(mystr.cp32);
			while ((ptr.cp32 = memmeml(begin.cp32, end.cp32 - begin.cp32,
			                           findstr.cp32, findlen)) != NULL) {
				if (unlikely(unicode_printer_print32(&p, begin.cp32, (size_t)(ptr.cp32 - begin.cp32)) < 0) ||
				    unlikely(unicode_printer_printstring(&p, (DeeObject *)replace) < 0))
					goto err_printer;
				if unlikely(!--max_count)
					break;
				begin.cp32 = ptr.cp32 + findlen;
			}
			if (UNICODE_PRINTER_ISEMPTY(&p) && begin.cp32 == mystr.cp32)
				goto retself;
			if unlikely(unicode_printer_print32(&p, begin.cp32, (size_t)(end.cp32 - begin.cp32)) < 0)
				goto err_printer;
			break;
		}
		return (DREF String *)unicode_printer_pack(&p);
err_printer:
		unicode_printer_fini(&p);
	}
err:
	return NULL;
retrepl_if_self:
	if (!DeeString_IsEmpty(self))
		goto retself;
	return_reference_(replace);
retself:
	return_reference_(self);
}

PRIVATE DREF String *DCALL
string_casereplace(String *__restrict self, size_t argc,
                   DeeObject **__restrict argv, DeeObject *kw) {
	String *find, *replace;
	size_t max_count = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, replace_kwlist, "oo|Iu:casereplace", &find, &replace, &max_count) ||
	    DeeObject_AssertTypeExact((DeeObject *)find, &DeeString_Type) ||
	    DeeObject_AssertTypeExact((DeeObject *)replace, &DeeString_Type))
		goto err;
	if unlikely(!max_count)
		goto retself;
	{
		struct unicode_printer p = UNICODE_PRINTER_INIT;
		union dcharptr ptr, mystr, begin, end, findstr;
		size_t findlen, match_length;
		SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON(DeeString_WIDTH(self),
		                                        DeeString_WIDTH(find))) {

		CASE_WIDTH_1BYTE:
			mystr.cp8   = DeeString_As1Byte((DeeObject *)self);
			findstr.cp8 = DeeString_As1Byte((DeeObject *)find);
			findlen     = WSTR_LENGTH(findstr.cp8);
			/* Handle special cases. */
			if unlikely(findlen > WSTR_LENGTH(mystr.cp8))
				goto retself;
			if unlikely(!findlen)
				goto retrepl_if_self;
			begin.cp8 = mystr.cp8;
			end.cp8   = begin.cp8 + WSTR_LENGTH(mystr.cp8);
			while ((ptr.cp8 = memcasememb(begin.cp8, end.cp8 - begin.cp8,
			                              findstr.cp8, findlen,
			                              &match_length)) != NULL) {
				/* Found one */
				if (unlikely(unicode_printer_print8(&p, begin.cp8, (size_t)(ptr.cp8 - begin.cp8)) < 0) ||
				    unlikely(unicode_printer_printstring(&p, (DeeObject *)replace) < 0))
					goto err_printer;
				if unlikely(!--max_count)
					break;
				begin.cp8 = ptr.cp8 + match_length;
			}
			/* If we never found `find', our printer will still be empty.
			 * >> In that case we don't need to write the entire string to it,
			 *    but can simply return a reference to the original string,
			 *    saving on memory and speeding up the function by a lot. */
			if (UNICODE_PRINTER_ISEMPTY(&p) && begin.cp8 == mystr.cp8)
				goto retself;
			if unlikely(unicode_printer_print8(&p, begin.cp8, (size_t)(end.cp8 - begin.cp8)) < 0)
				goto err_printer;
			break;

		CASE_WIDTH_2BYTE:
			mystr.cp16 = DeeString_As2Byte((DeeObject *)self);
			if unlikely(!mystr.cp16)
				goto err_printer;
			findstr.cp16 = DeeString_As2Byte((DeeObject *)find);
			if unlikely(!findstr.cp16)
				goto err_printer;
			findlen = WSTR_LENGTH(findstr.cp16);
			if unlikely(findlen > WSTR_LENGTH(mystr.cp16))
				goto retself;
			if unlikely(!findlen)
				goto retrepl_if_self;
			begin.cp16 = mystr.cp16;
			end.cp16   = begin.cp16 + WSTR_LENGTH(mystr.cp16);
			while ((ptr.cp16 = memcasememw(begin.cp16, end.cp16 - begin.cp16,
			                               findstr.cp16, findlen,
			                               &match_length)) != NULL) {
				if (unlikely(unicode_printer_print16(&p, begin.cp16, (size_t)(ptr.cp16 - begin.cp16)) < 0) ||
				    unlikely(unicode_printer_printstring(&p, (DeeObject *)replace) < 0))
					goto err_printer;
				if unlikely(!--max_count)
					break;
				begin.cp16 = ptr.cp16 + match_length;
			}
			if (UNICODE_PRINTER_ISEMPTY(&p) && begin.cp16 == mystr.cp16)
				goto retself;
			if unlikely(unicode_printer_print16(&p, begin.cp16, (size_t)(end.cp16 - begin.cp16)) < 0)
				goto err_printer;
			break;

		CASE_WIDTH_4BYTE:
			mystr.cp32 = DeeString_As4Byte((DeeObject *)self);
			if unlikely(!mystr.cp32)
				goto err_printer;
			findstr.cp32 = DeeString_As4Byte((DeeObject *)find);
			if unlikely(!findstr.cp32)
				goto err_printer;
			findlen = WSTR_LENGTH(findstr.cp32);
			if unlikely(findlen > WSTR_LENGTH(mystr.cp32))
				goto retself;
			if unlikely(!findlen)
				goto retrepl_if_self;
			begin.cp32 = mystr.cp32;
			end.cp32   = begin.cp32 + WSTR_LENGTH(mystr.cp32);
			while ((ptr.cp32 = memcasememl(begin.cp32, end.cp32 - begin.cp32,
			                               findstr.cp32, findlen,
			                               &match_length)) != NULL) {
				if (unlikely(unicode_printer_print32(&p, begin.cp32, (size_t)(ptr.cp32 - begin.cp32)) < 0) ||
				    unlikely(unicode_printer_printstring(&p, (DeeObject *)replace) < 0))
					goto err_printer;
				if unlikely(!--max_count)
					break;
				begin.cp32 = ptr.cp32 + match_length;
			}
			if (UNICODE_PRINTER_ISEMPTY(&p) && begin.cp32 == mystr.cp32)
				goto retself;
			if unlikely(unicode_printer_print32(&p, begin.cp32, (size_t)(end.cp32 - begin.cp32)) < 0)
				goto err_printer;
			break;
		}
		return (DREF String *)unicode_printer_pack(&p);
err_printer:
		unicode_printer_fini(&p);
	}
err:
	return NULL;
retrepl_if_self:
	if (!DeeString_IsEmpty(self))
		goto retself;
	return_reference_(replace);
retself:
	return_reference_(self);
}

PRIVATE DREF DeeObject *DCALL
string_ord(String *__restrict self,
           size_t argc, DeeObject **__restrict argv) {
	size_t index = 0;
	if (argc) {
		if (DeeArg_Unpack(argc, argv, "Iu:ord", &index))
			goto err;
		if (index >= DeeString_WLEN(self)) {
			err_index_out_of_bounds((DeeObject *)self,
			                        index,
			                        DeeString_WLEN(self));
			goto err;
		}
	} else if unlikely(DeeString_WLEN(self) != 1) {
		err_expected_single_character_string((DeeObject *)self);
		goto err;
	}
	return DeeInt_NewU32(DeeString_GetChar((DeeObject *)self, index));
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
string_bytes(String *__restrict self, size_t argc,
             DeeObject **__restrict argv) {
	bool allow_invalid = false;
	size_t start = 0, end = (size_t)-1;
	uint8_t *my_bytes;
	if (argc == 1) {
		int temp = DeeObject_Bool(argv[0]);
		if unlikely(temp < 0)
			goto err;
		allow_invalid = !!temp;
	} else {
		if (DeeArg_Unpack(argc, argv, "|IdIdb:Bytes", &start, &end, &allow_invalid))
			goto err;
	}
	my_bytes = DeeString_AsBytes((DeeObject *)self, allow_invalid);
	if unlikely(!my_bytes)
		goto err;
	if (start > WSTR_LENGTH(my_bytes))
		start = WSTR_LENGTH(my_bytes);
	if (end > WSTR_LENGTH(my_bytes))
		end = WSTR_LENGTH(my_bytes);
	return DeeBytes_NewView((DeeObject *)self,
	                        my_bytes + start,
	                        end - start,
	                        Dee_BUFFER_FREADONLY);
err:
	return NULL;
}

#ifndef PRIVATE_SUBSTR_KWLIST_DEFINED
#define PRIVATE_SUBSTR_KWLIST_DEFINED 1
PRIVATE struct keyword substr_kwlist[] = { K(start), K(end), KEND };
#endif /* !PRIVATE_SUBSTR_KWLIST_DEFINED */



#define DEFINE_STRING_TRAIT(name, function, test_ch)                    \
	PRIVATE DREF DeeObject *DCALL                                       \
	string_##name(String *__restrict self,                              \
	              size_t argc, DeeObject **__restrict argv) {           \
		size_t start = 0, end = (size_t)-1;                             \
		if (argc == 1) {                                                \
			uint32_t ch;                                                \
			if (DeeObject_AsSize(argv[0], &start))                      \
				goto err;                                               \
			if unlikely(start >= DeeString_WLEN(self)) {                \
				err_index_out_of_bounds((DeeObject *)self,              \
				                        start,                          \
				                        DeeString_WLEN(self));          \
				goto err;                                               \
			}                                                           \
			ch = DeeString_GetChar(self, start);                        \
			return_bool(test_ch);                                       \
		} else {                                                        \
			if (DeeArg_Unpack(argc, argv, "|IdId" #name, &start, &end)) \
				goto err;                                               \
			return_bool(function((DeeObject *)self, start, end));       \
		}                                                               \
	err:                                                                \
		return NULL;                                                    \
	}
#define DEFINE_ANY_STRING_TRAIT(name, function)                                          \
	PRIVATE DREF DeeObject *DCALL                                                        \
	string_##name(String *__restrict self, size_t argc,                                  \
	              DeeObject **__restrict argv, DeeObject *kw) {                          \
		size_t start = 0, end = (size_t)-1;                                              \
		if (DeeArg_UnpackKw(argc, argv, kw, substr_kwlist, "|IdId" #name, &start, &end)) \
			return NULL;                                                                 \
		return_bool(function((DeeObject *)self, start, end));                            \
	}
DEFINE_STRING_TRAIT(isprint, DeeString_IsPrint, DeeUni_Flags(ch) & UNICODE_FPRINT)
DEFINE_STRING_TRAIT(isalpha, DeeString_IsAlpha, DeeUni_Flags(ch) & UNICODE_FALPHA)
DEFINE_STRING_TRAIT(isspace, DeeString_IsSpace, DeeUni_Flags(ch) & UNICODE_FSPACE)
DEFINE_STRING_TRAIT(islf, DeeString_IsLF, DeeUni_Flags(ch) & UNICODE_FLF)
DEFINE_STRING_TRAIT(islower, DeeString_IsLower, DeeUni_Flags(ch) & UNICODE_FLOWER)
DEFINE_STRING_TRAIT(isupper, DeeString_IsUpper, DeeUni_Flags(ch) & UNICODE_FUPPER)
DEFINE_STRING_TRAIT(iscntrl, DeeString_IsCntrl, DeeUni_Flags(ch) & UNICODE_FCNTRL)
DEFINE_STRING_TRAIT(isdigit, DeeString_IsDigit, DeeUni_Flags(ch) & UNICODE_FDIGIT)
DEFINE_STRING_TRAIT(isdecimal, DeeString_IsDecimal, DeeUni_Flags(ch) & UNICODE_FDECIMAL)
DEFINE_STRING_TRAIT(issymstrt, DeeString_IsSymStrt, DeeUni_Flags(ch) & UNICODE_FSYMSTRT)
DEFINE_STRING_TRAIT(issymcont, DeeString_IsSymCont, DeeUni_Flags(ch) & UNICODE_FSYMCONT)
DEFINE_STRING_TRAIT(isalnum, DeeString_IsAlnum, DeeUni_Flags(ch) & (UNICODE_FALPHA | UNICODE_FDECIMAL))
DEFINE_STRING_TRAIT(isnumeric, DeeString_IsNumeric, DeeUni_Flags(ch) & (UNICODE_FDECIMAL | UNICODE_FDIGIT))
DEFINE_STRING_TRAIT(istitle, DeeString_IsTitle, DeeUni_Flags(ch) & UNICODE_FTITLE)
DEFINE_STRING_TRAIT(issymbol, DeeString_IsSymbol, DeeUni_Flags(ch) & UNICODE_FSYMSTRT)
DEFINE_STRING_TRAIT(isascii, DeeString_IsAscii, ch <= 0x7f)
DEFINE_ANY_STRING_TRAIT(isanyprint, DeeString_IsAnyPrint)
DEFINE_ANY_STRING_TRAIT(isanyalpha, DeeString_IsAnyAlpha)
DEFINE_ANY_STRING_TRAIT(isanyspace, DeeString_IsAnySpace)
DEFINE_ANY_STRING_TRAIT(isanylf, DeeString_IsAnyLF)
DEFINE_ANY_STRING_TRAIT(isanylower, DeeString_IsAnyLower)
DEFINE_ANY_STRING_TRAIT(isanyupper, DeeString_IsAnyUpper)
DEFINE_ANY_STRING_TRAIT(isanycntrl, DeeString_IsAnyCntrl)
DEFINE_ANY_STRING_TRAIT(isanydigit, DeeString_IsAnyDigit)
DEFINE_ANY_STRING_TRAIT(isanydecimal, DeeString_IsAnyDecimal)
DEFINE_ANY_STRING_TRAIT(isanysymstrt, DeeString_IsAnySymStrt)
DEFINE_ANY_STRING_TRAIT(isanysymcont, DeeString_IsAnySymCont)
DEFINE_ANY_STRING_TRAIT(isanyalnum, DeeString_IsAnyAlnum)
DEFINE_ANY_STRING_TRAIT(isanynumeric, DeeString_IsAnyNumeric)
DEFINE_ANY_STRING_TRAIT(isanytitle, DeeString_IsAnyTitle)
DEFINE_ANY_STRING_TRAIT(isanyascii, DeeString_IsAnyAscii)
#undef DEFINE_ANY_STRING_TRAIT
#undef DEFINE_STRING_TRAIT

PRIVATE DREF DeeObject *DCALL
string_asnumber(String *__restrict self,
                size_t argc, DeeObject **__restrict argv) {
	uint32_t ch;
	DeeObject *defl = NULL;
	struct unitraits *trt;
	if (argc == 0) {
		if unlikely(DeeString_WLEN(self) != 1)
			goto err_not_single_char;
		ch = DeeString_GetChar(self, 0);
	} else {
		size_t index;
		if (DeeArg_Unpack(argc, argv, "Iu|o:asnumber", &index, &defl))
			goto err;
		if unlikely(index >= DeeString_WLEN(self)) {
			err_index_out_of_bounds((DeeObject *)self, index,
			                        DeeString_WLEN(self));
			goto err;
		}
		ch = DeeString_GetChar(self, index);
	}
	trt = DeeUni_Descriptor(ch);
	if (!(trt->ut_flags & (UNICODE_FDIGIT | UNICODE_FDECIMAL)))
		goto err_not_numeric;
	return DeeInt_NewU8(trt->ut_digit);
err_not_numeric:
	if (defl)
		return_reference_(defl);
	DeeError_Throwf(&DeeError_ValueError,
	                "Unicode character %I32C is not numeric",
	                ch);
	goto err;
err_not_single_char:
	err_expected_single_character_string((DeeObject *)self);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
string_asdigit(String *__restrict self,
               size_t argc, DeeObject **__restrict argv) {
	uint32_t ch;
	DeeObject *defl = NULL;
	struct unitraits *trt;
	if (argc == 0) {
		if unlikely(DeeString_WLEN(self) != 1)
			goto err_not_single_char;
		ch = DeeString_GetChar(self, 0);
	} else {
		size_t index;
		if (DeeArg_Unpack(argc, argv, "Iu|o:asdigit", &index, &defl))
			goto err;
		if unlikely(index >= DeeString_WLEN(self)) {
			err_index_out_of_bounds((DeeObject *)self, index,
			                        DeeString_WLEN(self));
			goto err;
		}
		ch = DeeString_GetChar(self, index);
	}
	trt = DeeUni_Descriptor(ch);
	if (!(trt->ut_flags & UNICODE_FDIGIT))
		goto err_not_numeric;
	return DeeInt_NewU8(trt->ut_digit);
err_not_numeric:
	if (defl)
		return_reference_(defl);
	DeeError_Throwf(&DeeError_ValueError,
	                "Unicode character %I32C isn't a digit",
	                ch);
	goto err;
err_not_single_char:
	err_expected_single_character_string((DeeObject *)self);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
string_asdecimal(String *__restrict self,
                 size_t argc, DeeObject **__restrict argv) {
	uint32_t ch;
	DeeObject *defl = NULL;
	struct unitraits *trt;
	if (argc == 0) {
		if unlikely(DeeString_WLEN(self) != 1)
			goto err_not_single_char;
		ch = DeeString_GetChar(self, 0);
	} else {
		size_t index;
		if (DeeArg_Unpack(argc, argv, "Iu|o:asdecimal", &index, &defl))
			goto err;
		if unlikely(index >= DeeString_WLEN(self)) {
			err_index_out_of_bounds((DeeObject *)self, index,
			                        DeeString_WLEN(self));
			goto err;
		}
		ch = DeeString_GetChar(self, index);
	}
	trt = DeeUni_Descriptor(ch);
	if (!(trt->ut_flags & UNICODE_FDECIMAL))
		goto err_not_numeric;
	return DeeInt_NewU8(trt->ut_digit);
err_not_numeric:
	if (defl)
		return_reference_(defl);
	DeeError_Throwf(&DeeError_ValueError,
	                "Unicode character %I32C isn't a decimal",
	                ch);
	goto err;
err_not_single_char:
	err_expected_single_character_string((DeeObject *)self);
err:
	return NULL;
}


INTDEF DREF DeeObject *DCALL DeeString_Convert(DeeObject *__restrict self, size_t start, size_t end, uintptr_t kind);
INTDEF DREF DeeObject *DCALL DeeString_ToTitle(DeeObject *__restrict self, size_t start, size_t end);
INTDEF DREF DeeObject *DCALL DeeString_Capitalize(DeeObject *__restrict self, size_t start, size_t end);
INTDEF DREF DeeObject *DCALL DeeString_Swapcase(DeeObject *__restrict self, size_t start, size_t end);
#define DeeString_ToLower(self, start, end) DeeString_Convert(self, start, end, UNICODE_CONVERT_LOWER)
#define DeeString_ToUpper(self, start, end) DeeString_Convert(self, start, end, UNICODE_CONVERT_UPPER)


PRIVATE DREF DeeObject *DCALL
string_lower(String *__restrict self, size_t argc,
             DeeObject **__restrict argv, DeeObject *kw) {
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, substr_kwlist, "|IdId:lower", &start, &end))
		return NULL;
	return DeeString_ToLower((DeeObject *)self, start, end);
}

PRIVATE DREF DeeObject *DCALL
string_upper(String *__restrict self, size_t argc,
             DeeObject **__restrict argv, DeeObject *kw) {
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, substr_kwlist, "|IdId:upper", &start, &end))
		return NULL;
	return DeeString_ToUpper((DeeObject *)self, start, end);
}

PRIVATE DREF DeeObject *DCALL
string_title(String *__restrict self, size_t argc,
             DeeObject **__restrict argv, DeeObject *kw) {
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, substr_kwlist, "|IdId:title", &start, &end))
		return NULL;
	return DeeString_ToTitle((DeeObject *)self, start, end);
}

PRIVATE DREF DeeObject *DCALL
string_capitalize(String *__restrict self, size_t argc,
                  DeeObject **__restrict argv, DeeObject *kw) {
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, substr_kwlist, "|IdId:capitalize", &start, &end))
		return NULL;
	return DeeString_Capitalize((DeeObject *)self, start, end);
}

PRIVATE DREF DeeObject *DCALL
string_swapcase(String *__restrict self, size_t argc,
                DeeObject **__restrict argv, DeeObject *kw) {
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, substr_kwlist, "|IdId:swapcase", &start, &end))
		return NULL;
	return DeeString_Swapcase((DeeObject *)self, start, end);
}

PRIVATE DREF DeeObject *DCALL
string_casefold(String *__restrict self, size_t argc,
                DeeObject **__restrict argv, DeeObject *kw) {
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, substr_kwlist, "|IdId:casefold", &start, &end))
		goto err;
	{
		struct unicode_printer printer = UNICODE_PRINTER_INIT;
		union dcharptr my_iter, my_flush_start, my_end;
		uint32_t buf[UNICODE_FOLDED_MAX];
		size_t foldlen;
		SWITCH_SIZEOF_WIDTH(DeeString_WIDTH(self)) {

		CASE_WIDTH_1BYTE:
			my_iter.cp8 = DeeString_Get1Byte((DeeObject *)self);
			if (end > WSTR_LENGTH(my_iter.cp8))
				end = WSTR_LENGTH(my_iter.cp8);
			if unlikely(start >= end)
				goto return_empty;
			my_end.cp8 = my_iter.cp8 + end;
			my_iter.cp8 += start;
			my_flush_start.cp8 = my_iter.cp8;
			unicode_printer_allocate(&printer, (size_t)(my_end.cp8 - my_iter.cp8),
			                         STRING_WIDTH_1BYTE);
			for (; my_iter.cp8 < my_end.cp8; ++my_iter.cp8) {
				uint8_t ch = *my_iter.cp8;
				foldlen    = DeeUni_ToFolded(ch, buf);
				if (foldlen == 1 && buf[0] == (uint32_t)ch)
					continue; /* The character representation doesn't change. */
				/* Flush all unwritten data. */
				if unlikely(unicode_printer_print8(&printer, my_flush_start.cp8,
					                                (size_t)(my_iter.cp8 - my_flush_start.cp8)) < 0)
				goto err_printer;
				/* Print the case-folded character representation. */
				if unlikely(unicode_printer_print32(&printer, buf, foldlen) < 0)
					goto err_printer;
				my_flush_start.cp8 = my_iter.cp8 + 1;
			}
			/* Optimization: don't return a new string if nothing was folded. */
			if (UNICODE_PRINTER_ISEMPTY(&printer))
				goto return_self;
			/* Flush the remainder. */
			if unlikely(unicode_printer_print8(&printer, my_flush_start.cp8,
				                                (size_t)(my_end.cp8 - my_flush_start.cp8)) < 0)
			goto err_printer;
			break;

		CASE_WIDTH_2BYTE:
			my_iter.cp16 = DeeString_Get2Byte((DeeObject *)self);
			if (end > WSTR_LENGTH(my_iter.cp16))
				end = WSTR_LENGTH(my_iter.cp16);
			if unlikely(start >= end)
				goto return_empty;
			my_end.cp16 = my_iter.cp16 + end;
			my_iter.cp16 += start;
			my_flush_start.cp16 = my_iter.cp16;
			unicode_printer_allocate(&printer, (size_t)(my_end.cp16 - my_iter.cp16),
			                         STRING_WIDTH_2BYTE);
			for (; my_iter.cp16 < my_end.cp16; ++my_iter.cp16) {
				uint16_t ch = *my_iter.cp16;
				foldlen     = DeeUni_ToFolded(ch, buf);
				if (foldlen == 1 && buf[0] == (uint32_t)ch)
					continue; /* The character representation doesn't change. */
				/* Flush all unwritten data. */
				if unlikely(unicode_printer_print16(&printer, my_flush_start.cp16,
					                                 (size_t)(my_iter.cp16 - my_flush_start.cp16)) < 0)
				goto err_printer;
				/* Print the case-folded character representation. */
				if unlikely(unicode_printer_print32(&printer, buf, foldlen) < 0)
					goto err_printer;
				my_flush_start.cp16 = my_iter.cp16 + 1;
			}
			/* Optimization: don't return a new string if nothing was folded. */
			if (UNICODE_PRINTER_ISEMPTY(&printer))
				goto return_self;
			/* Flush the remainder. */
			if unlikely(unicode_printer_print16(&printer, my_flush_start.cp16,
				                                 (size_t)(my_end.cp16 - my_flush_start.cp16)) < 0)
			goto err_printer;
			break;

		CASE_WIDTH_4BYTE:
			my_iter.cp32 = DeeString_Get4Byte((DeeObject *)self);
			if (end > WSTR_LENGTH(my_iter.cp32))
				end = WSTR_LENGTH(my_iter.cp32);
			if unlikely(start >= end)
				goto return_empty;
			my_end.cp32 = my_iter.cp32 + end;
			my_iter.cp32 += start;
			my_flush_start.cp32 = my_iter.cp32;
			unicode_printer_allocate(&printer, (size_t)(my_end.cp32 - my_iter.cp32),
			                         STRING_WIDTH_4BYTE);
			for (; my_iter.cp32 < my_end.cp32; ++my_iter.cp32) {
				uint32_t ch = *my_iter.cp32;
				foldlen     = DeeUni_ToFolded(ch, buf);
				if (foldlen == 1 && buf[0] == (uint32_t)ch)
					continue; /* The character representation doesn't change. */
				/* Flush all unwritten data. */
				if unlikely(unicode_printer_print32(&printer, my_flush_start.cp32,
					                                 (size_t)(my_iter.cp32 - my_flush_start.cp32)) < 0)
				goto err_printer;
				/* Print the case-folded character representation. */
				if unlikely(unicode_printer_print32(&printer, buf, foldlen) < 0)
					goto err_printer;
				my_flush_start.cp32 = my_iter.cp32 + 1;
			}
			/* Optimization: don't return a new string if nothing was folded. */
			if (UNICODE_PRINTER_ISEMPTY(&printer))
				goto return_self;
			/* Flush the remainder. */
			if unlikely(unicode_printer_print32(&printer, my_flush_start.cp32,
				                                 (size_t)(my_end.cp32 - my_flush_start.cp32)) < 0)
			goto err_printer;
			break;
		}
		return unicode_printer_pack(&printer);
return_self:
		unicode_printer_fini(&printer);
		return_reference_((DeeObject *)self);
return_empty:
		unicode_printer_fini(&printer);
		return_empty_string;
err_printer:
		unicode_printer_fini(&printer);
	}
err:
	return NULL;
}


#ifndef PRIVATE_FIND_KWLIST_DEFINED
#define PRIVATE_FIND_KWLIST_DEFINED 1
PRIVATE struct keyword find_kwlist[] = { K(needle), K(start), K(end), KEND };
#endif /* !PRIVATE_FIND_KWLIST_DEFINED */

PRIVATE DREF DeeObject *DCALL
string_find(String *__restrict self, size_t argc,
            DeeObject **__restrict argv, DeeObject *kw) {
	String *other;
	size_t begin = 0, end = (size_t)-1;
	size_t result;
	union dcharptr ptr, lhs, rhs;
	size_t mylen;
	if (DeeArg_UnpackKw(argc, argv, kw, find_kwlist, "o|IdId:find", &other, &begin, &end) ||
	    DeeObject_AssertTypeExact((DeeObject *)other, &DeeString_Type))
		goto err;
	SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON(DeeString_WIDTH(self),
	                                        DeeString_WIDTH(other))) {

	CASE_WIDTH_1BYTE:
		lhs.cp8 = DeeString_As1Byte((DeeObject *)self);
		rhs.cp8 = DeeString_As1Byte((DeeObject *)other);
		mylen   = WSTR_LENGTH(lhs.ptr);
		if unlikely(begin > mylen)
			begin = mylen;
		if likely(end > mylen)
			end = mylen;
		if unlikely(end <= begin)
			goto not_found;
		ptr.cp8 = memmemb(lhs.cp8 + begin, end - begin,
		                  rhs.cp8, WSTR_LENGTH(rhs.cp8));
		if (!ptr.cp8)
			goto not_found;
		result = (size_t)(ptr.cp8 - lhs.cp8);
		break;

	CASE_WIDTH_2BYTE:
		lhs.cp16 = DeeString_As2Byte((DeeObject *)self);
		if unlikely(!lhs.cp16)
			goto err;
		rhs.cp16 = DeeString_As2Byte((DeeObject *)other);
		if unlikely(!rhs.cp16)
			goto err;
		mylen = WSTR_LENGTH(lhs.ptr);
		if unlikely(begin > mylen)
			begin = mylen;
		if likely(end > mylen)
			end = mylen;
		if unlikely(end <= begin)
			goto not_found;
		ptr.cp16 = memmemw(lhs.cp16 + begin, end - begin,
		                   rhs.cp16, WSTR_LENGTH(rhs.cp16));
		if (!ptr.cp16)
			goto not_found;
		result = (size_t)(ptr.cp16 - lhs.cp16);
		break;

	CASE_WIDTH_4BYTE:
		lhs.cp32 = DeeString_As4Byte((DeeObject *)self);
		if unlikely(!lhs.cp32)
			goto err;
		rhs.cp32 = DeeString_As4Byte((DeeObject *)other);
		if unlikely(!rhs.cp32)
			goto err;
		mylen = WSTR_LENGTH(lhs.ptr);
		if unlikely(begin > mylen)
			begin = mylen;
		if likely(end > mylen)
			end = mylen;
		if unlikely(end <= begin)
			goto not_found;
		ptr.cp32 = memmeml(lhs.cp32 + begin, end - begin,
		                   rhs.cp32, WSTR_LENGTH(rhs.cp32));
		if (!ptr.cp32)
			goto not_found;
		result = (size_t)(ptr.cp32 - lhs.cp32);
		break;
	}
	return DeeInt_NewSize(result);
not_found:
	return_reference_(&DeeInt_MinusOne);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
string_rfind(String *__restrict self, size_t argc,
             DeeObject **__restrict argv, DeeObject *kw) {
	String *other;
	size_t begin = 0, end = (size_t)-1;
	size_t result;
	union dcharptr ptr, lhs, rhs;
	size_t mylen;
	if (DeeArg_UnpackKw(argc, argv, kw, find_kwlist, "o|IdId:rfind", &other, &begin, &end) ||
	    DeeObject_AssertTypeExact((DeeObject *)other, &DeeString_Type))
		goto err;
	SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON(DeeString_WIDTH(self),
	                                        DeeString_WIDTH(other))) {

	CASE_WIDTH_1BYTE:
		lhs.cp8 = DeeString_As1Byte((DeeObject *)self);
		rhs.cp8 = DeeString_As1Byte((DeeObject *)other);
		mylen   = WSTR_LENGTH(lhs.ptr);
		if unlikely(begin > mylen)
			begin = mylen;
		if likely(end > mylen)
			end = mylen;
		if unlikely(end <= begin)
			goto not_found;
		ptr.cp8 = memrmemb(lhs.cp8 + begin, end - begin,
		                   rhs.cp8, WSTR_LENGTH(rhs.cp8));
		if (!ptr.cp8)
			goto not_found;
		result = (size_t)(ptr.cp8 - lhs.cp8);
		break;

	CASE_WIDTH_2BYTE:
		lhs.cp16 = DeeString_As2Byte((DeeObject *)self);
		if unlikely(!lhs.cp16)
			goto err;
		rhs.cp16 = DeeString_As2Byte((DeeObject *)other);
		if unlikely(!rhs.cp16)
			goto err;
		mylen = WSTR_LENGTH(lhs.ptr);
		if unlikely(begin > mylen)
			begin = mylen;
		if likely(end > mylen)
			end = mylen;
		if unlikely(end <= begin)
			goto not_found;
		ptr.cp16 = memrmemw(lhs.cp16 + begin, end - begin,
		                    rhs.cp16, WSTR_LENGTH(rhs.cp16));
		if (!ptr.cp16)
			goto not_found;
		result = (size_t)(ptr.cp16 - lhs.cp16);
		break;

	CASE_WIDTH_4BYTE:
		lhs.cp32 = DeeString_As4Byte((DeeObject *)self);
		if unlikely(!lhs.cp32)
			goto err;
		rhs.cp32 = DeeString_As4Byte((DeeObject *)other);
		if unlikely(!rhs.cp32)
			goto err;
		mylen = WSTR_LENGTH(lhs.ptr);
		if unlikely(begin > mylen)
			begin = mylen;
		if likely(end > mylen)
			end = mylen;
		if unlikely(end <= begin)
			goto not_found;
		ptr.cp32 = memrmeml(lhs.cp32 + begin, end - begin,
		                    rhs.cp32, WSTR_LENGTH(rhs.cp32));
		if (!ptr.cp32)
			goto not_found;
		result = (size_t)(ptr.cp32 - lhs.cp32);
		break;
	}
	return DeeInt_NewSize(result);
not_found:
	return_reference_(&DeeInt_MinusOne);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
string_index(String *__restrict self, size_t argc,
             DeeObject **__restrict argv, DeeObject *kw) {
	String *other;
	size_t begin = 0, end = (size_t)-1;
	size_t result;
	union dcharptr ptr, lhs, rhs;
	size_t mylen;
	if (DeeArg_UnpackKw(argc, argv, kw, find_kwlist, "o|IdId:index", &other, &begin, &end) ||
	    DeeObject_AssertTypeExact((DeeObject *)other, &DeeString_Type))
		goto err;
	SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON(DeeString_WIDTH(self),
	                                        DeeString_WIDTH(other))) {

	CASE_WIDTH_1BYTE:
		lhs.cp8 = DeeString_As1Byte((DeeObject *)self);
		rhs.cp8 = DeeString_As1Byte((DeeObject *)other);
		mylen   = WSTR_LENGTH(lhs.ptr);
		if unlikely(begin > mylen)
			begin = mylen;
		if likely(end > mylen)
			end = mylen;
		if unlikely(end <= begin)
			goto not_found;
		ptr.cp8 = memmemb(lhs.cp8 + begin, end - begin,
		                  rhs.cp8, WSTR_LENGTH(rhs.cp8));
		if unlikely(!ptr.cp8)
			goto not_found;
		result = (size_t)(ptr.cp8 - lhs.cp8);
		break;

	CASE_WIDTH_2BYTE:
		lhs.cp16 = DeeString_As2Byte((DeeObject *)self);
		if unlikely(!lhs.cp16)
			goto err;
		rhs.cp16 = DeeString_As2Byte((DeeObject *)other);
		if unlikely(!rhs.cp16)
			goto err;
		mylen = WSTR_LENGTH(lhs.ptr);
		if unlikely(begin > mylen)
			begin = mylen;
		if likely(end > mylen)
			end = mylen;
		if unlikely(end <= begin)
			goto not_found;
		ptr.cp16 = memmemw(lhs.cp16 + begin, end - begin,
		                   rhs.cp16, WSTR_LENGTH(rhs.cp16));
		if unlikely(!ptr.cp16)
			goto not_found;
		result = (size_t)(ptr.cp16 - lhs.cp16);
		break;

	CASE_WIDTH_4BYTE:
		lhs.cp32 = DeeString_As4Byte((DeeObject *)self);
		if unlikely(!lhs.cp32)
			goto err;
		rhs.cp32 = DeeString_As4Byte((DeeObject *)other);
		if unlikely(!rhs.cp32)
			goto err;
		mylen = WSTR_LENGTH(lhs.ptr);
		if unlikely(begin > mylen)
			begin = mylen;
		if likely(end > mylen)
			end = mylen;
		if unlikely(end <= begin)
			goto not_found;
		ptr.cp32 = memmeml(lhs.cp32 + begin, end - begin,
		                   rhs.cp32, WSTR_LENGTH(rhs.cp32));
		if unlikely(!ptr.cp32)
			goto not_found;
		result = (size_t)(ptr.cp32 - lhs.cp32);
		break;
	}
	return DeeInt_NewSize(result);
not_found:
	err_index_not_found((DeeObject *)self,
	                    (DeeObject *)other);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
string_rindex(String *__restrict self, size_t argc,
              DeeObject **__restrict argv, DeeObject *kw) {
	String *other;
	size_t begin = 0, end = (size_t)-1;
	size_t result;
	union dcharptr ptr, lhs, rhs;
	size_t mylen;
	if (DeeArg_UnpackKw(argc, argv, kw, find_kwlist, "o|IdId:rindex", &other, &begin, &end) ||
	    DeeObject_AssertTypeExact((DeeObject *)other, &DeeString_Type))
		goto err;
	SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON(DeeString_WIDTH(self),
	                                        DeeString_WIDTH(other))) {

	CASE_WIDTH_1BYTE:
		lhs.cp8 = DeeString_As1Byte((DeeObject *)self);
		rhs.cp8 = DeeString_As1Byte((DeeObject *)other);
		mylen   = WSTR_LENGTH(lhs.ptr);
		if unlikely(begin > mylen)
			begin = mylen;
		if likely(end > mylen)
			end = mylen;
		if unlikely(end <= begin)
			goto not_found;
		ptr.cp8 = memrmemb(lhs.cp8 + begin, end - begin,
		                   rhs.cp8, WSTR_LENGTH(rhs.cp8));
		if unlikely(!ptr.cp8)
			goto not_found;
		result = (size_t)(ptr.cp8 - lhs.cp8);
		break;

	CASE_WIDTH_2BYTE:
		lhs.cp16 = DeeString_As2Byte((DeeObject *)self);
		if unlikely(!lhs.cp16)
			goto err;
		rhs.cp16 = DeeString_As2Byte((DeeObject *)other);
		if unlikely(!rhs.cp16)
			goto err;
		mylen = WSTR_LENGTH(lhs.ptr);
		if unlikely(begin > mylen)
			begin = mylen;
		if likely(end > mylen)
			end = mylen;
		if unlikely(end <= begin)
			goto not_found;
		ptr.cp16 = memrmemw(lhs.cp16 + begin, end - begin,
		                    rhs.cp16, WSTR_LENGTH(rhs.cp16));
		if unlikely(!ptr.cp16)
			goto not_found;
		result = (size_t)(ptr.cp16 - lhs.cp16);
		break;

	CASE_WIDTH_4BYTE:
		lhs.cp32 = DeeString_As4Byte((DeeObject *)self);
		if unlikely(!lhs.cp32)
			goto err;
		rhs.cp32 = DeeString_As4Byte((DeeObject *)other);
		if unlikely(!rhs.cp32)
			goto err;
		mylen = WSTR_LENGTH(lhs.ptr);
		if unlikely(begin > mylen)
			begin = mylen;
		if likely(end > mylen)
			end = mylen;
		if unlikely(end <= begin)
			goto not_found;
		ptr.cp32 = memrmeml(lhs.cp32 + begin, end - begin,
		                    rhs.cp32, WSTR_LENGTH(rhs.cp32));
		if unlikely(!ptr.cp32)
			goto not_found;
		result = (size_t)(ptr.cp32 - lhs.cp32);
		break;
	}
	return DeeInt_NewSize(result);
not_found:
	err_index_not_found((DeeObject *)self,
	                    (DeeObject *)other);
err:
	return NULL;
}
INTDEF DREF DeeObject *DCALL
DeeString_FindAll(String *self, String *other,
                  size_t start, size_t end);
INTDEF DREF DeeObject *DCALL
DeeString_CaseFindAll(String *self, String *other,
                      size_t start, size_t end);

PRIVATE DREF DeeObject *DCALL
string_findall(String *__restrict self, size_t argc,
               DeeObject **__restrict argv, DeeObject *kw) {
	String *other;
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, find_kwlist, "o|IdId:findall", &other, &start, &end) ||
	    DeeObject_AssertTypeExact((DeeObject *)other, &DeeString_Type))
		return NULL;
	return DeeString_FindAll(self, other, start, end);
}

PRIVATE DREF DeeObject *DCALL
string_casefindall(String *__restrict self, size_t argc,
                   DeeObject **__restrict argv, DeeObject *kw) {
	String *other;
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, find_kwlist, "o|IdId:casefindall", &other, &start, &end) ||
	    DeeObject_AssertTypeExact((DeeObject *)other, &DeeString_Type))
		return NULL;
	return DeeString_CaseFindAll(self, other, start, end);
}

PRIVATE DREF DeeObject *DCALL
string_casefind(String *__restrict self, size_t argc,
                DeeObject **__restrict argv, DeeObject *kw) {
	String *other;
	size_t start = 0, end = (size_t)-1;
	size_t mylen, result, match_length;
	union dcharptr ptr, lhs, rhs;
	if (DeeArg_UnpackKw(argc, argv, kw, find_kwlist, "o|IdId:casefind", &other, &start, &end) ||
	    DeeObject_AssertTypeExact((DeeObject *)other, &DeeString_Type))
		goto err;
	SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON(DeeString_WIDTH(self),
	                                        DeeString_WIDTH(other))) {

	CASE_WIDTH_1BYTE:
		lhs.cp8 = DeeString_As1Byte((DeeObject *)self);
		rhs.cp8 = DeeString_As1Byte((DeeObject *)other);
		mylen   = WSTR_LENGTH(lhs.ptr);
		if unlikely(start > mylen)
			start = mylen;
		if likely(end > mylen)
			end = mylen;
		if unlikely(end <= start)
			goto not_found;
		ptr.cp8 = memcasememb(lhs.cp8 + start, end - start,
		                      rhs.cp8, WSTR_LENGTH(rhs.cp8),
		                      &match_length);
		if (!ptr.cp8)
			goto not_found;
		result = ptr.cp8 - lhs.cp8;
		break;

	CASE_WIDTH_2BYTE:
		lhs.cp16 = DeeString_As2Byte((DeeObject *)self);
		if unlikely(!lhs.cp16)
			goto err;
		rhs.cp16 = DeeString_As2Byte((DeeObject *)other);
		if unlikely(!rhs.cp16)
			goto err;
		mylen = WSTR_LENGTH(lhs.ptr);
		if unlikely(start > mylen)
			start = mylen;
		if likely(end > mylen)
			end = mylen;
		if unlikely(end <= start)
			goto not_found;
		ptr.cp16 = memcasememw(lhs.cp16 + start, end - start,
		                       rhs.cp16, WSTR_LENGTH(rhs.cp16),
		                       &match_length);
		if (!ptr.cp16)
			goto not_found;
		result = ptr.cp16 - lhs.cp16;
		break;

	CASE_WIDTH_4BYTE:
		lhs.cp32 = DeeString_As4Byte((DeeObject *)self);
		if unlikely(!lhs.cp32)
			goto err;
		rhs.cp32 = DeeString_As4Byte((DeeObject *)other);
		if unlikely(!rhs.cp32)
			goto err;
		mylen = WSTR_LENGTH(lhs.ptr);
		if unlikely(start > mylen)
			start = mylen;
		if likely(end > mylen)
			end = mylen;
		if unlikely(end <= start)
			goto not_found;
		ptr.cp32 = memcasememl(lhs.cp32 + start, end - start,
		                       rhs.cp32, WSTR_LENGTH(rhs.cp32),
		                       &match_length);
		if (!ptr.cp32)
			goto not_found;
		result = ptr.cp32 - lhs.cp32;
		break;
	}
	return DeeTuple_Newf(DEE_FMT_SIZE_T
	                     DEE_FMT_SIZE_T,
	                     result,
	                     result + match_length);
not_found:
	return_none;
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
string_caserfind(String *__restrict self, size_t argc,
                 DeeObject **__restrict argv, DeeObject *kw) {
	String *other;
	size_t start = 0, end = (size_t)-1;
	size_t mylen, result, match_length;
	union dcharptr ptr, lhs, rhs;
	if (DeeArg_UnpackKw(argc, argv, kw, find_kwlist, "o|IdId:caserfind", &other, &start, &end) ||
	    DeeObject_AssertTypeExact((DeeObject *)other, &DeeString_Type))
		goto err;
	SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON(DeeString_WIDTH(self),
	                                        DeeString_WIDTH(other))) {

	CASE_WIDTH_1BYTE:
		lhs.cp8 = DeeString_As1Byte((DeeObject *)self);
		rhs.cp8 = DeeString_As1Byte((DeeObject *)other);
		mylen   = WSTR_LENGTH(lhs.ptr);
		if unlikely(start > mylen)
			start = mylen;
		if likely(end > mylen)
			end = mylen;
		if unlikely(end <= start)
			goto not_found;
		ptr.cp8 = memcasermemb(lhs.cp8 + start, end - start,
		                       rhs.cp8, WSTR_LENGTH(rhs.cp8),
		                       &match_length);
		if (!ptr.cp8)
			goto not_found;
		result = ptr.cp8 - lhs.cp8;
		break;

	CASE_WIDTH_2BYTE:
		lhs.cp16 = DeeString_As2Byte((DeeObject *)self);
		if unlikely(!lhs.cp16)
			goto err;
		rhs.cp16 = DeeString_As2Byte((DeeObject *)other);
		if unlikely(!rhs.cp16)
			goto err;
		mylen = WSTR_LENGTH(lhs.ptr);
		if unlikely(start > mylen)
			start = mylen;
		if likely(end > mylen)
			end = mylen;
		if unlikely(end <= start)
			goto not_found;
		ptr.cp16 = memcasermemw(lhs.cp16 + start, end - start,
		                        rhs.cp16, WSTR_LENGTH(rhs.cp16),
		                        &match_length);
		if (!ptr.cp16)
			goto not_found;
		result = ptr.cp16 - lhs.cp16;
		break;

	CASE_WIDTH_4BYTE:
		lhs.cp32 = DeeString_As4Byte((DeeObject *)self);
		if unlikely(!lhs.cp32)
			goto err;
		rhs.cp32 = DeeString_As4Byte((DeeObject *)other);
		if unlikely(!rhs.cp32)
			goto err;
		mylen = WSTR_LENGTH(lhs.ptr);
		if unlikely(start > mylen)
			start = mylen;
		if likely(end > mylen)
			end = mylen;
		if unlikely(end <= start)
			goto not_found;
		ptr.cp32 = memcasermeml(lhs.cp32 + start, end - start,
		                        rhs.cp32, WSTR_LENGTH(rhs.cp32),
		                        &match_length);
		if (!ptr.cp32)
			goto not_found;
		result = ptr.cp32 - lhs.cp32;
		break;
	}
	return DeeTuple_Newf(DEE_FMT_SIZE_T
	                     DEE_FMT_SIZE_T,
	                     result,
	                     result + match_length);
not_found:
	return_none;
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
string_caseindex(String *__restrict self, size_t argc,
                 DeeObject **__restrict argv, DeeObject *kw) {
	String *other;
	size_t start = 0, end = (size_t)-1;
	size_t mylen, result, match_length;
	union dcharptr ptr, lhs, rhs;
	if (DeeArg_UnpackKw(argc, argv, kw, find_kwlist, "o|IdId:caseindex", &other, &start, &end) ||
	    DeeObject_AssertTypeExact((DeeObject *)other, &DeeString_Type))
		goto err;
	SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON(DeeString_WIDTH(self),
	                                        DeeString_WIDTH(other))) {

	CASE_WIDTH_1BYTE:
		lhs.cp8 = DeeString_As1Byte((DeeObject *)self);
		rhs.cp8 = DeeString_As1Byte((DeeObject *)other);
		mylen   = WSTR_LENGTH(lhs.ptr);
		if unlikely(start > mylen)
			start = mylen;
		if likely(end > mylen)
			end = mylen;
		if unlikely(end <= start)
			goto not_found;
		ptr.cp8 = memcasememb(lhs.cp8 + start, end - start,
		                      rhs.cp8, WSTR_LENGTH(rhs.cp8),
		                      &match_length);
		if unlikely(!ptr.cp8)
			goto not_found;
		result = ptr.cp8 - lhs.cp8;
		break;

	CASE_WIDTH_2BYTE:
		lhs.cp16 = DeeString_As2Byte((DeeObject *)self);
		if unlikely(!lhs.cp16)
			goto err;
		rhs.cp16 = DeeString_As2Byte((DeeObject *)other);
		if unlikely(!rhs.cp16)
			goto err;
		mylen = WSTR_LENGTH(lhs.ptr);
		if unlikely(start > mylen)
			start = mylen;
		if likely(end > mylen)
			end = mylen;
		if unlikely(end <= start)
			goto not_found;
		ptr.cp16 = memcasememw(lhs.cp16 + start, end - start,
		                       rhs.cp16, WSTR_LENGTH(rhs.cp16),
		                       &match_length);
		if unlikely(!ptr.cp16)
			goto not_found;
		result = ptr.cp16 - lhs.cp16;
		break;

	CASE_WIDTH_4BYTE:
		lhs.cp32 = DeeString_As4Byte((DeeObject *)self);
		if unlikely(!lhs.cp32)
			goto err;
		rhs.cp32 = DeeString_As4Byte((DeeObject *)other);
		if unlikely(!rhs.cp32)
			goto err;
		mylen = WSTR_LENGTH(lhs.ptr);
		if unlikely(start > mylen)
			start = mylen;
		if likely(end > mylen)
			end = mylen;
		if unlikely(end <= start)
			goto not_found;
		ptr.cp32 = memcasememl(lhs.cp32 + start, end - start,
		                       rhs.cp32, WSTR_LENGTH(rhs.cp32),
		                       &match_length);
		if unlikely(!ptr.cp32)
			goto not_found;
		result = ptr.cp32 - lhs.cp32;
		break;
	}
	return DeeTuple_Newf(DEE_FMT_SIZE_T
	                     DEE_FMT_SIZE_T,
	                     result,
	                     result + match_length);
not_found:
	err_index_not_found((DeeObject *)self,
	                    (DeeObject *)other);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
string_caserindex(String *__restrict self, size_t argc,
                  DeeObject **__restrict argv, DeeObject *kw) {
	String *other;
	size_t start = 0, end = (size_t)-1;
	size_t mylen, result, match_length;
	union dcharptr ptr, lhs, rhs;
	if (DeeArg_UnpackKw(argc, argv, kw, find_kwlist, "o|IdId:caserindex", &other, &start, &end) ||
	    DeeObject_AssertTypeExact((DeeObject *)other, &DeeString_Type))
		goto err;
	SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON(DeeString_WIDTH(self),
	                                        DeeString_WIDTH(other))) {

	CASE_WIDTH_1BYTE:
		lhs.cp8 = DeeString_As1Byte((DeeObject *)self);
		rhs.cp8 = DeeString_As1Byte((DeeObject *)other);
		mylen   = WSTR_LENGTH(lhs.ptr);
		if unlikely(start > mylen)
			start = mylen;
		if likely(end > mylen)
			end = mylen;
		if unlikely(end <= start)
			goto not_found;
		ptr.cp8 = memcasermemb(lhs.cp8 + start, end - start,
		                       rhs.cp8, WSTR_LENGTH(rhs.cp8),
		                       &match_length);
		if unlikely(!ptr.cp8)
			goto not_found;
		result = ptr.cp8 - lhs.cp8;
		break;

	CASE_WIDTH_2BYTE:
		lhs.cp16 = DeeString_As2Byte((DeeObject *)self);
		if unlikely(!lhs.cp16)
			goto err;
		rhs.cp16 = DeeString_As2Byte((DeeObject *)other);
		if unlikely(!rhs.cp16)
			goto err;
		mylen = WSTR_LENGTH(lhs.ptr);
		if unlikely(start > mylen)
			start = mylen;
		if likely(end > mylen)
			end = mylen;
		if unlikely(end <= start)
			goto not_found;
		ptr.cp16 = memcasermemw(lhs.cp16 + start, end - start,
		                        rhs.cp16, WSTR_LENGTH(rhs.cp16),
		                        &match_length);
		if unlikely(!ptr.cp16)
			goto not_found;
		result = ptr.cp16 - lhs.cp16;
		break;

	CASE_WIDTH_4BYTE:
		lhs.cp32 = DeeString_As4Byte((DeeObject *)self);
		if unlikely(!lhs.cp32)
			goto err;
		rhs.cp32 = DeeString_As4Byte((DeeObject *)other);
		if unlikely(!rhs.cp32)
			goto err;
		mylen = WSTR_LENGTH(lhs.ptr);
		if unlikely(start > mylen)
			start = mylen;
		if likely(end > mylen)
			end = mylen;
		if unlikely(end <= start)
			goto not_found;
		ptr.cp32 = memcasermeml(lhs.cp32 + start, end - start,
		                        rhs.cp32, WSTR_LENGTH(rhs.cp32),
		                        &match_length);
		if unlikely(!ptr.cp32)
			goto not_found;
		result = ptr.cp32 - lhs.cp32;
		break;
	}
	return DeeTuple_Newf(DEE_FMT_SIZE_T
	                     DEE_FMT_SIZE_T,
	                     result,
	                     result + match_length);
not_found:
	err_index_not_found((DeeObject *)self,
	                    (DeeObject *)other);
err:
	return NULL;
}

PRIVATE DREF String *DCALL
string_getsubstr_ib(String *__restrict self,
                    size_t start, size_t end) {
	DREF String *result;
	union dcharptr str;
	size_t len;
	str.ptr = DeeString_WSTR(self);
	len     = WSTR_LENGTH(str.ptr);
	ASSERT(start <= end);
	ASSERT(start <= len);
	ASSERT(end <= len);
	if (start == 0 && end >= len) {
		result = self;
		Dee_Incref(result);
	} else {
		int width = DeeString_WIDTH(self);
		result = (DREF String *)DeeString_NewWithWidth(str.cp8 +
		                                               (start * STRING_SIZEOF_WIDTH(width)),
		                                               end - (size_t)start,
		                                               width);
	}
	return result;
}

PRIVATE DREF String *DCALL
string_getsubstr(String *__restrict self,
                 size_t start, size_t end) {
	DREF String *result;
	union dcharptr str;
	size_t len;
	str.ptr = DeeString_WSTR(self);
	len     = WSTR_LENGTH(str.ptr);
	if (start == 0 && end >= len) {
		result = self;
		Dee_Incref(result);
	} else {
		if (end >= len)
			end = len;
		if (start >= end) {
			result = (DREF String *)Dee_EmptyString;
			Dee_Incref(Dee_EmptyString);
		} else {
			int width = DeeString_WIDTH(self);
			result = (DREF String *)DeeString_NewWithWidth(str.cp8 +
			                                               (start * STRING_SIZEOF_WIDTH(width)),
			                                               end - (size_t)start,
			                                               width);
		}
	}
	return result;
}

PRIVATE DREF DeeObject *DCALL
string_substr(String *__restrict self, size_t argc,
              DeeObject **__restrict argv, DeeObject *kw) {
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, substr_kwlist, "|IdId:substr", &start, &end))
		return NULL;
	return (DREF DeeObject *)string_getsubstr(self, start, end);
}

PRIVATE DREF DeeObject *DCALL
string_strip(String *__restrict self,
             size_t argc, DeeObject **__restrict argv) {
	DeeObject *mask = NULL;
	if (DeeArg_Unpack(argc, argv, "|o:strip", &mask))
		return NULL;
	return mask ? (DeeObject_AssertTypeExact(mask, &DeeString_Type) ? NULL : DeeString_StripMask((DeeObject *)self, mask))
	            : DeeString_StripSpc((DeeObject *)self);
}

PRIVATE DREF DeeObject *DCALL
string_lstrip(String *__restrict self,
              size_t argc, DeeObject **__restrict argv) {
	DeeObject *mask = NULL;
	if (DeeArg_Unpack(argc, argv, "|o:lstrip", &mask))
		return NULL;
	return mask ? (DeeObject_AssertTypeExact(mask, &DeeString_Type) ? NULL : DeeString_LStripMask((DeeObject *)self, mask))
	            : DeeString_LStripSpc((DeeObject *)self);
}

PRIVATE DREF DeeObject *DCALL
string_rstrip(String *__restrict self,
              size_t argc, DeeObject **__restrict argv) {
	DeeObject *mask = NULL;
	if (DeeArg_Unpack(argc, argv, "|o:rstrip", &mask))
		return NULL;
	return mask ? (DeeObject_AssertTypeExact(mask, &DeeString_Type) ? NULL : DeeString_RStripMask((DeeObject *)self, mask))
	            : DeeString_RStripSpc((DeeObject *)self);
}

PRIVATE DREF DeeObject *DCALL
string_casestrip(String *__restrict self,
                 size_t argc, DeeObject **__restrict argv) {
	DeeObject *mask = NULL;
	if (DeeArg_Unpack(argc, argv, "|o:casestrip", &mask))
		return NULL;
	return mask ? (DeeObject_AssertTypeExact(mask, &DeeString_Type) ? NULL : DeeString_CaseStripMask((DeeObject *)self, mask))
	            : DeeString_StripSpc((DeeObject *)self);
}

PRIVATE DREF DeeObject *DCALL
string_caselstrip(String *__restrict self,
                  size_t argc, DeeObject **__restrict argv) {
	DeeObject *mask = NULL;
	if (DeeArg_Unpack(argc, argv, "|o:caselstrip", &mask))
		return NULL;
	return mask ? (DeeObject_AssertTypeExact(mask, &DeeString_Type) ? NULL : DeeString_CaseLStripMask((DeeObject *)self, mask))
	            : DeeString_LStripSpc((DeeObject *)self);
}

PRIVATE DREF DeeObject *DCALL
string_caserstrip(String *__restrict self,
                  size_t argc, DeeObject **__restrict argv) {
	DeeObject *mask = NULL;
	if (DeeArg_Unpack(argc, argv, "|o:caserstrip", &mask))
		return NULL;
	return mask ? (DeeObject_AssertTypeExact(mask, &DeeString_Type) ? NULL : DeeString_CaseRStripMask((DeeObject *)self, mask))
	            : DeeString_RStripSpc((DeeObject *)self);
}

#undef CONFIG_STRING_STARTSWITH_ENDSWITH_SPECIALCASE_OPTIMIZATIONS
#define CONFIG_STRING_STARTSWITH_ENDSWITH_SPECIALCASE_OPTIMIZATIONS 1

PRIVATE DREF DeeObject *DCALL
string_startswith(String *__restrict self, size_t argc,
                  DeeObject **__restrict argv, DeeObject *kw) {
	String *other;
	size_t begin = 0, end = (size_t)-1;
	union dcharptr my_str, ot_str;
	size_t my_len, ot_len;
	if (DeeArg_UnpackKw(argc, argv, kw, find_kwlist, "o|IdId:startswith", &other, &begin, &end) ||
	    DeeObject_AssertTypeExact((DeeObject *)other, &DeeString_Type))
		goto err;
#ifdef CONFIG_STRING_STARTSWITH_ENDSWITH_SPECIALCASE_OPTIMIZATIONS
	if (begin == 0 && end >= DeeString_WLEN(self) &&
	    /* NOTE: This checks that `DeeString_STR()' being either LATIN-1, or
	     *       UTF-8 is the same for both our own, and the `other' string. */
	    (DeeString_STR_ISUTF8(self) == DeeString_STR_ISUTF8(other))) {
		/* Special case: Since we don't have to count characters, we can simply
		 *               match the UTF-8 representations against each other. */
		if (DeeString_SIZE(other) > DeeString_SIZE(self) ||
		    !MEMEQB(DeeString_STR(self), DeeString_STR(other),
		            DeeString_SIZE(other)))
			goto nope;
		return_true;
	}
#endif /* CONFIG_STRING_STARTSWITH_ENDSWITH_SPECIALCASE_OPTIMIZATIONS */
	/* Must decode the other string in order to match its contents
	 * against data from our string at a specific offset. */
	SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON(DeeString_WIDTH(self),
	                                        DeeString_WIDTH(other))) {

	CASE_WIDTH_1BYTE:
		my_str.cp8 = DeeString_As1Byte((DeeObject *)self);
		ot_str.cp8 = DeeString_As1Byte((DeeObject *)other);
		my_len     = WSTR_LENGTH(my_str.cp8);
		ot_len     = WSTR_LENGTH(ot_str.cp8);
		if (my_len > end)
			my_len = end;
		if (my_len <= begin)
			break;
		my_len -= begin;
		if (ot_len > my_len)
			goto nope;
		return_bool(MEMEQB(my_str.cp8 + begin, ot_str.cp8, ot_len));

	CASE_WIDTH_2BYTE:
		my_str.cp16 = DeeString_As2Byte((DeeObject *)self);
		if unlikely(!my_str.cp16)
			goto err;
		ot_str.cp16 = DeeString_As2Byte((DeeObject *)other);
		my_len      = WSTR_LENGTH(my_str.cp16);
		ot_len      = WSTR_LENGTH(ot_str.cp16);
		if (my_len > end)
			my_len = end;
		if (my_len <= begin)
			break;
		my_len -= begin;
		if (ot_len > my_len)
			goto nope;
		return_bool(MEMEQW(my_str.cp16 + begin, ot_str.cp16, ot_len));

	CASE_WIDTH_4BYTE:
		my_str.cp32 = DeeString_As4Byte((DeeObject *)self);
		if unlikely(!my_str.cp32)
			goto err;
		ot_str.cp32 = DeeString_As4Byte((DeeObject *)other);
		my_len      = WSTR_LENGTH(my_str.cp32);
		ot_len      = WSTR_LENGTH(ot_str.cp32);
		if (my_len > end)
			my_len = end;
		if (my_len <= begin)
			break;
		my_len -= begin;
		if (ot_len > my_len)
			goto nope;
		return_bool(MEMEQL(my_str.cp32 + begin, ot_str.cp32, ot_len));
	}
	return_bool_(ot_len == 0);
nope:
	return_false;
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
string_endswith(String *__restrict self, size_t argc,
                DeeObject **__restrict argv, DeeObject *kw) {
	String *other;
	size_t begin = 0, end = (size_t)-1;
	union dcharptr my_str, ot_str;
	size_t my_len, ot_len;
	if (DeeArg_UnpackKw(argc, argv, kw, find_kwlist, "o|IdId:endswith", &other, &begin, &end) ||
	    DeeObject_AssertTypeExact((DeeObject *)other, &DeeString_Type))
		goto err;
#ifdef CONFIG_STRING_STARTSWITH_ENDSWITH_SPECIALCASE_OPTIMIZATIONS
	if (begin == 0 && end >= DeeString_WLEN(self) &&
	    /* NOTE: This checks that `DeeString_STR()' being either LATIN-1, or
	     *       UTF-8 is the same for both our own, and the `other' string. */
	    (DeeString_STR_ISUTF8(self) == DeeString_STR_ISUTF8(other))) {
		/* Special case: Since we don't have to count characters, we can simply
		 *               match the UTF-8 representations against each other. */
		if (DeeString_SIZE(other) > DeeString_SIZE(self) ||
		    !MEMEQB(DeeString_STR(self) +
		            (DeeString_SIZE(self) - DeeString_SIZE(other)),
		            DeeString_STR(other), DeeString_SIZE(other)))
			goto nope;
		return_true;
	}
#endif /* CONFIG_STRING_STARTSWITH_ENDSWITH_SPECIALCASE_OPTIMIZATIONS */
	/* Must decode the other string in order to match its contents
	 * against data from our string at a specific offset. */
	SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON(DeeString_WIDTH(self),
	                                        DeeString_WIDTH(other))) {

	CASE_WIDTH_1BYTE:
		my_str.cp8 = DeeString_As1Byte((DeeObject *)self);
		ot_str.cp8 = DeeString_As1Byte((DeeObject *)other);
		my_len     = WSTR_LENGTH(my_str.cp8);
		ot_len     = WSTR_LENGTH(ot_str.cp8);
		if (my_len > end)
			my_len = end;
		if (my_len <= begin)
			break;
		my_len -= begin;
		if (ot_len > my_len)
			goto nope;
		begin += my_len;
		begin -= ot_len;
		return_bool(MEMEQB(my_str.cp8 + begin, ot_str.cp8, ot_len));

	CASE_WIDTH_2BYTE:
		my_str.cp16 = DeeString_As2Byte((DeeObject *)self);
		if unlikely(!my_str.cp16)
			goto err;
		ot_str.cp16 = DeeString_As2Byte((DeeObject *)other);
		my_len      = WSTR_LENGTH(my_str.cp16);
		ot_len      = WSTR_LENGTH(ot_str.cp16);
		if (my_len > end)
			my_len = end;
		if (my_len <= begin)
			break;
		my_len -= begin;
		if (ot_len > my_len)
			goto nope;
		begin += my_len;
		begin -= ot_len;
		return_bool(MEMEQW(my_str.cp16 + begin, ot_str.cp16, ot_len));

	CASE_WIDTH_4BYTE:
		my_str.cp32 = DeeString_As4Byte((DeeObject *)self);
		if unlikely(!my_str.cp32)
			goto err;
		ot_str.cp32 = DeeString_As4Byte((DeeObject *)other);
		my_len      = WSTR_LENGTH(my_str.cp32);
		ot_len      = WSTR_LENGTH(ot_str.cp32);
		if (my_len > end)
			my_len = end;
		if (my_len <= begin)
			break;
		my_len -= begin;
		if (ot_len > my_len)
			goto nope;
		begin += my_len;
		begin -= ot_len;
		return_bool(MEMEQL(my_str.cp32 + begin, ot_str.cp32, ot_len));
	}
	return_bool_(ot_len == 0);
nope:
	return_false;
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
string_casestartswith(String *__restrict self, size_t argc,
                      DeeObject **__restrict argv, DeeObject *kw) {
	String *other;
	size_t begin = 0, end = (size_t)-1;
	union dcharptr my_str, ot_str;
	size_t my_len, ot_len;
	if (DeeArg_UnpackKw(argc, argv, kw, find_kwlist, "o|IdId:casestartswith", &other, &begin, &end) ||
	    DeeObject_AssertTypeExact((DeeObject *)other, &DeeString_Type))
		goto err;
	/* Must decode the other string in order to match its contents
	 * against data from our string at a specific offset. */
	SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON(DeeString_WIDTH(self),
	                                        DeeString_WIDTH(other))) {

	CASE_WIDTH_1BYTE:
		my_str.cp8 = DeeString_As1Byte((DeeObject *)self);
		ot_str.cp8 = DeeString_As1Byte((DeeObject *)other);
		my_len     = WSTR_LENGTH(my_str.cp8);
		ot_len     = WSTR_LENGTH(ot_str.cp8);
		if (my_len > end)
			my_len = end;
		if (my_len <= begin)
			break;
		my_len -= begin;
		return_bool(MEMCASESTARTSWITHB(my_str.cp8 + begin, my_len, ot_str.cp8, ot_len));

	CASE_WIDTH_2BYTE:
		my_str.cp16 = DeeString_As2Byte((DeeObject *)self);
		if unlikely(!my_str.cp16)
			goto err;
		ot_str.cp16 = DeeString_As2Byte((DeeObject *)other);
		my_len      = WSTR_LENGTH(my_str.cp16);
		ot_len      = WSTR_LENGTH(ot_str.cp16);
		if (my_len > end)
			my_len = end;
		if (my_len <= begin)
			break;
		my_len -= begin;
		return_bool(MEMCASESTARTSWITHW(my_str.cp16 + begin, my_len, ot_str.cp16, ot_len));

	CASE_WIDTH_4BYTE:
		my_str.cp32 = DeeString_As4Byte((DeeObject *)self);
		if unlikely(!my_str.cp32)
			goto err;
		ot_str.cp32 = DeeString_As4Byte((DeeObject *)other);
		my_len      = WSTR_LENGTH(my_str.cp32);
		ot_len      = WSTR_LENGTH(ot_str.cp32);
		if (my_len > end)
			my_len = end;
		if (my_len <= begin)
			break;
		my_len -= begin;
		return_bool(MEMCASESTARTSWITHL(my_str.cp32 + begin, my_len, ot_str.cp32, ot_len));
	}
	return_bool_(ot_len == 0);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
string_caseendswith(String *__restrict self, size_t argc,
                    DeeObject **__restrict argv, DeeObject *kw) {
	String *other;
	size_t begin = 0, end = (size_t)-1;
	union dcharptr my_str, ot_str;
	size_t my_len, ot_len;
	if (DeeArg_UnpackKw(argc, argv, kw, find_kwlist, "o|IdId:caseendswith", &other, &begin, &end) ||
	    DeeObject_AssertTypeExact((DeeObject *)other, &DeeString_Type))
		goto err;
	/* Must decode the other string in order to match its contents
	 * against data from our string at a specific offset. */
	SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON(DeeString_WIDTH(self),
	                                        DeeString_WIDTH(other))) {

	CASE_WIDTH_1BYTE:
		my_str.cp8 = DeeString_As1Byte((DeeObject *)self);
		ot_str.cp8 = DeeString_As1Byte((DeeObject *)other);
		my_len     = WSTR_LENGTH(my_str.cp8);
		ot_len     = WSTR_LENGTH(ot_str.cp8);
		if (my_len > end)
			my_len = end;
		if (my_len <= begin)
			break;
		my_len -= begin;
		return_bool(MEMCASEENDSWITHB(my_str.cp8 + begin, my_len, ot_str.cp8, ot_len));

	CASE_WIDTH_2BYTE:
		my_str.cp16 = DeeString_As2Byte((DeeObject *)self);
		if unlikely(!my_str.cp16)
			goto err;
		ot_str.cp16 = DeeString_As2Byte((DeeObject *)other);
		my_len      = WSTR_LENGTH(my_str.cp16);
		ot_len      = WSTR_LENGTH(ot_str.cp16);
		if (my_len > end)
			my_len = end;
		if (my_len <= begin)
			break;
		my_len -= begin;
		return_bool(MEMCASEENDSWITHW(my_str.cp16 + begin, my_len, ot_str.cp16, ot_len));

	CASE_WIDTH_4BYTE:
		my_str.cp32 = DeeString_As4Byte((DeeObject *)self);
		if unlikely(!my_str.cp32)
			goto err;
		ot_str.cp32 = DeeString_As4Byte((DeeObject *)other);
		my_len      = WSTR_LENGTH(my_str.cp32);
		ot_len      = WSTR_LENGTH(ot_str.cp32);
		if (my_len > end)
			my_len = end;
		if (my_len <= begin)
			break;
		my_len -= begin;
		return_bool(MEMCASEENDSWITHL(my_str.cp32 + begin, my_len, ot_str.cp32, ot_len));
	}
	return_bool_(ot_len == 0);
err:
	return NULL;
}

INTDEF unsigned int DCALL
DeeCodec_GetErrorMode(char const *__restrict errors);


PRIVATE struct keyword decode_encode_kwlist[] = { K(codec), K(errors), KEND };

INTERN DREF DeeObject *DCALL
string_decode(DeeObject *__restrict self, size_t argc,
              DeeObject **__restrict argv, DeeObject *kw) {
	DeeObject *codec;
	char *errors            = NULL;
	unsigned int error_mode = STRING_ERROR_FSTRICT;
	if (DeeArg_UnpackKw(argc, argv, kw, decode_encode_kwlist, "o|s:decode", &codec, &errors) ||
	    DeeObject_AssertTypeExact(codec, &DeeString_Type))
		goto err;
	if (errors) {
		error_mode = DeeCodec_GetErrorMode(errors);
		if unlikely(error_mode == (unsigned int)-1)
			goto err;
	}
	return DeeCodec_Decode(self, codec, error_mode);
err:
	return NULL;
}

INTERN DREF DeeObject *DCALL
string_encode(DeeObject *__restrict self, size_t argc,
              DeeObject **__restrict argv, DeeObject *kw) {
	DeeObject *codec;
	char *errors            = NULL;
	unsigned int error_mode = STRING_ERROR_FSTRICT;
	if (DeeArg_UnpackKw(argc, argv, kw, decode_encode_kwlist, "o|s:encode", &codec, &errors) ||
	    DeeObject_AssertTypeExact(codec, &DeeString_Type))
		goto err;
	if (errors) {
		error_mode = DeeCodec_GetErrorMode(errors);
		if unlikely(error_mode == (unsigned int)-1)
			goto err;
	}
	return DeeCodec_Encode(self, codec, error_mode);
err:
	return NULL;
}


INTDEF DREF DeeObject *DCALL
DeeString_Segments(String *__restrict self, size_t substring_length);

PRIVATE DREF DeeObject *DCALL
string_segments(String *__restrict self,
                size_t argc, DeeObject **__restrict argv) {
	size_t substring_length;
	if (DeeArg_Unpack(argc, argv, "Iu:segments", &substring_length))
		return NULL;
	if unlikely(!substring_length) {
		err_invalid_segment_size(substring_length);
		return NULL;
	}
	return DeeString_Segments(self, substring_length);
}

PRIVATE DREF DeeObject *DCALL
string_distribute(String *__restrict self,
                  size_t argc, DeeObject **__restrict argv) {
	size_t substring_count;
	size_t substring_length;
	if (DeeArg_Unpack(argc, argv, "Iu:distribute", &substring_count))
		return NULL;
	if unlikely(!substring_count) {
		err_invalid_distribution_count(substring_count);
		return NULL;
	}
	substring_length = DeeString_WLEN(self);
	substring_length += substring_count - 1;
	substring_length /= substring_count;
	if unlikely(!substring_length)
		return_empty_seq;
	return DeeString_Segments(self, substring_length);
}

PRIVATE ATTR_COLD int DCALL err_empty_filler(void) {
	return DeeError_Throwf(&DeeError_ValueError,
	                       "Empty filler");
}

PRIVATE DREF DeeObject *DCALL
string_center(String *__restrict self,
              size_t argc, DeeObject **__restrict argv) {
	size_t result_length, my_len, fl_len;
	size_t fill_front, fill_back;
	DeeObject *filler_ob = NULL;
	DREF DeeObject *result;
	union dcharptr dst, buf, my_str, fl_str;
	if (DeeArg_Unpack(argc, argv, "Iu|o:center", &result_length, &filler_ob))
		goto err;
	my_len = DeeString_WLEN(self);
	if (result_length <= my_len)
		return_reference_((DeeObject *)self);
	fill_front = (result_length - my_len);
	fill_back  = fill_front / 2;
	fill_front -= fill_back;
	if (filler_ob) {
		if (DeeObject_AssertTypeExact(filler_ob, &DeeString_Type))
			goto err;
		SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON(DeeString_WIDTH(self),
		                                        DeeString_WIDTH(filler_ob))) {

		CASE_WIDTH_1BYTE:
			my_str.cp8 = DeeString_As1Byte((DeeObject *)self);
			fl_str.cp8 = DeeString_As1Byte((DeeObject *)filler_ob);
			fl_len     = WSTR_LENGTH(fl_str.cp8);
			if unlikely(!fl_len)
				goto empty_filler;
			result = DeeString_NewBuffer(result_length);
			if unlikely(!result)
				goto err;
			dst.cp8 = DeeString_As1Byte(result);
			memfilb(dst.cp8, fill_front, fl_str.cp8, fl_len);
			dst.cp8 += fill_front;
			memcpyb(dst.cp8, my_str.cp8, my_len);
			dst.cp8 += my_len;
			memfilb(dst.cp8, fill_back, fl_str.cp8, fl_len);
			break;

		CASE_WIDTH_2BYTE:
			my_str.cp16 = DeeString_As2Byte((DeeObject *)self);
			if unlikely(!my_str.cp16)
				goto err;
			fl_str.cp16 = DeeString_As2Byte((DeeObject *)filler_ob);
			if unlikely(!fl_str.cp16)
				goto err;
			fl_len = WSTR_LENGTH(fl_str.cp8);
			if unlikely(!fl_len)
				goto empty_filler;
			dst.cp16 = buf.cp16 = DeeString_New2ByteBuffer(result_length);
			if unlikely(!buf.cp16)
				goto err;
			memfilw(dst.cp16, fill_front, fl_str.cp16, fl_len);
			dst.cp16 += fill_front;
			memcpyw(dst.cp16, my_str.cp16, my_len);
			dst.cp16 += my_len;
			memfilw(dst.cp16, fill_back, fl_str.cp16, fl_len);
			result = DeeString_Pack2ByteBuffer(buf.cp16);
			break;

		CASE_WIDTH_4BYTE:
			my_str.cp32 = DeeString_As4Byte((DeeObject *)self);
			if unlikely(!my_str.cp32)
				goto err;
			fl_str.cp32 = DeeString_As4Byte((DeeObject *)filler_ob);
			if unlikely(!fl_str.cp32)
				goto err;
			fl_len = WSTR_LENGTH(fl_str.cp8);
			if unlikely(!fl_len)
				goto empty_filler;
			dst.cp32 = buf.cp32 = DeeString_New4ByteBuffer(result_length);
			if unlikely(!buf.cp32)
				goto err;
			memfill(dst.cp32, fill_front, fl_str.cp32, fl_len);
			dst.cp32 += fill_front;
			memcpyl(dst.cp32, my_str.cp32, my_len);
			dst.cp32 += my_len;
			memfill(dst.cp32, fill_back, fl_str.cp32, fl_len);
			result = DeeString_Pack4ByteBuffer(buf.cp32);
			break;
		}
	} else {
		SWITCH_SIZEOF_WIDTH(DeeString_WIDTH(self)) {

		CASE_WIDTH_1BYTE:
			my_str.cp8 = DeeString_Get1Byte((DeeObject *)self);
			result     = DeeString_NewBuffer(result_length);
			if unlikely(!result)
				goto err;
			dst.cp8 = DeeString_As1Byte(result);
			memsetb(dst.cp8, UNICODE_SPACE, fill_front);
			dst.cp8 += fill_front;
			memcpyb(dst.cp8, my_str.cp8, my_len);
			dst.cp8 += my_len;
			memsetb(dst.cp8, UNICODE_SPACE, fill_back);
			break;

		CASE_WIDTH_2BYTE:
			my_str.cp16 = DeeString_Get2Byte((DeeObject *)self);
			dst.cp16 = buf.cp16 = DeeString_New2ByteBuffer(result_length);
			if unlikely(!buf.cp16)
				goto err;
			memsetw(dst.cp16, UNICODE_SPACE, fill_front);
			dst.cp16 += fill_front;
			memcpyw(dst.cp16, my_str.cp16, my_len);
			dst.cp16 += my_len;
			memsetw(dst.cp16, UNICODE_SPACE, fill_back);
			result = DeeString_Pack2ByteBuffer(buf.cp16);
			break;

		CASE_WIDTH_4BYTE:
			my_str.cp32 = DeeString_Get4Byte((DeeObject *)self);
			dst.cp32 = buf.cp32 = DeeString_New4ByteBuffer(result_length);
			if unlikely(!buf.cp32)
				goto err;
			memsetl(dst.cp32, UNICODE_SPACE, fill_front);
			dst.cp32 += fill_front;
			memcpyl(dst.cp32, my_str.cp32, my_len);
			dst.cp32 += my_len;
			memsetl(dst.cp32, UNICODE_SPACE, fill_back);
			result = DeeString_Pack4ByteBuffer(buf.cp32);
			break;
		}
	}
	return result;
empty_filler:
	err_empty_filler();
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
string_ljust(String *__restrict self,
             size_t argc, DeeObject **__restrict argv) {
	size_t result_length, my_len, fl_len, fill_back;
	DeeObject *filler_ob = NULL;
	DREF DeeObject *result;
	union dcharptr dst, buf, my_str, fl_str;
	if (DeeArg_Unpack(argc, argv, "Iu|o:ljust", &result_length, &filler_ob))
		goto err;
	my_len = DeeString_WLEN(self);
	if (result_length <= my_len)
		return_reference_((DeeObject *)self);
	fill_back = (result_length - my_len);
	if (filler_ob) {
		if (DeeObject_AssertTypeExact(filler_ob, &DeeString_Type))
			goto err;
		SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON(DeeString_WIDTH(self),
		                                        DeeString_WIDTH(filler_ob))) {

		CASE_WIDTH_1BYTE:
			my_str.cp8 = DeeString_As1Byte((DeeObject *)self);
			fl_str.cp8 = DeeString_As1Byte((DeeObject *)filler_ob);
			fl_len     = WSTR_LENGTH(fl_str.cp8);
			if unlikely(!fl_len)
				goto empty_filler;
			result = DeeString_NewBuffer(result_length);
			if unlikely(!result)
				goto err;
			dst.cp8 = DeeString_As1Byte(result);
			memcpyb(dst.cp8, my_str.cp8, my_len);
			dst.cp8 += my_len;
			memfilb(dst.cp8, fill_back, fl_str.cp8, fl_len);
			break;

		CASE_WIDTH_2BYTE:
			my_str.cp16 = DeeString_As2Byte((DeeObject *)self);
			if unlikely(!my_str.cp16)
				goto err;
			fl_str.cp16 = DeeString_As2Byte((DeeObject *)filler_ob);
			if unlikely(!fl_str.cp16)
				goto err;
			fl_len = WSTR_LENGTH(fl_str.cp8);
			if unlikely(!fl_len)
				goto empty_filler;
			dst.cp16 = buf.cp16 = DeeString_New2ByteBuffer(result_length);
			if unlikely(!buf.cp16)
				goto err;
			memcpyw(dst.cp16, my_str.cp16, my_len);
			dst.cp16 += my_len;
			memfilw(dst.cp16, fill_back, fl_str.cp16, fl_len);
			result = DeeString_Pack2ByteBuffer(buf.cp16);
			break;

		CASE_WIDTH_4BYTE:
			my_str.cp32 = DeeString_As4Byte((DeeObject *)self);
			if unlikely(!my_str.cp32)
				goto err;
			fl_str.cp32 = DeeString_As4Byte((DeeObject *)filler_ob);
			if unlikely(!fl_str.cp32)
				goto err;
			fl_len = WSTR_LENGTH(fl_str.cp8);
			if unlikely(!fl_len)
				goto empty_filler;
			dst.cp32 = buf.cp32 = DeeString_New4ByteBuffer(result_length);
			if unlikely(!buf.cp32)
				goto err;
			memcpyl(dst.cp32, my_str.cp32, my_len);
			dst.cp32 += my_len;
			memfill(dst.cp32, fill_back, fl_str.cp32, fl_len);
			result = DeeString_Pack4ByteBuffer(buf.cp32);
			break;
		}
	} else {
		SWITCH_SIZEOF_WIDTH(DeeString_WIDTH(self)) {

		CASE_WIDTH_1BYTE:
			my_str.cp8 = DeeString_Get1Byte((DeeObject *)self);
			result     = DeeString_NewBuffer(result_length);
			if unlikely(!result)
				goto err;
			dst.cp8 = DeeString_As1Byte(result);
			memcpyb(dst.cp8, my_str.cp8, my_len);
			dst.cp8 += my_len;
			memsetb(dst.cp8, UNICODE_SPACE, fill_back);
			break;

		CASE_WIDTH_2BYTE:
			my_str.cp16 = DeeString_Get2Byte((DeeObject *)self);
			dst.cp16 = buf.cp16 = DeeString_New2ByteBuffer(result_length);
			if unlikely(!buf.cp16)
				goto err;
			memcpyw(dst.cp16, my_str.cp16, my_len);
			dst.cp16 += my_len;
			memsetw(dst.cp16, UNICODE_SPACE, fill_back);
			result = DeeString_Pack2ByteBuffer(buf.cp16);
			break;

		CASE_WIDTH_4BYTE:
			my_str.cp32 = DeeString_Get4Byte((DeeObject *)self);
			dst.cp32 = buf.cp32 = DeeString_New4ByteBuffer(result_length);
			if unlikely(!buf.cp32)
				goto err;
			memcpyl(dst.cp32, my_str.cp32, my_len);
			dst.cp32 += my_len;
			memsetl(dst.cp32, UNICODE_SPACE, fill_back);
			result = DeeString_Pack4ByteBuffer(buf.cp32);
			break;
		}
	}
	return result;
empty_filler:
	err_empty_filler();
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
string_rjust(String *__restrict self,
             size_t argc, DeeObject **__restrict argv) {
	size_t result_length, my_len, fl_len, fill_front;
	DeeObject *filler_ob = NULL;
	DREF DeeObject *result;
	union dcharptr dst, buf, my_str, fl_str;
	if (DeeArg_Unpack(argc, argv, "Iu|o:rjust", &result_length, &filler_ob))
		goto err;
	my_len = DeeString_WLEN(self);
	if (result_length <= my_len)
		return_reference_((DeeObject *)self);
	fill_front = (result_length - my_len);
	if (filler_ob) {
		if (DeeObject_AssertTypeExact(filler_ob, &DeeString_Type))
			goto err;
		SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON(DeeString_WIDTH(self),
		                                        DeeString_WIDTH(filler_ob))) {

		CASE_WIDTH_1BYTE:
			my_str.cp8 = DeeString_As1Byte((DeeObject *)self);
			fl_str.cp8 = DeeString_As1Byte((DeeObject *)filler_ob);
			fl_len     = WSTR_LENGTH(fl_str.cp8);
			if unlikely(!fl_len)
				goto empty_filler;
			result = DeeString_NewBuffer(result_length);
			if unlikely(!result)
				goto err;
			dst.cp8 = DeeString_As1Byte(result);
			memfilb(dst.cp8, fill_front, fl_str.cp8, fl_len);
			dst.cp8 += fill_front;
			memcpyb(dst.cp8, my_str.cp8, my_len);
			break;

		CASE_WIDTH_2BYTE:
			my_str.cp16 = DeeString_As2Byte((DeeObject *)self);
			if unlikely(!my_str.cp16)
				goto err;
			fl_str.cp16 = DeeString_As2Byte((DeeObject *)filler_ob);
			if unlikely(!fl_str.cp16)
				goto err;
			fl_len = WSTR_LENGTH(fl_str.cp8);
			if unlikely(!fl_len)
				goto empty_filler;
			dst.cp16 = buf.cp16 = DeeString_New2ByteBuffer(result_length);
			if unlikely(!buf.cp16)
				goto err;
			memfilw(dst.cp16, fill_front, fl_str.cp16, fl_len);
			dst.cp16 += fill_front;
			memcpyw(dst.cp16, my_str.cp16, my_len);
			result = DeeString_Pack2ByteBuffer(buf.cp16);
			break;

		CASE_WIDTH_4BYTE:
			my_str.cp32 = DeeString_As4Byte((DeeObject *)self);
			if unlikely(!my_str.cp32)
				goto err;
			fl_str.cp32 = DeeString_As4Byte((DeeObject *)filler_ob);
			if unlikely(!fl_str.cp32)
				goto err;
			fl_len = WSTR_LENGTH(fl_str.cp8);
			if unlikely(!fl_len)
				goto empty_filler;
			dst.cp32 = buf.cp32 = DeeString_New4ByteBuffer(result_length);
			if unlikely(!buf.cp32)
				goto err;
			memfill(dst.cp32, fill_front, fl_str.cp32, fl_len);
			dst.cp32 += fill_front;
			memcpyl(dst.cp32, my_str.cp32, my_len);
			result = DeeString_Pack4ByteBuffer(buf.cp32);
			break;
		}
	} else {
		SWITCH_SIZEOF_WIDTH(DeeString_WIDTH(self)) {

		CASE_WIDTH_1BYTE:
			my_str.cp8 = DeeString_Get1Byte((DeeObject *)self);
			result     = DeeString_NewBuffer(result_length);
			if unlikely(!result)
				goto err;
			dst.cp8 = DeeString_As1Byte(result);
			memsetb(dst.cp8, UNICODE_SPACE, fill_front);
			dst.cp8 += fill_front;
			memcpyb(dst.cp8, my_str.cp8, my_len);
			break;

		CASE_WIDTH_2BYTE:
			my_str.cp16 = DeeString_Get2Byte((DeeObject *)self);
			dst.cp16 = buf.cp16 = DeeString_New2ByteBuffer(result_length);
			if unlikely(!buf.cp16)
				goto err;
			memsetw(dst.cp16, UNICODE_SPACE, fill_front);
			dst.cp16 += fill_front;
			memcpyw(dst.cp16, my_str.cp16, my_len);
			result = DeeString_Pack2ByteBuffer(buf.cp16);
			break;

		CASE_WIDTH_4BYTE:
			my_str.cp32 = DeeString_Get4Byte((DeeObject *)self);
			dst.cp32 = buf.cp32 = DeeString_New4ByteBuffer(result_length);
			if unlikely(!buf.cp32)
				goto err;
			memsetl(dst.cp32, UNICODE_SPACE, fill_front);
			dst.cp32 += fill_front;
			memcpyl(dst.cp32, my_str.cp32, my_len);
			result = DeeString_Pack4ByteBuffer(buf.cp32);
			break;
		}
	}
	return result;
empty_filler:
	err_empty_filler();
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
string_count(String *__restrict self, size_t argc,
             DeeObject **__restrict argv, DeeObject *kw) {
	String *other;
	size_t result = 0;
	size_t begin = 0, end = (size_t)-1;
	union dcharptr lhs, rhs, endptr;
	size_t mylen;
	if (DeeArg_UnpackKw(argc, argv, kw, find_kwlist, "o|IdId:count", &other, &begin, &end) ||
	    DeeObject_AssertTypeExact((DeeObject *)other, &DeeString_Type))
		goto err;
	SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON(DeeString_WIDTH(self),
	                                        DeeString_WIDTH(other))) {

	CASE_WIDTH_1BYTE:
		lhs.cp8 = DeeString_As1Byte((DeeObject *)self);
		rhs.cp8 = DeeString_As1Byte((DeeObject *)other);
		mylen   = DeeString_SIZE(self);
		if unlikely(begin > mylen)
			begin = mylen;
		if likely(end > mylen)
			end = mylen;
		if unlikely(end <= begin)
			break;
		endptr.cp8 = lhs.cp8 + end;
		lhs.cp8 += begin;
		while ((lhs.cp8 = memmemb(lhs.cp8, endptr.cp8 - lhs.cp8,
		                          rhs.cp8, WSTR_LENGTH(rhs.cp8))) != NULL)
			++result, lhs.cp8 += WSTR_LENGTH(rhs.cp8);
		break;

	CASE_WIDTH_2BYTE:
		lhs.cp16 = DeeString_As2Byte((DeeObject *)self);
		if unlikely(!lhs.cp16)
			goto err;
		rhs.cp16 = DeeString_As2Byte((DeeObject *)other);
		if unlikely(!rhs.cp16)
			goto err;
		mylen = WSTR_LENGTH(lhs.cp16);
		if unlikely(begin > mylen)
			begin = mylen;
		if likely(end > mylen)
			end = mylen;
		if unlikely(end <= begin)
			break;
		endptr.cp16 = lhs.cp16 + end;
		lhs.cp16 += begin;
		while ((lhs.cp16 = memmemw(lhs.cp16, endptr.cp16 - lhs.cp16,
		                           rhs.cp16, WSTR_LENGTH(rhs.cp16))) != NULL)
			++result, lhs.cp16 += WSTR_LENGTH(rhs.cp16);
		break;

	CASE_WIDTH_4BYTE:
		lhs.cp32 = DeeString_As4Byte((DeeObject *)self);
		if unlikely(!lhs.cp32)
			goto err;
		rhs.cp32 = DeeString_As4Byte((DeeObject *)other);
		if unlikely(!rhs.cp32)
			goto err;
		mylen = WSTR_LENGTH(lhs.cp32);
		if unlikely(begin > mylen)
			begin = mylen;
		if likely(end > mylen)
			end = mylen;
		if unlikely(end <= begin)
			break;
		endptr.cp32 = lhs.cp32 + end;
		lhs.cp32 += begin;
		while ((lhs.cp32 = memmeml(lhs.cp32, endptr.cp32 - lhs.cp32,
		                           rhs.cp32, WSTR_LENGTH(rhs.cp32))) != NULL)
			++result, lhs.cp32 += WSTR_LENGTH(rhs.cp32);
		break;
	}
	return DeeInt_NewSize(result);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
string_casecount(String *__restrict self, size_t argc,
                 DeeObject **__restrict argv, DeeObject *kw) {
	String *other;
	size_t result = 0;
	size_t begin = 0, end = (size_t)-1, match_length;
	union dcharptr lhs, rhs, endptr;
	size_t mylen;
	if (DeeArg_UnpackKw(argc, argv, kw, find_kwlist, "o|IdId:casecount", &other, &begin, &end) ||
	    DeeObject_AssertTypeExact((DeeObject *)other, &DeeString_Type))
		goto err;
	SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON(DeeString_WIDTH(self),
	                                        DeeString_WIDTH(other))) {

	CASE_WIDTH_1BYTE:
		lhs.cp8 = DeeString_As1Byte((DeeObject *)self);
		rhs.cp8 = DeeString_As1Byte((DeeObject *)other);
		mylen   = DeeString_SIZE(self);
		if unlikely(begin > mylen)
			begin = mylen;
		if likely(end > mylen)
			end = mylen;
		if unlikely(end <= begin)
			break;
		endptr.cp8 = lhs.cp8 + end;
		lhs.cp8 += begin;
		while ((lhs.cp8 = memcasememb(lhs.cp8, endptr.cp8 - lhs.cp8,
		                              rhs.cp8, WSTR_LENGTH(rhs.cp8),
		                              &match_length)) != NULL)
			++result, lhs.cp8 += match_length;
		break;

	CASE_WIDTH_2BYTE:
		lhs.cp16 = DeeString_As2Byte((DeeObject *)self);
		if unlikely(!lhs.cp16)
			goto err;
		rhs.cp16 = DeeString_As2Byte((DeeObject *)other);
		if unlikely(!rhs.cp16)
			goto err;
		mylen = WSTR_LENGTH(lhs.cp16);
		if unlikely(begin > mylen)
			begin = mylen;
		if likely(end > mylen)
			end = mylen;
		if unlikely(end <= begin)
			break;
		endptr.cp16 = lhs.cp16 + end;
		lhs.cp16 += begin;
		while ((lhs.cp16 = memcasememw(lhs.cp16, endptr.cp16 - lhs.cp16,
		                               rhs.cp16, WSTR_LENGTH(rhs.cp16),
		                               &match_length)) != NULL)
			++result, lhs.cp16 += match_length;
		break;

	CASE_WIDTH_4BYTE:
		lhs.cp32 = DeeString_As4Byte((DeeObject *)self);
		if unlikely(!lhs.cp32)
			goto err;
		rhs.cp32 = DeeString_As4Byte((DeeObject *)other);
		if unlikely(!rhs.cp32)
			goto err;
		mylen = WSTR_LENGTH(lhs.cp32);
		if unlikely(begin > mylen)
			begin = mylen;
		if likely(end > mylen)
			end = mylen;
		if unlikely(end <= begin)
			break;
		endptr.cp32 = lhs.cp32 + end;
		lhs.cp32 += begin;
		while ((lhs.cp32 = memcasememl(lhs.cp32, endptr.cp32 - lhs.cp32,
		                               rhs.cp32, WSTR_LENGTH(rhs.cp32),
		                               &match_length)) != NULL)
			++result, lhs.cp32 += match_length;
		break;
	}
	return DeeInt_NewSize(result);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
string_contains_f(String *__restrict self, size_t argc,
                  DeeObject **__restrict argv, DeeObject *kw) {
	String *other;
	size_t begin = 0, end = (size_t)-1;
	union dcharptr lhs, rhs;
	size_t mylen;
	if (DeeArg_UnpackKw(argc, argv, kw, find_kwlist, "o|IdId:contains", &other, &begin, &end) ||
	    DeeObject_AssertTypeExact((DeeObject *)other, &DeeString_Type))
		goto err;
	SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON(DeeString_WIDTH(self),
	                                        DeeString_WIDTH(other))) {

	CASE_WIDTH_1BYTE:
		lhs.cp8 = DeeString_As1Byte((DeeObject *)self);
		rhs.cp8 = DeeString_As1Byte((DeeObject *)other);
		mylen   = DeeString_SIZE(self);
		if unlikely(begin > mylen)
			begin = mylen;
		if likely(end > mylen)
			end = mylen;
		if unlikely(end <= begin)
			goto nope;
		if (!memmemb(lhs.cp8 + begin, end - begin,
		             rhs.cp8, WSTR_LENGTH(rhs.cp8)))
			goto nope;
		break;

	CASE_WIDTH_2BYTE:
		lhs.cp16 = DeeString_As2Byte((DeeObject *)self);
		if unlikely(!lhs.cp16)
			goto err;
		rhs.cp16 = DeeString_As2Byte((DeeObject *)other);
		if unlikely(!rhs.cp16)
			goto err;
		mylen = WSTR_LENGTH(lhs.cp16);
		if unlikely(begin > mylen)
			begin = mylen;
		if likely(end > mylen)
			end = mylen;
		if unlikely(end <= begin)
			goto nope;
		if (!memmemw(lhs.cp16 + begin, end - begin,
		             rhs.cp16, WSTR_LENGTH(rhs.cp16)))
			goto nope;
		break;

	CASE_WIDTH_4BYTE:
		lhs.cp32 = DeeString_As4Byte((DeeObject *)self);
		if unlikely(!lhs.cp32)
			goto err;
		rhs.cp32 = DeeString_As4Byte((DeeObject *)other);
		if unlikely(!rhs.cp32)
			goto err;
		mylen = WSTR_LENGTH(lhs.cp32);
		if unlikely(begin > mylen)
			begin = mylen;
		if likely(end > mylen)
			end = mylen;
		if unlikely(end <= begin)
			goto nope;
		if (!memmeml(lhs.cp32 + begin, end - begin,
		             rhs.cp32, WSTR_LENGTH(rhs.cp32)))
			goto nope;
		break;
	}
	return_true;
nope:
	return_false;
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
string_casecontains_f(String *__restrict self, size_t argc,
                      DeeObject **__restrict argv, DeeObject *kw) {
	String *other;
	size_t begin = 0, end = (size_t)-1;
	union dcharptr lhs, rhs;
	size_t mylen;
	if (DeeArg_UnpackKw(argc, argv, kw, find_kwlist, "o|IdId:casecontains", &other, &begin, &end) ||
	    DeeObject_AssertTypeExact((DeeObject *)other, &DeeString_Type))
		goto err;
	SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON(DeeString_WIDTH(self),
	                                        DeeString_WIDTH(other))) {

	CASE_WIDTH_1BYTE:
		lhs.cp8 = DeeString_As1Byte((DeeObject *)self);
		rhs.cp8 = DeeString_As1Byte((DeeObject *)other);
		mylen   = DeeString_SIZE(self);
		if unlikely(begin > mylen)
			begin = mylen;
		if likely(end > mylen)
			end = mylen;
		if unlikely(end <= begin)
			goto nope;
		if (!memcasememb(lhs.cp8 + begin, end - begin,
		                 rhs.cp8, WSTR_LENGTH(rhs.cp8), NULL))
			goto nope;
		break;

	CASE_WIDTH_2BYTE:
		lhs.cp16 = DeeString_As2Byte((DeeObject *)self);
		if unlikely(!lhs.cp16)
			goto err;
		rhs.cp16 = DeeString_As2Byte((DeeObject *)other);
		if unlikely(!rhs.cp16)
			goto err;
		mylen = WSTR_LENGTH(lhs.cp16);
		if unlikely(begin > mylen)
			begin = mylen;
		if likely(end > mylen)
			end = mylen;
		if unlikely(end <= begin)
			goto nope;
		if (!memcasememw(lhs.cp16 + begin, end - begin,
		                 rhs.cp16, WSTR_LENGTH(rhs.cp16), NULL))
			goto nope;
		break;

	CASE_WIDTH_4BYTE:
		lhs.cp32 = DeeString_As4Byte((DeeObject *)self);
		if unlikely(!lhs.cp32)
			goto err;
		rhs.cp32 = DeeString_As4Byte((DeeObject *)other);
		if unlikely(!rhs.cp32)
			goto err;
		mylen = WSTR_LENGTH(lhs.cp32);
		if unlikely(begin > mylen)
			begin = mylen;
		if likely(end > mylen)
			end = mylen;
		if unlikely(end <= begin)
			goto nope;
		if (!memcasememl(lhs.cp32 + begin, end - begin,
		                 rhs.cp32, WSTR_LENGTH(rhs.cp32), NULL))
			goto nope;
		break;
	}
	return_true;
nope:
	return_false;
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
string_zfill(String *__restrict self,
             size_t argc, DeeObject **__restrict argv) {
	size_t result_length, my_len, fl_len, fill_front;
	DeeObject *filler_ob = NULL;
	DREF DeeObject *result;
	union dcharptr dst, buf, my_str, fl_str;
	if (DeeArg_Unpack(argc, argv, "Iu|o:zfill", &result_length, &filler_ob))
		goto err;
	my_len = DeeString_WLEN(self);
	if (result_length <= my_len)
		return_reference_((DeeObject *)self);
	fill_front = (result_length - my_len);
	if (filler_ob) {
		if (DeeObject_AssertTypeExact(filler_ob, &DeeString_Type))
			goto err;
		SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON(DeeString_WIDTH(self),
		                                        DeeString_WIDTH(filler_ob))) {

		CASE_WIDTH_1BYTE:
			my_str.cp8 = DeeString_As1Byte((DeeObject *)self);
			fl_str.cp8 = DeeString_As1Byte((DeeObject *)filler_ob);
			fl_len     = WSTR_LENGTH(fl_str.cp8);
			if unlikely(!fl_len)
				goto empty_filler;
			result = DeeString_NewBuffer(result_length);
			if unlikely(!result)
				goto err;
			dst.cp8 = DeeString_As1Byte(result);
			while (my_len && DeeUni_IsSign(my_str.cp8[0])) {
				*dst.cp8++ = *my_str.cp8++;
				--my_len;
			}
			memfilb(dst.cp8, fill_front, fl_str.cp8, fl_len);
			dst.cp8 += fill_front;
			memcpyb(dst.cp8, my_str.cp8, my_len);
			break;

		CASE_WIDTH_2BYTE:
			my_str.cp16 = DeeString_As2Byte((DeeObject *)self);
			if unlikely(!my_str.cp16)
				goto err;
			fl_str.cp16 = DeeString_As2Byte((DeeObject *)filler_ob);
			if unlikely(!fl_str.cp16)
				goto err;
			fl_len = WSTR_LENGTH(fl_str.cp8);
			if unlikely(!fl_len)
				goto empty_filler;
			dst.cp16 = buf.cp16 = DeeString_New2ByteBuffer(result_length);
			if unlikely(!buf.cp16)
				goto err;
			while (my_len && DeeUni_IsSign(my_str.cp16[0])) {
				*dst.cp16++ = *my_str.cp16++;
				--my_len;
			}
			memfilw(dst.cp16, fill_front, fl_str.cp16, fl_len);
			dst.cp16 += fill_front;
			memcpyw(dst.cp16, my_str.cp16, my_len);
			result = DeeString_Pack2ByteBuffer(buf.cp16);
			break;

		CASE_WIDTH_4BYTE:
			my_str.cp32 = DeeString_As4Byte((DeeObject *)self);
			if unlikely(!my_str.cp32)
				goto err;
			fl_str.cp32 = DeeString_As4Byte((DeeObject *)filler_ob);
			if unlikely(!fl_str.cp32)
				goto err;
			fl_len = WSTR_LENGTH(fl_str.cp8);
			if unlikely(!fl_len)
				goto empty_filler;
			dst.cp32 = buf.cp32 = DeeString_New4ByteBuffer(result_length);
			if unlikely(!buf.cp32)
				goto err;
			while (my_len && DeeUni_IsSign(my_str.cp32[0])) {
				*dst.cp32++ = *my_str.cp32++;
				--my_len;
			}
			memfill(dst.cp32, fill_front, fl_str.cp32, fl_len);
			dst.cp32 += fill_front;
			memcpyl(dst.cp32, my_str.cp32, my_len);
			result = DeeString_Pack4ByteBuffer(buf.cp32);
			break;
		}
	} else {
		SWITCH_SIZEOF_WIDTH(DeeString_WIDTH(self)) {

		CASE_WIDTH_1BYTE:
			my_str.cp8 = DeeString_Get1Byte((DeeObject *)self);
			result     = DeeString_NewBuffer(result_length);
			if unlikely(!result)
				goto err;
			dst.cp8 = DeeString_As1Byte(result);
			while (my_len && DeeUni_IsSign(my_str.cp8[0])) {
				*dst.cp8++ = *my_str.cp8++;
				--my_len;
			}
			memsetb(dst.cp8, UNICODE_ZERO, fill_front);
			dst.cp8 += fill_front;
			memcpyb(dst.cp8, my_str.cp8, my_len);
			break;

		CASE_WIDTH_2BYTE:
			my_str.cp16 = DeeString_Get2Byte((DeeObject *)self);
			dst.cp16 = buf.cp16 = DeeString_New2ByteBuffer(result_length);
			if unlikely(!buf.cp16)
				goto err;
			while (my_len && DeeUni_IsSign(my_str.cp16[0])) {
				*dst.cp16++ = *my_str.cp16++;
				--my_len;
			}
			memsetw(dst.cp16, UNICODE_ZERO, fill_front);
			dst.cp16 += fill_front;
			memcpyw(dst.cp16, my_str.cp16, my_len);
			result = DeeString_Pack2ByteBuffer(buf.cp16);
			break;

		CASE_WIDTH_4BYTE:
			my_str.cp32 = DeeString_Get4Byte((DeeObject *)self);
			dst.cp32 = buf.cp32 = DeeString_New4ByteBuffer(result_length);
			if unlikely(!buf.cp32)
				goto err;
			while (my_len && DeeUni_IsSign(my_str.cp32[0])) {
				*dst.cp32++ = *my_str.cp32++;
				--my_len;
			}
			memsetl(dst.cp32, UNICODE_ZERO, fill_front);
			dst.cp32 += fill_front;
			memcpyl(dst.cp32, my_str.cp32, my_len);
			result = DeeString_Pack4ByteBuffer(buf.cp32);
			break;
		}
	}
	return result;
empty_filler:
	err_empty_filler();
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
string_reversed(String *__restrict self, size_t argc,
                DeeObject **__restrict argv, DeeObject *kw) {
	size_t begin = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, substr_kwlist, "|IdId:reversed", &begin, &end))
		return NULL;
	return DeeString_Reversed((DeeObject *)self, begin, end);
}

PRIVATE DREF DeeObject *DCALL
string_expandtabs(String *__restrict self,
                  size_t argc, DeeObject **__restrict argv) {
	size_t tab_width = 8;
	if (DeeArg_Unpack(argc, argv, "|Iu:expandtabs", &tab_width))
		return NULL;
	return DeeString_ExpandTabs((DeeObject *)self, tab_width);
}

PRIVATE DREF DeeObject *DCALL
string_join(String *__restrict self,
            size_t argc, DeeObject **__restrict argv) {
	DeeObject *items;
	if (DeeArg_Unpack(argc, argv, "o:join", &items))
		return NULL;
	return DeeString_Join((DeeObject *)self, items);
}

PRIVATE DREF DeeObject *DCALL
string_unifylines(String *__restrict self,
                  size_t argc, DeeObject **__restrict argv) {
	DeeObject *replacement = NULL;
	if (DeeArg_Unpack(argc, argv, "|o:unifylines", &replacement))
		return NULL;
	if likely(!replacement)
		return DeeString_UnifyLinesLf((DeeObject *)self);
	return DeeString_UnifyLines((DeeObject *)self, replacement);
}

PRIVATE DREF DeeObject *DCALL
partition_pack_notfoundb(String *__restrict self,
                         uint8_t *__restrict lhs,
                         size_t lhs_length) {
	DREF String *lhs_string;
	DREF DeeObject *result;
	result = DeeTuple_NewUninitialized(3);
	if unlikely(!result)
		goto err;
	if (lhs == DeeString_Get1Byte((DeeObject *)self) &&
	    WSTR_LENGTH(lhs) == lhs_length) {
		lhs_string = self;
		Dee_Incref(self);
	} else {
		lhs_string = (DREF String *)DeeString_New1Byte(lhs, lhs_length);
		if unlikely(!lhs_string)
			goto err_r;
	}
	Dee_Incref_n(Dee_EmptyString, 2);
	DeeTuple_SET(result, 0, (DeeObject *)lhs_string);
	DeeTuple_SET(result, 1, (DeeObject *)Dee_EmptyString);
	DeeTuple_SET(result, 2, (DeeObject *)Dee_EmptyString);
	return result;
err_r:
	DeeTuple_FreeUninitialized(result);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
partition_pack_notfoundw(String *__restrict self,
                         uint16_t *__restrict lhs,
                         size_t lhs_length) {
	DREF String *lhs_string;
	DREF DeeObject *result;
	result = DeeTuple_NewUninitialized(3);
	if unlikely(!result)
		goto err;
	if (lhs == DeeString_Get2Byte((DeeObject *)self) &&
	    WSTR_LENGTH(lhs) == lhs_length) {
		lhs_string = self;
		Dee_Incref(self);
	} else {
		lhs_string = (DREF String *)DeeString_New2Byte(lhs, lhs_length);
		if unlikely(!lhs_string)
			goto err_r;
	}
	Dee_Incref_n(Dee_EmptyString, 2);
	DeeTuple_SET(result, 0, (DeeObject *)lhs_string);
	DeeTuple_SET(result, 1, (DeeObject *)Dee_EmptyString);
	DeeTuple_SET(result, 2, (DeeObject *)Dee_EmptyString);
	return result;
err_r:
	DeeTuple_FreeUninitialized(result);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
partition_pack_notfoundl(String *__restrict self,
                         uint32_t *__restrict lhs,
                         size_t lhs_length) {
	DREF String *lhs_string;
	DREF DeeObject *result;
	result = DeeTuple_NewUninitialized(3);
	if unlikely(!result)
		goto err;
	if (lhs == DeeString_Get4Byte((DeeObject *)self) &&
	    WSTR_LENGTH(lhs) == lhs_length) {
		lhs_string = self;
		Dee_Incref(self);
	} else {
		lhs_string = (DREF String *)DeeString_New4Byte(lhs, lhs_length);
		if unlikely(!lhs_string)
			goto err_r;
	}
	Dee_Incref_n(Dee_EmptyString, 2);
	DeeTuple_SET(result, 0, (DeeObject *)lhs_string);
	DeeTuple_SET(result, 1, (DeeObject *)Dee_EmptyString);
	DeeTuple_SET(result, 2, (DeeObject *)Dee_EmptyString);
	return result;
err_r:
	DeeTuple_FreeUninitialized(result);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
partition_packb(String *__restrict other,
                uint8_t *__restrict lhs, size_t lhs_length,
                uint8_t *__restrict ptr, size_t ptr_length) {
	DREF String *prestr, *poststr;
	DREF DeeObject *result;
	result = DeeTuple_NewUninitialized(3);
	if unlikely(!result)
		goto err;
	if (ptr == lhs) {
		uint8_t *post_ptr = ptr + ptr_length;
		poststr = (DREF String *)DeeString_New1Byte(post_ptr,
		                                            lhs_length - ptr_length);
		if unlikely(!poststr)
			goto err_r;
		prestr = (DREF String *)Dee_EmptyString;
		Dee_Incref(prestr);
	} else if (ptr == lhs + lhs_length - ptr_length) {
		prestr = (DREF String *)DeeString_New1Byte(lhs,
		                                           lhs_length - ptr_length);
		if unlikely(!prestr)
			goto err_r;
		poststr = (DREF String *)Dee_EmptyString;
		Dee_Incref(poststr);
	} else {
		uint8_t *post_ptr = ptr + ptr_length;
		prestr            = (DREF String *)DeeString_New1Byte(lhs, ptr - lhs);
		if unlikely(!prestr)
			goto err_r;
		poststr = (DREF String *)DeeString_New1Byte(post_ptr, (lhs + lhs_length) - post_ptr);
		if unlikely(!prestr)
			goto err_r_pre;
	}
	DeeTuple_SET(result, 0, (DeeObject *)prestr);
	DeeTuple_SET(result, 1, (DeeObject *)other);
	DeeTuple_SET(result, 2, (DeeObject *)poststr);
	Dee_Incref(other);
	return result;
err_r_pre:
	Dee_Decref_likely(prestr);
err_r:
	DeeTuple_FreeUninitialized(result);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
partition_packw(String *__restrict other,
                uint16_t *__restrict lhs, size_t lhs_length,
                uint16_t *__restrict ptr, size_t ptr_length) {
	DREF String *prestr, *poststr;
	DREF DeeObject *result;
	result = DeeTuple_NewUninitialized(3);
	if unlikely(!result)
		goto err;
	if (ptr == lhs) {
		uint16_t *post_ptr = ptr + ptr_length;
		poststr = (DREF String *)DeeString_New2Byte(post_ptr,
		                                            lhs_length - ptr_length);
		if unlikely(!poststr)
			goto err_r;
		prestr = (DREF String *)Dee_EmptyString;
		Dee_Incref(prestr);
	} else if (ptr == lhs + lhs_length - ptr_length) {
		prestr = (DREF String *)DeeString_New2Byte(lhs,
		                                           lhs_length - ptr_length);
		if unlikely(!prestr)
			goto err_r;
		poststr = (DREF String *)Dee_EmptyString;
		Dee_Incref(poststr);
	} else {
		uint16_t *post_ptr = ptr + ptr_length;
		prestr             = (DREF String *)DeeString_New2Byte(lhs, ptr - lhs);
		if unlikely(!prestr)
			goto err_r;
		poststr = (DREF String *)DeeString_New2Byte(post_ptr, (lhs + lhs_length) - post_ptr);
		if unlikely(!prestr)
			goto err_r_pre;
	}
	DeeTuple_SET(result, 0, (DeeObject *)prestr);
	DeeTuple_SET(result, 1, (DeeObject *)other);
	DeeTuple_SET(result, 2, (DeeObject *)poststr);
	Dee_Incref(other);
	return result;
err_r_pre:
	Dee_Decref_likely(prestr);
err_r:
	DeeTuple_FreeUninitialized(result);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
partition_packl(String *__restrict other,
                uint32_t *__restrict lhs, size_t lhs_length,
                uint32_t *__restrict ptr, size_t ptr_length) {
	DREF String *prestr, *poststr;
	DREF DeeObject *result;
	result = DeeTuple_NewUninitialized(3);
	if unlikely(!result)
		goto err;
	if (ptr == lhs) {
		uint32_t *post_ptr = ptr + ptr_length;
		poststr = (DREF String *)DeeString_New4Byte(post_ptr,
		                                            lhs_length - ptr_length);
		if unlikely(!poststr)
			goto err_r;
		prestr = (DREF String *)Dee_EmptyString;
		Dee_Incref(prestr);
	} else if (ptr == lhs + lhs_length - ptr_length) {
		prestr = (DREF String *)DeeString_New4Byte(lhs,
		                                           lhs_length - ptr_length);
		if unlikely(!prestr)
			goto err_r;
		poststr = (DREF String *)Dee_EmptyString;
		Dee_Incref(poststr);
	} else {
		uint32_t *post_ptr = ptr + ptr_length;
		prestr             = (DREF String *)DeeString_New4Byte(lhs, ptr - lhs);
		if unlikely(!prestr)
			goto err_r;
		poststr = (DREF String *)DeeString_New4Byte(post_ptr, (lhs + lhs_length) - post_ptr);
		if unlikely(!prestr)
			goto err_r_pre;
	}
	DeeTuple_SET(result, 0, (DeeObject *)prestr);
	DeeTuple_SET(result, 1, (DeeObject *)other);
	DeeTuple_SET(result, 2, (DeeObject *)poststr);
	Dee_Incref(other);
	return result;
err_r_pre:
	Dee_Decref_likely(prestr);
err_r:
	DeeTuple_FreeUninitialized(result);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
partition_packb_fold(uint8_t *__restrict lhs, size_t lhs_length,
                     uint8_t *__restrict mtc, size_t mtc_length,
                     uint8_t *__restrict rhs, size_t rhs_length) {
	DREF String *temp;
	DREF DeeObject *result;
	result = DeeTuple_NewUninitialized(3);
	if unlikely(!result)
		goto err;
	temp = (DREF String *)DeeString_New1Byte(lhs, lhs_length);
	if unlikely(!temp)
		goto err_r_0;
	DeeTuple_SET(result, 0, (DeeObject *)temp); /* Inherit reference */
	temp = (DREF String *)DeeString_New1Byte(mtc, mtc_length);
	if unlikely(!temp)
		goto err_r_1;
	DeeTuple_SET(result, 1, (DeeObject *)temp); /* Inherit reference */
	temp = (DREF String *)DeeString_New1Byte(rhs, rhs_length);
	if unlikely(!temp)
		goto err_r_2;
	DeeTuple_SET(result, 2, (DeeObject *)temp); /* Inherit reference */
	return result;
err_r_2:
	Dee_Decref_likely(DeeTuple_GET(result, 1));
err_r_1:
	Dee_Decref_likely(DeeTuple_GET(result, 0));
err_r_0:
	DeeTuple_FreeUninitialized(result);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
partition_packw_fold(uint16_t *__restrict lhs, size_t lhs_length,
                     uint16_t *__restrict mtc, size_t mtc_length,
                     uint16_t *__restrict rhs, size_t rhs_length) {
	DREF String *temp;
	DREF DeeObject *result;
	result = DeeTuple_NewUninitialized(3);
	if unlikely(!result)
		goto err;
	temp = (DREF String *)DeeString_New2Byte(lhs, lhs_length);
	if unlikely(!temp)
		goto err_r_0;
	DeeTuple_SET(result, 0, (DeeObject *)temp); /* Inherit reference */
	temp = (DREF String *)DeeString_New2Byte(mtc, mtc_length);
	if unlikely(!temp)
		goto err_r_1;
	DeeTuple_SET(result, 1, (DeeObject *)temp); /* Inherit reference */
	temp = (DREF String *)DeeString_New2Byte(rhs, rhs_length);
	if unlikely(!temp)
		goto err_r_2;
	DeeTuple_SET(result, 2, (DeeObject *)temp); /* Inherit reference */
	return result;
err_r_2:
	Dee_Decref_likely(DeeTuple_GET(result, 1));
err_r_1:
	Dee_Decref_likely(DeeTuple_GET(result, 0));
err_r_0:
	DeeTuple_FreeUninitialized(result);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
partition_packl_fold(uint32_t *__restrict lhs, size_t lhs_length,
                     uint32_t *__restrict mtc, size_t mtc_length,
                     uint32_t *__restrict rhs, size_t rhs_length) {
	DREF String *temp;
	DREF DeeObject *result;
	result = DeeTuple_NewUninitialized(3);
	if unlikely(!result)
		goto err;
	temp = (DREF String *)DeeString_New4Byte(lhs, lhs_length);
	if unlikely(!temp)
		goto err_r_0;
	DeeTuple_SET(result, 0, (DeeObject *)temp); /* Inherit reference */
	temp = (DREF String *)DeeString_New4Byte(mtc, mtc_length);
	if unlikely(!temp)
		goto err_r_1;
	DeeTuple_SET(result, 1, (DeeObject *)temp); /* Inherit reference */
	temp = (DREF String *)DeeString_New4Byte(rhs, rhs_length);
	if unlikely(!temp)
		goto err_r_2;
	DeeTuple_SET(result, 2, (DeeObject *)temp); /* Inherit reference */
	return result;
err_r_2:
	Dee_Decref_likely(DeeTuple_GET(result, 1));
err_r_1:
	Dee_Decref_likely(DeeTuple_GET(result, 0));
err_r_0:
	DeeTuple_FreeUninitialized(result);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
string_parition(String *__restrict self, size_t argc,
                DeeObject **__restrict argv, DeeObject *kw) {
	String *other;
	union dcharptr lhs, rhs, ptr;
	size_t mylen, begin = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, find_kwlist, "o|IdId:parition", &other, &begin, &end) ||
	    DeeObject_AssertTypeExact((DeeObject *)other, &DeeString_Type))
		goto err;
	SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON(DeeString_WIDTH(self),
	                                        DeeString_WIDTH(other))) {

	CASE_WIDTH_1BYTE:
		lhs.cp8 = DeeString_As1Byte((DeeObject *)self);
		rhs.cp8 = DeeString_As1Byte((DeeObject *)other);
		mylen   = WSTR_LENGTH(lhs.ptr);
		if unlikely(begin > mylen)
			begin = mylen;
		if likely(end > mylen)
			end = mylen;
		if unlikely(end <= begin)
			goto not_foundb_zero;
		end -= begin;
		lhs.cp8 += begin;
		ptr.cp8 = memmemb(lhs.cp8, end,
		                  rhs.cp8, WSTR_LENGTH(rhs.cp8));
		if (!ptr.cp8)
			goto not_foundb;
		return partition_packb(other,
		                       lhs.cp8, end,
		                       ptr.cp8, WSTR_LENGTH(rhs.cp8));
not_foundb_zero:
		end = 0;
not_foundb:
		return partition_pack_notfoundb(self, lhs.cp8, end);

	CASE_WIDTH_2BYTE:
		lhs.cp16 = DeeString_As2Byte((DeeObject *)self);
		if unlikely(!lhs.cp16)
			goto err;
		rhs.cp16 = DeeString_As2Byte((DeeObject *)other);
		if unlikely(!rhs.cp16)
			goto err;
		mylen = WSTR_LENGTH(lhs.ptr);
		if unlikely(begin > mylen)
			begin = mylen;
		if likely(end > mylen)
			end = mylen;
		if unlikely(end <= begin)
			goto not_foundw_zero;
		end -= begin;
		lhs.cp16 += begin;
		ptr.cp16 = memmemw(lhs.cp16, end,
		                   rhs.cp16, WSTR_LENGTH(rhs.cp16));
		if (!ptr.cp16)
			goto not_foundw;
		return partition_packw(other,
		                       lhs.cp16, end,
		                       ptr.cp16, WSTR_LENGTH(rhs.cp16));
not_foundw_zero:
		end = 0;
not_foundw:
		return partition_pack_notfoundw(self, lhs.cp16, end);

	CASE_WIDTH_4BYTE:
		lhs.cp32 = DeeString_As4Byte((DeeObject *)self);
		if unlikely(!lhs.cp32)
			goto err;
		rhs.cp32 = DeeString_As4Byte((DeeObject *)other);
		if unlikely(!rhs.cp32)
			goto err;
		mylen = WSTR_LENGTH(lhs.ptr);
		if unlikely(begin > mylen)
			begin = mylen;
		if likely(end > mylen)
			end = mylen;
		if unlikely(end <= begin)
			goto not_foundl_zero;
		end -= begin;
		lhs.cp32 += begin;
		ptr.cp32 = memmeml(lhs.cp32, end,
		                   rhs.cp32, WSTR_LENGTH(rhs.cp32));
		if (!ptr.cp32)
			goto not_foundl;
		return partition_packl(other,
		                       lhs.cp32, end,
		                       ptr.cp32, WSTR_LENGTH(rhs.cp32));
not_foundl_zero:
		end = 0;
not_foundl:
		return partition_pack_notfoundl(self, lhs.cp32, end);
	}
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
string_rparition(String *__restrict self, size_t argc,
                 DeeObject **__restrict argv, DeeObject *kw) {
	String *other;
	union dcharptr lhs, rhs, ptr;
	size_t mylen, begin = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, find_kwlist, "o|IdId:rparition", &other, &begin, &end) ||
	    DeeObject_AssertTypeExact((DeeObject *)other, &DeeString_Type))
		goto err;
	SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON(DeeString_WIDTH(self),
	                                        DeeString_WIDTH(other))) {

	CASE_WIDTH_1BYTE:
		lhs.cp8 = DeeString_As1Byte((DeeObject *)self);
		rhs.cp8 = DeeString_As1Byte((DeeObject *)other);
		mylen   = WSTR_LENGTH(lhs.ptr);
		if unlikely(begin > mylen)
			begin = mylen;
		if likely(end > mylen)
			end = mylen;
		if unlikely(end <= begin)
			goto not_foundb_zero;
		end -= begin;
		lhs.cp8 += begin;
		ptr.cp8 = memrmemb(lhs.cp8, end,
		                   rhs.cp8, WSTR_LENGTH(rhs.cp8));
		if (!ptr.cp8)
			goto not_foundb;
		return partition_packb(other,
		                       lhs.cp8, end,
		                       ptr.cp8, WSTR_LENGTH(rhs.cp8));
not_foundb_zero:
		end = 0;
not_foundb:
		return partition_pack_notfoundb(self, lhs.cp8, end);

	CASE_WIDTH_2BYTE:
		lhs.cp16 = DeeString_As2Byte((DeeObject *)self);
		if unlikely(!lhs.cp16)
			goto err;
		rhs.cp16 = DeeString_As2Byte((DeeObject *)other);
		if unlikely(!rhs.cp16)
			goto err;
		mylen = WSTR_LENGTH(lhs.ptr);
		if unlikely(begin > mylen)
			begin = mylen;
		if likely(end > mylen)
			end = mylen;
		if unlikely(end <= begin)
			goto not_foundw_zero;
		end -= begin;
		lhs.cp16 += begin;
		ptr.cp16 = memrmemw(lhs.cp16, end,
		                    rhs.cp16, WSTR_LENGTH(rhs.cp16));
		if (!ptr.cp16)
			goto not_foundw;
		return partition_packw(other,
		                       lhs.cp16, end,
		                       ptr.cp16, WSTR_LENGTH(rhs.cp16));
not_foundw_zero:
		end = 0;
not_foundw:
		return partition_pack_notfoundw(self, lhs.cp16, end);

	CASE_WIDTH_4BYTE:
		lhs.cp32 = DeeString_As4Byte((DeeObject *)self);
		if unlikely(!lhs.cp32)
			goto err;
		rhs.cp32 = DeeString_As4Byte((DeeObject *)other);
		if unlikely(!rhs.cp32)
			goto err;
		mylen = WSTR_LENGTH(lhs.ptr);
		if unlikely(begin > mylen)
			begin = mylen;
		if likely(end > mylen)
			end = mylen;
		if unlikely(end <= begin)
			goto not_foundl_zero;
		end -= begin;
		lhs.cp32 += begin;
		ptr.cp32 = memrmeml(lhs.cp32, end,
		                    rhs.cp32, WSTR_LENGTH(rhs.cp32));
		if (!ptr.cp32)
			goto not_foundl;
		return partition_packl(other,
		                       lhs.cp32, end,
		                       ptr.cp32, WSTR_LENGTH(rhs.cp32));
not_foundl_zero:
		end = 0;
not_foundl:
		return partition_pack_notfoundl(self, lhs.cp32, end);
	}
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
string_caseparition(String *__restrict self, size_t argc,
                    DeeObject **__restrict argv, DeeObject *kw) {
	String *other;
	union dcharptr lhs, rhs, ptr;
	size_t mylen, begin = 0, end = (size_t)-1, match_length;
	if (DeeArg_UnpackKw(argc, argv, kw, find_kwlist, "o|IdId:caseparition", &other, &begin, &end) ||
	    DeeObject_AssertTypeExact((DeeObject *)other, &DeeString_Type))
		goto err;
	SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON(DeeString_WIDTH(self),
	                                        DeeString_WIDTH(other))) {

	CASE_WIDTH_1BYTE:
		lhs.cp8 = DeeString_As1Byte((DeeObject *)self);
		rhs.cp8 = DeeString_As1Byte((DeeObject *)other);
		mylen   = WSTR_LENGTH(lhs.ptr);
		if unlikely(begin > mylen)
			begin = mylen;
		if likely(end > mylen)
			end = mylen;
		if unlikely(end <= begin)
			goto not_foundb_zero;
		end -= begin;
		lhs.cp8 += begin;
		ptr.cp8 = memcasememb(lhs.cp8, end,
		                      rhs.cp8, WSTR_LENGTH(rhs.cp8),
		                      &match_length);
		if (!ptr.cp8)
			goto not_foundb;
		return partition_packb_fold(lhs.cp8, (size_t)(ptr.cp8 - lhs.cp8),
		                            ptr.cp8, match_length,
		                            ptr.cp8 + match_length,
		                            end - ((size_t)(ptr.cp8 - lhs.cp8) + match_length));
not_foundb_zero:
		end = 0;
not_foundb:
		return partition_pack_notfoundb(self, lhs.cp8, end);

	CASE_WIDTH_2BYTE:
		lhs.cp16 = DeeString_As2Byte((DeeObject *)self);
		if unlikely(!lhs.cp16)
			goto err;
		rhs.cp16 = DeeString_As2Byte((DeeObject *)other);
		if unlikely(!rhs.cp16)
			goto err;
		mylen = WSTR_LENGTH(lhs.ptr);
		if unlikely(begin > mylen)
			begin = mylen;
		if likely(end > mylen)
			end = mylen;
		if unlikely(end <= begin)
			goto not_foundw_zero;
		end -= begin;
		lhs.cp16 += begin;
		ptr.cp16 = memcasememw(lhs.cp16, end,
		                       rhs.cp16, WSTR_LENGTH(rhs.cp16),
		                       &match_length);
		if (!ptr.cp16)
			goto not_foundw;
		return partition_packw_fold(lhs.cp16, (size_t)(ptr.cp16 - lhs.cp16),
		                            ptr.cp16, match_length,
		                            ptr.cp16 + match_length,
		                            end - ((size_t)(ptr.cp16 - lhs.cp16) + match_length));
not_foundw_zero:
		end = 0;
not_foundw:
		return partition_pack_notfoundw(self, lhs.cp16, end);

	CASE_WIDTH_4BYTE:
		lhs.cp32 = DeeString_As4Byte((DeeObject *)self);
		if unlikely(!lhs.cp32)
			goto err;
		rhs.cp32 = DeeString_As4Byte((DeeObject *)other);
		if unlikely(!rhs.cp32)
			goto err;
		mylen = WSTR_LENGTH(lhs.ptr);
		if unlikely(begin > mylen)
			begin = mylen;
		if likely(end > mylen)
			end = mylen;
		if unlikely(end <= begin)
			goto not_foundl_zero;
		end -= begin;
		lhs.cp32 += begin;
		ptr.cp32 = memcasememl(lhs.cp32, end,
		                       rhs.cp32, WSTR_LENGTH(rhs.cp32),
		                       &match_length);
		if (!ptr.cp32)
			goto not_foundl;
		return partition_packl_fold(lhs.cp32, (size_t)(ptr.cp32 - lhs.cp32),
		                            ptr.cp32, match_length,
		                            ptr.cp32 + match_length,
		                            end - ((size_t)(ptr.cp32 - lhs.cp32) + match_length));
not_foundl_zero:
		end = 0;
not_foundl:
		return partition_pack_notfoundl(self, lhs.cp32, end);
	}
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
string_caserparition(String *__restrict self, size_t argc,
                     DeeObject **__restrict argv, DeeObject *kw) {
	String *other;
	union dcharptr lhs, rhs, ptr;
	size_t mylen, begin = 0, end = (size_t)-1, match_length;
	if (DeeArg_UnpackKw(argc, argv, kw, find_kwlist, "o|IdId:caserparition", &other, &begin, &end) ||
	    DeeObject_AssertTypeExact((DeeObject *)other, &DeeString_Type))
		goto err;
	SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON(DeeString_WIDTH(self),
	                                        DeeString_WIDTH(other))) {

	CASE_WIDTH_1BYTE:
		lhs.cp8 = DeeString_As1Byte((DeeObject *)self);
		rhs.cp8 = DeeString_As1Byte((DeeObject *)other);
		mylen   = WSTR_LENGTH(lhs.ptr);
		if unlikely(begin > mylen)
			begin = mylen;
		if likely(end > mylen)
			end = mylen;
		if unlikely(end <= begin)
			goto not_foundb_zero;
		end -= begin;
		lhs.cp8 += begin;
		ptr.cp8 = memcasermemb(lhs.cp8, end,
		                       rhs.cp8, WSTR_LENGTH(rhs.cp8),
		                       &match_length);
		if (!ptr.cp8)
			goto not_foundb;
		return partition_packb_fold(lhs.cp8, (size_t)(ptr.cp8 - lhs.cp8),
		                            ptr.cp8, match_length,
		                            ptr.cp8 + match_length,
		                            end - ((size_t)(ptr.cp8 - lhs.cp8) + match_length));
not_foundb_zero:
		end = 0;
not_foundb:
		return partition_pack_notfoundb(self, lhs.cp8, end);

	CASE_WIDTH_2BYTE:
		lhs.cp16 = DeeString_As2Byte((DeeObject *)self);
		if unlikely(!lhs.cp16)
			goto err;
		rhs.cp16 = DeeString_As2Byte((DeeObject *)other);
		if unlikely(!rhs.cp16)
			goto err;
		mylen = WSTR_LENGTH(lhs.ptr);
		if unlikely(begin > mylen)
			begin = mylen;
		if likely(end > mylen)
			end = mylen;
		if unlikely(end <= begin)
			goto not_foundw_zero;
		end -= begin;
		lhs.cp16 += begin;
		ptr.cp16 = memcasermemw(lhs.cp16, end,
		                        rhs.cp16, WSTR_LENGTH(rhs.cp16),
		                        &match_length);
		if (!ptr.cp16)
			goto not_foundw;
		return partition_packw_fold(lhs.cp16, (size_t)(ptr.cp16 - lhs.cp16),
		                            ptr.cp16, match_length,
		                            ptr.cp16 + match_length,
		                            end - ((size_t)(ptr.cp16 - lhs.cp16) + match_length));
not_foundw_zero:
		end = 0;
not_foundw:
		return partition_pack_notfoundw(self, lhs.cp16, end);

	CASE_WIDTH_4BYTE:
		lhs.cp32 = DeeString_As4Byte((DeeObject *)self);
		if unlikely(!lhs.cp32)
			goto err;
		rhs.cp32 = DeeString_As4Byte((DeeObject *)other);
		if unlikely(!rhs.cp32)
			goto err;
		mylen = WSTR_LENGTH(lhs.ptr);
		if unlikely(begin > mylen)
			begin = mylen;
		if likely(end > mylen)
			end = mylen;
		if unlikely(end <= begin)
			goto not_foundl_zero;
		end -= begin;
		lhs.cp32 += begin;
		ptr.cp32 = memcasermeml(lhs.cp32, end,
		                        rhs.cp32, WSTR_LENGTH(rhs.cp32),
		                        &match_length);
		if (!ptr.cp32)
			goto not_foundl;
		return partition_packl_fold(lhs.cp32, (size_t)(ptr.cp32 - lhs.cp32),
		                            ptr.cp32, match_length,
		                            ptr.cp32 + match_length,
		                            end - ((size_t)(ptr.cp32 - lhs.cp32) + match_length));
not_foundl_zero:
		end = 0;
not_foundl:
		return partition_pack_notfoundl(self, lhs.cp32, end);
	}
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
string_sstrip(String *__restrict self,
              size_t argc, DeeObject **__restrict argv) {
	DeeObject *other;
	if (DeeArg_Unpack(argc, argv, "o:sstrip", &other) ||
	    DeeObject_AssertTypeExact(other, &DeeString_Type))
		return NULL;
	return DeeString_SStrip((DeeObject *)self, other);
}

PRIVATE DREF DeeObject *DCALL
string_lsstrip(String *__restrict self,
               size_t argc, DeeObject **__restrict argv) {
	DeeObject *other;
	if (DeeArg_Unpack(argc, argv, "o:lsstrip", &other) ||
	    DeeObject_AssertTypeExact(other, &DeeString_Type))
		return NULL;
	return DeeString_LSStrip((DeeObject *)self, other);
}

PRIVATE DREF DeeObject *DCALL
string_rsstrip(String *__restrict self,
               size_t argc, DeeObject **__restrict argv) {
	DeeObject *other;
	if (DeeArg_Unpack(argc, argv, "o:rsstrip", &other) ||
	    DeeObject_AssertTypeExact(other, &DeeString_Type))
		return NULL;
	return DeeString_RSStrip((DeeObject *)self, other);
}

PRIVATE DREF DeeObject *DCALL
string_casesstrip(String *__restrict self,
                  size_t argc, DeeObject **__restrict argv) {
	DeeObject *other;
	if (DeeArg_Unpack(argc, argv, "o:casesstrip", &other) ||
	    DeeObject_AssertTypeExact(other, &DeeString_Type))
		return NULL;
	return DeeString_CaseSStrip((DeeObject *)self, other);
}

PRIVATE DREF DeeObject *DCALL
string_caselsstrip(String *__restrict self,
                   size_t argc, DeeObject **__restrict argv) {
	DeeObject *other;
	if (DeeArg_Unpack(argc, argv, "o:caselsstrip", &other) ||
	    DeeObject_AssertTypeExact(other, &DeeString_Type))
		return NULL;
	return DeeString_CaseLSStrip((DeeObject *)self, other);
}

PRIVATE DREF DeeObject *DCALL
string_casersstrip(String *__restrict self,
                   size_t argc, DeeObject **__restrict argv) {
	DeeObject *other;
	if (DeeArg_Unpack(argc, argv, "o:casersstrip", &other) ||
	    DeeObject_AssertTypeExact(other, &DeeString_Type))
		return NULL;
	return DeeString_CaseRSStrip((DeeObject *)self, other);
}

struct compare_args {
	DeeObject *other;
	size_t my_start;
	size_t my_end;
	size_t ot_start;
	size_t ot_end;
};

PRIVATE int DCALL
get_compare_args(struct compare_args *__restrict args,
                 size_t argc, DeeObject **__restrict argv,
                 char const *__restrict funname) {
	args->my_start = 0;
	args->my_end   = (size_t)-1;
	args->ot_start = 0;
	args->ot_end   = (size_t)-1;
	switch (argc) {
	case 1:
		args->other = argv[0];
		if (DeeObject_AssertTypeExact(args->other, &DeeString_Type))
			goto err;
		break;
	case 2:
		if (DeeString_Check(argv[0])) {
			args->other = argv[0];
			if (DeeObject_AsSSize(argv[1], (dssize_t *)&args->ot_start))
				goto err;
		} else {
			if (DeeObject_AsSSize(argv[0], (dssize_t *)&args->my_start))
				goto err;
			args->other = argv[1];
			if (DeeObject_AssertTypeExact(args->other, &DeeString_Type))
				goto err;
		}
		break;
	case 3:
		if (DeeString_Check(argv[0])) {
			args->other = argv[0];
			if (DeeObject_AsSSize(argv[1], (dssize_t *)&args->ot_start))
				goto err;
			if (DeeObject_AsSSize(argv[2], (dssize_t *)&args->ot_end))
				goto err;
		} else if (DeeString_Check(argv[1])) {
			if (DeeObject_AsSSize(argv[0], (dssize_t *)&args->my_start))
				goto err;
			args->other = argv[1];
			if (DeeObject_AsSSize(argv[2], (dssize_t *)&args->ot_start))
				goto err;
		} else {
			if (DeeObject_AsSSize(argv[0], (dssize_t *)&args->my_start))
				goto err;
			if (DeeObject_AsSSize(argv[1], (dssize_t *)&args->my_end))
				goto err;
			args->other = argv[2];
			if (DeeObject_AssertTypeExact(args->other, &DeeString_Type))
				goto err;
		}
		break;
	case 4:
		if (DeeObject_AsSSize(argv[0], (dssize_t *)&args->my_start))
			goto err;
		if (DeeString_Check(argv[1])) {
			args->other = argv[1];
			if (DeeObject_AsSSize(argv[2], (dssize_t *)&args->ot_start))
				goto err;
			if (DeeObject_AsSSize(argv[3], (dssize_t *)&args->ot_end))
				goto err;
		} else {
			if (DeeObject_AsSSize(argv[1], (dssize_t *)&args->my_end))
				goto err;
			args->other = argv[2];
			if (DeeObject_AsSSize(argv[3], (dssize_t *)&args->ot_start))
				goto err;
			if (DeeObject_AssertTypeExact(args->other, &DeeString_Type))
				goto err;
		}
		break;
	case 5:
		if (DeeObject_AsSSize(argv[0], (dssize_t *)&args->my_start))
			goto err;
		if (DeeObject_AsSSize(argv[1], (dssize_t *)&args->my_end))
			goto err;
		args->other = argv[2];
		if (DeeObject_AssertTypeExact(args->other, &DeeString_Type))
			goto err;
		if (DeeObject_AsSSize(argv[3], (dssize_t *)&args->ot_start))
			goto err;
		if (DeeObject_AsSSize(argv[4], (dssize_t *)&args->ot_end))
			goto err;
		break;
	default:
		err_invalid_argc(funname, argc, 1, 5);
		goto err;
	}
	return 0;
err:
	return -1;
}


PRIVATE int DCALL
compare_strings_ex(String *__restrict lhs, size_t lhs_start, size_t lhs_end,
                   String *__restrict rhs, size_t rhs_start, size_t rhs_end) {
	size_t lhs_len;
	size_t rhs_len;
	if (!lhs->s_data ||
	    lhs->s_data->u_width == STRING_WIDTH_1BYTE) {
		uint8_t *lhs_str;
		/* Compare against single-byte string. */
		lhs_str = (uint8_t *)lhs->s_str;
		lhs_len = lhs->s_len;
		if (lhs_end > lhs_len)
			lhs_end = lhs_len;
		if (lhs_start >= lhs_end)
			lhs_len = 0;
		else {
			lhs_str += lhs_start;
			lhs_len = lhs_end - lhs_start;
		}
		if (!rhs->s_data ||
		    rhs->s_data->u_width == STRING_WIDTH_1BYTE) {
			int result;
			rhs_len = rhs->s_len;
			if (rhs_end > rhs_len)
				rhs_end = rhs_len;
			if (rhs_start >= rhs_end)
				rhs_len = 0;
			else {
				rhs_len = rhs_end - rhs_start;
			}
			/* Most simple case: compare ascii/single-byte strings. */
			result = memcmp(lhs_str, rhs->s_str + rhs_start, MIN(lhs_len, rhs_len));
			if (result != 0)
				return result;
		} else {
			struct string_utf *rhs_utf = rhs->s_data;
			switch (rhs_utf->u_width) {

				{
					uint16_t *rhs_str;
					size_t i, common_len;

				CASE_WIDTH_2BYTE:
					rhs_str = (uint16_t *)rhs_utf->u_data[rhs_utf->u_width];
					rhs_len = WSTR_LENGTH(rhs_str);
					if (rhs_end > rhs_len)
						rhs_end = rhs_len;
					if (rhs_start >= rhs_end)
						rhs_len = 0;
					else {
						rhs_str += rhs_start;
						rhs_len = rhs_end - rhs_start;
					}
					common_len = MIN(lhs_len, rhs_len);
					for (i = 0; i < common_len; ++i) {
						if ((uint16_t)(lhs_str[i]) == rhs_str[i])
							continue;
						return (uint16_t)(lhs_str[i]) < rhs_str[i] ? -1 : 1;
					}
				}
				break;

				{
					uint32_t *rhs_str;
					size_t i, common_len;

				CASE_WIDTH_4BYTE:
					rhs_str = (uint32_t *)rhs_utf->u_data[rhs_utf->u_width];
					rhs_len = WSTR_LENGTH(rhs_str);
					if (rhs_end > rhs_len)
						rhs_end = rhs_len;
					if (rhs_start >= rhs_end)
						rhs_len = 0;
					else {
						rhs_str += rhs_start;
						rhs_len = rhs_end - rhs_start;
					}
					common_len = MIN(lhs_len, rhs_len);
					for (i = 0; i < common_len; ++i) {
						if ((uint32_t)(lhs_str[i]) == rhs_str[i])
							continue;
						return (uint32_t)(lhs_str[i]) < rhs_str[i] ? -1 : 1;
					}
				}
				break;

			default: __builtin_unreachable();
			}
		}
	} else if (!rhs->s_data ||
	           rhs->s_data->u_width == STRING_WIDTH_1BYTE) {
		uint8_t *rhs_str;
		struct string_utf *lhs_utf;
		/* Compare against single-byte string. */
		rhs_str = (uint8_t *)rhs->s_str;
		rhs_len = rhs->s_len;
		if (rhs_end > rhs_len)
			rhs_end = rhs_len;
		if (rhs_start >= rhs_end)
			rhs_len = 0;
		else {
			rhs_str += rhs_start;
			rhs_len = rhs_end - rhs_start;
		}
		lhs_utf = lhs->s_data;
		switch (lhs_utf->u_width) {

			{
				uint16_t *lhs_str;
				size_t i, common_len;

			CASE_WIDTH_2BYTE:
				lhs_str = (uint16_t *)lhs_utf->u_data[lhs_utf->u_width];
				lhs_len = WSTR_LENGTH(lhs_str);
				if (lhs_end > lhs_len)
					lhs_end = lhs_len;
				if (lhs_start >= lhs_end)
					lhs_len = 0;
				else {
					lhs_str += lhs_start;
					lhs_len = lhs_end - lhs_start;
				}
				common_len = MIN(rhs_len, lhs_len);
				for (i = 0; i < common_len; ++i) {
					if (lhs_str[i] == (uint16_t)(rhs_str[i]))
						continue;
					return lhs_str[i] < (uint16_t)(rhs_str[i]) ? -1 : 1;
				}
			}
			break;

			{
				uint32_t *lhs_str;
				size_t i, common_len;

			CASE_WIDTH_4BYTE:
				lhs_str = (uint32_t *)lhs_utf->u_data[lhs_utf->u_width];
				lhs_len = WSTR_LENGTH(lhs_str);
				if (lhs_end > lhs_len)
					lhs_end = lhs_len;
				if (lhs_start >= lhs_end)
					lhs_len = 0;
				else {
					lhs_str += lhs_start;
					lhs_len = lhs_end - lhs_start;
				}
				common_len = MIN(rhs_len, lhs_len);
				for (i = 0; i < common_len; ++i) {
					if (lhs_str[i] == (uint32_t)(rhs_str[i]))
						continue;
					return lhs_str[i] < (uint32_t)(rhs_str[i]) ? -1 : 1;
				}
			}
			break;

		default: __builtin_unreachable();
		}
	} else {
		struct string_utf *lhs_utf;
		struct string_utf *rhs_utf;
		lhs_utf = lhs->s_data;
		rhs_utf = rhs->s_data;
		ASSERT(lhs_utf);
		ASSERT(rhs_utf);
		ASSERT(lhs_utf->u_width != STRING_WIDTH_1BYTE);
		ASSERT(rhs_utf->u_width != STRING_WIDTH_1BYTE);
		/* Complex string comparison. */
		switch (lhs_utf->u_width) {

			{
				uint16_t *lhs_str;

			CASE_WIDTH_2BYTE:
				lhs_str = (uint16_t *)lhs_utf->u_data[lhs_utf->u_width];
				lhs_len = WSTR_LENGTH(lhs_str);
				if (lhs_end > lhs_len)
					lhs_end = lhs_len;
				if (lhs_start >= lhs_end)
					lhs_len = 0;
				else {
					lhs_str += lhs_start;
					lhs_len = lhs_end - lhs_start;
				}
				switch (rhs_utf->u_width) {

					{
						uint16_t *rhs_str;
						size_t common_len;

					CASE_WIDTH_2BYTE:
						rhs_str = (uint16_t *)rhs_utf->u_data[rhs_utf->u_width];
						rhs_len = WSTR_LENGTH(rhs_str);
						if (rhs_end > rhs_len)
							rhs_end = rhs_len;
						if (rhs_start >= rhs_end)
							rhs_len = 0;
						else {
							rhs_str += rhs_start;
							rhs_len = rhs_end - rhs_start;
						}
						common_len = MIN(lhs_len, rhs_len);
#ifdef __USE_KOS
						{
							int result;
							result = memcmpw(lhs_str, rhs_str, common_len);
							if (result != 0)
								return result;
						}
#else
						{
							size_t i;
							for (i = 0; i < common_len; ++i) {
								if (lhs_str[i] == rhs_str[i])
									continue;
								return lhs_str[i] < rhs_str[i] ? -1 : 1;
							}
						}
#endif
					}
					break;

					{
						uint32_t *rhs_str;
						size_t i, common_len;

					CASE_WIDTH_4BYTE:
						rhs_str = (uint32_t *)rhs_utf->u_data[rhs_utf->u_width];
						rhs_len = WSTR_LENGTH(rhs_str);
						if (rhs_end > rhs_len)
							rhs_end = rhs_len;
						if (rhs_start >= rhs_end)
							rhs_len = 0;
						else {
							rhs_str += rhs_start;
							rhs_len = rhs_end - rhs_start;
						}
						common_len = MIN(lhs_len, rhs_len);
						for (i = 0; i < common_len; ++i) {
							if ((uint32_t)(lhs_str[i]) == rhs_str[i])
								continue;
							return (uint32_t)(lhs_str[i]) < rhs_str[i] ? -1 : 1;
						}
					}
					break;

				default: __builtin_unreachable();
				}
			}
			break;

			{
				uint32_t *lhs_str;

			CASE_WIDTH_4BYTE:
				lhs_str = (uint32_t *)lhs_utf->u_data[lhs_utf->u_width];
				lhs_len = WSTR_LENGTH(lhs_str);
				if (lhs_end > lhs_len)
					lhs_end = lhs_len;
				if (lhs_start >= lhs_end)
					lhs_len = 0;
				else {
					lhs_str += lhs_start;
					lhs_len = lhs_end - lhs_start;
				}
				switch (rhs_utf->u_width) {

					{
						uint16_t *rhs_str;
						size_t i, common_len;

					CASE_WIDTH_2BYTE:
						rhs_str = (uint16_t *)rhs_utf->u_data[rhs_utf->u_width];
						rhs_len = WSTR_LENGTH(rhs_str);
						if (rhs_end > rhs_len)
							rhs_end = rhs_len;
						if (rhs_start >= rhs_end)
							rhs_len = 0;
						else {
							rhs_str += rhs_start;
							rhs_len = rhs_end - rhs_start;
						}
						common_len = MIN(lhs_len, rhs_len);
						for (i = 0; i < common_len; ++i) {
							if (lhs_str[i] == (uint32_t)(rhs_str[i]))
								continue;
							return lhs_str[i] < (uint32_t)(rhs_str[i]) ? -1 : 1;
						}
					}
					break;

					{
						uint32_t *rhs_str;
						size_t common_len;

					CASE_WIDTH_4BYTE:
						rhs_str = (uint32_t *)rhs_utf->u_data[rhs_utf->u_width];
						rhs_len = WSTR_LENGTH(rhs_str);
						if (rhs_end > rhs_len)
							rhs_end = rhs_len;
						if (rhs_start >= rhs_end)
							rhs_len = 0;
						else {
							rhs_str += rhs_start;
							rhs_len = rhs_end - rhs_start;
						}
						common_len = MIN(lhs_len, rhs_len);
#ifdef __USE_KOS
						{
							int result;
							result = memcmpl(lhs_str, rhs_str, common_len);
							if (result != 0)
								return result;
						}
#else
						{
							size_t i;
							for (i = 0; i < common_len; ++i) {
								if (lhs_str[i] == rhs_str[i])
									continue;
								return lhs_str[i] < rhs_str[i] ? -1 : 1;
							}
						}
#endif
					}
					break;

				default: __builtin_unreachable();
				}
			}
			break;

		default: __builtin_unreachable();
		}
	}

	/* If string contents are identical, leave off by comparing their lengths. */
	if (lhs_len == rhs_len)
		return 0;
	if (lhs_len < rhs_len)
		return -1;
	return 1;
}

PRIVATE int DCALL
casecompare_strings_ex(String *__restrict lhs, size_t lhs_start, size_t lhs_end,
                       String *__restrict rhs, size_t rhs_start, size_t rhs_end) {
	size_t lhs_len;
	size_t rhs_len;
	union {
		struct unicode_foldreaderb b;
		struct unicode_foldreaderw w;
		struct unicode_foldreaderl l;
	} lhs_reader, rhs_reader;
#define DO_COMPARE(lhs_bwl, lhs_str, lhs_len, rhs_bwl, rhs_str, rhs_len)      \
	{                                                                         \
		unicode_foldreader_init(lhs_reader.lhs_bwl, lhs_str, lhs_len);        \
		unicode_foldreader_init(rhs_reader.rhs_bwl, rhs_str, rhs_len);        \
		for (;;) {                                                            \
			uint32_t ch_lhs, ch_rhs;                                          \
			if (unicode_foldreader_empty(lhs_reader.lhs_bwl))                 \
				return unicode_foldreader_empty(rhs_reader.rhs_bwl) ? 0 : -1; \
			if (unicode_foldreader_empty(rhs_reader.rhs_bwl))                 \
				return 1;                                                     \
			ch_lhs = unicode_foldreader_getc(lhs_reader.lhs_bwl);             \
			ch_rhs = unicode_foldreader_getc(rhs_reader.rhs_bwl);             \
			if (ch_lhs != ch_rhs)                                             \
				return ch_lhs < ch_rhs ? -1 : 1;                              \
		}                                                                     \
	}


	if (!lhs->s_data ||
	    lhs->s_data->u_width == STRING_WIDTH_1BYTE) {
		uint8_t *lhs_str;
		/* Compare against single-byte string. */
		lhs_str = (uint8_t *)lhs->s_str;
		lhs_len = lhs->s_len;
		if (lhs_end > lhs_len)
			lhs_end = lhs_len;
		if (lhs_start >= lhs_end)
			lhs_len = 0;
		else {
			lhs_str += lhs_start;
			lhs_len = lhs_end - lhs_start;
		}
		if (!rhs->s_data ||
		    rhs->s_data->u_width == STRING_WIDTH_1BYTE) {
			rhs_len = rhs->s_len;
			if (rhs_end > rhs_len)
				rhs_end = rhs_len;
			if (rhs_start >= rhs_end)
				rhs_len = 0;
			else {
				rhs_len = rhs_end - rhs_start;
			}
			/* Most simple case: compare ascii/single-byte strings. */
			return dee_memcasecmpb(lhs_str, lhs_len,
			                       (uint8_t *)rhs->s_str + rhs_start, rhs_len);
		} else {
			struct string_utf *rhs_utf = rhs->s_data;
			switch (rhs_utf->u_width) {

				{
					uint16_t *rhs_str;

				CASE_WIDTH_2BYTE:
					rhs_str = (uint16_t *)rhs_utf->u_data[rhs_utf->u_width];
					rhs_len = WSTR_LENGTH(rhs_str);
					if (rhs_end > rhs_len)
						rhs_end = rhs_len;
					if (rhs_start >= rhs_end)
						rhs_len = 0;
					else {
						rhs_str += rhs_start;
						rhs_len = rhs_end - rhs_start;
					}
					DO_COMPARE(b, lhs_str, lhs_len,
					           w, rhs_str, rhs_len);
				}
				break;

				{
					uint32_t *rhs_str;

				CASE_WIDTH_4BYTE:
					rhs_str = (uint32_t *)rhs_utf->u_data[rhs_utf->u_width];
					rhs_len = WSTR_LENGTH(rhs_str);
					if (rhs_end > rhs_len)
						rhs_end = rhs_len;
					if (rhs_start >= rhs_end)
						rhs_len = 0;
					else {
						rhs_str += rhs_start;
						rhs_len = rhs_end - rhs_start;
					}
					DO_COMPARE(b, lhs_str, lhs_len,
					           l, rhs_str, rhs_len);
				}
				break;

			default: __builtin_unreachable();
			}
		}
	} else if (!rhs->s_data ||
	           rhs->s_data->u_width == STRING_WIDTH_1BYTE) {
		uint8_t *rhs_str;
		struct string_utf *lhs_utf;
		/* Compare against single-byte string. */
		rhs_str = (uint8_t *)rhs->s_str;
		rhs_len = rhs->s_len;
		if (rhs_end > rhs_len)
			rhs_end = rhs_len;
		if (rhs_start >= rhs_end)
			rhs_len = 0;
		else {
			rhs_str += rhs_start;
			rhs_len = rhs_end - rhs_start;
		}
		lhs_utf = lhs->s_data;
		switch (lhs_utf->u_width) {

			{
				uint16_t *lhs_str;

			CASE_WIDTH_2BYTE:
				lhs_str = (uint16_t *)lhs_utf->u_data[lhs_utf->u_width];
				lhs_len = WSTR_LENGTH(lhs_str);
				if (lhs_end > lhs_len)
					lhs_end = lhs_len;
				if (lhs_start >= lhs_end)
					lhs_len = 0;
				else {
					lhs_str += lhs_start;
					lhs_len = lhs_end - lhs_start;
				}
				DO_COMPARE(w, lhs_str, lhs_len,
				           b, rhs_str, rhs_len);
			}
			break;

			{
				uint32_t *lhs_str;

			CASE_WIDTH_4BYTE:
				lhs_str = (uint32_t *)lhs_utf->u_data[lhs_utf->u_width];
				lhs_len = WSTR_LENGTH(lhs_str);
				if (lhs_end > lhs_len)
					lhs_end = lhs_len;
				if (lhs_start >= lhs_end)
					lhs_len = 0;
				else {
					lhs_str += lhs_start;
					lhs_len = lhs_end - lhs_start;
				}
				DO_COMPARE(l, lhs_str, lhs_len,
				           b, rhs_str, rhs_len);
			}
			break;

		default: __builtin_unreachable();
		}
	} else {
		struct string_utf *lhs_utf;
		struct string_utf *rhs_utf;
		lhs_utf = lhs->s_data;
		rhs_utf = rhs->s_data;
		ASSERT(lhs_utf);
		ASSERT(rhs_utf);
		ASSERT(lhs_utf->u_width != STRING_WIDTH_1BYTE);
		ASSERT(rhs_utf->u_width != STRING_WIDTH_1BYTE);
		/* Complex string comparison. */
		switch (lhs_utf->u_width) {

			{
				uint16_t *lhs_str;

			CASE_WIDTH_2BYTE:
				lhs_str = (uint16_t *)lhs_utf->u_data[lhs_utf->u_width];
				lhs_len = WSTR_LENGTH(lhs_str);
				if (lhs_end > lhs_len)
					lhs_end = lhs_len;
				if (lhs_start >= lhs_end)
					lhs_len = 0;
				else {
					lhs_str += lhs_start;
					lhs_len = lhs_end - lhs_start;
				}
				switch (rhs_utf->u_width) {

					{
						uint16_t *rhs_str;

					CASE_WIDTH_2BYTE:
						rhs_str = (uint16_t *)rhs_utf->u_data[rhs_utf->u_width];
						rhs_len = WSTR_LENGTH(rhs_str);
						if (rhs_end > rhs_len)
							rhs_end = rhs_len;
						if (rhs_start >= rhs_end)
							rhs_len = 0;
						else {
							rhs_str += rhs_start;
							rhs_len = rhs_end - rhs_start;
						}
						DO_COMPARE(w, lhs_str, lhs_len,
						           w, rhs_str, rhs_len);
					}
					break;

					{
						uint32_t *rhs_str;

					CASE_WIDTH_4BYTE:
						rhs_str = (uint32_t *)rhs_utf->u_data[rhs_utf->u_width];
						rhs_len = WSTR_LENGTH(rhs_str);
						if (rhs_end > rhs_len)
							rhs_end = rhs_len;
						if (rhs_start >= rhs_end)
							rhs_len = 0;
						else {
							rhs_str += rhs_start;
							rhs_len = rhs_end - rhs_start;
						}
						DO_COMPARE(w, lhs_str, lhs_len,
						           l, rhs_str, rhs_len);
					}
					break;

				default: __builtin_unreachable();
				}
			}
			break;

			{
				uint32_t *lhs_str;

			CASE_WIDTH_4BYTE:
				lhs_str = (uint32_t *)lhs_utf->u_data[lhs_utf->u_width];
				lhs_len = WSTR_LENGTH(lhs_str);
				if (lhs_end > lhs_len)
					lhs_end = lhs_len;
				if (lhs_start >= lhs_end)
					lhs_len = 0;
				else {
					lhs_str += lhs_start;
					lhs_len = lhs_end - lhs_start;
				}
				switch (rhs_utf->u_width) {

					{
						uint16_t *rhs_str;

					CASE_WIDTH_2BYTE:
						rhs_str = (uint16_t *)rhs_utf->u_data[rhs_utf->u_width];
						rhs_len = WSTR_LENGTH(rhs_str);
						if (rhs_end > rhs_len)
							rhs_end = rhs_len;
						if (rhs_start >= rhs_end)
							rhs_len = 0;
						else {
							rhs_str += rhs_start;
							rhs_len = rhs_end - rhs_start;
						}
						DO_COMPARE(l, lhs_str, lhs_len,
						           w, rhs_str, rhs_len);
					}
					break;

					{
						uint32_t *rhs_str;

					CASE_WIDTH_4BYTE:
						rhs_str = (uint32_t *)rhs_utf->u_data[rhs_utf->u_width];
						rhs_len = WSTR_LENGTH(rhs_str);
						if (rhs_end > rhs_len)
							rhs_end = rhs_len;
						if (rhs_start >= rhs_end)
							rhs_len = 0;
						else {
							rhs_str += rhs_start;
							rhs_len = rhs_end - rhs_start;
						}
						DO_COMPARE(l, lhs_str, lhs_len,
						           l, rhs_str, rhs_len);
					}
					break;

				default: __builtin_unreachable();
				}
			}
			break;

		default: __builtin_unreachable();
		}
	}
#if 0
	/* If string contents are identical, leave off by comparing their lengths. */
	if (lhs_len == rhs_len)
		return 0;
	if (lhs_len < rhs_len)
		return -1;
	return 1;
#endif
}
#undef DO_COMPARE



PRIVATE DREF DeeObject *DCALL
string_compare(String *__restrict self,
               size_t argc, DeeObject **__restrict argv) {
	int result;
	struct compare_args args;
	if (get_compare_args(&args, argc, argv, "compare"))
		goto err;
	result = compare_strings_ex(self, args.my_start, args.my_end,
	                            (String *)args.other, args.ot_start, args.ot_end);
	return DeeInt_NewInt(result);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
string_casecompare(String *__restrict self,
                   size_t argc, DeeObject **__restrict argv) {
	int result;
	struct compare_args args;
	if (get_compare_args(&args, argc, argv, "compare"))
		goto err;
	result = casecompare_strings_ex(self, args.my_start, args.my_end,
	                                (String *)args.other, args.ot_start, args.ot_end);
	return DeeInt_NewInt(result);
err:
	return NULL;
}


PRIVATE DREF DeeObject *DCALL
string_vercompare(String *__restrict self,
                  size_t argc, DeeObject **__restrict argv) {
	union dcharptr my_str, ot_str;
	size_t my_len, ot_len;
	int32_t result;
	struct compare_args args;
	if (get_compare_args(&args, argc, argv, "vercompare"))
		goto err;
	SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON(DeeString_WIDTH(self),
	                                        DeeString_WIDTH(args.other))) {

	CASE_WIDTH_1BYTE:
		my_str.cp8 = DeeString_As1Byte((DeeObject *)self);
		ot_str.cp8 = DeeString_As1Byte((DeeObject *)args.other);
		my_len     = WSTR_LENGTH(my_str.cp8);
		ot_len     = WSTR_LENGTH(ot_str.cp8);
		if (args.my_end > my_len)
			args.my_end = my_len;
		if (args.my_end <= args.my_start)
			my_len = 0;
		else {
			my_str.cp8 += args.my_start;
			my_len = args.my_end - args.my_start;
		}
		if (args.ot_end > ot_len)
			args.ot_end = ot_len;
		if (args.ot_end <= args.ot_start)
			ot_len = 0;
		else {
			ot_str.cp8 += args.ot_start;
			ot_len = args.ot_end - args.ot_start;
		}
		result = dee_strverscmpb(my_str.cp8, my_len,
		                         ot_str.cp8, ot_len);
		break;

	CASE_WIDTH_2BYTE:
		my_str.cp16 = DeeString_As2Byte((DeeObject *)self);
		if unlikely(!my_str.cp16)
			goto err;
		ot_str.cp16 = DeeString_As2Byte((DeeObject *)args.other);
		if unlikely(!ot_str.cp16)
			goto err;
		my_len = WSTR_LENGTH(my_str.cp16);
		ot_len = WSTR_LENGTH(ot_str.cp16);
		if (args.my_end > my_len)
			args.my_end = my_len;
		if (args.my_end <= args.my_start)
			my_len = 0;
		else {
			my_str.cp16 += args.my_start;
			my_len = args.my_end - args.my_start;
		}
		if (args.ot_end > ot_len)
			args.ot_end = ot_len;
		if (args.ot_end <= args.ot_start)
			ot_len = 0;
		else {
			ot_str.cp16 += args.ot_start;
			ot_len = args.ot_end - args.ot_start;
		}
		result = dee_strverscmpw(my_str.cp16, my_len,
		                         ot_str.cp16, ot_len);
		break;

	CASE_WIDTH_4BYTE:
		my_str.cp32 = DeeString_As4Byte((DeeObject *)self);
		if unlikely(!my_str.cp32)
			goto err;
		ot_str.cp32 = DeeString_As4Byte((DeeObject *)args.other);
		if unlikely(!ot_str.cp32)
			goto err;
		my_len = WSTR_LENGTH(my_str.cp32);
		ot_len = WSTR_LENGTH(ot_str.cp32);
		if (args.my_end > my_len)
			args.my_end = my_len;
		if (args.my_end <= args.my_start)
			my_len = 0;
		else {
			my_str.cp32 += args.my_start;
			my_len = args.my_end - args.my_start;
		}
		if (args.ot_end > ot_len)
			args.ot_end = ot_len;
		if (args.ot_end <= args.ot_start)
			ot_len = 0;
		else {
			ot_str.cp32 += args.ot_start;
			ot_len = args.ot_end - args.ot_start;
		}
		result = dee_strverscmpl(my_str.cp32, my_len,
		                         ot_str.cp32, ot_len);
		break;
	}
	return DeeInt_NewS32(result);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
string_casevercompare(String *__restrict self,
                      size_t argc, DeeObject **__restrict argv) {
	union dcharptr my_str, ot_str;
	size_t my_len, ot_len;
	int32_t result;
	struct compare_args args;
	if (get_compare_args(&args, argc, argv, "casevercompare"))
		goto err;
	SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON(DeeString_WIDTH(self),
	                                        DeeString_WIDTH(args.other))) {

	CASE_WIDTH_1BYTE:
		my_str.cp8 = DeeString_As1Byte((DeeObject *)self);
		ot_str.cp8 = DeeString_As1Byte((DeeObject *)args.other);
		my_len     = WSTR_LENGTH(my_str.cp8);
		ot_len     = WSTR_LENGTH(ot_str.cp8);
		if (args.my_end > my_len)
			args.my_end = my_len;
		if (args.my_end <= args.my_start)
			my_len = 0;
		else {
			my_str.cp8 += args.my_start;
			my_len = args.my_end - args.my_start;
		}
		if (args.ot_end > ot_len)
			args.ot_end = ot_len;
		if (args.ot_end <= args.ot_start)
			ot_len = 0;
		else {
			ot_str.cp8 += args.ot_start;
			ot_len = args.ot_end - args.ot_start;
		}
		result = dee_strcaseverscmpb(my_str.cp8, my_len,
		                             ot_str.cp8, ot_len);
		break;

	CASE_WIDTH_2BYTE:
		my_str.cp16 = DeeString_As2Byte((DeeObject *)self);
		if unlikely(!my_str.cp16)
			goto err;
		ot_str.cp16 = DeeString_As2Byte((DeeObject *)args.other);
		if unlikely(!ot_str.cp16)
			goto err;
		my_len = WSTR_LENGTH(my_str.cp16);
		ot_len = WSTR_LENGTH(ot_str.cp16);
		if (args.my_end > my_len)
			args.my_end = my_len;
		if (args.my_end <= args.my_start)
			my_len = 0;
		else {
			my_str.cp16 += args.my_start;
			my_len = args.my_end - args.my_start;
		}
		if (args.ot_end > ot_len)
			args.ot_end = ot_len;
		if (args.ot_end <= args.ot_start)
			ot_len = 0;
		else {
			ot_str.cp16 += args.ot_start;
			ot_len = args.ot_end - args.ot_start;
		}
		result = dee_strcaseverscmpw(my_str.cp16, my_len,
		                             ot_str.cp16, ot_len);
		break;

	CASE_WIDTH_4BYTE:
		my_str.cp32 = DeeString_As4Byte((DeeObject *)self);
		if unlikely(!my_str.cp32)
			goto err;
		ot_str.cp32 = DeeString_As4Byte((DeeObject *)args.other);
		if unlikely(!ot_str.cp32)
			goto err;
		my_len = WSTR_LENGTH(my_str.cp32);
		ot_len = WSTR_LENGTH(ot_str.cp32);
		if (args.my_end > my_len)
			args.my_end = my_len;
		if (args.my_end <= args.my_start)
			my_len = 0;
		else {
			my_str.cp32 += args.my_start;
			my_len = args.my_end - args.my_start;
		}
		if (args.ot_end > ot_len)
			args.ot_end = ot_len;
		if (args.ot_end <= args.ot_start)
			ot_len = 0;
		else {
			ot_str.cp32 += args.ot_start;
			ot_len = args.ot_end - args.ot_start;
		}
		result = dee_strcaseverscmpl(my_str.cp32, my_len,
		                             ot_str.cp32, ot_len);
		break;
	}
	return DeeInt_NewS32(result);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
string_fuzzycompare(String *__restrict self,
                    size_t argc, DeeObject **__restrict argv) {
	union dcharptr my_str, ot_str;
	size_t my_len, ot_len;
	dssize_t result;
	struct compare_args args;
	if (get_compare_args(&args, argc, argv, "fuzzycompare"))
		goto err;
	SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON(DeeString_WIDTH(self),
	                                        DeeString_WIDTH(args.other))) {

	CASE_WIDTH_1BYTE:
		my_str.cp8 = DeeString_As1Byte((DeeObject *)self);
		ot_str.cp8 = DeeString_As1Byte((DeeObject *)args.other);
		my_len     = WSTR_LENGTH(my_str.cp8);
		ot_len     = WSTR_LENGTH(ot_str.cp8);
		if (args.my_end > my_len)
			args.my_end = my_len;
		if (args.my_end <= args.my_start)
			my_len = 0;
		else {
			my_str.cp8 += args.my_start;
			my_len = args.my_end - args.my_start;
		}
		if (args.ot_end > ot_len)
			args.ot_end = ot_len;
		if (args.ot_end <= args.ot_start)
			ot_len = 0;
		else {
			ot_str.cp8 += args.ot_start;
			ot_len = args.ot_end - args.ot_start;
		}
		result = fuzzy_compareb(my_str.cp8, my_len,
		                        ot_str.cp8, ot_len);
		break;

	CASE_WIDTH_2BYTE:
		my_str.cp16 = DeeString_As2Byte((DeeObject *)self);
		if unlikely(!my_str.cp16)
			goto err;
		ot_str.cp16 = DeeString_As2Byte((DeeObject *)args.other);
		if unlikely(!ot_str.cp16)
			goto err;
		my_len = WSTR_LENGTH(my_str.cp16);
		ot_len = WSTR_LENGTH(ot_str.cp16);
		if (args.my_end > my_len)
			args.my_end = my_len;
		if (args.my_end <= args.my_start)
			my_len = 0;
		else {
			my_str.cp16 += args.my_start;
			my_len = args.my_end - args.my_start;
		}
		if (args.ot_end > ot_len)
			args.ot_end = ot_len;
		if (args.ot_end <= args.ot_start)
			ot_len = 0;
		else {
			ot_str.cp16 += args.ot_start;
			ot_len = args.ot_end - args.ot_start;
		}
		result = fuzzy_comparew(my_str.cp16, my_len,
		                        ot_str.cp16, ot_len);
		break;

	CASE_WIDTH_4BYTE:
		my_str.cp32 = DeeString_As4Byte((DeeObject *)self);
		if unlikely(!my_str.cp32)
			goto err;
		ot_str.cp32 = DeeString_As4Byte((DeeObject *)args.other);
		if unlikely(!ot_str.cp32)
			goto err;
		my_len = WSTR_LENGTH(my_str.cp32);
		ot_len = WSTR_LENGTH(ot_str.cp32);
		if (args.my_end > my_len)
			args.my_end = my_len;
		if (args.my_end <= args.my_start)
			my_len = 0;
		else {
			my_str.cp32 += args.my_start;
			my_len = args.my_end - args.my_start;
		}
		if (args.ot_end > ot_len)
			args.ot_end = ot_len;
		if (args.ot_end <= args.ot_start)
			ot_len = 0;
		else {
			ot_str.cp32 += args.ot_start;
			ot_len = args.ot_end - args.ot_start;
		}
		result = fuzzy_comparel(my_str.cp32, my_len,
		                        ot_str.cp32, ot_len);
		break;
	}
	if unlikely(result == (dssize_t)-1)
		goto err;
	return DeeInt_NewSize((size_t)result);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
string_casefuzzycompare(String *__restrict self,
                        size_t argc, DeeObject **__restrict argv) {
	union dcharptr my_str, ot_str;
	size_t my_len, ot_len;
	dssize_t result;
	struct compare_args args;
	if (get_compare_args(&args, argc, argv, "casefuzzycompare"))
		goto err;
	SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON(DeeString_WIDTH(self),
	                                        DeeString_WIDTH(args.other))) {

	CASE_WIDTH_1BYTE:
		my_str.cp8 = DeeString_As1Byte((DeeObject *)self);
		ot_str.cp8 = DeeString_As1Byte((DeeObject *)args.other);
		my_len     = WSTR_LENGTH(my_str.cp8);
		ot_len     = WSTR_LENGTH(ot_str.cp8);
		if (args.my_end > my_len)
			args.my_end = my_len;
		if (args.my_end <= args.my_start)
			my_len = 0;
		else {
			my_str.cp8 += args.my_start;
			my_len = args.my_end - args.my_start;
		}
		if (args.ot_end > ot_len)
			args.ot_end = ot_len;
		if (args.ot_end <= args.ot_start)
			ot_len = 0;
		else {
			ot_str.cp8 += args.ot_start;
			ot_len = args.ot_end - args.ot_start;
		}
		result = fuzzy_casecompareb(my_str.cp8, my_len,
		                            ot_str.cp8, ot_len);
		break;

	CASE_WIDTH_2BYTE:
		my_str.cp16 = DeeString_As2Byte((DeeObject *)self);
		if unlikely(!my_str.cp16)
			goto err;
		ot_str.cp16 = DeeString_As2Byte((DeeObject *)args.other);
		if unlikely(!ot_str.cp16)
			goto err;
		my_len = WSTR_LENGTH(my_str.cp16);
		ot_len = WSTR_LENGTH(ot_str.cp16);
		if (args.my_end > my_len)
			args.my_end = my_len;
		if (args.my_end <= args.my_start)
			my_len = 0;
		else {
			my_str.cp16 += args.my_start;
			my_len = args.my_end - args.my_start;
		}
		if (args.ot_end > ot_len)
			args.ot_end = ot_len;
		if (args.ot_end <= args.ot_start)
			ot_len = 0;
		else {
			ot_str.cp16 += args.ot_start;
			ot_len = args.ot_end - args.ot_start;
		}
		result = fuzzy_casecomparew(my_str.cp16, my_len,
		                            ot_str.cp16, ot_len);
		break;

	CASE_WIDTH_4BYTE:
		my_str.cp32 = DeeString_As4Byte((DeeObject *)self);
		if unlikely(!my_str.cp32)
			goto err;
		ot_str.cp32 = DeeString_As4Byte((DeeObject *)args.other);
		if unlikely(!ot_str.cp32)
			goto err;
		my_len = WSTR_LENGTH(my_str.cp32);
		ot_len = WSTR_LENGTH(ot_str.cp32);
		if (args.my_end > my_len)
			args.my_end = my_len;
		if (args.my_end <= args.my_start)
			my_len = 0;
		else {
			my_str.cp32 += args.my_start;
			my_len = args.my_end - args.my_start;
		}
		if (args.ot_end > ot_len)
			args.ot_end = ot_len;
		if (args.ot_end <= args.ot_start)
			ot_len = 0;
		else {
			ot_str.cp32 += args.ot_start;
			ot_len = args.ot_end - args.ot_start;
		}
		result = fuzzy_casecomparel(my_str.cp32, my_len,
		                            ot_str.cp32, ot_len);
		break;
	}
	if unlikely(result == (dssize_t)-1)
		goto err;
	return DeeInt_NewSize((size_t)result);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
string_common(String *__restrict self,
              size_t argc, DeeObject **__restrict argv) {
	union dcharptr my_str, ot_str;
	size_t my_len, ot_len, result = 0;
	struct compare_args args;
	if (get_compare_args(&args, argc, argv, "common"))
		goto err;
	SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON(DeeString_WIDTH(self),
	                                        DeeString_WIDTH(args.other))) {

	CASE_WIDTH_1BYTE:
		my_str.cp8 = DeeString_As1Byte((DeeObject *)self);
		ot_str.cp8 = DeeString_As1Byte((DeeObject *)args.other);
		my_len     = WSTR_LENGTH(my_str.cp8);
		ot_len     = WSTR_LENGTH(ot_str.cp8);
		if (args.my_end > my_len)
			args.my_end = my_len;
		if (args.my_end <= args.my_start)
			my_len = 0;
		else {
			my_str.cp8 += args.my_start;
			my_len = args.my_end - args.my_start;
		}
		if (args.ot_end > ot_len)
			args.ot_end = ot_len;
		if (args.ot_end <= args.ot_start)
			ot_len = 0;
		else {
			ot_str.cp8 += args.ot_start;
			ot_len = args.ot_end - args.ot_start;
		}
		while (ot_len && my_len) {
			uint8_t a = *my_str.cp8;
			uint8_t b = *ot_str.cp8;
			if (a != b)
				break;
			++my_str.cp8;
			++ot_str.cp8;
			--my_len;
			--ot_len;
			++result;
		}
		break;

	CASE_WIDTH_2BYTE:
		my_str.cp16 = DeeString_As2Byte((DeeObject *)self);
		if unlikely(!my_str.cp16)
			goto err;
		ot_str.cp16 = DeeString_As2Byte((DeeObject *)args.other);
		if unlikely(!ot_str.cp16)
			goto err;
		my_len = WSTR_LENGTH(my_str.cp16);
		ot_len = WSTR_LENGTH(ot_str.cp16);
		if (args.my_end > my_len)
			args.my_end = my_len;
		if (args.my_end <= args.my_start)
			my_len = 0;
		else {
			my_str.cp16 += args.my_start;
			my_len = args.my_end - args.my_start;
		}
		if (args.ot_end > ot_len)
			args.ot_end = ot_len;
		if (args.ot_end <= args.ot_start)
			ot_len = 0;
		else {
			ot_str.cp16 += args.ot_start;
			ot_len = args.ot_end - args.ot_start;
		}
		while (ot_len && my_len) {
			uint16_t a = *my_str.cp16;
			uint16_t b = *ot_str.cp16;
			if (a != b)
				break;
			++my_str.cp16;
			++ot_str.cp16;
			--my_len;
			--ot_len;
			++result;
		}
		break;

	CASE_WIDTH_4BYTE:
		my_str.cp32 = DeeString_As4Byte((DeeObject *)self);
		if unlikely(!my_str.cp32)
			goto err;
		ot_str.cp32 = DeeString_As4Byte((DeeObject *)args.other);
		if unlikely(!ot_str.cp32)
			goto err;
		my_len = WSTR_LENGTH(my_str.cp32);
		ot_len = WSTR_LENGTH(ot_str.cp32);
		if (args.my_end > my_len)
			args.my_end = my_len;
		if (args.my_end <= args.my_start)
			my_len = 0;
		else {
			my_str.cp32 += args.my_start;
			my_len = args.my_end - args.my_start;
		}
		if (args.ot_end > ot_len)
			args.ot_end = ot_len;
		if (args.ot_end <= args.ot_start)
			ot_len = 0;
		else {
			ot_str.cp32 += args.ot_start;
			ot_len = args.ot_end - args.ot_start;
		}
		while (ot_len && my_len) {
			uint32_t a = *my_str.cp32;
			uint32_t b = *ot_str.cp32;
			if (a != b)
				break;
			++my_str.cp32;
			++ot_str.cp32;
			--my_len;
			--ot_len;
			++result;
		}
		break;
	}
	return DeeInt_NewSize(result);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
string_rcommon(String *__restrict self,
               size_t argc, DeeObject **__restrict argv) {
	union dcharptr my_str, ot_str;
	size_t my_len, ot_len, result = 0;
	struct compare_args args;
	if (get_compare_args(&args, argc, argv, "rcommon"))
		goto err;
	SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON(DeeString_WIDTH(self),
	                                        DeeString_WIDTH(args.other))) {

	CASE_WIDTH_1BYTE:
		my_str.cp8 = DeeString_As1Byte((DeeObject *)self);
		ot_str.cp8 = DeeString_As1Byte((DeeObject *)args.other);
		my_len     = WSTR_LENGTH(my_str.cp8);
		ot_len     = WSTR_LENGTH(ot_str.cp8);
		if (args.my_end > my_len)
			args.my_end = my_len;
		if (args.my_end <= args.my_start)
			my_len = 0;
		else {
			my_str.cp8 += args.my_start;
			my_len = args.my_end - args.my_start;
		}
		if (args.ot_end > ot_len)
			args.ot_end = ot_len;
		if (args.ot_end <= args.ot_start)
			ot_len = 0;
		else {
			ot_str.cp8 += args.ot_start;
			ot_len = args.ot_end - args.ot_start;
		}
		while (ot_len && my_len) {
			uint8_t a;
			uint8_t b;
			--my_len;
			--ot_len;
			a = my_str.cp8[my_len];
			b = ot_str.cp8[ot_len];
			if (a != b)
				break;
			++my_str.cp8;
			++ot_str.cp8;
			++result;
		}
		break;

	CASE_WIDTH_2BYTE:
		my_str.cp16 = DeeString_As2Byte((DeeObject *)self);
		if unlikely(!my_str.cp16)
			goto err;
		ot_str.cp16 = DeeString_As2Byte((DeeObject *)args.other);
		if unlikely(!ot_str.cp16)
			goto err;
		my_len = WSTR_LENGTH(my_str.cp16);
		ot_len = WSTR_LENGTH(ot_str.cp16);
		if (args.my_end > my_len)
			args.my_end = my_len;
		if (args.my_end <= args.my_start)
			my_len = 0;
		else {
			my_str.cp16 += args.my_start;
			my_len = args.my_end - args.my_start;
		}
		if (args.ot_end > ot_len)
			args.ot_end = ot_len;
		if (args.ot_end <= args.ot_start)
			ot_len = 0;
		else {
			ot_str.cp16 += args.ot_start;
			ot_len = args.ot_end - args.ot_start;
		}
		while (ot_len && my_len) {
			uint16_t a;
			uint16_t b;
			--my_len;
			--ot_len;
			a = my_str.cp16[my_len];
			b = ot_str.cp16[ot_len];
			if (a != b)
				break;
			++my_str.cp16;
			++ot_str.cp16;
			++result;
		}
		break;

	CASE_WIDTH_4BYTE:
		my_str.cp32 = DeeString_As4Byte((DeeObject *)self);
		if unlikely(!my_str.cp32)
			goto err;
		ot_str.cp32 = DeeString_As4Byte((DeeObject *)args.other);
		if unlikely(!ot_str.cp32)
			goto err;
		my_len = WSTR_LENGTH(my_str.cp32);
		ot_len = WSTR_LENGTH(ot_str.cp32);
		if (args.my_end > my_len)
			args.my_end = my_len;
		if (args.my_end <= args.my_start)
			my_len = 0;
		else {
			my_str.cp32 += args.my_start;
			my_len = args.my_end - args.my_start;
		}
		if (args.ot_end > ot_len)
			args.ot_end = ot_len;
		if (args.ot_end <= args.ot_start)
			ot_len = 0;
		else {
			ot_str.cp32 += args.ot_start;
			ot_len = args.ot_end - args.ot_start;
		}
		while (ot_len && my_len) {
			uint32_t a;
			uint32_t b;
			--my_len;
			--ot_len;
			a = my_str.cp32[my_len];
			b = ot_str.cp32[ot_len];
			if (a != b)
				break;
			++my_str.cp32;
			++ot_str.cp32;
			++result;
		}
		break;
	}
	return DeeInt_NewSize(result);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
string_casecommon(String *__restrict self,
                  size_t argc, DeeObject **__restrict argv) {
	union dcharptr my_str, ot_str;
	size_t my_len, ot_len, result = 0;
	struct compare_args args;
	union {
		struct unicode_foldreaderb b;
		struct unicode_foldreaderw w;
		struct unicode_foldreaderl l;
	} my_reader, ot_reader;
	if (get_compare_args(&args, argc, argv, "casecommon"))
		goto err;
	SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON(DeeString_WIDTH(self),
	                                        DeeString_WIDTH(args.other))) {

	CASE_WIDTH_1BYTE:
		my_str.cp8 = DeeString_As1Byte((DeeObject *)self);
		ot_str.cp8 = DeeString_As1Byte((DeeObject *)args.other);
		my_len     = WSTR_LENGTH(my_str.cp8);
		ot_len     = WSTR_LENGTH(ot_str.cp8);
		if (args.my_end > my_len)
			args.my_end = my_len;
		if (args.my_end <= args.my_start)
			my_len = 0;
		else {
			my_str.cp8 += args.my_start;
			my_len = args.my_end - args.my_start;
		}
		if (args.ot_end > ot_len)
			args.ot_end = ot_len;
		if (args.ot_end <= args.ot_start)
			ot_len = 0;
		else {
			ot_str.cp8 += args.ot_start;
			ot_len = args.ot_end - args.ot_start;
		}
		unicode_foldreader_init(my_reader.b, my_str.cp8, my_len);
		unicode_foldreader_init(ot_reader.b, ot_str.cp8, ot_len);
		while (!unicode_foldreader_empty(my_reader.b) &&
		       !unicode_foldreader_empty(ot_reader.b)) {
			if (unicode_foldreader_getc(my_reader.b) !=
			    unicode_foldreader_getc(ot_reader.b))
				break;
			++result;
		}
		break;

	CASE_WIDTH_2BYTE:
		my_str.cp16 = DeeString_As2Byte((DeeObject *)self);
		if unlikely(!my_str.cp16)
			goto err;
		ot_str.cp16 = DeeString_As2Byte((DeeObject *)args.other);
		if unlikely(!ot_str.cp16)
			goto err;
		my_len = WSTR_LENGTH(my_str.cp16);
		ot_len = WSTR_LENGTH(ot_str.cp16);
		if (args.my_end > my_len)
			args.my_end = my_len;
		if (args.my_end <= args.my_start)
			my_len = 0;
		else {
			my_str.cp16 += args.my_start;
			my_len = args.my_end - args.my_start;
		}
		if (args.ot_end > ot_len)
			args.ot_end = ot_len;
		if (args.ot_end <= args.ot_start)
			ot_len = 0;
		else {
			ot_str.cp16 += args.ot_start;
			ot_len = args.ot_end - args.ot_start;
		}
		unicode_foldreader_init(my_reader.w, my_str.cp16, my_len);
		unicode_foldreader_init(ot_reader.w, ot_str.cp16, ot_len);
		while (!unicode_foldreader_empty(my_reader.w) &&
		       !unicode_foldreader_empty(ot_reader.w)) {
			if (unicode_foldreader_getc(my_reader.w) !=
			    unicode_foldreader_getc(ot_reader.w))
				break;
			++result;
		}
		break;

	CASE_WIDTH_4BYTE:
		my_str.cp32 = DeeString_As4Byte((DeeObject *)self);
		if unlikely(!my_str.cp32)
			goto err;
		ot_str.cp32 = DeeString_As4Byte((DeeObject *)args.other);
		if unlikely(!ot_str.cp32)
			goto err;
		my_len = WSTR_LENGTH(my_str.cp32);
		ot_len = WSTR_LENGTH(ot_str.cp32);
		if (args.my_end > my_len)
			args.my_end = my_len;
		if (args.my_end <= args.my_start)
			my_len = 0;
		else {
			my_str.cp32 += args.my_start;
			my_len = args.my_end - args.my_start;
		}
		if (args.ot_end > ot_len)
			args.ot_end = ot_len;
		if (args.ot_end <= args.ot_start)
			ot_len = 0;
		else {
			ot_str.cp32 += args.ot_start;
			ot_len = args.ot_end - args.ot_start;
		}
		unicode_foldreader_init(my_reader.l, my_str.cp32, my_len);
		unicode_foldreader_init(ot_reader.l, ot_str.cp32, ot_len);
		while (!unicode_foldreader_empty(my_reader.l) &&
		       !unicode_foldreader_empty(ot_reader.l)) {
			if (unicode_foldreader_getc(my_reader.l) !=
			    unicode_foldreader_getc(ot_reader.l))
				break;
			++result;
		}
		break;
	}
	return DeeInt_NewSize(result);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
string_casercommon(String *__restrict self,
                   size_t argc, DeeObject **__restrict argv) {
	union dcharptr my_str, ot_str;
	size_t my_len, ot_len, result = 0;
	struct compare_args args;
	union {
		struct unicode_foldreaderb b;
		struct unicode_foldreaderw w;
		struct unicode_foldreaderl l;
	} my_reader, ot_reader;
	if (get_compare_args(&args, argc, argv, "rcommon"))
		goto err;
	SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON(DeeString_WIDTH(self),
	                                        DeeString_WIDTH(args.other))) {

	CASE_WIDTH_1BYTE:
		my_str.cp8 = DeeString_As1Byte((DeeObject *)self);
		ot_str.cp8 = DeeString_As1Byte((DeeObject *)args.other);
		my_len     = WSTR_LENGTH(my_str.cp8);
		ot_len     = WSTR_LENGTH(ot_str.cp8);
		if (args.my_end > my_len)
			args.my_end = my_len;
		if (args.my_end <= args.my_start)
			my_len = 0;
		else {
			my_str.cp8 += args.my_start;
			my_len = args.my_end - args.my_start;
		}
		if (args.ot_end > ot_len)
			args.ot_end = ot_len;
		if (args.ot_end <= args.ot_start)
			ot_len = 0;
		else {
			ot_str.cp8 += args.ot_start;
			ot_len = args.ot_end - args.ot_start;
		}
		unicode_foldreader_init(my_reader.b, my_str.cp8, my_len);
		unicode_foldreader_init(ot_reader.b, ot_str.cp8, ot_len);
		while (!unicode_foldreader_empty(my_reader.b) &&
		       !unicode_foldreader_empty(ot_reader.b)) {
			if (unicode_foldreader_getc_back(my_reader.b) !=
			    unicode_foldreader_getc_back(ot_reader.b))
				break;
			++result;
		}
		break;

	CASE_WIDTH_2BYTE:
		my_str.cp16 = DeeString_As2Byte((DeeObject *)self);
		if unlikely(!my_str.cp16)
			goto err;
		ot_str.cp16 = DeeString_As2Byte((DeeObject *)args.other);
		if unlikely(!ot_str.cp16)
			goto err;
		my_len = WSTR_LENGTH(my_str.cp16);
		ot_len = WSTR_LENGTH(ot_str.cp16);
		if (args.my_end > my_len)
			args.my_end = my_len;
		if (args.my_end <= args.my_start)
			my_len = 0;
		else {
			my_str.cp16 += args.my_start;
			my_len = args.my_end - args.my_start;
		}
		if (args.ot_end > ot_len)
			args.ot_end = ot_len;
		if (args.ot_end <= args.ot_start)
			ot_len = 0;
		else {
			ot_str.cp16 += args.ot_start;
			ot_len = args.ot_end - args.ot_start;
		}
		unicode_foldreader_init(my_reader.w, my_str.cp16, my_len);
		unicode_foldreader_init(ot_reader.w, ot_str.cp16, ot_len);
		while (!unicode_foldreader_empty(my_reader.w) &&
		       !unicode_foldreader_empty(ot_reader.w)) {
			if (unicode_foldreader_getc_back(my_reader.w) !=
			    unicode_foldreader_getc_back(ot_reader.w))
				break;
			++result;
		}
		break;

	CASE_WIDTH_4BYTE:
		my_str.cp32 = DeeString_As4Byte((DeeObject *)self);
		if unlikely(!my_str.cp32)
			goto err;
		ot_str.cp32 = DeeString_As4Byte((DeeObject *)args.other);
		if unlikely(!ot_str.cp32)
			goto err;
		my_len = WSTR_LENGTH(my_str.cp32);
		ot_len = WSTR_LENGTH(ot_str.cp32);
		if (args.my_end > my_len)
			args.my_end = my_len;
		if (args.my_end <= args.my_start)
			my_len = 0;
		else {
			my_str.cp32 += args.my_start;
			my_len = args.my_end - args.my_start;
		}
		if (args.ot_end > ot_len)
			args.ot_end = ot_len;
		if (args.ot_end <= args.ot_start)
			ot_len = 0;
		else {
			ot_str.cp32 += args.ot_start;
			ot_len = args.ot_end - args.ot_start;
		}
		unicode_foldreader_init(my_reader.l, my_str.cp32, my_len);
		unicode_foldreader_init(ot_reader.l, ot_str.cp32, ot_len);
		while (!unicode_foldreader_empty(my_reader.l) &&
		       !unicode_foldreader_empty(ot_reader.l)) {
			if (unicode_foldreader_getc_back(my_reader.l) !=
			    unicode_foldreader_getc_back(ot_reader.l))
				break;
			++result;
		}
		break;
	}
	return DeeInt_NewSize(result);
err:
	return NULL;
}


PRIVATE DREF DeeObject *DCALL
string_wildcompare(String *__restrict self,
                   size_t argc, DeeObject **__restrict argv) {
	union dcharptr my_str, ot_str;
	size_t my_len, ot_len;
	int64_t result;
	struct compare_args args;
	if (get_compare_args(&args, argc, argv, "wildcompare"))
		goto err;
	SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON(DeeString_WIDTH(self),
	                                        DeeString_WIDTH(args.other))) {

	CASE_WIDTH_1BYTE:
		my_str.cp8 = DeeString_As1Byte((DeeObject *)self);
		ot_str.cp8 = DeeString_As1Byte((DeeObject *)args.other);
		my_len     = WSTR_LENGTH(my_str.cp8);
		ot_len     = WSTR_LENGTH(ot_str.cp8);
		if (args.my_end > my_len)
			args.my_end = my_len;
		if (args.my_end <= args.my_start)
			my_len = 0;
		else {
			my_str.cp8 += args.my_start;
			my_len = args.my_end - args.my_start;
		}
		if (args.ot_end > ot_len)
			args.ot_end = ot_len;
		if (args.ot_end <= args.ot_start)
			ot_len = 0;
		else {
			ot_str.cp8 += args.ot_start;
			ot_len = args.ot_end - args.ot_start;
		}
		result = wildcompareb(my_str.cp8, my_len,
		                      ot_str.cp8, ot_len);
		break;

	CASE_WIDTH_2BYTE:
		my_str.cp16 = DeeString_As2Byte((DeeObject *)self);
		if unlikely(!my_str.cp16)
			goto err;
		ot_str.cp16 = DeeString_As2Byte((DeeObject *)args.other);
		if unlikely(!ot_str.cp16)
			goto err;
		my_len = WSTR_LENGTH(my_str.cp16);
		ot_len = WSTR_LENGTH(ot_str.cp16);
		if (args.my_end > my_len)
			args.my_end = my_len;
		if (args.my_end <= args.my_start)
			my_len = 0;
		else {
			my_str.cp16 += args.my_start;
			my_len = args.my_end - args.my_start;
		}
		if (args.ot_end > ot_len)
			args.ot_end = ot_len;
		if (args.ot_end <= args.ot_start)
			ot_len = 0;
		else {
			ot_str.cp16 += args.ot_start;
			ot_len = args.ot_end - args.ot_start;
		}
		result = wildcomparew(my_str.cp16, my_len,
		                      ot_str.cp16, ot_len);
		break;

	CASE_WIDTH_4BYTE:
		my_str.cp32 = DeeString_As4Byte((DeeObject *)self);
		if unlikely(!my_str.cp32)
			goto err;
		ot_str.cp32 = DeeString_As4Byte((DeeObject *)args.other);
		if unlikely(!ot_str.cp32)
			goto err;
		my_len = WSTR_LENGTH(my_str.cp32);
		ot_len = WSTR_LENGTH(ot_str.cp32);
		if (args.my_end > my_len)
			args.my_end = my_len;
		if (args.my_end <= args.my_start)
			my_len = 0;
		else {
			my_str.cp32 += args.my_start;
			my_len = args.my_end - args.my_start;
		}
		if (args.ot_end > ot_len)
			args.ot_end = ot_len;
		if (args.ot_end <= args.ot_start)
			ot_len = 0;
		else {
			ot_str.cp32 += args.ot_start;
			ot_len = args.ot_end - args.ot_start;
		}
		result = wildcomparel(my_str.cp32, my_len,
		                      ot_str.cp32, ot_len);
		break;
	}
	return DeeInt_NewS64(result);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
string_wmatch(String *__restrict self,
              size_t argc, DeeObject **__restrict argv) {
	union dcharptr my_str, ot_str;
	size_t my_len, ot_len;
	bool result;
	struct compare_args args;
	if (get_compare_args(&args, argc, argv, "wmatch"))
		goto err;
	SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON(DeeString_WIDTH(self),
	                                        DeeString_WIDTH(args.other))) {

	CASE_WIDTH_1BYTE:
		my_str.cp8 = DeeString_As1Byte((DeeObject *)self);
		ot_str.cp8 = DeeString_As1Byte((DeeObject *)args.other);
		my_len     = WSTR_LENGTH(my_str.cp8);
		ot_len     = WSTR_LENGTH(ot_str.cp8);
		if (args.my_end > my_len)
			args.my_end = my_len;
		if (args.my_end <= args.my_start)
			my_len = 0;
		else {
			my_str.cp8 += args.my_start;
			my_len = args.my_end - args.my_start;
		}
		if (args.ot_end > ot_len)
			args.ot_end = ot_len;
		if (args.ot_end <= args.ot_start)
			ot_len = 0;
		else {
			ot_str.cp8 += args.ot_start;
			ot_len = args.ot_end - args.ot_start;
		}
		result = wildcompareb(my_str.cp8, my_len,
		                      ot_str.cp8, ot_len) == 0;
		break;

	CASE_WIDTH_2BYTE:
		my_str.cp16 = DeeString_As2Byte((DeeObject *)self);
		if unlikely(!my_str.cp16)
			goto err;
		ot_str.cp16 = DeeString_As2Byte((DeeObject *)args.other);
		if unlikely(!ot_str.cp16)
			goto err;
		my_len = WSTR_LENGTH(my_str.cp16);
		ot_len = WSTR_LENGTH(ot_str.cp16);
		if (args.my_end > my_len)
			args.my_end = my_len;
		if (args.my_end <= args.my_start)
			my_len = 0;
		else {
			my_str.cp16 += args.my_start;
			my_len = args.my_end - args.my_start;
		}
		if (args.ot_end > ot_len)
			args.ot_end = ot_len;
		if (args.ot_end <= args.ot_start)
			ot_len = 0;
		else {
			ot_str.cp16 += args.ot_start;
			ot_len = args.ot_end - args.ot_start;
		}
		result = wildcomparew(my_str.cp16, my_len,
		                      ot_str.cp16, ot_len) == 0;
		break;

	CASE_WIDTH_4BYTE:
		my_str.cp32 = DeeString_As4Byte((DeeObject *)self);
		if unlikely(!my_str.cp32)
			goto err;
		ot_str.cp32 = DeeString_As4Byte((DeeObject *)args.other);
		if unlikely(!ot_str.cp32)
			goto err;
		my_len = WSTR_LENGTH(my_str.cp32);
		ot_len = WSTR_LENGTH(ot_str.cp32);
		if (args.my_end > my_len)
			args.my_end = my_len;
		if (args.my_end <= args.my_start)
			my_len = 0;
		else {
			my_str.cp32 += args.my_start;
			my_len = args.my_end - args.my_start;
		}
		if (args.ot_end > ot_len)
			args.ot_end = ot_len;
		if (args.ot_end <= args.ot_start)
			ot_len = 0;
		else {
			ot_str.cp32 += args.ot_start;
			ot_len = args.ot_end - args.ot_start;
		}
		result = wildcomparel(my_str.cp32, my_len,
		                      ot_str.cp32, ot_len) == 0;
		break;
	}
	return_bool_(result);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
string_casewildcompare(String *__restrict self,
                       size_t argc, DeeObject **__restrict argv) {
	union dcharptr my_str, ot_str;
	size_t my_len, ot_len;
	int result;
	struct compare_args args;
	union {
		struct unicode_foldreaderb b;
		struct unicode_foldreaderw w;
		struct unicode_foldreaderl l;
	} my_reader, ot_reader;
	if (get_compare_args(&args, argc, argv, "casewildcompare"))
		goto err;
	SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON(DeeString_WIDTH(self),
	                                        DeeString_WIDTH(args.other))) {

	CASE_WIDTH_1BYTE:
		my_str.cp8 = DeeString_As1Byte((DeeObject *)self);
		ot_str.cp8 = DeeString_As1Byte((DeeObject *)args.other);
		my_len     = WSTR_LENGTH(my_str.cp8);
		ot_len     = WSTR_LENGTH(ot_str.cp8);
		if (args.my_end > my_len)
			args.my_end = my_len;
		if (args.my_end <= args.my_start)
			my_len = 0;
		else {
			my_str.cp8 += args.my_start;
			my_len = args.my_end - args.my_start;
		}
		if (args.ot_end > ot_len)
			args.ot_end = ot_len;
		if (args.ot_end <= args.ot_start)
			ot_len = 0;
		else {
			ot_str.cp8 += args.ot_start;
			ot_len = args.ot_end - args.ot_start;
		}
		unicode_foldreader_init(my_reader.b, my_str.cp8, my_len);
		unicode_foldreader_init(ot_reader.b, ot_str.cp8, ot_len);
		result = wildcasecompareb(&my_reader.b, &ot_reader.b);
		break;

	CASE_WIDTH_2BYTE:
		my_str.cp16 = DeeString_As2Byte((DeeObject *)self);
		if unlikely(!my_str.cp16)
			goto err;
		ot_str.cp16 = DeeString_As2Byte((DeeObject *)args.other);
		if unlikely(!ot_str.cp16)
			goto err;
		my_len = WSTR_LENGTH(my_str.cp16);
		ot_len = WSTR_LENGTH(ot_str.cp16);
		if (args.my_end > my_len)
			args.my_end = my_len;
		if (args.my_end <= args.my_start)
			my_len = 0;
		else {
			my_str.cp16 += args.my_start;
			my_len = args.my_end - args.my_start;
		}
		if (args.ot_end > ot_len)
			args.ot_end = ot_len;
		if (args.ot_end <= args.ot_start)
			ot_len = 0;
		else {
			ot_str.cp16 += args.ot_start;
			ot_len = args.ot_end - args.ot_start;
		}
		unicode_foldreader_init(my_reader.w, my_str.cp16, my_len);
		unicode_foldreader_init(ot_reader.w, ot_str.cp16, ot_len);
		result = wildcasecomparew(&my_reader.w, &ot_reader.w);
		break;

	CASE_WIDTH_4BYTE:
		my_str.cp32 = DeeString_As4Byte((DeeObject *)self);
		if unlikely(!my_str.cp32)
			goto err;
		ot_str.cp32 = DeeString_As4Byte((DeeObject *)args.other);
		if unlikely(!ot_str.cp32)
			goto err;
		my_len = WSTR_LENGTH(my_str.cp32);
		ot_len = WSTR_LENGTH(ot_str.cp32);
		if (args.my_end > my_len)
			args.my_end = my_len;
		if (args.my_end <= args.my_start)
			my_len = 0;
		else {
			my_str.cp32 += args.my_start;
			my_len = args.my_end - args.my_start;
		}
		if (args.ot_end > ot_len)
			args.ot_end = ot_len;
		if (args.ot_end <= args.ot_start)
			ot_len = 0;
		else {
			ot_str.cp32 += args.ot_start;
			ot_len = args.ot_end - args.ot_start;
		}
		unicode_foldreader_init(my_reader.l, my_str.cp32, my_len);
		unicode_foldreader_init(ot_reader.l, ot_str.cp32, ot_len);
		result = wildcasecomparel(&my_reader.l, &ot_reader.l);
		break;
	}
	return DeeInt_NewS64(result);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
string_casewmatch(String *__restrict self,
                  size_t argc, DeeObject **__restrict argv) {
	union dcharptr my_str, ot_str;
	size_t my_len, ot_len;
	int result;
	struct compare_args args;
	union {
		struct unicode_foldreaderb b;
		struct unicode_foldreaderw w;
		struct unicode_foldreaderl l;
	} my_reader, ot_reader;
	if (get_compare_args(&args, argc, argv, "casewmatch"))
		goto err;
	SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON(DeeString_WIDTH(self),
	                                        DeeString_WIDTH(args.other))) {

	CASE_WIDTH_1BYTE:
		my_str.cp8 = DeeString_As1Byte((DeeObject *)self);
		ot_str.cp8 = DeeString_As1Byte((DeeObject *)args.other);
		my_len     = WSTR_LENGTH(my_str.cp8);
		ot_len     = WSTR_LENGTH(ot_str.cp8);
		if (args.my_end > my_len)
			args.my_end = my_len;
		if (args.my_end <= args.my_start)
			my_len = 0;
		else {
			my_str.cp8 += args.my_start;
			my_len = args.my_end - args.my_start;
		}
		if (args.ot_end > ot_len)
			args.ot_end = ot_len;
		if (args.ot_end <= args.ot_start)
			ot_len = 0;
		else {
			ot_str.cp8 += args.ot_start;
			ot_len = args.ot_end - args.ot_start;
		}
		unicode_foldreader_init(my_reader.b, my_str.cp8, my_len);
		unicode_foldreader_init(ot_reader.b, ot_str.cp8, ot_len);
		result = wildcasecompareb(&my_reader.b, &ot_reader.b);
		break;

	CASE_WIDTH_2BYTE:
		my_str.cp16 = DeeString_As2Byte((DeeObject *)self);
		if unlikely(!my_str.cp16)
			goto err;
		ot_str.cp16 = DeeString_As2Byte((DeeObject *)args.other);
		if unlikely(!ot_str.cp16)
			goto err;
		my_len = WSTR_LENGTH(my_str.cp16);
		ot_len = WSTR_LENGTH(ot_str.cp16);
		if (args.my_end > my_len)
			args.my_end = my_len;
		if (args.my_end <= args.my_start)
			my_len = 0;
		else {
			my_str.cp16 += args.my_start;
			my_len = args.my_end - args.my_start;
		}
		if (args.ot_end > ot_len)
			args.ot_end = ot_len;
		if (args.ot_end <= args.ot_start)
			ot_len = 0;
		else {
			ot_str.cp16 += args.ot_start;
			ot_len = args.ot_end - args.ot_start;
		}
		unicode_foldreader_init(my_reader.w, my_str.cp16, my_len);
		unicode_foldreader_init(ot_reader.w, ot_str.cp16, ot_len);
		result = wildcasecomparew(&my_reader.w, &ot_reader.w);
		break;

	CASE_WIDTH_4BYTE:
		my_str.cp32 = DeeString_As4Byte((DeeObject *)self);
		if unlikely(!my_str.cp32)
			goto err;
		ot_str.cp32 = DeeString_As4Byte((DeeObject *)args.other);
		if unlikely(!ot_str.cp32)
			goto err;
		my_len = WSTR_LENGTH(my_str.cp32);
		ot_len = WSTR_LENGTH(ot_str.cp32);
		if (args.my_end > my_len)
			args.my_end = my_len;
		if (args.my_end <= args.my_start)
			my_len = 0;
		else {
			my_str.cp32 += args.my_start;
			my_len = args.my_end - args.my_start;
		}
		if (args.ot_end > ot_len)
			args.ot_end = ot_len;
		if (args.ot_end <= args.ot_start)
			ot_len = 0;
		else {
			ot_str.cp32 += args.ot_start;
			ot_len = args.ot_end - args.ot_start;
		}
		unicode_foldreader_init(my_reader.l, my_str.cp32, my_len);
		unicode_foldreader_init(ot_reader.l, ot_str.cp32, ot_len);
		result = wildcasecomparel(&my_reader.l, &ot_reader.l);
		break;
	}
	return_bool_(result == 0);
err:
	return NULL;
}

INTDEF DREF DeeObject *DCALL
DeeString_Split(DeeObject *__restrict self, DeeObject *__restrict seperator);
INTDEF DREF DeeObject *DCALL
DeeString_CaseSplit(DeeObject *__restrict self, DeeObject *__restrict seperator);

PRIVATE DREF DeeObject *DCALL
string_split(String *__restrict self,
             size_t argc, DeeObject **__restrict argv) {
	DeeObject *other;
	if (DeeArg_Unpack(argc, argv, "o:split", &other) ||
	    DeeObject_AssertTypeExact(other, &DeeString_Type))
		return NULL;
	return DeeString_Split((DeeObject *)self, other);
}

PRIVATE DREF DeeObject *DCALL
string_casesplit(String *__restrict self,
                 size_t argc, DeeObject **__restrict argv) {
	DeeObject *other;
	if (DeeArg_Unpack(argc, argv, "o:casesplit", &other) ||
	    DeeObject_AssertTypeExact(other, &DeeString_Type))
		return NULL;
	return DeeString_CaseSplit((DeeObject *)self, other);
}

INTDEF DREF DeeObject *DCALL
DeeString_SplitLines(DeeObject *__restrict self, bool keepends);

PRIVATE DREF DeeObject *DCALL
string_splitlines(String *__restrict self,
                  size_t argc, DeeObject **__restrict argv) {
	bool keepends = false;
	if (DeeArg_Unpack(argc, argv, "|b:splitlines", &keepends))
		return NULL;
	return DeeString_SplitLines((DeeObject *)self, keepends);
}

PRIVATE DREF DeeObject *DCALL
string_indent(String *__restrict self,
              size_t argc, DeeObject **__restrict argv) {
	DeeObject *filler = &str_tab;
	if (DeeArg_Unpack(argc, argv, "|o:indent", &filler) ||
	    DeeObject_AssertTypeExact(filler, &DeeString_Type))
		return NULL;
	return DeeString_Indent((DeeObject *)self, filler);
}

PRIVATE DREF DeeObject *DCALL
string_dedent(String *__restrict self,
              size_t argc, DeeObject **__restrict argv) {
	size_t max_chars = 1;
	DeeObject *mask  = NULL;
	if (DeeArg_Unpack(argc, argv, "|Iuo:dedent", &max_chars, &mask))
		return NULL;
	return mask
	       ? DeeString_Dedent((DeeObject *)self, max_chars, mask)
	       : DeeString_DedentSpc((DeeObject *)self, max_chars);
}


INTDEF dssize_t DCALL
DeeString_Format(dformatprinter printer, void *arg,
                 /*utf-8*/ char const *__restrict format,
                 size_t format_len, DeeObject *__restrict args);

PRIVATE DREF DeeObject *DCALL
string_format(String *__restrict self,
              size_t argc, DeeObject **__restrict argv) {
	DeeObject *args;
	char *utf8_repr;
	if (DeeArg_Unpack(argc, argv, "o:format", &args))
		goto err;
	utf8_repr = DeeString_AsUtf8((DeeObject *)self);
	if unlikely(!utf8_repr)
		goto err;
	{
		struct unicode_printer printer = UNICODE_PRINTER_INIT;
		if unlikely(DeeString_Format(&unicode_printer_print,
			                          &printer,
			                          utf8_repr,
			                          WSTR_LENGTH(utf8_repr),
			                          args) < 0)
		goto err_printer;
		return unicode_printer_pack(&printer);
err_printer:
		unicode_printer_fini(&printer);
	}
err:
	return NULL;
}

INTDEF DREF DeeObject *DCALL
DeeString_Scanf(DeeObject *__restrict self,
                DeeObject *__restrict format);
PRIVATE DREF DeeObject *DCALL
string_scanf(String *__restrict self,
             size_t argc, DeeObject **__restrict argv) {
	DeeObject *format;
	if (DeeArg_Unpack(argc, argv, "o:scanf", &format) ||
	    DeeObject_AssertTypeExact(format, &DeeString_Type))
		return NULL;
	return DeeString_Scanf((DeeObject *)self, format);
}



PRIVATE DREF DeeObject *DCALL
string_findmatch(String *__restrict self,
                 size_t argc, DeeObject **__restrict argv) {
	String *s_open, *s_clos;
	size_t start = 0, end = (size_t)-1;
	union dcharptr scan_str, open_str, clos_str, ptr;
	size_t scan_len, open_len, clos_len;
	size_t result;
	if (DeeArg_Unpack(argc, argv, "oo|IdId:findmatch", &s_open, &s_clos, &start, &end) ||
	    DeeObject_AssertTypeExact((DeeObject *)s_open, &DeeString_Type) ||
	    DeeObject_AssertTypeExact((DeeObject *)s_clos, &DeeString_Type))
		goto err;
	SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON3(DeeString_WIDTH(self),
	                                         DeeString_WIDTH(s_open),
	                                         DeeString_WIDTH(s_clos))) {

	CASE_WIDTH_1BYTE:
		scan_str.cp8 = DeeString_As1Byte((DeeObject *)self);
		open_str.cp8 = DeeString_As1Byte((DeeObject *)s_open);
		clos_str.cp8 = DeeString_As1Byte((DeeObject *)s_clos);
		open_len     = WSTR_LENGTH(open_str.cp8);
		clos_len     = WSTR_LENGTH(clos_str.cp8);
		if (end > WSTR_LENGTH(scan_str.cp8))
			end = WSTR_LENGTH(scan_str.cp8);
		if unlikely(end <= start)
			goto err_not_found; /* Empty search area. */
		scan_len = end - start;
		ptr.cp8 = find_matchb(scan_str.cp8 + start, scan_len,
		                      open_str.cp8, open_len,
		                      clos_str.cp8, clos_len);
		if unlikely(!ptr.cp8)
			goto err_not_found;
		result = (size_t)(ptr.cp8 - scan_str.cp8);
		break;

	CASE_WIDTH_2BYTE:
		scan_str.cp16 = DeeString_As2Byte((DeeObject *)self);
		if unlikely(!scan_str.cp16)
			goto err;
		open_str.cp16 = DeeString_As2Byte((DeeObject *)s_open);
		if unlikely(!open_str.cp16)
			goto err;
		clos_str.cp16 = DeeString_As2Byte((DeeObject *)s_clos);
		if unlikely(!clos_str.cp16)
			goto err;
		open_len = WSTR_LENGTH(open_str.cp16);
		clos_len = WSTR_LENGTH(clos_str.cp16);
		if (end > WSTR_LENGTH(scan_str.cp16))
			end = WSTR_LENGTH(scan_str.cp16);
		if unlikely(end <= start)
			goto err_not_found; /* Empty search area. */
		scan_len = end - start;
		ptr.cp16 = find_matchw(scan_str.cp16 + start, scan_len,
		                       open_str.cp16, open_len,
		                       clos_str.cp16, clos_len);
		if unlikely(!ptr.cp16)
			goto err_not_found;
		result = (size_t)(ptr.cp16 - scan_str.cp16);
		break;

	CASE_WIDTH_4BYTE:
		scan_str.cp32 = DeeString_As4Byte((DeeObject *)self);
		if unlikely(!scan_str.cp32)
			goto err;
		open_str.cp32 = DeeString_As4Byte((DeeObject *)s_open);
		if unlikely(!open_str.cp32)
			goto err;
		clos_str.cp32 = DeeString_As4Byte((DeeObject *)s_clos);
		if unlikely(!clos_str.cp32)
			goto err;
		open_len = WSTR_LENGTH(open_str.cp32);
		clos_len = WSTR_LENGTH(clos_str.cp32);
		if (end > WSTR_LENGTH(scan_str.cp32))
			end = WSTR_LENGTH(scan_str.cp32);
		if unlikely(end <= start)
			goto err_not_found; /* Empty search area. */
		scan_len = end - start;
		ptr.cp32 = find_matchl(scan_str.cp32 + start, scan_len,
		                       open_str.cp32, open_len,
		                       clos_str.cp32, clos_len);
		if unlikely(!ptr.cp32)
			goto err_not_found;
		result = (size_t)(ptr.cp32 - scan_str.cp32);
		break;
	}
	return DeeInt_NewSize(result);
err_not_found:
	return_reference_(&DeeInt_MinusOne);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
string_indexmatch(String *__restrict self,
                  size_t argc, DeeObject **__restrict argv) {
	String *s_open, *s_clos;
	size_t start = 0, end = (size_t)-1;
	union dcharptr scan_str, open_str, clos_str, ptr;
	size_t scan_len, open_len, clos_len;
	size_t result;
	if (DeeArg_Unpack(argc, argv, "oo|IdId:findmatch", &s_open, &s_clos, &start, &end) ||
	    DeeObject_AssertTypeExact((DeeObject *)s_open, &DeeString_Type) ||
	    DeeObject_AssertTypeExact((DeeObject *)s_clos, &DeeString_Type))
		goto err;
	SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON3(DeeString_WIDTH(self),
	                                         DeeString_WIDTH(s_open),
	                                         DeeString_WIDTH(s_clos))) {

	CASE_WIDTH_1BYTE:
		scan_str.cp8 = DeeString_As1Byte((DeeObject *)self);
		open_str.cp8 = DeeString_As1Byte((DeeObject *)s_open);
		clos_str.cp8 = DeeString_As1Byte((DeeObject *)s_clos);
		open_len     = WSTR_LENGTH(open_str.cp8);
		clos_len     = WSTR_LENGTH(clos_str.cp8);
		if (end > WSTR_LENGTH(scan_str.cp8))
			end = WSTR_LENGTH(scan_str.cp8);
		if unlikely(end <= start)
			goto err_not_found; /* Empty search area. */
		scan_len = end - start;
		ptr.cp8 = find_matchb(scan_str.cp8 + start, scan_len,
		                      open_str.cp8, open_len,
		                      clos_str.cp8, clos_len);
		if unlikely(!ptr.cp8)
			goto err_not_found;
		result = (size_t)(ptr.cp8 - scan_str.cp8);
		break;

	CASE_WIDTH_2BYTE:
		scan_str.cp16 = DeeString_As2Byte((DeeObject *)self);
		if unlikely(!scan_str.cp16)
			goto err;
		open_str.cp16 = DeeString_As2Byte((DeeObject *)s_open);
		if unlikely(!open_str.cp16)
			goto err;
		clos_str.cp16 = DeeString_As2Byte((DeeObject *)s_clos);
		if unlikely(!clos_str.cp16)
			goto err;
		open_len = WSTR_LENGTH(open_str.cp16);
		clos_len = WSTR_LENGTH(clos_str.cp16);
		if (end > WSTR_LENGTH(scan_str.cp16))
			end = WSTR_LENGTH(scan_str.cp16);
		if unlikely(end <= start)
			goto err_not_found; /* Empty search area. */
		scan_len = end - start;
		ptr.cp16 = find_matchw(scan_str.cp16 + start, scan_len,
		                       open_str.cp16, open_len,
		                       clos_str.cp16, clos_len);
		if unlikely(!ptr.cp16)
			goto err_not_found;
		result = (size_t)(ptr.cp16 - scan_str.cp16);
		break;

	CASE_WIDTH_4BYTE:
		scan_str.cp32 = DeeString_As4Byte((DeeObject *)self);
		if unlikely(!scan_str.cp32)
			goto err;
		open_str.cp32 = DeeString_As4Byte((DeeObject *)s_open);
		if unlikely(!open_str.cp32)
			goto err;
		clos_str.cp32 = DeeString_As4Byte((DeeObject *)s_clos);
		if unlikely(!clos_str.cp32)
			goto err;
		open_len = WSTR_LENGTH(open_str.cp32);
		clos_len = WSTR_LENGTH(clos_str.cp32);
		if (end > WSTR_LENGTH(scan_str.cp32))
			end = WSTR_LENGTH(scan_str.cp32);
		if unlikely(end <= start)
			goto err_not_found; /* Empty search area. */
		scan_len = end - start;
		ptr.cp32 = find_matchl(scan_str.cp32 + start, scan_len,
		                       open_str.cp32, open_len,
		                       clos_str.cp32, clos_len);
		if unlikely(!ptr.cp32)
			goto err_not_found;
		result = (size_t)(ptr.cp32 - scan_str.cp32);
		break;
	}
	return DeeInt_NewSize(result);
err_not_found:
	err_index_not_found((DeeObject *)self,
	                    (DeeObject *)s_clos);
err:
	return NULL;
}


PRIVATE DREF DeeObject *DCALL
string_casefindmatch(String *__restrict self,
                     size_t argc, DeeObject **__restrict argv) {
	String *s_open, *s_clos;
	size_t start = 0, end = (size_t)-1;
	union dcharptr scan_str, open_str, clos_str, ptr;
	size_t scan_len, open_len, clos_len, match_length;
	size_t result;
	if (DeeArg_Unpack(argc, argv, "oo|IdId:casefindmatch", &s_open, &s_clos, &start, &end) ||
	    DeeObject_AssertTypeExact((DeeObject *)s_open, &DeeString_Type) ||
	    DeeObject_AssertTypeExact((DeeObject *)s_clos, &DeeString_Type))
		goto err;
	SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON3(DeeString_WIDTH(self),
	                                         DeeString_WIDTH(s_open),
	                                         DeeString_WIDTH(s_clos))) {

	CASE_WIDTH_1BYTE:
		scan_str.cp8 = DeeString_As1Byte((DeeObject *)self);
		open_str.cp8 = DeeString_As1Byte((DeeObject *)s_open);
		clos_str.cp8 = DeeString_As1Byte((DeeObject *)s_clos);
		open_len     = WSTR_LENGTH(open_str.cp8);
		clos_len     = WSTR_LENGTH(clos_str.cp8);
		if (end > WSTR_LENGTH(scan_str.cp8))
			end = WSTR_LENGTH(scan_str.cp8);
		if unlikely(end <= start)
			goto err_not_found; /* Empty search area. */
		scan_len = end - start;
		ptr.cp8 = find_casematchb(scan_str.cp8 + start, scan_len,
		                          open_str.cp8, open_len,
		                          clos_str.cp8, clos_len,
		                          &match_length);
		if unlikely(!ptr.cp8)
			goto err_not_found;
		result = (size_t)(ptr.cp8 - scan_str.cp8);
		break;

	CASE_WIDTH_2BYTE:
		scan_str.cp16 = DeeString_As2Byte((DeeObject *)self);
		if unlikely(!scan_str.cp16)
			goto err;
		open_str.cp16 = DeeString_As2Byte((DeeObject *)s_open);
		if unlikely(!open_str.cp16)
			goto err;
		clos_str.cp16 = DeeString_As2Byte((DeeObject *)s_clos);
		if unlikely(!clos_str.cp16)
			goto err;
		open_len = WSTR_LENGTH(open_str.cp16);
		clos_len = WSTR_LENGTH(clos_str.cp16);
		if (end > WSTR_LENGTH(scan_str.cp16))
			end = WSTR_LENGTH(scan_str.cp16);
		if unlikely(end <= start)
			goto err_not_found; /* Empty search area. */
		scan_len = end - start;
		ptr.cp16 = find_casematchw(scan_str.cp16 + start, scan_len,
		                           open_str.cp16, open_len,
		                           clos_str.cp16, clos_len,
		                           &match_length);
		if unlikely(!ptr.cp16)
			goto err_not_found;
		result = (size_t)(ptr.cp16 - scan_str.cp16);
		break;

	CASE_WIDTH_4BYTE:
		scan_str.cp32 = DeeString_As4Byte((DeeObject *)self);
		if unlikely(!scan_str.cp32)
			goto err;
		open_str.cp32 = DeeString_As4Byte((DeeObject *)s_open);
		if unlikely(!open_str.cp32)
			goto err;
		clos_str.cp32 = DeeString_As4Byte((DeeObject *)s_clos);
		if unlikely(!clos_str.cp32)
			goto err;
		open_len = WSTR_LENGTH(open_str.cp32);
		clos_len = WSTR_LENGTH(clos_str.cp32);
		if (end > WSTR_LENGTH(scan_str.cp32))
			end = WSTR_LENGTH(scan_str.cp32);
		if unlikely(end <= start)
			goto err_not_found; /* Empty search area. */
		scan_len = end - start;
		ptr.cp32 = find_casematchl(scan_str.cp32 + start, scan_len,
		                           open_str.cp32, open_len,
		                           clos_str.cp32, clos_len,
		                           &match_length);
		if unlikely(!ptr.cp32)
			goto err_not_found;
		result = (size_t)(ptr.cp32 - scan_str.cp32);
		break;
	}
	return DeeTuple_Newf(DEE_FMT_SIZE_T
	                     DEE_FMT_SIZE_T,
	                     result,
	                     result + match_length);
err_not_found:
	return_none;
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
string_caseindexmatch(String *__restrict self,
                      size_t argc, DeeObject **__restrict argv) {
	String *s_open, *s_clos;
	size_t start = 0, end = (size_t)-1;
	union dcharptr scan_str, open_str, clos_str, ptr;
	size_t scan_len, open_len, clos_len, match_length;
	size_t result;
	if (DeeArg_Unpack(argc, argv, "oo|IdId:caseindexmatch", &s_open, &s_clos, &start, &end) ||
	    DeeObject_AssertTypeExact((DeeObject *)s_open, &DeeString_Type) ||
	    DeeObject_AssertTypeExact((DeeObject *)s_clos, &DeeString_Type))
		goto err;
	SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON3(DeeString_WIDTH(self),
	                                         DeeString_WIDTH(s_open),
	                                         DeeString_WIDTH(s_clos))) {

	CASE_WIDTH_1BYTE:
		scan_str.cp8 = DeeString_As1Byte((DeeObject *)self);
		open_str.cp8 = DeeString_As1Byte((DeeObject *)s_open);
		clos_str.cp8 = DeeString_As1Byte((DeeObject *)s_clos);
		open_len     = WSTR_LENGTH(open_str.cp8);
		clos_len     = WSTR_LENGTH(clos_str.cp8);
		if (end > WSTR_LENGTH(scan_str.cp8))
			end = WSTR_LENGTH(scan_str.cp8);
		if unlikely(end <= start)
			goto err_not_found; /* Empty search area. */
		scan_len = end - start;
		ptr.cp8 = find_casematchb(scan_str.cp8 + start, scan_len,
		                          open_str.cp8, open_len,
		                          clos_str.cp8, clos_len,
		                          &match_length);
		if unlikely(!ptr.cp8)
			goto err_not_found;
		result = (size_t)(ptr.cp8 - scan_str.cp8);
		break;

	CASE_WIDTH_2BYTE:
		scan_str.cp16 = DeeString_As2Byte((DeeObject *)self);
		if unlikely(!scan_str.cp16)
			goto err;
		open_str.cp16 = DeeString_As2Byte((DeeObject *)s_open);
		if unlikely(!open_str.cp16)
			goto err;
		clos_str.cp16 = DeeString_As2Byte((DeeObject *)s_clos);
		if unlikely(!clos_str.cp16)
			goto err;
		open_len = WSTR_LENGTH(open_str.cp16);
		clos_len = WSTR_LENGTH(clos_str.cp16);
		if (end > WSTR_LENGTH(scan_str.cp16))
			end = WSTR_LENGTH(scan_str.cp16);
		if unlikely(end <= start)
			goto err_not_found; /* Empty search area. */
		scan_len = end - start;
		ptr.cp16 = find_casematchw(scan_str.cp16 + start, scan_len,
		                           open_str.cp16, open_len,
		                           clos_str.cp16, clos_len,
		                           &match_length);
		if unlikely(!ptr.cp16)
			goto err_not_found;
		result = (size_t)(ptr.cp16 - scan_str.cp16);
		break;

	CASE_WIDTH_4BYTE:
		scan_str.cp32 = DeeString_As4Byte((DeeObject *)self);
		if unlikely(!scan_str.cp32)
			goto err;
		open_str.cp32 = DeeString_As4Byte((DeeObject *)s_open);
		if unlikely(!open_str.cp32)
			goto err;
		clos_str.cp32 = DeeString_As4Byte((DeeObject *)s_clos);
		if unlikely(!clos_str.cp32)
			goto err;
		open_len = WSTR_LENGTH(open_str.cp32);
		clos_len = WSTR_LENGTH(clos_str.cp32);
		if (end > WSTR_LENGTH(scan_str.cp32))
			end = WSTR_LENGTH(scan_str.cp32);
		if unlikely(end <= start)
			goto err_not_found; /* Empty search area. */
		scan_len = end - start;
		ptr.cp32 = find_casematchl(scan_str.cp32 + start, scan_len,
		                           open_str.cp32, open_len,
		                           clos_str.cp32, clos_len,
		                           &match_length);
		if unlikely(!ptr.cp32)
			goto err_not_found;
		result = (size_t)(ptr.cp32 - scan_str.cp32);
		break;
	}
	return DeeTuple_Newf(DEE_FMT_SIZE_T
	                     DEE_FMT_SIZE_T,
	                     result,
	                     result + match_length);
err_not_found:
	err_index_not_found((DeeObject *)self,
	                    (DeeObject *)s_clos);
err:
	return NULL;
}


PRIVATE DREF DeeObject *DCALL
string_rfindmatch(String *__restrict self,
                  size_t argc, DeeObject **__restrict argv) {
	String *s_open, *s_clos;
	size_t start = 0, end = (size_t)-1;
	union dcharptr scan_str, open_str, clos_str, ptr;
	size_t scan_len, open_len, clos_len;
	size_t result;
	if (DeeArg_Unpack(argc, argv, "oo|IdId:rfindmatch", &s_open, &s_clos, &start, &end) ||
	    DeeObject_AssertTypeExact((DeeObject *)s_open, &DeeString_Type) ||
	    DeeObject_AssertTypeExact((DeeObject *)s_clos, &DeeString_Type))
		goto err;
	SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON3(DeeString_WIDTH(self),
	                                         DeeString_WIDTH(s_open),
	                                         DeeString_WIDTH(s_clos))) {

	CASE_WIDTH_1BYTE:
		scan_str.cp8 = DeeString_As1Byte((DeeObject *)self);
		open_str.cp8 = DeeString_As1Byte((DeeObject *)s_open);
		clos_str.cp8 = DeeString_As1Byte((DeeObject *)s_clos);
		open_len     = WSTR_LENGTH(open_str.cp8);
		clos_len     = WSTR_LENGTH(clos_str.cp8);
		if (end > WSTR_LENGTH(scan_str.cp8))
			end = WSTR_LENGTH(scan_str.cp8);
		if unlikely(end <= start)
			goto err_not_found; /* Empty search area. */
		scan_len = end - start;
		ptr.cp8 = rfind_matchb(scan_str.cp8 + start, scan_len,
		                       open_str.cp8, open_len,
		                       clos_str.cp8, clos_len);
		if unlikely(!ptr.cp8)
			goto err_not_found;
		result = (size_t)(ptr.cp8 - scan_str.cp8);
		break;

	CASE_WIDTH_2BYTE:
		scan_str.cp16 = DeeString_As2Byte((DeeObject *)self);
		if unlikely(!scan_str.cp16)
			goto err;
		open_str.cp16 = DeeString_As2Byte((DeeObject *)s_open);
		if unlikely(!open_str.cp16)
			goto err;
		clos_str.cp16 = DeeString_As2Byte((DeeObject *)s_clos);
		if unlikely(!clos_str.cp16)
			goto err;
		open_len = WSTR_LENGTH(open_str.cp16);
		clos_len = WSTR_LENGTH(clos_str.cp16);
		if (end > WSTR_LENGTH(scan_str.cp16))
			end = WSTR_LENGTH(scan_str.cp16);
		if unlikely(end <= start)
			goto err_not_found; /* Empty search area. */
		scan_len = end - start;
		ptr.cp16 = rfind_matchw(scan_str.cp16 + start, scan_len,
		                        open_str.cp16, open_len,
		                        clos_str.cp16, clos_len);
		if unlikely(!ptr.cp16)
			goto err_not_found;
		result = (size_t)(ptr.cp16 - scan_str.cp16);
		break;

	CASE_WIDTH_4BYTE:
		scan_str.cp32 = DeeString_As4Byte((DeeObject *)self);
		if unlikely(!scan_str.cp32)
			goto err;
		open_str.cp32 = DeeString_As4Byte((DeeObject *)s_open);
		if unlikely(!open_str.cp32)
			goto err;
		clos_str.cp32 = DeeString_As4Byte((DeeObject *)s_clos);
		if unlikely(!clos_str.cp32)
			goto err;
		open_len = WSTR_LENGTH(open_str.cp32);
		clos_len = WSTR_LENGTH(clos_str.cp32);
		if (end > WSTR_LENGTH(scan_str.cp32))
			end = WSTR_LENGTH(scan_str.cp32);
		if unlikely(end <= start)
			goto err_not_found; /* Empty search area. */
		scan_len = end - start;
		ptr.cp32 = rfind_matchl(scan_str.cp32 + start, scan_len,
		                        open_str.cp32, open_len,
		                        clos_str.cp32, clos_len);
		if unlikely(!ptr.cp32)
			goto err_not_found;
		result = (size_t)(ptr.cp32 - scan_str.cp32);
		break;
	}
	return DeeInt_NewSize(result);
err_not_found:
	return_reference_(&DeeInt_MinusOne);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
string_rindexmatch(String *__restrict self,
                   size_t argc, DeeObject **__restrict argv) {
	String *s_open, *s_clos;
	size_t start = 0, end = (size_t)-1;
	union dcharptr scan_str, open_str, clos_str, ptr;
	size_t scan_len, open_len, clos_len;
	size_t result;
	if (DeeArg_Unpack(argc, argv, "oo|IdId:rindexmatch", &s_open, &s_clos, &start, &end) ||
	    DeeObject_AssertTypeExact((DeeObject *)s_open, &DeeString_Type) ||
	    DeeObject_AssertTypeExact((DeeObject *)s_clos, &DeeString_Type))
		goto err;
	SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON3(DeeString_WIDTH(self),
	                                         DeeString_WIDTH(s_open),
	                                         DeeString_WIDTH(s_clos))) {

	CASE_WIDTH_1BYTE:
		scan_str.cp8 = DeeString_As1Byte((DeeObject *)self);
		open_str.cp8 = DeeString_As1Byte((DeeObject *)s_open);
		clos_str.cp8 = DeeString_As1Byte((DeeObject *)s_clos);
		open_len     = WSTR_LENGTH(open_str.cp8);
		clos_len     = WSTR_LENGTH(clos_str.cp8);
		if (end > WSTR_LENGTH(scan_str.cp8))
			end = WSTR_LENGTH(scan_str.cp8);
		if unlikely(end <= start)
			goto err_not_found; /* Empty search area. */
		scan_len = end - start;
		ptr.cp8 = rfind_matchb(scan_str.cp8 + start, scan_len,
		                       open_str.cp8, open_len,
		                       clos_str.cp8, clos_len);
		if unlikely(!ptr.cp8)
			goto err_not_found;
		result = (size_t)(ptr.cp8 - scan_str.cp8);
		break;

	CASE_WIDTH_2BYTE:
		scan_str.cp16 = DeeString_As2Byte((DeeObject *)self);
		if unlikely(!scan_str.cp16)
			goto err;
		open_str.cp16 = DeeString_As2Byte((DeeObject *)s_open);
		if unlikely(!open_str.cp16)
			goto err;
		clos_str.cp16 = DeeString_As2Byte((DeeObject *)s_clos);
		if unlikely(!clos_str.cp16)
			goto err;
		open_len = WSTR_LENGTH(open_str.cp16);
		clos_len = WSTR_LENGTH(clos_str.cp16);
		if (end > WSTR_LENGTH(scan_str.cp16))
			end = WSTR_LENGTH(scan_str.cp16);
		if unlikely(end <= start)
			goto err_not_found; /* Empty search area. */
		scan_len = end - start;
		ptr.cp16 = rfind_matchw(scan_str.cp16 + start, scan_len,
		                        open_str.cp16, open_len,
		                        clos_str.cp16, clos_len);
		if unlikely(!ptr.cp16)
			goto err_not_found;
		result = (size_t)(ptr.cp16 - scan_str.cp16);
		break;

	CASE_WIDTH_4BYTE:
		scan_str.cp32 = DeeString_As4Byte((DeeObject *)self);
		if unlikely(!scan_str.cp32)
			goto err;
		open_str.cp32 = DeeString_As4Byte((DeeObject *)s_open);
		if unlikely(!open_str.cp32)
			goto err;
		clos_str.cp32 = DeeString_As4Byte((DeeObject *)s_clos);
		if unlikely(!clos_str.cp32)
			goto err;
		open_len = WSTR_LENGTH(open_str.cp32);
		clos_len = WSTR_LENGTH(clos_str.cp32);
		if (end > WSTR_LENGTH(scan_str.cp32))
			end = WSTR_LENGTH(scan_str.cp32);
		if unlikely(end <= start)
			goto err_not_found; /* Empty search area. */
		scan_len = end - start;
		ptr.cp32 = rfind_matchl(scan_str.cp32 + start, scan_len,
		                        open_str.cp32, open_len,
		                        clos_str.cp32, clos_len);
		if unlikely(!ptr.cp32)
			goto err_not_found;
		result = (size_t)(ptr.cp32 - scan_str.cp32);
		break;
	}
	return DeeInt_NewSize(result);
err_not_found:
	err_index_not_found((DeeObject *)self,
	                    (DeeObject *)s_open);
err:
	return NULL;
}


PRIVATE DREF DeeObject *DCALL
string_caserfindmatch(String *__restrict self,
                      size_t argc, DeeObject **__restrict argv) {
	String *s_open, *s_clos;
	size_t start = 0, end = (size_t)-1;
	union dcharptr scan_str, open_str, clos_str, ptr;
	size_t scan_len, open_len, clos_len, match_length;
	size_t result;
	if (DeeArg_Unpack(argc, argv, "oo|IdId:caserfindmatch", &s_open, &s_clos, &start, &end) ||
	    DeeObject_AssertTypeExact((DeeObject *)s_open, &DeeString_Type) ||
	    DeeObject_AssertTypeExact((DeeObject *)s_clos, &DeeString_Type))
		goto err;
	SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON3(DeeString_WIDTH(self),
	                                         DeeString_WIDTH(s_open),
	                                         DeeString_WIDTH(s_clos))) {

	CASE_WIDTH_1BYTE:
		scan_str.cp8 = DeeString_As1Byte((DeeObject *)self);
		open_str.cp8 = DeeString_As1Byte((DeeObject *)s_open);
		clos_str.cp8 = DeeString_As1Byte((DeeObject *)s_clos);
		open_len     = WSTR_LENGTH(open_str.cp8);
		clos_len     = WSTR_LENGTH(clos_str.cp8);
		if (end > WSTR_LENGTH(scan_str.cp8))
			end = WSTR_LENGTH(scan_str.cp8);
		if unlikely(end <= start)
			goto err_not_found; /* Empty search area. */
		scan_len = end - start;
		ptr.cp8 = rfind_casematchb(scan_str.cp8 + start, scan_len,
		                           open_str.cp8, open_len,
		                           clos_str.cp8, clos_len,
		                           &match_length);
		if unlikely(!ptr.cp8)
			goto err_not_found;
		result = (size_t)(ptr.cp8 - scan_str.cp8);
		break;

	CASE_WIDTH_2BYTE:
		scan_str.cp16 = DeeString_As2Byte((DeeObject *)self);
		if unlikely(!scan_str.cp16)
			goto err;
		open_str.cp16 = DeeString_As2Byte((DeeObject *)s_open);
		if unlikely(!open_str.cp16)
			goto err;
		clos_str.cp16 = DeeString_As2Byte((DeeObject *)s_clos);
		if unlikely(!clos_str.cp16)
			goto err;
		open_len = WSTR_LENGTH(open_str.cp16);
		clos_len = WSTR_LENGTH(clos_str.cp16);
		if (end > WSTR_LENGTH(scan_str.cp16))
			end = WSTR_LENGTH(scan_str.cp16);
		if unlikely(end <= start)
			goto err_not_found; /* Empty search area. */
		scan_len = end - start;
		ptr.cp16 = rfind_casematchw(scan_str.cp16 + start, scan_len,
		                            open_str.cp16, open_len,
		                            clos_str.cp16, clos_len,
		                            &match_length);
		if unlikely(!ptr.cp16)
			goto err_not_found;
		result = (size_t)(ptr.cp16 - scan_str.cp16);
		break;

	CASE_WIDTH_4BYTE:
		scan_str.cp32 = DeeString_As4Byte((DeeObject *)self);
		if unlikely(!scan_str.cp32)
			goto err;
		open_str.cp32 = DeeString_As4Byte((DeeObject *)s_open);
		if unlikely(!open_str.cp32)
			goto err;
		clos_str.cp32 = DeeString_As4Byte((DeeObject *)s_clos);
		if unlikely(!clos_str.cp32)
			goto err;
		open_len = WSTR_LENGTH(open_str.cp32);
		clos_len = WSTR_LENGTH(clos_str.cp32);
		if (end > WSTR_LENGTH(scan_str.cp32))
			end = WSTR_LENGTH(scan_str.cp32);
		if unlikely(end <= start)
			goto err_not_found; /* Empty search area. */
		scan_len = end - start;
		ptr.cp32 = rfind_casematchl(scan_str.cp32 + start, scan_len,
		                            open_str.cp32, open_len,
		                            clos_str.cp32, clos_len,
		                            &match_length);
		if unlikely(!ptr.cp32)
			goto err_not_found;
		result = (size_t)(ptr.cp32 - scan_str.cp32);
		break;
	}
	return DeeTuple_Newf(DEE_FMT_SIZE_T
	                     DEE_FMT_SIZE_T,
	                     result,
	                     result + match_length);
err_not_found:
	return_none;
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
string_caserindexmatch(String *__restrict self,
                       size_t argc, DeeObject **__restrict argv) {
	String *s_open, *s_clos;
	size_t start = 0, end = (size_t)-1;
	union dcharptr scan_str, open_str, clos_str, ptr;
	size_t scan_len, open_len, clos_len, match_length;
	size_t result;
	if (DeeArg_Unpack(argc, argv, "oo|IdId:caserindexmatch", &s_open, &s_clos, &start, &end) ||
	    DeeObject_AssertTypeExact((DeeObject *)s_open, &DeeString_Type) ||
	    DeeObject_AssertTypeExact((DeeObject *)s_clos, &DeeString_Type))
		goto err;
	SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON3(DeeString_WIDTH(self),
	                                         DeeString_WIDTH(s_open),
	                                         DeeString_WIDTH(s_clos))) {

	CASE_WIDTH_1BYTE:
		scan_str.cp8 = DeeString_As1Byte((DeeObject *)self);
		open_str.cp8 = DeeString_As1Byte((DeeObject *)s_open);
		clos_str.cp8 = DeeString_As1Byte((DeeObject *)s_clos);
		open_len     = WSTR_LENGTH(open_str.cp8);
		clos_len     = WSTR_LENGTH(clos_str.cp8);
		if (end > WSTR_LENGTH(scan_str.cp8))
			end = WSTR_LENGTH(scan_str.cp8);
		if unlikely(end <= start)
			goto err_not_found; /* Empty search area. */
		scan_len = end - start;
		ptr.cp8 = rfind_casematchb(scan_str.cp8 + start, scan_len,
		                           open_str.cp8, open_len,
		                           clos_str.cp8, clos_len,
		                           &match_length);
		if unlikely(!ptr.cp8)
			goto err_not_found;
		result = (size_t)(ptr.cp8 - scan_str.cp8);
		break;

	CASE_WIDTH_2BYTE:
		scan_str.cp16 = DeeString_As2Byte((DeeObject *)self);
		if unlikely(!scan_str.cp16)
			goto err;
		open_str.cp16 = DeeString_As2Byte((DeeObject *)s_open);
		if unlikely(!open_str.cp16)
			goto err;
		clos_str.cp16 = DeeString_As2Byte((DeeObject *)s_clos);
		if unlikely(!clos_str.cp16)
			goto err;
		open_len = WSTR_LENGTH(open_str.cp16);
		clos_len = WSTR_LENGTH(clos_str.cp16);
		if (end > WSTR_LENGTH(scan_str.cp16))
			end = WSTR_LENGTH(scan_str.cp16);
		if unlikely(end <= start)
			goto err_not_found; /* Empty search area. */
		scan_len = end - start;
		ptr.cp16 = rfind_casematchw(scan_str.cp16 + start, scan_len,
		                            open_str.cp16, open_len,
		                            clos_str.cp16, clos_len,
		                            &match_length);
		if unlikely(!ptr.cp16)
			goto err_not_found;
		result = (size_t)(ptr.cp16 - scan_str.cp16);
		break;

	CASE_WIDTH_4BYTE:
		scan_str.cp32 = DeeString_As4Byte((DeeObject *)self);
		if unlikely(!scan_str.cp32)
			goto err;
		open_str.cp32 = DeeString_As4Byte((DeeObject *)s_open);
		if unlikely(!open_str.cp32)
			goto err;
		clos_str.cp32 = DeeString_As4Byte((DeeObject *)s_clos);
		if unlikely(!clos_str.cp32)
			goto err;
		open_len = WSTR_LENGTH(open_str.cp32);
		clos_len = WSTR_LENGTH(clos_str.cp32);
		if (end > WSTR_LENGTH(scan_str.cp32))
			end = WSTR_LENGTH(scan_str.cp32);
		if unlikely(end <= start)
			goto err_not_found; /* Empty search area. */
		scan_len = end - start;
		ptr.cp32 = rfind_casematchl(scan_str.cp32 + start, scan_len,
		                            open_str.cp32, open_len,
		                            clos_str.cp32, clos_len,
		                            &match_length);
		if unlikely(!ptr.cp32)
			goto err_not_found;
		result = (size_t)(ptr.cp32 - scan_str.cp32);
		break;
	}
	return DeeTuple_Newf(DEE_FMT_SIZE_T
	                     DEE_FMT_SIZE_T,
	                     result,
	                     result + match_length);
err_not_found:
	err_index_not_found((DeeObject *)self,
	                    (DeeObject *)s_open);
err:
	return NULL;
}



PRIVATE DREF DeeObject *DCALL
string_partitionmatch(String *__restrict self,
                      size_t argc, DeeObject **__restrict argv) {
	String *s_open, *s_clos;
	size_t start = 0, end = (size_t)-1;
	union dcharptr scan_str, open_str, clos_str, match_start, match_end;
	size_t scan_len, open_len, clos_len;
	DREF DeeTupleObject *result;
	if (DeeArg_Unpack(argc, argv, "oo|IdId:partitionmatch", &s_open, &s_clos, &start, &end) ||
	    DeeObject_AssertTypeExact((DeeObject *)s_open, &DeeString_Type) ||
	    DeeObject_AssertTypeExact((DeeObject *)s_clos, &DeeString_Type))
		goto err;
#define SET_STRING(a, b, c)                                      \
	do {                                                         \
		if ((result->t_elem[0] = (DREF DeeObject *)(a)) == NULL) \
			goto err_r_0;                                        \
		if ((result->t_elem[1] = (DREF DeeObject *)(b)) == NULL) \
			goto err_r_1;                                        \
		if ((result->t_elem[2] = (DREF DeeObject *)(c)) == NULL) \
			goto err_r_2;                                        \
	} __WHILE0
	result = (DREF DeeTupleObject *)DeeTuple_NewUninitialized(3);
	if unlikely(!result)
		goto err;
	SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON3(DeeString_WIDTH(self),
	                                         DeeString_WIDTH(s_open),
	                                         DeeString_WIDTH(s_clos))) {

	CASE_WIDTH_1BYTE:
		scan_str.cp8 = DeeString_As1Byte((DeeObject *)self);
		open_str.cp8 = DeeString_As1Byte((DeeObject *)s_open);
		clos_str.cp8 = DeeString_As1Byte((DeeObject *)s_clos);
		open_len     = WSTR_LENGTH(open_str.cp8);
		clos_len     = WSTR_LENGTH(clos_str.cp8);
		if (end > WSTR_LENGTH(scan_str.cp8))
			end = WSTR_LENGTH(scan_str.cp8);
		if unlikely(end <= start)
			goto match_not_found; /* Empty search area. */
		scan_len        = end - start;
		match_start.cp8 = memmemb(scan_str.cp8 + start, scan_len,
		                          open_str.cp8, open_len);
		if unlikely(!match_start.cp8)
			goto match_not_found;
		match_end.cp8 = find_matchb(match_start.cp8 + open_len, scan_len - (match_start.cp8 - (scan_str.cp8 + start)),
		                            open_str.cp8, open_len,
		                            clos_str.cp8, clos_len);
		if unlikely(!match_end.cp8)
			goto match_not_found;
		SET_STRING(DeeString_New1Byte(scan_str.cp8,
		                              (size_t)(match_start.cp8 - scan_str.cp8)),
		           DeeString_New1Byte(match_start.cp8,
		                              (match_end.cp8 + clos_len) -
		                              match_start.cp8),
		           DeeString_New1Byte(match_end.cp8 + clos_len,
		                              (size_t)(scan_str.cp8 + end) -
		                              (size_t)(match_end.cp8 + clos_len)));
		break;

	CASE_WIDTH_2BYTE:
		scan_str.cp16 = DeeString_As2Byte((DeeObject *)self);
		if unlikely(!scan_str.cp16)
			goto err_r_0;
		open_str.cp16 = DeeString_As2Byte((DeeObject *)s_open);
		if unlikely(!open_str.cp16)
			goto err_r_0;
		clos_str.cp16 = DeeString_As2Byte((DeeObject *)s_clos);
		if unlikely(!clos_str.cp16)
			goto err_r_0;
		open_len = WSTR_LENGTH(open_str.cp16);
		clos_len = WSTR_LENGTH(clos_str.cp16);
		if (end > WSTR_LENGTH(scan_str.cp16))
			end = WSTR_LENGTH(scan_str.cp16);
		if unlikely(end <= start)
			goto match_not_found; /* Empty search area. */
		scan_len         = end - start;
		match_start.cp16 = memmemw(scan_str.cp16 + start, scan_len,
		                           open_str.cp16, open_len);
		if unlikely(!match_start.cp16)
			goto match_not_found;
		match_end.cp16 = find_matchw(match_start.cp16 + open_len, scan_len - (match_start.cp16 - (scan_str.cp16 + start)),
		                             open_str.cp16, open_len,
		                             clos_str.cp16, clos_len);
		if unlikely(!match_end.cp16)
			goto match_not_found;
		SET_STRING(DeeString_New2Byte(scan_str.cp16,
		                              (size_t)(match_start.cp16 - scan_str.cp16)),
		           DeeString_New2Byte(match_start.cp16,
		                              (match_end.cp16 + clos_len) -
		                              match_start.cp16),
		           DeeString_New2Byte(match_end.cp16 + clos_len,
		                              (size_t)(scan_str.cp16 + end) -
		                              (size_t)(match_end.cp16 + clos_len)));
		break;

	CASE_WIDTH_4BYTE:
		scan_str.cp32 = DeeString_As4Byte((DeeObject *)self);
		if unlikely(!scan_str.cp32)
			goto err_r_0;
		open_str.cp32 = DeeString_As4Byte((DeeObject *)s_open);
		if unlikely(!open_str.cp32)
			goto err_r_0;
		clos_str.cp32 = DeeString_As4Byte((DeeObject *)s_clos);
		if unlikely(!clos_str.cp32)
			goto err_r_0;
		open_len = WSTR_LENGTH(open_str.cp32);
		clos_len = WSTR_LENGTH(clos_str.cp32);
		if (end > WSTR_LENGTH(scan_str.cp32))
			end = WSTR_LENGTH(scan_str.cp32);
		if unlikely(end <= start)
			goto match_not_found; /* Empty search area. */
		scan_len         = end - start;
		match_start.cp32 = memmeml(scan_str.cp32 + start, scan_len,
		                           open_str.cp32, open_len);
		if unlikely(!match_start.cp32)
			goto match_not_found;
		match_end.cp32 = find_matchl(match_start.cp32 + open_len, scan_len - (match_start.cp32 - (scan_str.cp32 + start)),
		                             open_str.cp32, open_len,
		                             clos_str.cp32, clos_len);
		if unlikely(!match_end.cp32)
			goto match_not_found;
		SET_STRING(DeeString_New4Byte(scan_str.cp32,
		                              (size_t)(match_start.cp32 - scan_str.cp32)),
		           DeeString_New4Byte(match_start.cp32,
		                              (match_end.cp32 + clos_len) -
		                              match_start.cp32),
		           DeeString_New4Byte(match_end.cp32 + clos_len,
		                              (size_t)(scan_str.cp32 + end) -
		                              (size_t)(match_end.cp32 + clos_len)));
		break;
	}
#undef SET_STRING
done:
	return (DREF DeeObject *)result;
match_not_found:
	result->t_elem[0] = (DREF DeeObject *)string_getsubstr(self, start, end);
	if unlikely(!result->t_elem[0])
		goto err_r_0;
	result->t_elem[1] = Dee_EmptyString;
	result->t_elem[2] = Dee_EmptyString;
	Dee_Incref_n(Dee_EmptyString, 2);
	goto done;
err_r_2:
	Dee_Decref_likely(result->t_elem[1]);
err_r_1:
	Dee_Decref_likely(result->t_elem[0]);
err_r_0:
	DeeTuple_FreeUninitialized((DeeObject *)result);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
string_rpartitionmatch(String *__restrict self,
                       size_t argc, DeeObject **__restrict argv) {
	String *s_open, *s_clos;
	size_t start = 0, end = (size_t)-1;
	union dcharptr scan_str, open_str, clos_str, match_start, match_end;
	size_t scan_len, open_len, clos_len;
	DREF DeeTupleObject *result;
	if (DeeArg_Unpack(argc, argv, "oo|IdId:rpartitionmatch", &s_open, &s_clos, &start, &end) ||
	    DeeObject_AssertTypeExact((DeeObject *)s_open, &DeeString_Type) ||
	    DeeObject_AssertTypeExact((DeeObject *)s_clos, &DeeString_Type))
		goto err;
#define SET_STRING(a, b, c)                                      \
	do {                                                         \
		if ((result->t_elem[0] = (DREF DeeObject *)(a)) == NULL) \
			goto err_r_0;                                        \
		if ((result->t_elem[1] = (DREF DeeObject *)(b)) == NULL) \
			goto err_r_1;                                        \
		if ((result->t_elem[2] = (DREF DeeObject *)(c)) == NULL) \
			goto err_r_2;                                        \
	} __WHILE0
	result = (DREF DeeTupleObject *)DeeTuple_NewUninitialized(3);
	if unlikely(!result)
		goto err;
	SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON3(DeeString_WIDTH(self),
	                                         DeeString_WIDTH(s_open),
	                                         DeeString_WIDTH(s_clos))) {

	CASE_WIDTH_1BYTE:
		scan_str.cp8 = DeeString_As1Byte((DeeObject *)self);
		open_str.cp8 = DeeString_As1Byte((DeeObject *)s_open);
		clos_str.cp8 = DeeString_As1Byte((DeeObject *)s_clos);
		open_len     = WSTR_LENGTH(open_str.cp8);
		clos_len     = WSTR_LENGTH(clos_str.cp8);
		if (end > WSTR_LENGTH(scan_str.cp8))
			end = WSTR_LENGTH(scan_str.cp8);
		if unlikely(end <= start)
			goto match_not_found; /* Empty search area. */
		scan_len      = end - start;
		match_end.cp8 = memrmemb(scan_str.cp8 + start, scan_len,
		                         clos_str.cp8, clos_len);
		if unlikely(!match_end.cp8)
			goto match_not_found;
		match_start.cp8 = rfind_matchb(scan_str.cp8 + start,
		                               (size_t)(match_end.cp8 - (scan_str.cp8 + start)),
		                               open_str.cp8, open_len,
		                               clos_str.cp8, clos_len);
		if unlikely(!match_start.cp8)
			goto match_not_found;
		SET_STRING(DeeString_New1Byte(scan_str.cp8,
		                              (size_t)(match_start.cp8 - scan_str.cp8)),
		           DeeString_New1Byte(match_start.cp8,
		                              (size_t)((match_end.cp8 + clos_len) -
		                                       match_start.cp8)),
		           DeeString_New1Byte(match_end.cp8 + clos_len,
		                              (size_t)(scan_str.cp8 + end) -
		                              (size_t)(match_end.cp8 + clos_len)));
		break;

	CASE_WIDTH_2BYTE:
		scan_str.cp16 = DeeString_As2Byte((DeeObject *)self);
		if unlikely(!scan_str.cp16)
			goto err_r_0;
		open_str.cp16 = DeeString_As2Byte((DeeObject *)s_open);
		if unlikely(!open_str.cp16)
			goto err_r_0;
		clos_str.cp16 = DeeString_As2Byte((DeeObject *)s_clos);
		if unlikely(!clos_str.cp16)
			goto err_r_0;
		open_len = WSTR_LENGTH(open_str.cp16);
		clos_len = WSTR_LENGTH(clos_str.cp16);
		if (end > WSTR_LENGTH(scan_str.cp16))
			end = WSTR_LENGTH(scan_str.cp16);
		if unlikely(end <= start)
			goto match_not_found; /* Empty search area. */
		scan_len       = end - start;
		match_end.cp16 = memrmemw(scan_str.cp16 + start, scan_len,
		                          clos_str.cp16, clos_len);
		if unlikely(!match_end.cp16)
			goto match_not_found;
		match_start.cp16 = rfind_matchw(scan_str.cp16 + start,
		                                (size_t)(match_end.cp16 - (scan_str.cp16 + start)),
		                                open_str.cp16, open_len,
		                                clos_str.cp16, clos_len);
		if unlikely(!match_start.cp16)
			goto match_not_found;
		SET_STRING(DeeString_New2Byte(scan_str.cp16,
		                              (size_t)(match_start.cp16 - scan_str.cp16)),
		           DeeString_New2Byte(match_start.cp16,
		                              (size_t)((match_end.cp16 + clos_len) -
		                                       match_start.cp16)),
		           DeeString_New2Byte(match_end.cp16 + clos_len,
		                              (size_t)(scan_str.cp16 + end) -
		                              (size_t)(match_end.cp16 + clos_len)));
		break;

	CASE_WIDTH_4BYTE:
		scan_str.cp32 = DeeString_As4Byte((DeeObject *)self);
		if unlikely(!scan_str.cp32)
			goto err_r_0;
		open_str.cp32 = DeeString_As4Byte((DeeObject *)s_open);
		if unlikely(!open_str.cp32)
			goto err_r_0;
		clos_str.cp32 = DeeString_As4Byte((DeeObject *)s_clos);
		if unlikely(!clos_str.cp32)
			goto err_r_0;
		open_len = WSTR_LENGTH(open_str.cp32);
		clos_len = WSTR_LENGTH(clos_str.cp32);
		if (end > WSTR_LENGTH(scan_str.cp32))
			end = WSTR_LENGTH(scan_str.cp32);
		if unlikely(end <= start)
			goto match_not_found; /* Empty search area. */
		scan_len       = end - start;
		match_end.cp32 = memrmeml(scan_str.cp32 + start, scan_len,
		                          clos_str.cp32, clos_len);
		if unlikely(!match_end.cp32)
			goto match_not_found;
		match_start.cp32 = rfind_matchl(scan_str.cp32 + start,
		                                (size_t)(match_end.cp32 - (scan_str.cp32 + start)),
		                                open_str.cp32, open_len,
		                                clos_str.cp32, clos_len);
		if unlikely(!match_start.cp32)
			goto match_not_found;
		SET_STRING(DeeString_New4Byte(scan_str.cp32,
		                              (size_t)(match_start.cp32 - scan_str.cp32)),
		           DeeString_New4Byte(match_start.cp32,
		                              (size_t)((match_end.cp32 + clos_len) -
		                                       match_start.cp32)),
		           DeeString_New4Byte(match_end.cp32 + clos_len,
		                              (size_t)(scan_str.cp32 + end) -
		                              (size_t)(match_end.cp32 + clos_len)));
		break;
	}
#undef SET_STRING
done:
	return (DREF DeeObject *)result;
match_not_found:
	result->t_elem[0] = (DREF DeeObject *)string_getsubstr(self, start, end);
	if unlikely(!result->t_elem[0])
		goto err_r_0;
	result->t_elem[1] = Dee_EmptyString;
	result->t_elem[2] = Dee_EmptyString;
	Dee_Incref_n(Dee_EmptyString, 2);
	goto done;
err_r_2:
	Dee_Decref_likely(result->t_elem[1]);
err_r_1:
	Dee_Decref_likely(result->t_elem[0]);
err_r_0:
	DeeTuple_FreeUninitialized((DeeObject *)result);
err:
	return NULL;
}


PRIVATE DREF DeeObject *DCALL
string_casepartitionmatch(String *__restrict self,
                          size_t argc, DeeObject **__restrict argv) {
	String *s_open, *s_clos;
	size_t start = 0, end = (size_t)-1;
	union dcharptr scan_str, open_str, clos_str, match_start, match_end;
	size_t scan_len, open_len, clos_len, match_length;
	DREF DeeTupleObject *result;
	if (DeeArg_Unpack(argc, argv, "oo|IdId:casepartitionmatch", &s_open, &s_clos, &start, &end) ||
	    DeeObject_AssertTypeExact((DeeObject *)s_open, &DeeString_Type) ||
	    DeeObject_AssertTypeExact((DeeObject *)s_clos, &DeeString_Type))
		goto err;
#define SET_STRING(a, b, c)                                      \
	do {                                                         \
		if ((result->t_elem[0] = (DREF DeeObject *)(a)) == NULL) \
			goto err_r_0;                                        \
		if ((result->t_elem[1] = (DREF DeeObject *)(b)) == NULL) \
			goto err_r_1;                                        \
		if ((result->t_elem[2] = (DREF DeeObject *)(c)) == NULL) \
			goto err_r_2;                                        \
	} __WHILE0
	result = (DREF DeeTupleObject *)DeeTuple_NewUninitialized(3);
	if unlikely(!result)
		goto err;
	SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON3(DeeString_WIDTH(self),
	                                         DeeString_WIDTH(s_open),
	                                         DeeString_WIDTH(s_clos))) {

	CASE_WIDTH_1BYTE:
		scan_str.cp8 = DeeString_As1Byte((DeeObject *)self);
		open_str.cp8 = DeeString_As1Byte((DeeObject *)s_open);
		clos_str.cp8 = DeeString_As1Byte((DeeObject *)s_clos);
		open_len     = WSTR_LENGTH(open_str.cp8);
		clos_len     = WSTR_LENGTH(clos_str.cp8);
		if (end > WSTR_LENGTH(scan_str.cp8))
			end = WSTR_LENGTH(scan_str.cp8);
		if unlikely(end <= start)
			goto match_not_found; /* Empty search area. */
		scan_len        = end - start;
		match_start.cp8 = memcasememb(scan_str.cp8 + start, scan_len,
		                              open_str.cp8, open_len,
		                              &match_length);
		if unlikely(!match_start.cp8)
			goto match_not_found;
		match_end.cp8 = find_casematchb(match_start.cp8 + match_length, scan_len - (match_start.cp8 - (scan_str.cp8 + start)),
		                                open_str.cp8, open_len,
		                                clos_str.cp8, clos_len,
		                                &match_length);
		if unlikely(!match_end.cp8)
			goto match_not_found;
		SET_STRING(DeeString_New1Byte(scan_str.cp8,
		                              (size_t)(match_start.cp8 - scan_str.cp8)),
		           DeeString_New1Byte(match_start.cp8,
		                              (match_end.cp8 + match_length) -
		                              match_start.cp8),
		           DeeString_New1Byte(match_end.cp8 + match_length,
		                              (size_t)(scan_str.cp8 + end) -
		                              (size_t)(match_end.cp8 + match_length)));
		break;

	CASE_WIDTH_2BYTE:
		scan_str.cp16 = DeeString_As2Byte((DeeObject *)self);
		if unlikely(!scan_str.cp16)
			goto err_r_0;
		open_str.cp16 = DeeString_As2Byte((DeeObject *)s_open);
		if unlikely(!open_str.cp16)
			goto err_r_0;
		clos_str.cp16 = DeeString_As2Byte((DeeObject *)s_clos);
		if unlikely(!clos_str.cp16)
			goto err_r_0;
		open_len = WSTR_LENGTH(open_str.cp16);
		clos_len = WSTR_LENGTH(clos_str.cp16);
		if (end > WSTR_LENGTH(scan_str.cp16))
			end = WSTR_LENGTH(scan_str.cp16);
		if unlikely(end <= start)
			goto match_not_found; /* Empty search area. */
		scan_len         = end - start;
		match_start.cp16 = memcasememw(scan_str.cp16 + start, scan_len,
		                               open_str.cp16, open_len,
		                               &match_length);
		if unlikely(!match_start.cp16)
			goto match_not_found;
		match_end.cp16 = find_casematchw(match_start.cp16 + match_length, scan_len - (match_start.cp16 - (scan_str.cp16 + start)),
		                                 open_str.cp16, open_len,
		                                 clos_str.cp16, clos_len,
		                                 &match_length);
		if unlikely(!match_end.cp16)
			goto match_not_found;
		SET_STRING(DeeString_New2Byte(scan_str.cp16,
		                              (size_t)(match_start.cp16 - scan_str.cp16)),
		           DeeString_New2Byte(match_start.cp16,
		                              (match_end.cp16 + match_length) -
		                              match_start.cp16),
		           DeeString_New2Byte(match_end.cp16 + match_length,
		                              (size_t)(scan_str.cp16 + end) -
		                              (size_t)(match_end.cp16 + match_length)));
		break;

	CASE_WIDTH_4BYTE:
		scan_str.cp32 = DeeString_As4Byte((DeeObject *)self);
		if unlikely(!scan_str.cp32)
			goto err_r_0;
		open_str.cp32 = DeeString_As4Byte((DeeObject *)s_open);
		if unlikely(!open_str.cp32)
			goto err_r_0;
		clos_str.cp32 = DeeString_As4Byte((DeeObject *)s_clos);
		if unlikely(!clos_str.cp32)
			goto err_r_0;
		open_len = WSTR_LENGTH(open_str.cp32);
		clos_len = WSTR_LENGTH(clos_str.cp32);
		if (end > WSTR_LENGTH(scan_str.cp32))
			end = WSTR_LENGTH(scan_str.cp32);
		if unlikely(end <= start)
			goto match_not_found; /* Empty search area. */
		scan_len         = end - start;
		match_start.cp32 = memcasememl(scan_str.cp32 + start, scan_len,
		                               open_str.cp32, open_len,
		                               &match_length);
		if unlikely(!match_start.cp32)
			goto match_not_found;
		match_end.cp32 = find_casematchl(match_start.cp32 + match_length, scan_len - (match_start.cp32 - (scan_str.cp32 + start)),
		                                 open_str.cp32, open_len,
		                                 clos_str.cp32, clos_len,
		                                 &match_length);
		if unlikely(!match_end.cp32)
			goto match_not_found;
		SET_STRING(DeeString_New4Byte(scan_str.cp32,
		                              (size_t)(match_start.cp32 - scan_str.cp32)),
		           DeeString_New4Byte(match_start.cp32,
		                              (match_end.cp32 + match_length) -
		                              match_start.cp32),
		           DeeString_New4Byte(match_end.cp32 + match_length,
		                              (size_t)(scan_str.cp32 + end) -
		                              (size_t)(match_end.cp32 + match_length)));
		break;
	}
#undef SET_STRING
done:
	return (DREF DeeObject *)result;
match_not_found:
	result->t_elem[0] = (DREF DeeObject *)string_getsubstr(self, start, end);
	if unlikely(!result->t_elem[0])
		goto err_r_0;
	result->t_elem[1] = Dee_EmptyString;
	result->t_elem[2] = Dee_EmptyString;
	Dee_Incref_n(Dee_EmptyString, 2);
	goto done;
err_r_2:
	Dee_Decref_likely(result->t_elem[1]);
err_r_1:
	Dee_Decref_likely(result->t_elem[0]);
err_r_0:
	DeeTuple_FreeUninitialized((DeeObject *)result);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
string_caserpartitionmatch(String *__restrict self,
                           size_t argc, DeeObject **__restrict argv) {
	String *s_open, *s_clos;
	size_t start = 0, end = (size_t)-1;
	union dcharptr scan_str, open_str, clos_str, match_start, match_end;
	size_t scan_len, open_len, clos_len;
	DREF DeeTupleObject *result;
	if (DeeArg_Unpack(argc, argv, "oo|IdId:caserpartitionmatch", &s_open, &s_clos, &start, &end) ||
	    DeeObject_AssertTypeExact((DeeObject *)s_open, &DeeString_Type) ||
	    DeeObject_AssertTypeExact((DeeObject *)s_clos, &DeeString_Type))
		goto err;
#define SET_STRING(a, b, c)                                      \
	do {                                                         \
		if ((result->t_elem[0] = (DREF DeeObject *)(a)) == NULL) \
			goto err_r_0;                                        \
		if ((result->t_elem[1] = (DREF DeeObject *)(b)) == NULL) \
			goto err_r_1;                                        \
		if ((result->t_elem[2] = (DREF DeeObject *)(c)) == NULL) \
			goto err_r_2;                                        \
	} __WHILE0
	result = (DREF DeeTupleObject *)DeeTuple_NewUninitialized(3);
	if unlikely(!result)
		goto err;
	SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON3(DeeString_WIDTH(self),
	                                         DeeString_WIDTH(s_open),
	                                         DeeString_WIDTH(s_clos))) {

	CASE_WIDTH_1BYTE:
		scan_str.cp8 = DeeString_As1Byte((DeeObject *)self);
		open_str.cp8 = DeeString_As1Byte((DeeObject *)s_open);
		clos_str.cp8 = DeeString_As1Byte((DeeObject *)s_clos);
		open_len     = WSTR_LENGTH(open_str.cp8);
		clos_len     = WSTR_LENGTH(clos_str.cp8);
		if (end > WSTR_LENGTH(scan_str.cp8))
			end = WSTR_LENGTH(scan_str.cp8);
		if unlikely(end <= start)
			goto match_not_found; /* Empty search area. */
		scan_len      = end - start;
		match_end.cp8 = memcasermemb(scan_str.cp8 + start, scan_len,
		                             clos_str.cp8, clos_len, NULL);
		if unlikely(!match_end.cp8)
			goto match_not_found;
		match_start.cp8 = rfind_casematchb(scan_str.cp8 + start,
		                                   (size_t)(match_end.cp8 - (scan_str.cp8 + start)),
		                                   open_str.cp8, open_len,
		                                   clos_str.cp8, clos_len, NULL);
		if unlikely(!match_start.cp8)
			goto match_not_found;
		SET_STRING(DeeString_New1Byte(scan_str.cp8,
		                              (size_t)(match_start.cp8 - scan_str.cp8)),
		           DeeString_New1Byte(match_start.cp8,
		                              (size_t)((match_end.cp8 + clos_len) -
		                                       match_start.cp8)),
		           DeeString_New1Byte(match_end.cp8 + clos_len,
		                              (size_t)(scan_str.cp8 + end) -
		                              (size_t)(match_end.cp8 + clos_len)));
		break;

	CASE_WIDTH_2BYTE:
		scan_str.cp16 = DeeString_As2Byte((DeeObject *)self);
		if unlikely(!scan_str.cp16)
			goto err_r_0;
		open_str.cp16 = DeeString_As2Byte((DeeObject *)s_open);
		if unlikely(!open_str.cp16)
			goto err_r_0;
		clos_str.cp16 = DeeString_As2Byte((DeeObject *)s_clos);
		if unlikely(!clos_str.cp16)
			goto err_r_0;
		open_len = WSTR_LENGTH(open_str.cp16);
		clos_len = WSTR_LENGTH(clos_str.cp16);
		if (end > WSTR_LENGTH(scan_str.cp16))
			end = WSTR_LENGTH(scan_str.cp16);
		if unlikely(end <= start)
			goto match_not_found; /* Empty search area. */
		scan_len       = end - start;
		match_end.cp16 = memcasermemw(scan_str.cp16 + start, scan_len,
		                              clos_str.cp16, clos_len, NULL);
		if unlikely(!match_end.cp16)
			goto match_not_found;
		match_start.cp16 = rfind_casematchw(scan_str.cp16 + start,
		                                    (size_t)(match_end.cp16 - (scan_str.cp16 + start)),
		                                    open_str.cp16, open_len,
		                                    clos_str.cp16, clos_len, NULL);
		if unlikely(!match_start.cp16)
			goto match_not_found;
		SET_STRING(DeeString_New2Byte(scan_str.cp16,
		                              (size_t)(match_start.cp16 - scan_str.cp16)),
		           DeeString_New2Byte(match_start.cp16,
		                              (size_t)((match_end.cp16 + clos_len) -
		                                       match_start.cp16)),
		           DeeString_New2Byte(match_end.cp16 + clos_len,
		                              (size_t)(scan_str.cp16 + end) -
		                              (size_t)(match_end.cp16 + clos_len)));
		break;

	CASE_WIDTH_4BYTE:
		scan_str.cp32 = DeeString_As4Byte((DeeObject *)self);
		if unlikely(!scan_str.cp32)
			goto err_r_0;
		open_str.cp32 = DeeString_As4Byte((DeeObject *)s_open);
		if unlikely(!open_str.cp32)
			goto err_r_0;
		clos_str.cp32 = DeeString_As4Byte((DeeObject *)s_clos);
		if unlikely(!clos_str.cp32)
			goto err_r_0;
		open_len = WSTR_LENGTH(open_str.cp32);
		clos_len = WSTR_LENGTH(clos_str.cp32);
		if (end > WSTR_LENGTH(scan_str.cp32))
			end = WSTR_LENGTH(scan_str.cp32);
		if unlikely(end <= start)
			goto match_not_found; /* Empty search area. */
		scan_len       = end - start;
		match_end.cp32 = memcasermeml(scan_str.cp32 + start, scan_len,
		                              clos_str.cp32, clos_len, NULL);
		if unlikely(!match_end.cp32)
			goto match_not_found;
		match_start.cp32 = rfind_casematchl(scan_str.cp32 + start,
		                                    (size_t)(match_end.cp32 - (scan_str.cp32 + start)),
		                                    open_str.cp32, open_len,
		                                    clos_str.cp32, clos_len, NULL);
		if unlikely(!match_start.cp32)
			goto match_not_found;
		SET_STRING(DeeString_New4Byte(scan_str.cp32,
		                              (size_t)(match_start.cp32 - scan_str.cp32)),
		           DeeString_New4Byte(match_start.cp32,
		                              (size_t)((match_end.cp32 + clos_len) -
		                                       match_start.cp32)),
		           DeeString_New4Byte(match_end.cp32 + clos_len,
		                              (size_t)(scan_str.cp32 + end) -
		                              (size_t)(match_end.cp32 + clos_len)));
		break;
	}
#undef SET_STRING
done:
	return (DREF DeeObject *)result;
match_not_found:
	result->t_elem[0] = (DREF DeeObject *)string_getsubstr(self, start, end);
	if unlikely(!result->t_elem[0])
		goto err_r_0;
	result->t_elem[1] = Dee_EmptyString;
	result->t_elem[2] = Dee_EmptyString;
	Dee_Incref_n(Dee_EmptyString, 2);
	goto done;
err_r_2:
	Dee_Decref_likely(result->t_elem[1]);
err_r_1:
	Dee_Decref_likely(result->t_elem[0]);
err_r_0:
	DeeTuple_FreeUninitialized((DeeObject *)result);
err:
	return NULL;
}


struct re_args {
	char *re_dataptr;                          /* Starting pointer to regex input data (in UTF-8). */
	size_t re_datalen;                         /* Number of _bytes_ of regex input data. */
	char *re_patternptr;                       /* Starting pointer to regex pattern data (in UTF-8). */
	size_t re_patternlen;                      /* Number of _bytes_ of regex pattern data. */
	uint16_t re_flags;                         /* Regex flags. */
	uint16_t re_pad[(sizeof(void *) - 2) / 2]; /* ... */
	size_t re_offset;                          /* Starting character-offset into the matched string. */
};

struct re_args_ex {
	char *re_dataptr;                          /* Starting pointer to regex input data (in UTF-8). */
	size_t re_datalen;                         /* Number of _bytes_ of regex input data. */
	char *re_patternptr;                       /* Starting pointer to regex pattern data (in UTF-8). */
	size_t re_patternlen;                      /* Number of _bytes_ of regex pattern data. */
	uint16_t re_flags;                         /* Regex flags. */
	uint16_t re_pad[(sizeof(void *) - 2) / 2]; /* ... */
	size_t re_offset;                          /* Starting character-offset into the matched string. */
	size_t re_endindex;                        /* Ending character-offset within the matched string. */
};

#ifndef __NO_builtin_expect
#define regex_getargs_generic(self, function_name, argc, argv, result) \
	__builtin_expect(regex_getargs_generic(self, function_name, argc, argv, result), 0)
#define regex_getargs_generic_ex(self, function_name, argc, argv, result) \
	__builtin_expect(regex_getargs_generic_ex(self, function_name, argc, argv, result), 0)
#define regex_get_rules(rules_str, result) \
	__builtin_expect(regex_get_rules(rules_str, result), 0)
#endif

struct regex_rule_name {
	char name[14];
	uint16_t flag;
};

PRIVATE struct regex_rule_name const regex_rule_names[] = {
	{ "dotall", Dee_REGEX_FDOTALL },
	{ "multiline", Dee_REGEX_FMULTILINE },
	{ "nocase", Dee_REGEX_FNOCASE }
};


PRIVATE int (DCALL regex_get_rules)(char const *__restrict rules_str,
                                   uint16_t *__restrict result) {
	if (*rules_str == '.' || *rules_str == '\n' ||
	    *rules_str == 'l' || *rules_str == 'c') {
		/* Flag-mode */
		for (;;) {
			char ch = *rules_str++;
			if (!ch)
				break;
			if (ch == '.')
				*result |= Dee_REGEX_FDOTALL;
			else if (ch == '\n')
				*result |= Dee_REGEX_FMULTILINE;
			else if (ch == 'c')
				*result |= Dee_REGEX_FNOCASE;
			else {
				DeeError_Throwf(&DeeError_ValueError,
				                "Invalid regex rules string flag %:1q",
				                rules_str - 1);
				goto err;
			}
		}
	} else {
		for (;;) {
			/* Comma-mode */
			char *name_start = (char *)rules_str;
			while (*rules_str && *rules_str != ',')
				++rules_str;
			if (rules_str != name_start) {
				size_t rule_length = (size_t)(rules_str - name_start);
				size_t i;
				if (rule_length < COMPILER_LENOF(regex_rule_names[0].name)) {
					for (i = 0; i < COMPILER_LENOF(regex_rule_names); ++i) {
						if (!dee_asciicaseeq(regex_rule_names[i].name, name_start, rule_length))
							continue;
						*result |= regex_rule_names[i].flag;
						goto got_rule_name;
					}
				}
				DeeError_Throwf(&DeeError_ValueError,
				                "Invalid regex rules name %$q",
				                rule_length, name_start);
				goto err;
			}
got_rule_name:
			if (!*rules_str)
				break;
			++rules_str;
		}
	}
	return 0;
err:
	return -1;
}


PRIVATE int (DCALL regex_getargs_generic)(String *__restrict self,
                                         char const *__restrict function_name,
                                         size_t argc, DeeObject **__restrict argv,
                                         struct re_args *__restrict result) {
	DeeObject *pattern;
	result->re_flags  = Dee_REGEX_FNORMAL;
	result->re_offset = 0;
	switch (argc) {
	case 1:
do_full_data_match:
		result->re_dataptr = DeeString_AsUtf8((DeeObject *)self);
		if unlikely(!result->re_dataptr)
			goto err;
		result->re_datalen = WSTR_LENGTH(result->re_dataptr);
		break;
	case 2:
		if (DeeString_Check(argv[1])) {
			/* Rules. */
			if (regex_get_rules(DeeString_STR(argv[1]), &result->re_flags))
				goto err;
			goto do_full_data_match;
		}
		/* Start offset. */
		if (DeeObject_AsSSize(argv[1], (dssize_t *)&result->re_offset))
			goto err;
do_start_offset_only:
		if (result->re_offset >= DeeString_WLEN(self)) {
do_empty_scan:
			result->re_offset  = DeeString_WLEN(self);
			result->re_dataptr = "";
			result->re_datalen = 0;
		} else {
			result->re_dataptr = DeeString_AsUtf8((DeeObject *)self);
			if unlikely(!result->re_dataptr)
				goto err;
			result->re_datalen = WSTR_LENGTH(result->re_dataptr);
			if (result->re_dataptr == DeeString_STR(self)) {
				/* ASCII string. */
				result->re_dataptr += result->re_offset;
				result->re_datalen -= result->re_offset;
			} else {
				size_t i;
				char *iter, *end;
				/* UTF-8 string (manually adjust). */
				end = (iter = result->re_dataptr) + result->re_datalen;
				for (i = 0; i < result->re_offset; ++i) {
					if (iter >= end)
						break;
					utf8_readchar((char const **)&iter, end);
				}
				result->re_dataptr = iter;
				result->re_datalen = (size_t)(end - iter);
			}
		}
		break;

	case 3:
		if (DeeString_Check(argv[1])) {
			if (regex_get_rules(DeeString_STR(argv[1]), &result->re_flags))
				goto err;
			if (DeeObject_AsSSize(argv[2], (dssize_t *)&result->re_offset))
				goto err;
			goto do_start_offset_only;
		}
		goto do_load_3args;

		{
			size_t end_offset;
		case 4:
			if (DeeString_Check(argv[1])) {
				/* pattern,rules,start,end */
				if (regex_get_rules(DeeString_STR(argv[1]), &result->re_flags))
					goto err;
				if (DeeObject_AsSSize(argv[2], (dssize_t *)&result->re_offset))
					goto err;
				if (DeeObject_AsSSize(argv[3], (dssize_t *)&end_offset))
					goto err;
			} else {
				if (DeeObject_AssertTypeExact(argv[3], &DeeString_Type))
					goto err;
				if (regex_get_rules(DeeString_STR(argv[3]), &result->re_flags))
					goto err;
do_load_3args:
				if (DeeObject_AsSSize(argv[2], (dssize_t *)&end_offset))
					goto err;
				if (DeeObject_AsSSize(argv[1], (dssize_t *)&result->re_offset))
					goto err;
			}
			if (end_offset >= DeeString_WLEN(self))
				goto do_start_offset_only;
			if (result->re_offset >= end_offset)
				goto do_empty_scan;
			/* Do a sub-string scan. */
			result->re_dataptr = DeeString_AsUtf8((DeeObject *)self);
			if unlikely(!result->re_dataptr)
				goto err;
			result->re_datalen = WSTR_LENGTH(result->re_dataptr);
			if (result->re_dataptr == DeeString_STR(self)) {
				/* ASCII string. */
				result->re_dataptr += result->re_offset;
				result->re_datalen = end_offset - result->re_offset;
			} else {
				size_t i;
				char *iter, *end;
				/* UTF-8 string (manually adjust). */
				end = (iter = result->re_dataptr) + result->re_datalen;
				for (i = 0; i < result->re_offset; ++i) {
					if (iter >= end)
						break;
					utf8_readchar((char const **)&iter, end);
				}
				result->re_dataptr = iter;
				/* Continue scanning until the full sub-string has been indexed. */
				for (; i < end_offset; ++i) {
					if (iter >= end)
						break;
					utf8_readchar((char const **)&iter, end);
				}
				result->re_datalen = (size_t)(iter - result->re_dataptr);
			}
		}
		break;

	default:
		err_invalid_argc(function_name, argc, 1, 4);
		goto err;
	}
	pattern = argv[0];
	if (DeeObject_AssertTypeExact(pattern, &DeeString_Type))
		goto err;
	result->re_patternptr = DeeString_AsUtf8(pattern);
	if unlikely(!result->re_patternptr)
		goto err;
	result->re_patternlen = WSTR_LENGTH(result->re_patternptr);
	return 0;
err:
	return -1;
}

PRIVATE int (DCALL regex_getargs_generic_ex)(String *__restrict self,
                                            char const *__restrict function_name,
                                            size_t argc, DeeObject **__restrict argv,
                                            struct re_args_ex *__restrict result) {
	DeeObject *pattern;
	result->re_flags    = Dee_REGEX_FNORMAL;
	result->re_offset   = 0;
	result->re_endindex = DeeString_WLEN(self);
	switch (argc) {
	case 1:
do_full_data_match:
		result->re_dataptr = DeeString_AsUtf8((DeeObject *)self);
		if unlikely(!result->re_dataptr)
			goto err;
		result->re_datalen = WSTR_LENGTH(result->re_dataptr);
		break;
	case 2:
		if (DeeString_Check(argv[1])) {
			/* Rules. */
			if (regex_get_rules(DeeString_STR(argv[1]), &result->re_flags))
				goto err;
			goto do_full_data_match;
		}
		/* Start offset. */
		if (DeeObject_AsSSize(argv[1], (dssize_t *)&result->re_offset))
			goto err;
do_start_offset_only:
		if (result->re_offset >= DeeString_WLEN(self)) {
do_empty_scan:
			result->re_offset   = DeeString_WLEN(self);
			result->re_endindex = result->re_offset;
			result->re_dataptr  = "";
			result->re_datalen  = 0;
		} else {
			result->re_dataptr = DeeString_AsUtf8((DeeObject *)self);
			if unlikely(!result->re_dataptr)
				goto err;
			result->re_datalen = WSTR_LENGTH(result->re_dataptr);
			if (result->re_dataptr == DeeString_STR(self)) {
				/* ASCII string. */
				result->re_dataptr += result->re_offset;
				result->re_datalen -= result->re_offset;
			} else {
				size_t i;
				char *iter, *end;
				/* UTF-8 string (manually adjust). */
				end = (iter = result->re_dataptr) + result->re_datalen;
				for (i = 0; i < result->re_offset; ++i) {
					if (iter >= end)
						break;
					utf8_readchar((char const **)&iter, end);
				}
				result->re_dataptr = iter;
				result->re_datalen = (size_t)(end - iter);
			}
		}
		break;

	case 3:
		if (DeeString_Check(argv[1])) {
			if (regex_get_rules(DeeString_STR(argv[1]), &result->re_flags))
				goto err;
			if (DeeObject_AsSSize(argv[2], (dssize_t *)&result->re_offset))
				goto err;
			goto do_start_offset_only;
		}
		goto do_load_3args;

	case 4:
		if (DeeString_Check(argv[1])) {
			/* pattern,rules,start,end */
			if (regex_get_rules(DeeString_STR(argv[1]), &result->re_flags))
				goto err;
			if (DeeObject_AsSSize(argv[2], (dssize_t *)&result->re_offset))
				goto err;
			if (DeeObject_AsSSize(argv[3], (dssize_t *)&result->re_endindex))
				goto err;
		} else {
			if (DeeObject_AssertTypeExact(argv[3], &DeeString_Type))
				goto err;
			if (regex_get_rules(DeeString_STR(argv[3]), &result->re_flags))
				goto err;
do_load_3args:
			if (DeeObject_AsSSize(argv[1], (dssize_t *)&result->re_offset))
				goto err;
			if (DeeObject_AsSSize(argv[2], (dssize_t *)&result->re_endindex))
				goto err;
		}
		if (result->re_endindex >= DeeString_WLEN(self)) {
			result->re_endindex = DeeString_WLEN(self);
			goto do_start_offset_only;
		}
		if (result->re_offset >= result->re_endindex)
			goto do_empty_scan;
		/* Do a sub-string scan. */
		result->re_dataptr = DeeString_AsUtf8((DeeObject *)self);
		if unlikely(!result->re_dataptr)
			goto err;
		result->re_datalen = WSTR_LENGTH(result->re_dataptr);
		if (result->re_dataptr == DeeString_STR(self)) {
			/* ASCII string. */
			result->re_dataptr += result->re_offset;
			result->re_datalen = result->re_endindex - result->re_offset;
		} else {
			size_t i;
			char *iter, *end;
			/* UTF-8 string (manually adjust). */
			end = (iter = result->re_dataptr) + result->re_datalen;
			for (i = 0; i < result->re_offset; ++i) {
				if (iter >= end)
					break;
				utf8_readchar((char const **)&iter, end);
			}
			result->re_dataptr = iter;
			/* Continue scanning until the full sub-string has been indexed. */
			for (; i < result->re_endindex; ++i) {
				if (iter >= end)
					break;
				utf8_readchar((char const **)&iter, end);
			}
			result->re_datalen = (size_t)(iter - result->re_dataptr);
		}
		break;

	default:
		err_invalid_argc(function_name, argc, 1, 4);
		goto err;
	}
	pattern = argv[0];
	if (DeeObject_AssertTypeExact(pattern, &DeeString_Type))
		goto err;
	result->re_patternptr = DeeString_AsUtf8(pattern);
	if unlikely(!result->re_patternptr)
		goto err;
	result->re_patternlen = WSTR_LENGTH(result->re_patternptr);
	return 0;
err:
	return -1;
}

PRIVATE DREF DeeObject *DCALL
string_rematch(String *__restrict self,
               size_t argc, DeeObject **__restrict argv) {
	struct re_args args;
	size_t result;
	if (regex_getargs_generic(self, "rematch", argc, argv, &args))
		goto err;
	result = DeeRegex_Matches(args.re_dataptr,
	                          args.re_datalen,
	                          args.re_patternptr,
	                          args.re_patternlen,
	                          args.re_flags);
	if unlikely(result == (size_t)-1)
		goto err;
	return DeeInt_NewSize(result);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
string_rematches(String *__restrict self,
                 size_t argc, DeeObject **__restrict argv) {
	struct re_args args;
	size_t result;
	char const *data_endptr;
	if (regex_getargs_generic(self, "rematches", argc, argv, &args))
		goto err;
	result = DeeRegex_MatchesPtr(args.re_dataptr,
	                             args.re_datalen,
	                             args.re_patternptr,
	                             args.re_patternlen,
	                             &data_endptr,
	                             args.re_flags);
	if unlikely(result == (size_t)-1)
		goto err;
	return_bool(args.re_dataptr +
	            args.re_datalen ==
	            data_endptr);
err:
	return NULL;
}


PRIVATE DREF DeeObject *DCALL
regex_pack_range(size_t offset,
                 struct regex_range const *__restrict range) {
	DREF DeeObject *result, *temp;
	result = DeeTuple_NewUninitialized(2);
	if unlikely(!result)
		goto err;
	temp = DeeInt_NewSize(offset + range->rr_start);
	if unlikely(!temp)
		goto err_r;
	DeeTuple_SET(result, 0, temp);
	temp = DeeInt_NewSize(offset + range->rr_end);
	if unlikely(!temp)
		goto err_r_0;
	DeeTuple_SET(result, 1, temp);
	return result;
err_r_0:
	Dee_Decref_likely(DeeTuple_GET(result, 0));
err_r:
	DeeTuple_FreeUninitialized(result);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
string_refind(String *__restrict self,
              size_t argc, DeeObject **__restrict argv) {
	struct re_args args;
	int error;
	struct regex_range result_range;
	if (regex_getargs_generic(self, "refind", argc, argv, &args))
		goto err;
	error = DeeRegex_Find(args.re_dataptr,
	                      args.re_datalen,
	                      args.re_patternptr,
	                      args.re_patternlen,
	                      &result_range,
	                      args.re_flags);
	if unlikely(error < 0)
		goto err;
	if (!error)
		return_none;
	return regex_pack_range(args.re_offset, &result_range);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
string_rerfind(String *__restrict self,
               size_t argc, DeeObject **__restrict argv) {
	struct re_args args;
	int error;
	struct regex_range result_range;
	if (regex_getargs_generic(self, "rerfind", argc, argv, &args))
		goto err;
	error = DeeRegex_RFind(args.re_dataptr,
	                       args.re_datalen,
	                       args.re_patternptr,
	                       args.re_patternlen,
	                       &result_range,
	                       args.re_flags);
	if unlikely(error < 0)
		goto err;
	if (!error)
		return_none;
	return regex_pack_range(args.re_offset, &result_range);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
string_reindex(String *__restrict self,
               size_t argc, DeeObject **__restrict argv) {
	struct re_args args;
	int error;
	struct regex_range result_range;
	if (regex_getargs_generic(self, "reindex", argc, argv, &args))
		goto err;
	error = DeeRegex_Find(args.re_dataptr,
	                      args.re_datalen,
	                      args.re_patternptr,
	                      args.re_patternlen,
	                      &result_range,
	                      args.re_flags);
	if unlikely(error < 0)
		goto err;
	if (!error)
		goto err_not_found;
	return regex_pack_range(args.re_offset, &result_range);
err_not_found:
	err_index_not_found((DeeObject *)self, argv[0]);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
string_rerindex(String *__restrict self,
                size_t argc, DeeObject **__restrict argv) {
	struct re_args args;
	int error;
	struct regex_range result_range;
	if (regex_getargs_generic(self, "rerindex", argc, argv, &args))
		goto err;
	error = DeeRegex_RFind(args.re_dataptr,
	                       args.re_datalen,
	                       args.re_patternptr,
	                       args.re_patternlen,
	                       &result_range,
	                       args.re_flags);
	if unlikely(error < 0)
		goto err;
	if (!error)
		goto err_not_found;
	return regex_pack_range(args.re_offset, &result_range);
err_not_found:
	err_index_not_found((DeeObject *)self, argv[0]);
err:
	return NULL;
}

PRIVATE DREF String *DCALL
string_relocate(String *__restrict self,
                size_t argc, DeeObject **__restrict argv) {
	struct re_args args;
	int error;
	struct regex_range result_range;
	if (regex_getargs_generic(self, "relocate", argc, argv, &args))
		goto err;
	error = DeeRegex_Find(args.re_dataptr,
	                      args.re_datalen,
	                      args.re_patternptr,
	                      args.re_patternlen,
	                      &result_range,
	                      args.re_flags);
	if unlikely(error < 0)
		goto err;
	if (!error)
		return_reference_((String *)Dee_EmptyString);
	return string_getsubstr_ib(self,
	                           args.re_offset + result_range.rr_start,
	                           args.re_offset + result_range.rr_end);
err:
	return NULL;
}

PRIVATE DREF String *DCALL
string_rerlocate(String *__restrict self,
                 size_t argc, DeeObject **__restrict argv) {
	struct re_args args;
	int error;
	struct regex_range result_range;
	if (regex_getargs_generic(self, "rerlocate", argc, argv, &args))
		goto err;
	error = DeeRegex_RFind(args.re_dataptr,
	                       args.re_datalen,
	                       args.re_patternptr,
	                       args.re_patternlen,
	                       &result_range,
	                       args.re_flags);
	if unlikely(error < 0)
		goto err;
	if (!error)
		return_reference_((String *)Dee_EmptyString);
	return string_getsubstr_ib(self,
	                           args.re_offset + result_range.rr_start,
	                           args.re_offset + result_range.rr_end);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
regex_pack_partition(String *__restrict self, size_t offset,
                     struct regex_range const *__restrict range) {
	DREF DeeObject *result;
	DREF String *temp;
	result = DeeTuple_NewUninitialized(3);
	if unlikely(!result)
		goto err;
	if (!range->rr_start && !offset) {
		DeeTuple_SET(result, 0, Dee_EmptyString);
		Dee_Incref(Dee_EmptyString);
		if (range->rr_end != DeeString_WLEN(self))
			goto allocate_matched_substring;
		DeeTuple_SET(result, 1, (DeeObject *)self);
		Dee_Incref(self);
		DeeTuple_SET(result, 2, Dee_EmptyString);
		Dee_Incref(Dee_EmptyString);
		return result;
	}
	temp = string_getsubstr_ib(self,
	                           0,
	                           offset + range->rr_start);
	if unlikely(!temp)
		goto err_r;
	DeeTuple_SET(result, 0, (DeeObject *)temp); /* Inherit reference. */
allocate_matched_substring:
	temp = string_getsubstr_ib(self,
	                           offset + range->rr_start,
	                           offset + range->rr_end);
	if unlikely(!temp)
		goto err_r_0;
	DeeTuple_SET(result, 1, (DeeObject *)temp); /* Inherit reference. */
	if (offset + range->rr_end == 0) {
		DeeTuple_SET(result, 2, (DeeObject *)self);
		Dee_Incref(self);
	} else if (offset + range->rr_end == DeeString_WLEN(self)) {
		DeeTuple_SET(result, 2, Dee_EmptyString);
		Dee_Incref(Dee_EmptyString);
	} else {
		temp = string_getsubstr(self,
		                        offset + range->rr_end,
		                        (size_t)-1);
		if unlikely(!temp)
			goto err_r_1;
		DeeTuple_SET(result, 2, (DeeObject *)temp); /* Inherit reference. */
	}
	return result;
err_r_1:
	Dee_Decref_likely(DeeTuple_GET(result, 1));
err_r_0:
	Dee_Decref_likely(DeeTuple_GET(result, 0));
err_r:
	DeeTuple_FreeUninitialized(result);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
string_repartition(String *__restrict self,
                   size_t argc, DeeObject **__restrict argv) {
	struct re_args args;
	int error;
	struct regex_range result_range;
	if (regex_getargs_generic(self, "repartition", argc, argv, &args))
		goto err;
	error = DeeRegex_Find(args.re_dataptr,
	                      args.re_datalen,
	                      args.re_patternptr,
	                      args.re_patternlen,
	                      &result_range,
	                      args.re_flags);
	if unlikely(error < 0)
		goto err;
	if (!error)
		return DeeTuple_Pack(3, self, Dee_EmptyString, Dee_EmptyString);
	return regex_pack_partition(self, args.re_offset, &result_range);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
string_rerpartition(String *__restrict self,
                    size_t argc, DeeObject **__restrict argv) {
	struct re_args args;
	int error;
	struct regex_range result_range;
	if (regex_getargs_generic(self, "rerpartition", argc, argv, &args))
		goto err;
	error = DeeRegex_RFind(args.re_dataptr,
	                       args.re_datalen,
	                       args.re_patternptr,
	                       args.re_patternlen,
	                       &result_range,
	                       args.re_flags);
	if unlikely(error < 0)
		goto err;
	if (!error)
		return DeeTuple_Pack(3, self, Dee_EmptyString, Dee_EmptyString);
	return regex_pack_partition(self, args.re_offset, &result_range);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
string_rereplace(String *__restrict self,
                 size_t argc, DeeObject **__restrict argv) {
	String *find_pattern, *replace;
	DeeObject *opt1 = NULL, *opt2 = NULL;
	size_t max_count  = (size_t)-1;
	uint16_t re_flags = Dee_REGEX_FNORMAL;
	char *pattern_ptr;
	size_t pattern_len;
	char *data_ptr;
	size_t data_len;
	if (DeeArg_Unpack(argc, argv, "oo|oo:rereplace", &find_pattern, &replace, &opt1, &opt2) ||
	    DeeObject_AssertTypeExact((DeeObject *)find_pattern, &DeeString_Type) ||
	    DeeObject_AssertTypeExact((DeeObject *)replace, &DeeString_Type))
		goto err;
	if (opt1) {
		if (DeeString_Check(opt1)) {
			if (regex_get_rules(DeeString_STR(opt1), &re_flags))
				goto err;
			if (opt2 && DeeObject_AsSize(opt2, &max_count))
				goto err;
		} else {
			if (DeeObject_AsSize(opt1, &max_count))
				goto err;
			if (opt2 &&
			    (DeeObject_AssertTypeExact(opt2, &DeeString_Type) ||
			     regex_get_rules(DeeString_STR(opt2), &re_flags)))
				goto err;
		}
	}
	pattern_ptr = DeeString_AsUtf8((DeeObject *)find_pattern);
	if unlikely(!pattern_ptr)
		goto err;
	data_ptr = DeeString_AsUtf8((DeeObject *)self);
	if unlikely(!data_ptr)
		goto err;
	pattern_len = WSTR_LENGTH(pattern_ptr);
	data_len    = WSTR_LENGTH(data_ptr);
	{
		struct unicode_printer printer = UNICODE_PRINTER_INIT;
		char *iter, *end, *flush_start;
		end = (flush_start = iter = data_ptr) + data_len;
		while (iter < end) {
			struct regex_range_ptr range;
			int error;
			error = DeeRegex_FindPtr(iter,
			                         (size_t)(end - iter),
			                         pattern_ptr,
			                         pattern_len,
			                         &range,
			                         re_flags);
			if unlikely(error < 0)
				goto err_printer;
			if (!error)
				break;
			if (flush_start < range.rr_start) {
				if (unicode_printer_printutf8(&printer, flush_start,
				                              (size_t)(range.rr_start - flush_start)) < 0)
					goto err_printer;
			}
			if (unicode_printer_printstring(&printer, (DeeObject *)replace) < 0)
				goto err_printer;
			flush_start = range.rr_end;
			if (!max_count--)
				break;
			iter = range.rr_end;
		}
		/* Flush the remainder. */
		if (unicode_printer_printutf8(&printer, flush_start,
		                              (size_t)(end - flush_start)) < 0)
			goto err_printer;
		return unicode_printer_pack(&printer);
err_printer:
		unicode_printer_fini(&printer);
	}
err:
	return NULL;
}

INTDEF DREF DeeObject *DCALL string_re_findall(String *__restrict self, String *__restrict pattern, struct re_args const *__restrict args);
INTDEF DREF DeeObject *DCALL string_re_locateall(String *__restrict self, String *__restrict pattern, struct re_args const *__restrict args);
INTDEF DREF DeeObject *DCALL string_re_split(String *__restrict self, String *__restrict pattern, struct re_args const *__restrict args);

PRIVATE DREF DeeObject *DCALL
string_refindall(String *__restrict self,
                 size_t argc, DeeObject **__restrict argv) {
	struct re_args args;
	if (regex_getargs_generic(self, "refindall", argc, argv, &args))
		goto err;
	return string_re_findall(self, (String *)argv[0], &args);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
string_relocateall(String *__restrict self,
                   size_t argc, DeeObject **__restrict argv) {
	struct re_args args;
	if (regex_getargs_generic(self, "relocateall", argc, argv, &args))
		goto err;
	return string_re_locateall(self, (String *)argv[0], &args);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
string_resplit(String *__restrict self,
               size_t argc, DeeObject **__restrict argv) {
	struct re_args args;
	if (regex_getargs_generic(self, "resplit", argc, argv, &args))
		goto err;
	return string_re_split(self, (String *)argv[0], &args);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
string_restartswith(String *__restrict self,
                    size_t argc, DeeObject **__restrict argv) {
	struct re_args args;
	size_t result;
	if (regex_getargs_generic(self, "restartswith", argc, argv, &args))
		goto err;
	result = DeeRegex_Matches(args.re_dataptr,
	                          args.re_datalen,
	                          args.re_patternptr,
	                          args.re_patternlen,
	                          args.re_flags);
	if unlikely(result == (size_t)-1)
		goto err;
	return_bool_(result != 0);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
string_reendswith(String *__restrict self,
                  size_t argc, DeeObject **__restrict argv) {
	struct re_args args;
	int error;
	struct regex_range_ptr range;
	if (regex_getargs_generic(self, "reendswith", argc, argv, &args))
		goto err;
	error = DeeRegex_RFindPtr(args.re_dataptr,
	                          args.re_datalen,
	                          args.re_patternptr,
	                          args.re_patternlen,
	                          &range,
	                          args.re_flags);
	if unlikely(error < 0)
		goto err;
	return_bool_(error &&
	             range.rr_end == args.re_dataptr +
	                             args.re_datalen);
err:
	return NULL;
}

PRIVATE DREF String *DCALL
string_relstrip(String *__restrict self,
                size_t argc, DeeObject **__restrict argv) {
	struct re_args args;
	size_t temp;
	if (regex_getargs_generic(self, "relstrip", argc, argv, &args))
		goto err;
	for (;;) {
		char *data_end;
		temp = DeeRegex_MatchesPtr(args.re_dataptr,
		                           args.re_datalen,
		                           args.re_patternptr,
		                           args.re_patternlen,
		                           (char const **)&data_end,
		                           args.re_flags);
		if unlikely(temp == (size_t)-1)
			goto err;
		if (!temp)
			break;
		args.re_offset += temp;
		args.re_datalen -= (size_t)(data_end - args.re_dataptr);
		args.re_dataptr = data_end;
	}
	return string_getsubstr(self, args.re_offset, (size_t)-1);
err:
	return NULL;
}

PRIVATE DREF String *DCALL
string_rerstrip(String *__restrict self,
                size_t argc, DeeObject **__restrict argv) {
	struct re_args_ex args;
	int error;
	struct regex_range_ex range;
	if (regex_getargs_generic_ex(self, "rerstrip", argc, argv, &args))
		goto err;
	for (;;) {
		error = DeeRegex_RFindEx(args.re_dataptr,
		                         args.re_datalen,
		                         args.re_patternptr,
		                         args.re_patternlen,
		                         &range,
		                         args.re_flags);
		if unlikely(error < 0)
			goto err;
		if (!error)
			break;
		if (range.rr_end_ptr != args.re_dataptr + args.re_datalen)
			break;
		ASSERT(range.rr_start < range.rr_end);
		ASSERT(range.rr_start_ptr < range.rr_end_ptr);
		args.re_datalen  = (size_t)(range.rr_start_ptr - args.re_dataptr);
		args.re_endindex = args.re_offset + range.rr_start;
	}
	return string_getsubstr_ib(self, args.re_offset, args.re_endindex);
err:
	return NULL;
}

PRIVATE DREF String *DCALL
string_restrip(String *__restrict self,
               size_t argc, DeeObject **__restrict argv) {
	struct re_args_ex args;
	int error;
	struct regex_range_ex range;
	if (regex_getargs_generic_ex(self, "restrip", argc, argv, &args))
		goto err;
	for (;;) {
		char *data_end;
		size_t temp;
		temp = DeeRegex_MatchesPtr(args.re_dataptr,
		                           args.re_datalen,
		                           args.re_patternptr,
		                           args.re_patternlen,
		                           (char const **)&data_end,
		                           args.re_flags);
		if unlikely(temp == (size_t)-1)
			goto err;
		if (!temp)
			break;
		args.re_offset += temp;
		args.re_datalen -= (size_t)(data_end - args.re_dataptr);
		args.re_dataptr = data_end;
	}
	for (;;) {
		error = DeeRegex_RFindEx(args.re_dataptr,
		                         args.re_datalen,
		                         args.re_patternptr,
		                         args.re_patternlen,
		                         &range,
		                         args.re_flags);
		if unlikely(error < 0)
			goto err;
		if (!error)
			break;
		if (range.rr_end_ptr != args.re_dataptr + args.re_datalen)
			break;
		ASSERT(range.rr_start < range.rr_end);
		ASSERT(range.rr_start_ptr < range.rr_end_ptr);
		args.re_datalen  = (size_t)(range.rr_start_ptr - args.re_dataptr);
		args.re_endindex = args.re_offset + range.rr_start;
	}
	return string_getsubstr_ib(self, args.re_offset, args.re_endindex);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
string_recontains(String *__restrict self,
                  size_t argc, DeeObject **__restrict argv) {
	struct re_args_ex args;
	int error;
	struct regex_range_ptr range;
	if (regex_getargs_generic_ex(self, "recontains", argc, argv, &args))
		goto err;
	error = DeeRegex_FindPtr(args.re_dataptr,
	                         args.re_datalen,
	                         args.re_patternptr,
	                         args.re_patternlen,
	                         &range,
	                         args.re_flags);
	if unlikely(error < 0)
		goto err;
	return_bool_(error);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
string_recount(String *__restrict self,
               size_t argc, DeeObject **__restrict argv) {
	struct re_args_ex args;
	int error;
	size_t result;
	struct regex_range_ptr range;
	if (regex_getargs_generic_ex(self, "recount", argc, argv, &args))
		goto err;
	result = 0;
	for (;;) {
		error = DeeRegex_FindPtr(args.re_dataptr,
		                         args.re_datalen,
		                         args.re_patternptr,
		                         args.re_patternlen,
		                         &range,
		                         args.re_flags);
		if unlikely(error < 0)
			goto err;
		if (!error)
			break;
		++result;
		args.re_datalen -= (size_t)(range.rr_end - args.re_dataptr);
		args.re_dataptr = range.rr_end;
	}
	return DeeInt_NewSize(result);
err:
	return NULL;
}


PRIVATE DREF DeeObject *DCALL
string_sizeof(String *__restrict self,
              size_t argc, DeeObject **__restrict argv) {
	size_t result;
	struct string_utf *utf;
	if (DeeArg_Unpack(argc, argv, ":__sizeof__"))
		goto err;
	result = offsetof(String, s_str);
	result += (self->s_len + 1) * sizeof(char);
	utf = self->s_data;
	if (utf) {
		unsigned int i;
		result += sizeof(struct string_utf);
		for (i = 1; i < STRING_WIDTH_COUNT; ++i) {
			if (!utf->u_data[i])
				continue;
			result += STRING_MUL_SIZEOF_WIDTH(WSTR_LENGTH(utf->u_data[i]), i);
		}
		if (utf->u_data[STRING_WIDTH_1BYTE] &&
		    utf->u_data[STRING_WIDTH_1BYTE] != (size_t *)DeeString_STR(self))
			result += WSTR_LENGTH(utf->u_data[STRING_WIDTH_1BYTE]) * 1;
		if (utf->u_utf8 && utf->u_utf8 != (char *)DeeString_STR(self) &&
		    utf->u_utf8 != (char *)utf->u_data[STRING_WIDTH_1BYTE])
			result += WSTR_LENGTH(utf->u_utf8) * 1;
		if (utf->u_utf16 && (uint16_t *)utf->u_utf16 != (uint16_t *)utf->u_data[STRING_WIDTH_2BYTE])
			result += WSTR_LENGTH(utf->u_utf16) * 2;
	}
	return DeeInt_NewSize(result);
err:
	return NULL;
}


INTERN struct type_method string_methods[] = {

	/* String encode/decode functions */
	{ "decode",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_decode,
	  DOC("(codec:?.,errors=!Pstrict)->?X2?.?O\n"
	      "@throw ValueError The given @codec or @errors wasn't recognized\n"
	      "@throw UnicodeDecodeError @this string could not be decoded as @codec and @errors was set to $\"strict\"\n"
	      "@param errors The way that decode-errors are handled as one of $\"strict\", $\"replace\" or $\"ignore\"\n"
	      "Decode @this string, re-interpreting its underlying character bytes as @codec\n"
	      "Prior to processing, @codec is normalized as follows:\n"
	      ">name = name.lower().replace(\"_\",\"-\");\n"
	      ">if (name.startswith(\"iso-\"))\n"
	      "> name = \"iso\"+name[4:];\n"
	      ">else if (name.startswith(\"cp-\")) {\n"
	      "> name = \"cp\"+name[3:];\n"
	      ">}\n"
	      "Following that, @codec is compared against the following list of builtin codecs\n"
	      "%{table Codec Name|Aliases|Return type|Description\n"
	      "$\"ascii\"|$\"646\", $\"us-ascii\"|Same as $this|"
	      "Validate that all character of @this are apart of the unicode range U+0000 - U+007F\n"
	      "$\"latin-1\"|$\"iso8859-1\", $\"iso8859\", $\"8859\", $\"cp819\", $\"latin\", $\"latin1\", $\"l1\"|Same as $this|"
	      "Validate that all character of @this are apart of the unicode range U+0000 - U+00FF\n"
	      "$\"utf-8\"|$\"utf8\", $\"u8\", $\"utf\"|:string|Decode ${this.bytes()} as a UTF-8 encoded byte sequence\n"
	      "$\"utf-16\"|$\"utf16\", $\"u16\"|:string|Decode ${this.bytes()} as a UTF-16 sequence, encoded in host-endian\n"
	      "$\"utf-16-le\"|$\"utf16-le\", $\"u16-le\", $\"utf-16le\", $\"utf16le\", $\"u16le\"|:string|Decode ${this.bytes()} as a UTF-16 sequence, encoded in little-endian\n"
	      "$\"utf-16-be\"|$\"utf16-be\", $\"u16-be\", $\"utf-16be\", $\"utf16be\", $\"u16be\"|:string|Decode ${this.bytes()} as a UTF-16 sequence, encoded in big-endian\n"
	      "$\"utf-32\"|$\"utf32\", $\"u32\"|:string|Decode ${this.bytes()} as a UTF-32 sequence, encoded in host-endian\n"
	      "$\"utf-32-le\"|$\"utf32-le\", $\"u32-le\", $\"utf-32le\", $\"utf32le\", $\"u32le\"|:string|Decode ${this.bytes()} as a UTF-32 sequence, encoded in little-endian\n"
	      "$\"utf-32-be\"|$\"utf32-be\", $\"u32-be\", $\"utf-32be\", $\"utf32be\", $\"u32be\"|:string|Decode ${this.bytes()} as a UTF-32 sequence, encoded in big-endian\n"
	      "$\"string-escape\"|$\"backslash-escape\", $\"c-escape\"|:string|Decode a backslash-escaped string after stripping an optional leading and trailing $\"\\\"\" or $\"\\\'\" character\n"
	      "}\n"
	      "If the given @codec is not apart of this list, a call is made to :codecs:decode"),
	  TYPE_METHOD_FKWDS },
	{ "encode",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_encode,
	  DOC("(codec:?.,errors=!Pstrict)->?X3?DBytes?.?O\n"
	      "@throw ValueError The given @codec or @errors wasn't recognized\n"
	      "@throw UnicodeEncodeError @this string could not be decoded as @codec and @errors was set to $\"strict\"\n"
	      "@param errors The way that decode-errors are handled as one of $\"strict\", $\"replace\" or $\"ignore\"\n"
	      "Encode @this string, re-interpreting its underlying character bytes as @codec\n"
	      "Prior to processing, @codec is normalized as follows:\n"
	      ">name = name.lower().replace(\"_\",\"-\");\n"
	      ">if (name.startswith(\"iso-\"))\n"
	      "> name = \"iso\"+name[4:];\n"
	      ">else if (name.startswith(\"cp-\")) {\n"
	      "> name = \"cp\"+name[3:];\n"
	      ">}\n"
	      "Following that, @codec is compared against the following list of builtin codecs\n"
	      "%{table Codec Name|Aliases|Return type|Description\n"
	      "$\"ascii\"|$\"646\", $\"us-ascii\"|Same as $this|"
	      "Validate that all character of @this are apart of the unicode range U+0000 - U+007F\n"
	      "$\"latin-1\"|$\"iso8859-1\", $\"iso8859\", $\"8859\", $\"cp819\", $\"latin\", $\"latin1\", $\"l1\"|Same as $this|"
	      "Validate that all character of @this are apart of the unicode range U+0000 - U+00FF\n"
	      "$\"utf-8\"|$\"utf8\", $\"u8\", $\"utf\"|:Bytes|Encode character of @this string as a UTF-8 encoded byte sequence\n"
	      "$\"utf-16\"|$\"utf16\", $\"u16\"|:Bytes|Encode 'as a UTF-16 sequence, encoded in host-endian\n"
	      "$\"utf-16-le\"|$\"utf16-le\", $\"u16-le\", $\"utf-16le\", $\"utf16le\", $\"u16le\"|:Bytes|Encode @this string as a UTF-16 sequence, encoded in little-endian\n"
	      "$\"utf-16-be\"|$\"utf16-be\", $\"u16-be\", $\"utf-16be\", $\"utf16be\", $\"u16be\"|:Bytes|Encode @this string as a UTF-16 sequence, encoded in big-endian\n"
	      "$\"utf-32\"|$\"utf32\", $\"u32\"|:Bytes|Encode @this string as a UTF-32 sequence, encoded in host-endian\n"
	      "$\"utf-32-le\"|$\"utf32-le\", $\"u32-le\", $\"utf-32le\", $\"utf32le\", $\"u32le\"|:Bytes|Encode @this string as a UTF-32 sequence, encoded in little-endian\n"
	      "$\"utf-32-be\"|$\"utf32-be\", $\"u32-be\", $\"utf-32be\", $\"utf32be\", $\"u32be\"|:Bytes|Encode @this string as a UTF-32 sequence, encoded in big-endian\n"
	      "$\"string-escape\"|$\"backslash-escape\", $\"c-escape\"|:string|Encode @this string as a backslash-escaped string. This is similar to #op:repr, however the string is not surrounded by $\"\\\"\"-characters\n"
	      "}\n"
	      "If the given @codec is not apart of this list, a call is made to :codecs:encode"),
	  TYPE_METHOD_FKWDS },
	{ "bytes",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_bytes,
	  DOC("(allow_invalid=!f)->?DBytes\n"
	      "(start:?Dint,end:?Dint,allow_invalid=!f)->?DBytes\n"
	      "@throw ValueError @allow_invalid is :false, and @this string contains characters above $0xff\n"
	      "Returns a read-only bytes representation of the characters within ${this.substr(start,end)}, "
	      "using a single byte per character. A character greater than $0xff either causes : ValueError "
	      "to be thrown (when @allow_invalid is false), or is replaced with the ASCII character "
	      "$\"?\" in the returned Bytes object") },
	{ "ord",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_ord,
	  DOC("->?Dint\n"
	      "@throw ValueError The length of @this string is not equal to $1\n"
	      "Return the ordinal integral value of @this single-character string\n"
	      "\n"
	      "(index:?Dint)->?Dint\n"
	      "@throw IntegerOverflow The given @index is lower than $0\n"
	      "@throw IndexError The given @index is greater than ${#this}\n"
	      "Returns the ordinal integral value of the @index'th character of @this string") },

	/* String formatting / scanning. */
	{ "format",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_format,
	  DOC("(args:?S?O)->?.\n"
	      "Format @this string using @args:\n"
	      "This kind of formating is the most powerful variant of string formatting available in deemon.\n"
	      "Like most other ways of formatting strings, all text outside of format specifiers is simply "
	      "copied into the output string. Special rules are applied to text found inside or curly-braces ($\"{...}\")\n"
	      "In order to escape either $\"{\" or $\"}\" characters, use $\"{{\" and $\"}}\" respectively\n"
	      "Processing of text inside of curly-brace is split into 2 parts, both of which are optional and separated by $\":\":\n"
	      "-The object selection expression part\n"
	      "-The object spec string portion (which may contain further $\"{...}\"-blocks that are expanded beforehand)\n"
	      "%{table Selection expression|Description\n"
	      "$\"{}\"|Lazily create an iterator $iter as ${args.operator iter()} when encountered the "
	      "first time, then invoke ${iter.operator next()} and use its return value as format object\n"
	      "$\"{foo}\"|Use ${args[\"foo\"]} as format object\n"
	      "$\"{42}\"|Use ${args[42]} as format object\n"
	      "$\"{(x)}\"|Alias for $\"{x}\"\n"
	      "$\"{x.<expr>}\"|With $x being another selection expression, use ${x.operator . (<expr>)} (whitespace before and after $\".\" is ignored)\n"
	      "$\"{x[<expr>]}\"|With $x being another selection expression, use ${x.operator [] (<expr>)} (whitespace before and after $\"[\" and $\"]\" is ignored)\n"
	      "$\"{x[<expr>:]}\"|With $x being another selection expression, use ${x.operator [:] (<expr>,none)} (whitespace before and after $\"[\" and $\"]\" is ignored)\n"
	      "$\"{x[:<expr>]}\"|With $x being another selection expression, use ${x.operator [:] (none,<expr>)} (whitespace before and after $\"[\" and $\"]\" is ignored)\n"
	      "$\"{x[<expr1>:<expr2>]}\"|With $x being another selection expression, use ${x.operator [:] (<expr1>,<expr2>)} (whitespace before and after $\"[\", $\":\" and $\"]\" is ignored)\n"
	      "$\"{x(<expr1>,<expr2>,[...])}\"|With $x being another selection expression, use ${x(<expr1>,<expr2>,[...])} (whitespace before and after $\"(\", $\",\" and $\")\" is ignored)\n"
	      "$\"{x(<expr1>,<expr2>...)}\"|With $x being another selection expression, use ${x(<expr1>,<expr2>...)} (i.e. you're able to use expand expressions here) (whitespace before and after $\"(\", $\",\", $\"...\" and $\")\" is ignored)\n"
	      "$\"{x ? <expr1> : <expr2>}\"|With $x being another selection expression, use sub-expression <expr1> if ${x.operator bool()} is true, or <expr2> otherwise (whitespace before and after $\"?\" and $\":\" is ignored)\n"
	      "$\"{x ? : <expr2>}\"|Re-use $x as true-result, similar to $\"{x ? {x} : <expr2>}\" (whitespace before and after $\"?\" and $\":\" is ignored)\n"
	      "$\"{x ? <expr1>}\"|Use :none as false-result\" (whitespace before and after $\"?\" is ignored)\n"
	      "}\n"
	      "Sub-expressions in selections strings (the ${<expr>} above). "
	      "Note however that the angle brackets are not part of the syntax, "
	      "but used to highlight association:\n"
	      "%{table Sub-expression|Description\n"
	      "$\"42\"|Evaluates to ${int(42)}\n"
	      "$\"foobar\"|Evaluates to $\"foobar\"\n"
	      "$\"{x}\"|Evaluates to the object selected by another selection expression $x}\n"
	      "Once an object to-be formatted has been selected, the way in which it should "
	      "then be formatted can be altered through use of spec string portion\n"
	      "If a spec portion is not present, ${str selected_object} is simply appended "
	      "to the resulting string. Otherwise, ${selected_object.__format__(spec_string)} "
	      "is invoked, and the resulting object is appended instead\n"
	      "For this purpose, :Object implements a function :object.__format__ that provides "
	      "some basic spec options, which are also used for types not derived from :Object, "
	      "or ones overwriting ${operator .}, where invocationg with $\"__format__\" throws "
	      "either a :NotImplemented or :AttributeError error.\n"
	      "When used, :Object.__format__ provides the following functionality, with a "
	      ":ValueError being thrown for anything else, or anything not matching these "
	      "criteria\n"
	      "%{table Spec option|Description\n"
	      "$\"{:42}\"|Will append ${selected_object.operator str().ljust(42)} to the resulting string (s.a. #ljust)\n"
	      "$\"{:<42}\"|Same as $\"{:42}\"\n"
	      "$\"{:>42}\"|Will append ${selected_object.operator str().rjust(42)} to the resulting string (s.a. #rjust)\n"
	      "$\"{:^42}\"|Will append ${selected_object.operator str().center(42)} to the resulting string (s.a. #center)\n"
	      "$\"{:=42}\"|Will append ${selected_object.operator str().zfill(42)} to the resulting string (s.a. #zfill)\n"
	      "$\"{:42:foo}\"|Will append ${selected_object.operator str().ljust(42,\"foo\")} to the resulting string (s.a. #ljust)\n"
	      "$\"{:<42:foo}\"|Same as $\"{:42:foo}\"\n"
	      "$\"{:>42:foo}\"|Will append ${selected_object.operator str().rjust(42,\"foo\")} to the resulting string (s.a. #rjust)\n"
	      "$\"{:^42:foo}\"|Will append ${selected_object.operator str().center(42,\"foo\")} to the resulting string (s.a. #center)\n"
	      "$\"{:=42:foo}\"|Will append ${selected_object.operator str().zfill(42,\"foo\")} to the resulting string (s.a. #zfill)}") },
	{ "scanf",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_scanf,
	  DOC("(format:?.)->?S?O\n"
	      "@throw ValueError The given @format is malformed\n"
	      "@throw ValueError Conversion to an integer failed\n"
	      "Scan @this string using a scanf-like format string @format\n"
	      "No major changes have been made from C's scanf function, however regex-like range "
	      "expressions are supported, and the returned sequence ends as soon as either @format "
	      "or @this has been exhausted, or a miss-match has occurred\n"
	      "Scanf command blocks are structured as ${%[*][width]pattern}\n"
	      "Besides this, for convenience and better unicode integration, the following changes "
	      "have been made to C's regular scanf function:\n"
	      "%{table Format pattern|Yielded type|Description\n"
	      "$\" \"|-|Skip any number of characters from input data for which #isspace returns :true (${r\"\\s*\"})\n"
	      "$\"\\n\"|-|Skip any kind of line-feed, including $\"\\r\\n\", as well as any character for which #islf returns :true (${r\"\\n\"})\n"
	      "$\"%o\"|:int|Match up to `width' characters with ${r\"[+-]*(?\\d<8)+\"} and yield the result as an octal integer\n"
	      "$\"%d\"|:int|Match up to `width' characters with ${r\"[+-]*(?\\d<10)+\"} and yield the result as an decimal integer\n"
	      "$\"%x\", $\"%p\"|:int|Match up to `width' characters with ${r\"[+-]*((?\\d<16)|[a-fA-F])+\"} and yield the result as a hexadecimal integer\n"
	      "$\"%i\", $\"%u\"|:int|Match up to `width' characters with ${r\"[+-]*((?\\d=0)([xX](?\\d<16)+|[bB](?\\d<2)+)|(?\\d<10)+)\"} and yield the result as an integer with automatic radix\n"
	      "$\"%s\"|:string|Match up to `width' characters with ${r\"\\S+\"} and return them as a string\n"
	      "$\"%c\"|:string|Consume exactly `width' (see above) or one characters and return them as a string\n"
	      "$\"%[...]\"|:string|Similar to the regex (s.a. #rematch) range function (e.g. $\"%[^xyz]\", $\"%[abc]\", $\"%[a-z]\", $\"%[^\\]]\")}\n"
	      "Integer-width modifiers ($\"h\", $\"hh\", $\"l\", $\"ll\", $\"j\", $\"z\", "
	      "$\"t\", $\"L\", $\"I\", $\"I8\", $\"I16\", $\"I32\" and $\"I64\") are ignored") },

	/* What about something like this?:
	 * >> print "You name is $your_name, and I'm ${my_name}"
	 * >>       .substitute({ .your_name = "foo", .my_name = "bar" });
	 * >> print "You owe $guy $$10 dollar!"
	 * >>       .substitute({ .guy = "me" });
	 */

	/* String/Character traits */
	{ "isprint",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_isprint,
	  DOC("->?Dbool\n"
	      "(index:?Dint)->?Dbool\n"
	      "(start:?Dint,end:?Dint)->?Dbool\n"
	      "@throw IndexError The given @index is larger than ${#this}\n"
	      "@throw IntegerOverflow The given @index is negative or too large\n"
	      "Returns :true if $this, ${this[index]}, or all "
	      "characters in ${this.substr(start,end)} are printable") },
	{ "isalpha",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_isalpha,
	  DOC("->?Dbool\n"
	      "(index:?Dint)->?Dbool\n"
	      "(start:?Dint,end:?Dint)->?Dbool\n"
	      "@throw IndexError The given @index is larger than ${#this}\n"
	      "@throw IntegerOverflow The given @index is negative or too large\n"
	      "Returns :true if $this, ${this[index]}, or all characters "
	      "in ${this.substr(start,end)} are alphabetical") },
	{ "isspace",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_isspace,
	  DOC("->?Dbool\n"
	      "(index:?Dint)->?Dbool\n"
	      "(start:?Dint,end:?Dint)->?Dbool\n"
	      "@throw IndexError The given @index is larger than ${#this}\n"
	      "@throw IntegerOverflow The given @index is negative or too large\n"
	      "Returns :true if $this, ${this[index]}, or all characters "
	      "in ${this.substr(start,end)} are space-characters") },
	{ "islf",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_islf,
	  DOC("->?Dbool\n"
	      "(index:?Dint)->?Dbool\n"
	      "(start:?Dint,end:?Dint)->?Dbool\n"
	      "@throw IndexError The given @index is larger than ${#this}\n"
	      "@throw IntegerOverflow The given @index is negative or too large\n"
	      "Returns :true if $this, ${this[index]}, or all characters "
	      "in ${this.substr(start,end)} are line-feeds") },
	{ "islower",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_islower,
	  DOC("->?Dbool\n"
	      "(index:?Dint)->?Dbool\n"
	      "(start:?Dint,end:?Dint)->?Dbool\n"
	      "@throw IndexError The given @index is larger than ${#this}\n"
	      "@throw IntegerOverflow The given @index is negative or too large\n"
	      "Returns :true if $this, ${this[index]}, or all characters "
	      "in ${this.substr(start,end)} are lower-case") },
	{ "isupper",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_isupper,
	  DOC("->?Dbool\n"
	      "(index:?Dint)->?Dbool\n"
	      "(start:?Dint,end:?Dint)->?Dbool\n"
	      "@throw IndexError The given @index is larger than ${#this}\n"
	      "@throw IntegerOverflow The given @index is negative or too large\n"
	      "Returns :true if $this, ${this[index]}, or all characters "
	      "in ${this.substr(start,end)} are upper-case") },
	{ "iscntrl",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_iscntrl,
	  DOC("->?Dbool\n"
	      "(index:?Dint)->?Dbool\n"
	      "(start:?Dint,end:?Dint)->?Dbool\n"
	      "@throw IndexError The given @index is larger than ${#this}\n"
	      "@throw IntegerOverflow The given @index is negative or too large\n"
	      "Returns :true if $this, ${this[index]}, or all characters "
	      "in ${this.substr(start,end)} are control characters") },
	{ "isdigit",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_isdigit,
	  DOC("->?Dbool\n"
	      "(index:?Dint)->?Dbool\n"
	      "(start:?Dint,end:?Dint)->?Dbool\n"
	      "@throw IndexError The given @index is larger than ${#this}\n"
	      "@throw IntegerOverflow The given @index is negative or too large\n"
	      "Returns :true if $this, ${this[index]}, or all characters "
	      "in ${this.substr(start,end)} are digits") },
	{ "isdecimal",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_isdecimal,
	  DOC("->?Dbool\n"
	      "(index:?Dint)->?Dbool\n"
	      "(start:?Dint,end:?Dint)->?Dbool\n"
	      "@throw IndexError The given @index is larger than ${#this}\n"
	      "@throw IntegerOverflow The given @index is negative or too large\n"
	      "Returns :true if $this, ${this[index]}, or all characters "
	      "in ${this.substr(start,end)} are dicimal characters") },
	{ "issymstrt",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_issymstrt,
	  DOC("->?Dbool\n"
	      "(index:?Dint)->?Dbool\n"
	      "(start:?Dint,end:?Dint)->?Dbool\n"
	      "@throw IndexError The given @index is larger than ${#this}\n"
	      "@throw IntegerOverflow The given @index is negative or too large\n"
	      "Returns :true if $this, ${this[index]}, or all characters "
	      "in ${this.substr(start,end)} can be used to start a symbol name") },
	{ "issymcont",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_issymcont,
	  DOC("->?Dbool\n"
	      "(index:?Dint)->?Dbool\n"
	      "(start:?Dint,end:?Dint)->?Dbool\n"
	      "@throw IndexError The given @index is larger than ${#this}\n"
	      "@throw IntegerOverflow The given @index is negative or too large\n"
	      "Returns :true if $this, ${this[index]}, or all characters "
	      "in ${this.substr(start,end)} can be used to continue a symbol name") },
	{ "isalnum",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_isalnum,
	  DOC("->?Dbool\n"
	      "(index:?Dint)->?Dbool\n"
	      "(start:?Dint,end:?Dint)->?Dbool\n"
	      "@throw IndexError The given @index is larger than ${#this}\n"
	      "@throw IntegerOverflow The given @index is negative or too large\n"
	      "Returns :true if $this, ${this[index]}, or all characters "
	      "in ${this.substr(start,end)} are alpha-numerical") },
	{ "isnumeric",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_isnumeric,
	  DOC("->?Dbool\n"
	      "(index:?Dint)->?Dbool\n"
	      "(start:?Dint,end:?Dint)->?Dbool\n"
	      "@throw IndexError The given @index is larger than ${#this}\n"
	      "@throw IntegerOverflow The given @index is negative or too large\n"
	      "Returns :true if $this, ${this[index]}, or all characters "
	      "in ${this.substr(start,end)} qualify as digit or decimal characters\n"
	      "This function is the logical union of #isdigit and #isdecimal") },
	{ "istitle",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_istitle,
	  DOC("(index:?Dint)->?Dbool\n"
	      "@throw IndexError The given @index is larger than ${#this}\n"
	      "@throw IntegerOverflow The given @index is negative or too large\n"
	      "Returns :true if the character at ${this[index]} has title-casing\n"
	      "\n"
	      "->?Dbool\n"
	      "(start:?Dint,end:?Dint)->?Dbool\n"
	      "Returns :true if $this, or the sub-string ${this.substr(start,end)} "
	      "follows title-casing, meaning that space is followed by title-case, "
	      "with all remaining characters not being title-case") },
	{ "issymbol",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_issymbol,
	  DOC("(index:?Dint)->?Dbool\n"
	      "@throw IndexError The given @index is larger than ${#this}\n"
	      "@throw IntegerOverflow The given @index is negative or too large\n"
	      "Returns :true if the character at ${this[index]} can be used "
	      "to start a symbol name. Same as ${this.issymstrt(index)}\n"
	      "\n"
	      "->?Dbool\n"
	      "(start:?Dint,end:?Dint)->?Dbool\n"
	      "Returns :true if $this, or the sub-string ${this.substr(start,end)} "
	      "is a valid symbol name") },
	{ "isascii",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_isascii,
	  DOC("->?Dbool\n"
	      "(index:?Dint)->?Dbool\n"
	      "(start:?Dint,end:?Dint)->?Dbool\n"
	      "@throw IndexError The given @index is larger than ${#this}\n"
	      "@throw IntegerOverflow The given @index is negative or too large\n"
	      "Returns :true if $this, ${this[index]}, or all characters in ${this.substr(start,end)} "
	      "are ascii-characters, that is have an ordinal value ${<= 0x7f}") },
	{ "asnumber",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_asnumber,
	  DOC("->?Dint\n"
	      "@throw ValueError The string is longer than a single character\n"
	      "(index:?Dint)->?Dint\n"
	      "@throw ValueError The character at @index isn't numeric\n"
	      "(index:?Dint,defl:?Dint)->?Dint\n"
	      "(index:?Dint,defl)->\n"
	      "@throw IntegerOverflow The given @index is negative or too large\n"
	      "@throw IndexError The given @index is out of bounds\n"
	      "Return the numeric value of the @index'th or only character of @this string, "
	      "or throw a :ValueError or return @defl if that character isn't #isnumeric\n"
	      ">print \"5\".isdigit();   /* true */\n"
	      ">print \"5\".isdecimal(); /* true */\n"
	      ">print \"5\".isnumeric(); /* true */\n"
	      ">print \"5\".asnumber();  /* 5 */\n"
	      ">print \"\xC2\xB2\".isdigit();   /* true */\n"
	      ">print \"\xC2\xB2\".isdecimal(); /* false */\n"
	      ">print \"\xC2\xB2\".isnumeric(); /* true */\n"
	      ">print \"\xC2\xB2\".asnumber();  /* 2 */") },
	{ "asdigit",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_asdigit,
	  DOC("->?Dint\n"
	      "@throw ValueError The string is longer than a single character\n"
	      "(index:?Dint)->?Dint\n"
	      "@throw ValueError The character at @index isn't numeric\n"
	      "(index:?Dint,defl:?Dint)->?Dint\n"
	      "(index:?Dint,defl)->\n"
	      "@throw IntegerOverflow The given @index is negative or too large\n"
	      "@throw IndexError The given @index is out of bounds\n"
	      "Same as #asnumber, but only succeed if the selected character matches #isdigit, rather than #isnumeric") },
	{ "asdecimal",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_asdecimal,
	  DOC("->?Dint\n"
	      "@throw ValueError The string is longer than a single character\n"
	      "(index:?Dint)->?Dint\n"
	      "@throw ValueError The character at @index isn't numeric\n"
	      "(index:?Dint,defl:?Dint)->?Dint\n"
	      "(index:?Dint,defl)->\n"
	      "@throw IntegerOverflow The given @index is negative or too large\n"
	      "@throw IndexError The given @index is out of bounds\n"
	      "Same as #asnumber, but only succeed if the selected character matches #isdecimal, rather than #isnumeric") },

	{ "isanyprint",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_isanyprint,
	  DOC("(start=!0,end=!-1)->?Dbool\n"
	      "Returns :true if any character in "
	      "${this.substr(start,end)} is printable"),
	  TYPE_METHOD_FKWDS },
	{ "isanyalpha",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_isanyalpha,
	  DOC("(start=!0,end=!-1)->?Dbool\n"
	      "Returns :true if any character in "
	      "${this.substr(start,end)} is alphabetical"),
	  TYPE_METHOD_FKWDS },
	{ "isanyspace",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_isanyspace,
	  DOC("(start=!0,end=!-1)->?Dbool\n"
	      "Returns :true if any character in "
	      "${this.substr(start,end)} is a space character"),
	  TYPE_METHOD_FKWDS },
	{ "isanylf",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_isanylf,
	  DOC("(start=!0,end=!-1)->?Dbool\n"
	      "Returns :true if any character in "
	      "${this.substr(start,end)} is a line-feeds"),
	  TYPE_METHOD_FKWDS },
	{ "isanylower",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_isanylower,
	  DOC("(start=!0,end=!-1)->?Dbool\n"
	      "Returns :true if any character in "
	      "${this.substr(start,end)} is lower-case"),
	  TYPE_METHOD_FKWDS },
	{ "isanyupper",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_isanyupper,
	  DOC("(start=!0,end=!-1)->?Dbool\n"
	      "Returns :true if any character in "
	      "${this.substr(start,end)} is upper-case"),
	  TYPE_METHOD_FKWDS },
	{ "isanycntrl",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_isanycntrl,
	  DOC("(start=!0,end=!-1)->?Dbool\n"
	      "Returns :true if any character in "
	      "${this.substr(start,end)} is a control character"),
	  TYPE_METHOD_FKWDS },
	{ "isanydigit",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_isanydigit,
	  DOC("(start=!0,end=!-1)->?Dbool\n"
	      "Returns :true if any character in "
	      "${this.substr(start,end)} is a digit"),
	  TYPE_METHOD_FKWDS },
	{ "isanydecimal",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_isanydecimal,
	  DOC("(start=!0,end=!-1)->?Dbool\n"
	      "Returns :true if any character in "
	      "${this.substr(start,end)} is a dicimal character"),
	  TYPE_METHOD_FKWDS },
	{ "isanysymstrt",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_isanysymstrt,
	  DOC("(start=!0,end=!-1)->?Dbool\n"
	      "Returns :true if any character in "
	      "${this.substr(start,end)} can be used to start a symbol name"),
	  TYPE_METHOD_FKWDS },
	{ "isanysymcont",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_isanysymcont,
	  DOC("(start=!0,end=!-1)->?Dbool\n"
	      "Returns :true if any character in "
	      "${this.substr(start,end)} can be used to continue a symbol name"),
	  TYPE_METHOD_FKWDS },
	{ "isanyalnum",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_isanyalnum,
	  DOC("(start=!0,end=!-1)->?Dbool\n"
	      "Returns :true if any character in "
	      "${this.substr(start,end)} is alpha-numerical"),
	  TYPE_METHOD_FKWDS },
	{ "isanynumeric",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_isanynumeric,
	  DOC("(start=!0,end=!-1)->?Dbool\n"
	      "Returns :true if any character in "
	      "${this.substr(start,end)} qualifies as digit or decimal characters"),
	  TYPE_METHOD_FKWDS },
	{ "isanytitle",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_isanytitle,
	  DOC("(start=!0,end=!-1)->?Dbool\n"
	      "Returns :true if any character in "
	      "${this.substr(start,end)} has title-casing"),
	  TYPE_METHOD_FKWDS },
	{ "isanyascii",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_isanyascii,
	  DOC("(start=!0,end=!-1)->?Dbool\n"
	      "Returns :true if any character in ${this.substr(start,end)} is "
	      "an ascii character, that is has an ordinal value ${<= 0x7f}"),
	  TYPE_METHOD_FKWDS },

	/* String conversion */
	{ "lower",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_lower,
	  DOC("(start=!0,end=!-1)->?.\n"
	      "Returns @this string converted to lower-case"),
	  TYPE_METHOD_FKWDS },
	{ "upper",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_upper,
	  DOC("(start=!0,end=!-1)->?.\n"
	      "Returns @this string converted to upper-case"),
	  TYPE_METHOD_FKWDS },
	{ "title",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_title,
	  DOC("(start=!0,end=!-1)->?.\n"
	      "Returns @this string converted to title-casing"),
	  TYPE_METHOD_FKWDS },
	{ "capitalize",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_capitalize,
	  DOC("(start=!0,end=!-1)->?.\n"
	      "Returns @this string with each word capitalized"),
	  TYPE_METHOD_FKWDS },
	{ "swapcase",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_swapcase,
	  DOC("(start=!0,end=!-1)->?.\n"
	      "Returns @this string with the casing of each "
	      "character that has two different casings swapped"),
	  TYPE_METHOD_FKWDS },
	{ "casefold",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_casefold,
	  DOC("(start=!0,end=!-1)->?.\n"
	      "Returns @this string with its casing folded.\n"
	      "The equivalent of the string returned by this function is what is "
	      "used by the case-insensitive string APIs, such as #casefind\n"
	      "The case folding algorithm implemented matches what "
	      "%{link https://www.w3.org/International/wiki/Case_folding unicode} "
	      "describes as full case folding\n"
	      "At its core, case-folding a string is fairly similar to #lower, however "
	      "the differences start to appear when characters such as $\"\xC3\x9F\" are being "
	      "used, $\"\xC3\x9F\" being a german character that doesn't have a lower- or upper-case "
	      "variant, however in the absence of unicode support is often written as $\"ss\".\n"
	      "The obvious problem here is that this alternative representation uses 2 characters "
	      "where previously there was only one. #casefold solves this problem by replacing $\"\xC3\x9F\" "
	      "with $\"ss\", allowing functions such as #casecompare to indicate equal strings in "
	      "cases such as ${\"Stra\xC3\x9F"
	      "e\".casecompare(\"Strasse\") == 0}"),
	  TYPE_METHOD_FKWDS },

	/* Case-sensitive query functions */
	{ DeeString_STR(&str_replace),
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_replace,
	  DOC("(find:?.,replace:?.,max:?Dint=!A!Dint!PSIZE_MAX)->?.\n"
	      "Find up to @max occurrances of @find and replace each with @replace, then return the resulting string"),
	  TYPE_METHOD_FKWDS },
	{ "find",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_find,
	  DOC("(needle:?.,start=!0,end=!-1)->?Dint\n"
	      "Find the first instance of @needle within ${this.substr(start,end)}, "
	      "and return its starting index, or ${-1} if no such position exists"),
	  TYPE_METHOD_FKWDS },
	{ "rfind",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_rfind,
	  DOC("(needle:?.,start=!0,end=!-1)->?Dint\n"
	      "Find the last instance of @needle within ${this.substr(start,end)}, "
	      "and return its starting index, or ${-1} if no such position exists"),
	  TYPE_METHOD_FKWDS },
	{ "index",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_index,
	  DOC("(needle:?.,start=!0,end=!-1)->?Dint\n"
	      "@throw IndexError No instance of @needle can be found within ${this.substr(start,end)}\n"
	      "Find the first instance of @needle within ${this.substr(start,end)}, "
	      "and return its starting index"),
	  TYPE_METHOD_FKWDS },
	{ "rindex",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_rindex,
	  DOC("(needle:?.,start=!0,end=!-1)->?Dint\n"
	      "@throw IndexError No instance of @needle can be found within ${this.substr(start,end)}\n"
	      "Find the last instance of @needle within ${this.substr(start,end)}, "
	      "and return its starting index"),
	  TYPE_METHOD_FKWDS },
	{ "findall",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_findall,
	  DOC("(needle:?.,start=!0,end=!-1)->?S?Dint\n"
	      "Find all instances of @needle within ${this.substr(start,end)}, "
	      "and return their starting indeces as a sequence"),
	  TYPE_METHOD_FKWDS },
	{ "count",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_count,
	  DOC("(needle:?.,start=!0,end=!-1)->?Dint\n"
	      "Count the number of instances of @needle that exist within ${this.substr(start,end)}, "
	      "and return now many were found"),
	  TYPE_METHOD_FKWDS },
	{ "contains",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_contains_f,
	  DOC("(needle:?.,start=!0,end=!-1)->?Dbool\n"
	      "Check if @needle can be found within ${this.substr(start,end)}, and return a boolean indicative of that"),
	  TYPE_METHOD_FKWDS },
	{ "substr",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_substr,
	  DOC("(start=!0,end=!-1)->?.\n"
	      "Similar to ${this[start:end]}, however only integer-convertible objects may "
	      "be passed (passing :none will invoke ${(int)none}, which results in $0), and "
	      "passing negative values for either @start or @end will cause :int.SIZE_MAX to "
	      "be used for that argument:\n"
	      ">s = \"foo bar foobar\";\n"
	      ">print repr s.substr(0,1);    /* \"f\" */\n"
	      ">print repr s[0:1];           /* \"f\" */\n"
	      ">print repr s.substr(0,#s);   /* \"foo bar foobar\" */\n"
	      ">print repr s[0:#s];          /* \"foo bar foobar\" */\n"
	      ">print repr s.substr(0,1234); /* \"foo bar foobar\" */\n"
	      ">print repr s[0:1234];        /* \"foo bar foobar\" */\n"
	      ">print repr s.substr(0,-1);   /* \"foo bar foobar\" -- Negative indices intentionally underflow into positive infinity */\n"
	      ">print repr s[0:-1];          /* \"foo bar fooba\" */\n"
	      "Also note that this way of interpreting integer indices is mirrored by all other "
	      "string functions that allow start/end-style arguments, including #find, #compare, "
	      "as well as many others"),
	  TYPE_METHOD_FKWDS },
	{ "strip",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_strip,
	  DOC("(mask?:?.)->?.\n"
	      "Strip all leading and trailing whitespace-characters, or "
	      "characters apart of @mask, and return the resulting string") },
	{ "lstrip",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_lstrip,
	  DOC("(mask?:?.)->?.\n"
	      "Strip all leading whitespace-characters, or "
	      "characters apart of @mask, and return the resulting string") },
	{ "rstrip",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_rstrip,
	  DOC("(mask?:?.)->?.\n"
	      "Strip all trailing whitespace-characters, or "
	      "characters apart of @mask, and return the resulting string") },
	{ "sstrip",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_sstrip,
	  DOC("(needle:?.)->?.\n"
	      "Strip all leading and trailing instances of @needle from @this string\n"
	      ">local result = this;\n"
	      ">while (result.startswith(needle))\n"
	      "> result = result[#needle:];\n"
	      ">while (result.endswith(needle))\n"
	      "> result = result[:#result-#needle];") },
	{ "lsstrip",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_lsstrip,
	  DOC("(needle:?.)->?.\n"
	      "Strip all leading instances of @needle from @this string\n"
	      ">local result = this;\n"
	      ">while (result.startswith(needle))\n"
	      "> result = result[#needle:];") },
	{ "rsstrip",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_rsstrip,
	  DOC("(needle:?.)->?.\n"
	      "Strip all trailing instances of @needle from @this string\n"
	      ">local result = this;\n"
	      ">while (result.endswith(needle))\n"
	      "> result = result[:#result-#needle];") },
	{ "startswith",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_startswith,
	  DOC("(needle:?.,start=!0,end=!-1)->?Dbool\n"
	      "Return :true if the sub-string ${this.substr(start,end)} starts with @needle"),
	  TYPE_METHOD_FKWDS },
	{ "endswith",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_endswith,
	  DOC("(needle:?.,start=!0,end=!-1)->?Dbool\n"
	      "Return :true if the sub-string ${this.substr(start,end)} ends with @needle"),
	  TYPE_METHOD_FKWDS },
	{ "partition",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_parition,
	  DOC("(needle:?.,start=!0,end=!-1)->?T3?.?.?.\n"
	      "Search for the first instance of @needle within ${this.substr(start,end)} and "
	      "return a 3-element sequence of strings ${(this[:pos],needle,this[pos+#needle:])}.\n"
	      "If @needle could not be found, ${(this,\"\",\"\")} is returned"),
	  TYPE_METHOD_FKWDS },
	{ "rpartition",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_rparition,
	  DOC("(needle:?.,start=!0,end=!-1)->?T3?.?.?.\n"
	      "Search for the last instance of @needle within ${this.substr(start,end)} and "
	      "return a 3-element sequence of strings ${(this[:pos],needle,this[pos+#needle:])}.\n"
	      "If @needle could not be found, ${(this,\"\",\"\")} is returned"),
	  TYPE_METHOD_FKWDS },
	{ "compare",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_compare,
	  DOC("(other:?.,other_start=!0,other_end=!-1)->?Dint\n"
	      "(my_start:?Dint,other:?.,other_start=!0,other_end=!-1)->?Dint\n"
	      "(my_start:?Dint,my_end:?Dint,other:?.,other_start=!0,other_end=!-1)->?Dint\n"
	      "Compare the sub-string ${left = this.substr(my_start,my_end)} with ${right = other.substr(other_start,other_end)}, returning "
	      "${< 0} if ${left < right}, ${> 0} if ${left > right}, or ${== 0} if they are equal") },
	{ "vercompare",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_vercompare,
	  DOC("(other:?.,other_start=!0,other_end=!-1)->?Dint\n"
	      "(my_start:?Dint,other:?.,other_start=!0,other_end=!-1)->?Dint\n"
	      "(my_start:?Dint,my_end:?Dint,other:?.,other_start=!0,other_end=!-1)->?Dint\n"
	      "Performs a version-string comparison. This is similar to #compare, but rather than "
	      "performing a strict lexicographical comparison, the numbers found in the strings "
	      "being compared are comparsed as a whole, solving the common problem seen in applications "
	      "such as file navigators showing a file order of `foo1.txt', `foo10.txt', `foo11.txt', `foo2.txt', etc...\n"
	      "This function is a portable implementation of the GNU function "
	      "%{link https://linux.die.net/man/3/strverscmp strverscmp}, "
	      "for which you may follow the link for further details") },
	{ "wildcompare",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_wildcompare,
	  DOC("(pattern:?.,pattern_start=!0,pattern_end=!-1)->?Dint\n"
	      "(my_start:?Dint,pattern:?.,pattern_start=!0,pattern_end=!-1)->?Dint\n"
	      "(my_start:?Dint,my_end:?Dint,pattern:?.,pattern_start=!0,pattern_end=!-1)->?Dint\n"
	      "Perform a wild-character-enabled comparising of the sub-string ${left = this.substr(my_start,my_end)} "
	      "with ${right = pattern.substr(pattern_start,pattern_end)}, returning ${< 0} if ${left < right}, ${> 0} "
	      "if ${left > right}, or ${== 0} if they are equal\n"
	      "Wild-compare characters are only parsed from @pattern, allowing $\"?\" to "
	      "be matched with any single character from @this, and $\"*\" to be matched to "
	      "any number of characters") },
	{ "fuzzycompare",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_fuzzycompare,
	  DOC("(other:?.,other_start=!0,other_end=!-1)->?Dint\n"
	      "(my_start:?Dint,other:?.,other_start=!0,other_end=!-1)->?Dint\n"
	      "(my_start:?Dint,my_end:?Dint,other:?.,other_start=!0,other_end=!-1)->?Dint\n"
	      "Perform a fuzzy string comparison between ${this.substr(my_start,my_end)} and ${other.substr(other_start,other_end)}\n"
	      "The return value is a similarty-factor that can be used to score how close the two strings look alike.\n"
	      "How exactly the scoring is done is implementation-specific, however a score of $0 is reserved for two "
	      "strings that are perfectly identical, any two differing strings always have a score ${> 0}, and the closer "
	      "the score is to $0, the more alike they are\n"
	      "The intended use of this function is for auto-completion, as well as warning "
	      "messages and recommendations in the sense of I-dont-know-foo-but-did-you-mean-bar\n"
	      "Note that there is another version #casefuzzycompare that also ignores casing") },
	{ "wmatch",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_wmatch,
	  DOC("(pattern:?.,other_start=!0,other_end=!-1)->?Dbool\n"
	      "(my_start:?Dint,pattern:?.,other_start=!0,other_end=!-1)->?Dbool\n"
	      "(my_start:?Dint,my_end:?Dint,pattern:?.,other_start=!0,other_end=!-1)->?Dbool\n"
	      "Same as #wildcompare, returning :true where #wildcompare would return $0, and :false in all pattern cases") },

	/* Case-insensitive query functions */
	{ "casereplace",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_casereplace,
	  DOC("(find:?.,replace:?.,max:?Dint=!A!Dint!PSIZE_MAX)->?Dint\n"
	      "Same as #replace, however perform a case-folded search (s.a. #casefold)"),
	  TYPE_METHOD_FKWDS },
	{ "casefind",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_casefind,
	  DOC("(needle:?.,start=!0,end=!-1)->?X2?T2?Dint?Dint?N\n"
	      "Same as #find, however perform a case-folded search and return the start and end indices of the match (s.a. #casefold)\n"
	      "If no match if found, :none is returned"),
	  TYPE_METHOD_FKWDS },
	{ "caserfind",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_caserfind,
	  DOC("(needle:?.,start=!0,end=!-1)->?X2?T2?Dint?Dint?N\n"
	      "Same as #rfind, however perform a case-folded search and return the start and end indices of the match (s.a. #casefold)\n"
	      "If no match if found, :none is returned"),
	  TYPE_METHOD_FKWDS },
	{ "caseindex",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_caseindex,
	  DOC("(needle:?.,start=!0,end=!-1)->?T2?Dint?Dint\n"
	      "Same as #index, however perform a case-folded search and return the start and end indices of the match (s.a. #casefold)"),
	  TYPE_METHOD_FKWDS },
	{ "caserindex",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_caserindex,
	  DOC("(needle:?.,start=!0,end=!-1)->?T2?Dint?Dint\n"
	      "Same as #rindex, however perform a case-folded search and return the start and end indices of the match (s.a. #casefold)"),
	  TYPE_METHOD_FKWDS },
	{ "casefindall",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_casefindall,
	  DOC("(needle:?.,start=!0,end=!-1)->?S?T2?Dint?Dint\n"
	      "Same as #findall, however perform a case-folded search and return the star and end indices of matches (s.a. #casefold)"),
	  TYPE_METHOD_FKWDS },
	{ "casecount",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_casecount,
	  DOC("(needle:?.,start=!0,end=!-1)->?Dint\n"
	      "Same as #count, however perform a case-folded search (s.a. #casefold)"),
	  TYPE_METHOD_FKWDS },
	{ "casecontains",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_casecontains_f,
	  DOC("(needle:?.,start=!0,end=!-1)->?Dbool\n"
	      "Same as #contains, however perform a case-folded search (s.a. #casefold)"),
	  TYPE_METHOD_FKWDS },
	{ "casestrip",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_casestrip,
	  DOC("(mask?:?.)->?.\n"
	      "Same as #strip, however perform a case-folded search when @mask is given (s.a. #casefold)") },
	{ "caselstrip",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_caselstrip,
	  DOC("(mask?:?.)->?.\n"
	      "Same as #lstrip, however perform a case-folded search when @mask is given (s.a. #casefold)") },
	{ "caserstrip",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_caserstrip,
	  DOC("(mask?:?.)->?.\n"
	      "Same as #rstrip, however perform a case-folded search when @mask is given (s.a. #casefold)") },
	{ "casesstrip",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_casesstrip,
	  DOC("(needle:?.)->?.\n"
	      "Same as #sstrip, however perform a case-folded search (s.a. #casefold)") },
	{ "caselsstrip",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_caselsstrip,
	  DOC("(needle:?.)->?.\n"
	      "Same as #lsstrip, however perform a case-folded search (s.a. #casefold)") },
	{ "casersstrip",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_casersstrip,
	  DOC("(needle:?.)->?.\n"
	      "Same as #rsstrip, however perform a case-folded search (s.a. #casefold)") },
	{ "casestartswith",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_casestartswith,
	  DOC("(needle:?.,start=!0,end=!-1)->?Dbool\n"
	      "Same as #startswith, however perform a case-folded search (s.a. #casefold)"),
	  TYPE_METHOD_FKWDS },
	{ "caseendswith",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_caseendswith,
	  DOC("(needle:?.,start=!0,end=!-1)->?Dbool\n"
	      "Same as #endswith, however perform a case-folded search (s.a. #casefold)"),
	  TYPE_METHOD_FKWDS },
	{ "casepartition",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_caseparition,
	  DOC("(needle:?.,start=!0,end=!-1)->?T3?.?.?.\n"
	      "Same as #partition, however perform a case-folded search (s.a. #casefold)"),
	  TYPE_METHOD_FKWDS },
	{ "caserpartition",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_caserparition,
	  DOC("(needle:?.,start=!0,end=!-1)->?T3?.?.?.\n"
	      "Same as #rpartition, however perform a case-folded search (s.a. #casefold)"),
	  TYPE_METHOD_FKWDS },
	{ "casecompare",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_casecompare,
	  DOC("(other:?.,other_start=!0,other_end=!-1)->?Dint\n"
	      "(my_start:?Dint,other:?.,other_start=!0,other_end=!-1)->?Dint\n"
	      "(my_start:?Dint,my_end:?Dint,other:?.,other_start=!0,other_end=!-1)->?Dint\n"
	      "Same as #compare, however compare strings with their casing folded (s.a. #casefold)") },
	{ "casevercompare",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_casevercompare,
	  DOC("(other:?.,other_start=!0,other_end=!-1)->?Dint\n"
	      "(my_start:?Dint,other:?.,other_start=!0,other_end=!-1)->?Dint\n"
	      "(my_start:?Dint,my_end:?Dint,other:?.,other_start=!0,other_end=!-1)->?Dint\n"
	      "Same as #vercompare, however compare strings with their casing folded (s.a. #casefold)") },
	{ "casewildcompare",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_casewildcompare,
	  DOC("(pattern:?.,pattern_start=!0,pattern_end=!-1)->?Dint\n"
	      "(my_start:?Dint,pattern:?.,pattern_start=!0,pattern_end=!-1)->?Dint\n"
	      "(my_start:?Dint,my_end:?Dint,pattern:?.,pattern_start=!0,pattern_end=!-1)->?Dint\n"
	      "Same as #wildcompare, however compare strings with their casing folded (s.a. #casefold)") },
	{ "casefuzzycompare",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_casefuzzycompare,
	  DOC("(other:?.,other_start=!0,other_end=!-1)->?Dint\n"
	      "(my_start:?Dint,other:?.,other_start=!0,other_end=!-1)->?Dint\n"
	      "(my_start:?Dint,my_end:?Dint,other:?.,other_start=!0,other_end=!-1)->?Dint\n"
	      "Same as #fuzzycompare, however compare strings with their casing folded (s.a. #casefold)") },
	{ "casewmatch",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_casewmatch,
	  DOC("(pattern:?.,pattern_start=!0,pattern_end=!-1)->?Dbool\n"
	      "(my_start:?Dint,pattern:?.,pattern_start=!0,pattern_end=!-1)->?Dbool\n"
	      "(my_start:?Dint,my_end:?Dint,pattern:?.,pattern_start=!0,pattern_end=!-1)->?Dbool\n"
	      "Same as #wmatch, however compare strings with their casing folded (s.a. #casefold)") },

	/* String alignment functions. */
	{ "center",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_center,
	  DOC("(width:?Dint,filler=!P{ })->?.\n"
	      "Use @this string as result, then evenly insert @filler at "
	      "the front and back to pad its length to @width characters") },
	{ "ljust",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_ljust,
	  DOC("(width:?Dint,filler=!P{ })->?.\n"
	      "Use @this string as result, then insert @filler "
	      "at the back to pad its length to @width characters") },
	{ "rjust",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_rjust,
	  DOC("(width:?Dint,filler=!P{ })->?.\n"
	      "Use @this string as result, then insert @filler "
	      "at the front to pad its length to @width characters") },
	{ "zfill",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_zfill,
	  DOC("(width:?Dint,filler=!P{0})->?.\n"
	      "Skip leading ${\'+\'} and ${\'-\'} characters, then insert @filler "
	      "to pad the resulting string to a length of @width characters") },
	{ "reversed",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_reversed,
	  DOC("(start=!0,end=!-1)->?.\n"
	      "Return the sub-string ${this.substr(start,end)} with its character order reversed"),
	  TYPE_METHOD_FKWDS },
	{ "expandtabs",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_expandtabs,
	  DOC("(tabwidth=!8)->?.\n"
	      "Expand tab characters with whitespace offset from the start of "
	      "their respective line at multiples of @tabwidth") },
	{ "unifylines",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_unifylines,
	  DOC("(replacement=!P{\\\n})->?.\n"
	      "Unify all linefeed character sequences found in @this string to "
	      "make exclusive use of @replacement") },

	/* String -- sequence interaction. */
	{ "join",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_join,
	  DOC("(seq:?S?O)->?.\n"
	      "Iterate @seq and convert all items into string, inserting @this "
	      "string before each element, starting only with the second") },
	{ "split",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_split,
	  DOC("(sep:?.)->?S?.\n"
	      "Split @this string at each instance of @sep, returning a sequence of the resulting parts") },
	{ "casesplit",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_casesplit,
	  DOC("(sep:?.)->?S?.\n"
	      "Same as #split, however perform a case-folded search") },
	{ "splitlines",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_splitlines,
	  DOC("(keepends=!f)->?S?.\n"
	      "Split @this string at each linefeed, returning a sequence of all contained lines\n"
	      "When @keepends is :false, this is identical to ${this.unifylines().split(\"\\n\")}\n"
	      "When @keepends is :true, items found in the returned sequence will still have their "
	      "original, trailing line-feed appended") },

	/* String indentation. */
	{ "indent",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_indent,
	  DOC("(filler=!P{\t})->?.\n"
	      "Using @this string as result, insert @filler at the front, as well as after "
	      "every linefeed with the exception of one that may be located at its end\n"
	      "The inteded use is for generating strings from structured data, such as HTML:\n"
	      ">text = get_html();\n"
	      ">text = \"<html>\n{}\n</html>\".format({ text.strip().indent() });") },
	{ "dedent",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_dedent,
	  DOC("(max_chars=!1,mask?:?.)->?.\n"
	      "Using @this string as result, remove up to @max_chars whitespace "
	      "(s.a. #isspace) characters, or if given: characters apart of @mask "
	      "from the front, as well as following any linefeed") },

	/* Common-character search functions. */
	{ "common",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_common,
	  DOC("(other:?.,other_start=!0,other_end=!-1)->?Dint\n"
	      "(my_start:?Dint,other:?.,other_start=!0,other_end=!-1)->?Dint\n"
	      "(my_start:?Dint,my_end:?Dint,other:?.,other_start=!0,other_end=!-1)->?Dint\n"
	      "Returns the number of common leading characters shared between @this and @other, "
	      "or in other words: the lowest index $i for which ${this[i] != other[i]} is true") },
	{ "rcommon",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_rcommon,
	  DOC("(other:?.,other_start=!0,other_end=!-1)->?Dint\n"
	      "(my_start:?Dint,other:?.,other_start=!0,other_end=!-1)->?Dint\n"
	      "(my_start:?Dint,my_end:?Dint,other:?.,other_start=!0,other_end=!-1)->?Dint\n"
	      "Returns the number of common trailing characters shared between @this and @other") },
	{ "casecommon",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_casecommon,
	  DOC("(other:?.,other_start=!0,other_end=!-1)->?Dint\n"
	      "(my_start:?Dint,other:?.,other_start=!0,other_end=!-1)->?Dint\n"
	      "(my_start:?Dint,my_end:?Dint,other:?.,other_start=!0,other_end=!-1)->?Dint\n"
	      "Same as #common, however perform a case-folded search") },
	{ "casercommon",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_casercommon,
	  DOC("(other:?.,other_start=!0,other_end=!-1)->?Dint\n"
	      "(my_start:?Dint,other:?.,other_start=!0,other_end=!-1)->?Dint\n"
	      "(my_start:?Dint,my_end:?Dint,other:?.,other_start=!0,other_end=!-1)->?Dint\n"
	      "Same as #rcommon, however perform a case-folded search") },

	/* Find match character sequences */
	{ "findmatch",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_findmatch,
	  DOC("(open:?.,close:?.,start=!0,end=!-1)->?Dint\n"
	      "Similar to #find, but do a recursive search for the "
	      "first @close that doesn't have a match @{open}:\n"
	      ">s = \"foo(bar(),baz(42),7).strip()\";\n"
	      ">lcol = s.find(\"(\");\n"
	      ">print lcol; /* 3 */\n"
	      ">mtch = s.findmatch(\"(\",\")\",lcol+1);\n"
	      ">print repr s[lcol:mtch+1]; /* \"(bar(),baz(42),7)\" */\n"
	      "If no @close without a match @open exists, ${-1} is returned\n"
	      "Note that @open and @close are not restricted to single-character "
	      "strings, are allowed to be of any length") },
	{ "indexmatch",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_indexmatch,
	  DOC("(open:?.,close:?.,start=!0,end=!-1)->?Dint\n"
	      "@throw IndexError No instance of @close without a match @open exists within ${this.substr(start,end)}\n"
	      "Same as #findmatch, but throw an :IndexError instead of "
	      "returning ${-1} if no @close without a match @open exists") },
	{ "casefindmatch",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_casefindmatch,
	  DOC("(open:?.,close:?.,start=!0,end=!-1)->?X2?T2?Dint?Dint?N\n"
	      "Same as #findmatch, however perform a case-folded search and return the start and end indices of the match\n"
	      "If no match if found, :none is returned") },
	{ "caseindexmatch",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_caseindexmatch,
	  DOC("(open:?.,close:?.,start=!0,end=!-1)->?T2?Dint?Dint\n"
	      "@throw IndexError No instance of @close without a match @open exists within ${this.substr(start,end)}\n"
	      "Same as #indexmatch, however perform a case-folded search and return the start and end indices of the match") },
	{ "rfindmatch",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_rfindmatch,
	  DOC("(open:?.,close:?.,start=!0,end=!-1)->?Dint\n"
	      "Similar to #findmatch, but operate in a mirrored fashion, searching for the "
	      "last instance of @open that has no match @close within ${this.substr(start,end)}:\n"
	      ">s = \"get_string().foo(bar(),baz(42),7).length\";\n"
	      ">lcol = s.find(\")\");\n"
	      ">print lcol; /* 19 */\n"
	      ">mtch = s.rfindmatch(\"(\",\")\",0,lcol);\n"
	      ">print repr s[mtch:lcol+1]; /* \"(bar(),baz(42),7)\" */\n"
	      "If no @open without a match @close exists, ${-1} is returned") },
	{ "rindexmatch",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_rindexmatch,
	  DOC("(open:?.,close:?.,start=!0,end=!-1)->?Dint\n"
	      "@throw IndexError No instance of @open without a match @close exists within ${this.substr(start,end)}\n"
	      "Same as #rfindmatch, but throw an :IndexError instead of "
	      "returning ${-1} if no @open without a match @close exists") },
	{ "caserfindmatch",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_caserfindmatch,
	  DOC("(open:?.,close:?.,start=!0,end=!-1)->?X2?T2?Dint?Dint?N\n"
	      "Same as #rfindmatch, however perform a case-folded search and return the start and end indices of the match\n"
	      "If no match if found, :none is returned") },
	{ "caserindexmatch",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_caserindexmatch,
	  DOC("(open:?.,close:?.,start=!0,end=!-1)->?T2?Dint?Dint\n"
	      "@throw IndexError No instance of @open without a match @close exists within ${this.substr(start,end)}\n"
	      "Same as #rindexmatch, however perform a case-folded search and return the start and end indices of the match") },

	/* Using the find-match functionality, also provide a partitioning version */
	{ "partitionmatch",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_partitionmatch,
	  DOC("(open:?.,close:?.,start=!0,end=!-1)->?T3?.?.?.\n"
	      "A hybrid between #find, #findmatch and #partition that returns the strings surrounding "
	      "the matched string portion, the first being the substring prior to the match, "
	      "the second being the matched string itself (including the @open and @close strings), "
	      "and the third being the substring after the match:\n"
	      ">s = \"foo {x,y,{13,19,42,{}},w} -- tail {}\";\n"
	      ">print repr s.partitionmatch(\"{\",\"\"); /* { \"foo \", \"{x,y,{13,19,42,{}},w}\", \" -- tail {}\" } */\n"
	      "If no matching @open + @close pair could be found, ${(this[start:end],\"\",\"\")} is returned\n"
	      ">function partitionmatch(open, close, start = 0, end = -1) {\n"
	      "> local j;\n"
	      "> local i = this.find(open,start,end);\n"
	      "> if (i < 0 || (j = this.findmatch(open,close,i+#open,end)) < 0)\n"
	      ">  return (this.substr(start,end),\"\",\"\");\n"
	      "> return (this.substr(start,i),this.substr(i,j+#close),this.substr(j+#close,end))\n"
	      ">}") },
	{ "rpartitionmatch",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_rpartitionmatch,
	  DOC("(open:?.,close:?.,start=!0,end=!-1)->?T3?.?.?.\n"
	      "A hybrid between #rfind, #rfindmatch and #rpartition that returns the strings surrounding "
	      "the matched string portion, the first being the substring prior to the match, "
	      "the second being the matched string itself (including the @open and @close strings), "
	      "and the third being the substring after the match:\n"
	      ">s = \"{} foo {x,y,{13,19,42,{}},w} -- tail\";\n"
	      ">print repr s.rpartitionmatch(\"{\",\"\"); /* { \"{} foo \", \"{x,y,{13,19,42,{}},w}\", \" -- tail\" } */\n"
	      "If no matching @open + @close pair could be found, ${(this[start:end],\"\",\"\")} is returned\n"
	      ">function rpartitionmatch(open, close, start = 0, end = -1) {\n"
	      "> local i;\n"
	      "> local j = this.rfind(close,start,end);\n"
	      "> if (j < 0 || (i = this.rfindmatch(open,close,start,j)) < 0)\n"
	      ">  return (this.substr(start,end),\"\",\"\");\n"
	      "> return (this.substr(start,i),this.substr(i,j+#close),this.substr(j+#close,end))\n"
	      ">}") },
	{ "casepartitionmatch",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_casepartitionmatch,
	  DOC("(open:?.,close:?.,start=!0,end=!-1)->?T3?.?.?.\n"
	      "Same as #partitionmatch, however perform a case-folded search") },
	{ "caserpartitionmatch",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_caserpartitionmatch,
	  DOC("(open:?.,close:?.,start=!0,end=!-1)->?T3?.?.?.\n"
	      "Same as #rpartitionmatch, however perform a case-folded search") },

	{ "segments",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_segments,
	  DOC("(substring_length:?Dint)->?S?.\n"
	      "Split @this string into segments, each exactly @substring_length characters long, with the "
	      "last segment containing the remaining characters and having a length of between "
	      "$1 and @substring_length characters.\n"
	      "This function is similar to #distribute, but instead of being given the "
	      "length of sub-strings and figuring out their amount, this function takes "
	      "the amount of sub-strings and figures out their lengths") },
	{ "distribute",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_distribute,
	  DOC("(substring_count:?Dint)->?S?.\n"
	      "Split @this string into @substring_count similarly sized sub-strings, each with a "
	      "length of ${(#this + (substring_count - 1)) / substring_count}, followed by a last, optional "
	      "sub-string containing all remaining characters.\n"
	      "This function is similar to #segments, but instead of being given the "
	      "amount of sub-strings and figuring out their lengths, this function takes "
	      "the length of sub-strings and figures out their amount") },

	/* Regex functions. */
	{ "rematch",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_rematch,
	  DOC("(pattern:?.,start=!0,end=!-1,rules=!P{})->?Dint\n"
	      "(pattern:?.,rules:?.,start=!0,end=!-1)->?Dint\n"
	      "@throw ValueError The given @pattern is malformed\n"
	      "@return The number of leading characters in ${this.substr(start,end)} "
	      "matched by @pattern, or $0 if @pattern could not be fully matched\n"
	      "Check if ${this.substr(start,end)} string matches the given regular expression @pattern\n"
	      "When specified, @rules must be a comma-separated and case-insensitive string "
	      "consisting of a set of the following Name-options, or a tightly packed set of the "
	      "following Short-options:\n"
	      "%{table Name|Short|Inline|Description\n"
	      "$\"DOTALL\"|$\"s\"|$\"s\"|The $\".\" regex meta-character matches anything (including new-lines, which otherwise wouldn't be matched)\n"
	      "$\"MULTILINE\"|$\"m\"|$\"m\"|Allow $\"^\" to match immediately after a line-feed, rather than just at the start of the string\n"
	      "$\"NOCASE\"|$\"i\"|$\"i\"|Ignore casing when matching single characters, as well as characters in ranges (e.g.: $\"[a-z]\")}\n"
	      "The builtin regular expression API for strings spans across the following functions:\n"
	      "%{table Function|Non-regex Variant|Description\n"
	      "#rematch|#common|Count how many character at the start of a sub-string match a regex pattern\n"
	      "#rematches|#equal|Check that a given sub-string match the regex pattern exactly\n"
	      "#refind|#find|Find the first sub-range matched by a regex pattern\n"
	      "#rerfind|#rfind|Find the last sub-range matched by a regex pattern\n"
	      "#reindex|#index|Same as #refind, but throws an error if not found\n"
	      "#rerindex|#rindex|Same as #rerfind, but throws an error if not found\n"
	      "#relocate|-|Same as #refind, but return the sub-string that was matched, rather than its indices\n"
	      "#rerlocate|-|Same as #rerfind, but return the sub-string that was matched, rather than its indices\n"
	      "#repartition|#partition|Same as #relocate, but return a 3-tuple of strings (before_match,match,after_match)\n"
	      "#rerpartition|#rpartition|Same as #rerlocate, but return a 3-tuple of strings (before_match,match,after_match)\n"
	      "#rereplace|#replace|Find and replace all sub-ranges matched by a regex pattern with a different string\n"
	      "#refindall|#findall|Enumerate all sub-ranges matched by a regex pattern in ascending order\n"
	      "#relocateall|-|Enumerate all sub-strings matched by a regex pattern in ascending order\n"
	      "#resplit|#split|Enumerate all sub-strings matched by a regex pattern in ascending order\n"
	      "#restartswith|#startswith|Check if @this string starts with a regular expression\n"
	      "#reendswith|#endswith|Check if @this string ends with a regular expression\n"
	      "#recontains|#contains|Check if @this stirng contains a regular expression anywhere\n"
	      "#recount|#count|Count the number of occurances of a regular expression\n"
	      "#restrip|#strip|Strip all leading and trailing regular expression matches\n"
	      "#relstrip|#lstrip|Strip all leading regular expression matches\n"
	      "#rerstrip|#rstrip|Strip all trailing regular expression matches\n"
	      "}\n"
	      "Deemon implements support for the following regex matching functions:\n"
	      "%{table Feature|Description\n"
	      "${r\".\"}|Match anything except for newlines. When $\"DOTALL\" is enabled, match anything including newlines\n"
	      "${r\"^\"}|Match at the start of the string. When $\"MULTILINE\", also match at the start of lines (${r\"(?<=\\A|\\n)\"})\n"
	      "${r\"$\"}|Match at the end of the string. When $\"MULTILINE\", also match at the end of lines (${r\"(?=\\Z|\\n)\"})\n"
	      "${r\"\\A\"}|Match only at the start of the string\n"
	      "${r\"\\Z\"}|Match only at the end of the string\n"
	      "${r\"(x...|y...)\"}|Match either `x...' or `y...' (Note special behavior when repeated)\n"
	      "${r\"(?...)\"}|Regex extension (see below)\n"
	      "${r\"[...]\"}|Match any character apart of `...' (also accept the `a-z' notation, as well as any of the ${r\"\\...\"}) functions below\n"
	      "${r\"\\d\"}|Match any character $ch with ${ch.isdigit()}\n"
	      "${r\"\\D\"}|Match any character $ch with ${!ch.isdigit()}\n"
	      "${r\"\\s\"}|Match any character $ch with ${ch.isspace()}\n"
	      "${r\"\\S\"}|Match any character $ch with ${!ch.isspace()}\n"
	      "${r\"\\w\"}|Match any character $ch with ${ch.issymstrt() || ch.issymcont()}\n"
	      "${r\"\\W\"}|Match any character $ch with ${!ch.issymstrt() && !ch.issymcont()}\n"
	      "${r\"\\n\"}|Match any character $ch with ${ch.islf()} (NOTE: deemon-specific extension)\n"
	      "${r\"\\N\"}|Match any character $ch with ${!ch.islf()} (NOTE: deemon-specific extension)\n"
	      "${r\"\\a\"}|Match the character ${string.chr(0x07)} aka $\"\\a\"\n"
	      "${r\"\\b\"}|Match the character ${string.chr(0x08)} aka $\"\\b\"\n"
	      "${r\"\\f\"}|Match the character ${string.chr(0x0c)} aka $\"\\f\"\n"
	      "${r\"\\r\"}|Match the character ${string.chr(0x0d)} aka $\"\\r\"\n"
	      "${r\"\\t\"}|Match the character ${string.chr(0x09)} aka $\"\\t\"\n"
	      "${r\"\\v\"}|Match the character ${string.chr(0x0b)} aka $\"\\v\"\n"
	      "${r\"\\e\"}|Match the character ${string.chr(0x1b)} aka $\"\\e\"\n"
	      "${r\"\\[...]\"}|Match a character via its explicit unicode traits (see below)\n"
	      "${r\"\\...\"}|For anything else, match `...' exactly\n"
	      "${r\"...\"}|Match the given character `...' exactly}\n"
	      "Deemon implements support for the following regex repetition suffixes:\n"
	      "%{table Suffix|Min|Max|Greedy\n"
	      "${r\"*\"}|$0|$INF|$true\n"
	      "${r\"*?\"}|$0|$INF|$false\n"
	      "${r\"+\"}|$1|$INF|$true\n"
	      "${r\"+?\"}|$1|$INF|$false\n"
	      "${r\"?\"}|$0|$1|$true\n"
	      "${r\"??\"}|$0|$1|$false\n"
	      "${r\"{m}\"}|$m|$m|$true\n"
	      "${r\"{m}?\"}|$m|$m|$false\n"
	      "${r\"{,n}\"}|$0|$n|$true\n"
	      "${r\"{,n}?\"}|$0|$n|$false\n"
	      "${r\"{m,}\"}|$m|$INF|$true\n"
	      "${r\"{m,}?\"}|$m|$INF|$false\n"
	      "${r\"{m,n}\"}|$m|$n|$true\n"
	      "${r\"{m,n}?\"}|$m|$n|$false}\n"
	      "Deemon implements support for the following regex extensions (which also include some deemon-specific ones):\n"
	      "%{table Extension|Description\n"
	      "${r\"(?<=...)\"}|Positive look-behind assertion (Ensure that the current data position is preceded by `...')\n"
	      "${r\"(?<!...)\"}|Negative look-behind assertion (Ensure that the current data position isn't preceded by `...')\n"
	      "${r\"(?=...)\"}|Positive look-ahead assertion (Ensure that `...' follows the current data position)\n"
	      "${r\"(?!...)\"}|Negative look-ahead assertion (Ensure that `...' doesn't follow the current data position)\n"
	      "${r\"(?~ims)\"}|Set/delete regex flags (each character corresponding to the `Inline' column above). "
	      "Each rule may be prefixed by `~' in order to unset that flag. e.g. $\"(?i~m)\" or $\"(?~mi~s)\"\n"
	      "${r\"(?#...)\"}|Comment\n"
	      "${r\"(?\\d=7)\"}|Match any unicode digit-like character with a value equal to $7\n"
	      "${r\"(?\\d>2)\"}, ${r\"(?\\d>=3)\"}|Match any unicode digit-like character with a value of at least $3\n"
	      "${r\"(?\\d<7)\"}, ${r\"(?\\d<=6)\"}|Match any unicode digit-like character with a value of at most $6\n"
	      "${r\"(?\\d>=2<=6)\"}, ${r\"(?\\d<=6>=2)\"}, ${r\"(?\\d>1<7)\"}, ${r\"(?\\d<7>1)\"}|Match any unicode digit-like character with a value between $2 and $6}\n"
	      "As an extension, deemon implements unicode trait-based character matching "
	      "through ${r\"\\[...]\"}, where ${r\"...\"} is encoded as follows, where TRAITS "
	      "is a tightly packed string of characters acting as flags, which are described further below:\n"
	      "%{table Encoding|Description\n"
	      "${r\"\\[TRAITS]\"}|Match characters implementing all of the given TRAITS (see below)\n"
	      "${r\"\\[+TRAITS]\"}|Match characters implementing any of the given TRAITS (see below)\n"
	      "${r\"\\[^TRAITS]\"}|Match characters implementing none of the given TRAITS (see below)\n"
	      "${r\"\\[WANTED-UNWANTED]\"}|Match characters implementing all of WANTED, but none of UNWANTED (see below)}\n"
	      "%{table Trait-Character|Function|Description\n"
	      "${\"p\"}|#isprint|Is the character printable or SPC\n"
	      "${\"a\"}|#isalpha|Is the character alphabetic\n"
	      "${\"c\"}|#isspace|Is the character a space-character\n"
	      "${\"n\"}|#islf|Is the character a line-feed (Warning: Unlike ${r\"\\n\"}, doesn't match $\"\\r\\n\" as a single line-feed)\n"
	      "${\"l\"}|#islower|Is the character lower-case\n"
	      "${\"u\"}|#isupper|Is the character upper-case\n"
	      "${\"t\"}|#istitle|Is the character title-case\n"
	      "${\"c\"}|#iscntrl|Is the character a control character\n"
	      "${\"d\"}|#isdecimal|Is the character a decimal (e.g. $\"2\" (two))\n"
	      "${\"D\"}|#isdigit|Is the character a digit (e.g. $\"²\" (square))\n"
	      "${\"0\"}|#issymstrt|Is the character allowed as start of a symbol\n"
	      "${\"1\"}|#issymcont|Is the character allowed as continuation of a symbol}\n"
	      "%{table Example pattern|Description\n"
	      "${text.relocateall(r\"\\[0]\\[1]*\")}|Locate all unicode-compliant symbol names\n"
	      "${text.relocateall(r\"(?<!\\[1])\\[d]\\[1]*\")}|Locate all c-compliant integer literal ($\"12\", $\"0x12\" or $\"0x12ul\")\n"
	      "${text.relocateall(r\"(?<!\\[1])\\[u]\\[l]*(?!\\[0])\")}|Locate all words starting with an uppcase letter\n"
	      "${text.relocateall(r\'\"(\\.|.)*\"\')}|Locate all c-compliant string constants\n"
	      "${text.relocateall(r\"(?s)/\\*.*\\*/\")}|Locate all c-like comments}\n"
	      "In order to improve performance for repetitious data, deemon's regex implementation "
	      "has a minor quirk when it comes to groups containing $\"|\" to indicate multiple variants "
	      "in situations where that group is repeated more than once.\n"
	      "If one of the variants is found to match the data string, instead of restarting from the "
	      "beginning when checking if the variant matches more than once, the variant already matched "
	      "(then called the primary variant) is re-tried first. Only if it cannot be used to match "
	      "another data block will other variants be tried again in ascending order, but skipping the "
	      "old primary variant. If one of the secondary variants then ends up being a match, it will "
	      "become the new primary variant, and the process is repeated.\n"
	      "This design decision was done on purpose in order to more efficiently match repetitious data, "
	      "however when presented with multiple variants in a repeating context, it will not always be "
	      "the first variant that gets matched.\n"
	      ">local data = \"bar,  foobar\";\n"
	      ">/* When faced by multiple variants, deemon will always\n"
	      "> * preferr a variant that matched before the current:\n"
	      "> *  - Available variants: r\",\", r\" f\", r\" \"\n"
	      "> *  - ...\n"
	      "> *  - Try variant #0\n"
	      "> *    Match: \"bar,  foobar\"\n"
	      "> *               ^ Variant #0\n"
	      "> *    Remaining data: \"  foobar\"\n"
	      "> *    Set primary variant: #0\n"
	      "> *  - Try variant #0\n"
	      "> *    Missmatch (Primary variant)\n"
	      "> *    Search for new primary variant\n"
	      "> *  - Try variant #1\n"
	      "> *    Missmatch (\"  foobar\".startswith(\" f\") == false)\n"
	      "> *  - Try variant #2\n"
	      "> *    Match\n"
	      "> *    Set primary variant: #3\n"
	      "> *    Remaining data: \" foobar\"\n"
	      "> *  - Try variant #2\n"
	      "> *    Match\n"
	      "> *    Remaining data: \"foobar\"\n"
	      "> *  - Try variant #2\n"
	      "> *    Missmatch (Primary variant)\n"
	      "> *    Search for new primary variant\n"
	      "> *  - ...\n"
	      "> * -> At this point, none of the variants continue to match\n"
	      "> *    and the resulting match-range doesn't include the `f'\n"
	      "> *    because variant #1 (despite having a higher precedence\n"
	      "> *    than variant #2) was never checked for \" foobar\"\n"
	      "> * -> This quirk can only ever be a problem in badly written\n"
	      "> *    expressions that contain multiple variants where a variant\n"
	      "> *    with a lower index starts with the same condition as another\n"
	      "> *    with a greater index\n"
	      "> *    This example should really be written as r\"(,| f?)+\", which\n"
	      "> *    wold never even run into this problem in the first place. */\n"
	      ">print repr data.relocateall(r\"(,| f| )+\"); /* { \",  \" } */\n"
	      ">\n"
	      ">/* This regular expression doesn't have the quirk due to the double-parenthesis, \n"
	      "> * which leaves the outter (repeating) group to only have 1 variant, which it will\n"
	      "> * then always execute linearly, meaning that for each repetition, `,' is checked\n"
	      "> * first, followed by ` f', and finally ` '\n"
	      "> *  - ...\n"
	      "> *  - Try variant #0\n"
	      "> *    - Try variant #0.1\n"
	      "> *    - Try variant #0.2\n"
	      "> *    - Try variant #0.3\n"
	      "> *  - ...\n"
	      "> * -> Since The repeating group has only 1 variant, that variant\n"
	      "> *    is always the primary one, and checked as a whole when being\n"
	      "> *    repeated */\n"
	      ">print repr data.relocateall(r\"((,| f| ))+\"); /* { \",  f\" } */") },
	{ "rematches",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_rematches,
	  DOC("(pattern:?.,start=!0,end=!-1,rules=!P{})->?Dbool\n"
	      "(pattern:?.,rules:?.,start=!0,end=!-1)->?Dbool\n"
	      "@param pattern The regular expression patterm (s.a. #rematch)\n"
	      "@param rules The regular expression rules (s.a. #rematch)\n"
	      "@throw ValueError The given @pattern is malformed\n"
	      "Check if @pattern matches the entirety of the specified range of @this string\n"
	      "This function behaves identical to ${this.rematch(...) == #this}") },
	{ "refind",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_refind,
	  DOC("(pattern:?.,start=!0,end=!-1,rules=!P{})->?X2?T2?Dint?Dint?N\n"
	      "(pattern:?.,rules:?.,start=!0,end=!-1)->?X2?T2?Dint?Dint?N\n"
	      "@param pattern The regular expression patterm (s.a. #rematch)\n"
	      "@param rules The regular expression rules (s.a. #rematch)\n"
	      "@throw ValueError The given @pattern is malformed\n"
	      "Find the first sub-string matched by @pattern, and return its start/end indices, or :none if no match exists\n"
	      "Note that using :none in an expand expression will result in whatever number of targets are required:\n"
	      ">/* If the pattern count not be matched, both `start' and `end' will be `none' singleton */\n"
	      ">local start,end = data.refind(r\"\\b\\w\\b\")...;\n"
	      ">/* Since `none' is equal to `0' when casted to `int', calling `substr' with none-arguments\n"
	      "> * is the same as calling `data.substr(0,0)', meaning that this an empty string will be\n"
	      "> * returned when `refind' didn't manage to find anything.\n"
	      "> * Note however that `operator [:]' functions differently, as it interprets `none' as a\n"
	      "> * placeholder for either `0' of `#data', so calling `data[start:end]' would re-produce\n"
	      "> * `data' itself in the event of `refind' having failed. */\n"
	      ">print repr data.substr(start,end);") },
	{ "rerfind",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_rerfind,
	  DOC("(pattern:?.,start=!0,end=!-1,rules=!P{})->?X2?T2?Dint?Dint?N\n"
	      "(pattern:?.,rules:?.,start=!0,end=!-1)->?X2?T2?Dint?Dint?N\n"
	      "@param pattern The regular expression patterm (s.a. #rematch)\n"
	      "@param rules The regular expression rules (s.a. #rematch)\n"
	      "@throw ValueError The given @pattern is malformed\n"
	      "Find the last sub-string matched by @pattern, and return its start/end indices, or :none if no match exists (s.a. #refind)") },
	{ "reindex",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_reindex,
	  DOC("(pattern:?.,start=!0,end=!-1,rules=!P{})->?T2?Dint?Dint\n"
	      "(pattern:?.,rules:?.,start=!0,end=!-1)->?T2?Dint?Dint\n"
	      "@param pattern The regular expression patterm (s.a. #rematch)\n"
	      "@param rules The regular expression rules (s.a. #rematch)\n"
	      "@throw ValueError The given @pattern is malformed\n"
	      "@throw IndexError No substring matching the given @pattern could be found\n"
	      "Same as #refind, but throw an :IndexError when no match can be found") },
	{ "rerindex",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_rerindex,
	  DOC("(pattern:?.,start=!0,end=!-1,rules=!P{})->?T2?Dint?Dint\n"
	      "(pattern:?.,rules:?.,start=!0,end=!-1)->?T2?Dint?Dint\n"
	      "@param pattern The regular expression patterm (s.a. #rematch)\n"
	      "@param rules The regular expression rules (s.a. #rematch)\n"
	      "@throw ValueError The given @pattern is malformed\n"
	      "@throw IndexError No substring matching the given @pattern could be found\n"
	      "Same as #rerfind, but throw an :IndexError when no match can be found") },
	{ "relocate",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_relocate,
	  DOC("(pattern:?.,start=!0,end=!-1,rules=!P{})->?.\n"
	      "(pattern:?.,rules:?.,start=!0,end=!-1)->?.\n"
	      "@param pattern The regular expression patterm (s.a. #rematch)\n"
	      "@param rules The regular expression rules (s.a. #rematch)\n"
	      "@throw ValueError The given @pattern is malformed\n"
	      "Same as ${this.substr(this.refind(pattern,start,end,rules)...)}\n"
	      "In other words: return the first sub-string matched by the "
	      "given regular expression, or an empty string if not found\n"
	      "This function has nothing to do with relocations! - it's pronounced R.E. locate") },
	{ "rerlocate",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_rerlocate,
	  DOC("(pattern:?.,start=!0,end=!-1,rules=!P{})->?.\n"
	      "(pattern:?.,rules:?.,start=!0,end=!-1)->?.\n"
	      "@param pattern The regular expression patterm (s.a. #rematch)\n"
	      "@param rules The regular expression rules (s.a. #rematch)\n"
	      "@throw ValueError The given @pattern is malformed\n"
	      "Same as ${this.substr(this.rerfind(pattern,start,end,rules)...)}\n"
	      "In other words: return the last sub-string matched by the "
	      "given regular expression, or an empty string if not found") },
	{ "repartition",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_repartition,
	  DOC("(pattern:?.,start=!0,end=!-1,rules=!P{})->?T3?.?.?.\n"
	      "(pattern:?.,rules:?.,start=!0,end=!-1)->?T3?.?.?.\n"
	      "@param pattern The regular expression patterm (s.a. #rematch)\n"
	      "@param rules The regular expression rules (s.a. #rematch)\n"
	      "@throw ValueError The given @pattern is malformed\n"
	      "A hybrid between #refind and #partition\n"
	      ">function repartition(pattern,start,end,rules) {\n"
	      "> local start,end = this.refind(pattern,start,end,rules)...;\n"
	      "> if (start is none) return (this,\"\",\"\");\n"
	      "> return (this.substr(0,start),this.substr(start,end),this.substr(end,-1));\n"
	      ">}") },
	{ "rerpartition",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_rerpartition,
	  DOC("(pattern:?.,start=!0,end=!-1,rules=!P{})->?T3?.?.?.\n"
	      "(pattern:?.,rules:?.,start=!0,end=!-1)->?T3?.?.?.\n"
	      "@param pattern The regular expression patterm (s.a. #rematch)\n"
	      "@param rules The regular expression rules (s.a. #rematch)\n"
	      "@throw ValueError The given @pattern is malformed\n"
	      "A hybrid between #rrefind and #rpartition\n"
	      ">function rerpartition(pattern,start,end,rules) {\n"
	      "> local start,end = this.rerfind(pattern,start,end,rules)...;\n"
	      "> if (start is none) return (this,\"\",\"\");\n"
	      "> return (this.substr(0,start),this.substr(start,end),this.substr(end,-1));\n"
	      ">}") },
	{ "rereplace",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_rereplace,
	  DOC("(pattern:?.,replace_str:?.,max_count:?Dint=!A!Dint!PSIZE_MAX,rules=!P{})->?.\n"
	      "(pattern:?.,replace_str:?.,rules=!P{},max_count:?Dint=!A!Dint!PSIZE_MAX)->?.\n"
	      "@param pattern The regular expression patterm (s.a. #rematch)\n"
	      "@param rules The regular expression rules (s.a. #rematch)\n"
	      "@throw ValueError The given @pattern is malformed\n"
	      "Similar to #replace, however the string to search for is implemented as a regular expression "
	      "pattern, with the sub-string matched by it then getting replaced by @replace_str") },
	{ "refindall",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_refindall,
	  DOC("(pattern:?.,start=!0,end=!-1,rules=!P{})->?S?T2?Dint?Dint\n"
	      "(pattern:?.,rules:?.,start=!0,end=!-1)->?S?T2?Dint?Dint\n"
	      "@param pattern The regular expression patterm (s.a. #rematch)\n"
	      "@param rules The regular expression rules (s.a. #rematch)\n"
	      "@throw ValueError The given @pattern is malformed\n"
	      "Similar to #refind, but return a sequence of all matches found within ${this.substr(start,end)}\n"
	      "Note that the matches returned are ordered ascendingly") },
	{ "relocateall",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_relocateall,
	  DOC("(pattern:?.,start=!0,end=!-1,rules=!P{})->?S?.\n"
	      "(pattern:?.,rules:?.,start=!0,end=!-1)->?S?.\n"
	      "@param pattern The regular expression patterm (s.a. #rematch)\n"
	      "@param rules The regular expression rules (s.a. #rematch)\n"
	      "@throw ValueError The given @pattern is malformed\n"
	      "Similar to #relocate, but return a sequence of all matched "
	      "sub-strings found within ${this.substr(start,end)}\n"
	      "Note that the matches returned are ordered ascendingly\n"
	      "This function has nothing to do with relocations! - it's pronounced R.E. locate all") },
	{ "resplit",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_resplit,
	  DOC("(pattern:?.,start=!0,end=!-1,rules=!P{})->?S?.\n"
	      "(pattern:?.,rules:?.,start=!0,end=!-1)->?S?.\n"
	      "@param pattern The regular expression patterm (s.a. #rematch)\n"
	      "@param rules The regular expression rules (s.a. #rematch)\n"
	      "@throw ValueError The given @pattern is malformed\n"
	      "Similar to #split, but use a regular expression in order to "
	      "express the sections of the string around which to perform the split\n"
	      ">local data = \"10 , 20,30 40, 50\";\n"
	      ">for (local x: data.resplit(\"(\\\\s*,?)+\\\\s*\"))\n"
	      "> print x; /* `10' `20' `30' `40' `50' */\n"
	      "If you wish to do the reverse and enumerate matches, rather than the "
	      "strings between matches, use #relocateall instead, which also behaves "
	      "as a sequence") },
	{ "restartswith",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_restartswith,
	  DOC("(pattern:?.,start=!0,end=!-1,rules=!P{})->?Dbool\n"
	      "(pattern:?.,rules:?.,start=!0,end=!-1)->?Dbool\n"
	      "@param pattern The regular expression patterm (s.a. #rematch)\n"
	      "@param rules The regular expression rules (s.a. #rematch)\n"
	      "@throw ValueError The given @pattern is malformed\n"
	      "Check if @this string starts with a regular expression described by @pattern (s.a. #startswith)\n"
	      ">function restartswith(pattern) {\n"
	      "> return this.rematch(pattern) != 0;\n"
	      ">}") },
	{ "reendswith",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_reendswith,
	  DOC("(pattern:?.,start=!0,end=!-1,rules=!P{})->?Dbool\n"
	      "(pattern:?.,rules:?.,start=!0,end=!-1)->?Dbool\n"
	      "@param pattern The regular expression patterm (s.a. #rematch)\n"
	      "@param rules The regular expression rules (s.a. #rematch)\n"
	      "@throw ValueError The given @pattern is malformed\n"
	      "Check if @this string ends with a regular expression described by @pattern (s.a. #endswith)\n"
	      ">function restartswith(pattern) {\n"
	      "> local rpos = this.rerfind(pattern);\n"
	      "> return rpos !is none && rpos[1] == #this;\n"
	      ">}") },
	{ "restrip",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_restrip,
	  DOC("(pattern:?.,start=!0,end=!-1,rules=!P{})->?.\n"
	      "(pattern:?.,rules:?.,start=!0,end=!-1)->?.\n"
	      "@param pattern The regular expression patterm (s.a. #rematch)\n"
	      "@param rules The regular expression rules (s.a. #rematch)\n"
	      "@throw ValueError The given @pattern is malformed\n"
	      "Strip all leading and trailing matches for @pattern from @this string and return the result (s.a. #strip)") },
	{ "relstrip",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_relstrip,
	  DOC("(pattern:?.,start=!0,end=!-1,rules=!P{})->?.\n"
	      "(pattern:?.,rules:?.,start=!0,end=!-1)->?.\n"
	      "@param pattern The regular expression patterm (s.a. #rematch)\n"
	      "@param rules The regular expression rules (s.a. #rematch)\n"
	      "@throw ValueError The given @pattern is malformed\n"
	      "Strip all leading matches for @pattern from @this string and return the result (s.a. #lstrip)") },
	{ "rerstrip",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_rerstrip,
	  DOC("(pattern:?.,start=!0,end=!-1,rules=!P{})->?.\n"
	      "(pattern:?.,rules:?.,start=!0,end=!-1)->?.\n"
	      "@param pattern The regular expression patterm (s.a. #rematch)\n"
	      "@param rules The regular expression rules (s.a. #rematch)\n"
	      "@throw ValueError The given @pattern is malformed\n"
	      "Strip all trailing matches for @pattern from @this string and return the result (s.a. #lstrip)") },
	{ "recount",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_recount,
	  DOC("(pattern:?.,start=!0,end=!-1,rules=!P{})->?Dint\n"
	      "(pattern:?.,rules:?.,start=!0,end=!-1)->?Dint\n"
	      "@param pattern The regular expression patterm (s.a. #rematch)\n"
	      "@param rules The regular expression rules (s.a. #rematch)\n"
	      "@throw ValueError The given @pattern is malformed\n"
	      "Count the number of matches of a given regular expression @pattern (s.a. #count)\n"
	      "Hint: This is the same as ${#this.refindall(pattern)} or ${#this.relocateall(pattern)}") },
	{ "recontains",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_recontains,
	  DOC("(pattern:?.,start=!0,end=!-1,rules=!P{})->?Dbool\n"
	      "(pattern:?.,rules:?.,start=!0,end=!-1)->?Dbool\n"
	      "@param pattern The regular expression patterm (s.a. #rematch)\n"
	      "@param rules The regular expression rules (s.a. #rematch)\n"
	      "@throw ValueError The given @pattern is malformed\n"
	      "Check if @this contains a match for the given regular expression @pattern (s.a. #contains)\n"
	      "Hint: This is the same as ${!!this.refindall(pattern)} or ${!!this.relocateall(pattern)}") },

	{ "__sizeof__",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_sizeof,
	  DOC("->?Dint") },

	/* Deprecated functions. */
	{ "reverse",
	  (DREF DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject **))&string_reversed,
	  DOC("(start=!0,end=!-1)->?.\n"
	      "Deprecated alias for #reversed"),
	  TYPE_METHOD_FKWDS },
	{ NULL }
};

PRIVATE DREF String *DCALL
string_cat(String *__restrict self, DeeObject *__restrict other) {
	/* Simple case: `self' is an empty string, so just cast `other' into a string. */
	if (DeeString_IsEmpty(self))
		return (DREF String *)DeeObject_Str(other);
	if (DeeString_Check(other)) {
		/* In the likely case of `other' also being a string, we can
		 * try to perform some optimizations by looking that the common,
		 * required character width, and creating the resulting string in
		 * accordance to what _it_ requires (bypassing the need of for printer). */
		struct string_utf *lhs_utf;
		struct string_utf *rhs_utf;
		/* Simple case: `other' is an empty string, so just re-use `self'. */
		if (DeeString_IsEmpty(other))
			return_reference_(self);
		lhs_utf = self->s_data;
		rhs_utf = ((String *)other)->s_data;
		if (!lhs_utf || lhs_utf->u_width == STRING_WIDTH_1BYTE) {
			if (!rhs_utf || rhs_utf->u_width == STRING_WIDTH_1BYTE) {
				DREF String *result;
				size_t total_length = self->s_len + DeeString_SIZE(other);
				/* Most likely case: both strings use 1-byte characters,
				 * so we don't even need to use a multi-byte buffer! */
				result = (DREF String *)DeeObject_Malloc(COMPILER_OFFSETOF(String, s_str) +
				                                         (total_length + 1) * sizeof(char));
				if unlikely(!result)
					goto err;
				result->s_len = total_length;
				/* Copy characters into the resulting string. */
				memcpy(result->s_str, self->s_str, self->s_len * sizeof(char));
				memcpy(result->s_str + self->s_len,
				       DeeString_STR(other),
				       DeeString_SIZE(other) * sizeof(char));
				/* finalize the resulting string. */
				result->s_str[total_length] = '\0';
				result->s_hash              = DEE_STRING_HASH_UNSET;
				result->s_data              = NULL;
				DeeObject_Init(result, &DeeString_Type);
				return result;
			}
		} else if (rhs_utf && rhs_utf->u_width != STRING_WIDTH_1BYTE) {
			/* >> 2/4-byte + 2/4-byte
			 * This case we can optimize as well, because both the left,
			 * as well as the right string already feature their UTF-8
			 * representations, meaning that while we will have to generate
			 * a 2/4-byte string, we don't have to painfully convert that
			 * string into UTF-8, since we can simply generate the UTF-8
			 * sequence by concat-ing the 2 we already got! */
			DREF String *result;
			struct string_utf *result_utf;
			size_t total_length = self->s_len + DeeString_SIZE(other);
			/* Most likely case: both strings use 1-byte characters,
			 * so we don't even need to use a multi-byte buffer! */
			result = (DREF String *)DeeObject_Malloc(COMPILER_OFFSETOF(String, s_str) +
			                                         (total_length + 1) * sizeof(char));
			if unlikely(!result)
				goto err;
			result_utf = Dee_string_utf_alloc();
			if unlikely(!result_utf)
				goto err_r_2_4;
			memset(result_utf, 0, sizeof(struct string_utf));

			/* Determine the common width of the left and right string,
			 * then construct a 16-bit, or 32-bit character string. */
			ASSERT(lhs_utf->u_width == STRING_WIDTH_2BYTE || lhs_utf->u_width == STRING_WIDTH_4BYTE);
			ASSERT(rhs_utf->u_width == STRING_WIDTH_2BYTE || rhs_utf->u_width == STRING_WIDTH_4BYTE);
			if (lhs_utf->u_width == STRING_WIDTH_2BYTE) {
				if (rhs_utf->u_width == STRING_WIDTH_2BYTE) {
					/* 2-byte + 2-byte --> 2-byte */
					uint16_t *result_string;
					size_t lhs_len, rhs_len;
					uint16_t *lhs_str = (uint16_t *)lhs_utf->u_data[STRING_WIDTH_2BYTE];
					uint16_t *rhs_str = (uint16_t *)rhs_utf->u_data[STRING_WIDTH_2BYTE];
					lhs_len           = WSTR_LENGTH(lhs_str);
					rhs_len           = WSTR_LENGTH(rhs_str);
					result_string     = DeeString_New2ByteBuffer(lhs_len + rhs_len);
					if unlikely(!result_string)
						goto err_r_2_4_utf;
					result_string[lhs_len + rhs_len] = 0;
					memcpyw(result_string, lhs_str, lhs_len);
					memcpyw(result_string + lhs_len, rhs_str, rhs_len);
					result_utf->u_data[STRING_WIDTH_2BYTE] = (size_t *)result_string;
					result_utf->u_width                    = STRING_WIDTH_2BYTE;
				} else {
					/* 2-byte + 4-byte --> 4-byte */
					uint32_t *result_string;
					size_t i, lhs_len, rhs_len;
					uint16_t *lhs_str = (uint16_t *)lhs_utf->u_data[STRING_WIDTH_2BYTE];
					uint32_t *rhs_str = (uint32_t *)rhs_utf->u_data[STRING_WIDTH_4BYTE];
					lhs_len           = WSTR_LENGTH(lhs_str);
					rhs_len           = WSTR_LENGTH(rhs_str);
					result_string     = DeeString_New4ByteBuffer(lhs_len + rhs_len);
					if unlikely(!result_string)
						goto err_r_2_4_utf;
					result_string[lhs_len + rhs_len] = 0;
					for (i = 0; i < lhs_len; ++i)
						result_string[i] = lhs_str[i];
					memcpyl(result_string + lhs_len, rhs_str, rhs_len);
					result_utf->u_data[STRING_WIDTH_4BYTE] = (size_t *)result_string;
					result_utf->u_width                    = STRING_WIDTH_4BYTE;
				}
			} else {
				if (rhs_utf->u_width == STRING_WIDTH_2BYTE) {
					/* 4-byte + 2-byte --> 4-byte */
					uint32_t *result_string;
					size_t i, rhs_len, lhs_len;
					uint32_t *lhs_str = (uint32_t *)lhs_utf->u_data[STRING_WIDTH_4BYTE];
					uint16_t *rhs_str = (uint16_t *)rhs_utf->u_data[STRING_WIDTH_2BYTE];
					lhs_len           = WSTR_LENGTH(lhs_str);
					rhs_len           = WSTR_LENGTH(rhs_str);
					result_string     = DeeString_New4ByteBuffer(lhs_len + rhs_len);
					if unlikely(!result_string)
						goto err_r_2_4_utf;
					result_string[lhs_len + rhs_len] = 0;
					memcpyl(result_string, lhs_str, lhs_len);
					for (i = 0; i < rhs_len; ++i)
						result_string[lhs_len + i] = rhs_str[i];
					result_utf->u_data[STRING_WIDTH_4BYTE] = (size_t *)result_string;
					result_utf->u_width                    = STRING_WIDTH_4BYTE;
				} else {
					/* 4-byte + 4-byte --> 4-byte */
					uint32_t *result_string;
					size_t rhs_len, lhs_len;
					uint16_t *lhs_str = (uint16_t *)lhs_utf->u_data[STRING_WIDTH_4BYTE];
					uint32_t *rhs_str = (uint32_t *)rhs_utf->u_data[STRING_WIDTH_4BYTE];
					lhs_len           = WSTR_LENGTH(lhs_str);
					rhs_len           = WSTR_LENGTH(rhs_str);
					result_string     = DeeString_New4ByteBuffer(lhs_len + rhs_len);
					if unlikely(!result_string)
						goto err_r_2_4_utf;
					result_string[lhs_len + rhs_len] = 0;
					memcpyl(result_string, lhs_str, lhs_len);
					memcpyl(result_string + lhs_len, rhs_str, rhs_len);
					result_utf->u_data[STRING_WIDTH_4BYTE] = (size_t *)result_string;
					result_utf->u_width                    = STRING_WIDTH_4BYTE;
				}
			}
			result->s_len = total_length;
			/* Copy characters into the resulting string. */
			memcpy(result->s_str, self->s_str, self->s_len * sizeof(char));
			memcpy(result->s_str + self->s_len,
			       DeeString_STR(other),
			       DeeString_SIZE(other) * sizeof(char));
			/* finalize the resulting string. */
			result->s_str[total_length] = '\0';
			result->s_hash              = DEE_STRING_HASH_UNSET;
			if (lhs_utf->u_data[STRING_WIDTH_1BYTE] == (size_t *)DeeString_STR(self) &&
			    rhs_utf->u_data[STRING_WIDTH_1BYTE] == (size_t *)DeeString_STR(other))
				result_utf->u_data[STRING_WIDTH_1BYTE] = (size_t *)result->s_str;
			result->s_data = result_utf;
			DeeObject_Init(result, &DeeString_Type);
			return result;
err_r_2_4_utf:
			Dee_string_utf_free(result_utf);
err_r_2_4:
			DeeObject_Free(result);
			return NULL;
		}
	}
	/* Fallback: use a string printer to append `other' to a copy of `self'. */
	{
		struct unicode_printer printer = UNICODE_PRINTER_INIT;
		/* Print our own string. */
		if unlikely(unicode_printer_printstring(&printer, (DeeObject *)self) < 0)
			goto err_printer;
		/* Print the other object (as a string). */
		if unlikely(DeeObject_Print(other, &unicode_printer_print, &printer) < 0)
			goto err_printer;
		return (DREF String *)unicode_printer_pack(&printer);
err_printer:
		unicode_printer_fini(&printer);
	}
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
string_mul(String *__restrict self, DeeObject *__restrict other) {
	size_t my_length, total_length, repeat;
	unsigned int width;
	if (DeeObject_AsSize(other, &repeat))
		return NULL;
	if (DeeString_IsEmpty(self) || !repeat)
		return_empty_string;
	if (repeat == 1)
		return_reference_((DeeObject *)self);
	width = DeeString_WIDTH(self);
	SWITCH_SIZEOF_WIDTH(width) {
		{
			DREF DeeObject *result;
			uint8_t *dst, *src;

		CASE_WIDTH_1BYTE:
			my_length = DeeString_SIZE(self);
			if (OVERFLOW_UMUL(my_length, repeat, &total_length))
				goto err_overflow;
			result = DeeString_NewBuffer(total_length);
			if unlikely(!result)
				goto err;
			src = (uint8_t *)DeeString_STR(self);
			dst = (uint8_t *)DeeString_STR(result);
			while (repeat--) {
				memcpyb(dst, src, my_length);
				dst += my_length;
			}
			return result;
		}
		break;
		{
			uint16_t *dst, *src, *str;

		CASE_WIDTH_2BYTE:
			src       = DeeString_Get2Byte((DeeObject *)self);
			my_length = WSTR_LENGTH(src);
			if (OVERFLOW_UMUL(my_length, repeat, &total_length))
				goto err_overflow;
			dst = str = DeeString_New2ByteBuffer(total_length);
			if unlikely(!str)
				goto err;
			while (repeat--) {
				memcpyw(dst, src, my_length);
				dst += my_length;
			}
			return DeeString_Pack2ByteBuffer(str);
		}
		break;
		{
			uint32_t *dst, *src, *str;

		CASE_WIDTH_4BYTE:
			src       = DeeString_Get4Byte((DeeObject *)self);
			my_length = WSTR_LENGTH(src);
			if (OVERFLOW_UMUL(my_length, repeat, &total_length))
				goto err_overflow;
			dst = str = DeeString_New4ByteBuffer(total_length);
			if unlikely(!str)
				goto err;
			while (repeat--) {
				memcpyl(dst, src, my_length);
				dst += my_length;
			}
			return DeeString_Pack4ByteBuffer(str);
		}
		break;
	}
err_overflow:
	err_integer_overflow_i(sizeof(size_t) * 8, true);
err:
	return NULL;
}


INTDEF dssize_t DCALL
DeeString_CFormat(dformatprinter printer,
                  dformatprinter format_printer, void *arg,
                  /*utf-8*/ char const *__restrict format, size_t format_len,
                  size_t argc, DeeObject **__restrict argv);

PRIVATE DREF String *DCALL
string_mod(String *__restrict self, DeeObject *__restrict args) {
	struct unicode_printer printer = UNICODE_PRINTER_INIT;
	DeeObject **argv;
	size_t argc;
	char *format_str;
	/* C-style string formating */
	if (DeeTuple_Check(args)) {
		argv = DeeTuple_ELEM(args);
		argc = DeeTuple_SIZE(args);
	} else {
		argv = (DeeObject **)&args;
		argc = 1;
	}
	format_str = DeeString_AsUtf8((DeeObject *)self);
	if unlikely(!format_str)
		goto err;
	if unlikely(DeeString_CFormat(&unicode_printer_print,
		                           &unicode_printer_print,
		                           &printer,
		                           format_str,
		                           WSTR_LENGTH(format_str),
		                           argc,
		                           argv) < 0)
	goto err;
	return (DREF String *)unicode_printer_pack(&printer);
err:
	unicode_printer_fini(&printer);
	return NULL;
}

INTERN struct type_math string_math = {
	/* .tp_int32  = */ NULL,
	/* .tp_int64  = */ NULL,
	/* .tp_double = */ NULL,
	/* .tp_int    = */ NULL,
	/* .tp_inv    = */ NULL,
	/* .tp_pos    = */ NULL,
	/* .tp_neg    = */ NULL,
	/* .tp_add    = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&string_cat,
	/* .tp_sub    = */ NULL,
	/* .tp_mul    = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&string_mul,
	/* .tp_div    = */ NULL,
	/* .tp_mod    = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&string_mod,
	/* .tp_shl    = */ NULL,
	/* .tp_shr    = */ NULL,
	/* .tp_and    = */ NULL,
	/* .tp_or     = */ NULL,
	/* .tp_xor    = */ NULL,
	/* .tp_pow    = */ NULL
};


INTERN bool DCALL
string_eq_bytes(String *__restrict self,
                DeeBytesObject *__restrict other) {
	union dcharptr my_str;
	uint8_t *bytes_data;
	size_t bytes_size;
	bytes_data = DeeBytes_DATA(other);
	bytes_size = DeeBytes_SIZE(other);
	SWITCH_SIZEOF_WIDTH(DeeString_WIDTH(self)) {

	CASE_WIDTH_1BYTE:
		my_str.cp8 = DeeString_Get1Byte((DeeObject *)self);
		if (WSTR_LENGTH(my_str.cp8) != bytes_size)
			break;
		return MEMEQB(my_str.cp8, bytes_data, bytes_size);

	CASE_WIDTH_2BYTE:
		my_str.cp16 = DeeString_Get2Byte((DeeObject *)self);
		if (bytes_size != WSTR_LENGTH(my_str.cp16))
			break;
		while (bytes_size--) {
			if (my_str.cp16[bytes_size] != (uint16_t)(bytes_data[bytes_size]))
				goto nope;
		}
		return true;

	CASE_WIDTH_4BYTE:
		my_str.cp32 = DeeString_Get4Byte((DeeObject *)self);
		if (bytes_size != WSTR_LENGTH(my_str.cp32))
			break;
		while (bytes_size--) {
			if (my_str.cp32[bytes_size] != (uint32_t)(bytes_data[bytes_size]))
				goto nope;
		}
		return true;
	}
nope:
	return false;
}

LOCAL uint16_t *DCALL
memmemwb(uint16_t const *__restrict haystack, size_t haystack_length,
         uint8_t const *__restrict needle, size_t needle_length) {
	uint16_t *candidate;
	uint8_t marker;
	if unlikely(!needle_length || needle_length > haystack_length)
		return NULL;
	haystack_length -= needle_length - 1, marker = *(uint8_t *)needle;
	while ((candidate = memchrw(haystack, marker, haystack_length)) != NULL) {
		size_t i;
		for (i = 1; i < needle_length; ++i) {
			if (candidate[i] != (uint16_t)needle[i])
				goto next_candidate;
		}
		return candidate;
next_candidate:
		++candidate;
		haystack_length = (haystack + haystack_length) - candidate;
		haystack        = candidate;
	}
	return NULL;
}

LOCAL uint32_t *DCALL
memmemlb(uint32_t const *__restrict haystack, size_t haystack_length,
         uint8_t const *__restrict needle, size_t needle_length) {
	uint32_t *candidate;
	uint8_t marker;
	if unlikely(!needle_length || needle_length > haystack_length)
		return NULL;
	haystack_length -= needle_length - 1, marker = *(uint8_t *)needle;
	while ((candidate = memchrl(haystack, marker, haystack_length)) != NULL) {
		size_t i;
		for (i = 1; i < needle_length; ++i) {
			if (candidate[i] != (uint32_t)needle[i])
				goto next_candidate;
		}
		return candidate;
next_candidate:
		++candidate;
		haystack_length = (haystack + haystack_length) - candidate;
		haystack        = candidate;
	}
	return NULL;
}


INTERN DREF DeeObject *DCALL
string_contains(String *__restrict self,
                DeeObject *__restrict some_object) {
	union dcharptr str, other, ptr;
	if (DeeBytes_Check(some_object)) {
		size_t other_len = DeeBytes_SIZE(some_object);
		other.cp8        = DeeBytes_DATA(some_object);
		SWITCH_SIZEOF_WIDTH(DeeString_WIDTH(self)) {

		CASE_WIDTH_1BYTE:
			str.cp8 = DeeString_Get1Byte((DeeObject *)self);
			ptr.cp8 = memmemb(str.cp16, WSTR_LENGTH(str.cp16), other.cp8, other_len);
			break;

		CASE_WIDTH_2BYTE:
			str.cp16 = DeeString_Get2Byte((DeeObject *)self);
			ptr.cp16 = memmemwb(str.cp16, WSTR_LENGTH(str.cp16), other.cp8, other_len);
			break;

		CASE_WIDTH_4BYTE:
			str.cp32 = DeeString_Get4Byte((DeeObject *)self);
			ptr.cp32 = memmemlb(str.cp32, WSTR_LENGTH(str.cp32), other.cp8, other_len);
			break;
		}
	} else {
		if (DeeObject_AssertTypeExact(some_object, &DeeString_Type))
			goto err;
		/* Search for an occurrence of `some_object' */
		SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON(DeeString_WIDTH(self),
		                                        DeeString_WIDTH(some_object))) {

		CASE_WIDTH_1BYTE:
			str.cp8   = DeeString_As1Byte((DeeObject *)self);
			other.cp8 = DeeString_As1Byte((DeeObject *)some_object);
			ptr.cp8 = memmemb(str.cp8, WSTR_LENGTH(str.cp8),
			                  other.cp8, WSTR_LENGTH(other.cp8));
			break;

		CASE_WIDTH_2BYTE:
			str.cp16 = DeeString_As2Byte((DeeObject *)self);
			if unlikely(!str.cp16)
				goto err;
			other.cp16 = DeeString_As2Byte((DeeObject *)some_object);
			if unlikely(!other.cp16)
				goto err;
			ptr.cp16 = memmemw(str.cp16, WSTR_LENGTH(str.cp16),
			                   other.cp16, WSTR_LENGTH(other.cp16));
			break;

		CASE_WIDTH_4BYTE:
			str.cp32 = DeeString_As4Byte((DeeObject *)self);
			if unlikely(!str.cp32)
				goto err;
			other.cp32 = DeeString_As4Byte((DeeObject *)some_object);
			if unlikely(!other.cp32)
				goto err;
			ptr.cp32 = memmeml(str.cp32, WSTR_LENGTH(str.cp32),
			                   other.cp32, WSTR_LENGTH(other.cp32));
			break;
		}
	}
	return_bool_(ptr.ptr != NULL);
err:
	return NULL;
}


PUBLIC dssize_t DCALL
Dee_unicode_printer_memchr(struct unicode_printer *__restrict self,
                           uint32_t chr, size_t start, size_t length) {
	union dcharptr ptr, str;
	size_t result;
	str.ptr = self->up_buffer;
	ASSERT(start + length <= (str.ptr ? WSTR_LENGTH(str.ptr) : 0));
	if unlikely(!str.ptr)
		goto not_found;
	SWITCH_SIZEOF_WIDTH(self->up_flags & UNICODE_PRINTER_FWIDTH) {

	CASE_WIDTH_1BYTE:
		if (chr > 0xff)
			goto not_found;
		ptr.cp8 = memchrb(str.cp8 + start, (uint8_t)chr, length);
		if (!ptr.cp8)
			goto not_found;
		result = (size_t)(ptr.cp8 - str.cp8);
		break;

	CASE_WIDTH_2BYTE:
		if (chr > 0xffff)
			goto not_found;
		ptr.cp16 = memchrw(str.cp16 + start, (uint16_t)chr, length);
		if (!ptr.cp16)
			goto not_found;
		result = (size_t)(ptr.cp16 - str.cp16);
		break;

	CASE_WIDTH_4BYTE:
		ptr.cp32 = memchrl(str.cp32 + start, (uint32_t)chr, length);
		if (!ptr.cp32)
			goto not_found;
		result = (size_t)(ptr.cp32 - str.cp32);
		break;
	}
	return (dssize_t)result;
not_found:
	return -1;
}

PUBLIC dssize_t DCALL
Dee_unicode_printer_memrchr(struct unicode_printer *__restrict self,
                            uint32_t chr, size_t start, size_t length) {
	union dcharptr ptr, str;
	size_t result;
	str.ptr = self->up_buffer;
	ASSERT(start + length <= (str.ptr ? WSTR_LENGTH(str.ptr) : 0));
	if unlikely(!str.ptr)
		goto not_found;
	SWITCH_SIZEOF_WIDTH(self->up_flags & UNICODE_PRINTER_FWIDTH) {

	CASE_WIDTH_1BYTE:
		if (chr > 0xff)
			goto not_found;
		ptr.cp8 = memrchrb(str.cp8 + start, (uint8_t)chr, length);
		if (!ptr.cp8)
			goto not_found;
		result = (size_t)(ptr.cp8 - str.cp8);
		break;

	CASE_WIDTH_2BYTE:
		if (chr > 0xffff)
			goto not_found;
		ptr.cp16 = memrchrw(str.cp16 + start, (uint16_t)chr, length);
		if (!ptr.cp16)
			goto not_found;
		result = (size_t)(ptr.cp16 - str.cp16);
		break;

	CASE_WIDTH_4BYTE:
		ptr.cp32 = memrchrl(str.cp32 + start, (uint32_t)chr, length);
		if (!ptr.cp32)
			goto not_found;
		result = (size_t)(ptr.cp32 - str.cp32);
		break;
	}
	return (dssize_t)result;
not_found:
	return -1;
}

PUBLIC void (DCALL Dee_unicode_printer_memmove)(struct unicode_printer *__restrict self,
                                                size_t dst, size_t src, size_t length) {
	union dcharptr str;
	str.ptr = self->up_buffer;
	ASSERT(dst + length <= (str.ptr ? WSTR_LENGTH(str.ptr) : 0));
	ASSERT(src + length <= (str.ptr ? WSTR_LENGTH(str.ptr) : 0));
	if unlikely(!str.ptr)
		return;
	SWITCH_SIZEOF_WIDTH(self->up_flags & UNICODE_PRINTER_FWIDTH) {

	CASE_WIDTH_1BYTE:
		memmoveb(str.cp8 + dst, str.cp8 + src, length);
		break;

	CASE_WIDTH_2BYTE:
		memmovew(str.cp16 + dst, str.cp16 + src, length);
		break;

	CASE_WIDTH_4BYTE:
		memmovel(str.cp32 + dst, str.cp32 + src, length);
		break;
	}
}


DECL_END

#ifndef __INTELLISENSE__
#include "ordinals.c.inl"
#include "split.c.inl"
#include "segments.c.inl"
#include "reproxy.c.inl"
#include "finder.c.inl"

/* Include this last! */
#include "bytes_functions.c.inl"
#endif

#endif /* !GUARD_DEEMON_OBJECTS_UNICODE_STRING_FUNCTIONS_C */
