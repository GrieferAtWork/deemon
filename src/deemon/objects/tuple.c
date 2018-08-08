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
#ifndef GUARD_DEEMON_OBJECTS_TUPLE_C
#define GUARD_DEEMON_OBJECTS_TUPLE_C 1
#define _KOS_SOURCE 1

#include <deemon/api.h>
#include <deemon/object.h>
#include <deemon/none.h>
#include <deemon/string.h>
#include <deemon/bool.h>
#include <deemon/thread.h>
#include <deemon/int.h>
#include <deemon/seq.h>
#include <deemon/arg.h>
#include <deemon/tuple.h>
#include <deemon/list.h>
#include <deemon/util/string.h>
#ifndef CONFIG_NO_THREADS
#include <deemon/util/rwlock.h>
#endif /* !CONFIG_NO_THREADS */

#include <stddef.h>
#ifdef __KOS_SYSTEM_HEADERS__
#include <hybrid/minmax.h>
#endif

#include "../runtime/strings.h"
#include "../runtime/runtime_error.h"

#ifndef TUPLE_CACHE_MAXSIZE
#define TUPLE_CACHE_MAXSIZE   64
#endif
#ifndef TUPLE_CACHE_MAXCOUNT
#define TUPLE_CACHE_MAXCOUNT  8
#endif

#if !TUPLE_CACHE_MAXSIZE
#undef TUPLE_CACHE_MAXCOUNT
#define TUPLE_CACHE_MAXCOUNT 0
#endif

#if TUPLE_CACHE_MAXCOUNT && !defined(CONFIG_NO_THREADS)
#include <hybrid/atomic.h>
#include <hybrid/sched/yield.h>
#endif

DECL_BEGIN

typedef DeeTupleObject Tuple;

#ifndef MIN
#define MIN(x,y)     ((x) < (y) ? (x) : (y))
#endif
#ifndef MAX
#define MAX(x,y)     ((x) < (y) ? (y) : (x))
#endif


#if TUPLE_CACHE_MAXCOUNT
struct cached_object {
 struct cached_object *co_next;
};
struct tuple_cache {
 size_t                c_count; /* [lock(c_lock)][<= TUPLE_CACHE_MAXSIZE] Amount of cached objects int `c_head' */
 struct cached_object *c_head;  /* [lock(c_lock)][0..1] Linked list of cached tuple objects. */
#ifndef CONFIG_NO_THREADS
 rwlock_t              c_lock;  /* Lock for this tuple cache. */
#endif /* !CONFIG_NO_THREADS */
};

#ifdef CONFIG_NO_THREADS
#define LOCK(x)   (void)0
#define UNLOCK(x) (void)0
#else /* CONFIG_NO_THREADS */
#define LOCK(x)   rwlock_write(&(x))
#define UNLOCK(x) rwlock_endwrite(&(x))
#endif /* !CONFIG_NO_THREADS */

PRIVATE struct tuple_cache cache[TUPLE_CACHE_MAXCOUNT];

INTERN size_t DCALL
tuplecache_clear(size_t max_clear) {
 size_t result = 0;
 struct tuple_cache *iter;
 for (iter  = cache;
      iter != COMPILER_ENDOF(cache); ++iter) {
  struct cached_object *elem;
  LOCK(iter->c_lock);
  while ((elem = iter->c_head) != NULL && result < max_clear) {
   iter->c_head = elem->co_next;
   ASSERT(iter->c_count);
   --iter->c_count;
   result += (offsetof(DeeTupleObject,t_elem)+
             ((size_t)(iter - cache + 1)*sizeof(DeeObject *)));
   DeeObject_Free(elem);
  }
  ASSERT((iter->c_head != NULL) ==
         (iter->c_count != 0));
  UNLOCK(iter->c_lock);
  if (result >= max_clear) break;
 }
 return result;
}


#else /* TUPLE_CACHE_MAXCOUNT */
INTERN size_t DCALL
tuplecache_clear(size_t UNUSED(max_clear)) {
 return 0;
}
#endif /* !TUPLE_CACHE_MAXCOUNT */


PUBLIC DREF DeeObject *DCALL
DeeTuple_NewUninitialized(size_t n) {
 DREF DeeTupleObject *result;
 if unlikely(!n)
    return_empty_tuple;
#if TUPLE_CACHE_MAXCOUNT
 if (n < TUPLE_CACHE_MAXCOUNT) {
  struct tuple_cache *c = &cache[n-1];
  if (c->c_count) {
   LOCK(c->c_lock);
#ifndef CONFIG_NO_THREADS
   COMPILER_READ_BARRIER();
   if (c->c_head)
#endif /* !CONFIG_NO_THREADS */
   {
    ASSERT(c->c_head != NULL);
    result = (DREF DeeTupleObject *)c->c_head;
    c->c_head = ((struct cached_object *)result)->co_next;
    --c->c_count;
    ASSERT((c->c_count == 0) == (c->c_head == NULL));
    ASSERT(result->t_size == n);
    UNLOCK(c->c_lock);
    goto got_result;
   }
   UNLOCK(c->c_lock);
  }
 }
#endif /* TUPLE_CACHE_MAXCOUNT */
 result = (DREF DeeTupleObject *)DeeObject_Malloc(offsetof(DeeTupleObject,t_elem)+
                                                  n*sizeof(DeeObject *));
 if unlikely(!result) return NULL;
 result->t_size = n;
#if TUPLE_CACHE_MAXCOUNT
got_result:
#endif
 DeeObject_Init(result,&DeeTuple_Type);
#ifndef NDEBUG
 MEMSET_PTR(result->t_elem,0xcc,n);
#endif
 return (DREF DeeObject *)result;
}

#if TUPLE_CACHE_MAXCOUNT
INTERN void DCALL
tuple_tp_free(void *__restrict ob) {
 ASSERT(ob);
 ASSERT(!DeeTuple_IsEmpty(ob));
 ASSERT(DeeTuple_SIZE(ob) != 0);
#ifndef NDEBUG
 MEMSET_PTR(DeeTuple_ELEM(ob),0xcc,DeeTuple_SIZE(ob));
#endif
 if (DeeTuple_SIZE(ob) < TUPLE_CACHE_MAXCOUNT) {
  struct tuple_cache *c = &cache[DeeTuple_SIZE(ob)-1];
  if (c->c_count < TUPLE_CACHE_MAXSIZE) {
   LOCK(c->c_lock);
#ifndef CONFIG_NO_THREADS
   COMPILER_READ_BARRIER();
   if (c->c_count < TUPLE_CACHE_MAXSIZE)
#endif /* !CONFIG_NO_THREADS */
   {
    ((struct cached_object *)ob)->co_next = c->c_head;
    c->c_head = (struct cached_object *)ob;
    ++c->c_count;
    UNLOCK(c->c_lock);
    return;
   }
   UNLOCK(c->c_lock);
  }
 }
 DeeObject_Free(ob);
}
#else /* TUPLE_CACHE_MAXCOUNT */
#define tuple_tp_free(ob) DeeObject_Free(ob)
#endif /* TUPLE_CACHE_MAXCOUNT */

