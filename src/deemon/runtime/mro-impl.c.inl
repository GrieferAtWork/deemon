/* Copyright (c) 2019 Griefer@Work                                            *
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
#ifdef __INTELLISENSE__
#include "mro.c"
#define MRO_LEN 1
#endif

DECL_BEGIN

#ifdef MRO_LEN
#define S(without_len, with_len) with_len
#define NLen(x)                  x##Len
#define N_len(x)                 x##_len
#define IF_LEN(...)              __VA_ARGS__
#define IF_NLEN(...)             /* nothing */
#define ATTR_ARG                 char const *__restrict attr, size_t attrlen
#define ATTREQ(item)             streq_len((item)->mcs_name, attr, attrlen)
#define NAMEEQ(name)             streq_len(name, attr, attrlen)
#else /* MRO_LEN */
#define S(without_len, with_len) without_len
#define NLen(x)                  x
#define N_len(x)                 x
#define IF_LEN(...)              /* nothing */
#define IF_NLEN(...)             __VA_ARGS__
#define ATTR_ARG                 char const *__restrict attr
#define ATTREQ(item)             streq((item)->mcs_name, attr)
#define NAMEEQ(name)             streq(name, attr)
#endif /* !MRO_LEN */


/* Lookup an attribute from cache.
 * @return: * :        The attribute value.
 * @return: NULL:      An error occurred.
 * @return: ITER_DONE: The attribute could not be found in the cache. */
INTERN DREF DeeObject *DCALL
NLen(DeeType_GetCachedAttr)(DeeTypeObject *__restrict tp_self,
                            DeeObject *__restrict self,
                            ATTR_ARG, dhash_t hash) {
 dhash_t i,perturb;
 MEMBERCACHE_READ(&tp_self->tp_cache);
 if unlikely(!tp_self->tp_cache.mc_table) goto done;
 perturb = i = MEMBERCACHE_HASHST(&tp_self->tp_cache,hash);
 for (;; i = MEMBERCACHE_HASHNX(i,perturb),MEMBERCACHE_HASHPT(perturb)) {
  struct membercache_slot *item = MEMBERCACHE_HASHIT(&tp_self->tp_cache,i);
  if (item->mcs_type == MEMBERCACHE_UNUSED) break;
  if (item->mcs_hash != hash) continue;
  if (!ATTREQ(item)) continue;
  switch (item->mcs_type) {
  {
   dobjmethod_t func;
  case MEMBERCACHE_METHOD:
   func = item->mcs_method.m_func;
   if (item->mcs_method.m_flag & TYPE_METHOD_FKWDS) {
    MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
    return DeeKwObjMethod_New((dkwobjmethod_t)func,self);
   }
   MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
   return DeeObjMethod_New(func,self);
  }
  {
   dgetmethod_t getter;
   DeeTypeObject *type;
  case MEMBERCACHE_GETSET:
   getter = item->mcs_getset.gs_get;
   if likely(getter) {
    MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
    return (*getter)(self);
   }
   type = item->mcs_decl;
   MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
   err_cant_access_attribute(type,attr,ATTR_ACCESS_GET);
   goto err;
  }
  {
   struct buffer { uint8_t dat[COMPILER_OFFSETOF(struct type_member,m_doc)]; } buf;
  case MEMBERCACHE_MEMBER:
   buf = *(struct buffer *)&item->mcs_member;
   MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
   return type_member_get((struct type_member *)&buf,self);
  }
  {
   struct class_attribute *catt;
   struct class_desc *desc;
  case MEMBERCACHE_ATTRIB:
   catt = item->mcs_attrib.a_attr;
   desc = item->mcs_attrib.a_desc;
   MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
   return DeeInstance_GetAttribute(desc,
                                   DeeInstance_DESC(desc,self),
                                   self,catt);
  }
  default: __builtin_unreachable();
  }
 }
done:
 MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
 return ITER_DONE;
err:
 return NULL;
}
INTERN DREF DeeObject *DCALL
NLen(DeeType_GetCachedClassAttr)(DeeTypeObject *__restrict tp_self,
                                 ATTR_ARG, dhash_t hash) {
 dhash_t i,perturb;
 MEMBERCACHE_READ(&tp_self->tp_class_cache);
 if unlikely(!tp_self->tp_class_cache.mc_table) goto done;
 perturb = i = MEMBERCACHE_HASHST(&tp_self->tp_class_cache,hash);
 for (;; i = MEMBERCACHE_HASHNX(i,perturb),MEMBERCACHE_HASHPT(perturb)) {
  struct membercache_slot *item = MEMBERCACHE_HASHIT(&tp_self->tp_class_cache,i);
  if (item->mcs_type == MEMBERCACHE_UNUSED) break;
  if (item->mcs_hash != hash) continue;
  if (!ATTREQ(item)) continue;
  switch (item->mcs_type) {
  {
   dobjmethod_t func;
  case MEMBERCACHE_METHOD:
   func = item->mcs_method.m_func;
   if (item->mcs_method.m_flag & TYPE_METHOD_FKWDS) {
    MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
    return DeeKwObjMethod_New((dkwobjmethod_t)func,(DeeObject *)tp_self);
   }
   MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
   return DeeObjMethod_New(func,(DeeObject *)tp_self);
  }
  {
   dgetmethod_t getter;
   DeeTypeObject *type;
  case MEMBERCACHE_GETSET:
   getter = item->mcs_getset.gs_get;
   if likely(getter) {
    MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
    return (*getter)((DeeObject *)tp_self);
   }
   type = item->mcs_decl;
   MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
   err_cant_access_attribute(type,attr,ATTR_ACCESS_GET);
   goto err;
  }
  {
   struct buffer { uint8_t dat[COMPILER_OFFSETOF(struct type_member,m_doc)]; } buf;
  case MEMBERCACHE_MEMBER:
   buf = *(struct buffer *)&item->mcs_member;
   MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
   return type_member_get((struct type_member *)&buf,(DeeObject *)tp_self);
  }
  {
   struct class_attribute *catt;
   struct class_desc *desc;
  case MEMBERCACHE_ATTRIB:
   catt = item->mcs_attrib.a_attr;
   desc = item->mcs_attrib.a_desc;
   MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
   return DeeInstance_GetAttribute(desc,class_desc_as_instance(desc),(DeeObject *)tp_self,catt);
  }
  {
   dobjmethod_t func;
   DeeTypeObject *type;
  case MEMBERCACHE_INSTANCE_METHOD:
   func = item->mcs_method.m_func;
   type = item->mcs_decl;
   if (item->mcs_method.m_flag & TYPE_METHOD_FKWDS) {
    MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
    return DeeKwClsMethod_New(type,(dkwobjmethod_t)func);
   }
   MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
   return DeeClsMethod_New(type,func);
  }
  {
   dgetmethod_t get;
   ddelmethod_t del;
   dsetmethod_t set;
   DeeTypeObject *type;
  case MEMBERCACHE_INSTANCE_GETSET:
   get = item->mcs_getset.gs_get;
   del = item->mcs_getset.gs_del;
   set = item->mcs_getset.gs_set;
   type = item->mcs_decl;
   MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
   return DeeClsProperty_New(type,get,del,set);
  }
  {
   struct type_member member;
   DeeTypeObject *type;
  case MEMBERCACHE_INSTANCE_MEMBER:
   member = item->mcs_member;
   type = item->mcs_decl;
   MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
   return DeeClsMember_New(type,&member);
  }
  {
   struct class_attribute *catt;
   DeeTypeObject *type;
  case MEMBERCACHE_INSTANCE_ATTRIB:
   catt = item->mcs_attrib.a_attr;
   type = item->mcs_decl;
   MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
   return DeeClass_GetInstanceAttribute(type,catt);
  }
  default: __builtin_unreachable();
  }
 }
done:
 MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
 return ITER_DONE;
err:
 return NULL;
}
INTERN DREF DeeObject *DCALL
NLen(DeeType_GetCachedInstanceAttr)(DeeTypeObject *__restrict tp_self,
                                    ATTR_ARG, dhash_t hash) {
 dhash_t i,perturb;
 MEMBERCACHE_READ(&tp_self->tp_cache);
 if unlikely(!tp_self->tp_cache.mc_table) goto done;
 perturb = i = MEMBERCACHE_HASHST(&tp_self->tp_cache,hash);
 for (;; i = MEMBERCACHE_HASHNX(i,perturb),MEMBERCACHE_HASHPT(perturb)) {
  struct membercache_slot *item = MEMBERCACHE_HASHIT(&tp_self->tp_cache,i);
  if (item->mcs_type == MEMBERCACHE_UNUSED) break;
  if (item->mcs_hash != hash) continue;
  if (!ATTREQ(item)) continue;
  switch (item->mcs_type) {
  {
   dobjmethod_t func;
   DeeTypeObject *type;
  case MEMBERCACHE_METHOD:
   func = item->mcs_method.m_func;
   type = item->mcs_decl;
   if (item->mcs_method.m_flag & TYPE_METHOD_FKWDS) {
    MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
    return DeeKwClsMethod_New(type,(dkwobjmethod_t)func);
   }
   MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
   return DeeClsMethod_New(type,func);
  }
  {
   dgetmethod_t get;
   ddelmethod_t del;
   dsetmethod_t set;
   DeeTypeObject *type;
  case MEMBERCACHE_GETSET:
   get = item->mcs_getset.gs_get;
   del = item->mcs_getset.gs_del;
   set = item->mcs_getset.gs_set;
   type = item->mcs_decl;
   MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
   return DeeClsProperty_New(type,get,del,set);
  }
  {
   struct type_member member;
   DeeTypeObject *type;
  case MEMBERCACHE_MEMBER:
   member = item->mcs_member;
   type = item->mcs_decl;
   MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
   return DeeClsMember_New(type,&member);
  }
  {
   struct class_attribute *catt;
   DeeTypeObject *type;
  case MEMBERCACHE_ATTRIB:
   catt = item->mcs_attrib.a_attr;
   type = item->mcs_decl;
   MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
   return DeeClass_GetInstanceAttribute(type,catt);
  }
  default: __builtin_unreachable();
  }
 }
done:
 MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
 return ITER_DONE;
}


/* @return: 1 : Attribute is bound.
 * @return: 0 : Attribute isn't bound.
 * @return: -1: An error occurred.
 * @return: -2: The attribute doesn't exist. */
INTERN int DCALL
NLen(DeeType_BoundCachedAttr)(DeeTypeObject *__restrict tp_self,
                              DeeObject *__restrict self,
                              ATTR_ARG, dhash_t hash) {
 dhash_t i,perturb;
 MEMBERCACHE_READ(&tp_self->tp_cache);
 if unlikely(!tp_self->tp_cache.mc_table) goto done;
 perturb = i = MEMBERCACHE_HASHST(&tp_self->tp_cache,hash);
 for (;; i = MEMBERCACHE_HASHNX(i,perturb),MEMBERCACHE_HASHPT(perturb)) {
  struct membercache_slot *item = MEMBERCACHE_HASHIT(&tp_self->tp_cache,i);
  if (item->mcs_type == MEMBERCACHE_UNUSED) break;
  if (item->mcs_hash != hash) continue;
  if (!ATTREQ(item)) continue;
  switch (item->mcs_type) {
  case MEMBERCACHE_METHOD:
   MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
   return 1;
  {
   dgetmethod_t getter;
   DREF DeeObject *temp;
  case MEMBERCACHE_GETSET:
   getter = item->mcs_getset.gs_get;
   MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
   if unlikely(!getter) return 0;
   temp = (*getter)(self);
   if unlikely(!temp) {
    if (CATCH_ATTRIBUTE_ERROR())
        return -3;
    if (DeeError_Catch(&DeeError_UnboundAttribute))
        return 0;
    goto err;
   }
   Dee_Decref(temp);
   return 1;
  }
  {
   struct buffer { uint8_t dat[COMPILER_OFFSETOF(struct type_member,m_doc)]; } buf;
  case MEMBERCACHE_MEMBER:
   buf = *(struct buffer *)&item->mcs_member;
   MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
   return type_member_bound((struct type_member *)&buf,self);
  }
  {
   struct class_attribute *catt;
   struct class_desc *desc;
  case MEMBERCACHE_ATTRIB:
   catt = item->mcs_attrib.a_attr;
   desc = item->mcs_attrib.a_desc;
   MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
   return DeeInstance_BoundAttribute(desc,
                                     DeeInstance_DESC(desc,self),
                                     self,catt);
  }
  default: __builtin_unreachable();
  }
 }
done:
 MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
 return -2;
err:
 return -1;
}
INTERN int DCALL
NLen(DeeType_BoundCachedClassAttr)(DeeTypeObject *__restrict tp_self,
                                   ATTR_ARG, dhash_t hash) {
 dhash_t i,perturb;
 MEMBERCACHE_READ(&tp_self->tp_class_cache);
 if unlikely(!tp_self->tp_class_cache.mc_table) goto done;
 perturb = i = MEMBERCACHE_HASHST(&tp_self->tp_class_cache,hash);
 for (;; i = MEMBERCACHE_HASHNX(i,perturb),MEMBERCACHE_HASHPT(perturb)) {
  struct membercache_slot *item = MEMBERCACHE_HASHIT(&tp_self->tp_class_cache,i);
  if (item->mcs_type == MEMBERCACHE_UNUSED) break;
  if (item->mcs_hash != hash) continue;
  if (!ATTREQ(item)) continue;
  switch (item->mcs_type) {
  case MEMBERCACHE_METHOD:
  case MEMBERCACHE_INSTANCE_METHOD:
  case MEMBERCACHE_INSTANCE_GETSET:
  case MEMBERCACHE_INSTANCE_MEMBER:
   MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
   return 1;
  {
   dgetmethod_t getter;
   DREF DeeObject *temp;
  case MEMBERCACHE_GETSET:
   getter = item->mcs_getset.gs_get;
   MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
   if unlikely(!getter) return 0;
   temp = (*getter)((DeeObject *)tp_self);
   if unlikely(!temp) {
    if (CATCH_ATTRIBUTE_ERROR())
        return -3;
    if (DeeError_Catch(&DeeError_UnboundAttribute))
        return 0;
    goto err;
   }
   Dee_Decref(temp);
   return 1;
  }
  {
   struct buffer { uint8_t dat[COMPILER_OFFSETOF(struct type_member,m_doc)]; } buf;
  case MEMBERCACHE_MEMBER:
   buf = *(struct buffer *)&item->mcs_member;
   MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
   return type_member_bound((struct type_member *)&buf,(DeeObject *)tp_self);
  }
  {
   struct class_attribute *catt;
   struct class_desc *desc;
  case MEMBERCACHE_ATTRIB:
   catt = item->mcs_attrib.a_attr;
   desc = item->mcs_attrib.a_desc;
   MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
   return DeeInstance_BoundAttribute(desc,class_desc_as_instance(desc),(DeeObject *)tp_self,catt);
  }
  {
   struct class_attribute *catt;
   DeeTypeObject *type;
  case MEMBERCACHE_INSTANCE_ATTRIB:
   catt = item->mcs_attrib.a_attr;
   type = item->mcs_decl;
   MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
   return DeeClass_BoundInstanceAttribute(type,catt);
  }
  default: __builtin_unreachable();
  }
 }
done:
 MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
 return -2;
err:
 return -1;
}
INTERN int DCALL
NLen(DeeType_BoundCachedInstanceAttr)(DeeTypeObject *__restrict tp_self,
                                      ATTR_ARG, dhash_t hash) {
 dhash_t i,perturb;
 MEMBERCACHE_READ(&tp_self->tp_cache);
 if unlikely(!tp_self->tp_cache.mc_table) goto done;
 perturb = i = MEMBERCACHE_HASHST(&tp_self->tp_cache,hash);
 for (;; i = MEMBERCACHE_HASHNX(i,perturb),MEMBERCACHE_HASHPT(perturb)) {
  struct membercache_slot *item = MEMBERCACHE_HASHIT(&tp_self->tp_cache,i);
  if (item->mcs_type == MEMBERCACHE_UNUSED) break;
  if (item->mcs_hash != hash) continue;
  if (!ATTREQ(item)) continue;
  switch (item->mcs_type) {
  case MEMBERCACHE_METHOD:
  case MEMBERCACHE_GETSET:
  case MEMBERCACHE_MEMBER:
   MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
   return 1;
  {
   struct class_attribute *catt;
   DeeTypeObject *type;
  case MEMBERCACHE_ATTRIB:
   catt = item->mcs_attrib.a_attr;
   type = item->mcs_decl;
   MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
   return DeeClass_BoundInstanceAttribute(type,catt);
  }
  default: __builtin_unreachable();
  }
 }
done:
 MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
 return -2;
}


/* @return: true : The attribute exists.
 * @return: false: The attribute doesn't exist. */
INTERN bool DCALL
NLen(DeeType_HasCachedAttr)(DeeTypeObject *__restrict tp_self,
                            ATTR_ARG, dhash_t hash) {
 dhash_t i,perturb;
 MEMBERCACHE_READ(&tp_self->tp_cache);
 if unlikely(!tp_self->tp_cache.mc_table) goto done;
 perturb = i = MEMBERCACHE_HASHST(&tp_self->tp_cache,hash);
 for (;; i = MEMBERCACHE_HASHNX(i,perturb),MEMBERCACHE_HASHPT(perturb)) {
  struct membercache_slot *item = MEMBERCACHE_HASHIT(&tp_self->tp_cache,i);
  if (item->mcs_type == MEMBERCACHE_UNUSED) break;
  if (item->mcs_hash != hash) continue;
  if (!ATTREQ(item)) continue;
  MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
  return true;
 }
done:
 MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
 return false;
}
INTERN bool DCALL
NLen(DeeType_HasCachedClassAttr)(DeeTypeObject *__restrict tp_self,
                                 ATTR_ARG, dhash_t hash) {
 dhash_t i,perturb;
 MEMBERCACHE_READ(&tp_self->tp_class_cache);
 if unlikely(!tp_self->tp_class_cache.mc_table) goto done;
 perturb = i = MEMBERCACHE_HASHST(&tp_self->tp_class_cache,hash);
 for (;; i = MEMBERCACHE_HASHNX(i,perturb),MEMBERCACHE_HASHPT(perturb)) {
  struct membercache_slot *item = MEMBERCACHE_HASHIT(&tp_self->tp_class_cache,i);
  if (item->mcs_type == MEMBERCACHE_UNUSED) break;
  if (item->mcs_hash != hash) continue;
  if (!ATTREQ(item)) continue;
  MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
  return true;
 }
done:
 MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
 return false;
}

/* @return:  1: The attribute could not be found in the cache.
 * @return:  0: Successfully invoked the delete-operator on the attribute.
 * @return: -1: An error occurred. */
