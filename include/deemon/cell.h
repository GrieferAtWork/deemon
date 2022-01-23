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
#ifndef GUARD_DEEMON_CELL_H
#define GUARD_DEEMON_CELL_H 1

#include "api.h"

#include "object.h"
#include "util/lock.h"

DECL_BEGIN

#ifdef DEE_SOURCE
#define Dee_cell_object cell_object
#endif /* DEE_SOURCE */

typedef struct Dee_cell_object DeeCellObject;

struct Dee_cell_object {
	Dee_OBJECT_HEAD /* GC Object. */
	DREF DeeObject     *c_item; /* [0..1] The object contained within this Cell. */
#ifndef CONFIG_NO_THREADS
	Dee_atomic_rwlock_t c_lock; /* Lock used for accessing this Cell. */
#endif /* !CONFIG_NO_THREADS */
};

#define DeeCell_Item(x)  ((DeeCellObject *)Dee_REQUIRES_OBJECT(x))->c_item
#ifdef CONFIG_NO_THREADS
#define DeeCell_Bound(x) (DeeCell_Item(x) != NULL)
#else /* CONFIG_NO_THREADS */
#define DeeCell_Bound(x) (__hybrid_atomic_load(DeeCell_Item(x), __ATOMIC_ACQUIRE) != NULL)
#endif /* !CONFIG_NO_THREADS */

#define DeeCell_LockReading(x)    Dee_atomic_rwlock_reading(&((DeeCellObject *)Dee_REQUIRES_OBJECT(x))->c_lock)
#define DeeCell_LockWriting(x)    Dee_atomic_rwlock_writing(&((DeeCellObject *)Dee_REQUIRES_OBJECT(x))->c_lock)
#define DeeCell_LockTryread(x)    Dee_atomic_rwlock_tryread(&((DeeCellObject *)Dee_REQUIRES_OBJECT(x))->c_lock)
#define DeeCell_LockTrywrite(x)   Dee_atomic_rwlock_trywrite(&((DeeCellObject *)Dee_REQUIRES_OBJECT(x))->c_lock)
#define DeeCell_LockRead(x)       Dee_atomic_rwlock_read(&((DeeCellObject *)Dee_REQUIRES_OBJECT(x))->c_lock)
#define DeeCell_LockWrite(x)      Dee_atomic_rwlock_write(&((DeeCellObject *)Dee_REQUIRES_OBJECT(x))->c_lock)
#define DeeCell_LockTryUpgrade(x) Dee_atomic_rwlock_tryupgrade(&((DeeCellObject *)Dee_REQUIRES_OBJECT(x))->c_lock)
#define DeeCell_LockUpgrade(x)    Dee_atomic_rwlock_upgrade(&((DeeCellObject *)Dee_REQUIRES_OBJECT(x))->c_lock)
#define DeeCell_LockDowngrade(x)  Dee_atomic_rwlock_downgrade(&((DeeCellObject *)Dee_REQUIRES_OBJECT(x))->c_lock)
#define DeeCell_LockEndWrite(x)   Dee_atomic_rwlock_endwrite(&((DeeCellObject *)Dee_REQUIRES_OBJECT(x))->c_lock)
#define DeeCell_LockEndRead(x)    Dee_atomic_rwlock_endread(&((DeeCellObject *)Dee_REQUIRES_OBJECT(x))->c_lock)
#define DeeCell_LockEnd(x)        Dee_atomic_rwlock_end(&((DeeCellObject *)Dee_REQUIRES_OBJECT(x))->c_lock)


DDATDEF DeeTypeObject DeeCell_Type;
#define DeeCell_Check(x)      DeeObject_InstanceOf(x, &DeeCell_Type)
#define DeeCell_CheckExact(x) DeeObject_InstanceOfExact(x, &DeeCell_Type)

/* Construct a new Cell object. */
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeCell_New(DeeObject *__restrict item);
#define DeeCell_NewEmpty() DeeObject_NewDefault(&DeeCell_Type)

/* Get/Del/Set the value associated with a given Cell.
 * HINT:  These are the getset callbacks used for `Cell.item' (or its deprecated name `Cell.value').
 *        With that in mind, `DeeCell_Del()' and `DeeCell_Set()'
 *        always return `0' indicative of a successful callback.
 * NOTE: `DeeCell_Get' will return `NULL' and throw an `AttributeError' if the `self' is
 *        empty, whereas `DeeCell_TryGet()' will do the same, but never throw any error. */
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeCell_TryGet(DeeObject *__restrict self);
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeCell_Get(DeeObject *__restrict self);
DFUNDEF WUNUSED NONNULL((1)) int DCALL DeeCell_Del(DeeObject *__restrict self);
DFUNDEF WUNUSED NONNULL((1)) int DCALL DeeCell_Set(DeeObject *self, DeeObject *value);

/* Exchange the Cell's value.
 * NOTE: `DeeCell_XchNonNull()' will only set the new value when the old was non-NULL. */
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeCell_Xch(DeeObject *self, DeeObject *value);
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeCell_XchNonNull(DeeObject *self, DeeObject *value);

/* Perform a compare-exchange, returning the old value of the Cell. */
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeCell_CmpXch(DeeObject *self,
               DeeObject *old_value,
               DeeObject *new_value);

DECL_END

#endif /* !GUARD_DEEMON_CELL_H */
