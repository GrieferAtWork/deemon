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
#ifndef GUARD_DEEMON_OBJECTS_CLASS_DESC_C
#define GUARD_DEEMON_OBJECTS_CLASS_DESC_C 1
#define _KOS_SOURCE 1

#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/class.h>
#include <deemon/module.h>
#include <deemon/seq.h>
#include <deemon/super.h>
#include <deemon/int.h>
#include <deemon/map.h>
#include <deemon/none.h>
#include <deemon/tuple.h>
#include <deemon/bool.h>
#include <deemon/string.h>
#include <deemon/mro.h>
#include <deemon/thread.h>
#include <deemon/code.h>
#include <deemon/error.h>
#include <deemon/instancemethod.h>
#include <deemon/property.h>
#include <deemon/attribute.h>


#include "../runtime/runtime_error.h"

DECL_BEGIN

typedef DeeClassDescriptorObject ClassDescriptor;

INTERN struct class_operator empty_class_operators[] = {
    {
        /* .co_name = */(uint16_t)-1,
        /* .co_addr = */0
    }
};

INTERN struct class_attribute empty_class_attributes[] = {
    {
        /* .ca_name = */NULL,
        /* .ca_hash = */0,
        /* .ca_doc  = */NULL,
        /* .ca_addr = */0,
        /* .ca_flag = */CLASS_ATTRIBUTE_FNORMAL
    }
};

typedef struct {
    /* A mapping-like {(string | int,int)...} object used for mapping
     * operator names to their respective class instance table slots. */
    OBJECT_HEAD
    DREF ClassDescriptor *co_desc; /* [1..1][const] The referenced class descriptor. */
} ClassOperatorTable;

typedef struct {
    /* A mapping-like {(string | int,int)...} object used for mapping
     * operator names to their respective class instance table slots. */
    OBJECT_HEAD
    DREF ClassDescriptor              *co_desc; /* [1..1][const] The referenced class descriptor. */
    ATOMIC_DATA struct class_operator *co_iter; /* [1..1] Current iterator position. */
    struct class_operator             *co_end;  /* [1..1][const] Iterator end position. */
} ClassOperatorTableIterator;

INTDEF DeeTypeObject ClassOperatorTableIterator_Type;
INTDEF DeeTypeObject ClassOperatorTable_Type;

#ifdef CONFIG_NO_THREADS
#define COTI_GETITER(x) (x)->co_iter
#else
#define COTI_GETITER(x) ATOMIC_READ((x)->co_iter)
#endif

LOCAL struct class_operator *DCALL
coti_next_ent(ClassOperatorTableIterator *__restrict self) {
#ifdef CONFIG_NO_THREADS
 struct class_operator *result;
 result = self->co_iter;
 for (;; ++result) {
  if (result >= self->co_end)
      return NULL;
  if (result->co_name != (uint16_t)-1)
      break;
 }
 self->co_iter = result + 1;
#else
 struct class_operator *result,*start;
 for (;;) {
  result = start = ATOMIC_READ(self->co_iter);
  for (;; ++result) {
   if (result >= self->co_end)
       return NULL;
   if (result->co_name != (uint16_t)-1)
       break;
  }
  if (ATOMIC_CMPXCH_WEAK(self->co_iter,start,result + 1))
      break;
 }
#endif
 return result;
}

PRIVATE DREF DeeObject *DCALL
coti_next(ClassOperatorTableIterator *__restrict self) {
 struct class_operator *ent;
 struct opinfo *info;
 ent = coti_next_ent(self);
 if (!ent) return ITER_DONE;
 info = Dee_OperatorInfo(NULL,ent->co_name);
 if (info) return DeeTuple_Newf("sI16u",info->oi_sname,ent->co_addr);
 return DeeTuple_Newf("I16uI16u",ent->co_name,ent->co_addr);
}
PRIVATE DREF DeeObject *DCALL
coti_next_key(ClassOperatorTableIterator *__restrict self) {
 struct class_operator *ent;
 struct opinfo *info;
 ent = coti_next_ent(self);
 if (!ent) return ITER_DONE;
 info = Dee_OperatorInfo(NULL,ent->co_name);
 if (info) return DeeString_New(info->oi_sname);
 return DeeInt_NewU16(ent->co_name);
}
PRIVATE DREF DeeObject *DCALL
coti_next_value(ClassOperatorTableIterator *__restrict self) {
 struct class_operator *ent;
 ent = coti_next_ent(self);
 if (!ent) return ITER_DONE;
 return DeeInt_NewU16(ent->co_addr);
}

PRIVATE void DCALL
coti_fini(ClassOperatorTableIterator *__restrict self) {
 Dee_Decref(self->co_desc);
}

PRIVATE int DCALL
coti_copy(ClassOperatorTableIterator *__restrict self,
          ClassOperatorTableIterator *__restrict other) {
 self->co_desc = other->co_desc;
 self->co_iter = COTI_GETITER(other);
 self->co_end = other->co_end;
 Dee_Incref(self->co_desc);
 return 0;
}

PRIVATE DREF ClassOperatorTable *DCALL
coti_getseq(ClassOperatorTableIterator *__restrict self) {
 DREF ClassOperatorTable *result;
 result = DeeObject_MALLOC(ClassOperatorTable);
 if unlikely(!result) goto done;
 result->co_desc = self->co_desc;
 Dee_Incref(self->co_desc);
 DeeObject_Init(result,&ClassOperatorTable_Type);
done:
 return result;
}

#define DEFINE_COTI_COMPARE(name,op) \
PRIVATE DREF DeeObject *DCALL \
name(ClassOperatorTableIterator *__restrict self, \
     ClassOperatorTableIterator *__restrict other) { \
 if (DeeObject_AssertTypeExact((DeeObject *)other,&ClassOperatorTableIterator_Type)) \
     goto err; \
 return_bool(COTI_GETITER(self) op COTI_GETITER(other)); \
err: \
 return NULL; \
} \
/**/
DEFINE_COTI_COMPARE(coti_eq,==)
DEFINE_COTI_COMPARE(coti_ne,!=)
DEFINE_COTI_COMPARE(coti_lo,<)
DEFINE_COTI_COMPARE(coti_le,<=)
DEFINE_COTI_COMPARE(coti_gr,>)
DEFINE_COTI_COMPARE(coti_ge,>=)
#undef DEFINE_COTI_COMPARE

PRIVATE struct type_cmp coti_cmp = {
    /* .tp_hash = */NULL,
    /* .tp_eq   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&coti_eq,
    /* .tp_ne   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&coti_ne,
    /* .tp_lo   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&coti_lo,
    /* .tp_le   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&coti_le,
    /* .tp_gr   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&coti_gr,
    /* .tp_ge   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&coti_ge,
};

PRIVATE struct type_getset coti_getsets[] = {
    { "seq", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&coti_getseq, NULL, NULL,
      DOC("->?Aoperators?Ert:classdescriptor") },
    { NULL }
};

PRIVATE struct type_member coti_members[] = {
    TYPE_MEMBER_FIELD_DOC("__class__",STRUCT_OBJECT,offsetof(ClassOperatorTableIterator,co_desc),"->?Ert:classdescriptor"),
    TYPE_MEMBER_END
};

INTERN DeeTypeObject ClassOperatorTableIterator_Type = {
    OBJECT_HEAD_INIT(&DeeType_Type),
    /* .tp_name     = */"_classoperatortableiterator",
    /* .tp_doc      = */NULL,
    /* .tp_flags    = */TP_FFINAL,
    /* .tp_weakrefs = */0,
    /* .tp_features = */TF_NONE,
    /* .tp_base     = */&DeeIterator_Type,
    /* .tp_init = */{
        {
            /* .tp_alloc = */{
                /* .tp_ctor      = */NULL,
                /* .tp_copy_ctor = */(void *)&coti_copy,
                /* .tp_deep_ctor = */NULL,
                /* .tp_any_ctor  = */NULL,
                /* .tp_free      = */NULL,
                {
                    /* .tp_instance_size = */sizeof(ClassOperatorTableIterator)
                }
            }
        },
        /* .tp_dtor        = */(void(DCALL *)(DeeObject *__restrict))&coti_fini,
        /* .tp_assign      = */NULL,
        /* .tp_move_assign = */NULL
    },
    /* .tp_cast = */{
        /* .tp_str  = */NULL,
        /* .tp_repr = */NULL,
        /* .tp_bool = */NULL
    },
    /* .tp_call          = */NULL,
    /* .tp_visit         = */NULL, /* No visit, because only ClassDescriptor is reachable, which doesn't have visit itself */
    /* .tp_gc            = */NULL,
    /* .tp_math          = */NULL,
    /* .tp_cmp           = */&coti_cmp,
    /* .tp_seq           = */NULL,
    /* .tp_iter_next     = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&coti_next,
    /* .tp_attr          = */NULL,
    /* .tp_with          = */NULL,
    /* .tp_buffer        = */NULL,
    /* .tp_methods       = */NULL,
    /* .tp_getsets       = */coti_getsets,
    /* .tp_members       = */coti_members,
    /* .tp_class_methods = */NULL,
    /* .tp_class_getsets = */NULL,
    /* .tp_class_members = */NULL
};


STATIC_ASSERT(COMPILER_OFFSETOF(ClassOperatorTable,co_desc) ==
              COMPILER_OFFSETOF(ClassOperatorTableIterator,co_desc));
#define cot_fini    coti_fini
#define cot_members coti_members

PRIVATE int DCALL
cot_bool(ClassOperatorTable *__restrict self) {
 return self->co_desc->cd_clsop_list != empty_class_operators;
}

PRIVATE DREF ClassOperatorTableIterator *DCALL
cot_iter(ClassOperatorTable *__restrict self) {
 DREF ClassOperatorTableIterator *result;
 result = DeeObject_MALLOC(ClassOperatorTableIterator);
 if unlikely(!result) goto done;
 result->co_desc = self->co_desc;
 result->co_iter = self->co_desc->cd_clsop_list;
 result->co_end  = (self->co_desc->cd_clsop_list +
                    self->co_desc->cd_clsop_mask + 1);
 Dee_Incref(self->co_desc);
 DeeObject_Init(result,&ClassOperatorTableIterator_Type);
done:
 return result;
}

PRIVATE size_t DCALL
cot_nsi_getsize(ClassOperatorTable *__restrict self) {
 uint16_t i; size_t result = 0;
 ClassDescriptor *desc = self->co_desc;
 for (i = 0; i <= desc->cd_clsop_mask; ++i) {
  if (desc->cd_clsop_list[i].co_name != (uint16_t)-1)
      ++result;
 }
 return result;
}

PRIVATE DREF DeeObject *DCALL
cot_size(ClassOperatorTable *__restrict self) {
 return DeeInt_NewSize(cot_nsi_getsize(self));
}

