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
#ifndef GUARD_DEEMON_RUNTIME_ATTRIBUTE_C
#define GUARD_DEEMON_RUNTIME_ATTRIBUTE_C 1

#include <deemon/api.h>
#include <deemon/class.h>
#include <deemon/code.h>
#include <deemon/error.h>
#include <deemon/module.h>
#include <deemon/attribute.h>
#include <deemon/object.h>
#include <deemon/string.h>
#include <deemon/super.h>
#include <deemon/tuple.h>
#include <deemon/objmethod.h>

#include <stdarg.h>

#include "runtime_error.h"

/* Attribute access. */

DECL_BEGIN

INTDEF DREF DeeObject *DCALL type_getattr(DeeObject *__restrict self, DeeObject *__restrict name);
INTDEF DREF DeeObject *DCALL type_callattr(DeeObject *__restrict self, DeeObject *__restrict name, size_t argc, DeeObject **__restrict argv);
INTDEF DREF DeeObject *DCALL type_callattr_kw(DeeObject *__restrict self, DeeObject *__restrict name, size_t argc, DeeObject **__restrict argv, DeeObject *kw);
INTDEF int DCALL type_delattr(DeeObject *__restrict self, DeeObject *__restrict name);
INTDEF int DCALL type_setattr(DeeObject *__restrict self, DeeObject *__restrict name, DeeObject *__restrict value);

/* For type-type, these should be accessed as members, not as class-wrappers:
 * >> import type_ from deemon;
 * >> print type_.baseof(x); // Should be a bound instance-method,
 * >>                        // rather than an unbound class method!
 */
#undef CONFIG_TYPE_ATTRIBUTE_FORWARD_GENERIC
#undef CONFIG_TYPE_ATTRIBUTE_ENUM_PREVENT_DUPLICATES
#undef CONFIG_TYPE_ATTRIBUTE_SPECIALCASE_TYPETYPE
#undef CONFIG_TYPE_ATTRIBUTE_FOLLOWUP_GENERIC
#define CONFIG_TYPE_ATTRIBUTE_FORWARD_GENERIC 1
#define CONFIG_TYPE_ATTRIBUTE_SPECIALCASE_TYPETYPE 1
//#define CONFIG_TYPE_ATTRIBUTE_FOLLOWUP_GENERIC 1
//#define CONFIG_TYPE_ATTRIBUTE_ENUM_PREVENT_DUPLICATES 1

PUBLIC dssize_t DCALL
DeeObject_EnumAttr(DeeTypeObject *__restrict tp_self,
                   DeeObject *__restrict self, denum_t proc, void *arg) {
 dssize_t temp,result = 0;
 DeeTypeObject *iter = tp_self;
 do {
  if (iter->tp_attr) {
   if (!iter->tp_attr->tp_enumattr) break;
   temp = (*iter->tp_attr->tp_enumattr)(iter,self,proc,arg);
   if unlikely(temp < 0) goto err;
   result += temp;
   break;
  }
  if (DeeType_IsClass(iter)) {
#ifdef CONFIG_USE_NEW_CLASS_SYSTEM
   temp = DeeClass_EnumInstanceAttributes(iter,self,proc,arg);
#else
   struct class_desc *desc = DeeClass_DESC(iter);
   temp = membertable_enum(iter,self,desc->c_mem,proc,arg);
#endif
   if unlikely(temp < 0) goto err;
   result += temp;
  } else {
   if (iter->tp_methods) {
    /* Methods can be access from instances & classes! */
    temp = type_method_enum(iter,iter->tp_methods,
                            ATTR_IMEMBER|ATTR_CMEMBER,proc,arg);
    if unlikely(temp < 0) goto err;
    result += temp;
   }
   if (iter->tp_getsets) {
    /* Getsets can be access from instances & classes! */
    temp = type_getset_enum(iter,iter->tp_getsets,
                            ATTR_IMEMBER|ATTR_CMEMBER|ATTR_PROPERTY,
                            proc,arg);
    if unlikely(temp < 0) goto err;
    result += temp;
   }
   if (iter->tp_members) {
    /* Members can be access from instances & classes! */
    temp = type_member_enum(iter,iter->tp_members,
                            ATTR_IMEMBER|ATTR_CMEMBER,proc,arg);
    if unlikely(temp < 0) goto err;
    result += temp;
   }
  }
 } while ((iter = DeeType_Base(iter)) != NULL);
 return result;
err:
 return temp;
}


PUBLIC DREF DeeObject *DCALL
DeeObject_TGenericGetAttrString(DeeTypeObject *__restrict tp_self,
                                DeeObject *__restrict self,
                                char const *__restrict name, dhash_t hash) {
 DREF DeeObject *result;
 struct membercache *cache;
 ASSERT_OBJECT(tp_self);
 ASSERT_OBJECT(self);
 ASSERT(DeeType_Check(tp_self));
 ASSERT(DeeObject_InstanceOf(self,tp_self));
 cache = &tp_self->tp_cache;
 if ((result = membercache_getattr(cache,self,name,hash)) == ITER_DONE) {
  do {
   if (tp_self->tp_methods &&
      (result = type_method_getattr(cache,tp_self->tp_methods,self,name,hash)) != ITER_DONE)
       goto done;
   if (tp_self->tp_getsets &&
      (result = type_getset_getattr(cache,tp_self->tp_getsets,self,name,hash)) != ITER_DONE)
       goto done;
   if (tp_self->tp_members &&
      (result = type_member_getattr(cache,tp_self->tp_members,self,name,hash)) != ITER_DONE)
       goto done;
  } while ((tp_self = DeeType_Base(tp_self)) != NULL);
  return ITER_DONE;
 }
done:
 return result;
}
INTERN int DCALL
DeeObject_TGenericBoundAttrString(DeeTypeObject *__restrict tp_self,
                                  DeeObject *__restrict self,
                                  char const *__restrict name, dhash_t hash) {
 int result;
 struct membercache *cache;
 ASSERT_OBJECT(tp_self);
 ASSERT_OBJECT(self);
 ASSERT(DeeType_Check(tp_self));
 ASSERT(DeeObject_InstanceOf(self,tp_self));
 cache = &tp_self->tp_cache;
 if ((result = membercache_boundattr(cache,self,name,hash)) == -2) {
  do {
   if (tp_self->tp_methods &&
       type_method_hasattr(cache,tp_self->tp_methods,name,hash))
       goto is_bound;
   if (tp_self->tp_getsets &&
      (result = type_getset_boundattr(cache,tp_self->tp_getsets,self,name,hash)) != -2)
       goto done;
   if (tp_self->tp_members &&
       type_member_hasattr(cache,tp_self->tp_members,name,hash))
       goto is_bound;
  } while ((tp_self = DeeType_Base(tp_self)) != NULL);
  return -2;
 }
done:
 return result;
is_bound:
 return 1;
}
INTERN DREF DeeObject *DCALL
DeeObject_GenericDocAttrString(DeeTypeObject *__restrict tp_self,
                               char const *__restrict name, dhash_t hash) {
 DREF DeeObject *result;
 struct membercache *cache;
 ASSERT_OBJECT(tp_self);
 ASSERT(DeeType_Check(tp_self));
 cache = &tp_self->tp_cache;
 if ((result = membercache_docattr(cache,tp_self->tp_name,name,hash)) == ITER_DONE) {
  do {
   if (tp_self->tp_methods &&
      (result = type_method_docattr(cache,tp_self->tp_methods,tp_self->tp_name,name,hash)) != ITER_DONE)
       goto done;
   if (tp_self->tp_getsets &&
      (result = type_getset_docattr(cache,tp_self->tp_getsets,tp_self->tp_name,name,hash)) != ITER_DONE)
       goto done;
   if (tp_self->tp_members &&
      (result = type_member_docattr(cache,tp_self->tp_members,tp_self->tp_name,name,hash)) != ITER_DONE)
       goto done;
  } while ((tp_self = DeeType_Base(tp_self)) != NULL);
  return ITER_DONE;
 }
done:
 return result;
}
PUBLIC DREF DeeObject *DCALL
DeeObject_TGenericCallAttrString(DeeTypeObject *__restrict tp_self,
                                 DeeObject *__restrict self,
                                 char const *__restrict name, dhash_t hash,
                                 size_t argc, DeeObject **__restrict argv) {
 struct membercache *cache;
 DREF DeeObject *result;
 ASSERT_OBJECT(tp_self);
 ASSERT_OBJECT(self);
 ASSERT(DeeType_Check(tp_self));
 ASSERT(DeeObject_InstanceOf(self,tp_self));
 cache = &tp_self->tp_cache;
 if ((result = membercache_callattr(cache,self,name,hash,argc,argv)) == ITER_DONE) {
  do {
   if (tp_self->tp_methods &&
      (result = type_method_callattr(cache,tp_self->tp_methods,self,name,hash,argc,argv)) != ITER_DONE)
       goto done;
   if (tp_self->tp_getsets &&
      (result = type_getset_getattr(cache,tp_self->tp_getsets,self,name,hash)) != ITER_DONE)
       goto done_call;
   if (tp_self->tp_members &&
      (result = type_member_getattr(cache,tp_self->tp_members,self,name,hash)) != ITER_DONE)
       goto done_call;
  } while ((tp_self = DeeType_Base(tp_self)) != NULL);
  return ITER_DONE;
done_call:
  if likely(result) {
   DREF DeeObject *real_result;
   real_result = DeeObject_Call(result,argc,argv);
   Dee_Decref(result);
   result = real_result;
  }
 }
done:
 return result;
}
PUBLIC DREF DeeObject *DCALL
DeeObject_TGenericCallAttrStringKw(DeeTypeObject *__restrict tp_self,
                                   DeeObject *__restrict self,
                                   char const *__restrict name, dhash_t hash,
                                   size_t argc, DeeObject **__restrict argv,
                                   DeeObject *kw) {
 struct membercache *cache;
 DREF DeeObject *result;
 ASSERT_OBJECT(tp_self);
 ASSERT_OBJECT(self);
 ASSERT(DeeType_Check(tp_self));
 ASSERT(DeeObject_InstanceOf(self,tp_self));
 cache = &tp_self->tp_cache;
 if ((result = membercache_callattr_kw(cache,self,name,hash,argc,argv,kw)) == ITER_DONE) {
  do {
   if (tp_self->tp_methods &&
      (result = type_method_callattr_kw(cache,tp_self->tp_methods,self,name,hash,argc,argv,kw)) != ITER_DONE)
       goto done;
   if (tp_self->tp_getsets &&
      (result = type_getset_getattr(cache,tp_self->tp_getsets,self,name,hash)) != ITER_DONE)
       goto done_call;
   if (tp_self->tp_members &&
      (result = type_member_getattr(cache,tp_self->tp_members,self,name,hash)) != ITER_DONE)
       goto done_call;
  } while ((tp_self = DeeType_Base(tp_self)) != NULL);
  return ITER_DONE;
done_call:
  if likely(result) {
   DREF DeeObject *real_result;
   real_result = DeeObject_CallKw(result,argc,argv,kw);
   Dee_Decref(result);
   result = real_result;
  }
 }
done:
 return result;
}
INTERN bool DCALL
DeeObject_TGenericHasAttrString(DeeTypeObject *__restrict tp_self,
                                char const *__restrict name, dhash_t hash) {
 struct membercache *cache;
 ASSERT_OBJECT(tp_self);
 ASSERT(DeeType_Check(tp_self));
 cache = &tp_self->tp_cache;
 if (membercache_hasattr(cache,name,hash))
     return true;
 do {
  if (tp_self->tp_methods &&
      type_method_hasattr(cache,tp_self->tp_methods,name,hash))
      goto done;
  if (tp_self->tp_getsets &&
      type_getset_hasattr(cache,tp_self->tp_getsets,name,hash))
      goto done;
  if (tp_self->tp_members &&
      type_member_hasattr(cache,tp_self->tp_members,name,hash))
      goto done;
 } while ((tp_self = DeeType_Base(tp_self)) != NULL);
 return false;
done:
 return true;
}
PUBLIC int DCALL
DeeObject_TGenericDelAttrString(DeeTypeObject *__restrict tp_self,
                                DeeObject *__restrict self,
                                char const *__restrict name, dhash_t hash) {
 struct membercache *cache;
 int result;
 ASSERT_OBJECT(tp_self);
 ASSERT_OBJECT(self);
 ASSERT(DeeType_Check(tp_self));
 ASSERT(DeeObject_InstanceOf(self,tp_self));
 cache = &tp_self->tp_cache;
 if ((result = membercache_delattr(cache,self,name,hash)) > 0) {
  do {
   if (tp_self->tp_methods &&
       type_method_hasattr(cache,tp_self->tp_methods,name,hash)) {
    err_cant_access_attribute(tp_self,name,ATTR_ACCESS_DEL);
    return -1;
   }
   if (tp_self->tp_getsets &&
      (result = type_getset_delattr(cache,tp_self->tp_getsets,self,name,hash)) <= 0)
       goto done;
   if (tp_self->tp_members &&
      (result = type_member_delattr(cache,tp_self->tp_members,self,name,hash)) <= 0)
       goto done;
  } while ((tp_self = DeeType_Base(tp_self)) != NULL);
  return 1;
 }
done:
 return result;
}
PUBLIC int DCALL
DeeObject_TGenericSetAttrString(DeeTypeObject *__restrict tp_self,
                                DeeObject *__restrict self,
                                char const *__restrict name, dhash_t hash,
                                DeeObject *__restrict value) {
 struct membercache *cache;
 int result;
 ASSERT_OBJECT(tp_self);
 ASSERT_OBJECT(self);
 ASSERT(DeeType_Check(tp_self));
 ASSERT(DeeObject_InstanceOf(self,tp_self));
 cache = &tp_self->tp_cache;
 if ((result = membercache_setattr(cache,self,name,hash,value)) > 0) {
  do {
   if (tp_self->tp_getsets &&
       type_method_hasattr(cache,tp_self->tp_methods,name,hash)) {
    err_cant_access_attribute(tp_self,name,ATTR_ACCESS_SET);
    return -1;
   }
   if (tp_self->tp_getsets &&
      (result = type_getset_setattr(cache,tp_self->tp_getsets,self,name,hash,value)) <= 0)
       goto done;
   if (tp_self->tp_members &&
      (result = type_member_setattr(cache,tp_self->tp_members,self,name,hash,value)) <= 0)
       goto done;
  } while ((tp_self = DeeType_Base(tp_self)) != NULL);
  return 1;
 }
done:
 return result;
}
PUBLIC dssize_t DCALL
DeeObject_GenericEnumAttr(DeeTypeObject *__restrict tp_self,
                          denum_t proc, void *arg) {
 dssize_t temp,result = 0;
 ASSERT_OBJECT(tp_self);
 ASSERT(DeeType_Check(tp_self));
 do {
  if (tp_self->tp_methods) {
   temp = type_method_enum(tp_self,tp_self->tp_methods,
                           ATTR_IMEMBER|ATTR_CMEMBER,proc,arg);
   if unlikely(temp < 0) goto err;
   result += temp;
  }
  if (tp_self->tp_getsets) {
   temp = type_getset_enum(tp_self,tp_self->tp_getsets,
                           ATTR_IMEMBER|ATTR_CMEMBER|ATTR_PROPERTY,
                           proc,arg);
   if unlikely(temp < 0) goto err;
   result += temp;
  }
  if (tp_self->tp_members) {
   temp = type_member_enum(tp_self,tp_self->tp_members,
                           ATTR_IMEMBER|ATTR_CMEMBER,proc,arg);
   if unlikely(temp < 0) goto err;
   result += temp;
  }
 } while ((tp_self = DeeType_Base(tp_self)) != NULL);
 return result;
err:
 return temp;
}

