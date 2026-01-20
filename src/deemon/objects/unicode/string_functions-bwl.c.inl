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
#include "string_functions.h"
//#define DEFINE_STRING_FUNCTIONS_CASEASCII
#define DEFINE_STRING_FUNCTIONS_B
//#define DEFINE_STRING_FUNCTIONS_W
//#define DEFINE_STRING_FUNCTIONS_L
#endif /* __INTELLISENSE__ */

#include <deemon/api.h>

#include <deemon/alloc.h>
#include <deemon/object.h>
#include <deemon/string.h>
#include <deemon/stringutils.h>
#include <deemon/system-features.h>

#include <hybrid/limitcore.h> /* __SSIZE_MAX__ */
#include <hybrid/typecore.h>  /* __SBYTE_TYPE__ */

#include <stdbool.h> /* bool, false */
#include <stddef.h>  /* NULL, size_t */
#include <stdint.h>  /* int8_t, int16_t, int32_t, uint8_t, uint16_t, uint32_t, uintptr_t */

#undef SSIZE_MAX
#define SSIZE_MAX __SSIZE_MAX__

#if (defined(DEFINE_STRING_FUNCTIONS_CASEASCII) + \
     defined(DEFINE_STRING_FUNCTIONS_B) +     \
     defined(DEFINE_STRING_FUNCTIONS_W) +     \
     defined(DEFINE_STRING_FUNCTIONS_L)) != 1
#error "Must #define exactly one of these"
#endif

#ifdef DEFINE_STRING_FUNCTIONS_CASEASCII
#define LOCAL_schar_t     __SBYTE_TYPE__
#define LOCAL_uchar_t     byte_t
#define LOCAL_cpX         cp_char
#define LOCAL_SIZEOF_CHAR 1
#define LOCAL_FUNC(x)     x##_ascii
#elif defined(DEFINE_STRING_FUNCTIONS_B)
#define LOCAL_schar_t     int8_t
#define LOCAL_uchar_t     uint8_t
#define LOCAL_cpX         cp8
#define LOCAL_SIZEOF_CHAR 1
#define LOCAL_FUNC(x)     x##b
#elif defined(DEFINE_STRING_FUNCTIONS_W)
#define LOCAL_schar_t     int16_t
#define LOCAL_uchar_t     uint16_t
#define LOCAL_cpX         cp16
#define LOCAL_SIZEOF_CHAR 2
#define LOCAL_FUNC(x)     x##w
#elif defined(DEFINE_STRING_FUNCTIONS_L)
#define LOCAL_schar_t     int32_t
#define LOCAL_uchar_t     uint32_t
#define LOCAL_cpX         cp32
#define LOCAL_SIZEOF_CHAR 4
#define LOCAL_FUNC(x)     x##l
#else /* ... */
#error "Invalid configuration"
#endif /* !... */

DECL_BEGIN

/************************************************************************/
/* PROVISIONING CONFIGURATION                                           */
/************************************************************************/
#ifdef DEFINE_STRING_FUNCTIONS_CASEASCII
#define LOCAL_fuzzy_compare            LOCAL_FUNC(dee_fuzzy_casecompare)
#define LOCAL_strverscmp               LOCAL_FUNC(dee_strcaseverscmp)
#define LOCAL_find_match               LOCAL_FUNC(dee_find_casematch)
#define LOCAL_rfind_match              LOCAL_FUNC(dee_rfind_casematch)
#define LOCAL_wildcompare              LOCAL_FUNC(dee_wildcasecompare)
#else /* DEFINE_STRING_FUNCTIONS_CASEASCII */
#define LOCAL_mempfil                  LOCAL_FUNC(dee_mempfil)
#define LOCAL_foldcmp                  LOCAL_FUNC(dee_foldcmp)
#define LOCAL_memcasechr               LOCAL_FUNC(dee_memcasechr)
/*efine LOCAL_memcaserchr              LOCAL_FUNC(dee_memcaserchr)*/
#define LOCAL_unicode_foldreader_init  LOCAL_FUNC(unicode_foldreader_init)
#define LOCAL_unicode_foldreader_getc  LOCAL_FUNC(unicode_foldreader_getc)
#define LOCAL_unicode_foldreader_rgetc LOCAL_FUNC(unicode_foldreader_rgetc)
/*efine LOCAL_memcaseeq                LOCAL_FUNC(dee_memcaseeq)*/
#define LOCAL_memcasestartswith        LOCAL_FUNC(dee_memcasestartswith)
#define LOCAL_memcaseendswith          LOCAL_FUNC(dee_memcaseendswith)
#define LOCAL_memcasemem               LOCAL_FUNC(dee_memcasemem)
#define LOCAL_memcasermem              LOCAL_FUNC(dee_memcasermem)
#define LOCAL_memcasecnt               LOCAL_FUNC(dee_memcasecnt)
#if LOCAL_SIZEOF_CHAR == 1
#define LOCAL_memcasecmp               LOCAL_FUNC(dee_memcasecmp)
#endif /* LOCAL_SIZEOF_CHAR == ... */
#define LOCAL_fuzzy_compare            LOCAL_FUNC(dee_fuzzy_compare)
#define LOCAL_fuzzy_casecompare        LOCAL_FUNC(dee_fuzzy_casecompare)
#define LOCAL_strverscmp               LOCAL_FUNC(dee_strverscmp)
#define LOCAL_strcaseverscmp           LOCAL_FUNC(dee_strcaseverscmp)
#define LOCAL_find_match               LOCAL_FUNC(dee_find_match)
#define LOCAL_find_casematch           LOCAL_FUNC(dee_find_casematch)
#define LOCAL_rfind_match              LOCAL_FUNC(dee_rfind_match)
#define LOCAL_rfind_casematch          LOCAL_FUNC(dee_rfind_casematch)
#define LOCAL_wildcompare              LOCAL_FUNC(dee_wildcompare)
#define LOCAL_wildcasecompare          LOCAL_FUNC(dee_wildcasecompare)
#endif /* !DEFINE_STRING_FUNCTIONS_CASEASCII */