PRIVATE DREF DeeObject *DCALL
cot_getitemdef(ClassOperatorTable *__restrict self,
               DeeObject *__restrict key,
               DeeObject *__restrict defl) {
 uint16_t opname,i,perturb;
 ClassDescriptor *desc = self->co_desc;
 if (DeeString_Check(key)) {
  opname = Dee_OperatorFromName(NULL,DeeString_STR(key));
  if (opname == (uint16_t)-1) goto nope;
 } else {
  if (DeeObject_AsUInt16(key,&opname))
      goto err;
 }
 i = perturb = opname & desc->cd_clsop_mask;
 for (;; DeeClassDescriptor_CLSOPNEXT(i,perturb)) {
  struct class_operator *op;
  op = &desc->cd_clsop_list[i & desc->cd_clsop_mask];
  if (op->co_name == (uint16_t)-1) break;
  if (op->co_name != opname) continue;
  return DeeInt_NewU16(op->co_addr);
 }
nope:
 if (defl != ITER_DONE)
     Dee_Incref(defl);
 return defl;
err:
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
cot_getitem(ClassOperatorTable *__restrict self,
            DeeObject *__restrict key) {
 DREF DeeObject *result;
 result = cot_getitemdef(self,key,ITER_DONE);
 if (result == ITER_DONE) {
  err_unknown_key((DeeObject *)self,key);
  result = NULL;
 }
 return result;
}

PRIVATE struct type_nsi cot_nsi = {
    /* .nsi_class   = */TYPE_SEQX_CLASS_MAP,
    /* .nsi_flags   = */TYPE_SEQX_FNORMAL,
    {
        /* .nsi_maplike = */{
                /* .nsi_getsize    = */(void *)&cot_nsi_getsize,
                /* .nsi_nextkey    = */(void *)&coti_next_key,
                /* .nsi_nextvalue  = */(void *)&coti_next_value,
                /* .nsi_getdefault = */(void *)&cot_getitemdef,
                /* .nsi_setdefault = */(void *)NULL,
                /* .nsi_updateold  = */(void *)NULL,
                /* .nsi_insertnew  = */(void *)NULL
        }
    }
};

PRIVATE struct type_seq cot_seq = {
    /* .tp_iter_self = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&cot_iter,
    /* .tp_size      = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&cot_size,
    /* .tp_contains  = */NULL,
    /* .tp_get       = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&cot_getitem,
    /* .tp_del       = */NULL,
    /* .tp_set       = */NULL,
    /* .tp_range_get = */NULL,
    /* .tp_range_del = */NULL,
    /* .tp_range_set = */NULL,
    /* .tp_nsi       = */&cot_nsi
};

PRIVATE struct type_member cot_class_members[] = {
    TYPE_MEMBER_CONST("iterator",&ClassOperatorTableIterator_Type),
    TYPE_MEMBER_END
};

INTERN DeeTypeObject ClassOperatorTable_Type = {
    OBJECT_HEAD_INIT(&DeeType_Type),
    /* .tp_name     = */"_classoperatortable",
    /* .tp_doc      = */NULL,
    /* .tp_flags    = */TP_FFINAL,
    /* .tp_weakrefs = */0,
    /* .tp_features = */TF_NONE,
    /* .tp_base     = */&DeeMapping_Type,
    /* .tp_init = */{
        {
            /* .tp_alloc = */{
                /* .tp_ctor      = */NULL,
                /* .tp_copy_ctor = */NULL,
                /* .tp_deep_ctor = */NULL,
                /* .tp_any_ctor  = */NULL,
                /* .tp_free      = */NULL,
                {
                    /* .tp_instance_size = */sizeof(ClassOperatorTable)
                }
            }
        },
        /* .tp_dtor        = */(void(DCALL *)(DeeObject *__restrict))&cot_fini,
        /* .tp_assign      = */NULL,
        /* .tp_move_assign = */NULL
    },
    /* .tp_cast = */{
        /* .tp_str  = */NULL,
        /* .tp_repr = */NULL,
        /* .tp_bool = */(int(DCALL *)(DeeObject *__restrict))&cot_bool
    },
    /* .tp_call          = */NULL,
    /* .tp_visit         = */NULL, /* No visit, because only ClassDescriptor is reachable, which doesn't have visit itself */
    /* .tp_gc            = */NULL,
    /* .tp_math          = */NULL,
    /* .tp_cmp           = */NULL,
    /* .tp_seq           = */&cot_seq,
    /* .tp_iter_next     = */NULL,
    /* .tp_attr          = */NULL,
    /* .tp_with          = */NULL,
    /* .tp_buffer        = */NULL,
    /* .tp_methods       = */NULL,
    /* .tp_getsets       = */NULL,
    /* .tp_members       = */cot_members,
    /* .tp_class_methods = */NULL,
    /* .tp_class_getsets = */NULL,
    /* .tp_class_members = */cot_class_members
};




typedef struct {
    OBJECT_HEAD
    DREF ClassDescriptor   *ca_desc; /* [1..1][const] Class descriptor. */
    struct class_attribute *ca_attr; /* [1..1][const] The attribute that was queried. */
} ClassAttribute;

typedef struct {
    OBJECT_HEAD
    DREF ClassDescriptor               *ca_desc; /* [1..1][const] Class descriptor. */
    ATOMIC_DATA struct class_attribute *ca_iter; /* [1..1] Current iterator position. */
    struct class_attribute             *ca_end;  /* [1..1][const] Iterator end. */
} ClassAttributeTableIterator;

typedef struct {
    OBJECT_HEAD
    DREF ClassDescriptor   *ca_desc;  /* [1..1][const] Class descriptor. */
    struct class_attribute *ca_start; /* [1..1][const] Hash-vector starting pointer. */
    size_t                  ca_mask;  /* [const] Mask-vector size mask. */
} ClassAttributeTable;

INTDEF DeeTypeObject ClassAttribute_Type;
INTDEF DeeTypeObject ClassAttributeTable_Type;
INTDEF DeeTypeObject ClassAttributeTableIterator_Type;

PRIVATE DREF ClassAttribute *DCALL
cattr_new(ClassDescriptor *__restrict desc,
          struct class_attribute *__restrict attr) {
 DREF ClassAttribute *result;
 result = DeeObject_MALLOC(DREF ClassAttribute);
 if unlikely(!result) goto done;
 result->ca_desc = desc;
 result->ca_attr = attr;
 Dee_Incref(desc);
 DeeObject_Init(result,&ClassAttribute_Type);
done:
 return result;
}

STATIC_ASSERT(COMPILER_OFFSETOF(ClassOperatorTable,co_desc) ==
              COMPILER_OFFSETOF(ClassAttribute,ca_desc));
#define ca_fini    cot_fini
#define ca_members cot_members
STATIC_ASSERT(COMPILER_OFFSETOF(ClassOperatorTable,co_desc) ==
              COMPILER_OFFSETOF(ClassAttributeTable,ca_desc));
#define cat_fini    cot_fini
#define cat_members cot_members
STATIC_ASSERT(COMPILER_OFFSETOF(ClassOperatorTable,co_desc) ==
              COMPILER_OFFSETOF(ClassAttributeTableIterator,ca_desc));
#define cati_fini    cot_fini
#define cati_members cot_members

STATIC_ASSERT(COMPILER_OFFSETOF(ClassOperatorTableIterator,co_desc) == COMPILER_OFFSETOF(ClassAttributeTableIterator,ca_desc));
STATIC_ASSERT(COMPILER_OFFSETOF(ClassOperatorTableIterator,co_iter) == COMPILER_OFFSETOF(ClassAttributeTableIterator,ca_iter));
STATIC_ASSERT(COMPILER_OFFSETOF(ClassOperatorTableIterator,co_end) == COMPILER_OFFSETOF(ClassAttributeTableIterator,ca_end));
#define cati_cmp  coti_cmp
#define cati_copy coti_copy

#ifdef CONFIG_NO_THREADS
#define CATI_GETITER(x) (x)->ca_iter
#else
#define CATI_GETITER(x) ATOMIC_READ((x)->ca_iter)
#endif

LOCAL struct class_attribute *DCALL
cati_next_ent(ClassAttributeTableIterator *__restrict self) {
#ifdef CONFIG_NO_THREADS
 struct class_attribute *result;
 result = self->ca_iter;
 for (;; ++result) {
  if (result >= self->ca_end)
      return NULL;
  if (result->co_name != NULL)
      break;
 }
 self->co_iter = result + 1;
#else
 struct class_attribute *result,*start;
 for (;;) {
  result = start = ATOMIC_READ(self->ca_iter);
  for (;; ++result) {
   if (result >= self->ca_end)
       return NULL;
   if (result->ca_name != NULL)
       break;
  }
  if (ATOMIC_CMPXCH_WEAK(self->ca_iter,start,result + 1))
      break;
 }
#endif
 return result;
}

PRIVATE DREF DeeObject *DCALL
cati_next(ClassAttributeTableIterator *__restrict self) {
 DREF DeeObject *result;
 DREF ClassAttribute *attr;
 struct class_attribute *ent;
 ent = cati_next_ent(self);
 if (!ent) return ITER_DONE;
 attr = cattr_new(self->ca_desc,ent);
 if unlikely(!attr) return NULL;
 result = DeeTuple_Pack(2,ent->ca_name,attr);
 Dee_Decref(attr);
 return result;
}
PRIVATE DREF DeeObject *DCALL
cati_next_key(ClassAttributeTableIterator *__restrict self) {
 struct class_attribute *ent;
 ent = cati_next_ent(self);
 if (!ent) return ITER_DONE;
 return_reference_((DeeObject *)ent->ca_name);
}
PRIVATE DREF ClassAttribute *DCALL
cati_next_value(ClassAttributeTableIterator *__restrict self) {
 struct class_attribute *ent;
 ent = cati_next_ent(self);
 if (!ent) return (DREF ClassAttribute *)ITER_DONE;
 return cattr_new(self->ca_desc,ent);
}

PRIVATE DREF ClassAttributeTableIterator *DCALL
cat_iter(ClassAttributeTable *__restrict self) {
 DREF ClassAttributeTableIterator *result;
 result = DeeObject_MALLOC(ClassAttributeTableIterator);
 if unlikely(!result) goto done;
 result->ca_desc = self->ca_desc;
 result->ca_iter = self->ca_start;
 result->ca_end  = (self->ca_start + self->ca_mask + 1);
 Dee_Incref(self->ca_desc);
 DeeObject_Init(result,&ClassAttributeTableIterator_Type);
done:
 return result;
}

PRIVATE int DCALL
cat_bool(ClassAttributeTable *__restrict self) {
 size_t i;
 for (i = 0; i <= self->ca_mask; ++i) {
  if (self->ca_start[i].ca_name != NULL)
      return 1;
 }
 return 0;
}

PRIVATE size_t DCALL
cat_nsi_getsize(ClassAttributeTable *__restrict self) {
 size_t i,result = 0;
 for (i = 0; i <= self->ca_mask; ++i) {
  if (self->ca_desc[i].cd_name != NULL)
      ++result;
 }
 return result;
}

PRIVATE DREF DeeObject *DCALL
cat_size(ClassAttributeTable *__restrict self) {
 return DeeInt_NewSize(cat_nsi_getsize(self));
}

PRIVATE DREF DeeObject *DCALL
cat_getitemdef(ClassAttributeTable *__restrict self,
               DeeObject *__restrict key,
               DeeObject *__restrict defl) {
 dhash_t hash,i,perturb;
 if (DeeObject_AssertTypeExact(key,&DeeString_Type))
     goto err;
 hash = DeeString_Hash(key);
 i = perturb = hash & self->ca_mask;
 for (;; DeeClassDescriptor_CLSOPNEXT(i,perturb)) {
  struct class_attribute *at;
  at = &self->ca_start[i & self->ca_mask];
  if (at->ca_name == NULL) break;
  if (at->ca_hash != hash) continue;
  if (DeeString_SIZE(at->ca_name) != DeeString_SIZE(key))
      continue;
  if (memcmp(DeeString_STR(at->ca_name),DeeString_STR(key),
             DeeString_SIZE(key) * sizeof(char)) != 0)
      continue;
  return (DREF DeeObject *)cattr_new(self->ca_desc,at);
 }
/*nope:*/
 if (defl != ITER_DONE)
     Dee_Incref(defl);
 return defl;
err:
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
cat_getitem(ClassAttributeTable *__restrict self,
            DeeObject *__restrict key) {
 DREF DeeObject *result;
 result = cat_getitemdef(self,key,ITER_DONE);
 if (result == ITER_DONE) {
  err_unknown_key((DeeObject *)self,key);
  result = NULL;
 }
 return result;
}

PRIVATE DREF ClassAttributeTable *DCALL
cati_getseq(ClassAttributeTableIterator *__restrict self) {
 DREF ClassAttributeTable *result;
 result = DeeObject_MALLOC(ClassAttributeTable);
 if unlikely(!result) goto done;
 result->ca_desc = self->ca_desc;
 if (self->ca_end == self->ca_desc->cd_iattr_list + self->ca_desc->cd_iattr_mask + 1) {
  result->ca_start = self->ca_desc->cd_iattr_list;
  result->ca_mask  = self->ca_desc->cd_iattr_mask;
 } else {
  result->ca_start = self->ca_desc->cd_cattr_list;
  result->ca_mask  = self->ca_desc->cd_cattr_mask;
 }
 Dee_Incref(self->ca_desc);
 DeeObject_Init(result,&ClassAttributeTable_Type);
done:
 return result;
}

PRIVATE struct type_getset cati_getsets[] = {
    { "seq", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&cati_getseq, NULL, NULL,
      DOC("->?Aattributes?Ert:classdescriptor") },
    { NULL }
};

PRIVATE struct type_member cat_class_members[] = {
    TYPE_MEMBER_CONST("iterator",&ClassAttributeTableIterator_Type),
    TYPE_MEMBER_END
};


PRIVATE DREF DeeObject *DCALL
ca_str(ClassAttribute *__restrict self) {
 DeeStringObject *cnam = self->ca_desc->cd_name;
 if (cnam)
     return DeeString_Newf("%k.%k",cnam,self->ca_attr->ca_name);
 return_reference_((DeeObject *)self->ca_attr->ca_name);
}

PRIVATE DREF DeeObject *DCALL
ca_repr(ClassAttribute *__restrict self) {
 ClassDescriptor *desc = self->ca_desc;
 DeeStringObject *cnam = desc->cd_name;
 char field_id = 'c';
 if (self->ca_attr >= desc->cd_iattr_list &&
     self->ca_attr <= desc->cd_iattr_list + desc->cd_iattr_mask)
     field_id = 'i';
 if (cnam)
     return DeeString_Newf("%k.__class__.%cattr[%r]",cnam,field_id,self->ca_attr->ca_name);
 return DeeString_Newf("<anonymous>.__class__.%cattr[%r]",field_id,self->ca_attr->ca_name);
}

PRIVATE DREF DeeObject *DCALL
ca_getname(ClassAttribute *__restrict self) {
 return_reference_((DeeObject *)self->ca_attr->ca_name);
}

PRIVATE DREF DeeObject *DCALL
ca_getdoc(ClassAttribute *__restrict self) {
 DeeStringObject *result;
 result = self->ca_attr->ca_doc;
 if (!result) result = (DeeStringObject *)Dee_None;
 return_reference_((DeeObject *)result);
}

PRIVATE DREF DeeObject *DCALL
ca_getaddr(ClassAttribute *__restrict self) {
 return DeeInt_NewU16(self->ca_attr->ca_addr);
}

PRIVATE DREF DeeObject *DCALL
ca_isprivate(ClassAttribute *__restrict self) {
 return_bool(self->ca_attr->ca_flag & CLASS_ATTRIBUTE_FPRIVATE);
}
PRIVATE DREF DeeObject *DCALL
ca_isfinal(ClassAttribute *__restrict self) {
 return_bool(self->ca_attr->ca_flag & CLASS_ATTRIBUTE_FFINAL);
}
PRIVATE DREF DeeObject *DCALL
ca_isreadonly(ClassAttribute *__restrict self) {
 return_bool(self->ca_attr->ca_flag & CLASS_ATTRIBUTE_FREADONLY);
}
PRIVATE DREF DeeObject *DCALL
ca_ismethod(ClassAttribute *__restrict self) {
 return_bool(self->ca_attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD);
}
PRIVATE DREF DeeObject *DCALL
ca_isproperty(ClassAttribute *__restrict self) {
 return_bool(self->ca_attr->ca_flag & CLASS_ATTRIBUTE_FGETSET);
}
PRIVATE DREF DeeObject *DCALL
ca_isclassmem(ClassAttribute *__restrict self) {
 return_bool(self->ca_attr->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM);
}

PRIVATE DREF DeeObject *DCALL
ca_getisclassns(ClassAttribute *__restrict self) {
 ClassDescriptor *desc = self->ca_desc;
 return_bool(!(self->ca_attr >= desc->cd_iattr_list &&
               self->ca_attr <= desc->cd_iattr_list +
                                desc->cd_iattr_mask));
}

PRIVATE struct type_getset ca_getsets[] = {
    { "name", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&ca_getname, NULL, NULL, DOC("->?Dstring") },
    { "doc", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&ca_getdoc, NULL, NULL, DOC("->?X2?Dstring?N") },
    { "addr", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&ca_getaddr, NULL, NULL,
      DOC("->?Dint\n"
          "Index into the class/instance object table, where @this attribute is stored\n"
          "When #isclassmem or #isclassns are :true, this index and any index offset from it "
          "refer to the class object table. Otherwise, the instance object table is dereferenced\n"
          "This is done so-as to allow instance attributes such as member functions to be stored "
          "within the class itself, rather than having to be copied into each and every instance "
          "of the class\n"
          "S.a. :type.__ctable__ and :type.__itable__") },
    { "isprivate", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&ca_isprivate, NULL, NULL,
      DOC("->?Dbool\n"
          "Evaluates to :true if @this class attribute was declared as $private") },
    { "isfinal", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&ca_isfinal, NULL, NULL,
      DOC("->?Dbool\n"
          "Evaluates to :true if @this class attribute was declared as $final") },
    { "isreadonly", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&ca_isreadonly, NULL, NULL,
      DOC("->?Dbool\n"
          "Evaluates to :true if @this class attribute can only be read from\n"
          "When this is case, a property-like attribute can only ever have a getter "
          "associated with itself, while field- or method-like attribute can only be "
          "written once (aka. when not already bound)") },
    { "ismethod", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&ca_ismethod, NULL, NULL,
      DOC("->?Dbool\n"
          "Evaluates to :true if @this class attribute referrs to a method\n"
          "When set, reading from the attribute will return a an object "
          "${instancemethod(obj.MEMBER_TABLE[this.addr],obj)}\n"
          "Note however that this is rarely ever required to be done, as method attributes "
          "are usually called directly, in which case a callattr instruction can silently "
          "prepend the this-argument before the passed argument list") },
    { "isproperty", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&ca_isproperty, NULL, NULL,
      DOC("->?Dbool\n"
          "Evaluates to :true if @this class attribute was defined as a property\n"
          "When this is the case, a #readonly attribute only has a single callback "
          "that may be stored at #addr + 0, with that callback being the getter\n"
          "Otherwise, up to 3 indices within the associated object table are used by "
          "@this attribute, each of which may be left unbound:\n"
          "%{table Offset|Callback\n"
          "$" PP_STR(CLASS_GETSET_GET) "|The getter callback ($get)\n"
          "$" PP_STR(CLASS_GETSET_DEL) "|The delete callback ($del)\n"
          "$" PP_STR(CLASS_GETSET_SET) "|The setter callback ($set)}") },
    { "isclassmem", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&ca_isclassmem, NULL, NULL,
      DOC("->?Dbool\n"
          "Set if #addr is an index into the class object table, rather than into the "
          "instance object table. Note however that when #isclassns") },
    { "isclassns", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&ca_getisclassns, NULL, NULL,
      DOC("->?Dbool\n"
          "Returns :true if @this class attribute is exclusive to the "
          "class-namespace (i.e. was declared as $static)\n"
          "During enumeration of attributes, all attributes where this is :true "
          "are enumated by :classdescriptor.cattr, while all for which it isn't "
          "are enumated by :classdescriptor.iattr") },
    { NULL }
};


PRIVATE struct type_nsi cat_nsi = {
    /* .nsi_class   = */TYPE_SEQX_CLASS_MAP,
    /* .nsi_flags   = */TYPE_SEQX_FNORMAL,
    {
        /* .nsi_maplike = */{
                /* .nsi_getsize    = */(void *)&cat_nsi_getsize,
                /* .nsi_nextkey    = */(void *)&cati_next_key,
                /* .nsi_nextvalue  = */(void *)&cati_next_value,
                /* .nsi_getdefault = */(void *)&cat_getitemdef,
                /* .nsi_setdefault = */(void *)NULL,
                /* .nsi_updateold  = */(void *)NULL,
                /* .nsi_insertnew  = */(void *)NULL
        }
    }
};

PRIVATE struct type_seq cat_seq = {
    /* .tp_iter_self = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&cat_iter,
    /* .tp_size      = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&cat_size,
    /* .tp_contains  = */NULL,
    /* .tp_get       = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&cat_getitem,
    /* .tp_del       = */NULL,
    /* .tp_set       = */NULL,
    /* .tp_range_get = */NULL,
    /* .tp_range_del = */NULL,
    /* .tp_range_set = */NULL,
    /* .tp_nsi       = */&cat_nsi
};

INTERN DeeTypeObject ClassAttribute_Type = {
    OBJECT_HEAD_INIT(&DeeType_Type),
    /* .tp_name     = */"_classattribute",
    /* .tp_doc      = */NULL,
    /* .tp_flags    = */TP_FFINAL,
    /* .tp_weakrefs = */0,
    /* .tp_features = */TF_NONE,
    /* .tp_base     = */&DeeObject_Type,
    /* .tp_init = */{
        {
            /* .tp_alloc = */{
                /* .tp_ctor      = */NULL,
                /* .tp_copy_ctor = */NULL,
                /* .tp_deep_ctor = */NULL,
                /* .tp_any_ctor  = */NULL,
                /* .tp_free      = */NULL,
                {
                    /* .tp_instance_size = */sizeof(ClassAttribute)
                }
            }
        },
        /* .tp_dtor        = */(void(DCALL *)(DeeObject *__restrict))&ca_fini,
        /* .tp_assign      = */NULL,
        /* .tp_move_assign = */NULL
    },
    /* .tp_cast = */{
        /* .tp_str  = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&ca_str,
        /* .tp_repr = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&ca_repr,
        /* .tp_bool = */NULL
    },
    /* .tp_call          = */NULL,
    /* .tp_visit         = */NULL, /* No visit, because only ClassDescriptor is reachable, which doesn't have visit itself */
    /* .tp_gc            = */NULL,
    /* .tp_math          = */NULL,
    /* .tp_cmp           = */NULL,
    /* .tp_seq           = */NULL,
    /* .tp_iter_next     = */NULL,
    /* .tp_attr          = */NULL,
    /* .tp_with          = */NULL,
    /* .tp_buffer        = */NULL,
    /* .tp_methods       = */NULL,
    /* .tp_getsets       = */ca_getsets,
    /* .tp_members       = */ca_members,
    /* .tp_class_methods = */NULL,
    /* .tp_class_getsets = */NULL,
    /* .tp_class_members = */NULL
};

INTERN DeeTypeObject ClassAttributeTableIterator_Type = {
    OBJECT_HEAD_INIT(&DeeType_Type),
    /* .tp_name     = */"_classattributetableiterator",
    /* .tp_doc      = */NULL,
    /* .tp_flags    = */TP_FFINAL,
    /* .tp_weakrefs = */0,
    /* .tp_features = */TF_NONE,
    /* .tp_base     = */&DeeMapping_Type,
    /* .tp_init = */{
        {
            /* .tp_alloc = */{
                /* .tp_ctor      = */NULL,
                /* .tp_copy_ctor = */(void *)&cati_copy,
                /* .tp_deep_ctor = */NULL,
                /* .tp_any_ctor  = */NULL,
                /* .tp_free      = */NULL,
                {
                    /* .tp_instance_size = */sizeof(ClassAttributeTableIterator)
                }
            }
        },
        /* .tp_dtor        = */(void(DCALL *)(DeeObject *__restrict))&cati_fini,
        /* .tp_assign      = */NULL,
        /* .tp_move_assign = */NULL
    },
    /* .tp_cast = */{
        /* .tp_str  = */NULL,
        /* .tp_repr = */NULL,
        /* .tp_bool = */NULL
    },
    /* .tp_call          = */NULL,
    /* .tp_visit         = */NULL, /* No visit, because only ClassDescriptor is reachable, which doesn't have visit itself */
    /* .tp_gc            = */NULL,
    /* .tp_math          = */NULL,
    /* .tp_cmp           = */&cati_cmp,
    /* .tp_seq           = */NULL,
    /* .tp_iter_next     = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&cati_next,
    /* .tp_attr          = */NULL,
    /* .tp_with          = */NULL,
    /* .tp_buffer        = */NULL,
    /* .tp_methods       = */NULL,
    /* .tp_getsets       = */cati_getsets,
    /* .tp_members       = */cati_members,
    /* .tp_class_methods = */NULL,
    /* .tp_class_getsets = */NULL,
    /* .tp_class_members = */NULL
};

INTERN DeeTypeObject ClassAttributeTable_Type = {
    OBJECT_HEAD_INIT(&DeeType_Type),
    /* .tp_name     = */"_classattributetable",
    /* .tp_doc      = */NULL,
    /* .tp_flags    = */TP_FFINAL,
    /* .tp_weakrefs = */0,
    /* .tp_features = */TF_NONE,
    /* .tp_base     = */&DeeMapping_Type,
    /* .tp_init = */{
        {
            /* .tp_alloc = */{
                /* .tp_ctor      = */NULL,
                /* .tp_copy_ctor = */NULL,
                /* .tp_deep_ctor = */NULL,
                /* .tp_any_ctor  = */NULL,
                /* .tp_free      = */NULL,
                {
                    /* .tp_instance_size = */sizeof(ClassAttributeTable)
                }
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
    /* .tp_visit         = */NULL, /* No visit, because only ClassDescriptor is reachable, which doesn't have visit itself */
    /* .tp_gc            = */NULL,
    /* .tp_math          = */NULL,
    /* .tp_cmp           = */NULL,
    /* .tp_seq           = */&cat_seq,
    /* .tp_iter_next     = */NULL,
    /* .tp_attr          = */NULL,
    /* .tp_with          = */NULL,
    /* .tp_buffer        = */NULL,
    /* .tp_methods       = */NULL,
    /* .tp_getsets       = */NULL,
    /* .tp_members       = */cat_members,
    /* .tp_class_methods = */NULL,
    /* .tp_class_getsets = */NULL,
    /* .tp_class_members = */cat_class_members
};








PRIVATE void DCALL
cd_fini(ClassDescriptor *__restrict self) {
 size_t i;
 if (self->cd_cattr_list != empty_class_attributes) {
  for (i = 0; i <= self->cd_cattr_mask; ++i) {
   if (!self->cd_cattr_list[i].ca_name)
        continue;
   Dee_Decref(self->cd_cattr_list[i].ca_name);
   Dee_XDecref(self->cd_cattr_list[i].ca_doc);
  }
  Dee_Free(self->cd_cattr_list);
 }
 if (self->cd_clsop_list != empty_class_operators)
     Dee_Free(self->cd_clsop_list);
 for (i = 0; i <= self->cd_iattr_mask; ++i) {
  if (!self->cd_iattr_list[i].ca_name)
       continue;
  Dee_Decref(self->cd_iattr_list[i].ca_name);
  Dee_XDecref(self->cd_iattr_list[i].ca_doc);
 }
 Dee_XDecref(self->cd_name);
 Dee_XDecref(self->cd_doc);
}


PRIVATE bool DCALL
class_attribute_eq(struct class_attribute *__restrict lhs,
                   struct class_attribute *__restrict rhs) {
 if (!lhs->ca_name)
     return rhs->ca_name == NULL;
 if (!rhs->ca_name)
     goto nope;
 if ((lhs->ca_doc != NULL) != (rhs->ca_doc != NULL))
     goto nope;
 if (lhs->ca_flag != rhs->ca_flag)
     goto nope;
 if (lhs->ca_addr != rhs->ca_addr)
     goto nope;
 if (lhs->ca_hash != rhs->ca_hash)
     goto nope;
 if (DeeString_SIZE(lhs->ca_name) !=
     DeeString_SIZE(rhs->ca_name))
     goto nope;
 if (memcmp(DeeString_STR(lhs->ca_name),
            DeeString_STR(rhs->ca_name),
            DeeString_SIZE(lhs->ca_name) * sizeof(char)) != 0)
     goto nope;
 if (lhs->ca_doc) {
  if (DeeString_SIZE(lhs->ca_doc) !=
      DeeString_SIZE(rhs->ca_doc))
      goto nope;
  if (memcmp(DeeString_STR(lhs->ca_doc),
             DeeString_STR(rhs->ca_doc),
             DeeString_SIZE(lhs->ca_doc) * sizeof(char)) != 0)
      goto nope;
 }
 return true;
nope:
 return false;
}

PRIVATE DREF DeeObject *DCALL
cd_eq(ClassDescriptor *__restrict self,
      ClassDescriptor *__restrict other) {
 size_t i;
 if (!DeeClassDescriptor_Check(other))
      goto nope;
 if (self->cd_flags != other->cd_flags) goto nope;
 if (self->cd_cmemb_size != other->cd_cmemb_size) goto nope;
 if (self->cd_imemb_size != other->cd_imemb_size) goto nope;
 if (self->cd_clsop_mask != other->cd_clsop_mask) goto nope;
 if (self->cd_cattr_mask != other->cd_cattr_mask) goto nope;
 if (self->cd_iattr_mask != other->cd_iattr_mask) goto nope;
 if (self->cd_name) {
  if (!other->cd_name) goto nope;
  if (DeeString_SIZE(self->cd_name) != DeeString_SIZE(other->cd_name))
      goto nope;
  if (memcmp(DeeString_STR(self->cd_name),
             DeeString_STR(other->cd_name),
             DeeString_SIZE(other->cd_name) * sizeof(char)) != 0)
      goto nope;
 } else {
  if (other->cd_name) goto nope;
 }
 if (self->cd_doc) {
  if (!other->cd_doc) goto nope;
  if (DeeString_SIZE(self->cd_doc) != DeeString_SIZE(other->cd_doc))
      goto nope;
  if (memcmp(DeeString_STR(self->cd_doc),
             DeeString_STR(other->cd_doc),
             DeeString_SIZE(other->cd_doc) * sizeof(char)) != 0)
      goto nope;
 } else {
  if (other->cd_doc) goto nope;
 }
 if (memcmp(self->cd_clsop_list,
            other->cd_clsop_list,
           (self->cd_clsop_mask + 1) *
            sizeof(struct class_operator)) != 0)
     goto nope;
 for (i = 0; i <= self->cd_cattr_mask; ++i) {
  if (!class_attribute_eq(&self->cd_cattr_list[i],
                          &other->cd_cattr_list[i]))
      goto nope;
 }
 for (i = 0; i <= self->cd_iattr_mask; ++i) {
  if (!class_attribute_eq(&self->cd_iattr_list[i],
                          &other->cd_iattr_list[i]))
      goto nope;
 }
 return_true;
nope:
 return_false;
}


PRIVATE struct type_cmp cd_cmp = {
    /* .tp_hash = */NULL,
    /* .tp_eq   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&cd_eq
};


PRIVATE DREF DeeObject *DCALL
cd_isfinal(ClassDescriptor *__restrict self) {
 return_bool(self->cd_flags & TP_FFINAL);
}
PRIVATE DREF DeeObject *DCALL
cd_isinttruncated(ClassDescriptor *__restrict self) {
 return_bool(self->cd_flags & TP_FTRUNCATE);
}
PRIVATE DREF DeeObject *DCALL
cd_isinterrupt(ClassDescriptor *__restrict self) {
 return_bool(self->cd_flags & TP_FINTERRUPT);
}
PRIVATE DREF DeeObject *DCALL
cd_ismoveany(ClassDescriptor *__restrict self) {
 return_bool(self->cd_flags & TP_FMOVEANY);
}
PRIVATE DREF ClassOperatorTable *DCALL
cd_operators(ClassDescriptor *__restrict self) {
 DREF ClassOperatorTable *result;
 result = DeeObject_MALLOC(ClassOperatorTable);
 if unlikely(!result) goto done;
 result->co_desc = self;
 Dee_Incref(self);
 DeeObject_Init(result,&ClassOperatorTable_Type);
done:
 return result;
}
PRIVATE DREF ClassAttributeTable *DCALL
cd_iattr(ClassDescriptor *__restrict self) {
 DREF ClassAttributeTable *result;
 result = DeeObject_MALLOC(ClassAttributeTable);
 if unlikely(!result) goto done;
 result->ca_desc  = self;
 result->ca_start = self->cd_iattr_list;
 result->ca_mask  = self->cd_iattr_mask;
 Dee_Incref(self);
 DeeObject_Init(result,&ClassAttributeTable_Type);
done:
 return result;
}
PRIVATE DREF ClassAttributeTable *DCALL
cd_cattr(ClassDescriptor *__restrict self) {
 DREF ClassAttributeTable *result;
 result = DeeObject_MALLOC(ClassAttributeTable);
 if unlikely(!result) goto done;
 result->ca_desc  = self;
 result->ca_start = self->cd_cattr_list;
 result->ca_mask  = self->cd_cattr_mask;
 Dee_Incref(self);
 DeeObject_Init(result,&ClassAttributeTable_Type);
done:
 return result;
}

PRIVATE struct type_getset cd_getsets[] = {
    { "isfinal", (DeeObject *(DCALL *)(DeeObject *__restrict))&cd_isfinal, NULL, NULL, DOC("->?Dbool") },
    { "isinterrupt", (DeeObject *(DCALL *)(DeeObject *__restrict))&cd_isinterrupt, NULL, NULL, DOC("->?Dbool") },
    { "__isinttruncated__", (DeeObject *(DCALL *)(DeeObject *__restrict))&cd_isinttruncated, NULL, NULL, DOC("->?Dbool") },
    { "__ismoveany__", (DeeObject *(DCALL *)(DeeObject *__restrict))&cd_ismoveany, NULL, NULL, DOC("->?Dbool") },
    { "operators", (DeeObject *(DCALL *)(DeeObject *__restrict))&cd_operators, NULL, NULL,
      DOC("->?#operators\n"
          "Enumerate operators implemented by @this class, as well as their associated "
          "class object table indices which are holding the respective implementations\n"
          "Note that the class object table entry may be left unbound to explicitly "
          "define an operator as having been deleted") },
    { "iattr", (DeeObject *(DCALL *)(DeeObject *__restrict))&cd_iattr, NULL, NULL,
      DOC("->?#attributes\n"
          "Enumerate user-defined instance attributes as a mapping-like string-attribute sequence") },
    { "cattr", (DeeObject *(DCALL *)(DeeObject *__restrict))&cd_cattr, NULL, NULL,
      DOC("->?#attributes\n"
          "Enumerate user-defined class ($static) attributes as a mapping-like string-attribute sequence") },
    { NULL }
};

PRIVATE struct type_member cd_members[] = {
    TYPE_MEMBER_FIELD_DOC("__name__",STRUCT_OBJECT_OPT,offsetof(ClassDescriptor,cd_name),"->?X2?Dstring?N"),
    TYPE_MEMBER_FIELD_DOC("__doc__",STRUCT_OBJECT_OPT,offsetof(ClassDescriptor,cd_doc),"->?X2?Dstring?N"),
    TYPE_MEMBER_FIELD_DOC("__csize__",STRUCT_CONST|STRUCT_UINT16_T,offsetof(ClassDescriptor,cd_cmemb_size),"Size of the class object table"),
    TYPE_MEMBER_FIELD_DOC("__isize__",STRUCT_CONST|STRUCT_UINT16_T,offsetof(ClassDescriptor,cd_imemb_size),"Size of the instance object table"),
    TYPE_MEMBER_END
};

INTDEF DeeTypeObject ObjectTable_Type;
PRIVATE struct type_member cd_class_members[] = {
    TYPE_MEMBER_CONST("operators",&ClassOperatorTable_Type),
    TYPE_MEMBER_CONST("attribute",&ClassAttribute_Type),
    TYPE_MEMBER_CONST("attributes",&ClassAttributeTable_Type),
    TYPE_MEMBER_CONST("objecttable",&ObjectTable_Type),
    TYPE_MEMBER_END
};

PRIVATE DREF DeeObject *DCALL
cd_str(ClassDescriptor *__restrict self) {
 if (self->cd_name)
     return_reference_((DeeObject *)self->cd_name);
 return DeeString_New("<anonymous>");
}


PUBLIC DeeTypeObject DeeClassDescriptor_Type = {
    OBJECT_HEAD_INIT(&DeeType_Type),
    /* .tp_name     = */"_classdescriptor",
    /* .tp_doc      = */NULL,
    /* .tp_flags    = */TP_FVARIABLE | TP_FFINAL,
    /* .tp_weakrefs = */0,
    /* .tp_features = */TF_NONE,
    /* .tp_base     = */&DeeObject_Type,
    /* .tp_init = */{
        {
            /* .tp_var = */{
                /* .tp_ctor        = */NULL,
                /* .tp_copy_ctor   = */&DeeObject_NewRef,
                /* .tp_deep_ctor   = */&DeeObject_NewRef,
                /* .tp_any_ctor    = */NULL,
                /* .tp_free        = */NULL
            }
        },
        /* .tp_dtor        = */(void(DCALL *)(DeeObject *__restrict))&cd_fini,
        /* .tp_assign      = */NULL,
        /* .tp_move_assign = */NULL
    },
    /* .tp_cast = */{
        /* .tp_str  = */(DeeObject *(DCALL *)(DeeObject *__restrict))&cd_str,
        /* .tp_repr = */NULL,
        /* .tp_bool = */NULL
    },
    /* .tp_call          = */NULL,
    /* .tp_visit         = */NULL, /* No visit, because only string objects are referenced! */
    /* .tp_gc            = */NULL,
    /* .tp_math          = */NULL,
    /* .tp_cmp           = */&cd_cmp,
    /* .tp_seq           = */NULL,
    /* .tp_iter_next     = */NULL,
    /* .tp_attr          = */NULL,
    /* .tp_with          = */NULL,
    /* .tp_buffer        = */NULL,
    /* .tp_methods       = */NULL,
    /* .tp_getsets       = */cd_getsets,
    /* .tp_members       = */cd_members,
    /* .tp_class_methods = */NULL,
    /* .tp_class_getsets = */NULL,
    /* .tp_class_members = */cd_class_members
};



typedef struct {
    OBJECT_HEAD
    DREF DeeObject       *ot_owner; /* [1..1][const] The associated owner object.
                                     * NOTE: This may be a super-object, in which case the referenced
                                     *       object table refers to the described super-type. */
    struct instance_desc *ot_desc;  /* [1..1][valid_if(ot_size != 0)][const] The referenced instance descriptor. */
    uint16_t              ot_size;  /* [const] The length of the object table contained within `ot_desc' */
} ObjectTable;

STATIC_ASSERT(COMPILER_OFFSETOF(ObjectTable,ot_owner) ==
              COMPILER_OFFSETOF(ClassAttributeTable,ca_desc));
#define ot_fini    cot_fini

PRIVATE DREF DeeObject *DCALL
ot_str(ObjectTable *__restrict self) {
 DeeTypeObject *tp = DeeObject_Class(self->ot_owner);
 if (DeeType_IsTypeType(tp))
     return DeeString_Newf("<class object table for %k>",tp);
 return DeeString_Newf("<object table for instance of %k>",tp);
}

PRIVATE int DCALL
ot_bool(ObjectTable *__restrict self) {
 return self->ot_size != 0;
}

PRIVATE void DCALL
ot_visit(ObjectTable *__restrict self, dvisit_t proc, void *arg) {
 Dee_Visit(self->ot_owner);
}


PRIVATE size_t DCALL
ot_nsi_getsize(ObjectTable *__restrict self) {
 return self->ot_size;
}
PRIVATE DREF DeeObject *DCALL
ot_size(ObjectTable *__restrict self) {
 return DeeInt_NewU16(self->ot_size);
}

PRIVATE DREF DeeObject *DCALL
ot_nsi_getitem(ObjectTable *__restrict self, size_t index) {
 DREF DeeObject *result;
 if unlikely(index >= self->ot_size)
    goto err_index;
 rwlock_read(&self->ot_desc->id_lock);
 result = self->ot_desc->id_vtab[index];
 if unlikely(!result) goto err_unbound;
 Dee_Incref(result);
 rwlock_endread(&self->ot_desc->id_lock);
 return result;
err_unbound:
 rwlock_endread(&self->ot_desc->id_lock);
 err_unbound_index((DeeObject *)self,index);
 return NULL;
err_index:
 err_index_out_of_bounds((DeeObject *)self,index,self->ot_size);
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
ot_nsi_getitem_fast(ObjectTable *__restrict self, size_t index) {
 DREF DeeObject *result;
 ASSERT(index < self->ot_size);
 rwlock_read(&self->ot_desc->id_lock);
 result = self->ot_desc->id_vtab[index];
 Dee_XIncref(result);
 rwlock_endread(&self->ot_desc->id_lock);
 return result;
}

PRIVATE int DCALL
ot_nsi_delitem(ObjectTable *__restrict self, size_t index) {
 DREF DeeObject *oldval;
 if unlikely(index >= self->ot_size)
    goto err_index;
 rwlock_write(&self->ot_desc->id_lock);
 oldval = self->ot_desc->id_vtab[index];
#ifdef CONFIG_ERROR_DELETE_UNBOUND
 if unlikely(!oldval) goto err_unbound;
#endif
 self->ot_desc->id_vtab[index] = NULL;
 rwlock_endwrite(&self->ot_desc->id_lock);
#ifdef CONFIG_ERROR_DELETE_UNBOUND
 Dee_Decref(oldval);
#else
 Dee_XDecref(oldval);
#endif
 return 0;
#ifdef CONFIG_ERROR_DELETE_UNBOUND
err_unbound:
 rwlock_endwrite(&self->ot_desc->id_lock);
 return err_unbound_index((DeeObject *)self,index);
#endif
err_index:
 return err_index_out_of_bounds((DeeObject *)self,index,self->ot_size);
}

PRIVATE int DCALL
ot_nsi_setitem(ObjectTable *__restrict self, size_t index,
               DeeObject *__restrict value) {
 DREF DeeObject *oldval;
 if unlikely(index >= self->ot_size)
    goto err_index;
 Dee_Incref(value);
 rwlock_write(&self->ot_desc->id_lock);
 oldval = self->ot_desc->id_vtab[index];
 self->ot_desc->id_vtab[index] = value;
 rwlock_endwrite(&self->ot_desc->id_lock);
 Dee_XDecref(oldval);
 return 0;
err_index:
 return err_index_out_of_bounds((DeeObject *)self,index,self->ot_size);
}

PRIVATE DREF DeeObject *DCALL
ot_nsi_xchitem(ObjectTable *__restrict self, size_t index,
               DeeObject *__restrict newval) {
 DREF DeeObject *oldval;
 if unlikely(index >= self->ot_size)
    goto err_index;
 rwlock_write(&self->ot_desc->id_lock);
 oldval = self->ot_desc->id_vtab[index];
 if unlikely(!oldval) goto err_unbound;
 Dee_Incref(newval);
 self->ot_desc->id_vtab[index] = newval;
 rwlock_endwrite(&self->ot_desc->id_lock);
 return oldval;
err_unbound:
 rwlock_endwrite(&self->ot_desc->id_lock);
 err_unbound_index((DeeObject *)self,index);
 goto err;
err_index:
 err_index_out_of_bounds((DeeObject *)self,index,self->ot_size);
err:
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
ot_getitem(ObjectTable *__restrict self,
           DeeObject *__restrict index) {
 size_t i;
 if (DeeObject_AsSize(index,&i))
     goto err;
 return ot_nsi_getitem(self,i);
err:
 return NULL;
}

PRIVATE int DCALL
ot_delitem(ObjectTable *__restrict self,
           DeeObject *__restrict index) {
 size_t i;
 if (DeeObject_AsSize(index,&i))
     goto err;
 return ot_nsi_delitem(self,i);
err:
 return -1;
}

PRIVATE int DCALL
ot_setitem(ObjectTable *__restrict self,
           DeeObject *__restrict index,
           DeeObject *__restrict value) {
 size_t i;
 if (DeeObject_AsSize(index,&i))
     goto err;
 return ot_nsi_setitem(self,i,value);
err:
 return -1;
}



PRIVATE struct type_nsi ot_nsi = {
    /* .nsi_class   = */TYPE_SEQX_CLASS_SEQ,
    /* .nsi_flags   = */TYPE_SEQX_FMUTABLE,
    {
        /* .nsi_seqlike = */{
            /* .nsi_getsize      = */(void *)&ot_nsi_getsize,
            /* .nsi_getsize_fast = */(void *)&ot_nsi_getsize,
            /* .nsi_getitem      = */(void *)&ot_nsi_getitem,
            /* .nsi_delitem      = */(void *)&ot_nsi_delitem,
            /* .nsi_setitem      = */(void *)&ot_nsi_setitem,
            /* .nsi_getitem_fast = */(void *)&ot_nsi_getitem_fast,
            /* .nsi_getrange     = */(void *)NULL,
            /* .nsi_getrange_n   = */(void *)NULL,
            /* .nsi_setrange     = */(void *)NULL,
            /* .nsi_setrange_n   = */(void *)NULL,
            /* .nsi_find         = */(void *)NULL,
            /* .nsi_rfind        = */(void *)NULL,
            /* .nsi_xch          = */(void *)&ot_nsi_xchitem,
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

PRIVATE struct type_seq ot_seq = {
    /* .tp_iter_self = */NULL,
    /* .tp_size      = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&ot_size,
    /* .tp_contains  = */NULL,
    /* .tp_get       = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&ot_getitem,
    /* .tp_del       = */(int(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&ot_delitem,
    /* .tp_set       = */(int(DCALL *)(DeeObject *__restrict,DeeObject *__restrict,DeeObject *__restrict))&ot_setitem,
    /* .tp_range_get = */NULL,
    /* .tp_range_del = */NULL,
    /* .tp_range_set = */NULL,
    /* .tp_nsi       = */&ot_nsi
};


PRIVATE DREF DeeTypeObject *DCALL
ot_gettype(ObjectTable *__restrict self) {
 DREF DeeTypeObject *result;
 result = DeeType_Check(self->ot_owner)
        ? (DeeTypeObject *)self->ot_owner
        : Dee_TYPE(self->ot_owner);
 if (result == &DeeSuper_Type)
     result = DeeSuper_TYPE(self->ot_owner);
 return_reference_(result);
}

PRIVATE DREF ClassDescriptor *DCALL
ot_getclass(ObjectTable *__restrict self) {
 DREF ClassDescriptor *result;
 DREF DeeTypeObject *type;
 type = DeeType_Check(self->ot_owner)
      ? (DeeTypeObject *)self->ot_owner
      : Dee_TYPE(self->ot_owner);
 if (type == &DeeSuper_Type)
     type = DeeSuper_TYPE(self->ot_owner);
 result = DeeClass_DESC(type)->cd_desc;
 return_reference_(result);
}

PRIVATE DREF DeeObject *DCALL
ot_isctable(ObjectTable *__restrict self) {
 return_bool(DeeType_Check(self->ot_owner));
}
PRIVATE DREF DeeObject *DCALL
ot_isitable(ObjectTable *__restrict self) {
 return_bool(!DeeType_Check(self->ot_owner));
}

PRIVATE struct type_getset ot_getsets[] = {
    { "__type__", (DeeObject *(DCALL *)(DeeObject *__restrict))&ot_gettype, NULL, NULL,
       DOC("->?Dtype\nThe type describing @this object table") },
    { "__class__", (DeeObject *(DCALL *)(DeeObject *__restrict))&ot_getclass, NULL, NULL,
      DOC("->?Ert:classdescriptor\nSame as ${this.__type__.__class__}") },
    { "__isctable__", (DeeObject *(DCALL *)(DeeObject *__restrict))&ot_isctable, NULL, NULL,
      DOC("->?Dbool\nEvaluates to :true if @this is a class object table") },
    { "__isitable__", (DeeObject *(DCALL *)(DeeObject *__restrict))&ot_isitable, NULL, NULL,
      DOC("->?Dbool\nEvaluates to :true if @this is an instance object table") },
    { NULL }
};

PRIVATE struct type_member ot_members[] = {
    TYPE_MEMBER_FIELD_DOC("__owner__",STRUCT_OBJECT,offsetof(ClassOperatorTableIterator,co_desc),
                          "The object that owns @this object table"),
    TYPE_MEMBER_END
};


INTERN DeeTypeObject ObjectTable_Type = {
    OBJECT_HEAD_INIT(&DeeType_Type),
    /* .tp_name     = */"_objecttable",
    /* .tp_doc      = */NULL,
    /* .tp_flags    = */TP_FFINAL,
    /* .tp_weakrefs = */0,
    /* .tp_features = */TF_NONE,
    /* .tp_base     = */&DeeSeq_Type,
    /* .tp_init = */{
        {
            /* .tp_alloc = */{
                /* .tp_ctor      = */NULL,
                /* .tp_copy_ctor = */NULL,
                /* .tp_deep_ctor = */NULL,
                /* .tp_any_ctor  = */NULL,
                /* .tp_free      = */NULL,
                {
                    /* .tp_instance_size = */sizeof(ObjectTable)
                }
            }
        },
        /* .tp_dtor        = */(void(DCALL *)(DeeObject *__restrict))&ot_fini,
        /* .tp_assign      = */NULL,
        /* .tp_move_assign = */NULL
    },
    /* .tp_cast = */{
        /* .tp_str  = */(DeeObject *(DCALL *)(DeeObject *__restrict))&ot_str,
        /* .tp_repr = */NULL,
        /* .tp_bool = */(int(DCALL *)(DeeObject *__restrict))&ot_bool
    },
    /* .tp_call          = */NULL,
    /* .tp_visit         = */(void(DCALL *)(DeeObject *__restrict,dvisit_t,void*))&ot_visit,
    /* .tp_gc            = */NULL,
    /* .tp_math          = */NULL,
    /* .tp_cmp           = */NULL,
    /* .tp_seq           = */&ot_seq,
    /* .tp_iter_next     = */NULL,
    /* .tp_attr          = */NULL,
    /* .tp_with          = */NULL,
    /* .tp_buffer        = */NULL,
    /* .tp_methods       = */NULL,
    /* .tp_getsets       = */ot_getsets,
    /* .tp_members       = */ot_members,
    /* .tp_class_methods = */NULL,
    /* .tp_class_getsets = */NULL,
    /* .tp_class_members = */NULL
};


INTERN DREF DeeObject *DCALL
type_get_ctable(DeeTypeObject *__restrict self) {
 DREF ObjectTable *result;
 struct class_desc *desc;
 if (!DeeType_IsClass(self))
     return_empty_seq;
 desc = DeeClass_DESC(self);
 result = DeeObject_MALLOC(DREF ObjectTable);
 if unlikely(!result) goto done;
 result->ot_owner = (DREF DeeObject *)self;
 result->ot_desc  = class_desc_as_instance(desc);
 result->ot_size  = desc->cd_desc->cd_cmemb_size;
 Dee_Incref(self);
 DeeObject_Init(result,&ObjectTable_Type);
done:
 return (DREF DeeObject *)result;
}

INTERN DREF DeeObject *DCALL
instance_get_itable(DeeObject *__restrict self) {
 DREF ObjectTable *result;
 struct class_desc *desc;
 DeeTypeObject *type;
 DeeObject *real_self = self;
 type = Dee_TYPE(self);
 if (type == &DeeSuper_Type) {
  type = DeeSuper_TYPE(self);
  real_self = DeeSuper_SELF(self);
 }
 if (!DeeType_IsClass(type))
     return_empty_seq;
 desc = DeeClass_DESC(type);
 result = DeeObject_MALLOC(DREF ObjectTable);
 if unlikely(!result) goto done;
 result->ot_owner = (DREF DeeObject *)self;
 result->ot_desc  = DeeInstance_DESC(desc,real_self);
 result->ot_size  = desc->cd_desc->cd_imemb_size;
 Dee_Incref(self);
 DeeObject_Init(result,&ObjectTable_Type);
done:
 return (DREF DeeObject *)result;
}





PRIVATE struct keyword thisarg_kwlist[] = { K(thisarg), KEND };

PRIVATE DREF DeeObject *DCALL
instancemember_get(DeeInstanceMemberObject *__restrict self,
                   size_t argc, DeeObject **__restrict argv,
                   DeeObject *kw) {
 DeeObject *thisarg; struct class_desc *desc;
 if (DeeArg_UnpackKw(argc,argv,kw,thisarg_kwlist,"o:get",&thisarg))
     goto err;
 if (DeeObject_AssertType(thisarg,self->im_type))
     goto err;
 desc = DeeClass_DESC(self->im_type);
 return DeeInstance_GetAttribute(desc,
                                 DeeInstance_DESC(desc,
                                                  thisarg),
                                 thisarg,
                                 self->im_attribute);
err:
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
instancemember_delete(DeeInstanceMemberObject *__restrict self,
                      size_t argc, DeeObject **__restrict argv,
                      DeeObject *kw) {
 DeeObject *thisarg; struct class_desc *desc;
 if (DeeArg_UnpackKw(argc,argv,kw,thisarg_kwlist,"o:delete",&thisarg))
     goto err;
 if (DeeObject_AssertType(thisarg,self->im_type))
     goto err;
 desc = DeeClass_DESC(self->im_type);
 if (DeeInstance_DelAttribute(desc,
                              DeeInstance_DESC(desc,
                                               thisarg),
                              thisarg,
                              self->im_attribute))
     goto err;
 return_none;
err:
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
instancemember_set(DeeInstanceMemberObject *__restrict self,
                   size_t argc, DeeObject **__restrict argv,
                   DeeObject *kw) {
 DeeObject *thisarg,*value; struct class_desc *desc;
 PRIVATE struct keyword kwlist[] = { K(thisarg), K(value), KEND };
 if (DeeArg_UnpackKw(argc,argv,kw,kwlist,"oo:set",&thisarg,&value))
     goto err;
 if (DeeObject_AssertType(thisarg,self->im_type))
     goto err;
 desc = DeeClass_DESC(self->im_type);
 if (DeeInstance_SetAttribute(desc,
                              DeeInstance_DESC(desc,
                                               thisarg),
                              thisarg,
                              self->im_attribute,
                              value))
     goto err;
 return_none;
err:
 return NULL;
}

PRIVATE int DCALL
instancemember_copy(DeeInstanceMemberObject *__restrict self,
                    DeeInstanceMemberObject *__restrict other) {
 self->im_type      = other->im_type;
 self->im_attribute = other->im_attribute;
 Dee_Incref(other->im_type);
 return 0;
}

PRIVATE void DCALL
instancemember_fini(DeeInstanceMemberObject *__restrict self) {
 Dee_Decref(self->im_type);
}
PRIVATE void DCALL
instancemember_visit(DeeInstanceMemberObject *__restrict self, dvisit_t proc, void *arg) {
 Dee_Visit(self->im_type);
}


PRIVATE struct type_method instancemember_methods[] = {
    { "get",
     (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&instancemember_get,
      DOC("(thisarg)->\n"
          "Return the @thisarg's value of @this member"),
      TYPE_METHOD_FKWDS },
    { "delete",
     (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&instancemember_delete,
      DOC("(thisarg)\n"
          "Delete @thisarg's value of @this member"),
      TYPE_METHOD_FKWDS },
    { "set",
     (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&instancemember_set,
      DOC("(thisarg,value)\n"
          "Set @thisarg's value of @this member to @value"),
      TYPE_METHOD_FKWDS },
    { NULL }
};

PRIVATE DREF DeeObject *DCALL
instancemember_get_module(DeeInstanceMemberObject *__restrict self) {
 DREF DeeObject *result;
 result = DeeType_GetModule(self->im_type);
 if (!result) return_none;
 return result;
}

PRIVATE DREF DeeObject *DCALL
instancemember_get_name(DeeInstanceMemberObject *__restrict self) {
 return_reference_((DREF DeeObject *)self->im_attribute->ca_name);
}
PRIVATE DREF DeeObject *DCALL
instancemember_get_doc(DeeInstanceMemberObject *__restrict self) {
 if (!self->im_attribute->ca_doc) return_none;
 return_reference_((DREF DeeObject *)self->im_attribute->ca_doc);
}
PRIVATE DREF DeeObject *DCALL
instancemember_get_canget(DeeInstanceMemberObject *__restrict self) {
 ASSERT(self);
 if ((self->im_attribute->ca_flag & (CLASS_ATTRIBUTE_FGETSET | CLASS_ATTRIBUTE_FCLASSMEM)) ==
                                    (CLASS_ATTRIBUTE_FGETSET | CLASS_ATTRIBUTE_FCLASSMEM)) {
  if (!self->im_type->tp_class->cd_members[self->im_attribute->ca_addr + CLASS_GETSET_GET])
       return_false;
 }
 return_true;
}
PRIVATE DREF DeeObject *DCALL
instancemember_get_candel(DeeInstanceMemberObject *__restrict self) {
 ASSERT(self);
 if (self->im_attribute->ca_flag & CLASS_ATTRIBUTE_FREADONLY)
     return_false;
 if ((self->im_attribute->ca_flag & (CLASS_ATTRIBUTE_FGETSET | CLASS_ATTRIBUTE_FCLASSMEM)) ==
                                    (CLASS_ATTRIBUTE_FGETSET | CLASS_ATTRIBUTE_FCLASSMEM)) {
  if (!self->im_type->tp_class->cd_members[self->im_attribute->ca_addr + CLASS_GETSET_DEL])
       return_false;
 }
 return_true;
}
PRIVATE DREF DeeObject *DCALL
instancemember_get_canset(DeeInstanceMemberObject *__restrict self) {
 ASSERT(self);
 if (self->im_attribute->ca_flag & CLASS_ATTRIBUTE_FREADONLY)
     return_false;
 if ((self->im_attribute->ca_flag & (CLASS_ATTRIBUTE_FGETSET | CLASS_ATTRIBUTE_FCLASSMEM)) ==
                                    (CLASS_ATTRIBUTE_FGETSET | CLASS_ATTRIBUTE_FCLASSMEM)) {
  if (!self->im_type->tp_class->cd_members[self->im_attribute->ca_addr + CLASS_GETSET_SET])
       return_false;
 }
 return_true;
}

PRIVATE struct type_getset instancemember_getsets[] = {
    { "canget", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&instancemember_get_canget, NULL, NULL,
      DOC("->?Dbool\n"
          "Returns :true if @this member can be read from") },
    { "candel", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&instancemember_get_candel, NULL, NULL,
      DOC("->?Dbool\n"
          "Returns :true if @this member can be deleted") },
    { "canset", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&instancemember_get_canset, NULL, NULL,
      DOC("->?Dbool\n"
          "Returns :true if @this member can be written to") },
    { "__name__", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&instancemember_get_name, NULL, NULL,
      DOC("->?Dstring\n"
          "The name of @this instance member") },
    { "__doc__", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&instancemember_get_doc, NULL, NULL,
      DOC("->?Dstring\n"
          "->?N\n"
          "The documentation string associated with @this instance member") },
    { "__module__", (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&instancemember_get_module, NULL, NULL,
      DOC("->?Dmodule\n"
          "->?N\n"
          "Returns the module that is defining @this instance "
          "member, or :none if that module could not be defined") },
    { NULL }
};

PRIVATE struct type_member instancemember_members[] = {
    TYPE_MEMBER_FIELD("__type__",STRUCT_OBJECT,offsetof(DeeInstanceMemberObject,im_type)),
    TYPE_MEMBER_END
};

/* NOTE: Must also hash and compare the type because if the class is
 *       created multiple times, then the member descriptor remains
 *       the same and is shared between all instances. */
PRIVATE dhash_t DCALL
instancemember_hash(DeeInstanceMemberObject *__restrict self) {
 return (Dee_HashPointer(self->im_type) ^
         Dee_HashPointer(self->im_attribute));
}
PRIVATE DREF DeeObject *DCALL
instancemember_eq(DeeInstanceMemberObject *__restrict self,
                  DeeInstanceMemberObject *__restrict other) {
 if (DeeObject_AssertType((DeeObject *)other,&DeeInstanceMember_Type))
     return NULL;
 return_bool_(self->im_type      == other->im_type &&
              self->im_attribute == other->im_attribute);
}

PRIVATE struct type_cmp instancemember_cmp = {
    /* .tp_hash = */(dhash_t(DCALL *)(DeeObject *__restrict))&instancemember_hash,
    /* .tp_eq   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&instancemember_eq
};

PUBLIC DeeTypeObject DeeInstanceMember_Type = {
    OBJECT_HEAD_INIT(&DeeType_Type),
    /* .tp_name     = */"_instancemember",
    /* .tp_doc      = */NULL,
    /* .tp_flags    = */TP_FNORMAL,
    /* .tp_weakrefs = */0,
    /* .tp_features = */TF_NONE,
    /* .tp_base     = */&DeeObject_Type,
    /* .tp_init = */{
        {
            /* .tp_alloc = */{
                /* .tp_ctor      = */NULL,
                /* .tp_copy_ctor = */instancemember_copy,
                /* .tp_deep_ctor = */NULL,
                /* .tp_any_ctor  = */NULL,
                /* .tp_free      = */NULL,
                {
                    /* .tp_instance_size = */sizeof(DeeInstanceMemberObject),
                }
            }
        },
        /* .tp_dtor        = */(void(DCALL *)(DeeObject *__restrict))&instancemember_fini,
        /* .tp_assign      = */NULL,
        /* .tp_move_assign = */NULL
    },
    /* .tp_cast = */{
        /* .tp_str  = */NULL,
        /* .tp_repr = */NULL,
        /* .tp_bool = */NULL
    },
    /* .tp_call          = */NULL,
    /* .tp_visit         = */(void(DCALL *)(DeeObject *__restrict,dvisit_t,void*))&instancemember_visit,
    /* .tp_gc            = */NULL,
    /* .tp_math          = */NULL,
    /* .tp_cmp           = */&instancemember_cmp,
    /* .tp_seq           = */NULL,
    /* .tp_iter_next     = */NULL,
    /* .tp_attr          = */NULL,
    /* .tp_with          = */NULL,
    /* .tp_buffer        = */NULL,
    /* .tp_methods       = */instancemember_methods,
    /* .tp_getsets       = */instancemember_getsets,
    /* .tp_members       = */instancemember_members,
    /* .tp_class_methods = */NULL,
    /* .tp_class_getsets = */NULL,
    /* .tp_class_members = */NULL,
    /* .tp_call_kw       = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict,DeeObject*))&instancemember_get
};


PUBLIC DREF DeeObject *DCALL
DeeInstanceMember_New(DeeTypeObject *__restrict class_type,
                      struct class_attribute *__restrict attribute) {
 DREF DeeInstanceMemberObject *result;
 ASSERT_OBJECT_TYPE(class_type,&DeeType_Type);
 ASSERT(DeeType_IsClass(class_type));
 ASSERT(attribute);
 result = DeeObject_MALLOC(DeeInstanceMemberObject);
 if unlikely(!result) goto done;
 result->im_type      = class_type;
 result->im_attribute = attribute;
 Dee_Incref(class_type);
 DeeObject_Init(result,&DeeInstanceMember_Type);
done:
 return (DREF DeeObject *)result;
}


INTERN dssize_t DCALL
DeeClass_EnumClassInstanceAttributes(DeeTypeObject *__restrict self,
                                     denum_t proc, void *arg) {
 dssize_t temp,result = 0; size_t i;
 struct class_desc *my_class = DeeClass_DESC(self);
 DeeClassDescriptorObject *desc = my_class->cd_desc;
 for (i = 0; i <= desc->cd_iattr_mask; ++i) {
  struct class_attribute *attr;
  DREF DeeTypeObject *attr_type; uint16_t perm;
  attr = &desc->cd_iattr_list[i];
  if (!attr->ca_name) continue;
  attr_type = NULL;
  perm = (ATTR_IMEMBER | ATTR_CMEMBER | ATTR_WRAPPER |
          ATTR_PERMGET | ATTR_NAMEOBJ);
  if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET)
      perm |= ATTR_PROPERTY;
  else if (attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD)
      perm |= ATTR_PERMCALL;
  if (attr->ca_flag & CLASS_ATTRIBUTE_FPRIVATE)
      perm |= ATTR_PRIVATE;
  if (attr->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM) {
   rwlock_read(&my_class->cd_lock);
   if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
    /* Special case: Figure out what property callbacks have been assigned. */
    perm &= ~ATTR_PERMGET;
    if (my_class->cd_members[attr->ca_addr + CLASS_GETSET_GET])
        perm |= ATTR_PERMGET;
    if (!(attr->ca_flag & CLASS_ATTRIBUTE_FREADONLY)) {
     if (my_class->cd_members[attr->ca_addr + CLASS_GETSET_DEL])
         perm |= ATTR_PERMDEL;
     if (my_class->cd_members[attr->ca_addr + CLASS_GETSET_SET])
         perm |= ATTR_PERMSET;
    }
   } else {
    DeeObject *obj;
    if (!(attr->ca_flag & CLASS_ATTRIBUTE_FREADONLY))
        perm |= (ATTR_PERMDEL | ATTR_PERMSET);
    obj = my_class->cd_members[attr->ca_addr];
    if (obj) {
     attr_type = Dee_TYPE(obj);
     Dee_Incref(attr_type);
    }
   }
   rwlock_endread(&my_class->cd_lock);
  } else {
   if (!(attr->ca_flag & (CLASS_ATTRIBUTE_FGETSET | CLASS_ATTRIBUTE_FREADONLY)))
        perm |= (ATTR_PERMDEL | ATTR_PERMSET);
  }
  if (attr->ca_doc) perm |= ATTR_DOCOBJ;
  temp = (*proc)((DeeObject *)self,DeeString_STR(attr->ca_name),
                  attr->ca_doc ? DeeString_STR(attr->ca_doc) : NULL,
                  perm,attr_type,arg);
  Dee_XDecref(attr_type);
  if unlikely(temp < 0) return temp;
  result += temp;
 }
 return result;
}
INTERN dssize_t DCALL
DeeClass_EnumClassAttributes(DeeTypeObject *__restrict self,
                             denum_t proc, void *arg) {
 dssize_t temp,result = 0; size_t i;
 struct class_desc *my_class = DeeClass_DESC(self);
 DeeClassDescriptorObject *desc = my_class->cd_desc;
 for (i = 0; i <= desc->cd_cattr_mask; ++i) {
  struct class_attribute *attr; uint16_t perm;
  DREF DeeTypeObject *attr_type;
  attr = &desc->cd_cattr_list[i];
  if (!attr->ca_name) continue;
  attr_type = NULL;
  perm = (ATTR_CMEMBER | ATTR_PERMGET | ATTR_NAMEOBJ);
  /* Figure out which instance descriptor the property is connected to. */
  rwlock_read(&my_class->cd_lock);
  if (!(attr->ca_flag & CLASS_ATTRIBUTE_FGETSET)) {
   /* Actually figure out the type of the attr. */
   attr_type = (DREF DeeTypeObject *)my_class->cd_members[attr->ca_addr];
   if (attr_type) {
    attr_type = Dee_TYPE(attr_type);
    Dee_Incref(attr_type);
   }
  }
  if (!(attr->ca_flag & CLASS_ATTRIBUTE_FREADONLY)) {
   perm |= (ATTR_PERMDEL | ATTR_PERMSET);
   if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
    perm = (ATTR_CMEMBER | ATTR_NAMEOBJ);
    /* Actually figure out what callbacks are assigned. */
    if (my_class->cd_members[attr->ca_addr + CLASS_GETSET_GET]) perm |= ATTR_PERMGET;
    if (my_class->cd_members[attr->ca_addr + CLASS_GETSET_DEL]) perm |= ATTR_PERMDEL;
    if (my_class->cd_members[attr->ca_addr + CLASS_GETSET_SET]) perm |= ATTR_PERMSET;
   }
  }
  rwlock_endread(&my_class->cd_lock);
  if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET)
      perm |= ATTR_PROPERTY;
  else if (attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD)
      perm |= ATTR_PERMCALL;
  if (attr->ca_flag & CLASS_ATTRIBUTE_FPRIVATE)
      perm |= ATTR_PRIVATE;
  if (attr->ca_doc) perm |= ATTR_DOCOBJ;
  temp = (*proc)((DeeObject *)self,DeeString_STR(attr->ca_name),
                  attr->ca_doc ? DeeString_STR(attr->ca_doc) : NULL,
                  perm,attr_type,arg);
  Dee_XDecref(attr_type);
  if unlikely(temp < 0) return temp;
  result += temp;
 }
 return result;
}
INTERN dssize_t DCALL
DeeClass_EnumInstanceAttributes(DeeTypeObject *__restrict self,
                                DeeObject *instance,
                                denum_t proc, void *arg) {
 dssize_t temp,result = 0; size_t i;
 struct class_desc *my_class = DeeClass_DESC(self);
 DeeClassDescriptorObject *desc = my_class->cd_desc;
 for (i = 0; i <= desc->cd_iattr_mask; ++i) {
  struct class_attribute *attr; uint16_t perm;
  DREF DeeTypeObject *attr_type;
  struct instance_desc *inst;
  attr = &desc->cd_iattr_list[i];
  if (!attr->ca_name) continue;
  inst = NULL,attr_type = NULL;
  perm = (ATTR_IMEMBER | ATTR_PERMGET | ATTR_NAMEOBJ);
  /* Figure out which instance descriptor the property is connected to. */
  if (attr->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM)
      inst = class_desc_as_instance(my_class);
  else if (instance)
      inst = DeeInstance_DESC(my_class,instance);
  if (inst) rwlock_read(&inst->id_lock);
  if (inst && !(attr->ca_flag & CLASS_ATTRIBUTE_FGETSET)) {
   /* Actually figure out the type of the attr. */
   attr_type = (DREF DeeTypeObject *)inst->id_vtab[attr->ca_addr];
   if (attr_type) {
    attr_type = Dee_TYPE(attr_type);
    Dee_Incref(attr_type);
   } else {
    perm &= ~ATTR_PERMGET;
   }
  }
  if (!(attr->ca_flag & CLASS_ATTRIBUTE_FREADONLY)) {
   perm |= (ATTR_PERMDEL | ATTR_PERMSET);
   if (inst && attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
    perm = (ATTR_IMEMBER | ATTR_NAMEOBJ);
    /* Actually figure out what callbacks are assigned. */
    if (inst->id_vtab[attr->ca_addr + CLASS_GETSET_GET]) perm |= ATTR_PERMGET;
    if (inst->id_vtab[attr->ca_addr + CLASS_GETSET_DEL]) perm |= ATTR_PERMDEL;
    if (inst->id_vtab[attr->ca_addr + CLASS_GETSET_SET]) perm |= ATTR_PERMSET;
   }
  }
  if (inst) rwlock_endread(&inst->id_lock);
  if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET)
      perm |= ATTR_PROPERTY;
  else if (attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD)
      perm |= ATTR_PERMCALL;
  if (attr->ca_flag & CLASS_ATTRIBUTE_FPRIVATE)
      perm |= ATTR_PRIVATE;
  if (attr->ca_doc) perm |= ATTR_DOCOBJ;
  temp = (*proc)((DeeObject *)self,DeeString_STR(attr->ca_name),
                  attr->ca_doc ? DeeString_STR(attr->ca_doc) : NULL,
                  perm,attr_type,arg);
  Dee_XDecref(attr_type);
  if unlikely(temp < 0) return temp;
  result += temp;
 }
 return result;
}


/* Find a specific class-attribute, matching the given lookup rules.
 * @return:  0: Attribute found (*result was filled with data).
 * @return:  1: No attribute matching the given requirements was found.
 * @return: -1: An error occurred. */
INTERN int DCALL
DeeClass_FindClassAttribute(DeeTypeObject *__restrict tp_invoker,
                            DeeTypeObject *__restrict self,
                            struct attribute_info *__restrict result,
                            struct attribute_lookup_rules const *__restrict rules) {
 struct class_attribute *attr;
 struct class_desc *my_class = DeeClass_DESC(self);
 uint16_t perm; DREF DeeTypeObject *attr_type;
 attr = DeeType_QueryClassAttributeStringWithHash(tp_invoker,self,
                                                  rules->alr_name,
                                                  rules->alr_hash);
 if (!attr) goto not_found;
 attr_type = NULL;
 perm = (ATTR_CMEMBER | ATTR_PERMGET | ATTR_NAMEOBJ);
 /* Figure out which instance descriptor the property is connected to. */
 rwlock_read(&my_class->cd_lock);
 if (!(attr->ca_flag & CLASS_ATTRIBUTE_FGETSET)) {
  /* Actually figure out the type of the attr. */
  attr_type = (DREF DeeTypeObject *)my_class->cd_members[attr->ca_addr];
  if (attr_type) { attr_type = Dee_TYPE(attr_type); Dee_Incref(attr_type); }
 }
 if (!(attr->ca_flag & CLASS_ATTRIBUTE_FREADONLY)) {
  perm |= (ATTR_PERMDEL | ATTR_PERMSET);
  if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
   perm = (ATTR_CMEMBER | ATTR_NAMEOBJ);
   /* Actually figure out what callbacks are assigned. */
   if (my_class->cd_members[attr->ca_addr + CLASS_GETSET_GET]) perm |= ATTR_PERMGET;
   if (my_class->cd_members[attr->ca_addr + CLASS_GETSET_DEL]) perm |= ATTR_PERMDEL;
   if (my_class->cd_members[attr->ca_addr + CLASS_GETSET_SET]) perm |= ATTR_PERMSET;
  }
 }
 rwlock_endread(&my_class->cd_lock);
 if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET)
     perm |= ATTR_PROPERTY;
 else if (attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD)
     perm |= ATTR_PERMCALL;
 if (attr->ca_flag & CLASS_ATTRIBUTE_FPRIVATE)
     perm |= ATTR_PRIVATE;
 if ((perm & rules->alr_perm_mask) != rules->alr_perm_value) {
  Dee_XDecref(attr_type);
not_found:
  return 1;
 }
 result->a_doc      = NULL;
 result->a_decl     = (DREF DeeObject *)self;
 result->a_attrtype = attr_type; /* Inherit reference. */
 if (attr->ca_doc) {
  result->a_doc = DeeString_STR(attr->ca_doc);
  perm         |= ATTR_DOCOBJ;
  Dee_Incref(attr->ca_doc);
 }
 result->a_perm = perm;
 Dee_Incref(result->a_decl);
 return 0;
}


/* Find a specific instance-through-class-attribute, matching the given lookup rules.
 * @return:  0: Attribute found (*result was filled with data).
 * @return:  1: No attribute matching the given requirements was found.
 * @return: -1: An error occurred. */
INTERN int DCALL
DeeClass_FindClassInstanceAttribute(DeeTypeObject *__restrict tp_invoker,
                                    DeeTypeObject *__restrict self,
                                    struct attribute_info *__restrict result,
                                    struct attribute_lookup_rules const *__restrict rules) {
 struct class_attribute *attr;
 struct class_desc *my_class = DeeClass_DESC(self);
 uint16_t perm; DREF DeeTypeObject *attr_type;
 attr = DeeType_QueryInstanceAttributeStringWithHash(tp_invoker,self,
                                                     rules->alr_name,
                                                     rules->alr_hash);
 if (!attr) goto not_found;
 attr_type = NULL;
 perm = (ATTR_IMEMBER | ATTR_CMEMBER |
         ATTR_WRAPPER | ATTR_PERMGET |
         ATTR_NAMEOBJ);
 if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET)
     perm |= ATTR_PROPERTY;
 else if (attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD)
     perm |= ATTR_PERMCALL;
 if (attr->ca_flag & CLASS_ATTRIBUTE_FPRIVATE)
     perm |= ATTR_PRIVATE;
 if (attr->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM) {
  rwlock_read(&my_class->cd_lock);
  if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
   /* Special case: Figure out what property callbacks have been assigned. */
   perm &= ~ATTR_PERMGET;
   if (my_class->cd_members[attr->ca_addr + CLASS_GETSET_GET])
       perm |= ATTR_PERMGET;
   if (!(attr->ca_flag & CLASS_ATTRIBUTE_FREADONLY)) {
    if (my_class->cd_members[attr->ca_addr + CLASS_GETSET_DEL])
        perm |= ATTR_PERMDEL;
    if (my_class->cd_members[attr->ca_addr + CLASS_GETSET_SET])
        perm |= ATTR_PERMSET;
   }
  } else {
   DeeObject *obj;
   if (!(attr->ca_flag & CLASS_ATTRIBUTE_FREADONLY))
       perm |= (ATTR_PERMDEL | ATTR_PERMSET);
   obj = my_class->cd_members[attr->ca_addr];
   if (obj) {
    attr_type = Dee_TYPE(obj);
    Dee_Incref(attr_type);
   }
  }
  rwlock_endread(&my_class->cd_lock);
 } else {
  if (!(attr->ca_flag & (CLASS_ATTRIBUTE_FGETSET | CLASS_ATTRIBUTE_FREADONLY)))
      perm |= (ATTR_PERMDEL | ATTR_PERMSET);
 }
 if ((perm & rules->alr_perm_mask) != rules->alr_perm_value) {
  Dee_XDecref(attr_type);
not_found:
  return 1;
 }
 result->a_decl     = (DREF DeeObject *)self;
 result->a_attrtype = attr_type; /* Inherit reference. */
 result->a_doc      = NULL;
 if (attr->ca_doc) {
  result->a_doc = DeeString_STR(attr->ca_doc);
  perm         |= ATTR_DOCOBJ;
  Dee_Incref(attr->ca_doc);
 }
 result->a_perm = perm;
 Dee_Incref(result->a_decl);
 return 0;
}

/* Find a specific instance-attribute, matching the given lookup rules.
 * @return:  0: Attribute found (*result was filled with data).
 * @return:  1: No attribute matching the given requirements was found.
 * @return: -1: An error occurred. */
INTERN int DCALL
DeeClass_FindInstanceAttribute(DeeTypeObject *__restrict tp_invoker,
                               DeeTypeObject *__restrict self,
                               DeeObject *instance,
                               struct attribute_info *__restrict result,
                               struct attribute_lookup_rules const *__restrict rules) {
 struct class_attribute *attr; struct instance_desc *inst;
 struct class_desc *my_class = DeeClass_DESC(self);
 uint16_t perm; DREF DeeTypeObject *attr_type;
 attr = DeeType_QueryAttributeStringWithHash(tp_invoker,self,
                                             rules->alr_name,
                                             rules->alr_hash);
 if (!attr) goto not_found;
 attr_type = NULL,inst = NULL;
 perm = (ATTR_IMEMBER | ATTR_PERMGET | ATTR_NAMEOBJ);
 /* Figure out which instance descriptor the property is connected to. */
 if (attr->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM)
     inst = class_desc_as_instance(my_class);
 else if (instance)
     inst = DeeInstance_DESC(my_class,instance);
 if (inst) rwlock_read(&inst->id_lock);
 if (inst && !(attr->ca_flag & CLASS_ATTRIBUTE_FGETSET)) {
  /* Actually figure out the type of the attr. */
  attr_type = (DREF DeeTypeObject *)inst->id_vtab[attr->ca_addr];
  if (attr_type) {
   attr_type = Dee_TYPE(attr_type);
   Dee_Incref(attr_type);
  } else {
   perm &= ~ATTR_PERMGET;
  }
 }
 if (!(attr->ca_flag & CLASS_ATTRIBUTE_FREADONLY)) {
  perm |= (ATTR_PERMDEL | ATTR_PERMSET);
  if (inst && attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
   perm = (ATTR_IMEMBER | ATTR_NAMEOBJ);
   /* Actually figure out what callbacks are assigned. */
   if (inst->id_vtab[attr->ca_addr + CLASS_GETSET_GET]) perm |= ATTR_PERMGET;
   if (inst->id_vtab[attr->ca_addr + CLASS_GETSET_DEL]) perm |= ATTR_PERMDEL;
   if (inst->id_vtab[attr->ca_addr + CLASS_GETSET_SET]) perm |= ATTR_PERMSET;
  }
 }
 if (inst) rwlock_endread(&inst->id_lock);
 if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET)
     perm |= ATTR_PROPERTY;
 else if (attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD)
     perm |= ATTR_PERMCALL;
 if (attr->ca_flag & CLASS_ATTRIBUTE_FPRIVATE)
     perm |= ATTR_PRIVATE;
 if ((perm & rules->alr_perm_mask) != rules->alr_perm_value) {
  Dee_XDecref(attr_type);
not_found:
  return 1;
 }
 result->a_decl     = (DREF DeeObject *)self;
 result->a_attrtype = attr_type; /* Inherit reference. */
 result->a_doc      = NULL;
 if (attr->ca_doc) {
  result->a_doc = DeeString_STR(attr->ca_doc);
  perm         |= ATTR_DOCOBJ;
  Dee_Incref(attr->ca_doc);
 }
 result->a_perm = perm;
 Dee_Incref(result->a_decl);
 return 0;
}




INTERN DREF DeeObject *DCALL
DeeClass_GetInstanceAttribute(DeeTypeObject *__restrict class_type,
                              struct class_attribute *__restrict attr) {
 DREF DeePropertyObject *result;
 struct class_desc *my_class;
 /* Return an instance-wrapper for instance-members. */
 if (!(attr->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM))
     return DeeInstanceMember_New(class_type,attr);
 my_class = DeeClass_DESC(class_type);
 if (!(attr->ca_flag & CLASS_ATTRIBUTE_FGETSET)) {
  DREF DeeObject *member_value;
  /* Simple case: direct access to unbound class-based attr. */
  rwlock_read(&my_class->cd_lock);
  member_value = my_class->cd_members[attr->ca_addr];
  Dee_XIncref(member_value);
  rwlock_endread(&my_class->cd_lock);
  if unlikely(!member_value) goto unbound;
  return member_value;
 }
 result = DeeObject_MALLOC(DeePropertyObject);
 if unlikely(!result) return NULL;
 result->p_del = NULL;
 result->p_set = NULL;
 rwlock_read(&my_class->cd_lock);
 result->p_get = my_class->cd_members[attr->ca_addr + CLASS_GETSET_GET];
 Dee_XIncref(result->p_get);
 /* Only non-readonly property members have callbacks other than a getter.
  * In this case, the readonly flag both protects the property from being
  * overwritten, as well as being invoked using something other than read-access. */
 if (!(attr->ca_flag & CLASS_ATTRIBUTE_FREADONLY)) {
  result->p_del = my_class->cd_members[attr->ca_addr + CLASS_GETSET_DEL];
  Dee_XIncref(result->p_del);
  result->p_set = my_class->cd_members[attr->ca_addr + CLASS_GETSET_SET];
  Dee_XIncref(result->p_set);
 }
 rwlock_endread(&my_class->cd_lock);
 /* Make sure that at least a single property callback has been assigned.
  * If not, raise an unbound-attr error. */
 if (!result->p_get && !result->p_del && !result->p_set) {
  DeeObject_Free(result);
  goto unbound;
 }
 /* Finalize initialization of the property wrapper and return it. */
 DeeObject_Init(result,&DeeProperty_Type);
 return (DREF DeeObject *)result;
unbound:
 err_unbound_attribute(class_type,
                       DeeString_STR(attr->ca_name));
 return NULL;
}
INTERN int DCALL
DeeClass_BoundInstanceAttribute(DeeTypeObject *__restrict class_type,
                                struct class_attribute *__restrict attr) {
 int result;
 struct class_desc *my_class;
 /* Return an instance-wrapper for instance-members. */
 if (!(attr->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM))
     return 1; /* instance-members outside of class memory are
                * accessed through wrappers, which are always bound. */
 my_class = DeeClass_DESC(class_type);
 /* Check if the member is assigned. */
 rwlock_read(&my_class->cd_lock);
 if ((attr->ca_flag & (CLASS_ATTRIBUTE_FGETSET|CLASS_ATTRIBUTE_FREADONLY)) ==
                       CLASS_ATTRIBUTE_FGETSET) {
  result = ((my_class->cd_members[attr->ca_addr + CLASS_GETSET_GET] != NULL) ||
            (my_class->cd_members[attr->ca_addr + CLASS_GETSET_DEL] != NULL) ||
            (my_class->cd_members[attr->ca_addr + CLASS_GETSET_SET] != NULL));
 } else {
  result = my_class->cd_members[attr->ca_addr] != NULL;
 }
 rwlock_endread(&my_class->cd_lock);
 return result;
}
INTERN DREF DeeObject *DCALL
DeeClass_CallInstanceAttribute(DeeTypeObject *__restrict class_type,
                               struct class_attribute *__restrict attr,
                               size_t argc, DeeObject **__restrict argv) {
 DREF DeeObject *callback,*result;
 struct class_desc *my_class = DeeClass_DESC(class_type);
 if (!(attr->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM)) {
  /* Simulate `operator ()' for wrappers generated by `instancemember_wrapper()'. */
  if (argc != 1) {
   DeeError_Throwf(&DeeError_TypeError,
                   "instancemember `%k' must be called with exactly 1 argument",
                   attr->ca_name);
   goto err;
  }
  if (DeeObject_AssertType(argv[0],class_type))
      goto err;
  return DeeInstance_GetAttribute(my_class,
                                  DeeInstance_DESC(my_class,argv[0]),
                                  argv[0],
                                  attr);
 }
 /* Simple case: direct access to unbound class-based attr. */
#if 0
 if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
  /* Calling an instance property using the class as base
   * will simply invoke the getter associated with that property.
   * Technically, we could assert that `argc == 1' at this point,
   * as well as that `argv[0] is class_type', but there is no
   * need for us to do this, as the callback that's going to be
   * invoked will perform those same check (should that guaranty
   * become relevant), because it's yet another object over which
   * the user has full control. */
 }
#endif
 rwlock_read(&my_class->cd_lock);
#if CLASS_GETSET_GET != 0
 if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
  callback = my_class->cd_members[attr->ca_addr + CLASS_GETSET_GET];
 } else {
  callback = my_class->cd_members[attr->ca_addr];
 }
#else
 callback = my_class->cd_members[attr->ca_addr];
#endif
 Dee_XIncref(callback);
 rwlock_endread(&my_class->cd_lock);
 if unlikely(!callback) goto unbound;
 /* Invoke the callback. */
 result = DeeObject_Call(callback,argc,argv);
 Dee_Decref(callback);
 return result;
unbound:
 err_unbound_attribute(class_type,
                       DeeString_STR(attr->ca_name));
err:
 return NULL;
}

INTERN DREF DeeObject *DCALL
DeeClass_CallInstanceAttributeKw(DeeTypeObject *__restrict class_type,
                                 struct class_attribute *__restrict attr,
                                 size_t argc, DeeObject **__restrict argv,
                                 DeeObject *kw) {
 DREF DeeObject *callback,*result;
 struct class_desc *my_class = DeeClass_DESC(class_type);
 if (!(attr->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM)) {
  /* Simulate `operator ()' for wrappers generated by `instancemember_wrapper()'. */
  DeeObject *thisarg;
  if (DeeArg_UnpackKw(argc,argv,kw,thisarg_kwlist,"o:get",&thisarg) ||
      DeeObject_AssertType(thisarg,class_type))
      goto err;
  return DeeInstance_GetAttribute(my_class,
                                  DeeInstance_DESC(my_class,thisarg),
                                  thisarg,
                                  attr);
 }
 /* Simple case: direct access to unbound class-based attr. */
#if 0
 if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
  /* Calling an instance property using the class as base
   * will simply invoke the getter associated with that property.
   * Technically, we could assert that `argc == 1' at this point,
   * as well as that `argv[0] is class_type', but there is no
   * need for us to do this, as the callback that's going to be
   * invoked will perform those same check (should that guaranty
   * become relevant), because it's yet another object over which
   * the user has full control. */
 }
#endif
 rwlock_read(&my_class->cd_lock);
#if CLASS_GETSET_GET != 0
 if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
  callback = my_class->cd_members[attr->ca_addr + CLASS_GETSET_GET];
 } else {
  callback = my_class->cd_members[attr->ca_addr];
 }
#else
 callback = my_class->cd_members[attr->ca_addr];
#endif
 Dee_XIncref(callback);
 rwlock_endread(&my_class->cd_lock);
 if unlikely(!callback) goto unbound;
 /* Invoke the callback. */
 result = DeeObject_CallKw(callback,argc,argv,kw);
 Dee_Decref(callback);
 return result;
unbound:
 err_unbound_attribute(class_type,
                       DeeString_STR(attr->ca_name));
err:
 return NULL;
}

#ifdef CONFIG_HAVE_CALLTUPLE_OPTIMIZATIONS
INTERN DREF DeeObject *DCALL
DeeClass_CallInstanceAttributeTuple(DeeTypeObject *__restrict class_type,
                                    struct class_attribute *__restrict attr,
                                    DeeObject *__restrict args) {
 DREF DeeObject *callback,*result;
 struct class_desc *my_class = DeeClass_DESC(class_type);
 if (!(attr->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM)) {
  /* Simulate `operator ()' for wrappers generated by `instancemember_wrapper()'. */
  if (DeeTuple_SIZE(args) != 1) {
   DeeError_Throwf(&DeeError_TypeError,
                   "instancemember `%k' must be called with exactly 1 argument",
                   attr->ca_name);
   goto err;
  }
  if (DeeObject_AssertType(DeeTuple_GET(args,0),class_type))
      goto err;
  return DeeInstance_GetAttribute(my_class,
                                  DeeInstance_DESC(my_class,DeeTuple_GET(args,0)),
                                  DeeTuple_GET(args,0),
                                  attr);
 }
 /* Simple case: direct access to unbound class-based attr. */
#if 0
 if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
  /* Calling an instance property using the class as base
   * will simply invoke the getter associated with that property.
   * Technically, we could assert that `argc == 1' at this point,
   * as well as that `DeeTuple_GET(args,0) is class_type', but there is no
   * need for us to do this, as the callback that's going to be
   * invoked will perform those same check (should that guaranty
   * become relevant), because it's yet another object over which
   * the user has full control. */
 }
#endif
 rwlock_read(&my_class->cd_lock);
#if CLASS_GETSET_GET != 0
 if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
  callback = my_class->cd_members[attr->ca_addr + CLASS_GETSET_GET];
 } else {
  callback = my_class->cd_members[attr->ca_addr];
 }
#else
 callback = my_class->cd_members[attr->ca_addr];
#endif
 Dee_XIncref(callback);
 rwlock_endread(&my_class->cd_lock);
 if unlikely(!callback) goto unbound;
 /* Invoke the callback. */
 result = DeeObject_CallTuple(callback,args);
 Dee_Decref(callback);
 return result;
unbound:
 err_unbound_attribute(class_type,
                       DeeString_STR(attr->ca_name));
err:
 return NULL;
}

INTERN DREF DeeObject *DCALL
DeeClass_CallInstanceAttributeTupleKw(DeeTypeObject *__restrict class_type,
                                      struct class_attribute *__restrict attr,
                                      DeeObject *__restrict args, DeeObject *kw) {
 DREF DeeObject *callback,*result;
 struct class_desc *my_class = DeeClass_DESC(class_type);
 if (!(attr->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM)) {
  /* Simulate `operator ()' for wrappers generated by `instancemember_wrapper()'. */
  DeeObject *thisarg;
  if (DeeArg_UnpackKw(DeeTuple_SIZE(args),DeeTuple_ELEM(args),kw,thisarg_kwlist,"o:get",&thisarg) ||
      DeeObject_AssertType(thisarg,class_type))
      goto err;
  return DeeInstance_GetAttribute(my_class,
                                  DeeInstance_DESC(my_class,thisarg),
                                  thisarg,
                                  attr);
 }
 /* Simple case: direct access to unbound class-based attr. */
#if 0
 if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
  /* Calling an instance property using the class as base
   * will simply invoke the getter associated with that property.
   * Technically, we could assert that `argc == 1' at this point,
   * as well as that `DeeTuple_GET(args,0) is class_type', but there is no
   * need for us to do this, as the callback that's going to be
   * invoked will perform those same check (should that guaranty
   * become relevant), because it's yet another object over which
   * the user has full control. */
 }
#endif
 rwlock_read(&my_class->cd_lock);
#if CLASS_GETSET_GET != 0
 if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
  callback = my_class->cd_members[attr->ca_addr + CLASS_GETSET_GET];
 } else {
  callback = my_class->cd_members[attr->ca_addr];
 }
