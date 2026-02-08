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
#include "int.c"
//#define DEFINE_int_get_signed8
//#define DEFINE_int_get_signed16
//#define DEFINE_int_get_signed32
//#define DEFINE_int_get_signed64
//#define DEFINE_int_get_signed128
//#define DEFINE_int_get_unsigned8
//#define DEFINE_int_get_unsigned16
//#define DEFINE_int_get_unsigned32
#define DEFINE_int_get_unsigned64
//#define DEFINE_int_get_unsigned128
#endif /* __INTELLISENSE__ */

#include <deemon/api.h>

#include <deemon/int.h>   /* DeeIntObject, DeeInt_*, Dee_DIGIT_BITS */
#include <deemon/types.h> /* DREF, DeeObject, Dee_ssize_t */

#include <hybrid/align.h> /* CEILDIV */

#if (defined(DEFINE_int_get_signed8) +    \
     defined(DEFINE_int_get_signed16) +   \
     defined(DEFINE_int_get_signed32) +   \
     defined(DEFINE_int_get_signed64) +   \
     defined(DEFINE_int_get_signed128) +  \
     defined(DEFINE_int_get_unsigned8) +  \
     defined(DEFINE_int_get_unsigned16) + \
     defined(DEFINE_int_get_unsigned32) + \
     defined(DEFINE_int_get_unsigned64) + \
     defined(DEFINE_int_get_unsigned128)) != 1
#error "Must #define exactly one of these macros"
#endif /* ... */

DECL_BEGIN

#ifdef DEFINE_int_get_signed8
#define LOCAL_int_get_fixedbit int_get_signed8
#define LOCAL_BITCOUNT         8
#define LOCAL_IS_SIGNED
#elif defined(DEFINE_int_get_signed16)
#define LOCAL_int_get_fixedbit int_get_signed16
#define LOCAL_BITCOUNT         16
#define LOCAL_IS_SIGNED
#elif defined(DEFINE_int_get_signed32)
#define LOCAL_int_get_fixedbit int_get_signed32
#define LOCAL_BITCOUNT         32
#define LOCAL_IS_SIGNED
#elif defined(DEFINE_int_get_signed64)
#define LOCAL_int_get_fixedbit int_get_signed64
#define LOCAL_BITCOUNT         64
#define LOCAL_IS_SIGNED
#elif defined(DEFINE_int_get_signed128)
#define LOCAL_int_get_fixedbit int_get_signed128
#define LOCAL_BITCOUNT         128
#define LOCAL_IS_SIGNED
#elif defined(DEFINE_int_get_unsigned8)
#define LOCAL_int_get_fixedbit int_get_unsigned8
#define LOCAL_BITCOUNT         8
#elif defined(DEFINE_int_get_unsigned16)
#define LOCAL_int_get_fixedbit int_get_unsigned16
#define LOCAL_BITCOUNT         16
#elif defined(DEFINE_int_get_unsigned32)
#define LOCAL_int_get_fixedbit int_get_unsigned32
#define LOCAL_BITCOUNT         32
#elif defined(DEFINE_int_get_unsigned64)
#define LOCAL_int_get_fixedbit int_get_unsigned64
#define LOCAL_BITCOUNT         64
#elif defined(DEFINE_int_get_unsigned128)
#define LOCAL_int_get_fixedbit int_get_unsigned128
#define LOCAL_BITCOUNT         128
#else /* ... */
#error "Invalid configuration"
#endif /* !... */


