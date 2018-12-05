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
#ifndef GUARD_DEEMON_OBJECTS_SEQ_SUBRANGE_C
#define GUARD_DEEMON_OBJECTS_SEQ_SUBRANGE_C 1

#include <deemon/api.h>
#include <deemon/object.h>
#include <deemon/seq.h>
#include <deemon/error.h>
#include <deemon/none.h>
#include <deemon/bool.h>
#include <deemon/thread.h>
#include <deemon/arg.h>
#include <deemon/int.h>
#include <deemon/string.h>

#include <hybrid/minmax.h>

#ifndef CONFIG_NO_THREADS
#include <hybrid/atomic.h>
#endif /* !CONFIG_NO_THREADS */

#include "../../runtime/runtime_error.h"
#include "../../runtime/strings.h"

#include "subrange.h"

DECL_BEGIN

PRIVATE int DCALL
subrangeiterator_init(SubRangeIterator *__restrict self,
                      size_t argc, DeeObject **__restrict argv) {
 self->sr_size = (size_t)-1;
 if (DeeArg_Unpack(argc,argv,"o|" DEE_FMT_SIZE_T ":_SubRangeIterator",
                  &self->sr_iter,&self->sr_size))
     return -1;
 Dee_Incref(self->sr_iter);
 return 0;
}
PRIVATE void DCALL
subrangeiterator_fini(SubRangeIterator *__restrict self) {
 Dee_Decref(self->sr_iter);
}
PRIVATE void DCALL
subrangeiterator_visit(SubRangeIterator *__restrict self,
                       dvisit_t proc, void *arg) {
 Dee_Visit(self->sr_iter);
}

PRIVATE DREF DeeObject *DCALL
subrangeiterator_next(SubRangeIterator *__restrict self) {
#ifdef CONFIG_NO_THREADS
 if (!self->sr_size) return ITER_DONE;
 --self->sr_size;
#else
 size_t oldval;
 /* Consume one from the max-iteration size. */
 do {
  oldval = ATOMIC_READ(self->sr_size);
  if (!oldval) return ITER_DONE;
 } while (!ATOMIC_CMPXCH(self->sr_size,oldval,oldval-1));
#endif
 return DeeObject_IterNext(self->sr_iter);
}

INTDEF DeeTypeObject SeqSubRangeIterator_Type;

#define DEFINE_COMPARE(name,op) \
PRIVATE DREF DeeObject *DCALL \
name(SubRangeIterator *__restrict self, \
     SubRangeIterator *__restrict other) { \
 if (DeeObject_AssertTypeExact((DeeObject *)other,&SeqSubRangeIterator_Type)) \
     return NULL; \
 return_bool(READ_SIZE(other) op READ_SIZE(self)); \
}
DEFINE_COMPARE(subrangeiterator_eq,==)
DEFINE_COMPARE(subrangeiterator_ne,!=)
DEFINE_COMPARE(subrangeiterator_lo,<)
DEFINE_COMPARE(subrangeiterator_le,<=)
DEFINE_COMPARE(subrangeiterator_gr,>)
DEFINE_COMPARE(subrangeiterator_ge,>=)
#undef DEFINE_COMPARE

PRIVATE struct type_cmp subrangeiterator_cmp = {
    /* .tp_hash = */NULL,
    /* .tp_eq   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&subrangeiterator_eq,
    /* .tp_ne   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&subrangeiterator_ne,
    /* .tp_lo   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&subrangeiterator_lo,
    /* .tp_le   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&subrangeiterator_le,
    /* .tp_gr   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&subrangeiterator_gr,
    /* .tp_ge   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&subrangeiterator_ge
};

PRIVATE DREF DeeObject *DCALL
subrangeiterator_seq_get(SubRangeIterator *__restrict self) {
 /* Forward access to this attribute to the pointed-to iterator. */
 /* TODO: this returns the real underlying sequence, when we'd need to
  *       return a sub-range proxy to it, rather than the actual sequence! */
 return DeeObject_GetAttr(self->sr_iter,&str_seq);
}

