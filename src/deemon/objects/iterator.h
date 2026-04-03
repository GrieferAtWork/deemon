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
#ifndef GUARD_DEEMON_OBJECTS_ITERATOR_H
#define GUARD_DEEMON_OBJECTS_ITERATOR_H 1

#include <deemon/api.h>

#include <deemon/object.h> /* DREF, DeeObject, DeeTypeObject, Dee_AsObject */

#include "generic-proxy.h"


DECL_BEGIN

typedef struct {
	PROXY_OBJECT_HEAD(if_iter) /* [1..1][const] The iterator who's future is viewed. */
} IteratorFuture;

#define IteratorFuture_New(ob)                   ((DREF IteratorFuture *)ProxyObject_New(&IteratorFuture_Type, Dee_AsObject(ob)))
#define IteratorFuture_NewInherited(ob)          ((DREF IteratorFuture *)ProxyObject_NewInherited(&IteratorFuture_Type, Dee_AsObject(ob)))
#define IteratorFuture_NewInheritedOnSuccess(ob) ((DREF IteratorFuture *)ProxyObject_NewInheritedOnSuccess(&IteratorFuture_Type, Dee_AsObject(ob)))

INTDEF DeeTypeObject IteratorFuture_Type;
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL IteratorFuture_Of(DeeObject *__restrict self);



typedef struct {
	PROXY_OBJECT_HEAD(ip_iter) /* [1..1][const] The iterator who's remainder is viewed. */
} IteratorPending;

#define IteratorPending_New(ob)                   ((DREF IteratorPending *)ProxyObject_New(&IteratorPending_Type, Dee_AsObject(ob)))
#define IteratorPending_NewInherited(ob)          ((DREF IteratorPending *)ProxyObject_NewInherited(&IteratorPending_Type, Dee_AsObject(ob)))
#define IteratorPending_NewInheritedOnSuccess(ob) ((DREF IteratorPending *)ProxyObject_NewInheritedOnSuccess(&IteratorPending_Type, Dee_AsObject(ob)))

INTDEF DeeTypeObject IteratorPending_Type;
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL IteratorPending_For(DeeObject *__restrict self);

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_ITERATOR_H */
