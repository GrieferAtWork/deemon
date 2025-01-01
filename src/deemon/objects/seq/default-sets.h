/* Copyright (c) 2018-2025 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2025 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_OBJECTS_SEQ_DEFAULT_SETS_H
#define GUARD_DEEMON_OBJECTS_SEQ_DEFAULT_SETS_H 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/object.h>
#include <deemon/set.h>
#include <deemon/util/lock.h>

/**/
#include "../generic-proxy.h"

DECL_BEGIN

/* Set proxy types used to implement set operations on the C-level. */

typedef struct {
	/* An inverse set, that is the symbolic set containing all
	 * object, excluding those already contained within `si_set'
	 * Since such a set cannot be iterated, working with it
	 * requires some special operations, as well as special
	 * support in some places, which is why it is exposed here.
	 * In user-code, such a set is created through use of `operator ~()' */
	PROXY_OBJECT_HEAD(si_set); /* [1..1][const] The underlying set. */
} SetInversion;

#define SetInversion_GetSet(self) ((SetInversion *)(self))->si_set /* TODO: Remove this macro! */

INTDEF DeeTypeObject SetInversion_Type;
#define SetInversion_Check(ob)      DeeObject_InstanceOfExact(ob, &SetInversion_Type)
#define SetInversion_CheckExact(ob) DeeObject_InstanceOfExact(ob, &SetInversion_Type)


/* Check for a symbolic, empty set.
 * NOTE: This function isn't guarantied to capture any kind of empty set,
 *       only sets that are meant to symbolically represent an empty one.
 * This set is represented as `{}' */
#define DeeSet_CheckEmpty(x) DeeObject_InstanceOfExact(x, &DeeSet_Type)



typedef struct {
	PROXY_OBJECT_HEAD2(su_a,  /* [1..1][const] The first set of the union. */
	                   su_b); /* [1..1][const] The second set of the union. */
} SetUnion;

typedef struct {
	PROXY_OBJECT_HEAD2_EX(DeeObject, sui_iter,  /* [1..1][lock(sui_lock)] The current iterator. */
	                      SetUnion,  sui_union) /* [1..1][const] The underlying union-set. */
#ifndef CONFIG_NO_THREADS
	Dee_atomic_rwlock_t              sui_lock;  /* Lock for `sui_iter' and `sui_in2nd' */
#endif /* !CONFIG_NO_THREADS */
	bool                             sui_in2nd; /* [lock(sui_lock)] The second set is being iterated. */
} SetUnionIterator;

#define SetUnionIterator_LockReading(self)    Dee_atomic_rwlock_reading(&(self)->sui_lock)
#define SetUnionIterator_LockWriting(self)    Dee_atomic_rwlock_writing(&(self)->sui_lock)
#define SetUnionIterator_LockTryRead(self)    Dee_atomic_rwlock_tryread(&(self)->sui_lock)
#define SetUnionIterator_LockTryWrite(self)   Dee_atomic_rwlock_trywrite(&(self)->sui_lock)
#define SetUnionIterator_LockCanRead(self)    Dee_atomic_rwlock_canread(&(self)->sui_lock)
#define SetUnionIterator_LockCanWrite(self)   Dee_atomic_rwlock_canwrite(&(self)->sui_lock)
#define SetUnionIterator_LockWaitRead(self)   Dee_atomic_rwlock_waitread(&(self)->sui_lock)
#define SetUnionIterator_LockWaitWrite(self)  Dee_atomic_rwlock_waitwrite(&(self)->sui_lock)
#define SetUnionIterator_LockRead(self)       Dee_atomic_rwlock_read(&(self)->sui_lock)
#define SetUnionIterator_LockWrite(self)      Dee_atomic_rwlock_write(&(self)->sui_lock)
#define SetUnionIterator_LockTryUpgrade(self) Dee_atomic_rwlock_tryupgrade(&(self)->sui_lock)
#define SetUnionIterator_LockUpgrade(self)    Dee_atomic_rwlock_upgrade(&(self)->sui_lock)
#define SetUnionIterator_LockDowngrade(self)  Dee_atomic_rwlock_downgrade(&(self)->sui_lock)
#define SetUnionIterator_LockEndWrite(self)   Dee_atomic_rwlock_endwrite(&(self)->sui_lock)
#define SetUnionIterator_LockEndRead(self)    Dee_atomic_rwlock_endread(&(self)->sui_lock)
#define SetUnionIterator_LockEnd(self)        Dee_atomic_rwlock_end(&(self)->sui_lock)