INTERN int DCALL
NLen(DeeType_DelCachedAttr)(DeeTypeObject *__restrict tp_self,
                            DeeObject *__restrict self,
                            ATTR_ARG, dhash_t hash) {
 dhash_t i,perturb;
 MEMBERCACHE_READ(&tp_self->tp_cache);
 if unlikely(!tp_self->tp_cache.mc_table) goto done;
 perturb = i = MEMBERCACHE_HASHST(&tp_self->tp_cache,hash);
 for (;; i = MEMBERCACHE_HASHNX(i,perturb),MEMBERCACHE_HASHPT(perturb)) {
  struct membercache_slot *item = MEMBERCACHE_HASHIT(&tp_self->tp_cache,i);
  if (item->mcs_type == MEMBERCACHE_UNUSED) break;
  if (item->mcs_hash != hash) continue;
  if (!ATTREQ(item)) continue;
  switch (item->mcs_type) {
  {
   DeeTypeObject *type;
  case MEMBERCACHE_METHOD:
   type = item->mcs_decl;
   MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
   return err_cant_access_attribute(type,attr,ATTR_ACCESS_DEL);
  }
  {
   ddelmethod_t del;
   DeeTypeObject *type;
  case MEMBERCACHE_GETSET:
   del = item->mcs_getset.gs_del;
   if likely(del) {
    MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
    return (*del)(self);
   }
   type = item->mcs_decl;
   MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
   err_cant_access_attribute(type,attr,ATTR_ACCESS_DEL);
   goto err;
  }
  {
   struct buffer { uint8_t dat[COMPILER_OFFSETOF(struct type_member,m_doc)]; } buf;
  case MEMBERCACHE_MEMBER:
   buf = *(struct buffer *)&item->mcs_member;
   MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
   return type_member_del((struct type_member *)&buf,self);
  }
  {
   struct class_attribute *catt;
   struct class_desc *desc;
  case MEMBERCACHE_ATTRIB:
   catt = item->mcs_attrib.a_attr;
   desc = item->mcs_attrib.a_desc;
   MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
   return DeeInstance_DelAttribute(desc,
                                   DeeInstance_DESC(desc,self),
                                   self,catt);
  }
  default: __builtin_unreachable();
  }
 }
done:
 MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
 return 1;
err:
 return -1;
}
INTERN int DCALL
NLen(DeeType_DelCachedClassAttr)(DeeTypeObject *__restrict tp_self,
                                 ATTR_ARG, dhash_t hash) {
 dhash_t i,perturb;
 MEMBERCACHE_READ(&tp_self->tp_class_cache);
 if unlikely(!tp_self->tp_class_cache.mc_table) goto done;
 perturb = i = MEMBERCACHE_HASHST(&tp_self->tp_class_cache,hash);
 for (;; i = MEMBERCACHE_HASHNX(i,perturb),MEMBERCACHE_HASHPT(perturb)) {
  struct membercache_slot *item = MEMBERCACHE_HASHIT(&tp_self->tp_class_cache,i);
  if (item->mcs_type == MEMBERCACHE_UNUSED) break;
  if (item->mcs_hash != hash) continue;
  if (!ATTREQ(item)) continue;
  switch (item->mcs_type) {
  {
   DeeTypeObject *type;
  case MEMBERCACHE_METHOD:
  case MEMBERCACHE_INSTANCE_METHOD:
  case MEMBERCACHE_INSTANCE_GETSET:
  case MEMBERCACHE_INSTANCE_MEMBER:
   type = item->mcs_decl;
   MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
   return err_cant_access_attribute(type,attr,ATTR_ACCESS_DEL);
  }
  {
   ddelmethod_t del;
   DeeTypeObject *type;
  case MEMBERCACHE_GETSET:
   del = item->mcs_getset.gs_del;
   if likely(del) {
    MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
    return (*del)((DeeObject *)tp_self);
   }
   type = item->mcs_decl;
   MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
   err_cant_access_attribute(type,attr,ATTR_ACCESS_DEL);
   goto err;
  }
  {
   struct buffer { uint8_t dat[COMPILER_OFFSETOF(struct type_member,m_doc)]; } buf;
  case MEMBERCACHE_MEMBER:
   buf = *(struct buffer *)&item->mcs_member;
   MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
   return type_member_del((struct type_member *)&buf,(DeeObject *)tp_self);
  }
  {
   struct class_attribute *catt;
   struct class_desc *desc;
  case MEMBERCACHE_ATTRIB:
   catt = item->mcs_attrib.a_attr;
   desc = item->mcs_attrib.a_desc;
   MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
   return DeeInstance_DelAttribute(desc,class_desc_as_instance(desc),(DeeObject *)tp_self,catt);
  }
  {
   struct class_attribute *catt;
   DeeTypeObject *type;
  case MEMBERCACHE_INSTANCE_ATTRIB:
   catt = item->mcs_attrib.a_attr;
   type = item->mcs_decl;
   MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
   return DeeClass_DelInstanceAttribute(type,catt);
  }
  default: __builtin_unreachable();
  }
 }
done:
 MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
 return 1;
err:
 return -1;
}
INTERN int DCALL
NLen(DeeType_DelCachedInstanceAttr)(DeeTypeObject *__restrict tp_self,
                                    ATTR_ARG, dhash_t hash) {
 dhash_t i,perturb;
 MEMBERCACHE_READ(&tp_self->tp_cache);
 if unlikely(!tp_self->tp_cache.mc_table) goto done;
 perturb = i = MEMBERCACHE_HASHST(&tp_self->tp_cache,hash);
 for (;; i = MEMBERCACHE_HASHNX(i,perturb),MEMBERCACHE_HASHPT(perturb)) {
  struct membercache_slot *item = MEMBERCACHE_HASHIT(&tp_self->tp_cache,i);
  if (item->mcs_type == MEMBERCACHE_UNUSED) break;
  if (item->mcs_hash != hash) continue;
  if (!ATTREQ(item)) continue;
  switch (item->mcs_type) {
  {
   DeeTypeObject *type;
  case MEMBERCACHE_METHOD:
  case MEMBERCACHE_GETSET:
  case MEMBERCACHE_MEMBER:
   type = item->mcs_decl;
   MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
   return err_cant_access_attribute(type,attr,ATTR_ACCESS_DEL);
  }
  {
   struct class_attribute *catt;
   DeeTypeObject *type;
  case MEMBERCACHE_ATTRIB:
   catt = item->mcs_attrib.a_attr;
   type = item->mcs_decl;
   MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
   return DeeClass_DelInstanceAttribute(type,catt);
  }
  default: __builtin_unreachable();
  }
 }
done:
 MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
 return 1;
}

/* @return:  1: The attribute could not be found in the cache.
 * @return:  0: Successfully invoked the set-operator on the attribute.
 * @return: -1: An error occurred. */
INTERN int DCALL
NLen(DeeType_SetCachedAttr)(DeeTypeObject *__restrict tp_self,
                            DeeObject *__restrict self,
                            ATTR_ARG, dhash_t hash,
                            DeeObject *__restrict value) {
 dhash_t i,perturb;
 MEMBERCACHE_READ(&tp_self->tp_cache);
 if unlikely(!tp_self->tp_cache.mc_table) goto done;
 perturb = i = MEMBERCACHE_HASHST(&tp_self->tp_cache,hash);
 for (;; i = MEMBERCACHE_HASHNX(i,perturb),MEMBERCACHE_HASHPT(perturb)) {
  struct membercache_slot *item = MEMBERCACHE_HASHIT(&tp_self->tp_cache,i);
  if (item->mcs_type == MEMBERCACHE_UNUSED) break;
  if (item->mcs_hash != hash) continue;
  if (!ATTREQ(item)) continue;
  switch (item->mcs_type) {
  {
   DeeTypeObject *type;
  case MEMBERCACHE_METHOD:
   type = item->mcs_decl;
   MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
   return err_cant_access_attribute(type,attr,ATTR_ACCESS_SET);
  }
  {
   dsetmethod_t set;
   DeeTypeObject *type;
  case MEMBERCACHE_GETSET:
   set = item->mcs_getset.gs_set;
   if likely(set) {
    MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
    return (*set)(self,value);
   }
   type = item->mcs_decl;
   MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
   err_cant_access_attribute(type,attr,ATTR_ACCESS_SET);
   goto err;
  }
  {
   struct buffer { uint8_t dat[COMPILER_OFFSETOF(struct type_member,m_doc)]; } buf;
  case MEMBERCACHE_MEMBER:
   buf = *(struct buffer *)&item->mcs_member;
   MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
   return type_member_set((struct type_member *)&buf,self,value);
  }
  {
   struct class_attribute *catt;
   struct class_desc *desc;
  case MEMBERCACHE_ATTRIB:
   catt = item->mcs_attrib.a_attr;
   desc = item->mcs_attrib.a_desc;
   MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
   return DeeInstance_SetAttribute(desc,
                                   DeeInstance_DESC(desc,self),
                                   self,catt,value);
  }
  default: __builtin_unreachable();
  }
 }
done:
 MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
 return 1;
err:
 return -1;
}
INTERN int DCALL
NLen(DeeType_SetCachedClassAttr)(DeeTypeObject *__restrict tp_self,
                                 ATTR_ARG, dhash_t hash,
                                 DeeObject *__restrict value) {
 dhash_t i,perturb;
 MEMBERCACHE_READ(&tp_self->tp_class_cache);
 if unlikely(!tp_self->tp_class_cache.mc_table) goto done;
 perturb = i = MEMBERCACHE_HASHST(&tp_self->tp_class_cache,hash);
 for (;; i = MEMBERCACHE_HASHNX(i,perturb),MEMBERCACHE_HASHPT(perturb)) {
  struct membercache_slot *item = MEMBERCACHE_HASHIT(&tp_self->tp_class_cache,i);
  if (item->mcs_type == MEMBERCACHE_UNUSED) break;
  if (item->mcs_hash != hash) continue;
  if (!ATTREQ(item)) continue;
  switch (item->mcs_type) {
  {
   DeeTypeObject *type;
  case MEMBERCACHE_METHOD:
  case MEMBERCACHE_INSTANCE_METHOD:
  case MEMBERCACHE_INSTANCE_GETSET:
  case MEMBERCACHE_INSTANCE_MEMBER:
   type = item->mcs_decl;
   MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
   return err_cant_access_attribute(type,attr,ATTR_ACCESS_SET);
  }
  {
   dsetmethod_t set;
   DeeTypeObject *type;
  case MEMBERCACHE_GETSET:
   set = item->mcs_getset.gs_set;
   if likely(set) {
    MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
    return (*set)((DeeObject *)tp_self,value);
   }
   type = item->mcs_decl;
   MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
   err_cant_access_attribute(type,attr,ATTR_ACCESS_SET);
   goto err;
  }
  {
   struct buffer { uint8_t dat[COMPILER_OFFSETOF(struct type_member,m_doc)]; } buf;
  case MEMBERCACHE_MEMBER:
   buf = *(struct buffer *)&item->mcs_member;
   MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
   return type_member_set((struct type_member *)&buf,(DeeObject *)tp_self,value);
  }
  {
   struct class_attribute *catt;
   struct class_desc *desc;
  case MEMBERCACHE_ATTRIB:
   catt = item->mcs_attrib.a_attr;
   desc = item->mcs_attrib.a_desc;
   MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
   return DeeInstance_SetAttribute(desc,class_desc_as_instance(desc),(DeeObject *)tp_self,catt,value);
  }
  {
   struct class_attribute *catt;
   DeeTypeObject *type;
  case MEMBERCACHE_INSTANCE_ATTRIB:
   catt = item->mcs_attrib.a_attr;
   type = item->mcs_decl;
   MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
   return DeeClass_SetInstanceAttribute(type,catt,value);
  }
  default: __builtin_unreachable();
  }
 }
done:
 MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
 return 1;
err:
 return -1;
}
INTERN int DCALL
NLen(DeeType_SetCachedInstanceAttr)(DeeTypeObject *__restrict tp_self,
                                    ATTR_ARG, dhash_t hash,
                                    DeeObject *__restrict value) {
 dhash_t i,perturb;
 MEMBERCACHE_READ(&tp_self->tp_cache);
 if unlikely(!tp_self->tp_cache.mc_table) goto done;
 perturb = i = MEMBERCACHE_HASHST(&tp_self->tp_cache,hash);
 for (;; i = MEMBERCACHE_HASHNX(i,perturb),MEMBERCACHE_HASHPT(perturb)) {
  struct membercache_slot *item = MEMBERCACHE_HASHIT(&tp_self->tp_cache,i);
  if (item->mcs_type == MEMBERCACHE_UNUSED) break;
  if (item->mcs_hash != hash) continue;
  if (!ATTREQ(item)) continue;
  switch (item->mcs_type) {
  {
   DeeTypeObject *type;
  case MEMBERCACHE_METHOD:
  case MEMBERCACHE_GETSET:
  case MEMBERCACHE_MEMBER:
   type = item->mcs_decl;
   MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
   return err_cant_access_attribute(type,attr,ATTR_ACCESS_SET);
  }
  {
   struct class_attribute *catt;
   DeeTypeObject *type;
  case MEMBERCACHE_ATTRIB:
   catt = item->mcs_attrib.a_attr;
   type = item->mcs_decl;
   MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
   return DeeClass_SetInstanceAttribute(type,catt,value);
  }
  default: __builtin_unreachable();
  }
 }
done:
 MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
 return 1;
}

/* @return:  2: The attribute is non-basic.
/* @return:  1: The attribute could not be found in the cache.
 * @return:  0: Successfully invoked the set-operator on the attribute.
 * @return: -1: An error occurred. */
INTERN int DCALL
NLen(DeeType_SetBasicCachedAttr)(DeeTypeObject *__restrict tp_self,
                                 DeeObject *__restrict self,
                                 ATTR_ARG, dhash_t hash,
                                 DeeObject *__restrict value) {
 dhash_t i,perturb;
 MEMBERCACHE_READ(&tp_self->tp_cache);
 if unlikely(!tp_self->tp_cache.mc_table) goto done;
 perturb = i = MEMBERCACHE_HASHST(&tp_self->tp_cache,hash);
 for (;; i = MEMBERCACHE_HASHNX(i,perturb),MEMBERCACHE_HASHPT(perturb)) {
  struct membercache_slot *item = MEMBERCACHE_HASHIT(&tp_self->tp_cache,i);
  if (item->mcs_type == MEMBERCACHE_UNUSED) break;
  if (item->mcs_hash != hash) continue;
  if (!ATTREQ(item)) continue;
  switch (item->mcs_type) {
  {
   struct buffer { uint8_t dat[COMPILER_OFFSETOF(struct type_member,m_doc)]; } buf;
  case MEMBERCACHE_MEMBER:
   buf = *(struct buffer *)&item->mcs_member;
   MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
   return type_member_set((struct type_member *)&buf,self,value);
  }
  {
   struct class_attribute *catt;
   struct class_desc *desc;
  case MEMBERCACHE_ATTRIB:
   catt = item->mcs_attrib.a_attr;
   desc = item->mcs_attrib.a_desc;
   MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
   return DeeInstance_SetBasicAttribute(desc,
                                        DeeInstance_DESC(desc,self),
                                        self,catt,value);
  }
  default:
   MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
   return 2;
  }
 }
done:
 MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
 return 1;
}
INTERN int DCALL
NLen(DeeType_SetBasicCachedClassAttr)(DeeTypeObject *__restrict tp_self,
                                      ATTR_ARG, dhash_t hash,
                                      DeeObject *__restrict value) {
 dhash_t i,perturb;
 MEMBERCACHE_READ(&tp_self->tp_class_cache);
 if unlikely(!tp_self->tp_class_cache.mc_table) goto done;
 perturb = i = MEMBERCACHE_HASHST(&tp_self->tp_class_cache,hash);
 for (;; i = MEMBERCACHE_HASHNX(i,perturb),MEMBERCACHE_HASHPT(perturb)) {
  struct membercache_slot *item = MEMBERCACHE_HASHIT(&tp_self->tp_class_cache,i);
  if (item->mcs_type == MEMBERCACHE_UNUSED) break;
  if (item->mcs_hash != hash) continue;
  if (!ATTREQ(item)) continue;
  switch (item->mcs_type) {
  {
   struct buffer { uint8_t dat[COMPILER_OFFSETOF(struct type_member,m_doc)]; } buf;
  case MEMBERCACHE_MEMBER:
   buf = *(struct buffer *)&item->mcs_member;
   MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
   return type_member_set((struct type_member *)&buf,(DeeObject *)tp_self,value);
  }
  {
   struct class_attribute *catt;
   struct class_desc *desc;
  case MEMBERCACHE_ATTRIB:
   catt = item->mcs_attrib.a_attr;
   desc = item->mcs_attrib.a_desc;
   MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
   return DeeInstance_SetBasicAttribute(desc,class_desc_as_instance(desc),(DeeObject *)tp_self,catt,value);
  }
  {
   struct class_attribute *catt;
   DeeTypeObject *type;
  case MEMBERCACHE_INSTANCE_ATTRIB:
   catt = item->mcs_attrib.a_attr;
   type = item->mcs_decl;
   MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
   return DeeClass_SetBasicInstanceAttribute(type,catt,value);
  }
  default:
   MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
   return 2;
  }
 }
done:
 MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
 return 1;
}
#if 0
INTERN int DCALL
NLen(DeeType_SetBasicCachedInstanceAttr)(DeeTypeObject *__restrict tp_self,
                                         ATTR_ARG, dhash_t hash,
                                         DeeObject *__restrict value) {
 dhash_t i,perturb;
 MEMBERCACHE_READ(&tp_self->tp_cache);
 if unlikely(!tp_self->tp_cache.mc_table) goto done;
 perturb = i = MEMBERCACHE_HASHST(&tp_self->tp_cache,hash);
 for (;; i = MEMBERCACHE_HASHNX(i,perturb),MEMBERCACHE_HASHPT(perturb)) {
  struct membercache_slot *item = MEMBERCACHE_HASHIT(&tp_self->tp_cache,i);
  if (item->mcs_type == MEMBERCACHE_UNUSED) break;
  if (item->mcs_hash != hash) continue;
  if (!ATTREQ(item)) continue;
  switch (item->mcs_type) {
  {
   struct class_attribute *catt;
   DeeTypeObject *type;
  case MEMBERCACHE_ATTRIB:
   catt = item->mcs_attrib.a_attr;
   type = item->mcs_decl;
   MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
   return DeeClass_SetBasicInstanceAttribute(type,catt,value);
  }
  default:
   MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
   return 2;
  }
 }
done:
 MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
 return 1;
}
#endif

/* @return: * :        The returned value.
 * @return: NULL:      An error occurred.
 * @return: ITER_DONE: The attribute could not be found in the cache. */
INTERN DREF DeeObject *DCALL
NLen(DeeType_CallCachedAttr)(DeeTypeObject *__restrict tp_self,
                             DeeObject *__restrict self,
                             ATTR_ARG, dhash_t hash,
                             size_t argc, DeeObject **__restrict argv) {
 dhash_t i,perturb; DREF DeeObject *callback,*result;
 MEMBERCACHE_READ(&tp_self->tp_cache);
 if unlikely(!tp_self->tp_cache.mc_table) goto done;
 perturb = i = MEMBERCACHE_HASHST(&tp_self->tp_cache,hash);
 for (;; i = MEMBERCACHE_HASHNX(i,perturb),MEMBERCACHE_HASHPT(perturb)) {
  struct membercache_slot *item = MEMBERCACHE_HASHIT(&tp_self->tp_cache,i);
  if (item->mcs_type == MEMBERCACHE_UNUSED) break;
  if (item->mcs_hash != hash) continue;
  if (!ATTREQ(item)) continue;
  switch (item->mcs_type) {
  {
   dobjmethod_t func;
  case MEMBERCACHE_METHOD:
   func = item->mcs_method.m_func;
   if (item->mcs_method.m_flag & TYPE_METHOD_FKWDS) {
    MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
    return (*(dkwobjmethod_t)func)(self,argc, argv,NULL);
   }
   MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
   return (*func)(self,argc, argv);
  }
  {
   dgetmethod_t getter;
   DeeTypeObject *type;
  case MEMBERCACHE_GETSET:
   getter = item->mcs_getset.gs_get;
   if likely(getter) {
    MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
    callback = (*getter)(self);
check_and_invoke_callback:
    if unlikely(!callback) goto err;
    result = DeeObject_Call(callback,argc, argv);
    Dee_Decref(callback);
    return result;
   }
   type = item->mcs_decl;
   MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
   err_cant_access_attribute(type,attr,ATTR_ACCESS_GET);
   goto err;
  }
  {
   struct buffer { uint8_t dat[COMPILER_OFFSETOF(struct type_member,m_doc)]; } buf;
  case MEMBERCACHE_MEMBER:
   buf = *(struct buffer *)&item->mcs_member;
   MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
   callback = type_member_get((struct type_member *)&buf,self);
   goto check_and_invoke_callback;
  }
  {
   struct class_attribute *catt;
   struct class_desc *desc;
  case MEMBERCACHE_ATTRIB:
   catt = item->mcs_attrib.a_attr;
   desc = item->mcs_attrib.a_desc;
   MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
   return DeeInstance_CallAttribute(desc,
                                    DeeInstance_DESC(desc,self),
                                    self,catt,argc, argv);
  }
  default: __builtin_unreachable();
  }
 }
done:
 MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
 return ITER_DONE;
err:
 return NULL;
}


#ifndef ERR_CLSPROPERTY_DEFINED
#define ERR_CLSPROPERTY_DEFINED 1
PRIVATE ATTR_COLD int DCALL err_cant_access_clsproperty_get(void) {
 return err_cant_access_attribute(&DeeClsProperty_Type,
                                   DeeString_STR(&str_get),
                                   ATTR_ACCESS_GET);
}
#endif /* !ERR_CLSPROPERTY_DEFINED */

PRIVATE ATTR_COLD int DCALL
N_len(err_classmember_requires_1_argument)(ATTR_ARG) {
 return DeeError_Throwf(&DeeError_TypeError,
                        "classmember `%" IF_LEN("$") "s' must be called with exactly 1 argument",
                        IF_LEN(attrlen,) attr);
}
PRIVATE ATTR_COLD int DCALL
N_len(err_classproperty_requires_1_argument)(ATTR_ARG) {
 return DeeError_Throwf(&DeeError_TypeError,
                        "classproperty `%" IF_LEN("$") "s' must be called with exactly 1 argument",
                        IF_LEN(attrlen,) attr);
}
PRIVATE ATTR_COLD int DCALL
N_len(err_classmethod_requires_at_least_1_argument)(ATTR_ARG) {
 return DeeError_Throwf(&DeeError_TypeError,
                        "classmethod `%" IF_LEN("$") "s' must be called with at least 1 argument",
                         IF_LEN(attrlen,) attr);
}

