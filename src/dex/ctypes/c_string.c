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
#ifndef GUARD_DEX_CTYPES_C_STRING_C
#define GUARD_DEX_CTYPES_C_STRING_C 1
#define DEE_SOURCE

#include "libctypes.h"
/**/

#include <deemon/arg.h>
#include <deemon/api.h>
#include <deemon/error-rt.h>
#include <deemon/error.h>
#include <deemon/int.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/system-features.h> /* memmem(), tolower(), toupper(), ... */
#include <deemon/system.h>

#include <hybrid/overflow.h>
/**/

#include "c_api.h" /* Prototypes */
/**/

#include <stddef.h> /* size_t */

DECL_BEGIN

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
LOCAL WUNUSED NONNULL((1)) char *
dee_strrchr(char const *haystack, int needle) {
	char *result = NULL;
	for (;; ++haystack) {
		char ch = *haystack;
		if (ch == (char)(unsigned char)needle)
			result = (char *)haystack;
		if (!ch)
			break;
	}
	return result;
}
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
	for (; maxlen-- && *haystack && (unsigned char)*haystack != (unsigned char)needle; ++haystack)
		;
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
#ifdef DEE_SYSTEM_FS_DRIVES
	/* Skip drive letter. */
	if (isalpha(iter[0]) && iter[1] == ':')
		iter += 2;
#endif /* DEE_SYSTEM_FS_DRIVES */
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

INTERN WUNUSED DREF DeeObject *DCALL
capi_memcpy(size_t argc, DeeObject *const *argv) {
	union pointer dst, src;
	size_t total;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("memcpy", params: """
	DeeObject *dst;
	DeeObject *src;
	size_t num_bytes;
	size_t elem_count = 1;
""");]]]*/
	struct {
		DeeObject *dst;
		DeeObject *src;
		size_t num_bytes;
		size_t elem_count;
	} args;
	args.elem_count = 1;
	if (DeeArg_UnpackStruct(argc, argv, "oo" UNPuSIZ "|" UNPuSIZ ":memcpy", &args))
		goto err;
/*[[[end]]]*/
	if (DeeObject_AsPointer(args.dst, &DeeCVoid_Type, &dst))
		goto err;
	if (DeeObject_AsPointer(args.src, &DeeCVoid_Type, &src))
		goto err;
	if (OVERFLOW_UMUL(args.num_bytes, args.elem_count, &total))
		goto err_overflow;
	CTYPES_FAULTPROTECT(memcpy(dst.ptr, src.ptr, total), goto err);
	return DeePointer_NewVoid(dst.ptr);
err_overflow:
	DeeRT_ErrIntegerOverflowUMul(args.num_bytes, args.elem_count);
err:
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_mempcpy(size_t argc, DeeObject *const *argv) {
	union pointer dst, src;
	size_t total;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("mempcpy", params: """
	DeeObject *dst;
	DeeObject *src;
	size_t num_bytes;
	size_t elem_count = 1;
""");]]]*/
	struct {
		DeeObject *dst;
		DeeObject *src;
		size_t num_bytes;
		size_t elem_count;
	} args;
	args.elem_count = 1;
	if (DeeArg_UnpackStruct(argc, argv, "oo" UNPuSIZ "|" UNPuSIZ ":mempcpy", &args))
		goto err;
/*[[[end]]]*/
	if (DeeObject_AsPointer(args.dst, &DeeCVoid_Type, &dst))
		goto err;
	if (DeeObject_AsPointer(args.src, &DeeCVoid_Type, &src))
		goto err;
	if (OVERFLOW_UMUL(args.num_bytes, args.elem_count, &total))
		goto err_overflow;
	CTYPES_FAULTPROTECT(memcpy(dst.ptr, src.ptr, total), goto err);
	return DeePointer_NewVoid((void *)(dst.uint + total));
err_overflow:
	DeeRT_ErrIntegerOverflowUMul(args.num_bytes, args.elem_count);
err:
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_memccpy(size_t argc, DeeObject *const *argv) {
	union pointer dst, src;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("memccpy", params: """
	DeeObject *dst;
	DeeObject *src;
	int needle;
	size_t num_bytes;
""");]]]*/
	struct {
		DeeObject *dst;
		DeeObject *src;
		int needle;
		size_t num_bytes;
	} args;
	if (DeeArg_UnpackStruct(argc, argv, "ood" UNPuSIZ ":memccpy", &args))
		goto err;
/*[[[end]]]*/
	if (DeeObject_AsPointer(args.dst, &DeeCVoid_Type, &dst))
		goto err;
	if (DeeObject_AsPointer(args.src, &DeeCVoid_Type, &src))
		goto err;
	CTYPES_FAULTPROTECT(dst.ptr = memccpy(dst.ptr, src.ptr, args.needle, args.num_bytes), goto err);
	--dst.p8;
	return DeePointer_NewVoid(dst.ptr);
err:
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_memset(size_t argc, DeeObject *const *argv) {
	union pointer dst;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("memset", params: """
	DeeObject *dst;
	int byte;
	size_t num_bytes;
""");]]]*/
	struct {
		DeeObject *dst;
		int byte;
		size_t num_bytes;
	} args;
	DeeArg_UnpackStruct3X(err, argc, argv, "memset", &args, &args.dst, "o", _DeeArg_AsObject, &args.byte, "d", DeeObject_AsInt, &args.num_bytes, UNPuSIZ, DeeObject_AsSize);
/*[[[end]]]*/
	if (DeeObject_AsPointer(args.dst, &DeeCVoid_Type, &dst))
		goto err;
	CTYPES_FAULTPROTECT(memset(dst.ptr, args.byte, args.num_bytes), goto err);
	return DeePointer_NewVoid(dst.ptr);
err:
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_mempset(size_t argc, DeeObject *const *argv) {
	union pointer dst;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("mempset", params: """
	DeeObject *dst;
	int byte;
	size_t num_bytes;
""");]]]*/
	struct {
		DeeObject *dst;
		int byte;
		size_t num_bytes;
	} args;
	DeeArg_UnpackStruct3X(err, argc, argv, "mempset", &args, &args.dst, "o", _DeeArg_AsObject, &args.byte, "d", DeeObject_AsInt, &args.num_bytes, UNPuSIZ, DeeObject_AsSize);
/*[[[end]]]*/
	if (DeeObject_AsPointer(args.dst, &DeeCVoid_Type, &dst))
		goto err;
	CTYPES_FAULTPROTECT(memset(dst.ptr, args.byte, args.num_bytes), goto err);
	return DeePointer_NewVoid((void *)(dst.uint + args.num_bytes));
err:
	return NULL;
}


INTERN WUNUSED DREF DeeObject *DCALL
capi_bzero(size_t argc, DeeObject *const *argv) {
	union pointer dst;
	size_t total;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("bzero", params: """
	DeeObject *dst;
	size_t num_bytes;
	size_t elem_count = 1;
""");]]]*/
	struct {
		DeeObject *dst;
		size_t num_bytes;
		size_t elem_count;
	} args;
	args.elem_count = 1;
	if (DeeArg_UnpackStruct(argc, argv, "o" UNPuSIZ "|" UNPuSIZ ":bzero", &args))
		goto err;
/*[[[end]]]*/
	if (DeeObject_AsPointer(args.dst, &DeeCVoid_Type, &dst))
		goto err;
	if (OVERFLOW_UMUL(args.num_bytes, args.elem_count, &total))
		goto err_overflow;
	CTYPES_FAULTPROTECT(bzero(dst.ptr, total), goto err);
	return_none;
err_overflow:
	DeeRT_ErrIntegerOverflowUMul(args.num_bytes, args.elem_count);
err:
	return NULL;
}


INTERN WUNUSED DREF DeeObject *DCALL
capi_memmove(size_t argc, DeeObject *const *argv) {
	union pointer dst, src;
	size_t total;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("memmove", params: """
	DeeObject *dst;
	DeeObject *src;
	size_t num_bytes;
	size_t elem_count = 1;
""");]]]*/
	struct {
		DeeObject *dst;
		DeeObject *src;
		size_t num_bytes;
		size_t elem_count;
	} args;
	args.elem_count = 1;
	if (DeeArg_UnpackStruct(argc, argv, "oo" UNPuSIZ "|" UNPuSIZ ":memmove", &args))
		goto err;
