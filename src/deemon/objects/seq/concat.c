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
#ifndef GUARD_DEEMON_OBJECTS_SEQ_CONCAT_C
#define GUARD_DEEMON_OBJECTS_SEQ_CONCAT_C 1

#include <deemon/api.h>
#include <deemon/object.h>
#include <deemon/seq.h>
#include <deemon/error.h>
#include <deemon/bool.h>
#include <deemon/arg.h>
#include <deemon/tuple.h>
#include <deemon/int.h>

#ifndef CONFIG_NO_THREADS
#include <deemon/util/rwlock.h>
#endif /* !CONFIG_NO_THREADS */

#include "svec.h"
#include "../../runtime/runtime_error.h"

DECL_BEGIN

typedef DeeTupleObject Tuple;
typedef DeeTupleObject Cat;

INTDEF DeeTypeObject DeeCat_Type;
INTDEF DeeTypeObject DeeCatIterator_Type;

typedef struct {
    OBJECT_HEAD
    DREF DeeObject      *c_curr; /* [1..1][lock(c_lock)] The current iterator. */
    DeeObject    *const *c_pseq; /* [1..1][1..1][lock(c_lock)][in(c_cat)] The current sequence. */
    DREF Cat            *c_cat;  /* [1..1][const] The underly sequence cat. */
#ifndef CONFIG_NO_THREADS
    rwlock_t             c_lock; /* Lock for this iterator. */
#endif /* !CONFIG_NO_THREADS */
} CatIterator;


PRIVATE int DCALL
catiterator_ctor(CatIterator *__restrict self) {
 self->c_cat = (DREF Cat *)DeeSeq_Concat(Dee_EmptySeq,Dee_EmptySeq);
 if unlikely(!self->c_cat) return -1;
 self->c_curr = DeeObject_IterSelf(Dee_EmptySeq);
 if unlikely(!self->c_curr) {
  Dee_Decref(self->c_cat);
  return -1;
 }
 self->c_pseq = DeeTuple_ELEM(self->c_cat);
 rwlock_init(&self->c_lock);
 return 0;
}
PRIVATE int DCALL
catiterator_copy(CatIterator *__restrict self,
                 CatIterator *__restrict other) {
 DREF DeeObject *iterator;
 rwlock_read(&other->c_lock);
 iterator = other->c_curr;
 self->c_pseq = other->c_pseq;
 Dee_Incref(iterator);
 rwlock_endread(&other->c_lock);
 self->c_curr = DeeObject_Copy(iterator);
 Dee_Decref(iterator);
 if unlikely(!self->c_curr) return -1;
 self->c_cat = other->c_cat;
 Dee_Incref(self->c_cat);
 return 0;
}
PRIVATE int DCALL
catiterator_init(CatIterator *__restrict self,
                 size_t argc, DeeObject **__restrict argv) {
 if (DeeArg_Unpack(argc,argv,"o:_catseqiterator",&self->c_cat) ||
     DeeObject_AssertTypeExact((DeeObject *)self->c_cat,&DeeCat_Type))
     return -1;
 self->c_pseq = DeeTuple_ELEM(self->c_cat);
 self->c_curr = DeeObject_IterSelf(self->c_pseq[0]);
 if unlikely(!self->c_curr) return -1;
 Dee_Incref(self->c_cat);
 rwlock_init(&self->c_lock);
 return 0;
}
PRIVATE void DCALL
catiterator_fini(CatIterator *__restrict self) {
 Dee_Decref(self->c_curr);
 Dee_Decref(self->c_cat);
}
PRIVATE void DCALL
catiterator_visit(CatIterator *__restrict self, dvisit_t proc, void *arg) {
 Dee_Visit(self->c_curr);
 Dee_Visit(self->c_cat);
}
PRIVATE int DCALL
catiterator_bool(CatIterator *__restrict self) {
 int result; DeeObject **iter,**end;
 DREF DeeObject *curr;
 rwlock_read(&self->c_lock);
 curr = self->c_curr;
 Dee_Incref(curr);
 rwlock_endread(&self->c_lock);
 /* Check if the current iterator has remaining elements. */
 result = DeeObject_Bool(curr);
 Dee_Decref(curr);
 if (result != 0) goto done;
#ifndef CONFIG_NO_THREADS
 iter = (DeeObject **)ATOMIC_READ(self->c_pseq);
#else
 iter = (DeeObject **)self->c_pseq;
#endif
 /* Check if one of the upcoming sequences is non-empty. */
 end = DeeTuple_ELEM(self->c_cat) + DeeTuple_SIZE(self->c_cat);
 for (; iter < end; ++iter) {
  result = DeeObject_Bool(*iter);
  if (result != 0) break;
 }
done:
 return result;
}