INTERN DREF DeeObject *DCALL
NLen(DeeType_CallCachedClassAttr)(DeeTypeObject *__restrict tp_self,
                                  ATTR_ARG, dhash_t hash,
                                  size_t argc, DeeObject **__restrict argv) {
 dhash_t i,perturb; DREF DeeObject *callback,*result;
 MEMBERCACHE_READ(&tp_self->tp_class_cache);
 if unlikely(!tp_self->tp_class_cache.mc_table) goto done;
 perturb = i = MEMBERCACHE_HASHST(&tp_self->tp_class_cache,hash);
 for (;; i = MEMBERCACHE_HASHNX(i,perturb),MEMBERCACHE_HASHPT(perturb)) {
  struct membercache_slot *item = MEMBERCACHE_HASHIT(&tp_self->tp_class_cache,i);
  if (item->mcs_type == MEMBERCACHE_UNUSED) break;
  if (item->mcs_hash != hash) continue;
  if (!ATTREQ(item)) continue;
  switch (item->mcs_type) {
  {
   dobjmethod_t func;
  case MEMBERCACHE_METHOD:
   func = item->mcs_method.m_func;
   if (item->mcs_method.m_flag & TYPE_METHOD_FKWDS) {
    MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
    return (*(dkwobjmethod_t)func)((DeeObject *)tp_self,argc, argv,NULL);
   }
   MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
   return (*func)((DeeObject *)tp_self,argc, argv);
  }
  {
   dgetmethod_t getter;
   DeeTypeObject *type;
  case MEMBERCACHE_GETSET:
   getter = item->mcs_getset.gs_get;
   if likely(getter) {
    MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
    callback = (*getter)((DeeObject *)tp_self);
check_and_invoke_callback:
    if unlikely(!callback) goto err;
    result = DeeObject_Call(callback,argc, argv);
    Dee_Decref(callback);
    return result;
   }
   type = item->mcs_decl;
   MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
   err_cant_access_attribute(type,attr,ATTR_ACCESS_GET);
   goto err;
  }
  {
   struct buffer { uint8_t dat[COMPILER_OFFSETOF(struct type_member,m_doc)]; } buf;
  case MEMBERCACHE_MEMBER:
   buf = *(struct buffer *)&item->mcs_member;
   MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
   callback = type_member_get((struct type_member *)&buf,(DeeObject *)tp_self);
   goto check_and_invoke_callback;
  }
  {
   struct class_attribute *catt;
   struct class_desc *desc;
  case MEMBERCACHE_ATTRIB:
   catt = item->mcs_attrib.a_attr;
   desc = item->mcs_attrib.a_desc;
   MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
   return DeeInstance_CallAttribute(desc,class_desc_as_instance(desc),(DeeObject *)tp_self,catt,argc, argv);
  }
  {
   dobjmethod_t func;
   DeeTypeObject *type;
  case MEMBERCACHE_INSTANCE_METHOD:
   func = item->mcs_method.m_func;
   type = item->mcs_decl;
   if (item->mcs_method.m_flag & TYPE_METHOD_FKWDS) {
    MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
    if unlikely(!argc) goto err_classmethod_noargs;
    /* Allow non-instance objects for generic types. */
    if unlikely(!(type->tp_flags & TP_FABSTRACT) && DeeObject_AssertType(argv[0],type))
       goto err;
    /* Use the first argument as the this-argument. */
    return (*(dkwobjmethod_t)func)(argv[0],argc-1,argv+1,NULL);
   }
   MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
   if unlikely(!argc) goto err_classmethod_noargs;
   /* Allow non-instance objects for generic types. */
   if unlikely(!(type->tp_flags & TP_FABSTRACT) && DeeObject_AssertType(argv[0],type))
      goto err;
   /* Use the first argument as the this-argument. */
   return (*func)(argv[0],argc-1,argv+1);
  }
  {
   dgetmethod_t get;
   DeeTypeObject *type;
  case MEMBERCACHE_INSTANCE_GETSET:
   get = item->mcs_getset.gs_get;
   type = item->mcs_decl;
   MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
   if unlikely(!get) goto err_no_getter;
   if unlikely(argc != 1) goto err_classproperty_invalid_args;
   /* Allow non-instance objects for generic types. */
   if unlikely(!(type->tp_flags & TP_FABSTRACT) && DeeObject_AssertType(argv[0],type))
      goto err;
   /* Use the first argument as the this-argument. */
   return (*get)(argv[0]);
  }
  {
   struct type_member member;
   DeeTypeObject *type;
  case MEMBERCACHE_INSTANCE_MEMBER:
   member = item->mcs_member;
   type = item->mcs_decl;
   MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
   if unlikely(argc != 1) goto err_classmember_invalid_args;
   /* Allow non-instance objects for generic types. */
   if unlikely(!(type->tp_flags & TP_FABSTRACT) && DeeObject_AssertType(argv[0],type))
      goto err;
   /* Use the first argument as the this-argument. */
   return type_member_get(&member,argv[0]);
  }
  {
   struct class_attribute *catt;
   DeeTypeObject *type;
  case MEMBERCACHE_INSTANCE_ATTRIB:
   catt = item->mcs_attrib.a_attr;
   type = item->mcs_decl;
   MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
   return DeeClass_CallInstanceAttribute(type,catt,argc, argv);
  }
  default: __builtin_unreachable();
  }
 }
done:
 MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
 return ITER_DONE;
err_no_getter:
 err_cant_access_clsproperty_get();
 goto err;
err_classmember_invalid_args:
 N_len(err_classmember_requires_1_argument)(attr IF_LEN(,attrlen));
 goto err;
err_classproperty_invalid_args:
 N_len(err_classproperty_requires_1_argument)(attr IF_LEN(,attrlen));
 goto err;
err_classmethod_noargs:
 N_len(err_classmethod_requires_at_least_1_argument)(attr IF_LEN(,attrlen));
err:
 return NULL;
}

#if 0
INTERN DREF DeeObject *DCALL
NLen(DeeType_CallCachedInstanceAttr)(DeeTypeObject *__restrict tp_self,
                                     ATTR_ARG, dhash_t hash,
                                     size_t argc, DeeObject **__restrict argv) {
 dhash_t i,perturb; DREF DeeObject *callback,*result;
 MEMBERCACHE_READ(&tp_self->tp_cache);
 if unlikely(!tp_self->tp_cache.mc_table) goto done;
 perturb = i = MEMBERCACHE_HASHST(&tp_self->tp_cache,hash);
 for (;; i = MEMBERCACHE_HASHNX(i,perturb),MEMBERCACHE_HASHPT(perturb)) {
  struct membercache_slot *item = MEMBERCACHE_HASHIT(&tp_self->tp_cache,i);
  if (item->mcs_type == MEMBERCACHE_UNUSED) break;
  if (item->mcs_hash != hash) continue;
  if (!ATTREQ(item)) continue;
  switch (item->mcs_type) {
  {
   dobjmethod_t func;
   DeeTypeObject *type;
  case MEMBERCACHE_METHOD:
   func = item->mcs_method.m_func;
   type = item->mcs_decl;
   if (item->mcs_method.m_flag & TYPE_METHOD_FKWDS) {
    MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
    if unlikely(!argc) goto err_classmethod_noargs;
    /* Allow non-instance objects for generic types. */
    if unlikely(!(type->tp_flags & TP_FABSTRACT) && DeeObject_AssertType(argv[0],type))
       goto err;
    /* Use the first argument as the this-argument. */
    return (*(dkwobjmethod_t)func)(argv[0],argc-1,argv+1,NULL);
   }
   MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
   if unlikely(!argc) goto err_classmethod_noargs;
   /* Allow non-instance objects for generic types. */
   if unlikely(!(type->tp_flags & TP_FABSTRACT) && DeeObject_AssertType(argv[0],type))
      goto err;
   /* Use the first argument as the this-argument. */
   return (*func)(argv[0],argc-1,argv+1);
  }
  {
   dgetmethod_t get;
   DeeTypeObject *type;
  case MEMBERCACHE_GETSET:
   get = item->mcs_getset.gs_get;
   type = item->mcs_decl;
   MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
   if unlikely(!get) goto err_no_getter;
   if unlikely(argc != 1) goto err_classproperty_invalid_args;
   /* Allow non-instance objects for generic types. */
   if unlikely(!(type->tp_flags & TP_FABSTRACT) && DeeObject_AssertType(argv[0],type))
      goto err;
   /* Use the first argument as the this-argument. */
   return (*get)(argv[0]);
  }
  {
   struct type_member member;
   DeeTypeObject *type;
  case MEMBERCACHE_MEMBER:
   member = item->mcs_member;
   type = item->mcs_decl;
   MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
   if unlikely(argc != 1) goto err_classmember_invalid_args;
   /* Allow non-instance objects for generic types. */
   if unlikely(!(type->tp_flags & TP_FABSTRACT) && DeeObject_AssertType(argv[0],type))
      goto err;
   /* Use the first argument as the this-argument. */
   return type_member_get(&member,argv[0]);
  }
  {
   struct class_attribute *catt;
   DeeTypeObject *type;
  case MEMBERCACHE_ATTRIB:
   catt = item->mcs_attrib.a_attr;
   type = item->mcs_decl;
   MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
   return DeeClass_CallInstanceAttribute(type,catt,argc, argv);
  }
  default: __builtin_unreachable();
  }
 }
done:
 MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
 return ITER_DONE;
err_no_getter:
 err_cant_access_clsproperty_get();
 goto err;
err_classmember_invalid_args:
 err_classmember_requires_1_argument(attr);
 goto err;
err_classproperty_invalid_args:
 err_classproperty_requires_1_argument(attr);
 goto err;
err_classmethod_noargs:
 err_classmethod_requires_at_least_1_argument(attr);
err:
 return NULL;
}
#endif

/* @return: * :        The returned value.
 * @return: NULL:      An error occurred.
 * @return: ITER_DONE: The attribute could not be found in the cache. */
INTERN DREF DeeObject *DCALL
S(DeeType_CallCachedAttrKw,
  DeeType_CallCachedAttrLenKw)(DeeTypeObject *__restrict tp_self,
                               DeeObject *__restrict self,
                               ATTR_ARG, dhash_t hash,
                               size_t argc, DeeObject **__restrict argv,
                               DeeObject *kw) {
 dhash_t i,perturb; DREF DeeObject *callback,*result;
 MEMBERCACHE_READ(&tp_self->tp_cache);
 if unlikely(!tp_self->tp_cache.mc_table) goto done;
 perturb = i = MEMBERCACHE_HASHST(&tp_self->tp_cache,hash);
 for (;; i = MEMBERCACHE_HASHNX(i,perturb),MEMBERCACHE_HASHPT(perturb)) {
  struct membercache_slot *item = MEMBERCACHE_HASHIT(&tp_self->tp_cache,i);
  if (item->mcs_type == MEMBERCACHE_UNUSED) break;
  if (item->mcs_hash != hash) continue;
  if (!ATTREQ(item)) continue;
  switch (item->mcs_type) {
  {
   dobjmethod_t func;
  case MEMBERCACHE_METHOD:
   func = item->mcs_method.m_func;
   if (item->mcs_method.m_flag & TYPE_METHOD_FKWDS) {
    MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
    return (*(dkwobjmethod_t)func)(self,argc, argv, kw);
   }
   MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
   if (kw) {
    if (DeeKwds_Check(kw)) {
     if (DeeKwds_SIZE(kw) != 0)
         goto err_no_keywords;
    } else {
     size_t temp = DeeObject_Size(kw);
     if unlikely(temp == (size_t)-1) goto err;
     if (temp != 0) goto err_no_keywords;
    }
   }
   return (*func)(self,argc, argv);
  }
  {
   dgetmethod_t getter;
   DeeTypeObject *type;
  case MEMBERCACHE_GETSET:
   getter = item->mcs_getset.gs_get;
   if likely(getter) {
    MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
    callback = (*getter)(self);
check_and_invoke_callback:
    if unlikely(!callback) goto err;
    result = DeeObject_CallKw(callback,argc, argv, kw);
    Dee_Decref(callback);
    return result;
   }
   type = item->mcs_decl;
   MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
   err_cant_access_attribute(type,attr,ATTR_ACCESS_GET);
   goto err;
  }
  {
   struct buffer { uint8_t dat[COMPILER_OFFSETOF(struct type_member,m_doc)]; } buf;
  case MEMBERCACHE_MEMBER:
   buf = *(struct buffer *)&item->mcs_member;
   MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
   callback = type_member_get((struct type_member *)&buf,self);
   goto check_and_invoke_callback;
  }
  {
   struct class_attribute *catt;
   struct class_desc *desc;
  case MEMBERCACHE_ATTRIB:
   catt = item->mcs_attrib.a_attr;
   desc = item->mcs_attrib.a_desc;
   MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
   return DeeInstance_CallAttributeKw(desc,
                                      DeeInstance_DESC(desc,self),
                                      self,catt,argc, argv, kw);
  }
  default: __builtin_unreachable();
  }
 }
done:
 MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
 return ITER_DONE;
err_no_keywords:
 err_keywords_func_not_accepted(attr,kw);
err:
 return NULL;
}

INTDEF struct keyword getter_kwlist[];

INTERN DREF DeeObject *DCALL
S(DeeType_CallCachedClassAttrKw,
  DeeType_CallCachedClassAttrLenKw)(DeeTypeObject *__restrict tp_self,
                                    ATTR_ARG, dhash_t hash,
                                    size_t argc, DeeObject **__restrict argv,
                                    DeeObject *kw) {
 dhash_t i,perturb; DREF DeeObject *callback,*result;
 MEMBERCACHE_READ(&tp_self->tp_class_cache);
 if unlikely(!tp_self->tp_class_cache.mc_table) goto done;
 perturb = i = MEMBERCACHE_HASHST(&tp_self->tp_class_cache,hash);
 for (;; i = MEMBERCACHE_HASHNX(i,perturb),MEMBERCACHE_HASHPT(perturb)) {
  struct membercache_slot *item = MEMBERCACHE_HASHIT(&tp_self->tp_class_cache,i);
  if (item->mcs_type == MEMBERCACHE_UNUSED) break;
  if (item->mcs_hash != hash) continue;
  if (!ATTREQ(item)) continue;
  switch (item->mcs_type) {
  {
   dobjmethod_t func;
  case MEMBERCACHE_METHOD:
   func = item->mcs_method.m_func;
   if (item->mcs_method.m_flag & TYPE_METHOD_FKWDS) {
    MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
    return (*(dkwobjmethod_t)func)((DeeObject *)tp_self,argc, argv, kw);
   }
   MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
   if (kw) {
    if (DeeKwds_Check(kw)) {
     if (DeeKwds_SIZE(kw) != 0)
         goto err_no_keywords;
    } else {
     size_t temp = DeeObject_Size(kw);
     if unlikely(temp == (size_t)-1) goto err;
     if (temp != 0) goto err_no_keywords;
    }
   }
   return (*func)((DeeObject *)tp_self,argc, argv);
  }
  {
   dgetmethod_t getter;
   DeeTypeObject *type;
  case MEMBERCACHE_GETSET:
   getter = item->mcs_getset.gs_get;
   if likely(getter) {
    MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
    callback = (*getter)((DeeObject *)tp_self);
check_and_invoke_callback:
    if unlikely(!callback) goto err;
    result = DeeObject_CallKw(callback,argc, argv, kw);
    Dee_Decref(callback);
    return result;
   }
   type = item->mcs_decl;
   MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
   err_cant_access_attribute(type,attr,ATTR_ACCESS_GET);
   goto err;
  }
  {
   struct buffer { uint8_t dat[COMPILER_OFFSETOF(struct type_member,m_doc)]; } buf;
  case MEMBERCACHE_MEMBER:
   buf = *(struct buffer *)&item->mcs_member;
   MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
   callback = type_member_get((struct type_member *)&buf,(DeeObject *)tp_self);
   goto check_and_invoke_callback;
  }
  {
   struct class_attribute *catt;
   struct class_desc *desc;
  case MEMBERCACHE_ATTRIB:
   catt = item->mcs_attrib.a_attr;
   desc = item->mcs_attrib.a_desc;
   MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
   return DeeInstance_CallAttributeKw(desc,class_desc_as_instance(desc),(DeeObject *)tp_self,catt,argc, argv, kw);
  }
  {
   dobjmethod_t func;
   DeeTypeObject *type;
  case MEMBERCACHE_INSTANCE_METHOD:
   func = item->mcs_method.m_func;
   type = item->mcs_decl;
   if (item->mcs_method.m_flag & TYPE_METHOD_FKWDS) {
    MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
    if unlikely(!argc) goto err_classmethod_noargs;
    /* Allow non-instance objects for generic types. */
    if unlikely(!(type->tp_flags & TP_FABSTRACT) && DeeObject_AssertType(argv[0],type))
       goto err;
    /* Use the first argument as the this-argument. */
    return (*(dkwobjmethod_t)func)(argv[0],argc-1,argv+1,kw);
   }
   MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
   if unlikely(!argc) goto err_classmethod_noargs;
   /* Allow non-instance objects for generic types. */
   if unlikely(!(type->tp_flags & TP_FABSTRACT) && DeeObject_AssertType(argv[0],type))
      goto err;
   if (kw) {
    if (DeeKwds_Check(kw)) {
     if (DeeKwds_SIZE(kw) != 0)
         goto err_no_keywords;
    } else {
     size_t temp = DeeObject_Size(kw);
     if unlikely(temp == (size_t)-1) goto err;
     if (temp != 0) goto err_no_keywords;
    }
   }
   /* Use the first argument as the this-argument. */
   return (*func)(argv[0],argc-1,argv+1);
  }
  {
   dgetmethod_t get;
   DeeTypeObject *type;
   DeeObject *thisarg;
  case MEMBERCACHE_INSTANCE_GETSET:
   get = item->mcs_getset.gs_get;
   type = item->mcs_decl;
   MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
   if unlikely(!get) goto err_no_getter;
   if unlikely(argc != 1) goto err_classproperty_invalid_args;
   /* Allow non-instance objects for generic types. */
   if unlikely(!(type->tp_flags & TP_FABSTRACT) && DeeObject_AssertType(argv[0],type))
      goto err;
   if (DeeArg_UnpackKw(argc, argv, kw,getter_kwlist,"o:get",&thisarg)) goto err;
   return (*get)(thisarg);
  }
  {
   struct type_member member;
   DeeTypeObject *type;
   DeeObject *thisarg;
  case MEMBERCACHE_INSTANCE_MEMBER:
   member = item->mcs_member;
   type = item->mcs_decl;
   MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
   if unlikely(argc != 1) goto err_classmember_invalid_args;
   /* Allow non-instance objects for generic types. */
   if unlikely(!(type->tp_flags & TP_FABSTRACT) && DeeObject_AssertType(argv[0],type))
      goto err;
   if (DeeArg_UnpackKw(argc, argv, kw,getter_kwlist,"o:get",&thisarg)) goto err;
   return type_member_get(&member,thisarg);
  }
  {
   struct class_attribute *catt;
   DeeTypeObject *type;
  case MEMBERCACHE_INSTANCE_ATTRIB:
   catt = item->mcs_attrib.a_attr;
   type = item->mcs_decl;
   MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
   return DeeClass_CallInstanceAttributeKw(type,catt,argc, argv, kw);
  }
  default: __builtin_unreachable();
  }
 }
done:
 MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
 return ITER_DONE;
err_no_getter:
 err_cant_access_clsproperty_get();
 goto err;
err_classmember_invalid_args:
 N_len(err_classmember_requires_1_argument)(attr IF_LEN(,attrlen));
 goto err;
err_classproperty_invalid_args:
 N_len(err_classproperty_requires_1_argument)(attr IF_LEN(,attrlen));
 goto err;
err_classmethod_noargs:
 N_len(err_classmethod_requires_at_least_1_argument)(attr IF_LEN(,attrlen));
 goto err;
err_no_keywords:
 err_keywords_func_not_accepted(attr,kw);
err:
 return NULL;
}