/*[[[end]]]*/
	if (DeeObject_AsPointer(args.dst, &DeeCVoid_Type, &dst))
		goto err;
	if (DeeObject_AsPointer(args.src, &DeeCVoid_Type, &src))
		goto err;
	if (OVERFLOW_UMUL(args.num_bytes, args.elem_count, &total))
		goto err_overflow;
	CTYPES_FAULTPROTECT(memmove(dst.ptr, src.ptr, total), goto err);
	return DeePointer_NewVoid(dst.ptr);
err_overflow:
	DeeRT_ErrIntegerOverflowUMul(args.num_bytes, args.elem_count);
err:
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_mempmove(size_t argc, DeeObject *const *argv) {
	union pointer dst, src;
	size_t total;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("memmove", params: """
	DeeObject *dst;
	DeeObject *src;
	size_t num_bytes;
	size_t elem_count = 1;
""");]]]*/
	struct {
		DeeObject *dst;
		DeeObject *src;
		size_t num_bytes;
		size_t elem_count;
	} args;
	args.elem_count = 1;
	if (DeeArg_UnpackStruct(argc, argv, "oo" UNPuSIZ "|" UNPuSIZ ":memmove", &args))
		goto err;
/*[[[end]]]*/
	if (DeeObject_AsPointer(args.dst, &DeeCVoid_Type, &dst))
		goto err;
	if (DeeObject_AsPointer(args.src, &DeeCVoid_Type, &src))
		goto err;
	if (OVERFLOW_UMUL(args.num_bytes, args.elem_count, &total))
		goto err_overflow;
	CTYPES_FAULTPROTECT(memmove(dst.ptr, src.ptr, total), goto err);
	return DeePointer_NewVoid(dst.p8 + total);
err_overflow:
	DeeRT_ErrIntegerOverflowUMul(args.num_bytes, args.elem_count);
err:
	return NULL;
}



INTERN WUNUSED DREF DeeObject *DCALL
capi_memchr(size_t argc, DeeObject *const *argv) {
	union pointer result;
	union pointer haystack;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("memchr", params: """
	DeeObject *haystack;
	int needle;
	size_t num_bytes;
""");]]]*/
	struct {
		DeeObject *haystack;
		int needle;
		size_t num_bytes;
	} args;
	DeeArg_UnpackStruct3X(err, argc, argv, "memchr", &args, &args.haystack, "o", _DeeArg_AsObject, &args.needle, "d", DeeObject_AsInt, &args.num_bytes, UNPuSIZ, DeeObject_AsSize);
/*[[[end]]]*/
	if (DeeObject_AsPointer(args.haystack, &DeeCVoid_Type, &haystack))
		goto err;
	CTYPES_FAULTPROTECT(result.ptr = memchr(haystack.ptr, args.needle, args.num_bytes), goto err);
	return DeePointer_NewVoid(result.ptr);
err:
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_memrchr(size_t argc, DeeObject *const *argv) {
	union pointer result;
	union pointer haystack;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("memrchr", params: """
	DeeObject *haystack;
	int needle;
	size_t num_bytes;
""");]]]*/
	struct {
		DeeObject *haystack;
		int needle;
		size_t num_bytes;
	} args;
	DeeArg_UnpackStruct3X(err, argc, argv, "memrchr", &args, &args.haystack, "o", _DeeArg_AsObject, &args.needle, "d", DeeObject_AsInt, &args.num_bytes, UNPuSIZ, DeeObject_AsSize);
/*[[[end]]]*/
	if (DeeObject_AsPointer(args.haystack, &DeeCVoid_Type, &haystack))
		goto err;
	CTYPES_FAULTPROTECT(result.ptr = memrchr(haystack.ptr, args.needle, args.num_bytes), goto err);
	return DeePointer_NewVoid(result.ptr);
err:
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_memend(size_t argc, DeeObject *const *argv) {
	union pointer result;
	union pointer haystack;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("memend", params: """
	DeeObject *haystack;
	int needle;
	size_t num_bytes;
""");]]]*/
	struct {
		DeeObject *haystack;
		int needle;
		size_t num_bytes;
	} args;
	DeeArg_UnpackStruct3X(err, argc, argv, "memend", &args, &args.haystack, "o", _DeeArg_AsObject, &args.needle, "d", DeeObject_AsInt, &args.num_bytes, UNPuSIZ, DeeObject_AsSize);
/*[[[end]]]*/
	if (DeeObject_AsPointer(args.haystack, &DeeCVoid_Type, &haystack))
		goto err;
	CTYPES_FAULTPROTECT(result.ptr = memend(haystack.ptr, args.needle, args.num_bytes), goto err);
	return DeePointer_NewVoid(result.ptr);
err:
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_memrend(size_t argc, DeeObject *const *argv) {
	union pointer result;
	union pointer haystack;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("memrend", params: """
	DeeObject *haystack;
	int needle;
	size_t num_bytes;
""");]]]*/
	struct {
		DeeObject *haystack;
		int needle;
		size_t num_bytes;
	} args;
	DeeArg_UnpackStruct3X(err, argc, argv, "memrend", &args, &args.haystack, "o", _DeeArg_AsObject, &args.needle, "d", DeeObject_AsInt, &args.num_bytes, UNPuSIZ, DeeObject_AsSize);
/*[[[end]]]*/
	if (DeeObject_AsPointer(args.haystack, &DeeCVoid_Type, &haystack))
		goto err;
	CTYPES_FAULTPROTECT(result.ptr = memrend(haystack.ptr, args.needle, args.num_bytes), goto err);
	return DeePointer_NewVoid(result.ptr);
err:
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_memlen(size_t argc, DeeObject *const *argv) {
	size_t result;
	union pointer haystack;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("memlen", params: """
	DeeObject *haystack;
	int needle;
	size_t num_bytes;
""");]]]*/
	struct {
		DeeObject *haystack;
		int needle;
		size_t num_bytes;
	} args;
	DeeArg_UnpackStruct3X(err, argc, argv, "memlen", &args, &args.haystack, "o", _DeeArg_AsObject, &args.needle, "d", DeeObject_AsInt, &args.num_bytes, UNPuSIZ, DeeObject_AsSize);
/*[[[end]]]*/
	if (DeeObject_AsPointer(args.haystack, &DeeCVoid_Type, &haystack))
		goto err;
	CTYPES_FAULTPROTECT(result = memlen(haystack.ptr, args.needle, args.num_bytes), goto err);
	return DeeInt_NewSize(result);
err:
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_memrlen(size_t argc, DeeObject *const *argv) {
	size_t result;
	union pointer haystack;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("memrlen", params: """
	DeeObject *haystack;
	int needle;
	size_t num_bytes;
""");]]]*/
	struct {
		DeeObject *haystack;
		int needle;
		size_t num_bytes;
	} args;
	DeeArg_UnpackStruct3X(err, argc, argv, "memrlen", &args, &args.haystack, "o", _DeeArg_AsObject, &args.needle, "d", DeeObject_AsInt, &args.num_bytes, UNPuSIZ, DeeObject_AsSize);
/*[[[end]]]*/
	if (DeeObject_AsPointer(args.haystack, &DeeCVoid_Type, &haystack))
		goto err;
	CTYPES_FAULTPROTECT(result = memrlen(haystack.ptr, args.needle, args.num_bytes), goto err);
	return DeeInt_NewSize(result);
err:
	return NULL;
}

INTDEF WUNUSED DREF DeeObject *DCALL
capi_rawmemchr(size_t argc, DeeObject *const *argv) {
	union pointer result;
	union pointer haystack;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("rawmemchr", params: """
	DeeObject *haystack;
	int needle;
""");]]]*/
	struct {
		DeeObject *haystack;
		int needle;
	} args;
	DeeArg_UnpackStruct2X(err, argc, argv, "rawmemchr", &args, &args.haystack, "o", _DeeArg_AsObject, &args.needle, "d", DeeObject_AsInt);
/*[[[end]]]*/
	if (DeeObject_AsPointer(args.haystack, &DeeCVoid_Type, &haystack))
		goto err;
	CTYPES_FAULTPROTECT(result.ptr = rawmemchr(haystack.ptr, args.needle), goto err);
	return DeePointer_NewVoid(result.ptr);
err:
	return NULL;
}

INTDEF WUNUSED DREF DeeObject *DCALL
capi_rawmemrchr(size_t argc, DeeObject *const *argv) {
	union pointer result;
	union pointer haystack;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("rawmemrchr", params: """
	DeeObject *haystack;
	int needle;
""");]]]*/
	struct {
		DeeObject *haystack;
		int needle;
	} args;
	DeeArg_UnpackStruct2X(err, argc, argv, "rawmemrchr", &args, &args.haystack, "o", _DeeArg_AsObject, &args.needle, "d", DeeObject_AsInt);
