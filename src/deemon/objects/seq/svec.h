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
#ifndef GUARD_DEEMON_OBJECTS_SEQ_SVEC_H
#define GUARD_DEEMON_OBJECTS_SEQ_SVEC_H 1

#include <deemon/api.h>
#include <deemon/object.h>
#include <deemon/seq.h>
#include <deemon/util/lock.h>

DECL_BEGIN

/* A type `RefVector' that acts and works very much the same as `SharedVector',
 * but instead allows other objects to enumerate private vectors, such as the
 * global objects of modules, or their imports, as well as static variables of
 * code objects, etc. etc. etc... */
typedef struct {
	OBJECT_HEAD
	DREF DeeObject      *rv_owner;    /* [1..1] The object that is actually owning the vector. */
	size_t               rv_length;   /* [const] The number of items in this vector. */
	DREF DeeObject     **rv_vector;   /* [0..1][lock(*rv_plock)][0..rv_length][lock(*rv_plock)][const]
	                                   * The vector of objects that is being referenced.
	                                   * NOTE: Elements of this vector must not be changed. */
#ifndef CONFIG_NO_THREADS
	Dee_atomic_rwlock_t *rv_plock;    /* [0..1][const] An optional lock that must be held when accessing the list.
	                                   * Also: when non-NULL, items of the vector can be modified. */
#define RefVector_IsWritable(self) ((self)->rv_plock != NULL)
#else /* !CONFIG_NO_THREADS */
	bool                 rv_writable; /* [const] Set to true if items of the vector can be modified. */
#define RefVector_IsWritable(self) ((self)->rv_writable)
#endif /* CONFIG_NO_THREADS */
} RefVector;

typedef struct {
	OBJECT_HEAD
	DREF RefVector  *rvi_vector; /* [1..1][const] The underlying vector being iterated. */
	DREF DeeObject **rvi_pos;    /* [0..1][lock(*rvi_vector->rv_plock)][1..1][in(rvi_vector->rv_vector)][atomic]
	                              * The current iterator position. */
} RefVectorIterator;


INTDEF DeeTypeObject RefVector_Type;
INTDEF DeeTypeObject RefVectorIterator_Type;


typedef struct {
	OBJECT_HEAD
#ifndef CONFIG_NO_THREADS
	Dee_atomic_rwlock_t    sv_lock;   /* Lock for this shared-vector. */
#endif /* !CONFIG_NO_THREADS */
	size_t                 sv_length; /* [lock(sv_lock)] The number of items in this vector. */
	DREF DeeObject *const *sv_vector; /* [1..1][const][0..sv_length][lock(sv_lock)][owned]
	                                   * The vector of objects that is being referenced.
	                                   * NOTE: Elements of this vector must not be changed. */
} SharedVector;

INTDEF DeeTypeObject SharedVector_Type;


typedef struct {
	OBJECT_HEAD
	DREF SharedVector *si_seq;   /* [1..1][const] The shared-vector that is being iterated. */
	size_t             si_index; /* [atomic] The current sequence index.
	                              * Should this value be `>= si_seq->sv_length',
	                              * then the iterator has been exhausted. */
} SharedVectorIterator;

INTDEF DeeTypeObject SharedVectorIterator_Type;


DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_SVEC_H */
