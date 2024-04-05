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
#ifndef GUARD_DEEMON_OBJECTS_UNICODE_STRING_FUNCTIONS_H
#define GUARD_DEEMON_OBJECTS_UNICODE_STRING_FUNCTIONS_H 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/object.h>
#include <deemon/string.h>
#include <deemon/stringutils.h>
#include <deemon/system-features.h> /* memrchr(), memmem(), ... */

#include <hybrid/minmax.h>

#include <stddef.h>

#include "../../runtime/runtime_error.h"
#include "../../runtime/strings.h"

#undef SSIZE_MAX
#include <hybrid/limitcore.h>
#define SSIZE_MAX __SSIZE_MAX__

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








/* Case-insensitive string functions. */
#undef memcasechr
#undef memcaserchr
#undef memcasechrb
#undef memcaserchrb
#undef memcasechrw
#undef memcaserchrw
#undef memcasechrl
#undef memcaserchrl
#define memcasechr(haystack, needle, haystack_length)   dee_memcasechrb((uint8_t *)(haystack), (uint8_t)(needle), haystack_length)
#define memcaserchr(haystack, needle, haystack_length)  dee_memcaserchrb((uint8_t *)(haystack), (uint8_t)(needle), haystack_length)
#define memcasechrb(haystack, needle, haystack_length)  dee_memcasechrb(haystack, needle, haystack_length)
#define memcaserchrb(haystack, needle, haystack_length) dee_memcaserchrb(haystack, needle, haystack_length)
#define memcasechrw(haystack, needle, haystack_length)  dee_memcasechrw(haystack, needle, haystack_length)
#define memcaserchrw(haystack, needle, haystack_length) dee_memcaserchrw(haystack, needle, haystack_length)
#define memcasechrl(haystack, needle, haystack_length)  dee_memcasechrl(haystack, needle, haystack_length)
#define memcaserchrl(haystack, needle, haystack_length) dee_memcaserchrl(haystack, needle, haystack_length)

#ifdef __INTELLISENSE__
#define DEFINE_FOLD_COMPARE(name, T)                            \
	PRIVATE WUNUSED ATTR_INS(1, 2) ATTR_OUTS(3, 4) size_t DCALL \
	name(T const *__restrict data, size_t datalen,              \
	     uint32_t fold[UNICODE_FOLDED_MAX], size_t fold_len);
#else /* __INTELLISENSE__ */
#define DEFINE_FOLD_COMPARE(name, T)                                             \
	PRIVATE WUNUSED ATTR_INS(1, 2) ATTR_OUTS(3, 4) size_t DCALL                  \
	name(T const *__restrict data, size_t datalen,                               \
	     uint32_t fold[UNICODE_FOLDED_MAX], size_t fold_len) {                   \
		uint32_t buf[UNICODE_FOLDED_MAX];                                        \
		size_t buflen;                                                           \
		ASSERT(datalen >= 1);                                                    \
		buflen = DeeUni_ToFolded(data[0], buf);                                  \
		switch (fold_len) {                                                      \
		case 1:                                                                  \
			if (buf[0] == fold[0])                                               \
				goto ok_1;                                                       \
			if (buflen >= 2 && buf[1] == fold[0])                                \
				goto ok_1;                                                       \
			if (buflen >= 3 && buf[2] == fold[0])                                \
				goto ok_1;                                                       \
			break;                                                               \
		case 2:                                                                  \
			if (buflen == 1) {                                                   \
				if (buf[0] != fold[0])                                           \
					break;                                                       \
				if (datalen < 2)                                                 \
					break;                                                       \
				(void)DeeUni_ToFolded(data[1], buf);                             \
				if (buf[0] == fold[1])                                           \
					goto ok_2;                                                   \
			} else if (buflen == 2) {                                            \
				if (buf[0] == fold[0] && buf[1] == fold[1])                      \
					goto ok_1;                                                   \
				if (buf[1] == fold[0]) {                                         \
					if (datalen < 2)                                             \
						break;                                                   \
					(void)DeeUni_ToFolded(data[1], buf);                         \
					if (buf[0] == fold[1])                                       \
						goto ok_2;                                               \
				}                                                                \
			} else {                                                             \
				if (buf[0] == fold[0] && buf[1] == fold[1])                      \
					goto ok_1;                                                   \
				if (buf[1] == fold[0] && buf[2] == fold[1])                      \
					goto ok_1;                                                   \
				if (buf[2] == fold[0]) {                                         \
					if (datalen < 2)                                             \
						break;                                                   \
					(void)DeeUni_ToFolded(data[1], buf);                         \
					if (buf[0] == fold[1])                                       \
						goto ok_2;                                               \
				}                                                                \
			}                                                                    \
			break;                                                               \
		case 3:                                                                  \
			if (buflen == 1) {                                                   \
				if (buf[0] != fold[0])                                           \
					break;                                                       \
				if (datalen < 2)                                                 \
					break;                                                       \
				buflen = DeeUni_ToFolded(data[1], buf);                          \
				if (buf[0] != fold[1])                                           \
					break;                                                       \
				if (buflen == 1) {                                               \
					if (datalen < 3)                                             \
						break;                                                   \
					(void)DeeUni_ToFolded(data[2], buf);                         \
					if (buf[0] == fold[2])                                       \
						goto ok_3;                                               \
				} else {                                                         \
					if (buf[1] == fold[2])                                       \
						goto ok_2;                                               \
				}                                                                \
			} else if (buflen == 2) {                                            \
				if (buf[0] == fold[0] && buf[1] == fold[1]) {                    \
					if (datalen < 2)                                             \
						break;                                                   \
					(void)DeeUni_ToFolded(data[1], buf);                         \
					if (buf[0] == fold[2])                                       \
						goto ok_2;                                               \
				}                                                                \
				if (buf[1] == fold[0]) {                                         \
					if (datalen < 2)                                             \
						break;                                                   \
					buflen = DeeUni_ToFolded(data[1], buf);                      \
					if (buf[0] != fold[1])                                       \
						break;                                                   \
					if (buflen == 1) {                                           \
						if (datalen < 3)                                         \
							break;                                               \
						(void)DeeUni_ToFolded(data[2], buf);                     \
						if (buf[0] == fold[2])                                   \
							goto ok_3;                                           \
					} else {                                                     \
						if (buf[1] == fold[2])                                   \
							goto ok_2;                                           \
					}                                                            \
				}                                                                \
			} else {                                                             \
				if (buf[0] == fold[0] && buf[1] == fold[1] && buf[2] == fold[2]) \
					goto ok_1;                                                   \
				if (buf[1] == fold[0] && buf[2] == fold[1]) {                    \
					if (datalen < 2)                                             \
						break;                                                   \
					(void)DeeUni_ToFolded(data[1], buf);                         \
					if (buf[0] == fold[2])                                       \
						goto ok_2;                                               \
				}                                                                \
				if (buf[2] == fold[0]) {                                         \
					if (datalen < 2)                                             \
						break;                                                   \
					buflen = DeeUni_ToFolded(data[1], buf);                      \
					if (buf[0] != fold[1])                                       \
						break;                                                   \
					if (buflen == 1) {                                           \
						if (datalen < 3)                                         \
							break;                                               \
						buflen = DeeUni_ToFolded(data[2], buf);                  \
						if (buf[0] == fold[2])                                   \
							goto ok_3;                                           \
					} else {                                                     \
						if (buf[1] == fold[2])                                   \
							goto ok_2;                                           \
					}                                                            \
				}                                                                \
			}                                                                    \
			break;                                                               \
		default: __builtin_unreachable();                                        \
		}                                                                        \
		return 0;                                                                \
	ok_1:                                                                        \
		return 1;                                                                \
	ok_2:                                                                        \
		return 2;                                                                \
	ok_3:                                                                        \
		return 3;                                                                \
	}