PUBLIC void DCALL
DeeTuple_FreeUninitialized(DREF DeeObject *__restrict self) {
 ASSERT(self->ob_refcnt == 1);
 ASSERT(self->ob_type == &DeeTuple_Type);
 tuple_tp_free(self);
}

PRIVATE int DCALL
DeeTuple_ResizeUninitialized(DREF DeeTupleObject **__restrict pself,
                             size_t new_size) {
 DREF DeeTupleObject *old_tuple;
 DREF DeeTupleObject *new_tuple;
 ASSERT(pself);
 old_tuple = *pself;
 ASSERT_OBJECT(old_tuple);
 ASSERT(DeeTuple_Check(old_tuple));
 if unlikely(DeeTuple_SIZE(old_tuple) == new_size)
    return 0;
 if unlikely(DeeTuple_IsEmpty(old_tuple)) {
  /* Special case: Must not resize the empty tuple. */
  new_tuple = (DREF DeeTupleObject *)DeeTuple_NewUninitialized(new_size);
  if unlikely(!new_tuple) return -1;
  Dee_Decref(Dee_EmptyTuple);
  *pself = new_tuple;
  return 0;
 }
 ASSERT(DeeTuple_SIZE(old_tuple) != 0);
 ASSERTF(old_tuple->ob_refcnt == 1,"The tuple is being shared");
 if unlikely(!new_size) {
  /* Special case: Resize to an empty tuple. */
  Dee_DecrefNokill(&DeeTuple_Type);
  tuple_tp_free(old_tuple);
  Dee_Incref(Dee_EmptyTuple);
  *pself = (DREF DeeTupleObject *)Dee_EmptyTuple;
  return 0;
 }

#if TUPLE_CACHE_MAXCOUNT
 /* Check if we can use a cached tuple. */
 if (new_size < TUPLE_CACHE_MAXCOUNT) {
  struct tuple_cache *c = &cache[new_size-1];
  if (c->c_count) {
   LOCK(c->c_lock);
#ifndef CONFIG_NO_THREADS
   COMPILER_READ_BARRIER();
   if (c->c_head)
#endif
   {
    ASSERT(c->c_head);
    new_tuple = (DREF DeeTupleObject *)c->c_head;
    c->c_head = ((struct cached_object *)new_tuple)->co_next;
    --c->c_count;
    UNLOCK(c->c_lock);
    /* Copy tuple data (And inherit the reference to `DeeTuple_Type') */
    MEMCPY_PTR(new_tuple,old_tuple,
              (offsetof(DeeTupleObject,t_elem)/sizeof(void *))+
               MIN(DeeTuple_SIZE(old_tuple),new_size));
#ifndef NDEBUG
    if (new_size > old_tuple->t_size) {
     MEMSET_PTR(&new_tuple->t_elem[old_tuple->t_size],0xcc,
                 new_size-old_tuple->t_size);
    }
#endif
    tuple_tp_free(old_tuple);
    new_tuple->t_size = new_size;
    *pself = new_tuple;
    return 0;
   }
   UNLOCK(c->c_lock);
  }
 }
#endif /* TUPLE_CACHE_MAXCOUNT */
 /* Allocate a new tuple. */
 new_tuple = (DREF DeeTupleObject *)DeeObject_Realloc(old_tuple,offsetof(DeeTupleObject,t_elem)+
                                                      new_size*sizeof(DREF DeeObject *));
 if unlikely(!new_tuple) return -1;
#ifndef NDEBUG
 if (new_size > new_tuple->t_size) {
  MEMSET_PTR(&new_tuple->t_elem[new_tuple->t_size],
              0xcc,new_size-new_tuple->t_size);
 }
#endif
 new_tuple->t_size = new_size;
 *pself = new_tuple;
 return 0;
}


PUBLIC DREF DeeObject *DCALL
DeeTuple_NewVector(size_t objc, DeeObject *const *__restrict objv) {
 DREF DeeObject *result,**iter,*temp;
 result = DeeTuple_NewUninitialized(objc);
 if unlikely(!result) return NULL;
 iter = DeeTuple_ELEM(result);
 while (objc--) temp = *objv++,Dee_Incref(temp),*iter++ = temp;
 return result;
}

PUBLIC DREF DeeObject *DCALL
DeeTuple_NewVectorSymbolic(size_t objc, DeeObject *const *__restrict objv) {
 DREF DeeObject *result;
 result = DeeTuple_NewUninitialized(objc);
 if unlikely(!result) return NULL;
 MEMCPY_PTR(DeeTuple_ELEM(result),objv,objc);
 return result;
}


