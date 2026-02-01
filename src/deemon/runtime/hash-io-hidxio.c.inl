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
#include "hash-io.c"
#define LOCAL_HIDXIO_NBITS 16
#endif /* __INTELLISENSE__ */

#include <deemon/api.h>

#include <deemon/system-features.h> /* memmoveb, memmovel, memmoveq, memmovew */
#include <deemon/types.h>
#include <deemon/util/hash-io.h> /* Dee_HASH_HIDXIO_COUNT, Dee_hash_htab */

#include <hybrid/typecore.h> /* __CHAR_BIT__ */

#include <stddef.h> /* size_t */

DECL_BEGIN

#define F(x) PP_CAT2(x, LOCAL_HIDXIO_NBITS)
#define T    PP_CAT3(uint, LOCAL_HIDXIO_NBITS, _t)

#if LOCAL_HIDXIO_NBITS == 8
#define LOCAL_MAYBE_PUBLIC PUBLIC
#else /* LOCAL_HIDXIO_NBITS == 8 */
#define LOCAL_MAYBE_PUBLIC INTERN
#endif /* LOCAL_HIDXIO_NBITS == 8 */

#if LOCAL_HIDXIO_NBITS == 4
#define LOCAL_HIDXIO_NBITS_DIV2 2
#define LOCAL_HIDXIO_NBITS_MUL2 8
#elif LOCAL_HIDXIO_NBITS == 8
#define LOCAL_HIDXIO_NBITS_DIV2 4
#define LOCAL_HIDXIO_NBITS_MUL2 16
#elif LOCAL_HIDXIO_NBITS == 16
#define LOCAL_HIDXIO_NBITS_DIV2 8
#define LOCAL_HIDXIO_NBITS_MUL2 32
#elif LOCAL_HIDXIO_NBITS == 32
#define LOCAL_HIDXIO_NBITS_DIV2 16
#define LOCAL_HIDXIO_NBITS_MUL2 64
#elif LOCAL_HIDXIO_NBITS == 64
#define LOCAL_HIDXIO_NBITS_DIV2 32
#define LOCAL_HIDXIO_NBITS_MUL2 128
#elif LOCAL_HIDXIO_NBITS == 128
#define LOCAL_HIDXIO_NBITS_DIV2 64
#define LOCAL_HIDXIO_NBITS_MUL2 256
#else /* LOCAL_HIDXIO_NBITS == ... */
#define LOCAL_HIDXIO_NBITS_DIV2 (LOCAL_HIDXIO_NBITS/2)
#define LOCAL_HIDXIO_NBITS_MUL2 (LOCAL_HIDXIO_NBITS*2)
#endif /* LOCAL_HIDXIO_NBITS != ... */

#if LOCAL_HIDXIO_NBITS == 8
#define LOCAL_memmove memmoveb
#elif LOCAL_HIDXIO_NBITS == 16
#define LOCAL_memmove memmovew
#elif LOCAL_HIDXIO_NBITS == 32
#define LOCAL_memmove memmovel
#elif LOCAL_HIDXIO_NBITS == 64
#define LOCAL_memmove memmoveq
#endif /* ... */

#define Tlwr PP_CAT3(uint, LOCAL_HIDXIO_NBITS_DIV2, _t)
#define Tupr PP_CAT3(uint, LOCAL_HIDXIO_NBITS_MUL2, _t)

LOCAL_MAYBE_PUBLIC WUNUSED NONNULL((1)) size_t DCALL
F(Dee_hash_gethidx)(union Dee_hash_htab const *__restrict htab, size_t index) {
	return ((T const *)htab)[index];
}

LOCAL_MAYBE_PUBLIC NONNULL((1)) void DCALL
F(Dee_hash_sethidx)(union Dee_hash_htab *__restrict htab, size_t index, size_t value) {
	((T *)htab)[index] = (T)value;
}

