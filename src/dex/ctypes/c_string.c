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
#ifndef GUARD_DEX_CTYPES_C_STRING_C
#define GUARD_DEX_CTYPES_C_STRING_C 1
#define DEE_SOURCE

#include "libctypes.h"
/**/

#include <deemon/api.h>

#include <deemon/arg.h>             /* DeeArg_UnpackStruct*, UNPuSIZ, _DeeArg_AsObject */
#include <deemon/error-rt.h>        /* DeeRT_ErrIntegerOverflowUMul */
#include <deemon/error.h>           /* DeeError_NOTIMPLEMENTED */
#include <deemon/int.h>             /* DeeInt_NewInt, DeeInt_NewSize, Dee_return_smallint */
#include <deemon/none.h>            /* return_none */
#include <deemon/object.h>
#include <deemon/objmethod.h>       /*  */
#include <deemon/system-features.h> /* DeeSystem_DEFINE_*, bcmp, bzero, isalpha, memchr, memcmp, memcpy, memmove, memset, stpcpy, strcat, strchr, strcpy, strend, strlen, strncat, to(lower|upper) */
#include <deemon/system.h>          /* DeeSystem_HAVE_FS_DRIVES, DeeSystem_IsSep */

#include <hybrid/overflow.h> /* OVERFLOW_UMUL */
#include <hybrid/typecore.h> /* __BYTE_TYPE__ */

#include "c_api.h" /* Prototypes */

#include <stddef.h> /* NULL, size_t */

DECL_BEGIN

#undef byte_t
#define byte_t __BYTE_TYPE__

#ifndef CONFIG_HAVE_memrchr
#define CONFIG_HAVE_memrchr
#undef memrchr
#define memrchr dee_memrchr
DeeSystem_DEFINE_memrchr(dee_memrchr)
#endif /* !CONFIG_HAVE_memrchr */

#ifndef CONFIG_HAVE_memccpy
#define CONFIG_HAVE_memccpy
#undef memccpy
#define memccpy dee_memccpy
DeeSystem_DEFINE_memccpy(dee_memccpy)
#endif /* !CONFIG_HAVE_memccpy */

#ifndef CONFIG_HAVE_rawmemchr
#define CONFIG_HAVE_rawmemchr
#undef rawmemchr
#define rawmemchr dee_rawmemchr
DeeSystem_DEFINE_rawmemchr(dee_rawmemchr)
#endif /* !CONFIG_HAVE_rawmemchr */

#ifndef CONFIG_HAVE_strnlen
#define CONFIG_HAVE_strnlen
#undef strnlen
#define strnlen dee_strnlen
DeeSystem_DEFINE_strnlen(strnlen)
#endif /* !CONFIG_HAVE_strnlen */

#ifndef CONFIG_HAVE_memmem
#define CONFIG_HAVE_memmem
#undef memmem
#define memmem dee_memmem
DeeSystem_DEFINE_memmem(dee_memmem)
#endif /* !CONFIG_HAVE_memmem */

#ifndef CONFIG_HAVE_memrmem
#define CONFIG_HAVE_memrmem
#undef memrmem
#define memrmem dee_memrmem
DeeSystem_DEFINE_memrmem(dee_memrmem)
#endif /* !CONFIG_HAVE_memrmem */

#ifndef CONFIG_HAVE_rawmemrchr
#define CONFIG_HAVE_rawmemrchr
#undef rawmemrchr
#define rawmemrchr dee_rawmemrchr
DeeSystem_DEFINE_rawmemrchr(dee_rawmemrchr)
#endif /* !CONFIG_HAVE_rawmemrchr */

#ifndef CONFIG_HAVE_memend
#define CONFIG_HAVE_memend
#undef memend
#define memend dee_memend
DeeSystem_DEFINE_memend(dee_memend)
#endif /* !CONFIG_HAVE_memend */

#ifndef CONFIG_HAVE_memxend
#define CONFIG_HAVE_memxend
#undef memxend
#define memxend dee_memxend
DeeSystem_DEFINE_memxend(dee_memxend)
#endif /* !CONFIG_HAVE_memxend */

#ifndef CONFIG_HAVE_memrend
#define CONFIG_HAVE_memrend
#undef memrend
#define memrend dee_memrend
DeeSystem_DEFINE_memrend(dee_memrend)
#endif /* !CONFIG_HAVE_memrend */

#ifndef CONFIG_HAVE_memlen
#define CONFIG_HAVE_memlen
#undef memlen
#define memlen dee_memlen
DeeSystem_DEFINE_memlen(dee_memlen)
#endif /* !CONFIG_HAVE_memlen */

#ifndef CONFIG_HAVE_memxlen
#define CONFIG_HAVE_memxlen
#undef memxlen
#define memxlen dee_memxlen
DeeSystem_DEFINE_memxlen(dee_memxlen)
#endif /* !CONFIG_HAVE_memxlen */

#ifndef CONFIG_HAVE_memrlen
#define CONFIG_HAVE_memrlen
#undef memrlen
#define memrlen dee_memrlen
DeeSystem_DEFINE_memrlen(dee_memrlen)
#endif /* !CONFIG_HAVE_memrlen */

#ifndef CONFIG_HAVE_rawmemlen
#define CONFIG_HAVE_rawmemlen
#undef rawmemlen
#define rawmemlen dee_rawmemlen
DeeSystem_DEFINE_rawmemlen(dee_rawmemlen)
#endif /* !CONFIG_HAVE_rawmemlen */

#ifndef CONFIG_HAVE_rawmemrlen
#define CONFIG_HAVE_rawmemrlen
#undef rawmemrlen
#define rawmemrlen dee_rawmemrlen
DeeSystem_DEFINE_rawmemrlen(dee_rawmemrlen)
#endif /* !CONFIG_HAVE_rawmemrlen */

#ifndef CONFIG_HAVE_memxchr
#define CONFIG_HAVE_memxchr
#undef memxchr
#define memxchr dee_memxchr
DeeSystem_DEFINE_memxchr(dee_memxchr)
#endif /* !CONFIG_HAVE_memxchr */

#ifndef CONFIG_HAVE_rawmemxchr
#define CONFIG_HAVE_rawmemxchr
#undef rawmemxchr
#define rawmemxchr dee_rawmemxchr
DeeSystem_DEFINE_rawmemxchr(dee_rawmemxchr)
#endif /* !CONFIG_HAVE_rawmemxchr */

#ifndef CONFIG_HAVE_rawmemxlen
#define CONFIG_HAVE_rawmemxlen
#undef rawmemxlen
#define rawmemxlen dee_rawmemxlen
DeeSystem_DEFINE_rawmemxlen(dee_rawmemxlen)
#endif /* !CONFIG_HAVE_rawmemxlen */

#ifndef CONFIG_HAVE_memcasecmp
#define CONFIG_HAVE_memcasecmp
#undef memcasecmp
#define memcasecmp dee_memcasecmp
DeeSystem_DEFINE_memcasecmp(dee_memcasecmp)
#endif /* !CONFIG_HAVE_memcasecmp */

#ifndef CONFIG_HAVE_memcasemem
#define CONFIG_HAVE_memcasemem
#undef memcasemem
#define memcasemem dee_memcasemem
DeeSystem_DEFINE_memcasemem(dee_memcasemem)
#endif /* !CONFIG_HAVE_memcasemem */

#ifndef CONFIG_HAVE_memrev
#define CONFIG_HAVE_memrev
#undef memrev
#define memrev dee_memrev
DeeSystem_DEFINE_memrev(dee_memrev)
#endif /* !CONFIG_HAVE_memrev */

#ifndef CONFIG_HAVE_memrxchr
#define CONFIG_HAVE_memrxchr
#undef memrxchr
#define memrxchr dee_memrxchr
DeeSystem_DEFINE_memrxchr(dee_memrxchr)
#endif /* !CONFIG_HAVE_memrxchr */

#ifndef CONFIG_HAVE_memrxend
#define CONFIG_HAVE_memrxend
#undef memrxend
#define memrxend dee_memrxend
DeeSystem_DEFINE_memrxend(dee_memrxend)
#endif /* !CONFIG_HAVE_memrxend */

#ifndef CONFIG_HAVE_memrxlen
#define CONFIG_HAVE_memrxlen
#undef memrxlen
#define memrxlen dee_memrxlen
DeeSystem_DEFINE_memrxlen(dee_memrxlen)
#endif /* !CONFIG_HAVE_memrxlen */

#ifndef CONFIG_HAVE_rawmemrxchr
#define CONFIG_HAVE_rawmemrxchr
#undef rawmemrxchr
#define rawmemrxchr dee_rawmemrxchr
DeeSystem_DEFINE_rawmemrxchr(dee_rawmemrxchr)
#endif /* !CONFIG_HAVE_rawmemrxchr */

#ifndef CONFIG_HAVE_rawmemrxlen
#define CONFIG_HAVE_rawmemrxlen
#undef rawmemrxlen
#define rawmemrxlen dee_rawmemrxlen
DeeSystem_DEFINE_rawmemrxlen(dee_rawmemrxlen)
#endif /* !CONFIG_HAVE_rawmemrxlen */

#ifndef CONFIG_HAVE_memcasermem
#define CONFIG_HAVE_memcasermem
#undef memcasermem
#define memcasermem dee_memcasermem
DeeSystem_DEFINE_memcasermem(dee_memcasermem)
#endif /* !CONFIG_HAVE_memcasermem */

#ifndef CONFIG_HAVE_strcmp
#define CONFIG_HAVE_strcmp
#undef strcmp
#define strcmp dee_strcmp
DeeSystem_DEFINE_strcmp(dee_strcmp)
#endif /* !CONFIG_HAVE_strcmp */

#ifndef CONFIG_HAVE_strncmp
#define CONFIG_HAVE_strncmp
#undef strncmp
#define strncmp dee_strncmp
DeeSystem_DEFINE_strncmp(dee_strncmp)
#endif /* !CONFIG_HAVE_strncmp */

#ifndef CONFIG_HAVE_strcasecmp
#define CONFIG_HAVE_strcasecmp
#undef strcasecmp
#define strcasecmp dee_strcasecmp
DeeSystem_DEFINE_strcasecmp(dee_strcasecmp)
#endif /* !CONFIG_HAVE_strcasecmp */

#ifndef CONFIG_HAVE_strncasecmp
#define CONFIG_HAVE_strncasecmp
#undef strncasecmp
#define strncasecmp dee_strncasecmp
DeeSystem_DEFINE_strncasecmp(dee_strncasecmp)
#endif /* !CONFIG_HAVE_strncasecmp */

#ifndef CONFIG_HAVE_stpncpy
#define CONFIG_HAVE_stpncpy
#undef stpncpy
#define stpncpy dee_stpncpy
DeeSystem_DEFINE_stpncpy(dee_stpncpy)
#endif /* !CONFIG_HAVE_stpncpy */

#ifndef CONFIG_HAVE_strncpy
#define CONFIG_HAVE_strncpy
#undef strncpy
#define strncpy dee_strncpy
DeeSystem_DEFINE_strncpy(dee_strncpy)
#endif /* !CONFIG_HAVE_strncpy */

#ifndef CONFIG_HAVE_strnend
#define CONFIG_HAVE_strnend
#undef strnend
#define strnend(x, maxlen) ((x) + strnlen(x, maxlen))
#endif /* !CONFIG_HAVE_strnend */

#ifndef CONFIG_HAVE_strrchr
#define CONFIG_HAVE_strrchr
#undef strrchr
#define strrchr dee_strrchr
DeeSystem_DEFINE_strrchr(dee_strrchr)
#endif /* !CONFIG_HAVE_strrchr */

#ifndef CONFIG_HAVE_strnchr
#define CONFIG_HAVE_strnchr
#undef strnchr
#define strnchr dee_strnchr
LOCAL WUNUSED NONNULL((1)) char *
dee_strnchr(char const *haystack, int needle, size_t maxlen) {
	char *result = NULL;
	for (; maxlen--; ++haystack) {
		char ch = *haystack;
		if (ch == (char)(unsigned char)needle) {
			result = (char *)haystack;
			break;
		}
		if (!ch)
			break;
	}
	return result;
}
#endif /* !CONFIG_HAVE_strnchr */

#ifndef CONFIG_HAVE_strnrchr
#define CONFIG_HAVE_strnrchr
#undef strnrchr
#define strnrchr dee_strnrchr
LOCAL WUNUSED NONNULL((1)) char *
dee_strnrchr(char const *haystack, int needle, size_t maxlen) {
	char *result = NULL;
	for (; maxlen--; ++haystack) {
		char ch = *haystack;
		if (ch == (char)(unsigned char)needle)
			result = (char *)haystack;
		if (!ch)
			break;
	}
	return result;
}
#endif /* !CONFIG_HAVE_strnrchr */

#ifndef CONFIG_HAVE_strchrnul
#define CONFIG_HAVE_strchrnul
#undef strchrnul
#define strchrnul dee_strchrnul
LOCAL ATTR_RETNONNULL WUNUSED NONNULL((1)) char *
dee_strchrnul(char const *haystack, int needle) {
	for (; *haystack; ++haystack) {
		if ((unsigned char)*haystack == (unsigned char)needle)
			break;
	}
	return (char *)haystack;
}
#endif /* !CONFIG_HAVE_strchrnul */

#ifndef CONFIG_HAVE_strrchrnul
#define CONFIG_HAVE_strrchrnul
#undef strrchrnul
#define strrchrnul dee_strrchrnul
LOCAL ATTR_RETNONNULL WUNUSED NONNULL((1)) char *
dee_strrchrnul(char const *haystack, int needle) {
	char const *result = haystack - 1;
	do {
		if unlikely((unsigned char)*haystack == (unsigned char)needle)
			result = haystack;
	} while (*haystack++);
	return (char *)result;
}
#endif /* !CONFIG_HAVE_strrchrnul */

#ifndef CONFIG_HAVE_strnchrnul
#define CONFIG_HAVE_strnchrnul
#undef strnchrnul
#define strnchrnul dee_strnchrnul
LOCAL WUNUSED NONNULL((1)) char *
dee_strnchrnul(char const *haystack, int needle, size_t maxlen) {
	while (maxlen-- && *haystack && (unsigned char)*haystack != (unsigned char)needle)
		++haystack;
	return (char *)haystack;
}
#endif /* !CONFIG_HAVE_strnchrnul */

#ifndef CONFIG_HAVE_strnrchrnul
#define CONFIG_HAVE_strnrchrnul
#undef strnrchrnul
#define strnrchrnul dee_strnrchrnul
LOCAL WUNUSED NONNULL((1)) char *
dee_strnrchrnul(char const *haystack, int needle, size_t maxlen) {
	char const *result = haystack - 1;
	for (; maxlen-- && *haystack; ++haystack) {
		if unlikely((unsigned char)*haystack == (unsigned char)needle)
			result = haystack;
	}
	return (char *)result;
}
#endif /* !CONFIG_HAVE_strnrchrnul */

#ifndef CONFIG_HAVE_strstr
#define CONFIG_HAVE_strstr
#undef strstr
#define strstr dee_strstr
LOCAL WUNUSED NONNULL((1, 2)) char *
dee_strstr(char const *haystack, char const *needle) {
	char ch, needle_start = *needle++;
	while ((ch = *haystack++) != '\0') {
		if (ch == needle_start) {
			char const *hay2, *ned_iter;
			hay2     = haystack;
			ned_iter = needle;
			while ((ch = *ned_iter++) != '\0') {
				if (*hay2++ != ch)
					goto miss;
			}
			return (char *)haystack - 1;
		}
miss:
		;
	}
	return NULL;
}
#endif /* !CONFIG_HAVE_strstr */

#ifndef CONFIG_HAVE_strcasestr
#define CONFIG_HAVE_strcasestr
#undef strcasestr
#define strcasestr dee_strcasestr
LOCAL WUNUSED NONNULL((1, 2)) char *
dee_strcasestr(char const *haystack, char const *needle) {
	char ch, needle_start = *needle++;
	needle_start = (char)tolower((unsigned char)needle_start);
	while ((ch = *haystack++) != '\0') {
		if (ch == needle_start || (char)tolower((unsigned char)ch) == needle_start) {
			char const *hay2, *ned_iter;
			hay2     = haystack;
			ned_iter = needle;
			while ((ch = *ned_iter++) != '\0') {
				char char2_ch = *hay2++;
				if (char2_ch != ch && (char)tolower((unsigned char)char2_ch) != (char)tolower((unsigned char)ch))
					goto miss;
			}
			return (char *)haystack - 1;
		}
miss:
		;
	}
	return NULL;
}
#endif /* !CONFIG_HAVE_strcasestr */

#ifndef CONFIG_HAVE_strnstr
#define CONFIG_HAVE_strnstr
#undef strnstr
#define strnstr dee_strnstr
LOCAL WUNUSED NONNULL((1, 2)) char *
dee_strnstr(char const *haystack, char const *needle, size_t haystack_maxlen) {
	char ch, needle_start = *needle++;
	while (haystack_maxlen-- && (ch = *haystack++) != '\0') {
		if (ch == needle_start) {
			char const *hay2, *ned_iter;
			size_t maxlen2;
			hay2     = haystack;
			ned_iter = needle;
			maxlen2  = haystack_maxlen;
			while ((ch = *ned_iter++) != '\0') {
				if (!maxlen2-- || *hay2++ != ch)
					goto miss;
			}
			return (char *)haystack - 1;
		}
miss:
		;
	}
	return NULL;
}
#endif /* !CONFIG_HAVE_strnstr */

#ifndef CONFIG_HAVE_strncasestr
#define CONFIG_HAVE_strncasestr
#undef strncasestr
#define strncasestr dee_strncasestr
LOCAL WUNUSED NONNULL((1, 2)) char *
dee_strncasestr(char const *haystack, char const *needle, size_t haystack_maxlen) {
	char ch, needle_start = *needle++;
	needle_start = (char)tolower((unsigned char)needle_start);
	while (haystack_maxlen-- && (ch = *haystack++) != '\0') {
		if (ch == needle_start || (char)tolower((unsigned char)ch) == needle_start) {
			char const *hay2, *ned_iter;
			size_t maxlen2;
			hay2     = haystack;
			ned_iter = needle;
			maxlen2  = haystack_maxlen;
			while ((ch = *ned_iter++) != '\0') {
				char hay2_ch;
				if (!maxlen2--)
					goto miss;
				hay2_ch = *hay2++;
				if (hay2_ch != ch && (char)tolower((unsigned char)hay2_ch) != (char)tolower((unsigned char)ch))
					goto miss;
			}
			return (char *)haystack - 1;
		}
miss:
		;
	}
	return NULL;
}
#endif /* !CONFIG_HAVE_strncasestr */

#ifndef CONFIG_HAVE_strverscmp
#define CONFIG_HAVE_strverscmp
#undef strverscmp
#define strverscmp dee_strverscmp
LOCAL WUNUSED NONNULL((1, 2)) int
dee_strverscmp(char const *s1, char const *s2) {
	char const *s1_start = s1;
	char c1, c2;
	do {
		if ((c1 = *s1) != (c2 = *s2)) {
			unsigned int vala, valb;

			/* Unwind common digits. */
			while (s1 != s1_start) {
				if (s1[-1] < '0' || s1[-1] > '9')
					break;
				c2 = c1 = *--s1, --s2;
			}

			/* Check if both strings have digit sequences in the same places. */
			if ((c1 < '0' || c1 > '9') &&
			    (c2 < '0' || c2 > '9'))
				return (int)((unsigned char)c1 - (unsigned char)c2);

			/* Deal with leading zeros. */
			if (c1 == '0')
				return -1;
			if (c2 == '0')
				return 1;

			/* Compare digits. */
			vala = c1 - '0';
			valb = c2 - '0';
			for (;;) {
				c1 = *s1++;
				if (c1 < '0' || c1 > '9')
					break;
				vala *= 10;
				vala += c1 - '0';
			}
			for (;;) {
				c2 = *s2++;
				if (c2 < '0' || c2 > '9')
					break;
				valb *= 10;
				valb += c2 - '0';
			}

			/* Return difference between digits. */
			return (int)vala - (int)valb;
		}
		++s1;
		++s2;
	} while (c1 != '\0');
	return 0;
}
#endif /* !CONFIG_HAVE_strverscmp */