#ifdef DEFINE_STRING_FUNCTIONS_CASEASCII
#define LOCAL_memcpy        memcpyb
#define LOCAL_mempcpy       mempcpyb
#define LOCAL_memset        memsetb
#define LOCAL_mempset       mempsetb
#define LOCAL_memmem        (byte_t *)memcasemem
#define LOCAL_memrmem       (byte_t *)memcasermem
#define LOCAL_memcmp        memcasecmp
#define LOCAL_TRANSFORM(ch) (byte_t)tolower(ch)
#elif LOCAL_SIZEOF_CHAR == 1
#define LOCAL_memcpy  memcpyb
#define LOCAL_mempcpy mempcpyb
#define LOCAL_memset  memsetb
#define LOCAL_mempset mempsetb
#define LOCAL_memmem  memmemb
#define LOCAL_memrmem memrmemb
#define LOCAL_memcmp  memcmpb
#define LOCAL_bcmp    bcmpb
#elif LOCAL_SIZEOF_CHAR == 2
#define LOCAL_memcpy  memcpyw
#define LOCAL_mempcpy mempcpyw
#define LOCAL_memset  memsetw
#define LOCAL_mempset mempsetw
#define LOCAL_memmem  memmemw
#define LOCAL_memrmem memrmemw
#define LOCAL_memcmp  memcmpw
#define LOCAL_bcmp    bcmpw
#elif LOCAL_SIZEOF_CHAR == 4
#define LOCAL_memcpy  memcpyl
#define LOCAL_mempcpy mempcpyl
#define LOCAL_memset  memsetl
#define LOCAL_mempset mempsetl
#define LOCAL_memmem  memmeml
#define LOCAL_memrmem memrmeml
#define LOCAL_memcmp  memcmpl
#define LOCAL_bcmp    bcmpl
#elif LOCAL_SIZEOF_CHAR == 8
#define LOCAL_memcpy  memcpyq
#define LOCAL_mempcpy mempcpyq
#define LOCAL_memset  memsetq
#define LOCAL_mempset mempsetq
#define LOCAL_memmem  memmemq
#define LOCAL_memrmem memrmemq
#define LOCAL_memcmp  memcmpq
#define LOCAL_bcmp    bcmpq
#else /* LOCAL_SIZEOF_CHAR == ... */
#error "Invalid configuration"
#endif /* LOCAL_SIZEOF_CHAR != ... */

#ifndef LOCAL_bcmp
#define LOCAL_bcmp LOCAL_memcmp
#endif /* !LOCAL_bcmp */

#ifndef LOCAL_TRANSFORM
#define LOCAL_TRANSFORM_IS_NOOP
#define LOCAL_TRANSFORM(ch) ch
#endif /* !LOCAL_TRANSFORM */





/************************************************************************/
/* IMPLEMENTATION                                                       */
/************************************************************************/
#ifdef LOCAL_mempfil
PRIVATE ATTR_OUTS(1, 2) ATTR_INS(3, 4) LOCAL_uchar_t *DCALL
LOCAL_mempfil(LOCAL_uchar_t *__restrict dst, size_t num_words,
              LOCAL_uchar_t const *__restrict src, size_t src_words) {
	ASSERT(src_words != 0);
	if (src_words == 1) {
		dst = LOCAL_mempset(dst, src[0], num_words);
	} else {
		while (num_words > src_words) {
			dst = LOCAL_mempcpy(dst, src, src_words);
			num_words -= src_words;
		}
		dst = LOCAL_mempcpy(dst, src, num_words);
	}
	return dst;
}
#endif /* LOCAL_mempfil */


#ifdef LOCAL_foldcmp
PRIVATE WUNUSED ATTR_INS(1, 2) ATTR_OUTS(3, 4) size_t DCALL
LOCAL_foldcmp(LOCAL_uchar_t const *__restrict data, size_t datalen,
              uint32_t fold[UNICODE_FOLDED_MAX], size_t fold_len) {
	uint32_t buf[UNICODE_FOLDED_MAX];
	size_t buflen;
	ASSERT(datalen >= 1);
	buflen = DeeUni_ToFolded(data[0], buf);
	switch (fold_len) {

	case 1:
		if (buf[0] == fold[0])
			goto ok_1;
		if (buflen >= 2 && buf[1] == fold[0])
			goto ok_1;
		if (buflen >= 3 && buf[2] == fold[0])
			goto ok_1;
		break;

	case 2:
		if (buflen == 1) {
			if (buf[0] != fold[0])
				break;
			if (datalen < 2)
				break;
			(void)DeeUni_ToFolded(data[1], buf);
			if (buf[0] == fold[1])
				goto ok_2;
		} else if (buflen == 2) {
			if (buf[0] == fold[0] && buf[1] == fold[1])
				goto ok_1;
			if (buf[1] == fold[0]) {
				if (datalen < 2)
					break;
				(void)DeeUni_ToFolded(data[1], buf);
				if (buf[0] == fold[1])
					goto ok_2;
			}
		} else {
			if (buf[0] == fold[0] && buf[1] == fold[1])
				goto ok_1;
			if (buf[1] == fold[0] && buf[2] == fold[1])
				goto ok_1;
			if (buf[2] == fold[0]) {
				if (datalen < 2)
					break;
				(void)DeeUni_ToFolded(data[1], buf);
				if (buf[0] == fold[1])
					goto ok_2;
			}
		}
		break;

	case 3:
		if (buflen == 1) {
			if (buf[0] != fold[0])
				break;
			if (datalen < 2)
				break;
			buflen = DeeUni_ToFolded(data[1], buf);
			if (buf[0] != fold[1])
				break;
			if (buflen == 1) {
				if (datalen < 3)
					break;
				(void)DeeUni_ToFolded(data[2], buf);
				if (buf[0] == fold[2])
					goto ok_3;
			} else {
				if (buf[1] == fold[2])
					goto ok_2;
			}
		} else if (buflen == 2) {
			if (buf[0] == fold[0] && buf[1] == fold[1]) {
				if (datalen < 2)
					break;
				(void)DeeUni_ToFolded(data[1], buf);
				if (buf[0] == fold[2])
					goto ok_2;
			}
			if (buf[1] == fold[0]) {
				if (datalen < 2)
					break;
				buflen = DeeUni_ToFolded(data[1], buf);
				if (buf[0] != fold[1])
					break;
				if (buflen == 1) {
					if (datalen < 3)
						break;
					(void)DeeUni_ToFolded(data[2], buf);
					if (buf[0] == fold[2])
						goto ok_3;
				} else {
					if (buf[1] == fold[2])
						goto ok_2;
				}
			}
		} else {
			if (buf[0] == fold[0] && buf[1] == fold[1] && buf[2] == fold[2])
				goto ok_1;
			if (buf[1] == fold[0] && buf[2] == fold[1]) {
				if (datalen < 2)
					break;
				(void)DeeUni_ToFolded(data[1], buf);
				if (buf[0] == fold[2])
					goto ok_2;
			}
			if (buf[2] == fold[0]) {
				if (datalen < 2)
					break;
				buflen = DeeUni_ToFolded(data[1], buf);
				if (buf[0] != fold[1])
					break;
				if (buflen == 1) {
					if (datalen < 3)
						break;
					buflen = DeeUni_ToFolded(data[2], buf);
					if (buf[0] == fold[2])
						goto ok_3;
				} else {
					if (buf[1] == fold[2])
						goto ok_2;
				}
			}
		}
		break;

	default: __builtin_unreachable();
	}
	return 0;
ok_1:
	return 1;
ok_2:
	return 2;
ok_3:
	return 3;
}
#endif /* LOCAL_foldcmp */


