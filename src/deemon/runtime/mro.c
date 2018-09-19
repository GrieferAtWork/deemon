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
#ifndef GUARD_DEEMON_RUNTIME_MRO_C
#define GUARD_DEEMON_RUNTIME_MRO_C 1

#include <deemon/api.h>
#include <deemon/object.h>
#include <deemon/mro.h>
#include <deemon/objmethod.h>
#include <deemon/class.h>
#include <deemon/file.h>
#include <deemon/instancemethod.h>
#include <deemon/arg.h>
#include <deemon/none.h>
#include <deemon/string.h>
#include <deemon/attribute.h>
#include <deemon/error.h>
#include <deemon/tuple.h>

#include "runtime_error.h"

DECL_BEGIN

#ifndef CONFIG_NO_THREADS
PRIVATE DEFINE_RWLOCK(membercache_list_lock);
#endif
/* [0..1][lock(membercache_list_lock)]
 *  Linked list of all existing type-member caches. */
PRIVATE struct membercache *membercache_list;


INTERN void DCALL
membercache_fini(struct membercache *__restrict self) {
 MEMBERCACHE_WRITE(self);
 ASSERT((self->mc_table != NULL) ==
        (self->mc_pself != NULL));
 if (!self->mc_table) {
  MEMBERCACHE_ENDWRITE(self);
  return;
 }
 Dee_Free(self->mc_table);
 self->mc_table = NULL;
 MEMBERCACHE_ENDWRITE(self);
#ifndef CONFIG_NO_THREADS
 COMPILER_READ_BARRIER();
 rwlock_write(&membercache_list_lock);
#endif
 ASSERT(!self->mc_table);
 /* Check check `mc_pself != NULL' again because another thread
  * may have cleared the tables of all member caches while collecting
  * memory, in the process unlinking all of them from the global chain. */
 if (self->mc_pself) {
  if ((*self->mc_pself = self->mc_next) != NULL)
        self->mc_next->mc_pself = self->mc_pself;
 }
#ifndef CONFIG_NO_THREADS
 rwlock_endwrite(&membercache_list_lock);
#endif
}

INTERN size_t DCALL
membercache_clear(size_t max_clear) {
 size_t result = 0;
 struct membercache *entry;
#ifndef CONFIG_NO_THREADS
 rwlock_write(&membercache_list_lock);
#endif
 while ((entry = membercache_list) != NULL) {
  MEMBERCACHE_WRITE(entry);
  /* Pop this entry from the global chain. */
  if ((membercache_list = entry->mc_next) != NULL) {
   membercache_list->mc_pself = &membercache_list;
  }
  if (entry->mc_table) {
   /* Track how much member this operation will be freeing up. */
   result += (entry->mc_mask + 1) * sizeof(struct membercache);
   /* Clear this entry's table. */
   Dee_Free(entry->mc_table);
   entry->mc_table = NULL;
   entry->mc_mask  = 0;
   entry->mc_size  = 0;
  }
  entry->mc_pself = NULL;
#ifndef NDEBUG
  memset(&entry->mc_next,0xcc,sizeof(void *));
#endif
  MEMBERCACHE_ENDWRITE(entry);
  if (result >= max_clear)
      break;
 }
#ifndef CONFIG_NO_THREADS
 rwlock_endwrite(&membercache_list_lock);
#endif
 return result;
}

STATIC_ASSERT(MEMBERCACHE_UNUSED == 0);
PRIVATE ATTR_NOINLINE bool DCALL
membercache_rehash(struct membercache *__restrict self) {
 struct membercache_slot *new_vector,*iter,*end;
 size_t new_mask = self->mc_mask;
 new_mask = (new_mask << 1)|1;
 if unlikely(new_mask == 1) new_mask = 16-1; /* Start out bigger than 2. */
 ASSERT(self->mc_size < new_mask);
 new_vector = (struct membercache_slot *)Dee_TryCalloc((new_mask+1)*sizeof(struct membercache_slot));
 if unlikely(!new_vector) return false;
 ASSERT((self->mc_table == NULL) == (self->mc_size == 0));
 ASSERT((self->mc_table == NULL) == (self->mc_mask == 0));
 if (self->mc_table == NULL) {
  /* This is the first time that this cache is being rehashed.
   * >> Register it in the global chain of member caches,
   *    so we can clear it when memory gets low. */
#ifndef CONFIG_NO_THREADS
  rwlock_write(&membercache_list_lock);
#endif
  if ((self->mc_next = membercache_list) != NULL)
       membercache_list->mc_pself = &self->mc_next;
  self->mc_pself   = &membercache_list;
  membercache_list = self;
#ifndef CONFIG_NO_THREADS
  rwlock_endwrite(&membercache_list_lock);
#endif
 } else {
  /* Re-insert all existing items into the new table vector. */
  end = (iter = self->mc_table)+(self->mc_mask+1);
  for (; iter != end; ++iter) {
   struct membercache_slot *item;
   dhash_t i,perturb;
   /* Skip unused entires. */
   if (iter->mcs_type == MEMBERCACHE_UNUSED) continue;
   perturb = i = iter->mcs_hash & new_mask;
   for (;; i = MEMBERCACHE_HASHNX(i,perturb),MEMBERCACHE_HASHPT(perturb)) {
    item = &new_vector[i & new_mask];
    if (item->mcs_type == MEMBERCACHE_UNUSED) break; /* Empty slot found. */
   }
   /* Transfer this object. */
   memcpy(item,iter,sizeof(struct membercache_slot));
  }
  Dee_Free(self->mc_table);
 }
 self->mc_mask  = new_mask;
 self->mc_table = new_vector;
 return true;
}


#ifdef CONFIG_USE_NEW_TYPE_ATTRIBUTE_CACHING

/* Since this cache is totally optional, don't slow down when we can't get the lock. */
#ifdef CONFIG_NO_THREADS
#define MEMBERCACHE_TRYWRITE_OR_RETURN(self) (void)0
#elif 1
#define MEMBERCACHE_TRYWRITE_OR_RETURN(self) \
        if (!rwlock_trywrite(&(self)->mc_lock)) return
#else
#define MEMBERCACHE_TRYWRITE_OR_RETURN(self) \
        MEMBERCACHE_WRITE(self)
#endif


#define MEMBERCACHE_ADDENTRY(name,init) \
{ \
 dhash_t i,perturb; \
 struct membercache_slot *item; \
 MEMBERCACHE_TRYWRITE_OR_RETURN(self); \
again: \
 if (!self->mc_table) goto rehash_initial; \
 /* Re-check that the named attribute isn't already in-cache. */ \
 perturb = i = MEMBERCACHE_HASHST(self,hash); \
 for (;; i = MEMBERCACHE_HASHNX(i,perturb),MEMBERCACHE_HASHPT(perturb)) { \
  item = MEMBERCACHE_HASHIT(self,i); \
  if (item->mcs_type == MEMBERCACHE_UNUSED) break; \
  if (item->mcs_hash != hash) continue; \
  if (item->mcs_method.m_name != (name) && \
      strcmp(item->mcs_name,(name)) != 0) \
      continue; \
  /* Already in cache! */ \
  MEMBERCACHE_ENDWRITE(self); \
  return; \
 } \
 if (self->mc_size+1 >= self->mc_mask) { \
rehash_initial: \
  if (membercache_rehash(self)) goto again; \
  goto done; /* Well... We couldn't rehash the cache so we can't add this entry. */ \
 } \
 /* Not found. - Use this empty slot. */ \
 item->mcs_hash   = hash; \
 item->mcs_decl   = decl; \
 init; \
 ++self->mc_size; \
 /* Try to keep the table vector big at least twice as big as the element count. */ \
 if (self->mc_size*2 > self->mc_mask) \
     membercache_rehash(self); \
done: \
 MEMBERCACHE_ENDWRITE(self); \
}

#define PRIVATE_IS_KNOWN_TYPETYPE(x) ((x) == &DeeType_Type || (x) == &DeeFileType_Type)
#define MEMBERCACHE_GETTYPENAME(x) \
 (PRIVATE_IS_KNOWN_TYPETYPE(COMPILER_CONTAINER_OF(x,DeeTypeObject,tp_cache)->ob_type) ? \
  COMPILER_CONTAINER_OF(x,DeeTypeObject,tp_cache)->tp_name : \
  PRIVATE_IS_KNOWN_TYPETYPE(COMPILER_CONTAINER_OF(x,DeeTypeObject,tp_class_cache)->ob_type) ? \
  COMPILER_CONTAINER_OF(x,DeeTypeObject,tp_class_cache)->tp_name : "?") \


INTERN void DCALL
membercache_addmethod(struct membercache *__restrict self,
                      DeeTypeObject *__restrict decl, dhash_t hash,
                      struct type_method const *__restrict method) {
 DEE_DPRINTF("[RT] Caching method `%s.%s' in `%s'\n",
             decl->tp_name,method->m_name,
             MEMBERCACHE_GETTYPENAME(self));
 MEMBERCACHE_ADDENTRY(method->m_name,{
  item->mcs_type   = MEMBERCACHE_METHOD;
  item->mcs_method = *method;
 });
}
INTERN void DCALL
membercache_addinstancemethod(struct membercache *__restrict self,
                              DeeTypeObject *__restrict decl, dhash_t hash,
                              struct type_method const *__restrict method) {
 DEE_DPRINTF("[RT] Caching instance_method `%s.%s' in `%s'\n",
             decl->tp_name,method->m_name,
             MEMBERCACHE_GETTYPENAME(self));
 ASSERT(self != &decl->tp_cache);
 MEMBERCACHE_ADDENTRY(method->m_name,{
  item->mcs_type   = MEMBERCACHE_INSTANCE_METHOD;
  item->mcs_method = *method;
 });
}

INTERN void DCALL
membercache_addgetset(struct membercache *__restrict self,
                      DeeTypeObject *__restrict decl, dhash_t hash,
                      struct type_getset const *__restrict getset) {
 DEE_DPRINTF("[RT] Caching getset `%s.%s' in `%s'\n",
             decl->tp_name,getset->gs_name,
             MEMBERCACHE_GETTYPENAME(self));
 MEMBERCACHE_ADDENTRY(getset->gs_name,{
  item->mcs_type   = MEMBERCACHE_GETSET;
  item->mcs_getset = *getset;
 });
}
INTERN void DCALL
membercache_addinstancegetset(struct membercache *__restrict self,
                              DeeTypeObject *__restrict decl, dhash_t hash,
                              struct type_getset const *__restrict getset) {
 DEE_DPRINTF("[RT] Caching instance_getset `%s.%s' in `%s'\n",
             decl->tp_name,getset->gs_name,
             MEMBERCACHE_GETTYPENAME(self));
 ASSERT(self != &decl->tp_cache);
 MEMBERCACHE_ADDENTRY(getset->gs_name,{
  item->mcs_type   = MEMBERCACHE_INSTANCE_GETSET;
  item->mcs_getset = *getset;
 });
}

INTERN void DCALL
membercache_addmember(struct membercache *__restrict self,
                      DeeTypeObject *__restrict decl, dhash_t hash,
                      struct type_member const *__restrict member) {
 DEE_DPRINTF("[RT] Caching member `%s.%s' in `%s'\n",
             decl->tp_name,member->m_name,
             MEMBERCACHE_GETTYPENAME(self));
 MEMBERCACHE_ADDENTRY(member->m_name,{
  item->mcs_type   = MEMBERCACHE_MEMBER;
  item->mcs_member = *member;
 });
}
INTERN void DCALL
membercache_addinstancemember(struct membercache *__restrict self,
                              DeeTypeObject *__restrict decl, dhash_t hash,
                              struct type_member const *__restrict member) {
 DEE_DPRINTF("[RT] Caching instance_member `%s.%s' in `%s'\n",
             decl->tp_name,member->m_name,
             MEMBERCACHE_GETTYPENAME(self));
 ASSERT(self != &decl->tp_cache);
 MEMBERCACHE_ADDENTRY(member->m_name,{
  item->mcs_type   = MEMBERCACHE_INSTANCE_MEMBER;
  item->mcs_member = *member;
 });
}

INTERN void DCALL
membercache_addattrib(struct membercache *__restrict self,
                      DeeTypeObject *__restrict decl, dhash_t hash,
                      struct class_attribute *__restrict attrib) {
 char const *name = DeeString_STR(attrib->ca_name);
 DEE_DPRINTF("[RT] Caching attribute `%s.%s' in `%s'\n",
             decl->tp_name,name,
             MEMBERCACHE_GETTYPENAME(self));
 MEMBERCACHE_ADDENTRY(name,{
  item->mcs_type          = MEMBERCACHE_ATTRIB;
  item->mcs_attrib.a_name = name;
  item->mcs_attrib.a_attr = attrib;
  item->mcs_attrib.a_desc = DeeClass_DESC(decl);
 });
}
INTERN void DCALL
membercache_addinstanceattrib(struct membercache *__restrict self,
                              DeeTypeObject *__restrict decl, dhash_t hash,
                              struct class_attribute *__restrict attrib) {
 char const *name = DeeString_STR(attrib->ca_name);
 DEE_DPRINTF("[RT] Caching instance_attribute `%s.%s' in `%s'\n",
             decl->tp_name,name,
             MEMBERCACHE_GETTYPENAME(self));
 ASSERT(self != &decl->tp_cache);
 MEMBERCACHE_ADDENTRY(name,{
  item->mcs_type          = MEMBERCACHE_INSTANCE_ATTRIB;
  item->mcs_attrib.a_name = name;
  item->mcs_attrib.a_attr = attrib;
  item->mcs_attrib.a_desc = DeeClass_DESC(decl);
 });
}

/* Lookup an attribute from cache.
 * @return: * :        The attribute value.
 * @return: NULL:      An error occurred.
 * @return: ITER_DONE: The attribute could not be found in the cache. */