INTERN int DCALL
DeeObject_GenericFindAttrString(DeeTypeObject *__restrict tp_self,
                                struct attribute_info *__restrict result,
                                struct attribute_lookup_rules const *__restrict rules) {
 int error;
 struct membercache *cache;
 ASSERT_OBJECT(tp_self);
 ASSERT(DeeType_Check(tp_self));
 cache = &tp_self->tp_cache;
 if ((error = membercache_find(cache,(DeeObject *)tp_self,
                               ATTR_IMEMBER|ATTR_CMEMBER,
                               result,rules)) > 0) {
  do {
   if (tp_self->tp_methods &&
      (error = type_method_find(cache,
                               (DeeObject *)tp_self,
                                tp_self->tp_methods,
                                ATTR_IMEMBER|ATTR_CMEMBER,
                                result,rules)) <= 0)
       goto done;
   if (tp_self->tp_getsets &&
      (error = type_getset_find(cache,
                               (DeeObject *)tp_self,
                                tp_self->tp_getsets,
                                ATTR_IMEMBER|ATTR_CMEMBER|ATTR_PROPERTY,
                                result,rules)) <= 0)
       goto done;
   if (tp_self->tp_members &&
      (error = type_member_find(cache,
                               (DeeObject *)tp_self,
                                tp_self->tp_members,
                                ATTR_IMEMBER|ATTR_CMEMBER,
                                result,rules)) <= 0)
       goto done;
  } while ((tp_self = DeeType_Base(tp_self)) != NULL);
  return 1; /* Not Found */
 }
done:
 return error;
}

INTERN int DCALL
DeeType_FindAttrString(DeeTypeObject *__restrict self,
                       struct attribute_info *__restrict result,
                       struct attribute_lookup_rules const *__restrict rules) {
 int error;
 DeeTypeObject *iter = self;
 do {
  if (rules->alr_decl && rules->alr_decl != (DeeObject *)iter)
      goto find_generic;
  if (DeeType_IsClass(iter)) {
   struct class_desc *desc = DeeClass_DESC(iter);
#ifdef CONFIG_USE_NEW_CLASS_SYSTEM
   if ((error = DeeClass_FindClassAttribute(iter,result,rules)) <= 0)
        goto done;
   if ((error = DeeClass_FindClassInstanceAttribute(iter,result,rules)) <= 0)
        goto done;
#else
   if ((error = membertable_find(iter,NULL,desc->c_cmem,result,rules)) <= 0)
        goto done;
   if ((error = membertable_find_class(iter,desc->c_mem,result,rules)) <= 0)
        goto done;
#endif
  } else {
   if ((error = membercache_find(&iter->tp_class_cache,(DeeObject *)iter,
                                  ATTR_CMEMBER,result,rules)) <= 0)
        goto done;
   if (iter->tp_class_methods &&
      (error = type_method_find(&iter->tp_class_cache,
                               (DeeObject *)iter,
                                iter->tp_class_methods,
                                ATTR_CMEMBER,
                                result,rules)) <= 0)
       goto done;
   if (iter->tp_class_getsets &&
      (error = type_getset_find(&iter->tp_class_cache,
                               (DeeObject *)iter,
                                iter->tp_class_getsets,
                                ATTR_CMEMBER|ATTR_PROPERTY,
                                result,rules)) <= 0)
       goto done;
   if (iter->tp_class_members &&
      (error = type_member_find(&iter->tp_class_cache,
                               (DeeObject *)iter,
                                iter->tp_class_members,
                                ATTR_CMEMBER,
                                result,rules)) <= 0)
       goto done;
#ifdef CONFIG_TYPE_ATTRIBUTE_SPECIALCASE_TYPETYPE
   if (iter != &DeeType_Type)
#endif
   {
    if ((error = membercache_findinstanceattr(iter,result,rules)) <= 0)
         goto done;
    if (iter->tp_methods && /* Access instance methods using `DeeClsMethodObject' */
       (error = type_obmeth_find(iter,result,rules)) <= 0)
        goto done;
    if (iter->tp_getsets && /* Access instance getsets using `DeeClsPropertyObject' */
       (error = type_obprop_find(iter,result,rules)) <= 0)
        goto done;
    if (iter->tp_members && /* Access instance members using `DeeClsMemberObject' */
       (error = type_obmemb_find(iter,result,rules)) <= 0)
        goto done;
   }
  }
find_generic:
#ifdef CONFIG_TYPE_ATTRIBUTE_FORWARD_GENERIC
  if ((error = DeeObject_GenericFindAttrString(iter,result,rules)) <= 0)
       goto done;
#endif
  ;
 } while ((iter = DeeType_Base(iter)) != NULL);
#ifdef CONFIG_TYPE_ATTRIBUTE_FOLLOWUP_GENERIC
 return DeeObject_GenericFindAttrString(self,result,rules);
#else
 return 1; /* Not found */
#endif
done:
 return error;
}

INTERN DREF DeeObject *DCALL
DeeType_GetAttrString(DeeTypeObject *__restrict self,
                      char const *__restrict name, dhash_t hash) {
 DeeTypeObject *iter = self;
 DREF DeeObject *result;
 do {
  if (DeeType_IsClass(iter)) {
   struct member_entry *attr;
   struct class_desc *desc = DeeClass_DESC(iter);
   if ((attr = DeeClassDesc_QueryClassAttributeStringWithHash(desc,name,hash)) != NULL) {
    if (!member_mayaccess(iter,attr)) {
     err_protected_member(iter,attr);
     return NULL;
    }
#ifdef CONFIG_USE_NEW_CLASS_SYSTEM
    return DeeClass_GetClassAttribute(iter,attr);
#else
    return member_get((DeeTypeObject *)iter,&desc->c_class,
                      (DeeObject *)iter,attr);
#endif
   }
   if ((attr = DeeClassDesc_QueryInstanceAttributeStringWithHash(desc,name,hash)) != NULL) {
    if (!member_mayaccess(iter,attr)) {
     err_protected_member(iter,attr);
     return NULL;
    }
#ifdef CONFIG_USE_NEW_CLASS_SYSTEM
    return DeeClass_GetInstanceAttribute(iter,attr);
#else
    return class_member_get((DeeTypeObject *)iter,&desc->c_class,attr);
#endif
   }
  } else {
   if ((result = membercache_getattr(&iter->tp_class_cache,(DeeObject *)iter,name,hash)) != ITER_DONE)
       goto done;
   if (iter->tp_class_methods &&
      (result = type_method_getattr(&iter->tp_class_cache,iter->tp_class_methods,(DeeObject *)iter,name,hash)) != ITER_DONE)
       goto done;
   if (iter->tp_class_getsets &&
      (result = type_getset_getattr(&iter->tp_class_cache,iter->tp_class_getsets,(DeeObject *)iter,name,hash)) != ITER_DONE)
       goto done;
   if (iter->tp_class_members &&
      (result = type_member_getattr(&iter->tp_class_cache,iter->tp_class_members,(DeeObject *)iter,name,hash)) != ITER_DONE)
       goto done;
#ifdef CONFIG_TYPE_ATTRIBUTE_SPECIALCASE_TYPETYPE
   if (iter != &DeeType_Type)
#endif
   {
    if ((result = membercache_getinstanceattr(iter,name,hash)) != ITER_DONE)
        goto done;
    if (iter->tp_methods && /* Access instance methods using `DeeClsMethodObject' */
       (result = type_obmeth_getattr(iter,name,hash)) != ITER_DONE)
        goto done;
    if (iter->tp_getsets && /* Access instance getsets using `DeeClsPropertyObject' */
       (result = type_obprop_getattr(iter,name,hash)) != ITER_DONE)
        goto done;
    if (iter->tp_members && /* Access instance members using `DeeClsMemberObject' */
       (result = type_obmemb_getattr(iter,name,hash)) != ITER_DONE)
        goto done;
   }
  }
#ifdef CONFIG_TYPE_ATTRIBUTE_FORWARD_GENERIC
  if ((result = DeeObject_GenericGetAttrString((DeeObject *)iter,name,hash)) != ITER_DONE)
      goto done;
#endif
 } while ((iter = DeeType_Base(iter)) != NULL);
#ifdef CONFIG_TYPE_ATTRIBUTE_FOLLOWUP_GENERIC
 if ((result = DeeObject_GenericGetAttrString((DeeObject *)self,name,hash)) != ITER_DONE)
     goto done;
#endif
 err_unknown_attribute(self,name,ATTR_ACCESS_GET);
 return NULL;
done:
 return result;
}
INTERN int DCALL
DeeType_BoundAttrString(DeeTypeObject *__restrict self,
                        char const *__restrict name, dhash_t hash) {
 DeeTypeObject *iter = self; int result;
 do {
  if (DeeType_IsClass(iter)) {
   struct member_entry *attr;
   struct class_desc *desc = DeeClass_DESC(iter);
   if ((attr = DeeClassDesc_QueryClassAttributeStringWithHash(desc,name,hash)) != NULL) {
    if (!member_mayaccess(iter,attr)) {
     err_protected_member(iter,attr);
     return -1;
    }
#ifdef CONFIG_USE_NEW_CLASS_SYSTEM
    return DeeClass_BoundClassAttribute(iter,attr);
#else
    return member_bound((DeeTypeObject *)iter,&desc->c_class,
                        (DeeObject *)iter,attr);
#endif
   }
   if ((attr = DeeClassDesc_QueryInstanceAttributeStringWithHash(desc,name,hash)) != NULL) {
    if (!member_mayaccess(iter,attr)) {
     err_protected_member(iter,attr);
     return -1;
    }
    goto is_bound; /* Wrapper objects are always bound. */
   }
  } else {
   if ((result = membercache_boundattr(&iter->tp_class_cache,(DeeObject *)iter,name,hash)) != -2)
       goto done;
   if (iter->tp_class_methods &&
       type_method_hasattr(&iter->tp_class_cache,iter->tp_class_methods,name,hash))
       goto is_bound;
   if (iter->tp_class_getsets &&
      (result = type_getset_boundattr(&iter->tp_class_cache,iter->tp_class_getsets,(DeeObject *)iter,name,hash)) != -2)
       goto done;
   if (iter->tp_class_members &&
       type_member_hasattr(&iter->tp_class_cache,iter->tp_class_members,name,hash))
       goto is_bound;
#ifdef CONFIG_TYPE_ATTRIBUTE_SPECIALCASE_TYPETYPE
   if (iter != &DeeType_Type)
#endif
   {
    if (membercache_hasinstanceattr(iter,name,hash))
        goto is_bound;
    if (iter->tp_methods && /* Access instance methods using `DeeClsMethodObject' */
        type_obmeth_hasattr(iter,name,hash))
        goto is_bound;
    if (iter->tp_getsets && /* Access instance getsets using `DeeClsPropertyObject' */
        type_obprop_hasattr(iter,name,hash))
        goto is_bound;
    if (iter->tp_members && /* Access instance members using `DeeClsMemberObject' */
        type_obmemb_hasattr(iter,name,hash))
        goto is_bound;
   }
  }
#ifdef CONFIG_TYPE_ATTRIBUTE_FORWARD_GENERIC
  if ((result = DeeObject_GenericBoundAttrString((DeeObject *)iter,name,hash)) != -2)
      goto done;
#endif
 } while ((iter = DeeType_Base(iter)) != NULL);
#ifdef CONFIG_TYPE_ATTRIBUTE_FOLLOWUP_GENERIC
 if ((result = DeeObject_GenericBoundAttrString((DeeObject *)self,name,hash)) != -2)
     goto done;
#endif
 return -2;
is_bound:
 return 1;
done:
 return result;
}
INTERN DREF DeeObject *DCALL
DeeType_DocAttrString(DeeTypeObject *__restrict self,
                      char const *__restrict name, dhash_t hash) {
 DeeTypeObject *iter = self;
 DREF DeeObject *result;
 do {
  if (DeeType_IsClass(iter)) {
   struct member_entry *attr;
   struct class_desc *desc = DeeClass_DESC(iter);
   if ((attr = DeeClassDesc_QueryClassAttributeStringWithHash(desc,name,hash)) != NULL ||
       (attr = DeeClassDesc_QueryInstanceAttributeStringWithHash(desc,name,hash)) != NULL) {
#ifdef CONFIG_USE_NEW_CLASS_SYSTEM
    if (attr->ca_doc)
        return_reference_((DeeObject *)attr->ca_doc);
#else
    if (attr->cme_doc)
        return_reference_((DeeObject *)attr->cme_doc);
#endif
    err_nodoc_attribute(iter->tp_name,name);
    goto err;
   }
  } else {
   /* NOTE: Also look into the instance-attribute cache
    *       for documentation information on this attribute. */
   if ((result = membercache_docattr(&iter->tp_class_cache,iter->tp_name,name,hash)) != ITER_DONE)
        goto done;
   if (iter->tp_class_methods &&
      (result = type_method_docattr(&iter->tp_class_cache,iter->tp_class_methods,iter->tp_name,name,hash)) != ITER_DONE)
       goto done;
   if (iter->tp_class_getsets &&
      (result = type_getset_docattr(&iter->tp_class_cache,iter->tp_class_getsets,iter->tp_name,name,hash)) != ITER_DONE)
       goto done;
   if (iter->tp_class_members &&
      (result = type_member_docattr(&iter->tp_class_cache,iter->tp_class_members,iter->tp_name,name,hash)) != ITER_DONE)
       goto done;
#ifdef CONFIG_TYPE_ATTRIBUTE_SPECIALCASE_TYPETYPE
   if (iter != &DeeType_Type)
#endif
   {
    if ((result = membercache_docattr(&iter->tp_cache,iter->tp_name,name,hash)) != ITER_DONE)
         goto done;
    if (iter->tp_methods && /* Access instance methods using `DeeClsMethodObject' */
       (result = type_obmeth_docattr(iter,name,hash)) != ITER_DONE)
        goto done;
    if (iter->tp_getsets && /* Access instance getsets using `DeeClsPropertyObject' */
       (result = type_obprop_docattr(iter,name,hash)) != ITER_DONE)
        goto done;
    if (iter->tp_members && /* Access instance members using `DeeClsMemberObject' */
       (result = type_obmemb_docattr(iter,name,hash)) != ITER_DONE)
        goto done;
   }
  }
  /* Query generic documentation information on this type as a generic object. */
#ifdef CONFIG_TYPE_ATTRIBUTE_FORWARD_GENERIC
  if ((result = DeeObject_GenericDocAttrString(Dee_TYPE(iter),name,hash)) != ITER_DONE)
      goto done;
#endif
 } while ((iter = DeeType_Base(iter)) != NULL);
#ifdef CONFIG_TYPE_ATTRIBUTE_FOLLOWUP_GENERIC
 if ((result = DeeObject_GenericDocAttrString(Dee_TYPE(self),name,hash)) != ITER_DONE)
     goto done;
#endif
 err_unknown_attribute(self,name,ATTR_ACCESS_GET);
err:
 return NULL;
done:
 return result;
}