#ifdef LOCAL_memcasechr
PRIVATE ATTR_PURE WUNUSED ATTR_INS(1, 3) LOCAL_uchar_t *DCALL
LOCAL_memcasechr(LOCAL_uchar_t const *__restrict haystack,
                 LOCAL_uchar_t needle, size_t haystack_length) {
	uint32_t fold[UNICODE_FOLDED_MAX];
	size_t len = DeeUni_ToFolded(needle, fold);
	for (; haystack_length; ++haystack, --haystack_length) {
		if (LOCAL_foldcmp(haystack, haystack_length, fold, len))
			return (LOCAL_uchar_t *)haystack;
	}
	return NULL;
}
#endif /* LOCAL_memcasechr */


#ifdef LOCAL_memcaserchr
PRIVATE ATTR_PURE WUNUSED ATTR_INS(1, 3) LOCAL_uchar_t *DCALL
LOCAL_memcaserchr(LOCAL_uchar_t const *__restrict haystack,
                  LOCAL_uchar_t needle, size_t haystack_length) {
	LOCAL_uchar_t const *iter = haystack + haystack_length;
	uint32_t fold[UNICODE_FOLDED_MAX];
	size_t len     = DeeUni_ToFolded(needle, fold);
	size_t datalen = 0;
	while (iter > haystack) {
		--iter;
		++datalen;
		if (LOCAL_foldcmp(iter, datalen, fold, len))
			return (LOCAL_uchar_t *)iter;
	}
	return NULL;
}
#endif /* LOCAL_memcaserchr */




#ifdef LOCAL_unicode_foldreader_getc
PRIVATE WUNUSED ATTR_INOUT(1) uint32_t DCALL
LOCAL_unicode_foldreader_getc(struct unicode_foldreader *__restrict self) {
	if (self->ufr_idx < self->ufr_len)
		return self->ufr_buf[self->ufr_idx++];
	ASSERT(self->ufr_datalen);
	self->ufr_idx = 1;
	self->ufr_len = (uint8_t)DeeUni_ToFolded(*self->ufr_dataptr.LOCAL_cpX, self->ufr_buf);
	++self->ufr_dataptr.LOCAL_cpX;
	--self->ufr_datalen;
	return self->ufr_buf[0];
}
#endif /* LOCAL_unicode_foldreader_getc */

#ifdef LOCAL_unicode_foldreader_rgetc
PRIVATE WUNUSED ATTR_INOUT(1) uint32_t DCALL
LOCAL_unicode_foldreader_rgetc(struct unicode_foldreader *__restrict self) {
	if (self->ufr_idx < self->ufr_len)
		return self->ufr_buf[--self->ufr_len];
	ASSERT(self->ufr_datalen);
	self->ufr_idx = 0;
	--self->ufr_datalen;
	self->ufr_len = (uint8_t)DeeUni_ToFolded(self->ufr_dataptr.LOCAL_cpX[self->ufr_datalen],
	                                         self->ufr_buf) -
	                1;
	return self->ufr_buf[self->ufr_len];
}
#endif /* LOCAL_unicode_foldreader_rgetc */


#ifdef LOCAL_memcaseeq
PRIVATE ATTR_PURE WUNUSED ATTR_INS(1, 2) ATTR_INS(3, 4) bool DCALL
LOCAL_memcaseeq(LOCAL_uchar_t const *a, size_t a_size,
                LOCAL_uchar_t const *b, size_t b_size) {
	struct unicode_foldreader a_reader;
	struct unicode_foldreader b_reader;
	LOCAL_unicode_foldreader_init(&a_reader, a, a_size);
	LOCAL_unicode_foldreader_init(&b_reader, b, b_size);
	while (!unicode_foldreader_empty(&a_reader)) {
		if (unicode_foldreader_empty(&b_reader))
			return false;
		if (LOCAL_unicode_foldreader_getc(&a_reader) !=
		    LOCAL_unicode_foldreader_getc(&b_reader))
			return false;
	}
	return unicode_foldreader_empty(&b_reader);
}
#endif /* LOCAL_memcaseeq */