INTERN DREF DeeObject *DCALL
DeeType_GetCachedAttr(DeeTypeObject *__restrict tp_self,
                      DeeObject *__restrict self,
                      char const *__restrict name, dhash_t hash) {
 dhash_t i,perturb;
 MEMBERCACHE_READ(&tp_self->tp_cache);
 if unlikely(!tp_self->tp_cache.mc_table) goto done;
 perturb = i = MEMBERCACHE_HASHST(&tp_self->tp_cache,hash);
 for (;; i = MEMBERCACHE_HASHNX(i,perturb),MEMBERCACHE_HASHPT(perturb)) {
  struct membercache_slot *item = MEMBERCACHE_HASHIT(&tp_self->tp_cache,i);
  if (item->mcs_type == MEMBERCACHE_UNUSED) break;
  if (item->mcs_hash != hash) continue;
  if (strcmp(item->mcs_name,name)) continue;
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
   err_cant_access_attribute(type,name,ATTR_ACCESS_GET);
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
   struct class_attribute *attr;
   struct class_desc *desc;
  case MEMBERCACHE_ATTRIB:
   attr = item->mcs_attrib.a_attr;
   desc = item->mcs_attrib.a_desc;
   MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
   return DeeInstance_GetAttribute(desc,
                                   DeeInstance_DESC(desc,self),
                                   self,attr);
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
DeeType_GetCachedClassAttr(DeeTypeObject *__restrict tp_self,
                           char const *__restrict name, dhash_t hash) {
 dhash_t i,perturb;
 MEMBERCACHE_READ(&tp_self->tp_class_cache);
 if unlikely(!tp_self->tp_class_cache.mc_table) goto done;
 perturb = i = MEMBERCACHE_HASHST(&tp_self->tp_class_cache,hash);
 for (;; i = MEMBERCACHE_HASHNX(i,perturb),MEMBERCACHE_HASHPT(perturb)) {
  struct membercache_slot *item = MEMBERCACHE_HASHIT(&tp_self->tp_class_cache,i);
  if (item->mcs_type == MEMBERCACHE_UNUSED) break;
  if (item->mcs_hash != hash) continue;
  if (strcmp(item->mcs_name,name)) continue;
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
   err_cant_access_attribute(type,name,ATTR_ACCESS_GET);
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
   struct class_attribute *attr;
   DeeTypeObject *type;
   struct class_desc *desc;
  case MEMBERCACHE_ATTRIB:
   attr = item->mcs_attrib.a_attr;
   desc = item->mcs_attrib.a_desc;
   type = item->mcs_decl;
   MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
   return DeeInstance_GetAttribute(desc,class_desc_as_instance(desc),(DeeObject *)tp_self,attr);
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
   struct class_attribute *attr;
   DeeTypeObject *type;
  case MEMBERCACHE_INSTANCE_ATTRIB:
   attr = item->mcs_attrib.a_attr;
   type = item->mcs_decl;
   MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
   return DeeClass_GetInstanceAttribute(type,attr);
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
DeeType_GetCachedInstanceAttr(DeeTypeObject *__restrict tp_self,
                              char const *__restrict name, dhash_t hash) {
 dhash_t i,perturb;
 MEMBERCACHE_READ(&tp_self->tp_cache);
 if unlikely(!tp_self->tp_cache.mc_table) goto done;
 perturb = i = MEMBERCACHE_HASHST(&tp_self->tp_cache,hash);
 for (;; i = MEMBERCACHE_HASHNX(i,perturb),MEMBERCACHE_HASHPT(perturb)) {
  struct membercache_slot *item = MEMBERCACHE_HASHIT(&tp_self->tp_cache,i);
  if (item->mcs_type == MEMBERCACHE_UNUSED) break;
  if (item->mcs_hash != hash) continue;
  if (strcmp(item->mcs_name,name)) continue;
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
   struct class_attribute *attr;
   DeeTypeObject *type;
  case MEMBERCACHE_ATTRIB:
   attr = item->mcs_attrib.a_attr;
   type = item->mcs_decl;
   MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
   return DeeClass_GetInstanceAttribute(type,attr);
  }
  default: __builtin_unreachable();
  }
 }
done:
 MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
 return ITER_DONE;
}


INTERN DREF DeeObject *DCALL
membercache_docattr(struct membercache *__restrict self,
                    char const *__restrict name, dhash_t hash) {
 dhash_t i,perturb; struct membercache_slot *item;
 MEMBERCACHE_READ(self);
 if unlikely(!self->mc_table) goto done;
 perturb = i = MEMBERCACHE_HASHST(self,hash);
 for (;; i = MEMBERCACHE_HASHNX(i,perturb),MEMBERCACHE_HASHPT(perturb)) {
  char const *c_string;
  item = MEMBERCACHE_HASHIT(self,i);
  if (item->mcs_type == MEMBERCACHE_UNUSED) break;
  if (item->mcs_hash != hash) continue;
  if (strcmp(item->mcs_name,name)) continue;
  switch (item->mcs_type) {
  case MEMBERCACHE_METHOD:
  case MEMBERCACHE_INSTANCE_METHOD:
   c_string = item->mcs_method.m_doc;
return_c_string:
   if (!c_string) goto err_no_doc;
   MEMBERCACHE_ENDREAD(self);
   return DeeString_NewUtf8(c_string,
                            strlen(c_string),
                            STRING_ERROR_FIGNORE);
  case MEMBERCACHE_GETSET:
  case MEMBERCACHE_INSTANCE_GETSET:
   c_string = item->mcs_getset.gs_doc;
   goto return_c_string;
  case MEMBERCACHE_MEMBER:
  case MEMBERCACHE_INSTANCE_MEMBER:
   c_string = item->mcs_getset.gs_doc;
   goto return_c_string;

  {
   DREF DeeStringObject *result;
  case MEMBERCACHE_ATTRIB:
  case MEMBERCACHE_INSTANCE_ATTRIB:
   result = item->mcs_attrib.a_attr->ca_doc;
   if unlikely(!result) goto err_no_doc;
   Dee_Incref(result);
   MEMBERCACHE_ENDREAD(self);
   return (DREF DeeObject *)result;
  }

  default: __builtin_unreachable();
  }
 }
done:
 MEMBERCACHE_ENDREAD(self);
 return ITER_DONE;
err_no_doc:
 {
  DeeTypeObject *base = item->mcs_decl;
  MEMBERCACHE_ENDREAD(self);
  err_nodoc_attribute(base->tp_name,name);
 }
 return NULL;
}

/* @return: * :        The attribute doc.
 * @return: NULL:      An error occurred.
 * @return: ITER_DONE: The attribute could not be found in the cache. */
INTERN DREF DeeObject *DCALL
DeeType_DocCachedAttr(DeeTypeObject *__restrict tp_self,
                      char const *__restrict name, dhash_t hash) {
 return membercache_docattr(&tp_self->tp_cache,name,hash);
}
INTERN DREF DeeObject *DCALL
DeeType_DocCachedClassAttr(DeeTypeObject *__restrict tp_self,
                           char const *__restrict name, dhash_t hash) {
 return membercache_docattr(&tp_self->tp_class_cache,name,hash);
}


/* @return: 1 : Attribute is bound.
 * @return: 0 : Attribute isn't bound.
 * @return: -1: An error occurred.
 * @return: -2: The attribute doesn't exist. */
INTERN int DCALL
DeeType_BoundCachedAttr(DeeTypeObject *__restrict tp_self,
                        DeeObject *__restrict self,
                        char const *__restrict name, dhash_t hash) {
 dhash_t i,perturb;
 MEMBERCACHE_READ(&tp_self->tp_cache);
 if unlikely(!tp_self->tp_cache.mc_table) goto done;
 perturb = i = MEMBERCACHE_HASHST(&tp_self->tp_cache,hash);
 for (;; i = MEMBERCACHE_HASHNX(i,perturb),MEMBERCACHE_HASHPT(perturb)) {
  struct membercache_slot *item = MEMBERCACHE_HASHIT(&tp_self->tp_cache,i);
  if (item->mcs_type == MEMBERCACHE_UNUSED) break;
  if (item->mcs_hash != hash) continue;
  if (strcmp(item->mcs_name,name)) continue;
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
   struct class_attribute *attr;
   struct class_desc *desc;
  case MEMBERCACHE_ATTRIB:
   attr = item->mcs_attrib.a_attr;
   desc = item->mcs_attrib.a_desc;
   MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
   return DeeInstance_BoundAttribute(desc,
                                     DeeInstance_DESC(desc,self),
                                     self,attr);
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
DeeType_BoundCachedClassAttr(DeeTypeObject *__restrict tp_self,
                             char const *__restrict name, dhash_t hash) {
 dhash_t i,perturb;
 MEMBERCACHE_READ(&tp_self->tp_class_cache);
 if unlikely(!tp_self->tp_class_cache.mc_table) goto done;
 perturb = i = MEMBERCACHE_HASHST(&tp_self->tp_class_cache,hash);
 for (;; i = MEMBERCACHE_HASHNX(i,perturb),MEMBERCACHE_HASHPT(perturb)) {
  struct membercache_slot *item = MEMBERCACHE_HASHIT(&tp_self->tp_class_cache,i);
  if (item->mcs_type == MEMBERCACHE_UNUSED) break;
  if (item->mcs_hash != hash) continue;
  if (strcmp(item->mcs_name,name)) continue;
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
   struct class_attribute *attr;
   DeeTypeObject *type;
   struct class_desc *desc;
  case MEMBERCACHE_ATTRIB:
   attr = item->mcs_attrib.a_attr;
   desc = item->mcs_attrib.a_desc;
   type = item->mcs_decl;
   MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
   return DeeInstance_BoundAttribute(desc,class_desc_as_instance(desc),(DeeObject *)tp_self,attr);
  }
  {
   struct class_attribute *attr;
   DeeTypeObject *type;
  case MEMBERCACHE_INSTANCE_ATTRIB:
   attr = item->mcs_attrib.a_attr;
   type = item->mcs_decl;
   MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
   return DeeClass_BoundInstanceAttribute(type,attr);
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
DeeType_BoundCachedInstanceAttr(DeeTypeObject *__restrict tp_self,
                                char const *__restrict name, dhash_t hash) {
 dhash_t i,perturb;
 MEMBERCACHE_READ(&tp_self->tp_cache);
 if unlikely(!tp_self->tp_cache.mc_table) goto done;
 perturb = i = MEMBERCACHE_HASHST(&tp_self->tp_cache,hash);
 for (;; i = MEMBERCACHE_HASHNX(i,perturb),MEMBERCACHE_HASHPT(perturb)) {
  struct membercache_slot *item = MEMBERCACHE_HASHIT(&tp_self->tp_cache,i);
  if (item->mcs_type == MEMBERCACHE_UNUSED) break;
  if (item->mcs_hash != hash) continue;
  if (strcmp(item->mcs_name,name)) continue;
  switch (item->mcs_type) {
  case MEMBERCACHE_METHOD:
  case MEMBERCACHE_GETSET:
  case MEMBERCACHE_MEMBER:
   MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
   return 1;
  {
   struct class_attribute *attr;
   DeeTypeObject *type;
  case MEMBERCACHE_ATTRIB:
   attr = item->mcs_attrib.a_attr;
   type = item->mcs_decl;
   MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
   return DeeClass_BoundInstanceAttribute(type,attr);
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
DeeType_HasCachedAttr(DeeTypeObject *__restrict tp_self,
                      char const *__restrict name, dhash_t hash) {
 dhash_t i,perturb;
 MEMBERCACHE_READ(&tp_self->tp_cache);
 if unlikely(!tp_self->tp_cache.mc_table) goto done;
 perturb = i = MEMBERCACHE_HASHST(&tp_self->tp_cache,hash);
 for (;; i = MEMBERCACHE_HASHNX(i,perturb),MEMBERCACHE_HASHPT(perturb)) {
  struct membercache_slot *item = MEMBERCACHE_HASHIT(&tp_self->tp_cache,i);
  if (item->mcs_type == MEMBERCACHE_UNUSED) break;
  if (item->mcs_hash != hash) continue;
  if (strcmp(item->mcs_name,name)) continue;
  MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
  return true;
 }
done:
 MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
 return false;
}
INTERN bool DCALL
DeeType_HasCachedClassAttr(DeeTypeObject *__restrict tp_self,
                           char const *__restrict name, dhash_t hash) {
 dhash_t i,perturb;
 MEMBERCACHE_READ(&tp_self->tp_class_cache);
 if unlikely(!tp_self->tp_class_cache.mc_table) goto done;
 perturb = i = MEMBERCACHE_HASHST(&tp_self->tp_class_cache,hash);
 for (;; i = MEMBERCACHE_HASHNX(i,perturb),MEMBERCACHE_HASHPT(perturb)) {
  struct membercache_slot *item = MEMBERCACHE_HASHIT(&tp_self->tp_class_cache,i);
  if (item->mcs_type == MEMBERCACHE_UNUSED) break;
  if (item->mcs_hash != hash) continue;
  if (strcmp(item->mcs_name,name)) continue;
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
DeeType_DelCachedAttr(DeeTypeObject *__restrict tp_self,
                      DeeObject *__restrict self,
                      char const *__restrict name, dhash_t hash) {
 dhash_t i,perturb;
 MEMBERCACHE_READ(&tp_self->tp_cache);
 if unlikely(!tp_self->tp_cache.mc_table) goto done;
 perturb = i = MEMBERCACHE_HASHST(&tp_self->tp_cache,hash);
 for (;; i = MEMBERCACHE_HASHNX(i,perturb),MEMBERCACHE_HASHPT(perturb)) {
  struct membercache_slot *item = MEMBERCACHE_HASHIT(&tp_self->tp_cache,i);
  if (item->mcs_type == MEMBERCACHE_UNUSED) break;
  if (item->mcs_hash != hash) continue;
  if (strcmp(item->mcs_name,name)) continue;
  switch (item->mcs_type) {
  {
   DeeTypeObject *type;
  case MEMBERCACHE_METHOD:
   type = item->mcs_decl;
   MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
   return err_cant_access_attribute(type,name,ATTR_ACCESS_DEL);
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
   err_cant_access_attribute(type,name,ATTR_ACCESS_DEL);
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
   struct class_attribute *attr;
   struct class_desc *desc;
  case MEMBERCACHE_ATTRIB:
   attr = item->mcs_attrib.a_attr;
   desc = item->mcs_attrib.a_desc;
   MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
   return DeeInstance_DelAttribute(desc,
                                   DeeInstance_DESC(desc,self),
                                   self,attr);
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
DeeType_DelCachedClassAttr(DeeTypeObject *__restrict tp_self,
                           char const *__restrict name, dhash_t hash) {
 dhash_t i,perturb;
 MEMBERCACHE_READ(&tp_self->tp_class_cache);
 if unlikely(!tp_self->tp_class_cache.mc_table) goto done;
 perturb = i = MEMBERCACHE_HASHST(&tp_self->tp_class_cache,hash);
 for (;; i = MEMBERCACHE_HASHNX(i,perturb),MEMBERCACHE_HASHPT(perturb)) {
  struct membercache_slot *item = MEMBERCACHE_HASHIT(&tp_self->tp_class_cache,i);
  if (item->mcs_type == MEMBERCACHE_UNUSED) break;
  if (item->mcs_hash != hash) continue;
  if (strcmp(item->mcs_name,name)) continue;
  switch (item->mcs_type) {
  {
   DeeTypeObject *type;
  case MEMBERCACHE_METHOD:
  case MEMBERCACHE_INSTANCE_METHOD:
  case MEMBERCACHE_INSTANCE_GETSET:
  case MEMBERCACHE_INSTANCE_MEMBER:
   type = item->mcs_decl;
   MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
   return err_cant_access_attribute(type,name,ATTR_ACCESS_DEL);
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
   err_cant_access_attribute(type,name,ATTR_ACCESS_DEL);
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
   struct class_attribute *attr;
   DeeTypeObject *type;
   struct class_desc *desc;
  case MEMBERCACHE_ATTRIB:
   attr = item->mcs_attrib.a_attr;
   desc = item->mcs_attrib.a_desc;
   type = item->mcs_decl;
   MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
   return DeeInstance_DelAttribute(desc,class_desc_as_instance(desc),(DeeObject *)tp_self,attr);
  }
  {
   struct class_attribute *attr;
   DeeTypeObject *type;
  case MEMBERCACHE_INSTANCE_ATTRIB:
   attr = item->mcs_attrib.a_attr;
   type = item->mcs_decl;
   MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
   return DeeClass_DelInstanceAttribute(type,attr);
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
DeeType_DelCachedInstanceAttr(DeeTypeObject *__restrict tp_self,
                              char const *__restrict name, dhash_t hash) {
 dhash_t i,perturb;
 MEMBERCACHE_READ(&tp_self->tp_cache);
 if unlikely(!tp_self->tp_cache.mc_table) goto done;
 perturb = i = MEMBERCACHE_HASHST(&tp_self->tp_cache,hash);
 for (;; i = MEMBERCACHE_HASHNX(i,perturb),MEMBERCACHE_HASHPT(perturb)) {
  struct membercache_slot *item = MEMBERCACHE_HASHIT(&tp_self->tp_cache,i);
  if (item->mcs_type == MEMBERCACHE_UNUSED) break;
  if (item->mcs_hash != hash) continue;
  if (strcmp(item->mcs_name,name)) continue;
  switch (item->mcs_type) {
  {
   DeeTypeObject *type;
  case MEMBERCACHE_METHOD:
  case MEMBERCACHE_GETSET:
  case MEMBERCACHE_MEMBER:
   type = item->mcs_decl;
   MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
   return err_cant_access_attribute(type,name,ATTR_ACCESS_DEL);
  }
  {
   struct class_attribute *attr;
   DeeTypeObject *type;
  case MEMBERCACHE_ATTRIB:
   attr = item->mcs_attrib.a_attr;
   type = item->mcs_decl;
   MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
   return DeeClass_DelInstanceAttribute(type,attr);
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
DeeType_SetCachedAttr(DeeTypeObject *__restrict tp_self,
                      DeeObject *__restrict self,
                      char const *__restrict name, dhash_t hash,
                      DeeObject *__restrict value) {
 dhash_t i,perturb;
 MEMBERCACHE_READ(&tp_self->tp_cache);
 if unlikely(!tp_self->tp_cache.mc_table) goto done;
 perturb = i = MEMBERCACHE_HASHST(&tp_self->tp_cache,hash);
 for (;; i = MEMBERCACHE_HASHNX(i,perturb),MEMBERCACHE_HASHPT(perturb)) {
  struct membercache_slot *item = MEMBERCACHE_HASHIT(&tp_self->tp_cache,i);
  if (item->mcs_type == MEMBERCACHE_UNUSED) break;
  if (item->mcs_hash != hash) continue;
  if (strcmp(item->mcs_name,name)) continue;
  switch (item->mcs_type) {
  {
   DeeTypeObject *type;
  case MEMBERCACHE_METHOD:
   type = item->mcs_decl;
   MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
   return err_cant_access_attribute(type,name,ATTR_ACCESS_SET);
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
   err_cant_access_attribute(type,name,ATTR_ACCESS_SET);
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
   struct class_attribute *attr;
   struct class_desc *desc;
  case MEMBERCACHE_ATTRIB:
   attr = item->mcs_attrib.a_attr;
   desc = item->mcs_attrib.a_desc;
   MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
   return DeeInstance_SetAttribute(desc,
                                   DeeInstance_DESC(desc,self),
                                   self,attr,value);
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
DeeType_SetCachedClassAttr(DeeTypeObject *__restrict tp_self,
                           char const *__restrict name, dhash_t hash,
                           DeeObject *__restrict value) {
 dhash_t i,perturb;
 MEMBERCACHE_READ(&tp_self->tp_class_cache);
 if unlikely(!tp_self->tp_class_cache.mc_table) goto done;
 perturb = i = MEMBERCACHE_HASHST(&tp_self->tp_class_cache,hash);
 for (;; i = MEMBERCACHE_HASHNX(i,perturb),MEMBERCACHE_HASHPT(perturb)) {
  struct membercache_slot *item = MEMBERCACHE_HASHIT(&tp_self->tp_class_cache,i);
  if (item->mcs_type == MEMBERCACHE_UNUSED) break;
  if (item->mcs_hash != hash) continue;
  if (strcmp(item->mcs_name,name)) continue;
  switch (item->mcs_type) {
  {
   DeeTypeObject *type;
  case MEMBERCACHE_METHOD:
  case MEMBERCACHE_INSTANCE_METHOD:
  case MEMBERCACHE_INSTANCE_GETSET:
  case MEMBERCACHE_INSTANCE_MEMBER:
   type = item->mcs_decl;
   MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
   return err_cant_access_attribute(type,name,ATTR_ACCESS_SET);
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
   err_cant_access_attribute(type,name,ATTR_ACCESS_SET);
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
   struct class_attribute *attr;
   DeeTypeObject *type;
   struct class_desc *desc;
  case MEMBERCACHE_ATTRIB:
   attr = item->mcs_attrib.a_attr;
   desc = item->mcs_attrib.a_desc;
   type = item->mcs_decl;
   MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
   return DeeInstance_SetAttribute(desc,class_desc_as_instance(desc),(DeeObject *)tp_self,attr,value);
  }
  {
   struct class_attribute *attr;
   DeeTypeObject *type;
  case MEMBERCACHE_INSTANCE_ATTRIB:
   attr = item->mcs_attrib.a_attr;
   type = item->mcs_decl;
   MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
   return DeeClass_SetInstanceAttribute(type,attr,value);
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
DeeType_SetCachedInstanceAttr(DeeTypeObject *__restrict tp_self,
                              char const *__restrict name, dhash_t hash,
                              DeeObject *__restrict value) {
 dhash_t i,perturb;
 MEMBERCACHE_READ(&tp_self->tp_cache);
 if unlikely(!tp_self->tp_cache.mc_table) goto done;
 perturb = i = MEMBERCACHE_HASHST(&tp_self->tp_cache,hash);
 for (;; i = MEMBERCACHE_HASHNX(i,perturb),MEMBERCACHE_HASHPT(perturb)) {
  struct membercache_slot *item = MEMBERCACHE_HASHIT(&tp_self->tp_cache,i);
  if (item->mcs_type == MEMBERCACHE_UNUSED) break;
  if (item->mcs_hash != hash) continue;
  if (strcmp(item->mcs_name,name)) continue;
  switch (item->mcs_type) {
  {
   DeeTypeObject *type;
  case MEMBERCACHE_METHOD:
  case MEMBERCACHE_GETSET:
  case MEMBERCACHE_MEMBER:
   type = item->mcs_decl;
   MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
   return err_cant_access_attribute(type,name,ATTR_ACCESS_SET);
  }
  {
   struct class_attribute *attr;
   DeeTypeObject *type;
  case MEMBERCACHE_ATTRIB:
   attr = item->mcs_attrib.a_attr;
   type = item->mcs_decl;
   MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
   return DeeClass_SetInstanceAttribute(type,attr,value);
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
DeeType_SetBasicCachedAttr(DeeTypeObject *__restrict tp_self,
                           DeeObject *__restrict self,
                           char const *__restrict name, dhash_t hash,
                           DeeObject *__restrict value) {
 dhash_t i,perturb;
 MEMBERCACHE_READ(&tp_self->tp_cache);
 if unlikely(!tp_self->tp_cache.mc_table) goto done;
 perturb = i = MEMBERCACHE_HASHST(&tp_self->tp_cache,hash);
 for (;; i = MEMBERCACHE_HASHNX(i,perturb),MEMBERCACHE_HASHPT(perturb)) {
  struct membercache_slot *item = MEMBERCACHE_HASHIT(&tp_self->tp_cache,i);
  if (item->mcs_type == MEMBERCACHE_UNUSED) break;
  if (item->mcs_hash != hash) continue;
  if (strcmp(item->mcs_name,name)) continue;
  switch (item->mcs_type) {
  {
   struct buffer { uint8_t dat[COMPILER_OFFSETOF(struct type_member,m_doc)]; } buf;
  case MEMBERCACHE_MEMBER:
   buf = *(struct buffer *)&item->mcs_member;
   MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
   return type_member_set((struct type_member *)&buf,self,value);
  }
  {
   struct class_attribute *attr;
   struct class_desc *desc;
  case MEMBERCACHE_ATTRIB:
   attr = item->mcs_attrib.a_attr;
   desc = item->mcs_attrib.a_desc;
   MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
   return DeeInstance_SetBasicAttribute(desc,
                                        DeeInstance_DESC(desc,self),
                                        self,attr,value);
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
DeeType_SetBasicCachedClassAttr(DeeTypeObject *__restrict tp_self,
                                char const *__restrict name, dhash_t hash,
                                DeeObject *__restrict value) {
 dhash_t i,perturb;
 MEMBERCACHE_READ(&tp_self->tp_class_cache);
 if unlikely(!tp_self->tp_class_cache.mc_table) goto done;
 perturb = i = MEMBERCACHE_HASHST(&tp_self->tp_class_cache,hash);
 for (;; i = MEMBERCACHE_HASHNX(i,perturb),MEMBERCACHE_HASHPT(perturb)) {
  struct membercache_slot *item = MEMBERCACHE_HASHIT(&tp_self->tp_class_cache,i);
  if (item->mcs_type == MEMBERCACHE_UNUSED) break;
  if (item->mcs_hash != hash) continue;
  if (strcmp(item->mcs_name,name)) continue;
  switch (item->mcs_type) {
  {
   struct buffer { uint8_t dat[COMPILER_OFFSETOF(struct type_member,m_doc)]; } buf;
  case MEMBERCACHE_MEMBER:
   buf = *(struct buffer *)&item->mcs_member;
   MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
   return type_member_set((struct type_member *)&buf,(DeeObject *)tp_self,value);
  }
  {
   struct class_attribute *attr;
   DeeTypeObject *type;
   struct class_desc *desc;
  case MEMBERCACHE_ATTRIB:
   attr = item->mcs_attrib.a_attr;
   desc = item->mcs_attrib.a_desc;
   type = item->mcs_decl;
   MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
   return DeeInstance_SetBasicAttribute(desc,class_desc_as_instance(desc),(DeeObject *)tp_self,attr,value);
  }
  {
   struct class_attribute *attr;
   DeeTypeObject *type;
  case MEMBERCACHE_INSTANCE_ATTRIB:
   attr = item->mcs_attrib.a_attr;
   type = item->mcs_decl;
   MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
   return DeeClass_SetBasicInstanceAttribute(type,attr,value);
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
DeeType_SetBasicCachedInstanceAttr(DeeTypeObject *__restrict tp_self,
                                   char const *__restrict name, dhash_t hash,
                                   DeeObject *__restrict value) {
 dhash_t i,perturb;
 MEMBERCACHE_READ(&tp_self->tp_cache);
 if unlikely(!tp_self->tp_cache.mc_table) goto done;
 perturb = i = MEMBERCACHE_HASHST(&tp_self->tp_cache,hash);
 for (;; i = MEMBERCACHE_HASHNX(i,perturb),MEMBERCACHE_HASHPT(perturb)) {
  struct membercache_slot *item = MEMBERCACHE_HASHIT(&tp_self->tp_cache,i);
  if (item->mcs_type == MEMBERCACHE_UNUSED) break;
  if (item->mcs_hash != hash) continue;
  if (strcmp(item->mcs_name,name)) continue;
  switch (item->mcs_type) {
  {
   struct class_attribute *attr;
   DeeTypeObject *type;
  case MEMBERCACHE_ATTRIB:
   attr = item->mcs_attrib.a_attr;
   type = item->mcs_decl;
   MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
   return DeeClass_SetBasicInstanceAttribute(type,attr,value);
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
DeeType_CallCachedAttr(DeeTypeObject *__restrict tp_self,
                       DeeObject *__restrict self,
                       char const *__restrict name, dhash_t hash,
                       size_t argc, DeeObject **__restrict argv) {
 dhash_t i,perturb; DREF DeeObject *callback,*result;
 MEMBERCACHE_READ(&tp_self->tp_cache);
 if unlikely(!tp_self->tp_cache.mc_table) goto done;
 perturb = i = MEMBERCACHE_HASHST(&tp_self->tp_cache,hash);
 for (;; i = MEMBERCACHE_HASHNX(i,perturb),MEMBERCACHE_HASHPT(perturb)) {
  struct membercache_slot *item = MEMBERCACHE_HASHIT(&tp_self->tp_cache,i);
  if (item->mcs_type == MEMBERCACHE_UNUSED) break;
  if (item->mcs_hash != hash) continue;
  if (strcmp(item->mcs_name,name)) continue;
  switch (item->mcs_type) {
  {
   dobjmethod_t func;
  case MEMBERCACHE_METHOD:
   func = item->mcs_method.m_func;
   if (item->mcs_method.m_flag & TYPE_METHOD_FKWDS) {
    MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
    return (*(dkwobjmethod_t)func)(self,argc,argv,NULL);
   }
   MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
   return (*func)(self,argc,argv);
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
    result = DeeObject_Call(callback,argc,argv);
    Dee_Decref(callback);
    return result;
   }
   type = item->mcs_decl;
   MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
   err_cant_access_attribute(type,name,ATTR_ACCESS_GET);
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
   struct class_attribute *attr;
   struct class_desc *desc;
  case MEMBERCACHE_ATTRIB:
   attr = item->mcs_attrib.a_attr;
   desc = item->mcs_attrib.a_desc;
   MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
   return DeeInstance_CallAttribute(desc,
                                    DeeInstance_DESC(desc,self),
                                    self,attr,argc,argv);
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

PRIVATE ATTR_COLD int DCALL err_cant_access_clsproperty_get(void) {
 return err_cant_access_attribute(&DeeClsProperty_Type,"get",ATTR_ACCESS_GET);
}
PRIVATE ATTR_COLD int DCALL
err_classmember_requires_1_argument(char const *__restrict name) {
 return DeeError_Throwf(&DeeError_TypeError,
                        "classmember `%s' must be called with exactly 1 argument",
                        name);
}
PRIVATE ATTR_COLD int DCALL
err_classproperty_requires_1_argument(char const *__restrict name) {
 return DeeError_Throwf(&DeeError_TypeError,
                        "classproperty `%s' must be called with exactly 1 argument",
                        name);
}
PRIVATE ATTR_COLD int DCALL
err_classmethod_requires_at_least_1_argument(char const *__restrict name) {
 return DeeError_Throwf(&DeeError_TypeError,
                        "classmethod `%s' must be called with at least 1 argument",
                        name);
}

INTERN DREF DeeObject *DCALL
DeeType_CallCachedClassAttr(DeeTypeObject *__restrict tp_self,
                            char const *__restrict name, dhash_t hash,
                            size_t argc, DeeObject **__restrict argv) {
 dhash_t i,perturb; DREF DeeObject *callback,*result;
 MEMBERCACHE_READ(&tp_self->tp_class_cache);
 if unlikely(!tp_self->tp_class_cache.mc_table) goto done;
 perturb = i = MEMBERCACHE_HASHST(&tp_self->tp_class_cache,hash);
 for (;; i = MEMBERCACHE_HASHNX(i,perturb),MEMBERCACHE_HASHPT(perturb)) {
  struct membercache_slot *item = MEMBERCACHE_HASHIT(&tp_self->tp_class_cache,i);
  if (item->mcs_type == MEMBERCACHE_UNUSED) break;
  if (item->mcs_hash != hash) continue;
  if (strcmp(item->mcs_name,name)) continue;
  switch (item->mcs_type) {
  {
   dobjmethod_t func;
  case MEMBERCACHE_METHOD:
   func = item->mcs_method.m_func;
   if (item->mcs_method.m_flag & TYPE_METHOD_FKWDS) {
    MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
    return (*(dkwobjmethod_t)func)((DeeObject *)tp_self,argc,argv,NULL);
   }
   MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
   return (*func)((DeeObject *)tp_self,argc,argv);
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
    result = DeeObject_Call(callback,argc,argv);
    Dee_Decref(callback);
    return result;
   }
   type = item->mcs_decl;
   MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
   err_cant_access_attribute(type,name,ATTR_ACCESS_GET);
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
   struct class_attribute *attr;
   DeeTypeObject *type;
   struct class_desc *desc;
  case MEMBERCACHE_ATTRIB:
   attr = item->mcs_attrib.a_attr;
   desc = item->mcs_attrib.a_desc;
   type = item->mcs_decl;
   MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
   return DeeInstance_CallAttribute(desc,class_desc_as_instance(desc),(DeeObject *)tp_self,attr,argc,argv);
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
   struct class_attribute *attr;
   DeeTypeObject *type;
  case MEMBERCACHE_INSTANCE_ATTRIB:
   attr = item->mcs_attrib.a_attr;
   type = item->mcs_decl;
   MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
   return DeeClass_CallInstanceAttribute(type,attr,argc,argv);
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
 err_classmember_requires_1_argument(name);
 goto err;
err_classproperty_invalid_args:
 err_classproperty_requires_1_argument(name);
 goto err;
err_classmethod_noargs:
 err_classmethod_requires_at_least_1_argument(name);
err:
 return NULL;
}

#if 0
INTERN DREF DeeObject *DCALL
DeeType_CallCachedInstanceAttr(DeeTypeObject *__restrict tp_self,
                               char const *__restrict name, dhash_t hash,
                               size_t argc, DeeObject **__restrict argv) {
 dhash_t i,perturb; DREF DeeObject *callback,*result;
 MEMBERCACHE_READ(&tp_self->tp_cache);
 if unlikely(!tp_self->tp_cache.mc_table) goto done;
 perturb = i = MEMBERCACHE_HASHST(&tp_self->tp_cache,hash);
 for (;; i = MEMBERCACHE_HASHNX(i,perturb),MEMBERCACHE_HASHPT(perturb)) {
  struct membercache_slot *item = MEMBERCACHE_HASHIT(&tp_self->tp_cache,i);
  if (item->mcs_type == MEMBERCACHE_UNUSED) break;
  if (item->mcs_hash != hash) continue;
  if (strcmp(item->mcs_name,name)) continue;
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
   struct class_attribute *attr;
   DeeTypeObject *type;
  case MEMBERCACHE_ATTRIB:
   attr = item->mcs_attrib.a_attr;
   type = item->mcs_decl;
   MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
   return DeeClass_CallInstanceAttribute(type,attr,argc,argv);
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
 err_classmember_requires_1_argument(name);
 goto err;
err_classproperty_invalid_args:
 err_classproperty_requires_1_argument(name);
 goto err;
err_classmethod_noargs:
 err_classmethod_requires_at_least_1_argument(name);
err:
 return NULL;
}
#endif

/* @return: * :        The returned value.
 * @return: NULL:      An error occurred.
 * @return: ITER_DONE: The attribute could not be found in the cache. */
INTERN DREF DeeObject *DCALL
DeeType_CallCachedAttrKw(DeeTypeObject *__restrict tp_self,
                         DeeObject *__restrict self,
                         char const *__restrict name, dhash_t hash,
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
  if (strcmp(item->mcs_name,name)) continue;
  switch (item->mcs_type) {
  {
   dobjmethod_t func;
  case MEMBERCACHE_METHOD:
   func = item->mcs_method.m_func;
   if (item->mcs_method.m_flag & TYPE_METHOD_FKWDS) {
    MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
    return (*(dkwobjmethod_t)func)(self,argc,argv,kw);
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
   return (*func)(self,argc,argv);
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
    result = DeeObject_CallKw(callback,argc,argv,kw);
    Dee_Decref(callback);
    return result;
   }
   type = item->mcs_decl;
   MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
   err_cant_access_attribute(type,name,ATTR_ACCESS_GET);
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
   struct class_attribute *attr;
   struct class_desc *desc;
  case MEMBERCACHE_ATTRIB:
   attr = item->mcs_attrib.a_attr;
   desc = item->mcs_attrib.a_desc;
   MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
   return DeeInstance_CallAttributeKw(desc,
                                      DeeInstance_DESC(desc,self),
                                      self,attr,argc,argv,kw);
  }
  default: __builtin_unreachable();
  }
 }
done:
 MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
 return ITER_DONE;
err_no_keywords:
 err_keywords_func_not_accepted(name,kw);
err:
 return NULL;
}

INTDEF struct keyword getter_kwlist[];

INTERN DREF DeeObject *DCALL
DeeType_CallCachedClassAttrKw(DeeTypeObject *__restrict tp_self,
                              char const *__restrict name, dhash_t hash,
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
  if (strcmp(item->mcs_name,name)) continue;
  switch (item->mcs_type) {
  {
   dobjmethod_t func;
  case MEMBERCACHE_METHOD:
   func = item->mcs_method.m_func;
   if (item->mcs_method.m_flag & TYPE_METHOD_FKWDS) {
    MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
    return (*(dkwobjmethod_t)func)((DeeObject *)tp_self,argc,argv,kw);
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
   return (*func)((DeeObject *)tp_self,argc,argv);
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
    result = DeeObject_CallKw(callback,argc,argv,kw);
    Dee_Decref(callback);
    return result;
   }
   type = item->mcs_decl;
   MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
   err_cant_access_attribute(type,name,ATTR_ACCESS_GET);
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
   struct class_attribute *attr;
   DeeTypeObject *type;
   struct class_desc *desc;
  case MEMBERCACHE_ATTRIB:
   attr = item->mcs_attrib.a_attr;
   desc = item->mcs_attrib.a_desc;
   type = item->mcs_decl;
   MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
   return DeeInstance_CallAttributeKw(desc,class_desc_as_instance(desc),(DeeObject *)tp_self,attr,argc,argv,kw);
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
   if (DeeArg_UnpackKw(argc,argv,kw,getter_kwlist,"o:get",&thisarg)) goto err;
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
   if (DeeArg_UnpackKw(argc,argv,kw,getter_kwlist,"o:get",&thisarg)) goto err;
   return type_member_get(&member,thisarg);
  }
  {
   struct class_attribute *attr;
   DeeTypeObject *type;
  case MEMBERCACHE_INSTANCE_ATTRIB:
   attr = item->mcs_attrib.a_attr;
   type = item->mcs_decl;
   MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
   return DeeClass_CallInstanceAttributeKw(type,attr,argc,argv,kw);
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
 err_classmember_requires_1_argument(name);
 goto err;
err_classproperty_invalid_args:
 err_classproperty_requires_1_argument(name);
 goto err;
err_classmethod_noargs:
 err_classmethod_requires_at_least_1_argument(name);
 goto err;
err_no_keywords:
 err_keywords_func_not_accepted(name,kw);
err:
 return NULL;
}

INTERN DREF DeeObject *DCALL
DeeType_CallCachedInstanceAttrKw(DeeTypeObject *__restrict tp_self,
                                 char const *__restrict name, dhash_t hash,
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
  if (strcmp(item->mcs_name,name)) continue;
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
   if (DeeArg_UnpackKw(argc,argv,kw,getter_kwlist,"o:get",&thisarg)) goto err;
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
   if (DeeArg_UnpackKw(argc,argv,kw,getter_kwlist,"o:get",&thisarg)) goto err;
   return type_member_get(&member,thisarg);
  }
  {
   struct class_attribute *attr;
   DeeTypeObject *type;
  case MEMBERCACHE_ATTRIB:
   attr = item->mcs_attrib.a_attr;
   type = item->mcs_decl;
   MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
   return DeeClass_CallInstanceAttributeKw(type,attr,argc,argv,kw);
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
 err_classmember_requires_1_argument(name);
 goto err;
err_classproperty_invalid_args:
 err_classproperty_requires_1_argument(name);
 goto err;
err_classmethod_noargs:
 err_classmethod_requires_at_least_1_argument(name);
 goto err;
err_no_keywords:
 err_keywords_func_not_accepted(name,kw);
err:
 return NULL;
}

#ifdef CONFIG_HAVE_CALLTUPLE_OPTIMIZATIONS
/* @return: * :        The returned value.
 * @return: NULL:      An error occurred.
 * @return: ITER_DONE: The attribute could not be found in the cache. */
INTERN DREF DeeObject *DCALL
DeeType_CallCachedAttrTuple(DeeTypeObject *__restrict tp_self,
                            DeeObject *__restrict self,
                            char const *__restrict name, dhash_t hash,
                            DeeObject *__restrict args) {
 dhash_t i,perturb; DREF DeeObject *callback,*result;
 MEMBERCACHE_READ(&tp_self->tp_cache);
 if unlikely(!tp_self->tp_cache.mc_table) goto done;
 perturb = i = MEMBERCACHE_HASHST(&tp_self->tp_cache,hash);
 for (;; i = MEMBERCACHE_HASHNX(i,perturb),MEMBERCACHE_HASHPT(perturb)) {
  struct membercache_slot *item = MEMBERCACHE_HASHIT(&tp_self->tp_cache,i);
  if (item->mcs_type == MEMBERCACHE_UNUSED) break;
  if (item->mcs_hash != hash) continue;
  if (strcmp(item->mcs_name,name)) continue;
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
   err_cant_access_attribute(type,name,ATTR_ACCESS_GET);
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
   struct class_attribute *attr;
   struct class_desc *desc;
  case MEMBERCACHE_ATTRIB:
   attr = item->mcs_attrib.a_attr;
   desc = item->mcs_attrib.a_desc;
   MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
   return DeeInstance_CallAttributeTuple(desc,
                                         DeeInstance_DESC(desc,self),
                                         self,attr,args);
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
DeeType_CallCachedClassAttrTuple(DeeTypeObject *__restrict tp_self,
                                 char const *__restrict name, dhash_t hash,
                                 DeeObject *__restrict args) {
 dhash_t i,perturb; DREF DeeObject *callback,*result;
 MEMBERCACHE_READ(&tp_self->tp_class_cache);
 if unlikely(!tp_self->tp_class_cache.mc_table) goto done;
 perturb = i = MEMBERCACHE_HASHST(&tp_self->tp_class_cache,hash);
 for (;; i = MEMBERCACHE_HASHNX(i,perturb),MEMBERCACHE_HASHPT(perturb)) {
  struct membercache_slot *item = MEMBERCACHE_HASHIT(&tp_self->tp_class_cache,i);
  if (item->mcs_type == MEMBERCACHE_UNUSED) break;
  if (item->mcs_hash != hash) continue;
  if (strcmp(item->mcs_name,name)) continue;
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
   err_cant_access_attribute(type,name,ATTR_ACCESS_GET);
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
   struct class_attribute *attr;
   DeeTypeObject *type;
   struct class_desc *desc;
  case MEMBERCACHE_ATTRIB:
   attr = item->mcs_attrib.a_attr;
   desc = item->mcs_attrib.a_desc;
   type = item->mcs_decl;
   MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
   return DeeInstance_CallAttributeTuple(desc,class_desc_as_instance(desc),
                                        (DeeObject *)tp_self,attr,args);
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
   struct class_attribute *attr;
   DeeTypeObject *type;
  case MEMBERCACHE_INSTANCE_ATTRIB:
   attr = item->mcs_attrib.a_attr;
   type = item->mcs_decl;
   MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
   return DeeClass_CallInstanceAttributeTuple(type,attr,args);
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
 err_classmember_requires_1_argument(name);
 goto err;
err_classproperty_invalid_args:
 err_classproperty_requires_1_argument(name);
 goto err;
err_classmethod_noargs:
 err_classmethod_requires_at_least_1_argument(name);
err:
 return NULL;
}
#if 0
INTERN DREF DeeObject *DCALL
DeeType_CallCachedInstanceAttrTuple(DeeTypeObject *__restrict tp_self,
                                    char const *__restrict name, dhash_t hash,
                                    DeeObject *__restrict args) {
 dhash_t i,perturb; DREF DeeObject *callback,*result;
 MEMBERCACHE_READ(&tp_self->tp_cache);
 if unlikely(!tp_self->tp_cache.mc_table) goto done;
 perturb = i = MEMBERCACHE_HASHST(&tp_self->tp_cache,hash);
 for (;; i = MEMBERCACHE_HASHNX(i,perturb),MEMBERCACHE_HASHPT(perturb)) {
  struct membercache_slot *item = MEMBERCACHE_HASHIT(&tp_self->tp_cache,i);
  if (item->mcs_type == MEMBERCACHE_UNUSED) break;
  if (item->mcs_hash != hash) continue;
  if (strcmp(item->mcs_name,name)) continue;
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
   struct class_attribute *attr;
   DeeTypeObject *type;
  case MEMBERCACHE_ATTRIB:
   attr = item->mcs_attrib.a_attr;
   type = item->mcs_decl;
   MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
   return DeeClass_CallInstanceAttributeTuple(type,attr,args);
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
 err_classmember_requires_1_argument(name);
 goto err;
err_classproperty_invalid_args:
 err_classproperty_requires_1_argument(name);
 goto err;
err_classmethod_noargs:
 err_classmethod_requires_at_least_1_argument(name);
err:
 return NULL;
}
#endif
INTERN DREF DeeObject *DCALL
DeeType_CallCachedAttrTupleKw(DeeTypeObject *__restrict tp_self,
                              DeeObject *__restrict self,
                              char const *__restrict name, dhash_t hash,
                              DeeObject *__restrict args, DeeObject *kw) {
 dhash_t i,perturb; DREF DeeObject *callback,*result;
 MEMBERCACHE_READ(&tp_self->tp_cache);
 if unlikely(!tp_self->tp_cache.mc_table) goto done;
 perturb = i = MEMBERCACHE_HASHST(&tp_self->tp_cache,hash);
 for (;; i = MEMBERCACHE_HASHNX(i,perturb),MEMBERCACHE_HASHPT(perturb)) {
  struct membercache_slot *item = MEMBERCACHE_HASHIT(&tp_self->tp_cache,i);
  if (item->mcs_type == MEMBERCACHE_UNUSED) break;
  if (item->mcs_hash != hash) continue;
  if (strcmp(item->mcs_name,name)) continue;
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
   err_cant_access_attribute(type,name,ATTR_ACCESS_GET);
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
   struct class_attribute *attr;
   struct class_desc *desc;
  case MEMBERCACHE_ATTRIB:
   attr = item->mcs_attrib.a_attr;
   desc = item->mcs_attrib.a_desc;
   MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
   return DeeInstance_CallAttributeTupleKw(desc,
                                           DeeInstance_DESC(desc,self),
                                           self,attr,args,kw);
  }
  default: __builtin_unreachable();
  }
 }
done:
 MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
 return ITER_DONE;
err_no_keywords:
 err_keywords_func_not_accepted(name,kw);
err:
 return NULL;
}

INTERN DREF DeeObject *DCALL
DeeType_CallCachedClassAttrTupleKw(DeeTypeObject *__restrict tp_self,
                                   char const *__restrict name, dhash_t hash,
                                   DeeObject *__restrict args, DeeObject *kw) {
 dhash_t i,perturb; DREF DeeObject *callback,*result;
 MEMBERCACHE_READ(&tp_self->tp_class_cache);
 if unlikely(!tp_self->tp_class_cache.mc_table) goto done;
 perturb = i = MEMBERCACHE_HASHST(&tp_self->tp_class_cache,hash);
 for (;; i = MEMBERCACHE_HASHNX(i,perturb),MEMBERCACHE_HASHPT(perturb)) {
  struct membercache_slot *item = MEMBERCACHE_HASHIT(&tp_self->tp_class_cache,i);
  if (item->mcs_type == MEMBERCACHE_UNUSED) break;
  if (item->mcs_hash != hash) continue;
  if (strcmp(item->mcs_name,name)) continue;
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
   err_cant_access_attribute(type,name,ATTR_ACCESS_GET);
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
   struct class_attribute *attr;
   DeeTypeObject *type;
   struct class_desc *desc;
  case MEMBERCACHE_ATTRIB:
   attr = item->mcs_attrib.a_attr;
   desc = item->mcs_attrib.a_desc;
   type = item->mcs_decl;
   MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
   return DeeInstance_CallAttributeTupleKw(desc,class_desc_as_instance(desc),(DeeObject *)tp_self,attr,args,kw);
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
   struct class_attribute *attr;
   DeeTypeObject *type;
  case MEMBERCACHE_INSTANCE_ATTRIB:
   attr = item->mcs_attrib.a_attr;
   type = item->mcs_decl;
   MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
   return DeeClass_CallInstanceAttributeTupleKw(type,attr,args,kw);
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
 err_classmember_requires_1_argument(name);
 goto err;
err_classproperty_invalid_args:
 err_classproperty_requires_1_argument(name);
 goto err;
err_classmethod_noargs:
 err_classmethod_requires_at_least_1_argument(name);
 goto err;
err_no_keywords:
 err_keywords_func_not_accepted(name,kw);
err:
 return NULL;
}
#if 0
INTERN DREF DeeObject *DCALL
DeeType_CallCachedInstanceAttrTupleKw(DeeTypeObject *__restrict tp_self,
                                      char const *__restrict name, dhash_t hash,
                                      DeeObject *__restrict args, DeeObject *kw) {
 dhash_t i,perturb; DREF DeeObject *callback,*result;
 MEMBERCACHE_READ(&tp_self->tp_cache);
 if unlikely(!tp_self->tp_cache.mc_table) goto done;
 perturb = i = MEMBERCACHE_HASHST(&tp_self->tp_cache,hash);
 for (;; i = MEMBERCACHE_HASHNX(i,perturb),MEMBERCACHE_HASHPT(perturb)) {
  struct membercache_slot *item = MEMBERCACHE_HASHIT(&tp_self->tp_cache,i);
  if (item->mcs_type == MEMBERCACHE_UNUSED) break;
  if (item->mcs_hash != hash) continue;
  if (strcmp(item->mcs_name,name)) continue;
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
   struct class_attribute *attr;
   DeeTypeObject *type;
  case MEMBERCACHE_ATTRIB:
   attr = item->mcs_attrib.a_attr;
   type = item->mcs_decl;
   MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
   return DeeClass_CallInstanceAttributeTupleKw(type,attr,args,kw);
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
 err_classmember_requires_1_argument(name);
 goto err;
err_classproperty_invalid_args:
 err_classproperty_requires_1_argument(name);
 goto err;
err_classmethod_noargs:
 err_classmethod_requires_at_least_1_argument(name);
 goto err;
err_no_keywords:
 err_keywords_func_not_accepted(name,kw);
err:
 return NULL;
}
#endif
#endif /* CONFIG_HAVE_CALLTUPLE_OPTIMIZATIONS */


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

/* @return: * :        The returned value.
 * @return: NULL:      An error occurred.
 * @return: ITER_DONE: The attribute could not be found in the cache. */
INTERN DREF DeeObject *DCALL
DeeType_VCallCachedAttrf(DeeTypeObject *__restrict tp_self,
                         DeeObject *__restrict self,
                         char const *__restrict name, dhash_t hash,
                         char const *__restrict format, va_list args) {
 dhash_t i,perturb; DREF DeeObject *callback,*result;
 MEMBERCACHE_READ(&tp_self->tp_cache);
 if unlikely(!tp_self->tp_cache.mc_table) goto done;
 perturb = i = MEMBERCACHE_HASHST(&tp_self->tp_cache,hash);
 for (;; i = MEMBERCACHE_HASHNX(i,perturb),MEMBERCACHE_HASHPT(perturb)) {
  struct membercache_slot *item = MEMBERCACHE_HASHIT(&tp_self->tp_cache,i);
  if (item->mcs_type == MEMBERCACHE_UNUSED) break;
  if (item->mcs_hash != hash) continue;
  if (strcmp(item->mcs_name,name)) continue;
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
   err_cant_access_attribute(type,name,ATTR_ACCESS_GET);
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
   struct class_attribute *attr;
   struct class_desc *desc;
  case MEMBERCACHE_ATTRIB:
   attr = item->mcs_attrib.a_attr;
   desc = item->mcs_attrib.a_desc;
   MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
   return DeeInstance_VCallAttributef(desc,
                                      DeeInstance_DESC(desc,self),
                                      self,attr,format,args);
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
DeeType_VCallCachedClassAttrf(DeeTypeObject *__restrict tp_self,
                              char const *__restrict name, dhash_t hash,
                              char const *__restrict format, va_list args) {
 dhash_t i,perturb; DREF DeeObject *callback,*result,*args_tuple;
 MEMBERCACHE_READ(&tp_self->tp_class_cache);
 if unlikely(!tp_self->tp_class_cache.mc_table) goto done;
 perturb = i = MEMBERCACHE_HASHST(&tp_self->tp_class_cache,hash);
 for (;; i = MEMBERCACHE_HASHNX(i,perturb),MEMBERCACHE_HASHPT(perturb)) {
  struct membercache_slot *item = MEMBERCACHE_HASHIT(&tp_self->tp_class_cache,i);
  if (item->mcs_type == MEMBERCACHE_UNUSED) break;
  if (item->mcs_hash != hash) continue;
  if (strcmp(item->mcs_name,name)) continue;
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
   err_cant_access_attribute(type,name,ATTR_ACCESS_GET);
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
   struct class_attribute *attr;
   DeeTypeObject *type;
   struct class_desc *desc;
  case MEMBERCACHE_ATTRIB:
   attr = item->mcs_attrib.a_attr;
   desc = item->mcs_attrib.a_desc;
   type = item->mcs_decl;
   MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
   return DeeInstance_VCallAttributef(desc,class_desc_as_instance(desc),(DeeObject *)tp_self,attr,format,args);
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
   struct class_attribute *attr;
   DeeTypeObject *type;
  case MEMBERCACHE_INSTANCE_ATTRIB:
   attr = item->mcs_attrib.a_attr;
   type = item->mcs_decl;
   MEMBERCACHE_ENDREAD(&tp_self->tp_class_cache);
   return DeeClass_VCallInstanceAttributef(type,attr,format,args);
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
 err_classmember_requires_1_argument(name);
 goto err;
err_classproperty_invalid_args_args_tuple:
 Dee_Decref(args_tuple);
/*err_classproperty_invalid_args:*/
 err_classproperty_requires_1_argument(name);
 goto err;
err_classmethod_noargs_args_tuple:
 Dee_Decref(args_tuple);
/*err_classmethod_noargs:*/
 err_classmethod_requires_at_least_1_argument(name);
 goto err;
err_args_tuple:
 Dee_Decref(args_tuple);
err:
 return NULL;
}
#if 0
INTERN DREF DeeObject *DCALL
DeeType_VCallCachedInstanceAttrf(DeeTypeObject *__restrict tp_self,
                                 char const *__restrict name, dhash_t hash,
                                 char const *__restrict format, va_list args) {
 dhash_t i,perturb; DREF DeeObject *callback,*result,*args_tuple;
 MEMBERCACHE_READ(&tp_self->tp_cache);
 if unlikely(!tp_self->tp_cache.mc_table) goto done;
 perturb = i = MEMBERCACHE_HASHST(&tp_self->tp_cache,hash);
 for (;; i = MEMBERCACHE_HASHNX(i,perturb),MEMBERCACHE_HASHPT(perturb)) {
  struct membercache_slot *item = MEMBERCACHE_HASHIT(&tp_self->tp_cache,i);
  if (item->mcs_type == MEMBERCACHE_UNUSED) break;
  if (item->mcs_hash != hash) continue;
  if (strcmp(item->mcs_name,name)) continue;
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
   struct class_attribute *attr;
   DeeTypeObject *type;
  case MEMBERCACHE_ATTRIB:
   attr = item->mcs_attrib.a_attr;
   type = item->mcs_decl;
   MEMBERCACHE_ENDREAD(&tp_self->tp_cache);
   return DeeClass_VCallInstanceAttributef(type,attr,format,args);
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
 err_classmember_requires_1_argument(name);
 goto err;
err_classproperty_invalid_args_args_tuple:
 Dee_Decref(args_tuple);
/*err_classproperty_invalid_args:*/
 err_classproperty_requires_1_argument(name);
 goto err;
err_classmethod_noargs_args_tuple:
 Dee_Decref(args_tuple);
/*err_classmethod_noargs:*/
 err_classmethod_requires_at_least_1_argument(name);
 goto err;
err_args_tuple:
 Dee_Decref(args_tuple);
err:
 return NULL;
}
#endif

INTDEF DeeTypeObject *DCALL
type_member_typefor(struct type_member *__restrict self);

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
  if (strcmp(item->mcs_name,rules->alr_name)) continue;
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
   struct class_attribute *attr;
   struct instance_desc *inst;
  case MEMBERCACHE_ATTRIB:
   doc  = NULL;
   attr = item->mcs_attrib.a_attr;
   perm = ATTR_IMEMBER | ATTR_PERMGET | ATTR_PERMDEL | ATTR_PERMSET;
   if (attr->ca_doc) {
    doc   = DeeString_STR(attr->ca_doc);
    perm |= ATTR_DOCOBJ;
    Dee_Incref(attr->ca_doc);
   }
   if (attr->ca_flag & CLASS_ATTRIBUTE_FPRIVATE)
       perm |= ATTR_PRIVATE;
   if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
    perm       |= ATTR_PROPERTY;
    member_type = NULL;
   } else if (attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD) {
    perm       |= ATTR_PERMCALL;
    member_type = &DeeInstanceMethod_Type;
    Dee_Incref(member_type);
   } else {
    member_type = NULL;
   }
   inst = NULL;
   if (attr->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM)
       inst = class_desc_as_instance(item->mcs_attrib.a_desc);
   else if (instance)
       inst = DeeInstance_DESC(item->mcs_attrib.a_desc,instance);
   if (inst) {
    rwlock_read(&inst->id_lock);
    if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
     if (!inst->id_vtab[attr->ca_addr + CLASS_GETSET_GET])
          perm &= ~ATTR_PERMGET;
     if (!(attr->ca_flag & CLASS_ATTRIBUTE_FREADONLY)) {
      if (!inst->id_vtab[attr->ca_addr + CLASS_GETSET_DEL])
           perm &= ~ATTR_PERMDEL;
      if (!inst->id_vtab[attr->ca_addr + CLASS_GETSET_SET])
           perm &= ~ATTR_PERMSET;
     }
    } else if (!(attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD)) {
     ASSERT(!member_type);
     member_type = (DREF DeeTypeObject *)inst->id_vtab[attr->ca_addr + CLASS_GETSET_GET];
     if (member_type) { member_type = Dee_TYPE(member_type); Dee_Incref(member_type); }
    }
    rwlock_endread(&inst->id_lock);
   }
   if (attr->ca_flag & CLASS_ATTRIBUTE_FREADONLY)
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
 MEMBERCACHE_READ(&tp_self->tp_cache);
 if unlikely(!tp_self->tp_cache.mc_table) goto done;
 perturb = i = MEMBERCACHE_HASHST(&tp_self->tp_cache,rules->alr_hash);
 for (;; i = MEMBERCACHE_HASHNX(i,perturb),MEMBERCACHE_HASHPT(perturb)) {
  struct membercache_slot *item = MEMBERCACHE_HASHIT(&tp_self->tp_cache,i);
  char const *doc; uint16_t perm;
  DREF DeeTypeObject *member_decl,*member_type;
  if (item->mcs_type == MEMBERCACHE_UNUSED) break;
  if (item->mcs_hash != rules->alr_hash) continue;
  if (strcmp(item->mcs_name,rules->alr_name)) continue;
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
   struct class_attribute *attr;
   struct instance_desc *inst;
  case MEMBERCACHE_ATTRIB:
   doc  = NULL;
   attr = item->mcs_attrib.a_attr;
   perm = ATTR_CMEMBER | ATTR_PERMGET | ATTR_PERMDEL | ATTR_PERMSET;
   if (attr->ca_doc) {
    doc   = DeeString_STR(attr->ca_doc);
    perm |= ATTR_DOCOBJ;
    Dee_Incref(attr->ca_doc);
   }
   if (attr->ca_flag & CLASS_ATTRIBUTE_FPRIVATE)
       perm |= ATTR_PRIVATE;
   if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
    perm       |= ATTR_PROPERTY;
    member_type = NULL;
   } else if (attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD) {
    perm       |= ATTR_PERMCALL;
    member_type = &DeeInstanceMethod_Type;
    Dee_Incref(member_type);
   } else {
    member_type = NULL;
   }
   inst = class_desc_as_instance(item->mcs_attrib.a_desc);
   rwlock_read(&inst->id_lock);
   if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
    if (!inst->id_vtab[attr->ca_addr + CLASS_GETSET_GET])
         perm &= ~ATTR_PERMGET;
    if (!(attr->ca_flag & CLASS_ATTRIBUTE_FREADONLY)) {
     if (!inst->id_vtab[attr->ca_addr + CLASS_GETSET_DEL])
          perm &= ~ATTR_PERMDEL;
     if (!inst->id_vtab[attr->ca_addr + CLASS_GETSET_SET])
          perm &= ~ATTR_PERMSET;
    }
   } else if (!(attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD)) {
    ASSERT(!member_type);
    member_type = (DREF DeeTypeObject *)inst->id_vtab[attr->ca_addr + CLASS_GETSET_GET];
    if (member_type) { member_type = Dee_TYPE(member_type); Dee_Incref(member_type); }
   }
   rwlock_endread(&inst->id_lock);
   if (attr->ca_flag & CLASS_ATTRIBUTE_FREADONLY)
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
   struct class_attribute *attr;
  case MEMBERCACHE_INSTANCE_ATTRIB:
   doc  = NULL;
   attr = item->mcs_attrib.a_attr;
   perm = ATTR_CMEMBER | ATTR_IMEMBER | ATTR_WRAPPER | ATTR_PERMGET | ATTR_PERMDEL | ATTR_PERMSET;
   if (attr->ca_doc) {
    doc   = DeeString_STR(attr->ca_doc);
    perm |= ATTR_DOCOBJ;
    Dee_Incref(attr->ca_doc);
   }
   if (attr->ca_flag & CLASS_ATTRIBUTE_FPRIVATE)
       perm |= ATTR_PRIVATE;
   if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
    perm       |= ATTR_PROPERTY;
    member_type = NULL;
   } else if (attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD) {
    perm       |= ATTR_PERMCALL;
    member_type = &DeeInstanceMethod_Type;
    Dee_Incref(member_type);
   } else {
    member_type = NULL;
   }
   if (attr->ca_flag & CLASS_ATTRIBUTE_FCLASSMEM) {
    struct instance_desc *inst;
    inst = class_desc_as_instance(item->mcs_attrib.a_desc);
    rwlock_read(&inst->id_lock);
    if (attr->ca_flag & CLASS_ATTRIBUTE_FGETSET) {
     if (!inst->id_vtab[attr->ca_addr + CLASS_GETSET_GET])
          perm &= ~ATTR_PERMGET;
     if (!(attr->ca_flag & CLASS_ATTRIBUTE_FREADONLY)) {
      if (!inst->id_vtab[attr->ca_addr + CLASS_GETSET_DEL])
           perm &= ~ATTR_PERMDEL;
      if (!inst->id_vtab[attr->ca_addr + CLASS_GETSET_SET])
           perm &= ~ATTR_PERMSET;
     }
    } else if (!(attr->ca_flag & CLASS_ATTRIBUTE_FMETHOD)) {
     ASSERT(!member_type);
     member_type = (DREF DeeTypeObject *)inst->id_vtab[attr->ca_addr + CLASS_GETSET_GET];
     if (member_type) { member_type = Dee_TYPE(member_type); Dee_Incref(member_type); }
    }
    rwlock_endread(&inst->id_lock);
   }
   if (attr->ca_flag & CLASS_ATTRIBUTE_FREADONLY)
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


INTDEF struct class_attribute *
(DCALL DeeType_QueryAttributeWithHash)(DeeTypeObject *__restrict tp_invoker,
                                       DeeTypeObject *__restrict tp_self,
                                       /*String*/DeeObject *__restrict name,
                                       dhash_t hash) {
 struct class_attribute *result;
 result = DeeClass_QueryInstanceAttributeWithHash(tp_self,name,hash);
 if (result)
     membercache_addattrib(&tp_invoker->tp_cache,tp_self,hash,result);
 return result;
}
INTDEF struct class_attribute *
(DCALL DeeType_QueryAttributeStringWithHash)(DeeTypeObject *__restrict tp_invoker,
                                             DeeTypeObject *__restrict tp_self,
                                             char const *__restrict name, dhash_t hash) {
 struct class_attribute *result;
 result = DeeClass_QueryInstanceAttributeStringWithHash(tp_self,name,hash);
 if (result)
     membercache_addattrib(&tp_invoker->tp_cache,tp_self,hash,result);
 return result;
}
INTDEF struct class_attribute *
(DCALL DeeType_QueryClassAttributeWithHash)(DeeTypeObject *__restrict tp_invoker,
                                            DeeTypeObject *__restrict tp_self,
                                            /*String*/DeeObject *__restrict name,
                                            dhash_t hash) {
 struct class_attribute *result;
 result = DeeClass_QueryClassAttributeWithHash(tp_self,name,hash);
 if (result)
     membercache_addattrib(&tp_invoker->tp_class_cache,tp_self,hash,result);
 return result;
}
INTDEF struct class_attribute *
(DCALL DeeType_QueryClassAttributeStringWithHash)(DeeTypeObject *__restrict tp_invoker,
                                                  DeeTypeObject *__restrict tp_self,
                                                  char const *__restrict name, dhash_t hash) {
 struct class_attribute *result;
 result = DeeClass_QueryClassAttributeStringWithHash(tp_self,name,hash);
 if (result)
     membercache_addattrib(&tp_invoker->tp_class_cache,tp_self,hash,result);
 return result;
}
INTDEF struct class_attribute *
(DCALL DeeType_QueryInstanceAttributeWithHash)(DeeTypeObject *__restrict tp_invoker,
                                               DeeTypeObject *__restrict tp_self,
                                               /*String*/DeeObject *__restrict name,
                                               dhash_t hash) {
 struct class_attribute *result;
 result = DeeClass_QueryInstanceAttributeWithHash(tp_self,name,hash);
 if (result)
     membercache_addinstanceattrib(&tp_invoker->tp_class_cache,tp_self,hash,result);
 return result;
}
INTDEF struct class_attribute *
(DCALL DeeType_QueryInstanceAttributeStringWithHash)(DeeTypeObject *__restrict tp_invoker,
                                                     DeeTypeObject *__restrict tp_self,
                                                     char const *__restrict name, dhash_t hash) {
 struct class_attribute *result;
 result = DeeClass_QueryInstanceAttributeStringWithHash(tp_self,name,hash);
 if (result)
     membercache_addinstanceattrib(&tp_invoker->tp_class_cache,tp_self,hash,result);
 return result;
}



INTERN DREF DeeObject *DCALL
type_method_getattr(struct membercache *__restrict cache, DeeTypeObject *__restrict decl,
                    struct type_method *__restrict chain, DeeObject *__restrict self,
                    char const *__restrict attr_name, dhash_t hash) {
 for (; chain->m_name; ++chain) {
  if (strcmp(chain->m_name,attr_name) != 0) continue;
  membercache_addmethod(cache,decl,hash,chain);
  return type_method_get(chain,self);
 }
 return ITER_DONE;
}
INTERN DREF DeeObject *
(DCALL DeeType_GetInstanceMethodAttr)(DeeTypeObject *__restrict tp_invoker,
                                      DeeTypeObject *__restrict tp_self,
                                      char const *__restrict attr_name, dhash_t hash) {
 struct type_method *chain = tp_self->tp_methods;
 for (; chain->m_name; ++chain) {
  if (strcmp(chain->m_name,attr_name) != 0) continue;
  membercache_addinstancemethod(&tp_invoker->tp_class_cache,tp_self,hash,chain);
  return type_obmeth_get(tp_self,chain);
 }
 return ITER_DONE;
}
INTERN DREF DeeObject *
(DCALL DeeType_GetIInstanceMethodAttr)(DeeTypeObject *__restrict tp_invoker,
                                       DeeTypeObject *__restrict tp_self,
                                       char const *__restrict attr_name, dhash_t hash) {
 struct type_method *chain = tp_self->tp_methods;
 for (; chain->m_name; ++chain) {
  if (strcmp(chain->m_name,attr_name) != 0) continue;
  membercache_addmethod(&tp_invoker->tp_cache,tp_self,hash,chain);
  return type_obmeth_get(tp_self,chain);
 }
 return ITER_DONE;
}
INTERN DREF DeeObject *DCALL
type_method_docattr(struct membercache *__restrict cache,
                    DeeTypeObject *__restrict decl,
                    struct type_method *__restrict chain,
                    char const *__restrict attr_name, dhash_t hash) {
 for (; chain->m_name; ++chain) {
  if (strcmp(chain->m_name,attr_name) != 0) continue;
  membercache_addmethod(cache,decl,hash,chain);
  return type_method_doc(chain);
 }
 return ITER_DONE;
}
INTERN DREF DeeObject *
(DCALL DeeType_DocInstanceMethodAttr)(DeeTypeObject *__restrict tp_invoker,
                                      DeeTypeObject *__restrict tp_self,
                                      char const *__restrict attr_name,
                                      dhash_t hash) {
 struct type_method *chain = tp_self->tp_methods;
 for (; chain->m_name; ++chain) {
  if (strcmp(chain->m_name,attr_name) != 0) continue;
  membercache_addinstancemethod(&tp_invoker->tp_class_cache,tp_self,hash,chain);
  return type_method_doc(chain);
 }
 return ITER_DONE;
}

INTERN DREF DeeObject *DCALL
type_method_callattr(struct membercache *__restrict cache, DeeTypeObject *__restrict decl,
                     struct type_method *__restrict chain, DeeObject *__restrict self,
                     char const *__restrict attr_name, dhash_t hash,
                     size_t argc, DeeObject **__restrict argv) {
 for (; chain->m_name; ++chain) {
  if (strcmp(chain->m_name,attr_name) != 0) continue;
  membercache_addmethod(cache,decl,hash,chain);
  return type_method_call(chain,self,argc,argv);
 }
 return ITER_DONE;
}
INTDEF DREF DeeObject *
(DCALL DeeType_CallInstanceMethodAttr)(DeeTypeObject *__restrict tp_invoker,
                                       DeeTypeObject *__restrict tp_self,
                                       char const *__restrict attr_name, dhash_t hash,
                                       size_t argc, DeeObject **__restrict argv) {
 struct type_method *chain = tp_self->tp_methods;
 for (; chain->m_name; ++chain) {
  if (strcmp(chain->m_name,attr_name) != 0) continue;
  membercache_addinstancemethod(&tp_invoker->tp_class_cache,tp_self,hash,chain);
  return type_obmeth_call(tp_self,chain,argc,argv);
 }
 return ITER_DONE;
}

INTERN DREF DeeObject *DCALL
type_method_callattr_kw(struct membercache *__restrict cache, DeeTypeObject *__restrict decl,
                        struct type_method *__restrict chain, DeeObject *__restrict self,
                        char const *__restrict attr_name, dhash_t hash,
                        size_t argc, DeeObject **__restrict argv, DeeObject *kw) {
 for (; chain->m_name; ++chain) {
  if (strcmp(chain->m_name,attr_name) != 0) continue;
  membercache_addmethod(cache,decl,hash,chain);
  return type_method_call_kw(chain,self,argc,argv,kw);
 }
 return ITER_DONE;
}

INTERN DREF DeeObject *
(DCALL DeeType_CallInstanceMethodAttrKw)(DeeTypeObject *__restrict tp_invoker,
                                         DeeTypeObject *__restrict tp_self,
                                         char const *__restrict attr_name, dhash_t hash,
                                         size_t argc, DeeObject **__restrict argv,
                                         DeeObject *kw) {
 struct type_method *chain = tp_self->tp_methods;
 for (; chain->m_name; ++chain) {
  if (strcmp(chain->m_name,attr_name) != 0) continue;
  membercache_addinstancemethod(&tp_invoker->tp_class_cache,tp_self,hash,chain);
  return type_obmeth_call_kw(tp_self,chain,argc,argv,kw);
 }
 return ITER_DONE;
}
INTERN DREF DeeObject *
(DCALL DeeType_CallIInstanceMethodAttrKw)(DeeTypeObject *__restrict tp_invoker,
                                          DeeTypeObject *__restrict tp_self,
                                          char const *__restrict attr_name, dhash_t hash,
                                          size_t argc, DeeObject **__restrict argv,
                                          DeeObject *kw) {
 struct type_method *chain = tp_self->tp_methods;
 for (; chain->m_name; ++chain) {
  if (strcmp(chain->m_name,attr_name) != 0) continue;
  membercache_addmethod(&tp_invoker->tp_cache,tp_self,hash,chain);
  return type_obmeth_call_kw(tp_self,chain,argc,argv,kw);
 }
 return ITER_DONE;
}

INTERN DREF DeeObject *DCALL
type_instance_method_callattr_kw(struct membercache *__restrict cache,
                                 DeeTypeObject *__restrict decl,
                                 struct type_method *__restrict chain,
                                 char const *__restrict attr_name, dhash_t hash,
                                 size_t argc, DeeObject **__restrict argv, DeeObject *kw) {
 for (; chain->m_name; ++chain) {
  if (strcmp(chain->m_name,attr_name) != 0) continue;
  membercache_addinstancemethod(cache,decl,hash,chain);
  return type_obmeth_call_kw(decl,chain,argc,argv,kw);
 }
 return ITER_DONE;
}

INTERN DREF DeeObject *DCALL
type_method_vcallattrf(struct membercache *__restrict cache, DeeTypeObject *__restrict decl,
                       struct type_method *__restrict chain, DeeObject *__restrict self,
                       char const *__restrict attr_name, dhash_t hash,
                       char const *__restrict format, va_list args) {
 DREF DeeObject *result,*args_tuple;
 for (; chain->m_name; ++chain) {
  if (strcmp(chain->m_name,attr_name) != 0) continue;
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
                                         char const *__restrict attr_name, dhash_t hash,
                                         char const *__restrict format, va_list args) {
 DREF DeeObject *result,*args_tuple;
 struct type_method *chain = tp_self->tp_methods;
 for (; chain->m_name; ++chain) {
  if (strcmp(chain->m_name,attr_name) != 0) continue;
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
                                          char const *__restrict attr_name, dhash_t hash,
                                          char const *__restrict format, va_list args) {
 DREF DeeObject *result,*args_tuple;
 struct type_method *chain = tp_self->tp_methods;
 for (; chain->m_name; ++chain) {
  if (strcmp(chain->m_name,attr_name) != 0) continue;
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


INTERN DREF DeeObject *DCALL /* GET_GETSET */
type_getset_getattr(struct membercache *__restrict cache, DeeTypeObject *__restrict decl,
                    struct type_getset *__restrict chain, DeeObject *__restrict self,
                    char const *__restrict attr_name, dhash_t hash) {
 for (; chain->gs_name; ++chain) {
  if (strcmp(chain->gs_name,attr_name) != 0) continue;
  membercache_addgetset(cache,decl,hash,chain);
  return type_getset_get(chain,self);
 }
 return ITER_DONE;
}

INTERN DREF DeeObject *
(DCALL DeeType_GetInstanceGetSetAttr)(DeeTypeObject *__restrict tp_invoker,
                                      DeeTypeObject *__restrict tp_self,
                                      char const *__restrict attr_name, dhash_t hash) {
 struct type_getset *chain = tp_self->tp_getsets;
 for (; chain->gs_name; ++chain) {
  if (strcmp(chain->gs_name,attr_name) != 0) continue;
  membercache_addinstancegetset(&tp_invoker->tp_class_cache,tp_self,hash,chain);
  return type_obprop_get(tp_self,chain);
 }
 return ITER_DONE;
}
INTERN DREF DeeObject *
(DCALL DeeType_GetIInstanceGetSetAttr)(DeeTypeObject *__restrict tp_invoker,
                                       DeeTypeObject *__restrict tp_self,
                                       char const *__restrict attr_name, dhash_t hash) {
 struct type_getset *chain = tp_self->tp_getsets;
 for (; chain->gs_name; ++chain) {
  if (strcmp(chain->gs_name,attr_name) != 0) continue;
  membercache_addgetset(&tp_invoker->tp_cache,tp_self,hash,chain);
  return type_obprop_get(tp_self,chain);
 }
 return ITER_DONE;
}
INTERN DREF DeeObject *
(DCALL DeeType_CallInstanceGetSetAttr)(DeeTypeObject *__restrict tp_invoker,
                                       DeeTypeObject *__restrict tp_self,
                                       char const *__restrict attr_name, dhash_t hash,
                                       size_t argc, DeeObject **__restrict argv) {
 struct type_getset *chain = tp_self->tp_getsets;
 for (; chain->gs_name; ++chain) {
  if (strcmp(chain->gs_name,attr_name) != 0) continue;
  membercache_addinstancegetset(&tp_invoker->tp_class_cache,tp_self,hash,chain);
  return type_obprop_call(tp_self,chain,argc,argv);
 }
 return ITER_DONE;
}
#if 0
INTERN DREF DeeObject *
(DCALL DeeType_CallIInstanceGetSetAttr)(DeeTypeObject *__restrict tp_invoker,
                                        DeeTypeObject *__restrict tp_self,
                                        char const *__restrict attr_name, dhash_t hash,
                                        size_t argc, DeeObject **__restrict argv) {
 struct type_getset *chain = tp_self->tp_getsets;
 for (; chain->gs_name; ++chain) {
  if (strcmp(chain->gs_name,attr_name) != 0) continue;
  membercache_addgetset(&tp_invoker->tp_cache,tp_self,hash,chain);
  return type_obprop_call(tp_self,chain,argc,argv);
 }
 return ITER_DONE;
}
#endif
INTERN DREF DeeObject *
(DCALL DeeType_CallInstanceGetSetAttrKw)(DeeTypeObject *__restrict tp_invoker,
                                         DeeTypeObject *__restrict tp_self,
                                         char const *__restrict attr_name, dhash_t hash,
                                         size_t argc, DeeObject **__restrict argv,
                                         DeeObject *kw) {
 struct type_getset *chain = tp_self->tp_getsets;
 for (; chain->gs_name; ++chain) {
  if (strcmp(chain->gs_name,attr_name) != 0) continue;
  membercache_addinstancegetset(&tp_invoker->tp_class_cache,tp_self,hash,chain);
  return type_obprop_call_kw(tp_self,chain,argc,argv,kw);
 }
 return ITER_DONE;
}
INTERN DREF DeeObject *
(DCALL DeeType_CallIInstanceGetSetAttrKw)(DeeTypeObject *__restrict tp_invoker,
                                          DeeTypeObject *__restrict tp_self,
                                          char const *__restrict attr_name, dhash_t hash,
                                          size_t argc, DeeObject **__restrict argv,
                                          DeeObject *kw) {
 struct type_getset *chain = tp_self->tp_getsets;
 for (; chain->gs_name; ++chain) {
  if (strcmp(chain->gs_name,attr_name) != 0) continue;
  membercache_addgetset(&tp_invoker->tp_cache,tp_self,hash,chain);
  return type_obprop_call_kw(tp_self,chain,argc,argv,kw);
 }
 return ITER_DONE;
}


INTERN DREF DeeObject *DCALL /* DOC_GETSET */
type_getset_docattr(struct membercache *__restrict cache,
                    DeeTypeObject *__restrict decl,
                    struct type_getset *__restrict chain,
                    char const *__restrict attr_name, dhash_t hash) {
 for (; chain->gs_name; ++chain) {
  if (strcmp(chain->gs_name,attr_name) != 0) continue;
  membercache_addgetset(cache,decl,hash,chain);
  return type_getset_doc(chain);
 }
 return ITER_DONE;
}
INTERN DREF DeeObject *
(DCALL DeeType_DocInstanceGetSetAttr)(DeeTypeObject *__restrict tp_invoker,
                                      DeeTypeObject *__restrict tp_self,
                                      char const *__restrict attr_name,
                                      dhash_t hash) {
 struct type_getset *chain = tp_self->tp_getsets;
 for (; chain->gs_name; ++chain) {
  if (strcmp(chain->gs_name,attr_name) != 0) continue;
  membercache_addinstancegetset(&tp_invoker->tp_class_cache,tp_self,hash,chain);
  return type_getset_doc(chain);
 }
 return ITER_DONE;
}

INTERN int DCALL /* BOUND_GETSET */
type_getset_boundattr(struct membercache *__restrict cache, DeeTypeObject *__restrict decl,
                      struct type_getset *__restrict chain, DeeObject *__restrict self,
                      char const *__restrict attr_name, dhash_t hash) {
 DREF DeeObject *temp;
 for (; chain->gs_name; ++chain) {
  if (strcmp(chain->gs_name,attr_name) != 0) continue;
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
type_getset_delattr(struct membercache *__restrict cache, DeeTypeObject *__restrict decl,
                    struct type_getset *__restrict chain, DeeObject *__restrict self,
                    char const *__restrict attr_name, dhash_t hash) {
 for (; chain->gs_name; ++chain) {
  if (strcmp(chain->gs_name,attr_name) != 0) continue;
  membercache_addgetset(cache,decl,hash,chain);
  return type_getset_del(chain,self);
 }
 return 1;
}

INTERN int DCALL /* SET_GETSET */
type_getset_setattr(struct membercache *__restrict cache, DeeTypeObject *__restrict decl,
                    struct type_getset *__restrict chain, DeeObject *__restrict self,
                    char const *__restrict attr_name, dhash_t hash, DeeObject *__restrict value) {
 for (; chain->gs_name; ++chain) {
  if (strcmp(chain->gs_name,attr_name) != 0) continue;
  membercache_addgetset(cache,decl,hash,chain);
  return type_getset_set(chain,self,value);
 }
 return 1;
}


INTERN DREF DeeObject *DCALL /* GET_MEMBER */
type_member_getattr(struct membercache *__restrict cache, DeeTypeObject *__restrict decl,
                    struct type_member *__restrict chain, DeeObject *__restrict self,
                    char const *__restrict attr_name, dhash_t hash) {
 for (; chain->m_name; ++chain) {
  if (strcmp(chain->m_name,attr_name) != 0) continue;
  membercache_addmember(cache,decl,hash,chain);
  return type_member_get(chain,self);
 }
 return ITER_DONE;
}

INTERN DREF DeeObject *
(DCALL DeeType_GetInstanceMemberAttr)(DeeTypeObject *__restrict tp_invoker,
                                      DeeTypeObject *__restrict tp_self,
                                      char const *__restrict attr_name, dhash_t hash) {
 struct type_member *chain = tp_self->tp_members;
 for (; chain->m_name; ++chain) {
  if (strcmp(chain->m_name,attr_name) != 0) continue;
  membercache_addinstancemember(&tp_invoker->tp_class_cache,tp_self,hash,chain);
  return type_obmemb_get(tp_self,chain);
 }
 return ITER_DONE;
}
INTERN DREF DeeObject *
(DCALL DeeType_GetIInstanceMemberAttr)(DeeTypeObject *__restrict tp_invoker,
                                       DeeTypeObject *__restrict tp_self,
                                       char const *__restrict attr_name, dhash_t hash) {
 struct type_member *chain = tp_self->tp_members;
 for (; chain->m_name; ++chain) {
  if (strcmp(chain->m_name,attr_name) != 0) continue;
  membercache_addmember(&tp_invoker->tp_cache,tp_self,hash,chain);
  return type_obmemb_get(tp_self,chain);
 }
 return ITER_DONE;
}
INTERN DREF DeeObject *
(DCALL DeeType_CallInstanceMemberAttr)(DeeTypeObject *__restrict tp_invoker,
                                       DeeTypeObject *__restrict tp_self,
                                       char const *__restrict attr_name, dhash_t hash,
                                       size_t argc, DeeObject **__restrict argv) {
 struct type_member *chain = tp_self->tp_members;
 for (; chain->m_name; ++chain) {
  if (strcmp(chain->m_name,attr_name) != 0) continue;
  membercache_addinstancemember(&tp_invoker->tp_class_cache,tp_self,hash,chain);
  return type_obmemb_call(tp_self,chain,argc,argv);
 }
 return ITER_DONE;
}
#if 0
INTERN DREF DeeObject *
(DCALL DeeType_CallIInstanceMemberAttr)(DeeTypeObject *__restrict tp_invoker,
                                        DeeTypeObject *__restrict tp_self,
                                        char const *__restrict attr_name, dhash_t hash,
                                        size_t argc, DeeObject **__restrict argv) {
 struct type_member *chain = tp_self->tp_members;
 for (; chain->m_name; ++chain) {
  if (strcmp(chain->m_name,attr_name) != 0) continue;
  membercache_addmember(&tp_invoker->tp_cache,tp_self,hash,chain);
  return type_obmemb_call(tp_self,chain,argc,argv);
 }
 return ITER_DONE;
}
#endif
INTERN DREF DeeObject *
(DCALL DeeType_CallInstanceMemberAttrKw)(DeeTypeObject *__restrict tp_invoker,
                                         DeeTypeObject *__restrict tp_self,
                                         char const *__restrict attr_name, dhash_t hash,
                                         size_t argc, DeeObject **__restrict argv,
                                         DeeObject *kw) {
 struct type_member *chain = tp_self->tp_members;
 for (; chain->m_name; ++chain) {
  if (strcmp(chain->m_name,attr_name) != 0) continue;
  membercache_addinstancemember(&tp_invoker->tp_class_cache,tp_self,hash,chain);
  return type_obmemb_call_kw(tp_self,chain,argc,argv,kw);
 }
 return ITER_DONE;
}
INTERN DREF DeeObject *
(DCALL DeeType_CallIInstanceMemberAttrKw)(DeeTypeObject *__restrict tp_invoker,
                                          DeeTypeObject *__restrict tp_self,
                                          char const *__restrict attr_name, dhash_t hash,
                                          size_t argc, DeeObject **__restrict argv,
                                          DeeObject *kw) {
 struct type_member *chain = tp_self->tp_members;
 for (; chain->m_name; ++chain) {
  if (strcmp(chain->m_name,attr_name) != 0) continue;
  membercache_addmember(&tp_invoker->tp_cache,tp_self,hash,chain);
  return type_obmemb_call_kw(tp_self,chain,argc,argv,kw);
 }
 return ITER_DONE;
}



INTERN DREF DeeObject *DCALL /* DOC_MEMBER */
type_member_docattr(struct membercache *__restrict cache,
                    DeeTypeObject *__restrict decl,
                    struct type_member *__restrict chain,
                    char const *__restrict attr_name, dhash_t hash) {
 for (; chain->m_name; ++chain) {
  if (strcmp(chain->m_name,attr_name) != 0) continue;
  membercache_addmember(cache,decl,hash,chain);
  return type_member_doc(chain);
 }
 return ITER_DONE;
}
INTERN DREF DeeObject *
(DCALL DeeType_DocInstanceMemberAttr)(DeeTypeObject *__restrict tp_invoker,
                                      DeeTypeObject *__restrict tp_self,
                                      char const *__restrict attr_name,
                                      dhash_t hash) {
 struct type_member *chain = tp_self->tp_members;
 for (; chain->m_name; ++chain) {
  if (strcmp(chain->m_name,attr_name) != 0) continue;
  membercache_addinstancemember(&tp_invoker->tp_class_cache,tp_self,hash,chain);
  return type_member_doc(chain);
 }
 return ITER_DONE;
}

INTERN int DCALL /* BOUND_MEMBER */
type_member_boundattr(struct membercache *__restrict cache, DeeTypeObject *__restrict decl,
                      struct type_member *__restrict chain, DeeObject *__restrict self,
                      char const *__restrict attr_name, dhash_t hash) {
 for (; chain->m_name; ++chain) {
  if (strcmp(chain->m_name,attr_name) != 0) continue;
  membercache_addmember(cache,decl,hash,chain);
  return type_member_bound(chain,self);
 }
 return -2;
}

INTERN int DCALL /* DEL_MEMBER */
type_member_delattr(struct membercache *__restrict cache, DeeTypeObject *__restrict decl,
                    struct type_member *__restrict chain, DeeObject *__restrict self,
                    char const *__restrict attr_name, dhash_t hash) {
 for (; chain->m_name; ++chain) {
  if (strcmp(chain->m_name,attr_name) != 0) continue;
  membercache_addmember(cache,decl,hash,chain);
  return type_member_del(chain,self);
 }
 return 1;
}

INTERN int DCALL /* SET_MEMBER */
type_member_setattr(struct membercache *__restrict cache, DeeTypeObject *__restrict decl,
                    struct type_member *__restrict chain, DeeObject *__restrict self,
                    char const *__restrict attr_name, dhash_t hash, DeeObject *__restrict value) {
 for (; chain->m_name; ++chain) {
  if (strcmp(chain->m_name,attr_name) != 0) continue;
  membercache_addmember(cache,decl,hash,chain);
  return type_member_set(chain,self,value);
 }
 return 1;
}


INTERN bool DCALL /* METHOD */
type_method_hasattr(struct membercache *__restrict cache,
                    DeeTypeObject *__restrict decl,
                    struct type_method *__restrict chain,
                    char const *__restrict attr_name, dhash_t hash) {
 for (; chain->m_name; ++chain) {
  if (strcmp(chain->m_name,attr_name) != 0) continue;
  membercache_addmethod(cache,decl,hash,chain);
  return true;
 }
 return false;
}
INTERN bool
(DCALL DeeType_HasInstanceMethodAttr)(DeeTypeObject *__restrict tp_invoker,
                                      DeeTypeObject *__restrict tp_self,
                                      char const *__restrict attr_name, dhash_t hash) {
 struct type_method *chain = tp_self->tp_methods;
 for (; chain->m_name; ++chain) {
  if (strcmp(chain->m_name,attr_name) != 0) continue;
  membercache_addinstancemethod(&tp_invoker->tp_class_cache,tp_self,hash,chain);
  return true;
 }
 return false;
}

INTERN bool DCALL /* GETSET */
type_getset_hasattr(struct membercache *__restrict cache,
                    DeeTypeObject *__restrict decl,
                    struct type_getset *__restrict chain,
                    char const *__restrict attr_name, dhash_t hash) {
 for (; chain->gs_name; ++chain) {
  if (strcmp(chain->gs_name,attr_name) != 0) continue;
  membercache_addgetset(cache,decl,hash,chain);
  return true;
 }
 return false;
}
INTERN bool
(DCALL DeeType_HasInstanceGetSetAttr)(DeeTypeObject *__restrict tp_invoker,
                                      DeeTypeObject *__restrict tp_self,
                                      char const *__restrict attr_name,
                                      dhash_t hash) {
 struct type_getset *chain = tp_self->tp_getsets;
 for (; chain->gs_name; ++chain) {
  if (strcmp(chain->gs_name,attr_name) != 0) continue;
  membercache_addinstancegetset(&tp_invoker->tp_class_cache,tp_self,hash,chain);
  return true;
 }
 return false;
}

INTERN bool DCALL /* MEMBER */
type_member_hasattr(struct membercache *__restrict cache,
                    DeeTypeObject *__restrict decl,
                    struct type_member *__restrict chain,
                    char const *__restrict attr_name, dhash_t hash) {
 for (; chain->m_name; ++chain) {
  if (strcmp(chain->m_name,attr_name) != 0) continue;
  membercache_addmember(cache,decl,hash,chain);
  return true;
 }
 return false;
}
INTERN bool
(DCALL DeeType_HasInstanceMemberAttr)(DeeTypeObject *__restrict tp_invoker,
                                      DeeTypeObject *__restrict tp_self,
                                      char const *__restrict attr_name,
                                      dhash_t hash) {
 struct type_member *chain = tp_self->tp_members;
 for (; chain->m_name; ++chain) {
  if (strcmp(chain->m_name,attr_name) != 0) continue;
  membercache_addinstancemember(&tp_invoker->tp_class_cache,tp_self,hash,chain);
  return true;
 }
 return false;
}


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
  if (strcmp(chain->m_name,rules->alr_name)) continue;
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
  if (strcmp(chain->m_name,rules->alr_name)) continue;
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
  if (strcmp(chain->gs_name,rules->alr_name)) continue;
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
  if (strcmp(chain->gs_name,rules->alr_name)) continue;
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
  if (strcmp(chain->m_name,rules->alr_name)) continue;
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
  if (strcmp(chain->m_name,rules->alr_name)) continue;
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

#else /* CONFIG_USE_NEW_TYPE_ATTRIBUTE_CACHING */

PRIVATE uint8_t const info_size[MEMBERCACHE_COUNT] = {
   /* [MEMBERCACHE_UNUSED]          = */0,
   /* [MEMBERCACHE_METHOD]          = */sizeof(struct type_method),
   /* [MEMBERCACHE_GETSET]          = */sizeof(struct type_getset),
   /* [MEMBERCACHE_MEMBER]          = */sizeof(struct type_member),
   /* [MEMBERCACHE_ATTRIB]          = */sizeof(((struct membercache_slot *)0)->mcs_attrib),
   /* [MEMBERCACHE_INSTANCE_METHOD] = */sizeof(struct type_method),
   /* [MEMBERCACHE_INSTANCE_GETSET] = */sizeof(struct type_getset),
   /* [MEMBERCACHE_INSTANCE_MEMBER] = */sizeof(struct type_member),
   /* [MEMBERCACHE_INSTANCE_ATTRIB] = */sizeof(((struct membercache_slot *)0)->mcs_attrib),
};

PRIVATE void DCALL
membercache_add(struct membercache *__restrict cache,
                char const *__restrict name, dhash_t hash,
                uint16_t type, void *__restrict data) {
 dhash_t i,perturb;
 struct membercache_slot *item;
 ASSERT(type < COMPILER_LENOF(info_size));
#ifndef CONFIG_NO_THREADS
#if 1
 /* Since this cache is totally optional, don't slow down when we can't get the lock. */
 if (!rwlock_trywrite(&cache->mc_lock))
      return;
#else
 MEMBERCACHE_WRITE(cache);
#endif
#endif
again:
 if (!cache->mc_table) goto rehash_initial;
 /* Re-check that the named attribute isn't already in-cache. */
 perturb = i = MEMBERCACHE_HASHST(cache,hash);
 for (;; i = MEMBERCACHE_HASHNX(i,perturb),MEMBERCACHE_HASHPT(perturb)) {
  item = MEMBERCACHE_HASHIT(cache,i);
  if (item->mcs_type == MEMBERCACHE_UNUSED) break;
  if (item->mcs_hash != hash) continue;
  if (strcmp(item->mcs_name,name)) continue;
  /* Already in cache! */
  MEMBERCACHE_ENDWRITE(cache);
  return;
 }
 if (cache->mc_size+1 >= cache->mc_mask) {
rehash_initial:
  if (membercache_rehash(cache)) goto again;
  goto done; /* Well... We couldn't rehash the cache so we can't add this entry. */
 }
 /* Not found. - Use this empty slot. */
 item->mcs_type = type;
 item->mcs_hash = hash;
 memcpy(&item->mcs_name,data,info_size[type]);
 ASSERT(!strcmp(item->mcs_name,name));
 ++cache->mc_size;
 /* Try to keep the table vector big at least twice as big as the element count. */
 if (cache->mc_size*2 > cache->mc_mask)
     membercache_rehash(cache);
done:
 MEMBERCACHE_ENDWRITE(cache);
}

PRIVATE struct type_method *DCALL
methods_find(struct type_method *__restrict chain, char const *__restrict name) {
 for (; chain->m_name; ++chain) if (!strcmp(chain->m_name,name)) return chain;
 return NULL;
}
PRIVATE struct type_getset *DCALL
getsets_find(struct type_getset *__restrict chain, char const *__restrict name) {
 for (; chain->gs_name; ++chain) if (!strcmp(chain->gs_name,name)) return chain;
 return NULL;
}
PRIVATE struct type_member *DCALL
members_find(struct type_member *__restrict chain, char const *__restrict name) {
 for (; chain->m_name; ++chain) if (!strcmp(chain->m_name,name)) return chain;
 return NULL;
}


PRIVATE DeeTypeObject *DCALL
find_type_for_getset(DeeObject *__restrict self,
                     char const *__restrict name) {
 DeeTypeObject *result = Dee_TYPE(self);
 do {
  if (result->tp_getsets && getsets_find(result->tp_getsets,name))
      return result;
 } while ((result = DeeType_Base(result)) != NULL);
 if (DeeType_Check(self)) {
  result = (DeeTypeObject *)self;
  do {
   if (result->tp_class_getsets &&
       getsets_find(result->tp_class_getsets,name))
       return result;
  } while ((result = DeeType_Base(result)) != NULL);
  return (DeeTypeObject *)self;
 }
 return Dee_TYPE(self);
}
PRIVATE DeeTypeObject *DCALL
find_type_for_method(DeeObject *__restrict self,
                     char const *__restrict name) {
 DeeTypeObject *result = Dee_TYPE(self);
 do {
  if (result->tp_methods && methods_find(result->tp_methods,name))
      return result;
 } while ((result = DeeType_Base(result)) != NULL);
 if (DeeType_Check(self)) {
  result = (DeeTypeObject *)self;
  do {
   if (result->tp_class_methods &&
       methods_find(result->tp_class_methods,name))
       return result;
  } while ((result = DeeType_Base(result)) != NULL);
  return (DeeTypeObject *)self;
 }
 return Dee_TYPE(self);
}
#if 0
PRIVATE DeeTypeObject *DCALL
find_type_for_member(DeeObject *__restrict self,
                     char const *__restrict name) {
 DeeTypeObject *result = Dee_TYPE(self);
 do {
  if (result->tp_members && members_find(result->tp_members,name))
      return result;
 } while ((result = DeeType_Base(result)) != NULL);
 if (DeeType_Check(self)) {
  result = (DeeTypeObject *)self;
  do {
   if (result->tp_class_members &&
       members_find(result->tp_class_members,name))
       return result;
  } while ((result = DeeType_Base(result)) != NULL);
  return (DeeTypeObject *)self;
 }
 return Dee_TYPE(self);
}
#endif


PRIVATE DeeTypeObject *DCALL
find_type_for_class_getset(DeeTypeObject *__restrict self,
                           char const *__restrict name) {
 DeeTypeObject *result = self;
 do {
  if (result->tp_class_getsets &&
      getsets_find(result->tp_class_getsets,name))
      return result;
 } while ((result = DeeType_Base(result)) != NULL);
 return self;
}

PRIVATE DeeTypeObject *DCALL
find_type_for_class_method(DeeTypeObject *__restrict self,
                           char const *__restrict name) {
 DeeTypeObject *result = self;
 do {
  if (result->tp_class_methods &&
      methods_find(result->tp_class_methods,name))
      return result;
 } while ((result = DeeType_Base(result)) != NULL);
 return self;
}
PRIVATE DeeTypeObject *DCALL
find_type_for_instance_member(DeeTypeObject *__restrict self,
                              char const *__restrict name) {
 DeeTypeObject *result = self;
 do {
  if (result->tp_class_members &&
      members_find(result->tp_class_members,name))
      return result;
 } while ((result = DeeType_Base(result)) != NULL);
 return self;
}

/* Lookup an attribute from cache.
 * @return: * :        The attribute value.
 * @return: NULL:      An error occurred.
 * @return: ITER_DONE: The attribute could not be found in the cache. */
INTERN DREF DeeObject *DCALL
membercache_getattr(struct membercache *__restrict cache,
                    DeeObject *__restrict self,
                    char const *__restrict name, dhash_t hash) {
 dhash_t i,perturb;
 MEMBERCACHE_READ(cache);
 if unlikely(!cache->mc_table) goto done;
 perturb = i = MEMBERCACHE_HASHST(cache,hash);
 for (;; i = MEMBERCACHE_HASHNX(i,perturb),MEMBERCACHE_HASHPT(perturb)) {
  struct membercache_slot *item = MEMBERCACHE_HASHIT(cache,i);
  if (item->mcs_type == MEMBERCACHE_UNUSED) break;
  if (item->mcs_hash != hash) continue;
  if (strcmp(item->mcs_name,name)) continue;
  switch (item->mcs_type) {
  {
   dobjmethod_t func;
  case MEMBERCACHE_METHOD:
   func = item->mcs_method.m_func;
   if (item->mcs_method.m_flag & TYPE_METHOD_FKWDS) {
    MEMBERCACHE_ENDREAD(cache);
    return DeeKwObjMethod_New((dkwobjmethod_t)func,self);
   }
   MEMBERCACHE_ENDREAD(cache);
   return DeeObjMethod_New(func,self);
  }
  {
   DREF DeeObject *(DCALL *getter)(DeeObject *__restrict self);
  case MEMBERCACHE_GETSET:
   getter = item->mcs_getset.gs_get;
   MEMBERCACHE_ENDREAD(cache);
   if likely(getter) return (*getter)(self);
   err_cant_access_attribute(find_type_for_getset(self,name),
                             name,ATTR_ACCESS_GET);
   return NULL;
  }
  {
   struct buffer { uint8_t dat[COMPILER_OFFSETOF(struct type_member,m_doc)]; } buf;
  case MEMBERCACHE_MEMBER:
   buf = *(struct buffer *)&item->mcs_member;
   MEMBERCACHE_ENDREAD(cache);
   return type_member_get((struct type_member *)&buf,self);
  }
  {
   struct class_attribute *attr;
   struct class_desc *desc;
  case MEMBERCACHE_ATTRIB:
   attr = item->mcs_attrib.a_attr;
   desc = item->mcs_attrib.a_desc;
   MEMBERCACHE_ENDREAD(cache);
   return DeeInstance_GetAttribute(desc,
                                   DeeInstance_DESC(desc,self),
                                   self,attr);
  }
  default: __builtin_unreachable();
  }
 }
done:
 MEMBERCACHE_ENDREAD(cache);
 return ITER_DONE;
}

/* @return: 1 : Attribute is bound.
 * @return: 0 : Attribute isn't bound.
 * @return: -1: An error occurred.
 * @return: -2: The attribute doesn't exist. */
INTERN int DCALL
membercache_boundattr(struct membercache *__restrict cache,
                      DeeObject *__restrict self,
                      char const *__restrict name, dhash_t hash) {
 dhash_t i,perturb;
 MEMBERCACHE_READ(cache);
 if unlikely(!cache->mc_table) goto done;
 perturb = i = MEMBERCACHE_HASHST(cache,hash);
 for (;; i = MEMBERCACHE_HASHNX(i,perturb),MEMBERCACHE_HASHPT(perturb)) {
  struct membercache_slot *item = MEMBERCACHE_HASHIT(cache,i);
  if (item->mcs_type == MEMBERCACHE_UNUSED) break;
  if (item->mcs_hash != hash) continue;
  if (strcmp(item->mcs_name,name)) continue;
  switch (item->mcs_type) {
  case MEMBERCACHE_METHOD:
   MEMBERCACHE_ENDREAD(cache);
   return 1;
  {
   DREF DeeObject *(DCALL *getter)(DeeObject *__restrict self);
  case MEMBERCACHE_GETSET:
   getter = item->mcs_getset.gs_get;
   MEMBERCACHE_ENDREAD(cache);
   if likely(getter) {
    DREF DeeObject *temp;
    temp = (*getter)(self);
    if unlikely(!temp) {
     if (CATCH_ATTRIBUTE_ERROR())
         return -3;
     if (DeeError_Catch(&DeeError_UnboundAttribute))
         return 0;
     return -1;
    }
    Dee_Decref(temp);
    return 1;
   }
   return 0;
  }
  {
   struct buffer { uint8_t dat[COMPILER_OFFSETOF(struct type_member,m_doc)]; } buf;
  case MEMBERCACHE_MEMBER:
   buf = *(struct buffer *)&item->mcs_member;
   MEMBERCACHE_ENDREAD(cache);
   return type_member_bound((struct type_member *)&buf,self);
  }
  {
   struct class_attribute *attr;
   struct class_desc *desc;
  case MEMBERCACHE_ATTRIB:
   attr = item->mcs_attrib.a_attr;
   desc = item->mcs_attrib.a_desc;
   MEMBERCACHE_ENDREAD(cache);
   return DeeInstance_BoundAttribute(desc,
                                     DeeInstance_DESC(desc,self),
                                     self,attr);
  }
  default: __builtin_unreachable();
  }
 }
done:
 MEMBERCACHE_ENDREAD(cache);
 return -2;
}


INTERN DREF DeeObject *DCALL
membercache_docattr(struct membercache *__restrict cache, char const *base,
                    char const *__restrict name, dhash_t hash) {
 dhash_t i,perturb; char const *cstring;
 MEMBERCACHE_READ(cache);
 if unlikely(!cache->mc_table) goto done;
 perturb = i = MEMBERCACHE_HASHST(cache,hash);
 for (;; i = MEMBERCACHE_HASHNX(i,perturb),MEMBERCACHE_HASHPT(perturb)) {
  struct membercache_slot *item = MEMBERCACHE_HASHIT(cache,i);
  if (item->mcs_type == MEMBERCACHE_UNUSED) break;
  if (item->mcs_hash != hash) continue;
  if (strcmp(item->mcs_name,name)) continue;
  switch (item->mcs_type) {
  case MEMBERCACHE_METHOD:
   cstring = item->mcs_method.m_doc;
   break;
  case MEMBERCACHE_GETSET:
   cstring = item->mcs_getset.gs_doc;
   break;
  case MEMBERCACHE_MEMBER:
   cstring = item->mcs_member.m_doc;
   break;
  {
   struct class_attribute *attr;
  case MEMBERCACHE_ATTRIB:
   attr = item->mcs_attrib.a_attr;
   MEMBERCACHE_ENDREAD(cache);
   if unlikely(!attr->ca_doc) goto err_no_doc;
   return_reference_((DeeObject *)attr->ca_doc);
  }
  default: __builtin_unreachable();
  }
  MEMBERCACHE_ENDREAD(cache);
  if unlikely(!cstring) goto err_no_doc;
  return DeeString_NewUtf8(cstring,strlen(cstring),STRING_ERROR_FIGNORE);
 }
done:
 MEMBERCACHE_ENDREAD(cache);
 return ITER_DONE;
err_no_doc:
 err_nodoc_attribute(base,name);
 return NULL;
}

INTERN DREF DeeObject *DCALL
membercache_callattr(struct membercache *__restrict cache,
                     DeeObject *__restrict self,
                     char const *__restrict name, dhash_t hash,
                     size_t argc, DeeObject **__restrict argv) {
 dhash_t i,perturb;
 DREF DeeObject *result;
 MEMBERCACHE_READ(cache);
 if unlikely(!cache->mc_table) goto done;
 perturb = i = MEMBERCACHE_HASHST(cache,hash);
 for (;; i = MEMBERCACHE_HASHNX(i,perturb),MEMBERCACHE_HASHPT(perturb)) {
  struct membercache_slot *item = MEMBERCACHE_HASHIT(cache,i);
  if (item->mcs_type == MEMBERCACHE_UNUSED) break;
  if (item->mcs_hash != hash) continue;
  if (strcmp(item->mcs_name,name)) continue;
  switch (item->mcs_type) {
  {
   dobjmethod_t func;
  case MEMBERCACHE_METHOD:
   func = item->mcs_method.m_func;
   if (item->mcs_method.m_flag & TYPE_METHOD_FKWDS) {
    MEMBERCACHE_ENDREAD(cache);
    return DeeKwObjMethod_CallFunc((dkwobjmethod_t)func,self,argc,argv,NULL);
   }
   MEMBERCACHE_ENDREAD(cache);
   return DeeObjMethod_CallFunc(func,self,argc,argv);
  }
  {
   DREF DeeObject *(DCALL *getter)(DeeObject *__restrict self);
  case MEMBERCACHE_GETSET:
   getter = item->mcs_getset.gs_get;
   MEMBERCACHE_ENDREAD(cache);
   if likely(getter) { result = (*getter)(self); goto invoke_result; }
   err_cant_access_attribute(find_type_for_getset(self,name),
                             name,ATTR_ACCESS_GET);
   goto err;
  }
  {
   struct buffer { uint8_t dat[COMPILER_OFFSETOF(struct type_member,m_doc)]; } buf;
  case MEMBERCACHE_MEMBER:
   buf = *(struct buffer *)&item->mcs_member;
   MEMBERCACHE_ENDREAD(cache);
   result = type_member_get((struct type_member *)&buf,self);
   goto invoke_result;
  }
  {
   struct class_attribute *attr;
   struct class_desc *desc;
  case MEMBERCACHE_ATTRIB:
   attr = item->mcs_attrib.a_attr;
   desc = item->mcs_attrib.a_desc;
   MEMBERCACHE_ENDREAD(cache);
   return DeeInstance_CallAttribute(desc,
                                    DeeInstance_DESC(desc,self),
                                    self,attr,argc,argv);
  }
  default: __builtin_unreachable();
  }
 }
done:
 MEMBERCACHE_ENDREAD(cache);
 return ITER_DONE;
invoke_result:
 if likely(result) {
  DREF DeeObject *callback_result;
  callback_result = DeeObject_Call(result,argc,argv);
  Dee_Decref(result);
  result = callback_result;
 }
 return result;
err:
 return NULL;
}

INTERN DREF DeeObject *DCALL
membercache_callattr_kw(struct membercache *__restrict cache,
                        DeeObject *__restrict self,
                        char const *__restrict name, dhash_t hash,
                        size_t argc, DeeObject **__restrict argv,
                        DeeObject *kw) {
 dhash_t i,perturb;
 DREF DeeObject *result;
 MEMBERCACHE_READ(cache);
 if unlikely(!cache->mc_table) goto done;
 perturb = i = MEMBERCACHE_HASHST(cache,hash);
 for (;; i = MEMBERCACHE_HASHNX(i,perturb),MEMBERCACHE_HASHPT(perturb)) {
  struct membercache_slot *item = MEMBERCACHE_HASHIT(cache,i);
  if (item->mcs_type == MEMBERCACHE_UNUSED) break;
  if (item->mcs_hash != hash) continue;
  if (strcmp(item->mcs_name,name)) continue;
  switch (item->mcs_type) {
  {
   dobjmethod_t func;
  case MEMBERCACHE_METHOD:
   func = item->mcs_method.m_func;
   if (item->mcs_method.m_flag & TYPE_METHOD_FKWDS) {
    MEMBERCACHE_ENDREAD(cache);
    return DeeKwObjMethod_CallFunc((dkwobjmethod_t)func,self,argc,argv,kw);
   }
   MEMBERCACHE_ENDREAD(cache);
   if (kw) {
    if (DeeKwds_Check(kw)) {
     if (DeeKwds_SIZE(kw) != 0)
         goto err_no_keywords;
    } else {
     size_t temp = DeeObject_Size(kw);
     if unlikely(temp == (size_t)-1) return NULL;
     if (temp != 0) goto err_no_keywords;
    }
   }
   return DeeObjMethod_CallFunc(func,self,argc,argv);
  }
  {
   DREF DeeObject *(DCALL *getter)(DeeObject *__restrict self);
  case MEMBERCACHE_GETSET:
   getter = item->mcs_getset.gs_get;
   MEMBERCACHE_ENDREAD(cache);
   if likely(getter) { result = (*getter)(self); goto invoke_result; }
   err_cant_access_attribute(find_type_for_getset(self,name),
                             name,ATTR_ACCESS_GET);
   goto err;
  }
  {
   struct buffer { uint8_t dat[COMPILER_OFFSETOF(struct type_member,m_doc)]; } buf;
  case MEMBERCACHE_MEMBER:
   buf = *(struct buffer *)&item->mcs_member;
   MEMBERCACHE_ENDREAD(cache);
   result = type_member_get((struct type_member *)&buf,self);
   goto invoke_result;
  }
  {
   struct class_attribute *attr;
   struct class_desc *desc;
  case MEMBERCACHE_ATTRIB:
   attr = item->mcs_attrib.a_attr;
   desc = item->mcs_attrib.a_desc;
   MEMBERCACHE_ENDREAD(cache);
   return DeeInstance_CallAttributeKw(desc,
                                      DeeInstance_DESC(desc,self),
                                      self,attr,argc,argv,kw);
  }
  default: __builtin_unreachable();
  }
 }
done:
 MEMBERCACHE_ENDREAD(cache);
 return ITER_DONE;
invoke_result:
 if likely(result) {
  DREF DeeObject *callback_result;
  callback_result = DeeObject_CallKw(result,argc,argv,kw);
  Dee_Decref(result);
  result = callback_result;
 }
 return result;
err_no_keywords:
 err_keywords_func_not_accepted(name,kw);
err:
 return NULL;
}

INTERN DREF DeeObject *DCALL
membercache_vcallattrf(struct membercache *__restrict cache,
                       DeeObject *__restrict self,
                       char const *__restrict name, dhash_t hash,
                       char const *__restrict format, va_list args) {
 dhash_t i,perturb;
 DREF DeeObject *result;
 MEMBERCACHE_READ(cache);
 if unlikely(!cache->mc_table) goto done;
 perturb = i = MEMBERCACHE_HASHST(cache,hash);
 for (;; i = MEMBERCACHE_HASHNX(i,perturb),MEMBERCACHE_HASHPT(perturb)) {
  struct membercache_slot *item = MEMBERCACHE_HASHIT(cache,i);
  if (item->mcs_type == MEMBERCACHE_UNUSED) break;
  if (item->mcs_hash != hash) continue;
  if (strcmp(item->mcs_name,name)) continue;
  switch (item->mcs_type) {
  {
   dobjmethod_t func; uintptr_t flags;
   DREF DeeObject *args_tuple,*result;
  case MEMBERCACHE_METHOD:
   func  = item->mcs_method.m_func;
   flags = item->mcs_method.m_flag;
   MEMBERCACHE_ENDREAD(cache);
   args_tuple = DeeTuple_VNewf(format,args);
   if unlikely(!args_tuple) return NULL;
   if (flags & TYPE_METHOD_FKWDS) {
    result = DeeKwObjMethod_CallFunc((dkwobjmethod_t)func,
                                      self,
                                      DeeTuple_SIZE(args_tuple),
                                      DeeTuple_ELEM(args_tuple),
                                      NULL);
   } else {
    result = DeeObjMethod_CallFunc(func,
                                   self,
                                   DeeTuple_SIZE(args_tuple),
                                   DeeTuple_ELEM(args_tuple));
   }
   Dee_Decref(args_tuple);
   return result;
  }
  {
   DREF DeeObject *(DCALL *getter)(DeeObject *__restrict self);
  case MEMBERCACHE_GETSET:
   getter = item->mcs_getset.gs_get;
   MEMBERCACHE_ENDREAD(cache);
   if likely(getter) { result = (*getter)(self); goto invoke_result; }
   err_cant_access_attribute(find_type_for_getset(self,name),
                             name,ATTR_ACCESS_GET);
   goto err;
  }
  {
   struct buffer { uint8_t dat[COMPILER_OFFSETOF(struct type_member,m_doc)]; } buf;
  case MEMBERCACHE_MEMBER:
   buf = *(struct buffer *)&item->mcs_member;
   MEMBERCACHE_ENDREAD(cache);
   result = type_member_get((struct type_member *)&buf,self);
   goto invoke_result;
  }
  {
   struct class_attribute *attr;
   struct class_desc *desc;
  case MEMBERCACHE_ATTRIB:
   attr = item->mcs_attrib.a_attr;
   desc = item->mcs_attrib.a_desc;
   MEMBERCACHE_ENDREAD(cache);
   return DeeInstance_VCallAttributef(desc,
                                      DeeInstance_DESC(desc,self),
                                      self,attr,format,args);
  }
  default: __builtin_unreachable();
  }
 }
done:
 MEMBERCACHE_ENDREAD(cache);
 return ITER_DONE;
invoke_result:
 if likely(result) {
  DREF DeeObject *callback_result;
  callback_result = DeeObject_VCallf(result,format,args);
  Dee_Decref(result);
  result = callback_result;
 }
 return result;
err:
 return NULL;
}


INTERN bool DCALL
membercache_hasattr(struct membercache *__restrict cache,
                    char const *__restrict name, dhash_t hash) {
 dhash_t i,perturb;
 MEMBERCACHE_READ(cache);
 if unlikely(!cache->mc_table) goto done;
 perturb = i = MEMBERCACHE_HASHST(cache,hash);
 for (;; i = MEMBERCACHE_HASHNX(i,perturb),MEMBERCACHE_HASHPT(perturb)) {
  struct membercache_slot *item = MEMBERCACHE_HASHIT(cache,i);
  if (item->mcs_type == MEMBERCACHE_UNUSED) break;
  if (item->mcs_hash != hash) continue;
  if (strcmp(item->mcs_name,name)) continue;
  MEMBERCACHE_ENDREAD(cache);
  return true;
 }
done:
 MEMBERCACHE_ENDREAD(cache);
 return false;
}


/* Lookup an attribute from cache.
 * @return:  0: Successfully invoked the delete-operator on the attribute.
 * @return: -1: An error occurred.
 * @return:  1: The attribute could not be found in the cache. */
INTERN int DCALL
membercache_delattr(struct membercache *__restrict cache,
                    DeeObject *__restrict self,
                    char const *__restrict name, dhash_t hash) {
 dhash_t i,perturb;
 MEMBERCACHE_READ(cache);
 if unlikely(!cache->mc_table) goto done;
 perturb = i = MEMBERCACHE_HASHST(cache,hash);
 for (;; i = MEMBERCACHE_HASHNX(i,perturb),MEMBERCACHE_HASHPT(perturb)) {
  struct membercache_slot *item = MEMBERCACHE_HASHIT(cache,i);
  if (item->mcs_type == MEMBERCACHE_UNUSED) break;
  if (item->mcs_hash != hash) continue;
  if (strcmp(item->mcs_name,name)) continue;
  switch (item->mcs_type) {
  {
   int (DCALL *del)(DeeObject *__restrict self);
  case MEMBERCACHE_GETSET:
   del = item->mcs_getset.gs_del;
   MEMBERCACHE_ENDREAD(cache);
   if likely(del) return (*del)(self);
   return err_cant_access_attribute(find_type_for_getset(self,name),
                                    name,ATTR_ACCESS_DEL);
  }
  {
   struct buffer { uint8_t dat[COMPILER_OFFSETOF(struct type_member,m_doc)]; } buf;
  case MEMBERCACHE_MEMBER:
   buf = *(struct buffer *)&item->mcs_member;
   MEMBERCACHE_ENDREAD(cache);
   return type_member_del((struct type_member *)&buf,self);
  }
  case MEMBERCACHE_METHOD:
   MEMBERCACHE_ENDREAD(cache);
   return err_cant_access_attribute(find_type_for_method(self,name),
                                    name,ATTR_ACCESS_DEL);
  {
   struct class_attribute *attr;
   struct class_desc *desc;
  case MEMBERCACHE_ATTRIB:
   attr = item->mcs_attrib.a_attr;
   desc = item->mcs_attrib.a_desc;
   MEMBERCACHE_ENDREAD(cache);
   return DeeInstance_DelAttribute(desc,
                                   DeeInstance_DESC(desc,self),
                                   self,attr);
  }
  default: __builtin_unreachable();
  }
 }
done:
 MEMBERCACHE_ENDREAD(cache);
 return 1;
}

INTERN int DCALL
membercache_setattr(struct membercache *__restrict cache,
                    DeeObject *__restrict self,
                    char const *__restrict name, dhash_t hash,
                    DeeObject *__restrict value) {
 dhash_t i,perturb;
 MEMBERCACHE_READ(cache);
 if unlikely(!cache->mc_table) goto done;
 perturb = i = MEMBERCACHE_HASHST(cache,hash);
 for (;; i = MEMBERCACHE_HASHNX(i,perturb),MEMBERCACHE_HASHPT(perturb)) {
  struct membercache_slot *item = MEMBERCACHE_HASHIT(cache,i);
  if (item->mcs_type == MEMBERCACHE_UNUSED) break;
  if (item->mcs_hash != hash) continue;
  if (strcmp(item->mcs_name,name)) continue;
  switch (item->mcs_type) {
  {
   int (DCALL *setter)(DeeObject *__restrict self, DeeObject *__restrict value);
  case MEMBERCACHE_GETSET:
   setter = item->mcs_getset.gs_set;
   MEMBERCACHE_ENDREAD(cache);
   if likely(setter) return (*setter)(self,value);
   return err_cant_access_attribute(find_type_for_getset(self,name),
                                    name,ATTR_ACCESS_SET);
  }
  {
   struct buffer { uint8_t dat[COMPILER_OFFSETOF(struct type_member,m_doc)]; } buf;
  case MEMBERCACHE_MEMBER:
   buf = *(struct buffer *)&item->mcs_member;
   MEMBERCACHE_ENDREAD(cache);
   return type_member_set((struct type_member *)&buf,self,value);
  }
  case MEMBERCACHE_METHOD:
   MEMBERCACHE_ENDREAD(cache);
   return err_cant_access_attribute(find_type_for_method(self,name),
                                    name,ATTR_ACCESS_SET);
  {
   struct class_attribute *attr;
   struct class_desc *desc;
  case MEMBERCACHE_ATTRIB:
   attr = item->mcs_attrib.a_attr;
   desc = item->mcs_attrib.a_desc;
   MEMBERCACHE_ENDREAD(cache);
   return DeeInstance_SetAttribute(desc,
                                   DeeInstance_DESC(desc,self),
                                   self,attr,value);
  }
  default: __builtin_unreachable();
  }
 }
done:
 MEMBERCACHE_ENDREAD(cache);
 return 1;
}

INTERN int DCALL
membercache_setbasicattr(struct membercache *__restrict cache,
                         DeeObject *__restrict self,
                         char const *__restrict name, dhash_t hash,
                         DeeObject *__restrict value) {
 dhash_t i,perturb;
 MEMBERCACHE_READ(cache);
 if unlikely(!cache->mc_table) goto done;
 perturb = i = MEMBERCACHE_HASHST(cache,hash);
 for (;; i = MEMBERCACHE_HASHNX(i,perturb),MEMBERCACHE_HASHPT(perturb)) {
  struct membercache_slot *item = MEMBERCACHE_HASHIT(cache,i);
  if (item->mcs_type == MEMBERCACHE_UNUSED) break;
  if (item->mcs_hash != hash) continue;
  if (strcmp(item->mcs_name,name)) continue;
  switch (item->mcs_type) {
  {
   struct buffer { uint8_t dat[COMPILER_OFFSETOF(struct type_member,m_doc)]; } buf;
  case MEMBERCACHE_MEMBER:
   buf = *(struct buffer *)&item->mcs_member;
   MEMBERCACHE_ENDREAD(cache);
   return type_member_set((struct type_member *)&buf,self,value);
  }
  {
   struct class_attribute *attr;
   struct class_desc *desc;
   int result;
  case MEMBERCACHE_ATTRIB:
   attr = item->mcs_attrib.a_attr;
   desc = item->mcs_attrib.a_desc;
   MEMBERCACHE_ENDREAD(cache);
   result = DeeInstance_SetBasicAttribute(desc,
                                          DeeInstance_DESC(desc,self),
                                          self,attr,value);
   if (result == 2) result = 1;
   return result;
  }
  default: goto done;
  }
 }
done:
 MEMBERCACHE_ENDREAD(cache);
 return 1;
}


INTERN DREF DeeObject *DCALL
membercache_getinstanceattr(DeeTypeObject *__restrict self,
                            char const *__restrict name, dhash_t hash) {
 dhash_t i,perturb;
 MEMBERCACHE_READ(&self->tp_cache);
 ASSERT(DeeType_Check(self));
 if unlikely(!self->tp_cache.mc_table) goto done;
 perturb = i = MEMBERCACHE_HASHST(&self->tp_cache,hash);
 for (;; i = MEMBERCACHE_HASHNX(i,perturb),MEMBERCACHE_HASHPT(perturb)) {
  struct membercache_slot *item = MEMBERCACHE_HASHIT(&self->tp_cache,i);
  if (item->mcs_type == MEMBERCACHE_UNUSED) break;
  if (item->mcs_hash != hash) continue;
  if (strcmp(item->mcs_name,name)) continue;
  switch (item->mcs_type) {
  {
   dobjmethod_t func;
  case MEMBERCACHE_METHOD:
   func = item->mcs_method.m_func;
   if (item->mcs_method.m_flag & TYPE_METHOD_FKWDS) {
    MEMBERCACHE_ENDREAD(&self->tp_cache);
    return DeeKwClsMethod_New((DeeTypeObject *)self,(dkwobjmethod_t)func);
   }
   MEMBERCACHE_ENDREAD(&self->tp_cache);
   return DeeClsMethod_New((DeeTypeObject *)self,func);
  }
  {
   struct buffer { uint8_t dat[COMPILER_OFFSETOF(struct type_getset,gs_doc)-
                               COMPILER_OFFSETOF(struct type_getset,gs_get)]; } buf;
  case MEMBERCACHE_GETSET:
   buf = *(struct buffer *)&item->mcs_getset.gs_get;
   MEMBERCACHE_ENDREAD(&self->tp_cache);
   return DeeClsProperty_New((DeeTypeObject *)self,
                             COMPILER_CONTAINER_OF(&buf,struct type_getset,gs_get)->gs_get,
                             COMPILER_CONTAINER_OF(&buf,struct type_getset,gs_get)->gs_del,
                             COMPILER_CONTAINER_OF(&buf,struct type_getset,gs_get)->gs_set);
  }
  {
   struct type_member buf;
  case MEMBERCACHE_MEMBER:
   buf = item->mcs_member;
   MEMBERCACHE_ENDREAD(&self->tp_cache);
   return DeeClsMember_New((DeeTypeObject *)self,&buf);
  }
  {
   struct class_attribute *attr;
   DeeTypeObject *type;
  case MEMBERCACHE_ATTRIB:
   attr = item->mcs_attrib.a_attr;
   type = item->mcs_attrib.a_type;
   MEMBERCACHE_ENDREAD(&self->tp_cache);
   return DeeClass_GetInstanceAttribute(type,attr);
  }
  default: __builtin_unreachable();
  }
 }
done:
 MEMBERCACHE_ENDREAD(&self->tp_cache);
 return ITER_DONE;
}
INTERN DREF DeeObject *DCALL
membercache_callinstanceattr(DeeTypeObject *__restrict self,
                             char const *__restrict name, dhash_t hash,
                             size_t argc, DeeObject **__restrict argv) {
 dhash_t i,perturb;
 MEMBERCACHE_READ(&self->tp_cache);
 ASSERT(DeeType_Check(self));
 if unlikely(!self->tp_cache.mc_table) goto done;
 perturb = i = MEMBERCACHE_HASHST(&self->tp_cache,hash);
 for (;; i = MEMBERCACHE_HASHNX(i,perturb),MEMBERCACHE_HASHPT(perturb)) {
  struct membercache_slot *item = MEMBERCACHE_HASHIT(&self->tp_cache,i);
  if (item->mcs_type == MEMBERCACHE_UNUSED) break;
  if (item->mcs_hash != hash) continue;
  if (strcmp(item->mcs_name,name)) continue;
  switch (item->mcs_type) {
  {
   dobjmethod_t func;
   uintptr_t flags;
  case MEMBERCACHE_METHOD:
   func  = item->mcs_method.m_func;
   flags = item->mcs_method.m_flag;
   MEMBERCACHE_ENDREAD(&self->tp_cache);
   if unlikely(!argc) {
    DeeError_Throwf(&DeeError_TypeError,
                    "classmethod `%s' must be called with at least 1 argument",
                    name);
    goto err;
   }
   /* Allow non-instance objects for generic types. */
   if (!(((DeeTypeObject *)self)->tp_flags&TP_FABSTRACT) &&
           DeeObject_AssertType(argv[0],(DeeTypeObject *)self))
           goto err;
   /* Use the first argument as the this-argument. */
   if (flags & TYPE_METHOD_FKWDS)
       return (*(dkwobjmethod_t)func)(argv[0],argc-1,argv+1,NULL);
   return (*func)(argv[0],argc-1,argv+1);
  }
  {
   dgetmethod_t getter;
  case MEMBERCACHE_GETSET:
   getter = item->mcs_getset.gs_get;
   MEMBERCACHE_ENDREAD(&self->tp_cache);
   if (!getter) {
    err_cant_access_attribute(&DeeClsProperty_Type,"get",ATTR_ACCESS_GET);
    goto err;
   }
   if unlikely(argc != 1) {
    DeeError_Throwf(&DeeError_TypeError,
                    "classproperty `%s' must be called with exactly 1 argument",
                    name);
    goto err;
   }
   /* Allow non-instance objects for generic types. */
   if (!(((DeeTypeObject *)self)->tp_flags&TP_FABSTRACT) &&
           DeeObject_AssertType(argv[0],(DeeTypeObject *)self))
           goto err;
   /* Use the first argument as the this-argument. */
   return (*getter)(argv[0]);
  }
  {
   struct type_member member;
  case MEMBERCACHE_MEMBER:
   member = item->mcs_member;
   MEMBERCACHE_ENDREAD(&self->tp_cache);
   if unlikely(argc != 1) {
    DeeError_Throwf(&DeeError_TypeError,
                    "classmember `%s' must be called with exactly 1 argument",
                    name);
    goto err;
   }
   /* Allow non-instance objects for generic types. */
   if (!(((DeeTypeObject *)self)->tp_flags&TP_FABSTRACT) &&
           DeeObject_AssertType(argv[0],(DeeTypeObject *)self))
           goto err;
   /* Use the first argument as the this-argument. */
   return type_member_get(&member,argv[0]);
  }
  {
   struct class_attribute *attr;
   DeeTypeObject *type;
  case MEMBERCACHE_ATTRIB:
   attr = item->mcs_attrib.a_attr;
   type = item->mcs_attrib.a_type;
   MEMBERCACHE_ENDREAD(&self->tp_cache);
   return DeeClass_CallInstanceAttribute(type,attr,argc,argv);
  }
  default: __builtin_unreachable();
  }
 }
done:
 MEMBERCACHE_ENDREAD(&self->tp_cache);
 return ITER_DONE;
err:
 return NULL;
}

INTERN DREF DeeObject *DCALL
membercache_callinstanceattr_kw(DeeTypeObject *__restrict self,
                                char const *__restrict name, dhash_t hash,
                                size_t argc, DeeObject **__restrict argv,
                                DeeObject *kw) {
 dhash_t i,perturb;
 MEMBERCACHE_READ(&self->tp_cache);
 ASSERT(DeeType_Check(self));
 if unlikely(!self->tp_cache.mc_table) goto done;
 perturb = i = MEMBERCACHE_HASHST(&self->tp_cache,hash);
 for (;; i = MEMBERCACHE_HASHNX(i,perturb),MEMBERCACHE_HASHPT(perturb)) {
  struct membercache_slot *item = MEMBERCACHE_HASHIT(&self->tp_cache,i);
  if (item->mcs_type == MEMBERCACHE_UNUSED) break;
  if (item->mcs_hash != hash) continue;
  if (strcmp(item->mcs_name,name)) continue;
  switch (item->mcs_type) {
  {
   dobjmethod_t func;
   uintptr_t flags;
  case MEMBERCACHE_METHOD:
   func  = item->mcs_method.m_func;
   flags = item->mcs_method.m_flag;
   MEMBERCACHE_ENDREAD(&self->tp_cache);
   if unlikely(!argc) {
    DeeError_Throwf(&DeeError_TypeError,
                    "classmethod `%s' must be called with at least 1 argument",
                    name);
    goto err;
   }
   /* Allow non-instance objects for generic types. */
   if (!(((DeeTypeObject *)self)->tp_flags&TP_FABSTRACT) &&
           DeeObject_AssertType(argv[0],(DeeTypeObject *)self))
           goto err;
   /* Use the first argument as the this-argument. */
   if (flags & TYPE_METHOD_FKWDS)
       return (*(dkwobjmethod_t)func)(argv[0],argc-1,argv+1,kw);
   if (kw) {
    if (DeeKwds_Check(kw)) {
     if (DeeKwds_SIZE(kw) != 0)
         goto err_no_keywords;
    } else {
     size_t temp = DeeObject_Size(kw);
     if unlikely(temp == (size_t)-1) return NULL;
     if (temp != 0) goto err_no_keywords;
    }
   }
   return (*func)(argv[0],argc-1,argv+1);
  }
  case MEMBERCACHE_GETSET: {
   dgetmethod_t getter;
   DeeObject *this_arg;
   PRIVATE struct keyword kwlist[] = { K(thisarg), KEND };
   getter = item->mcs_getset.gs_get;
   MEMBERCACHE_ENDREAD(&self->tp_cache);
   if (!getter) {
    err_cant_access_attribute(&DeeClsProperty_Type,"get",ATTR_ACCESS_GET);
    goto err;
   }
   if (DeeArg_UnpackKw(argc,argv,kw,kwlist,"o:get",&this_arg) ||
         /* Allow non-instance objects for generic types. */
      (!(self->tp_flags&TP_FABSTRACT) &&
         DeeObject_AssertType(this_arg,self)))
         return NULL;
   /* Use the first argument as the this-argument. */
   return (*getter)(this_arg);
  }
  {
   struct type_member member;
  case MEMBERCACHE_MEMBER:
   member = item->mcs_member;
   MEMBERCACHE_ENDREAD(&self->tp_cache);
   return type_obmemb_call_kw(self,&member,argc,argv,kw);
  }
  {
   struct class_attribute *attr;
   DeeTypeObject *type;
  case MEMBERCACHE_ATTRIB:
   attr = item->mcs_attrib.a_attr;
   type = item->mcs_attrib.a_type;
   MEMBERCACHE_ENDREAD(&self->tp_cache);
   return DeeClass_CallInstanceAttributeKw(type,attr,argc,argv,kw);
  }
  default: __builtin_unreachable();
  }
 }
done:
 MEMBERCACHE_ENDREAD(&self->tp_cache);
 return ITER_DONE;
err_no_keywords:
 err_keywords_func_not_accepted(name,kw);
err:
 return NULL;
}


INTERN int DCALL
membercache_delinstanceattr(DeeTypeObject *__restrict self,
                            char const *__restrict name, dhash_t hash) {
 dhash_t i,perturb;
 ASSERT(DeeType_Check(self));
 MEMBERCACHE_READ(&self->tp_cache);
 if unlikely(!self->tp_cache.mc_table) goto done;
 perturb = i = MEMBERCACHE_HASHST(&self->tp_cache,hash);
 for (;; i = MEMBERCACHE_HASHNX(i,perturb),MEMBERCACHE_HASHPT(perturb)) {
  struct membercache_slot *item = MEMBERCACHE_HASHIT(&self->tp_cache,i);
  if (item->mcs_type == MEMBERCACHE_UNUSED) break;
  if (item->mcs_hash != hash) continue;
  if (strcmp(item->mcs_name,name)) continue;
  switch (item->mcs_type) {
  case MEMBERCACHE_METHOD:
   MEMBERCACHE_ENDREAD(&self->tp_cache);
   return err_cant_access_attribute(find_type_for_class_method(self,name),
                                    name,ATTR_ACCESS_DEL);
  case MEMBERCACHE_GETSET:
   MEMBERCACHE_ENDREAD(&self->tp_cache);
   return err_cant_access_attribute(find_type_for_class_getset(self,name),
                                    name,ATTR_ACCESS_DEL);
  case MEMBERCACHE_MEMBER:
   MEMBERCACHE_ENDREAD(&self->tp_cache);
   return err_cant_access_attribute(find_type_for_instance_member(self,name),
                                    name,ATTR_ACCESS_DEL);
  {
   struct class_attribute *attr;
   DeeTypeObject *type;
  case MEMBERCACHE_ATTRIB:
   attr = item->mcs_attrib.a_attr;
   type = item->mcs_attrib.a_type;
   MEMBERCACHE_ENDREAD(&self->tp_cache);
   return DeeClass_DelInstanceAttribute(type,attr);
  }
  default: __builtin_unreachable();
  }
 }
done:
 MEMBERCACHE_ENDREAD(&self->tp_cache);
 return 1;
}

INTERN int DCALL
membercache_setinstanceattr(DeeTypeObject *__restrict self,
                            char const *__restrict name, dhash_t hash,
                            DeeObject *__restrict value) {
 dhash_t i,perturb;
 ASSERT(DeeType_Check(self));
 (void)value; /* Unused... */
 MEMBERCACHE_READ(&self->tp_cache);
 if unlikely(!self->tp_cache.mc_table) goto done;
 perturb = i = MEMBERCACHE_HASHST(&self->tp_cache,hash);
 for (;; i = MEMBERCACHE_HASHNX(i,perturb),MEMBERCACHE_HASHPT(perturb)) {
  struct membercache_slot *item = MEMBERCACHE_HASHIT(&self->tp_cache,i);
  if (item->mcs_type == MEMBERCACHE_UNUSED) break;
  if (item->mcs_hash != hash) continue;
  if (strcmp(item->mcs_name,name)) continue;
  switch (item->mcs_type) {
  case MEMBERCACHE_METHOD:
   MEMBERCACHE_ENDREAD(&self->tp_cache);
   return err_cant_access_attribute(find_type_for_class_method(self,name),
                                    name,ATTR_ACCESS_SET);
  case MEMBERCACHE_GETSET:
   MEMBERCACHE_ENDREAD(&self->tp_cache);
   return err_cant_access_attribute(find_type_for_class_getset(self,name),
                                    name,ATTR_ACCESS_SET);
  case MEMBERCACHE_MEMBER:
   MEMBERCACHE_ENDREAD(&self->tp_cache);
   return err_cant_access_attribute(find_type_for_class_getset(self,name),
                                    name,ATTR_ACCESS_SET);
  {
   struct class_attribute *attr;
   DeeTypeObject *type;
  case MEMBERCACHE_ATTRIB:
   attr = item->mcs_attrib.a_attr;
   type = item->mcs_attrib.a_type;
   MEMBERCACHE_ENDREAD(&self->tp_cache);
   return DeeClass_SetInstanceAttribute(type,attr,value);
  }
  default: __builtin_unreachable();
  }
 }
done:
 MEMBERCACHE_ENDREAD(&self->tp_cache);
 return 1;
}

INTERN DREF DeeObject *DCALL
type_method_getattr(struct membercache *__restrict cache,
                    struct type_method *__restrict chain, DeeObject *__restrict self,
                    char const *__restrict attr_name, dhash_t hash) {
 for (; chain->m_name; ++chain) {
  if (strcmp(chain->m_name,attr_name)) continue;
  membercache_add(cache,attr_name,hash,MEMBERCACHE_METHOD,chain);
  return type_method_get(chain,self);
 }
 return ITER_DONE;
}
INTERN DREF DeeObject *DCALL
type_method_docattr(struct membercache *__restrict cache,
                    struct type_method *__restrict chain, char const *base,
                    char const *__restrict attr_name, dhash_t hash) {
 for (; chain->m_name; ++chain) {
  if (strcmp(chain->m_name,attr_name)) continue;
  membercache_add(cache,attr_name,hash,MEMBERCACHE_METHOD,chain);
  if unlikely(!chain->m_doc) {
   err_nodoc_attribute(base,attr_name);
   return NULL;
  }
  return type_method_doc(chain);
 }
 return ITER_DONE;
}
INTERN DREF DeeObject *DCALL
type_method_callattr(struct membercache *__restrict cache,
                     struct type_method *__restrict chain, DeeObject *__restrict self,
                     char const *__restrict attr_name, dhash_t hash,
                     size_t argc, DeeObject **__restrict argv) {
 for (; chain->m_name; ++chain) {
  if (strcmp(chain->m_name,attr_name)) continue;
  membercache_add(cache,attr_name,hash,MEMBERCACHE_METHOD,chain);
  return type_method_call(chain,self,argc,argv);
 }
 return ITER_DONE;
}
INTERN DREF DeeObject *DCALL
type_method_callattr_kw(struct membercache *__restrict cache,
                        struct type_method *__restrict chain, DeeObject *__restrict self,
                        char const *__restrict attr_name, dhash_t hash,
                        size_t argc, DeeObject **__restrict argv, DeeObject *kw) {
 for (; chain->m_name; ++chain) {
  if (strcmp(chain->m_name,attr_name)) continue;
  membercache_add(cache,attr_name,hash,MEMBERCACHE_METHOD,chain);
  return type_method_call_kw(chain,self,argc,argv,kw);
 }
 return ITER_DONE;
}
INTERN DREF DeeObject *DCALL
type_method_vcallattrf(struct membercache *__restrict cache,
                       struct type_method *__restrict chain, DeeObject *__restrict self,
                       char const *__restrict attr_name, dhash_t hash,
                       char const *__restrict format, va_list args) {
 for (; chain->m_name; ++chain) {
  DREF DeeObject *args_tuple,*result;
  if (strcmp(chain->m_name,attr_name)) continue;
  membercache_add(cache,attr_name,hash,MEMBERCACHE_METHOD,chain);
  args_tuple = DeeTuple_VNewf(format,args);
  if unlikely(!args_tuple) return NULL;
  result = type_method_call(chain,
                            self,
                            DeeTuple_SIZE(args_tuple),
                            DeeTuple_ELEM(args_tuple));
  Dee_Decref(args_tuple);
  return result;
 }
 return ITER_DONE;
}
INTERN DREF DeeObject *DCALL
type_obmeth_getattr(DeeTypeObject *__restrict self,
                    char const *__restrict attr_name, dhash_t hash) {
 struct type_method *chain;
 ASSERT(self->tp_methods);
 for (chain = self->tp_methods; chain->m_name; ++chain) {
  if (strcmp(chain->m_name,attr_name)) continue;
  membercache_add(&self->tp_cache,attr_name,hash,MEMBERCACHE_METHOD,chain);
  return type_obmeth_get(self,chain);
 }
 return ITER_DONE;
}
INTERN DREF DeeObject *DCALL
type_obmeth_docattr(DeeTypeObject *__restrict self,
                    char const *__restrict attr_name, dhash_t hash) {
 struct type_method *chain;
 ASSERT(self->tp_methods);
 for (chain = self->tp_methods; chain->m_name; ++chain) {
  if (strcmp(chain->m_name,attr_name)) continue;
  membercache_add(&self->tp_cache,attr_name,hash,MEMBERCACHE_METHOD,chain);
  if unlikely(!chain->m_doc) {
   err_nodoc_attribute(self->tp_name,attr_name);
   return NULL;
  }
  return type_obmeth_doc(chain);
 }
 return ITER_DONE;
}
INTERN DREF DeeObject *DCALL
type_obmeth_callattr(DeeTypeObject *__restrict self,
                     char const *__restrict attr_name, dhash_t hash,
                     size_t argc, DeeObject **__restrict argv) {
 struct type_method *chain;
 ASSERT(self->tp_methods);
 for (chain = self->tp_methods; chain->m_name; ++chain) {
  if (strcmp(chain->m_name,attr_name)) continue;
  membercache_add(&self->tp_cache,attr_name,hash,MEMBERCACHE_METHOD,chain);
  return type_obmeth_call(self,chain,argc,argv);
 }
 return ITER_DONE;
}
INTERN DREF DeeObject *DCALL
type_obmeth_callattr_kw(DeeTypeObject *__restrict self,
                        char const *__restrict attr_name, dhash_t hash,
                        size_t argc, DeeObject **__restrict argv,
                        DeeObject *kw) {
 struct type_method *chain;
 ASSERT(self->tp_methods);
 for (chain = self->tp_methods; chain->m_name; ++chain) {
  if (strcmp(chain->m_name,attr_name)) continue;
  membercache_add(&self->tp_cache,attr_name,hash,MEMBERCACHE_METHOD,chain);
  return type_obmeth_call_kw(self,chain,argc,argv,kw);
 }
 return ITER_DONE;
}
INTERN DREF DeeObject *DCALL
type_getset_getattr(struct membercache *__restrict cache,
                    struct type_getset *__restrict chain, DeeObject *__restrict self,
                    char const *__restrict attr_name, dhash_t hash) {
 for (; chain->gs_name; ++chain) {
  if (strcmp(chain->gs_name,attr_name)) continue;
  membercache_add(cache,attr_name,hash,MEMBERCACHE_GETSET,chain);
  return type_getset_get(chain,self);
 }
 return ITER_DONE;
}
INTERN int DCALL
type_getset_boundattr(struct membercache *__restrict cache,
                      struct type_getset *__restrict chain, DeeObject *__restrict self,
                      char const *__restrict attr_name, dhash_t hash) {
 for (; chain->gs_name; ++chain) {
  DREF DeeObject *found_object;
  if (strcmp(chain->gs_name,attr_name)) continue;
  membercache_add(cache,attr_name,hash,MEMBERCACHE_GETSET,chain);
  if unlikely(!chain->gs_get) return 0; /* Unbound */
  found_object = (*chain->gs_get)(self);
  if likely(found_object) {
   Dee_Decref(found_object);
   return 1;
  }
  if (CATCH_ATTRIBUTE_ERROR())
      return -3;
  if (DeeError_Catch(&DeeError_UnboundAttribute))
      return 0;
  return -1;
 }
 return -2;
}
INTERN DREF DeeObject *DCALL
type_getset_docattr(struct membercache *__restrict cache,
                    struct type_getset *__restrict chain, char const *base,
                    char const *__restrict attr_name, dhash_t hash) {
 for (; chain->gs_name; ++chain) {
  if (strcmp(chain->gs_name,attr_name)) continue;
  membercache_add(cache,attr_name,hash,MEMBERCACHE_GETSET,chain);
  if unlikely(!chain->gs_doc) {
   err_nodoc_attribute(base,attr_name);
   return NULL;
  }
  return type_getset_doc(chain);
 }
 return ITER_DONE;
}
INTERN DREF DeeObject *DCALL
type_obprop_getattr(DeeTypeObject *__restrict self,
                    char const *__restrict attr_name, dhash_t hash) {
 struct type_getset *chain;
 ASSERT(self->tp_getsets);
 for (chain = self->tp_getsets; chain->gs_name; ++chain) {
  if (strcmp(chain->gs_name,attr_name)) continue;
  membercache_add(&self->tp_cache,attr_name,hash,MEMBERCACHE_GETSET,chain);
  return type_obprop_get(self,chain);
 }
 return ITER_DONE;
}
INTERN DREF DeeObject *DCALL
type_obprop_callattr(DeeTypeObject *__restrict self,
                     char const *__restrict attr_name, dhash_t hash,
                     size_t argc, DeeObject **__restrict argv) {
 struct type_getset *chain;
 ASSERT(self->tp_getsets);
 for (chain = self->tp_getsets; chain->gs_name; ++chain) {
  if (strcmp(chain->gs_name,attr_name)) continue;
  membercache_add(&self->tp_cache,attr_name,hash,MEMBERCACHE_GETSET,chain);
  return type_obprop_call(self,chain,argc,argv);
 }
 return ITER_DONE;
}
INTERN DREF DeeObject *DCALL
type_obprop_callattr_kw(DeeTypeObject *__restrict self,
                        char const *__restrict attr_name, dhash_t hash,
                        size_t argc, DeeObject **__restrict argv, DeeObject *kw) {
 struct type_getset *chain;
 ASSERT(self->tp_getsets);
 for (chain = self->tp_getsets; chain->gs_name; ++chain) {
  if (strcmp(chain->gs_name,attr_name)) continue;
  membercache_add(&self->tp_cache,attr_name,hash,MEMBERCACHE_GETSET,chain);
  return type_obprop_call_kw(self,chain,argc,argv,kw);
 }
 return ITER_DONE;
}
INTERN DREF DeeObject *DCALL
type_obprop_docattr(DeeTypeObject *__restrict self,
                    char const *__restrict attr_name, dhash_t hash) {
 struct type_getset *chain;
 ASSERT(self->tp_getsets);
 for (chain = self->tp_getsets; chain->gs_name; ++chain) {
  if (strcmp(chain->gs_name,attr_name)) continue;
  membercache_add(&self->tp_cache,attr_name,hash,MEMBERCACHE_GETSET,chain);
  if unlikely(!chain->gs_doc) {
   err_nodoc_attribute(self->tp_name,attr_name);
   return NULL;
  }
  return type_obprop_doc(chain);
 }
 return ITER_DONE;
}
INTERN DREF DeeObject *DCALL
type_obmemb_getattr(DeeTypeObject *__restrict self,
                    char const *__restrict attr_name, dhash_t hash) {
 struct type_member *chain;
 ASSERT(self->tp_members);
 for (chain = self->tp_members; chain->m_name; ++chain) {
  if (strcmp(chain->m_name,attr_name)) continue;
  membercache_add(&self->tp_cache,attr_name,hash,MEMBERCACHE_MEMBER,chain);
  return type_obmemb_get(self,chain);
 }
 return ITER_DONE;
}
INTERN DREF DeeObject *DCALL
type_obmemb_callattr(DeeTypeObject *__restrict self,
                     char const *__restrict attr_name, dhash_t hash,
                     size_t argc, DeeObject **__restrict argv) {
 struct type_member *chain;
 ASSERT(self->tp_members);
 for (chain = self->tp_members; chain->m_name; ++chain) {
  if (strcmp(chain->m_name,attr_name)) continue;
  membercache_add(&self->tp_cache,attr_name,hash,MEMBERCACHE_MEMBER,chain);
  return type_obmemb_call(self,chain,argc,argv);
 }
 return ITER_DONE;
}
INTERN DREF DeeObject *DCALL
type_obmemb_callattr_kw(DeeTypeObject *__restrict self,
                        char const *__restrict attr_name, dhash_t hash,
                        size_t argc, DeeObject **__restrict argv, DeeObject *kw) {
 struct type_member *chain;
 ASSERT(self->tp_members);
 for (chain = self->tp_members; chain->m_name; ++chain) {
  if (strcmp(chain->m_name,attr_name)) continue;
  membercache_add(&self->tp_cache,attr_name,hash,MEMBERCACHE_MEMBER,chain);
  return type_obmemb_call_kw(self,chain,argc,argv,kw);
 }
 return ITER_DONE;
}
INTERN DREF DeeObject *DCALL
type_obmemb_docattr(DeeTypeObject *__restrict self,
                    char const *__restrict attr_name, dhash_t hash) {
 struct type_member *chain;
 ASSERT(self->tp_members);
 for (chain = self->tp_members; chain->m_name; ++chain) {
  if (strcmp(chain->m_name,attr_name)) continue;
  membercache_add(&self->tp_cache,attr_name,hash,MEMBERCACHE_MEMBER,chain);
  if unlikely(!chain->m_doc) {
   err_nodoc_attribute(self->tp_name,attr_name);
   return NULL;
  }
  return type_member_doc(chain);
 }
 return ITER_DONE;
}
INTERN DREF DeeObject *DCALL
type_member_getattr(struct membercache *__restrict cache,
                    struct type_member *__restrict chain, DeeObject *__restrict self,
                    char const *__restrict attr_name, dhash_t hash) {
 for (; chain->m_name; ++chain) {
  if (strcmp(chain->m_name,attr_name)) continue;
  membercache_add(cache,attr_name,hash,MEMBERCACHE_MEMBER,chain);
  return type_member_get(chain,self);
 }
 return ITER_DONE;
}
INTERN DREF DeeObject *DCALL
type_member_docattr(struct membercache *__restrict cache,
                    struct type_member *__restrict chain, char const *base,
                    char const *__restrict attr_name, dhash_t hash) {
 for (; chain->m_name; ++chain) {
  if (strcmp(chain->m_name,attr_name)) continue;
  membercache_add(cache,attr_name,hash,MEMBERCACHE_MEMBER,chain);
  if unlikely(!chain->m_doc) {
   err_nodoc_attribute(base,attr_name);
   return NULL;
  }
  return type_member_doc(chain);
 }
 return ITER_DONE;
}
INTERN int DCALL
type_getset_delattr(struct membercache *__restrict cache,
                    struct type_getset *__restrict chain, DeeObject *__restrict self,
                    char const *__restrict attr_name, dhash_t hash) {
 for (; chain->gs_name; ++chain) {
  if (strcmp(chain->gs_name,attr_name)) continue;
  membercache_add(cache,attr_name,hash,MEMBERCACHE_GETSET,chain);
  return type_getset_del(chain,self);
 }
 return 1;
}
INTERN int DCALL
type_getset_setattr(struct membercache *__restrict cache,
                    struct type_getset *__restrict chain, DeeObject *__restrict self,
                    char const *__restrict attr_name, dhash_t hash, DeeObject *__restrict value) {
 for (; chain->gs_name; ++chain) {
  if (strcmp(chain->gs_name,attr_name)) continue;
  membercache_add(cache,attr_name,hash,MEMBERCACHE_GETSET,chain);
  return type_getset_set(chain,self,value);
 }
 return 1;
}
INTERN int DCALL
type_member_delattr(struct membercache *__restrict cache,
                    struct type_member *__restrict chain, DeeObject *__restrict self,
                    char const *__restrict attr_name, dhash_t hash) {
 for (; chain->m_name; ++chain) {
  if (strcmp(chain->m_name,attr_name)) continue;
  membercache_add(cache,attr_name,hash,MEMBERCACHE_MEMBER,chain);
  return type_member_del(chain,self);
 }
 return 1;
}
INTERN int DCALL
type_member_setattr(struct membercache *__restrict cache,
                    struct type_member *__restrict chain, DeeObject *__restrict self,
                    char const *__restrict attr_name, dhash_t hash, DeeObject *__restrict value) {
 for (; chain->m_name; ++chain) {
  if (strcmp(chain->m_name,attr_name)) continue;
  membercache_add(cache,attr_name,hash,MEMBERCACHE_MEMBER,chain);
  return type_member_set(chain,self,value);
 }
 return 1;
}
INTERN bool DCALL
type_method_hasattr(struct membercache *__restrict cache,
                    struct type_method *__restrict chain,
                    char const *__restrict attr_name, dhash_t hash) {
 for (; chain->m_name; ++chain) {
  if (strcmp(chain->m_name,attr_name)) continue;
  membercache_add(cache,attr_name,hash,MEMBERCACHE_METHOD,chain);
  return true;
 }
 return false;
}
INTERN bool DCALL
type_obmeth_hasattr(DeeTypeObject *__restrict self,
                    char const *__restrict attr_name, dhash_t hash) {
 struct type_method *chain;
 ASSERT(self->tp_methods);
 for (chain = self->tp_methods; chain->m_name; ++chain) {
  if (strcmp(chain->m_name,attr_name)) continue;
  membercache_add(&self->tp_cache,attr_name,hash,MEMBERCACHE_METHOD,chain);
  return true;
 }
 return false;
}
INTERN bool DCALL
type_getset_hasattr(struct membercache *__restrict cache,
                    struct type_getset *__restrict chain,
                    char const *__restrict attr_name, dhash_t hash) {
 for (; chain->gs_name; ++chain) {
  if (strcmp(chain->gs_name,attr_name)) continue;
  membercache_add(cache,attr_name,hash,MEMBERCACHE_GETSET,chain);
  return true;
 }
 return false;
}
INTERN bool DCALL
type_obprop_hasattr(DeeTypeObject *__restrict self,
                    char const *__restrict attr_name, dhash_t hash) {
 struct type_getset *chain;
 ASSERT(self->tp_getsets);
 for (chain = self->tp_getsets; chain->gs_name; ++chain) {
  if (strcmp(chain->gs_name,attr_name)) continue;
  membercache_add(&self->tp_cache,attr_name,hash,MEMBERCACHE_GETSET,chain);
  return true;
 }
 return false;
}
INTERN bool DCALL
type_obmemb_hasattr(DeeTypeObject *__restrict self,
                    char const *__restrict attr_name, dhash_t hash) {
 struct type_member *chain;
 ASSERT(self->tp_members);
 for (chain = self->tp_members; chain->m_name; ++chain) {
  if (strcmp(chain->m_name,attr_name)) continue;
  membercache_add(&self->tp_cache,attr_name,hash,MEMBERCACHE_MEMBER,chain);
  return true;
 }
 return false;
}
INTERN bool DCALL
type_member_hasattr(struct membercache *__restrict cache,
                    struct type_member *__restrict chain,
                    char const *__restrict attr_name, dhash_t hash) {
 for (; chain->m_name; ++chain) {
  if (strcmp(chain->m_name,attr_name)) continue;
  membercache_add(cache,attr_name,hash,MEMBERCACHE_MEMBER,chain);
  return true;
 }
 return false;
}


INTERN int DCALL
type_method_find(struct membercache *__restrict cache, DeeObject *__restrict decl,
                 struct type_method *__restrict chain, uint16_t flags,
                 struct attribute_info *__restrict result,
                 struct attribute_lookup_rules const *__restrict rules) {
 flags |= ATTR_PERMGET|ATTR_PERMCALL;
 if ((flags & rules->alr_perm_mask) != rules->alr_perm_value)
      return 1;
 for (; chain->m_name; ++chain) {
  if (strcmp(chain->m_name,rules->alr_name)) continue;
  membercache_add(cache,
                  chain->m_name,
                  rules->alr_hash,
                  MEMBERCACHE_METHOD,
                  chain);
  result->a_doc = NULL;
  if (chain->m_doc) {
   result->a_doc = (DREF DeeStringObject *)type_method_doc(chain);
   if unlikely(!result->a_doc) return -1;
  }
  result->a_decl = decl;
  result->a_perm = flags;
  result->a_attrtype = &DeeObjMethod_Type;
  Dee_Incref(&DeeObjMethod_Type);
  Dee_Incref(decl);
  return 0;
 }
 return 1;
}

INTERN int DCALL
type_getset_find(struct membercache *__restrict cache, DeeObject *__restrict decl,
                 struct type_getset *__restrict chain, uint16_t flags,
                 struct attribute_info *__restrict result,
                 struct attribute_lookup_rules const *__restrict rules) {
 ASSERT(flags & ATTR_PROPERTY);
 for (; chain->gs_name; ++chain) {
  uint16_t perm = flags;
  if (chain->gs_get) perm |= ATTR_PERMGET;
  if (chain->gs_del) perm |= ATTR_PERMDEL;
  if (chain->gs_set) perm |= ATTR_PERMSET;
  if ((perm & rules->alr_perm_mask) != rules->alr_perm_value) continue;
  if (strcmp(chain->gs_name,rules->alr_name)) continue;
  membercache_add(cache,chain->gs_name,rules->alr_hash,MEMBERCACHE_GETSET,chain);
  result->a_doc = NULL;
  if (chain->gs_doc) {
   result->a_doc = (DREF DeeStringObject *)type_getset_doc(chain);
   if unlikely(!result->a_doc) return -1;
  }
  result->a_perm     = perm;
  result->a_decl     = decl;
  result->a_attrtype = NULL;
  Dee_Incref(decl);
  return 0;
 }
 return 1;
}

INTDEF DeeTypeObject *DCALL
type_member_typefor(struct type_member *__restrict self);

INTERN int DCALL
type_member_find(struct membercache *__restrict cache, DeeObject *__restrict decl,
                 struct type_member *__restrict chain, uint16_t flags,
                 struct attribute_info *__restrict result,
                 struct attribute_lookup_rules const *__restrict rules) {
 for (; chain->m_name; ++chain) {
  uint16_t perm;
  if (strcmp(chain->m_name,rules->alr_name)) continue;
  membercache_add(cache,chain->m_name,rules->alr_hash,MEMBERCACHE_MEMBER,chain);
  if (TYPE_MEMBER_ISCONST(chain)) {
   perm = flags|ATTR_PERMGET;
   if ((perm & rules->alr_perm_mask) != rules->alr_perm_value) continue;
   result->a_attrtype = Dee_TYPE(chain->m_const);
  } else {
   perm = flags|ATTR_PERMGET;
   if (!(chain->m_field.m_type&STRUCT_CONST))
         perm |= (ATTR_PERMDEL|ATTR_PERMSET);
   if ((perm & rules->alr_perm_mask) != rules->alr_perm_value) continue;
   result->a_attrtype = type_member_typefor(chain);
  }
  result->a_doc = NULL;
  if (chain->m_doc) {
   result->a_doc = (DREF DeeStringObject *)type_member_doc(chain);
   if unlikely(!result->a_doc) return -1;
  }
  result->a_decl = decl;
  result->a_perm = perm;
  Dee_Incref(decl);
  Dee_XIncref(result->a_attrtype);
  return 0;
 }
 return 1;
}


INTERN int DCALL
membercache_find(struct membercache *__restrict cache,
                 DeeObject *__restrict decl, uint16_t flags,
                 struct attribute_info *__restrict result,
                 struct attribute_lookup_rules const *__restrict rules) {
 dhash_t i,perturb;
 MEMBERCACHE_READ(cache);
 if unlikely(!cache->mc_table) goto done;
 perturb = i = MEMBERCACHE_HASHST(cache,rules->alr_hash);
 for (;; i = MEMBERCACHE_HASHNX(i,perturb),MEMBERCACHE_HASHPT(perturb)) {
  uint16_t perm;
  struct membercache_slot *item = MEMBERCACHE_HASHIT(cache,i);
  if (item->mcs_type == MEMBERCACHE_UNUSED) break;
  if (item->mcs_hash != rules->alr_hash) continue;
  if (strcmp(item->mcs_name,rules->alr_name)) continue;
  perm = flags;
  switch (item->mcs_type) {
  {
   char const *doc;
  case MEMBERCACHE_METHOD:
   doc = item->mcs_method.m_doc;
   MEMBERCACHE_ENDREAD(cache);
   perm |= ATTR_PERMGET|ATTR_PERMCALL;
   if ((perm & rules->alr_perm_mask) != rules->alr_perm_value)
        goto not_found;
   result->a_doc = NULL;
   if (doc) {
    result->a_doc = (DREF DeeStringObject *)DeeString_NewUtf8(doc,strlen(doc),STRING_ERROR_FIGNORE);
    if unlikely(!result->a_doc) return -1;
   }
   result->a_decl = decl;
   result->a_perm = perm;
   result->a_attrtype = &DeeObjMethod_Type;
   Dee_Incref(&DeeObjMethod_Type);
   Dee_Incref(decl);
   return 0;
  }
  {
   char const *doc;
  case MEMBERCACHE_GETSET:
   doc = item->mcs_getset.gs_doc;
   if (item->mcs_getset.gs_get) perm |= ATTR_PERMGET;
   if (item->mcs_getset.gs_del) perm |= ATTR_PERMDEL;
   if (item->mcs_getset.gs_set) perm |= ATTR_PERMSET;
   MEMBERCACHE_ENDREAD(cache);
   perm |= ATTR_PROPERTY;
   if ((perm & rules->alr_perm_mask) != rules->alr_perm_value)
        goto not_found;
   result->a_doc = NULL;
   if (doc) {
    result->a_doc = (DREF DeeStringObject *)DeeString_NewUtf8(doc,strlen(doc),STRING_ERROR_FIGNORE);
    if unlikely(!result->a_doc) return -1;
   }
   result->a_perm     = perm;
   result->a_decl     = decl;
   result->a_attrtype = NULL;
   Dee_Incref(decl);
   return 0;
  }
  {
   struct type_member member;
  case MEMBERCACHE_MEMBER:
   memcpy(&member,&item->mcs_member,sizeof(struct type_member));
   MEMBERCACHE_ENDREAD(cache);
   if (TYPE_MEMBER_ISCONST(&member)) {
    perm = flags|ATTR_PERMGET;
    if ((perm & rules->alr_perm_mask) != rules->alr_perm_value)
         goto not_found;
    result->a_attrtype = Dee_TYPE(member.m_const);
   } else {
    perm = flags|ATTR_PERMGET;
    if (!(member.m_field.m_type & STRUCT_CONST))
          perm |= (ATTR_PERMDEL|ATTR_PERMSET);
    if ((perm & rules->alr_perm_mask) != rules->alr_perm_value)
         goto not_found;
    result->a_attrtype = type_member_typefor(&member);
   }
   result->a_doc = NULL;
   if (member.m_doc) {
    result->a_doc = (DREF DeeStringObject *)type_member_doc(&member);
    if unlikely(!result->a_doc) return -1;
   }
   result->a_decl = decl;
   result->a_perm = perm;
   Dee_Incref(decl);
   Dee_XIncref(result->a_attrtype);
   return 0;
  }
  default: __builtin_unreachable();
  }
 }
done:
 MEMBERCACHE_ENDREAD(cache);
not_found:
 return 1;
}

INTERN int DCALL
membercache_findinstanceattr(DeeTypeObject *__restrict self,
                             struct attribute_info *__restrict result,
                             struct attribute_lookup_rules const *__restrict rules) {
 dhash_t i,perturb;
 MEMBERCACHE_READ(&self->tp_cache);
 if unlikely(!self->tp_cache.mc_table) goto done;
 perturb = i = MEMBERCACHE_HASHST(&self->tp_cache,rules->alr_hash);
 for (;; i = MEMBERCACHE_HASHNX(i,perturb),MEMBERCACHE_HASHPT(perturb)) {
  struct membercache_slot *item = MEMBERCACHE_HASHIT(&self->tp_cache,i);
  if (item->mcs_type == MEMBERCACHE_UNUSED) break;
  if (item->mcs_hash != rules->alr_hash) continue;
  if (strcmp(item->mcs_name,rules->alr_name)) continue;
  switch (item->mcs_type) {
  {
   uint16_t perm;
   char const *doc;
  case MEMBERCACHE_METHOD:
   doc = item->mcs_method.m_doc;
   MEMBERCACHE_ENDREAD(&self->tp_cache);
   perm = ATTR_IMEMBER|ATTR_CMEMBER|ATTR_PERMGET|ATTR_PERMCALL|ATTR_WRAPPER;
   if ((perm & rules->alr_perm_mask) != rules->alr_perm_value)
        goto not_found;
   result->a_doc = NULL;
   if (doc) {
    result->a_doc = (DREF DeeStringObject *)DeeString_NewUtf8(doc,strlen(doc),STRING_ERROR_FIGNORE);
    if unlikely(!result->a_doc) return -1;
   }
   result->a_decl = (DREF DeeObject *)self;
   result->a_perm = perm;
   result->a_attrtype = &DeeObjMethod_Type;
   Dee_Incref(&DeeObjMethod_Type);
   Dee_Incref(self);
   return 0;
  }
  {
   uint16_t perm;
   char const *doc;
  case MEMBERCACHE_GETSET:
   doc = item->mcs_getset.gs_doc;
   perm = ATTR_IMEMBER|ATTR_CMEMBER|ATTR_PROPERTY|ATTR_WRAPPER;
   if (item->mcs_getset.gs_get) perm |= ATTR_PERMGET;
   if (item->mcs_getset.gs_del) perm |= ATTR_PERMDEL;
   if (item->mcs_getset.gs_set) perm |= ATTR_PERMSET;
   MEMBERCACHE_ENDREAD(&self->tp_cache);
   if ((perm & rules->alr_perm_mask) != rules->alr_perm_value)
        goto not_found;
   result->a_doc = NULL;
   if (doc) {
    result->a_doc = (DREF DeeStringObject *)DeeString_NewUtf8(doc,strlen(doc),STRING_ERROR_FIGNORE);
    if unlikely(!result->a_doc) return -1;
   }
   result->a_perm     = perm;
   result->a_decl     = (DREF DeeObject *)self;
   result->a_attrtype = NULL;
   Dee_Incref(self);
   return 0;
  }
  {
   struct type_member member;
   uint16_t perm;
  case MEMBERCACHE_MEMBER:
   memcpy(&member,&item->mcs_member,sizeof(struct type_member));
   MEMBERCACHE_ENDREAD(&self->tp_cache);
   perm = ATTR_IMEMBER|ATTR_CMEMBER|ATTR_PERMGET|ATTR_WRAPPER;
   if (TYPE_MEMBER_ISCONST(&member)) {
    if ((perm & rules->alr_perm_mask) != rules->alr_perm_value)
         goto not_found;
    result->a_attrtype = Dee_TYPE(member.m_const);
   } else {
    if (!(member.m_field.m_type & STRUCT_CONST))
          perm |= (ATTR_PERMDEL|ATTR_PERMSET);
    if ((perm & rules->alr_perm_mask) != rules->alr_perm_value)
         goto not_found;
    result->a_attrtype = type_member_typefor(&member);
   }
   result->a_doc = NULL;
   if (member.m_doc) {
    result->a_doc = (DREF DeeStringObject *)type_member_doc(&member);
    if unlikely(!result->a_doc) return -1;
   }
   result->a_decl = (DREF DeeObject *)self;
   result->a_perm = perm;
   Dee_Incref(self);
   Dee_XIncref(result->a_attrtype);
   return 0;
  }
  default: __builtin_unreachable();
  }
 }
done:
 MEMBERCACHE_ENDREAD(&self->tp_cache);
not_found:
 return 1;
}




INTERN int DCALL
type_obmeth_find(DeeTypeObject *__restrict tp_self,
                 struct attribute_info *__restrict result,
                 struct attribute_lookup_rules const *__restrict rules) {
 struct type_method *chain;
 chain = tp_self->tp_methods;
 ASSERT(chain);
 for (; chain->m_name; ++chain) {
  if (((ATTR_IMEMBER|ATTR_CMEMBER|ATTR_PERMGET|ATTR_PERMCALL|ATTR_WRAPPER) &
        rules->alr_perm_mask) != rules->alr_perm_value)
        continue;
  if (strcmp(chain->m_name,rules->alr_name) != 0) continue;
  membercache_add(&tp_self->tp_cache,chain->m_name,rules->alr_hash,MEMBERCACHE_METHOD,chain);
  result->a_doc = NULL;
  if (chain->m_doc) {
   result->a_doc = (DREF DeeStringObject *)type_obmeth_doc(chain);
   if unlikely(!result->a_doc) return -1;
  }
  result->a_perm = ATTR_IMEMBER|ATTR_CMEMBER|ATTR_PERMGET|ATTR_PERMCALL|ATTR_WRAPPER;
  result->a_decl = (DeeObject *)tp_self;
  result->a_attrtype = &DeeObjMethod_Type; /* &DeeClsMethod_Type */
  Dee_Incref(tp_self);
  Dee_Incref(result->a_attrtype);
  return 0;
 }
 return 1;
}
INTERN int DCALL
type_obprop_find(DeeTypeObject *__restrict tp_self,
                 struct attribute_info *__restrict result,
                 struct attribute_lookup_rules const *__restrict rules) {
 struct type_getset *chain;
 chain = tp_self->tp_getsets;
 ASSERT(chain);
 for (; chain->gs_name; ++chain) {
  uint16_t perm = ATTR_IMEMBER|ATTR_CMEMBER|ATTR_PROPERTY|ATTR_WRAPPER;
  if (chain->gs_get) perm |= ATTR_PERMGET;
  if (chain->gs_del) perm |= ATTR_PERMDEL;
  if (chain->gs_set) perm |= ATTR_PERMSET;
  if ((perm & rules->alr_perm_mask) != rules->alr_perm_value) continue;
  if (strcmp(chain->gs_name,rules->alr_name) != 0) continue;
  membercache_add(&tp_self->tp_cache,chain->gs_name,rules->alr_hash,MEMBERCACHE_GETSET,chain);
  result->a_doc = NULL;
  if (chain->gs_doc) {
   result->a_doc = (DREF DeeStringObject *)type_obprop_doc(chain);
   if unlikely(!result->a_doc) return -1;
  }
  result->a_perm     = perm;
  result->a_decl     = (DeeObject *)tp_self;
  result->a_attrtype = NULL; /* &DeeClsProperty_Type */
  Dee_Incref(tp_self);
  return 0;
 }
 return 1;
}
INTERN int DCALL
type_obmemb_find(DeeTypeObject *__restrict tp_self,
                 struct attribute_info *__restrict result,
                 struct attribute_lookup_rules const *__restrict rules) {
 struct type_member *chain;
 chain = tp_self->tp_members;
 ASSERT(chain);
 for (; chain->m_name; ++chain) {
  uint16_t perm = ATTR_PERMGET|ATTR_IMEMBER|ATTR_CMEMBER|ATTR_WRAPPER;
  if (!(chain->m_field.m_type&STRUCT_CONST))
        perm |= ATTR_PERMDEL|ATTR_PERMSET;
  if ((perm & rules->alr_perm_mask) != rules->alr_perm_value) continue;
  if (strcmp(chain->m_name,rules->alr_name) != 0) continue;
  membercache_add(&tp_self->tp_cache,chain->m_name,rules->alr_hash,MEMBERCACHE_MEMBER,chain);
  result->a_doc = NULL;
  if (chain->m_doc) {
   result->a_doc = (DREF DeeStringObject *)type_obmemb_doc(chain);
   if unlikely(!result->a_doc) return -1;
  }
  result->a_perm     = perm;
  result->a_decl     = (DeeObject *)tp_self;
  result->a_attrtype = type_member_typefor(chain); /*&DeeClsMember_Type*/
  Dee_Incref(tp_self);
  Dee_XIncref(result->a_attrtype);
  return 0;
 }
 return 1;
}

#endif /* !CONFIG_USE_NEW_TYPE_ATTRIBUTE_CACHING */

DECL_END

#endif /* !GUARD_DEEMON_RUNTIME_MRO_C */
