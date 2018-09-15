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
#ifndef GUARD_DEEMON_OBJECTS_SEQ_SVEC_C
#define GUARD_DEEMON_OBJECTS_SEQ_SVEC_C 1

#include <deemon/api.h>
#include <deemon/object.h>
#include <deemon/seq.h>
#include <deemon/int.h>
#include <deemon/none.h>
#include <deemon/bool.h>
#include <deemon/error.h>
#include <deemon/arg.h>
#include <deemon/util/string.h>

#include "svec.h"
#include "../../runtime/runtime_error.h"

DECL_BEGIN

#ifdef CONFIG_NO_THREADS
#define RVI_GETPOS(x)            ((x)->rvi_pos)
#else
#define RVI_GETPOS(x) ATOMIC_READ((x)->rvi_pos)
#endif

PRIVATE int DCALL
rveciter_copy(RefVectorIterator *__restrict self,
              RefVectorIterator *__restrict other) {
 self->rvi_vector = other->rvi_vector;
 self->rvi_pos    = RVI_GETPOS(other);
 return 0;
}
PRIVATE int DCALL
rveciter_ctor(RefVectorIterator *__restrict self,
              size_t argc, DeeObject **__restrict argv) {
 if (DeeArg_Unpack(argc,argv,"o:_refvectoriterator",&self->rvi_vector) ||
     DeeObject_AssertTypeExact((DeeObject *)self->rvi_vector,&RefVector_Type))
     return -1;
 Dee_Incref(self->rvi_vector);
 self->rvi_pos = self->rvi_vector->rv_vector;
 return 0;
}

PRIVATE void DCALL
rveciter_fini(RefVectorIterator *__restrict self) {
 Dee_Decref(self->rvi_vector);
}
PRIVATE void DCALL
rveciter_visit(RefVectorIterator *__restrict self, dvisit_t proc, void *arg) {
 Dee_Visit(self->rvi_vector);
}

PRIVATE DREF DeeObject *DCALL
rveciter_next(RefVectorIterator *__restrict self) {
 DREF DeeObject **presult,*result;
 RefVector *vector = self->rvi_vector;
 for (;;) {
#ifdef CONFIG_NO_THREADS
  presult = self->rvi_pos;
  if (presult >= vector->rv_vector+vector->rv_length)
      return ITER_DONE;
  ++self->rvi_pos;
#else
  do {
   presult = ATOMIC_READ(self->rvi_pos);
   if (presult >= vector->rv_vector+vector->rv_length)
       return ITER_DONE;
  } while (!ATOMIC_CMPXCH_WEAK(self->rvi_pos,presult,presult+1));
#endif
#ifndef CONFIG_NO_THREADS
  if (vector->rv_plock)
      rwlock_read(vector->rv_plock);
#endif
  result = *presult;
  Dee_XIncref(result);
#ifndef CONFIG_NO_THREADS
  if (vector->rv_plock)
      rwlock_endread(vector->rv_plock);
#endif
  /* Skip NULL entires. */
  if (result) break;
 }
 return result;
}

PRIVATE int DCALL
rveciter_bool(RefVectorIterator *__restrict self) {
 RefVector *vector = self->rvi_vector;
 if (RVI_GETPOS(self) >= vector->rv_vector+vector->rv_length)
     return 0;
 return 1;
}

PRIVATE DREF DeeObject *DCALL
rveciter_eq(RefVectorIterator *__restrict self,
            RefVectorIterator *__restrict other) {
 if (DeeObject_AssertTypeExact((DeeObject *)other,&RefVectorIterator_Type))
     return NULL;
 return_bool_(self->rvi_vector == other->rvi_vector &&
              RVI_GETPOS(self) == RVI_GETPOS(other));
}
PRIVATE DREF DeeObject *DCALL
rveciter_ne(RefVectorIterator *__restrict self,
            RefVectorIterator *__restrict other) {
 if (DeeObject_AssertTypeExact((DeeObject *)other,&RefVectorIterator_Type))
     return NULL;
 return_bool_(self->rvi_vector != other->rvi_vector ||
              RVI_GETPOS(self) != RVI_GETPOS(other));
}
PRIVATE DREF DeeObject *DCALL
rveciter_lo(RefVectorIterator *__restrict self,
            RefVectorIterator *__restrict other) {
 if (DeeObject_AssertTypeExact((DeeObject *)other,&RefVectorIterator_Type))
     return NULL;
 return_bool_(self->rvi_vector == other->rvi_vector
            ? RVI_GETPOS(self) < RVI_GETPOS(other)
            : self->rvi_vector < other->rvi_vector);
}
PRIVATE DREF DeeObject *DCALL
rveciter_le(RefVectorIterator *__restrict self,
            RefVectorIterator *__restrict other) {
 if (DeeObject_AssertTypeExact((DeeObject *)other,&RefVectorIterator_Type))
     return NULL;
 return_bool_(self->rvi_vector == other->rvi_vector
            ? RVI_GETPOS(self) <= RVI_GETPOS(other)
            : self->rvi_vector <= other->rvi_vector);
}
PRIVATE DREF DeeObject *DCALL
rveciter_gr(RefVectorIterator *__restrict self,
            RefVectorIterator *__restrict other) {
 if (DeeObject_AssertTypeExact((DeeObject *)other,&RefVectorIterator_Type))
     return NULL;
 return_bool_(self->rvi_vector == other->rvi_vector
            ? RVI_GETPOS(self) > RVI_GETPOS(other)
            : self->rvi_vector > other->rvi_vector);
}
PRIVATE DREF DeeObject *DCALL
rveciter_ge(RefVectorIterator *__restrict self,
            RefVectorIterator *__restrict other) {
 if (DeeObject_AssertTypeExact((DeeObject *)other,&RefVectorIterator_Type))
     return NULL;
 return_bool_(self->rvi_vector == other->rvi_vector
            ? RVI_GETPOS(self) >= RVI_GETPOS(other)
            : self->rvi_vector >= other->rvi_vector);
}