INTDEF DeeTypeObject SetUnion_Type;
INTDEF DeeTypeObject SetUnionIterator_Type;



typedef struct {
	PROXY_OBJECT_HEAD2(si_a,  /* [1..1][const] The first set of the intersection. */
	                   si_b); /* [1..1][const] The second set of the intersection. */
} SetIntersection;

typedef struct {
	PROXY_OBJECT_HEAD2_EX(DeeObject,       sii_iter,       /* [1..1][const] An iterator for `sii_intersect->si_a' */
	                      SetIntersection, sii_intersect); /* [1..1][const] The underlying intersection-set. */
	DeeObject                             *sii_other;      /* [1..1][const][== sii_intersect->si_b]. */
} SetIntersectionIterator;

INTDEF DeeTypeObject SetIntersection_Type;
INTDEF DeeTypeObject SetIntersectionIterator_Type;



typedef struct {
	PROXY_OBJECT_HEAD2(sd_a,  /* [1..1][const] The primary set. */
	                   sd_b); /* [1..1][const] The set of objects excluded from `sd_a'. */
} SetDifference;

typedef struct {
	PROXY_OBJECT_HEAD2_EX(DeeObject,     sdi_iter,  /* [1..1][const] An iterator for `sdi_diff->sd_a' */
	                      SetDifference, sdi_diff); /* [1..1][const] The underlying difference-set. */
	DeeObject                           *sdi_other; /* [1..1][const][== sdi_diff->sd_b]. */
} SetDifferenceIterator;

INTDEF DeeTypeObject SetDifference_Type;
INTDEF DeeTypeObject SetDifferenceIterator_Type;



typedef struct {
	PROXY_OBJECT_HEAD2(ssd_a,  /* [1..1][const] The first set. */
	                   ssd_b); /* [1..1][const] The second set. */
} SetSymmetricDifference;

typedef struct {
	PROXY_OBJECT_HEAD2_EX(DeeObject,              ssd_iter,  /* [1..1][lock(ssd_lock)] The current iterator. */
	                      SetSymmetricDifference, ssd_set);  /* [1..1][const] The underlying set. */
#ifndef CONFIG_NO_THREADS
	Dee_atomic_rwlock_t                           ssd_lock;  /* Lock for `ssd_iter' and `ssd_in2nd' */
#endif /* !CONFIG_NO_THREADS */
	bool                                          ssd_in2nd; /* [lock(ssd_lock)] The second set is being iterated. */
} SetSymmetricDifferenceIterator;

#define SetSymmetricDifferenceIterator_LockReading(self)    Dee_atomic_rwlock_reading(&(self)->ssd_lock)
#define SetSymmetricDifferenceIterator_LockWriting(self)    Dee_atomic_rwlock_writing(&(self)->ssd_lock)
#define SetSymmetricDifferenceIterator_LockTryRead(self)    Dee_atomic_rwlock_tryread(&(self)->ssd_lock)
#define SetSymmetricDifferenceIterator_LockTryWrite(self)   Dee_atomic_rwlock_trywrite(&(self)->ssd_lock)
#define SetSymmetricDifferenceIterator_LockCanRead(self)    Dee_atomic_rwlock_canread(&(self)->ssd_lock)
#define SetSymmetricDifferenceIterator_LockCanWrite(self)   Dee_atomic_rwlock_canwrite(&(self)->ssd_lock)
#define SetSymmetricDifferenceIterator_LockWaitRead(self)   Dee_atomic_rwlock_waitread(&(self)->ssd_lock)
#define SetSymmetricDifferenceIterator_LockWaitWrite(self)  Dee_atomic_rwlock_waitwrite(&(self)->ssd_lock)
#define SetSymmetricDifferenceIterator_LockRead(self)       Dee_atomic_rwlock_read(&(self)->ssd_lock)
#define SetSymmetricDifferenceIterator_LockWrite(self)      Dee_atomic_rwlock_write(&(self)->ssd_lock)
#define SetSymmetricDifferenceIterator_LockTryUpgrade(self) Dee_atomic_rwlock_tryupgrade(&(self)->ssd_lock)
#define SetSymmetricDifferenceIterator_LockUpgrade(self)    Dee_atomic_rwlock_upgrade(&(self)->ssd_lock)
#define SetSymmetricDifferenceIterator_LockDowngrade(self)  Dee_atomic_rwlock_downgrade(&(self)->ssd_lock)
#define SetSymmetricDifferenceIterator_LockEndWrite(self)   Dee_atomic_rwlock_endwrite(&(self)->ssd_lock)
#define SetSymmetricDifferenceIterator_LockEndRead(self)    Dee_atomic_rwlock_endread(&(self)->ssd_lock)
#define SetSymmetricDifferenceIterator_LockEnd(self)        Dee_atomic_rwlock_end(&(self)->ssd_lock)

