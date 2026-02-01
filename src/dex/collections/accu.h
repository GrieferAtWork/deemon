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
#ifndef GUARD_DEX_COLLECTIONS_ACCU_H
#define GUARD_DEX_COLLECTIONS_ACCU_H 1

#include "libcollections.h"
/**/

#include <deemon/api.h>

#include <deemon/accu.h>        /* Dee_accu */
#include <deemon/object.h>      /* DeeTypeObject, Dee_OBJECT_HEAD */
#include <deemon/util/nrlock.h> /* Dee_nrshared_lock_t */

DECL_BEGIN

typedef struct {
	Dee_OBJECT_HEAD
	struct Dee_accu     a_accu; /* [lock(a_lock)] Accumulator */
	Dee_nrshared_lock_t a_lock; /* Lock for accessing `a_accu' (non-recursive; throws error on reentrancy) */
} AccuObject;

INTDEF DeeTypeObject Accu_Type;

DECL_END

#endif /* !GUARD_DEX_COLLECTIONS_ACCU_H */