PRIVATE struct type_cmp rveciter_cmp = {
    /* .tp_hash = */NULL,
    /* .tp_eq   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&rveciter_eq,
    /* .tp_ne   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&rveciter_ne,
    /* .tp_lo   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&rveciter_lo,
    /* .tp_le   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&rveciter_le,
    /* .tp_gr   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&rveciter_gr,
    /* .tp_ge   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&rveciter_ge,
};

PRIVATE struct type_member rveciter_members[] = {
    TYPE_MEMBER_FIELD("seq",STRUCT_OBJECT,offsetof(RefVectorIterator,rvi_vector)),
    TYPE_MEMBER_END
};

INTERN DeeTypeObject RefVectorIterator_Type = {
    OBJECT_HEAD_INIT(&DeeType_Type),
    /* .tp_name     = */"_refvectoriterator",
    /* .tp_doc      = */NULL,
    /* .tp_flags    = */TP_FNORMAL|TP_FFINAL,
    /* .tp_weakrefs = */0,
    /* .tp_features = */TF_NONE,
    /* .tp_base     = */&DeeIterator_Type,
    /* .tp_init = */{
        {
            /* .tp_alloc = */{
                /* .tp_ctor      = */NULL,
                /* .tp_copy_ctor = */&rveciter_copy,
                /* .tp_deep_ctor = */NULL,
                /* .tp_any_ctor  = */&rveciter_ctor,
                /* .tp_free      = */NULL,
                {
                    /* .tp_instance_size = */sizeof(RefVectorIterator)
                }
            }
        },
        /* .tp_dtor        = */(void(DCALL *)(DeeObject *__restrict))&rveciter_fini,
        /* .tp_assign      = */NULL,
        /* .tp_move_assign = */NULL
    },
    /* .tp_cast = */{
        /* .tp_str  = */NULL,
        /* .tp_repr = */NULL,
        /* .tp_bool = */(int(DCALL *)(DeeObject *__restrict))&rveciter_bool
    },
    /* .tp_call          = */NULL,
    /* .tp_visit         = */(void(DCALL *)(DeeObject *__restrict,dvisit_t,void*))&rveciter_visit,
    /* .tp_gc            = */NULL,
    /* .tp_math          = */NULL,
    /* .tp_cmp           = */&rveciter_cmp,
    /* .tp_seq           = */NULL,
    /* .tp_iter_next     = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&rveciter_next,
    /* .tp_attr          = */NULL,
    /* .tp_with          = */NULL,
    /* .tp_buffer        = */NULL,
    /* .tp_methods       = */NULL,
    /* .tp_getsets       = */NULL,
    /* .tp_members       = */rveciter_members,
    /* .tp_class_methods = */NULL,
    /* .tp_class_getsets = */NULL,
    /* .tp_class_members = */NULL
};


PRIVATE void DCALL
rvec_fini(RefVector *__restrict self) {
 Dee_Decref(self->rv_owner);
}

PRIVATE void DCALL
rvec_visit(RefVector *__restrict self, dvisit_t proc, void *arg) {
 Dee_Visit(self->rv_owner);
}

PRIVATE int DCALL
rvec_bool(RefVector *__restrict self) {
 return self->rv_length != 0;
}

PRIVATE DREF RefVectorIterator *DCALL
rvec_iter(RefVector *__restrict self) {
 DREF RefVectorIterator *result;
 result = DeeObject_MALLOC(RefVectorIterator);
 if unlikely(!result) goto done;
 DeeObject_Init(result,&RefVectorIterator_Type);
 Dee_Incref(self); /* Reference stored in `rvi_vector' */
 result->rvi_vector = self;
 result->rvi_pos    = self->rv_vector; /* Start at index 0 */
done:
 return result;
}

PRIVATE DREF DeeObject *DCALL
rvec_size(RefVector *__restrict self) {
 return DeeInt_NewSize(self->rv_length);
}

PRIVATE DREF DeeObject *DCALL
rvec_contains(RefVector *__restrict self,
              DeeObject *__restrict other) {
 size_t index; int temp;
 for (index = 0; index < self->rv_length; ++index) {
  DREF DeeObject *item;
#ifndef CONFIG_NO_THREADS
  if (self->rv_plock)
      rwlock_read(self->rv_plock);
#endif
  item = self->rv_vector[index];
  Dee_XIncref(item);
#ifndef CONFIG_NO_THREADS
  if (self->rv_plock)
      rwlock_endread(self->rv_plock);
#endif
  if (!item) continue;
  temp = DeeObject_CompareEq(item,other);
  Dee_Decref(item);
  if (temp != 0) {
   if unlikely(temp < 0)
      return NULL;
   return_true;
  }
 }
 return_false;
}

PRIVATE DREF DeeObject *DCALL
rvec_getitem(RefVector *__restrict self,
             DeeObject *__restrict index_ob) {
 size_t index;
 DREF DeeObject *result;
 if (DeeObject_AsSize(index_ob,&index))
     return NULL;
 if unlikely(index >= self->rv_length) {
  err_index_out_of_bounds((DeeObject *)self,
                           index,
                           self->rv_length);
  goto err;
 }
#ifndef CONFIG_NO_THREADS
 if (self->rv_plock)
     rwlock_read(self->rv_plock);
#endif
 result = self->rv_vector[index];
 if unlikely(!result) {
#ifndef CONFIG_NO_THREADS
  if (self->rv_plock)
      rwlock_endread(self->rv_plock);
#endif
  err_unbound_index((DeeObject *)self,index);
  goto err;
 }
 Dee_Incref(result);
#ifndef CONFIG_NO_THREADS
 if (self->rv_plock)
     rwlock_endread(self->rv_plock);
#endif
 return result;
err:
 return NULL;
}

PRIVATE ATTR_COLD int DCALL err_readonly_rvec(void) {
 return DeeError_Throwf(&DeeError_ValueError,
                        "Reference vector is not writable");
}


PRIVATE int DCALL
rvec_setitem(RefVector *__restrict self,
             DeeObject *__restrict index_ob,
             DeeObject *value) {
 size_t index;
 DREF DeeObject *old_item;
#ifndef CONFIG_NO_THREADS
 if (!self->rv_plock)
#else
 if (!self->rv_writable)
#endif
 {
  err_readonly_rvec();
  goto err;
 }
 if (DeeObject_AsSize(index_ob,&index))
     return -1;
 if unlikely(index >= self->rv_length) {
  err_index_out_of_bounds((DeeObject *)self,
                           index,
                           self->rv_length);
  goto err;
 }
 Dee_XIncref(value); /* Value may be NULL for the delitem callback. */
#ifndef CONFIG_NO_THREADS
 rwlock_write(self->rv_plock);
#endif
 old_item = self->rv_vector[index];
 self->rv_vector[index] = value;
#ifndef CONFIG_NO_THREADS
 rwlock_endwrite(self->rv_plock);
#endif
 Dee_XDecref(old_item);
 return 0;
err:
 return -1;
}

PRIVATE int DCALL
rvec_delitem(RefVector *__restrict self,
             DeeObject *__restrict index_ob) {
 return rvec_setitem(self,index_ob,NULL);
}