#define DEFINE_CATITERATOR_COMPARE(name,if_equal,if_diffseq,compare_object) \
PRIVATE DREF DeeObject *DCALL \
name(CatIterator *__restrict self, \
     CatIterator *__restrict other) { \
 DREF DeeObject *result; \
 DREF DeeObject *my_curr,*ot_curr; \
 DREF DeeObject **my_pseq,**ot_pseq; \
 if (DeeObject_AssertTypeExact((DeeObject *)other,&DeeCatIterator_Type)) \
     return NULL; \
 if (self == other) if_equal; \
 for (;;) { \
  rwlock_read(&self->c_lock); \
  if (!rwlock_tryread(&other->c_lock)) { \
   rwlock_endread(&self->c_lock); \
   rwlock_read(&other->c_lock); \
   if (!rwlock_tryread(&self->c_lock)) \
        continue; \
  } \
  break; \
 } \
 my_pseq = (DREF DeeObject **)self->c_pseq; \
 ot_pseq = (DREF DeeObject **)other->c_pseq; \
 if (my_pseq != ot_pseq) { \
  rwlock_endread(&other->c_lock); \
  rwlock_endread(&self->c_lock); \
  if_diffseq; \
 } \
 my_curr = self->c_curr; \
 Dee_Incref(my_curr); \
 ot_curr = other->c_curr; \
 Dee_Incref(ot_curr); \
 rwlock_endread(&other->c_lock); \
 rwlock_endread(&self->c_lock); \
 result = compare_object(my_curr,ot_curr); \
 Dee_Decref(ot_curr); \
 Dee_Decref(my_curr); \
 return result; \
}
DEFINE_CATITERATOR_COMPARE(catiterator_eq,return_true,return_false,DeeObject_CompareEqObject)
DEFINE_CATITERATOR_COMPARE(catiterator_ne,return_false,return_true,DeeObject_CompareNeObject)
DEFINE_CATITERATOR_COMPARE(catiterator_lo,return_false,return_bool_(my_pseq < ot_pseq),DeeObject_CompareLoObject)
DEFINE_CATITERATOR_COMPARE(catiterator_le,return_true,return_bool_(my_pseq < ot_pseq),DeeObject_CompareLeObject)
DEFINE_CATITERATOR_COMPARE(catiterator_gr,return_false,return_bool_(my_pseq > ot_pseq),DeeObject_CompareGrObject)
DEFINE_CATITERATOR_COMPARE(catiterator_ge,return_true,return_bool_(my_pseq > ot_pseq),DeeObject_CompareGeObject)
#undef DEFINE_CATITERATOR_COMPARE

