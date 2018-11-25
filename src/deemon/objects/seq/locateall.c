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
#ifndef GUARD_DEEMON_OBJECTS_SEQ_LOCATE_ALL_C
#define GUARD_DEEMON_OBJECTS_SEQ_LOCATE_ALL_C 1

#include <deemon/api.h>
#include <deemon/object.h>
#include <deemon/seq.h>
#include <deemon/none.h>
#include <deemon/error.h>
#include <deemon/bool.h>
#include <deemon/thread.h>
#include <deemon/arg.h>
#include <deemon/string.h>
#include <hybrid/minmax.h>

#ifndef CONFIG_NO_THREADS
#include <hybrid/atomic.h>
#endif /* !CONFIG_NO_THREADS */

#include "../../runtime/runtime_error.h"
#include "../../runtime/strings.h"

DECL_BEGIN

typedef struct {
    OBJECT_HEAD
    DREF DeeObject *l_seq;  /* [1..1][const] The sequence being transformed. */
    DREF DeeObject *l_elem; /* [1..1][const] The element being searched. */
    DREF DeeObject *l_pred; /* [0..1][const] The key function invoked to transform elements. */
} Locator;


typedef struct {
    OBJECT_HEAD
    DREF DeeObject *li_iter; /* [1..1][const] The iterator in which items are being located. */
    DREF DeeObject *li_elem; /* [1..1][const] The element being searched. */
    DREF DeeObject *li_pred; /* [0..1][const] The key function invoked to transform elements. */
} LocatorIterator;

INTDEF DeeTypeObject DeeLocator_Type;
INTDEF DeeTypeObject DeeLocatorIterator_Type;


PRIVATE int DCALL
locatoriter_ctor(LocatorIterator *__restrict self) {
 self->li_iter = DeeObject_IterSelf(Dee_EmptySeq);
 if unlikely(!self->li_iter) return -1;
 self->li_elem = Dee_None;
 self->li_pred = NULL;
 Dee_Incref(Dee_None);
 return 0;
}
PRIVATE int DCALL
locatoriter_copy(LocatorIterator *__restrict self,
                 LocatorIterator *__restrict other) {
 self->li_iter = DeeObject_Copy(other->li_iter);
 if unlikely(!self->li_iter) return -1;
 self->li_elem = other->li_elem;
 self->li_pred = other->li_pred;
 Dee_Incref(self->li_elem);
 Dee_XIncref(self->li_pred);
 return 0;
}
PRIVATE int DCALL
locatoriter_init(LocatorIterator *__restrict self,
                 size_t argc, DeeObject **__restrict argv) {
 Locator *loc;
 if (DeeArg_Unpack(argc,argv,"o:_locator.iterator",&loc) ||
     DeeObject_AssertTypeExact((DeeObject *)loc,&DeeLocator_Type))
     return -1;
 self->li_iter = DeeObject_IterSelf(loc->l_seq);
 if unlikely(!self->li_iter) return -1;
 self->li_elem = loc->l_elem;
 self->li_pred = loc->l_pred;
 Dee_Incref(self->li_elem);
 Dee_XIncref(self->li_pred);
 return 0;
}
PRIVATE void DCALL
locatoriter_fini(LocatorIterator *__restrict self) {
 Dee_Decref(self->li_iter);
 Dee_Decref(self->li_elem);
 Dee_XDecref(self->li_pred);
}
PRIVATE void DCALL
locatoriter_visit(LocatorIterator *__restrict self, dvisit_t proc, void *arg) {
 Dee_Visit(self->li_iter);
 Dee_Visit(self->li_elem);
 Dee_XVisit(self->li_pred);
}