INTDEF DeeTypeObject SetSymmetricDifference_Type;
INTDEF DeeTypeObject SetSymmetricDifferenceIterator_Type;



#define SetInversion_New(a) \
	_SetInversion_New((DeeObject *)Dee_REQUIRES_OBJECT(a))
LOCAL WUNUSED NONNULL((1)) DREF SetInversion *DCALL
_SetInversion_New(DeeObject *a) {
	DREF SetInversion *result;
	result = DeeObject_MALLOC(SetInversion);
	if unlikely(!result)
		goto err;
	Dee_Incref(a);
	result->si_set = a;
	DeeObject_Init(result, &SetInversion_Type);
	return result;
err:
	return NULL;
}

#define SetInversion_New_inherit(a) \
	_SetInversion_New_inherit((DREF DeeObject *)Dee_REQUIRES_OBJECT(a))
LOCAL WUNUSED NONNULL((1)) DREF SetInversion *DCALL
_SetInversion_New_inherit(/*inherit(always)*/ DREF DeeObject *a) {
	DREF SetInversion *result;
	result = DeeObject_MALLOC(SetInversion);
	if unlikely(!result)
		goto err;
	result->si_set = a; /* Inherited */
	DeeObject_Init(result, &SetInversion_Type);
	return result;
err:
	Dee_Decref(a);
	return NULL;
}





#define SetUnion_New(a, b) \
	_SetUnion_New((DeeObject *)Dee_REQUIRES_OBJECT(a), (DeeObject *)Dee_REQUIRES_OBJECT(b))
LOCAL WUNUSED NONNULL((1, 2)) DREF SetUnion *DCALL
_SetUnion_New(DeeObject *a, DeeObject *b) {
	DREF SetUnion *result;
	result = DeeObject_MALLOC(SetUnion);
	if unlikely(!result)
		goto err;
	Dee_Incref(a);
	Dee_Incref(b);
	result->su_a = a;
	result->su_b = b;
	DeeObject_Init(result, &SetUnion_Type);
	return result;
err:
	return NULL;
}

#define SetUnion_New_inherit_a(a, b) \
	_SetUnion_New_inherit_a((DREF DeeObject *)Dee_REQUIRES_OBJECT(a), (DeeObject *)Dee_REQUIRES_OBJECT(b))
LOCAL WUNUSED NONNULL((1, 2)) DREF SetUnion *DCALL
_SetUnion_New_inherit_a(/*inherit(always)*/ DREF DeeObject *a, DeeObject *b) {
	DREF SetUnion *result;
	result = DeeObject_MALLOC(SetUnion);
	if unlikely(!result)
		goto err;
	Dee_Incref(b);
	result->su_a = a; /* Inherited */
	result->su_b = b;
	DeeObject_Init(result, &SetUnion_Type);
	return result;
err:
	Dee_Decref(a);
	return NULL;
}

#define SetUnion_New_inherit_b(a, b) \
	_SetUnion_New_inherit_b((DeeObject *)Dee_REQUIRES_OBJECT(a), (DREF DeeObject *)Dee_REQUIRES_OBJECT(b))
