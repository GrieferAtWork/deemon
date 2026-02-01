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





#define MapUnion_New(a, b) \
	_MapUnion_New(Dee_AsObject(a), Dee_AsObject(b))
LOCAL WUNUSED NONNULL((1, 2)) DREF MapUnion *DCALL
_MapUnion_New(DeeObject *a, DeeObject *b) {
	DREF MapUnion *result;
	result = DeeObject_MALLOC(MapUnion);
	if unlikely(!result)
		goto err;
	Dee_Incref(a);
	Dee_Incref(b);
	result->mu_a = a;
	result->mu_b = b;
	DeeObject_Init(result, &MapUnion_Type);
	return result;
err:
	return NULL;
}

#define MapUnion_New_inherit_a(a, b) \
	_MapUnion_New_inherit_a(Dee_AsObject(a), Dee_AsObject(b))
LOCAL WUNUSED NONNULL((1, 2)) DREF MapUnion *DCALL
_MapUnion_New_inherit_a(/*inherit(always)*/ DREF DeeObject *a, DeeObject *b) {
	DREF MapUnion *result;
	result = DeeObject_MALLOC(MapUnion);
	if unlikely(!result)
		goto err;
	Dee_Incref(b);
	result->mu_a = a; /* Inherited */
	result->mu_b = b;
	DeeObject_Init(result, &MapUnion_Type);
	return result;
err:
	Dee_Decref(a);
	return NULL;
}

#define MapUnion_New_inherit_b(a, b) \
	_MapUnion_New_inherit_b(Dee_AsObject(a), Dee_AsObject(b))
LOCAL WUNUSED NONNULL((1, 2)) DREF MapUnion *DCALL
_MapUnion_New_inherit_b(DeeObject *a, /*inherit(always)*/ DREF DeeObject *b) {
	DREF MapUnion *result;
	result = DeeObject_MALLOC(MapUnion);
	if unlikely(!result)
		goto err;
	Dee_Incref(a);
	result->mu_a = a;
	result->mu_b = b; /* Inherited */
	DeeObject_Init(result, &MapUnion_Type);
	return result;
err:
	Dee_Decref(b);
	return NULL;
}

#define MapUnion_New_inherit_ab(a, b) \
	_MapUnion_New_inherit_ab(Dee_AsObject(a), Dee_AsObject(b))
LOCAL WUNUSED NONNULL((1, 2)) DREF MapUnion *DCALL
_MapUnion_New_inherit_ab(/*inherit(always)*/ DREF DeeObject *a,
                         /*inherit(always)*/ DREF DeeObject *b) {
	DREF MapUnion *result;
	result = DeeObject_MALLOC(MapUnion);
	if unlikely(!result)
		goto err;
	result->mu_a = a; /* Inherited */
	result->mu_b = b; /* Inherited */
	DeeObject_Init(result, &MapUnion_Type);
	return result;
err:
	Dee_Decref(b);
	Dee_Decref(a);
	return NULL;
}





#define MapIntersection_New(a, b) \
	_MapIntersection_New(Dee_AsObject(a), Dee_AsObject(b))
LOCAL WUNUSED NONNULL((1, 2)) DREF MapIntersection *DCALL
_MapIntersection_New(DeeObject *a, DeeObject *b) {
	DREF MapIntersection *result;
	result = DeeObject_MALLOC(MapIntersection);
	if unlikely(!result)
		goto err;
	Dee_Incref(a);
	Dee_Incref(b);
	result->mi_map = a;
	result->mi_keys = b;
	DeeObject_Init(result, &MapIntersection_Type);
	return result;
err:
	return NULL;
}

#define MapIntersection_New_inherit_a(a, b) \
	_MapIntersection_New_inherit_a(Dee_AsObject(a), Dee_AsObject(b))
LOCAL WUNUSED NONNULL((1, 2)) DREF MapIntersection *DCALL
_MapIntersection_New_inherit_a(/*inherit(always)*/ DREF DeeObject *a, DeeObject *b) {
	DREF MapIntersection *result;
	result = DeeObject_MALLOC(MapIntersection);
	if unlikely(!result)
		goto err;
	Dee_Incref(b);
	result->mi_map = a; /* Inherited */
	result->mi_keys = b;
	DeeObject_Init(result, &MapIntersection_Type);
	return result;
err:
	Dee_Decref(a);
	return NULL;
}

