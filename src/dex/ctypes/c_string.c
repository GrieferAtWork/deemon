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
#ifndef GUARD_DEX_CTYPES_C_STRING_C
#define GUARD_DEX_CTYPES_C_STRING_C 1
#define DEE_SOURCE 1

#include "libctypes.h"
/**/

#include "c_api.h"
/**/

#include <deemon/arg.h>
#include <deemon/error.h>
#include <deemon/int.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/string.h>
#include <deemon/system-features.h> /* memmem(), tolower(), toupper(), ... */

DECL_BEGIN

#ifndef CONFIG_HAVE_memrchr
#define CONFIG_HAVE_memrchr 1
#define memrchr dee_memrchr
DeeSystem_DEFINE_memrchr(dee_memrchr)
#endif /* !CONFIG_HAVE_memrchr */

#ifndef CONFIG_HAVE_rawmemchr
#define CONFIG_HAVE_rawmemchr 1
#define rawmemchr dee_rawmemchr
DeeSystem_DEFINE_rawmemchr(dee_rawmemchr)
#endif /* !CONFIG_HAVE_rawmemchr */

#ifndef CONFIG_HAVE_strnlen
#define CONFIG_HAVE_strnlen 1
#define strnlen dee_strnlen
DeeSystem_DEFINE_strnlen(strnlen)
#endif /* !CONFIG_HAVE_strnlen */

#ifndef CONFIG_HAVE_memmem
#define CONFIG_HAVE_memmem 1
#define memmem dee_memmem
DeeSystem_DEFINE_memmem(dee_memmem)
#endif /* !CONFIG_HAVE_memmem */

#ifndef CONFIG_HAVE_memrmem
#define CONFIG_HAVE_memrmem 1
#define memrmem dee_memrmem
DeeSystem_DEFINE_memrmem(dee_memrmem)
#endif /* !CONFIG_HAVE_memrmem */

#ifndef CONFIG_HAVE_rawmemrchr
#define CONFIG_HAVE_rawmemrchr 1
#define rawmemrchr dee_rawmemrchr
DeeSystem_DEFINE_rawmemrchr(dee_rawmemrchr)
#endif /* !CONFIG_HAVE_rawmemrchr */

#ifndef CONFIG_HAVE_memend
#define CONFIG_HAVE_memend 1
#define memend dee_memend
DeeSystem_DEFINE_memend(dee_memend)
#endif /* !CONFIG_HAVE_memend */

#ifndef CONFIG_HAVE_memxend
#define CONFIG_HAVE_memxend 1
#define memxend dee_memxend
DeeSystem_DEFINE_memxend(dee_memxend)
#endif /* !CONFIG_HAVE_memxend */

#ifndef CONFIG_HAVE_memrend
#define CONFIG_HAVE_memrend 1
#define memrend dee_memrend
DeeSystem_DEFINE_memrend(dee_memrend)
#endif /* !CONFIG_HAVE_memrend */

#ifndef CONFIG_HAVE_memlen
#define CONFIG_HAVE_memlen 1
#define memlen dee_memlen
DeeSystem_DEFINE_memlen(dee_memlen)
#endif /* !CONFIG_HAVE_memlen */

#ifndef CONFIG_HAVE_memxlen
#define CONFIG_HAVE_memxlen 1
#define memxlen dee_memxlen
DeeSystem_DEFINE_memxlen(dee_memxlen)
#endif /* !CONFIG_HAVE_memxlen */

#ifndef CONFIG_HAVE_memrlen
#define CONFIG_HAVE_memrlen 1
#define memrlen dee_memrlen
DeeSystem_DEFINE_memrlen(dee_memrlen)
#endif /* !CONFIG_HAVE_memrlen */

#ifndef CONFIG_HAVE_rawmemlen
#define CONFIG_HAVE_rawmemlen 1
#define rawmemlen dee_rawmemlen
DeeSystem_DEFINE_rawmemlen(dee_rawmemlen)
#endif /* !CONFIG_HAVE_rawmemlen */

#ifndef CONFIG_HAVE_rawmemrlen
#define CONFIG_HAVE_rawmemrlen 1
#define rawmemrlen dee_rawmemrlen
DeeSystem_DEFINE_rawmemrlen(dee_rawmemrlen)
#endif /* !CONFIG_HAVE_rawmemrlen */

#ifndef CONFIG_HAVE_memxchr
#define CONFIG_HAVE_memxchr 1
#define memxchr dee_memxchr
DeeSystem_DEFINE_memxchr(dee_memxchr)
#endif /* !CONFIG_HAVE_memxchr */

#ifndef CONFIG_HAVE_rawmemxchr
#define CONFIG_HAVE_rawmemxchr 1
#define rawmemxchr dee_rawmemxchr
DeeSystem_DEFINE_rawmemxchr(dee_rawmemxchr)
#endif /* !CONFIG_HAVE_rawmemxchr */

#ifndef CONFIG_HAVE_rawmemxlen
#define CONFIG_HAVE_rawmemxlen 1
#define rawmemxlen dee_rawmemxlen
DeeSystem_DEFINE_rawmemxlen(dee_rawmemxlen)
#endif /* !CONFIG_HAVE_rawmemxlen */

#ifndef CONFIG_HAVE_memcasecmp
#define CONFIG_HAVE_memcasecmp 1
#define memcasecmp dee_memcasecmp
DeeSystem_DEFINE_memcasecmp(dee_memcasecmp)
#endif /* !CONFIG_HAVE_memcasecmp */

#ifndef CONFIG_HAVE_memcasemem
#define CONFIG_HAVE_memcasemem 1
#define memcasemem dee_memcasemem
DeeSystem_DEFINE_memcasemem(dee_memcasemem)
#endif /* !CONFIG_HAVE_memcasemem */

#ifndef CONFIG_HAVE_memrev
#define CONFIG_HAVE_memrev 1
#define memrev dee_memrev
DeeSystem_DEFINE_memrev(dee_memrev)
#endif /* !CONFIG_HAVE_memrev */

#ifndef CONFIG_HAVE_memxrchr
#define CONFIG_HAVE_memxrchr 1
#define memxrchr dee_memxrchr
DeeSystem_DEFINE_memxrchr(dee_memxrchr)
#endif /* !CONFIG_HAVE_memxrchr */

#ifndef CONFIG_HAVE_memxrend
#define CONFIG_HAVE_memxrend 1
#define memxrend dee_memxrend
DeeSystem_DEFINE_memxrend(dee_memxrend)
#endif /* !CONFIG_HAVE_memxrend */

#ifndef CONFIG_HAVE_memxrlen
#define CONFIG_HAVE_memxrlen 1
#define memxrlen dee_memxrlen
DeeSystem_DEFINE_memxrlen(dee_memxrlen)
#endif /* !CONFIG_HAVE_memxrlen */

#ifndef CONFIG_HAVE_rawmemxrchr
#define CONFIG_HAVE_rawmemxrchr 1
#define rawmemxrchr dee_rawmemxrchr
DeeSystem_DEFINE_rawmemxrchr(dee_rawmemxrchr)
#endif /* !CONFIG_HAVE_rawmemxrchr */

