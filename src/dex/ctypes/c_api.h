/* Copyright (c) 2018-2023 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2023 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEX_CTYPES_C_API_H
#define GUARD_DEX_CTYPES_C_API_H 1

#include "libctypes.h"
/**/

#include <deemon/object.h>

DECL_BEGIN

/* c_malloc */
INTDEF WUNUSED DREF DeeObject *DCALL capi_malloc(size_t argc, DeeObject *const *argv);
INTDEF WUNUSED DREF DeeObject *DCALL capi_free(size_t argc, DeeObject *const *argv);
INTDEF WUNUSED DREF DeeObject *DCALL capi_realloc(size_t argc, DeeObject *const *argv);
INTDEF WUNUSED DREF DeeObject *DCALL capi_calloc(size_t argc, DeeObject *const *argv);
INTDEF WUNUSED DREF DeeObject *DCALL capi_strdup(size_t argc, DeeObject *const *argv);
INTDEF WUNUSED DREF DeeObject *DCALL capi_trymalloc(size_t argc, DeeObject *const *argv);
INTDEF WUNUSED DREF DeeObject *DCALL capi_tryrealloc(size_t argc, DeeObject *const *argv);
INTDEF WUNUSED DREF DeeObject *DCALL capi_trycalloc(size_t argc, DeeObject *const *argv);
INTDEF WUNUSED DREF DeeObject *DCALL capi_trystrdup(size_t argc, DeeObject *const *argv);

/* c_string (memory) */
INTDEF WUNUSED DREF DeeObject *DCALL capi_memcpy(size_t argc, DeeObject *const *argv);
INTDEF WUNUSED DREF DeeObject *DCALL capi_mempcpy(size_t argc, DeeObject *const *argv);
INTDEF WUNUSED DREF DeeObject *DCALL capi_memccpy(size_t argc, DeeObject *const *argv);
INTDEF WUNUSED DREF DeeObject *DCALL capi_memset(size_t argc, DeeObject *const *argv);
INTDEF WUNUSED DREF DeeObject *DCALL capi_mempset(size_t argc, DeeObject *const *argv);
INTDEF WUNUSED DREF DeeObject *DCALL capi_bzero(size_t argc, DeeObject *const *argv);
INTDEF WUNUSED DREF DeeObject *DCALL capi_memmove(size_t argc, DeeObject *const *argv);
INTDEF WUNUSED DREF DeeObject *DCALL capi_mempmove(size_t argc, DeeObject *const *argv);
INTDEF WUNUSED DREF DeeObject *DCALL capi_memchr(size_t argc, DeeObject *const *argv);
INTDEF WUNUSED DREF DeeObject *DCALL capi_memlen(size_t argc, DeeObject *const *argv);
INTDEF WUNUSED DREF DeeObject *DCALL capi_memend(size_t argc, DeeObject *const *argv);
INTDEF WUNUSED DREF DeeObject *DCALL capi_memrchr(size_t argc, DeeObject *const *argv);
INTDEF WUNUSED DREF DeeObject *DCALL capi_memrlen(size_t argc, DeeObject *const *argv);
INTDEF WUNUSED DREF DeeObject *DCALL capi_memrend(size_t argc, DeeObject *const *argv);
INTDEF WUNUSED DREF DeeObject *DCALL capi_rawmemchr(size_t argc, DeeObject *const *argv);
INTDEF WUNUSED DREF DeeObject *DCALL capi_rawmemlen(size_t argc, DeeObject *const *argv);
INTDEF WUNUSED DREF DeeObject *DCALL capi_rawmemrchr(size_t argc, DeeObject *const *argv);
INTDEF WUNUSED DREF DeeObject *DCALL capi_rawmemrlen(size_t argc, DeeObject *const *argv);
INTDEF WUNUSED DREF DeeObject *DCALL capi_memxchr(size_t argc, DeeObject *const *argv);
INTDEF WUNUSED DREF DeeObject *DCALL capi_memxlen(size_t argc, DeeObject *const *argv);
INTDEF WUNUSED DREF DeeObject *DCALL capi_memxend(size_t argc, DeeObject *const *argv);
INTDEF WUNUSED DREF DeeObject *DCALL capi_memxrchr(size_t argc, DeeObject *const *argv);
INTDEF WUNUSED DREF DeeObject *DCALL capi_memxrlen(size_t argc, DeeObject *const *argv);
INTDEF WUNUSED DREF DeeObject *DCALL capi_memxrend(size_t argc, DeeObject *const *argv);
INTDEF WUNUSED DREF DeeObject *DCALL capi_rawmemxchr(size_t argc, DeeObject *const *argv);
INTDEF WUNUSED DREF DeeObject *DCALL capi_rawmemxlen(size_t argc, DeeObject *const *argv);
INTDEF WUNUSED DREF DeeObject *DCALL capi_rawmemxrchr(size_t argc, DeeObject *const *argv);
INTDEF WUNUSED DREF DeeObject *DCALL capi_rawmemxrlen(size_t argc, DeeObject *const *argv);
INTDEF WUNUSED DREF DeeObject *DCALL capi_bcmp(size_t argc, DeeObject *const *argv);
INTDEF WUNUSED DREF DeeObject *DCALL capi_memcmp(size_t argc, DeeObject *const *argv);
INTDEF WUNUSED DREF DeeObject *DCALL capi_memcasecmp(size_t argc, DeeObject *const *argv);
INTDEF WUNUSED DREF DeeObject *DCALL capi_memmem(size_t argc, DeeObject *const *argv);
INTDEF WUNUSED DREF DeeObject *DCALL capi_memcasemem(size_t argc, DeeObject *const *argv);
INTDEF WUNUSED DREF DeeObject *DCALL capi_memrmem(size_t argc, DeeObject *const *argv);
INTDEF WUNUSED DREF DeeObject *DCALL capi_memcasermem(size_t argc, DeeObject *const *argv);
INTDEF WUNUSED DREF DeeObject *DCALL capi_memrev(size_t argc, DeeObject *const *argv);