#ifdef LOCAL_memcasestartswith
PRIVATE ATTR_PURE WUNUSED ATTR_INS(1, 2) ATTR_INS(3, 4) size_t DCALL
LOCAL_memcasestartswith(LOCAL_uchar_t const *a, size_t a_size,
                        LOCAL_uchar_t const *b, size_t b_size) {
	struct unicode_foldreader a_reader;
	struct unicode_foldreader b_reader;
	LOCAL_unicode_foldreader_init(&a_reader, a, a_size);
	LOCAL_unicode_foldreader_init(&b_reader, b, b_size);
	while (!unicode_foldreader_empty(&b_reader)) {
		if (unicode_foldreader_empty(&a_reader) ||
		    (LOCAL_unicode_foldreader_getc(&a_reader) !=
		     LOCAL_unicode_foldreader_getc(&b_reader)))
			return 0;
	}
	return (size_t)(a_reader.ufr_dataptr.LOCAL_cpX - a);
}
#endif /* LOCAL_memcasestartswith */

#ifdef LOCAL_memcaseendswith
PRIVATE ATTR_PURE WUNUSED ATTR_INS(1, 2) ATTR_INS(3, 4) size_t DCALL
LOCAL_memcaseendswith(LOCAL_uchar_t const *a, size_t a_size,
                      LOCAL_uchar_t const *b, size_t b_size) {
	struct unicode_foldreader a_reader;
	struct unicode_foldreader b_reader;
	LOCAL_unicode_foldreader_init(&a_reader, a, a_size);
	LOCAL_unicode_foldreader_init(&b_reader, b, b_size);
	while (!unicode_foldreader_empty(&b_reader)) {
		if (unicode_foldreader_empty(&a_reader) ||
		    (LOCAL_unicode_foldreader_rgetc(&a_reader) !=
		     LOCAL_unicode_foldreader_rgetc(&b_reader)))
			return 0;
	}
	return a_size - a_reader.ufr_datalen;
}
#endif /* LOCAL_memcaseendswith */



#ifdef LOCAL_memcasemem
PRIVATE WUNUSED ATTR_INS(1, 2) ATTR_INS(3, 4) ATTR_OUT_OPT(5) LOCAL_uchar_t *DCALL
LOCAL_memcasemem(LOCAL_uchar_t const *haystack, size_t haystack_length,
                 LOCAL_uchar_t const *needle, size_t needle_length,
                 size_t *p_match_length) {
	if unlikely(!needle_length) {
		if (p_match_length)
			*p_match_length = 0;
		return (LOCAL_uchar_t *)haystack;
	}
	for (; haystack_length; --haystack_length, ++haystack) {
		size_t match_length = LOCAL_memcasestartswith(haystack, haystack_length, needle, needle_length);
		if (match_length) {
			if (p_match_length)
				*p_match_length = match_length;
			return (LOCAL_uchar_t *)haystack;
		}
	}
	return NULL;
}
#endif /* LOCAL_memcasemem */

#ifdef LOCAL_memcasermem
PRIVATE WUNUSED ATTR_INS(1, 2) ATTR_INS(3, 4) ATTR_OUT_OPT(5) LOCAL_uchar_t *DCALL
LOCAL_memcasermem(LOCAL_uchar_t const *haystack, size_t haystack_length,
                  LOCAL_uchar_t const *needle, size_t needle_length,
                  size_t *p_match_length) {
	if unlikely(!needle_length) {
		if (p_match_length)
			*p_match_length = 0;
		return (LOCAL_uchar_t *)haystack + haystack_length;
	}
	for (; haystack_length; --haystack_length) {
		size_t match_length = LOCAL_memcaseendswith(haystack, haystack_length, needle, needle_length);
		if (match_length) {
			if (p_match_length)
				*p_match_length = match_length;
			return (LOCAL_uchar_t *)haystack + haystack_length - match_length;
		}
	}
	return NULL;
}
#endif /* LOCAL_memcasermem */

#ifdef LOCAL_memcasecnt
PRIVATE ATTR_PURE WUNUSED ATTR_INS(1, 2) ATTR_INS(3, 4) size_t DCALL
LOCAL_memcasecnt(LOCAL_uchar_t const *haystack, size_t haystack_length,
                 LOCAL_uchar_t const *needle, size_t needle_length) {
	size_t result;
	if unlikely(!needle_length)
		return haystack_length + 1;
	for (result = 0;; ++result) {
		size_t match_length;
		LOCAL_uchar_t const *next;
		next = LOCAL_memcasemem(haystack, haystack_length,
		                        needle, needle_length,
		                        &match_length);
		if (!next)
			break;
		ASSERT(match_length > 0);
		next += match_length;
		haystack_length -= (size_t)(next - haystack);
		haystack = next;
	}
	return result;
}
#endif /* LOCAL_memcasecnt */




#ifdef LOCAL_memcasecmp
PRIVATE ATTR_PURE WUNUSED ATTR_INS(1, 2) ATTR_INS(3, 4) int DCALL
LOCAL_memcasecmp(LOCAL_uchar_t const *lhs, size_t lhs_size,
                 LOCAL_uchar_t const *rhs, size_t rhs_size) {
	struct unicode_foldreader lhs_reader;
	struct unicode_foldreader rhs_reader;
	LOCAL_unicode_foldreader_init(&lhs_reader, lhs, lhs_size);
	LOCAL_unicode_foldreader_init(&rhs_reader, rhs, rhs_size);
	for (;;) {
		uint32_t lhs_ch, rhs_ch;
		if (unicode_foldreader_empty(&lhs_reader)) {
			bool rhs_empty = unicode_foldreader_empty(&rhs_reader);
			return rhs_empty ? Dee_COMPARE_EQ : Dee_COMPARE_LO;
		}
		if (unicode_foldreader_empty(&rhs_reader))
			return Dee_COMPARE_GR;
		lhs_ch = LOCAL_unicode_foldreader_getc(&lhs_reader);
		rhs_ch = LOCAL_unicode_foldreader_getc(&rhs_reader);
		if (lhs_ch != rhs_ch)
			return Dee_CompareNe(lhs_ch, rhs_ch);
	}
}
#endif /* LOCAL_memcasecmp */