PRIVATE size_t DCALL
rvec_nsi_getsize(RefVector *__restrict self) {
 ASSERT(self->rv_length != (size_t)-1);
 return self->rv_length;
}
PRIVATE DREF DeeObject *DCALL
rvec_nsi_getitem(RefVector *__restrict self, size_t index) {
 DREF DeeObject *result;
 if unlikely(index >= self->rv_length) {
  err_index_out_of_bounds((DeeObject *)self,index,self->rv_length);
  return NULL;
 }
#ifndef CONFIG_NO_THREADS
 if (self->rv_plock)
     rwlock_read(self->rv_plock);
#endif
 result = self->rv_vector[index];
 if unlikely(!result) {
#ifndef CONFIG_NO_THREADS
  if (self->rv_plock)
      rwlock_endread(self->rv_plock);
#endif
  err_unbound_index((DeeObject *)self,index);
  return NULL;
 }
 Dee_Incref(result);
#ifndef CONFIG_NO_THREADS
 if (self->rv_plock)
     rwlock_endread(self->rv_plock);
#endif
 return result;
}
PRIVATE int DCALL
rvec_nsi_delitem(RefVector *__restrict self, size_t index) {
 DREF DeeObject *oldobj;
 if unlikely(index >= self->rv_length)
    return err_index_out_of_bounds((DeeObject *)self,index,self->rv_length);
#ifndef CONFIG_NO_THREADS
 if (self->rv_plock)
  rwlock_write(self->rv_plock);
 else
#else
 if (!self->rv_writable)
#endif
 {
  return err_readonly_rvec();
 }
 oldobj = self->rv_vector[index];
 if unlikely(!oldobj) {
#ifndef CONFIG_NO_THREADS
  rwlock_endwrite(self->rv_plock);
#endif
  return err_unbound_index((DeeObject *)self,index);
 }
 self->rv_vector[index] = NULL;
#ifndef CONFIG_NO_THREADS
 rwlock_endwrite(self->rv_plock);
#endif
 Dee_Decref(oldobj);
 return 0;
}
PRIVATE void DCALL
rvec_nsi_delitem_fast(RefVector *__restrict self, size_t index) {
 DREF DeeObject *oldobj;
 ASSERT(index < self->rv_length);
#ifndef CONFIG_NO_THREADS
 ASSERT(self->rv_plock);
 rwlock_write(self->rv_plock);
#else
 ASSERT(self->rv_writable);
#endif
 oldobj = self->rv_vector[index];
 self->rv_vector[index] = NULL;
#ifndef CONFIG_NO_THREADS
 rwlock_endwrite(self->rv_plock);
#endif
 Dee_XDecref(oldobj);
}
PRIVATE void DCALL
rvec_nsi_setitem_fast(RefVector *__restrict self, size_t index,
                      /*inherit(always)*/DREF DeeObject *__restrict value) {
 DREF DeeObject *oldobj;
 ASSERT(index < self->rv_length);
#ifndef CONFIG_NO_THREADS
 ASSERT(self->rv_plock);
 rwlock_write(self->rv_plock);
#else
 ASSERT(self->rv_writable);
#endif
 oldobj = self->rv_vector[index];
 self->rv_vector[index] = value;
#ifndef CONFIG_NO_THREADS
 rwlock_endwrite(self->rv_plock);
#endif
 Dee_XDecref(oldobj);
}
PRIVATE int DCALL
rvec_nsi_setitem(RefVector *__restrict self, size_t index,
                 DeeObject *__restrict value) {
 DREF DeeObject *oldobj;
 if unlikely(index >= self->rv_length)
    return err_index_out_of_bounds((DeeObject *)self,index,self->rv_length);
#ifndef CONFIG_NO_THREADS
 if (self->rv_plock)
  rwlock_write(self->rv_plock);
 else
#else
 if (!self->rv_writable)
#endif
 {
  return err_readonly_rvec();
 }
 Dee_Incref(value);
 oldobj = self->rv_vector[index];
 self->rv_vector[index] = value;
#ifndef CONFIG_NO_THREADS
 rwlock_endwrite(self->rv_plock);
#endif
 Dee_XDecref(oldobj);
 return 0;
}
PRIVATE DREF DeeObject *DCALL
rvec_nsi_getitem_fast(RefVector *__restrict self, size_t index) {
 DREF DeeObject *result;
 ASSERT(index < self->rv_length);
#ifndef CONFIG_NO_THREADS
 if (self->rv_plock)
     rwlock_read(self->rv_plock);
#endif
 result = self->rv_vector[index];
 Dee_XIncref(result);
#ifndef CONFIG_NO_THREADS
 if (self->rv_plock)
     rwlock_endread(self->rv_plock);
#endif
 return result;
}

PRIVATE DREF DeeObject *DCALL
rvec_nsi_xchitem(RefVector *__restrict self, size_t index,
                 DeeObject *__restrict value) {
 DREF DeeObject *result;
 if unlikely(index >= self->rv_length) {
  err_index_out_of_bounds((DeeObject *)self,index,self->rv_length);
  goto err;
 }
#ifndef CONFIG_NO_THREADS
 if (self->rv_plock)
  rwlock_write(self->rv_plock);
 else
#else
 if (!self->rv_writable)
#endif
 {
  err_readonly_rvec();
  goto err;
 }
 result = self->rv_vector[index];
 if unlikely(!result) {
#ifndef CONFIG_NO_THREADS
  rwlock_endwrite(self->rv_plock);
#endif
  err_unbound_index((DeeObject *)self,index);
  goto err;
 }
 Dee_Incref(value);
 self->rv_vector[index] = value;
#ifndef CONFIG_NO_THREADS
 rwlock_endwrite(self->rv_plock);
#endif
 return result;
err:
 return NULL;
}

PRIVATE bool DCALL
rvec_nsi_cmpdelitem(RefVector *__restrict self, size_t index,
                    DeeObject *__restrict old_value) {
 ASSERT(index < self->rv_length);
#ifndef CONFIG_NO_THREADS
 ASSERT(self->rv_plock);
 rwlock_write(self->rv_plock);
#else
 ASSERT(self->rv_writable);
#endif
 if unlikely(self->rv_vector[index] != old_value) {
#ifndef CONFIG_NO_THREADS
  rwlock_endwrite(self->rv_plock);
#endif
  return false;
 }
 self->rv_vector[index] = NULL;
#ifndef CONFIG_NO_THREADS
 rwlock_endwrite(self->rv_plock);
#endif
 Dee_Decref(old_value);
 return true;
}

PRIVATE size_t DCALL
rvec_nsi_find(RefVector *__restrict self,
              size_t start, size_t end,
              DeeObject *__restrict keyed_search_item,
              DeeObject *key) {
 size_t i;
 DREF DeeObject *item; int temp;
 if (start > self->rv_length)
     start = self->rv_length;
 for (i = start; i < end; ++i) {
  item = rvec_nsi_getitem_fast(self,i);
  if (!item) continue; /* Unbound index */
  temp = DeeObject_CompareKeyEq(keyed_search_item,item,key);
  Dee_Decref(item);
  if (temp == 0) continue;
  if unlikely(temp < 0) goto err;
  return i;
 }
 return (size_t)-1;
err:
 return (size_t)-2;
}
PRIVATE size_t DCALL
rvec_nsi_rfind(RefVector *__restrict self,
               size_t start, size_t end,
               DeeObject *__restrict keyed_search_item,
               DeeObject *key) {
 size_t i;
 DREF DeeObject *item; int temp;
 if (start > self->rv_length)
     start = self->rv_length;
 i = end;
 while (i > start) {
  --i;
  item = rvec_nsi_getitem_fast(self,i);
  if (!item) continue; /* Unbound index */
  temp = DeeObject_CompareKeyEq(keyed_search_item,item,key);
  Dee_Decref(item);
  if (temp == 0) continue;
  if unlikely(temp < 0) goto err;
  return i;
 }
 return (size_t)-1;
err:
 return (size_t)-2;
}