/* c_string (strings) */
INTDEF WUNUSED DREF DeeObject *DCALL capi_strlen(size_t argc, DeeObject *const *argv);
INTDEF WUNUSED DREF DeeObject *DCALL capi_strend(size_t argc, DeeObject *const *argv);
INTDEF WUNUSED DREF DeeObject *DCALL capi_strnlen(size_t argc, DeeObject *const *argv);
INTDEF WUNUSED DREF DeeObject *DCALL capi_strnend(size_t argc, DeeObject *const *argv);
INTDEF WUNUSED DREF DeeObject *DCALL capi_strchr(size_t argc, DeeObject *const *argv);
INTDEF WUNUSED DREF DeeObject *DCALL capi_strrchr(size_t argc, DeeObject *const *argv);
INTDEF WUNUSED DREF DeeObject *DCALL capi_strnchr(size_t argc, DeeObject *const *argv);
INTDEF WUNUSED DREF DeeObject *DCALL capi_strnrchr(size_t argc, DeeObject *const *argv);
INTDEF WUNUSED DREF DeeObject *DCALL capi_stroff(size_t argc, DeeObject *const *argv);
INTDEF WUNUSED DREF DeeObject *DCALL capi_strroff(size_t argc, DeeObject *const *argv);
INTDEF WUNUSED DREF DeeObject *DCALL capi_strnoff(size_t argc, DeeObject *const *argv);
INTDEF WUNUSED DREF DeeObject *DCALL capi_strnroff(size_t argc, DeeObject *const *argv);
INTDEF WUNUSED DREF DeeObject *DCALL capi_strchrnul(size_t argc, DeeObject *const *argv);
INTDEF WUNUSED DREF DeeObject *DCALL capi_strrchrnul(size_t argc, DeeObject *const *argv);
INTDEF WUNUSED DREF DeeObject *DCALL capi_strnchrnul(size_t argc, DeeObject *const *argv);
INTDEF WUNUSED DREF DeeObject *DCALL capi_strnrchrnul(size_t argc, DeeObject *const *argv);
INTDEF WUNUSED DREF DeeObject *DCALL capi_strcmp(size_t argc, DeeObject *const *argv);
INTDEF WUNUSED DREF DeeObject *DCALL capi_strncmp(size_t argc, DeeObject *const *argv);
INTDEF WUNUSED DREF DeeObject *DCALL capi_strcasecmp(size_t argc, DeeObject *const *argv);
INTDEF WUNUSED DREF DeeObject *DCALL capi_strncasecmp(size_t argc, DeeObject *const *argv);
INTDEF WUNUSED DREF DeeObject *DCALL capi_strcpy(size_t argc, DeeObject *const *argv);
INTDEF WUNUSED DREF DeeObject *DCALL capi_strcat(size_t argc, DeeObject *const *argv);
INTDEF WUNUSED DREF DeeObject *DCALL capi_strncpy(size_t argc, DeeObject *const *argv);
INTDEF WUNUSED DREF DeeObject *DCALL capi_strncat(size_t argc, DeeObject *const *argv);
INTDEF WUNUSED DREF DeeObject *DCALL capi_stpcpy(size_t argc, DeeObject *const *argv);
INTDEF WUNUSED DREF DeeObject *DCALL capi_stpncpy(size_t argc, DeeObject *const *argv);
INTDEF WUNUSED DREF DeeObject *DCALL capi_strstr(size_t argc, DeeObject *const *argv);
INTDEF WUNUSED DREF DeeObject *DCALL capi_strcasestr(size_t argc, DeeObject *const *argv);
INTDEF WUNUSED DREF DeeObject *DCALL capi_strnstr(size_t argc, DeeObject *const *argv);
INTDEF WUNUSED DREF DeeObject *DCALL capi_strncasestr(size_t argc, DeeObject *const *argv);
INTDEF WUNUSED DREF DeeObject *DCALL capi_strverscmp(size_t argc, DeeObject *const *argv);
INTDEF WUNUSED DREF DeeObject *DCALL capi_strspn(size_t argc, DeeObject *const *argv);
INTDEF WUNUSED DREF DeeObject *DCALL capi_strcspn(size_t argc, DeeObject *const *argv);
INTDEF WUNUSED DREF DeeObject *DCALL capi_strpbrk(size_t argc, DeeObject *const *argv);
INTDEF WUNUSED DREF DeeObject *DCALL capi_strrev(size_t argc, DeeObject *const *argv);
INTDEF WUNUSED DREF DeeObject *DCALL capi_strnrev(size_t argc, DeeObject *const *argv);
INTDEF WUNUSED DREF DeeObject *DCALL capi_strlwr(size_t argc, DeeObject *const *argv);
INTDEF WUNUSED DREF DeeObject *DCALL capi_strupr(size_t argc, DeeObject *const *argv);
INTDEF WUNUSED DREF DeeObject *DCALL capi_strnlwr(size_t argc, DeeObject *const *argv);
INTDEF WUNUSED DREF DeeObject *DCALL capi_strnupr(size_t argc, DeeObject *const *argv);
INTDEF WUNUSED DREF DeeObject *DCALL capi_strset(size_t argc, DeeObject *const *argv);
INTDEF WUNUSED DREF DeeObject *DCALL capi_strnset(size_t argc, DeeObject *const *argv);
INTDEF WUNUSED DREF DeeObject *DCALL capi_strfry(size_t argc, DeeObject *const *argv);
INTDEF WUNUSED DREF DeeObject *DCALL capi_memfrob(size_t argc, DeeObject *const *argv);
INTDEF WUNUSED DREF DeeObject *DCALL capi_strsep(size_t argc, DeeObject *const *argv);
INTDEF WUNUSED DREF DeeObject *DCALL capi_stresep(size_t argc, DeeObject *const *argv);
INTDEF WUNUSED DREF DeeObject *DCALL capi_strtok(size_t argc, DeeObject *const *argv);
INTDEF WUNUSED DREF DeeObject *DCALL capi_strtok_r(size_t argc, DeeObject *const *argv);
INTDEF WUNUSED DREF DeeObject *DCALL capi_basename(size_t argc, DeeObject *const *argv);

