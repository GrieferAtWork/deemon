/* Copyright (c) 2018-2025 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2025 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_STRINGUTILS_H
#define GUARD_DEEMON_STRINGUTILS_H 1

#include "api.h"
/**/

#include "object.h"
#include "string.h"
/**/

#include <hybrid/byteorder.h>
#include <hybrid/__byteswap.h>
/**/

#include <stddef.h> /* size_t */

#ifdef DEE_SOURCE
#ifdef CONFIG_NO_UNICODE_H
#undef CONFIG_HAVE_UNICODE_H
#elif !defined(CONFIG_HAVE_UNICODE_H) && \
      (__has_include(<unicode.h>) || (defined(__NO_has_include) && \
      (defined(__KOS__) && __KOS_VERSION__ >= 400)))
#define CONFIG_HAVE_UNICODE_H
#endif

/* Must include KOS's `<unicode.h>' first since we kind-of
 * re-define some of its symbols when `DEE_SOURCE' is enabled. */
#ifdef CONFIG_HAVE_UNICODE_H
#include <unicode.h>
#endif /* CONFIG_HAVE_UNICODE_H */
#endif /* DEE_SOURCE */


DECL_BEGIN

#ifdef DEE_SOURCE
#undef unicode_utf8seqlen
#undef unicode_readutf8_n
#undef unicode_readutf8
#undef unicode_readutf8_rev
#undef unicode_readutf8_rev_n
#undef unicode_skiputf8
#undef unicode_skiputf8_c
#undef unicode_writeutf8
#undef unicode_skipspaceutf8
#undef unicode_skipspaceutf8_n
#undef unicode_skipspaceutf8_rev
#undef unicode_skipspaceutf8_rev_n
#undef unicode_readutf16_n
#undef unicode_readutf32_n
#undef unicode_readutf16_swap_n
#undef unicode_readutf32_swap_n
#undef unicode_readutf16_rev_n
#undef unicode_readutf32_rev_n
#undef unicode_readutf16_swap_rev_n
#undef unicode_readutf32_swap_rev_n
#undef unicode_readutf16le_n
#undef unicode_readutf32le_n
#undef unicode_readutf16be_n
#undef unicode_readutf32be_n
#undef unicode_readutf16le_rev_n
#undef unicode_readutf32le_rev_n
#undef unicode_readutf16be_rev_n
#undef unicode_readutf32be_rev_n
#undef UNICODE_UTF8_MAXLEN
#undef UNICODE_UTF8_CURLEN
#undef UNICODE_UTF8_DEFLEN
#define unicode_utf8seqlen           Dee_unicode_utf8seqlen
#define unicode_readutf8_n           Dee_unicode_readutf8_n
#define unicode_readutf8             Dee_unicode_readutf8
#define unicode_readutf8_rev         Dee_unicode_readutf8_rev
#define unicode_readutf8_rev_n       Dee_unicode_readutf8_rev_n
#define unicode_skiputf8             Dee_unicode_skiputf8
#define unicode_skiputf8_c           Dee_unicode_skiputf8_c
#define unicode_writeutf8            Dee_unicode_writeutf8
#define unicode_skipspaceutf8        Dee_unicode_skipspaceutf8
#define unicode_skipspaceutf8_n      Dee_unicode_skipspaceutf8_n
#define unicode_skipspaceutf8_rev    Dee_unicode_skipspaceutf8_rev
#define unicode_skipspaceutf8_rev_n  Dee_unicode_skipspaceutf8_rev_n
#define unicode_readutf16_n          Dee_unicode_readutf16_n
#define unicode_readutf32_n          Dee_unicode_readutf32_n
#define unicode_readutf16_swap_n     Dee_unicode_readutf16_swap_n
#define unicode_readutf32_swap_n     Dee_unicode_readutf32_swap_n
#define unicode_readutf16_rev_n      Dee_unicode_readutf16_rev_n
#define unicode_readutf32_rev_n      Dee_unicode_readutf32_rev_n
#define unicode_readutf16_swap_rev_n Dee_unicode_readutf16_swap_rev_n
#define unicode_readutf32_swap_rev_n Dee_unicode_readutf32_swap_rev_n
#define unicode_readutf16le_n        Dee_unicode_readutf16le_n
#define unicode_readutf32le_n        Dee_unicode_readutf32le_n
#define unicode_readutf16be_n        Dee_unicode_readutf16be_n
#define unicode_readutf32be_n        Dee_unicode_readutf32be_n
#define unicode_readutf16le_rev_n    Dee_unicode_readutf16le_rev_n
#define unicode_readutf32le_rev_n    Dee_unicode_readutf32le_rev_n
#define unicode_readutf16be_rev_n    Dee_unicode_readutf16be_rev_n
#define unicode_readutf32be_rev_n    Dee_unicode_readutf32be_rev_n
#define UNICODE_UTF8_MAXLEN          Dee_UNICODE_UTF8_MAXLEN
#define UNICODE_UTF8_CURLEN          Dee_UNICODE_UTF8_CURLEN
#define UNICODE_UTF8_DEFLEN          Dee_UNICODE_UTF8_DEFLEN
#endif /* DEE_SOURCE */