INTERN DREF DeeObject *DCALL
S(DeeType_CallCachedInstanceAttrKw,
  DeeType_CallCachedInstanceAttrLenKw)(DeeTypeObject *__restrict tp_self,
                                       ATTR_ARG, dhash_t hash,
                                       size_t argc, DeeObject **__restrict argv,
                                       DeeObject *kw) {
 dhash_t i,perturb;
 MEMBERCACHE_READ(&tp_self->tp_cache);
 if unlikely(!tp_self->tp_cache.mc_table) goto done;
 perturb = i = MEMBERCACHE_HASHST(&tp_self->tp_cache,hash);
 for (;; i = MEMBERCACHE_HASHNX(i,perturb),MEMBERCACHE_HASHPT(perturb)) {
  struct membercache_slot *item = MEMBERCACHE_HASHIT(&tp_self->tp_cache,i);
  if (item->mcs_type == MEMBERCACHE_UNUSED) break;
  if (item->mcs_hash != hash) continue;
  if (!ATTREQ(item)) continue;
  switch (item->mcs_type) {
  {
   dobjmethod_t func;
   DeeTypeObject *type;
  case MEMBERCACHE_METHOD:
   func = item->mcs_method.m_func;
   type = item->mcs_decl;
   if (item->mcs_method.m_flag & TYPE_METHOD_FKWDS) {
    MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
    if unlikely(!argc) goto err_classmethod_noargs;
    /* Allow non-instance objects for generic types. */
    if unlikely(!(type->tp_flags & TP_FABSTRACT) && DeeObject_AssertType(argv[0],type))
       goto err;
    /* Use the first argument as the this-argument. */
    return (*(dkwobjmethod_t)func)(argv[0],argc-1,argv+1,kw);
   }
   MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
   if unlikely(!argc) goto err_classmethod_noargs;
   /* Allow non-instance objects for generic types. */
   if unlikely(!(type->tp_flags & TP_FABSTRACT) && DeeObject_AssertType(argv[0],type))
      goto err;
   if (kw) {
    if (DeeKwds_Check(kw)) {
     if (DeeKwds_SIZE(kw) != 0)
         goto err_no_keywords;
    } else {
     size_t temp = DeeObject_Size(kw);
     if unlikely(temp == (size_t)-1) goto err;
     if (temp != 0) goto err_no_keywords;
    }
   }
   /* Use the first argument as the this-argument. */
   return (*func)(argv[0],argc-1,argv+1);
  }
  {
   dgetmethod_t get;
   DeeTypeObject *type;
   DeeObject *thisarg;
  case MEMBERCACHE_GETSET:
   get = item->mcs_getset.gs_get;
   type = item->mcs_decl;
   MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
   if unlikely(!get) goto err_no_getter;
   if unlikely(argc != 1) goto err_classproperty_invalid_args;
   /* Allow non-instance objects for generic types. */
   if unlikely(!(type->tp_flags & TP_FABSTRACT) && DeeObject_AssertType(argv[0],type))
      goto err;
   if (DeeArg_UnpackKw(argc, argv, kw,getter_kwlist,"o:get",&thisarg)) goto err;
   return (*get)(thisarg);
  }
  {
   struct type_member member;
   DeeTypeObject *type;
   DeeObject *thisarg;
  case MEMBERCACHE_MEMBER:
   member = item->mcs_member;
   type = item->mcs_decl;
   MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
   if unlikely(argc != 1) goto err_classmember_invalid_args;
   /* Allow non-instance objects for generic types. */
   if unlikely(!(type->tp_flags & TP_FABSTRACT) && DeeObject_AssertType(argv[0],type))
      goto err;
   if (DeeArg_UnpackKw(argc, argv, kw,getter_kwlist,"o:get",&thisarg)) goto err;
   return type_member_get(&member,thisarg);
  }
  {
   struct class_attribute *catt;
   DeeTypeObject *type;
  case MEMBERCACHE_ATTRIB:
   catt = item->mcs_attrib.a_attr;
   type = item->mcs_decl;
   MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
   return DeeClass_CallInstanceAttributeKw(type,catt,argc, argv, kw);
  }
  default: __builtin_unreachable();
  }
 }
done:
 MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
 return ITER_DONE;
err_no_getter:
 err_cant_access_clsproperty_get();
 goto err;
err_classmember_invalid_args:
 N_len(err_classmember_requires_1_argument)(attr IF_LEN(,attrlen));
 goto err;
err_classproperty_invalid_args:
 N_len(err_classproperty_requires_1_argument)(attr IF_LEN(,attrlen));
 goto err;
err_classmethod_noargs:
 N_len(err_classmethod_requires_at_least_1_argument)(attr IF_LEN(,attrlen));
 goto err;
err_no_keywords:
 err_keywords_func_not_accepted(attr,kw);
err:
 return NULL;
}

#ifndef MRO_LEN
#ifdef CONFIG_HAVE_CALLTUPLE_OPTIMIZATIONS
/* @return: * :        The returned value.
 * @return: NULL:      An error occurred.
 * @return: ITER_DONE: The attribute could not be found in the cache. */
INTERN DREF DeeObject *DCALL
S(DeeType_CallCachedAttrTuple,
  DeeType_CallCachedAttrLenTuple)(DeeTypeObject *__restrict tp_self,
                                  DeeObject *__restrict self,
                                  ATTR_ARG, dhash_t hash,
                                  DeeObject *__restrict args) {
 dhash_t i,perturb; DREF DeeObject *callback,*result;
 MEMBERCACHE_READ(&tp_self->tp_cache);
 if unlikely(!tp_self->tp_cache.mc_table) goto done;
 perturb = i = MEMBERCACHE_HASHST(&tp_self->tp_cache,hash);
 for (;; i = MEMBERCACHE_HASHNX(i,perturb),MEMBERCACHE_HASHPT(perturb)) {
  struct membercache_slot *item = MEMBERCACHE_HASHIT(&tp_self->tp_cache,i);
  if (item->mcs_type == MEMBERCACHE_UNUSED) break;
  if (item->mcs_hash != hash) continue;
  if (!ATTREQ(item)) continue;
  switch (item->mcs_type) {
  {
   dobjmethod_t func;
  case MEMBERCACHE_METHOD:
   func = item->mcs_method.m_func;
   if (item->mcs_method.m_flag & TYPE_METHOD_FKWDS) {
    MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
    return (*(dkwobjmethod_t)func)(self,DeeTuple_SIZE(args),DeeTuple_ELEM(args),NULL);
   }
   MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
   return (*func)(self,DeeTuple_SIZE(args),DeeTuple_ELEM(args));
  }
  {
   dgetmethod_t getter;
   DeeTypeObject *type;
  case MEMBERCACHE_GETSET:
   getter = item->mcs_getset.gs_get;
   if likely(getter) {
    MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
    callback = (*getter)(self);
check_and_invoke_callback:
    if unlikely(!callback) goto err;
    result = DeeObject_Call(callback,DeeTuple_SIZE(args),DeeTuple_ELEM(args));
    Dee_Decref(callback);
    return result;
   }
   type = item->mcs_decl;
   MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
   err_cant_access_attribute(type,attr,ATTR_ACCESS_GET);
   goto err;
  }
  {
   struct buffer { uint8_t dat[COMPILER_OFFSETOF(struct type_member,m_doc)]; } buf;
  case MEMBERCACHE_MEMBER:
   buf = *(struct buffer *)&item->mcs_member;
   MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
   callback = type_member_get((struct type_member *)&buf,self);
   goto check_and_invoke_callback;
  }
  {
   struct class_attribute *catt;
   struct class_desc *desc;
  case MEMBERCACHE_ATTRIB:
   catt = item->mcs_attrib.a_attr;
   desc = item->mcs_attrib.a_desc;
   MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
   return DeeInstance_CallAttributeTuple(desc,
                                         DeeInstance_DESC(desc,self),
                                         self,catt,args);
  }
  default: __builtin_unreachable();
  }
 }
done:
 MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
 return ITER_DONE;
err:
 return NULL;
}

INTERN DREF DeeObject *DCALL
S(DeeType_CallCachedClassAttrTuple,
  DeeType_CallCachedClassAttrLenTuple)(DeeTypeObject *__restrict tp_self,
                                       ATTR_ARG, dhash_t hash,
                                       DeeObject *__restrict args) {
 dhash_t i,perturb; DREF DeeObject *callback,*result;
 MEMBERCACHE_READ(&tp_self->tp_class_cache);
 if unlikely(!tp_self->tp_class_cache.mc_table) goto done;
 perturb = i = MEMBERCACHE_HASHST(&tp_self->tp_class_cache,hash);
 for (;; i = MEMBERCACHE_HASHNX(i,perturb),MEMBERCACHE_HASHPT(perturb)) {
  struct membercache_slot *item = MEMBERCACHE_HASHIT(&tp_self->tp_class_cache,i);
  if (item->mcs_type == MEMBERCACHE_UNUSED) break;
  if (item->mcs_hash != hash) continue;
  if (!ATTREQ(item)) continue;
  switch (item->mcs_type) {
  {
   dobjmethod_t func;
  case MEMBERCACHE_METHOD:
   func = item->mcs_method.m_func;
   if (item->mcs_method.m_flag & TYPE_METHOD_FKWDS) {
    MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
    return (*(dkwobjmethod_t)func)((DeeObject *)tp_self,DeeTuple_SIZE(args),DeeTuple_ELEM(args),NULL);
   }
   MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
   return (*func)((DeeObject *)tp_self,DeeTuple_SIZE(args),DeeTuple_ELEM(args));
  }
  {
   dgetmethod_t getter;
   DeeTypeObject *type;
  case MEMBERCACHE_GETSET:
   getter = item->mcs_getset.gs_get;
   if likely(getter) {
    MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
    callback = (*getter)((DeeObject *)tp_self);
check_and_invoke_callback:
    if unlikely(!callback) goto err;
    result = DeeObject_Call(callback,DeeTuple_SIZE(args),DeeTuple_ELEM(args));
    Dee_Decref(callback);
    return result;
   }
   type = item->mcs_decl;
   MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
   err_cant_access_attribute(type,attr,ATTR_ACCESS_GET);
   goto err;
  }
  {
   struct buffer { uint8_t dat[COMPILER_OFFSETOF(struct type_member,m_doc)]; } buf;
  case MEMBERCACHE_MEMBER:
   buf = *(struct buffer *)&item->mcs_member;
   MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
   callback = type_member_get((struct type_member *)&buf,(DeeObject *)tp_self);
   goto check_and_invoke_callback;
  }
  {
   struct class_attribute *catt;
   struct class_desc *desc;
  case MEMBERCACHE_ATTRIB:
   catt = item->mcs_attrib.a_attr;
   desc = item->mcs_attrib.a_desc;
   MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
   return DeeInstance_CallAttributeTuple(desc,class_desc_as_instance(desc),
                                        (DeeObject *)tp_self,catt,args);
  }
  {
   dobjmethod_t func;
   DeeTypeObject *type;
  case MEMBERCACHE_INSTANCE_METHOD:
   func = item->mcs_method.m_func;
   type = item->mcs_decl;
   if (item->mcs_method.m_flag & TYPE_METHOD_FKWDS) {
    MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
    if unlikely(!DeeTuple_SIZE(args)) goto err_classmethod_noargs;
    /* Allow non-instance objects for generic types. */
    if unlikely(!(type->tp_flags & TP_FABSTRACT) && DeeObject_AssertType(DeeTuple_GET(args,0),type))
       goto err;
    /* Use the first argument as the this-argument. */
    return (*(dkwobjmethod_t)func)(DeeTuple_GET(args,0),DeeTuple_SIZE(args)-1,DeeTuple_ELEM(args)+1,NULL);
   }
   MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
   if unlikely(!DeeTuple_SIZE(args)) goto err_classmethod_noargs;
   /* Allow non-instance objects for generic types. */
   if unlikely(!(type->tp_flags & TP_FABSTRACT) && DeeObject_AssertType(DeeTuple_GET(args,0),type))
      goto err;
   /* Use the first argument as the this-argument. */
   return (*func)(DeeTuple_GET(args,0),DeeTuple_SIZE(args)-1,DeeTuple_ELEM(args)+1);
  }
  {
   dgetmethod_t get;
   DeeTypeObject *type;
  case MEMBERCACHE_INSTANCE_GETSET:
   get = item->mcs_getset.gs_get;
   type = item->mcs_decl;
   MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
   if unlikely(!get) goto err_no_getter;
   if unlikely(DeeTuple_SIZE(args) != 1) goto err_classproperty_invalid_args;
   /* Allow non-instance objects for generic types. */
   if unlikely(!(type->tp_flags & TP_FABSTRACT) && DeeObject_AssertType(DeeTuple_GET(args,0),type))
      goto err;
   /* Use the first argument as the this-argument. */
   return (*get)(DeeTuple_GET(args,0));
  }
  {
   struct type_member member;
   DeeTypeObject *type;
  case MEMBERCACHE_INSTANCE_MEMBER:
   member = item->mcs_member;
   type = item->mcs_decl;
   MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
   if unlikely(DeeTuple_SIZE(args) != 1) goto err_classmember_invalid_args;
   /* Allow non-instance objects for generic types. */
   if unlikely(!(type->tp_flags & TP_FABSTRACT) && DeeObject_AssertType(DeeTuple_GET(args,0),type))
      goto err;
   /* Use the first argument as the this-argument. */
   return type_member_get(&member,DeeTuple_GET(args,0));
  }
  {
   struct class_attribute *catt;
   DeeTypeObject *type;
  case MEMBERCACHE_INSTANCE_ATTRIB:
   catt = item->mcs_attrib.a_attr;
   type = item->mcs_decl;
   MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
   return DeeClass_CallInstanceAttributeTuple(type,catt,args);
  }
  default: __builtin_unreachable();
  }
 }
done:
 MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
 return ITER_DONE;
err_no_getter:
 err_cant_access_clsproperty_get();
 goto err;
err_classmember_invalid_args:
 err_classmember_requires_1_argument(attr);
 goto err;
err_classproperty_invalid_args:
 err_classproperty_requires_1_argument(attr);
 goto err;
err_classmethod_noargs:
 err_classmethod_requires_at_least_1_argument(attr);
err:
 return NULL;
}
#if 0
INTERN DREF DeeObject *DCALL
S(DeeType_CallCachedInstanceAttrTuple,
  DeeType_CallCachedInstanceAttrLenTuple)(DeeTypeObject *__restrict tp_self,
                                          ATTR_ARG, dhash_t hash,
                                          DeeObject *__restrict args) {
 dhash_t i,perturb; DREF DeeObject *callback,*result;
 MEMBERCACHE_READ(&tp_self->tp_cache);
 if unlikely(!tp_self->tp_cache.mc_table) goto done;
 perturb = i = MEMBERCACHE_HASHST(&tp_self->tp_cache,hash);
 for (;; i = MEMBERCACHE_HASHNX(i,perturb),MEMBERCACHE_HASHPT(perturb)) {
  struct membercache_slot *item = MEMBERCACHE_HASHIT(&tp_self->tp_cache,i);
  if (item->mcs_type == MEMBERCACHE_UNUSED) break;
  if (item->mcs_hash != hash) continue;
  if (!ATTREQ(item)) continue;
  switch (item->mcs_type) {
  {
   dobjmethod_t func;
   DeeTypeObject *type;
  case MEMBERCACHE_METHOD:
   func = item->mcs_method.m_func;
   type = item->mcs_decl;
   if (item->mcs_method.m_flag & TYPE_METHOD_FKWDS) {
    MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
    if unlikely(!DeeTuple_SIZE(args)) goto err_classmethod_noargs;
    /* Allow non-instance objects for generic types. */
    if unlikely(!(type->tp_flags & TP_FABSTRACT) && DeeObject_AssertType(DeeTuple_GET(args,0),type))
       goto err;
    /* Use the first argument as the this-argument. */
    return (*(dkwobjmethod_t)func)(DeeTuple_GET(args,0),DeeTuple_SIZE(args)-1,DeeTuple_ELEM(args)+1,NULL);
   }
   MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
   if unlikely(!DeeTuple_SIZE(args)) goto err_classmethod_noargs;
   /* Allow non-instance objects for generic types. */
   if unlikely(!(type->tp_flags & TP_FABSTRACT) && DeeObject_AssertType(DeeTuple_GET(args,0),type))
      goto err;
   /* Use the first argument as the this-argument. */
   return (*func)(DeeTuple_GET(args,0),DeeTuple_SIZE(args)-1,DeeTuple_ELEM(args)+1);
  }
  {
   dgetmethod_t get;
   DeeTypeObject *type;
  case MEMBERCACHE_GETSET:
   get = item->mcs_getset.gs_get;
   type = item->mcs_decl;
   MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
   if unlikely(!get) goto err_no_getter;
   if unlikely(DeeTuple_SIZE(args) != 1) goto err_classproperty_invalid_args;
   /* Allow non-instance objects for generic types. */
   if unlikely(!(type->tp_flags & TP_FABSTRACT) && DeeObject_AssertType(DeeTuple_GET(args,0),type))
      goto err;
   /* Use the first argument as the this-argument. */
   return (*get)(DeeTuple_GET(args,0));
  }
  {
   struct type_member member;
   DeeTypeObject *type;
  case MEMBERCACHE_MEMBER:
   member = item->mcs_member;
   type = item->mcs_decl;
   MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
   if unlikely(DeeTuple_SIZE(args) != 1) goto err_classmember_invalid_args;
   /* Allow non-instance objects for generic types. */
   if unlikely(!(type->tp_flags & TP_FABSTRACT) && DeeObject_AssertType(DeeTuple_GET(args,0),type))
      goto err;
   /* Use the first argument as the this-argument. */
   return type_member_get(&member,DeeTuple_GET(args,0));
  }
  {
   struct class_attribute *catt;
   DeeTypeObject *type;
  case MEMBERCACHE_ATTRIB:
   catt = item->mcs_attrib.a_attr;
   type = item->mcs_decl;
   MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
   return DeeClass_CallInstanceAttributeTuple(type,catt,args);
  }
  default: __builtin_unreachable();
  }
 }
done:
 MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
 return ITER_DONE;
err_no_getter:
 err_cant_access_clsproperty_get();
 goto err;
err_classmember_invalid_args:
 err_classmember_requires_1_argument(attr);
 goto err;
err_classproperty_invalid_args:
 err_classproperty_requires_1_argument(attr);
 goto err;
err_classmethod_noargs:
 err_classmethod_requires_at_least_1_argument(attr);
err:
 return NULL;
}
#endif
INTERN DREF DeeObject *DCALL
S(DeeType_CallCachedAttrTupleKw,
  DeeType_CallCachedAttrLenTupleKw)(DeeTypeObject *__restrict tp_self,
                                    DeeObject *__restrict self,
                                    ATTR_ARG, dhash_t hash,
                                    DeeObject *__restrict args, DeeObject *kw) {
 dhash_t i,perturb; DREF DeeObject *callback,*result;
 MEMBERCACHE_READ(&tp_self->tp_cache);
 if unlikely(!tp_self->tp_cache.mc_table) goto done;
 perturb = i = MEMBERCACHE_HASHST(&tp_self->tp_cache,hash);
 for (;; i = MEMBERCACHE_HASHNX(i,perturb),MEMBERCACHE_HASHPT(perturb)) {
  struct membercache_slot *item = MEMBERCACHE_HASHIT(&tp_self->tp_cache,i);
  if (item->mcs_type == MEMBERCACHE_UNUSED) break;
  if (item->mcs_hash != hash) continue;
  if (!ATTREQ(item)) continue;
  switch (item->mcs_type) {
  {
   dobjmethod_t func;
  case MEMBERCACHE_METHOD:
   func = item->mcs_method.m_func;
   if (item->mcs_method.m_flag & TYPE_METHOD_FKWDS) {
    MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
    return (*(dkwobjmethod_t)func)(self,DeeTuple_SIZE(args),DeeTuple_ELEM(args),kw);
   }
   MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
   if (kw) {
    if (DeeKwds_Check(kw)) {
     if (DeeKwds_SIZE(kw) != 0)
         goto err_no_keywords;
    } else {
     size_t temp = DeeObject_Size(kw);
     if unlikely(temp == (size_t)-1) goto err;
     if (temp != 0) goto err_no_keywords;
    }
   }
   return (*func)(self,DeeTuple_SIZE(args),DeeTuple_ELEM(args));
  }
  {
   dgetmethod_t getter;
   DeeTypeObject *type;
  case MEMBERCACHE_GETSET:
   getter = item->mcs_getset.gs_get;
   if likely(getter) {
    MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
    callback = (*getter)(self);
check_and_invoke_callback:
    if unlikely(!callback) goto err;
    result = DeeObject_CallTupleKw(callback,args,kw);
    Dee_Decref(callback);
    return result;
   }
   type = item->mcs_decl;
   MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
   err_cant_access_attribute(type,attr,ATTR_ACCESS_GET);
   goto err;
  }
  {
   struct buffer { uint8_t dat[COMPILER_OFFSETOF(struct type_member,m_doc)]; } buf;
  case MEMBERCACHE_MEMBER:
   buf = *(struct buffer *)&item->mcs_member;
   MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
   callback = type_member_get((struct type_member *)&buf,self);
   goto check_and_invoke_callback;
  }
  {
   struct class_attribute *catt;
   struct class_desc *desc;
  case MEMBERCACHE_ATTRIB:
   catt = item->mcs_attrib.a_attr;
   desc = item->mcs_attrib.a_desc;
   MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
   return DeeInstance_CallAttributeTupleKw(desc,
                                           DeeInstance_DESC(desc,self),
                                           self,catt,args,kw);
  }
  default: __builtin_unreachable();
  }
 }
done:
 MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
 return ITER_DONE;
err_no_keywords:
 err_keywords_func_not_accepted(attr,kw);
err:
 return NULL;
}