#define LOCAL_intN_t         PP_CAT3(int, LOCAL_BITCOUNT, _t)
#define LOCAL_uintN_t        PP_CAT3(uint, LOCAL_BITCOUNT, _t)
#define LOCAL_DeeInt_NewUInt PP_CAT2(DeeInt_NewUInt, LOCAL_BITCOUNT)
#define LOCAL_DeeInt_NewInt  PP_CAT2(DeeInt_NewInt, LOCAL_BITCOUNT)
#define LOCAL_DIGITS         CEILDIV(LOCAL_BITCOUNT, Dee_DIGIT_BITS)

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
LOCAL_int_get_fixedbit(DeeIntObject *__restrict self) {
#if LOCAL_BITCOUNT >= 128
	/* TODO */

#else /* LOCAL_BITCOUNT >= 128 */
	LOCAL_uintN_t value;
	Dee_ssize_t size = self->ob_size;
	if (size > LOCAL_DIGITS) {
		size = LOCAL_DIGITS;
	} else if (size < -LOCAL_DIGITS) {
		size = -LOCAL_DIGITS;
	} else {
		/* TODO: Check if "self" fits mandated value-range (if so: just re-return "self" as-is) */
	}
	switch (size) {
	case 0:
		return DeeInt_NewZero();
	case 1:
		value = (LOCAL_uintN_t)self->ob_digit[0];
		break;
	case -1:
		value = (LOCAL_uintN_t)self->ob_digit[0];
		value = ~(value - 1);
		break;
#if LOCAL_DIGITS >= 2
	case 2:
		value = (LOCAL_uintN_t)(self->ob_digit[0]);
		value |= (LOCAL_uintN_t)self->ob_digit[1] << (1 * Dee_DIGIT_BITS);
		break;
	case -2:
		value = (LOCAL_uintN_t)(self->ob_digit[0]);
		value |= (LOCAL_uintN_t)self->ob_digit[1] << (1 * Dee_DIGIT_BITS);
		value = ~(value - 1);
		break;
#if LOCAL_DIGITS >= 3
	case 3:
		value = (LOCAL_uintN_t)(self->ob_digit[0]);
		value |= (LOCAL_uintN_t)self->ob_digit[1] << (1 * Dee_DIGIT_BITS);
		value |= (LOCAL_uintN_t)self->ob_digit[2] << (2 * Dee_DIGIT_BITS);
		break;
	case -3:
		value = (LOCAL_uintN_t)(self->ob_digit[0]);
		value |= (LOCAL_uintN_t)self->ob_digit[1] << (1 * Dee_DIGIT_BITS);
		value |= (LOCAL_uintN_t)self->ob_digit[2] << (2 * Dee_DIGIT_BITS);
		value = ~(value - 1);
		break;
#if LOCAL_DIGITS >= 4
	case 4:
		value = (LOCAL_uintN_t)(self->ob_digit[0]);
		value |= (LOCAL_uintN_t)self->ob_digit[1] << (1 * Dee_DIGIT_BITS);
		value |= (LOCAL_uintN_t)self->ob_digit[2] << (2 * Dee_DIGIT_BITS);
		value |= (LOCAL_uintN_t)self->ob_digit[3] << (3 * Dee_DIGIT_BITS);
		break;
	case -4:
		value = (LOCAL_uintN_t)(self->ob_digit[0]);
		value |= (LOCAL_uintN_t)self->ob_digit[1] << (1 * Dee_DIGIT_BITS);
		value |= (LOCAL_uintN_t)self->ob_digit[2] << (2 * Dee_DIGIT_BITS);
		value |= (LOCAL_uintN_t)self->ob_digit[3] << (3 * Dee_DIGIT_BITS);
		value = ~(value - 1);
		break;
#if LOCAL_DIGITS >= 5
	case 5:
		value = (LOCAL_uintN_t)(self->ob_digit[0]);
		value |= (LOCAL_uintN_t)self->ob_digit[1] << (1 * Dee_DIGIT_BITS);
		value |= (LOCAL_uintN_t)self->ob_digit[2] << (2 * Dee_DIGIT_BITS);
		value |= (LOCAL_uintN_t)self->ob_digit[3] << (3 * Dee_DIGIT_BITS);
		value |= (LOCAL_uintN_t)self->ob_digit[4] << (4 * Dee_DIGIT_BITS);
		break;
	case -5:
		value = (LOCAL_uintN_t)(self->ob_digit[0]);
		value |= (LOCAL_uintN_t)self->ob_digit[1] << (1 * Dee_DIGIT_BITS);
		value |= (LOCAL_uintN_t)self->ob_digit[2] << (2 * Dee_DIGIT_BITS);
		value |= (LOCAL_uintN_t)self->ob_digit[3] << (3 * Dee_DIGIT_BITS);
		value |= (LOCAL_uintN_t)self->ob_digit[4] << (4 * Dee_DIGIT_BITS);
		value = ~(value - 1);
		break;
#if LOCAL_DIGITS >= 6
#error "Too many digits"
#endif /* LOCAL_DIGITS >= 6 */
#endif /* LOCAL_DIGITS >= 5 */
#endif /* LOCAL_DIGITS >= 4 */
#endif /* LOCAL_DIGITS >= 3 */
#endif /* LOCAL_DIGITS >= 2 */
	default: __builtin_unreachable();
	}
#ifdef LOCAL_IS_SIGNED
	return LOCAL_DeeInt_NewInt((LOCAL_intN_t)value);
#else /* LOCAL_IS_SIGNED */
	return LOCAL_DeeInt_NewUInt(value);
#endif /* !LOCAL_IS_SIGNED */
#endif /* LOCAL_BITCOUNT < 128 */
}

#undef LOCAL_intN_t
#undef LOCAL_uintN_t
#undef LOCAL_DeeInt_NewUInt
#undef LOCAL_DeeInt_NewInt
#undef LOCAL_DIGITS
#undef LOCAL_int_get_fixedbit
#undef LOCAL_BITCOUNT
#undef LOCAL_IS_SIGNED

DECL_END

#undef DEFINE_int_get_signed8
#undef DEFINE_int_get_signed16
#undef DEFINE_int_get_signed32
#undef DEFINE_int_get_signed64
#undef DEFINE_int_get_signed128
#undef DEFINE_int_get_unsigned8
#undef DEFINE_int_get_unsigned16
#undef DEFINE_int_get_unsigned32
#undef DEFINE_int_get_unsigned64
#undef DEFINE_int_get_unsigned128
