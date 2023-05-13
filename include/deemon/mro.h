/* Copyright (c) 2018-2023 Griefer@Work                                       *
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
 *    in a product, an acknowledgement (see the following) in the product     *
 *    documentation is required:                                              *
 *    Portions Copyright (c) 2018-2023 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_MRO_H
#define GUARD_DEEMON_MRO_H 1

#include "api.h"

#include "object.h"
#include "util/lock.h"

#include <hybrid/sched/__yield.h>

DECL_BEGIN

/* MRO -- Method (or rather Attribute) Resolution Order.
 *
 * Defines all the functions and types (except for `Dee_membercache' itself,
 * which needs to be defined in `object.h', because it appears 2x as an
 * inlined structure in every type object) that are required to resolve,
 * as well as cache the attributes of types/classes, both builtin, and
 * user-defined. */

#ifdef DEE_SOURCE
#define Dee_class_attribute class_attribute
#define Dee_class_desc      class_desc
#endif /* DEE_SOURCE */

#ifdef CONFIG_BUILDING_DEEMON

/* Type codes for `struct Dee_membercache_slot::mcs_type' */
#define MEMBERCACHE_UNUSED          0  /* Unused slot. */
#define MEMBERCACHE_UNINITIALIZED   1  /* Uninitialized slot (when encountered, keep searching)
                                        * Used as a marker for a slot that is currently being
                                        * entered into the cache. */
#define MEMBERCACHE_METHOD          2  /* Method slot. */
#define MEMBERCACHE_GETSET          3  /* Getset slot. */
#define MEMBERCACHE_MEMBER          4  /* Member slot. */
#define MEMBERCACHE_ATTRIB          5  /* Class attribute. */
#define MEMBERCACHE_INSTANCE_METHOD 6  /* Same as `MEMBERCACHE_METHOD', but only found in `tp_class_cache', referring to an instance-method */
#define MEMBERCACHE_INSTANCE_GETSET 7  /* Same as `MEMBERCACHE_GETSET', but only found in `tp_class_cache', referring to an instance-getset */
#define MEMBERCACHE_INSTANCE_MEMBER 8  /* Same as `MEMBERCACHE_MEMBER', but only found in `tp_class_cache', referring to an instance-member */
#define MEMBERCACHE_INSTANCE_ATTRIB 9  /* Same as `MEMBERCACHE_ATTRIB', but only found in `tp_class_cache', referring to an instance-attribute */
#define MEMBERCACHE_COUNT           10 /* Amount of different cache types. */

struct Dee_class_attribute;
struct Dee_class_desc;
struct Dee_membercache_slot {
	/* A slot inside of a `struct Dee_membercache' table. */
	uint16_t               mcs_type;   /* The type of this slot (One of `MEMBERCACHE_*') */
	uint16_t              _mcs_pad[(sizeof(void *)-2)/2];
	Dee_hash_t             mcs_hash;   /* [valid_if(mcs_type != MEMBERCACHE_UNUSED && mcs_type != MEMBERCACHE_UNINITIALIZED)][== Dee_HashStr(mcs_name)] */
	DeeTypeObject         *mcs_decl;   /* [valid_if(mcs_type != MEMBERCACHE_UNUSED && mcs_type != MEMBERCACHE_UNINITIALIZED)][1..1][const]
	                                    * The type that is providing this attribute, which must be
	                                    * the associated type itself, or one of its base-classes. */
	union {
		char const            *mcs_name;   /* [valid_if(mcs_type != MEMBERCACHE_UNUSED && mcs_type != MEMBERCACHE_UNINITIALIZED)] */
		struct Dee_type_method mcs_method; /* [valid_if(mcs_type == MEMBERCACHE_METHOD || mcs_type == MEMBERCACHE_INSTANCE_METHOD)] */
		struct Dee_type_getset mcs_getset; /* [valid_if(mcs_type == MEMBERCACHE_GETSET || mcs_type == MEMBERCACHE_INSTANCE_GETSET)] */
		struct Dee_type_member mcs_member; /* [valid_if(mcs_type == MEMBERCACHE_MEMBER || mcs_type == MEMBERCACHE_INSTANCE_MEMBER)] */
		struct {
			char const                 *a_name; /* [1..1][const][== DeeString_STR(a_attr->ca_name)] The attribute attr. */
			struct Dee_class_attribute *a_attr; /* [1..1][const] The class attribute. */
			struct Dee_class_desc      *a_desc; /* [1..1][const][== DeeClass_DESC(mcs_decl)] The class implementing the attribute. */
		} mcs_attrib; /* [valid_if(mcs_type == MEMBERCACHE_ATTRIB || mcs_type == MEMBERCACHE_INSTANCE_ATTRIB)] */
	}
#ifndef __COMPILER_HAVE_TRANSPARENT_UNION
	_dee_aunion
#define mcs_name   _dee_aunion.mcs_name
#define mcs_method _dee_aunion.mcs_method
#define mcs_getset _dee_aunion.mcs_getset
#define mcs_member _dee_aunion.mcs_member
#define mcs_attrib _dee_aunion.mcs_attrib
#endif /* !__COMPILER_HAVE_TRANSPARENT_UNION */
	;
};

struct Dee_membercache_table {
	Dee_refcnt_t                                         mc_refcnt; /* [lock(ATOMIC)] Reference counter. */
	size_t                                               mc_mask;   /* [const] Allocated table size -1. */
	size_t                                               mc_size;   /* [lock(ATOMIC)] Amount of used table entries (always `<= mc_mask'). */
	COMPILER_FLEXIBLE_ARRAY(struct Dee_membercache_slot, mc_table); /* [0..mc_mask+1] Member cache table. */
};

/* Hashing functions for `Dee_membercache_table' */
#define Dee_membercache_table_hashst(self, hash)  ((hash) & (self)->mc_mask)
#define Dee_membercache_table_hashnx(hs, perturb) (void)((hs) = ((hs) << 2) + (hs) + (perturb) + 1, (perturb) >>= 5) /* This `5' is tunable. */
#define Dee_membercache_table_hashit(self, i)     ((self)->mc_table+((i) & (self)->mc_mask))

/* Member-cache table reference counting functions. */
#define Dee_membercache_table_destroy(self) Dee_Free(self)
#define Dee_membercache_table_incref(self)  __hybrid_atomic_inc(&(self)->mc_refcnt, __ATOMIC_SEQ_CST)
#define Dee_membercache_table_decref(self)  (void)(__hybrid_atomic_decfetch(&(self)->mc_refcnt, __ATOMIC_SEQ_CST) || (Dee_membercache_table_destroy(self), 0))

/* Member-cache synchronization helpers. */
#ifndef CONFIG_NO_THREADS
#define Dee_membercache_waitfor(self)                                           \
	do {                                                                        \
		while (__hybrid_atomic_load(&(self)->mc_tabuse, __ATOMIC_ACQUIRE) != 0) \
			__hybrid_yield();                                                   \
	}	__WHILE0
#define Dee_membercache_tabuse_inc(self) __hybrid_atomic_inc(&(self)->mc_tabuse, __ATOMIC_ACQUIRE)
#define Dee_membercache_tabuse_dec(self) __hybrid_atomic_dec(&(self)->mc_tabuse, __ATOMIC_RELEASE)
#else /* !CONFIG_NO_THREADS */
#define Dee_membercache_waitfor(self)    (void)0
#define Dee_membercache_tabuse_inc(self) (void)0
#define Dee_membercache_tabuse_dec(self) (void)0
#endif /* CONFIG_NO_THREADS */

/* Finalize a given member-cache. */
INTDEF NONNULL((1)) void DCALL Dee_membercache_fini(struct Dee_membercache *__restrict self);

/* Try to insert a new caching point into the given Dee_membercache `self'.
 * @param: self: The cache to insert into.
 * @param: decl: The type providing the declaration. */
INTDEF NONNULL((1, 2, 4)) bool DCALL Dee_membercache_addmethod(struct Dee_membercache *self, DeeTypeObject *decl, dhash_t hash, struct type_method const *method);
INTDEF NONNULL((1, 2, 4)) bool DCALL Dee_membercache_addgetset(struct Dee_membercache *self, DeeTypeObject *decl, dhash_t hash, struct type_getset const *getset);
INTDEF NONNULL((1, 2, 4)) bool DCALL Dee_membercache_addmember(struct Dee_membercache *self, DeeTypeObject *decl, dhash_t hash, struct type_member const *member);
INTDEF NONNULL((1, 2, 4)) bool DCALL Dee_membercache_addattrib(struct Dee_membercache *self, DeeTypeObject *decl, dhash_t hash, struct Dee_class_attribute *attrib);
INTDEF NONNULL((1, 2, 4)) bool DCALL Dee_membercache_addinstancemethod(struct Dee_membercache *self, DeeTypeObject *decl, dhash_t hash, struct type_method const *method);
INTDEF NONNULL((1, 2, 4)) bool DCALL Dee_membercache_addinstancegetset(struct Dee_membercache *self, DeeTypeObject *decl, dhash_t hash, struct type_getset const *getset);
INTDEF NONNULL((1, 2, 4)) bool DCALL Dee_membercache_addinstancemember(struct Dee_membercache *self, DeeTypeObject *decl, dhash_t hash, struct type_member const *member);
INTDEF NONNULL((1, 2, 4)) bool DCALL Dee_membercache_addinstanceattrib(struct Dee_membercache *self, DeeTypeObject *decl, dhash_t hash, struct Dee_class_attribute *attrib);

#ifdef __INTELLISENSE__
/* Cache an instance member (e.g. `tp_methods') in `tp_cache'. */
INTDEF NONNULL((1, 2, 4)) bool DCALL DeeType_CacheMethod(DeeTypeObject *self, DeeTypeObject *decl, dhash_t hash, struct type_method const *method);
INTDEF NONNULL((1, 2, 4)) bool DCALL DeeType_CacheGetSet(DeeTypeObject *self, DeeTypeObject *decl, dhash_t hash, struct type_getset const *getset);
INTDEF NONNULL((1, 2, 4)) bool DCALL DeeType_CacheMember(DeeTypeObject *self, DeeTypeObject *decl, dhash_t hash, struct type_member const *member);
INTDEF NONNULL((1, 2, 4)) bool DCALL DeeType_CacheAttrib(DeeTypeObject *self, DeeTypeObject *decl, dhash_t hash, struct Dee_class_attribute const *__restrict attrib);
/* Cache a class member (e.g. `tp_class_methods') in `tp_class_cache'. */
INTDEF NONNULL((1, 2, 4)) bool DCALL DeeType_CacheClassMethod(DeeTypeObject *self, DeeTypeObject *decl, dhash_t hash, struct type_method const *method);
INTDEF NONNULL((1, 2, 4)) bool DCALL DeeType_CacheClassGetSet(DeeTypeObject *self, DeeTypeObject *decl, dhash_t hash, struct type_getset const *getset);
INTDEF NONNULL((1, 2, 4)) bool DCALL DeeType_CacheClassMember(DeeTypeObject *self, DeeTypeObject *decl, dhash_t hash, struct type_member const *member);
INTDEF NONNULL((1, 2, 4)) bool DCALL DeeType_CacheClassAttrib(DeeTypeObject *self, DeeTypeObject *decl, dhash_t hash, struct Dee_class_attribute const *__restrict attrib);
/* Cache an instance member (e.g. `tp_methods') in `tp_class_cache'. */
INTDEF NONNULL((1, 2, 4)) bool DCALL DeeType_CacheInstanceMethod(DeeTypeObject *self, DeeTypeObject *decl, dhash_t hash, struct type_method const *method);
INTDEF NONNULL((1, 2, 4)) bool DCALL DeeType_CacheInstanceGetSet(DeeTypeObject *self, DeeTypeObject *decl, dhash_t hash, struct type_getset const *getset);
INTDEF NONNULL((1, 2, 4)) bool DCALL DeeType_CacheInstanceMember(DeeTypeObject *self, DeeTypeObject *decl, dhash_t hash, struct type_member const *member);
INTDEF NONNULL((1, 2, 4)) bool DCALL DeeType_CacheInstanceAttrib(DeeTypeObject *self, DeeTypeObject *decl, dhash_t hash, struct Dee_class_attribute const *__restrict attrib);
#else /* __INTELLISENSE__ */
#define DeeType_CacheMethod(self, decl, hash, method)         Dee_membercache_addmethod(&(self)->tp_cache, decl, hash, method)
#define DeeType_CacheGetSet(self, decl, hash, getset)         Dee_membercache_addgetset(&(self)->tp_cache, decl, hash, getset)
#define DeeType_CacheMember(self, decl, hash, member)         Dee_membercache_addmember(&(self)->tp_cache, decl, hash, member)
#define DeeType_CacheAttrib(self, decl, hash, attrib)         Dee_membercache_addattrib(&(self)->tp_cache, decl, hash, attrib)
#define DeeType_CacheClassMethod(self, decl, hash, method)    Dee_membercache_addmethod(&(self)->tp_class_cache, decl, hash, method)
#define DeeType_CacheClassGetSet(self, decl, hash, getset)    Dee_membercache_addgetset(&(self)->tp_class_cache, decl, hash, getset)
#define DeeType_CacheClassMember(self, decl, hash, member)    Dee_membercache_addmember(&(self)->tp_class_cache, decl, hash, member)
#define DeeType_CacheClassAttrib(self, decl, hash, attrib)    Dee_membercache_addattrib(&(self)->tp_class_cache, decl, hash, attrib)
#define DeeType_CacheInstanceMethod(self, decl, hash, method) Dee_membercache_addinstancemethod(&(self)->tp_class_cache, decl, hash, method)
#define DeeType_CacheInstanceGetSet(self, decl, hash, getset) Dee_membercache_addinstancegetset(&(self)->tp_class_cache, decl, hash, getset)
#define DeeType_CacheInstanceMember(self, decl, hash, member) Dee_membercache_addinstancemember(&(self)->tp_class_cache, decl, hash, member)
#define DeeType_CacheInstanceAttrib(self, decl, hash, attrib) Dee_membercache_addinstanceattrib(&(self)->tp_class_cache, decl, hash, attrib)
#endif /* !__INTELLISENSE__ */