#define MapIntersection_New_inherit_b(a, b) \
	_MapIntersection_New_inherit_b(Dee_AsObject(a), Dee_AsObject(b))
LOCAL WUNUSED NONNULL((1, 2)) DREF MapIntersection *DCALL
_MapIntersection_New_inherit_b(DeeObject *a, /*inherit(always)*/ DREF DeeObject *b) {
	DREF MapIntersection *result;
	result = DeeObject_MALLOC(MapIntersection);
	if unlikely(!result)
		goto err;
	Dee_Incref(a);
	result->mi_map = a;
	result->mi_keys = b; /* Inherited */
	DeeObject_Init(result, &MapIntersection_Type);
	return result;
err:
	Dee_Decref(b);
	return NULL;
}

#define MapIntersection_New_inherit_ab(a, b) \
	_MapIntersection_New_inherit_ab(Dee_AsObject(a), Dee_AsObject(b))
LOCAL WUNUSED NONNULL((1, 2)) DREF MapIntersection *DCALL
_MapIntersection_New_inherit_ab(/*inherit(always)*/ DREF DeeObject *a,
                                /*inherit(always)*/ DREF DeeObject *b) {
	DREF MapIntersection *result;
	result = DeeObject_MALLOC(MapIntersection);
	if unlikely(!result)
		goto err;
	result->mi_map = a; /* Inherited */
	result->mi_keys = b; /* Inherited */
	DeeObject_Init(result, &MapIntersection_Type);
	return result;
err:
	Dee_Decref(b);
	Dee_Decref(a);
	return NULL;
}





#define MapDifference_New(a, b) \
	_MapDifference_New(Dee_AsObject(a), Dee_AsObject(b))
LOCAL WUNUSED NONNULL((1, 2)) DREF MapDifference *DCALL
_MapDifference_New(DeeObject *a, DeeObject *b) {
	DREF MapDifference *result;
	result = DeeObject_MALLOC(MapDifference);
	if unlikely(!result)
		goto err;
	Dee_Incref(a);
	Dee_Incref(b);
	result->md_map = a;
	result->md_keys = b;
	DeeObject_Init(result, &MapDifference_Type);
	return result;
err:
	return NULL;
}

#define MapDifference_New_inherit_a(a, b) \
	_MapDifference_New_inherit_a(Dee_AsObject(a), Dee_AsObject(b))
LOCAL WUNUSED NONNULL((1, 2)) DREF MapDifference *DCALL
_MapDifference_New_inherit_a(/*inherit(always)*/ DREF DeeObject *a, DeeObject *b) {
	DREF MapDifference *result;
	result = DeeObject_MALLOC(MapDifference);
	if unlikely(!result)
		goto err;
	Dee_Incref(b);
	result->md_map = a; /* Inherited */
	result->md_keys = b;
	DeeObject_Init(result, &MapDifference_Type);
	return result;
err:
	Dee_Decref(a);
	return NULL;
}

#define MapDifference_New_inherit_b(a, b) \
	_MapDifference_New_inherit_b(Dee_AsObject(a), Dee_AsObject(b))
LOCAL WUNUSED NONNULL((1, 2)) DREF MapDifference *DCALL
_MapDifference_New_inherit_b(DeeObject *a, /*inherit(always)*/ DREF DeeObject *b) {
	DREF MapDifference *result;
	result = DeeObject_MALLOC(MapDifference);
	if unlikely(!result)
		goto err;
	Dee_Incref(a);
	result->md_map = a;
	result->md_keys = b; /* Inherited */
	DeeObject_Init(result, &MapDifference_Type);
	return result;
err:
	Dee_Decref(b);
	return NULL;
}

#define MapDifference_New_inherit_ab(a, b) \
	_MapDifference_New_inherit_ab(Dee_AsObject(a), Dee_AsObject(b))