#else
 callback = my_class->cd_members[attr->ca_addr];
#endif
 Dee_XIncref(callback);
 rwlock_endread(&my_class->cd_lock);
 if unlikely(!callback) goto unbound;
 /* Invoke the callback. */
 result = DeeObject_CallTupleKw(callback,args,kw);
 Dee_Decref(callback);
 return result;
unbound:
 err_unbound_attribute(class_type,
                       DeeString_STR(attr->ca_name));
err:
 return NULL;
}
#endif /* CONFIG_HAVE_CALLTUPLE_OPTIMIZATIONS */

INTERN DREF DeeObject *DCALL
DeeClass_VCallInstanceAttributef(DeeTypeObject *__restrict class_type,
                                 struct class_attribute *__restrict attr,
                                 char const *__restrict format, va_list args) {
 DREF DeeObject *callback,*result,*args_tuple;
 struct class_desc *my_class = DeeClass_DESC(class_type);
 if (!(attr->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM)) {
  /* Simulate `operator ()' for wrappers generated by `instancemember_wrapper()'. */
  DeeObject *thisarg;
  args_tuple = DeeTuple_VNewf(format,args);
  if unlikely(!args_tuple) goto err;
  if (DeeArg_Unpack(DeeTuple_SIZE(args_tuple),
                    DeeTuple_ELEM(args_tuple),
                    "o:get",&thisarg))
      goto err_args_tuple;
  if (DeeObject_AssertType(thisarg,class_type))
      goto err_args_tuple;
  result = DeeInstance_GetAttribute(my_class,
                                    DeeInstance_DESC(my_class,thisarg),
                                    thisarg,
                                    attr);
  Dee_Decref(args_tuple);
  return result;
 }
 /* Simple case: direct access to unbound class-based attr. */
#if 0
 if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
  /* Calling an instance property using the class as base
   * will simply invoke the getter associated with that property.
   * Technically, we could assert that `argc == 1' at this point,
   * as well as that `argv[0] is class_type', but there is no
   * need for us to do this, as the callback that's going to be
   * invoked will perform those same check (should that guaranty
   * become relevant), because it's yet another object over which
   * the user has full control. */
 }
