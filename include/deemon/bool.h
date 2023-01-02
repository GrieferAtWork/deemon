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
#ifndef GUARD_DEEMON_BOOL_H
#define GUARD_DEEMON_BOOL_H 1

#include "api.h"

#include <stdbool.h>
#include <stddef.h>

#include "object.h"

DECL_BEGIN

#ifdef DEE_SOURCE
#define Dee_bool_object bool_object
#define return_bool   Dee_return_bool
#define return_bool_  Dee_return_bool_
#define return_true   Dee_return_true
#define return_false  Dee_return_false
#endif /* DEE_SOURCE */

typedef struct Dee_bool_object DeeBoolObject;

/* HINT: In i386 assembly, `bool' is 8 bytes, so if you want to
 *       convert an integer 0/1 into a boolean, you can use the
 *       following assembly:
 *    >> leal Dee_FalseTrue(,%reg,8), %reg
 *       The fact that this can be done is the reason why a boolean
 *       doesn't store its value in its structure, but rather in its
 *       self-address.
 * WARNING: Only possible when `CONFIG_TRACE_REFCHANGES' is disabled! */
struct Dee_bool_object {
	Dee_OBJECT_HEAD
};

#define DeeBool_Check(x)      DeeObject_InstanceOfExact(x, &DeeBool_Type) /* `bool' is final. */
#define DeeBool_CheckExact(x) DeeObject_InstanceOfExact(x, &DeeBool_Type)
#define DeeBool_IsTrue(x)     ((DeeBoolObject *)Dee_REQUIRES_OBJECT(x) != &Dee_FalseTrue[0])
#define DeeBool_For(val)      ((DeeObject *)&Dee_FalseTrue[!!(val)])
#define Dee_return_bool(val)  Dee_return_reference(DeeBool_For(val))
#define Dee_return_bool_(val) Dee_return_reference_(DeeBool_For(val))
#define Dee_return_true       Dee_return_reference_(Dee_True)
#define Dee_return_false      Dee_return_reference_(Dee_False)

DDATDEF DeeTypeObject DeeBool_Type;
DDATDEF DeeBoolObject Dee_FalseTrue[2];
#define Dee_False   ((DeeObject *)&Dee_FalseTrue[0])
#define Dee_True    ((DeeObject *)&Dee_FalseTrue[1])


DECL_END

#endif /* !GUARD_DEEMON_BOOL_H */