/*[[[end]]]*/
	if (DeeObject_AsPointer(args.haystack, &DeeCVoid_Type, &haystack))
		goto err;
	CTYPES_FAULTPROTECT(result.ptr = rawmemrchr(haystack.ptr, args.needle), goto err);
	return DeePointer_NewVoid(result.ptr);
err:
	return NULL;
}

INTDEF WUNUSED DREF DeeObject *DCALL
capi_rawmemlen(size_t argc, DeeObject *const *argv) {
	size_t result;
	union pointer haystack;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("rawmemlen", params: """
	DeeObject *haystack;
	int needle;
""");]]]*/
	struct {
		DeeObject *haystack;
		int needle;
	} args;
	DeeArg_UnpackStruct2X(err, argc, argv, "rawmemlen", &args, &args.haystack, "o", _DeeArg_AsObject, &args.needle, "d", DeeObject_AsInt);
/*[[[end]]]*/
	if (DeeObject_AsPointer(args.haystack, &DeeCVoid_Type, &haystack))
		goto err;
	CTYPES_FAULTPROTECT(result = rawmemlen(haystack.ptr, args.needle), goto err);
	return DeeInt_NewSize(result);
err:
	return NULL;
}

INTDEF WUNUSED DREF DeeObject *DCALL
capi_rawmemrlen(size_t argc, DeeObject *const *argv) {
	size_t result;
	union pointer haystack;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("rawmemrlen", params: """
	DeeObject *haystack;
	int needle;
""");]]]*/
	struct {
		DeeObject *haystack;
		int needle;
	} args;
	DeeArg_UnpackStruct2X(err, argc, argv, "rawmemrlen", &args, &args.haystack, "o", _DeeArg_AsObject, &args.needle, "d", DeeObject_AsInt);
/*[[[end]]]*/
	if (DeeObject_AsPointer(args.haystack, &DeeCVoid_Type, &haystack))
		goto err;
	CTYPES_FAULTPROTECT(result = rawmemrlen(haystack.ptr, args.needle), goto err);
	return DeeInt_NewSize(result);
err:
	return NULL;
}


INTERN WUNUSED DREF DeeObject *DCALL
capi_memxchr(size_t argc, DeeObject *const *argv) {
	union pointer result;
	union pointer haystack;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("memxchr", params: """
	DeeObject *haystack;
	int needle;
	size_t num_bytes;
""");]]]*/
	struct {
		DeeObject *haystack;
		int needle;
		size_t num_bytes;
	} args;
	DeeArg_UnpackStruct3X(err, argc, argv, "memxchr", &args, &args.haystack, "o", _DeeArg_AsObject, &args.needle, "d", DeeObject_AsInt, &args.num_bytes, UNPuSIZ, DeeObject_AsSize);
/*[[[end]]]*/
	if (DeeObject_AsPointer(args.haystack, &DeeCVoid_Type, &haystack))
		goto err;
	CTYPES_FAULTPROTECT(result.ptr = memxchr(haystack.ptr, args.needle, args.num_bytes), goto err);
	return DeePointer_NewVoid(result.ptr);
err:
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_memrxchr(size_t argc, DeeObject *const *argv) {
	union pointer result;
	union pointer haystack;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("memrxchr", params: """
	DeeObject *haystack;
	int needle;
	size_t num_bytes;
""");]]]*/
	struct {
		DeeObject *haystack;
		int needle;
		size_t num_bytes;
	} args;
	DeeArg_UnpackStruct3X(err, argc, argv, "memrxchr", &args, &args.haystack, "o", _DeeArg_AsObject, &args.needle, "d", DeeObject_AsInt, &args.num_bytes, UNPuSIZ, DeeObject_AsSize);
/*[[[end]]]*/
	if (DeeObject_AsPointer(args.haystack, &DeeCVoid_Type, &haystack))
		goto err;
	CTYPES_FAULTPROTECT(result.ptr = memrxchr(haystack.ptr, args.needle, args.num_bytes), goto err);
	return DeePointer_NewVoid(result.ptr);
err:
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_memxend(size_t argc, DeeObject *const *argv) {
	union pointer result;
	union pointer haystack;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("memxend", params: """
	DeeObject *haystack;
	int needle;
	size_t num_bytes;
""");]]]*/
	struct {
		DeeObject *haystack;
		int needle;
		size_t num_bytes;
	} args;
	DeeArg_UnpackStruct3X(err, argc, argv, "memxend", &args, &args.haystack, "o", _DeeArg_AsObject, &args.needle, "d", DeeObject_AsInt, &args.num_bytes, UNPuSIZ, DeeObject_AsSize);
/*[[[end]]]*/
	if (DeeObject_AsPointer(args.haystack, &DeeCVoid_Type, &haystack))
		goto err;
	CTYPES_FAULTPROTECT(result.ptr = memxend(haystack.ptr, args.needle, args.num_bytes), goto err);
	return DeePointer_NewVoid(result.ptr);
err:
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_memrxend(size_t argc, DeeObject *const *argv) {
	union pointer result;
	union pointer haystack;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("memrxend", params: """
	DeeObject *haystack;
	int needle;
	size_t num_bytes;
""");]]]*/
	struct {
		DeeObject *haystack;
		int needle;
		size_t num_bytes;
	} args;
	DeeArg_UnpackStruct3X(err, argc, argv, "memrxend", &args, &args.haystack, "o", _DeeArg_AsObject, &args.needle, "d", DeeObject_AsInt, &args.num_bytes, UNPuSIZ, DeeObject_AsSize);
/*[[[end]]]*/
	if (DeeObject_AsPointer(args.haystack, &DeeCVoid_Type, &haystack))
		goto err;
	CTYPES_FAULTPROTECT(result.ptr = memrxend(haystack.ptr, args.needle, args.num_bytes), goto err);
	return DeePointer_NewVoid(result.ptr);
err:
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_memxlen(size_t argc, DeeObject *const *argv) {
	size_t result;
	union pointer haystack;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("memxlen", params: """
	DeeObject *haystack;
	int needle;
	size_t num_bytes;
""");]]]*/
	struct {
		DeeObject *haystack;
		int needle;
		size_t num_bytes;
	} args;
	DeeArg_UnpackStruct3X(err, argc, argv, "memxlen", &args, &args.haystack, "o", _DeeArg_AsObject, &args.needle, "d", DeeObject_AsInt, &args.num_bytes, UNPuSIZ, DeeObject_AsSize);
/*[[[end]]]*/
	if (DeeObject_AsPointer(args.haystack, &DeeCVoid_Type, &haystack))
		goto err;
	CTYPES_FAULTPROTECT(result = memxlen(haystack.ptr, args.needle, args.num_bytes), goto err);
	return DeeInt_NewSize(result);
err:
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_memrxlen(size_t argc, DeeObject *const *argv) {
	size_t result;
	union pointer haystack;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("memrxlen", params: """
	DeeObject *haystack;
	int needle;
	size_t num_bytes;
""");]]]*/
	struct {
		DeeObject *haystack;
		int needle;
		size_t num_bytes;
	} args;
	DeeArg_UnpackStruct3X(err, argc, argv, "memrxlen", &args, &args.haystack, "o", _DeeArg_AsObject, &args.needle, "d", DeeObject_AsInt, &args.num_bytes, UNPuSIZ, DeeObject_AsSize);
/*[[[end]]]*/
	if (DeeObject_AsPointer(args.haystack, &DeeCVoid_Type, &haystack))
		goto err;
	CTYPES_FAULTPROTECT(result = memrxlen(haystack.ptr, args.needle, args.num_bytes), goto err);
	return DeeInt_NewSize(result);
err:
	return NULL;
}

INTDEF WUNUSED DREF DeeObject *DCALL
capi_rawmemxchr(size_t argc, DeeObject *const *argv) {
	union pointer result;
	union pointer haystack;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("rawmemxchr", params: """
	DeeObject *haystack;
	int needle;
""");]]]*/
	struct {
		DeeObject *haystack;
		int needle;
	} args;
	DeeArg_UnpackStruct2X(err, argc, argv, "rawmemxchr", &args, &args.haystack, "o", _DeeArg_AsObject, &args.needle, "d", DeeObject_AsInt);
/*[[[end]]]*/
	if (DeeObject_AsPointer(args.haystack, &DeeCVoid_Type, &haystack))
		goto err;
	CTYPES_FAULTPROTECT(result.ptr = rawmemxchr(haystack.ptr, args.needle), goto err);
	return DeePointer_NewVoid(result.ptr);