/* NOTES:
 *  - DeeType_GetCachedAttr         -- `"foo".lower'
 *  - DeeType_GetCachedClassAttr    -- `string.chr', `string.lower'
 *  - DeeType_GetCachedInstanceAttr -- `string.getinstanceattr("lower")'
 */

/* Lookup an attribute from cache.
 * @return: * :        The attribute value.
 * @return: NULL:      An error occurred.
 * @return: ITER_DONE: The attribute could not be found in the cache. */
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_GetCachedAttr)(DeeTypeObject *tp_self, DeeObject *self, char const *__restrict attr, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_GetCachedAttrLen)(DeeTypeObject *tp_self, DeeObject *self, char const *__restrict attr, size_t attrlen, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL DeeType_GetCachedClassAttr)(DeeTypeObject *__restrict tp_self, char const *__restrict attr, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL DeeType_GetCachedClassAttrLen)(DeeTypeObject *__restrict tp_self, char const *__restrict attr, size_t attrlen, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL DeeType_GetCachedInstanceAttr)(DeeTypeObject *__restrict tp_self, char const *__restrict attr, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL DeeType_GetCachedInstanceAttrLen)(DeeTypeObject *__restrict tp_self, char const *__restrict attr, size_t attrlen, dhash_t hash);

/* @return: 1 : Attribute is bound.
 * @return: 0 : Attribute isn't bound.
 * @return: -1: An error occurred.
 * @return: -2: The attribute doesn't exist. */
INTDEF WUNUSED NONNULL((1, 2, 3)) int (DCALL DeeType_BoundCachedAttr)(DeeTypeObject *tp_self, DeeObject *self, char const *__restrict attr, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int (DCALL DeeType_BoundCachedAttrLen)(DeeTypeObject *tp_self, DeeObject *self, char const *__restrict attr, size_t attrlen, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeType_BoundCachedClassAttr)(DeeTypeObject *__restrict tp_self, char const *__restrict attr, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeType_BoundCachedClassAttrLen)(DeeTypeObject *__restrict tp_self, char const *__restrict attr, size_t attrlen, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeType_BoundCachedInstanceAttr)(DeeTypeObject *__restrict tp_self, char const *__restrict attr, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeType_BoundCachedInstanceAttrLen)(DeeTypeObject *__restrict tp_self, char const *__restrict attr, size_t attrlen, dhash_t hash);

/* @return: true : The attribute exists.
 * @return: false: The attribute doesn't exist. */
INTDEF WUNUSED NONNULL((1, 2)) bool (DCALL DeeType_HasCachedAttr)(DeeTypeObject *__restrict tp_self, char const *__restrict attr, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) bool (DCALL DeeType_HasCachedAttrLen)(DeeTypeObject *__restrict tp_self, char const *__restrict attr, size_t attrlen, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) bool (DCALL DeeType_HasCachedClassAttr)(DeeTypeObject *__restrict tp_self, char const *__restrict attr, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) bool (DCALL DeeType_HasCachedClassAttrLen)(DeeTypeObject *__restrict tp_self, char const *__restrict attr, size_t attrlen, dhash_t hash);
#ifdef __INTELLISENSE__
INTDEF WUNUSED NONNULL((1, 2)) bool (DCALL DeeType_HasCachedInstanceAttr)(DeeTypeObject *__restrict tp_self, char const *__restrict attr, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) bool (DCALL DeeType_HasCachedInstanceAttrLen)(DeeTypeObject *__restrict tp_self, char const *__restrict attr, size_t attrlen, dhash_t hash);
#else /* __INTELLISENSE__ */
#define DeeType_HasCachedInstanceAttr(tp_self, attr, hash)             DeeType_HasCachedAttr(tp_self, attr, hash)
#define DeeType_HasCachedInstanceAttrLen(tp_self, attr, attrlen, hash) DeeType_HasCachedAttrLen(tp_self, attr, attrlen, hash)
#endif /* !__INTELLISENSE__ */

/* @return:  1: The attribute could not be found in the cache.
 * @return:  0: Successfully invoked the delete-operator on the attribute.
 * @return: -1: An error occurred. */
INTDEF WUNUSED NONNULL((1, 2, 3)) int (DCALL DeeType_DelCachedAttr)(DeeTypeObject *tp_self, DeeObject *self, char const *__restrict attr, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int (DCALL DeeType_DelCachedAttrLen)(DeeTypeObject *tp_self, DeeObject *self, char const *__restrict attr, size_t attrlen, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeType_DelCachedClassAttr)(DeeTypeObject *__restrict tp_self, char const *__restrict attr, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeType_DelCachedClassAttrLen)(DeeTypeObject *__restrict tp_self, char const *__restrict attr, size_t attrlen, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeType_DelCachedInstanceAttr)(DeeTypeObject *__restrict tp_self, char const *__restrict attr, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeType_DelCachedInstanceAttrLen)(DeeTypeObject *__restrict tp_self, char const *__restrict attr, size_t attrlen, dhash_t hash);

/* @return:  1: The attribute could not be found in the cache.
 * @return:  0: Successfully invoked the set-operator on the attribute.
 * @return: -1: An error occurred. */
INTDEF WUNUSED NONNULL((1, 2, 3, 5)) int (DCALL DeeType_SetCachedAttr)(DeeTypeObject *tp_self, DeeObject *self, char const *__restrict attr, dhash_t hash, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3, 6)) int (DCALL DeeType_SetCachedAttrLen)(DeeTypeObject *tp_self, DeeObject *self, char const *__restrict attr, size_t attrlen, dhash_t hash, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 4)) int (DCALL DeeType_SetCachedClassAttr)(DeeTypeObject *tp_self, char const *__restrict attr, dhash_t hash, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 5)) int (DCALL DeeType_SetCachedClassAttrLen)(DeeTypeObject *tp_self, char const *__restrict attr, size_t attrlen, dhash_t hash, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 4)) int (DCALL DeeType_SetCachedInstanceAttr)(DeeTypeObject *tp_self, char const *__restrict attr, dhash_t hash, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 5)) int (DCALL DeeType_SetCachedInstanceAttrLen)(DeeTypeObject *tp_self, char const *__restrict attr, size_t attrlen, dhash_t hash, DeeObject *value);

/* @return:  1: The attribute could not be found in the cache.
 * @return:  0: Successfully invoked the set-operator on the attribute.
 * @return: -1: An error occurred. )(An error is also thrown for non-basic attributes) */
INTDEF WUNUSED NONNULL((1, 2, 3, 5)) int (DCALL DeeType_SetBasicCachedAttr)(DeeTypeObject *tp_self, DeeObject *self, char const *__restrict attr, dhash_t hash, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3, 6)) int (DCALL DeeType_SetBasicCachedAttrLen)(DeeTypeObject *tp_self, DeeObject *self, char const *__restrict attr, size_t attrlen, dhash_t hash, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 4)) int (DCALL DeeType_SetBasicCachedClassAttr)(DeeTypeObject *tp_self, char const *__restrict attr, dhash_t hash, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 5)) int (DCALL DeeType_SetBasicCachedClassAttrLen)(DeeTypeObject *tp_self, char const *__restrict attr, size_t attrlen, dhash_t hash, DeeObject *value);
//INTDEF WUNUSED NONNULL((1, 2, 4)) int (DCALL DeeType_SetBasicCachedInstanceAttr)(DeeTypeObject *tp_self, char const *__restrict attr, dhash_t hash, DeeObject *value);
//INTDEF WUNUSED NONNULL((1, 2, 5)) int (DCALL DeeType_SetBasicCachedInstanceAttrLen)(DeeTypeObject *tp_self, char const *__restrict attr, size_t attrlen, dhash_t hash, DeeObject *value);

/* @return: * :        The returned value.
 * @return: NULL:      An error occurred.
 * @return: ITER_DONE: The attribute could not be found in the cache. */
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallCachedAttr)(DeeTypeObject *tp_self, DeeObject *self, char const *__restrict attr, dhash_t hash, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallCachedAttrLen)(DeeTypeObject *tp_self, DeeObject *self, char const *__restrict attr, size_t attrlen, dhash_t hash, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL DeeType_CallCachedClassAttr)(DeeTypeObject *tp_self, char const *__restrict attr, dhash_t hash, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL DeeType_CallCachedClassAttrLen)(DeeTypeObject *tp_self, char const *__restrict attr, size_t attrlen, dhash_t hash, size_t argc, DeeObject *const *argv);
#define DeeType_CallCachedInstanceAttr(tp_self, attr, hash, argc, argv) DeeType_CallCachedInstanceAttrKw(tp_self, attr, hash, argc, argv, NULL)
#define DeeType_CallCachedInstanceAttrLen(tp_self, attr, attrlen, hash, argc, argv) DeeType_CallCachedInstanceAttrLenKw(tp_self, attr, attrlen, hash, argc, argv, NULL)
//INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL DeeType_CallCachedInstanceAttr)(DeeTypeObject *tp_self, char const *__restrict attr, dhash_t hash, size_t argc, DeeObject *const *argv);
//INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL DeeType_CallCachedInstanceAttrLen)(DeeTypeObject *tp_self, char const *__restrict attr, size_t attrlen, dhash_t hash, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallCachedAttrKw)(DeeTypeObject *tp_self, DeeObject *self, char const *__restrict attr, dhash_t hash, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallCachedAttrLenKw)(DeeTypeObject *tp_self, DeeObject *self, char const *__restrict attr, size_t attrlen, dhash_t hash, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL DeeType_CallCachedClassAttrKw)(DeeTypeObject *tp_self, char const *__restrict attr, dhash_t hash, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL DeeType_CallCachedClassAttrLenKw)(DeeTypeObject *tp_self, char const *__restrict attr, size_t attrlen, dhash_t hash, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL DeeType_CallCachedInstanceAttrKw)(DeeTypeObject *tp_self, char const *__restrict attr, dhash_t hash, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL DeeType_CallCachedInstanceAttrLenKw)(DeeTypeObject *tp_self, char const *__restrict attr, size_t attrlen, dhash_t hash, size_t argc, DeeObject *const *argv, DeeObject *kw);

#ifdef CONFIG_CALLTUPLE_OPTIMIZATIONS
/* @return: * :        The returned value.
 * @return: NULL:      An error occurred.
 * @return: ITER_DONE: The attribute could not be found in the cache. */
