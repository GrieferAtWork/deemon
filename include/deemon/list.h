/* Copyright (c) 2018-2024 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2024 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_LIST_H
#define GUARD_DEEMON_LIST_H 1

#include "api.h"

#include "alloc.h" /* Dee_MallocUsableSize */
#include "object.h"
#include "util/atomic.h"
#include "util/lock.h"
#include "util/objectlist.h"
/**/

#include <stdarg.h>
#include <stddef.h>

DECL_BEGIN

#ifdef DEE_SOURCE
#define Dee_list_object list_object
#endif /* DEE_SOURCE */

typedef struct Dee_list_object DeeListObject;

struct Dee_list_object {
	Dee_OBJECT_HEAD /* GC Object */
	struct Dee_objectlist l_list; /* [owned][lock(l_lock)] Object list. */
#define DeeList_GetAlloc(self)     Dee_objectlist_getalloc(&(self)->l_list)
#define _DeeList_SetAlloc(self, v) _Dee_objectlist_setalloc(&(self)->l_list, v)
#ifdef DEE_OBJECTLIST_HAVE_ELEMA
#define DeeList_GetAlloc_ATOMIC(ob) Dee_atomic_read(&(ob)->l_list.ol_elema)
#endif /* DEE_OBJECTLIST_HAVE_ELEMA */
#ifndef CONFIG_NO_THREADS
	Dee_atomic_rwlock_t l_lock;  /* Lock used for accessing this list. */
#endif /* !CONFIG_NO_THREADS */
	Dee_WEAKREF_SUPPORT
};

#define DeeList_LockReading(self)    Dee_atomic_rwlock_reading(&(self)->l_lock)
#define DeeList_LockWriting(self)    Dee_atomic_rwlock_writing(&(self)->l_lock)
#define DeeList_LockTryRead(self)    Dee_atomic_rwlock_tryread(&(self)->l_lock)
#define DeeList_LockTryWrite(self)   Dee_atomic_rwlock_trywrite(&(self)->l_lock)
#define DeeList_LockRead(self)       Dee_atomic_rwlock_read(&(self)->l_lock)
#define DeeList_LockWrite(self)      Dee_atomic_rwlock_write(&(self)->l_lock)
#define DeeList_LockTryUpgrade(self) Dee_atomic_rwlock_tryupgrade(&(self)->l_lock)
#define DeeList_LockUpgrade(self)    Dee_atomic_rwlock_upgrade(&(self)->l_lock)
#define DeeList_LockDowngrade(self)  Dee_atomic_rwlock_downgrade(&(self)->l_lock)
#define DeeList_LockEndWrite(self)   Dee_atomic_rwlock_endwrite(&(self)->l_lock)
#define DeeList_LockEndRead(self)    Dee_atomic_rwlock_endread(&(self)->l_lock)
#define DeeList_LockEnd(self)        Dee_atomic_rwlock_end(&(self)->l_lock)
#define DeeList_LockWrite2(a, b)     Dee_atomic_rwlock_write_2(&(a)->l_lock, &(b)->l_lock)

#define DeeList_IsEmpty(ob)     ((ob)->l_list.ol_elemc == 0)
#define DeeList_SIZE(ob)        (ob)->l_list.ol_elemc
#define DeeList_SIZE_ATOMIC(ob) Dee_atomic_read(&(ob)->l_list.ol_elemc)
#define DeeList_ELEM(ob)        (ob)->l_list.ol_elemv
#define DeeList_GET(ob, i)      (ob)->l_list.ol_elemv[i]
#define DeeList_SET(ob, i, v)   (void)((ob)->l_list.ol_elemv[i] = (v))

#define DeeList_Check(x)       DeeObject_InstanceOf(x, &DeeList_Type)
#define DeeList_CheckExact(x)  DeeObject_InstanceOfExact(x, &DeeList_Type)

DDATDEF DeeTypeObject DeeList_Type;

/* Create a new list object from a vector. */
DFUNDEF WUNUSED DREF DeeObject *DCALL
DeeList_NewVector(size_t objc, DeeObject *const *objv);
DFUNDEF WUNUSED DREF DeeObject *DCALL
DeeList_NewVectorInherited(size_t objc, /*inherit(on_success)*/ DREF DeeObject *const *objv);

/* Inherit the entire vector, which must have been allocated using `Dee_Malloc()' and friends. */
DFUNDEF WUNUSED DREF DeeObject *DCALL
DeeList_NewVectorInheritedHeap(/*inherit(on_success)*/ DREF DeeObject **objv,
                               size_t objc, size_t obja);
#ifndef DEE_OBJECTLIST_HAVE_ELEMA
DFUNDEF WUNUSED DREF DeeObject *DCALL
DeeList_NewVectorInheritedHeap2(/*inherit(on_success)*/ DREF DeeObject **objv,
                                size_t objc);
#endif /* !DEE_OBJECTLIST_HAVE_ELEMA */

/* Create a new List object. */
#define DeeList_New() DeeObject_NewDefault(&DeeList_Type)
DFUNDEF WUNUSED DREF DeeObject *DCALL DeeList_NewWithHint(size_t n_prealloc);
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeList_FromSequence(DeeObject *__restrict self);
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeList_FromTuple(DeeObject *__restrict self);

/* WARNING: The caller must start gc-tracking the list once elements are initialized. */
DFUNDEF WUNUSED DREF DeeListObject *DCALL DeeList_NewUninitialized(size_t n_elem);
DFUNDEF NONNULL((1)) void DCALL DeeList_FreeUninitialized(DREF DeeListObject *__restrict self);

