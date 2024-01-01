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
#ifdef __INTELLISENSE__
#include "string_functions.c"
//#define DEFINE_DeeString_StripSpc
//#define DEFINE_DeeString_LStripSpc
//#define DEFINE_DeeString_RStripSpc
//#define DEFINE_DeeString_StripMask
//#define DEFINE_DeeString_LStripMask
//#define DEFINE_DeeString_RStripMask
//#define DEFINE_DeeString_CaseStripMask
//#define DEFINE_DeeString_CaseLStripMask
//#define DEFINE_DeeString_CaseRStripMask
//#define DEFINE_DeeString_SStrip
//#define DEFINE_DeeString_LSStrip
//#define DEFINE_DeeString_RSStrip
//#define DEFINE_DeeString_CaseSStrip
//#define DEFINE_DeeString_CaseLSStrip
//#define DEFINE_DeeString_CaseRSStrip
#define DEFINE_DeeString_StripLinesSpc
//#define DEFINE_DeeString_LStripLinesSpc
//#define DEFINE_DeeString_RStripLinesSpc
//#define DEFINE_DeeString_StripLinesMask
//#define DEFINE_DeeString_LStripLinesMask
//#define DEFINE_DeeString_RStripLinesMask
//#define DEFINE_DeeString_CaseStripLinesMask
//#define DEFINE_DeeString_CaseLStripLinesMask
//#define DEFINE_DeeString_CaseRStripLinesMask
//#define DEFINE_DeeString_SStripLines
//#define DEFINE_DeeString_LSStripLines
//#define DEFINE_DeeString_RSStripLines
//#define DEFINE_DeeString_CaseSStripLines
//#define DEFINE_DeeString_CaseLSStripLines
//#define DEFINE_DeeString_CaseRSStripLines
#endif /* __INTELLISENSE__ */

DECL_BEGIN

#if (defined(DEFINE_DeeString_StripSpc) +            \
     defined(DEFINE_DeeString_LStripSpc) +           \
     defined(DEFINE_DeeString_RStripSpc) +           \
     defined(DEFINE_DeeString_StripMask) +           \
     defined(DEFINE_DeeString_LStripMask) +          \
     defined(DEFINE_DeeString_RStripMask) +          \
     defined(DEFINE_DeeString_CaseStripMask) +       \
     defined(DEFINE_DeeString_CaseLStripMask) +      \
     defined(DEFINE_DeeString_CaseRStripMask) +      \
     defined(DEFINE_DeeString_SStrip) +              \
     defined(DEFINE_DeeString_LSStrip) +             \
     defined(DEFINE_DeeString_RSStrip) +             \
     defined(DEFINE_DeeString_CaseSStrip) +          \
     defined(DEFINE_DeeString_CaseLSStrip) +         \
     defined(DEFINE_DeeString_CaseRSStrip) +         \
     defined(DEFINE_DeeString_StripLinesSpc) +       \
     defined(DEFINE_DeeString_LStripLinesSpc) +      \
     defined(DEFINE_DeeString_RStripLinesSpc) +      \
     defined(DEFINE_DeeString_StripLinesMask) +      \
     defined(DEFINE_DeeString_LStripLinesMask) +     \
     defined(DEFINE_DeeString_RStripLinesMask) +     \
     defined(DEFINE_DeeString_CaseStripLinesMask) +  \
     defined(DEFINE_DeeString_CaseLStripLinesMask) + \
     defined(DEFINE_DeeString_CaseRStripLinesMask) + \
     defined(DEFINE_DeeString_SStripLines) +         \
     defined(DEFINE_DeeString_LSStripLines) +        \
     defined(DEFINE_DeeString_RSStripLines) +        \
     defined(DEFINE_DeeString_CaseSStripLines) +     \
     defined(DEFINE_DeeString_CaseLSStripLines) +    \
     defined(DEFINE_DeeString_CaseRSStripLines)) != 1
#error "Must #define exactly one of these macros!"
#endif /* ... */