#endif /* !__INTELLISENSE__ */
#undef foldcmpb
#undef foldcmpw
#undef foldcmpl
#define foldcmpb dee_foldcmpb
#define foldcmpw dee_foldcmpw
#define foldcmpl dee_foldcmpl
DEFINE_FOLD_COMPARE(dee_foldcmpb, uint8_t)
DEFINE_FOLD_COMPARE(dee_foldcmpw, uint16_t)
DEFINE_FOLD_COMPARE(dee_foldcmpl, uint32_t)
#undef DEFINE_FOLD_COMPARE



#define DEFINE_MEMCASECHR(name, rname, T, dee_foldcmp)             \
	LOCAL ATTR_PURE WUNUSED ATTR_INS(1, 3) T *DCALL                \
	name(T const *__restrict haystack,                             \
	     T needle, size_t haystack_length) {                       \
		uint32_t fold[UNICODE_FOLDED_MAX];                         \
		size_t len = DeeUni_ToFolded(needle, fold);                \
		for (; haystack_length; ++haystack, --haystack_length) {   \
			if (dee_foldcmp(haystack, haystack_length, fold, len)) \
				return (T *)haystack;                              \
		}                                                          \
		return NULL;                                               \
	}                                                              \
	LOCAL ATTR_PURE WUNUSED ATTR_INS(1, 3) T *DCALL                \
	rname(T const *__restrict haystack,                            \
	      T needle, size_t haystack_length) {                      \
		T *iter = (T *)haystack + haystack_length;                 \
		uint32_t fold[UNICODE_FOLDED_MAX];                         \
		size_t len     = DeeUni_ToFolded(needle, fold);            \
		size_t datalen = 0;                                        \
		while (iter > (T *)haystack) {                             \
			--iter;                                                \
			++datalen;                                             \
			if (dee_foldcmp(iter, datalen, fold, len))             \
				return iter;                                       \
		}                                                          \
		return NULL;                                               \
	}
DEFINE_MEMCASECHR(dee_memcasechrb, dee_memcaserchrb, uint8_t, dee_foldcmpb)
DEFINE_MEMCASECHR(dee_memcasechrw, dee_memcaserchrw, uint16_t, dee_foldcmpw)
DEFINE_MEMCASECHR(dee_memcasechrl, dee_memcaserchrl, uint32_t, dee_foldcmpl)
#undef DEFINE_MEMCASECHR

#define DEFINE_UNICODE_ISOLDREADER_API(name, T)                                     \
	struct name {                                                                   \
		T const *uf_dataptr;                                                        \
		size_t   uf_datalen;                                                        \
		uint32_t uf_buf[UNICODE_FOLDED_MAX];                                        \
		uint8_t  uf_len;                                                            \
		uint8_t  uf_idx;                                                            \
	};                                                                              \
	LOCAL WUNUSED ATTR_INOUT(1) uint32_t DCALL                                      \
	name##_getc(struct name *__restrict self) {                                     \
		if (self->uf_idx < self->uf_len)                                            \
			return self->uf_buf[self->uf_idx++];                                    \
		ASSERT(self->uf_datalen);                                                   \
		self->uf_idx = 1;                                                           \
		self->uf_len = (uint8_t)DeeUni_ToFolded(*self->uf_dataptr, self->uf_buf);   \
		++self->uf_dataptr;                                                         \
		--self->uf_datalen;                                                         \
		return self->uf_buf[0];                                                     \
	}                                                                               \
	LOCAL WUNUSED ATTR_INOUT(1) uint32_t DCALL                                      \
	name##_getc_back(struct name *__restrict self) {                                \
		if (self->uf_idx < self->uf_len)                                            \
			return self->uf_buf[--self->uf_len];                                    \
		ASSERT(self->uf_datalen);                                                   \
		self->uf_idx = 0;                                                           \
		--self->uf_datalen;                                                         \
		self->uf_len = (uint8_t)DeeUni_ToFolded(self->uf_dataptr[self->uf_datalen], \
		                                        self->uf_buf) -                     \
		               1;                                                           \
		return self->uf_buf[self->uf_len];                                          \
	}
DEFINE_UNICODE_ISOLDREADER_API(unicode_foldreaderb, uint8_t)
DEFINE_UNICODE_ISOLDREADER_API(unicode_foldreaderw, uint16_t)
DEFINE_UNICODE_ISOLDREADER_API(unicode_foldreaderl, uint32_t)
#undef DEFINE_UNICODE_ISOLDREADER_API

#ifdef __INTELLISENSE__
extern "C++" {
uint32_t unicode_foldreader_getc(struct unicode_foldreaderb &x);
uint32_t unicode_foldreader_getc(struct unicode_foldreaderw &x);
uint32_t unicode_foldreader_getc(struct unicode_foldreaderl &x);
uint32_t unicode_foldreader_getc_back(struct unicode_foldreaderb &x);
uint32_t unicode_foldreader_getc_back(struct unicode_foldreaderw &x);
uint32_t unicode_foldreader_getc_back(struct unicode_foldreaderl &x);
}
#elif !defined(__NO_builtin_choose_expr)
#define unicode_foldreader_getc(x)                                                                                    \
	__builtin_choose_expr(sizeof(*(x).uf_dataptr) == 1, unicode_foldreaderb_getc((struct unicode_foldreaderb *)&(x)), \
	__builtin_choose_expr(sizeof(*(x).uf_dataptr) == 2, unicode_foldreaderw_getc((struct unicode_foldreaderw *)&(x)), \
	                                                    unicode_foldreaderl_getc((struct unicode_foldreaderl *)&(x))))

#define unicode_foldreader_getc_back(x)                                                                                    \
	__builtin_choose_expr(sizeof(*(x).uf_dataptr) == 1, unicode_foldreaderb_getc_back((struct unicode_foldreaderb *)&(x)), \
	__builtin_choose_expr(sizeof(*(x).uf_dataptr) == 2, unicode_foldreaderw_getc_back((struct unicode_foldreaderw *)&(x)), \
	                                                    unicode_foldreaderl_getc_back((struct unicode_foldreaderl *)&(x))))

