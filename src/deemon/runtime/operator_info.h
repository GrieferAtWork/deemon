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
#ifndef GUARD_DEEMON_RUNTIME_OPERATOR_INFO_H
#define GUARD_DEEMON_RUNTIME_OPERATOR_INFO_H 1

#include <deemon/api.h>

#include <deemon/object.h> /* DREF, DeeObject, DeeTypeObject */
#include <deemon/type.h>   /* Dee_operator_t */

#include "method-hint-defaults.h"

#include <stdbool.h> /* bool */

DECL_BEGIN

typedef struct {
	PROXY_OBJECT_HEAD_EX(DeeTypeObject, toi_type); /* [1..1][const] The type who's operators should be enumerated. */
	Dee_operator_t                      toi_opid;  /* [lock(ATOMIC)] Next operator ID to check. */
	bool                                toi_name;  /* [const] When true, try to assign human-readable names to operators. */
} TypeOperatorsIterator;

typedef struct {
	PROXY_OBJECT_HEAD_EX(DeeTypeObject, to_type); /* [1..1][const] The type who's operators should be enumerated. */
	bool                                to_name;  /* [const] When true, try to assign human-readable names to operators. */
} TypeOperators;

INTDEF DeeTypeObject TypeOperators_Type;
INTDEF DeeTypeObject TypeOperatorsIterator_Type;

INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL type_get_operators(DeeTypeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL type_get_operatorids(DeeTypeObject *__restrict self);

DECL_END

#endif /* !GUARD_DEEMON_RUNTIME_OPERATOR_INFO_H */
