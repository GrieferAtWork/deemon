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
#ifndef GUARD_DEX_CTYPES_C_API_H
#define GUARD_DEX_CTYPES_C_API_H 1

#include <deemon/api.h>
#include <deemon/object.h>
#include <deemon/objmethod.h>

DECL_BEGIN

/* c_malloc */
INTDEF DeeCMethodObject c_malloc_free;
INTDEF DeeCMethodObject c_malloc_malloc;
INTDEF DeeCMethodObject c_malloc_realloc;
INTDEF DeeCMethodObject c_malloc_calloc;
INTDEF DeeCMethodObject c_malloc_strdup;
INTDEF DeeCMethodObject c_malloc_trymalloc;
INTDEF DeeCMethodObject c_malloc_tryrealloc;
INTDEF DeeCMethodObject c_malloc_trycalloc;
INTDEF DeeCMethodObject c_malloc_trystrdup;

/* c_string (memory) */
INTDEF DeeCMethodObject c_string_memcpy;
INTDEF DeeCMethodObject c_string_mempcpy;
INTDEF DeeCMethodObject c_string_memccpy;
INTDEF DeeCMethodObject c_string_memset;
INTDEF DeeCMethodObject c_string_mempset;
INTDEF DeeCMethodObject c_string_bzero;
INTDEF DeeCMethodObject c_string_memmove;
INTDEF DeeCMethodObject c_string_mempmove;
INTDEF DeeCMethodObject c_string_memchr;
INTDEF DeeCMethodObject c_string_memlen;
INTDEF DeeCMethodObject c_string_memend;
INTDEF DeeCMethodObject c_string_memrchr;
INTDEF DeeCMethodObject c_string_memrlen;
INTDEF DeeCMethodObject c_string_memrend;
INTDEF DeeCMethodObject c_string_rawmemchr;
INTDEF DeeCMethodObject c_string_rawmemlen;
INTDEF DeeCMethodObject c_string_rawmemrchr;
INTDEF DeeCMethodObject c_string_rawmemrlen;
INTDEF DeeCMethodObject c_string_memxchr;
INTDEF DeeCMethodObject c_string_memxlen;
INTDEF DeeCMethodObject c_string_memxend;
INTDEF DeeCMethodObject c_string_memrxchr;
INTDEF DeeCMethodObject c_string_memrxlen;
INTDEF DeeCMethodObject c_string_memrxend;
INTDEF DeeCMethodObject c_string_rawmemxchr;
INTDEF DeeCMethodObject c_string_rawmemxlen;
INTDEF DeeCMethodObject c_string_rawmemrxchr;
INTDEF DeeCMethodObject c_string_rawmemrxlen;
INTDEF DeeCMethodObject c_string_bcmp;
INTDEF DeeCMethodObject c_string_memcmp;
INTDEF DeeCMethodObject c_string_memcasecmp;
INTDEF DeeCMethodObject c_string_memmem;
INTDEF DeeCMethodObject c_string_memcasemem;
INTDEF DeeCMethodObject c_string_memrmem;
INTDEF DeeCMethodObject c_string_memcasermem;
INTDEF DeeCMethodObject c_string_memrev;