PUBLIC DREF DeeObject *DCALL
DeeTuple_FromSequence(DeeObject *__restrict self) {
 DREF DeeObject *result;
 ASSERT_OBJECT(self);
 /* Optimizations for specific types such as `tuple' and `list' */
 if (DeeTuple_CheckExact(self))
     return_reference_(self);
 if (DeeList_CheckExact(self)) {
  DREF DeeObject **src;
  size_t i,list_size = ATOMIC_READ(DeeList_SIZE(self));
list_size_changed:
  result = DeeTuple_NewUninitialized(list_size);
  if unlikely(!result) return NULL;
  COMPILER_READ_BARRIER();
  DeeList_LockRead(self);
  if unlikely(list_size != DeeList_SIZE(self)) {
   list_size = DeeList_SIZE(self);
   DeeList_LockEndRead(self);
   DeeTuple_FreeUninitialized(result);
   goto list_size_changed;
  }
  src = DeeList_ELEM(self);
  MEMCPY_PTR(DeeTuple_ELEM(result),src,list_size);
  for (i = 0; i < list_size; ++i)
      Dee_Incref(src[i]);
  DeeList_LockEndRead(self);
  return result;
 }
 /* Use general-purpose iterators to create a new tuple. */
 self = DeeObject_IterSelf(self);
 if unlikely(!self) return NULL;
 result = DeeTuple_FromIterator(self);
 Dee_Decref(self);
 return result;
}
PUBLIC DREF DeeObject *DCALL
DeeTuple_FromIterator(DeeObject *__restrict self) {
 DREF DeeObject *elem,*next; size_t used_size;
 DREF DeeTupleObject *result;
 /* Optimizations for empty- and single-element iterators. */
 elem = DeeObject_IterNext(self);
 if unlikely(!elem) return NULL;
 if (elem == ITER_DONE) return_empty_tuple;
 next = DeeObject_IterNext(self);
 if unlikely(!next) { err_elem: Dee_Decref(elem); return NULL; }
 if (next == ITER_DONE) {
  result = (DREF DeeTupleObject *)DeeTuple_PackSymbolic(1,elem);
  if unlikely(!result) goto err_elem;
  return (DREF DeeObject *)result;
 }
 result = (DREF DeeTupleObject *)DeeTuple_NewUninitialized(2);
 if unlikely(!result) { Dee_Decref(next); goto err_elem; }
 result->t_elem[0] = elem;
 result->t_elem[1] = next;
 used_size = 2;
 while (ITER_ISOK(elem = DeeObject_IterNext(self))) {
  ASSERT(used_size <= result->t_size);
  if (used_size == result->t_size) {
   /* Allocate more memory. */
   if unlikely(DeeTuple_ResizeUninitialized(&result,result->t_size*2)) {
    Dee_Decref(elem);
    goto err_cleanup;
   }
  }
  result->t_elem[used_size++] = elem; /* Inherit reference. */
  if (DeeThread_CheckInterrupt())
      goto err_cleanup;
 }
 if unlikely(!elem) {
err_cleanup:
  /* Cleanup elements we've already assigned. */
  while (used_size--)
         Dee_Decref(result->t_elem[used_size]);
  Dee_DecrefNokill(&DeeTuple_Type);
  tuple_tp_free(result);
  result = NULL;
 } else if (used_size != result->t_size) {
  /* Fix the actually used size. */
#if TUPLE_CACHE_MAXCOUNT
  if (used_size < TUPLE_CACHE_MAXCOUNT) {
   DREF DeeTupleObject *new_tuple;
   struct tuple_cache *c = &cache[used_size-1];
   if (c->c_count) {
    LOCK(c->c_lock);
#ifndef CONFIG_NO_THREADS
    COMPILER_READ_BARRIER();
    if (c->c_head)
#endif
    {
     ASSERT(c->c_head);
     new_tuple = (DREF DeeTupleObject *)c->c_head;
     c->c_head = ((struct cached_object *)new_tuple)->co_next;
     --c->c_count;
     UNLOCK(c->c_lock);
     /* Copy tuple data (And inherit the reference to `DeeTuple_Type') */
     ASSERT(used_size < DeeTuple_SIZE(result));
     MEMCPY_PTR(new_tuple,result,
               (offsetof(DeeTupleObject,t_elem)/sizeof(void *))+
                used_size);
     tuple_tp_free(result);
     new_tuple->t_size = used_size;
     ASSERT(new_tuple->ob_refcnt == 1);
     return (DREF DeeObject *)new_tuple;
    }
    UNLOCK(c->c_lock);
   }
  }
#endif /* TUPLE_CACHE_MAXCOUNT */
  ASSERT(result->ob_refcnt == 1);
  next = (DREF DeeObject *)DeeObject_TryRealloc(result,offsetof(DeeTupleObject,t_elem)+
                                                used_size*sizeof(DREF DeeObject *));
  if likely(next) result = (DREF DeeTupleObject *)next;
  result->t_size = used_size;
 }
 return (DREF DeeObject *)result;
}

PUBLIC DREF DeeObject *DCALL
DeeTuple_Types(DeeObject *__restrict self) {
 DREF DeeObject *result,**iter,**end,**src;
 ASSERT_OBJECT(self);
 ASSERT(DeeTuple_Check(self));
 result = DeeTuple_NewUninitialized(DeeTuple_SIZE(self));
 if unlikely(!result) return NULL;
 end = (iter = DeeTuple_ELEM(result))+DeeTuple_SIZE(result);
 src =  DeeTuple_ELEM(self);
 for (; iter != end; ++iter,++src)
      Dee_INCREF(*iter = (DeeObject *)Dee_TYPE(*src));
 return result;
}

#ifdef CONFIG_VA_LIST_IS_STACK_POINTER
#ifndef __NO_DEFINE_ALIAS
DEFINE_PUBLIC_ALIAS(ASSEMBLY_NAME(DeeTuple_VPack,8),
                    ASSEMBLY_NAME(DeeTuple_NewVector,8));
DEFINE_PUBLIC_ALIAS(ASSEMBLY_NAME(DeeTuple_VPackSymbolic,8),
                    ASSEMBLY_NAME(DeeTuple_NewVectorSymbolic,8));
#else /* !__NO_DEFINE_ALIAS */
PUBLIC DREF DeeObject *DCALL
DeeTuple_VPack(size_t n, va_list args) {
 return DeeTuple_NewVector(n,(DeeObject **)args);
}
PUBLIC DREF DeeObject *DCALL
DeeTuple_VPackSymbolic(size_t n, va_list args) {
 return DeeTuple_NewVectorSymbolic(n,(DeeObject **)args);
}
#endif /* __NO_DEFINE_ALIAS */
#else
PUBLIC DREF DeeObject *DCALL
DeeTuple_VPack(size_t n, va_list args) {
 DREF DeeObject *result,**iter,*elem;
 result = DeeTuple_NewUninitialized(n);
 if unlikely(!result) return NULL;
 iter = DeeTuple_ELEM(result);
 while (n--) {
  elem = va_arg(args,DeeObject *);
  Dee_Incref(elem);
  *iter++ = elem;
 }
 return result;
}
PUBLIC DREF DeeObject *DCALL
DeeTuple_VPackSymbolic(size_t n, va_list args) {
 DREF DeeObject *result,**iter;
 result = DeeTuple_NewUninitialized(n);
 if unlikely(!result) return NULL;
 iter = DeeTuple_ELEM(result);
 while (n--) *iter++ = va_arg(args,DeeObject *);
 return result;
}
#endif

PUBLIC DREF DeeObject *DeeTuple_Pack(size_t n, ...) {
 DREF DeeObject *result;
 va_list args;
 va_start(args,n);
 result = DeeTuple_VPack(n,args);
 va_end(args);
 return result;
}
PUBLIC DREF DeeObject *DeeTuple_PackSymbolic(size_t n, ...) {
 DREF DeeObject *result;
 va_list args;
 va_start(args,n);
 result = DeeTuple_VPackSymbolic(n,args);
 va_end(args);
 return result;
}


