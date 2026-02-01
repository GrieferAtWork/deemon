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
#ifndef GUARD_DEEMON_RUNTIME_HASH_IO_C
#define GUARD_DEEMON_RUNTIME_HASH_IO_C 1

#include <deemon/api.h>

#include <deemon/util/hash-io.h> /* Dee_HASH_HIDXIO_COUNT, Dee_hash_gethidx8, Dee_hash_hidxio_struct, Dee_hash_htab, Dee_hash_sethidx8 */

DECL_BEGIN

/*DFUNDEF WUNUSED NONNULL((1)) size_t DCALL Dee_hash_gethidx8(void *__restrict htab, size_t index);*/
/*DFUNDEF NONNULL((1)) void DCALL Dee_hash_sethidx8(void *__restrict htab, size_t index, size_t value);*/
INTDEF NONNULL((1)) void DCALL Dee_hash_movhidx8(union Dee_hash_htab *__restrict dst, union Dee_hash_htab const *__restrict src, size_t n_words);

#if Dee_HASH_HIDXIO_COUNT >= 2
INTDEF WUNUSED NONNULL((1)) size_t DCALL Dee_hash_gethidx16(union Dee_hash_htab const *__restrict htab, size_t index);
INTDEF NONNULL((1)) void DCALL Dee_hash_sethidx16(union Dee_hash_htab *__restrict htab, size_t index, size_t value);
INTDEF NONNULL((1)) void DCALL Dee_hash_movhidx16(union Dee_hash_htab *__restrict dst, union Dee_hash_htab const *__restrict src, size_t n_words);
#define Dee_hash_uprhidx8_PTR &Dee_hash_uprhidx8
INTDEF NONNULL((1)) void DCALL Dee_hash_uprhidx8(union Dee_hash_htab *__restrict dst, union Dee_hash_htab const *__restrict src, size_t n_words);
INTDEF NONNULL((1)) void DCALL Dee_hash_lwrhidx16(union Dee_hash_htab *__restrict dst, union Dee_hash_htab const *__restrict src, size_t n_words);
#endif /* Dee_HASH_HIDXIO_COUNT >= 2 */

#if Dee_HASH_HIDXIO_COUNT >= 3
INTDEF WUNUSED NONNULL((1)) size_t DCALL Dee_hash_gethidx32(union Dee_hash_htab const *__restrict htab, size_t index);
INTDEF NONNULL((1)) void DCALL Dee_hash_sethidx32(union Dee_hash_htab *__restrict htab, size_t index, size_t value);
INTDEF NONNULL((1)) void DCALL Dee_hash_movhidx32(union Dee_hash_htab *__restrict dst, union Dee_hash_htab const *__restrict src, size_t n_words);
#define Dee_hash_uprhidx16_PTR &Dee_hash_uprhidx16
INTDEF NONNULL((1)) void DCALL Dee_hash_uprhidx16(union Dee_hash_htab *__restrict dst, union Dee_hash_htab const *__restrict src, size_t n_words);
INTDEF NONNULL((1)) void DCALL Dee_hash_lwrhidx32(union Dee_hash_htab *__restrict dst, union Dee_hash_htab const *__restrict src, size_t n_words);
#endif /* Dee_HASH_HIDXIO_COUNT >= 3 */

#if Dee_HASH_HIDXIO_COUNT >= 4
INTDEF WUNUSED NONNULL((1)) size_t DCALL Dee_hash_gethidx64(void const *__restrict htab, size_t index);
INTDEF NONNULL((1)) void DCALL Dee_hash_sethidx64(void *__restrict htab, size_t index, size_t value);
INTDEF NONNULL((1)) void DCALL Dee_hash_movhidx64(void *__restrict dst, void const *__restrict src, size_t n_words);
#define Dee_hash_uprhidx32_PTR &Dee_hash_uprhidx32
INTDEF NONNULL((1)) void DCALL Dee_hash_uprhidx32(void *__restrict dst, void const *__restrict src, size_t n_words);
INTDEF NONNULL((1)) void DCALL Dee_hash_lwrhidx64(void *__restrict dst, void const *__restrict src, size_t n_words);
#endif /* Dee_HASH_HIDXIO_COUNT >= 4 */

