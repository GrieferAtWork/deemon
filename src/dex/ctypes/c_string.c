/* Copyright (c) 2019 Griefer@Work                                            *
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
 *    in a product, an acknowledgement in the product documentation would be  *
 *    appreciated but is not required.                                        *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEX_CTYPES_C_STRING_C
#define GUARD_DEX_CTYPES_C_STRING_C 1
#define DEE_SOURCE 1
#define _KOS_SOURCE 1
#define _GNU_SOURCE 1

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

#ifndef CONFIG_NO_CTYPE
#include <ctype.h>
#else /* !CONFIG_NO_CTYPE */
#define tolower(x) DeeUni_ToLower(x)
#define toupper(x) DeeUni_ToUpper(x)
#endif /* CONFIG_NO_CTYPE */

#include <string.h>

DECL_BEGIN

INTERN DREF DeeObject *DCALL
capi_memcpy(size_t argc, DeeObject **__restrict argv) {
	DREF DeeObject *ob_dst, *ob_src;
	union pointer dst, src;
	size_t num_bytes;
	if (DeeArg_Unpack(argc, argv, "ooIu:memcpy", &ob_dst, &ob_src, &num_bytes))
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

INTERN DREF DeeObject *DCALL
capi_mempcpy(size_t argc, DeeObject **__restrict argv) {
	DREF DeeObject *ob_dst, *ob_src;
	union pointer dst, src;
	size_t num_bytes;
	if (DeeArg_Unpack(argc, argv, "ooIu:mempcpy", &ob_dst, &ob_src, &num_bytes))
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

INTERN DREF DeeObject *DCALL
capi_memccpy(size_t argc, DeeObject **__restrict argv) {
	DREF DeeObject *ob_dst, *ob_src;
	union pointer dst, src;
	int val;
	size_t num_bytes;
	if (DeeArg_Unpack(argc, argv, "oodIu:memccpy", &ob_dst, &ob_src, &val, &num_bytes) ||
	    DeeObject_AsPointer(ob_dst, &DeeCVoid_Type, &dst) ||
	    DeeObject_AsPointer(ob_src, &DeeCVoid_Type, &src))
		goto err;
#ifdef CONFIG_HAVE_CTYPES_FAULTPROTECT
#ifdef CONFIG_HAVE_CTYPES_RECURSIVE_PROTECT
	CTYPES_FAULTPROTECT(dst.ptr = memccpy(dst.ptr, src.ptr, val, num_bytes),
	goto err);
#else /* CONFIG_HAVE_CTYPES_RECURSIVE_PROTECT */
	CTYPES_FAULTPROTECT({
		while (num_bytes--) {
			uint8_t byte = *src.p8++;
			*dst.p8++    = byte;
			if (byte == (uint8_t)val)
				break;
		}
	},
	goto err);
	--dst.p8;
#endif /* !CONFIG_HAVE_CTYPES_RECURSIVE_PROTECT */
#else /* CONFIG_HAVE_CTYPES_FAULTPROTECT */
	dst.ptr = memccpy(dst.ptr, src.ptr, val, num_bytes);
#endif /* !CONFIG_HAVE_CTYPES_FAULTPROTECT */
	return DeePointer_NewVoid(dst.ptr);
err:
	return NULL;
}

INTERN DREF DeeObject *DCALL
capi_memset(size_t argc, DeeObject **__restrict argv) {
	DREF DeeObject *ob_dst;
	int byte;
	union pointer dst;
	size_t num_bytes;
	if (DeeArg_Unpack(argc, argv, "odIu:memset", &ob_dst, &byte, &num_bytes) ||
	    DeeObject_AsPointer(ob_dst, &DeeCVoid_Type, &dst))
		goto err;
#ifdef CONFIG_HAVE_CTYPES_FAULTPROTECT
#ifdef CONFIG_HAVE_CTYPES_RECURSIVE_PROTECT
	CTYPES_FAULTPROTECT(memset(dst.ptr, byte, num_bytes),
	goto err);
#else /* CONFIG_HAVE_CTYPES_RECURSIVE_PROTECT */
	CTYPES_FAULTPROTECT({
		uint8_t *iter = dst.p8;
		while (num_bytes--)
			*iter++ = (uint8_t)byte;
	},
	goto err);
#endif /* !CONFIG_HAVE_CTYPES_RECURSIVE_PROTECT */
#else /* CONFIG_HAVE_CTYPES_FAULTPROTECT */
	memset(dst.ptr, byte, num_bytes);
#endif /* !CONFIG_HAVE_CTYPES_FAULTPROTECT */
	return DeePointer_NewVoid(dst.ptr);
err:
	return NULL;
}

INTERN DREF DeeObject *DCALL
capi_mempset(size_t argc, DeeObject **__restrict argv) {
	DREF DeeObject *ob_dst;
	int byte;
	union pointer dst;
	size_t num_bytes;
	if (DeeArg_Unpack(argc, argv, "odIu:mempset", &ob_dst, &byte, &num_bytes) ||
	    DeeObject_AsPointer(ob_dst, &DeeCVoid_Type, &dst))
		goto err;
#ifdef CONFIG_HAVE_CTYPES_FAULTPROTECT
#ifdef CONFIG_HAVE_CTYPES_RECURSIVE_PROTECT
	CTYPES_FAULTPROTECT(memset(dst.ptr, byte, num_bytes),
	goto err);
#else /* CONFIG_HAVE_CTYPES_RECURSIVE_PROTECT */
	CTYPES_FAULTPROTECT({
		while (num_bytes--)
			*dst.p8++ = (uint8_t)byte;
	},
	goto err);
#endif /* !CONFIG_HAVE_CTYPES_RECURSIVE_PROTECT */
#else /* CONFIG_HAVE_CTYPES_FAULTPROTECT */
	memset(dst.ptr, byte, num_bytes);
#endif /* !CONFIG_HAVE_CTYPES_FAULTPROTECT */
	return DeePointer_NewVoid((void *)(dst.uint + num_bytes));
err:
	return NULL;
}


INTERN DREF DeeObject *DCALL
capi_memmove(size_t argc, DeeObject **__restrict argv) {
	DREF DeeObject *ob_dst, *ob_src;
	union pointer dst, src;
	size_t num_bytes;
	if (DeeArg_Unpack(argc, argv, "ooIu:memmove", &ob_dst, &ob_src, &num_bytes) ||
	    DeeObject_AsPointer(ob_dst, &DeeCVoid_Type, &dst) ||
	    DeeObject_AsPointer(ob_src, &DeeCVoid_Type, &src))
		goto err;
#ifdef CONFIG_HAVE_CTYPES_FAULTPROTECT
#ifdef CONFIG_HAVE_CTYPES_RECURSIVE_PROTECT
	CTYPES_FAULTPROTECT(memmove(dst.ptr, src.ptr, num_bytes),
	goto err);
#else /* CONFIG_HAVE_CTYPES_RECURSIVE_PROTECT */
	CTYPES_FAULTPROTECT({
		uint8_t *iter;
		uint8_t *end;
		if (dst.p8 < src.p8) {
			end = (iter = dst.p8) + num_bytes;
			while (iter != end)
				*iter++ = *src.p8++;
		} else {
			iter = (end = dst.p8) + num_bytes;
			src.p8 += num_bytes;
			while (iter != end)
				*--iter = *--src.p8;
		}
	},
	goto err);
#endif /* !CONFIG_HAVE_CTYPES_RECURSIVE_PROTECT */
#else /* CONFIG_HAVE_CTYPES_FAULTPROTECT */
	memmove(dst.ptr, src.ptr, num_bytes);
#endif /* !CONFIG_HAVE_CTYPES_FAULTPROTECT */
	return DeePointer_NewVoid(dst.ptr);
err:
	return NULL;
}

INTERN DREF DeeObject *DCALL
capi_mempmove(size_t argc, DeeObject **__restrict argv) {
	DREF DeeObject *ob_dst, *ob_src;
	union pointer dst, src;
	size_t num_bytes;
	if (DeeArg_Unpack(argc, argv, "ooIu:mempmove", &ob_dst, &ob_src, &num_bytes) ||
	    DeeObject_AsPointer(ob_dst, &DeeCVoid_Type, &dst) ||
	    DeeObject_AsPointer(ob_src, &DeeCVoid_Type, &src))
		goto err;
#ifdef CONFIG_HAVE_CTYPES_FAULTPROTECT
#ifdef CONFIG_HAVE_CTYPES_RECURSIVE_PROTECT
	CTYPES_FAULTPROTECT(memmove(dst.ptr, src.ptr, num_bytes),
	goto err);
#else /* CONFIG_HAVE_CTYPES_RECURSIVE_PROTECT */
	CTYPES_FAULTPROTECT({
		uint8_t *iter;
		uint8_t *end;
		if (dst.p8 < src.p8) {
			end = (iter = dst.p8) + num_bytes;
			while (iter != end)
				*iter++ = *src.p8++;
		} else {
			iter = (end = dst.p8) + num_bytes;
			src.p8 += num_bytes;
			while (iter != end)
				*--iter = *--src.p8;
		}
	},
	goto err);
#endif /* !CONFIG_HAVE_CTYPES_RECURSIVE_PROTECT */
#else /* CONFIG_HAVE_CTYPES_FAULTPROTECT */
	memmove(dst.ptr, src.ptr, num_bytes);
#endif /* !CONFIG_HAVE_CTYPES_FAULTPROTECT */
	return DeePointer_NewVoid(dst.p8 + num_bytes);
err:
	return NULL;
}


#ifndef __USE_GNU
#define memrchr dee_memrchr
LOCAL void *dee_memrchr(void const *__restrict p, int c, size_t n) {
	uint8_t *iter = (uint8_t *)p + n;
	while (iter != (uint8_t *)p) {
		if (*--iter == (uint8_t)c)
			return iter;
	}
	return NULL;
}
#define rawmemchr dee_rawmemchr
LOCAL void *dee_rawmemchr(void const *__restrict p, int c) {
	uint8_t *haystack = (uint8_t *)p;
	for (;; ++haystack) {
		if (*haystack == (uint8_t)c)
			break;
	}
	return haystack;
}

#ifndef _MSC_VER
#define strnlen dee_strnlen
LOCAL size_t dee_strnlen(char const *__restrict str, size_t maxlen) {
	size_t result;
	for (result = 0; maxlen && *str; --maxlen, ++str, ++result)
		;
	return result;
}
#endif /* !_MSC_VER */
#endif /* !__USE_GNU */

/* linux's memmem() doesn't do what we need it to do. - We
 * need it to return `NULL' when `needle_length' is 0
 * Additionally, there has never been a point where
 * it was working entirely flawless. */
#if !defined(__USE_KOS) || !defined(__USE_GNU)
#define memmem dee_memmem
LOCAL void *dee_memmem(void const *__restrict haystack, size_t haystack_length,
                       void const *__restrict needle, size_t needle_length) {
	uint8_t *candidate;
	uint8_t marker;
	if unlikely(!needle_length || needle_length > haystack_length)
		return NULL;
	haystack_length -= (needle_length - 1), marker = *(uint8_t *)needle;
	while ((candidate = (uint8_t *)memchr(haystack, marker, haystack_length)) != NULL) {
		if (memcmp(candidate, needle, needle_length) == 0)
			return (void *)candidate;
		++candidate;
		haystack_length = ((uint8_t *)haystack + haystack_length) - candidate;
		haystack        = (void const *)candidate;
	}
	return NULL;
}
#endif /* !__KOS__ || !__USE_GNU */


#ifndef __USE_KOS
#define rawmemrchr dee_rawmemrchr
LOCAL void *dee_rawmemrchr(void const *__restrict p, int c) {
	uint8_t *haystack = (uint8_t *)p;
	for (;;) {
		if (*--haystack == (uint8_t)c)
			break;
	}
	return haystack;
}

#define memend dee_memend
LOCAL void *dee_memend(void const *p, int byte, size_t num_bytes) {
	uint8_t *haystack = (uint8_t *)p;
	for (; num_bytes--; ++haystack) {
		if (*haystack == (uint8_t)byte)
			break;
	}
	return haystack;
}

#define memxend dee_memxend
LOCAL void *dee_memxend(void const *p, int byte, size_t num_bytes) {
	uint8_t *haystack = (uint8_t *)p;
	for (; num_bytes--; ++haystack) {
		if (*haystack != (uint8_t)byte)
			break;
	}
	return haystack;
}

#define memrend dee_memrend
LOCAL void *dee_memrend(void const *p, int byte, size_t num_bytes) {
	uint8_t *haystack = (uint8_t *)p;
	haystack += num_bytes;
	while (num_bytes--) {
		if (*--haystack == (uint8_t)byte)
			break;
	}
	return haystack;
}

#define memxrend dee_memxrend
LOCAL void *dee_memxrend(void const *p, int byte, size_t num_bytes) {
	uint8_t *haystack = (uint8_t *)p;
	haystack += num_bytes;
	while (num_bytes--) {
		if (*--haystack != (uint8_t)byte)
			break;
	}
	return haystack;
}

#define memlen dee_memlen
LOCAL size_t dee_memlen(void const *p, int byte, size_t num_bytes) {
	return (size_t)((uintptr_t)memend(p, byte, num_bytes) - (uintptr_t)p);
}

#define memxlen dee_memxlen
LOCAL size_t dee_memxlen(void const *p, int byte, size_t num_bytes) {
	return (size_t)((uintptr_t)memxend(p, byte, num_bytes) - (uintptr_t)p);
}

#define memrlen dee_memrlen
LOCAL size_t dee_memrlen(void const *p, int byte, size_t num_bytes) {
	return (size_t)((uintptr_t)memrend(p, byte, num_bytes) - (uintptr_t)p);
}

#define memxrlen dee_memxrlen
LOCAL size_t dee_memxrlen(void const *p, int byte, size_t num_bytes) {
	return (size_t)((uintptr_t)memxrend(p, byte, num_bytes) - (uintptr_t)p);
}

#define rawmemlen dee_rawmemlen
LOCAL size_t dee_rawmemlen(void const *p, int byte) {
	return (size_t)((uintptr_t)rawmemchr(p, byte) - (uintptr_t)p);
}

#define rawmemrlen dee_rawmemrlen
LOCAL size_t dee_rawmemrlen(void const *p, int byte) {
	return (size_t)((uintptr_t)rawmemrchr(p, byte) - (uintptr_t)p);
}

#define memxchr dee_memxchr
LOCAL void *dee_memxchr(void const *__restrict p, int c, size_t n) {
	uint8_t *haystack = (uint8_t *)p;
	for (; n--; ++haystack) {
		if (*haystack != (uint8_t)c)
			return haystack;
	}
	return NULL;
}

#define memxrchr dee_memxrchr
LOCAL void *dee_memxrchr(void const *__restrict p, int c, size_t n) {
	uint8_t *haystack = (uint8_t *)p + n;
	while (n--) {
		if (*--haystack != (uint8_t)c)
			return haystack;
	}
	return NULL;
}

#define rawmemxchr dee_rawmemxchr
LOCAL void *dee_rawmemxchr(void const *__restrict p, int c) {
	uint8_t *haystack = (uint8_t *)p;
	for (;; ++haystack) {
		if (*haystack != (uint8_t)c)
			break;
	}
	return haystack;
}

#define rawmemxrchr dee_rawmemxrchr
LOCAL void *dee_rawmemxrchr(void const *__restrict p, int c) {
	uint8_t *haystack = (uint8_t *)p;
	for (;;) {
		if (*--haystack != (uint8_t)c)
			break;
	}
	return haystack;
}

#define rawmemxlen dee_rawmemxlen
LOCAL size_t dee_rawmemxlen(void const *p, int byte) {
	return (size_t)((uintptr_t)rawmemxchr(p, byte) - (uintptr_t)p);
}

#define rawmemxrlen dee_rawmemxrlen
LOCAL size_t dee_rawmemxrlen(void const *p, int byte) {
	return (size_t)((uintptr_t)rawmemxrchr(p, byte) - (uintptr_t)p);
}

#if defined(_MSC_VER) || defined(__USE_DOS) || defined(__CRT_DOS)
#define memcasecmp _memicmp
#else /* _MSC_VER || __USE_DOS || __CRT_DOS */
#define memcasecmp dee_memcasecmp
LOCAL int dee_memcasecmp(void const *a, void const *b, size_t n) {
	uint8_t *pa = (uint8_t *)a, *pb = (uint8_t *)b;
	uint8_t av, bv;
	av = bv = 0;
	while (n-- &&
	       (((av = *pa++) == (bv = *pb++)) ||
	        (av = tolower(av), bv = tolower(bv),
	         av == bv)))
		;
	return (int)av - (int)bv;
}
#endif /* !_MSC_VER && !__USE_DOS && !__CRT_DOS */

#define memcasemem dee_memcasemem
LOCAL void *dee_memcasemem(void const *__restrict haystack, size_t haystack_len,
                           void const *__restrict needle, size_t needle_len) {
	void const *candidate;
	uint8_t marker1, marker2;
	if unlikely(!needle_len || needle_len > haystack_len)
		return NULL;
	haystack_len -= needle_len;
	marker1 = (uint8_t)tolower(*(uint8_t *)needle);
	marker2 = (uint8_t)toupper(*(uint8_t *)needle);
	while ((candidate = memchr(haystack, marker1, haystack_len)) != NULL ||
	       (candidate = memchr(haystack, marker2, haystack_len)) != NULL) {
		if (memcasecmp(candidate, needle, needle_len) == 0)
			return (void *)candidate;
		haystack_len = ((uintptr_t)haystack + haystack_len) - (uintptr_t)candidate;
		haystack     = (void const *)((uintptr_t)candidate + 1);
	}
	return NULL;
}

#define memrmem dee_memrmem
LOCAL void *dee_memrmem(void const *__restrict haystack, size_t haystack_len,
                        void const *__restrict needle, size_t needle_len) {
	void const *candidate;
	uint8_t marker;
	if unlikely(!needle_len || needle_len > haystack_len)
		return NULL;
	haystack_len -= needle_len - 1, marker = *(uint8_t *)needle;
	while ((candidate = memrchr(haystack, marker, haystack_len)) != NULL) {
		if (memcmp(candidate, needle, needle_len) == 0)
			return (void *)candidate;
		if unlikely(candidate == haystack)
			break;
		haystack_len = (uintptr_t)candidate - (uintptr_t)haystack;
	}
	return NULL;
}


LOCAL void *dee_memlowerrchr(void const *__restrict p, uint8_t c, size_t n) {
	uint8_t *iter = (uint8_t *)p + n;
	while (iter-- != (uint8_t *)p) {
#ifdef CONFIG_NO_CTYPE
		if ((uint8_t)DeeUni_ToLower(*iter) == c)
			return iter;
#else /* CONFIG_NO_CTYPE */
		if ((uint8_t)tolower(*iter) == c)
			return iter;
#endif /* !CONFIG_NO_CTYPE */
	}
	return NULL;
}

#define memcasermem dee_memcasermem
LOCAL void *dee_memcasermem(void const *__restrict haystack, size_t haystack_len,
                            void const *__restrict needle, size_t needle_len) {
	void const *candidate;
	uint8_t marker;
	if unlikely(!needle_len || needle_len > haystack_len)
		return NULL;
	haystack_len -= needle_len;
	marker = (uint8_t)tolower(*(uint8_t *)needle);
	while ((candidate = dee_memlowerrchr(haystack, marker, haystack_len)) != NULL) {
		if (memcasecmp(candidate, needle, needle_len) == 0)
			return (void *)candidate;
		if unlikely(candidate == haystack)
			break;
		haystack_len = (((uintptr_t)candidate) - 1) - (uintptr_t)haystack;
	}
	return NULL;
}

#define memrev dee_memrev
LOCAL void *dee_memrev(void *__restrict buf, size_t n) {
	uint8_t *iter, *end;
	end = (iter = (uint8_t *)buf) + n;
	while (iter < end) {
		uint8_t temp = *iter;
		*iter++      = *--end;
		*end         = temp;
	}
	return buf;
}

#endif /* !__USE_KOS */






INTERN DREF DeeObject *DCALL
capi_memchr(size_t argc, DeeObject **__restrict argv) {
	DREF DeeObject *ob_dst;
	void *result;
	union pointer dst;
	int val;
	size_t num_bytes;
	if (DeeArg_Unpack(argc, argv, "odIu:memchr", &ob_dst, &val, &num_bytes) ||
	    DeeObject_AsPointer(ob_dst, &DeeCVoid_Type, &dst))
		goto err;
#ifdef CONFIG_HAVE_CTYPES_FAULTPROTECT
#ifdef CONFIG_HAVE_CTYPES_RECURSIVE_PROTECT
	CTYPES_FAULTPROTECT(result = memchr(dst.ptr, val, num_bytes),
	goto err);
#else /* CONFIG_HAVE_CTYPES_RECURSIVE_PROTECT */
	CTYPES_FAULTPROTECT({
		result = NULL;
		for (; num_bytes--; ++dst.p8) {
			if (*dst.p8 == (uint8_t)val) {
				result = haystack;
				break;
			}
		}
	},
	goto err);
#endif /* !CONFIG_HAVE_CTYPES_RECURSIVE_PROTECT */
#else /* CONFIG_HAVE_CTYPES_FAULTPROTECT */
	result = memchr(dst.ptr, val, num_bytes);
#endif /* !CONFIG_HAVE_CTYPES_FAULTPROTECT */
	return DeePointer_NewVoid(result);
err:
	return NULL;
}

INTERN DREF DeeObject *DCALL
capi_memrchr(size_t argc, DeeObject **__restrict argv) {
	DREF DeeObject *ob_dst;
	void *result;
	union pointer dst;
	int val;
	size_t num_bytes;
	if (DeeArg_Unpack(argc, argv, "odIu:memrchr", &ob_dst, &val, &num_bytes) ||
	    DeeObject_AsPointer(ob_dst, &DeeCVoid_Type, &dst))
		goto err;
#ifdef CONFIG_HAVE_CTYPES_FAULTPROTECT
#ifdef CONFIG_HAVE_CTYPES_RECURSIVE_PROTECT
	CTYPES_FAULTPROTECT(result = memrchr(dst.ptr, val, num_bytes),
	goto err);
#else /* CONFIG_HAVE_CTYPES_RECURSIVE_PROTECT */
	CTYPES_FAULTPROTECT({
		uint8_t *iter = dst.p8 + n;
		result = NULL;
		while (iter != dst.p8) {
			if (*--iter == (uint8_t)val) {
				result = iter;
				break;
			}
		}
	},
	goto err);
#endif /* !CONFIG_HAVE_CTYPES_RECURSIVE_PROTECT */
#else /* CONFIG_HAVE_CTYPES_FAULTPROTECT */
	result = memrchr(dst.ptr, val, num_bytes);
#endif /* !CONFIG_HAVE_CTYPES_FAULTPROTECT */
	return DeePointer_NewVoid(result);
err:
	return NULL;
}

INTERN DREF DeeObject *DCALL
capi_memend(size_t argc, DeeObject **__restrict argv) {
	DREF DeeObject *ob_dst;
	void *result;
	union pointer dst;
	int val;
	size_t num_bytes;
	if (DeeArg_Unpack(argc, argv, "odIu:memend", &ob_dst, &val, &num_bytes) ||
	    DeeObject_AsPointer(ob_dst, &DeeCVoid_Type, &dst))
		goto err;
#ifdef CONFIG_HAVE_CTYPES_FAULTPROTECT
#ifdef CONFIG_HAVE_CTYPES_RECURSIVE_PROTECT
	CTYPES_FAULTPROTECT(result = memend(dst.ptr, val, num_bytes),
	goto err);
#else /* CONFIG_HAVE_CTYPES_RECURSIVE_PROTECT */
	CTYPES_FAULTPROTECT({
		result = dst.p8;
		for (; num_bytes--; ++result) {
			if (*result == (uint8_t)byte)
				break;
		}
	},
	goto err);
#endif /* !CONFIG_HAVE_CTYPES_RECURSIVE_PROTECT */
#else /* CONFIG_HAVE_CTYPES_FAULTPROTECT */
	result = memend(dst.ptr, val, num_bytes);
#endif /* !CONFIG_HAVE_CTYPES_FAULTPROTECT */
	return DeePointer_NewVoid(result);
err:
	return NULL;
}

INTERN DREF DeeObject *DCALL
capi_memrend(size_t argc, DeeObject **__restrict argv) {
	DREF DeeObject *ob_dst;
	void *result;
	union pointer dst;
	int val;
	size_t num_bytes;
	if (DeeArg_Unpack(argc, argv, "odIu:memrend", &ob_dst, &val, &num_bytes) ||
	    DeeObject_AsPointer(ob_dst, &DeeCVoid_Type, &dst))
		goto err;
#ifdef CONFIG_HAVE_CTYPES_FAULTPROTECT
#ifdef CONFIG_HAVE_CTYPES_RECURSIVE_PROTECT
	CTYPES_FAULTPROTECT(result = memrend(dst.ptr, val, num_bytes),
	goto err);
#else /* CONFIG_HAVE_CTYPES_RECURSIVE_PROTECT */
	CTYPES_FAULTPROTECT({
		result = dst.p8;
		result += num_bytes;
		while (num_bytes--) {
			if (*--result == (uint8_t)val)
				break;
		}
	},
	goto err);
#endif /* !CONFIG_HAVE_CTYPES_RECURSIVE_PROTECT */
#else /* CONFIG_HAVE_CTYPES_FAULTPROTECT */
	result = memrend(dst.ptr, val, num_bytes);
#endif /* !CONFIG_HAVE_CTYPES_FAULTPROTECT */
	return DeePointer_NewVoid(result);
err:
	return NULL;
}

INTERN DREF DeeObject *DCALL
capi_memlen(size_t argc, DeeObject **__restrict argv) {
	DREF DeeObject *ob_dst;
	size_t result;
	union pointer dst;
	int val;
	size_t num_bytes;
	if (DeeArg_Unpack(argc, argv, "odIu:memlen", &ob_dst, &val, &num_bytes) ||
	    DeeObject_AsPointer(ob_dst, &DeeCVoid_Type, &dst))
		goto err;
#ifdef CONFIG_HAVE_CTYPES_FAULTPROTECT
#ifdef CONFIG_HAVE_CTYPES_RECURSIVE_PROTECT
	CTYPES_FAULTPROTECT(result = memlen(dst.ptr, val, num_bytes),
	goto err);
#else /* CONFIG_HAVE_CTYPES_RECURSIVE_PROTECT */
	CTYPES_FAULTPROTECT({
		uint8_t *haystack = dst.p8;
		for (; num_bytes--; ++haystack) {
			if (*haystack == (uint8_t)val)
				break;
		}
		result = (size_t)(haystack - dst.p8);
	},
	goto err);
#endif /* !CONFIG_HAVE_CTYPES_RECURSIVE_PROTECT */
#else /* CONFIG_HAVE_CTYPES_FAULTPROTECT */
	result = memlen(dst.ptr, val, num_bytes);
#endif /* !CONFIG_HAVE_CTYPES_FAULTPROTECT */
	return DeeInt_NewSize(result);
err:
	return NULL;
}

INTERN DREF DeeObject *DCALL
capi_memrlen(size_t argc, DeeObject **__restrict argv) {
	DREF DeeObject *ob_dst;
	size_t result;
	union pointer dst;
	int val;
	size_t num_bytes;
	if (DeeArg_Unpack(argc, argv, "odIu:memrlen", &ob_dst, &val, &num_bytes) ||
	    DeeObject_AsPointer(ob_dst, &DeeCVoid_Type, &dst))
		goto err;
#ifdef CONFIG_HAVE_CTYPES_FAULTPROTECT
#ifdef CONFIG_HAVE_CTYPES_RECURSIVE_PROTECT
	CTYPES_FAULTPROTECT(result = memrlen(dst.ptr, val, num_bytes),
	goto err);
#else /* CONFIG_HAVE_CTYPES_RECURSIVE_PROTECT */
	CTYPES_FAULTPROTECT({
		uint8_t *haystack = dst.p8;
		haystack += num_bytes;
		while (num_bytes--) {
			if (*--haystack == (uint8_t)val)
				break;
		}
		result = (size_t)(haystack - dst.p8);
	},
	goto err);
#endif /* !CONFIG_HAVE_CTYPES_RECURSIVE_PROTECT */
#else /* CONFIG_HAVE_CTYPES_FAULTPROTECT */
	result = memrlen(dst.ptr, val, num_bytes);
#endif /* !CONFIG_HAVE_CTYPES_FAULTPROTECT */
	return DeeInt_NewSize(result);
err:
	return NULL;
}

INTDEF DREF DeeObject *DCALL
capi_rawmemchr(size_t argc, DeeObject **__restrict argv) {
	DREF DeeObject *ob_dst;
	void *result;
	union pointer dst;
	int val;
	if (DeeArg_Unpack(argc, argv, "od:rawmemchr", &ob_dst, &val) ||
	    DeeObject_AsPointer(ob_dst, &DeeCVoid_Type, &dst))
		goto err;
#ifdef CONFIG_HAVE_CTYPES_FAULTPROTECT
#ifdef CONFIG_HAVE_CTYPES_RECURSIVE_PROTECT
	CTYPES_FAULTPROTECT(result = rawmemchr(dst.ptr, val),
	goto err);
#else /* CONFIG_HAVE_CTYPES_RECURSIVE_PROTECT */
	CTYPES_FAULTPROTECT({
		result = dst.p8;
		for (;; ++result) {
			if (*result == (uint8_t)byte)
				break;
		}
	},
	goto err);
#endif /* !CONFIG_HAVE_CTYPES_RECURSIVE_PROTECT */
#else /* CONFIG_HAVE_CTYPES_FAULTPROTECT */
	result = rawmemchr(dst.ptr, val);
#endif /* !CONFIG_HAVE_CTYPES_FAULTPROTECT */
	return DeePointer_NewVoid(result);
err:
	return NULL;
}

INTDEF DREF DeeObject *DCALL
capi_rawmemrchr(size_t argc, DeeObject **__restrict argv) {
	DREF DeeObject *ob_dst;
	void *result;
	union pointer dst;
	int val;
	if (DeeArg_Unpack(argc, argv, "od:rawmemrchr", &ob_dst, &val) ||
	    DeeObject_AsPointer(ob_dst, &DeeCVoid_Type, &dst))
		goto err;
#ifdef CONFIG_HAVE_CTYPES_FAULTPROTECT
#ifdef CONFIG_HAVE_CTYPES_RECURSIVE_PROTECT
	CTYPES_FAULTPROTECT(result = rawmemrchr(dst.ptr, val),
	goto err);
#else /* CONFIG_HAVE_CTYPES_RECURSIVE_PROTECT */
	CTYPES_FAULTPROTECT({
		result = dst.p8;
		for (;;) {
			if (*--result == (uint8_t)byte)
				break;
		}
	},
	goto err);
#endif /* !CONFIG_HAVE_CTYPES_RECURSIVE_PROTECT */
#else /* CONFIG_HAVE_CTYPES_FAULTPROTECT */
	result = rawmemrchr(dst.ptr, val);
#endif /* !CONFIG_HAVE_CTYPES_FAULTPROTECT */
	return DeePointer_NewVoid(result);
err:
	return NULL;
}

INTDEF DREF DeeObject *DCALL
capi_rawmemlen(size_t argc, DeeObject **__restrict argv) {
	DREF DeeObject *ob_dst;
	size_t result;
	union pointer dst;
	int val;
	if (DeeArg_Unpack(argc, argv, "od:rawmemlen", &ob_dst, &val) ||
	    DeeObject_AsPointer(ob_dst, &DeeCVoid_Type, &dst))
		goto err;
#ifdef CONFIG_HAVE_CTYPES_FAULTPROTECT
#ifdef CONFIG_HAVE_CTYPES_RECURSIVE_PROTECT
	CTYPES_FAULTPROTECT(result = rawmemlen(dst.ptr, val),
	goto err);
#else /* CONFIG_HAVE_CTYPES_RECURSIVE_PROTECT */
	CTYPES_FAULTPROTECT({
		uint8_t *iter = dst.p8;
		for (;; ++iter) {
			if (*iter == (uint8_t)byte)
				break;
		}
		result = (size_t)(iter - dst.p8);
	},
	goto err);
#endif /* !CONFIG_HAVE_CTYPES_RECURSIVE_PROTECT */
#else /* CONFIG_HAVE_CTYPES_FAULTPROTECT */
	result = rawmemlen(dst.ptr, val);
#endif /* !CONFIG_HAVE_CTYPES_FAULTPROTECT */
	return DeeInt_NewSize(result);
err:
	return NULL;
}

INTDEF DREF DeeObject *DCALL
capi_rawmemrlen(size_t argc, DeeObject **__restrict argv) {
	DREF DeeObject *ob_dst;
	size_t result;
	union pointer dst;
	int val;
	if (DeeArg_Unpack(argc, argv, "od:rawmemrlen", &ob_dst, &val) ||
	    DeeObject_AsPointer(ob_dst, &DeeCVoid_Type, &dst))
		goto err;
#ifdef CONFIG_HAVE_CTYPES_FAULTPROTECT
#ifdef CONFIG_HAVE_CTYPES_RECURSIVE_PROTECT
	CTYPES_FAULTPROTECT(result = rawmemrlen(dst.ptr, val),
	goto err);
#else /* CONFIG_HAVE_CTYPES_RECURSIVE_PROTECT */
	CTYPES_FAULTPROTECT({
		uint8_t *iter = dst.p8;
		for (;;) {
			if (*--iter == (uint8_t)byte)
				break;
		}
		result = (size_t)(iter - dst.p8);
	},
	goto err);
#endif /* !CONFIG_HAVE_CTYPES_RECURSIVE_PROTECT */
#else /* CONFIG_HAVE_CTYPES_FAULTPROTECT */
	result = rawmemrlen(dst.ptr, val);
#endif /* !CONFIG_HAVE_CTYPES_FAULTPROTECT */
	return DeeInt_NewSize(result);
err:
	return NULL;
}


INTERN DREF DeeObject *DCALL
capi_memxchr(size_t argc, DeeObject **__restrict argv) {
	DREF DeeObject *ob_dst;
	void *result;
	union pointer dst;
	int val;
	size_t num_bytes;
	if (DeeArg_Unpack(argc, argv, "odIu:memxchr", &ob_dst, &val, &num_bytes) ||
	    DeeObject_AsPointer(ob_dst, &DeeCVoid_Type, &dst))
		goto err;
#ifdef CONFIG_HAVE_CTYPES_FAULTPROTECT
#ifdef CONFIG_HAVE_CTYPES_RECURSIVE_PROTECT
	CTYPES_FAULTPROTECT(result = memxchr(dst.ptr, val, num_bytes),
	goto err);
#else /* CONFIG_HAVE_CTYPES_RECURSIVE_PROTECT */
	CTYPES_FAULTPROTECT({
		result = NULL;
		for (; num_bytes--; ++dst.p8) {
			if (*dst.p8 != (uint8_t)val) {
				result = haystack;
				break;
			}
		}
	},
	goto err);
#endif /* !CONFIG_HAVE_CTYPES_RECURSIVE_PROTECT */
#else /* CONFIG_HAVE_CTYPES_FAULTPROTECT */
	result = memxchr(dst.ptr, val, num_bytes);
#endif /* !CONFIG_HAVE_CTYPES_FAULTPROTECT */
	return DeePointer_NewVoid(result);
err:
	return NULL;
}

INTERN DREF DeeObject *DCALL
capi_memxrchr(size_t argc, DeeObject **__restrict argv) {
	DREF DeeObject *ob_dst;
	void *result;
	union pointer dst;
	int val;
	size_t num_bytes;
	if (DeeArg_Unpack(argc, argv, "odIu:memxrchr", &ob_dst, &val, &num_bytes) ||
	    DeeObject_AsPointer(ob_dst, &DeeCVoid_Type, &dst))
		goto err;
#ifdef CONFIG_HAVE_CTYPES_FAULTPROTECT
#ifdef CONFIG_HAVE_CTYPES_RECURSIVE_PROTECT
	CTYPES_FAULTPROTECT(result = memxrchr(dst.ptr, val, num_bytes),
	goto err);
#else /* CONFIG_HAVE_CTYPES_RECURSIVE_PROTECT */
	CTYPES_FAULTPROTECT({
		uint8_t *iter = dst.p8 + n;
		result = NULL;
		while (iter != dst.p8) {
			if (*--iter != (uint8_t)val) {
				result = iter;
				break;
			}
		}
	},
	goto err);
#endif /* !CONFIG_HAVE_CTYPES_RECURSIVE_PROTECT */
#else /* CONFIG_HAVE_CTYPES_FAULTPROTECT */
	result = memxrchr(dst.ptr, val, num_bytes);
#endif /* !CONFIG_HAVE_CTYPES_FAULTPROTECT */
	return DeePointer_NewVoid(result);
err:
	return NULL;
}

INTERN DREF DeeObject *DCALL
capi_memxend(size_t argc, DeeObject **__restrict argv) {
	DREF DeeObject *ob_dst;
	void *result;
	union pointer dst;
	int val;
	size_t num_bytes;
	if (DeeArg_Unpack(argc, argv, "odIu:memxend", &ob_dst, &val, &num_bytes) ||
	    DeeObject_AsPointer(ob_dst, &DeeCVoid_Type, &dst))
		goto err;
#ifdef CONFIG_HAVE_CTYPES_FAULTPROTECT
#ifdef CONFIG_HAVE_CTYPES_RECURSIVE_PROTECT
	CTYPES_FAULTPROTECT(result = memxend(dst.ptr, val, num_bytes),
	goto err);
#else /* CONFIG_HAVE_CTYPES_RECURSIVE_PROTECT */
	CTYPES_FAULTPROTECT({
		result = dst.p8;
		for (; num_bytes--; ++result) {
			if (*result != (uint8_t)byte)
				break;
		}
	},
	goto err);
#endif /* !CONFIG_HAVE_CTYPES_RECURSIVE_PROTECT */
#else /* CONFIG_HAVE_CTYPES_FAULTPROTECT */
	result = memxend(dst.ptr, val, num_bytes);
#endif /* !CONFIG_HAVE_CTYPES_FAULTPROTECT */
	return DeePointer_NewVoid(result);
err:
	return NULL;
}

INTERN DREF DeeObject *DCALL
capi_memxrend(size_t argc, DeeObject **__restrict argv) {
	DREF DeeObject *ob_dst;
	void *result;
	union pointer dst;
	int val;
	size_t num_bytes;
	if (DeeArg_Unpack(argc, argv, "odIu:memxrend", &ob_dst, &val, &num_bytes) ||
	    DeeObject_AsPointer(ob_dst, &DeeCVoid_Type, &dst))
		goto err;
#ifdef CONFIG_HAVE_CTYPES_FAULTPROTECT
#ifdef CONFIG_HAVE_CTYPES_RECURSIVE_PROTECT
	CTYPES_FAULTPROTECT(result = memxrend(dst.ptr, val, num_bytes),
	goto err);
#else /* CONFIG_HAVE_CTYPES_RECURSIVE_PROTECT */
	CTYPES_FAULTPROTECT({
		result = dst.p8;
		result += num_bytes;
		while (num_bytes--) {
			if (*--result != (uint8_t)val)
				break;
		}
	},
	goto err);
#endif /* !CONFIG_HAVE_CTYPES_RECURSIVE_PROTECT */
#else /* CONFIG_HAVE_CTYPES_FAULTPROTECT */
	result = memxrend(dst.ptr, val, num_bytes);
#endif /* !CONFIG_HAVE_CTYPES_FAULTPROTECT */
	return DeePointer_NewVoid(result);
err:
	return NULL;
}

INTERN DREF DeeObject *DCALL
capi_memxlen(size_t argc, DeeObject **__restrict argv) {
	DREF DeeObject *ob_dst;
	size_t result;
	union pointer dst;
	int val;
	size_t num_bytes;
	if (DeeArg_Unpack(argc, argv, "odIu:memxlen", &ob_dst, &val, &num_bytes) ||
	    DeeObject_AsPointer(ob_dst, &DeeCVoid_Type, &dst))
		goto err;
#ifdef CONFIG_HAVE_CTYPES_FAULTPROTECT
#ifdef CONFIG_HAVE_CTYPES_RECURSIVE_PROTECT
	CTYPES_FAULTPROTECT(result = memxlen(dst.ptr, val, num_bytes),
	goto err);
#else /* CONFIG_HAVE_CTYPES_RECURSIVE_PROTECT */
	CTYPES_FAULTPROTECT({
		uint8_t *haystack = dst.p8;
		for (; num_bytes--; ++haystack) {
			if (*haystack != (uint8_t)val)
				break;
		}
		result = (size_t)(haystack - dst.p8);
	},
	goto err);
#endif /* !CONFIG_HAVE_CTYPES_RECURSIVE_PROTECT */
#else /* CONFIG_HAVE_CTYPES_FAULTPROTECT */
	result = memxlen(dst.ptr, val, num_bytes);
#endif /* !CONFIG_HAVE_CTYPES_FAULTPROTECT */
	return DeeInt_NewSize(result);
err:
	return NULL;
}

INTERN DREF DeeObject *DCALL
capi_memxrlen(size_t argc, DeeObject **__restrict argv) {
	DREF DeeObject *ob_dst;
	size_t result;
	union pointer dst;
	int val;
	size_t num_bytes;
	if (DeeArg_Unpack(argc, argv, "odIu:memxrlen", &ob_dst, &val, &num_bytes) ||
	    DeeObject_AsPointer(ob_dst, &DeeCVoid_Type, &dst))
		goto err;
#ifdef CONFIG_HAVE_CTYPES_FAULTPROTECT
#ifdef CONFIG_HAVE_CTYPES_RECURSIVE_PROTECT
	CTYPES_FAULTPROTECT(result = memxrlen(dst.ptr, val, num_bytes),
	goto err);
#else /* CONFIG_HAVE_CTYPES_RECURSIVE_PROTECT */
	CTYPES_FAULTPROTECT({
		uint8_t *haystack = dst.p8;
		haystack += num_bytes;
		while (num_bytes--) {
			if (*--haystack != (uint8_t)val)
				break;
		}
		result = (size_t)(haystack - dst.p8);
	},
	goto err);
#endif /* !CONFIG_HAVE_CTYPES_RECURSIVE_PROTECT */
#else /* CONFIG_HAVE_CTYPES_FAULTPROTECT */
	result = memxrlen(dst.ptr, val, num_bytes);
#endif /* !CONFIG_HAVE_CTYPES_FAULTPROTECT */
	return DeeInt_NewSize(result);
err:
	return NULL;
}

INTDEF DREF DeeObject *DCALL
capi_rawmemxchr(size_t argc, DeeObject **__restrict argv) {
	DREF DeeObject *ob_dst;
	void *result;
	union pointer dst;
	int val;
	if (DeeArg_Unpack(argc, argv, "od:rawmemxchr", &ob_dst, &val) ||
	    DeeObject_AsPointer(ob_dst, &DeeCVoid_Type, &dst))
		goto err;
#ifdef CONFIG_HAVE_CTYPES_FAULTPROTECT
#ifdef CONFIG_HAVE_CTYPES_RECURSIVE_PROTECT
	CTYPES_FAULTPROTECT(result = rawmemxchr(dst.ptr, val),
	goto err);
#else /* CONFIG_HAVE_CTYPES_RECURSIVE_PROTECT */
	CTYPES_FAULTPROTECT({
		result = dst.p8;
		for (;; ++result) {
			if (*result != (uint8_t)byte)
				break;
		}
	},
	goto err);
#endif /* !CONFIG_HAVE_CTYPES_RECURSIVE_PROTECT */
#else /* CONFIG_HAVE_CTYPES_FAULTPROTECT */
	result = rawmemxchr(dst.ptr, val);
#endif /* !CONFIG_HAVE_CTYPES_FAULTPROTECT */
	return DeePointer_NewVoid(result);
err:
	return NULL;
}

INTDEF DREF DeeObject *DCALL
capi_rawmemxrchr(size_t argc, DeeObject **__restrict argv) {
	DREF DeeObject *ob_dst;
	void *result;
	union pointer dst;
	int val;
	if (DeeArg_Unpack(argc, argv, "od:rawmemxrchr", &ob_dst, &val) ||
	    DeeObject_AsPointer(ob_dst, &DeeCVoid_Type, &dst))
		goto err;
	CTYPES_PROTECTED(
	result = rawmemxrchr(dst.ptr, val), {
		result = dst.p8;
		for (;;) {
			if (*--result != (uint8_t)byte)
				break;
		}
	},
	goto err);
	return DeePointer_NewVoid(result);
err:
	return NULL;
}

INTDEF DREF DeeObject *DCALL
capi_rawmemxlen(size_t argc, DeeObject **__restrict argv) {
	DREF DeeObject *ob_dst;
	size_t result;
	union pointer dst;
	int val;
	if (DeeArg_Unpack(argc, argv, "od:rawmemxlen", &ob_dst, &val) ||
	    DeeObject_AsPointer(ob_dst, &DeeCVoid_Type, &dst))
		goto err;
	CTYPES_PROTECTED(
	result = rawmemxlen(dst.ptr, val), {
		uint8_t *iter = dst.p8;
		for (;; ++iter) {
			if (*iter != (uint8_t)byte)
				break;
		}
		result = (size_t)(iter - dst.p8);
	},
	goto err);
	return DeeInt_NewSize(result);
err:
	return NULL;
}

INTDEF DREF DeeObject *DCALL
capi_rawmemxrlen(size_t argc, DeeObject **__restrict argv) {
	DREF DeeObject *ob_dst;
	size_t result;
	union pointer dst;
	int val;
	if (DeeArg_Unpack(argc, argv, "od:rawmemxrlen", &ob_dst, &val) ||
	    DeeObject_AsPointer(ob_dst, &DeeCVoid_Type, &dst))
		goto err;
	CTYPES_PROTECTED(
	result = rawmemxrlen(dst.ptr, val), {
		uint8_t *iter = dst.p8;
		for (;;) {
			if (*--iter != (uint8_t)byte)
				break;
		}
		result = (size_t)(iter - dst.p8);
	},
	goto err);
	return DeeInt_NewSize(result);
err:
	return NULL;
}



INTDEF DREF DeeObject *DCALL
capi_memcmp(size_t argc, DeeObject **__restrict argv) {
	DREF DeeObject *ob_a, *ob_b;
	int result;
	union pointer a, b;
	size_t num_bytes;
	if (DeeArg_Unpack(argc, argv, "ooIu:memcmp", &ob_a, &ob_b, &num_bytes) ||
	    DeeObject_AsPointer(ob_a, &DeeCVoid_Type, &a) ||
	    DeeObject_AsPointer(ob_b, &DeeCVoid_Type, &b))
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

INTDEF DREF DeeObject *DCALL
capi_memcasecmp(size_t argc, DeeObject **__restrict argv) {
	DREF DeeObject *ob_a, *ob_b;
	int result;
	union pointer a, b;
	size_t num_bytes;
	if (DeeArg_Unpack(argc, argv, "ooIu:memcasecmp", &ob_a, &ob_b, &num_bytes) ||
	    DeeObject_AsPointer(ob_a, &DeeCVoid_Type, &a) ||
	    DeeObject_AsPointer(ob_b, &DeeCVoid_Type, &b))
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

INTDEF DREF DeeObject *DCALL
capi_memmem(size_t argc, DeeObject **__restrict argv) {
	DREF DeeObject *ob_a, *ob_b;
	void *result;
	union pointer a, b;
	size_t haystack_len, needle_len;
	if (DeeArg_Unpack(argc, argv, "oIuoIu:memmem", &ob_a, &haystack_len, &ob_b, &needle_len) ||
	    DeeObject_AsPointer(ob_a, &DeeCVoid_Type, &a) ||
	    DeeObject_AsPointer(ob_b, &DeeCVoid_Type, &b))
		goto err;
	CTYPES_PROTECTED(
	result = memmem(a.ptr, haystack_len, b.ptr, needle_len), {
		void const *candidate;
		uint8_t marker;
		result = NULL;
		if unlikely(!needle_len || needle_len > haystack_len);
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
					result = (void *)candidate;
					break;
				}
				haystack_len = ((uintptr_t)a.p8 + haystack_len) - (uintptr_t)candidate;
				a.ptr        = (void *)((uintptr_t)candidate + 1);
			}
		}
	},
	goto err);
	return DeePointer_NewVoid(result);
err:
	return NULL;
}


INTDEF DREF DeeObject *DCALL
capi_memcasemem(size_t argc, DeeObject **__restrict argv) {
	DREF DeeObject *ob_a, *ob_b;
	void *result;
	union pointer a, b;
	size_t haystack_len, needle_len;
	if (DeeArg_Unpack(argc, argv, "oIuoIu:memcasemem", &ob_a, &haystack_len, &ob_b, &needle_len) ||
	    DeeObject_AsPointer(ob_a, &DeeCVoid_Type, &a) ||
	    DeeObject_AsPointer(ob_b, &DeeCVoid_Type, &b))
		goto err;
	CTYPES_PROTECTED(
	result = memcasemem(a.ptr, haystack_len, b.ptr, needle_len), {
		void const *candidate;
		uint8_t marker1;
		uint8_t marker2;
		result = NULL;
		if unlikely(!needle_len || needle_len > haystack_len);
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
					result = (void *)candidate;
					break;
				}
				haystack_len = ((uintptr_t)a.p8 + haystack_len) - (uintptr_t)candidate;
				a.ptr        = (void *)((uintptr_t)candidate + 1);
			}
		}
	},
	goto err);
	return DeePointer_NewVoid(result);