PUBLIC void DCALL
DeeTuple_DecrefSymbolic(DeeObject *__restrict self) {
 ASSERT_OBJECT(self);
 ASSERT(DeeTuple_Check(self));
 if (!DeeObject_IsShared(self)) {
  DeeTuple_FreeUninitialized(self);
 } else {
  DeeObject **iter,**end;
  end = (iter = DeeTuple_ELEM(self))+DeeTuple_SIZE(self);
  for (; iter != end; ++iter) Dee_Incref(*iter);
  Dee_Decref(self);
 }
}

PUBLIC int (DCALL DeeTuple_AppendIterator)(DREF DeeObject **__restrict pself,
                                           DeeObject *__restrict iterator) {
 DREF DeeObject *elem; size_t incfactor = 2;
 size_t used_size = DeeTuple_SIZE(*pself);
 while (ITER_ISOK(elem = DeeObject_IterNext(iterator))) {
  if (used_size == DeeTuple_SIZE(*pself)) {
   /* Must increase the tuple's size. */
   if unlikely(DeeTuple_ResizeUninitialized((DREF DeeTupleObject **)pself,
                                             used_size+incfactor)) {
    Dee_Decref(elem);
    goto err;
   }
   incfactor *= 2;
  }
  DeeTuple_SET(*pself,used_size,elem); /* Inherit reference. */
  ++used_size;
  if (DeeThread_CheckInterrupt())
      goto err;
 }
 if (used_size != DeeTuple_SIZE(*pself)) {
  if unlikely(DeeTuple_ResizeUninitialized((DREF DeeTupleObject **)pself,used_size)) {
   DeeObject **iter,**end;
   /* To bring the tuple back into a valid state, fill unused elements with `none'. */
   iter = DeeTuple_ELEM(*pself)+used_size;
   end  = DeeTuple_ELEM(*pself)+DeeTuple_SIZE(*pself);
   for (; iter != end; ++iter) Dee_INCREF(*iter = Dee_None);
   goto err;
  }
 }
 if likely(elem)
    return 0;
err:
 return -1;
}
PUBLIC int (DCALL DeeTuple_Append)(DREF DeeObject **__restrict pself,
                                   DeeObject *__restrict item) {
 int result; size_t index;
 /* Must increase the tuple's size. */
 index  = DeeTuple_SIZE(*pself);
 result = DeeTuple_ResizeUninitialized((DREF DeeTupleObject **)pself,index+1);
 if likely(!result) {
  DeeTuple_SET(*pself,index,item);
  Dee_Incref(item);
 }
 return result;
}


INTERN DREF DeeObject *DCALL
DeeTuple_ExtendInherited(DREF DeeObject *__restrict self,
                         size_t argc, DREF DeeObject **__restrict argv) {
 DREF DeeTupleObject *result;
 ASSERT_OBJECT(self);
 ASSERT(DeeTuple_Check(self));
 if (!DeeObject_IsShared(self)) {
  /* Optimizations for specific types. */
  size_t old_size;
  /* Re-use `self'. */
  result = (DREF DeeTupleObject *)self;
  old_size = result->t_size;
  if (DeeTuple_ResizeUninitialized(&result,old_size+argc))
      goto err;
  MEMCPY_PTR(result->t_elem+old_size,argv,argc);
 } else {
  DREF DeeObject **iter,**end,**dst;
  result = (DREF DeeTupleObject *)DeeTuple_NewUninitialized(DeeTuple_SIZE(self)+argc);
  if unlikely(!result) goto err;
  dst = DeeTuple_ELEM(result);
  end = (iter = DeeTuple_ELEM(self))+DeeTuple_SIZE(self);
  while (iter != end) Dee_INCREF(*dst++ = *iter++);
  MEMCPY_PTR(dst,argv,argc);
  Dee_Decref(self);
 }
 return (DREF DeeObject *)result;
err:
 return NULL;
}
INTERN DREF DeeObject *DCALL
DeeTuple_ConcatInherited(DREF DeeObject *__restrict self,
                         DeeObject *__restrict sequence) {
 DeeObject **iter,**end,**srcdst;
 DREF DeeTupleObject *result;
 ASSERT_OBJECT(self);
 ASSERT_OBJECT(sequence);
 ASSERT(DeeTuple_Check(self));
 if (!DeeObject_IsShared(self)) {
  /* Re-use `self'. */
  result = (DREF DeeTupleObject *)self;
  /* Optimizations for specific types. */
#if 0
  if (DeeTuple_CheckExact(sequence)) {
   size_t old_size = result->t_size;
   if (DeeTuple_ResizeUninitialized(&result,old_size+DeeTuple_SIZE(sequence)))
       goto err_inherit;
   end = (iter = result->t_elem+old_size)+DeeTuple_SIZE(sequence);
   src = DeeTuple_ELEM(sequence);
   for (; iter != end; ++iter,++src) Dee_INCREF(*iter = *src);
   goto done;
  }
#endif
  if (DeeList_CheckExact(sequence)) {
   size_t old_size = result->t_size;
   size_t sequence_size = ATOMIC_READ(DeeList_SIZE(sequence));
   if unlikely(!sequence_size) goto done;
handle_list_size:
   if (DeeTuple_ResizeUninitialized(&result,old_size+DeeTuple_SIZE(sequence))) {
    ASSERT(!DeeObject_IsShared(result));
    end = (iter = DeeTuple_ELEM(result))+old_size;
    for (; iter != end; ++iter) Dee_Decref(*iter);
    Dee_DecrefNokill(&DeeTuple_Type);
    tuple_tp_free(result);
    return NULL;
   }
   COMPILER_READ_BARRIER();
   DeeList_LockRead(sequence);
   /* Check if the list's size has changes in the mean time. */
   if unlikely(sequence_size != DeeList_SIZE(sequence)) {
    sequence_size = DeeList_SIZE(sequence);
    DeeList_LockEndRead(sequence);
    goto handle_list_size;
   }
   /* Copy elements from the list. */
   end = (iter = result->t_elem+old_size)+DeeTuple_SIZE(sequence);
   srcdst = DeeList_ELEM(sequence);
   for (; iter != end; ++iter,++srcdst) Dee_INCREF(*iter = *srcdst);
   DeeList_LockEndRead(sequence);
   goto done;
  }
 } else {
#if 0
  if (DeeTuple_CheckExact(sequence)) {
   result = (DREF DeeTupleObject *)DeeTuple_NewUninitialized(DeeTuple_SIZE(self)+
                                                             DeeTuple_SIZE(sequence));
   if unlikely(!result) goto err_inherit;
   src = DeeTuple_ELEM(result);
   end = (iter = DeeTuple_ELEM(self))+DeeTuple_SIZE(self);
   while (iter != end) Dee_INCREF(*src++ = *iter++);
   end = (iter = DeeTuple_ELEM(sequence))+DeeTuple_SIZE(sequence);
   while (iter != end) Dee_INCREF(*src++ = *iter++);
   Dee_Decref(self);
   goto done;
  }
#endif
  if (DeeList_CheckExact(sequence)) {
   size_t used_list_size;
   used_list_size = ATOMIC_READ(DeeList_SIZE(sequence));
handle_list_size2:
   result = (DREF DeeTupleObject *)DeeTuple_NewUninitialized(DeeTuple_SIZE(self)+used_list_size);
   if unlikely(!result) goto err;
   COMPILER_READ_BARRIER();
   DeeList_LockRead(sequence);
   /* Make sure the list didn't change size in he mean time. */
   if unlikely(used_list_size != DeeList_SIZE(sequence)) {
    used_list_size = DeeList_SIZE(sequence);
    goto handle_list_size2;
   }
   srcdst = DeeTuple_ELEM(result)+DeeTuple_SIZE(self);
   end = (iter = DeeList_ELEM(sequence))+used_list_size;
   while (iter != end) Dee_INCREF(*srcdst++ = *iter++);
   DeeList_LockEndRead(sequence);
   srcdst = DeeTuple_ELEM(result);
   end = (iter = DeeTuple_ELEM(self))+DeeTuple_SIZE(self);
   while (iter != end) Dee_INCREF(*srcdst++ = *iter++);
   Dee_Decref(self);
   goto done;
  }

  /* Copy the `self' tuple, so-as to allow us to modify it. */
  result = (DREF DeeTupleObject *)DeeTuple_NewVector(DeeTuple_SIZE(self),DeeTuple_ELEM(self));
  Dee_Decref(self);
  if unlikely(!result) return NULL;
 }

 sequence = DeeObject_IterSelf(sequence);
 if unlikely(!sequence) goto err_result;
 { int error = DeeTuple_AppendIterator((DREF DeeObject **)&result,sequence);
   Dee_Decref(sequence);
   if (error) goto err_result;
 }
done:
 return (DREF DeeObject *)result;
err_result:
 if (result != (DREF DeeTupleObject *)self)
     Dee_DecrefDokill(result);
err:
 return NULL;
}










