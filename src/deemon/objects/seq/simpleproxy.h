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
#ifndef GUARD_DEEMON_OBJECTS_SEQ_SIMPLEPROXY_H
#define GUARD_DEEMON_OBJECTS_SEQ_SIMPLEPROXY_H 1

#include <deemon/api.h>

#include <deemon/object.h> /* DREF, DeeObject, DeeTypeObject, Dee_AsObject */

#include "../generic-proxy.h"

DECL_BEGIN

typedef struct {
	PROXY_OBJECT_HEAD(sp_seq) /* [1..1][const] The underlying sequence. */
} SeqSimpleProxy;

#define SeqSimpleProxy_New(type, seq)                   ((DREF SeqSimpleProxy *)ProxyObject_New(type, Dee_AsObject(seq)))
#define SeqSimpleProxy_NewInherited(type, seq)          ((DREF SeqSimpleProxy *)ProxyObject_NewInherited(type, Dee_AsObject(seq)))
#define SeqSimpleProxy_NewInheritedOnSuccess(type, seq) ((DREF SeqSimpleProxy *)ProxyObject_NewInheritedOnSuccess(type, Dee_AsObject(seq)))
#define SeqSimpleProxy_DecrefSymbolic(self)             ProxyObject_DecrefSymbolic((DREF ProxyObject *)Dee_REQUIRES_TYPE(DREF SeqSimpleProxy *, self))

INTDEF DeeTypeObject SeqIds_Type;             /* Sequence.Ids */
INTDEF DeeTypeObject SeqTypes_Type;           /* Sequence.Types */
INTDEF DeeTypeObject SeqClasses_Type;         /* Sequence.Classes */

#define SeqIds_New(seq)                       SeqSimpleProxy_New(&SeqIds_Type, seq)
#define SeqIds_NewInherited(seq)              SeqSimpleProxy_NewInherited(&SeqIds_Type, seq)
#define SeqIds_NewInheritedOnSuccess(seq)     SeqSimpleProxy_NewInheritedOnSuccess(&SeqIds_Type, seq)
#define SeqIds_DecrefSymbolic(self)           SeqSimpleProxy_DecrefSymbolic(self)
#define SeqTypes_New(seq)                     SeqSimpleProxy_New(&SeqTypes_Type, seq)
#define SeqTypes_NewInherited(seq)            SeqSimpleProxy_NewInherited(&SeqTypes_Type, seq)
#define SeqTypes_NewInheritedOnSuccess(seq)   SeqSimpleProxy_NewInheritedOnSuccess(&SeqTypes_Type, seq)
#define SeqTypes_DecrefSymbolic(self)         SeqSimpleProxy_DecrefSymbolic(self)
#define SeqClasses_New(seq)                   SeqSimpleProxy_New(&SeqClasses_Type, seq)
#define SeqClasses_NewInherited(seq)          SeqSimpleProxy_NewInherited(&SeqClasses_Type, seq)
#define SeqClasses_NewInheritedOnSuccess(seq) SeqSimpleProxy_NewInheritedOnSuccess(&SeqClasses_Type, seq)
#define SeqClasses_DecrefSymbolic(self)       SeqSimpleProxy_DecrefSymbolic(self)


typedef struct {
	PROXY_OBJECT_HEAD(si_iter) /* [1..1][const] The underlying iterator. */
} SeqSimpleProxyIterator;

#define SeqSimpleProxyIterator_New(type, seq)                   ((DREF SeqSimpleProxyIterator *)ProxyObject_New(type, Dee_AsObject(seq)))
#define SeqSimpleProxyIterator_NewInherited(type, seq)          ((DREF SeqSimpleProxyIterator *)ProxyObject_NewInherited(type, Dee_AsObject(seq)))
#define SeqSimpleProxyIterator_NewInheritedOnSuccess(type, seq) ((DREF SeqSimpleProxyIterator *)ProxyObject_NewInheritedOnSuccess(type, Dee_AsObject(seq)))
#define SeqSimpleProxyIterator_DecrefSymbolic(self)             ProxyObject_DecrefSymbolic((DREF ProxyObject *)Dee_REQUIRES_TYPE(DREF SeqSimpleProxyIterator *, self))

INTDEF DeeTypeObject SeqIdsIterator_Type;     /* Sequence.Ids.Iterator */
INTDEF DeeTypeObject SeqTypesIterator_Type;   /* Sequence.Types.Iterator */
INTDEF DeeTypeObject SeqClassesIterator_Type; /* Sequence.Classes.Iterator */

#define SeqIdsIterator_New(seq)                       SeqSimpleProxyIterator_New(&SeqIdsIterator_Type, seq)
#define SeqIdsIterator_NewInherited(seq)              SeqSimpleProxyIterator_NewInherited(&SeqIdsIterator_Type, seq)
#define SeqIdsIterator_NewInheritedOnSuccess(seq)     SeqSimpleProxyIterator_NewInheritedOnSuccess(&SeqIdsIterator_Type, seq)
#define SeqIdsIterator_DecrefSymbolic(self)           SeqSimpleProxyIterator_DecrefSymbolic(self)
#define SeqTypesIterator_New(seq)                     SeqSimpleProxyIterator_New(&SeqTypesIterator_Type, seq)
#define SeqTypesIterator_NewInherited(seq)            SeqSimpleProxyIterator_NewInherited(&SeqTypesIterator_Type, seq)
#define SeqTypesIterator_NewInheritedOnSuccess(seq)   SeqSimpleProxyIterator_NewInheritedOnSuccess(&SeqTypesIterator_Type, seq)
#define SeqTypesIterator_DecrefSymbolic(self)         SeqSimpleProxyIterator_DecrefSymbolic(self)
#define SeqClassesIterator_New(seq)                   SeqSimpleProxyIterator_New(&SeqClassesIterator_Type, seq)
#define SeqClassesIterator_NewInherited(seq)          SeqSimpleProxyIterator_NewInherited(&SeqClassesIterator_Type, seq)
#define SeqClassesIterator_NewInheritedOnSuccess(seq) SeqSimpleProxyIterator_NewInheritedOnSuccess(&SeqClassesIterator_Type, seq)
#define SeqClassesIterator_DecrefSymbolic(self)       SeqSimpleProxyIterator_DecrefSymbolic(self)

INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL SeqIds_Of(DeeObject *__restrict seq);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL SeqTypes_Of(DeeObject *__restrict seq);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL SeqClasses_Of(DeeObject *__restrict seq);


DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_SIMPLEPROXY_H */