#ifdef LOCAL_fuzzy_compare
PRIVATE ATTR_PURE WUNUSED ATTR_INS(1, 2) ATTR_INS(3, 4) size_t DCALL
LOCAL_fuzzy_compare(LOCAL_uchar_t const *lhs, size_t lhs_len,
                    LOCAL_uchar_t const *rhs, size_t rhs_len) {
	size_t *v0, *v1, i, j, cost, temp;
	if unlikely(!lhs_len)
		return rhs_len;
	if unlikely(!rhs_len)
		return lhs_len;
	v0 = (size_t *)Dee_Mallocac(rhs_len + 1, sizeof(size_t));
	if unlikely(!v0)
		goto err;
	v1 = (size_t *)Dee_Mallocac(rhs_len + 1, sizeof(size_t));
	if unlikely(!v1)
		goto err_v0;
	for (i = 0; i < rhs_len; ++i)
		v0[i] = i;
	for (i = 0; i < lhs_len; ++i) {
		LOCAL_uchar_t lhs_value = LOCAL_TRANSFORM(lhs[i]);
		v1[0] = i + 1;
		for (j = 0; j < rhs_len; ++j) {
			cost = (lhs_value == LOCAL_TRANSFORM(rhs[j])) ? 0 : 1;
			cost += v0[j];
			temp = v1[j] + 1;
			if (temp < cost)
				cost = temp;
			temp = v0[j + 1] + 1;
			if (temp < cost)
				cost = temp;
			v1[j + 1] = cost;
		}
		memcpyc(v0, v1, rhs_len, sizeof(size_t));
	}
	temp = v1[rhs_len];
	Dee_Freea(v0);
	Dee_Freea(v1);
	if (temp > (size_t)-2)
		temp = (size_t)-2;
	return temp;
err_v0:
	Dee_Freea(v0);
err:
	return (size_t)-1;
}
#endif /* LOCAL_fuzzy_compare */


#ifdef LOCAL_fuzzy_casecompare
PRIVATE ATTR_PURE WUNUSED ATTR_INS(1, 2) ATTR_INS(3, 4) size_t DCALL
LOCAL_fuzzy_casecompare(LOCAL_uchar_t const *lhs, size_t lhs_len,
                        LOCAL_uchar_t const *rhs, size_t rhs_len) {
	size_t *v0, *v1, i, j, cost, temp;
	struct unicode_foldreader lhs_reader;
	struct unicode_foldreader rhs_reader;
	size_t folded_lhs_len, folded_rhs_len;
	folded_lhs_len = DeeUni_FoldedLength(lhs, lhs_len);
	folded_rhs_len = DeeUni_FoldedLength(rhs, rhs_len);
	if unlikely(!folded_lhs_len)
		return folded_rhs_len;
	if unlikely(!folded_rhs_len)
		return folded_lhs_len;
	v0 = (size_t *)Dee_Mallocac(folded_rhs_len + 1, sizeof(size_t));
	if unlikely(!v0)
		goto err;
	v1 = (size_t *)Dee_Mallocac(folded_rhs_len + 1, sizeof(size_t));
	if unlikely(!v1)
		goto err_v0;
	for (i = 0; i < folded_rhs_len; ++i)
		v0[i] = i;
	LOCAL_unicode_foldreader_init(&lhs_reader, lhs, lhs_len);
	for (i = 0; i < folded_lhs_len; ++i) {
		uint32_t a_value;
		a_value = LOCAL_unicode_foldreader_getc(&lhs_reader);
		v1[0] = i + 1;
		LOCAL_unicode_foldreader_init(&rhs_reader, rhs, rhs_len);
		for (j = 0; j < folded_rhs_len; ++j) {
			cost = (a_value == LOCAL_unicode_foldreader_getc(&rhs_reader)) ? 0u : 1u;
			cost += v0[j];
			temp = v1[j] + 1;
			if (temp < cost)
				cost = temp;
			temp = v0[j + 1] + 1;
			if (temp < cost)
				cost = temp;
			v1[j + 1] = cost;
		}
		ASSERT(unicode_foldreader_empty(&rhs_reader));
		memcpyc(v0, v1, folded_rhs_len, sizeof(size_t));
	}
	ASSERT(unicode_foldreader_empty(&lhs_reader));
	temp = v1[folded_rhs_len];
	Dee_Freea(v0);
	Dee_Freea(v1);
	if (temp > (size_t)-2)
		temp = (size_t)-2;
	return temp;
err_v0:
	Dee_Freea(v0);
err:
	return (size_t)-1;
}
#endif /* LOCAL_fuzzy_casecompare */