err:
	return NULL;
}



INTDEF DREF DeeObject *DCALL
capi_memrmem(size_t argc, DeeObject **__restrict argv) {
	DREF DeeObject *ob_a, *ob_b;
	void *result;
	union pointer a, b;
	size_t haystack_len, needle_len;
	if (DeeArg_Unpack(argc, argv, "oIuoIu:memrmem", &ob_a, &haystack_len, &ob_b, &needle_len) ||
	    DeeObject_AsPointer(ob_a, &DeeCVoid_Type, &a) ||
	    DeeObject_AsPointer(ob_b, &DeeCVoid_Type, &b))
		goto err;
	CTYPES_PROTECTED(
	result = memrmem(a.ptr, haystack_len, b.ptr, needle_len), {
		void const *candidate;
		uint8_t marker;
		result = NULL;
		if unlikely(!needle_len || needle_len > haystack_len);
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
					result = (void *)candidate;
					break;
				}
				if unlikely(candidate == b.ptr)
					break;
				haystack_len = (uintptr_t)candidate - (uintptr_t)a.ptr;
			}
		}
	},
	goto err);
	return DeePointer_NewVoid(result);
err:
	return NULL;
}



INTDEF DREF DeeObject *DCALL
capi_memcasermem(size_t argc, DeeObject **__restrict argv) {
	DREF DeeObject *ob_a, *ob_b;
	void *result;
	union pointer a, b;
	size_t haystack_len, needle_len;
	if (DeeArg_Unpack(argc, argv, "oIuoIu:memcasermem", &ob_a, &haystack_len, &ob_b, &needle_len) ||
	    DeeObject_AsPointer(ob_a, &DeeCVoid_Type, &a) ||
	    DeeObject_AsPointer(ob_b, &DeeCVoid_Type, &b))
		goto err;
	CTYPES_PROTECTED(
	result = memcasermem(a.ptr, haystack_len, b.ptr, needle_len), {
		void const *candidate;
		uint8_t marker1;
		uint8_t marker2;
		result = NULL;
		if unlikely(!needle_len || needle_len > haystack_len);
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
					result = (void *)candidate;
					break;
				}
				if unlikely(candidate == b.ptr)
					break;
				haystack_len = (uintptr_t)candidate - (uintptr_t)a.ptr;
			}
		}
	},
	goto err);
	return DeePointer_NewVoid(result);