INTDEF WUNUSED NONNULL((1, 2, 3, 5)) DREF DeeObject *(DCALL DeeType_CallCachedAttrTuple)(DeeTypeObject *tp_self, DeeObject *self, char const *__restrict attr, dhash_t hash, DeeObject *args);
INTDEF WUNUSED NONNULL((1, 2, 4)) DREF DeeObject *(DCALL DeeType_CallCachedClassAttrTuple)(DeeTypeObject *tp_self, char const *__restrict attr, dhash_t hash, DeeObject *args);
//INTDEF WUNUSED NONNULL((1, 2, 4)) DREF DeeObject *(DCALL DeeType_CallCachedInstanceAttrTuple)(DeeTypeObject *tp_self, char const *__restrict attr, dhash_t hash, DeeObject *args);
INTDEF WUNUSED NONNULL((1, 2, 3, 5)) DREF DeeObject *(DCALL DeeType_CallCachedAttrTupleKw)(DeeTypeObject *tp_self, DeeObject *self, char const *__restrict attr, dhash_t hash, DeeObject *args, DeeObject *kw);
INTDEF WUNUSED NONNULL((1, 2, 4)) DREF DeeObject *(DCALL DeeType_CallCachedClassAttrTupleKw)(DeeTypeObject *tp_self, char const *__restrict attr, dhash_t hash, DeeObject *args, DeeObject *kw);
//INTDEF WUNUSED NONNULL((1, 2, 4)) DREF DeeObject *(DCALL DeeType_CallCachedInstanceAttrTupleKw)(DeeTypeObject *tp_self, char const *__restrict attr, dhash_t hash, DeeObject *args, DeeObject *kw);
//INTDEF WUNUSED NONNULL((1, 2, 3, 6)) DREF DeeObject *(DCALL DeeType_CallCachedAttrLenTuple)(DeeTypeObject *tp_self, DeeObject *self, char const *__restrict attr, size_t attrlen, dhash_t hash, DeeObject *args);
//INTDEF WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *(DCALL DeeType_CallCachedClassAttrLenTuple)(DeeTypeObject *tp_self, char const *__restrict attr, size_t attrlen, dhash_t hash, DeeObject *args);
//INTDEF WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *(DCALL DeeType_CallCachedInstanceAttrLenTuple)(DeeTypeObject *tp_self, char const *__restrict attr, size_t attrlen, dhash_t hash, DeeObject *args);
//INTDEF WUNUSED NONNULL((1, 2, 3, 6)) DREF DeeObject *(DCALL DeeType_CallCachedAttrLenTupleKw)(DeeTypeObject *tp_self, DeeObject *self, char const *__restrict attr, size_t attrlen, dhash_t hash, DeeObject *args, DeeObject *kw);
//INTDEF WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *(DCALL DeeType_CallCachedClassAttrLenTupleKw)(DeeTypeObject *tp_self, char const *__restrict attr, size_t attrlen, dhash_t hash, DeeObject *args, DeeObject *kw);
//INTDEF WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *(DCALL DeeType_CallCachedInstanceAttrLenTupleKw)(DeeTypeObject *tp_self, char const *__restrict attr, size_t attrlen, dhash_t hash, DeeObject *args, DeeObject *kw);
#endif /* CONFIG_CALLTUPLE_OPTIMIZATIONS */

/* @return: * :        The returned value.
 * @return: NULL:      An error occurred.
 * @return: ITER_DONE: The attribute could not be found in the cache. */
INTDEF WUNUSED NONNULL((1, 2, 3, 5)) DREF DeeObject *(DCALL DeeType_VCallCachedAttrf)(DeeTypeObject *tp_self, DeeObject *self, char const *__restrict attr, dhash_t hash, char const *__restrict format, va_list args);
INTDEF WUNUSED NONNULL((1, 2, 4)) DREF DeeObject *(DCALL DeeType_VCallCachedClassAttrf)(DeeTypeObject *tp_self, char const *__restrict attr, dhash_t hash, char const *__restrict format, va_list args);
//INTDEF WUNUSED DREF DeeObject *(DCALL DeeType_VCallCachedInstanceAttrf)(DeeTypeObject *__restrict tp_self, char const *__restrict attr, dhash_t hash, char const *__restrict format, va_list args);

struct attribute_info;
struct attribute_lookup_rules;

/* @return:  0: Attribute was found.
 * @return:  1: Attribute wasn't found.
 * @return: -1: An error occurred. */
INTDEF WUNUSED NONNULL((1, 3, 4)) int (DCALL DeeType_FindCachedAttr)(DeeTypeObject *tp_self, DeeObject *instance, struct attribute_info *__restrict result, struct attribute_lookup_rules const *__restrict rules);
INTDEF WUNUSED NONNULL((1, 2, 3)) int (DCALL DeeType_FindCachedClassAttr)(DeeTypeObject *tp_self, struct attribute_info *__restrict result, struct attribute_lookup_rules const *__restrict rules);



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
INTDEF WUNUSED NONNULL((1, 2, 3)) struct Dee_class_attribute *(DCALL DeeType_QueryAttribute)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, /*String*/ DeeObject *attr);
INTDEF WUNUSED NONNULL((1, 2, 3)) struct Dee_class_attribute *(DCALL DeeType_QueryAttributeWithHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, /*String*/ DeeObject *attr, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) struct Dee_class_attribute *(DCALL DeeType_QueryAttributeString)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr);
INTDEF WUNUSED NONNULL((1, 2, 3)) struct Dee_class_attribute *(DCALL DeeType_QueryAttributeStringLen)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, size_t namelen);
INTDEF WUNUSED NONNULL((1, 2, 3)) struct Dee_class_attribute *(DCALL DeeType_QueryAttributeStringWithHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) struct Dee_class_attribute *(DCALL DeeType_QueryAttributeStringLenWithHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, size_t namelen, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) struct Dee_class_attribute *(DCALL DeeType_QueryClassAttribute)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, /*String*/ DeeObject *attr);
INTDEF WUNUSED NONNULL((1, 2, 3)) struct Dee_class_attribute *(DCALL DeeType_QueryClassAttributeWithHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, /*String*/ DeeObject *attr, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) struct Dee_class_attribute *(DCALL DeeType_QueryClassAttributeString)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr);
INTDEF WUNUSED NONNULL((1, 2, 3)) struct Dee_class_attribute *(DCALL DeeType_QueryClassAttributeStringLen)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, size_t namelen);
INTDEF WUNUSED NONNULL((1, 2, 3)) struct Dee_class_attribute *(DCALL DeeType_QueryClassAttributeStringWithHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) struct Dee_class_attribute *(DCALL DeeType_QueryClassAttributeStringLenWithHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, size_t namelen, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) struct Dee_class_attribute *(DCALL DeeType_QueryInstanceAttribute)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, /*String*/ DeeObject *attr);
INTDEF WUNUSED NONNULL((1, 2, 3)) struct Dee_class_attribute *(DCALL DeeType_QueryInstanceAttributeWithHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, /*String*/ DeeObject *attr, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) struct Dee_class_attribute *(DCALL DeeType_QueryInstanceAttributeString)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr);
INTDEF WUNUSED NONNULL((1, 2, 3)) struct Dee_class_attribute *(DCALL DeeType_QueryInstanceAttributeStringLen)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, size_t namelen);
INTDEF WUNUSED NONNULL((1, 2, 3)) struct Dee_class_attribute *(DCALL DeeType_QueryInstanceAttributeStringWithHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) struct Dee_class_attribute *(DCALL DeeType_QueryInstanceAttributeStringLenWithHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, size_t namelen, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) struct Dee_class_attribute *(DCALL DeeType_QueryIInstanceAttribute)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, /*String*/ DeeObject *attr);
INTDEF WUNUSED NONNULL((1, 2, 3)) struct Dee_class_attribute *(DCALL DeeType_QueryIInstanceAttributeWithHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, /*String*/ DeeObject *attr, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) struct Dee_class_attribute *(DCALL DeeType_QueryIInstanceAttributeString)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr);
INTDEF WUNUSED NONNULL((1, 2, 3)) struct Dee_class_attribute *(DCALL DeeType_QueryIInstanceAttributeStringLen)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, size_t namelen);
INTDEF WUNUSED NONNULL((1, 2, 3)) struct Dee_class_attribute *(DCALL DeeType_QueryIInstanceAttributeStringWithHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) struct Dee_class_attribute *(DCALL DeeType_QueryIInstanceAttributeStringLenWithHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, size_t namelen, dhash_t hash);
#else /* __INTELLISENSE__ */
INTDEF WUNUSED NONNULL((1, 2, 3)) struct Dee_class_attribute *(DCALL DeeType_QueryAttributeWithHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, /*String*/ DeeObject *attr, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) struct Dee_class_attribute *(DCALL DeeType_QueryAttributeStringWithHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) struct Dee_class_attribute *(DCALL DeeType_QueryAttributeStringLenWithHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, size_t namelen, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) struct Dee_class_attribute *(DCALL DeeType_QueryClassAttributeWithHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, /*String*/ DeeObject *attr, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) struct Dee_class_attribute *(DCALL DeeType_QueryClassAttributeStringWithHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) struct Dee_class_attribute *(DCALL DeeType_QueryClassAttributeStringLenWithHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, size_t namelen, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) struct Dee_class_attribute *(DCALL DeeType_QueryInstanceAttributeWithHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, /*String*/ DeeObject *attr, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) struct Dee_class_attribute *(DCALL DeeType_QueryInstanceAttributeStringWithHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) struct Dee_class_attribute *(DCALL DeeType_QueryInstanceAttributeStringLenWithHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, size_t namelen, dhash_t hash);
#define DeeType_QueryIInstanceAttributeWithHash(tp_invoker, tp_self, attr, hash)                   DeeType_QueryInstanceAttributeWithHash(tp_invoker, tp_self, attr, hash)
#define DeeType_QueryIInstanceAttributeStringWithHash(tp_invoker, tp_self, attr, hash)             DeeType_QueryInstanceAttributeStringWithHash(tp_invoker, tp_self, attr, hash)
#define DeeType_QueryIInstanceAttributeStringLenWithHash(tp_invoker, tp_self, attr, namelen, hash) DeeType_QueryInstanceAttributeStringLenWithHash(tp_invoker, tp_self, attr, namelen, hash)
#define DeeType_QueryAttribute(tp_invoker, tp_self, attr)                                          DeeType_QueryAttributeWithHash(tp_invoker, tp_self, attr, DeeString_Hash(attr))
#define DeeType_QueryAttributeString(tp_invoker, tp_self, attr)                                    DeeType_QueryAttributeStringWithHash(tp_invoker, tp_self, attr, Dee_HashStr(attr))
#define DeeType_QueryAttributeStringLen(tp_invoker, tp_self, attr, namelen)                        DeeType_QueryAttributeStringLenWithHash(tp_invoker, tp_self, attr, namelen, Dee_HashPtr(attr, namelen))
#define DeeType_QueryClassAttribute(tp_invoker, tp_self, attr)                                     DeeType_QueryClassAttributeWithHash(tp_invoker, tp_self, attr, DeeString_Hash(attr))
#define DeeType_QueryClassAttributeString(tp_invoker, tp_self, attr)                               DeeType_QueryClassAttributeStringWithHash(tp_invoker, tp_self, attr, Dee_HashStr(attr))
#define DeeType_QueryClassAttributeStringLen(tp_invoker, tp_self, attr, namelen)                   DeeType_QueryClassAttributeStringLenWithHash(tp_invoker, tp_self, attr, namelen, Dee_HashPtr(attr, namelen))
#define DeeType_QueryInstanceAttribute(tp_invoker, tp_self, attr)                                  DeeType_QueryInstanceAttributeWithHash(tp_invoker, tp_self, attr, DeeString_Hash(attr))
#define DeeType_QueryInstanceAttributeString(tp_invoker, tp_self, attr)                            DeeType_QueryInstanceAttributeStringWithHash(tp_invoker, tp_self, attr, Dee_HashStr(attr))
#define DeeType_QueryInstanceAttributeStringLen(tp_invoker, tp_self, attr, namelen)                DeeType_QueryInstanceAttributeStringLenWithHash(tp_invoker, tp_self, attr, namelen, Dee_HashPtr(attr, namelen))
#define DeeType_QueryIInstanceAttribute(tp_invoker, tp_self, attr)                                 DeeType_QueryIInstanceAttributeWithHash(tp_invoker, tp_self, attr, DeeString_Hash(attr))
#define DeeType_QueryIInstanceAttributeString(tp_invoker, tp_self, attr)                           DeeType_QueryIInstanceAttributeStringWithHash(tp_invoker, tp_self, attr, Dee_HashStr(attr))
#define DeeType_QueryIInstanceAttributeStringLen(tp_invoker, tp_self, attr, namelen)               DeeType_QueryIInstanceAttributeStringLenWithHash(tp_invoker, tp_self, attr, namelen, Dee_HashPtr(attr, namelen))
#endif /* !__INTELLISENSE__ */

/* Invoke attributes from `tp_self->tp_methods' / `tp_self->tp_class_methods'.
 * @return: * :        The attribute value.
 * @return: NULL:      An error occurred.
 * @return: ITER_DONE: The attribute could not be found in `chain'. */
