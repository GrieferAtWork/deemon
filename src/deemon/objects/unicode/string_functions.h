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
#ifndef GUARD_DEEMON_OBJECTS_UNICODE_STRING_FUNCTIONS_H
#define GUARD_DEEMON_OBJECTS_UNICODE_STRING_FUNCTIONS_H 1

#include <deemon/api.h>
#include <deemon/object.h>
#include <deemon/string.h>
#include <deemon/system-features.h> /* memrchr(), memmem(), ... */

#include <hybrid/overflow.h> /* OVERFLOW_USUB */
#include <hybrid/typecore.h> /* __BYTE_TYPE__ */
/**/

#include <stddef.h> /* size_t */
#include <stdint.h> /* uint8_t, uint16_t, uint32_t, int64_t */

#undef byte_t
#define byte_t __BYTE_TYPE__

DECL_BEGIN

#define DeeUni_IsSign(x) ((x) == '+' || (x) == '-')
#define ASCII_SPACE    32 /* ' ' */
#define ASCII_ZERO     48 /* '0' */
#define ASCII_CR       13 /* '\r' */
#define ASCII_LF       10 /* '\n' */
#define ASCII_TAB       9 /* '\t' */

#define UNICODE_SPACE  32 /* ' ' */
#define UNICODE_ZERO   48 /* '0' */
#define UNICODE_CR     13 /* '\r' */
#define UNICODE_LF     10 /* '\n' */

#define MEMEQB(a, b, s) (bcmpb(a, b, s) == 0)
#define MEMEQW(a, b, s) (bcmpw(a, b, s) == 0)
#define MEMEQL(a, b, s) (bcmpl(a, b, s) == 0)

/* Basic aliases */
#undef memrchrb
#define memrchrb(p, c, n) ((uint8_t *)memrchr(p, c, n))
#undef memmemb
#define memmemb(haystack, haystack_length, needle, needle_length) \
	((uint8_t *)memmem(haystack, haystack_length, needle, needle_length))
#undef memrmemb
#define memrmemb(haystack, haystack_length, needle, needle_length) \
	((uint8_t *)memrmem(haystack, haystack_length, needle, needle_length))

/* Supplement non-standard <string.h> functions */
#ifndef CONFIG_HAVE_memrchr
#define CONFIG_HAVE_memrchr
#undef memrchr
#define memrchr dee_memrchr
DeeSystem_DEFINE_memrchr(dee_memrchr)
#endif /* !CONFIG_HAVE_memrchr */

#ifndef CONFIG_HAVE_memmem
#define CONFIG_HAVE_memmem
#undef memmem
#define memmem  dee_memmem
DeeSystem_DEFINE_memmem(dee_memmem)
#endif /* !CONFIG_HAVE_memmem */

#ifndef CONFIG_HAVE_memrmem
#define CONFIG_HAVE_memrmem
#undef memrmem
#define memrmem dee_memrmem
DeeSystem_DEFINE_memrmem(dee_memrmem)
#endif /* !CONFIG_HAVE_memrmem */

#ifndef CONFIG_HAVE_memsetw
#define CONFIG_HAVE_memsetw
#undef memsetw
#define memsetw dee_memsetw
DeeSystem_DEFINE_memsetw(dee_memsetw)
#endif /* !CONFIG_HAVE_memsetw */

#ifndef CONFIG_HAVE_memsetl
#define CONFIG_HAVE_memsetl
#undef memsetl
#define memsetl dee_memsetl
DeeSystem_DEFINE_memsetl(dee_memsetl)
#endif /* !CONFIG_HAVE_memsetl */

#ifndef CONFIG_HAVE_mempsetw
#define CONFIG_HAVE_mempsetw
#undef mempsetw
#define mempsetw dee_mempsetw
DeeSystem_DEFINE_mempsetw(dee_mempsetw)
#endif /* !CONFIG_HAVE_mempsetw */

#ifndef CONFIG_HAVE_mempsetl
#define CONFIG_HAVE_mempsetl
#undef mempsetl
#define mempsetl dee_mempsetl
DeeSystem_DEFINE_mempsetl(dee_mempsetl)
#endif /* !CONFIG_HAVE_mempsetl */

