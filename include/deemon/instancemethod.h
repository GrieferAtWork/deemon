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
#ifndef GUARD_DEEMON_INSTANCEMETHOD_H
#define GUARD_DEEMON_INSTANCEMETHOD_H 1

#include "api.h"
#include "object.h"

DECL_BEGIN

typedef struct instance_method DeeInstanceMethodObject;

struct instance_method {
    OBJECT_HEAD
    DREF DeeObject *im_func; /* [1..1] The function to-be called. */
    DREF DeeObject *im_self; /* [1..1] The this argument. */
};

DDATDEF DeeTypeObject DeeInstanceMethod_Type;

/* Create a new instance method.
 * This is a simple wrapper object that simply invokes a thiscall on
 * `im_func', using `this_arg' as the this-argument when called normally.
 * In user-code, it is used to implement the temporary/split type when
 * an instance attribute with the `CLASS_ATTRIBUTE_FMETHOD' flag is loaded
 * as an object, rather than being called directly. */
DFUNDEF DREF DeeObject *DCALL
DeeInstanceMethod_New(DeeObject *__restrict func,
                      DeeObject *__restrict this_arg);

DECL_END

#endif /* !GUARD_DEEMON_INSTANCEMETHOD_H */
