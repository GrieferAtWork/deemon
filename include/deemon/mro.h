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
#ifndef GUARD_DEEMON_MRO_H
#define GUARD_DEEMON_MRO_H 1

#include "api.h"
#include "object.h"
#ifndef CONFIG_NO_THREADS
#include "util/rwlock.h"
#endif

DECL_BEGIN

/* MRO -- Method (or rather attribute) Resolution Order.
 *
 * Defines all the functions and types (except for `membercache' itself,
 * which needs to be defined in `object.h', because it appears 2x as an
 * inlined structure in each type object) that are required to resolve,
 * as well as cache the attributes of types, both builtin, and user-defined. */


struct class_attribute;
struct class_desc;
struct membercache_slot {
    /* A slot inside of a `struct membercache' table. */
#define MEMBERCACHE_UNUSED          0  /* Unused slot. */
#define MEMBERCACHE_METHOD          1  /* Method slot. */
#define MEMBERCACHE_GETSET          2  /* Getset slot. */
#define MEMBERCACHE_MEMBER          3  /* Member slot. */
#define MEMBERCACHE_ATTRIB          4  /* Class attribute. */
#define MEMBERCACHE_INSTANCE_METHOD 5  /* Same as `MEMBERCACHE_METHOD', but only found in `tp_class_cache', referring to an instance-method */
#define MEMBERCACHE_INSTANCE_GETSET 6  /* Same as `MEMBERCACHE_GETSET', but only found in `tp_class_cache', referring to an instance-getset */
#define MEMBERCACHE_INSTANCE_MEMBER 7  /* Same as `MEMBERCACHE_MEMBER', but only found in `tp_class_cache', referring to an instance-member */
#define MEMBERCACHE_INSTANCE_ATTRIB 8  /* Same as `MEMBERCACHE_ATTRIB', but only found in `tp_class_cache', referring to an instance-attribute */
#define MEMBERCACHE_COUNT           9  /* Amount of different cache types. */
    uint16_t               mcs_type;   /* The type of this slot (One of `MEMBERCACHE_*') */
    uint16_t               mcs_pad[(sizeof(void *)-2)/2];
    dhash_t                mcs_hash;   /* [valid_if(mcs_type != MEMBERCACHE_UNUSED)][== hash_str(mcs_name)] */
    DeeTypeObject         *mcs_decl;   /* [valid_if(mcs_type != MEMBERCACHE_UNUSED)][1..1][const]
                                        * One of the base-classes of type, providing this attribute. */
    union {
        char const        *mcs_name;   /* [valid_if(mcs_type != MEMBERCACHE_UNUSED)] */
        struct type_method mcs_method; /* [valid_if(mcs_type == MEMBERCACHE_METHOD || mcs_type == MEMBERCACHE_INSTANCE_METHOD)] */
        struct type_getset mcs_getset; /* [valid_if(mcs_type == MEMBERCACHE_GETSET || mcs_type == MEMBERCACHE_INSTANCE_GETSET)] */
        struct type_member mcs_member; /* [valid_if(mcs_type == MEMBERCACHE_MEMBER || mcs_type == MEMBERCACHE_INSTANCE_MEMBER)] */
        struct {
            char const             *a_name; /* [1..1][const][== DeeString_STR(a_attr->ca_name)] The attribute name. */
            struct class_attribute *a_attr; /* [1..1][const] The class attribute. */
            struct class_desc      *a_desc; /* [1..1][const][== DeeClass_DESC(mcs_decl)] The class implementing the attribute. */
        }                  mcs_attrib; /* [valid_if(mcs_type == MEMBERCACHE_ATTRIB || mcs_type == MEMBERCACHE_INSTANCE_ATTRIB)] */
    };
};


#ifndef CONFIG_NO_THREADS
#define MEMBERCACHE_READ(self)     rwlock_read(&(self)->mc_lock)
#define MEMBERCACHE_WRITE(self)    rwlock_write(&(self)->mc_lock)
#define MEMBERCACHE_ENDREAD(self)  rwlock_endread(&(self)->mc_lock)
#define MEMBERCACHE_ENDWRITE(self) rwlock_endwrite(&(self)->mc_lock)
#else /* !CONFIG_NO_THREADS */
#define MEMBERCACHE_READ(self)     (void)0
#define MEMBERCACHE_WRITE(self)    (void)0
#define MEMBERCACHE_ENDREAD(self)  (void)0
#define MEMBERCACHE_ENDWRITE(self) (void)0
#endif /* CONFIG_NO_THREADS */
#define MEMBERCACHE_HASHST(self,hash)  ((hash) & (self)->mc_mask)
#define MEMBERCACHE_HASHNX(hs,perturb) (((hs) << 2) + (hs) + (perturb) + 1)
#define MEMBERCACHE_HASHPT(perturb)    ((perturb) >>= 5) /* This `5' is tunable. */
#define MEMBERCACHE_HASHIT(self,i)     ((self)->mc_table+((i) & (self)->mc_mask))


#ifdef CONFIG_BUILDING_DEEMON
/* Finalize a given membercache. */
INTDEF void DCALL membercache_fini(struct membercache *__restrict self);

/* Try to insert a new caching point into the given membercache `self'.
 * @param: self: The cache to insert into.
 * @param: decl: The type providing the declaration. */
INTDEF void DCALL membercache_addmethod(struct membercache *__restrict self, DeeTypeObject *__restrict decl, dhash_t hash, struct type_method const *__restrict method);
INTDEF void DCALL membercache_addgetset(struct membercache *__restrict self, DeeTypeObject *__restrict decl, dhash_t hash, struct type_getset const *__restrict getset);
INTDEF void DCALL membercache_addmember(struct membercache *__restrict self, DeeTypeObject *__restrict decl, dhash_t hash, struct type_member const *__restrict member);
INTDEF void DCALL membercache_addattrib(struct membercache *__restrict self, DeeTypeObject *__restrict decl, dhash_t hash, struct class_attribute *__restrict attrib);
INTDEF void DCALL membercache_addinstancemethod(struct membercache *__restrict self, DeeTypeObject *__restrict decl, dhash_t hash, struct type_method const *__restrict method);
INTDEF void DCALL membercache_addinstancegetset(struct membercache *__restrict self, DeeTypeObject *__restrict decl, dhash_t hash, struct type_getset const *__restrict getset);
INTDEF void DCALL membercache_addinstancemember(struct membercache *__restrict self, DeeTypeObject *__restrict decl, dhash_t hash, struct type_member const *__restrict member);
INTDEF void DCALL membercache_addinstanceattrib(struct membercache *__restrict self, DeeTypeObject *__restrict decl, dhash_t hash, struct class_attribute *__restrict attrib);

#ifdef __INTELLISENSE__
/* Cache an instance member (e.g. `tp_methods') in `tp_cache'. */
INTDEF void DCALL DeeType_CacheMethod(DeeTypeObject *__restrict self, DeeTypeObject *__restrict decl, dhash_t hash,  struct type_method const *__restrict method);
INTDEF void DCALL DeeType_CacheGetSet(DeeTypeObject *__restrict self, DeeTypeObject *__restrict decl, dhash_t hash,  struct type_getset const *__restrict getset);
INTDEF void DCALL DeeType_CacheMember(DeeTypeObject *__restrict self, DeeTypeObject *__restrict decl, dhash_t hash,  struct type_member const *__restrict member);
INTDEF void DCALL DeeType_CacheAttrib(DeeTypeObject *__restrict self, DeeTypeObject *__restrict decl, dhash_t hash,  struct class_attribute const *__restrict attrib);
/* Cache a class member (e.g. `tp_class_methods') in `tp_class_cache'. */
INTDEF void DCALL DeeType_CacheClassMethod(DeeTypeObject *__restrict self, DeeTypeObject *__restrict decl, dhash_t hash,  struct type_method const *__restrict method);
INTDEF void DCALL DeeType_CacheClassGetSet(DeeTypeObject *__restrict self, DeeTypeObject *__restrict decl, dhash_t hash,  struct type_getset const *__restrict getset);
INTDEF void DCALL DeeType_CacheClassMember(DeeTypeObject *__restrict self, DeeTypeObject *__restrict decl, dhash_t hash,  struct type_member const *__restrict member);
INTDEF void DCALL DeeType_CacheClassAttrib(DeeTypeObject *__restrict self, DeeTypeObject *__restrict decl, dhash_t hash,  struct class_attribute const *__restrict attrib);
/* Cache an instance member (e.g. `tp_methods') in `tp_class_cache'. */
INTDEF void DCALL DeeType_CacheInstanceMethod(DeeTypeObject *__restrict self, DeeTypeObject *__restrict decl, dhash_t hash,  struct type_method const *__restrict method);
INTDEF void DCALL DeeType_CacheInstanceGetSet(DeeTypeObject *__restrict self, DeeTypeObject *__restrict decl, dhash_t hash,  struct type_getset const *__restrict getset);
INTDEF void DCALL DeeType_CacheInstanceMember(DeeTypeObject *__restrict self, DeeTypeObject *__restrict decl, dhash_t hash,  struct type_member const *__restrict member);
INTDEF void DCALL DeeType_CacheInstanceAttrib(DeeTypeObject *__restrict self, DeeTypeObject *__restrict decl, dhash_t hash,  struct class_attribute const *__restrict attrib);
#else
#define DeeType_CacheMethod(self,decl,hash,method)         membercache_addmethod(&(self)->tp_cache,decl,hash,method)
#define DeeType_CacheGetSet(self,decl,hash,getset)         membercache_addgetset(&(self)->tp_cache,decl,hash,getset)
#define DeeType_CacheMember(self,decl,hash,member)         membercache_addmember(&(self)->tp_cache,decl,hash,member)
#define DeeType_CacheAttrib(self,decl,hash,attrib)         membercache_addattrib(&(self)->tp_cache,decl,hash,attrib)
#define DeeType_CacheClassMethod(self,decl,hash,method)    membercache_addmethod(&(self)->tp_class_cache,decl,hash,method)
#define DeeType_CacheClassGetSet(self,decl,hash,getset)    membercache_addgetset(&(self)->tp_class_cache,decl,hash,getset)
#define DeeType_CacheClassMember(self,decl,hash,member)    membercache_addmember(&(self)->tp_class_cache,decl,hash,member)
#define DeeType_CacheClassAttrib(self,decl,hash,attrib)    membercache_addattrib(&(self)->tp_class_cache,decl,hash,attrib)
#define DeeType_CacheInstanceMethod(self,decl,hash,method) membercache_addinstancemethod(&(self)->tp_class_cache,decl,hash,method)
#define DeeType_CacheInstanceGetSet(self,decl,hash,getset) membercache_addinstancegetset(&(self)->tp_class_cache,decl,hash,getset)
#define DeeType_CacheInstanceMember(self,decl,hash,member) membercache_addinstancemember(&(self)->tp_class_cache,decl,hash,member)
#define DeeType_CacheInstanceAttrib(self,decl,hash,attrib) membercache_addinstanceattrib(&(self)->tp_class_cache,decl,hash,attrib)
#endif