#ifdef __INTELLISENSE__
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) DREF DeeObject *(DCALL DeeType_GetMethodAttr)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, DeeObject *self, char const *__restrict attr_name, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_GetClassMethodAttr)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_GetInstanceMethodAttr)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_GetIInstanceMethodAttr)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) DREF DeeObject *(DCALL DeeType_CallMethodAttr)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, DeeObject *self, char const *__restrict attr_name, dhash_t hash, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallClassMethodAttr)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, dhash_t hash, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallInstanceMethodAttr)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, dhash_t hash, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallIInstanceMethodAttr)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, dhash_t hash, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) DREF DeeObject *(DCALL DeeType_CallMethodAttrKw)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, DeeObject *self, char const *__restrict attr_name, dhash_t hash, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallClassMethodAttrKw)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, dhash_t hash, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallInstanceMethodAttrKw)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, dhash_t hash, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallIInstanceMethodAttrKw)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, dhash_t hash, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) DREF DeeObject *(DCALL DeeType_GetMethodAttrLen)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, DeeObject *self, char const *__restrict attr_name, size_t attrlen, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_GetClassMethodAttrLen)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, size_t attrlen, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_GetInstanceMethodAttrLen)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, size_t attrlen, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_GetIInstanceMethodAttrLen)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, size_t attrlen, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) DREF DeeObject *(DCALL DeeType_CallMethodAttrLen)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, DeeObject *self, char const *__restrict attr_name, size_t attrlen, dhash_t hash, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallClassMethodAttrLen)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, size_t attrlen, dhash_t hash, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallInstanceMethodAttrLen)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, size_t attrlen, dhash_t hash, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallIInstanceMethodAttrLen)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, size_t attrlen, dhash_t hash, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) DREF DeeObject *(DCALL DeeType_CallMethodAttrLenKw)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, DeeObject *self, char const *__restrict attr_name, size_t attrlen, dhash_t hash, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallClassMethodAttrLenKw)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, size_t attrlen, dhash_t hash, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallInstanceMethodAttrLenKw)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, size_t attrlen, dhash_t hash, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallIInstanceMethodAttrLenKw)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, size_t attrlen, dhash_t hash, size_t argc, DeeObject *const *argv, DeeObject *kw);
#ifdef CONFIG_CALLTUPLE_OPTIMIZATIONS
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) DREF DeeObject *(DCALL DeeType_CallMethodAttrTuple)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, DeeObject *self, char const *__restrict attr_name, dhash_t hash, DeeObject *args);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallClassMethodAttrTuple)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, dhash_t hash, DeeObject *args);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallInstanceMethodAttrTuple)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, dhash_t hash, DeeObject *args);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallIInstanceMethodAttrTuple)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, dhash_t hash, DeeObject *args);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) DREF DeeObject *(DCALL DeeType_CallMethodAttrTupleKw)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, DeeObject *self, char const *__restrict attr_name, dhash_t hash, DeeObject *args, DeeObject *kw);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallClassMethodAttrTupleKw)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, dhash_t hash, DeeObject *args, DeeObject *kw);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallInstanceMethodAttrTupleKw)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, dhash_t hash, DeeObject *args, DeeObject *kw);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallIInstanceMethodAttrTupleKw)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, dhash_t hash, DeeObject *args, DeeObject *kw);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) DREF DeeObject *(DCALL DeeType_CallMethodAttrLenTuple)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, DeeObject *self, char const *__restrict attr_name, size_t attrlen, dhash_t hash, DeeObject *args);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallClassMethodAttrLenTuple)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, size_t attrlen, dhash_t hash, DeeObject *args);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallInstanceMethodAttrLenTuple)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, size_t attrlen, dhash_t hash, DeeObject *args);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallIInstanceMethodAttrLenTuple)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, size_t attrlen, dhash_t hash, DeeObject *args);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) DREF DeeObject *(DCALL DeeType_CallMethodAttrLenTupleKw)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, DeeObject *self, char const *__restrict attr_name, size_t attrlen, dhash_t hash, DeeObject *args, DeeObject *kw);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallClassMethodAttrLenTupleKw)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, size_t attrlen, dhash_t hash, DeeObject *args, DeeObject *kw);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallInstanceMethodAttrLenTupleKw)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, size_t attrlen, dhash_t hash, DeeObject *args, DeeObject *kw);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallIInstanceMethodAttrLenTupleKw)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, size_t attrlen, dhash_t hash, DeeObject *args, DeeObject *kw);
#endif /* CONFIG_CALLTUPLE_OPTIMIZATIONS */
INTDEF WUNUSED NONNULL((1, 2, 3, 4, 6)) DREF DeeObject *(DCALL DeeType_VCallMethodAttrf)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, DeeObject *self, char const *__restrict attr_name, dhash_t hash, char const *__restrict format, va_list args);
INTDEF WUNUSED NONNULL((1, 2, 3, 5)) DREF DeeObject *(DCALL DeeType_VCallClassMethodAttrf)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, dhash_t hash, char const *__restrict format, va_list args);
//INTDEF WUNUSED NONNULL((1, 2, 3, 5)) DREF DeeObject *(DCALL DeeType_VCallInstanceMethodAttrf)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, dhash_t hash, char const *__restrict format, va_list args);
//INTDEF WUNUSED NONNULL((1, 2, 3, 5)) DREF DeeObject *(DCALL DeeType_VCallIInstanceMethodAttrf)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, dhash_t hash, char const *__restrict format, va_list args);
#else /* __INTELLISENSE__ */
INTDEF NONNULL((1, 2, 3, 4, 5)) DREF DeeObject *DCALL /* GET_METHOD */
type_method_getattr(struct Dee_membercache *cache, DeeTypeObject *decl,
                    struct type_method const *chain, DeeObject *self,
                    char const *__restrict attr_name, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3, 4, 5)) DREF DeeObject *DCALL /* GET_METHOD */
type_method_getattr_len(struct Dee_membercache *cache, DeeTypeObject *decl,
                        struct type_method const *chain, DeeObject *self,
                        char const *__restrict attr_name, size_t attrlen, dhash_t hash);
#define DeeType_GetMethodAttr(tp_invoker, tp_self, self, attr_name, hash)             type_method_getattr(&(tp_invoker)->tp_cache, tp_self, (tp_self)->tp_methods, self, attr_name, hash)
#define DeeType_GetClassMethodAttr(tp_invoker, tp_self, attr_name, hash)              type_method_getattr(&(tp_invoker)->tp_class_cache, tp_self, (tp_self)->tp_class_methods, (DeeObject *)(tp_invoker), attr_name, hash)
#define DeeType_GetMethodAttrLen(tp_invoker, tp_self, self, attr_name, attrlen, hash) type_method_getattr_len(&(tp_invoker)->tp_cache, tp_self, (tp_self)->tp_methods, self, attr_name, attrlen, hash)
#define DeeType_GetClassMethodAttrLen(tp_invoker, tp_self, attr_name, attrlen, hash)  type_method_getattr_len(&(tp_invoker)->tp_class_cache, tp_self, (tp_self)->tp_class_methods, (DeeObject *)(tp_invoker), attr_name, attrlen, hash)
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_GetInstanceMethodAttr)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_GetIInstanceMethodAttr)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_GetInstanceMethodAttrLen)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, size_t attrlen, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_GetIInstanceMethodAttrLen)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, size_t attrlen, dhash_t hash);

INTDEF WUNUSED NONNULL((1, 2, 3, 4, 5)) DREF DeeObject *DCALL /* CALL_METHOD */
type_method_callattr(struct Dee_membercache *cache, DeeTypeObject *decl,
                     struct type_method const *chain, DeeObject *self,
                     char const *__restrict attr_name, dhash_t hash,
                     size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1, 2, 3, 4, 5)) DREF DeeObject *DCALL /* CALL_METHOD */
type_method_callattr_len(struct Dee_membercache *cache, DeeTypeObject *decl,
                         struct type_method const *chain, DeeObject *self,
                         char const *__restrict attr_name, size_t attrlen, dhash_t hash,
                         size_t argc, DeeObject *const *argv);
#define DeeType_CallMethodAttr(tp_invoker, tp_self, self, attr_name, hash, argc, argv)             type_method_callattr(&(tp_invoker)->tp_cache, tp_self, (tp_self)->tp_methods, self, attr_name, hash, argc, argv)
#define DeeType_CallClassMethodAttr(tp_invoker, tp_self, attr_name, hash, argc, argv)              type_method_callattr(&(tp_invoker)->tp_class_cache, tp_self, (tp_self)->tp_class_methods, (DeeObject *)(tp_invoker), attr_name, hash, argc, argv)
#define DeeType_CallMethodAttrLen(tp_invoker, tp_self, self, attr_name, attrlen, hash, argc, argv) type_method_callattr_len(&(tp_invoker)->tp_cache, tp_self, (tp_self)->tp_methods, self, attr_name, attrlen, hash, argc, argv)
#define DeeType_CallClassMethodAttrLen(tp_invoker, tp_self, attr_name, attrlen, hash, argc, argv)  type_method_callattr_len(&(tp_invoker)->tp_class_cache, tp_self, (tp_self)->tp_class_methods, (DeeObject *)(tp_invoker), attr_name, attrlen, hash, argc, argv)
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallInstanceMethodAttr)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, dhash_t hash, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallInstanceMethodAttrLen)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, size_t attrlen, dhash_t hash, size_t argc, DeeObject *const *argv);
#define DeeType_CallIInstanceMethodAttr(tp_invoker, tp_self, attr_name, hash, argc, argv) \
	DeeType_CallIInstanceMethodAttrKw(tp_invoker, tp_self, attr_name, hash, argc, argv, NULL)
#define DeeType_CallIInstanceMethodAttrLen(tp_invoker, tp_self, attr_name, attrlen, hash, argc, argv) \
	DeeType_CallIInstanceMethodAttrLenKw(tp_invoker, tp_self, attr_name, attrlen, hash, argc, argv, NULL)

INTDEF WUNUSED NONNULL((1, 2, 3, 4, 5)) DREF DeeObject *DCALL /* CALL_METHOD_KW */
type_method_callattr_kw(struct Dee_membercache *cache, DeeTypeObject *decl,
                        struct type_method const *chain, DeeObject *self,
                        char const *__restrict attr_name, dhash_t hash,
                        size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1, 2, 3, 4, 5)) DREF DeeObject *DCALL /* CALL_METHOD_KW */
type_method_callattr_len_kw(struct Dee_membercache *cache, DeeTypeObject *decl,
                            struct type_method const *chain, DeeObject *self,
                            char const *__restrict attr_name, size_t attrlen, dhash_t hash,
                            size_t argc, DeeObject *const *argv, DeeObject *kw);
#define DeeType_CallMethodAttrKw(tp_invoker, tp_self, self, attr_name, hash, argc, argv, kw)             type_method_callattr_kw(&(tp_invoker)->tp_cache, tp_self, (tp_self)->tp_methods, self, attr_name, hash, argc, argv, kw)
#define DeeType_CallClassMethodAttrKw(tp_invoker, tp_self, attr_name, hash, argc, argv, kw)              type_method_callattr_kw(&(tp_invoker)->tp_class_cache, tp_self, (tp_self)->tp_class_methods, (DeeObject *)(tp_invoker), attr_name, hash, argc, argv, kw)
#define DeeType_CallMethodAttrLenKw(tp_invoker, tp_self, self, attr_name, attrlen, hash, argc, argv, kw) type_method_callattr_len_kw(&(tp_invoker)->tp_cache, tp_self, (tp_self)->tp_methods, self, attr_name, attrlen, hash, argc, argv, kw)
#define DeeType_CallClassMethodAttrLenKw(tp_invoker, tp_self, attr_name, attrlen, hash, argc, argv, kw)  type_method_callattr_len_kw(&(tp_invoker)->tp_class_cache, tp_self, (tp_self)->tp_class_methods, (DeeObject *)(tp_invoker), attr_name, attrlen, hash, argc, argv, kw)
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallInstanceMethodAttrKw)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, dhash_t hash, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallInstanceMethodAttrLenKw)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, size_t attrlen, dhash_t hash, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallIInstanceMethodAttrKw)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, dhash_t hash, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallIInstanceMethodAttrLenKw)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, size_t attrlen, dhash_t hash, size_t argc, DeeObject *const *argv, DeeObject *kw);

