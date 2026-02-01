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
/*!export **/
/*!export DeeList_**/
#ifndef GUARD_DEEMON_LIST_H
#define GUARD_DEEMON_LIST_H 1 /*!export-*/

#include "api.h"

#include "object.h"          /* DeeObject_NewDefault, Dee_DecrefDokill */
#include "types.h"           /* DREF, DeeObject, DeeObject_InstanceOf, DeeObject_InstanceOfExact, DeeTypeObject, Dee_AsObject, Dee_OBJECT_HEAD, Dee_WEAKREF_SUPPORT, Dee_ssize_t */
#include "util/atomic.h"     /* Dee_atomic_read */
#include "util/lock.h"       /* Dee_atomic_rwlock_* */
#include "util/objectlist.h" /* Dee_OBJECTLIST_HAVE_ELEMA, Dee_objectlist, Dee_objectlist_getalloc, _Dee_objectlist_setalloc */

#include <stdbool.h> /* bool */
#include <stddef.h>  /* size_t */

#ifndef __INTELLISENSE__
#include "gc.h" /* DeeGC_Track */
#endif /* !__INTELLISENSE__ */

DECL_BEGIN

typedef struct Dee_list_object {
	Dee_OBJECT_HEAD /* GC Object */
	struct Dee_objectlist l_list; /* [owned][lock(l_lock)] Object list. */
#define DeeList_GetAlloc(self)     Dee_objectlist_getalloc(&(self)->l_list)
#define _DeeList_SetAlloc(self, v) _Dee_objectlist_setalloc(&(self)->l_list, v)
#ifdef Dee_OBJECTLIST_HAVE_ELEMA
#define DeeList_GetAlloc_ATOMIC(ob) Dee_atomic_read(&(ob)->l_list.ol_elema)
#endif /* Dee_OBJECTLIST_HAVE_ELEMA */
#ifndef CONFIG_NO_THREADS
	Dee_atomic_rwlock_t l_lock;  /* Lock used for accessing this list. */
#endif /* !CONFIG_NO_THREADS */
	Dee_WEAKREF_SUPPORT
} DeeListObject;

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

#define DeeList_Check(x)      DeeObject_InstanceOf(x, &DeeList_Type)
#define DeeList_CheckExact(x) DeeObject_InstanceOfExact(x, &DeeList_Type)

DDATDEF DeeTypeObject DeeList_Type;
#define DeeList_Destroy(self) Dee_DecrefDokill(self)

/* Create a new list object from a vector. */
DFUNDEF WUNUSED DREF DeeObject *DCALL
DeeList_NewVector(size_t objc, DeeObject *const *objv);
DFUNDEF WUNUSED DREF DeeObject *DCALL
DeeList_NewVectorInherited(size_t objc, /*inherit(on_success)*/ DREF DeeObject *const *objv);

/* Inherit the entire vector, which must have been allocated using `Dee_Malloc()' and friends. */
DFUNDEF WUNUSED DREF DeeObject *DCALL
DeeList_NewVectorInheritedHeap(/*inherit(on_success)*/ DREF DeeObject **objv,
                               size_t objc, size_t obja);
#ifndef Dee_OBJECTLIST_HAVE_ELEMA
DFUNDEF WUNUSED DREF DeeObject *DCALL
DeeList_NewVectorInheritedHeap2(/*inherit(on_success)*/ DREF DeeObject **objv,
                                size_t objc);
#endif /* !Dee_OBJECTLIST_HAVE_ELEMA */

/* Create a new List object. */
#define DeeList_New() DeeObject_NewDefault(&DeeList_Type)
DFUNDEF WUNUSED DREF DeeObject *DCALL DeeList_NewWithHint(size_t n_prealloc);
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeList_FromSequence(DeeObject *__restrict self);
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeList_FromSequenceInheritedOnSuccess(/*inherit(on_success)*/ DREF DeeObject *__restrict self);
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL DeeList_FromTuple(DeeObject *__restrict self);

/* WARNING: The caller must start gc-tracking the list once elements are initialized. */
DFUNDEF WUNUSED DREF DeeListObject *DCALL DeeList_NewUninitialized(size_t n_elem);
DFUNDEF NONNULL((1)) void DCALL DeeList_FreeUninitialized(DREF DeeListObject *__restrict self);

/* Finalize an List object originally created with `DeeList_NewUninitialized()' */
#ifdef __INTELLISENSE__
DFUNDEF WUNUSED DREF DeeObject *DCALL
DeeList_FinalizeUninitialized(/*inherit(always)*/ DREF DeeListObject *__restrict self);
#else /* __INTELLISENSE__ */
#define DeeList_FinalizeUninitialized(self) DeeGC_Track(Dee_AsObject(self))
#endif /* !__INTELLISENSE__ */


