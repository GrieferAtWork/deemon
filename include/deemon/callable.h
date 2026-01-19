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
#ifndef GUARD_DEEMON_CALLABLE_H
#define GUARD_DEEMON_CALLABLE_H 1

#include "api.h"

#include "types.h"

DECL_BEGIN

/* `Callable from deemon'
 *
 * Base class for callable wrapper types, such as ObjMethod, CMethod,
 * InstanceMethod or just a plain old function. There is no particular
 * reason why this exists, other than to allow user-code to query
 * for a type for that particular set of objects by simply writing
 * `x is Callable from deemon' */
DDATDEF DeeTypeObject DeeCallable_Type;
#define DeeCallable_Check(ob) DeeObject_Implements(ob, &DeeCallable_Type)

DECL_END

#endif /* !GUARD_DEEMON_CALLABLE_H */
