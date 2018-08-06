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
#ifndef GUARD_DEEMON_OBJECTS_SEQ_FILTER_C
#define GUARD_DEEMON_OBJECTS_SEQ_FILTER_C 1

#include <deemon/api.h>
#include <deemon/object.h>
#include <deemon/seq.h>
#include <deemon/none.h>
#include <deemon/arg.h>

#include "filter.h"
#include "../../runtime/strings.h"

DECL_BEGIN

PRIVATE void DCALL
filter_fini(Filter *__restrict self) {
 Dee_Decref(self->f_seq);
 Dee_Decref(self->f_fun);
}
PRIVATE void DCALL
filter_visit(FilterIterator *__restrict self, dvisit_t proc, void *arg) {
 Dee_Visit(self->fi_func);
 Dee_Visit(self->fi_iter);
}
PRIVATE int DCALL
filteriterator_ctor(FilterIterator *__restrict self) {
 self->fi_iter = DeeObject_IterSelf(Dee_EmptySeq);
 if unlikely(!self->fi_iter) return -1;
 self->fi_func = Dee_None;
 Dee_Incref(Dee_None);
 return 0;
}
PRIVATE int DCALL
filteriterator_init(FilterIterator *__restrict self,
                    size_t argc, DeeObject **__restrict argv) {
 Filter *filter;
 if (DeeArg_Unpack(argc,argv,"o:_filter.iterator",&filter) ||
     DeeObject_AssertTypeExact((DeeObject *)filter,&DeeFilter_Type))
     return -1;
 self->fi_iter = DeeObject_IterSelf(filter->f_seq);
 if unlikely(!self->fi_iter) return -1;
 self->fi_func = filter->f_fun;
 Dee_Incref(filter->f_fun);
 return 0;
}
PRIVATE int DCALL
filteriterator_copy(FilterIterator *__restrict self,
                    FilterIterator *__restrict other) {
 self->fi_iter = DeeObject_Copy(other->fi_iter);
 if unlikely(!self->fi_iter) return -1;
 self->fi_func = other->fi_func;
 Dee_Incref(self->fi_func);
 return 0;
}


STATIC_ASSERT(COMPILER_OFFSETOF(FilterIterator,fi_iter) == COMPILER_OFFSETOF(Filter,f_seq));
STATIC_ASSERT(COMPILER_OFFSETOF(FilterIterator,fi_func) == COMPILER_OFFSETOF(Filter,f_fun));
#define filteriterator_fini  filter_fini
#define filteriterator_visit filter_visit

PRIVATE DREF DeeObject *DCALL
filteriterator_next(FilterIterator *__restrict self) {
 DREF DeeObject *result;
 DREF DeeObject *pred_result;
 int pred_bool;
again:
 result = DeeObject_IterNext(self->fi_iter);
 if unlikely(!ITER_ISOK(result)) goto done;
 /* Invoke the predicate for the discovered element. */
 pred_result = DeeObject_Call(self->fi_func,1,&result);
 if unlikely(!pred_result) goto err_r;
 /* Cast the filter's return value to a boolean. */
 pred_bool = DeeObject_Bool(pred_result);
 Dee_Decref(pred_result);
 if unlikely(pred_bool < 0) goto err_r;
 if (!pred_bool) {
  Dee_Decref(result);
  goto again;
 }
done:
 return result;
err_r:
 Dee_Decref(result);
 return NULL;
}


PRIVATE DREF Filter *DCALL
filteriterator_seq_get(FilterIterator *__restrict self) {
 DREF Filter *result;
 DREF DeeObject *base_seq;
 base_seq = DeeObject_GetAttr(self->fi_iter,&str_seq);
 if unlikely(!base_seq) goto err;
 result = DeeObject_MALLOC(Filter);
 if unlikely(!result) { Dee_Decref(base_seq); goto err; }
 result->f_seq = base_seq; /* Inherit reference. */
 result->f_fun = self->fi_func;
 Dee_Incref(result->f_fun);
 DeeObject_Init(result,&DeeFilter_Type);
 return result;
err:
 return NULL;
}


PRIVATE struct type_getset filteriterator_getsets[] = {
    { "seq", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&filteriterator_seq_get, NULL, NULL,
      DOC("->_filter") },
    { NULL }
};