#define Dee_UNICODE_UTF8_MAXLEN 8 /* The max length of a UTF-8 multi-byte sequence (100% future-proof, as this is the theoretical limit) */
#define Dee_UNICODE_UTF8_CURLEN 7 /* Enough for any 32-bit unicode character ordinal */
#define Dee_UNICODE_UTF8_DEFLEN 4 /* Enough for any currently defined unicode character ordinal */


/* UTF-8 helper API */
DDATDEF uint8_t const Dee_unicode_utf8seqlen[256];
#ifdef __INTELLISENSE__
extern "C++" {
WUNUSED ATTR_INOUT(1) uint32_t DCALL Dee_unicode_readutf8(unsigned char const **__restrict ptext);
WUNUSED ATTR_INOUT(1) uint32_t DCALL Dee_unicode_readutf8(unsigned char **__restrict ptext);
WUNUSED ATTR_INOUT(1) uint32_t DCALL Dee_unicode_readutf8(char const **__restrict ptext);
WUNUSED ATTR_INOUT(1) uint32_t DCALL Dee_unicode_readutf8(char **__restrict ptext);
ATTR_INOUT(1) NONNULL((2)) uint32_t DCALL Dee_unicode_readutf8_n(unsigned char const **__restrict ptext, unsigned char const *text_end);
ATTR_INOUT(1) NONNULL((2)) uint32_t DCALL Dee_unicode_readutf8_n(unsigned char const **__restrict ptext, char const *text_end);
ATTR_INOUT(1) NONNULL((2)) uint32_t DCALL Dee_unicode_readutf8_n(unsigned char **__restrict ptext, unsigned char const *text_end);
ATTR_INOUT(1) NONNULL((2)) uint32_t DCALL Dee_unicode_readutf8_n(unsigned char **__restrict ptext, char const *text_end);
ATTR_INOUT(1) NONNULL((2)) uint32_t DCALL Dee_unicode_readutf8_n(char const **__restrict ptext, unsigned char const *text_end);
ATTR_INOUT(1) NONNULL((2)) uint32_t DCALL Dee_unicode_readutf8_n(char const **__restrict ptext, char const *text_end);
ATTR_INOUT(1) NONNULL((2)) uint32_t DCALL Dee_unicode_readutf8_n(char **__restrict ptext, unsigned char const *text_end);
ATTR_INOUT(1) NONNULL((2)) uint32_t DCALL Dee_unicode_readutf8_n(char **__restrict ptext, char const *text_end);
ATTR_INOUT(1) uint32_t DCALL Dee_unicode_readutf8_rev(unsigned char const **__restrict ptext);
ATTR_INOUT(1) uint32_t DCALL Dee_unicode_readutf8_rev(unsigned char **__restrict ptext);
ATTR_INOUT(1) uint32_t DCALL Dee_unicode_readutf8_rev(char const **__restrict ptext);
ATTR_INOUT(1) uint32_t DCALL Dee_unicode_readutf8_rev(char **__restrict ptext);
ATTR_INOUT(1) uint32_t DCALL Dee_unicode_readutf8_rev_n(unsigned char const **__restrict ptext, unsigned char const *text_start);
ATTR_INOUT(1) uint32_t DCALL Dee_unicode_readutf8_rev_n(unsigned char const **__restrict ptext, char const *text_start);
ATTR_INOUT(1) uint32_t DCALL Dee_unicode_readutf8_rev_n(unsigned char **__restrict ptext, unsigned char const *text_start);
ATTR_INOUT(1) uint32_t DCALL Dee_unicode_readutf8_rev_n(unsigned char **__restrict ptext, char const *text_start);
ATTR_INOUT(1) uint32_t DCALL Dee_unicode_readutf8_rev_n(char const **__restrict ptext, unsigned char const *text_start);
ATTR_INOUT(1) uint32_t DCALL Dee_unicode_readutf8_rev_n(char const **__restrict ptext, char const *text_start);
ATTR_INOUT(1) uint32_t DCALL Dee_unicode_readutf8_rev_n(char **__restrict ptext, unsigned char const *text_start);
ATTR_INOUT(1) uint32_t DCALL Dee_unicode_readutf8_rev_n(char **__restrict ptext, char const *text_start);
ATTR_INOUT(1) char *DCALL Dee_unicode_skiputf8(unsigned char const *__restrict text);
ATTR_INOUT(1) char *DCALL Dee_unicode_skiputf8(unsigned char *__restrict text);
ATTR_INOUT(1) char *DCALL Dee_unicode_skiputf8(char const *__restrict text);
ATTR_INOUT(1) char *DCALL Dee_unicode_skiputf8(char *__restrict text);
ATTR_INOUT(1) char *DCALL Dee_unicode_skiputf8_c(unsigned char const *__restrict text, size_t count);
ATTR_INOUT(1) char *DCALL Dee_unicode_skiputf8_c(unsigned char *__restrict text, size_t count);
ATTR_INOUT(1) char *DCALL Dee_unicode_skiputf8_c(char const *__restrict text, size_t count);
ATTR_INOUT(1) char *DCALL Dee_unicode_skiputf8_c(char *__restrict text, size_t count);
} /* extern "C++" */
#else /* __INTELLISENSE__ */
DFUNDEF WUNUSED ATTR_INOUT(1) uint32_t (DCALL Dee_unicode_readutf8)(char const **__restrict ptext);
DFUNDEF ATTR_INOUT(1) NONNULL((2)) uint32_t (DCALL Dee_unicode_readutf8_n)(char const **__restrict ptext, char const *text_end);
DFUNDEF ATTR_INOUT(1) uint32_t (DCALL Dee_unicode_readutf8_rev_n)(char const **__restrict ptext, char const *text_start);
#define Dee_unicode_readutf8(ptext)                   (Dee_unicode_readutf8)((char const **)(ptext))
#define Dee_unicode_readutf8_n(ptext, text_end)       (Dee_unicode_readutf8_n)((char const **)(ptext), (char const *)(text_end))
#define Dee_unicode_readutf8_rev(ptext)               (Dee_unicode_readutf8_rev_n)((char const **)(ptext), (char const *)0)
#define Dee_unicode_readutf8_rev_n(ptext, text_start) (Dee_unicode_readutf8_rev_n)((char const **)(ptext), (char const *)(text_start))
#define Dee_unicode_skiputf8(text)                    ((char *)(text) + Dee_unicode_utf8seqlen[(uint8_t)(*(text))])
#define Dee_unicode_skiputf8_c(text, count)           (Dee_unicode_skiputf8_c)((char const *)(text), count)
LOCAL ATTR_RETNONNULL WUNUSED NONNULL((1)) char *
(DCALL Dee_unicode_skiputf8_c)(char const *__restrict text, size_t count) {
	while (count--)
		text += Dee_unicode_utf8seqlen[(uint8_t)*text];
	return (char *)text;
}
#endif /* !__INTELLISENSE__ */