LOCAL WUNUSED NONNULL((1, 2)) DREF SetUnion *DCALL
_SetUnion_New_inherit_b(DeeObject *a, /*inherit(always)*/ DREF DeeObject *b) {
	DREF SetUnion *result;
	result = DeeObject_MALLOC(SetUnion);
	if unlikely(!result)
		goto err;
	Dee_Incref(a);
	result->su_a = a;
	result->su_b = b; /* Inherited */
	DeeObject_Init(result, &SetUnion_Type);
	return result;
err:
	Dee_Decref(b);
	return NULL;
}

#define SetUnion_New_inherit_ab(a, b) \
	_SetUnion_New_inherit_ab((DREF DeeObject *)Dee_REQUIRES_OBJECT(a), (DREF DeeObject *)Dee_REQUIRES_OBJECT(b))
LOCAL WUNUSED NONNULL((1, 2)) DREF SetUnion *DCALL
_SetUnion_New_inherit_ab(/*inherit(always)*/ DREF DeeObject *a,
                         /*inherit(always)*/ DREF DeeObject *b) {
	DREF SetUnion *result;
	result = DeeObject_MALLOC(SetUnion);
	if unlikely(!result)
		goto err;
	result->su_a = a; /* Inherited */
	result->su_b = b; /* Inherited */
	DeeObject_Init(result, &SetUnion_Type);
	return result;
err:
	Dee_Decref(b);
	Dee_Decref(a);
	return NULL;
}





#define SetIntersection_New(a, b) \
	_SetIntersection_New((DeeObject *)Dee_REQUIRES_OBJECT(a), (DeeObject *)Dee_REQUIRES_OBJECT(b))
LOCAL WUNUSED NONNULL((1, 2)) DREF SetIntersection *DCALL
_SetIntersection_New(DeeObject *a, DeeObject *b) {
	DREF SetIntersection *result;
	result = DeeObject_MALLOC(SetIntersection);
	if unlikely(!result)
		goto err;
	Dee_Incref(a);
	Dee_Incref(b);
	result->si_a = a;
	result->si_b = b;
	DeeObject_Init(result, &SetIntersection_Type);
	return result;
err:
	return NULL;
}

#define SetIntersection_New_inherit_a(a, b) \
	_SetIntersection_New_inherit_a((DREF DeeObject *)Dee_REQUIRES_OBJECT(a), (DeeObject *)Dee_REQUIRES_OBJECT(b))
LOCAL WUNUSED NONNULL((1, 2)) DREF SetIntersection *DCALL
_SetIntersection_New_inherit_a(/*inherit(always)*/ DREF DeeObject *a, DeeObject *b) {
	DREF SetIntersection *result;
	result = DeeObject_MALLOC(SetIntersection);
	if unlikely(!result)
		goto err;
	Dee_Incref(b);
	result->si_a = a; /* Inherited */
	result->si_b = b;
	DeeObject_Init(result, &SetIntersection_Type);
	return result;
err:
	Dee_Decref(a);
	return NULL;
}

#define SetIntersection_New_inherit_b(a, b) \
	_SetIntersection_New_inherit_b((DeeObject *)Dee_REQUIRES_OBJECT(a), (DREF DeeObject *)Dee_REQUIRES_OBJECT(b))
LOCAL WUNUSED NONNULL((1, 2)) DREF SetIntersection *DCALL
_SetIntersection_New_inherit_b(DeeObject *a, /*inherit(always)*/ DREF DeeObject *b) {
	DREF SetIntersection *result;
	result = DeeObject_MALLOC(SetIntersection);
	if unlikely(!result)
		goto err;
	Dee_Incref(a);
	result->si_a = a;
	result->si_b = b; /* Inherited */
	DeeObject_Init(result, &SetIntersection_Type);
	return result;
err:
	Dee_Decref(b);
	return NULL;
}

#define SetIntersection_New_inherit_ab(a, b) \
	_SetIntersection_New_inherit_ab((DREF DeeObject *)Dee_REQUIRES_OBJECT(a), (DREF DeeObject *)Dee_REQUIRES_OBJECT(b))