#else /* !__NO_builtin_choose_expr */
#define unicode_foldreader_getc(x)                                    \
	(sizeof(*(x).uf_dataptr) == 1                                     \
	 ? unicode_foldreaderb_getc((struct unicode_foldreaderb *)&(x))   \
	 : sizeof(*(x).uf_dataptr) == 2                                   \
	   ? unicode_foldreaderw_getc((struct unicode_foldreaderw *)&(x)) \
	   : unicode_foldreaderl_getc((struct unicode_foldreaderl *)&(x)))
#define unicode_foldreader_getc_back(x)                                    \
	(sizeof(*(x).uf_dataptr) == 1                                          \
	 ? unicode_foldreaderb_getc_back((struct unicode_foldreaderb *)&(x))   \
	 : sizeof(*(x).uf_dataptr) == 2                                        \
	   ? unicode_foldreaderw_getc_back((struct unicode_foldreaderw *)&(x)) \
	   : unicode_foldreaderl_getc_back((struct unicode_foldreaderl *)&(x)))
#endif /* __NO_builtin_choose_expr */
#define unicode_foldreader_init(x, data, len) \
	((x).uf_dataptr = (data),                 \
	 (x).uf_datalen = (len),                  \
	 (x).uf_len = (x).uf_idx = 0)
#define unicode_foldreader_empty(x) \
	(!(x).uf_datalen && ((x).uf_idx >= (x).uf_len))
#define DEE_PRIVATE_UNICODE_ISOLDREADER_uint8_t  unicode_foldreaderb
#define DEE_PRIVATE_UNICODE_ISOLDREADER_uint16_t unicode_foldreaderw
#define DEE_PRIVATE_UNICODE_ISOLDREADER_uint32_t unicode_foldreaderl
#define unicode_foldreader(T) struct PP_PRIVATE_CAT2(DEE_PRIVATE_UNICODE_ISOLDREADER_, T)





/* Use libc functions for case-insensitive UTF-8 string compare when available. */
#define MEMCASEEQB(a, a_size, b, b_size)         dee_memcaseeqb(a, a_size, b, b_size)
#define MEMCASEEQW(a, a_size, b, b_size)         dee_memcaseeqw(a, a_size, b, b_size)
#define MEMCASEEQL(a, a_size, b, b_size)         dee_memcaseeql(a, a_size, b, b_size)
#define MEMCASESTARTSWITHB(a, a_size, b, b_size) dee_memcasestartswithb(a, a_size, b, b_size)
#define MEMCASESTARTSWITHW(a, a_size, b, b_size) dee_memcasestartswithw(a, a_size, b, b_size)
#define MEMCASESTARTSWITHL(a, a_size, b, b_size) dee_memcasestartswithl(a, a_size, b, b_size)
#define MEMCASEENDSWITHB(a, a_size, b, b_size)   dee_memcaseendswithb(a, a_size, b, b_size)
#define MEMCASEENDSWITHW(a, a_size, b, b_size)   dee_memcaseendswithw(a, a_size, b, b_size)
#define MEMCASEENDSWITHL(a, a_size, b, b_size)   dee_memcaseendswithl(a, a_size, b, b_size)

#define DEFINE_MEMCASEEQ(name, T)                     \
	LOCAL bool DCALL                                  \
	name(T const *a, size_t a_size,                   \
	     T const *b, size_t b_size) {                 \
		unicode_foldreader(T) a_reader;               \
		unicode_foldreader(T) b_reader;               \
		unicode_foldreader_init(a_reader, a, a_size); \
		unicode_foldreader_init(b_reader, b, b_size); \
		while (!unicode_foldreader_empty(a_reader)) { \
			if (unicode_foldreader_empty(b_reader))   \
				return false;                         \
			if (unicode_foldreader_getc(a_reader) !=  \
			    unicode_foldreader_getc(b_reader))    \
				return false;                         \
		}                                             \
		return unicode_foldreader_empty(b_reader);    \
	}
#undef memcaseeqb
#undef memcaseeqw
#undef memcaseeql
#define memcaseeqb dee_memcaseeqb
#define memcaseeqw dee_memcaseeqw
#define memcaseeql dee_memcaseeql
DEFINE_MEMCASEEQ(dee_memcaseeqb, uint8_t)
DEFINE_MEMCASEEQ(dee_memcaseeqw, uint16_t)
DEFINE_MEMCASEEQ(dee_memcaseeql, uint32_t)
#undef DEFINE_MEMCASEEQ

#define DEFINE_MEMCASESTARTSWITH(name, T)             \
	LOCAL size_t DCALL                                \
	name(T const *a, size_t a_size,                   \
	     T const *b, size_t b_size) {                 \
		unicode_foldreader(T) a_reader;               \
		unicode_foldreader(T) b_reader;               \
		unicode_foldreader_init(a_reader, a, a_size); \
		unicode_foldreader_init(b_reader, b, b_size); \
		while (!unicode_foldreader_empty(b_reader)) { \
			if (unicode_foldreader_empty(a_reader) || \
			    (unicode_foldreader_getc(a_reader) != \
			     unicode_foldreader_getc(b_reader)))  \
				return 0;                             \
		}                                             \
		return (size_t)(a_reader.uf_dataptr - a);     \
	}
#undef memcasestartswithb
#undef memcasestartswithw
#undef memcasestartswithl
#define memcasestartswithb dee_memcasestartswithb
#define memcasestartswithw dee_memcasestartswithw
#define memcasestartswithl dee_memcasestartswithl
DEFINE_MEMCASESTARTSWITH(dee_memcasestartswithb, uint8_t)
DEFINE_MEMCASESTARTSWITH(dee_memcasestartswithw, uint16_t)
DEFINE_MEMCASESTARTSWITH(dee_memcasestartswithl, uint32_t)
#undef DEFINE_MEMCASESTARTSWITH

#define DEFINE_MEMCASEENDSWITH(name, T)                    \
	LOCAL size_t DCALL                                     \
	name(T const *a, size_t a_size,                        \
	     T const *b, size_t b_size) {                      \
		unicode_foldreader(T) a_reader;                    \
		unicode_foldreader(T) b_reader;                    \
		unicode_foldreader_init(a_reader, a, a_size);      \
		unicode_foldreader_init(b_reader, b, b_size);      \
		while (!unicode_foldreader_empty(b_reader)) {      \
			if (unicode_foldreader_empty(a_reader) ||      \
			    (unicode_foldreader_getc_back(a_reader) != \
			     unicode_foldreader_getc_back(b_reader)))  \
				return 0;                                  \
		}                                                  \
		return a_size - a_reader.uf_datalen;               \
	}