#ifdef CONFIG_CALLTUPLE_OPTIMIZATIONS
/* CALL_METHOD_TUPLE */
#define DeeType_CallMethodAttrTuple(tp_invoker, tp_self, self, attr_name, hash, args)                      DeeType_CallMethodAttr(tp_invoker, tp_self, self, attr_name, hash, DeeTuple_SIZE(args), DeeTuple_ELEM(args))
#define DeeType_CallClassMethodAttrTuple(tp_invoker, tp_self, attr_name, hash, args)                       DeeType_CallClassMethodAttr(tp_invoker, tp_self, attr_name, hash, DeeTuple_SIZE(args), DeeTuple_ELEM(args))
#define DeeType_CallInstanceMethodAttrTuple(tp_invoker, tp_self, attr_name, hash, args)                    DeeType_CallInstanceMethodAttr(tp_invoker, tp_self, attr_name, hash, DeeTuple_SIZE(args), DeeTuple_ELEM(args))
#define DeeType_CallIInstanceMethodAttrTuple(tp_invoker, tp_self, attr_name, hash, args)                   DeeType_CallIInstanceMethodAttr(tp_invoker, tp_self, attr_name, hash, DeeTuple_SIZE(args), DeeTuple_ELEM(args))
#define DeeType_CallMethodAttrTupleKw(tp_invoker, tp_self, self, attr_name, hash, args, kw)                DeeType_CallMethodAttrKw(tp_invoker, tp_self, self, attr_name, hash, DeeTuple_SIZE(args), DeeTuple_ELEM(args), kw)
#define DeeType_CallClassMethodAttrTupleKw(tp_invoker, tp_self, attr_name, hash, args, kw)                 DeeType_CallClassMethodAttrKw(tp_invoker, tp_self, attr_name, hash, DeeTuple_SIZE(args), DeeTuple_ELEM(args), kw)
#define DeeType_CallInstanceMethodAttrTupleKw(tp_invoker, tp_self, attr_name, hash, args, kw)              DeeType_CallInstanceMethodAttrKw(tp_invoker, tp_self, attr_name, hash, DeeTuple_SIZE(args), DeeTuple_ELEM(args), kw)
#define DeeType_CallIInstanceMethodAttrTupleKw(tp_invoker, tp_self, attr_name, hash, args, kw)             DeeType_CallIInstanceMethodAttrKw(tp_invoker, tp_self, attr_name, hash, DeeTuple_SIZE(args), DeeTuple_ELEM(args), kw)
#define DeeType_CallMethodAttrLenTuple(tp_invoker, tp_self, self, attr_name, attrlen, hash, args)          DeeType_CallMethodAttrLen(tp_invoker, tp_self, self, attr_name, attrlen, hash, DeeTuple_SIZE(args), DeeTuple_ELEM(args))
#define DeeType_CallClassMethodAttrLenTuple(tp_invoker, tp_self, attr_name, attrlen, hash, args)           DeeType_CallClassMethodAttrLen(tp_invoker, tp_self, attr_name, attrlen, hash, DeeTuple_SIZE(args), DeeTuple_ELEM(args))
#define DeeType_CallInstanceMethodAttrLenTuple(tp_invoker, tp_self, attr_name, attrlen, hash, args)        DeeType_CallInstanceMethodAttrLen(tp_invoker, tp_self, attr_name, attrlen, hash, DeeTuple_SIZE(args), DeeTuple_ELEM(args))
#define DeeType_CallIInstanceMethodAttrLenTuple(tp_invoker, tp_self, attr_name, attrlen, hash, args)       DeeType_CallIInstanceMethodAttrLen(tp_invoker, tp_self, attr_name, attrlen, hash, DeeTuple_SIZE(args), DeeTuple_ELEM(args))
#define DeeType_CallMethodAttrLenTupleKw(tp_invoker, tp_self, self, attr_name, attrlen, hash, args, kw)    DeeType_CallMethodAttrLenKw(tp_invoker, tp_self, self, attr_name, attrlen, hash, DeeTuple_SIZE(args), DeeTuple_ELEM(args), kw)
#define DeeType_CallClassMethodAttrLenTupleKw(tp_invoker, tp_self, attr_name, attrlen, hash, args, kw)     DeeType_CallClassMethodAttrLenKw(tp_invoker, tp_self, attr_name, attrlen, hash, DeeTuple_SIZE(args), DeeTuple_ELEM(args), kw)
#define DeeType_CallInstanceMethodAttrLenTupleKw(tp_invoker, tp_self, attr_name, attrlen, hash, args, kw)  DeeType_CallInstanceMethodAttrLenKw(tp_invoker, tp_self, attr_name, attrlen, hash, DeeTuple_SIZE(args), DeeTuple_ELEM(args), kw)
#define DeeType_CallIInstanceMethodAttrLenTupleKw(tp_invoker, tp_self, attr_name, attrlen, hash, args, kw) DeeType_CallIInstanceMethodAttrLenKw(tp_invoker, tp_self, attr_name, attrlen, hash, DeeTuple_SIZE(args), DeeTuple_ELEM(args), kw)
#endif /* CONFIG_CALLTUPLE_OPTIMIZATIONS */

INTDEF WUNUSED NONNULL((1, 2, 3, 4, 5, 7)) DREF DeeObject *DCALL /* CALL_METHOD */
type_method_vcallattrf(struct Dee_membercache *cache, DeeTypeObject *decl,
                       struct type_method const *chain, DeeObject *self,
                       char const *__restrict attr_name, dhash_t hash,
                       char const *__restrict format, va_list args);
#define DeeType_VCallMethodAttrf(tp_invoker, tp_self, self, attr_name, hash, format, args) \
	type_method_vcallattrf(&(tp_invoker)->tp_cache, tp_self, (tp_self)->tp_methods, self, attr_name, hash, format, args)
#define DeeType_VCallClassMethodAttrf(tp_invoker, tp_self, attr_name, hash, format, args) \
	type_method_vcallattrf(&(tp_invoker)->tp_class_cache, tp_self, (tp_self)->tp_class_methods, (DeeObject *)(tp_invoker), attr_name, hash, format, args)
//INTDEF WUNUSED NONNULL((1, 2, 3, 5)) DREF DeeObject *(DCALL DeeType_VCallInstanceMethodAttrf)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, dhash_t hash, char const *__restrict format, va_list args);
//INTDEF WUNUSED NONNULL((1, 2, 3, 5)) DREF DeeObject *(DCALL DeeType_VCallIInstanceMethodAttrf)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, dhash_t hash, char const *__restrict format, va_list args);
#endif /* !__INTELLISENSE__ */

/* Access attributes from `tp_self->tp_getsets' / `tp_self->tp_class_getsets'.
 * @return: -         /  - /  1: - / - / The attribute is bound.
 * @return: *         /  0 /  0: The attribute value / operation was successful / The attribute is unbound.
 * @return: NULL      / -1 / -1: An error occurred.
 * @return: ITER_DONE /  1 / -2: The attribute could not be found in `chain'. */
#ifdef __INTELLISENSE__
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) DREF DeeObject *(DCALL DeeType_GetGetSetAttr)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, DeeObject *self, char const *__restrict attr_name, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_GetClassGetSetAttr)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_GetInstanceGetSetAttr)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_GetIInstanceGetSetAttr)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallInstanceGetSetAttr)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, dhash_t hash, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallIInstanceGetSetAttr)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, dhash_t hash, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallInstanceGetSetAttrKw)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, dhash_t hash, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallIInstanceGetSetAttrKw)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, dhash_t hash, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) DREF DeeObject *(DCALL DeeType_GetGetSetAttrLen)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, DeeObject *self, char const *__restrict attr_name, size_t attrlen, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_GetClassGetSetAttrLen)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, size_t attrlen, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_GetInstanceGetSetAttrLen)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, size_t attrlen, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_GetIInstanceGetSetAttrLen)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, size_t attrlen, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallInstanceGetSetAttrLen)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, size_t attrlen, dhash_t hash, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallIInstanceGetSetAttrLen)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, size_t attrlen, dhash_t hash, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallInstanceGetSetAttrLenKw)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, size_t attrlen, dhash_t hash, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallIInstanceGetSetAttrLenKw)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, size_t attrlen, dhash_t hash, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int (DCALL DeeType_BoundGetSetAttr)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, DeeObject *self, char const *__restrict attr_name, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int (DCALL DeeType_BoundClassGetSetAttr)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int (DCALL DeeType_DelGetSetAttr)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, DeeObject *self, char const *__restrict attr_name, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int (DCALL DeeType_DelClassGetSetAttr)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3, 4, 6)) int (DCALL DeeType_SetGetSetAttr)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, DeeObject *self, char const *__restrict attr_name, dhash_t hash, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3, 5)) int (DCALL DeeType_SetClassGetSetAttr)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, dhash_t hash, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int (DCALL DeeType_BoundGetSetAttrLen)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, DeeObject *self, char const *__restrict attr_name, size_t attrlen, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int (DCALL DeeType_BoundClassGetSetAttrLen)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, size_t attrlen, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int (DCALL DeeType_DelGetSetAttrLen)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, DeeObject *self, char const *__restrict attr_name, size_t attrlen, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int (DCALL DeeType_DelClassGetSetAttrLen)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, size_t attrlen, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3, 4, 7)) int (DCALL DeeType_SetGetSetAttrLen)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, DeeObject *self, char const *__restrict attr_name, size_t attrlen, dhash_t hash, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3, 6)) int (DCALL DeeType_SetClassGetSetAttrLen)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, size_t attrlen, dhash_t hash, DeeObject *value);
#else /* __INTELLISENSE__ */
INTDEF WUNUSED NONNULL((1, 2, 3, 4, 5)) DREF DeeObject *DCALL /* GET_GETSET */
type_getset_getattr(struct Dee_membercache *cache, DeeTypeObject *decl,
                    struct type_getset const *chain, DeeObject *self,
                    char const *__restrict attr_name, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3, 4, 5)) DREF DeeObject *DCALL /* GET_GETSET */
type_getset_getattr_len(struct Dee_membercache *cache, DeeTypeObject *decl,
                        struct type_getset const *chain, DeeObject *self,
                        char const *__restrict attr_name, size_t attrlen, dhash_t hash);
#define DeeType_GetGetSetAttr(tp_invoker, tp_self, self, attr_name, hash)             type_getset_getattr(&(tp_invoker)->tp_cache, tp_self, (tp_self)->tp_getsets, self, attr_name, hash)
#define DeeType_GetClassGetSetAttr(tp_invoker, tp_self, attr_name, hash)              type_getset_getattr(&(tp_invoker)->tp_class_cache, tp_self, (tp_self)->tp_class_getsets, (DeeObject *)(tp_invoker), attr_name, hash)
#define DeeType_GetGetSetAttrLen(tp_invoker, tp_self, self, attr_name, attrlen, hash) type_getset_getattr_len(&(tp_invoker)->tp_cache, tp_self, (tp_self)->tp_getsets, self, attr_name, attrlen, hash)
#define DeeType_GetClassGetSetAttrLen(tp_invoker, tp_self, attr_name, attrlen, hash)  type_getset_getattr_len(&(tp_invoker)->tp_class_cache, tp_self, (tp_self)->tp_class_getsets, (DeeObject *)(tp_invoker), attr_name, attrlen, hash)
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_GetInstanceGetSetAttr)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_GetIInstanceGetSetAttr)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallInstanceGetSetAttr)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, dhash_t hash, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_GetInstanceGetSetAttrLen)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, size_t attrlen, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_GetIInstanceGetSetAttrLen)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, size_t attrlen, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallInstanceGetSetAttrLen)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, size_t attrlen, dhash_t hash, size_t argc, DeeObject *const *argv);
#define DeeType_CallIInstanceGetSetAttr(tp_invoker, tp_self, attr_name, hash, argc, argv) \
	DeeType_CallIInstanceGetSetAttrKw(tp_invoker, tp_self, attr_name, hash, argc, argv, NULL)
#define DeeType_CallIInstanceGetSetAttrLen(tp_invoker, tp_self, attr_name, attrlen, hash, argc, argv) \
	DeeType_CallIInstanceGetSetAttrLenKw(tp_invoker, tp_self, attr_name, attrlen, hash, argc, argv, NULL)
//INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallIInstanceGetSetAttr)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, dhash_t hash, size_t argc, DeeObject *const *argv);
//INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallIInstanceGetSetAttrLen)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, size_t attrlen, dhash_t hash, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallInstanceGetSetAttrKw)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, dhash_t hash, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallInstanceGetSetAttrLenKw)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, size_t attrlen, dhash_t hash, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallIInstanceGetSetAttrKw)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, dhash_t hash, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallIInstanceGetSetAttrLenKw)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, size_t attrlen, dhash_t hash, size_t argc, DeeObject *const *argv, DeeObject *kw);

INTDEF WUNUSED NONNULL((1, 2, 3, 4, 5)) int DCALL /* BOUND_GETSET */
type_getset_boundattr(struct Dee_membercache *cache, DeeTypeObject *decl,
                      struct type_getset const *chain, DeeObject *self,
                      char const *__restrict attr_name, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3, 4, 5)) int DCALL /* BOUND_GETSET */
type_getset_boundattr_len(struct Dee_membercache *cache, DeeTypeObject *decl,
                          struct type_getset const *chain, DeeObject *self,
                          char const *__restrict attr_name, size_t attrlen, dhash_t hash);