PRIVATE int DCALL
rvec_nsi_remove(RefVector *__restrict self,
                size_t start, size_t end,
                DeeObject *__restrict keyed_search_item,
                DeeObject *key) {
 size_t i;
 DREF DeeObject *item; int temp;
 if (!RefVector_IsWritable(self))
      return err_readonly_rvec();
 if (start > self->rv_length)
     start = self->rv_length;
again:
 for (i = start; i < end; ++i) {
  item = rvec_nsi_getitem_fast(self,i);
  if (!item) continue; /* Unbound index */
  temp = DeeObject_CompareKeyEq(keyed_search_item,item,key);
  Dee_Decref(item);
  if (temp == 0) continue;
  if unlikely(temp < 0) goto err;
  /* Found the item. */
  if (!rvec_nsi_cmpdelitem(self,i,item))
       goto again;
  return 1;
 }
 return 0;
err:
 return -1;
}
PRIVATE int DCALL
rvec_nsi_rremove(RefVector *__restrict self,
                 size_t start, size_t end,
                 DeeObject *__restrict keyed_search_item,
                 DeeObject *key) {
 size_t i;
 DREF DeeObject *item; int temp;
 if (!RefVector_IsWritable(self))
      return err_readonly_rvec();
 if (start > self->rv_length)
     start = self->rv_length;
again:
 i = end;
 while (i > start) {
  --i;
  item = rvec_nsi_getitem_fast(self,i);
  if (!item) continue; /* Unbound index */
  temp = DeeObject_CompareKeyEq(keyed_search_item,item,key);
  Dee_Decref(item);
  if (temp == 0) continue;
  if unlikely(temp < 0) goto err;
  /* Found the item. */
  if (!rvec_nsi_cmpdelitem(self,i,item))
       goto again;
  return 1;
 }
 return 0;
err:
 return -1;
}
PRIVATE size_t DCALL
rvec_nsi_removeall(RefVector *__restrict self,
                   size_t start, size_t end,
                   DeeObject *__restrict keyed_search_item,
                   DeeObject *key) {
 size_t i; size_t result = 0;
 DREF DeeObject *item; int temp;
 if (!RefVector_IsWritable(self))
      return err_readonly_rvec();
 if (start > self->rv_length)
     start = self->rv_length;
again:
 for (i = start; i < end; ++i) {
  item = rvec_nsi_getitem_fast(self,i);
  if (!item) continue; /* Unbound index */
  temp = DeeObject_CompareKeyEq(keyed_search_item,item,key);
  Dee_Decref(item);
  if (temp == 0) continue;
  if unlikely(temp < 0) goto err;
  /* Found the item. */
  if (!rvec_nsi_cmpdelitem(self,i,item))
       goto again;
  ++result;
  if unlikely(result == (size_t)-2)
     break; /* Prevent overflows. */
 }
 return result;
err:
 return (size_t)-1;
}
PRIVATE size_t DCALL
rvec_nsi_removeif(RefVector *__restrict self,
                  size_t start, size_t end,
                  DeeObject *__restrict should_remove) {
 size_t i; size_t result = 0; int temp;
 DREF DeeObject *item,*callback_result;
 if (!RefVector_IsWritable(self))
      return err_readonly_rvec();
 if (start > self->rv_length)
     start = self->rv_length;
again:
 for (i = start; i < end; ++i) {
  item = rvec_nsi_getitem_fast(self,i);
  if (!item) continue; /* Unbound index */
  callback_result = DeeObject_Call(should_remove,1,&item);
  Dee_Decref(item);
  if unlikely(!callback_result) goto err;
  temp = DeeObject_Bool(callback_result);
  Dee_Decref(callback_result);
  if (temp == 0) continue;
  if unlikely(temp < 0) goto err;
  /* Found the item. */
  if (!rvec_nsi_cmpdelitem(self,i,item))
       goto again;
  ++result;
  if unlikely(result == (size_t)-2)
     break; /* Prevent overflows. */
 }
 return result;
err:
 return (size_t)-1;
}

PRIVATE int DCALL
rvec_nsi_delrange(RefVector *__restrict self,
                  dssize_t start, dssize_t end) {
 size_t i;
 if (!RefVector_IsWritable(self))
      return err_readonly_rvec();
 if (start < 0) start += self->rv_length;
 if (end < 0) end += self->rv_length;
 if ((size_t)end > self->rv_length) end = (dssize_t)self->rv_length;
 for (i = (size_t)start; i < (size_t)end; ++i)
    rvec_nsi_delitem_fast(self,i);
 return 0;
}

PRIVATE int DCALL
rvec_nsi_delrange_n(RefVector *__restrict self,
                    dssize_t start) {
 size_t i,end = self->rv_length;
 if (!RefVector_IsWritable(self))
      return err_readonly_rvec();
 if (start < 0) start += end;
 for (i = (size_t)start; i < end; ++i)
    rvec_nsi_delitem_fast(self,i);
 return 0;
}

PRIVATE int DCALL
rvec_nsi_setrange(RefVector *__restrict self,
                  dssize_t start, dssize_t end,
                  DeeObject *__restrict values) {
 size_t i,fast_length;
 DREF DeeObject *elem;
 if (DeeNone_Check(values))
     return rvec_nsi_delrange(self,start,end);
 if (!RefVector_IsWritable(self))
     return err_readonly_rvec();
 if (start < 0) start += self->rv_length;
 if (end < 0) end += self->rv_length;
 if ((size_t)end > self->rv_length)
     end = (dssize_t)self->rv_length;
 fast_length = DeeFastSeq_GetSize(values);
 if (fast_length != DEE_FASTSEQ_NOTFAST) {
  if (fast_length != ((size_t)end - (size_t)start)) {
   return err_invalid_unpack_size(values,
                                 (size_t)end - (size_t)start,
                                  fast_length);
  }
  for (i = (size_t)start; i < (size_t)end; ++i) {
   elem = DeeFastSeq_GetItem(values,i - (size_t)start);
   if unlikely(!elem) goto err;
   rvec_nsi_setitem_fast(self,i,elem); /* Inherit reference. */
  }
 } else {
  DREF DeeObject *iterator;
  iterator = DeeObject_IterSelf(values);
  if unlikely(!iterator) goto err;
  for (i = (size_t)start; i < (size_t)end; ++i) {
   elem = DeeObject_IterNext(iterator);
   if unlikely(!ITER_ISOK(elem)) {
    if unlikely(elem == ITER_DONE) {
     err_invalid_unpack_size(values,
                            (size_t)end - (size_t)start,
                             i - (size_t)start);
    }
err_iterator:
    Dee_Decref(iterator);
    goto err;
   }
   rvec_nsi_setitem_fast(self,i,elem); /* Inherit reference. */
  }
  /* Make sure that the given iterator ends here! */
  elem = DeeObject_IterNext(iterator);
  if unlikely(elem != ITER_DONE) {
   if (elem) {
    err_invalid_unpack_iter_size(values,iterator,(size_t)end - (size_t)start);
    Dee_Decref(elem);
   }
   goto err_iterator;
  }
  Dee_Decref(iterator);
 }
 return 0;
err:
 return -1;
}

PRIVATE int DCALL
rvec_nsi_setrange_n(RefVector *__restrict self,
                    dssize_t start,
                    DeeObject *__restrict values) {
 if (DeeNone_Check(values))
     return rvec_nsi_delrange_n(self,start);
 return rvec_nsi_setrange(self,start,
                         (size_t)self->rv_length,
                          values);
}

PRIVATE int DCALL
rvec_delrange(RefVector *__restrict self,
              DeeObject *__restrict start,
              DeeObject *__restrict end) {
 dssize_t start_index;
 dssize_t end_index;
 if (DeeObject_AsSSize(start,&start_index))
     goto err;
 if (DeeNone_Check(end))
     return rvec_nsi_delrange_n(self,start_index);
 if (DeeObject_AsSSize(end,&end_index))
     goto err;
 return rvec_nsi_delrange(self,start_index,end_index);
err:
 return -1;
}

