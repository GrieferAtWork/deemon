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
#ifndef GUARD_DEEMON_LIST_H
#define GUARD_DEEMON_LIST_H 1

#include "api.h"
#include "object.h"
#ifndef CONFIG_NO_THREADS
#include "util/rwlock.h"
#endif /* !CONFIG_NO_THREADS */
#include <stddef.h>
#include <stdarg.h>

DECL_BEGIN

typedef struct list_object DeeListObject;

struct list_object {
    /* WARNING: Changes must be mirrored in `/src/deemon/execute/asm/exec-386.S' */
    OBJECT_HEAD /* GC Object */
    size_t           l_alloc; /* [lock(l_lock)][>= l_size] Allocated list size. */
    size_t           l_size;  /* [lock(l_lock)] List size. */
    DREF DeeObject **l_elem;  /* [1..1][0..l_size|ALLOC(l_alloc)][owned][lock(l_lock)] List elements. */
#ifndef CONFIG_NO_THREADS
    rwlock_t         l_lock;  /* Lock used for accessing this list. */
#endif /* !CONFIG_NO_THREADS */
};

#ifndef CONFIG_NO_THREADS
#define DeeList_LockReading(x)    rwlock_reading(&((DeeListObject *)REQUIRES_OBJECT(x))->l_lock)
#define DeeList_LockWriting(x)    rwlock_writing(&((DeeListObject *)REQUIRES_OBJECT(x))->l_lock)
#define DeeList_LockTryread(x)    rwlock_tryread(&((DeeListObject *)REQUIRES_OBJECT(x))->l_lock)
#define DeeList_LockTrywrite(x)   rwlock_trywrite(&((DeeListObject *)REQUIRES_OBJECT(x))->l_lock)
#define DeeList_LockRead(x)       rwlock_read(&((DeeListObject *)REQUIRES_OBJECT(x))->l_lock)
#define DeeList_LockWrite(x)      rwlock_write(&((DeeListObject *)REQUIRES_OBJECT(x))->l_lock)
#define DeeList_LockTryUpgrade(x) rwlock_tryupgrade(&((DeeListObject *)REQUIRES_OBJECT(x))->l_lock)
#define DeeList_LockUpgrade(x)    rwlock_upgrade(&((DeeListObject *)REQUIRES_OBJECT(x))->l_lock)
#define DeeList_LockDowngrade(x)  rwlock_downgrade(&((DeeListObject *)REQUIRES_OBJECT(x))->l_lock)
#define DeeList_LockEndWrite(x)   rwlock_endwrite(&((DeeListObject *)REQUIRES_OBJECT(x))->l_lock)
#define DeeList_LockEndRead(x)    rwlock_endread(&((DeeListObject *)REQUIRES_OBJECT(x))->l_lock)
#define DeeList_LockEnd(x)        rwlock_end(&((DeeListObject *)REQUIRES_OBJECT(x))->l_lock)
#else
#define DeeList_LockReading(x)          1
#define DeeList_LockWriting(x)          1
#define DeeList_LockTryread(x)          1
#define DeeList_LockTrywrite(x)         1
#define DeeList_LockRead(x)       (void)0
#define DeeList_LockWrite(x)      (void)0
#define DeeList_LockTryUpgrade(x)       1
#define DeeList_LockUpgrade(x)          1
#define DeeList_LockDowngrade(x)  (void)0
#define DeeList_LockEndWrite(x)   (void)0
#define DeeList_LockEndRead(x)    (void)0
#define DeeList_LockEnd(x)        (void)0
#endif

#define DeeList_IsEmpty(ob)                (!DeeList_SIZE(ob))
#define DeeList_CAPACITY(ob)               ((DeeListObject *)REQUIRES_OBJECT(ob))->l_alloc
#define DeeList_SIZE(ob)                   ((DeeListObject *)REQUIRES_OBJECT(ob))->l_size
#define DeeList_ELEM(ob)                   ((DeeListObject *)REQUIRES_OBJECT(ob))->l_elem
#define DeeList_GET(ob,i)                  ((DeeListObject *)REQUIRES_OBJECT(ob))->l_elem[i]
#define DeeList_SET(ob,i,v)                ((DeeListObject *)REQUIRES_OBJECT(ob))->l_elem[i]=(v)

#define DeeList_Check(x)       DeeObject_InstanceOf(x,&DeeList_Type)
#define DeeList_CheckExact(x)  DeeObject_InstanceOfExact(x,&DeeList_Type)

DDATDEF DeeTypeObject DeeList_Type;

/* Create a new list object from a vector. */
DFUNDEF DREF DeeObject *DCALL DeeList_NewVector(size_t objc, DeeObject *const *__restrict objv);
DFUNDEF DREF DeeObject *DCALL DeeList_NewVectorInherited(size_t objc, DREF DeeObject *const *__restrict objv);

