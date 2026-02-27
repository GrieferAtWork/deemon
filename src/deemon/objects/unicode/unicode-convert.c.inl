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
#ifdef __INTELLISENSE__
#include "unicode.c"
#define DEFINE_DeeString_Convert
//#define DEFINE_DeeString_ToTitle
//#define DEFINE_DeeString_Capitalize
//#define DEFINE_DeeString_Swapcase
#endif /* __INTELLISENSE__ */

#include <deemon/api.h>

#include <deemon/object.h> /* ASSERT_OBJECT_TYPE_EXACT, DREF, return_reference_ */
#include <deemon/string.h> /* CASE_WIDTH_nBYTE, DeeString*, DeeUni_*, Dee_EmptyString, Dee_UNICODE_CONVERT_*, Dee_UNICODE_ISSPACE, Dee_charptr, Dee_charptr_const, Dee_unitraits, SWITCH_SIZEOF_WIDTH, WSTR_LENGTH */

#include <stddef.h> /* NULL, size_t */
#include <stdint.h> /* int32_t, uint8_t, uint16_t, uint32_t, uintptr_t */

#if (defined(DEFINE_DeeString_Convert) +    \
     defined(DEFINE_DeeString_ToTitle) +    \
     defined(DEFINE_DeeString_Capitalize) + \
     defined(DEFINE_DeeString_Swapcase)) != 1
#error "Must #define exactly one of these"
#endif

#ifdef DEFINE_DeeString_Convert
#define LOCAL_DeeString_Convert DeeString_Convert
#elif defined(DEFINE_DeeString_ToTitle)
#define LOCAL_DeeString_Convert DeeString_ToTitle
#elif defined(DEFINE_DeeString_Capitalize)
#define LOCAL_DeeString_Convert DeeString_Capitalize
#elif defined(DEFINE_DeeString_Swapcase)
#define LOCAL_DeeString_Convert DeeString_Swapcase
#else /* ... */
#error "Invalid configuration"
#endif /* !... */

DECL_BEGIN