/*  ====== `tuple.iterator' type implementation ======  */
INTDEF DeeTypeObject DeeTupleIterator_Type;
typedef struct {
 OBJECT_HEAD
 DREF DeeTupleObject *ti_tuple; /* [1..1][const] Referenced tuple. */
 size_t               ti_index; /* [<= ti_tuple->t_size] Next-element index. */
} TupleIterator;

PRIVATE int DCALL
tuple_iterator_ctor(TupleIterator *__restrict self) {
 self->ti_tuple = (DREF DeeTupleObject *)Dee_EmptyTuple;
 self->ti_index = 0;
 Dee_Incref(Dee_EmptyTuple);
 return 0;
}

PRIVATE int DCALL
tuple_iterator_copy(TupleIterator *__restrict self,
                    TupleIterator *__restrict other) {
 self->ti_tuple = other->ti_tuple;
 Dee_Incref(self->ti_tuple);
#ifdef CONFIG_NO_THREADS
 self->ti_index = other->ti_index;
#else
 self->ti_index = ATOMIC_READ(other->ti_index);
#endif
 return 0;
}

PRIVATE int DCALL
tuple_iterator_init(TupleIterator *__restrict self,
                    size_t argc, DeeObject **__restrict argv) {
 self->ti_tuple = (DREF DeeTupleObject *)Dee_EmptyTuple;
 self->ti_index = 0;
 if (DeeArg_Unpack(argc,argv,"|oIu:tuple.iterator",
                    &self->ti_tuple,&self->ti_index))
     return -1;
 if (DeeObject_AssertTypeExact((DeeObject *)self->ti_tuple,&DeeTuple_Type))
     return -1;
 if (self->ti_index >= DeeTuple_SIZE(self->ti_tuple)) {
  err_index_out_of_bounds((DeeObject *)self->ti_tuple,
                           self->ti_index,DeeTuple_SIZE(self->ti_tuple));
  return -1;
 }
 Dee_Incref(self->ti_tuple);
 return 0;
}

PRIVATE void DCALL
tuple_iterator_fini(TupleIterator *__restrict self) {
 Dee_Decref(self->ti_tuple);
}

PRIVATE DREF DeeObject *DCALL
tuple_iterator_next(TupleIterator *__restrict self) {
 DREF DeeObject *result;
 ASSERT(self->ti_index <= DeeTuple_SIZE(self->ti_tuple));
 if (self->ti_index == DeeTuple_SIZE(self->ti_tuple))
     return ITER_DONE;
 result = DeeTuple_GET(self->ti_tuple,self->ti_index);
 ASSERT_OBJECT(result);
 ++self->ti_index;
 Dee_Incref(result);
 return result;
}

PRIVATE struct type_member tupleiterator_members[] = {
    TYPE_MEMBER_FIELD("seq",STRUCT_OBJECT,offsetof(TupleIterator,ti_tuple)),
    TYPE_MEMBER_FIELD("__index__",STRUCT_CONST|STRUCT_SIZE_T,offsetof(TupleIterator,ti_index)),
    TYPE_MEMBER_END
};

INTERN DeeTypeObject DeeTupleIterator_Type = {
    OBJECT_HEAD_INIT(&DeeType_Type),
    /* .tp_name     = */"tuple.iterator",
    /* .tp_doc      = */NULL,
    /* .tp_flags    = */TP_FNORMAL|TP_FFINAL,
    /* .tp_weakrefs = */0,
    /* .tp_features = */TF_NONE,
    /* .tp_base     = */&DeeIterator_Type,
    /* .tp_init = */{
        {
            /* .tp_alloc = */{
                /* .tp_ctor      = */&tuple_iterator_ctor,
                /* .tp_copy_ctor = */&tuple_iterator_copy,
                /* .tp_deep_ctor = */NULL,
                /* .tp_any_ctor  = */&tuple_iterator_init,
                /* .tp_free      = */NULL,
                {
                    /* .tp_instance_size = */sizeof(TupleIterator)
                }
            }
        },
        /* .tp_dtor        = */(void(DCALL *)(DeeObject *__restrict))&tuple_iterator_fini,
        /* .tp_assign      = */NULL,
        /* .tp_move_assign = */NULL
    },
    /* .tp_cast = */{
        /* .tp_str  = */NULL,
        /* .tp_repr = */NULL,
        /* .tp_bool = */NULL
    },
    /* .tp_call          = */NULL,
    /* .tp_visit         = */NULL,
    /* .tp_gc            = */NULL,
    /* .tp_math          = */NULL,
    /* .tp_cmp           = */NULL,
    /* .tp_seq           = */NULL,
    /* .tp_iter_next     = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&tuple_iterator_next,
    /* .tp_attr          = */NULL,
    /* .tp_with          = */NULL,
    /* .tp_buffer        = */NULL,
    /* .tp_methods       = */NULL,
    /* .tp_getsets       = */NULL,
    /* .tp_members       = */tupleiterator_members,
    /* .tp_class_methods = */NULL,
    /* .tp_class_getsets = */NULL,
    /* .tp_class_members = */NULL
};











