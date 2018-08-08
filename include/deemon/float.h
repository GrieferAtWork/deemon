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
#ifndef GUARD_DEEMON_FLOAT_H
#define GUARD_DEEMON_FLOAT_H 1

#include "api.h"
#include "object.h"

DECL_BEGIN

typedef struct float_object DeeFloatObject;

struct float_object {
    OBJECT_HEAD
    double      f_value; /* [const] The value of this float as a C-double. */
};
#define DEFINE_FLOAT(name,value) \
  DeeFloatObject name = { OBJECT_HEAD_INIT(&DeeFloat_Type), value }


#define DeeFloat_VALUE(x) ((DeeFloatObject *)REQUIRES_OBJECT(x))->f_value

#define DeeFloat_Check(x)      DeeObject_InstanceOfExact(x,&DeeFloat_Type) /* `float' is `final' */
#define DeeFloat_CheckExact(x) DeeObject_InstanceOfExact(x,&DeeFloat_Type)
DDATDEF DeeTypeObject DeeFloat_Type;

/* Create and return a new floating point object. */
DFUNDEF DREF DeeObject *DCALL DeeFloat_New(double value);


DECL_END

#endif /* !GUARD_DEEMON_FLOAT_H */
