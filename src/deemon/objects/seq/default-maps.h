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
#ifndef GUARD_DEEMON_OBJECTS_SEQ_DEFAULT_MAPS_H
#define GUARD_DEEMON_OBJECTS_SEQ_DEFAULT_MAPS_H 1

#include <deemon/api.h>

#include <deemon/alloc.h>     /* DeeObject_MALLOC */
#include <deemon/map.h>       /* DeeMapping_Type */
#include <deemon/object.h>    /* DREF, DeeObject, DeeObject_InstanceOfExact, DeeTypeObject, Dee_AsObject, Dee_Decref, Dee_Incref */
#include <deemon/type.h>      /* DeeObject_Init */
#include <deemon/util/lock.h> /* Dee_atomic_rwlock_* */

#include "../generic-proxy.h"

#include <stdbool.h> /* bool */
#include <stddef.h>  /* NULL */

DECL_BEGIN

/* Map proxy types used to implement map operations on the C-level. */

/* Check for a symbolic, empty map.
 * NOTE: This function isn't guarantied to capture any kind of empty map,
 *       only maps that are meant to symbolically represent an empty one.
 * This map is represented as `{}' */
#define DeeMap_CheckEmpty(x) DeeObject_InstanceOfExact(x, &DeeMapping_Type)

typedef struct {
	PROXY_OBJECT_HEAD2(mu_a,  /* [1..1][const] The first map of the union. */
	                   mu_b); /* [1..1][const] The second map of the union. */
} MapUnion;

#define MapUnion_New(obj1, obj2)                   ((DREF MapUnion *)ProxyObject2_New(&MapUnion_Type, Dee_AsObject(obj1), Dee_AsObject(obj2)))
#define MapUnion_NewInherited(obj1, obj2)          ((DREF MapUnion *)ProxyObject2_NewInherited(&MapUnion_Type, Dee_AsObject(obj1), Dee_AsObject(obj2)))
#define MapUnion_NewInherited1(obj1, obj2)         ((DREF MapUnion *)ProxyObject2_NewInherited1(&MapUnion_Type, Dee_AsObject(obj1), Dee_AsObject(obj2)))
#define MapUnion_NewInherited2(obj1, obj2)         ((DREF MapUnion *)ProxyObject2_NewInherited2(&MapUnion_Type, Dee_AsObject(obj1), Dee_AsObject(obj2)))
#define MapUnion_NewInheritedOnSuccess(obj1, obj2) ((DREF MapUnion *)ProxyObject2_NewInheritedOnSuccess(&MapUnion_Type, Dee_AsObject(obj1), Dee_AsObject(obj2)))

typedef struct {
	PROXY_OBJECT_HEAD2_EX(DeeObject, mui_iter,  /* [1..1][lock(mui_lock)] The current iterator. */
	                      MapUnion,  mui_union) /* [1..1][const] The underlying union-map. */
#ifndef CONFIG_NO_THREADS
	Dee_atomic_rwlock_t              mui_lock;  /* Lock for `mui_iter' and `mui_in2nd' */
#endif /* !CONFIG_NO_THREADS */
	bool                             mui_in2nd; /* [lock(mui_lock)] The second map is being iterated. */
} MapUnionIterator;

#define MapUnionIterator_LockReading(self)    Dee_atomic_rwlock_reading(&(self)->mui_lock)
#define MapUnionIterator_LockWriting(self)    Dee_atomic_rwlock_writing(&(self)->mui_lock)
#define MapUnionIterator_LockTryRead(self)    Dee_atomic_rwlock_tryread(&(self)->mui_lock)
#define MapUnionIterator_LockTryWrite(self)   Dee_atomic_rwlock_trywrite(&(self)->mui_lock)
#define MapUnionIterator_LockCanRead(self)    Dee_atomic_rwlock_canread(&(self)->mui_lock)
#define MapUnionIterator_LockCanWrite(self)   Dee_atomic_rwlock_canwrite(&(self)->mui_lock)
#define MapUnionIterator_LockWaitRead(self)   Dee_atomic_rwlock_waitread(&(self)->mui_lock)
#define MapUnionIterator_LockWaitWrite(self)  Dee_atomic_rwlock_waitwrite(&(self)->mui_lock)
#define MapUnionIterator_LockRead(self)       Dee_atomic_rwlock_read(&(self)->mui_lock)
#define MapUnionIterator_LockWrite(self)      Dee_atomic_rwlock_write(&(self)->mui_lock)
#define MapUnionIterator_LockTryUpgrade(self) Dee_atomic_rwlock_tryupgrade(&(self)->mui_lock)
#define MapUnionIterator_LockUpgrade(self)    Dee_atomic_rwlock_upgrade(&(self)->mui_lock)
#define MapUnionIterator_LockDowngrade(self)  Dee_atomic_rwlock_downgrade(&(self)->mui_lock)
#define MapUnionIterator_LockEndWrite(self)   Dee_atomic_rwlock_endwrite(&(self)->mui_lock)
#define MapUnionIterator_LockEndRead(self)    Dee_atomic_rwlock_endread(&(self)->mui_lock)
#define MapUnionIterator_LockEnd(self)        Dee_atomic_rwlock_end(&(self)->mui_lock)