PRIVATE struct type_cmp catiterator_cmp = {
    /* .tp_hash = */NULL,
    /* .tp_eq   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&catiterator_eq,
    /* .tp_ne   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&catiterator_ne,
    /* .tp_lo   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&catiterator_lo,
    /* .tp_le   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&catiterator_le,
    /* .tp_gr   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&catiterator_gr,
    /* .tp_ge   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&catiterator_ge
};



PRIVATE DREF DeeObject *DCALL
catiterator_next(CatIterator *__restrict self) {
 DREF DeeObject *iter,*result;
again_locked:
 rwlock_read(&self->c_lock);
again:
 iter = self->c_curr;
 Dee_Incref(iter);
 rwlock_endread(&self->c_lock);
do_iter:
 result = DeeObject_IterNext(iter);
 Dee_Decref(iter);
 if (!ITER_ISOK(result)) {
  DeeObject *const *pnext;
  if unlikely(!result) return NULL;
#ifndef CONFIG_NO_THREADS
  rwlock_write(&self->c_lock);
#endif /* !CONFIG_NO_THREADS */
  /* Check if the iterator has changed. */
  if (self->c_curr != iter) {
#ifndef CONFIG_NO_THREADS
   rwlock_downgrade(&self->c_lock);
#endif /* !CONFIG_NO_THREADS */
   goto again;
  }
  /* Load the next sequence. */
  pnext = self->c_pseq+1;
  ASSERT(pnext >  DeeTuple_ELEM(self->c_cat));
  ASSERT(pnext <= DeeTuple_ELEM(self->c_cat)+DeeTuple_SIZE(self->c_cat));
  if unlikely(pnext == (DeeTuple_ELEM(self->c_cat)+
                        DeeTuple_SIZE(self->c_cat))) {
   /* Fully exhausted. */
#ifndef CONFIG_NO_THREADS
   rwlock_endwrite(&self->c_lock);
#endif /* !CONFIG_NO_THREADS */
   return ITER_DONE;
  }
#ifndef CONFIG_NO_THREADS
  rwlock_endwrite(&self->c_lock);
#endif /* !CONFIG_NO_THREADS */
  /* Create an iterator for this sequence. */
  iter = DeeObject_IterSelf(*pnext);
  if unlikely(!iter) return NULL;
#ifndef CONFIG_NO_THREADS
  rwlock_write(&self->c_lock);
#endif /* !CONFIG_NO_THREADS */
  COMPILER_READ_BARRIER();
  /* Check if the sequence was changed by someone else. */
  if (self->c_pseq != pnext-1) {
#ifndef CONFIG_NO_THREADS
   rwlock_endwrite(&self->c_lock);
#endif /* !CONFIG_NO_THREADS */
   Dee_Decref(iter);
   goto again_locked;
  }
  /* Update the current sequence pointer. */
  self->c_pseq = pnext;
  /* Store our new iterator, replacing the previous one. */
  result = self->c_curr;
  self->c_curr = iter;
  Dee_Incref(iter); /* The reference now stored in `self->c_curr' */
#ifndef CONFIG_NO_THREADS
  rwlock_endwrite(&self->c_lock);
#endif /* !CONFIG_NO_THREADS */
  /* Drop the old iterator. */
  Dee_Decref(result);
  goto do_iter;
 }
 return result;
}

PRIVATE DREF DeeObject *DCALL
catiterator_seq_get(CatIterator *__restrict self) {
 DREF DeeObject *result;
#ifndef CONFIG_NO_THREADS
 rwlock_read(&self->c_lock);
#endif /* !CONFIG_NO_THREADS */
 result = *self->c_pseq;
 Dee_Incref(result);
#ifndef CONFIG_NO_THREADS
 rwlock_endread(&self->c_lock);
#endif /* !CONFIG_NO_THREADS */
 return result;
}
PRIVATE DREF DeeObject *DCALL
catiterator_curr_get(CatIterator *__restrict self) {
 DREF DeeObject *result;
#ifndef CONFIG_NO_THREADS
 rwlock_read(&self->c_lock);
#endif /* !CONFIG_NO_THREADS */
 result = self->c_curr;
 Dee_Incref(result);
#ifndef CONFIG_NO_THREADS
 rwlock_endread(&self->c_lock);
#endif /* !CONFIG_NO_THREADS */
 return result;
}
PRIVATE struct type_getset catiterator_getsets[] = {
    { "seq", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&catiterator_seq_get, NULL, NULL },
    { "__curr__", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&catiterator_curr_get, NULL, NULL },
    { NULL }
};
PRIVATE struct type_member catiterator_members[] = {
    TYPE_MEMBER_FIELD("__sequences__",STRUCT_OBJECT,offsetof(CatIterator,c_cat)),
    TYPE_MEMBER_END
};