INTERN DREF DeeObject *DCALL
DeeType_CallAttrString(DeeTypeObject *__restrict self,
                       char const *__restrict name, dhash_t hash,
                       size_t argc, DeeObject **__restrict argv) {
 DeeTypeObject *iter = self;
 DREF DeeObject *result;
 do {
  if (DeeType_IsClass(iter)) {
   struct member_entry *attr;
   struct class_desc *desc = DeeClass_DESC(iter);
   if ((attr = DeeClassDesc_QueryClassAttributeStringWithHash(desc,name,hash)) != NULL) {
    if (!member_mayaccess(iter,attr)) {
     err_protected_member(iter,attr);
     return NULL;
    }
#ifdef CONFIG_USE_NEW_CLASS_SYSTEM
    return DeeClass_CallClassAttribute(iter,attr,argc,argv);
#else
    return member_call(iter,&desc->c_class,
                      (DeeObject *)iter,attr,argc,argv);
#endif
   }
   if ((attr = DeeClassDesc_QueryInstanceAttributeStringWithHash(desc,name,hash)) != NULL) {
    if (!member_mayaccess(iter,attr)) {
     err_protected_member(iter,attr);
     goto err;
    }
#ifdef CONFIG_USE_NEW_CLASS_SYSTEM
    return DeeClass_CallInstanceAttribute(iter,attr,argc,argv);
#else
    return class_member_call(iter,&desc->c_class,attr,argc,argv);
#endif
   }
  } else {
   if ((result = membercache_callattr(&self->tp_class_cache,(DeeObject *)self,name,hash,argc,argv)) != ITER_DONE)
       goto done;
   if (iter->tp_class_methods &&
      (result = type_method_callattr(&iter->tp_class_cache,iter->tp_class_methods,(DeeObject *)iter,name,hash,argc,argv)) != ITER_DONE)
       goto done;
   if (iter->tp_class_getsets &&
      (result = type_getset_getattr(&iter->tp_class_cache,iter->tp_class_getsets,(DeeObject *)iter,name,hash)) != ITER_DONE)
       goto done_invoke;
   if (iter->tp_class_members &&
      (result = type_member_getattr(&iter->tp_class_cache,iter->tp_class_members,(DeeObject *)iter,name,hash)) != ITER_DONE)
       goto done_invoke;
#ifdef CONFIG_TYPE_ATTRIBUTE_SPECIALCASE_TYPETYPE
   if (iter != &DeeType_Type)
#endif
   {
    if ((result = membercache_callinstanceattr(self,name,hash,argc,argv)) != ITER_DONE)
        goto done;
    if (iter->tp_methods && /* Access instance methods using `DeeClsMethodObject' */
       (result = type_obmeth_callattr(iter,name,hash,argc,argv)) != ITER_DONE)
        goto done;
    if (iter->tp_getsets && /* Access instance getsets using `DeeClsPropertyObject' */
       (result = type_obprop_callattr(iter,name,hash,argc,argv)) != ITER_DONE)
        goto done;
    if (iter->tp_members && /* Access instance members using `DeeClsMemberObject' */
       (result = type_obmemb_callattr(iter,name,hash,argc,argv)) != ITER_DONE)
        goto done;
   }
  }
#ifdef CONFIG_TYPE_ATTRIBUTE_FORWARD_GENERIC
  if ((result = DeeObject_GenericCallAttrString((DeeObject *)iter,name,hash,argc,argv)) != ITER_DONE)
      goto done;
#endif
 } while ((iter = DeeType_Base(iter)) != NULL);
#ifdef CONFIG_TYPE_ATTRIBUTE_FOLLOWUP_GENERIC
 if ((result = DeeObject_GenericCallAttrString((DeeObject *)self,name,hash,argc,argv)) != ITER_DONE)
     goto done;
#endif
 err_unknown_attribute(self,name,ATTR_ACCESS_GET);
err:
 return NULL;
done_invoke:
 if (result) {
  DREF DeeObject *real_result;
  real_result = DeeObject_Call(result,argc,argv);
  Dee_Decref(result);
  return real_result;
 }
done:
 return result;
}

INTERN DREF DeeObject *DCALL
DeeType_CallAttrStringKw(DeeTypeObject *__restrict self,
                         char const *__restrict name, dhash_t hash,
                         size_t argc, DeeObject **__restrict argv,
                         DeeObject *kw) {
 DeeTypeObject *iter = self;
 DREF DeeObject *result;
 do {
  if (DeeType_IsClass(iter)) {
   struct member_entry *attr;
   struct class_desc *desc = DeeClass_DESC(iter);
   if ((attr = DeeClassDesc_QueryClassAttributeStringWithHash(desc,name,hash)) != NULL) {
    if (!member_mayaccess(iter,attr)) {
     err_protected_member(iter,attr);
     return NULL;
    }
#ifdef CONFIG_USE_NEW_CLASS_SYSTEM
    return DeeClass_CallClassAttributeKw(iter,attr,argc,argv,kw);
#else
    return member_call_kw(iter,&desc->c_class,
                         (DeeObject *)iter,attr,
                          argc,argv,kw);
#endif
   }
   if ((attr = DeeClassDesc_QueryInstanceAttributeStringWithHash(desc,name,hash)) != NULL) {
    if (!member_mayaccess(iter,attr)) {
     err_protected_member(iter,attr);
     goto err;
    }
#ifdef CONFIG_USE_NEW_CLASS_SYSTEM
    return DeeClass_CallInstanceAttributeKw(iter,attr,argc,argv,kw);
#else
    return class_member_call_kw(iter,&desc->c_class,
                                attr,argc,argv,kw);
#endif
   }
  } else {
   if ((result = membercache_callattr_kw(&self->tp_class_cache,(DeeObject *)self,name,hash,argc,argv,kw)) != ITER_DONE)
       goto done;
   if (iter->tp_class_methods &&
      (result = type_method_callattr_kw(&iter->tp_class_cache,iter->tp_class_methods,(DeeObject *)iter,name,hash,argc,argv,kw)) != ITER_DONE)
       goto done;
   if (iter->tp_class_getsets &&
      (result = type_getset_getattr(&iter->tp_class_cache,iter->tp_class_getsets,(DeeObject *)iter,name,hash)) != ITER_DONE)
       goto done_invoke;
   if (iter->tp_class_members &&
      (result = type_member_getattr(&iter->tp_class_cache,iter->tp_class_members,(DeeObject *)iter,name,hash)) != ITER_DONE)
       goto done_invoke;
#ifdef CONFIG_TYPE_ATTRIBUTE_SPECIALCASE_TYPETYPE
   if (iter != &DeeType_Type)
#endif
   {
    if ((result = membercache_callinstanceattr_kw(self,name,hash,argc,argv,kw)) != ITER_DONE)
        goto done;
    if (iter->tp_methods && /* Access instance methods using `DeeClsMethodObject' */
       (result = type_obmeth_callattr_kw(iter,name,hash,argc,argv,kw)) != ITER_DONE)
        goto done;
    if (iter->tp_getsets && /* Access instance getsets using `DeeClsPropertyObject' */
       (result = type_obprop_callattr_kw(iter,name,hash,argc,argv,kw)) != ITER_DONE)
        goto done;
    if (iter->tp_members && /* Access instance members using `DeeClsMemberObject' */
       (result = type_obmemb_callattr_kw(iter,name,hash,argc,argv,kw)) != ITER_DONE)
        goto done;
   }
  }
#ifdef CONFIG_TYPE_ATTRIBUTE_FORWARD_GENERIC
  if ((result = DeeObject_GenericCallAttrStringKw((DeeObject *)iter,name,hash,argc,argv,kw)) != ITER_DONE)
      goto done;
#endif
 } while ((iter = DeeType_Base(iter)) != NULL);
#ifdef CONFIG_TYPE_ATTRIBUTE_FOLLOWUP_GENERIC
 if ((result = DeeObject_GenericCallAttrStringKw((DeeObject *)self,name,hash,argc,argv,kw)) != ITER_DONE)
     goto done;
#endif
 err_unknown_attribute(self,name,ATTR_ACCESS_GET);
err:
 return NULL;
done_invoke:
 if (result) {
  DREF DeeObject *real_result;
  real_result = DeeObject_CallKw(result,argc,argv,kw);
  Dee_Decref(result);
  return real_result;
 }
done:
 return result;
}