/* Up to `Dee_UNICODE_UTF8_CURLEN' bytes may be used in `buffer' */
DFUNDEF ATTR_RETNONNULL WUNUSED NONNULL((1)) char *
(DCALL Dee_unicode_writeutf8)(char *__restrict buffer, uint32_t ch);

/* Determine the length of folded unicode text. */
#ifdef __INTELLISENSE__
extern "C++" {
WUNUSED ATTR_INS(1, 2) size_t (DCALL DeeUni_FoldedLength)(uint8_t const *__restrict text, size_t length);
WUNUSED ATTR_INS(1, 2) size_t (DCALL DeeUni_FoldedLength)(uint16_t const *__restrict text, size_t length);
WUNUSED ATTR_INS(1, 2) size_t (DCALL DeeUni_FoldedLength)(uint32_t const *__restrict text, size_t length);
} /* extern "C++" */
#else /* __INTELLISENSE__ */
#define DeeUni_FoldedLength(text, length)                                              \
	(sizeof(*(text)) == 1 ? _DeeUni_FoldedLength_b((uint8_t const *)(text), length) :  \
	 sizeof(*(text)) == 2 ? _DeeUni_FoldedLength_w((uint16_t const *)(text), length) : \
	                        _DeeUni_FoldedLength_l((uint32_t const *)(text), length))
