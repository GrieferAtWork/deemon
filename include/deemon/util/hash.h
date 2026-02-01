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
/*!export **/
/*!export Dee_Hash**/
/*!export Dee_HASHOF_***/
#ifndef GUARD_DEEMON_UTIL_HASH_H
#define GUARD_DEEMON_UTIL_HASH_H 1 /*!export-*/

#include "../api.h"

#include "../types.h" /* Dee_hash_t */

#ifndef __INTELLISENSE__
#include "../system-features.h" /* strlen */
#endif /* !__INTELLISENSE__ */

DECL_BEGIN

/* Hashing helpers. */
DFUNDEF ATTR_PURE WUNUSED ATTR_INS(1, 2) Dee_hash_t (DCALL Dee_HashPtr)(void const *__restrict ptr, size_t n_bytes);
DFUNDEF ATTR_PURE WUNUSED ATTR_INS(1, 2) Dee_hash_t (DCALL Dee_HashCasePtr)(void const *__restrict ptr, size_t n_bytes);
DFUNDEF ATTR_PURE WUNUSED ATTR_IN(1) Dee_hash_t (DCALL Dee_HashStr)(char const *__restrict str);
#ifdef __INTELLISENSE__
DFUNDEF ATTR_PURE WUNUSED ATTR_IN(1) Dee_hash_t (DCALL Dee_HashCaseStr)(char const *__restrict str);
#else /* __INTELLISENSE__ */
#define Dee_HashCaseStr(str) Dee_HashCasePtr(str, strlen(str))
#endif /* !__INTELLISENSE__ */

/* Combine 2 hash values into 1, while losing as little entropy
 * from either as possible. Note that this function tries to
 * include the order of arguments in the result, meaning that:
 * >> Dee_HashCombine(a, b) != Dee_HashCombine(b, a) */
DFUNDEF ATTR_CONST WUNUSED Dee_hash_t
(DFCALL Dee_HashCombine)(Dee_hash_t a, Dee_hash_t b);

/* This is the special hash we assign to empty sequences.
 *
 * It doesn't *have* to be zero; it could be anything. But
 * thinking about it, only zero *really* makes sense... */
#define Dee_HASHOF_EMPTY_SEQUENCE 0
#define Dee_HASHOF_UNBOUND_ITEM   0
#define Dee_HASHOF_RECURSIVE_ITEM 0



/* Hash a utf-8 encoded string.
 * You can think of these as hashing the ordinal values of the given string,
 * thus allowing this hashing function to return the same value for a string
 * encoded in utf-8, as `Dee_Hash2Byte()' would for a 2-byte, and Dee_Hash4Byte()
 * for a 4-byte string. */
DFUNDEF ATTR_PURE WUNUSED ATTR_INS(1, 2) Dee_hash_t (DCALL Dee_HashUtf8)(char const *__restrict ptr, size_t n_bytes);
DFUNDEF ATTR_PURE WUNUSED ATTR_INS(1, 2) Dee_hash_t (DCALL Dee_HashCaseUtf8)(char const *__restrict ptr, size_t n_bytes);

/* Same as the regular hashing function, but with the guaranty that
 * for integer arrays where all items contain values `<= 0xff', the
 * return value is identical to a call to `Dee_HashPtr()' with the
 * array contents down-casted to the fitting data type. */
#ifdef __INTELLISENSE__
DFUNDEF ATTR_PURE WUNUSED ATTR_INS(1, 2) Dee_hash_t (DCALL Dee_Hash1Byte)(uint8_t const *__restrict ptr, size_t n_bytes);
DFUNDEF ATTR_PURE WUNUSED ATTR_INS(1, 2) Dee_hash_t (DCALL Dee_HashCase1Byte)(uint8_t const *__restrict ptr, size_t n_bytes);
#else /* __INTELLISENSE__ */
#define Dee_Hash1Byte(ptr, n_bytes)     Dee_HashPtr(ptr, n_bytes)
#define Dee_HashCase1Byte(ptr, n_bytes) Dee_HashCasePtr(ptr, n_bytes)
#endif /* !__INTELLISENSE__ */
DFUNDEF ATTR_PURE WUNUSED ATTR_INS(1, 2) Dee_hash_t (DCALL Dee_Hash2Byte)(uint16_t const *__restrict ptr, size_t n_words);
DFUNDEF ATTR_PURE WUNUSED ATTR_INS(1, 2) Dee_hash_t (DCALL Dee_Hash4Byte)(uint32_t const *__restrict ptr, size_t n_dwords);
DFUNDEF ATTR_PURE WUNUSED ATTR_INS(1, 2) Dee_hash_t (DCALL Dee_HashCase2Byte)(uint16_t const *__restrict ptr, size_t n_words);
DFUNDEF ATTR_PURE WUNUSED ATTR_INS(1, 2) Dee_hash_t (DCALL Dee_HashCase4Byte)(uint32_t const *__restrict ptr, size_t n_dwords);

/* Generic object hashing: Use the address of the object.
 * HINT: We ignore the lower 6 bits because they're
 *       often just ZERO(0) due to alignment. */
#define DeeObject_HashGeneric(ob) Dee_HashPointer(ob)
#define Dee_HashPointer(ptr)      ((Dee_hash_t)(ptr) >> 6)
#define DeeObject_Id(ob)          ((uintptr_t)(ob))

DECL_END

#endif /* !GUARD_DEEMON_UTIL_HASH_H */