INTERN bool DCALL
DeeType_HasAttrString(DeeTypeObject *__restrict self,
                      char const *__restrict name, dhash_t hash) {
 DeeTypeObject *iter = self;
 do {
  if (DeeType_IsClass(iter)) {
   struct class_desc *desc = DeeClass_DESC(iter);
   if (DeeClassDesc_QueryClassAttributeStringWithHash(desc,name,hash) ||
       DeeClassDesc_QueryInstanceAttributeStringWithHash(desc,name,hash))
       return true;
  } else {
   if (membercache_hasattr(&iter->tp_class_cache,name,hash) ||
       membercache_hasinstanceattr(iter,name,hash))
       goto done;
   if (iter->tp_class_methods &&
       type_method_hasattr(&iter->tp_class_cache,iter->tp_class_methods,name,hash))
       goto done;
   if (iter->tp_class_getsets &&
       type_getset_hasattr(&iter->tp_class_cache,iter->tp_class_getsets,name,hash))
       goto done;
   if (iter->tp_class_members &&
       type_member_hasattr(&iter->tp_class_cache,iter->tp_class_members,name,hash))
       goto done;
#ifdef CONFIG_TYPE_ATTRIBUTE_SPECIALCASE_TYPETYPE
   if (iter != &DeeType_Type)
#endif
   {
    if (iter->tp_methods && /* Access instance methods using `DeeClsMethodObject' */
        type_obmeth_hasattr(iter,name,hash))
        goto done;
    if (iter->tp_getsets && /* Access instance getsets using `DeeClsPropertyObject' */
        type_obprop_hasattr(iter,name,hash))
        goto done;
    if (iter->tp_members && /* Access instance members using `DeeClsMemberObject' */
        type_obmemb_hasattr(iter,name,hash))
        goto done;
   }
  }
#ifdef CONFIG_TYPE_ATTRIBUTE_FORWARD_GENERIC
  if (DeeObject_GenericHasAttrString((DeeObject *)iter,name,hash)) goto done;
#endif
 } while ((iter = DeeType_Base(iter)) != NULL);
#ifdef CONFIG_TYPE_ATTRIBUTE_FOLLOWUP_GENERIC
 if (DeeObject_GenericHasAttrString((DeeObject *)self,name,hash)) goto done;
#endif
 return false;
done:
 return true;
}
INTERN int (DCALL DeeType_DelAttrString)(DeeTypeObject *__restrict self,
                                         char const *__restrict name, dhash_t hash) {
 DeeTypeObject *iter = self; int error;
 do {
  if (DeeType_IsClass(iter)) {
   struct member_entry *attr;
   struct class_desc *desc = DeeClass_DESC(iter);
   if ((attr = DeeClassDesc_QueryClassAttributeStringWithHash(desc,name,hash)) != NULL) {
    if (!member_mayaccess(iter,attr)) {
     err_protected_member(iter,attr);
     goto err;
    }
#ifdef CONFIG_USE_NEW_CLASS_SYSTEM
    return DeeClass_DelClassAttribute(iter,attr);
#else
    return member_del(iter,&desc->c_class,(DeeObject *)iter,attr);
#endif
   }
   if ((attr = DeeClassDesc_QueryInstanceAttributeStringWithHash(desc,name,hash)) != NULL) {
    if (!member_mayaccess(iter,attr)) {
     err_protected_member(iter,attr);
     goto err;
    }
#ifdef CONFIG_USE_NEW_CLASS_SYSTEM
    return DeeClass_DelInstanceAttribute(iter,attr);
#else
    if (attr->ca_flag & CLASS_MEMBER_FCLASSMEM)
        return class_member_del(iter,&desc->c_class,attr);
    /* Instance attr attributes cannot be deleted without an instance operand. */
    goto noaccess;
#endif
   }
  } else {
   if ((error = membercache_delattr(&self->tp_class_cache,(DeeObject *)self,name,hash)) <= 0)
        goto done;
   if (iter->tp_class_methods &&
       type_method_hasattr(&iter->tp_class_cache,iter->tp_class_methods,name,hash))
       goto noaccess;
   if (iter->tp_class_getsets &&
      (error = type_getset_delattr(&iter->tp_class_cache,iter->tp_class_getsets,(DeeObject *)iter,name,hash)) <= 0)
       goto done;
   if (iter->tp_class_members &&
      (error = type_member_delattr(&iter->tp_class_cache,iter->tp_class_members,(DeeObject *)iter,name,hash)) <= 0)
       goto done;
#ifdef CONFIG_TYPE_ATTRIBUTE_SPECIALCASE_TYPETYPE
   if (iter != &DeeType_Type)
#endif
   {
    if ((error = membercache_delinstanceattr(self,name,hash)) <= 0)
         goto done;
    if (iter->tp_methods && /* Access instance methods using `DeeClsMethodObject' */
        type_obmeth_hasattr(iter,name,hash))
        goto noaccess;
    if (iter->tp_getsets && /* Access instance getsets using `DeeClsPropertyObject' */
        type_obprop_hasattr(iter,name,hash))
        goto noaccess;
    if (iter->tp_members && /* Access instance members using `DeeClsMemberObject' */
        type_obmemb_hasattr(iter,name,hash))
        goto noaccess;
   }
  }
#ifdef CONFIG_TYPE_ATTRIBUTE_FORWARD_GENERIC
  if ((error = DeeObject_GenericDelAttrString((DeeObject *)iter,name,hash)) <= 0)
       goto done;
#endif
 } while ((iter = DeeType_Base(iter)) != NULL);
#ifdef CONFIG_TYPE_ATTRIBUTE_FOLLOWUP_GENERIC
 if ((error = DeeObject_GenericDelAttrString((DeeObject *)self,name,hash)) <= 0)
      goto done;
#endif
 err_unknown_attribute(self,name,ATTR_ACCESS_DEL);
err:      return -1;
noaccess: err_cant_access_attribute(iter,name,ATTR_ACCESS_DEL); goto err;
done:     return error;
}
INTERN int (DCALL DeeType_SetAttrString)(DeeTypeObject *__restrict self,
                                         char const *__restrict name, dhash_t hash,
                                         DeeObject *__restrict value) {
 DeeTypeObject *iter = self; int error;
 do {
  if (DeeType_IsClass(iter)) {
   struct member_entry *attr;
   struct class_desc *desc = DeeClass_DESC(iter);
   if ((attr = DeeClassDesc_QueryClassAttributeStringWithHash(desc,name,hash)) != NULL) {
    if (!member_mayaccess(iter,attr)) {
     err_protected_member(iter,attr);
     goto err;
    }
#ifdef CONFIG_USE_NEW_CLASS_SYSTEM
    return DeeClass_SetClassAttribute(iter,attr,value);
#else
    return member_set(iter,&desc->c_class,(DeeObject *)iter,attr,value);
#endif
   }
   if ((attr = DeeClassDesc_QueryInstanceAttributeStringWithHash(desc,name,hash)) != NULL) {
    if (!member_mayaccess(iter,attr)) {
     err_protected_member(iter,attr);
     goto err;
    }
#ifdef CONFIG_USE_NEW_CLASS_SYSTEM
    return DeeClass_SetInstanceAttribute(iter,attr,value);
#else
    if (attr->ca_flag & CLASS_MEMBER_FCLASSMEM)
        return class_member_set(iter,&desc->c_class,attr,value);
    /* Instance attr attributes cannot be set without an instance operand. */
    goto noaccess;
#endif
   }
  } else {
   if ((error = membercache_setattr(&self->tp_class_cache,(DeeObject *)self,name,hash,value)) <= 0)
        goto done;
   if (iter->tp_class_methods &&
       type_method_hasattr(&iter->tp_class_cache,iter->tp_class_methods,name,hash))
       goto noaccess;
   if (iter->tp_class_getsets &&
      (error = type_getset_setattr(&iter->tp_class_cache,iter->tp_class_getsets,(DeeObject *)iter,name,hash,value)) <= 0)
       goto done;
   if (iter->tp_class_members &&
      (error = type_member_setattr(&iter->tp_class_cache,iter->tp_class_members,(DeeObject *)iter,name,hash,value)) <= 0)
       goto done;
#ifdef CONFIG_TYPE_ATTRIBUTE_SPECIALCASE_TYPETYPE
   if (iter != &DeeType_Type)
#endif
   {
    if ((error = membercache_setinstanceattr(self,name,hash,value)) <= 0)
         goto done;
    if (iter->tp_methods && /* Access instance methods using `DeeClsMethodObject' */
        type_obmeth_hasattr(iter,name,hash))
        goto noaccess;
    if (iter->tp_getsets && /* Access instance getsets using `DeeClsPropertyObject' */
        type_obprop_hasattr(iter,name,hash))
        goto noaccess;
    if (iter->tp_members && /* Access instance members using `DeeClsMemberObject' */
        type_obmemb_hasattr(iter,name,hash))
        goto noaccess;
   }
  }
#ifdef CONFIG_TYPE_ATTRIBUTE_FORWARD_GENERIC
  if ((error = DeeObject_GenericSetAttrString((DeeObject *)iter,name,hash,value)) <= 0)
       goto done;
#endif
 } while ((iter = DeeType_Base(iter)) != NULL);
#ifdef CONFIG_TYPE_ATTRIBUTE_FOLLOWUP_GENERIC
 if ((error = DeeObject_GenericSetAttrString((DeeObject *)self,name,hash,value)) <= 0)
      goto done;
#endif
 err_unknown_attribute(self,name,ATTR_ACCESS_GET);
err:      return -1;
noaccess: err_cant_access_attribute(iter,name,ATTR_ACCESS_SET); goto err;
done:     return error;
}


#ifdef CONFIG_TYPE_ATTRIBUTE_ENUM_PREVENT_DUPLICATES
struct tiny_set {
    DeeObject      **ts_map;      /* [owned_if(!= ts_smap)] Map of all objects. */
    size_t           ts_mask;     /* Current hash-mask */
    size_t           ts_size;     /* Number of contained objects. */
    DREF DeeObject  *ts_smap[16]; /* Statically allocated map. */
};

#define tiny_set_init(x) \
  ((x)->ts_map = (x)->ts_smap,(x)->ts_size = 0, \
    memset((x)->ts_smap,0,sizeof((x)->ts_smap)), \
   (x)->ts_mask = COMPILER_LENOF((x)->ts_smap) - 1)
#define tiny_set_fini(x) \
  ((x)->ts_map == (x)->ts_smap ? (void)0 : (void)Dee_Free((x)->ts_map))
LOCAL bool DCALL
tiny_set_contains(struct tiny_set *__restrict self,
                  DeeObject *__restrict obj) {
 dhash_t i,perturb;
 perturb = i = Dee_HashPointer(obj) & self->ts_mask;
 for (;; i = ((i << 2) + i + perturb + 1),perturb >>= 5) {
  DeeObject *item = self->ts_map[i & self->ts_mask];
  if (!item) break; /* Not found */
  if (item == obj) return true;
 }
 return false;
}
LOCAL bool DCALL
tiny_set_rehash(struct tiny_set *__restrict self) {
 dhash_t i,j,perturb; DeeObject **new_map;
 size_t new_mask = (self->ts_mask << 1)|1;
 /* Allocate the new map. */
 new_map = (DeeObject **)Dee_Calloc((new_mask+1)*sizeof(DeeObject *));
 if unlikely(!new_map) return false;
 /* Rehash the old map. */
 for (i = 0; i <= self->ts_mask; ++i) {
  DeeObject *obj = self->ts_map[i];
  if (!obj) continue;
  perturb = j = Dee_HashPointer(obj) & new_mask;
  for (;; j = ((j << 2) + j + perturb + 1),perturb >>= 5) {
   if (new_map[j]) continue;
   new_map[j] = obj;
   break;
  }
 }
 /* Install the new map. */
 if (self->ts_map != self->ts_smap)
     Dee_Free(self->ts_map);
 self->ts_map  = new_map;
 self->ts_mask = new_mask;
 return true;
}

LOCAL bool DCALL
tiny_set_insert(struct tiny_set *__restrict self,
                DeeObject *__restrict obj) {
 dhash_t i,perturb;
again:
 perturb = i = Dee_HashPointer(obj) & self->ts_mask;
 for (;; i = ((i << 2) + i + perturb + 1),perturb >>= 5) {
  DeeObject *item = self->ts_map[i & self->ts_mask];
  if (item == obj) break; /* Already inserted */
  if (item != NULL) continue; /* Used slot */
  /* Check if the set must be rehashed. */
  if (self->ts_size >= self->ts_mask-1) {
   if (!tiny_set_rehash(self))
        return false;
   goto again;
  }
  self->ts_map[i & self->ts_mask] = obj;
  ++self->ts_size;
  break;
 }
 return true;
}
#endif




INTERN dssize_t DCALL
DeeType_EnumAttr(DeeTypeObject *__restrict self,
                 denum_t proc, void *arg) {
 dssize_t temp,result = 0;
 DeeTypeObject *iter = self;
#ifdef CONFIG_TYPE_ATTRIBUTE_ENUM_PREVENT_DUPLICATES
 struct tiny_set finished_set;
 tiny_set_init(&finished_set);
#endif
 do {
#ifdef CONFIG_TYPE_ATTRIBUTE_ENUM_PREVENT_DUPLICATES
  if unlikely(!tiny_set_insert(&finished_set,(DeeObject *)self))
     goto err_m1;
#endif
  if (DeeType_IsClass(iter)) {
   struct class_desc *desc = DeeClass_DESC(iter);
#ifdef CONFIG_USE_NEW_CLASS_SYSTEM
   temp = DeeClass_EnumClassAttributes(iter,proc,arg);
#else
   temp = membertable_enum(iter,NULL,desc->c_cmem,proc,arg);
#endif
   if unlikely(temp < 0) goto err;
   result += temp;
#ifdef CONFIG_USE_NEW_CLASS_SYSTEM
   temp = DeeClass_EnumClassInstanceAttributes(iter,proc,arg);
#else
   temp = membertable_enum_class(iter,desc->c_mem,proc,arg);
#endif
   if unlikely(temp < 0) goto err;
   result += temp;
  } else {
   if (iter->tp_class_methods) {
    temp = type_method_enum(iter,iter->tp_class_methods,
                            ATTR_CMEMBER,proc,arg);
    if unlikely(temp < 0) goto err;
    result += temp;
   }
   if (iter->tp_class_getsets) {
    temp = type_getset_enum(iter,iter->tp_class_getsets,
                            ATTR_CMEMBER|ATTR_PROPERTY,
                            proc,arg);
    if unlikely(temp < 0) goto err;
    result += temp;
   }
   if (iter->tp_class_members) {
    temp = type_member_enum(iter,iter->tp_class_members,
                            ATTR_CMEMBER,proc,arg);
    if unlikely(temp < 0) goto err;
    result += temp;
   }
#ifdef CONFIG_TYPE_ATTRIBUTE_SPECIALCASE_TYPETYPE
   if (iter != &DeeType_Type)
#endif
   {
    if (iter->tp_methods) {
     /* Access instance methods using `DeeClsMethodObject' */
     temp = type_obmeth_enum(iter,proc,arg);
     if unlikely(temp < 0) goto err;
     result += temp;
    }
    if (iter->tp_getsets) {
     /* Access instance getsets using `DeeClsPropertyObject' */
     temp = type_obprop_enum(iter,proc,arg);
     if unlikely(temp < 0) goto err;
     result += temp;
    }
    if (iter->tp_members) {
     /* Access instance members using `DeeClsMemberObject' */
     temp = type_obmemb_enum(iter,proc,arg);
     if unlikely(temp < 0) goto err;
     result += temp;
    }
   }
  }
#ifdef CONFIG_TYPE_ATTRIBUTE_FORWARD_GENERIC
#ifdef CONFIG_TYPE_ATTRIBUTE_ENUM_PREVENT_DUPLICATES
  if (!tiny_set_contains(&finished_set,(DeeObject *)Dee_TYPE(iter))) {
   temp = DeeObject_GenericEnumAttr(Dee_TYPE(iter),proc,arg);
   if unlikely(temp < 0) goto err;
   result += temp;
  }
#else
  temp = DeeObject_GenericEnumAttr(Dee_TYPE(iter),proc,arg);
  if unlikely(temp < 0) goto err;
  result += temp;
#endif
#endif
 } while ((iter = DeeType_Base(iter)) != NULL);
#ifdef CONFIG_TYPE_ATTRIBUTE_FOLLOWUP_GENERIC
 temp = DeeObject_GenericEnumAttr(Dee_TYPE(self),proc,arg);
 if unlikely(temp < 0) goto err;
 result += temp;
#endif
done:
#ifdef CONFIG_TYPE_ATTRIBUTE_ENUM_PREVENT_DUPLICATES
 tiny_set_fini(&finished_set);
#endif
 return result;
#ifdef CONFIG_TYPE_ATTRIBUTE_ENUM_PREVENT_DUPLICATES
err_m1:
 temp = -1;
#endif
err:
 result = temp;
 goto done;
}

