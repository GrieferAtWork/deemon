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
#ifdef __INTELLISENSE__
#include "string_functions.c"
#define DEFINE_DeeString_StripSpc
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
#endif /* __INTELLISENSE__ */

DECL_BEGIN

#if (defined(DEFINE_DeeString_StripSpc) +       \
     defined(DEFINE_DeeString_LStripSpc) +      \
     defined(DEFINE_DeeString_RStripSpc) +      \
     defined(DEFINE_DeeString_StripMask) +      \
     defined(DEFINE_DeeString_LStripMask) +     \
     defined(DEFINE_DeeString_RStripMask) +     \
     defined(DEFINE_DeeString_CaseStripMask) +  \
     defined(DEFINE_DeeString_CaseLStripMask) + \
     defined(DEFINE_DeeString_CaseRStripMask) + \
     defined(DEFINE_DeeString_SStrip) +         \
     defined(DEFINE_DeeString_LSStrip) +        \
     defined(DEFINE_DeeString_RSStrip) +        \
     defined(DEFINE_DeeString_CaseSStrip) +     \
     defined(DEFINE_DeeString_CaseLSStrip) +    \
     defined(DEFINE_DeeString_CaseRSStrip)) != 1
#error "Must #define exactly one of these macros!"
#endif /* ... */

#ifdef DEFINE_DeeString_StripSpc
#define LOCAL_DeeString_Strip DeeString_StripSpc
#define LOCAL_IS_LSTRIP
#define LOCAL_IS_RSTRIP
#elif defined(DEFINE_DeeString_LStripSpc)
#define LOCAL_DeeString_Strip DeeString_LStripSpc
#define LOCAL_IS_LSTRIP
#elif defined(DEFINE_DeeString_RStripSpc)
#define LOCAL_DeeString_Strip DeeString_RStripSpc
#define LOCAL_IS_RSTRIP
#elif defined(DEFINE_DeeString_StripMask)
#define LOCAL_DeeString_Strip DeeString_StripMask
#define LOCAL_IS_LSTRIP
#define LOCAL_IS_RSTRIP
#define LOCAL_IS_MASKED
#elif defined(DEFINE_DeeString_LStripMask)
#define LOCAL_DeeString_Strip DeeString_LStripMask
#define LOCAL_IS_LSTRIP
#define LOCAL_IS_MASKED
#elif defined(DEFINE_DeeString_RStripMask)
#define LOCAL_DeeString_Strip DeeString_RStripMask
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
#define LOCAL_IS_LSTRIP
#define LOCAL_IS_MASKED
#define LOCAL_IS_NOCASE
#elif defined(DEFINE_DeeString_CaseRStripMask)
#define LOCAL_DeeString_Strip DeeString_CaseRStripMask
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
#define LOCAL_IS_LSTRIP
#define LOCAL_IS_SSTRIP
#elif defined(DEFINE_DeeString_RSStrip)
#define LOCAL_DeeString_Strip DeeString_RSStrip
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
#define LOCAL_IS_LSTRIP
#define LOCAL_IS_SSTRIP
#define LOCAL_IS_NOCASE
#elif defined(DEFINE_DeeString_CaseRSStrip)
#define LOCAL_DeeString_Strip DeeString_CaseRSStrip
#define LOCAL_IS_RSTRIP
#define LOCAL_IS_SSTRIP
#define LOCAL_IS_NOCASE
#endif

#ifdef LOCAL_IS_SSTRIP
#define LOCAL_IS_MASKED
#endif /* LOCAL_IS_SSTRIP */