#define DeeType_BoundGetSetAttr(tp_invoker, tp_self, self, attr_name, hash)             type_getset_boundattr(&(tp_invoker)->tp_cache, tp_self, (tp_self)->tp_getsets, self, attr_name, hash)
#define DeeType_BoundClassGetSetAttr(tp_invoker, tp_self, attr_name, hash)              type_getset_boundattr(&(tp_invoker)->tp_class_cache, tp_self, (tp_self)->tp_class_getsets, (DeeObject *)(tp_invoker), attr_name, hash)
#define DeeType_BoundGetSetAttrLen(tp_invoker, tp_self, self, attr_name, attrlen, hash) type_getset_boundattr_len(&(tp_invoker)->tp_cache, tp_self, (tp_self)->tp_getsets, self, attr_name, attrlen, hash)
#define DeeType_BoundClassGetSetAttrLen(tp_invoker, tp_self, attr_name, attrlen, hash)  type_getset_boundattr_len(&(tp_invoker)->tp_class_cache, tp_self, (tp_self)->tp_class_getsets, (DeeObject *)(tp_invoker), attr_name, attrlen, hash)

INTDEF WUNUSED NONNULL((1, 2, 3, 4, 5)) int DCALL /* DEL_GETSET */
type_getset_delattr(struct Dee_membercache *cache, DeeTypeObject *decl,
                    struct type_getset const *chain, DeeObject *self,
                    char const *__restrict attr_name, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3, 4, 5)) int DCALL /* DEL_GETSET */
type_getset_delattr_len(struct Dee_membercache *cache, DeeTypeObject *decl,
                        struct type_getset const *chain, DeeObject *self,
                        char const *__restrict attr_name, size_t attrlen, dhash_t hash);
#define DeeType_DelGetSetAttr(tp_invoker, tp_self, self, attr_name, hash)             type_getset_delattr(&(tp_invoker)->tp_cache, tp_self, (tp_self)->tp_getsets, self, attr_name, hash)
#define DeeType_DelClassGetSetAttr(tp_invoker, tp_self, attr_name, hash)              type_getset_delattr(&(tp_invoker)->tp_class_cache, tp_self, (tp_self)->tp_class_getsets, (DeeObject *)(tp_invoker), attr_name, hash)
#define DeeType_DelGetSetAttrLen(tp_invoker, tp_self, self, attr_name, attrlen, hash) type_getset_delattr_len(&(tp_invoker)->tp_cache, tp_self, (tp_self)->tp_getsets, self, attr_name, attrlen, hash)
#define DeeType_DelClassGetSetAttrLen(tp_invoker, tp_self, attr_name, attrlen, hash)  type_getset_delattr_len(&(tp_invoker)->tp_class_cache, tp_self, (tp_self)->tp_class_getsets, (DeeObject *)(tp_invoker), attr_name, attrlen, hash)

INTDEF WUNUSED NONNULL((1, 2, 3, 4, 5, 7)) int DCALL /* SET_GETSET */
type_getset_setattr(struct Dee_membercache *cache, DeeTypeObject *decl,
                    struct type_getset const *chain, DeeObject *self,
                    char const *__restrict attr_name, dhash_t hash, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3, 4, 5, 8)) int DCALL /* SET_GETSET */
type_getset_setattr_len(struct Dee_membercache *cache, DeeTypeObject *decl,
                        struct type_getset const *chain, DeeObject *self,
                        char const *__restrict attr_name, size_t attrlen,
                        dhash_t hash, DeeObject *value);
#define DeeType_SetGetSetAttr(tp_invoker, tp_self, self, attr_name, hash, value)             type_getset_setattr(&(tp_invoker)->tp_cache, tp_self, (tp_self)->tp_getsets, self, attr_name, hash, value)
#define DeeType_SetClassGetSetAttr(tp_invoker, tp_self, attr_name, hash, value)              type_getset_setattr(&(tp_invoker)->tp_class_cache, tp_self, (tp_self)->tp_class_getsets, (DeeObject *)(tp_invoker), attr_name, hash, value)
#define DeeType_SetGetSetAttrLen(tp_invoker, tp_self, self, attr_name, attrlen, hash, value) type_getset_setattr_len(&(tp_invoker)->tp_cache, tp_self, (tp_self)->tp_getsets, self, attr_name, attrlen, hash, value)
#define DeeType_SetClassGetSetAttrLen(tp_invoker, tp_self, attr_name, attrlen, hash, value)  type_getset_setattr_len(&(tp_invoker)->tp_class_cache, tp_self, (tp_self)->tp_class_getsets, (DeeObject *)(tp_invoker), attr_name, attrlen, hash, value)
#endif /* !__INTELLISENSE__ */

/* Access attributes from `tp_self->tp_members' / `tp_self->tp_class_members'.
 * @return: -         /  - /  1: - / - / The attribute is bound.
 * @return: *         /  0 /  0: The attribute value / operation was successful / The attribute is unbound.
 * @return: NULL      / -1 / -1: An error occurred.
 * @return: ITER_DONE /  1 / -2: The attribute could not be found in `chain'. */
#ifdef __INTELLISENSE__
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) DREF DeeObject *(DCALL DeeType_GetMemberAttr)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, DeeObject *self, char const *__restrict attr_name, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_GetClassMemberAttr)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_GetInstanceMemberAttr)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_GetIInstanceMemberAttr)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallInstanceMemberAttr)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, dhash_t hash, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallIInstanceMemberAttr)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, dhash_t hash, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallInstanceMemberAttrKw)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, dhash_t hash, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallIInstanceMemberAttrKw)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, dhash_t hash, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) DREF DeeObject *(DCALL DeeType_GetMemberAttrLen)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, DeeObject *self, char const *__restrict attr_name, size_t attrlen, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_GetClassMemberAttrLen)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, size_t attrlen, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_GetInstanceMemberAttrLen)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, size_t attrlen, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_GetIInstanceMemberAttrLen)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, size_t attrlen, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallInstanceMemberAttrLen)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, size_t attrlen, dhash_t hash, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallIInstanceMemberAttrLen)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, size_t attrlen, dhash_t hash, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallInstanceMemberAttrLenKw)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, size_t attrlen, dhash_t hash, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallIInstanceMemberAttrLenKw)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, size_t attrlen, dhash_t hash, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int (DCALL DeeType_BoundMemberAttr)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, DeeObject *self, char const *__restrict attr_name, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int (DCALL DeeType_BoundClassMemberAttr)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int (DCALL DeeType_DelMemberAttr)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, DeeObject *self, char const *__restrict attr_name, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int (DCALL DeeType_DelClassMemberAttr)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3, 4, 6)) int (DCALL DeeType_SetMemberAttr)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, DeeObject *self, char const *__restrict attr_name, dhash_t hash, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3, 5)) int (DCALL DeeType_SetClassMemberAttr)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, dhash_t hash, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int (DCALL DeeType_BoundMemberAttrLen)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, DeeObject *self, char const *__restrict attr_name, size_t attrlen, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int (DCALL DeeType_BoundClassMemberAttrLen)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, size_t attrlen, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int (DCALL DeeType_DelMemberAttrLen)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, DeeObject *self, char const *__restrict attr_name, size_t attrlen, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int (DCALL DeeType_DelClassMemberAttrLen)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, size_t attrlen, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3, 4, 7)) int (DCALL DeeType_SetMemberAttrLen)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, DeeObject *self, char const *__restrict attr_name, size_t attrlen, dhash_t hash, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3, 6)) int (DCALL DeeType_SetClassMemberAttrLen)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, size_t attrlen, dhash_t hash, DeeObject *value);
#else /* __INTELLISENSE__ */
INTDEF WUNUSED NONNULL((1, 2, 3, 4, 5)) DREF DeeObject *DCALL /* GET_MEMBER */
type_member_getattr(struct Dee_membercache *cache, DeeTypeObject *decl,
                    struct type_member const *chain, DeeObject *self,
                    char const *__restrict attr_name, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3, 4, 5)) DREF DeeObject *DCALL /* GET_MEMBER */
type_member_getattr_len(struct Dee_membercache *cache, DeeTypeObject *decl,
                        struct type_member const *chain, DeeObject *self,
                        char const *__restrict attr_name, size_t attrlen, dhash_t hash);
#define DeeType_GetMemberAttr(tp_invoker, tp_self, self, attr_name, hash)             type_member_getattr(&(tp_invoker)->tp_cache, tp_self, (tp_self)->tp_members, self, attr_name, hash)
#define DeeType_GetClassMemberAttr(tp_invoker, tp_self, attr_name, hash)              type_member_getattr(&(tp_invoker)->tp_class_cache, tp_self, (tp_self)->tp_class_members, (DeeObject *)(tp_invoker), attr_name, hash)
#define DeeType_GetMemberAttrLen(tp_invoker, tp_self, self, attr_name, attrlen, hash) type_member_getattr_len(&(tp_invoker)->tp_cache, tp_self, (tp_self)->tp_members, self, attr_name, attrlen, hash)
#define DeeType_GetClassMemberAttrLen(tp_invoker, tp_self, attr_name, attrlen, hash)  type_member_getattr_len(&(tp_invoker)->tp_class_cache, tp_self, (tp_self)->tp_class_members, (DeeObject *)(tp_invoker), attr_name, attrlen, hash)
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_GetInstanceMemberAttr)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_GetIInstanceMemberAttr)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallInstanceMemberAttr)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, dhash_t hash, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_GetInstanceMemberAttrLen)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, size_t attrlen, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_GetIInstanceMemberAttrLen)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, size_t attrlen, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallInstanceMemberAttrLen)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, size_t attrlen, dhash_t hash, size_t argc, DeeObject *const *argv);
#define DeeType_CallIInstanceMemberAttr(tp_invoker, tp_self, attr_name, hash, argc, argv) \
	DeeType_CallIInstanceMemberAttrKw(tp_invoker, tp_self, attr_name, hash, argc, argv, NULL)
#define DeeType_CallIInstanceMemberAttrLen(tp_invoker, tp_self, attr_name, attrlen, hash, argc, argv) \
	DeeType_CallIInstanceMemberAttrLenKw(tp_invoker, tp_self, attr_name, attrlen, hash, argc, argv, NULL)
//INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallIInstanceMemberAttr)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, dhash_t hash, size_t argc, DeeObject *const *argv);
//INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallIInstanceMemberAttrLen)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, size_t attrlen, dhash_t hash, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallInstanceMemberAttrKw)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, dhash_t hash, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallInstanceMemberAttrLenKw)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, size_t attrlen, dhash_t hash, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallIInstanceMemberAttrKw)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, dhash_t hash, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallIInstanceMemberAttrLenKw)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, size_t attrlen, dhash_t hash, size_t argc, DeeObject *const *argv, DeeObject *kw);

INTDEF WUNUSED NONNULL((1, 2, 3, 4, 5)) int DCALL /* BOUND_MEMBER */
type_member_boundattr(struct Dee_membercache *cache, DeeTypeObject *decl,
                      struct type_member const *chain, DeeObject *self,
                      char const *__restrict attr_name, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3, 4, 5)) int DCALL /* BOUND_MEMBER */
type_member_boundattr_len(struct Dee_membercache *cache, DeeTypeObject *decl,
                          struct type_member const *chain, DeeObject *self,
                          char const *__restrict attr_name, size_t attrlen, dhash_t hash);
#define DeeType_BoundMemberAttr(tp_invoker, tp_self, self, attr_name, hash)             type_member_boundattr(&(tp_invoker)->tp_cache, tp_self, (tp_self)->tp_members, self, attr_name, hash)
#define DeeType_BoundClassMemberAttr(tp_invoker, tp_self, attr_name, hash)              type_member_boundattr(&(tp_invoker)->tp_class_cache, tp_self, (tp_self)->tp_class_members, (DeeObject *)(tp_invoker), attr_name, hash)
#define DeeType_BoundMemberAttrLen(tp_invoker, tp_self, self, attr_name, attrlen, hash) type_member_boundattr_len(&(tp_invoker)->tp_cache, tp_self, (tp_self)->tp_members, self, attr_name, attrlen, hash)
#define DeeType_BoundClassMemberAttrLen(tp_invoker, tp_self, attr_name, attrlen, hash)  type_member_boundattr_len(&(tp_invoker)->tp_class_cache, tp_self, (tp_self)->tp_class_members, (DeeObject *)(tp_invoker), attr_name, attrlen, hash)

INTDEF WUNUSED NONNULL((1, 2, 3, 4, 5)) int DCALL /* DEL_MEMBER */
type_member_delattr(struct Dee_membercache *cache, DeeTypeObject *decl,
                    struct type_member const *chain, DeeObject *self,
                    char const *__restrict attr_name, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3, 4, 5)) int DCALL /* DEL_MEMBER */
type_member_delattr_len(struct Dee_membercache *cache, DeeTypeObject *decl,
                        struct type_member const *chain, DeeObject *self,
                        char const *__restrict attr_name, size_t attrlen, dhash_t hash);
