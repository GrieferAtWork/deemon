/* Copyright (c) 2018 Griefer@Work                                            *
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
#ifndef GUARD_DEEMON_OBJECTS_SEQ_SET_H
#define GUARD_DEEMON_OBJECTS_SEQ_SET_H 1

#include <deemon/api.h>
#include <deemon/object.h>
#ifndef CONFIG_NO_THREADS
#include <deemon/util/rwlock.h>
#endif

DECL_BEGIN

/* Set proxy types used to implement set operations on the C-level. */

typedef struct {
    OBJECT_HEAD
    DREF DeeObject *su_a; /* [1..1][const] The first set of the union. */
    DREF DeeObject *su_b; /* [1..1][const] The second set of the union. */
} SetUnion;

typedef struct {
    OBJECT_HEAD
    DREF SetUnion   *sui_union; /* [1..1][const] The underlying union-set. */
    DREF DeeObject  *sui_iter;  /* [1..1][lock(sui_lock)] The current iterator. */
#ifndef CONFIG_NO_THREADS
    rwlock_t         sui_lock;  /* Lock for `sui_iter' and `sui_in2nd' */
#endif
    bool             sui_in2nd; /* [lock(sui_lock)] The second set is being iterated. */
} SetUnionIterator;

INTDEF DeeTypeObject SetUnion_Type;
INTDEF DeeTypeObject SetUnionIterator_Type;



typedef struct {
    OBJECT_HEAD
    DREF DeeObject *si_a; /* [1..1][const] The first set of the intersection. */
    DREF DeeObject *si_b; /* [1..1][const] The second set of the intersection. */
} SetIntersection;

typedef struct {
    OBJECT_HEAD
    DREF SetIntersection *sii_intersect; /* [1..1][const] The underlying intersection-set. */
    DREF DeeObject       *sii_iter;      /* [1..1][const] An iterator for `sii_intersect->si_a' */
    DREF DeeObject       *sii_other;     /* [1..1][const][== sii_intersect->si_b]. */
} SetIntersectionIterator;

INTDEF DeeTypeObject SetIntersection_Type;
INTDEF DeeTypeObject SetIntersectionIterator_Type;



typedef struct {
    OBJECT_HEAD
    DREF DeeObject *sd_a; /* [1..1][const] The primary set. */
    DREF DeeObject *sd_b; /* [1..1][const] The set of objects removed from `sd_a'. */
} SetDifference;

typedef struct {
    OBJECT_HEAD
    DREF SetDifference *sdi_diff;  /* [1..1][const] The underlying difference-set. */
    DREF DeeObject     *sdi_iter;  /* [1..1][const] An iterator for `sdi_diff->sd_a' */
    DREF DeeObject     *sdi_other; /* [1..1][const][== sdi_diff->sd_b]. */
} SetDifferenceIterator;

INTDEF DeeTypeObject SetDifference_Type;
INTDEF DeeTypeObject SetDifferenceIterator_Type;



typedef struct {
    OBJECT_HEAD
    DREF DeeObject *ssd_a; /* [1..1][const] The first set. */
    DREF DeeObject *ssd_b; /* [1..1][const] The second set. */
} SetSymmetricDifference;

typedef struct {
    OBJECT_HEAD
    DREF SetSymmetricDifference *ssd_set;   /* [1..1][const] The underlying set. */
    DREF DeeObject              *ssd_iter;  /* [1..1][lock(sui_lock)] The current iterator. */
#ifndef CONFIG_NO_THREADS
    rwlock_t                     ssd_lock;  /* Lock for `ssd_iter' and `ssd_in2nd' */
#endif
    bool                         ssd_in2nd; /* [lock(sui_lock)] The second set is being iterated. */
} SetSymmetricDifferenceIterator;

INTDEF DeeTypeObject SetSymmetricDifference_Type;
INTDEF DeeTypeObject SetSymmetricDifferenceIterator_Type;



DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_SET_H */