LOCAL WUNUSED ATTR_INS(1, 2) size_t
(DCALL _DeeUni_FoldedLength_b)(uint8_t const *__restrict text, size_t length) {
	size_t i, result = 0;
	uint32_t buf[Dee_UNICODE_FOLDED_MAX];
	for (i = 0; i < length; ++i)
		result += DeeUni_ToFolded(text[i], buf);
	return result;
}

LOCAL WUNUSED ATTR_INS(1, 2) size_t
(DCALL _DeeUni_FoldedLength_w)(uint16_t const *__restrict text, size_t length) {
	size_t i, result = 0;
	uint32_t buf[Dee_UNICODE_FOLDED_MAX];
	for (i = 0; i < length; ++i)
		result += DeeUni_ToFolded(text[i], buf);
	return result;
}

LOCAL WUNUSED ATTR_INS(1, 2) size_t
(DCALL _DeeUni_FoldedLength_l)(uint32_t const *__restrict text, size_t length) {
	size_t i, result = 0;
	uint32_t buf[Dee_UNICODE_FOLDED_MAX];
	for (i = 0; i < length; ++i)
		result += DeeUni_ToFolded(text[i], buf);
	return result;
}
#endif /* !__INTELLISENSE__ */


/* Skip whitespace in UTF-8 */
LOCAL WUNUSED NONNULL((1, 2)) char *
(DCALL Dee_unicode_skipspaceutf8_n)(char const *text, char const *end) {
	char *result;
	for (;;) {
		uint32_t chr;
		result = (char *)text;
		chr    = Dee_unicode_readutf8_n(&text, end);
		if (!DeeUni_IsSpace(chr))
			break;
	}
	return result;
}

LOCAL WUNUSED NONNULL((1)) char *
(DCALL Dee_unicode_skipspaceutf8)(char const *text) {
	char *result;
	for (;;) {
		uint32_t chr;
		result = (char *)text;
		chr    = Dee_unicode_readutf8(&text);
		if (!DeeUni_IsSpace(chr))
			break;
	}
	return result;
}

#define Dee_unicode_skipspaceutf8_rev(text) \
	Dee_unicode_skipspaceutf8_rev_n(text, (char const *)0)
LOCAL WUNUSED NONNULL((1)) char *
(DCALL Dee_unicode_skipspaceutf8_rev_n)(char const *text, char const *text_start) {
	char *result;
	for (;;) {
		uint32_t chr;
		result = (char *)text;
		chr    = Dee_unicode_readutf8_rev_n(&text, text_start);
		if (!DeeUni_IsSpace(chr))
			break;
	}
	return result;
}


/* Read characters from UTF-16 and UTF-32 sources */
LOCAL ATTR_INOUT(1) NONNULL((2)) uint32_t
NOTHROW_NCX(DCALL Dee_unicode_readutf16_n)(uint16_t const **__restrict ptext,
                                           uint16_t const *text_end) {
	uint32_t result;
	uint16_t const *text = *ptext;
	if (text >= text_end)
		return 0;
	result = (uint32_t)(uint16_t)*text++;
	if (result >= 0xd800 &&
	    result <= 0xdbff &&
	    text < text_end) {
		result -= 0xd800;
		result <<= 10;
		result += 0x10000 - 0xdc00;
		result += *text++; /* low surrogate */
	}
	*ptext = text;
	return result;
}

