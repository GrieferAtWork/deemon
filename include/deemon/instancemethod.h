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
/*!export **/
/*!export DeeInstanceMethod_**/
#ifndef GUARD_DEEMON_INSTANCEMETHOD_H
#define GUARD_DEEMON_INSTANCEMETHOD_H 1 /*!export-*/

#include "api.h"

#include "types.h"

DECL_BEGIN

typedef struct Dee_instance_method {
	Dee_OBJECT_HEAD
	DREF DeeObject *im_func; /* [1..1] The function to-be called. */
	DREF DeeObject *im_this; /* [1..1] The this argument. */
} DeeInstanceMethodObject;

#define DeeInstanceMethod_FUNC(x) Dee_REQUIRES_OBJECT(DeeInstanceMethodObject, x)->im_func
#define DeeInstanceMethod_THIS(x) Dee_REQUIRES_OBJECT(DeeInstanceMethodObject, x)->im_this

DDATDEF DeeTypeObject DeeInstanceMethod_Type;
#define DeeInstanceMethod_Check(ob)      DeeObject_InstanceOf(ob, &DeeInstanceMethod_Type)
#define DeeInstanceMethod_CheckExact(ob) DeeObject_InstanceOfExact(ob, &DeeInstanceMethod_Type)

#define Dee_DEFINE_INSTANCEMETHOD(name, func, thisarg) \
	DeeInstanceMethodObject name = {                   \
		Dee_OBJECT_HEAD_INIT(&DeeInstanceMethod_Type), \
		Dee_AsObject(func),                            \
		Dee_AsObject(thisarg)                          \
	}

/* Create a new instance method.
 *
 * This is a simple wrapper object that simply invokes a thiscall on
 * `im_func', using `this_arg' as the this-argument when called normally.
 *
 * In user-code, it is used to implement the temporary/split type when an
 * instance attribute with the `Dee_CLASS_ATTRIBUTE_FMETHOD' flag is loaded
 * as an object, rather than being called directly. */
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeInstanceMethod_New(DeeObject *func, DeeObject *this_arg);
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeInstanceMethod_NewInherited(/*inherit(always)*/ DREF DeeObject *func,
                               /*inherit(always)*/ DREF DeeObject *this_arg);

DECL_END

#endif /* !GUARD_DEEMON_INSTANCEMETHOD_H */