INTERN DREF DeeObject *DCALL
DeeType_GetInstanceAttrString(DeeTypeObject *__restrict self,
                              char const *__restrict name, dhash_t hash) {
 DeeTypeObject *iter = self;
 DREF DeeObject *result;
 do {
  if (DeeType_IsClass(iter)) {
   struct member_entry *attr;
   struct class_desc *desc = DeeClass_DESC(iter);
   if ((attr = DeeClassDesc_QueryInstanceAttributeStringWithHash(desc,name,hash)) != NULL) {
    if (!member_mayaccess(iter,attr)) {
     err_protected_member(iter,attr);
     return NULL;
    }
#ifdef CONFIG_USE_NEW_CLASS_SYSTEM
    return DeeClass_GetInstanceAttribute(iter,attr);
#else
    return class_member_get((DeeTypeObject *)iter,&desc->c_class,attr);
#endif
   }
  } else {
   if ((result = membercache_getinstanceattr(iter,name,hash)) != ITER_DONE)
        goto done;
   if (iter->tp_methods && /* Access instance methods using `DeeClsMethodObject' */
      (result = type_obmeth_getattr(iter,name,hash)) != ITER_DONE)
       goto done;
   if (iter->tp_getsets && /* Access instance getsets using `DeeClsPropertyObject' */
      (result = type_obprop_getattr(iter,name,hash)) != ITER_DONE)
       goto done;
   if (iter->tp_members && /* Access instance members using `DeeClsMemberObject' */
      (result = type_obmemb_getattr(iter,name,hash)) != ITER_DONE)
       goto done;
  }
 } while ((iter = DeeType_Base(iter)) != NULL);
 err_unknown_attribute(self,name,ATTR_ACCESS_GET);
 return NULL;
done:
 return result;
}
INTERN DREF DeeObject *DCALL
DeeType_DocInstanceAttrString(DeeTypeObject *__restrict self,
                              char const *__restrict name, dhash_t hash) {
 DeeTypeObject *iter = self;
 DREF DeeObject *result;
 do {
  if (DeeType_IsClass(iter)) {
   struct member_entry *attr;
   struct class_desc *desc = DeeClass_DESC(iter);
   if ((attr = DeeClassDesc_QueryInstanceAttributeStringWithHash(desc,name,hash)) != NULL) {
#ifdef CONFIG_USE_NEW_CLASS_SYSTEM
    if (attr->ca_doc)
        return_reference_((DeeObject *)attr->ca_doc);
#else
    if (attr->cme_doc)
        return_reference_((DeeObject *)attr->cme_doc);
#endif
    err_nodoc_attribute(iter->tp_name,name);
    goto err;
   }
  } else {
   if ((result = membercache_docinstanceattr(iter,name,hash)) != ITER_DONE)
        goto done;
   if (iter->tp_methods && /* Access instance methods using `DeeClsMethodObject' */
      (result = type_obmeth_docattr(iter,name,hash)) != ITER_DONE)
       goto done;
   if (iter->tp_getsets && /* Access instance getsets using `DeeClsPropertyObject' */
      (result = type_obprop_docattr(iter,name,hash)) != ITER_DONE)
       goto done;
   if (iter->tp_members && /* Access instance members using `DeeClsMemberObject' */
      (result = type_obmemb_docattr(iter,name,hash)) != ITER_DONE)
       goto done;
  }
 } while ((iter = DeeType_Base(iter)) != NULL);
 err_unknown_attribute(self,name,ATTR_ACCESS_GET);
err:
 return NULL;
done:
 return result;
}

INTERN DREF DeeObject *DCALL
DeeType_CallInstanceAttrStringKw(DeeTypeObject *__restrict self,
                                 char const *__restrict name, dhash_t hash,
                                 size_t argc, DeeObject **__restrict argv,
                                 DeeObject *kw) {
 DeeTypeObject *iter = self;
 DREF DeeObject *result;
 do {
  if (DeeType_IsClass(iter)) {
   struct member_entry *attr;
   struct class_desc *desc = DeeClass_DESC(iter);
   if ((attr = DeeClassDesc_QueryInstanceAttributeStringWithHash(desc,name,hash)) != NULL) {
    if (!member_mayaccess(iter,attr)) {
     err_protected_member(iter,attr);
     goto err;
    }
#ifdef CONFIG_USE_NEW_CLASS_SYSTEM
    return DeeClass_CallInstanceAttributeKw(iter,attr,argc,argv,kw);
#else
    return class_member_call_kw(iter,&desc->c_class,attr,argc,argv,kw);
#endif
   }
  } else {
   if ((result = membercache_callinstanceattr_kw(self,name,hash,argc,argv,kw)) != ITER_DONE)
       goto done;
   if (iter->tp_methods && /* Access instance methods using `DeeClsMethodObject' */
      (result = type_obmeth_callattr_kw(iter,name,hash,argc,argv,kw)) != ITER_DONE)
       goto done;
   if (iter->tp_getsets && /* Access instance getsets using `DeeClsPropertyObject' */
      (result = type_obprop_callattr_kw(iter,name,hash,argc,argv,kw)) != ITER_DONE)
       goto done;
   if (iter->tp_members && /* Access instance members using `DeeClsMemberObject' */
      (result = type_obmemb_callattr_kw(iter,name,hash,argc,argv,kw)) != ITER_DONE)
       goto done;
  }
 } while ((iter = DeeType_Base(iter)) != NULL);
 err_unknown_attribute(self,name,ATTR_ACCESS_GET);
err:
 return NULL;
done:
 return result;
}

INTERN bool DCALL
DeeType_HasInstanceAttrString(DeeTypeObject *__restrict self,
                              char const *__restrict name, dhash_t hash) {
 do {
  if (DeeType_IsClass(self)) {
   struct class_desc *desc = DeeClass_DESC(self);
   if (DeeClassDesc_QueryInstanceAttributeStringWithHash(desc,name,hash))
       return true;
  } else {
   if (membercache_hasinstanceattr(self,name,hash))
       goto done;
   if (self->tp_methods && /* Access instance methods using `DeeClsMethodObject' */
       type_obmeth_hasattr(self,name,hash))
       goto done;
   if (self->tp_getsets && /* Access instance getsets using `DeeClsPropertyObject' */
       type_obprop_hasattr(self,name,hash))
       goto done;
   if (self->tp_members && /* Access instance members using `DeeClsMemberObject' */
       type_obmemb_hasattr(self,name,hash))
       goto done;
  }
 } while ((self = DeeType_Base(self)) != NULL);
 return false;
done:
 return true;
}
INTERN int (DCALL DeeType_DelInstanceAttrString)(DeeTypeObject *__restrict self,
                                                 char const *__restrict name, dhash_t hash) {
 DeeTypeObject *iter = self; int error;
 do {
  if (DeeType_IsClass(iter)) {
   struct member_entry *attr;
   struct class_desc *desc = DeeClass_DESC(iter);
   if ((attr = DeeClassDesc_QueryInstanceAttributeStringWithHash(desc,name,hash)) != NULL) {
    if (!member_mayaccess(iter,attr)) {
     err_protected_member(iter,attr);
     goto err;
    }
#ifdef CONFIG_USE_NEW_CLASS_SYSTEM
    return DeeClass_DelInstanceAttribute(iter,attr);
#else
    if (attr->ca_flag & CLASS_MEMBER_FCLASSMEM)
        return class_member_del(iter,&desc->c_class,attr);
    /* Instance attr attributes cannot be deleted without an instance operand. */
    goto noaccess;
#endif
   }
  } else {
   if ((error = membercache_delinstanceattr(self,name,hash)) <= 0)
        goto done;
   if (iter->tp_methods && /* Access instance methods using `DeeClsMethodObject' */
       type_obmeth_hasattr(iter,name,hash))
       goto noaccess;
   if (self->tp_getsets && /* Access instance getsets using `DeeClsPropertyObject' */
       type_obprop_hasattr(self,name,hash))
       goto noaccess;
   if (self->tp_members && /* Access instance members using `DeeClsMemberObject' */
       type_obmemb_hasattr(self,name,hash))
       goto noaccess;
  }
 } while ((iter = DeeType_Base(iter)) != NULL);
 err_unknown_attribute(self,name,ATTR_ACCESS_DEL);
err:      return -1;
noaccess: err_cant_access_attribute(iter,name,ATTR_ACCESS_DEL); goto err;
done:     return error;
}
INTERN int (DCALL DeeType_SetInstanceAttrString)(DeeTypeObject *__restrict self,
                                                 char const *__restrict name, dhash_t hash,
                                                 DeeObject *__restrict value) {
 DeeTypeObject *iter = self; int error;
 do {
  if (DeeType_IsClass(iter)) {
   struct member_entry *attr;
   struct class_desc *desc = DeeClass_DESC(iter);
   if ((attr = DeeClassDesc_QueryInstanceAttributeStringWithHash(desc,name,hash)) != NULL) {
    if (!member_mayaccess(iter,attr)) {
     err_protected_member(iter,attr);
     goto err;
    }
#ifdef CONFIG_USE_NEW_CLASS_SYSTEM
    return DeeClass_SetInstanceAttribute(iter,attr,value);
#else
    if (attr->ca_flag & CLASS_MEMBER_FCLASSMEM)
        return class_member_set(iter,&desc->c_class,attr,value);
    /* Instance attr attributes cannot be set without an instance operand. */
    goto noaccess;
#endif
   }
  } else {
   if ((error = membercache_setinstanceattr(self,name,hash,value)) <= 0)
        goto done;
   if (iter->tp_methods && /* Access instance methods using `DeeClsMethodObject' */
       type_obmeth_hasattr(iter,name,hash))
       goto noaccess;
   if (self->tp_getsets && /* Access instance getsets using `DeeClsPropertyObject' */
       type_obprop_hasattr(self,name,hash))
       goto noaccess;
   if (self->tp_members && /* Access instance members using `DeeClsMemberObject' */
       type_obmemb_hasattr(self,name,hash))
       goto noaccess;
  }
 } while ((iter = DeeType_Base(iter)) != NULL);
 err_unknown_attribute(self,name,ATTR_ACCESS_GET);
err:      return -1;
noaccess: err_cant_access_attribute(iter,name,ATTR_ACCESS_SET); goto err;
done:     return error;
}


INTDEF DREF DeeObject *DCALL class_getattr(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, DeeObject *__restrict attr);
INTDEF DREF DeeObject *DCALL class_wrap_getattr(DeeObject *__restrict self, DeeObject *__restrict attr);


INTDEF DREF DeeObject *DCALL
module_getattr(DeeObject *__restrict self,
               DeeObject *__restrict name);
INTDEF int DCALL
module_delattr(DeeObject *__restrict self,
               DeeObject *__restrict name);
INTDEF int DCALL
module_setattr(DeeObject *__restrict self,
               DeeObject *__restrict name,
               DeeObject *__restrict value);


PUBLIC int
(DCALL DeeObject_HasAttr)(DeeObject *__restrict self,
                          /*String*/DeeObject *__restrict attr_name) {
 DeeTypeObject *iter; dhash_t hash;
 struct membercache *cache;
 ASSERT_OBJECT(self);
 ASSERT_OBJECT(attr_name);
 ASSERT(DeeString_Check(attr_name));
 iter = Dee_TYPE(self);
 if (iter == &DeeSuper_Type) {
  iter = DeeSuper_TYPE(self);
  self = DeeSuper_SELF(self);
 }
 hash = DeeString_Hash(attr_name);
 if (iter->tp_attr) goto do_iter_attr;
 cache = &iter->tp_cache;
 if (membercache_hasattr(cache,DeeString_STR(attr_name),hash))
     goto found;
 iter = iter;
 for (;;) {
  if (DeeType_IsClass(iter)) {
   struct member_entry *attr;
   struct class_desc *desc;
   desc = DeeClass_DESC(iter);
   if ((attr = DeeClassDesc_QueryInstanceAttributeWithHash(desc,attr_name,hash)) != NULL)
        return member_mayaccess(iter,attr);
  } else {
   if (iter->tp_methods &&
       type_method_hasattr(cache,iter->tp_methods,
                           DeeString_STR(attr_name),hash))
       goto found;
   if (iter->tp_getsets &&
       type_getset_hasattr(cache,iter->tp_getsets,
                           DeeString_STR(attr_name),hash))
       goto found;
   if (iter->tp_members &&
       type_member_hasattr(cache,iter->tp_members,
                           DeeString_STR(attr_name),hash))
       goto found;
  }
  iter = DeeType_Base(iter);
  if (!iter) break;
  if (iter->tp_attr) {
do_iter_attr:
   if likely(iter->tp_attr->tp_getattr) {
    DREF DeeObject *(DCALL *getattr)(DeeObject *__restrict,DeeObject *__restrict );
    DREF DeeObject *found_object;
    getattr = iter->tp_attr->tp_getattr;
    if (getattr == &module_getattr)
        return DeeModule_HasAttrString((DeeModuleObject *)self,DeeString_STR(attr_name),hash);
    if (getattr == &type_getattr)
        return DeeType_HasAttrString((DeeTypeObject *)self,DeeString_STR(attr_name),hash);
    if (getattr == &class_wrap_getattr)
     found_object = class_getattr(iter,self,attr_name);
    else {
     found_object = (*getattr)(self,attr_name);
    }
    if likely(found_object) {
     Dee_Decref(found_object);
     return 1;
    }
    return CATCH_ATTRIBUTE_ERROR() ? 0 : -1;
   }
   /* Don't consider attributes from lower levels for custom attr access. */
   break;
  }
 }
 return 0;
found:
 return 1;
}
PUBLIC int
(DCALL DeeObject_HasAttrStringHash)(DeeObject *__restrict self,
                                    char const *__restrict attr_name,
                                    dhash_t hash) {
 DeeTypeObject *iter;
 struct membercache *cache;
 ASSERT_OBJECT(self);
 iter = Dee_TYPE(self);
 if (iter == &DeeSuper_Type) {
  iter = DeeSuper_TYPE(self);
  self = DeeSuper_SELF(self);
 }
 if (iter->tp_attr) goto do_iter_attr;
 cache = &iter->tp_cache;
 if (membercache_hasattr(cache,attr_name,hash))
     goto found;
 iter = iter;
 for (;;) {
  if (DeeType_IsClass(iter)) {
   struct member_entry *attr;
   struct class_desc *desc;
   desc = DeeClass_DESC(iter);
   if ((attr = DeeClassDesc_QueryInstanceAttributeStringWithHash(desc,attr_name,hash)) != NULL)
        return member_mayaccess(iter,attr);
  } else {
   if (iter->tp_methods &&
       type_method_hasattr(cache,iter->tp_methods,
                           attr_name,hash))
       goto found;
   if (iter->tp_getsets &&
       type_getset_hasattr(cache,iter->tp_getsets,
                           attr_name,hash))
       goto found;
   if (iter->tp_members &&
       type_member_hasattr(cache,iter->tp_members,
                           attr_name,hash))
       goto found;
  }
  iter = DeeType_Base(iter);
  if (!iter) break;
  if (iter->tp_attr) {
do_iter_attr:
   if likely(iter->tp_attr->tp_getattr) {
    DREF DeeObject *(DCALL *getattr)(DeeObject *__restrict,DeeObject *__restrict );
    DREF DeeObject *found_object,*attr_obj;
    getattr = iter->tp_attr->tp_getattr;
    if (getattr == &module_getattr)
        return DeeModule_HasAttrString((DeeModuleObject *)self,attr_name,hash);
    if (getattr == &type_getattr)
        return DeeType_HasAttrString((DeeTypeObject *)self,attr_name,hash);
    attr_obj = DeeString_NewWithHash(attr_name,hash);
    if unlikely(!attr_obj) return -1;
    if (getattr == &class_wrap_getattr)
     found_object = class_getattr(iter,self,attr_obj);
    else {
     found_object = (*getattr)(self,attr_obj);
    }
    Dee_Decref(attr_obj);
    if likely(found_object) {
     Dee_Decref(found_object);
     return 1;
    }
    return CATCH_ATTRIBUTE_ERROR() ? 0 : -1;
   }
   /* Don't consider attributes from lower levels for custom attr access. */
   break;
  }
 }
 return 0;
found:
 return 1;
}