#ifndef CONFIG_HAVE_memcmpw
#define CONFIG_HAVE_memcmpw
#undef memcmpw
#define memcmpw dee_memcmpw
DeeSystem_DEFINE_memcmpw(dee_memcmpw)
#endif /* !CONFIG_HAVE_memcmpw */

#ifndef CONFIG_HAVE_memcmpl
#define CONFIG_HAVE_memcmpl
#undef memcmpl
#define memcmpl dee_memcmpl
DeeSystem_DEFINE_memcmpl(dee_memcmpl)
#endif /* !CONFIG_HAVE_memcmpl */

#ifndef CONFIG_HAVE_memchrw
#define CONFIG_HAVE_memchrw
#undef memchrw
#define memchrw dee_memchrw
DeeSystem_DEFINE_memchrw(dee_memchrw)
#endif /* !CONFIG_HAVE_memchrw */

#ifndef CONFIG_HAVE_memchrl
#define CONFIG_HAVE_memchrl
#undef memchrl
#define memchrl dee_memchrl
DeeSystem_DEFINE_memchrl(dee_memchrl)
#endif /* !CONFIG_HAVE_memchrl */

#ifndef CONFIG_HAVE_memrchrw
#define CONFIG_HAVE_memrchrw
#undef memrchrw
#define memrchrw dee_memrchrw
DeeSystem_DEFINE_memrchrw(dee_memrchrw)
#endif /* !CONFIG_HAVE_memrchrw */

#ifndef CONFIG_HAVE_memrchrl
#define CONFIG_HAVE_memrchrl
#undef memrchrl
#define memrchrl dee_memrchrl
DeeSystem_DEFINE_memrchrl(dee_memrchrl)
#endif /* !CONFIG_HAVE_memrchrl */

#ifndef CONFIG_HAVE_memmemw
#define CONFIG_HAVE_memmemw
#undef memmemw
#define memmemw dee_memmemw
DeeSystem_DEFINE_memmemw(dee_memmemw, memchrw, MEMEQW)
#endif /* !CONFIG_HAVE_memmemw */

#ifndef CONFIG_HAVE_memmeml
#define CONFIG_HAVE_memmeml
#undef memmeml
#define memmeml dee_memmeml
DeeSystem_DEFINE_memmeml(dee_memmeml, memchrl, MEMEQL)
#endif /* !CONFIG_HAVE_memmeml */

#ifndef CONFIG_HAVE_memrmemw
#define CONFIG_HAVE_memrmemw
#undef memrmemw
#define memrmemw dee_memrmemw
DeeSystem_DEFINE_memrmemw(dee_memrmemw, memrchrw, MEMEQW)
#endif /* !CONFIG_HAVE_memrmemw */

#ifndef CONFIG_HAVE_memrmeml
#define CONFIG_HAVE_memrmeml
#undef memrmeml
#define memrmeml dee_memrmeml
DeeSystem_DEFINE_memrmeml(dee_memrmeml, memrchrl, MEMEQL)
#endif /* !CONFIG_HAVE_memrmeml */

#undef memcnt
#define memcnt dee_memcnt
#undef memcntb
#define memcntb dee_memcnt
DeeSystem_DEFINE_memcnt(dee_memcnt)

#undef memcntw
#define memcntw dee_memcntw
DeeSystem_DEFINE_memcntw(dee_memcntw)

#undef memcntl
#define memcntl dee_memcntl
DeeSystem_DEFINE_memcntl(dee_memcntl)





/* ASCII case-insensitive functions (for `Bytes'). */
#ifndef CONFIG_HAVE_memcasecmp
#define CONFIG_HAVE_memcasecmp
#undef memcasecmp
#define memcasecmp dee_memcasecmp
DeeSystem_DEFINE_memcasecmp(dee_memcasecmp)
#endif /* !CONFIG_HAVE_memcasecmp */
#undef memasciicaseeq
#define memasciicaseeq(a, b, num_bytes) (memcasecmp(a, b, num_bytes) == 0)

#undef memasciicasechr
#define memasciicasechr(haystack, needle, haystack_length) \
	_dee_memasciicasechr(haystack, (uint8_t)tolower(needle), haystack_length)
DeeSystem_DEFINE__memlowerchr(_dee_memasciicasechr)

#undef memasciicaserchr
#define memasciicaserchr(haystack, needle, haystack_length) \
	_dee_memasciicaserchr(haystack, (uint8_t)tolower(needle), haystack_length)