#ifdef DEFINE_DeeString_StripSpc
#define LOCAL_DeeString_Strip DeeString_StripSpc
#define LOCAL_IS_LSTRIP
#define LOCAL_IS_RSTRIP
#elif defined(DEFINE_DeeString_LStripSpc)
#define LOCAL_DeeString_Strip DeeString_LStripSpc
#define LOCAL_HAVE_max_count
#define LOCAL_IS_LSTRIP
#elif defined(DEFINE_DeeString_RStripSpc)
#define LOCAL_DeeString_Strip DeeString_RStripSpc
#define LOCAL_HAVE_max_count
#define LOCAL_IS_RSTRIP
#elif defined(DEFINE_DeeString_StripMask)
#define LOCAL_DeeString_Strip DeeString_StripMask
#define LOCAL_IS_LSTRIP
#define LOCAL_IS_RSTRIP
#define LOCAL_IS_MASKED
#elif defined(DEFINE_DeeString_LStripMask)
#define LOCAL_DeeString_Strip DeeString_LStripMask
#define LOCAL_HAVE_max_count
#define LOCAL_IS_LSTRIP
#define LOCAL_IS_MASKED
#elif defined(DEFINE_DeeString_RStripMask)
#define LOCAL_DeeString_Strip DeeString_RStripMask
#define LOCAL_HAVE_max_count
#define LOCAL_IS_RSTRIP
#define LOCAL_IS_MASKED
#elif defined(DEFINE_DeeString_CaseStripMask)
#define LOCAL_DeeString_Strip DeeString_CaseStripMask
#define LOCAL_IS_LSTRIP
#define LOCAL_IS_RSTRIP
#define LOCAL_IS_MASKED
#define LOCAL_IS_NOCASE
#elif defined(DEFINE_DeeString_CaseLStripMask)
#define LOCAL_DeeString_Strip DeeString_CaseLStripMask
#define LOCAL_HAVE_max_count
#define LOCAL_IS_LSTRIP
#define LOCAL_IS_MASKED
#define LOCAL_IS_NOCASE
#elif defined(DEFINE_DeeString_CaseRStripMask)
#define LOCAL_DeeString_Strip DeeString_CaseRStripMask
#define LOCAL_HAVE_max_count
#define LOCAL_IS_RSTRIP
#define LOCAL_IS_MASKED
#define LOCAL_IS_NOCASE
#elif defined(DEFINE_DeeString_SStrip)
#define LOCAL_DeeString_Strip DeeString_SStrip
#define LOCAL_IS_LSTRIP
#define LOCAL_IS_RSTRIP
#define LOCAL_IS_SSTRIP
#elif defined(DEFINE_DeeString_LSStrip)
#define LOCAL_DeeString_Strip DeeString_LSStrip
#define LOCAL_HAVE_max_count
#define LOCAL_IS_LSTRIP
#define LOCAL_IS_SSTRIP
#elif defined(DEFINE_DeeString_RSStrip)
#define LOCAL_DeeString_Strip DeeString_RSStrip
#define LOCAL_HAVE_max_count
#define LOCAL_IS_RSTRIP
#define LOCAL_IS_SSTRIP
#elif defined(DEFINE_DeeString_CaseSStrip)
#define LOCAL_DeeString_Strip DeeString_CaseSStrip
#define LOCAL_IS_LSTRIP
#define LOCAL_IS_RSTRIP
#define LOCAL_IS_SSTRIP
#define LOCAL_IS_NOCASE
#elif defined(DEFINE_DeeString_CaseLSStrip)
#define LOCAL_DeeString_Strip DeeString_CaseLSStrip
#define LOCAL_HAVE_max_count
#define LOCAL_IS_LSTRIP
#define LOCAL_IS_SSTRIP
#define LOCAL_IS_NOCASE
#elif defined(DEFINE_DeeString_CaseRSStrip)
#define LOCAL_DeeString_Strip DeeString_CaseRSStrip
#define LOCAL_HAVE_max_count
#define LOCAL_IS_RSTRIP
#define LOCAL_IS_SSTRIP
#define LOCAL_IS_NOCASE
#elif defined(DEFINE_DeeString_StripLinesSpc)
#define LOCAL_DeeString_Strip DeeString_StripLinesSpc
#define LOCAL_IS_LINES
#define LOCAL_IS_LSTRIP
#define LOCAL_IS_RSTRIP
#elif defined(DEFINE_DeeString_LStripLinesSpc)
#define LOCAL_DeeString_Strip DeeString_LStripLinesSpc
#define LOCAL_IS_LINES
#define LOCAL_IS_LSTRIP
#elif defined(DEFINE_DeeString_RStripLinesSpc)
#define LOCAL_DeeString_Strip DeeString_RStripLinesSpc
#define LOCAL_IS_LINES
#define LOCAL_IS_RSTRIP
#elif defined(DEFINE_DeeString_StripLinesMask)
#define LOCAL_DeeString_Strip DeeString_StripLinesMask
#define LOCAL_IS_LINES
#define LOCAL_IS_LSTRIP
#define LOCAL_IS_RSTRIP
#define LOCAL_IS_MASKED
#elif defined(DEFINE_DeeString_LStripLinesMask)
#define LOCAL_DeeString_Strip DeeString_LStripLinesMask
#define LOCAL_IS_LINES
#define LOCAL_IS_LSTRIP
#define LOCAL_IS_MASKED
#elif defined(DEFINE_DeeString_RStripLinesMask)
#define LOCAL_DeeString_Strip DeeString_RStripLinesMask
#define LOCAL_IS_LINES
#define LOCAL_IS_RSTRIP
#define LOCAL_IS_MASKED
#elif defined(DEFINE_DeeString_CaseStripLinesMask)
#define LOCAL_DeeString_Strip DeeString_CaseStripLinesMask
#define LOCAL_IS_LINES
#define LOCAL_IS_LSTRIP
#define LOCAL_IS_RSTRIP
#define LOCAL_IS_MASKED
#define LOCAL_IS_NOCASE
#elif defined(DEFINE_DeeString_CaseLStripLinesMask)
#define LOCAL_DeeString_Strip DeeString_CaseLStripLinesMask
#define LOCAL_IS_LINES
#define LOCAL_IS_LSTRIP
#define LOCAL_IS_MASKED
#define LOCAL_IS_NOCASE
#elif defined(DEFINE_DeeString_CaseRStripLinesMask)
#define LOCAL_DeeString_Strip DeeString_CaseRStripLinesMask
#define LOCAL_IS_LINES
#define LOCAL_IS_RSTRIP
#define LOCAL_IS_MASKED
#define LOCAL_IS_NOCASE
#elif defined(DEFINE_DeeString_SStripLines)
#define LOCAL_DeeString_Strip DeeString_SStripLines
#define LOCAL_IS_LINES
#define LOCAL_IS_LSTRIP
#define LOCAL_IS_RSTRIP
#define LOCAL_IS_SSTRIP
#elif defined(DEFINE_DeeString_LSStripLines)
#define LOCAL_DeeString_Strip DeeString_LSStripLines
#define LOCAL_IS_LINES
#define LOCAL_IS_LSTRIP
#define LOCAL_IS_SSTRIP
#elif defined(DEFINE_DeeString_RSStripLines)
#define LOCAL_DeeString_Strip DeeString_RSStripLines
#define LOCAL_IS_LINES
#define LOCAL_IS_RSTRIP
#define LOCAL_IS_SSTRIP
#elif defined(DEFINE_DeeString_CaseSStripLines)
#define LOCAL_DeeString_Strip DeeString_CaseSStripLines
#define LOCAL_IS_LINES
#define LOCAL_IS_LSTRIP
#define LOCAL_IS_RSTRIP
#define LOCAL_IS_SSTRIP
#define LOCAL_IS_NOCASE
#elif defined(DEFINE_DeeString_CaseLSStripLines)
#define LOCAL_DeeString_Strip DeeString_CaseLSStripLines
#define LOCAL_IS_LINES
#define LOCAL_IS_LSTRIP
#define LOCAL_IS_SSTRIP
#define LOCAL_IS_NOCASE
#elif defined(DEFINE_DeeString_CaseRSStripLines)
#define LOCAL_DeeString_Strip DeeString_CaseRSStripLines
#define LOCAL_IS_LINES
#define LOCAL_IS_RSTRIP
#define LOCAL_IS_SSTRIP
#define LOCAL_IS_NOCASE
#endif