PUBLIC int
(DCALL DeeObject_BoundAttr)(DeeObject *__restrict self,
                            /*String*/DeeObject *__restrict attr_name) {
 struct membercache *cache;
 int result; dhash_t hash;
 DeeTypeObject *iter = Dee_TYPE(self);
 ASSERT_OBJECT(attr_name);
 ASSERT(DeeString_Check(attr_name));
 if (iter == &DeeSuper_Type) {
  iter = DeeSuper_TYPE(self);
  self = DeeSuper_SELF(self);
 }
 hash = DeeString_Hash(attr_name);
 if (iter->tp_attr) goto do_iter_attr;
 cache = &iter->tp_cache;
 /* Search through the cache for the requested attribute. */
 if ((result = membercache_boundattr(cache,self,
                                     DeeString_STR(attr_name),
                                     hash)) != -2)
      goto done;
 for (;;) {
  if (DeeType_IsClass(iter)) {
   struct member_entry *attr;
   struct class_desc *desc;
   desc = DeeClass_DESC(iter);
   if ((attr = DeeClassDesc_QueryInstanceAttributeWithHash(desc,attr_name,hash)) != NULL) {
    /* Check if we're allowed to access this attr. */
    if (!member_mayaccess(iter,attr)) {
     err_protected_member(iter,attr);
     return -1;
    }
#ifdef CONFIG_USE_NEW_CLASS_SYSTEM
    return DeeInstance_BoundAttribute(iter,
                                      DeeInstance_DESC(desc,
                                                       self),
                                      self,
                                      attr);
#else
    return member_bound(iter,DeeInstance_DESC(desc,self),self,attr);
#endif
   }
  } else {
   if (iter->tp_methods &&
       type_method_hasattr(cache,iter->tp_methods,
                           DeeString_STR(attr_name),hash))
       goto is_bound;
   if (iter->tp_getsets &&
      (result = type_getset_boundattr(cache,iter->tp_getsets,self,
                                      DeeString_STR(attr_name),hash)) != -2)
       goto done;
   if (iter->tp_members &&
       type_member_hasattr(cache,iter->tp_members,
                           DeeString_STR(attr_name),hash))
       goto is_bound;
  }
  iter = DeeType_Base(iter);
  if (!iter) break;
  if (iter->tp_attr) {
do_iter_attr:
   if likely(iter->tp_attr->tp_getattr) {
    DREF DeeObject *(DCALL *getattr)(DeeObject *__restrict,DeeObject *__restrict );
    DREF DeeObject *found_object;
    getattr = iter->tp_attr->tp_getattr;
    if (getattr == &module_getattr)
        return DeeModule_BoundAttrString((DeeModuleObject *)self,DeeString_STR(attr_name),hash);
    if (getattr == &type_getattr)
        return DeeType_BoundAttrString((DeeTypeObject *)self,DeeString_STR(attr_name),hash);
    if (iter->tp_attr->tp_getattr == &class_wrap_getattr) {
     found_object = class_getattr(iter,self,attr_name);
    } else {
     found_object = (*iter->tp_attr->tp_getattr)(self,attr_name);
    }
    if likely(found_object) {
     Dee_Decref(found_object);
     goto is_bound;
    }
    if (CATCH_ATTRIBUTE_ERROR())
        return -3;
    if (DeeError_Catch(&DeeError_UnboundAttribute))
        return 0;
    return -1;
   }
   /* Don't consider attributes from lower levels for custom attr access. */
   break;
  }
 }
 return -2;
done:
 return result;
is_bound:
 return 1;
}

PUBLIC int
(DCALL DeeObject_BoundAttrStringHash)(DeeObject *__restrict self,
                                      char const *__restrict attr_name,
                                      dhash_t hash) {
 struct membercache *cache; int result;
 DeeTypeObject *iter = Dee_TYPE(self);
 if (iter == &DeeSuper_Type) {
  iter = DeeSuper_TYPE(self);
  self = DeeSuper_SELF(self);
 }
 if (iter->tp_attr) goto do_iter_attr;
 cache = &iter->tp_cache;
 /* Search through the cache for the requested attribute. */
 if ((result = membercache_boundattr(cache,self,
                                     attr_name,
                                     hash)) != -2)
      goto done;
 for (;;) {
  if (DeeType_IsClass(iter)) {
   struct member_entry *attr;
   struct class_desc *desc;
   desc = DeeClass_DESC(iter);
   if ((attr = DeeClassDesc_QueryInstanceAttributeStringWithHash(desc,attr_name,hash)) != NULL) {
    /* Check if we're allowed to access this attr. */
    if (!member_mayaccess(iter,attr)) {
     err_protected_member(iter,attr);
     return -1;
    }
#ifdef CONFIG_USE_NEW_CLASS_SYSTEM
    return DeeInstance_BoundAttribute(iter,
                                      DeeInstance_DESC(desc,
                                                       self),
                                      self,
                                      attr);
#else
    return member_bound(iter,DeeInstance_DESC(desc,self),self,attr);
#endif
   }
  } else {
   if (iter->tp_methods &&
       type_method_hasattr(cache,iter->tp_methods,
                           attr_name,hash))
       goto is_bound;
   if (iter->tp_getsets &&
      (result = type_getset_boundattr(cache,iter->tp_getsets,self,
                                      attr_name,hash)) != -2)
       goto done;
   if (iter->tp_members &&
       type_member_hasattr(cache,iter->tp_members,
                           attr_name,hash))
       goto is_bound;
  }
  iter = DeeType_Base(iter);
  if (!iter) break;
  if (iter->tp_attr) {
do_iter_attr:
   if likely(iter->tp_attr->tp_getattr) {
    DREF DeeObject *(DCALL *getattr)(DeeObject *__restrict,DeeObject *__restrict );
    DREF DeeObject *found_object,*attr_obj;
    getattr = iter->tp_attr->tp_getattr;
    attr_obj = DeeString_NewWithHash(attr_name,hash);
    if unlikely(!attr_obj) return -1;
    if (getattr == &module_getattr)
        return DeeModule_BoundAttrString((DeeModuleObject *)self,attr_name,hash);
    if (getattr == &type_getattr)
        return DeeType_BoundAttrString((DeeTypeObject *)self,attr_name,hash);
    if (getattr == &class_wrap_getattr)
     found_object = class_getattr(iter,self,attr_obj);
    else {
     found_object = (*getattr)(self,attr_obj);
    }
    Dee_Decref(attr_obj);
    if likely(found_object) {
     Dee_Decref(found_object);
     goto is_bound;
    }
    if (CATCH_ATTRIBUTE_ERROR())
        return -3;
    if (DeeError_Catch(&DeeError_UnboundAttribute))
        return 0;
    return -1;
   }
   /* Don't consider attributes from lower levels for custom attr access. */
   break;
  }
 }
 return -2;
done:
 return result;
is_bound:
 return 1;
}

PRIVATE DREF DeeObject *DCALL
DeeModule_DocAttrString(DeeModuleObject *__restrict self,
                        char const *__restrict attr_name, dhash_t hash) {
 dhash_t i,perturb;
 ASSERT_OBJECT_TYPE(self,&DeeModule_Type);
 DeeModule_LockSymbols((DeeObject *)self);
 perturb = i = MODULE_HASHST(self,hash);
 for (;; i = MODULE_HASHNX(i,perturb),MODULE_HASHPT(perturb)) {
  struct module_symbol *item = MODULE_HASHIT(self,i);
  if (!item->ss_name) break; /* Not found */
  if (item->ss_hash != hash) continue; /* Non-matching hash */
  if (strcmp(DeeString_STR(item->ss_name),attr_name)) continue;
  if (item->ss_doc) return_reference_((DeeObject *)item->ss_doc);
  DeeModule_UnlockSymbols((DeeObject *)self);
  err_nodoc_attribute(self->mo_name->s_str,attr_name);
  return NULL;
 }
 DeeModule_UnlockSymbols((DeeObject *)self);
 return ITER_DONE;
}

PUBLIC DREF DeeObject *DCALL
DeeObject_DocAttr(DeeObject *__restrict self,
                 /*String*/DeeObject *__restrict attr_name) {
 DREF DeeObject *result; struct membercache *cache;
 DeeTypeObject *iter; dhash_t hash;
 ASSERT_OBJECT(self);
 ASSERT_OBJECT(attr_name);
 ASSERT(DeeString_Check(attr_name));
 iter = Dee_TYPE(self);
#if 0 /* Super isn't used that often. - This is unnecessary overhead! */
 if (iter == &DeeSuper_Type) {
  /* Optimization... */
  iter = DeeSuper_TYPE(self);
  self = DeeSuper_SELF(self);
 }
#endif
 hash = DeeString_Hash(attr_name);
 cache = &iter->tp_cache;
 if (iter->tp_attr) goto do_iter_attr;
 if ((result = membercache_docattr(cache,DeeString_STR(attr_name),
                                   Dee_TYPE(self)->tp_name,hash)) != ITER_DONE)
      goto done;
 for (;;) {
continue_search:
  if (DeeType_IsClass(iter)) {
   struct member_entry *attr;
   struct class_desc *desc = DeeClass_DESC(iter);
   if ((attr = DeeClassDesc_QueryInstanceAttributeWithHash(desc,attr_name,hash)) != NULL) {
#ifdef CONFIG_USE_NEW_CLASS_SYSTEM
    if (attr->ca_doc)
        return_reference_((DeeObject *)attr->ca_doc);
#else
    if (attr->cme_doc)
        return_reference_((DeeObject *)attr->cme_doc);
#endif
    break;
   }
  }
  if (iter->tp_methods &&
     (result = type_method_docattr(cache,iter->tp_methods,iter->tp_name,DeeString_STR(attr_name),hash)) != ITER_DONE)
      goto done;
  if (iter->tp_getsets &&
     (result = type_getset_docattr(cache,iter->tp_getsets,iter->tp_name,DeeString_STR(attr_name),hash)) != ITER_DONE)
      goto done;
  if (iter->tp_members &&
     (result = type_member_docattr(cache,iter->tp_members,iter->tp_name,DeeString_STR(attr_name),hash)) != ITER_DONE)
      goto done;
  iter = DeeType_Base(iter);
  if (!iter) break;
  if (iter->tp_attr) {
   DREF DeeObject *(DCALL *getattr)(DeeObject *__restrict,DeeObject *__restrict);
do_iter_attr:
   getattr = iter->tp_attr->tp_getattr;
   if (getattr == &type_getattr)
       return DeeType_DocAttrString((DeeTypeObject *)self,DeeString_STR(attr_name),hash);
   if (getattr == &module_getattr) {
    if ((result = DeeModule_DocAttrString((DeeModuleObject *)self,
                                           DeeString_STR(attr_name),
                                           hash)) != ITER_DONE)
         goto done;
    goto continue_search;
   }
#if 0 /* Technically, we should break here after executing
       * a dynamic operator, yet no such operand exists...
       * Though because of this, anything we detect from this
       * point forward may not actually be accessible from the
       * type/instance because of custom attribute operators. */
   break;
#endif
  }
 }
 err_unknown_attribute(DeeObject_Class(self),
                       DeeString_STR(attr_name),
                       ATTR_ACCESS_GET);
 return NULL;
done:
 return result;
}

