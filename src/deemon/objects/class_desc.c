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
        /* .tp_str  = */NULL,
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
    /* .tp_methods       = */NULL, /* TODO */
    /* .tp_getsets       = */NULL, /* TODO */
    /* .tp_members       = */NULL, /* TODO */
    /* .tp_class_methods = */NULL,
    /* .tp_class_getsets = */NULL,
    /* .tp_class_members = */NULL
};



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