#undef memcaseendswithb
#undef memcaseendswithw
#undef memcaseendswithl
#define memcaseendswithb dee_memcaseendswithb
#define memcaseendswithw dee_memcaseendswithw
#define memcaseendswithl dee_memcaseendswithl
DEFINE_MEMCASEENDSWITH(dee_memcaseendswithb, uint8_t)
DEFINE_MEMCASEENDSWITH(dee_memcaseendswithw, uint16_t)
DEFINE_MEMCASEENDSWITH(dee_memcaseendswithl, uint32_t)
#undef DEFINE_MEMCASEENDSWITH

#define DEFINE_MEMCASEMEM(name, rname, T, MEMCASESTARTSWITH, MEMCASEENDSWITH)                   \
	LOCAL T *DCALL                                                                              \
	name(T const *__restrict haystack, size_t haystack_length,                                  \
	     T const *__restrict needle, size_t needle_length,                                      \
	     size_t *p_match_length) {                                                              \
		for (; haystack_length; --haystack_length, ++haystack) {                                \
			size_t match_length;                                                                \
			match_length = MEMCASESTARTSWITH(haystack, haystack_length, needle, needle_length); \
			if (match_length) {                                                                 \
				if (p_match_length)                                                             \
					*p_match_length = match_length;                                             \
				return (T *)haystack;                                                           \
			}                                                                                   \
		}                                                                                       \
		return NULL;                                                                            \
	}                                                                                           \
	LOCAL T *DCALL                                                                              \
	rname(T const *__restrict haystack, size_t haystack_length,                                 \
	      T const *__restrict needle, size_t needle_length,                                     \
	      size_t *p_match_length) {                                                             \
		for (; haystack_length; --haystack_length) {                                            \
			size_t match_length;                                                                \
			match_length = MEMCASEENDSWITH(haystack, haystack_length, needle, needle_length);   \
			if (match_length) {                                                                 \
				if (p_match_length)                                                             \
					*p_match_length = match_length;                                             \
				return (T *)haystack + haystack_length - match_length;                          \
			}                                                                                   \
		}                                                                                       \
		return NULL;                                                                            \
	}
#undef memcasememb
#undef memcasermemb
#undef memcasememw
#undef memcasermemw
#undef memcasememl
#undef memcasermeml
#define memcasememb  dee_memcasememb
#define memcasermemb dee_memcasermemb
#define memcasememw  dee_memcasememw
#define memcasermemw dee_memcasermemw
#define memcasememl  dee_memcasememl
#define memcasermeml dee_memcasermeml
DEFINE_MEMCASEMEM(dee_memcasememb, dee_memcasermemb, uint8_t, MEMCASESTARTSWITHB, MEMCASEENDSWITHB)
DEFINE_MEMCASEMEM(dee_memcasememw, dee_memcasermemw, uint16_t, MEMCASESTARTSWITHW, MEMCASEENDSWITHW)
DEFINE_MEMCASEMEM(dee_memcasememl, dee_memcasermeml, uint32_t, MEMCASESTARTSWITHL, MEMCASEENDSWITHL)
#undef DEFINE_MEMCASEMEM

#undef memcasecmpb
#define memcasecmpb dee_memcasecmpb
LOCAL ATTR_PURE WUNUSED ATTR_INS(1, 2) ATTR_INS(3, 4) int DCALL
dee_memcasecmpb(uint8_t const *a, size_t a_size,
                uint8_t const *b, size_t b_size) {
	unicode_foldreader(uint8_t) a_reader;
	unicode_foldreader(uint8_t) b_reader;
	unicode_foldreader_init(a_reader, a, a_size);
	unicode_foldreader_init(b_reader, b, b_size);
	for (;;) {
		uint32_t cha, chb;
		if (unicode_foldreader_empty(a_reader))
			return unicode_foldreader_empty(b_reader) ? 0 : -1;
		if (unicode_foldreader_empty(b_reader))
			return 1;
		cha = unicode_foldreader_getc(a_reader);
		chb = unicode_foldreader_getc(b_reader);
		if (cha != chb)
			return cha < chb ? -1 : 1;
	}
}



/* ASCII case-insensitive functions (for `Bytes'). */
#undef memasciicaseeq
#define memasciicaseeq dee_memasciicaseeq
LOCAL ATTR_PURE WUNUSED ATTR_INS(1, 3) ATTR_INS(2, 3) bool DCALL
dee_memasciicaseeq(uint8_t const *a, uint8_t const *b, size_t num_bytes) {
	for (; num_bytes; --num_bytes, ++a, ++b) {
		uint8_t lhs = *a, rhs = *b;
		if (lhs != rhs) {
			lhs = (uint8_t)tolower(lhs);
			rhs = (uint8_t)tolower(rhs);
			if (lhs != rhs)
				return false;
		}
	}
	return true;
}

#undef memasciicasechr
#define memasciicasechr dee_memasciicasechr
LOCAL ATTR_PURE WUNUSED ATTR_INS(1, 3) uint8_t *DCALL
dee_memasciicasechr(uint8_t const *__restrict haystack,
                    uint8_t needle, size_t haystack_length) {
	needle = (uint8_t)tolower(needle);
	while (haystack_length--) {
		if ((uint8_t)tolower(*haystack) == needle)
			return (uint8_t *)haystack;
		++haystack;
	}
	return NULL;
}

#undef memasciicaserchr
#define memasciicaserchr dee_memasciicaserchr
LOCAL ATTR_PURE WUNUSED ATTR_INS(1, 3) uint8_t *DCALL
dee_memasciicaserchr(uint8_t const *__restrict haystack,
                     uint8_t needle, size_t haystack_length) {
	uint8_t *iter;
	iter   = (uint8_t *)haystack + haystack_length;
	needle = (uint8_t)tolower(needle);
	while (iter > (uint8_t *)haystack) {
		--iter;
		if ((uint8_t)tolower(*iter) == needle)
			return iter;
	}
	return NULL;
}

#undef memasciicasemem
#define memasciicasemem dee_memasciicasemem
LOCAL ATTR_PURE WUNUSED ATTR_INS(1, 2) ATTR_INS(3, 4) uint8_t *DCALL
dee_memasciicasemem(uint8_t const *haystack, size_t haystack_length,
                    uint8_t const *needle, size_t needle_length) {
	uint8_t *candidate, marker;
	if unlikely(!needle_length || needle_length > haystack_length)
		return NULL;
	haystack_length -= needle_length - 1;
	marker = (uint8_t)tolower(*needle);
	while ((candidate = dee_memasciicasechr(haystack, marker, haystack_length)) != NULL) {
		if (dee_memasciicaseeq(candidate, needle, needle_length))
			return candidate;
		++candidate;
		haystack_length = (size_t)((haystack + haystack_length) - candidate);
		haystack        = candidate;
	}
	return NULL;
}

