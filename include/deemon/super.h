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
#ifndef GUARD_DEEMON_SUPER_H
#define GUARD_DEEMON_SUPER_H 1

#include "api.h"

#include <stddef.h>

#include "object.h"

DECL_BEGIN

#ifdef DEE_SOURCE
#define Dee_super_object  super_object
#endif /* DEE_SOURCE */

typedef struct Dee_super_object DeeSuperObject;

struct Dee_super_object {
	/* WARNING: Changes must be mirrored in `/src/deemon/execute/asm/exec.gas-386.S' */
	Dee_OBJECT_HEAD
	DREF DeeTypeObject *s_type; /* [1..1][const] Super-type.
	                             * NOTE: This is never `&DeeSuper_Type' itself and the
	                             *       check `DeeObject_InstanceOf(s_self, s_type)'
	                             *       must always succeed. */
	DREF DeeObject     *s_self; /* [1..1][const] Wrapped object (Never another super-object). */
};

#define DeeSuper_TYPE(x) ((DeeSuperObject *)Dee_REQUIRES_OBJECT(x))->s_type
#define DeeSuper_SELF(x) ((DeeSuperObject *)Dee_REQUIRES_OBJECT(x))->s_self

DDATDEF DeeTypeObject DeeSuper_Type;
#define DeeSuper_Check(ob)      DeeObject_InstanceOfExact(ob, &DeeSuper_Type) /* `super' is final */
#define DeeSuper_CheckExact(ob) DeeObject_InstanceOfExact(ob, &DeeSuper_Type)

/* Create a new super-wrapper for `tp_self:self'.
 * NOTE: This function automatically checks the given operands for validity:
 *        - DeeType_Check(tp_self);
 *        - DeeObject_InstanceOf(self, tp_self);
 * It also automatically unwraps `self' should it already be a super-object. */
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeSuper_New(DeeTypeObject *tp_self, DeeObject *self);

/* Taking some object, return the effective super-class of it.
 * HINT: When `self' is another super-object, this is identical to
 *       `DeeSuper_New(DeeType_BASE(DeeSuper_TYPE(self)), DeeSuper_SELF(self))'
 * @throws: Error.TypeError: The class of `self' has no super-class. */
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeSuper_Of(DeeObject *__restrict self);


DECL_END

#endif /* !GUARD_DEEMON_SUPER_H */