INTERN DeeTypeObject DeeCatIterator_Type = {
    OBJECT_HEAD_INIT(&DeeType_Type),
    /* .tp_name     = */"_catseqiterator",
    /* .tp_doc      = */NULL,
    /* .tp_flags    = */TP_FNORMAL|TP_FFINAL,
    /* .tp_weakrefs = */0,
    /* .tp_features = */TF_NONE,
    /* .tp_base     = */&DeeIterator_Type,
    /* .tp_init = */{
        {
            /* .tp_alloc = */{
                /* .tp_ctor      = */&catiterator_ctor,
                /* .tp_copy_ctor = */&catiterator_copy,
                /* .tp_deep_ctor = */NULL,
                /* .tp_any_ctor  = */&catiterator_init,
                /* .tp_free      = */NULL,
                {
                    /* .tp_instance_size = */sizeof(CatIterator)
                }
            }
        },
        /* .tp_dtor        = */(void(DCALL *)(DeeObject *__restrict))&catiterator_fini,
        /* .tp_assign      = */NULL,
        /* .tp_move_assign = */NULL
    },
    /* .tp_cast = */{
        /* .tp_str  = */NULL,
        /* .tp_repr = */NULL,
        /* .tp_bool = */(int(DCALL *)(DeeObject *__restrict))&catiterator_bool
    },
    /* .tp_call          = */NULL,
    /* .tp_visit         = */(void(DCALL *)(DeeObject *__restrict,dvisit_t,void*))&catiterator_visit,
    /* .tp_gc            = */NULL,
    /* .tp_math          = */NULL,
    /* .tp_cmp           = */&catiterator_cmp,
    /* .tp_seq           = */NULL,
    /* .tp_iter_next     = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&catiterator_next,
    /* .tp_attr          = */NULL,
    /* .tp_with          = */NULL,
    /* .tp_buffer        = */NULL,
    /* .tp_methods       = */NULL,
    /* .tp_getsets       = */catiterator_getsets,
    /* .tp_members       = */catiterator_members,
    /* .tp_class_methods = */NULL,
    /* .tp_class_getsets = */NULL,
    /* .tp_class_members = */NULL
};


INTDEF void DCALL tuple_fini(Tuple *__restrict self);
INTDEF void DCALL tuple_visit(Tuple *__restrict self, dvisit_t proc, void *arg);
#define cat_fini   tuple_fini
#define cat_visit  tuple_visit