#ifdef LOCAL_IS_SSTRIP
#define LOCAL_IS_MASKED
#endif /* LOCAL_IS_SSTRIP */

#ifdef LOCAL_HAVE_max_count
#define LOCAL__PARAM_max_count  , size_t max_count
#define LOCAL_max_count_OR_true max_count
#define LOCAL_max_count_dec()   (void)--max_count
#else /* LOCAL_HAVE_max_count */
#define LOCAL__PARAM_max_count  /* nothing */
#define LOCAL_max_count_OR_true 1
#define LOCAL_max_count_dec()   (void)0
#endif /* !LOCAL_HAVE_max_count */

#ifdef LOCAL_IS_MASKED
PRIVATE WUNUSED NONNULL((1, 2)) DREF String *DCALL
LOCAL_DeeString_Strip(String *self, String *mask LOCAL__PARAM_max_count)
#else /* LOCAL_IS_MASKED */
PRIVATE WUNUSED NONNULL((1)) DREF String *DCALL
LOCAL_DeeString_Strip(String *__restrict self LOCAL__PARAM_max_count)
#endif /* !LOCAL_IS_MASKED */
{
#ifdef LOCAL_IS_LINES
	struct unicode_printer printer = UNICODE_PRINTER_INIT;
	union dcharptr flush_start, newend;
#endif /* LOCAL_IS_LINES */
#ifdef LOCAL_IS_MASKED
	union dcharptr mskstr;
	size_t msklen;
#endif /* LOCAL_IS_MASKED */
	union dcharptr mystr, newstr;
	size_t mylen, newlen;

#ifdef LOCAL_IS_MASKED
	SWITCH_SIZEOF_WIDTH(STRING_WIDTH_COMMON(DeeString_WIDTH(self),
	                                        DeeString_WIDTH(mask)))
#else /* LOCAL_IS_MASKED */
	mystr.ptr = newstr.ptr = DeeString_WSTR(self);
	mylen = newlen = WSTR_LENGTH(mystr.ptr);
	SWITCH_SIZEOF_WIDTH(DeeString_WIDTH(self))
#endif /* !LOCAL_IS_MASKED */
	{
#ifdef LOCAL_IS_SSTRIP
#ifdef LOCAL_IS_NOCASE
		size_t matchlen;
#define LOCAL_matchlen matchlen
#define LOCAL_MEMSTARTSWITHB(haystack_base, haystack_len) ((matchlen = MEMCASESTARTSWITHB(haystack_base, haystack_len, mskstr.cp8, msklen)) != 0)
#define LOCAL_MEMSTARTSWITHW(haystack_base, haystack_len) ((matchlen = MEMCASESTARTSWITHW(haystack_base, haystack_len, mskstr.cp16, msklen)) != 0)
#define LOCAL_MEMSTARTSWITHL(haystack_base, haystack_len) ((matchlen = MEMCASESTARTSWITHL(haystack_base, haystack_len, mskstr.cp32, msklen)) != 0)
#define LOCAL_MEMENDSWITHB(haystack_base, haystack_len)   ((matchlen = MEMCASEENDSWITHB(haystack_base, haystack_len, mskstr.cp8, msklen)) != 0)
#define LOCAL_MEMENDSWITHW(haystack_base, haystack_len)   ((matchlen = MEMCASEENDSWITHW(haystack_base, haystack_len, mskstr.cp16, msklen)) != 0)
#define LOCAL_MEMENDSWITHL(haystack_base, haystack_len)   ((matchlen = MEMCASEENDSWITHL(haystack_base, haystack_len, mskstr.cp32, msklen)) != 0)
#else /* LOCAL_IS_NOCASE */
#define LOCAL_MEMSTARTSWITHB(haystack_base, haystack_len) ((haystack_len) >= msklen && MEMEQB(haystack_base, mskstr.cp8, msklen))
#define LOCAL_MEMSTARTSWITHW(haystack_base, haystack_len) ((haystack_len) >= msklen && MEMEQW(haystack_base, mskstr.cp16, msklen))
#define LOCAL_MEMSTARTSWITHL(haystack_base, haystack_len) ((haystack_len) >= msklen && MEMEQL(haystack_base, mskstr.cp32, msklen))
#define LOCAL_MEMENDSWITHB(haystack_base, haystack_len)   ((haystack_len) >= msklen && MEMEQB((haystack_base) + ((haystack_len) - msklen), mskstr.cp8, msklen))
#define LOCAL_MEMENDSWITHW(haystack_base, haystack_len)   ((haystack_len) >= msklen && MEMEQW((haystack_base) + ((haystack_len) - msklen), mskstr.cp16, msklen))
#define LOCAL_MEMENDSWITHL(haystack_base, haystack_len)   ((haystack_len) >= msklen && MEMEQL((haystack_base) + ((haystack_len) - msklen), mskstr.cp32, msklen))
#define LOCAL_matchlen msklen
#endif /* !LOCAL_IS_NOCASE */
#else /* LOCAL_IS_SSTRIP */
#ifdef LOCAL_IS_MASKED
#ifdef LOCAL_IS_NOCASE
#define LOCAL_isspace_b(ch) memcasechrb(mskstr.cp8, ch, msklen)
#define LOCAL_isspace_w(ch) memcasechrw(mskstr.cp16, ch, msklen)
#define LOCAL_isspace_l(ch) memcasechrl(mskstr.cp32, ch, msklen)
#else /* LOCAL_IS_NOCASE */
#define LOCAL_isspace_b(ch) memchrb(mskstr.cp8, ch, msklen)
#define LOCAL_isspace_w(ch) memchrw(mskstr.cp16, ch, msklen)
#define LOCAL_isspace_l(ch) memchrl(mskstr.cp32, ch, msklen)
#endif /* !LOCAL_IS_NOCASE */
#elif defined(LOCAL_IS_LINES)
#define LOCAL_isspace_b DeeUni_IsSpaceNoLf
#define LOCAL_isspace_w DeeUni_IsSpaceNoLf
#define LOCAL_isspace_l DeeUni_IsSpaceNoLf
#else /* ... */
#define LOCAL_isspace_b DeeUni_IsSpace
#define LOCAL_isspace_w DeeUni_IsSpace
#define LOCAL_isspace_l DeeUni_IsSpace
#endif /* !... */
#define LOCAL_MEMSTARTSWITHB(haystack_base, haystack_len) ((haystack_len) >= 1 && LOCAL_isspace_b((haystack_base)[0]))
#define LOCAL_MEMSTARTSWITHW(haystack_base, haystack_len) ((haystack_len) >= 1 && LOCAL_isspace_w((haystack_base)[0]))
#define LOCAL_MEMSTARTSWITHL(haystack_base, haystack_len) ((haystack_len) >= 1 && LOCAL_isspace_l((haystack_base)[0]))
#define LOCAL_MEMENDSWITHB(haystack_base, haystack_len)   ((haystack_len) >= 1 && LOCAL_isspace_b((haystack_base)[(haystack_len) - 1]))
#define LOCAL_MEMENDSWITHW(haystack_base, haystack_len)   ((haystack_len) >= 1 && LOCAL_isspace_w((haystack_base)[(haystack_len) - 1]))
#define LOCAL_MEMENDSWITHL(haystack_base, haystack_len)   ((haystack_len) >= 1 && LOCAL_isspace_l((haystack_base)[(haystack_len) - 1]))
#endif /* !LOCAL_IS_SSTRIP */

#ifndef LOCAL_matchlen
#define LOCAL_matchlen 1
#endif /* !LOCAL_matchlen */


		/************************************************************************/
		/* 8-bit                                                                */
		/************************************************************************/
	CASE_WIDTH_1BYTE:
#ifdef LOCAL_IS_MASKED
		mystr.cp8  = DeeString_As1Byte((DeeObject *)self);
		mskstr.cp8 = DeeString_As1Byte((DeeObject *)mask);
		mylen      = WSTR_LENGTH(mystr.cp8);
		msklen     = WSTR_LENGTH(mskstr.cp8);
		newstr.cp8 = mystr.cp8;
		newlen     = mylen;
#ifdef LOCAL_IS_SSTRIP
		if unlikely(!msklen)
			goto return_self;
#endif /* LOCAL_IS_SSTRIP */
#endif /* LOCAL_IS_MASKED */
#ifdef LOCAL_IS_LSTRIP
		while (LOCAL_max_count_OR_true && LOCAL_MEMSTARTSWITHB(newstr.cp8, newlen)) {
			newstr.cp8 += LOCAL_matchlen;
			newlen -= LOCAL_matchlen;
			LOCAL_max_count_dec();
		}
#endif /* LOCAL_IS_LSTRIP */
#ifdef LOCAL_IS_RSTRIP
		while (LOCAL_max_count_OR_true && LOCAL_MEMENDSWITHB(newstr.cp8, newlen)) {
			newlen -= LOCAL_matchlen;
			LOCAL_max_count_dec();
		}
#endif /* LOCAL_IS_RSTRIP */

		/* Strip around line-feeds. */
#ifdef LOCAL_IS_LINES
		flush_start.cp8 = newstr.cp8;
		newend.cp8      = newstr.cp8 + newlen;
		while (newstr.cp8 < newend.cp8) {
			uint8_t ch = *newstr.cp8;
			if (!DeeUni_IsLF(ch)) {
				++newstr.cp8;
				continue;
			}

#ifdef LOCAL_IS_RSTRIP
			/* rstrip (at the end of lines) */
			{
				union dcharptr flush_end;
				flush_end.cp8 = newstr.cp8;
				while (LOCAL_MEMENDSWITHB(flush_start.cp8, (size_t)(flush_end.cp8 - flush_start.cp8))) {
					flush_end.cp8 -= LOCAL_matchlen;
				}
				if (flush_end.cp8 < newstr.cp8) {
					if unlikely(unicode_printer_print8(&printer, flush_start.cp8,
					                                   (size_t)(flush_end.cp8 - flush_start.cp8)) < 0)
						goto err;
					flush_start.cp8 = newstr.cp8; /* Skip over whitespace */
				}
			}
#endif /* LOCAL_IS_RSTRIP */

			/* Skip over the linefeed (preserving it) */
			++newstr.cp8;
			if (ch == UNICODE_CR && newstr.cp8 < newend.cp8 && *newstr.cp8 == UNICODE_LF)
				++newstr.cp8; /* Deal with CRLF-style linefeeds */

#ifdef LOCAL_IS_LSTRIP
			/* lstrip (at the start of lines) */
			{
				union dcharptr new_flush_start;
				new_flush_start.cp8 = newstr.cp8;
				while (LOCAL_MEMSTARTSWITHB(new_flush_start.cp8, (size_t)(newend.cp8 - new_flush_start.cp8))) {
					new_flush_start.cp8 += LOCAL_matchlen;
				}
				if (new_flush_start.cp8 > newstr.cp8) {
					if unlikely(unicode_printer_print8(&printer, flush_start.cp8,
					                                   (size_t)(newstr.cp8 - flush_start.cp8)) < 0)
						goto err;
					flush_start.cp8 = new_flush_start.cp8; /* Skip over whitespace */
					newstr.cp8      = new_flush_start.cp8;
				}
			}
#endif /* LOCAL_IS_LSTRIP */
		}
		ASSERT(newstr.cp8 == newend.cp8);
		if (!UNICODE_PRINTER_ISEMPTY(&printer)) {
			if unlikely(unicode_printer_print8(&printer, flush_start.cp8,
			                                   (size_t)(newstr.cp8 - flush_start.cp8)) < 0)
				goto err;
			goto return_printer;
		}
		unicode_printer_fini(&printer);
#endif /* LOCAL_IS_LINES */

		if (newlen == mylen)
			goto return_self_noprinter;
		return (DREF String *)DeeString_New1Byte(newstr.cp8, newlen);


		/************************************************************************/
		/* 16-bit                                                               */
		/************************************************************************/
	CASE_WIDTH_2BYTE:
#ifdef LOCAL_IS_MASKED
		mystr.cp16 = DeeString_As2Byte((DeeObject *)self);
		if unlikely(!mystr.cp16)
			goto err;
		mskstr.cp16 = DeeString_As2Byte((DeeObject *)mask);
		if unlikely(!mskstr.cp16)
			goto err;
		mylen       = WSTR_LENGTH(mystr.cp16);
		msklen      = WSTR_LENGTH(mskstr.cp16);
		newstr.cp16 = mystr.cp16;
		newlen      = mylen;
#ifdef LOCAL_IS_SSTRIP
		if unlikely(!msklen)
			goto return_self;
#endif /* LOCAL_IS_SSTRIP */
#endif /* LOCAL_IS_MASKED */
#ifdef LOCAL_IS_LSTRIP
		while (LOCAL_max_count_OR_true && LOCAL_MEMSTARTSWITHW(newstr.cp16, newlen)) {
			newstr.cp16 += LOCAL_matchlen;
			newlen -= LOCAL_matchlen;
			LOCAL_max_count_dec();
		}
#endif /* LOCAL_IS_LSTRIP */
#ifdef LOCAL_IS_RSTRIP
		while (LOCAL_max_count_OR_true && LOCAL_MEMENDSWITHW(newstr.cp16, newlen)) {
			newlen -= LOCAL_matchlen;
			LOCAL_max_count_dec();
		}
#endif /* LOCAL_IS_RSTRIP */

		/* Strip around line-feeds. */
#ifdef LOCAL_IS_LINES
		flush_start.cp16 = newstr.cp16;
		newend.cp16      = newstr.cp16 + newlen;
		while (newstr.cp16 < newend.cp16) {
			uint16_t ch = *newstr.cp16;
			if (!DeeUni_IsLF(ch)) {
				++newstr.cp16;
				continue;
			}

#ifdef LOCAL_IS_RSTRIP
			/* rstrip (at the end of lines) */
			{
				union dcharptr flush_end;
				flush_end.cp16 = newstr.cp16;
				while (LOCAL_MEMENDSWITHW(flush_start.cp16, (size_t)(flush_end.cp16 - flush_start.cp16))) {
					flush_end.cp16 -= LOCAL_matchlen;
				}
				if (flush_end.cp16 < newstr.cp16) {
					if unlikely(unicode_printer_print16(&printer, flush_start.cp16,
					                                    (size_t)(flush_end.cp16 - flush_start.cp16)) < 0)
						goto err;
					flush_start.cp16 = newstr.cp16; /* Skip over whitespace */
				}
			}
#endif /* LOCAL_IS_RSTRIP */

			/* Skip over the linefeed (preserving it) */
			++newstr.cp16;
			if (ch == UNICODE_CR && newstr.cp16 < newend.cp16 && *newstr.cp16 == UNICODE_LF)
				++newstr.cp16; /* Deal with CRLF-style linefeeds */

#ifdef LOCAL_IS_LSTRIP
			/* lstrip (at the start of lines) */
			{
				union dcharptr new_flush_start;
				new_flush_start.cp16 = newstr.cp16;
				while (LOCAL_MEMSTARTSWITHW(new_flush_start.cp16, (size_t)(newend.cp16 - new_flush_start.cp16))) {
					new_flush_start.cp16 += LOCAL_matchlen;
				}
				if (new_flush_start.cp16 > newstr.cp16) {
					if unlikely(unicode_printer_print16(&printer, flush_start.cp16,
					                                    (size_t)(newstr.cp16 - flush_start.cp16)) < 0)
						goto err;
					flush_start.cp16 = new_flush_start.cp16; /* Skip over whitespace */
					newstr.cp16      = new_flush_start.cp16;
				}
			}
#endif /* LOCAL_IS_LSTRIP */
		}
		ASSERT(newstr.cp16 == newend.cp16);
		if (!UNICODE_PRINTER_ISEMPTY(&printer)) {
			if unlikely(unicode_printer_print16(&printer, flush_start.cp16,
			                                    (size_t)(newstr.cp16 - flush_start.cp16)) < 0)
				goto err;
			goto return_printer;
		}
		unicode_printer_fini(&printer);
#endif /* LOCAL_IS_LINES */

		if (newlen == mylen)
			goto return_self_noprinter;
		if (DeeString_WIDTH(self) == STRING_WIDTH_1BYTE) {
			uint8_t *base = DeeString_Get1Byte((DeeObject *)self);
			return (DREF String *)DeeString_New1Byte(base + (newstr.cp16 - mystr.cp16), newlen);
		}
		return (DREF String *)DeeString_New2Byte(newstr.cp16, newlen);


		/************************************************************************/
		/* 32-bit                                                               */
		/************************************************************************/
	CASE_WIDTH_4BYTE:
#ifdef LOCAL_IS_MASKED
		mystr.cp32 = DeeString_As4Byte((DeeObject *)self);
		if unlikely(!mystr.cp32)
			goto err;
		mskstr.cp32 = DeeString_As4Byte((DeeObject *)mask);
		if unlikely(!mskstr.cp32)
			goto err;
		mylen       = WSTR_LENGTH(mystr.cp32);
		msklen      = WSTR_LENGTH(mskstr.cp32);
		newstr.cp32 = mystr.cp32;
		newlen      = mylen;
#ifdef LOCAL_IS_SSTRIP
		if unlikely(!msklen)
			goto return_self;
#endif /* LOCAL_IS_SSTRIP */
#endif /* LOCAL_IS_MASKED */
#ifdef LOCAL_IS_LSTRIP
		while (LOCAL_max_count_OR_true && LOCAL_MEMSTARTSWITHL(newstr.cp32, newlen)) {
			newstr.cp32 += LOCAL_matchlen;
			newlen -= LOCAL_matchlen;
			LOCAL_max_count_dec();
		}
#endif /* LOCAL_IS_LSTRIP */
#ifdef LOCAL_IS_RSTRIP
		while (LOCAL_max_count_OR_true && LOCAL_MEMENDSWITHL(newstr.cp32, newlen)) {
			newlen -= LOCAL_matchlen;
			LOCAL_max_count_dec();
		}
#endif /* LOCAL_IS_RSTRIP */

		/* Strip around line-feeds. */
#ifdef LOCAL_IS_LINES
		flush_start.cp32 = newstr.cp32;
		newend.cp32      = newstr.cp32 + newlen;
		while (newstr.cp32 < newend.cp32) {
			uint32_t ch = *newstr.cp32;
			if (!DeeUni_IsLF(ch)) {
				++newstr.cp32;
				continue;
			}

#ifdef LOCAL_IS_RSTRIP
			/* rstrip (at the end of lines) */
			{
				union dcharptr flush_end;
				flush_end.cp32 = newstr.cp32;
				while (LOCAL_MEMENDSWITHL(flush_start.cp32, (size_t)(flush_end.cp32 - flush_start.cp32))) {
					flush_end.cp32 -= LOCAL_matchlen;
				}
				if (flush_end.cp32 < newstr.cp32) {
					if unlikely(unicode_printer_print32(&printer, flush_start.cp32,
					                                    (size_t)(flush_end.cp32 - flush_start.cp32)) < 0)
						goto err;
					flush_start.cp32 = newstr.cp32; /* Skip over whitespace */
				}
			}
#endif /* LOCAL_IS_RSTRIP */

			/* Skip over the linefeed (preserving it) */
			++newstr.cp32;
			if (ch == UNICODE_CR && newstr.cp32 < newend.cp32 && *newstr.cp32 == UNICODE_LF)
				++newstr.cp32; /* Deal with CRLF-style linefeeds */

#ifdef LOCAL_IS_LSTRIP
			/* lstrip (at the start of lines) */
			{
				union dcharptr new_flush_start;
				new_flush_start.cp32 = newstr.cp32;
				while (LOCAL_MEMSTARTSWITHL(new_flush_start.cp32, (size_t)(newend.cp32 - new_flush_start.cp32))) {
					new_flush_start.cp32 += LOCAL_matchlen;
				}
				if (new_flush_start.cp32 > newstr.cp32) {
					if unlikely(unicode_printer_print32(&printer, flush_start.cp32,
					                                    (size_t)(newstr.cp32 - flush_start.cp32)) < 0)
						goto err;
					flush_start.cp32 = new_flush_start.cp32; /* Skip over whitespace */
					newstr.cp32      = new_flush_start.cp32;
				}
			}
#endif /* LOCAL_IS_LSTRIP */
		}
		ASSERT(newstr.cp32 == newend.cp32);
		if (!UNICODE_PRINTER_ISEMPTY(&printer)) {
			if unlikely(unicode_printer_print32(&printer, flush_start.cp32,
			                                    (size_t)(newstr.cp32 - flush_start.cp32)) < 0)
				goto err;
			goto return_printer;
		}
		unicode_printer_fini(&printer);
#endif /* LOCAL_IS_LINES */

		if (newlen == mylen)
			goto return_self_noprinter;
		if (DeeString_WIDTH(self) == STRING_WIDTH_1BYTE) {
			uint8_t *base = DeeString_Get1Byte((DeeObject *)self);
			return (DREF String *)DeeString_New1Byte(base + (newstr.cp32 - mystr.cp32), newlen);
		}
		if (DeeString_WIDTH(self) == STRING_WIDTH_2BYTE) {
			uint16_t *base = DeeString_Get2Byte((DeeObject *)self);
			return (DREF String *)DeeString_New2Byte(base + (newstr.cp32 - mystr.cp32), newlen);
		}
		return (DREF String *)DeeString_New4Byte(newstr.cp32, newlen);

#undef LOCAL_MEMSTARTSWITHB
#undef LOCAL_MEMSTARTSWITHW
#undef LOCAL_MEMSTARTSWITHL
#undef LOCAL_MEMENDSWITHB
#undef LOCAL_MEMENDSWITHW
#undef LOCAL_MEMENDSWITHL
#undef LOCAL_matchlen
#undef LOCAL_isspace_b
#undef LOCAL_isspace_w
#undef LOCAL_isspace_l
	}
#ifdef LOCAL_IS_SSTRIP
return_self:
#ifdef LOCAL_IS_LINES
	unicode_printer_fini(&printer);
#endif /* LOCAL_IS_LINES */
#endif /* LOCAL_IS_SSTRIP */
return_self_noprinter:
	return_reference_(self);
#ifdef LOCAL_IS_LINES
return_printer:
	return (DREF String *)unicode_printer_pack(&printer);
err:
	unicode_printer_fini(&printer);
	return NULL;
#elif defined(LOCAL_IS_MASKED)
err:
	return NULL;
#endif /* ... */
}