/*  ====== `tuple' type implementation ======  */

PRIVATE DREF Tuple *DCALL tuple_ctor(void) {
 return_reference_((DREF Tuple *)Dee_EmptyTuple);
}

PRIVATE DREF Tuple *DCALL
tuple_init(size_t argc, DeeObject **__restrict argv) {
 DeeObject *seq;
 if (DeeArg_Unpack(argc,argv,"o:tuple",&seq))
     return NULL;
 return (DREF Tuple *)DeeTuple_FromSequence(seq);
}

INTERN void DCALL
tuple_fini(Tuple *__restrict self) {
 DREF DeeObject **iter,**end;
 end = (iter = DeeTuple_ELEM(self))+DeeTuple_SIZE(self);
 for (; iter != end; ++iter) Dee_Decref(*iter);
}

PRIVATE DREF DeeObject *DCALL
tuple_iter(Tuple *__restrict self) {
 DREF TupleIterator *result;
 result = (DREF TupleIterator *)DeeObject_Malloc(sizeof(TupleIterator));
 if likely(result) {
  DeeObject_Init(result,&DeeTupleIterator_Type);
  result->ti_index = 0;
  result->ti_tuple = self;
  Dee_Incref(self);
 }
 return (DREF DeeObject *)result;
}

PRIVATE DREF DeeObject *DCALL
tuple_size(Tuple *__restrict self) {
 return DeeInt_NewSize(DeeTuple_SIZE(self));
}

PRIVATE DREF DeeObject *DCALL
tuple_contains(Tuple *__restrict self, DeeObject *__restrict item) {
 DeeObject **iter,**end; int error;
 end = (iter = DeeTuple_ELEM(self))+DeeTuple_SIZE(self);
 for (; iter != end; ++iter) {
  error = DeeObject_CompareEq(*iter,item);
  if unlikely(error < 0) return NULL;
  if (error) return_true;
 }
 return_false;
}

PRIVATE DREF DeeObject *DCALL
tuple_getitem(Tuple *__restrict self, DeeObject *__restrict index) {
 size_t i;
 if (DeeObject_AsSize(index,&i))
     return NULL;
 if unlikely(i >= DeeTuple_SIZE(self)) {
  err_index_out_of_bounds((DeeObject *)self,
                           i,DeeTuple_SIZE(self));
  return NULL;
 }
 return_reference(DeeTuple_GET(self,i));
}

INTERN DREF DeeObject *DCALL
tuple_getrange_i(Tuple *__restrict self,
                 dssize_t begin, dssize_t end) {
 if unlikely(begin < 0) begin += DeeTuple_SIZE(self);
 if unlikely(end < 0) end += DeeTuple_SIZE(self);
 if unlikely((size_t)begin >= DeeTuple_SIZE(self) ||
             (size_t)begin >= (size_t)end)
    return_empty_tuple;
 if unlikely((size_t)end > DeeTuple_SIZE(self))
             end = (dssize_t)DeeTuple_SIZE(self);
 return DeeTuple_NewVector((size_t)(end-begin),
                            DeeTuple_ELEM(self)+begin);
}

INTERN DREF DeeObject *DCALL
tuple_getrange_in(Tuple *__restrict self,
                  dssize_t begin) {
 if unlikely(begin < 0) begin += DeeTuple_SIZE(self);
 if unlikely((size_t)begin >= DeeTuple_SIZE(self))
    return_empty_tuple;
 return DeeTuple_NewVector(DeeTuple_SIZE(self) - (size_t)begin,
                           DeeTuple_ELEM(self) + (size_t)begin);
}

PRIVATE DREF DeeObject *DCALL
tuple_getrange(Tuple *__restrict self,
               DeeObject *__restrict begin,
               DeeObject *__restrict end) {
 dssize_t i_begin,i_end = DeeTuple_SIZE(self);
 if (DeeObject_AsSSize(begin,&i_begin) ||
    (!DeeNone_Check(end) && DeeObject_AsSSize(end,&i_end)))
     return NULL;
 return tuple_getrange_i(self,i_begin,i_end);
}



PRIVATE size_t DCALL
tuple_nsi_getsize(Tuple *__restrict self) {
 ASSERT(self->t_size != (size_t)-1);
 return self->t_size;
}
PRIVATE DREF DeeObject *DCALL
tuple_nsi_getitem(Tuple *__restrict self, size_t index) {
 if unlikely(index >= self->t_size) {
  err_index_out_of_bounds((DeeObject *)self,index,self->t_size);
  return NULL;
 }
 return_reference(self->t_elem[index]);
}
PRIVATE DREF DeeObject *DCALL
tuple_nsi_getitem_fast(Tuple *__restrict self, size_t index) {
 ASSERT(index < self->t_size);
 return_reference(self->t_elem[index]);
}

PRIVATE size_t DCALL
tuple_nsi_find(Tuple *__restrict self, size_t start, size_t end,
               DeeObject *__restrict value, DeeObject *pred_eq) {
 size_t i;
 if (start > self->t_size) start = self->t_size;
 if (end > self->t_size) end = self->t_size;
 for (i = start; i < end; ++i) {
  int temp;
  if (pred_eq) {
   DREF DeeObject *pred_result;
   pred_result = DeeObject_CallPack(pred_eq,2,self->t_elem[i],value);
   if unlikely(!pred_result) goto err;
   temp = DeeObject_Bool(pred_result);
   Dee_Decref(pred_result);
  } else {
   temp = DeeObject_CompareEq(self->t_elem[i],value);
  }
  if (temp != 0) {
   if unlikely(temp < 0) goto err;
   return i;
  }
 }
 return (size_t)-1;
err:
 return (size_t)-2;
}
PRIVATE size_t DCALL
tuple_nsi_rfind(Tuple *__restrict self, size_t start, size_t end,
                DeeObject *__restrict value, DeeObject *pred_eq) {
 size_t i;
 if (start > self->t_size) start = self->t_size;
 if (end > self->t_size) end = self->t_size;
 i = end;
 while (i > start) {
  int temp;
  --i;
  if (pred_eq) {
   DREF DeeObject *pred_result;
   pred_result = DeeObject_CallPack(pred_eq,2,self->t_elem[i],value);
   if unlikely(!pred_result) goto err;
   temp = DeeObject_Bool(pred_result);
   Dee_Decref(pred_result);
  } else {
   temp = DeeObject_CompareEq(self->t_elem[i],value);
  }
  if (temp != 0) {
   if unlikely(temp < 0) goto err;
   return i;
  }
 }
 return (size_t)-1;
err:
 return (size_t)-2;
}