PRIVATE DREF CatIterator *DCALL
cat_iter(Cat *__restrict self) {
 DREF CatIterator *result;
 result = DeeObject_MALLOC(CatIterator);
 if unlikely(!result) goto done;
 ASSERT(DeeTuple_SIZE(self) != 0);
 result->c_curr = DeeObject_IterSelf(DeeTuple_GET(self,0));
 if unlikely(!result->c_curr) goto err_r;
 result->c_pseq = DeeTuple_ELEM(self);
 result->c_cat  = self;
 Dee_Incref(self);
#ifndef CONFIG_NO_THREADS
 rwlock_init(&result->c_lock);
#endif /* !CONFIG_NO_THREADS */
 DeeObject_Init(result,&DeeCatIterator_Type);
done:
 return result;
err_r:
 DeeObject_Free(result);
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
cat_getsequences(Cat *__restrict self) {
 return DeeRefVector_NewReadonly((DeeObject *)self,
                                  DeeTuple_SIZE(self),
                                  DeeTuple_ELEM(self));
}

PRIVATE struct type_getset cat_getsets[] = {
    { "__sequences__", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&cat_getsequences, NULL, NULL,
      DOC("->{sequence...}") },
    { NULL }
};

PRIVATE struct type_member cat_class_members[] = {
    TYPE_MEMBER_CONST("iterator",&DeeCatIterator_Type),
    TYPE_MEMBER_END
};


PRIVATE size_t DCALL
cat_nsi_getsize(Cat *__restrict self) {
 size_t i,result = 0;
 for (i = 0; i < DeeTuple_SIZE(self); ++i) {
  size_t temp = DeeObject_Size(DeeTuple_GET(self,i));
  if unlikely(temp == (size_t)-1) return (size_t)-1;
  if unlikely((result + temp) < result) {
   err_integer_overflow_i(sizeof(size_t)*8,true);
   return (size_t)-1;
  }
  result += temp;
 }
 return result;
}
PRIVATE DREF DeeObject *DCALL
cat_size(Cat *__restrict self) {
 size_t result = cat_nsi_getsize(self);
 if unlikely(result == (size_t)-1) return NULL;
 return DeeInt_NewSize(result);
}
PRIVATE DREF DeeObject *DCALL
cat_contains(Cat *__restrict self, DeeObject *__restrict elem) {
 size_t i;
 for (i = 0; i < DeeTuple_SIZE(self); ++i) {
  DREF DeeObject *result; int error;
  result = DeeObject_ContainsObject(DeeTuple_GET(self,i),elem);
  if unlikely(!result) return NULL;
  error = DeeObject_Bool(result);
  if (error != 0) {
   if unlikely(error < 0)
      Dee_Clear(result);
   return result;
  }
  Dee_Decref(result);
 }
 return_false;
}
PRIVATE DREF DeeObject *DCALL
cat_nsi_getitem(Cat *__restrict self, size_t index) {
 size_t i,temp,sub_index = index,total_length = 0;
 for (i = 0; i < DeeTuple_SIZE(self); ++i) {
  temp = DeeObject_Size(DeeTuple_GET(self,i));
  if (sub_index >= temp) {
   sub_index    -= temp;
   total_length += temp;
   continue;
  }
  return DeeObject_GetItemIndex(DeeTuple_GET(self,i),sub_index);
 }
 err_index_out_of_bounds((DeeObject *)self,index,total_length);
 return NULL;
}
PRIVATE DREF DeeObject *DCALL
cat_getitem(Cat *__restrict self,
            DeeObject *__restrict index_ob) {
 size_t index;
 if (DeeObject_AsSize(index_ob,&index))
     return NULL;
 return cat_nsi_getitem(self,index);
}

PRIVATE size_t DCALL
cat_nsi_find(Cat *__restrict self,
             size_t start, size_t end,
             DeeObject *__restrict elem,
             DeeObject *pred_eq) {
 size_t temp,i,offset = 0;
 for (i = 0; i < DeeTuple_SIZE(self); ++i) {
  temp = DeeSeq_Find(DeeTuple_GET(self,i),start,end,elem,pred_eq);
  if ((dssize_t)temp < 0) {
   if unlikely(temp == (size_t)-2)
      goto err;
   if (temp != (size_t)-1) {
    if unlikely((offset + temp) < offset ||
                (offset + temp) < temp)
       goto index_overflow;
    offset += temp;
    if unlikely(offset == (size_t)-1 ||
                offset == (size_t)-2)
       goto index_overflow;
    return offset;
   }
  }
  temp = DeeObject_Size(DeeTuple_GET(self,i));
  if unlikely(temp == (size_t)-1) goto err;
  if (temp >= end) break;
  start   = 0;
  end    -= temp;
  offset += temp;
 } 
 return (size_t)-1;
index_overflow:
 err_integer_overflow_i(sizeof(size_t)*8,true);
err:
 return (size_t)-2;
}


PRIVATE struct type_nsi cat_nsi = {
    /* .nsi_class   = */TYPE_SEQX_CLASS_SEQ,
    {
        /* .nsi_seqlike = */{
            /* .nsi_getsize      = */(void *)&cat_nsi_getsize,
            /* .nsi_getsize_fast = */(void *)NULL,
            /* .nsi_getitem      = */(void *)&cat_nsi_getitem,
            /* .nsi_delitem      = */(void *)NULL,
            /* .nsi_setitem      = */(void *)NULL,
            /* .nsi_getitem_fast = */(void *)NULL,
            /* .nsi_getrange     = */(void *)NULL,
            /* .nsi_getrange_n   = */(void *)NULL,
            /* .nsi_setrange     = */(void *)NULL,
            /* .nsi_setrange_n   = */(void *)NULL,
            /* .nsi_find         = */(void *)&cat_nsi_find,
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

PRIVATE struct type_seq cat_seq = {
    /* .tp_iter_self = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&cat_iter,
    /* .tp_size      = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&cat_size,
    /* .tp_contains  = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&cat_contains,
    /* .tp_get       = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&cat_getitem,
    /* .tp_del       = */NULL,
    /* .tp_set       = */NULL,
    /* .tp_range_get = */NULL,
    /* .tp_range_del = */NULL,
    /* .tp_range_set = */NULL,
    /* .tp_nsi       = */&cat_nsi
};


INTDEF void DCALL
tuple_tp_free(void *__restrict ob);

PRIVATE int DCALL cat_bool(Cat *__restrict self) {
 size_t i; int temp;
 for (i = 0; i < self->t_size; ++i) {
  temp = DeeObject_Bool(self->t_elem[i]);
  if (temp != 0) return temp;
 }
 return 0;
}


INTERN DeeTypeObject DeeCat_Type = {
    OBJECT_HEAD_INIT(&DeeType_Type),
    /* .tp_name     = */"_catseq",
    /* .tp_doc      = */NULL,
    /* .tp_flags    = */TP_FNORMAL|TP_FVARIABLE|TP_FFINAL,
    /* .tp_weakrefs = */0,
    /* .tp_features = */TF_NONE,
    /* .tp_base     = */&DeeSeq_Type,
    /* .tp_init = */{
        {
            /* .tp_var = */{
                /* .tp_ctor      = */NULL, /* TODO */
                /* .tp_copy_ctor = */NULL,
                /* .tp_deep_ctor = */NULL,
                /* .tp_any_ctor  = */NULL, /* TODO */
                /* .tp_free      = */&tuple_tp_free,
            }
        },
        /* .tp_dtor        = */(void(DCALL *)(DeeObject *__restrict))&cat_fini,
        /* .tp_assign      = */NULL,
        /* .tp_move_assign = */NULL
    },
    /* .tp_cast = */{
        /* .tp_str  = */NULL,
        /* .tp_repr = */NULL,
        /* .tp_bool = */(int(DCALL *)(DeeObject *__restrict))&cat_bool
    },
    /* .tp_call          = */NULL,
    /* .tp_visit         = */(void(DCALL *)(DeeObject *__restrict,dvisit_t,void*))&cat_visit,
    /* .tp_gc            = */NULL,
    /* .tp_math          = */NULL,
    /* .tp_cmp           = */NULL,
    /* .tp_seq           = */&cat_seq,
    /* .tp_iter_next     = */NULL,
    /* .tp_attr          = */NULL,
    /* .tp_with          = */NULL,
    /* .tp_buffer        = */NULL,
    /* .tp_methods       = */NULL,
    /* .tp_getsets       = */cat_getsets,
    /* .tp_members       = */NULL,
    /* .tp_class_methods = */NULL,
    /* .tp_class_getsets = */NULL,
    /* .tp_class_members = */cat_class_members
};

INTERN DREF DeeObject *DCALL
DeeSeq_Concat(DeeObject *__restrict self,
              DeeObject *__restrict other) {
 DREF DeeObject **dst,**iter,**end;
 DREF DeeTupleObject *result;
 /* Special handling for recursive cats. */
 if (DeeObject_InstanceOf(self,&DeeCat_Type)) {
  if (DeeObject_InstanceOf(other,&DeeCat_Type)) {
   result = (DREF DeeTupleObject *)DeeTuple_NewUninitialized(DeeTuple_SIZE(self)+
                                                             DeeTuple_SIZE(other));
   if unlikely(!result) goto err;
   dst = DeeTuple_ELEM(result);
   end = (iter = DeeTuple_ELEM(self))+DeeTuple_SIZE(self);
   for (; iter != end; ++iter,++dst) {
    DeeObject *ob = *iter;
    Dee_Incref(ob);
    *dst = ob;
   }
   end = (iter = DeeTuple_ELEM(other))+DeeTuple_SIZE(other);
   for (; iter != end; ++iter,++dst) {
    DeeObject *ob = *iter;
    Dee_Incref(ob);
    *dst = ob;
   }
  } else {
   result = (DREF DeeTupleObject *)DeeTuple_NewUninitialized(DeeTuple_SIZE(self)+1);
   if unlikely(!result) goto err;
   dst = DeeTuple_ELEM(result);
   end = (iter = DeeTuple_ELEM(self))+DeeTuple_SIZE(self);
   for (; iter != end; ++iter,++dst) {
    DeeObject *ob = *iter;
    Dee_Incref(ob);
    *dst = ob;
   }
   *dst = other;
   Dee_Incref(other);
  }
 } else if (DeeObject_InstanceOf(other,&DeeCat_Type)) {
  result = (DREF DeeTupleObject *)DeeTuple_NewUninitialized(1+DeeTuple_SIZE(other));
  if unlikely(!result) goto err;
  dst = DeeTuple_ELEM(result);
  *dst++ = self;
  Dee_Incref(self);
  end = (iter = DeeTuple_ELEM(other))+DeeTuple_SIZE(other);
  for (; iter != end; ++iter,++dst) {
   DeeObject *ob = *iter;
   Dee_Incref(ob);
   *dst = ob;
  }
 } else {
  result = (DREF DeeTupleObject *)DeeTuple_Pack(2,self,other);
  if unlikely(!result) goto err;
 }
 /* Fix the resulting object type. */
 ASSERT(result->ob_type == &DeeTuple_Type);
 Dee_DecrefNokill(&DeeTuple_Type);
 Dee_Incref(&DeeCat_Type);
 result->ob_type = &DeeCat_Type;
 return (DREF DeeObject *)result;
err:
 return NULL;
}


DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_CONCAT_C */
