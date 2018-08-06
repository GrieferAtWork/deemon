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
#ifndef GUARD_DEEMON_CELL_H
#define GUARD_DEEMON_CELL_H 1

#include "api.h"
#include "object.h"
#ifndef CONFIG_NO_THREADS
#include "util/rwlock.h"
#endif /* !CONFIG_NO_THREADS */

DECL_BEGIN

typedef struct cell_object DeeCellObject;

struct cell_object {
    OBJECT_HEAD /* GC Object. */
    DREF DeeObject *c_item;  /* [0..1] The object contained within this cell. */
#ifndef CONFIG_NO_THREADS
    rwlock_t        c_lock; /* Lock used for accessing this cell. */
#endif /* !CONFIG_NO_THREADS */
};

#define DeeCell_Item(x) ((DeeCellObject *)(x))->c_item

#ifndef CONFIG_NO_THREADS
#define DeeCell_LockReading(x)    rwlock_reading(&((DeeCellObject *)(x))->c_lock)
#define DeeCell_LockWriting(x)    rwlock_writing(&((DeeCellObject *)(x))->c_lock)
#define DeeCell_LockTryread(x)    rwlock_tryread(&((DeeCellObject *)(x))->c_lock)
#define DeeCell_LockTrywrite(x)   rwlock_trywrite(&((DeeCellObject *)(x))->c_lock)
#define DeeCell_LockRead(x)       rwlock_read(&((DeeCellObject *)(x))->c_lock)
#define DeeCell_LockWrite(x)      rwlock_write(&((DeeCellObject *)(x))->c_lock)
#define DeeCell_LockTryUpgrade(x) rwlock_tryupgrade(&((DeeCellObject *)(x))->c_lock)
#define DeeCell_LockUpgrade(x)    rwlock_upgrade(&((DeeCellObject *)(x))->c_lock)
#define DeeCell_LockDowngrade(x)  rwlock_downgrade(&((DeeCellObject *)(x))->c_lock)
#define DeeCell_LockEndWrite(x)   rwlock_endwrite(&((DeeCellObject *)(x))->c_lock)
#define DeeCell_LockEndRead(x)    rwlock_endread(&((DeeCellObject *)(x))->c_lock)
#define DeeCell_LockEnd(x)        rwlock_end(&((DeeCellObject *)(x))->c_lock)
#else
#define DeeCell_LockReading(x)          1
#define DeeCell_LockWriting(x)          1
#define DeeCell_LockTryread(x)          1
#define DeeCell_LockTrywrite(x)         1
#define DeeCell_LockRead(x)       (void)0
#define DeeCell_LockWrite(x)      (void)0
#define DeeCell_LockTryUpgrade(x)       1
#define DeeCell_LockUpgrade(x)          1
#define DeeCell_LockDowngrade(x)  (void)0
#define DeeCell_LockEndWrite(x)   (void)0
#define DeeCell_LockEndRead(x)    (void)0
#define DeeCell_LockEnd(x)        (void)0
#endif


DDATDEF DeeTypeObject DeeCell_Type;
#define DeeCell_Check(x)      DeeObject_InstanceOf(x,&DeeCell_Type)
#define DeeCell_CheckExact(x) DeeObject_InstanceOfExact(x,&DeeCell_Type)


DFUNDEF DREF DeeObject *DCALL DeeCell_New(DeeObject *__restrict item);
#define DeeCell_NewEmpty() DeeObject_NewDefault(&DeeCell_Type)

/* Get/Del/Set the value associated with a given cell.
 * HINT:  These are the getset callbacks used for `cell.item' (or its deprecated name `cell.value').
 *        With that in mind, `DeeCell_Del()' and `DeeCell_Set()'
 *        always return `0' indicative of a successful callback.
 * NOTE: `DeeCell_Get' will return `NULL' and throw an `AttributeError' if the `self' is
 *        empty, whereas `DeeCell_TryGet()' will do the same, but never throw any error. */
DFUNDEF DREF DeeObject *DCALL DeeCell_TryGet(DeeObject *__restrict self);
DFUNDEF DREF DeeObject *DCALL DeeCell_Get(DeeObject *__restrict self);
DFUNDEF int DCALL DeeCell_Del(DeeObject *__restrict self);
DFUNDEF int DCALL DeeCell_Set(DeeObject *__restrict self, DeeObject *__restrict value);
/* Exchange the cell's value.
 * NOTE: `DeeCell_XchNonNull()' will only set the new value when the old was non-NULL. */
DFUNDEF DREF DeeObject *DCALL DeeCell_Xch(DeeObject *__restrict self, DeeObject *value);
DFUNDEF DREF DeeObject *DCALL DeeCell_XchNonNull(DeeObject *__restrict self, DeeObject *value);
/* Perform a compare-exchange, returning the old value of the cell. */
DFUNDEF DREF DeeObject *DCALL DeeCell_CmpXch(DeeObject *__restrict self,
                                             DeeObject *old_value,
                                             DeeObject *new_value);

DECL_END

#endif /* !GUARD_DEEMON_CELL_H */
