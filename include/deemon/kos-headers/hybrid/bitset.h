/* Copyright (c) 2018 Griefer@Work                                            *
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
#ifndef __GUARD_HYBRID_BITSET_H
#define __GUARD_HYBRID_BITSET_H 1

#include "compiler.h"
#include "types.h"
#include "atomic.h"

DECL_BEGIN

#define BIT_MASK(i)          (1 << ((i) % 8))
#define BIT_BYTE(set,i)      ((byte_t *)(set))[(i)/8]
#define BIT_GT(set,i)        (BIT_BYTE(set,i) & BIT_MASK(i))
#define BIT_ST(set,i)        (BIT_BYTE(set,i) |= BIT_MASK(i))
#define BIT_CL(set,i)        (BIT_BYTE(set,i) &= ~BIT_MASK(i))

/* Atomically set/clear, returning ZERO(0) if it wasn't set before, or non-ZERO(0) if it was. */
#define BIT_ATOMIC_ST(set,i) XBLOCK({ byte_t const _mask = BIT_MASK(i); XRETURN ATOMIC_FETCHOR(BIT_BYTE(set,i),_mask)&_mask; })
#define BIT_ATOMIC_CL(set,i) XBLOCK({ byte_t const _mask = BIT_MASK(i); XRETURN ATOMIC_FETCHAND(BIT_BYTE(set,i),~_mask)&_mask; })

DECL_END

#endif /* !__GUARD_HYBRID_BITSET_H */