/* c_string (strings) */
INTDEF DeeCMethodObject c_string_strlen;
INTDEF DeeCMethodObject c_string_strend;
INTDEF DeeCMethodObject c_string_strnlen;
INTDEF DeeCMethodObject c_string_strnend;
INTDEF DeeCMethodObject c_string_strchr;
INTDEF DeeCMethodObject c_string_strrchr;
INTDEF DeeCMethodObject c_string_strnchr;
INTDEF DeeCMethodObject c_string_strnrchr;
INTDEF DeeCMethodObject c_string_stroff;
INTDEF DeeCMethodObject c_string_strroff;
INTDEF DeeCMethodObject c_string_strnoff;
INTDEF DeeCMethodObject c_string_strnroff;
INTDEF DeeCMethodObject c_string_strchrnul;
INTDEF DeeCMethodObject c_string_strrchrnul;
INTDEF DeeCMethodObject c_string_strnchrnul;
INTDEF DeeCMethodObject c_string_strnrchrnul;
INTDEF DeeCMethodObject c_string_strcmp;
INTDEF DeeCMethodObject c_string_strncmp;
INTDEF DeeCMethodObject c_string_strcasecmp;
INTDEF DeeCMethodObject c_string_strncasecmp;
INTDEF DeeCMethodObject c_string_strcpy;
INTDEF DeeCMethodObject c_string_strcat;
INTDEF DeeCMethodObject c_string_strncpy;
INTDEF DeeCMethodObject c_string_strncat;
INTDEF DeeCMethodObject c_string_stpcpy;
INTDEF DeeCMethodObject c_string_stpncpy;
INTDEF DeeCMethodObject c_string_strstr;
INTDEF DeeCMethodObject c_string_strcasestr;
INTDEF DeeCMethodObject c_string_strnstr;
INTDEF DeeCMethodObject c_string_strncasestr;
INTDEF DeeCMethodObject c_string_strverscmp;
INTDEF DeeCMethodObject c_string_strspn;
INTDEF DeeCMethodObject c_string_strcspn;
INTDEF DeeCMethodObject c_string_strpbrk;
INTDEF DeeCMethodObject c_string_strrev;
INTDEF DeeCMethodObject c_string_strnrev;
INTDEF DeeCMethodObject c_string_strlwr;
INTDEF DeeCMethodObject c_string_strupr;
INTDEF DeeCMethodObject c_string_strnlwr;
INTDEF DeeCMethodObject c_string_strnupr;
INTDEF DeeCMethodObject c_string_strset;
INTDEF DeeCMethodObject c_string_strnset;
INTDEF DeeCMethodObject c_string_strfry;
INTDEF DeeCMethodObject c_string_memfrob;
INTDEF DeeCMethodObject c_string_strsep;
INTDEF DeeCMethodObject c_string_stresep;
INTDEF DeeCMethodObject c_string_strtok;
INTDEF DeeCMethodObject c_string_strtok_r;
INTDEF DeeCMethodObject c_string_basename;

/* Atomic functions */
INTDEF DeeCMethodObject c_atomic_atomic_cmpxch;
INTDEF DeeCMethodObject c_atomic_atomic_cmpxch_val;
INTDEF DeeCMethodObject c_atomic_atomic_fetchadd;
INTDEF DeeCMethodObject c_atomic_atomic_fetchsub;
INTDEF DeeCMethodObject c_atomic_atomic_fetchand;
INTDEF DeeCMethodObject c_atomic_atomic_fetchor;
INTDEF DeeCMethodObject c_atomic_atomic_fetchxor;
INTDEF DeeCMethodObject c_atomic_atomic_fetchnand;
INTDEF DeeCMethodObject c_atomic_atomic_addfetch;
INTDEF DeeCMethodObject c_atomic_atomic_subfetch;
INTDEF DeeCMethodObject c_atomic_atomic_andfetch;
INTDEF DeeCMethodObject c_atomic_atomic_orfetch;
INTDEF DeeCMethodObject c_atomic_atomic_xorfetch;
INTDEF DeeCMethodObject c_atomic_atomic_nandfetch;
INTDEF DeeCMethodObject c_atomic_atomic_add;
INTDEF DeeCMethodObject c_atomic_atomic_sub;
INTDEF DeeCMethodObject c_atomic_atomic_and;
INTDEF DeeCMethodObject c_atomic_atomic_or;
INTDEF DeeCMethodObject c_atomic_atomic_xor;
INTDEF DeeCMethodObject c_atomic_atomic_nand;
INTDEF DeeCMethodObject c_atomic_atomic_write;
INTDEF DeeCMethodObject c_atomic_atomic_fetchinc;
INTDEF DeeCMethodObject c_atomic_atomic_fetchdec;
INTDEF DeeCMethodObject c_atomic_atomic_incfetch;
INTDEF DeeCMethodObject c_atomic_atomic_decfetch;
INTDEF DeeCMethodObject c_atomic_atomic_read;
INTDEF DeeCMethodObject c_atomic_atomic_inc;
INTDEF DeeCMethodObject c_atomic_atomic_dec;

/* Futex API */
INTDEF DeeCMethodObject c_atomic_futex_wakeone;
INTDEF DeeCMethodObject c_atomic_futex_wakeall;
INTDEF DeeCMethodObject c_atomic_futex_wait;
INTDEF DeeCMethodObject c_atomic_futex_timedwait;

DECL_END


#endif /* !GUARD_DEX_CTYPES_C_API_H */