/* Atomic functions */
INTDEF WUNUSED DREF DeeObject *DCALL capi_atomic_cmpxch(size_t argc, DeeObject *const *argv);
INTDEF WUNUSED DREF DeeObject *DCALL capi_atomic_cmpxch_val(size_t argc, DeeObject *const *argv);
INTDEF WUNUSED DREF DeeObject *DCALL capi_atomic_fetchadd(size_t argc, DeeObject *const *argv);
INTDEF WUNUSED DREF DeeObject *DCALL capi_atomic_fetchsub(size_t argc, DeeObject *const *argv);
INTDEF WUNUSED DREF DeeObject *DCALL capi_atomic_fetchand(size_t argc, DeeObject *const *argv);
INTDEF WUNUSED DREF DeeObject *DCALL capi_atomic_fetchor(size_t argc, DeeObject *const *argv);
INTDEF WUNUSED DREF DeeObject *DCALL capi_atomic_fetchxor(size_t argc, DeeObject *const *argv);
INTDEF WUNUSED DREF DeeObject *DCALL capi_atomic_fetchnand(size_t argc, DeeObject *const *argv);
INTDEF WUNUSED DREF DeeObject *DCALL capi_atomic_addfetch(size_t argc, DeeObject *const *argv);
INTDEF WUNUSED DREF DeeObject *DCALL capi_atomic_subfetch(size_t argc, DeeObject *const *argv);
INTDEF WUNUSED DREF DeeObject *DCALL capi_atomic_andfetch(size_t argc, DeeObject *const *argv);
INTDEF WUNUSED DREF DeeObject *DCALL capi_atomic_orfetch(size_t argc, DeeObject *const *argv);
INTDEF WUNUSED DREF DeeObject *DCALL capi_atomic_xorfetch(size_t argc, DeeObject *const *argv);
INTDEF WUNUSED DREF DeeObject *DCALL capi_atomic_nandfetch(size_t argc, DeeObject *const *argv);
INTDEF WUNUSED DREF DeeObject *DCALL capi_atomic_add(size_t argc, DeeObject *const *argv);
INTDEF WUNUSED DREF DeeObject *DCALL capi_atomic_sub(size_t argc, DeeObject *const *argv);
INTDEF WUNUSED DREF DeeObject *DCALL capi_atomic_and(size_t argc, DeeObject *const *argv);
INTDEF WUNUSED DREF DeeObject *DCALL capi_atomic_or(size_t argc, DeeObject *const *argv);
INTDEF WUNUSED DREF DeeObject *DCALL capi_atomic_xor(size_t argc, DeeObject *const *argv);
INTDEF WUNUSED DREF DeeObject *DCALL capi_atomic_nand(size_t argc, DeeObject *const *argv);
INTDEF WUNUSED DREF DeeObject *DCALL capi_atomic_write(size_t argc, DeeObject *const *argv);
INTDEF WUNUSED DREF DeeObject *DCALL capi_atomic_fetchinc(size_t argc, DeeObject *const *argv);
INTDEF WUNUSED DREF DeeObject *DCALL capi_atomic_fetchdec(size_t argc, DeeObject *const *argv);
INTDEF WUNUSED DREF DeeObject *DCALL capi_atomic_incfetch(size_t argc, DeeObject *const *argv);
INTDEF WUNUSED DREF DeeObject *DCALL capi_atomic_decfetch(size_t argc, DeeObject *const *argv);
INTDEF WUNUSED DREF DeeObject *DCALL capi_atomic_read(size_t argc, DeeObject *const *argv);
INTDEF WUNUSED DREF DeeObject *DCALL capi_atomic_inc(size_t argc, DeeObject *const *argv);
INTDEF WUNUSED DREF DeeObject *DCALL capi_atomic_dec(size_t argc, DeeObject *const *argv);

/* Futex API */
INTDEF WUNUSED DREF DeeObject *DCALL capi_futex_wakeone(size_t argc, DeeObject *const *argv);
INTDEF WUNUSED DREF DeeObject *DCALL capi_futex_wakeall(size_t argc, DeeObject *const *argv);
INTDEF WUNUSED DREF DeeObject *DCALL capi_futex_wait(size_t argc, DeeObject *const *argv);

DECL_END


#endif /* !GUARD_DEX_CTYPES_C_API_H */