#undef memasciicasermem
#define memasciicasermem dee_memasciicasermem
LOCAL ATTR_PURE WUNUSED ATTR_INS(1, 2) ATTR_INS(3, 4) uint8_t *DCALL
dee_memasciicasermem(uint8_t const *haystack, size_t haystack_length,
                     uint8_t const *needle, size_t needle_length) {
	uint8_t *candidate, marker;
	if unlikely(!needle_length || needle_length > haystack_length)
		return NULL;
	marker = (uint8_t)tolower(*needle);
	haystack_length -= needle_length - 1;
	while ((candidate = dee_memasciicaserchr(haystack, marker, haystack_length)) != NULL) {
		if (dee_memasciicaseeq(candidate, needle, needle_length))
			return candidate;
		if unlikely(candidate == haystack)
			break;
		haystack_length = (size_t)(candidate - haystack);
	}
	return NULL;
}

#undef memasciicasecmp
#define memasciicasecmp dee_memasciicasecmp
LOCAL ATTR_PURE WUNUSED ATTR_INS(1, 3) ATTR_INS(2, 3) int
dee_memasciicasecmp(uint8_t const *a, uint8_t const *b, size_t num_bytes) {
	for (; num_bytes; --num_bytes, ++a, ++b) {
		uint8_t lhs = *a, rhs = *b;
		if (lhs != rhs) {
			lhs = (uint8_t)tolower(lhs);
			rhs = (uint8_t)tolower(rhs);
			if (lhs != rhs)
				return (int)lhs - (int)rhs;
		}
	}
	return 0;
}




#define STRCASEEQ(a, b) dee_strcaseeq(a, b)
LOCAL ATTR_PURE WUNUSED NONNULL((1, 2)) bool DCALL
dee_strcaseeq(char const *a, char const *b) {
	while (*a && tolower(*a) == tolower(*b)) {
		++a;
		++b;
	}
	return !*b;
}

#undef asciicaseeq
#define asciicaseeq dee_asciicaseeq
LOCAL ATTR_PURE WUNUSED ATTR_INS(1, 3) ATTR_INS(2, 3) bool DCALL
dee_asciicaseeq(char const *a, char const *b, size_t num_chars) {
	for (; num_chars; --num_chars, ++a, ++b) {
		char ca = *a, cb = *b;
		if (ca != cb) {
			ca = (char)(unsigned char)tolower((unsigned char)ca);
			cb = (char)(unsigned char)tolower((unsigned char)cb);
			if (ca != cb)
				return false;
		}
	}
	return true;
}


/* As found here: https://en.wikipedia.org/wiki/Levenshtein_distance */
#define DEFINE_FUZZY_COMPARE_FUNCTION(name, T, transform)      \
	PRIVATE dssize_t DCALL                                     \
	name(T const *__restrict a, size_t alen,                   \
	     T const *__restrict b, size_t blen) {                 \
		size_t *v0, *v1, i, j, cost, temp;                     \
		if unlikely(!alen)                                     \
			return blen;                                       \
		if unlikely(!blen)                                     \
			return alen;                                       \
		v0 = (size_t *)Dee_Mallocac(blen + 1, sizeof(size_t)); \
		if unlikely(!v0)                                       \
			return -1;                                         \
		v1 = (size_t *)Dee_Mallocac(blen + 1, sizeof(size_t)); \
		if unlikely(!v1) {                                     \
			Dee_Freea(v0);                                     \
			return -1;                                         \
		}                                                      \
		for (i = 0; i < blen; ++i)                             \
			v0[i] = i;                                         \
		for (i = 0; i < alen; ++i) {                           \
			T a_value;                                         \
			a_value = transform(a[i]);                         \
			v1[0]   = i + 1;                                   \
			for (j = 0; j < blen; ++j) {                       \
				cost = (a_value == transform(b[j])) ? 0 : 1;   \
				cost += v0[j];                                 \
				temp = v1[j] + 1;                              \
				if (temp < cost)                               \
					cost = temp;                               \
				temp = v0[j + 1] + 1;                          \
				if (temp < cost)                               \
					cost = temp;                               \
				v1[j + 1] = cost;                              \
			}                                                  \
			memcpyc(v0, v1, blen, sizeof(size_t));             \
		}                                                      \
		temp = v1[blen];                                       \
		Dee_Freea(v0);                                         \
		Dee_Freea(v1);                                         \
		if (temp > SSIZE_MAX)                                  \
			temp = SSIZE_MAX;                                  \
		return temp;                                           \
	}
#undef fuzzy_compareb
#undef fuzzy_comparew
#undef fuzzy_comparel
#undef fuzzy_asciicasecompareb
#define fuzzy_compareb          dee_fuzzy_compareb
#define fuzzy_comparew          dee_fuzzy_comparew
#define fuzzy_comparel          dee_fuzzy_comparel
#define fuzzy_asciicasecompareb dee_fuzzy_asciicasecompareb
DEFINE_FUZZY_COMPARE_FUNCTION(dee_fuzzy_compareb, uint8_t, )
DEFINE_FUZZY_COMPARE_FUNCTION(dee_fuzzy_comparew, uint16_t, )
DEFINE_FUZZY_COMPARE_FUNCTION(dee_fuzzy_comparel, uint32_t, )
DEFINE_FUZZY_COMPARE_FUNCTION(dee_fuzzy_asciicasecompareb, uint8_t, (uint8_t)tolower)
#undef DEFINE_FUZZY_COMPARE_FUNCTION

#define DEFINE_FUZZY_FOLDCOMPARE_FUNCTION(name, T)                               \
	PRIVATE dssize_t DCALL                                                       \
	name(T const *__restrict a, size_t alen,                                     \
	     T const *__restrict b, size_t blen) {                                   \
		size_t *v0, *v1, i, j, cost, temp;                                       \
		unicode_foldreader(T) a_reader;                                          \
		unicode_foldreader(T) b_reader;                                          \
		size_t folded_alen, folded_blen;                                         \
		folded_alen = DeeUni_FoldedLength(a, alen);                              \
		folded_blen = DeeUni_FoldedLength(b, blen);                              \
		if unlikely(!folded_alen)                                                \
			return folded_blen;                                                  \
		if unlikely(!folded_blen)                                                \
			return folded_alen;                                                  \
		v0 = (size_t *)Dee_Mallocac(folded_blen + 1, sizeof(size_t));            \
		if unlikely(!v0)                                                         \
			return -1;                                                           \
		v1 = (size_t *)Dee_Mallocac(folded_blen + 1, sizeof(size_t));            \
		if unlikely(!v1) {                                                       \
			Dee_Freea(v0);                                                       \
			return -1;                                                           \
		}                                                                        \
		for (i = 0; i < folded_blen; ++i)                                        \
			v0[i] = i;                                                           \
		unicode_foldreader_init(a_reader, a, alen);                              \
		for (i = 0; i < folded_alen; ++i) {                                      \
			uint32_t a_value = unicode_foldreader_getc(a_reader);                \
			v1[0]            = i + 1;                                            \
			unicode_foldreader_init(b_reader, b, blen);                          \
			for (j = 0; j < folded_blen; ++j) {                                  \
				cost = (a_value == unicode_foldreader_getc(b_reader)) ? 0u : 1u; \
				cost += v0[j];                                                   \
				temp = v1[j] + 1;                                                \
				if (temp < cost)                                                 \
					cost = temp;                                                 \
				temp = v0[j + 1] + 1;                                            \
				if (temp < cost)                                                 \
					cost = temp;                                                 \
				v1[j + 1] = cost;                                                \
			}                                                                    \
			ASSERT(unicode_foldreader_empty(b_reader));                          \
			memcpyc(v0, v1, folded_blen, sizeof(size_t));                        \
		}                                                                        \
		ASSERT(unicode_foldreader_empty(a_reader));                              \
		temp = v1[folded_blen];                                                  \
		Dee_Freea(v0);                                                           \
		Dee_Freea(v1);                                                           \
		if (temp > SSIZE_MAX)                                                    \
			temp = SSIZE_MAX;                                                    \
		return temp;                                                             \
	}
