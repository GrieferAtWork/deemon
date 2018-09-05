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
#ifndef GUARD_DEEMON_OBJECTS_CLASS_C
#define GUARD_DEEMON_OBJECTS_CLASS_C 1
#define _KOS_SOURCE 1

#include <deemon/api.h>
#include <deemon/class.h>

#ifndef CONFIG_USE_NEW_CLASS_SYSTEM
#include <deemon/object.h>
#include <deemon/code.h>
#include <deemon/none.h>
#include <deemon/attribute.h>
#include <deemon/bool.h>
#include <deemon/string.h>
#include <deemon/file.h>
#include <deemon/arg.h>
#include <deemon/tuple.h>
#include <deemon/int.h>
#include <deemon/callable.h>
#include <deemon/util/cache.h>
#include <deemon/error.h>
#include <deemon/gc.h>
#include <deemon/super.h>
#include <deemon/thread.h>
#include <deemon/util/string.h>

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "../runtime/strings.h"
#include "../runtime/runtime_error.h"

DECL_BEGIN

typedef DeeTypeObject           Type;
typedef DeeMemberTableObject    MemberTable;
typedef DeeInstanceMethodObject InstanceMethod;


#define CLASS_DESC(x) DeeClass_DESC(x)

INTERN bool DCALL
member_mayaccess(DeeTypeObject *__restrict class_type,
                 struct member_entry *__restrict member) {
 ASSERT_OBJECT_TYPE(class_type,&DeeType_Type);
 ASSERT(DeeType_IsClass(class_type));
 ASSERT(member);
 if (member->cme_flag&CLASS_MEMBER_FPRIVATE) {
  struct code_frame *caller_frame;
  /* Only allow access if the calling code-frame originates from
   * a this-call who's this-argument derives from `class_type'. */
  caller_frame = DeeThread_Self()->t_exec;
  if (!caller_frame ||
     !(caller_frame->cf_func->fo_code->co_flags&CODE_FTHISCALL))
       return false;
  return DeeType_IsInherited(DeeObject_Class(caller_frame->cf_this),class_type);
 }
 return true;
}

INTERN struct member_entry empty_class_members[] = {
    {
        /* .cme_name    = */NULL,
        /* .cme_hash    = */0,
        /* .cme_addr    = */0,
        /* .cme_flag    = */0
#if __SIZEOF_POINTER__ >= 8
        ,
        /* .cme_padding = */0
#endif
    }
};

PRIVATE void DCALL
memtab_fini(MemberTable *__restrict self) {
 struct member_entry *iter,*end;
 ASSERT(self != &DeeMemberTable_Empty);
 ASSERT((self->mt_list == empty_class_members) ==
        (self->mt_mask == 0));
 if (self->mt_list == empty_class_members) return;
 /* Clear out all field descriptors. */
 end = (iter = self->mt_list)+(self->mt_mask+1);
 for (; iter != end; ++iter) Dee_XDecref(iter->cme_name);
 Dee_Free(self->mt_list);
}


PUBLIC MemberTable DeeMemberTable_Empty = {
    OBJECT_HEAD_INIT(&DeeMemberTable_Type),
    /* .mt_size = */0,
    /* .mt_mask = */0,
    /* .mt_list = */empty_class_members
};

PRIVATE DREF DeeObject *DCALL
member_table_eq(MemberTable *__restrict self,
                MemberTable *__restrict other) {
 struct member_entry *iter,*end,*iter2;
 if (DeeObject_AssertType((DeeObject *)other,&DeeMemberTable_Type))
     return NULL;
 if (self->mt_size != other->mt_size) goto ne;
 if (self->mt_mask != other->mt_mask) goto ne;
 end = (iter = self->mt_list)+(self->mt_mask+1);
 iter2 = other->mt_list;
 for (; iter != end; ++iter,++iter2) {
  if (iter->cme_addr != iter2->cme_addr) goto ne;
  if (iter->cme_flag != iter2->cme_flag) goto ne;
  if (iter->cme_hash != iter2->cme_hash) goto ne;
  if ((iter->cme_name != NULL) != (iter2->cme_name != NULL)) goto ne;
  if (!iter->cme_name) continue;
  if (iter->cme_name->s_len != iter2->cme_name->s_len) goto ne;
  if (memcmp(iter->cme_name->s_str,iter2->cme_name->s_str,
             iter->cme_name->s_len*sizeof(char)) != 0) goto ne;
 }
 return_true;
ne: return_false;
}

PRIVATE dhash_t DCALL
member_table_hash(MemberTable *__restrict self) {
 struct member_entry *iter,*end;
 dhash_t result = (self->mt_size ^ self->mt_mask);
 end = (iter = self->mt_list)+(self->mt_mask+1);
 for (; iter != end; ++iter) {
  result ^= iter->cme_addr;
  result ^= iter->cme_flag;
  result ^= iter->cme_hash;
 }
 return result;
}

PRIVATE struct type_cmp member_table_cmp = {
    /* .tp_hash = */(dhash_t(DCALL *)(DeeObject *__restrict))&member_table_hash,
    /* .tp_eq   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&member_table_eq,
    /* .tp_ne   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))NULL
};

PUBLIC DeeTypeObject DeeMemberTable_Type = {
    OBJECT_HEAD_INIT(&DeeType_Type),
    /* .tp_name     = */"_membertable",
    /* .tp_doc      = */NULL,
    /* .tp_flags    = */TP_FNORMAL,
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
                    /* .tp_instance_size = */sizeof(MemberTable)
                }
            }
        },
        /* .tp_dtor        = */(void(DCALL *)(DeeObject *__restrict))&memtab_fini,
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
    /* .tp_cmp           = */&member_table_cmp,
    /* .tp_seq           = */NULL,
    /* .tp_iter_next     = */NULL,
    /* .tp_attr          = */NULL,
    /* .tp_with          = */NULL,
    /* .tp_buffer        = */NULL,
    /* .tp_methods       = */NULL,
    /* .tp_getsets       = */NULL,
    /* .tp_members       = */NULL,
    /* .tp_class_methods = */NULL,
    /* .tp_class_getsets = */NULL,
    /* .tp_class_members = */NULL
};

INTERN struct member_entry *DCALL
membertable_lookup_string(MemberTable *__restrict self,
                          char const *__restrict attr, dhash_t hash) {
 dhash_t i,perturb;
 perturb = i = DeeMemberTable_HASHST(self,hash);
 for (;; i = DeeMemberTable_HASHNX(i,perturb),DeeMemberTable_HASHPT(perturb)) {
  struct member_entry *item = DeeMemberTable_HASHIT(self,i);
  if unlikely(!item->cme_name) break; /* Not found */
  if (item->cme_hash != hash) continue; /* Non-matching hash */
  if (strcmp(item->cme_name->s_str,attr) == 0)
      return item;
 }
 return NULL;
}
INTERN dssize_t DCALL
membertable_enum(DeeTypeObject *__restrict tp_self, DeeObject *ob_self,
                 MemberTable *__restrict self, denum_t proc, void *arg) {
 struct member_entry *iter,*end;
 dssize_t temp,result = 0;
 struct class_desc *desc = DeeClass_DESC(tp_self);
 end = (iter = self->mt_list)+(self->mt_mask+1);
 for (; iter != end; ++iter) {
  DREF DeeTypeObject *attr_type;
  struct instance_desc *inst;
  uint16_t perm;
  if (!iter->cme_name) continue;
  inst = NULL,attr_type = NULL;
  perm = ATTR_IMEMBER|ATTR_CMEMBER|ATTR_ACCESS_GET|ATTR_NAMEOBJ|ATTR_DOCOBJ;
  /* Figure out which instance descriptor the property is connected to. */
  if (iter->cme_flag&CLASS_MEMBER_FCLASSMEM) {
   inst = &desc->c_class;
  } else if (ob_self) {
   ASSERT(DeeObject_InstanceOf(ob_self,tp_self));
   inst = DeeInstance_DESC(desc,ob_self);
  }
  if (inst) INSTANCE_DESC_READ(inst);
  if (inst && !(iter->cme_flag&CLASS_MEMBER_FPROPERTY)) {
   /* Actually figure out the type of the member. */
   perm = ATTR_IMEMBER|ATTR_CMEMBER|ATTR_NAMEOBJ|ATTR_DOCOBJ;
   attr_type = (DREF DeeTypeObject *)inst->ih_vtab[iter->cme_addr];
   if (attr_type) {
    attr_type = Dee_TYPE(attr_type);
    Dee_Incref(attr_type);
   }
  }
  if (!(iter->cme_flag&CLASS_MEMBER_FREADONLY)) {
   perm |= (ATTR_ACCESS_DEL|ATTR_ACCESS_SET);
   if (inst && iter->cme_flag&CLASS_MEMBER_FPROPERTY) {
    perm = ATTR_IMEMBER|ATTR_CMEMBER|ATTR_NAMEOBJ|ATTR_DOCOBJ;
    /* Actually figure out what callbacks are assigned. */
    if (inst->ih_vtab[iter->cme_addr + CLASS_PROPERTY_GET]) perm |= ATTR_PERMGET;
    if (inst->ih_vtab[iter->cme_addr + CLASS_PROPERTY_DEL]) perm |= ATTR_PERMDEL;
    if (inst->ih_vtab[iter->cme_addr + CLASS_PROPERTY_SET]) perm |= ATTR_PERMSET;
   }
  }
  if (inst) INSTANCE_DESC_ENDREAD(inst);
  if (iter->cme_flag&CLASS_MEMBER_FPROPERTY)
   perm |= ATTR_PROPERTY;
  else if (iter->cme_flag&CLASS_MEMBER_FMETHOD) {
   perm |= ATTR_PERMCALL;
  }
  if (iter->cme_flag&CLASS_MEMBER_FPRIVATE)
      perm |= ATTR_PRIVATE;
  temp = (*proc)((DeeObject *)tp_self,iter->cme_name->s_str,
                  iter->cme_doc ? iter->cme_doc->s_str : NULL,
                  perm,attr_type,arg);
  Dee_XDecref(attr_type);
  if unlikely(temp < 0) return temp;
  result += temp;
 }
 return result;
}
INTERN dssize_t DCALL
membertable_enum_class(DeeTypeObject *__restrict tp_self,
                       MemberTable *__restrict self,
                       denum_t proc, void *arg) {
 struct member_entry *iter,*end;
 dssize_t temp,result = 0;
 struct class_desc *desc = DeeClass_DESC(tp_self);
 end = (iter = self->mt_list)+(self->mt_mask+1);
 for (; iter != end; ++iter) {
  uint16_t perm;
  DREF DeeTypeObject *attr_type;
  if (!iter->cme_name) continue;
  attr_type = NULL;
  perm = ATTR_IMEMBER|ATTR_CMEMBER|ATTR_WRAPPER|ATTR_ACCESS_GET|ATTR_NAMEOBJ|ATTR_DOCOBJ;
  if (iter->cme_flag & CLASS_MEMBER_FPROPERTY)
   perm |= ATTR_PROPERTY;
  else if (iter->cme_flag & CLASS_MEMBER_FMETHOD) {
   perm |= ATTR_PERMCALL;
  }
  if (iter->cme_flag & CLASS_MEMBER_FPRIVATE)
      perm |= ATTR_PRIVATE;
  if (iter->cme_flag & CLASS_MEMBER_FCLASSMEM) {
   INSTANCE_DESC_READ(&desc->c_class);
   if (iter->cme_flag & CLASS_MEMBER_FPROPERTY) {
    /* Special case: Figure out what property callbacks have been assigned. */
    perm &= ~ATTR_ACCESS_GET;
    if (desc->c_class.ih_vtab[iter->cme_addr + CLASS_PROPERTY_GET])
        perm |= ATTR_ACCESS_GET;
    if (!(iter->cme_flag & CLASS_MEMBER_FREADONLY)) {
     if (desc->c_class.ih_vtab[iter->cme_addr + CLASS_PROPERTY_DEL])
         perm |= ATTR_ACCESS_DEL;
     if (desc->c_class.ih_vtab[iter->cme_addr + CLASS_PROPERTY_SET])
         perm |= ATTR_ACCESS_SET;
    }
   } else {
    DeeObject *obj;
    perm |= (ATTR_ACCESS_DEL|ATTR_ACCESS_SET);
    obj = desc->c_class.ih_vtab[iter->cme_addr];
    if (obj) {
     attr_type = Dee_TYPE(obj);
     Dee_Incref(attr_type);
    }
   }
   INSTANCE_DESC_ENDREAD(&desc->c_class);
  }
  temp = (*proc)((DeeObject *)tp_self,iter->cme_name->s_str,
                  iter->cme_doc ? iter->cme_doc->s_str : NULL,
                  perm,attr_type,arg);
  Dee_XDecref(attr_type);
  if unlikely(temp < 0) return temp;
  result += temp;
 }
 return result;
}

INTERN int DCALL
membertable_find_class(DeeTypeObject *__restrict tp_self,
                       MemberTable *__restrict self,
                       struct attribute_info *__restrict result,
                       struct attribute_lookup_rules const *__restrict rules) {
 struct member_entry *symbol;
 struct class_desc *desc = DeeClass_DESC(tp_self);
 uint16_t perm; DREF DeeTypeObject *attr_type;
 symbol = membertable_lookup_string(self,
                                    rules->alr_name,
                                    rules->alr_hash);
 if (!symbol) return 1;
 attr_type = NULL;
 perm = ATTR_IMEMBER|ATTR_CMEMBER|ATTR_WRAPPER|ATTR_ACCESS_GET|ATTR_NAMEOBJ|ATTR_DOCOBJ;
 if (symbol->cme_flag & CLASS_MEMBER_FPROPERTY)
  perm |= ATTR_PROPERTY;
 else if (symbol->cme_flag & CLASS_MEMBER_FMETHOD) {
  perm |= ATTR_PERMCALL;
 }
 if (symbol->cme_flag & CLASS_MEMBER_FPRIVATE)
     perm |= ATTR_PRIVATE;
 if (symbol->cme_flag & CLASS_MEMBER_FCLASSMEM) {
  INSTANCE_DESC_READ(&desc->c_class);
  if (symbol->cme_flag & CLASS_MEMBER_FPROPERTY) {
   /* Special case: Figure out what property callbacks have been assigned. */
   perm &= ~ATTR_ACCESS_GET;
   if (desc->c_class.ih_vtab[symbol->cme_addr + CLASS_PROPERTY_GET])
       perm |= ATTR_ACCESS_GET;
   if (!(symbol->cme_flag & CLASS_MEMBER_FREADONLY)) {
    if (desc->c_class.ih_vtab[symbol->cme_addr + CLASS_PROPERTY_DEL])
        perm |= ATTR_ACCESS_DEL;
    if (desc->c_class.ih_vtab[symbol->cme_addr + CLASS_PROPERTY_SET])
        perm |= ATTR_ACCESS_SET;
   }
  } else {
   DeeObject *obj;
   perm |= (ATTR_ACCESS_DEL|ATTR_ACCESS_SET);
   obj = desc->c_class.ih_vtab[symbol->cme_addr];
   if (obj) {
    attr_type = Dee_TYPE(obj);
    Dee_Incref(attr_type);
   }
  }
  INSTANCE_DESC_ENDREAD(&desc->c_class);
 }
 if ((perm & rules->alr_perm_mask) != rules->alr_perm_value) {
  Dee_XDecref(attr_type);
  return 1;
 }
 result->a_decl     = (DREF DeeObject *)tp_self;
 result->a_perm     = perm;
 result->a_attrtype = attr_type; /* Inherit reference. */
 result->a_doc      = symbol->cme_doc;
 Dee_Incref(result->a_decl);
 Dee_XIncref(result->a_doc);
 return 0;
}

INTERN int DCALL
membertable_find(DeeTypeObject *__restrict tp_self, DeeObject *ob_self,
                 DeeMemberTableObject *__restrict self,
                 struct attribute_info *__restrict result,
                 struct attribute_lookup_rules const *__restrict rules) {
 struct member_entry *symbol;
 struct instance_desc *inst;
 struct class_desc *desc = DeeClass_DESC(tp_self);
 uint16_t perm; DREF DeeTypeObject *attr_type;
 symbol = membertable_lookup_string(self,
                                    rules->alr_name,
                                    rules->alr_hash);
 if (!symbol) return 1;
 attr_type = NULL;
 perm = ATTR_IMEMBER|ATTR_CMEMBER|ATTR_ACCESS_GET|ATTR_NAMEOBJ|ATTR_DOCOBJ;
 /* Figure out which instance descriptor the property is connected to. */
 if (symbol->cme_flag&CLASS_MEMBER_FCLASSMEM) {
  inst = &desc->c_class;
 } else if (ob_self) {
  inst = DeeInstance_DESC(desc,ob_self);
 } else {
  inst = NULL;
 }
 if (inst) INSTANCE_DESC_READ(inst);
 if (inst && !(symbol->cme_flag&CLASS_MEMBER_FPROPERTY)) {
  /* Actually figure out the type of the member. */
  perm = ATTR_IMEMBER|ATTR_CMEMBER|ATTR_NAMEOBJ|ATTR_DOCOBJ;
  attr_type = (DREF DeeTypeObject *)inst->ih_vtab[symbol->cme_addr];
  if (attr_type) {
   attr_type = Dee_TYPE(attr_type);
   Dee_Incref(attr_type);
  }
 }
 if (!(symbol->cme_flag&CLASS_MEMBER_FREADONLY)) {
  perm |= (ATTR_ACCESS_DEL|ATTR_ACCESS_SET);
  if (inst && symbol->cme_flag&CLASS_MEMBER_FPROPERTY) {
   perm = ATTR_IMEMBER|ATTR_CMEMBER|ATTR_NAMEOBJ|ATTR_DOCOBJ;
   /* Actually figure out what callbacks are assigned. */
   if (inst->ih_vtab[symbol->cme_addr + CLASS_PROPERTY_GET]) perm |= ATTR_PERMGET;
   if (inst->ih_vtab[symbol->cme_addr + CLASS_PROPERTY_DEL]) perm |= ATTR_PERMDEL;
   if (inst->ih_vtab[symbol->cme_addr + CLASS_PROPERTY_SET]) perm |= ATTR_PERMSET;
  }
 }
 if (inst) INSTANCE_DESC_ENDREAD(inst);
 if (symbol->cme_flag&CLASS_MEMBER_FPROPERTY)
  perm |= ATTR_PROPERTY;
 else if (symbol->cme_flag&CLASS_MEMBER_FMETHOD) {
  perm |= ATTR_PERMCALL;
 }
 if (symbol->cme_flag&CLASS_MEMBER_FPRIVATE)
     perm |= ATTR_PRIVATE;
 if ((perm & rules->alr_perm_mask) != rules->alr_perm_value) {
  Dee_XDecref(attr_type);
  return 1;
 }
 result->a_decl     = (DREF DeeObject *)tp_self;
 result->a_perm     = perm;
 result->a_attrtype = attr_type; /* Inherit reference. */
 result->a_doc      = symbol->cme_doc;
 Dee_Incref(result->a_decl);
 Dee_XIncref(result->a_doc);
 return 0;
}