#undef LOCAL_max_count_OR_true
#undef LOCAL_max_count_dec
#undef LOCAL__PARAM_max_count
#undef LOCAL_DeeString_Strip
#undef LOCAL_HAVE_max_count
#undef LOCAL_IS_LINES
#undef LOCAL_IS_LSTRIP
#undef LOCAL_IS_RSTRIP
#undef LOCAL_IS_SSTRIP
#undef LOCAL_IS_MASKED
#undef LOCAL_IS_NOCASE

DECL_END

#undef DEFINE_DeeString_CaseRSStripLines
#undef DEFINE_DeeString_CaseLSStripLines
#undef DEFINE_DeeString_CaseSStripLines
#undef DEFINE_DeeString_RSStripLines
#undef DEFINE_DeeString_LSStripLines
#undef DEFINE_DeeString_SStripLines
#undef DEFINE_DeeString_CaseRStripLinesMask
#undef DEFINE_DeeString_CaseLStripLinesMask
#undef DEFINE_DeeString_CaseStripLinesMask
#undef DEFINE_DeeString_RStripLinesMask
#undef DEFINE_DeeString_LStripLinesMask
#undef DEFINE_DeeString_StripLinesMask
#undef DEFINE_DeeString_RStripLinesSpc
#undef DEFINE_DeeString_LStripLinesSpc
#undef DEFINE_DeeString_StripLinesSpc
#undef DEFINE_DeeString_CaseRSStrip
#undef DEFINE_DeeString_CaseLSStrip
#undef DEFINE_DeeString_CaseSStrip
#undef DEFINE_DeeString_RSStrip
#undef DEFINE_DeeString_LSStrip
#undef DEFINE_DeeString_SStrip
#undef DEFINE_DeeString_CaseRStripMask
#undef DEFINE_DeeString_CaseLStripMask
#undef DEFINE_DeeString_CaseStripMask
#undef DEFINE_DeeString_RStripMask
#undef DEFINE_DeeString_LStripMask
#undef DEFINE_DeeString_StripMask
#undef DEFINE_DeeString_RStripSpc
#undef DEFINE_DeeString_LStripSpc
#undef DEFINE_DeeString_StripSpc