#ifdef LOCAL_IS_MASKED
PRIVATE WUNUSED NONNULL((1, 2)) DREF String *DCALL
LOCAL_DeeString_Strip(String *self, String *mask)
#else /* LOCAL_IS_MASKED */
PRIVATE WUNUSED NONNULL((1)) DREF String *DCALL
LOCAL_DeeString_Strip(String *__restrict self)
#endif /* !LOCAL_IS_MASKED */
{
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
#endif /* LOCAL_IS_NOCASE */
#elif defined(LOCAL_IS_MASKED)
#ifdef LOCAL_IS_NOCASE
#define LOCAL_isspace_b(ch) memcasechrb(mskstr.cp8, ch, msklen)
#define LOCAL_isspace_w(ch) memcasechrw(mskstr.cp16, ch, msklen)
#define LOCAL_isspace_l(ch) memcasechrl(mskstr.cp32, ch, msklen)
#else /* LOCAL_IS_NOCASE */
#define LOCAL_isspace_b(ch) memchrb(mskstr.cp8, ch, msklen)
#define LOCAL_isspace_w(ch) memchrw(mskstr.cp16, ch, msklen)
#define LOCAL_isspace_l(ch) memchrl(mskstr.cp32, ch, msklen)
#endif /* !LOCAL_IS_NOCASE */
#else /* LOCAL_IS_MASKED */
#define LOCAL_isspace_b DeeUni_IsSpace
#define LOCAL_isspace_w DeeUni_IsSpace
#define LOCAL_isspace_l DeeUni_IsSpace
#endif /* !LOCAL_IS_MASKED */

		/* 8-bit */
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
#ifdef LOCAL_IS_SSTRIP
#ifdef LOCAL_IS_NOCASE
		while ((matchlen = MEMCASESTARTSWITHB(newstr.cp8, newlen, mskstr.cp8, msklen)) != 0) {
			newstr.cp8 += matchlen;
			newlen -= matchlen;
		}
#else /* LOCAL_IS_NOCASE */
		while (newlen >= msklen &&
		       MEMEQB(newstr.cp8, mskstr.cp8, msklen)) {
			newstr.cp8 += msklen;
			newlen -= msklen;
		}
#endif /* !LOCAL_IS_NOCASE */
#else /* LOCAL_IS_SSTRIP */
		while (newlen && LOCAL_isspace_b(newstr.cp8[0])) {
			++newstr.cp8;
			--newlen;
		}
#endif /* !LOCAL_IS_SSTRIP */
#endif /* LOCAL_IS_LSTRIP */
#ifdef LOCAL_IS_RSTRIP
#ifdef LOCAL_IS_SSTRIP
#ifdef LOCAL_IS_NOCASE
		while ((matchlen = MEMCASEENDSWITHB(newstr.cp8, newlen, mskstr.cp8, msklen)) != 0) {
			newlen -= matchlen;
		}
#else /* LOCAL_IS_NOCASE */
		while (newlen >= msklen &&
		       MEMEQB(newstr.cp8 + (newlen - msklen),
		              mskstr.cp8, msklen)) {
			newlen -= msklen;
		}
#endif /* !LOCAL_IS_NOCASE */
#else /* LOCAL_IS_SSTRIP */
		while (newlen && LOCAL_isspace_b(newstr.cp8[newlen - 1])) {
			--newlen;
		}
#endif /* !LOCAL_IS_SSTRIP */
#endif /* LOCAL_IS_RSTRIP */
		if (newlen == mylen)
			goto return_self;
		return (DREF String *)DeeString_New1Byte(newstr.cp8, newlen);


		/* 16-bit */
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
#ifdef LOCAL_IS_SSTRIP
#ifdef LOCAL_IS_NOCASE
		while ((matchlen = MEMCASESTARTSWITHW(newstr.cp16, newlen, mskstr.cp16, msklen)) != 0) {
			newstr.cp16 += matchlen;
			newlen -= matchlen;
		}
#else /* LOCAL_IS_NOCASE */
		while (newlen >= msklen &&
		       MEMEQW(newstr.cp16, mskstr.cp16, msklen)) {
			newstr.cp16 += msklen;
			newlen -= msklen;
		}
#endif /* !LOCAL_IS_NOCASE */
#else /* LOCAL_IS_SSTRIP */
		while (newlen && LOCAL_isspace_w(newstr.cp16[0])) {
			++newstr.cp16;
			--newlen;
		}
#endif /* !LOCAL_IS_SSTRIP */
#endif /* LOCAL_IS_LSTRIP */
#ifdef LOCAL_IS_RSTRIP
#ifdef LOCAL_IS_SSTRIP
#ifdef LOCAL_IS_NOCASE
		while ((matchlen = MEMCASEENDSWITHW(newstr.cp16, newlen, mskstr.cp16, msklen)) != 0) {
			newlen -= matchlen;
		}
#else /* LOCAL_IS_NOCASE */
		while (newlen >= msklen &&
		       MEMEQW(newstr.cp16 + (newlen - msklen),
		              mskstr.cp16, msklen)) {
			newlen -= msklen;
		}
#endif /* !LOCAL_IS_NOCASE */
#else /* LOCAL_IS_SSTRIP */
		while (newlen && LOCAL_isspace_w(newstr.cp16[newlen - 1])) {
			--newlen;
		}
#endif /* !LOCAL_IS_SSTRIP */
#endif /* LOCAL_IS_RSTRIP */
		if (newlen == mylen)
			goto return_self;
		if (DeeString_WIDTH(self) == STRING_WIDTH_1BYTE) {
			uint8_t *base = DeeString_Get1Byte((DeeObject *)self);
			return (DREF String *)DeeString_New1Byte(base + (newstr.cp16 - mystr.cp16), newlen);
		}
		return (DREF String *)DeeString_New2Byte(newstr.cp16, newlen);


		/* 32-bit */
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
#ifdef LOCAL_IS_SSTRIP
#ifdef LOCAL_IS_NOCASE
		while ((matchlen = MEMCASESTARTSWITHL(newstr.cp32, newlen, mskstr.cp32, msklen)) != 0) {
			newstr.cp32 += matchlen;
			newlen -= matchlen;
		}
#else /* LOCAL_IS_NOCASE */
		while (newlen >= msklen &&
		       MEMEQL(newstr.cp32, mskstr.cp32, msklen)) {
			newstr.cp32 += msklen;
			newlen -= msklen;
		}
#endif /* !LOCAL_IS_NOCASE */
#else /* LOCAL_IS_SSTRIP */
		while (newlen && LOCAL_isspace_l(newstr.cp32[0])) {
			++newstr.cp32;
			--newlen;
		}
#endif /* !LOCAL_IS_SSTRIP */
#endif /* LOCAL_IS_LSTRIP */
#ifdef LOCAL_IS_RSTRIP
#ifdef LOCAL_IS_SSTRIP
#ifdef LOCAL_IS_NOCASE
		while ((matchlen = MEMCASEENDSWITHL(newstr.cp32, newlen, mskstr.cp32, msklen)) != 0) {
			newlen -= matchlen;
		}
#else /* LOCAL_IS_NOCASE */
		while (newlen >= msklen &&
		       MEMEQL(newstr.cp32 + (newlen - msklen),
		              mskstr.cp32, msklen)) {
			newlen -= msklen;
		}
#endif /* !LOCAL_IS_NOCASE */
#else /* LOCAL_IS_SSTRIP */
		while (newlen && LOCAL_isspace_l(newstr.cp32[newlen - 1])) {
			--newlen;
		}
#endif /* !LOCAL_IS_SSTRIP */
#endif /* LOCAL_IS_RSTRIP */
		if (newlen == mylen)
			goto return_self;
		if (DeeString_WIDTH(self) == STRING_WIDTH_1BYTE) {
			uint8_t *base = DeeString_Get1Byte((DeeObject *)self);
			return (DREF String *)DeeString_New1Byte(base + (newstr.cp32 - mystr.cp32), newlen);
		}
		if (DeeString_WIDTH(self) == STRING_WIDTH_2BYTE) {
			uint16_t *base = DeeString_Get2Byte((DeeObject *)self);
			return (DREF String *)DeeString_New2Byte(base + (newstr.cp32 - mystr.cp32), newlen);
		}
		return (DREF String *)DeeString_New4Byte(newstr.cp32, newlen);

#undef LOCAL_isspace_b
#undef LOCAL_isspace_w
#undef LOCAL_isspace_l
	}
return_self:
	return_reference_(self);
#ifdef LOCAL_IS_MASKED
err:
	return NULL;
#endif /* LOCAL_IS_MASKED */
}

#undef LOCAL_DeeString_Strip
#undef LOCAL_IS_LSTRIP
#undef LOCAL_IS_RSTRIP
#undef LOCAL_IS_SSTRIP
#undef LOCAL_IS_MASKED
#undef LOCAL_IS_NOCASE

DECL_END

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