#endif
 rwlock_read(&my_class->cd_lock);
#if CLASS_GETSET_GET != 0
 if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
  callback = my_class->cd_members[attr->ca_addr + CLASS_GETSET_GET];
 } else {
  callback = my_class->cd_members[attr->ca_addr];
 }
#else
 callback = my_class->cd_members[attr->ca_addr];
#endif
 Dee_XIncref(callback);
 rwlock_endread(&my_class->cd_lock);
 if unlikely(!callback) goto unbound;
 /* Invoke the callback. */
 result = DeeObject_VCallf(callback,format,args);
 Dee_Decref(callback);
 return result;
unbound:
 err_unbound_attribute(class_type,
                       DeeString_STR(attr->ca_name));
 goto err;
err_args_tuple:
 Dee_Decref(args_tuple);
err:
 return NULL;
}
INTERN int DCALL
DeeClass_DelInstanceAttribute(DeeTypeObject *__restrict class_type,
                              struct class_attribute *__restrict attr) {
 struct class_desc *my_class = DeeClass_DESC(class_type);
 if unlikely(!(attr->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM))
    goto err_noaccess;
 /* Make sure not to re-write readonly attributes. */
 if (attr->ca_flag & CLASS_ATTRIBUTE_FREADONLY) {
  return err_cant_access_attribute(class_type,
                                   DeeString_STR(attr->ca_name),
                                   ATTR_ACCESS_DEL);
 }
 if (!(attr->ca_flag & CLASS_ATTRIBUTE_FGETSET)) {
  DREF DeeObject *old_value;
  /* Simple case: directly delete a class-based attr. */
  rwlock_write(&my_class->cd_lock);
  old_value = my_class->cd_members[attr->ca_addr];
  my_class->cd_members[attr->ca_addr] = NULL;
  rwlock_endwrite(&my_class->cd_lock);
#ifdef CONFIG_ERROR_DELETE_UNBOUND
  if unlikely(!old_value) goto unbound;
  Dee_Decref(old_value);
#else /* CONFIG_ERROR_DELETE_UNBOUND */
  Dee_XDecref(old_value);
#endif /* !CONFIG_ERROR_DELETE_UNBOUND */
 } else {
  /* Property callbacks (delete 3 bindings, rather than 1) */
  DREF DeeObject *old_value[3];
  rwlock_write(&my_class->cd_lock);
  old_value[0] = my_class->cd_members[attr->ca_addr + CLASS_GETSET_GET];
  old_value[1] = my_class->cd_members[attr->ca_addr + CLASS_GETSET_DEL];
  old_value[2] = my_class->cd_members[attr->ca_addr + CLASS_GETSET_SET];
  my_class->cd_members[attr->ca_addr + CLASS_GETSET_GET] = NULL;
  my_class->cd_members[attr->ca_addr + CLASS_GETSET_DEL] = NULL;
  my_class->cd_members[attr->ca_addr + CLASS_GETSET_SET] = NULL;
  rwlock_endwrite(&my_class->cd_lock);
#ifdef CONFIG_ERROR_DELETE_UNBOUND
  /* Only thrown an unbound-error when none of the callbacks were assigned. */
  if unlikely(!old_value[0] &&
              !old_value[1] &&
              !old_value[2])
     goto unbound;
#endif /* CONFIG_ERROR_DELETE_UNBOUND */
  Dee_XDecref(old_value[2]);
  Dee_XDecref(old_value[1]);
  Dee_XDecref(old_value[0]);
 }
 return 0;
#ifdef CONFIG_ERROR_DELETE_UNBOUND
unbound:
 return err_unbound_attribute(class_type,DeeString_STR(attr->ca_name));
#endif /* CONFIG_ERROR_DELETE_UNBOUND */
err_noaccess:
 return err_cant_access_attribute(class_type,
                                  DeeString_STR(attr->ca_name),
                                  ATTR_ACCESS_DEL);
}
INTERN int DCALL
DeeClass_SetInstanceAttribute(DeeTypeObject *__restrict class_type,
                              struct class_attribute *__restrict attr,
                              DeeObject *__restrict value) {
 struct class_desc *my_class = DeeClass_DESC(class_type);
 if unlikely(!(attr->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM))
    goto err_noaccess;
 /* Make sure not to re-write readonly attributes. */
 if (attr->ca_flag & CLASS_ATTRIBUTE_FREADONLY) {
  return err_cant_access_attribute(class_type,
                                   DeeString_STR(attr->ca_name),
                                   ATTR_ACCESS_SET);
 }
 if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
  DREF DeeObject *old_value[3];
  if (DeeObject_AssertType(value,&DeeProperty_Type))
      return -1;
  /* Unpack and assign a property wrapper.
   * NOTE: Because only properties with the read-only flag can get away
   *       with only a getter VTABLE slot, we can assume that this property
   *       has 3 slots because we're not allowed to override readonly properties. */
  Dee_XIncref(((DeePropertyObject *)value)->p_get);
  Dee_XIncref(((DeePropertyObject *)value)->p_del);
  Dee_XIncref(((DeePropertyObject *)value)->p_set);
  rwlock_write(&my_class->cd_lock);
  old_value[0] = my_class->cd_members[attr->ca_addr + CLASS_GETSET_GET];
  old_value[1] = my_class->cd_members[attr->ca_addr + CLASS_GETSET_DEL];
  old_value[2] = my_class->cd_members[attr->ca_addr + CLASS_GETSET_SET];
  my_class->cd_members[attr->ca_addr + CLASS_GETSET_GET] = ((DeePropertyObject *)value)->p_get;
  my_class->cd_members[attr->ca_addr + CLASS_GETSET_DEL] = ((DeePropertyObject *)value)->p_del;
  my_class->cd_members[attr->ca_addr + CLASS_GETSET_SET] = ((DeePropertyObject *)value)->p_set;
  rwlock_endwrite(&my_class->cd_lock);
  /* Drop references from the old callbacks. */
  Dee_XDecref(old_value[2]);
  Dee_XDecref(old_value[1]);
  Dee_XDecref(old_value[0]);
 } else {
  /* Simple case: direct overwrite an unbound class-based attr. */
  DREF DeeObject *old_value;
  Dee_Incref(value);
  rwlock_write(&my_class->cd_lock);
  old_value = my_class->cd_members[attr->ca_addr];
  my_class->cd_members[attr->ca_addr] = value;
  rwlock_endwrite(&my_class->cd_lock);
  Dee_XDecref(old_value); /* Decref the old value. */
 }
 return 0;