/* NOTES:
 *  - DeeType_GetCachedAttr         -- `"foo".lower'
 *  - DeeType_GetCachedClassAttr    -- `string.chr', `string.lower'
 *  - DeeType_GetCachedInstanceAttr -- `string.getinstanceattr("lower")'
 */

/* Lookup an attribute from cache.
 * @return: * :        The attribute value.
 * @return: NULL:      An error occurred.
 * @return: ITER_DONE: The attribute could not be found in the cache. */
INTDEF DREF DeeObject *(DCALL DeeType_GetCachedAttr)(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, char const *__restrict name, dhash_t hash);
INTDEF DREF DeeObject *(DCALL DeeType_GetCachedClassAttr)(DeeTypeObject *__restrict tp_self, char const *__restrict name, dhash_t hash);
INTDEF DREF DeeObject *(DCALL DeeType_GetCachedInstanceAttr)(DeeTypeObject *__restrict tp_self, char const *__restrict name, dhash_t hash);

/* @return: * :        The attribute doc.
 * @return: NULL:      An error occurred.
 * @return: ITER_DONE: The attribute could not be found in the cache. */
INTDEF DREF DeeObject *(DCALL DeeType_DocCachedAttr)(DeeTypeObject *__restrict tp_self, char const *__restrict name, dhash_t hash);
INTDEF DREF DeeObject *(DCALL DeeType_DocCachedClassAttr)(DeeTypeObject *__restrict tp_self, char const *__restrict name, dhash_t hash);
#ifdef __INTELLISENSE__
INTDEF DREF DeeObject *(DCALL DeeType_DocCachedInstanceAttr)(DeeTypeObject *__restrict tp_self, char const *__restrict name, dhash_t hash);
#else
#define DeeType_DocCachedInstanceAttr(tp_self,name,hash) DeeType_DocCachedAttr(tp_self,name,hash)
#endif

/* @return: 1 : Attribute is bound.
 * @return: 0 : Attribute isn't bound.
 * @return: -1: An error occurred.
 * @return: -2: The attribute doesn't exist. */
INTDEF int (DCALL DeeType_BoundCachedAttr)(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, char const *__restrict name, dhash_t hash);
INTDEF int (DCALL DeeType_BoundCachedClassAttr)(DeeTypeObject *__restrict tp_self, char const *__restrict name, dhash_t hash);
INTDEF int (DCALL DeeType_BoundCachedInstanceAttr)(DeeTypeObject *__restrict tp_self, char const *__restrict name, dhash_t hash);

/* @return: true : The attribute exists.
 * @return: false: The attribute doesn't exist. */
INTDEF bool (DCALL DeeType_HasCachedAttr)(DeeTypeObject *__restrict tp_self, char const *__restrict name, dhash_t hash);
INTDEF bool (DCALL DeeType_HasCachedClassAttr)(DeeTypeObject *__restrict tp_self, char const *__restrict name, dhash_t hash);
#ifdef __INTELLISENSE__
INTDEF bool (DCALL DeeType_HasCachedInstanceAttr)(DeeTypeObject *__restrict tp_self, char const *__restrict name, dhash_t hash);
#else
#define DeeType_HasCachedInstanceAttr(tp_self,name,hash) DeeType_HasCachedAttr(tp_self,name,hash)
#endif

/* @return:  1: The attribute could not be found in the cache.
 * @return:  0: Successfully invoked the delete-operator on the attribute.
 * @return: -1: An error occurred. */
INTDEF int (DCALL DeeType_DelCachedAttr)(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, char const *__restrict name, dhash_t hash);
INTDEF int (DCALL DeeType_DelCachedClassAttr)(DeeTypeObject *__restrict tp_self, char const *__restrict name, dhash_t hash);
INTDEF int (DCALL DeeType_DelCachedInstanceAttr)(DeeTypeObject *__restrict tp_self, char const *__restrict name, dhash_t hash);

/* @return:  1: The attribute could not be found in the cache.
 * @return:  0: Successfully invoked the set-operator on the attribute.
 * @return: -1: An error occurred. */
INTDEF int (DCALL DeeType_SetCachedAttr)(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, char const *__restrict name, dhash_t hash, DeeObject *__restrict value);
INTDEF int (DCALL DeeType_SetCachedClassAttr)(DeeTypeObject *__restrict tp_self, char const *__restrict name, dhash_t hash, DeeObject *__restrict value);
INTDEF int (DCALL DeeType_SetCachedInstanceAttr)(DeeTypeObject *__restrict tp_self, char const *__restrict name, dhash_t hash, DeeObject *__restrict value);

/* @return:  1: The attribute could not be found in the cache.
 * @return:  0: Successfully invoked the set-operator on the attribute.
 * @return: -1: An error occurred. )(An error is also thrown for non-basic attributes) */
INTDEF int (DCALL DeeType_SetBasicCachedAttr)(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, char const *__restrict name, dhash_t hash, DeeObject *__restrict value);
INTDEF int (DCALL DeeType_SetBasicCachedClassAttr)(DeeTypeObject *__restrict tp_self, char const *__restrict name, dhash_t hash, DeeObject *__restrict value);
//INTDEF int (DCALL DeeType_SetBasicCachedInstanceAttr)(DeeTypeObject *__restrict tp_self, char const *__restrict name, dhash_t hash, DeeObject *__restrict value);

/* @return: * :        The returned value.
 * @return: NULL:      An error occurred.
 * @return: ITER_DONE: The attribute could not be found in the cache. */
INTDEF DREF DeeObject *(DCALL DeeType_CallCachedAttr)(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, char const *__restrict name, dhash_t hash, size_t argc, DeeObject **__restrict argv);
INTDEF DREF DeeObject *(DCALL DeeType_CallCachedClassAttr)(DeeTypeObject *__restrict tp_self, char const *__restrict name, dhash_t hash, size_t argc, DeeObject **__restrict argv);
#define DeeType_CallCachedInstanceAttr(tp_self,name,hash,argc,argv) DeeType_CallCachedInstanceAttrKw(tp_self,name,hash,argc,argv,NULL)
//INTDEF DREF DeeObject *(DCALL DeeType_CallCachedInstanceAttr)(DeeTypeObject *__restrict tp_self, char const *__restrict name, dhash_t hash, size_t argc, DeeObject **__restrict argv);
INTDEF DREF DeeObject *(DCALL DeeType_CallCachedAttrKw)(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, char const *__restrict name, dhash_t hash, size_t argc, DeeObject **__restrict argv, DeeObject *kw);
INTDEF DREF DeeObject *(DCALL DeeType_CallCachedClassAttrKw)(DeeTypeObject *__restrict tp_self, char const *__restrict name, dhash_t hash, size_t argc, DeeObject **__restrict argv, DeeObject *kw);
INTDEF DREF DeeObject *(DCALL DeeType_CallCachedInstanceAttrKw)(DeeTypeObject *__restrict tp_self, char const *__restrict name, dhash_t hash, size_t argc, DeeObject **__restrict argv, DeeObject *kw);

#ifdef CONFIG_HAVE_CALLTUPLE_OPTIMIZATIONS
/* @return: * :        The returned value.
 * @return: NULL:      An error occurred.
 * @return: ITER_DONE: The attribute could not be found in the cache. */
INTDEF DREF DeeObject *(DCALL DeeType_CallCachedAttrTuple)(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, char const *__restrict name, dhash_t hash, DeeObject *__restrict args);
INTDEF DREF DeeObject *(DCALL DeeType_CallCachedClassAttrTuple)(DeeTypeObject *__restrict tp_self, char const *__restrict name, dhash_t hash, DeeObject *__restrict args);
//INTDEF DREF DeeObject *(DCALL DeeType_CallCachedInstanceAttrTuple)(DeeTypeObject *__restrict tp_self, char const *__restrict name, dhash_t hash, DeeObject *__restrict args);
INTDEF DREF DeeObject *(DCALL DeeType_CallCachedAttrTupleKw)(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, char const *__restrict name, dhash_t hash, DeeObject *__restrict args, DeeObject *kw);
INTDEF DREF DeeObject *(DCALL DeeType_CallCachedClassAttrTupleKw)(DeeTypeObject *__restrict tp_self, char const *__restrict name, dhash_t hash, DeeObject *__restrict args, DeeObject *kw);
//INTDEF DREF DeeObject *(DCALL DeeType_CallCachedInstanceAttrTupleKw)(DeeTypeObject *__restrict tp_self, char const *__restrict name, dhash_t hash, DeeObject *__restrict args, DeeObject *kw);
#endif /* CONFIG_HAVE_CALLTUPLE_OPTIMIZATIONS */