#define DEFINE_FILTERITERATOR_COMPARE(name,compare_object) \
PRIVATE DREF DeeObject *DCALL \
name(FilterIterator *__restrict self, \
     FilterIterator *__restrict other) { \
 if (DeeObject_AssertTypeExact((DeeObject *)other,&DeeFilterIterator_Type)) \
     return NULL; \
 return compare_object(self->fi_iter,other->fi_iter); \
}
DEFINE_FILTERITERATOR_COMPARE(filteriterator_eq,DeeObject_CompareEqObject)
DEFINE_FILTERITERATOR_COMPARE(filteriterator_ne,DeeObject_CompareNeObject)
DEFINE_FILTERITERATOR_COMPARE(filteriterator_lo,DeeObject_CompareLoObject)
DEFINE_FILTERITERATOR_COMPARE(filteriterator_le,DeeObject_CompareLeObject)
DEFINE_FILTERITERATOR_COMPARE(filteriterator_gr,DeeObject_CompareGrObject)
DEFINE_FILTERITERATOR_COMPARE(filteriterator_ge,DeeObject_CompareGeObject)
#undef DEFINE_FILTERITERATOR_COMPARE

PRIVATE struct type_cmp filteriterator_cmp = {
    /* .tp_hash = */NULL,
    /* .tp_eq   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict self, DeeObject *__restrict some_object))&filteriterator_eq,
    /* .tp_ne   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict self, DeeObject *__restrict some_object))&filteriterator_ne,
    /* .tp_lo   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict self, DeeObject *__restrict some_object))&filteriterator_lo,
    /* .tp_le   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict self, DeeObject *__restrict some_object))&filteriterator_le,
    /* .tp_gr   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict self, DeeObject *__restrict some_object))&filteriterator_gr,
    /* .tp_ge   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict self, DeeObject *__restrict some_object))&filteriterator_ge,
};



INTERN DeeTypeObject DeeFilterIterator_Type = {
    OBJECT_HEAD_INIT(&DeeType_Type),
    /* .tp_name     = */"_filter.iterator",
    /* .tp_doc      = */NULL,
    /* .tp_flags    = */TP_FNORMAL|TP_FFINAL,
    /* .tp_weakrefs = */0,
    /* .tp_features = */TF_NONE,
    /* .tp_base     = */&DeeIterator_Type,
    /* .tp_init = */{
        {
            /* .tp_alloc = */{
                /* .tp_ctor      = */&filteriterator_ctor,
                /* .tp_copy_ctor = */&filteriterator_copy,
                /* .tp_deep_ctor = */NULL,
                /* .tp_any_ctor  = */&filteriterator_init,
                /* .tp_free      = */NULL,
                {
                    /* .tp_instance_size = */sizeof(FilterIterator)
                }
            }
        },
        /* .tp_dtor        = */(void(DCALL *)(DeeObject *__restrict))&filteriterator_fini,
        /* .tp_assign      = */NULL,
        /* .tp_move_assign = */NULL
    },
    /* .tp_cast = */{
        /* .tp_str  = */NULL,
        /* .tp_repr = */NULL,
        /* .tp_bool = */NULL
    },
    /* .tp_call          = */NULL,
    /* .tp_visit         = */(void(DCALL *)(DeeObject *__restrict,dvisit_t,void*))&filteriterator_visit,
    /* .tp_gc            = */NULL,
    /* .tp_math          = */NULL,
    /* .tp_cmp           = */&filteriterator_cmp,
    /* .tp_seq           = */NULL,
    /* .tp_iter_next     = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&filteriterator_next,
    /* .tp_attr          = */NULL,
    /* .tp_with          = */NULL,
    /* .tp_buffer        = */NULL,
    /* .tp_methods       = */NULL,
    /* .tp_getsets       = */filteriterator_getsets,
    /* .tp_members       = */NULL,
    /* .tp_class_methods = */NULL,
    /* .tp_class_getsets = */NULL,
    /* .tp_class_members = */NULL
};


PRIVATE DREF FilterIterator *DCALL
filter_iter(Filter *__restrict self) {
 DREF FilterIterator *result;
 result = DeeObject_MALLOC(FilterIterator);
 if unlikely(!result) goto done;
 result->fi_iter = DeeObject_IterSelf(self->f_seq);
 if unlikely(!result->fi_iter) {
  DeeObject_Free(result);
  return NULL;
 }
 result->fi_func = self->f_fun;
 Dee_Incref(result->fi_func);
 DeeObject_Init(result,&DeeFilterIterator_Type);
done:
 return result;
}