#ifndef CONFIG_HAVE_basename
#define CONFIG_HAVE_basename
#undef basename
#define basename dee_basename
LOCAL ATTR_RETNONNULL WUNUSED NONNULL((1)) char *
dee_basename(char const *filename) {
	/* >> char *slash = strrchr(filename, '/');
	 * >> return slash ? slash + 1 : (char *)filename; */
	char *result, *iter = (char *)filename;
#ifdef DeeSystem_HAVE_FS_DRIVES
	/* Skip drive letter. */
	if (isalpha(iter[0]) && iter[1] == ':')
		iter += 2;
#endif /* DeeSystem_HAVE_FS_DRIVES */
	result = iter;
	for (;;) {
		char ch = *iter++;
		if (DeeSystem_IsSep(ch))
			result = iter;
		if (!ch)
			break;
	}
	return result;
}
#endif /* !CONFIG_HAVE_basename */

#ifndef CONFIG_HAVE_strspn
#define CONFIG_HAVE_strspn
#undef strspn
#define strspn dee_strspn
LOCAL ATTR_PURE WUNUSED NONNULL((1, 2)) size_t
dee_strspn(char const *haystack, char const *accept) {
	char const *iter = haystack;
	while (*iter && strchr(accept, *iter))
		++iter;
	return (size_t)(iter - haystack);
}
#endif /* !CONFIG_HAVE_strspn */

#ifndef CONFIG_HAVE_strcspn
#define CONFIG_HAVE_strcspn
#undef strcspn
#define strcspn dee_strcspn
LOCAL ATTR_PURE WUNUSED NONNULL((1, 2)) size_t
dee_strcspn(char const *haystack, char const *accept) {
	char const *iter = haystack;
	while (*iter && !strchr(accept, *iter))
		++iter;
	return (size_t)(iter - haystack);
}
#endif /* !CONFIG_HAVE_strcspn */

#ifndef CONFIG_HAVE_strpbrk
#define CONFIG_HAVE_strpbrk
#undef strpbrk
#define strpbrk dee_strpbrk
LOCAL ATTR_PURE WUNUSED NONNULL((1, 2)) char const *
dee_strpbrk(char const *haystack, char const *accept) {
	return haystack + strcspn(haystack, accept);
}
#endif /* !CONFIG_HAVE_strpbrk */

#ifndef CONFIG_HAVE_strrev
#define CONFIG_HAVE_strrev
#undef strrev
#define strrev dee_strrev
LOCAL ATTR_PURE NONNULL((1)) char *
dee_strrev(char *str) {
	return (char *)memrev(str, strlen(str) * sizeof(char));
}
#endif /* !CONFIG_HAVE_strrev */

#ifndef CONFIG_HAVE_strnrev
#define CONFIG_HAVE_strnrev
#undef strnrev
#define strnrev dee_strnrev
LOCAL ATTR_PURE NONNULL((1)) char *
dee_strnrev(char *str, size_t maxlen) {
	return (char *)memrev(str, strnlen(str, maxlen) * sizeof(char));
}
#endif /* !CONFIG_HAVE_strnrev */

#ifndef CONFIG_HAVE_strlwr
#define CONFIG_HAVE_strlwr
#undef strlwr
#define strlwr dee_strlwr
LOCAL ATTR_PURE NONNULL((1)) char *
dee_strlwr(char *str) {
	char *iter, ch;
	for (iter = str; (ch = *iter) != '\0'; ++iter)
		*iter = (char)tolower((unsigned char)ch);
	return str;
}
#endif /* !CONFIG_HAVE_strlwr */

#ifndef CONFIG_HAVE_strnlwr
#define CONFIG_HAVE_strnlwr
#undef strnlwr
#define strnlwr dee_strnlwr
LOCAL ATTR_PURE NONNULL((1)) char *
dee_strnlwr(char *str, size_t maxlen) {
	char *iter, ch;
	for (iter = str; maxlen-- && (ch = *iter) != '\0'; ++iter)
		*iter = (char)tolower((unsigned char)ch);
	return str;
}
#endif /* !CONFIG_HAVE_strnlwr */

#ifndef CONFIG_HAVE_strupr
#define CONFIG_HAVE_strupr
#undef strupr
#define strupr dee_strupr
LOCAL ATTR_PURE NONNULL((1)) char *
dee_strupr(char *str) {
	char *iter, ch;
	for (iter = str; (ch = *iter) != '\0'; ++iter)
		*iter = (char)toupper((unsigned char)ch);
	return str;
}
#endif /* !CONFIG_HAVE_strupr */

#ifndef CONFIG_HAVE_strnupr
#define CONFIG_HAVE_strnupr
#undef strnupr
#define strnupr dee_strnupr
LOCAL ATTR_PURE NONNULL((1)) char *
dee_strnupr(char *str, size_t maxlen) {
	char *iter, ch;
	for (iter = str; maxlen-- && (ch = *iter) != '\0'; ++iter)
		*iter = (char)toupper((unsigned char)ch);
	return str;
}
#endif /* !CONFIG_HAVE_strnupr */

#ifndef CONFIG_HAVE_strset
#define CONFIG_HAVE_strset
#undef strset
#define strset dee_strset
LOCAL ATTR_PURE NONNULL((1)) char *
dee_strset(char *str, int ch) {
	char *iter;
	for (iter = str; *iter; ++iter)
		*iter = (char)(unsigned int)ch;
	return str;
}
#endif /* !CONFIG_HAVE_strset */

#ifndef CONFIG_HAVE_strnset
#define CONFIG_HAVE_strnset
#undef strnset
#define strnset dee_strnset
LOCAL ATTR_PURE NONNULL((1)) char *
dee_strnset(char *str, int ch, size_t maxlen) {
	char *iter;
	for (iter = str; maxlen-- && *iter; ++iter)
		*iter = (char)(unsigned int)ch;
	return str;
}
#endif /* !CONFIG_HAVE_strnset */

#ifndef CONFIG_HAVE_memfrob
#define CONFIG_HAVE_memfrob
#undef memfrob
#define memfrob dee_memfrob
LOCAL ATTR_PURE NONNULL((1)) void *
dee_memfrob(void *buf, size_t num_bytes) {
	byte_t *iter = (byte_t *)buf;
	while (num_bytes--)
		*iter++ ^= 42; /* -_-   yeah... */
	return buf;
}
#endif /* !CONFIG_HAVE_memfrob */

#ifndef CONFIG_HAVE_strsep
#define CONFIG_HAVE_strsep
#undef strsep
#define strsep dee_strsep
LOCAL ATTR_PURE NONNULL((1, 2)) char *
dee_strsep(char **stringp, char const *delim) {
	char *result, *iter;
	if ((result = *stringp) == NULL || !*result)
		return NULL;
	for (iter = result; *iter && !strchr(delim, *iter); ++iter)
		;
	if (*iter)
		*iter++ = '\0';
	*stringp = iter;
	return result;
}
#endif /* !CONFIG_HAVE_strsep */

#ifndef CONFIG_HAVE_stresep
#define CONFIG_HAVE_stresep
#undef stresep
#define stresep dee_stresep
LOCAL ATTR_PURE NONNULL((1, 2)) char *
dee_stresep(char **stringp, char const *delim, int escape) {
	char *result, *iter;
	if ((result = *stringp) == NULL || !*result)
		return NULL;
	for (iter = result;; ++iter) {
		char ch = *iter;
		if (!ch)
			break;
		if ((int)(unsigned int)(unsigned char)ch == escape) {
			/* Escape the next character. */
			ch = *++iter;
			if (!ch)
				break;
		}
		if (strchr(delim, ch))
			break;
	}
	if (*iter)
		*iter++ = '\0';
	*stringp = iter;
	return result;
}
#endif /* !CONFIG_HAVE_stresep */

#ifndef CONFIG_HAVE_strtok_r
#define CONFIG_HAVE_strtok_r
#undef strtok_r
#define strtok_r dee_strtok_r
LOCAL ATTR_PURE NONNULL((2, 3)) char *
dee_strtok_r(char *str, char const *delim, char **save_ptr) {
	char *end;
	if (!str)
		str = *save_ptr;
	if (!*str) {
		*save_ptr = str;
		return NULL;
	}
	str += strspn(str, delim);
	if (!*str) {
		*save_ptr = str;
		return NULL;
	}
	end = str + strcspn(str, delim);
	if (!*end) {
		*save_ptr = end;
		return str;
	}
	*end = '\0';
	*save_ptr = end + 1;
	return str;
}
#endif /* !CONFIG_HAVE_strtok_r */

#ifndef CONFIG_HAVE_strtok
#define CONFIG_HAVE_strtok
#undef strtok
#define strtok dee_strtok
LOCAL ATTR_PURE NONNULL((1, 2)) char *
dee_strtok(char *str, char const *delim) {
	static char *save_ptr = NULL;
	return strtok_r(str, delim, &save_ptr);
}
#endif /* !CONFIG_HAVE_strtok */