#ifdef LOCAL_strverscmp
/* As found here: https://en.wikipedia.org/wiki/Levenshtein_distance */
PRIVATE ATTR_PURE WUNUSED ATTR_INS(1, 2) ATTR_INS(3, 4) int DCALL
LOCAL_strverscmp(LOCAL_uchar_t const *lhs, size_t lhs_size,
                 LOCAL_uchar_t const *rhs, size_t rhs_size) {
	LOCAL_uchar_t clhs, crhs;
	LOCAL_uchar_t const *lhs_start = lhs;
	while (lhs_size && rhs_size) {
		if ((clhs = *lhs) != (crhs = *rhs)
#ifndef LOCAL_TRANSFORM_IS_NOOP
		    && ((clhs = LOCAL_TRANSFORM(clhs)) != (crhs = LOCAL_TRANSFORM(crhs)))
#endif /* !LOCAL_TRANSFORM_IS_NOOP */
		    ) {
			struct unitraits const *arec;
			struct unitraits const *brec;
			uintptr_t vala, valb; /* Unwind common digits. */
			while (lhs > lhs_start) {
				if (!DeeUni_IsDigit(lhs[-1]))
					break;
				crhs = clhs = *--lhs;
				--rhs;
				++lhs_size;
				++rhs_size;
			}

			/* Check if both strings have digit sequences in the same places. */
			arec = DeeUni_Descriptor(clhs);
			brec = DeeUni_Descriptor(crhs);
			if (!(arec->ut_flags & UNICODE_ISDIGIT) &&
			    !(brec->ut_flags & UNICODE_ISDIGIT))
				return Dee_Compare(clhs, crhs); /* Deal with leading zeros. */
			if ((arec->ut_flags & UNICODE_ISDIGIT) && arec->ut_digit_idx == 0)
				return Dee_COMPARE_LO;
			if ((brec->ut_flags & UNICODE_ISDIGIT) && brec->ut_digit_idx == 0)
				return Dee_COMPARE_GR;

			/* Compare digits. */
			vala = arec->ut_digit_idx;
			valb = brec->ut_digit_idx;
			while (--lhs_size) {
				clhs   = *++lhs;
				arec = DeeUni_Descriptor(clhs);
				if (!(arec->ut_flags & UNICODE_ISDIGIT))
					break;
				vala *= 10;
				vala += arec->ut_digit_idx;
			}
			while (--rhs_size) {
				crhs   = *++rhs;
				brec = DeeUni_Descriptor(crhs);
				if (!(brec->ut_flags & UNICODE_ISDIGIT))
					break;
				valb *= 10;
				valb += brec->ut_digit_idx;
			}
			return Dee_Compare(vala, valb);
		}
		++lhs;
		--lhs_size;
		++rhs;
		--rhs_size;
	}
	if (lhs_size)
		return Dee_COMPARE_GR;
	if (rhs_size)
		return Dee_COMPARE_LO;
	return Dee_COMPARE_EQ;
}
#endif /* LOCAL_strverscmp */



#ifdef LOCAL_strcaseverscmp
/* As found here: https://en.wikipedia.org/wiki/Levenshtein_distance */
PRIVATE ATTR_PURE WUNUSED ATTR_INS(1, 2) ATTR_INS(3, 4) int DCALL
LOCAL_strcaseverscmp(LOCAL_uchar_t const *lhs, size_t lhs_size,
                     LOCAL_uchar_t const *rhs, size_t rhs_size) {
	LOCAL_uchar_t clhs, crhs;
	LOCAL_uchar_t const *lhs_start = lhs;
	while (lhs_size && rhs_size) {
		if ((clhs = *lhs) != (crhs = *rhs) /* TODO: case-fold */
		    && ((clhs = (LOCAL_uchar_t)DeeUni_ToLower(clhs)) !=
		        (crhs = (LOCAL_uchar_t)DeeUni_ToLower(crhs)))) {
			struct unitraits const *arec;
			struct unitraits const *brec;
			uintptr_t vala, valb; /* Unwind common digits. */
			while (lhs > lhs_start) {
				if (!DeeUni_IsDigit(lhs[-1]))
					break;
				crhs = clhs = *--lhs;
				--rhs;
				++lhs_size;
				++rhs_size;
			}

			/* Check if both strings have digit sequences in the same places. */
			arec = DeeUni_Descriptor(clhs);
			brec = DeeUni_Descriptor(crhs);
			if (!(arec->ut_flags & UNICODE_ISDIGIT) &&
			    !(brec->ut_flags & UNICODE_ISDIGIT))
				return Dee_Compare(clhs, crhs); /* Deal with leading zeros. */
			if ((arec->ut_flags & UNICODE_ISDIGIT) && arec->ut_digit_idx == 0)
				return Dee_COMPARE_LO;
			if ((brec->ut_flags & UNICODE_ISDIGIT) && brec->ut_digit_idx == 0)
				return Dee_COMPARE_GR; /* Compare digits. */
			vala = arec->ut_digit_idx;
			valb = brec->ut_digit_idx;
			while (--lhs_size) {
				clhs   = *++lhs;
				arec = DeeUni_Descriptor(clhs);
				if (!(arec->ut_flags & UNICODE_ISDIGIT))
					break;
				vala *= 10;
				vala += arec->ut_digit_idx;
			}
			while (--rhs_size) {
				crhs   = *++rhs;
				brec = DeeUni_Descriptor(crhs);
				if (!(brec->ut_flags & UNICODE_ISDIGIT))
					break;
				valb *= 10;
				valb += brec->ut_digit_idx;
			}
			return Dee_Compare(vala, valb);
		}
		++lhs;
		--lhs_size;
		++rhs;
		--rhs_size;
	}
	if (lhs_size)
		return Dee_COMPARE_GR;
	if (rhs_size)
		return Dee_COMPARE_LO;
	return Dee_COMPARE_EQ;
}
#endif /* LOCAL_strcaseverscmp */



/* Returns a pointer into `scan_str', or NULL if no match was found. */
#ifdef LOCAL_find_match
PRIVATE ATTR_PURE WUNUSED ATTR_INS(1, 2) ATTR_INS(3, 4) ATTR_INS(5, 6) LOCAL_uchar_t const *DCALL
LOCAL_find_match(LOCAL_uchar_t const *scan_str, size_t scan_size,
                 LOCAL_uchar_t const *open_str, size_t open_size,
                 LOCAL_uchar_t const *clos_str, size_t clos_size) {
	size_t recursion = 0;
	if unlikely(!clos_size)
		return NULL;
	if unlikely(!open_size)
		return LOCAL_memmem(scan_str, scan_size, clos_str, clos_size);
	for (;;) {
		if (scan_size < clos_size)
			return NULL;
		if (LOCAL_bcmp(scan_str, clos_str, clos_size) == 0) {
			if (!recursion)
				break; /* Found it! */
			--recursion;
			scan_str += clos_size;
			scan_size -= clos_size;
			continue;
		}
		if (scan_size >= open_size &&
		    LOCAL_bcmp(scan_str, open_str, open_size) == 0) {
			++recursion;
			scan_str += open_size;
			scan_size -= open_size;
			continue;
		}
		++scan_str;
		--scan_size;
	}
	return scan_str;
}
#endif /* LOCAL_find_match */