DeeSystem_DEFINE__memlowerrchr(_dee_memasciicaserchr)

#ifndef CONFIG_HAVE_memcasemem
#define CONFIG_HAVE_memcasemem
#undef memcasemem
#define memcasemem dee_memcasemem
_DeeSystem_DEFINE_memcasemem(dee_memcasemem, _dee_memasciicasechr)
#endif /* !CONFIG_HAVE_memcasemem */
#undef memasciicasemem
#define memasciicasemem(haystack, haystack_length, needle, needle_length) \
	((uint8_t *)memcasemem(haystack, haystack_length, needle, needle_length))

#ifndef CONFIG_HAVE_memcasermem
#define CONFIG_HAVE_memcasermem
#undef memcasermem
#define memcasermem dee_memcasermem
_DeeSystem_DEFINE_memcasermem(dee_memcasermem, _dee_memasciicaserchr)
#endif /* !CONFIG_HAVE_memcasermem */
#undef memasciicasermem
#define memasciicasermem(haystack, haystack_length, needle, needle_length) \
	((uint8_t *)memcasermem(haystack, haystack_length, needle, needle_length))

#undef memcasecnt
#define memcasecnt dee_memcasecnt
DeeSystem_DEFINE_memcasecnt(dee_memcasecnt)






struct unicode_foldreader {
	union dcharptr_const ufr_dataptr; /* [0..ufr_datalen] Input data pointer */
	size_t               ufr_datalen; /* # of remaining words in `ufr_dataptr' (relevant word-type depends on API usage) */
	uint32_t             ufr_buf[UNICODE_FOLDED_MAX]; /* Buffer for unread casefold characters */
	uint8_t              ufr_len;     /* [<= UNICODE_FOLDED_MAX] # of characters stored in `ufr_buf' */
	uint8_t              ufr_idx;     /* [<= ufr_len] Index of next unread casefold character in `ufr_buf' */
};

#define _unicode_foldreader_init(self, cpX, data, len) \
	((self)->ufr_dataptr.cpX = (data),                 \
	 (self)->ufr_datalen     = (len),                  \
	 (self)->ufr_len = (self)->ufr_idx = 0)
#define unicode_foldreader_initb(self, data, len) _unicode_foldreader_init(self, cp8, data, len)
#define unicode_foldreader_initw(self, data, len) _unicode_foldreader_init(self, cp16, data, len)
#define unicode_foldreader_initl(self, data, len) _unicode_foldreader_init(self, cp32, data, len)
#define unicode_foldreader_empty(self) (!(self)->ufr_datalen && ((self)->ufr_idx >= (self)->ufr_len))