err:
	return NULL;
}

INTDEF WUNUSED DREF DeeObject *DCALL
capi_rawmemrxchr(size_t argc, DeeObject *const *argv) {
	union pointer result;
	union pointer haystack;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("rawmemrxchr", params: """
	DeeObject *haystack;
	int needle;
""");]]]*/
	struct {
		DeeObject *haystack;
		int needle;
	} args;
	DeeArg_UnpackStruct2X(err, argc, argv, "rawmemrxchr", &args, &args.haystack, "o", _DeeArg_AsObject, &args.needle, "d", DeeObject_AsInt);
/*[[[end]]]*/
	if (DeeObject_AsPointer(args.haystack, &DeeCVoid_Type, &haystack))
		goto err;
	CTYPES_FAULTPROTECT(result.ptr = rawmemrxchr(haystack.ptr, args.needle), goto err);
	return DeePointer_NewVoid(result.ptr);
err:
	return NULL;
}

INTDEF WUNUSED DREF DeeObject *DCALL
capi_rawmemxlen(size_t argc, DeeObject *const *argv) {
	size_t result;
	union pointer haystack;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("rawmemxlen", params: """
	DeeObject *haystack;
	int needle;
""");]]]*/
	struct {
		DeeObject *haystack;
		int needle;
	} args;
	DeeArg_UnpackStruct2X(err, argc, argv, "rawmemxlen", &args, &args.haystack, "o", _DeeArg_AsObject, &args.needle, "d", DeeObject_AsInt);
/*[[[end]]]*/
	if (DeeObject_AsPointer(args.haystack, &DeeCVoid_Type, &haystack))
		goto err;
	CTYPES_FAULTPROTECT(result = rawmemxlen(haystack.ptr, args.needle), goto err);
	return DeeInt_NewSize(result);
err:
	return NULL;
}

INTDEF WUNUSED DREF DeeObject *DCALL
capi_rawmemrxlen(size_t argc, DeeObject *const *argv) {
	size_t result;
	union pointer haystack;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("rawmemrxlen", params: """
	DeeObject *haystack;
	int needle;
""");]]]*/
	struct {
		DeeObject *haystack;
		int needle;
	} args;
	DeeArg_UnpackStruct2X(err, argc, argv, "rawmemrxlen", &args, &args.haystack, "o", _DeeArg_AsObject, &args.needle, "d", DeeObject_AsInt);
/*[[[end]]]*/
	if (DeeObject_AsPointer(args.haystack, &DeeCVoid_Type, &haystack))
		goto err;
	CTYPES_FAULTPROTECT(result = rawmemrxlen(haystack.ptr, args.needle), goto err);
	return DeeInt_NewSize(result);
err:
	return NULL;
}



INTDEF WUNUSED DREF DeeObject *DCALL
capi_bcmp(size_t argc, DeeObject *const *argv) {
	int result;
	union pointer a, b;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("bcmp", params: """
	DeeObject *a;
	DeeObject *b;
	size_t num_bytes;
""");]]]*/
	struct {
		DeeObject *a;
		DeeObject *b;
		size_t num_bytes;
	} args;
	DeeArg_UnpackStruct3X(err, argc, argv, "bcmp", &args, &args.a, "o", _DeeArg_AsObject, &args.b, "o", _DeeArg_AsObject, &args.num_bytes, UNPuSIZ, DeeObject_AsSize);
/*[[[end]]]*/
	if (DeeObject_AsPointer(args.a, &DeeCVoid_Type, &a))
		goto err;
	if (DeeObject_AsPointer(args.b, &DeeCVoid_Type, &b))
		goto err;
	CTYPES_FAULTPROTECT(result = bcmp(a.ptr, b.ptr, args.num_bytes), goto err);
	return DeeInt_NewInt(result);
err:
	return NULL;
}

INTDEF WUNUSED DREF DeeObject *DCALL
capi_memcmp(size_t argc, DeeObject *const *argv) {
	int result;
	union pointer lhs, rhs;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("memcmp", params: """
	DeeObject *lhs;
	DeeObject *rhs;
	size_t num_bytes;
""");]]]*/
	struct {
		DeeObject *lhs;
		DeeObject *rhs;
		size_t num_bytes;
	} args;
	DeeArg_UnpackStruct3X(err, argc, argv, "memcmp", &args, &args.lhs, "o", _DeeArg_AsObject, &args.rhs, "o", _DeeArg_AsObject, &args.num_bytes, UNPuSIZ, DeeObject_AsSize);
/*[[[end]]]*/
	if (DeeObject_AsPointer(args.lhs, &DeeCVoid_Type, &lhs))
		goto err;
	if (DeeObject_AsPointer(args.rhs, &DeeCVoid_Type, &rhs))
		goto err;
	CTYPES_FAULTPROTECT(result = memcmp(lhs.ptr, rhs.ptr, args.num_bytes), goto err);
	return DeeInt_NewInt(result);
err:
	return NULL;
}

INTDEF WUNUSED DREF DeeObject *DCALL
capi_memcasecmp(size_t argc, DeeObject *const *argv) {
	int result;
	union pointer lhs, rhs;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("memcasecmp", params: """
	DeeObject *lhs;
	DeeObject *rhs;
	size_t num_bytes;
""");]]]*/
	struct {
		DeeObject *lhs;
		DeeObject *rhs;
		size_t num_bytes;
	} args;
	DeeArg_UnpackStruct3X(err, argc, argv, "memcasecmp", &args, &args.lhs, "o", _DeeArg_AsObject, &args.rhs, "o", _DeeArg_AsObject, &args.num_bytes, UNPuSIZ, DeeObject_AsSize);
/*[[[end]]]*/
	if (DeeObject_AsPointer(args.lhs, &DeeCVoid_Type, &lhs))
		goto err;
	if (DeeObject_AsPointer(args.rhs, &DeeCVoid_Type, &rhs))
		goto err;
	CTYPES_FAULTPROTECT(result = memcasecmp(lhs.ptr, rhs.ptr, args.num_bytes), goto err);
	return DeeInt_NewInt(result);
err:
	return NULL;
}

INTDEF WUNUSED DREF DeeObject *DCALL
capi_memmem(size_t argc, DeeObject *const *argv) {
	union pointer result;
	union pointer haystack, needle;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("memmem", params: """
	DeeObject *haystack;
	size_t haystack_len;
	DeeObject *needle;
	size_t needle_len;
""");]]]*/
	struct {
		DeeObject *haystack;
		size_t haystack_len;
		DeeObject *needle;
		size_t needle_len;
	} args;
	if (DeeArg_UnpackStruct(argc, argv, "o" UNPuSIZ "o" UNPuSIZ ":memmem", &args))
		goto err;
/*[[[end]]]*/
	if (DeeObject_AsPointer(args.haystack, &DeeCVoid_Type, &haystack))
		goto err;
	if (DeeObject_AsPointer(args.needle, &DeeCVoid_Type, &needle))
		goto err;
	CTYPES_FAULTPROTECT(result.ptr = memmem(haystack.ptr, args.haystack_len,
	                                        needle.ptr, args.needle_len),
	                    goto err);
	return DeePointer_NewVoid(result.ptr);
err:
	return NULL;
}


INTDEF WUNUSED DREF DeeObject *DCALL
capi_memcasemem(size_t argc, DeeObject *const *argv) {
	union pointer result;
	union pointer haystack, needle;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("memcasemem", params: """
	DeeObject *haystack;
	size_t haystack_len;
	DeeObject *needle;
	size_t needle_len;
""");]]]*/
	struct {
		DeeObject *haystack;
		size_t haystack_len;
		DeeObject *needle;
		size_t needle_len;
	} args;
	if (DeeArg_UnpackStruct(argc, argv, "o" UNPuSIZ "o" UNPuSIZ ":memcasemem", &args))
		goto err;
/*[[[end]]]*/
	if (DeeObject_AsPointer(args.haystack, &DeeCVoid_Type, &haystack))
		goto err;
	if (DeeObject_AsPointer(args.needle, &DeeCVoid_Type, &needle))
		goto err;
	CTYPES_FAULTPROTECT(result.ptr = memcasemem(haystack.ptr, args.haystack_len,
	                                            needle.ptr, args.needle_len),
	                    goto err);
	return DeePointer_NewVoid(result.ptr);
err:
	return NULL;
}



