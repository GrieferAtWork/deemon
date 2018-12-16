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
#ifndef GUARD_DEEMON_OBJECTS_SEQ_RANGE_C
#define GUARD_DEEMON_OBJECTS_SEQ_RANGE_C 1

#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/object.h>
#include <deemon/seq.h>
#include <deemon/none.h>
#include <deemon/string.h>
#include <deemon/int.h>
#include <deemon/error.h>

#include "range.h"
#include "../gc_inspect.h"
#include "../../runtime/runtime_error.h"


#include <hybrid/overflow.h>
#include <hybrid/__unaligned.h>
#include <deemon/util/rwlock.h>

DECL_BEGIN

PRIVATE int DCALL
ri_ctor(RangeIterator *__restrict self) {
 self->ri_range = (DREF Range *)DeeRange_New(Dee_None,Dee_None,NULL);
 if unlikely(!self->ri_range) return -1;
 self->ri_index = Dee_None;
 self->ri_end   = Dee_None;
 self->ri_step  = NULL;
 self->ri_first = true;
 Dee_Incref_n(Dee_None,2);
 rwlock_init(&self->ri_lock);
 return 0;
}

INTDEF DeeTypeObject SeqRange_Type;

PRIVATE int DCALL
ri_init(RangeIterator *__restrict self,
        size_t argc, DeeObject **__restrict argv) {
 if (DeeArg_Unpack(argc,argv,"o:_SeqRangeIterator",&self->ri_range) ||
     DeeObject_AssertTypeExact(self->ri_range,&SeqRange_Type))
     return -1;
 self->ri_index = self->ri_range->r_begin;
 self->ri_end   = self->ri_range->r_end;
 self->ri_step  = self->ri_range->r_step;
 Dee_Incref(self->ri_index);
 Dee_Incref(self->ri_range);
 Dee_Incref(self->ri_end);
 Dee_XIncref(self->ri_step);
 self->ri_first = true;
 rwlock_init(&self->ri_lock);
 return 0;
}

PRIVATE int DCALL
ri_copy(RangeIterator *__restrict self,
        RangeIterator *__restrict other) {
 DREF DeeObject *new_index,*old_index;
again:
 rwlock_read(&other->ri_lock);
 old_index = other->ri_index;
 self->ri_first = other->ri_first;
 Dee_Incref(old_index);
 rwlock_endread(&other->ri_lock);
 /* Create a copy of the index (may not be correct if it already changed) */
 new_index = DeeObject_Copy(old_index);
 Dee_Decref(old_index);
 if unlikely(!new_index)
    goto err;
 COMPILER_READ_BARRIER();
 if (old_index != other->ri_index) {
  Dee_Decref(new_index);
  /* Try to read the old index again.
   * This can happen if the other iterator
   * was spun while we were copying its index. */
  goto again;
 }
 rwlock_init(&self->ri_lock);
 /* Other members are constant, so we don't
  * need to bother with synchronizing them. */
 self->ri_range = other->ri_range;
 self->ri_end   = other->ri_end;
 self->ri_step  = other->ri_step;
 Dee_Incref(self->ri_range);
 Dee_Incref(self->ri_end);
 Dee_XIncref(self->ri_step);
 return 0;
err:
 return -1;
}

PRIVATE void DCALL
ri_fini(RangeIterator *__restrict self) {
 Dee_Decref(self->ri_index);
 Dee_Decref(self->ri_range);
 Dee_Decref(self->ri_end);
 Dee_XDecref(self->ri_step);
}
PRIVATE void DCALL
ri_visit(RangeIterator *__restrict self,
         dvisit_t proc, void *arg) {
 rwlock_read(&self->ri_lock);
 Dee_Visit(self->ri_index);
 rwlock_endread(&self->ri_lock);
 Dee_Visit(self->ri_range);
 Dee_Visit(self->ri_end);
 Dee_XVisit(self->ri_step);
}

PRIVATE DREF DeeObject *DCALL
ri_next(RangeIterator *__restrict self) {
 DREF DeeObject *result;
 DREF DeeObject *old_index;
 int temp; bool is_first;
 rwlock_read(&self->ri_lock);
 result = self->ri_index;
 is_first = self->ri_first;
 self->ri_first = false;
 Dee_Incref(result);
 rwlock_endread(&self->ri_lock);
 /* Skip the index modification on the first loop. */
 if (!is_first) {
  if (self->ri_step) {
   temp = DeeObject_InplaceAdd(&result,self->ri_step);
  } else if (self->ri_range->r_rev) {
   temp = DeeObject_Dec(&result);
  } else {
   temp = DeeObject_Inc(&result);
  }
  if unlikely(temp)
     goto err_result;
 }
 if unlikely(self->ri_range->r_rev) {
  temp = DeeObject_CompareLo(result,self->ri_end);
 } else {
  temp = DeeObject_CompareGe(result,self->ri_end);
 }
 if (temp != 0) {
  /* Error, or done. */
  Dee_Decref(result);
  if unlikely(temp < 0)
     goto err;
  return ITER_DONE;
 }
 /* Save the new index object. */
 rwlock_write(&self->ri_lock);
 old_index = self->ri_index; /* Inherit reference. */
 self->ri_index = result;
 Dee_Incref(result); /* Reference for `self->ri_index' */
 rwlock_endwrite(&self->ri_lock);
 Dee_Decref(old_index); /* Decref() the old index. */
 return result;
err_result:
 Dee_Decref(result);
err:
 return NULL;
}