/* @return: * :        The returned value.
 * @return: NULL:      An error occurred.
 * @return: ITER_DONE: The attribute could not be found in the cache. */
INTDEF DREF DeeObject *(DCALL DeeType_VCallCachedAttrf)(DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, char const *__restrict name, dhash_t hash, char const *__restrict format, va_list args);
INTDEF DREF DeeObject *(DCALL DeeType_VCallCachedClassAttrf)(DeeTypeObject *__restrict tp_self, char const *__restrict name, dhash_t hash, char const *__restrict format, va_list args);
//INTDEF DREF DeeObject *(DCALL DeeType_VCallCachedInstanceAttrf)(DeeTypeObject *__restrict tp_self, char const *__restrict name, dhash_t hash, char const *__restrict format, va_list args);

/* @return:  0: Attribute was found.
 * @return:  1: Attribute wasn't found.
 * @return: -1: An error occurred. */
INTDEF int (DCALL DeeType_FindCachedAttr)(DeeTypeObject *__restrict tp_self, DeeObject *instance, struct attribute_info *__restrict result, struct attribute_lookup_rules const *__restrict rules);
INTDEF int (DCALL DeeType_FindCachedClassAttr)(DeeTypeObject *__restrict tp_self, struct attribute_info *__restrict result, struct attribute_lookup_rules const *__restrict rules);



/* NOTES:
 *  - GetMethodAttr                  --   instance -> instance  ("foo".lower)                     (cache in `tp_cache' as `MEMBERCACHE_METHOD')
 *  - GetClassMethodAttr             --   class -> class        (string.chr)                      (cache in `tp_class_cache' as `MEMBERCACHE_METHOD')
 *  - DeeType_GetInstanceMethodAttr  --   class -> instance     (string.lower)                    (cache in `tp_class_cache' as `MEMBERCACHE_INSTANCE_METHOD')
 *  - DeeType_GetIInstanceMethodAttr --   class -> instance     (string.getinstanceattr("lower")) (cache in `tp_class' as `MEMBERCACHE_METHOD')
 */

/* Query user-class attributes, and cache them if some were found!
 * @return: * :   The attribute of the user-defined class.
 * @return: NULL: The attribute could not be found. */
#ifdef __INTELLISENSE__
INTDEF struct class_attribute *(DCALL DeeType_QueryAttribute)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, /*String*/DeeObject *__restrict name);
INTDEF struct class_attribute *(DCALL DeeType_QueryAttributeWithHash)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, /*String*/DeeObject *__restrict name, dhash_t hash);
INTDEF struct class_attribute *(DCALL DeeType_QueryAttributeString)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, char const *__restrict name);
INTDEF struct class_attribute *(DCALL DeeType_QueryAttributeStringWithHash)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, char const *__restrict name, dhash_t hash);
INTDEF struct class_attribute *(DCALL DeeType_QueryClassAttribute)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, /*String*/DeeObject *__restrict name);
INTDEF struct class_attribute *(DCALL DeeType_QueryClassAttributeWithHash)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, /*String*/DeeObject *__restrict name, dhash_t hash);
INTDEF struct class_attribute *(DCALL DeeType_QueryClassAttributeString)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, char const *__restrict name);
INTDEF struct class_attribute *(DCALL DeeType_QueryClassAttributeStringWithHash)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, char const *__restrict name, dhash_t hash);
INTDEF struct class_attribute *(DCALL DeeType_QueryInstanceAttribute)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, /*String*/DeeObject *__restrict name);
INTDEF struct class_attribute *(DCALL DeeType_QueryInstanceAttributeWithHash)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, /*String*/DeeObject *__restrict name, dhash_t hash);
INTDEF struct class_attribute *(DCALL DeeType_QueryInstanceAttributeString)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, char const *__restrict name);
INTDEF struct class_attribute *(DCALL DeeType_QueryInstanceAttributeStringWithHash)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, char const *__restrict name, dhash_t hash);
INTDEF struct class_attribute *(DCALL DeeType_QueryIInstanceAttribute)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, /*String*/DeeObject *__restrict name);
INTDEF struct class_attribute *(DCALL DeeType_QueryIInstanceAttributeWithHash)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, /*String*/DeeObject *__restrict name, dhash_t hash);
INTDEF struct class_attribute *(DCALL DeeType_QueryIInstanceAttributeString)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, char const *__restrict name);
INTDEF struct class_attribute *(DCALL DeeType_QueryIInstanceAttributeStringWithHash)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, char const *__restrict name, dhash_t hash);
#else
INTDEF struct class_attribute *(DCALL DeeType_QueryAttributeWithHash)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, /*String*/DeeObject *__restrict name, dhash_t hash);
INTDEF struct class_attribute *(DCALL DeeType_QueryAttributeStringWithHash)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, char const *__restrict name, dhash_t hash);
INTDEF struct class_attribute *(DCALL DeeType_QueryClassAttributeWithHash)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, /*String*/DeeObject *__restrict name, dhash_t hash);
INTDEF struct class_attribute *(DCALL DeeType_QueryClassAttributeStringWithHash)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, char const *__restrict name, dhash_t hash);
INTDEF struct class_attribute *(DCALL DeeType_QueryInstanceAttributeWithHash)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, /*String*/DeeObject *__restrict name, dhash_t hash);
INTDEF struct class_attribute *(DCALL DeeType_QueryInstanceAttributeStringWithHash)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, char const *__restrict name, dhash_t hash);
#define DeeType_QueryIInstanceAttributeWithHash(tp_invoker,tp_self,name,hash)       DeeType_QueryInstanceAttributeWithHash(tp_invoker,tp_self,name,hash)
#define DeeType_QueryIInstanceAttributeStringWithHash(tp_invoker,tp_self,name,hash) DeeType_QueryInstanceAttributeStringWithHash(tp_invoker,tp_self,name,hash)
#define DeeType_QueryAttribute(tp_invoker,tp_self,name)                DeeType_QueryAttributeWithHash(tp_invoker,tp_self,name,DeeString_Hash(name))
#define DeeType_QueryAttributeString(tp_invoker,tp_self,name)          DeeType_QueryAttributeStringWithHash(tp_invoker,tp_self,name,hash_str(name))
#define DeeType_QueryClassAttribute(tp_invoker,tp_self,name)           DeeType_QueryClassAttributeWithHash(tp_invoker,tp_self,name,DeeString_Hash(name))
#define DeeType_QueryClassAttributeString(tp_invoker,tp_self,name)     DeeType_QueryClassAttributeStringWithHash(tp_invoker,tp_self,name,hash_str(name))
#define DeeType_QueryInstanceAttribute(tp_invoker,tp_self,name)        DeeType_QueryInstanceAttributeWithHash(tp_invoker,tp_self,name,DeeString_Hash(name))
#define DeeType_QueryInstanceAttributeString(tp_invoker,tp_self,name)  DeeType_QueryInstanceAttributeStringWithHash(tp_invoker,tp_self,name,hash_str(name))
#define DeeType_QueryIInstanceAttribute(tp_invoker,tp_self,name)       DeeType_QueryIInstanceAttributeWithHash(tp_invoker,tp_self,name,DeeString_Hash(name))
#define DeeType_QueryIInstanceAttributeString(tp_invoker,tp_self,name) DeeType_QueryIInstanceAttributeStringWithHash(tp_invoker,tp_self,name,hash_str(name))
#endif

/* Invoke attributes from `tp_self->tp_methods' / `tp_self->tp_class_methods'.
 * @return: * :        The attribute value.
 * @return: NULL:      An error occurred.
 * @return: ITER_DONE: The attribute could not be found in `chain'. */