INTDEF WUNUSED DREF DeeObject *DCALL
capi_memrmem(size_t argc, DeeObject *const *argv) {
	union pointer result;
	union pointer haystack, needle;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("memrmem", params: """
	DeeObject *haystack;
	size_t haystack_len;
	DeeObject *needle;
	size_t needle_len;
""");]]]*/
	struct {
		DeeObject *haystack;
		size_t haystack_len;
		DeeObject *needle;
		size_t needle_len;
	} args;
	if (DeeArg_UnpackStruct(argc, argv, "o" UNPuSIZ "o" UNPuSIZ ":memrmem", &args))
		goto err;
/*[[[end]]]*/
	if (DeeObject_AsPointer(args.haystack, &DeeCVoid_Type, &haystack))
		goto err;
	if (DeeObject_AsPointer(args.needle, &DeeCVoid_Type, &needle))
		goto err;
	CTYPES_FAULTPROTECT(result.ptr = memrmem(haystack.ptr, args.haystack_len,
	                                         needle.ptr, args.needle_len),
	                    goto err);
	return DeePointer_NewVoid(result.ptr);
err:
	return NULL;
}



INTDEF WUNUSED DREF DeeObject *DCALL
capi_memcasermem(size_t argc, DeeObject *const *argv) {
	union pointer result;
	union pointer haystack, needle;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("memcasermem", params: """
	DeeObject *haystack;
	size_t haystack_len;
	DeeObject *needle;
	size_t needle_len;
""");]]]*/
	struct {
		DeeObject *haystack;
		size_t haystack_len;
		DeeObject *needle;
		size_t needle_len;
	} args;
	if (DeeArg_UnpackStruct(argc, argv, "o" UNPuSIZ "o" UNPuSIZ ":memcasermem", &args))
		goto err;
/*[[[end]]]*/
	if (DeeObject_AsPointer(args.haystack, &DeeCVoid_Type, &haystack))
		goto err;
	if (DeeObject_AsPointer(args.needle, &DeeCVoid_Type, &needle))
		goto err;
	CTYPES_FAULTPROTECT(result.ptr = memcasermem(haystack.ptr, args.haystack_len,
	                                             needle.ptr, args.needle_len),
	                    goto err);
	return DeePointer_NewVoid(result.ptr);
err:
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_memrev(size_t argc, DeeObject *const *argv) {
	union pointer dst;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("memrev", params: """
	DeeObject *dst;
	size_t num_bytes;
""");]]]*/
	struct {
		DeeObject *dst;
		size_t num_bytes;
	} args;
	DeeArg_UnpackStruct2X(err, argc, argv, "memrev", &args, &args.dst, "o", _DeeArg_AsObject, &args.num_bytes, UNPuSIZ, DeeObject_AsSize);
/*[[[end]]]*/
	if (DeeObject_AsPointer(args.dst, &DeeCVoid_Type, &dst))
		goto err;
	CTYPES_FAULTPROTECT(memrev(dst.ptr, args.num_bytes), goto err);
	return DeePointer_NewVoid(dst.ptr);
err:
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_strlen(size_t argc, DeeObject *const *argv) {
	union pointer str;
	size_t result;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("strlen", params: """
	DeeObject *str;
""");]]]*/
	struct {
		DeeObject *str;
	} args;
	DeeArg_Unpack1(err, argc, argv, "strlen", &args.str);
/*[[[end]]]*/
	if (DeeObject_AsPointer(args.str, &DeeCChar_Type, &str))
		goto err;
	CTYPES_FAULTPROTECT(result = strlen(str.pchar), goto err);
	return DeeInt_NewSize(result);
err:
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_strend(size_t argc, DeeObject *const *argv) {
	union pointer str;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("strend", params: """
	DeeObject *str;
""");]]]*/
	struct {
		DeeObject *str;
	} args;
	DeeArg_Unpack1(err, argc, argv, "strend", &args.str);
/*[[[end]]]*/
	if (DeeObject_AsPointer(args.str, &DeeCChar_Type, &str))
		goto err;
	CTYPES_FAULTPROTECT(str.pchar = strend(str.pchar), goto err);
	return DeePointer_NewFor(&DeeCChar_Type, str.ptr);
err:
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_strnlen(size_t argc, DeeObject *const *argv) {
	union pointer str;
	size_t result;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("strnlen", params: """
	DeeObject *str;
	size_t maxlen;
""");]]]*/
	struct {
		DeeObject *str;
		size_t maxlen;
	} args;
	DeeArg_UnpackStruct2X(err, argc, argv, "strnlen", &args, &args.str, "o", _DeeArg_AsObject, &args.maxlen, UNPuSIZ, DeeObject_AsSize);
/*[[[end]]]*/
	if (DeeObject_AsPointer(args.str, &DeeCChar_Type, &str))
		goto err;
	CTYPES_FAULTPROTECT(result = strnlen(str.pchar, args.maxlen), goto err);
	return DeeInt_NewSize(result);
err:
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_strnend(size_t argc, DeeObject *const *argv) {
	union pointer str;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("strnend", params: """
	DeeObject *str;
	size_t maxlen;
""");]]]*/
	struct {
		DeeObject *str;
		size_t maxlen;
	} args;
	DeeArg_UnpackStruct2X(err, argc, argv, "strnend", &args, &args.str, "o", _DeeArg_AsObject, &args.maxlen, UNPuSIZ, DeeObject_AsSize);
/*[[[end]]]*/
	if (DeeObject_AsPointer(args.str, &DeeCChar_Type, &str))
		goto err;
	CTYPES_FAULTPROTECT(str.pchar = strnend(str.pchar, args.maxlen), goto err);
	return DeePointer_NewFor(&DeeCChar_Type, str.ptr);
err:
	return NULL;
}



INTERN WUNUSED DREF DeeObject *DCALL
capi_strchr(size_t argc, DeeObject *const *argv) {
	union pointer result;
	union pointer haystack;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("strchr", params: """
	DeeObject *haystack;
	int needle;
""");]]]*/
	struct {
		DeeObject *haystack;
		int needle;
	} args;
	DeeArg_UnpackStruct2X(err, argc, argv, "strchr", &args, &args.haystack, "o", _DeeArg_AsObject, &args.needle, "d", DeeObject_AsInt);
/*[[[end]]]*/
	if (DeeObject_AsPointer(args.haystack, &DeeCChar_Type, &haystack))
		goto err;
	CTYPES_FAULTPROTECT(result.pchar = strchr(haystack.pchar, args.needle), goto err);
	return DeePointer_NewFor(&DeeCChar_Type, result.pchar);
err:
	return NULL;
}


INTERN WUNUSED DREF DeeObject *DCALL
capi_strrchr(size_t argc, DeeObject *const *argv) {
	union pointer result;
	union pointer haystack;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("strrchr", params: """
	DeeObject *haystack;
	int needle;
""");]]]*/
	struct {
		DeeObject *haystack;
		int needle;
	} args;
	DeeArg_UnpackStruct2X(err, argc, argv, "strrchr", &args, &args.haystack, "o", _DeeArg_AsObject, &args.needle, "d", DeeObject_AsInt);
/*[[[end]]]*/
	if (DeeObject_AsPointer(args.haystack, &DeeCChar_Type, &haystack))
		goto err;
	CTYPES_FAULTPROTECT(result.pchar = strrchr(haystack.pchar, args.needle), goto err);
	return DeePointer_NewFor(&DeeCChar_Type, result.pchar);
err:
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_strnchr(size_t argc, DeeObject *const *argv) {
	union pointer result;
	union pointer haystack;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("strnchr", params: """
	DeeObject *haystack;
	int needle;
	size_t maxlen;
""");]]]*/
	struct {
		DeeObject *haystack;
		int needle;
		size_t maxlen;
	} args;
	DeeArg_UnpackStruct3X(err, argc, argv, "strnchr", &args, &args.haystack, "o", _DeeArg_AsObject, &args.needle, "d", DeeObject_AsInt, &args.maxlen, UNPuSIZ, DeeObject_AsSize);
/*[[[end]]]*/
	if (DeeObject_AsPointer(args.haystack, &DeeCChar_Type, &haystack))
		goto err;
	CTYPES_FAULTPROTECT(result.pchar = strnchr(haystack.pchar, args.needle, args.maxlen), goto err);
	return DeePointer_NewFor(&DeeCChar_Type, result.pchar);