#undef fuzzy_casecompareb
#undef fuzzy_casecomparew
#undef fuzzy_casecomparel
#define fuzzy_casecompareb dee_fuzzy_casecompareb
#define fuzzy_casecomparew dee_fuzzy_casecomparew
#define fuzzy_casecomparel dee_fuzzy_casecomparel
DEFINE_FUZZY_FOLDCOMPARE_FUNCTION(dee_fuzzy_casecompareb, uint8_t)
DEFINE_FUZZY_FOLDCOMPARE_FUNCTION(dee_fuzzy_casecomparew, uint16_t)
DEFINE_FUZZY_FOLDCOMPARE_FUNCTION(dee_fuzzy_casecomparel, uint32_t)
#undef DEFINE_FUZZY_FOLDCOMPARE_FUNCTION

#define DEFINE_VERSION_COMPARE_FUNCTION(name, T, transform, IF_TRANSFORM)               \
	PRIVATE int DCALL                                                                   \
	name(T *__restrict a, size_t a_size,                                                \
	     T *__restrict b, size_t b_size) {                                              \
		T ca, cb, *a_start = a;                                                         \
		while (a_size && b_size) {                                                      \
			if ((ca = *a) != (cb = *b)                                                  \
			    IF_TRANSFORM(&&((ca = (T)transform(ca)) != (cb = (T)transform(cb))))) { \
				struct unitraits *arec;                                                 \
				struct unitraits *brec;                                                 \
				uintptr_t vala, valb;                                                   \
				/* Unwind common digits. */                                             \
				while (a > a_start) {                                                   \
					if (!DeeUni_IsDigit(a[-1]))                                         \
						break;                                                          \
					cb = ca = *--a, --b;                                                \
					++a_size, ++b_size;                                                 \
				}                                                                       \
				/* Check if both strings have digit sequences in the same places. */    \
				arec = DeeUni_Descriptor(ca);                                           \
				brec = DeeUni_Descriptor(cb);                                           \
				if (!(arec->ut_flags & UNICODE_ISDIGIT) &&                              \
				    !(brec->ut_flags & UNICODE_ISDIGIT))                                \
					return ca < cb ? -1 : ca > cb ? 1 : 0;                              \
				/* Deal with leading zeros. */                                          \
				if ((arec->ut_flags & UNICODE_ISDIGIT) && arec->ut_digit_idx == 0)      \
					return -1;                                                          \
				if ((brec->ut_flags & UNICODE_ISDIGIT) && brec->ut_digit_idx == 0)      \
					return 1;                                                           \
				/* Compare digits. */                                                   \
				vala = arec->ut_digit_idx;                                              \
				valb = brec->ut_digit_idx;                                              \
				while (--a_size) {                                                      \
					ca = *++a;                                                          \
					arec = DeeUni_Descriptor(ca);                                       \
					if (!(arec->ut_flags & UNICODE_ISDIGIT))                            \
						break;                                                          \
					vala *= 10;                                                         \
					vala += arec->ut_digit_idx;                                         \
				}                                                                       \
				while (--b_size) {                                                      \
					cb = *++b;                                                          \
					brec = DeeUni_Descriptor(cb);                                       \
					if (!(brec->ut_flags & UNICODE_ISDIGIT))                            \
						break;                                                          \
					valb *= 10;                                                         \
					valb += brec->ut_digit_idx;                                         \
				}                                                                       \
				return vala < valb ? -1 : vala > valb ? 1 : 0;                          \
			}                                                                           \
			++a, --a_size;                                                              \
			++b, --b_size;                                                              \
		}                                                                               \
		return a_size ? 1 : b_size ? -1 : 0;                                            \
	}
#define DEE_PRIVATE_IF_FALSE(x)
#define DEE_PRIVATE_IF_TRUE(x) x
#undef strverscmpb
#undef strverscmpw
#undef strverscmpl
#undef strcaseverscmpb
#undef strcaseverscmpw
#undef strcaseverscmpl
#define strverscmpb     dee_strverscmpb
#define strverscmpw     dee_strverscmpw
#define strverscmpl     dee_strverscmpl
#define strcaseverscmpb dee_strcaseverscmpb
#define strcaseverscmpw dee_strcaseverscmpw
#define strcaseverscmpl dee_strcaseverscmpl
DEFINE_VERSION_COMPARE_FUNCTION(dee_strverscmpb, uint8_t, , DEE_PRIVATE_IF_FALSE)
DEFINE_VERSION_COMPARE_FUNCTION(dee_strverscmpw, uint16_t, , DEE_PRIVATE_IF_FALSE)
DEFINE_VERSION_COMPARE_FUNCTION(dee_strverscmpl, uint32_t, , DEE_PRIVATE_IF_FALSE)
DEFINE_VERSION_COMPARE_FUNCTION(dee_strcaseverscmpb, uint8_t, DeeUni_ToLower, DEE_PRIVATE_IF_TRUE)   /* TODO: case-fold */
DEFINE_VERSION_COMPARE_FUNCTION(dee_strcaseverscmpw, uint16_t, DeeUni_ToLower, DEE_PRIVATE_IF_TRUE) /* TODO: case-fold */
DEFINE_VERSION_COMPARE_FUNCTION(dee_strcaseverscmpl, uint32_t, DeeUni_ToLower, DEE_PRIVATE_IF_TRUE) /* TODO: case-fold */
#undef DEE_PRIVATE_IF_TRUE
#undef DEE_PRIVATE_IF_FALSE
#undef DEFINE_VERSION_COMPARE_FUNCTION