PRIVATE DREF DeeObject *DCALL
locatoriter_next(LocatorIterator *__restrict self) {
 DREF DeeObject *result;
 for (;;) {
  int temp;
  result = DeeObject_IterNext(self->li_iter);
  if (!ITER_ISOK(result)) break;
  temp = DeeObject_CompareKeyEq(self->li_elem,result,self->li_pred);
  if (temp != 0) {
   if unlikely(temp < 0) goto err_r;
   break; /* Found it */
  }
  Dee_Decref(result);
  if (DeeThread_CheckInterrupt())
      goto err;
 }
 /* Return the located element. */
 return result;
err_r:
 Dee_Decref(result);
err:
 return NULL;
}


#define DEFINE_FILTERITERATOR_COMPARE(name,compare_object) \
PRIVATE DREF DeeObject *DCALL \
name(LocatorIterator *__restrict self, \
     LocatorIterator *__restrict other) { \
 if (DeeObject_AssertTypeExact((DeeObject *)other,&DeeLocatorIterator_Type)) \
     return NULL; \
 return compare_object(self->li_iter,other->li_iter); \
}
DEFINE_FILTERITERATOR_COMPARE(locatoriter_eq,DeeObject_CompareEqObject)
DEFINE_FILTERITERATOR_COMPARE(locatoriter_ne,DeeObject_CompareNeObject)
DEFINE_FILTERITERATOR_COMPARE(locatoriter_lo,DeeObject_CompareLoObject)
DEFINE_FILTERITERATOR_COMPARE(locatoriter_le,DeeObject_CompareLeObject)
DEFINE_FILTERITERATOR_COMPARE(locatoriter_gr,DeeObject_CompareGrObject)
DEFINE_FILTERITERATOR_COMPARE(locatoriter_ge,DeeObject_CompareGeObject)
#undef DEFINE_FILTERITERATOR_COMPARE

PRIVATE struct type_cmp locatoriter_cmp = {
    /* .tp_hash = */NULL,
    /* .tp_eq   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict self, DeeObject *__restrict some_object))&locatoriter_eq,
    /* .tp_ne   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict self, DeeObject *__restrict some_object))&locatoriter_ne,
    /* .tp_lo   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict self, DeeObject *__restrict some_object))&locatoriter_lo,
    /* .tp_le   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict self, DeeObject *__restrict some_object))&locatoriter_le,
    /* .tp_gr   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict self, DeeObject *__restrict some_object))&locatoriter_gr,
    /* .tp_ge   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict self, DeeObject *__restrict some_object))&locatoriter_ge,
};



PRIVATE DREF DeeObject *DCALL
locatoriter_seq_get(LocatorIterator *__restrict self) {
 DREF DeeObject *inner_seq,*result;
 /* Forward access to this attribute to the pointed-to iterator. */
 inner_seq = DeeObject_GetAttr(self->li_iter,&str_seq);
 if unlikely(!inner_seq) return NULL;
 result = DeeSeq_LocateAll(inner_seq,self->li_elem,self->li_pred);
 Dee_Decref(inner_seq);
 return result;
}

