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
#ifndef GUARD_DEEMON_INSTANCEMETHOD_H
#define GUARD_DEEMON_INSTANCEMETHOD_H 1

#include "api.h"
/**/

#include "types.h"

DECL_BEGIN

#ifdef DEE_SOURCE
#define Dee_instance_method    instance_method
#define DEFINE_INSTANCEMETHOD  Dee_DEFINE_INSTANCEMETHOD
#endif /* DEE_SOURCE */

typedef struct Dee_instance_method DeeInstanceMethodObject;

struct Dee_instance_method {
	Dee_OBJECT_HEAD
	DREF DeeObject *im_func; /* [1..1] The function to-be called. */
	DREF DeeObject *im_this; /* [1..1] The this argument. */
};
#define DeeInstanceMethod_FUNC(x) ((DeeInstanceMethodObject *)Dee_REQUIRES_OBJECT(x))->im_func
#define DeeInstanceMethod_THIS(x) ((DeeInstanceMethodObject *)Dee_REQUIRES_OBJECT(x))->im_this

DDATDEF DeeTypeObject DeeInstanceMethod_Type;
#define DeeInstanceMethod_Check(ob)      DeeObject_InstanceOf(ob, &DeeInstanceMethod_Type)
#define DeeInstanceMethod_CheckExact(ob) DeeObject_InstanceOfExact(ob, &DeeInstanceMethod_Type)

#define Dee_DEFINE_INSTANCEMETHOD(name, func, thisarg) \
	DeeInstanceMethodObject name = {                   \
		Dee_OBJECT_HEAD_INIT(&DeeInstanceMethod_Type), \
		(DREF DeeObject *)(func),                      \
		(DREF DeeObject *)(thisarg)                    \
	}

/* Create a new instance method.
 * This is a simple wrapper object that simply invokes a thiscall on
 * `im_func', using `this_arg' as the this-argument when called normally.
 * In user-code, it is used to implement the temporary/split type when
 * an instance attribute with the `CLASS_ATTRIBUTE_FMETHOD' flag is loaded
 * as an object, rather than being called directly. */
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeInstanceMethod_New(DeeObject *func, DeeObject *this_arg);

DECL_END

#endif /* !GUARD_DEEMON_INSTANCEMETHOD_H */