err_noaccess:
 return err_cant_access_attribute(class_type,
                                  DeeString_STR(attr->ca_name),
                                  ATTR_ACCESS_SET);
}




INTERN DREF DeeObject *DCALL
DeeInstance_GetAttribute(struct class_desc *__restrict desc,
                         struct instance_desc *__restrict self,
                         DeeObject *__restrict this_arg,
                         struct class_attribute *__restrict attr) {
 DREF DeeObject *result;
 ASSERT(self);
 ASSERT(attr);
 ASSERT_OBJECT(this_arg);
 if (attr->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM)
     self = class_desc_as_instance(desc);
 if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
  DREF DeeObject *getter;
  rwlock_read(&self->id_lock);
  getter = self->id_vtab[attr->ca_addr + CLASS_GETSET_GET];
  Dee_XIncref(getter);
  rwlock_endread(&self->id_lock);
  if unlikely(!getter) goto illegal;
  /* Invoke the getter. */
  result = (attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD)
         ? DeeObject_ThisCall(getter,this_arg,0,NULL)
         : DeeObject_Call(getter,0,NULL);
  Dee_Decref(getter);
 } else if (attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD) {
  /* Construct a thiscall function. */
  DREF DeeObject *callback;
  rwlock_read(&self->id_lock);
  callback = self->id_vtab[attr->ca_addr];
  Dee_XIncref(callback);
  rwlock_endread(&self->id_lock);
  if unlikely(!callback) goto unbound;
  result = DeeInstanceMethod_New(callback,this_arg);
  Dee_Decref(callback);
 } else {
  /* Simply return the attribute as-is. */
  rwlock_read(&self->id_lock);
  result = self->id_vtab[attr->ca_addr];
  Dee_XIncref(result);
  rwlock_endread(&self->id_lock);
  if unlikely(!result) goto unbound;
 }
 return result;