/*[[[deemon
import * from rt.gen.unpack;
import * from deemon;
function def(spec: string) {
	local name, none, params = spec.partition("(")...;
	local params, none, impl = params.partition(")")...;
	name = name.strip();
	params = params.strip();
	impl = impl.strip();
	local tags = Dict();
	while (name.startswith("@")) {
		local tagStart = 1;
		local tagEnd = tagStart;
		while (tagEnd < #name && name.issymcont(tagEnd))
			++tagEnd;
		local tag = name[tagStart:tagEnd];
		tags[tag] = true;
		name = name[tagEnd:].lstrip();
	}

	print("/" "* ", name, " *" "/");
	tags["visi"] = "INTERN";
	print_CMethod(name, params, **tags);
	print impl;
	print;
}

function defs(specs: string) {
	local start = 0;
	for (;;) {
		local implStart = specs.find("{", start);
		if (implStart < 0)
			break;
		local implEnd = specs.findmatch("{", "}", implStart + 1);
		if (implEnd < 0)
			break;
		++implEnd;
		def(specs[start:implEnd]);
		start = implEnd;
	}
}



defs("""

memcpy(dst:ctypes:void*, src:ctypes:void_const*,
       size_t num_bytes, size_t elem_count = 1) {
	size_t total;
	if (OVERFLOW_UMUL(num_bytes, elem_count, &total))
		goto err_overflow;
	CTYPES_FAULTPROTECT(dst = memcpy(dst, src, total), return NULL);
	return DeePointer_NewVoid(dst);
err_overflow:
	DeeRT_ErrIntegerOverflowUMul(num_bytes, elem_count);
	return NULL;
}

mempcpy(dst:ctypes:void*, src:ctypes:void_const*,
        size_t num_bytes, size_t elem_count = 1) {
	size_t total;
	if (OVERFLOW_UMUL(num_bytes, elem_count, &total))
		goto err_overflow;
	CTYPES_FAULTPROTECT(dst = memcpy(dst, src, total), return NULL);
	return DeePointer_NewVoid((byte_t *)dst + total);
err_overflow:
	DeeRT_ErrIntegerOverflowUMul(num_bytes, elem_count);
	return NULL;
}

memmove(dst:ctypes:void*, src:ctypes:void_const*,
       size_t num_bytes, size_t elem_count = 1) {
	size_t total;
	if (OVERFLOW_UMUL(num_bytes, elem_count, &total))
		goto err_overflow;
	CTYPES_FAULTPROTECT(dst = memmove(dst, src, total), return NULL);
	return DeePointer_NewVoid(dst);
err_overflow:
	DeeRT_ErrIntegerOverflowUMul(num_bytes, elem_count);
	return NULL;
}

mempmove(dst:ctypes:void*, src:ctypes:void_const*,
        size_t num_bytes, size_t elem_count = 1) {
	size_t total;
	if (OVERFLOW_UMUL(num_bytes, elem_count, &total))
		goto err_overflow;
	CTYPES_FAULTPROTECT(dst = memmove(dst, src, total), return NULL);
	return DeePointer_NewVoid((byte_t *)dst + total);
err_overflow:
	DeeRT_ErrIntegerOverflowUMul(num_bytes, elem_count);
	return NULL;
}

memccpy(dst:ctypes:void*, src:ctypes:void_const*, int needle, size_t num_bytes) {
	CTYPES_FAULTPROTECT(dst = memccpy(dst, src, needle, num_bytes), return NULL);
	return DeePointer_NewVoid(dst);
}

memset(dst:ctypes:void*, int byte, size_t num_bytes) {
	CTYPES_FAULTPROTECT(dst = memset(dst, byte, num_bytes), return NULL);
	return DeePointer_NewVoid(dst);
}

mempset(dst:ctypes:void*, int byte, size_t num_bytes) {
	CTYPES_FAULTPROTECT(dst = memset(dst, byte, num_bytes), return NULL);
	return DeePointer_NewVoid((byte_t *)dst + num_bytes);
}

bzero(dst:ctypes:void*, size_t num_bytes, size_t elem_count = 1) {
	size_t total;
	if (OVERFLOW_UMUL(num_bytes, elem_count, &total))
		goto err_overflow;
	CTYPES_FAULTPROTECT(bzero(dst, total), return NULL);
	return_none;
err_overflow:
	DeeRT_ErrIntegerOverflowUMul(num_bytes, elem_count);
	return NULL;
}

memchr(haystack:ctypes:void*, int needle, size_t num_bytes) {
	void *result;
	CTYPES_FAULTPROTECT(result = memchr(haystack, needle, num_bytes), return NULL);
	return DeePointer_NewVoid(result);
}

memrchr(haystack:ctypes:void*, int needle, size_t num_bytes) {
	void *result;
	CTYPES_FAULTPROTECT(result = memrchr(haystack, needle, num_bytes), return NULL);
	return DeePointer_NewVoid(result);
}

memend(haystack:ctypes:void*, int needle, size_t num_bytes) {
	void *result;
	CTYPES_FAULTPROTECT(result = memend(haystack, needle, num_bytes), return NULL);
	return DeePointer_NewVoid(result);
}

memrend(haystack:ctypes:void*, int needle, size_t num_bytes) {
	void *result;
	CTYPES_FAULTPROTECT(result = memrend(haystack, needle, num_bytes), return NULL);
	return DeePointer_NewVoid(result);
}

memlen(haystack:ctypes:void*, int needle, size_t num_bytes) {
	size_t result;
	CTYPES_FAULTPROTECT(result = memlen(haystack, needle, num_bytes), return NULL);
	return DeeInt_NewSize(result);
}

memrlen(haystack:ctypes:void*, int needle, size_t num_bytes) {
	size_t result;
	CTYPES_FAULTPROTECT(result = memrlen(haystack, needle, num_bytes), return NULL);
	return DeeInt_NewSize(result);
}

rawmemchr(haystack:ctypes:void*, int needle) {
	void *result;
	CTYPES_FAULTPROTECT(result = rawmemchr(haystack, needle), return NULL);
	return DeePointer_NewVoid(result);
}

rawmemrchr(haystack:ctypes:void*, int needle) {
	void *result;
	CTYPES_FAULTPROTECT(result = rawmemrchr(haystack, needle), return NULL);
	return DeePointer_NewVoid(result);
}

rawmemlen(haystack:ctypes:void*, int needle) {
	size_t result;
	CTYPES_FAULTPROTECT(result = rawmemlen(haystack, needle), return NULL);
	return DeeInt_NewSize(result);
}

rawmemrlen(haystack:ctypes:void*, int needle) {
	size_t result;
	CTYPES_FAULTPROTECT(result = rawmemrlen(haystack, needle), return NULL);
	return DeeInt_NewSize(result);
}

memxchr(haystack:ctypes:void*, int needle, size_t num_bytes) {
	void *result;
	CTYPES_FAULTPROTECT(result = memxchr(haystack, needle, num_bytes), return NULL);
	return DeePointer_NewVoid(result);
}

memrxchr(haystack:ctypes:void*, int needle, size_t num_bytes) {
	void *result;
	CTYPES_FAULTPROTECT(result = memrxchr(haystack, needle, num_bytes), return NULL);
	return DeePointer_NewVoid(result);
}

memxend(haystack:ctypes:void*, int needle, size_t num_bytes) {
	void *result;
	CTYPES_FAULTPROTECT(result = memxend(haystack, needle, num_bytes), return NULL);
	return DeePointer_NewVoid(result);
}

memrxend(haystack:ctypes:void*, int needle, size_t num_bytes) {
	void *result;
	CTYPES_FAULTPROTECT(result = memrxend(haystack, needle, num_bytes), return NULL);
	return DeePointer_NewVoid(result);
}

memxlen(haystack:ctypes:void*, int needle, size_t num_bytes) {
	size_t result;
	CTYPES_FAULTPROTECT(result = memxlen(haystack, needle, num_bytes), return NULL);
	return DeeInt_NewSize(result);
}

memrxlen(haystack:ctypes:void*, int needle, size_t num_bytes) {
	size_t result;
	CTYPES_FAULTPROTECT(result = memrxlen(haystack, needle, num_bytes), return NULL);
	return DeeInt_NewSize(result);
}

rawmemxchr(haystack:ctypes:void*, int needle) {
	void *result;
	CTYPES_FAULTPROTECT(result = rawmemxchr(haystack, needle), return NULL);
	return DeePointer_NewVoid(result);
}

rawmemrxchr(haystack:ctypes:void*, int needle) {
	void *result;
	CTYPES_FAULTPROTECT(result = rawmemrxchr(haystack, needle), return NULL);
	return DeePointer_NewVoid(result);
}

rawmemxlen(haystack:ctypes:void*, int needle) {
	size_t result;
	CTYPES_FAULTPROTECT(result = rawmemxlen(haystack, needle), return NULL);
	return DeeInt_NewSize(result);
}

rawmemrxlen(haystack:ctypes:void*, int needle) {
	size_t result;
	CTYPES_FAULTPROTECT(result = rawmemrxlen(haystack, needle), return NULL);
	return DeeInt_NewSize(result);
}

@ispure
bcmp(a:ctypes:void_const*, b:ctypes:void_const*, size_t num_bytes) {
	int result;
	CTYPES_FAULTPROTECT(result = bcmp(a, b, num_bytes), return NULL);
	Dee_return_smallint(result ? 1 : 0);
}

@ispure
memcmp(lhs:ctypes:void_const*, rhs:ctypes:void_const*, size_t num_bytes) {
	int result;
	CTYPES_FAULTPROTECT(result = memcmp(lhs, rhs, num_bytes), return NULL);
	if (result < -1)
		result = -1;
	if (result > 1)
		result = 1;
	Dee_return_smallint(result);
}

@ispure
memcasecmp(lhs:ctypes:void_const*, rhs:ctypes:void_const*, size_t num_bytes) {
	int result;
	CTYPES_FAULTPROTECT(result = memcasecmp(lhs, rhs, num_bytes), return NULL);
	return DeeInt_NewInt(result);
}

@ispure
memmem(haystack:ctypes:void_const*, size_t haystack_len,
       needle:ctypes:void_const*, size_t needle_len) {
	void *result;
	CTYPES_FAULTPROTECT(result = memmem(haystack, haystack_len,
	                                    needle, needle_len),
	                    return NULL);
	return DeePointer_NewVoid(result);
}

@ispure
memcasemem(haystack:ctypes:void_const*, size_t haystack_len,
       needle:ctypes:void_const*, size_t needle_len) {
	void *result;
	CTYPES_FAULTPROTECT(result = memcasemem(haystack, haystack_len,
	                                        needle, needle_len),
	                    return NULL);
	return DeePointer_NewVoid(result);
}

@ispure
memrmem(haystack:ctypes:void_const*, size_t haystack_len,
        needle:ctypes:void_const*, size_t needle_len) {
	void *result;
	CTYPES_FAULTPROTECT(result = memrmem(haystack, haystack_len,
	                                     needle, needle_len),
	                    return NULL);
	return DeePointer_NewVoid(result);
}

@ispure
memcasermem(haystack:ctypes:void_const*, size_t haystack_len,
            needle:ctypes:void_const*, size_t needle_len) {
	void *result;
	CTYPES_FAULTPROTECT(result = memcasermem(haystack, haystack_len,
	                                         needle, needle_len),
	                    return NULL);
	return DeePointer_NewVoid(result);
}

memrev(dst:ctypes:void*, size_t num_bytes) {
	CTYPES_FAULTPROTECT(dst = memrev(dst, num_bytes), return NULL);
	return DeePointer_NewVoid(dst);
}

@ispure
strlen(string:ctypes:char_const*) {
	size_t result;
	CTYPES_FAULTPROTECT(result = strlen(string), return NULL);
	return DeeInt_NewSize(result);
}

@ispure
strend(string:ctypes:char_const*) {
	char const *result;
	CTYPES_FAULTPROTECT(result = strend(string), return NULL);
	return DeePointer_NewFor(&DeeCChar_Type, (void *)result);
}

@ispure
strnlen(string:ctypes:char_const*, size_t maxlen) {
	size_t result;
	CTYPES_FAULTPROTECT(result = strnlen(string, maxlen), return NULL);
	return DeeInt_NewSize(result);
}

@ispure
strnend(string:ctypes:char_const*, size_t maxlen) {
	char const *result;
	CTYPES_FAULTPROTECT(result = strnend(string, maxlen), return NULL);
	return DeePointer_NewFor(&DeeCChar_Type, (void *)result);
}

@ispure
strchr(haystack:ctypes:char_const*, int needle) {
	char const *result;
	CTYPES_FAULTPROTECT(result = strchr(haystack, needle), return NULL);
	return DeePointer_NewFor(&DeeCChar_Type, (void *)result);
}

@ispure
strrchr(haystack:ctypes:char_const*, int needle) {
	char const *result;
	CTYPES_FAULTPROTECT(result = strrchr(haystack, needle), return NULL);
	return DeePointer_NewFor(&DeeCChar_Type, (void *)result);
}

@ispure
strnchr(haystack:ctypes:char_const*, int needle, size_t maxlen) {
	char const *result;
	CTYPES_FAULTPROTECT(result = strnchr(haystack, needle, maxlen), return NULL);
	return DeePointer_NewFor(&DeeCChar_Type, (void *)result);
}

@ispure
strnrchr(haystack:ctypes:char_const*, int needle, size_t maxlen) {
	char const *result;
	CTYPES_FAULTPROTECT(result = strnrchr(haystack, needle, maxlen), return NULL);
	return DeePointer_NewFor(&DeeCChar_Type, (void *)result);
}

@ispure
strchrnul(haystack:ctypes:char_const*, int needle) {
	char const *result;
	CTYPES_FAULTPROTECT(result = strchrnul(haystack, needle), return NULL);
	return DeePointer_NewFor(&DeeCChar_Type, (void *)result);
}

@ispure
strrchrnul(haystack:ctypes:char_const*, int needle) {
	char const *result;
	CTYPES_FAULTPROTECT(result = strrchrnul(haystack, needle), return NULL);
	return DeePointer_NewFor(&DeeCChar_Type, (void *)result);
}

@ispure
strnchrnul(haystack:ctypes:char_const*, int needle, size_t maxlen) {
	char const *result;
	CTYPES_FAULTPROTECT(result = strnchrnul(haystack, needle, maxlen), return NULL);
	return DeePointer_NewFor(&DeeCChar_Type, (void *)result);
}

@ispure
strnrchrnul(haystack:ctypes:char_const*, int needle, size_t maxlen) {
	char const *result;
	CTYPES_FAULTPROTECT(result = strnrchrnul(haystack, needle, maxlen), return NULL);
	return DeePointer_NewFor(&DeeCChar_Type, (void *)result);
}

@ispure
stroff(haystack:ctypes:char_const*, int needle) {
	char const *result;
	CTYPES_FAULTPROTECT(result = strchrnul(haystack, needle), return NULL);
	return DeeInt_NewSize((size_t)(result - haystack));
}

@ispure
strroff(haystack:ctypes:char_const*, int needle) {
	char const *result;
	CTYPES_FAULTPROTECT(result = strrchrnul(haystack, needle), return NULL);
	return DeeInt_NewSize((size_t)(result - haystack));
}

@ispure
strnoff(haystack:ctypes:char_const*, int needle, size_t maxlen) {
	char const *result;
	CTYPES_FAULTPROTECT(result = strnchrnul(haystack, needle, maxlen), return NULL);
	return DeeInt_NewSize((size_t)(result - haystack));
}

@ispure
strnroff(haystack:ctypes:char_const*, int needle, size_t maxlen) {
	char const *result;
	CTYPES_FAULTPROTECT(result = strnrchrnul(haystack, needle, maxlen), return NULL);
	return DeeInt_NewSize((size_t)(result - haystack));
}

@ispure
strcmp(lhs:ctypes:char_const*, rhs:ctypes:char_const*) {
	int result;
	CTYPES_FAULTPROTECT(result = strcmp(lhs, rhs), return NULL);
	if (result < -1)
		result = -1;
	if (result > 1)
		result = 1;
	Dee_return_smallint(result);
}

@ispure
strncmp(lhs:ctypes:char_const*, rhs:ctypes:char_const*, size_t maxlen) {
	int result;
	CTYPES_FAULTPROTECT(result = strncmp(lhs, rhs, maxlen), return NULL);
	if (result < -1)
		result = -1;
	if (result > 1)
		result = 1;
	Dee_return_smallint(result);
}

@ispure
strcasecmp(lhs:ctypes:char_const*, rhs:ctypes:char_const*) {
	int result;
	CTYPES_FAULTPROTECT(result = strcasecmp(lhs, rhs), return NULL);
	if (result < -1)
		result = -1;
	if (result > 1)
		result = 1;
	Dee_return_smallint(result);
}

@ispure
strncasecmp(lhs:ctypes:char_const*, rhs:ctypes:char_const*, size_t maxlen) {
	int result;
	CTYPES_FAULTPROTECT(result = strncasecmp(lhs, rhs, maxlen), return NULL);
	if (result < -1)
		result = -1;
	if (result > 1)
		result = 1;
	Dee_return_smallint(result);
}

@ispure
strverscmp(lhs:ctypes:char_const*, rhs:ctypes:char_const*) {
	int result;
	CTYPES_FAULTPROTECT(result = strverscmp(lhs, rhs), return NULL);
	if (result < -1)
		result = -1;
	if (result > 1)
		result = 1;
	Dee_return_smallint(result);
}

strcpy(dst:ctypes:char*, src:ctypes:char_const*) {
	CTYPES_FAULTPROTECT(dst = strcpy(dst, src), return NULL);
	return DeePointer_NewFor(&DeeCChar_Type, (void *)dst);
}

strcat(dst:ctypes:char*, src:ctypes:char_const*) {
	CTYPES_FAULTPROTECT(dst = strcat(dst, src), return NULL);
	return DeePointer_NewFor(&DeeCChar_Type, (void *)dst);
}

stpcpy(dst:ctypes:char*, src:ctypes:char_const*) {
	CTYPES_FAULTPROTECT(dst = stpcpy(dst, src), return NULL);
	return DeePointer_NewFor(&DeeCChar_Type, (void *)dst);
}

strncpy(dst:ctypes:char*, src:ctypes:char_const*, size_t maxlen) {
	CTYPES_FAULTPROTECT(dst = strncpy(dst, src, maxlen), return NULL);
	return DeePointer_NewFor(&DeeCChar_Type, (void *)dst);
}

strncat(dst:ctypes:char*, src:ctypes:char_const*, size_t maxlen) {
	CTYPES_FAULTPROTECT(dst = strncat(dst, src, maxlen), return NULL);
	return DeePointer_NewFor(&DeeCChar_Type, (void *)dst);
}

stpncpy(dst:ctypes:char*, src:ctypes:char_const*, size_t maxlen) {
	CTYPES_FAULTPROTECT(dst = stpncpy(dst, src, maxlen), return NULL);
	return DeePointer_NewFor(&DeeCChar_Type, (void *)dst);
}

@ispure
strstr(haystack:ctypes:char_const*, needle:ctypes:char_const*) {
	char const *result;
	CTYPES_FAULTPROTECT(result = strstr(haystack, needle), return NULL);
	return DeePointer_NewFor(&DeeCChar_Type, (void *)result);
}

@ispure
strnstr(haystack:ctypes:char_const*, needle:ctypes:char_const*, size_t haystack_maxlen) {
	char const *result;
	CTYPES_FAULTPROTECT(result = strnstr(haystack, needle, haystack_maxlen), return NULL);
	return DeePointer_NewFor(&DeeCChar_Type, (void *)result);
}

@ispure
strcasestr(haystack:ctypes:char_const*, needle:ctypes:char_const*) {
	char const *result;
	CTYPES_FAULTPROTECT(result = strcasestr(haystack, needle), return NULL);
	return DeePointer_NewFor(&DeeCChar_Type, (void *)result);
}

@ispure
strncasestr(haystack:ctypes:char_const*, needle:ctypes:char_const*, size_t haystack_maxlen) {
	char const *result;
	CTYPES_FAULTPROTECT(result = strncasestr(haystack, needle, haystack_maxlen), return NULL);
	return DeePointer_NewFor(&DeeCChar_Type, (void *)result);
}

@ispure
strspn(haystack:ctypes:char_const*, accept:ctypes:char_const*) {
	size_t result;
	CTYPES_FAULTPROTECT(result = strspn(haystack, accept), return NULL);
	return DeeInt_NewSize(result);
}

@ispure
strcspn(haystack:ctypes:char_const*, accept:ctypes:char_const*) {
	size_t result;
	CTYPES_FAULTPROTECT(result = strcspn(haystack, accept), return NULL);
	return DeeInt_NewSize(result);
}

@ispure
strpbrk(haystack:ctypes:char_const*, accept:ctypes:char_const*) {
	char const *result;
	CTYPES_FAULTPROTECT(result = strpbrk(haystack, accept), return NULL);
	return DeePointer_NewFor(&DeeCChar_Type, (void *)result);
}


@ispure
basename(str:ctypes:char_const*) {
	CTYPES_FAULTPROTECT(str = basename(str), return NULL);
	return DeePointer_NewFor(&DeeCChar_Type, (void *)str);
}

strrev(str:ctypes:char*) {
	CTYPES_FAULTPROTECT(str = strrev(str), return NULL);
	return DeePointer_NewFor(&DeeCChar_Type, (void *)str);
}

strnrev(str:ctypes:char*, size_t maxlen) {
	CTYPES_FAULTPROTECT(str = strnrev(str, maxlen), return NULL);
	return DeePointer_NewFor(&DeeCChar_Type, (void *)str);
}

strlwr(str:ctypes:char*) {
	CTYPES_FAULTPROTECT(str = strlwr(str), return NULL);
	return DeePointer_NewFor(&DeeCChar_Type, (void *)str);
}

strnlwr(str:ctypes:char*, size_t maxlen) {
	CTYPES_FAULTPROTECT(str = strnlwr(str, maxlen), return NULL);
	return DeePointer_NewFor(&DeeCChar_Type, (void *)str);
}

strupr(str:ctypes:char*) {
	CTYPES_FAULTPROTECT(str = strupr(str), return NULL);
	return DeePointer_NewFor(&DeeCChar_Type, (void *)str);
}

strnupr(str:ctypes:char*, size_t maxlen) {
	CTYPES_FAULTPROTECT(str = strnupr(str, maxlen), return NULL);
	return DeePointer_NewFor(&DeeCChar_Type, (void *)str);
}

strset(str:ctypes:char*, int ch) {
	CTYPES_FAULTPROTECT(str = strset(str, ch), return NULL);
	return DeePointer_NewFor(&DeeCChar_Type, (void *)str);
}

strnset(str:ctypes:char*, int ch, size_t maxlen) {
	CTYPES_FAULTPROTECT(str = strnset(str, ch, maxlen), return NULL);
	return DeePointer_NewFor(&DeeCChar_Type, (void *)str);
}

memfrob(buf:ctypes:void*, size_t num_bytes) {
	CTYPES_FAULTPROTECT(buf = memfrob(buf, num_bytes), return NULL);
	return DeePointer_NewVoid(buf);
}

strsep(stringp:ctypes:char**, delim:ctypes:char_const*) {
	char *result;
	CTYPES_FAULTPROTECT(result = strsep(stringp, delim), return NULL);
	return DeePointer_NewFor(&DeeCChar_Type, (void *)result);
}

stresep(stringp:ctypes:char**, delim:ctypes:char_const*, int escape) {
	char *result;
	CTYPES_FAULTPROTECT(result = stresep(stringp, delim, escape), return NULL);
	return DeePointer_NewFor(&DeeCChar_Type, (void *)result);
}

strtok(str:ctypes:char*, delim:ctypes:char_const*) {
	char *result;
	CTYPES_FAULTPROTECT(result = strtok(str, delim), return NULL);
	return DeePointer_NewFor(&DeeCChar_Type, (void *)result);
}

strtok_r(str:ctypes:char*, delim:ctypes:char_const*, save_ptr:ctypes:char**) {
	char *result;
	CTYPES_FAULTPROTECT(result = strtok_r(str, delim, save_ptr), return NULL);
	return DeePointer_NewFor(&DeeCChar_Type, (void *)result);
}

""");

// TODO: strfry

]]]*/
/* memcpy */
#define c_string_memcpy_params "dst:?Aptr?Gvoid,src:?Aptr?Gvoid,num_bytes:?Dint,elem_count=!1"
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_memcpy_f_impl(void *dst, void const *src, size_t num_bytes, size_t elem_count);
PRIVATE WUNUSED DREF DeeObject *DCALL c_string_memcpy_f(size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *raw_dst;
		DeeObject *raw_src;
		size_t num_bytes;
		size_t elem_count;
	} args;
	union pointer dst;
	union pointer src;
	args.elem_count = 1;
	if (DeeArg_UnpackStruct(argc, argv, "oo" UNPuSIZ "|" UNPuSIZ ":memcpy", &args))
		goto err;
	if unlikely(DeeObject_AsPointer(args.raw_dst, &DeeCVoid_Type, &dst))
		goto err;
	if unlikely(DeeObject_AsPointer(args.raw_src, &DeeCVoid_Type, &src))
		goto err;
	return c_string_memcpy_f_impl(dst.pvoid, src.pcvoid, args.num_bytes, args.elem_count);