#ifdef __INTELLISENSE__
INTDEF DREF DeeObject *(DCALL DeeType_GetMethodAttr)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, char const *__restrict attr_name, dhash_t hash);
INTDEF DREF DeeObject *(DCALL DeeType_GetClassMethodAttr)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, char const *__restrict attr_name, dhash_t hash);
INTDEF DREF DeeObject *(DCALL DeeType_GetInstanceMethodAttr)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, char const *__restrict attr_name, dhash_t hash);
INTDEF DREF DeeObject *(DCALL DeeType_GetIInstanceMethodAttr)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, char const *__restrict attr_name, dhash_t hash);
INTDEF DREF DeeObject *(DCALL DeeType_DocMethodAttr)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, char const *__restrict attr_name, dhash_t hash);
INTDEF DREF DeeObject *(DCALL DeeType_DocClassMethodAttr)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, char const *__restrict attr_name, dhash_t hash);
INTDEF DREF DeeObject *(DCALL DeeType_DocInstanceMethodAttr)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, char const *__restrict attr_name, dhash_t hash);
INTDEF DREF DeeObject *(DCALL DeeType_DocIInstanceMethodAttr)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, char const *__restrict attr_name, dhash_t hash);
INTDEF DREF DeeObject *(DCALL DeeType_CallMethodAttr)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, char const *__restrict attr_name, dhash_t hash, size_t argc, DeeObject **__restrict argv);
INTDEF DREF DeeObject *(DCALL DeeType_CallClassMethodAttr)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, char const *__restrict attr_name, dhash_t hash, size_t argc, DeeObject **__restrict argv);
INTDEF DREF DeeObject *(DCALL DeeType_CallInstanceMethodAttr)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, char const *__restrict attr_name, dhash_t hash, size_t argc, DeeObject **__restrict argv);
INTDEF DREF DeeObject *(DCALL DeeType_CallIInstanceMethodAttr)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, char const *__restrict attr_name, dhash_t hash, size_t argc, DeeObject **__restrict argv);
INTDEF DREF DeeObject *(DCALL DeeType_CallMethodAttrKw)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, char const *__restrict attr_name, dhash_t hash, size_t argc, DeeObject **__restrict argv, DeeObject *kw);
INTDEF DREF DeeObject *(DCALL DeeType_CallClassMethodAttrKw)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, char const *__restrict attr_name, dhash_t hash, size_t argc, DeeObject **__restrict argv, DeeObject *kw);
INTDEF DREF DeeObject *(DCALL DeeType_CallInstanceMethodAttrKw)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, char const *__restrict attr_name, dhash_t hash, size_t argc, DeeObject **__restrict argv, DeeObject *kw);
INTDEF DREF DeeObject *(DCALL DeeType_CallIInstanceMethodAttrKw)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, char const *__restrict attr_name, dhash_t hash, size_t argc, DeeObject **__restrict argv, DeeObject *kw);
#ifdef CONFIG_HAVE_CALLTUPLE_OPTIMIZATIONS
INTDEF DREF DeeObject *(DCALL DeeType_CallMethodAttrTuple)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, char const *__restrict attr_name, dhash_t hash, DeeObject *__restrict args);
INTDEF DREF DeeObject *(DCALL DeeType_CallClassMethodAttrTuple)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, char const *__restrict attr_name, dhash_t hash, DeeObject *__restrict args);
INTDEF DREF DeeObject *(DCALL DeeType_CallInstanceMethodAttrTuple)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, char const *__restrict attr_name, dhash_t hash, DeeObject *__restrict args);
INTDEF DREF DeeObject *(DCALL DeeType_CallIInstanceMethodAttrTuple)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, char const *__restrict attr_name, dhash_t hash, DeeObject *__restrict args);
INTDEF DREF DeeObject *(DCALL DeeType_CallMethodAttrTupleKw)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, char const *__restrict attr_name, dhash_t hash, DeeObject *__restrict args, DeeObject *kw);
INTDEF DREF DeeObject *(DCALL DeeType_CallClassMethodAttrTupleKw)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, char const *__restrict attr_name, dhash_t hash, DeeObject *__restrict args, DeeObject *kw);
INTDEF DREF DeeObject *(DCALL DeeType_CallInstanceMethodAttrTupleKw)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, char const *__restrict attr_name, dhash_t hash, DeeObject *__restrict args, DeeObject *kw);
INTDEF DREF DeeObject *(DCALL DeeType_CallIInstanceMethodAttrTupleKw)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, char const *__restrict attr_name, dhash_t hash, DeeObject *__restrict args, DeeObject *kw);
#endif /* CONFIG_HAVE_CALLTUPLE_OPTIMIZATIONS */
INTDEF DREF DeeObject *(DCALL DeeType_VCallMethodAttrf)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, char const *__restrict attr_name, dhash_t hash, char const *__restrict format, va_list args);
INTDEF DREF DeeObject *(DCALL DeeType_VCallClassMethodAttrf)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, char const *__restrict attr_name, dhash_t hash, char const *__restrict format, va_list args);
//INTDEF DREF DeeObject *(DCALL DeeType_VCallInstanceMethodAttrf)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, char const *__restrict attr_name, dhash_t hash, char const *__restrict format, va_list args);
//INTDEF DREF DeeObject *(DCALL DeeType_VCallIInstanceMethodAttrf)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, char const *__restrict attr_name, dhash_t hash, char const *__restrict format, va_list args);
#else
INTDEF DREF DeeObject *DCALL /* GET_METHOD */
type_method_getattr(struct membercache *__restrict cache, DeeTypeObject *__restrict decl,
                    struct type_method *__restrict chain, DeeObject *__restrict self,
                    char const *__restrict attr_name, dhash_t hash);
#define DeeType_GetMethodAttr(tp_invoker,tp_self,self,attr_name,hash)     type_method_getattr(&(tp_invoker)->tp_cache,tp_self,(tp_self)->tp_methods,self,attr_name,hash)
#define DeeType_GetClassMethodAttr(tp_invoker,tp_self,attr_name,hash)     type_method_getattr(&(tp_invoker)->tp_class_cache,tp_self,(tp_self)->tp_class_methods,(DeeObject *)(tp_invoker),attr_name,hash)
INTDEF DREF DeeObject *(DCALL DeeType_GetInstanceMethodAttr)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, char const *__restrict attr_name, dhash_t hash);
INTDEF DREF DeeObject *(DCALL DeeType_GetIInstanceMethodAttr)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, char const *__restrict attr_name, dhash_t hash);

INTDEF DREF DeeObject *DCALL /* DOC_METHOD */
type_method_docattr(struct membercache *__restrict cache, DeeTypeObject *__restrict decl,
                    struct type_method *__restrict chain,
                    char const *__restrict attr_name, dhash_t hash);
#define DeeType_DocMethodAttr(tp_invoker,tp_self,attr_name,hash)          type_method_docattr(&(tp_invoker)->tp_cache,tp_self,(tp_self)->tp_methods,attr_name,hash)
#define DeeType_DocClassMethodAttr(tp_invoker,tp_self,attr_name,hash)     type_method_docattr(&(tp_invoker)->tp_class_cache,tp_self,(tp_self)->tp_class_methods,attr_name,hash)
INTDEF DREF DeeObject *(DCALL DeeType_DocInstanceMethodAttr)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, char const *__restrict attr_name, dhash_t hash);
#define DeeType_DocIInstanceMethodAttr(tp_invoker,tp_self,attr_name,hash) type_method_docattr(&(tp_invoker)->tp_cache,tp_self,(tp_self)->tp_methods,attr_name,hash)
                    
INTDEF DREF DeeObject *DCALL /* CALL_METHOD */
type_method_callattr(struct membercache *__restrict cache, DeeTypeObject *__restrict decl,
                     struct type_method *__restrict chain, DeeObject *__restrict self,
                     char const *__restrict attr_name, dhash_t hash,
                     size_t argc, DeeObject **__restrict argv);
#define DeeType_CallMethodAttr(tp_invoker,tp_self,self,attr_name,hash,argc,argv)     type_method_callattr(&(tp_invoker)->tp_cache,tp_self,(tp_self)->tp_methods,self,attr_name,hash,argc,argv)
#define DeeType_CallClassMethodAttr(tp_invoker,tp_self,attr_name,hash,argc,argv)     type_method_callattr(&(tp_invoker)->tp_class_cache,tp_self,(tp_self)->tp_class_methods,(DeeObject *)(tp_invoker),attr_name,hash,argc,argv)
INTDEF DREF DeeObject *(DCALL DeeType_CallInstanceMethodAttr)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, char const *__restrict attr_name, dhash_t hash, size_t argc, DeeObject **__restrict argv);
#define DeeType_CallIInstanceMethodAttr(tp_invoker,tp_self,attr_name,hash,argc,argv) \
        DeeType_CallIInstanceMethodAttrKw(tp_invoker,tp_self,attr_name,hash,argc,argv,NULL)

INTDEF DREF DeeObject *DCALL /* CALL_METHOD_KW */
type_method_callattr_kw(struct membercache *__restrict cache, DeeTypeObject *__restrict decl,
                        struct type_method *__restrict chain, DeeObject *__restrict self,
                        char const *__restrict attr_name, dhash_t hash,
                        size_t argc, DeeObject **__restrict argv, DeeObject *kw);
#define DeeType_CallMethodAttrKw(tp_invoker,tp_self,self,attr_name,hash,argc,argv,kw)  type_method_callattr_kw(&(tp_invoker)->tp_cache,tp_self,(tp_self)->tp_methods,self,attr_name,hash,argc,argv,kw)
#define DeeType_CallClassMethodAttrKw(tp_invoker,tp_self,attr_name,hash,argc,argv,kw)  type_method_callattr_kw(&(tp_invoker)->tp_class_cache,tp_self,(tp_self)->tp_class_methods,(DeeObject *)(tp_invoker),attr_name,hash,argc,argv,kw)
INTDEF DREF DeeObject *(DCALL DeeType_CallInstanceMethodAttrKw)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, char const *__restrict attr_name, dhash_t hash, size_t argc, DeeObject **__restrict argv, DeeObject *kw);
INTDEF DREF DeeObject *(DCALL DeeType_CallIInstanceMethodAttrKw)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, char const *__restrict attr_name, dhash_t hash, size_t argc, DeeObject **__restrict argv, DeeObject *kw);

#ifdef CONFIG_HAVE_CALLTUPLE_OPTIMIZATIONS
/* CALL_METHOD_TUPLE */
#define DeeType_CallMethodAttrTuple(tp_invoker,tp_self,self,attr_name,hash,args)          DeeType_CallMethodAttr(tp_invoker,tp_self,self,attr_name,hash,DeeTuple_SIZE(args),DeeTuple_ELEM(args))
#define DeeType_CallClassMethodAttrTuple(tp_invoker,tp_self,attr_name,hash,args)          DeeType_CallClassMethodAttr(tp_invoker,tp_self,attr_name,hash,DeeTuple_SIZE(args),DeeTuple_ELEM(args))
#define DeeType_CallInstanceMethodAttrTuple(tp_invoker,tp_self,attr_name,hash,args)       DeeType_CallInstanceMethodAttr(tp_invoker,tp_self,attr_name,hash,DeeTuple_SIZE(args),DeeTuple_ELEM(args))
#define DeeType_CallIInstanceMethodAttrTuple(tp_invoker,tp_self,attr_name,hash,args)      DeeType_CallIInstanceMethodAttr(tp_invoker,tp_self,attr_name,hash,DeeTuple_SIZE(args),DeeTuple_ELEM(args))
#define DeeType_CallMethodAttrTupleKw(tp_invoker,tp_self,self,attr_name,hash,args,kw)     DeeType_CallMethodAttrKw(tp_invoker,tp_self,self,attr_name,hash,DeeTuple_SIZE(args),DeeTuple_ELEM(args),kw)
#define DeeType_CallClassMethodAttrTupleKw(tp_invoker,tp_self,attr_name,hash,args,kw)     DeeType_CallClassMethodAttrKw(tp_invoker,tp_self,attr_name,hash,DeeTuple_SIZE(args),DeeTuple_ELEM(args),kw)
#define DeeType_CallInstanceMethodAttrTupleKw(tp_invoker,tp_self,attr_name,hash,args,kw)  DeeType_CallInstanceMethodAttrKw(tp_invoker,tp_self,attr_name,hash,DeeTuple_SIZE(args),DeeTuple_ELEM(args),kw)
#define DeeType_CallIInstanceMethodAttrTupleKw(tp_invoker,tp_self,attr_name,hash,args,kw) DeeType_CallIInstanceMethodAttrKw(tp_invoker,tp_self,attr_name,hash,DeeTuple_SIZE(args),DeeTuple_ELEM(args),kw)
#endif /* CONFIG_HAVE_CALLTUPLE_OPTIMIZATIONS */

