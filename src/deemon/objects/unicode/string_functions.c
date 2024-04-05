/* Copyright (c) 2018-2024 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2024 Griefer@Work                           *
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
#include <deemon/float.h>
#include <deemon/int.h>
#include <deemon/none.h>
#include <deemon/regex.h>
#include <deemon/seq.h>
#include <deemon/stringutils.h>
#include <deemon/system-features.h> /* memcpy(), bzero(), ... */
#include <deemon/thread.h>
#include <deemon/tuple.h>

#include <hybrid/byteswap.h>
#include <hybrid/overflow.h>

#include "regroups.h"

#ifndef SIZE_MAX
#include <hybrid/limitcore.h>
#ifndef SIZE_MAX
#define SIZE_MAX __SIZE_MAX__
#endif /* !SIZE_MAX */
#endif /* !SIZE_MAX */

DECL_BEGIN

#ifdef __INTELLISENSE__
PRIVATE WUNUSED NONNULL((1)) DREF String *DCALL DeeString_StripSpc(String *__restrict self);
PRIVATE WUNUSED NONNULL((1)) DREF String *DCALL DeeString_LStripSpc(String *__restrict self, size_t max_count);
PRIVATE WUNUSED NONNULL((1)) DREF String *DCALL DeeString_RStripSpc(String *__restrict self, size_t max_count);
PRIVATE WUNUSED NONNULL((1, 2)) DREF String *DCALL DeeString_StripMask(String *self, String *mask);
PRIVATE WUNUSED NONNULL((1, 2)) DREF String *DCALL DeeString_LStripMask(String *self, String *mask, size_t max_count);
PRIVATE WUNUSED NONNULL((1, 2)) DREF String *DCALL DeeString_RStripMask(String *self, String *mask, size_t max_count);
PRIVATE WUNUSED NONNULL((1, 2)) DREF String *DCALL DeeString_CaseStripMask(String *self, String *mask);
PRIVATE WUNUSED NONNULL((1, 2)) DREF String *DCALL DeeString_CaseLStripMask(String *self, String *mask, size_t max_count);
PRIVATE WUNUSED NONNULL((1, 2)) DREF String *DCALL DeeString_CaseRStripMask(String *self, String *mask, size_t max_count);
PRIVATE WUNUSED NONNULL((1, 2)) DREF String *DCALL DeeString_SStrip(String *self, String *mask);
PRIVATE WUNUSED NONNULL((1, 2)) DREF String *DCALL DeeString_LSStrip(String *self, String *mask, size_t max_count);
PRIVATE WUNUSED NONNULL((1, 2)) DREF String *DCALL DeeString_RSStrip(String *self, String *mask, size_t max_count);
PRIVATE WUNUSED NONNULL((1, 2)) DREF String *DCALL DeeString_CaseSStrip(String *self, String *mask);
PRIVATE WUNUSED NONNULL((1, 2)) DREF String *DCALL DeeString_CaseLSStrip(String *self, String *mask, size_t max_count);
PRIVATE WUNUSED NONNULL((1, 2)) DREF String *DCALL DeeString_CaseRSStrip(String *self, String *mask, size_t max_count);
PRIVATE WUNUSED NONNULL((1)) DREF String *DCALL DeeString_StripLinesSpc(String *__restrict self);
PRIVATE WUNUSED NONNULL((1)) DREF String *DCALL DeeString_LStripLinesSpc(String *__restrict self);
PRIVATE WUNUSED NONNULL((1)) DREF String *DCALL DeeString_RStripLinesSpc(String *__restrict self);
PRIVATE WUNUSED NONNULL((1, 2)) DREF String *DCALL DeeString_StripLinesMask(String *self, String *mask);
PRIVATE WUNUSED NONNULL((1, 2)) DREF String *DCALL DeeString_LStripLinesMask(String *self, String *mask);
PRIVATE WUNUSED NONNULL((1, 2)) DREF String *DCALL DeeString_RStripLinesMask(String *self, String *mask);
PRIVATE WUNUSED NONNULL((1, 2)) DREF String *DCALL DeeString_CaseStripLinesMask(String *self, String *mask);
PRIVATE WUNUSED NONNULL((1, 2)) DREF String *DCALL DeeString_CaseLStripLinesMask(String *self, String *mask);
PRIVATE WUNUSED NONNULL((1, 2)) DREF String *DCALL DeeString_CaseRStripLinesMask(String *self, String *mask);
PRIVATE WUNUSED NONNULL((1, 2)) DREF String *DCALL DeeString_SStripLines(String *self, String *mask);
PRIVATE WUNUSED NONNULL((1, 2)) DREF String *DCALL DeeString_LSStripLines(String *self, String *mask);
PRIVATE WUNUSED NONNULL((1, 2)) DREF String *DCALL DeeString_RSStripLines(String *self, String *mask);
PRIVATE WUNUSED NONNULL((1, 2)) DREF String *DCALL DeeString_CaseSStripLines(String *self, String *mask);
PRIVATE WUNUSED NONNULL((1, 2)) DREF String *DCALL DeeString_CaseLSStripLines(String *self, String *mask);
PRIVATE WUNUSED NONNULL((1, 2)) DREF String *DCALL DeeString_CaseRSStripLines(String *self, String *mask);
#else /* __INTELLISENSE__ */
DECL_END
#define DEFINE_DeeString_StripSpc
#include "string_functions-strip.c.inl"
#define DEFINE_DeeString_LStripSpc
#include "string_functions-strip.c.inl"
#define DEFINE_DeeString_RStripSpc
#include "string_functions-strip.c.inl"
#define DEFINE_DeeString_StripMask
#include "string_functions-strip.c.inl"
#define DEFINE_DeeString_LStripMask
#include "string_functions-strip.c.inl"
#define DEFINE_DeeString_RStripMask
#include "string_functions-strip.c.inl"
#define DEFINE_DeeString_CaseStripMask
#include "string_functions-strip.c.inl"
#define DEFINE_DeeString_CaseLStripMask
#include "string_functions-strip.c.inl"
#define DEFINE_DeeString_CaseRStripMask
#include "string_functions-strip.c.inl"
#define DEFINE_DeeString_SStrip
#include "string_functions-strip.c.inl"
#define DEFINE_DeeString_LSStrip
#include "string_functions-strip.c.inl"
#define DEFINE_DeeString_RSStrip
#include "string_functions-strip.c.inl"
#define DEFINE_DeeString_CaseSStrip
#include "string_functions-strip.c.inl"
#define DEFINE_DeeString_CaseLSStrip
#include "string_functions-strip.c.inl"
#define DEFINE_DeeString_CaseRSStrip
#include "string_functions-strip.c.inl"
#define DEFINE_DeeString_StripLinesSpc
#include "string_functions-strip.c.inl"
#define DEFINE_DeeString_LStripLinesSpc
#include "string_functions-strip.c.inl"
#define DEFINE_DeeString_RStripLinesSpc
#include "string_functions-strip.c.inl"
#define DEFINE_DeeString_StripLinesMask
#include "string_functions-strip.c.inl"
#define DEFINE_DeeString_LStripLinesMask
#include "string_functions-strip.c.inl"
#define DEFINE_DeeString_RStripLinesMask
#include "string_functions-strip.c.inl"
#define DEFINE_DeeString_CaseStripLinesMask
#include "string_functions-strip.c.inl"
#define DEFINE_DeeString_CaseLStripLinesMask
#include "string_functions-strip.c.inl"
#define DEFINE_DeeString_CaseRStripLinesMask
#include "string_functions-strip.c.inl"
#define DEFINE_DeeString_SStripLines
#include "string_functions-strip.c.inl"
#define DEFINE_DeeString_LSStripLines
#include "string_functions-strip.c.inl"
#define DEFINE_DeeString_RSStripLines
#include "string_functions-strip.c.inl"
#define DEFINE_DeeString_CaseSStripLines
#include "string_functions-strip.c.inl"
#define DEFINE_DeeString_CaseLSStripLines
#include "string_functions-strip.c.inl"
#define DEFINE_DeeString_CaseRSStripLines
#include "string_functions-strip.c.inl"
DECL_BEGIN
#endif /* !__INTELLISENSE__ */

PRIVATE WUNUSED NONNULL((1)) DREF String *DCALL
DeeString_Reversed(String *__restrict self,
                   size_t begin, size_t end) {
	DREF String *result;
	int width;
	uint8_t *my_str, *dst;
	size_t flip_size;
	width  = DeeString_WIDTH(self);
	my_str = (uint8_t *)DeeString_WSTR(self);
	if (end > WSTR_LENGTH(my_str))
		end = WSTR_LENGTH(my_str);
	if (end <= begin)
		return_reference_((String *)Dee_EmptyString); /* Empty string area. */
	flip_size = (size_t)(end - begin);
	ASSERT(flip_size != 0);
	/* Actually perform the search for the given string. */
	SWITCH_SIZEOF_WIDTH(width) {

	CASE_WIDTH_1BYTE: {
		result = (DREF String *)DeeString_NewBuffer(flip_size);
		if unlikely(!result)
			goto err;
		dst = (uint8_t *)DeeString_STR(result);
		do {
			--flip_size;
			*(uint8_t *)dst = ((uint8_t *)my_str)[flip_size];
			dst += 1;
		} while (flip_size);
	}	break;

	CASE_WIDTH_2BYTE: {
		uint16_t *buf;
		buf = DeeString_New2ByteBuffer(flip_size);
		if unlikely(!buf)
			goto err;
		dst = (uint8_t *)buf;
		do {
			--flip_size;
			*(uint16_t *)dst = ((uint16_t *)my_str)[flip_size];
			dst += 2;
		} while (flip_size);
		result = (DREF String *)DeeString_Pack2ByteBuffer(buf);
	}	break;

	CASE_WIDTH_4BYTE: {
		uint32_t *buf;
		buf = DeeString_New4ByteBuffer(flip_size);
		if unlikely(!buf)
			goto err;
		dst = (uint8_t *)buf;
		do {
			--flip_size;
			*(uint32_t *)dst = ((uint32_t *)my_str)[flip_size];
			dst += 4;
		} while (flip_size);
		result = (DREF String *)DeeString_Pack4ByteBuffer(buf);
	}	break;

	}
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF String *DCALL
DeeString_ExpandTabs(String *__restrict self, size_t tab_width) {
	union dcharptr iter, end, flush_start;
	size_t line_inset              = 0;
	struct unicode_printer printer = UNICODE_PRINTER_INIT;
	SWITCH_SIZEOF_WIDTH(DeeString_WIDTH(self)) {

	CASE_WIDTH_1BYTE:
		iter.cp8        = DeeString_Get1Byte((DeeObject *)self);
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
		iter.cp16        = DeeString_Get2Byte((DeeObject *)self);
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
		iter.cp32        = DeeString_Get4Byte((DeeObject *)self);
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
	return (DREF String *)unicode_printer_pack(&printer);
retself:
	unicode_printer_fini(&printer);
	return_reference_(self);
err:
	unicode_printer_fini(&printer);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF String *DCALL
DeeString_UnifyLines(String *self, String *replacement) {
	union dcharptr iter, end, flush_start;
	struct unicode_printer printer = UNICODE_PRINTER_INIT;
	SWITCH_SIZEOF_WIDTH(DeeString_WIDTH(self)) {

	CASE_WIDTH_1BYTE:
		iter.cp8        = DeeString_Get1Byte((DeeObject *)self);
		end.cp8         = iter.cp8 + WSTR_LENGTH(iter.cp8);
		flush_start.cp8 = iter.cp8;
		unicode_printer_allocate(&printer,
		                         (size_t)(end.cp8 - iter.cp8),
		                         STRING_WIDTH_1BYTE);
		for (; iter.cp8 < end.cp8; ++iter.cp8) {
			uint8_t ch = *iter.cp8;
			if (!DeeUni_IsLF(ch))
				continue;
			if unlikely(unicode_printer_print8(&printer, flush_start.cp8, (size_t)(iter.cp8 - flush_start.cp8)) < 0)
				goto err;
			if unlikely(unicode_printer_printstring(&printer, (DeeObject *)replacement) < 0)
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
		iter.cp16        = DeeString_Get2Byte((DeeObject *)self);
		end.cp16         = iter.cp16 + WSTR_LENGTH(iter.cp16);
		flush_start.cp16 = iter.cp16;
		unicode_printer_allocate(&printer,
		                         (size_t)(end.cp16 - iter.cp16),
		                         STRING_WIDTH_2BYTE);
		for (; iter.cp16 < end.cp16; ++iter.cp16) {
			uint16_t ch = *iter.cp16;
			if (!DeeUni_IsLF(ch))
				continue;
			if unlikely(unicode_printer_print16(&printer, flush_start.cp16, (size_t)(iter.cp16 - flush_start.cp16)) < 0)
				goto err;
			if unlikely(unicode_printer_printstring(&printer, (DeeObject *)replacement) < 0)
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
		iter.cp32        = DeeString_Get4Byte((DeeObject *)self);
		end.cp32         = iter.cp32 + WSTR_LENGTH(iter.cp32);
		flush_start.cp32 = iter.cp32;
		unicode_printer_allocate(&printer,
		                         (size_t)(end.cp32 - iter.cp32),
		                         STRING_WIDTH_4BYTE);
		for (; iter.cp32 < end.cp32; ++iter.cp32) {
			uint32_t ch = *iter.cp32;
			if (!DeeUni_IsLF(ch))
				continue;
			if unlikely(unicode_printer_print32(&printer, flush_start.cp32, (size_t)(iter.cp32 - flush_start.cp32)) < 0)
				goto err;
			if unlikely(unicode_printer_printstring(&printer, (DeeObject *)replacement) < 0)
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
	return (DREF String *)unicode_printer_pack(&printer);
retself:
	unicode_printer_fini(&printer);
	return_reference_(self);
err:
	unicode_printer_fini(&printer);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF String *DCALL
DeeString_UnifyLinesLf(String *__restrict self) {
	union dcharptr iter, end, flush_start;
	struct unicode_printer printer = UNICODE_PRINTER_INIT;
	SWITCH_SIZEOF_WIDTH(DeeString_WIDTH(self)) {

	CASE_WIDTH_1BYTE:
		iter.cp8        = DeeString_Get1Byte((DeeObject *)self);
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
			if unlikely(unicode_printer_print8(&printer, flush_start.cp8, (size_t)(iter.cp8 - flush_start.cp8)) < 0)
				goto err;
			if unlikely(unicode_printer_putascii(&printer, UNICODE_LF) < 0)
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
		iter.cp16        = DeeString_Get2Byte((DeeObject *)self);
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
			if unlikely(unicode_printer_print16(&printer, flush_start.cp16, (size_t)(iter.cp16 - flush_start.cp16)) < 0)
				goto err;
			if unlikely(unicode_printer_putascii(&printer, UNICODE_LF) < 0)
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
		iter.cp32        = DeeString_Get4Byte((DeeObject *)self);
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
			if unlikely(unicode_printer_print32(&printer, flush_start.cp32, (size_t)(iter.cp32 - flush_start.cp32)) < 0)
				goto err;
			if unlikely(unicode_printer_putascii(&printer, UNICODE_LF) < 0)
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
	return (DREF String *)unicode_printer_pack(&printer);
retself:
	unicode_printer_fini(&printer);
	return_reference_(self);
err:
	unicode_printer_fini(&printer);
	return NULL;
}

#define DeeString_IsCntrl(self, start, end)    DeeString_TestTrait(self, start, end, UNICODE_ISCNTRL)
#define DeeString_IsTab(self, start, end)      DeeString_TestTrait(self, start, end, UNICODE_ISTAB)
#define DeeString_IsCempty(self, start, end)   DeeString_TestTrait(self, start, end, UNICODE_ISEMPTY)
#define DeeString_IsWhite(self, start, end)    DeeString_TestTrait(self, start, end, UNICODE_ISWHITE)
#define DeeString_IsLF(self, start, end)       DeeString_TestTrait(self, start, end, UNICODE_ISLF)
#define DeeString_IsSpace(self, start, end)    DeeString_TestTrait(self, start, end, UNICODE_ISSPACE)
#define DeeString_IsLower(self, start, end)    DeeString_TestTrait(self, start, end, UNICODE_ISLOWER)
#define DeeString_IsUpper(self, start, end)    DeeString_TestTrait(self, start, end, UNICODE_ISUPPER)
#define DeeString_IsAlpha(self, start, end)    DeeString_TestTrait(self, start, end, UNICODE_ISALPHA)
#define DeeString_IsDigit(self, start, end)    DeeString_TestTrait(self, start, end, UNICODE_ISDIGIT)
#define DeeString_IsHex(self, start, end)      DeeString_TestTrait(self, start, end, UNICODE_ISHEX)
#define DeeString_IsXdigit(self, start, end)   DeeString_TestTrait(self, start, end, UNICODE_ISXDIGIT)
#define DeeString_IsAlnum(self, start, end)    DeeString_TestTrait(self, start, end, UNICODE_ISALNUM)
#define DeeString_IsPunct(self, start, end)    DeeString_TestTrait(self, start, end, UNICODE_ISPUNCT)
#define DeeString_IsGraph(self, start, end)    DeeString_TestTrait(self, start, end, UNICODE_ISGRAPH)
#define DeeString_IsPrint(self, start, end)    DeeString_TestTrait(self, start, end, UNICODE_ISPRINT)
#define DeeString_IsBlank(self, start, end)    DeeString_TestTrait(self, start, end, UNICODE_ISBLANK)
#define DeeString_IsNumeric(self, start, end)  DeeString_TestTrait(self, start, end, UNICODE_ISNUMERIC)
#define DeeString_IsSymStrt(self, start, end)  DeeString_TestTrait(self, start, end, UNICODE_ISSYMSTRT)
#define DeeString_IsSymCont(self, start, end)  DeeString_TestTrait(self, start, end, UNICODE_ISSYMCONT)

#define DeeString_IsAnyCntrl(self, start, end)    DeeString_TestAnyTrait(self, start, end, UNICODE_ISCNTRL)
#define DeeString_IsAnyTab(self, start, end)      DeeString_TestAnyTrait(self, start, end, UNICODE_ISTAB)
#define DeeString_IsAnyCempty(self, start, end)   DeeString_TestAnyTrait(self, start, end, UNICODE_ISEMPTY)
#define DeeString_IsAnyWhite(self, start, end)    DeeString_TestAnyTrait(self, start, end, UNICODE_ISWHITE)
#define DeeString_IsAnyLF(self, start, end)       DeeString_TestAnyTrait(self, start, end, UNICODE_ISLF)
#define DeeString_IsAnySpace(self, start, end)    DeeString_TestAnyTrait(self, start, end, UNICODE_ISSPACE)
#define DeeString_IsAnyLower(self, start, end)    DeeString_TestAnyTrait(self, start, end, UNICODE_ISLOWER)
#define DeeString_IsAnyUpper(self, start, end)    DeeString_TestAnyTrait(self, start, end, UNICODE_ISUPPER)
#define DeeString_IsAnyAlpha(self, start, end)    DeeString_TestAnyTrait(self, start, end, UNICODE_ISALPHA)
#define DeeString_IsAnyDigit(self, start, end)    DeeString_TestAnyTrait(self, start, end, UNICODE_ISDIGIT)
#define DeeString_IsAnyHex(self, start, end)      DeeString_TestAnyTrait(self, start, end, UNICODE_ISHEX)
#define DeeString_IsAnyXdigit(self, start, end)   DeeString_TestAnyTrait(self, start, end, UNICODE_ISXDIGIT)
#define DeeString_IsAnyAlnum(self, start, end)    DeeString_TestAnyTrait(self, start, end, UNICODE_ISALNUM)
#define DeeString_IsAnyPunct(self, start, end)    DeeString_TestAnyTrait(self, start, end, UNICODE_ISPUNCT)
#define DeeString_IsAnyGraph(self, start, end)    DeeString_TestAnyTrait(self, start, end, UNICODE_ISGRAPH)
#define DeeString_IsAnyPrint(self, start, end)    DeeString_TestAnyTrait(self, start, end, UNICODE_ISPRINT)
#define DeeString_IsAnyBlank(self, start, end)    DeeString_TestAnyTrait(self, start, end, UNICODE_ISBLANK)
#define DeeString_IsAnyTitle(self, start, end)    DeeString_TestAnyTrait(self, start, end, UNICODE_ISTITLE)
#define DeeString_IsAnyNumeric(self, start, end)  DeeString_TestAnyTrait(self, start, end, UNICODE_ISNUMERIC)
#define DeeString_IsAnySymStrt(self, start, end)  DeeString_TestAnyTrait(self, start, end, UNICODE_ISSYMSTRT)
#define DeeString_IsAnySymCont(self, start, end)  DeeString_TestAnyTrait(self, start, end, UNICODE_ISSYMCONT)

PRIVATE WUNUSED NONNULL((1)) bool DCALL
DeeString_TestTrait(String *__restrict self,
                    size_t start_index,
                    size_t end_index,
                    uniflag_t flags) {
	DeeString_Foreach(self, start_index, end_index, iter, end, {
		if (!(DeeUni_Flags(*iter) & flags))
			return false;
	});
	return true;
}

PRIVATE WUNUSED NONNULL((1)) bool DCALL
DeeString_TestAnyTrait(String *__restrict self,
                       size_t start_index,
                       size_t end_index,
                       uniflag_t flags) {
	DeeString_Foreach(self, start_index, end_index, iter, end, {
		if (DeeUni_Flags(*iter) & flags)
			return true;
	});
	return false;
}

PRIVATE WUNUSED NONNULL((1)) bool DCALL
DeeString_IsAscii(String *__restrict self,
                  size_t start_index,
                  size_t end_index) {
	struct string_utf *utf = self->s_data;
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

PRIVATE WUNUSED NONNULL((1)) bool DCALL
DeeString_IsAnyAscii(String *__restrict self,
                     size_t start_index,
                     size_t end_index) {
	DeeString_Foreach(self, start_index, end_index, iter, end, {
		if (*iter <= 0x7f)
			return true;
	});
	return false;
}


PRIVATE WUNUSED NONNULL((1)) bool DCALL
DeeString_IsTitle(String *__restrict self,
                  size_t start_index,
                  size_t end_index) {
	bool was_space = false;
	DeeString_Foreach(self, start_index, end_index, iter, end, {
		uniflag_t f = DeeUni_Flags(*iter);
		if (f & UNICODE_ISSPACE) {
			was_space = true;
		} else if (was_space) {
			was_space = false;
			/* Space must be followed by title- or upper-case */
			if (!(f & (UNICODE_ISTITLE | UNICODE_ISUPPER)))
				return false;
		} else {
			/* Title- or upper-case anywhere else is illegal */
			if (f & (UNICODE_ISTITLE | UNICODE_ISUPPER))
				return false;
		}
	});
	return true;
}

/* NOTE: `INTERN', because also used in `/src/deemon/compiler/interface/iast.c' */
INTERN WUNUSED NONNULL((1)) bool DCALL
DeeString_IsSymbol(String *__restrict self,
                   size_t start_index,
                   size_t end_index) {
	uniflag_t flags = UNICODE_ISSYMSTRT;
	DeeString_Foreach(self, start_index, end_index, iter, end, {
		if (!(DeeUni_Flags(*iter) & flags))
			return false;
		flags = UNICODE_ISSYMCONT;
	});
	return true;
}



PRIVATE WUNUSED NONNULL((1, 2)) DREF String *DCALL
DeeString_Indent(String *self, String *filler) {
	/* Simple case: if the filler, or self-string are
	 *              empty, nothing would get inserted! */
	if unlikely(DeeString_IsEmpty(filler))
		return_reference_(self);
	if unlikely(DeeString_IsEmpty(self))
		return_reference_(filler);
	{
		struct unicode_printer printer = UNICODE_PRINTER_INIT;
		union dcharptr flush_start, iter, end;
		/* Start by inserting the initial, unconditional indentation at the start. */
		if (unicode_printer_printstring(&printer, (DeeObject *)filler) < 0)
			goto err;
		SWITCH_SIZEOF_WIDTH(DeeString_WIDTH(self)) {

		CASE_WIDTH_1BYTE:
			iter.cp8        = DeeString_Get1Byte((DeeObject *)self);
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
					if (unicode_printer_printobject(&printer, (DeeObject *)filler) < 0)
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
			iter.cp16        = DeeString_Get2Byte((DeeObject *)self);
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
					if (unicode_printer_printobject(&printer, (DeeObject *)filler) < 0)
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
			iter.cp32        = DeeString_Get4Byte((DeeObject *)self);
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
					if (unicode_printer_printobject(&printer, (DeeObject *)filler) < 0)
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
		return (DREF String *)unicode_printer_pack(&printer);
err:
		unicode_printer_fini(&printer);
		return NULL;
	}
}

PRIVATE WUNUSED NONNULL((1)) bool DCALL
mask_containsb(String *__restrict self, uint8_t ch) {
	union dcharptr str;
	SWITCH_SIZEOF_WIDTH(DeeString_WIDTH(self)) {

	CASE_WIDTH_1BYTE:
		str.cp8 = DeeString_Get1Byte((DeeObject *)self);
		return memchrb(str.cp8, ch, WSTR_LENGTH(str.cp8)) != NULL;

	CASE_WIDTH_2BYTE:
		str.cp16 = DeeString_Get2Byte((DeeObject *)self);
		return memchrw(str.cp16, ch, WSTR_LENGTH(str.cp16)) != NULL;

	CASE_WIDTH_4BYTE:
		str.cp32 = DeeString_Get4Byte((DeeObject *)self);
		return memchrl(str.cp32, ch, WSTR_LENGTH(str.cp32)) != NULL;
	}
}

PRIVATE WUNUSED NONNULL((1)) bool DCALL
mask_containsw(String *__restrict self, uint16_t ch) {
	union dcharptr str;
	SWITCH_SIZEOF_WIDTH(DeeString_WIDTH(self)) {

	CASE_WIDTH_1BYTE:
		if (ch > 0xff)
			return false;
		str.cp8 = DeeString_Get1Byte((DeeObject *)self);
		return memchrb(str.cp8, (uint8_t)ch, WSTR_LENGTH(str.cp8)) != NULL;

	CASE_WIDTH_2BYTE:
		str.cp16 = DeeString_Get2Byte((DeeObject *)self);
		return memchrw(str.cp16, ch, WSTR_LENGTH(str.cp16)) != NULL;

	CASE_WIDTH_4BYTE:
		str.cp32 = DeeString_Get4Byte((DeeObject *)self);
		return memchrl(str.cp32, ch, WSTR_LENGTH(str.cp32)) != NULL;
	}
}

PRIVATE WUNUSED NONNULL((1)) bool DCALL
mask_containsl(String *__restrict self, uint32_t ch) {
	union dcharptr str;
	SWITCH_SIZEOF_WIDTH(DeeString_WIDTH(self)) {

	CASE_WIDTH_1BYTE:
		if (ch > 0xff)
			return false;
		str.cp8 = DeeString_Get1Byte((DeeObject *)self);
		return memchrb(str.cp8, (uint8_t)ch, WSTR_LENGTH(str.cp8)) != NULL;

	CASE_WIDTH_2BYTE:
		if (ch > 0xffff)
			return false;
		str.cp16 = DeeString_Get2Byte((DeeObject *)self);
		return memchrw(str.cp16, (uint16_t)ch, WSTR_LENGTH(str.cp16)) != NULL;

	CASE_WIDTH_4BYTE:
		str.cp32 = DeeString_Get4Byte((DeeObject *)self);
		return memchrl(str.cp32, ch, WSTR_LENGTH(str.cp32)) != NULL;
	}
}


PRIVATE WUNUSED DREF String *DCALL
DeeString_Dedent(String *__restrict self,
                 size_t max_chars,
                 String *__restrict mask) {
	struct unicode_printer printer = UNICODE_PRINTER_INIT;
	union dcharptr flush_start, iter, end;
	size_t i;
	/* Simple case: Nothing should be removed. */
	if unlikely(!max_chars)
		return_reference_(self);
	SWITCH_SIZEOF_WIDTH(DeeString_WIDTH(self)) {

	CASE_WIDTH_1BYTE:
		iter.cp8 = DeeString_Get1Byte((DeeObject *)self);
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
		iter.cp16 = DeeString_Get2Byte((DeeObject *)self);
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
		iter.cp32 = DeeString_Get4Byte((DeeObject *)self);
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
	return (DREF String *)unicode_printer_pack(&printer);
err:
	unicode_printer_fini(&printer);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF String *DCALL
DeeString_DedentSpc(String *__restrict self,
                    size_t max_chars) {
	struct unicode_printer printer = UNICODE_PRINTER_INIT;
	union dcharptr flush_start, iter, end;
	size_t i;
	/* Simple case: Nothing should be removed. */
	if unlikely(!max_chars)
		return_reference_(self);
	SWITCH_SIZEOF_WIDTH(DeeString_WIDTH(self)) {

	CASE_WIDTH_1BYTE:
		iter.cp8 = DeeString_Get1Byte((DeeObject *)self);
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
		iter.cp16 = DeeString_Get2Byte((DeeObject *)self);
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
		iter.cp32 = DeeString_Get4Byte((DeeObject *)self);
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
	return (DREF String *)unicode_printer_pack(&printer);
err:
	unicode_printer_fini(&printer);
	return NULL;
}

#ifdef NDEBUG
PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeString_New2Byte(uint16_t const *__restrict str,
                   size_t length)
#else /* NDEBUG */
PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeDbgString_New2Byte(uint16_t const *__restrict str,
                      size_t length, char const *file, int line)
#endif /* !NDEBUG */
{
	uint16_t *buffer;
#ifdef NDEBUG
	buffer = DeeString_New2ByteBuffer(length);
#else /* NDEBUG */
	buffer = DeeDbgString_New2ByteBuffer(length, file, line);
#endif /* !NDEBUG */
	if unlikely(!buffer)
		goto err;
	buffer = memcpyw(buffer, str, length);
	return DeeString_Pack2ByteBuffer(buffer);
err:
	return NULL;
}

#ifdef NDEBUG
PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *
(DCALL DeeString_New4Byte)(uint32_t const *__restrict str,
                           size_t length)
#else /* NDEBUG */
PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *
(DCALL DeeDbgString_New4Byte)(uint32_t const *__restrict str,
                              size_t length, char const *file, int line)
#endif /* !NDEBUG */
{
	uint32_t *buffer;
#ifdef NDEBUG
	buffer = DeeString_New4ByteBuffer(length);
#else /* NDEBUG */
	buffer = DeeDbgString_New4ByteBuffer(length, file, line);
#endif /* !NDEBUG */
	if unlikely(!buffer)
		goto err;
	buffer = memcpyl(buffer, str, length);
	return DeeString_Pack4ByteBuffer(buffer);
err:
	return NULL;
}

#ifdef NDEBUG
PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *
(DCALL DeeDbgString_New2Byte)(uint16_t const *__restrict str,
                              size_t length, char const *file, int line) {
	(void)file;
	(void)line;
	return DeeString_New2Byte(str, length);
}
PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *
(DCALL DeeDbgString_New4Byte)(uint32_t const *__restrict str,
                              size_t length, char const *file, int line) {
	(void)file;
	(void)line;
	return DeeString_New4Byte(str, length);
}
#else /* NDEBUG */
PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *
(DCALL DeeString_New2Byte)(uint16_t const *__restrict str,
                           size_t length) {
	return DeeDbgString_New2Byte(str, length, NULL, 0);
}
PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *
(DCALL DeeString_New4Byte)(uint32_t const *__restrict str,
                           size_t length) {
	return DeeDbgString_New4Byte(str, length, NULL, 0);
}
#endif /* !NDEBUG */

/* Construct strings from UTF-16/32 encoded content.
 * @param: error_mode: One of `STRING_ERROR_F*' */
PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeString_NewUtf16(uint16_t const *__restrict str,
                   size_t length,
                   unsigned int error_mode) {
	uint16_t *buffer;
	buffer = DeeString_New2ByteBuffer(length);
	if unlikely(!buffer)
		goto err;
	memcpyw(buffer, str, length);
	return DeeString_PackUtf16Buffer(buffer, error_mode);
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeString_NewUtf32(uint32_t const *__restrict str,
                   size_t length,
                   unsigned int error_mode) {
	uint32_t *buffer;
	buffer = DeeString_New4ByteBuffer(length);
	if unlikely(!buffer)
		goto err;
	memcpyl(buffer, str, length);
	return DeeString_PackUtf32Buffer(buffer, error_mode);
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeString_NewUtf16AltEndian(uint16_t const *__restrict str,
                            size_t length,
                            unsigned int error_mode) {
	uint16_t *buffer;
	size_t i;
	buffer = DeeString_New2ByteBuffer(length);
	if unlikely(!buffer)
		goto err;
	for (i = 0; i < length; ++i)
		buffer[i] = BSWAP16(str[i]);
	return DeeString_PackUtf16Buffer(buffer, error_mode);
err:
	return NULL;
}

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeString_NewUtf32AltEndian(uint32_t const *__restrict str,
                            size_t length,
                            unsigned int error_mode) {
	uint32_t *buffer;
	size_t i;
	buffer = DeeString_New4ByteBuffer(length);
	if unlikely(!buffer)
		goto err;
	for (i = 0; i < length; ++i)
		buffer[i] = BSWAP32(str[i]);
	return DeeString_PackUtf32Buffer(buffer, error_mode);
err:
	return NULL;
}



#ifndef PRIVATE_REPLACE_KWLIST_DEFINED
#define PRIVATE_REPLACE_KWLIST_DEFINED 1
PRIVATE struct keyword replace_kwlist[] = { K(find), K(replace), K(max), KEND };
#endif /* !PRIVATE_REPLACE_KWLIST_DEFINED */

PRIVATE WUNUSED DREF String *DCALL
string_replace(String *__restrict self, size_t argc,
               DeeObject *const *argv, DeeObject *Kw) {
	String *find, *replace;
	size_t max_count = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, Kw, replace_kwlist, "oo|" UNPuSIZ ":replace", &find, &replace, &max_count))
		goto err;
	if (DeeObject_AssertTypeExact(find, &DeeString_Type))
		goto err;
	if (DeeObject_AssertTypeExact(replace, &DeeString_Type))
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
				if unlikely(unicode_printer_print8(&p, begin.cp8, (size_t)(ptr.cp8 - begin.cp8)) < 0)
					goto err_printer;
				if unlikely(unicode_printer_printstring(&p, (DeeObject *)replace) < 0)
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
				if unlikely(unicode_printer_print16(&p, begin.cp16, (size_t)(ptr.cp16 - begin.cp16)) < 0)
					goto err_printer;
				if unlikely(unicode_printer_printstring(&p, (DeeObject *)replace) < 0)
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
				if unlikely(unicode_printer_print32(&p, begin.cp32, (size_t)(ptr.cp32 - begin.cp32)) < 0)
					goto err_printer;
				if unlikely(unicode_printer_printstring(&p, (DeeObject *)replace) < 0)
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

PRIVATE WUNUSED DREF String *DCALL
string_casereplace(String *__restrict self, size_t argc,
                   DeeObject *const *argv, DeeObject *kw) {
	String *find, *replace;
	size_t max_count = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, replace_kwlist, "oo|" UNPuSIZ ":casereplace", &find, &replace, &max_count))
		goto err;
	if (DeeObject_AssertTypeExact(find, &DeeString_Type))
		goto err;
	if (DeeObject_AssertTypeExact(replace, &DeeString_Type))
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
				if unlikely(unicode_printer_print8(&p, begin.cp8, (size_t)(ptr.cp8 - begin.cp8)) < 0)
					goto err_printer;
				if unlikely(unicode_printer_printstring(&p, (DeeObject *)replace) < 0)
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
				if unlikely(unicode_printer_print16(&p, begin.cp16, (size_t)(ptr.cp16 - begin.cp16)) < 0)
					goto err_printer;
				if unlikely(unicode_printer_printstring(&p, (DeeObject *)replace) < 0)
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
				if unlikely(unicode_printer_print32(&p, begin.cp32, (size_t)(ptr.cp32 - begin.cp32)) < 0)
					goto err_printer;
				if unlikely(unicode_printer_printstring(&p, (DeeObject *)replace) < 0)
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

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
string_ord(String *self, size_t argc, DeeObject *const *argv) {
	size_t index = 0;
	if (argc) {
		if (DeeArg_Unpack(argc, argv, UNPuSIZ ":ord", &index))
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
	return DeeInt_NewUInt32(DeeString_GetChar((DeeObject *)self, index));
err:
	return NULL;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
string_bytes(String *self, size_t argc,
             DeeObject *const *argv) {
	bool allow_invalid = false;
	size_t mylen, start = 0, end = (size_t)-1;
	uint8_t *my_bytes;
	if (argc == 1) {
		int temp = DeeObject_Bool(argv[0]);
		if unlikely(temp < 0)
			goto err;
		allow_invalid = !!temp;
	} else {
		if (DeeArg_Unpack(argc, argv, "|" UNPdSIZ UNPdSIZ "b:Bytes", &start, &end, &allow_invalid))
			goto err;
	}
	my_bytes = DeeString_AsBytes((DeeObject *)self, allow_invalid);
	if unlikely(!my_bytes)
		goto err;
	mylen = WSTR_LENGTH(my_bytes);
	CLAMP_SUBSTR(&start, &end, &mylen, empty_substr);
	return DeeBytes_NewView((DeeObject *)self, my_bytes + start,
	                        mylen, Dee_BUFFER_FREADONLY);
empty_substr:
	return_empty_bytes;
err:
	return NULL;
}

#ifndef PRIVATE_SUBSTR_KWLIST_DEFINED
#define PRIVATE_SUBSTR_KWLIST_DEFINED 1
PRIVATE struct keyword substr_kwlist[] = { K(start), K(end), KEND };
#endif /* !PRIVATE_SUBSTR_KWLIST_DEFINED */



#define DEFINE_STRING_TRAIT(name, function, test_ch)                   \
	PRIVATE WUNUSED DREF DeeObject *DCALL                              \
	string_##name(String *self, size_t argc, DeeObject *const *argv) { \
		size_t start = 0, end = (size_t)-1;                            \
		if (argc == 1) {                                               \
			uint32_t ch;                                               \
			if (DeeObject_AsSize(argv[0], &start))                     \
				goto err;                                              \
			if unlikely(start >= DeeString_WLEN(self)) {               \
				err_index_out_of_bounds((DeeObject *)self,             \
				                        start,                         \
				                        DeeString_WLEN(self));         \
				goto err;                                              \
			}                                                          \
			ch = DeeString_GetChar(self, start);                       \
			return_bool(test_ch);                                      \
		} else {                                                       \
			if (DeeArg_Unpack(argc, argv,                              \
			                  "|" UNPdSIZ UNPdSIZ ":" #name,           \
			                  &start, &end))                           \
				goto err;                                              \
			return_bool(function(self, start, end));                   \
		}                                                              \
	err:                                                               \
		return NULL;                                                   \
	}
#define DEFINE_ANY_STRING_TRAIT(name, function)                                       \
	PRIVATE WUNUSED DREF DeeObject *DCALL                                             \
	string_##name(String *self, size_t argc, DeeObject *const *argv, DeeObject *kw) { \
		size_t start = 0, end = (size_t)-1;                                           \
		if (DeeArg_UnpackKw(argc, argv, kw, substr_kwlist,                            \
		                    "|" UNPdSIZ UNPdSIZ ":" #name,                            \
		                    &start, &end))                                            \
			goto err;                                                                 \
		return_bool(function(self, start, end));                                      \
	err:                                                                              \
		return NULL;                                                                  \
	}
DEFINE_STRING_TRAIT(iscntrl, DeeString_IsCntrl, DeeUni_IsCntrl(ch))
DEFINE_STRING_TRAIT(istab, DeeString_IsTab, DeeUni_IsTab(ch))
DEFINE_STRING_TRAIT(iscempty, DeeString_IsCempty, DeeUni_IsEmpty(ch))
DEFINE_STRING_TRAIT(iswhite, DeeString_IsWhite, DeeUni_IsWhite(ch))
DEFINE_STRING_TRAIT(islf, DeeString_IsLF, DeeUni_IsLF(ch))
DEFINE_STRING_TRAIT(isspace, DeeString_IsSpace, DeeUni_IsSpace(ch))
DEFINE_STRING_TRAIT(islower, DeeString_IsLower, DeeUni_IsLower(ch))
DEFINE_STRING_TRAIT(isupper, DeeString_IsUpper, DeeUni_IsUpper(ch))
DEFINE_STRING_TRAIT(isalpha, DeeString_IsAlpha, DeeUni_IsAlpha(ch))
DEFINE_STRING_TRAIT(isdigit, DeeString_IsDigit, DeeUni_IsDigit(ch))
DEFINE_STRING_TRAIT(ishex, DeeString_IsHex, DeeUni_IsHex(ch))
DEFINE_STRING_TRAIT(isxdigit, DeeString_IsXdigit, DeeUni_IsXDigit(ch))
DEFINE_STRING_TRAIT(isalnum, DeeString_IsAlnum, DeeUni_IsAlnum(ch))
DEFINE_STRING_TRAIT(ispunct, DeeString_IsPunct, DeeUni_IsPunct(ch))
DEFINE_STRING_TRAIT(isgraph, DeeString_IsGraph, DeeUni_IsGraph(ch))
DEFINE_STRING_TRAIT(isprint, DeeString_IsPrint, DeeUni_IsPrint(ch))
DEFINE_STRING_TRAIT(isblank, DeeString_IsBlank, DeeUni_IsBlank(ch))
DEFINE_STRING_TRAIT(istitle, DeeString_IsTitle, DeeUni_IsTitle(ch))
DEFINE_STRING_TRAIT(isnumeric, DeeString_IsNumeric, DeeUni_IsNumeric(ch))
DEFINE_STRING_TRAIT(issymstrt, DeeString_IsSymStrt, DeeUni_IsSymStrt(ch))
DEFINE_STRING_TRAIT(issymcont, DeeString_IsSymCont, DeeUni_IsSymCont(ch))
DEFINE_STRING_TRAIT(issymbol, DeeString_IsSymbol, DeeUni_IsSymStrt(ch))
DEFINE_STRING_TRAIT(isascii, DeeString_IsAscii, ch <= 0x7f)
DEFINE_ANY_STRING_TRAIT(isanycntrl, DeeString_IsAnyCntrl)
DEFINE_ANY_STRING_TRAIT(isanytab, DeeString_IsAnyTab)
DEFINE_ANY_STRING_TRAIT(isanycempty, DeeString_IsAnyCempty)
DEFINE_ANY_STRING_TRAIT(isanywhite, DeeString_IsAnyWhite)
DEFINE_ANY_STRING_TRAIT(isanylf, DeeString_IsAnyLF)
DEFINE_ANY_STRING_TRAIT(isanyspace, DeeString_IsAnySpace)
DEFINE_ANY_STRING_TRAIT(isanylower, DeeString_IsAnyLower)
DEFINE_ANY_STRING_TRAIT(isanyupper, DeeString_IsAnyUpper)
DEFINE_ANY_STRING_TRAIT(isanyalpha, DeeString_IsAnyAlpha)
DEFINE_ANY_STRING_TRAIT(isanydigit, DeeString_IsAnyDigit)
DEFINE_ANY_STRING_TRAIT(isanyhex, DeeString_IsAnyHex)
DEFINE_ANY_STRING_TRAIT(isanyxdigit, DeeString_IsAnyXdigit)
DEFINE_ANY_STRING_TRAIT(isanyalnum, DeeString_IsAnyAlnum)
DEFINE_ANY_STRING_TRAIT(isanypunct, DeeString_IsAnyPunct)
DEFINE_ANY_STRING_TRAIT(isanygraph, DeeString_IsAnyGraph)
DEFINE_ANY_STRING_TRAIT(isanyprint, DeeString_IsAnyPrint)
DEFINE_ANY_STRING_TRAIT(isanyblank, DeeString_IsAnyBlank)
DEFINE_ANY_STRING_TRAIT(isanytitle, DeeString_IsAnyTitle)
DEFINE_ANY_STRING_TRAIT(isanynumeric, DeeString_IsAnyNumeric)
DEFINE_ANY_STRING_TRAIT(isanysymstrt, DeeString_IsAnySymStrt)
DEFINE_ANY_STRING_TRAIT(isanysymcont, DeeString_IsAnySymCont)
DEFINE_ANY_STRING_TRAIT(isanyascii, DeeString_IsAnyAscii)
#undef DEFINE_ANY_STRING_TRAIT
#undef DEFINE_STRING_TRAIT

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
string_asdigit(String *self, size_t argc, DeeObject *const *argv) {
	uint32_t ch;
	DeeObject *defl = NULL;
	struct unitraits *trt;
	if (argc == 0) {
		if unlikely(DeeString_WLEN(self) != 1)
			goto err_not_single_char;
		ch = DeeString_GetChar(self, 0);
	} else {
		size_t index;
		if (DeeArg_Unpack(argc, argv, UNPuSIZ "|o:asdigit", &index, &defl))
			goto err;
		if unlikely(index >= DeeString_WLEN(self)) {
			err_index_out_of_bounds((DeeObject *)self, index,
			                        DeeString_WLEN(self));
			goto err;
		}
		ch = DeeString_GetChar(self, index);
	}
	trt = DeeUni_Descriptor(ch);
	if (!(trt->ut_flags & UNICODE_ISDIGIT))
		goto err_not_numeric;
	if likely(trt->ut_digit_idx < Dee_UNICODE_DIGIT_IDENTITY_COUNT)
		return DeeInt_NewUInt8(trt->ut_digit_idx);
	return DeeInt_NewUInt64(DeeUni_GetNumericIdx64(trt->ut_digit_idx));
err_not_numeric:
	if (defl)
		return_reference_(defl);
	DeeError_Throwf(&DeeError_ValueError,
	                "Unicode character %I32C is not a digit",
	                ch);
	goto err;
err_not_single_char:
	err_expected_single_character_string((DeeObject *)self);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
string_asxdigit(String *self, size_t argc, DeeObject *const *argv) {
	uint32_t ch;
	DeeObject *defl = NULL;
	struct unitraits *trt;
	if (argc == 0) {
		if unlikely(DeeString_WLEN(self) != 1)
			goto err_not_single_char;
		ch = DeeString_GetChar(self, 0);
	} else {
		size_t index;
		if (DeeArg_Unpack(argc, argv, UNPuSIZ "|o:asxdigit", &index, &defl))
			goto err;
		if unlikely(index >= DeeString_WLEN(self)) {
			err_index_out_of_bounds((DeeObject *)self, index,
			                        DeeString_WLEN(self));
			goto err;
		}
		ch = DeeString_GetChar(self, index);
	}
	trt = DeeUni_Descriptor(ch);
	if (!(trt->ut_flags & UNICODE_ISXDIGIT))
		goto err_not_numeric;
	if likely(trt->ut_digit_idx < Dee_UNICODE_DIGIT_IDENTITY_COUNT)
		return DeeInt_NewUInt8(trt->ut_digit_idx);
	return DeeInt_NewUInt64(DeeUni_GetNumericIdx64(trt->ut_digit_idx));
err_not_numeric:
	if (defl)
		return_reference_(defl);
	DeeError_Throwf(&DeeError_ValueError,
	                "Unicode character %I32C is not a digit",
	                ch);
	goto err;
err_not_single_char:
	err_expected_single_character_string((DeeObject *)self);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
string_asnumeric(String *self, size_t argc, DeeObject *const *argv) {
	uint32_t ch;
	DeeObject *defl = NULL;
	struct unitraits *trt;
	double floatval;
	if (argc == 0) {
		if unlikely(DeeString_WLEN(self) != 1)
			goto err_not_single_char;
		ch = DeeString_GetChar(self, 0);
	} else {
		size_t index;
		if (DeeArg_Unpack(argc, argv, UNPuSIZ "|o:asnumeric", &index, &defl))
			goto err;
		if unlikely(index >= DeeString_WLEN(self)) {
			err_index_out_of_bounds((DeeObject *)self, index,
			                        DeeString_WLEN(self));
			goto err;
		}
		ch = DeeString_GetChar(self, index);
	}
	trt = DeeUni_Descriptor(ch);
	if (!(trt->ut_flags & UNICODE_ISNUMERIC))
		goto err_not_numeric;
	if likely(trt->ut_digit_idx < Dee_UNICODE_DIGIT_IDENTITY_COUNT)
		return DeeInt_NewUInt8(trt->ut_digit_idx);
	floatval = DeeUni_GetNumericIdxD(trt->ut_digit_idx);
	if ((double)(uint64_t)floatval == floatval)
		return DeeInt_NewUInt64(DeeUni_GetNumericIdx64(trt->ut_digit_idx));
	return DeeFloat_New(floatval);
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


INTDEF WUNUSED NONNULL((1)) DREF String *DCALL DeeString_Convert(String *__restrict self, size_t start, size_t end, uintptr_t kind);
INTDEF WUNUSED NONNULL((1)) DREF String *DCALL DeeString_ToTitle(String *__restrict self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) DREF String *DCALL DeeString_Capitalize(String *__restrict self, size_t start, size_t end);
INTDEF WUNUSED NONNULL((1)) DREF String *DCALL DeeString_Swapcase(String *__restrict self, size_t start, size_t end);
#define DeeString_ToLower(self, start, end) DeeString_Convert(self, start, end, UNICODE_CONVERT_LOWER)
#define DeeString_ToUpper(self, start, end) DeeString_Convert(self, start, end, UNICODE_CONVERT_UPPER)


PRIVATE WUNUSED NONNULL((1)) DREF String *DCALL
string_lower(String *self, size_t argc,
             DeeObject *const *argv, DeeObject *kw) {
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, substr_kwlist,
	                    "|" UNPdSIZ UNPdSIZ ":lower",
	                    &start, &end))
		goto err;
	return DeeString_ToLower(self, start, end);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF String *DCALL
string_upper(String *self, size_t argc,
             DeeObject *const *argv, DeeObject *kw) {
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, substr_kwlist,
	                    "|" UNPdSIZ UNPdSIZ ":upper",
	                    &start, &end))
		goto err;
	return DeeString_ToUpper(self, start, end);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF String *DCALL
string_title(String *self, size_t argc,
             DeeObject *const *argv, DeeObject *kw) {
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, substr_kwlist,
	                    "|" UNPdSIZ UNPdSIZ ":title",
	                    &start, &end))
		goto err;
	return DeeString_ToTitle(self, start, end);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF String *DCALL
string_capitalize(String *self, size_t argc,
                  DeeObject *const *argv, DeeObject *kw) {
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, substr_kwlist,
	                    "|" UNPdSIZ UNPdSIZ ":capitalize",
	                    &start, &end))
		goto err;
	return DeeString_Capitalize(self, start, end);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF String *DCALL
string_swapcase(String *self, size_t argc,
                DeeObject *const *argv, DeeObject *kw) {
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, substr_kwlist,
	                    "|" UNPdSIZ UNPdSIZ ":swapcase",
	                    &start, &end))
		goto err;
	return DeeString_Swapcase(self, start, end);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF String *DCALL
string_casefold(String *self, size_t argc,
                DeeObject *const *argv, DeeObject *kw) {
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, substr_kwlist,
	                    "|" UNPdSIZ UNPdSIZ ":casefold",
	                    &start, &end))
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
		return (DREF String *)unicode_printer_pack(&printer);
return_self:
		unicode_printer_fini(&printer);
		return_reference_(self);
return_empty:
		unicode_printer_fini(&printer);
		return_reference_((String *)Dee_EmptyString);
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

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
string_find(String *self, size_t argc,
            DeeObject *const *argv, DeeObject *kw) {
	String *other;
	size_t start = 0, end = (size_t)-1;
	size_t result;
	union dcharptr ptr, lhs, rhs;
	size_t mylen;
	if (DeeArg_UnpackKw(argc, argv, kw, find_kwlist,
	                    "o|" UNPdSIZ UNPdSIZ ":find",
	                    &other, &start, &end))
		goto err;
	if (DeeObject_AssertTypeExact(other, &DeeString_Type))
		goto err;
	SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON(DeeString_WIDTH(self),
	                                        DeeString_WIDTH(other))) {

	CASE_WIDTH_1BYTE:
		lhs.cp8 = DeeString_As1Byte((DeeObject *)self);
		mylen = DeeString_WLEN(self);
		CLAMP_SUBSTR(&start, &end, &mylen, not_found);
		rhs.cp8 = DeeString_As1Byte((DeeObject *)other);
		ptr.cp8 = memmemb(lhs.cp8 + start, mylen,
		                  rhs.cp8, WSTR_LENGTH(rhs.cp8));
		if (!ptr.cp8)
			goto not_found;
		result = (size_t)(ptr.cp8 - lhs.cp8);
		break;

	CASE_WIDTH_2BYTE:
		lhs.cp16 = DeeString_As2Byte((DeeObject *)self);
		if unlikely(!lhs.cp16)
			goto err;
		mylen = DeeString_WLEN(self);
		CLAMP_SUBSTR(&start, &end, &mylen, not_found);
		rhs.cp16 = DeeString_As2Byte((DeeObject *)other);
		if unlikely(!rhs.cp16)
			goto err;
		ptr.cp16 = memmemw(lhs.cp16 + start, mylen,
		                   rhs.cp16, WSTR_LENGTH(rhs.cp16));
		if (!ptr.cp16)
			goto not_found;
		result = (size_t)(ptr.cp16 - lhs.cp16);
		break;

	CASE_WIDTH_4BYTE:
		lhs.cp32 = DeeString_As4Byte((DeeObject *)self);
		if unlikely(!lhs.cp32)
			goto err;
		mylen = WSTR_LENGTH(lhs.ptr);
		CLAMP_SUBSTR(&start, &end, &mylen, not_found);
		rhs.cp32 = DeeString_As4Byte((DeeObject *)other);
		if unlikely(!rhs.cp32)
			goto err;
		ptr.cp32 = memmeml(lhs.cp32 + start, mylen,
		                   rhs.cp32, WSTR_LENGTH(rhs.cp32));
		if (!ptr.cp32)
			goto not_found;
		result = (size_t)(ptr.cp32 - lhs.cp32);
		break;
	}
	return DeeInt_NewSize(result);
not_found:
	return_reference_(DeeInt_MinusOne);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
string_rfind(String *self, size_t argc,
             DeeObject *const *argv, DeeObject *kw) {
	String *other;
	size_t start = 0, end = (size_t)-1;
	size_t result;
	union dcharptr ptr, lhs, rhs;
	size_t mylen;
	if (DeeArg_UnpackKw(argc, argv, kw, find_kwlist,
	                    "o|" UNPdSIZ UNPdSIZ ":rfind",
	                    &other, &start, &end))
		goto err;
	if (DeeObject_AssertTypeExact(other, &DeeString_Type))
		goto err;
	SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON(DeeString_WIDTH(self),
	                                        DeeString_WIDTH(other))) {

	CASE_WIDTH_1BYTE:
		lhs.cp8 = DeeString_As1Byte((DeeObject *)self);
		mylen   = WSTR_LENGTH(lhs.ptr);
		CLAMP_SUBSTR(&start, &end, &mylen, not_found);
		rhs.cp8 = DeeString_As1Byte((DeeObject *)other);
		ptr.cp8 = memrmemb(lhs.cp8 + start, mylen,
		                   rhs.cp8, WSTR_LENGTH(rhs.cp8));
		if (!ptr.cp8)
			goto not_found;
		result = (size_t)(ptr.cp8 - lhs.cp8);
		break;

	CASE_WIDTH_2BYTE:
		lhs.cp16 = DeeString_As2Byte((DeeObject *)self);
		if unlikely(!lhs.cp16)
			goto err;
		mylen = WSTR_LENGTH(lhs.ptr);
		CLAMP_SUBSTR(&start, &end, &mylen, not_found);
		rhs.cp16 = DeeString_As2Byte((DeeObject *)other);
		if unlikely(!rhs.cp16)
			goto err;
		ptr.cp16 = memrmemw(lhs.cp16 + start, mylen,
		                    rhs.cp16, WSTR_LENGTH(rhs.cp16));
		if (!ptr.cp16)
			goto not_found;
		result = (size_t)(ptr.cp16 - lhs.cp16);
		break;

	CASE_WIDTH_4BYTE:
		lhs.cp32 = DeeString_As4Byte((DeeObject *)self);
		if unlikely(!lhs.cp32)
			goto err;
		mylen = WSTR_LENGTH(lhs.ptr);
		CLAMP_SUBSTR(&start, &end, &mylen, not_found);
		rhs.cp32 = DeeString_As4Byte((DeeObject *)other);
		if unlikely(!rhs.cp32)
			goto err;
		ptr.cp32 = memrmeml(lhs.cp32 + start, mylen,
		                    rhs.cp32, WSTR_LENGTH(rhs.cp32));
		if (!ptr.cp32)
			goto not_found;
		result = (size_t)(ptr.cp32 - lhs.cp32);
		break;
	}
	return DeeInt_NewSize(result);
not_found:
	return_reference_(DeeInt_MinusOne);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
string_index(String *self, size_t argc,
             DeeObject *const *argv, DeeObject *kw) {
	String *other;
	size_t start = 0, end = (size_t)-1;
	size_t result;
	union dcharptr ptr, lhs, rhs;
	size_t mylen;
	if (DeeArg_UnpackKw(argc, argv, kw, find_kwlist,
	                    "o|" UNPdSIZ UNPdSIZ ":index",
	                    &other, &start, &end))
		goto err;
	if (DeeObject_AssertTypeExact(other, &DeeString_Type))
		goto err;
	SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON(DeeString_WIDTH(self),
	                                        DeeString_WIDTH(other))) {

	CASE_WIDTH_1BYTE:
		lhs.cp8 = DeeString_As1Byte((DeeObject *)self);
		mylen   = WSTR_LENGTH(lhs.ptr);
		CLAMP_SUBSTR(&start, &end, &mylen, not_found);
		rhs.cp8 = DeeString_As1Byte((DeeObject *)other);
		ptr.cp8 = memmemb(lhs.cp8 + start, mylen,
		                  rhs.cp8, WSTR_LENGTH(rhs.cp8));
		if unlikely(!ptr.cp8)
			goto not_found;
		result = (size_t)(ptr.cp8 - lhs.cp8);
		break;

	CASE_WIDTH_2BYTE:
		lhs.cp16 = DeeString_As2Byte((DeeObject *)self);
		if unlikely(!lhs.cp16)
			goto err;
		mylen = WSTR_LENGTH(lhs.ptr);
		CLAMP_SUBSTR(&start, &end, &mylen, not_found);
		rhs.cp16 = DeeString_As2Byte((DeeObject *)other);
		if unlikely(!rhs.cp16)
			goto err;
		ptr.cp16 = memmemw(lhs.cp16 + start, mylen,
		                   rhs.cp16, WSTR_LENGTH(rhs.cp16));
		if unlikely(!ptr.cp16)
			goto not_found;
		result = (size_t)(ptr.cp16 - lhs.cp16);
		break;

	CASE_WIDTH_4BYTE:
		lhs.cp32 = DeeString_As4Byte((DeeObject *)self);
		if unlikely(!lhs.cp32)
			goto err;
		mylen = WSTR_LENGTH(lhs.ptr);
		CLAMP_SUBSTR(&start, &end, &mylen, not_found);
		rhs.cp32 = DeeString_As4Byte((DeeObject *)other);
		if unlikely(!rhs.cp32)
			goto err;
		ptr.cp32 = memmeml(lhs.cp32 + start, mylen,
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

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
string_rindex(String *self, size_t argc,
              DeeObject *const *argv, DeeObject *kw) {
	String *other;
	size_t start = 0, end = (size_t)-1;
	size_t result;
	union dcharptr ptr, lhs, rhs;
	size_t mylen;
	if (DeeArg_UnpackKw(argc, argv, kw, find_kwlist,
	                    "o|" UNPdSIZ UNPdSIZ ":rindex",
	                    &other, &start, &end))
		goto err;
	if (DeeObject_AssertTypeExact(other, &DeeString_Type))
		goto err;
	SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON(DeeString_WIDTH(self),
	                                        DeeString_WIDTH(other))) {

	CASE_WIDTH_1BYTE:
		lhs.cp8 = DeeString_As1Byte((DeeObject *)self);
		mylen   = WSTR_LENGTH(lhs.ptr);
		CLAMP_SUBSTR(&start, &end, &mylen, not_found);
		rhs.cp8 = DeeString_As1Byte((DeeObject *)other);
		ptr.cp8 = memrmemb(lhs.cp8 + start, mylen,
		                   rhs.cp8, WSTR_LENGTH(rhs.cp8));
		if unlikely(!ptr.cp8)
			goto not_found;
		result = (size_t)(ptr.cp8 - lhs.cp8);
		break;

	CASE_WIDTH_2BYTE:
		lhs.cp16 = DeeString_As2Byte((DeeObject *)self);
		if unlikely(!lhs.cp16)
			goto err;
		mylen = WSTR_LENGTH(lhs.ptr);
		CLAMP_SUBSTR(&start, &end, &mylen, not_found);
		rhs.cp16 = DeeString_As2Byte((DeeObject *)other);
		if unlikely(!rhs.cp16)
			goto err;
		ptr.cp16 = memrmemw(lhs.cp16 + start, mylen,
		                    rhs.cp16, WSTR_LENGTH(rhs.cp16));
		if unlikely(!ptr.cp16)
			goto not_found;
		result = (size_t)(ptr.cp16 - lhs.cp16);
		break;

	CASE_WIDTH_4BYTE:
		lhs.cp32 = DeeString_As4Byte((DeeObject *)self);
		if unlikely(!lhs.cp32)
			goto err;
		mylen = WSTR_LENGTH(lhs.ptr);
		CLAMP_SUBSTR(&start, &end, &mylen, not_found);
		rhs.cp32 = DeeString_As4Byte((DeeObject *)other);
		if unlikely(!rhs.cp32)
			goto err;
		ptr.cp32 = memrmeml(lhs.cp32 + start, mylen,
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


#ifndef DeeTuple_NewII_DEFINED
#define DeeTuple_NewII_DEFINED
#ifdef __OPTIMIZE_SIZE__
#define DeeTuple_NewII(a, b) DeeTuple_Newf(PCKuSIZ PCKuSIZ, a, b)
#else /* __OPTIMIZE_SIZE__ */
PRIVATE WUNUSED DREF DeeObject *DCALL
DeeTuple_NewII(size_t a, size_t b) {
	DREF DeeObject *aval, *bval;
	DREF DeeTupleObject *result;
	aval = DeeInt_NewSize(a);
	if unlikely(!aval)
		goto err;
	bval = DeeInt_NewSize(b);
	if unlikely(!bval)
		goto err_aval;
	result = DeeTuple_NewUninitialized(2);
	if unlikely(!result)
		goto err_aval_bval;
	DeeTuple_SET(result, 0, aval); /* Inherit reference */
	DeeTuple_SET(result, 1, bval); /* Inherit reference */
	return (DREF DeeObject *)result;
err_aval_bval:
	Dee_Decref(bval);
err_aval:
	Dee_Decref(aval);
err:
	return NULL;
}
#endif /* !__OPTIMIZE_SIZE__ */
#endif /* !DeeTuple_NewII_DEFINED */

struct string_findany_data {
	String *sfad_self;   /* [1..1][const] The string being searched. */
	size_t  sfad_base;   /* [const] Character offset into `sfad_self' of search start. */
	size_t  sfad_size;   /* [const] Sub-string length to search in `sfad_self', starting at `sfad_base'. */
	size_t  sfad_result; /* [<= sfad_size] Offset from `sfad_base' of best match. */
};

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
string_findany_cb(void *arg, DeeObject *elem) {
	struct string_findany_data *data = (struct string_findany_data *)arg;
	union dcharptr ptr, lhs, rhs;
	size_t search_size;
	if (DeeObject_AssertTypeExact(elem, &DeeString_Type))
		goto err;
	SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON(DeeString_WIDTH(data->sfad_self),
	                                        DeeString_WIDTH(elem))) {

	CASE_WIDTH_1BYTE:
		lhs.cp8 = DeeString_As1Byte((DeeObject *)data->sfad_self);
		lhs.cp8 += data->sfad_base;
		rhs.cp8 = DeeString_As1Byte((DeeObject *)elem);
		search_size = data->sfad_result + WSTR_LENGTH(rhs.cp8) - 1;
		if (search_size > data->sfad_size)
			search_size = data->sfad_size;
		ptr.cp8 = memmemb(lhs.cp8, search_size,
		                  rhs.cp8, WSTR_LENGTH(rhs.cp8));
		if (ptr.cp8 != NULL) {
			size_t hit = (size_t)(ptr.cp8 - lhs.cp8);
			ASSERT(hit < data->sfad_result);
			data->sfad_result = hit;
			if (hit == 0)
				return -2; /* Found hit at offset=0 -> can stop enumerating needles. */
		}
		break;

	CASE_WIDTH_2BYTE:
		lhs.cp16 = DeeString_As2Byte((DeeObject *)data->sfad_self);
		if unlikely(!lhs.cp16)
			goto err;
		lhs.cp16 += data->sfad_base;
		rhs.cp16 = DeeString_As2Byte((DeeObject *)elem);
		if unlikely(!rhs.cp16)
			goto err;
		search_size = data->sfad_result + WSTR_LENGTH(rhs.cp16) - 1;
		if (search_size > data->sfad_size)
			search_size = data->sfad_size;
		ptr.cp16 = memmemw(lhs.cp16, search_size,
		                   rhs.cp16, WSTR_LENGTH(rhs.cp16));
		if (ptr.cp16 != NULL) {
			size_t hit = (size_t)(ptr.cp16 - lhs.cp16);
			ASSERT(hit < data->sfad_result);
			data->sfad_result = hit;
			if (hit == 0)
				return -2; /* Found hit at offset=0 -> can stop enumerating needles. */
		}
		break;

	CASE_WIDTH_4BYTE:
		lhs.cp32 = DeeString_As4Byte((DeeObject *)data->sfad_self);
		if unlikely(!lhs.cp32)
			goto err;
		lhs.cp32 += data->sfad_base;
		rhs.cp32 = DeeString_As4Byte((DeeObject *)elem);
		if unlikely(!rhs.cp32)
			goto err;
		search_size = data->sfad_result + WSTR_LENGTH(rhs.cp32) - 1;
		if (search_size > data->sfad_size)
			search_size = data->sfad_size;
		ptr.cp32 = memmeml(lhs.cp32, search_size,
		                   rhs.cp32, WSTR_LENGTH(rhs.cp32));
		if (ptr.cp32 != NULL) {
			size_t hit = (size_t)(ptr.cp32 - lhs.cp32);
			ASSERT(hit < data->sfad_result);
			data->sfad_result = hit;
			if (hit == 0)
				return -2; /* Found hit at offset=0 -> can stop enumerating needles. */
		}
		break;
	}
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
string_findany(String *self, size_t argc,
               DeeObject *const *argv, DeeObject *kw) {
	size_t start = 0, end = (size_t)-1;
	DeeObject *needles;
	struct string_findany_data data;
	if (DeeArg_UnpackKw(argc, argv, kw, find_kwlist,
	                    "o|" UNPdSIZ UNPdSIZ ":findany",
	                    &needles, &start, &end))
		goto err;
	data.sfad_self = self;
	data.sfad_size = DeeString_WLEN(self);
	CLAMP_SUBSTR(&start, &end, &data.sfad_size, not_found);
	data.sfad_base   = start;
	data.sfad_result = data.sfad_size;
	if unlikely(DeeObject_Foreach(needles, &string_findany_cb, &data) == -1)
		goto err;
	if (data.sfad_result < data.sfad_size)
		return DeeInt_NewSize(data.sfad_base + data.sfad_result);
not_found:
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
string_indexany(String *self, size_t argc,
                DeeObject *const *argv, DeeObject *kw) {
	size_t start = 0, end = (size_t)-1;
	DeeObject *needles;
	struct string_findany_data data;
	if (DeeArg_UnpackKw(argc, argv, kw, find_kwlist,
	                    "o|" UNPdSIZ UNPdSIZ ":indexany",
	                    &needles, &start, &end))
		goto err;
	data.sfad_self = self;
	data.sfad_size = DeeString_WLEN(self);
	CLAMP_SUBSTR(&start, &end, &data.sfad_size, not_found);
	data.sfad_base   = start;
	data.sfad_result = data.sfad_size;
	if unlikely(DeeObject_Foreach(needles, &string_findany_cb, &data) == -1)
		goto err;
	if (data.sfad_result < data.sfad_size)
		return DeeInt_NewSize(data.sfad_base + data.sfad_result);
not_found:
	err_index_not_found((DeeObject *)self, needles);
err:
	return NULL;
}

struct string_casefindany_data {
	String *scfad_self;   /* [1..1][const] The string being searched. */
	size_t  scfad_base;   /* [const] Character offset into `scfad_self' of search start. */
	size_t  scfad_size;   /* [const] Sub-string length to search in `scfad_self', starting at `scfad_base'. */
	size_t  scfad_result; /* [<= scfad_size] Offset from `scfad_base' of best match. */
	size_t  scfad_reslen; /* # of matched characters at `scfad_result'. */
};

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
string_casefindany_cb(void *arg, DeeObject *elem) {
	struct string_casefindany_data *data = (struct string_casefindany_data *)arg;
	union dcharptr ptr, lhs, rhs;
	size_t search_size, match_length;
	if (DeeObject_AssertTypeExact(elem, &DeeString_Type))
		goto err;
	SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON(DeeString_WIDTH(data->scfad_self),
	                                        DeeString_WIDTH(elem))) {

	CASE_WIDTH_1BYTE:
		lhs.cp8 = DeeString_As1Byte((DeeObject *)data->scfad_self);
		lhs.cp8 += data->scfad_base;
		rhs.cp8 = DeeString_As1Byte((DeeObject *)elem);
		search_size = data->scfad_result + WSTR_LENGTH(rhs.cp8) - 1;
		if (search_size > data->scfad_size)
			search_size = data->scfad_size;
		ptr.cp8 = memcasememb(lhs.cp8, search_size,
		                      rhs.cp8, WSTR_LENGTH(rhs.cp8),
		                      &match_length);
		if (ptr.cp8 != NULL) {
			size_t hit = (size_t)(ptr.cp8 - lhs.cp8);
			ASSERT(hit < data->scfad_result);
			data->scfad_result = hit;
			data->scfad_reslen = match_length;
			if (hit == 0)
				return -2; /* Found hit at offset=0 -> can stop enumerating needles. */
		}
		break;

	CASE_WIDTH_2BYTE:
		lhs.cp16 = DeeString_As2Byte((DeeObject *)data->scfad_self);
		if unlikely(!lhs.cp16)
			goto err;
		lhs.cp16 += data->scfad_base;
		rhs.cp16 = DeeString_As2Byte((DeeObject *)elem);
		if unlikely(!rhs.cp16)
			goto err;
		search_size = data->scfad_result + WSTR_LENGTH(rhs.cp16) - 1;
		if (search_size > data->scfad_size)
			search_size = data->scfad_size;
		ptr.cp16 = memcasememw(lhs.cp16, search_size,
		                       rhs.cp16, WSTR_LENGTH(rhs.cp16),
		                       &match_length);
		if (ptr.cp16 != NULL) {
			size_t hit = (size_t)(ptr.cp16 - lhs.cp16);
			ASSERT(hit < data->scfad_result);
			data->scfad_result = hit;
			data->scfad_reslen = match_length;
			if (hit == 0)
				return -2; /* Found hit at offset=0 -> can stop enumerating needles. */
		}
		break;

	CASE_WIDTH_4BYTE:
		lhs.cp32 = DeeString_As4Byte((DeeObject *)data->scfad_self);
		if unlikely(!lhs.cp32)
			goto err;
		lhs.cp32 += data->scfad_base;
		rhs.cp32 = DeeString_As4Byte((DeeObject *)elem);
		if unlikely(!rhs.cp32)
			goto err;
		search_size = data->scfad_result + WSTR_LENGTH(rhs.cp32) - 1;
		if (search_size > data->scfad_size)
			search_size = data->scfad_size;
		ptr.cp32 = memcasememl(lhs.cp32, search_size,
		                       rhs.cp32, WSTR_LENGTH(rhs.cp32),
		                       &match_length);
		if (ptr.cp32 != NULL) {
			size_t hit = (size_t)(ptr.cp32 - lhs.cp32);
			ASSERT(hit < data->scfad_result);
			data->scfad_result = hit;
			data->scfad_reslen = match_length;
			if (hit == 0)
				return -2; /* Found hit at offset=0 -> can stop enumerating needles. */
		}
		break;
	}
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
string_casefindany(String *self, size_t argc,
                   DeeObject *const *argv, DeeObject *kw) {
	size_t start = 0, end = (size_t)-1;
	DeeObject *needles;
	struct string_casefindany_data data;
	if (DeeArg_UnpackKw(argc, argv, kw, find_kwlist,
	                    "o|" UNPdSIZ UNPdSIZ ":casefindany",
	                    &needles, &start, &end))
		goto err;
	data.scfad_self = self;
	data.scfad_size = DeeString_WLEN(self);
	CLAMP_SUBSTR(&start, &end, &data.scfad_size, not_found);
	data.scfad_base   = start;
	data.scfad_result = data.scfad_size;
	if unlikely(DeeObject_Foreach(needles, &string_casefindany_cb, &data) == -1)
		goto err;
	if (data.scfad_result < data.scfad_size) {
		size_t index = data.scfad_base + data.scfad_result;
		return DeeTuple_NewII(index, index + data.scfad_reslen);
	}
not_found:
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
string_caseindexany(String *self, size_t argc,
                    DeeObject *const *argv, DeeObject *kw) {
	size_t start = 0, end = (size_t)-1;
	DeeObject *needles;
	struct string_casefindany_data data;
	if (DeeArg_UnpackKw(argc, argv, kw, find_kwlist,
	                    "o|" UNPdSIZ UNPdSIZ ":caseindexany",
	                    &needles, &start, &end))
		goto err;
	data.scfad_self = self;
	data.scfad_size = DeeString_WLEN(self);
	CLAMP_SUBSTR(&start, &end, &data.scfad_size, not_found);
	data.scfad_base   = start;
	data.scfad_result = data.scfad_size;
	if unlikely(DeeObject_Foreach(needles, &string_casefindany_cb, &data) == -1)
		goto err;
	if (data.scfad_result < data.scfad_size) {
		size_t index = data.scfad_base + data.scfad_result;
		return DeeTuple_NewII(index, index + data.scfad_reslen);
	}
not_found:
	err_index_not_found((DeeObject *)self, needles);
err:
	return NULL;
}


struct string_rfindany_data {
	String *srfad_self;   /* [1..1][const] The string being searched. */
	size_t  srfad_base;   /* [const] Character offset into `scfad_self' of search start. */
	size_t  srfad_size;   /* [const] Sub-string length to search in `scfad_self', starting at `srfad_base'. */
};

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
string_rfindany_cb(void *arg, DeeObject *elem) {
	struct string_rfindany_data *data = (struct string_rfindany_data *)arg;
	union dcharptr ptr, lhs, rhs;
	if (DeeObject_AssertTypeExact(elem, &DeeString_Type))
		goto err;
	SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON(DeeString_WIDTH(data->srfad_self),
	                                        DeeString_WIDTH(elem))) {

	CASE_WIDTH_1BYTE:
		lhs.cp8 = DeeString_As1Byte((DeeObject *)data->srfad_self);
		lhs.cp8 += data->srfad_base;
		rhs.cp8 = DeeString_As1Byte((DeeObject *)elem);
		ptr.cp8 = memrmemb(lhs.cp8, data->srfad_size,
		                   rhs.cp8, WSTR_LENGTH(rhs.cp8));
		if (ptr.cp8 != NULL) {
			size_t hit = (size_t)(ptr.cp8 - lhs.cp8) + 1;
			ASSERT(hit <= data->srfad_size);
			data->srfad_base += hit;
			data->srfad_size -= hit;
			if (data->srfad_size == 0)
				return -2; /* Found hit at the very end -> can stop enumerating needles. */
		}
		break;

	CASE_WIDTH_2BYTE:
		lhs.cp16 = DeeString_As2Byte((DeeObject *)data->srfad_self);
		if unlikely(!lhs.cp16)
			goto err;
		lhs.cp16 += data->srfad_base;
		rhs.cp16 = DeeString_As2Byte((DeeObject *)elem);
		if unlikely(!rhs.cp16)
			goto err;
		ptr.cp16 = memrmemw(lhs.cp16, data->srfad_size,
		                    rhs.cp16, WSTR_LENGTH(rhs.cp16));
		if (ptr.cp16 != NULL) {
			size_t hit = (size_t)(ptr.cp16 - lhs.cp16) + 1;
			ASSERT(hit <= data->srfad_size);
			data->srfad_base += hit;
			data->srfad_size -= hit;
			if (data->srfad_size == 0)
				return -2; /* Found hit at the very end -> can stop enumerating needles. */
		}
		break;

	CASE_WIDTH_4BYTE:
		lhs.cp32 = DeeString_As4Byte((DeeObject *)data->srfad_self);
		if unlikely(!lhs.cp32)
			goto err;
		lhs.cp32 += data->srfad_base;
		rhs.cp32 = DeeString_As4Byte((DeeObject *)elem);
		if unlikely(!rhs.cp32)
			goto err;
		ptr.cp32 = memrmeml(lhs.cp32, data->srfad_size,
		                    rhs.cp32, WSTR_LENGTH(rhs.cp32));
		if (ptr.cp32 != NULL) {
			size_t hit = (size_t)(ptr.cp32 - lhs.cp32) + 1;
			ASSERT(hit <= data->srfad_size);
			data->srfad_base += hit;
			data->srfad_size -= hit;
			if (data->srfad_size == 0)
				return -2; /* Found hit at the very end -> can stop enumerating needles. */
		}
	}
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
string_rfindany(String *self, size_t argc,
                DeeObject *const *argv, DeeObject *kw) {
	size_t start = 0, end = (size_t)-1;
	DeeObject *needles;
	struct string_rfindany_data data;
	if (DeeArg_UnpackKw(argc, argv, kw, find_kwlist,
	                    "o|" UNPdSIZ UNPdSIZ ":rfindany",
	                    &needles, &start, &end))
		goto err;
	data.srfad_self = self;
	data.srfad_size = DeeString_WLEN(self);
	CLAMP_SUBSTR(&start, &end, &data.srfad_size, not_found);
	data.srfad_base = start;
	if unlikely(DeeObject_Foreach(needles, &string_rfindany_cb, &data) == -1)
		goto err;
	if (data.srfad_base > start)
		return DeeInt_NewSize(data.srfad_base - 1);
not_found:
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
string_rindexany(String *self, size_t argc,
                 DeeObject *const *argv, DeeObject *kw) {
	size_t start = 0, end = (size_t)-1;
	DeeObject *needles;
	struct string_rfindany_data data;
	if (DeeArg_UnpackKw(argc, argv, kw, find_kwlist,
	                    "o|" UNPdSIZ UNPdSIZ ":rindexany",
	                    &needles, &start, &end))
		goto err;
	data.srfad_self = self;
	data.srfad_size = DeeString_WLEN(self);
	CLAMP_SUBSTR(&start, &end, &data.srfad_size, not_found);
	data.srfad_base = start;
	if unlikely(DeeObject_Foreach(needles, &string_rfindany_cb, &data) == -1)
		goto err;
	if (data.srfad_base > start)
		return DeeInt_NewSize(data.srfad_base - 1);
not_found:
	err_index_not_found((DeeObject *)self, needles);
err:
	return NULL;
}

struct string_caserfindany_data {
	String *scrfad_self;   /* [1..1][const] The string being searched. */
	size_t  scrfad_base;   /* [const] Character offset into `scfad_self' of search start. */
	size_t  scrfad_size;   /* [const] Sub-string length to search in `scfad_self', starting at `scrfad_base'. */
	size_t  scrfad_reslen; /* # of matched characters at last match. */
};

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
string_caserfindany_cb(void *arg, DeeObject *elem) {
	struct string_caserfindany_data *data = (struct string_caserfindany_data *)arg;
	union dcharptr ptr, lhs, rhs;
	size_t match_length;
	if (DeeObject_AssertTypeExact(elem, &DeeString_Type))
		goto err;
	SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON(DeeString_WIDTH(data->scrfad_self),
	                                        DeeString_WIDTH(elem))) {

	CASE_WIDTH_1BYTE:
		lhs.cp8 = DeeString_As1Byte((DeeObject *)data->scrfad_self);
		lhs.cp8 += data->scrfad_base;
		rhs.cp8 = DeeString_As1Byte((DeeObject *)elem);
		ptr.cp8 = memcasermemb(lhs.cp8, data->scrfad_size,
		                       rhs.cp8, WSTR_LENGTH(rhs.cp8),
		                       &match_length);
		if (ptr.cp8 != NULL) {
			size_t hit = (size_t)(ptr.cp8 - lhs.cp8) + 1;
			ASSERT(hit <= data->scrfad_size);
			data->scrfad_base += hit;
			data->scrfad_size -= hit;
			data->scrfad_reslen = match_length;
			if (data->scrfad_size == 0)
				return -2; /* Found hit at the very end -> can stop enumerating needles. */
		}
		break;

	CASE_WIDTH_2BYTE:
		lhs.cp16 = DeeString_As2Byte((DeeObject *)data->scrfad_self);
		if unlikely(!lhs.cp16)
			goto err;
		lhs.cp16 += data->scrfad_base;
		rhs.cp16 = DeeString_As2Byte((DeeObject *)elem);
		if unlikely(!rhs.cp16)
			goto err;
		ptr.cp16 = memcasermemw(lhs.cp16, data->scrfad_size,
		                        rhs.cp16, WSTR_LENGTH(rhs.cp16),
		                        &match_length);
		if (ptr.cp16 != NULL) {
			size_t hit = (size_t)(ptr.cp16 - lhs.cp16) + 1;
			ASSERT(hit <= data->scrfad_size);
			data->scrfad_base += hit;
			data->scrfad_size -= hit;
			data->scrfad_reslen = match_length;
			if (data->scrfad_size == 0)
				return -2; /* Found hit at the very end -> can stop enumerating needles. */
		}
		break;

	CASE_WIDTH_4BYTE:
		lhs.cp32 = DeeString_As4Byte((DeeObject *)data->scrfad_self);
		if unlikely(!lhs.cp32)
			goto err;
		lhs.cp32 += data->scrfad_base;
		rhs.cp32 = DeeString_As4Byte((DeeObject *)elem);
		if unlikely(!rhs.cp32)
			goto err;
		ptr.cp32 = memcasermeml(lhs.cp32, data->scrfad_size,
		                        rhs.cp32, WSTR_LENGTH(rhs.cp32),
		                        &match_length);
		if (ptr.cp32 != NULL) {
			size_t hit = (size_t)(ptr.cp32 - lhs.cp32) + 1;
			ASSERT(hit <= data->scrfad_size);
			data->scrfad_base += hit;
			data->scrfad_size -= hit;
			data->scrfad_reslen = match_length;
			if (data->scrfad_size == 0)
				return -2; /* Found hit at the very end -> can stop enumerating needles. */
		}
	}
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
string_caserfindany(String *self, size_t argc,
                    DeeObject *const *argv, DeeObject *kw) {
	size_t start = 0, end = (size_t)-1;
	DeeObject *needles;
	struct string_caserfindany_data data;
	if (DeeArg_UnpackKw(argc, argv, kw, find_kwlist,
	                    "o|" UNPdSIZ UNPdSIZ ":caserfindany",
	                    &needles, &start, &end))
		goto err;
	data.scrfad_self = self;
	data.scrfad_size = DeeString_WLEN(self);
	CLAMP_SUBSTR(&start, &end, &data.scrfad_size, not_found);
	data.scrfad_base = start;
	if unlikely(DeeObject_Foreach(needles, &string_caserfindany_cb, &data) == -1)
		goto err;
	if (data.scrfad_base > start) {
		size_t index = data.scrfad_base - 1;
		return DeeTuple_NewII(index, index + data.scrfad_reslen);
	}
not_found:
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
string_caserindexany(String *self, size_t argc,
                     DeeObject *const *argv, DeeObject *kw) {
	size_t start = 0, end = (size_t)-1;
	DeeObject *needles;
	struct string_caserfindany_data data;
	if (DeeArg_UnpackKw(argc, argv, kw, find_kwlist,
	                    "o|" UNPdSIZ UNPdSIZ ":caserindexany",
	                    &needles, &start, &end))
		goto err;
	data.scrfad_self = self;
	data.scrfad_size = DeeString_WLEN(self);
	CLAMP_SUBSTR(&start, &end, &data.scrfad_size, not_found);
	data.scrfad_base = start;
	if unlikely(DeeObject_Foreach(needles, &string_caserfindany_cb, &data) == -1)
		goto err;
	if (data.scrfad_base > start) {
		size_t index = data.scrfad_base - 1;
		return DeeTuple_NewII(index, index + data.scrfad_reslen);
	}
not_found:
	err_index_not_found((DeeObject *)self, needles);
err:
	return NULL;
}


INTDEF WUNUSED DREF DeeObject *DCALL
DeeString_FindAll(String *self, String *other,
                  size_t start, size_t end);
INTDEF WUNUSED DREF DeeObject *DCALL
DeeString_CaseFindAll(String *self, String *other,
                      size_t start, size_t end);

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
string_findall(String *self, size_t argc,
               DeeObject *const *argv, DeeObject *kw) {
	String *other;
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, find_kwlist,
	                    "o|" UNPdSIZ UNPdSIZ ":findall",
	                    &other, &start, &end))
		goto err;
	if (DeeObject_AssertTypeExact(other, &DeeString_Type))
		goto err;
	return DeeString_FindAll(self, other, start, end);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
string_casefindall(String *self, size_t argc,
                   DeeObject *const *argv, DeeObject *kw) {
	String *other;
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, find_kwlist,
	                    "o|" UNPdSIZ UNPdSIZ ":casefindall",
	                    &other, &start, &end))
		goto err;
	if (DeeObject_AssertTypeExact(other, &DeeString_Type))
		goto err;
	return DeeString_CaseFindAll(self, other, start, end);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
string_casefind(String *self, size_t argc,
                DeeObject *const *argv, DeeObject *kw) {
	String *other;
	size_t start = 0, end = (size_t)-1;
	size_t mylen, result, match_length;
	union dcharptr ptr, lhs, rhs;
	if (DeeArg_UnpackKw(argc, argv, kw, find_kwlist,
	                    "o|" UNPdSIZ UNPdSIZ ":casefind",
	                    &other, &start, &end))
		goto err;
	if (DeeObject_AssertTypeExact(other, &DeeString_Type))
		goto err;
	SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON(DeeString_WIDTH(self),
	                                        DeeString_WIDTH(other))) {

	CASE_WIDTH_1BYTE:
		lhs.cp8 = DeeString_As1Byte((DeeObject *)self);
		rhs.cp8 = DeeString_As1Byte((DeeObject *)other);
		mylen   = WSTR_LENGTH(lhs.ptr);
		CLAMP_SUBSTR(&start, &end, &mylen, not_found);
		ptr.cp8 = memcasememb(lhs.cp8 + start, mylen,
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
		CLAMP_SUBSTR(&start, &end, &mylen, not_found);
		ptr.cp16 = memcasememw(lhs.cp16 + start, mylen,
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
		CLAMP_SUBSTR(&start, &end, &mylen, not_found);
		ptr.cp32 = memcasememl(lhs.cp32 + start, mylen,
		                       rhs.cp32, WSTR_LENGTH(rhs.cp32),
		                       &match_length);
		if (!ptr.cp32)
			goto not_found;
		result = ptr.cp32 - lhs.cp32;
		break;
	}
	return DeeTuple_NewII(result, result + match_length);
not_found:
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
string_caserfind(String *self, size_t argc,
                 DeeObject *const *argv, DeeObject *kw) {
	String *other;
	size_t start = 0, end = (size_t)-1;
	size_t mylen, result, match_length;
	union dcharptr ptr, lhs, rhs;
	if (DeeArg_UnpackKw(argc, argv, kw, find_kwlist,
	                    "o|" UNPdSIZ UNPdSIZ ":caserfind",
	                    &other, &start, &end))
		goto err;
	if (DeeObject_AssertTypeExact(other, &DeeString_Type))
		goto err;
	SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON(DeeString_WIDTH(self),
	                                        DeeString_WIDTH(other))) {

	CASE_WIDTH_1BYTE:
		lhs.cp8 = DeeString_As1Byte((DeeObject *)self);
		rhs.cp8 = DeeString_As1Byte((DeeObject *)other);
		mylen   = WSTR_LENGTH(lhs.ptr);
		CLAMP_SUBSTR(&start, &end, &mylen, not_found);
		ptr.cp8 = memcasermemb(lhs.cp8 + start, mylen,
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
		CLAMP_SUBSTR(&start, &end, &mylen, not_found);
		ptr.cp16 = memcasermemw(lhs.cp16 + start, mylen,
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
		CLAMP_SUBSTR(&start, &end, &mylen, not_found);
		ptr.cp32 = memcasermeml(lhs.cp32 + start, mylen,
		                        rhs.cp32, WSTR_LENGTH(rhs.cp32),
		                        &match_length);
		if (!ptr.cp32)
			goto not_found;
		result = ptr.cp32 - lhs.cp32;
		break;
	}
	return DeeTuple_NewII(result, result + match_length);
not_found:
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
string_caseindex(String *self, size_t argc,
                 DeeObject *const *argv, DeeObject *kw) {
	String *other;
	size_t start = 0, end = (size_t)-1;
	size_t mylen, result, match_length;
	union dcharptr ptr, lhs, rhs;
	if (DeeArg_UnpackKw(argc, argv, kw, find_kwlist,
	                    "o|" UNPdSIZ UNPdSIZ ":caseindex",
	                    &other, &start, &end))
		goto err;
	if (DeeObject_AssertTypeExact(other, &DeeString_Type))
		goto err;
	SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON(DeeString_WIDTH(self),
	                                        DeeString_WIDTH(other))) {

	CASE_WIDTH_1BYTE:
		lhs.cp8 = DeeString_As1Byte((DeeObject *)self);
		rhs.cp8 = DeeString_As1Byte((DeeObject *)other);
		mylen   = WSTR_LENGTH(lhs.ptr);
		CLAMP_SUBSTR(&start, &end, &mylen, not_found);
		ptr.cp8 = memcasememb(lhs.cp8 + start, mylen,
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
		CLAMP_SUBSTR(&start, &end, &mylen, not_found);
		ptr.cp16 = memcasememw(lhs.cp16 + start, mylen,
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
		CLAMP_SUBSTR(&start, &end, &mylen, not_found);
		ptr.cp32 = memcasememl(lhs.cp32 + start, mylen,
		                       rhs.cp32, WSTR_LENGTH(rhs.cp32),
		                       &match_length);
		if unlikely(!ptr.cp32)
			goto not_found;
		result = ptr.cp32 - lhs.cp32;
		break;
	}
	return DeeTuple_NewII(result, result + match_length);
not_found:
	err_index_not_found((DeeObject *)self,
	                    (DeeObject *)other);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
string_caserindex(String *self, size_t argc,
                  DeeObject *const *argv, DeeObject *kw) {
	String *other;
	size_t start = 0, end = (size_t)-1;
	size_t mylen, result, match_length;
	union dcharptr ptr, lhs, rhs;
	if (DeeArg_UnpackKw(argc, argv, kw, find_kwlist,
	                    "o|" UNPdSIZ UNPdSIZ ":caserindex",
	                    &other, &start, &end))
		goto err;
	if (DeeObject_AssertTypeExact(other, &DeeString_Type))
		goto err;
	SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON(DeeString_WIDTH(self),
	                                        DeeString_WIDTH(other))) {

	CASE_WIDTH_1BYTE:
		lhs.cp8 = DeeString_As1Byte((DeeObject *)self);
		rhs.cp8 = DeeString_As1Byte((DeeObject *)other);
		mylen   = WSTR_LENGTH(lhs.ptr);
		CLAMP_SUBSTR(&start, &end, &mylen, not_found);
		ptr.cp8 = memcasermemb(lhs.cp8 + start, mylen,
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
		CLAMP_SUBSTR(&start, &end, &mylen, not_found);
		ptr.cp16 = memcasermemw(lhs.cp16 + start, mylen,
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
		CLAMP_SUBSTR(&start, &end, &mylen, not_found);
		ptr.cp32 = memcasermeml(lhs.cp32 + start, mylen,
		                        rhs.cp32, WSTR_LENGTH(rhs.cp32),
		                        &match_length);
		if unlikely(!ptr.cp32)
			goto not_found;
		result = ptr.cp32 - lhs.cp32;
		break;
	}
	return DeeTuple_NewII(result, result + match_length);
not_found:
	err_index_not_found((DeeObject *)self,
	                    (DeeObject *)other);
err:
	return NULL;
}

INTERN WUNUSED NONNULL((1)) DREF String *DCALL
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
			                                               STRING_MUL_SIZEOF_WIDTH(start, width),
			                                               end - (size_t)start,
			                                               width);
		}
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF String *DCALL
string_substr(String *self, size_t argc,
              DeeObject *const *argv, DeeObject *kw) {
	size_t start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, substr_kwlist,
	                    "|" UNPdSIZ UNPdSIZ ":substr",
	                    &start, &end))
		goto err;
	return string_getsubstr(self, start, end);
err:
	return NULL;
}

#undef CONFIG_STRING_STARTSWITH_ENDSWITH_SPECIALCASE_OPTIMIZATIONS
#define CONFIG_STRING_STARTSWITH_ENDSWITH_SPECIALCASE_OPTIMIZATIONS

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
string_startswith(String *self, size_t argc,
                  DeeObject *const *argv, DeeObject *kw) {
	String *other;
	size_t begin = 0, end = (size_t)-1;
	union dcharptr my_str, ot_str;
	size_t my_len, ot_len;
	if (DeeArg_UnpackKw(argc, argv, kw, find_kwlist,
	                    "o|" UNPdSIZ UNPdSIZ ":startswith",
	                    &other, &begin, &end))
		goto err;
	if (DeeObject_AssertTypeExact(other, &DeeString_Type))
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

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
string_endswith(String *self, size_t argc,
                DeeObject *const *argv, DeeObject *kw) {
	String *other;
	size_t begin = 0, end = (size_t)-1;
	union dcharptr my_str, ot_str;
	size_t my_len, ot_len;
	if (DeeArg_UnpackKw(argc, argv, kw, find_kwlist,
	                    "o|" UNPdSIZ UNPdSIZ ":endswith",
	                    &other, &begin, &end))
		goto err;
	if (DeeObject_AssertTypeExact(other, &DeeString_Type))
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

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
string_casestartswith(String *self, size_t argc,
                      DeeObject *const *argv, DeeObject *kw) {
	String *other;
	size_t begin = 0, end = (size_t)-1;
	union dcharptr my_str, ot_str;
	size_t my_len, ot_len;
	if (DeeArg_UnpackKw(argc, argv, kw, find_kwlist,
	                    "o|" UNPdSIZ UNPdSIZ ":casestartswith",
	                    &other, &begin, &end))
		goto err;
	if (DeeObject_AssertTypeExact(other, &DeeString_Type))
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

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
string_caseendswith(String *self, size_t argc,
                    DeeObject *const *argv, DeeObject *kw) {
	String *other;
	size_t begin = 0, end = (size_t)-1;
	union dcharptr my_str, ot_str;
	size_t my_len, ot_len;
	if (DeeArg_UnpackKw(argc, argv, kw, find_kwlist,
	                    "o|" UNPdSIZ UNPdSIZ ":caseendswith",
	                    &other, &begin, &end))
		goto err;
	if (DeeObject_AssertTypeExact(other, &DeeString_Type))
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

/* INTERN, because also used in `/src/deemon/compiler/optimize/opt_operators.c' */
/* INTERN, because also used in `/src/deemon/objects/unicode/bytes_functions.c.inl' */
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
string_decode(DeeObject *self, size_t argc,
              DeeObject *const *argv, DeeObject *kw) {
	DeeObject *codec;
	char *errors            = NULL;
	unsigned int error_mode = STRING_ERROR_FSTRICT;
	if (DeeArg_UnpackKw(argc, argv, kw, decode_encode_kwlist, "o|s:decode", &codec, &errors))
		goto err;
	if (DeeObject_AssertTypeExact(codec, &DeeString_Type))
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

/* INTERN, because also used in `/src/deemon/compiler/optimize/opt_operators.c' */
/* INTERN, because also used in `/src/deemon/objects/unicode/bytes_functions.c.inl' */
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
string_encode(DeeObject *self, size_t argc,
              DeeObject *const *argv, DeeObject *kw) {
	DeeObject *codec;
	char *errors            = NULL;
	unsigned int error_mode = STRING_ERROR_FSTRICT;
	if (DeeArg_UnpackKw(argc, argv, kw, decode_encode_kwlist, "o|s:encode", &codec, &errors))
		goto err;
	if (DeeObject_AssertTypeExact(codec, &DeeString_Type))
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


INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeString_Segments(String *__restrict self, size_t substring_length);

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
string_segments(String *self, size_t argc, DeeObject *const *argv) {
	size_t substring_length;
	if (DeeArg_Unpack(argc, argv, UNPuSIZ ":segments", &substring_length))
		goto err;
	if unlikely(!substring_length) {
		err_invalid_segment_size(substring_length);
		goto err;
	}
	return DeeString_Segments(self, substring_length);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
string_distribute(String *self, size_t argc, DeeObject *const *argv) {
	size_t substring_count;
	size_t substring_length;
	if (DeeArg_Unpack(argc, argv, UNPuSIZ ":distribute", &substring_count))
		goto err;
	if unlikely(!substring_count) {
		err_invalid_distribution_count(substring_count);
		goto err;
	}
	substring_length = DeeString_WLEN(self);
	substring_length += substring_count - 1;
	substring_length /= substring_count;
	if unlikely(!substring_length)
		return_empty_seq;
	return DeeString_Segments(self, substring_length);
err:
	return NULL;
}

#ifndef DEFINED_err_empty_filler
#define DEFINED_err_empty_filler
PRIVATE ATTR_COLD int DCALL err_empty_filler(void) {
	return DeeError_Throwf(&DeeError_ValueError,
	                       "Empty filler");
}
#endif /* !DEFINED_err_empty_filler */

PRIVATE WUNUSED NONNULL((1)) DREF String *DCALL
string_center(String *self, size_t argc, DeeObject *const *argv) {
	size_t result_length, my_len, fl_len;
	size_t fill_front, fill_back;
	String *filler_ob = NULL;
	DREF String *result;
	union dcharptr dst, buf, my_str, fl_str;
	if (DeeArg_Unpack(argc, argv, UNPuSIZ "|o:center", &result_length, &filler_ob))
		goto err;
	my_len = DeeString_WLEN(self);
	if (result_length <= my_len)
		return_reference_(self);
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
			result = (DREF String *)DeeString_NewBuffer(result_length);
			if unlikely(!result)
				goto err;
			dst.cp8 = DeeString_As1Byte((DeeObject *)result);
			dst.cp8 = mempfilb(dst.cp8, fill_front, fl_str.cp8, fl_len);
			dst.cp8 = mempcpyb(dst.cp8, my_str.cp8, my_len);
			mempfilb(dst.cp8, fill_back, fl_str.cp8, fl_len);
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
			dst.cp16 = mempfilw(dst.cp16, fill_front, fl_str.cp16, fl_len);
			dst.cp16 = mempcpyw(dst.cp16, my_str.cp16, my_len);
			mempfilw(dst.cp16, fill_back, fl_str.cp16, fl_len);
			result = (DREF String *)DeeString_Pack2ByteBuffer(buf.cp16);
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
			dst.cp32 = mempfill(dst.cp32, fill_front, fl_str.cp32, fl_len);
			dst.cp32 = mempcpyl(dst.cp32, my_str.cp32, my_len);
			mempfill(dst.cp32, fill_back, fl_str.cp32, fl_len);
			result = (DREF String *)DeeString_Pack4ByteBuffer(buf.cp32);
			break;
		}
	} else {
		SWITCH_SIZEOF_WIDTH(DeeString_WIDTH(self)) {

		CASE_WIDTH_1BYTE:
			my_str.cp8 = DeeString_Get1Byte((DeeObject *)self);
			result     = (DREF String *)DeeString_NewBuffer(result_length);
			if unlikely(!result)
				goto err;
			dst.cp8 = DeeString_As1Byte((DeeObject *)result);
			dst.cp8 = mempsetb(dst.cp8, UNICODE_SPACE, fill_front);
			dst.cp8 = mempcpyb(dst.cp8, my_str.cp8, my_len);
			memsetb(dst.cp8, UNICODE_SPACE, fill_back);
			break;

		CASE_WIDTH_2BYTE:
			my_str.cp16 = DeeString_Get2Byte((DeeObject *)self);
			dst.cp16 = buf.cp16 = DeeString_New2ByteBuffer(result_length);
			if unlikely(!buf.cp16)
				goto err;
			dst.cp16 = mempsetw(dst.cp16, UNICODE_SPACE, fill_front);
			dst.cp16 = mempcpyw(dst.cp16, my_str.cp16, my_len);
			memsetw(dst.cp16, UNICODE_SPACE, fill_back);
			result = (DREF String *)DeeString_Pack2ByteBuffer(buf.cp16);
			break;

		CASE_WIDTH_4BYTE:
			my_str.cp32 = DeeString_Get4Byte((DeeObject *)self);
			dst.cp32 = buf.cp32 = DeeString_New4ByteBuffer(result_length);
			if unlikely(!buf.cp32)
				goto err;
			dst.cp32 = mempsetl(dst.cp32, UNICODE_SPACE, fill_front);
			dst.cp32 = mempcpyl(dst.cp32, my_str.cp32, my_len);
			memsetl(dst.cp32, UNICODE_SPACE, fill_back);
			result = (DREF String *)DeeString_Pack4ByteBuffer(buf.cp32);
			break;
		}
	}
	return result;
empty_filler:
	err_empty_filler();
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF String *DCALL
string_ljust(String *self, size_t argc, DeeObject *const *argv) {
	size_t result_length, my_len, fl_len, fill_back;
	String *filler_ob = NULL;
	DREF String *result;
	union dcharptr dst, buf, my_str, fl_str;
	if (DeeArg_Unpack(argc, argv, UNPuSIZ "|o:ljust", &result_length, &filler_ob))
		goto err;
	my_len = DeeString_WLEN(self);
	if (result_length <= my_len)
		return_reference_(self);
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
			result = (DREF String *)DeeString_NewBuffer(result_length);
			if unlikely(!result)
				goto err;
			dst.cp8 = DeeString_As1Byte((DeeObject *)result);
			dst.cp8 = mempcpyb(dst.cp8, my_str.cp8, my_len);
			mempfilb(dst.cp8, fill_back, fl_str.cp8, fl_len);
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
			dst.cp16 = mempcpyw(dst.cp16, my_str.cp16, my_len);
			mempfilw(dst.cp16, fill_back, fl_str.cp16, fl_len);
			result = (DREF String *)DeeString_Pack2ByteBuffer(buf.cp16);
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
			dst.cp32 = mempcpyl(dst.cp32, my_str.cp32, my_len);
			mempfill(dst.cp32, fill_back, fl_str.cp32, fl_len);
			result = (DREF String *)DeeString_Pack4ByteBuffer(buf.cp32);
			break;
		}
	} else {
		SWITCH_SIZEOF_WIDTH(DeeString_WIDTH(self)) {

		CASE_WIDTH_1BYTE:
			my_str.cp8 = DeeString_Get1Byte((DeeObject *)self);
			result     = (DREF String *)DeeString_NewBuffer(result_length);
			if unlikely(!result)
				goto err;
			dst.cp8 = DeeString_As1Byte((DeeObject *)result);
			dst.cp8 = mempcpyb(dst.cp8, my_str.cp8, my_len);
			memsetb(dst.cp8, UNICODE_SPACE, fill_back);
			break;

		CASE_WIDTH_2BYTE:
			my_str.cp16 = DeeString_Get2Byte((DeeObject *)self);
			dst.cp16 = buf.cp16 = DeeString_New2ByteBuffer(result_length);
			if unlikely(!buf.cp16)
				goto err;
			dst.cp16 = mempcpyw(dst.cp16, my_str.cp16, my_len);
			memsetw(dst.cp16, UNICODE_SPACE, fill_back);
			result = (DREF String *)DeeString_Pack2ByteBuffer(buf.cp16);
			break;

		CASE_WIDTH_4BYTE:
			my_str.cp32 = DeeString_Get4Byte((DeeObject *)self);
			dst.cp32 = buf.cp32 = DeeString_New4ByteBuffer(result_length);
			if unlikely(!buf.cp32)
				goto err;
			dst.cp32 = mempcpyl(dst.cp32, my_str.cp32, my_len);
			memsetl(dst.cp32, UNICODE_SPACE, fill_back);
			result = (DREF String *)DeeString_Pack4ByteBuffer(buf.cp32);
			break;
		}
	}
	return result;
empty_filler:
	err_empty_filler();
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF String *DCALL
string_rjust(String *self, size_t argc, DeeObject *const *argv) {
	size_t result_length, my_len, fl_len, fill_front;
	String *filler_ob = NULL;
	DREF String *result;
	union dcharptr dst, buf, my_str, fl_str;
	if (DeeArg_Unpack(argc, argv, UNPuSIZ "|o:rjust", &result_length, &filler_ob))
		goto err;
	my_len = DeeString_WLEN(self);
	if (result_length <= my_len)
		return_reference_(self);
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
			result = (DREF String *)DeeString_NewBuffer(result_length);
			if unlikely(!result)
				goto err;
			dst.cp8 = DeeString_As1Byte((DeeObject *)result);
			dst.cp8 = mempfilb(dst.cp8, fill_front, fl_str.cp8, fl_len);
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
			dst.cp16 = mempfilw(dst.cp16, fill_front, fl_str.cp16, fl_len);
			memcpyw(dst.cp16, my_str.cp16, my_len);
			result = (DREF String *)DeeString_Pack2ByteBuffer(buf.cp16);
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
			dst.cp32 = mempfill(dst.cp32, fill_front, fl_str.cp32, fl_len);
			memcpyl(dst.cp32, my_str.cp32, my_len);
			result = (DREF String *)DeeString_Pack4ByteBuffer(buf.cp32);
			break;
		}
	} else {
		SWITCH_SIZEOF_WIDTH(DeeString_WIDTH(self)) {

		CASE_WIDTH_1BYTE:
			my_str.cp8 = DeeString_Get1Byte((DeeObject *)self);
			result     = (DREF String *)DeeString_NewBuffer(result_length);
			if unlikely(!result)
				goto err;
			dst.cp8 = DeeString_As1Byte((DeeObject *)result);
			dst.cp8 = mempsetb(dst.cp8, UNICODE_SPACE, fill_front);
			memcpyb(dst.cp8, my_str.cp8, my_len);
			break;

		CASE_WIDTH_2BYTE:
			my_str.cp16 = DeeString_Get2Byte((DeeObject *)self);
			dst.cp16 = buf.cp16 = DeeString_New2ByteBuffer(result_length);
			if unlikely(!buf.cp16)
				goto err;
			dst.cp16 = mempsetw(dst.cp16, UNICODE_SPACE, fill_front);
			memcpyw(dst.cp16, my_str.cp16, my_len);
			result = (DREF String *)DeeString_Pack2ByteBuffer(buf.cp16);
			break;

		CASE_WIDTH_4BYTE:
			my_str.cp32 = DeeString_Get4Byte((DeeObject *)self);
			dst.cp32 = buf.cp32 = DeeString_New4ByteBuffer(result_length);
			if unlikely(!buf.cp32)
				goto err;
			dst.cp32 = mempsetl(dst.cp32, UNICODE_SPACE, fill_front);
			memcpyl(dst.cp32, my_str.cp32, my_len);
			result = (DREF String *)DeeString_Pack4ByteBuffer(buf.cp32);
			break;
		}
	}
	return result;
empty_filler:
	err_empty_filler();
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
string_count(String *self, size_t argc,
             DeeObject *const *argv, DeeObject *kw) {
	String *other;
	size_t result = 0;
	size_t start = 0, end = (size_t)-1;
	union dcharptr lhs, lep, rhs;
	size_t mylen;
	if (DeeArg_UnpackKw(argc, argv, kw, find_kwlist,
	                    "o|" UNPdSIZ UNPdSIZ ":count",
	                    &other, &start, &end))
		goto err;
	if (DeeObject_AssertTypeExact(other, &DeeString_Type))
		goto err;
	SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON(DeeString_WIDTH(self),
	                                        DeeString_WIDTH(other))) {

	CASE_WIDTH_1BYTE:
		lhs.cp8 = DeeString_As1Byte((DeeObject *)self);
		mylen   = DeeString_SIZE(self);
		CLAMP_SUBSTR(&start, &end, &mylen, not_found);
		lhs.cp8 += start;
		lep.cp8 = lhs.cp8 + mylen;
		rhs.cp8 = DeeString_As1Byte((DeeObject *)other);
		while ((lhs.cp8 = memmemb(lhs.cp8, (size_t)(lep.cp8 - lhs.cp8),
		                          rhs.cp8, WSTR_LENGTH(rhs.cp8))) != NULL) {
			lhs.cp8 += WSTR_LENGTH(rhs.cp8);
			++result;
		}
		break;

	CASE_WIDTH_2BYTE:
		lhs.cp16 = DeeString_As2Byte((DeeObject *)self);
		if unlikely(!lhs.cp16)
			goto err;
		mylen = WSTR_LENGTH(lhs.cp16);
		CLAMP_SUBSTR(&start, &end, &mylen, not_found);
		lhs.cp16 += start;
		lep.cp16 = lhs.cp16 + mylen;
		rhs.cp16 = DeeString_As2Byte((DeeObject *)other);
		if unlikely(!rhs.cp16)
			goto err;
		while ((lhs.cp16 = memmemw(lhs.cp16, (size_t)(lep.cp16 - lhs.cp16),
		                           rhs.cp16, WSTR_LENGTH(rhs.cp16))) != NULL) {
			lhs.cp16 += WSTR_LENGTH(rhs.cp16);
			++result;
		}
		break;

	CASE_WIDTH_4BYTE:
		lhs.cp32 = DeeString_As4Byte((DeeObject *)self);
		if unlikely(!lhs.cp32)
			goto err;
		mylen = WSTR_LENGTH(lhs.cp32);
		CLAMP_SUBSTR(&start, &end, &mylen, not_found);
		lhs.cp32 += start;
		lep.cp32 = lhs.cp32 + mylen;
		rhs.cp32 = DeeString_As4Byte((DeeObject *)other);
		if unlikely(!rhs.cp32)
			goto err;
		while ((lhs.cp32 = memmeml(lhs.cp32, (size_t)(lep.cp32 - lhs.cp32),
		                           rhs.cp32, WSTR_LENGTH(rhs.cp32))) != NULL) {
			lhs.cp32 += WSTR_LENGTH(rhs.cp32);
			++result;
		}
		break;
	}
not_found:
	return DeeInt_NewSize(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
string_casecount(String *self, size_t argc,
                 DeeObject *const *argv, DeeObject *kw) {
	String *other;
	size_t result = 0;
	size_t start = 0, end = (size_t)-1, match_length;
	union dcharptr lhs, lep, rhs;
	size_t mylen;
	if (DeeArg_UnpackKw(argc, argv, kw, find_kwlist,
	                    "o|" UNPdSIZ UNPdSIZ ":casecount",
	                    &other, &start, &end))
		goto err;
	if (DeeObject_AssertTypeExact(other, &DeeString_Type))
		goto err;
	SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON(DeeString_WIDTH(self),
	                                        DeeString_WIDTH(other))) {

	CASE_WIDTH_1BYTE:
		lhs.cp8 = DeeString_As1Byte((DeeObject *)self);
		mylen   = DeeString_SIZE(self);
		CLAMP_SUBSTR(&start, &end, &mylen, not_found);
		lhs.cp8 += start;
		lep.cp8 = lhs.cp8 + mylen;
		rhs.cp8 = DeeString_As1Byte((DeeObject *)other);
		while ((lhs.cp8 = memcasememb(lhs.cp8, (size_t)(lep.cp8 - lhs.cp8),
		                              rhs.cp8, WSTR_LENGTH(rhs.cp8),
		                              &match_length)) != NULL) {
			lhs.cp8 += match_length;
			++result;
		}
		break;

	CASE_WIDTH_2BYTE:
		lhs.cp16 = DeeString_As2Byte((DeeObject *)self);
		if unlikely(!lhs.cp16)
			goto err;
		mylen = WSTR_LENGTH(lhs.cp16);
		CLAMP_SUBSTR(&start, &end, &mylen, not_found);
		lhs.cp16 += start;
		lep.cp16 = lhs.cp16 + mylen;
		rhs.cp16 = DeeString_As2Byte((DeeObject *)other);
		if unlikely(!rhs.cp16)
			goto err;
		while ((lhs.cp16 = memcasememw(lhs.cp16, (size_t)(lep.cp16 - lhs.cp16),
		                               rhs.cp16, WSTR_LENGTH(rhs.cp16),
		                               &match_length)) != NULL) {
			lhs.cp16 += match_length;
			++result;
		}
		break;

	CASE_WIDTH_4BYTE:
		lhs.cp32 = DeeString_As4Byte((DeeObject *)self);
		if unlikely(!lhs.cp32)
			goto err;
		mylen = WSTR_LENGTH(lhs.cp32);
		CLAMP_SUBSTR(&start, &end, &mylen, not_found);
		lhs.cp32 += start;
		lep.cp32 = lhs.cp32 + mylen;
		rhs.cp32 = DeeString_As4Byte((DeeObject *)other);
		if unlikely(!rhs.cp32)
			goto err;
		while ((lhs.cp32 = memcasememl(lhs.cp32, (size_t)(lep.cp32 - lhs.cp32),
		                               rhs.cp32, WSTR_LENGTH(rhs.cp32),
		                               &match_length)) != NULL) {
			lhs.cp32 += match_length;
			++result;
		}
		break;
	}
not_found:
	return DeeInt_NewSize(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
string_contains_f(String *self, size_t argc,
                  DeeObject *const *argv, DeeObject *kw) {
	String *other;
	size_t start = 0, end = (size_t)-1;
	union dcharptr lhs, rhs;
	size_t mylen;
	if (DeeArg_UnpackKw(argc, argv, kw, find_kwlist,
	                    "o|" UNPdSIZ UNPdSIZ ":contains",
	                    &other, &start, &end))
		goto err;
	if (DeeObject_AssertTypeExact(other, &DeeString_Type))
		goto err;
	SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON(DeeString_WIDTH(self),
	                                        DeeString_WIDTH(other))) {

	CASE_WIDTH_1BYTE:
		lhs.cp8 = DeeString_As1Byte((DeeObject *)self);
		mylen   = DeeString_SIZE(self);
		CLAMP_SUBSTR(&start, &end, &mylen, not_found);
		rhs.cp8 = DeeString_As1Byte((DeeObject *)other);
		if (!memmemb(lhs.cp8 + start, mylen,
		             rhs.cp8, WSTR_LENGTH(rhs.cp8)))
			goto not_found;
		break;

	CASE_WIDTH_2BYTE:
		lhs.cp16 = DeeString_As2Byte((DeeObject *)self);
		if unlikely(!lhs.cp16)
			goto err;
		mylen = WSTR_LENGTH(lhs.cp16);
		CLAMP_SUBSTR(&start, &end, &mylen, not_found);
		rhs.cp16 = DeeString_As2Byte((DeeObject *)other);
		if unlikely(!rhs.cp16)
			goto err;
		if (!memmemw(lhs.cp16 + start, mylen,
		             rhs.cp16, WSTR_LENGTH(rhs.cp16)))
			goto not_found;
		break;

	CASE_WIDTH_4BYTE:
		lhs.cp32 = DeeString_As4Byte((DeeObject *)self);
		if unlikely(!lhs.cp32)
			goto err;
		mylen = WSTR_LENGTH(lhs.cp32);
		CLAMP_SUBSTR(&start, &end, &mylen, not_found);
		rhs.cp32 = DeeString_As4Byte((DeeObject *)other);
		if unlikely(!rhs.cp32)
			goto err;
		if (!memmeml(lhs.cp32 + start, mylen,
		             rhs.cp32, WSTR_LENGTH(rhs.cp32)))
			goto not_found;
		break;
	}
	return_true;
not_found:
	return_false;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
string_casecontains_f(String *self, size_t argc,
                      DeeObject *const *argv, DeeObject *kw) {
	String *other;
	size_t start = 0, end = (size_t)-1;
	union dcharptr lhs, rhs;
	size_t mylen;
	if (DeeArg_UnpackKw(argc, argv, kw, find_kwlist,
	                    "o|" UNPdSIZ UNPdSIZ ":casecontains",
	                    &other, &start, &end))
		goto err;
	if (DeeObject_AssertTypeExact(other, &DeeString_Type))
		goto err;
	SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON(DeeString_WIDTH(self),
	                                        DeeString_WIDTH(other))) {

	CASE_WIDTH_1BYTE:
		lhs.cp8 = DeeString_As1Byte((DeeObject *)self);
		mylen   = DeeString_SIZE(self);
		CLAMP_SUBSTR(&start, &end, &mylen, not_found);
		rhs.cp8 = DeeString_As1Byte((DeeObject *)other);
		if (!memcasememb(lhs.cp8 + start, mylen,
		                 rhs.cp8, WSTR_LENGTH(rhs.cp8), NULL))
			goto not_found;
		break;

	CASE_WIDTH_2BYTE:
		lhs.cp16 = DeeString_As2Byte((DeeObject *)self);
		if unlikely(!lhs.cp16)
			goto err;
		mylen = WSTR_LENGTH(lhs.cp16);
		CLAMP_SUBSTR(&start, &end, &mylen, not_found);
		rhs.cp16 = DeeString_As2Byte((DeeObject *)other);
		if unlikely(!rhs.cp16)
			goto err;
		if (!memcasememw(lhs.cp16 + start, mylen,
		                 rhs.cp16, WSTR_LENGTH(rhs.cp16), NULL))
			goto not_found;
		break;

	CASE_WIDTH_4BYTE:
		lhs.cp32 = DeeString_As4Byte((DeeObject *)self);
		if unlikely(!lhs.cp32)
			goto err;
		mylen = WSTR_LENGTH(lhs.cp32);
		CLAMP_SUBSTR(&start, &end, &mylen, not_found);
		rhs.cp32 = DeeString_As4Byte((DeeObject *)other);
		if unlikely(!rhs.cp32)
			goto err;
		if (!memcasememl(lhs.cp32 + start, mylen,
		                 rhs.cp32, WSTR_LENGTH(rhs.cp32), NULL))
			goto not_found;
		break;
	}
	return_true;
not_found:
	return_false;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF String *DCALL
string_zfill(String *self, size_t argc, DeeObject *const *argv) {
	size_t result_length, my_len, fl_len, fill_front;
	String *filler_ob = NULL;
	DREF String *result;
	union dcharptr dst, buf, my_str, fl_str;
	if (DeeArg_Unpack(argc, argv, UNPuSIZ "|o:zfill", &result_length, &filler_ob))
		goto err;
	my_len = DeeString_WLEN(self);
	if (result_length <= my_len)
		return_reference_(self);
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
			result = (DREF String *)DeeString_NewBuffer(result_length);
			if unlikely(!result)
				goto err;
			dst.cp8 = DeeString_As1Byte((DeeObject *)result);
			while (my_len && DeeUni_IsSign(my_str.cp8[0])) {
				*dst.cp8++ = *my_str.cp8++;
				--my_len;
			}
			dst.cp8 = mempfilb(dst.cp8, fill_front, fl_str.cp8, fl_len);
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
			dst.cp16 = mempfilw(dst.cp16, fill_front, fl_str.cp16, fl_len);
			memcpyw(dst.cp16, my_str.cp16, my_len);
			result = (DREF String *)DeeString_Pack2ByteBuffer(buf.cp16);
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
			dst.cp32 = mempfill(dst.cp32, fill_front, fl_str.cp32, fl_len);
			memcpyl(dst.cp32, my_str.cp32, my_len);
			result = (DREF String *)DeeString_Pack4ByteBuffer(buf.cp32);
			break;
		}
	} else {
		SWITCH_SIZEOF_WIDTH(DeeString_WIDTH(self)) {

		CASE_WIDTH_1BYTE:
			my_str.cp8 = DeeString_Get1Byte((DeeObject *)self);
			result     = (DREF String *)DeeString_NewBuffer(result_length);
			if unlikely(!result)
				goto err;
			dst.cp8 = DeeString_As1Byte((DeeObject *)result);
			while (my_len && DeeUni_IsSign(my_str.cp8[0])) {
				*dst.cp8++ = *my_str.cp8++;
				--my_len;
			}
			dst.cp8 = mempsetb(dst.cp8, UNICODE_ZERO, fill_front);
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
			dst.cp16 = mempsetw(dst.cp16, UNICODE_ZERO, fill_front);
			memcpyw(dst.cp16, my_str.cp16, my_len);
			result = (DREF String *)DeeString_Pack2ByteBuffer(buf.cp16);
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
			dst.cp32 = mempsetl(dst.cp32, UNICODE_ZERO, fill_front);
			memcpyl(dst.cp32, my_str.cp32, my_len);
			result = (DREF String *)DeeString_Pack4ByteBuffer(buf.cp32);
			break;
		}
	}
	return result;
empty_filler:
	err_empty_filler();
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF String *DCALL
string_reversed(String *self, size_t argc,
                DeeObject *const *argv, DeeObject *kw) {
	size_t begin = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, substr_kwlist, "|" UNPdSIZ UNPdSIZ ":reversed", &begin, &end))
		goto err;
	return DeeString_Reversed(self, begin, end);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF String *DCALL
string_expandtabs(String *self, size_t argc, DeeObject *const *argv) {
	size_t tab_width = 8;
	if (DeeArg_Unpack(argc, argv, "|" UNPuSIZ ":expandtabs", &tab_width))
		goto err;
	return DeeString_ExpandTabs(self, tab_width);
err:
	return NULL;
}

struct string_join_data {
	struct unicode_printer sjd_out;   /* Output printer */
	String                *sjd_sep;   /* [1..1] Separator */
	bool                   sjd_first; /* True if this is the first element. */
};

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
string_join_cb(void *arg, DeeObject *elem) {
	struct string_join_data *data = (struct string_join_data *)arg;
	/* Print `self' prior to every object, starting with the 2nd one. */
	if (!data->sjd_first) {
		if unlikely(unicode_printer_printstring(&data->sjd_out, (DeeObject *)data->sjd_sep) < 0)
			goto err;
	}
	if unlikely(unicode_printer_printobject(&data->sjd_out, elem) < 0)
		goto err;
	data->sjd_first = false;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF String *DCALL
string_join(String *self, size_t argc, DeeObject *const *argv) {
	DeeObject *seq;
	struct string_join_data data;
	if (DeeArg_Unpack(argc, argv, "o:join", &seq))
		goto err;
	unicode_printer_init(&data.sjd_out);
	data.sjd_sep   = self;
	data.sjd_first = true;
	if unlikely(DeeObject_Foreach(seq, &string_join_cb, &data) < 0)
		goto err_printer;
	return (DREF String *)unicode_printer_pack(&data.sjd_out);
err_printer:
	unicode_printer_fini(&data.sjd_out);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF String *DCALL
string_unifylines(String *self, size_t argc, DeeObject *const *argv) {
	String *replacement = NULL;
	if (DeeArg_Unpack(argc, argv, "|o:unifylines", &replacement))
		goto err;
	if likely(!replacement)
		return DeeString_UnifyLinesLf(self);
	if (DeeObject_AssertTypeExact(replacement, &DeeString_Type))
		goto err;
	return DeeString_UnifyLines(self, replacement);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeTupleObject *DCALL
partition_pack_notfoundb(String *__restrict self,
                         uint8_t *lhs,
                         size_t lhs_length) {
	DREF String *lhs_string;
	DREF DeeTupleObject *result;
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
	DeeTuple_SET(result, 0, lhs_string);
	DeeTuple_SET(result, 1, Dee_EmptyString);
	DeeTuple_SET(result, 2, Dee_EmptyString);
	return result;
err_r:
	DeeTuple_FreeUninitialized(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeTupleObject *DCALL
partition_pack_notfoundw(String *__restrict self,
                         uint16_t *lhs,
                         size_t lhs_length) {
	DREF String *lhs_string;
	DREF DeeTupleObject *result;
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
	DeeTuple_SET(result, 0, lhs_string);
	DeeTuple_SET(result, 1, Dee_EmptyString);
	DeeTuple_SET(result, 2, Dee_EmptyString);
	return result;
err_r:
	DeeTuple_FreeUninitialized(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeTupleObject *DCALL
partition_pack_notfoundl(String *__restrict self,
                         uint32_t *lhs,
                         size_t lhs_length) {
	DREF String *lhs_string;
	DREF DeeTupleObject *result;
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
	DeeTuple_SET(result, 0, lhs_string);
	DeeTuple_SET(result, 1, Dee_EmptyString);
	DeeTuple_SET(result, 2, Dee_EmptyString);
	return result;
err_r:
	DeeTuple_FreeUninitialized(result);
err:
	return NULL;
}

PRIVATE WUNUSED DREF DeeTupleObject *DCALL
partition_packb(String *__restrict other,
                uint8_t *lhs, size_t lhs_length,
                uint8_t *ptr, size_t ptr_length) {
	DREF String *prestr, *poststr;
	DREF DeeTupleObject *result;
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
	DeeTuple_SET(result, 0, prestr);
	DeeTuple_SET(result, 1, other);
	DeeTuple_SET(result, 2, poststr);
	Dee_Incref(other);
	return result;
err_r_pre:
	Dee_Decref_likely(prestr);
err_r:
	DeeTuple_FreeUninitialized(result);
err:
	return NULL;
}

PRIVATE WUNUSED DREF DeeTupleObject *DCALL
partition_packw(String *__restrict other,
                uint16_t *lhs, size_t lhs_length,
                uint16_t *ptr, size_t ptr_length) {
	DREF String *prestr, *poststr;
	DREF DeeTupleObject *result;
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
	DeeTuple_SET(result, 0, prestr);
	DeeTuple_SET(result, 1, other);
	DeeTuple_SET(result, 2, poststr);
	Dee_Incref(other);
	return result;
err_r_pre:
	Dee_Decref_likely(prestr);
err_r:
	DeeTuple_FreeUninitialized(result);
err:
	return NULL;
}

PRIVATE WUNUSED DREF DeeTupleObject *DCALL
partition_packl(String *__restrict other,
                uint32_t *lhs, size_t lhs_length,
                uint32_t *ptr, size_t ptr_length) {
	DREF String *prestr, *poststr;
	DREF DeeTupleObject *result;
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
	DeeTuple_SET(result, 0, prestr);
	DeeTuple_SET(result, 1, other);
	DeeTuple_SET(result, 2, poststr);
	Dee_Incref(other);
	return result;
err_r_pre:
	Dee_Decref_likely(prestr);
err_r:
	DeeTuple_FreeUninitialized(result);
err:
	return NULL;
}

PRIVATE WUNUSED DREF DeeTupleObject *DCALL
partition_packb_fold(uint8_t *__restrict lhs, size_t lhs_length,
                     uint8_t *__restrict mtc, size_t mtc_length,
                     uint8_t *__restrict rhs, size_t rhs_length) {
	DREF String *temp;
	DREF DeeTupleObject *result;
	result = DeeTuple_NewUninitialized(3);
	if unlikely(!result)
		goto err;
	temp = (DREF String *)DeeString_New1Byte(lhs, lhs_length);
	if unlikely(!temp)
		goto err_r_0;
	DeeTuple_SET(result, 0, temp); /* Inherit reference */
	temp = (DREF String *)DeeString_New1Byte(mtc, mtc_length);
	if unlikely(!temp)
		goto err_r_1;
	DeeTuple_SET(result, 1, temp); /* Inherit reference */
	temp = (DREF String *)DeeString_New1Byte(rhs, rhs_length);
	if unlikely(!temp)
		goto err_r_2;
	DeeTuple_SET(result, 2, temp); /* Inherit reference */
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

PRIVATE WUNUSED DREF DeeTupleObject *DCALL
partition_packw_fold(uint16_t *__restrict lhs, size_t lhs_length,
                     uint16_t *__restrict mtc, size_t mtc_length,
                     uint16_t *__restrict rhs, size_t rhs_length) {
	DREF String *temp;
	DREF DeeTupleObject *result;
	result = DeeTuple_NewUninitialized(3);
	if unlikely(!result)
		goto err;
	temp = (DREF String *)DeeString_New2Byte(lhs, lhs_length);
	if unlikely(!temp)
		goto err_r_0;
	DeeTuple_SET(result, 0, temp); /* Inherit reference */
	temp = (DREF String *)DeeString_New2Byte(mtc, mtc_length);
	if unlikely(!temp)
		goto err_r_1;
	DeeTuple_SET(result, 1, temp); /* Inherit reference */
	temp = (DREF String *)DeeString_New2Byte(rhs, rhs_length);
	if unlikely(!temp)
		goto err_r_2;
	DeeTuple_SET(result, 2, temp); /* Inherit reference */
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

PRIVATE WUNUSED DREF DeeTupleObject *DCALL
partition_packl_fold(uint32_t *__restrict lhs, size_t lhs_length,
                     uint32_t *__restrict mtc, size_t mtc_length,
                     uint32_t *__restrict rhs, size_t rhs_length) {
	DREF String *temp;
	DREF DeeTupleObject *result;
	result = DeeTuple_NewUninitialized(3);
	if unlikely(!result)
		goto err;
	temp = (DREF String *)DeeString_New4Byte(lhs, lhs_length);
	if unlikely(!temp)
		goto err_r_0;
	DeeTuple_SET(result, 0, temp); /* Inherit reference */
	temp = (DREF String *)DeeString_New4Byte(mtc, mtc_length);
	if unlikely(!temp)
		goto err_r_1;
	DeeTuple_SET(result, 1, temp); /* Inherit reference */
	temp = (DREF String *)DeeString_New4Byte(rhs, rhs_length);
	if unlikely(!temp)
		goto err_r_2;
	DeeTuple_SET(result, 2, temp); /* Inherit reference */
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

PRIVATE WUNUSED NONNULL((1)) DREF DeeTupleObject *DCALL
string_partition(String *self, size_t argc,
                 DeeObject *const *argv, DeeObject *kw) {
	String *other;
	union dcharptr lhs, rhs, ptr;
	size_t mylen, start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, find_kwlist,
	                    "o|" UNPdSIZ UNPdSIZ ":partition",
	                    &other, &start, &end))
		goto err;
	if (DeeObject_AssertTypeExact(other, &DeeString_Type))
		goto err;
	SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON(DeeString_WIDTH(self),
	                                        DeeString_WIDTH(other))) {

	CASE_WIDTH_1BYTE:
		lhs.cp8 = DeeString_As1Byte((DeeObject *)self);
		rhs.cp8 = DeeString_As1Byte((DeeObject *)other);
		mylen   = WSTR_LENGTH(lhs.ptr);
		CLAMP_SUBSTR(&start, &end, &mylen, not_foundb_zero);
		lhs.cp8 += start;
		ptr.cp8 = memmemb(lhs.cp8, mylen,
		                  rhs.cp8, WSTR_LENGTH(rhs.cp8));
		if (!ptr.cp8)
			goto not_foundb;
		return partition_packb(other,
		                       lhs.cp8, mylen,
		                       ptr.cp8, WSTR_LENGTH(rhs.cp8));
not_foundb_zero:
		mylen = 0;
not_foundb:
		return partition_pack_notfoundb(self, lhs.cp8, mylen);

	CASE_WIDTH_2BYTE:
		lhs.cp16 = DeeString_As2Byte((DeeObject *)self);
		if unlikely(!lhs.cp16)
			goto err;
		mylen = WSTR_LENGTH(lhs.ptr);
		CLAMP_SUBSTR(&start, &end, &mylen, not_foundw_zero);
		lhs.cp16 += start;
		rhs.cp16 = DeeString_As2Byte((DeeObject *)other);
		if unlikely(!rhs.cp16)
			goto err;
		ptr.cp16 = memmemw(lhs.cp16, mylen,
		                   rhs.cp16, WSTR_LENGTH(rhs.cp16));
		if (!ptr.cp16)
			goto not_foundw;
		return partition_packw(other,
		                       lhs.cp16, mylen,
		                       ptr.cp16, WSTR_LENGTH(rhs.cp16));
not_foundw_zero:
		mylen = 0;
not_foundw:
		return partition_pack_notfoundw(self, lhs.cp16, mylen);

	CASE_WIDTH_4BYTE:
		lhs.cp32 = DeeString_As4Byte((DeeObject *)self);
		if unlikely(!lhs.cp32)
			goto err;
		mylen = WSTR_LENGTH(lhs.ptr);
		CLAMP_SUBSTR(&start, &end, &mylen, not_foundl_zero);
		lhs.cp32 += start;
		rhs.cp32 = DeeString_As4Byte((DeeObject *)other);
		if unlikely(!rhs.cp32)
			goto err;
		ptr.cp32 = memmeml(lhs.cp32, mylen,
		                   rhs.cp32, WSTR_LENGTH(rhs.cp32));
		if (!ptr.cp32)
			goto not_foundl;
		return partition_packl(other,
		                       lhs.cp32, mylen,
		                       ptr.cp32, WSTR_LENGTH(rhs.cp32));
not_foundl_zero:
		mylen = 0;
not_foundl:
		return partition_pack_notfoundl(self, lhs.cp32, mylen);
	}
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeTupleObject *DCALL
string_rpartition(String *self, size_t argc,
                  DeeObject *const *argv, DeeObject *kw) {
	String *other;
	union dcharptr lhs, rhs, ptr;
	size_t mylen, start = 0, end = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, find_kwlist,
	                    "o|" UNPdSIZ UNPdSIZ ":rpartition",
	                    &other, &start, &end))
		goto err;
	if (DeeObject_AssertTypeExact(other, &DeeString_Type))
		goto err;
	SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON(DeeString_WIDTH(self),
	                                        DeeString_WIDTH(other))) {

	CASE_WIDTH_1BYTE:
		lhs.cp8 = DeeString_As1Byte((DeeObject *)self);
		mylen   = WSTR_LENGTH(lhs.ptr);
		CLAMP_SUBSTR(&start, &end, &mylen, not_foundb_zero);
		lhs.cp8 += start;
		rhs.cp8 = DeeString_As1Byte((DeeObject *)other);
		ptr.cp8 = memrmemb(lhs.cp8, mylen,
		                   rhs.cp8, WSTR_LENGTH(rhs.cp8));
		if (!ptr.cp8)
			goto not_foundb;
		return partition_packb(other,
		                       lhs.cp8, mylen,
		                       ptr.cp8, WSTR_LENGTH(rhs.cp8));
not_foundb_zero:
		mylen = 0;
not_foundb:
		return partition_pack_notfoundb(self, lhs.cp8, mylen);

	CASE_WIDTH_2BYTE:
		lhs.cp16 = DeeString_As2Byte((DeeObject *)self);
		if unlikely(!lhs.cp16)
			goto err;
		mylen = WSTR_LENGTH(lhs.ptr);
		CLAMP_SUBSTR(&start, &end, &mylen, not_foundw_zero);
		lhs.cp16 += start;
		rhs.cp16 = DeeString_As2Byte((DeeObject *)other);
		if unlikely(!rhs.cp16)
			goto err;
		ptr.cp16 = memrmemw(lhs.cp16, mylen,
		                    rhs.cp16, WSTR_LENGTH(rhs.cp16));
		if (!ptr.cp16)
			goto not_foundw;
		return partition_packw(other,
		                       lhs.cp16, mylen,
		                       ptr.cp16, WSTR_LENGTH(rhs.cp16));
not_foundw_zero:
		mylen = 0;
not_foundw:
		return partition_pack_notfoundw(self, lhs.cp16, mylen);

	CASE_WIDTH_4BYTE:
		lhs.cp32 = DeeString_As4Byte((DeeObject *)self);
		if unlikely(!lhs.cp32)
			goto err;
		mylen = WSTR_LENGTH(lhs.ptr);
		CLAMP_SUBSTR(&start, &end, &mylen, not_foundl_zero);
		lhs.cp32 += start;
		rhs.cp32 = DeeString_As4Byte((DeeObject *)other);
		if unlikely(!rhs.cp32)
			goto err;
		ptr.cp32 = memrmeml(lhs.cp32, mylen,
		                    rhs.cp32, WSTR_LENGTH(rhs.cp32));
		if (!ptr.cp32)
			goto not_foundl;
		return partition_packl(other,
		                       lhs.cp32, mylen,
		                       ptr.cp32, WSTR_LENGTH(rhs.cp32));
not_foundl_zero:
		mylen = 0;
not_foundl:
		return partition_pack_notfoundl(self, lhs.cp32, mylen);
	}
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeTupleObject *DCALL
string_casepartition(String *self, size_t argc,
                     DeeObject *const *argv, DeeObject *kw) {
	String *other;
	union dcharptr lhs, rhs, ptr;
	size_t mylen, start = 0, end = (size_t)-1, match_length;
	if (DeeArg_UnpackKw(argc, argv, kw, find_kwlist,
	                    "o|" UNPdSIZ UNPdSIZ ":casepartition",
	                    &other, &start, &end))
		goto err;
	if (DeeObject_AssertTypeExact(other, &DeeString_Type))
		goto err;
	SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON(DeeString_WIDTH(self),
	                                        DeeString_WIDTH(other))) {

	CASE_WIDTH_1BYTE:
		lhs.cp8 = DeeString_As1Byte((DeeObject *)self);
		mylen   = WSTR_LENGTH(lhs.ptr);
		CLAMP_SUBSTR(&start, &end, &mylen, not_foundb_zero);
		lhs.cp8 += start;
		rhs.cp8 = DeeString_As1Byte((DeeObject *)other);
		ptr.cp8 = memcasememb(lhs.cp8, mylen,
		                      rhs.cp8, WSTR_LENGTH(rhs.cp8),
		                      &match_length);
		if (!ptr.cp8)
			goto not_foundb;
		return partition_packb_fold(lhs.cp8, (size_t)(ptr.cp8 - lhs.cp8),
		                            ptr.cp8, match_length,
		                            ptr.cp8 + match_length,
		                            mylen - ((size_t)(ptr.cp8 - lhs.cp8) + match_length));
not_foundb_zero:
		mylen = 0;
not_foundb:
		return partition_pack_notfoundb(self, lhs.cp8, mylen);

	CASE_WIDTH_2BYTE:
		lhs.cp16 = DeeString_As2Byte((DeeObject *)self);
		if unlikely(!lhs.cp16)
			goto err;
		mylen = WSTR_LENGTH(lhs.ptr);
		CLAMP_SUBSTR(&start, &end, &mylen, not_foundw_zero);
		lhs.cp16 += start;
		rhs.cp16 = DeeString_As2Byte((DeeObject *)other);
		if unlikely(!rhs.cp16)
			goto err;
		ptr.cp16 = memcasememw(lhs.cp16, mylen,
		                       rhs.cp16, WSTR_LENGTH(rhs.cp16),
		                       &match_length);
		if (!ptr.cp16)
			goto not_foundw;
		return partition_packw_fold(lhs.cp16, (size_t)(ptr.cp16 - lhs.cp16),
		                            ptr.cp16, match_length,
		                            ptr.cp16 + match_length,
		                            mylen - ((size_t)(ptr.cp16 - lhs.cp16) + match_length));
not_foundw_zero:
		mylen = 0;
not_foundw:
		return partition_pack_notfoundw(self, lhs.cp16, mylen);

	CASE_WIDTH_4BYTE:
		lhs.cp32 = DeeString_As4Byte((DeeObject *)self);
		if unlikely(!lhs.cp32)
			goto err;
		mylen = WSTR_LENGTH(lhs.ptr);
		CLAMP_SUBSTR(&start, &end, &mylen, not_foundl_zero);
		lhs.cp32 += start;
		rhs.cp32 = DeeString_As4Byte((DeeObject *)other);
		if unlikely(!rhs.cp32)
			goto err;
		ptr.cp32 = memcasememl(lhs.cp32, mylen,
		                       rhs.cp32, WSTR_LENGTH(rhs.cp32),
		                       &match_length);
		if (!ptr.cp32)
			goto not_foundl;
		return partition_packl_fold(lhs.cp32, (size_t)(ptr.cp32 - lhs.cp32),
		                            ptr.cp32, match_length,
		                            ptr.cp32 + match_length,
		                            mylen - ((size_t)(ptr.cp32 - lhs.cp32) + match_length));
not_foundl_zero:
		mylen = 0;
not_foundl:
		return partition_pack_notfoundl(self, lhs.cp32, mylen);
	}
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeTupleObject *DCALL
string_caserpartition(String *self, size_t argc,
                      DeeObject *const *argv, DeeObject *kw) {
	String *other;
	union dcharptr lhs, rhs, ptr;
	size_t mylen, start = 0, end = (size_t)-1, match_length;
	if (DeeArg_UnpackKw(argc, argv, kw, find_kwlist,
	                    "o|" UNPdSIZ UNPdSIZ ":caserpartition",
	                    &other, &start, &end))
		goto err;
	if (DeeObject_AssertTypeExact(other, &DeeString_Type))
		goto err;
	SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON(DeeString_WIDTH(self),
	                                        DeeString_WIDTH(other))) {

	CASE_WIDTH_1BYTE:
		lhs.cp8 = DeeString_As1Byte((DeeObject *)self);
		mylen   = WSTR_LENGTH(lhs.ptr);
		CLAMP_SUBSTR(&start, &end, &mylen, not_foundb_zero);
		lhs.cp8 += start;
		rhs.cp8 = DeeString_As1Byte((DeeObject *)other);
		ptr.cp8 = memcasermemb(lhs.cp8, mylen,
		                       rhs.cp8, WSTR_LENGTH(rhs.cp8),
		                       &match_length);
		if (!ptr.cp8)
			goto not_foundb;
		return partition_packb_fold(lhs.cp8, (size_t)(ptr.cp8 - lhs.cp8),
		                            ptr.cp8, match_length,
		                            ptr.cp8 + match_length,
		                            mylen - ((size_t)(ptr.cp8 - lhs.cp8) + match_length));
not_foundb_zero:
		mylen = 0;
not_foundb:
		return partition_pack_notfoundb(self, lhs.cp8, mylen);

	CASE_WIDTH_2BYTE:
		lhs.cp16 = DeeString_As2Byte((DeeObject *)self);
		if unlikely(!lhs.cp16)
			goto err;
		mylen = WSTR_LENGTH(lhs.ptr);
		CLAMP_SUBSTR(&start, &end, &mylen, not_foundw_zero);
		lhs.cp16 += start;
		rhs.cp16 = DeeString_As2Byte((DeeObject *)other);
		if unlikely(!rhs.cp16)
			goto err;
		ptr.cp16 = memcasermemw(lhs.cp16, mylen,
		                        rhs.cp16, WSTR_LENGTH(rhs.cp16),
		                        &match_length);
		if (!ptr.cp16)
			goto not_foundw;
		return partition_packw_fold(lhs.cp16, (size_t)(ptr.cp16 - lhs.cp16),
		                            ptr.cp16, match_length,
		                            ptr.cp16 + match_length,
		                            mylen - ((size_t)(ptr.cp16 - lhs.cp16) + match_length));
not_foundw_zero:
		mylen = 0;
not_foundw:
		return partition_pack_notfoundw(self, lhs.cp16, mylen);

	CASE_WIDTH_4BYTE:
		lhs.cp32 = DeeString_As4Byte((DeeObject *)self);
		if unlikely(!lhs.cp32)
			goto err;
		mylen = WSTR_LENGTH(lhs.ptr);
		CLAMP_SUBSTR(&start, &end, &mylen, not_foundl_zero);
		lhs.cp32 += start;
		rhs.cp32 = DeeString_As4Byte((DeeObject *)other);
		if unlikely(!rhs.cp32)
			goto err;
		ptr.cp32 = memcasermeml(lhs.cp32, mylen,
		                        rhs.cp32, WSTR_LENGTH(rhs.cp32),
		                        &match_length);
		if (!ptr.cp32)
			goto not_foundl;
		return partition_packl_fold(lhs.cp32, (size_t)(ptr.cp32 - lhs.cp32),
		                            ptr.cp32, match_length,
		                            ptr.cp32 + match_length,
		                            mylen - ((size_t)(ptr.cp32 - lhs.cp32) + match_length));
not_foundl_zero:
		mylen = 0;
not_foundl:
		return partition_pack_notfoundl(self, lhs.cp32, mylen);
	}
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF String *DCALL
string_strip(String *self, size_t argc, DeeObject *const *argv) {
	String *mask = NULL;
	if (DeeArg_Unpack(argc, argv, "|o:strip", &mask))
		goto err;
	if (!mask)
		return DeeString_StripSpc(self);
	if (DeeObject_AssertTypeExact(mask, &DeeString_Type))
		goto err;
	return DeeString_StripMask(self, mask);
err:
	return NULL;
}

#ifndef LSTRIP_RSTRIP_KWLIST_DEFINED
#define LSTRIP_RSTRIP_KWLIST_DEFINED
PRIVATE struct keyword lstrip_rstrip_kwlist[] = { K(mask), K(max), KEND };
#endif /* !lstrip_rstrip_kwlist_DEFINED */

PRIVATE WUNUSED NONNULL((1)) DREF String *DCALL
string_lstrip(String *self, size_t argc,
              DeeObject *const *argv, DeeObject *kw) {
	String *mask = NULL;
	size_t max_count = SIZE_MAX;
	if (DeeArg_UnpackKw(argc, argv, kw, lstrip_rstrip_kwlist,
	                    "|o" UNPuSIZ ":lstrip", &mask, &max_count))
		goto err;
	if (!mask)
		return DeeString_LStripSpc(self, max_count);
	if (DeeObject_AssertTypeExact(mask, &DeeString_Type))
		goto err;
	return DeeString_LStripMask(self, mask, max_count);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF String *DCALL
string_rstrip(String *self, size_t argc,
              DeeObject *const *argv, DeeObject *kw) {
	String *mask = NULL;
	size_t max_count = SIZE_MAX;
	if (DeeArg_UnpackKw(argc, argv, kw, lstrip_rstrip_kwlist,
	                    "|o" UNPuSIZ ":rstrip", &mask, &max_count))
		goto err;
	if (!mask)
		return DeeString_RStripSpc(self, max_count);
	if (DeeObject_AssertTypeExact(mask, &DeeString_Type))
		goto err;
	return DeeString_RStripMask(self, mask, max_count);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF String *DCALL
string_casestrip(String *self, size_t argc, DeeObject *const *argv) {
	String *mask = NULL;
	if (DeeArg_Unpack(argc, argv, "|o:casestrip", &mask))
		goto err;
	if (!mask)
		return DeeString_StripSpc(self);
	if (DeeObject_AssertTypeExact(mask, &DeeString_Type))
		goto err;
	return DeeString_CaseStripMask(self, mask);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF String *DCALL
string_caselstrip(String *self, size_t argc,
                  DeeObject *const *argv, DeeObject *kw) {
	String *mask = NULL;
	size_t max_count = SIZE_MAX;
	if (DeeArg_UnpackKw(argc, argv, kw, lstrip_rstrip_kwlist,
	                    "|o" UNPuSIZ ":caselstrip", &mask, &max_count))
		goto err;
	if (!mask)
		return DeeString_LStripSpc(self, max_count);
	if (DeeObject_AssertTypeExact(mask, &DeeString_Type))
		goto err;
	return DeeString_CaseLStripMask(self, mask, max_count);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF String *DCALL
string_caserstrip(String *self, size_t argc,
                  DeeObject *const *argv, DeeObject *kw) {
	String *mask = NULL;
	size_t max_count = SIZE_MAX;
	if (DeeArg_UnpackKw(argc, argv, kw, lstrip_rstrip_kwlist,
	                    "|o" UNPuSIZ ":caserstrip", &mask, &max_count))
		goto err;
	if (!mask)
		return DeeString_RStripSpc(self, max_count);
	if (DeeObject_AssertTypeExact(mask, &DeeString_Type))
		goto err;
	return DeeString_CaseRStripMask(self, mask, max_count);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF String *DCALL
string_sstrip(String *self, size_t argc, DeeObject *const *argv) {
	String *needle;
	if (DeeArg_Unpack(argc, argv, "o:sstrip", &needle))
		goto err;
	if (DeeObject_AssertTypeExact(needle, &DeeString_Type))
		goto err;
	return DeeString_SStrip(self, needle);
err:
	return NULL;
}

#ifndef LSSTRIP_RSSTRIP_KWLIST_DEFINED
#define LSSTRIP_RSSTRIP_KWLIST_DEFINED
PRIVATE struct keyword lsstrip_rsstrip_kwlist[] = { K(needle), K(max), KEND };
#endif /* !lsstrip_rsstrip_kwlist_DEFINED */

PRIVATE WUNUSED NONNULL((1)) DREF String *DCALL
string_lsstrip(String *self, size_t argc,
               DeeObject *const *argv, DeeObject *kw) {
	String *needle;
	size_t max_count = SIZE_MAX;
	if (DeeArg_UnpackKw(argc, argv, kw, lstrip_rstrip_kwlist,
	                    "o|" UNPuSIZ ":lsstrip", &needle, &max_count))
		goto err;
	if (DeeObject_AssertTypeExact(needle, &DeeString_Type))
		goto err;
	return DeeString_LSStrip(self, needle, max_count);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF String *DCALL
string_rsstrip(String *self, size_t argc,
               DeeObject *const *argv, DeeObject *kw) {
	String *needle;
	size_t max_count = SIZE_MAX;
	if (DeeArg_UnpackKw(argc, argv, kw, lstrip_rstrip_kwlist,
	                    "o|" UNPuSIZ ":rsstrip", &needle, &max_count))
		goto err;
	if (DeeObject_AssertTypeExact(needle, &DeeString_Type))
		goto err;
	return DeeString_RSStrip(self, needle, max_count);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF String *DCALL
string_casesstrip(String *self, size_t argc, DeeObject *const *argv) {
	String *needle;
	if (DeeArg_Unpack(argc, argv, "o:casesstrip", &needle))
		goto err;
	if (DeeObject_AssertTypeExact(needle, &DeeString_Type))
		goto err;
	return DeeString_CaseSStrip(self, needle);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF String *DCALL
string_caselsstrip(String *self, size_t argc,
                   DeeObject *const *argv, DeeObject *kw) {
	String *needle;
	size_t max_count = SIZE_MAX;
	if (DeeArg_UnpackKw(argc, argv, kw, lstrip_rstrip_kwlist,
	                    "o|" UNPuSIZ ":caselsstrip", &needle, &max_count))
		goto err;
	if (DeeObject_AssertTypeExact(needle, &DeeString_Type))
		goto err;
	return DeeString_CaseLSStrip(self, needle, max_count);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF String *DCALL
string_casersstrip(String *self, size_t argc,
                   DeeObject *const *argv, DeeObject *kw) {
	String *needle;
	size_t max_count = SIZE_MAX;
	if (DeeArg_UnpackKw(argc, argv, kw, lstrip_rstrip_kwlist,
	                    "o|" UNPuSIZ ":casersstrip", &needle, &max_count))
		goto err;
	if (DeeObject_AssertTypeExact(needle, &DeeString_Type))
		goto err;
	return DeeString_CaseRSStrip(self, needle, max_count);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF String *DCALL
string_striplines(String *self, size_t argc, DeeObject *const *argv) {
	String *mask = NULL;
	if (DeeArg_Unpack(argc, argv, "|o:striplines", &mask))
		goto err;
	if (!mask)
		return DeeString_StripLinesSpc(self);
	if (DeeObject_AssertTypeExact(mask, &DeeString_Type))
		goto err;
	return DeeString_StripLinesMask(self, mask);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF String *DCALL
string_lstriplines(String *self, size_t argc, DeeObject *const *argv) {
	String *mask = NULL;
	if (DeeArg_Unpack(argc, argv, "|o:lstriplines", &mask))
		goto err;
	if (!mask)
		return DeeString_LStripLinesSpc(self);
	if (DeeObject_AssertTypeExact(mask, &DeeString_Type))
		goto err;
	return DeeString_LStripLinesMask(self, mask);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF String *DCALL
string_rstriplines(String *self, size_t argc, DeeObject *const *argv) {
	String *mask = NULL;
	if (DeeArg_Unpack(argc, argv, "|o:rstriplines", &mask))
		goto err;
	if (!mask)
		return DeeString_RStripLinesSpc(self);
	if (DeeObject_AssertTypeExact(mask, &DeeString_Type))
		goto err;
	return DeeString_RStripLinesMask(self, mask);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF String *DCALL
string_casestriplines(String *self, size_t argc, DeeObject *const *argv) {
	String *mask = NULL;
	if (DeeArg_Unpack(argc, argv, "|o:casestriplines", &mask))
		goto err;
	if (!mask)
		return DeeString_StripLinesSpc(self);
	if (DeeObject_AssertTypeExact(mask, &DeeString_Type))
		goto err;
	return DeeString_CaseStripLinesMask(self, mask);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF String *DCALL
string_caselstriplines(String *self, size_t argc, DeeObject *const *argv) {
	String *mask = NULL;
	if (DeeArg_Unpack(argc, argv, "|o:caselstriplines", &mask))
		goto err;
	if (!mask)
		return DeeString_LStripLinesSpc(self);
	if (DeeObject_AssertTypeExact(mask, &DeeString_Type))
		goto err;
	return DeeString_CaseLStripLinesMask(self, mask);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF String *DCALL
string_caserstriplines(String *self, size_t argc, DeeObject *const *argv) {
	String *mask = NULL;
	if (DeeArg_Unpack(argc, argv, "|o:caserstriplines", &mask))
		goto err;
	if (!mask)
		return DeeString_RStripLinesSpc(self);
	if (DeeObject_AssertTypeExact(mask, &DeeString_Type))
		goto err;
	return DeeString_CaseRStripLinesMask(self, mask);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF String *DCALL
string_sstriplines(String *self, size_t argc, DeeObject *const *argv) {
	String *other;
	if (DeeArg_Unpack(argc, argv, "o:sstriplines", &other))
		goto err;
	if (DeeObject_AssertTypeExact(other, &DeeString_Type))
		goto err;
	return DeeString_SStripLines(self, other);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF String *DCALL
string_lsstriplines(String *self, size_t argc, DeeObject *const *argv) {
	String *other;
	if (DeeArg_Unpack(argc, argv, "o:lsstriplines", &other))
		goto err;
	if (DeeObject_AssertTypeExact(other, &DeeString_Type))
		goto err;
	return DeeString_LSStripLines(self, other);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF String *DCALL
string_rsstriplines(String *self, size_t argc, DeeObject *const *argv) {
	String *other;
	if (DeeArg_Unpack(argc, argv, "o:rsstriplines", &other))
		goto err;
	if (DeeObject_AssertTypeExact(other, &DeeString_Type))
		goto err;
	return DeeString_RSStripLines(self, other);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF String *DCALL
string_casesstriplines(String *self, size_t argc, DeeObject *const *argv) {
	String *other;
	if (DeeArg_Unpack(argc, argv, "o:casesstriplines", &other))
		goto err;
	if (DeeObject_AssertTypeExact(other, &DeeString_Type))
		goto err;
	return DeeString_CaseSStripLines(self, other);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF String *DCALL
string_caselsstriplines(String *self, size_t argc, DeeObject *const *argv) {
	String *other;
	if (DeeArg_Unpack(argc, argv, "o:caselsstriplines", &other))
		goto err;
	if (DeeObject_AssertTypeExact(other, &DeeString_Type))
		goto err;
	return DeeString_CaseLSStripLines(self, other);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF String *DCALL
string_casersstriplines(String *self, size_t argc, DeeObject *const *argv) {
	String *other;
	if (DeeArg_Unpack(argc, argv, "o:casersstriplines", &other))
		goto err;
	if (DeeObject_AssertTypeExact(other, &DeeString_Type))
		goto err;
	return DeeString_CaseRSStripLines(self, other);
err:
	return NULL;
}

struct compare_args {
	String *other;
	size_t  my_start;
	size_t  my_end;
	size_t  ot_start;
	size_t  ot_end;
};

PRIVATE int DCALL
get_compare_args(struct compare_args *__restrict args,
                 size_t argc, DeeObject *const *argv,
                 char const *__restrict funname) {
	args->my_start = 0;
	args->my_end   = (size_t)-1;
	args->ot_start = 0;
	args->ot_end   = (size_t)-1;
	switch (argc) {
	case 1:
		args->other = (String *)argv[0];
		if (DeeObject_AssertTypeExact(args->other, &DeeString_Type))
			goto err;
		break;
	case 2:
		if (DeeString_Check(argv[0])) {
			args->other = (String *)argv[0];
			if (DeeObject_AsSSize(argv[1], (dssize_t *)&args->ot_start))
				goto err;
		} else {
			if (DeeObject_AsSSize(argv[0], (dssize_t *)&args->my_start))
				goto err;
			args->other = (String *)argv[1];
			if (DeeObject_AssertTypeExact(args->other, &DeeString_Type))
				goto err;
		}
		break;
	case 3:
		if (DeeString_Check(argv[0])) {
			args->other = (String *)argv[0];
			if (DeeObject_AsSSize(argv[1], (dssize_t *)&args->ot_start))
				goto err;
			if (DeeObject_AsSSize(argv[2], (dssize_t *)&args->ot_end))
				goto err;
		} else if (DeeString_Check(argv[1])) {
			if (DeeObject_AsSSize(argv[0], (dssize_t *)&args->my_start))
				goto err;
			args->other = (String *)argv[1];
			if (DeeObject_AsSSize(argv[2], (dssize_t *)&args->ot_start))
				goto err;
		} else {
			if (DeeObject_AsSSize(argv[0], (dssize_t *)&args->my_start))
				goto err;
			if (DeeObject_AsSSize(argv[1], (dssize_t *)&args->my_end))
				goto err;
			args->other = (String *)argv[2];
			if (DeeObject_AssertTypeExact(args->other, &DeeString_Type))
				goto err;
		}
		break;
	case 4:
		if (DeeObject_AsSSize(argv[0], (dssize_t *)&args->my_start))
			goto err;
		if (DeeString_Check(argv[1])) {
			args->other = (String *)argv[1];
			if (DeeObject_AsSSize(argv[2], (dssize_t *)&args->ot_start))
				goto err;
			if (DeeObject_AsSSize(argv[3], (dssize_t *)&args->ot_end))
				goto err;
		} else {
			if (DeeObject_AsSSize(argv[1], (dssize_t *)&args->my_end))
				goto err;
			args->other = (String *)argv[2];
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
		args->other = (String *)argv[2];
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


/* NOTE: Allowed to returns *OUTSIDE* of [-1,1] */
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
		if (lhs_start >= lhs_end) {
			lhs_len = 0;
		} else {
			lhs_str += lhs_start;
			lhs_len = lhs_end - lhs_start;
		}
		if (!rhs->s_data ||
		    rhs->s_data->u_width == STRING_WIDTH_1BYTE) {
			int result;
			rhs_len = rhs->s_len;
			if (rhs_end > rhs_len)
				rhs_end = rhs_len;
			if (rhs_start >= rhs_end) {
				rhs_len = 0;
			} else {
				rhs_len = rhs_end - rhs_start;
			}
			/* Most simple case: compare ascii/single-byte strings. */
			result = memcmp(lhs_str, rhs->s_str + rhs_start, MIN(lhs_len, rhs_len));
			if (result != 0)
				return result;
		} else {
			struct string_utf *rhs_utf = rhs->s_data;
			switch (rhs_utf->u_width) {

			CASE_WIDTH_2BYTE: {
				uint16_t *rhs_str;
				size_t i, common_len;
				rhs_str = (uint16_t *)rhs_utf->u_data[rhs_utf->u_width];
				rhs_len = WSTR_LENGTH(rhs_str);
				if (rhs_end > rhs_len)
					rhs_end = rhs_len;
				if (rhs_start >= rhs_end) {
					rhs_len = 0;
				} else {
					rhs_str += rhs_start;
					rhs_len = rhs_end - rhs_start;
				}
				common_len = MIN(lhs_len, rhs_len);
				for (i = 0; i < common_len; ++i) {
					if ((uint16_t)(lhs_str[i]) == rhs_str[i])
						continue;
					return (uint16_t)(lhs_str[i]) < rhs_str[i] ? -1 : 1;
				}
			}	break;

			CASE_WIDTH_4BYTE: {
				uint32_t *rhs_str;
				size_t i, common_len;
				rhs_str = (uint32_t *)rhs_utf->u_data[rhs_utf->u_width];
				rhs_len = WSTR_LENGTH(rhs_str);
				if (rhs_end > rhs_len)
					rhs_end = rhs_len;
				if (rhs_start >= rhs_end) {
					rhs_len = 0;
				} else {
					rhs_str += rhs_start;
					rhs_len = rhs_end - rhs_start;
				}
				common_len = MIN(lhs_len, rhs_len);
				for (i = 0; i < common_len; ++i) {
					if ((uint32_t)(lhs_str[i]) == rhs_str[i])
						continue;
					return (uint32_t)(lhs_str[i]) < rhs_str[i] ? -1 : 1;
				}
			}	break;

			default:
				__builtin_unreachable();
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
		if (rhs_start >= rhs_end) {
			rhs_len = 0;
		} else {
			rhs_str += rhs_start;
			rhs_len = rhs_end - rhs_start;
		}
		lhs_utf = lhs->s_data;
		switch (lhs_utf->u_width) {

		CASE_WIDTH_2BYTE: {
			uint16_t *lhs_str;
			size_t i, common_len;
			lhs_str = (uint16_t *)lhs_utf->u_data[lhs_utf->u_width];
			lhs_len = WSTR_LENGTH(lhs_str);
			if (lhs_end > lhs_len)
				lhs_end = lhs_len;
			if (lhs_start >= lhs_end) {
				lhs_len = 0;
			} else {
				lhs_str += lhs_start;
				lhs_len = lhs_end - lhs_start;
			}
			common_len = MIN(rhs_len, lhs_len);
			for (i = 0; i < common_len; ++i) {
				if (lhs_str[i] == (uint16_t)(rhs_str[i]))
					continue;
				return lhs_str[i] < (uint16_t)(rhs_str[i]) ? -1 : 1;
			}
		}	break;

		CASE_WIDTH_4BYTE: {
			uint32_t *lhs_str;
			size_t i, common_len;
			lhs_str = (uint32_t *)lhs_utf->u_data[lhs_utf->u_width];
			lhs_len = WSTR_LENGTH(lhs_str);
			if (lhs_end > lhs_len)
				lhs_end = lhs_len;
			if (lhs_start >= lhs_end) {
				lhs_len = 0;
			} else {
				lhs_str += lhs_start;
				lhs_len = lhs_end - lhs_start;
			}
			common_len = MIN(rhs_len, lhs_len);
			for (i = 0; i < common_len; ++i) {
				if (lhs_str[i] == (uint32_t)(rhs_str[i]))
					continue;
				return lhs_str[i] < (uint32_t)(rhs_str[i]) ? -1 : 1;
			}
		}	break;

		default:
			__builtin_unreachable();
		}
	} else {
		struct string_utf *lhs_utf;
		struct string_utf *rhs_utf;
		lhs_utf = lhs->s_data;
		rhs_utf = rhs->s_data;
		ASSERT(lhs_utf != NULL);
		ASSERT(rhs_utf != NULL);
		ASSERT(lhs_utf->u_width != STRING_WIDTH_1BYTE);
		ASSERT(rhs_utf->u_width != STRING_WIDTH_1BYTE);
		/* Complex string comparison. */
		switch (lhs_utf->u_width) {

		CASE_WIDTH_2BYTE: {
			uint16_t *lhs_str;
			lhs_str = (uint16_t *)lhs_utf->u_data[lhs_utf->u_width];
			lhs_len = WSTR_LENGTH(lhs_str);
			if (lhs_end > lhs_len)
				lhs_end = lhs_len;
			if (lhs_start >= lhs_end) {
				lhs_len = 0;
			} else {
				lhs_str += lhs_start;
				lhs_len = lhs_end - lhs_start;
			}
			switch (rhs_utf->u_width) {

			CASE_WIDTH_2BYTE: {
				int result;
				uint16_t *rhs_str;
				size_t common_len;
				rhs_str = (uint16_t *)rhs_utf->u_data[rhs_utf->u_width];
				rhs_len = WSTR_LENGTH(rhs_str);
				if (rhs_end > rhs_len)
					rhs_end = rhs_len;
				if (rhs_start >= rhs_end) {
					rhs_len = 0;
				} else {
					rhs_str += rhs_start;
					rhs_len = rhs_end - rhs_start;
				}
				common_len = MIN(lhs_len, rhs_len);
				result = memcmpw(lhs_str, rhs_str, common_len);
				if (result != 0)
					return result;
			}	break;

			CASE_WIDTH_4BYTE: {
				uint32_t *rhs_str;
				size_t i, common_len;
				rhs_str = (uint32_t *)rhs_utf->u_data[rhs_utf->u_width];
				rhs_len = WSTR_LENGTH(rhs_str);
				if (rhs_end > rhs_len)
					rhs_end = rhs_len;
				if (rhs_start >= rhs_end) {
					rhs_len = 0;
				} else {
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

			default:
				__builtin_unreachable();
			}
		}	break;

		CASE_WIDTH_4BYTE: {
			uint32_t *lhs_str;
			lhs_str = (uint32_t *)lhs_utf->u_data[lhs_utf->u_width];
			lhs_len = WSTR_LENGTH(lhs_str);
			if (lhs_end > lhs_len)
				lhs_end = lhs_len;
			if (lhs_start >= lhs_end) {
				lhs_len = 0;
			} else {
				lhs_str += lhs_start;
				lhs_len = lhs_end - lhs_start;
			}
			switch (rhs_utf->u_width) {

			CASE_WIDTH_2BYTE: {
				uint16_t *rhs_str;
				size_t i, common_len;
				rhs_str = (uint16_t *)rhs_utf->u_data[rhs_utf->u_width];
				rhs_len = WSTR_LENGTH(rhs_str);
				if (rhs_end > rhs_len)
					rhs_end = rhs_len;
				if (rhs_start >= rhs_end) {
					rhs_len = 0;
				} else {
					rhs_str += rhs_start;
					rhs_len = rhs_end - rhs_start;
				}
				common_len = MIN(lhs_len, rhs_len);
				for (i = 0; i < common_len; ++i) {
					if (lhs_str[i] == (uint32_t)(rhs_str[i]))
						continue;
					return lhs_str[i] < (uint32_t)(rhs_str[i]) ? -1 : 1;
				}
			}	break;

			CASE_WIDTH_4BYTE: {
				int result;
				uint32_t *rhs_str;
				size_t common_len;
				rhs_str = (uint32_t *)rhs_utf->u_data[rhs_utf->u_width];
				rhs_len = WSTR_LENGTH(rhs_str);
				if (rhs_end > rhs_len)
					rhs_end = rhs_len;
				if (rhs_start >= rhs_end) {
					rhs_len = 0;
				} else {
					rhs_str += rhs_start;
					rhs_len = rhs_end - rhs_start;
				}
				common_len = MIN(lhs_len, rhs_len);
				result = memcmpl(lhs_str, rhs_str, common_len);
				if (result != 0)
					return result;
			}	break;

			default:
				__builtin_unreachable();
			}
		}	break;

		default:
			__builtin_unreachable();
		}
	}

	/* If string contents are identical, leave off by comparing their lengths. */
	if (lhs_len == rhs_len)
		return 0;
	if (lhs_len < rhs_len)
		return -1;
	return 1;
}

/* NOTE: Always returns within [-1,1] */
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
		if (lhs_start >= lhs_end) {
			lhs_len = 0;
		} else {
			lhs_str += lhs_start;
			lhs_len = lhs_end - lhs_start;
		}
		if (!rhs->s_data ||
		    rhs->s_data->u_width == STRING_WIDTH_1BYTE) {
			rhs_len = rhs->s_len;
			if (rhs_end > rhs_len)
				rhs_end = rhs_len;
			if (rhs_start >= rhs_end) {
				rhs_len = 0;
			} else {
				rhs_len = rhs_end - rhs_start;
			}
			/* Most simple case: compare ascii/single-byte strings. */
			return dee_memcasecmpb(lhs_str, lhs_len,
			                       (uint8_t *)rhs->s_str + rhs_start, rhs_len);
		} else {
			struct string_utf *rhs_utf = rhs->s_data;
			switch (rhs_utf->u_width) {

			CASE_WIDTH_2BYTE: {
				uint16_t *rhs_str;
				rhs_str = (uint16_t *)rhs_utf->u_data[rhs_utf->u_width];
				rhs_len = WSTR_LENGTH(rhs_str);
				if (rhs_end > rhs_len)
					rhs_end = rhs_len;
				if (rhs_start >= rhs_end) {
					rhs_len = 0;
				} else {
					rhs_str += rhs_start;
					rhs_len = rhs_end - rhs_start;
				}
				DO_COMPARE(b, lhs_str, lhs_len,
				           w, rhs_str, rhs_len);
			}	break;

			CASE_WIDTH_4BYTE: {
				uint32_t *rhs_str;
				rhs_str = (uint32_t *)rhs_utf->u_data[rhs_utf->u_width];
				rhs_len = WSTR_LENGTH(rhs_str);
				if (rhs_end > rhs_len)
					rhs_end = rhs_len;
				if (rhs_start >= rhs_end) {
					rhs_len = 0;
				} else {
					rhs_str += rhs_start;
					rhs_len = rhs_end - rhs_start;
				}
				DO_COMPARE(b, lhs_str, lhs_len,
				           l, rhs_str, rhs_len);
			}	break;

			default:
				__builtin_unreachable();
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
		if (rhs_start >= rhs_end) {
			rhs_len = 0;
		} else {
			rhs_str += rhs_start;
			rhs_len = rhs_end - rhs_start;
		}
		lhs_utf = lhs->s_data;
		switch (lhs_utf->u_width) {

		CASE_WIDTH_2BYTE: {
			uint16_t *lhs_str;
			lhs_str = (uint16_t *)lhs_utf->u_data[lhs_utf->u_width];
			lhs_len = WSTR_LENGTH(lhs_str);
			if (lhs_end > lhs_len)
				lhs_end = lhs_len;
			if (lhs_start >= lhs_end) {
				lhs_len = 0;
			} else {
				lhs_str += lhs_start;
				lhs_len = lhs_end - lhs_start;
			}
			DO_COMPARE(w, lhs_str, lhs_len,
			           b, rhs_str, rhs_len);
		}	break;

		CASE_WIDTH_4BYTE: {
			uint32_t *lhs_str;
			lhs_str = (uint32_t *)lhs_utf->u_data[lhs_utf->u_width];
			lhs_len = WSTR_LENGTH(lhs_str);
			if (lhs_end > lhs_len)
				lhs_end = lhs_len;
			if (lhs_start >= lhs_end) {
				lhs_len = 0;
			} else {
				lhs_str += lhs_start;
				lhs_len = lhs_end - lhs_start;
			}
			DO_COMPARE(l, lhs_str, lhs_len,
			           b, rhs_str, rhs_len);
		}	break;

		default:
			__builtin_unreachable();
		}
	} else {
		struct string_utf *lhs_utf;
		struct string_utf *rhs_utf;
		lhs_utf = lhs->s_data;
		rhs_utf = rhs->s_data;
		ASSERT(lhs_utf != NULL);
		ASSERT(rhs_utf != NULL);
		ASSERT(lhs_utf->u_width != STRING_WIDTH_1BYTE);
		ASSERT(rhs_utf->u_width != STRING_WIDTH_1BYTE);
		/* Complex string comparison. */
		switch (lhs_utf->u_width) {

		CASE_WIDTH_2BYTE: {
			uint16_t *lhs_str;
			lhs_str = (uint16_t *)lhs_utf->u_data[lhs_utf->u_width];
			lhs_len = WSTR_LENGTH(lhs_str);
			if (lhs_end > lhs_len)
				lhs_end = lhs_len;
			if (lhs_start >= lhs_end) {
				lhs_len = 0;
			} else {
				lhs_str += lhs_start;
				lhs_len = lhs_end - lhs_start;
			}
			switch (rhs_utf->u_width) {

			CASE_WIDTH_2BYTE: {
				uint16_t *rhs_str;
				rhs_str = (uint16_t *)rhs_utf->u_data[rhs_utf->u_width];
				rhs_len = WSTR_LENGTH(rhs_str);
				if (rhs_end > rhs_len)
					rhs_end = rhs_len;
				if (rhs_start >= rhs_end) {
					rhs_len = 0;
				} else {
					rhs_str += rhs_start;
					rhs_len = rhs_end - rhs_start;
				}
				DO_COMPARE(w, lhs_str, lhs_len,
				           w, rhs_str, rhs_len);
			}	break;

			CASE_WIDTH_4BYTE: {
				uint32_t *rhs_str;
				rhs_str = (uint32_t *)rhs_utf->u_data[rhs_utf->u_width];
				rhs_len = WSTR_LENGTH(rhs_str);
				if (rhs_end > rhs_len)
					rhs_end = rhs_len;
				if (rhs_start >= rhs_end) {
					rhs_len = 0;
				} else {
					rhs_str += rhs_start;
					rhs_len = rhs_end - rhs_start;
				}
				DO_COMPARE(w, lhs_str, lhs_len,
				           l, rhs_str, rhs_len);
			}	break;

			default:
				__builtin_unreachable();
			}
		}	break;

		CASE_WIDTH_4BYTE: {
			uint32_t *lhs_str;

			lhs_str = (uint32_t *)lhs_utf->u_data[lhs_utf->u_width];
			lhs_len = WSTR_LENGTH(lhs_str);
			if (lhs_end > lhs_len)
				lhs_end = lhs_len;
			if (lhs_start >= lhs_end) {
				lhs_len = 0;
			} else {
				lhs_str += lhs_start;
				lhs_len = lhs_end - lhs_start;
			}
			switch (rhs_utf->u_width) {

			CASE_WIDTH_2BYTE: {
				uint16_t *rhs_str;
				rhs_str = (uint16_t *)rhs_utf->u_data[rhs_utf->u_width];
				rhs_len = WSTR_LENGTH(rhs_str);
				if (rhs_end > rhs_len)
					rhs_end = rhs_len;
				if (rhs_start >= rhs_end) {
					rhs_len = 0;
				} else {
					rhs_str += rhs_start;
					rhs_len = rhs_end - rhs_start;
				}
				DO_COMPARE(l, lhs_str, lhs_len,
				           w, rhs_str, rhs_len);
			}	break;

			CASE_WIDTH_4BYTE: {
				uint32_t *rhs_str;
				rhs_str = (uint32_t *)rhs_utf->u_data[rhs_utf->u_width];
				rhs_len = WSTR_LENGTH(rhs_str);
				if (rhs_end > rhs_len)
					rhs_end = rhs_len;
				if (rhs_start >= rhs_end) {
					rhs_len = 0;
				} else {
					rhs_str += rhs_start;
					rhs_len = rhs_end - rhs_start;
				}
				DO_COMPARE(l, lhs_str, lhs_len,
				           l, rhs_str, rhs_len);
			}	break;

			default:
				__builtin_unreachable();
			}
		}	break;

		default:
			__builtin_unreachable();
		}
	}
}
#undef DO_COMPARE



PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
string_compare(String *self, size_t argc, DeeObject *const *argv) {
	int result;
	struct compare_args args;
	if (get_compare_args(&args, argc, argv, "compare"))
		goto err;
	result = compare_strings_ex(self, args.my_start, args.my_end,
	                            args.other, args.ot_start, args.ot_end);
	return DeeInt_NewInt(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
string_casecompare(String *self, size_t argc, DeeObject *const *argv) {
	int result;
	struct compare_args args;
	if (get_compare_args(&args, argc, argv, "compare"))
		goto err;
	result = casecompare_strings_ex(self, args.my_start, args.my_end,
	                                args.other, args.ot_start, args.ot_end);
	return_reference(DeeInt_FromSign(result));
err:
	return NULL;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
string_vercompare(String *self, size_t argc, DeeObject *const *argv) {
	int result;
	union dcharptr my_str, ot_str;
	size_t my_len, ot_len;
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
		if (args.my_end <= args.my_start) {
			my_len = 0;
		} else {
			my_str.cp8 += args.my_start;
			my_len = args.my_end - args.my_start;
		}
		if (args.ot_end > ot_len)
			args.ot_end = ot_len;
		if (args.ot_end <= args.ot_start) {
			ot_len = 0;
		} else {
			ot_str.cp8 += args.ot_start;
			ot_len = args.ot_end - args.ot_start;
		}
		result = strverscmpb(my_str.cp8, my_len,
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
		if (args.my_end <= args.my_start) {
			my_len = 0;
		} else {
			my_str.cp16 += args.my_start;
			my_len = args.my_end - args.my_start;
		}
		if (args.ot_end > ot_len)
			args.ot_end = ot_len;
		if (args.ot_end <= args.ot_start) {
			ot_len = 0;
		} else {
			ot_str.cp16 += args.ot_start;
			ot_len = args.ot_end - args.ot_start;
		}
		result = strverscmpw(my_str.cp16, my_len,
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
		if (args.my_end <= args.my_start) {
			my_len = 0;
		} else {
			my_str.cp32 += args.my_start;
			my_len = args.my_end - args.my_start;
		}
		if (args.ot_end > ot_len)
			args.ot_end = ot_len;
		if (args.ot_end <= args.ot_start) {
			ot_len = 0;
		} else {
			ot_str.cp32 += args.ot_start;
			ot_len = args.ot_end - args.ot_start;
		}
		result = strverscmpl(my_str.cp32, my_len,
		                     ot_str.cp32, ot_len);
		break;
	}
	return_reference(DeeInt_FromSign(result));
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
string_casevercompare(String *self, size_t argc, DeeObject *const *argv) {
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
		if (args.my_end <= args.my_start) {
			my_len = 0;
		} else {
			my_str.cp8 += args.my_start;
			my_len = args.my_end - args.my_start;
		}
		if (args.ot_end > ot_len)
			args.ot_end = ot_len;
		if (args.ot_end <= args.ot_start) {
			ot_len = 0;
		} else {
			ot_str.cp8 += args.ot_start;
			ot_len = args.ot_end - args.ot_start;
		}
		result = strcaseverscmpb(my_str.cp8, my_len,
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
		if (args.my_end <= args.my_start) {
			my_len = 0;
		} else {
			my_str.cp16 += args.my_start;
			my_len = args.my_end - args.my_start;
		}
		if (args.ot_end > ot_len)
			args.ot_end = ot_len;
		if (args.ot_end <= args.ot_start) {
			ot_len = 0;
		} else {
			ot_str.cp16 += args.ot_start;
			ot_len = args.ot_end - args.ot_start;
		}
		result = strcaseverscmpw(my_str.cp16, my_len,
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
		if (args.my_end <= args.my_start) {
			my_len = 0;
		} else {
			my_str.cp32 += args.my_start;
			my_len = args.my_end - args.my_start;
		}
		if (args.ot_end > ot_len)
			args.ot_end = ot_len;
		if (args.ot_end <= args.ot_start) {
			ot_len = 0;
		} else {
			ot_str.cp32 += args.ot_start;
			ot_len = args.ot_end - args.ot_start;
		}
		result = strcaseverscmpl(my_str.cp32, my_len,
		                         ot_str.cp32, ot_len);
		break;
	}
	return DeeInt_NewInt32(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
string_fuzzycompare(String *self, size_t argc, DeeObject *const *argv) {
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
		if (args.my_end <= args.my_start) {
			my_len = 0;
		} else {
			my_str.cp8 += args.my_start;
			my_len = args.my_end - args.my_start;
		}
		if (args.ot_end > ot_len)
			args.ot_end = ot_len;
		if (args.ot_end <= args.ot_start) {
			ot_len = 0;
		} else {
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
		if (args.my_end <= args.my_start) {
			my_len = 0;
		} else {
			my_str.cp16 += args.my_start;
			my_len = args.my_end - args.my_start;
		}
		if (args.ot_end > ot_len)
			args.ot_end = ot_len;
		if (args.ot_end <= args.ot_start) {
			ot_len = 0;
		} else {
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
		if (args.my_end <= args.my_start) {
			my_len = 0;
		} else {
			my_str.cp32 += args.my_start;
			my_len = args.my_end - args.my_start;
		}
		if (args.ot_end > ot_len)
			args.ot_end = ot_len;
		if (args.ot_end <= args.ot_start) {
			ot_len = 0;
		} else {
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

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
string_casefuzzycompare(String *self, size_t argc, DeeObject *const *argv) {
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
		if (args.my_end <= args.my_start) {
			my_len = 0;
		} else {
			my_str.cp8 += args.my_start;
			my_len = args.my_end - args.my_start;
		}
		if (args.ot_end > ot_len)
			args.ot_end = ot_len;
		if (args.ot_end <= args.ot_start) {
			ot_len = 0;
		} else {
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
		if (args.my_end <= args.my_start) {
			my_len = 0;
		} else {
			my_str.cp16 += args.my_start;
			my_len = args.my_end - args.my_start;
		}
		if (args.ot_end > ot_len)
			args.ot_end = ot_len;
		if (args.ot_end <= args.ot_start) {
			ot_len = 0;
		} else {
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
		if (args.my_end <= args.my_start) {
			my_len = 0;
		} else {
			my_str.cp32 += args.my_start;
			my_len = args.my_end - args.my_start;
		}
		if (args.ot_end > ot_len)
			args.ot_end = ot_len;
		if (args.ot_end <= args.ot_start) {
			ot_len = 0;
		} else {
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

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
string_common(String *self, size_t argc, DeeObject *const *argv) {
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
		if (args.my_end <= args.my_start) {
			my_len = 0;
		} else {
			my_str.cp8 += args.my_start;
			my_len = args.my_end - args.my_start;
		}
		if (args.ot_end > ot_len)
			args.ot_end = ot_len;
		if (args.ot_end <= args.ot_start) {
			ot_len = 0;
		} else {
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
		if (args.my_end <= args.my_start) {
			my_len = 0;
		} else {
			my_str.cp16 += args.my_start;
			my_len = args.my_end - args.my_start;
		}
		if (args.ot_end > ot_len)
			args.ot_end = ot_len;
		if (args.ot_end <= args.ot_start) {
			ot_len = 0;
		} else {
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
		if (args.my_end <= args.my_start) {
			my_len = 0;
		} else {
			my_str.cp32 += args.my_start;
			my_len = args.my_end - args.my_start;
		}
		if (args.ot_end > ot_len)
			args.ot_end = ot_len;
		if (args.ot_end <= args.ot_start) {
			ot_len = 0;
		} else {
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

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
string_rcommon(String *self, size_t argc, DeeObject *const *argv) {
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
		if (args.my_end <= args.my_start) {
			my_len = 0;
		} else {
			my_str.cp8 += args.my_start;
			my_len = args.my_end - args.my_start;
		}
		if (args.ot_end > ot_len)
			args.ot_end = ot_len;
		if (args.ot_end <= args.ot_start) {
			ot_len = 0;
		} else {
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
		if (args.my_end <= args.my_start) {
			my_len = 0;
		} else {
			my_str.cp16 += args.my_start;
			my_len = args.my_end - args.my_start;
		}
		if (args.ot_end > ot_len)
			args.ot_end = ot_len;
		if (args.ot_end <= args.ot_start) {
			ot_len = 0;
		} else {
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
		if (args.my_end <= args.my_start) {
			my_len = 0;
		} else {
			my_str.cp32 += args.my_start;
			my_len = args.my_end - args.my_start;
		}
		if (args.ot_end > ot_len)
			args.ot_end = ot_len;
		if (args.ot_end <= args.ot_start) {
			ot_len = 0;
		} else {
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

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
string_casecommon(String *self, size_t argc, DeeObject *const *argv) {
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
		if (args.my_end <= args.my_start) {
			my_len = 0;
		} else {
			my_str.cp8 += args.my_start;
			my_len = args.my_end - args.my_start;
		}
		if (args.ot_end > ot_len)
			args.ot_end = ot_len;
		if (args.ot_end <= args.ot_start) {
			ot_len = 0;
		} else {
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
		if (args.my_end <= args.my_start) {
			my_len = 0;
		} else {
			my_str.cp16 += args.my_start;
			my_len = args.my_end - args.my_start;
		}
		if (args.ot_end > ot_len)
			args.ot_end = ot_len;
		if (args.ot_end <= args.ot_start) {
			ot_len = 0;
		} else {
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
		if (args.my_end <= args.my_start) {
			my_len = 0;
		} else {
			my_str.cp32 += args.my_start;
			my_len = args.my_end - args.my_start;
		}
		if (args.ot_end > ot_len)
			args.ot_end = ot_len;
		if (args.ot_end <= args.ot_start) {
			ot_len = 0;
		} else {
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

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
string_casercommon(String *self, size_t argc, DeeObject *const *argv) {
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
		if (args.my_end <= args.my_start) {
			my_len = 0;
		} else {
			my_str.cp8 += args.my_start;
			my_len = args.my_end - args.my_start;
		}
		if (args.ot_end > ot_len)
			args.ot_end = ot_len;
		if (args.ot_end <= args.ot_start) {
			ot_len = 0;
		} else {
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
		if (args.my_end <= args.my_start) {
			my_len = 0;
		} else {
			my_str.cp16 += args.my_start;
			my_len = args.my_end - args.my_start;
		}
		if (args.ot_end > ot_len)
			args.ot_end = ot_len;
		if (args.ot_end <= args.ot_start) {
			ot_len = 0;
		} else {
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
		if (args.my_end <= args.my_start) {
			my_len = 0;
		} else {
			my_str.cp32 += args.my_start;
			my_len = args.my_end - args.my_start;
		}
		if (args.ot_end > ot_len)
			args.ot_end = ot_len;
		if (args.ot_end <= args.ot_start) {
			ot_len = 0;
		} else {
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


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
string_wildcompare(String *self, size_t argc, DeeObject *const *argv) {
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
		if (args.my_end <= args.my_start) {
			my_len = 0;
		} else {
			my_str.cp8 += args.my_start;
			my_len = args.my_end - args.my_start;
		}
		if (args.ot_end > ot_len)
			args.ot_end = ot_len;
		if (args.ot_end <= args.ot_start) {
			ot_len = 0;
		} else {
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
		if (args.my_end <= args.my_start) {
			my_len = 0;
		} else {
			my_str.cp16 += args.my_start;
			my_len = args.my_end - args.my_start;
		}
		if (args.ot_end > ot_len)
			args.ot_end = ot_len;
		if (args.ot_end <= args.ot_start) {
			ot_len = 0;
		} else {
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
		if (args.my_end <= args.my_start) {
			my_len = 0;
		} else {
			my_str.cp32 += args.my_start;
			my_len = args.my_end - args.my_start;
		}
		if (args.ot_end > ot_len)
			args.ot_end = ot_len;
		if (args.ot_end <= args.ot_start) {
			ot_len = 0;
		} else {
			ot_str.cp32 += args.ot_start;
			ot_len = args.ot_end - args.ot_start;
		}
		result = wildcomparel(my_str.cp32, my_len,
		                      ot_str.cp32, ot_len);
		break;
	}
	return DeeInt_NewInt64(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
string_wmatch(String *self, size_t argc, DeeObject *const *argv) {
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
		if (args.my_end <= args.my_start) {
			my_len = 0;
		} else {
			my_str.cp8 += args.my_start;
			my_len = args.my_end - args.my_start;
		}
		if (args.ot_end > ot_len)
			args.ot_end = ot_len;
		if (args.ot_end <= args.ot_start) {
			ot_len = 0;
		} else {
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
		if (args.my_end <= args.my_start) {
			my_len = 0;
		} else {
			my_str.cp16 += args.my_start;
			my_len = args.my_end - args.my_start;
		}
		if (args.ot_end > ot_len)
			args.ot_end = ot_len;
		if (args.ot_end <= args.ot_start) {
			ot_len = 0;
		} else {
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
		if (args.my_end <= args.my_start) {
			my_len = 0;
		} else {
			my_str.cp32 += args.my_start;
			my_len = args.my_end - args.my_start;
		}
		if (args.ot_end > ot_len)
			args.ot_end = ot_len;
		if (args.ot_end <= args.ot_start) {
			ot_len = 0;
		} else {
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

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
string_casewildcompare(String *self, size_t argc, DeeObject *const *argv) {
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
		if (args.my_end <= args.my_start) {
			my_len = 0;
		} else {
			my_str.cp8 += args.my_start;
			my_len = args.my_end - args.my_start;
		}
		if (args.ot_end > ot_len)
			args.ot_end = ot_len;
		if (args.ot_end <= args.ot_start) {
			ot_len = 0;
		} else {
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
		if (args.my_end <= args.my_start) {
			my_len = 0;
		} else {
			my_str.cp16 += args.my_start;
			my_len = args.my_end - args.my_start;
		}
		if (args.ot_end > ot_len)
			args.ot_end = ot_len;
		if (args.ot_end <= args.ot_start) {
			ot_len = 0;
		} else {
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
		if (args.my_end <= args.my_start) {
			my_len = 0;
		} else {
			my_str.cp32 += args.my_start;
			my_len = args.my_end - args.my_start;
		}
		if (args.ot_end > ot_len)
			args.ot_end = ot_len;
		if (args.ot_end <= args.ot_start) {
			ot_len = 0;
		} else {
			ot_str.cp32 += args.ot_start;
			ot_len = args.ot_end - args.ot_start;
		}
		unicode_foldreader_init(my_reader.l, my_str.cp32, my_len);
		unicode_foldreader_init(ot_reader.l, ot_str.cp32, ot_len);
		result = wildcasecomparel(&my_reader.l, &ot_reader.l);
		break;
	}
	return DeeInt_NewInt64(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
string_casewmatch(String *self, size_t argc, DeeObject *const *argv) {
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
		if (args.my_end <= args.my_start) {
			my_len = 0;
		} else {
			my_str.cp8 += args.my_start;
			my_len = args.my_end - args.my_start;
		}
		if (args.ot_end > ot_len)
			args.ot_end = ot_len;
		if (args.ot_end <= args.ot_start) {
			ot_len = 0;
		} else {
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
		if (args.my_end <= args.my_start) {
			my_len = 0;
		} else {
			my_str.cp16 += args.my_start;
			my_len = args.my_end - args.my_start;
		}
		if (args.ot_end > ot_len)
			args.ot_end = ot_len;
		if (args.ot_end <= args.ot_start) {
			ot_len = 0;
		} else {
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
		if (args.my_end <= args.my_start) {
			my_len = 0;
		} else {
			my_str.cp32 += args.my_start;
			my_len = args.my_end - args.my_start;
		}
		if (args.ot_end > ot_len)
			args.ot_end = ot_len;
		if (args.ot_end <= args.ot_start) {
			ot_len = 0;
		} else {
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

INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeString_Split(String *self, String *separator);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeString_CaseSplit(String *self, String *separator);

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
string_split(String *self, size_t argc, DeeObject *const *argv) {
	String *other;
	if (DeeArg_Unpack(argc, argv, "o:split", &other))
		goto err;
	if (DeeObject_AssertTypeExact(other, &DeeString_Type))
		goto err;
	return DeeString_Split(self, other);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
string_casesplit(String *self, size_t argc, DeeObject *const *argv) {
	String *other;
	if (DeeArg_Unpack(argc, argv, "o:casesplit", &other))
		goto err;
	if (DeeObject_AssertTypeExact(other, &DeeString_Type))
		goto err;
	return DeeString_CaseSplit(self, other);
err:
	return NULL;
}

INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeString_SplitLines(DeeObject *__restrict self, bool keepends);

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
string_splitlines(String *self, size_t argc, DeeObject *const *argv) {
	bool keepends = false;
	if (DeeArg_Unpack(argc, argv, "|b:splitlines", &keepends))
		goto err;
	return DeeString_SplitLines((DeeObject *)self, keepends);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF String *DCALL
string_indent(String *self, size_t argc, DeeObject *const *argv) {
	String *filler = &str_tab;
	if (DeeArg_Unpack(argc, argv, "|o:indent", &filler))
		goto err;
	if (DeeObject_AssertTypeExact(filler, &DeeString_Type))
		goto err;
	return DeeString_Indent(self, filler);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF String *DCALL
string_dedent(String *self, size_t argc, DeeObject *const *argv) {
	size_t max_chars = 1;
	String *mask  = NULL;
	if (DeeArg_Unpack(argc, argv, "|" UNPuSIZ "o:dedent", &max_chars, &mask))
		goto err;
	if (!mask)
		return DeeString_DedentSpc(self, max_chars);
	if (DeeObject_AssertTypeExact(mask, &DeeString_Type))
		goto err;
	return DeeString_Dedent(self, max_chars, mask);
err:
	return NULL;
}


INTDEF WUNUSED NONNULL((1, 3, 5)) dssize_t DCALL
DeeString_Format(dformatprinter printer, void *arg,
                 /*utf-8*/ char const *__restrict format,
                 size_t format_len, DeeObject *__restrict args);

INTERN WUNUSED NONNULL((1)) DREF String *DCALL
string_format(String *self, size_t argc, DeeObject *const *argv) {
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
		return (DREF String *)unicode_printer_pack(&printer);
err_printer:
		unicode_printer_fini(&printer);
	}
err:
	return NULL;
}

INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeString_Scanf(DeeObject *self, DeeObject *format);

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
string_scanf(String *self, size_t argc, DeeObject *const *argv) {
	DeeObject *format;
	if (DeeArg_Unpack(argc, argv, "o:scanf", &format))
		goto err;
	if (DeeObject_AssertTypeExact(format, &DeeString_Type))
		goto err;
	return DeeString_Scanf((DeeObject *)self, format);
err:
	return NULL;
}



PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
string_findmatch(String *self, size_t argc, DeeObject *const *argv) {
	String *s_open, *s_clos;
	size_t start = 0, end = (size_t)-1;
	union dcharptr scan_str, open_str, clos_str, ptr;
	size_t scan_len, open_len, clos_len;
	size_t result;
	if (DeeArg_Unpack(argc, argv, "oo|" UNPdSIZ UNPdSIZ ":findmatch", &s_open, &s_clos, &start, &end))
		goto err;
	if (DeeObject_AssertTypeExact(s_open, &DeeString_Type))
		goto err;
	if (DeeObject_AssertTypeExact(s_clos, &DeeString_Type))
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
	return_reference_(DeeInt_MinusOne);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
string_indexmatch(String *self, size_t argc, DeeObject *const *argv) {
	String *s_open, *s_clos;
	size_t start = 0, end = (size_t)-1;
	union dcharptr scan_str, open_str, clos_str, ptr;
	size_t scan_len, open_len, clos_len;
	size_t result;
	if (DeeArg_Unpack(argc, argv, "oo|" UNPdSIZ UNPdSIZ ":findmatch", &s_open, &s_clos, &start, &end))
		goto err;
	if (DeeObject_AssertTypeExact(s_open, &DeeString_Type))
		goto err;
	if (DeeObject_AssertTypeExact(s_clos, &DeeString_Type))
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


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
string_casefindmatch(String *self, size_t argc, DeeObject *const *argv) {
	String *s_open, *s_clos;
	size_t start = 0, end = (size_t)-1;
	union dcharptr scan_str, open_str, clos_str, ptr;
	size_t scan_len, open_len, clos_len, match_length;
	size_t result;
	if (DeeArg_Unpack(argc, argv, "oo|" UNPdSIZ UNPdSIZ ":casefindmatch", &s_open, &s_clos, &start, &end))
		goto err;
	if (DeeObject_AssertTypeExact(s_open, &DeeString_Type))
		goto err;
	if (DeeObject_AssertTypeExact(s_clos, &DeeString_Type))
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
	return DeeTuple_NewII(result, result + match_length);
err_not_found:
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
string_caseindexmatch(String *self, size_t argc, DeeObject *const *argv) {
	String *s_open, *s_clos;
	size_t start = 0, end = (size_t)-1;
	union dcharptr scan_str, open_str, clos_str, ptr;
	size_t scan_len, open_len, clos_len, match_length;
	size_t result;
	if (DeeArg_Unpack(argc, argv, "oo|" UNPdSIZ UNPdSIZ ":caseindexmatch", &s_open, &s_clos, &start, &end))
		goto err;
	if (DeeObject_AssertTypeExact(s_open, &DeeString_Type))
		goto err;
	if (DeeObject_AssertTypeExact(s_clos, &DeeString_Type))
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
	return DeeTuple_NewII(result, result + match_length);
err_not_found:
	err_index_not_found((DeeObject *)self,
	                    (DeeObject *)s_clos);
err:
	return NULL;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
string_rfindmatch(String *self, size_t argc, DeeObject *const *argv) {
	String *s_open, *s_clos;
	size_t start = 0, end = (size_t)-1;
	union dcharptr scan_str, open_str, clos_str, ptr;
	size_t scan_len, open_len, clos_len;
	size_t result;
	if (DeeArg_Unpack(argc, argv, "oo|" UNPdSIZ UNPdSIZ ":rfindmatch", &s_open, &s_clos, &start, &end))
		goto err;
	if (DeeObject_AssertTypeExact(s_open, &DeeString_Type))
		goto err;
	if (DeeObject_AssertTypeExact(s_clos, &DeeString_Type))
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
	return_reference_(DeeInt_MinusOne);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
string_rindexmatch(String *self, size_t argc, DeeObject *const *argv) {
	String *s_open, *s_clos;
	size_t start = 0, end = (size_t)-1;
	union dcharptr scan_str, open_str, clos_str, ptr;
	size_t scan_len, open_len, clos_len;
	size_t result;
	if (DeeArg_Unpack(argc, argv, "oo|" UNPdSIZ UNPdSIZ ":rindexmatch", &s_open, &s_clos, &start, &end))
		goto err;
	if (DeeObject_AssertTypeExact(s_open, &DeeString_Type))
		goto err;
	if (DeeObject_AssertTypeExact(s_clos, &DeeString_Type))
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


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
string_caserfindmatch(String *self, size_t argc, DeeObject *const *argv) {
	String *s_open, *s_clos;
	size_t start = 0, end = (size_t)-1;
	union dcharptr scan_str, open_str, clos_str, ptr;
	size_t scan_len, open_len, clos_len, match_length;
	size_t result;
	if (DeeArg_Unpack(argc, argv, "oo|" UNPdSIZ UNPdSIZ ":caserfindmatch", &s_open, &s_clos, &start, &end))
		goto err;
	if (DeeObject_AssertTypeExact(s_open, &DeeString_Type))
		goto err;
	if (DeeObject_AssertTypeExact(s_clos, &DeeString_Type))
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
	return DeeTuple_NewII(result, result + match_length);
err_not_found:
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
string_caserindexmatch(String *self, size_t argc, DeeObject *const *argv) {
	String *s_open, *s_clos;
	size_t start = 0, end = (size_t)-1;
	union dcharptr scan_str, open_str, clos_str, ptr;
	size_t scan_len, open_len, clos_len, match_length;
	size_t result;
	if (DeeArg_Unpack(argc, argv, "oo|" UNPdSIZ UNPdSIZ ":caserindexmatch", &s_open, &s_clos, &start, &end))
		goto err;
	if (DeeObject_AssertTypeExact(s_open, &DeeString_Type))
		goto err;
	if (DeeObject_AssertTypeExact(s_clos, &DeeString_Type))
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
	return DeeTuple_NewII(result, result + match_length);
err_not_found:
	err_index_not_found((DeeObject *)self,
	                    (DeeObject *)s_open);
err:
	return NULL;
}



PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
string_partitionmatch(String *self, size_t argc, DeeObject *const *argv) {
	String *s_open, *s_clos;
	size_t start = 0, end = (size_t)-1;
	union dcharptr scan_str, open_str, clos_str, match_start, match_end;
	size_t scan_len, open_len, clos_len;
	DREF DeeTupleObject *result;
	if (DeeArg_Unpack(argc, argv, "oo|" UNPdSIZ UNPdSIZ ":partitionmatch", &s_open, &s_clos, &start, &end))
		goto err;
	if (DeeObject_AssertTypeExact(s_open, &DeeString_Type))
		goto err;
	if (DeeObject_AssertTypeExact(s_clos, &DeeString_Type))
		goto err;
#define SET_STRING(a, b, c)                                      \
	do {                                                         \
		if ((result->t_elem[0] = (DREF DeeObject *)(a)) == NULL) \
			goto err_r_0;                                        \
		if ((result->t_elem[1] = (DREF DeeObject *)(b)) == NULL) \
			goto err_r_1;                                        \
		if ((result->t_elem[2] = (DREF DeeObject *)(c)) == NULL) \
			goto err_r_2;                                        \
	}	__WHILE0
	result = DeeTuple_NewUninitialized(3);
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
		match_end.cp8 = find_matchb(match_start.cp8 + open_len,
		                            scan_len - (match_start.cp8 - (scan_str.cp8 + start)),
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
		match_end.cp16 = find_matchw(match_start.cp16 + open_len,
		                             scan_len - (match_start.cp16 - (scan_str.cp16 + start)),
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
		match_end.cp32 = find_matchl(match_start.cp32 + open_len,
		                             scan_len - (match_start.cp32 - (scan_str.cp32 + start)),
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
	DeeTuple_FreeUninitialized(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
string_rpartitionmatch(String *self, size_t argc, DeeObject *const *argv) {
	String *s_open, *s_clos;
	size_t start = 0, end = (size_t)-1;
	union dcharptr scan_str, open_str, clos_str, match_start, match_end;
	size_t scan_len, open_len, clos_len;
	DREF DeeTupleObject *result;
	if (DeeArg_Unpack(argc, argv, "oo|" UNPdSIZ UNPdSIZ ":rpartitionmatch", &s_open, &s_clos, &start, &end))
		goto err;
	if (DeeObject_AssertTypeExact(s_open, &DeeString_Type))
		goto err;
	if (DeeObject_AssertTypeExact(s_clos, &DeeString_Type))
		goto err;
#define SET_STRING(a, b, c)                                      \
	do {                                                         \
		if ((result->t_elem[0] = (DREF DeeObject *)(a)) == NULL) \
			goto err_r_0;                                        \
		if ((result->t_elem[1] = (DREF DeeObject *)(b)) == NULL) \
			goto err_r_1;                                        \
		if ((result->t_elem[2] = (DREF DeeObject *)(c)) == NULL) \
			goto err_r_2;                                        \
	}	__WHILE0
	result = DeeTuple_NewUninitialized(3);
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
	DeeTuple_FreeUninitialized(result);
err:
	return NULL;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
string_casepartitionmatch(String *self, size_t argc, DeeObject *const *argv) {
	String *s_open, *s_clos;
	size_t start = 0, end = (size_t)-1;
	union dcharptr scan_str, open_str, clos_str, match_start, match_end;
	size_t scan_len, open_len, clos_len, match_length;
	DREF DeeTupleObject *result;
	if (DeeArg_Unpack(argc, argv, "oo|" UNPdSIZ UNPdSIZ ":casepartitionmatch", &s_open, &s_clos, &start, &end))
		goto err;
	if (DeeObject_AssertTypeExact(s_open, &DeeString_Type))
		goto err;
	if (DeeObject_AssertTypeExact(s_clos, &DeeString_Type))
		goto err;
#define SET_STRING(a, b, c)                                      \
	do {                                                         \
		if ((result->t_elem[0] = (DREF DeeObject *)(a)) == NULL) \
			goto err_r_0;                                        \
		if ((result->t_elem[1] = (DREF DeeObject *)(b)) == NULL) \
			goto err_r_1;                                        \
		if ((result->t_elem[2] = (DREF DeeObject *)(c)) == NULL) \
			goto err_r_2;                                        \
	}	__WHILE0
	result = DeeTuple_NewUninitialized(3);
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
		match_end.cp8 = find_casematchb(match_start.cp8 + match_length,
		                                scan_len - (match_start.cp8 - (scan_str.cp8 + start)),
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
		match_end.cp16 = find_casematchw(match_start.cp16 + match_length,
		                                 scan_len - (match_start.cp16 - (scan_str.cp16 + start)),
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
		match_end.cp32 = find_casematchl(match_start.cp32 + match_length,
		                                 scan_len - (match_start.cp32 - (scan_str.cp32 + start)),
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
	DeeTuple_FreeUninitialized(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
string_caserpartitionmatch(String *self, size_t argc, DeeObject *const *argv) {
	String *s_open, *s_clos;
	size_t start = 0, end = (size_t)-1;
	union dcharptr scan_str, open_str, clos_str, match_start, match_end;
	size_t scan_len, open_len, clos_len;
	DREF DeeTupleObject *result;
	if (DeeArg_Unpack(argc, argv, "oo|" UNPdSIZ UNPdSIZ ":caserpartitionmatch", &s_open, &s_clos, &start, &end))
		goto err;
	if (DeeObject_AssertTypeExact(s_open, &DeeString_Type))
		goto err;
	if (DeeObject_AssertTypeExact(s_clos, &DeeString_Type))
		goto err;
#define SET_STRING(a, b, c)                                      \
	do {                                                         \
		if ((result->t_elem[0] = (DREF DeeObject *)(a)) == NULL) \
			goto err_r_0;                                        \
		if ((result->t_elem[1] = (DREF DeeObject *)(b)) == NULL) \
			goto err_r_1;                                        \
		if ((result->t_elem[2] = (DREF DeeObject *)(c)) == NULL) \
			goto err_r_2;                                        \
	}	__WHILE0
	result = DeeTuple_NewUninitialized(3);
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
	DeeTuple_FreeUninitialized(result);
err:
	return NULL;
}


/* Convert a character-offset into a byte-offset within `self's utf-8 repr */
PRIVATE WUNUSED NONNULL((1, 3)) size_t DCALL
string_charcnt2bytecnt(String const *self, size_t charpos, char const *utf8) {
	char const *iter;
	size_t num_bytes;
	if (!self->s_data || (self->s_data->u_flags & STRING_UTF_FASCII))
		return charpos;
	iter      = utf8;
	num_bytes = WSTR_LENGTH(utf8);
	for (; charpos; --charpos) {
		uint8_t charlen = unicode_utf8seqlen[(unsigned char)*iter];
		if unlikely(charlen > num_bytes) {
			iter += num_bytes;
			break;
		}
		iter += charlen;
		num_bytes -= charlen;
	}
	return (size_t)(iter - utf8);
}

/* Convert a character-offset into a byte-offset within `self's utf-8 repr */
PRIVATE WUNUSED NONNULL((1, 3)) size_t DCALL
string_bytecnt2charcnt(String const *self, size_t bytepos, char const *utf8) {
	size_t charpos;
	if (!self->s_data || (self->s_data->u_flags & STRING_UTF_FASCII))
		return bytepos;
	ASSERT(bytepos <= WSTR_LENGTH(utf8));
	charpos = 0;
	while (bytepos) {
		uint8_t charlen = unicode_utf8seqlen[(unsigned char)*utf8];
		if unlikely(charlen > bytepos)
			break;
		utf8 += charlen;
		bytepos -= charlen;
		++charpos;
	}
	return charpos;
}

PRIVATE NONNULL((1, 2, 3)) void DCALL
string_bytecnt2charcnt_v(String const *self, char const *utf8,
                         struct DeeRegexMatch *__restrict pmatch,
                         size_t nmatch) {
	size_t i;
	for (i = 0; i < nmatch; ++i) {
		size_t eo;
		size_t so = pmatch[i].rm_so;
		/* Check if this group was ever matched. */
		if (so == (size_t)-1)
			continue;
		eo = pmatch[i].rm_eo - so;                        /* eo = MATCH_SIZE_IN_BYTES */
		eo = string_bytecnt2charcnt(self, eo, utf8 + so); /* eo = MATCH_SIZE_IN_CHARS */
		so = string_bytecnt2charcnt(self, so, utf8);      /* so = MATCH_START_IN_CHARS */
		eo += so;                                         /* eo = MATCH_END_IN_CHARS */
		pmatch[i].rm_so = so;
		pmatch[i].rm_eo = eo;
	}
}

#define GENERIC_REGEX_GETARGS_FMT(name) "o|" UNPdSIZ UNPdSIZ "o:" name
PRIVATE struct keyword generic_regex_kwlist[] = { K(pattern), K(start), K(end), K(rules), KEND };
PRIVATE WUNUSED NONNULL((1, 5, 6)) int DCALL
generic_regex_getargs(String *self, size_t argc, DeeObject *const *argv,
                      DeeObject *kw, char const *__restrict fmt,
                      struct DeeRegexExec *__restrict result) {
	DeeObject *pattern, *rules = NULL;
	result->rx_startoff = 0;
	result->rx_endoff   = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, generic_regex_kwlist, fmt,
	                    &pattern, &result->rx_startoff, &result->rx_endoff,
	                    &rules))
		goto err;
	if (DeeObject_AssertTypeExact(pattern, &DeeString_Type))
		goto err;
	result->rx_code = DeeString_GetRegex(pattern, DEE_REGEX_COMPILE_NORMAL, rules);
	if unlikely(!result->rx_code)
		goto err;
	result->rx_nmatch = 0;
	result->rx_pmatch = NULL;
	result->rx_eflags = 0; /* TODO: NOTBOL/NOTEOL */
	result->rx_inbase = DeeString_AsUtf8((DeeObject *)self);
	if unlikely(!result->rx_inbase)
		goto err;
	result->rx_insize = WSTR_LENGTH(result->rx_inbase);
	if (result->rx_endoff >= result->rx_insize) {
		result->rx_endoff = result->rx_insize;
	} else {
		result->rx_endoff = string_charcnt2bytecnt(self, result->rx_endoff, (char const *)result->rx_inbase);
	}
	if (result->rx_startoff > 0)
		result->rx_startoff = string_charcnt2bytecnt(self, result->rx_startoff, (char const *)result->rx_inbase);
	return 0;
err:
	return -1;
}


struct DeeRegexExecWithRange {
	struct DeeRegexExec rewr_exec;  /* Normal exec args */
	size_t              rewr_range; /* Max # of search attempts to perform (in bytes) */
};

#define SEARCH_REGEX_GETARGS_FMT(name) "o|" UNPdSIZ UNPdSIZ UNPdSIZ "o:" name
PRIVATE struct keyword search_regex_kwlist[] = { K(pattern), K(start), K(end), K(range), K(rules), KEND };
PRIVATE WUNUSED NONNULL((1, 5, 6)) int DCALL
search_regex_getargs(String *self, size_t argc, DeeObject *const *argv,
                     DeeObject *kw, char const *__restrict fmt,
                     struct DeeRegexExecWithRange *__restrict result) {
	DeeObject *pattern, *rules = NULL;
	result->rewr_exec.rx_startoff = 0;
	result->rewr_exec.rx_endoff   = (size_t)-1;
	result->rewr_range            = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, search_regex_kwlist, fmt,
	                    &pattern, &result->rewr_exec.rx_startoff,
	                    &result->rewr_exec.rx_endoff,
	                    &result->rewr_range,
	                    &rules))
		goto err;
	if (DeeObject_AssertTypeExact(pattern, &DeeString_Type))
		goto err;
	result->rewr_exec.rx_code = DeeString_GetRegex(pattern, DEE_REGEX_COMPILE_NORMAL, rules);
	if unlikely(!result->rewr_exec.rx_code)
		goto err;
	result->rewr_exec.rx_nmatch = 0;
	result->rewr_exec.rx_pmatch = NULL;
	result->rewr_exec.rx_eflags = 0; /* TODO: NOTBOL/NOTEOL */
	result->rewr_exec.rx_inbase = DeeString_AsUtf8((DeeObject *)self);
	if unlikely(!result->rewr_exec.rx_inbase)
		goto err;
	result->rewr_exec.rx_insize = WSTR_LENGTH(result->rewr_exec.rx_inbase);
	if (result->rewr_exec.rx_endoff >= result->rewr_exec.rx_insize) {
		result->rewr_exec.rx_endoff = result->rewr_exec.rx_insize;
	} else {
		result->rewr_exec.rx_endoff = string_charcnt2bytecnt(self, result->rewr_exec.rx_endoff,
		                                                     (char const *)result->rewr_exec.rx_inbase);
	}
	if (result->rewr_range != (size_t)-1) {
		if (OVERFLOW_UADD(result->rewr_range, result->rewr_exec.rx_startoff, &result->rewr_range)) {
			result->rewr_range = (size_t)-1;
			goto convert_startoff;
		}
		result->rewr_range = string_charcnt2bytecnt(self, result->rewr_range,
		                                            (char const *)result->rewr_exec.rx_inbase);
		if (result->rewr_exec.rx_startoff > 0) {
			result->rewr_exec.rx_startoff = string_charcnt2bytecnt(self, result->rewr_exec.rx_startoff,
			                                                       (char const *)result->rewr_exec.rx_inbase);
			result->rewr_range -= result->rewr_exec.rx_startoff;
		}
	} else {
convert_startoff:
		if (result->rewr_exec.rx_startoff > 0) {
			result->rewr_exec.rx_startoff = string_charcnt2bytecnt(self, result->rewr_exec.rx_startoff,
			                                                       (char const *)result->rewr_exec.rx_inbase);
		}
	}
	return 0;
err:
	return -1;
}



PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
string_rematch(String *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	Dee_ssize_t result;
	struct DeeRegexExec exec;
	if unlikely(generic_regex_getargs(self, argc, argv, kw, GENERIC_REGEX_GETARGS_FMT("rematch"), &exec))
		goto err;
	result = DeeRegex_Match(&exec);
	if unlikely(result == DEE_RE_STATUS_ERROR)
		goto err;
	if (result == DEE_RE_STATUS_NOMATCH)
		return_none;
	result = (Dee_ssize_t)string_bytecnt2charcnt(self, (size_t)result, (char const *)exec.rx_inbase);
	return DeeInt_NewSize((size_t)result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
string_regmatch(String *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DREF ReGroups *groups;
	Dee_ssize_t result;
	struct DeeRegexExec exec;
	if unlikely(generic_regex_getargs(self, argc, argv, kw, GENERIC_REGEX_GETARGS_FMT("regmatch"), &exec))
		goto err;
	groups = ReGroups_Malloc(1 + exec.rx_code->rc_ngrps);
	if unlikely(!groups)
		goto err;
	exec.rx_nmatch = exec.rx_code->rc_ngrps;
	exec.rx_pmatch = groups->rg_groups + 1;
	result = DeeRegex_Match(&exec);
	if unlikely(result == DEE_RE_STATUS_ERROR)
		goto err_g;
	if (result == DEE_RE_STATUS_NOMATCH) {
		ReGroups_Free(groups);
		return_empty_seq;
	}
	string_bytecnt2charcnt_v(self, (char const *)exec.rx_inbase,
	                         groups->rg_groups + 1,
	                         exec.rx_code->rc_ngrps);
	groups->rg_groups[0].rm_so = 0;
	groups->rg_groups[0].rm_eo = string_bytecnt2charcnt(self, (size_t)result, (char const *)exec.rx_inbase);
	ReGroups_Init(groups, 1 + exec.rx_code->rc_ngrps);
	return (DREF DeeObject *)groups;
err_g:
	ReGroups_Free(groups);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
string_rematches(String *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	Dee_ssize_t result;
	struct DeeRegexExec exec;
	if unlikely(generic_regex_getargs(self, argc, argv, kw, GENERIC_REGEX_GETARGS_FMT("rematches"), &exec))
		goto err;
	result = DeeRegex_Match(&exec);
	if unlikely(result == DEE_RE_STATUS_ERROR)
		goto err;
	if (result == DEE_RE_STATUS_NOMATCH)
		return_false;
	ASSERT((size_t)result <= exec.rx_endoff);
	return_bool((size_t)result >= exec.rx_endoff);
err:
	return NULL;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
string_refind(String *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	Dee_ssize_t result;
	size_t match_size;
	struct DeeRegexExecWithRange exec;
	if unlikely(search_regex_getargs(self, argc, argv, kw, SEARCH_REGEX_GETARGS_FMT("refind"), &exec))
		goto err;
	result = DeeRegex_Search(&exec.rewr_exec, exec.rewr_range, &match_size);
	if unlikely(result == DEE_RE_STATUS_ERROR)
		goto err;
	if (result == DEE_RE_STATUS_NOMATCH)
		return_none;
	match_size = string_bytecnt2charcnt(self, (size_t)match_size, (char const *)exec.rewr_exec.rx_inbase + (size_t)result);
	result     = string_bytecnt2charcnt(self, (size_t)result, (char const *)exec.rewr_exec.rx_inbase);
	match_size += (size_t)result;
	return DeeTuple_NewII((size_t)result, (size_t)match_size);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
string_rerfind(String *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	Dee_ssize_t result;
	size_t match_size;
	struct DeeRegexExecWithRange exec;
	if unlikely(search_regex_getargs(self, argc, argv, kw, SEARCH_REGEX_GETARGS_FMT("rerfind"), &exec))
		goto err;
	result = DeeRegex_RSearch(&exec.rewr_exec, exec.rewr_range, &match_size);
	if unlikely(result == DEE_RE_STATUS_ERROR)
		goto err;
	if (result == DEE_RE_STATUS_NOMATCH)
		return_none;
	match_size = string_bytecnt2charcnt(self, (size_t)match_size, (char const *)exec.rewr_exec.rx_inbase + (size_t)result);
	result     = string_bytecnt2charcnt(self, (size_t)result, (char const *)exec.rewr_exec.rx_inbase);
	match_size += (size_t)result;
	return DeeTuple_NewII((size_t)result, (size_t)match_size);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
string_rescanf(String *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DREF ReSubStrings *substrings;
	Dee_ssize_t result;
	struct DeeRegexExec exec;
	if unlikely(generic_regex_getargs(self, argc, argv, kw, GENERIC_REGEX_GETARGS_FMT("rescanf"), &exec))
		goto err;
	substrings = ReSubStrings_Malloc(exec.rx_code->rc_ngrps);
	if unlikely(!substrings)
		goto err;
	exec.rx_nmatch = exec.rx_code->rc_ngrps;
	exec.rx_pmatch = substrings->rss_groups;
	result = DeeRegex_Match(&exec);
	if unlikely(result == DEE_RE_STATUS_ERROR)
		goto err_g;
	if (result == DEE_RE_STATUS_NOMATCH) {
		ReSubStrings_Free(substrings);
		return_empty_seq;
	}
	ReSubStrings_Init(substrings, (DeeObject *)self,
	                  exec.rx_inbase,
	                  exec.rx_code->rc_ngrps);
	return (DREF DeeObject *)substrings;
err_g:
	ReSubStrings_Free(substrings);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
string_regfind(String *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DREF ReGroups *groups;
	Dee_ssize_t result;
	size_t match_size;
	struct DeeRegexExecWithRange exec;
	if unlikely(search_regex_getargs(self, argc, argv, kw, SEARCH_REGEX_GETARGS_FMT("regfind"), &exec))
		goto err;
	groups = ReGroups_Malloc(1 + exec.rewr_exec.rx_code->rc_ngrps);
	if unlikely(!groups)
		goto err;
	exec.rewr_exec.rx_nmatch = exec.rewr_exec.rx_code->rc_ngrps;
	exec.rewr_exec.rx_pmatch = groups->rg_groups + 1;
	result = DeeRegex_Search(&exec.rewr_exec, exec.rewr_range, &match_size);
	if unlikely(result == DEE_RE_STATUS_ERROR)
		goto err_g;
	if (result == DEE_RE_STATUS_NOMATCH) {
		ReGroups_Free(groups);
		return_empty_seq;
	}
	string_bytecnt2charcnt_v(self, (char const *)exec.rewr_exec.rx_inbase,
	                         groups->rg_groups + 1,
	                         exec.rewr_exec.rx_code->rc_ngrps);
	match_size = string_bytecnt2charcnt(self, (size_t)match_size, (char const *)exec.rewr_exec.rx_inbase + (size_t)result);
	result     = string_bytecnt2charcnt(self, (size_t)result, (char const *)exec.rewr_exec.rx_inbase);
	match_size += (size_t)result;
	groups->rg_groups[0].rm_so = (size_t)result;
	groups->rg_groups[0].rm_eo = match_size;
	ReGroups_Init(groups, 1 + exec.rewr_exec.rx_code->rc_ngrps);
	return (DREF DeeObject *)groups;
err_g:
	ReGroups_Free(groups);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
string_regrfind(String *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DREF ReGroups *groups;
	Dee_ssize_t result;
	size_t match_size;
	struct DeeRegexExecWithRange exec;
	if unlikely(search_regex_getargs(self, argc, argv, kw, SEARCH_REGEX_GETARGS_FMT("regrfind"), &exec))
		goto err;
	groups = ReGroups_Malloc(1 + exec.rewr_exec.rx_code->rc_ngrps);
	if unlikely(!groups)
		goto err;
	exec.rewr_exec.rx_nmatch = exec.rewr_exec.rx_code->rc_ngrps;
	exec.rewr_exec.rx_pmatch = groups->rg_groups + 1;
	result = DeeRegex_RSearch(&exec.rewr_exec, exec.rewr_range, &match_size);
	if unlikely(result == DEE_RE_STATUS_ERROR)
		goto err_g;
	if (result == DEE_RE_STATUS_NOMATCH) {
		ReGroups_Free(groups);
		return_empty_seq;
	}
	string_bytecnt2charcnt_v(self, (char const *)exec.rewr_exec.rx_inbase,
	                         groups->rg_groups + 1,
	                         exec.rewr_exec.rx_code->rc_ngrps);
	match_size = string_bytecnt2charcnt(self, (size_t)match_size, (char const *)exec.rewr_exec.rx_inbase + (size_t)result);
	result     = string_bytecnt2charcnt(self, (size_t)result, (char const *)exec.rewr_exec.rx_inbase);
	match_size += (size_t)result;
	groups->rg_groups[0].rm_so = (size_t)result;
	groups->rg_groups[0].rm_eo = match_size;
	ReGroups_Init(groups, 1 + exec.rewr_exec.rx_code->rc_ngrps);
	return (DREF DeeObject *)groups;
err_g:
	ReGroups_Free(groups);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
string_reindex(String *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	Dee_ssize_t result;
	size_t match_size;
	struct DeeRegexExecWithRange exec;
	if unlikely(search_regex_getargs(self, argc, argv, kw, SEARCH_REGEX_GETARGS_FMT("reindex"), &exec))
		goto err;
	result = DeeRegex_Search(&exec.rewr_exec, exec.rewr_range, &match_size);
	if unlikely(result == DEE_RE_STATUS_ERROR)
		goto err;
	if (result == DEE_RE_STATUS_NOMATCH)
		goto err_not_found;
	match_size = string_bytecnt2charcnt(self, (size_t)match_size, (char const *)exec.rewr_exec.rx_inbase + (size_t)result);
	result     = string_bytecnt2charcnt(self, (size_t)result, (char const *)exec.rewr_exec.rx_inbase);
	match_size += (size_t)result;
	return DeeTuple_NewII((size_t)result, (size_t)match_size);
err_not_found:
	err_regex_index_not_found((DeeObject *)self);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
string_rerindex(String *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	Dee_ssize_t result;
	size_t match_size;
	struct DeeRegexExecWithRange exec;
	if unlikely(search_regex_getargs(self, argc, argv, kw, SEARCH_REGEX_GETARGS_FMT("rerindex"), &exec))
		goto err;
	result = DeeRegex_RSearch(&exec.rewr_exec, exec.rewr_range, &match_size);
	if unlikely(result == DEE_RE_STATUS_ERROR)
		goto err;
	if (result == DEE_RE_STATUS_NOMATCH)
		goto err_not_found;
	match_size = string_bytecnt2charcnt(self, (size_t)match_size, (char const *)exec.rewr_exec.rx_inbase + (size_t)result);
	result     = string_bytecnt2charcnt(self, (size_t)result, (char const *)exec.rewr_exec.rx_inbase);
	match_size += (size_t)result;
	return DeeTuple_NewII((size_t)result, (size_t)match_size);
err_not_found:
	err_regex_index_not_found((DeeObject *)self);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
string_regindex(String *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DREF ReGroups *groups;
	Dee_ssize_t result;
	size_t match_size;
	struct DeeRegexExecWithRange exec;
	if unlikely(search_regex_getargs(self, argc, argv, kw, SEARCH_REGEX_GETARGS_FMT("regindex"), &exec))
		goto err;
	groups = ReGroups_Malloc(1 + exec.rewr_exec.rx_code->rc_ngrps);
	if unlikely(!groups)
		goto err;
	exec.rewr_exec.rx_nmatch = exec.rewr_exec.rx_code->rc_ngrps;
	exec.rewr_exec.rx_pmatch = groups->rg_groups + 1;
	result = DeeRegex_Search(&exec.rewr_exec, exec.rewr_range, &match_size);
	if unlikely(result == DEE_RE_STATUS_ERROR)
		goto err_g;
	if (result == DEE_RE_STATUS_NOMATCH)
		goto err_not_found;
	string_bytecnt2charcnt_v(self, (char const *)exec.rewr_exec.rx_inbase,
	                         groups->rg_groups + 1,
	                         exec.rewr_exec.rx_code->rc_ngrps);
	match_size = string_bytecnt2charcnt(self, (size_t)match_size, (char const *)exec.rewr_exec.rx_inbase + (size_t)result);
	result     = string_bytecnt2charcnt(self, (size_t)result, (char const *)exec.rewr_exec.rx_inbase);
	match_size += (size_t)result;
	groups->rg_groups[0].rm_so = (size_t)result;
	groups->rg_groups[0].rm_eo = match_size;
	ReGroups_Init(groups, 1 + exec.rewr_exec.rx_code->rc_ngrps);
	return (DREF DeeObject *)groups;
err_not_found:
	err_regex_index_not_found((DeeObject *)self);
err_g:
	ReGroups_Free(groups);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
string_regrindex(String *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	DREF ReGroups *groups;
	Dee_ssize_t result;
	size_t match_size;
	struct DeeRegexExecWithRange exec;
	if unlikely(search_regex_getargs(self, argc, argv, kw, SEARCH_REGEX_GETARGS_FMT("regrindex"), &exec))
		goto err;
	groups = ReGroups_Malloc(1 + exec.rewr_exec.rx_code->rc_ngrps);
	if unlikely(!groups)
		goto err;
	exec.rewr_exec.rx_nmatch = exec.rewr_exec.rx_code->rc_ngrps;
	exec.rewr_exec.rx_pmatch = groups->rg_groups + 1;
	result = DeeRegex_RSearch(&exec.rewr_exec, exec.rewr_range, &match_size);
	if unlikely(result == DEE_RE_STATUS_ERROR)
		goto err_g;
	if (result == DEE_RE_STATUS_NOMATCH)
		goto err_not_found;
	string_bytecnt2charcnt_v(self, (char const *)exec.rewr_exec.rx_inbase,
	                         groups->rg_groups + 1,
	                         exec.rewr_exec.rx_code->rc_ngrps);
	match_size = string_bytecnt2charcnt(self, (size_t)match_size, (char const *)exec.rewr_exec.rx_inbase + (size_t)result);
	result     = string_bytecnt2charcnt(self, (size_t)result, (char const *)exec.rewr_exec.rx_inbase);
	match_size += (size_t)result;
	groups->rg_groups[0].rm_so = (size_t)result;
	groups->rg_groups[0].rm_eo = match_size;
	ReGroups_Init(groups, 1 + exec.rewr_exec.rx_code->rc_ngrps);
	return (DREF DeeObject *)groups;
err_not_found:
	err_regex_index_not_found((DeeObject *)self);
err_g:
	ReGroups_Free(groups);
err:
	return NULL;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
string_relocate(String *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	Dee_ssize_t result;
	size_t match_size;
	struct DeeRegexExecWithRange exec;
	if unlikely(search_regex_getargs(self, argc, argv, kw, SEARCH_REGEX_GETARGS_FMT("relocate"), &exec))
		goto err;
	result = DeeRegex_Search(&exec.rewr_exec, exec.rewr_range, &match_size);
	if unlikely(result == DEE_RE_STATUS_ERROR)
		goto err;
	if (result == DEE_RE_STATUS_NOMATCH)
		return_none;
	return DeeString_NewUtf8((char const *)exec.rewr_exec.rx_inbase + (size_t)result,
	                         match_size, STRING_ERROR_FSTRICT);
err:
	return NULL;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
string_rerlocate(String *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	Dee_ssize_t result;
	size_t match_size;
	struct DeeRegexExecWithRange exec;
	if unlikely(search_regex_getargs(self, argc, argv, kw, SEARCH_REGEX_GETARGS_FMT("rerlocate"), &exec))
		goto err;
	result = DeeRegex_RSearch(&exec.rewr_exec, exec.rewr_range, &match_size);
	if unlikely(result == DEE_RE_STATUS_ERROR)
		goto err;
	if (result == DEE_RE_STATUS_NOMATCH)
		return_none;
	return DeeString_NewUtf8((char const *)exec.rewr_exec.rx_inbase + (size_t)result,
	                         match_size, STRING_ERROR_FSTRICT);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeTupleObject *DCALL
string_pack_utf8_partition_not_found(char const *__restrict utf8_base,
                                     size_t startoff, size_t endoff) {
	DREF DeeTupleObject *result;
	result = DeeTuple_NewUninitialized(3);
	if unlikely(!result)
		goto done;
	if (startoff < endoff) {
		DREF DeeObject *str0;
		str0 = DeeString_NewUtf8(utf8_base + startoff,
		                         endoff - startoff,
		                         STRING_ERROR_FSTRICT);
		if unlikely(!str0)
			goto err_r;
		DeeTuple_SET(result, 0, str0);
	} else {
		DeeTuple_SET(result, 0, Dee_EmptyString);
		Dee_Incref(Dee_EmptyString);
	}
	DeeTuple_SET(result, 1, Dee_EmptyString);
	DeeTuple_SET(result, 2, Dee_EmptyString);
	Dee_Incref_n(Dee_EmptyString, 2);
	return result;
done:
	return result;
err_r:
	DeeTuple_FreeUninitialized(result);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeTupleObject *DCALL
string_pack_utf8_partition_found(char const *__restrict utf8_base,
                                 size_t match_startoff,
                                 size_t match_endoff,
                                 size_t str_endoff) {
	DREF DeeTupleObject *result;
	DREF DeeObject *str;
	result = DeeTuple_NewUninitialized(3);
	if unlikely(!result)
		goto done;
	str = DeeString_NewUtf8(utf8_base, match_startoff, STRING_ERROR_FSTRICT);
	if unlikely(!str)
		goto err_r_0;
	DeeTuple_SET(result, 0, str);
	str = DeeString_NewUtf8(utf8_base + match_startoff, match_endoff - match_startoff, STRING_ERROR_FSTRICT);
	if unlikely(!str)
		goto err_r_1;
	DeeTuple_SET(result, 1, str);
	str = DeeString_NewUtf8(utf8_base + match_endoff, str_endoff - match_endoff, STRING_ERROR_FSTRICT);
	if unlikely(!str)
		goto err_r_2;
	DeeTuple_SET(result, 2, str);
	return result;
done:
	return result;
err_r_2:
	Dee_Decref_likely(DeeTuple_GET(result, 1));
err_r_1:
	Dee_Decref_likely(DeeTuple_GET(result, 0));
err_r_0:
	DeeTuple_FreeUninitialized(result);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeTupleObject *DCALL
string_repartition(String *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	Dee_ssize_t result;
	size_t match_size;
	struct DeeRegexExecWithRange exec;
	if unlikely(search_regex_getargs(self, argc, argv, kw, SEARCH_REGEX_GETARGS_FMT("repartition"), &exec))
		goto err;
	result = DeeRegex_Search(&exec.rewr_exec, exec.rewr_range, &match_size);
	if unlikely(result == DEE_RE_STATUS_ERROR)
		goto err;
	if (result == DEE_RE_STATUS_NOMATCH) {
		return string_pack_utf8_partition_not_found((char const *)exec.rewr_exec.rx_inbase,
		                                            exec.rewr_exec.rx_startoff,
		                                            exec.rewr_exec.rx_endoff);
	}
	result -= exec.rewr_exec.rx_startoff;
	exec.rewr_exec.rx_endoff -= exec.rewr_exec.rx_startoff;
	exec.rewr_exec.rx_inbase = (char const *)exec.rewr_exec.rx_inbase + exec.rewr_exec.rx_startoff;
	return string_pack_utf8_partition_found((char const *)exec.rewr_exec.rx_inbase,
	                                        (size_t)result,
	                                        (size_t)result + match_size,
	                                        exec.rewr_exec.rx_endoff);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeTupleObject *DCALL
string_rerpartition(String *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	Dee_ssize_t result;
	size_t match_size;
	struct DeeRegexExecWithRange exec;
	if unlikely(search_regex_getargs(self, argc, argv, kw, SEARCH_REGEX_GETARGS_FMT("rerpartition"), &exec))
		goto err;
	result = DeeRegex_RSearch(&exec.rewr_exec, exec.rewr_range, &match_size);
	if unlikely(result == DEE_RE_STATUS_ERROR)
		goto err;
	if (result == DEE_RE_STATUS_NOMATCH) {
		return string_pack_utf8_partition_not_found((char const *)exec.rewr_exec.rx_inbase,
		                                            exec.rewr_exec.rx_startoff,
		                                            exec.rewr_exec.rx_endoff);
	}
	result -= exec.rewr_exec.rx_startoff;
	exec.rewr_exec.rx_endoff -= exec.rewr_exec.rx_startoff;
	exec.rewr_exec.rx_inbase = (char const *)exec.rewr_exec.rx_inbase + exec.rewr_exec.rx_startoff;
	return string_pack_utf8_partition_found((char const *)exec.rewr_exec.rx_inbase,
	                                        (size_t)result,
	                                        (size_t)result + match_size,
	                                        exec.rewr_exec.rx_endoff);
err:
	return NULL;
}

#ifndef CONFIG_HAVE_memsetp
#define CONFIG_HAVE_memsetp
#define memsetp(dst, pointer, num_pointers) \
	dee_memsetp(dst, (__UINTPTR_TYPE__)(pointer), num_pointers)
DeeSystem_DEFINE_memsetp(dee_memsetp)
#endif /* !CONFIG_HAVE_memsetp */

PRIVATE struct keyword string_rereplace_kwlist[] = { K(pattern), K(replace), K(max), K(rules), KEND };
PRIVATE WUNUSED NONNULL((1)) DREF String *DCALL
string_rereplace(String *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct unicode_printer printer = UNICODE_PRINTER_INIT;
	DeeObject *pattern, *replace, *rules = NULL;
	struct DeeRegexMatch groups[9];
	size_t maxreplace = (size_t)-1;
	char const *replace_start, *replace_end;
	struct DeeRegexExec exec;
	if (DeeArg_UnpackKw(argc, argv, kw, string_rereplace_kwlist,
	                    "oo|" UNPuSIZ "o:rereplace",
	                    &pattern, &replace, &maxreplace, &rules))
		goto err;
	if (DeeObject_AssertTypeExact(pattern, &DeeString_Type))
		goto err;
	if (DeeObject_AssertTypeExact(replace, &DeeString_Type))
		goto err;
	replace_start = DeeString_AsUtf8(replace);
	if unlikely(!replace_start)
		goto err;
	replace_end = replace_start + WSTR_LENGTH(replace_start);
	exec.rx_eflags = 0; /* TODO: NOTBOL/NOTEOL */
	exec.rx_code = DeeString_GetRegex(pattern, DEE_REGEX_COMPILE_NORMAL, rules);
	if unlikely(!exec.rx_code)
		goto err;
	exec.rx_nmatch = COMPILER_LENOF(groups);
	exec.rx_pmatch = groups;
	exec.rx_inbase = DeeString_AsUtf8((DeeObject *)self);
	if unlikely(!exec.rx_inbase)
		goto err;
	exec.rx_insize   = WSTR_LENGTH(exec.rx_inbase);
	exec.rx_startoff = 0;
	exec.rx_endoff   = exec.rx_insize;
	while (exec.rx_startoff < exec.rx_endoff && maxreplace) {
		char const *replace_iter, *replace_flush;
		Dee_ssize_t match_offset;
		size_t match_size;
		memsetp(groups, (void *)(uintptr_t)(size_t)-1, 2 * 9);
		match_offset = DeeRegex_SearchNoEpsilon(&exec, (size_t)-1, &match_size);
		if unlikely(match_offset == DEE_RE_STATUS_ERROR)
			goto err;
		if (match_offset == DEE_RE_STATUS_NOMATCH)
			break;
		/* Flush until start-of-match. */
		if unlikely(unicode_printer_printutf8(&printer,
		                                      (char const *)exec.rx_inbase + exec.rx_startoff,
		                                      (size_t)match_offset - exec.rx_startoff) < 0)
			goto err;

		/* Parse and print the replacement string. */
		for (replace_iter = replace_flush = replace_start;
		     replace_iter < replace_end;) {
			struct DeeRegexMatch match;
			char ch = *replace_iter;
			switch (ch) {

			case '&':
				/* Insert full match. */
				match.rm_so = (size_t)match_offset;
				match.rm_eo = (size_t)match_offset + match_size;
do_insert_match:
				if unlikely(unicode_printer_printutf8(&printer, replace_flush,
				                                      (size_t)(replace_iter - replace_flush)) < 0)
					goto err;
				if unlikely(unicode_printer_printutf8(&printer,
				                                      (char const *)exec.rx_inbase + match.rm_so,
				                                      (size_t)(match.rm_eo - match.rm_so)) < 0)
					goto err;
				++replace_iter;
				if (ch != '&')
					++replace_iter;
				replace_flush = replace_iter;
				break;

			case '\\':
				ch = replace_iter[1];
				if (ch == '&' || ch == '\\') {
					/* Insert literal '&' or '\' */
					if unlikely(unicode_printer_printutf8(&printer, replace_flush,
					                                      (size_t)(replace_iter - replace_flush)) < 0)
						goto err;
					++replace_iter;
					replace_flush = replace_iter;
					++replace_iter;
				} else if (ch >= '1' && ch <= '9') {
					/* Insert matched group N.
					 * NOTE: When the group was never matched, both of its offsets will be equal
					 *       here, meaning that the code above will simply print an empty string! */
					match = groups[ch - '1'];
					goto do_insert_match;
				} else {
					++replace_iter;
				}
				break;

			default:
				++replace_iter;
				break;
			}
		}

		/* Flush remainder of replacement string. */
		if (replace_flush < replace_end) {
			if unlikely(unicode_printer_printutf8(&printer, replace_flush,
			                                      (size_t)(replace_end - replace_flush)) < 0)
				goto err;
		}

		/* Keep on scanning after the matched area. */
		exec.rx_startoff = (size_t)match_offset + match_size;
		--maxreplace;
	}

	/* Flush remainder */
#ifndef __OPTIMIZE_SIZE__
	if unlikely(exec.rx_startoff == 0) {
		unicode_printer_fini(&printer);
		return_reference_(self);
	}
#endif /* !__OPTIMIZE_SIZE__ */
	if unlikely(unicode_printer_printutf8(&printer,
	                                       (char const *)exec.rx_inbase + exec.rx_startoff,
	                                       exec.rx_endoff - exec.rx_startoff) < 0)
		goto err;
	return (DREF String *)unicode_printer_pack(&printer);
err:
	unicode_printer_fini(&printer);
	return NULL;
}

struct DeeRegexBaseExec {
	DREF String               *rx_pattern;  /* [1..1] Pattern string (only a reference within objects in "./reproxy.c.inl") */
	struct DeeRegexCode const *rx_code;     /* [1..1] Regex code */
	void const                *rx_inbase;   /* [0..rx_insize][valid_if(rx_startoff < rx_endoff)] Input data to scan
	                                         * When `rx_code' was compiled with `DEE_REGEX_COMPILE_NOUTF8', this data
	                                         * is treated as raw bytes; otherwise, it is treated as a utf-8 string.
	                                         * In either case, `rx_insize' is the # of bytes within this buffer. */
	size_t                     rx_insize;   /* Total # of bytes starting at `rx_inbase' */
	size_t                     rx_startoff; /* Starting byte offset into `rx_inbase' of data to match. */
	size_t                     rx_endoff;   /* Ending byte offset into `rx_inbase' of data to match. */
	unsigned int               rx_eflags;   /* Execution-flags (set of `DEE_RE_EXEC_*') */
};

/* Functions from "./reproxy.c.inl" */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
string_re_scanf(String *__restrict self,
                struct DeeRegexBaseExec const *__restrict exec);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
string_re_findall(String *__restrict self,
                  struct DeeRegexBaseExec const *__restrict exec);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
string_reg_findall(String *__restrict self,
                  struct DeeRegexBaseExec const *__restrict exec);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
string_re_locateall(String *__restrict self,
                    struct DeeRegexBaseExec const *__restrict exec);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
string_re_split(String *__restrict self,
                struct DeeRegexBaseExec const *__restrict exec);


#define BASE_REGEX_GETARGS_FMT GENERIC_REGEX_GETARGS_FMT
#define base_regex_kwlist      generic_regex_kwlist
PRIVATE WUNUSED NONNULL((1, 5, 6)) int DCALL
base_regex_getargs(String *self, size_t argc, DeeObject *const *argv,
                   DeeObject *kw, char const *__restrict fmt,
                   struct DeeRegexBaseExec *__restrict result) {
	DeeObject *rules = NULL;
	result->rx_startoff = 0;
	result->rx_endoff   = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, base_regex_kwlist, fmt,
	                    &result->rx_pattern,
	                    &result->rx_startoff,
	                    &result->rx_endoff,
	                    &rules))
		goto err;
	if (DeeObject_AssertTypeExact(result->rx_pattern, &DeeString_Type))
		goto err;
	result->rx_code = DeeString_GetRegex((DeeObject *)result->rx_pattern,
	                                     DEE_REGEX_COMPILE_NORMAL, rules);
	if unlikely(!result->rx_code)
		goto err;
	result->rx_eflags = 0; /* TODO: NOTBOL/NOTEOL */
	result->rx_inbase = DeeString_AsUtf8((DeeObject *)self);
	if unlikely(!result->rx_inbase)
		goto err;
	result->rx_insize = WSTR_LENGTH(result->rx_inbase);
	if (result->rx_endoff >= result->rx_insize) {
		result->rx_endoff = result->rx_insize;
	} else {
		result->rx_endoff = string_charcnt2bytecnt(self, result->rx_endoff, (char const *)result->rx_inbase);
	}
	if (result->rx_startoff > 0)
		result->rx_startoff = string_charcnt2bytecnt(self, result->rx_startoff, (char const *)result->rx_inbase);
	return 0;
err:
	return -1;
}



PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
string_refindall(String *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct DeeRegexBaseExec exec;
	if unlikely(base_regex_getargs(self, argc, argv, kw, BASE_REGEX_GETARGS_FMT("refindall"), &exec))
		goto err;
	return string_re_findall(self, &exec);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
string_regfindall(String *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct DeeRegexBaseExec exec;
	if unlikely(base_regex_getargs(self, argc, argv, kw, BASE_REGEX_GETARGS_FMT("regfindall"), &exec))
		goto err;
	return string_reg_findall(self, &exec);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
string_relocateall(String *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct DeeRegexBaseExec exec;
	if unlikely(base_regex_getargs(self, argc, argv, kw, BASE_REGEX_GETARGS_FMT("relocateall"), &exec))
		goto err;
	return string_re_locateall(self, &exec);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
string_resplit(String *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct DeeRegexBaseExec exec;
	if unlikely(base_regex_getargs(self, argc, argv, kw, BASE_REGEX_GETARGS_FMT("resplit"), &exec))
		goto err;
	return string_re_split(self, &exec);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
string_restartswith(String *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	Dee_ssize_t result;
	struct DeeRegexExec exec;
	if unlikely(generic_regex_getargs(self, argc, argv, kw, GENERIC_REGEX_GETARGS_FMT("restartswith"), &exec))
		goto err;
	result = DeeRegex_Match(&exec);
	if unlikely(result == DEE_RE_STATUS_ERROR)
		goto err;
	return_bool(result != DEE_RE_STATUS_NOMATCH);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
string_reendswith(String *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	size_t match_size;
	Dee_ssize_t result;
	struct DeeRegexExec exec;
	if unlikely(generic_regex_getargs(self, argc, argv, kw, GENERIC_REGEX_GETARGS_FMT("reendswith"), &exec))
		goto err;
	result = DeeRegex_RSearch(&exec, (size_t)-1, &match_size);
	if unlikely(result == DEE_RE_STATUS_ERROR)
		goto err;
	if unlikely(result == DEE_RE_STATUS_NOMATCH)
		return_false;
	ASSERT((size_t)result + match_size <= exec.rx_endoff || !match_size);
	return_bool((size_t)result + match_size >= exec.rx_endoff);
err:
	return NULL;
}

PRIVATE WUNUSED DREF String *DCALL
string_relstrip(String *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct DeeRegexExec exec;
	if unlikely(generic_regex_getargs(self, argc, argv, kw, GENERIC_REGEX_GETARGS_FMT("relstrip"), &exec))
		goto err;
	for (;;) {
		Dee_ssize_t result;
		result = DeeRegex_Match(&exec);
		if unlikely(result == DEE_RE_STATUS_ERROR)
			goto err;
		if (result == DEE_RE_STATUS_NOMATCH)
			break;
		if (result == 0)
			break; /* Prevent infinite loop when epsilon is matched. */
		exec.rx_startoff = (size_t)result;
		if (exec.rx_startoff >= exec.rx_endoff)
			break;
	}

	/* End-of-matching-area */
	if unlikely(exec.rx_startoff >= exec.rx_endoff)
		return_reference_((String *)Dee_EmptyString);
	return (DREF String *)DeeString_NewUtf8((char const *)exec.rx_inbase + exec.rx_startoff,
	                                        exec.rx_endoff - exec.rx_startoff,
	                                        STRING_ERROR_FSTRICT);
err:
	return NULL;
}

PRIVATE WUNUSED DREF String *DCALL
string_rerstrip(String *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct DeeRegexExec exec;
	if unlikely(generic_regex_getargs(self, argc, argv, kw, GENERIC_REGEX_GETARGS_FMT("rerstrip"), &exec))
		goto err;
	for (;;) {
		size_t match_size;
		Dee_ssize_t result;
		result = DeeRegex_RSearch(&exec, (size_t)-1, &match_size);
		if unlikely(result == DEE_RE_STATUS_ERROR)
			goto err;
		if (result == DEE_RE_STATUS_NOMATCH)
			break;
		if (match_size == 0)
			break; /* Prevent infinite loop when epsilon is matched. */
		ASSERT((size_t)result + match_size <= exec.rx_endoff);
		if ((size_t)result + match_size < exec.rx_endoff)
			break;
		exec.rx_endoff = (size_t)result;
		if (exec.rx_startoff >= exec.rx_endoff)
			break;
	}

	/* End-of-matching-area */
	if unlikely(exec.rx_startoff >= exec.rx_endoff)
		return_reference_((String *)Dee_EmptyString);
	return (DREF String *)DeeString_NewUtf8((char const *)exec.rx_inbase + exec.rx_startoff,
	                                        exec.rx_endoff - exec.rx_startoff,
	                                        STRING_ERROR_FSTRICT);
err:
	return NULL;
}

PRIVATE WUNUSED DREF String *DCALL
string_restrip(String *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	struct DeeRegexExec exec;
	if unlikely(generic_regex_getargs(self, argc, argv, kw, GENERIC_REGEX_GETARGS_FMT("restrip"), &exec))
		goto err;
	/* lstrip */
	for (;;) {
		Dee_ssize_t result;
		result = DeeRegex_Match(&exec);
		if unlikely(result == DEE_RE_STATUS_ERROR)
			goto err;
		if (result == DEE_RE_STATUS_NOMATCH)
			break;
		if (result == 0)
			break; /* Prevent infinite loop when epsilon is matched. */
		exec.rx_startoff = (size_t)result;
		if (exec.rx_startoff >= exec.rx_endoff)
			break;
	}

	/* rstrip */
	for (;;) {
		size_t match_size;
		Dee_ssize_t result;
		result = DeeRegex_RSearch(&exec, (size_t)-1, &match_size);
		if unlikely(result == DEE_RE_STATUS_ERROR)
			goto err;
		if (result == DEE_RE_STATUS_NOMATCH)
			break;
		if (match_size == 0)
			break; /* Prevent infinite loop when epsilon is matched. */
		ASSERT((size_t)result + match_size <= exec.rx_endoff);
		if ((size_t)result + match_size < exec.rx_endoff)
			break;
		exec.rx_endoff = (size_t)result;
		if (exec.rx_startoff >= exec.rx_endoff)
			break;
	}

	/* End-of-matching-area */
	if unlikely(exec.rx_startoff >= exec.rx_endoff)
		return_reference_((String *)Dee_EmptyString);
	return (DREF String *)DeeString_NewUtf8((char const *)exec.rx_inbase + exec.rx_startoff,
	                                        exec.rx_endoff - exec.rx_startoff,
	                                        STRING_ERROR_FSTRICT);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
string_recontains(String *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	Dee_ssize_t result;
	struct DeeRegexExec exec;
	if unlikely(generic_regex_getargs(self, argc, argv, kw, GENERIC_REGEX_GETARGS_FMT("recontains"), &exec))
		goto err;
	result = DeeRegex_Search(&exec, (size_t)-1, NULL);
	if unlikely(result == DEE_RE_STATUS_ERROR)
		goto err;
	return_bool(result != DEE_RE_STATUS_NOMATCH);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
string_recount(String *self, size_t argc, DeeObject *const *argv, DeeObject *kw) {
	size_t count;
	struct DeeRegexExec exec;
	if unlikely(generic_regex_getargs(self, argc, argv, kw, GENERIC_REGEX_GETARGS_FMT("recount"), &exec))
		goto err;
	count = 0;
	for (;;) {
		Dee_ssize_t result;
		size_t match_size;
		result = DeeRegex_SearchNoEpsilon(&exec, (size_t)-1, &match_size);
		if unlikely(result == DEE_RE_STATUS_ERROR)
			goto err;
		if (result == DEE_RE_STATUS_NOMATCH)
			break;
		++count;
		exec.rx_startoff = (size_t)result + match_size;
		if (exec.rx_startoff >= exec.rx_endoff)
			break;
	}
	return DeeInt_NewSize(count);
err:
	return NULL;
}


INTDEF struct type_method tpconst string_methods[];
INTERN_TPCONST struct type_method tpconst string_methods[] = {

	/* String encode/decode functions */
	TYPE_KWMETHOD("decode", &string_decode,
	              /* TODO: constexpr if "codec" is one of the built-ins */
	              "(codec:?.,errors=!Pstrict)->?X2?.?O\n"
	              "#tValueError{The given @codec or @errors wasn't recognized}"
	              "#tUnicodeDecodeError{@this ?. could not be decoded as @codec and @errors was set to $\"strict\"}"
	              "#perrors{The way that decode-errors are handled as one of $\"strict\", $\"replace\" or $\"ignore\"}"
	              "Decode @this ?., re-interpreting its underlying character bytes as @codec\n"
	              "Prior to processing, @codec is normalized as follows:\n"
	              "${"
	              /**/ "name = name.lower().replace(\"_\", \"-\");\n"
	              /**/ "if (name.startswith(\"iso-\"))\n"
	              /**/ "	name = \"iso\" + name[4:];\n"
	              /**/ "else if (name.startswith(\"cp-\")) {\n"
	              /**/ "	name = \"cp\" + name[3:];\n"
	              /**/ "}"
	              "}\n"
	              "Following that, @codec is compared against the following list of builtin codecs\n"
	              "#T{Codec Name|Aliases|Return type|Description~"
	              "$\"ascii\"|$\"646\", $\"us-ascii\"|Same as $this|"
	              /**/ "Validate that all character of @this are apart of the unicode range U+0000 - U+007F&"
	              "$\"latin-1\"|$\"iso8859-1\", $\"iso8859\", $\"8859\", $\"cp819\", $\"latin\", $\"latin1\", $\"l1\"|Same as $this|"
	              /**/ "Validate that all character of @this are apart of the unicode range U+0000 - U+00FF&"
	              "$\"utf-8\"|$\"utf8\", $\"u8\", $\"utf\"|?.|Decode ${this.bytes()} as a UTF-8 encoded byte sequence&"
	              "$\"utf-16\"|$\"utf16\", $\"u16\"|?.|Decode ${this.bytes()} as a UTF-16 sequence, encoded in host-endian&"
	              "$\"utf-16-le\"|$\"utf16-le\", $\"u16-le\", $\"utf-16le\", $\"utf16le\", $\"u16le\"|?.|Decode ${this.bytes()} as a UTF-16 sequence, encoded in little-endian&"
	              "$\"utf-16-be\"|$\"utf16-be\", $\"u16-be\", $\"utf-16be\", $\"utf16be\", $\"u16be\"|?.|Decode ${this.bytes()} as a UTF-16 sequence, encoded in big-endian&"
	              "$\"utf-32\"|$\"utf32\", $\"u32\"|?.|Decode ${this.bytes()} as a UTF-32 sequence, encoded in host-endian&"
	              "$\"utf-32-le\"|$\"utf32-le\", $\"u32-le\", $\"utf-32le\", $\"utf32le\", $\"u32le\"|?.|Decode ${this.bytes()} as a UTF-32 sequence, encoded in little-endian&"
	              "$\"utf-32-be\"|$\"utf32-be\", $\"u32-be\", $\"utf-32be\", $\"utf32be\", $\"u32be\"|?.|Decode ${this.bytes()} as a UTF-32 sequence, encoded in big-endian&"
	              "$\"string-escape\"|$\"backslash-escape\", $\"c-escape\"|?.|Decode a backslash-escaped string after stripping an optional leading and trailing $\"\\\"\" or $\"\\\'\" character}\n"
	              "If the given @codec is not apart of this list, a call is made to ?Ecodecs:decode"),
	TYPE_KWMETHOD("encode", &string_encode,
	              "(codec:?.,errors=!Pstrict)->?X3?DBytes?.?O\n"
	              "#tValueError{The given @codec or @errors wasn't recognized}"
	              "#tUnicodeEncodeError{@this ?. could not be decoded as @codec and @errors was set to $\"strict\"}"
	              "#perrors{The way that decode-errors are handled as one of $\"strict\", $\"replace\" or $\"ignore\"}"
	              "Encode @this ?., re-interpreting its underlying character bytes as @codec\n"
	              "Prior to processing, @codec is normalized as follows:\n"
	              "${"
	              /**/ "name = name.lower().replace(\"_\", \"-\");\n"
	              /**/ "if (name.startswith(\"iso-\"))\n"
	              /**/ "	name = \"iso\" + name[4:];\n"
	              /**/ "else if (name.startswith(\"cp-\")) {\n"
	              /**/ "	name = \"cp\" + name[3:];\n"
	              /**/ "}"
	              "}\n"
	              "Following that, @codec is compared against the following list of builtin codecs\n"
	              "#T{Codec Name|Aliases|Return type|Description~"
	              "$\"ascii\"|$\"646\", $\"us-ascii\"|Same as $this|"
	              /**/ "Validate that all character of @this are apart of the unicode range U+0000 - U+007F&"
	              "$\"latin-1\"|$\"iso8859-1\", $\"iso8859\", $\"8859\", $\"cp819\", $\"latin\", $\"latin1\", $\"l1\"|Same as $this|"
	              /**/ "Validate that all character of @this are apart of the unicode range U+0000 - U+00FF&"
	              "$\"utf-8\"|$\"utf8\", $\"u8\", $\"utf\"|?DBytes|Encode character of @this ?. as a UTF-8 encoded byte sequence&"
	              "$\"utf-16\"|$\"utf16\", $\"u16\"|?DBytes|Encode 'as a UTF-16 sequence, encoded in host-endian&"
	              "$\"utf-16-le\"|$\"utf16-le\", $\"u16-le\", $\"utf-16le\", $\"utf16le\", $\"u16le\"|?DBytes|Encode @this ?. as a UTF-16 sequence, encoded in little-endian&"
	              "$\"utf-16-be\"|$\"utf16-be\", $\"u16-be\", $\"utf-16be\", $\"utf16be\", $\"u16be\"|?DBytes|Encode @this ?. as a UTF-16 sequence, encoded in big-endian&"
	              "$\"utf-32\"|$\"utf32\", $\"u32\"|?DBytes|Encode @this ?. as a UTF-32 sequence, encoded in host-endian&"
	              "$\"utf-32-le\"|$\"utf32-le\", $\"u32-le\", $\"utf-32le\", $\"utf32le\", $\"u32le\"|?DBytes|Encode @this ?. as a UTF-32 sequence, encoded in little-endian&"
	              "$\"utf-32-be\"|$\"utf32-be\", $\"u32-be\", $\"utf-32be\", $\"utf32be\", $\"u32be\"|?DBytes|Encode @this ?. as a UTF-32 sequence, encoded in big-endian&"
	              "$\"string-escape\"|$\"backslash-escape\", $\"c-escape\"|?.|Encode @this ?. as a backslash-escaped string. This is similar to ?#{op:repr}, however the string is not surrounded by $\"\\\"\"-characters}\n"
	              "If the given @codec is not apart of this list, a call is made to ?Ecodecs:encode"),
	TYPE_METHOD_F("bytes", &string_bytes,
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST,
	              "(allow_invalid=!f)->?DBytes\n"
	              "(start:?Dint,end:?Dint,allow_invalid=!f)->?DBytes\n"
	              "#tValueError{@allow_invalid is ?f, and @this ?. contains characters above $0xff}"
	              "Returns a read-only bytes representation of the characters within ${this.substr(start, end)}, "
	              /**/ "using a single byte per character. A character greater than $0xff either causes :ValueError "
	              /**/ "to be thrown (when @allow_invalid is false), or is replaced with the ASCII character "
	              /**/ "$\"?\" in the returned Bytes object"),
	TYPE_METHOD_F("ord", &string_ord,
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST | METHOD_FNOREFESCAPE,
	              "->?Dint\n"
	              "#tValueError{The length of @this ?. is not equal to $1}"
	              "Return the ordinal integral value of @this single-character ?.\n"
	              "\n"
	              "(index:?Dint)->?Dint\n"
	              "#tIntegerOverflow{The given @index is lower than $0}"
	              "#tIndexError{The given @index is greater than ${##this}}"
	              "Returns the ordinal integral value of the @index'th character of @this ?."),

	/* String formatting / scanning. */
	TYPE_METHOD_F(STR_format, &string_format,
	              /* TODO: CONSTEXPR based on what appears in the template string of "thisarg", and how it uses each argument */
	              METHOD_FNOREFESCAPE,
	              "(args:?S?O)->?.\n"
	              "Format @this ?. using @args:\n"

	              "This kind of formating is the most powerful variant of string "
	              /**/ "formatting available in deemon.\n"

	              "Like most other ways of formatting strings, all text outside of format "
	              /**/ "specifiers is simply copied into the output string. Special rules "
	              /**/ "are applied to text found inside or curly-braces ($\"{...}\")\n"

	              "In order to escape either $\"{\" or $\"}\" characters, use $\"{{\" and $\"}}\" respectively\n"

	              "Processing of text inside of curly-brace is split into 2 parts, "
	              /**/ "both of which are optional and separated by $\":\":\n"

	              "#L-{"
	              "The object selection expression part|"
	              "The object spec string portion (which may contain further "
	              /**/ "$\"{...}\"-blocks that are expanded beforehand)"
	              "}\n"

	              "#T{Selection expression|Description~"
	              "$\"{}\"|Lazily create an iterator $iter as ${args.operator iter()} when "
	              /*   */ "encountered the first time, then invoke ${iter.operator next()} "
	              /*   */ "and use its return value as format object&"
	              "$\"{foo}\"|Use ${args[\"foo\"]} as format object&"
	              "$\"{42}\"|Use ${args[42]} as format object&"
	              "$\"{(x)}\"|Alias for $\"{x}\"&"
	              "$\"{x.<expr>}\"|With $x being another selection expression, use "
	              /*           */ "${x.operator . (<expr>)} (whitespace before and "
	              /*           */ "after $\".\" is ignored)&"
	              "$\"{x[<expr>]}\"|With $x being another selection expression, use "
	              /*            */ "${x.operator [] (<expr>)} (whitespace before "
	              /*            */ "and after $\"[\" and $\"]\" is ignored)&"
	              "$\"{x[<expr>:]}\"|With $x being another selection expression, use "
	              /*             */ "${x.operator [:] (<expr>, none)} (whitespace "
	              /*             */ "before and after $\"[\" and $\"]\" is ignored)&"
	              "$\"{x[:<expr>]}\"|With $x being another selection expression, use "
	              /*             */ "${x.operator [:] (none, <expr>)} (whitespace "
	              /*             */ "before and after $\"[\" and $\"]\" is ignored)&"
	              "$\"{x[<expr1>:<expr2>]}\"|With $x being another selection expression, use "
	              /*                     */ "${x.operator [:] (<expr1>, <expr2>)} (whitespace before "
	              /*                     */ "and after $\"[\", $\":\" and $\"]\" is ignored)&"
	              "$\"{x(<expr1>, <expr2>, [...])}\"|With $x being another selection expression, use "
	              /*                             */ "${x(<expr1>, <expr2>,[...])} (whitespace before "
	              /*                             */ "and after $\"(\", $\",\" and $\")\" is ignored)&"
	              "$\"{x(<expr1>, <expr2>...)}\"|With $x being another selection expression, use "
	              /*                         */ "${x(<expr1>, <expr2>...)} (i.e. you're able to use "
	              /*                         */ "expand expressions here) (whitespace before and after "
	              /*                         */ "$\"(\", $\",\", $\"...\" and $\")\" is ignored)&"
	              "$\"{x ? <expr1> : <expr2>}\"|With $x being another selection expression, use sub-"
	              /*                        */ "expression <expr1> if ${x.operator bool()} is true, "
	              /*                        */ "or <expr2> otherwise (whitespace before and after "
	              /*                        */ "$\"?\" and $\":\" is ignored)&"
	              "$\"{x ? : <expr2>}\"|Re-use $x as true-result, similar to $\"{x ? {x} : <expr2>}\" "
	              /*                */ "(whitespace before and after $\"?\" and $\":\" is ignored)&"
	              "$\"{x ? <expr1>}\"|Use ?N as false-result\" (whitespace before and after "
	              /*              */ "$\"?\" is ignored)}\n"

	              "Sub-expressions in selections strings (the ${<expr>} above). "
	              /**/ "Note however that the angle brackets are not part of the syntax, "
	              /**/ "but used to highlight association:\n"

	              "#T{Sub-expression|Description~"
	              "$\"42\"|Evaluates to ${int(42)}&"
	              "$\"foobar\"|Evaluates to $\"foobar\"&"
	              "$\"{x}\"|Evaluates to the object selected by another selection expression $x}\n"

	              "Once an object to-be formatted has been selected, the way in which it should "
	              /**/ "then be formatted can be altered through use of spec string portion\n"

	              "If a spec portion is not present, ${str selected_object} is simply appended "
	              /**/ "to the resulting ?.. Otherwise, ${selected_object.__format__(spec_string)} "
	              /**/ "is invoked, and the resulting object is appended instead\n"

	              "For this purpose, ?DObject implements a function ?A__format__?DObject that provides "
	              /**/ "some basic spec options, which are also used for types not derived from ?DObject, "
	              /**/ "or ones overwriting ${operator .}, where invocationg with $\"__format__\" throws "
	              /**/ "either a :NotImplemented or :AttributeError error.\n"

	              "When used, ?A__format__?DObject provides the following functionality, with a "
	              /**/ ":ValueError being thrown for anything else, or anything not matching these "
	              /**/ "criteria\n"

	              "#T{Spec option|Description~"
	              /**/ "$\"{:42}\"|Will append ${selected_object.operator str().ljust(42)} to the resulting ?. (s.a. ?#ljust)&"
	              /**/ "$\"{:<42}\"|Same as $\"{:42}\"&"
	              /**/ "$\"{:>42}\"|Will append ${selected_object.operator str().rjust(42)} to the resulting ?. (s.a. ?#rjust)&"
	              /**/ "$\"{:^42}\"|Will append ${selected_object.operator str().center(42)} to the resulting ?. (s.a. ?#center)&"
	              /**/ "$\"{:=42}\"|Will append ${selected_object.operator str().zfill(42)} to the resulting ?. (s.a. ?#zfill)&"
	              /**/ "$\"{:42:foo}\"|Will append ${selected_object.operator str().ljust(42, \"foo\")} to the resulting ?. (s.a. ?#ljust)&"
	              /**/ "$\"{:<42:foo}\"|Same as $\"{:42:foo}\"&"
	              /**/ "$\"{:>42:foo}\"|Will append ${selected_object.operator str().rjust(42, \"foo\")} to the resulting ?. (s.a. ?#rjust)&"
	              /**/ "$\"{:^42:foo}\"|Will append ${selected_object.operator str().center(42, \"foo\")} to the resulting ?. (s.a. ?#center)&"
	              /**/ "$\"{:=42:foo}\"|Will append ${selected_object.operator str().zfill(42, \"foo\")} to the resulting ?. (s.a. ?#zfill)"
	              "}"),
	TYPE_METHOD_F("scanf", &string_scanf, METHOD_FCONSTCALL,
	              "(format:?.)->?S?O\n"
	              "#tValueError{The given @format is malformed}"
	              "#tValueError{Conversion to an integer failed}"

	              "Scan @this ?. using a scanf-like format string @format\n"

	              "No major changes have been made from C's scanf function, however regex-like range "
	              /**/ "expressions are supported, and the returned sequence ends as soon as either @format "
	              /**/ "or @this has been exhausted, or a miss-match has occurred\n"

	              "Scanf command blocks are structured as ${%[*][width]pattern}\n"

	              "Besides this, for convenience and better unicode integration, the following changes "
	              /**/ "have been made to C's regular scanf function:\n"

	              "#T{Format pattern|Yielded type|Description~"
	              "$\" \"|-|Skip any number of characters from input data for which ?#isspace returns ?t ($r\"\\s*\")&"
	              "$\"\\n\"|-|Skip any kind of line-feed, including $\"\\r\\n\", as well as any character for which ?#islf returns ?t ($r\"\\n\")&"
	              "$\"%o\"|?Dint|Match up to #Cwidth characters with $r\"[+-]*[0-7]+\" and yield the result as an octal integer&"
	              "$\"%d\"|?Dint|Match up to #Cwidth characters with $r\"[+-]*[0-9]+\" and yield the result as an decimal integer&"
	              "$\"%x\", $\"%p\"|?Dint|Match up to #Cwidth characters with $r\"[+-]*[0-9a-fA-F]+\" and yield the result as a hexadecimal integer&"
	              "$\"%i\", $\"%u\"|?Dint|Match up to #Cwidth characters with $r\"[+-]*(0([xX][0-9a-fA-F]+#|[bB][01]+)#|[0-9]+)\" and yield the result as an integer with automatic radix&"
	              "$\"%s\"|?.|Match up to `width' characters with $r\"\\S+\" and return them as a ?.&"
	              "$\"%c\"|?.|Consume exactly `width' (see above) or one characters and return them as a ?.&"
	              "$\"%[...]\"|?.|Similar to the regex (s.a. ?#rematch) range function (e.g. $\"%[^xyz]\", $\"%[abc]\", $\"%[a-z]\", $\"%[^\\]]\")"
	              "}\n"

	              "Integer-width modifiers ($\"h\", $\"hh\", $\"l\", $\"ll\", $\"j\", $\"z\", "
	              /**/ "$\"t\", $\"L\", $\"I\", $\"I8\", $\"I16\", $\"I32\" and $\"I64\") are ignored"),

	/* TODO: What about something like this?:
	 * >> print "Your name is $your_name, and I'm ${my_name}"
	 * >>       .substitute({ .your_name = "foo", .my_name = "bar" });
	 * >> print "You owe $guy $$10 dollars!"
	 * >>       .substitute({ .guy = "me" });
	 */

/* String/Character traits */
#define DEFINE_STRING_TRAIT_EX(name, func, doc)                                  \
	TYPE_METHOD_F(name, &func,                                                   \
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST |      \
	              METHOD_FNOREFESCAPE,                                           \
	              "->?Dbool\n"                                                   \
	              "(index:?Dint)->?Dbool\n"                                      \
	              "(start:?Dint,end:?Dint)->?Dbool\n"                            \
	              "#tIndexError{The given @index is larger than ${##this}}"      \
	              "#tIntegerOverflow{The given @index is negative or too large}" \
	              doc)
#define DEFINE_STRING_TRAIT(name, func, are_xxx) \
	DEFINE_STRING_TRAIT_EX(name, func, "Returns ?t if $this, ${this[index]}, or all characters in ${this.substr(start, end)} " are_xxx)
	DEFINE_STRING_TRAIT("iscntrl", string_iscntrl, "are control characters"),
	DEFINE_STRING_TRAIT("istab", string_istab, "are tabulator characters (#C{U+0009}, #C{U+000B}, #C{U+000C}, ...)"),
	DEFINE_STRING_TRAIT("iscempty", string_iscempty, "are tabulator (?#istab) or white-space (?#iswhite) characters (alias for ?#isspacexlf)"),
	DEFINE_STRING_TRAIT("iswhite", string_iswhite, "are white-space characters (#C{U+0020}, ...)"),
	DEFINE_STRING_TRAIT("islf", string_islf, "are line-feeds"),
	DEFINE_STRING_TRAIT("isspace", string_isspace, "are space-characters"),
	DEFINE_STRING_TRAIT("islower", string_islower, "are lower-case"),
	DEFINE_STRING_TRAIT("isupper", string_isupper, "are upper-case"),
	DEFINE_STRING_TRAIT("isalpha", string_isalpha, "are alphabetical"),
	DEFINE_STRING_TRAIT("isdigit", string_isdigit, "are digits"),
	DEFINE_STRING_TRAIT("ishex", string_ishex, "are a alphabetical hex-characters (${0x41-0x46}, ${0x61-0x66})"),
	DEFINE_STRING_TRAIT("isxdigit", string_isxdigit, "are digit (?#isdigit) or alphabetical hex-characters (?#ishex)"),
	DEFINE_STRING_TRAIT("isalnum", string_isalnum, "are alpha-numerical"),
	DEFINE_STRING_TRAIT("ispunct", string_ispunct, "are punctuational characters"),
	DEFINE_STRING_TRAIT("isgraph", string_isgraph, "are graphical characters"),
	DEFINE_STRING_TRAIT("isprint", string_isprint, "are printable"),
	DEFINE_STRING_TRAIT("isblank", string_isblank, "are blank"),
	DEFINE_STRING_TRAIT("isnumeric", string_isnumeric, "qualify as digits or otherwise numeric characters"),
	DEFINE_STRING_TRAIT("issymstrt", string_issymstrt, "can be used to start a symbol name"),
	DEFINE_STRING_TRAIT("issymcont", string_issymcont, "can be used to continue a symbol name"),
	DEFINE_STRING_TRAIT("isspacexlf", string_iscempty, "are space-characters, where linefeeds are not considered as spaces (IsSpaceeXcludingLineFeed) (alias for ?#iscempty)"),
	DEFINE_STRING_TRAIT("isascii", string_isascii, "are ascii-characters, that is have an ordinal value ${<= 0x7f}"),
	TYPE_METHOD_F("istitle", &string_istitle,
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST | METHOD_FNOREFESCAPE,
	              "(index:?Dint)->?Dbool\n"
	              "#tIndexError{The given @index is larger than ${?#this}}"
	              "#tIntegerOverflow{The given @index is negative or too large}"
	              "Returns ?t if the character at ${this[index]} has title-casing\n"

	              "\n"
	              "->?Dbool\n"
	              "(start:?Dint,end:?Dint)->?Dbool\n"
	              "Returns ?t if $this, or the sub-string ${this.substr(start, end)} "
	              /**/ "follows title-casing, meaning that space is followed by title-case, "
	              /**/ "with all remaining characters not being title-case"),
	TYPE_METHOD_F("issymbol", &string_issymbol,
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST | METHOD_FNOREFESCAPE,
	              "(index:?Dint)->?Dbool\n"
	              "#tIndexError{The given @index is larger than ${?#this}}"
	              "#tIntegerOverflow{The given @index is negative or too large}"
	              "Returns ?t if the character at ${this[index]} can be used "
	              /**/ "to start a symbol name. Same as ${this.issymstrt(index)}\n"

	              "\n"
	              "->?Dbool\n"
	              "(start:?Dint,end:?Dint)->?Dbool\n"
	              "Returns ?t if $this, or the sub-string ${this.substr(start, end)} "
	              /**/ "is a valid symbol name"),
#undef DEFINE_STRING_TRAIT

#define DEFINE_ANY_STRING_TRAIT_EX(name, func, doc)                           \
	TYPE_KWMETHOD_F(name, &func,                                              \
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST | \
	                METHOD_FNOREFESCAPE,                                      \
	                "(start=!0,end=!-1)->?Dbool\n" doc)
#define DEFINE_ANY_STRING_TRAIT(name, func, is_xxx) \
	DEFINE_ANY_STRING_TRAIT_EX(name, func, "Returns ?t if any character in ${this.substr(start, end)} " is_xxx)
	DEFINE_ANY_STRING_TRAIT("isanycntrl", string_isanycntrl, "is a control character"),
	DEFINE_ANY_STRING_TRAIT("isanytab", string_isanytab, "is a tabulator character ($0x09, $0x0B or $0x0C)"),
	DEFINE_ANY_STRING_TRAIT("isanycempty", string_isanycempty, "is a tabulator (?#istab) or white-space (?#iswhite) character (alias for ?#isanyspacexlf)"),
	DEFINE_ANY_STRING_TRAIT("isanywhite", string_isanywhite, "is a white-space character ($0x20)"),
	DEFINE_ANY_STRING_TRAIT("isanylf", string_isanylf, "is a line-feed"),
	DEFINE_ANY_STRING_TRAIT("isanyspace", string_isanyspace, "is a space character"),
	DEFINE_ANY_STRING_TRAIT("isanylower", string_isanylower, "is lower-case"),
	DEFINE_ANY_STRING_TRAIT("isanyupper", string_isanyupper, "is upper-case"),
	DEFINE_ANY_STRING_TRAIT("isanyalpha", string_isanyalpha, "is alphabetical"),
	DEFINE_ANY_STRING_TRAIT("isanydigit", string_isanydigit, "is a digit"),
	DEFINE_ANY_STRING_TRAIT("isanyhex", string_isanyhex, "is an alphabetical hex-character (${0x41-0x46}, ${0x61-0x66})"),
	DEFINE_ANY_STRING_TRAIT("isanyxdigit", string_isanyxdigit, "is a digit (?#isdigit) or an alphabetical hex-character (?#ishex)"),
	DEFINE_ANY_STRING_TRAIT("isanyalnum", string_isanyalnum, "is alpha-numerical"),
	DEFINE_ANY_STRING_TRAIT("isanypunct", string_isanypunct, "is a punctuational character"),
	DEFINE_ANY_STRING_TRAIT("isanygraph", string_isanygraph, "is a graphical character"),
	DEFINE_ANY_STRING_TRAIT("isanyprint", string_isanyprint, "is printable"),
	DEFINE_ANY_STRING_TRAIT("isanyblank", string_isanyblank, "is blank"),
	DEFINE_ANY_STRING_TRAIT("isanytitle", string_isanytitle, "has title-casing"),
	DEFINE_ANY_STRING_TRAIT("isanynumeric", string_isanynumeric, "qualifies as digit or some other numeric character"),
	DEFINE_ANY_STRING_TRAIT("isanysymstrt", string_isanysymstrt, "can be used to start a symbol name"),
	DEFINE_ANY_STRING_TRAIT("isanysymcont", string_isanysymcont, "can be used to continue a symbol name"),
	DEFINE_ANY_STRING_TRAIT("isanyspacexlf", string_isanycempty, "is a space character, where linefeeds are not considered as spaces (IsSpaceeXcludingLineFeed) (alias for ?#isanycempty)"),
	DEFINE_ANY_STRING_TRAIT("isanyascii", string_isanyascii, " is an ascii character, that is has an ordinal value ${<= 0x7f}"),
#undef DEFINE_ANY_STRING_TRAIT
#undef DEFINE_ANY_STRING_TRAIT_EX

	TYPE_METHOD_F("asnumeric", &string_asnumeric,
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST | METHOD_FNOREFESCAPE,
	              "->?X2?Dfloat?Dint\n"
	              "#tValueError{The string is longer than a single character}"
	              "(index:?Dint)->?X2?Dfloat?Dint\n"
	              "#tValueError{The character at @index isn't numeric}"
	              "(index:?Dint,defl:?Dint)->?X2?Dfloat?Dint\n"
	              "(index:?Dint,defl)->\n"
	              "#tIntegerOverflow{The given @index is negative or too large}"
	              "#tIndexError{The given @index is out of bounds}"
	              "Return the numeric value of the @index'th or only character of @this ?., "
	              /**/ "or throw a :ValueError or return @defl if that character isn't ?#isnumeric\n"
	              "${"
	              /**/ "print \"5\".isdigit();   /* true */\n"
	              /**/ "print \"5\".isnumeric(); /* true */\n"
	              /**/ "print \"5\".asnumeric(); /* 5 */\n"
	              /**/ "print \"\xC2\xB2\".isdigit();   /* false */\n"
	              /**/ "print \"\xC2\xB2\".isnumeric(); /* true */\n"
	              /**/ "print \"\xC2\xB2\".asnumeric(); /* 2 */"
	              "}"),
	TYPE_METHOD_F("asdigit", &string_asdigit,
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST | METHOD_FNOREFESCAPE,
	              "->?Dint\n"
	              "#tValueError{The string is longer than a single character}"
	              "(index:?Dint)->?Dint\n"
	              "#tValueError{The character at @index isn't numeric}"
	              "(index:?Dint,defl:?Dint)->?Dint\n"
	              "(index:?Dint,defl)->\n"
	              "#tIntegerOverflow{The given @index is negative or too large}"
	              "#tIndexError{The given @index is out of bounds}"
	              "Same as ?#asnumeric, but only succeed if the selected character matches ?#isdigit, rather than ?#isnumeric"),
	TYPE_METHOD_F("asxdigit", &string_asxdigit,
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST | METHOD_FNOREFESCAPE,
	              "->?Dint\n"
	              "#tValueError{The string is longer than a single character}"
	              "(index:?Dint)->?Dint\n"
	              "#tValueError{The character at @index isn't numeric}"
	              "(index:?Dint,defl:?Dint)->?Dint\n"
	              "(index:?Dint,defl)->\n"
	              "#tIntegerOverflow{The given @index is negative or too large}"
	              "#tIndexError{The given @index is out of bounds}"
	              "Same as ?#asdigit, but also accepts #C{a-f} and #C{A-F}"),

	/* String conversion */
	TYPE_KWMETHOD_F("lower", &string_lower,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST | METHOD_FNOREFESCAPE,
	                "(start=!0,end=!-1)->?.\n"
	                "Returns @this ?. converted to lower-case"),
	TYPE_KWMETHOD_F("upper", &string_upper,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST | METHOD_FNOREFESCAPE,
	                "(start=!0,end=!-1)->?.\n"
	                "Returns @this ?. converted to upper-case"),
	TYPE_KWMETHOD_F("title", &string_title,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST | METHOD_FNOREFESCAPE,
	                "(start=!0,end=!-1)->?.\n"
	                "Returns @this ?. converted to title-casing"),
	TYPE_KWMETHOD_F("capitalize", &string_capitalize,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST | METHOD_FNOREFESCAPE,
	                "(start=!0,end=!-1)->?.\n"
	                "Returns @this ?. with each word capitalized"),
	TYPE_KWMETHOD_F("swapcase", &string_swapcase,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST | METHOD_FNOREFESCAPE,
	                "(start=!0,end=!-1)->?.\n"
	                "Returns @this ?. with the casing of each "
	                /**/ "character that has two different casings swapped"),
	TYPE_KWMETHOD_F("casefold", &string_casefold,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST | METHOD_FNOREFESCAPE,
	                "(start=!0,end=!-1)->?.\n"
	                "Returns @this ?. with its casing folded.\n"

	                "The equivalent of the string returned by this function is what is "
	                /**/ "used by the case-insensitive string APIs, such as ?#casefind\n"

	                "The case folding algorithm implemented matches what "
	                /**/ "#A{unicode|https://www.w3.org/International/wiki/Case_folding} "
	                /**/ "describes as full case folding\n"

	                "At its core, case-folding a string is fairly similar to ?#lower, however "
	                /**/ "the differences start to appear when characters such as $\"\xC3\x9F\" are being "
	                /**/ "used, $\"\xC3\x9F\" being a german character that doesn't have a lower- or upper-case "
	                /**/ "variant, however in the absence of unicode support is often written as $\"ss\".\n"

	                "The obvious problem here is that this alternative representation uses 2 characters "
	                /**/ "where previously there was only one. ?#casefold solves this problem by replacing $\"\xC3\x9F\" "
	                /**/ "with $\"ss\", allowing functions such as ?#casecompare to indicate equal strings in "
	                /**/ "cases such as ${\"Stra\xc3\x9f"
	                "e\".casecompare(\"Strasse\") == 0}"),

	/* Case-sensitive query functions */
	TYPE_KWMETHOD_F(STR_replace, &string_replace,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST,
	                "(find:?.,replace:?.,max:?Dint=!A!Dint!PSIZE_MAX)->?.\n"
	                "Find up to @max occurrences of @find and replace each with @replace, then return the resulting ?."),
	TYPE_KWMETHOD_F("find", &string_find,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST | METHOD_FNOREFESCAPE,
	                "(needle:?.,start=!0,end=!-1)->?Dint\n"
	                "Find the first instance of @needle within ${this.substr(start, end)}, "
	                "and return its starting index, or ${-1} if no such position exists"),
	TYPE_KWMETHOD_F("rfind", &string_rfind,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST | METHOD_FNOREFESCAPE,
	                "(needle:?.,start=!0,end=!-1)->?Dint\n"
	                "Find the last instance of @needle within ${this.substr(start, end)}, "
	                "and return its starting index, or ${-1} if no such position exists"),
	TYPE_KWMETHOD_F(STR_index, &string_index,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST | METHOD_FNOREFESCAPE,
	                "(needle:?.,start=!0,end=!-1)->?Dint\n"
	                "#tIndexError{No instance of @needle can be found within ${this.substr(start, end)}}"
	                "Find the first instance of @needle within ${this.substr(start, end)}, "
	                "and return its starting index"),
	TYPE_KWMETHOD_F("rindex", &string_rindex,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST | METHOD_FNOREFESCAPE,
	                "(needle:?.,start=!0,end=!-1)->?Dint\n"
	                "#tIndexError{No instance of @needle can be found within ${this.substr(start, end)}}"
	                "Find the last instance of @needle within ${this.substr(start, end)}, "
	                "and return its starting index"),
	TYPE_KWMETHOD_F("findany", &string_findany,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST | METHOD_FNOREFESCAPE,
	                "(needles:?S?.,start=!0,end=!-1)->?X2?Dint?N\n"
	                "Find the first instance of any of the given @needles within ${this.substr(start, end)}, "
	                "and return its starting index, or ?N if no such position exists"),
	TYPE_KWMETHOD_F("rfindany", &string_rfindany,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST | METHOD_FNOREFESCAPE,
	                "(needles:?S?.,start=!0,end=!-1)->?X2?Dint?N\n"
	                "Find the last instance of any of the given @needles within ${this.substr(start, end)}, "
	                "and return its starting index, or ?N if no such position exists"),
	TYPE_KWMETHOD_F("indexany", &string_indexany,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST | METHOD_FNOREFESCAPE,
	                "(needles:?S?.,start=!0,end=!-1)->?Dint\n"
	                "#tIndexError{No instance of @needles can be found within ${this.substr(start, end)}}"
	                "Find the first instance of any of the given @needles within ${this.substr(start, end)}, "
	                "and return its starting index"),
	TYPE_KWMETHOD_F("rindexany", &string_rindexany,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST | METHOD_FNOREFESCAPE,
	                "(needles:?S?.,start=!0,end=!-1)->?Dint\n"
	                "#tIndexError{No instance of @needles can be found within ${this.substr(start, end)}}"
	                "Find the last instance of any of the given @needles within ${this.substr(start, end)}, "
	                "and return its starting index"),
	TYPE_KWMETHOD_F("findall", &string_findall,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST,
	                "(needle:?.,start=!0,end=!-1)->?S?Dint\n"
	                "Find all instances of @needle within ${this.substr(start, end)}, "
	                "and return their starting indices as a sequence"),
	TYPE_KWMETHOD_F("count", &string_count,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST | METHOD_FNOREFESCAPE,
	                "(needle:?.,start=!0,end=!-1)->?Dint\n"
	                "Count the number of instances of @needle that exist within ${this.substr(start, end)}, "
	                /**/ "and return now many were found"),
	TYPE_KWMETHOD_F("contains", &string_contains_f,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST | METHOD_FNOREFESCAPE,
	                "(needle:?.,start=!0,end=!-1)->?Dbool\n"
	                "Check if @needle can be found within ${this.substr(start, end)}, "
	                /**/ "and return a boolean indicative of that"),
	TYPE_KWMETHOD_F("substr", &string_substr,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST,
	                "(start=!0,end=!-1)->?.\n"
	                "Similar to ${this[start:end]}, however only integer-convertible objects may "
	                /**/ "be passed (passing ?N will invoke ${(int)none}, which results in $0), and "
	                /**/ "passing negative values for either @start or @end will cause ?ASIZE_MAX?Dint to "
	                /**/ "be used for that argument:\n"
	                "${"
	                /**/ "s = \"foo bar foobar\";\n"
	                /**/ "print repr s.substr(0, 1);    /* \"f\" */\n"
	                /**/ "print repr s[0:1];            /* \"f\" */\n"
	                /**/ "print repr s.substr(0, ##s);   /* \"foo bar foobar\" */\n"
	                /**/ "print repr s[0:##s];           /* \"foo bar foobar\" */\n"
	                /**/ "print repr s.substr(0, 1234); /* \"foo bar foobar\" */\n"
	                /**/ "print repr s[0:1234];         /* \"foo bar foobar\" */\n"
	                /**/ "print repr s.substr(0, -1);   /* \"foo bar foobar\" -- Negative indices intentionally underflow into positive infinity */\n"
	                /**/ "print repr s[0:-1];           /* \"foo bar fooba\" */"
	                "}\n"
	                "Also note that this way of interpreting integer indices is mirrored by all other "
	                /**/ "string functions that allow start/end-style arguments, including ?#find, ?#compare, "
	                /**/ "as well as many others"),
	TYPE_METHOD_F("strip", &string_strip,
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST,
	              "(mask?:?.)->?.\n"
	              "Strip all leading and trailing whitespace-characters, or "
	              /**/ "characters apart of @mask, and return the resulting ?."),
	TYPE_KWMETHOD_F("lstrip", &string_lstrip,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST,
	                "(mask?:?.)->?.\n"
	                "Strip all leading whitespace-characters, or "
	                /**/ "characters apart of @mask, and return the resulting ?."),
	TYPE_KWMETHOD_F("rstrip", &string_rstrip,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST,
	                "(mask?:?.)->?.\n"
	                "Strip all trailing whitespace-characters, or "
	                /**/ "characters apart of @mask, and return the resulting ?."),
	TYPE_METHOD_F("sstrip", &string_sstrip,
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST,
	              "(needle:?.)->?.\n"
	              "Strip all leading and trailing instances of @needle from @this ?.\n"
	              "${"
	              /**/ "function sstrip(needle: string): string {\n"
	              /**/ "	local result = this;\n"
	              /**/ "	while (result.startswith(needle))\n"
	              /**/ "		result = result[##needle:];\n"
	              /**/ "	while (result.endswith(needle))\n"
	              /**/ "		result = result[:##result - ##needle];\n"
	              /**/ "	return result;\n"
	              /**/ "}"
	              "}"),
	TYPE_KWMETHOD_F("lsstrip", &string_lsstrip,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST,
	                "(needle:?.)->?.\n"
	                "Strip all leading instances of @needle from @this ?.\n"
	                "${"
	                /**/ "function lsstrip(needle: string): string {\n"
	                /**/ "	local result = this;\n"
	                /**/ "	while (result.startswith(needle))\n"
	                /**/ "		result = result[##needle:];\n"
	                /**/ "	return result;\n"
	                /**/ "}"
	                "}"),
	TYPE_KWMETHOD_F("rsstrip", &string_rsstrip,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST,
	                "(needle:?.)->?.\n"
	                "Strip all trailing instances of @needle from @this ?.\n"
	                "${"
	                /**/ "function lsstrip(needle: string): string {\n"
	                /**/ "	local result = this;\n"
	                /**/ "	while (result.endswith(needle))\n"
	                /**/ "		result = result[:##result - ##needle];}\n"
	                /**/ "	return result;\n"
	                /**/ "}"
	                "}"),
	TYPE_METHOD_F("striplines", &string_striplines,
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST,
	              "(mask?:?.)->?.\n"
	              "Strip all whitspace, or @mask characters at the start, end, and before/after linefeeds\n"
	              "Note that for this purpose, linefeed characters don't count as whitespace\n"
	              "aka: strip all leading and trailing whitespace\n"
	              "Similar to ${\"\".join(this.splitlines(true).each.strip())}"),
	TYPE_METHOD_F("lstriplines", &string_lstriplines,
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST,
	              "(mask?:?.)->?.\n"
	              "Strip all whitspace, or @mask characters at the start, and after linefeeds\n"
	              "Note that for this purpose, linefeed characters don't count as whitespace\n"
	              "aka: strip all leading whitespace\n"
	              "Similar to ${\"\".join(this.splitlines(true).each.lstrip())}"),
	TYPE_METHOD_F("rstriplines", &string_rstriplines,
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST,
	              "(mask?:?.)->?.\n"
	              "Strip all whitspace, or @mask characters at the end, and before linefeeds\n"
	              "Note that for this purpose, linefeed characters don't count as whitespace\n"
	              "aka: strip all trailing whitespace\n"
	              "Similar to ${\"\".join(this.splitlines(true).each.rstrip())}"),
	TYPE_METHOD_F("sstriplines", &string_sstriplines,
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST,
	              "(needle:?.)->?.\n"
	              "Same as ?#striplines, but sequence for complete sequences of #needle, rather "
	              "than bytes apart of its $mask character."),
	TYPE_METHOD_F("lsstriplines", &string_lsstriplines,
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST,
	              "(needle:?.)->?.\n"
	              "Same as ?#lstriplines, but sequence for complete sequences of #needle, rather "
	              "than bytes apart of its $mask character."),
	TYPE_METHOD_F("rsstriplines", &string_rsstriplines,
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST,
	              "(needle:?.)->?.\n"
	              "Same as ?#rstriplines, but sequence for complete sequences of #needle, rather "
	              "than bytes apart of its $mask character."),
	TYPE_KWMETHOD_F("startswith", &string_startswith,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST | METHOD_FNOREFESCAPE,
	                "(needle:?.,start=!0,end=!-1)->?Dbool\n"
	                "Return ?t if the sub-string ${this.substr(start, end)} starts with @needle"),
	TYPE_KWMETHOD_F("endswith", &string_endswith,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST | METHOD_FNOREFESCAPE,
	                "(needle:?.,start=!0,end=!-1)->?Dbool\n"
	                "Return ?t if the sub-string ${this.substr(start, end)} ends with @needle"),
	TYPE_KWMETHOD_F("partition", &string_partition,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST,
	                "(needle:?.,start=!0,end=!-1)->?T3?.?.?.\n"
	                "Search for the first instance of @needle within ${this.substr(start, end)} and "
	                /**/ "return a 3-element sequence of strings ${(this[:pos], needle, this[pos + ##needle:])}.\n"
	                "If @needle could not be found, ${(this, \"\", \"\")} is returned"),
	TYPE_KWMETHOD_F("rpartition", &string_rpartition,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST,
	                "(needle:?.,start=!0,end=!-1)->?T3?.?.?.\n"
	                "Search for the last instance of @needle within ${this.substr(start, end)} and "
	                /**/ "return a 3-element sequence of strings ${(this[:pos], needle, this[pos + ##needle:])}.\n"
	                "If @needle could not be found, ${(this, \"\", \"\")} is returned"),
	TYPE_METHOD_F("compare", &string_compare,
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST | METHOD_FNOREFESCAPE,
	              "(other:?.,other_start=!0,other_end=!-1)->?Dint\n"
	              "(my_start:?Dint,other:?.,other_start=!0,other_end=!-1)->?Dint\n"
	              "(my_start:?Dint,my_end:?Dint,other:?.,other_start=!0,other_end=!-1)->?Dint\n"
	              "Compare the sub-string ${left = this.substr(my_start, my_end)} with "
	              "${right = other.substr(other_start, other_end)}, returning ${< 0} if "
	              "${left < right}, ${> 0} if ${left > right}, and ${== 0} if they are equal"),
	TYPE_METHOD_F("vercompare", &string_vercompare,
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST | METHOD_FNOREFESCAPE,
	              "(other:?.,other_start=!0,other_end=!-1)->?Dint\n"
	              "(my_start:?Dint,other:?.,other_start=!0,other_end=!-1)->?Dint\n"
	              "(my_start:?Dint,my_end:?Dint,other:?.,other_start=!0,other_end=!-1)->?Dint\n"
	              "Performs a version-string comparison. This is similar to ?#compare, but rather than "
	              /**/ "performing a strict lexicographical comparison, the numbers found in the strings "
	              /**/ "being compared are compared as a whole, solving the common problem seen in applications "
	              /**/ "such as file navigators showing a file order of #C{foo1.txt}, #C{foo10.txt}, #C{foo11.txt}, "
	              /**/ "#C{foo2.txt}, etc...\n"

	              "This function is a portable implementation of the GNU function "
	              /**/ "#A{strverscmp|https://linux.die.net/man/3/strverscmp}, "
	              /**/ "for which you may follow the link for further details"),
	TYPE_METHOD_F("wildcompare", &string_wildcompare,
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST | METHOD_FNOREFESCAPE,
	              "(pattern:?.,pattern_start=!0,pattern_end=!-1)->?Dint\n"
	              "(my_start:?Dint,pattern:?.,pattern_start=!0,pattern_end=!-1)->?Dint\n"
	              "(my_start:?Dint,my_end:?Dint,pattern:?.,pattern_start=!0,pattern_end=!-1)->?Dint\n"
	              "Perform a wild-character-enabled comparising of the sub-string ${left = this.substr(my_start, my_end)} "
	              /**/ "with ${right = pattern.substr(pattern_start, pattern_end)}, returning ${< 0} if ${left < right}, ${> 0} "
	              /**/ "if ${left > right}, or ${== 0} if they are equal\n"
	              "Wild-compare characters are only parsed from @pattern, allowing $\"?\" to "
	              /**/ "be matched with any single character from @this, and $\"*\" to be matched to "
	              /**/ "any number of characters"),
	TYPE_METHOD_F("fuzzycompare", &string_fuzzycompare,
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST | METHOD_FNOREFESCAPE,
	              "(other:?.,other_start=!0,other_end=!-1)->?Dint\n"
	              "(my_start:?Dint,other:?.,other_start=!0,other_end=!-1)->?Dint\n"
	              "(my_start:?Dint,my_end:?Dint,other:?.,other_start=!0,other_end=!-1)->?Dint\n"
	              "Perform a fuzzy string comparison between ${this.substr(my_start, my_end)} and "
	              /**/ "${other.substr(other_start, other_end)}\n"
	              "The return value is a similarty-factor that can be used to score how close the "
	              /**/ "two strings look alike.\n"
	              "How exactly the scoring is done is implementation-specific, however a score of $0 "
	              /**/ "is reserved for two strings that are perfectly identical, any two differing "
	              /**/ "strings always have a score ${> 0}, and the closer the score is to $0, the "
	              /**/ "more alike they are\n"
	              "The intended use of this function is for auto-completion, as well as warning "
	              /**/ "messages and recommendations in the sense of I-dont-know-foo-but-did-you-mean-bar\n"
	              "Note that there is another version ?#casefuzzycompare that also ignores casing"),
	TYPE_METHOD_F("wmatch", &string_wmatch,
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST | METHOD_FNOREFESCAPE,
	              "(pattern:?.,other_start=!0,other_end=!-1)->?Dbool\n"
	              "(my_start:?Dint,pattern:?.,other_start=!0,other_end=!-1)->?Dbool\n"
	              "(my_start:?Dint,my_end:?Dint,pattern:?.,other_start=!0,other_end=!-1)->?Dbool\n"
	              "Same as ?#wildcompare, returning ?t where ?#wildcompare would return $0, "
	              /**/ "and ?f in all pattern cases"),

	/* Case-insensitive query functions */
	TYPE_KWMETHOD_F("casereplace", &string_casereplace,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST,
	                "(find:?.,replace:?.,max:?Dint=!A!Dint!PSIZE_MAX)->?Dint\n"
	                "Same as ?#replace, however perform a case-folded search (s.a. ?#casefold)"),
	TYPE_KWMETHOD_F("casefind", &string_casefind,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST | METHOD_FNOREFESCAPE,
	                "(needle:?.,start=!0,end=!-1)->?X2?T2?Dint?Dint?N\n"
	                "Same as ?#find, however perform a case-folded search and return the start and end "
	                /**/ "indices of the match (s.a. ?#casefold)\n"
	                "If no match if found, ?N is returned"),
	TYPE_KWMETHOD_F("caserfind", &string_caserfind,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST | METHOD_FNOREFESCAPE,
	                "(needle:?.,start=!0,end=!-1)->?X2?T2?Dint?Dint?N\n"
	                "Same as ?#rfind, however perform a case-folded search and return the start and end "
	                /**/ "indices of the match (s.a. ?#casefold)\n"
	                "If no match if found, ?N is returned"),
	TYPE_KWMETHOD_F("caseindex", &string_caseindex,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST | METHOD_FNOREFESCAPE,
	                "(needle:?.,start=!0,end=!-1)->?T2?Dint?Dint\n"
	                "Same as ?#index, however perform a case-folded search and return the start and end "
	                /**/ "indices of the match (s.a. ?#casefold)"),
	TYPE_KWMETHOD_F("caserindex", &string_caserindex,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST | METHOD_FNOREFESCAPE,
	                "(needle:?.,start=!0,end=!-1)->?T2?Dint?Dint\n"
	                "Same as ?#rindex, however perform a case-folded search and return the start and end "
	                /**/ "indices of the match (s.a. ?#casefold)"),
	TYPE_KWMETHOD_F("casefindany", &string_casefindany,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST | METHOD_FNOREFESCAPE,
	                "(needles:?S?.,start=!0,end=!-1)->?X2?T2?Dint?Dint?N\n"
	                "Same as ?#findany, however perform a case-folded search and return the start and end "
	                /**/ "indices of the match (s.a. ?#casefold)\n"
	                "If no match if found, ?N is returned"),
	TYPE_KWMETHOD_F("caserfindany", &string_caserfindany,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST | METHOD_FNOREFESCAPE,
	                "(needles:?S?.,start=!0,end=!-1)->?X2?T2?Dint?Dint?N\n"
	                "Same as ?#rfindany, however perform a case-folded search and return the start and end "
	                /**/ "indices of the match (s.a. ?#casefold)\n"
	                "If no match if found, ?N is returned"),
	TYPE_KWMETHOD_F("caseindexany", &string_caseindexany,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST | METHOD_FNOREFESCAPE,
	                "(needles:?S?.,start=!0,end=!-1)->?T2?Dint?Dint\n"
	                "Same as ?#indexany, however perform a case-folded search and return the start and end "
	                /**/ "indices of the match (s.a. ?#casefold)"),
	TYPE_KWMETHOD_F("caserindexany", &string_caserindexany,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST | METHOD_FNOREFESCAPE,
	                "(needles:?S?.,start=!0,end=!-1)->?T2?Dint?Dint\n"
	                "Same as ?#rindexany, however perform a case-folded search and return the start and end "
	                /**/ "indices of the match (s.a. ?#casefold)"),
	TYPE_KWMETHOD_F("casefindall", &string_casefindall,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST,
	                "(needle:?.,start=!0,end=!-1)->?S?T2?Dint?Dint\n"
	                "Same as ?#findall, however perform a case-folded search and return the star and end "
	                /**/ "indices of matches (s.a. ?#casefold)"),
	TYPE_KWMETHOD_F("casecount", &string_casecount,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST | METHOD_FNOREFESCAPE,
	                "(needle:?.,start=!0,end=!-1)->?Dint\n"
	                "Same as ?#count, however perform a case-folded search (s.a. ?#casefold)"),
	TYPE_KWMETHOD_F("casecontains", &string_casecontains_f,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST | METHOD_FNOREFESCAPE,
	                "(needle:?.,start=!0,end=!-1)->?Dbool\n"
	                "Same as ?#contains, however perform a case-folded search (s.a. ?#casefold)"),
	TYPE_METHOD_F("casestrip", &string_casestrip,
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST,
	              "(mask?:?.)->?.\n"
	              "Same as ?#strip, however perform a case-folded search when @mask is given (s.a. ?#casefold)"),
	TYPE_KWMETHOD_F("caselstrip", &string_caselstrip,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST,
	                "(mask?:?.)->?.\n"
	                "Same as ?#lstrip, however perform a case-folded search when @mask is given (s.a. ?#casefold)"),
	TYPE_KWMETHOD_F("caserstrip", &string_caserstrip,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST,
	                "(mask?:?.)->?.\n"
	                "Same as ?#rstrip, however perform a case-folded search when @mask is given (s.a. ?#casefold)"),
	TYPE_METHOD_F("casesstrip", &string_casesstrip,
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST,
	              "(needle:?.)->?.\n"
	              "Same as ?#sstrip, however perform a case-folded search (s.a. ?#casefold)"),
	TYPE_KWMETHOD_F("caselsstrip", &string_caselsstrip,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST,
	                "(needle:?.)->?.\n"
	                "Same as ?#lsstrip, however perform a case-folded search (s.a. ?#casefold)"),
	TYPE_KWMETHOD_F("casersstrip", &string_casersstrip,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST,
	                "(needle:?.)->?.\n"
	                "Same as ?#rsstrip, however perform a case-folded search (s.a. ?#casefold)"),
	TYPE_METHOD_F("casestriplines", &string_casestriplines,
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST,
	              "(mask?:?.)->?.\n"
	              "Same as ?#striplines, however perform a case-folded search when @mask is given (s.a. ?#casefold)"),
	TYPE_METHOD_F("caselstriplines", &string_caselstriplines,
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST,
	              "(mask?:?.)->?.\n"
	              "Same as ?#lstriplines, however perform a case-folded search when @mask is given (s.a. ?#casefold)"),
	TYPE_METHOD_F("caserstriplines", &string_caserstriplines,
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST,
	              "(mask?:?.)->?.\n"
	              "Same as ?#rstriplines, however perform a case-folded search when @mask is given (s.a. ?#casefold)"),
	TYPE_METHOD_F("casesstriplines", &string_casesstriplines,
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST,
	              "(needle:?.)->?.\n"
	              "Same as ?#sstriplines, however perform a case-folded search (s.a. ?#casefold)"),
	TYPE_METHOD_F("caselsstriplines", &string_caselsstriplines,
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST,
	              "(needle:?.)->?.\n"
	              "Same as ?#lsstriplines, however perform a case-folded search (s.a. ?#casefold)"),
	TYPE_METHOD_F("casersstriplines", &string_casersstriplines,
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST,
	              "(needle:?.)->?.\n"
	              "Same as ?#rsstriplines, however perform a case-folded search (s.a. ?#casefold)"),
	TYPE_KWMETHOD_F("casestartswith", &string_casestartswith,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST | METHOD_FNOREFESCAPE,
	                "(needle:?.,start=!0,end=!-1)->?Dbool\n"
	                "Same as ?#startswith, however perform a case-folded search (s.a. ?#casefold)"),
	TYPE_KWMETHOD_F("caseendswith", &string_caseendswith,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST | METHOD_FNOREFESCAPE,
	                "(needle:?.,start=!0,end=!-1)->?Dbool\n"
	                "Same as ?#endswith, however perform a case-folded search (s.a. ?#casefold)"),
	TYPE_KWMETHOD_F("casepartition", &string_casepartition,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST,
	                "(needle:?.,start=!0,end=!-1)->?T3?.?.?.\n"
	                "Same as ?#partition, however perform a case-folded search (s.a. ?#casefold)"),
	TYPE_KWMETHOD_F("caserpartition", &string_caserpartition,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST,
	                "(needle:?.,start=!0,end=!-1)->?T3?.?.?.\n"
	                "Same as ?#rpartition, however perform a case-folded search (s.a. ?#casefold)"),
	TYPE_METHOD_F("casecompare", &string_casecompare,
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST | METHOD_FNOREFESCAPE,
	              "(other:?.,other_start=!0,other_end=!-1)->?Dint\n"
	              "(my_start:?Dint,other:?.,other_start=!0,other_end=!-1)->?Dint\n"
	              "(my_start:?Dint,my_end:?Dint,other:?.,other_start=!0,other_end=!-1)->?Dint\n"
	              "Same as ?#compare, however compare strings with their casing folded (s.a. ?#casefold)"),
	TYPE_METHOD_F("casevercompare", &string_casevercompare,
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST | METHOD_FNOREFESCAPE,
	              "(other:?.,other_start=!0,other_end=!-1)->?Dint\n"
	              "(my_start:?Dint,other:?.,other_start=!0,other_end=!-1)->?Dint\n"
	              "(my_start:?Dint,my_end:?Dint,other:?.,other_start=!0,other_end=!-1)->?Dint\n"
	              "Same as ?#vercompare, however compare strings with their casing folded (s.a. ?#casefold)"),
	TYPE_METHOD_F("casewildcompare", &string_casewildcompare,
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST | METHOD_FNOREFESCAPE,
	              "(pattern:?.,pattern_start=!0,pattern_end=!-1)->?Dint\n"
	              "(my_start:?Dint,pattern:?.,pattern_start=!0,pattern_end=!-1)->?Dint\n"
	              "(my_start:?Dint,my_end:?Dint,pattern:?.,pattern_start=!0,pattern_end=!-1)->?Dint\n"
	              "Same as ?#wildcompare, however compare strings with their casing folded (s.a. ?#casefold)"),
	TYPE_METHOD_F("casefuzzycompare", &string_casefuzzycompare,
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST | METHOD_FNOREFESCAPE,
	              "(other:?.,other_start=!0,other_end=!-1)->?Dint\n"
	              "(my_start:?Dint,other:?.,other_start=!0,other_end=!-1)->?Dint\n"
	              "(my_start:?Dint,my_end:?Dint,other:?.,other_start=!0,other_end=!-1)->?Dint\n"
	              "Same as ?#fuzzycompare, however compare strings with their casing folded (s.a. ?#casefold)"),
	TYPE_METHOD_F("casewmatch", &string_casewmatch,
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST | METHOD_FNOREFESCAPE,
	              "(pattern:?.,pattern_start=!0,pattern_end=!-1)->?Dbool\n"
	              "(my_start:?Dint,pattern:?.,pattern_start=!0,pattern_end=!-1)->?Dbool\n"
	              "(my_start:?Dint,my_end:?Dint,pattern:?.,pattern_start=!0,pattern_end=!-1)->?Dbool\n"
	              "Same as ?#wmatch, however compare strings with their casing folded (s.a. ?#casefold)"),

	/* String alignment functions. */
	TYPE_METHOD_F("center", &string_center,
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST,
	              "(width:?Dint,filler=!P{ })->?.\n"
	              "Use @this ?. as result, then evenly insert @filler at "
	              /**/ "the front and back to pad its length to @width characters"),
	TYPE_METHOD_F("ljust", &string_ljust,
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST,
	              "(width:?Dint,filler=!P{ })->?.\n"
	              "Use @this ?. as result, then insert @filler "
	              /**/ "at the back to pad its length to @width characters"),
	TYPE_METHOD_F("rjust", &string_rjust,
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST,
	              "(width:?Dint,filler=!P{ })->?.\n"
	              "Use @this ?. as result, then insert @filler "
	              /**/ "at the front to pad its length to @width characters"),
	TYPE_METHOD_F("zfill", &string_zfill,
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST,
	              "(width:?Dint,filler=!P{0})->?.\n"
	              "Skip leading ${\'+\'} and ${\'-\'} characters, then insert @filler "
	              /**/ "to pad the resulting ?. to a length of @width characters"),
	TYPE_KWMETHOD_F("reversed", &string_reversed,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST | METHOD_FNOREFESCAPE,
	                "(start=!0,end=!-1)->?.\n"
	                "Return the sub-string ${this.substr(start, end)} with its character order reversed"),
	TYPE_METHOD_F("expandtabs", &string_expandtabs,
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST,
	              "(tabwidth=!8)->?.\n"
	              "Expand tab characters with whitespace offset from the start "
	              /**/ "of their respective line at multiples of @tabwidth"),
	TYPE_METHOD_F("unifylines", &string_unifylines,
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST,
	              "(replacement=!P{\\\n})->?.\n"
	              "Unify all linefeed character sequences found in @this ?. to "
	              /**/ "make exclusive use of @replacement"),

	/* String -- sequence interaction. */
	TYPE_METHOD_F("join", &string_join,
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGSELEM_CONSTSTR_ROBYTES | METHOD_FNOREFESCAPE,
	              "(seq:?S?O)->?.\n"
	              "Iterate @seq and convert all items into strings, inserting @this "
	              /**/ "?. before each element, starting only with the second"),
	TYPE_METHOD_F("split", &string_split,
	              METHOD_FCONSTCALL,
	              "(sep:?.)->?S?.\n"
	              "Split @this ?. at each instance of @sep, returning a "
	              /**/ "sequence of the resulting parts"),
	TYPE_METHOD_F("casesplit", &string_casesplit,
	              METHOD_FCONSTCALL,
	              "(sep:?.)->?S?.\n"
	              "Same as ?#split, however perform a case-folded search"),
	TYPE_METHOD_F("splitlines", &string_splitlines,
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST,
	              "(keepends=!f)->?S?.\n"
	              "Split @this ?. at each linefeed, returning a sequence of all contained lines\n"
	              "When @keepends is ?f, this is identical to ${this.unifylines().split(\"\\n\")}\n"
	              "When @keepends is ?t, items found in the returned sequence will still have their "
	              /**/ "original, trailing line-feed appended"),

	/* String indentation. */
	TYPE_METHOD_F("indent", &string_indent,
	              METHOD_FCONSTCALL,
	              "(filler=!P{\t})->?.\n"
	              "Using @this ?. as result, insert @filler at the front, as well as after "
	              /**/ "every linefeed with the exception of one that may be located at its end\n"
	              "The intended use is for generating strings from structured data, such as HTML:\n"
	              "${"
	              /**/ "text = getHtmlText();\n"
	              /**/ "text = f\"<html>\n{text.strip().indent()}\n</html>\";"
	              "}"),
	TYPE_METHOD_F("dedent", &string_dedent,
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST,
	              "(max_chars=!1,mask?:?.)->?.\n"
	              "Using @this ?. as result, remove up to @max_chars whitespace "
	              /**/ "(s.a. ?#isspace) characters, or if given: characters apart of @mask "
	              /**/ "from the front, as well as following any linefeed"),

	/* Common-character search functions. */
	TYPE_METHOD_F("common", &string_common,
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST | METHOD_FNOREFESCAPE,
	              "(other:?.,other_start=!0,other_end=!-1)->?Dint\n"
	              "(my_start:?Dint,other:?.,other_start=!0,other_end=!-1)->?Dint\n"
	              "(my_start:?Dint,my_end:?Dint,other:?.,other_start=!0,other_end=!-1)->?Dint\n"
	              "Returns the number of common leading characters shared between @this and @other, "
	              /**/ "or in other words: the lowest index $i for which ${this[i] != other[i]} is true"),
	TYPE_METHOD_F("rcommon", &string_rcommon,
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST | METHOD_FNOREFESCAPE,
	              "(other:?.,other_start=!0,other_end=!-1)->?Dint\n"
	              "(my_start:?Dint,other:?.,other_start=!0,other_end=!-1)->?Dint\n"
	              "(my_start:?Dint,my_end:?Dint,other:?.,other_start=!0,other_end=!-1)->?Dint\n"
	              "Returns the number of common trailing characters shared between @this and @other"),
	TYPE_METHOD_F("casecommon", &string_casecommon,
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST | METHOD_FNOREFESCAPE,
	              "(other:?.,other_start=!0,other_end=!-1)->?Dint\n"
	              "(my_start:?Dint,other:?.,other_start=!0,other_end=!-1)->?Dint\n"
	              "(my_start:?Dint,my_end:?Dint,other:?.,other_start=!0,other_end=!-1)->?Dint\n"
	              "Same as ?#common, however perform a case-folded search"),
	TYPE_METHOD_F("casercommon", &string_casercommon,
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST | METHOD_FNOREFESCAPE,
	              "(other:?.,other_start=!0,other_end=!-1)->?Dint\n"
	              "(my_start:?Dint,other:?.,other_start=!0,other_end=!-1)->?Dint\n"
	              "(my_start:?Dint,my_end:?Dint,other:?.,other_start=!0,other_end=!-1)->?Dint\n"
	              "Same as ?#rcommon, however perform a case-folded search"),

	/* Find match character sequences */
	TYPE_METHOD_F("findmatch", &string_findmatch,
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST | METHOD_FNOREFESCAPE,
	              "(open:?.,close:?.,start=!0,end=!-1)->?Dint\n"
	              "Similar to ?#find, but do a recursive search for the "
	              /**/ "first @close that doesn't have a match @{open}:\n"
	              "${"
	              /**/ "s = \"foo(bar(), baz(42), 7).strip()\";\n"
	              /**/ "local lcol = s.find(\"(\");\n"
	              /**/ "print lcol; /* 3 */\n"
	              /**/ "local mtch = s.findmatch(\"(\", \")\", lcol + 1);\n"
	              /**/ "print repr s[lcol:mtch+1]; /* \"(bar(), baz(42), 7)\" */"
	              "}\n"
	              "If no @close without a matching @open exists, $-1 is returned\n"
	              "Note that @open and @close are not restricted to single-character "
	              /**/ "strings, are allowed to be of any length"),
	TYPE_METHOD_F("indexmatch", &string_indexmatch,
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST | METHOD_FNOREFESCAPE,
	              "(open:?.,close:?.,start=!0,end=!-1)->?Dint\n"
	              "#tIndexError{No instance of @close without a matching @open "
	              /*             */ "exists within ${this.substr(start, end)}}"
	              "Same as ?#findmatch, but throw an :IndexError instead of "
	              /**/ "returning ${-1} if no @close without a matching @open exists"),
	TYPE_METHOD_F("casefindmatch", &string_casefindmatch,
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST | METHOD_FNOREFESCAPE,
	              "(open:?.,close:?.,start=!0,end=!-1)->?X2?T2?Dint?Dint?N\n"
	              "Same as ?#findmatch, however perform a case-folded search and "
	              /**/ "return the start and end indices of the match\n"
	              "If no match if found, ?N is returned"),
	TYPE_METHOD_F("caseindexmatch", &string_caseindexmatch,
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST | METHOD_FNOREFESCAPE,
	              "(open:?.,close:?.,start=!0,end=!-1)->?T2?Dint?Dint\n"
	              "#tIndexError{No instance of @close without a matching @open "
	              /*             */ "exists within ${this.substr(start, end)}}"
	              "Same as ?#indexmatch, however perform a case-folded search and "
	              /**/ "return the start and end indices of the match"),
	TYPE_METHOD_F("rfindmatch", &string_rfindmatch,
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST | METHOD_FNOREFESCAPE,
	              "(open:?.,close:?.,start=!0,end=!-1)->?Dint\n"
	              "Similar to ?#findmatch, but operate in a mirrored fashion, "
	              /**/ "searching for the last instance of @open that has no match "
	              /**/ "@close within ${this.substr(start, end)}:\n"
	              "${"
	              /**/ "s = \"get_string().foo(bar(), baz(42), 7).length\";\n"
	              /**/ "lcol = s.find(\")\");\n"
	              /**/ "print lcol; /* 19 */\n"
	              /**/ "mtch = s.rfindmatch(\"(\", \")\", 0, lcol);\n"
	              /**/ "print repr s[mtch:lcol + 1]; /* \"(bar(), baz(42), 7)\" */"
	              "}\n"
	              "If no @open without a matching @close exists, ${-1} is returned"),
	TYPE_METHOD_F("rindexmatch", &string_rindexmatch,
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST | METHOD_FNOREFESCAPE,
	              "(open:?.,close:?.,start=!0,end=!-1)->?Dint\n"
	              "#tIndexError{No instance of @open without a matching @close "
	              /*             */ "exists within ${this.substr(start, end)}}"
	              "Same as ?#rfindmatch, but throw an :IndexError instead of "
	              /**/ "returning ${-1} if no @open without a matching @close exists"),
	TYPE_METHOD_F("caserfindmatch", &string_caserfindmatch,
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST | METHOD_FNOREFESCAPE,
	              "(open:?.,close:?.,start=!0,end=!-1)->?X2?T2?Dint?Dint?N\n"
	              "Same as ?#rfindmatch, however perform a case-folded search and "
	              /**/ "return the start and end indices of the match\n"
	              "If no match if found, ?N is returned"),
	TYPE_METHOD_F("caserindexmatch", &string_caserindexmatch,
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST | METHOD_FNOREFESCAPE,
	              "(open:?.,close:?.,start=!0,end=!-1)->?T2?Dint?Dint\n"
	              "#tIndexError{No instance of @open without a matching @close exists "
	              /**/ "within ${this.substr(start, end)}}"
	              "Same as ?#rindexmatch, however perform a case-folded search and return "
	              /**/ "the start and end indices of the match"),

	/* Using the find-match functionality, also provide a partitioning version */
	TYPE_METHOD_F("partitionmatch", &string_partitionmatch,
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST,
	              "(open:?.,close:?.,start=!0,end=!-1)->?T3?.?.?.\n"
	              "A hybrid between ?#find, ?#findmatch and ?#partition that returns the strings surrounding "
	              "the matched string portion, the first being the substring prior to the match, "
	              "the second being the matched ?. itself (including the @open and @close strings), "
	              "and the third being the substring after the match:\n"
	              "${"
	              /**/ "s = \"foo { x, y, { 13, 19, 42, { } }, w } -- tail {}\";\n"
	              /**/ "/* { \"foo \", \"{ x, y, { 13, 19, 42, { } }, w }\", \" -- tail {}\" } */\n"
	              /**/ "print repr s.partitionmatch(\"{\", \"}\");"
	              "}\n"
	              "if no matching @open + @close pair could be found, ${(this[start:end], \"\", \"\")} is returned\n"
	              "${\n"
	              /**/ "function partitionmatch(open: string, close: string, start: int = 0, end: int = -1) {\n"
	              /**/ "	local j;\n"
	              /**/ "	local i = this.find(open, start, end);\n"
	              /**/ "	if (i < 0 || (j = this.findmatch(open, close, i + #open, end)) < 0)\n"
	              /**/ "		return (this.substr(start, end), \"\", \"\");\n"
	              /**/ "	return (\n"
	              /**/ "		this.substr(start, i),\n"
	              /**/ "		this.substr(i, j + #close),\n"
	              /**/ "		this.substr(j + #close, end)\n"
	              /**/ "	);\n"
	              /**/ "}"
	              "}"),
	TYPE_METHOD_F("rpartitionmatch", &string_rpartitionmatch,
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST,
	              "(open:?.,close:?.,start=!0,end=!-1)->?T3?.?.?.\n"
	              "A hybrid between ?#rfind, ?#rfindmatch and ?#rpartition that returns the strings surrounding "
	              /**/ "the matched string portion, the first being the substring prior to the match, "
	              /**/ "the second being the matched ?. itself (including the @open and @close strings), "
	              /**/ "and the third being the substring after the match:\n"
	              "${"
	              /**/ "s = \"{ } foo { x, y, { 13, 19, 42, { } }, w } -- tail\";\n"
	              /**/ "/* { \"{ } foo \", \"{ x, y, { 13, 19, 42, { } }, w }\", \" -- tail\" } */\n"
	              /**/ "print repr s.rpartitionmatch(\"{\", \"}\");"
	              "}\n"
	              "If no matching @open + @close pair could be found, ${(this[start:end], \"\", \"\")} is returned\n"
	              "${"
	              /**/ "function rpartitionmatch(open: string, close: string, start: int = 0, end: int = -1) {\n"
	              /**/ "	local i;\n"
	              /**/ "	local j = this.rfind(close, start, end);\n"
	              /**/ "	if (j < 0 || (i = this.rfindmatch(open, close, start, j)) < 0)\n"
	              /**/ "		return (this.substr(start, end), \"\", \"\");\n"
	              /**/ "	return (\n"
	              /**/ "		this.substr(start, i),\n"
	              /**/ "		this.substr(i, j + #close),\n"
	              /**/ "		this.substr(j + #close, end)\n"
	              /**/ "	);\n"
	              /**/ "}"
	              "}"),
	TYPE_METHOD_F("casepartitionmatch", &string_casepartitionmatch,
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST,
	              "(open:?.,close:?.,start=!0,end=!-1)->?T3?.?.?.\n"
	              "Same as #partitionmatch, however perform a case-folded search"),
	TYPE_METHOD_F("caserpartitionmatch", &string_caserpartitionmatch,
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST,
	              "(open:?.,close:?.,start=!0,end=!-1)->?T3?.?.?.\n"
	              "Same as #rpartitionmatch, however perform a case-folded search"),

	TYPE_METHOD_F("segments", &string_segments,
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST,
	              "(substring_length:?Dint)->?S?.\n"
	              "Split @this ?. into segments, each exactly @substring_length characters long, with the "
	              /**/ "last segment containing the remaining characters and having a length of between "
	              /**/ "$1 and @substring_length characters.\n"
	              "This function is similar to ?#distribute, but instead of being given the "
	              /**/ "amount of sub-strings and figuring out their lengths, this function takes "
	              /**/ "the length of sub-strings and figures out their amount"),
	TYPE_METHOD_F("distribute", &string_distribute,
	              METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST,
	              "(substring_count:?Dint)->?S?.\n"
	              "Split @this ?. into @substring_count similarly sized sub-strings, each with a "
	              /**/ "length of ${(##this + (substring_count - 1)) / substring_count}, followed by a last, optional "
	              /**/ "sub-string containing all remaining characters.\n"
	              "This function is similar to ?#segments, but instead of being given the "
	              /**/ "length of sub-strings and figuring out their amount, this function takes "
	              /**/ "the amount of sub-strings and figures out their lengths"),

	/* Regex functions. */
	TYPE_KWMETHOD_F("rematch", &string_rematch,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST | METHOD_FNOREFESCAPE,
	                "(pattern:?.,start=!0,end=!-1,rules=!P{})->?X2?Dint?N\n"
	                "#tValueError{The given @pattern is malformed}"
	                "#r{The number of leading characters in ${this.substr(start, end)} "
	                /*    */ "matched by @pattern, or ?N if @pattern doesn't match}"
	                "Check if ${this.substr(start, end)} matches the given regular expression @pattern\n"
	                "When given, @rules specifies extra compilation rules for the regex. These take the form of a tightly "
	                /**/ "packed string, where each character represents a regex flag. The following characters are defined:"
	                "#T{Character|Effect~"
	                /**/ "$\"i\"|Perform a case-insensitive regex match. Note that due to limitations, this sort of match may "
	                /**/ /*  */ "be different from (e.g.) ?#casecompare, in that neither the @pattern, nor @this string are "
	                /**/ /*  */ "case-folded prior to being compared to each other (as is done by ?#casecompare; s.a. ?#casefold)."
	                /**/ /*  */ "Instead, case-insensitive compare only takes into account different character representations "
	                /**/ /*  */ "that all consist of the same number of actual characters (e.g. #Ca and #CA, but not #C{ß} and #Css)&"
	                /**/ "...|Illegal character cause an error :ValueError being thrown"
	                "}\n"
	                "Note that in deemon, regex patterns are compiled lazily and stored alongside the given @pattern string "
	                /**/ "itself. As such, regular code that simply hard-codes the regular expressions it uses will execute "
	                /**/ "slowly only the first time around. However, be careful not to write code where the used patterns "
	                /**/ "are derived from other objects every time they are used (e.g. sub-strings), or decoded from bytes. "
	                /**/ "To prevent regex patterns from constantly needing to be re-compiled, make sure to store the pattern "
	                /**/ "strings with duration that spans across all use instances (constant string literals share their "
	                /**/ "storage duration with that of the surrounding module and are thus the perfect candidate, meaning "
	                /**/ "that hard-coded regex pattern strings are always optimal).\n"
	                "Supported Match expressions:"
	                "#T{Expression|Description~"
	                /**/ "#C{XY}|Match #CX followed by #CY&"
	                /**/ "#C{text}|Match the literal $\"text\"&"
	                /**/ "#C{.}|Matches anything&"
	                /**/ "#C{[CHARSET]}|Charset matching (see below)&"
	                /**/ "#C{[^CHARSET]}|Negated charset matching (see below)&"
	                /**/ "#C{\\w}|Alias for #C{[[:symcont:]]}&"
	                /**/ "#C{\\W}|Alias for #C{[^[:symcont:]]}&"
	                /**/ "#C{\\n}|Alias for #C{[[:lf:]]}&"
	                /**/ "#C{\\N}|Alias for #C{[^[:lf:]]}&"
	                /**/ "#C{\\s}|Alias for #C{[[:space:]]}&"
	                /**/ "#C{\\S}|Alias for #C{[^[:space:]]}&"
	                /**/ "#C{\\d}|Alias for #C{[[:digit:]]}&"
	                /**/ "#C{\\D}|Alias for #C{[^[:digit:]]}&"
	                /**/ "#C{\\0123}|Match the octal-encoded byte #C{0123} (only allowed in byte-mode; s.a. ?Arematch?DBytes)&"
	                /**/ "#C{\\xAB}|Match the hex-encoded byte #C{0xAB} (only allowed in byte-mode; s.a. ?Arematch?DBytes)&"
	                /**/ "#C{\\uABCD}|Match the unicode-character #C{U+ABCD} (not allowed in byte-mode; s.a. ?Arematch?DBytes)&"
	                /**/ "#C{\\U12345678}|Match the unicode-character #C{U+12345678} (not allowed in byte-mode; s.a. ?Arematch?DBytes)&"
	                /**/ "#C{\\u{1234 5689}}|Match the unicode-characters #C{U+1234}, followed by #C{U+5678} (not allowed in byte-mode; s.a. ?Arematch?DBytes)&"
	                /**/ "#C{\\1-9}|Back-reference to a preceding group (i.e. #C{( ... )}-pairs). "
	                /**/ /*      */ "Group indexes start at 1, and get assigned when an open-${(} is encountered in the input). "
	                /**/ /*      */ "There is no way to create back-references for groups other than the first 9. "
	                /**/ /*      */ "Character-ranges matched by groups can also be returned explicitly by ?#{regmatch}. "
	                /**/ /*      */ "Matches exactly what was previously matched by said group&"
	                /**/ "#C{\\x}|Match the literal $\"x\" (where #Cx is not one of the special escapes above). "
	                /**/ /*   */ "For the sake of compatibility, it is recommended not to use this, but to instead "
	                /**/ /*   */ "make use of #C{[x]} for the purpose of escaping potentially special characters."
	                "}\n"
	                "Supported repetition and group expressions:"
	                "#T{Expression|Description~"
	                /**/ "#I{#CBLANK}| Matches epsilon. Can appear like #C{()} or #C{(|X|Y)}, ...&"
	                /**/ "#C{(X)}|The given #CX forms a group that is treated as a unit. "
	                /**/ /*    */ "Its bounds can be determined explicitly using ?#regmatch&"
	                /**/ "#C{X#|Y}|Either #CX or #CY is matched&"
	                /**/ "#C{X?}|#CX is matched either 0 or 1 times (same as #C{X{0,1}})&"
	                /**/ "#C{X*}|#CX is matched any number of times (same as #C{X{0,}})&"
	                /**/ "#C{X+}|#CX is matched at least once (same as #C{X{1,}})&"
	                /**/ "#C{X{n,m}}|#CX is matched at least #Cn times, and at most #Cm times"
	                "}\n"
	                "Supported Charset expressions:"
	                "#T{Expression|Description~"
	                /**/ "#CXY|Match either #CX or #CY&"
	                /**/ "#Cc|Match the character #Cc&"
	                /**/ "#C{a-z}|Match any character #Cc such that #C{ORD(c) >= ORD(z) && ORD(c) <= ORD(z)}&"
	                /**/ "#C{[:CLASS:]}|Match any character apart of the named #C{CLASS}. Available classes are:"
	                /**/ "#T{#C{CLASS}|Trait function~"
	                /**/ /**/ "#Ccntrl|?#iscntrl (control characters)&"
	                /**/ /**/ "#Cspace|?#isspace (space characters)&"
	                /**/ /**/ "#Cupper|?#isupper (upper-case character)&"
	                /**/ /**/ "#Clower|?#islower (lower-case character)&"
	                /**/ /**/ "#Calpha|?#isalpha (alphabetical character)&"
	                /**/ /**/ "#Cdigit|?#isdigit (digit)&"
	                /**/ /**/ "#Cxdigit|?#isxdigit (hexadecimal-digit)&"
	                /**/ /**/ "#Calnum|?#isalnum (alphanumeric character)&"
	                /**/ /**/ "#Cpunct|?#ispunct (punctuation character)&"
	                /**/ /**/ "#Cgraph|?#isgraph (graphical character)&"
	                /**/ /**/ "#Cprint|?#isprint (printable character)&"
	                /**/ /**/ "#Cblank|?#isblank (blank character)&"
	                /**/ /**/ "#Csymstrt|?#issymstrt (symbol-start character)&"
	                /**/ /**/ "#Csymcont|?#issymcont (symbol-continuation character)&"
	                /**/ /**/ "#Ctab|?#istab (tabulator)&"
	                /**/ /**/ "#Cwhite|?#iswhite (white-space character)&"
	                /**/ /**/ "#Cempty|?#isempty (empty character)&"
	                /**/ /**/ "#Clf|?#islf (line-feed)&"
	                /**/ /**/ "#Chex|?#ishex (hex character)&"
	                /**/ /**/ "#Ctitle|?#istitle (title-case character)&"
	                /**/ /**/ "#Cnumeric|?#isnumeric (numerical character)"
	                /**/ "}&"
	                /**/ "#C{[=COLLATION=]}|Match all characters within the same equivalence group as #CCOLLATION&"
	                /**/ "#C{[.COLLATION.]}|Match the character named by #CCOLLATION&"
	                /**/ "#C{\\w}|Alias for #C{[:symcont:]}&"
	                /**/ "#C{\\n}|Alias for #C{[:lf:]}&"
	                /**/ "#C{\\s}|Alias for #C{[:space:]}&"
	                /**/ "#C{\\d}|Alias for #C{[:digit:]}&"
	                /**/ "#C{\\0123}|Match the octal-encoded byte #C{0123}&"
	                /**/ "#C{\\xAB}|Match the hex-encoded byte #C{0xAB}&"
	                /**/ "#C{\\uABCD}|Match the unicode-character #C{U+ABCD}&"
	                /**/ "#C{\\U12345678}|Match the unicode-character #C{U+12345678}&"
	                /**/ "#C{\\u{1234 5689}}|Match the unicode-characters #C{U+1234} or #C{U+5678}&"
	                /**/ "#C{\\x}|Match the literal $\"x\" (but see special escape-sequences above)"
	                "}\n"
	                "Supported location-assertion expressions:"
	                "#T{Expression|Description~"
	                /**/ "#C{^}|At start-of-input, or following a line-feed character&"
	                /**/ "#C{$}|At end-of-input, or preceding a line-feed character&"
	                /**/ "#C{\\`}|At start-of-input&"
	                /**/ "#C{\\A}|At start-of-input&"
	                /**/ "#C{\\'}|At end-of-input&"
	                /**/ "#C{\\Z}|At end-of-input&"
	                /**/ "#C{\\b}|At a word-boundary (prev/next character differ in #C{[[:symcont:]]}. "
	                /**/ /*   */ "Out-of-bounds characters are treated as not matching #Csymcont)&"
	                /**/ "#C{\\B}|NOT at a word-boundary&"
	                /**/ "#C{\\<}|At a start-of-word (prev/next character must match #C{[^[:symcont:]][[:symcont:]]}. "
	                /**/ /*   */ "Out-of-bounds characters are treated as not matching #Csymcont)&"
	                /**/ "#C{\\>}|At a end-of-word (prev/next character must match #C{[[:symcont:]][^[:symcont:]]}. "
	                /**/ /*   */ "Out-of-bounds characters are treated as not matching #Csymcont)&"
	                /**/ "#C{\\_<}|At a start-of-symbol (prev/next character must match #C{[^[:symcont:]][[:symstrt:]]}. "
	                /**/ /*    */ "Out-of-bounds characters are treated as not matching #Csymcont)&"
	                /**/ "#C{\\_>}|At a end-of-symbol (alias for #C{\\>})"
	                "}"),
	TYPE_KWMETHOD_F("rematches", &string_rematches,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST | METHOD_FNOREFESCAPE,
	                "(pattern:?.,start=!0,end=!-1,rules=!P{})->?Dbool\n"
	                "#ppattern{The regular expression pattern (s.a. ?#rematch)}"
	                "#prules{The regular expression rules (s.a. ?#rematch)}"
	                "#tValueError{The given @pattern is malformed}"
	                "Check if @pattern matches the entirety of the specified range of @this ?.\n"
	                "This function behaves identical to ${this.rematch(...) == ?#this}"),
	TYPE_KWMETHOD_F("refind", &string_refind,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST | METHOD_FNOREFESCAPE,
	                "(pattern:?.,start=!0,end=!-1,range:?Dint=!A!Dint!PSIZE_MAX,rules=!P{})->?X2?T2?Dint?Dint?N\n"
	                "#ppattern{The regular expression pattern (s.a. ?#rematch)}"
	                "#prange{The max number of search attempts to perform}"
	                "#prules{The regular expression rules (s.a. ?#rematch)}"
	                "#tValueError{The given @pattern is malformed}"
	                "Find the first sub-string matched by @pattern, and return its start/end indices, or ?N if no match exists\n"
	                "Note that using ?N in an expand expression will expand to the all ?N-values"),
	TYPE_KWMETHOD_F("rerfind", &string_rerfind,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST | METHOD_FNOREFESCAPE,
	                "(pattern:?.,start=!0,end=!-1,range:?Dint=!A!Dint!PSIZE_MAX,rules=!P{})->?X2?T2?Dint?Dint?N\n"
	                "#ppattern{The regular expression pattern (s.a. ?#rematch)}"
	                "#prange{The max number of search attempts to perform}"
	                "#prules{The regular expression rules (s.a. ?#rematch)}"
	                "#tValueError{The given @pattern is malformed}"
	                "Find the last sub-string matched by @pattern, and return its start/end indices, "
	                /**/ "or ?N if no match exists (s.a. #refind)"),
	TYPE_KWMETHOD_F("reindex", &string_reindex,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST | METHOD_FNOREFESCAPE,
	                "(pattern:?.,start=!0,end=!-1,range:?Dint=!A!Dint!PSIZE_MAX,rules=!P{})->?T2?Dint?Dint\n"
	                "#ppattern{The regular expression pattern (s.a. ?#rematch)}"
	                "#prange{The max number of search attempts to perform}"
	                "#prules{The regular expression rules (s.a. ?#rematch)}"
	                "#tValueError{The given @pattern is malformed}"
	                "#tIndexError{No substring matching the given @pattern could be found}"
	                "Same as ?#refind, but throw an :IndexError when no match can be found"),
	TYPE_KWMETHOD_F("rerindex", &string_rerindex,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST | METHOD_FNOREFESCAPE,
	                "(pattern:?.,start=!0,end=!-1,range:?Dint=!A!Dint!PSIZE_MAX,rules=!P{})->?T2?Dint?Dint\n"
	                "#ppattern{The regular expression pattern (s.a. ?#rematch)}"
	                "#prange{The max number of search attempts to perform}"
	                "#prules{The regular expression rules (s.a. ?#rematch)}"
	                "#tValueError{The given @pattern is malformed}"
	                "#tIndexError{No substring matching the given @pattern could be found}"
	                "Same as ?#rerfind, but throw an :IndexError when no match can be found"),
	TYPE_KWMETHOD_F("relocate", &string_relocate,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST | METHOD_FNOREFESCAPE,
	                "(pattern:?.,start=!0,end=!-1,range:?Dint=!A!Dint!PSIZE_MAX,rules=!P{})->?X2?.?N\n"
	                "#ppattern{The regular expression pattern (s.a. ?#rematch)}"
	                "#prange{The max number of search attempts to perform}"
	                "#prules{The regular expression rules (s.a. ?#rematch)}"
	                "#tValueError{The given @pattern is malformed}"
	                "Same as ${this.substr(this.refind(pattern, start, end, rules)...)}\n"
	                "In other words: return the first sub-string matched by the "
	                /**/ "given regular expression, or ?N if not found\n"
	                "This function has nothing to do with relocations! - it's pronounced R.E. locate"),
	TYPE_KWMETHOD_F("rerlocate", &string_rerlocate,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST | METHOD_FNOREFESCAPE,
	                "(pattern:?.,start=!0,end=!-1,range:?Dint=!A!Dint!PSIZE_MAX,rules=!P{})->?X2?.?N\n"
	                "#ppattern{The regular expression pattern (s.a. ?#rematch)}"
	                "#prange{The max number of search attempts to perform}"
	                "#prules{The regular expression rules (s.a. ?#rematch)}"
	                "#tValueError{The given @pattern is malformed}"
	                "Same as ${this.substr(this.rerfind(pattern, start, end, rules)...)}\n"
	                "In other words: return the last sub-string matched by the "
	                /**/ "given regular expression, or ?N if not found"),
	TYPE_KWMETHOD_F("repartition", &string_repartition,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST | METHOD_FNOREFESCAPE,
	                "(pattern:?.,start=!0,end=!-1,range:?Dint=!A!Dint!PSIZE_MAX,rules=!P{})->?T3?.?.?.\n"
	                "#ppattern{The regular expression pattern (s.a. ?#rematch)}"
	                "#prange{The max number of search attempts to perform}"
	                "#prules{The regular expression rules (s.a. ?#rematch)}"
	                "#tValueError{The given @pattern is malformed}"
	                "A hybrid between ?#refind and ?#partition\n${"
	                "function repartition(pattern: string, start: int, end: int, rules: string) {\n"
	                "	local start, end = this.refind(pattern, start, end, rules)...;\n"
	                "	if (start is none)\n"
	                "		return (this, \"\", \"\");\n"
	                "	return (\n"
	                "		this.substr(0, start),\n"
	                "		this.substr(start, end),\n"
	                "		this.substr(end, -1)\n"
	                "	);\n"
	                "}}"),
	TYPE_KWMETHOD_F("rerpartition", &string_rerpartition,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST | METHOD_FNOREFESCAPE,
	                "(pattern:?.,start=!0,end=!-1,range:?Dint=!A!Dint!PSIZE_MAX,rules=!P{})->?T3?.?.?.\n"
	                "#ppattern{The regular expression pattern (s.a. ?#rematch)}"
	                "#prange{The max number of search attempts to perform}"
	                "#prules{The regular expression rules (s.a. ?#rematch)}"
	                "#tValueError{The given @pattern is malformed}"
	                "A hybrid between ?#rerfind and ?#rpartition\n${"
	                "function rerpartition(pattern: string, start: int, end: int, rules: string) {\n"
	                "	local start, end = this.rerfind(pattern, start, end, rules)...;\n"
	                "	if (start is none)\n"
	                "		return (this, \"\", \"\");\n"
	                "	return (\n"
	                "		this.substr(0, start),\n"
	                "		this.substr(start, end), \n"
	                "		this.substr(end, -1)\n"
	                "	);\n"
	                "}}"),
	TYPE_KWMETHOD_F("rereplace", &string_rereplace,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST,
	                "(pattern:?.,replace:?.,max:?Dint=!A!Dint!PSIZE_MAX,rules=!P{})->?.\n"
	                "#ppattern{The regular expression pattern (s.a. ?#rematch)}"
	                "#prules{The regular expression rules (s.a. ?#rematch)}"
	                "#tValueError{The given @pattern is malformed}"
	                "Similar to ?#replace, however the ?. to search for is implemented as a regular expression "
	                "pattern, with the sub-string matched by it then getting replaced by @replace.\n"
	                "Locations where @pattern matches epsilon are not replaced\n"
	                "Additionally, @replace may contain sed-like match sequences:\n"
	                "#T{Expression|Description~"
	                /**/ "#C{&}|Replaced with the entire sub-string matched by @pattern&"
	                /**/ "#C{\\n}|Where $n is a digit ${1-9} specifying the n'th (1-based) group in "
	                /**/ /*   */ "@pattern (groups are determined by parenthesis in regex patterns)&"
	                /**/ "#C{\\\\}|Outputs a literal $r\"\\\" into the returned ?.&"
	                /**/ "#C{\\#&}|Outputs a literal $r\"#&\" into the returned ?."
	                "}"),
	TYPE_KWMETHOD_F("refindall", &string_refindall,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST,
	                "(pattern:?.,start=!0,end=!-1,rules=!P{})->?S?T2?Dint?Dint\n"
	                "#ppattern{The regular expression pattern (s.a. ?#rematch)}"
	                "#prules{The regular expression rules (s.a. ?#rematch)}"
	                "#tValueError{The given @pattern is malformed}"
	                "Similar to ?#refind, but return a sequence of all matches found within ${this.substr(start, end)}\n"
	                "Locations where @pattern matches epsilon are not included in the returned sequence\n"
	                "Note that the matches returned are ordered ascendingly"),
	TYPE_KWMETHOD_F("relocateall", &string_relocateall,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST,
	                "(pattern:?.,start=!0,end=!-1,rules=!P{})->?S?.\n"
	                "#ppattern{The regular expression pattern (s.a. ?#rematch)}"
	                "#prules{The regular expression rules (s.a. ?#rematch)}"
	                "#tValueError{The given @pattern is malformed}"
	                "Similar to ?#relocate, but return a sequence of all matched "
	                "sub-strings found within ${this.substr(start, end)}\n"
	                "Note that the matches returned are ordered ascendingly\n"
	                "Locations where @pattern matches epsilon are not included in the returned sequence\n"
	                "This function has nothing to do with relocations! - it's pronounced R.E. locate all"),
	TYPE_KWMETHOD_F("resplit", &string_resplit,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST,
	                "(pattern:?.,start=!0,end=!-1,rules=!P{})->?S?.\n"
	                "#ppattern{The regular expression pattern (s.a. ?#rematch)}"
	                "#prules{The regular expression rules (s.a. ?#rematch)}"
	                "#tValueError{The given @pattern is malformed}"
	                "Similar to ?#split, but use a regular expression in order to "
	                "express the sections of the ?. around which to perform the split\n"
	                "Locations where @pattern matches epsilon do not trigger a split\n"

	                "${"
	                /**/ "local data = \"10 , 20,30 40, 50\";\n"
	                /**/ "for (local x: data.resplit(r\"[[:space:],]+\"))\n"
	                /**/ "	print x; /* `10' `20' `30' `40' `50' */"
	                "}\n"

	                "If you wish to do the inverse and enumerate matches, rather than the "
	                "strings between matches, use ?#relocateall instead, which also behaves "
	                "as a sequence"),
	TYPE_KWMETHOD_F("restartswith", &string_restartswith,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST | METHOD_FNOREFESCAPE,
	                "(pattern:?.,start=!0,end=!-1,rules=!P{})->?Dbool\n"
	                "#ppattern{The regular expression pattern (s.a. ?#rematch)}"
	                "#prules{The regular expression rules (s.a. ?#rematch)}"
	                "#tValueError{The given @pattern is malformed}"
	                "Check if @this ?. starts with a regular expression described by @pattern (s.a. ?#startswith)\n"
	                "${"
	                /**/ "function restartswith(pattern: string) {\n"
	                /**/ "	return this.rematch(pattern) !is none;\n"
	                /**/ "}"
	                "}"),
	TYPE_KWMETHOD_F("reendswith", &string_reendswith,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST | METHOD_FNOREFESCAPE,
	                "(pattern:?.,start=!0,end=!-1,rules=!P{})->?Dbool\n"
	                "#ppattern{The regular expression pattern (s.a. ?#rematch)}"
	                "#prules{The regular expression rules (s.a. ?#rematch)}"
	                "#tValueError{The given @pattern is malformed}"
	                "Check if @this ?. ends with a regular expression described by @pattern (s.a. ?#endswith)\n"
	                "${"
	                /**/ "function restartswith(pattern: string) {\n"
	                /**/ "	local rpos = this.rerfind(pattern);\n"
	                /**/ "	return rpos !is none && rpos[1] == ##this;\n"
	                /**/ "}"
	                "}"),
	TYPE_KWMETHOD_F("restrip", &string_restrip,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST,
	                "(pattern:?.,start=!0,end=!-1,rules=!P{})->?.\n"
	                "#ppattern{The regular expression pattern (s.a. ?#rematch)}"
	                "#prules{The regular expression rules (s.a. ?#rematch)}"
	                "#tValueError{The given @pattern is malformed}"
	                "Strip all leading and trailing matches for @pattern from @this ?. and return the result (s.a. ?#strip)"),
	TYPE_KWMETHOD_F("relstrip", &string_relstrip,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST,
	                "(pattern:?.,start=!0,end=!-1,rules=!P{})->?.\n"
	                "#ppattern{The regular expression pattern (s.a. ?#rematch)}"
	                "#prules{The regular expression rules (s.a. ?#rematch)}"
	                "#tValueError{The given @pattern is malformed}"
	                "Strip all leading matches for @pattern from @this ?. and return the result (s.a. ?#lstrip)"),
	TYPE_KWMETHOD_F("rerstrip", &string_rerstrip,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST,
	                "(pattern:?.,start=!0,end=!-1,rules=!P{})->?.\n"
	                "#ppattern{The regular expression pattern (s.a. ?#rematch)}"
	                "#prules{The regular expression rules (s.a. ?#rematch)}"
	                "#tValueError{The given @pattern is malformed}"
	                "Strip all trailing matches for @pattern from @this ?. and return the result (s.a. ?#lstrip)"),
	TYPE_KWMETHOD_F("recount", &string_recount,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST | METHOD_FNOREFESCAPE,
	                "(pattern:?.,start=!0,end=!-1,rules=!P{})->?Dint\n"
	                "#ppattern{The regular expression pattern (s.a. ?#rematch)}"
	                "#prules{The regular expression rules (s.a. ?#rematch)}"
	                "#tValueError{The given @pattern is malformed}"
	                "Count the number of matches of a given regular expression @pattern (s.a. ?#count)\n"
	                "Hint: This is the same as ${##this.refindall(pattern)} or ${##this.relocateall(pattern)}\n"
	                "Instances where @pattern matches epsilon are not counted"),
	TYPE_KWMETHOD_F("recontains", &string_recontains,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST | METHOD_FNOREFESCAPE,
	                "(pattern:?.,start=!0,end=!-1,rules=!P{})->?Dbool\n"
	                "#ppattern{The regular expression pattern (s.a. ?#rematch)}"
	                "#prules{The regular expression rules (s.a. ?#rematch)}"
	                "#tValueError{The given @pattern is malformed}"
	                "Check if @this contains a match for the given regular expression @pattern (s.a. ?#contains)\n"
	                "Hint: This is the same as ${!!this.refindall(pattern)} or ${!!this.relocateall(pattern)}"),
	TYPE_KWMETHOD_F("rescanf", &string_rescanf,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST,
	                "(pattern:?.,start=!0,end=!-1,rules=!P{})->?S?X2?.?N\n"
	                "#ppattern{The regular expression pattern (s.a. ?#rematch)}"
	                "#prange{The max number of search attempts to perform}"
	                "#prules{The regular expression rules (s.a. ?#rematch)}"
	                "#tValueError{The given @pattern is malformed}"
	                "Similar to ?#regmatch, but rather than return a list of matched offsets, a sequence of "
	                "matched strings is returned, allowing the user to easily extract matched text in a way "
	                "that is similar to ?#scanf. Returns an empty sequence when @pattern can't be matched."),

	/* Regex functions that return the start-/end-offsets of all groups (rather than only the whole match) */
	TYPE_KWMETHOD_F("regmatch", &string_regmatch,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST | METHOD_FNOREFESCAPE,
	                "(pattern:?.,start=!0,end=!-1,rules=!P{})->?S?X2?T2?Dint?Dint?N\n"
	                "Similar to ?#rematch, but rather than only return the number of characters that were "
	                /**/ "matched by the regular expression as a whole, return a sequence of start-/end-"
	                /**/ "offsets for both the whole match itself (in ${return[0]}), as well as the "
	                /**/ "start-/end-offsets of each individual group referenced by @pattern. Groups "
	                /**/ "that didn't get matched (because they might be optional) appear as ?N in the "
	                /**/ "returned sequence.\n"
	                "When nothing was matched, an empty sequence is returned.\n"
	                "Example:\n"
	                "${"
	                /**/ "local groups = \"foo bar foobar\".regmatch(r\"fo(o) (b(x)r|bar) fo(o?bar)\");\n"
	                /**/ "assert groups == {\n"
	                /**/ "	{0, 14},  /* Whole match */\n"
	                /**/ "	{2, 3},   /* \"o\" */\n"
	                /**/ "	{4, 7},   /* \"bar\" */\n"
	                /**/ "	none,     /* never matched: \"x\" */\n"
	                /**/ "	{10, 14}, /* \"obar\" */\n"
	                /**/ "};"
	                "}\n"
	                "Note that (if something was matched), this function still only matches at the "
	                "start of @this ?.. If you want to search for @pattern and get the offsets of "
	                "all of the matched groups, you should use ?#regfind instead."),
	TYPE_KWMETHOD_F("regfind", &string_regfind,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST | METHOD_FNOREFESCAPE,
	                "(pattern:?.,start=!0,end=!-1,range:?Dint=!A!Dint!PSIZE_MAX,rules=!P{})->?S?X2?T2?Dint?Dint?N\n"
	                "Similar to ?#refind, but rather than only return the character-range "
	                /**/ "matched by the regular expression as a whole, return a sequence of start-/end-"
	                /**/ "offsets for both the whole match itself (in ${return[0]}), as well as the "
	                /**/ "start-/end-offsets of each individual group referenced by @pattern.\n"
	                "When nothing was matched, an empty sequence is returned (s.a. ?#regmatch)."),
	TYPE_KWMETHOD_F("regrfind", &string_regrfind,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST | METHOD_FNOREFESCAPE,
	                "(pattern:?.,start=!0,end=!-1,range:?Dint=!A!Dint!PSIZE_MAX,rules=!P{})->?S?X2?T2?Dint?Dint?N\n"
	                "Similar to ?#rerfind, but rather than only return the character-range "
	                /**/ "matched by the regular expression as a whole, return a sequence of start-/end-"
	                /**/ "offsets for both the whole match itself (in ${return[0]}), as well as the "
	                /**/ "start-/end-offsets of each individual group referenced by @pattern.\n"
	                "When nothing was matched, an empty sequence is returned (s.a. ?#regmatch)."),
	TYPE_KWMETHOD_F("regfindall", &string_regfindall,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST,
	                "(pattern:?.,start=!0,end=!-1,rules=!P{})->?S?S?X2?T2?Dint?Dint?N\n"
	                "Similar to ?#refindall, but rather than only return the character-ranges "
	                /**/ "matched by the regular expression as a whole, return a sequence of start-/end-"
	                /**/ "offsets for both the whole match itself (in ${return[0]}), as well as the "
	                /**/ "start-/end-offsets of each individual group referenced by @pattern.\n"
	                "When nothing was matched, an empty sequence is returned (s.a. ?#regmatch)."),
	TYPE_KWMETHOD_F("regindex", &string_regindex,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST | METHOD_FNOREFESCAPE,
	                "(pattern:?.,start=!0,end=!-1,range:?Dint=!A!Dint!PSIZE_MAX,rules=!P{})->?S?X2?T2?Dint?Dint?N\n"
	                "#ppattern{The regular expression pattern (s.a. ?#rematch)}"
	                "#prange{The max number of search attempts to perform}"
	                "#prules{The regular expression rules (s.a. ?#rematch)}"
	                "#tValueError{The given @pattern is malformed}"
	                "#tIndexError{No substring matching the given @pattern could be found}"
	                "Same as ?#regfind, but throw an :IndexError when no match can be found"),
	TYPE_KWMETHOD_F("regrindex", &string_regrindex,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST | METHOD_FNOREFESCAPE,
	                "(pattern:?.,start=!0,end=!-1,range:?Dint=!A!Dint!PSIZE_MAX,rules=!P{})->?S?X2?T2?Dint?Dint?N\n"
	                "#ppattern{The regular expression pattern (s.a. ?#rematch)}"
	                "#prange{The max number of search attempts to perform}"
	                "#prules{The regular expression rules (s.a. ?#rematch)}"
	                "#tValueError{The given @pattern is malformed}"
	                "#tIndexError{No substring matching the given @pattern could be found}"
	                "Same as ?#regrfind, but throw an :IndexError when no match can be found"),

	/* Deprecated functions. */
	TYPE_KWMETHOD_F("reverse", &string_reversed,
	                METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST | METHOD_FNOREFESCAPE,
	                "(start=!0,end=!-1)->?.\n"
	                "Deprecated alias for ?#reversed"),
	TYPE_METHOD_END
};

PRIVATE WUNUSED NONNULL((1, 2)) DREF String *DCALL
string_cat(String *__restrict self, DeeObject *__restrict other) {
	/* Simple case: `self' is an empty string, so just cast `other' into a string. */
	if (DeeString_IsEmpty(self))
		return (DREF String *)DeeObject_Str(other);
	if (DeeString_Check(other)) {
		/* In the likely case of `other' also being a string, we can
		 * try to perform some optimizations by looking at the common,
		 * required character width, and creating the resulting string
		 * in accordance to what _it_ requires (bypassing the need of
		 * a printer). */
		struct string_utf *lhs_utf;
		struct string_utf *rhs_utf;

		/* Simple case: `other' is an empty string, so just re-use `self'. */
		if (DeeString_IsEmpty(other))
			return_reference_(self);
		lhs_utf = self->s_data;
		rhs_utf = ((String *)other)->s_data;
		if (!lhs_utf || lhs_utf->u_width == STRING_WIDTH_1BYTE) {
			if (!rhs_utf || rhs_utf->u_width == STRING_WIDTH_1BYTE) {
				char *p;
				DREF String *result;
				size_t total_length = self->s_len + DeeString_SIZE(other);

				/* Most likely case: both strings use 1-byte characters,
				 * so we don't even need to use a multi-byte buffer! */
				result = (DREF String *)DeeObject_Malloc(offsetof(String, s_str) +
				                                         (total_length + 1) * sizeof(char));
				if unlikely(!result)
					goto err;
				result->s_len = total_length;

				/* Copy characters into the resulting string. */
				p = (char *)mempcpyc(result->s_str, self->s_str,
				                     self->s_len, sizeof(char));
				p = (char *)mempcpyc(p, DeeString_STR(other),
				                     DeeString_SIZE(other), sizeof(char));

				/* finalize the resulting string. */
				ASSERT(p == result->s_str + total_length);
				*p             = '\0';
				result->s_hash = DEE_STRING_HASH_UNSET;
				result->s_data = NULL;
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
			char *p;
			DREF String *result;
			struct string_utf *result_utf;
			size_t total_length = self->s_len + DeeString_SIZE(other);

			/* Most likely case: both strings use 1-byte characters,
			 * so we don't even need to use a multi-byte buffer! */
			result = (DREF String *)DeeObject_Malloc(offsetof(String, s_str) +
			                                         (total_length + 1) * sizeof(char));
			if unlikely(!result)
				goto err;
			result_utf = Dee_string_utf_alloc();
			if unlikely(!result_utf)
				goto err_r_2_4;
			bzero(result_utf, sizeof(struct string_utf));

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
			p = (char *)mempcpyc(result->s_str, self->s_str,
			                     self->s_len, sizeof(char));
			p = (char *)mempcpyc(p, DeeString_STR(other),
			                     DeeString_SIZE(other), sizeof(char));

			/* finalize the resulting string. */
			ASSERT(p == result->s_str + total_length);
			*p             = '\0';
			result->s_hash = DEE_STRING_HASH_UNSET;
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

PRIVATE WUNUSED NONNULL((1, 2)) DREF String *DCALL
string_mul(String *self, DeeObject *other) {
	size_t my_length, total_length, repeat;
	unsigned int width;
	if (DeeObject_AsSize(other, &repeat))
		goto err;
	if (DeeString_IsEmpty(self) || !repeat)
		return_reference_((String *)Dee_EmptyString);
	if (repeat == 1)
		return_reference_(self);
	width = DeeString_WIDTH(self);
	SWITCH_SIZEOF_WIDTH(width) {

	CASE_WIDTH_1BYTE: {
		DREF String *result;
		uint8_t *dst, *src;
		my_length = DeeString_SIZE(self);
		if (OVERFLOW_UMUL(my_length, repeat, &total_length))
			goto err_overflow;
		result = (DREF String *)DeeString_NewBuffer(total_length);
		if unlikely(!result)
			goto err;
		src = (uint8_t *)DeeString_STR(self);
		dst = (uint8_t *)DeeString_STR(result);
		while (repeat--)
			dst = (uint8_t *)mempcpyb(dst, src, my_length);
		return result;
	}	break;

	CASE_WIDTH_2BYTE: {
		uint16_t *dst, *src, *str;
		src       = DeeString_Get2Byte((DeeObject *)self);
		my_length = WSTR_LENGTH(src);
		if (OVERFLOW_UMUL(my_length, repeat, &total_length))
			goto err_overflow;
		dst = str = DeeString_New2ByteBuffer(total_length);
		if unlikely(!str)
			goto err;
		while (repeat--)
			dst = mempcpyw(dst, src, my_length);
		return (DREF String *)DeeString_Pack2ByteBuffer(str);
	}	break;

	CASE_WIDTH_4BYTE: {
		uint32_t *dst, *src, *str;
		src       = DeeString_Get4Byte((DeeObject *)self);
		my_length = WSTR_LENGTH(src);
		if (OVERFLOW_UMUL(my_length, repeat, &total_length))
			goto err_overflow;
		dst = str = DeeString_New4ByteBuffer(total_length);
		if unlikely(!str)
			goto err;
		while (repeat--)
			dst = mempcpyl(dst, src, my_length);
		return (DREF String *)DeeString_Pack4ByteBuffer(str);
	}	break;

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
                  size_t argc, DeeObject *const *argv);

PRIVATE WUNUSED NONNULL((1, 2)) DREF String *DCALL
string_mod(String *__restrict self, DeeObject *__restrict args) {
	struct unicode_printer printer = UNICODE_PRINTER_INIT;
	DeeObject *const *argv;
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


/* INTERN, because also used in `/src/deemon/objects/bytes.c' */
/* INTERN, because also used in `/src/deemon/objects/string.c' */
INTERN WUNUSED NONNULL((1, 2)) bool DCALL
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
		while (bytes_size) {
			--bytes_size;
			if (my_str.cp16[bytes_size] != (uint16_t)(bytes_data[bytes_size]))
				goto nope;
		}
		return true;

	CASE_WIDTH_4BYTE:
		my_str.cp32 = DeeString_Get4Byte((DeeObject *)self);
		if (bytes_size != WSTR_LENGTH(my_str.cp32))
			break;
		while (bytes_size) {
			--bytes_size;
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
		goto nope;
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
nope:
	return NULL;
}

LOCAL uint32_t *DCALL
memmemlb(uint32_t const *__restrict haystack, size_t haystack_length,
         uint8_t const *__restrict needle, size_t needle_length) {
	uint32_t *candidate;
	uint8_t marker;
	if unlikely(!needle_length || needle_length > haystack_length)
		goto nope;
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
nope:
	return NULL;
}


/* INTERN, because also used in `/src/deemon/objects/string.c' */
INTERN WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
string_contains(String *self, DeeObject *some_object) {
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


PUBLIC WUNUSED NONNULL((1)) size_t DCALL
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
	return result;
not_found:
	return (size_t)-1;
}

PUBLIC WUNUSED NONNULL((1)) size_t DCALL
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
	return result;
not_found:
	return (size_t)-1;
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

PUBLIC WUNUSED NONNULL((1, 2)) int
(DCALL Dee_unicode_printer_memcmp8)(struct unicode_printer const *__restrict self,
                                    uint8_t const *rhs, size_t lhs_start, size_t num_chars) {
	union dcharptr str;
	str.ptr = self->up_buffer;
	ASSERT(lhs_start + num_chars >= lhs_start);
	ASSERT(lhs_start + num_chars <= (str.ptr ? WSTR_LENGTH(str.ptr) : 0) || !num_chars);
	SWITCH_SIZEOF_WIDTH(self->up_flags & UNICODE_PRINTER_FWIDTH) {

	CASE_WIDTH_1BYTE:
		return memcmp(str.cp8 + lhs_start, rhs, num_chars);

	CASE_WIDTH_2BYTE: {
		size_t i;
		str.cp16 += lhs_start;
		for (i = 0; i < num_chars; ++i) {
			uint16_t l = str.cp16[i];
			uint8_t r  = rhs[i];
			if (l != r)
				return l < r ? -1 : 1;
		}
	}	break;

	CASE_WIDTH_4BYTE: {
		size_t i;
		str.cp32 += lhs_start;
		for (i = 0; i < num_chars; ++i) {
			uint32_t l = str.cp32[i];
			uint8_t r  = rhs[i];
			if (l != r)
				return l < r ? -1 : 1;
		}
	}	break;

	}
	return 0;
}

PUBLIC WUNUSED NONNULL((1, 2)) int
(DCALL Dee_unicode_printer_memcmp16)(struct unicode_printer const *__restrict self,
                                     uint16_t const *rhs, size_t lhs_start, size_t num_chars) {
	union dcharptr str;
	str.ptr = self->up_buffer;
	ASSERT(lhs_start + num_chars >= lhs_start);
	ASSERT(lhs_start + num_chars <= (str.ptr ? WSTR_LENGTH(str.ptr) : 0) || !num_chars);
	SWITCH_SIZEOF_WIDTH(self->up_flags & UNICODE_PRINTER_FWIDTH) {

	CASE_WIDTH_1BYTE: {
		size_t i;
		str.cp8 += lhs_start;
		for (i = 0; i < num_chars; ++i) {
			uint8_t l  = str.cp8[i];
			uint16_t r = rhs[i];
			if (l != r)
				return l < r ? -1 : 1;
		}
	}	break;

	CASE_WIDTH_2BYTE:
		return memcmp(str.cp16 + lhs_start, rhs, num_chars * 2); /* FIXME: Incorrect on LE for !=0 */

	CASE_WIDTH_4BYTE: {
		size_t i;
		str.cp32 += lhs_start;
		for (i = 0; i < num_chars; ++i) {
			uint32_t l = str.cp32[i];
			uint16_t r = rhs[i];
			if (l != r)
				return l < r ? -1 : 1;
		}
	}	break;

	}
	return 0;
}

PUBLIC WUNUSED NONNULL((1, 2)) int
(DCALL Dee_unicode_printer_memcmp32)(struct unicode_printer const *__restrict self,
                                     uint32_t const *rhs, size_t lhs_start, size_t num_chars) {
	union dcharptr str;
	str.ptr = self->up_buffer;
	ASSERT(lhs_start + num_chars >= lhs_start);
	ASSERT(lhs_start + num_chars <= (str.ptr ? WSTR_LENGTH(str.ptr) : 0) || !num_chars);
	SWITCH_SIZEOF_WIDTH(self->up_flags & UNICODE_PRINTER_FWIDTH) {

	CASE_WIDTH_1BYTE: {
		size_t i;
		str.cp8 += lhs_start;
		for (i = 0; i < num_chars; ++i) {
			uint8_t l  = str.cp8[i];
			uint32_t r = rhs[i];
			if (l != r)
				return l < r ? -1 : 1;
		}
	}	break;

	CASE_WIDTH_2BYTE: {
		size_t i;
		str.cp16 += lhs_start;
		for (i = 0; i < num_chars; ++i) {
			uint16_t l = str.cp16[i];
			uint32_t r = rhs[i];
			if (l != r)
				return l < r ? -1 : 1;
		}
	}	break;

	CASE_WIDTH_4BYTE:
		return memcmp(str.cp32 + lhs_start, rhs, num_chars * 4); /* FIXME: Incorrect on LE for !=0 */

	}
	return 0;
}

PUBLIC NONNULL((1, 2)) void
(DCALL Dee_unicode_printer_memcpy8)(struct Dee_unicode_printer *__restrict self,
                                    uint8_t const *src, size_t dst, size_t num_chars) {
	union dcharptr str;
	str.ptr = self->up_buffer;
	ASSERT(dst + num_chars >= dst);
	ASSERT(dst + num_chars <= (str.ptr ? WSTR_LENGTH(str.ptr) : 0) || !num_chars);
	SWITCH_SIZEOF_WIDTH(self->up_flags & UNICODE_PRINTER_FWIDTH) {

	CASE_WIDTH_1BYTE:
		memcpy(str.cp8 + dst, src, num_chars);
		break;

	CASE_WIDTH_2BYTE: {
		size_t i;
		str.cp16 += dst;
		for (i = 0; i < num_chars; ++i) {
			str.cp16[i] = (uint16_t)src[i];
		}
	}	break;

	CASE_WIDTH_4BYTE: {
		size_t i;
		str.cp32 += dst;
		for (i = 0; i < num_chars; ++i) {
			str.cp32[i] = (uint32_t)src[i];
		}
	}	break;

	}
}

PUBLIC NONNULL((1, 2)) void
(DCALL Dee_unicode_printer_memcpy16)(struct Dee_unicode_printer *__restrict self,
                                     uint16_t const *src, size_t dst, size_t num_chars) {
	union dcharptr str;
	str.ptr = self->up_buffer;
	ASSERT(dst + num_chars >= dst);
	ASSERT(dst + num_chars <= (str.ptr ? WSTR_LENGTH(str.ptr) : 0) || !num_chars);
	SWITCH_SIZEOF_WIDTH(self->up_flags & UNICODE_PRINTER_FWIDTH) {

	CASE_WIDTH_1BYTE: {
		size_t i;
		str.cp8 += dst;
		for (i = 0; i < num_chars; ++i) {
			str.cp8[i] = (uint8_t)src[i];
		}
	}	break;

	CASE_WIDTH_2BYTE:
		memcpyw(str.cp16 + dst, src, num_chars);
		break;

	CASE_WIDTH_4BYTE: {
		size_t i;
		str.cp32 += dst;
		for (i = 0; i < num_chars; ++i) {
			str.cp32[i] = (uint32_t)src[i];
		}
	}	break;

	}
}

PUBLIC NONNULL((1, 2)) void
(DCALL Dee_unicode_printer_memcpy32)(struct Dee_unicode_printer *__restrict self,
                                     uint32_t const *src, size_t dst, size_t num_chars) {
	union dcharptr str;
	str.ptr = self->up_buffer;
	ASSERT(dst + num_chars >= dst);
	ASSERT(dst + num_chars <= (str.ptr ? WSTR_LENGTH(str.ptr) : 0) || !num_chars);
	SWITCH_SIZEOF_WIDTH(self->up_flags & UNICODE_PRINTER_FWIDTH) {

	CASE_WIDTH_1BYTE: {
		size_t i;
		str.cp8 += dst;
		for (i = 0; i < num_chars; ++i) {
			str.cp8[i] = (uint8_t)src[i];
		}
	}	break;

	CASE_WIDTH_2BYTE: {
		size_t i;
		str.cp16 += dst;
		for (i = 0; i < num_chars; ++i) {
			str.cp16[i] = (uint16_t)src[i];
		}
	}	break;

	CASE_WIDTH_4BYTE:
		memcpyl(str.cp32 + dst, src, num_chars);
		break;
	}
}


DECL_END

#ifndef __INTELLISENSE__
#include "finder.c.inl"
#include "ordinals.c.inl"
#include "reproxy.c.inl"
#include "segments.c.inl"
#include "split.c.inl"

/* Include this last! */
#include "bytes_functions.c.inl"
#endif /* !__INTELLISENSE__ */

#endif /* !GUARD_DEEMON_OBJECTS_UNICODE_STRING_FUNCTIONS_C */