#ifndef Dee_hash_uprhidx8_PTR
#define Dee_hash_uprhidx8_PTR NULL
#endif /* !Dee_hash_uprhidx8_PTR */
#ifndef Dee_hash_uprhidx16_PTR
#define Dee_hash_uprhidx16_PTR NULL
#endif /* !Dee_hash_uprhidx16_PTR */
#ifndef Dee_hash_uprhidx32_PTR
#define Dee_hash_uprhidx32_PTR NULL
#endif /* !Dee_hash_uprhidx32_PTR */

#ifndef __INTELLISENSE__
DECL_END
#define LOCAL_HIDXIO_NBITS 8
#include "hash-io-hidxio.c.inl"

#if Dee_HASH_HIDXIO_COUNT >= 2
#define LOCAL_HIDXIO_NBITS 16
#include "hash-io-hidxio.c.inl"
#endif /* Dee_HASH_HIDXIO_COUNT >= 2 */

#if Dee_HASH_HIDXIO_COUNT >= 3
#define LOCAL_HIDXIO_NBITS 32
#include "hash-io-hidxio.c.inl"
#endif /* Dee_HASH_HIDXIO_COUNT >= 3 */

#if Dee_HASH_HIDXIO_COUNT >= 4
#define LOCAL_HIDXIO_NBITS 64
#include "hash-io-hidxio.c.inl"
#endif /* Dee_HASH_HIDXIO_COUNT >= 4 */
DECL_BEGIN
#endif /* !__INTELLISENSE__ */


PUBLIC_TPCONST struct Dee_hash_hidxio_struct tpconst Dee_hash_hidxio[Dee_HASH_HIDXIO_COUNT] = {
	/* [0] = */ {
		/* .hxio_get = */ &Dee_hash_gethidx8,
		/* .hxio_set = */ &Dee_hash_sethidx8,
		/* .hxio_mov = */ &Dee_hash_movhidx8,
		/* .hxio_upr = */ Dee_hash_uprhidx8_PTR,
		/* .hxio_lwr = */ NULL
	},
#if Dee_HASH_HIDXIO_COUNT >= 2
	/* [1] = */ {
		/* .hxio_get = */ &Dee_hash_gethidx16,
		/* .hxio_set = */ &Dee_hash_sethidx16,
		/* .hxio_mov = */ &Dee_hash_movhidx16,
		/* .hxio_upr = */ Dee_hash_uprhidx16_PTR,
		/* .hxio_lwr = */ &Dee_hash_lwrhidx16
	},
#if Dee_HASH_HIDXIO_COUNT >= 3
	/* [2] = */ {
		/* .hxio_get = */ &Dee_hash_gethidx32,
		/* .hxio_set = */ &Dee_hash_sethidx32,
		/* .hxio_mov = */ &Dee_hash_movhidx32,
		/* .hxio_upr = */ Dee_hash_uprhidx32_PTR,
		/* .hxio_lwr = */ &Dee_hash_lwrhidx32
	},
#if Dee_HASH_HIDXIO_COUNT >= 4
	/* [3] = */ {
		/* .hxio_get = */ &Dee_hash_gethidx64,
		/* .hxio_set = */ &Dee_hash_sethidx64,
		/* .hxio_mov = */ &Dee_hash_movhidx64,
		/* .hxio_upr = */ NULL,
		/* .hxio_lwr = */ &Dee_hash_lwrhidx64
	},
#endif /* Dee_HASH_HIDXIO_COUNT >= 4 */
#endif /* Dee_HASH_HIDXIO_COUNT >= 3 */
#endif /* Dee_HASH_HIDXIO_COUNT >= 2 */
};

DECL_END

#endif /* !GUARD_DEEMON_RUNTIME_HASH_IO_C */