LOCAL WUNUSED NONNULL((1, 2)) DREF SetIntersection *DCALL
_SetIntersection_New_inherit_ab(/*inherit(always)*/ DREF DeeObject *a,
                                /*inherit(always)*/ DREF DeeObject *b) {
	DREF SetIntersection *result;
	result = DeeObject_MALLOC(SetIntersection);
	if unlikely(!result)
		goto err;
	result->si_a = a; /* Inherited */
	result->si_b = b; /* Inherited */
	DeeObject_Init(result, &SetIntersection_Type);
	return result;
err:
	Dee_Decref(b);
	Dee_Decref(a);
	return NULL;
}





#define SetDifference_New(a, b) \
	_SetDifference_New((DeeObject *)Dee_REQUIRES_OBJECT(a), (DeeObject *)Dee_REQUIRES_OBJECT(b))
LOCAL WUNUSED NONNULL((1, 2)) DREF SetDifference *DCALL
_SetDifference_New(DeeObject *a, DeeObject *b) {
	DREF SetDifference *result;
	result = DeeObject_MALLOC(SetDifference);
	if unlikely(!result)
		goto err;
	Dee_Incref(a);
	Dee_Incref(b);
	result->sd_a = a;
	result->sd_b = b;
	DeeObject_Init(result, &SetDifference_Type);
	return result;
err:
	return NULL;
}

#define SetDifference_New_inherit_a(a, b) \
	_SetDifference_New_inherit_a((DREF DeeObject *)Dee_REQUIRES_OBJECT(a), (DeeObject *)Dee_REQUIRES_OBJECT(b))
LOCAL WUNUSED NONNULL((1, 2)) DREF SetDifference *DCALL
_SetDifference_New_inherit_a(/*inherit(always)*/ DREF DeeObject *a, DeeObject *b) {
	DREF SetDifference *result;
	result = DeeObject_MALLOC(SetDifference);
	if unlikely(!result)
		goto err;
	Dee_Incref(b);
	result->sd_a = a; /* Inherited */
	result->sd_b = b;
	DeeObject_Init(result, &SetDifference_Type);
	return result;
err:
	Dee_Decref(a);
	return NULL;
}

#define SetDifference_New_inherit_b(a, b) \
	_SetDifference_New_inherit_b((DeeObject *)Dee_REQUIRES_OBJECT(a), (DREF DeeObject *)Dee_REQUIRES_OBJECT(b))
LOCAL WUNUSED NONNULL((1, 2)) DREF SetDifference *DCALL
_SetDifference_New_inherit_b(DeeObject *a, /*inherit(always)*/ DREF DeeObject *b) {
	DREF SetDifference *result;
	result = DeeObject_MALLOC(SetDifference);
	if unlikely(!result)
		goto err;
	Dee_Incref(a);
	result->sd_a = a;
	result->sd_b = b; /* Inherited */
	DeeObject_Init(result, &SetDifference_Type);
	return result;
err:
	Dee_Decref(b);
	return NULL;
}

#define SetDifference_New_inherit_ab(a, b) \
	_SetDifference_New_inherit_ab((DREF DeeObject *)Dee_REQUIRES_OBJECT(a), (DREF DeeObject *)Dee_REQUIRES_OBJECT(b))
LOCAL WUNUSED NONNULL((1, 2)) DREF SetDifference *DCALL
_SetDifference_New_inherit_ab(/*inherit(always)*/ DREF DeeObject *a,
                              /*inherit(always)*/ DREF DeeObject *b) {
	DREF SetDifference *result;
	result = DeeObject_MALLOC(SetDifference);
	if unlikely(!result)
		goto err;
	result->sd_a = a; /* Inherited */
	result->sd_b = b; /* Inherited */
	DeeObject_Init(result, &SetDifference_Type);
	return result;
err:
	Dee_Decref(b);
	Dee_Decref(a);
	return NULL;
}





#define SetSymmetricDifference_New(a, b) \
	_SetSymmetricDifference_New((DeeObject *)Dee_REQUIRES_OBJECT(a), (DeeObject *)Dee_REQUIRES_OBJECT(b))