PRIVATE int DCALL
ri_bool(RangeIterator *__restrict self) {
 DREF DeeObject *next_value;
 int temp; bool is_first;
 rwlock_read(&self->ri_lock);
 next_value = self->ri_index;
 is_first = self->ri_first;
 self->ri_first = false;
 Dee_Incref(next_value);
 rwlock_endread(&self->ri_lock);
 /* Skip the index modification on the first loop. */
 if (!is_first) {
  if (self->ri_step) {
   temp = DeeObject_InplaceAdd(&next_value,self->ri_step);
  } else if (self->ri_range->r_rev) {
   temp = DeeObject_Dec(&next_value);
  } else {
   temp = DeeObject_Inc(&next_value);
  }
  if unlikely(temp)
     goto err_result;
 }
 if unlikely(self->ri_range->r_rev) {
  temp = DeeObject_CompareLo(next_value,self->ri_end);
 } else {
  temp = DeeObject_CompareGe(next_value,self->ri_end);
 }
 Dee_Decref(next_value);
 if unlikely(temp < 0)
    goto err;
 return !temp;
err_result:
 Dee_Decref(next_value);
err:
 return -1;
}

PRIVATE DREF DeeObject *DCALL
ri_index_get(RangeIterator *__restrict self) {
 DREF DeeObject *result;
 rwlock_read(&self->ri_lock);
 result = self->ri_index;
 Dee_Incref(result);
 rwlock_endread(&self->ri_lock);
 return result;
}

PRIVATE int DCALL
ri_index_del(RangeIterator *__restrict self) {
 DREF DeeObject *old_index;
 rwlock_write(&self->ri_lock);
 old_index = self->ri_index;
 /* Assign the original begin-index. */
 self->ri_index = self->ri_range->r_begin;
 self->ri_first = true;
 Dee_Incref(self->ri_index);
 rwlock_endwrite(&self->ri_lock);
 Dee_Decref(old_index);
 return 0;
}

PRIVATE int DCALL
ri_index_set(RangeIterator *__restrict self,
             DeeObject *__restrict value) {
 DREF DeeObject *old_index;
 if (DeeGC_ReferredBy(value,(DeeObject *)self))
     return err_reference_loop((DeeObject *)self,value);
 rwlock_write(&self->ri_lock);
 old_index = self->ri_index;
 /* Assign the given value. */
 self->ri_index = value;
 self->ri_first = true;
 Dee_Incref(value);
 rwlock_endwrite(&self->ri_lock);
 Dee_Decref(old_index);
 return 0;
}