/* Case-insensitive string functions. */
#ifdef __INTELLISENSE__
PRIVATE ATTR_OUTS(1, 2) ATTR_INS(3, 4) uint8_t *DCALL dee_mempfilb(uint8_t *__restrict dst, size_t num_bytes, uint8_t const *__restrict src, size_t src_bytes);
PRIVATE ATTR_OUTS(1, 2) ATTR_INS(3, 4) uint16_t *DCALL dee_mempfilw(uint16_t *__restrict dst, size_t num_bytes, uint16_t const *__restrict src, size_t src_bytes);
PRIVATE ATTR_OUTS(1, 2) ATTR_INS(3, 4) uint32_t *DCALL dee_mempfill(uint32_t *__restrict dst, size_t num_bytes, uint32_t const *__restrict src, size_t src_bytes);
PRIVATE WUNUSED ATTR_INS(1, 2) ATTR_OUTS(3, 4) size_t DCALL dee_foldcmpb(uint8_t const *__restrict data, size_t datalen, uint32_t fold[UNICODE_FOLDED_MAX], size_t fold_len);
PRIVATE WUNUSED ATTR_INS(1, 2) ATTR_OUTS(3, 4) size_t DCALL dee_foldcmpw(uint16_t const *__restrict data, size_t datalen, uint32_t fold[UNICODE_FOLDED_MAX], size_t fold_len);
PRIVATE WUNUSED ATTR_INS(1, 2) ATTR_OUTS(3, 4) size_t DCALL dee_foldcmpl(uint32_t const *__restrict data, size_t datalen, uint32_t fold[UNICODE_FOLDED_MAX], size_t fold_len);
PRIVATE ATTR_PURE WUNUSED ATTR_INS(1, 3) uint8_t *DCALL dee_memcasechrb(uint8_t const *__restrict haystack, uint8_t needle, size_t haystack_length);
PRIVATE ATTR_PURE WUNUSED ATTR_INS(1, 3) uint16_t *DCALL dee_memcasechrw(uint16_t const *__restrict haystack, uint16_t needle, size_t haystack_length);
PRIVATE ATTR_PURE WUNUSED ATTR_INS(1, 3) uint32_t *DCALL dee_memcasechrl(uint32_t const *__restrict haystack, uint32_t needle, size_t haystack_length);
/*PRIVATE ATTR_PURE WUNUSED ATTR_INS(1, 3) uint8_t *DCALL dee_memcaserchrb(uint8_t const *__restrict haystack, uint8_t needle, size_t haystack_length);*/
/*PRIVATE ATTR_PURE WUNUSED ATTR_INS(1, 3) uint16_t *DCALL dee_memcaserchrw(uint16_t const *__restrict haystack, uint16_t needle, size_t haystack_length);*/
/*PRIVATE ATTR_PURE WUNUSED ATTR_INS(1, 3) uint32_t *DCALL dee_memcaserchrl(uint32_t const *__restrict haystack, uint32_t needle, size_t haystack_length);*/
PRIVATE WUNUSED ATTR_INOUT(1) uint32_t DCALL unicode_foldreader_getcb(struct unicode_foldreader *__restrict self);
PRIVATE WUNUSED ATTR_INOUT(1) uint32_t DCALL unicode_foldreader_getcw(struct unicode_foldreader *__restrict self);
PRIVATE WUNUSED ATTR_INOUT(1) uint32_t DCALL unicode_foldreader_getcl(struct unicode_foldreader *__restrict self);
PRIVATE WUNUSED ATTR_INOUT(1) uint32_t DCALL unicode_foldreader_rgetcb(struct unicode_foldreader *__restrict self);
PRIVATE WUNUSED ATTR_INOUT(1) uint32_t DCALL unicode_foldreader_rgetcw(struct unicode_foldreader *__restrict self);
PRIVATE WUNUSED ATTR_INOUT(1) uint32_t DCALL unicode_foldreader_rgetcl(struct unicode_foldreader *__restrict self);
/*PRIVATE ATTR_PURE WUNUSED ATTR_INS(1, 2) ATTR_INS(3, 4) bool DCALL dee_memcaseeqb(uint8_t const *a, size_t a_size, uint8_t const *b, size_t b_size);*/
/*PRIVATE ATTR_PURE WUNUSED ATTR_INS(1, 2) ATTR_INS(3, 4) bool DCALL dee_memcaseeqw(uint16_t const *a, size_t a_size, uint16_t const *b, size_t b_size);*/
/*PRIVATE ATTR_PURE WUNUSED ATTR_INS(1, 2) ATTR_INS(3, 4) bool DCALL dee_memcaseeql(uint32_t const *a, size_t a_size, uint32_t const *b, size_t b_size);*/
PRIVATE ATTR_PURE WUNUSED ATTR_INS(1, 2) ATTR_INS(3, 4) size_t DCALL dee_memcasestartswithb(uint8_t const *a, size_t a_size, uint8_t const *b, size_t b_size);
PRIVATE ATTR_PURE WUNUSED ATTR_INS(1, 2) ATTR_INS(3, 4) size_t DCALL dee_memcasestartswithw(uint16_t const *a, size_t a_size, uint16_t const *b, size_t b_size);
PRIVATE ATTR_PURE WUNUSED ATTR_INS(1, 2) ATTR_INS(3, 4) size_t DCALL dee_memcasestartswithl(uint32_t const *a, size_t a_size, uint32_t const *b, size_t b_size);
PRIVATE ATTR_PURE WUNUSED ATTR_INS(1, 2) ATTR_INS(3, 4) size_t DCALL dee_memcaseendswithb(uint8_t const *a, size_t a_size, uint8_t const *b, size_t b_size);
PRIVATE ATTR_PURE WUNUSED ATTR_INS(1, 2) ATTR_INS(3, 4) size_t DCALL dee_memcaseendswithw(uint16_t const *a, size_t a_size, uint16_t const *b, size_t b_size);
PRIVATE ATTR_PURE WUNUSED ATTR_INS(1, 2) ATTR_INS(3, 4) size_t DCALL dee_memcaseendswithl(uint32_t const *a, size_t a_size, uint32_t const *b, size_t b_size);
PRIVATE WUNUSED ATTR_INS(1, 2) ATTR_INS(3, 4) ATTR_OUT_OPT(5) uint8_t *DCALL dee_memcasememb(uint8_t const *haystack, size_t haystack_length, uint8_t const *needle, size_t needle_length, size_t *p_match_length);
PRIVATE WUNUSED ATTR_INS(1, 2) ATTR_INS(3, 4) ATTR_OUT_OPT(5) uint16_t *DCALL dee_memcasememw(uint16_t const *haystack, size_t haystack_length, uint16_t const *needle, size_t needle_length, size_t *p_match_length);
PRIVATE WUNUSED ATTR_INS(1, 2) ATTR_INS(3, 4) ATTR_OUT_OPT(5) uint32_t *DCALL dee_memcasememl(uint32_t const *haystack, size_t haystack_length, uint32_t const *needle, size_t needle_length, size_t *p_match_length);
PRIVATE WUNUSED ATTR_INS(1, 2) ATTR_INS(3, 4) ATTR_OUT_OPT(5) uint8_t *DCALL dee_memcasermemb(uint8_t const *haystack, size_t haystack_length, uint8_t const *needle, size_t needle_length, size_t *p_match_length);
PRIVATE WUNUSED ATTR_INS(1, 2) ATTR_INS(3, 4) ATTR_OUT_OPT(5) uint16_t *DCALL dee_memcasermemw(uint16_t const *haystack, size_t haystack_length, uint16_t const *needle, size_t needle_length, size_t *p_match_length);
PRIVATE WUNUSED ATTR_INS(1, 2) ATTR_INS(3, 4) ATTR_OUT_OPT(5) uint32_t *DCALL dee_memcasermeml(uint32_t const *haystack, size_t haystack_length, uint32_t const *needle, size_t needle_length, size_t *p_match_length);
PRIVATE ATTR_PURE WUNUSED ATTR_INS(1, 2) ATTR_INS(3, 4) int DCALL dee_memcasecmpb(uint8_t const *lhs, size_t lhs_size, uint8_t const *rhs, size_t rhs_size);
/*PRIVATE ATTR_PURE WUNUSED ATTR_INS(1, 2) ATTR_INS(3, 4) int DCALL dee_memcasecmpw(uint16_t const *lhs, size_t lhs_size, uint16_t const *rhs, size_t rhs_size);*/
/*PRIVATE ATTR_PURE WUNUSED ATTR_INS(1, 2) ATTR_INS(3, 4) int DCALL dee_memcasecmpl(uint32_t const *lhs, size_t lhs_size, uint32_t const *rhs, size_t rhs_size);*/
PRIVATE ATTR_PURE WUNUSED ATTR_INS(1, 2) ATTR_INS(3, 4) size_t DCALL dee_memcasecntb(uint8_t const *haystack, size_t haystack_length, uint8_t const *needle, size_t needle_length);
PRIVATE ATTR_PURE WUNUSED ATTR_INS(1, 2) ATTR_INS(3, 4) size_t DCALL dee_memcasecntw(uint16_t const *haystack, size_t haystack_length, uint16_t const *needle, size_t needle_length);
PRIVATE ATTR_PURE WUNUSED ATTR_INS(1, 2) ATTR_INS(3, 4) size_t DCALL dee_memcasecntl(uint32_t const *haystack, size_t haystack_length, uint32_t const *needle, size_t needle_length);
PRIVATE ATTR_PURE WUNUSED ATTR_INS(1, 2) ATTR_INS(3, 4) size_t DCALL dee_fuzzy_compareb(uint8_t const *lhs, size_t lhs_len, uint8_t const *rhs, size_t rhs_len);
PRIVATE ATTR_PURE WUNUSED ATTR_INS(1, 2) ATTR_INS(3, 4) size_t DCALL dee_fuzzy_comparew(uint16_t const *lhs, size_t lhs_len, uint16_t const *rhs, size_t rhs_len);
PRIVATE ATTR_PURE WUNUSED ATTR_INS(1, 2) ATTR_INS(3, 4) size_t DCALL dee_fuzzy_comparel(uint32_t const *lhs, size_t lhs_len, uint32_t const *rhs, size_t rhs_len);
PRIVATE ATTR_PURE WUNUSED ATTR_INS(1, 2) ATTR_INS(3, 4) size_t DCALL dee_fuzzy_casecompare_ascii(byte_t const *lhs, size_t lhs_len, byte_t const *rhs, size_t rhs_len);
PRIVATE ATTR_PURE WUNUSED ATTR_INS(1, 2) ATTR_INS(3, 4) size_t DCALL dee_fuzzy_casecompareb(uint8_t const *lhs, size_t lhs_len, uint8_t const *rhs, size_t rhs_len);
PRIVATE ATTR_PURE WUNUSED ATTR_INS(1, 2) ATTR_INS(3, 4) size_t DCALL dee_fuzzy_casecomparew(uint16_t const *lhs, size_t lhs_len, uint16_t const *rhs, size_t rhs_len);
PRIVATE ATTR_PURE WUNUSED ATTR_INS(1, 2) ATTR_INS(3, 4) size_t DCALL dee_fuzzy_casecomparel(uint32_t const *lhs, size_t lhs_len, uint32_t const *rhs, size_t rhs_len);
PRIVATE ATTR_PURE WUNUSED ATTR_INS(1, 2) ATTR_INS(3, 4) int DCALL dee_strverscmpb(uint8_t const *lhs, size_t lhs_size, uint8_t const *rhs, size_t rhs_size);
PRIVATE ATTR_PURE WUNUSED ATTR_INS(1, 2) ATTR_INS(3, 4) int DCALL dee_strverscmpw(uint16_t const *lhs, size_t lhs_size, uint16_t const *rhs, size_t rhs_size);
PRIVATE ATTR_PURE WUNUSED ATTR_INS(1, 2) ATTR_INS(3, 4) int DCALL dee_strverscmpl(uint32_t const *lhs, size_t lhs_size, uint32_t const *rhs, size_t rhs_size);
PRIVATE ATTR_PURE WUNUSED ATTR_INS(1, 2) ATTR_INS(3, 4) int DCALL dee_strcaseverscmp_ascii(uint8_t const *lhs, size_t lhs_size, uint8_t const *rhs, size_t rhs_size);
PRIVATE ATTR_PURE WUNUSED ATTR_INS(1, 2) ATTR_INS(3, 4) int DCALL dee_strcaseverscmpb(uint8_t const *lhs, size_t lhs_size, uint8_t const *rhs, size_t rhs_size);
PRIVATE ATTR_PURE WUNUSED ATTR_INS(1, 2) ATTR_INS(3, 4) int DCALL dee_strcaseverscmpw(uint16_t const *lhs, size_t lhs_size, uint16_t const *rhs, size_t rhs_size);
PRIVATE ATTR_PURE WUNUSED ATTR_INS(1, 2) ATTR_INS(3, 4) int DCALL dee_strcaseverscmpl(uint32_t const *lhs, size_t lhs_size, uint32_t const *rhs, size_t rhs_size);
PRIVATE ATTR_PURE WUNUSED ATTR_INS(1, 2) ATTR_INS(3, 4) ATTR_INS(5, 6) uint8_t const *DCALL dee_find_matchb(uint8_t const *scan_str, size_t scan_size, uint8_t const *open_str, size_t open_size, uint8_t const *clos_str, size_t clos_size);
PRIVATE ATTR_PURE WUNUSED ATTR_INS(1, 2) ATTR_INS(3, 4) ATTR_INS(5, 6) uint16_t const *DCALL dee_find_matchw(uint16_t const *scan_str, size_t scan_size, uint16_t const *open_str, size_t open_size, uint16_t const *clos_str, size_t clos_size);
PRIVATE ATTR_PURE WUNUSED ATTR_INS(1, 2) ATTR_INS(3, 4) ATTR_INS(5, 6) uint32_t const *DCALL dee_find_matchl(uint32_t const *scan_str, size_t scan_size, uint32_t const *open_str, size_t open_size, uint32_t const *clos_str, size_t clos_size);
PRIVATE ATTR_PURE WUNUSED ATTR_INS(1, 2) ATTR_INS(3, 4) ATTR_INS(5, 6) byte_t const *DCALL dee_find_casematch_ascii(byte_t const *scan_str, size_t scan_size, byte_t const *open_str, size_t open_size, byte_t const *clos_str, size_t clos_size);
PRIVATE ATTR_PURE WUNUSED ATTR_INS(1, 2) ATTR_INS(3, 4) ATTR_INS(5, 6) uint8_t const *DCALL dee_find_casematchb(uint8_t const *scan_str, size_t scan_size, uint8_t const *open_str, size_t open_size, uint8_t const *clos_str, size_t clos_size, size_t *p_match_length);
PRIVATE ATTR_PURE WUNUSED ATTR_INS(1, 2) ATTR_INS(3, 4) ATTR_INS(5, 6) uint16_t const *DCALL dee_find_casematchw(uint16_t const *scan_str, size_t scan_size, uint16_t const *open_str, size_t open_size, uint16_t const *clos_str, size_t clos_size, size_t *p_match_length);
PRIVATE ATTR_PURE WUNUSED ATTR_INS(1, 2) ATTR_INS(3, 4) ATTR_INS(5, 6) uint32_t const *DCALL dee_find_casematchl(uint32_t const *scan_str, size_t scan_size, uint32_t const *open_str, size_t open_size, uint32_t const *clos_str, size_t clos_size, size_t *p_match_length);
PRIVATE ATTR_PURE WUNUSED ATTR_INS(1, 2) ATTR_INS(3, 4) ATTR_INS(5, 6) uint8_t const *DCALL dee_rfind_matchb(uint8_t const *scan_str, size_t scan_size, uint8_t const *open_str, size_t open_size, uint8_t const *clos_str, size_t clos_size);
PRIVATE ATTR_PURE WUNUSED ATTR_INS(1, 2) ATTR_INS(3, 4) ATTR_INS(5, 6) uint16_t const *DCALL dee_rfind_matchw(uint16_t const *scan_str, size_t scan_size, uint16_t const *open_str, size_t open_size, uint16_t const *clos_str, size_t clos_size);
PRIVATE ATTR_PURE WUNUSED ATTR_INS(1, 2) ATTR_INS(3, 4) ATTR_INS(5, 6) uint32_t const *DCALL dee_rfind_matchl(uint32_t const *scan_str, size_t scan_size, uint32_t const *open_str, size_t open_size, uint32_t const *clos_str, size_t clos_size);
PRIVATE ATTR_PURE WUNUSED ATTR_INS(1, 2) ATTR_INS(3, 4) ATTR_INS(5, 6) byte_t const *DCALL dee_rfind_casematch_ascii(byte_t const *scan_str, size_t scan_size, byte_t const *open_str, size_t open_size, byte_t const *clos_str, size_t clos_size);
PRIVATE ATTR_PURE WUNUSED ATTR_INS(1, 2) ATTR_INS(3, 4) ATTR_INS(5, 6) uint8_t const *DCALL dee_rfind_casematchb(uint8_t const *scan_str, size_t scan_size, uint8_t const *open_str, size_t open_size, uint8_t const *clos_str, size_t clos_size, size_t *p_match_length);
PRIVATE ATTR_PURE WUNUSED ATTR_INS(1, 2) ATTR_INS(3, 4) ATTR_INS(5, 6) uint16_t const *DCALL dee_rfind_casematchw(uint16_t const *scan_str, size_t scan_size, uint16_t const *open_str, size_t open_size, uint16_t const *clos_str, size_t clos_size, size_t *p_match_length);
PRIVATE ATTR_PURE WUNUSED ATTR_INS(1, 2) ATTR_INS(3, 4) ATTR_INS(5, 6) uint32_t const *DCALL dee_rfind_casematchl(uint32_t const *scan_str, size_t scan_size, uint32_t const *open_str, size_t open_size, uint32_t const *clos_str, size_t clos_size, size_t *p_match_length);
PRIVATE ATTR_PURE WUNUSED ATTR_INS(1, 2) ATTR_INS(3, 4) int DCALL dee_wildcompareb(uint8_t const *string, size_t string_length, uint8_t const *pattern, size_t pattern_length);
PRIVATE ATTR_PURE WUNUSED ATTR_INS(1, 2) ATTR_INS(3, 4) int DCALL dee_wildcomparew(uint16_t const *string, size_t string_length, uint16_t const *pattern, size_t pattern_length);
PRIVATE ATTR_PURE WUNUSED ATTR_INS(1, 2) ATTR_INS(3, 4) int DCALL dee_wildcomparel(uint32_t const *string, size_t string_length, uint32_t const *pattern, size_t pattern_length);
PRIVATE ATTR_PURE WUNUSED ATTR_INS(1, 2) ATTR_INS(3, 4) int DCALL dee_wildcasecompare_ascii(uint8_t const *string, size_t string_length, uint8_t const *pattern, size_t pattern_length);
PRIVATE WUNUSED ATTR_INOUT(1) ATTR_INOUT(2) int DCALL dee_wildcasecompareb(struct unicode_foldreader *__restrict string, struct unicode_foldreader *__restrict pattern);
PRIVATE WUNUSED ATTR_INOUT(1) ATTR_INOUT(2) int DCALL dee_wildcasecomparew(struct unicode_foldreader *__restrict string, struct unicode_foldreader *__restrict pattern);
PRIVATE WUNUSED ATTR_INOUT(1) ATTR_INOUT(2) int DCALL dee_wildcasecomparel(struct unicode_foldreader *__restrict string, struct unicode_foldreader *__restrict pattern);
#else /* __INTELLISENSE__ */
#define DEFINE_STRING_FUNCTIONS_CASEASCII
#include "string_functions-bwl.c.inl"
#define DEFINE_STRING_FUNCTIONS_B
#include "string_functions-bwl.c.inl"
#define DEFINE_STRING_FUNCTIONS_W
#include "string_functions-bwl.c.inl"
#define DEFINE_STRING_FUNCTIONS_L
#include "string_functions-bwl.c.inl"
#endif /* !__INTELLISENSE__ */

