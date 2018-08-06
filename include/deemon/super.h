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
#ifndef GUARD_DEEMON_SUPER_H
#define GUARD_DEEMON_SUPER_H 1

#include "api.h"
#include "object.h"
#include <stddef.h>

DECL_BEGIN

typedef struct super_object DeeSuperObject;

struct super_object {
    /* WARNING: Changes must be mirrored in `/src/deemon/execute/asm/exec-386.S' */
    OBJECT_HEAD
    DREF DeeTypeObject *s_type; /* [1..1][const] Super-type.
                                 *   NOTE: This is never `&DeeSuper_Type' itself and the
                                 *         check `DeeObject_InstanceOf(s_self,s_type)'
                                 *         must always succeed. */
    DREF DeeObject     *s_self; /* [1..1][const] Wrapped object (Never another super-object). */
};

#define DeeSuper_TYPE(x) ((DeeSuperObject *)(x))->s_type
#define DeeSuper_SELF(x) ((DeeSuperObject *)(x))->s_self

DDATDEF DeeTypeObject DeeSuper_Type;
#define DeeSuper_Check(ob)      DeeObject_InstanceOfExact(ob,&DeeSuper_Type) /* `super' is `final' */
#define DeeSuper_CheckExact(ob) DeeObject_InstanceOfExact(ob,&DeeSuper_Type) /* `super' is `final' */

/* Create a new super-wrapper for `tp_self:self'.
 * NOTE: This function automatically checks the given operands for validity:
 *        - DeeType_Check(tp_self);
 *        - DeeObject_InstanceOf(self,tp_self);
 *       It also automatically unwraps `self' should it already be a super-object. */
DFUNDEF DREF DeeObject *DCALL DeeSuper_New(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self);

/* Taking some object, return the effective super-class of it.
 * HINT: When `self' is another super-object, this is identical to
 *      `DeeSuper_New(DeeType_BASE(DeeSuper_TYPE(self)),DeeSuper_SELF(self))'
 * WARNING: This function may perform non-shared
 *          optimizations by re-using `self' when allowed.
 * @throws: Error.TypeError: The class of `self' has no super-class. */
DFUNDEF DREF DeeObject *DCALL DeeSuper_Of(DeeObject *__restrict self);


DECL_END

#endif /* !GUARD_DEEMON_SUPER_H */