INTDEF DeeTypeObject MapUnion_Type;
INTDEF DeeTypeObject MapUnionIterator_Type;



typedef struct {
	PROXY_OBJECT_HEAD2(mi_map,   /* [1..1][const] The mapping of the intersection. */
	                   mi_keys); /* [1..1][const] Key filter of the intersection. */
} MapIntersection;

#define MapIntersection_New(obj1, obj2)                   ((DREF MapIntersection *)ProxyObject2_New(&MapIntersection_Type, Dee_AsObject(obj1), Dee_AsObject(obj2)))
#define MapIntersection_NewInherited(obj1, obj2)          ((DREF MapIntersection *)ProxyObject2_NewInherited(&MapIntersection_Type, Dee_AsObject(obj1), Dee_AsObject(obj2)))
#define MapIntersection_NewInherited1(obj1, obj2)         ((DREF MapIntersection *)ProxyObject2_NewInherited1(&MapIntersection_Type, Dee_AsObject(obj1), Dee_AsObject(obj2)))
#define MapIntersection_NewInherited2(obj1, obj2)         ((DREF MapIntersection *)ProxyObject2_NewInherited2(&MapIntersection_Type, Dee_AsObject(obj1), Dee_AsObject(obj2)))
#define MapIntersection_NewInheritedOnSuccess(obj1, obj2) ((DREF MapIntersection *)ProxyObject2_NewInheritedOnSuccess(&MapIntersection_Type, Dee_AsObject(obj1), Dee_AsObject(obj2)))

typedef struct {
	PROXY_OBJECT_HEAD2_EX(DeeObject,       mii_iter,       /* [1..1][const] An iterator for `mii_intersect->mi_map' */
	                      MapIntersection, mii_intersect); /* [1..1][const] The underlying intersection-map. */
	DeeObject                             *mii_keys;       /* [1..1][const][== mii_intersect->mi_keys]. */
} MapIntersectionIterator;

INTDEF DeeTypeObject MapIntersection_Type;
INTDEF DeeTypeObject MapIntersectionIterator_Type;



typedef struct {
	PROXY_OBJECT_HEAD2(md_map,   /* [1..1][const] The primary map. */
	                   md_keys); /* [1..1][const] Set of keys that should be excluded from `md_map' */
} MapDifference;

#define MapDifference_New(obj1, obj2)                   ((DREF MapDifference *)ProxyObject2_New(&MapDifference_Type, Dee_AsObject(obj1), Dee_AsObject(obj2)))
#define MapDifference_NewInherited(obj1, obj2)          ((DREF MapDifference *)ProxyObject2_NewInherited(&MapDifference_Type, Dee_AsObject(obj1), Dee_AsObject(obj2)))
#define MapDifference_NewInherited1(obj1, obj2)         ((DREF MapDifference *)ProxyObject2_NewInherited1(&MapDifference_Type, Dee_AsObject(obj1), Dee_AsObject(obj2)))
#define MapDifference_NewInherited2(obj1, obj2)         ((DREF MapDifference *)ProxyObject2_NewInherited2(&MapDifference_Type, Dee_AsObject(obj1), Dee_AsObject(obj2)))
#define MapDifference_NewInheritedOnSuccess(obj1, obj2) ((DREF MapDifference *)ProxyObject2_NewInheritedOnSuccess(&MapDifference_Type, Dee_AsObject(obj1), Dee_AsObject(obj2)))

typedef struct {
	PROXY_OBJECT_HEAD2_EX(DeeObject,     mdi_iter,  /* [1..1][const] An iterator for `mdi_diff->md_map' */
	                      MapDifference, mdi_diff); /* [1..1][const] The underlying difference-map. */
	DeeObject                           *mdi_keys;  /* [1..1][const][== mdi_diff->md_keys]. */
} MapDifferenceIterator;

INTDEF DeeTypeObject MapDifference_Type;
INTDEF DeeTypeObject MapDifferenceIterator_Type;



typedef struct {
	PROXY_OBJECT_HEAD2(msd_a,  /* [1..1][const] The first map. */
	                   msd_b); /* [1..1][const] The second map. */
} MapSymmetricDifference;