#ifdef LOCAL_find_casematch
PRIVATE WUNUSED ATTR_INS(1, 2) ATTR_INS(3, 4) ATTR_INS(5, 6) ATTR_OUT_OPT(7) LOCAL_uchar_t const *DCALL
LOCAL_find_casematch(LOCAL_uchar_t const *scan_str, size_t scan_size,
                     LOCAL_uchar_t const *open_str, size_t open_size,
                     LOCAL_uchar_t const *clos_str, size_t clos_size,
                     size_t *p_match_length) {
	size_t recursion = 0;
	if unlikely(!clos_size)
		return NULL;
	if unlikely(!open_size)
		return LOCAL_memcasemem(scan_str, scan_size, clos_str, clos_size, p_match_length);
	while (scan_size) {
		size_t length;
		length = LOCAL_memcasestartswith(scan_str, scan_size, clos_str, clos_size);
		if (length) {
			if (!recursion) {
				if (p_match_length)
					*p_match_length = length;
				return scan_str;
			}
			--recursion;
			scan_str += length;
			scan_size -= length;
			continue;
		}
		length = LOCAL_memcasestartswith(scan_str, scan_size, open_str, open_size);
		if (length) {
			++recursion;
			scan_str += length;
			scan_size -= length;
			continue;
		}
		++scan_str;
		--scan_size;
	}
	return NULL;
}
#endif /* LOCAL_find_casematch */







#ifdef LOCAL_rfind_match
PRIVATE ATTR_PURE WUNUSED ATTR_INS(1, 2) ATTR_INS(3, 4) ATTR_INS(5, 6) LOCAL_uchar_t const *DCALL
LOCAL_rfind_match(LOCAL_uchar_t const *scan_str, size_t scan_size,
                  LOCAL_uchar_t const *open_str, size_t open_size,
                  LOCAL_uchar_t const *clos_str, size_t clos_size) {
	size_t recursion = 0;
	if unlikely(!open_size)
		return NULL;
	if unlikely(!clos_size)
		return LOCAL_memrmem(scan_str, scan_size, open_str, open_size);
	scan_str += scan_size;
	for (;;) {
		if (scan_size < open_size)
			return NULL;
		if (LOCAL_bcmp(scan_str - open_size, open_str, open_size) == 0) {
			scan_str -= open_size;
			if (!recursion)
				break; /* Found it! */
			--recursion;
			scan_size -= open_size;
			continue;
		}
		if (scan_size >= clos_size &&
		    LOCAL_bcmp(scan_str - clos_size, clos_str, clos_size) == 0) {
			++recursion;
			scan_str -= clos_size;
			scan_size -= clos_size;
			continue;
		}
		--scan_str;
		--scan_size;
	}
	return scan_str;
}
#endif /* LOCAL_rfind_match */

#ifdef LOCAL_rfind_casematch
PRIVATE WUNUSED ATTR_INS(1, 2) ATTR_INS(3, 4) ATTR_INS(5, 6) ATTR_OUT_OPT(7) LOCAL_uchar_t const *DCALL
LOCAL_rfind_casematch(LOCAL_uchar_t const *scan_str, size_t scan_size,
                      LOCAL_uchar_t const *open_str, size_t open_size,
                      LOCAL_uchar_t const *clos_str, size_t clos_size,
                      size_t *p_match_length) {
	size_t recursion = 0;
	if unlikely(!open_size)
		return NULL;
	if unlikely(!clos_size)
		return LOCAL_memcasermem(scan_str, scan_size, open_str, open_size, p_match_length);
	while (scan_size) {
		size_t length;
		length = LOCAL_memcaseendswith(scan_str, scan_size, open_str, open_size);
		if (length) {
			scan_size -= length;
			if (!recursion) {
				if (p_match_length)
					*p_match_length = length;
				return scan_str + scan_size;
			}
			--recursion;
			continue;
		}
		length = LOCAL_memcaseendswith(scan_str, scan_size, clos_str, clos_size);
		if (length) {
			++recursion;
			scan_size -= length;
			continue;
		}
		--scan_size;
	}
	return NULL;
}
#endif /* LOCAL_rfind_casematch */