INTDEF DREF DeeObject *DCALL /* CALL_METHOD */
type_method_vcallattrf(struct membercache *__restrict cache, DeeTypeObject *__restrict decl,
                       struct type_method *__restrict chain, DeeObject *__restrict self,
                       char const *__restrict attr_name, dhash_t hash,
                       char const *__restrict format, va_list args);
#define DeeType_VCallMethodAttrf(tp_invoker,tp_self,self,attr_name,hash,format,args)     type_method_vcallattrf(&(tp_invoker)->tp_cache,tp_self,(tp_self)->tp_methods,self,attr_name,hash,format,args)
#define DeeType_VCallClassMethodAttrf(tp_invoker,tp_self,attr_name,hash,format,args)     type_method_vcallattrf(&(tp_invoker)->tp_class_cache,tp_self,(tp_self)->tp_class_methods,(DeeObject *)(tp_invoker),attr_name,hash,format,args)
//INTDEF DREF DeeObject *(DCALL DeeType_VCallInstanceMethodAttrf)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, char const *__restrict attr_name, dhash_t hash, char const *__restrict format, va_list args);
//INTDEF DREF DeeObject *(DCALL DeeType_VCallIInstanceMethodAttrf)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, char const *__restrict attr_name, dhash_t hash, char const *__restrict format, va_list args);
#endif

/* Access attributes from `tp_self->tp_getsets' / `tp_self->tp_class_getsets'.
 * @return: -         /  - /  1: - / - / The attribute is bound.
 * @return: *         /  0 /  0: The attribute value / operation was successful / The attribute is unbound.
 * @return: NULL      / -1 / -1: An error occurred.
 * @return: ITER_DONE /  1 / -2: The attribute could not be found in `chain'. */
#ifdef __INTELLISENSE__
INTDEF DREF DeeObject *(DCALL DeeType_GetGetSetAttr)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, char const *__restrict attr_name, dhash_t hash);
INTDEF DREF DeeObject *(DCALL DeeType_GetClassGetSetAttr)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, char const *__restrict attr_name, dhash_t hash);
INTDEF DREF DeeObject *(DCALL DeeType_GetInstanceGetSetAttr)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, char const *__restrict attr_name, dhash_t hash);
INTDEF DREF DeeObject *(DCALL DeeType_GetIInstanceGetSetAttr)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, char const *__restrict attr_name, dhash_t hash);
INTDEF DREF DeeObject *(DCALL DeeType_CallInstanceGetSetAttr)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, char const *__restrict attr_name, dhash_t hash, size_t argc, DeeObject **__restrict argv);
INTDEF DREF DeeObject *(DCALL DeeType_CallIInstanceGetSetAttr)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, char const *__restrict attr_name, dhash_t hash, size_t argc, DeeObject **__restrict argv);
INTDEF DREF DeeObject *(DCALL DeeType_CallInstanceGetSetAttrKw)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, char const *__restrict attr_name, dhash_t hash, size_t argc, DeeObject **__restrict argv, DeeObject *kw);
INTDEF DREF DeeObject *(DCALL DeeType_CallIInstanceGetSetAttrKw)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, char const *__restrict attr_name, dhash_t hash, size_t argc, DeeObject **__restrict argv, DeeObject *kw);
INTDEF DREF DeeObject *(DCALL DeeType_DocGetSetAttr)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, char const *__restrict attr_name, dhash_t hash);
INTDEF DREF DeeObject *(DCALL DeeType_DocClassGetSetAttr)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, char const *__restrict attr_name, dhash_t hash);
INTDEF DREF DeeObject *(DCALL DeeType_DocInstanceGetSetAttr)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, char const *__restrict attr_name, dhash_t hash);
INTDEF DREF DeeObject *(DCALL DeeType_DocIInstanceGetSetAttr)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, char const *__restrict attr_name, dhash_t hash);
INTDEF int (DCALL DeeType_BoundGetSetAttr)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, char const *__restrict attr_name, dhash_t hash);
INTDEF int (DCALL DeeType_BoundClassGetSetAttr)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, char const *__restrict attr_name, dhash_t hash);
INTDEF int (DCALL DeeType_DelGetSetAttr)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, char const *__restrict attr_name, dhash_t hash);
INTDEF int (DCALL DeeType_DelClassGetSetAttr)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, char const *__restrict attr_name, dhash_t hash);
INTDEF int (DCALL DeeType_SetGetSetAttr)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, char const *__restrict attr_name, dhash_t hash, DeeObject *__restrict value);
INTDEF int (DCALL DeeType_SetClassGetSetAttr)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, char const *__restrict attr_name, dhash_t hash, DeeObject *__restrict value);
#else
INTDEF DREF DeeObject *DCALL /* GET_GETSET */
type_getset_getattr(struct membercache *__restrict cache, DeeTypeObject *__restrict decl,
                    struct type_getset *__restrict chain, DeeObject *__restrict self,
                    char const *__restrict attr_name, dhash_t hash);
#define DeeType_GetGetSetAttr(tp_invoker,tp_self,self,attr_name,hash) type_getset_getattr(&(tp_invoker)->tp_cache,tp_self,(tp_self)->tp_getsets,self,attr_name,hash)
#define DeeType_GetClassGetSetAttr(tp_invoker,tp_self,attr_name,hash) type_getset_getattr(&(tp_invoker)->tp_class_cache,tp_self,(tp_self)->tp_class_getsets,(DeeObject *)(tp_invoker),attr_name,hash)
INTDEF DREF DeeObject *(DCALL DeeType_GetInstanceGetSetAttr)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, char const *__restrict attr_name, dhash_t hash);
INTDEF DREF DeeObject *(DCALL DeeType_GetIInstanceGetSetAttr)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, char const *__restrict attr_name, dhash_t hash);
INTDEF DREF DeeObject *(DCALL DeeType_CallInstanceGetSetAttr)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, char const *__restrict attr_name, dhash_t hash, size_t argc, DeeObject **__restrict argv);
#define DeeType_CallIInstanceGetSetAttr(tp_invoker,tp_self,attr_name,hash,argc,argv) \
        DeeType_CallIInstanceGetSetAttrKw(tp_invoker,tp_self,attr_name,hash,argc,argv,NULL)
//INTDEF DREF DeeObject *(DCALL DeeType_CallIInstanceGetSetAttr)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, char const *__restrict attr_name, dhash_t hash, size_t argc, DeeObject **__restrict argv);
INTDEF DREF DeeObject *(DCALL DeeType_CallInstanceGetSetAttrKw)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, char const *__restrict attr_name, dhash_t hash, size_t argc, DeeObject **__restrict argv, DeeObject *kw);
INTDEF DREF DeeObject *(DCALL DeeType_CallIInstanceGetSetAttrKw)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, char const *__restrict attr_name, dhash_t hash, size_t argc, DeeObject **__restrict argv, DeeObject *kw);

INTDEF DREF DeeObject *DCALL /* DOC_GETSET */
type_getset_docattr(struct membercache *__restrict cache,
                    DeeTypeObject *__restrict decl,
                    struct type_getset *__restrict chain,
                    char const *__restrict attr_name, dhash_t hash);
#define DeeType_DocGetSetAttr(tp_invoker,tp_self,attr_name,hash)          type_getset_docattr(&(tp_invoker)->tp_cache,tp_self,(tp_self)->tp_getsets,attr_name,hash)
#define DeeType_DocClassGetSetAttr(tp_invoker,tp_self,attr_name,hash)     type_getset_docattr(&(tp_invoker)->tp_class_cache,tp_self,(tp_self)->tp_class_getsets,attr_name,hash)
INTDEF DREF DeeObject *(DCALL DeeType_DocInstanceGetSetAttr)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, char const *__restrict attr_name, dhash_t hash);
#define DeeType_DocIInstanceGetSetAttr(tp_invoker,tp_self,attr_name,hash) type_getset_docattr(&(tp_invoker)->tp_cache,tp_self,(tp_self)->tp_getsets,attr_name,hash)

INTDEF int DCALL /* BOUND_GETSET */
type_getset_boundattr(struct membercache *__restrict cache, DeeTypeObject *__restrict decl,
                      struct type_getset *__restrict chain, DeeObject *__restrict self,
                      char const *__restrict attr_name, dhash_t hash);
#define DeeType_BoundGetSetAttr(tp_invoker,tp_self,self,attr_name,hash)    type_getset_boundattr(&(tp_invoker)->tp_cache,tp_self,(tp_self)->tp_getsets,self,attr_name,hash)
#define DeeType_BoundClassGetSetAttr(tp_invoker,tp_self,attr_name,hash)    type_getset_boundattr(&(tp_invoker)->tp_class_cache,tp_self,(tp_self)->tp_class_getsets,(DeeObject *)(tp_invoker),attr_name,hash)