/* Returns a pointer into `scan_str', or NULL if no match was found. */
#define DEFINE_FIND_MATCH_FUNCTION(name, T, memmem, MEMEQ)           \
	PRIVATE T *DCALL                                                 \
	name(T *__restrict scan_str, size_t scan_size,                   \
	     T *__restrict open_str, size_t open_size,                   \
	     T *__restrict clos_str, size_t clos_size) {                 \
		size_t recursion = 0;                                        \
		if unlikely(!clos_size)                                      \
			return NULL;                                             \
		if unlikely(!open_size)                                      \
			return memmem(scan_str, scan_size, clos_str, clos_size); \
		for (;;) {                                                   \
			if (scan_size < clos_size)                               \
				return NULL;                                         \
			if (MEMEQ(scan_str, clos_str, clos_size)) {              \
				if (!recursion)                                      \
					break; /* Found it! */                           \
				--recursion;                                         \
				scan_str += clos_size;                               \
				scan_size -= clos_size;                              \
				continue;                                            \
			}                                                        \
			if (scan_size >= open_size &&                            \
			    MEMEQ(scan_str, open_str, open_size)) {              \
				++recursion;                                         \
				scan_str += open_size;                               \
				scan_size -= open_size;                              \
				continue;                                            \
			}                                                        \
			++scan_str;                                              \
			--scan_size;                                             \
		}                                                            \
		return scan_str;                                             \
	}
#define DEFINE_CASEFIND_MATCH_FUNCTION(name, T, memcasemem, MEMCASESTARTSWITH)           \
	PRIVATE T *DCALL                                                                     \
	name(T *__restrict scan_str, size_t scan_size,                                       \
	     T *__restrict open_str, size_t open_size,                                       \
	     T *__restrict clos_str, size_t clos_size,                                       \
	     size_t *p_match_length) {                                                       \
		size_t recursion = 0;                                                            \
		if unlikely(!clos_size)                                                          \
			return NULL;                                                                 \
		if unlikely(!open_size)                                                          \
			return memcasemem(scan_str, scan_size, clos_str, clos_size, p_match_length); \
		while (scan_size) {                                                              \
			size_t length;                                                               \
			length = MEMCASESTARTSWITH(scan_str, scan_size, clos_str, clos_size);        \
			if (length) {                                                                \
				if (!recursion) {                                                        \
					if (p_match_length)                                                  \
						*p_match_length = length;                                        \
					return scan_str;                                                     \
				}                                                                        \
				--recursion;                                                             \
				scan_str += length;                                                      \
				scan_size -= length;                                                     \
				continue;                                                                \
			}                                                                            \
			length = MEMCASESTARTSWITH(scan_str, scan_size, open_str, open_size);        \
			if (length) {                                                                \
				++recursion;                                                             \
				scan_str += length;                                                      \
				scan_size -= length;                                                     \
				continue;                                                                \
			}                                                                            \
			++scan_str;                                                                  \
			--scan_size;                                                                 \
		}                                                                                \
		return NULL;                                                                     \
	}
#undef find_asciicasematchb
#undef find_matchb
#undef find_matchw
#undef find_matchl
#undef find_casematchb
#undef find_casematchw
#undef find_casematchl
#define find_asciicasematchb dee_find_asciicasematchb
#define find_matchb          dee_find_matchb
#define find_matchw          dee_find_matchw
#define find_matchl          dee_find_matchl
#define find_casematchb      dee_find_casematchb
#define find_casematchw      dee_find_casematchw
#define find_casematchl      dee_find_casematchl
DEFINE_FIND_MATCH_FUNCTION(dee_find_asciicasematchb, uint8_t, memasciicasemem, memasciicaseeq)
DEFINE_FIND_MATCH_FUNCTION(dee_find_matchb, uint8_t, memmemb, MEMEQB)
DEFINE_FIND_MATCH_FUNCTION(dee_find_matchw, uint16_t, memmemw, MEMEQW)
DEFINE_FIND_MATCH_FUNCTION(dee_find_matchl, uint32_t, memmeml, MEMEQL)
DEFINE_CASEFIND_MATCH_FUNCTION(dee_find_casematchb, uint8_t, memcasememb, MEMCASESTARTSWITHB)
DEFINE_CASEFIND_MATCH_FUNCTION(dee_find_casematchw, uint16_t, memcasememw, MEMCASESTARTSWITHW)
DEFINE_CASEFIND_MATCH_FUNCTION(dee_find_casematchl, uint32_t, memcasememl, MEMCASESTARTSWITHL)
#undef DEFINE_CASEFIND_MATCH_FUNCTION
#undef DEFINE_FIND_MATCH_FUNCTION

#define DEFINE_RFIND_MATCH_FUNCTION(name, T, memrmem, MEMEQ)          \
	PRIVATE T *DCALL                                                  \
	name(T *__restrict scan_str, size_t scan_size,                    \
	     T *__restrict open_str, size_t open_size,                    \
	     T *__restrict clos_str, size_t clos_size) {                  \
		size_t recursion = 0;                                         \
		if unlikely(!open_size)                                       \
			return NULL;                                              \
		if unlikely(!clos_size)                                       \
			return memrmem(scan_str, scan_size, open_str, open_size); \
		scan_str += scan_size;                                        \
		for (;;) {                                                    \
			if (scan_size < open_size)                                \
				return NULL;                                          \
			if (MEMEQ(scan_str - open_size, open_str, open_size)) {   \
				scan_str -= open_size;                                \
				if (!recursion)                                       \
					break; /* Found it! */                            \
				--recursion;                                          \
				scan_size -= open_size;                               \
				continue;                                             \
			}                                                         \
			if (scan_size >= clos_size &&                             \
			    MEMEQ(scan_str - clos_size, clos_str, clos_size)) {   \
				++recursion;                                          \
				scan_str -= clos_size;                                \
				scan_size -= clos_size;                               \
				continue;                                             \
			}                                                         \
			--scan_str;                                               \
			--scan_size;                                              \
		}                                                             \
		return scan_str;                                              \
	}
#define DEFINE_CASERFIND_MATCH_FUNCTION(name, T, memcasermem, MEMCASEENDSWITH)            \
	PRIVATE T *DCALL                                                                      \
	name(T *__restrict scan_str, size_t scan_size,                                        \
	     T *__restrict open_str, size_t open_size,                                        \
	     T *__restrict clos_str, size_t clos_size,                                        \
	     size_t *p_match_length) {                                                        \
		size_t recursion = 0;                                                             \
		if unlikely(!open_size)                                                           \
			return NULL;                                                                  \
		if unlikely(!clos_size)                                                           \
			return memcasermem(scan_str, scan_size, open_str, open_size, p_match_length); \
		while (scan_size) {                                                               \
			size_t length;                                                                \
			length = MEMCASEENDSWITH(scan_str, scan_size, open_str, open_size);           \
			if (length) {                                                                 \
				scan_size -= length;                                                      \
				if (!recursion) {                                                         \
					if (p_match_length)                                                   \
						*p_match_length = length;                                         \
					return scan_str + scan_size;                                          \
				}                                                                         \
				--recursion;                                                              \
				continue;                                                                 \
			}                                                                             \
			length = MEMCASEENDSWITH(scan_str, scan_size, clos_str, clos_size);           \
			if (length) {                                                                 \
				++recursion;                                                              \
				scan_size -= length;                                                      \
				continue;                                                                 \
			}                                                                             \
			--scan_size;                                                                  \
		}                                                                                 \
		return NULL;                                                                      \
	}