#ifndef CONFIG_HAVE_rawmemxrlen
#define CONFIG_HAVE_rawmemxrlen 1
#define rawmemxrlen dee_rawmemxrlen
DeeSystem_DEFINE_rawmemxrlen(dee_rawmemxrlen)
#endif /* !CONFIG_HAVE_rawmemxrlen */

#ifndef CONFIG_HAVE_memcasermem
#define CONFIG_HAVE_memcasermem 1
#define memcasermem dee_memcasermem
DeeSystem_DEFINE_memcasermem(dee_memcasermem)
#endif /* !CONFIG_HAVE_memcasermem */

#ifndef CONFIG_HAVE_strnend
#define CONFIG_HAVE_strnend 1
#define strnend(x, maxlen) ((x) + strnlen(x, maxlen))
#endif /* !CONFIG_HAVE_strnend */

#ifndef CONFIG_HAVE_strrchr
#define CONFIG_HAVE_strrchr 1
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
#define CONFIG_HAVE_strnchr 1
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
#define CONFIG_HAVE_strnrchr 1
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


INTERN WUNUSED DREF DeeObject *DCALL
capi_memcpy(size_t argc, DeeObject *const *argv) {
	DREF DeeObject *ob_dst, *ob_src;
	union pointer dst, src;
	size_t num_bytes;
	if (DeeArg_Unpack(argc, argv, "oo" UNPuSIZ ":memcpy", &ob_dst, &ob_src, &num_bytes))
		goto err;
	if (DeeObject_AsPointer(ob_dst, &DeeCVoid_Type, &dst))
		goto err;
	if (DeeObject_AsPointer(ob_src, &DeeCVoid_Type, &src))
		goto err;
	CTYPES_PROTECTED_MEMCPY(dst.ptr, src.ptr, num_bytes, goto err);
	return DeePointer_NewVoid(dst.ptr);
err:
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_mempcpy(size_t argc, DeeObject *const *argv) {
	DREF DeeObject *ob_dst, *ob_src;
	union pointer dst, src;
	size_t num_bytes;
	if (DeeArg_Unpack(argc, argv, "oo" UNPuSIZ ":mempcpy", &ob_dst, &ob_src, &num_bytes))
		goto err;
	if (DeeObject_AsPointer(ob_dst, &DeeCVoid_Type, &dst))
		goto err;
	if (DeeObject_AsPointer(ob_src, &DeeCVoid_Type, &src))
		goto err;
	CTYPES_PROTECTED_MEMCPY(dst.ptr, src.ptr, num_bytes, goto err);
	return DeePointer_NewVoid((void *)(dst.uint + num_bytes));
err:
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_memccpy(size_t argc, DeeObject *const *argv) {
	DREF DeeObject *ob_dst, *ob_src;
	union pointer dst, src;
	int needle;
	size_t num_bytes;
	if (DeeArg_Unpack(argc, argv, "ood" UNPuSIZ ":memccpy", &ob_dst, &ob_src, &needle, &num_bytes))
		goto err;
	if (DeeObject_AsPointer(ob_dst, &DeeCVoid_Type, &dst))
		goto err;
	if (DeeObject_AsPointer(ob_src, &DeeCVoid_Type, &src))
		goto err;
	CTYPES_PROTECTED(
	dst.ptr = memccpy(dst.ptr, src.ptr, needle, num_bytes), {
		while (num_bytes--) {
			uint8_t byte = *src.p8++;
			*dst.p8++    = byte;
			if (byte == (uint8_t)needle)
				break;
		}
	},
	goto err);
	--dst.p8;
	return DeePointer_NewVoid(dst.ptr);
err:
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_memset(size_t argc, DeeObject *const *argv) {
	DREF DeeObject *ob_dst;
	int byte;
	union pointer dst;
	size_t num_bytes;
	if (DeeArg_Unpack(argc, argv, "od" UNPuSIZ ":memset", &ob_dst, &byte, &num_bytes))
		goto err;
	if (DeeObject_AsPointer(ob_dst, &DeeCVoid_Type, &dst))
		goto err;
	CTYPES_PROTECTED(
	memset(dst.ptr, byte, num_bytes), {
		uint8_t *iter = dst.p8;
		while (num_bytes--)
			*iter++ = (uint8_t)byte;
	},
	goto err);
	return DeePointer_NewVoid(dst.ptr);
err:
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_mempset(size_t argc, DeeObject *const *argv) {
	DREF DeeObject *ob_dst;
	int byte;
	union pointer dst;
	size_t num_bytes;
	if (DeeArg_Unpack(argc, argv, "od" UNPuSIZ ":mempset", &ob_dst, &byte, &num_bytes))
		goto err;
	if (DeeObject_AsPointer(ob_dst, &DeeCVoid_Type, &dst))
		goto err;
	CTYPES_PROTECTED(
	memset(dst.ptr, byte, num_bytes), {
		while (num_bytes--)
			*dst.p8++ = (uint8_t)byte;
	},
	goto err);
	return DeePointer_NewVoid((void *)(dst.uint + num_bytes));
err:
	return NULL;
}


INTERN WUNUSED DREF DeeObject *DCALL
capi_memmove(size_t argc, DeeObject *const *argv) {
	DREF DeeObject *ob_dst, *ob_src;
	union pointer dst, src;
	size_t num_bytes;
	if (DeeArg_Unpack(argc, argv, "oo" UNPuSIZ ":memmove", &ob_dst, &ob_src, &num_bytes))
		goto err;
	if (DeeObject_AsPointer(ob_dst, &DeeCVoid_Type, &dst))
		goto err;
	if (DeeObject_AsPointer(ob_src, &DeeCVoid_Type, &src))
		goto err;
	CTYPES_PROTECTED(
	memmove(dst.ptr, src.ptr, num_bytes), {
		uint8_t *iter;
		uint8_t *end;
		if (dst.p8 < src.p8) {
			end = (iter = dst.p8) + num_bytes;
			while (iter < end)
				*iter++ = *src.p8++;
		} else {
			iter = (end = dst.p8) + num_bytes;
			src.p8 += num_bytes;
			while (iter < end)
				*--iter = *--src.p8;
		}
	},
	goto err);
	return DeePointer_NewVoid(dst.ptr);
err:
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_mempmove(size_t argc, DeeObject *const *argv) {
	DREF DeeObject *ob_dst, *ob_src;
	union pointer dst, src;
	size_t num_bytes;
	if (DeeArg_Unpack(argc, argv, "oo" UNPuSIZ ":mempmove", &ob_dst, &ob_src, &num_bytes))
		goto err;
	if (DeeObject_AsPointer(ob_dst, &DeeCVoid_Type, &dst))
		goto err;
	if (DeeObject_AsPointer(ob_src, &DeeCVoid_Type, &src))
		goto err;
	CTYPES_PROTECTED(
	memmove(dst.ptr, src.ptr, num_bytes), {
		uint8_t *iter;
		uint8_t *end;
		if (dst.p8 < src.p8) {
			end = (iter = dst.p8) + num_bytes;
			while (iter < end)
				*iter++ = *src.p8++;
		} else {
			iter = (end = dst.p8) + num_bytes;
			src.p8 += num_bytes;
			while (iter < end)
				*--iter = *--src.p8;
		}
	},
	goto err);
	return DeePointer_NewVoid(dst.p8 + num_bytes);
err:
	return NULL;
}





INTERN WUNUSED DREF DeeObject *DCALL
capi_memchr(size_t argc, DeeObject *const *argv) {
	DREF DeeObject *ob_haystack;
	union pointer result;
	union pointer haystack;
	int needle;
	size_t num_bytes;
	if (DeeArg_Unpack(argc, argv, "od" UNPuSIZ ":memchr", &ob_haystack, &needle, &num_bytes))
		goto err;
	if (DeeObject_AsPointer(ob_haystack, &DeeCVoid_Type, &haystack))
		goto err;
	CTYPES_PROTECTED(
	result.ptr = memchr(haystack.ptr, needle, num_bytes), {
		result.ptr = NULL;
		for (; num_bytes--; ++haystack.p8) {
			if (*haystack.p8 == (uint8_t)needle) {
				result.ptr = haystack.ptr;
				break;
			}
		}
	},
	goto err);
	return DeePointer_NewVoid(result.ptr);
err:
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_memrchr(size_t argc, DeeObject *const *argv) {
	DREF DeeObject *ob_haystack;
	union pointer result;
	union pointer haystack;
	int needle;
	size_t num_bytes;
	if (DeeArg_Unpack(argc, argv, "od" UNPuSIZ ":memrchr", &ob_haystack, &needle, &num_bytes))
		goto err;
	if (DeeObject_AsPointer(ob_haystack, &DeeCVoid_Type, &haystack))
		goto err;
	CTYPES_PROTECTED(
	result.ptr = memrchr(haystack.ptr, needle, num_bytes), {
		uint8_t *iter = haystack.p8 + num_bytes;
		result.ptr = NULL;
		while (iter != haystack.p8) {
			if (*--iter == (uint8_t)needle) {
				result.ptr = iter;
				break;
			}
		}
	},
	goto err);
	return DeePointer_NewVoid(result.ptr);
err:
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_memend(size_t argc, DeeObject *const *argv) {
	DREF DeeObject *ob_haystack;
	union pointer result;
	union pointer haystack;
	int needle;
	size_t num_bytes;
	if (DeeArg_Unpack(argc, argv, "od" UNPuSIZ ":memend", &ob_haystack, &needle, &num_bytes))
		goto err;
	if (DeeObject_AsPointer(ob_haystack, &DeeCVoid_Type, &haystack))
		goto err;
	CTYPES_PROTECTED(
	result.ptr = memend(haystack.ptr, needle, num_bytes), {
		result.p8 = haystack.p8;
		for (; num_bytes--; ++result.p8) {
			if (*result.p8 == (uint8_t)needle)
				break;
		}
	},
	goto err);
	return DeePointer_NewVoid(result.ptr);
err:
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_memrend(size_t argc, DeeObject *const *argv) {
	DREF DeeObject *ob_haystack;
	union pointer result;
	union pointer haystack;
	int needle;
	size_t num_bytes;
	if (DeeArg_Unpack(argc, argv, "od" UNPuSIZ ":memrend", &ob_haystack, &needle, &num_bytes))
		goto err;
	if (DeeObject_AsPointer(ob_haystack, &DeeCVoid_Type, &haystack))
		goto err;
	CTYPES_PROTECTED(
	result.ptr = memrend(haystack.ptr, needle, num_bytes), {
		result.p8 = haystack.p8;
		result.p8 += num_bytes;
		while (num_bytes--) {
			if (*--result.p8 == (uint8_t)needle)
				break;
		}
	},
	goto err);
	return DeePointer_NewVoid(result.ptr);
err:
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_memlen(size_t argc, DeeObject *const *argv) {
	DREF DeeObject *ob_haystack;
	size_t result;
	union pointer haystack;
	int needle;
	size_t num_bytes;
	if (DeeArg_Unpack(argc, argv, "od" UNPuSIZ ":memlen", &ob_haystack, &needle, &num_bytes))
		goto err;
	if (DeeObject_AsPointer(ob_haystack, &DeeCVoid_Type, &haystack))
		goto err;
	CTYPES_PROTECTED(
	result = memlen(haystack.ptr, needle, num_bytes), {
		uint8_t *iter = haystack.p8;
		for (; num_bytes--; ++iter) {
			if (*iter == (uint8_t)needle)
				break;
		}
		result = (size_t)(iter - haystack.p8);
	},
	goto err);
	return DeeInt_NewSize(result);
err:
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_memrlen(size_t argc, DeeObject *const *argv) {
	DREF DeeObject *ob_haystack;
	size_t result;
	union pointer haystack;
	int needle;
	size_t num_bytes;
	if (DeeArg_Unpack(argc, argv, "od" UNPuSIZ ":memrlen", &ob_haystack, &needle, &num_bytes))
		goto err;
	if (DeeObject_AsPointer(ob_haystack, &DeeCVoid_Type, &haystack))
		goto err;
	CTYPES_PROTECTED(
	result = memrlen(haystack.ptr, needle, num_bytes), {
		uint8_t *iter = haystack.p8 + num_bytes;
		while (num_bytes--) {
			if (*--iter == (uint8_t)needle)
				break;
		}
		result = (size_t)(iter - haystack.p8);
	},
	goto err);
	return DeeInt_NewSize(result);
err:
	return NULL;
}

INTDEF WUNUSED DREF DeeObject *DCALL
capi_rawmemchr(size_t argc, DeeObject *const *argv) {
	DREF DeeObject *ob_haystack;
	union pointer result;
	union pointer haystack;
	int needle;
	if (DeeArg_Unpack(argc, argv, "od:rawmemchr", &ob_haystack, &needle))
		goto err;
	if (DeeObject_AsPointer(ob_haystack, &DeeCVoid_Type, &haystack))
		goto err;
	CTYPES_PROTECTED(
	result.ptr = rawmemchr(haystack.ptr, needle), {
		result.p8 = haystack.p8;
		for (;; ++result.p8) {
			if (*result.p8 == (uint8_t)needle)
				break;
		}
	},
	goto err);
	return DeePointer_NewVoid(result.ptr);
err:
	return NULL;
}

INTDEF WUNUSED DREF DeeObject *DCALL
capi_rawmemrchr(size_t argc, DeeObject *const *argv) {
	DREF DeeObject *ob_haystack;
	union pointer result;
	union pointer haystack;
	int needle;
	if (DeeArg_Unpack(argc, argv, "od:rawmemrchr", &ob_haystack, &needle))
		goto err;
	if (DeeObject_AsPointer(ob_haystack, &DeeCVoid_Type, &haystack))
		goto err;
	CTYPES_PROTECTED(
	result.ptr = rawmemrchr(haystack.ptr, needle), {
		result.p8 = haystack.p8;
		for (;;) {
			if (*--result.p8 == (uint8_t)needle)
				break;
		}
	},
	goto err);
	return DeePointer_NewVoid(result.ptr);
err:
	return NULL;
}

INTDEF WUNUSED DREF DeeObject *DCALL
capi_rawmemlen(size_t argc, DeeObject *const *argv) {
	DREF DeeObject *ob_haystack;
	size_t result;
	union pointer haystack;
	int needle;
	if (DeeArg_Unpack(argc, argv, "od:rawmemlen", &ob_haystack, &needle))
		goto err;
	if (DeeObject_AsPointer(ob_haystack, &DeeCVoid_Type, &haystack))
		goto err;
	CTYPES_PROTECTED(
	result = rawmemlen(haystack.ptr, needle), {
		uint8_t *iter = haystack.p8;
		for (;; ++iter) {
			if (*iter == (uint8_t)needle)
				break;
		}
		result = (size_t)(iter - haystack.p8);
	},
	goto err);
	return DeeInt_NewSize(result);
err:
	return NULL;
}

INTDEF WUNUSED DREF DeeObject *DCALL
capi_rawmemrlen(size_t argc, DeeObject *const *argv) {
	DREF DeeObject *ob_haystack;
	size_t result;
	union pointer haystack;
	int needle;
	if (DeeArg_Unpack(argc, argv, "od:rawmemrlen", &ob_haystack, &needle))
		goto err;
	if (DeeObject_AsPointer(ob_haystack, &DeeCVoid_Type, &haystack))
		goto err;
	CTYPES_PROTECTED(
	result = rawmemrlen(haystack.ptr, needle), {
		uint8_t *iter = haystack.p8;
		for (;;) {
			if (*--iter == (uint8_t)needle)
				break;
		}
		result = (size_t)(iter - haystack.p8);
	},
	goto err);
	return DeeInt_NewSize(result);
err:
	return NULL;
}


INTERN WUNUSED DREF DeeObject *DCALL
capi_memxchr(size_t argc, DeeObject *const *argv) {
	DREF DeeObject *ob_haystack;
	union pointer result;
	union pointer haystack;
	int needle;
	size_t num_bytes;
	if (DeeArg_Unpack(argc, argv, "od" UNPuSIZ ":memxchr", &ob_haystack, &needle, &num_bytes))
		goto err;
	if (DeeObject_AsPointer(ob_haystack, &DeeCVoid_Type, &haystack))
		goto err;
	CTYPES_PROTECTED(
	result.ptr = memxchr(haystack.ptr, needle, num_bytes), {
		result.ptr = NULL;
		for (; num_bytes--; ++haystack.p8) {
			if (*haystack.p8 != (uint8_t)needle) {
				result.ptr = haystack.ptr;
				break;
			}
		}
	},
	goto err);
	return DeePointer_NewVoid(result.ptr);
err:
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_memxrchr(size_t argc, DeeObject *const *argv) {
	DREF DeeObject *ob_haystack;
	union pointer result;
	union pointer haystack;
	int needle;
	size_t num_bytes;
	if (DeeArg_Unpack(argc, argv, "od" UNPuSIZ ":memxrchr", &ob_haystack, &needle, &num_bytes))
		goto err;
	if (DeeObject_AsPointer(ob_haystack, &DeeCVoid_Type, &haystack))
		goto err;
	CTYPES_PROTECTED(
	result.ptr = memxrchr(haystack.ptr, needle, num_bytes), {
		uint8_t *iter = haystack.p8 + num_bytes;
		result.ptr = NULL;
		while (iter != haystack.p8) {
			if (*--iter != (uint8_t)needle) {
				result.ptr = iter;
				break;
			}
		}
	},
	goto err);
	return DeePointer_NewVoid(result.ptr);
err:
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_memxend(size_t argc, DeeObject *const *argv) {
	DREF DeeObject *ob_haystack;
	union pointer result;
	union pointer haystack;
	int needle;
	size_t num_bytes;
	if (DeeArg_Unpack(argc, argv, "od" UNPuSIZ ":memxend", &ob_haystack, &needle, &num_bytes))
		goto err;
	if (DeeObject_AsPointer(ob_haystack, &DeeCVoid_Type, &haystack))
		goto err;
	CTYPES_PROTECTED(
	result.ptr = memxend(haystack.ptr, needle, num_bytes), {
		result.p8 = haystack.p8;
		for (; num_bytes--; ++result.p8) {
			if (*result.p8 != (uint8_t)needle)
				break;
		}
	},
	goto err);
	return DeePointer_NewVoid(result.ptr);
err:
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_memxrend(size_t argc, DeeObject *const *argv) {
	DREF DeeObject *ob_haystack;
	union pointer result;
	union pointer haystack;
	int needle;
	size_t num_bytes;
	if (DeeArg_Unpack(argc, argv, "od" UNPuSIZ ":memxrend", &ob_haystack, &needle, &num_bytes))
		goto err;
	if (DeeObject_AsPointer(ob_haystack, &DeeCVoid_Type, &haystack))
		goto err;
	CTYPES_PROTECTED(
	result.ptr = memxrend(haystack.ptr, needle, num_bytes), {
		result.p8 = haystack.p8 + num_bytes;
		while (num_bytes--) {
			if (*--result.p8 != (uint8_t)needle)
				break;
		}
	},
	goto err);
	return DeePointer_NewVoid(result.ptr);
err:
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_memxlen(size_t argc, DeeObject *const *argv) {
	DREF DeeObject *ob_haystack;
	size_t result;
	union pointer haystack;
	int needle;
	size_t num_bytes;
	if (DeeArg_Unpack(argc, argv, "od" UNPuSIZ ":memxlen", &ob_haystack, &needle, &num_bytes))
		goto err;
	if (DeeObject_AsPointer(ob_haystack, &DeeCVoid_Type, &haystack))
		goto err;
	CTYPES_PROTECTED(
	result = memxlen(haystack.ptr, needle, num_bytes), {
		uint8_t *iter = haystack.p8;
		for (; num_bytes--; ++iter) {
			if (*iter != (uint8_t)needle)
				break;
		}
		result = (size_t)(iter - haystack.p8);
	},
	goto err);
	return DeeInt_NewSize(result);
err:
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_memxrlen(size_t argc, DeeObject *const *argv) {
	DREF DeeObject *ob_haystack;
	size_t result;
	union pointer haystack;
	int needle;
	size_t num_bytes;
	if (DeeArg_Unpack(argc, argv, "od" UNPuSIZ ":memxrlen", &ob_haystack, &needle, &num_bytes))
		goto err;
	if (DeeObject_AsPointer(ob_haystack, &DeeCVoid_Type, &haystack))
		goto err;
	CTYPES_PROTECTED(
	result = memxrlen(haystack.ptr, needle, num_bytes), {
		uint8_t *iter = haystack.p8 + num_bytes;
		while (num_bytes--) {
			if (*--iter != (uint8_t)needle)
				break;
		}
		result = (size_t)(iter - haystack.p8);
	},
	goto err);
	return DeeInt_NewSize(result);
err:
	return NULL;
}

INTDEF WUNUSED DREF DeeObject *DCALL
capi_rawmemxchr(size_t argc, DeeObject *const *argv) {
	DREF DeeObject *ob_haystack;
	union pointer result;
	union pointer haystack;
	int needle;
	if (DeeArg_Unpack(argc, argv, "od:rawmemxchr", &ob_haystack, &needle))
		goto err;
	if (DeeObject_AsPointer(ob_haystack, &DeeCVoid_Type, &haystack))
		goto err;
	CTYPES_PROTECTED(
	result.ptr = rawmemxchr(haystack.ptr, needle), {
		result.p8 = haystack.p8;
		for (;; ++result.p8) {
			if (*result.p8 != (uint8_t)needle)
				break;
		}
	},
	goto err);
	return DeePointer_NewVoid(result.ptr);
err:
	return NULL;
}

INTDEF WUNUSED DREF DeeObject *DCALL
capi_rawmemxrchr(size_t argc, DeeObject *const *argv) {
	DREF DeeObject *ob_haystack;
	union pointer result;
	union pointer haystack;
	int needle;
	if (DeeArg_Unpack(argc, argv, "od:rawmemxrchr", &ob_haystack, &needle))
		goto err;
	if (DeeObject_AsPointer(ob_haystack, &DeeCVoid_Type, &haystack))
		goto err;
	CTYPES_PROTECTED(
	result.ptr = rawmemxrchr(haystack.ptr, needle), {
		result.p8 = haystack.p8;
		for (;;) {
			if (*--result.p8 != (uint8_t)needle)
				break;
		}
	},
	goto err);
	return DeePointer_NewVoid(result.ptr);
err:
	return NULL;
}

INTDEF WUNUSED DREF DeeObject *DCALL
capi_rawmemxlen(size_t argc, DeeObject *const *argv) {
	DREF DeeObject *ob_haystack;
	size_t result;
	union pointer haystack;
	int needle;
	if (DeeArg_Unpack(argc, argv, "od:rawmemxlen", &ob_haystack, &needle))
		goto err;
	if (DeeObject_AsPointer(ob_haystack, &DeeCVoid_Type, &haystack))
		goto err;
	CTYPES_PROTECTED(
	result = rawmemxlen(haystack.ptr, needle), {
		uint8_t *iter = haystack.p8;
		for (;; ++iter) {
			if (*iter != (uint8_t)needle)
				break;
		}
		result = (size_t)(iter - haystack.p8);
	},
	goto err);
	return DeeInt_NewSize(result);
err:
	return NULL;
}

INTDEF WUNUSED DREF DeeObject *DCALL
capi_rawmemxrlen(size_t argc, DeeObject *const *argv) {
	DREF DeeObject *ob_haystack;
	size_t result;
	union pointer haystack;
	int needle;
	if (DeeArg_Unpack(argc, argv, "od:rawmemxrlen", &ob_haystack, &needle))
		goto err;
	if (DeeObject_AsPointer(ob_haystack, &DeeCVoid_Type, &haystack))
		goto err;
	CTYPES_PROTECTED(
	result = rawmemxrlen(haystack.ptr, needle), {
		uint8_t *iter = haystack.p8;
		for (;;) {
			if (*--iter != (uint8_t)needle)
				break;
		}
		result = (size_t)(iter - haystack.p8);
	},
	goto err);
	return DeeInt_NewSize(result);
err:
	return NULL;
}



INTDEF WUNUSED DREF DeeObject *DCALL
capi_memcmp(size_t argc, DeeObject *const *argv) {
	DREF DeeObject *ob_a, *ob_b;
	int result;
	union pointer a, b;
	size_t num_bytes;
	if (DeeArg_Unpack(argc, argv, "oo" UNPuSIZ ":memcmp", &ob_a, &ob_b, &num_bytes))
		goto err;
	if (DeeObject_AsPointer(ob_a, &DeeCVoid_Type, &a))
		goto err;
	if (DeeObject_AsPointer(ob_b, &DeeCVoid_Type, &b))
		goto err;
	CTYPES_PROTECTED(
	result = memcmp(a.ptr, b.ptr, num_bytes), {
		uint8_t av;
		uint8_t bv;
		av = bv = 0;
		while (num_bytes-- && ((av = *a.p8++) == (bv = *b.p8++)))
			;
		result = (int)av - (int)bv;
	},
	goto err);
	return DeeInt_NewInt(result);
err:
	return NULL;
}

INTDEF WUNUSED DREF DeeObject *DCALL
capi_memcasecmp(size_t argc, DeeObject *const *argv) {
	DREF DeeObject *ob_a, *ob_b;
	int result;
	union pointer a, b;
	size_t num_bytes;
	if (DeeArg_Unpack(argc, argv, "oo" UNPuSIZ ":memcasecmp", &ob_a, &ob_b, &num_bytes))
		goto err;
	if (DeeObject_AsPointer(ob_a, &DeeCVoid_Type, &a))
		goto err;
	if (DeeObject_AsPointer(ob_b, &DeeCVoid_Type, &b))
		goto err;
	CTYPES_PROTECTED(
	result = memcasecmp(a.ptr, b.ptr, num_bytes), {
		uint8_t av;
		uint8_t bv;
		av = bv = 0;
		while (num_bytes-- &&
		       (((av = *a.p8++) == (bv = *b.p8++)) ||
		        (av = tolower(av), bv = tolower(bv),
		         av == bv)))
			;
		result = (int)av - (int)bv;
	},
	goto err);
	return DeeInt_NewInt(result);
err:
	return NULL;
}

INTDEF WUNUSED DREF DeeObject *DCALL
capi_memmem(size_t argc, DeeObject *const *argv) {
	DREF DeeObject *ob_a, *ob_b;
	union pointer result;
	union pointer a, b;
	size_t haystack_len, needle_len;
	if (DeeArg_Unpack(argc, argv, "o" UNPuSIZ "o" UNPuSIZ ":memmem",
	                  &ob_a, &haystack_len, &ob_b, &needle_len))
		goto err;
	if (DeeObject_AsPointer(ob_a, &DeeCVoid_Type, &a))
		goto err;
	if (DeeObject_AsPointer(ob_b, &DeeCVoid_Type, &b))
		goto err;
	CTYPES_PROTECTED(
	result.ptr = memmem(a.ptr, haystack_len, b.ptr, needle_len), {
		void const *candidate;
		uint8_t marker;
		result.ptr = NULL;
		if unlikely(!needle_len)
			result.ptr = a.ptr;
		else if unlikely(!needle_len > haystack_len)
			;
		else {
			haystack_len -= needle_len;
			marker = *b.p8;
			for (;;) {
				uint8_t *iter = a.p8;
				uint8_t *iter2;
				uint8_t av;
				uint8_t bv;
				size_t temp = haystack_len;
				candidate   = NULL;
				for (; temp--; ++iter) {
					if (*iter == marker) {
						candidate = iter;
						break;
					}
				}
				if (!candidate)
					break;
				av = bv = 0;
				temp    = needle_len;
				iter    = (uint8_t *)candidate;
				iter2   = b.p8;
				while (temp-- && ((av = *iter++) == (bv = *iter2++)))
					;
				if (av == bv) {
					result.ptr = (void *)candidate;
					break;
				}
				haystack_len = ((uintptr_t)a.p8 + haystack_len) - (uintptr_t)candidate;
				a.ptr        = (void *)((uintptr_t)candidate + 1);
			}
		}
	},
	goto err);
	return DeePointer_NewVoid(result.ptr);
err:
	return NULL;
}


INTDEF WUNUSED DREF DeeObject *DCALL
capi_memcasemem(size_t argc, DeeObject *const *argv) {
	DREF DeeObject *ob_a, *ob_b;
	union pointer result;
	union pointer a, b;
	size_t haystack_len, needle_len;
	if (DeeArg_Unpack(argc, argv, "o" UNPuSIZ "o" UNPuSIZ ":memcasemem", &ob_a, &haystack_len, &ob_b, &needle_len))
		goto err;
	if (DeeObject_AsPointer(ob_a, &DeeCVoid_Type, &a))
		goto err;
	if (DeeObject_AsPointer(ob_b, &DeeCVoid_Type, &b))
		goto err;
	CTYPES_PROTECTED(
	result.ptr = memcasemem(a.ptr, haystack_len, b.ptr, needle_len), {
		void const *candidate;
		uint8_t marker1;
		uint8_t marker2;
		result.ptr = NULL;
		if unlikely(!needle_len)
			result.ptr = a.ptr;
		else if unlikely(!needle_len > haystack_len)
			;
		else {
			haystack_len -= needle_len;
			marker1 = (uint8_t)tolower(*b.p8);
			marker2 = (uint8_t)toupper(marker1);
			for (;;) {
				uint8_t *iter = a.p8;
				uint8_t *iter2;
				uint8_t av;
				uint8_t bv;
				size_t temp = haystack_len;
				candidate   = NULL;
				for (; temp--; ++iter) {
					if (*iter == marker1 || *iter == marker2) {
						candidate = iter;
						break;
					}
				}
				if (!candidate)
					break;
				av = bv = 0;
				temp    = needle_len;
				iter    = (uint8_t *)candidate;
				iter2   = b.p8;
				while (temp-- &&
				       (((av = *iter++) == (bv = *iter2++)) ||
				        (av = tolower(av), bv = tolower(bv),
				         av == bv)))
					;
				if (av == bv) {
					result.ptr = (void *)candidate;
					break;
				}
				haystack_len = ((uintptr_t)a.p8 + haystack_len) - (uintptr_t)candidate;
				a.ptr        = (void *)((uintptr_t)candidate + 1);
			}
		}
	},
	goto err);
	return DeePointer_NewVoid(result.ptr);
err:
	return NULL;
}



INTDEF WUNUSED DREF DeeObject *DCALL
capi_memrmem(size_t argc, DeeObject *const *argv) {
	DREF DeeObject *ob_a, *ob_b;
	union pointer result;
	union pointer a, b;
	size_t haystack_len, needle_len;
	if (DeeArg_Unpack(argc, argv, "o" UNPuSIZ "o" UNPuSIZ ":memrmem", &ob_a, &haystack_len, &ob_b, &needle_len))
		goto err;
	if (DeeObject_AsPointer(ob_a, &DeeCVoid_Type, &a))
		goto err;
	if (DeeObject_AsPointer(ob_b, &DeeCVoid_Type, &b))
		goto err;
	CTYPES_PROTECTED(
	result.ptr = memrmem(a.ptr, haystack_len, b.ptr, needle_len), {
		void const *candidate;
		uint8_t marker;
		result.ptr = NULL;
		if unlikely(!needle_len)
			result.ptr = a.p8 + haystack_len;
		else if unlikely(!needle_len > haystack_len)
			;
		else {
			haystack_len -= needle_len - 1;
			marker = *(uint8_t *)b.ptr;
			for (;;) {
				uint8_t *iter = a.p8 + haystack_len;
				uint8_t *iter2;
				uint8_t av;
				uint8_t bv;
				size_t temp;
				while (iter != a.p8) {
					if (*--iter == (uint8_t)marker)
						break;
				}
				if (iter == a.p8)
					break;
				candidate = iter;
				av = bv = 0;
				temp    = needle_len;
				iter    = (uint8_t *)candidate;
				iter2   = b.p8;
				while (temp-- && ((av = *iter++) == (bv = *iter2++)))
					;
				if (av == bv) {
					result.ptr = (void *)candidate;
					break;
				}
				if unlikely(candidate == b.ptr)
					break;
				haystack_len = (uintptr_t)candidate - (uintptr_t)a.ptr;
			}
		}
	},
	goto err);
	return DeePointer_NewVoid(result.ptr);
err:
	return NULL;
}



INTDEF WUNUSED DREF DeeObject *DCALL
capi_memcasermem(size_t argc, DeeObject *const *argv) {
	DREF DeeObject *ob_a, *ob_b;
	union pointer result;
	union pointer a, b;
	size_t haystack_len, needle_len;
	if (DeeArg_Unpack(argc, argv, "o" UNPuSIZ "o" UNPuSIZ ":memcasermem", &ob_a, &haystack_len, &ob_b, &needle_len))
		goto err;
	if (DeeObject_AsPointer(ob_a, &DeeCVoid_Type, &a))
		goto err;
	if (DeeObject_AsPointer(ob_b, &DeeCVoid_Type, &b))
		goto err;
	CTYPES_PROTECTED(
	result.ptr = memcasermem(a.ptr, haystack_len, b.ptr, needle_len), {
		void const *candidate;
		uint8_t marker1;
		uint8_t marker2;
		result.ptr = NULL;
		if unlikely(!needle_len)
			result.ptr = a.p8 + haystack_len;
		else if unlikely(!needle_len > haystack_len)
			;
		else {
			haystack_len -= needle_len - 1;
			marker1 = (uint8_t)tolower(*(uint8_t *)b.ptr);
			marker2 = (uint8_t)toupper(marker1);
			for (;;) {
				uint8_t *iter = a.p8 + haystack_len;
				uint8_t *iter2;
				uint8_t av;
				uint8_t bv;
				size_t temp;
				while (iter != a.p8) {
					--iter;
					if (*iter == (uint8_t)marker1)
						break;
					if (*iter == (uint8_t)marker2)
						break;
				}
				if (iter == a.p8)
					break;
				candidate = iter;
				av = bv = 0;
				temp    = needle_len;
				iter    = (uint8_t *)candidate;
				iter2   = b.p8;
				while (temp-- &&
				       (((av = *iter++) == (bv = *iter2++)) ||
				        (av = tolower(av), bv = tolower(bv),
				         av == bv)))
					;
				if (av == bv) {
					result.ptr = (void *)candidate;
					break;
				}
				if unlikely(candidate == b.ptr)
					break;
				haystack_len = (uintptr_t)candidate - (uintptr_t)a.ptr;
			}
		}
	},
	goto err);
	return DeePointer_NewVoid(result.ptr);
err:
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_memrev(size_t argc, DeeObject *const *argv) {
	DREF DeeObject *ob_dst;
	union pointer dst;
	size_t num_bytes;
	if (DeeArg_Unpack(argc, argv, "o" UNPuSIZ ":memrev", &ob_dst, &num_bytes))
		goto err;
	if (DeeObject_AsPointer(ob_dst, &DeeCVoid_Type, &dst))
		goto err;
	CTYPES_PROTECTED(
	memrev(dst.ptr, num_bytes), {
		uint8_t *iter;
		uint8_t *end;
		end = (iter = (uint8_t *)dst.ptr) + num_bytes;
		while (iter < end) {
			uint8_t temp = *iter;
			*iter++      = *--end;
			*end         = temp;
		}
	},
	goto err);
	return DeePointer_NewVoid(dst.ptr);
err:
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_strlen(size_t argc, DeeObject *const *argv) {
	DeeObject *ob_str;
	union pointer str;
	size_t result;
	if (DeeArg_Unpack(argc, argv, "o:strlen", &ob_str))
		goto err;
	if (DeeObject_AsPointer(ob_str, &DeeCChar_Type, &str))
		goto err;
	CTYPES_PROTECTED_STRLEN(result, str.pchar, goto err);
	return DeeInt_NewSize(result);
err:
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_strend(size_t argc, DeeObject *const *argv) {
	DeeObject *ob_str;
	union pointer str;
	if (DeeArg_Unpack(argc, argv, "o:strend", &ob_str))
		goto err;
	if (DeeObject_AsPointer(ob_str, &DeeCChar_Type, &str))
		goto err;
	CTYPES_PROTECTED(
	str.pchar = strend(str.pchar), {
		while (*str.pchar++)
			;
	},
	goto err);
	return DeePointer_NewFor(&DeeCChar_Type, str.ptr);
err:
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_strnlen(size_t argc, DeeObject *const *argv) {
	DeeObject *ob_str;
	size_t maxlen;
	union pointer str;
	size_t result;
	if (DeeArg_Unpack(argc, argv, "o" UNPuSIZ ":strnlen", &ob_str, &maxlen))
		goto err;
	if (DeeObject_AsPointer(ob_str, &DeeCChar_Type, &str))
		goto err;
	CTYPES_PROTECTED_STRNLEN(result, str.pchar, maxlen, goto err);
	return DeeInt_NewSize(result);
err:
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_strnend(size_t argc, DeeObject *const *argv) {
	DeeObject *ob_str;
	union pointer str;
	size_t maxlen;
	if (DeeArg_Unpack(argc, argv, "o" UNPuSIZ ":strnend", &ob_str, &maxlen))
		goto err;
	if (DeeObject_AsPointer(ob_str, &DeeCChar_Type, &str))
		goto err;
	CTYPES_PROTECTED(
	str.pchar = strnend(str.pchar, maxlen), {
		for (; maxlen && *str.pchar; --maxlen, ++str.pchar)
			;
	},
	goto err);
	return DeePointer_NewFor(&DeeCChar_Type, str.ptr);
err:
	return NULL;
}



INTERN WUNUSED DREF DeeObject *DCALL
capi_strchr(size_t argc, DeeObject *const *argv) {
	DREF DeeObject *ob_dst;
	union pointer result;
	union pointer haystack;
	int needle;
	if (DeeArg_Unpack(argc, argv, "od:strchr", &ob_dst, &needle))
		goto err;
	if (DeeObject_AsPointer(ob_dst, &DeeCChar_Type, &haystack))
		goto err;
	CTYPES_PROTECTED(
	result.pchar = strchr(haystack.pchar, needle), {
		result.pchar = NULL;
		for (;; ++haystack.pchar) {
			char ch = *haystack.pchar;
			if (ch == (char)(unsigned char)needle) {
				result.pchar = haystack.pchar;
				break;
			}
			if (!ch)
				break;
		}
	},
	goto err);
	return DeePointer_NewFor(&DeeCChar_Type, result.pchar);
err:
	return NULL;
}


INTERN WUNUSED DREF DeeObject *DCALL
capi_strrchr(size_t argc, DeeObject *const *argv) {
	DREF DeeObject *ob_dst;
	union pointer result;
	union pointer haystack;
	int needle;
	if (DeeArg_Unpack(argc, argv, "od:strrchr", &ob_dst, &needle))
		goto err;
	if (DeeObject_AsPointer(ob_dst, &DeeCChar_Type, &haystack))
		goto err;
	CTYPES_PROTECTED(
	result.pchar = strrchr(haystack.pchar, needle), {
		result.pchar = NULL;
		for (;; ++haystack.pchar) {
			char ch = *haystack.pchar;
			if (ch == (char)(unsigned char)needle)
				result.pchar = haystack.pchar;
			if (!ch)
				break;
		}
	},
	goto err);
	return DeePointer_NewFor(&DeeCChar_Type, result.pchar);
err:
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_strnchr(size_t argc, DeeObject *const *argv) {
	DREF DeeObject *ob_dst;
	union pointer result;
	union pointer haystack;
	int needle;
	size_t maxlen;
	if (DeeArg_Unpack(argc, argv, "od" UNPuSIZ ":strnchr", &ob_dst, &needle, &maxlen))
		goto err;
	if (DeeObject_AsPointer(ob_dst, &DeeCChar_Type, &haystack))
		goto err;
	CTYPES_PROTECTED(
	result.pchar = strnchr(haystack.pchar, needle, maxlen), {
		result.pchar = NULL;
		for (; maxlen--; ++haystack.pchar) {
			char ch = *haystack.pchar;
			if (ch == (char)(unsigned char)needle) {
				result.pchar = haystack.pchar;
				break;
			}
			if (!ch)
				break;
		}
	},
	goto err);
	return DeePointer_NewFor(&DeeCChar_Type, result.pchar);
err:
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_strnrchr(size_t argc, DeeObject *const *argv) {
	DREF DeeObject *ob_dst;
	union pointer result;
	union pointer haystack;
	int needle;
	size_t maxlen;
	if (DeeArg_Unpack(argc, argv, "od" UNPuSIZ ":strnrchr", &ob_dst, &needle, &maxlen))
		goto err;
	if (DeeObject_AsPointer(ob_dst, &DeeCChar_Type, &haystack))
		goto err;
	CTYPES_PROTECTED(
	result.pchar = strnrchr(haystack.pchar, needle, maxlen), {
		result.pchar = NULL;
		for (; maxlen--; ++haystack.pchar) {
			char ch = *haystack.pchar;
			if (ch == (char)(unsigned char)needle)
				result.pchar = haystack.pchar;
			if (!ch)
				break;
		}
	},
	goto err);
	return DeePointer_NewFor(&DeeCChar_Type, result.pchar);
err:
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_stroff(size_t argc, DeeObject *const *argv) {
	(void)argc;
	(void)argv;
	/* TODO */
	DERROR_NOTIMPLEMENTED();
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_strroff(size_t argc, DeeObject *const *argv) {
	(void)argc;
	(void)argv;
	/* TODO */
	DERROR_NOTIMPLEMENTED();
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_strnoff(size_t argc, DeeObject *const *argv) {
	(void)argc;
	(void)argv;
	/* TODO */
	DERROR_NOTIMPLEMENTED();
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_strnroff(size_t argc, DeeObject *const *argv) {
	(void)argc;
	(void)argv;
	/* TODO */
	DERROR_NOTIMPLEMENTED();
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_strchrnul(size_t argc, DeeObject *const *argv) {
	(void)argc;
	(void)argv;
	/* TODO */
	DERROR_NOTIMPLEMENTED();
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_strrchrnul(size_t argc, DeeObject *const *argv) {
	(void)argc;
	(void)argv;
	/* TODO */
	DERROR_NOTIMPLEMENTED();
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_strnchrnul(size_t argc, DeeObject *const *argv) {
	(void)argc;
	(void)argv;
	/* TODO */
	DERROR_NOTIMPLEMENTED();
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_strnrchrnul(size_t argc, DeeObject *const *argv) {
	(void)argc;
	(void)argv;
	/* TODO */
	DERROR_NOTIMPLEMENTED();
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_strcmp(size_t argc, DeeObject *const *argv) {
	(void)argc;
	(void)argv;
	/* TODO */
	DERROR_NOTIMPLEMENTED();
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_strncmp(size_t argc, DeeObject *const *argv) {
	(void)argc;
	(void)argv;
	/* TODO */
	DERROR_NOTIMPLEMENTED();
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_strcasecmp(size_t argc, DeeObject *const *argv) {
	(void)argc;
	(void)argv;
	/* TODO */
	DERROR_NOTIMPLEMENTED();
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_strncasecmp(size_t argc, DeeObject *const *argv) {
	(void)argc;
	(void)argv;
	/* TODO */
	DERROR_NOTIMPLEMENTED();
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_strcpy(size_t argc, DeeObject *const *argv) {
	(void)argc;
	(void)argv;
	/* TODO */
	DERROR_NOTIMPLEMENTED();
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_strcat(size_t argc, DeeObject *const *argv) {
	(void)argc;
	(void)argv;
	/* TODO */
	DERROR_NOTIMPLEMENTED();
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_strncpy(size_t argc, DeeObject *const *argv) {
	(void)argc;
	(void)argv;
	/* TODO */
	DERROR_NOTIMPLEMENTED();
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_strncat(size_t argc, DeeObject *const *argv) {
	(void)argc;
	(void)argv;
	/* TODO */
	DERROR_NOTIMPLEMENTED();
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_stpcpy(size_t argc, DeeObject *const *argv) {
	(void)argc;
	(void)argv;
	/* TODO */
	DERROR_NOTIMPLEMENTED();
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_stpncpy(size_t argc, DeeObject *const *argv) {
	(void)argc;
	(void)argv;
	/* TODO */
	DERROR_NOTIMPLEMENTED();
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_strstr(size_t argc, DeeObject *const *argv) {
	(void)argc;
	(void)argv;
	/* TODO */
	DERROR_NOTIMPLEMENTED();
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_strcasestr(size_t argc, DeeObject *const *argv) {
	(void)argc;
	(void)argv;
	/* TODO */
	DERROR_NOTIMPLEMENTED();
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_strverscmp(size_t argc, DeeObject *const *argv) {
	(void)argc;
	(void)argv;
	/* TODO */
	DERROR_NOTIMPLEMENTED();
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_strtok(size_t argc, DeeObject *const *argv) {
	(void)argc;
	(void)argv;
	/* TODO */
	DERROR_NOTIMPLEMENTED();
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_index(size_t argc, DeeObject *const *argv) {
	(void)argc;
	(void)argv;
	/* TODO */
	DERROR_NOTIMPLEMENTED();
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_rindex(size_t argc, DeeObject *const *argv) {
	(void)argc;
	(void)argv;
	/* TODO */
	DERROR_NOTIMPLEMENTED();
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_strspn(size_t argc, DeeObject *const *argv) {
	(void)argc;
	(void)argv;
	/* TODO */
	DERROR_NOTIMPLEMENTED();
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_strcspn(size_t argc, DeeObject *const *argv) {
	(void)argc;
	(void)argv;
	/* TODO */
	DERROR_NOTIMPLEMENTED();
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_strpbrk(size_t argc, DeeObject *const *argv) {
	(void)argc;
	(void)argv;
	/* TODO */
	DERROR_NOTIMPLEMENTED();
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_strcoll(size_t argc, DeeObject *const *argv) {
	(void)argc;
	(void)argv;
	/* TODO */
	DERROR_NOTIMPLEMENTED();
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_strncoll(size_t argc, DeeObject *const *argv) {
	(void)argc;
	(void)argv;
	/* TODO */
	DERROR_NOTIMPLEMENTED();
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_strcasecoll(size_t argc, DeeObject *const *argv) {
	(void)argc;
	(void)argv;
	/* TODO */
	DERROR_NOTIMPLEMENTED();
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_strncasecoll(size_t argc, DeeObject *const *argv) {
	(void)argc;
	(void)argv;
	/* TODO */
	DERROR_NOTIMPLEMENTED();
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_strxfrm(size_t argc, DeeObject *const *argv) {
	(void)argc;
	(void)argv;
	/* TODO */
	DERROR_NOTIMPLEMENTED();
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_strrev(size_t argc, DeeObject *const *argv) {
	(void)argc;
	(void)argv;
	/* TODO */
	DERROR_NOTIMPLEMENTED();
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_strnrev(size_t argc, DeeObject *const *argv) {
	(void)argc;
	(void)argv;
	/* TODO */
	DERROR_NOTIMPLEMENTED();
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_strlwr(size_t argc, DeeObject *const *argv) {
	(void)argc;
	(void)argv;
	/* TODO */
	DERROR_NOTIMPLEMENTED();
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_strupr(size_t argc, DeeObject *const *argv) {
	(void)argc;
	(void)argv;
	/* TODO */
	DERROR_NOTIMPLEMENTED();
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_strset(size_t argc, DeeObject *const *argv) {
	(void)argc;
	(void)argv;
	/* TODO */
	DERROR_NOTIMPLEMENTED();
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_strnset(size_t argc, DeeObject *const *argv) {
	(void)argc;
	(void)argv;
	/* TODO */
	DERROR_NOTIMPLEMENTED();
	return NULL;
}

INTERN WUNUSED DREF DeeObject *DCALL
capi_strfry(size_t argc, DeeObject *const *argv) {
	(void)argc;
	(void)argv;
	/* TODO */
	DERROR_NOTIMPLEMENTED();
	return NULL;
}

//INTERN WUNUSED DREF DeeObject *DCALL capi_strsep(size_t argc, DeeObject *const *argv);
//INTERN WUNUSED DREF DeeObject *DCALL capi_strtok_r(size_t argc, DeeObject *const *argv);


DECL_END


#endif /* !GUARD_DEX_CTYPES_C_STRING_C */