err:
	return NULL;
}
INTERN DEFINE_CMETHOD(c_string_memcpy, &c_string_memcpy_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_memcpy_f_impl(void *dst, void const *src, size_t num_bytes, size_t elem_count)
{
	size_t total;
	if (OVERFLOW_UMUL(num_bytes, elem_count, &total))
		goto err_overflow;
	CTYPES_FAULTPROTECT(dst = memcpy(dst, src, total), return NULL);
	return DeePointer_NewVoid(dst);
err_overflow:
	DeeRT_ErrIntegerOverflowUMul(num_bytes, elem_count);
	return NULL;
}

/* mempcpy */
#define c_string_mempcpy_params "dst:?Aptr?Gvoid,src:?Aptr?Gvoid,num_bytes:?Dint,elem_count=!1"
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_mempcpy_f_impl(void *dst, void const *src, size_t num_bytes, size_t elem_count);
PRIVATE WUNUSED DREF DeeObject *DCALL c_string_mempcpy_f(size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *raw_dst;
		DeeObject *raw_src;
		size_t num_bytes;
		size_t elem_count;
	} args;
	union pointer dst;
	union pointer src;
	args.elem_count = 1;
	if (DeeArg_UnpackStruct(argc, argv, "oo" UNPuSIZ "|" UNPuSIZ ":mempcpy", &args))
		goto err;
	if unlikely(DeeObject_AsPointer(args.raw_dst, &DeeCVoid_Type, &dst))
		goto err;
	if unlikely(DeeObject_AsPointer(args.raw_src, &DeeCVoid_Type, &src))
		goto err;
	return c_string_mempcpy_f_impl(dst.pvoid, src.pcvoid, args.num_bytes, args.elem_count);
err:
	return NULL;
}
INTERN DEFINE_CMETHOD(c_string_mempcpy, &c_string_mempcpy_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_mempcpy_f_impl(void *dst, void const *src, size_t num_bytes, size_t elem_count)
{
	size_t total;
	if (OVERFLOW_UMUL(num_bytes, elem_count, &total))
		goto err_overflow;
	CTYPES_FAULTPROTECT(dst = memcpy(dst, src, total), return NULL);
	return DeePointer_NewVoid((byte_t *)dst + total);
err_overflow:
	DeeRT_ErrIntegerOverflowUMul(num_bytes, elem_count);
	return NULL;
}

/* memmove */
#define c_string_memmove_params "dst:?Aptr?Gvoid,src:?Aptr?Gvoid,num_bytes:?Dint,elem_count=!1"
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_memmove_f_impl(void *dst, void const *src, size_t num_bytes, size_t elem_count);
PRIVATE WUNUSED DREF DeeObject *DCALL c_string_memmove_f(size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *raw_dst;
		DeeObject *raw_src;
		size_t num_bytes;
		size_t elem_count;
	} args;
	union pointer dst;
	union pointer src;
	args.elem_count = 1;
	if (DeeArg_UnpackStruct(argc, argv, "oo" UNPuSIZ "|" UNPuSIZ ":memmove", &args))
		goto err;
	if unlikely(DeeObject_AsPointer(args.raw_dst, &DeeCVoid_Type, &dst))
		goto err;
	if unlikely(DeeObject_AsPointer(args.raw_src, &DeeCVoid_Type, &src))
		goto err;
	return c_string_memmove_f_impl(dst.pvoid, src.pcvoid, args.num_bytes, args.elem_count);
err:
	return NULL;
}
INTERN DEFINE_CMETHOD(c_string_memmove, &c_string_memmove_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_memmove_f_impl(void *dst, void const *src, size_t num_bytes, size_t elem_count)
{
	size_t total;
	if (OVERFLOW_UMUL(num_bytes, elem_count, &total))
		goto err_overflow;
	CTYPES_FAULTPROTECT(dst = memmove(dst, src, total), return NULL);
	return DeePointer_NewVoid(dst);
err_overflow:
	DeeRT_ErrIntegerOverflowUMul(num_bytes, elem_count);
	return NULL;
}

/* mempmove */
#define c_string_mempmove_params "dst:?Aptr?Gvoid,src:?Aptr?Gvoid,num_bytes:?Dint,elem_count=!1"
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_mempmove_f_impl(void *dst, void const *src, size_t num_bytes, size_t elem_count);
PRIVATE WUNUSED DREF DeeObject *DCALL c_string_mempmove_f(size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *raw_dst;
		DeeObject *raw_src;
		size_t num_bytes;
		size_t elem_count;
	} args;
	union pointer dst;
	union pointer src;
	args.elem_count = 1;
	if (DeeArg_UnpackStruct(argc, argv, "oo" UNPuSIZ "|" UNPuSIZ ":mempmove", &args))
		goto err;
	if unlikely(DeeObject_AsPointer(args.raw_dst, &DeeCVoid_Type, &dst))
		goto err;
	if unlikely(DeeObject_AsPointer(args.raw_src, &DeeCVoid_Type, &src))
		goto err;
	return c_string_mempmove_f_impl(dst.pvoid, src.pcvoid, args.num_bytes, args.elem_count);
err:
	return NULL;
}
INTERN DEFINE_CMETHOD(c_string_mempmove, &c_string_mempmove_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_mempmove_f_impl(void *dst, void const *src, size_t num_bytes, size_t elem_count)
{
	size_t total;
	if (OVERFLOW_UMUL(num_bytes, elem_count, &total))
		goto err_overflow;
	CTYPES_FAULTPROTECT(dst = memmove(dst, src, total), return NULL);
	return DeePointer_NewVoid((byte_t *)dst + total);
err_overflow:
	DeeRT_ErrIntegerOverflowUMul(num_bytes, elem_count);
	return NULL;
}

/* memccpy */
#define c_string_memccpy_params "dst:?Aptr?Gvoid,src:?Aptr?Gvoid,needle:?Dint,num_bytes:?Dint"
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_memccpy_f_impl(void *dst, void const *src, int needle, size_t num_bytes);
PRIVATE WUNUSED DREF DeeObject *DCALL c_string_memccpy_f(size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *raw_dst;
		DeeObject *raw_src;
		int needle;
		size_t num_bytes;
	} args;
	union pointer dst;
	union pointer src;
	if (DeeArg_UnpackStruct(argc, argv, "ood" UNPuSIZ ":memccpy", &args))
		goto err;
	if unlikely(DeeObject_AsPointer(args.raw_dst, &DeeCVoid_Type, &dst))
		goto err;
	if unlikely(DeeObject_AsPointer(args.raw_src, &DeeCVoid_Type, &src))
		goto err;
	return c_string_memccpy_f_impl(dst.pvoid, src.pcvoid, args.needle, args.num_bytes);
err:
	return NULL;
}
INTERN DEFINE_CMETHOD(c_string_memccpy, &c_string_memccpy_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_memccpy_f_impl(void *dst, void const *src, int needle, size_t num_bytes)
{
	CTYPES_FAULTPROTECT(dst = memccpy(dst, src, needle, num_bytes), return NULL);
	return DeePointer_NewVoid(dst);
}

/* memset */
#define c_string_memset_params "dst:?Aptr?Gvoid,byte:?Dint,num_bytes:?Dint"
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_memset_f_impl(void *dst, int byte, size_t num_bytes);
PRIVATE WUNUSED DREF DeeObject *DCALL c_string_memset_f(size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *raw_dst;
		int byte;
		size_t num_bytes;
	} args;
	union pointer dst;
	DeeArg_UnpackStruct3X(err, argc, argv, "memset", &args, &args.raw_dst, "o", _DeeArg_AsObject, &args.byte, "d", DeeObject_AsInt, &args.num_bytes, UNPuSIZ, DeeObject_AsSize);
	if unlikely(DeeObject_AsPointer(args.raw_dst, &DeeCVoid_Type, &dst))
		goto err;
	return c_string_memset_f_impl(dst.pvoid, args.byte, args.num_bytes);
err:
	return NULL;
}
INTERN DEFINE_CMETHOD(c_string_memset, &c_string_memset_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_memset_f_impl(void *dst, int byte, size_t num_bytes)
{
	CTYPES_FAULTPROTECT(dst = memset(dst, byte, num_bytes), return NULL);
	return DeePointer_NewVoid(dst);
}

/* mempset */
#define c_string_mempset_params "dst:?Aptr?Gvoid,byte:?Dint,num_bytes:?Dint"
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_mempset_f_impl(void *dst, int byte, size_t num_bytes);
PRIVATE WUNUSED DREF DeeObject *DCALL c_string_mempset_f(size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *raw_dst;
		int byte;
		size_t num_bytes;
	} args;
	union pointer dst;
	DeeArg_UnpackStruct3X(err, argc, argv, "mempset", &args, &args.raw_dst, "o", _DeeArg_AsObject, &args.byte, "d", DeeObject_AsInt, &args.num_bytes, UNPuSIZ, DeeObject_AsSize);
	if unlikely(DeeObject_AsPointer(args.raw_dst, &DeeCVoid_Type, &dst))
		goto err;
	return c_string_mempset_f_impl(dst.pvoid, args.byte, args.num_bytes);
err:
	return NULL;
}
INTERN DEFINE_CMETHOD(c_string_mempset, &c_string_mempset_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_mempset_f_impl(void *dst, int byte, size_t num_bytes)
{
	CTYPES_FAULTPROTECT(dst = memset(dst, byte, num_bytes), return NULL);
	return DeePointer_NewVoid((byte_t *)dst + num_bytes);
}

/* bzero */
#define c_string_bzero_params "dst:?Aptr?Gvoid,num_bytes:?Dint,elem_count=!1"
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_bzero_f_impl(void *dst, size_t num_bytes, size_t elem_count);
PRIVATE WUNUSED DREF DeeObject *DCALL c_string_bzero_f(size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *raw_dst;
		size_t num_bytes;
		size_t elem_count;
	} args;
	union pointer dst;
	args.elem_count = 1;
	if (DeeArg_UnpackStruct(argc, argv, "o" UNPuSIZ "|" UNPuSIZ ":bzero", &args))
		goto err;
	if unlikely(DeeObject_AsPointer(args.raw_dst, &DeeCVoid_Type, &dst))
		goto err;
	return c_string_bzero_f_impl(dst.pvoid, args.num_bytes, args.elem_count);
err:
	return NULL;
}
INTERN DEFINE_CMETHOD(c_string_bzero, &c_string_bzero_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_bzero_f_impl(void *dst, size_t num_bytes, size_t elem_count)
{
	size_t total;
	if (OVERFLOW_UMUL(num_bytes, elem_count, &total))
		goto err_overflow;
	CTYPES_FAULTPROTECT(bzero(dst, total), return NULL);
	return_none;
err_overflow:
	DeeRT_ErrIntegerOverflowUMul(num_bytes, elem_count);
	return NULL;
}

/* memchr */
#define c_string_memchr_params "haystack:?Aptr?Gvoid,needle:?Dint,num_bytes:?Dint"
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_memchr_f_impl(void *haystack, int needle, size_t num_bytes);
PRIVATE WUNUSED DREF DeeObject *DCALL c_string_memchr_f(size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *raw_haystack;
		int needle;
		size_t num_bytes;
	} args;
	union pointer haystack;
	DeeArg_UnpackStruct3X(err, argc, argv, "memchr", &args, &args.raw_haystack, "o", _DeeArg_AsObject, &args.needle, "d", DeeObject_AsInt, &args.num_bytes, UNPuSIZ, DeeObject_AsSize);
	if unlikely(DeeObject_AsPointer(args.raw_haystack, &DeeCVoid_Type, &haystack))
		goto err;
	return c_string_memchr_f_impl(haystack.pvoid, args.needle, args.num_bytes);
err:
	return NULL;
}
INTERN DEFINE_CMETHOD(c_string_memchr, &c_string_memchr_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_memchr_f_impl(void *haystack, int needle, size_t num_bytes)
{
	void *result;
	CTYPES_FAULTPROTECT(result = memchr(haystack, needle, num_bytes), return NULL);
	return DeePointer_NewVoid(result);
}

/* memrchr */
#define c_string_memrchr_params "haystack:?Aptr?Gvoid,needle:?Dint,num_bytes:?Dint"
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_memrchr_f_impl(void *haystack, int needle, size_t num_bytes);
PRIVATE WUNUSED DREF DeeObject *DCALL c_string_memrchr_f(size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *raw_haystack;
		int needle;
		size_t num_bytes;
	} args;
	union pointer haystack;
	DeeArg_UnpackStruct3X(err, argc, argv, "memrchr", &args, &args.raw_haystack, "o", _DeeArg_AsObject, &args.needle, "d", DeeObject_AsInt, &args.num_bytes, UNPuSIZ, DeeObject_AsSize);
	if unlikely(DeeObject_AsPointer(args.raw_haystack, &DeeCVoid_Type, &haystack))
		goto err;
	return c_string_memrchr_f_impl(haystack.pvoid, args.needle, args.num_bytes);
err:
	return NULL;
}
INTERN DEFINE_CMETHOD(c_string_memrchr, &c_string_memrchr_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_memrchr_f_impl(void *haystack, int needle, size_t num_bytes)
{
	void *result;
	CTYPES_FAULTPROTECT(result = memrchr(haystack, needle, num_bytes), return NULL);
	return DeePointer_NewVoid(result);
}

/* memend */
#define c_string_memend_params "haystack:?Aptr?Gvoid,needle:?Dint,num_bytes:?Dint"
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_memend_f_impl(void *haystack, int needle, size_t num_bytes);
PRIVATE WUNUSED DREF DeeObject *DCALL c_string_memend_f(size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *raw_haystack;
		int needle;
		size_t num_bytes;
	} args;
	union pointer haystack;
	DeeArg_UnpackStruct3X(err, argc, argv, "memend", &args, &args.raw_haystack, "o", _DeeArg_AsObject, &args.needle, "d", DeeObject_AsInt, &args.num_bytes, UNPuSIZ, DeeObject_AsSize);
	if unlikely(DeeObject_AsPointer(args.raw_haystack, &DeeCVoid_Type, &haystack))
		goto err;
	return c_string_memend_f_impl(haystack.pvoid, args.needle, args.num_bytes);
err:
	return NULL;
}
INTERN DEFINE_CMETHOD(c_string_memend, &c_string_memend_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_memend_f_impl(void *haystack, int needle, size_t num_bytes)
{
	void *result;
	CTYPES_FAULTPROTECT(result = memend(haystack, needle, num_bytes), return NULL);
	return DeePointer_NewVoid(result);
}

/* memrend */
#define c_string_memrend_params "haystack:?Aptr?Gvoid,needle:?Dint,num_bytes:?Dint"
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_memrend_f_impl(void *haystack, int needle, size_t num_bytes);
PRIVATE WUNUSED DREF DeeObject *DCALL c_string_memrend_f(size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *raw_haystack;
		int needle;
		size_t num_bytes;
	} args;
	union pointer haystack;
	DeeArg_UnpackStruct3X(err, argc, argv, "memrend", &args, &args.raw_haystack, "o", _DeeArg_AsObject, &args.needle, "d", DeeObject_AsInt, &args.num_bytes, UNPuSIZ, DeeObject_AsSize);
	if unlikely(DeeObject_AsPointer(args.raw_haystack, &DeeCVoid_Type, &haystack))
		goto err;
	return c_string_memrend_f_impl(haystack.pvoid, args.needle, args.num_bytes);
err:
	return NULL;
}
INTERN DEFINE_CMETHOD(c_string_memrend, &c_string_memrend_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_memrend_f_impl(void *haystack, int needle, size_t num_bytes)
{
	void *result;
	CTYPES_FAULTPROTECT(result = memrend(haystack, needle, num_bytes), return NULL);
	return DeePointer_NewVoid(result);
}

/* memlen */
#define c_string_memlen_params "haystack:?Aptr?Gvoid,needle:?Dint,num_bytes:?Dint"
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_memlen_f_impl(void *haystack, int needle, size_t num_bytes);
PRIVATE WUNUSED DREF DeeObject *DCALL c_string_memlen_f(size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *raw_haystack;
		int needle;
		size_t num_bytes;
	} args;
	union pointer haystack;
	DeeArg_UnpackStruct3X(err, argc, argv, "memlen", &args, &args.raw_haystack, "o", _DeeArg_AsObject, &args.needle, "d", DeeObject_AsInt, &args.num_bytes, UNPuSIZ, DeeObject_AsSize);
	if unlikely(DeeObject_AsPointer(args.raw_haystack, &DeeCVoid_Type, &haystack))
		goto err;
	return c_string_memlen_f_impl(haystack.pvoid, args.needle, args.num_bytes);
err:
	return NULL;
}
INTERN DEFINE_CMETHOD(c_string_memlen, &c_string_memlen_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_memlen_f_impl(void *haystack, int needle, size_t num_bytes)
{
	size_t result;
	CTYPES_FAULTPROTECT(result = memlen(haystack, needle, num_bytes), return NULL);
	return DeeInt_NewSize(result);
}

/* memrlen */
#define c_string_memrlen_params "haystack:?Aptr?Gvoid,needle:?Dint,num_bytes:?Dint"
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_memrlen_f_impl(void *haystack, int needle, size_t num_bytes);
PRIVATE WUNUSED DREF DeeObject *DCALL c_string_memrlen_f(size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *raw_haystack;
		int needle;
		size_t num_bytes;
	} args;
	union pointer haystack;
	DeeArg_UnpackStruct3X(err, argc, argv, "memrlen", &args, &args.raw_haystack, "o", _DeeArg_AsObject, &args.needle, "d", DeeObject_AsInt, &args.num_bytes, UNPuSIZ, DeeObject_AsSize);
	if unlikely(DeeObject_AsPointer(args.raw_haystack, &DeeCVoid_Type, &haystack))
		goto err;
	return c_string_memrlen_f_impl(haystack.pvoid, args.needle, args.num_bytes);
err:
	return NULL;
}
INTERN DEFINE_CMETHOD(c_string_memrlen, &c_string_memrlen_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_memrlen_f_impl(void *haystack, int needle, size_t num_bytes)
{
	size_t result;
	CTYPES_FAULTPROTECT(result = memrlen(haystack, needle, num_bytes), return NULL);
	return DeeInt_NewSize(result);
}

/* rawmemchr */
#define c_string_rawmemchr_params "haystack:?Aptr?Gvoid,needle:?Dint"
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_rawmemchr_f_impl(void *haystack, int needle);
PRIVATE WUNUSED DREF DeeObject *DCALL c_string_rawmemchr_f(size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *raw_haystack;
		int needle;
	} args;
	union pointer haystack;
	DeeArg_UnpackStruct2X(err, argc, argv, "rawmemchr", &args, &args.raw_haystack, "o", _DeeArg_AsObject, &args.needle, "d", DeeObject_AsInt);
	if unlikely(DeeObject_AsPointer(args.raw_haystack, &DeeCVoid_Type, &haystack))
		goto err;
	return c_string_rawmemchr_f_impl(haystack.pvoid, args.needle);
err:
	return NULL;
}
INTERN DEFINE_CMETHOD(c_string_rawmemchr, &c_string_rawmemchr_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_rawmemchr_f_impl(void *haystack, int needle)
{
	void *result;
	CTYPES_FAULTPROTECT(result = rawmemchr(haystack, needle), return NULL);
	return DeePointer_NewVoid(result);
}

/* rawmemrchr */
#define c_string_rawmemrchr_params "haystack:?Aptr?Gvoid,needle:?Dint"
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_rawmemrchr_f_impl(void *haystack, int needle);
PRIVATE WUNUSED DREF DeeObject *DCALL c_string_rawmemrchr_f(size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *raw_haystack;
		int needle;
	} args;
	union pointer haystack;
	DeeArg_UnpackStruct2X(err, argc, argv, "rawmemrchr", &args, &args.raw_haystack, "o", _DeeArg_AsObject, &args.needle, "d", DeeObject_AsInt);
	if unlikely(DeeObject_AsPointer(args.raw_haystack, &DeeCVoid_Type, &haystack))
		goto err;
	return c_string_rawmemrchr_f_impl(haystack.pvoid, args.needle);
err:
	return NULL;
}
INTERN DEFINE_CMETHOD(c_string_rawmemrchr, &c_string_rawmemrchr_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_rawmemrchr_f_impl(void *haystack, int needle)
{
	void *result;
	CTYPES_FAULTPROTECT(result = rawmemrchr(haystack, needle), return NULL);
	return DeePointer_NewVoid(result);
}

/* rawmemlen */
#define c_string_rawmemlen_params "haystack:?Aptr?Gvoid,needle:?Dint"
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_rawmemlen_f_impl(void *haystack, int needle);
PRIVATE WUNUSED DREF DeeObject *DCALL c_string_rawmemlen_f(size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *raw_haystack;
		int needle;
	} args;
	union pointer haystack;
	DeeArg_UnpackStruct2X(err, argc, argv, "rawmemlen", &args, &args.raw_haystack, "o", _DeeArg_AsObject, &args.needle, "d", DeeObject_AsInt);
	if unlikely(DeeObject_AsPointer(args.raw_haystack, &DeeCVoid_Type, &haystack))
		goto err;
	return c_string_rawmemlen_f_impl(haystack.pvoid, args.needle);
err:
	return NULL;
}
INTERN DEFINE_CMETHOD(c_string_rawmemlen, &c_string_rawmemlen_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_rawmemlen_f_impl(void *haystack, int needle)
{
	size_t result;
	CTYPES_FAULTPROTECT(result = rawmemlen(haystack, needle), return NULL);
	return DeeInt_NewSize(result);
}

/* rawmemrlen */
#define c_string_rawmemrlen_params "haystack:?Aptr?Gvoid,needle:?Dint"
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_rawmemrlen_f_impl(void *haystack, int needle);
PRIVATE WUNUSED DREF DeeObject *DCALL c_string_rawmemrlen_f(size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *raw_haystack;
		int needle;
	} args;
	union pointer haystack;
	DeeArg_UnpackStruct2X(err, argc, argv, "rawmemrlen", &args, &args.raw_haystack, "o", _DeeArg_AsObject, &args.needle, "d", DeeObject_AsInt);
	if unlikely(DeeObject_AsPointer(args.raw_haystack, &DeeCVoid_Type, &haystack))
		goto err;
	return c_string_rawmemrlen_f_impl(haystack.pvoid, args.needle);
err:
	return NULL;
}
INTERN DEFINE_CMETHOD(c_string_rawmemrlen, &c_string_rawmemrlen_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_rawmemrlen_f_impl(void *haystack, int needle)
{
	size_t result;
	CTYPES_FAULTPROTECT(result = rawmemrlen(haystack, needle), return NULL);
	return DeeInt_NewSize(result);
}

/* memxchr */
#define c_string_memxchr_params "haystack:?Aptr?Gvoid,needle:?Dint,num_bytes:?Dint"
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_memxchr_f_impl(void *haystack, int needle, size_t num_bytes);
PRIVATE WUNUSED DREF DeeObject *DCALL c_string_memxchr_f(size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *raw_haystack;
		int needle;
		size_t num_bytes;
	} args;
	union pointer haystack;
	DeeArg_UnpackStruct3X(err, argc, argv, "memxchr", &args, &args.raw_haystack, "o", _DeeArg_AsObject, &args.needle, "d", DeeObject_AsInt, &args.num_bytes, UNPuSIZ, DeeObject_AsSize);
	if unlikely(DeeObject_AsPointer(args.raw_haystack, &DeeCVoid_Type, &haystack))
		goto err;
	return c_string_memxchr_f_impl(haystack.pvoid, args.needle, args.num_bytes);
err:
	return NULL;
}
INTERN DEFINE_CMETHOD(c_string_memxchr, &c_string_memxchr_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_memxchr_f_impl(void *haystack, int needle, size_t num_bytes)
{
	void *result;
	CTYPES_FAULTPROTECT(result = memxchr(haystack, needle, num_bytes), return NULL);
	return DeePointer_NewVoid(result);
}

/* memrxchr */
#define c_string_memrxchr_params "haystack:?Aptr?Gvoid,needle:?Dint,num_bytes:?Dint"
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_memrxchr_f_impl(void *haystack, int needle, size_t num_bytes);
PRIVATE WUNUSED DREF DeeObject *DCALL c_string_memrxchr_f(size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *raw_haystack;
		int needle;
		size_t num_bytes;
	} args;
	union pointer haystack;
	DeeArg_UnpackStruct3X(err, argc, argv, "memrxchr", &args, &args.raw_haystack, "o", _DeeArg_AsObject, &args.needle, "d", DeeObject_AsInt, &args.num_bytes, UNPuSIZ, DeeObject_AsSize);
	if unlikely(DeeObject_AsPointer(args.raw_haystack, &DeeCVoid_Type, &haystack))
		goto err;
	return c_string_memrxchr_f_impl(haystack.pvoid, args.needle, args.num_bytes);
err:
	return NULL;
}
INTERN DEFINE_CMETHOD(c_string_memrxchr, &c_string_memrxchr_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_memrxchr_f_impl(void *haystack, int needle, size_t num_bytes)
{
	void *result;
	CTYPES_FAULTPROTECT(result = memrxchr(haystack, needle, num_bytes), return NULL);
	return DeePointer_NewVoid(result);
}

/* memxend */
#define c_string_memxend_params "haystack:?Aptr?Gvoid,needle:?Dint,num_bytes:?Dint"
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_memxend_f_impl(void *haystack, int needle, size_t num_bytes);
PRIVATE WUNUSED DREF DeeObject *DCALL c_string_memxend_f(size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *raw_haystack;
		int needle;
		size_t num_bytes;
	} args;
	union pointer haystack;
	DeeArg_UnpackStruct3X(err, argc, argv, "memxend", &args, &args.raw_haystack, "o", _DeeArg_AsObject, &args.needle, "d", DeeObject_AsInt, &args.num_bytes, UNPuSIZ, DeeObject_AsSize);
	if unlikely(DeeObject_AsPointer(args.raw_haystack, &DeeCVoid_Type, &haystack))
		goto err;
	return c_string_memxend_f_impl(haystack.pvoid, args.needle, args.num_bytes);
err:
	return NULL;
}
INTERN DEFINE_CMETHOD(c_string_memxend, &c_string_memxend_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_memxend_f_impl(void *haystack, int needle, size_t num_bytes)
{
	void *result;
	CTYPES_FAULTPROTECT(result = memxend(haystack, needle, num_bytes), return NULL);
	return DeePointer_NewVoid(result);
}

/* memrxend */
#define c_string_memrxend_params "haystack:?Aptr?Gvoid,needle:?Dint,num_bytes:?Dint"
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_memrxend_f_impl(void *haystack, int needle, size_t num_bytes);
PRIVATE WUNUSED DREF DeeObject *DCALL c_string_memrxend_f(size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *raw_haystack;
		int needle;
		size_t num_bytes;
	} args;
	union pointer haystack;
	DeeArg_UnpackStruct3X(err, argc, argv, "memrxend", &args, &args.raw_haystack, "o", _DeeArg_AsObject, &args.needle, "d", DeeObject_AsInt, &args.num_bytes, UNPuSIZ, DeeObject_AsSize);
	if unlikely(DeeObject_AsPointer(args.raw_haystack, &DeeCVoid_Type, &haystack))
		goto err;
	return c_string_memrxend_f_impl(haystack.pvoid, args.needle, args.num_bytes);
err:
	return NULL;
}
INTERN DEFINE_CMETHOD(c_string_memrxend, &c_string_memrxend_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_memrxend_f_impl(void *haystack, int needle, size_t num_bytes)
{
	void *result;
	CTYPES_FAULTPROTECT(result = memrxend(haystack, needle, num_bytes), return NULL);
	return DeePointer_NewVoid(result);
}

/* memxlen */
#define c_string_memxlen_params "haystack:?Aptr?Gvoid,needle:?Dint,num_bytes:?Dint"
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_memxlen_f_impl(void *haystack, int needle, size_t num_bytes);
PRIVATE WUNUSED DREF DeeObject *DCALL c_string_memxlen_f(size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *raw_haystack;
		int needle;
		size_t num_bytes;
	} args;
	union pointer haystack;
	DeeArg_UnpackStruct3X(err, argc, argv, "memxlen", &args, &args.raw_haystack, "o", _DeeArg_AsObject, &args.needle, "d", DeeObject_AsInt, &args.num_bytes, UNPuSIZ, DeeObject_AsSize);
	if unlikely(DeeObject_AsPointer(args.raw_haystack, &DeeCVoid_Type, &haystack))
		goto err;
	return c_string_memxlen_f_impl(haystack.pvoid, args.needle, args.num_bytes);
err:
	return NULL;
}
INTERN DEFINE_CMETHOD(c_string_memxlen, &c_string_memxlen_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_memxlen_f_impl(void *haystack, int needle, size_t num_bytes)
{
	size_t result;
	CTYPES_FAULTPROTECT(result = memxlen(haystack, needle, num_bytes), return NULL);
	return DeeInt_NewSize(result);
}

/* memrxlen */
#define c_string_memrxlen_params "haystack:?Aptr?Gvoid,needle:?Dint,num_bytes:?Dint"
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_memrxlen_f_impl(void *haystack, int needle, size_t num_bytes);
PRIVATE WUNUSED DREF DeeObject *DCALL c_string_memrxlen_f(size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *raw_haystack;
		int needle;
		size_t num_bytes;
	} args;
	union pointer haystack;
	DeeArg_UnpackStruct3X(err, argc, argv, "memrxlen", &args, &args.raw_haystack, "o", _DeeArg_AsObject, &args.needle, "d", DeeObject_AsInt, &args.num_bytes, UNPuSIZ, DeeObject_AsSize);
	if unlikely(DeeObject_AsPointer(args.raw_haystack, &DeeCVoid_Type, &haystack))
		goto err;
	return c_string_memrxlen_f_impl(haystack.pvoid, args.needle, args.num_bytes);
err:
	return NULL;
}
INTERN DEFINE_CMETHOD(c_string_memrxlen, &c_string_memrxlen_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_memrxlen_f_impl(void *haystack, int needle, size_t num_bytes)
{
	size_t result;
	CTYPES_FAULTPROTECT(result = memrxlen(haystack, needle, num_bytes), return NULL);
	return DeeInt_NewSize(result);
}

/* rawmemxchr */
#define c_string_rawmemxchr_params "haystack:?Aptr?Gvoid,needle:?Dint"
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_rawmemxchr_f_impl(void *haystack, int needle);
PRIVATE WUNUSED DREF DeeObject *DCALL c_string_rawmemxchr_f(size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *raw_haystack;
		int needle;
	} args;
	union pointer haystack;
	DeeArg_UnpackStruct2X(err, argc, argv, "rawmemxchr", &args, &args.raw_haystack, "o", _DeeArg_AsObject, &args.needle, "d", DeeObject_AsInt);
	if unlikely(DeeObject_AsPointer(args.raw_haystack, &DeeCVoid_Type, &haystack))
		goto err;
	return c_string_rawmemxchr_f_impl(haystack.pvoid, args.needle);
err:
	return NULL;
}
INTERN DEFINE_CMETHOD(c_string_rawmemxchr, &c_string_rawmemxchr_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_rawmemxchr_f_impl(void *haystack, int needle)
{
	void *result;
	CTYPES_FAULTPROTECT(result = rawmemxchr(haystack, needle), return NULL);
	return DeePointer_NewVoid(result);
}

/* rawmemrxchr */
#define c_string_rawmemrxchr_params "haystack:?Aptr?Gvoid,needle:?Dint"
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_rawmemrxchr_f_impl(void *haystack, int needle);
PRIVATE WUNUSED DREF DeeObject *DCALL c_string_rawmemrxchr_f(size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *raw_haystack;
		int needle;
	} args;
	union pointer haystack;
	DeeArg_UnpackStruct2X(err, argc, argv, "rawmemrxchr", &args, &args.raw_haystack, "o", _DeeArg_AsObject, &args.needle, "d", DeeObject_AsInt);
	if unlikely(DeeObject_AsPointer(args.raw_haystack, &DeeCVoid_Type, &haystack))
		goto err;
	return c_string_rawmemrxchr_f_impl(haystack.pvoid, args.needle);
err:
	return NULL;
}
INTERN DEFINE_CMETHOD(c_string_rawmemrxchr, &c_string_rawmemrxchr_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_rawmemrxchr_f_impl(void *haystack, int needle)
{
	void *result;
	CTYPES_FAULTPROTECT(result = rawmemrxchr(haystack, needle), return NULL);
	return DeePointer_NewVoid(result);
}

/* rawmemxlen */
#define c_string_rawmemxlen_params "haystack:?Aptr?Gvoid,needle:?Dint"
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_rawmemxlen_f_impl(void *haystack, int needle);
PRIVATE WUNUSED DREF DeeObject *DCALL c_string_rawmemxlen_f(size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *raw_haystack;
		int needle;
	} args;
	union pointer haystack;
	DeeArg_UnpackStruct2X(err, argc, argv, "rawmemxlen", &args, &args.raw_haystack, "o", _DeeArg_AsObject, &args.needle, "d", DeeObject_AsInt);
	if unlikely(DeeObject_AsPointer(args.raw_haystack, &DeeCVoid_Type, &haystack))
		goto err;
	return c_string_rawmemxlen_f_impl(haystack.pvoid, args.needle);
err:
	return NULL;
}
INTERN DEFINE_CMETHOD(c_string_rawmemxlen, &c_string_rawmemxlen_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_rawmemxlen_f_impl(void *haystack, int needle)
{
	size_t result;
	CTYPES_FAULTPROTECT(result = rawmemxlen(haystack, needle), return NULL);
	return DeeInt_NewSize(result);
}

/* rawmemrxlen */
#define c_string_rawmemrxlen_params "haystack:?Aptr?Gvoid,needle:?Dint"
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_rawmemrxlen_f_impl(void *haystack, int needle);
PRIVATE WUNUSED DREF DeeObject *DCALL c_string_rawmemrxlen_f(size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *raw_haystack;
		int needle;
	} args;
	union pointer haystack;
	DeeArg_UnpackStruct2X(err, argc, argv, "rawmemrxlen", &args, &args.raw_haystack, "o", _DeeArg_AsObject, &args.needle, "d", DeeObject_AsInt);
	if unlikely(DeeObject_AsPointer(args.raw_haystack, &DeeCVoid_Type, &haystack))
		goto err;
	return c_string_rawmemrxlen_f_impl(haystack.pvoid, args.needle);
err:
	return NULL;
}
INTERN DEFINE_CMETHOD(c_string_rawmemrxlen, &c_string_rawmemrxlen_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_rawmemrxlen_f_impl(void *haystack, int needle)
{
	size_t result;
	CTYPES_FAULTPROTECT(result = rawmemrxlen(haystack, needle), return NULL);
	return DeeInt_NewSize(result);
}

/* bcmp */
#define c_string_bcmp_params "a:?Aptr?Gvoid,b:?Aptr?Gvoid,num_bytes:?Dint"
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_bcmp_f_impl(void const *a, void const *b, size_t num_bytes);
PRIVATE WUNUSED DREF DeeObject *DCALL c_string_bcmp_f(size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *raw_a;
		DeeObject *raw_b;
		size_t num_bytes;
	} args;
	union pointer a;
	union pointer b;
	DeeArg_UnpackStruct3X(err, argc, argv, "bcmp", &args, &args.raw_a, "o", _DeeArg_AsObject, &args.raw_b, "o", _DeeArg_AsObject, &args.num_bytes, UNPuSIZ, DeeObject_AsSize);
	if unlikely(DeeObject_AsPointer(args.raw_a, &DeeCVoid_Type, &a))
		goto err;
	if unlikely(DeeObject_AsPointer(args.raw_b, &DeeCVoid_Type, &b))
		goto err;
	return c_string_bcmp_f_impl(a.pcvoid, b.pcvoid, args.num_bytes);
err:
	return NULL;
}
INTERN DEFINE_CMETHOD(c_string_bcmp, &c_string_bcmp_f, METHOD_FPURECALL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_bcmp_f_impl(void const *a, void const *b, size_t num_bytes)
{
	int result;
	CTYPES_FAULTPROTECT(result = bcmp(a, b, num_bytes), return NULL);
	Dee_return_smallint(result ? 1 : 0);
}

/* memcmp */
#define c_string_memcmp_params "lhs:?Aptr?Gvoid,rhs:?Aptr?Gvoid,num_bytes:?Dint"
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_memcmp_f_impl(void const *lhs, void const *rhs, size_t num_bytes);
PRIVATE WUNUSED DREF DeeObject *DCALL c_string_memcmp_f(size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *raw_lhs;
		DeeObject *raw_rhs;
		size_t num_bytes;
	} args;
	union pointer lhs;
	union pointer rhs;
	DeeArg_UnpackStruct3X(err, argc, argv, "memcmp", &args, &args.raw_lhs, "o", _DeeArg_AsObject, &args.raw_rhs, "o", _DeeArg_AsObject, &args.num_bytes, UNPuSIZ, DeeObject_AsSize);
	if unlikely(DeeObject_AsPointer(args.raw_lhs, &DeeCVoid_Type, &lhs))
		goto err;
	if unlikely(DeeObject_AsPointer(args.raw_rhs, &DeeCVoid_Type, &rhs))
		goto err;
	return c_string_memcmp_f_impl(lhs.pcvoid, rhs.pcvoid, args.num_bytes);
err:
	return NULL;
}
INTERN DEFINE_CMETHOD(c_string_memcmp, &c_string_memcmp_f, METHOD_FPURECALL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_memcmp_f_impl(void const *lhs, void const *rhs, size_t num_bytes)
{
	int result;
	CTYPES_FAULTPROTECT(result = memcmp(lhs, rhs, num_bytes), return NULL);
	if (result < -1)
		result = -1;
	if (result > 1)
		result = 1;
	Dee_return_smallint(result);
}

/* memcasecmp */
#define c_string_memcasecmp_params "lhs:?Aptr?Gvoid,rhs:?Aptr?Gvoid,num_bytes:?Dint"
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_memcasecmp_f_impl(void const *lhs, void const *rhs, size_t num_bytes);
PRIVATE WUNUSED DREF DeeObject *DCALL c_string_memcasecmp_f(size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *raw_lhs;
		DeeObject *raw_rhs;
		size_t num_bytes;
	} args;
	union pointer lhs;
	union pointer rhs;
	DeeArg_UnpackStruct3X(err, argc, argv, "memcasecmp", &args, &args.raw_lhs, "o", _DeeArg_AsObject, &args.raw_rhs, "o", _DeeArg_AsObject, &args.num_bytes, UNPuSIZ, DeeObject_AsSize);
	if unlikely(DeeObject_AsPointer(args.raw_lhs, &DeeCVoid_Type, &lhs))
		goto err;
	if unlikely(DeeObject_AsPointer(args.raw_rhs, &DeeCVoid_Type, &rhs))
		goto err;
	return c_string_memcasecmp_f_impl(lhs.pcvoid, rhs.pcvoid, args.num_bytes);
err:
	return NULL;
}
INTERN DEFINE_CMETHOD(c_string_memcasecmp, &c_string_memcasecmp_f, METHOD_FPURECALL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_memcasecmp_f_impl(void const *lhs, void const *rhs, size_t num_bytes)
{
	int result;
	CTYPES_FAULTPROTECT(result = memcasecmp(lhs, rhs, num_bytes), return NULL);
	return DeeInt_NewInt(result);
}

/* memmem */
#define c_string_memmem_params "haystack:?Aptr?Gvoid,haystack_len:?Dint,needle:?Aptr?Gvoid,needle_len:?Dint"
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_memmem_f_impl(void const *haystack, size_t haystack_len, void const *needle, size_t needle_len);
PRIVATE WUNUSED DREF DeeObject *DCALL c_string_memmem_f(size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *raw_haystack;
		size_t haystack_len;
		DeeObject *raw_needle;
		size_t needle_len;
	} args;
	union pointer haystack;
	union pointer needle;
	if (DeeArg_UnpackStruct(argc, argv, "o" UNPuSIZ "o" UNPuSIZ ":memmem", &args))
		goto err;
	if unlikely(DeeObject_AsPointer(args.raw_haystack, &DeeCVoid_Type, &haystack))
		goto err;
	if unlikely(DeeObject_AsPointer(args.raw_needle, &DeeCVoid_Type, &needle))
		goto err;
	return c_string_memmem_f_impl(haystack.pcvoid, args.haystack_len, needle.pcvoid, args.needle_len);
err:
	return NULL;
}
INTERN DEFINE_CMETHOD(c_string_memmem, &c_string_memmem_f, METHOD_FPURECALL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_memmem_f_impl(void const *haystack, size_t haystack_len, void const *needle, size_t needle_len)
{
	void *result;
	CTYPES_FAULTPROTECT(result = memmem(haystack, haystack_len,
	                                    needle, needle_len),
	                    return NULL);
	return DeePointer_NewVoid(result);
}

/* memcasemem */
#define c_string_memcasemem_params "haystack:?Aptr?Gvoid,haystack_len:?Dint,needle:?Aptr?Gvoid,needle_len:?Dint"
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_memcasemem_f_impl(void const *haystack, size_t haystack_len, void const *needle, size_t needle_len);
PRIVATE WUNUSED DREF DeeObject *DCALL c_string_memcasemem_f(size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *raw_haystack;
		size_t haystack_len;
		DeeObject *raw_needle;
		size_t needle_len;
	} args;
	union pointer haystack;
	union pointer needle;
	if (DeeArg_UnpackStruct(argc, argv, "o" UNPuSIZ "o" UNPuSIZ ":memcasemem", &args))
		goto err;
	if unlikely(DeeObject_AsPointer(args.raw_haystack, &DeeCVoid_Type, &haystack))
		goto err;
	if unlikely(DeeObject_AsPointer(args.raw_needle, &DeeCVoid_Type, &needle))
		goto err;
	return c_string_memcasemem_f_impl(haystack.pcvoid, args.haystack_len, needle.pcvoid, args.needle_len);
err:
	return NULL;
}
INTERN DEFINE_CMETHOD(c_string_memcasemem, &c_string_memcasemem_f, METHOD_FPURECALL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_memcasemem_f_impl(void const *haystack, size_t haystack_len, void const *needle, size_t needle_len)
{
	void *result;
	CTYPES_FAULTPROTECT(result = memcasemem(haystack, haystack_len,
	                                        needle, needle_len),
	                    return NULL);
	return DeePointer_NewVoid(result);
}

/* memrmem */
#define c_string_memrmem_params "haystack:?Aptr?Gvoid,haystack_len:?Dint,needle:?Aptr?Gvoid,needle_len:?Dint"
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_memrmem_f_impl(void const *haystack, size_t haystack_len, void const *needle, size_t needle_len);
PRIVATE WUNUSED DREF DeeObject *DCALL c_string_memrmem_f(size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *raw_haystack;
		size_t haystack_len;
		DeeObject *raw_needle;
		size_t needle_len;
	} args;
	union pointer haystack;
	union pointer needle;
	if (DeeArg_UnpackStruct(argc, argv, "o" UNPuSIZ "o" UNPuSIZ ":memrmem", &args))
		goto err;
	if unlikely(DeeObject_AsPointer(args.raw_haystack, &DeeCVoid_Type, &haystack))
		goto err;
	if unlikely(DeeObject_AsPointer(args.raw_needle, &DeeCVoid_Type, &needle))
		goto err;
	return c_string_memrmem_f_impl(haystack.pcvoid, args.haystack_len, needle.pcvoid, args.needle_len);
err:
	return NULL;
}
INTERN DEFINE_CMETHOD(c_string_memrmem, &c_string_memrmem_f, METHOD_FPURECALL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_memrmem_f_impl(void const *haystack, size_t haystack_len, void const *needle, size_t needle_len)
{
	void *result;
	CTYPES_FAULTPROTECT(result = memrmem(haystack, haystack_len,
	                                     needle, needle_len),
	                    return NULL);
	return DeePointer_NewVoid(result);
}

/* memcasermem */
#define c_string_memcasermem_params "haystack:?Aptr?Gvoid,haystack_len:?Dint,needle:?Aptr?Gvoid,needle_len:?Dint"
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_memcasermem_f_impl(void const *haystack, size_t haystack_len, void const *needle, size_t needle_len);
PRIVATE WUNUSED DREF DeeObject *DCALL c_string_memcasermem_f(size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *raw_haystack;
		size_t haystack_len;
		DeeObject *raw_needle;
		size_t needle_len;
	} args;
	union pointer haystack;
	union pointer needle;
	if (DeeArg_UnpackStruct(argc, argv, "o" UNPuSIZ "o" UNPuSIZ ":memcasermem", &args))
		goto err;
	if unlikely(DeeObject_AsPointer(args.raw_haystack, &DeeCVoid_Type, &haystack))
		goto err;
	if unlikely(DeeObject_AsPointer(args.raw_needle, &DeeCVoid_Type, &needle))
		goto err;
	return c_string_memcasermem_f_impl(haystack.pcvoid, args.haystack_len, needle.pcvoid, args.needle_len);
err:
	return NULL;
}
INTERN DEFINE_CMETHOD(c_string_memcasermem, &c_string_memcasermem_f, METHOD_FPURECALL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_memcasermem_f_impl(void const *haystack, size_t haystack_len, void const *needle, size_t needle_len)
{
	void *result;
	CTYPES_FAULTPROTECT(result = memcasermem(haystack, haystack_len,
	                                         needle, needle_len),
	                    return NULL);
	return DeePointer_NewVoid(result);
}

/* memrev */
#define c_string_memrev_params "dst:?Aptr?Gvoid,num_bytes:?Dint"
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_memrev_f_impl(void *dst, size_t num_bytes);
PRIVATE WUNUSED DREF DeeObject *DCALL c_string_memrev_f(size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *raw_dst;
		size_t num_bytes;
	} args;
	union pointer dst;
	DeeArg_UnpackStruct2X(err, argc, argv, "memrev", &args, &args.raw_dst, "o", _DeeArg_AsObject, &args.num_bytes, UNPuSIZ, DeeObject_AsSize);
	if unlikely(DeeObject_AsPointer(args.raw_dst, &DeeCVoid_Type, &dst))
		goto err;
	return c_string_memrev_f_impl(dst.pvoid, args.num_bytes);
err:
	return NULL;
}
INTERN DEFINE_CMETHOD(c_string_memrev, &c_string_memrev_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_memrev_f_impl(void *dst, size_t num_bytes)
{
	CTYPES_FAULTPROTECT(dst = memrev(dst, num_bytes), return NULL);
	return DeePointer_NewVoid(dst);
}

/* strlen */
#define c_string_strlen_params "string:?Aptr?Gchar"
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_strlen_f_impl(char const *string);
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL c_string_strlen_f(DeeObject *__restrict arg0) {
	union pointer string;
	if unlikely(DeeObject_AsPointer(arg0, &DeeCChar_Type, &string))
		goto err;
	return c_string_strlen_f_impl(string.pcchar);
err:
	return NULL;
}
INTERN DEFINE_CMETHOD1(c_string_strlen, &c_string_strlen_f, METHOD_FPURECALL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_strlen_f_impl(char const *string)
{
	size_t result;
	CTYPES_FAULTPROTECT(result = strlen(string), return NULL);
	return DeeInt_NewSize(result);
}

/* strend */
#define c_string_strend_params "string:?Aptr?Gchar"
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_strend_f_impl(char const *string);
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL c_string_strend_f(DeeObject *__restrict arg0) {
	union pointer string;
	if unlikely(DeeObject_AsPointer(arg0, &DeeCChar_Type, &string))
		goto err;
	return c_string_strend_f_impl(string.pcchar);
err:
	return NULL;
}
INTERN DEFINE_CMETHOD1(c_string_strend, &c_string_strend_f, METHOD_FPURECALL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_strend_f_impl(char const *string)
{
	char const *result;
	CTYPES_FAULTPROTECT(result = strend(string), return NULL);
	return DeePointer_NewFor(&DeeCChar_Type, (void *)result);
}

/* strnlen */
#define c_string_strnlen_params "string:?Aptr?Gchar,maxlen:?Dint"
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_strnlen_f_impl(char const *string, size_t maxlen);
PRIVATE WUNUSED DREF DeeObject *DCALL c_string_strnlen_f(size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *raw_string;
		size_t maxlen;
	} args;
	union pointer string;
	DeeArg_UnpackStruct2X(err, argc, argv, "strnlen", &args, &args.raw_string, "o", _DeeArg_AsObject, &args.maxlen, UNPuSIZ, DeeObject_AsSize);
	if unlikely(DeeObject_AsPointer(args.raw_string, &DeeCChar_Type, &string))
		goto err;
	return c_string_strnlen_f_impl(string.pcchar, args.maxlen);
err:
	return NULL;
}
INTERN DEFINE_CMETHOD(c_string_strnlen, &c_string_strnlen_f, METHOD_FPURECALL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_strnlen_f_impl(char const *string, size_t maxlen)
{
	size_t result;
	CTYPES_FAULTPROTECT(result = strnlen(string, maxlen), return NULL);
	return DeeInt_NewSize(result);
}

/* strnend */
#define c_string_strnend_params "string:?Aptr?Gchar,maxlen:?Dint"
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_strnend_f_impl(char const *string, size_t maxlen);
PRIVATE WUNUSED DREF DeeObject *DCALL c_string_strnend_f(size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *raw_string;
		size_t maxlen;
	} args;
	union pointer string;
	DeeArg_UnpackStruct2X(err, argc, argv, "strnend", &args, &args.raw_string, "o", _DeeArg_AsObject, &args.maxlen, UNPuSIZ, DeeObject_AsSize);
	if unlikely(DeeObject_AsPointer(args.raw_string, &DeeCChar_Type, &string))
		goto err;
	return c_string_strnend_f_impl(string.pcchar, args.maxlen);
err:
	return NULL;
}
INTERN DEFINE_CMETHOD(c_string_strnend, &c_string_strnend_f, METHOD_FPURECALL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_strnend_f_impl(char const *string, size_t maxlen)
{
	char const *result;
	CTYPES_FAULTPROTECT(result = strnend(string, maxlen), return NULL);
	return DeePointer_NewFor(&DeeCChar_Type, (void *)result);
}

/* strchr */
#define c_string_strchr_params "haystack:?Aptr?Gchar,needle:?Dint"
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_strchr_f_impl(char const *haystack, int needle);
PRIVATE WUNUSED DREF DeeObject *DCALL c_string_strchr_f(size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *raw_haystack;
		int needle;
	} args;
	union pointer haystack;
	DeeArg_UnpackStruct2X(err, argc, argv, "strchr", &args, &args.raw_haystack, "o", _DeeArg_AsObject, &args.needle, "d", DeeObject_AsInt);
	if unlikely(DeeObject_AsPointer(args.raw_haystack, &DeeCChar_Type, &haystack))
		goto err;
	return c_string_strchr_f_impl(haystack.pcchar, args.needle);
err:
	return NULL;
}
INTERN DEFINE_CMETHOD(c_string_strchr, &c_string_strchr_f, METHOD_FPURECALL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_strchr_f_impl(char const *haystack, int needle)
{
	char const *result;
	CTYPES_FAULTPROTECT(result = strchr(haystack, needle), return NULL);
	return DeePointer_NewFor(&DeeCChar_Type, (void *)result);
}

/* strrchr */
#define c_string_strrchr_params "haystack:?Aptr?Gchar,needle:?Dint"
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_strrchr_f_impl(char const *haystack, int needle);
PRIVATE WUNUSED DREF DeeObject *DCALL c_string_strrchr_f(size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *raw_haystack;
		int needle;
	} args;
	union pointer haystack;
	DeeArg_UnpackStruct2X(err, argc, argv, "strrchr", &args, &args.raw_haystack, "o", _DeeArg_AsObject, &args.needle, "d", DeeObject_AsInt);
	if unlikely(DeeObject_AsPointer(args.raw_haystack, &DeeCChar_Type, &haystack))
		goto err;
	return c_string_strrchr_f_impl(haystack.pcchar, args.needle);
err:
	return NULL;
}
INTERN DEFINE_CMETHOD(c_string_strrchr, &c_string_strrchr_f, METHOD_FPURECALL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_strrchr_f_impl(char const *haystack, int needle)
{
	char const *result;
	CTYPES_FAULTPROTECT(result = strrchr(haystack, needle), return NULL);
	return DeePointer_NewFor(&DeeCChar_Type, (void *)result);
}

/* strnchr */
#define c_string_strnchr_params "haystack:?Aptr?Gchar,needle:?Dint,maxlen:?Dint"
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_strnchr_f_impl(char const *haystack, int needle, size_t maxlen);
PRIVATE WUNUSED DREF DeeObject *DCALL c_string_strnchr_f(size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *raw_haystack;
		int needle;
		size_t maxlen;
	} args;
	union pointer haystack;
	DeeArg_UnpackStruct3X(err, argc, argv, "strnchr", &args, &args.raw_haystack, "o", _DeeArg_AsObject, &args.needle, "d", DeeObject_AsInt, &args.maxlen, UNPuSIZ, DeeObject_AsSize);
	if unlikely(DeeObject_AsPointer(args.raw_haystack, &DeeCChar_Type, &haystack))
		goto err;
	return c_string_strnchr_f_impl(haystack.pcchar, args.needle, args.maxlen);
err:
	return NULL;
}
INTERN DEFINE_CMETHOD(c_string_strnchr, &c_string_strnchr_f, METHOD_FPURECALL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_strnchr_f_impl(char const *haystack, int needle, size_t maxlen)
{
	char const *result;
	CTYPES_FAULTPROTECT(result = strnchr(haystack, needle, maxlen), return NULL);
	return DeePointer_NewFor(&DeeCChar_Type, (void *)result);
}

/* strnrchr */
#define c_string_strnrchr_params "haystack:?Aptr?Gchar,needle:?Dint,maxlen:?Dint"
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_strnrchr_f_impl(char const *haystack, int needle, size_t maxlen);
PRIVATE WUNUSED DREF DeeObject *DCALL c_string_strnrchr_f(size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *raw_haystack;
		int needle;
		size_t maxlen;
	} args;
	union pointer haystack;
	DeeArg_UnpackStruct3X(err, argc, argv, "strnrchr", &args, &args.raw_haystack, "o", _DeeArg_AsObject, &args.needle, "d", DeeObject_AsInt, &args.maxlen, UNPuSIZ, DeeObject_AsSize);
	if unlikely(DeeObject_AsPointer(args.raw_haystack, &DeeCChar_Type, &haystack))
		goto err;
	return c_string_strnrchr_f_impl(haystack.pcchar, args.needle, args.maxlen);
err:
	return NULL;
}
INTERN DEFINE_CMETHOD(c_string_strnrchr, &c_string_strnrchr_f, METHOD_FPURECALL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_strnrchr_f_impl(char const *haystack, int needle, size_t maxlen)
{
	char const *result;
	CTYPES_FAULTPROTECT(result = strnrchr(haystack, needle, maxlen), return NULL);
	return DeePointer_NewFor(&DeeCChar_Type, (void *)result);
}

/* strchrnul */
#define c_string_strchrnul_params "haystack:?Aptr?Gchar,needle:?Dint"
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_strchrnul_f_impl(char const *haystack, int needle);
PRIVATE WUNUSED DREF DeeObject *DCALL c_string_strchrnul_f(size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *raw_haystack;
		int needle;
	} args;
	union pointer haystack;
	DeeArg_UnpackStruct2X(err, argc, argv, "strchrnul", &args, &args.raw_haystack, "o", _DeeArg_AsObject, &args.needle, "d", DeeObject_AsInt);
	if unlikely(DeeObject_AsPointer(args.raw_haystack, &DeeCChar_Type, &haystack))
		goto err;
	return c_string_strchrnul_f_impl(haystack.pcchar, args.needle);
err:
	return NULL;
}
INTERN DEFINE_CMETHOD(c_string_strchrnul, &c_string_strchrnul_f, METHOD_FPURECALL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_strchrnul_f_impl(char const *haystack, int needle)
{
	char const *result;
	CTYPES_FAULTPROTECT(result = strchrnul(haystack, needle), return NULL);
	return DeePointer_NewFor(&DeeCChar_Type, (void *)result);
}

/* strrchrnul */
#define c_string_strrchrnul_params "haystack:?Aptr?Gchar,needle:?Dint"
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_strrchrnul_f_impl(char const *haystack, int needle);
PRIVATE WUNUSED DREF DeeObject *DCALL c_string_strrchrnul_f(size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *raw_haystack;
		int needle;
	} args;
	union pointer haystack;
	DeeArg_UnpackStruct2X(err, argc, argv, "strrchrnul", &args, &args.raw_haystack, "o", _DeeArg_AsObject, &args.needle, "d", DeeObject_AsInt);
	if unlikely(DeeObject_AsPointer(args.raw_haystack, &DeeCChar_Type, &haystack))
		goto err;
	return c_string_strrchrnul_f_impl(haystack.pcchar, args.needle);
err:
	return NULL;
}
INTERN DEFINE_CMETHOD(c_string_strrchrnul, &c_string_strrchrnul_f, METHOD_FPURECALL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_strrchrnul_f_impl(char const *haystack, int needle)
{
	char const *result;
	CTYPES_FAULTPROTECT(result = strrchrnul(haystack, needle), return NULL);
	return DeePointer_NewFor(&DeeCChar_Type, (void *)result);
}

/* strnchrnul */
#define c_string_strnchrnul_params "haystack:?Aptr?Gchar,needle:?Dint,maxlen:?Dint"
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_strnchrnul_f_impl(char const *haystack, int needle, size_t maxlen);
PRIVATE WUNUSED DREF DeeObject *DCALL c_string_strnchrnul_f(size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *raw_haystack;
		int needle;
		size_t maxlen;
	} args;
	union pointer haystack;
	DeeArg_UnpackStruct3X(err, argc, argv, "strnchrnul", &args, &args.raw_haystack, "o", _DeeArg_AsObject, &args.needle, "d", DeeObject_AsInt, &args.maxlen, UNPuSIZ, DeeObject_AsSize);
	if unlikely(DeeObject_AsPointer(args.raw_haystack, &DeeCChar_Type, &haystack))
		goto err;
	return c_string_strnchrnul_f_impl(haystack.pcchar, args.needle, args.maxlen);
err:
	return NULL;
}
INTERN DEFINE_CMETHOD(c_string_strnchrnul, &c_string_strnchrnul_f, METHOD_FPURECALL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_strnchrnul_f_impl(char const *haystack, int needle, size_t maxlen)
{
	char const *result;
	CTYPES_FAULTPROTECT(result = strnchrnul(haystack, needle, maxlen), return NULL);
	return DeePointer_NewFor(&DeeCChar_Type, (void *)result);
}

/* strnrchrnul */
#define c_string_strnrchrnul_params "haystack:?Aptr?Gchar,needle:?Dint,maxlen:?Dint"
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_strnrchrnul_f_impl(char const *haystack, int needle, size_t maxlen);
PRIVATE WUNUSED DREF DeeObject *DCALL c_string_strnrchrnul_f(size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *raw_haystack;
		int needle;
		size_t maxlen;
	} args;
	union pointer haystack;
	DeeArg_UnpackStruct3X(err, argc, argv, "strnrchrnul", &args, &args.raw_haystack, "o", _DeeArg_AsObject, &args.needle, "d", DeeObject_AsInt, &args.maxlen, UNPuSIZ, DeeObject_AsSize);
	if unlikely(DeeObject_AsPointer(args.raw_haystack, &DeeCChar_Type, &haystack))
		goto err;
	return c_string_strnrchrnul_f_impl(haystack.pcchar, args.needle, args.maxlen);
err:
	return NULL;
}
INTERN DEFINE_CMETHOD(c_string_strnrchrnul, &c_string_strnrchrnul_f, METHOD_FPURECALL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_strnrchrnul_f_impl(char const *haystack, int needle, size_t maxlen)
{
	char const *result;
	CTYPES_FAULTPROTECT(result = strnrchrnul(haystack, needle, maxlen), return NULL);
	return DeePointer_NewFor(&DeeCChar_Type, (void *)result);
}

/* stroff */
#define c_string_stroff_params "haystack:?Aptr?Gchar,needle:?Dint"
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_stroff_f_impl(char const *haystack, int needle);
PRIVATE WUNUSED DREF DeeObject *DCALL c_string_stroff_f(size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *raw_haystack;
		int needle;
	} args;
	union pointer haystack;
	DeeArg_UnpackStruct2X(err, argc, argv, "stroff", &args, &args.raw_haystack, "o", _DeeArg_AsObject, &args.needle, "d", DeeObject_AsInt);
	if unlikely(DeeObject_AsPointer(args.raw_haystack, &DeeCChar_Type, &haystack))
		goto err;
	return c_string_stroff_f_impl(haystack.pcchar, args.needle);
err:
	return NULL;
}
INTERN DEFINE_CMETHOD(c_string_stroff, &c_string_stroff_f, METHOD_FPURECALL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_stroff_f_impl(char const *haystack, int needle)
{
	char const *result;
	CTYPES_FAULTPROTECT(result = strchrnul(haystack, needle), return NULL);
	return DeeInt_NewSize((size_t)(result - haystack));
}

/* strroff */
#define c_string_strroff_params "haystack:?Aptr?Gchar,needle:?Dint"
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_strroff_f_impl(char const *haystack, int needle);
PRIVATE WUNUSED DREF DeeObject *DCALL c_string_strroff_f(size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *raw_haystack;
		int needle;
	} args;
	union pointer haystack;
	DeeArg_UnpackStruct2X(err, argc, argv, "strroff", &args, &args.raw_haystack, "o", _DeeArg_AsObject, &args.needle, "d", DeeObject_AsInt);
	if unlikely(DeeObject_AsPointer(args.raw_haystack, &DeeCChar_Type, &haystack))
		goto err;
	return c_string_strroff_f_impl(haystack.pcchar, args.needle);
err:
	return NULL;
}
INTERN DEFINE_CMETHOD(c_string_strroff, &c_string_strroff_f, METHOD_FPURECALL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_strroff_f_impl(char const *haystack, int needle)
{
	char const *result;
	CTYPES_FAULTPROTECT(result = strrchrnul(haystack, needle), return NULL);
	return DeeInt_NewSize((size_t)(result - haystack));
}

/* strnoff */
#define c_string_strnoff_params "haystack:?Aptr?Gchar,needle:?Dint,maxlen:?Dint"
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_strnoff_f_impl(char const *haystack, int needle, size_t maxlen);
PRIVATE WUNUSED DREF DeeObject *DCALL c_string_strnoff_f(size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *raw_haystack;
		int needle;
		size_t maxlen;
	} args;
	union pointer haystack;
	DeeArg_UnpackStruct3X(err, argc, argv, "strnoff", &args, &args.raw_haystack, "o", _DeeArg_AsObject, &args.needle, "d", DeeObject_AsInt, &args.maxlen, UNPuSIZ, DeeObject_AsSize);
	if unlikely(DeeObject_AsPointer(args.raw_haystack, &DeeCChar_Type, &haystack))
		goto err;
	return c_string_strnoff_f_impl(haystack.pcchar, args.needle, args.maxlen);
err:
	return NULL;
}
INTERN DEFINE_CMETHOD(c_string_strnoff, &c_string_strnoff_f, METHOD_FPURECALL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_strnoff_f_impl(char const *haystack, int needle, size_t maxlen)
{
	char const *result;
	CTYPES_FAULTPROTECT(result = strnchrnul(haystack, needle, maxlen), return NULL);
	return DeeInt_NewSize((size_t)(result - haystack));
}

/* strnroff */
#define c_string_strnroff_params "haystack:?Aptr?Gchar,needle:?Dint,maxlen:?Dint"
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_strnroff_f_impl(char const *haystack, int needle, size_t maxlen);
PRIVATE WUNUSED DREF DeeObject *DCALL c_string_strnroff_f(size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *raw_haystack;
		int needle;
		size_t maxlen;
	} args;
	union pointer haystack;
	DeeArg_UnpackStruct3X(err, argc, argv, "strnroff", &args, &args.raw_haystack, "o", _DeeArg_AsObject, &args.needle, "d", DeeObject_AsInt, &args.maxlen, UNPuSIZ, DeeObject_AsSize);
	if unlikely(DeeObject_AsPointer(args.raw_haystack, &DeeCChar_Type, &haystack))
		goto err;
	return c_string_strnroff_f_impl(haystack.pcchar, args.needle, args.maxlen);
err:
	return NULL;
}
INTERN DEFINE_CMETHOD(c_string_strnroff, &c_string_strnroff_f, METHOD_FPURECALL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_strnroff_f_impl(char const *haystack, int needle, size_t maxlen)
{
	char const *result;
	CTYPES_FAULTPROTECT(result = strnrchrnul(haystack, needle, maxlen), return NULL);
	return DeeInt_NewSize((size_t)(result - haystack));
}

/* strcmp */
#define c_string_strcmp_params "lhs:?Aptr?Gchar,rhs:?Aptr?Gchar"
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_strcmp_f_impl(char const *lhs, char const *rhs);
PRIVATE WUNUSED DREF DeeObject *DCALL c_string_strcmp_f(size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *raw_lhs;
		DeeObject *raw_rhs;
	} args;
	union pointer lhs;
	union pointer rhs;
	DeeArg_UnpackStruct2(err, argc, argv, "strcmp", &args, &args.raw_lhs, &args.raw_rhs);
	if unlikely(DeeObject_AsPointer(args.raw_lhs, &DeeCChar_Type, &lhs))
		goto err;
	if unlikely(DeeObject_AsPointer(args.raw_rhs, &DeeCChar_Type, &rhs))
		goto err;
	return c_string_strcmp_f_impl(lhs.pcchar, rhs.pcchar);
err:
	return NULL;
}
INTERN DEFINE_CMETHOD(c_string_strcmp, &c_string_strcmp_f, METHOD_FPURECALL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_strcmp_f_impl(char const *lhs, char const *rhs)
{
	int result;
	CTYPES_FAULTPROTECT(result = strcmp(lhs, rhs), return NULL);
	if (result < -1)
		result = -1;
	if (result > 1)
		result = 1;
	Dee_return_smallint(result);
}

/* strncmp */
#define c_string_strncmp_params "lhs:?Aptr?Gchar,rhs:?Aptr?Gchar,maxlen:?Dint"
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_strncmp_f_impl(char const *lhs, char const *rhs, size_t maxlen);
PRIVATE WUNUSED DREF DeeObject *DCALL c_string_strncmp_f(size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *raw_lhs;
		DeeObject *raw_rhs;
		size_t maxlen;
	} args;
	union pointer lhs;
	union pointer rhs;
	DeeArg_UnpackStruct3X(err, argc, argv, "strncmp", &args, &args.raw_lhs, "o", _DeeArg_AsObject, &args.raw_rhs, "o", _DeeArg_AsObject, &args.maxlen, UNPuSIZ, DeeObject_AsSize);
	if unlikely(DeeObject_AsPointer(args.raw_lhs, &DeeCChar_Type, &lhs))
		goto err;
	if unlikely(DeeObject_AsPointer(args.raw_rhs, &DeeCChar_Type, &rhs))
		goto err;
	return c_string_strncmp_f_impl(lhs.pcchar, rhs.pcchar, args.maxlen);
err:
	return NULL;
}
INTERN DEFINE_CMETHOD(c_string_strncmp, &c_string_strncmp_f, METHOD_FPURECALL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_strncmp_f_impl(char const *lhs, char const *rhs, size_t maxlen)
{
	int result;
	CTYPES_FAULTPROTECT(result = strncmp(lhs, rhs, maxlen), return NULL);
	if (result < -1)
		result = -1;
	if (result > 1)
		result = 1;
	Dee_return_smallint(result);
}

/* strcasecmp */
#define c_string_strcasecmp_params "lhs:?Aptr?Gchar,rhs:?Aptr?Gchar"
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_strcasecmp_f_impl(char const *lhs, char const *rhs);
PRIVATE WUNUSED DREF DeeObject *DCALL c_string_strcasecmp_f(size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *raw_lhs;
		DeeObject *raw_rhs;
	} args;
	union pointer lhs;
	union pointer rhs;
	DeeArg_UnpackStruct2(err, argc, argv, "strcasecmp", &args, &args.raw_lhs, &args.raw_rhs);
	if unlikely(DeeObject_AsPointer(args.raw_lhs, &DeeCChar_Type, &lhs))
		goto err;
	if unlikely(DeeObject_AsPointer(args.raw_rhs, &DeeCChar_Type, &rhs))
		goto err;
	return c_string_strcasecmp_f_impl(lhs.pcchar, rhs.pcchar);
err:
	return NULL;
}
INTERN DEFINE_CMETHOD(c_string_strcasecmp, &c_string_strcasecmp_f, METHOD_FPURECALL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_strcasecmp_f_impl(char const *lhs, char const *rhs)
{
	int result;
	CTYPES_FAULTPROTECT(result = strcasecmp(lhs, rhs), return NULL);
	if (result < -1)
		result = -1;
	if (result > 1)
		result = 1;
	Dee_return_smallint(result);
}

/* strncasecmp */
#define c_string_strncasecmp_params "lhs:?Aptr?Gchar,rhs:?Aptr?Gchar,maxlen:?Dint"
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_strncasecmp_f_impl(char const *lhs, char const *rhs, size_t maxlen);
PRIVATE WUNUSED DREF DeeObject *DCALL c_string_strncasecmp_f(size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *raw_lhs;
		DeeObject *raw_rhs;
		size_t maxlen;
	} args;
	union pointer lhs;
	union pointer rhs;
	DeeArg_UnpackStruct3X(err, argc, argv, "strncasecmp", &args, &args.raw_lhs, "o", _DeeArg_AsObject, &args.raw_rhs, "o", _DeeArg_AsObject, &args.maxlen, UNPuSIZ, DeeObject_AsSize);
	if unlikely(DeeObject_AsPointer(args.raw_lhs, &DeeCChar_Type, &lhs))
		goto err;
	if unlikely(DeeObject_AsPointer(args.raw_rhs, &DeeCChar_Type, &rhs))
		goto err;
	return c_string_strncasecmp_f_impl(lhs.pcchar, rhs.pcchar, args.maxlen);
err:
	return NULL;
}
INTERN DEFINE_CMETHOD(c_string_strncasecmp, &c_string_strncasecmp_f, METHOD_FPURECALL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_strncasecmp_f_impl(char const *lhs, char const *rhs, size_t maxlen)
{
	int result;
	CTYPES_FAULTPROTECT(result = strncasecmp(lhs, rhs, maxlen), return NULL);
	if (result < -1)
		result = -1;
	if (result > 1)
		result = 1;
	Dee_return_smallint(result);
}

/* strverscmp */
#define c_string_strverscmp_params "lhs:?Aptr?Gchar,rhs:?Aptr?Gchar"
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_strverscmp_f_impl(char const *lhs, char const *rhs);
PRIVATE WUNUSED DREF DeeObject *DCALL c_string_strverscmp_f(size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *raw_lhs;
		DeeObject *raw_rhs;
	} args;
	union pointer lhs;
	union pointer rhs;
	DeeArg_UnpackStruct2(err, argc, argv, "strverscmp", &args, &args.raw_lhs, &args.raw_rhs);
	if unlikely(DeeObject_AsPointer(args.raw_lhs, &DeeCChar_Type, &lhs))
		goto err;
	if unlikely(DeeObject_AsPointer(args.raw_rhs, &DeeCChar_Type, &rhs))
		goto err;
	return c_string_strverscmp_f_impl(lhs.pcchar, rhs.pcchar);
err:
	return NULL;
}
INTERN DEFINE_CMETHOD(c_string_strverscmp, &c_string_strverscmp_f, METHOD_FPURECALL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_strverscmp_f_impl(char const *lhs, char const *rhs)
{
	int result;
	CTYPES_FAULTPROTECT(result = strverscmp(lhs, rhs), return NULL);
	if (result < -1)
		result = -1;
	if (result > 1)
		result = 1;
	Dee_return_smallint(result);
}

/* strcpy */
#define c_string_strcpy_params "dst:?Aptr?Gchar,src:?Aptr?Gchar"
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_strcpy_f_impl(char *dst, char const *src);
PRIVATE WUNUSED DREF DeeObject *DCALL c_string_strcpy_f(size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *raw_dst;
		DeeObject *raw_src;
	} args;
	union pointer dst;
	union pointer src;
	DeeArg_UnpackStruct2(err, argc, argv, "strcpy", &args, &args.raw_dst, &args.raw_src);
	if unlikely(DeeObject_AsPointer(args.raw_dst, &DeeCChar_Type, &dst))
		goto err;
	if unlikely(DeeObject_AsPointer(args.raw_src, &DeeCChar_Type, &src))
		goto err;
	return c_string_strcpy_f_impl(dst.pchar, src.pcchar);
err:
	return NULL;
}
INTERN DEFINE_CMETHOD(c_string_strcpy, &c_string_strcpy_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_strcpy_f_impl(char *dst, char const *src)
{
	CTYPES_FAULTPROTECT(dst = strcpy(dst, src), return NULL);
	return DeePointer_NewFor(&DeeCChar_Type, (void *)dst);
}

/* strcat */
#define c_string_strcat_params "dst:?Aptr?Gchar,src:?Aptr?Gchar"
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_strcat_f_impl(char *dst, char const *src);
PRIVATE WUNUSED DREF DeeObject *DCALL c_string_strcat_f(size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *raw_dst;
		DeeObject *raw_src;
	} args;
	union pointer dst;
	union pointer src;
	DeeArg_UnpackStruct2(err, argc, argv, "strcat", &args, &args.raw_dst, &args.raw_src);
	if unlikely(DeeObject_AsPointer(args.raw_dst, &DeeCChar_Type, &dst))
		goto err;
	if unlikely(DeeObject_AsPointer(args.raw_src, &DeeCChar_Type, &src))
		goto err;
	return c_string_strcat_f_impl(dst.pchar, src.pcchar);
err:
	return NULL;
}
INTERN DEFINE_CMETHOD(c_string_strcat, &c_string_strcat_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_strcat_f_impl(char *dst, char const *src)
{
	CTYPES_FAULTPROTECT(dst = strcat(dst, src), return NULL);
	return DeePointer_NewFor(&DeeCChar_Type, (void *)dst);
}

/* stpcpy */
#define c_string_stpcpy_params "dst:?Aptr?Gchar,src:?Aptr?Gchar"
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_stpcpy_f_impl(char *dst, char const *src);
PRIVATE WUNUSED DREF DeeObject *DCALL c_string_stpcpy_f(size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *raw_dst;
		DeeObject *raw_src;
	} args;
	union pointer dst;
	union pointer src;
	DeeArg_UnpackStruct2(err, argc, argv, "stpcpy", &args, &args.raw_dst, &args.raw_src);
	if unlikely(DeeObject_AsPointer(args.raw_dst, &DeeCChar_Type, &dst))
		goto err;
	if unlikely(DeeObject_AsPointer(args.raw_src, &DeeCChar_Type, &src))
		goto err;
	return c_string_stpcpy_f_impl(dst.pchar, src.pcchar);
err:
	return NULL;
}
INTERN DEFINE_CMETHOD(c_string_stpcpy, &c_string_stpcpy_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_stpcpy_f_impl(char *dst, char const *src)
{
	CTYPES_FAULTPROTECT(dst = stpcpy(dst, src), return NULL);
	return DeePointer_NewFor(&DeeCChar_Type, (void *)dst);
}

/* strncpy */
#define c_string_strncpy_params "dst:?Aptr?Gchar,src:?Aptr?Gchar,maxlen:?Dint"
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_strncpy_f_impl(char *dst, char const *src, size_t maxlen);
PRIVATE WUNUSED DREF DeeObject *DCALL c_string_strncpy_f(size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *raw_dst;
		DeeObject *raw_src;
		size_t maxlen;
	} args;
	union pointer dst;
	union pointer src;
	DeeArg_UnpackStruct3X(err, argc, argv, "strncpy", &args, &args.raw_dst, "o", _DeeArg_AsObject, &args.raw_src, "o", _DeeArg_AsObject, &args.maxlen, UNPuSIZ, DeeObject_AsSize);
	if unlikely(DeeObject_AsPointer(args.raw_dst, &DeeCChar_Type, &dst))
		goto err;
	if unlikely(DeeObject_AsPointer(args.raw_src, &DeeCChar_Type, &src))
		goto err;
	return c_string_strncpy_f_impl(dst.pchar, src.pcchar, args.maxlen);
err:
	return NULL;
}
INTERN DEFINE_CMETHOD(c_string_strncpy, &c_string_strncpy_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_strncpy_f_impl(char *dst, char const *src, size_t maxlen)
{
	CTYPES_FAULTPROTECT(dst = strncpy(dst, src, maxlen), return NULL);
	return DeePointer_NewFor(&DeeCChar_Type, (void *)dst);
}

/* strncat */
#define c_string_strncat_params "dst:?Aptr?Gchar,src:?Aptr?Gchar,maxlen:?Dint"
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_strncat_f_impl(char *dst, char const *src, size_t maxlen);
PRIVATE WUNUSED DREF DeeObject *DCALL c_string_strncat_f(size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *raw_dst;
		DeeObject *raw_src;
		size_t maxlen;
	} args;
	union pointer dst;
	union pointer src;
	DeeArg_UnpackStruct3X(err, argc, argv, "strncat", &args, &args.raw_dst, "o", _DeeArg_AsObject, &args.raw_src, "o", _DeeArg_AsObject, &args.maxlen, UNPuSIZ, DeeObject_AsSize);
	if unlikely(DeeObject_AsPointer(args.raw_dst, &DeeCChar_Type, &dst))
		goto err;
	if unlikely(DeeObject_AsPointer(args.raw_src, &DeeCChar_Type, &src))
		goto err;
	return c_string_strncat_f_impl(dst.pchar, src.pcchar, args.maxlen);
err:
	return NULL;
}
INTERN DEFINE_CMETHOD(c_string_strncat, &c_string_strncat_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_strncat_f_impl(char *dst, char const *src, size_t maxlen)
{
	CTYPES_FAULTPROTECT(dst = strncat(dst, src, maxlen), return NULL);
	return DeePointer_NewFor(&DeeCChar_Type, (void *)dst);
}

/* stpncpy */
#define c_string_stpncpy_params "dst:?Aptr?Gchar,src:?Aptr?Gchar,maxlen:?Dint"
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_stpncpy_f_impl(char *dst, char const *src, size_t maxlen);
PRIVATE WUNUSED DREF DeeObject *DCALL c_string_stpncpy_f(size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *raw_dst;
		DeeObject *raw_src;
		size_t maxlen;
	} args;
	union pointer dst;
	union pointer src;
	DeeArg_UnpackStruct3X(err, argc, argv, "stpncpy", &args, &args.raw_dst, "o", _DeeArg_AsObject, &args.raw_src, "o", _DeeArg_AsObject, &args.maxlen, UNPuSIZ, DeeObject_AsSize);
	if unlikely(DeeObject_AsPointer(args.raw_dst, &DeeCChar_Type, &dst))
		goto err;
	if unlikely(DeeObject_AsPointer(args.raw_src, &DeeCChar_Type, &src))
		goto err;
	return c_string_stpncpy_f_impl(dst.pchar, src.pcchar, args.maxlen);
err:
	return NULL;
}
INTERN DEFINE_CMETHOD(c_string_stpncpy, &c_string_stpncpy_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_stpncpy_f_impl(char *dst, char const *src, size_t maxlen)
{
	CTYPES_FAULTPROTECT(dst = stpncpy(dst, src, maxlen), return NULL);
	return DeePointer_NewFor(&DeeCChar_Type, (void *)dst);
}

/* strstr */
#define c_string_strstr_params "haystack:?Aptr?Gchar,needle:?Aptr?Gchar"
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_strstr_f_impl(char const *haystack, char const *needle);
PRIVATE WUNUSED DREF DeeObject *DCALL c_string_strstr_f(size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *raw_haystack;
		DeeObject *raw_needle;
	} args;
	union pointer haystack;
	union pointer needle;
	DeeArg_UnpackStruct2(err, argc, argv, "strstr", &args, &args.raw_haystack, &args.raw_needle);
	if unlikely(DeeObject_AsPointer(args.raw_haystack, &DeeCChar_Type, &haystack))
		goto err;
	if unlikely(DeeObject_AsPointer(args.raw_needle, &DeeCChar_Type, &needle))
		goto err;
	return c_string_strstr_f_impl(haystack.pcchar, needle.pcchar);
err:
	return NULL;
}
INTERN DEFINE_CMETHOD(c_string_strstr, &c_string_strstr_f, METHOD_FPURECALL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_strstr_f_impl(char const *haystack, char const *needle)
{
	char const *result;
	CTYPES_FAULTPROTECT(result = strstr(haystack, needle), return NULL);
	return DeePointer_NewFor(&DeeCChar_Type, (void *)result);
}

/* strnstr */
#define c_string_strnstr_params "haystack:?Aptr?Gchar,needle:?Aptr?Gchar,haystack_maxlen:?Dint"
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_strnstr_f_impl(char const *haystack, char const *needle, size_t haystack_maxlen);
PRIVATE WUNUSED DREF DeeObject *DCALL c_string_strnstr_f(size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *raw_haystack;
		DeeObject *raw_needle;
		size_t haystack_maxlen;
	} args;
	union pointer haystack;
	union pointer needle;
	DeeArg_UnpackStruct3X(err, argc, argv, "strnstr", &args, &args.raw_haystack, "o", _DeeArg_AsObject, &args.raw_needle, "o", _DeeArg_AsObject, &args.haystack_maxlen, UNPuSIZ, DeeObject_AsSize);
	if unlikely(DeeObject_AsPointer(args.raw_haystack, &DeeCChar_Type, &haystack))
		goto err;
	if unlikely(DeeObject_AsPointer(args.raw_needle, &DeeCChar_Type, &needle))
		goto err;
	return c_string_strnstr_f_impl(haystack.pcchar, needle.pcchar, args.haystack_maxlen);
err:
	return NULL;
}
INTERN DEFINE_CMETHOD(c_string_strnstr, &c_string_strnstr_f, METHOD_FPURECALL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_strnstr_f_impl(char const *haystack, char const *needle, size_t haystack_maxlen)
{
	char const *result;
	CTYPES_FAULTPROTECT(result = strnstr(haystack, needle, haystack_maxlen), return NULL);
	return DeePointer_NewFor(&DeeCChar_Type, (void *)result);
}

/* strcasestr */
#define c_string_strcasestr_params "haystack:?Aptr?Gchar,needle:?Aptr?Gchar"
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_strcasestr_f_impl(char const *haystack, char const *needle);
PRIVATE WUNUSED DREF DeeObject *DCALL c_string_strcasestr_f(size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *raw_haystack;
		DeeObject *raw_needle;
	} args;
	union pointer haystack;
	union pointer needle;
	DeeArg_UnpackStruct2(err, argc, argv, "strcasestr", &args, &args.raw_haystack, &args.raw_needle);
	if unlikely(DeeObject_AsPointer(args.raw_haystack, &DeeCChar_Type, &haystack))
		goto err;
	if unlikely(DeeObject_AsPointer(args.raw_needle, &DeeCChar_Type, &needle))
		goto err;
	return c_string_strcasestr_f_impl(haystack.pcchar, needle.pcchar);
err:
	return NULL;
}
INTERN DEFINE_CMETHOD(c_string_strcasestr, &c_string_strcasestr_f, METHOD_FPURECALL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_strcasestr_f_impl(char const *haystack, char const *needle)
{
	char const *result;
	CTYPES_FAULTPROTECT(result = strcasestr(haystack, needle), return NULL);
	return DeePointer_NewFor(&DeeCChar_Type, (void *)result);
}

/* strncasestr */
#define c_string_strncasestr_params "haystack:?Aptr?Gchar,needle:?Aptr?Gchar,haystack_maxlen:?Dint"
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_strncasestr_f_impl(char const *haystack, char const *needle, size_t haystack_maxlen);
PRIVATE WUNUSED DREF DeeObject *DCALL c_string_strncasestr_f(size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *raw_haystack;
		DeeObject *raw_needle;
		size_t haystack_maxlen;
	} args;
	union pointer haystack;
	union pointer needle;
	DeeArg_UnpackStruct3X(err, argc, argv, "strncasestr", &args, &args.raw_haystack, "o", _DeeArg_AsObject, &args.raw_needle, "o", _DeeArg_AsObject, &args.haystack_maxlen, UNPuSIZ, DeeObject_AsSize);
	if unlikely(DeeObject_AsPointer(args.raw_haystack, &DeeCChar_Type, &haystack))
		goto err;
	if unlikely(DeeObject_AsPointer(args.raw_needle, &DeeCChar_Type, &needle))
		goto err;
	return c_string_strncasestr_f_impl(haystack.pcchar, needle.pcchar, args.haystack_maxlen);
err:
	return NULL;
}
INTERN DEFINE_CMETHOD(c_string_strncasestr, &c_string_strncasestr_f, METHOD_FPURECALL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_strncasestr_f_impl(char const *haystack, char const *needle, size_t haystack_maxlen)
{
	char const *result;
	CTYPES_FAULTPROTECT(result = strncasestr(haystack, needle, haystack_maxlen), return NULL);
	return DeePointer_NewFor(&DeeCChar_Type, (void *)result);
}

/* strspn */
#define c_string_strspn_params "haystack:?Aptr?Gchar,accept:?Aptr?Gchar"
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_strspn_f_impl(char const *haystack, char const *accept);
PRIVATE WUNUSED DREF DeeObject *DCALL c_string_strspn_f(size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *raw_haystack;
		DeeObject *raw_accept;
	} args;
	union pointer haystack;
	union pointer accept;
	DeeArg_UnpackStruct2(err, argc, argv, "strspn", &args, &args.raw_haystack, &args.raw_accept);
	if unlikely(DeeObject_AsPointer(args.raw_haystack, &DeeCChar_Type, &haystack))
		goto err;
	if unlikely(DeeObject_AsPointer(args.raw_accept, &DeeCChar_Type, &accept))
		goto err;
	return c_string_strspn_f_impl(haystack.pcchar, accept.pcchar);
err:
	return NULL;
}
INTERN DEFINE_CMETHOD(c_string_strspn, &c_string_strspn_f, METHOD_FPURECALL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_strspn_f_impl(char const *haystack, char const *accept)
{
	size_t result;
	CTYPES_FAULTPROTECT(result = strspn(haystack, accept), return NULL);
	return DeeInt_NewSize(result);
}

/* strcspn */
#define c_string_strcspn_params "haystack:?Aptr?Gchar,accept:?Aptr?Gchar"
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_strcspn_f_impl(char const *haystack, char const *accept);
PRIVATE WUNUSED DREF DeeObject *DCALL c_string_strcspn_f(size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *raw_haystack;
		DeeObject *raw_accept;
	} args;
	union pointer haystack;
	union pointer accept;
	DeeArg_UnpackStruct2(err, argc, argv, "strcspn", &args, &args.raw_haystack, &args.raw_accept);
	if unlikely(DeeObject_AsPointer(args.raw_haystack, &DeeCChar_Type, &haystack))
		goto err;
	if unlikely(DeeObject_AsPointer(args.raw_accept, &DeeCChar_Type, &accept))
		goto err;
	return c_string_strcspn_f_impl(haystack.pcchar, accept.pcchar);
err:
	return NULL;
}
INTERN DEFINE_CMETHOD(c_string_strcspn, &c_string_strcspn_f, METHOD_FPURECALL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_strcspn_f_impl(char const *haystack, char const *accept)
{
	size_t result;
	CTYPES_FAULTPROTECT(result = strcspn(haystack, accept), return NULL);
	return DeeInt_NewSize(result);
}

/* strpbrk */
#define c_string_strpbrk_params "haystack:?Aptr?Gchar,accept:?Aptr?Gchar"
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_strpbrk_f_impl(char const *haystack, char const *accept);
PRIVATE WUNUSED DREF DeeObject *DCALL c_string_strpbrk_f(size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *raw_haystack;
		DeeObject *raw_accept;
	} args;
	union pointer haystack;
	union pointer accept;
	DeeArg_UnpackStruct2(err, argc, argv, "strpbrk", &args, &args.raw_haystack, &args.raw_accept);
	if unlikely(DeeObject_AsPointer(args.raw_haystack, &DeeCChar_Type, &haystack))
		goto err;
	if unlikely(DeeObject_AsPointer(args.raw_accept, &DeeCChar_Type, &accept))
		goto err;
	return c_string_strpbrk_f_impl(haystack.pcchar, accept.pcchar);
err:
	return NULL;
}
INTERN DEFINE_CMETHOD(c_string_strpbrk, &c_string_strpbrk_f, METHOD_FPURECALL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_strpbrk_f_impl(char const *haystack, char const *accept)
{
	char const *result;
	CTYPES_FAULTPROTECT(result = strpbrk(haystack, accept), return NULL);
	return DeePointer_NewFor(&DeeCChar_Type, (void *)result);
}

/* basename */
#define c_string_basename_params "str:?Aptr?Gchar"
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_basename_f_impl(char const *str);
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL c_string_basename_f(DeeObject *__restrict arg0) {
	union pointer str;
	if unlikely(DeeObject_AsPointer(arg0, &DeeCChar_Type, &str))
		goto err;
	return c_string_basename_f_impl(str.pcchar);
err:
	return NULL;
}
INTERN DEFINE_CMETHOD1(c_string_basename, &c_string_basename_f, METHOD_FPURECALL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_basename_f_impl(char const *str)
{
	CTYPES_FAULTPROTECT(str = basename(str), return NULL);
	return DeePointer_NewFor(&DeeCChar_Type, (void *)str);
}

/* strrev */
#define c_string_strrev_params "str:?Aptr?Gchar"
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_strrev_f_impl(char *str);
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL c_string_strrev_f(DeeObject *__restrict arg0) {
	union pointer str;
	if unlikely(DeeObject_AsPointer(arg0, &DeeCChar_Type, &str))
		goto err;
	return c_string_strrev_f_impl(str.pchar);
err:
	return NULL;
}
INTERN DEFINE_CMETHOD1(c_string_strrev, &c_string_strrev_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_strrev_f_impl(char *str)
{
	CTYPES_FAULTPROTECT(str = strrev(str), return NULL);
	return DeePointer_NewFor(&DeeCChar_Type, (void *)str);
}

/* strnrev */
#define c_string_strnrev_params "str:?Aptr?Gchar,maxlen:?Dint"
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_strnrev_f_impl(char *str, size_t maxlen);
PRIVATE WUNUSED DREF DeeObject *DCALL c_string_strnrev_f(size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *raw_str;
		size_t maxlen;
	} args;
	union pointer str;
	DeeArg_UnpackStruct2X(err, argc, argv, "strnrev", &args, &args.raw_str, "o", _DeeArg_AsObject, &args.maxlen, UNPuSIZ, DeeObject_AsSize);
	if unlikely(DeeObject_AsPointer(args.raw_str, &DeeCChar_Type, &str))
		goto err;
	return c_string_strnrev_f_impl(str.pchar, args.maxlen);
err:
	return NULL;
}
INTERN DEFINE_CMETHOD(c_string_strnrev, &c_string_strnrev_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_strnrev_f_impl(char *str, size_t maxlen)
{
	CTYPES_FAULTPROTECT(str = strnrev(str, maxlen), return NULL);
	return DeePointer_NewFor(&DeeCChar_Type, (void *)str);
}

/* strlwr */
#define c_string_strlwr_params "str:?Aptr?Gchar"
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_strlwr_f_impl(char *str);
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL c_string_strlwr_f(DeeObject *__restrict arg0) {
	union pointer str;
	if unlikely(DeeObject_AsPointer(arg0, &DeeCChar_Type, &str))
		goto err;
	return c_string_strlwr_f_impl(str.pchar);
err:
	return NULL;
}
INTERN DEFINE_CMETHOD1(c_string_strlwr, &c_string_strlwr_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_strlwr_f_impl(char *str)
{
	CTYPES_FAULTPROTECT(str = strlwr(str), return NULL);
	return DeePointer_NewFor(&DeeCChar_Type, (void *)str);
}

/* strnlwr */
#define c_string_strnlwr_params "str:?Aptr?Gchar,maxlen:?Dint"
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_strnlwr_f_impl(char *str, size_t maxlen);
PRIVATE WUNUSED DREF DeeObject *DCALL c_string_strnlwr_f(size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *raw_str;
		size_t maxlen;
	} args;
	union pointer str;
	DeeArg_UnpackStruct2X(err, argc, argv, "strnlwr", &args, &args.raw_str, "o", _DeeArg_AsObject, &args.maxlen, UNPuSIZ, DeeObject_AsSize);
	if unlikely(DeeObject_AsPointer(args.raw_str, &DeeCChar_Type, &str))
		goto err;
	return c_string_strnlwr_f_impl(str.pchar, args.maxlen);
err:
	return NULL;
}
INTERN DEFINE_CMETHOD(c_string_strnlwr, &c_string_strnlwr_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_strnlwr_f_impl(char *str, size_t maxlen)
{
	CTYPES_FAULTPROTECT(str = strnlwr(str, maxlen), return NULL);
	return DeePointer_NewFor(&DeeCChar_Type, (void *)str);
}

/* strupr */
#define c_string_strupr_params "str:?Aptr?Gchar"
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_strupr_f_impl(char *str);
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL c_string_strupr_f(DeeObject *__restrict arg0) {
	union pointer str;
	if unlikely(DeeObject_AsPointer(arg0, &DeeCChar_Type, &str))
		goto err;
	return c_string_strupr_f_impl(str.pchar);
err:
	return NULL;
}
INTERN DEFINE_CMETHOD1(c_string_strupr, &c_string_strupr_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_strupr_f_impl(char *str)
{
	CTYPES_FAULTPROTECT(str = strupr(str), return NULL);
	return DeePointer_NewFor(&DeeCChar_Type, (void *)str);
}

/* strnupr */
#define c_string_strnupr_params "str:?Aptr?Gchar,maxlen:?Dint"
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_strnupr_f_impl(char *str, size_t maxlen);
PRIVATE WUNUSED DREF DeeObject *DCALL c_string_strnupr_f(size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *raw_str;
		size_t maxlen;
	} args;
	union pointer str;
	DeeArg_UnpackStruct2X(err, argc, argv, "strnupr", &args, &args.raw_str, "o", _DeeArg_AsObject, &args.maxlen, UNPuSIZ, DeeObject_AsSize);
	if unlikely(DeeObject_AsPointer(args.raw_str, &DeeCChar_Type, &str))
		goto err;
	return c_string_strnupr_f_impl(str.pchar, args.maxlen);
err:
	return NULL;
}
INTERN DEFINE_CMETHOD(c_string_strnupr, &c_string_strnupr_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_strnupr_f_impl(char *str, size_t maxlen)
{
	CTYPES_FAULTPROTECT(str = strnupr(str, maxlen), return NULL);
	return DeePointer_NewFor(&DeeCChar_Type, (void *)str);
}

/* strset */
#define c_string_strset_params "str:?Aptr?Gchar,ch:?Dint"
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_strset_f_impl(char *str, int ch);
PRIVATE WUNUSED DREF DeeObject *DCALL c_string_strset_f(size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *raw_str;
		int ch;
	} args;
	union pointer str;
	DeeArg_UnpackStruct2X(err, argc, argv, "strset", &args, &args.raw_str, "o", _DeeArg_AsObject, &args.ch, "d", DeeObject_AsInt);
	if unlikely(DeeObject_AsPointer(args.raw_str, &DeeCChar_Type, &str))
		goto err;
	return c_string_strset_f_impl(str.pchar, args.ch);
err:
	return NULL;
}
INTERN DEFINE_CMETHOD(c_string_strset, &c_string_strset_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_strset_f_impl(char *str, int ch)
{
	CTYPES_FAULTPROTECT(str = strset(str, ch), return NULL);
	return DeePointer_NewFor(&DeeCChar_Type, (void *)str);
}

/* strnset */
#define c_string_strnset_params "str:?Aptr?Gchar,ch:?Dint,maxlen:?Dint"
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_strnset_f_impl(char *str, int ch, size_t maxlen);
PRIVATE WUNUSED DREF DeeObject *DCALL c_string_strnset_f(size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *raw_str;
		int ch;
		size_t maxlen;
	} args;
	union pointer str;
	DeeArg_UnpackStruct3X(err, argc, argv, "strnset", &args, &args.raw_str, "o", _DeeArg_AsObject, &args.ch, "d", DeeObject_AsInt, &args.maxlen, UNPuSIZ, DeeObject_AsSize);
	if unlikely(DeeObject_AsPointer(args.raw_str, &DeeCChar_Type, &str))
		goto err;
	return c_string_strnset_f_impl(str.pchar, args.ch, args.maxlen);
err:
	return NULL;
}
INTERN DEFINE_CMETHOD(c_string_strnset, &c_string_strnset_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_strnset_f_impl(char *str, int ch, size_t maxlen)
{
	CTYPES_FAULTPROTECT(str = strnset(str, ch, maxlen), return NULL);
	return DeePointer_NewFor(&DeeCChar_Type, (void *)str);
}

/* memfrob */
#define c_string_memfrob_params "buf:?Aptr?Gvoid,num_bytes:?Dint"
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_memfrob_f_impl(void *buf, size_t num_bytes);
PRIVATE WUNUSED DREF DeeObject *DCALL c_string_memfrob_f(size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *raw_buf;
		size_t num_bytes;
	} args;
	union pointer buf;
	DeeArg_UnpackStruct2X(err, argc, argv, "memfrob", &args, &args.raw_buf, "o", _DeeArg_AsObject, &args.num_bytes, UNPuSIZ, DeeObject_AsSize);
	if unlikely(DeeObject_AsPointer(args.raw_buf, &DeeCVoid_Type, &buf))
		goto err;
	return c_string_memfrob_f_impl(buf.pvoid, args.num_bytes);
err:
	return NULL;
}
INTERN DEFINE_CMETHOD(c_string_memfrob, &c_string_memfrob_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_memfrob_f_impl(void *buf, size_t num_bytes)
{
	CTYPES_FAULTPROTECT(buf = memfrob(buf, num_bytes), return NULL);
	return DeePointer_NewVoid(buf);
}

/* strsep */
#define c_string_strsep_params "stringp:?Aptr?Aptr?Gchar,delim:?Aptr?Gchar"
FORCELOCAL WUNUSED NONNULL((1)) DREF DeeObject *DCALL c_string_strsep_f_impl(char **stringp, char const *delim);
PRIVATE WUNUSED DREF DeeObject *DCALL c_string_strsep_f(size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *raw_stringp;
		DeeObject *raw_delim;
	} args;
	void **stringp;
	union pointer delim;
	DeeArg_UnpackStruct2(err, argc, argv, "strsep", &args, &args.raw_stringp, &args.raw_delim);
	if unlikely(DeeObject_AsPPointer(args.raw_stringp, &DeeCChar_Type, &stringp))
		goto err;
	if unlikely(DeeObject_AsPointer(args.raw_delim, &DeeCChar_Type, &delim))
		goto err;
	return c_string_strsep_f_impl((char **)stringp, delim.pcchar);
err:
	return NULL;
}
INTERN DEFINE_CMETHOD(c_string_strsep, &c_string_strsep_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED NONNULL((1)) DREF DeeObject *DCALL c_string_strsep_f_impl(char **stringp, char const *delim)
{
	char *result;
	CTYPES_FAULTPROTECT(result = strsep(stringp, delim), return NULL);
	return DeePointer_NewFor(&DeeCChar_Type, (void *)result);
}

/* stresep */
#define c_string_stresep_params "stringp:?Aptr?Aptr?Gchar,delim:?Aptr?Gchar,escape:?Dint"
FORCELOCAL WUNUSED NONNULL((1)) DREF DeeObject *DCALL c_string_stresep_f_impl(char **stringp, char const *delim, int escape);
PRIVATE WUNUSED DREF DeeObject *DCALL c_string_stresep_f(size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *raw_stringp;
		DeeObject *raw_delim;
		int escape;
	} args;
	void **stringp;
	union pointer delim;
	DeeArg_UnpackStruct3X(err, argc, argv, "stresep", &args, &args.raw_stringp, "o", _DeeArg_AsObject, &args.raw_delim, "o", _DeeArg_AsObject, &args.escape, "d", DeeObject_AsInt);
	if unlikely(DeeObject_AsPPointer(args.raw_stringp, &DeeCChar_Type, &stringp))
		goto err;
	if unlikely(DeeObject_AsPointer(args.raw_delim, &DeeCChar_Type, &delim))
		goto err;
	return c_string_stresep_f_impl((char **)stringp, delim.pcchar, args.escape);
err:
	return NULL;
}
INTERN DEFINE_CMETHOD(c_string_stresep, &c_string_stresep_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED NONNULL((1)) DREF DeeObject *DCALL c_string_stresep_f_impl(char **stringp, char const *delim, int escape)
{
	char *result;
	CTYPES_FAULTPROTECT(result = stresep(stringp, delim, escape), return NULL);
	return DeePointer_NewFor(&DeeCChar_Type, (void *)result);
}

/* strtok */
#define c_string_strtok_params "str:?Aptr?Gchar,delim:?Aptr?Gchar"
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_strtok_f_impl(char *str, char const *delim);
PRIVATE WUNUSED DREF DeeObject *DCALL c_string_strtok_f(size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *raw_str;
		DeeObject *raw_delim;
	} args;
	union pointer str;
	union pointer delim;
	DeeArg_UnpackStruct2(err, argc, argv, "strtok", &args, &args.raw_str, &args.raw_delim);
	if unlikely(DeeObject_AsPointer(args.raw_str, &DeeCChar_Type, &str))
		goto err;
	if unlikely(DeeObject_AsPointer(args.raw_delim, &DeeCChar_Type, &delim))
		goto err;
	return c_string_strtok_f_impl(str.pchar, delim.pcchar);
err:
	return NULL;
}
INTERN DEFINE_CMETHOD(c_string_strtok, &c_string_strtok_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED DREF DeeObject *DCALL c_string_strtok_f_impl(char *str, char const *delim)
{
	char *result;
	CTYPES_FAULTPROTECT(result = strtok(str, delim), return NULL);
	return DeePointer_NewFor(&DeeCChar_Type, (void *)result);
}

/* strtok_r */
#define c_string_strtok_r_params "str:?Aptr?Gchar,delim:?Aptr?Gchar,save_ptr:?Aptr?Aptr?Gchar"
FORCELOCAL WUNUSED NONNULL((3)) DREF DeeObject *DCALL c_string_strtok_r_f_impl(char *str, char const *delim, char **save_ptr);
PRIVATE WUNUSED DREF DeeObject *DCALL c_string_strtok_r_f(size_t argc, DeeObject *const *argv) {
	struct {
		DeeObject *raw_str;
		DeeObject *raw_delim;
		DeeObject *raw_save_ptr;
	} args;
	union pointer str;
	union pointer delim;
	void **save_ptr;
	DeeArg_UnpackStruct3(err, argc, argv, "strtok_r", &args, &args.raw_str, &args.raw_delim, &args.raw_save_ptr);
	if unlikely(DeeObject_AsPointer(args.raw_str, &DeeCChar_Type, &str))
		goto err;
	if unlikely(DeeObject_AsPointer(args.raw_delim, &DeeCChar_Type, &delim))
		goto err;
	if unlikely(DeeObject_AsPPointer(args.raw_save_ptr, &DeeCChar_Type, &save_ptr))
		goto err;
	return c_string_strtok_r_f_impl(str.pchar, delim.pcchar, (char **)save_ptr);
err:
	return NULL;
}
INTERN DEFINE_CMETHOD(c_string_strtok_r, &c_string_strtok_r_f, METHOD_FNORMAL);
FORCELOCAL WUNUSED NONNULL((3)) DREF DeeObject *DCALL c_string_strtok_r_f_impl(char *str, char const *delim, char **save_ptr)
{
	char *result;
	CTYPES_FAULTPROTECT(result = strtok_r(str, delim, save_ptr), return NULL);
	return DeePointer_NewFor(&DeeCChar_Type, (void *)result);
}
/*[[[end]]]*/

INTERN WUNUSED DREF DeeObject *DCALL
c_string_strfry_f(size_t argc, DeeObject *const *argv) {
	(void)argc;
	(void)argv;
	/* TODO */
	DeeError_NOTIMPLEMENTED();
	return NULL;
}

INTERN DEFINE_CMETHOD(c_string_strfry, &c_string_strfry_f, METHOD_FNORMAL);

DECL_END

#endif /* !GUARD_DEX_CTYPES_C_STRING_C */