err:
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_strnrchr(size_t argc, DeeObject *const *argv) {
	union pointer result;
	union pointer haystack;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("strnrchr", params: """
	DeeObject *haystack;
	int needle;
	size_t maxlen;
""");]]]*/
	struct {
		DeeObject *haystack;
		int needle;
		size_t maxlen;
	} args;
	DeeArg_UnpackStruct3X(err, argc, argv, "strnrchr", &args, &args.haystack, "o", _DeeArg_AsObject, &args.needle, "d", DeeObject_AsInt, &args.maxlen, UNPuSIZ, DeeObject_AsSize);
/*[[[end]]]*/
	if (DeeObject_AsPointer(args.haystack, &DeeCChar_Type, &haystack))
		goto err;
	CTYPES_FAULTPROTECT(result.pchar = strnrchr(haystack.pchar, args.needle, args.maxlen), goto err);
	return DeePointer_NewFor(&DeeCChar_Type, result.pchar);
err:
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_stroff(size_t argc, DeeObject *const *argv) {
	union pointer result;
	union pointer haystack;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("stroff", params: """
	DeeObject *haystack;
	int needle;
""");]]]*/
	struct {
		DeeObject *haystack;
		int needle;
	} args;
	DeeArg_UnpackStruct2X(err, argc, argv, "stroff", &args, &args.haystack, "o", _DeeArg_AsObject, &args.needle, "d", DeeObject_AsInt);
/*[[[end]]]*/
	if (DeeObject_AsPointer(args.haystack, &DeeCChar_Type, &haystack))
		goto err;
	CTYPES_FAULTPROTECT(result.pchar = strchrnul(haystack.pchar, args.needle), goto err);
	return DeeInt_NewSize((size_t)(result.pchar - haystack.pchar));
err:
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_strroff(size_t argc, DeeObject *const *argv) {
	union pointer result;
	union pointer haystack;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("strroff", params: """
	DeeObject *haystack;
	int needle;
""");]]]*/
	struct {
		DeeObject *haystack;
		int needle;
	} args;
	DeeArg_UnpackStruct2X(err, argc, argv, "strroff", &args, &args.haystack, "o", _DeeArg_AsObject, &args.needle, "d", DeeObject_AsInt);
/*[[[end]]]*/
	if (DeeObject_AsPointer(args.haystack, &DeeCChar_Type, &haystack))
		goto err;
	CTYPES_FAULTPROTECT(result.pchar = strrchrnul(haystack.pchar, args.needle), goto err);
	return DeeInt_NewSize((size_t)(result.pchar - haystack.pchar));
err:
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_strnoff(size_t argc, DeeObject *const *argv) {
	union pointer result;
	union pointer haystack;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("strnoff", params: """
	DeeObject *haystack;
	int needle;
	size_t maxlen;
""");]]]*/
	struct {
		DeeObject *haystack;
		int needle;
		size_t maxlen;
	} args;
	DeeArg_UnpackStruct3X(err, argc, argv, "strnoff", &args, &args.haystack, "o", _DeeArg_AsObject, &args.needle, "d", DeeObject_AsInt, &args.maxlen, UNPuSIZ, DeeObject_AsSize);
/*[[[end]]]*/
	if (DeeObject_AsPointer(args.haystack, &DeeCChar_Type, &haystack))
		goto err;
	CTYPES_FAULTPROTECT(result.pchar = strnchrnul(haystack.pchar, args.needle, args.maxlen), goto err);
	return DeeInt_NewSize((size_t)(result.pchar - haystack.pchar));
err:
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_strnroff(size_t argc, DeeObject *const *argv) {
	union pointer result;
	union pointer haystack;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("strnroff", params: """
	DeeObject *haystack;
	int needle;
	size_t maxlen;
""");]]]*/
	struct {
		DeeObject *haystack;
		int needle;
		size_t maxlen;
	} args;
	DeeArg_UnpackStruct3X(err, argc, argv, "strnroff", &args, &args.haystack, "o", _DeeArg_AsObject, &args.needle, "d", DeeObject_AsInt, &args.maxlen, UNPuSIZ, DeeObject_AsSize);
/*[[[end]]]*/
	if (DeeObject_AsPointer(args.haystack, &DeeCChar_Type, &haystack))
		goto err;
	CTYPES_FAULTPROTECT(result.pchar = strnrchrnul(haystack.pchar, args.needle, args.maxlen), goto err);
	return DeeInt_NewSize((size_t)(result.pchar - haystack.pchar));
err:
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_strchrnul(size_t argc, DeeObject *const *argv) {
	union pointer result;
	union pointer haystack;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("strchrnul", params: """
	DeeObject *haystack;
	int needle;
""");]]]*/
	struct {
		DeeObject *haystack;
		int needle;
	} args;
	DeeArg_UnpackStruct2X(err, argc, argv, "strchrnul", &args, &args.haystack, "o", _DeeArg_AsObject, &args.needle, "d", DeeObject_AsInt);
/*[[[end]]]*/
	if (DeeObject_AsPointer(args.haystack, &DeeCChar_Type, &haystack))
		goto err;
	CTYPES_FAULTPROTECT(result.pchar = strchrnul(haystack.pchar, args.needle), goto err);
	return DeePointer_NewFor(&DeeCChar_Type, result.pchar);
err:
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_strrchrnul(size_t argc, DeeObject *const *argv) {
	union pointer result;
	union pointer haystack;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("strrchrnul", params: """
	DeeObject *haystack;
	int needle;
""");]]]*/
	struct {
		DeeObject *haystack;
		int needle;
	} args;
	DeeArg_UnpackStruct2X(err, argc, argv, "strrchrnul", &args, &args.haystack, "o", _DeeArg_AsObject, &args.needle, "d", DeeObject_AsInt);
/*[[[end]]]*/
	if (DeeObject_AsPointer(args.haystack, &DeeCChar_Type, &haystack))
		goto err;
	CTYPES_FAULTPROTECT(result.pchar = strrchrnul(haystack.pchar, args.needle), goto err);
	return DeePointer_NewFor(&DeeCChar_Type, result.pchar);
err:
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_strnchrnul(size_t argc, DeeObject *const *argv) {
	union pointer haystack;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("strnchrnul", params: """
	DeeObject *haystack;
	int needle;
	size_t maxlen;
""");]]]*/
	struct {
		DeeObject *haystack;
		int needle;
		size_t maxlen;
	} args;
	DeeArg_UnpackStruct3X(err, argc, argv, "strnchrnul", &args, &args.haystack, "o", _DeeArg_AsObject, &args.needle, "d", DeeObject_AsInt, &args.maxlen, UNPuSIZ, DeeObject_AsSize);
/*[[[end]]]*/
	if (DeeObject_AsPointer(args.haystack, &DeeCChar_Type, &haystack))
		goto err;
	CTYPES_FAULTPROTECT(haystack.pchar = strnchrnul(haystack.pchar, args.needle, args.maxlen), goto err);
	return DeePointer_NewFor(&DeeCChar_Type, haystack.pchar);
err:
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_strnrchrnul(size_t argc, DeeObject *const *argv) {
	union pointer result;
	union pointer haystack;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("strnrchrnul", params: """
	DeeObject *haystack;
	int needle;
	size_t maxlen;
""");]]]*/
	struct {
		DeeObject *haystack;
		int needle;
		size_t maxlen;
	} args;
	DeeArg_UnpackStruct3X(err, argc, argv, "strnrchrnul", &args, &args.haystack, "o", _DeeArg_AsObject, &args.needle, "d", DeeObject_AsInt, &args.maxlen, UNPuSIZ, DeeObject_AsSize);
/*[[[end]]]*/
	if (DeeObject_AsPointer(args.haystack, &DeeCChar_Type, &haystack))
		goto err;
	CTYPES_FAULTPROTECT(result.pchar = strnrchrnul(haystack.pchar, args.needle, args.maxlen), goto err);
	return DeePointer_NewFor(&DeeCChar_Type, result.pchar);
err:
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_strcmp(size_t argc, DeeObject *const *argv) {
	int result;
	union pointer lhs, rhs;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("strcmp", params: """
	DeeObject *lhs;
	DeeObject *rhs;
""");]]]*/
	struct {
		DeeObject *lhs;
		DeeObject *rhs;
	} args;
	DeeArg_UnpackStruct2(err, argc, argv, "strcmp", &args, &args.lhs, &args.rhs);
/*[[[end]]]*/
	if (DeeObject_AsPointer(args.lhs, &DeeCChar_Type, &lhs))
		goto err;
	if (DeeObject_AsPointer(args.rhs, &DeeCChar_Type, &rhs))
		goto err;
	CTYPES_FAULTPROTECT(result = strcmp(lhs.pchar, rhs.pchar), goto err);
	return DeeInt_NewInt(result);
