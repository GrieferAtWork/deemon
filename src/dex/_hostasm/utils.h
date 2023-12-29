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
#ifndef GUARD_DEX_HOSTASM_UTILS_H
#define GUARD_DEX_HOSTASM_UTILS_H 1

#include "host.h"
#include <hybrid/typecore.h>
/**/

#ifdef CONFIG_HAVE_LIBHOSTASM
DECL_BEGIN

typedef byte_t bitset_t;
#define _BITSET_BITS                  __CHAR_BIT__
#define _bitset_byte(i)               ((i) / _BITSET_BITS)
#define _bitset_mask(i)               ((bitset_t)1 << ((i) % _BITSET_BITS))
#define _bitset_sizeof(n_bits)        (((n_bits) + _BITSET_BITS - 1) / _BITSET_BITS)
#define bitset_test(self, i)          ((self)[_bitset_byte(i)] & _bitset_mask(i))
#define bitset_set(self, i)           (void)((self)[_bitset_byte(i)] |= _bitset_mask(i))
#define bitset_clear(self, i)         (void)((self)[_bitset_byte(i)] &= ~_bitset_mask(i))
#define bitset_clearall(self, n_bits) bzero(self, _bitset_sizeof(n_bits))

DECL_END
#endif /* CONFIG_HAVE_LIBHOSTASM */

#endif /* !GUARD_DEX_HOSTASM_UTILS_H */