#undef mempfilb
#define mempfilb dee_mempfilb
#undef mempfilw
#define mempfilw dee_mempfilw
#undef mempfill
#define mempfill dee_mempfill


typedef DeeStringObject String;

/* Helper macro to facilitate the logic when it comes to custom string sub-ranges.
 * @param: p_start: [1..1] Pointer to the "size_t start" given by the callers
 * @param: p_end:   [1..1] Pointer to the "size_t end" given by the callers (may get adjusted)
 * @param: p_mylen: [1..1][in]  Pointer to the *full* length of the associated string
 *                        [out] Size of the effective sub-range (OUT(*p_end) - OUT(*p_start))
 * @param: Lnegative_len: Name of a label to jump to when OUT(*p_mylen) would end up negative */
#define CLAMP_SUBSTR(/*in*/ p_start, /*in|out*/ p_end,            \
                     /*in|out*/ p_mylen, Lnegative_len)           \
	do {                                                          \
		if likely(*(p_end) > *(p_mylen))                          \
			*(p_end) = *(p_mylen);                                \
		if unlikely(OVERFLOW_USUB(*(p_end), *(p_start), p_mylen)) \
			goto Lnegative_len;                                   \
	}	__WHILE0
/* Same as `CLAMP_SUBSTR()', but accepts "Lempty_len" which is
 * jumped to when OUT(*p_mylen) would end up negative, or zero. */
#define CLAMP_SUBSTR_NONEMPTY(/*in*/ p_start, /*in|out*/ p_end, \
                              /*in|out*/ p_mylen, Lempty_len)   \
	do {                                                        \
		if likely(*(p_end) > *(p_mylen))                        \
			*(p_end) = *(p_mylen);                              \
		if unlikely(*(p_start) >= *(p_end))                     \
			goto Lempty_len;                                    \
		*(p_mylen) = *(p_end) - *(p_start);                     \
	}	__WHILE0

/* Same as `CLAMP_SUBSTR()', but set `*p_mylen = 0' when the range
 * underflows, rather than jump to a given label */
#define CLAMP_SUBSTR_IMPLICIT(/*in*/ p_start, /*in|out*/ p_end,   \
                              /*in|out*/ p_mylen)                 \
	do {                                                          \
		if likely(*(p_end) > *(p_mylen))                          \
			*(p_end) = *(p_mylen);                                \
		if unlikely(OVERFLOW_USUB(*(p_end), *(p_start), p_mylen)) \
			*(p_mylen) = 0;                                       \
	}	__WHILE0


DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_UNICODE_STRING_FUNCTIONS_H */
