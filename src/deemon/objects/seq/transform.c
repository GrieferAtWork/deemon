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
#ifndef GUARD_DEEMON_OBJECTS_SEQ_TRANSFORM_C
#define GUARD_DEEMON_OBJECTS_SEQ_TRANSFORM_C 1

#include <deemon/api.h>
#include <deemon/alloc.h>
#include <deemon/object.h>
#include <deemon/seq.h>
#include <deemon/error.h>
#include <deemon/none.h>
#include <deemon/bool.h>
#include <deemon/arg.h>
#include <deemon/string.h>

#include <hybrid/minmax.h>

#ifndef CONFIG_NO_THREADS
#include <hybrid/atomic.h>
#endif /* !CONFIG_NO_THREADS */

#include "../../runtime/runtime_error.h"
#include "../../runtime/strings.h"

#include "transform.h"

DECL_BEGIN

PRIVATE void DCALL
transiter_fini(TransformationIterator *__restrict self) {
 Dee_Decref(self->ti_iter);
 Dee_Decref(self->ti_func);
}
PRIVATE void DCALL
transiter_visit(TransformationIterator *__restrict self, dvisit_t proc, void *arg) {
 Dee_Visit(self->ti_iter);
 Dee_Visit(self->ti_func);
}
PRIVATE int DCALL
transiter_bool(TransformationIterator *__restrict self) {
 return DeeObject_Bool(self->ti_iter);
}

PRIVATE DREF DeeObject *DCALL
transiter_next(TransformationIterator *__restrict self) {
 DREF DeeObject *result;
 result = DeeObject_IterNext(self->ti_iter);
 if (ITER_ISOK(result)) {
  DREF DeeObject *new_result;
  /* Invoke the transformation callback. */
  new_result = DeeObject_Call(self->ti_func,1,&result);
  Dee_Decref(result);
  result = new_result;
 }
 return result;
}

PRIVATE DREF DeeObject *DCALL
transiter_seq_get(TransformationIterator *__restrict self) {
 /* Forward access to this attribute to the pointed-to iterator. */
 DREF DeeObject *orig,*result;
 orig = DeeObject_GetAttr(self->ti_iter,&str_seq);
 if unlikely(!orig) return NULL;
 result = DeeSeq_Transform(orig,self->ti_func);
 Dee_Decref(orig);
 return result;
}