INTERN DREF DeeObject *DCALL
S(DeeType_CallCachedClassAttrTupleKw,
  DeeType_CallCachedClassAttrLenTupleKw)(DeeTypeObject *__restrict tp_self,
                                         ATTR_ARG, dhash_t hash,
                                         DeeObject *__restrict args, DeeObject *kw) {
 dhash_t i,perturb; DREF DeeObject *callback,*result;
 MEMBERCACHE_READ(&tp_self->tp_class_cache);
 if unlikely(!tp_self->tp_class_cache.mc_table) goto done;
 perturb = i = MEMBERCACHE_HASHST(&tp_self->tp_class_cache,hash);
 for (;; i = MEMBERCACHE_HASHNX(i,perturb),MEMBERCACHE_HASHPT(perturb)) {
  struct membercache_slot *item = MEMBERCACHE_HASHIT(&tp_self->tp_class_cache,i);
  if (item->mcs_type == MEMBERCACHE_UNUSED) break;
  if (item->mcs_hash != hash) continue;
  if (!ATTREQ(item)) continue;
  switch (item->mcs_type) {
  {
   dobjmethod_t func;
  case MEMBERCACHE_METHOD:
   func = item->mcs_method.m_func;
   if (item->mcs_method.m_flag & TYPE_METHOD_FKWDS) {
    MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
    return (*(dkwobjmethod_t)func)((DeeObject *)tp_self,DeeTuple_SIZE(args),DeeTuple_ELEM(args),kw);
   }
   MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
   if (kw) {
    if (DeeKwds_Check(kw)) {
     if (DeeKwds_SIZE(kw) != 0)
         goto err_no_keywords;
    } else {
     size_t temp = DeeObject_Size(kw);
     if unlikely(temp == (size_t)-1) goto err;
     if (temp != 0) goto err_no_keywords;
    }
   }
   return (*func)((DeeObject *)tp_self,DeeTuple_SIZE(args),DeeTuple_ELEM(args));
  }
  {
   dgetmethod_t getter;
   DeeTypeObject *type;
  case MEMBERCACHE_GETSET:
   getter = item->mcs_getset.gs_get;
   if likely(getter) {
    MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
    callback = (*getter)((DeeObject *)tp_self);
check_and_invoke_callback:
    if unlikely(!callback) goto err;
    result = DeeObject_CallTupleKw(callback,args,kw);
    Dee_Decref(callback);
    return result;
   }
   type = item->mcs_decl;
   MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
   err_cant_access_attribute(type,attr,ATTR_ACCESS_GET);
   goto err;
  }
  {
   struct buffer { uint8_t dat[COMPILER_OFFSETOF(struct type_member,m_doc)]; } buf;
  case MEMBERCACHE_MEMBER:
   buf = *(struct buffer *)&item->mcs_member;
   MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
   callback = type_member_get((struct type_member *)&buf,(DeeObject *)tp_self);
   goto check_and_invoke_callback;
  }
  {
   struct class_attribute *catt;
   struct class_desc *desc;
  case MEMBERCACHE_ATTRIB:
   catt = item->mcs_attrib.a_attr;
   desc = item->mcs_attrib.a_desc;
   MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
   return DeeInstance_CallAttributeTupleKw(desc,class_desc_as_instance(desc),(DeeObject *)tp_self,catt,args,kw);
  }
  {
   dobjmethod_t func;
   DeeTypeObject *type;
  case MEMBERCACHE_INSTANCE_METHOD:
   func = item->mcs_method.m_func;
   type = item->mcs_decl;
   if (item->mcs_method.m_flag & TYPE_METHOD_FKWDS) {
    MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
    if unlikely(!DeeTuple_SIZE(args)) goto err_classmethod_noargs;
    /* Allow non-instance objects for generic types. */
    if unlikely(!(type->tp_flags & TP_FABSTRACT) && DeeObject_AssertType(DeeTuple_GET(args,0),type))
       goto err;
    /* Use the first argument as the this-argument. */
    return (*(dkwobjmethod_t)func)(DeeTuple_GET(args,0),
                                   DeeTuple_SIZE(args) - 1,
                                   DeeTuple_ELEM(args) + 1,
                                   kw);
   }
   MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
   if unlikely(!DeeTuple_SIZE(args)) goto err_classmethod_noargs;
   /* Allow non-instance objects for generic types. */
   if unlikely(!(type->tp_flags & TP_FABSTRACT) && DeeObject_AssertType(DeeTuple_GET(args,0),type))
      goto err;
   if (kw) {
    if (DeeKwds_Check(kw)) {
     if (DeeKwds_SIZE(kw) != 0)
         goto err_no_keywords;
    } else {
     size_t temp = DeeObject_Size(kw);
     if unlikely(temp == (size_t)-1) goto err;
     if (temp != 0) goto err_no_keywords;
    }
   }
   /* Use the first argument as the this-argument. */
   return (*func)(DeeTuple_GET(args,0),
                  DeeTuple_SIZE(args) - 1,
                  DeeTuple_ELEM(args) + 1);
  }
  {
   dgetmethod_t get;
   DeeTypeObject *type;
   DeeObject *thisarg;
  case MEMBERCACHE_INSTANCE_GETSET:
   get = item->mcs_getset.gs_get;
   type = item->mcs_decl;
   MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
   if unlikely(!get) goto err_no_getter;
   if unlikely(DeeTuple_SIZE(args) != 1) goto err_classproperty_invalid_args;
   /* Allow non-instance objects for generic types. */
   if unlikely(!(type->tp_flags & TP_FABSTRACT) && DeeObject_AssertType(DeeTuple_GET(args,0),type))
      goto err;
   if (DeeArg_UnpackKw(DeeTuple_SIZE(args),DeeTuple_ELEM(args),kw,getter_kwlist,"o:get",&thisarg)) goto err;
   return (*get)(thisarg);
  }
  {
   struct type_member member;
   DeeTypeObject *type;
   DeeObject *thisarg;
  case MEMBERCACHE_INSTANCE_MEMBER:
   member = item->mcs_member;
   type = item->mcs_decl;
   MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
   if unlikely(DeeTuple_SIZE(args) != 1) goto err_classmember_invalid_args;
   /* Allow non-instance objects for generic types. */
   if unlikely(!(type->tp_flags & TP_FABSTRACT) && DeeObject_AssertType(DeeTuple_GET(args,0),type))
      goto err;
   if (DeeArg_UnpackKw(DeeTuple_SIZE(args),DeeTuple_ELEM(args),kw,getter_kwlist,"o:get",&thisarg)) goto err;
   return type_member_get(&member,thisarg);
  }
  {
   struct class_attribute *catt;
   DeeTypeObject *type;
  case MEMBERCACHE_INSTANCE_ATTRIB:
   catt = item->mcs_attrib.a_attr;
   type = item->mcs_decl;
   MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
   return DeeClass_CallInstanceAttributeTupleKw(type,catt,args,kw);
  }
  default: __builtin_unreachable();
  }
 }
done:
 MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
 return ITER_DONE;
err_no_getter:
 err_cant_access_clsproperty_get();
 goto err;
err_classmember_invalid_args:
 err_classmember_requires_1_argument(attr);
 goto err;
err_classproperty_invalid_args:
 err_classproperty_requires_1_argument(attr);
 goto err;
err_classmethod_noargs:
 err_classmethod_requires_at_least_1_argument(attr);
 goto err;
err_no_keywords:
 err_keywords_func_not_accepted(attr,kw);
err:
 return NULL;
}
#if 0
INTERN DREF DeeObject *DCALL
S(DeeType_CallCachedInstanceAttrTupleKw,
  DeeType_CallCachedInstanceAttrLenTupleKw)(DeeTypeObject *__restrict tp_self,
                                            ATTR_ARG, dhash_t hash,
                                            DeeObject *__restrict args, DeeObject *kw) {
 dhash_t i,perturb; DREF DeeObject *callback,*result;
 MEMBERCACHE_READ(&tp_self->tp_cache);
 if unlikely(!tp_self->tp_cache.mc_table) goto done;
 perturb = i = MEMBERCACHE_HASHST(&tp_self->tp_cache,hash);
 for (;; i = MEMBERCACHE_HASHNX(i,perturb),MEMBERCACHE_HASHPT(perturb)) {
  struct membercache_slot *item = MEMBERCACHE_HASHIT(&tp_self->tp_cache,i);
  if (item->mcs_type == MEMBERCACHE_UNUSED) break;
  if (item->mcs_hash != hash) continue;
  if (!ATTREQ(item)) continue;
  switch (item->mcs_type) {
  {
   dobjmethod_t func;
   DeeTypeObject *type;
  case MEMBERCACHE_METHOD:
   func = item->mcs_method.m_func;
   type = item->mcs_decl;
   if (item->mcs_method.m_flag & TYPE_METHOD_FKWDS) {
    MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
    if unlikely(!DeeTuple_SIZE(args)) goto err_classmethod_noargs;
    /* Allow non-instance objects for generic types. */
    if unlikely(!(type->tp_flags & TP_FABSTRACT) && DeeObject_AssertType(DeeTuple_GET(args,0),type))
       goto err;
    /* Use the first argument as the this-argument. */
    return (*(dkwobjmethod_t)func)(DeeTuple_GET(args,0),
                                   DeeTuple_SIZE(args) - 1,
                                   DeeTuple_ELEM(args) + 1,
                                   kw);
   }
   MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
   if unlikely(!DeeTuple_SIZE(args)) goto err_classmethod_noargs;
   /* Allow non-instance objects for generic types. */
   if unlikely(!(type->tp_flags & TP_FABSTRACT) && DeeObject_AssertType(DeeTuple_GET(args,0),type))
      goto err;
   if (kw) {
    if (DeeKwds_Check(kw)) {
     if (DeeKwds_SIZE(kw) != 0)
         goto err_no_keywords;
    } else {
     size_t temp = DeeObject_Size(kw);
     if unlikely(temp == (size_t)-1) goto err;
     if (temp != 0) goto err_no_keywords;
    }
   }
   /* Use the first argument as the this-argument. */
   return (*func)(DeeTuple_GET(args,0),
                  DeeTuple_SIZE(args) - 1,
                  DeeTuple_ELEM(args) + 1);
  }
  {
   dgetmethod_t get;
   DeeTypeObject *type;
   DeeObject *thisarg;
  case MEMBERCACHE_GETSET:
   get = item->mcs_getset.gs_get;
   type = item->mcs_decl;
   MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
   if unlikely(!get) goto err_no_getter;
   if unlikely(DeeTuple_SIZE(args) != 1) goto err_classproperty_invalid_args;
   /* Allow non-instance objects for generic types. */
   if unlikely(!(type->tp_flags & TP_FABSTRACT) && DeeObject_AssertType(DeeTuple_GET(args,0),type))
      goto err;
   if (DeeArg_UnpackKw(DeeTuple_SIZE(args),DeeTuple_ELEM(args),kw,getter_kwlist,"o:get",&thisarg)) goto err;
   return (*get)(thisarg);
  }
  {
   struct type_member member;
   DeeTypeObject *type;
   DeeObject *thisarg;
  case MEMBERCACHE_MEMBER:
   member = item->mcs_member;
   type = item->mcs_decl;
   MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
   if unlikely(DeeTuple_SIZE(args) != 1) goto err_classmember_invalid_args;
   /* Allow non-instance objects for generic types. */
   if unlikely(!(type->tp_flags & TP_FABSTRACT) && DeeObject_AssertType(DeeTuple_GET(args,0),type))
      goto err;
   if (DeeArg_UnpackKw(DeeTuple_SIZE(args),DeeTuple_ELEM(args),kw,getter_kwlist,"o:get",&thisarg)) goto err;
   return type_member_get(&member,thisarg);
  }
  {
   struct class_attribute *catt;
   DeeTypeObject *type;
  case MEMBERCACHE_ATTRIB:
   catt = item->mcs_attrib.a_attr;
   type = item->mcs_decl;
   MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
   return DeeClass_CallInstanceAttributeTupleKw(type,catt,args,kw);
  }
  default: __builtin_unreachable();
  }
 }
done:
 MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
 return ITER_DONE;
err_no_getter:
 err_cant_access_clsproperty_get();
 goto err;
err_classmember_invalid_args:
 err_classmember_requires_1_argument(attr);
 goto err;
err_classproperty_invalid_args:
 err_classproperty_requires_1_argument(attr);
 goto err;
err_classmethod_noargs:
 err_classmethod_requires_at_least_1_argument(attr);
 goto err;
err_no_keywords:
 err_keywords_func_not_accepted(attr,kw);
err:
 return NULL;
}
#endif
#endif /* CONFIG_HAVE_CALLTUPLE_OPTIMIZATIONS */
#endif /* !MRO_LEN */


#ifndef MRO_LEN
#ifndef DKWOBJMETHOD_VCALLF_DEFINED
#define DKWOBJMETHOD_VCALLF_DEFINED 1
PRIVATE DREF DeeObject *DCALL
dkwobjmethod_vcallf(dkwobjmethod_t self,
                    DeeObject *__restrict thisarg,
                    char const *__restrict format,
                    va_list args, DeeObject *kw) {
 DREF DeeObject *result,*args_tuple;
 args_tuple = DeeTuple_VNewf(format,args);
 if unlikely(!args_tuple) goto err;
 result = (*self)(thisarg,
                  DeeTuple_SIZE(args_tuple),
                  DeeTuple_ELEM(args_tuple),
                  kw);
 Dee_Decref(args_tuple);
 return result;
err:
 return NULL;
}

PRIVATE DREF DeeObject *DCALL
dobjmethod_vcallf(dobjmethod_t self,
                  DeeObject *__restrict thisarg,
                  char const *__restrict format,
                  va_list args) {
 DREF DeeObject *result,*args_tuple;
 args_tuple = DeeTuple_VNewf(format,args);
 if unlikely(!args_tuple) goto err;
 result = (*self)(thisarg,
                  DeeTuple_SIZE(args_tuple),
                  DeeTuple_ELEM(args_tuple));
 Dee_Decref(args_tuple);
 return result;
err:
 return NULL;
}
#endif /* !DKWOBJMETHOD_VCALLF_DEFINED */

/* @return: * :        The returned value.
 * @return: NULL:      An error occurred.
 * @return: ITER_DONE: The attribute could not be found in the cache. */
