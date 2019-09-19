/* Copyright (c) 2019 Griefer@Work                                            *
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
#ifndef GUARD_DEEMON_WEAKREF_H
#define GUARD_DEEMON_WEAKREF_H 1

#include "api.h"

#include "object.h"

DECL_BEGIN

#ifdef DEE_SOURCE
#define Dee_weakref_object     weakref_object
#define Dee_weakrefable_object weakrefable_object
#endif /* DEE_SOURCE */

typedef struct Dee_weakref_object DeeWeakRefObject;
typedef struct Dee_weakrefable_object DeeWeakRefAbleObject;

struct Dee_weakref_object {
	Dee_OBJECT_HEAD
	struct Dee_weakref wr_ref; /* The weak reference pointer. */
	DREF DeeObject    *wr_del; /* [0..1][const] Deletion callback. */
};

#define DeeWeakRef_Check(self)      DeeObject_InstanceOfExact(self, &DeeWeakRef_Type) /* `WeakRef' is final */
#define DeeWeakRef_CheckExact(self) DeeObject_InstanceOfExact(self, &DeeWeakRef_Type)
DDATDEF DeeTypeObject DeeWeakRef_Type;


/* Base class that should be used for user-defined classes
 * that are supposed to support being weakly referenced. */
struct Dee_weakrefable_object {
	Dee_OBJECT_HEAD
	Dee_WEAKREF_SUPPORT
};

DDATDEF DeeTypeObject DeeWeakRefAble_Type;

DECL_END

#endif /* !GUARD_DEEMON_WEAKREF_H */