PRIVATE struct type_getset locatoriter_getsets[] = {
    { DeeString_STR(&str_seq), (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&locatoriter_seq_get, NULL, NULL },
    { NULL }
};

PRIVATE struct type_member locatoriter_members[] = {
    TYPE_MEMBER_FIELD("__iterator__",STRUCT_OBJECT,offsetof(LocatorIterator,li_iter)),
    TYPE_MEMBER_FIELD("__elem__",STRUCT_OBJECT,offsetof(LocatorIterator,li_elem)),
    TYPE_MEMBER_FIELD("__pred__",STRUCT_OBJECT_OPT,offsetof(LocatorIterator,li_pred)),
    TYPE_MEMBER_END
};

INTERN DeeTypeObject DeeLocatorIterator_Type = {
    OBJECT_HEAD_INIT(&DeeType_Type),
    /* .tp_name     = */"_locator.iterator",
    /* .tp_doc      = */NULL,
    /* .tp_flags    = */TP_FNORMAL|TP_FFINAL,
    /* .tp_weakrefs = */0,
    /* .tp_features = */TF_NONE,
    /* .tp_base     = */&DeeIterator_Type,
    /* .tp_init = */{
        {
            /* .tp_alloc = */{
                /* .tp_ctor      = */&locatoriter_ctor,
                /* .tp_copy_ctor = */&locatoriter_copy,
                /* .tp_deep_ctor = */NULL,
                /* .tp_any_ctor  = */&locatoriter_init,
                TYPE_FIXED_ALLOCATOR(LocatorIterator)
            }
        },
        /* .tp_dtor        = */(void(DCALL *)(DeeObject *__restrict))&locatoriter_fini,
        /* .tp_assign      = */NULL,
        /* .tp_move_assign = */NULL
    },
    /* .tp_cast = */{
        /* .tp_str  = */NULL,
        /* .tp_repr = */NULL,
        /* .tp_bool = */NULL
    },
    /* .tp_call          = */NULL,
    /* .tp_visit         = */(void(DCALL *)(DeeObject *__restrict,dvisit_t,void*))&locatoriter_visit,
    /* .tp_gc            = */NULL,
    /* .tp_math          = */NULL,
    /* .tp_cmp           = */&locatoriter_cmp,
    /* .tp_seq           = */NULL,
    /* .tp_iter_next     = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&locatoriter_next,
    /* .tp_attr          = */NULL,
    /* .tp_with          = */NULL,
    /* .tp_buffer        = */NULL,
    /* .tp_methods       = */NULL,
    /* .tp_getsets       = */locatoriter_getsets,
    /* .tp_members       = */locatoriter_members,
    /* .tp_class_methods = */NULL,
    /* .tp_class_getsets = */NULL,
    /* .tp_class_members = */NULL
};

PRIVATE int DCALL
locator_ctor(Locator *__restrict self) {
 self->l_seq  = Dee_EmptySeq;
 self->l_elem = Dee_None;
 Dee_Incref(Dee_EmptySeq);
 Dee_Incref(Dee_None);
 return 0;
}
PRIVATE int DCALL
locator_init(Locator *__restrict self,
             size_t argc, DeeObject **__restrict argv) {
 self->l_pred = NULL;
 if (DeeArg_Unpack(argc,argv,"oo|o:_locator",&self->l_seq,&self->l_elem,&self->l_pred))
     goto err;
 if (self->l_pred) {
  self->l_elem = DeeObject_Call(self->l_pred,1,&self->l_elem);
  if unlikely(!self->l_elem) goto err;
  Dee_Incref(self->l_pred);
 } else {
  Dee_Incref(self->l_elem);
 }
 Dee_Incref(self->l_seq);
 return 0;
err:
 return -1;
}
PRIVATE void DCALL
locator_fini(Locator *__restrict self) {
 Dee_Decref(self->l_seq);
 Dee_Decref(self->l_elem);
 Dee_XDecref(self->l_pred);
}
PRIVATE void DCALL
locator_visit(Locator *__restrict self, dvisit_t proc, void *arg) {
 Dee_Visit(self->l_seq);
 Dee_Visit(self->l_elem);
 Dee_XVisit(self->l_pred);
}

PRIVATE DREF DeeObject *DCALL
locator_iter(Locator *__restrict self) {
 DREF LocatorIterator *result;
 result = DeeObject_FMALLOC(LocatorIterator);
 if unlikely(!result) goto done;
 /* Create the underlying iterator. */
 result->li_iter = DeeObject_IterSelf(self->l_seq);
 if unlikely(!result->li_iter) goto err_r;
 /* Assign the locator element & key function. */
 result->li_elem = self->l_elem;
 Dee_Incref(self->l_elem);
 result->li_pred = self->l_pred;
 Dee_XIncref(self->l_pred);
 DeeObject_Init(result,&DeeLocatorIterator_Type);
done:
 return (DREF DeeObject *)result;
err_r:
 DeeObject_FFREE(result);
 return NULL;
}

PRIVATE struct type_member locator_members[] = {
   TYPE_MEMBER_FIELD("__seq__",STRUCT_OBJECT,offsetof(Locator,l_seq)),
   TYPE_MEMBER_FIELD("__elem__",STRUCT_OBJECT,offsetof(Locator,l_elem)),
   TYPE_MEMBER_FIELD("__pred__",STRUCT_OBJECT_OPT,offsetof(Locator,l_pred)),
   TYPE_MEMBER_END
};

PRIVATE struct type_member locator_class_members[] = {
   TYPE_MEMBER_CONST("iterator",&DeeLocatorIterator_Type),
   TYPE_MEMBER_END
};

PRIVATE struct type_seq locator_seq = {
    /* .tp_iter_self = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&locator_iter,
    /* .tp_size      = */NULL,
    /* .tp_contains  = */NULL,
    /* .tp_get       = */NULL,
    /* .tp_del       = */NULL,
    /* .tp_set       = */NULL,
    /* .tp_range_get = */NULL,
    /* .tp_range_del = */NULL,
    /* .tp_range_set = */NULL
};

INTERN DeeTypeObject DeeLocator_Type = {
    OBJECT_HEAD_INIT(&DeeType_Type),
    /* .tp_name     = */"_locator",
    /* .tp_doc      = */NULL,
    /* .tp_flags    = */TP_FNORMAL|TP_FFINAL,
    /* .tp_weakrefs = */0,
    /* .tp_features = */TF_NONE,
    /* .tp_base     = */&DeeSeq_Type,
    /* .tp_init = */{
        {
            /* .tp_alloc = */{
                /* .tp_ctor      = */&locator_ctor,
                /* .tp_copy_ctor = */NULL,
                /* .tp_deep_ctor = */NULL,
                /* .tp_any_ctor  = */&locator_init,
                TYPE_FIXED_ALLOCATOR(Locator)
            }
        },
        /* .tp_dtor        = */(void(DCALL *)(DeeObject *__restrict))&locator_fini,
        /* .tp_assign      = */NULL,
        /* .tp_move_assign = */NULL
    },
    /* .tp_cast = */{
        /* .tp_str  = */NULL,
        /* .tp_repr = */NULL,
        /* .tp_bool = */NULL
    },
    /* .tp_call          = */NULL,
    /* .tp_visit         = */(void(DCALL *)(DeeObject *__restrict,dvisit_t,void*))&locator_visit,
    /* .tp_gc            = */NULL,
    /* .tp_math          = */NULL,
    /* .tp_cmp           = */NULL,
    /* .tp_seq           = */&locator_seq,
    /* .tp_iter_next     = */NULL,
    /* .tp_attr          = */NULL,
    /* .tp_with          = */NULL,
    /* .tp_buffer        = */NULL,
    /* .tp_methods       = */NULL,
    /* .tp_getsets       = */NULL,
    /* .tp_members       = */locator_members,
    /* .tp_class_methods = */NULL,
    /* .tp_class_getsets = */NULL,
    /* .tp_class_members = */locator_class_members
};



INTERN DREF DeeObject *DCALL
DeeSeq_LocateAll(DeeObject *__restrict self,
                 DeeObject *__restrict keyed_search_item,
                 DeeObject *key) {
 DREF Locator *result;
 /* Create a new locator sequence. */
 result = DeeObject_FMALLOC(Locator);
 if unlikely(!result) return NULL;
 result->l_seq  = self;
 result->l_elem = keyed_search_item;
 result->l_pred = key;
 Dee_Incref(self);
 Dee_Incref(keyed_search_item);
 Dee_XIncref(key);
 DeeObject_Init(result,&DeeLocator_Type);
 return (DREF DeeObject *)result;
}

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_LOCATE_ALL_C */