PRIVATE struct type_seq filter_seq = {
    /* .tp_iter_self = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&filter_iter,
    /* .tp_size      = */NULL,
    /* .tp_contains  = */NULL,
    /* .tp_get       = */NULL,
    /* .tp_del       = */NULL,
    /* .tp_set       = */NULL,
    /* .tp_range_get = */NULL,
    /* .tp_range_del = */NULL,
    /* .tp_range_set = */NULL
};

PRIVATE struct type_member filter_members[] = {
    TYPE_MEMBER_FIELD("__seq__",STRUCT_OBJECT,offsetof(Filter,f_seq)),
    TYPE_MEMBER_FIELD("__filter__",STRUCT_OBJECT,offsetof(Filter,f_fun)),
    TYPE_MEMBER_END
};

PRIVATE struct type_member filter_class_members[] = {
    TYPE_MEMBER_CONST("iterator",&DeeFilterIterator_Type),
    TYPE_MEMBER_END
};

PRIVATE int DCALL
filter_ctor(Filter *__restrict self) {
 self->f_seq = Dee_EmptySeq;
 self->f_fun = Dee_None;
 Dee_Incref(Dee_EmptySeq);
 Dee_Incref(Dee_None);
 return 0;
}

PRIVATE int DCALL
filter_init(Filter *__restrict self,
            size_t argc, DeeObject **__restrict argv) {
 if (DeeArg_Unpack(argc,argv,"oo:_filter",&self->f_seq,&self->f_fun))
     return -1;
 self->f_seq = Dee_EmptySeq;
 self->f_fun = Dee_None;
 Dee_Incref(self->f_seq);
 Dee_Incref(self->f_fun);
 return 0;
}


INTERN DeeTypeObject DeeFilter_Type = {
    OBJECT_HEAD_INIT(&DeeType_Type),
    /* .tp_name     = */"_filter",
    /* .tp_doc      = */NULL,
    /* .tp_flags    = */TP_FNORMAL|TP_FFINAL,
    /* .tp_weakrefs = */0,
    /* .tp_features = */TF_NONE,
    /* .tp_base     = */&DeeSeq_Type,
    /* .tp_init = */{
        {
            /* .tp_alloc = */{
                /* .tp_ctor      = */&filter_ctor,
                /* .tp_copy_ctor = */NULL,
                /* .tp_deep_ctor = */NULL,
                /* .tp_any_ctor  = */&filter_init,
                /* .tp_free      = */NULL,
                {
                    /* .tp_instance_size = */sizeof(Filter)
                }
            }
        },
        /* .tp_dtor        = */(void(DCALL *)(DeeObject *__restrict))&filter_fini,
        /* .tp_assign      = */NULL,
        /* .tp_move_assign = */NULL
    },
    /* .tp_cast = */{
        /* .tp_str  = */NULL,
        /* .tp_repr = */NULL,
        /* .tp_bool = */NULL
    },
    /* .tp_call          = */NULL,
    /* .tp_visit         = */(void(DCALL *)(DeeObject *__restrict,dvisit_t,void*))&filter_visit,
    /* .tp_gc            = */NULL,
    /* .tp_math          = */NULL,
    /* .tp_cmp           = */NULL,
    /* .tp_seq           = */&filter_seq,
    /* .tp_iter_next     = */NULL,
    /* .tp_attr          = */NULL,
    /* .tp_with          = */NULL,
    /* .tp_buffer        = */NULL,
    /* .tp_methods       = */NULL,
    /* .tp_getsets       = */NULL,
    /* .tp_members       = */filter_members,
    /* .tp_class_methods = */NULL,
    /* .tp_class_getsets = */NULL,
    /* .tp_class_members = */filter_class_members
};

INTERN DREF DeeObject *DCALL
DeeSeq_Filter(DeeObject *__restrict self,
              DeeObject *__restrict pred_keep) {
 DREF Filter *result;
 result = DeeObject_MALLOC(DREF Filter);
 if unlikely(!result) goto done;
 Dee_Incref(self);
 Dee_Incref(pred_keep);
 result->f_seq = self;
 result->f_fun = pred_keep;
 DeeObject_Init(result,&DeeFilter_Type);
done:
 return (DREF DeeObject *)result;
}


DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_FILTER_C */