#define DeeType_DelMemberAttr(tp_invoker, tp_self, self, attr_name, hash)             type_member_delattr(&(tp_invoker)->tp_cache, tp_self, (tp_self)->tp_members, self, attr_name, hash)
#define DeeType_DelClassMemberAttr(tp_invoker, tp_self, attr_name, hash)              type_member_delattr(&(tp_invoker)->tp_class_cache, tp_self, (tp_self)->tp_class_members, (DeeObject *)(tp_invoker), attr_name, hash)
#define DeeType_DelMemberAttrLen(tp_invoker, tp_self, self, attr_name, attrlen, hash) type_member_delattr_len(&(tp_invoker)->tp_cache, tp_self, (tp_self)->tp_members, self, attr_name, attrlen, hash)
#define DeeType_DelClassMemberAttrLen(tp_invoker, tp_self, attr_name, attrlen, hash)  type_member_delattr_len(&(tp_invoker)->tp_class_cache, tp_self, (tp_self)->tp_class_members, (DeeObject *)(tp_invoker), attr_name, attrlen, hash)

INTDEF WUNUSED NONNULL((1, 2, 3, 4, 5, 7)) int DCALL /* SET_MEMBER */
type_member_setattr(struct Dee_membercache *cache, DeeTypeObject *decl,
                    struct type_member const *chain, DeeObject *self,
                    char const *__restrict attr_name, dhash_t hash, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3, 4, 5, 8)) int DCALL /* SET_MEMBER */
type_member_setattr_len(struct Dee_membercache *cache, DeeTypeObject *decl,
                        struct type_member const *chain, DeeObject *self,
                        char const *__restrict attr_name, size_t attrlen,
                        dhash_t hash, DeeObject *value);
#define DeeType_SetMemberAttr(tp_invoker, tp_self, self, attr_name, hash, value)             type_member_setattr(&(tp_invoker)->tp_cache, tp_self, (tp_self)->tp_members, self, attr_name, hash, value)
#define DeeType_SetClassMemberAttr(tp_invoker, tp_self, attr_name, hash, value)              type_member_setattr(&(tp_invoker)->tp_class_cache, tp_self, (tp_self)->tp_class_members, (DeeObject *)(tp_invoker), attr_name, hash, value)
#define DeeType_SetMemberAttrLen(tp_invoker, tp_self, self, attr_name, attrlen, hash, value) type_member_setattr_len(&(tp_invoker)->tp_cache, tp_self, (tp_self)->tp_members, self, attr_name, attrlen, hash, value)
#define DeeType_SetClassMemberAttrLen(tp_invoker, tp_self, attr_name, attrlen, hash, value)  type_member_setattr_len(&(tp_invoker)->tp_class_cache, tp_self, (tp_self)->tp_class_members, (DeeObject *)(tp_invoker), attr_name, attrlen, hash, value)
#endif /* !__INTELLISENSE__ */


/* Check for the existence of specific attributes. */
#ifdef __INTELLISENSE__
INTDEF WUNUSED NONNULL((1, 2, 3)) bool (DCALL DeeType_HasMethodAttr)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) bool (DCALL DeeType_HasClassMethodAttr)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) bool (DCALL DeeType_HasInstanceMethodAttr)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) bool (DCALL DeeType_HasIInstanceMethodAttr)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) bool (DCALL DeeType_HasGetSetAttr)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) bool (DCALL DeeType_HasClassGetSetAttr)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) bool (DCALL DeeType_HasInstanceGetSetAttr)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) bool (DCALL DeeType_HasIInstanceGetSetAttr)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) bool (DCALL DeeType_HasMemberAttr)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) bool (DCALL DeeType_HasClassMemberAttr)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) bool (DCALL DeeType_HasInstanceMemberAttr)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) bool (DCALL DeeType_HasIInstanceMemberAttr)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) bool (DCALL DeeType_HasMethodAttrLen)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, size_t attrlen, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) bool (DCALL DeeType_HasClassMethodAttrLen)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, size_t attrlen, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) bool (DCALL DeeType_HasInstanceMethodAttrLen)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, size_t attrlen, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) bool (DCALL DeeType_HasIInstanceMethodAttrLen)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, size_t attrlen, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) bool (DCALL DeeType_HasGetSetAttrLen)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, size_t attrlen, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) bool (DCALL DeeType_HasClassGetSetAttrLen)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, size_t attrlen, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) bool (DCALL DeeType_HasInstanceGetSetAttrLen)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, size_t attrlen, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) bool (DCALL DeeType_HasIInstanceGetSetAttrLen)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, size_t attrlen, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) bool (DCALL DeeType_HasMemberAttrLen)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, size_t attrlen, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) bool (DCALL DeeType_HasClassMemberAttrLen)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, size_t attrlen, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) bool (DCALL DeeType_HasInstanceMemberAttrLen)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, size_t attrlen, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) bool (DCALL DeeType_HasIInstanceMemberAttrLen)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, size_t attrlen, dhash_t hash);
#else /* __INTELLISENSE__ */
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) bool DCALL /* METHOD */
type_method_hasattr(struct Dee_membercache *cache, DeeTypeObject *decl,
                    struct type_method const *chain,
                    char const *__restrict attr_name, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) bool DCALL /* METHOD */
type_method_hasattr_len(struct Dee_membercache *cache, DeeTypeObject *decl,
                        struct type_method const *chain,
                        char const *__restrict attr_name,
                        size_t attrlen, dhash_t hash);
#define DeeType_HasMethodAttr(tp_invoker, tp_self, attr_name, hash)                  type_method_hasattr(&(tp_invoker)->tp_cache, tp_self, (tp_self)->tp_methods, attr_name, hash)
#define DeeType_HasClassMethodAttr(tp_invoker, tp_self, attr_name, hash)             type_method_hasattr(&(tp_invoker)->tp_class_cache, tp_self, (tp_self)->tp_class_methods, attr_name, hash)
#define DeeType_HasMethodAttrLen(tp_invoker, tp_self, attr_name, attrlen, hash)      type_method_hasattr_len(&(tp_invoker)->tp_cache, tp_self, (tp_self)->tp_methods, attr_name, attrlen, hash)
#define DeeType_HasClassMethodAttrLen(tp_invoker, tp_self, attr_name, attrlen, hash) type_method_hasattr_len(&(tp_invoker)->tp_class_cache, tp_self, (tp_self)->tp_class_methods, attr_name, attrlen, hash)
INTDEF WUNUSED NONNULL((1, 2, 3)) bool (DCALL DeeType_HasInstanceMethodAttr)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) bool (DCALL DeeType_HasInstanceMethodAttrLen)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, size_t attrlen, dhash_t hash);
#define DeeType_HasIInstanceMethodAttr(tp_invoker, tp_self, attr_name, hash)    type_method_hasattr(&(tp_invoker)->tp_cache, tp_self, (tp_self)->tp_methods, attr_name, hash)
#define DeeType_HasIInstanceMethodAttrLen(tp_invoker, tp_self, attr_name, hash) type_method_hasattr_len(&(tp_invoker)->tp_cache, tp_self, (tp_self)->tp_methods, attr_name, attrlen, hash)

INTDEF WUNUSED NONNULL((1, 2, 3, 4)) bool DCALL /* GETSET */
type_getset_hasattr(struct Dee_membercache *cache, DeeTypeObject *decl,
                    struct type_getset const *chain,
                    char const *__restrict attr_name, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) bool DCALL /* GETSET */
type_getset_hasattr_len(struct Dee_membercache *cache, DeeTypeObject *decl,
                        struct type_getset const *chain,
                        char const *__restrict attr_name,
                        size_t attrlen, dhash_t hash);
#define DeeType_HasGetSetAttr(tp_invoker, tp_self, attr_name, hash)                  type_getset_hasattr(&(tp_invoker)->tp_cache, tp_self, (tp_self)->tp_getsets, attr_name, hash)
#define DeeType_HasClassGetSetAttr(tp_invoker, tp_self, attr_name, hash)             type_getset_hasattr(&(tp_invoker)->tp_class_cache, tp_self, (tp_self)->tp_class_getsets, attr_name, hash)
#define DeeType_HasGetSetAttrLen(tp_invoker, tp_self, attr_name, attrlen, hash)      type_getset_hasattr_len(&(tp_invoker)->tp_cache, tp_self, (tp_self)->tp_getsets, attr_name, attrlen, hash)
#define DeeType_HasClassGetSetAttrLen(tp_invoker, tp_self, attr_name, attrlen, hash) type_getset_hasattr_len(&(tp_invoker)->tp_class_cache, tp_self, (tp_self)->tp_class_getsets, attr_name, attrlen, hash)
INTDEF WUNUSED NONNULL((1, 2, 3)) bool (DCALL DeeType_HasInstanceGetSetAttr)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) bool (DCALL DeeType_HasInstanceGetSetAttrLen)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, size_t attrlen, dhash_t hash);
#define DeeType_HasIInstanceGetSetAttr(tp_invoker, tp_self, attr_name, hash)    type_getset_hasattr(&(tp_invoker)->tp_cache, tp_self, (tp_self)->tp_getsets, attr_name, hash)
#define DeeType_HasIInstanceGetSetAttrLen(tp_invoker, tp_self, attr_name, hash) type_getset_hasattr(&(tp_invoker)->tp_cache, tp_self, (tp_self)->tp_getsets, attr_name, attrlen, hash)

INTDEF WUNUSED NONNULL((1, 2, 3, 4)) bool DCALL /* MEMBER */
type_member_hasattr(struct Dee_membercache *cache, DeeTypeObject *decl,
                    struct type_member const *chain,
                    char const *__restrict attr_name, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) bool DCALL /* MEMBER */
type_member_hasattr_len(struct Dee_membercache *cache, DeeTypeObject *decl,
                        struct type_member const *chain,
                        char const *__restrict attr_name,
                        size_t attrlen, dhash_t hash);
#define DeeType_HasMemberAttr(tp_invoker, tp_self, attr_name, hash)                  type_member_hasattr(&(tp_invoker)->tp_cache, tp_self, (tp_self)->tp_members, attr_name, hash)
#define DeeType_HasClassMemberAttr(tp_invoker, tp_self, attr_name, hash)             type_member_hasattr(&(tp_invoker)->tp_class_cache, tp_self, (tp_self)->tp_class_members, attr_name, hash)
#define DeeType_HasMemberAttrLen(tp_invoker, tp_self, attr_name, attrlen, hash)      type_member_hasattr_len(&(tp_invoker)->tp_cache, tp_self, (tp_self)->tp_members, attr_name, attrlen, hash)
#define DeeType_HasClassMemberAttrLen(tp_invoker, tp_self, attr_name, attrlen, hash) type_member_hasattr_len(&(tp_invoker)->tp_class_cache, tp_self, (tp_self)->tp_class_members, attr_name, attrlen, hash)
INTDEF WUNUSED NONNULL((1, 2, 3)) bool DCALL DeeType_HasInstanceMemberAttr(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) bool DCALL DeeType_HasInstanceMemberAttrLen(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr_name, size_t attrlen, dhash_t hash);
#define DeeType_HasIInstanceMemberAttr(tp_invoker, tp_self, attr_name, hash)             type_member_hasattr(&(tp_invoker)->tp_cache, tp_self, (tp_self)->tp_members, attr_name, hash)
#define DeeType_HasIInstanceMemberAttrLen(tp_invoker, tp_self, attr_name, attrlen, hash) type_member_hasattr_len(&(tp_invoker)->tp_cache, tp_self, (tp_self)->tp_members, attr_name, attrlen, hash)
#endif /* !__INTELLISENSE__ */

/* Find matching attributes. */
#ifdef __INTELLISENSE__
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int (DCALL DeeType_FindMethodAttr)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, struct attribute_info *__restrict result, struct attribute_lookup_rules const *__restrict rules);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int (DCALL DeeType_FindClassMethodAttr)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, struct attribute_info *__restrict result, struct attribute_lookup_rules const *__restrict rules);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int (DCALL DeeType_FindInstanceMethodAttr)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, struct attribute_info *__restrict result, struct attribute_lookup_rules const *__restrict rules);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int (DCALL DeeType_FindGetSetAttr)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, struct attribute_info *__restrict result, struct attribute_lookup_rules const *__restrict rules);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int (DCALL DeeType_FindClassGetSetAttr)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, struct attribute_info *__restrict result, struct attribute_lookup_rules const *__restrict rules);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int (DCALL DeeType_FindInstanceGetSetAttr)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, struct attribute_info *__restrict result, struct attribute_lookup_rules const *__restrict rules);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int (DCALL DeeType_FindMemberAttr)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, struct attribute_info *__restrict result, struct attribute_lookup_rules const *__restrict rules);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int (DCALL DeeType_FindClassMemberAttr)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, struct attribute_info *__restrict result, struct attribute_lookup_rules const *__restrict rules);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int (DCALL DeeType_FindInstanceMemberAttr)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, struct attribute_info *__restrict result, struct attribute_lookup_rules const *__restrict rules);
#else /* __INTELLISENSE__ */
INTDEF WUNUSED NONNULL((1, 2, 3, 5, 6)) int DCALL /* METHOD */
type_method_findattr(struct Dee_membercache *cache, DeeTypeObject *decl,
                     struct type_method const *chain, uint16_t perm,
                     struct attribute_info *__restrict result,
                     struct attribute_lookup_rules const *__restrict rules);