PRIVATE struct type_getset subrangeiterator_getsets[] = {
    { DeeString_STR(&str_seq),
     (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&subrangeiterator_seq_get,
      NULL, NULL },
    { NULL }
};

PRIVATE struct type_member subrangeiterator_members[] = {
    TYPE_MEMBER_FIELD_DOC("__iter__",STRUCT_OBJECT,offsetof(SubRangeIterator,sr_iter),"->?Diterator"),
    TYPE_MEMBER_FIELD("__size__",STRUCT_SIZE_T,offsetof(SubRangeIterator,sr_size)),
    TYPE_MEMBER_END
};

PRIVATE int DCALL
subrangeiterator_ctor(SubRangeIterator *__restrict self) {
 self->sr_size = 0;
 self->sr_iter = Dee_None;
 Dee_Incref(Dee_None);
 return 0;
}

PRIVATE int DCALL
subrangeiterator_copy(SubRangeIterator *__restrict self,
                      SubRangeIterator *__restrict other) {
 self->sr_size = READ_SIZE(other);
 self->sr_iter = DeeObject_Copy(other->sr_iter);
 if unlikely(!self->sr_iter) return -1;
 return 0;
}

PRIVATE int DCALL
subrangeiterator_bool(SubRangeIterator *__restrict self) {
 if (!READ_SIZE(self)) return 0;
 return DeeObject_Bool(self->sr_iter);
}

INTERN DeeTypeObject SeqSubRangeIterator_Type = {
    OBJECT_HEAD_INIT(&DeeType_Type),
    /* .tp_name     = */"_SeqSubRangeIterator",
    /* .tp_doc      = */NULL,
    /* .tp_flags    = */TP_FNORMAL|TP_FFINAL,
    /* .tp_weakrefs = */0,
    /* .tp_features = */TF_NONE,
    /* .tp_base     = */&DeeIterator_Type,
    /* .tp_init = */{
        {
            /* .tp_alloc = */{
                /* .tp_ctor      = */&subrangeiterator_ctor,
                /* .tp_copy_ctor = */&subrangeiterator_copy,
                /* .tp_deep_ctor = */NULL,
                /* .tp_any_ctor  = */&subrangeiterator_init,
                TYPE_FIXED_ALLOCATOR(SubRangeIterator)
            }
        },
        /* .tp_dtor        = */(void(DCALL *)(DeeObject *__restrict))&subrangeiterator_fini,
        /* .tp_assign      = */NULL,
        /* .tp_move_assign = */NULL
    },
    /* .tp_cast = */{
        /* .tp_str  = */NULL,
        /* .tp_repr = */NULL,
        /* .tp_bool = */(int(DCALL *)(DeeObject *__restrict))&subrangeiterator_bool
    },
    /* .tp_call          = */NULL,
    /* .tp_visit         = */(void(DCALL *)(DeeObject *__restrict,dvisit_t,void*))&subrangeiterator_visit,
    /* .tp_gc            = */NULL,
    /* .tp_math          = */NULL,
    /* .tp_cmp           = */&subrangeiterator_cmp,
    /* .tp_seq           = */NULL,
    /* .tp_iter_next     = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&subrangeiterator_next,
    /* .tp_attr          = */NULL,
    /* .tp_with          = */NULL,
    /* .tp_buffer        = */NULL,
    /* .tp_methods       = */NULL,
    /* .tp_getsets       = */subrangeiterator_getsets,
    /* .tp_members       = */subrangeiterator_members,
    /* .tp_class_methods = */NULL,
    /* .tp_class_getsets = */NULL,
    /* .tp_class_members = */NULL
};




PRIVATE void DCALL
subrange_fini(SubRange *__restrict self) {
 Dee_Decref(self->sr_seq);
}
PRIVATE void DCALL
subrange_visit(SubRange *__restrict self, dvisit_t proc, void *arg) {
 Dee_Visit(self->sr_seq);
}

PRIVATE DREF DeeObject *DCALL
subrange_size(SubRange *__restrict self) {
 size_t inner_size = DeeObject_Size(self->sr_seq);
 if unlikely(inner_size == (size_t)-1) return NULL;
 if (self->sr_begin >= inner_size) return_reference_(&DeeInt_Zero);
 inner_size -= self->sr_begin;
 if (inner_size > self->sr_size)
     inner_size = self->sr_size;
 return DeeInt_NewSize(inner_size);
}
PRIVATE DREF DeeObject *DCALL
subrange_iter(SubRange *__restrict self) {
 DREF SubRangeIterator *result;
 DREF DeeObject *iterator; size_t begin_index;
 iterator = DeeObject_IterSelf(self->sr_seq);
 if unlikely(!iterator) return NULL;
 result = DeeObject_MALLOC(SubRangeIterator);
 if unlikely(!result) goto err_iterator;
 begin_index = self->sr_begin;
 result->sr_size = self->sr_size;
 result->sr_iter = iterator; /* Inherit reference. */
 /* Discard leading items. */
 while (begin_index--) {
  DREF DeeObject *discard;
  discard = DeeObject_IterNext(iterator);
  if unlikely(!ITER_ISOK(discard)) {
   if (!discard) goto err_iterator_r;
   /* Empty iterator (the base iterator was exhausted during the discard-phase) */
   result->sr_size = 0;
   break;
  }
  Dee_Decref(discard);
  if (DeeThread_CheckInterrupt())
      goto err_iterator_r;
 }
 DeeObject_Init(result,&SeqSubRangeIterator_Type);
 return (DREF DeeObject *)result;
err_iterator_r:
 DeeObject_FREE(result);
err_iterator:
 Dee_Decref(iterator);
/*err:*/
 return NULL;
}

PRIVATE struct type_member subrange_members[] = {
    TYPE_MEMBER_FIELD_DOC("__seq__",STRUCT_OBJECT,offsetof(SubRange,sr_seq),"->?Dsequence"),
    TYPE_MEMBER_FIELD("__begin__",STRUCT_CONST|STRUCT_SIZE_T,offsetof(SubRange,sr_begin)),
    TYPE_MEMBER_FIELD("__size__",STRUCT_CONST|STRUCT_SIZE_T,offsetof(SubRange,sr_size)),
    TYPE_MEMBER_END
};


PRIVATE struct type_member subrange_class_members[] = {
    TYPE_MEMBER_CONST("iterator",&SeqSubRangeIterator_Type),
    TYPE_MEMBER_END
};

PRIVATE DREF DeeObject *DCALL
subrange_getitem(SubRange *__restrict self,
                 DeeObject *__restrict index_ob) {
 size_t index;
 if (DeeObject_AsSize(index_ob,&index)) return NULL;
 return DeeObject_GetItemIndex(self->sr_seq,self->sr_begin+index);
}

PRIVATE size_t DCALL
subrange_nsi_getsize(SubRange *__restrict self) {
 size_t inner_size = DeeObject_Size(self->sr_seq);
 if unlikely(inner_size == (size_t)-1) return (size_t)-1;
 if (self->sr_begin >= inner_size) return 0;
 inner_size -= self->sr_begin;
 if (inner_size > self->sr_size)
     inner_size = self->sr_size;
 return inner_size;
}
PRIVATE size_t DCALL
subrange_nsi_getsize_fast(SubRange *__restrict self) {
 size_t inner_size = DeeFastSeq_GetSize(self->sr_seq);
 if unlikely(inner_size == (size_t)-1) return (size_t)-1;
 if (self->sr_begin >= inner_size) return 0;
 inner_size -= self->sr_begin;
 if (inner_size > self->sr_size)
     inner_size = self->sr_size;
 return inner_size;
}
PRIVATE DREF DeeObject *DCALL
subrange_nsi_getitem(SubRange *__restrict self, size_t index) {
 return DeeObject_GetItemIndex(self->sr_seq,self->sr_begin+index);
}
PRIVATE size_t DCALL
subrange_nsi_find(SubRange *__restrict self, size_t start, size_t end,
                  DeeObject *__restrict keyed_search_item,
                  DeeObject *key) {
 if (start >= self->sr_size) return (size_t)-1;
 if (end > self->sr_size) end = self->sr_size;
 return DeeSeq_Find(self->sr_seq,
                    start + self->sr_begin,
                    end + self->sr_begin,
                    keyed_search_item,
                    key);
}
PRIVATE size_t DCALL
subrange_nsi_rfind(SubRange *__restrict self, size_t start, size_t end,
                   DeeObject *__restrict keyed_search_item, DeeObject *key) {
 if (start >= self->sr_size) return (size_t)-1;
 if (end > self->sr_size) end = self->sr_size;
 return DeeSeq_RFind(self->sr_seq,
                     start + self->sr_begin,
                     end + self->sr_begin,
                     keyed_search_item,
                     key);
}


PRIVATE struct type_nsi subrange_nsi = {
    /* .nsi_class   = */TYPE_SEQX_CLASS_SEQ,
    /* .nsi_flags   = */TYPE_SEQX_FNORMAL,
    {
        /* .nsi_seqlike = */{
            /* .nsi_getsize      = */(void *)&subrange_nsi_getsize,
            /* .nsi_getsize_fast = */(void *)&subrange_nsi_getsize_fast,
            /* .nsi_getitem      = */(void *)&subrange_nsi_getitem,
            /* .nsi_delitem      = */(void *)NULL,
            /* .nsi_setitem      = */(void *)NULL,
            /* .nsi_getitem_fast = */(void *)NULL,
            /* .nsi_getrange     = */(void *)NULL,
            /* .nsi_getrange_n   = */(void *)NULL,
            /* .nsi_setrange     = */(void *)NULL,
            /* .nsi_setrange_n   = */(void *)NULL,
            /* .nsi_find         = */(void *)&subrange_nsi_find,
            /* .nsi_rfind        = */(void *)&subrange_nsi_rfind,
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

PRIVATE struct type_seq subrange_seq = {
    /* .tp_iter_self = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&subrange_iter,
    /* .tp_size      = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&subrange_size,
    /* .tp_contains  = */NULL,
    /* .tp_get       = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&subrange_getitem,
    /* .tp_del       = */NULL,
    /* .tp_set       = */NULL,
    /* .tp_range_get = */NULL,
    /* .tp_range_del = */NULL,
    /* .tp_range_set = */NULL,
    /* .tp_nsi       = */&subrange_nsi
};

INTERN DeeTypeObject SeqSubRange_Type = {
    OBJECT_HEAD_INIT(&DeeType_Type),
    /* .tp_name     = */"_SeqSubRange",
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
                /* .tp_deep_ctor = */NULL,
                /* .tp_any_ctor  = */NULL, /* TODO */
                TYPE_FIXED_ALLOCATOR(SubRange)
            }
        },
        /* .tp_dtor        = */(void(DCALL *)(DeeObject *__restrict))&subrange_fini,
        /* .tp_assign      = */NULL,
        /* .tp_move_assign = */NULL
    },
    /* .tp_cast = */{
        /* .tp_str  = */NULL,
        /* .tp_repr = */NULL,
        /* .tp_bool = */NULL
    },
    /* .tp_call          = */NULL,
    /* .tp_visit         = */(void(DCALL *)(DeeObject *__restrict,dvisit_t,void*))&subrange_visit,
    /* .tp_gc            = */NULL,
    /* .tp_math          = */NULL,
    /* .tp_cmp           = */NULL,
    /* .tp_seq           = */&subrange_seq,
    /* .tp_iter_next     = */NULL,
    /* .tp_attr          = */NULL,
    /* .tp_with          = */NULL,
    /* .tp_buffer        = */NULL,
    /* .tp_methods       = */NULL,
    /* .tp_getsets       = */NULL,
    /* .tp_members       = */subrange_members,
    /* .tp_class_methods = */NULL,
    /* .tp_class_getsets = */NULL,
    /* .tp_class_members = */subrange_class_members
};

INTERN DREF DeeObject *DCALL
DeeSeq_GetRange(DeeObject *__restrict self,
                size_t begin, size_t end) {
 DREF SubRange *result;
 if unlikely(begin >= end)
    return_reference_(Dee_EmptySeq);
 /* Create a sub-range sequence. */
 result = DeeObject_MALLOC(SubRange);
 if unlikely(!result) goto done;
 if (DeeObject_InstanceOfExact(self,&SeqSubRange_Type)) {
  SubRange *me = (SubRange *)self;
  /* Special handling for recursion. */
  Dee_Incref(me->sr_seq);
  result->sr_seq   = me->sr_seq;
  result->sr_begin = begin+me->sr_begin;
  result->sr_size  = MIN((size_t)(end-begin),me->sr_size);
 } else {
  Dee_Incref(self);
  result->sr_seq   = self;
  result->sr_begin = begin;
  result->sr_size  = (size_t)(end-begin);
 }
 DeeObject_Init(result,&SeqSubRange_Type);
done:
 return (DREF DeeObject *)result;
}

INTERN DREF DeeObject *DCALL
DeeSeq_GetRangeN(DeeObject *__restrict self,
                 size_t begin) {
 DREF SubRangeN *result;
 if (!begin)
      return_reference_(self);
 /* Create a sub-range sequence. */
 result = DeeObject_MALLOC(SubRangeN);
 if unlikely(!result) goto done;
 if (DeeObject_InstanceOfExact(self,&SeqSubRangeN_Type)) {
  SubRangeN *me = (SubRangeN *)self;
  /* Special handling for recursion. */
  Dee_Incref(me->sr_seq);
  result->sr_seq   = me->sr_seq;
  result->sr_begin = begin+me->sr_begin;
 } else {
  Dee_Incref(self);
  result->sr_seq   = self;
  result->sr_begin = begin;
 }
 DeeObject_Init(result,&SeqSubRangeN_Type);
done:
 return (DREF DeeObject *)result;
}




STATIC_ASSERT(COMPILER_OFFSETOF(SubRangeN,sr_seq) ==
              COMPILER_OFFSETOF(SubRange,sr_seq));
#define subrangen_fini  subrange_fini
#define subrangen_visit subrange_visit


PRIVATE DREF DeeObject *DCALL
subrangen_iter(SubRangeN *__restrict self) {
 DREF DeeObject *result,*elem; size_t offset;
 result = DeeObject_IterSelf(self->sr_seq);
 if unlikely(!result) goto done;
 offset = self->sr_begin;
 while (offset--) {
  elem = DeeObject_IterNext(result);
  if (!ITER_ISOK(elem)) {
   if (!elem) goto err;
   break; /* End of sequence. */
  }
  Dee_Decref(elem);
  if (DeeThread_CheckInterrupt())
      goto err;
 }
done:
 return result;
err:
 Dee_Decref(result);
 return NULL;
}

PRIVATE size_t DCALL
subrangen_nsi_getsize(SubRangeN *__restrict self) {
 size_t result;
 result = DeeObject_Size(self->sr_seq);
 if likely(result != (size_t)-1) {
  if (result <= self->sr_begin)
   result = 0;
  else {
   result -= self->sr_begin;
  }
 }
 return result;
}
PRIVATE size_t DCALL
subrangen_nsi_getsize_fast(SubRangeN *__restrict self) {
 size_t result;
 result = DeeFastSeq_GetSize(self->sr_seq);
 if likely(result != (size_t)-1) {
  if (result <= self->sr_begin)
   result = 0;
  else {
   result -= self->sr_begin;
  }
 }
 return result;
}
PRIVATE DREF DeeObject *DCALL
subrangen_size(SubRangeN *__restrict self) {
 size_t result = subrangen_nsi_getsize(self);
 if unlikely(result == (size_t)-1) return NULL;
 return DeeInt_NewSize(result);
}
PRIVATE DREF DeeObject *DCALL
subrangen_getitem(SubRangeN *__restrict self,
                  DeeObject *__restrict index_ob) {
 size_t index;
 if (DeeObject_AsSize(index_ob,&index)) return NULL;
 return DeeObject_GetItemIndex(self->sr_seq,self->sr_begin+index);
}

PRIVATE DREF DeeObject *DCALL
subrangen_nsi_getitem(SubRangeN *__restrict self, size_t index) {
 return DeeObject_GetItemIndex(self->sr_seq,self->sr_begin+index);
}

PRIVATE size_t DCALL
subrangen_nsi_find(SubRangeN *__restrict self, size_t start, size_t end,
                   DeeObject *__restrict keyed_search_item, DeeObject *key) {
 return DeeSeq_Find(self->sr_seq,
                    start + self->sr_begin,
                    end + self->sr_begin,
                    keyed_search_item,
                    key);
}
PRIVATE size_t DCALL
subrangen_nsi_rfind(SubRangeN *__restrict self, size_t start, size_t end,
                    DeeObject *__restrict keyed_search_item, DeeObject *key) {
 return DeeSeq_RFind(self->sr_seq,
                     start + self->sr_begin,
                     end + self->sr_begin,
                     keyed_search_item,
                     key);
}

PRIVATE struct type_nsi subrangen_nsi = {
    /* .nsi_class   = */TYPE_SEQX_CLASS_SEQ,
    /* .nsi_flags   = */TYPE_SEQX_FNORMAL,
    {
        /* .nsi_seqlike = */{
            /* .nsi_getsize      = */(void *)&subrangen_nsi_getsize,
            /* .nsi_getsize_fast = */(void *)&subrangen_nsi_getsize_fast,
            /* .nsi_getitem      = */(void *)&subrangen_nsi_getitem,
            /* .nsi_delitem      = */(void *)NULL,
            /* .nsi_setitem      = */(void *)NULL,
            /* .nsi_getitem_fast = */(void *)NULL,
            /* .nsi_getrange     = */(void *)NULL,
            /* .nsi_getrange_n   = */(void *)NULL,
            /* .nsi_setrange     = */(void *)NULL,
            /* .nsi_setrange_n   = */(void *)NULL,
            /* .nsi_find         = */(void *)&subrangen_nsi_find,
            /* .nsi_rfind        = */(void *)&subrangen_nsi_rfind,
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

PRIVATE struct type_seq subrangen_seq = {
    /* .tp_iter_self = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&subrangen_iter,
    /* .tp_size      = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&subrangen_size,
    /* .tp_contains  = */NULL,
    /* .tp_get       = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&subrangen_getitem,
    /* .tp_del       = */NULL,
    /* .tp_set       = */NULL,
    /* .tp_range_get = */NULL,
    /* .tp_range_del = */NULL,
    /* .tp_range_set = */NULL,
    /* .tp_nsi       = */&subrangen_nsi
};

PRIVATE struct type_member subrangen_members[] = {
    TYPE_MEMBER_FIELD_DOC("__seq__",STRUCT_OBJECT,offsetof(SubRangeN,sr_seq),"->?Dsequence"),
    TYPE_MEMBER_FIELD("__begin__",STRUCT_CONST|STRUCT_SIZE_T,offsetof(SubRangeN,sr_begin)),
    TYPE_MEMBER_END
};


PRIVATE struct type_member subrangen_class_members[] = {
    TYPE_MEMBER_CONST("iterator",&DeeIterator_Type),
    TYPE_MEMBER_END
};

INTERN DeeTypeObject SeqSubRangeN_Type = {
    OBJECT_HEAD_INIT(&DeeType_Type),
    /* .tp_name     = */"_SeqSubRangeN",
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
                /* .tp_deep_ctor = */NULL,
                /* .tp_any_ctor  = */NULL, /* TODO */
                TYPE_FIXED_ALLOCATOR(SubRangeN)
            }
        },
        /* .tp_dtor        = */(void(DCALL *)(DeeObject *__restrict))&subrangen_fini,
        /* .tp_assign      = */NULL,
        /* .tp_move_assign = */NULL
    },
    /* .tp_cast = */{
        /* .tp_str  = */NULL,
        /* .tp_repr = */NULL,
        /* .tp_bool = */NULL
    },
    /* .tp_call          = */NULL,
    /* .tp_visit         = */(void(DCALL *)(DeeObject *__restrict,dvisit_t,void*))&subrangen_visit,
    /* .tp_gc            = */NULL,
    /* .tp_math          = */NULL,
    /* .tp_cmp           = */NULL,
    /* .tp_seq           = */&subrangen_seq,
    /* .tp_iter_next     = */NULL,
    /* .tp_attr          = */NULL,
    /* .tp_with          = */NULL,
    /* .tp_buffer        = */NULL,
    /* .tp_methods       = */NULL,
    /* .tp_getsets       = */NULL,
    /* .tp_members       = */subrangen_members,
    /* .tp_class_methods = */NULL,
    /* .tp_class_getsets = */NULL,
    /* .tp_class_members = */subrangen_class_members
};


DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_SUBRANGE_C */