LOCAL WUNUSED NONNULL((1, 2)) DREF MapDifference *DCALL
_MapDifference_New_inherit_ab(/*inherit(always)*/ DREF DeeObject *a,
                              /*inherit(always)*/ DREF DeeObject *b) {
	DREF MapDifference *result;
	result = DeeObject_MALLOC(MapDifference);
	if unlikely(!result)
		goto err;
	result->md_map = a; /* Inherited */
	result->md_keys = b; /* Inherited */
	DeeObject_Init(result, &MapDifference_Type);
	return result;
err:
	Dee_Decref(b);
	Dee_Decref(a);
	return NULL;
}





#define MapSymmetricDifference_New(a, b) \
	_MapSymmetricDifference_New(Dee_AsObject(a), Dee_AsObject(b))
LOCAL WUNUSED NONNULL((1, 2)) DREF MapSymmetricDifference *DCALL
_MapSymmetricDifference_New(DeeObject *a, DeeObject *b) {
	DREF MapSymmetricDifference *result;
	result = DeeObject_MALLOC(MapSymmetricDifference);
	if unlikely(!result)
		goto err;
	Dee_Incref(a);
	Dee_Incref(b);
	result->msd_a = a;
	result->msd_b = b;
	DeeObject_Init(result, &MapSymmetricDifference_Type);
	return result;
err:
	return NULL;
}

#define MapSymmetricDifference_New_inherit_a(a, b) \
	_MapSymmetricDifference_New_inherit_a(Dee_AsObject(a), Dee_AsObject(b))
LOCAL WUNUSED NONNULL((1, 2)) DREF MapSymmetricDifference *DCALL
_MapSymmetricDifference_New_inherit_a(/*inherit(always)*/ DREF DeeObject *a, DeeObject *b) {
	DREF MapSymmetricDifference *result;
	result = DeeObject_MALLOC(MapSymmetricDifference);
	if unlikely(!result)
		goto err;
	Dee_Incref(b);
	result->msd_a = a; /* Inherited */
	result->msd_b = b;
	DeeObject_Init(result, &MapSymmetricDifference_Type);
	return result;
err:
	Dee_Decref(a);
	return NULL;
}

#define MapSymmetricDifference_New_inherit_b(a, b) \
	_MapSymmetricDifference_New_inherit_b(Dee_AsObject(a), Dee_AsObject(b))
LOCAL WUNUSED NONNULL((1, 2)) DREF MapSymmetricDifference *DCALL
_MapSymmetricDifference_New_inherit_b(DeeObject *a, /*inherit(always)*/ DREF DeeObject *b) {
	DREF MapSymmetricDifference *result;
	result = DeeObject_MALLOC(MapSymmetricDifference);
	if unlikely(!result)
		goto err;
	Dee_Incref(a);
	result->msd_a = a;
	result->msd_b = b; /* Inherited */
	DeeObject_Init(result, &MapSymmetricDifference_Type);
	return result;
err:
	Dee_Decref(b);
	return NULL;
}

#define MapSymmetricDifference_New_inherit_ab(a, b) \
	_MapSymmetricDifference_New_inherit_ab(Dee_AsObject(a), Dee_AsObject(b))
LOCAL WUNUSED NONNULL((1, 2)) DREF MapSymmetricDifference *DCALL
_MapSymmetricDifference_New_inherit_ab(/*inherit(always)*/ DREF DeeObject *a,
                                       /*inherit(always)*/ DREF DeeObject *b) {
	DREF MapSymmetricDifference *result;
	result = DeeObject_MALLOC(MapSymmetricDifference);
	if unlikely(!result)
		goto err;
	result->msd_a = a; /* Inherited */
	result->msd_b = b; /* Inherited */
	DeeObject_Init(result, &MapSymmetricDifference_Type);
	return result;
err:
	Dee_Decref(b);
	Dee_Decref(a);
	return NULL;
}


INTDEF WUNUSED NONNULL((1, 2)) int DCALL MapIntersection_NonEmpty(DeeObject *map, DeeObject *keys);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL MapDifference_NonEmpty(DeeObject *map, DeeObject *keys);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL MapDifferenceMapKeys_NonEmpty(DeeObject *map, DeeObject *map2);

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_DEFAULT_MAPS_H */