#define MapSymmetricDifference_New(obj1, obj2)                   ((DREF MapSymmetricDifference *)ProxyObject2_New(&MapSymmetricDifference_Type, Dee_AsObject(obj1), Dee_AsObject(obj2)))
#define MapSymmetricDifference_NewInherited(obj1, obj2)          ((DREF MapSymmetricDifference *)ProxyObject2_NewInherited(&MapSymmetricDifference_Type, Dee_AsObject(obj1), Dee_AsObject(obj2)))
#define MapSymmetricDifference_NewInherited1(obj1, obj2)         ((DREF MapSymmetricDifference *)ProxyObject2_NewInherited1(&MapSymmetricDifference_Type, Dee_AsObject(obj1), Dee_AsObject(obj2)))
#define MapSymmetricDifference_NewInherited2(obj1, obj2)         ((DREF MapSymmetricDifference *)ProxyObject2_NewInherited2(&MapSymmetricDifference_Type, Dee_AsObject(obj1), Dee_AsObject(obj2)))
#define MapSymmetricDifference_NewInheritedOnSuccess(obj1, obj2) ((DREF MapSymmetricDifference *)ProxyObject2_NewInheritedOnSuccess(&MapSymmetricDifference_Type, Dee_AsObject(obj1), Dee_AsObject(obj2)))

typedef struct {
	PROXY_OBJECT_HEAD2_EX(DeeObject,              msdi_iter,     /* [1..1][lock(msdi_lock)] The current iterator. */
	                      MapSymmetricDifference, msdi_symdiff); /* [1..1][const] The underlying map. */
#ifndef CONFIG_NO_THREADS
	Dee_atomic_rwlock_t                           msdi_lock;     /* Lock for `msdi_iter' and `msdi_in2nd' */
#endif /* !CONFIG_NO_THREADS */
	bool                                          msdi_in2nd;    /* [lock(msdi_lock)] The second map is being iterated. */
} MapSymmetricDifferenceIterator;

#define MapSymmetricDifferenceIterator_LockReading(self)    Dee_atomic_rwlock_reading(&(self)->msdi_lock)
#define MapSymmetricDifferenceIterator_LockWriting(self)    Dee_atomic_rwlock_writing(&(self)->msdi_lock)
#define MapSymmetricDifferenceIterator_LockTryRead(self)    Dee_atomic_rwlock_tryread(&(self)->msdi_lock)
#define MapSymmetricDifferenceIterator_LockTryWrite(self)   Dee_atomic_rwlock_trywrite(&(self)->msdi_lock)
#define MapSymmetricDifferenceIterator_LockCanRead(self)    Dee_atomic_rwlock_canread(&(self)->msdi_lock)
#define MapSymmetricDifferenceIterator_LockCanWrite(self)   Dee_atomic_rwlock_canwrite(&(self)->msdi_lock)
#define MapSymmetricDifferenceIterator_LockWaitRead(self)   Dee_atomic_rwlock_waitread(&(self)->msdi_lock)
#define MapSymmetricDifferenceIterator_LockWaitWrite(self)  Dee_atomic_rwlock_waitwrite(&(self)->msdi_lock)
#define MapSymmetricDifferenceIterator_LockRead(self)       Dee_atomic_rwlock_read(&(self)->msdi_lock)
#define MapSymmetricDifferenceIterator_LockWrite(self)      Dee_atomic_rwlock_write(&(self)->msdi_lock)
#define MapSymmetricDifferenceIterator_LockTryUpgrade(self) Dee_atomic_rwlock_tryupgrade(&(self)->msdi_lock)
#define MapSymmetricDifferenceIterator_LockUpgrade(self)    Dee_atomic_rwlock_upgrade(&(self)->msdi_lock)
#define MapSymmetricDifferenceIterator_LockDowngrade(self)  Dee_atomic_rwlock_downgrade(&(self)->msdi_lock)
#define MapSymmetricDifferenceIterator_LockEndWrite(self)   Dee_atomic_rwlock_endwrite(&(self)->msdi_lock)
#define MapSymmetricDifferenceIterator_LockEndRead(self)    Dee_atomic_rwlock_endread(&(self)->msdi_lock)
#define MapSymmetricDifferenceIterator_LockEnd(self)        Dee_atomic_rwlock_end(&(self)->msdi_lock)

INTDEF DeeTypeObject MapSymmetricDifference_Type;
INTDEF DeeTypeObject MapSymmetricDifferenceIterator_Type;


INTDEF WUNUSED NONNULL((1, 2)) int DCALL MapIntersection_NonEmpty(DeeObject *map, DeeObject *keys);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL MapDifference_NonEmpty(DeeObject *map, DeeObject *keys);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL MapDifferenceMapKeys_NonEmpty(DeeObject *map, DeeObject *map2);

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_DEFAULT_MAPS_H */