PRIVATE int DCALL
rvec_setrange(RefVector *__restrict self,
              DeeObject *__restrict start,
              DeeObject *__restrict end,
              DeeObject *__restrict values) {
 dssize_t start_index;
 dssize_t end_index;
 if (DeeObject_AsSSize(start,&start_index))
     goto err;
 if (DeeNone_Check(end))
     return rvec_nsi_setrange_n(self,start_index,values);
 if (DeeObject_AsSSize(end,&end_index))
     goto err;
 return rvec_nsi_setrange(self,start_index,end_index,values);
err:
 return -1;
}


PRIVATE struct type_nsi rvec_nsi = {
    /* .nsi_class   = */TYPE_SEQX_CLASS_SEQ,
    /* .nsi_flags   = */TYPE_SEQX_FMUTABLE,
    {
        /* .nsi_seqlike = */{
            /* .nsi_getsize      = */(void *)&rvec_nsi_getsize,
            /* .nsi_getsize_fast = */(void *)&rvec_nsi_getsize,
            /* .nsi_getitem      = */(void *)&rvec_nsi_getitem,
            /* .nsi_delitem      = */(void *)&rvec_nsi_delitem,
            /* .nsi_setitem      = */(void *)&rvec_nsi_setitem,
            /* .nsi_getitem_fast = */(void *)&rvec_nsi_getitem_fast,
            /* .nsi_getrange     = */(void *)NULL,
            /* .nsi_getrange_n   = */(void *)NULL,
            /* .nsi_setrange     = */(void *)&rvec_nsi_setrange,
            /* .nsi_setrange_n   = */(void *)&rvec_nsi_setrange_n,
            /* .nsi_find         = */(void *)&rvec_nsi_find,
            /* .nsi_rfind        = */(void *)&rvec_nsi_rfind,
            /* .nsi_xch          = */(void *)&rvec_nsi_xchitem,
            /* .nsi_insert       = */(void *)NULL,
            /* .nsi_insertall    = */(void *)NULL,
            /* .nsi_insertvec    = */(void *)NULL,
            /* .nsi_pop          = */(void *)NULL,
            /* .nsi_erase        = */(void *)NULL,
            /* .nsi_remove       = */(void *)&rvec_nsi_remove,
            /* .nsi_rremove      = */(void *)&rvec_nsi_rremove,
            /* .nsi_removeall    = */(void *)&rvec_nsi_removeall,
            /* .nsi_removeif     = */(void *)&rvec_nsi_removeif
        }
    }
};