err:
	return NULL;
}

INTERN DREF DeeObject *DCALL
capi_memrev(size_t argc, DeeObject **__restrict argv) {
	DREF DeeObject *ob_dst;
	union pointer dst;
	size_t num_bytes;
	if (DeeArg_Unpack(argc, argv, "oIu:memrev", &ob_dst, &num_bytes) ||
	    DeeObject_AsPointer(ob_dst, &DeeCVoid_Type, &dst))
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



#ifndef __USE_KOS
#define strend(s)       ((s) + strlen(s))
#define strnend(s, len) ((s) + strnlen(s, len))
#endif /* !__USE_KOS */


INTERN DREF DeeObject *DCALL
capi_strlen(size_t argc, DeeObject **__restrict argv) {
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

INTERN DREF DeeObject *DCALL
capi_strend(size_t argc, DeeObject **__restrict argv) {
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

INTERN DREF DeeObject *DCALL
capi_strnlen(size_t argc, DeeObject **__restrict argv) {
	DeeObject *ob_str;
	size_t maxlen;
	union pointer str;
	size_t result;
	if (DeeArg_Unpack(argc, argv, "oIu:strnlen", &ob_str, &maxlen))
		goto err;
	if (DeeObject_AsPointer(ob_str, &DeeCChar_Type, &str))
		goto err;
	CTYPES_PROTECTED_STRNLEN(result, str.pchar, maxlen, goto err);
	return DeeInt_NewSize(result);
err:
	return NULL;
}

INTERN DREF DeeObject *DCALL
capi_strnend(size_t argc, DeeObject **__restrict argv) {
	DeeObject *ob_str;
	union pointer str;
	size_t maxlen;
	if (DeeArg_Unpack(argc, argv, "o:strnend", &ob_str, &maxlen))
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



INTERN DREF DeeObject *DCALL
capi_strchr(size_t argc, DeeObject **__restrict argv) {
	(void)argc;
	(void)argv;
	/* TODO */
	DERROR_NOTIMPLEMENTED();
	return NULL;
}


INTERN DREF DeeObject *DCALL
capi_strrchr(size_t argc, DeeObject **__restrict argv) {
	(void)argc;
	(void)argv;
	/* TODO */
	DERROR_NOTIMPLEMENTED();
	return NULL;
}

INTERN DREF DeeObject *DCALL
capi_strnchr(size_t argc, DeeObject **__restrict argv) {
	(void)argc;
	(void)argv;
	/* TODO */
	DERROR_NOTIMPLEMENTED();
	return NULL;
}

INTERN DREF DeeObject *DCALL
capi_strnrchr(size_t argc, DeeObject **__restrict argv) {
	(void)argc;
	(void)argv;
	/* TODO */
	DERROR_NOTIMPLEMENTED();
	return NULL;
}

INTERN DREF DeeObject *DCALL
capi_stroff(size_t argc, DeeObject **__restrict argv) {
	(void)argc;
	(void)argv;
	/* TODO */
	DERROR_NOTIMPLEMENTED();
	return NULL;
}

INTERN DREF DeeObject *DCALL
capi_strroff(size_t argc, DeeObject **__restrict argv) {
	(void)argc;
	(void)argv;
	/* TODO */
	DERROR_NOTIMPLEMENTED();
	return NULL;
}

INTERN DREF DeeObject *DCALL
capi_strnoff(size_t argc, DeeObject **__restrict argv) {
	(void)argc;
	(void)argv;
	/* TODO */
	DERROR_NOTIMPLEMENTED();
	return NULL;
}

INTERN DREF DeeObject *DCALL
capi_strnroff(size_t argc, DeeObject **__restrict argv) {
	(void)argc;
	(void)argv;
	/* TODO */
	DERROR_NOTIMPLEMENTED();
	return NULL;
}

INTERN DREF DeeObject *DCALL
capi_strchrnul(size_t argc, DeeObject **__restrict argv) {
	(void)argc;
	(void)argv;
	/* TODO */
	DERROR_NOTIMPLEMENTED();
	return NULL;
}

INTERN DREF DeeObject *DCALL
capi_strrchrnul(size_t argc, DeeObject **__restrict argv) {
	(void)argc;
	(void)argv;
	/* TODO */
	DERROR_NOTIMPLEMENTED();
	return NULL;
}

INTERN DREF DeeObject *DCALL
capi_strnchrnul(size_t argc, DeeObject **__restrict argv) {
	(void)argc;
	(void)argv;
	/* TODO */
	DERROR_NOTIMPLEMENTED();
	return NULL;
}

INTERN DREF DeeObject *DCALL
capi_strnrchrnul(size_t argc, DeeObject **__restrict argv) {
	(void)argc;
	(void)argv;
	/* TODO */
	DERROR_NOTIMPLEMENTED();
	return NULL;
}

INTERN DREF DeeObject *DCALL
capi_strcmp(size_t argc, DeeObject **__restrict argv) {
	(void)argc;
	(void)argv;
	/* TODO */
	DERROR_NOTIMPLEMENTED();
	return NULL;
}

INTERN DREF DeeObject *DCALL
capi_strncmp(size_t argc, DeeObject **__restrict argv) {
	(void)argc;
	(void)argv;
	/* TODO */
	DERROR_NOTIMPLEMENTED();
	return NULL;
}

INTERN DREF DeeObject *DCALL
capi_strcasecmp(size_t argc, DeeObject **__restrict argv) {
	(void)argc;
	(void)argv;
	/* TODO */
	DERROR_NOTIMPLEMENTED();
	return NULL;
}

INTERN DREF DeeObject *DCALL
capi_strncasecmp(size_t argc, DeeObject **__restrict argv) {
	(void)argc;
	(void)argv;
	/* TODO */
	DERROR_NOTIMPLEMENTED();
	return NULL;
}

INTERN DREF DeeObject *DCALL
capi_strcpy(size_t argc, DeeObject **__restrict argv) {
	(void)argc;
	(void)argv;
	/* TODO */
	DERROR_NOTIMPLEMENTED();
	return NULL;
}

INTERN DREF DeeObject *DCALL
capi_strcat(size_t argc, DeeObject **__restrict argv) {
	(void)argc;
	(void)argv;
	/* TODO */
	DERROR_NOTIMPLEMENTED();
	return NULL;
}

INTERN DREF DeeObject *DCALL
capi_strncpy(size_t argc, DeeObject **__restrict argv) {
	(void)argc;
	(void)argv;
	/* TODO */
	DERROR_NOTIMPLEMENTED();
	return NULL;
}

INTERN DREF DeeObject *DCALL
capi_strncat(size_t argc, DeeObject **__restrict argv) {
	(void)argc;
	(void)argv;
	/* TODO */
	DERROR_NOTIMPLEMENTED();
	return NULL;
}

INTERN DREF DeeObject *DCALL
capi_stpcpy(size_t argc, DeeObject **__restrict argv) {
	(void)argc;
	(void)argv;
	/* TODO */
	DERROR_NOTIMPLEMENTED();
	return NULL;
}

INTERN DREF DeeObject *DCALL
capi_stpncpy(size_t argc, DeeObject **__restrict argv) {
	(void)argc;
	(void)argv;
	/* TODO */
	DERROR_NOTIMPLEMENTED();
	return NULL;
}

INTERN DREF DeeObject *DCALL
capi_strstr(size_t argc, DeeObject **__restrict argv) {
	(void)argc;
	(void)argv;
	/* TODO */
	DERROR_NOTIMPLEMENTED();
	return NULL;
}

INTERN DREF DeeObject *DCALL
capi_strcasestr(size_t argc, DeeObject **__restrict argv) {
	(void)argc;
	(void)argv;
	/* TODO */
	DERROR_NOTIMPLEMENTED();
	return NULL;
}

INTERN DREF DeeObject *DCALL
capi_strverscmp(size_t argc, DeeObject **__restrict argv) {
	(void)argc;
	(void)argv;
	/* TODO */
	DERROR_NOTIMPLEMENTED();
	return NULL;
}

INTERN DREF DeeObject *DCALL
capi_strtok(size_t argc, DeeObject **__restrict argv) {
	(void)argc;
	(void)argv;
	/* TODO */
	DERROR_NOTIMPLEMENTED();
	return NULL;
}

INTERN DREF DeeObject *DCALL
capi_index(size_t argc, DeeObject **__restrict argv) {
	(void)argc;
	(void)argv;
	/* TODO */
	DERROR_NOTIMPLEMENTED();
	return NULL;
}

INTERN DREF DeeObject *DCALL
capi_rindex(size_t argc, DeeObject **__restrict argv) {
	(void)argc;
	(void)argv;
	/* TODO */
	DERROR_NOTIMPLEMENTED();
	return NULL;
}

INTERN DREF DeeObject *DCALL
capi_strspn(size_t argc, DeeObject **__restrict argv) {
	(void)argc;
	(void)argv;
	/* TODO */
	DERROR_NOTIMPLEMENTED();
	return NULL;
}

INTERN DREF DeeObject *DCALL
capi_strcspn(size_t argc, DeeObject **__restrict argv) {
	(void)argc;
	(void)argv;
	/* TODO */
	DERROR_NOTIMPLEMENTED();
	return NULL;
}

INTERN DREF DeeObject *DCALL
capi_strpbrk(size_t argc, DeeObject **__restrict argv) {
	(void)argc;
	(void)argv;
	/* TODO */
	DERROR_NOTIMPLEMENTED();
	return NULL;
}

INTERN DREF DeeObject *DCALL
capi_strcoll(size_t argc, DeeObject **__restrict argv) {
	(void)argc;
	(void)argv;
	/* TODO */
	DERROR_NOTIMPLEMENTED();
	return NULL;
}

INTERN DREF DeeObject *DCALL
capi_strncoll(size_t argc, DeeObject **__restrict argv) {
	(void)argc;
	(void)argv;
	/* TODO */
	DERROR_NOTIMPLEMENTED();
	return NULL;
}

INTERN DREF DeeObject *DCALL
capi_strcasecoll(size_t argc, DeeObject **__restrict argv) {
	(void)argc;
	(void)argv;
	/* TODO */
	DERROR_NOTIMPLEMENTED();
	return NULL;
}

INTERN DREF DeeObject *DCALL
capi_strncasecoll(size_t argc, DeeObject **__restrict argv) {
	(void)argc;
	(void)argv;
	/* TODO */
	DERROR_NOTIMPLEMENTED();
	return NULL;
}

INTERN DREF DeeObject *DCALL
capi_strxfrm(size_t argc, DeeObject **__restrict argv) {
	(void)argc;
	(void)argv;
	/* TODO */
	DERROR_NOTIMPLEMENTED();
	return NULL;
}

INTERN DREF DeeObject *DCALL
capi_strrev(size_t argc, DeeObject **__restrict argv) {
	(void)argc;
	(void)argv;
	/* TODO */
	DERROR_NOTIMPLEMENTED();
	return NULL;
}

INTERN DREF DeeObject *DCALL
capi_strnrev(size_t argc, DeeObject **__restrict argv) {
	(void)argc;
	(void)argv;
	/* TODO */
	DERROR_NOTIMPLEMENTED();
	return NULL;
}

INTERN DREF DeeObject *DCALL
capi_strlwr(size_t argc, DeeObject **__restrict argv) {
	(void)argc;
	(void)argv;
	/* TODO */
	DERROR_NOTIMPLEMENTED();
	return NULL;
}

INTERN DREF DeeObject *DCALL
capi_strupr(size_t argc, DeeObject **__restrict argv) {
	(void)argc;
	(void)argv;
	/* TODO */
	DERROR_NOTIMPLEMENTED();
	return NULL;
}

INTERN DREF DeeObject *DCALL
capi_strset(size_t argc, DeeObject **__restrict argv) {
	(void)argc;
	(void)argv;
	/* TODO */
	DERROR_NOTIMPLEMENTED();
	return NULL;
}

INTERN DREF DeeObject *DCALL
capi_strnset(size_t argc, DeeObject **__restrict argv) {
	(void)argc;
	(void)argv;
	/* TODO */
	DERROR_NOTIMPLEMENTED();
	return NULL;
}

INTERN DREF DeeObject *DCALL
capi_strfry(size_t argc, DeeObject **__restrict argv) {
	(void)argc;
	(void)argv;
	/* TODO */
	DERROR_NOTIMPLEMENTED();
	return NULL;
}

//INTERN DREF DeeObject *DCALL capi_strsep(size_t argc, DeeObject **__restrict argv);
//INTERN DREF DeeObject *DCALL capi_strtok_r(size_t argc, DeeObject **__restrict argv);


DECL_END


#endif /* !GUARD_DEX_CTYPES_C_STRING_C */