PUBLIC DREF DeeObject *DCALL
DeeObject_CallAttr(DeeObject *__restrict self,
                   /*String*/DeeObject *__restrict attr_name,
                   size_t argc, DeeObject **__restrict argv) {
 DREF DeeObject *result; struct membercache *cache;
 DeeTypeObject *iter; dhash_t hash;
 ASSERT_OBJECT(self);
 ASSERT_OBJECT(attr_name);
 ASSERT(DeeString_Check(attr_name));
 iter = Dee_TYPE(self);
 if (iter == &DeeSuper_Type) {
  /* Optimization... */
  iter = DeeSuper_TYPE(self);
  self = DeeSuper_SELF(self);
 }
 if (iter->tp_attr) goto do_iter_attr;
 hash = DeeString_Hash(attr_name);
 cache = &iter->tp_cache;
 if ((result = membercache_callattr(cache,self,DeeString_STR(attr_name),hash,argc,argv)) != ITER_DONE)
      goto done;
 for (;;) {
  if (DeeType_IsClass(iter)) {
   struct member_entry *attr;
   struct class_desc *desc = DeeClass_DESC(iter);
   if ((attr = DeeClassDesc_QueryInstanceAttributeWithHash(desc,attr_name,hash)) != NULL) {
    /* Check if we're allowed to access this attr. */
    if (!member_mayaccess(iter,attr)) {
     err_protected_member(iter,attr);
     return NULL;
    }
#ifdef CONFIG_USE_NEW_CLASS_SYSTEM
    return DeeInstance_CallAttribute(iter,
                                     DeeInstance_DESC(desc,
                                                      self),
                                     self,attr,argc,argv);
#else
    return member_call(iter,DeeInstance_DESC(desc,self),self,attr,argc,argv);
#endif
   }
  }
  if (iter->tp_methods &&
     (result = type_method_callattr(cache,iter->tp_methods,self,DeeString_STR(attr_name),hash,argc,argv)) != ITER_DONE)
      goto done;
  if (iter->tp_getsets &&
     (result = type_getset_getattr(cache,iter->tp_getsets,self,DeeString_STR(attr_name),hash)) != ITER_DONE)
      goto done_invoke;
  if (iter->tp_members &&
     (result = type_member_getattr(cache,iter->tp_members,self,DeeString_STR(attr_name),hash)) != ITER_DONE)
      goto done_invoke;
  iter = DeeType_Base(iter);
  if (!iter) break;
  if (iter->tp_attr) {
do_iter_attr:
   if (iter->tp_attr->tp_getattr == &type_getattr)
       return type_callattr(self,attr_name,argc,argv);
   if (!iter->tp_attr->tp_getattr) break;
   result = (*iter->tp_attr->tp_getattr)(self,attr_name);
   goto done_invoke;
  }
 }
 err_unknown_attribute(DeeObject_Class(self),
                       DeeString_STR(attr_name),
                       ATTR_ACCESS_GET);
 return NULL;
done_invoke:
 if likely(result) {
  DREF DeeObject *callback_result;
  callback_result = DeeObject_Call(result,argc,argv);
  Dee_Decref(result);
  result = callback_result;
 }
done:
 return result;
}

PUBLIC DREF DeeObject *DCALL
DeeObject_CallAttrKw(DeeObject *__restrict self,
                     /*String*/DeeObject *__restrict attr_name,
                     size_t argc, DeeObject **__restrict argv,
                     DeeObject *kw) {
 DREF DeeObject *result; struct membercache *cache;
 DeeTypeObject *iter; dhash_t hash;
 ASSERT_OBJECT(self);
 ASSERT_OBJECT(attr_name);
 ASSERT(DeeString_Check(attr_name));
 iter = Dee_TYPE(self);
 if (iter == &DeeSuper_Type) {
  /* Optimization... */
  iter = DeeSuper_TYPE(self);
  self = DeeSuper_SELF(self);
 }
 if (iter->tp_attr) goto do_iter_attr;
 hash = DeeString_Hash(attr_name);
 cache = &iter->tp_cache;
 if ((result = membercache_callattr_kw(cache,self,DeeString_STR(attr_name),hash,argc,argv,kw)) != ITER_DONE)
      goto done;
 for (;;) {
  if (DeeType_IsClass(iter)) {
   struct member_entry *attr;
   struct class_desc *desc = DeeClass_DESC(iter);
   if ((attr = DeeClassDesc_QueryInstanceAttributeWithHash(desc,attr_name,hash)) != NULL) {
    /* Check if we're allowed to access this attr. */
    if (!member_mayaccess(iter,attr)) {
     err_protected_member(iter,attr);
     return NULL;
    }
#ifdef CONFIG_USE_NEW_CLASS_SYSTEM
    return DeeInstance_CallAttributeKw(iter,
                                       DeeInstance_DESC(desc,
                                                        self),
                                       self,attr,argc,argv,kw);
#else
    return member_call_kw(iter,DeeInstance_DESC(desc,self),self,attr,argc,argv,kw);
#endif
   }
  }
  if (iter->tp_methods &&
     (result = type_method_callattr_kw(cache,iter->tp_methods,self,DeeString_STR(attr_name),hash,argc,argv,kw)) != ITER_DONE)
      goto done;
  if (iter->tp_getsets &&
     (result = type_getset_getattr(cache,iter->tp_getsets,self,DeeString_STR(attr_name),hash)) != ITER_DONE)
      goto done_invoke;
  if (iter->tp_members &&
     (result = type_member_getattr(cache,iter->tp_members,self,DeeString_STR(attr_name),hash)) != ITER_DONE)
      goto done_invoke;
  iter = DeeType_Base(iter);
  if (!iter) break;
  if (iter->tp_attr) {
do_iter_attr:
   if (iter->tp_attr->tp_getattr == &type_getattr)
       return type_callattr_kw(self,attr_name,argc,argv,kw);
   if (!iter->tp_attr->tp_getattr) break;
   result = (*iter->tp_attr->tp_getattr)(self,attr_name);
   goto done_invoke;
  }
 }
 err_unknown_attribute(DeeObject_Class(self),
                       DeeString_STR(attr_name),
                       ATTR_ACCESS_GET);
 return NULL;
done_invoke:
 if likely(result) {
  DREF DeeObject *callback_result;
  callback_result = DeeObject_CallKw(result,argc,argv,kw);
  Dee_Decref(result);
  result = callback_result;
 }
done:
 return result;
}

PUBLIC DREF DeeObject *DCALL
DeeObject_GetAttrStringHash(DeeObject *__restrict self,
                            char const *__restrict attr_name, dhash_t hash) {
 struct membercache *cache;
 DREF DeeObject *result;
 DeeTypeObject *iter;
 ASSERT_OBJECT(self);
 ASSERT(attr_name);
 iter = Dee_TYPE(self);
 if (DeeSuper_Check(self)) {
  iter = DeeSuper_TYPE(self);
  self = DeeSuper_SELF(self);
 }
 if (iter->tp_attr) goto do_iter_attr;
 cache = &iter->tp_cache;
 if ((result = membercache_getattr(cache,self,attr_name,hash)) != ITER_DONE)
      goto done;
 for (;;) {
  if (DeeType_IsClass(iter)) {
   struct member_entry *attr;
   struct class_desc *desc = DeeClass_DESC(iter);
   if ((attr = DeeClassDesc_QueryInstanceAttributeStringWithHash(desc,attr_name,hash)) != NULL) {
    /* Check if we're allowed to access this attr. */
    if (!member_mayaccess(iter,attr)) {
     err_protected_member(iter,attr);
     return NULL;
    }
#ifdef CONFIG_USE_NEW_CLASS_SYSTEM
    return DeeInstance_GetAttribute(iter,
                                    DeeInstance_DESC(desc,
                                                     self),
                                    self,attr);
#else
    return member_get(iter,DeeInstance_DESC(desc,self),self,attr);
#endif
   }
  }
  if (iter->tp_methods &&
     (result = type_method_getattr(cache,iter->tp_methods,self,attr_name,hash)) != ITER_DONE)
      goto done;
  if (iter->tp_getsets &&
     (result = type_getset_getattr(cache,iter->tp_getsets,self,attr_name,hash)) != ITER_DONE)
      goto done;
  if (iter->tp_members &&
     (result = type_member_getattr(cache,iter->tp_members,self,attr_name,hash)) != ITER_DONE)
      goto done;
  iter = DeeType_Base(iter);
  if (!iter) break;
  if (iter->tp_attr) {
do_iter_attr:
   if (iter->tp_attr->tp_getattr) {
    DREF DeeObject *(DCALL *getattr)(DeeObject *__restrict,DeeObject *__restrict);
    DREF DeeObject *attr_name_ob;
    getattr = iter->tp_attr->tp_getattr;
    if (getattr == &type_getattr)
        return DeeType_GetAttrString((DeeTypeObject *)self,attr_name,hash);
    if (getattr == &module_getattr)
        return DeeModule_GetAttrString((DeeModuleObject *)self,attr_name,hash);
    attr_name_ob = DeeString_NewWithHash(attr_name,hash);
    if unlikely(!attr_name_ob) return NULL;
    ASSERT(((DeeStringObject *)attr_name_ob)->s_hash == DEE_STRING_HASH_UNSET ||
           ((DeeStringObject *)attr_name_ob)->s_hash == hash);
    ((DeeStringObject *)attr_name_ob)->s_hash = hash;
    result = (*getattr)(self,attr_name_ob);
    Dee_Decref(attr_name_ob);
    return result;
   }
   break;
  }
 }
 err_unknown_attribute(Dee_TYPE(self),attr_name,ATTR_ACCESS_GET);
 return NULL;
done:
 return result;
}

PUBLIC int (DCALL DeeObject_DelAttrStringHash)(DeeObject *__restrict self,
                                               char const *__restrict attr_name, dhash_t hash) {
 struct membercache *cache;
 DeeTypeObject *iter;
 int result;
 ASSERT_OBJECT(self);
 ASSERT(attr_name);
 iter = Dee_TYPE(self);
 if (DeeSuper_Check(self)) {
  iter = DeeSuper_TYPE(self);
  self = DeeSuper_SELF(self);
 }
 if (iter->tp_attr) goto do_iter_attr;
 cache = &iter->tp_cache;
 if ((result = membercache_delattr(cache,self,attr_name,hash)) <= 0)
      goto done;
 for (;;) {
  if (DeeType_IsClass(iter)) {
   struct member_entry *attr;
   struct class_desc *desc = DeeClass_DESC(iter);
   if ((attr = DeeClassDesc_QueryInstanceAttributeStringWithHash(desc,attr_name,hash)) != NULL) {
    /* Check if we're allowed to access this attr. */
    if (!member_mayaccess(iter,attr)) {
     err_protected_member(iter,attr);
     goto err;
    }
#ifdef CONFIG_USE_NEW_CLASS_SYSTEM
    return DeeInstance_DelAttribute(iter,
                                    DeeInstance_DESC(desc,
                                                     self),
                                    self,attr);
#else
    return member_del(iter,DeeInstance_DESC(desc,self),self,attr);
#endif
   }
  }
  if (iter->tp_methods &&
      type_method_hasattr(cache,iter->tp_methods,attr_name,hash))
      goto noaccess;
  if (iter->tp_getsets &&
     (result = type_getset_delattr(cache,iter->tp_getsets,self,attr_name,hash)) <= 0)
      goto done;
  if (iter->tp_members &&
     (result = type_member_delattr(cache,iter->tp_members,self,attr_name,hash)) <= 0)
      goto done;
  iter = DeeType_Base(iter);
  if (!iter) break;
  if (iter->tp_attr) {
do_iter_attr:
   if (iter->tp_attr->tp_delattr) {
    int (DCALL *delattr)(DeeObject *__restrict,DeeObject *__restrict);
    DREF DeeObject *attr_name_ob;
    delattr = iter->tp_attr->tp_delattr;
    if (delattr == &type_delattr)
        return DeeType_DelAttrString((DeeTypeObject *)self,attr_name,hash);
    if (delattr == &module_delattr)
        return DeeModule_DelAttrString((DeeModuleObject *)self,attr_name,hash);
    attr_name_ob = DeeString_NewWithHash(attr_name,hash);
    if unlikely(!attr_name_ob) goto err;
    ASSERT(((DeeStringObject *)attr_name_ob)->s_hash == DEE_STRING_HASH_UNSET ||
           ((DeeStringObject *)attr_name_ob)->s_hash == hash);
    ((DeeStringObject *)attr_name_ob)->s_hash = hash;
    result = (*iter->tp_attr->tp_delattr)(self,attr_name_ob);
    Dee_Decref(attr_name_ob);
    return result;
   }
   break;
  }
 }
 /* Check if other attributes exist that may not be accessed in the requested fashion. */
 iter = Dee_TYPE(self);
 err_unknown_attribute(Dee_TYPE(self),attr_name,ATTR_ACCESS_DEL);
err:      return -1;
noaccess: err_cant_access_attribute(iter,attr_name,ATTR_ACCESS_DEL); goto err;
done:     return result;
}

PUBLIC int (DCALL DeeObject_SetAttrStringHash)(DeeObject *__restrict self,
                                               char const *__restrict attr_name, dhash_t hash,
                                               DeeObject *__restrict value) {
 struct membercache *cache;
 DeeTypeObject *iter;
 int result;
 ASSERT_OBJECT(self);
 ASSERT(attr_name);
 ASSERT_OBJECT(value);
 iter = Dee_TYPE(self);
 if (DeeSuper_Check(self)) {
  iter = DeeSuper_TYPE(self);
  self = DeeSuper_SELF(self);
 }
 if (iter->tp_attr) goto do_iter_attr;
 cache = &iter->tp_cache;
 if ((result = membercache_delattr(cache,self,attr_name,hash)) <= 0)
      goto done;
 for (;;) {
  if (DeeType_IsClass(iter)) {
   struct member_entry *attr;
   struct class_desc *desc = DeeClass_DESC(iter);
   if ((attr = DeeClassDesc_QueryInstanceAttributeStringWithHash(desc,attr_name,hash)) != NULL) {
    /* Check if we're allowed to access this attr. */
    if (!member_mayaccess(iter,attr)) {
     err_protected_member(iter,attr);
     goto err;
    }
#ifdef CONFIG_USE_NEW_CLASS_SYSTEM
    return DeeInstance_SetAttribute(iter,
                                    DeeInstance_DESC(desc,
                                                     self),
                                    self,attr,value);
#else
    return member_set(iter,DeeInstance_DESC(desc,self),self,attr,value);
#endif
   }
  }
  if (iter->tp_methods &&
      type_method_hasattr(cache,iter->tp_methods,attr_name,hash))
      goto noaccess;
  if (iter->tp_getsets &&
     (result = type_getset_setattr(cache,iter->tp_getsets,self,attr_name,hash,value)) <= 0)
      goto done;
  if (iter->tp_members &&
     (result = type_member_setattr(cache,iter->tp_members,self,attr_name,hash,value)) <= 0)
      goto done;
  iter = DeeType_Base(iter);
  if (!iter) break;
  if (iter->tp_attr) {
do_iter_attr:
   if (iter->tp_attr->tp_setattr) {
    int (DCALL *setattr)(DeeObject *__restrict,DeeObject *__restrict,DeeObject *__restrict);
    DREF DeeObject *attr_name_ob;
    setattr = iter->tp_attr->tp_setattr;
    if (setattr == &type_setattr)
        return DeeType_SetAttrString((DeeTypeObject *)self,attr_name,hash,value);
    if (setattr == &module_setattr)
        return DeeModule_SetAttrString((DeeModuleObject *)self,attr_name,hash,value);
    attr_name_ob = DeeString_NewWithHash(attr_name,hash);
    if unlikely(!attr_name_ob) goto err;
    ASSERT(((DeeStringObject *)attr_name_ob)->s_hash == DEE_STRING_HASH_UNSET ||
           ((DeeStringObject *)attr_name_ob)->s_hash == hash);
    ((DeeStringObject *)attr_name_ob)->s_hash = hash;
    result = (*setattr)(self,attr_name_ob,value);
    Dee_Decref(attr_name_ob);
    return result;
   }
   break;
  }
 }
 err_unknown_attribute(Dee_TYPE(self),attr_name,ATTR_ACCESS_SET);
err:      return -1;
noaccess: err_cant_access_attribute(iter,attr_name,ATTR_ACCESS_SET); goto err;
done:     return result;
}


