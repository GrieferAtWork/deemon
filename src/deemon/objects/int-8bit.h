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
#ifndef GUARD_DEEMON_OBJECTS_INT_8BIT_H
#define GUARD_DEEMON_OBJECTS_INT_8BIT_H 1

#include <deemon/api.h>

#include <deemon/int.h>

/* Config option to statically pre-allocate all 8-bit integer constants (that is: [-128,255]). */
#if (!defined(CONFIG_STRING_8BIT_STATIC) && \
     !defined(CONFIG_STRING_8BIT_NORMAL))
#ifdef __OPTIMIZE_SIZE__
#define CONFIG_STRING_8BIT_NORMAL
#else /* __OPTIMIZE_SIZE__ */
#define CONFIG_STRING_8BIT_STATIC
#endif /* !__OPTIMIZE_SIZE__ */
#endif /* ... */


DECL_BEGIN

#undef DeeInt_8bit
#ifdef CONFIG_STRING_8BIT_STATIC
INTDEF struct _Dee_int_1digit_object DeeInt_8bit_blob[128 + 256];
#define DeeInt_8bit (DeeInt_8bit_blob + 128)
#endif /* CONFIG_STRING_8BIT_STATIC */

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_INT_8BIT_H */