#undef rfind_asciicasematchb
#undef rfind_matchb
#undef rfind_matchw
#undef rfind_matchl
#undef rfind_casematchb
#undef rfind_casematchw
#undef rfind_casematchl
#define rfind_asciicasematchb dee_rfind_asciicasematchb
#define rfind_matchb          dee_rfind_matchb
#define rfind_matchw          dee_rfind_matchw
#define rfind_matchl          dee_rfind_matchl
#define rfind_casematchb      dee_rfind_casematchb
#define rfind_casematchw      dee_rfind_casematchw
#define rfind_casematchl      dee_rfind_casematchl
DEFINE_RFIND_MATCH_FUNCTION(dee_rfind_asciicasematchb, uint8_t, memasciicasermem, memasciicaseeq)
DEFINE_RFIND_MATCH_FUNCTION(dee_rfind_matchb, uint8_t, memrmemb, MEMEQB)
DEFINE_RFIND_MATCH_FUNCTION(dee_rfind_matchw, uint16_t, memrmemw, MEMEQW)
DEFINE_RFIND_MATCH_FUNCTION(dee_rfind_matchl, uint32_t, memrmeml, MEMEQL)
DEFINE_CASERFIND_MATCH_FUNCTION(dee_rfind_casematchb, uint8_t, memcasermemb, MEMCASEENDSWITHB)
DEFINE_CASERFIND_MATCH_FUNCTION(dee_rfind_casematchw, uint16_t, memcasermemw, MEMCASEENDSWITHW)
DEFINE_CASERFIND_MATCH_FUNCTION(dee_rfind_casematchl, uint32_t, memcasermeml, MEMCASEENDSWITHL)
#undef DEFINE_CASERFIND_MATCH_FUNCTION
#undef DEFINE_RFIND_MATCH_FUNCTION

#undef wildcompareb
#undef wildcomparew
#undef wildcomparel
#undef wildcasecompareb
#undef wildcasecomparew
#undef wildcasecomparel
#define wildcompareb     dee_wildcompareb
#define wildcomparew     dee_wildcomparew
#define wildcomparel     dee_wildcomparel
#define wildcasecompareb dee_wildcasecompareb
#define wildcasecomparew dee_wildcasecomparew
#define wildcasecomparel dee_wildcasecomparel

/* Pull in definitions for wild-compare functions. */
#define T               uint8_t
#define dee_wildcompare dee_wildcompareb
#include "wildcompare.c.inl"
#define T               uint16_t
#define dee_wildcompare dee_wildcomparew
#include "wildcompare.c.inl"
#define Treturn         int64_t
#define T               uint32_t
#define dee_wildcompare dee_wildcomparel
#include "wildcompare.c.inl"

#define NOCASE
#define T               uint8_t
#define dee_wildcompare dee_wildasccicasecompareb
#include "wildcompare.c.inl"


#define CASEFOLD
#define T               uint8_t
#define dee_wildcompare dee_wildcasecompareb
#include "wildcompare.c.inl"
#define CASEFOLD
#define T               uint16_t
#define dee_wildcompare dee_wildcasecomparew
#include "wildcompare.c.inl"
#define CASEFOLD
#define T               uint32_t
#define dee_wildcompare dee_wildcasecomparel
#include "wildcompare.c.inl"

typedef DeeStringObject String;

#undef mempfilb
#define mempfilb dee_mempfilb
PRIVATE ATTR_OUTS(1, 2) ATTR_INS(3, 4) uint8_t *DCALL
dee_mempfilb(uint8_t *__restrict dst, size_t num_bytes,
             uint8_t const *__restrict src, size_t src_bytes) {
	ASSERT(src_bytes != 0);
	if (src_bytes == 1) {
		dst = mempsetb(dst, src[0], num_bytes);
	} else {
		while (num_bytes > src_bytes) {
			dst = mempcpyb(dst, src, src_bytes);
			num_bytes -= src_bytes;
		}
		dst = mempcpyb(dst, src, num_bytes);
	}
	return dst;
}

#undef mempfilw
#define mempfilw dee_mempfilw
PRIVATE ATTR_OUTS(1, 2) ATTR_INS(3, 4) uint16_t *DCALL
dee_mempfilw(uint16_t *__restrict dst, size_t num_words,
             uint16_t const *__restrict src, size_t src_words) {
	ASSERT(src_words != 0);
	if (src_words == 1) {
		dst = mempsetw(dst, src[0], num_words);
	} else {
		while (num_words > src_words) {
			dst = mempcpyw(dst, src, src_words);
			num_words -= src_words;
		}
		dst = mempcpyw(dst, src, num_words);
	}
	return dst;
}

#undef mempfill
#define mempfill dee_mempfill
PRIVATE ATTR_OUTS(1, 2) ATTR_INS(3, 4) uint32_t *DCALL
dee_mempfill(uint32_t *__restrict dst, size_t num_dwords,
             uint32_t const *__restrict src, size_t src_dwords) {
	ASSERT(src_dwords != 0);
	if (src_dwords == 1) {
		dst = mempsetl(dst, src[0], num_dwords);
	} else {
		while (num_dwords > src_dwords) {
			dst = mempcpyl(dst, src, src_dwords);
			num_dwords -= src_dwords;
		}
		dst = mempcpyl(dst, src, num_dwords);
	}
	return dst;
}



/* Helper macro to facilitate the logic when it comes to custom string sub-ranges.
 * @param: p_start: [1..1] Pointer to the "size_t start" given by the callers
 * @param: p_end:   [1..1] Pointer to the "size_t end" given by the callers (may get adjusted)
 * @param: p_mylen: [1..1][in]  Pointer to the *full* length of the associated string
 *                        [out] Size of the effective sub-range (OUT(*p_end) - OUT(*p_start))
 * @param: Lempty_len: Name of a label to jump to when OUT(*p_mylen) would end up 0 or negative */
#define CLAMP_SUBSTR(/*in*/ p_start, /*in|out*/ p_end,            \
                     /*in|out*/ p_mylen, Lempty_len)              \
	do {                                                          \
		if likely(*(p_end) > *(p_mylen))                          \
			*(p_end) = *(p_mylen);                                \
		if unlikely(OVERFLOW_USUB(*(p_end), *(p_start), p_mylen)) \
			goto Lempty_len;                                      \
	}	__WHILE0


DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_UNICODE_STRING_FUNCTIONS_H */