INTERN DREF DeeObject *DCALL
S(DeeType_VCallCachedAttrf,
  DeeType_VCallCachedAttrLenf)(DeeTypeObject *__restrict tp_self,
                               DeeObject *__restrict self,
                               ATTR_ARG, dhash_t hash,
                               char const *__restrict format, va_list args) {
 dhash_t i,perturb; DREF DeeObject *callback,*result;
 MEMBERCACHE_READ(&tp_self->tp_cache);
 if unlikely(!tp_self->tp_cache.mc_table) goto done;
 perturb = i = MEMBERCACHE_HASHST(&tp_self->tp_cache,hash);
 for (;; i = MEMBERCACHE_HASHNX(i,perturb),MEMBERCACHE_HASHPT(perturb)) {
  struct membercache_slot *item = MEMBERCACHE_HASHIT(&tp_self->tp_cache,i);
  if (item->mcs_type == MEMBERCACHE_UNUSED) break;
  if (item->mcs_hash != hash) continue;
  if (!ATTREQ(item)) continue;
  switch (item->mcs_type) {
  {
   dobjmethod_t func;
  case MEMBERCACHE_METHOD:
   func = item->mcs_method.m_func;
   if (item->mcs_method.m_flag & TYPE_METHOD_FKWDS) {
    MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
    return dkwobjmethod_vcallf((dkwobjmethod_t)func,self,format,args,NULL);
   }
   MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
   return dobjmethod_vcallf(func,self,format,args);
  }
  {
   dgetmethod_t getter;
   DeeTypeObject *type;
  case MEMBERCACHE_GETSET:
   getter = item->mcs_getset.gs_get;
   if likely(getter) {
    MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
    callback = (*getter)(self);
check_and_invoke_callback:
    if unlikely(!callback) goto err;
    result = DeeObject_VCallf(callback,format,args);
    Dee_Decref(callback);
    return result;
   }
   type = item->mcs_decl;
   MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
   err_cant_access_attribute(type,attr,ATTR_ACCESS_GET);
   goto err;
  }
  {
   struct buffer { uint8_t dat[COMPILER_OFFSETOF(struct type_member,m_doc)]; } buf;
  case MEMBERCACHE_MEMBER:
   buf = *(struct buffer *)&item->mcs_member;
   MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
   callback = type_member_get((struct type_member *)&buf,self);
   goto check_and_invoke_callback;
  }
  {
   struct class_attribute *catt;
   struct class_desc *desc;
  case MEMBERCACHE_ATTRIB:
   catt = item->mcs_attrib.a_attr;
   desc = item->mcs_attrib.a_desc;
   MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
   return DeeInstance_VCallAttributef(desc,
                                      DeeInstance_DESC(desc,self),
                                      self,catt,format,args);
  }
  default: __builtin_unreachable();
  }
 }
done:
 MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
 return ITER_DONE;
err:
 return NULL;
}
INTERN DREF DeeObject *DCALL
S(DeeType_VCallCachedClassAttrf,
  DeeType_VCallCachedClassAttrLenf)(DeeTypeObject *__restrict tp_self,
                                    ATTR_ARG, dhash_t hash,
                                    char const *__restrict format, va_list args) {
 dhash_t i,perturb; DREF DeeObject *callback,*result,*args_tuple;
 MEMBERCACHE_READ(&tp_self->tp_class_cache);
 if unlikely(!tp_self->tp_class_cache.mc_table) goto done;
 perturb = i = MEMBERCACHE_HASHST(&tp_self->tp_class_cache,hash);
 for (;; i = MEMBERCACHE_HASHNX(i,perturb),MEMBERCACHE_HASHPT(perturb)) {
  struct membercache_slot *item = MEMBERCACHE_HASHIT(&tp_self->tp_class_cache,i);
  if (item->mcs_type == MEMBERCACHE_UNUSED) break;
  if (item->mcs_hash != hash) continue;
  if (!ATTREQ(item)) continue;
  switch (item->mcs_type) {
  {
   dobjmethod_t func;
  case MEMBERCACHE_METHOD:
   func = item->mcs_method.m_func;
   if (item->mcs_method.m_flag & TYPE_METHOD_FKWDS) {
    MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
    return dkwobjmethod_vcallf((dkwobjmethod_t)func,(DeeObject *)tp_self,format,args,NULL);
   }
   MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
   return dobjmethod_vcallf(func,(DeeObject *)tp_self,format,args);
  }
  {
   dgetmethod_t getter;
   DeeTypeObject *type;
  case MEMBERCACHE_GETSET:
   getter = item->mcs_getset.gs_get;
   if likely(getter) {
    MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
    callback = (*getter)((DeeObject *)tp_self);
check_and_invoke_callback:
    if unlikely(!callback) goto err;
    result = DeeObject_VCallf(callback,format,args);
    Dee_Decref(callback);
    return result;
   }
   type = item->mcs_decl;
   MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
   err_cant_access_attribute(type,attr,ATTR_ACCESS_GET);
   goto err;
  }
  {
   struct buffer { uint8_t dat[COMPILER_OFFSETOF(struct type_member,m_doc)]; } buf;
  case MEMBERCACHE_MEMBER:
   buf = *(struct buffer *)&item->mcs_member;
   MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
   callback = type_member_get((struct type_member *)&buf,(DeeObject *)tp_self);
   goto check_and_invoke_callback;
  }
  {
   struct class_attribute *catt;
   struct class_desc *desc;
  case MEMBERCACHE_ATTRIB:
   catt = item->mcs_attrib.a_attr;
   desc = item->mcs_attrib.a_desc;
   MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
   return DeeInstance_VCallAttributef(desc,class_desc_as_instance(desc),(DeeObject *)tp_self,catt,format,args);
  }
  {
   dobjmethod_t func;
   DeeTypeObject *type;
  case MEMBERCACHE_INSTANCE_METHOD:
   func = item->mcs_method.m_func;
   type = item->mcs_decl;
   if (item->mcs_method.m_flag & TYPE_METHOD_FKWDS) {
    MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
    args_tuple = DeeTuple_VNewf(format,args);
    if unlikely(!args_tuple) goto err;
    if unlikely(!DeeTuple_SIZE(args_tuple)) goto err_classmethod_noargs_args_tuple;
    /* Allow non-instance objects for generic types. */
    if unlikely(!(type->tp_flags & TP_FABSTRACT) && DeeObject_AssertType(DeeTuple_GET(args_tuple,0),type))
       goto err_args_tuple;
    /* Use the first argument as the this-argument. */
    result = (*(dkwobjmethod_t)func)(DeeTuple_GET(args_tuple,0),
                                     DeeTuple_SIZE(args_tuple) - 1,
                                     DeeTuple_ELEM(args_tuple) + 1,
                                     NULL);
    Dee_Decref(args_tuple);
    return result;
   }
   MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
   args_tuple = DeeTuple_VNewf(format,args);
   if unlikely(!args_tuple) goto err;
   if unlikely(!DeeTuple_SIZE(args_tuple)) goto err_classmethod_noargs_args_tuple;
   /* Allow non-instance objects for generic types. */
   if unlikely(!(type->tp_flags & TP_FABSTRACT) && DeeObject_AssertType(DeeTuple_GET(args_tuple,0),type))
      goto err_args_tuple;
   /* Use the first argument as the this-argument. */
   result = (*func)(DeeTuple_GET(args_tuple,0),
                    DeeTuple_SIZE(args_tuple) - 1,
                    DeeTuple_ELEM(args_tuple) + 1);
   Dee_Decref(args_tuple);
   return result;
  }
  {
   dgetmethod_t get;
   DeeTypeObject *type;
  case MEMBERCACHE_INSTANCE_GETSET:
   get  = item->mcs_getset.gs_get;
   type = item->mcs_decl;
   MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
   if unlikely(!get) goto err_no_getter;
   args_tuple = DeeTuple_VNewf(format,args);
   if unlikely(!args_tuple) goto err;
   if unlikely(DeeTuple_SIZE(args_tuple) != 1) goto err_classproperty_invalid_args_args_tuple;
   /* Allow non-instance objects for generic types. */
   if unlikely(!(type->tp_flags & TP_FABSTRACT) && DeeObject_AssertType(DeeTuple_GET(args_tuple,0),type))
      goto err_args_tuple;
   /* Use the first argument as the this-argument. */
   result = (*get)(DeeTuple_GET(args_tuple,0));
   Dee_Decref(args_tuple);
   return result;
  }
  {
   struct type_member member;
   DeeTypeObject *type;
  case MEMBERCACHE_INSTANCE_MEMBER:
   member = item->mcs_member;
   type = item->mcs_decl;
   MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
   args_tuple = DeeTuple_VNewf(format,args);
   if unlikely(!args_tuple) goto err;
   if unlikely(DeeTuple_SIZE(args_tuple) != 1) goto err_classmember_invalid_args_args_tuple;
   /* Allow non-instance objects for generic types. */
   if unlikely(!(type->tp_flags & TP_FABSTRACT) && DeeObject_AssertType(DeeTuple_GET(args_tuple,0),type))
      goto err_args_tuple;
   /* Use the first argument as the this-argument. */
   result = type_member_get(&member,DeeTuple_GET(args_tuple,0));
   Dee_Decref(args_tuple);
   return result;
  }
  {
   struct class_attribute *catt;
   DeeTypeObject *type;
  case MEMBERCACHE_INSTANCE_ATTRIB:
   catt = item->mcs_attrib.a_attr;
   type = item->mcs_decl;
   MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
   return DeeClass_VCallInstanceAttributef(type,catt,format,args);
  }
  default: __builtin_unreachable();
  }
 }
done:
 MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
 return ITER_DONE;
err_no_getter:
 err_cant_access_clsproperty_get();
 goto err;
err_classmember_invalid_args_args_tuple:
 Dee_Decref(args_tuple);
/*err_classmember_invalid_args:*/
 err_classmember_requires_1_argument(attr);
 goto err;
err_classproperty_invalid_args_args_tuple:
 Dee_Decref(args_tuple);
/*err_classproperty_invalid_args:*/
 err_classproperty_requires_1_argument(attr);
 goto err;
err_classmethod_noargs_args_tuple:
 Dee_Decref(args_tuple);
/*err_classmethod_noargs:*/
 err_classmethod_requires_at_least_1_argument(attr);
 goto err;
err_args_tuple:
 Dee_Decref(args_tuple);
err:
 return NULL;
}
#if 0
INTERN DREF DeeObject *DCALL
S(DeeType_VCallCachedInstanceAttrf,
  DeeType_VCallCachedInstanceAttrLenf)(DeeTypeObject *__restrict tp_self,
                                      ATTR_ARG, dhash_t hash,
                                      char const *__restrict format, va_list args) {
 dhash_t i,perturb; DREF DeeObject *callback,*result,*args_tuple;
 MEMBERCACHE_READ(&tp_self->tp_cache);
 if unlikely(!tp_self->tp_cache.mc_table) goto done;
 perturb = i = MEMBERCACHE_HASHST(&tp_self->tp_cache,hash);
 for (;; i = MEMBERCACHE_HASHNX(i,perturb),MEMBERCACHE_HASHPT(perturb)) {
  struct membercache_slot *item = MEMBERCACHE_HASHIT(&tp_self->tp_cache,i);
  if (item->mcs_type == MEMBERCACHE_UNUSED) break;
  if (item->mcs_hash != hash) continue;
  if (!ATTREQ(item)) continue;
  switch (item->mcs_type) {
  {
   dobjmethod_t func;
   DeeTypeObject *type;
  case MEMBERCACHE_METHOD:
   func = item->mcs_method.m_func;
   type = item->mcs_decl;
   if (item->mcs_method.m_flag & TYPE_METHOD_FKWDS) {
    MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
    args_tuple = DeeTuple_VNewf(format,args);
    if unlikely(!args_tuple) goto err;
    if unlikely(!DeeTuple_SIZE(args_tuple)) goto err_classmethod_noargs_args_tuple;
    /* Allow non-instance objects for generic types. */
    if unlikely(!(type->tp_flags & TP_FABSTRACT) && DeeObject_AssertType(DeeTuple_GET(args_tuple,0),type))
       goto err_args_tuple;
    /* Use the first argument as the this-argument. */
    result = (*(dkwobjmethod_t)func)(DeeTuple_GET(args_tuple,0),
                                     DeeTuple_SIZE(args_tuple) - 1,
                                     DeeTuple_ELEM(args_tuple) + 1,
                                     NULL);
    Dee_Decref(args_tuple);
    return result;
   }
   MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
   args_tuple = DeeTuple_VNewf(format,args);
   if unlikely(!args_tuple) goto err;
   if unlikely(!DeeTuple_SIZE(args_tuple)) goto err_classmethod_noargs_args_tuple;
   /* Allow non-instance objects for generic types. */
   if unlikely(!(type->tp_flags & TP_FABSTRACT) && DeeObject_AssertType(DeeTuple_GET(args_tuple,0),type))
      goto err_args_tuple;
   /* Use the first argument as the this-argument. */
   result = (*func)(DeeTuple_GET(args_tuple,0),
                    DeeTuple_SIZE(args_tuple) - 1,
                    DeeTuple_ELEM(args_tuple) + 1);
   Dee_Decref(args_tuple);
   return result;
  }
  {
   dgetmethod_t get;
   DeeTypeObject *type;
  case MEMBERCACHE_GETSET:
   get  = item->mcs_getset.gs_get;
   type = item->mcs_decl;
   MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
   if unlikely(!get) goto err_no_getter;
   args_tuple = DeeTuple_VNewf(format,args);
   if unlikely(!args_tuple) goto err;
   if unlikely(DeeTuple_SIZE(args_tuple) != 1) goto err_classproperty_invalid_args_args_tuple;
   /* Allow non-instance objects for generic types. */
   if unlikely(!(type->tp_flags & TP_FABSTRACT) && DeeObject_AssertType(DeeTuple_GET(args_tuple,0),type))
      goto err_args_tuple;
   /* Use the first argument as the this-argument. */
   result = (*get)(DeeTuple_GET(args_tuple,0));
   Dee_Decref(args_tuple);
   return result;
  }
  {
   struct type_member member;
   DeeTypeObject *type;
  case MEMBERCACHE_MEMBER:
   member = item->mcs_member;
   type = item->mcs_decl;
   MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
   args_tuple = DeeTuple_VNewf(format,args);
   if unlikely(!args_tuple) goto err;
   if unlikely(DeeTuple_SIZE(args_tuple) != 1) goto err_classmember_invalid_args_args_tuple;
   /* Allow non-instance objects for generic types. */
   if unlikely(!(type->tp_flags & TP_FABSTRACT) && DeeObject_AssertType(DeeTuple_GET(args_tuple,0),type))
      goto err_args_tuple;
   /* Use the first argument as the this-argument. */
   result = type_member_get(&member,DeeTuple_GET(args_tuple,0));
   Dee_Decref(args_tuple);
   return result;
  }
  {
   struct class_attribute *catt;
   DeeTypeObject *type;
  case MEMBERCACHE_ATTRIB:
   catt = item->mcs_attrib.a_attr;
   type = item->mcs_decl;
   MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
   return DeeClass_VCallInstanceAttributef(type,catt,format,args);
  }
  default: __builtin_unreachable();
  }
 }
done:
 MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
 return ITER_DONE;
err_no_getter:
 err_cant_access_clsproperty_get();
 goto err;
err_classmember_invalid_args_args_tuple:
 Dee_Decref(args_tuple);
/*err_classmember_invalid_args:*/
 err_classmember_requires_1_argument(attr);
 goto err;
err_classproperty_invalid_args_args_tuple:
 Dee_Decref(args_tuple);
/*err_classproperty_invalid_args:*/
 err_classproperty_requires_1_argument(attr);
 goto err;
err_classmethod_noargs_args_tuple:
 Dee_Decref(args_tuple);
/*err_classmethod_noargs:*/
 err_classmethod_requires_at_least_1_argument(attr);
 goto err;
err_args_tuple:
 Dee_Decref(args_tuple);
err:
 return NULL;
}
#endif
#endif /* !MRO_LEN */


#ifndef TYPE_MEMBER_TYPEFOR_DEFINED
#define TYPE_MEMBER_TYPEFOR_DEFINED 1
INTDEF DeeTypeObject *DCALL
type_member_typefor(struct type_member *__restrict self);
#endif

#ifndef MRO_LEN
INTERN int DCALL
DeeType_FindCachedAttr(DeeTypeObject *__restrict tp_self, DeeObject *instance,
                       struct attribute_info *__restrict result,
                       struct attribute_lookup_rules const *__restrict rules) {
 dhash_t i,perturb;
 MEMBERCACHE_READ(&tp_self->tp_cache);
 if unlikely(!tp_self->tp_cache.mc_table) goto done;
 perturb = i = MEMBERCACHE_HASHST(&tp_self->tp_cache,rules->alr_hash);
 for (;; i = MEMBERCACHE_HASHNX(i,perturb),MEMBERCACHE_HASHPT(perturb)) {
  struct membercache_slot *item = MEMBERCACHE_HASHIT(&tp_self->tp_cache,i);
  char const *doc; uint16_t perm;
  DREF DeeTypeObject *member_decl,*member_type;
  if (item->mcs_type == MEMBERCACHE_UNUSED) break;
  if (item->mcs_hash != rules->alr_hash) continue;
  if (!streq(item->mcs_name,rules->alr_name)) continue;
  if (rules->alr_decl && (DeeObject *)item->mcs_decl != result->a_decl)
      break; /* Attribute isn't declared by the requested declarator. */
  switch (item->mcs_type) {
  case MEMBERCACHE_METHOD:
   perm        = ATTR_IMEMBER | ATTR_PERMGET | ATTR_PERMCALL;
   doc         = item->mcs_method.m_doc;
   member_type = (item->mcs_method.m_flag & TYPE_METHOD_FKWDS)
               ? &DeeKwObjMethod_Type
               : &DeeObjMethod_Type;
   Dee_Incref(member_type);
   break;
  case MEMBERCACHE_GETSET:
   perm        = ATTR_IMEMBER | ATTR_PROPERTY;
   doc         = item->mcs_getset.gs_doc;
   member_type = NULL;
   if (item->mcs_getset.gs_get) perm |= ATTR_PERMGET;
   if (item->mcs_getset.gs_del) perm |= ATTR_PERMDEL;
   if (item->mcs_getset.gs_set) perm |= ATTR_PERMSET;
   break;
  case MEMBERCACHE_MEMBER:
   perm = ATTR_IMEMBER | ATTR_PERMGET;
   doc  = item->mcs_member.m_doc;
   if (TYPE_MEMBER_ISCONST(&item->mcs_member)) {
    member_type = Dee_TYPE(item->mcs_member.m_const);
    Dee_Incref(member_type);
   } else {
    /* TODO: Use `type_member_get(&item->mcs_member,instance)' to determine the proper attribute type! */
    member_type = type_member_typefor(&item->mcs_member);
    Dee_XIncref(member_type);
    if (!(item->mcs_member.m_field.m_type & STRUCT_CONST))
        perm |= ATTR_PERMDEL | ATTR_PERMSET;
   }
   break;
  {
   struct class_attribute *catt;
   struct instance_desc *inst;
  case MEMBERCACHE_ATTRIB:
   doc  = NULL;
   catt = item->mcs_attrib.a_attr;
   perm = ATTR_IMEMBER | ATTR_PERMGET | ATTR_PERMDEL | ATTR_PERMSET;
   if (catt->ca_doc) {
    doc   = DeeString_STR(catt->ca_doc);
    perm |= ATTR_DOCOBJ;
    Dee_Incref(catt->ca_doc);
   }
   if (catt->ca_flag & CLASS_ATTRIBUTE_FPRIVATE)
       perm |= ATTR_PRIVATE;
   if (catt->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
    perm       |= ATTR_PROPERTY;
    member_type = NULL;
   } else if (catt->ca_flag & CLASS_ATTRIBUTE_FMETHOD) {
    perm       |= ATTR_PERMCALL;
    member_type = &DeeInstanceMethod_Type;
    Dee_Incref(member_type);
   } else {
    member_type = NULL;
   }
   inst = NULL;
   if (catt->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM)
       inst = class_desc_as_instance(item->mcs_attrib.a_desc);
   else if (instance)
       inst = DeeInstance_DESC(item->mcs_attrib.a_desc,instance);
   if (inst) {
    rwlock_read(&inst->id_lock);
    if (catt->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
     if (!inst->id_vtab[catt->ca_addr + CLASS_GETSET_GET])
          perm &= ~ATTR_PERMGET;
     if (!(catt->ca_flag & CLASS_ATTRIBUTE_FREADONLY)) {
      if (!inst->id_vtab[catt->ca_addr + CLASS_GETSET_DEL])
           perm &= ~ATTR_PERMDEL;
      if (!inst->id_vtab[catt->ca_addr + CLASS_GETSET_SET])
           perm &= ~ATTR_PERMSET;
     }
    } else if (!(catt->ca_flag & CLASS_ATTRIBUTE_FMETHOD)) {
     ASSERT(!member_type);
     member_type = (DREF DeeTypeObject *)inst->id_vtab[catt->ca_addr + CLASS_GETSET_GET];
     if (member_type) { member_type = Dee_TYPE(member_type); Dee_Incref(member_type); }
    }
    rwlock_endread(&inst->id_lock);
   }
   if (catt->ca_flag & CLASS_ATTRIBUTE_FREADONLY)
       perm &= ~(ATTR_PERMDEL | ATTR_PERMSET);
  } break;
  default: __builtin_unreachable();
  }
  member_decl = item->mcs_decl;
  Dee_Incref(member_decl);
  MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
  if ((perm & rules->alr_perm_mask) != rules->alr_perm_value) {
   if (perm & ATTR_DOCOBJ)
       Dee_Decref(COMPILER_CONTAINER_OF(doc,DeeStringObject,s_str));
   Dee_XDecref(member_type);
   Dee_Decref(member_decl);
   goto not_found;
  }
  result->a_decl     = (DREF DeeObject *)member_decl;
  result->a_doc      = doc;
  result->a_perm     = perm;
  result->a_attrtype = member_type; /* Inherit reference. */
  return 0;
 }
done:
 MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
not_found:
 return 1;
}
INTERN int DCALL
DeeType_FindCachedClassAttr(DeeTypeObject *__restrict tp_self,
                            struct attribute_info *__restrict result,
                            struct attribute_lookup_rules const *__restrict rules) {
 dhash_t i,perturb;
 MEMBERCACHE_READ(&tp_self->tp_class_cache);
 if unlikely(!tp_self->tp_class_cache.mc_table) goto done;
 perturb = i = MEMBERCACHE_HASHST(&tp_self->tp_class_cache,rules->alr_hash);
 for (;; i = MEMBERCACHE_HASHNX(i,perturb),MEMBERCACHE_HASHPT(perturb)) {
  struct membercache_slot *item = MEMBERCACHE_HASHIT(&tp_self->tp_class_cache,i);
  char const *doc; uint16_t perm;
  DREF DeeTypeObject *member_decl,*member_type;
  if (item->mcs_type == MEMBERCACHE_UNUSED) break;
  if (item->mcs_hash != rules->alr_hash) continue;
  if (!streq(item->mcs_name,rules->alr_name)) continue;
  if (rules->alr_decl && (DeeObject *)item->mcs_decl != result->a_decl)
      break; /* Attribute isn't declared by the requested declarator. */
  switch (item->mcs_type) {
  case MEMBERCACHE_METHOD:
   perm        = ATTR_CMEMBER | ATTR_PERMGET | ATTR_PERMCALL;
   doc         = item->mcs_method.m_doc;
   member_type = (item->mcs_method.m_flag & TYPE_METHOD_FKWDS)
               ? &DeeKwObjMethod_Type
               : &DeeObjMethod_Type;
   Dee_Incref(member_type);
   break;
  case MEMBERCACHE_GETSET:
   perm        = ATTR_CMEMBER | ATTR_PROPERTY;
   doc         = item->mcs_getset.gs_doc;
   member_type = NULL;
   if (item->mcs_getset.gs_get) perm |= ATTR_PERMGET;
   if (item->mcs_getset.gs_del) perm |= ATTR_PERMDEL;
   if (item->mcs_getset.gs_set) perm |= ATTR_PERMSET;
   break;
  case MEMBERCACHE_MEMBER:
   perm = ATTR_CMEMBER | ATTR_PERMGET;
   doc  = item->mcs_member.m_doc;
   if (TYPE_MEMBER_ISCONST(&item->mcs_member)) {
    member_type = Dee_TYPE(item->mcs_member.m_const);
    Dee_Incref(member_type);
   } else {
    /* TODO: Use `type_member_get(&item->mcs_member,(DeeObject *)tp_self)' to determine the proper attribute type! */
    member_type = type_member_typefor(&item->mcs_member);
    Dee_XIncref(member_type);
    if (!(item->mcs_member.m_field.m_type & STRUCT_CONST))
        perm |= ATTR_PERMDEL | ATTR_PERMSET;
   }
   break;
  {
   struct class_attribute *catt;
   struct instance_desc *inst;
  case MEMBERCACHE_ATTRIB:
   doc  = NULL;
   catt = item->mcs_attrib.a_attr;
   perm = ATTR_CMEMBER | ATTR_PERMGET | ATTR_PERMDEL | ATTR_PERMSET;
   if (catt->ca_doc) {
    doc   = DeeString_STR(catt->ca_doc);
    perm |= ATTR_DOCOBJ;
    Dee_Incref(catt->ca_doc);
   }
   if (catt->ca_flag & CLASS_ATTRIBUTE_FPRIVATE)
       perm |= ATTR_PRIVATE;
   if (catt->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
    perm       |= ATTR_PROPERTY;
    member_type = NULL;
   } else if (catt->ca_flag & CLASS_ATTRIBUTE_FMETHOD) {
    perm       |= ATTR_PERMCALL;
    member_type = &DeeInstanceMethod_Type;
    Dee_Incref(member_type);
   } else {
    member_type = NULL;
   }
   inst = class_desc_as_instance(item->mcs_attrib.a_desc);
   rwlock_read(&inst->id_lock);
   if (catt->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
    if (!inst->id_vtab[catt->ca_addr + CLASS_GETSET_GET])
         perm &= ~ATTR_PERMGET;
    if (!(catt->ca_flag & CLASS_ATTRIBUTE_FREADONLY)) {
     if (!inst->id_vtab[catt->ca_addr + CLASS_GETSET_DEL])
          perm &= ~ATTR_PERMDEL;
     if (!inst->id_vtab[catt->ca_addr + CLASS_GETSET_SET])
          perm &= ~ATTR_PERMSET;
    }
   } else if (!(catt->ca_flag & CLASS_ATTRIBUTE_FMETHOD)) {
    ASSERT(!member_type);
    member_type = (DREF DeeTypeObject *)inst->id_vtab[catt->ca_addr + CLASS_GETSET_GET];
    if (member_type) { member_type = Dee_TYPE(member_type); Dee_Incref(member_type); }
   }
   rwlock_endread(&inst->id_lock);
   if (catt->ca_flag & CLASS_ATTRIBUTE_FREADONLY)
       perm &= ~(ATTR_PERMDEL | ATTR_PERMSET);
  } break;

  case MEMBERCACHE_INSTANCE_METHOD:
   perm        = ATTR_CMEMBER | ATTR_IMEMBER | ATTR_WRAPPER | ATTR_PERMGET | ATTR_PERMCALL;
   doc         = item->mcs_method.m_doc;
   member_type = (item->mcs_method.m_flag & TYPE_METHOD_FKWDS)
               ? &DeeKwClsMethod_Type
               : &DeeClsMethod_Type;
   Dee_Incref(member_type);
   break;
  case MEMBERCACHE_INSTANCE_GETSET:
   perm        = ATTR_CMEMBER | ATTR_IMEMBER | ATTR_WRAPPER | ATTR_PROPERTY;
   doc         = item->mcs_getset.gs_doc;
   member_type = NULL/*&DeeClsProperty_Type*/;
   if (item->mcs_getset.gs_get) perm |= ATTR_PERMGET;
   if (item->mcs_getset.gs_del) perm |= ATTR_PERMDEL;
   if (item->mcs_getset.gs_set) perm |= ATTR_PERMSET;
   break;
  case MEMBERCACHE_INSTANCE_MEMBER:
   perm = ATTR_CMEMBER | ATTR_IMEMBER | ATTR_WRAPPER;
   doc  = item->mcs_member.m_doc;
   /*member_type = &DeeClsMember_Type*/;
   if (TYPE_MEMBER_ISCONST(&item->mcs_member)) {
    member_type = Dee_TYPE(item->mcs_member.m_const);
    Dee_Incref(member_type);
   } else {
    member_type = type_member_typefor(&item->mcs_member);
    Dee_XIncref(member_type);
    if (!(item->mcs_member.m_field.m_type & STRUCT_CONST))
        perm |= ATTR_PERMDEL | ATTR_PERMSET;
   }
   break;

  {
   struct class_attribute *catt;
  case MEMBERCACHE_INSTANCE_ATTRIB:
   doc  = NULL;
   catt = item->mcs_attrib.a_attr;
   perm = ATTR_CMEMBER | ATTR_IMEMBER | ATTR_WRAPPER | ATTR_PERMGET | ATTR_PERMDEL | ATTR_PERMSET;
   if (catt->ca_doc) {
    doc   = DeeString_STR(catt->ca_doc);
    perm |= ATTR_DOCOBJ;
    Dee_Incref(catt->ca_doc);
   }
   if (catt->ca_flag & CLASS_ATTRIBUTE_FPRIVATE)
       perm |= ATTR_PRIVATE;
   if (catt->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
    perm       |= ATTR_PROPERTY;
    member_type = NULL;
   } else if (catt->ca_flag & CLASS_ATTRIBUTE_FMETHOD) {
    perm       |= ATTR_PERMCALL;
    member_type = &DeeInstanceMethod_Type;
    Dee_Incref(member_type);
   } else {
    member_type = NULL;
   }
   if (catt->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM) {
    struct instance_desc *inst;
    inst = class_desc_as_instance(item->mcs_attrib.a_desc);
    rwlock_read(&inst->id_lock);
    if (catt->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
     if (!inst->id_vtab[catt->ca_addr + CLASS_GETSET_GET])
          perm &= ~ATTR_PERMGET;
     if (!(catt->ca_flag & CLASS_ATTRIBUTE_FREADONLY)) {
      if (!inst->id_vtab[catt->ca_addr + CLASS_GETSET_DEL])
           perm &= ~ATTR_PERMDEL;
      if (!inst->id_vtab[catt->ca_addr + CLASS_GETSET_SET])
           perm &= ~ATTR_PERMSET;
     }
    } else if (!(catt->ca_flag & CLASS_ATTRIBUTE_FMETHOD)) {
     ASSERT(!member_type);
     member_type = (DREF DeeTypeObject *)inst->id_vtab[catt->ca_addr + CLASS_GETSET_GET];
     if (member_type) { member_type = Dee_TYPE(member_type); Dee_Incref(member_type); }
    }
    rwlock_endread(&inst->id_lock);
   }
   if (catt->ca_flag & CLASS_ATTRIBUTE_FREADONLY)
       perm &= ~(ATTR_PERMDEL | ATTR_PERMSET);
  } break;
  default: __builtin_unreachable();
  }
  member_decl = item->mcs_decl;
  Dee_Incref(member_decl);
  MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
  if ((perm & rules->alr_perm_mask) != rules->alr_perm_value) {
   if (perm & ATTR_DOCOBJ)
       Dee_Decref(COMPILER_CONTAINER_OF(doc,DeeStringObject,s_str));
   Dee_XDecref(member_type);
   Dee_Decref(member_decl);
   goto not_found;
  }
  result->a_decl     = (DREF DeeObject *)member_decl;
  result->a_doc      = doc;
  result->a_perm     = perm;
  result->a_attrtype = member_type; /* Inherit reference. */
  return 0;
 }
done:
 MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
not_found:
 return 1;
}
#endif /* !MRO_LEN */