INTERN WUNUSED NONNULL((1)) DREF String *DCALL
LOCAL_DeeString_Convert(String *__restrict self,
                        size_t start, size_t end
#ifdef DEFINE_DeeString_Convert
                        , uintptr_t kind
#endif /* DEFINE_DeeString_Convert */
                        )
{
#ifdef DEFINE_DeeString_Convert
#define LOCAL_DeeUni_Convert(ch) (void)((ch) = DeeUni_Convert(ch, kind))
#elif defined(DEFINE_DeeString_ToTitle)
	uintptr_t kind = Dee_UNICODE_CONVERT_TITLE;
	struct Dee_unitraits const *_desc;
#define LOCAL_DeeUni_Convert(ch)                                            \
	(void)(_desc = DeeUni_Descriptor(ch),                                   \
	       (ch)  = (ch) + *(int32_t const *)((byte_t const *)_desc + kind), \
	       kind  = (_desc->ut_flags & Dee_UNICODE_ISSPACE)                  \
	               ? Dee_UNICODE_CONVERT_TITLE                              \
	               : Dee_UNICODE_CONVERT_LOWER)
#elif defined(DEFINE_DeeString_Capitalize)
	uintptr_t kind = Dee_UNICODE_CONVERT_UPPER;
#define LOCAL_DeeUni_Convert(ch)            \
	(void)((ch) = DeeUni_Convert(ch, kind), \
	       kind = Dee_UNICODE_CONVERT_LOWER)
#elif defined(DEFINE_DeeString_Swapcase)
#define LOCAL_DeeUni_Convert(ch) (void)((ch) = DeeUni_SwapCase(ch))
#else /* ... */
#error "Invalid configuration"
#endif /* !... */

	unsigned int width;
	union Dee_charptr_const str;
	union Dee_charptr result;
	size_t i, length;
	uint32_t out;
	ASSERT_OBJECT_TYPE_EXACT(self, &DeeString_Type);
	width   = DeeString_WIDTH(self);
	str.ptr = DeeString_WSTR(self);
	length  = WSTR_LENGTH(str.ptr);
	if (start > length)
		start = length;
	if (end > length)
		end = length;
	if (start >= end)
		return_reference_((String *)Dee_EmptyString);
	end -= start;
	result.ptr = DeeString_NewWidthBuffer(end, width);
	if unlikely(!result.ptr)
		goto err;
	SWITCH_SIZEOF_WIDTH(width) {

	CASE_WIDTH_1BYTE:
		str.cp8 += start;
		for (i = 0; i < end; ++i) {
			out = str.cp8[i];
			LOCAL_DeeUni_Convert(out);
			if unlikely(out > 0xff)
				goto upgrade_1_to_x;
			result.cp8[i] = (uint8_t)out;
		}
		break;

	CASE_WIDTH_2BYTE:
		str.cp16 += start;
		for (i = 0; i < end; ++i) {
			out = str.cp16[i];
			LOCAL_DeeUni_Convert(out);
			if unlikely(out > 0xffff)
				goto upgrade_2_to_4_in_2;
			result.cp16[i] = (uint16_t)out;
		}
		break;

	CASE_WIDTH_4BYTE:
		str.cp32 += start;
		for (i = 0; i < end; ++i) {
			out = str.cp32[i];
			LOCAL_DeeUni_Convert(out);
			result.cp32[i] = out;
		}
		break;
	}
	return (DREF String *)DeeString_PackWidthBuffer(result.ptr, width);
	{
		size_t j;
		union Dee_charptr new_result;
upgrade_1_to_x:
		if (out > 0xffff)
			goto upgrade_1_to_4;
/*upgrade_1_to_2:*/
		new_result.cp16 = DeeString_New2ByteBuffer(end);
		if unlikely(!new_result.cp16)
			goto err_r;
		for (j = 0; j < i; ++j)
			new_result.cp16[j] = result.cp8[j];
		DeeString_Free1ByteBuffer(result.cp8);
		result = new_result;
		result.cp16[i++] = (uint16_t)out;
		for (; i < end; ++i) {
			out = str.cp8[i];
			LOCAL_DeeUni_Convert(out);
			if unlikely(out > 0xffff)
				goto upgrade_2_to_4_in_1;
			result.cp16[i] = (uint16_t)out;
		}
		return (DREF String *)DeeString_Pack2ByteBuffer(result.cp16);
	}
	{
		size_t j;
		union Dee_charptr new_result;
upgrade_2_to_4_in_1:
		new_result.cp32 = DeeString_New4ByteBuffer(end);
		if unlikely(!new_result.cp32)
			goto err_r;
		for (j = 0; j < i; ++j)
			new_result.cp32[j] = result.cp16[j];
		DeeString_Free2ByteBuffer(result.cp16);
		result = new_result;
		result.cp32[i++] = out;
		for (; i < end; ++i) {
			out = str.cp8[i];
			LOCAL_DeeUni_Convert(out);
			result.cp32[i] = out;
		}
		return (DREF String *)DeeString_Pack4ByteBuffer(result.cp32);
	}
	{
		size_t j;
		union Dee_charptr new_result;
upgrade_2_to_4_in_2:
		new_result.cp32 = DeeString_New4ByteBuffer(end);
		if unlikely(!new_result.cp32)
			goto err_r;
		for (j = 0; j < i; ++j)
			new_result.cp32[j] = result.cp16[j];
		DeeString_Free2ByteBuffer(result.cp16);
		result = new_result;
		result.cp32[i++] = out;
		for (; i < end; ++i) {
			out = str.cp16[i];
			LOCAL_DeeUni_Convert(out);
			result.cp32[i] = out;
		}
		return (DREF String *)DeeString_Pack4ByteBuffer(result.cp32);
	}
	{
		size_t j;
		union Dee_charptr new_result;
upgrade_1_to_4:
		new_result.cp32 = DeeString_New4ByteBuffer(end);
		if unlikely(!new_result.cp32)
			goto err_r;
		for (j = 0; j < i; ++j)
			new_result.cp32[j] = result.cp8[j];
		DeeString_Free1ByteBuffer(result.cp8);
		result = new_result;
		result.cp32[i++] = out;
		for (; i < end; ++i) {
			out = str.cp8[i];
			LOCAL_DeeUni_Convert(out);
			result.cp32[i] = out;
		}
		return (DREF String *)DeeString_Pack4ByteBuffer(result.cp32);
	}
err_r:
	DeeString_FreeWidthBuffer(result.ptr, width);
err:
	return NULL;
#undef LOCAL_DeeUni_Convert
}

#undef LOCAL_DeeString_Convert

DECL_END

#undef DEFINE_DeeString_Convert
#undef DEFINE_DeeString_ToTitle
#undef DEFINE_DeeString_Capitalize
#undef DEFINE_DeeString_Swapcase