/* Create a new list object. */
#define DeeList_New()   DeeObject_NewDefault(&DeeList_Type)
DFUNDEF DREF DeeObject *DCALL DeeList_NewHint(size_t n_prealloc);
DFUNDEF DREF DeeObject *DCALL DeeList_FromSequence(DeeObject *__restrict self);
DFUNDEF DREF DeeObject *DCALL DeeList_FromIterator(DeeObject *__restrict self);
/* WARNING: The caller must start gc-tracking the list once elements are initialized. */
DFUNDEF DREF DeeObject *DCALL DeeList_NewUninitialized(size_t n_elem);
DFUNDEF void DCALL DeeList_FreeUninitialized(DeeObject *__restrict self);

#ifdef CONFIG_BUILDING_DEEMON
/* Concat a list and some generic sequence,
 * inheriting a reference from `self' in the process. */
INTDEF DREF DeeObject *DCALL
DeeList_Concat(/*inherit(on_success)*/DREF DeeObject *__restrict self,
               DeeObject *__restrict sequence);
INTDEF DREF DeeObject *DCALL
DeeList_ExtendInherited(/*inherit(on_success)*/DREF DeeObject *__restrict self, size_t argc,
                        /*inherit(on_success)*/DREF DeeObject **__restrict argv);
#endif

/* @return: * : The actual number of deleted items.
 * @return: (size_t)-1: Error. */
DFUNDEF size_t DCALL
DeeList_Erase(DeeObject *__restrict self,
              size_t index, size_t count);

/* @return: 0 : The given `keyed_search_item' count not found found.
 * @return: 1 : The given `keyed_search_item' was deleted once.
 * @return: -1: An error occurred. */
DFUNDEF int DCALL
DeeList_Remove(DeeObject *__restrict self,
               size_t start, size_t end,
               DeeObject *__restrict keyed_search_item,
               DeeObject *key);
DFUNDEF int DCALL
DeeList_RRemove(DeeObject *__restrict self,
                size_t start, size_t end,
                DeeObject *__restrict keyed_search_item,
                DeeObject *key);

/* @return: * :   The popped element.
 * @return: NULL: The given index was out-of-bounds and an IndexError was thrown. */
DFUNDEF DREF DeeObject *DCALL DeeList_Pop(DeeObject *__restrict self, dssize_t index);

/* Clear the given list.
 * Returns `true' if the list wasn't empty before. */
DFUNDEF bool DCALL DeeList_Clear(DeeObject *__restrict self);

/* Sort the given list ascendingly, or according to `key' */
DFUNDEF int DCALL DeeList_Sort(DeeObject *__restrict self, DeeObject *key);
DFUNDEF DREF DeeObject *DCALL DeeList_Sorted(DeeObject *__restrict self, DeeObject *key);

/* Reverse the order of the elements of `self' */
DFUNDEF void DCALL DeeList_Reverse(DeeObject *__restrict self);

/* Append objects to a given list. */
DFUNDEF int DCALL DeeList_Append(DeeObject *__restrict self, DeeObject *__restrict elem);
DFUNDEF int DCALL DeeList_AppendIterator(DeeObject *__restrict self, DeeObject *__restrict iterator);
DFUNDEF int DCALL DeeList_AppendSequence(DeeObject *__restrict self, DeeObject *__restrict sequence);
DFUNDEF int DCALL DeeList_AppendVector(DeeObject *__restrict self, size_t objc, DeeObject *const *__restrict objv);

/* Insert objects into a given list. */
DFUNDEF int DCALL DeeList_Insert(DeeObject *__restrict self, size_t index, DeeObject *__restrict elem);
DFUNDEF int DCALL DeeList_InsertIterator(DeeObject *__restrict self, size_t index, DeeObject *__restrict iterator);
DFUNDEF int DCALL DeeList_InsertSequence(DeeObject *__restrict self, size_t index, DeeObject *__restrict sequence);
DFUNDEF int DCALL DeeList_InsertVector(DeeObject *__restrict self, size_t index, size_t objc, DeeObject *const *__restrict objv);


#ifndef __INTELLISENSE__
#ifndef __NO_builtin_expect
#define DeeList_Append(self,elem)                   __builtin_expect(DeeList_Append(self,elem),0)
#define DeeList_AppendIterator(self,iterator)       __builtin_expect(DeeList_AppendIterator(self,iterator),0)
#define DeeList_AppendSequence(self,sequence)       __builtin_expect(DeeList_AppendSequence(self,sequence),0)
#define DeeList_AppendVector(self,objc,objv)        __builtin_expect(DeeList_AppendVector(self,objc,objv),0)
#define DeeList_Insert(self,index,elem)             __builtin_expect(DeeList_Insert(self,index,elem),0)
#define DeeList_InsertIterator(self,index,iterator) __builtin_expect(DeeList_InsertIterator(self,index,iterator),0)
#define DeeList_InsertSequence(self,index,sequence) __builtin_expect(DeeList_InsertSequence(self,index,sequence),0)
#define DeeList_InsertVector(self,index,objc,objv)  __builtin_expect(DeeList_InsertVector(self,index,objc,objv),0)
#endif /* !__NO_builtin_expect */
#endif

DECL_END

#endif /* !GUARD_DEEMON_LIST_H */