#ifndef MRO_LEN
INTDEF struct class_attribute *
(DCALL DeeType_QueryAttributeWithHash)(DeeTypeObject *__restrict tp_invoker,
                                       DeeTypeObject *__restrict tp_self,
                                       /*String*/ DeeObject *__restrict attr,
                                       dhash_t hash) {
 struct class_attribute *result;
 result = DeeClass_QueryInstanceAttributeWithHash(tp_self,attr,hash);
 if (result)
     membercache_addattrib(&tp_invoker->tp_cache,tp_self,hash,result);
 return result;
}
#endif /* !MRO_LEN */
INTDEF struct class_attribute *
(DCALL S(DeeType_QueryAttributeStringWithHash,
         DeeType_QueryAttributeStringLenWithHash))(DeeTypeObject *__restrict tp_invoker,
                                                   DeeTypeObject *__restrict tp_self,
                                                   ATTR_ARG, dhash_t hash) {
 struct class_attribute *result;
 result = S(DeeClass_QueryInstanceAttributeStringWithHash(tp_self,attr,hash),
            DeeClass_QueryInstanceAttributeStringLenWithHash(tp_self,attr,attrlen,hash));
 if (result)
     membercache_addattrib(&tp_invoker->tp_cache,tp_self,hash,result);
 return result;
}
#ifndef MRO_LEN
INTDEF struct class_attribute *
(DCALL DeeType_QueryClassAttributeWithHash)(DeeTypeObject *__restrict tp_invoker,
                                            DeeTypeObject *__restrict tp_self,
                                            /*String*/ DeeObject *__restrict attr,
                                            dhash_t hash) {
 struct class_attribute *result;
 result = DeeClass_QueryClassAttributeWithHash(tp_self,attr,hash);
 if (result)
     membercache_addattrib(&tp_invoker->tp_class_cache,tp_self,hash,result);
 return result;
}
#endif /* !MRO_LEN */

INTDEF struct class_attribute *
(DCALL S(DeeType_QueryClassAttributeStringWithHash,
         DeeType_QueryClassAttributeStringLenWithHash))(DeeTypeObject *__restrict tp_invoker,
                                                        DeeTypeObject *__restrict tp_self,
                                                        ATTR_ARG, dhash_t hash) {
 struct class_attribute *result;
 result = S(DeeClass_QueryClassAttributeStringWithHash(tp_self,attr,hash),
            DeeClass_QueryClassAttributeStringLenWithHash(tp_self,attr,attrlen,hash));
 if (result)
     membercache_addattrib(&tp_invoker->tp_class_cache,tp_self,hash,result);
 return result;
}
#ifndef MRO_LEN
INTDEF struct class_attribute *
(DCALL DeeType_QueryInstanceAttributeWithHash)(DeeTypeObject *__restrict tp_invoker,
                                               DeeTypeObject *__restrict tp_self,
                                               /*String*/ DeeObject *__restrict attr,
                                               dhash_t hash) {
 struct class_attribute *result;
 result = DeeClass_QueryInstanceAttributeWithHash(tp_self,attr,hash);
 if (result)
     membercache_addinstanceattrib(&tp_invoker->tp_class_cache,tp_self,hash,result);
 return result;
}
#endif /* !MRO_LEN */
INTDEF struct class_attribute *
(DCALL S(DeeType_QueryInstanceAttributeStringWithHash,
         DeeType_QueryInstanceAttributeStringLenWithHash))(DeeTypeObject *__restrict tp_invoker,
                                                           DeeTypeObject *__restrict tp_self,
                                                           ATTR_ARG, dhash_t hash) {
 struct class_attribute *result;
 result = S(DeeClass_QueryInstanceAttributeStringWithHash(tp_self,attr,hash),
            DeeClass_QueryInstanceAttributeStringLenWithHash(tp_self,attr,attrlen,hash));
 if (result)
     membercache_addinstanceattrib(&tp_invoker->tp_class_cache,tp_self,hash,result);
 return result;
}



INTERN DREF DeeObject *DCALL
N_len(type_method_getattr)(struct membercache *__restrict cache, DeeTypeObject *__restrict decl,
                           struct type_method *__restrict chain, DeeObject *__restrict self,
                           ATTR_ARG, dhash_t hash) {
 for (; chain->m_name; ++chain) {
  if (!NAMEEQ(chain->m_name)) continue;
  membercache_addmethod(cache,decl,hash,chain);
  return type_method_get(chain,self);
 }
 return ITER_DONE;
}
INTERN DREF DeeObject *
(DCALL NLen(DeeType_GetInstanceMethodAttr))(DeeTypeObject *__restrict tp_invoker,
                                            DeeTypeObject *__restrict tp_self,
                                            ATTR_ARG, dhash_t hash) {
 struct type_method *chain = tp_self->tp_methods;
 for (; chain->m_name; ++chain) {
  if (!NAMEEQ(chain->m_name)) continue;
  membercache_addinstancemethod(&tp_invoker->tp_class_cache,tp_self,hash,chain);
  return type_obmeth_get(tp_self,chain);
 }
 return ITER_DONE;
}
INTERN DREF DeeObject *
(DCALL NLen(DeeType_GetIInstanceMethodAttr))(DeeTypeObject *__restrict tp_invoker,
                                             DeeTypeObject *__restrict tp_self,
                                             ATTR_ARG, dhash_t hash) {
 struct type_method *chain = tp_self->tp_methods;
 for (; chain->m_name; ++chain) {
  if (!NAMEEQ(chain->m_name)) continue;
  membercache_addmethod(&tp_invoker->tp_cache,tp_self,hash,chain);
  return type_obmeth_get(tp_self,chain);
 }
 return ITER_DONE;
}

INTERN DREF DeeObject *DCALL
N_len(type_method_callattr)(struct membercache *__restrict cache, DeeTypeObject *__restrict decl,
                            struct type_method *__restrict chain, DeeObject *__restrict self,
                            ATTR_ARG, dhash_t hash,
                            size_t argc, DeeObject **__restrict argv) {
 for (; chain->m_name; ++chain) {
  if (!NAMEEQ(chain->m_name)) continue;
  membercache_addmethod(cache,decl,hash,chain);
  return type_method_call(chain,self,argc, argv);
 }
 return ITER_DONE;
}
INTDEF DREF DeeObject *
(DCALL NLen(DeeType_CallInstanceMethodAttr))(DeeTypeObject *__restrict tp_invoker,
                                             DeeTypeObject *__restrict tp_self,
                                             ATTR_ARG, dhash_t hash,
                                             size_t argc, DeeObject **__restrict argv) {
 struct type_method *chain = tp_self->tp_methods;
 for (; chain->m_name; ++chain) {
  if (!NAMEEQ(chain->m_name)) continue;
  membercache_addinstancemethod(&tp_invoker->tp_class_cache,tp_self,hash,chain);
  return type_obmeth_call(tp_self,chain,argc, argv);
 }
 return ITER_DONE;
}

INTERN DREF DeeObject *DCALL
S(type_method_callattr_kw,
  type_method_callattr_len_kw)(struct membercache *__restrict cache, DeeTypeObject *__restrict decl,
                               struct type_method *__restrict chain, DeeObject *__restrict self,
                               ATTR_ARG, dhash_t hash,
                               size_t argc, DeeObject **__restrict argv, DeeObject *kw) {
 for (; chain->m_name; ++chain) {
  if (!NAMEEQ(chain->m_name)) continue;
  membercache_addmethod(cache,decl,hash,chain);
  return type_method_call_kw(chain,self,argc, argv, kw);
 }
 return ITER_DONE;
}

INTERN DREF DeeObject *
(DCALL S(DeeType_CallInstanceMethodAttrKw,
         DeeType_CallInstanceMethodAttrLenKw))(DeeTypeObject *__restrict tp_invoker,
                                               DeeTypeObject *__restrict tp_self,
                                               ATTR_ARG, dhash_t hash,
                                               size_t argc, DeeObject **__restrict argv,
                                               DeeObject *kw) {
 struct type_method *chain = tp_self->tp_methods;
 for (; chain->m_name; ++chain) {
  if (!NAMEEQ(chain->m_name)) continue;
  membercache_addinstancemethod(&tp_invoker->tp_class_cache,tp_self,hash,chain);
  return type_obmeth_call_kw(tp_self,chain,argc, argv, kw);
 }
 return ITER_DONE;
}
INTERN DREF DeeObject *
(DCALL S(DeeType_CallIInstanceMethodAttrKw,
         DeeType_CallIInstanceMethodAttrLenKw))(DeeTypeObject *__restrict tp_invoker,
                                                DeeTypeObject *__restrict tp_self,
                                                ATTR_ARG, dhash_t hash,
                                                size_t argc, DeeObject **__restrict argv,
                                                DeeObject *kw) {
 struct type_method *chain = tp_self->tp_methods;
 for (; chain->m_name; ++chain) {
  if (!NAMEEQ(chain->m_name)) continue;
  membercache_addmethod(&tp_invoker->tp_cache,tp_self,hash,chain);
  return type_obmeth_call_kw(tp_self,chain,argc, argv, kw);
 }
 return ITER_DONE;
}

INTERN DREF DeeObject *DCALL
S(type_instance_method_callattr_kw,
  type_instance_method_callattr_len_kw)(struct membercache *__restrict cache,
                                        DeeTypeObject *__restrict decl,
                                        struct type_method *__restrict chain,
                                        ATTR_ARG, dhash_t hash,
                                        size_t argc, DeeObject **__restrict argv, DeeObject *kw) {
 for (; chain->m_name; ++chain) {
  if (!NAMEEQ(chain->m_name)) continue;
  membercache_addinstancemethod(cache,decl,hash,chain);
  return type_obmeth_call_kw(decl,chain,argc, argv, kw);
 }
 return ITER_DONE;
}

#ifndef MRO_LEN
INTERN DREF DeeObject *DCALL
type_method_vcallattrf(struct membercache *__restrict cache, DeeTypeObject *__restrict decl,
                       struct type_method *__restrict chain, DeeObject *__restrict self,
                       ATTR_ARG, dhash_t hash,
                       char const *__restrict format, va_list args) {
 DREF DeeObject *result,*args_tuple;
 for (; chain->m_name; ++chain) {
  if (!NAMEEQ(chain->m_name)) continue;
  membercache_addmethod(cache,decl,hash,chain);
  args_tuple = DeeTuple_VNewf(format,args);
  if unlikely(!args_tuple) goto err;
  result = type_method_call(chain,self,
                            DeeTuple_SIZE(args_tuple),
                            DeeTuple_ELEM(args_tuple));
  Dee_Decref(args_tuple);
  return result;
 }
 return ITER_DONE;
err:
 return NULL;
}

#if 0
INTERN DREF DeeObject *
(DCALL DeeType_VCallInstanceMethodAttrf)(DeeTypeObject *__restrict tp_invoker,
                                         DeeTypeObject *__restrict tp_self,
                                         ATTR_ARG, dhash_t hash,
                                         char const *__restrict format, va_list args) {
 DREF DeeObject *result,*args_tuple;
 struct type_method *chain = tp_self->tp_methods;
 for (; chain->m_name; ++chain) {
  if (!NAMEEQ(chain->m_name)) continue;
  membercache_addinstancemethod(&tp_invoker->tp_class_cache,tp_self,hash,chain);
  args_tuple = DeeTuple_VNewf(format,args);
  if unlikely(!args_tuple) goto err;
  result = type_obmeth_call(tp_self,chain,
                            DeeTuple_SIZE(args_tuple),
                            DeeTuple_ELEM(args_tuple));
  Dee_Decref(args_tuple);
  return result;
 }
 return ITER_DONE;
err:
 return NULL;
}

INTERN DREF DeeObject *
(DCALL DeeType_VCallIInstanceMethodAttrf)(DeeTypeObject *__restrict tp_invoker,
                                          DeeTypeObject *__restrict tp_self,
                                          ATTR_ARG, dhash_t hash,
                                          char const *__restrict format, va_list args) {
 DREF DeeObject *result,*args_tuple;
 struct type_method *chain = tp_self->tp_methods;
 for (; chain->m_name; ++chain) {
  if (!NAMEEQ(chain->m_name)) continue;
  membercache_addmethod(&tp_invoker->tp_cache,tp_self,hash,chain);
  args_tuple = DeeTuple_VNewf(format,args);
  if unlikely(!args_tuple) goto err;
  result = type_obmeth_call(tp_self,chain,
                            DeeTuple_SIZE(args_tuple),
                            DeeTuple_ELEM(args_tuple));
  Dee_Decref(args_tuple);
  return result;
 }
 return ITER_DONE;
err:
 return NULL;
}
#endif
#endif /* !MRO_LEN */


INTERN DREF DeeObject *DCALL /* GET_GETSET */
N_len(type_getset_getattr)(struct membercache *__restrict cache, DeeTypeObject *__restrict decl,
                           struct type_getset *__restrict chain, DeeObject *__restrict self,
                           ATTR_ARG, dhash_t hash) {
 for (; chain->gs_name; ++chain) {
  if (!NAMEEQ(chain->gs_name)) continue;
  membercache_addgetset(cache,decl,hash,chain);
  return type_getset_get(chain,self);
 }
 return ITER_DONE;
}

INTERN DREF DeeObject *
(DCALL NLen(DeeType_GetInstanceGetSetAttr))(DeeTypeObject *__restrict tp_invoker,
                                            DeeTypeObject *__restrict tp_self,
                                            ATTR_ARG, dhash_t hash) {
 struct type_getset *chain = tp_self->tp_getsets;
 for (; chain->gs_name; ++chain) {
  if (!NAMEEQ(chain->gs_name)) continue;
  membercache_addinstancegetset(&tp_invoker->tp_class_cache,tp_self,hash,chain);
  return type_obprop_get(tp_self,chain);
 }
 return ITER_DONE;
}
INTERN DREF DeeObject *
(DCALL NLen(DeeType_GetIInstanceGetSetAttr))(DeeTypeObject *__restrict tp_invoker,
                                             DeeTypeObject *__restrict tp_self,
                                             ATTR_ARG, dhash_t hash) {
 struct type_getset *chain = tp_self->tp_getsets;
 for (; chain->gs_name; ++chain) {
  if (!NAMEEQ(chain->gs_name)) continue;
  membercache_addgetset(&tp_invoker->tp_cache,tp_self,hash,chain);
  return type_obprop_get(tp_self,chain);
 }
 return ITER_DONE;
}
INTERN DREF DeeObject *
(DCALL NLen(DeeType_CallInstanceGetSetAttr))(DeeTypeObject *__restrict tp_invoker,
                                             DeeTypeObject *__restrict tp_self,
                                             ATTR_ARG, dhash_t hash,
                                             size_t argc, DeeObject **__restrict argv) {
 struct type_getset *chain = tp_self->tp_getsets;
 for (; chain->gs_name; ++chain) {
  if (!NAMEEQ(chain->gs_name)) continue;
  membercache_addinstancegetset(&tp_invoker->tp_class_cache,tp_self,hash,chain);
  return type_obprop_call(tp_self,chain,argc, argv);
 }
 return ITER_DONE;
}
#if 0
INTERN DREF DeeObject *
(DCALL NLen(DeeType_CallIInstanceGetSetAttr))(DeeTypeObject *__restrict tp_invoker,
                                              DeeTypeObject *__restrict tp_self,
                                              ATTR_ARG, dhash_t hash,
                                              size_t argc, DeeObject **__restrict argv) {
 struct type_getset *chain = tp_self->tp_getsets;
 for (; chain->gs_name; ++chain) {
  if (!NAMEEQ(chain->gs_name)) continue;
  membercache_addgetset(&tp_invoker->tp_cache,tp_self,hash,chain);
  return type_obprop_call(tp_self,chain,argc, argv);
 }
 return ITER_DONE;
}
#endif
INTERN DREF DeeObject *
(DCALL S(DeeType_CallInstanceGetSetAttrKw,
         DeeType_CallInstanceGetSetAttrLenKw))(DeeTypeObject *__restrict tp_invoker,
                                               DeeTypeObject *__restrict tp_self,
                                               ATTR_ARG, dhash_t hash,
                                               size_t argc, DeeObject **__restrict argv,
                                               DeeObject *kw) {
 struct type_getset *chain = tp_self->tp_getsets;
 for (; chain->gs_name; ++chain) {
  if (!NAMEEQ(chain->gs_name)) continue;
  membercache_addinstancegetset(&tp_invoker->tp_class_cache,tp_self,hash,chain);
  return type_obprop_call_kw(tp_self,chain,argc, argv, kw);
 }
 return ITER_DONE;
}
INTERN DREF DeeObject *
(DCALL S(DeeType_CallIInstanceGetSetAttrKw,
         DeeType_CallIInstanceGetSetAttrLenKw))(DeeTypeObject *__restrict tp_invoker,
                                                DeeTypeObject *__restrict tp_self,
                                                ATTR_ARG, dhash_t hash,
                                                size_t argc, DeeObject **__restrict argv,
                                                DeeObject *kw) {
 struct type_getset *chain = tp_self->tp_getsets;
 for (; chain->gs_name; ++chain) {
  if (!NAMEEQ(chain->gs_name)) continue;
  membercache_addgetset(&tp_invoker->tp_cache,tp_self,hash,chain);
  return type_obprop_call_kw(tp_self,chain,argc, argv, kw);
 }
 return ITER_DONE;
}