unbound:
 err_unbound_attribute_c(desc,DeeString_STR(attr->ca_name));
 return NULL;
illegal:
 err_cant_access_attribute_c(desc,
                             DeeString_STR(attr->ca_name),
                             ATTR_ACCESS_GET);
 return NULL;
}
INTERN int DCALL
DeeInstance_BoundAttribute(struct class_desc *__restrict desc,
                           struct instance_desc *__restrict self,
                           DeeObject *__restrict this_arg,
                           struct class_attribute *__restrict attr) {
 DREF DeeObject *result;
 ASSERT(self);
 ASSERT(attr);
 ASSERT_OBJECT(this_arg);
 if (attr->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM)
     self = class_desc_as_instance(desc);
 if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
  DREF DeeObject *getter;
  rwlock_read(&self->id_lock);
  getter = self->id_vtab[attr->ca_addr + CLASS_GETSET_GET];
  Dee_XIncref(getter);
  rwlock_endread(&self->id_lock);
  if unlikely(!getter) goto unbound;
  /* Invoke the getter. */
  result = (attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD)
         ? DeeObject_ThisCall(getter,this_arg,0,NULL)
         : DeeObject_Call(getter,0,NULL);
  Dee_Decref(getter);
  if likely(result) { Dee_Decref(result); return 1; }
  if (CATCH_ATTRIBUTE_ERROR())
      return -3;
  if (DeeError_Catch(&DeeError_UnboundAttribute))
      return 0;
  return -1;
 } else {
  /* Simply return the attribute as-is. */
#ifdef CONFIG_NO_THREADS
  return self->id_vtab[attr->ca_addr] != NULL;
#else
  return ATOMIC_READ(self->id_vtab[attr->ca_addr]) != NULL;
#endif
 }
