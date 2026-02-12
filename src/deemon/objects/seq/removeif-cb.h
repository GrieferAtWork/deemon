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
#ifndef GUARD_DEEMON_OBJECTS_SEQ_REMOVEIF_CB_H
#define GUARD_DEEMON_OBJECTS_SEQ_REMOVEIF_CB_H 1

#include <deemon/api.h>

#include <deemon/object.h> /* DeeObject, DeeTypeObject */

#include "../generic-proxy.h"

DECL_BEGIN

typedef struct {
	PROXY_OBJECT_HEAD(srwrip_item); /* [1..1][const] Item to remove */
} SeqRemoveWithRemoveIfPredicate;

#define SeqRemoveWithRemoveIfPredicate_New(ob)                   ((DREF SeqRemoveWithRemoveIfPredicate *)ProxyObject_New(&SeqRemoveWithRemoveIfPredicate_Type, Dee_AsObject(ob)))
#define SeqRemoveWithRemoveIfPredicate_NewInherited(ob)          ((DREF SeqRemoveWithRemoveIfPredicate *)ProxyObject_NewInherited(&SeqRemoveWithRemoveIfPredicate_Type, Dee_AsObject(ob)))
#define SeqRemoveWithRemoveIfPredicate_NewInheritedOnSuccess(ob) ((DREF SeqRemoveWithRemoveIfPredicate *)ProxyObject_NewInheritedOnSuccess(&SeqRemoveWithRemoveIfPredicate_Type, Dee_AsObject(ob)))
#define SeqRemoveWithRemoveIfPredicate_DecrefSymbolic(self)      ProxyObject_DecrefSymbolic((DREF ProxyObject *)Dee_REQUIRES_TYPE(DREF SeqRemoveWithRemoveIfPredicate *, self))

typedef struct {
	PROXY_OBJECT_HEAD2(srwripwk_item, /* [1..1][const] Keyed item to remove */
	                   srwripwk_key)  /* [1..1][const] Key to use during compare */
} SeqRemoveWithRemoveIfPredicateWithKey;

#define SeqRemoveWithRemoveIfPredicateWithKey_New(item, key)                   ((DREF SeqRemoveWithRemoveIfPredicateWithKey *)ProxyObject2_New(&SeqRemoveWithRemoveIfPredicateWithKey_Type, Dee_AsObject(item), Dee_AsObject(key)))
#define SeqRemoveWithRemoveIfPredicateWithKey_NewInherited(item, key)          ((DREF SeqRemoveWithRemoveIfPredicateWithKey *)ProxyObject2_NewInherited(&SeqRemoveWithRemoveIfPredicateWithKey_Type, Dee_AsObject(item), Dee_AsObject(key)))
#define SeqRemoveWithRemoveIfPredicateWithKey_NewInherited1(item, key)         ((DREF SeqRemoveWithRemoveIfPredicateWithKey *)ProxyObject2_NewInherited1(&SeqRemoveWithRemoveIfPredicateWithKey_Type, Dee_AsObject(item), Dee_AsObject(key)))
#define SeqRemoveWithRemoveIfPredicateWithKey_NewInherited2(item, key)         ((DREF SeqRemoveWithRemoveIfPredicateWithKey *)ProxyObject2_NewInherited2(&SeqRemoveWithRemoveIfPredicateWithKey_Type, Dee_AsObject(item), Dee_AsObject(key)))
#define SeqRemoveWithRemoveIfPredicateWithKey_NewInheritedOnSuccess(item, key) ((DREF SeqRemoveWithRemoveIfPredicateWithKey *)ProxyObject2_NewInheritedOnSuccess(&SeqRemoveWithRemoveIfPredicateWithKey_Type, Dee_AsObject(item), Dee_AsObject(key)))
#define SeqRemoveWithRemoveIfPredicateWithKey_DecrefSymbolic(self)              ProxyObject2_DecrefSymbolic((DREF ProxyObject2 *)Dee_REQUIRES_TYPE(DREF SeqRemoveWithRemoveIfPredicateWithKey *, self))

INTDEF DeeTypeObject SeqRemoveWithRemoveIfPredicate_Type;
INTDEF DeeTypeObject SeqRemoveWithRemoveIfPredicateWithKey_Type;





INTDEF DeeTypeObject SeqRemoveIfWithRemoveAllItem_Type;
INTDEF DeeObject SeqRemoveIfWithRemoveAllItem_DummyInstance;

typedef struct {
	PROXY_OBJECT_HEAD(sriwrak_should) /* [1..1] Predicate to determine if an element should be removed. */
} SeqRemoveIfWithRemoveAllKey;

#define SeqRemoveIfWithRemoveAllKey_New(should)                   ((DREF SeqRemoveIfWithRemoveAllKey *)ProxyObject_New(&SeqRemoveIfWithRemoveAllKey_Type, Dee_AsObject(should)))
#define SeqRemoveIfWithRemoveAllKey_NewInherited(should)          ((DREF SeqRemoveIfWithRemoveAllKey *)ProxyObject_NewInherited(&SeqRemoveIfWithRemoveAllKey_Type, Dee_AsObject(should)))
#define SeqRemoveIfWithRemoveAllKey_NewInheritedOnSuccess(should) ((DREF SeqRemoveIfWithRemoveAllKey *)ProxyObject_NewInheritedOnSuccess(&SeqRemoveIfWithRemoveAllKey_Type, Dee_AsObject(should)))
#define SeqRemoveIfWithRemoveAllKey_DecrefSymbolic(self)          ProxyObject_DecrefSymbolic((DREF ProxyObject *)Dee_REQUIRES_TYPE(DREF SeqRemoveIfWithRemoveAllKey *, self))

INTDEF DeeTypeObject SeqRemoveIfWithRemoveAllKey_Type;

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_REMOVEIF_CB_H */