/* Concat a list and some generic sequence,
 * inheriting a reference from `self' in the process. */
DFUNDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
DeeList_ConcatInherited(/*inherit(always)*/ DREF DeeObject *self, DeeObject *sequence);
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeList_ExtendInherited(/*inherit(always)*/ DREF DeeObject *self, size_t argc,
                        /*inherit(always)*/ DREF DeeObject *const *argv);

DFUNDEF WUNUSED NONNULL((1)) int DCALL
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

/* Sort the given list ascendingly, or according to `key'
 * To use default sorting, pass `Dee_None' for `key' */
DFUNDEF WUNUSED NONNULL((1, 4)) int DCALL
DeeList_Sort(DeeObject *self, size_t start, size_t end, DeeObject *key);

/* Reverse the order of the elements of `self' */
DFUNDEF NONNULL((1)) void DCALL
DeeList_Reverse(DeeObject *__restrict self, size_t start, size_t end);

/* Remove all items matching `!!should(item)'
 * @return: * : The number of removed items.
 * @return: -1: An error occurred. */
DFUNDEF WUNUSED NONNULL((1, 2)) size_t DCALL
DeeList_RemoveIf(DeeObject *self, DeeObject *should,
                 size_t start, size_t end, size_t max);

/* Resize `self' to have a length of `newsize'.
 * If the size increases, use `filler' for new items.
 * @return: 0 : Success.
 * @return: -1: Error. */
DFUNDEF WUNUSED NONNULL((1, 3)) int DCALL
DeeList_Resize(DeeObject *self, size_t newsize, DeeObject *filler);

/* Append objects to a given list. */
DFUNDEF WUNUSED NONNULL((1, 2)) int DCALL DeeList_Append(DeeObject *self, DeeObject *elem);
DFUNDEF WUNUSED NONNULL((1, 2)) int DCALL DeeList_AppendSequence(DeeObject *self, DeeObject *sequence);
DFUNDEF WUNUSED NONNULL((1)) int DCALL DeeList_AppendVector(DeeObject *self, size_t objc, DeeObject *const *objv);

/* Insert objects into a given list. */
DFUNDEF WUNUSED NONNULL((1, 3)) int DCALL DeeList_Insert(DeeObject *self, size_t index, DeeObject *elem);
DFUNDEF WUNUSED NONNULL((1, 3)) int DCALL DeeList_InsertSequence(DeeObject *self, size_t index, DeeObject *sequence);
DFUNDEF WUNUSED NONNULL((1)) int DCALL DeeList_InsertVector(DeeObject *self, size_t index, size_t objc, DeeObject *const *objv);


#ifndef __INTELLISENSE__
#ifndef __NO_builtin_expect
#define DeeList_Append(self, elem)                    __builtin_expect(DeeList_Append(self, elem), 0)
#define DeeList_AppendSequence(self, sequence)        __builtin_expect(DeeList_AppendSequence(self, sequence), 0)
#define DeeList_AppendVector(self, objc, objv)        __builtin_expect(DeeList_AppendVector(self, objc, objv), 0)
#define DeeList_Insert(self, index, elem)             __builtin_expect(DeeList_Insert(self, index, elem), 0)
#define DeeList_InsertSequence(self, index, sequence) __builtin_expect(DeeList_InsertSequence(self, index, sequence), 0)
#define DeeList_InsertVector(self, index, objc, objv) __builtin_expect(DeeList_InsertVector(self, index, objc, objv), 0)
#endif /* !__NO_builtin_expect */
#endif /* !__INTELLISENSE__ */

/* Pack the given Dee_objectlist into a List.
 * Upon success, `self' will have been finalized. */
#ifdef __INTELLISENSE__
extern WUNUSED NONNULL((1)) DREF DeeObject *DCALL
Dee_objectlist_packlist(struct Dee_objectlist *__restrict self);
#elif defined(Dee_OBJECTLIST_HAVE_ELEMA)
#define Dee_objectlist_packlist(self) \
	DeeList_NewVectorInheritedHeap((self)->ol_elemv, (self)->ol_elemc, (self)->ol_elema)
#else /* ... */
#define Dee_objectlist_packlist(self) \
	DeeList_NewVectorInheritedHeap2((self)->ol_elemv, (self)->ol_elemc)
#endif /* !... */

DECL_END

#endif /* !GUARD_DEEMON_LIST_H */