PUBLIC DREF DeeObject *DCALL
DeeObject_CallAttrStringHash(DeeObject *__restrict self,
                             char const *__restrict attr_name, dhash_t hash,
                             size_t argc, DeeObject **__restrict argv) {
 struct membercache *cache;
 DeeTypeObject *iter;
 DeeObject *result;
 ASSERT_OBJECT(self);
 ASSERT(attr_name);
 iter = Dee_TYPE(self);
 if (DeeSuper_Check(self)) {
  iter = DeeSuper_TYPE(self);
  self = DeeSuper_SELF(self);
 }
 if (iter->tp_attr) goto do_iter_attr;
 cache = &iter->tp_cache;
 if ((result = membercache_callattr(cache,self,attr_name,hash,argc,argv)) != ITER_DONE)
      goto done;
 for (;;) {
  if (DeeType_IsClass(iter)) {
   struct member_entry *attr;
   struct class_desc *desc = DeeClass_DESC(iter);
   if ((attr = DeeClassDesc_QueryInstanceAttributeStringWithHash(desc,attr_name,hash)) != NULL) {
    /* Check if we're allowed to access this attr. */
    if (!member_mayaccess(iter,attr)) {
     err_protected_member(iter,attr);
     return NULL;
    }
#ifdef CONFIG_USE_NEW_CLASS_SYSTEM
    return DeeInstance_CallAttribute(iter,
                                     DeeInstance_DESC(desc,
                                                      self),
                                     self,attr,argc,argv);
#else
    return member_call(iter,DeeInstance_DESC(desc,self),
                       self,attr,argc,argv);
#endif
   }
  }
  if (iter->tp_methods &&
     (result = type_method_callattr(cache,iter->tp_methods,self,attr_name,hash,argc,argv)) != ITER_DONE)
      goto done;
  if (iter->tp_getsets &&
     (result = type_getset_getattr(cache,iter->tp_getsets,self,attr_name,hash)) != ITER_DONE)
      goto done_invoke;
  if (iter->tp_members &&
     (result = type_member_getattr(cache,iter->tp_members,self,attr_name,hash)) != ITER_DONE)
      goto done_invoke;
  iter = DeeType_Base(iter);
  if (!iter) break;
  if (iter->tp_attr) {
do_iter_attr:
   if (iter->tp_attr->tp_getattr) {
    DREF DeeObject *(DCALL *getattr)(DeeObject *__restrict,DeeObject *__restrict);
    DREF DeeObject *attr_name_ob;
    getattr = iter->tp_attr->tp_getattr;
    if (getattr == &type_getattr)
        return DeeType_CallAttrString((DeeTypeObject *)self,attr_name,hash,argc,argv);
    if (getattr == &module_getattr) {
     result = DeeModule_GetAttrString((DeeModuleObject *)self,attr_name,hash);
     goto done_invoke;
    }
    attr_name_ob = DeeString_NewWithHash(attr_name,hash);
    if unlikely(!attr_name_ob) return NULL;
    ASSERT(((DeeStringObject *)attr_name_ob)->s_hash == DEE_STRING_HASH_UNSET ||
           ((DeeStringObject *)attr_name_ob)->s_hash == hash);
    ((DeeStringObject *)attr_name_ob)->s_hash = hash;
    result = (*getattr)(self,attr_name_ob);
    Dee_Decref(attr_name_ob);
    goto done_invoke;
   }
   break;
  }
 }
 err_unknown_attribute(Dee_TYPE(self),attr_name,ATTR_ACCESS_GET);
 return NULL;
done_invoke:
 if likely(result) {
  DREF DeeObject *callback_result;
  callback_result = DeeObject_Call(result,argc,argv);
  Dee_Decref(result);
  result = callback_result;
 }
done:
 return result;
}

PUBLIC DREF DeeObject *
(DCALL DeeObject_GetAttrString)(DeeObject *__restrict self,
                                char const *__restrict attr_name) {
 return DeeObject_GetAttrStringHash(self,attr_name,hash_str(attr_name));
}
PUBLIC int
(DCALL DeeObject_HasAttrString)(DeeObject *__restrict self,
                                char const *__restrict attr_name) {
 return DeeObject_HasAttrStringHash(self,attr_name,hash_str(attr_name));
}
PUBLIC int
(DCALL DeeObject_BoundAttrString)(DeeObject *__restrict self,
                                  char const *__restrict attr_name) {
 return DeeObject_BoundAttrStringHash(self,attr_name,hash_str(attr_name));
}
PUBLIC int
(DCALL DeeObject_DelAttrString)(DeeObject *__restrict self,
                                char const *__restrict attr_name) {
 return DeeObject_DelAttrStringHash(self,attr_name,hash_str(attr_name));
}
PUBLIC int
(DCALL DeeObject_SetAttrString)(DeeObject *__restrict self,
                                char const *__restrict attr_name,
                                DeeObject *__restrict value) {
 return DeeObject_SetAttrStringHash(self,attr_name,hash_str(attr_name),value);
}
PUBLIC DREF DeeObject *
(DCALL DeeObject_CallAttrString)(DeeObject *__restrict self,
                                 char const *__restrict attr_name,
                                 size_t argc, DeeObject **__restrict argv) {
 return DeeObject_CallAttrStringHash(self,attr_name,hash_str(attr_name),argc,argv);
}



#ifdef CONFIG_VA_LIST_IS_STACK_POINTER
#ifndef __NO_DEFINE_ALIAS
DEFINE_PUBLIC_ALIAS(ASSEMBLY_NAME(DeeObject_VCallAttrPack,16),
                    ASSEMBLY_NAME(DeeObject_CallAttr,16));
#else
PUBLIC DREF DeeObject *DCALL
DeeObject_VCallAttrPack(DeeObject *__restrict self,
                        /*String*/DeeObject *__restrict attr_name,
                        size_t argc, va_list args) {
 return DeeObject_CallAttr(self,attr_name,argc,(DeeObject **)args);
}
#endif
#else
PUBLIC DREF DeeObject *DCALL
DeeObject_VCallAttrPack(DeeObject *__restrict self,
                        /*String*/DeeObject *__restrict attr_name,
                        size_t argc, va_list args) {
 DREF DeeObject *result,*args_tuple;
 args_tuple = DeeTuple_VPackSymbolic(argc,args);
 if unlikely(!args_tuple) return NULL;
 result = DeeObject_CallAttr(self,attr_name,argc,
                             DeeTuple_ELEM(args_tuple));
 DeeTuple_DecrefSymbolic(args_tuple);
 return result;
}
#endif
PUBLIC ATTR_SENTINEL DREF DeeObject *
DeeObject_CallAttrPack(DeeObject *__restrict self,
                       /*String*/DeeObject *__restrict attr_name,
                       size_t argc, ...) {
 DREF DeeObject *result;
 va_list args;
 va_start(args,argc);
 result = DeeObject_VCallAttrPack(self,attr_name,argc,args);
 va_end(args);
 return result;
}
PUBLIC DREF DeeObject *DCALL
DeeObject_VCallAttrf(DeeObject *__restrict self,
                     /*String*/DeeObject *__restrict attr_name,
                     char const *__restrict format, va_list args) {
 DREF DeeObject *result,*args_tuple;
 args_tuple = DeeTuple_VNewf(format,args);
 if unlikely(!args_tuple) return NULL;
 result = DeeObject_CallAttr(self,attr_name,
                             DeeTuple_SIZE(args_tuple),
                             DeeTuple_ELEM(args_tuple));
 Dee_Decref(args_tuple);
 return result;
}
PUBLIC DREF DeeObject *
DeeObject_CallAttrf(DeeObject *__restrict self,
                    /*String*/DeeObject *__restrict attr_name,
                    char const *__restrict format, ...) {
 DREF DeeObject *result;
 va_list args;
 va_start(args,format);
 result = DeeObject_VCallAttrf(self,attr_name,format,args);
 va_end(args);
 return result;
}

#ifdef CONFIG_VA_LIST_IS_STACK_POINTER
#ifndef __NO_DEFINE_ALIAS
DEFINE_PUBLIC_ALIAS(ASSEMBLY_NAME(DeeObject_VCallAttrStringPack,16),
                    ASSEMBLY_NAME(DeeObject_CallAttrString,16));
DEFINE_PUBLIC_ALIAS(ASSEMBLY_NAME(DeeObject_VCallAttrStringHashPack,20),
                    ASSEMBLY_NAME(DeeObject_CallAttrStringHash,20));
#else
PUBLIC DREF DeeObject *
(DCALL DeeObject_VCallAttrStringPack)(DeeObject *__restrict self,
                                      char const *__restrict attr_name,
                                      size_t argc, va_list args) {
 return DeeObject_CallAttrString(self,attr_name,argc,(DeeObject **)args);
}
PUBLIC DREF DeeObject *
(DCALL DeeObject_VCallAttrStringHashPack)(DeeObject *__restrict self,
                                          char const *__restrict attr_name,
                                          dhash_t hash, size_t argc, va_list args) {
 return DeeObject_CallAttrStringHash(self,attr_name,hash,argc,(DeeObject **)args);
}
#endif
#else
PUBLIC DREF DeeObject *
(DCALL DeeObject_VCallAttrStringHashPack)(DeeObject *__restrict self,
                                          char const *__restrict attr_name,
                                          dhash_t hash, size_t argc, va_list args) {
 DREF DeeObject *result,*args_tuple;
 args_tuple = DeeTuple_VPackSymbolic(argc,args);
 if unlikely(!args_tuple) return NULL;
 result = DeeObject_CallAttrStringHash(self,attr_name,hash,argc,
                                   DeeTuple_ELEM(args_tuple));
 DeeTuple_DecrefSymbolic(args_tuple);
 return result;
}
PUBLIC DREF DeeObject *
(DCALL DeeObject_VCallAttrStringPack)(DeeObject *__restrict self,
                                      char const *__restrict attr_name,
                                      size_t argc, va_list args) {
 return DeeObject_VCallAttrStringHashPack(self,attr_name,hash_str(attr_name),argc,args);
}
#endif
PUBLIC ATTR_SENTINEL DREF DeeObject *
(DeeObject_CallAttrStringPack)(DeeObject *__restrict self,
                               char const *__restrict attr_name,
                               size_t argc, ...) {
 DREF DeeObject *result;
 va_list args;
 va_start(args,argc);
 result = DeeObject_VCallAttrStringPack(self,attr_name,argc,args);
 va_end(args);
 return result;
}
PUBLIC ATTR_SENTINEL DREF DeeObject *
DeeObject_CallAttrStringPackHash(DeeObject *__restrict self,
                                 char const *__restrict attr_name,
                                 dhash_t hash, size_t argc, ...) {
 DREF DeeObject *result;
 va_list args;
 va_start(args,argc);
 result = DeeObject_VCallAttrStringHashPack(self,attr_name,hash,argc,args);
 va_end(args);
 return result;
}
PUBLIC DREF DeeObject *DCALL
DeeObject_VCallAttrStringHashf(DeeObject *__restrict self,
                               char const *__restrict attr_name, dhash_t hash,
                               char const *__restrict format, va_list args) {
 DREF DeeObject *result,*args_tuple;
 args_tuple = DeeTuple_VNewf(format,args);
 if unlikely(!args_tuple) return NULL;
 result = DeeObject_CallAttrStringHash(self,attr_name,hash,
                                       DeeTuple_SIZE(args_tuple),
                                       DeeTuple_ELEM(args_tuple));
 Dee_Decref(args_tuple);
 return result;
}
PUBLIC DREF DeeObject *
(DCALL DeeObject_VCallAttrStringf)(DeeObject *__restrict self,
                                   char const *__restrict attr_name,
                                   char const *__restrict format, va_list args) {
 return DeeObject_VCallAttrStringHashf(self,attr_name,hash_str(attr_name),format,args);
}
PUBLIC DREF DeeObject *
DeeObject_CallAttrStringHashf(DeeObject *__restrict self,
                              char const *__restrict attr_name, dhash_t hash,
                              char const *__restrict format, ...) {
 DREF DeeObject *result;
 va_list args;
 va_start(args,format);
 result = DeeObject_VCallAttrStringHashf(self,attr_name,hash,format,args);
 va_end(args);
 return result;
}
PUBLIC DREF DeeObject *
(DeeObject_CallAttrStringf)(DeeObject *__restrict self,
                            char const *__restrict attr_name,
                            char const *__restrict format, ...) {
 DREF DeeObject *result;
 va_list args;
 va_start(args,format);
 result = DeeObject_VCallAttrStringf(self,attr_name,format,args);
 va_end(args);
 return result;
}


DECL_END

#endif /* !GUARD_DEEMON_RUNTIME_ATTRIBUTE_C */