PRIVATE struct type_seq rvec_seq = {
    /* .tp_iter_self = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&rvec_iter,
    /* .tp_size      = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&rvec_size,
    /* .tp_contains  = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&rvec_contains,
    /* .tp_get       = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&rvec_getitem,
    /* .tp_del       = */(int(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&rvec_delitem,
    /* .tp_set       = */(int(DCALL *)(DeeObject *__restrict,DeeObject *__restrict,DeeObject *__restrict))&rvec_setitem,
    /* .tp_range_get = */NULL,
    /* .tp_range_del = */(int(DCALL *)(DeeObject *__restrict,DeeObject *__restrict,DeeObject *__restrict))&rvec_delrange,
    /* .tp_range_set = */(int(DCALL *)(DeeObject *__restrict,DeeObject *__restrict,DeeObject *__restrict,DeeObject *__restrict))&rvec_setrange,
    /* .tp_nsi       = */&rvec_nsi
};

PRIVATE struct type_member rvec_class_members[] = {
    TYPE_MEMBER_CONST("iterator",&RefVectorIterator_Type),
    TYPE_MEMBER_END
};

PRIVATE struct type_member rvec_members[] = {
    TYPE_MEMBER_FIELD("__owner__",STRUCT_OBJECT,offsetof(RefVector,rv_owner)),
    TYPE_MEMBER_FIELD("__length__",STRUCT_OBJECT,offsetof(RefVector,rv_length)),
    TYPE_MEMBER_END
};

PRIVATE DREF DeeObject *DCALL
rvec_get_writable(RefVector *__restrict self) {
#ifndef CONFIG_NO_THREADS
 return_bool_(self->rv_plock != NULL);
#else
 return_bool_(self->rv_writable);
#endif
}

PRIVATE struct type_getset rvec_getsets[] = {
    { "__writable__",
     (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&rvec_get_writable,
      NULL, NULL, DOC("->bool") },
    { NULL }
};

PRIVATE int DCALL
rvec_init(RefVector *__restrict self) {
 self->rv_owner = Dee_None;
 Dee_Incref(Dee_None);
 self->rv_length = 0;
 self->rv_vector = NULL;
#ifndef CONFIG_NO_THREADS
 self->rv_plock = NULL;
#else
 self->rv_writable = false;
#endif
 return 0;
}

PRIVATE int DCALL
rvec_copy(RefVector *__restrict self,
          RefVector *__restrict other) {
 self->rv_owner = other->rv_owner;
 Dee_Incref(self->rv_owner);
 self->rv_length   = other->rv_length;
 self->rv_vector   = other->rv_vector;
#ifndef CONFIG_NO_THREADS
 self->rv_plock    = other->rv_plock;
#else
 self->rv_writable = other->rv_writable;
#endif
 return 0;
}

INTERN DeeTypeObject RefVector_Type = {
    OBJECT_HEAD_INIT(&DeeType_Type),
    /* .tp_name     = */"_refvector",
    /* .tp_doc      = */NULL,
    /* .tp_flags    = */TP_FNORMAL|TP_FFINAL,
    /* .tp_weakrefs = */0,
    /* .tp_features = */TF_NONE,
    /* .tp_base     = */&DeeSeq_Type,
    /* .tp_init = */{
        {
            /* .tp_alloc = */{
                /* .tp_ctor      = */&rvec_init,
                /* .tp_copy_ctor = */&rvec_copy,
                /* .tp_deep_ctor = */NULL,
                /* .tp_any_ctor  = */NULL,
                /* .tp_free      = */NULL,
                {
                    /* .tp_instance_size = */sizeof(RefVector)
                }
            }
        },
        /* .tp_dtor        = */(void(DCALL *)(DeeObject *__restrict))&rvec_fini,
        /* .tp_assign      = */NULL,
        /* .tp_move_assign = */NULL
    },
    /* .tp_cast = */{
        /* .tp_str  = */NULL,
        /* .tp_repr = */NULL,
        /* .tp_bool = */(int(DCALL *)(DeeObject *__restrict))&rvec_bool
    },
    /* .tp_call          = */NULL,
    /* .tp_visit         = */(void(DCALL *)(DeeObject *__restrict,dvisit_t,void*))&rvec_visit,
    /* .tp_gc            = */NULL,
    /* .tp_math          = */NULL,
    /* .tp_cmp           = */NULL,
    /* .tp_seq           = */&rvec_seq,
    /* .tp_iter_next     = */NULL,
    /* .tp_attr          = */NULL,
    /* .tp_with          = */NULL,
    /* .tp_buffer        = */NULL,
    /* .tp_methods       = */NULL,
    /* .tp_getsets       = */rvec_getsets,
    /* .tp_members       = */rvec_members,
    /* .tp_class_methods = */NULL,
    /* .tp_class_getsets = */NULL,
    /* .tp_class_members = */rvec_class_members
};

PUBLIC DREF DeeObject *DCALL
DeeRefVector_New(DeeObject *__restrict owner, size_t length,
                 DeeObject **__restrict vector,
#ifndef CONFIG_NO_THREADS
                 rwlock_t *plock
#else
                 bool writable
#endif
                 ) {
 DREF RefVector *result;
 ASSERT_OBJECT(owner);
 ASSERT(!length || vector);
 result = DeeObject_MALLOC(RefVector);
 if unlikely(!result) goto done;
 DeeObject_Init(result,&RefVector_Type);
 Dee_Incref(owner); /* Create the reference for `rv_owner' */
 result->rv_length   = length;
 result->rv_vector   = vector;
 result->rv_owner    = owner;
#ifndef CONFIG_NO_THREADS
 result->rv_plock    = plock;
#else
 result->rv_writable = writable;
#endif
done:
 return (DREF DeeObject *)result;
}




PRIVATE int DCALL
sveciter_ctor(SharedVectorIterator *__restrict self,
              size_t argc, DeeObject **__restrict argv) {
 if (DeeArg_Unpack(argc,argv,"o:_sharedvectoriterator",&self->si_seq) ||
     DeeObject_AssertTypeExact((DeeObject *)self->si_seq,&SharedVector_Type))
     return -1;
 Dee_Incref(self->si_seq);
 self->si_index = 0;
 return 0;
}
INTERN void DCALL
sveciter_fini(SharedVectorIterator *__restrict self) {
 Dee_Decref(self->si_seq);
}
INTERN void DCALL
sveciter_visit(SharedVectorIterator *__restrict self, dvisit_t proc, void *arg) {
 Dee_Visit(self->si_seq);
}
INTERN DREF DeeObject *DCALL
sveciter_next(SharedVectorIterator *__restrict self) {
 DREF DeeObject *result = ITER_DONE;
 SharedVector *vec = self->si_seq;
#ifndef CONFIG_NO_THREADS
 for (;;) {
  size_t index;
  rwlock_read(&vec->sv_lock);
  index = ATOMIC_READ(self->si_index);
  if (self->si_index >= vec->sv_length) {
   rwlock_endread(&vec->sv_lock);
   break;
  }
  result = vec->sv_vector[index];
  /* Acquire a reference to keep the item alive. */
  Dee_Incref(result);
  rwlock_endread(&vec->sv_lock);
  if (ATOMIC_CMPXCH(self->si_index,index,index+1))
      break;
  /* If some other thread stole the index, drop their value. */
  Dee_Decref(result);
 }
#else
 if (self->si_index < vec->sv_length) {
  result = vec->sv_vector[self->si_index++];
  Dee_Incref(result);
 }
#endif
 return result;
}

INTERN int DCALL
sveciter_bool(SharedVectorIterator *__restrict self) {
#ifdef CONFIG_NO_THREADS
 return self->si_index < self->si_seq->sv_length;
#else
 return (ATOMIC_READ(self->si_index) <
         ATOMIC_READ(self->si_seq->sv_length));
#endif
}
INTERN int DCALL
sveciter_copy(SharedVectorIterator *__restrict self,
              SharedVectorIterator *__restrict other) {
#ifdef CONFIG_NO_THREADS
 self->si_index = other->si_index;
#else
 self->si_index = ATOMIC_READ(other->si_index);
#endif
 self->si_seq   = other->si_seq;
 Dee_Incref(self->si_seq);
 return 0;
}

INTERN struct type_member sveciter_members[] = {
    TYPE_MEMBER_FIELD("seq",STRUCT_OBJECT,offsetof(SharedVectorIterator,si_seq)),
    TYPE_MEMBER_END
};

#ifdef CONFIG_NO_THREADS
#define READ_INDEX(x)            ((x)->si_index)
#else
#define READ_INDEX(x) ATOMIC_READ((x)->si_index)
#endif

PRIVATE DREF DeeObject *DCALL
sveciter_eq(SharedVectorIterator *__restrict self,
            SharedVectorIterator *__restrict other) {
 if (DeeObject_AssertTypeExact((DeeObject *)other,&SharedVectorIterator_Type))
     return NULL;
 return_bool_(self->si_seq == other->si_seq &&
              READ_INDEX(self) == READ_INDEX(other));
}
PRIVATE DREF DeeObject *DCALL
sveciter_ne(SharedVectorIterator *__restrict self,
            SharedVectorIterator *__restrict other) {
 if (DeeObject_AssertTypeExact((DeeObject *)other,&SharedVectorIterator_Type))
     return NULL;
 return_bool_(self->si_seq != other->si_seq ||
              READ_INDEX(self) != READ_INDEX(other));
}
PRIVATE DREF DeeObject *DCALL
sveciter_lo(SharedVectorIterator *__restrict self,
            SharedVectorIterator *__restrict other) {
 if (DeeObject_AssertTypeExact((DeeObject *)other,&SharedVectorIterator_Type))
     return NULL;
 return_bool_(self->si_seq == other->si_seq
            ? READ_INDEX(self) < READ_INDEX(other)
            : self->si_seq < other->si_seq);
}
PRIVATE DREF DeeObject *DCALL
sveciter_le(SharedVectorIterator *__restrict self,
            SharedVectorIterator *__restrict other) {
 if (DeeObject_AssertTypeExact((DeeObject *)other,&SharedVectorIterator_Type))
     return NULL;
 return_bool_(self->si_seq == other->si_seq
            ? READ_INDEX(self) <= READ_INDEX(other)
            : self->si_seq <= other->si_seq);
}
PRIVATE DREF DeeObject *DCALL
sveciter_gr(SharedVectorIterator *__restrict self,
            SharedVectorIterator *__restrict other) {
 if (DeeObject_AssertTypeExact((DeeObject *)other,&SharedVectorIterator_Type))
     return NULL;
 return_bool_(self->si_seq == other->si_seq
            ? READ_INDEX(self) > READ_INDEX(other)
            : self->si_seq > other->si_seq);
}
PRIVATE DREF DeeObject *DCALL
sveciter_ge(SharedVectorIterator *__restrict self,
            SharedVectorIterator *__restrict other) {
 if (DeeObject_AssertTypeExact((DeeObject *)other,&SharedVectorIterator_Type))
     return NULL;
 return_bool_(self->si_seq == other->si_seq
            ? READ_INDEX(self) >= READ_INDEX(other)
            : self->si_seq >= other->si_seq);
}
INTERN struct type_cmp sveciter_cmp = {
    /* .tp_hash = */NULL,
    /* .tp_eq   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&sveciter_eq,
    /* .tp_ne   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&sveciter_ne,
    /* .tp_lo   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&sveciter_lo,
    /* .tp_le   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&sveciter_le,
    /* .tp_gr   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&sveciter_gr,
    /* .tp_ge   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&sveciter_ge,
};



INTERN DeeTypeObject SharedVectorIterator_Type = {
    OBJECT_HEAD_INIT(&DeeType_Type),
    /* .tp_name     = */"_sharedvectoriterator",
    /* .tp_doc      = */NULL,
    /* .tp_flags    = */TP_FNORMAL|TP_FFINAL,
    /* .tp_weakrefs = */0,
    /* .tp_features = */TF_NONE,
    /* .tp_base     = */&DeeIterator_Type,
    /* .tp_init = */{
        {
            /* .tp_alloc = */{
                /* .tp_ctor      = */NULL,
                /* .tp_copy_ctor = */&sveciter_copy,
                /* .tp_deep_ctor = */NULL,
                /* .tp_any_ctor  = */&sveciter_ctor,
                /* .tp_free      = */NULL,
                {
                    /* .tp_instance_size = */sizeof(SharedVectorIterator)
                }
            }
        },
        /* .tp_dtor        = */(void(DCALL *)(DeeObject *__restrict))&sveciter_fini,
        /* .tp_assign      = */NULL,
        /* .tp_move_assign = */NULL
    },
    /* .tp_cast = */{
        /* .tp_str  = */NULL,
        /* .tp_repr = */NULL,
        /* .tp_bool = */(int(DCALL *)(DeeObject *__restrict))&sveciter_bool
    },
    /* .tp_call          = */NULL,
    /* .tp_visit         = */(void(DCALL *)(DeeObject *__restrict,dvisit_t,void*))&sveciter_visit,
    /* .tp_gc            = */NULL,
    /* .tp_math          = */NULL,
    /* .tp_cmp           = */&sveciter_cmp,
    /* .tp_seq           = */NULL,
    /* .tp_iter_next     = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&sveciter_next,
    /* .tp_attr          = */NULL,
    /* .tp_with          = */NULL,
    /* .tp_buffer        = */NULL,
    /* .tp_methods       = */NULL,
    /* .tp_getsets       = */NULL,
    /* .tp_members       = */sveciter_members,
    /* .tp_class_methods = */NULL,
    /* .tp_class_getsets = */NULL,
    /* .tp_class_members = */NULL
};

PRIVATE void DCALL
svec_fini(SharedVector *__restrict self) {
 DREF DeeObject **begin,**iter;
 iter = (begin = self->sv_vector)+self->sv_length;
 while (iter-- != begin) Dee_Decref(*iter);
 Dee_Free(begin);
}

PRIVATE void DCALL
svec_visit(SharedVector *__restrict self, dvisit_t proc, void *arg) {
 DREF DeeObject **begin,**iter;
 iter = (begin = self->sv_vector)+self->sv_length;
 while (iter-- != begin) Dee_Visit(*iter);
}

PRIVATE DREF SharedVectorIterator *DCALL
svec_iter(SharedVector *__restrict self) {
 DREF SharedVectorIterator *result;
 result = DeeObject_MALLOC(SharedVectorIterator);
 if unlikely(!result) goto done;
 DeeObject_Init(result,&SharedVectorIterator_Type);
 Dee_Incref(self);
 result->si_seq   = self;
 result->si_index = 0;
done:
 return result;
}

PRIVATE DREF DeeObject *DCALL
svec_size(SharedVector *__restrict self) {
#ifdef CONFIG_NO_THREADS
 size_t result = self->sv_length;
#else
 size_t result = ATOMIC_READ(self->sv_length);
#endif
 return DeeInt_NewSize(result);
}

PRIVATE DREF DeeObject *DCALL
svec_contains(SharedVector *__restrict self,
              DeeObject *__restrict other) {
 size_t index; int temp;
#ifndef CONFIG_NO_THREADS
 rwlock_read(&self->sv_lock);
#endif
 for (index = 0; index < self->sv_length; ++index) {
  DREF DeeObject *item;
  item = self->sv_vector[index];
  Dee_Incref(item);
#ifndef CONFIG_NO_THREADS
  rwlock_endread(&self->sv_lock);
#endif
  temp = DeeObject_CompareEq(item,other);
  Dee_Decref(item);
  if (temp != 0) {
   if unlikely(temp < 0) return NULL;
   return_true;
  }
#ifndef CONFIG_NO_THREADS
  rwlock_read(&self->sv_lock);
#endif
 }
#ifndef CONFIG_NO_THREADS
 rwlock_endread(&self->sv_lock);
#endif
 return_false;
}

PRIVATE DREF DeeObject *DCALL
svec_getitem(SharedVector *__restrict self,
             DeeObject *__restrict index_ob) {
 size_t index;
 DREF DeeObject *result;
 if (DeeObject_AsSize(index_ob,&index))
     return NULL;
#ifndef CONFIG_NO_THREADS
 rwlock_read(&self->sv_lock);
#endif
 if unlikely(index >= self->sv_length) {
  size_t my_length = self->sv_length;
#ifndef CONFIG_NO_THREADS
  rwlock_endread(&self->sv_lock);
#endif
  err_index_out_of_bounds((DeeObject *)self,index,my_length);
  return NULL;
 }
 result = self->sv_vector[index];
 Dee_Incref(result);
#ifndef CONFIG_NO_THREADS
 rwlock_endread(&self->sv_lock);
#endif
 return result;
}


PRIVATE size_t DCALL
svec_nsi_getsize(SharedVector *__restrict self) {
 ASSERT(self->sv_length != (size_t)-1);
#ifdef CONFIG_NO_THREADS
 return self->sv_length;
#else
 return ATOMIC_READ(self->sv_length);
#endif
}
PRIVATE DREF DeeObject *DCALL
svec_nsi_getitem(SharedVector *__restrict self, size_t index) {
 DREF DeeObject *result;
 rwlock_read(&self->sv_lock);
 if unlikely(index >= self->sv_length) {
  size_t my_length = self->sv_length;
  rwlock_endread(&self->sv_lock);
  err_index_out_of_bounds((DeeObject *)self,index,my_length);
  return NULL;
 }
 result = self->sv_vector[index];
 Dee_Incref(result);
 rwlock_endread(&self->sv_lock);
 return result;
}
PRIVATE DREF DeeObject *DCALL
svec_nsi_getitem_fast(SharedVector *__restrict self, size_t index) {
 DREF DeeObject *result;
 rwlock_read(&self->sv_lock);
 if unlikely(index >= self->sv_length) {
  rwlock_endread(&self->sv_lock);
  return NULL;
 }
 result = self->sv_vector[index];
 Dee_Incref(result);
 rwlock_endread(&self->sv_lock);
 return result;
}


PRIVATE struct type_nsi svec_nsi = {
    /* .nsi_class   = */TYPE_SEQX_CLASS_SEQ,
    /* .nsi_flags   = */TYPE_SEQX_FNORMAL,
    {
        /* .nsi_seqlike = */{
            /* .nsi_getsize      = */(void *)&svec_nsi_getsize,
            /* .nsi_getsize_fast = */(void *)&svec_nsi_getsize,
            /* .nsi_getitem      = */(void *)&svec_nsi_getitem,
            /* .nsi_delitem      = */(void *)NULL,
            /* .nsi_setitem      = */(void *)NULL,
            /* .nsi_getitem_fast = */(void *)&svec_nsi_getitem_fast,
            /* .nsi_getrange     = */(void *)NULL,
            /* .nsi_getrange_n   = */(void *)NULL,
            /* .nsi_setrange     = */(void *)NULL,
            /* .nsi_setrange_n   = */(void *)NULL,
            /* .nsi_find         = */(void *)NULL, /* TODO */
            /* .nsi_rfind        = */(void *)NULL, /* TODO */
            /* .nsi_xch          = */(void *)NULL,
            /* .nsi_insert       = */(void *)NULL,
            /* .nsi_insertall    = */(void *)NULL,
            /* .nsi_insertvec    = */(void *)NULL,
            /* .nsi_pop          = */(void *)NULL,
            /* .nsi_erase        = */(void *)NULL,
            /* .nsi_remove       = */(void *)NULL,
            /* .nsi_rremove      = */(void *)NULL,
            /* .nsi_removeall    = */(void *)NULL,
            /* .nsi_removeif     = */(void *)NULL
        }
    }
};

PRIVATE struct type_seq svec_seq = {
    /* .tp_iter_self = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&svec_iter,
    /* .tp_size      = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&svec_size,
    /* .tp_contains  = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&svec_contains,
    /* .tp_get       = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&svec_getitem,
    /* .tp_del       = */NULL,
    /* .tp_set       = */NULL,
    /* .tp_range_get = */NULL,
    /* .tp_range_del = */NULL,
    /* .tp_range_set = */NULL,
    /* .tp_nsi       = */&svec_nsi,
};

PRIVATE struct type_member svec_class_members[] = {
    TYPE_MEMBER_CONST("iterator",&SharedVectorIterator_Type),
    TYPE_MEMBER_END
};

PRIVATE int DCALL
svec_ctor(SharedVector *__restrict self) {
#ifndef CONFIG_NO_THREADS
 rwlock_init(&self->sv_lock);
#endif
 self->sv_length = 0;
 self->sv_vector = NULL;
 return 0;
}

PRIVATE int DCALL
svec_bool(SharedVector *__restrict self) {
#ifdef CONFIG_NO_THREADS
 return self->sv_length != 0;
#else
 return ATOMIC_READ(self->sv_length) != 0;
#endif
}

INTERN DeeTypeObject SharedVector_Type = {
    OBJECT_HEAD_INIT(&DeeType_Type),
    /* .tp_name     = */"_sharedvector",
    /* .tp_doc      = */NULL,
    /* .tp_flags    = */TP_FNORMAL|TP_FFINAL,
    /* .tp_weakrefs = */0,
    /* .tp_features = */TF_NONE,
    /* .tp_base     = */&DeeSeq_Type,
    /* .tp_init = */{
        {
            /* .tp_alloc = */{
                /* .tp_ctor      = */&svec_ctor,
                /* .tp_copy_ctor = */NULL,
                /* .tp_deep_ctor = */NULL,
                /* .tp_any_ctor  = */NULL,
                /* .tp_free      = */NULL,
                {
                    /* .tp_instance_size = */sizeof(SharedVector)
                }
            }
        },
        /* .tp_dtor        = */(void(DCALL *)(DeeObject *__restrict))&svec_fini,
        /* .tp_assign      = */NULL,
        /* .tp_move_assign = */NULL
    },
    /* .tp_cast = */{
        /* .tp_str  = */NULL,
        /* .tp_repr = */NULL,
        /* .tp_bool = */(int(DCALL *)(DeeObject *__restrict))&svec_bool
    },
    /* .tp_call          = */NULL,
    /* .tp_visit         = */(void(DCALL *)(DeeObject *__restrict,dvisit_t,void*))&svec_visit,
    /* .tp_gc            = */NULL,
    /* .tp_math          = */NULL,
    /* .tp_cmp           = */NULL,
    /* .tp_seq           = */&svec_seq,
    /* .tp_iter_next     = */NULL,
    /* .tp_attr          = */NULL,
    /* .tp_with          = */NULL,
    /* .tp_buffer        = */NULL,
    /* .tp_methods       = */NULL,
    /* .tp_getsets       = */NULL,
    /* .tp_members       = */NULL,
    /* .tp_class_methods = */NULL,
    /* .tp_class_getsets = */NULL,
    /* .tp_class_members = */svec_class_members
};

INTERN DREF SharedVector *DCALL
SharedVector_NewShared(size_t length, DREF DeeObject **__restrict vector) {
 DREF SharedVector *result;
 result = DeeObject_MALLOC(SharedVector);
 if unlikely(!result) goto done;
 DeeObject_Init(result,&SharedVector_Type);
#ifndef CONFIG_NO_THREADS
 rwlock_init(&result->sv_lock);
#endif
 result->sv_length = length;
 result->sv_vector = vector;
done:
 return result;
}

/* Check if the reference counter of `self' is 1. When it is,
 * simply destroy the shared vector without freeing `sv_vector',
 * but still decref() all contained object.
 * Otherwise, try to allocate a new vector with a length of `sv_length'.
 * If doing so fails, don't raise an error but replace `sv_vector' with
 * `NULL' and `sv_length' with `0' before decref()-ing all elements
 * that that pair of members used to refer to.
 * If allocation does succeed, memcpy() all objects contained in
 * the original vector into the dynamically allocated one, thus
 * transferring ownership to that vector before writing it back
 * to the SharedVector object.
 * >> In the end, this behavior is required to implement a fast,
 *    general-purpose sequence type that can be used to implement
 *    the `ASM_CALL_SEQ' opcode, as generated for brace-initializers. */
INTERN void DCALL
SharedVector_Decref(DREF SharedVector *__restrict self) {
 DREF DeeObject **begin,**iter;
 DREF DeeObject **vector_copy;
 ASSERT_OBJECT_TYPE_EXACT(self,&SharedVector_Type);
 if (!DeeObject_IsShared(self)) {
  /* Simple case: The vector isn't being shared. */
  iter = (begin = self->sv_vector)+self->sv_length;
  while (iter-- != begin) Dee_Decref(*iter);
  Dee_DecrefNokill(&SharedVector_Type);
  DeeObject_FreeTracker((DeeObject *)self);
  DeeObject_Free(self);
  return;
 }
 /* Difficult case: must duplicate the vector. */
#ifndef CONFIG_NO_THREADS
 rwlock_write(&self->sv_lock);
#endif
 vector_copy = (DREF DeeObject **)Dee_TryMalloc(self->sv_length*
                                                sizeof(DREF DeeObject *));
 if unlikely(!vector_copy)
    goto err_cannot_inherit;
 /* Simply copy all the elements, transferring
  * all the references that they represent. */
 MEMCPY_PTR(vector_copy,self->sv_vector,self->sv_length);
 /* Give the SharedVector its very own copy
  * which it will take to its grave. */
 self->sv_vector = vector_copy;
#ifndef CONFIG_NO_THREADS
 rwlock_endwrite(&self->sv_lock);
#endif
 Dee_Decref(self);
 return;

err_cannot_inherit:
 /* Special case: failed to create a copy that the vector may call its own. */
 iter = (begin = self->sv_vector)+self->sv_length;
 /* Override with an empty vector. */
 self->sv_vector = NULL;
 self->sv_length = 0;
#ifndef CONFIG_NO_THREADS
 rwlock_endwrite(&self->sv_lock);
#endif
 /* Destroy the items that the caller wanted the vector to inherit. */
 while (iter-- != begin) Dee_Decref(*iter);
 Dee_Decref(self);
}



DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_SVEC_C */