PRIVATE struct type_nsi tuple_nsi = {
    /* .nsi_class   = */TYPE_SEQX_CLASS_SEQ,
    {
        /* .nsi_seqlike = */{
            /* .nsi_getsize      = */(void *)&tuple_nsi_getsize,
            /* .nsi_getsize_fast = */(void *)&tuple_nsi_getsize,
            /* .nsi_getitem      = */(void *)&tuple_nsi_getitem,
            /* .nsi_delitem      = */(void *)NULL,
            /* .nsi_setitem      = */(void *)NULL,
            /* .nsi_getitem_fast = */(void *)&tuple_nsi_getitem_fast,
            /* .nsi_getrange     = */(void *)&tuple_getrange_i,
            /* .nsi_getrange_n   = */(void *)&tuple_getrange_in,
            /* .nsi_setrange     = */(void *)NULL,
            /* .nsi_setrange_n   = */(void *)NULL,
            /* .nsi_find         = */(void *)&tuple_nsi_find,
            /* .nsi_rfind        = */(void *)&tuple_nsi_rfind,
            /* .nsi_xch          = */(void *)NULL,
            /* .nsi_insert       = */(void *)NULL,
            /* .nsi_insertall    = */(void *)NULL,
            /* .nsi_insertvec    = */(void *)NULL,
            /* .nsi_pop          = */(void *)NULL,
            /* .nsi_erase        = */(void *)NULL,
            /* .nsi_remove       = */(void *)NULL,
            /* .nsi_rremove      = */(void *)NULL,
            /* .nsi_removeall    = */(void *)NULL
        }
    }
};


PRIVATE struct type_seq tuple_seq = {
    /* .tp_iter_self = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&tuple_iter,
    /* .tp_size      = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&tuple_size,
    /* .tp_contains  = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&tuple_contains,
    /* .tp_get       = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&tuple_getitem,
    /* .tp_del       = */NULL,
    /* .tp_set       = */NULL,
    /* .tp_range_get = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict,DeeObject *__restrict))&tuple_getrange,
    /* .tp_range_del = */NULL,
    /* .tp_range_set = */NULL,
    /* .tp_nsi       = */&tuple_nsi
};

PRIVATE DREF DeeObject *DCALL
tuple_unpack(DeeObject *__restrict UNUSED(self),
             size_t argc, DeeObject **__restrict argv) {
 size_t num_items; DeeObject *init;
 DREF DeeObject *result;
 if (DeeArg_Unpack(argc,argv,"Iuo:unpack",&num_items,&init))
     return NULL;
 result = DeeTuple_NewUninitialized(num_items);
 if unlikely(!result) goto done;
 if unlikely(DeeObject_Unpack(init,num_items,DeeTuple_ELEM(result))) {
  DeeTuple_FreeUninitialized(result);
  result = NULL;
 }
done:
 return result;
}

PRIVATE struct type_method tuple_class_methods[] = {
    { "unpack", &tuple_unpack,
       DOC("(int num_items,sequence init)->tuple\n"
           "@throw UnpackError The given @init doesn't contain exactly @num_items elements\n"
           "Unpack the given sequence @init into a tuple consisting of @num_items elements") },
    { NULL }
};

PRIVATE struct type_member tuple_class_members[] = {
    TYPE_MEMBER_CONST("iterator",&DeeTupleIterator_Type),
    TYPE_MEMBER_END
};

INTERN void DCALL
tuple_visit(Tuple *__restrict self,
            dvisit_t proc, void *arg) {
 size_t i,count = DeeTuple_SIZE(self);
 for (i = 0; i < count; ++i)
     Dee_Visit(DeeTuple_GET(self,i));
}

PRIVATE DREF DeeObject *DCALL
tuple_str(Tuple *__restrict self) {
 struct unicode_printer p = UNICODE_PRINTER_INIT;
 DeeObject **iter,**end;
 if (unicode_printer_putascii(&p,'(') < 0) goto err;
 end = (iter = DeeTuple_ELEM(self))+DeeTuple_SIZE(self);
 for (; iter != end; ++iter) {
  /* Print this item. */
  if (iter != DeeTuple_ELEM(self) &&
      UNICODE_PRINTER_PRINT(&p,", ") < 0)
      goto err;
  if (unicode_printer_printobject(&p,*iter) < 0)
      goto err;
 }
 if (unicode_printer_putascii(&p,')') < 0) goto err;
 return unicode_printer_pack(&p);
err:
 unicode_printer_fini(&p);
 return NULL;
}
PRIVATE DREF DeeObject *DCALL
tuple_repr(Tuple *__restrict self) {
 struct unicode_printer p;
 DeeObject **iter,**end;
 /* Special case: single-item tuples are
  * must be encoded with a trailing comma. */
 if (DeeTuple_SIZE(self) == 1)
     return DeeString_Newf("(%r,)",DeeTuple_GET(self,0));
 unicode_printer_init(&p);
 if (unicode_printer_putascii(&p,'(') < 0) goto err;
 end = (iter = DeeTuple_ELEM(self))+DeeTuple_SIZE(self);
 for (; iter != end; ++iter) {
  /* Print this item. */
  if (iter != DeeTuple_ELEM(self) &&
      UNICODE_PRINTER_PRINT(&p,", ") < 0)
      goto err;
  if (unicode_printer_printobjectrepr(&p,*iter) < 0)
      goto err;
 }
 if (unicode_printer_putascii(&p,')') < 0) goto err;
 return unicode_printer_pack(&p);
err:
 unicode_printer_fini(&p);
 return NULL;
}

PRIVATE int DCALL
tuple_bool(Tuple *__restrict self) {
 return !DeeTuple_IsEmpty(self);
}

PRIVATE dhash_t DCALL
tuple_hash(Tuple *__restrict self) {
 DeeObject **iter,**end; dhash_t result;
 result = DeeTuple_SIZE(self);
 end = (iter = DeeTuple_ELEM(self))+DeeTuple_SIZE(self);
 for (; iter != end; ++iter) result ^= DeeObject_Hash(*iter);
 return result;
}