INTERN NONNULL((1)) void DCALL
F(Dee_hash_movhidx)(union Dee_hash_htab *dst, union Dee_hash_htab const *src, size_t n_words) {
	LOCAL_memmove(dst, src, n_words);
}

#if LOCAL_HIDXIO_NBITS < ((1 << (Dee_HASH_HIDXIO_COUNT - 1)) * __CHAR_BIT__)
INTERN NONNULL((1)) void DCALL
F(Dee_hash_uprhidx)(union Dee_hash_htab *dst, union Dee_hash_htab const *src, size_t n_words) {
	Tupr *tdst = (Tupr *)dst;
	T *tsrc = (T *)src;
	ASSERT(dst >= src);
	while (n_words) {
		--n_words;
		tdst[n_words] = (Tupr)tsrc[n_words];
	}
}
#endif /* LOCAL_HIDXIO_NBITS < ((1 << (Dee_HASH_HIDXIO_COUNT - 1)) * __CHAR_BIT__) */

#if LOCAL_HIDXIO_NBITS > (1 * __CHAR_BIT__)
INTERN NONNULL((1)) void DCALL
F(Dee_hash_lwrhidx)(union Dee_hash_htab *dst, union Dee_hash_htab const *src, size_t n_words) {
	size_t i;
	Tlwr *tdst = (Tlwr *)dst;
	T *tsrc = (T *)src;
	ASSERT(dst <= src);
	for (i = 0; i < n_words; ++i) {
		tdst[i] = (Tlwr)tsrc[i];
	}
}
#endif /* LOCAL_HIDXIO_NBITS > (1 * __CHAR_BIT__) */


INTERN NONNULL((1)) void DCALL
F(Dee_hash_decafter)(union Dee_hash_htab *htab, Dee_hash_t hmask,
                     /*virt*/ Dee_hash_vidx_t vtab_threshold) {
	Dee_hash_t i;
	T *typed_htab = (T *)htab;
	for (i = 0; i <= hmask; ++i) {
		if (typed_htab[i] >= (T)vtab_threshold)
			--typed_htab[i];
	}
}

INTERN NONNULL((1)) void DCALL
F(Dee_hash_incafter)(union Dee_hash_htab *htab, Dee_hash_t hmask,
                     /*virt*/ Dee_hash_vidx_t vtab_threshold) {
	Dee_hash_t i;
	T *typed_htab = (T *)htab;
	for (i = 0; i <= hmask; ++i) {
		if (typed_htab[i] >= (T)vtab_threshold)
			++typed_htab[i];
	}
}

INTERN NONNULL((1)) void DCALL
F(Dee_hash_decrange)(union Dee_hash_htab *htab, Dee_hash_t hmask,
                     /*virt*/ Dee_hash_vidx_t vtab_min,
                     /*virt*/ Dee_hash_vidx_t vtab_max) {
	Dee_hash_t i;
	T *typed_htab = (T *)htab;
	for (i = 0; i <= hmask; ++i) {
		if (typed_htab[i] >= (T)vtab_min && typed_htab[i] <= (T)vtab_max)
			--typed_htab[i];
	}
}

INTERN NONNULL((1)) void DCALL
F(Dee_hash_incrange)(union Dee_hash_htab *htab, Dee_hash_t hmask,
                     /*virt*/ Dee_hash_vidx_t vtab_min,
                     /*virt*/ Dee_hash_vidx_t vtab_max) {
	Dee_hash_t i;
	T *typed_htab = (T *)htab;
	for (i = 0; i <= hmask; ++i) {
		if (typed_htab[i] >= (T)vtab_min && typed_htab[i] <= (T)vtab_max)
			++typed_htab[i];
	}
}

#undef LOCAL_memmove
#undef Tlwr
#undef Tupr
#undef LOCAL_HIDXIO_NBITS_DIV2
#undef LOCAL_HIDXIO_NBITS_MUL2
#undef LOCAL_MAYBE_PUBLIC
#undef T
#undef F

DECL_END

#undef LOCAL_HIDXIO_NBITS