/* Finalize an List object originally created with `DeeList_NewUninitialized()' */
#ifdef __INTELLISENSE__
DFUNDEF WUNUSED DREF DeeObject *DCALL DeeList_FinalizeUninitialized(/*inherit(always)*/ DREF DeeListObject *__restrict self);
#else /* __INTELLISENSE__ */
#define DeeList_FinalizeUninitialized(self) DeeGC_Track((DeeObject *)(self))
#endif /* !__INTELLISENSE__ */


/* Concat a list and some generic sequence,
 * inheriting a reference from `self' in the process. */
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeList_ConcatInherited(/*inherit(on_success)*/ DREF DeeObject *self,
                        DeeObject *sequence);
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeList_ExtendInherited(/*inherit(on_success)*/ DREF DeeObject *self, size_t argc,
                        /*inherit(on_success)*/ DREF DeeObject *const *argv);

/* @return: * : The actual number of deleted items.
 * @return: (size_t)-1: Error. */
DFUNDEF WUNUSED NONNULL((1)) size_t DCALL
DeeList_Erase(DeeObject *__restrict self,
              size_t index, size_t count);

/* @return: * :   The popped element.
 * @return: NULL: The given index was out-of-bounds and an IndexError was thrown. */
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeList_Pop(DeeObject *__restrict self, Dee_ssize_t index);

/* Clear the given list.
 * Returns `true' if the list wasn't empty before. */
DFUNDEF NONNULL((1)) bool DCALL
DeeList_Clear(DeeObject *__restrict self);

/* Sort the given list ascendingly, or according to `key' */
DFUNDEF WUNUSED NONNULL((1)) int DCALL
DeeList_Sort(DeeObject *self, DeeObject *key);

/* Reverse the order of the elements of `self' */
DFUNDEF NONNULL((1)) void DCALL
DeeList_Reverse(DeeObject *__restrict self);

/* Remove all items matching `!!should(item)'
 * @return: * : The number of removed items.
 * @return: -1: An error occurred. */
DFUNDEF WUNUSED NONNULL((1, 4)) size_t DCALL
DeeList_RemoveIf(DeeObject *self, size_t start,
                 size_t end, DeeObject *should);

/* Resize `self' to have a length of `newsize'.
 * If the size increases, use `filler' for new items.
 * @return: 0 : Success.
 * @return: -1: Error. */
DFUNDEF WUNUSED NONNULL((1, 3)) int DCALL
DeeList_Resize(DeeObject *self, size_t newsize, DeeObject *filler);

/* Append objects to a given list. */
DFUNDEF WUNUSED NONNULL((1, 2)) int DCALL DeeList_Append(DeeObject *self, DeeObject *elem);
DFUNDEF WUNUSED NONNULL((1, 2)) int DCALL DeeList_AppendIterator(DeeObject *self, DeeObject *iterator);
DFUNDEF WUNUSED NONNULL((1, 2)) int DCALL DeeList_AppendSequence(DeeObject *self, DeeObject *sequence);
DFUNDEF WUNUSED NONNULL((1)) int DCALL DeeList_AppendVector(DeeObject *self, size_t objc, DeeObject *const *objv);

/* Insert objects into a given list. */
DFUNDEF WUNUSED NONNULL((1, 3)) int DCALL DeeList_Insert(DeeObject *self, size_t index, DeeObject *elem);
DFUNDEF WUNUSED NONNULL((1, 3)) int DCALL DeeList_InsertIterator(DeeObject *self, size_t index, DeeObject *iterator);
DFUNDEF WUNUSED NONNULL((1, 3)) int DCALL DeeList_InsertSequence(DeeObject *self, size_t index, DeeObject *sequence);
DFUNDEF WUNUSED NONNULL((1)) int DCALL DeeList_InsertVector(DeeObject *self, size_t index, size_t objc, DeeObject *const *objv);

#ifdef CONFIG_BUILDING_DEEMON
INTDEF WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
DeeList_PrintRepr(DeeObject *__restrict self,
                  Dee_formatprinter_t printer, void *arg);
#else /* CONFIG_BUILDING_DEEMON */
#define DeeList_PrintRepr(self, printer, arg) \
	DeeObject_PrintRepr(self, printer, arg)
#endif /* !CONFIG_BUILDING_DEEMON */


#ifndef __INTELLISENSE__
#ifndef __NO_builtin_expect
#define DeeList_Append(self, elem)                    __builtin_expect(DeeList_Append(self, elem), 0)
#define DeeList_AppendIterator(self, iterator)        __builtin_expect(DeeList_AppendIterator(self, iterator), 0)
#define DeeList_AppendSequence(self, sequence)        __builtin_expect(DeeList_AppendSequence(self, sequence), 0)
#define DeeList_AppendVector(self, objc, objv)        __builtin_expect(DeeList_AppendVector(self, objc, objv), 0)
#define DeeList_Insert(self, index, elem)             __builtin_expect(DeeList_Insert(self, index, elem), 0)
#define DeeList_InsertIterator(self, index, iterator) __builtin_expect(DeeList_InsertIterator(self, index, iterator), 0)
#define DeeList_InsertSequence(self, index, sequence) __builtin_expect(DeeList_InsertSequence(self, index, sequence), 0)
#define DeeList_InsertVector(self, index, objc, objv) __builtin_expect(DeeList_InsertVector(self, index, objc, objv), 0)
#endif /* !__NO_builtin_expect */
#endif /* !__INTELLISENSE__ */

DECL_END

#endif /* !GUARD_DEEMON_LIST_H */