INTDEF int DCALL /* DEL_GETSET */
type_getset_delattr(struct membercache *__restrict cache, DeeTypeObject *__restrict decl,
                    struct type_getset *__restrict chain, DeeObject *__restrict self,
                    char const *__restrict attr_name, dhash_t hash);
#define DeeType_DelGetSetAttr(tp_invoker,tp_self,self,attr_name,hash)    type_getset_delattr(&(tp_invoker)->tp_cache,tp_self,(tp_self)->tp_getsets,self,attr_name,hash)
#define DeeType_DelClassGetSetAttr(tp_invoker,tp_self,attr_name,hash)    type_getset_delattr(&(tp_invoker)->tp_class_cache,tp_self,(tp_self)->tp_class_getsets,(DeeObject *)(tp_invoker),attr_name,hash)

INTDEF int DCALL /* SET_GETSET */
type_getset_setattr(struct membercache *__restrict cache, DeeTypeObject *__restrict decl,
                    struct type_getset *__restrict chain, DeeObject *__restrict self,
                    char const *__restrict attr_name, dhash_t hash, DeeObject *__restrict value);
#define DeeType_SetGetSetAttr(tp_invoker,tp_self,self,attr_name,hash,value)    type_getset_setattr(&(tp_invoker)->tp_cache,tp_self,(tp_self)->tp_getsets,self,attr_name,hash,value)
#define DeeType_SetClassGetSetAttr(tp_invoker,tp_self,attr_name,hash,value)    type_getset_setattr(&(tp_invoker)->tp_class_cache,tp_self,(tp_self)->tp_class_getsets,(DeeObject *)(tp_invoker),attr_name,hash,value)
#endif

/* Access attributes from `tp_self->tp_members' / `tp_self->tp_class_members'.
 * @return: -         /  - /  1: - / - / The attribute is bound.
 * @return: *         /  0 /  0: The attribute value / operation was successful / The attribute is unbound.
 * @return: NULL      / -1 / -1: An error occurred.
 * @return: ITER_DONE /  1 / -2: The attribute could not be found in `chain'. */
#ifdef __INTELLISENSE__
INTDEF DREF DeeObject *(DCALL DeeType_GetMemberAttr)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, char const *__restrict attr_name, dhash_t hash);
INTDEF DREF DeeObject *(DCALL DeeType_GetClassMemberAttr)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, char const *__restrict attr_name, dhash_t hash);
INTDEF DREF DeeObject *(DCALL DeeType_GetInstanceMemberAttr)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, char const *__restrict attr_name, dhash_t hash);
INTDEF DREF DeeObject *(DCALL DeeType_GetIInstanceMemberAttr)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, char const *__restrict attr_name, dhash_t hash);
INTDEF DREF DeeObject *(DCALL DeeType_CallInstanceMemberAttr)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, char const *__restrict attr_name, dhash_t hash, size_t argc, DeeObject **__restrict argv);
INTDEF DREF DeeObject *(DCALL DeeType_CallIInstanceMemberAttr)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, char const *__restrict attr_name, dhash_t hash, size_t argc, DeeObject **__restrict argv);
INTDEF DREF DeeObject *(DCALL DeeType_CallInstanceMemberAttrKw)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, char const *__restrict attr_name, dhash_t hash, size_t argc, DeeObject **__restrict argv, DeeObject *kw);
INTDEF DREF DeeObject *(DCALL DeeType_CallIInstanceMemberAttrKw)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, char const *__restrict attr_name, dhash_t hash, size_t argc, DeeObject **__restrict argv, DeeObject *kw);
INTDEF DREF DeeObject *(DCALL DeeType_DocMemberAttr)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, char const *__restrict attr_name, dhash_t hash);
INTDEF DREF DeeObject *(DCALL DeeType_DocClassMemberAttr)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, char const *__restrict attr_name, dhash_t hash);
INTDEF DREF DeeObject *(DCALL DeeType_DocInstanceMemberAttr)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, char const *__restrict attr_name, dhash_t hash);
INTDEF DREF DeeObject *(DCALL DeeType_DocIInstanceMemberAttr)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, char const *__restrict attr_name, dhash_t hash);
INTDEF int (DCALL DeeType_BoundMemberAttr)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, char const *__restrict attr_name, dhash_t hash);
INTDEF int (DCALL DeeType_BoundClassMemberAttr)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, char const *__restrict attr_name, dhash_t hash);
INTDEF int (DCALL DeeType_DelMemberAttr)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, char const *__restrict attr_name, dhash_t hash);
INTDEF int (DCALL DeeType_DelClassMemberAttr)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, char const *__restrict attr_name, dhash_t hash);
INTDEF int (DCALL DeeType_SetMemberAttr)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, DeeObject *__restrict self, char const *__restrict attr_name, dhash_t hash, DeeObject *__restrict value);
INTDEF int (DCALL DeeType_SetClassMemberAttr)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, char const *__restrict attr_name, dhash_t hash, DeeObject *__restrict value);
#else
INTDEF DREF DeeObject *DCALL /* GET_MEMBER */
type_member_getattr(struct membercache *__restrict cache, DeeTypeObject *__restrict decl,
                    struct type_member *__restrict chain, DeeObject *__restrict self,
                    char const *__restrict attr_name, dhash_t hash);
#define DeeType_GetMemberAttr(tp_invoker,tp_self,self,attr_name,hash) type_member_getattr(&(tp_invoker)->tp_cache,tp_self,(tp_self)->tp_members,self,attr_name,hash)
#define DeeType_GetClassMemberAttr(tp_invoker,tp_self,attr_name,hash) type_member_getattr(&(tp_invoker)->tp_class_cache,tp_self,(tp_self)->tp_class_members,(DeeObject *)(tp_invoker),attr_name,hash)
INTDEF DREF DeeObject *(DCALL DeeType_GetInstanceMemberAttr)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, char const *__restrict attr_name, dhash_t hash);
INTDEF DREF DeeObject *(DCALL DeeType_GetIInstanceMemberAttr)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, char const *__restrict attr_name, dhash_t hash);
INTDEF DREF DeeObject *(DCALL DeeType_CallInstanceMemberAttr)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, char const *__restrict attr_name, dhash_t hash, size_t argc, DeeObject **__restrict argv);
#define DeeType_CallIInstanceMemberAttr(tp_invoker,tp_self,attr_name,hash,argc,argv) \
        DeeType_CallIInstanceMemberAttrKw(tp_invoker,tp_self,attr_name,hash,argc,argv,NULL)
//INTDEF DREF DeeObject *(DCALL DeeType_CallIInstanceMemberAttr)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, char const *__restrict attr_name, dhash_t hash, size_t argc, DeeObject **__restrict argv);
INTDEF DREF DeeObject *(DCALL DeeType_CallInstanceMemberAttrKw)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, char const *__restrict attr_name, dhash_t hash, size_t argc, DeeObject **__restrict argv, DeeObject *kw);
INTDEF DREF DeeObject *(DCALL DeeType_CallIInstanceMemberAttrKw)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, char const *__restrict attr_name, dhash_t hash, size_t argc, DeeObject **__restrict argv, DeeObject *kw);

INTDEF DREF DeeObject *DCALL /* DOC_MEMBER */
type_member_docattr(struct membercache *__restrict cache,
                    DeeTypeObject *__restrict decl,
                    struct type_member *__restrict chain,
                    char const *__restrict attr_name, dhash_t hash);
#define DeeType_DocMemberAttr(tp_invoker,tp_self,attr_name,hash)          type_member_docattr(&(tp_invoker)->tp_cache,tp_self,(tp_self)->tp_members,attr_name,hash)
#define DeeType_DocClassMemberAttr(tp_invoker,tp_self,attr_name,hash)     type_member_docattr(&(tp_invoker)->tp_class_cache,tp_self,(tp_self)->tp_class_members,attr_name,hash)
INTDEF DREF DeeObject *(DCALL DeeType_DocInstanceMemberAttr)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, char const *__restrict attr_name, dhash_t hash);
#define DeeType_DocIInstanceMemberAttr(tp_invoker,tp_self,attr_name,hash) type_member_docattr(&(tp_invoker)->tp_cache,tp_self,(tp_self)->tp_members,attr_name,hash)

INTDEF int DCALL /* BOUND_MEMBER */
type_member_boundattr(struct membercache *__restrict cache, DeeTypeObject *__restrict decl,
                      struct type_member *__restrict chain, DeeObject *__restrict self,
                      char const *__restrict attr_name, dhash_t hash);
#define DeeType_BoundMemberAttr(tp_invoker,tp_self,self,attr_name,hash)    type_member_boundattr(&(tp_invoker)->tp_cache,tp_self,(tp_self)->tp_members,self,attr_name,hash)
#define DeeType_BoundClassMemberAttr(tp_invoker,tp_self,attr_name,hash)    type_member_boundattr(&(tp_invoker)->tp_class_cache,tp_self,(tp_self)->tp_class_members,(DeeObject *)(tp_invoker),attr_name,hash)

INTDEF int DCALL /* DEL_MEMBER */
type_member_delattr(struct membercache *__restrict cache, DeeTypeObject *__restrict decl,
                    struct type_member *__restrict chain, DeeObject *__restrict self,
                    char const *__restrict attr_name, dhash_t hash);
#define DeeType_DelMemberAttr(tp_invoker,tp_self,self,attr_name,hash)    type_member_delattr(&(tp_invoker)->tp_cache,tp_self,(tp_self)->tp_members,self,attr_name,hash)
#define DeeType_DelClassMemberAttr(tp_invoker,tp_self,attr_name,hash)    type_member_delattr(&(tp_invoker)->tp_class_cache,tp_self,(tp_self)->tp_class_members,(DeeObject *)(tp_invoker),attr_name,hash)

INTDEF int DCALL /* SET_MEMBER */
type_member_setattr(struct membercache *__restrict cache, DeeTypeObject *__restrict decl,
                    struct type_member *__restrict chain, DeeObject *__restrict self,
                    char const *__restrict attr_name, dhash_t hash, DeeObject *__restrict value);
