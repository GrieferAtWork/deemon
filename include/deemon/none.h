/* Copyright (c) 2018-2022 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2022 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_NONE_H
#define GUARD_DEEMON_NONE_H 1

#include "api.h"

#include <stddef.h>

#include "object.h"

DECL_BEGIN

#ifdef DEE_SOURCE
#define Dee_none_object none_object
#define return_none     Dee_return_none
#endif /* DEE_SOURCE */

typedef struct Dee_none_object DeeNoneObject;
struct Dee_none_object {
	Dee_OBJECT_HEAD
	Dee_WEAKREF_SUPPORT
};

DDATDEF DeeTypeObject DeeNone_Type;
#ifdef GUARD_DEEMON_OBJECTS_NONE_C
DDATDEF DeeNoneObject DeeNone_Singleton;
#define Dee_None    ((DeeObject *)&DeeNone_Singleton)
#else /* GUARD_DEEMON_OBJECTS_NONE_C */
DDATDEF DeeObject     DeeNone_Singleton;
#define Dee_None    (&DeeNone_Singleton)
#endif /* !GUARD_DEEMON_OBJECTS_NONE_C */
#define Dee_return_none  Dee_return_reference_(Dee_None)

/* WARNING: If these two macros are ever changed, make sure to allow
 *         `NULL' to be passed for `x', as code exists that assumes
 *          this being possible (btw: `NULL' should evaluate to `false') */
#define DeeNone_Check(x)      ((DeeObject *)Dee_REQUIRES_OBJECT(x) == Dee_None) /* `none' is a singleton. */
#define DeeNone_CheckExact(x) ((DeeObject *)Dee_REQUIRES_OBJECT(x) == Dee_None)


DECL_END

#endif /* !GUARD_DEEMON_NONE_H */