unbound:
 return 0;
}
INTERN DREF DeeObject *DCALL
DeeInstance_CallAttribute(struct class_desc *__restrict desc,
                          struct instance_desc *__restrict self,
                          DeeObject *__restrict this_arg,
                          struct class_attribute *__restrict attr,
                          size_t argc, DeeObject **__restrict argv) {
 DREF DeeObject *result,*callback;
 ASSERT(self);
 ASSERT(attr);
 ASSERT_OBJECT(this_arg);
 if (attr->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM)
     self = class_desc_as_instance(desc);
 if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
  DREF DeeObject *getter;
  rwlock_read(&self->id_lock);
  getter = self->id_vtab[attr->ca_addr + CLASS_GETSET_GET];
  Dee_XIncref(getter);
  rwlock_endread(&self->id_lock);
  if unlikely(!getter) goto illegal;
  /* Invoke the getter. */
  callback = (attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD)
   ? DeeObject_ThisCall(getter,this_arg,0,NULL)
   : DeeObject_Call(getter,0,NULL);
  Dee_Decref(getter);
  /* Invoke the return value of the getter. */
  if unlikely(!callback) return NULL;
  result = DeeObject_Call(callback,argc,argv);
  Dee_Decref(callback);
 } else {
  /* Call the attr as-is. */
  rwlock_read(&self->id_lock);
  callback = self->id_vtab[attr->ca_addr];
  Dee_XIncref(callback);
  rwlock_endread(&self->id_lock);
  if unlikely(!callback) goto unbound;
  result = (attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD)
   ? DeeObject_ThisCall(callback,this_arg,argc,argv)
   : DeeObject_Call(callback,argc,argv);
  Dee_Decref(callback);
 }
 return result;
unbound:
 err_unbound_attribute_c(desc,
                         DeeString_STR(attr->ca_name));
 return NULL;
illegal:
 err_cant_access_attribute_c(desc,
                             DeeString_STR(attr->ca_name),
                             ATTR_ACCESS_GET);
 return NULL;
}
INTERN DREF DeeObject *DCALL
DeeInstance_VCallAttributef(struct class_desc *__restrict desc,
                            struct instance_desc *__restrict self,
                            DeeObject *__restrict this_arg,
                            struct class_attribute *__restrict attr,
                            char const *__restrict format, va_list args) {
 DREF DeeObject *result,*callback;
 ASSERT(self);
 ASSERT(attr);
 ASSERT_OBJECT(this_arg);
 if (attr->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM)
     self = class_desc_as_instance(desc);
 if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
  DREF DeeObject *getter;
  rwlock_read(&self->id_lock);
  getter = self->id_vtab[attr->ca_addr + CLASS_GETSET_GET];
  Dee_XIncref(getter);
  rwlock_endread(&self->id_lock);
  if unlikely(!getter) goto illegal;
  /* Invoke the getter. */
  callback = (attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD)
   ? DeeObject_ThisCall(getter,this_arg,0,NULL)
   : DeeObject_Call(getter,0,NULL);
  Dee_Decref(getter);
  /* Invoke the return value of the getter. */
  if unlikely(!callback) return NULL;
  result = DeeObject_VCallf(callback,format,args);
  Dee_Decref(callback);
 } else {
  /* Call the attr as-is. */
  rwlock_read(&self->id_lock);
  callback = self->id_vtab[attr->ca_addr];
  Dee_XIncref(callback);
  rwlock_endread(&self->id_lock);
  if unlikely(!callback) goto unbound;
  result = (attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD)
   ? DeeObject_VThisCallf(callback,this_arg,format,args)
   : DeeObject_VCallf(callback,format,args);
  Dee_Decref(callback);
 }
 return result;
unbound:
 err_unbound_attribute_c(desc,
                         DeeString_STR(attr->ca_name));
 return NULL;
illegal:
 err_cant_access_attribute_c(desc,
                             DeeString_STR(attr->ca_name),
                             ATTR_ACCESS_GET);
 return NULL;
}
INTERN DREF DeeObject *DCALL
DeeInstance_CallAttributeKw(struct class_desc *__restrict desc,
                            struct instance_desc *__restrict self,
                            DeeObject *__restrict this_arg,
                            struct class_attribute *__restrict attr,
                            size_t argc, DeeObject **__restrict argv,
                            DeeObject *kw) {
 DREF DeeObject *result,*callback;
 ASSERT(self);
 ASSERT(attr);
 ASSERT_OBJECT(this_arg);
 if (attr->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM)
     self = class_desc_as_instance(desc);
 if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
  DREF DeeObject *getter;
  rwlock_read(&self->id_lock);
  getter = self->id_vtab[attr->ca_addr + CLASS_GETSET_GET];
  Dee_XIncref(getter);
  rwlock_endread(&self->id_lock);
  if unlikely(!getter) goto illegal;
  /* Invoke the getter. */
  callback = (attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD)
   ? DeeObject_ThisCall(getter,this_arg,0,NULL)
   : DeeObject_Call(getter,0,NULL);
  Dee_Decref(getter);
  /* Invoke the return value of the getter. */
  if unlikely(!callback) return NULL;
  result = DeeObject_CallKw(callback,argc,argv,kw);
  Dee_Decref(callback);
 } else {
  /* Call the attr as-is. */
  rwlock_read(&self->id_lock);
  callback = self->id_vtab[attr->ca_addr];
  Dee_XIncref(callback);
  rwlock_endread(&self->id_lock);
  if unlikely(!callback) goto unbound;
  result = (attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD)
   ? DeeObject_ThisCallKw(callback,this_arg,argc,argv,kw)
   : DeeObject_CallKw(callback,argc,argv,kw);
  Dee_Decref(callback);
 }
 return result;
unbound:
 err_unbound_attribute_c(desc,
                         DeeString_STR(attr->ca_name));
 return NULL;
illegal:
 err_cant_access_attribute_c(desc,
                             DeeString_STR(attr->ca_name),
                             ATTR_ACCESS_GET);
 return NULL;
}
#ifdef CONFIG_HAVE_CALLTUPLE_OPTIMIZATIONS
INTERN DREF DeeObject *DCALL
DeeInstance_CallAttributeTuple(struct class_desc *__restrict desc,
                               struct instance_desc *__restrict self,
                               DeeObject *__restrict this_arg,
                               struct class_attribute *__restrict attr,
                               DeeObject *__restrict args) {
 DREF DeeObject *result,*callback;
 ASSERT(self);
 ASSERT(attr);
 ASSERT_OBJECT(this_arg);
 if (attr->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM)
     self = class_desc_as_instance(desc);
 if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
  DREF DeeObject *getter;
  rwlock_read(&self->id_lock);
  getter = self->id_vtab[attr->ca_addr + CLASS_GETSET_GET];
  Dee_XIncref(getter);
  rwlock_endread(&self->id_lock);
  if unlikely(!getter) goto illegal;
  /* Invoke the getter. */
  callback = (attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD)
   ? DeeObject_ThisCall(getter,this_arg,0,NULL)
   : DeeObject_Call(getter,0,NULL);
  Dee_Decref(getter);
  /* Invoke the return value of the getter. */
  if unlikely(!callback) return NULL;
  result = DeeObject_CallTuple(callback,args);
  Dee_Decref(callback);
 } else {
  /* Call the attr as-is. */
  rwlock_read(&self->id_lock);
  callback = self->id_vtab[attr->ca_addr];
  Dee_XIncref(callback);
  rwlock_endread(&self->id_lock);
  if unlikely(!callback) goto unbound;
  result = (attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD)
   ? DeeObject_ThisCallTuple(callback,this_arg,args)
   : DeeObject_CallTuple(callback,args);
  Dee_Decref(callback);
 }
 return result;
unbound:
 err_unbound_attribute_c(desc,
                         DeeString_STR(attr->ca_name));
 return NULL;
illegal:
 err_cant_access_attribute_c(desc,
                             DeeString_STR(attr->ca_name),
                             ATTR_ACCESS_GET);
 return NULL;
}
INTERN DREF DeeObject *DCALL
DeeInstance_CallAttributeTupleKw(struct class_desc *__restrict desc,
                                 struct instance_desc *__restrict self,
                                 DeeObject *__restrict this_arg,
                                 struct class_attribute *__restrict attr,
                                 DeeObject *__restrict args, DeeObject *kw) {
 DREF DeeObject *result,*callback;
 ASSERT(self);
 ASSERT(attr);
 ASSERT_OBJECT(this_arg);
 if (attr->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM)
     self = class_desc_as_instance(desc);
 if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
  DREF DeeObject *getter;
  rwlock_read(&self->id_lock);
  getter = self->id_vtab[attr->ca_addr + CLASS_GETSET_GET];
  Dee_XIncref(getter);
  rwlock_endread(&self->id_lock);
  if unlikely(!getter) goto illegal;
  /* Invoke the getter. */
  callback = (attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD)
           ? DeeObject_ThisCall(getter,this_arg,0,NULL)
           : DeeObject_Call(getter,0,NULL)
           ;
  Dee_Decref(getter);
  /* Invoke the return value of the getter. */
  if unlikely(!callback) return NULL;
  result = DeeObject_CallTupleKw(callback,args,kw);
  Dee_Decref(callback);
 } else {
  /* Call the attr as-is. */
  rwlock_read(&self->id_lock);
  callback = self->id_vtab[attr->ca_addr];
  Dee_XIncref(callback);
  rwlock_endread(&self->id_lock);
  if unlikely(!callback) goto unbound;
  result = (attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD)
         ? DeeObject_ThisCallTupleKw(callback,this_arg,args,kw)
         : DeeObject_CallTupleKw(callback,args,kw)
         ;
  Dee_Decref(callback);
 }
 return result;
unbound:
 err_unbound_attribute_c(desc,
                         DeeString_STR(attr->ca_name));
 return NULL;
illegal:
 err_cant_access_attribute_c(desc,
                             DeeString_STR(attr->ca_name),
                             ATTR_ACCESS_GET);
 return NULL;
}
#endif /* CONFIG_HAVE_CALLTUPLE_OPTIMIZATIONS */


INTERN int DCALL
DeeInstance_DelAttribute(struct class_desc *__restrict desc,
                         struct instance_desc *__restrict self,
                         DeeObject *__restrict this_arg,
                         struct class_attribute *__restrict attr) {
 ASSERT(self);
 ASSERT(attr);
 ASSERT_OBJECT(this_arg);
 /* Make sure that the access is allowed. */
 if (attr->ca_flag & CLASS_ATTRIBUTE_FREADONLY)
     goto illegal;
 if (attr->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM)
     self = class_desc_as_instance(desc);
 if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
  DREF DeeObject *delfun,*temp;
  rwlock_read(&self->id_lock);
  delfun = self->id_vtab[attr->ca_addr + CLASS_GETSET_DEL];
  Dee_XIncref(delfun);
  rwlock_endread(&self->id_lock);
  if unlikely(!delfun) goto illegal;
  /* Invoke the getter. */
  temp = (attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD)
   ? DeeObject_ThisCall(delfun,this_arg,0,NULL)
   : DeeObject_Call(delfun,0,NULL);
  Dee_Decref(delfun);
  if unlikely(!temp) return -1;
  Dee_Decref(temp);
 } else {
  DREF DeeObject *old_value;
  /* Simply unbind the field in the attr table. */
  rwlock_write(&self->id_lock);
  old_value = self->id_vtab[attr->ca_addr];
  self->id_vtab[attr->ca_addr] = NULL;
  rwlock_endwrite(&self->id_lock);
#ifdef CONFIG_ERROR_DELETE_UNBOUND
  if unlikely(!old_value) goto unbound;
  Dee_Decref(old_value);
#else /* CONFIG_ERROR_DELETE_UNBOUND */
  Dee_XDecref(old_value);
#endif /* !CONFIG_ERROR_DELETE_UNBOUND */
 }
 return 0;
#ifdef CONFIG_ERROR_DELETE_UNBOUND
unbound:
 return err_unbound_attribute_c(desc,
                                DeeString_STR(attr->ca_name));
#endif /* CONFIG_ERROR_DELETE_UNBOUND */
illegal:
 return err_cant_access_attribute_c(desc,
                                    DeeString_STR(attr->ca_name),
                                    ATTR_ACCESS_DEL);
}
INTERN int DCALL
DeeInstance_SetAttribute(struct class_desc *__restrict desc,
                         struct instance_desc *__restrict self,
                         DeeObject *__restrict this_arg,
                         struct class_attribute *__restrict attr,
                         DeeObject *__restrict value) {
 ASSERT(self);
 ASSERT(attr);
 ASSERT_OBJECT(this_arg);
 if (attr->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM)
     self = class_desc_as_instance(desc);
 if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
  DREF DeeObject *setter,*temp;
  /* Make sure that the access is allowed. */
  if (attr->ca_flag & CLASS_ATTRIBUTE_FREADONLY)
      goto illegal;
  rwlock_read(&self->id_lock);
  setter = self->id_vtab[attr->ca_addr + CLASS_GETSET_SET];
  Dee_XIncref(setter);
  rwlock_endread(&self->id_lock);
  if unlikely(!setter) goto illegal;
  /* Invoke the getter. */
  temp = (attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD)
   ? DeeObject_ThisCall(setter,this_arg,1,(DeeObject **)&value)
   : DeeObject_Call(setter,1,(DeeObject **)&value);
  Dee_Decref(setter);
  if unlikely(!temp) return -1;
  Dee_Decref(temp);
 } else {
  DREF DeeObject *old_value;
  /* Simply override the field in the attr table. */
  rwlock_write(&self->id_lock);
  old_value = self->id_vtab[attr->ca_addr];
  if (old_value && (attr->ca_flag & CLASS_ATTRIBUTE_FREADONLY)) {
   rwlock_endwrite(&self->id_lock);
   goto illegal; /* readonly fields can only be set once. */
  } else {
   Dee_Incref(value);
   self->id_vtab[attr->ca_addr] = value;
  }
  rwlock_endwrite(&self->id_lock);
  /* Drop a reference from the old value. */
  Dee_XDecref(old_value);
 }
 return 0;