#ifdef LOCAL_wildcompare
PRIVATE ATTR_PURE WUNUSED ATTR_INS(1, 2) ATTR_INS(3, 4) int DCALL
LOCAL_wildcompare(LOCAL_uchar_t const *string, size_t string_length,
                  LOCAL_uchar_t const *pattern, size_t pattern_length) {
	LOCAL_uchar_t card_post;
	LOCAL_uchar_t const *string_end  = string + string_length;
	LOCAL_uchar_t const *pattern_end = pattern + pattern_length;
	for (;;) {
		if (string >= string_end) {
			/* End of string (if the patter is empty, or only contains '*', we have a match) */
			for (;;) {
				if (pattern >= pattern_end)
					return Dee_COMPARE_EQ;
				if (*pattern != (LOCAL_uchar_t)'*')
					break;
				++pattern;
			}
			return Dee_COMPARE_LO;
		}
		if (pattern >= pattern_end)
			return Dee_COMPARE_GR; /* Pattern end doesn't match */
		if (*pattern == (LOCAL_uchar_t)'*') {
			/* Skip starts */
			do {
				++pattern;
				if (pattern >= pattern_end)
					return Dee_COMPARE_EQ; /* Pattern ends with '*' (matches everything) */
			} while (*pattern == (LOCAL_uchar_t)'*');
			card_post = *pattern++;
			if (card_post == (LOCAL_uchar_t)'?')
				goto next; /* Match any --> already found */
#ifndef LOCAL_TRANSFORM_IS_NOOP
			card_post = LOCAL_TRANSFORM(card_post);
#endif /* !LOCAL_TRANSFORM_IS_NOOP */
			for (;;) {
				LOCAL_uchar_t ch = *string++;
				if (ch == card_post
#ifndef LOCAL_TRANSFORM_IS_NOOP
				    || LOCAL_TRANSFORM(ch) == card_post
#endif /* !LOCAL_TRANSFORM_IS_NOOP */
				    ) {
					/* Recursively check if the rest of the string and pattern match */
					if (LOCAL_wildcompare(string, (size_t)(string_end - string),
					                      pattern, (size_t)(pattern_end - pattern)) == Dee_COMPARE_EQ)
						return Dee_COMPARE_EQ;
				} else if (string >= string_end) {
					return Dee_COMPARE_LO; /* Wildcard suffix not found */
				}
			}
		}
		if (*pattern == *string ||
		    *pattern == (LOCAL_uchar_t)'?'
#ifndef LOCAL_TRANSFORM_IS_NOOP
		    || LOCAL_TRANSFORM(*pattern) == LOCAL_TRANSFORM(*string)
#endif /* !LOCAL_TRANSFORM_IS_NOOP */
		) {
next:
			++string;
			++pattern;
			continue; /* single character match */
		}
		break; /* mismatch */
	}
	return Dee_CompareNe((LOCAL_uchar_t)*string,
	                     (LOCAL_uchar_t)*pattern);
}
#endif /* LOCAL_wildcompare */


#ifdef LOCAL_wildcasecompare
PRIVATE WUNUSED ATTR_INOUT(1) ATTR_INOUT(2) int DCALL
LOCAL_wildcasecompare(struct unicode_foldreader *__restrict string,
                      struct unicode_foldreader *__restrict pattern) {
	uint32_t pattern_ch, ch;
	for (;;) {
		if (unicode_foldreader_empty(string)) {
			/* End of string (if the patter is empty, or only contains '*', we have a match) */
			for (;;) {
				if (unicode_foldreader_empty(pattern))
					return Dee_COMPARE_EQ;
				ch = LOCAL_unicode_foldreader_getc(pattern);
				if (ch != (uint32_t)'*')
					break;
			}
			return Dee_COMPARE_LO;
		}
		if (unicode_foldreader_empty(pattern))
			return Dee_COMPARE_GR; /* Pattern end doesn't match */
		pattern_ch = LOCAL_unicode_foldreader_getc(pattern);
		if (pattern_ch == (uint32_t)'*') {
			/* Skip starts */
			do {
				if (unicode_foldreader_empty(pattern))
					return Dee_COMPARE_EQ; /* Pattern ends with '*' (matches everything) */
				pattern_ch = LOCAL_unicode_foldreader_getc(pattern);
			} while (pattern_ch == (uint32_t)'*');
			if (pattern_ch == (uint32_t)'?')
				continue; /* Match any --> already found */
			for (;;) {
				ch = LOCAL_unicode_foldreader_getc(string);
				if (ch == pattern_ch) {
					/* Recursively check if the rest of the string and pattern match */
					struct unicode_foldreader string_copy  = *string;
					struct unicode_foldreader pattern_copy = *pattern;
					if (LOCAL_wildcasecompare(&string_copy, &pattern_copy) == Dee_COMPARE_EQ)
						return Dee_COMPARE_EQ;
				} else if (unicode_foldreader_empty(string)) {
					return Dee_COMPARE_LO; /* Wildcard suffix not found */
				}
			}
		}
		ch = LOCAL_unicode_foldreader_getc(string);
		if (pattern_ch == ch ||
		    pattern_ch == (uint32_t)'?')
			continue; /* single character match */

		/* mismatch */
		break;
	}
	return Dee_CompareNe(ch, pattern_ch);
}
#endif /* LOCAL_wildcasecompare */






#undef LOCAL_foldcmp
#undef LOCAL_memcasechr
#undef LOCAL_memcaserchr
#undef LOCAL_unicode_foldreader_init
#undef LOCAL_unicode_foldreader_getc
#undef LOCAL_unicode_foldreader_rgetc
#undef LOCAL_memcaseeq
#undef LOCAL_memcasestartswith
#undef LOCAL_memcaseendswith
#undef LOCAL_memcasemem
#undef LOCAL_memcasermem
#undef LOCAL_memcasecnt
#undef LOCAL_memcasecmp
#undef LOCAL_fuzzy_compare
#undef LOCAL_fuzzy_casecompare
#undef LOCAL_strverscmp
#undef LOCAL_strcaseverscmp
#undef LOCAL_find_match
#undef LOCAL_find_casematch
#undef LOCAL_rfind_match
#undef LOCAL_rfind_casematch
#undef LOCAL_wildcompare
#undef LOCAL_wildcasecompare
#undef LOCAL_mempfil

#undef LOCAL_memcpy
#undef LOCAL_mempcpy
#undef LOCAL_memset
#undef LOCAL_mempset
#undef LOCAL_memmem
#undef LOCAL_memrmem
#undef LOCAL_memcmp
#undef LOCAL_bcmp
#undef LOCAL_TRANSFORM
#undef LOCAL_TRANSFORM_IS_NOOP

#undef LOCAL_schar_t
#undef LOCAL_uchar_t
#undef LOCAL_SIZEOF_CHAR
#undef LOCAL_FUNC
#undef LOCAL_cpX

DECL_END

#undef DEFINE_STRING_FUNCTIONS_CASEASCII
#undef DEFINE_STRING_FUNCTIONS_B
#undef DEFINE_STRING_FUNCTIONS_W
#undef DEFINE_STRING_FUNCTIONS_L