err:
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_strncmp(size_t argc, DeeObject *const *argv) {
	int result;
	union pointer lhs, rhs;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("strncmp", params: """
	DeeObject *lhs;
	DeeObject *rhs;
	size_t maxlen;
""");]]]*/
	struct {
		DeeObject *lhs;
		DeeObject *rhs;
		size_t maxlen;
	} args;
	DeeArg_UnpackStruct3X(err, argc, argv, "strncmp", &args, &args.lhs, "o", _DeeArg_AsObject, &args.rhs, "o", _DeeArg_AsObject, &args.maxlen, UNPuSIZ, DeeObject_AsSize);
/*[[[end]]]*/
	if (DeeObject_AsPointer(args.lhs, &DeeCChar_Type, &lhs))
		goto err;
	if (DeeObject_AsPointer(args.rhs, &DeeCChar_Type, &rhs))
		goto err;
	CTYPES_FAULTPROTECT(result = strncmp(lhs.pchar, rhs.pchar, args.maxlen), goto err);
	return DeeInt_NewInt(result);
err:
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_strcasecmp(size_t argc, DeeObject *const *argv) {
	int result;
	union pointer lhs, rhs;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("strcasecmp", params: """
	DeeObject *lhs;
	DeeObject *rhs;
""");]]]*/
	struct {
		DeeObject *lhs;
		DeeObject *rhs;
	} args;
	DeeArg_UnpackStruct2(err, argc, argv, "strcasecmp", &args, &args.lhs, &args.rhs);
/*[[[end]]]*/
	if (DeeObject_AsPointer(args.lhs, &DeeCChar_Type, &lhs))
		goto err;
	if (DeeObject_AsPointer(args.rhs, &DeeCChar_Type, &rhs))
		goto err;
	CTYPES_FAULTPROTECT(result = strcasecmp(lhs.pchar, rhs.pchar), goto err);
	return DeeInt_NewInt(result);
err:
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_strncasecmp(size_t argc, DeeObject *const *argv) {
	int result;
	union pointer lhs, rhs;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("strncasecmp", params: """
	DeeObject *lhs;
	DeeObject *rhs;
	size_t maxlen;
""");]]]*/
	struct {
		DeeObject *lhs;
		DeeObject *rhs;
		size_t maxlen;
	} args;
	DeeArg_UnpackStruct3X(err, argc, argv, "strncasecmp", &args, &args.lhs, "o", _DeeArg_AsObject, &args.rhs, "o", _DeeArg_AsObject, &args.maxlen, UNPuSIZ, DeeObject_AsSize);
/*[[[end]]]*/
	if (DeeObject_AsPointer(args.lhs, &DeeCChar_Type, &lhs))
		goto err;
	if (DeeObject_AsPointer(args.rhs, &DeeCChar_Type, &rhs))
		goto err;
	CTYPES_FAULTPROTECT(result = strncasecmp(lhs.pchar, rhs.pchar, args.maxlen), goto err);
	return DeeInt_NewInt(result);
err:
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_strcpy(size_t argc, DeeObject *const *argv) {
	union pointer dst, src, result;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("strcpy", params: """
	DeeObject *dst;
	DeeObject *src;
""");]]]*/
	struct {
		DeeObject *dst;
		DeeObject *src;
	} args;
	DeeArg_UnpackStruct2(err, argc, argv, "strcpy", &args, &args.dst, &args.src);
/*[[[end]]]*/
	if (DeeObject_AsPointer(args.dst, &DeeCChar_Type, &dst))
		goto err;
	if (DeeObject_AsPointer(args.src, &DeeCChar_Type, &src))
		goto err;
	CTYPES_FAULTPROTECT(result.pchar = strcpy(dst.pchar, src.pchar), goto err);
	return DeePointer_NewFor(&DeeCChar_Type, result.pchar);
err:
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_strncpy(size_t argc, DeeObject *const *argv) {
	union pointer dst, src, result;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("strncpy", params: """
	DeeObject *dst;
	DeeObject *src;
	size_t maxlen;
""");]]]*/
	struct {
		DeeObject *dst;
		DeeObject *src;
		size_t maxlen;
	} args;
	DeeArg_UnpackStruct3X(err, argc, argv, "strncpy", &args, &args.dst, "o", _DeeArg_AsObject, &args.src, "o", _DeeArg_AsObject, &args.maxlen, UNPuSIZ, DeeObject_AsSize);
/*[[[end]]]*/
	if (DeeObject_AsPointer(args.dst, &DeeCChar_Type, &dst))
		goto err;
	if (DeeObject_AsPointer(args.src, &DeeCChar_Type, &src))
		goto err;
	CTYPES_FAULTPROTECT(result.pchar = strncpy(dst.pchar, src.pchar, args.maxlen), goto err);
	return DeePointer_NewFor(&DeeCChar_Type, result.pchar);
err:
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_strcat(size_t argc, DeeObject *const *argv) {
	union pointer dst, src, result;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("strcat", params: """
	DeeObject *dst;
	DeeObject *src;
""");]]]*/
	struct {
		DeeObject *dst;
		DeeObject *src;
	} args;
	DeeArg_UnpackStruct2(err, argc, argv, "strcat", &args, &args.dst, &args.src);
/*[[[end]]]*/
	if (DeeObject_AsPointer(args.dst, &DeeCChar_Type, &dst))
		goto err;
	if (DeeObject_AsPointer(args.src, &DeeCChar_Type, &src))
		goto err;
	CTYPES_FAULTPROTECT(result.pchar = strcat(dst.pchar, src.pchar), goto err);
	return DeePointer_NewFor(&DeeCChar_Type, result.pchar);
err:
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_strncat(size_t argc, DeeObject *const *argv) {
	union pointer dst, src, result;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("strncat", params: """
	DeeObject *dst;
	DeeObject *src;
	size_t maxlen;
""");]]]*/
	struct {
		DeeObject *dst;
		DeeObject *src;
		size_t maxlen;
	} args;
	DeeArg_UnpackStruct3X(err, argc, argv, "strncat", &args, &args.dst, "o", _DeeArg_AsObject, &args.src, "o", _DeeArg_AsObject, &args.maxlen, UNPuSIZ, DeeObject_AsSize);
/*[[[end]]]*/
	if (DeeObject_AsPointer(args.dst, &DeeCChar_Type, &dst))
		goto err;
	if (DeeObject_AsPointer(args.src, &DeeCChar_Type, &src))
		goto err;
	CTYPES_FAULTPROTECT(result.pchar = strncat(dst.pchar, src.pchar, args.maxlen), goto err);
	return DeePointer_NewFor(&DeeCChar_Type, result.pchar);
err:
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_stpcpy(size_t argc, DeeObject *const *argv) {
	union pointer dst, src, result;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("stpcpy", params: """
	DeeObject *dst;
	DeeObject *src;
""");]]]*/
	struct {
		DeeObject *dst;
		DeeObject *src;
	} args;
	DeeArg_UnpackStruct2(err, argc, argv, "stpcpy", &args, &args.dst, &args.src);
/*[[[end]]]*/
	if (DeeObject_AsPointer(args.dst, &DeeCChar_Type, &dst))
		goto err;
	if (DeeObject_AsPointer(args.src, &DeeCChar_Type, &src))
		goto err;
	CTYPES_FAULTPROTECT(result.pchar = stpcpy(dst.pchar, src.pchar), goto err);
	return DeePointer_NewFor(&DeeCChar_Type, result.pchar);
err:
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_stpncpy(size_t argc, DeeObject *const *argv) {
	union pointer dst, src, result;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("stpncpy", params: """
	DeeObject *dst;
	DeeObject *src;
	size_t maxlen;
""");]]]*/
	struct {
		DeeObject *dst;
		DeeObject *src;
		size_t maxlen;
	} args;
	DeeArg_UnpackStruct3X(err, argc, argv, "stpncpy", &args, &args.dst, "o", _DeeArg_AsObject, &args.src, "o", _DeeArg_AsObject, &args.maxlen, UNPuSIZ, DeeObject_AsSize);
/*[[[end]]]*/
	if (DeeObject_AsPointer(args.dst, &DeeCChar_Type, &dst))
		goto err;
	if (DeeObject_AsPointer(args.src, &DeeCChar_Type, &src))
		goto err;
	CTYPES_FAULTPROTECT(result.pchar = stpncpy(dst.pchar, src.pchar, args.maxlen), goto err);
	return DeePointer_NewFor(&DeeCChar_Type, result.pchar);