#define DeeType_SetMemberAttr(tp_invoker,tp_self,self,attr_name,hash,value)    type_member_setattr(&(tp_invoker)->tp_cache,tp_self,(tp_self)->tp_members,self,attr_name,hash,value)
#define DeeType_SetClassMemberAttr(tp_invoker,tp_self,attr_name,hash,value)    type_member_setattr(&(tp_invoker)->tp_class_cache,tp_self,(tp_self)->tp_class_members,(DeeObject *)(tp_invoker),attr_name,hash,value)
#endif


/* Check for the existence of specific attributes. */
#ifdef __INTELLISENSE__
INTDEF bool (DCALL DeeType_HasMethodAttr)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, char const *__restrict attr_name, dhash_t hash);
INTDEF bool (DCALL DeeType_HasClassMethodAttr)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, char const *__restrict attr_name, dhash_t hash);
INTDEF bool (DCALL DeeType_HasInstanceMethodAttr)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, char const *__restrict attr_name, dhash_t hash);
INTDEF bool (DCALL DeeType_HasIInstanceMethodAttr)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, char const *__restrict attr_name, dhash_t hash);
INTDEF bool (DCALL DeeType_HasGetSetAttr)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, char const *__restrict attr_name, dhash_t hash);
INTDEF bool (DCALL DeeType_HasClassGetSetAttr)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, char const *__restrict attr_name, dhash_t hash);
INTDEF bool (DCALL DeeType_HasInstanceGetSetAttr)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, char const *__restrict attr_name, dhash_t hash);
INTDEF bool (DCALL DeeType_HasIInstanceGetSetAttr)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, char const *__restrict attr_name, dhash_t hash);
INTDEF bool (DCALL DeeType_HasMemberAttr)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, char const *__restrict attr_name, dhash_t hash);
INTDEF bool (DCALL DeeType_HasClassMemberAttr)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, char const *__restrict attr_name, dhash_t hash);
INTDEF bool (DCALL DeeType_HasInstanceMemberAttr)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, char const *__restrict attr_name, dhash_t hash);
INTDEF bool (DCALL DeeType_HasIInstanceMemberAttr)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, char const *__restrict attr_name, dhash_t hash);
#else
INTDEF bool DCALL /* METHOD */
type_method_hasattr(struct membercache *__restrict cache,
                    DeeTypeObject *__restrict decl,
                    struct type_method *__restrict chain,
                    char const *__restrict attr_name, dhash_t hash);
#define DeeType_HasMethodAttr(tp_invoker,tp_self,attr_name,hash)          type_method_hasattr(&(tp_invoker)->tp_cache,tp_self,(tp_self)->tp_methods,attr_name,hash)
#define DeeType_HasClassMethodAttr(tp_invoker,tp_self,attr_name,hash)     type_method_hasattr(&(tp_invoker)->tp_class_cache,tp_self,(tp_self)->tp_class_methods,attr_name,hash)
INTDEF bool (DCALL DeeType_HasInstanceMethodAttr)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, char const *__restrict attr_name, dhash_t hash);
#define DeeType_HasIInstanceMethodAttr(tp_invoker,tp_self,attr_name,hash) type_method_hasattr(&(tp_invoker)->tp_cache,tp_self,(tp_self)->tp_methods,attr_name,hash)

INTDEF bool DCALL /* GETSET */
type_getset_hasattr(struct membercache *__restrict cache,
                    DeeTypeObject *__restrict decl,
                    struct type_getset *__restrict chain,
                    char const *__restrict attr_name, dhash_t hash);
#define DeeType_HasGetSetAttr(tp_invoker,tp_self,attr_name,hash)         type_getset_hasattr(&(tp_invoker)->tp_cache,tp_self,(tp_self)->tp_getsets,attr_name,hash)
#define DeeType_HasClassGetSetAttr(tp_invoker,tp_self,attr_name,hash)    type_getset_hasattr(&(tp_invoker)->tp_class_cache,tp_self,(tp_self)->tp_class_getsets,attr_name,hash)
INTDEF bool (DCALL DeeType_HasInstanceGetSetAttr)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, char const *__restrict attr_name, dhash_t hash);
#define DeeType_HasIInstanceGetSetAttr(tp_invoker,tp_self,attr_name,hash) type_getset_hasattr(&(tp_invoker)->tp_cache,tp_self,(tp_self)->tp_getsets,attr_name,hash)

INTDEF bool DCALL /* MEMBER */
type_member_hasattr(struct membercache *__restrict cache,
                    DeeTypeObject *__restrict decl,
                    struct type_member *__restrict chain,
                    char const *__restrict attr_name, dhash_t hash);
#define DeeType_HasMemberAttr(tp_invoker,tp_self,attr_name,hash)         type_member_hasattr(&(tp_invoker)->tp_cache,tp_self,(tp_self)->tp_members,attr_name,hash)
#define DeeType_HasClassMemberAttr(tp_invoker,tp_self,attr_name,hash)    type_member_hasattr(&(tp_invoker)->tp_class_cache,tp_self,(tp_self)->tp_class_members,attr_name,hash)
INTDEF bool DCALL DeeType_HasInstanceMemberAttr(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, char const *__restrict attr_name, dhash_t hash);
#define DeeType_HasIInstanceMemberAttr(tp_invoker,tp_self,attr_name,hash) type_member_hasattr(&(tp_invoker)->tp_cache,tp_self,(tp_self)->tp_members,attr_name,hash)
#endif

/* Find matching attributes. */
#ifdef __INTELLISENSE__
INTDEF int (DCALL DeeType_FindMethodAttr)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, struct attribute_info *__restrict result, struct attribute_lookup_rules const *__restrict rules);
INTDEF int (DCALL DeeType_FindClassMethodAttr)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, struct attribute_info *__restrict result, struct attribute_lookup_rules const *__restrict rules);
INTDEF int (DCALL DeeType_FindInstanceMethodAttr)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, struct attribute_info *__restrict result, struct attribute_lookup_rules const *__restrict rules);
INTDEF int (DCALL DeeType_FindGetSetAttr)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, struct attribute_info *__restrict result, struct attribute_lookup_rules const *__restrict rules);
INTDEF int (DCALL DeeType_FindClassGetSetAttr)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, struct attribute_info *__restrict result, struct attribute_lookup_rules const *__restrict rules);
INTDEF int (DCALL DeeType_FindInstanceGetSetAttr)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, struct attribute_info *__restrict result, struct attribute_lookup_rules const *__restrict rules);
INTDEF int (DCALL DeeType_FindMemberAttr)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, struct attribute_info *__restrict result, struct attribute_lookup_rules const *__restrict rules);
INTDEF int (DCALL DeeType_FindClassMemberAttr)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, struct attribute_info *__restrict result, struct attribute_lookup_rules const *__restrict rules);
INTDEF int (DCALL DeeType_FindInstanceMemberAttr)(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, struct attribute_info *__restrict result, struct attribute_lookup_rules const *__restrict rules);
#else
INTDEF int DCALL /* METHOD */
type_method_findattr(struct membercache *__restrict cache, DeeTypeObject *__restrict decl,
                     struct type_method *__restrict chain, uint16_t perm,
                     struct attribute_info *__restrict result,
                     struct attribute_lookup_rules const *__restrict rules);
#define DeeType_FindMethodAttr(tp_invoker,tp_self,result,rules)         type_method_findattr(&(tp_invoker)->tp_cache,tp_self,(tp_self)->tp_methods,ATTR_IMEMBER,result,rules)
#define DeeType_FindClassMethodAttr(tp_invoker,tp_self,result,rules)    type_method_findattr(&(tp_invoker)->tp_class_cache,tp_self,(tp_self)->tp_class_methods,ATTR_CMEMBER,result,rules)
INTDEF int DCALL DeeType_FindInstanceMethodAttr(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, struct attribute_info *__restrict result, struct attribute_lookup_rules const *__restrict rules);

INTDEF int DCALL /* GETSET */
type_getset_findattr(struct membercache *__restrict cache, DeeTypeObject *__restrict decl,
                     struct type_getset *__restrict chain, uint16_t perm,
                     struct attribute_info *__restrict result,
                     struct attribute_lookup_rules const *__restrict rules);
#define DeeType_FindGetSetAttr(tp_invoker,tp_self,result,rules)         type_getset_findattr(&(tp_invoker)->tp_cache,tp_self,(tp_self)->tp_getsets,ATTR_IMEMBER,result,rules)
#define DeeType_FindClassGetSetAttr(tp_invoker,tp_self,result,rules)    type_getset_findattr(&(tp_invoker)->tp_class_cache,tp_self,(tp_self)->tp_class_getsets,ATTR_CMEMBER,result,rules)
INTDEF int DCALL DeeType_FindInstanceGetSetAttr(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, struct attribute_info *__restrict result, struct attribute_lookup_rules const *__restrict rules);

INTDEF int DCALL /* MEMBER */
type_member_findattr(struct membercache *__restrict cache, DeeTypeObject *__restrict decl,
                     struct type_member *__restrict chain, uint16_t perm,
                     struct attribute_info *__restrict result,
                     struct attribute_lookup_rules const *__restrict rules);
#define DeeType_FindMemberAttr(tp_invoker,tp_self,result,rules)         type_member_findattr(&(tp_invoker)->tp_cache,tp_self,(tp_self)->tp_members,ATTR_IMEMBER,result,rules)
#define DeeType_FindClassMemberAttr(tp_invoker,tp_self,result,rules)    type_member_findattr(&(tp_invoker)->tp_class_cache,tp_self,(tp_self)->tp_class_members,ATTR_CMEMBER,result,rules)
INTDEF int DCALL DeeType_FindInstanceMemberAttr(DeeTypeObject *__restrict tp_invoker, DeeTypeObject *__restrict tp_self, struct attribute_info *__restrict result, struct attribute_lookup_rules const *__restrict rules);
#endif