PRIVATE struct type_getset ri_getsets[] = {
    { "__index__", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&ri_index_get,
                   (int(DCALL *)(DeeObject *__restrict))&ri_index_del,
                   (int(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&ri_index_set },
    { NULL }
};

PRIVATE struct type_member ri_members[] = {
    TYPE_MEMBER_FIELD_DOC("seq",STRUCT_OBJECT,offsetof(RangeIterator,ri_range),"->?Ert:SeqRange"),
    TYPE_MEMBER_FIELD("__end__",STRUCT_OBJECT,offsetof(RangeIterator,ri_end)),
    TYPE_MEMBER_FIELD("__step__",STRUCT_OBJECT,offsetof(RangeIterator,ri_step)),
    TYPE_MEMBER_FIELD("__first__",STRUCT_ATOMIC|STRUCT_BOOL,offsetof(RangeIterator,ri_first)),
    TYPE_MEMBER_END
};

#define DEFINE_RANGEITERATOR_COMPARE(name,compare_object) \
PRIVATE DREF DeeObject *DCALL \
name(RangeIterator *__restrict self, \
     RangeIterator *__restrict other) { \
 DREF DeeObject *my_index,*ot_index,*result; \
 if (DeeObject_AssertTypeExact((DeeObject *)other,&SeqRangeIterator_Type)) \
     return NULL; \
 rwlock_read(&self->ri_lock); \
 my_index = self->ri_index; \
 Dee_Incref(my_index); \
 rwlock_endread(&self->ri_lock); \
 rwlock_read(&other->ri_lock); \
 ot_index = other->ri_index; \
 Dee_Incref(ot_index); \
 rwlock_endread(&other->ri_lock); \
 result = compare_object(my_index,ot_index); \
 Dee_Decref(ot_index); \
 Dee_Decref(my_index); \
 return result; \
}
DEFINE_RANGEITERATOR_COMPARE(ri_eq,DeeObject_CompareEqObject)
DEFINE_RANGEITERATOR_COMPARE(ri_ne,DeeObject_CompareNeObject)
DEFINE_RANGEITERATOR_COMPARE(ri_lo,DeeObject_CompareLoObject)
DEFINE_RANGEITERATOR_COMPARE(ri_le,DeeObject_CompareLeObject)
DEFINE_RANGEITERATOR_COMPARE(ri_gr,DeeObject_CompareGrObject)
DEFINE_RANGEITERATOR_COMPARE(ri_ge,DeeObject_CompareGeObject)
#undef DEFINE_RANGEITERATOR_COMPARE

PRIVATE struct type_cmp ri_cmp = {
    /* .tp_hash = */NULL,
    /* .tp_eq   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&ri_eq,
    /* .tp_ne   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&ri_ne,
    /* .tp_lo   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&ri_lo,
    /* .tp_le   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&ri_le,
    /* .tp_gr   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&ri_gr,
    /* .tp_ge   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&ri_ge,
};


INTERN DeeTypeObject SeqRangeIterator_Type = {
    OBJECT_HEAD_INIT(&DeeType_Type),
    /* .tp_name     = */"_SeqRangeIterator",
    /* .tp_doc      = */NULL,
    /* .tp_flags    = */TP_FNORMAL|TP_FFINAL,
    /* .tp_weakrefs = */0,
    /* .tp_features = */TF_NONE,
    /* .tp_base     = */&DeeIterator_Type,
    /* .tp_init = */{
        {
            /* .tp_alloc = */{
                /* .tp_ctor      = */&ri_ctor,
                /* .tp_copy_ctor = */&ri_copy,
                /* .tp_deep_ctor = */NULL,
                /* .tp_any_ctor  = */&ri_init,
                TYPE_FIXED_ALLOCATOR(RangeIterator)
            }
        },
        /* .tp_dtor        = */(void(DCALL *)(DeeObject *__restrict))&ri_fini,
        /* .tp_assign      = */NULL,
        /* .tp_move_assign = */NULL
    },
    /* .tp_cast = */{
        /* .tp_str  = */NULL,
        /* .tp_repr = */NULL,
        /* .tp_bool = */(int(DCALL *)(DeeObject *__restrict))&ri_bool
    },
    /* .tp_call          = */NULL,
    /* .tp_visit         = */(void(DCALL *)(DeeObject *__restrict,dvisit_t,void*))&ri_visit,
    /* .tp_gc            = */NULL,
    /* .tp_math          = */NULL,
    /* .tp_cmp           = */&ri_cmp,
    /* .tp_seq           = */NULL,
    /* .tp_iter_next     = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&ri_next,
    /* .tp_attr          = */NULL,
    /* .tp_with          = */NULL,
    /* .tp_buffer        = */NULL,
    /* .tp_methods       = */NULL,
    /* .tp_getsets       = */ri_getsets,
    /* .tp_members       = */ri_members,
    /* .tp_class_methods = */NULL,
    /* .tp_class_getsets = */NULL,
    /* .tp_class_members = */NULL
};


PRIVATE void DCALL
range_fini(Range *__restrict self) {
 Dee_Decref(self->r_begin);
 Dee_Decref(self->r_end);
 Dee_XDecref(self->r_step);
}
PRIVATE void DCALL
range_visit(Range *__restrict self, dvisit_t proc, void *arg) {
 Dee_Visit(self->r_begin);
 Dee_Visit(self->r_end);
 Dee_XVisit(self->r_step);
}

PRIVATE DREF RangeIterator *DCALL
range_iter(Range *__restrict self) {
 DREF RangeIterator *result;
 result = DeeObject_MALLOC(RangeIterator);
 if unlikely(!result) goto done;
 DeeObject_Init(result,&SeqRangeIterator_Type);
 result->ri_index = self->r_begin;
 result->ri_range = self;
 result->ri_end   = self->r_end;
 result->ri_step  = self->r_step;
 Dee_Incref(result->ri_index);
 Dee_Incref(result->ri_range);
 Dee_Incref(result->ri_end);
 Dee_XIncref(result->ri_step);
 result->ri_first = true;
 rwlock_init(&result->ri_lock);
done:
 return result;
}

PRIVATE DREF DeeObject *DCALL
range_size(Range *__restrict self) {
 DREF DeeObject *result,*temp; int cmp;
 result = DeeObject_Sub(self->r_end,self->r_begin);
 if (self->r_step && likely(result)) {
  cmp = DeeObject_CompareGe(self->r_step,&DeeInt_Zero);
  if unlikely(cmp < 0) goto err_r;
  if (cmp) {
   /* Do a ceil-division. */
   temp = DeeObject_Add(result,self->r_step);
   Dee_Decref(result);
   if unlikely(!temp) goto err;
   if unlikely(DeeObject_Dec(&temp)) goto err_temp;
   result = DeeObject_Div(temp,self->r_step);
   Dee_Decref(temp);
  } else {
   DREF DeeObject *neg_step;
   neg_step = DeeObject_Neg(self->r_step);
   if unlikely(!neg_step) goto err_r;
   temp = DeeObject_Div(result,neg_step);
   Dee_Decref(result);
   result = temp;
  }
  if unlikely(!result) goto err;
 }
 /* Make sure not to return a negative value. */
 cmp = DeeObject_CompareLo(result,&DeeInt_Zero);
 if (cmp != 0) {
  if unlikely(cmp < 0) goto err_r;
  temp = DeeObject_NewDefault(Dee_TYPE(result));
  Dee_Decref(result);
  result = temp;
 }
 return result;
err_temp:
 Dee_Decref(temp);
 goto err;
err_r:
 Dee_Decref(result);
err:
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
range_contains(Range *__restrict self,
               DeeObject *__restrict index) {
 int temp; DREF DeeObject *relative_value,*tempres;
 temp = DeeObject_CompareLo(index,self->r_begin);
 if (temp != 0) goto err_or_false; /* if (INDEX < BEGIN) return false; */
 temp = DeeObject_CompareGe(index,self->r_end);
 if (temp != 0) goto err_or_false; /* if (INDEX >= END) return false; */
 if (self->r_step) {
  temp = DeeObject_CompareGe(self->r_step,&DeeInt_Zero);
  if unlikely(temp < 0) goto err;
  if (temp) {
   /* ((index - self->ir_begin) % self->ir_step) == 0 */
   relative_value = DeeObject_Sub(index,self->r_begin);
   if unlikely(!relative_value) goto err;
   tempres = DeeObject_Mod(relative_value,self->r_step);
  } else {
   /* ((self->ir_end - index) % -self->ir_step) == 0 */
   DREF DeeObject *neg_step;
   relative_value = DeeObject_Sub(self->r_end,index);
   neg_step = DeeObject_Neg(self->r_step);
   if unlikely(!neg_step) { Dee_Decref(relative_value); goto err; }
   tempres = DeeObject_Mod(relative_value,neg_step);
   Dee_Decref(neg_step);
  }
  Dee_Decref(relative_value);
  if unlikely(!tempres) goto err;
  temp = DeeObject_CompareEq(tempres,&DeeInt_Zero);
  Dee_Decref(tempres);
  if (temp <= 0) goto err_or_false;
 }
 return_true;
err_or_false:
 if unlikely(temp < 0) goto err;
 return_false;
err:
 return NULL;
}

#if 0 /* TODO: Fix this */
PRIVATE DREF DeeObject *DCALL
range_get(Range *__restrict self,
          DeeObject *__restrict index) {
 DREF DeeObject *result,*temp; int cmp;
 if (self->r_step) {
  cmp = DeeObject_CompareLo(self->r_step,&DeeInt_Zero);
  if unlikely(cmp < 0) goto err;
  temp   = DeeObject_Mul(self->r_step,index);
  if unlikely(!temp) goto err;
  result = DeeObject_Add(self->r_begin,temp);
  Dee_Decref(temp);
  if (cmp) {
   cmp = DeeObject_CompareLe(result,self->r_end);
  } else {
   cmp = DeeObject_CompareGe(result,self->r_end);
  }
 } else if (self->r_rev) {
  result = DeeObject_Sub(self->r_begin,index);
  if unlikely(!result) goto err;
  cmp = DeeObject_CompareLe(result,self->r_end);
 } else {
  result = DeeObject_Add(self->r_begin,index);
  if unlikely(!result) goto err;
  cmp = DeeObject_CompareGe(result,self->r_end);
 }
 if unlikely(cmp != 0) {
  Dee_Decref(result);
  if (cmp > 0)
      err_index_out_of_bounds_ob((DeeObject *)self,index);
  goto err;
 }
 return result;
err:
 return NULL;
}
#endif


PRIVATE struct type_seq range_seq = {
    /* .tp_iter_self = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&range_iter,
    /* .tp_size      = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&range_size,
    /* .tp_contains  = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&range_contains,
    /* .tp_get       = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))NULL, // &range_get,
    /* .tp_del       = */NULL,
    /* .tp_set       = */NULL,
    /* .tp_range_get = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict,DeeObject *__restrict))NULL, /* TODO (return another `range' as sub-range type) */
    /* .tp_range_del = */NULL,
    /* .tp_range_set = */NULL,
    /* .tp_nsi       = */NULL /* TODO */
};


PRIVATE struct type_member range_members[] = {
    TYPE_MEMBER_FIELD("start",STRUCT_OBJECT,offsetof(Range,r_begin)),
    TYPE_MEMBER_FIELD("end",STRUCT_OBJECT,offsetof(Range,r_end)),
    TYPE_MEMBER_FIELD("step",STRUCT_OBJECT,offsetof(Range,r_step)),
    TYPE_MEMBER_FIELD("__rev__",STRUCT_CONST|STRUCT_BOOL,offsetof(Range,r_rev)),
    TYPE_MEMBER_END
};

PRIVATE struct type_member range_class_members[] = {
    TYPE_MEMBER_CONST("iterator",&SeqRangeIterator_Type),
    TYPE_MEMBER_END
};

PRIVATE DREF DeeObject *DCALL
range_repr(Range *__restrict self) {
 return self->r_step
       ? DeeString_Newf("[%r:%r,%r]",self->r_begin,self->r_end,self->r_step)
       : DeeString_Newf("[%r:%r]",self->r_begin,self->r_end)
       ;
}


INTERN DeeTypeObject SeqRange_Type = {
    OBJECT_HEAD_INIT(&DeeType_Type),
    /* .tp_name     = */"_SeqRange",
    /* .tp_doc      = */NULL,
    /* .tp_flags    = */TP_FNORMAL|TP_FFINAL,
    /* .tp_weakrefs = */0,
    /* .tp_features = */TF_NONE,
    /* .tp_base     = */&DeeSeq_Type,
    /* .tp_init = */{
        {
            /* .tp_alloc = */{
                /* .tp_ctor      = */NULL, /* TODO */
                /* .tp_copy_ctor = */NULL, /* TODO */
                /* .tp_deep_ctor = */NULL, /* TODO */
                /* .tp_any_ctor  = */NULL, /* TODO */
                TYPE_FIXED_ALLOCATOR(Range)
            }
        },
        /* .tp_dtor        = */(void(DCALL *)(DeeObject *__restrict))&range_fini,
        /* .tp_assign      = */NULL,
        /* .tp_move_assign = */NULL
    },
    /* .tp_cast = */{
        /* .tp_str  = */NULL,
        /* .tp_repr = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&range_repr,
        /* .tp_bool = */NULL /* TODO */
    },
    /* .tp_call          = */NULL,
    /* .tp_visit         = */(void(DCALL *)(DeeObject *__restrict,dvisit_t,void*))&range_visit,
    /* .tp_gc            = */NULL,
    /* .tp_math          = */NULL,
    /* .tp_cmp           = */NULL,
    /* .tp_seq           = */&range_seq,
    /* .tp_iter_next     = */NULL,
    /* .tp_attr          = */NULL,
    /* .tp_with          = */NULL,
    /* .tp_buffer        = */NULL,
    /* .tp_methods       = */NULL,
    /* .tp_getsets       = */NULL,
    /* .tp_members       = */range_members,
    /* .tp_class_methods = */NULL,
    /* .tp_class_getsets = */NULL,
    /* .tp_class_members = */range_class_members
};








PRIVATE int DCALL
iri_copy(IntRangeIterator *__restrict self,
         IntRangeIterator *__restrict other) {
#ifdef CONFIG_NO_THREADS
 self->iri_index = other->iri_index;
#else
 self->iri_index = ATOMIC_READ(other->iri_index);
#endif
 self->iri_end   = other->iri_end;
 self->iri_step  = other->iri_step;
 self->iri_range = other->iri_range;
 Dee_Incref(self->iri_range);
 return 0;
}
PRIVATE int DCALL
iri_bool(IntRangeIterator *__restrict self) {
#ifdef CONFIG_NO_THREADS
 dssize_t index = self->iri_index;
#else
 dssize_t index = ATOMIC_READ(self->iri_index);
#endif
 dssize_t new_index = index + self->iri_step;
 if likely(self->iri_step >= 0) {
  return !(new_index > self->iri_end || new_index < index);
 } else {
  return !(new_index < self->iri_end || new_index > index);
 }
}
PRIVATE void DCALL
iri_fini(IntRangeIterator *__restrict self) {
 Dee_Decref(self->iri_range);
}
PRIVATE void DCALL
iri_visit(IntRangeIterator *__restrict self,
          dvisit_t proc, void *arg) {
 Dee_Visit(self->iri_range);
}

PRIVATE DREF DeeObject *DCALL
iri_next(IntRangeIterator *__restrict self) {
 dssize_t result_index,new_index;
 do {
#ifdef CONFIG_NO_THREADS
  result_index = self->iri_index;
#else
  result_index = ATOMIC_READ(self->iri_index);
#endif
  new_index = result_index + self->iri_step;
  /* Test for overflow/iteration done. */
  if likely(self->iri_step >= 0) {
   if (result_index >= self->iri_end || new_index <= result_index)
       return ITER_DONE;
  } else {
   if (result_index < self->iri_end || new_index >= result_index)
       return ITER_DONE;
  }
#ifdef CONFIG_NO_THREADS
  self->iri_index = new_index;
#endif
 }
#ifdef CONFIG_NO_THREADS
 __WHILE0;
#else
 while (!ATOMIC_CMPXCH(self->iri_index,result_index,new_index));
#endif
 /* Return a new integer for the resulting index. */
 return DeeInt_NewSSize(result_index);
}

PRIVATE struct type_member iri_members[] = {
    TYPE_MEMBER_FIELD_DOC("seq",STRUCT_OBJECT,offsetof(IntRangeIterator,iri_range),"->?Ert:SeqIntRange"),
    /* We allow write-access to these members because doing so doesn't
     * actually harm anything, although fiddling with this stuff may
     * break some weak expectations but should never crash anything! */
    TYPE_MEMBER_FIELD("__index__",STRUCT_ATOMIC|STRUCT_SSIZE_T,offsetof(IntRangeIterator,iri_index)),
    TYPE_MEMBER_FIELD("__end__",STRUCT_ATOMIC|STRUCT_SSIZE_T,offsetof(IntRangeIterator,iri_end)),
    TYPE_MEMBER_FIELD("__step__",STRUCT_ATOMIC|STRUCT_SSIZE_T,offsetof(IntRangeIterator,iri_step)),
    TYPE_MEMBER_END
};

INTERN DeeTypeObject SeqIntRangeIterator_Type = {
    OBJECT_HEAD_INIT(&DeeType_Type),
    /* .tp_name     = */"_SeqIntRangeIterator",
    /* .tp_doc      = */NULL,
    /* .tp_flags    = */TP_FNORMAL|TP_FFINAL,
    /* .tp_weakrefs = */0,
    /* .tp_features = */TF_NONE,
    /* .tp_base     = */&DeeIterator_Type,
    /* .tp_init = */{
        {
            /* .tp_alloc = */{
                /* .tp_ctor      = */NULL, /* TODO */
                /* .tp_copy_ctor = */&iri_copy,
                /* .tp_deep_ctor = */NULL, /* TODO */
                /* .tp_any_ctor  = */NULL, /* TODO */
                TYPE_FIXED_ALLOCATOR(IntRangeIterator)
            }
        },
        /* .tp_dtor        = */(void(DCALL *)(DeeObject *__restrict))&iri_fini,
        /* .tp_assign      = */NULL,
        /* .tp_move_assign = */NULL
    },
    /* .tp_cast = */{
        /* .tp_str  = */NULL,
        /* .tp_repr = */NULL,
        /* .tp_bool = */(int(DCALL *)(DeeObject *__restrict))&iri_bool
    },
    /* .tp_call          = */NULL,
    /* .tp_visit         = */(void(DCALL *)(DeeObject *__restrict,dvisit_t,void*))&iri_visit,
    /* .tp_gc            = */NULL,
    /* .tp_math          = */NULL,
    /* .tp_cmp           = */NULL, /* TODO */
    /* .tp_seq           = */NULL,
    /* .tp_iter_next     = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&iri_next,
    /* .tp_attr          = */NULL,
    /* .tp_with          = */NULL,
    /* .tp_buffer        = */NULL,
    /* .tp_methods       = */NULL,
    /* .tp_getsets       = */NULL,
    /* .tp_members       = */iri_members,
    /* .tp_class_methods = */NULL,
    /* .tp_class_getsets = */NULL,
    /* .tp_class_members = */NULL
};

PRIVATE DREF IntRangeIterator *DCALL
intrange_iter(IntRange *__restrict self) {
 DREF IntRangeIterator *result;
 result = DeeObject_MALLOC(IntRangeIterator);
 if unlikely(!result) goto done;
 DeeObject_Init(result,&SeqIntRangeIterator_Type);
 result->iri_range = self;
 result->iri_step  = self->ir_step;
 if (self->ir_step >= 0) {
  result->iri_end   = self->ir_end;
  result->iri_index = self->ir_begin;
 } else {
  result->iri_end   = self->ir_begin;
  result->iri_index = self->ir_end + self->ir_step;
 }
 Dee_Incref(self);
done:
 return result;
}

PRIVATE DREF DeeObject *DCALL
intrange_contains(IntRange *__restrict self,
                  DeeObject *__restrict other) {
 dssize_t index;
 if (DeeObject_AsSSize(other,&index))
     return NULL;
 if (self->ir_step >= 0) {
  return_bool(index >= self->ir_begin &&
              index <  self->ir_end &&
            ((index - self->ir_begin) % self->ir_step) == 0);
 } else {
  return_bool(index >= self->ir_begin &&
              index <  self->ir_end &&
            ((self->ir_end - index) % -self->ir_step) == 0);
 }
}

PRIVATE DREF DeeObject *DCALL
intrange_size(IntRange *__restrict self) {
 dssize_t result;
 result = self->ir_end - self->ir_begin;
 if unlikely(!self->ir_step) {
  err_divide_by_zero_i(result);
  return NULL;
 }
 if (self->ir_step > 0) {
  /* Do a ceil-division. */
  result += self->ir_step;
  --result;
  result /= self->ir_step;
 } else {
  /* Do a trunc-division. */
  result /= -self->ir_step;
 }
 if (result < 0) result = 0;
 return DeeInt_NewSize((size_t)result);
}

PRIVATE size_t DCALL
intrange_nsi_getsize(IntRange *__restrict self) {
 dssize_t result;
 result = self->ir_end - self->ir_begin;
 if unlikely(!self->ir_step) {
  err_divide_by_zero_i(result);
  return (size_t)-1;
 }
 if (self->ir_step > 0) {
  /* Do a ceil-division. */
  result += self->ir_step;
  --result;
  result /= self->ir_step;
 } else {
  /* Do a trunc-division. */
  result /= -self->ir_step;
 }
 if (result < 0) result = 0;
 return (size_t)result;
}
PRIVATE size_t DCALL
intrange_nsi_getsize_fast(IntRange *__restrict self) {
 dssize_t result;
 result = self->ir_end - self->ir_begin;
 if unlikely(!self->ir_step)
    return (size_t)-1;
 if (self->ir_step > 0) {
  /* Do a ceil-division. */
  result += self->ir_step;
  --result;
  result /= self->ir_step;
 } else {
  /* Do a trunc-division. */
  result /= -self->ir_step;
 }
 if (result < 0) result = 0;
 return (size_t)result;
}

PRIVATE DREF DeeObject *DCALL
intrange_nsi_getitem(IntRange *__restrict self, size_t index) {
 dssize_t result;
 if (self->ir_step >= 0) {
  if unlikely(!self->ir_step)
     return DeeInt_NewSSize(self->ir_begin);
  /* Check for overflows in this arithmetic */
  if (OVERFLOW_SMUL(self->ir_step,index,&result) ||
      OVERFLOW_SADD(self->ir_begin,result,&result))
      goto oob;
  if (result >= self->ir_end) goto oob;
 } else {
  /* Check for overflows in this arithmetic */
  if (OVERFLOW_UADD(index,1,&index) ||
      OVERFLOW_SMUL(self->ir_step,index,&result) ||
      OVERFLOW_SADD(self->ir_end,result,&result))
      goto oob;
  if (result < self->ir_begin) goto oob;
 }
 return DeeInt_NewSSize(result);
oob:
 err_index_out_of_bounds((DeeObject *)self,index,intrange_nsi_getsize(self));
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
intrange_getitem(IntRange *__restrict self,
                 DeeObject *__restrict index_ob) {
 size_t index;
 if (DeeObject_AsSize(index_ob,&index))
     return NULL;
 return intrange_nsi_getitem(self,index);
}

#if 0 /* TODO: Fix these */
PRIVATE DREF DeeObject *DCALL
intrange_nsi_getrange(IntRange *__restrict self,
                      dssize_t start, dssize_t end) {
 size_t mylen = intrange_nsi_getsize(self);
 if unlikely(mylen == (size_t)-1) return NULL;
 if (start < 0) start += mylen;
 if (end < 0) end += mylen;
 if ((size_t)end > mylen) end = (dssize_t)mylen;
 if ((size_t)start >= (size_t)end)
      return_reference_(Dee_EmptySeq);
 return DeeRange_NewInt(self->ir_begin + (size_t)start * self->ir_step,
                        self->ir_begin + ((size_t)end - (size_t)start) * self->ir_step,
                        self->ir_step);
}
PRIVATE DREF DeeObject *DCALL
intrange_nsi_getrange_n(IntRange *__restrict self,
                        dssize_t start) {
 size_t mylen = intrange_nsi_getsize(self);
 if unlikely(mylen == (size_t)-1) return NULL;
 if (start < 0) start += mylen;
 if ((size_t)start >= mylen)
      return_reference_(Dee_EmptySeq);
 return DeeRange_NewInt(self->ir_begin + start * self->ir_step,
                        self->ir_begin + (mylen - (size_t)start) * self->ir_step,
                        self->ir_step);
}
PRIVATE DREF DeeObject *DCALL
intrange_getrange(IntRange *__restrict self,
                  DeeObject *__restrict start_ob,
                  DeeObject *__restrict end_ob) {
 dssize_t start_index,end_index;
 if (DeeObject_AsSSize(start_ob,&start_index))
     return NULL;
 if (DeeNone_Check(end_ob))
     return intrange_nsi_getrange_n(self,start_index);
 if (DeeObject_AsSSize(end_ob,&end_index))
     return NULL;
 return intrange_nsi_getrange(self,start_index,end_index);
}
#endif


PRIVATE struct type_nsi intrange_nsi = {
    /* .nsi_class   = */TYPE_SEQX_CLASS_SEQ,
    /* .nsi_flags   = */TYPE_SEQX_FNORMAL,
    {
        /* .nsi_seqlike = */{
            /* .nsi_getsize      = */(void *)&intrange_nsi_getsize,
            /* .nsi_getsize_fast = */(void *)&intrange_nsi_getsize_fast,
            /* .nsi_getitem      = */(void *)&intrange_nsi_getitem,
            /* .nsi_delitem      = */(void *)NULL,
            /* .nsi_setitem      = */(void *)NULL,
            /* .nsi_getitem_fast = */(void *)NULL,
            /* .nsi_getrange     = */(void *)NULL, // &intrange_nsi_getrange,
            /* .nsi_getrange_n   = */(void *)NULL, // &intrange_nsi_getrange_n,
            /* .nsi_setrange     = */(void *)NULL,
            /* .nsi_setrange_n   = */(void *)NULL,
            /* .nsi_find         = */(void *)NULL,
            /* .nsi_rfind        = */(void *)NULL,
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

PRIVATE struct type_seq intrange_seq = {
    /* .tp_iter_self = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&intrange_iter,
    /* .tp_size      = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&intrange_size,
    /* .tp_contains  = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&intrange_contains,
    /* .tp_get       = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&intrange_getitem,
    /* .tp_del       = */NULL,
    /* .tp_set       = */NULL,
    /* .tp_range_get = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict,DeeObject *__restrict))NULL, // &intrange_getrange,
    /* .tp_range_del = */NULL,
    /* .tp_range_set = */NULL,
    /* .tp_nsi       = */&intrange_nsi
};


PRIVATE struct type_member intrange_members[] = {
    TYPE_MEMBER_FIELD("start",STRUCT_ATOMIC|STRUCT_SSIZE_T,offsetof(IntRange,ir_begin)),
    TYPE_MEMBER_FIELD("end",STRUCT_ATOMIC|STRUCT_SSIZE_T,offsetof(IntRange,ir_end)),
    TYPE_MEMBER_FIELD("step",STRUCT_ATOMIC|STRUCT_SSIZE_T,offsetof(IntRange,ir_step)),
    TYPE_MEMBER_END
};

PRIVATE struct type_member intrange_class_members[] = {
    TYPE_MEMBER_CONST("iterator",&SeqIntRangeIterator_Type),
    TYPE_MEMBER_END
};

PRIVATE DREF DeeObject *DCALL
intrange_repr(IntRange *__restrict self) {
 return self->ir_step != 1
       ? DeeString_Newf("[%Id:%Id,%Id]",self->ir_begin,self->ir_end,self->ir_step)
       : DeeString_Newf("[%Id:%Id]",self->ir_begin,self->ir_end)
       ;
}

INTERN DeeTypeObject SeqIntRange_Type = {
    OBJECT_HEAD_INIT(&DeeType_Type),
    /* .tp_name     = */"_SeqIntRange",
    /* .tp_doc      = */NULL,
    /* .tp_flags    = */TP_FNORMAL|TP_FFINAL,
    /* .tp_weakrefs = */0,
    /* .tp_features = */TF_NONE,
    /* .tp_base     = */&DeeSeq_Type,
    /* .tp_init = */{
        {
            /* .tp_alloc = */{
                /* .tp_ctor      = */NULL, /* TODO */
                /* .tp_copy_ctor = */NULL, /* TODO */
                /* .tp_deep_ctor = */NULL, /* TODO */
                /* .tp_any_ctor  = */NULL, /* TODO */
                TYPE_FIXED_ALLOCATOR(IntRange)
            }
        },
        /* .tp_dtor        = */NULL,
        /* .tp_assign      = */NULL,
        /* .tp_move_assign = */NULL
    },
    /* .tp_cast = */{
        /* .tp_str  = */NULL,
        /* .tp_repr = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&intrange_repr,
        /* .tp_bool = */NULL /* TODO */
    },
    /* .tp_call          = */NULL,
    /* .tp_visit         = */NULL,
    /* .tp_gc            = */NULL,
    /* .tp_math          = */NULL,
    /* .tp_cmp           = */NULL,
    /* .tp_seq           = */&intrange_seq,
    /* .tp_iter_next     = */NULL,
    /* .tp_attr          = */NULL,
    /* .tp_with          = */NULL,
    /* .tp_buffer        = */NULL,
    /* .tp_methods       = */NULL,
    /* .tp_getsets       = */NULL,
    /* .tp_members       = */intrange_members,
    /* .tp_class_methods = */NULL,
    /* .tp_class_getsets = */NULL,
    /* .tp_class_members = */intrange_class_members
};














/* Test to force use of object-based ranges. */
#undef ALWAYS_USE_OBJECT_RANGES
/* #define ALWAYS_USE_OBJECT_RANGES 1 */

PUBLIC DREF DeeObject *DCALL
DeeRange_NewInt(dssize_t begin,
                dssize_t end,
                dssize_t step) {
#ifdef ALWAYS_USE_OBJECT_RANGES
 DREF DeeObject *begin_ob,*end_ob,*step_ob,*result;
 if ((begin_ob = DeeInt_NewSSize(begin)) == NULL) return NULL;
 if ((end_ob = DeeInt_NewSSize(end)) == NULL) { Dee_Decref(begin_ob); return NULL; }
 if ((step_ob = DeeInt_NewSSize(step)) == NULL) { Dee_Decref(end_ob); Dee_Decref(begin_ob); return NULL; }
 result = DeeRange_New(begin_ob,end_ob,step_ob);
 Dee_Decref(step_ob);
 Dee_Decref(end_ob);
 Dee_Decref(begin_ob);
 return result;
#else
 DREF IntRange *result;
 ASSERT(step != 0);

 /* Create the new range. */
 result = DeeObject_MALLOC(IntRange);
 if unlikely(!result) goto done;
 DeeObject_Init(result,&SeqIntRange_Type);
 /* Fill in members of the new range object. */
 result->ir_begin = begin;
 result->ir_end   = end;
 result->ir_step  = step;
done:
 return (DREF DeeObject *)result;
#endif
}

PUBLIC DREF DeeObject *DCALL
DeeRange_New(DeeObject *__restrict begin,
             DeeObject *__restrict end,
             DeeObject *step) {
 DREF Range *result; int temp;
 ASSERT_OBJECT(begin);
 ASSERT_OBJECT(end);
 ASSERT_OBJECT_OPT(step);
#ifndef ALWAYS_USE_OBJECT_RANGES
 /* Check for special optimizations for the likely case of int-only arguments. */
 {
  dssize_t i_begin,i_end,i_step;
  if ((DeeInt_Check(begin) && DeeInt_TryAsSSize(begin,&i_begin)) &&
      (DeeInt_Check(end) && DeeInt_TryAsSSize(end,&i_end)) &&
       i_begin <= i_end) {
   i_step = 1;
   if (step && (!DeeInt_Check(step) ||
                !DeeInt_TryAsSSize(step,&i_step) ||
                 i_step == 0))
       goto do_object_range;
   /* Create an integer-based range. */
   return DeeRange_NewInt(i_begin,i_end,i_step);
  }
 }
do_object_range:
#endif
 temp = 0;
 /* Check if `step' is negative (required for proper compare operations of the range iterator). */
 if (step) {
  temp = DeeObject_CompareLo(step,&DeeInt_Zero);
  if unlikely(temp < 0) return NULL;
 }
 /* Create the new range. */
 result = DeeObject_MALLOC(Range);
 if unlikely(!result) goto done;
 DeeObject_Init(result,&SeqRange_Type);
 /* Fill in members of the new range object. */
 result->r_begin = begin;
 result->r_end   = end;
 result->r_step  = step;
 result->r_rev   = !!temp;
 Dee_Incref(begin);
 Dee_Incref(end);
 Dee_XIncref(step);
done:
 return (DREF DeeObject *)result;
}


DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_RANGE_C */