LOCAL WUNUSED NONNULL((1, 2)) DREF SetSymmetricDifference *DCALL
_SetSymmetricDifference_New(DeeObject *a, DeeObject *b) {
	DREF SetSymmetricDifference *result;
	result = DeeObject_MALLOC(SetSymmetricDifference);
	if unlikely(!result)
		goto err;
	Dee_Incref(a);
	Dee_Incref(b);
	result->ssd_a = a;
	result->ssd_b = b;
	DeeObject_Init(result, &SetSymmetricDifference_Type);
	return result;
err:
	return NULL;
}

#define SetSymmetricDifference_New_inherit_a(a, b) \
	_SetSymmetricDifference_New_inherit_a((DREF DeeObject *)Dee_REQUIRES_OBJECT(a), (DeeObject *)Dee_REQUIRES_OBJECT(b))
LOCAL WUNUSED NONNULL((1, 2)) DREF SetSymmetricDifference *DCALL
_SetSymmetricDifference_New_inherit_a(/*inherit(always)*/ DREF DeeObject *a, DeeObject *b) {
	DREF SetSymmetricDifference *result;
	result = DeeObject_MALLOC(SetSymmetricDifference);
	if unlikely(!result)
		goto err;
	Dee_Incref(b);
	result->ssd_a = a; /* Inherited */
	result->ssd_b = b;
	DeeObject_Init(result, &SetSymmetricDifference_Type);
	return result;
err:
	Dee_Decref(a);
	return NULL;
}

#define SetSymmetricDifference_New_inherit_b(a, b) \
	_SetSymmetricDifference_New_inherit_b((DeeObject *)Dee_REQUIRES_OBJECT(a), (DREF DeeObject *)Dee_REQUIRES_OBJECT(b))
LOCAL WUNUSED NONNULL((1, 2)) DREF SetSymmetricDifference *DCALL
_SetSymmetricDifference_New_inherit_b(DeeObject *a, /*inherit(always)*/ DREF DeeObject *b) {
	DREF SetSymmetricDifference *result;
	result = DeeObject_MALLOC(SetSymmetricDifference);
	if unlikely(!result)
		goto err;
	Dee_Incref(a);
	result->ssd_a = a;
	result->ssd_b = b; /* Inherited */
	DeeObject_Init(result, &SetSymmetricDifference_Type);
	return result;
err:
	Dee_Decref(b);
	return NULL;
}

#define SetSymmetricDifference_New_inherit_ab(a, b) \
	_SetSymmetricDifference_New_inherit_ab((DREF DeeObject *)Dee_REQUIRES_OBJECT(a), (DREF DeeObject *)Dee_REQUIRES_OBJECT(b))
LOCAL WUNUSED NONNULL((1, 2)) DREF SetSymmetricDifference *DCALL
_SetSymmetricDifference_New_inherit_ab(/*inherit(always)*/ DREF DeeObject *a,
                                       /*inherit(always)*/ DREF DeeObject *b) {
	DREF SetSymmetricDifference *result;
	result = DeeObject_MALLOC(SetSymmetricDifference);
	if unlikely(!result)
		goto err;
	result->ssd_a = a; /* Inherited */
	result->ssd_b = b; /* Inherited */
	DeeObject_Init(result, &SetSymmetricDifference_Type);
	return result;
err:
	Dee_Decref(b);
	Dee_Decref(a);
	return NULL;
}




/*INTDEF WUNUSED NONNULL((1, 2)) int DCALL SetUnion_NonEmpty(DeeObject *a, DeeObject *b);*/ /* !!a || !!b */
INTDEF WUNUSED NONNULL((1, 2)) int DCALL SetIntersection_NonEmpty(DeeObject *a, DeeObject *b);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL SetDifference_NonEmpty(DeeObject *a, DeeObject *b);
/*INTDEF WUNUSED NONNULL((1, 2)) int DCALL SetSymmetricDifference_NonEmpty(DeeObject *a, DeeObject *b);*/ /* a != b */


DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_DEFAULT_SETS_H */