#define DeeType_FindMethodAttr(tp_invoker, tp_self, result, rules)      type_method_findattr(&(tp_invoker)->tp_cache, tp_self, (tp_self)->tp_methods, Dee_ATTR_IMEMBER, result, rules)
#define DeeType_FindClassMethodAttr(tp_invoker, tp_self, result, rules) type_method_findattr(&(tp_invoker)->tp_class_cache, tp_self, (tp_self)->tp_class_methods, Dee_ATTR_CMEMBER, result, rules)
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
DeeType_FindInstanceMethodAttr(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self,
                               struct attribute_info *__restrict result,
                               struct attribute_lookup_rules const *__restrict rules);

INTDEF WUNUSED NONNULL((1, 2, 3, 5, 6)) int DCALL /* GETSET */
type_getset_findattr(struct Dee_membercache *cache, DeeTypeObject *decl,
                     struct type_getset const *chain, uint16_t perm,
                     struct attribute_info *__restrict result,
                     struct attribute_lookup_rules const *__restrict rules);
#define DeeType_FindGetSetAttr(tp_invoker, tp_self, result, rules)      type_getset_findattr(&(tp_invoker)->tp_cache, tp_self, (tp_self)->tp_getsets, Dee_ATTR_IMEMBER, result, rules)
#define DeeType_FindClassGetSetAttr(tp_invoker, tp_self, result, rules) type_getset_findattr(&(tp_invoker)->tp_class_cache, tp_self, (tp_self)->tp_class_getsets, Dee_ATTR_CMEMBER, result, rules)
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
DeeType_FindInstanceGetSetAttr(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self,
                               struct attribute_info *__restrict result,
                               struct attribute_lookup_rules const *__restrict rules);

INTDEF WUNUSED NONNULL((1, 2, 3, 5, 6)) int DCALL /* MEMBER */
type_member_findattr(struct Dee_membercache *cache, DeeTypeObject *decl,
                     struct type_member const *chain, uint16_t perm,
                     struct attribute_info *__restrict result,
                     struct attribute_lookup_rules const *__restrict rules);
#define DeeType_FindMemberAttr(tp_invoker, tp_self, result, rules)      type_member_findattr(&(tp_invoker)->tp_cache, tp_self, (tp_self)->tp_members, Dee_ATTR_IMEMBER, result, rules)
#define DeeType_FindClassMemberAttr(tp_invoker, tp_self, result, rules) type_member_findattr(&(tp_invoker)->tp_class_cache, tp_self, (tp_self)->tp_class_members, Dee_ATTR_CMEMBER, result, rules)
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
DeeType_FindInstanceMemberAttr(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self,
                               struct attribute_info *__restrict result,
                               struct attribute_lookup_rules const *__restrict rules);
#endif /* !__INTELLISENSE__ */

/* Enumerate attributes. */
INTDEF WUNUSED NONNULL((1, 2, 4)) dssize_t DCALL type_method_enum(DeeTypeObject *__restrict tp_self, struct type_method const *chain, uint16_t flags, denum_t proc, void *arg);
INTDEF WUNUSED NONNULL((1, 2, 4)) dssize_t DCALL type_getset_enum(DeeTypeObject *__restrict tp_self, struct type_getset const *chain, uint16_t flags, denum_t proc, void *arg);
INTDEF WUNUSED NONNULL((1, 2, 4)) dssize_t DCALL type_member_enum(DeeTypeObject *__restrict tp_self, struct type_member const *chain, uint16_t flags, denum_t proc, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) dssize_t DCALL type_obmeth_enum(DeeTypeObject *__restrict tp_self, denum_t proc, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) dssize_t DCALL type_obprop_enum(DeeTypeObject *__restrict tp_self, denum_t proc, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) dssize_t DCALL type_obmemb_enum(DeeTypeObject *__restrict tp_self, denum_t proc, void *arg);

/* Helper functions for accessing type attributes. */
#define type_method_get(desc, self)                                                                  \
	(((desc)->m_flag & TYPE_METHOD_FKWDS) ? DeeKwObjMethod_New((dkwobjmethod_t)(desc)->m_func, self) \
	                                      : DeeObjMethod_New((desc)->m_func, self))
#define type_method_doc(desc) DeeString_NewUtf8((desc)->m_doc, strlen((desc)->m_doc), STRING_ERROR_FIGNORE)
#define type_method_call(desc, self, argc, argv)                                                                            \
	(((desc)->m_flag & TYPE_METHOD_FKWDS) ? DeeKwObjMethod_CallFunc((dkwobjmethod_t)(desc)->m_func, self, argc, argv, NULL) \
	                                      : DeeObjMethod_CallFunc((desc)->m_func, self, argc, argv))
#define type_method_call_kw(desc, self, argc, argv, kw)                                                                   \
	(((desc)->m_flag & TYPE_METHOD_FKWDS) ? DeeKwObjMethod_CallFunc((dkwobjmethod_t)(desc)->m_func, self, argc, argv, kw) \
	                                      : type_method_call_kw_normal(desc, self, argc, argv, kw))
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL type_method_call_kw_normal(struct type_method const *desc, DeeObject *self, size_t argc, DeeObject *const *argv, DeeObject *kw);
#define type_obmeth_get(cls_type, desc)                                                                  \
	(((desc)->m_flag & TYPE_METHOD_FKWDS) ? DeeKwClsMethod_New(cls_type, (dkwobjmethod_t)(desc)->m_func) \
	                                      : DeeClsMethod_New(cls_type, (desc)->m_func))
#define type_obmeth_doc(desc) DeeString_NewUtf8((desc)->m_doc, strlen((desc)->m_doc), STRING_ERROR_FIGNORE)
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL type_obmeth_call(DeeTypeObject *cls_type, struct type_method const *desc, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL type_obmeth_call_kw(DeeTypeObject *cls_type, struct type_method const *desc, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL type_getset_get(struct type_getset const *desc, DeeObject *__restrict self);
#define type_obprop_get(cls_type, desc) DeeClsProperty_New(cls_type, (desc)->gs_get, (desc)->gs_del, (desc)->gs_set)
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL type_obprop_call(DeeTypeObject *cls_type, struct type_getset const *desc, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL type_obprop_call_kw(DeeTypeObject *cls_type, struct type_getset const *desc, size_t argc, DeeObject *const *argv, DeeObject *kw);
#define type_obprop_doc(desc)           DeeString_NewUtf8((desc)->gs_doc, strlen((desc)->gs_doc), STRING_ERROR_FIGNORE)
#define type_obmemb_get(cls_type, desc) DeeClsMember_New(cls_type, desc)
#define type_obmemb_doc(desc)           DeeString_NewUtf8((desc)->m_doc, strlen((desc)->m_doc), STRING_ERROR_FIGNORE)
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL type_obmemb_call(DeeTypeObject *cls_type, struct type_member const *desc, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL type_obmemb_call_kw(DeeTypeObject *cls_type, struct type_member const *desc, size_t argc, DeeObject *const *argv, DeeObject *kw);
#define type_getset_doc(desc) DeeString_NewUtf8((desc)->gs_doc, strlen((desc)->gs_doc), STRING_ERROR_FIGNORE)
INTDEF WUNUSED NONNULL((1, 2)) int DCALL type_getset_del(struct type_getset const *desc, DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL type_getset_set(struct type_getset const *desc, DeeObject *self, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL type_member_get(struct type_member const *desc, DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1, 2)) bool DCALL type_member_bound(struct type_member const *desc, DeeObject *__restrict self);
#define type_member_doc(desc)       DeeString_NewUtf8((desc)->m_doc, strlen((desc)->m_doc), STRING_ERROR_FIGNORE)
#define type_member_del(desc, self) type_member_set(desc, self, Dee_None)
INTDEF WUNUSED NONNULL((1, 2)) int DCALL type_member_set(struct type_member const *desc, DeeObject *self, DeeObject *value);


/* Static (class) attribute access for type objects. */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeType_GetAttrString(DeeTypeObject *__restrict self, char const *__restrict attr, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeType_GetAttrStringLen(DeeTypeObject *__restrict self, char const *__restrict attr, size_t attrlen, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeType_BoundAttrString(DeeTypeObject *__restrict self, char const *__restrict attr, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeType_BoundAttrStringLen(DeeTypeObject *__restrict self, char const *__restrict attr, size_t attrlen, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL DeeType_FindAttrString(DeeTypeObject *__restrict self, struct attribute_info *__restrict result, struct attribute_lookup_rules const *__restrict rules);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeType_CallAttrString(DeeTypeObject *self, char const *__restrict attr, dhash_t hash, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeType_CallAttrStringLen(DeeTypeObject *self, char const *__restrict attr, size_t attrlen, dhash_t hash, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeType_CallAttrStringKw(DeeTypeObject *self, char const *__restrict attr, dhash_t hash, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeType_CallAttrStringLenKw(DeeTypeObject *self, char const *__restrict attr, size_t attrlen, dhash_t hash, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1, 2)) bool DCALL DeeType_HasAttrString(DeeTypeObject *__restrict self, char const *__restrict attr, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) bool DCALL DeeType_HasAttrStringLen(DeeTypeObject *__restrict self, char const *__restrict attr, size_t attrlen, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeType_DelAttrString(DeeTypeObject *__restrict self, char const *__restrict attr, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeType_DelAttrStringLen(DeeTypeObject *__restrict self, char const *__restrict attr, size_t attrlen, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 4)) int DCALL DeeType_SetAttrString(DeeTypeObject *self, char const *__restrict attr, dhash_t hash, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 5)) int DCALL DeeType_SetAttrStringLen(DeeTypeObject *self, char const *__restrict attr, size_t attrlen, dhash_t hash, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2)) dssize_t DCALL DeeType_EnumAttr(DeeTypeObject *__restrict self, denum_t proc, void *arg);

/* Instance-only (wrapper) attribute access to the attributes of instance of types. */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeType_GetInstanceAttrString(DeeTypeObject *__restrict self, char const *__restrict attr, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL DeeType_CallInstanceAttrStringKw(DeeTypeObject *self, char const *__restrict attr, dhash_t hash, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1, 2)) bool DCALL DeeType_HasInstanceAttrString(DeeTypeObject *__restrict self, char const *__restrict attr, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int DCALL DeeType_BoundInstanceAttrString(DeeTypeObject *__restrict self, char const *__restrict attr, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeType_DelInstanceAttrString)(DeeTypeObject *__restrict self, char const *__restrict attr, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 4)) int (DCALL DeeType_SetInstanceAttrString)(DeeTypeObject *self, char const *__restrict attr, dhash_t hash, DeeObject *value);


#ifndef __INTELLISENSE__
#ifndef __NO_builtin_expect
#define DeeType_DelAttrString(self, attr, hash)                            __builtin_expect(DeeType_DelAttrString(self, attr, hash), 0)
#define DeeType_DelAttrStringLen(self, attr, attrlen, hash)                __builtin_expect(DeeType_DelAttrStringLen(self, attr, attrlen, hash), 0)
#define DeeType_SetAttrString(self, attr, hash, value)                     __builtin_expect(DeeType_SetAttrString(self, attr, hash, value), 0)
#define DeeType_SetAttrStringLen(self, attr, attrlen, hash, value)         __builtin_expect(DeeType_SetAttrStringLen(self, attr, attrlen, hash, value), 0)
#define DeeType_DelInstanceAttrString(self, attr, hash)                    __builtin_expect(DeeType_DelInstanceAttrString(self, attr, hash), 0)
#define DeeType_DelInstanceAttrStringLen(self, attr, attrlen, hash)        __builtin_expect(DeeType_DelInstanceAttrStringLen(self, attr, attrlen, hash), 0)
#define DeeType_SetInstanceAttrString(self, attr, hash, value)             __builtin_expect(DeeType_SetInstanceAttrString(self, attr, hash, value), 0)
#define DeeType_SetInstanceAttrStringLen(self, attr, attrlen, hash, value) __builtin_expect(DeeType_SetInstanceAttrStringLen(self, attr, attrlen, hash, value), 0)
#endif /* !__NO_builtin_expect */
#endif /* !__INTELLISENSE__ */

#endif /* CONFIG_BUILDING_DEEMON */

DECL_END

#endif /* !GUARD_DEEMON_MRO_H */