INTERN DREF DeeObject *DCALL
member_get(DeeTypeObject *__restrict class_type,
           struct instance_desc *__restrict self,
           DeeObject *__restrict this_arg,
           struct member_entry *__restrict member) {
 DREF DeeObject *result;
 ASSERT(self);
 ASSERT(member);
 ASSERT_OBJECT(this_arg);
 if (member->cme_flag&CLASS_MEMBER_FCLASSMEM)
     self = &DeeClass_DESC(class_type)->c_class;
 if (member->cme_flag&CLASS_MEMBER_FPROPERTY) {
  DREF DeeObject *getter;
  INSTANCE_DESC_READ(self);
  getter = self->ih_vtab[member->cme_addr+CLASS_PROPERTY_GET];
  Dee_XIncref(getter);
  INSTANCE_DESC_ENDREAD(self);
  if unlikely(!getter) goto illegal;
  /* Invoke the getter. */
  result = (member->cme_flag&CLASS_MEMBER_FMETHOD)
         ? DeeObject_ThisCall(getter,this_arg,0,NULL)
         : DeeObject_Call(getter,0,NULL);
  Dee_Decref(getter);
 } else if (member->cme_flag&CLASS_MEMBER_FMETHOD) {
  /* Construct a thiscall function. */
  DREF DeeObject *callback;
  INSTANCE_DESC_READ(self);
  callback = self->ih_vtab[member->cme_addr];
  Dee_XIncref(callback);
  INSTANCE_DESC_ENDREAD(self);
  if unlikely(!callback) goto unbound;
  result = DeeInstanceMethod_New(callback,this_arg);
  Dee_Decref(callback);
 } else {
  /* Simply return the attribute as-is. */
  INSTANCE_DESC_READ(self);
  result = self->ih_vtab[member->cme_addr];
  Dee_XIncref(result);
  INSTANCE_DESC_ENDREAD(self);
  if unlikely(!result) goto unbound;
 }
 return result;
unbound:
 err_unbound_attribute(class_type,member->cme_name->s_str);
 return NULL;
illegal:
 err_cant_access_attribute(class_type,member->cme_name->s_str,
                           ATTR_ACCESS_GET);
 return NULL;
}
INTERN int DCALL
member_bound(DeeTypeObject *__restrict class_type,
             struct instance_desc *__restrict self,
             DeeObject *__restrict this_arg,
             struct member_entry *__restrict member) {
 DREF DeeObject *result;
 ASSERT(self);
 ASSERT(member);
 ASSERT_OBJECT(this_arg);
 if (member->cme_flag&CLASS_MEMBER_FCLASSMEM)
     self = &DeeClass_DESC(class_type)->c_class;
 if (member->cme_flag&CLASS_MEMBER_FPROPERTY) {
  DREF DeeObject *getter;
  INSTANCE_DESC_READ(self);
  getter = self->ih_vtab[member->cme_addr+CLASS_PROPERTY_GET];
  Dee_XIncref(getter);
  INSTANCE_DESC_ENDREAD(self);
  if unlikely(!getter) goto unbound;
  /* Invoke the getter. */
  result = (member->cme_flag&CLASS_MEMBER_FMETHOD)
         ? DeeObject_ThisCall(getter,this_arg,0,NULL)
         : DeeObject_Call(getter,0,NULL);
  Dee_Decref(getter);
  if likely(result) { Dee_Decref(result); return 1; }
  if (DeeError_Catch(&DeeError_UnboundAttribute))
      return 0;
  return -1;
 } else {
  /* Simply return the attribute as-is. */
#ifdef CONFIG_NO_THREADS
  return self->ih_vtab[member->cme_addr] != NULL;
#else
  return ATOMIC_READ(self->ih_vtab[member->cme_addr]) != NULL;
#endif
 }
unbound:
 return 0;
}
INTERN DREF DeeObject *DCALL
member_call(DeeTypeObject *__restrict class_type,
            struct instance_desc *__restrict self,
            DeeObject *__restrict this_arg,
            struct member_entry *__restrict member,
            size_t argc, DeeObject **__restrict argv) {
 DREF DeeObject *result,*callback;
 ASSERT(self);
 ASSERT(member);
 ASSERT_OBJECT(this_arg);
 if (member->cme_flag&CLASS_MEMBER_FCLASSMEM)
     self = &DeeClass_DESC(class_type)->c_class;
 if (member->cme_flag&CLASS_MEMBER_FPROPERTY) {
  DREF DeeObject *getter;
  INSTANCE_DESC_READ(self);
  getter = self->ih_vtab[member->cme_addr+CLASS_PROPERTY_GET];
  Dee_XIncref(getter);
  INSTANCE_DESC_ENDREAD(self);
  if unlikely(!getter) goto illegal;
  /* Invoke the getter. */
  callback = (member->cme_flag&CLASS_MEMBER_FMETHOD)
   ? DeeObject_ThisCall(getter,this_arg,0,NULL)
   : DeeObject_Call(getter,0,NULL);
  Dee_Decref(getter);
  /* Invoke the return value of the getter. */
  if unlikely(!callback) return NULL;
  result = DeeObject_Call(callback,argc,argv);
  Dee_Decref(callback);
 } else {
  /* Call the member as-is. */
  INSTANCE_DESC_READ(self);
  callback = self->ih_vtab[member->cme_addr];
  Dee_XIncref(callback);
  INSTANCE_DESC_ENDREAD(self);
  if unlikely(!callback) goto unbound;
  result = (member->cme_flag&CLASS_MEMBER_FMETHOD)
   ? DeeObject_ThisCall(callback,this_arg,argc,argv)
   : DeeObject_Call(callback,argc,argv);
  Dee_Decref(callback);
 }
 return result;
unbound:
 err_unbound_attribute(class_type,member->cme_name->s_str);
 return NULL;
illegal:
 err_cant_access_attribute(class_type,member->cme_name->s_str,
                           ATTR_ACCESS_GET);
 return NULL;
}
INTERN DREF DeeObject *DCALL
member_call_kw(DeeTypeObject *__restrict class_type,
               struct instance_desc *__restrict self,
               DeeObject *__restrict this_arg,
               struct member_entry *__restrict member,
               size_t argc, DeeObject **__restrict argv,
               DeeObject *kw) {
 DREF DeeObject *result,*callback;
 ASSERT(self);
 ASSERT(member);
 ASSERT_OBJECT(this_arg);
 if (member->cme_flag&CLASS_MEMBER_FCLASSMEM)
     self = &DeeClass_DESC(class_type)->c_class;
 if (member->cme_flag&CLASS_MEMBER_FPROPERTY) {
  DREF DeeObject *getter;
  INSTANCE_DESC_READ(self);
  getter = self->ih_vtab[member->cme_addr+CLASS_PROPERTY_GET];
  Dee_XIncref(getter);
  INSTANCE_DESC_ENDREAD(self);
  if unlikely(!getter) goto illegal;
  /* Invoke the getter. */
  callback = (member->cme_flag&CLASS_MEMBER_FMETHOD)
   ? DeeObject_ThisCall(getter,this_arg,0,NULL)
   : DeeObject_Call(getter,0,NULL);
  Dee_Decref(getter);
  /* Invoke the return value of the getter. */
  if unlikely(!callback) return NULL;
  result = DeeObject_CallKw(callback,argc,argv,kw);
  Dee_Decref(callback);
 } else {
  /* Call the member as-is. */
  INSTANCE_DESC_READ(self);
  callback = self->ih_vtab[member->cme_addr];
  Dee_XIncref(callback);
  INSTANCE_DESC_ENDREAD(self);
  if unlikely(!callback) goto unbound;
  result = (member->cme_flag&CLASS_MEMBER_FMETHOD)
   ? DeeObject_ThisCallKw(callback,this_arg,argc,argv,kw)
   : DeeObject_CallKw(callback,argc,argv,kw);
  Dee_Decref(callback);
 }
 return result;
unbound:
 err_unbound_attribute(class_type,member->cme_name->s_str);
 return NULL;
illegal:
 err_cant_access_attribute(class_type,member->cme_name->s_str,
                           ATTR_ACCESS_GET);
 return NULL;
}
INTERN int DCALL
member_del(DeeTypeObject *__restrict class_type,
           struct instance_desc *__restrict self,
           DeeObject *__restrict this_arg,
           struct member_entry *__restrict member) {
 ASSERT(self);
 ASSERT(member);
 ASSERT_OBJECT(this_arg);
 /* Make sure that the access is allowed. */
 if (member->cme_flag&CLASS_MEMBER_FREADONLY)
     goto illegal;
 if (member->cme_flag&CLASS_MEMBER_FCLASSMEM)
     self = &DeeClass_DESC(class_type)->c_class;
 if (member->cme_flag&CLASS_MEMBER_FPROPERTY) {
  DREF DeeObject *delfun,*temp;
  INSTANCE_DESC_READ(self);
  delfun = self->ih_vtab[member->cme_addr+CLASS_PROPERTY_DEL];
  Dee_XIncref(delfun);
  INSTANCE_DESC_ENDREAD(self);
  if unlikely(!delfun) goto illegal;
  /* Invoke the getter. */
  temp = (member->cme_flag&CLASS_MEMBER_FMETHOD)
   ? DeeObject_ThisCall(delfun,this_arg,0,NULL)
   : DeeObject_Call(delfun,0,NULL);
  Dee_Decref(delfun);
  if unlikely(!temp) return -1;
  Dee_Decref(temp);
 } else {
  DREF DeeObject *old_value;
  /* Simply unbind the field in the member table. */
  INSTANCE_DESC_WRITE(self);
  old_value = self->ih_vtab[member->cme_addr];
  self->ih_vtab[member->cme_addr] = NULL;
  INSTANCE_DESC_ENDWRITE(self);
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
 err_unbound_attribute(class_type,member->cme_name->s_str);
 return -1;
#endif /* CONFIG_ERROR_DELETE_UNBOUND */
illegal:
 err_cant_access_attribute(class_type,member->cme_name->s_str,
                           ATTR_ACCESS_DEL);
 return -1;
}
INTERN int DCALL
member_set(DeeTypeObject *__restrict class_type,
           struct instance_desc *__restrict self,
           DeeObject *__restrict this_arg,
           struct member_entry *__restrict member,
           DeeObject *__restrict value) {
 ASSERT(self);
 ASSERT(member);
 ASSERT_OBJECT(this_arg);
 if (member->cme_flag&CLASS_MEMBER_FCLASSMEM)
     self = &DeeClass_DESC(class_type)->c_class;
 if (member->cme_flag&CLASS_MEMBER_FPROPERTY) {
  DREF DeeObject *setter,*temp;
  /* Make sure that the access is allowed. */
  if (member->cme_flag&CLASS_MEMBER_FREADONLY)
      goto illegal;
  INSTANCE_DESC_READ(self);
  setter = self->ih_vtab[member->cme_addr+CLASS_PROPERTY_SET];
  Dee_XIncref(setter);
  INSTANCE_DESC_ENDREAD(self);
  if unlikely(!setter) goto illegal;
  /* Invoke the getter. */
  temp = (member->cme_flag&CLASS_MEMBER_FMETHOD)
   ? DeeObject_ThisCall(setter,this_arg,1,(DeeObject **)&value)
   : DeeObject_Call(setter,1,(DeeObject **)&value);
  Dee_Decref(setter);
  if unlikely(!temp) return -1;
  Dee_Decref(temp);
 } else {
  DREF DeeObject *old_value;
  /* Simply override the field in the member table. */
  INSTANCE_DESC_WRITE(self);
  old_value = self->ih_vtab[member->cme_addr];
  if (old_value && (member->cme_flag&CLASS_MEMBER_FREADONLY)) {
   INSTANCE_DESC_ENDWRITE(self);
   goto illegal; /* readonly fields can only be set once. */
  } else {
   Dee_Incref(value);
   self->ih_vtab[member->cme_addr] = value;
  }
  INSTANCE_DESC_ENDWRITE(self);
  /* Drop a reference from the old value. */
  Dee_XDecref(old_value);
 }
 return 0;
illegal:
 err_cant_access_attribute(class_type,member->cme_name->s_str,
                           ATTR_ACCESS_SET);
 return -1;
}


PRIVATE int DCALL
property_init(DeePropertyObject *__restrict self) {
 self->p_get = NULL;
 self->p_del = NULL;
 self->p_set = NULL;
 return 0;
}
PRIVATE int DCALL
property_copy(DeePropertyObject *__restrict self,
              DeePropertyObject *__restrict other) {
 self->p_get = other->p_get;
 self->p_del = other->p_del;
 self->p_set = other->p_set;
 Dee_XIncref(self->p_get);
 Dee_XIncref(self->p_del);
 Dee_XIncref(self->p_set);
 return 0;
}
PRIVATE int DCALL
property_deep(DeePropertyObject *__restrict self,
              DeePropertyObject *__restrict other) {
 self->p_get = NULL;
 self->p_del = NULL;
 self->p_set = NULL;
 if (other->p_get && unlikely((self->p_get = DeeObject_DeepCopy(other->p_get)) == NULL)) goto err;
 if (other->p_del && unlikely((self->p_del = DeeObject_DeepCopy(other->p_del)) == NULL)) goto err;
 if (other->p_set && unlikely((self->p_set = DeeObject_DeepCopy(other->p_set)) == NULL)) goto err;
 return 0;
err:
 /*Dee_XDecref(self->p_set);*/ /* Never set at this point. */
 Dee_XDecref(self->p_del);
 Dee_XDecref(self->p_get);
 return -1;
}
PRIVATE int DCALL
property_ctor(DeePropertyObject *__restrict self,
              size_t argc, DeeObject **__restrict argv) {
 self->p_get = NULL;
 self->p_del = NULL;
 self->p_set = NULL;
 if (DeeArg_Unpack(argc,argv,"|ooo:property",
                  &self->p_get,
                  &self->p_del,
                  &self->p_set))
     return -1;
 if (DeeNone_Check(self->p_get)) self->p_get = NULL;
 if (DeeNone_Check(self->p_del)) self->p_del = NULL;
 if (DeeNone_Check(self->p_set)) self->p_set = NULL;
 Dee_XIncref(self->p_get);
 Dee_XIncref(self->p_del);
 Dee_XIncref(self->p_set);
 return 0;
}
PRIVATE void DCALL
property_fini(DeePropertyObject *__restrict self) {
 Dee_XDecref(self->p_get);
 Dee_XDecref(self->p_del);
 Dee_XDecref(self->p_set);
}
PRIVATE void DCALL
property_visit(DeePropertyObject *__restrict self, dvisit_t proc, void *arg) {
 Dee_XVisit(self->p_get);
 Dee_XVisit(self->p_del);
 Dee_XVisit(self->p_set);
}
PRIVATE dhash_t DCALL
property_hash(DeePropertyObject *__restrict self) {
 return ((self->p_get ? DeeObject_Hash(self->p_get) : 0) ^
         (self->p_del ? DeeObject_Hash(self->p_del) : 0) ^
         (self->p_set ? DeeObject_Hash(self->p_set) : 0));
}
PRIVATE DREF DeeObject *DCALL
property_repr(DeePropertyObject *__restrict self) {
 struct unicode_printer printer = UNICODE_PRINTER_INIT;
 if (UNICODE_PRINTER_PRINT(&printer,"property { ") < 0) goto err;
 if (self->p_get && unicode_printer_printf(&printer,"get = %r%s",self->p_get,
                                            self->p_del || self->p_set ? ", " : "") < 0)
     goto err;
 if (self->p_del && unicode_printer_printf(&printer,"del = %r%s",self->p_del,
                                            self->p_set ? ", " : "") < 0)
     goto err;
 if (self->p_set && unicode_printer_printf(&printer,"set = %r",self->p_set) < 0)
     goto err;
 if (UNICODE_PRINTER_PRINT(&printer," }") < 0) goto err;
 return unicode_printer_pack(&printer);
err:
 unicode_printer_fini(&printer);
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
property_eq(DeePropertyObject *__restrict self,
            DeePropertyObject *__restrict other) {
 int temp;
 if (DeeObject_AssertType((DeeObject *)other,&DeeProperty_Type))
     return NULL;
 if (((self->p_get != NULL) != (other->p_get != NULL)) ||
     ((self->p_del != NULL) != (other->p_del != NULL)) ||
     ((self->p_set != NULL) != (other->p_set != NULL)))
       return_false;
 if (self->p_get && (temp = DeeObject_CompareEq(self->p_get,other->p_get)) <= 0) goto handle_temp;
 if (self->p_del && (temp = DeeObject_CompareEq(self->p_del,other->p_del)) <= 0) goto handle_temp;
 if (self->p_set && (temp = DeeObject_CompareEq(self->p_set,other->p_set)) <= 0) goto handle_temp;
 return_true;
handle_temp:
 return unlikely(temp < 0) ? NULL : (Dee_Incref(Dee_False),Dee_False);
}

PRIVATE struct type_cmp property_cmp = {
    /* .tp_hash = */(dhash_t(DCALL *)(DeeObject *__restrict))&property_hash,
    /* .tp_eq   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&property_eq
};

PRIVATE struct type_member property_members[] = {
    TYPE_MEMBER_FIELD_DOC("get",STRUCT_OBJECT,offsetof(DeePropertyObject,p_get),"->callable\nThe getter callback"),
    TYPE_MEMBER_FIELD_DOC("del",STRUCT_OBJECT,offsetof(DeePropertyObject,p_del),"->callable\nThe delete callback"),
    TYPE_MEMBER_FIELD_DOC("set",STRUCT_OBJECT,offsetof(DeePropertyObject,p_set),"->callable\nThe setter callback"),
    TYPE_MEMBER_END
};

PRIVATE DREF DeeObject *DCALL
property_call(DeePropertyObject *__restrict self,
              size_t argc, DeeObject **__restrict argv) {
 if likely(self->p_get)
    return DeeObject_Call(self->p_get,argc,argv);
 err_unbound_attribute(&DeeProperty_Type,"get");
 return NULL;
}


PUBLIC DeeTypeObject DeeProperty_Type = {
    OBJECT_HEAD_INIT(&DeeType_Type),
    /* .tp_name     = */DeeString_STR(&str_property),
    /* .tp_doc      = */DOC("()\n"
                            "(callable get)\n"
                            "(callable get,callable del)\n"
                            "(callable get,callable del,callable set)\n"
                            "\n"
                            "call(args...)\n"
                            "Same as ${this.get(args...)}"),
    /* .tp_flags    = */TP_FNORMAL|TP_FNAMEOBJECT,
    /* .tp_weakrefs = */0,
    /* .tp_features = */TF_NONE,
    /* .tp_base     = */&DeeObject_Type,
    /* .tp_init = */{
        {
            /* .tp_alloc = */{
                /* .tp_ctor      = */&property_init,
                /* .tp_copy_ctor = */&property_copy,
                /* .tp_deep_ctor = */&property_deep,
                /* .tp_any_ctor  = */&property_ctor,
                /* .tp_free      = */NULL,
                {
                    /* .tp_instance_size = */sizeof(DeePropertyObject),
                }
            }
        },
        /* .tp_dtor        = */(void(DCALL *)(DeeObject *__restrict))&property_fini,
        /* .tp_assign      = */NULL,
        /* .tp_move_assign = */NULL
    },
    /* .tp_cast = */{
        /* .tp_str  = */NULL,
        /* .tp_repr = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&property_repr,
        /* .tp_bool = */NULL
    },
    /* .tp_call          = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&property_call,
    /* .tp_visit         = */(void(DCALL *)(DeeObject *__restrict,dvisit_t,void*))&property_visit,
    /* .tp_gc            = */NULL,
    /* .tp_math          = */NULL,
    /* .tp_cmp           = */&property_cmp,
    /* .tp_seq           = */NULL,
    /* .tp_iter_next     = */NULL,
    /* .tp_attr          = */NULL,
    /* .tp_with          = */NULL,
    /* .tp_buffer        = */NULL,
    /* .tp_methods       = */NULL,
    /* .tp_getsets       = */NULL,
    /* .tp_members       = */property_members,
    /* .tp_class_methods = */NULL,
    /* .tp_class_getsets = */NULL,
    /* .tp_class_members = */NULL
};




INTERN DREF DeeObject *DCALL
instancemember_wrapper(DeeTypeObject *__restrict class_type,
                       struct member_entry *__restrict member) {
 DREF DeeInstanceMemberObject *result;
 ASSERT(member);
 ASSERT_OBJECT_TYPE(class_type,&DeeType_Type);
 ASSERTF(DeeType_IsClass(class_type),
         "Not actually a class type");
 ASSERTF(!(member->cme_flag&CLASS_MEMBER_FCLASSMEM),
         "Not actually an instance member");
 result = DeeObject_MALLOC(DeeInstanceMemberObject);
 if unlikely(!result) goto done;
 DeeObject_Init(result,&DeeInstanceMember_Type);
 result->im_member = member;
 result->im_type   = class_type;
 result->im_desc   = DeeClass_DESC(class_type);
 Dee_Incref(class_type);
done:
 return (DREF DeeObject *)result;
}

PRIVATE DREF DeeObject *DCALL
instancemember_get(DeeInstanceMemberObject *__restrict self,
                   size_t argc, DeeObject **__restrict argv) {
 DeeObject *this_arg;
 if (DeeArg_Unpack(argc,argv,"o:get",&this_arg) ||
     DeeObject_AssertType(this_arg,self->im_type))
     return NULL;
 return member_get(self->im_type,
                   DeeInstance_DESC(self->im_desc,this_arg),
                   this_arg,self->im_member);
}

PRIVATE DREF DeeObject *DCALL
instancemember_del(DeeInstanceMemberObject *__restrict self,
                   size_t argc, DeeObject **__restrict argv) {
 DeeObject *this_arg;
 if (DeeArg_Unpack(argc,argv,"o:del",&this_arg) ||
     DeeObject_AssertType(this_arg,self->im_type) ||
     member_del(self->im_type,
                DeeInstance_DESC(self->im_desc,this_arg),
                this_arg,self->im_member))
     return NULL;
 return_none;
}

PRIVATE DREF DeeObject *DCALL
instancemember_set(DeeInstanceMemberObject *__restrict self,
                   size_t argc, DeeObject **__restrict argv) {
 DeeObject *this_arg,*value;
 if (DeeArg_Unpack(argc,argv,"oo:del",&this_arg,&value) ||
     DeeObject_AssertType(this_arg,self->im_type) ||
     member_set(self->im_type,
                DeeInstance_DESC(self->im_desc,this_arg),
                this_arg,self->im_member,value))
     return NULL;
 return_none;
}

PRIVATE int DCALL
instancemember_copy(DeeInstanceMemberObject *__restrict self,
                    DeeInstanceMemberObject *__restrict other) {
 self->im_type   = other->im_type;
 self->im_member = other->im_member;
 self->im_desc   = other->im_desc;
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
    { "get", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&instancemember_get },
    { "del", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&instancemember_del },
    { "set", (DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&instancemember_set },
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
         Dee_HashPointer(self->im_member));
}
PRIVATE DREF DeeObject *DCALL
instancemember_eq(DeeInstanceMemberObject *__restrict self,
                  DeeInstanceMemberObject *__restrict other) {
 if (DeeObject_AssertType((DeeObject *)other,&DeeInstanceMember_Type))
     return NULL;
 return_bool_(self->im_type == other->im_type &&
              self->im_member == other->im_member);
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
    /* .tp_call          = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&instancemember_get,
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
    /* .tp_getsets       = */NULL,
    /* .tp_members       = */instancemember_members,
    /* .tp_class_methods = */NULL,
    /* .tp_class_getsets = */NULL,
    /* .tp_class_members = */NULL
};
















INTERN DREF DeeObject *DCALL
class_member_get(DeeTypeObject *__restrict class_type,
                 struct instance_desc *__restrict self,
                 struct member_entry *__restrict member) {
 DREF DeePropertyObject *result;
 if (!(member->cme_flag&CLASS_MEMBER_FCLASSMEM)) {
  /* Return an instance-wrapper for instance-members. */
  return instancemember_wrapper(class_type,member);
 }
 if (!(member->cme_flag&CLASS_MEMBER_FPROPERTY)) {
  DREF DeeObject *member_value;
  /* Simple case: direct access to unbound class-based member. */
  INSTANCE_DESC_READ(self);
  member_value = self->ih_vtab[member->cme_addr];
  Dee_XIncref(member_value);
  INSTANCE_DESC_ENDREAD(self);
  if unlikely(!member_value) goto unbound;
  return member_value;
 }
 result = DeeObject_MALLOC(DeePropertyObject);
 if unlikely(!result) return NULL;
 result->p_del = NULL;
 result->p_set = NULL;
 INSTANCE_DESC_READ(self);
 result->p_get = self->ih_vtab[member->cme_addr + CLASS_PROPERTY_GET];
 Dee_XIncref(result->p_get);
 /* Only non-readonly property members have callbacks other than a getter.
  * In this case, the readonly flag both protects the property from being
  * overwritten, as well as being invoked using something other than read-access. */
 if (!(member->cme_flag&CLASS_MEMBER_FREADONLY)) {
  result->p_del = self->ih_vtab[member->cme_addr + CLASS_PROPERTY_DEL];
  Dee_XIncref(result->p_del);
  result->p_set = self->ih_vtab[member->cme_addr + CLASS_PROPERTY_SET];
  Dee_XIncref(result->p_set);
 }
 INSTANCE_DESC_ENDREAD(self);
 /* Make sure that at least a single property callback has been assigned.
  * If not, raise an unbound-member error. */
 if (!result->p_get && !result->p_del && !result->p_set) {
  DeeObject_Free(result);
  goto unbound;
 }
 /* Finalize initialization of the property wrapper and return it. */
 DeeObject_Init(result,&DeeProperty_Type);
 return (DREF DeeObject *)result;
unbound:
 err_unbound_attribute(class_type,member->cme_name->s_str);
 return NULL;
}
INTERN DREF DeeObject *DCALL
class_member_call(DeeTypeObject *__restrict class_type,
                  struct instance_desc *__restrict self,
                  struct member_entry *__restrict member,
                  size_t argc, DeeObject **__restrict argv) {
 DREF DeeObject *callback,*result;
 if (!(member->cme_flag&CLASS_MEMBER_FCLASSMEM)) {
  /* Simulate `operator ()' for wrappers generated by `instancemember_wrapper()'. */
  if (argc != 1) {
   DeeError_Throwf(&DeeError_TypeError,
                   "instancemember `%k' must be called with exactly 1 argument",
                   member->cme_name);
   goto err;
  }
  if (DeeObject_AssertType(argv[0],class_type))
      goto err;
  ASSERT(self == &DeeClass_DESC(class_type)->c_class);
  return member_get(class_type,
                    DeeInstance_DESC(COMPILER_CONTAINER_OF(self,struct class_desc,c_class),
                                     argv[0]),
                    argv[0],member);
 }
 /* Simple case: direct access to unbound class-based member. */
#if 0
 if (member->cme_flag&CLASS_MEMBER_FPROPERTY) {
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
 INSTANCE_DESC_READ(self);
#if CLASS_PROPERTY_GET != 0
 if (member->cme_flag&CLASS_MEMBER_FPROPERTY) {
  callback = self->ih_vtab[member->cme_addr + CLASS_PROPERTY_GET];
 } else {
  callback = self->ih_vtab[member->cme_addr];
 }
#else
 CLASS_PROPERTY_GET;
 callback = self->ih_vtab[member->cme_addr];
#endif
 Dee_XIncref(callback);
 INSTANCE_DESC_ENDREAD(self);
 if unlikely(!callback) goto unbound;
 /* Invoke the callback. */
 result = DeeObject_Call(callback,argc,argv);
 Dee_Decref(callback);
 return result;
unbound:
 err_unbound_attribute(class_type,member->cme_name->s_str);
err:
 return NULL;
}

PRIVATE struct keyword getter_kwlist[] = { K(thisarg), KEND };

INTERN DREF DeeObject *DCALL
class_member_call_kw(DeeTypeObject *__restrict class_type,
                     struct instance_desc *__restrict self,
                     struct member_entry *__restrict member,
                     size_t argc, DeeObject **__restrict argv,
                     DeeObject *kw) {
 DREF DeeObject *callback,*result;
 if (!(member->cme_flag&CLASS_MEMBER_FCLASSMEM)) {
  /* Simulate `operator ()' for wrappers generated by `instancemember_wrapper()'. */
  DeeObject *thisarg;
  if (DeeArg_UnpackKw(argc,argv,kw,getter_kwlist,"o:get",&thisarg) ||
      DeeObject_AssertType(thisarg,class_type))
      goto err;
  ASSERT(self == &DeeClass_DESC(class_type)->c_class);
  return member_get(class_type,
                    DeeInstance_DESC(COMPILER_CONTAINER_OF(self,struct class_desc,c_class),
                                     thisarg),
                    thisarg,member);
 }
 /* Simple case: direct access to unbound class-based member. */
#if 0
 if (member->cme_flag&CLASS_MEMBER_FPROPERTY) {
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
 INSTANCE_DESC_READ(self);
#if CLASS_PROPERTY_GET != 0
 if (member->cme_flag&CLASS_MEMBER_FPROPERTY) {
  callback = self->ih_vtab[member->cme_addr + CLASS_PROPERTY_GET];
 } else {
  callback = self->ih_vtab[member->cme_addr];
 }
#else
 CLASS_PROPERTY_GET;
 callback = self->ih_vtab[member->cme_addr];
#endif
 Dee_XIncref(callback);
 INSTANCE_DESC_ENDREAD(self);
 if unlikely(!callback) goto unbound;
 /* Invoke the callback. */
 result = DeeObject_CallKw(callback,argc,argv,kw);
 Dee_Decref(callback);
 return result;
unbound:
 err_unbound_attribute(class_type,member->cme_name->s_str);
err:
 return NULL;
}
INTERN int DCALL
class_member_del(DeeTypeObject *__restrict class_type,
                 struct instance_desc *__restrict self,
                 struct member_entry *__restrict member) {
 ASSERT(member->cme_flag&CLASS_MEMBER_FCLASSMEM);
 /* Make sure not to re-write readonly attributes. */
 if (member->cme_flag&CLASS_MEMBER_FREADONLY) {
  err_cant_access_attribute(class_type,member->cme_name->s_str,
                            ATTR_ACCESS_DEL);
  return -1;
 }
 if (!(member->cme_flag&CLASS_MEMBER_FPROPERTY)) {
  DREF DeeObject *old_value;
  /* Simple case: directly delete a class-based member. */
  INSTANCE_DESC_READ(self);
  old_value = self->ih_vtab[member->cme_addr];
  self->ih_vtab[member->cme_addr] = NULL;
  INSTANCE_DESC_ENDREAD(self);
#ifdef CONFIG_ERROR_DELETE_UNBOUND
  if unlikely(!old_value) goto unbound;
  Dee_Decref(old_value);
#else /* CONFIG_ERROR_DELETE_UNBOUND */
  Dee_XDecref(old_value);
#endif /* !CONFIG_ERROR_DELETE_UNBOUND */
 } else {
  /* Property callbacks (delete 3 bindings, rather than 1) */
  DREF DeeObject *old_value[3];
  INSTANCE_DESC_WRITE(self);
  old_value[0] = self->ih_vtab[member->cme_addr + CLASS_PROPERTY_GET];
  old_value[1] = self->ih_vtab[member->cme_addr + CLASS_PROPERTY_DEL];
  old_value[2] = self->ih_vtab[member->cme_addr + CLASS_PROPERTY_SET];
  self->ih_vtab[member->cme_addr + CLASS_PROPERTY_GET] = NULL;
  self->ih_vtab[member->cme_addr + CLASS_PROPERTY_DEL] = NULL;
  self->ih_vtab[member->cme_addr + CLASS_PROPERTY_SET] = NULL;
  INSTANCE_DESC_ENDWRITE(self);
#ifdef CONFIG_ERROR_DELETE_UNBOUND
  /* Only thrown an unbound-error when none of the callbacks were assigned. */
  if unlikely(!old_value[0] && !old_value[1] && !old_value[2]) goto unbound;
#endif /* CONFIG_ERROR_DELETE_UNBOUND */
  Dee_XDecref(old_value[2]);
  Dee_XDecref(old_value[1]);
  Dee_XDecref(old_value[0]);
 }
 return 0;
#ifdef CONFIG_ERROR_DELETE_UNBOUND
unbound:
 err_unbound_attribute(class_type,member->cme_name->s_str);
 return -1;
#endif /* CONFIG_ERROR_DELETE_UNBOUND */
}
INTERN int DCALL
class_member_set(DeeTypeObject *__restrict class_type,
                 struct instance_desc *__restrict self,
                 struct member_entry *__restrict member,
                 DeeObject *__restrict value) {
 ASSERT(member->cme_flag&CLASS_MEMBER_FCLASSMEM);
 /* Make sure not to re-write readonly attributes. */
 if (member->cme_flag&CLASS_MEMBER_FREADONLY) {
  err_cant_access_attribute(class_type,member->cme_name->s_str,
                            ATTR_ACCESS_SET);
  return -1;
 }
 if (member->cme_flag&CLASS_MEMBER_FPROPERTY) {
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
  INSTANCE_DESC_WRITE(self);
  old_value[0] = self->ih_vtab[member->cme_addr + CLASS_PROPERTY_GET];
  old_value[1] = self->ih_vtab[member->cme_addr + CLASS_PROPERTY_DEL];
  old_value[2] = self->ih_vtab[member->cme_addr + CLASS_PROPERTY_SET];
  self->ih_vtab[member->cme_addr + CLASS_PROPERTY_GET] = ((DeePropertyObject *)value)->p_get;
  self->ih_vtab[member->cme_addr + CLASS_PROPERTY_DEL] = ((DeePropertyObject *)value)->p_del;
  self->ih_vtab[member->cme_addr + CLASS_PROPERTY_SET] = ((DeePropertyObject *)value)->p_set;
  INSTANCE_DESC_ENDWRITE(self);
  /* Drop references from the old callbacks. */
  Dee_XDecref(old_value[2]);
  Dee_XDecref(old_value[1]);
  Dee_XDecref(old_value[0]);
 } else {
  /* Simple case: direct overwrite an unbound class-based member. */
  DREF DeeObject *old_value;
  Dee_Incref(value);
  INSTANCE_DESC_WRITE(self);
  old_value = self->ih_vtab[member->cme_addr];
  self->ih_vtab[member->cme_addr] = value;
  INSTANCE_DESC_ENDWRITE(self);
  Dee_XDecref(old_value); /* Decref the old value. */
 }
 return 0;
}



/* Since `super' and `instancemethod' objects share the same
 * size, we also let them share a pool of pre-allocated objects. */
STATIC_ASSERT(sizeof(DeeSuperObject) == sizeof(InstanceMethod));
DECLARE_OBJECT_CACHE(super,DeeSuperObject);
#ifndef NDEBUG
#define super_alloc()  super_dbgalloc(__FILE__,__LINE__)
#endif
#define instance_method_alloc     (InstanceMethod *)super_alloc
#define instance_method_free(p)   super_free((DeeSuperObject *)(p))
#define instance_method_tp_alloc  super_tp_alloc
#define instance_method_tp_free   super_tp_free



PUBLIC DREF DeeObject *DCALL
DeeInstanceMethod_New(DeeObject *__restrict func,
                      DeeObject *__restrict this_arg) {
 DREF InstanceMethod *result;
 ASSERT_OBJECT(func);
 ASSERT_OBJECT(this_arg);
 result = instance_method_alloc();
 if unlikely(!result) return NULL;
 DeeObject_Init(result,&DeeInstanceMethod_Type);
 result->im_func = func;
 result->im_this = this_arg;
 Dee_Incref(func);
 Dee_Incref(this_arg);
 return (DREF DeeObject *)result;
}

PRIVATE int DCALL
im_init(InstanceMethod *__restrict self) {
 /* Initialize a stub instance-method. */
 self->im_func = Dee_None;
 self->im_this = Dee_None;
#ifdef CONFIG_NO_THREADS
 Dee_None->ob_refcnt += 2;
#else
 ATOMIC_FETCHADD(Dee_None->ob_refcnt,2);
#endif
 return 0;
}

PRIVATE int DCALL
im_copy(InstanceMethod *__restrict self,
        InstanceMethod *__restrict other) {
 self->im_this = other->im_this;
 self->im_func = other->im_func;
 Dee_Incref(self->im_this);
 Dee_Incref(self->im_func);
 return 0;
}

PRIVATE int DCALL
im_deepcopy(InstanceMethod *__restrict self,
            InstanceMethod *__restrict other) {
 self->im_this = DeeObject_DeepCopy(other->im_this);
 if unlikely(!self->im_this) return -1;
 self->im_func = DeeObject_DeepCopy(other->im_func);
 if unlikely(!self->im_func) { Dee_Decref(self->im_this); return -1; }
 return 0;
}

PRIVATE int DCALL
im_ctor(InstanceMethod *__restrict self,
        size_t argc, DeeObject **__restrict argv) {
 DeeObject *this_arg,*func;
 if (DeeArg_Unpack(argc,argv,"oo:instancemethod",&func,&this_arg))
     return -1;
 self->im_this = this_arg;
 self->im_func = func;
 Dee_Incref(this_arg);
 Dee_Incref(func);
 return 0;
}

PRIVATE DREF DeeObject *DCALL
im_repr(InstanceMethod *__restrict self) {
 return DeeString_Newf("instancemethod(%r,%r)",self->im_func,self->im_this);
}

PRIVATE DREF DeeObject *DCALL
im_call(InstanceMethod *__restrict self,
        size_t argc, DeeObject **__restrict argv) {
 return DeeObject_ThisCall(self->im_func,self->im_this,argc,argv);
}
PRIVATE dhash_t DCALL
im_hash(InstanceMethod *__restrict self) {
 return DeeObject_Hash(self->im_func) ^ DeeObject_Hash(self->im_this);
}

PRIVATE DREF DeeObject *DCALL
im_eq(InstanceMethod *__restrict self,
      InstanceMethod *__restrict other) {
 int temp;
 if (DeeObject_AssertType((DeeObject *)other,&DeeInstanceMethod_Type))
     return NULL;
 temp = DeeObject_CompareEq(self->im_func,other->im_func);
 if (temp <= 0) return unlikely(temp < 0) ? NULL : (Dee_Incref(Dee_False),Dee_False);
 temp = DeeObject_CompareEq(self->im_this,other->im_this);
 if unlikely(temp < 0) return NULL;
 return_bool_(temp);
}
PRIVATE DREF DeeObject *DCALL
im_ne(InstanceMethod *__restrict self,
      InstanceMethod *__restrict other) {
 int temp;
 if (DeeObject_AssertType((DeeObject *)other,&DeeInstanceMethod_Type))
     return NULL;
 temp = DeeObject_CompareNe(self->im_func,other->im_func);
 if (temp != 0) return unlikely(temp < 0) ? NULL : (Dee_Incref(Dee_True),Dee_True);
 temp = DeeObject_CompareNe(self->im_this,other->im_this);
 if unlikely(temp < 0) return NULL;
 return_bool_(temp);
}

PRIVATE struct type_cmp im_cmp = {
    /* .tp_hash = */(dhash_t(DCALL *)(DeeObject *__restrict))&im_hash,
    /* .tp_eq   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&im_eq,
    /* .tp_ne   = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,DeeObject *__restrict))&im_ne,
};


/* Since `super' and `instancemethod' share an identical
 * layout, we can re-use some operators here... */
INTDEF void DCALL super_fini(DeeSuperObject *__restrict self);
INTDEF void DCALL super_visit(DeeSuperObject *__restrict self, dvisit_t proc, void *arg);
#define im_fini   super_fini
#define im_visit  super_visit

PRIVATE struct type_member im_members[] = {
    TYPE_MEMBER_FIELD("__this__",STRUCT_OBJECT,offsetof(InstanceMethod,im_this)),
    TYPE_MEMBER_FIELD_DOC("__func__",STRUCT_OBJECT,offsetof(InstanceMethod,im_func),"->callable"),
    TYPE_MEMBER_END
};


PUBLIC DeeTypeObject DeeInstanceMethod_Type = {
    OBJECT_HEAD_INIT(&DeeType_Type),
    /* .tp_name     = */DeeString_STR(&str_instancemethod),
    /* .tp_doc      = */DOC("(callable func,object this_arg)\n"
                            "Construct an object-bound instance method that can be used to invoke @func\n"
                            "\n"
                            "()(object args...)\n"
                            "Invoke the $func used to construct @this "
                            "instancemethod as ${func(this_arg,args...)}"),
    /* .tp_flags    = */TP_FNORMAL|TP_FNAMEOBJECT,
    /* .tp_weakrefs = */0,
    /* .tp_features = */TF_NONE,
    /* .tp_base     = */&DeeCallable_Type,
    /* .tp_init = */{
        {
            /* .tp_alloc = */{
                /* .tp_ctor      = */&im_init,
                /* .tp_copy_ctor = */&im_copy,
                /* .tp_deep_ctor = */&im_deepcopy,
                /* .tp_any_ctor  = */&im_ctor,
                TYPE_ALLOCATOR(&instance_method_tp_alloc,
                               &instance_method_tp_free)
            }
        },
        /* .tp_dtor        = */(void(DCALL *)(DeeObject *__restrict))&im_fini,
        /* .tp_assign      = */NULL,
        /* .tp_move_assign = */NULL
    },
    /* .tp_cast = */{
        /* .tp_str  = */NULL,
        /* .tp_repr = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict))&im_repr,
        /* .tp_bool = */NULL
    },
    /* .tp_call          = */(DREF DeeObject *(DCALL *)(DeeObject *__restrict,size_t,DeeObject **__restrict))&im_call,
    /* .tp_visit         = */(void(DCALL *)(DeeObject *__restrict,dvisit_t,void*))&im_visit,
    /* .tp_gc            = */NULL,
    /* .tp_math          = */NULL,
    /* .tp_cmp           = */&im_cmp,
    /* .tp_seq           = */NULL,
    /* .tp_iter_next     = */NULL,
    /* .tp_attr          = */NULL,
    /* .tp_with          = */NULL,
    /* .tp_buffer        = */NULL,
    /* .tp_methods       = */NULL,
    /* .tp_getsets       = */NULL,
    /* .tp_members       = */im_members,
    /* .tp_class_methods = */NULL,
    /* .tp_class_getsets = */NULL,
    /* .tp_class_members = */NULL
};









INTERN void DCALL
class_fini(DeeTypeObject *__restrict self) {
 /* Called from `DeeType_Type.tp_init.tp_dtor' */
 struct class_optable **titer,**tend;
 struct class_desc *desc = CLASS_DESC(self);
 DREF DeeObject **iter,**end;
 tend = (titer = desc->c_ops)+COMPILER_LENOF(desc->c_ops);
 for (; titer != tend; ++titer) {
  if (!*titer) continue;
  end = (iter = (*titer)->co_operators)+COMPILER_LENOF((*titer)->co_operators);
  for (; iter != end; ++iter) Dee_XDecref(*iter);
  Dee_Free(*titer);
 }
 end = (iter = desc->c_class.ih_vtab)+desc->c_cmem->mt_size;
 for (; iter != end; ++iter) Dee_XDecref(*iter);
 Dee_Decref(desc->c_mem);
 Dee_Decref(desc->c_cmem);
 /* Class types own the operator tables, so we must clear them here.
  * WARNING: Because we can't do this in `tp_gc' (because of the
  *          WRITE_ONCE locking mechanism), all operator glue functions
  *          must be able to deal with their operator having been deleted. */
 if (!self->tp_base || self->tp_math != self->tp_base->tp_math)
      Dee_Free(self->tp_math);
 if (!self->tp_base || self->tp_cmp != self->tp_base->tp_cmp)
      Dee_Free(self->tp_cmp);
 if (!self->tp_base || self->tp_seq != self->tp_base->tp_seq)
      Dee_Free(self->tp_seq);
 if (!self->tp_base || self->tp_attr != self->tp_base->tp_attr)
      Dee_Free(self->tp_attr);
 if (!self->tp_base || self->tp_with != self->tp_base->tp_with)
      Dee_Free(self->tp_with);
}

INTERN void DCALL
class_visit(DeeTypeObject *__restrict self,
            dvisit_t proc, void *arg) {
 /* Called from `DeeType_Type.tp_visit' */
 struct class_optable **titer,**tend;
 struct class_desc *desc = CLASS_DESC(self);
 DREF DeeObject **iter,**end;
 tend = (titer = desc->c_ops)+COMPILER_LENOF(desc->c_ops);
 for (; titer != tend; ++titer) {
  if (!*titer) continue;
  end = (iter = (*titer)->co_operators)+COMPILER_LENOF((*titer)->co_operators);
  for (; iter != end; ++iter) Dee_XVisit(*iter);
 }
 INSTANCE_DESC_READ(&desc->c_class);
 end = (iter = desc->c_class.ih_vtab)+desc->c_cmem->mt_size;
 for (; iter != end; ++iter) Dee_XVisit(*iter);
 INSTANCE_DESC_ENDREAD(&desc->c_class);
#if 0 /* Cannot be visited themself, so this would be pointless. */
 Dee_Visit(desc->c_mem);
 Dee_Visit(desc->c_cmem);
#endif
}

INTERN void DCALL
class_clear(DeeTypeObject *__restrict self) {
 /* Called from `DeeType_Type.tp_gc' */
 struct class_desc *desc = CLASS_DESC(self);
 struct class_optable **titer,**tend;
 DREF DeeObject **iter,**end;
 size_t decref_laterc = 0;
 DREF DeeObject *ob;
 DREF DeeObject **decref_laterv = NULL;
 DREF DeeObject **new_decref_laterv;
again:
 tend = (titer = desc->c_ops)+COMPILER_LENOF(desc->c_ops);
 INSTANCE_DESC_WRITE(&desc->c_class);
 for (; titer != tend; ++titer) {
  if (!*titer) continue;
  end = (iter = (*titer)->co_operators)+COMPILER_LENOF((*titer)->co_operators);
  for (; iter != end; ++iter) {
   if ((ob = *iter) == NULL) continue;
   *iter = NULL; /* Inherit reference. */
   if (!Dee_DecrefIfNotOne(ob)) {
    new_decref_laterv = (DREF DeeObject **)DeeObject_TryRealloc(decref_laterv,(decref_laterc+1)*
                                                                sizeof(DREF DeeObject *));
    if unlikely(!new_decref_laterv) goto do_decref_now;
    decref_laterv = new_decref_laterv;
    decref_laterv[decref_laterc++] = ob; /* Drop this reference later. */
   }
  }
  /* Free this table. */
  Dee_Free(*titer);
  *titer = NULL;
 }
 end = (iter = desc->c_class.ih_vtab)+desc->c_cmem->mt_size;
 for (; iter != end; ++iter) {
  if ((ob = *iter) == NULL) continue;
  *iter = NULL; /* Inherit reference. */
  if (!Dee_DecrefIfNotOne(ob)) {
   new_decref_laterv = (DREF DeeObject **)DeeObject_TryRealloc(decref_laterv,(decref_laterc+1)*
                                                               sizeof(DREF DeeObject *));
   if unlikely(!new_decref_laterv) goto do_decref_now;
   decref_laterv = new_decref_laterv;
   decref_laterv[decref_laterc++] = ob; /* Drop this reference later. */
  }
 }
 INSTANCE_DESC_ENDWRITE(&desc->c_class);
 /* Drop all saved references. */
 while (decref_laterc--) Dee_Decref(decref_laterv[decref_laterc]);
 Dee_Free(decref_laterv);
 return;
do_decref_now:
 INSTANCE_DESC_ENDWRITE(&desc->c_class);
 Dee_Decref(ob);
 while (decref_laterc--) Dee_Decref(decref_laterv[decref_laterc]);
 Dee_Free(decref_laterv);
 goto again;
}


/* Return the operator associated with the given name or
 * call `err_unimplemented_operator()' to throw an error. */
PRIVATE DREF DeeObject *DCALL
class_getoperator(DeeTypeObject *__restrict self,
                  uint16_t name) {
 DREF DeeObject *result;
 struct class_desc *desc = DeeClass_DESC(self);
 if likely(name < CLASS_OPERATOR_USERCOUNT) {
  struct class_optable *table;
  /* Most likely case: in-bounds operator. */
  table = desc->c_ops[name/CLASS_HEADER_OPC2];
  if unlikely(!table) { result = NULL; goto missing; }
  INSTANCE_DESC_READ(&desc->c_class);
  result = table->co_operators[name % CLASS_HEADER_OPC2];
  Dee_XIncref(result);
  INSTANCE_DESC_ENDREAD(&desc->c_class);
 } else {
  /* Extended operator. */
  ASSERT(name >= OPERATOR_EXTENDED(0));
  ASSERT(desc->c_exopv);
  INSTANCE_DESC_READ(&desc->c_class);
  result = desc->c_exopv[name];
  Dee_XIncref(result);
  INSTANCE_DESC_ENDREAD(&desc->c_class);
 }
 if unlikely(!result) {
missing:
  err_unimplemented_operator(self,name);
 }
 return result;
}
/* Same as `class_getoperator', but don't throw an error when not available. */
PRIVATE DREF DeeObject *DCALL
class_trygetoperator(DeeTypeObject *__restrict self,
                     uint16_t name) {
 DREF DeeObject *result;
 struct class_desc *desc = DeeClass_DESC(self);
 if likely(name < CLASS_OPERATOR_USERCOUNT) {
  struct class_optable *table;
  /* Most likely case: in-bounds operator. */
  table = desc->c_ops[name/CLASS_HEADER_OPC2];
  if unlikely(!table) return NULL;
  INSTANCE_DESC_READ(&desc->c_class);
  result = table->co_operators[name % CLASS_HEADER_OPC2];
  Dee_XIncref(result);
  INSTANCE_DESC_ENDREAD(&desc->c_class);
 } else {
  /* Extended operator. */
  ASSERT(name >= OPERATOR_EXTENDED(0));
  ASSERT(desc->c_exopv);
  INSTANCE_DESC_READ(&desc->c_class);
  result = desc->c_exopv[name];
  Dee_XIncref(result);
  INSTANCE_DESC_ENDREAD(&desc->c_class);
 }
 return result;
}

#ifdef CONFIG_TYPE_ALLOW_OPERATOR_CACHE_INHERITANCE
INTERN void DCALL
DeeClass_InheritOperator(DeeTypeObject *__restrict self,
                         uint16_t name,
                         DeeObject *__restrict value) {
 struct class_desc *desc = DeeClass_DESC(self);
 if likely(name < CLASS_OPERATOR_USERCOUNT) {
  struct class_optable *table;
  /* Most likely case: in-bounds operator. */
again:
  table = desc->c_ops[name / CLASS_HEADER_OPC2];
  if unlikely(!table) {
   /* Allocate a missing table. */
   table = (struct class_optable *)Dee_TryCalloc(sizeof(struct class_optable));
   if unlikely(!table) return;
   if (!ATOMIC_CMPXCH(desc->c_ops[name / CLASS_HEADER_OPC2],NULL,table)) {
    Dee_Free(table);
    goto again;
   }
  }
  INSTANCE_DESC_WRITE(&desc->c_class);
  if (!table->co_operators[name % CLASS_HEADER_OPC2]) {
   table->co_operators[name % CLASS_HEADER_OPC2] = value;
   Dee_Incref(value);
  }
  INSTANCE_DESC_ENDWRITE(&desc->c_class);
 } else {
  /* Extended operator. */
  ASSERT(name >= OPERATOR_EXTENDED(0));
  if (!desc->c_exopv) return;
  INSTANCE_DESC_WRITE(&desc->c_class);
  if (!desc->c_exopv[name]) {
   desc->c_exopv[name] = value;
   Dee_Incref(value);
  }
  INSTANCE_DESC_ENDWRITE(&desc->c_class);
 }
}
#endif

INTERN DREF DeeObject *DCALL
DeeClass_GetOperator(DeeTypeObject *__restrict self, uint16_t name) {
 DREF DeeObject *result;
 result = DeeClass_TryGetOperator(self,name);
 if unlikely(!result)
    err_unimplemented_operator(self,name);
 return result;
}
INTERN DREF DeeObject *DCALL
DeeClass_TryGetOperator(DeeTypeObject *__restrict self, uint16_t name) {
 DREF DeeObject *result;
 struct class_desc *desc = DeeClass_DESC(self);
 if likely(name < CLASS_OPERATOR_USERCOUNT) {
  struct class_optable *table;
  /* Most likely case: in-bounds operator. */
  table = desc->c_ops[name/CLASS_HEADER_OPC2];
  if unlikely(!table) goto inherit_from_base;
  INSTANCE_DESC_READ(&desc->c_class);
  result = table->co_operators[name % CLASS_HEADER_OPC2];
  if unlikely(!result) {
   INSTANCE_DESC_ENDREAD(&desc->c_class);
   goto inherit_from_base;
  }
  Dee_Incref(result);
  INSTANCE_DESC_ENDREAD(&desc->c_class);
 } else {
  /* Extended operator. */
  ASSERT(name >= OPERATOR_EXTENDED(0));
  ASSERT(desc->c_exopv);
  INSTANCE_DESC_READ(&desc->c_class);
  result = desc->c_exopv[name];
  if unlikely(!result) {
   INSTANCE_DESC_ENDREAD(&desc->c_class);
   goto inherit_from_base;
  }
  Dee_Incref(result);
  INSTANCE_DESC_ENDREAD(&desc->c_class);
 }
 return result;
inherit_from_base:
 for (;;) {
  self = DeeType_Base(self);
  if (!self || !DeeType_IsClass(self))
       return NULL;
  result = class_trygetoperator(self,name);
  if (result) break;
 }
#ifdef CONFIG_TYPE_ALLOW_OPERATOR_CACHE_INHERITANCE
 DeeClass_InheritOperator(self,name,result);
#endif
 return result;
}
INTERN DREF DeeObject *DCALL
DeeClass_TryGetPrivateOperator(DeeTypeObject *__restrict self, uint16_t name) {
#ifdef CONFIG_TYPE_ALLOW_OPERATOR_CACHE_INHERITANCE
 /* TODO: Don't consider operators cached from base-classes. */
#endif
 return class_trygetoperator(self,name);
}



/* Type-specific class operator wrappers.
 * NOTE: These are required because they are also invoked from `super',
 *       thus allowing `super' to be used to invoke operators from
 *       base classes that have otherwise been overwritten. */
INTERN int DCALL
class_assign(DeeTypeObject *__restrict tp_self,
             DeeObject *__restrict self,
             DeeObject *__restrict some_object) {
 DREF DeeObject *callback,*callback_result;
 callback = class_getoperator(tp_self,OPERATOR_ASSIGN);
 if unlikely(!callback) return -1;
 callback_result = DeeObject_ThisCallf(callback,self,"o",some_object);
 Dee_Decref(callback);
 if unlikely(!callback_result) return -1;
 Dee_Decref(callback_result);
 return 0;
}
INTERN int DCALL
class_move_assign(DeeTypeObject *__restrict tp_self,
                  DeeObject *__restrict self,
                  DeeObject *__restrict other) {
 DREF DeeObject *callback,*callback_result;
 callback = class_getoperator(tp_self,OPERATOR_MOVEASSIGN);
 if unlikely(!callback) return -1;
 callback_result = DeeObject_ThisCallf(callback,self,"o",other);
 Dee_Decref(callback);
 if unlikely(!callback_result) return -1;
 Dee_Decref(callback_result);
 return 0;
}


INTDEF int DCALL class_ctor(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, size_t argc, DeeObject **__restrict argv);
INTDEF int DCALL class_wrap_ctor(DeeObject *__restrict self, size_t argc, DeeObject **__restrict argv);
INTDEF int DCALL class_copy(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, DeeObject *__restrict other);
INTDEF int DCALL class_wrap_copy(DeeObject *__restrict self, DeeObject *__restrict other);
INTDEF int DCALL class_deep(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, DeeObject *__restrict other);
INTDEF int DCALL class_wrap_deep(DeeObject *__restrict self, DeeObject *__restrict other);
INTDEF int DCALL class_defl_copy(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, DeeObject *__restrict other);
INTDEF int DCALL class_wrap_defl_copy(DeeObject *__restrict self, DeeObject *__restrict other);
INTDEF int DCALL class_defl_deepcopy(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, DeeObject *__restrict other);
INTDEF int DCALL class_wrap_defl_deepcopy(DeeObject *__restrict self, DeeObject *__restrict other);

#define DeeType_INVOKE_ALLOC_CTOR(tp_self,init_type,self)          (*(tp_self)->tp_init.tp_alloc.tp_ctor)(self)
#define DeeType_INVOKE_ALLOC_COPY(tp_self,init_type,self,other)    ((tp_self)->tp_init.tp_alloc.tp_copy_ctor == &class_wrap_copy ? class_copy(init_type,self,other) : (*(tp_self)->tp_init.tp_alloc.tp_copy_ctor)(self,other))
#define DeeType_INVOKE_ALLOC_DEEP(tp_self,init_type,self,other)    ((tp_self)->tp_init.tp_alloc.tp_deep_ctor == &class_wrap_deep ? class_deep(init_type,self,other) : (*(tp_self)->tp_init.tp_alloc.tp_deep_ctor)(self,other))
#define DeeType_INVOKE_ALLOC_ANY(tp_self,init_type,self,argc,argv) ((tp_self)->tp_init.tp_alloc.tp_any_ctor == &class_wrap_ctor ? class_ctor(init_type,self,argc,argv) : (*(tp_self)->tp_init.tp_alloc.tp_any_ctor)(self,argc,argv))
#define DeeType_INVOKE_ALLOC_FREE(tp_self,init_type,ob)            (*(tp_self)->tp_init.tp_alloc.tp_free)(ob)
#define DeeType_INVOKE_ALLOC_ALLOC(tp_self,init_type)              (*(tp_self)->tp_init.tp_alloc.tp_alloc)()
#define DeeType_INVOKE_VAR_CTOR(tp_self,init_type)                 (*(tp_self)->tp_init.tp_var.tp_ctor)()
#define DeeType_INVOKE_VAR_COPY(tp_self,init_type,other)           (*(tp_self)->tp_init.tp_var.tp_copy_ctor)(other)
#define DeeType_INVOKE_VAR_DEEP(tp_self,init_type,other)           (*(tp_self)->tp_init.tp_var.tp_deep_ctor)(other)
#define DeeType_INVOKE_VAR_ANY(tp_self,init_type,argc,argv)        (*(tp_self)->tp_init.tp_var.tp_any_ctor)(argc,argv)
#define DeeType_INVOKE_VAR_FREE(tp_self,init_type,ob)              (*(tp_self)->tp_init.tp_var.tp_free)(ob)


PRIVATE int DCALL
init_super(DeeTypeObject *__restrict tp_self,
           DeeObject *__restrict self,
           size_t argc, DeeObject **__restrict argv) {
 DeeTypeObject *super_type; int error;
 super_type = DeeType_Base(tp_self);
 if likely(super_type) {
  DeeTypeObject *base_type = super_type;
  /* Handle constructor inheritance */
  while (base_type->tp_flags&TP_FINHERITCTOR)
      ASSERTF(!(base_type->tp_flags&TP_FFINAL),
                "Type derived from final type"),
      ASSERT(DeeType_Base(base_type)),
      base_type = DeeType_Base(base_type);
  /* Initialize the super-type. */
  if (base_type->tp_init.tp_alloc.tp_copy_ctor && argc == 1 && /* Copy construction. */
      DeeObject_InstanceOf(argv[0],super_type)) {
   error = DeeType_INVOKE_ALLOC_COPY(base_type,super_type,self,argv[0]);
  } else if (base_type->tp_init.tp_alloc.tp_ctor && argc == 0) {
   error = DeeType_INVOKE_ALLOC_CTOR(base_type,super_type,self);
  } else if (base_type->tp_init.tp_alloc.tp_any_ctor) {
   error = DeeType_INVOKE_ALLOC_ANY(base_type,super_type,self,argc,argv);
  } else if (base_type->tp_init.tp_alloc.tp_deep_ctor &&
             argc == 1 && /* Deep-copy construction. */
             DeeObject_InstanceOf(argv[0],super_type)) {
   error = DeeType_INVOKE_ALLOC_DEEP(base_type,super_type,self,argv[0]);
  } else {
   err_unimplemented_constructor(super_type,argc,argv);
   error = -1;
  }
 } else {
  /* Without a super-type, we act as though
   * the super-type didn't take any arguments. */
  error = 0;
  if (argc != 0) {
   err_invalid_argc("<unnamed>.__constructor__",
                    argc,0,0);
   error = -1;
  }
 }
 return error;
}

PRIVATE int DCALL
init_super_copy(DeeTypeObject *__restrict tp_self,
                DeeObject *__restrict self,
                DeeObject *__restrict other,
                bool deepcopy) {
 DeeTypeObject *super_type; int error;
 super_type = DeeType_Base(tp_self);
 if unlikely(!super_type) return 0;
 /* Handle constructor inheritance */
 while (super_type->tp_flags&TP_FINHERITCTOR)
     ASSERTF(!(super_type->tp_flags&TP_FFINAL),
               "Type derived from final type"),
     ASSERT(DeeType_Base(super_type)),
     super_type = DeeType_Base(super_type);
 /* Initialize the super-type. */
 if (super_type->tp_init.tp_alloc.tp_copy_ctor && !deepcopy) {
  error = DeeType_INVOKE_ALLOC_COPY(super_type,super_type,self,other);
 } else if (super_type->tp_init.tp_alloc.tp_deep_ctor) {
  error = DeeType_INVOKE_ALLOC_DEEP(super_type,super_type,self,other);
 } else if (super_type->tp_init.tp_alloc.tp_any_ctor) {
  error = DeeType_INVOKE_ALLOC_ANY(super_type,super_type,self,1,(DeeObject **)&other);
 } else {
  err_unimplemented_operator(super_type,OPERATOR_COPY);
  return -1;
 }
 return error;
}

PRIVATE int DCALL
load_super_deep(DeeTypeObject *__restrict tp_self,
                DeeObject *__restrict self,
                DeeObject *__restrict other) {
 DeeTypeObject *super_type;
 super_type = DeeType_Base(tp_self);
 if (super_type && Dee_DeepCopyAddAssoc(self,other))
     return -1;
 while (super_type) {
  if (super_type->tp_init.tp_deepload &&
    (*super_type->tp_init.tp_deepload)(self))
      return -1;
  super_type = DeeType_Base(super_type);
 }
 return 0;
}

/* Error print reason when a constructor fails after an object is being shared. */
PRIVATE char const str_shared_ctor_failed[] = "Constructor of shared object failed\n";


/* Special type-level operators, such as constructors and destructors. */
INTERN int DCALL
class_ctor(DeeTypeObject *__restrict tp_self,
           DeeObject *__restrict self,
           size_t argc, DeeObject **__restrict argv) {
 DREF DeeObject *ctor,*superargs; size_t num_members;
 struct class_desc *desc = DeeClass_DESC(tp_self);
 /* Initialize the member table before initializing super, in case
  * the base constructor does some virtual function calls that would
  * otherwise end up accessing uninitialized memory. */
 num_members = desc->c_mem->mt_size;
 if (num_members) {
  struct instance_desc *instance;
  instance = DeeInstance_DESC(desc,self);
#ifndef CONFIG_NO_THREADS
  rwlock_init(&instance->ih_lock);
#endif
  /* Pre-initialize all members as unbound. */
  memset(instance->ih_vtab,0,num_members*sizeof(DREF DeeObject *));
 }
 superargs = class_trygetoperator(tp_self,CLASS_OPERATOR_SUPERARGS);
 if (superargs) {
  DREF DeeObject *callback_result;
  callback_result = DeeObject_Call(superargs,argc,argv);
  Dee_Decref(superargs);
  if unlikely(!callback_result)
     goto err;
  superargs = callback_result;
  /* Make sure that the superargs operator returned a tuple. */
  if (DeeObject_AssertTypeExact(superargs,&DeeTuple_Type))
      goto err_superargs;
  if (init_super(tp_self,self,DeeTuple_SIZE(superargs),DeeTuple_ELEM(superargs)))
      goto err_superargs;
  Dee_Decref(superargs);
 } else {
  if (init_super(tp_self,self,0,NULL))
      goto err;
 }
 /* Check for a user-provided constructor/initialization callback. */
 ctor = class_trygetoperator(tp_self,OPERATOR_CONSTRUCTOR);
 if (ctor) {
  /* Invoke the constructor. */
  DREF DeeObject *callback_result;
  /*DEE_DPRINTF("\n%r\n",((DeeFunctionObject *)ctor)->fo_code);*/
  callback_result = DeeObject_ThisCall(ctor,self,argc,argv);
  Dee_Decref(ctor);
  /* Check for errors in the user's constructor. */
  if unlikely(!callback_result)
     goto err_destroy_bases;
  Dee_Decref(callback_result);
 } else if unlikely(argc != 0) {
  /* Without a user-given constructor callback,
   * emulate a no-op constructor taking no arguments. */
  err_invalid_argc(tp_self->tp_name,argc,0,0);
  goto err_destroy_bases;
 }
 return 0;
err_destroy_bases:
 /* This is kind-of complicated, because we must
  * manually invoke destructors of base classes. */
 if (!DeeObject_UndoConstruction(DeeType_Base(tp_self),self)) {
  DeeError_Print(str_shared_ctor_failed,
                 ERROR_PRINT_DOHANDLE);
  return 0;
 }
 goto err;
err_superargs:
 Dee_Decref(superargs);
err:
 if (num_members) {
  struct instance_desc *instance;
  instance = DeeInstance_DESC(desc,self);
  while (num_members--)
      Dee_XDecref(instance->ih_vtab[num_members]);
 }
 return -1;
}
INTERN int DCALL
class_wrap_ctor(DeeObject *__restrict self,
                size_t argc, DeeObject **__restrict argv) {
 return class_ctor(Dee_TYPE(self),self,argc,argv);
}

INTERN int DCALL
class_copy(DeeTypeObject *__restrict tp_self,
           DeeObject *__restrict self,
           DeeObject *__restrict other) {
 struct instance_desc *instance;
 DREF DeeObject *ctor; size_t num_members;
 struct class_desc *desc = DeeClass_DESC(tp_self);
 /* With the super-class now constructed,
  * it's time to do our own initialization. */
 /* Check for a user-provided constructor/initialization callback. */
 ctor = class_trygetoperator(tp_self,OPERATOR_COPY);
 instance = DeeInstance_DESC(desc,self);
 num_members = desc->c_mem->mt_size;
 if (ctor) {
  DREF DeeObject *callback_result;
  if (num_members) {
#ifndef CONFIG_NO_THREADS
   rwlock_init(&instance->ih_lock);
#endif
   /* Pre-initialize all members as unbound. */
   memset(instance->ih_vtab,0,num_members*sizeof(DREF DeeObject *));
  }
  if (init_super_copy(tp_self,self,other,false))
      goto cleanup_members;
  /* Invoke the constructor. */
  callback_result = DeeObject_ThisCall(ctor,self,1,(DeeObject **)&other);
  Dee_Decref(ctor);
  /* Check for errors in the user's constructor. */
  if unlikely(!callback_result)
     goto err_destroy_bases;
  Dee_Decref(callback_result);
 } else {
  if (num_members) {
   struct instance_desc *other_instance;
   other_instance = DeeInstance_DESC(desc,other);
#ifndef CONFIG_NO_THREADS
   rwlock_init(&instance->ih_lock);
#endif
   INSTANCE_DESC_READ(other_instance);
   /* Reference all variables from the other instance in this one. */
   MEMCPY_PTR(instance->ih_vtab,other_instance->ih_vtab,num_members);
   while (num_members--)
     Dee_XIncref(instance->ih_vtab[num_members]);
   INSTANCE_DESC_ENDREAD(other_instance);
  }
  /* Initialize the super-class. */
  if (init_super_copy(tp_self,self,other,false)) {
cleanup_members:
   ASSERT(!DeeObject_IsShared(self));
   num_members = desc->c_mem->mt_size;
   if (num_members) {
    /* Cleanup all the references we've copied before. */
    while (num_members--)
      Dee_XDecref(instance->ih_vtab[num_members]);
   }
   goto err;
  }
 }
 return 0;
err_destroy_bases:
 if (DeeObject_IsShared(self)) goto consume_error;
 /* Destroy everything we've managed to copy thus far. */
 num_members = desc->c_mem->mt_size;
 while (num_members--)
     Dee_XDecref(instance->ih_vtab[num_members]);
 /* This is kind-of complicated, because we must
  * manually invoke destructors of base classes. */
 if (!DeeObject_UndoConstruction(DeeType_Base(tp_self),self)) {
consume_error:
  DeeError_Print(str_shared_ctor_failed,
                 ERROR_PRINT_DOHANDLE);
  return 0;
 }
err:
 return -1;
}
INTERN int DCALL
class_wrap_copy(DeeObject *__restrict self,
                DeeObject *__restrict other) {
 return class_copy(Dee_TYPE(self),self,other);
}

INTERN int DCALL
class_deep(DeeTypeObject *__restrict tp_self,
           DeeObject *__restrict self,
           DeeObject *__restrict other) {
 struct instance_desc *instance;
 DREF DeeObject *ctor; size_t num_members;
 struct class_desc *desc = DeeClass_DESC(tp_self);
 /* With the super-class now constructed,
  * it's time to do our own initialization. */
 /* Check for a user-provided constructor/initialization callback. */
 ctor = class_trygetoperator(tp_self,OPERATOR_DEEPCOPY);
 instance = DeeInstance_DESC(desc,self);
 num_members = desc->c_mem->mt_size;
 if (ctor) {
  DREF DeeObject *callback_result;
  if (num_members) {
#ifndef CONFIG_NO_THREADS
   rwlock_init(&instance->ih_lock);
#endif
   /* Pre-initialize all members as unbound. */
   memset(instance->ih_vtab,0,num_members*sizeof(DREF DeeObject *));
  }
  if (init_super_copy(tp_self,self,other,true))
      goto cleanup_members;
  if unlikely(load_super_deep(tp_self,self,other))
     goto err_destroy_bases;
  /* Invoke the constructor. */
  callback_result = DeeObject_ThisCall(ctor,self,1,(DeeObject **)&other);
  Dee_Decref(ctor);
  /* Check for errors in the user's constructor. */
  if unlikely(!callback_result)
     goto err_destroy_bases;
  Dee_Decref(callback_result);
 } else {
  if (num_members) {
   struct instance_desc *other_instance;
   other_instance = DeeInstance_DESC(desc,other);
#ifndef CONFIG_NO_THREADS
   rwlock_init(&instance->ih_lock);
#endif
   INSTANCE_DESC_READ(other_instance);
   /* Reference all variables from the other instance in this one. */
   MEMCPY_PTR(instance->ih_vtab,other_instance->ih_vtab,num_members);
   while (num_members--)
     Dee_XIncref(instance->ih_vtab[num_members]);
   INSTANCE_DESC_ENDREAD(other_instance);
  }
  /* Initialize the super-class. */
  if unlikely(init_super_copy(tp_self,self,other,true)) {
cleanup_members:
   ASSERT(!DeeObject_IsShared(self));
   num_members = desc->c_mem->mt_size;
   if (num_members) {
    /* Cleanup all the references we've copied before. */
    while (num_members--)
      Dee_XDecref(instance->ih_vtab[num_members]);
   }
   goto err;
  }
  if unlikely(load_super_deep(tp_self,self,other))
     goto err_destroy_bases;
  /* Replace all members with deep copies of themself. */
  num_members = desc->c_mem->mt_size;
  while (num_members--) {
   if (!instance->ih_vtab[num_members]) continue;
   /* Create an inplace deepcopy of this member. */
   if (DeeObject_InplaceXDeepCopyWithLock(&instance->ih_vtab[num_members],
                                          &instance->ih_lock))
       goto err_destroy_bases;
  }
 }
 return 0;
err_destroy_bases:
 if (DeeObject_IsShared(self)) goto consume_error;
 /* Destroy everything we've managed to copy thus far. */
 num_members = desc->c_mem->mt_size;
 while (num_members--)
     Dee_XDecref(instance->ih_vtab[num_members]);
 /* This is kind-of complicated, because we must
  * manually invoke destructors of base classes. */
 if (!DeeObject_UndoConstruction(DeeType_Base(tp_self),self)) {
consume_error:
  DeeError_Print(str_shared_ctor_failed,
                 ERROR_PRINT_DOHANDLE);
  return 0;
 }
err:
 return -1;
}
INTERN int DCALL
class_wrap_deep(DeeObject *__restrict self,
                DeeObject *__restrict other) {
 return class_deep(Dee_TYPE(self),self,other);
}

PRIVATE void DCALL
class_wrap_dtor(DeeObject *__restrict self) {
 DeeTypeObject *cls = Dee_TYPE(self);
 DREF DeeObject *callback,*callback_result;
 callback = class_trygetoperator(cls,OPERATOR_DESTRUCTOR);
 if unlikely(!callback) goto clear_members;
#ifdef CONFIG_NO_THREADS
 ++self->ob_refcnt;
#else
 ATOMIC_FETCHINC(self->ob_refcnt);
#endif
 callback_result = DeeObject_ThisCall(callback,self,0,NULL);
 Dee_Decref(callback);
 Dee_XDecref(callback_result);
 /* Destructors can't propagate errors,
  * so just dump whatever went wrong. */
 if (!callback_result)
      DeeError_Print("Unhandled error in destructor\n",
                     ERROR_PRINT_DOHANDLE);
 /* Drop the reference we've added above. */
 ASSERT(self->ob_refcnt >= 1);
#ifdef CONFIG_NO_THREADS
 if (--self->ob_refcnt == 0)
#else
 if (ATOMIC_DECFETCH(self->ob_refcnt) == 0)
#endif
 {
  /* Clear member variables. */
  struct class_desc *desc;
  struct instance_desc *inst;
  size_t i,num_members;
clear_members:
  desc = DeeClass_DESC(cls);
  inst = DeeInstance_DESC(desc,self);
  num_members = desc->c_mem->mt_size;
  for (i = 0; i < num_members; ++i)
       Dee_XDecref(inst->ih_vtab[i]);
 }
}
PRIVATE void DCALL
defl_dtor(DeeObject *__restrict self) {
 DeeTypeObject *cls = Dee_TYPE(self);
 struct class_desc *desc = DeeClass_DESC(cls);
 struct instance_desc *inst = DeeInstance_DESC(desc,self);
 size_t i,num_members = desc->c_mem->mt_size;
 /* Clear member variables. */
 for (i = 0; i < num_members; ++i)
      Dee_XDecref(inst->ih_vtab[i]);
}

LOCAL DREF DeeObject *DCALL
invoke_unary(DeeTypeObject *__restrict tp_self,
             DeeObject *__restrict self,
             uint16_t name) {
 DREF DeeObject *callback,*result;
 callback = class_getoperator(tp_self,name);
 if unlikely(!callback) return NULL;
 result = DeeObject_ThisCall(callback,self,0,NULL);
 Dee_Decref(callback);
 return result;
}
LOCAL DREF DeeObject *DCALL
invoke_binary(DeeTypeObject *__restrict tp_self,
              DeeObject *__restrict self,
              DeeObject *__restrict other,
              uint16_t name) {
 DREF DeeObject *callback,*result;
 callback = class_getoperator(tp_self,name);
 if unlikely(!callback) return NULL;
 result = DeeObject_ThisCallf(callback,self,"o",other);
 Dee_Decref(callback);
 return result;
}
LOCAL DREF DeeObject *DCALL
invoke_trinary(DeeTypeObject *__restrict tp_self,
               DeeObject *__restrict self,
               DeeObject *__restrict b,
               DeeObject *__restrict c,
               uint16_t name) {
 DREF DeeObject *callback,*result;
 callback = class_getoperator(tp_self,name);
 if unlikely(!callback) return NULL;
 result = DeeObject_ThisCallf(callback,self,"oo",b,c);
 Dee_Decref(callback);
 return result;
}


INTERN DREF DeeObject *DCALL
class_str(DeeTypeObject *__restrict tp_self,
          DeeObject *__restrict self) {
 DREF DeeObject *result = invoke_unary(tp_self,self,OPERATOR_STR);
 if (result && DeeObject_AssertTypeExact(result,&DeeString_Type))
     Dee_Clear(result);
 return result;
}
INTERN DREF DeeObject *DCALL
class_repr(DeeTypeObject *__restrict tp_self,
           DeeObject *__restrict self) {
 DREF DeeObject *result = invoke_unary(tp_self,self,OPERATOR_REPR);
 if (result && DeeObject_AssertTypeExact(result,&DeeString_Type))
     Dee_Clear(result);
 return result;
}
INTERN int DCALL
class_bool(DeeTypeObject *__restrict tp_self,
           DeeObject *__restrict self) {
 DREF DeeObject *result; int retval;
 result = invoke_unary(tp_self,self,OPERATOR_BOOL);
 if unlikely(!result) return -1;
 retval = DeeObject_Bool(result);
 Dee_Decref(result);
 return retval;
}
INTERN DREF DeeObject *DCALL
class_call(DeeTypeObject *__restrict tp_self,
           DeeObject *__restrict self,
           size_t argc, DeeObject **__restrict argv) {
 DREF DeeObject *callback,*result;
 callback = class_getoperator(tp_self,OPERATOR_CALL);
 if unlikely(!callback) return NULL;
 result = DeeObject_ThisCall(callback,self,argc,argv);
 Dee_Decref(callback);
 return result;
}
INTERN dhash_t DCALL
class_hash(DeeTypeObject *__restrict tp_self,
           DeeObject *__restrict self) {
 DREF DeeObject *callback,*result; dhash_t retval;
 callback = class_trygetoperator(tp_self,OPERATOR_HASH);
 if unlikely(!callback)
    goto hash_generic;
 result = DeeObject_ThisCall(callback,self,0,NULL);
 Dee_Decref(callback);
 if unlikely(!result || DeeObject_AsUIntptr(result,&retval)) {
  /* Hash callbacks can no longer propagate errors.
   * >> Handle anything that happened here. */
  Dee_XDecref(result);
  DeeError_Print("Unhandled error in hash operator\n",
                 ERROR_PRINT_DOHANDLE);
  goto hash_generic;
 }
 Dee_Decref(result);
 return retval;
hash_generic:
 return DeeObject_HashGeneric(self);
}
INTERN DREF DeeObject *DCALL
class_iter_next(DeeTypeObject *__restrict tp_self,
                DeeObject *__restrict self) {
 DREF DeeObject *callback,*result;
 callback = class_getoperator(tp_self,OPERATOR_ITERNEXT);
 if unlikely(!callback) return NULL;
 result = DeeObject_ThisCall(callback,self,0,NULL);
 Dee_Decref(callback);
 /* Transform a stop-iteration error into an ITER_DONE return value. */
 if (!result && DeeError_Catch(&DeeError_StopIteration))
      result = ITER_DONE;
 return result;
}
INTERN DREF DeeObject *DCALL
class_int(DeeTypeObject *__restrict tp_self,
          DeeObject *__restrict self) {
 DREF DeeObject *result = invoke_unary(tp_self,self,OPERATOR_INT);
 if (result && DeeObject_AssertTypeExact(result,&DeeInt_Type))
     Dee_Clear(result);
 return result;
}
INTERN int DCALL
class_double(DeeTypeObject *__restrict tp_self,
             DeeObject *__restrict self,
             double *__restrict presult) {
 DREF DeeObject *callback,*result; int error;
 callback = class_getoperator(tp_self,OPERATOR_FLOAT);
 if unlikely(!callback) return -1;
 result = DeeObject_ThisCall(callback,self,0,NULL);
 Dee_Decref(callback);
 /* Transform a stop-iteration error into an ITER_DONE return value. */
 if unlikely(!result) return -1;
 error = DeeObject_AsDouble(result,presult);
 Dee_Decref(result);
 return error;
}
INTERN DREF DeeObject *DCALL
class_inv(DeeTypeObject *__restrict tp_self,
          DeeObject *__restrict self) {
 return invoke_unary(tp_self,self,OPERATOR_INV);
}
INTERN DREF DeeObject *DCALL
class_pos(DeeTypeObject *__restrict tp_self,
          DeeObject *__restrict self) {
 return invoke_unary(tp_self,self,OPERATOR_POS);
}
INTERN DREF DeeObject *DCALL
class_neg(DeeTypeObject *__restrict tp_self,
          DeeObject *__restrict self) {
 return invoke_unary(tp_self,self,OPERATOR_NEG);
}
INTERN DREF DeeObject *DCALL
class_add(DeeTypeObject *__restrict tp_self,
          DeeObject *__restrict self,
          DeeObject *__restrict other) {
 return invoke_binary(tp_self,self,other,OPERATOR_ADD);
}
INTERN DREF DeeObject *DCALL
class_sub(DeeTypeObject *__restrict tp_self,
          DeeObject *__restrict self,
          DeeObject *__restrict other) {
 return invoke_binary(tp_self,self,other,OPERATOR_SUB);
}
INTERN DREF DeeObject *DCALL
class_mul(DeeTypeObject *__restrict tp_self,
          DeeObject *__restrict self,
          DeeObject *__restrict other) {
 return invoke_binary(tp_self,self,other,OPERATOR_MUL);
}
INTERN DREF DeeObject *DCALL
class_div(DeeTypeObject *__restrict tp_self,
          DeeObject *__restrict self,
          DeeObject *__restrict other) {
 return invoke_binary(tp_self,self,other,OPERATOR_DIV);
}
INTERN DREF DeeObject *DCALL
class_mod(DeeTypeObject *__restrict tp_self,
          DeeObject *__restrict self,
          DeeObject *__restrict other) {
 return invoke_binary(tp_self,self,other,OPERATOR_MOD);
}
INTERN DREF DeeObject *DCALL
class_shl(DeeTypeObject *__restrict tp_self,
          DeeObject *__restrict self,
          DeeObject *__restrict other) {
 return invoke_binary(tp_self,self,other,OPERATOR_SHL);
}
INTERN DREF DeeObject *DCALL
class_shr(DeeTypeObject *__restrict tp_self,
          DeeObject *__restrict self,
          DeeObject *__restrict other) {
 return invoke_binary(tp_self,self,other,OPERATOR_SHR);
}
INTERN DREF DeeObject *DCALL
class_and(DeeTypeObject *__restrict tp_self,
          DeeObject *__restrict self,
          DeeObject *__restrict other) {
 return invoke_binary(tp_self,self,other,OPERATOR_AND);
}
INTERN DREF DeeObject *DCALL
class_or(DeeTypeObject *__restrict tp_self,
         DeeObject *__restrict self,
         DeeObject *__restrict other) {
 return invoke_binary(tp_self,self,other,OPERATOR_OR);
}
INTERN DREF DeeObject *DCALL
class_xor(DeeTypeObject *__restrict tp_self,
          DeeObject *__restrict self,
          DeeObject *__restrict other) {
 return invoke_binary(tp_self,self,other,OPERATOR_XOR);
}
INTERN DREF DeeObject *DCALL
class_pow(DeeTypeObject *__restrict tp_self,
          DeeObject *__restrict self,
          DeeObject *__restrict other) {
 return invoke_binary(tp_self,self,other,OPERATOR_POW);
}
INTERN int DCALL
class_inc(DeeTypeObject *__restrict tp_self,
          DeeObject **__restrict pself) {
 DREF DeeObject *result;
 result = invoke_unary(tp_self,*pself,OPERATOR_INC);
 if unlikely(!result) return -1;
 Dee_Decref(*pself);
 *pself = result; /* Inherit refernece. */
 return 0;
}
INTERN int DCALL
class_dec(DeeTypeObject *__restrict tp_self,
          DeeObject **__restrict pself) {
 DREF DeeObject *result;
 result = invoke_unary(tp_self,*pself,OPERATOR_DEC);
 if unlikely(!result) return -1;
 Dee_Decref(*pself);
 *pself = result; /* Inherit refernece. */
 return 0;
}
INTERN int DCALL
class_inplace_add(DeeTypeObject *__restrict tp_self,
                  DeeObject **__restrict pself,
                  DeeObject *__restrict other) {
 DREF DeeObject *result;
 result = invoke_binary(tp_self,*pself,other,OPERATOR_INPLACE_ADD);
 if unlikely(!result) return -1;
 Dee_Decref(*pself);
 *pself = result; /* Inherit refernece. */
 return 0;
}
INTERN int DCALL
class_inplace_sub(DeeTypeObject *__restrict tp_self,
                  DeeObject **__restrict pself,
                  DeeObject *__restrict other) {
 DREF DeeObject *result;
 result = invoke_binary(tp_self,*pself,other,OPERATOR_INPLACE_SUB);
 if unlikely(!result) return -1;
 Dee_Decref(*pself);
 *pself = result; /* Inherit refernece. */
 return 0;
}
INTERN int DCALL
class_inplace_mul(DeeTypeObject *__restrict tp_self,
                  DeeObject **__restrict pself,
                  DeeObject *__restrict other) {
 DREF DeeObject *result;
 result = invoke_binary(tp_self,*pself,other,OPERATOR_INPLACE_MUL);
 if unlikely(!result) return -1;
 Dee_Decref(*pself);
 *pself = result; /* Inherit refernece. */
 return 0;
}
INTERN int DCALL
class_inplace_div(DeeTypeObject *__restrict tp_self,
                  DeeObject **__restrict pself,
                  DeeObject *__restrict other) {
 DREF DeeObject *result;
 result = invoke_binary(tp_self,*pself,other,OPERATOR_INPLACE_DIV);
 if unlikely(!result) return -1;
 Dee_Decref(*pself);
 *pself = result; /* Inherit refernece. */
 return 0;
}
INTERN int DCALL
class_inplace_mod(DeeTypeObject *__restrict tp_self,
                  DeeObject **__restrict pself,
                  DeeObject *__restrict other) {
 DREF DeeObject *result;
 result = invoke_binary(tp_self,*pself,other,OPERATOR_INPLACE_MOD);
 if unlikely(!result) return -1;
 Dee_Decref(*pself);
 *pself = result; /* Inherit refernece. */
 return 0;
}
INTERN int DCALL
class_inplace_shl(DeeTypeObject *__restrict tp_self,
                  DeeObject **__restrict pself,
                  DeeObject *__restrict other) {
 DREF DeeObject *result;
 result = invoke_binary(tp_self,*pself,other,OPERATOR_INPLACE_SHL);
 if unlikely(!result) return -1;
 Dee_Decref(*pself);
 *pself = result; /* Inherit refernece. */
 return 0;
}
INTERN int DCALL
class_inplace_shr(DeeTypeObject *__restrict tp_self,
                  DeeObject **__restrict pself,
                  DeeObject *__restrict other) {
 DREF DeeObject *result;
 result = invoke_binary(tp_self,*pself,other,OPERATOR_INPLACE_SHR);
 if unlikely(!result) return -1;
 Dee_Decref(*pself);
 *pself = result; /* Inherit refernece. */
 return 0;
}
INTERN int DCALL
class_inplace_and(DeeTypeObject *__restrict tp_self,
                  DeeObject **__restrict pself,
                  DeeObject *__restrict other) {
 DREF DeeObject *result;
 result = invoke_binary(tp_self,*pself,other,OPERATOR_INPLACE_AND);
 if unlikely(!result) return -1;
 Dee_Decref(*pself);
 *pself = result; /* Inherit refernece. */
 return 0;
}
INTERN int DCALL
class_inplace_or(DeeTypeObject *__restrict tp_self,
                 DeeObject **__restrict pself,
                 DeeObject *__restrict other) {
 DREF DeeObject *result;
 result = invoke_binary(tp_self,*pself,other,OPERATOR_INPLACE_OR);
 if unlikely(!result) return -1;
 Dee_Decref(*pself);
 *pself = result; /* Inherit refernece. */
 return 0;
}
INTERN int DCALL
class_inplace_xor(DeeTypeObject *__restrict tp_self,
                  DeeObject **__restrict pself,
                  DeeObject *__restrict other) {
 DREF DeeObject *result;
 result = invoke_binary(tp_self,*pself,other,OPERATOR_INPLACE_XOR);
 if unlikely(!result) return -1;
 Dee_Decref(*pself);
 *pself = result; /* Inherit refernece. */
 return 0;
}
INTERN int DCALL
class_inplace_pow(DeeTypeObject *__restrict tp_self,
                  DeeObject **__restrict pself,
                  DeeObject *__restrict other) {
 DREF DeeObject *result;
 result = invoke_binary(tp_self,*pself,other,OPERATOR_INPLACE_POW);
 if unlikely(!result) return -1;
 Dee_Decref(*pself);
 *pself = result; /* Inherit refernece. */
 return 0;
}
INTERN DREF DeeObject *DCALL
class_eq(DeeTypeObject *__restrict tp_self,
         DeeObject *__restrict self,
         DeeObject *__restrict other) {
 return invoke_binary(tp_self,self,other,OPERATOR_EQ);
}
INTERN DREF DeeObject *DCALL
class_ne(DeeTypeObject *__restrict tp_self,
         DeeObject *__restrict self,
         DeeObject *__restrict other) {
 return invoke_binary(tp_self,self,other,OPERATOR_NE);
}
INTERN DREF DeeObject *DCALL
class_lo(DeeTypeObject *__restrict tp_self,
         DeeObject *__restrict self,
         DeeObject *__restrict other) {
 return invoke_binary(tp_self,self,other,OPERATOR_LO);
}
INTERN DREF DeeObject *DCALL
class_le(DeeTypeObject *__restrict tp_self,
         DeeObject *__restrict self,
         DeeObject *__restrict other) {
 return invoke_binary(tp_self,self,other,OPERATOR_LE);
}
INTERN DREF DeeObject *DCALL
class_gr(DeeTypeObject *__restrict tp_self,
         DeeObject *__restrict self,
         DeeObject *__restrict other) {
 return invoke_binary(tp_self,self,other,OPERATOR_GR);
}
INTERN DREF DeeObject *DCALL
class_ge(DeeTypeObject *__restrict tp_self,
         DeeObject *__restrict self,
         DeeObject *__restrict other) {
 return invoke_binary(tp_self,self,other,OPERATOR_GE);
}
INTERN DREF DeeObject *DCALL
class_iter_self(DeeTypeObject *__restrict tp_self,
                DeeObject *__restrict self) {
 return invoke_unary(tp_self,self,OPERATOR_ITERSELF);
}
INTERN DREF DeeObject *DCALL
class_size(DeeTypeObject *__restrict tp_self,
           DeeObject *__restrict self) {
 return invoke_unary(tp_self,self,OPERATOR_SIZE);
}
INTERN DREF DeeObject *DCALL
class_contains(DeeTypeObject *__restrict tp_self,
               DeeObject *__restrict self,
               DeeObject *__restrict other) {
 return invoke_binary(tp_self,self,other,OPERATOR_CONTAINS);
}
INTERN DREF DeeObject *DCALL
class_getitem(DeeTypeObject *__restrict tp_self,
              DeeObject *__restrict self,
              DeeObject *__restrict index) {
 return invoke_binary(tp_self,self,index,OPERATOR_GETITEM);
}
INTERN int DCALL
class_delitem(DeeTypeObject *__restrict tp_self,
              DeeObject *__restrict self,
              DeeObject *__restrict index) {
 DREF DeeObject *result;
 result = invoke_binary(tp_self,self,index,OPERATOR_DELITEM);
 Dee_XDecref(result);
 return likely(result) ? 0 : -1;
}
INTERN int DCALL
class_setitem(DeeTypeObject *__restrict tp_self,
              DeeObject *__restrict self,
              DeeObject *__restrict index,
              DeeObject *__restrict value) {
 DREF DeeObject *result;
 result = invoke_trinary(tp_self,self,index,value,OPERATOR_SETITEM);
 Dee_XDecref(result);
 return likely(result) ? 0 : -1;
}
INTERN DREF DeeObject *DCALL
class_getrange(DeeTypeObject *__restrict tp_self,
               DeeObject *__restrict self,
               DeeObject *__restrict begin,
               DeeObject *__restrict end) {
 return invoke_trinary(tp_self,self,begin,end,OPERATOR_GETRANGE);
}
INTERN int DCALL
class_delrange(DeeTypeObject *__restrict tp_self,
               DeeObject *__restrict self,
               DeeObject *__restrict begin,
               DeeObject *__restrict end) {
 DREF DeeObject *result;
 result = invoke_trinary(tp_self,self,begin,end,OPERATOR_DELRANGE);
 Dee_XDecref(result);
 return likely(result) ? 0 : -1;
}
INTERN int DCALL
class_setrange(DeeTypeObject *__restrict tp_self,
               DeeObject *__restrict self,
               DeeObject *__restrict begin,
               DeeObject *__restrict end,
               DeeObject *__restrict value) {
 DREF DeeObject *callback,*result;
 callback = class_getoperator(tp_self,OPERATOR_SETRANGE);
 if unlikely(!callback) return -1;
 result = DeeObject_ThisCallf(callback,self,"ooo",begin,end,value);
 Dee_Decref(callback);
 Dee_XDecref(result);
 return likely(result) ? 0 : -1;
}
INTERN DREF DeeObject *DCALL
class_getattr(DeeTypeObject *__restrict tp_self,
              DeeObject *__restrict self,
              DeeObject *__restrict attr) {
 return invoke_binary(tp_self,self,attr,OPERATOR_GETATTR);
}
INTERN int DCALL
class_delattr(DeeTypeObject *__restrict tp_self,
              DeeObject *__restrict self,
              DeeObject *__restrict attr) {
 DREF DeeObject *result;
 result = invoke_binary(tp_self,self,attr,OPERATOR_DELATTR);
 Dee_XDecref(result);
 return likely(result) ? 0 : -1;
}
INTERN int DCALL
class_setattr(DeeTypeObject *__restrict tp_self,
              DeeObject *__restrict self,
              DeeObject *__restrict attr,
              DeeObject *__restrict value) {
 DREF DeeObject *result;
 result = invoke_trinary(tp_self,self,attr,value,OPERATOR_SETATTR);
 Dee_XDecref(result);
 return likely(result) ? 0 : -1;
}
INTERN int DCALL
class_enter(DeeTypeObject *__restrict tp_self,
            DeeObject *__restrict self) {
 DREF DeeObject *result;
 result = invoke_unary(tp_self,self,OPERATOR_ENTER);
 Dee_XDecref(result);
 return likely(result) ? 0 : -1;
}
INTERN int DCALL
class_leave(DeeTypeObject *__restrict tp_self,
            DeeObject *__restrict self) {
 DREF DeeObject *result;
 result = invoke_unary(tp_self,self,OPERATOR_LEAVE);
 Dee_XDecref(result);
 return likely(result) ? 0 : -1;
}
INTERN dssize_t DCALL
class_enumattr(DeeTypeObject *__restrict tp_self,
               DeeObject *__restrict self,
               denum_t proc, void *arg) {
 (void)tp_self;
 (void)self;
 (void)proc;
 (void)arg;
 /* XXX: Invoke user-operator? */
 return 0;
}



/* Type-level wrappers that don't take a type argument. */
#define UNARY_WRAPPER(Treturn,name) \
Treturn DCALL \
class_wrap_##name(DeeObject *__restrict self) \
{ \
 return class_##name(Dee_TYPE(self),self); \
}
#define UNARY_INPLACE_WRAPPER(Treturn,name) \
Treturn DCALL \
class_wrap_##name(DeeObject **__restrict pself) \
{ \
 return class_##name(Dee_TYPE(*pself),pself); \
}
#define BINARY_WRAPPER(Treturn,name) \
Treturn DCALL \
class_wrap_##name(DeeObject *__restrict self, DeeObject *__restrict other) \
{ \
 return class_##name(Dee_TYPE(self),self,other); \
}
#define BINARY_INPLACE_WRAPPER(Treturn,name) \
Treturn DCALL \
class_wrap_##name(DeeObject **__restrict pself, DeeObject *__restrict other) \
{ \
 return class_##name(Dee_TYPE(*pself),pself,other); \
}
#define TRINARY_WRAPPER(Treturn,name) \
Treturn DCALL \
class_wrap_##name(DeeObject *__restrict self, \
                  DeeObject *__restrict a, \
                  DeeObject *__restrict b) \
{ \
 return class_##name(Dee_TYPE(self),self,a,b); \
}
#define QUAD_WRAPPER(Treturn,name) \
Treturn DCALL \
class_wrap_##name(DeeObject *__restrict self, \
                  DeeObject *__restrict a, \
                  DeeObject *__restrict b, \
                  DeeObject *__restrict c) \
{ \
 return class_##name(Dee_TYPE(self),self,a,b,c); \
}

/* Define operator wrapper callbacks. */
INTERN BINARY_WRAPPER(int,assign)
INTERN BINARY_WRAPPER(int,move_assign)
INTERN UNARY_WRAPPER(DREF DeeObject *,str)
INTERN UNARY_WRAPPER(DREF DeeObject *,repr)
INTERN UNARY_WRAPPER(int,bool)
INTERN DREF DeeObject *DCALL
class_wrap_call(DeeObject *__restrict self,
                size_t argc, DeeObject **__restrict argv) {
 return class_call(Dee_TYPE(self),self,argc,argv);
}
INTERN UNARY_WRAPPER(dhash_t,hash)
INTERN UNARY_WRAPPER(DREF DeeObject *,iter_next)
INTERN UNARY_WRAPPER(DREF DeeObject *,int)
INTERN int DCALL
class_wrap_double(DeeObject *__restrict self,
                  double *__restrict presult) {
 return class_double(Dee_TYPE(self),self,presult);
}
INTERN UNARY_WRAPPER(DREF DeeObject *,inv)
INTERN UNARY_WRAPPER(DREF DeeObject *,pos)
INTERN UNARY_WRAPPER(DREF DeeObject *,neg)
INTERN BINARY_WRAPPER(DREF DeeObject *,add)
INTERN BINARY_WRAPPER(DREF DeeObject *,sub)
INTERN BINARY_WRAPPER(DREF DeeObject *,mul)
INTERN BINARY_WRAPPER(DREF DeeObject *,div)
INTERN BINARY_WRAPPER(DREF DeeObject *,mod)
INTERN BINARY_WRAPPER(DREF DeeObject *,shl)
INTERN BINARY_WRAPPER(DREF DeeObject *,shr)
INTERN BINARY_WRAPPER(DREF DeeObject *,and)
INTERN BINARY_WRAPPER(DREF DeeObject *,or)
INTERN BINARY_WRAPPER(DREF DeeObject *,xor)
INTERN BINARY_WRAPPER(DREF DeeObject *,pow)
INTERN UNARY_INPLACE_WRAPPER(int,inc)
INTERN UNARY_INPLACE_WRAPPER(int,dec)
INTERN BINARY_INPLACE_WRAPPER(int,inplace_add)
INTERN BINARY_INPLACE_WRAPPER(int,inplace_sub)
INTERN BINARY_INPLACE_WRAPPER(int,inplace_mul)
INTERN BINARY_INPLACE_WRAPPER(int,inplace_div)
INTERN BINARY_INPLACE_WRAPPER(int,inplace_mod)
INTERN BINARY_INPLACE_WRAPPER(int,inplace_shl)
INTERN BINARY_INPLACE_WRAPPER(int,inplace_shr)
INTERN BINARY_INPLACE_WRAPPER(int,inplace_and)
INTERN BINARY_INPLACE_WRAPPER(int,inplace_or)
INTERN BINARY_INPLACE_WRAPPER(int,inplace_xor)
INTERN BINARY_INPLACE_WRAPPER(int,inplace_pow)
INTERN BINARY_WRAPPER(DREF DeeObject *,eq)
INTERN BINARY_WRAPPER(DREF DeeObject *,ne)
INTERN BINARY_WRAPPER(DREF DeeObject *,lo)
INTERN BINARY_WRAPPER(DREF DeeObject *,le)
INTERN BINARY_WRAPPER(DREF DeeObject *,gr)
INTERN BINARY_WRAPPER(DREF DeeObject *,ge)
INTERN UNARY_WRAPPER(DREF DeeObject *,iter_self)
INTERN UNARY_WRAPPER(DREF DeeObject *,size)
INTERN BINARY_WRAPPER(DREF DeeObject *,contains)
INTERN BINARY_WRAPPER(DREF DeeObject *,getitem)
INTERN BINARY_WRAPPER(int,delitem)
INTERN TRINARY_WRAPPER(int,setitem)
INTERN TRINARY_WRAPPER(DREF DeeObject *,getrange)
INTERN TRINARY_WRAPPER(int,delrange)
INTERN QUAD_WRAPPER(int,setrange)
INTERN BINARY_WRAPPER(DREF DeeObject *,getattr)
INTERN BINARY_WRAPPER(int,delattr)
INTERN TRINARY_WRAPPER(int,setattr)
INTERN UNARY_WRAPPER(int,enter)
INTERN UNARY_WRAPPER(int,leave)
#undef QUAD_WRAPPER
#undef TRINARY_WRAPPER
#undef BINARY_INPLACE_WRAPPER
#undef UNARY_INPLACE_WRAPPER
#undef BINARY_WRAPPER
#undef UNARY_WRAPPER


struct opwrapper {
 uintptr_t offset;  /* Offset from the containing operator table to where `wrapper' must be written. */
 void     *wrapper; /* [0..1] The C wrapper function that should be assigned to
                     *         the operator to have it invoke user-callbacks.
                     *   NOTE: A value of NULL in this field acts as a sentinel. */
};

/* Operator wrapper descriptor tables. */
PRIVATE struct opwrapper type_wrappers[] = {
    /* [OPERATOR_CONSTRUCTOR - OPERATOR_TYPEMIN] = */{ offsetof(Type,tp_init.tp_alloc.tp_any_ctor), (void *)&class_wrap_ctor },
    /* [OPERATOR_COPY - OPERATOR_TYPEMIN]        = */{ offsetof(Type,tp_init.tp_alloc.tp_copy_ctor), (void *)&class_wrap_copy },
    /* [OPERATOR_DEEPCOPY - OPERATOR_TYPEMIN]    = */{ offsetof(Type,tp_init.tp_alloc.tp_deep_ctor), (void *)&class_wrap_deep },
    /* [OPERATOR_DESTRUCTOR - OPERATOR_TYPEMIN]  = */{ offsetof(Type,tp_init.tp_dtor), &class_wrap_dtor },
    /* [OPERATOR_ASSIGN - OPERATOR_TYPEMIN]      = */{ offsetof(Type,tp_init.tp_assign), (void *)&class_wrap_assign },
    /* [OPERATOR_MOVEASSIGN - OPERATOR_TYPEMIN]  = */{ offsetof(Type,tp_init.tp_move_assign), (void *)&class_wrap_move_assign },
    /* [OPERATOR_STR - OPERATOR_TYPEMIN]         = */{ offsetof(Type,tp_cast.tp_str), (void *)&class_wrap_str },
    /* [OPERATOR_REPR - OPERATOR_TYPEMIN]        = */{ offsetof(Type,tp_cast.tp_repr), (void *)&class_wrap_repr },
    /* [OPERATOR_BOOL - OPERATOR_TYPEMIN]        = */{ offsetof(Type,tp_cast.tp_bool), (void *)&class_wrap_bool },
    /* [OPERATOR_CALL - OPERATOR_TYPEMIN]        = */{ offsetof(Type,tp_call), (void *)&class_wrap_call },
    /* [OPERATOR_ITERNEXT - OPERATOR_TYPEMIN]    = */{ offsetof(Type,tp_iter_next), (void *)&class_wrap_iter_next }
};
PRIVATE struct opwrapper math_wrappers[] = {
    /* [OPERATOR_INT - OPERATOR_MATHMIN]         = */{ offsetof(struct type_math,tp_int), (void *)&class_wrap_int },
    /* [OPERATOR_FLOAT - OPERATOR_MATHMIN]       = */{ offsetof(struct type_math,tp_double), (void *)&class_wrap_double },
    /* [OPERATOR_INV - OPERATOR_MATHMIN]         = */{ offsetof(struct type_math,tp_inv), (void *)&class_wrap_inv },
    /* [OPERATOR_POS - OPERATOR_MATHMIN]         = */{ offsetof(struct type_math,tp_pos), (void *)&class_wrap_pos },
    /* [OPERATOR_NEG - OPERATOR_MATHMIN]         = */{ offsetof(struct type_math,tp_neg), (void *)&class_wrap_neg },
    /* [OPERATOR_ADD - OPERATOR_MATHMIN]         = */{ offsetof(struct type_math,tp_add), (void *)&class_wrap_add },
    /* [OPERATOR_SUB - OPERATOR_MATHMIN]         = */{ offsetof(struct type_math,tp_sub), (void *)&class_wrap_sub },
    /* [OPERATOR_MUL - OPERATOR_MATHMIN]         = */{ offsetof(struct type_math,tp_mul), (void *)&class_wrap_mul },
    /* [OPERATOR_DIV - OPERATOR_MATHMIN]         = */{ offsetof(struct type_math,tp_div), (void *)&class_wrap_div },
    /* [OPERATOR_MOD - OPERATOR_MATHMIN]         = */{ offsetof(struct type_math,tp_mod), (void *)&class_wrap_mod },
    /* [OPERATOR_SHL - OPERATOR_MATHMIN]         = */{ offsetof(struct type_math,tp_shl), (void *)&class_wrap_shl },
    /* [OPERATOR_SHR - OPERATOR_MATHMIN]         = */{ offsetof(struct type_math,tp_shr), (void *)&class_wrap_shr },
    /* [OPERATOR_AND - OPERATOR_MATHMIN]         = */{ offsetof(struct type_math,tp_and), (void *)&class_wrap_and },
    /* [OPERATOR_OR - OPERATOR_MATHMIN]          = */{ offsetof(struct type_math,tp_or), (void *)&class_wrap_or },
    /* [OPERATOR_XOR - OPERATOR_MATHMIN]         = */{ offsetof(struct type_math,tp_xor), (void *)&class_wrap_xor },
    /* [OPERATOR_POW - OPERATOR_MATHMIN]         = */{ offsetof(struct type_math,tp_pow), (void *)&class_wrap_pow },
    /* [OPERATOR_INC - OPERATOR_MATHMIN]         = */{ offsetof(struct type_math,tp_inc), (void *)&class_wrap_inc },
    /* [OPERATOR_DEC - OPERATOR_MATHMIN]         = */{ offsetof(struct type_math,tp_dec), (void *)&class_wrap_dec },
    /* [OPERATOR_INPLACE_ADD - OPERATOR_MATHMIN] = */{ offsetof(struct type_math,tp_inplace_add), (void *)&class_wrap_inplace_add },
    /* [OPERATOR_INPLACE_SUB - OPERATOR_MATHMIN] = */{ offsetof(struct type_math,tp_inplace_sub), (void *)&class_wrap_inplace_sub },
    /* [OPERATOR_INPLACE_MUL - OPERATOR_MATHMIN] = */{ offsetof(struct type_math,tp_inplace_mul), (void *)&class_wrap_inplace_mul },
    /* [OPERATOR_INPLACE_DIV - OPERATOR_MATHMIN] = */{ offsetof(struct type_math,tp_inplace_div), (void *)&class_wrap_inplace_div },
    /* [OPERATOR_INPLACE_MOD - OPERATOR_MATHMIN] = */{ offsetof(struct type_math,tp_inplace_mod), (void *)&class_wrap_inplace_mod },
    /* [OPERATOR_INPLACE_SHL - OPERATOR_MATHMIN] = */{ offsetof(struct type_math,tp_inplace_shl), (void *)&class_wrap_inplace_shl },
    /* [OPERATOR_INPLACE_SHR - OPERATOR_MATHMIN] = */{ offsetof(struct type_math,tp_inplace_shr), (void *)&class_wrap_inplace_shr },
    /* [OPERATOR_INPLACE_AND - OPERATOR_MATHMIN] = */{ offsetof(struct type_math,tp_inplace_and), (void *)&class_wrap_inplace_and },
    /* [OPERATOR_INPLACE_OR - OPERATOR_MATHMIN]  = */{ offsetof(struct type_math,tp_inplace_or), (void *)&class_wrap_inplace_or },
    /* [OPERATOR_INPLACE_XOR - OPERATOR_MATHMIN] = */{ offsetof(struct type_math,tp_inplace_xor), (void *)&class_wrap_inplace_xor },
    /* [OPERATOR_INPLACE_POW - OPERATOR_MATHMIN] = */{ offsetof(struct type_math,tp_inplace_pow), (void *)&class_wrap_inplace_pow }
};
PRIVATE struct opwrapper cmp_wrappers[] = {
    /* [OPERATOR_HASH - OPERATOR_CMPMIN] = */{ offsetof(struct type_cmp,tp_hash), (void *)&class_wrap_hash },
    /* [OPERATOR_EQ - OPERATOR_CMPMIN]   = */{ offsetof(struct type_cmp,tp_eq), (void *)&class_wrap_eq },
    /* [OPERATOR_NE - OPERATOR_CMPMIN]   = */{ offsetof(struct type_cmp,tp_ne), (void *)&class_wrap_ne },
    /* [OPERATOR_LO - OPERATOR_CMPMIN]   = */{ offsetof(struct type_cmp,tp_lo), (void *)&class_wrap_lo },
    /* [OPERATOR_LE - OPERATOR_CMPMIN]   = */{ offsetof(struct type_cmp,tp_le), (void *)&class_wrap_le },
    /* [OPERATOR_GR - OPERATOR_CMPMIN]   = */{ offsetof(struct type_cmp,tp_gr), (void *)&class_wrap_gr },
    /* [OPERATOR_GE - OPERATOR_CMPMIN]   = */{ offsetof(struct type_cmp,tp_ge), (void *)&class_wrap_ge }
};
PRIVATE struct opwrapper seq_wrappers[] = {
    /* [OPERATOR_ITERSELF - OPERATOR_SEQMIN] = */{ offsetof(struct type_seq,tp_iter_self), (void *)&class_wrap_iter_self },
    /* [OPERATOR_SIZE     - OPERATOR_SEQMIN] = */{ offsetof(struct type_seq,tp_size), (void *)&class_wrap_size },
    /* [OPERATOR_CONTAINS - OPERATOR_SEQMIN] = */{ offsetof(struct type_seq,tp_contains), (void *)&class_wrap_contains },
    /* [OPERATOR_GETITEM  - OPERATOR_SEQMIN] = */{ offsetof(struct type_seq,tp_get), (void *)&class_wrap_getitem },
    /* [OPERATOR_DELITEM  - OPERATOR_SEQMIN] = */{ offsetof(struct type_seq,tp_del), (void *)&class_wrap_delitem },
    /* [OPERATOR_SETITEM  - OPERATOR_SEQMIN] = */{ offsetof(struct type_seq,tp_set), (void *)&class_wrap_setitem },
    /* [OPERATOR_GETRANGE - OPERATOR_SEQMIN] = */{ offsetof(struct type_seq,tp_range_get), (void *)&class_wrap_getrange },
    /* [OPERATOR_DELRANGE - OPERATOR_SEQMIN] = */{ offsetof(struct type_seq,tp_range_del), (void *)&class_wrap_delrange },
    /* [OPERATOR_SETRANGE - OPERATOR_SEQMIN] = */{ offsetof(struct type_seq,tp_range_set), (void *)&class_wrap_setrange }
};
PRIVATE struct opwrapper attr_wrappers[] = {
    /* [OPERATOR_GETATTR  - OPERATOR_ATTRMIN] = */{ offsetof(struct type_attr,tp_getattr), (void *)&class_wrap_getattr },
    /* [OPERATOR_DELATTR  - OPERATOR_ATTRMIN] = */{ offsetof(struct type_attr,tp_delattr), (void *)&class_wrap_delattr },
    /* [OPERATOR_SETATTR  - OPERATOR_ATTRMIN] = */{ offsetof(struct type_attr,tp_setattr), (void *)&class_wrap_setattr },
    /* [OPERATOR_ENUMATTR - OPERATOR_ATTRMIN] = */{ offsetof(struct type_attr,tp_enumattr), (void *)&class_enumattr }
};
PRIVATE struct opwrapper with_wrappers[] = {
    /* [OPERATOR_ENTER - OPERATOR_WITHMIN] = */{ offsetof(struct type_with,tp_enter), (void *)&class_wrap_enter },
    /* [OPERATOR_LEAVE - OPERATOR_WITHMIN] = */{ offsetof(struct type_with,tp_leave), (void *)&class_wrap_leave },
};


PRIVATE int DCALL
lazy_allocate(void **ptable, size_t table_size) {
 void *new_table;
 if (*ptable) return 0;
 new_table = Dee_Calloc(table_size);
 if unlikely(!new_table) return -1;
#ifdef CONFIG_NO_THREADS
 if unlikely(*ptable)
    Dee_Free(new_table);
 else *ptable = new_table;
#else /* CONFIG_NO_THREADS */
 if unlikely(!ATOMIC_CMPXCH(*ptable,NULL,new_table))
    Dee_Free(new_table);
#endif /* !CONFIG_NO_THREADS */
 return 0;
}

#ifndef __INTELLISENSE__
#ifndef __NO_builtin_expect
#define lazy_allocate(ptable,table_size) \
        __builtin_expect(lazy_allocate(ptable,table_size),0)
#endif
#endif

#define LAZY_ALLOCATE(p) lazy_allocate((void **)&(p),sizeof(*(p)))


PUBLIC int (DCALL DeeClass_SetOperator)(DeeTypeObject *__restrict self,
                                        unsigned int name,
                                        DeeObject *__restrict callback) {
 int result = 0;
 struct class_desc *desc = DeeClass_DESC(self);
again:
 INSTANCE_DESC_WRITE(&desc->c_class);
 if (name < CLASS_OPERATOR_USERCOUNT) {
  DREF DeeObject **slot; void **target,*wrapper;
  struct class_optable *table; /* Basic operators. */
  unsigned int table_index = name/CLASS_HEADER_OPC2;
  ASSERT(table_index < CLASS_HEADER_OPC1);
  table = desc->c_ops[table_index];
  if (!table) {
   /* Create a new table for this operator. */
   table = (struct class_optable *)Dee_TryCalloc(sizeof(struct class_optable));
   if unlikely(!table) {
    INSTANCE_DESC_ENDWRITE(&desc->c_class);
    if (Dee_CollectMemory(sizeof(struct class_optable))) goto again;
    return -1;
   }
   /* Save the new table as the one that will be used for this group. */
   ASSERT(!desc->c_ops[table_index]);
   desc->c_ops[table_index] = table;
  }
  /* Get a pointer to the operator callback slot. */
  slot = &table->co_operators[name % CLASS_HEADER_OPC2];
  /* Store the given operator callback. */
  if (*slot) goto already_assigned;
  *slot = callback;
  Dee_Incref(callback);

  /* Assign operator wrapper. */
  if (name <= OPERATOR_TYPEMAX) {
   /* type operator. */
   wrapper = type_wrappers[name - OPERATOR_TYPEMIN].wrapper;
   target  = (void **)((uintptr_t)self+type_wrappers[name - OPERATOR_TYPEMIN].offset);
  } else if (name <= OPERATOR_MATHMAX) {
   if (LAZY_ALLOCATE(self->tp_math))
       return -1;
   /* math operator. */
   wrapper = math_wrappers[name - OPERATOR_MATHMIN].wrapper;
   target  = (void **)((uintptr_t)self->tp_math+math_wrappers[name - OPERATOR_MATHMIN].offset);
  } else if (name <= OPERATOR_CMPMAX) {
   if (LAZY_ALLOCATE(self->tp_cmp))
       return -1;
   /* compare operator. */
   wrapper = cmp_wrappers[name - OPERATOR_CMPMIN].wrapper;
   target  = (void **)((uintptr_t)self->tp_cmp+cmp_wrappers[name - OPERATOR_CMPMIN].offset);
  } else if (name <= OPERATOR_SEQMAX) {
   if (LAZY_ALLOCATE(self->tp_seq))
       return -1;
   /* compare operator. */
   wrapper = seq_wrappers[name - OPERATOR_SEQMIN].wrapper;
   target  = (void **)((uintptr_t)self->tp_seq+seq_wrappers[name - OPERATOR_SEQMIN].offset);
  } else if (name <= OPERATOR_ATTRMAX) {
   if (LAZY_ALLOCATE(self->tp_attr))
       return -1;
   /* attribute operator. */
   wrapper = attr_wrappers[name - OPERATOR_ATTRMIN].wrapper;
   target  = (void **)((uintptr_t)self->tp_attr+attr_wrappers[name - OPERATOR_ATTRMIN].offset);
  } else if (name <= OPERATOR_WITHMAX) {
   if (LAZY_ALLOCATE(self->tp_with))
       return -1;
   /* with operators. */
   wrapper = with_wrappers[name - OPERATOR_WITHMIN].wrapper;
   target  = (void **)((uintptr_t)self->tp_with+with_wrappers[name - OPERATOR_WITHMIN].offset);
  } else {
   ASSERT(name == CLASS_OPERATOR_SUPERARGS);
   /* Special case: The the superargs operator. */
   wrapper = (void *)&class_ctor;
   target  = (void **)&self->tp_init.tp_alloc.tp_any_ctor;
  }
  /* Assign the proper wrapper to the target. */
  *target = wrapper;
  /* Special handling for some specific operators. */
  switch (name) {
  case OPERATOR_CONSTRUCTOR:
  case OPERATOR_COPY:
  case OPERATOR_DEEPCOPY:
   /* TODO: Update interactions between these 3. */
   break;
  default: break;
  }
  goto done;
 }
 /* TODO: Extended operators (`DeeFileType_Type', etc...). */
 goto unknown_operator;

done:
 INSTANCE_DESC_ENDWRITE(&desc->c_class);
 return result;
already_assigned:
 INSTANCE_DESC_ENDWRITE(&desc->c_class);
 DeeError_Throwf(&DeeError_ValueError,
                 "Operator #%X has already been assigned",
                 name);
 return -1;
unknown_operator:
 INSTANCE_DESC_ENDWRITE(&desc->c_class);
 DeeError_Throwf(&DeeError_ValueError,"Unknown operator #%X",name);
 return -1;
}

PUBLIC void DCALL
DeeClass_SetMember(DeeTypeObject *__restrict self,
                   unsigned int index,
                   DeeObject *__restrict value) {
 DREF DeeObject *old_value;
 struct class_desc *desc = DeeClass_DESC(self);
 ASSERT_OBJECT(value);
 ASSERT(index < desc->c_cmem->mt_size);
 INSTANCE_DESC_WRITE(&desc->c_class);
 old_value = desc->c_class.ih_vtab[index];
 desc->c_class.ih_vtab[index] = value;
 Dee_Incref(value);
 INSTANCE_DESC_ENDWRITE(&desc->c_class);
 Dee_XDecref(old_value);
}



/* Default instance operators.
 * >> These are the implementation of auto-generated, general-purpose operators,
 *    such as the builtin copy, deepcopy, assign, etc. operators. */
INTERN int DCALL
class_defl_copy(DeeTypeObject *__restrict tp_self,
                DeeObject *__restrict self,
                DeeObject *__restrict other) {
 size_t num_members;
 struct class_desc *desc = DeeClass_DESC(tp_self);
 if (init_super_copy(tp_self,self,other,false))
     return -1;
 /* With the super-class now constructed,
  * it's time to do our own initialization. */
 num_members = desc->c_mem->mt_size;
 if (num_members) {
  struct instance_desc *instance;
  struct instance_desc *other_instance;
  instance       = DeeInstance_DESC(desc,self);
  other_instance = DeeInstance_DESC(desc,other);
#ifndef CONFIG_NO_THREADS
  rwlock_init(&instance->ih_lock);
#endif
  INSTANCE_DESC_READ(other_instance);
  /* Reference all variables from the other instance in this one. */
  MEMCPY_PTR(instance->ih_vtab,
             other_instance->ih_vtab,num_members);
  while (num_members--)
      Dee_XIncref(instance->ih_vtab[num_members]);
  INSTANCE_DESC_ENDREAD(other_instance);
 }
 return 0;
}
INTERN int DCALL
class_wrap_defl_copy(DeeObject *__restrict self,
                     DeeObject *__restrict other) {
 return class_defl_copy(Dee_TYPE(self),self,other);
}

INTERN int DCALL
class_defl_deepcopy(DeeTypeObject *__restrict tp_self,
                    DeeObject *__restrict self,
                    DeeObject *__restrict other) {
 size_t i,num_members;
 struct instance_desc *instance;
 struct instance_desc *other_instance;
 struct class_desc *desc = DeeClass_DESC(tp_self);
 if (init_super_copy(tp_self,self,other,true))
     return -1;
 /* With the super-class now constructed,
  * it's time to do our own initialization. */
 num_members = desc->c_mem->mt_size;
 if (!num_members) return 0;
 instance       = DeeInstance_DESC(desc,self);
 other_instance = DeeInstance_DESC(desc,other);
#ifndef CONFIG_NO_THREADS
 rwlock_init(&instance->ih_lock);
#endif
 INSTANCE_DESC_READ(other_instance);
 /* Reference all variables from the other instance in this one. */
 MEMCPY_PTR(instance->ih_vtab,
            other_instance->ih_vtab,num_members);
 for (i = 0; i < num_members; ++i)
      Dee_XIncref(instance->ih_vtab[i]);
 INSTANCE_DESC_ENDREAD(other_instance);
 /* Deep-load underlying types. */
 if unlikely(load_super_deep(tp_self,self,other))
    goto err_destroy_bases;
 /* Replace all members with deep copies of themself. */
 for (i = 0; i < num_members; ++i) {
  if (DeeObject_InplaceXDeepCopyWithLock(&instance->ih_vtab[i],
                                         &instance->ih_lock))
      goto err_destroy_bases;
 }
 return 0;
err_destroy_bases:
 /* Destroy everything we've managed to copy thus far. */
 while (num_members--)
     Dee_XDecref(instance->ih_vtab[num_members]);
 /* This is kind-of complicated, because we must
  * manually invoke destructors of base classes. */
 if (!DeeObject_UndoConstruction(DeeType_Base(tp_self),self)) {
  DeeError_Print(str_shared_ctor_failed,
                 ERROR_PRINT_DOHANDLE);
  return 0;
 }
 return -1;
}
INTERN int DCALL
class_wrap_defl_deepcopy(DeeObject *__restrict self,
                         DeeObject *__restrict other) {
 return class_defl_deepcopy(Dee_TYPE(self),self,other);
}

/* Operators always assign to class types (required for GC, etc.) */
INTERN void DCALL
instance_tvisit(DeeTypeObject *__restrict tp_self,
                DeeObject *__restrict self,
                dvisit_t proc, void *arg) {
 struct class_desc *desc;
 struct instance_desc *inst;
 size_t i,num_members;
 ASSERT(DeeType_IsClass(tp_self));
 desc = DeeClass_DESC(tp_self);
 num_members = desc->c_mem->mt_size;
 if likely(num_members) {
  inst = DeeInstance_DESC(desc,self);
  INSTANCE_DESC_READ(inst);
  for (i = 0; i < num_members; ++i)
       Dee_XVisit(inst->ih_vtab[i]);
  INSTANCE_DESC_ENDREAD(inst);
 }
}

INTERN void DCALL
instance_visit(DeeObject *__restrict self,
               dvisit_t proc, void *arg) {
 DeeTypeObject *iter = Dee_TYPE(self);
 do {
  /* Redundant implementation that visits everything multiple times. */
  if (DeeType_IsClass(iter))
      instance_tvisit(iter,self,proc,arg);
 } while ((iter = DeeType_Base(iter)) != NULL);
}


INTERN void DCALL
instance_tclear(DeeTypeObject *__restrict tp_self,
                DeeObject *__restrict self) {
 /* Called from `DeeType_Type.tp_gc' */
 struct class_desc *desc = CLASS_DESC(tp_self);
 struct instance_desc *inst = DeeInstance_DESC(desc,self);
 size_t count,decref_laterc = 0;
 DREF DeeObject **iter,**end,*ob;
 DREF DeeObject **decref_laterv = NULL;
 DREF DeeObject **new_decref_laterv;
 count = desc->c_mem->mt_size;
 if unlikely(!count) return;
again:
 INSTANCE_DESC_WRITE(inst);
 end = (iter = inst->ih_vtab)+count;
 for (; iter != end; ++iter) {
  if ((ob = *iter) == NULL) continue;
  *iter = NULL; /* Inherit reference. */
  if (!Dee_DecrefIfNotOne(ob)) {
   new_decref_laterv = (DREF DeeObject **)DeeObject_TryRealloc(decref_laterv,(decref_laterc+1)*
                                                               sizeof(DREF DeeObject *));
   if unlikely(!new_decref_laterv) goto do_decref_now;
   decref_laterv = new_decref_laterv;
   decref_laterv[decref_laterc++] = ob; /* Drop this reference later. */
  }
 }
 INSTANCE_DESC_ENDWRITE(inst);
 /* Drop all saved references. */
 while (decref_laterc--) Dee_Decref(decref_laterv[decref_laterc]);
 Dee_Free(decref_laterv);
 return;
do_decref_now:
 INSTANCE_DESC_ENDWRITE(inst);
 Dee_Decref(ob);
 while (decref_laterc--) Dee_Decref(decref_laterv[decref_laterc]);
 Dee_Free(decref_laterv);
 goto again;
}
INTERN void DCALL
instance_clear(DeeObject *__restrict self) {
 DeeTypeObject *iter = Dee_TYPE(self);
 do {
  /* Redundant implementation that visits everything multiple times. */
  if (DeeType_IsClass(iter))
      instance_tclear(iter,self);
 } while ((iter = DeeType_Base(iter)) != NULL);
}

INTERN void DCALL
instance_tpclear(DeeTypeObject *__restrict tp_self,
                 DeeObject *__restrict self,
                 unsigned int priority) {
 /* Called from `DeeType_Type.tp_gc' */
 struct class_desc *desc = CLASS_DESC(tp_self);
 struct instance_desc *inst = DeeInstance_DESC(desc,self);
 size_t count; DREF DeeObject *ob,**iter,**end;
 count = desc->c_mem->mt_size;
 if unlikely(!count) return;
 INSTANCE_DESC_WRITE(inst);
 end = (iter = inst->ih_vtab)+count;
 for (; iter != end; ++iter) {
  if ((ob = *iter) == NULL) continue;
  if (DeeObject_GCPriority(ob) < priority) continue;
  *iter = NULL; /* Inherit reference. */
  INSTANCE_DESC_ENDWRITE(inst);
  Dee_Decref(ob);
  INSTANCE_DESC_WRITE(inst);
 }
 INSTANCE_DESC_ENDWRITE(inst);
}
INTERN void DCALL
instance_pclear(DeeObject *__restrict self, unsigned int priority) {
 DeeTypeObject *iter = Dee_TYPE(self);
 do {
  /* Redundant implementation that visits everything multiple times. */
  if (DeeType_IsClass(iter))
      instance_tpclear(iter,self,priority);
 } while ((iter = DeeType_Base(iter)) != NULL);
}


PRIVATE struct type_gc instance_gc = {
    /* .tp_clear  = */(void(DCALL *)(DeeObject *__restrict))&instance_clear,
    /* .tp_pclear = */(void(DCALL *)(DeeObject *__restrict,unsigned int))&instance_pclear,
    /* .tp_gcprio = */GC_PRIORITY_INSTANCE
};

PUBLIC DREF DeeTypeObject *DCALL
DeeClass_New(DeeTypeObject *type,
             DeeTypeObject *base,
             /*StringObject*/DeeObject *name,
             /*StringObject*/DeeObject *doc,
             /*MemberTable*/DeeObject *instance_members,
             /*MemberTable*/DeeObject *class_members,
             uint16_t flags) {
 DREF DeeTypeObject *result;
 uintptr_t offsetof_class_descriptor,offsetof_instance_descriptor;
 size_t sizeof_class_descriptor,sizeof_instance_descriptor;
 struct class_desc *desc;
 /* Automatically deduce the class's type from its base, or create a default type. */
 if (!type) type = base ? Dee_TYPE(base) : &DeeType_Type;
 /* Can't use NULL for member tables, so use symbolic, empty tables instead. */
 if (!instance_members) instance_members = (DeeObject *)&DeeMemberTable_Empty;
 if (!class_members) class_members = (DeeObject *)&DeeMemberTable_Empty;
 ASSERT_OBJECT_TYPE(type,&DeeType_Type);
 ASSERT_OBJECT_TYPE_EXACT_OPT(name,&DeeString_Type);
 ASSERT_OBJECT_TYPE_EXACT_OPT(doc,&DeeString_Type);
 ASSERT_OBJECT_TYPE(class_members,&DeeMemberTable_Type);
 ASSERT_OBJECT_TYPE(instance_members,&DeeMemberTable_Type);
 /* Make sure that the given class-type type is actually a type-type.
  * AGAIN: A type is something like `DeeObject_Type' or `DeeString_type'
  *        And a type-type is a type for types, like `DeeType_Type' or `DeeFileType_Type'.
  *        Type-types are usually recognizable by the double `type' in their name,
  *        thus crowning the terminology of calling them type-types. */
 ASSERTF(DeeType_IsTypeType(type),"Type `%s' is not a type-type",type->tp_name);
 ASSERT_OBJECT_TYPE_OPT(base,type);
 ASSERTF(!(type->tp_flags&TP_FVARIABLE),"Type-types must not be variable-sized");
 ASSERTF(type->tp_flags&TP_FGC,"Type-types must be GC objects");
 ASSERTF(type->tp_init.tp_alloc.tp_ctor ||
         type->tp_init.tp_alloc.tp_any_ctor,
         "Type `%s' could not be instantiated",type->tp_name);
 /* Make sure the base type can actually be used to create user-code sub-classes. */
 if (base && (base->tp_flags&(TP_FVARIABLE|TP_FFINAL) ||
              base->tp_init.tp_alloc.tp_free != NULL)) {
  DeeError_Throwf(&DeeError_TypeError,
                  "Cannot create sub-class of %s type `%k'",
                  base->tp_flags&TP_FVARIABLE ? "variable-length" :
                  base->tp_init.tp_alloc.tp_free != NULL ? "custom-allocated" : "final",base);
  return NULL;
 }
 offsetof_class_descriptor    = type->tp_init.tp_alloc.tp_instance_size;
 /* TODO: Automatically add weakref support to the class if its base isn't already implementing it.
  *       Note that we should probably add some kind of option to this function call, allowing the
  *       caller to choose if this feature should be enabled or not...
  * NOTE: Weakref support should be located before the initial instance descriptor. */
 offsetof_instance_descriptor = base ? base->tp_init.tp_alloc.tp_instance_size : sizeof(DeeObject);
 sizeof_class_descriptor      = (offsetof(struct class_desc,c_class.ih_vtab)+
                                ((MemberTable *)class_members)->mt_size*sizeof(DREF DeeObject *));
 sizeof_instance_descriptor   = (offsetof(struct instance_desc,ih_vtab)+
                                ((MemberTable *)instance_members)->mt_size*sizeof(DREF DeeObject *));
#ifndef CONFIG_NO_THREAD /* offsetof(struct instance_desc,ih_vtab) != 0 */
 /* Without any members, instances of this class don't need a lock. */
 if (((MemberTable *)instance_members)->mt_size == 0)
       sizeof_instance_descriptor = 0;
#endif
 result = (DREF DeeTypeObject *)DeeGCObject_Malloc(offsetof_class_descriptor+
                                                   sizeof_class_descriptor);
 if unlikely(!result) return NULL;
 DeeObject_Init(result,type);
 /* Initialize the underlying type. */
 if unlikely(type->tp_init.tp_alloc.tp_ctor
  ? DeeType_INVOKE_ALLOC_CTOR(type,type,(DeeObject *)result)
  : DeeType_INVOKE_ALLOC_ANY(type,type,(DeeObject *)result,0,NULL))
    goto err_r;
 ASSERTF(result->tp_flags & TP_FHEAP,
         "Type constructor for `%s' did not set the FHEAP flag",
         type->tp_name);
 /* Get the descriptor and start setting up it. */
 desc = (struct class_desc *)((uintptr_t)result+offsetof_class_descriptor);
 memset(desc,0,sizeof_class_descriptor); /* ZERO-initialize the descriptor. */
 result->tp_class = desc; /* Save the address of the class descriptor. */
 desc->c_mem  = (DREF MemberTable *)instance_members;
 desc->c_cmem = (DREF MemberTable *)class_members;
#ifndef CONFIG_NO_THREADS
 rwlock_cinit(&desc->c_class.ih_lock);
#endif
 Dee_Incref(instance_members);
 Dee_Incref(class_members);
 desc->c_addr = offsetof_instance_descriptor;
 /* Initialize type members and assign automatic operators. */
 result->tp_init.tp_alloc.tp_any_ctor      = &class_wrap_ctor;
 result->tp_init.tp_alloc.tp_copy_ctor     = &class_wrap_defl_copy;
 result->tp_init.tp_alloc.tp_deep_ctor     = &class_wrap_defl_deepcopy;
 result->tp_init.tp_alloc.tp_instance_size = (offsetof_instance_descriptor+
                                              sizeof_instance_descriptor);
 if (sizeof_instance_descriptor) {
  /* Only if instances of this class actually have a descriptor
   * must be set a destructor, and visit + clear callbacks. */
  result->tp_init.tp_dtor = &defl_dtor;
  result->tp_visit        = &instance_visit;
  result->tp_gc           = &instance_gc;
 }
 result->tp_base  = base;
 /* TODO: Compiler-generated: tp_hash (member-wise) */
 /* TODO: Compiler-generated: tp_eq (member-wise) */
 /* TODO: Compiler-generated: tp_ne (member-wise) */
 Dee_XIncref(base);
 /* Apply flags and name/doc objects.
  * NOTE: All class types are _always_ GC objects because users
  *       cannot be trusted not to accidentally, or intentionally
  *       create reference loops that could otherwise not be resolved. */
 result->tp_flags |= TP_FGC|(flags&0xf);
 /* Add flags that are being inherited. */
 if (base) result->tp_flags |= base->tp_flags&TP_FINTERHITABLE;
 if (name) {
  result->tp_name = DeeString_STR(name);
  result->tp_flags |= TP_FNAMEOBJECT;
  Dee_Incref(name);
 }
 if (doc) {
  result->tp_doc = DeeString_STR(doc);
  result->tp_flags |= TP_FDOCOBJECT;
  Dee_Incref(doc);
 }
 /* With it now fully initialized, start tracking this new type. */
 DeeGC_Track((DeeObject *)result);
 return result;
#if 0
err_r2:
 if (type->tp_init.tp_dtor)
   (*type->tp_init.tp_dtor)((DeeObject *)result);
 ASSERT(!DeeObject_IsShared(result));
#endif
err_r:
 Dee_Decref(type);
 DeeObject_Free(result);
 return NULL;
}

PRIVATE void
(DCALL err_unbound_member)(/*Class*/DeeTypeObject *__restrict class_type,
                           struct class_desc *__restrict desc,
                           unsigned int index) {
 /* Check if we can find the proper member so we can pass its name. */
 struct member_entry *iter,*end; char const *name = "??" "?";
 DeeMemberTableObject *table = desc->c_mem;
 end = (iter = table->mt_list)+(table->mt_mask+1);
 for (; iter != end; ++iter) {
  if (!iter->cme_name) continue;
  if (iter->cme_addr != index) continue;
  if (iter->cme_flag&CLASS_MEMBER_FCLASSMEM) continue;
  name = iter->cme_name->s_str;
  break;
 }
 /* Throw the error. */
 err_unbound_attribute(class_type,name);
}

PUBLIC DREF DeeObject *
(DCALL DeeInstance_GetMember)(/*Class*/DeeTypeObject *__restrict tp_self,
                              /*Instance*/DeeObject *__restrict self,
                              unsigned int index) {
 struct class_desc *desc;
 struct instance_desc *inst;
 DREF DeeObject *result;
 ASSERT_OBJECT_TYPE(tp_self,&DeeType_Type);
 ASSERT(DeeType_IsClass(tp_self));
 ASSERT_OBJECT_TYPE(self,tp_self);
 desc = DeeClass_DESC(tp_self);
 ASSERT(index <= desc->c_mem->mt_size);
 inst = DeeInstance_DESC(desc,self);
 /* Lock and extract the member. */
 INSTANCE_DESC_READ(inst);
 result = inst->ih_vtab[index];
 Dee_XIncref(result);
 INSTANCE_DESC_ENDREAD(inst);
 if (!result)
      err_unbound_member(tp_self,desc,index);
 return result;
}
PUBLIC bool
(DCALL DeeInstance_BoundMember)(/*Class*/DeeTypeObject *__restrict tp_self,
                                /*Instance*/DeeObject *__restrict self,
                                unsigned int index) {
 struct class_desc *desc;
 struct instance_desc *inst;
 ASSERT_OBJECT_TYPE(tp_self,&DeeType_Type);
 ASSERT(DeeType_IsClass(tp_self));
 ASSERT_OBJECT_TYPE(self,tp_self);
 desc = DeeClass_DESC(tp_self);
 ASSERT(index <= desc->c_mem->mt_size);
 inst = DeeInstance_DESC(desc,self);
#ifdef CONFIG_NO_THREADS
 return inst->ih_vtab[index] != NULL;
#else
 return ATOMIC_READ(inst->ih_vtab[index]) != NULL;
#endif
}
PUBLIC int
(DCALL DeeInstance_DelMember)(/*Class*/DeeTypeObject *__restrict tp_self,
                              /*Instance*/DeeObject *__restrict self,
                              unsigned int index) {
 struct class_desc *desc;
 struct instance_desc *inst;
 DREF DeeObject *old_value;
 ASSERT_OBJECT_TYPE(tp_self,&DeeType_Type);
 ASSERT(DeeType_IsClass(tp_self));
 ASSERT_OBJECT_TYPE(self,tp_self);
 desc = DeeClass_DESC(tp_self);
 ASSERT(index <= desc->c_mem->mt_size);
 inst = DeeInstance_DESC(desc,self);
 /* Lock and extract the member. */
 INSTANCE_DESC_WRITE(inst);
 old_value = inst->ih_vtab[index];
 inst->ih_vtab[index] = NULL;
 INSTANCE_DESC_ENDWRITE(inst);
 if unlikely(!old_value) {
  err_unbound_member(tp_self,desc,index);
  return -1;
 }
 Dee_Decref(old_value);
 return 0;
}
PUBLIC void
(DCALL DeeInstance_SetMember)(/*Class*/DeeTypeObject *__restrict tp_self,
                              /*Instance*/DeeObject *__restrict self,
                              unsigned int index, DeeObject *__restrict value) {
 struct class_desc *desc;
 struct instance_desc *inst;
 DREF DeeObject *old_value;
 ASSERT_OBJECT_TYPE(tp_self,&DeeType_Type);
 ASSERT(DeeType_IsClass(tp_self));
 ASSERT_OBJECT_TYPE(self,tp_self);
 desc = DeeClass_DESC(tp_self);
 ASSERT(index <= desc->c_mem->mt_size);
 inst = DeeInstance_DESC(desc,self);
 /* Lock and extract the member. */
 Dee_Incref(value);
 INSTANCE_DESC_WRITE(inst);
 old_value = inst->ih_vtab[index];
 inst->ih_vtab[index] = value;
 INSTANCE_DESC_ENDWRITE(inst);
 Dee_XDecref(old_value);
}


INTERN DREF DeeObject *
(DCALL DeeInstance_GetMemberSafe)(DeeTypeObject *__restrict tp_self,
                                  DeeObject *__restrict self,
                                  unsigned int index) {
 if (DeeObject_AssertType((DeeObject *)tp_self,&DeeType_Type))
     goto err;
 if (DeeObject_AssertType(self,tp_self))
     goto err;
 if (!DeeType_IsClass(tp_self))
     goto err_req_class;
 if (index >= DeeClass_DESC(tp_self)->c_mem->mt_size)
     goto err_bad_index;
 return DeeInstance_GetMember(tp_self,self,index);
err_bad_index:
 err_invalid_class_index(tp_self,self,index);
 goto err;
err_req_class:
 err_requires_class(tp_self);
err:
 return NULL;
}
INTERN int
(DCALL DeeInstance_BoundMemberSafe)(DeeTypeObject *__restrict tp_self,
                                    DeeObject *__restrict self,
                                    unsigned int index) {
 if (DeeObject_AssertType((DeeObject *)tp_self,&DeeType_Type))
     goto err;
 if (DeeObject_AssertType(self,tp_self))
     goto err;
 if (!DeeType_IsClass(tp_self))
     goto err_req_class;
 if (index >= DeeClass_DESC(tp_self)->c_mem->mt_size)
     goto err_bad_index;
 return DeeInstance_BoundMember(tp_self,self,index);
err_bad_index:
 return err_invalid_class_index(tp_self,self,index);
err_req_class:
 return err_requires_class(tp_self);
err:
 return -1;
}
INTERN int
(DCALL DeeInstance_DelMemberSafe)(DeeTypeObject *__restrict tp_self,
                                  DeeObject *__restrict self,
                                  unsigned int index) {
 if (DeeObject_AssertType((DeeObject *)tp_self,&DeeType_Type))
     goto err;
 if (DeeObject_AssertType(self,tp_self))
     goto err;
 if (!DeeType_IsClass(tp_self))
     goto err_req_class;
 if (index >= DeeClass_DESC(tp_self)->c_mem->mt_size)
     goto err_bad_index;
 return DeeInstance_DelMember(tp_self,self,index);
err_bad_index:
 return err_invalid_class_index(tp_self,self,index);
err_req_class:
 return err_requires_class(tp_self);
err:
 return -1;
}
INTERN int
(DCALL DeeInstance_SetMemberSafe)(DeeTypeObject *__restrict tp_self,
                                  DeeObject *__restrict self,
                                  unsigned int index, DeeObject *__restrict value) {
 if (DeeObject_AssertType((DeeObject *)tp_self,&DeeType_Type))
     goto err;
 if (DeeObject_AssertType(self,tp_self))
     goto err;
 if (!DeeType_IsClass(tp_self))
     goto err_req_class;
 if (index >= DeeClass_DESC(tp_self)->c_mem->mt_size)
     goto err_bad_index;
 DeeInstance_SetMember(tp_self,self,index,value);
 return 0;
err_bad_index:
 return err_invalid_class_index(tp_self,self,index);
err_req_class:
 return err_requires_class(tp_self);
err:
 return -1;
}

DECL_END
#endif /* !CONFIG_USE_NEW_CLASS_SYSTEM */

#endif /* !GUARD_DEEMON_OBJECTS_CLASS_C */