INTERN int DCALL /* BOUND_GETSET */
N_len(type_getset_boundattr)(struct membercache *__restrict cache, DeeTypeObject *__restrict decl,
                             struct type_getset *__restrict chain, DeeObject *__restrict self,
                             ATTR_ARG, dhash_t hash) {
 DREF DeeObject *temp;
 for (; chain->gs_name; ++chain) {
  if (!NAMEEQ(chain->gs_name)) continue;
  membercache_addgetset(cache,decl,hash,chain);
  if unlikely(!chain->gs_get) return 0;
  temp = (*chain->gs_get)(self);
  if likely(temp) { Dee_Decref(temp); return 1; }
  if (CATCH_ATTRIBUTE_ERROR())
      return -3;
  if (DeeError_Catch(&DeeError_UnboundAttribute))
      return 0;
  return -1;
 }
 return -2;
}

INTERN int DCALL /* DEL_GETSET */
N_len(type_getset_delattr)(struct membercache *__restrict cache, DeeTypeObject *__restrict decl,
                           struct type_getset *__restrict chain, DeeObject *__restrict self,
                           ATTR_ARG, dhash_t hash) {
 for (; chain->gs_name; ++chain) {
  if (!NAMEEQ(chain->gs_name)) continue;
  membercache_addgetset(cache,decl,hash,chain);
  return type_getset_del(chain,self);
 }
 return 1;
}

INTERN int DCALL /* SET_GETSET */
N_len(type_getset_setattr)(struct membercache *__restrict cache, DeeTypeObject *__restrict decl,
                           struct type_getset *__restrict chain, DeeObject *__restrict self,
                           ATTR_ARG, dhash_t hash, DeeObject *__restrict value) {
 for (; chain->gs_name; ++chain) {
  if (!NAMEEQ(chain->gs_name)) continue;
  membercache_addgetset(cache,decl,hash,chain);
  return type_getset_set(chain,self,value);
 }
 return 1;
}


INTERN DREF DeeObject *DCALL /* GET_MEMBER */
N_len(type_member_getattr)(struct membercache *__restrict cache, DeeTypeObject *__restrict decl,
                           struct type_member *__restrict chain, DeeObject *__restrict self,
                           ATTR_ARG, dhash_t hash) {
 for (; chain->m_name; ++chain) {
  if (!NAMEEQ(chain->m_name)) continue;
  membercache_addmember(cache,decl,hash,chain);
  return type_member_get(chain,self);
 }
 return ITER_DONE;
}

INTERN DREF DeeObject *
(DCALL NLen(DeeType_GetInstanceMemberAttr))(DeeTypeObject *__restrict tp_invoker,
                                            DeeTypeObject *__restrict tp_self,
                                            ATTR_ARG, dhash_t hash) {
 struct type_member *chain = tp_self->tp_members;
 for (; chain->m_name; ++chain) {
  if (!NAMEEQ(chain->m_name)) continue;
  membercache_addinstancemember(&tp_invoker->tp_class_cache,tp_self,hash,chain);
  return type_obmemb_get(tp_self,chain);
 }
 return ITER_DONE;
}
INTERN DREF DeeObject *
(DCALL NLen(DeeType_GetIInstanceMemberAttr))(DeeTypeObject *__restrict tp_invoker,
                                             DeeTypeObject *__restrict tp_self,
                                             ATTR_ARG, dhash_t hash) {
 struct type_member *chain = tp_self->tp_members;
 for (; chain->m_name; ++chain) {
  if (!NAMEEQ(chain->m_name)) continue;
  membercache_addmember(&tp_invoker->tp_cache,tp_self,hash,chain);
  return type_obmemb_get(tp_self,chain);
 }
 return ITER_DONE;
}
INTERN DREF DeeObject *
(DCALL NLen(DeeType_CallInstanceMemberAttr))(DeeTypeObject *__restrict tp_invoker,
                                             DeeTypeObject *__restrict tp_self,
                                             ATTR_ARG, dhash_t hash,
                                             size_t argc, DeeObject **__restrict argv) {
 struct type_member *chain = tp_self->tp_members;
 for (; chain->m_name; ++chain) {
  if (!NAMEEQ(chain->m_name)) continue;
  membercache_addinstancemember(&tp_invoker->tp_class_cache,tp_self,hash,chain);
  return type_obmemb_call(tp_self,chain,argc, argv);
 }
 return ITER_DONE;
}
#if 0
INTERN DREF DeeObject *
(DCALL NLen(DeeType_CallIInstanceMemberAttr))(DeeTypeObject *__restrict tp_invoker,
                                              DeeTypeObject *__restrict tp_self,
                                              ATTR_ARG, dhash_t hash,
                                              size_t argc, DeeObject **__restrict argv) {
 struct type_member *chain = tp_self->tp_members;
 for (; chain->m_name; ++chain) {
  if (!NAMEEQ(chain->m_name)) continue;
  membercache_addmember(&tp_invoker->tp_cache,tp_self,hash,chain);
  return type_obmemb_call(tp_self,chain,argc, argv);
 }
 return ITER_DONE;
}
#endif
INTERN DREF DeeObject *
(DCALL S(DeeType_CallInstanceMemberAttrKw,
         DeeType_CallInstanceMemberAttrLenKw))(DeeTypeObject *__restrict tp_invoker,
                                               DeeTypeObject *__restrict tp_self,
                                               ATTR_ARG, dhash_t hash,
                                               size_t argc, DeeObject **__restrict argv,
                                               DeeObject *kw) {
 struct type_member *chain = tp_self->tp_members;
 for (; chain->m_name; ++chain) {
  if (!NAMEEQ(chain->m_name)) continue;
  membercache_addinstancemember(&tp_invoker->tp_class_cache,tp_self,hash,chain);
  return type_obmemb_call_kw(tp_self,chain,argc, argv, kw);
 }
 return ITER_DONE;
}
INTERN DREF DeeObject *
(DCALL S(DeeType_CallIInstanceMemberAttrKw,
         DeeType_CallIInstanceMemberAttrLenKw))(DeeTypeObject *__restrict tp_invoker,
                                                DeeTypeObject *__restrict tp_self,
                                                ATTR_ARG, dhash_t hash,
                                                size_t argc, DeeObject **__restrict argv,
                                                DeeObject *kw) {
 struct type_member *chain = tp_self->tp_members;
 for (; chain->m_name; ++chain) {
  if (!NAMEEQ(chain->m_name)) continue;
  membercache_addmember(&tp_invoker->tp_cache,tp_self,hash,chain);
  return type_obmemb_call_kw(tp_self,chain,argc, argv, kw);
 }
 return ITER_DONE;
}




INTERN int DCALL /* BOUND_MEMBER */
N_len(type_member_boundattr)(struct membercache *__restrict cache, DeeTypeObject *__restrict decl,
                             struct type_member *__restrict chain, DeeObject *__restrict self,
                             ATTR_ARG, dhash_t hash) {
 for (; chain->m_name; ++chain) {
  if (!NAMEEQ(chain->m_name)) continue;
  membercache_addmember(cache,decl,hash,chain);
  return type_member_bound(chain,self);
 }
 return -2;
}

INTERN int DCALL /* DEL_MEMBER */
N_len(type_member_delattr)(struct membercache *__restrict cache, DeeTypeObject *__restrict decl,
                           struct type_member *__restrict chain, DeeObject *__restrict self,
                           ATTR_ARG, dhash_t hash) {
 for (; chain->m_name; ++chain) {
  if (!NAMEEQ(chain->m_name)) continue;
  membercache_addmember(cache,decl,hash,chain);
  return type_member_del(chain,self);
 }
 return 1;
}

INTERN int DCALL /* SET_MEMBER */
N_len(type_member_setattr)(struct membercache *__restrict cache, DeeTypeObject *__restrict decl,
                           struct type_member *__restrict chain, DeeObject *__restrict self,
                           ATTR_ARG, dhash_t hash, DeeObject *__restrict value) {
 for (; chain->m_name; ++chain) {
  if (!NAMEEQ(chain->m_name)) continue;
  membercache_addmember(cache,decl,hash,chain);
  return type_member_set(chain,self,value);
 }
 return 1;
}


INTERN bool DCALL /* METHOD */
N_len(type_method_hasattr)(struct membercache *__restrict cache,
                           DeeTypeObject *__restrict decl,
                           struct type_method *__restrict chain,
                           ATTR_ARG, dhash_t hash) {
 for (; chain->m_name; ++chain) {
  if (!NAMEEQ(chain->m_name)) continue;
  membercache_addmethod(cache,decl,hash,chain);
  return true;
 }
 return false;
}
INTERN bool
(DCALL NLen(DeeType_HasInstanceMethodAttr))(DeeTypeObject *__restrict tp_invoker,
                                            DeeTypeObject *__restrict tp_self,
                                            ATTR_ARG, dhash_t hash) {
 struct type_method *chain = tp_self->tp_methods;
 for (; chain->m_name; ++chain) {
  if (!NAMEEQ(chain->m_name)) continue;
  membercache_addinstancemethod(&tp_invoker->tp_class_cache,tp_self,hash,chain);
  return true;
 }
 return false;
}

INTERN bool DCALL /* GETSET */
N_len(type_getset_hasattr)(struct membercache *__restrict cache,
                           DeeTypeObject *__restrict decl,
                           struct type_getset *__restrict chain,
                           ATTR_ARG, dhash_t hash) {
 for (; chain->gs_name; ++chain) {
  if (!NAMEEQ(chain->gs_name)) continue;
  membercache_addgetset(cache,decl,hash,chain);
  return true;
 }
 return false;
}
INTERN bool
(DCALL NLen(DeeType_HasInstanceGetSetAttr))(DeeTypeObject *__restrict tp_invoker,
                                            DeeTypeObject *__restrict tp_self,
                                            ATTR_ARG,
                                            dhash_t hash) {
 struct type_getset *chain = tp_self->tp_getsets;
 for (; chain->gs_name; ++chain) {
  if (!NAMEEQ(chain->gs_name)) continue;
  membercache_addinstancegetset(&tp_invoker->tp_class_cache,tp_self,hash,chain);
  return true;
 }
 return false;
}

INTERN bool DCALL /* MEMBER */
N_len(type_member_hasattr)(struct membercache *__restrict cache,
                           DeeTypeObject *__restrict decl,
                           struct type_member *__restrict chain,
                           ATTR_ARG, dhash_t hash) {
 for (; chain->m_name; ++chain) {
  if (!NAMEEQ(chain->m_name)) continue;
  membercache_addmember(cache,decl,hash,chain);
  return true;
 }
 return false;
}
INTERN bool
(DCALL NLen(DeeType_HasInstanceMemberAttr))(DeeTypeObject *__restrict tp_invoker,
                                            DeeTypeObject *__restrict tp_self,
                                            ATTR_ARG,
                                            dhash_t hash) {
 struct type_member *chain = tp_self->tp_members;
 for (; chain->m_name; ++chain) {
  if (!NAMEEQ(chain->m_name)) continue;
  membercache_addinstancemember(&tp_invoker->tp_class_cache,tp_self,hash,chain);
  return true;
 }
 return false;
}


#ifndef MRO_LEN
INTERN int DCALL /* METHOD */
type_method_findattr(struct membercache *__restrict cache, DeeTypeObject *__restrict decl,
                     struct type_method *__restrict chain, uint16_t perm,
                     struct attribute_info *__restrict result,
                     struct attribute_lookup_rules const *__restrict rules) {
 ASSERT(perm & (ATTR_IMEMBER | ATTR_CMEMBER));
 perm |= ATTR_PERMGET | ATTR_PERMCALL;
 if ((perm & rules->alr_perm_mask) != rules->alr_perm_value)
      goto nope;
 for (; chain->m_name; ++chain) {
  if (!streq(chain->m_name,rules->alr_name)) continue;
  membercache_addmethod(cache,decl,rules->alr_hash,chain);
  result->a_doc      = chain->m_doc;
  result->a_decl     = (DREF DeeObject *)decl;
  result->a_perm     = perm;
  result->a_attrtype = (chain->m_flag & TYPE_METHOD_FKWDS)
                     ? &DeeKwObjMethod_Type
                     : &DeeObjMethod_Type
                     ;
  Dee_Incref(result->a_attrtype);
  Dee_Incref(decl);
  return 0;
 }
nope:
 return 1;
}
INTERN int DCALL
DeeType_FindInstanceMethodAttr(DeeTypeObject *__restrict tp_invoker,
                               DeeTypeObject *__restrict tp_self,
                               struct attribute_info *__restrict result,
                               struct attribute_lookup_rules const *__restrict rules) {
 uint16_t perm; struct type_method *chain;
 perm = ATTR_IMEMBER | ATTR_CMEMBER | ATTR_PERMGET | ATTR_PERMCALL | ATTR_WRAPPER;
 if ((perm & rules->alr_perm_mask) != rules->alr_perm_value)
      goto nope;
 chain = tp_self->tp_methods;
 for (; chain->m_name; ++chain) {
  if (!streq(chain->m_name,rules->alr_name)) continue;
  membercache_addinstancemethod(&tp_invoker->tp_class_cache,tp_self,rules->alr_hash,chain);
  result->a_doc      = chain->m_doc;
  result->a_decl     = (DREF DeeObject *)tp_self;
  result->a_perm     = perm;
  result->a_attrtype = (chain->m_flag & TYPE_METHOD_FKWDS)
                     ? &DeeKwObjMethod_Type
                     : &DeeObjMethod_Type
                     ;
  Dee_Incref(result->a_attrtype);
  Dee_Incref(tp_self);
  return 0;
 }
nope:
 return 1;
}

INTERN int DCALL /* GETSET */
type_getset_findattr(struct membercache *__restrict cache, DeeTypeObject *__restrict decl,
                     struct type_getset *__restrict chain, uint16_t perm,
                     struct attribute_info *__restrict result,
                     struct attribute_lookup_rules const *__restrict rules) {
 ASSERT(perm & (ATTR_IMEMBER | ATTR_CMEMBER));
 perm |= ATTR_PROPERTY;
 for (; chain->gs_name; ++chain) {
  uint16_t flags = perm;
  if (chain->gs_get) flags |= ATTR_PERMGET;
  if (chain->gs_del) flags |= ATTR_PERMDEL;
  if (chain->gs_set) flags |= ATTR_PERMSET;
  if ((flags & rules->alr_perm_mask) != rules->alr_perm_value) continue;
  if (!streq(chain->gs_name,rules->alr_name)) continue;
  membercache_addgetset(cache,decl,rules->alr_hash,chain);
  result->a_doc      = chain->gs_doc;
  result->a_perm     = flags;
  result->a_decl     = (DREF DeeObject *)decl;
  result->a_attrtype = NULL;
  Dee_Incref(decl);
  return 0;
 }
 return 1;
}
INTERN int DCALL
DeeType_FindInstanceGetSetAttr(DeeTypeObject *__restrict tp_invoker,
                               DeeTypeObject *__restrict tp_self,
                               struct attribute_info *__restrict result,
                               struct attribute_lookup_rules const *__restrict rules) {
 uint16_t perm; struct type_getset *chain;
 perm = ATTR_PROPERTY | ATTR_WRAPPER | ATTR_IMEMBER | ATTR_CMEMBER;
 chain = tp_self->tp_getsets;
 for (; chain->gs_name; ++chain) {
  uint16_t flags = perm;
  if (chain->gs_get) flags |= ATTR_PERMGET;
  if (chain->gs_del) flags |= ATTR_PERMDEL;
  if (chain->gs_set) flags |= ATTR_PERMSET;
  if ((flags & rules->alr_perm_mask) != rules->alr_perm_value) continue;
  if (!streq(chain->gs_name,rules->alr_name)) continue;
  membercache_addinstancegetset(&tp_invoker->tp_class_cache,tp_self,rules->alr_hash,chain);
  result->a_doc      = chain->gs_doc;
  result->a_perm     = flags;
  result->a_decl     = (DREF DeeObject *)tp_self;
  result->a_attrtype = NULL;
  Dee_Incref(tp_self);
  return 0;
 }
 return 1;
}

INTERN int DCALL /* MEMBER */
type_member_findattr(struct membercache *__restrict cache, DeeTypeObject *__restrict decl,
                     struct type_member *__restrict chain, uint16_t perm,
                     struct attribute_info *__restrict result,
                     struct attribute_lookup_rules const *__restrict rules) {
 ASSERT(perm & (ATTR_IMEMBER | ATTR_CMEMBER));
 perm |= ATTR_PERMGET;
 for (; chain->m_name; ++chain) {
  uint16_t flags = perm;
  if (!TYPE_MEMBER_ISCONST(chain) &&
      !(chain->m_field.m_type & STRUCT_CONST))
      flags |= (ATTR_PERMDEL | ATTR_PERMSET);
  if ((flags & rules->alr_perm_mask) != rules->alr_perm_value) continue;
  if (!streq(chain->m_name,rules->alr_name)) continue;
  membercache_addmember(cache,decl,rules->alr_hash,chain);
  result->a_doc      = chain->m_doc;
  result->a_perm     = flags;
  result->a_decl     = (DREF DeeObject *)decl;
  result->a_attrtype = type_member_typefor(chain);
  Dee_Incref(decl);
  Dee_XIncref(result->a_attrtype);
  return 0;
 }
 return 1;
}
INTERN int DCALL
DeeType_FindInstanceMemberAttr(DeeTypeObject *__restrict tp_invoker,
                               DeeTypeObject *__restrict tp_self,
                               struct attribute_info *__restrict result,
                               struct attribute_lookup_rules const *__restrict rules) {
 uint16_t perm; struct type_member *chain;
 perm = ATTR_WRAPPER | ATTR_IMEMBER | ATTR_CMEMBER | ATTR_PERMGET;
 chain = tp_self->tp_members;
 for (; chain->m_name; ++chain) {
  uint16_t flags = perm;
  if (!TYPE_MEMBER_ISCONST(chain) &&
      !(chain->m_field.m_type & STRUCT_CONST))
      flags |= (ATTR_PERMDEL | ATTR_PERMSET);
  if ((flags & rules->alr_perm_mask) != rules->alr_perm_value) continue;
  if (!streq(chain->m_name,rules->alr_name)) continue;
  membercache_addinstancemember(&tp_invoker->tp_class_cache,tp_self,rules->alr_hash,chain);
  result->a_doc      = chain->m_doc;
  result->a_perm     = flags;
  result->a_decl     = (DREF DeeObject *)tp_self;
  result->a_attrtype = type_member_typefor(chain);
  Dee_Incref(tp_self);
  Dee_XIncref(result->a_attrtype);
  return 0;
 }
 return 1;
}
#endif /* !MRO_LEN */

DECL_END

#undef S
#undef N_len
#undef NLen
#undef IF_LEN
#undef IF_NLEN
#undef ATTR_ARG
#undef ATTREQ
#undef NAMEEQ
#undef MRO_LEN