LOCAL ATTR_INOUT(1) NONNULL((2)) uint32_t
NOTHROW_NCX(DCALL Dee_unicode_readutf16_swap_n)(uint16_t const **__restrict ptext,
                                                uint16_t const *text_end) {
	uint32_t result;
	uint16_t const *text = *ptext;
	if (text >= text_end)
		return 0;
	result = (uint32_t)__hybrid_bswap16((uint16_t)*text);
	++text;
	if (result >= 0xd800 &&
	    result <= 0xdbff &&
	    text < text_end) {
		result -= 0xd800;
		result <<= 10;
		result += 0x10000 - 0xdc00;
		result += __hybrid_bswap16(*text); /* low surrogate */
		++text;
	}
	*ptext = text;
	return result;
}

LOCAL ATTR_INOUT(1) NONNULL((2)) uint32_t
NOTHROW_NCX(DCALL Dee_unicode_readutf16_rev_n)(uint16_t const **__restrict ptext,
                                               uint16_t const *text_start) {
	uint32_t result;
	uint16_t const *text = *ptext;
	if (text <= text_start)
		return 0;
	result = (uint32_t)(uint16_t)*--text;
	if (result >= 0xdc00 &&
	    result <= 0xdfff && likely(text > text_start)) {
		uint32_t high = *--text;
		high   -= 0xd800;
		high   <<= 10;
		high   += 0x10000 - 0xdc00;
		result += high;
	}
	*ptext = text;
	return result;
}

LOCAL ATTR_IN(2) ATTR_INOUT(1) uint32_t
NOTHROW_NCX(DCALL Dee_unicode_readutf16_swap_rev_n)(uint16_t const **__restrict ptext,
                                                    uint16_t const *text_start) {
	uint32_t result;
	uint16_t const *text = *ptext;
	if (text <= text_start)
		return 0;
	--text;
	result = (uint32_t)__hybrid_bswap16((uint16_t)*text);
	if (result >= 0xdc00 &&
	    result <= 0xdfff && likely(text > text_start)) {
		uint32_t high = (--text, __hybrid_bswap16(*text));
		high   -= 0xd800;
		high   <<= 10;
		high   += 0x10000 - 0xdc00;
		result += high;
	}
	*ptext = text;
	return result;
}

LOCAL ATTR_INOUT(1) NONNULL((2)) uint32_t
NOTHROW_NCX(DCALL Dee_unicode_readutf32_n)(/*utf-32*/ uint32_t const **__restrict ptext,
                                           uint32_t const *text_end) {
	uint32_t result;
	uint32_t const *text = *ptext;
	if (text >= text_end)
		return 0;
	result = *text++;
	*ptext = text;
	return result;
}

LOCAL ATTR_INOUT(1) NONNULL((2)) uint32_t
NOTHROW_NCX(DCALL Dee_unicode_readutf32_rev_n)(/*utf-32*/ uint32_t const **__restrict ptext,
                                               uint32_t const *text_start) {
	uint32_t result;
	uint32_t const *text = *ptext;
	if (text <= text_start)
		return 0;
	result = *--text;
	*ptext = text;
	return result;
}

LOCAL ATTR_INOUT(1) NONNULL((2)) uint32_t
NOTHROW_NCX(DCALL Dee_unicode_readutf32_swap_n)(/*utf-32*/ uint32_t const **__restrict ptext,
                                                uint32_t const *text_end) {
	uint32_t result;
	uint32_t const *text = *ptext;
	if (text >= text_end)
		return 0;
	result = *text++;
	*ptext = text;
	return __hybrid_bswap32(result);
}