err:
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_strstr(size_t argc, DeeObject *const *argv) {
	union pointer haystack, needle, result;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("strstr", params: """
	DeeObject *haystack;
	DeeObject *needle;
""");]]]*/
	struct {
		DeeObject *haystack;
		DeeObject *needle;
	} args;
	DeeArg_UnpackStruct2(err, argc, argv, "strstr", &args, &args.haystack, &args.needle);
/*[[[end]]]*/
	if (DeeObject_AsPointer(args.haystack, &DeeCChar_Type, &haystack))
		goto err;
	if (DeeObject_AsPointer(args.needle, &DeeCChar_Type, &needle))
		goto err;
	CTYPES_FAULTPROTECT(result.pchar = strstr(haystack.pchar, needle.pchar), goto err);
	return DeePointer_NewFor(&DeeCChar_Type, result.pchar);
err:
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_strcasestr(size_t argc, DeeObject *const *argv) {
	union pointer haystack, needle, result;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("strcasestr", params: """
	DeeObject *haystack;
	DeeObject *needle;
""");]]]*/
	struct {
		DeeObject *haystack;
		DeeObject *needle;
	} args;
	DeeArg_UnpackStruct2(err, argc, argv, "strcasestr", &args, &args.haystack, &args.needle);
/*[[[end]]]*/
	if (DeeObject_AsPointer(args.haystack, &DeeCChar_Type, &haystack))
		goto err;
	if (DeeObject_AsPointer(args.needle, &DeeCChar_Type, &needle))
		goto err;
	CTYPES_FAULTPROTECT(result.pchar = strcasestr(haystack.pchar, needle.pchar), goto err);
	return DeePointer_NewFor(&DeeCChar_Type, result.pchar);
err:
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_strnstr(size_t argc, DeeObject *const *argv) {
	union pointer haystack, needle, result;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("strnstr", params: """
	DeeObject *haystack;
	DeeObject *needle;
	size_t haystack_maxlen;
""");]]]*/
	struct {
		DeeObject *haystack;
		DeeObject *needle;
		size_t haystack_maxlen;
	} args;
	DeeArg_UnpackStruct3X(err, argc, argv, "strnstr", &args, &args.haystack, "o", _DeeArg_AsObject, &args.needle, "o", _DeeArg_AsObject, &args.haystack_maxlen, UNPuSIZ, DeeObject_AsSize);
/*[[[end]]]*/
	if (DeeObject_AsPointer(args.haystack, &DeeCChar_Type, &haystack))
		goto err;
	if (DeeObject_AsPointer(args.needle, &DeeCChar_Type, &needle))
		goto err;
	CTYPES_FAULTPROTECT(result.pchar = strnstr(haystack.pchar, needle.pchar, args.haystack_maxlen), goto err);
	return DeePointer_NewFor(&DeeCChar_Type, result.pchar);
err:
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_strncasestr(size_t argc, DeeObject *const *argv) {
	union pointer haystack, needle, result;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("strncasestr", params: """
	DeeObject *haystack;
	DeeObject *needle;
	size_t haystack_maxlen;
""");]]]*/
	struct {
		DeeObject *haystack;
		DeeObject *needle;
		size_t haystack_maxlen;
	} args;
	DeeArg_UnpackStruct3X(err, argc, argv, "strncasestr", &args, &args.haystack, "o", _DeeArg_AsObject, &args.needle, "o", _DeeArg_AsObject, &args.haystack_maxlen, UNPuSIZ, DeeObject_AsSize);
/*[[[end]]]*/
	if (DeeObject_AsPointer(args.haystack, &DeeCChar_Type, &haystack))
		goto err;
	if (DeeObject_AsPointer(args.needle, &DeeCChar_Type, &needle))
		goto err;
	CTYPES_FAULTPROTECT(result.pchar = strncasestr(haystack.pchar, needle.pchar, args.haystack_maxlen), goto err);
	return DeePointer_NewFor(&DeeCChar_Type, result.pchar);
err:
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_strverscmp(size_t argc, DeeObject *const *argv) {
	int result;
	union pointer lhs, rhs;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("strverscmp", params: """
	DeeObject *lhs;
	DeeObject *rhs;
""");]]]*/
	struct {
		DeeObject *lhs;
		DeeObject *rhs;
	} args;
	DeeArg_UnpackStruct2(err, argc, argv, "strverscmp", &args, &args.lhs, &args.rhs);
/*[[[end]]]*/
	if (DeeObject_AsPointer(args.lhs, &DeeCChar_Type, &lhs))
		goto err;
	if (DeeObject_AsPointer(args.rhs, &DeeCChar_Type, &rhs))
		goto err;
	CTYPES_FAULTPROTECT(result = strverscmp(lhs.pchar, rhs.pchar), goto err);
	return DeeInt_NewInt(result);
err:
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_strspn(size_t argc, DeeObject *const *argv) {
	(void)argc;
	(void)argv;
	/* TODO */
	DeeError_NOTIMPLEMENTED();
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_strcspn(size_t argc, DeeObject *const *argv) {
	(void)argc;
	(void)argv;
	/* TODO */
	DeeError_NOTIMPLEMENTED();
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_strpbrk(size_t argc, DeeObject *const *argv) {
	(void)argc;
	(void)argv;
	/* TODO */
	DeeError_NOTIMPLEMENTED();
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_strrev(size_t argc, DeeObject *const *argv) {
	(void)argc;
	(void)argv;
	/* TODO */
	DeeError_NOTIMPLEMENTED();
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_strnrev(size_t argc, DeeObject *const *argv) {
	(void)argc;
	(void)argv;
	/* TODO */
	DeeError_NOTIMPLEMENTED();
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_strlwr(size_t argc, DeeObject *const *argv) {
	(void)argc;
	(void)argv;
	/* TODO */
	DeeError_NOTIMPLEMENTED();
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_strupr(size_t argc, DeeObject *const *argv) {
	(void)argc;
	(void)argv;
	/* TODO */
	DeeError_NOTIMPLEMENTED();
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_strnlwr(size_t argc, DeeObject *const *argv) {
	(void)argc;
	(void)argv;
	/* TODO */
	DeeError_NOTIMPLEMENTED();
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_strnupr(size_t argc, DeeObject *const *argv) {
	(void)argc;
	(void)argv;
	/* TODO */
	DeeError_NOTIMPLEMENTED();
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_strset(size_t argc, DeeObject *const *argv) {
	(void)argc;
	(void)argv;
	/* TODO */
	DeeError_NOTIMPLEMENTED();
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_strnset(size_t argc, DeeObject *const *argv) {
	(void)argc;
	(void)argv;
	/* TODO */
	DeeError_NOTIMPLEMENTED();
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_strfry(size_t argc, DeeObject *const *argv) {
	(void)argc;
	(void)argv;
	/* TODO */
	DeeError_NOTIMPLEMENTED();
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_memfrob(size_t argc, DeeObject *const *argv) {
	(void)argc;
	(void)argv;
	/* TODO */
	DeeError_NOTIMPLEMENTED();
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_strsep(size_t argc, DeeObject *const *argv) {
	(void)argc;
	(void)argv;
	/* TODO */
	DeeError_NOTIMPLEMENTED();
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_stresep(size_t argc, DeeObject *const *argv) {
	(void)argc;
	(void)argv;
	/* TODO */
	DeeError_NOTIMPLEMENTED();
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_strtok(size_t argc, DeeObject *const *argv) {
	(void)argc;
	(void)argv;
	/* TODO */
	DeeError_NOTIMPLEMENTED();
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_strtok_r(size_t argc, DeeObject *const *argv) {
	(void)argc;
	(void)argv;
	/* TODO */
	DeeError_NOTIMPLEMENTED();
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_basename(size_t argc, DeeObject *const *argv) {
	union pointer str;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("basename", params: """
	DeeObject *str;
""");]]]*/
	struct {
		DeeObject *str;
	} args;
	DeeArg_Unpack1(err, argc, argv, "basename", &args.str);
/*[[[end]]]*/
	if (DeeObject_AsPointer(args.str, &DeeCChar_Type, &str))
		goto err;
	CTYPES_FAULTPROTECT(str.pchar = basename(str.pchar), goto err);
	return DeePointer_NewFor(&DeeCChar_Type, str.ptr);
err:
	return NULL;
}

DECL_END

#endif /* !GUARD_DEX_CTYPES_C_STRING_C */