/* Enumerate attributes. */
INTDEF dssize_t DCALL type_method_enum(DeeTypeObject *__restrict tp_self, struct type_method *__restrict chain, uint16_t flags, denum_t proc, void *arg);
INTDEF dssize_t DCALL type_getset_enum(DeeTypeObject *__restrict tp_self, struct type_getset *__restrict chain, uint16_t flags, denum_t proc, void *arg);
INTDEF dssize_t DCALL type_member_enum(DeeTypeObject *__restrict tp_self, struct type_member *__restrict chain, uint16_t flags, denum_t proc, void *arg);
INTDEF dssize_t DCALL type_obmeth_enum(DeeTypeObject *__restrict tp_self, denum_t proc, void *arg);
INTDEF dssize_t DCALL type_obprop_enum(DeeTypeObject *__restrict tp_self, denum_t proc, void *arg);
INTDEF dssize_t DCALL type_obmemb_enum(DeeTypeObject *__restrict tp_self, denum_t proc, void *arg);

/* Helper functions for accessing type attributes. */
#define type_method_get(desc,self) \
  (((desc)->m_flag & TYPE_METHOD_FKWDS) ? DeeKwObjMethod_New((dkwobjmethod_t)(desc)->m_func,self) \
                                        : DeeObjMethod_New((desc)->m_func,self))
#define type_method_doc(desc)             DeeString_NewUtf8((desc)->m_doc,strlen((desc)->m_doc),STRING_ERROR_FIGNORE)
#define type_method_call(desc,self,argc,argv) \
  (((desc)->m_flag & TYPE_METHOD_FKWDS) ? DeeKwObjMethod_CallFunc((dkwobjmethod_t)(desc)->m_func,self,argc,argv,NULL) \
                                        : DeeObjMethod_CallFunc((desc)->m_func,self,argc,argv))
#define type_method_call_kw(desc,self,argc,argv,kw) \
  (((desc)->m_flag & TYPE_METHOD_FKWDS) ? DeeKwObjMethod_CallFunc((dkwobjmethod_t)(desc)->m_func,self,argc,argv,kw) \
                                        : type_method_call_kw_normal(desc,self,argc,argv,kw))
INTDEF DREF DeeObject *DCALL type_method_call_kw_normal(struct type_method *__restrict desc, DeeObject *__restrict self, size_t argc, DeeObject **__restrict argv, DeeObject *kw);
#define type_obmeth_get(cls_type,desc) \
  (((desc)->m_flag & TYPE_METHOD_FKWDS) ? DeeKwClsMethod_New(cls_type,(dkwobjmethod_t)(desc)->m_func) \
                                        : DeeClsMethod_New(cls_type,(desc)->m_func))
#define type_obmeth_doc(desc)             DeeString_NewUtf8((desc)->m_doc,strlen((desc)->m_doc),STRING_ERROR_FIGNORE)
INTDEF DREF DeeObject *DCALL type_obmeth_call(DeeTypeObject *__restrict cls_type, struct type_method *__restrict desc, size_t argc, DeeObject **__restrict argv);
INTDEF DREF DeeObject *DCALL type_obmeth_call_kw(DeeTypeObject *__restrict cls_type, struct type_method *__restrict desc, size_t argc, DeeObject **__restrict argv, DeeObject *kw);
INTDEF DREF DeeObject *DCALL type_getset_get(struct type_getset *__restrict desc, DeeObject *__restrict self);
#define type_obprop_get(cls_type,desc)  DeeClsProperty_New(cls_type,(desc)->gs_get,(desc)->gs_del,(desc)->gs_set)
INTDEF DREF DeeObject *DCALL type_obprop_call(DeeTypeObject *__restrict cls_type, struct type_getset *__restrict desc, size_t argc, DeeObject **__restrict argv);
INTDEF DREF DeeObject *DCALL type_obprop_call_kw(DeeTypeObject *__restrict cls_type, struct type_getset *__restrict desc, size_t argc, DeeObject **__restrict argv, DeeObject *kw);
#define type_obprop_doc(desc)             DeeString_NewUtf8((desc)->gs_doc,strlen((desc)->gs_doc),STRING_ERROR_FIGNORE)
#define type_obmemb_get(cls_type,desc)    DeeClsMember_New(cls_type,desc)
#define type_obmemb_doc(desc)             DeeString_NewUtf8((desc)->m_doc,strlen((desc)->m_doc),STRING_ERROR_FIGNORE)
INTDEF DREF DeeObject *DCALL type_obmemb_call(DeeTypeObject *__restrict cls_type, struct type_member *__restrict desc, size_t argc, DeeObject **__restrict argv);
INTDEF DREF DeeObject *DCALL type_obmemb_call_kw(DeeTypeObject *__restrict cls_type, struct type_member *__restrict desc, size_t argc, DeeObject **__restrict argv, DeeObject *kw);
#define type_getset_doc(desc)             DeeString_NewUtf8((desc)->gs_doc,strlen((desc)->gs_doc),STRING_ERROR_FIGNORE)
INTDEF int DCALL type_getset_del(struct type_getset *__restrict desc, DeeObject *__restrict self);
INTDEF int DCALL type_getset_set(struct type_getset *__restrict desc, DeeObject *__restrict self, DeeObject *__restrict value);
INTDEF DREF DeeObject *DCALL type_member_get(struct type_member *__restrict desc, DeeObject *__restrict self);
INTDEF bool DCALL type_member_bound(struct type_member *__restrict desc, DeeObject *__restrict self);
#define type_member_doc(desc)             DeeString_NewUtf8((desc)->m_doc,strlen((desc)->m_doc),STRING_ERROR_FIGNORE)
#define type_member_del(desc,self)        type_member_set(desc,self,Dee_None)
INTDEF int DCALL type_member_set(struct type_member *__restrict desc, DeeObject *__restrict self, DeeObject *__restrict value);


/* Static (class) attribute access for type objects. */
INTDEF DREF DeeObject *DCALL DeeType_GetAttrString(DeeTypeObject *__restrict self, char const *__restrict name, dhash_t hash);
INTDEF int DCALL DeeType_BoundAttrString(DeeTypeObject *__restrict self, char const *__restrict name, dhash_t hash);
INTDEF int DCALL DeeType_FindAttrString(DeeTypeObject *__restrict self, struct attribute_info *__restrict result, struct attribute_lookup_rules const *__restrict rules);
INTDEF DREF DeeObject *DCALL DeeType_DocAttrString(DeeTypeObject *__restrict self, char const *__restrict name, dhash_t hash);
INTDEF DREF DeeObject *DCALL DeeType_CallAttrString(DeeTypeObject *__restrict self, char const *__restrict name, dhash_t hash, size_t argc, DeeObject **__restrict argv);
INTDEF DREF DeeObject *DCALL DeeType_CallAttrStringKw(DeeTypeObject *__restrict self, char const *__restrict name, dhash_t hash, size_t argc, DeeObject **__restrict argv, DeeObject *kw);
INTDEF bool DCALL DeeType_HasAttrString(DeeTypeObject *__restrict self, char const *__restrict name, dhash_t hash);
INTDEF int DCALL DeeType_DelAttrString(DeeTypeObject *__restrict self, char const *__restrict name, dhash_t hash);
INTDEF int DCALL DeeType_SetAttrString(DeeTypeObject *__restrict self, char const *__restrict name, dhash_t hash, DeeObject *__restrict value);
INTDEF dssize_t DCALL DeeType_EnumAttr(DeeTypeObject *__restrict self, denum_t proc, void *arg);

/* Instance-only (wrapper) attribute access to the attributes of instance of types. */
INTDEF DREF DeeObject *DCALL DeeType_GetInstanceAttrString(DeeTypeObject *__restrict self, char const *__restrict name, dhash_t hash);
INTDEF DREF DeeObject *DCALL DeeType_DocInstanceAttrString(DeeTypeObject *__restrict self, char const *__restrict name, dhash_t hash);
INTDEF DREF DeeObject *DCALL DeeType_CallInstanceAttrStringKw(DeeTypeObject *__restrict self, char const *__restrict name, dhash_t hash, size_t argc, DeeObject **__restrict argv, DeeObject *kw);
INTDEF bool DCALL DeeType_HasInstanceAttrString(DeeTypeObject *__restrict self, char const *__restrict name, dhash_t hash);
INTDEF int DCALL DeeType_BoundInstanceAttrString(DeeTypeObject *__restrict self, char const *__restrict name, dhash_t hash);
INTDEF int (DCALL DeeType_DelInstanceAttrString)(DeeTypeObject *__restrict self, char const *__restrict name, dhash_t hash);
INTDEF int (DCALL DeeType_SetInstanceAttrString)(DeeTypeObject *__restrict self, char const *__restrict name, dhash_t hash, DeeObject *__restrict value);


#ifndef __INTELLISENSE__
#ifndef __NO_builtin_expect
#define DeeType_DelAttrString(self,name,hash)                  __builtin_expect(DeeType_DelAttrString(self,name,hash),0)
#define DeeType_SetAttrString(self,name,hash,value)            __builtin_expect(DeeType_SetAttrString(self,name,hash,value),0)
#define DeeType_DelInstanceAttrString(self,name,hash)          __builtin_expect(DeeType_DelInstanceAttrString(self,name,hash),0)
#define DeeType_SetInstanceAttrString(self,name,hash,value)    __builtin_expect(DeeType_SetInstanceAttrString(self,name,hash,value),0)
#endif /* !__NO_builtin_expect */
#endif

#endif /* CONFIG_BUILDING_DEEMON */

DECL_END

#endif /* !GUARD_DEEMON_MRO_H */