PRIVATE DREF DeeObject *DCALL
tuple_sizeof(Tuple *__restrict self,
             size_t argc, DeeObject **__restrict argv) {
 if (DeeArg_Unpack(argc,argv,":__sizeof__"))
     return NULL;
 return DeeInt_NewSize(offsetof(Tuple,t_elem)+
                       self->t_size*
                       sizeof(DeeObject *));
}

PRIVATE DREF DeeObject *DCALL
tuple_front(Tuple *__restrict self,
            size_t argc, DeeObject **__restrict argv) {
 if (DeeArg_Unpack(argc,argv,":front"))
     return NULL;
 if unlikely(DeeTuple_IsEmpty(self)) {
  err_empty_sequence((DeeObject *)self);
  return NULL;
 }
 return_reference_(DeeTuple_GET(self,0));
}
PRIVATE DREF DeeObject *DCALL
tuple_back(Tuple *__restrict self,
           size_t argc, DeeObject **__restrict argv) {
 if (DeeArg_Unpack(argc,argv,":back"))
     return NULL;
 if unlikely(DeeTuple_IsEmpty(self)) {
  err_empty_sequence((DeeObject *)self);
  return NULL;
 }
 return_reference_(DeeTuple_GET(self,DeeTuple_SIZE(self)-1));
}

PRIVATE struct type_method tuple_methods[] = {
    { "__sizeof__", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&tuple_sizeof },
    { "front", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&tuple_front },
    { "back", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&tuple_back },
    { NULL }
};

PRIVATE DREF DeeObject *DCALL
tuple_eq(Tuple *__restrict self, DeeObject *__restrict other) {
 int result = DeeSeq_EqVS(DeeTuple_ELEM(self),DeeTuple_SIZE(self),other);
 if unlikely(result < 0) return NULL;
 return_bool_(result);
}
PRIVATE DREF DeeObject *DCALL
tuple_ne(Tuple *__restrict self, DeeObject *__restrict other) {
 int result = DeeSeq_EqVS(DeeTuple_ELEM(self),DeeTuple_SIZE(self),other);
 if unlikely(result < 0) return NULL;
 return_bool_(!result);
}
PRIVATE DREF DeeObject *DCALL
tuple_lo(Tuple *__restrict self, DeeObject *__restrict other) {
 int result = DeeSeq_LoVS(DeeTuple_ELEM(self),DeeTuple_SIZE(self),other);
 if unlikely(result < 0) return NULL;
 return_bool_(result);
}
PRIVATE DREF DeeObject *DCALL
tuple_le(Tuple *__restrict self, DeeObject *__restrict other) {
 int result = DeeSeq_LeVS(DeeTuple_ELEM(self),DeeTuple_SIZE(self),other);
 if unlikely(result < 0) return NULL;
 return_bool_(result);
}
PRIVATE DREF DeeObject *DCALL
tuple_gr(Tuple *__restrict self, DeeObject *__restrict other) {
 int result = DeeSeq_LeVS(DeeTuple_ELEM(self),DeeTuple_SIZE(self),other);
 if unlikely(result < 0) return NULL;
 return_bool_(!result);
}
PRIVATE DREF DeeObject *DCALL
tuple_ge(Tuple *__restrict self, DeeObject *__restrict other) {
 int result = DeeSeq_LoVS(DeeTuple_ELEM(self),DeeTuple_SIZE(self),other);
 if unlikely(result < 0) return NULL;
 return_bool_(!result);
}

PRIVATE struct type_cmp tuple_cmp = {
    /* .tp_hash = */(dhash_t(DCALL *)(DeeObject *__restrict))&tuple_hash,
    /* .tp_eq   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&tuple_eq,
    /* .tp_ne   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&tuple_ne,
    /* .tp_lo   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&tuple_lo,
    /* .tp_le   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&tuple_le,
    /* .tp_gr   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&tuple_gr,
    /* .tp_ge   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&tuple_ge,
};


PUBLIC DeeTypeObject DeeTuple_Type = {
    OBJECT_HEAD_INIT(&DeeType_Type),
    /* .tp_name     = */DeeString_STR(&str_tuple),
    /* .tp_doc      = */NULL,
    /* .tp_flags    = */TP_FNORMAL|TP_FVARIABLE|TP_FFINAL|TP_FNAMEOBJECT,
    /* .tp_weakrefs = */0,
    /* .tp_features = */TF_NONE,
    /* .tp_base     = */&DeeSeq_Type,
    /* .tp_init = */{
        {
            /* .tp_var = */{
                /* .tp_ctor      = */&tuple_ctor,
                /* .tp_copy_ctor = */&noop_varcopy,
                /* .tp_deep_ctor = */&noop_varcopy,
                /* .tp_any_ctor  = */&tuple_init,
#if TUPLE_CACHE_MAXCOUNT
                /* .tp_free      = */&tuple_tp_free,
#else
                /* .tp_free      = */NULL
#endif
            }
        },
        /* .tp_dtor        = */(void(DCALL *)(DeeObject *__restrict))&tuple_fini,
        /* .tp_assign      = */NULL,
        /* .tp_move_assign = */NULL
    },
    /* .tp_cast = */{
        /* .tp_str  = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&tuple_str,
        /* .tp_repr = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&tuple_repr,
        /* .tp_bool = */(int(DCALL *)(DeeObject *__restrict))&tuple_bool
    },
    /* .tp_call          = */NULL,
    /* .tp_visit         = */(void(DCALL *)(DeeObject *__restrict,dvisit_t,void *))&tuple_visit,
    /* .tp_gc            = */NULL,
    /* .tp_math          = */NULL,
    /* .tp_cmp           = */&tuple_cmp,
    /* .tp_seq           = */&tuple_seq,
    /* .tp_iter_next     = */NULL,
    /* .tp_attr          = */NULL,
    /* .tp_with          = */NULL,
    /* .tp_buffer        = */NULL,
    /* .tp_methods       = */tuple_methods,
    /* .tp_getsets       = */NULL,
    /* .tp_members       = */NULL,
    /* .tp_class_methods = */tuple_class_methods,
    /* .tp_class_getsets = */NULL,
    /* .tp_class_members = */tuple_class_members
};

PUBLIC struct empty_tuple_object DeeTuple_Empty = {
    4, /* +1 in contrast to OBJECT_HEAD_INIT for the reference
        * saved as the original value for `Dee_GetArgv()' */
    &DeeTuple_Type,
#ifdef CONFIG_TRACE_REFCHANGES
    DEE_REFTRACKER_UNTRACKED,
#endif
    0
};

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_TUPLE_C */