PRIVATE struct type_getset transiter_getsets[] = {
    { DeeString_STR(&str_seq), (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&transiter_seq_get, NULL, NULL },
    { NULL }
};

PRIVATE struct type_member transiter_members[] = {
    TYPE_MEMBER_FIELD_DOC("__iter__",STRUCT_OBJECT,offsetof(TransformationIterator,ti_iter),"->?Diterator"),
    TYPE_MEMBER_FIELD_DOC("__func__",STRUCT_OBJECT,offsetof(TransformationIterator,ti_func),"->?Dcallable"),
    TYPE_MEMBER_END
};

#define DEFINE_COMPARE(name,base,opname) \
PRIVATE DREF DeeObject *DCALL \
name(TransformationIterator *__restrict self, \
     TransformationIterator *__restrict other) { \
 if (DeeObject_AssertTypeExact((DeeObject *)other,&SeqTransformationIterator_Type)) \
     return NULL; \
 if (self->ti_func != other->ti_func) { \
     int temp = DeeObject_CompareEq(self->ti_func,other->ti_func); \
     if (temp <= 0) { \
         if (temp == 0) \
             err_unimplemented_operator(&SeqTransformationIterator_Type,opname); \
         return NULL; \
     } \
 } \
 return base((DeeObject *)self,(DeeObject *)other); \
}
DEFINE_COMPARE(transiter_eq,DeeObject_CompareEqObject,OPERATOR_EQ)
DEFINE_COMPARE(transiter_ne,DeeObject_CompareNeObject,OPERATOR_NE)
DEFINE_COMPARE(transiter_lo,DeeObject_CompareLoObject,OPERATOR_LO)
DEFINE_COMPARE(transiter_le,DeeObject_CompareLeObject,OPERATOR_LE)
DEFINE_COMPARE(transiter_gr,DeeObject_CompareGrObject,OPERATOR_GR)
DEFINE_COMPARE(transiter_ge,DeeObject_CompareGeObject,OPERATOR_GE)
#undef DEFINE_COMPARE

PRIVATE struct type_cmp transiter_cmp = {
    /* .tp_hash = */NULL,
    /* .tp_eq   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&transiter_eq,
    /* .tp_ne   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&transiter_ne,
    /* .tp_lo   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&transiter_lo,
    /* .tp_le   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&transiter_le,
    /* .tp_gr   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&transiter_gr,
    /* .tp_ge   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&transiter_ge
};

PRIVATE int DCALL
transiter_copy(TransformationIterator *__restrict self,
               TransformationIterator *__restrict other) {
 self->ti_iter = DeeObject_Copy(other->ti_iter);
 if unlikely(!self->ti_iter) return -1;
 self->ti_func = other->ti_func;
 Dee_Incref(self->ti_func);
 return 0;
}
PRIVATE int DCALL
transiter_ctor(TransformationIterator *__restrict self) {
 self->ti_iter = DeeObject_IterSelf(Dee_EmptySeq);
 if unlikely(!self->ti_iter) return -1;
 self->ti_func = Dee_None;
 Dee_Incref(Dee_None);
 return 0;
}
PRIVATE int DCALL
transiter_init(TransformationIterator *__restrict self,
               size_t argc, DeeObject **__restrict argv) {
 Transformation *trans;
 if (DeeArg_Unpack(argc,argv,"o:_SeqTransformationIterator",&trans) ||
     DeeObject_AssertTypeExact((DeeObject *)trans,&SeqTransformation_Type))
     return -1;
 self->ti_iter = DeeObject_IterSelf(trans->t_seq);
 if unlikely(!self->ti_iter) return -1;
 self->ti_func = trans->t_fun;
 Dee_Incref(self->ti_func);
 return 0;
}

INTERN DeeTypeObject SeqTransformationIterator_Type = {
    OBJECT_HEAD_INIT(&DeeType_Type),
    /* .tp_name     = */"_SeqTransformationIterator",
    /* .tp_doc      = */NULL,
    /* .tp_flags    = */TP_FNORMAL|TP_FFINAL,
    /* .tp_weakrefs = */0,
    /* .tp_features = */TF_NONE,
    /* .tp_base     = */&DeeIterator_Type,
    /* .tp_init = */{
        {
            /* .tp_alloc = */{
                /* .tp_ctor      = */(void *)&transiter_ctor,
                /* .tp_copy_ctor = */(void *)&transiter_copy,
                /* .tp_deep_ctor = */(void *)NULL, /* TODO */
                /* .tp_any_ctor  = */(void *)&transiter_init,
                TYPE_FIXED_ALLOCATOR(TransformationIterator)
            }
        },
        /* .tp_dtor        = */(void(DCALL *)(DeeObject *__restrict))&transiter_fini,
        /* .tp_assign      = */NULL,
        /* .tp_move_assign = */NULL
    },
    /* .tp_cast = */{
        /* .tp_str  = */NULL,
        /* .tp_repr = */NULL,
        /* .tp_bool = */(int(DCALL *)(DeeObject *__restrict))&transiter_bool
    },
    /* .tp_call          = */NULL,
    /* .tp_visit         = */(void(DCALL *)(DeeObject *__restrict,dvisit_t,void*))&transiter_visit,
    /* .tp_gc            = */NULL,
    /* .tp_math          = */NULL,
    /* .tp_cmp           = */&transiter_cmp,
    /* .tp_seq           = */NULL,
    /* .tp_iter_next     = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&transiter_next,
    /* .tp_attr          = */NULL,
    /* .tp_with          = */NULL,
    /* .tp_buffer        = */NULL,
    /* .tp_methods       = */NULL,
    /* .tp_getsets       = */transiter_getsets,
    /* .tp_members       = */transiter_members,
    /* .tp_class_methods = */NULL,
    /* .tp_class_getsets = */NULL,
    /* .tp_class_members = */NULL
};

PRIVATE void DCALL
trans_fini(Transformation *__restrict self) {
 Dee_Decref(self->t_seq);
 Dee_Decref(self->t_fun);
}
PRIVATE void DCALL
trans_visit(Transformation *__restrict self, dvisit_t proc, void *arg) {
 Dee_Visit(self->t_seq);
 Dee_Visit(self->t_fun);
}
PRIVATE int DCALL
trans_bool(Transformation *__restrict self) {
 return DeeObject_Bool(self->t_seq);
}

PRIVATE DREF DeeObject *DCALL
trans_iter(Transformation *__restrict self) {
 DREF TransformationIterator *result;
 result = DeeObject_MALLOC(TransformationIterator);
 if unlikely(!result) goto err;
 /* Create the underlying iterator. */
 result->ti_iter = DeeObject_IterSelf(self->t_seq);
 if unlikely(!result->ti_iter) goto err_r;
 /* Assign the transformation functions. */
 result->ti_func = self->t_fun;
 Dee_Incref(self->t_fun);
 DeeObject_Init(result,&SeqTransformationIterator_Type);
 return (DREF DeeObject *)result;
err_r:
 DeeObject_FREE(result);
err:
 return NULL;
}

PRIVATE struct type_member trans_members[] = {
   TYPE_MEMBER_FIELD_DOC("__seq__",STRUCT_OBJECT,offsetof(Transformation,t_seq),"->?Dsequence"),
   TYPE_MEMBER_FIELD_DOC("__func__",STRUCT_OBJECT,offsetof(Transformation,t_fun),"->?Dcallable"),
   TYPE_MEMBER_END
};

PRIVATE struct type_member trans_class_members[] = {
   TYPE_MEMBER_CONST("iterator",&SeqTransformationIterator_Type),
   TYPE_MEMBER_END
};

PRIVATE DREF DeeObject *DCALL
trans_size(Transformation *__restrict self) {
 return DeeObject_SizeObject(self->t_seq);
}

PRIVATE DREF DeeObject *DCALL
trans_getitem(Transformation *__restrict self,
              DeeObject *__restrict index) {
 DREF DeeObject *orig,*result;
 orig = DeeObject_GetItem(self->t_seq,index);
 if unlikely(!orig) return NULL;
 result = DeeObject_Call(self->t_fun,1,&orig);
 Dee_Decref(orig);
 return result;
}

PRIVATE DREF DeeObject *DCALL
trans_getrange(Transformation *__restrict self,
               DeeObject *__restrict start,
               DeeObject *__restrict end) {
 DREF DeeObject *orig,*result;
 orig = DeeObject_GetRange(self->t_seq,start,end);
 if unlikely(!orig) return NULL;
 result = DeeSeq_Transform(orig,self->t_fun);
 Dee_Decref(orig);
 return result;
}


PRIVATE size_t DCALL
trans_nsi_getsize(Transformation *__restrict self) {
 return DeeObject_Size(self->t_seq);
}
PRIVATE size_t DCALL
trans_nsi_getsize_fast(Transformation *__restrict self) {
 return DeeFastSeq_GetSize(self->t_seq);
}
PRIVATE DREF DeeObject *DCALL
trans_nsi_getitem(Transformation *__restrict self, size_t index) {
 DREF DeeObject *inner[1],*result;
 inner[0] = DeeObject_GetItemIndex(self->t_seq,index);
 if unlikely(!inner[0]) return NULL;
 result = DeeObject_Call(self->t_fun,1,inner);
 Dee_Decref(inner[0]);
 return result;
}


PRIVATE struct type_nsi trans_nsi = {
    /* .nsi_class   = */TYPE_SEQX_CLASS_SEQ,
    /* .nsi_flags   = */TYPE_SEQX_FNORMAL,
    {
        /* .nsi_seqlike = */{
            /* .nsi_getsize      = */(void *)&trans_nsi_getsize,
            /* .nsi_getsize_fast = */(void *)&trans_nsi_getsize_fast,
            /* .nsi_getitem      = */(void *)&trans_nsi_getitem,
            /* .nsi_delitem      = */(void *)NULL,
            /* .nsi_setitem      = */(void *)NULL,
            /* .nsi_getitem_fast = */(void *)NULL,
            /* .nsi_getrange     = */(void *)NULL, /* TODO */
            /* .nsi_getrange_n   = */(void *)NULL, /* TODO */
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

PRIVATE struct type_seq trans_seq = {
    /* .tp_iter_self = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&trans_iter,
    /* .tp_size      = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&trans_size,
    /* .tp_contains  = */NULL,
    /* .tp_get       = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&trans_getitem,
    /* .tp_del       = */NULL,
    /* .tp_set       = */NULL,
    /* .tp_range_get = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict,DeeObject *__restrict))&trans_getrange,
    /* .tp_range_del = */NULL,
    /* .tp_range_set = */NULL,
    /* .tp_nsi       = */&trans_nsi
};

PRIVATE int DCALL
trans_init(Transformation *__restrict self,
           size_t argc, DeeObject **__restrict argv) {
 if (DeeArg_Unpack(argc,argv,"oo:_SeqTransformation",
                  &self->t_seq,&self->t_fun))
     return -1;
 Dee_Incref(self->t_seq);
 Dee_Incref(self->t_fun);
 return 0;
}

INTERN DeeTypeObject SeqTransformation_Type = {
    OBJECT_HEAD_INIT(&DeeType_Type),
    /* .tp_name     = */"_SeqTransformation",
    /* .tp_doc      = */DOC("(seq:?Dsequence,fun:?Dcallable)"),
    /* .tp_flags    = */TP_FNORMAL|TP_FFINAL,
    /* .tp_weakrefs = */0,
    /* .tp_features = */TF_NONE,
    /* .tp_base     = */&DeeSeq_Type,
    /* .tp_init = */{
        {
            /* .tp_alloc = */{
                /* .tp_ctor      = */(void *)NULL, /* TODO */
                /* .tp_copy_ctor = */(void *)NULL, /* TODO */
                /* .tp_deep_ctor = */(void *)NULL, /* TODO */
                /* .tp_any_ctor  = */(void *)&trans_init,
                TYPE_FIXED_ALLOCATOR(Transformation)
            }
        },
        /* .tp_dtor        = */(void(DCALL *)(DeeObject *__restrict))&trans_fini,
        /* .tp_assign      = */NULL,
        /* .tp_move_assign = */NULL
    },
    /* .tp_cast = */{
        /* .tp_str  = */NULL,
        /* .tp_repr = */NULL,
        /* .tp_bool = */(int(DCALL *)(DeeObject *__restrict))&trans_bool
    },
    /* .tp_call          = */NULL,
    /* .tp_visit         = */(void(DCALL *)(DeeObject *__restrict,dvisit_t,void*))&trans_visit,
    /* .tp_gc            = */NULL,
    /* .tp_math          = */NULL,
    /* .tp_cmp           = */NULL,
    /* .tp_seq           = */&trans_seq,
    /* .tp_iter_next     = */NULL,
    /* .tp_attr          = */NULL,
    /* .tp_with          = */NULL,
    /* .tp_buffer        = */NULL,
    /* .tp_methods       = */NULL,
    /* .tp_getsets       = */NULL,
    /* .tp_members       = */trans_members,
    /* .tp_class_methods = */NULL,
    /* .tp_class_getsets = */NULL,
    /* .tp_class_members = */trans_class_members
};



INTERN DREF DeeObject *DCALL
DeeSeq_Transform(DeeObject *__restrict self,
                 DeeObject *__restrict transformation) {
 DREF Transformation *result;
 /* Create a new transformation sequence. */
 result = DeeObject_MALLOC(Transformation);
 if unlikely(!result) goto done;
 result->t_seq = self;
 result->t_fun = transformation;
 Dee_Incref(self);
 Dee_Incref(transformation);
 DeeObject_Init(result,&SeqTransformation_Type);
done:
 return (DREF DeeObject *)result;
}

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_TRANSFORM_C */