illegal:
 return err_cant_access_attribute_c(desc,
                                    DeeString_STR(attr->ca_name),
                                    ATTR_ACCESS_SET);
}
INTERN int DCALL
DeeInstance_SetBasicAttribute(struct class_desc *__restrict desc,
                              struct instance_desc *__restrict self,
                              DeeObject *__restrict this_arg,
                              struct class_attribute *__restrict attr,
                              DeeObject *__restrict value) {
 DREF DeeObject *old_value;
 ASSERT(self);
 ASSERT(attr);
 ASSERT_OBJECT(this_arg);
 if (attr->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM)
     self = class_desc_as_instance(desc);
 if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET)
     return 2; /* Not a basic attribute. */
 /* Simply override the field in the attr table. */
 rwlock_write(&self->id_lock);
 old_value = self->id_vtab[attr->ca_addr];
 if (old_value && (attr->ca_flag & CLASS_ATTRIBUTE_FREADONLY)) {
  rwlock_endwrite(&self->id_lock);
  goto illegal; /* readonly fields can only be set once. */
 } else {
  Dee_Incref(value);
  self->id_vtab[attr->ca_addr] = value;
 }
 rwlock_endwrite(&self->id_lock);
 /* Drop a reference from the old value. */
 Dee_XDecref(old_value);
 return 0;
illegal:
 return err_cant_access_attribute_c(desc,
                                    DeeString_STR(attr->ca_name),
                                    ATTR_ACCESS_SET);
}



INTERN WUNUSED bool DCALL
class_attribute_mayaccess(struct class_attribute *__restrict self,
                          DeeTypeObject *__restrict impl_class) {
 ASSERT_OBJECT_TYPE(impl_class,&DeeType_Type);
 ASSERT(DeeType_IsClass(impl_class));
 ASSERT(self);
 if (self->ca_flag & CLASS_ATTRIBUTE_FPRIVATE) {
  struct code_frame *caller_frame;
  /* Only allow access if the calling code-frame originates from
   * a this-call who's this-argument derives from `class_type'. */
  caller_frame = DeeThread_Self()->t_exec;
  if (!caller_frame ||
     !(caller_frame->cf_func->fo_code->co_flags & CODE_FTHISCALL))
       return false;
  return DeeType_IsInherited(DeeObject_Class(caller_frame->cf_this),
                             impl_class);
 }
 return true;
}


PUBLIC struct class_attribute *DCALL
DeeClassDescriptor_QueryClassAttributeWithHash(DeeClassDescriptorObject *__restrict self,
                                               /*String*/DeeObject *__restrict name, dhash_t hash) {
 struct class_attribute *result; dhash_t i,perturb;
 ASSERT_OBJECT_TYPE_EXACT(name,&DeeString_Type);
 i = perturb = hash & self->cd_cattr_mask;
 for (;; DeeClassDescriptor_CATTRNEXT(i,perturb)) {
  result = &self->cd_cattr_list[i & self->cd_cattr_mask];
  if (!result->ca_name) break;
  if (result->ca_hash != hash) continue;
  if (DeeString_SIZE(result->ca_name) != DeeString_SIZE(name)) continue;
  if (memcmp(DeeString_STR(result->ca_name),DeeString_STR(name),
             DeeString_SIZE(name) * sizeof(char)) != 0)
      continue;
  return result;
 }
 return NULL;
}
PUBLIC struct class_attribute *DCALL
DeeClassDescriptor_QueryClassAttributeStringWithHash(DeeClassDescriptorObject *__restrict self,
                                                     char const *__restrict name, dhash_t hash) {
 struct class_attribute *result; dhash_t i,perturb;
 i = perturb = hash & self->cd_cattr_mask;
 for (;; DeeClassDescriptor_CATTRNEXT(i,perturb)) {
  result = &self->cd_cattr_list[i & self->cd_cattr_mask];
  if (!result->ca_name) break;
  if (result->ca_hash != hash) continue;
  if (strcmp(DeeString_STR(result->ca_name),name) != 0)
      continue;
  return result;
 }
 return NULL;
}
PUBLIC struct class_attribute *DCALL
DeeClassDescriptor_QueryInstanceAttributeWithHash(DeeClassDescriptorObject *__restrict self,
                                                  /*String*/DeeObject *__restrict name, dhash_t hash) {
 struct class_attribute *result; dhash_t i,perturb;
 ASSERT_OBJECT_TYPE_EXACT(name,&DeeString_Type);
 i = perturb = hash & self->cd_iattr_mask;
 for (;; DeeClassDescriptor_IATTRNEXT(i,perturb)) {
  result = &self->cd_iattr_list[i & self->cd_iattr_mask];
  if (!result->ca_name) break;
  if (result->ca_hash != hash) continue;
  if (DeeString_SIZE(result->ca_name) != DeeString_SIZE(name)) continue;
  if (memcmp(DeeString_STR(result->ca_name),DeeString_STR(name),
             DeeString_SIZE(name) * sizeof(char)) != 0)
      continue;
  return result;
 }
 return NULL;
}
PUBLIC struct class_attribute *DCALL
DeeClassDescriptor_QueryInstanceAttributeStringWithHash(DeeClassDescriptorObject *__restrict self,
                                                        char const *__restrict name, dhash_t hash) {
 struct class_attribute *result; dhash_t i,perturb;
 i = perturb = hash & self->cd_iattr_mask;
 for (;; DeeClassDescriptor_IATTRNEXT(i,perturb)) {
  result = &self->cd_iattr_list[i & self->cd_iattr_mask];
  if (!result->ca_name) break;
  if (result->ca_hash != hash) continue;
  if (strcmp(DeeString_STR(result->ca_name),name) != 0)
      continue;
  return result;
 }
 return NULL;
}

PRIVATE ATTR_COLD int DCALL
err_unbound_class_member(/*Class*/DeeTypeObject *__restrict class_type,
                         struct class_desc *__restrict desc,
                         uint16_t addr) {
 /* Check if we can find the proper member so we can pass its name. */
 size_t i; char const *name = "??" "?";
 for (i = 0; i <= desc->cd_desc->cd_cattr_mask; ++i) {
  struct class_attribute *attr;
  attr = &desc->cd_desc->cd_cattr_list[i];
  if (!attr->ca_name) continue;
  if (addr < attr->ca_addr) continue;
  if (addr >= (attr->ca_addr + ((attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) ? 3 : 1))) continue;
  name = DeeString_STR(attr->ca_name);
  goto got_it;
 }
 for (i = 0; i <= desc->cd_desc->cd_iattr_mask; ++i) {
  struct class_attribute *attr;
  attr = &desc->cd_desc->cd_iattr_list[i];
  if (!attr->ca_name) continue;
  if (addr < attr->ca_addr) continue;
  if (addr >= (attr->ca_addr + ((attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) ? 3 : 1))) continue;
  if (!(attr->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM)) continue;
  name = DeeString_STR(attr->ca_name);
  goto got_it;
 }
 /* Throw the error. */
got_it:
 return err_unbound_attribute(class_type,name);
}

PRIVATE ATTR_COLD int DCALL
err_unbound_member(/*Class*/DeeTypeObject *__restrict class_type,
                   struct class_desc *__restrict desc,
                   uint16_t addr) {
 /* Check if we can find the proper member so we can pass its name. */
 size_t i; char const *name = "??" "?";
 for (i = 0; i <= desc->cd_desc->cd_iattr_mask; ++i) {
  struct class_attribute *attr;
  attr = &desc->cd_desc->cd_iattr_list[i];
  if (!attr->ca_name) continue;
  if (addr < attr->ca_addr) continue;
  if (addr >= (attr->ca_addr + ((attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) ? 3 : 1))) continue;
  if (attr->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM) continue;
  name = DeeString_STR(attr->ca_name);
  break;
 }
 /* Throw the error. */
 return err_unbound_attribute(class_type,name);
}


/* Instance member access (by addr) */
INTERN DREF DeeObject *
(DCALL DeeInstance_GetMember)(/*Class*/DeeTypeObject *__restrict tp_self,
                              /*Instance*/DeeObject *__restrict self,
                              uint16_t addr) {
 struct class_desc *desc;
 struct instance_desc *inst;
 DREF DeeObject *result;
 ASSERT_OBJECT_TYPE(tp_self,&DeeType_Type);
 ASSERT(DeeType_IsClass(tp_self));
 ASSERT_OBJECT_TYPE_A(self,tp_self);
 desc = DeeClass_DESC(tp_self);
 ASSERT(addr <= desc->cd_desc->cd_imemb_size);
 inst = DeeInstance_DESC(desc,self);
 /* Lock and extract the member. */
 rwlock_read(&inst->id_lock);
 result = inst->id_vtab[addr];
 Dee_XIncref(result);
 rwlock_endread(&inst->id_lock);
 if (!result)
      err_unbound_member(tp_self,desc,addr);
 return result;
}
INTERN bool
(DCALL DeeInstance_BoundMember)(/*Class*/DeeTypeObject *__restrict tp_self,
                                /*Instance*/DeeObject *__restrict self,
                                uint16_t addr) {
 struct class_desc *desc;
 struct instance_desc *inst;
 ASSERT_OBJECT_TYPE(tp_self,&DeeType_Type);
 ASSERT(DeeType_IsClass(tp_self));
 ASSERT_OBJECT_TYPE_A(self,tp_self);
 desc = DeeClass_DESC(tp_self);
 ASSERT(addr <= desc->cd_desc->cd_imemb_size);
 inst = DeeInstance_DESC(desc,self);
#ifdef CONFIG_NO_THREADS
 return inst->id_vtab[addr] != NULL;
#else
 return ATOMIC_READ(inst->id_vtab[addr]) != NULL;
#endif
}
INTERN int
(DCALL DeeInstance_DelMember)(/*Class*/DeeTypeObject *__restrict tp_self,
                              /*Instance*/DeeObject *__restrict self,
                              uint16_t addr) {
 struct class_desc *desc;
 struct instance_desc *inst;
 DREF DeeObject *old_value;
 ASSERT_OBJECT_TYPE(tp_self,&DeeType_Type);
 ASSERT(DeeType_IsClass(tp_self));
 ASSERT_OBJECT_TYPE_A(self,tp_self);
 desc = DeeClass_DESC(tp_self);
 ASSERT(addr <= desc->cd_desc->cd_imemb_size);
 inst = DeeInstance_DESC(desc,self);
 /* Lock and extract the member. */
 rwlock_write(&inst->id_lock);
 old_value = inst->id_vtab[addr];
 inst->id_vtab[addr] = NULL;
 rwlock_endwrite(&inst->id_lock);
#ifdef CONFIG_ERROR_DELETE_UNBOUND
 if unlikely(!old_value)
    return err_unbound_member(tp_self,desc,addr);
 Dee_Decref(old_value);
#else
 Dee_XDecref(old_value);
#endif
 return 0;
}
INTERN void
(DCALL DeeInstance_SetMember)(/*Class*/DeeTypeObject *__restrict tp_self,
                              /*Instance*/DeeObject *__restrict self,
                              uint16_t addr, DeeObject *__restrict value) {
 struct class_desc *desc;
 struct instance_desc *inst;
 DREF DeeObject *old_value;
 ASSERT_OBJECT_TYPE(tp_self,&DeeType_Type);
 ASSERT(DeeType_IsClass(tp_self));
 ASSERT_OBJECT_TYPE_A(self,tp_self);
 desc = DeeClass_DESC(tp_self);
 ASSERT(addr <= desc->cd_desc->cd_imemb_size);
 inst = DeeInstance_DESC(desc,self);
 /* Lock and extract the member. */
 Dee_Incref(value);
 rwlock_write(&inst->id_lock);
 old_value = inst->id_vtab[addr];
 inst->id_vtab[addr] = value;
 rwlock_endwrite(&inst->id_lock);
 Dee_XDecref(old_value);
}


INTERN DREF DeeObject *
(DCALL DeeInstance_GetMemberSafe)(DeeTypeObject *__restrict tp_self,
                                  DeeObject *__restrict self,
                                  uint16_t addr) {
 if (DeeObject_AssertType((DeeObject *)tp_self,&DeeType_Type))
     goto err;
 if (DeeObject_AssertType(self,tp_self))
     goto err;
 if (!DeeType_IsClass(tp_self))
     goto err_req_class;
 if (addr >= DeeClass_DESC(tp_self)->cd_desc->cd_imemb_size)
     goto err_bad_index;
 return DeeInstance_GetMember(tp_self,self,addr);
err_bad_index:
 err_invalid_instance_addr(tp_self,self,addr);
 goto err;
err_req_class:
 err_requires_class(tp_self);
err:
 return NULL;
}
INTERN int
(DCALL DeeInstance_BoundMemberSafe)(DeeTypeObject *__restrict tp_self,
                                    DeeObject *__restrict self,
                                    uint16_t addr) {
 if (DeeObject_AssertType((DeeObject *)tp_self,&DeeType_Type))
     goto err;
 if (DeeObject_AssertType(self,tp_self))
     goto err;
 if (!DeeType_IsClass(tp_self))
     goto err_req_class;
 if (addr >= DeeClass_DESC(tp_self)->cd_desc->cd_imemb_size)
     goto err_bad_index;
 return DeeInstance_BoundMember(tp_self,self,addr);
err_bad_index:
 return err_invalid_instance_addr(tp_self,self,addr);
err_req_class:
 return err_requires_class(tp_self);
err:
 return -1;
}
INTERN int
(DCALL DeeInstance_DelMemberSafe)(DeeTypeObject *__restrict tp_self,
                                  DeeObject *__restrict self,
                                  uint16_t addr) {
 if (DeeObject_AssertType((DeeObject *)tp_self,&DeeType_Type))
     goto err;
 if (DeeObject_AssertType(self,tp_self))
     goto err;
 if (!DeeType_IsClass(tp_self))
     goto err_req_class;
 if (addr >= DeeClass_DESC(tp_self)->cd_desc->cd_imemb_size)
     goto err_bad_index;
 return DeeInstance_DelMember(tp_self,self,addr);
err_bad_index:
 return err_invalid_instance_addr(tp_self,self,addr);
err_req_class:
 return err_requires_class(tp_self);
err:
 return -1;
}
INTERN int
(DCALL DeeInstance_SetMemberSafe)(DeeTypeObject *__restrict tp_self,
                                  DeeObject *__restrict self,
                                  uint16_t addr, DeeObject *__restrict value) {
 if (DeeObject_AssertType((DeeObject *)tp_self,&DeeType_Type))
     goto err;
 if (DeeObject_AssertType(self,tp_self))
     goto err;
 if (!DeeType_IsClass(tp_self))
     goto err_req_class;
 if (addr >= DeeClass_DESC(tp_self)->cd_desc->cd_imemb_size)
     goto err_bad_index;
 DeeInstance_SetMember(tp_self,self,addr,value);
 return 0;
err_bad_index:
 return err_invalid_instance_addr(tp_self,self,addr);
err_req_class:
 return err_requires_class(tp_self);
err:
 return -1;
}

/* Class member access (by addr) */
INTERN void
(DCALL DeeClass_SetMember)(DeeTypeObject *__restrict self,
                           uint16_t addr, DeeObject *__restrict value) {
 struct class_desc *desc;
 DREF DeeObject *old_value;
 ASSERT_OBJECT_TYPE(self,&DeeType_Type);
 ASSERT(DeeType_IsClass(self));
 desc = DeeClass_DESC(self);
 ASSERT(addr <= desc->cd_desc->cd_cmemb_size);
 /* Lock and extract the member. */
 Dee_Incref(value);
 rwlock_write(&desc->cd_lock);
 old_value = desc->cd_members[addr];
 desc->cd_members[addr] = value;
 rwlock_endwrite(&desc->cd_lock);
 Dee_XDecref(old_value);
}
INTERN int
(DCALL DeeClass_SetMemberSafe)(DeeTypeObject *__restrict self,
                               uint16_t addr, DeeObject *__restrict value) {
 if (DeeObject_AssertType((DeeObject *)self,&DeeType_Type))
     goto err;
 if (!DeeType_IsClass(self))
     goto err_req_class;
 if (addr >= DeeClass_DESC(self)->cd_desc->cd_cmemb_size)
     goto err_bad_index;
 DeeClass_SetMember(self,addr,value);
 return 0;
err_bad_index:
 return err_invalid_class_addr(self,addr);
err_req_class:
 return err_requires_class(self);
err:
 return -1;
}

INTERN DREF DeeObject *
(DCALL DeeClass_GetMember)(DeeTypeObject *__restrict self,
                           uint16_t addr) {
 struct class_desc *desc;
 DREF DeeObject *result;
 ASSERT_OBJECT_TYPE(self,&DeeType_Type);
 ASSERT(DeeType_IsClass(self));
 desc = DeeClass_DESC(self);
 ASSERT(addr <= desc->cd_desc->cd_cmemb_size);
 /* Lock and extract the member. */
 rwlock_write(&desc->cd_lock);
 result = desc->cd_members[addr];
 Dee_XIncref(result);
 rwlock_endwrite(&desc->cd_lock);
 if unlikely(!result)
    err_unbound_class_member(self,desc,addr);
 return result;
}
INTERN DREF DeeObject *
(DCALL DeeClass_GetMemberSafe)(DeeTypeObject *__restrict self,
                               uint16_t addr) {
 if (DeeObject_AssertType((DeeObject *)self,&DeeType_Type))
     goto err;
 if (!DeeType_IsClass(self))
     goto err_req_class;
 if (addr >= DeeClass_DESC(self)->cd_desc->cd_cmemb_size)
     goto err_bad_index;
 return DeeClass_GetMember(self,addr);
err_bad_index:
 err_invalid_class_addr(self,addr);
 goto err;
err_req_class:
 err_requires_class(self);
err:
 return NULL;
}


DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_CLASS_DESC_C */
