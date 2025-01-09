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
#ifdef __INTELLISENSE__
#include "dict.c"
//#define DEFINE_dict_htab_decafter
//#define DEFINE_dict_htab_incafter
//#define DEFINE_dict_htab_decrange
#define DEFINE_dict_htab_incrange
#endif /* __INTELLISENSE__ */

#if (defined(DEFINE_dict_htab_decafter) + \
     defined(DEFINE_dict_htab_incafter) + \
     defined(DEFINE_dict_htab_decrange) + \
     defined(DEFINE_dict_htab_incrange)) != 1
#error "Must #define exactly one of these macros"
#endif /* ... */

#if defined(DEFINE_dict_htab_decafter)
#define LOCAL_dict_htab_modify           dict_htab_decafter
#define LOCAL_dict_htab_modify__PARAM(T) , /*virt*/T vtab_threshold
#define LOCAL_dict_htab_modify__ARGS(T)  , (T)vtab_threshold
#define LOCAL_modify(htab_lvalue)        \
	if ((htab_lvalue) >= vtab_threshold) \
		--(htab_lvalue)
#elif defined(DEFINE_dict_htab_incafter)
#define LOCAL_dict_htab_modify           dict_htab_incafter
#define LOCAL_dict_htab_modify__PARAM(T) , /*virt*/T vtab_threshold
#define LOCAL_dict_htab_modify__ARGS(T)  , (T)vtab_threshold
#define LOCAL_modify(htab_lvalue)        \
	if ((htab_lvalue) >= vtab_threshold) \
		++(htab_lvalue)
#elif defined(DEFINE_dict_htab_decrange)
#define LOCAL_dict_htab_modify           dict_htab_decrange
#define LOCAL_dict_htab_modify__PARAM(T) , /*virt*/T vtab_min, /*virt*/T vtab_max
#define LOCAL_dict_htab_modify__ARGS(T)  , (T)vtab_min, (T)vtab_max
#define LOCAL_modify(htab_lvalue)                               \
	if ((htab_lvalue) >= vtab_min && (htab_lvalue) <= vtab_max) \
		--(htab_lvalue)
#elif defined(DEFINE_dict_htab_incrange)
#define LOCAL_dict_htab_modify           dict_htab_incrange
#define LOCAL_dict_htab_modify__PARAM(T) , /*virt*/T vtab_min, /*virt*/T vtab_max
#define LOCAL_dict_htab_modify__ARGS(T)  , (T)vtab_min, (T)vtab_max
#define LOCAL_modify(htab_lvalue)                               \
	if ((htab_lvalue) >= vtab_min && (htab_lvalue) <= vtab_max) \
		++(htab_lvalue)
#else /* ... */
#error "Invalid configuration"
#endif /* !... */

DECL_BEGIN

#define LOCAL_dict_htab_modify8  PP_CAT2(LOCAL_dict_htab_modify, 8)
#define LOCAL_dict_htab_modify16 PP_CAT2(LOCAL_dict_htab_modify, 16)
#define LOCAL_dict_htab_modify32 PP_CAT2(LOCAL_dict_htab_modify, 32)
#define LOCAL_dict_htab_modify64 PP_CAT2(LOCAL_dict_htab_modify, 64)

LOCAL ATTR_NOINLINE NONNULL((1)) void DCALL
LOCAL_dict_htab_modify8(Dict *__restrict self LOCAL_dict_htab_modify__PARAM(uint8_t)) {
	size_t i;
	uint8_t *htab = (uint8_t *)self->d_htab;
	for (i = 0; i <= self->d_hmask; ++i) {
		LOCAL_modify(htab[i]);
	}
}

#if DEE_DICT_HIDXIO_COUNT >= 2
LOCAL ATTR_NOINLINE NONNULL((1)) void DCALL
LOCAL_dict_htab_modify16(Dict *__restrict self LOCAL_dict_htab_modify__PARAM(uint16_t)) {
	size_t i;
	uint16_t *htab = (uint16_t *)self->d_htab;
	for (i = 0; i <= self->d_hmask; ++i) {
		LOCAL_modify(htab[i]);
	}
}
#endif /* DEE_DICT_HIDXIO_COUNT >= 2 */

#if DEE_DICT_HIDXIO_COUNT >= 3
LOCAL ATTR_NOINLINE NONNULL((1)) void DCALL
LOCAL_dict_htab_modify32(Dict *__restrict self LOCAL_dict_htab_modify__PARAM(uint32_t)) {
	size_t i;
	uint32_t *htab = (uint32_t *)self->d_htab;
	for (i = 0; i <= self->d_hmask; ++i) {
		LOCAL_modify(htab[i]);
	}
}
#endif /* DEE_DICT_HIDXIO_COUNT >= 3 */

#if DEE_DICT_HIDXIO_COUNT >= 4
LOCAL ATTR_NOINLINE NONNULL((1)) void DCALL
LOCAL_dict_htab_modify64(Dict *__restrict self LOCAL_dict_htab_modify__PARAM(uint64_t)) {
	size_t i;
	uint64_t *htab = (uint64_t *)self->d_htab;
	for (i = 0; i <= self->d_hmask; ++i) {
		LOCAL_modify(htab[i]);
	}
}
#endif /* DEE_DICT_HIDXIO_COUNT >= 4 */

LOCAL ATTR_NOINLINE NONNULL((1)) void DCALL
LOCAL_dict_htab_modify(Dict *__restrict self LOCAL_dict_htab_modify__PARAM(Dee_dict_vidx_t)) {
	if (DEE_DICT_HIDXIO_IS8(self->d_valloc)) {
		LOCAL_dict_htab_modify8(self LOCAL_dict_htab_modify__ARGS(uint8_t));
	} else
#if DEE_DICT_HIDXIO_COUNT >= 2
	if (DEE_DICT_HIDXIO_IS16(self->d_valloc)) {
		LOCAL_dict_htab_modify16(self LOCAL_dict_htab_modify__ARGS(uint16_t));
	} else
#endif /* DEE_DICT_HIDXIO_COUNT >= 2 */
#if DEE_DICT_HIDXIO_COUNT >= 3
	if (DEE_DICT_HIDXIO_IS32(self->d_valloc)) {
		LOCAL_dict_htab_modify32(self LOCAL_dict_htab_modify__ARGS(uint32_t));
	} else
#endif /* DEE_DICT_HIDXIO_COUNT >= 3 */
#if DEE_DICT_HIDXIO_COUNT >= 4
	if (DEE_DICT_HIDXIO_IS64(self->d_valloc)) {
		LOCAL_dict_htab_modify64(self LOCAL_dict_htab_modify__ARGS(uint64_t));
	} else
#endif /* DEE_DICT_HIDXIO_COUNT >= 4 */
	{
		__builtin_unreachable();
	}
}

#undef LOCAL_dict_htab_modify8
#undef LOCAL_dict_htab_modify16
#undef LOCAL_dict_htab_modify32
#undef LOCAL_dict_htab_modify64

DECL_END

#undef LOCAL_dict_htab_modify
#undef LOCAL_dict_htab_modify__PARAM
#undef LOCAL_dict_htab_modify__ARGS
#undef LOCAL_modify

#undef DEFINE_dict_htab_decafter
#undef DEFINE_dict_htab_incafter
#undef DEFINE_dict_htab_decrange
#undef DEFINE_dict_htab_incrange
