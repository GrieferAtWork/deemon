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
#ifndef GUARD_DEEMON_OBJECTS_SEQ_BYATTR_H
#define GUARD_DEEMON_OBJECTS_SEQ_BYATTR_H 1

#include <deemon/api.h>

#include <deemon/object.h> /* DREF, DeeObject, DeeTypeObject, Dee_AsObject */

#include "../generic-proxy.h"

DECL_BEGIN

typedef struct {
	PROXY_OBJECT_HEAD(mba_map)  /* [1..1][const] The underlying map. */
} MapByAttr;

#define MapByAttr_New(ob)                   ((DREF MapByAttr *)ProxyObject_New(&MapByAttr_Type, Dee_AsObject(ob)))
#define MapByAttr_NewInherited(ob)          ((DREF MapByAttr *)ProxyObject_NewInherited(&MapByAttr_Type, Dee_AsObject(ob)))
#define MapByAttr_NewInheritedOnSuccess(ob) ((DREF MapByAttr *)ProxyObject_NewInheritedOnSuccess(&MapByAttr_Type, Dee_AsObject(ob)))

INTDEF DeeTypeObject MapByAttr_Type;

/* Create a new byattr proxy for `map' */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
MapByAttr_Of(DeeObject *__restrict map);

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_BYATTR_H */