LOCAL ATTR_INOUT(1) NONNULL((2)) uint32_t
NOTHROW_NCX(DCALL Dee_unicode_readutf32_swap_rev_n)(/*utf-32*/ uint32_t const **__restrict ptext,
                                                    uint32_t const *text_start) {
	uint32_t result;
	uint32_t const *text = *ptext;
	if (text <= text_start)
		return 0;
	result = *--text;
	*ptext = text;
	return __hybrid_bswap32(result);
}

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define Dee_unicode_readutf16le_n     Dee_unicode_readutf16_n
#define Dee_unicode_readutf32le_n     Dee_unicode_readutf32_n
#define Dee_unicode_readutf16be_n     Dee_unicode_readutf16_swap_n
#define Dee_unicode_readutf32be_n     Dee_unicode_readutf32_swap_n
#define Dee_unicode_readutf16le_rev_n Dee_unicode_readutf16_rev_n
#define Dee_unicode_readutf32le_rev_n Dee_unicode_readutf32_rev_n
#define Dee_unicode_readutf16be_rev_n Dee_unicode_readutf16_swap_rev_n
#define Dee_unicode_readutf32be_rev_n Dee_unicode_readutf32_swap_rev_n
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define Dee_unicode_readutf16be_n     Dee_unicode_readutf16_n
#define Dee_unicode_readutf32be_n     Dee_unicode_readutf32_n
#define Dee_unicode_readutf16le_n     Dee_unicode_readutf16_swap_n
#define Dee_unicode_readutf32le_n     Dee_unicode_readutf32_swap_n
#define Dee_unicode_readutf16be_rev_n Dee_unicode_readutf16_rev_n
#define Dee_unicode_readutf32be_rev_n Dee_unicode_readutf32_rev_n
#define Dee_unicode_readutf16le_rev_n Dee_unicode_readutf16_swap_rev_n
#define Dee_unicode_readutf32le_rev_n Dee_unicode_readutf32_swap_rev_n
#endif /* __BYTE_ORDER__ == ... */









/* Get/Set a character, given its index within the string. */
#define DeeString_GetChar(self, index)        DeeString_GetChar((DeeStringObject *)Dee_REQUIRES_OBJECT(self), index)
#define DeeString_SetChar(self, index, value) DeeString_SetChar((DeeStringObject *)Dee_REQUIRES_OBJECT(self), index, value)
DFUNDEF WUNUSED NONNULL((1)) uint32_t (DCALL DeeString_GetChar)(DeeStringObject *__restrict self, size_t index);
DFUNDEF NONNULL((1)) void (DCALL DeeString_SetChar)(DeeStringObject *__restrict self, size_t index, uint32_t value);


/* Move `num_chars' characters from `src' to `dst' */
#define DeeString_Memmove(self, dst, src, num_chars) \
	DeeString_Memmove((DeeStringObject *)Dee_REQUIRES_OBJECT(self), dst, src, num_chars)
DFUNDEF NONNULL((1)) void
(DCALL DeeString_Memmove)(DeeStringObject *__restrict self,
                          size_t dst, size_t src, size_t num_chars);


/* Helper macro to enumerate the character of a given string `self' */
#define DeeString_Foreach(self, ibegin, iend, iter, end, ...) \
	do {                                                      \
		void *_str_  = DeeString_WSTR(self);                  \
		size_t _len_ = WSTR_LENGTH(_str_);                    \
		if (_len_ > (iend))                                   \
			_len_ = (iend);                                   \
		if ((ibegin) < _len_) {                               \
			Dee_SWITCH_SIZEOF_WIDTH(DeeString_WIDTH(self)) {  \
			                                                  \
			Dee_CASE_WIDTH_1BYTE: {                           \
				uint8_t *iter, *end;                          \
				iter = (uint8_t *)_str_;                      \
				end  = (uint8_t *)_str_ + _len_;              \
				for (; iter < end; ++iter)                    \
					do __VA_ARGS__ __WHILE0;                  \
			}	break;                                        \
			                                                  \
			Dee_CASE_WIDTH_2BYTE: {                           \
				uint16_t *iter, *end;                         \
				iter = (uint16_t *)_str_;                     \
				end  = (uint16_t *)_str_ + _len_;             \
				for (; iter < end; ++iter)                    \
					do __VA_ARGS__ __WHILE0;                  \
			}	break;                                        \
			                                                  \
			Dee_CASE_WIDTH_4BYTE: {                           \
				uint32_t *iter, *end;                         \
				iter = (uint32_t *)_str_;                     \
				end  = (uint32_t *)_str_ + _len_;             \
				for (; iter < end; ++iter)                    \
					do __VA_ARGS__ __WHILE0;                  \
			}	break;                                        \
			                                                  \
			}                                                 \
		}                                                     \
	}	__WHILE0



DECL_END

#endif /* !GUARD_DEEMON_STRINGUTILS_H */
