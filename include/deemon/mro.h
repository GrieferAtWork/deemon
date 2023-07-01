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
 *  - DeeType_GetCachedAttrStringHash         -- `"foo".lower'
 *  - DeeType_GetCachedClassAttrStringHash    -- `string.chr', `string.lower'
 *  - DeeType_GetCachedInstanceAttrStringHash -- `string.getinstanceattr("lower")'
 */

/* Lookup an attribute from cache.
 * @return: * :        The attribute value.
 * @return: NULL:      An error occurred.
 * @return: ITER_DONE: The attribute could not be found in the cache. */
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_GetCachedAttrStringHash)(DeeTypeObject *tp_self, DeeObject *self, char const *__restrict attr, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_GetCachedAttrStringLenHash)(DeeTypeObject *tp_self, DeeObject *self, char const *__restrict attr, size_t attrlen, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL DeeType_GetCachedClassAttrStringHash)(DeeTypeObject *__restrict tp_self, char const *__restrict attr, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL DeeType_GetCachedClassAttrStringLenHash)(DeeTypeObject *__restrict tp_self, char const *__restrict attr, size_t attrlen, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL DeeType_GetCachedInstanceAttrStringHash)(DeeTypeObject *__restrict tp_self, char const *__restrict attr, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL DeeType_GetCachedInstanceAttrStringLenHash)(DeeTypeObject *__restrict tp_self, char const *__restrict attr, size_t attrlen, dhash_t hash);
#define DeeType_GetCachedAttr(tp_self, self, attr)                     DeeType_GetCachedAttrStringHash(tp_self, self, DeeString_STR(attr), DeeString_Hash(attr))
#define DeeType_GetCachedAttrHash(tp_self, self, attr, hash)           DeeType_GetCachedAttrStringHash(tp_self, self, DeeString_STR(attr), hash)
#define DeeType_GetCachedAttrString(tp_self, self, attr)               DeeType_GetCachedAttrStringHash(tp_self, self, attr, Dee_HashStr(attr))
#define DeeType_GetCachedAttrStringLen(tp_self, self, attr, attrlen)   DeeType_GetCachedAttrStringLenHash(tp_self, self, attr, attrlen, Dee_HashPtr(attr, attrlen))
#define DeeType_GetCachedClassAttr(tp_self, attr)                      DeeType_GetCachedClassAttrStringHash(tp_self, DeeString_STR(attr), DeeString_Hash(attr))
#define DeeType_GetCachedClassAttrHash(tp_self, attr, hash)            DeeType_GetCachedClassAttrStringHash(tp_self, DeeString_STR(attr), hash)
#define DeeType_GetCachedClassAttrString(tp_self, attr)                DeeType_GetCachedClassAttrStringHash(tp_self, attr, Dee_HashStr(attr))
#define DeeType_GetCachedClassAttrStringLen(tp_self, attr, attrlen)    DeeType_GetCachedClassAttrStringLenHash(tp_self, attr, attrlen, Dee_HashPtr(attr, attrlen))
#define DeeType_GetCachedInstanceAttr(tp_self, attr)                   DeeType_GetCachedInstanceAttrStringHash(tp_self, DeeString_STR(attr), DeeString_Hash(attr))
#define DeeType_GetCachedInstanceAttrHash(tp_self, attr, hash)         DeeType_GetCachedInstanceAttrStringHash(tp_self, DeeString_STR(attr), hash)
#define DeeType_GetCachedInstanceAttrString(tp_self, attr)             DeeType_GetCachedInstanceAttrStringHash(tp_self, attr, Dee_HashStr(attr))
#define DeeType_GetCachedInstanceAttrStringLen(tp_self, attr, attrlen) DeeType_GetCachedInstanceAttrStringLenHash(tp_self, attr, attrlen, Dee_HashPtr(attr, attrlen))

/* @return: 1 : Attribute is bound.
 * @return: 0 : Attribute isn't bound.
 * @return: -1: An error occurred.
 * @return: -2: The attribute doesn't exist. */
INTDEF WUNUSED NONNULL((1, 2, 3)) int (DCALL DeeType_BoundCachedAttrStringHash)(DeeTypeObject *tp_self, DeeObject *self, char const *__restrict attr, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int (DCALL DeeType_BoundCachedAttrStringLenHash)(DeeTypeObject *tp_self, DeeObject *self, char const *__restrict attr, size_t attrlen, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeType_BoundCachedClassAttrStringHash)(DeeTypeObject *__restrict tp_self, char const *__restrict attr, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeType_BoundCachedClassAttrStringLenHash)(DeeTypeObject *__restrict tp_self, char const *__restrict attr, size_t attrlen, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeType_BoundCachedInstanceAttrStringHash)(DeeTypeObject *__restrict tp_self, char const *__restrict attr, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeType_BoundCachedInstanceAttrStringLenHash)(DeeTypeObject *__restrict tp_self, char const *__restrict attr, size_t attrlen, dhash_t hash);
#define DeeType_BoundCachedAttr(tp_self, self, attr)                     DeeType_BoundCachedAttrStringHash(tp_self, self, DeeString_STR(attr), DeeString_Hash(attr))
#define DeeType_BoundCachedAttrHash(tp_self, self, attr, hash)           DeeType_BoundCachedAttrStringHash(tp_self, self, DeeString_STR(attr), hash)
#define DeeType_BoundCachedAttrString(tp_self, self, attr)               DeeType_BoundCachedAttrStringHash(tp_self, self, attr, Dee_HashStr(attr))
#define DeeType_BoundCachedAttrStringLen(tp_self, self, attr, attrlen)   DeeType_BoundCachedAttrStringLenHash(tp_self, self, attr, attrlen, Dee_HashPtr(attr, attrlen))
#define DeeType_BoundCachedClassAttr(tp_self, attr)                      DeeType_BoundCachedClassAttrStringHash(tp_self, DeeString_STR(attr), DeeString_Hash(attr))
#define DeeType_BoundCachedClassAttrHash(tp_self, attr, hash)            DeeType_BoundCachedClassAttrStringHash(tp_self, DeeString_STR(attr), hash)
#define DeeType_BoundCachedClassAttrString(tp_self, attr)                DeeType_BoundCachedClassAttrStringHash(tp_self, attr, Dee_HashStr(attr))
#define DeeType_BoundCachedClassAttrStringLen(tp_self, attr, attrlen)    DeeType_BoundCachedClassAttrStringLenHash(tp_self, attr, attrlen, Dee_HashPtr(attr, attrlen))
#define DeeType_BoundCachedInstanceAttr(tp_self, attr)                   DeeType_BoundCachedInstanceAttrStringHash(tp_self, DeeString_STR(attr), DeeString_Hash(attr))
#define DeeType_BoundCachedInstanceAttrHash(tp_self, attr, hash)         DeeType_BoundCachedInstanceAttrStringHash(tp_self, DeeString_STR(attr), hash)
#define DeeType_BoundCachedInstanceAttrString(tp_self, attr)             DeeType_BoundCachedInstanceAttrStringHash(tp_self, attr, Dee_HashStr(attr))
#define DeeType_BoundCachedInstanceAttrStringLen(tp_self, attr, attrlen) DeeType_BoundCachedInstanceAttrStringLenHash(tp_self, attr, attrlen, Dee_HashPtr(attr, attrlen))

/* @return: true : The attribute exists.
 * @return: false: The attribute doesn't exist. */
INTDEF WUNUSED NONNULL((1, 2)) bool (DCALL DeeType_HasCachedAttrStringHash)(DeeTypeObject *__restrict tp_self, char const *__restrict attr, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) bool (DCALL DeeType_HasCachedAttrStringLenHash)(DeeTypeObject *__restrict tp_self, char const *__restrict attr, size_t attrlen, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) bool (DCALL DeeType_HasCachedClassAttrStringHash)(DeeTypeObject *__restrict tp_self, char const *__restrict attr, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) bool (DCALL DeeType_HasCachedClassAttrStringLenHash)(DeeTypeObject *__restrict tp_self, char const *__restrict attr, size_t attrlen, dhash_t hash);
#ifdef __INTELLISENSE__
INTDEF WUNUSED NONNULL((1, 2)) bool (DCALL DeeType_HasCachedInstanceAttrStringHash)(DeeTypeObject *__restrict tp_self, char const *__restrict attr, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) bool (DCALL DeeType_HasCachedInstanceAttrStringLenHash)(DeeTypeObject *__restrict tp_self, char const *__restrict attr, size_t attrlen, dhash_t hash);
#else /* __INTELLISENSE__ */
#define DeeType_HasCachedInstanceAttrStringHash(tp_self, attr, hash)             DeeType_HasCachedAttrStringHash(tp_self, attr, hash)
#define DeeType_HasCachedInstanceAttrStringLenHash(tp_self, attr, attrlen, hash) DeeType_HasCachedAttrStringLenHash(tp_self, attr, attrlen, hash)
#endif /* !__INTELLISENSE__ */
#define DeeType_HasCachedAttr(tp_self, attr)                           DeeType_HasCachedAttrStringHash(tp_self, DeeString_STR(attr), DeeString_Hash(attr))
#define DeeType_HasCachedAttrHash(tp_self, attr, hash)                 DeeType_HasCachedAttrStringHash(tp_self, DeeString_STR(attr), hash)
#define DeeType_HasCachedAttrString(tp_self, attr)                     DeeType_HasCachedAttrStringHash(tp_self, attr, Dee_HashStr(attr))
#define DeeType_HasCachedAttrStringLen(tp_self, attr, attrlen)         DeeType_HasCachedAttrStringLenHash(tp_self, attr, attrlen, Dee_HashPtr(attr, attrlen))
#define DeeType_HasCachedClassAttr(tp_self, attr)                      DeeType_HasCachedClassAttrStringHash(tp_self, DeeString_STR(attr), DeeString_Hash(attr))
#define DeeType_HasCachedClassAttrHash(tp_self, attr, hash)            DeeType_HasCachedClassAttrStringHash(tp_self, DeeString_STR(attr), hash)
#define DeeType_HasCachedClassAttrString(tp_self, attr)                DeeType_HasCachedClassAttrStringHash(tp_self, attr, Dee_HashStr(attr))
#define DeeType_HasCachedClassAttrStringLen(tp_self, attr, attrlen)    DeeType_HasCachedClassAttrStringLenHash(tp_self, attr, attrlen, Dee_HashPtr(attr, attrlen))
#define DeeType_HasCachedInstanceAttr(tp_self, attr)                   DeeType_HasCachedInstanceAttrStringHash(tp_self, DeeString_STR(attr), DeeString_Hash(attr))
#define DeeType_HasCachedInstanceAttrHash(tp_self, attr, hash)         DeeType_HasCachedInstanceAttrStringHash(tp_self, DeeString_STR(attr), hash)
#define DeeType_HasCachedInstanceAttrString(tp_self, attr)             DeeType_HasCachedInstanceAttrStringHash(tp_self, attr, Dee_HashStr(attr))
#define DeeType_HasCachedInstanceAttrStringLen(tp_self, attr, attrlen) DeeType_HasCachedInstanceAttrStringLenHash(tp_self, attr, attrlen, Dee_HashPtr(attr, attrlen))

/* @return:  1: The attribute could not be found in the cache.
 * @return:  0: Successfully invoked the delete-operator on the attribute.
 * @return: -1: An error occurred. */
INTDEF WUNUSED NONNULL((1, 2, 3)) int (DCALL DeeType_DelCachedAttrStringHash)(DeeTypeObject *tp_self, DeeObject *self, char const *__restrict attr, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int (DCALL DeeType_DelCachedAttrStringLenHash)(DeeTypeObject *tp_self, DeeObject *self, char const *__restrict attr, size_t attrlen, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeType_DelCachedClassAttrStringHash)(DeeTypeObject *__restrict tp_self, char const *__restrict attr, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeType_DelCachedClassAttrStringLenHash)(DeeTypeObject *__restrict tp_self, char const *__restrict attr, size_t attrlen, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeType_DelCachedInstanceAttrStringHash)(DeeTypeObject *__restrict tp_self, char const *__restrict attr, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeType_DelCachedInstanceAttrStringLenHash)(DeeTypeObject *__restrict tp_self, char const *__restrict attr, size_t attrlen, dhash_t hash);
#define DeeType_DelCachedAttr(tp_self, self, attr)                     DeeType_DelCachedAttrStringHash(tp_self, self, DeeString_STR(attr), DeeString_Hash(attr))
#define DeeType_DelCachedAttrHash(tp_self, self, attr, hash)           DeeType_DelCachedAttrStringHash(tp_self, self, DeeString_STR(attr), hash)
#define DeeType_DelCachedAttrString(tp_self, self, attr)               DeeType_DelCachedAttrStringHash(tp_self, self, attr, Dee_HashStr(attr))
#define DeeType_DelCachedAttrStringLen(tp_self, self, attr, attrlen)   DeeType_DelCachedAttrStringLenHash(tp_self, self, attr, attrlen, Dee_HashPtr(attr, attrlen))
#define DeeType_DelCachedClassAttr(tp_self, attr)                      DeeType_DelCachedClassAttrStringHash(tp_self, DeeString_STR(attr), DeeString_Hash(attr))
#define DeeType_DelCachedClassAttrHash(tp_self, attr, hash)            DeeType_DelCachedClassAttrStringHash(tp_self, DeeString_STR(attr), hash)
#define DeeType_DelCachedClassAttrString(tp_self, attr)                DeeType_DelCachedClassAttrStringHash(tp_self, attr, Dee_HashStr(attr))
#define DeeType_DelCachedClassAttrStringLen(tp_self, attr, attrlen)    DeeType_DelCachedClassAttrStringLenHash(tp_self, attr, attrlen, Dee_HashPtr(attr, attrlen))
#define DeeType_DelCachedInstanceAttr(tp_self, attr)                   DeeType_DelCachedInstanceAttrStringHash(tp_self, DeeString_STR(attr), DeeString_Hash(attr))
#define DeeType_DelCachedInstanceAttrHash(tp_self, attr, hash)         DeeType_DelCachedInstanceAttrStringHash(tp_self, DeeString_STR(attr), hash)
#define DeeType_DelCachedInstanceAttrString(tp_self, attr)             DeeType_DelCachedInstanceAttrStringHash(tp_self, attr, Dee_HashStr(attr))
#define DeeType_DelCachedInstanceAttrStringLen(tp_self, attr, attrlen) DeeType_DelCachedInstanceAttrStringLenHash(tp_self, attr, attrlen, Dee_HashPtr(attr, attrlen))

/* @return:  1: The attribute could not be found in the cache.
 * @return:  0: Successfully invoked the set-operator on the attribute.
 * @return: -1: An error occurred. */
INTDEF WUNUSED NONNULL((1, 2, 3, 5)) int (DCALL DeeType_SetCachedAttrStringHash)(DeeTypeObject *tp_self, DeeObject *self, char const *__restrict attr, dhash_t hash, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3, 6)) int (DCALL DeeType_SetCachedAttrStringLenHash)(DeeTypeObject *tp_self, DeeObject *self, char const *__restrict attr, size_t attrlen, dhash_t hash, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 4)) int (DCALL DeeType_SetCachedClassAttrStringHash)(DeeTypeObject *tp_self, char const *__restrict attr, dhash_t hash, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 5)) int (DCALL DeeType_SetCachedClassAttrStringLenHash)(DeeTypeObject *tp_self, char const *__restrict attr, size_t attrlen, dhash_t hash, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 4)) int (DCALL DeeType_SetCachedInstanceAttrStringHash)(DeeTypeObject *tp_self, char const *__restrict attr, dhash_t hash, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 5)) int (DCALL DeeType_SetCachedInstanceAttrStringLenHash)(DeeTypeObject *tp_self, char const *__restrict attr, size_t attrlen, dhash_t hash, DeeObject *value);
#define DeeType_SetCachedAttr(tp_self, self, attr, value)                     DeeType_SetCachedAttrStringHash(tp_self, self, DeeString_STR(attr), DeeString_Hash(attr), value)
#define DeeType_SetCachedAttrHash(tp_self, self, attr, hash, value)           DeeType_SetCachedAttrStringHash(tp_self, self, DeeString_STR(attr), hash, value)
#define DeeType_SetCachedAttrString(tp_self, self, attr, value)               DeeType_SetCachedAttrStringHash(tp_self, self, attr, Dee_HashStr(attr), value)
#define DeeType_SetCachedAttrStringLen(tp_self, self, attr, attrlen, value)   DeeType_SetCachedAttrStringLenHash(tp_self, self, attr, attrlen, Dee_HashPtr(attr, attrlen), value)
#define DeeType_SetCachedClassAttr(tp_self, attr, value)                      DeeType_SetCachedClassAttrStringHash(tp_self, DeeString_STR(attr), DeeString_Hash(attr), value)
#define DeeType_SetCachedClassAttrHash(tp_self, attr, hash, value)            DeeType_SetCachedClassAttrStringHash(tp_self, DeeString_STR(attr), hash, value)
#define DeeType_SetCachedClassAttrString(tp_self, attr, value)                DeeType_SetCachedClassAttrStringHash(tp_self, attr, Dee_HashStr(attr), value)
#define DeeType_SetCachedClassAttrStringLen(tp_self, attr, attrlen, value)    DeeType_SetCachedClassAttrStringLenHash(tp_self, attr, attrlen, Dee_HashPtr(attr, attrlen), value)
#define DeeType_SetCachedInstanceAttr(tp_self, attr, value)                   DeeType_SetCachedInstanceAttrStringHash(tp_self, DeeString_STR(attr), DeeString_Hash(attr), value)
#define DeeType_SetCachedInstanceAttrHash(tp_self, attr, hash, value)         DeeType_SetCachedInstanceAttrStringHash(tp_self, DeeString_STR(attr), hash, value)
#define DeeType_SetCachedInstanceAttrString(tp_self, attr, value)             DeeType_SetCachedInstanceAttrStringHash(tp_self, attr, Dee_HashStr(attr), value)
#define DeeType_SetCachedInstanceAttrStringLen(tp_self, attr, attrlen, value) DeeType_SetCachedInstanceAttrStringLenHash(tp_self, attr, attrlen, Dee_HashPtr(attr, attrlen), value)

/* @return:  1: The attribute could not be found in the cache.
 * @return:  0: Successfully invoked the set-operator on the attribute.
 * @return: -1: An error occurred. )(An error is also thrown for non-basic attributes) */
INTDEF WUNUSED NONNULL((1, 2, 3, 5)) int (DCALL DeeType_SetBasicCachedAttrStringHash)(DeeTypeObject *tp_self, DeeObject *self, char const *__restrict attr, dhash_t hash, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3, 6)) int (DCALL DeeType_SetBasicCachedAttrStringLenHash)(DeeTypeObject *tp_self, DeeObject *self, char const *__restrict attr, size_t attrlen, dhash_t hash, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 4)) int (DCALL DeeType_SetBasicCachedClassAttrStringHash)(DeeTypeObject *tp_self, char const *__restrict attr, dhash_t hash, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 5)) int (DCALL DeeType_SetBasicCachedClassAttrStringLenHash)(DeeTypeObject *tp_self, char const *__restrict attr, size_t attrlen, dhash_t hash, DeeObject *value);
//INTDEF WUNUSED NONNULL((1, 2, 4)) int (DCALL DeeType_SetBasicCachedInstanceAttrStringHash)(DeeTypeObject *tp_self, char const *__restrict attr, dhash_t hash, DeeObject *value);
//INTDEF WUNUSED NONNULL((1, 2, 5)) int (DCALL DeeType_SetBasicCachedInstanceAttrStringLenHash)(DeeTypeObject *tp_self, char const *__restrict attr, size_t attrlen, dhash_t hash, DeeObject *value);
#define DeeType_SetBasicCachedAttr(tp_self, self, attr, value)                     DeeType_SetBasicCachedAttrStringHash(tp_self, self, DeeString_STR(attr), DeeString_Hash(attr), value)
#define DeeType_SetBasicCachedAttrHash(tp_self, self, attr, hash, value)           DeeType_SetBasicCachedAttrStringHash(tp_self, self, DeeString_STR(attr), hash, value)
#define DeeType_SetBasicCachedAttrString(tp_self, self, attr, value)               DeeType_SetBasicCachedAttrStringHash(tp_self, self, attr, Dee_HashStr(attr), value)
#define DeeType_SetBasicCachedAttrStringLen(tp_self, self, attr, attrlen, value)   DeeType_SetBasicCachedAttrStringLenHash(tp_self, self, attr, attrlen, Dee_HashPtr(attr, attrlen), value)
#define DeeType_SetBasicCachedClassAttr(tp_self, attr, value)                      DeeType_SetBasicCachedClassAttrStringHash(tp_self, DeeString_STR(attr), DeeString_Hash(attr), value)
#define DeeType_SetBasicCachedClassAttrHash(tp_self, attr, hash, value)            DeeType_SetBasicCachedClassAttrStringHash(tp_self, DeeString_STR(attr), hash, value)
#define DeeType_SetBasicCachedClassAttrString(tp_self, attr, value)                DeeType_SetBasicCachedClassAttrStringHash(tp_self, attr, Dee_HashStr(attr), value)
#define DeeType_SetBasicCachedClassAttrStringLen(tp_self, attr, attrlen, value)    DeeType_SetBasicCachedClassAttrStringLenHash(tp_self, attr, attrlen, Dee_HashPtr(attr, attrlen), value)
//#define DeeType_SetBasicCachedInstanceAttr(tp_self, attr, value)                   DeeType_SetBasicCachedInstanceAttrStringHash(tp_self, DeeString_STR(attr), DeeString_Hash(attr), value)
//#define DeeType_SetBasicCachedInstanceAttrHash(tp_self, attr, hash, value)         DeeType_SetBasicCachedInstanceAttrStringHash(tp_self, DeeString_STR(attr), hash, value)
//#define DeeType_SetBasicCachedInstanceAttrString(tp_self, attr, value)             DeeType_SetBasicCachedInstanceAttrStringHash(tp_self, attr, Dee_HashStr(attr), value)
//#define DeeType_SetBasicCachedInstanceAttrStringLen(tp_self, attr, attrlen, value) DeeType_SetBasicCachedInstanceAttrStringLenHash(tp_self, attr, attrlen, Dee_HashPtr(attr, attrlen), value)

/* @return: * :        The returned value.
 * @return: NULL:      An error occurred.
 * @return: ITER_DONE: The attribute could not be found in the cache. */
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallCachedAttrStringHash)(DeeTypeObject *tp_self, DeeObject *self, char const *__restrict attr, dhash_t hash, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallCachedAttrStringLenHash)(DeeTypeObject *tp_self, DeeObject *self, char const *__restrict attr, size_t attrlen, dhash_t hash, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL DeeType_CallCachedClassAttrStringHash)(DeeTypeObject *tp_self, char const *__restrict attr, dhash_t hash, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL DeeType_CallCachedClassAttrStringLenHash)(DeeTypeObject *tp_self, char const *__restrict attr, size_t attrlen, dhash_t hash, size_t argc, DeeObject *const *argv);
#define DeeType_CallCachedInstanceAttrStringHash(tp_self, attr, hash, argc, argv) DeeType_CallCachedInstanceAttrStringHashKw(tp_self, attr, hash, argc, argv, NULL)
#define DeeType_CallCachedInstanceAttrStringLenHash(tp_self, attr, attrlen, hash, argc, argv) DeeType_CallCachedInstanceAttrStringLenHashKw(tp_self, attr, attrlen, hash, argc, argv, NULL)
//INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL DeeType_CallCachedInstanceAttrStringHash)(DeeTypeObject *tp_self, char const *__restrict attr, dhash_t hash, size_t argc, DeeObject *const *argv);
//INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL DeeType_CallCachedInstanceAttrStringLenHash)(DeeTypeObject *tp_self, char const *__restrict attr, size_t attrlen, dhash_t hash, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallCachedAttrStringHashKw)(DeeTypeObject *tp_self, DeeObject *self, char const *__restrict attr, dhash_t hash, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallCachedAttrStringLenHashKw)(DeeTypeObject *tp_self, DeeObject *self, char const *__restrict attr, size_t attrlen, dhash_t hash, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL DeeType_CallCachedClassAttrStringHashKw)(DeeTypeObject *tp_self, char const *__restrict attr, dhash_t hash, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL DeeType_CallCachedClassAttrStringLenHashKw)(DeeTypeObject *tp_self, char const *__restrict attr, size_t attrlen, dhash_t hash, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL DeeType_CallCachedInstanceAttrStringHashKw)(DeeTypeObject *tp_self, char const *__restrict attr, dhash_t hash, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL DeeType_CallCachedInstanceAttrStringLenHashKw)(DeeTypeObject *tp_self, char const *__restrict attr, size_t attrlen, dhash_t hash, size_t argc, DeeObject *const *argv, DeeObject *kw);
#define DeeType_CallCachedAttr(tp_self, self, attr, argc, argv)                           DeeType_CallCachedAttrStringHash(tp_self, self, DeeString_STR(attr), DeeString_Hash(attr), argc, argv)
#define DeeType_CallCachedAttrHash(tp_self, self, attr, hash, argc, argv)                 DeeType_CallCachedAttrStringHash(tp_self, self, DeeString_STR(attr), hash, argc, argv)
#define DeeType_CallCachedAttrString(tp_self, self, attr, argc, argv)                     DeeType_CallCachedAttrStringHash(tp_self, self, attr, Dee_HashStr(attr), argc, argv)
#define DeeType_CallCachedAttrStringLen(tp_self, self, attr, attrlen, argc, argv)         DeeType_CallCachedAttrStringLenHash(tp_self, self, attr, attrlen, Dee_HashPtr(attr, attrlen), argc, argv)
#define DeeType_CallCachedClassAttr(tp_self, attr, argc, argv)                            DeeType_CallCachedClassAttrStringHash(tp_self, DeeString_STR(attr), DeeString_Hash(attr), argc, argv)
#define DeeType_CallCachedClassAttrHash(tp_self, attr, hash, argc, argv)                  DeeType_CallCachedClassAttrStringHash(tp_self, DeeString_STR(attr), hash, argc, argv)
#define DeeType_CallCachedClassAttrString(tp_self, attr, argc, argv)                      DeeType_CallCachedClassAttrStringHash(tp_self, attr, Dee_HashStr(attr), argc, argv)
#define DeeType_CallCachedClassAttrStringLen(tp_self, attr, attrlen, argc, argv)          DeeType_CallCachedClassAttrStringLenHash(tp_self, attr, attrlen, Dee_HashPtr(attr, attrlen), argc, argv)
#define DeeType_CallCachedInstanceAttr(tp_self, attr, argc, argv)                         DeeType_CallCachedInstanceAttrStringHash(tp_self, DeeString_STR(attr), DeeString_Hash(attr), argc, argv)
#define DeeType_CallCachedInstanceAttrHash(tp_self, attr, hash, argc, argv)               DeeType_CallCachedInstanceAttrStringHash(tp_self, DeeString_STR(attr), hash, argc, argv)
#define DeeType_CallCachedInstanceAttrString(tp_self, attr, argc, argv)                   DeeType_CallCachedInstanceAttrStringHash(tp_self, attr, Dee_HashStr(attr), argc, argv)
#define DeeType_CallCachedInstanceAttrStringLen(tp_self, attr, attrlen, argc, argv)       DeeType_CallCachedInstanceAttrStringLenHash(tp_self, attr, attrlen, Dee_HashPtr(attr, attrlen), argc, argv)
#define DeeType_CallCachedAttrKw(tp_self, self, attr, argc, argv, kw)                     DeeType_CallCachedAttrStringHashKw(tp_self, self, DeeString_STR(attr), DeeString_Hash(attr), argc, argv, kw)
#define DeeType_CallCachedAttrHashKw(tp_self, self, attr, hash, argc, argv, kw)           DeeType_CallCachedAttrStringHashKw(tp_self, self, DeeString_STR(attr), hash, argc, argv, kw)
#define DeeType_CallCachedAttrStringKw(tp_self, self, attr, argc, argv, kw)               DeeType_CallCachedAttrStringHashKw(tp_self, self, attr, Dee_HashStr(attr), argc, argv, kw)
#define DeeType_CallCachedAttrStringLenKw(tp_self, self, attr, attrlen, argc, argv, kw)   DeeType_CallCachedAttrStringLenHashKw(tp_self, self, attr, attrlen, Dee_HashPtr(attr, attrlen), argc, argv, kw)
#define DeeType_CallCachedClassAttrKw(tp_self, attr, argc, argv, kw)                      DeeType_CallCachedClassAttrStringHashKw(tp_self, DeeString_STR(attr), DeeString_Hash(attr), argc, argv, kw)
#define DeeType_CallCachedClassAttrHashKw(tp_self, attr, hash, argc, argv, kw)            DeeType_CallCachedClassAttrStringHashKw(tp_self, DeeString_STR(attr), hash, argc, argv, kw)
#define DeeType_CallCachedClassAttrStringKw(tp_self, attr, argc, argv, kw)                DeeType_CallCachedClassAttrStringHashKw(tp_self, attr, Dee_HashStr(attr), argc, argv, kw)
#define DeeType_CallCachedClassAttrStringLenKw(tp_self, attr, attrlen, argc, argv, kw)    DeeType_CallCachedClassAttrStringLenHashKw(tp_self, attr, attrlen, Dee_HashPtr(attr, attrlen), argc, argv, kw)
#define DeeType_CallCachedInstanceAttrKw(tp_self, attr, argc, argv, kw)                   DeeType_CallCachedInstanceAttrStringHashKw(tp_self, DeeString_STR(attr), DeeString_Hash(attr), argc, argv, kw)
#define DeeType_CallCachedInstanceAttrHashKw(tp_self, attr, hash, argc, argv, kw)         DeeType_CallCachedInstanceAttrStringHashKw(tp_self, DeeString_STR(attr), hash, argc, argv, kw)
#define DeeType_CallCachedInstanceAttrStringKw(tp_self, attr, argc, argv, kw)             DeeType_CallCachedInstanceAttrStringHashKw(tp_self, attr, Dee_HashStr(attr), argc, argv, kw)
#define DeeType_CallCachedInstanceAttrStringLenKw(tp_self, attr, attrlen, argc, argv, kw) DeeType_CallCachedInstanceAttrStringLenHashKw(tp_self, attr, attrlen, Dee_HashPtr(attr, attrlen), argc, argv, kw)


#ifdef CONFIG_CALLTUPLE_OPTIMIZATIONS
/* @return: * :        The returned value.
 * @return: NULL:      An error occurred.
 * @return: ITER_DONE: The attribute could not be found in the cache. */
INTDEF WUNUSED NONNULL((1, 2, 3, 5)) DREF DeeObject *(DCALL DeeType_CallCachedAttrStringHashTuple)(DeeTypeObject *tp_self, DeeObject *self, char const *__restrict attr, dhash_t hash, DeeObject *args);
INTDEF WUNUSED NONNULL((1, 2, 4)) DREF DeeObject *(DCALL DeeType_CallCachedClassAttrStringHashTuple)(DeeTypeObject *tp_self, char const *__restrict attr, dhash_t hash, DeeObject *args);
//INTDEF WUNUSED NONNULL((1, 2, 4)) DREF DeeObject *(DCALL DeeType_CallCachedInstanceAttrStringHashTuple)(DeeTypeObject *tp_self, char const *__restrict attr, dhash_t hash, DeeObject *args);
INTDEF WUNUSED NONNULL((1, 2, 3, 5)) DREF DeeObject *(DCALL DeeType_CallCachedAttrStringHashTupleKw)(DeeTypeObject *tp_self, DeeObject *self, char const *__restrict attr, dhash_t hash, DeeObject *args, DeeObject *kw);
INTDEF WUNUSED NONNULL((1, 2, 4)) DREF DeeObject *(DCALL DeeType_CallCachedClassAttrStringHashTupleKw)(DeeTypeObject *tp_self, char const *__restrict attr, dhash_t hash, DeeObject *args, DeeObject *kw);
//INTDEF WUNUSED NONNULL((1, 2, 4)) DREF DeeObject *(DCALL DeeType_CallCachedInstanceAttrStringHashTupleKw)(DeeTypeObject *tp_self, char const *__restrict attr, dhash_t hash, DeeObject *args, DeeObject *kw);
//INTDEF WUNUSED NONNULL((1, 2, 3, 6)) DREF DeeObject *(DCALL DeeType_CallCachedAttrStringLenHashTuple)(DeeTypeObject *tp_self, DeeObject *self, char const *__restrict attr, size_t attrlen, dhash_t hash, DeeObject *args);
//INTDEF WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *(DCALL DeeType_CallCachedClassAttrStringLenHashTuple)(DeeTypeObject *tp_self, char const *__restrict attr, size_t attrlen, dhash_t hash, DeeObject *args);
//INTDEF WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *(DCALL DeeType_CallCachedInstanceAttrStringLenHashTuple)(DeeTypeObject *tp_self, char const *__restrict attr, size_t attrlen, dhash_t hash, DeeObject *args);
//INTDEF WUNUSED NONNULL((1, 2, 3, 6)) DREF DeeObject *(DCALL DeeType_CallCachedAttrStringLenHashTupleKw)(DeeTypeObject *tp_self, DeeObject *self, char const *__restrict attr, size_t attrlen, dhash_t hash, DeeObject *args, DeeObject *kw);
//INTDEF WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *(DCALL DeeType_CallCachedClassAttrStringLenHashTupleKw)(DeeTypeObject *tp_self, char const *__restrict attr, size_t attrlen, dhash_t hash, DeeObject *args, DeeObject *kw);
//INTDEF WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *(DCALL DeeType_CallCachedInstanceAttrStringLenHashTupleKw)(DeeTypeObject *tp_self, char const *__restrict attr, size_t attrlen, dhash_t hash, DeeObject *args, DeeObject *kw);

#define DeeType_CallCachedAttrTuple(tp_self, self, attr, args)                           DeeType_CallCachedAttrStringHashTuple(tp_self, self, DeeString_STR(attr), DeeString_Hash(attr), args)
#define DeeType_CallCachedAttrHashTuple(tp_self, self, attr, hash, args)                 DeeType_CallCachedAttrStringHashTuple(tp_self, self, DeeString_STR(attr), hash, args)
#define DeeType_CallCachedAttrStringTuple(tp_self, self, attr, args)                     DeeType_CallCachedAttrStringHashTuple(tp_self, self, attr, Dee_HashStr(attr), args)
//#define DeeType_CallCachedAttrStringLenTuple(tp_self, self, attr, attrlen, args)         DeeType_CallCachedAttrStringLenHashTuple(tp_self, self, attr, attrlen, Dee_HashPtr(attr, attrlen), args)
#define DeeType_CallCachedClassAttrTuple(tp_self, attr, args)                            DeeType_CallCachedClassAttrStringHashTuple(tp_self, DeeString_STR(attr), DeeString_Hash(attr), args)
#define DeeType_CallCachedClassAttrHashTuple(tp_self, attr, hash, args)                  DeeType_CallCachedClassAttrStringHashTuple(tp_self, DeeString_STR(attr), hash, args)
#define DeeType_CallCachedClassAttrStringTuple(tp_self, attr, args)                      DeeType_CallCachedClassAttrStringHashTuple(tp_self, attr, Dee_HashStr(attr), args)
//#define DeeType_CallCachedClassAttrStringLenTuple(tp_self, attr, attrlen, args)          DeeType_CallCachedClassAttrStringLenHashTuple(tp_self, attr, attrlen, Dee_HashPtr(attr, attrlen), args)
//#define DeeType_CallCachedInstanceAttrTuple(tp_self, attr, args)                         DeeType_CallCachedInstanceAttrStringHashTuple(tp_self, DeeString_STR(attr), DeeString_Hash(attr), args)
//#define DeeType_CallCachedInstanceAttrHashTuple(tp_self, attr, hash, args)               DeeType_CallCachedInstanceAttrStringHashTuple(tp_self, DeeString_STR(attr), hash, args)
//#define DeeType_CallCachedInstanceAttrStringTuple(tp_self, attr, args)                   DeeType_CallCachedInstanceAttrStringHashTuple(tp_self, attr, Dee_HashStr(attr), args)
//#define DeeType_CallCachedInstanceAttrStringLenTuple(tp_self, attr, attrlen, args)       DeeType_CallCachedInstanceAttrStringLenHashTuple(tp_self, attr, attrlen, Dee_HashPtr(attr, attrlen), args)
#define DeeType_CallCachedAttrTupleKw(tp_self, self, attr, args, kw)                     DeeType_CallCachedAttrStringHashTupleKw(tp_self, self, DeeString_STR(attr), DeeString_Hash(attr), args, kw)
#define DeeType_CallCachedAttrHashTupleKw(tp_self, self, attr, hash, args, kw)           DeeType_CallCachedAttrStringHashTupleKw(tp_self, self, DeeString_STR(attr), hash, args, kw)
#define DeeType_CallCachedAttrStringTupleKw(tp_self, self, attr, args, kw)               DeeType_CallCachedAttrStringHashTupleKw(tp_self, self, attr, Dee_HashStr(attr), args, kw)
//#define DeeType_CallCachedAttrStringLenTupleKw(tp_self, self, attr, attrlen, args, kw)   DeeType_CallCachedAttrStringLenHashTupleKw(tp_self, self, attr, attrlen, Dee_HashPtr(attr, attrlen), args, kw)
#define DeeType_CallCachedClassAttrTupleKw(tp_self, attr, args, kw)                      DeeType_CallCachedClassAttrStringHashTupleKw(tp_self, DeeString_STR(attr), DeeString_Hash(attr), args, kw)
#define DeeType_CallCachedClassAttrHashTupleKw(tp_self, attr, hash, args, kw)            DeeType_CallCachedClassAttrStringHashTupleKw(tp_self, DeeString_STR(attr), hash, args, kw)
#define DeeType_CallCachedClassAttrStringTupleKw(tp_self, attr, args, kw)                DeeType_CallCachedClassAttrStringHashTupleKw(tp_self, attr, Dee_HashStr(attr), args, kw)
//#define DeeType_CallCachedClassAttrStringLenTupleKw(tp_self, attr, attrlen, args, kw)    DeeType_CallCachedClassAttrStringLenHashTupleKw(tp_self, attr, attrlen, Dee_HashPtr(attr, attrlen), args, kw)
//#define DeeType_CallCachedInstanceAttrTupleKw(tp_self, attr, args, kw)                   DeeType_CallCachedInstanceAttrStringHashTupleKw(tp_self, DeeString_STR(attr), DeeString_Hash(attr), args, kw)
//#define DeeType_CallCachedInstanceAttrHashTupleKw(tp_self, attr, hash, args, kw)         DeeType_CallCachedInstanceAttrStringHashTupleKw(tp_self, DeeString_STR(attr), hash, args, kw)
//#define DeeType_CallCachedInstanceAttrStringTupleKw(tp_self, attr, args, kw)             DeeType_CallCachedInstanceAttrStringHashTupleKw(tp_self, attr, Dee_HashStr(attr), args, kw)
//#define DeeType_CallCachedInstanceAttrStringLenTupleKw(tp_self, attr, attrlen, args, kw) DeeType_CallCachedInstanceAttrStringLenHashTupleKw(tp_self, attr, attrlen, Dee_HashPtr(attr, attrlen), args, kw)
#endif /* CONFIG_CALLTUPLE_OPTIMIZATIONS */

/* @return: * :        The returned value.
 * @return: NULL:      An error occurred.
 * @return: ITER_DONE: The attribute could not be found in the cache. */
INTDEF WUNUSED NONNULL((1, 2, 3, 5)) DREF DeeObject *(DCALL DeeType_VCallCachedAttrStringHashf)(DeeTypeObject *tp_self, DeeObject *self, char const *__restrict attr, dhash_t hash, char const *__restrict format, va_list args);
INTDEF WUNUSED NONNULL((1, 2, 4)) DREF DeeObject *(DCALL DeeType_VCallCachedClassAttrStringHashf)(DeeTypeObject *tp_self, char const *__restrict attr, dhash_t hash, char const *__restrict format, va_list args);
INTDEF WUNUSED NONNULL((1, 2, 4)) DREF DeeObject *(DCALL DeeType_VCallCachedInstanceAttrStringHashf)(DeeTypeObject *__restrict tp_self, char const *__restrict attr, dhash_t hash, char const *__restrict format, va_list args);
//INTDEF WUNUSED NONNULL((1, 2, 3, 6)) DREF DeeObject *(DCALL DeeType_VCallCachedAttrStringLenHashf)(DeeTypeObject *tp_self, DeeObject *self, char const *__restrict attr, size_t attrlen, dhash_t hash, char const *__restrict format, va_list args);
//INTDEF WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *(DCALL DeeType_VCallCachedClassAttrStringLenHashf)(DeeTypeObject *tp_self, char const *__restrict attr, size_t attrlen, dhash_t hash, char const *__restrict format, va_list args);
//INTDEF WUNUSED NONNULL((1, 2, 5)) DREF DeeObject *(DCALL DeeType_VCallCachedInstanceAttrStringLenHashf)(DeeTypeObject *__restrict tp_self, char const *__restrict attr, size_t attrlen, dhash_t hash, char const *__restrict format, va_list args);
#define DeeType_VCallCachedAttrf(tp_self, self, attr, format, args)                           DeeType_VCallCachedAttrStringHashf(tp_self, self, DeeString_STR(attr), DeeString_Hash(attr), format, args)
#define DeeType_VCallCachedAttrHashf(tp_self, self, attr, hash, format, args)                 DeeType_VCallCachedAttrStringHashf(tp_self, self, DeeString_STR(attr), hash, format, args)
#define DeeType_VCallCachedAttrStringf(tp_self, self, attr, format, args)                     DeeType_VCallCachedAttrStringHashf(tp_self, self, attr, Dee_HashStr(attr), format, args)
//#define DeeType_VCallCachedAttrStringLenf(tp_self, self, attr, attrlen, format, args)         DeeType_VCallCachedAttrStringLenHashf(tp_self, self, attr, attrlen, Dee_HashPtr(attr, attrlen), format, args)
#define DeeType_VCallCachedClassAttrf(tp_self, attr, format, args)                            DeeType_VCallCachedClassAttrStringHashf(tp_self, DeeString_STR(attr), DeeString_Hash(attr), format, args)
#define DeeType_VCallCachedClassAttrHashf(tp_self, attr, hash, format, args)                  DeeType_VCallCachedClassAttrStringHashf(tp_self, DeeString_STR(attr), hash, format, args)
#define DeeType_VCallCachedClassAttrStringf(tp_self, attr, format, args)                      DeeType_VCallCachedClassAttrStringHashf(tp_self, attr, Dee_HashStr(attr), format, args)
//#define DeeType_VCallCachedClassAttrStringLenf(tp_self, attr, attrlen, format, args)          DeeType_VCallCachedClassAttrStringLenHashf(tp_self, attr, attrlen, Dee_HashPtr(attr, attrlen), format, args)
#define DeeType_VCallCachedInstanceAttrf(tp_self, attr, format, args)                         DeeType_VCallCachedInstanceAttrStringHashf(tp_self, DeeString_STR(attr), DeeString_Hash(attr), format, args)
#define DeeType_VCallCachedInstanceAttrHashf(tp_self, attr, hash, format, args)               DeeType_VCallCachedInstanceAttrStringHashf(tp_self, DeeString_STR(attr), hash, format, args)
#define DeeType_VCallCachedInstanceAttrStringf(tp_self, attr, format, args)                   DeeType_VCallCachedInstanceAttrStringHashf(tp_self, attr, Dee_HashStr(attr), format, args)
//#define DeeType_VCallCachedInstanceAttrStringLenf(tp_self, attr, attrlen, format, args)       DeeType_VCallCachedInstanceAttrStringLenHashf(tp_self, attr, attrlen, Dee_HashPtr(attr, attrlen), format, args)


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
 *  - DeeType_GetInstanceMethodAttrStringHash  --   class -> instance     (string.lower)                    (cache in `tp_class_cache' as `MEMBERCACHE_INSTANCE_METHOD')
 *  - DeeType_GetIInstanceMethodAttrStringHash --   class -> instance     (string.getinstanceattr("lower")) (cache in `tp_class' as `MEMBERCACHE_METHOD')
 */

/* Query user-class attributes, and cache them if some were found!
 * @return: * :   The attribute of the user-defined class.
 * @return: NULL: The attribute could not be found. */
#ifdef __INTELLISENSE__
INTDEF WUNUSED NONNULL((1, 2, 3)) struct Dee_class_attribute *(DCALL DeeType_QueryAttribute)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, /*String*/ DeeObject *attr);
INTDEF WUNUSED NONNULL((1, 2, 3)) struct Dee_class_attribute *(DCALL DeeType_QueryAttributeHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, /*String*/ DeeObject *attr, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) struct Dee_class_attribute *(DCALL DeeType_QueryAttributeString)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr);
INTDEF WUNUSED NONNULL((1, 2, 3)) struct Dee_class_attribute *(DCALL DeeType_QueryAttributeStringLen)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, size_t namelen);
INTDEF WUNUSED NONNULL((1, 2, 3)) struct Dee_class_attribute *(DCALL DeeType_QueryAttributeStringHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) struct Dee_class_attribute *(DCALL DeeType_QueryAttributeStringLenHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, size_t namelen, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) struct Dee_class_attribute *(DCALL DeeType_QueryClassAttribute)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, /*String*/ DeeObject *attr);
INTDEF WUNUSED NONNULL((1, 2, 3)) struct Dee_class_attribute *(DCALL DeeType_QueryClassAttributeHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, /*String*/ DeeObject *attr, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) struct Dee_class_attribute *(DCALL DeeType_QueryClassAttributeString)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr);
INTDEF WUNUSED NONNULL((1, 2, 3)) struct Dee_class_attribute *(DCALL DeeType_QueryClassAttributeStringLen)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, size_t namelen);
INTDEF WUNUSED NONNULL((1, 2, 3)) struct Dee_class_attribute *(DCALL DeeType_QueryClassAttributeStringHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) struct Dee_class_attribute *(DCALL DeeType_QueryClassAttributeStringLenHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, size_t namelen, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) struct Dee_class_attribute *(DCALL DeeType_QueryInstanceAttribute)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, /*String*/ DeeObject *attr);
INTDEF WUNUSED NONNULL((1, 2, 3)) struct Dee_class_attribute *(DCALL DeeType_QueryInstanceAttributeHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, /*String*/ DeeObject *attr, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) struct Dee_class_attribute *(DCALL DeeType_QueryInstanceAttributeString)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr);
INTDEF WUNUSED NONNULL((1, 2, 3)) struct Dee_class_attribute *(DCALL DeeType_QueryInstanceAttributeStringLen)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, size_t namelen);
INTDEF WUNUSED NONNULL((1, 2, 3)) struct Dee_class_attribute *(DCALL DeeType_QueryInstanceAttributeStringHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) struct Dee_class_attribute *(DCALL DeeType_QueryInstanceAttributeStringLenHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, size_t namelen, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) struct Dee_class_attribute *(DCALL DeeType_QueryIInstanceAttribute)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, /*String*/ DeeObject *attr);
INTDEF WUNUSED NONNULL((1, 2, 3)) struct Dee_class_attribute *(DCALL DeeType_QueryIInstanceAttributeHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, /*String*/ DeeObject *attr, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) struct Dee_class_attribute *(DCALL DeeType_QueryIInstanceAttributeString)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr);
INTDEF WUNUSED NONNULL((1, 2, 3)) struct Dee_class_attribute *(DCALL DeeType_QueryIInstanceAttributeStringLen)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, size_t namelen);
INTDEF WUNUSED NONNULL((1, 2, 3)) struct Dee_class_attribute *(DCALL DeeType_QueryIInstanceAttributeStringHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) struct Dee_class_attribute *(DCALL DeeType_QueryIInstanceAttributeStringLenHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, size_t namelen, dhash_t hash);
#else /* __INTELLISENSE__ */
INTDEF WUNUSED NONNULL((1, 2, 3)) struct Dee_class_attribute *(DCALL DeeType_QueryAttributeHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, /*String*/ DeeObject *attr, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) struct Dee_class_attribute *(DCALL DeeType_QueryAttributeStringHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) struct Dee_class_attribute *(DCALL DeeType_QueryAttributeStringLenHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, size_t namelen, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) struct Dee_class_attribute *(DCALL DeeType_QueryClassAttributeHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, /*String*/ DeeObject *attr, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) struct Dee_class_attribute *(DCALL DeeType_QueryClassAttributeStringHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) struct Dee_class_attribute *(DCALL DeeType_QueryClassAttributeStringLenHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, size_t namelen, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) struct Dee_class_attribute *(DCALL DeeType_QueryInstanceAttributeHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, /*String*/ DeeObject *attr, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) struct Dee_class_attribute *(DCALL DeeType_QueryInstanceAttributeStringHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) struct Dee_class_attribute *(DCALL DeeType_QueryInstanceAttributeStringLenHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, size_t namelen, dhash_t hash);
#define DeeType_QueryIInstanceAttributeHash(tp_invoker, tp_self, attr, hash)                   DeeType_QueryInstanceAttributeHash(tp_invoker, tp_self, attr, hash)
#define DeeType_QueryIInstanceAttributeStringHash(tp_invoker, tp_self, attr, hash)             DeeType_QueryInstanceAttributeStringHash(tp_invoker, tp_self, attr, hash)
#define DeeType_QueryIInstanceAttributeStringLenHash(tp_invoker, tp_self, attr, namelen, hash) DeeType_QueryInstanceAttributeStringLenHash(tp_invoker, tp_self, attr, namelen, hash)
#define DeeType_QueryAttribute(tp_invoker, tp_self, attr)                                      DeeType_QueryAttributeHash(tp_invoker, tp_self, attr, DeeString_Hash(attr))
#define DeeType_QueryAttributeString(tp_invoker, tp_self, attr)                                DeeType_QueryAttributeStringHash(tp_invoker, tp_self, attr, Dee_HashStr(attr))
#define DeeType_QueryAttributeStringLen(tp_invoker, tp_self, attr, namelen)                    DeeType_QueryAttributeStringLenHash(tp_invoker, tp_self, attr, namelen, Dee_HashPtr(attr, namelen))
#define DeeType_QueryClassAttribute(tp_invoker, tp_self, attr)                                 DeeType_QueryClassAttributeHash(tp_invoker, tp_self, attr, DeeString_Hash(attr))
#define DeeType_QueryClassAttributeString(tp_invoker, tp_self, attr)                           DeeType_QueryClassAttributeStringHash(tp_invoker, tp_self, attr, Dee_HashStr(attr))
#define DeeType_QueryClassAttributeStringLen(tp_invoker, tp_self, attr, namelen)               DeeType_QueryClassAttributeStringLenHash(tp_invoker, tp_self, attr, namelen, Dee_HashPtr(attr, namelen))
#define DeeType_QueryInstanceAttribute(tp_invoker, tp_self, attr)                              DeeType_QueryInstanceAttributeHash(tp_invoker, tp_self, attr, DeeString_Hash(attr))
#define DeeType_QueryInstanceAttributeString(tp_invoker, tp_self, attr)                        DeeType_QueryInstanceAttributeStringHash(tp_invoker, tp_self, attr, Dee_HashStr(attr))
#define DeeType_QueryInstanceAttributeStringLen(tp_invoker, tp_self, attr, namelen)            DeeType_QueryInstanceAttributeStringLenHash(tp_invoker, tp_self, attr, namelen, Dee_HashPtr(attr, namelen))
#define DeeType_QueryIInstanceAttribute(tp_invoker, tp_self, attr)                             DeeType_QueryIInstanceAttributeHash(tp_invoker, tp_self, attr, DeeString_Hash(attr))
#define DeeType_QueryIInstanceAttributeString(tp_invoker, tp_self, attr)                       DeeType_QueryIInstanceAttributeStringHash(tp_invoker, tp_self, attr, Dee_HashStr(attr))
#define DeeType_QueryIInstanceAttributeStringLen(tp_invoker, tp_self, attr, namelen)           DeeType_QueryIInstanceAttributeStringLenHash(tp_invoker, tp_self, attr, namelen, Dee_HashPtr(attr, namelen))
#endif /* !__INTELLISENSE__ */

/* Get attributes from `tp_self->tp_methods' / `tp_self->tp_class_methods'.
 * @return: * :        The attribute value.
 * @return: NULL:      An error occurred.
 * @return: ITER_DONE: The attribute could not be found in `chain'. */
#ifdef __INTELLISENSE__
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) DREF DeeObject *(DCALL DeeType_GetMethodAttrStringHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, DeeObject *self, char const *__restrict attr, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) DREF DeeObject *(DCALL DeeType_GetMethodAttrStringLenHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, DeeObject *self, char const *__restrict attr, size_t attrlen, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_GetClassMethodAttrStringHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_GetClassMethodAttrStringLenHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, size_t attrlen, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_GetInstanceMethodAttrStringHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_GetInstanceMethodAttrStringLenHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, size_t attrlen, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_GetIInstanceMethodAttrStringHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_GetIInstanceMethodAttrStringLenHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, size_t attrlen, dhash_t hash);
#else /* __INTELLISENSE__ */
INTDEF NONNULL((1, 2, 3, 4, 5)) DREF DeeObject *DCALL /* GET_METHOD */
type_method_getattr_string_hash(struct Dee_membercache *cache, DeeTypeObject *decl,
                                struct type_method const *chain, DeeObject *self,
                                char const *__restrict attr, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3, 4, 5)) DREF DeeObject *DCALL /* GET_METHOD */
type_method_getattr_string_len_hash(struct Dee_membercache *cache, DeeTypeObject *decl,
                                    struct type_method const *chain, DeeObject *self,
                                    char const *__restrict attr, size_t attrlen, dhash_t hash);
#define DeeType_GetMethodAttrStringHash(tp_invoker, tp_self, self, attr, hash)             type_method_getattr_string_hash(&(tp_invoker)->tp_cache, tp_self, (tp_self)->tp_methods, self, attr, hash)
#define DeeType_GetMethodAttrStringLenHash(tp_invoker, tp_self, self, attr, attrlen, hash) type_method_getattr_string_len_hash(&(tp_invoker)->tp_cache, tp_self, (tp_self)->tp_methods, self, attr, attrlen, hash)
#define DeeType_GetClassMethodAttrStringHash(tp_invoker, tp_self, attr, hash)              type_method_getattr_string_hash(&(tp_invoker)->tp_class_cache, tp_self, (tp_self)->tp_class_methods, (DeeObject *)(tp_invoker), attr, hash)
#define DeeType_GetClassMethodAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, hash)  type_method_getattr_string_len_hash(&(tp_invoker)->tp_class_cache, tp_self, (tp_self)->tp_class_methods, (DeeObject *)(tp_invoker), attr, attrlen, hash)
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_GetInstanceMethodAttrStringHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_GetInstanceMethodAttrStringLenHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, size_t attrlen, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_GetIInstanceMethodAttrStringHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_GetIInstanceMethodAttrStringLenHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, size_t attrlen, dhash_t hash);
#endif /* !__INTELLISENSE__ */
#define DeeType_GetMethodAttr(tp_invoker, tp_self, self, attr)              DeeType_GetMethodAttrStringHash(tp_invoker, tp_self, self, DeeString_STR(attr), DeeString_Has(attr))
#define DeeType_GetMethodAttrHash(tp_invoker, tp_self, self, attr, hash)    DeeType_GetMethodAttrStringHash(tp_invoker, tp_self, self, DeeString_STR(attr), hash)
#define DeeType_GetMethodAttrString(tp_invoker, tp_self, self, attr)        DeeType_GetMethodAttrStringHash(tp_invoker, tp_self, self, attr, Dee_HashStr(attr))
#define DeeType_GetClassMethodAttr(tp_invoker, tp_self, attr)               DeeType_GetClassMethodAttrStringHash(tp_invoker, tp_self, DeeString_STR(attr), DeeString_Has(attr))
#define DeeType_GetClassMethodAttrHash(tp_invoker, tp_self, attr, hash)     DeeType_GetClassMethodAttrStringHash(tp_invoker, tp_self, DeeString_STR(attr), hash)
#define DeeType_GetClassMethodAttrString(tp_invoker, tp_self, attr)         DeeType_GetClassMethodAttrStringHash(tp_invoker, tp_self, attr, Dee_HashStr(attr))
#define DeeType_GetInstanceMethodAttr(tp_invoker, tp_self, attr)            DeeType_GetInstanceMethodAttrStringHash(tp_invoker, tp_self, DeeString_STR(attr), DeeString_Has(attr))
#define DeeType_GetInstanceMethodAttrHash(tp_invoker, tp_self, attr, hash)  DeeType_GetInstanceMethodAttrStringHash(tp_invoker, tp_self, DeeString_STR(attr), hash)
#define DeeType_GetInstanceMethodAttrString(tp_invoker, tp_self, attr)      DeeType_GetInstanceMethodAttrStringHash(tp_invoker, tp_self, attr, Dee_HashStr(attr))
#define DeeType_GetIInstanceMethodAttr(tp_invoker, tp_self, attr)           DeeType_GetIInstanceMethodAttrStringHash(tp_invoker, tp_self, DeeString_STR(attr), DeeString_Has(attr))
#define DeeType_GetIInstanceMethodAttrHash(tp_invoker, tp_self, attr, hash) DeeType_GetIInstanceMethodAttrStringHash(tp_invoker, tp_self, DeeString_STR(attr), hash)
#define DeeType_GetIInstanceMethodAttrString(tp_invoker, tp_self, attr)     DeeType_GetIInstanceMethodAttrStringHash(tp_invoker, tp_self, attr, Dee_HashStr(attr))

/* Call attributes from `tp_self->tp_methods' / `tp_self->tp_class_methods'.
 * @return: * :        The attribute value.
 * @return: NULL:      An error occurred.
 * @return: ITER_DONE: The attribute could not be found in `chain'. */
#ifdef __INTELLISENSE__
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) DREF DeeObject *(DCALL DeeType_CallMethodAttrStringHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, DeeObject *self, char const *__restrict attr, dhash_t hash, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) DREF DeeObject *(DCALL DeeType_CallMethodAttrStringLenHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, DeeObject *self, char const *__restrict attr, size_t attrlen, dhash_t hash, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) DREF DeeObject *(DCALL DeeType_CallMethodAttrStringHashKw)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, DeeObject *self, char const *__restrict attr, dhash_t hash, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) DREF DeeObject *(DCALL DeeType_CallMethodAttrStringLenHashKw)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, DeeObject *self, char const *__restrict attr, size_t attrlen, dhash_t hash, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallClassMethodAttrStringHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, dhash_t hash, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallClassMethodAttrStringLenHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, size_t attrlen, dhash_t hash, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallClassMethodAttrStringHashKw)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, dhash_t hash, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallClassMethodAttrStringLenHashKw)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, size_t attrlen, dhash_t hash, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallInstanceMethodAttrStringHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, dhash_t hash, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallInstanceMethodAttrStringLenHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, size_t attrlen, dhash_t hash, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallInstanceMethodAttrStringHashKw)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, dhash_t hash, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallInstanceMethodAttrStringLenHashKw)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, size_t attrlen, dhash_t hash, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallIInstanceMethodAttrStringHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, dhash_t hash, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallIInstanceMethodAttrStringLenHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, size_t attrlen, dhash_t hash, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallIInstanceMethodAttrStringHashKw)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, dhash_t hash, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallIInstanceMethodAttrStringLenHashKw)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, size_t attrlen, dhash_t hash, size_t argc, DeeObject *const *argv, DeeObject *kw);
#ifdef CONFIG_CALLTUPLE_OPTIMIZATIONS
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) DREF DeeObject *(DCALL DeeType_CallMethodAttrStringHashTuple)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, DeeObject *self, char const *__restrict attr, dhash_t hash, DeeObject *args);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) DREF DeeObject *(DCALL DeeType_CallMethodAttrStringLenHashTuple)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, DeeObject *self, char const *__restrict attr, size_t attrlen, dhash_t hash, DeeObject *args);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) DREF DeeObject *(DCALL DeeType_CallMethodAttrStringHashTupleKw)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, DeeObject *self, char const *__restrict attr, dhash_t hash, DeeObject *args, DeeObject *kw);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) DREF DeeObject *(DCALL DeeType_CallMethodAttrStringLenHashTupleKw)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, DeeObject *self, char const *__restrict attr, size_t attrlen, dhash_t hash, DeeObject *args, DeeObject *kw);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallClassMethodAttrStringHashTuple)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, dhash_t hash, DeeObject *args);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallClassMethodAttrStringLenHashTuple)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, size_t attrlen, dhash_t hash, DeeObject *args);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallClassMethodAttrStringHashTupleKw)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, dhash_t hash, DeeObject *args, DeeObject *kw);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallClassMethodAttrStringLenHashTupleKw)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, size_t attrlen, dhash_t hash, DeeObject *args, DeeObject *kw);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallInstanceMethodAttrStringHashTuple)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, dhash_t hash, DeeObject *args);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallInstanceMethodAttrStringLenHashTuple)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, size_t attrlen, dhash_t hash, DeeObject *args);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallInstanceMethodAttrStringHashTupleKw)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, dhash_t hash, DeeObject *args, DeeObject *kw);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallInstanceMethodAttrStringLenHashTupleKw)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, size_t attrlen, dhash_t hash, DeeObject *args, DeeObject *kw);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallIInstanceMethodAttrStringHashTuple)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, dhash_t hash, DeeObject *args);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallIInstanceMethodAttrStringLenHashTuple)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, size_t attrlen, dhash_t hash, DeeObject *args);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallIInstanceMethodAttrStringHashTupleKw)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, dhash_t hash, DeeObject *args, DeeObject *kw);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallIInstanceMethodAttrStringLenHashTupleKw)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, size_t attrlen, dhash_t hash, DeeObject *args, DeeObject *kw);
#endif /* CONFIG_CALLTUPLE_OPTIMIZATIONS */
INTDEF WUNUSED NONNULL((1, 2, 3, 4, 6)) DREF DeeObject *(DCALL DeeType_VCallMethodAttrStringHashf)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, DeeObject *self, char const *__restrict attr, dhash_t hash, char const *__restrict format, va_list args);
//INTDEF WUNUSED NONNULL((1, 2, 3, 4, 7)) DREF DeeObject *(DCALL DeeType_VCallMethodAttrStringLenHashf)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, DeeObject *self, char const *__restrict attr, size_t attrlen, dhash_t hash, char const *__restrict format, va_list args);
INTDEF WUNUSED NONNULL((1, 2, 3, 5)) DREF DeeObject *(DCALL DeeType_VCallClassMethodAttrStringHashf)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, dhash_t hash, char const *__restrict format, va_list args);
//INTDEF WUNUSED NONNULL((1, 2, 3, 6)) DREF DeeObject *(DCALL DeeType_VCallClassMethodAttrStringLenHashf)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, size_t attrlen, dhash_t hash, char const *__restrict format, va_list args);
INTDEF WUNUSED NONNULL((1, 2, 3, 5)) DREF DeeObject *(DCALL DeeType_VCallInstanceMethodAttrStringHashf)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, dhash_t hash, char const *__restrict format, va_list args);
//INTDEF WUNUSED NONNULL((1, 2, 3, 6)) DREF DeeObject *(DCALL DeeType_VCallInstanceMethodAttrStringLenHashf)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, size_t attrlen, dhash_t hash, char const *__restrict format, va_list args);
//INTDEF WUNUSED NONNULL((1, 2, 3, 5)) DREF DeeObject *(DCALL DeeType_VCallIInstanceMethodAttrStringHashf)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, dhash_t hash, char const *__restrict format, va_list args);
//INTDEF WUNUSED NONNULL((1, 2, 3, 6)) DREF DeeObject *(DCALL DeeType_VCallIInstanceMethodAttrStringLenHashf)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, size_t attrlen, dhash_t hash, char const *__restrict format, va_list args);
#else /* __INTELLISENSE__ */
INTDEF WUNUSED NONNULL((1, 2, 3, 4, 5)) DREF DeeObject *DCALL /* CALL_METHOD */
type_method_callattr_string_hash(struct Dee_membercache *cache, DeeTypeObject *decl,
                                 struct type_method const *chain, DeeObject *self,
                                 char const *__restrict attr, dhash_t hash,
                                 size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1, 2, 3, 4, 5)) DREF DeeObject *DCALL /* CALL_METHOD */
type_method_callattr_string_len_hash(struct Dee_membercache *cache, DeeTypeObject *decl,
                                     struct type_method const *chain, DeeObject *self,
                                     char const *__restrict attr, size_t attrlen, dhash_t hash,
                                     size_t argc, DeeObject *const *argv);
#define DeeType_CallMethodAttrStringHash(tp_invoker, tp_self, self, attr, hash, argc, argv)             type_method_callattr_string_hash(&(tp_invoker)->tp_cache, tp_self, (tp_self)->tp_methods, self, attr, hash, argc, argv)
#define DeeType_CallClassMethodAttrStringHash(tp_invoker, tp_self, attr, hash, argc, argv)              type_method_callattr_string_hash(&(tp_invoker)->tp_class_cache, tp_self, (tp_self)->tp_class_methods, (DeeObject *)(tp_invoker), attr, hash, argc, argv)
#define DeeType_CallMethodAttrStringLenHash(tp_invoker, tp_self, self, attr, attrlen, hash, argc, argv) type_method_callattr_string_len_hash(&(tp_invoker)->tp_cache, tp_self, (tp_self)->tp_methods, self, attr, attrlen, hash, argc, argv)
#define DeeType_CallClassMethodAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, hash, argc, argv)  type_method_callattr_string_len_hash(&(tp_invoker)->tp_class_cache, tp_self, (tp_self)->tp_class_methods, (DeeObject *)(tp_invoker), attr, attrlen, hash, argc, argv)
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallInstanceMethodAttrStringHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, dhash_t hash, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallInstanceMethodAttrStringLenHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, size_t attrlen, dhash_t hash, size_t argc, DeeObject *const *argv);
#define DeeType_CallIInstanceMethodAttrStringHash(tp_invoker, tp_self, attr, hash, argc, argv) \
	DeeType_CallIInstanceMethodAttrStringHashKw(tp_invoker, tp_self, attr, hash, argc, argv, NULL)
#define DeeType_CallIInstanceMethodAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, hash, argc, argv) \
	DeeType_CallIInstanceMethodAttrStringLenHashKw(tp_invoker, tp_self, attr, attrlen, hash, argc, argv, NULL)

INTDEF WUNUSED NONNULL((1, 2, 3, 4, 5)) DREF DeeObject *DCALL /* CALL_METHOD_KW */
type_method_callattr_string_hash_kw(struct Dee_membercache *cache, DeeTypeObject *decl,
                                    struct type_method const *chain, DeeObject *self,
                                    char const *__restrict attr, dhash_t hash,
                                    size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1, 2, 3, 4, 5)) DREF DeeObject *DCALL /* CALL_METHOD_KW */
type_method_callattr_string_len_hash_kw(struct Dee_membercache *cache, DeeTypeObject *decl,
                                        struct type_method const *chain, DeeObject *self,
                                        char const *__restrict attr, size_t attrlen, dhash_t hash,
                                        size_t argc, DeeObject *const *argv, DeeObject *kw);
#define DeeType_CallMethodAttrStringHashKw(tp_invoker, tp_self, self, attr, hash, argc, argv, kw)             type_method_callattr_string_hash_kw(&(tp_invoker)->tp_cache, tp_self, (tp_self)->tp_methods, self, attr, hash, argc, argv, kw)
#define DeeType_CallClassMethodAttrStringHashKw(tp_invoker, tp_self, attr, hash, argc, argv, kw)              type_method_callattr_string_hash_kw(&(tp_invoker)->tp_class_cache, tp_self, (tp_self)->tp_class_methods, (DeeObject *)(tp_invoker), attr, hash, argc, argv, kw)
#define DeeType_CallMethodAttrStringLenHashKw(tp_invoker, tp_self, self, attr, attrlen, hash, argc, argv, kw) type_method_callattr_string_len_hash_kw(&(tp_invoker)->tp_cache, tp_self, (tp_self)->tp_methods, self, attr, attrlen, hash, argc, argv, kw)
#define DeeType_CallClassMethodAttrStringLenHashKw(tp_invoker, tp_self, attr, attrlen, hash, argc, argv, kw)  type_method_callattr_string_len_hash_kw(&(tp_invoker)->tp_class_cache, tp_self, (tp_self)->tp_class_methods, (DeeObject *)(tp_invoker), attr, attrlen, hash, argc, argv, kw)
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallInstanceMethodAttrStringHashKw)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, dhash_t hash, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallInstanceMethodAttrStringLenHashKw)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, size_t attrlen, dhash_t hash, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallIInstanceMethodAttrStringHashKw)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, dhash_t hash, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallIInstanceMethodAttrStringLenHashKw)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, size_t attrlen, dhash_t hash, size_t argc, DeeObject *const *argv, DeeObject *kw);

#ifdef CONFIG_CALLTUPLE_OPTIMIZATIONS
/* CALL_METHOD_TUPLE */
#define DeeType_CallMethodAttrStringHashTuple(tp_invoker, tp_self, self, attr, hash, args)                      DeeType_CallMethodAttrStringHash(tp_invoker, tp_self, self, attr, hash, DeeTuple_SIZE(args), DeeTuple_ELEM(args))
#define DeeType_CallClassMethodAttrStringHashTuple(tp_invoker, tp_self, attr, hash, args)                       DeeType_CallClassMethodAttrStringHash(tp_invoker, tp_self, attr, hash, DeeTuple_SIZE(args), DeeTuple_ELEM(args))
#define DeeType_CallInstanceMethodAttrStringHashTuple(tp_invoker, tp_self, attr, hash, args)                    DeeType_CallInstanceMethodAttrStringHash(tp_invoker, tp_self, attr, hash, DeeTuple_SIZE(args), DeeTuple_ELEM(args))
#define DeeType_CallIInstanceMethodAttrStringHashTuple(tp_invoker, tp_self, attr, hash, args)                   DeeType_CallIInstanceMethodAttrStringHash(tp_invoker, tp_self, attr, hash, DeeTuple_SIZE(args), DeeTuple_ELEM(args))
#define DeeType_CallMethodAttrStringHashTupleKw(tp_invoker, tp_self, self, attr, hash, args, kw)                DeeType_CallMethodAttrStringHashKw(tp_invoker, tp_self, self, attr, hash, DeeTuple_SIZE(args), DeeTuple_ELEM(args), kw)
#define DeeType_CallClassMethodAttrStringHashTupleKw(tp_invoker, tp_self, attr, hash, args, kw)                 DeeType_CallClassMethodAttrStringHashKw(tp_invoker, tp_self, attr, hash, DeeTuple_SIZE(args), DeeTuple_ELEM(args), kw)
#define DeeType_CallInstanceMethodAttrStringHashTupleKw(tp_invoker, tp_self, attr, hash, args, kw)              DeeType_CallInstanceMethodAttrStringHashKw(tp_invoker, tp_self, attr, hash, DeeTuple_SIZE(args), DeeTuple_ELEM(args), kw)
#define DeeType_CallIInstanceMethodAttrStringHashTupleKw(tp_invoker, tp_self, attr, hash, args, kw)             DeeType_CallIInstanceMethodAttrStringHashKw(tp_invoker, tp_self, attr, hash, DeeTuple_SIZE(args), DeeTuple_ELEM(args), kw)
#define DeeType_CallMethodAttrStringLenHashTuple(tp_invoker, tp_self, self, attr, attrlen, hash, args)          DeeType_CallMethodAttrStringLenHash(tp_invoker, tp_self, self, attr, attrlen, hash, DeeTuple_SIZE(args), DeeTuple_ELEM(args))
#define DeeType_CallClassMethodAttrStringLenHashTuple(tp_invoker, tp_self, attr, attrlen, hash, args)           DeeType_CallClassMethodAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, hash, DeeTuple_SIZE(args), DeeTuple_ELEM(args))
#define DeeType_CallInstanceMethodAttrStringLenHashTuple(tp_invoker, tp_self, attr, attrlen, hash, args)        DeeType_CallInstanceMethodAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, hash, DeeTuple_SIZE(args), DeeTuple_ELEM(args))
#define DeeType_CallIInstanceMethodAttrStringLenHashTuple(tp_invoker, tp_self, attr, attrlen, hash, args)       DeeType_CallIInstanceMethodAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, hash, DeeTuple_SIZE(args), DeeTuple_ELEM(args))
#define DeeType_CallMethodAttrStringLenHashTupleKw(tp_invoker, tp_self, self, attr, attrlen, hash, args, kw)    DeeType_CallMethodAttrStringLenHashKw(tp_invoker, tp_self, self, attr, attrlen, hash, DeeTuple_SIZE(args), DeeTuple_ELEM(args), kw)
#define DeeType_CallClassMethodAttrStringLenHashTupleKw(tp_invoker, tp_self, attr, attrlen, hash, args, kw)     DeeType_CallClassMethodAttrStringLenHashKw(tp_invoker, tp_self, attr, attrlen, hash, DeeTuple_SIZE(args), DeeTuple_ELEM(args), kw)
#define DeeType_CallInstanceMethodAttrStringLenHashTupleKw(tp_invoker, tp_self, attr, attrlen, hash, args, kw)  DeeType_CallInstanceMethodAttrStringLenHashKw(tp_invoker, tp_self, attr, attrlen, hash, DeeTuple_SIZE(args), DeeTuple_ELEM(args), kw)
#define DeeType_CallIInstanceMethodAttrStringLenHashTupleKw(tp_invoker, tp_self, attr, attrlen, hash, args, kw) DeeType_CallIInstanceMethodAttrStringLenHashKw(tp_invoker, tp_self, attr, attrlen, hash, DeeTuple_SIZE(args), DeeTuple_ELEM(args), kw)
#endif /* CONFIG_CALLTUPLE_OPTIMIZATIONS */

INTDEF WUNUSED NONNULL((1, 2, 3, 4, 5, 7)) DREF DeeObject *DCALL /* CALL_METHOD */
type_method_vcallattr_string_hashf(struct Dee_membercache *cache, DeeTypeObject *decl,
                                   struct type_method const *chain, DeeObject *self,
                                   char const *__restrict attr, dhash_t hash,
                                   char const *__restrict format, va_list args);
#define DeeType_VCallMethodAttrStringHashf(tp_invoker, tp_self, self, attr, hash, format, args) \
	type_method_vcallattr_string_hashf(&(tp_invoker)->tp_cache, tp_self, (tp_self)->tp_methods, self, attr, hash, format, args)
#define DeeType_VCallClassMethodAttrStringHashf(tp_invoker, tp_self, attr, hash, format, args) \
	type_method_vcallattr_string_hashf(&(tp_invoker)->tp_class_cache, tp_self, (tp_self)->tp_class_methods, (DeeObject *)(tp_invoker), attr, hash, format, args)
INTDEF WUNUSED NONNULL((1, 2, 3, 5)) DREF DeeObject *(DCALL DeeType_VCallInstanceMethodAttrStringHashf)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, dhash_t hash, char const *__restrict format, va_list args);
//INTDEF WUNUSED NONNULL((1, 2, 3, 5)) DREF DeeObject *(DCALL DeeType_VCallIInstanceMethodAttrStringHashf)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, dhash_t hash, char const *__restrict format, va_list args);
#endif /* !__INTELLISENSE__ */
#define DeeType_CallMethodAttr(tp_invoker, tp_self, self, attr, argc, argv)                            DeeType_CallMethodAttrStringHash(tp_invoker, tp_self, self, DeeString_STR(attr), DeeString_Hash(attr), argc, argv)
#define DeeType_CallMethodAttrHash(tp_invoker, tp_self, self, attr, hash, argc, argv)                  DeeType_CallMethodAttrStringHash(tp_invoker, tp_self, self, DeeString_STR(attr), hash, argc, argv)
#define DeeType_CallMethodAttrString(tp_invoker, tp_self, self, attr, argc, argv)                      DeeType_CallMethodAttrStringHash(tp_invoker, tp_self, self, attr, Dee_HashStr(attr), argc, argv)
#define DeeType_CallMethodAttrStringLen(tp_invoker, tp_self, self, attr, attrlen, argc, argv)          DeeType_CallMethodAttrStringLenHash(tp_invoker, tp_self, self, attr, attrlen, Dee_HashPtr(attr, attrlen), argc, argv)
#define DeeType_CallMethodAttrKw(tp_invoker, tp_self, self, attr, argc, argv, kw)                      DeeType_CallMethodAttrStringHashKw(tp_invoker, tp_self, self, DeeString_STR(attr), DeeString_Hash(attr), argc, argv, kw)
#define DeeType_CallMethodAttrHashKw(tp_invoker, tp_self, self, attr, hash, argc, argv, kw)            DeeType_CallMethodAttrStringHashKw(tp_invoker, tp_self, self, DeeString_STR(attr), hash, argc, argv, kw)
#define DeeType_CallMethodAttrStringKw(tp_invoker, tp_self, self, attr, argc, argv, kw)                DeeType_CallMethodAttrStringHashKw(tp_invoker, tp_self, self, attr, Dee_HashStr(attr), argc, argv, kw)
#define DeeType_CallMethodAttrStringLenKw(tp_invoker, tp_self, self, attr, attrlen, argc, argv, kw)    DeeType_CallMethodAttrStringLenHashKw(tp_invoker, tp_self, self, attr, attrlen, Dee_HashPtr(attr, attrlen), argc, argv, kw)
#define DeeType_CallClassMethodAttr(tp_invoker, tp_self, attr, argc, argv)                             DeeType_CallClassMethodAttrStringHash(tp_invoker, tp_self, DeeString_STR(attr), DeeString_Hash(attr), argc, argv)
#define DeeType_CallClassMethodAttrHash(tp_invoker, tp_self, attr, hash, argc, argv)                   DeeType_CallClassMethodAttrStringHash(tp_invoker, tp_self, DeeString_STR(attr), hash, argc, argv)
#define DeeType_CallClassMethodAttrString(tp_invoker, tp_self, attr, argc, argv)                       DeeType_CallClassMethodAttrStringHash(tp_invoker, tp_self, attr, Dee_HashStr(attr), argc, argv)
#define DeeType_CallClassMethodAttrStringLen(tp_invoker, tp_self, attr, attrlen, argc, argv)           DeeType_CallClassMethodAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, Dee_HashPtr(attr, attrlen), argc, argv)
#define DeeType_CallClassMethodAttrKw(tp_invoker, tp_self, attr, argc, argv, kw)                       DeeType_CallClassMethodAttrStringHashKw(tp_invoker, tp_self, DeeString_STR(attr), DeeString_Hash(attr), argc, argv, kw)
#define DeeType_CallClassMethodAttrHashKw(tp_invoker, tp_self, attr, hash, argc, argv, kw)             DeeType_CallClassMethodAttrStringHashKw(tp_invoker, tp_self, DeeString_STR(attr), hash, argc, argv, kw)
#define DeeType_CallClassMethodAttrStringKw(tp_invoker, tp_self, attr, argc, argv, kw)                 DeeType_CallClassMethodAttrStringHashKw(tp_invoker, tp_self, attr, Dee_HashStr(attr), argc, argv, kw)
#define DeeType_CallClassMethodAttrStringLenKw(tp_invoker, tp_self, attr, attrlen, argc, argv, kw)     DeeType_CallClassMethodAttrStringLenHashKw(tp_invoker, tp_self, attr, attrlen, Dee_HashPtr(attr, attrlen), argc, argv, kw)
#define DeeType_CallInstanceMethodAttr(tp_invoker, tp_self, attr, argc, argv)                          DeeType_CallInstanceMethodAttrStringHash(tp_invoker, tp_self, DeeString_STR(attr), DeeString_Hash(attr), argc, argv)
#define DeeType_CallInstanceMethodAttrHash(tp_invoker, tp_self, attr, hash, argc, argv)                DeeType_CallInstanceMethodAttrStringHash(tp_invoker, tp_self, DeeString_STR(attr), hash, argc, argv)
#define DeeType_CallInstanceMethodAttrString(tp_invoker, tp_self, attr, argc, argv)                    DeeType_CallInstanceMethodAttrStringHash(tp_invoker, tp_self, attr, Dee_HashStr(attr), argc, argv)
#define DeeType_CallInstanceMethodAttrStringLen(tp_invoker, tp_self, attr, attrlen, argc, argv)        DeeType_CallInstanceMethodAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, Dee_HashPtr(attr, attrlen), argc, argv)
#define DeeType_CallInstanceMethodAttrKw(tp_invoker, tp_self, attr, argc, argv, kw)                    DeeType_CallInstanceMethodAttrStringHashKw(tp_invoker, tp_self, DeeString_STR(attr), DeeString_Hash(attr), argc, argv, kw)
#define DeeType_CallInstanceMethodAttrHashKw(tp_invoker, tp_self, attr, hash, argc, argv, kw)          DeeType_CallInstanceMethodAttrStringHashKw(tp_invoker, tp_self, DeeString_STR(attr), hash, argc, argv, kw)
#define DeeType_CallInstanceMethodAttrStringKw(tp_invoker, tp_self, attr, argc, argv, kw)              DeeType_CallInstanceMethodAttrStringHashKw(tp_invoker, tp_self, attr, Dee_HashStr(attr), argc, argv, kw)
#define DeeType_CallInstanceMethodAttrStringLenKw(tp_invoker, tp_self, attr, attrlen, argc, argv, kw)  DeeType_CallInstanceMethodAttrStringLenHashKw(tp_invoker, tp_self, attr, attrlen, Dee_HashPtr(attr, attrlen), argc, argv, kw)
#define DeeType_CallIInstanceMethodAttr(tp_invoker, tp_self, attr, argc, argv)                         DeeType_CallIInstanceMethodAttrStringHash(tp_invoker, tp_self, DeeString_STR(attr), DeeString_Hash(attr), argc, argv)
#define DeeType_CallIInstanceMethodAttrHash(tp_invoker, tp_self, attr, hash, argc, argv)               DeeType_CallIInstanceMethodAttrStringHash(tp_invoker, tp_self, DeeString_STR(attr), hash, argc, argv)
#define DeeType_CallIInstanceMethodAttrString(tp_invoker, tp_self, attr, argc, argv)                   DeeType_CallIInstanceMethodAttrStringHash(tp_invoker, tp_self, attr, Dee_HashStr(attr), argc, argv)
#define DeeType_CallIInstanceMethodAttrStringLen(tp_invoker, tp_self, attr, attrlen, argc, argv)       DeeType_CallIInstanceMethodAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, Dee_HashPtr(attr, attrlen), argc, argv)
#define DeeType_CallIInstanceMethodAttrKw(tp_invoker, tp_self, attr, argc, argv, kw)                   DeeType_CallIInstanceMethodAttrStringHashKw(tp_invoker, tp_self, DeeString_STR(attr), DeeString_Hash(attr), argc, argv, kw)
#define DeeType_CallIInstanceMethodAttrHashKw(tp_invoker, tp_self, attr, hash, argc, argv, kw)         DeeType_CallIInstanceMethodAttrStringHashKw(tp_invoker, tp_self, DeeString_STR(attr), hash, argc, argv, kw)
#define DeeType_CallIInstanceMethodAttrStringKw(tp_invoker, tp_self, attr, argc, argv, kw)             DeeType_CallIInstanceMethodAttrStringHashKw(tp_invoker, tp_self, attr, Dee_HashStr(attr), argc, argv, kw)
#define DeeType_CallIInstanceMethodAttrStringLenKw(tp_invoker, tp_self, attr, attrlen, argc, argv, kw) DeeType_CallIInstanceMethodAttrStringLenHashKw(tp_invoker, tp_self, attr, attrlen, Dee_HashPtr(attr, attrlen), argc, argv, kw)
#ifdef CONFIG_CALLTUPLE_OPTIMIZATIONS
#define DeeType_CallMethodAttrTuple(tp_invoker, tp_self, self, attr, args)                            DeeType_CallMethodAttrStringHashTuple(tp_invoker, tp_self, self, DeeString_STR(attr), DeeString_Hash(attr), args)
#define DeeType_CallMethodAttrHashTuple(tp_invoker, tp_self, self, attr, hash, args)                  DeeType_CallMethodAttrStringHashTuple(tp_invoker, tp_self, self, DeeString_STR(attr), hash, args)
#define DeeType_CallMethodAttrStringTuple(tp_invoker, tp_self, self, attr, args)                      DeeType_CallMethodAttrStringHashTuple(tp_invoker, tp_self, self, attr, Dee_HashStr(attr), args)
#define DeeType_CallMethodAttrStringLenTuple(tp_invoker, tp_self, self, attr, attrlen, args)          DeeType_CallMethodAttrStringLenHashTuple(tp_invoker, tp_self, self, attr, attrlen, Dee_HashPtr(attr, attrlen), args)
#define DeeType_CallMethodAttrTupleKw(tp_invoker, tp_self, self, attr, args, kw)                      DeeType_CallMethodAttrStringHashTupleKw(tp_invoker, tp_self, self, DeeString_STR(attr), DeeString_Hash(attr), args, kw)
#define DeeType_CallMethodAttrHashTupleKw(tp_invoker, tp_self, self, attr, hash, args, kw)            DeeType_CallMethodAttrStringHashTupleKw(tp_invoker, tp_self, self, DeeString_STR(attr), hash, args, kw)
#define DeeType_CallMethodAttrStringTupleKw(tp_invoker, tp_self, self, attr, args, kw)                DeeType_CallMethodAttrStringHashTupleKw(tp_invoker, tp_self, self, attr, Dee_HashStr(attr), args, kw)
#define DeeType_CallMethodAttrStringLenTupleKw(tp_invoker, tp_self, self, attr, attrlen, args, kw)    DeeType_CallMethodAttrStringLenHashTupleKw(tp_invoker, tp_self, self, attr, attrlen, Dee_HashPtr(attr, attrlen), args, kw)
#define DeeType_CallClassMethodAttrTuple(tp_invoker, tp_self, attr, args)                             DeeType_CallClassMethodAttrStringHashTuple(tp_invoker, tp_self, DeeString_STR(attr), DeeString_Hash(attr), args)
#define DeeType_CallClassMethodAttrHashTuple(tp_invoker, tp_self, attr, hash, args)                   DeeType_CallClassMethodAttrStringHashTuple(tp_invoker, tp_self, DeeString_STR(attr), hash, args)
#define DeeType_CallClassMethodAttrStringTuple(tp_invoker, tp_self, attr, args)                       DeeType_CallClassMethodAttrStringHashTuple(tp_invoker, tp_self, attr, Dee_HashStr(attr), args)
#define DeeType_CallClassMethodAttrStringLenTuple(tp_invoker, tp_self, attr, attrlen, args)           DeeType_CallClassMethodAttrStringLenHashTuple(tp_invoker, tp_self, attr, attrlen, Dee_HashPtr(attr, attrlen), args)
#define DeeType_CallClassMethodAttrTupleKw(tp_invoker, tp_self, attr, args, kw)                       DeeType_CallClassMethodAttrStringHashTupleKw(tp_invoker, tp_self, DeeString_STR(attr), DeeString_Hash(attr), args, kw)
#define DeeType_CallClassMethodAttrHashTupleKw(tp_invoker, tp_self, attr, hash, args, kw)             DeeType_CallClassMethodAttrStringHashTupleKw(tp_invoker, tp_self, DeeString_STR(attr), hash, args, kw)
#define DeeType_CallClassMethodAttrStringTupleKw(tp_invoker, tp_self, attr, args, kw)                 DeeType_CallClassMethodAttrStringHashTupleKw(tp_invoker, tp_self, attr, Dee_HashStr(attr), args, kw)
#define DeeType_CallClassMethodAttrStringLenTupleKw(tp_invoker, tp_self, attr, attrlen, args, kw)     DeeType_CallClassMethodAttrStringLenHashTupleKw(tp_invoker, tp_self, attr, attrlen, Dee_HashPtr(attr, attrlen), args, kw)
#define DeeType_CallInstanceMethodAttrTuple(tp_invoker, tp_self, attr, args)                          DeeType_CallInstanceMethodAttrStringHashTuple(tp_invoker, tp_self, DeeString_STR(attr), DeeString_Hash(attr), args)
#define DeeType_CallInstanceMethodAttrHashTuple(tp_invoker, tp_self, attr, hash, args)                DeeType_CallInstanceMethodAttrStringHashTuple(tp_invoker, tp_self, DeeString_STR(attr), hash, args)
#define DeeType_CallInstanceMethodAttrStringTuple(tp_invoker, tp_self, attr, args)                    DeeType_CallInstanceMethodAttrStringHashTuple(tp_invoker, tp_self, attr, Dee_HashStr(attr), args)
#define DeeType_CallInstanceMethodAttrStringLenTuple(tp_invoker, tp_self, attr, attrlen, args)        DeeType_CallInstanceMethodAttrStringLenHashTuple(tp_invoker, tp_self, attr, attrlen, Dee_HashPtr(attr, attrlen), args)
#define DeeType_CallInstanceMethodAttrTupleKw(tp_invoker, tp_self, attr, args, kw)                    DeeType_CallInstanceMethodAttrStringHashTupleKw(tp_invoker, tp_self, DeeString_STR(attr), DeeString_Hash(attr), args, kw)
#define DeeType_CallInstanceMethodAttrHashTupleKw(tp_invoker, tp_self, attr, hash, args, kw)          DeeType_CallInstanceMethodAttrStringHashTupleKw(tp_invoker, tp_self, DeeString_STR(attr), hash, args, kw)
#define DeeType_CallInstanceMethodAttrStringTupleKw(tp_invoker, tp_self, attr, args, kw)              DeeType_CallInstanceMethodAttrStringHashTupleKw(tp_invoker, tp_self, attr, Dee_HashStr(attr), args, kw)
#define DeeType_CallInstanceMethodAttrStringLenTupleKw(tp_invoker, tp_self, attr, attrlen, args, kw)  DeeType_CallInstanceMethodAttrStringLenHashTupleKw(tp_invoker, tp_self, attr, attrlen, Dee_HashPtr(attr, attrlen), args, kw)
#define DeeType_CallIInstanceMethodAttrTuple(tp_invoker, tp_self, attr, args)                         DeeType_CallIInstanceMethodAttrStringHashTuple(tp_invoker, tp_self, DeeString_STR(attr), DeeString_Hash(attr), args)
#define DeeType_CallIInstanceMethodAttrHashTuple(tp_invoker, tp_self, attr, hash, args)               DeeType_CallIInstanceMethodAttrStringHashTuple(tp_invoker, tp_self, DeeString_STR(attr), hash, args)
#define DeeType_CallIInstanceMethodAttrStringTuple(tp_invoker, tp_self, attr, args)                   DeeType_CallIInstanceMethodAttrStringHashTuple(tp_invoker, tp_self, attr, Dee_HashStr(attr), args)
#define DeeType_CallIInstanceMethodAttrStringLenTuple(tp_invoker, tp_self, attr, attrlen, args)       DeeType_CallIInstanceMethodAttrStringLenHashTuple(tp_invoker, tp_self, attr, attrlen, Dee_HashPtr(attr, attrlen), args)
#define DeeType_CallIInstanceMethodAttrTupleKw(tp_invoker, tp_self, attr, args, kw)                   DeeType_CallIInstanceMethodAttrStringHashTupleKw(tp_invoker, tp_self, DeeString_STR(attr), DeeString_Hash(attr), args, kw)
#define DeeType_CallIInstanceMethodAttrHashTupleKw(tp_invoker, tp_self, attr, hash, args, kw)         DeeType_CallIInstanceMethodAttrStringHashTupleKw(tp_invoker, tp_self, DeeString_STR(attr), hash, args, kw)
#define DeeType_CallIInstanceMethodAttrStringTupleKw(tp_invoker, tp_self, attr, args, kw)             DeeType_CallIInstanceMethodAttrStringHashTupleKw(tp_invoker, tp_self, attr, Dee_HashStr(attr), args, kw)
#define DeeType_CallIInstanceMethodAttrStringLenTupleKw(tp_invoker, tp_self, attr, attrlen, args, kw) DeeType_CallIInstanceMethodAttrStringLenHashTupleKw(tp_invoker, tp_self, attr, attrlen, Dee_HashPtr(attr, attrlen), args, kw)
#endif /* CONFIG_CALLTUPLE_OPTIMIZATIONS */
#define DeeType_VCallMethodAttrf(tp_invoker, tp_self, self, attr, format, args)                      DeeType_VCallMethodAttrStringHashf(tp_invoker, tp_self, self, DeeString_STR(attr), DeeString_Hash(attr), format, args)
#define DeeType_VCallMethodAttrHashf(tp_invoker, tp_self, self, attr, hash, format, args)            DeeType_VCallMethodAttrStringHashf(tp_invoker, tp_self, self, DeeString_STR(attr), hash, format, args)
#define DeeType_VCallMethodAttrStringf(tp_invoker, tp_self, self, attr, format, args)                DeeType_VCallMethodAttrStringHashf(tp_invoker, tp_self, self, attr, Dee_HashStr(attr), format, args)
//#define DeeType_VCallMethodAttrStringLenf(tp_invoker, tp_self, self, attr, attrlen, format, args)    DeeType_VCallMethodAttrStringLenHashf(tp_invoker, tp_self, self, attr, attrlen, Dee_HashPtr(attr, attrlen), format, args)
#define DeeType_VCallClassMethodAttrf(tp_invoker, tp_self, attr, format, args)                       DeeType_VCallClassMethodAttrStringHashf(tp_invoker, tp_self, DeeString_STR(attr), DeeString_Hash(attr), format, args)
#define DeeType_VCallClassMethodAttrHashf(tp_invoker, tp_self, attr, hash, format, args)             DeeType_VCallClassMethodAttrStringHashf(tp_invoker, tp_self, DeeString_STR(attr), hash, format, args)
#define DeeType_VCallClassMethodAttrStringf(tp_invoker, tp_self, attr, format, args)                 DeeType_VCallClassMethodAttrStringHashf(tp_invoker, tp_self, attr, Dee_HashStr(attr), format, args)
//#define DeeType_VCallClassMethodAttrStringLenf(tp_invoker, tp_self, attr, attrlen, format, args)     DeeType_VCallClassMethodAttrStringLenHashf(tp_invoker, tp_self, attr, attrlen, Dee_HashPtr(attr, attrlen), format, args)
#define DeeType_VCallInstanceMethodAttrf(tp_invoker, tp_self, attr, format, args)                    DeeType_VCallInstanceMethodAttrStringHashf(tp_invoker, tp_self, DeeString_STR(attr), DeeString_Hash(attr), format, args)
#define DeeType_VCallInstanceMethodAttrHashf(tp_invoker, tp_self, attr, hash, format, args)          DeeType_VCallInstanceMethodAttrStringHashf(tp_invoker, tp_self, DeeString_STR(attr), hash, format, args)
#define DeeType_VCallInstanceMethodAttrStringf(tp_invoker, tp_self, attr, format, args)              DeeType_VCallInstanceMethodAttrStringHashf(tp_invoker, tp_self, attr, Dee_HashStr(attr), format, args)
//#define DeeType_VCallInstanceMethodAttrStringLenf(tp_invoker, tp_self, attr, attrlen, format, args)  DeeType_VCallInstanceMethodAttrStringLenHashf(tp_invoker, tp_self, attr, attrlen, Dee_HashPtr(attr, attrlen), format, args)
//#define DeeType_VCallIInstanceMethodAttrf(tp_invoker, tp_self, attr, format, args)                   DeeType_VCallIInstanceMethodAttrStringHashf(tp_invoker, tp_self, DeeString_STR(attr), DeeString_Hash(attr), format, args)
//#define DeeType_VCallIInstanceMethodAttrHashf(tp_invoker, tp_self, attr, hash, format, args)         DeeType_VCallIInstanceMethodAttrStringHashf(tp_invoker, tp_self, DeeString_STR(attr), hash, format, args)
//#define DeeType_VCallIInstanceMethodAttrStringf(tp_invoker, tp_self, attr, format, args)             DeeType_VCallIInstanceMethodAttrStringHashf(tp_invoker, tp_self, attr, Dee_HashStr(attr), format, args)
//#define DeeType_VCallIInstanceMethodAttrStringLenf(tp_invoker, tp_self, attr, attrlen, format, args) DeeType_VCallIInstanceMethodAttrStringLenHashf(tp_invoker, tp_self, attr, attrlen, Dee_HashPtr(attr, attrlen), format, args)


/* Get attributes from `tp_self->tp_getsets' / `tp_self->tp_class_getsets'.
 * @return: * :        The attribute value.
 * @return: NULL:      An error occurred.
 * @return: ITER_DONE: The attribute could not be found in `chain'. */
#ifdef __INTELLISENSE__
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) DREF DeeObject *(DCALL DeeType_GetGetSetAttrStringHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, DeeObject *self, char const *__restrict attr, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_GetClassGetSetAttrStringHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_GetInstanceGetSetAttrStringHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_GetIInstanceGetSetAttrStringHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) DREF DeeObject *(DCALL DeeType_GetGetSetAttrStringLenHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, DeeObject *self, char const *__restrict attr, size_t attrlen, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_GetClassGetSetAttrStringLenHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, size_t attrlen, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_GetInstanceGetSetAttrStringLenHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, size_t attrlen, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_GetIInstanceGetSetAttrStringLenHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, size_t attrlen, dhash_t hash);
#else /* __INTELLISENSE__ */
INTDEF WUNUSED NONNULL((1, 2, 3, 4, 5)) DREF DeeObject *DCALL /* GET_GETSET */
type_getset_getattr_string_hash(struct Dee_membercache *cache, DeeTypeObject *decl,
                                struct type_getset const *chain, DeeObject *self,
                                char const *__restrict attr, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3, 4, 5)) DREF DeeObject *DCALL /* GET_GETSET */
type_getset_getattr_string_len_hash(struct Dee_membercache *cache, DeeTypeObject *decl,
                                    struct type_getset const *chain, DeeObject *self,
                                    char const *__restrict attr, size_t attrlen, dhash_t hash);
#define DeeType_GetGetSetAttrStringHash(tp_invoker, tp_self, self, attr, hash)             type_getset_getattr_string_hash(&(tp_invoker)->tp_cache, tp_self, (tp_self)->tp_getsets, self, attr, hash)
#define DeeType_GetClassGetSetAttrStringHash(tp_invoker, tp_self, attr, hash)              type_getset_getattr_string_hash(&(tp_invoker)->tp_class_cache, tp_self, (tp_self)->tp_class_getsets, (DeeObject *)(tp_invoker), attr, hash)
#define DeeType_GetGetSetAttrStringLenHash(tp_invoker, tp_self, self, attr, attrlen, hash) type_getset_getattr_string_len_hash(&(tp_invoker)->tp_cache, tp_self, (tp_self)->tp_getsets, self, attr, attrlen, hash)
#define DeeType_GetClassGetSetAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, hash)  type_getset_getattr_string_len_hash(&(tp_invoker)->tp_class_cache, tp_self, (tp_self)->tp_class_getsets, (DeeObject *)(tp_invoker), attr, attrlen, hash)
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_GetInstanceGetSetAttrStringHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_GetIInstanceGetSetAttrStringHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_GetInstanceGetSetAttrStringLenHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, size_t attrlen, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_GetIInstanceGetSetAttrStringLenHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, size_t attrlen, dhash_t hash);
#endif /* !__INTELLISENSE__ */
#define DeeType_GetGetSetAttr(tp_invoker, tp_self, self, attr)                      DeeType_GetGetSetAttrStringHash(tp_invoker, tp_self, self, DeeString_STR(attr), DeeString_Hash(attr))
#define DeeType_GetGetSetAttrHash(tp_invoker, tp_self, self, attr, hash)            DeeType_GetGetSetAttrStringHash(tp_invoker, tp_self, self, DeeString_STR(attr), hash)
#define DeeType_GetGetSetAttrString(tp_invoker, tp_self, self, attr)                DeeType_GetGetSetAttrStringHash(tp_invoker, tp_self, self, attr, Dee_HashStr(attr))
#define DeeType_GetGetSetAttrStringLen(tp_invoker, tp_self, self, attr, attrlen)    DeeType_GetGetSetAttrStringLenHash(tp_invoker, tp_self, self, attr, attrlen, Dee_HashPtr(attr, attrlen))
#define DeeType_GetClassGetSetAttr(tp_invoker, tp_self, attr)                       DeeType_GetClassGetSetAttrStringHash(tp_invoker, tp_self, DeeString_STR(attr), DeeString_Hash(attr))
#define DeeType_GetClassGetSetAttrHash(tp_invoker, tp_self, attr, hash)             DeeType_GetClassGetSetAttrStringHash(tp_invoker, tp_self, DeeString_STR(attr), hash)
#define DeeType_GetClassGetSetAttrString(tp_invoker, tp_self, attr)                 DeeType_GetClassGetSetAttrStringHash(tp_invoker, tp_self, attr, Dee_HashStr(attr))
#define DeeType_GetClassGetSetAttrStringLen(tp_invoker, tp_self, attr, attrlen)     DeeType_GetClassGetSetAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, Dee_HashPtr(attr, attrlen))
#define DeeType_GetInstanceGetSetAttr(tp_invoker, tp_self, attr)                    DeeType_GetInstanceGetSetAttrStringHash(tp_invoker, tp_self, DeeString_STR(attr), DeeString_Hash(attr))
#define DeeType_GetInstanceGetSetAttrHash(tp_invoker, tp_self, attr, hash)          DeeType_GetInstanceGetSetAttrStringHash(tp_invoker, tp_self, DeeString_STR(attr), hash)
#define DeeType_GetInstanceGetSetAttrString(tp_invoker, tp_self, attr)              DeeType_GetInstanceGetSetAttrStringHash(tp_invoker, tp_self, attr, Dee_HashStr(attr))
#define DeeType_GetInstanceGetSetAttrStringLen(tp_invoker, tp_self, attr, attrlen)  DeeType_GetInstanceGetSetAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, Dee_HashPtr(attr, attrlen))
#define DeeType_GetIInstanceGetSetAttr(tp_invoker, tp_self, attr)                   DeeType_GetIInstanceGetSetAttrStringHash(tp_invoker, tp_self, DeeString_STR(attr), DeeString_Hash(attr))
#define DeeType_GetIInstanceGetSetAttrHash(tp_invoker, tp_self, attr, hash)         DeeType_GetIInstanceGetSetAttrStringHash(tp_invoker, tp_self, DeeString_STR(attr), hash)
#define DeeType_GetIInstanceGetSetAttrString(tp_invoker, tp_self, attr)             DeeType_GetIInstanceGetSetAttrStringHash(tp_invoker, tp_self, attr, Dee_HashStr(attr))
#define DeeType_GetIInstanceGetSetAttrStringLen(tp_invoker, tp_self, attr, attrlen) DeeType_GetIInstanceGetSetAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, Dee_HashPtr(attr, attrlen))


/* Call attributes from `tp_self->tp_getsets' / `tp_self->tp_class_getsets'.
 * @return: * :        The functions's return value.
 * @return: NULL:      An error occurred.
 * @return: ITER_DONE: The attribute could not be found in `chain'. */
#ifdef __INTELLISENSE__
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallInstanceGetSetAttrStringHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, dhash_t hash, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallInstanceGetSetAttrStringLenHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, size_t attrlen, dhash_t hash, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallInstanceGetSetAttrStringHashKw)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, dhash_t hash, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallInstanceGetSetAttrStringLenHashKw)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, size_t attrlen, dhash_t hash, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallIInstanceGetSetAttrStringHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, dhash_t hash, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallIInstanceGetSetAttrStringLenHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, size_t attrlen, dhash_t hash, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallIInstanceGetSetAttrStringHashKw)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, dhash_t hash, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallIInstanceGetSetAttrStringLenHashKw)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, size_t attrlen, dhash_t hash, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1, 2, 3, 5)) DREF DeeObject *(DCALL DeeType_VCallInstanceGetSetAttrStringHashf)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, dhash_t hash, char const *__restrict format, va_list args);
#else /* __INTELLISENSE__ */
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallInstanceGetSetAttrStringHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, dhash_t hash, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallInstanceGetSetAttrStringLenHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, size_t attrlen, dhash_t hash, size_t argc, DeeObject *const *argv);
#define DeeType_CallIInstanceGetSetAttrStringHash(tp_invoker, tp_self, attr, hash, argc, argv)             DeeType_CallIInstanceGetSetAttrStringHashKw(tp_invoker, tp_self, attr, hash, argc, argv, NULL)
#define DeeType_CallIInstanceGetSetAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, hash, argc, argv) DeeType_CallIInstanceGetSetAttrStringLenHashKw(tp_invoker, tp_self, attr, attrlen, hash, argc, argv, NULL)
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallInstanceGetSetAttrStringHashKw)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, dhash_t hash, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallInstanceGetSetAttrStringLenHashKw)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, size_t attrlen, dhash_t hash, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallIInstanceGetSetAttrStringHashKw)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, dhash_t hash, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallIInstanceGetSetAttrStringLenHashKw)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, size_t attrlen, dhash_t hash, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1, 2, 3, 5)) DREF DeeObject *(DCALL DeeType_VCallInstanceGetSetAttrStringHashf)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, dhash_t hash, char const *__restrict format, va_list args);
#endif /* !__INTELLISENSE__ */
#define DeeType_CallInstanceGetSetAttr(tp_invoker, tp_self, attr, argc, argv)                          DeeType_CallInstanceGetSetAttrStringHash(tp_invoker, tp_self, DeeString_STR(attr), DeeString_Hash(attr), argc, argv)
#define DeeType_CallInstanceGetSetAttrHash(tp_invoker, tp_self, attr, hash, argc, argv)                DeeType_CallInstanceGetSetAttrStringHash(tp_invoker, tp_self, DeeString_STR(attr), hash, argc, argv)
#define DeeType_CallInstanceGetSetAttrString(tp_invoker, tp_self, attr, argc, argv)                    DeeType_CallInstanceGetSetAttrStringHash(tp_invoker, tp_self, attr, Dee_HashStr(attr), argc, argv)
#define DeeType_CallInstanceGetSetAttrStringLen(tp_invoker, tp_self, attr, attrlen, argc, argv)        DeeType_CallInstanceGetSetAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, Dee_HashPtr(attr, attrlen), argc, argv)
#define DeeType_CallInstanceGetSetAttrKw(tp_invoker, tp_self, attr, argc, argv, kw)                    DeeType_CallInstanceGetSetAttrStringHashKw(tp_invoker, tp_self, DeeString_STR(attr), DeeString_Hash(attr), argc, argv, kw)
#define DeeType_CallInstanceGetSetAttrHashKw(tp_invoker, tp_self, attr, hash, argc, argv, kw)          DeeType_CallInstanceGetSetAttrStringHashKw(tp_invoker, tp_self, DeeString_STR(attr), hash, argc, argv, kw)
#define DeeType_CallInstanceGetSetAttrStringKw(tp_invoker, tp_self, attr, argc, argv, kw)              DeeType_CallInstanceGetSetAttrStringHashKw(tp_invoker, tp_self, attr, Dee_HashStr(attr), argc, argv, kw)
#define DeeType_CallInstanceGetSetAttrStringLenKw(tp_invoker, tp_self, attr, attrlen, argc, argv, kw)  DeeType_CallInstanceGetSetAttrStringLenHashKw(tp_invoker, tp_self, attr, attrlen, Dee_HashPtr(attr, attrlen), argc, argv, kw)
#define DeeType_VCallInstanceGetSetAttrf(tp_invoker, tp_self, attr, format, args)                      DeeType_VCallInstanceGetSetAttrStringHashf(tp_invoker, tp_self, DeeString_STR(attr), DeeString_Hash(attr), format, args)
#define DeeType_VCallInstanceGetSetAttrHashf(tp_invoker, tp_self, attr, hash, format, args)            DeeType_VCallInstanceGetSetAttrStringHashf(tp_invoker, tp_self, DeeString_STR(attr), hash, format, args)
#define DeeType_VCallInstanceGetSetAttrStringf(tp_invoker, tp_self, attr, format, args)                DeeType_VCallInstanceGetSetAttrStringHashf(tp_invoker, tp_self, attr, Dee_HashStr(attr), format, args)
#define DeeType_CallIInstanceGetSetAttr(tp_invoker, tp_self, attr, argc, argv)                         DeeType_CallIInstanceGetSetAttrStringHash(tp_invoker, tp_self, DeeString_STR(attr), DeeString_Hash(attr), argc, argv)
#define DeeType_CallIInstanceGetSetAttrHash(tp_invoker, tp_self, attr, hash, argc, argv)               DeeType_CallIInstanceGetSetAttrStringHash(tp_invoker, tp_self, DeeString_STR(attr), hash, argc, argv)
#define DeeType_CallIInstanceGetSetAttrString(tp_invoker, tp_self, attr, argc, argv)                   DeeType_CallIInstanceGetSetAttrStringHash(tp_invoker, tp_self, attr, Dee_HashStr(attr), argc, argv)
#define DeeType_CallIInstanceGetSetAttrStringLen(tp_invoker, tp_self, attr, attrlen, argc, argv)       DeeType_CallIInstanceGetSetAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, Dee_HashPtr(attr, attrlen), argc, argv)
#define DeeType_CallIInstanceGetSetAttrKw(tp_invoker, tp_self, attr, argc, argv, kw)                   DeeType_CallIInstanceGetSetAttrStringHashKw(tp_invoker, tp_self, DeeString_STR(attr), DeeString_Hash(attr), argc, argv, kw)
#define DeeType_CallIInstanceGetSetAttrHashKw(tp_invoker, tp_self, attr, hash, argc, argv, kw)         DeeType_CallIInstanceGetSetAttrStringHashKw(tp_invoker, tp_self, DeeString_STR(attr), hash, argc, argv, kw)
#define DeeType_CallIInstanceGetSetAttrStringKw(tp_invoker, tp_self, attr, argc, argv, kw)             DeeType_CallIInstanceGetSetAttrStringHashKw(tp_invoker, tp_self, attr, Dee_HashStr(attr), argc, argv, kw)
#define DeeType_CallIInstanceGetSetAttrStringLenKw(tp_invoker, tp_self, attr, attrlen, argc, argv, kw) DeeType_CallIInstanceGetSetAttrStringLenHashKw(tp_invoker, tp_self, attr, attrlen, Dee_HashPtr(attr, attrlen), argc, argv, kw)


/* Check if attributes from `tp_self->tp_getsets' / `tp_self->tp_class_getsets' are bound.
 * @return:  1: The attribute is bound.
 * @return:  0: The attribute is unbound.
 * @return: -1: An error occurred.
 * @return: -2: The attribute could not be found in `chain'. */
#ifdef __INTELLISENSE__
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int (DCALL DeeType_BoundGetSetAttrStringHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, DeeObject *self, char const *__restrict attr, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int (DCALL DeeType_BoundGetSetAttrStringLenHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, DeeObject *self, char const *__restrict attr, size_t attrlen, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int (DCALL DeeType_BoundClassGetSetAttrStringHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int (DCALL DeeType_BoundClassGetSetAttrStringLenHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, size_t attrlen, dhash_t hash);
#else /* __INTELLISENSE__ */
INTDEF WUNUSED NONNULL((1, 2, 3, 4, 5)) int DCALL /* BOUND_GETSET */
type_getset_boundattr_string_hash(struct Dee_membercache *cache, DeeTypeObject *decl,
                                  struct type_getset const *chain, DeeObject *self,
                                  char const *__restrict attr, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3, 4, 5)) int DCALL /* BOUND_GETSET */
type_getset_boundattr_string_len_hash(struct Dee_membercache *cache, DeeTypeObject *decl,
                                      struct type_getset const *chain, DeeObject *self,
                                      char const *__restrict attr, size_t attrlen, dhash_t hash);
#define DeeType_BoundGetSetAttrStringHash(tp_invoker, tp_self, self, attr, hash)             type_getset_boundattr_string_hash(&(tp_invoker)->tp_cache, tp_self, (tp_self)->tp_getsets, self, attr, hash)
#define DeeType_BoundGetSetAttrStringLenHash(tp_invoker, tp_self, self, attr, attrlen, hash) type_getset_boundattr_string_len_hash(&(tp_invoker)->tp_cache, tp_self, (tp_self)->tp_getsets, self, attr, attrlen, hash)
#define DeeType_BoundClassGetSetAttrStringHash(tp_invoker, tp_self, attr, hash)              type_getset_boundattr_string_hash(&(tp_invoker)->tp_class_cache, tp_self, (tp_self)->tp_class_getsets, (DeeObject *)(tp_invoker), attr, hash)
#define DeeType_BoundClassGetSetAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, hash)  type_getset_boundattr_string_len_hash(&(tp_invoker)->tp_class_cache, tp_self, (tp_self)->tp_class_getsets, (DeeObject *)(tp_invoker), attr, attrlen, hash)
#endif /* !__INTELLISENSE__ */
#define DeeType_BoundGetSetAttr(tp_invoker, tp_self, self, attr)                   DeeType_BoundGetSetAttrStringHash(tp_invoker, tp_self, self, DeeString_STR(attr), DeeString_Hash(attr))
#define DeeType_BoundGetSetAttrHash(tp_invoker, tp_self, self, attr, hash)         DeeType_BoundGetSetAttrStringHash(tp_invoker, tp_self, self, DeeString_STR(attr), hash)
#define DeeType_BoundGetSetAttrString(tp_invoker, tp_self, self, attr)             DeeType_BoundGetSetAttrStringHash(tp_invoker, tp_self, self, attr, Dee_HashStr(attr))
#define DeeType_BoundGetSetAttrStringLen(tp_invoker, tp_self, self, attr, attrlen) DeeType_BoundGetSetAttrStringLenHash(tp_invoker, tp_self, self, attr, attrlen, Dee_HashPtr(attr, attrlen))
#define DeeType_BoundClassGetSetAttr(tp_invoker, tp_self, attr)                    DeeType_BoundClassGetSetAttrStringHash(tp_invoker, tp_self, DeeString_STR(attr), DeeString_Hash(attr))
#define DeeType_BoundClassGetSetAttrHash(tp_invoker, tp_self, attr, hash)          DeeType_BoundClassGetSetAttrStringHash(tp_invoker, tp_self, DeeString_STR(attr), hash)
#define DeeType_BoundClassGetSetAttrString(tp_invoker, tp_self, attr)              DeeType_BoundClassGetSetAttrStringHash(tp_invoker, tp_self, attr, Dee_HashStr(attr))
#define DeeType_BoundClassGetSetAttrStringLen(tp_invoker, tp_self, attr, attrlen)  DeeType_BoundClassGetSetAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, Dee_HashPtr(attr, attrlen))


/* Delete attributes from `tp_self->tp_getsets' / `tp_self->tp_class_getsets'.
 * @return:  0: Success.
 * @return: -1: An error occurred.
 * @return:  1: The attribute could not be found in `chain'. */
#ifdef __INTELLISENSE__
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int (DCALL DeeType_DelGetSetAttrStringHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, DeeObject *self, char const *__restrict attr, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int (DCALL DeeType_DelGetSetAttrStringLenHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, DeeObject *self, char const *__restrict attr, size_t attrlen, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int (DCALL DeeType_DelClassGetSetAttrStringHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int (DCALL DeeType_DelClassGetSetAttrStringLenHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, size_t attrlen, dhash_t hash);
#else /* __INTELLISENSE__ */
INTDEF WUNUSED NONNULL((1, 2, 3, 4, 5)) int DCALL /* DEL_GETSET */
type_getset_delattr_string_hash(struct Dee_membercache *cache, DeeTypeObject *decl,
                                struct type_getset const *chain, DeeObject *self,
                                char const *__restrict attr, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3, 4, 5)) int DCALL /* DEL_GETSET */
type_getset_delattr_string_len_hash(struct Dee_membercache *cache, DeeTypeObject *decl,
                                    struct type_getset const *chain, DeeObject *self,
                                    char const *__restrict attr, size_t attrlen, dhash_t hash);
#define DeeType_DelGetSetAttrStringHash(tp_invoker, tp_self, self, attr, hash)             type_getset_delattr_string_hash(&(tp_invoker)->tp_cache, tp_self, (tp_self)->tp_getsets, self, attr, hash)
#define DeeType_DelGetSetAttrStringLenHash(tp_invoker, tp_self, self, attr, attrlen, hash) type_getset_delattr_string_len_hash(&(tp_invoker)->tp_cache, tp_self, (tp_self)->tp_getsets, self, attr, attrlen, hash)
#define DeeType_DelClassGetSetAttrStringHash(tp_invoker, tp_self, attr, hash)              type_getset_delattr_string_hash(&(tp_invoker)->tp_class_cache, tp_self, (tp_self)->tp_class_getsets, (DeeObject *)(tp_invoker), attr, hash)
#define DeeType_DelClassGetSetAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, hash)  type_getset_delattr_string_len_hash(&(tp_invoker)->tp_class_cache, tp_self, (tp_self)->tp_class_getsets, (DeeObject *)(tp_invoker), attr, attrlen, hash)
#endif /* !__INTELLISENSE__ */
#define DeeType_DelGetSetAttr(tp_invoker, tp_self, self, attr)                   DeeType_DelGetSetAttrStringHash(tp_invoker, tp_self, self, DeeString_STR(attr), DeeString_Hash(attr))
#define DeeType_DelGetSetAttrHash(tp_invoker, tp_self, self, attr, hash)         DeeType_DelGetSetAttrStringHash(tp_invoker, tp_self, self, DeeString_STR(attr), hash)
#define DeeType_DelGetSetAttrString(tp_invoker, tp_self, self, attr)             DeeType_DelGetSetAttrStringHash(tp_invoker, tp_self, self, attr, Dee_HashStr(attr))
#define DeeType_DelGetSetAttrStringLen(tp_invoker, tp_self, self, attr, attrlen) DeeType_DelGetSetAttrStringLenHash(tp_invoker, tp_self, self, attr, attrlen, Dee_HashPtr(attr, attrlen))
#define DeeType_DelClassGetSetAttr(tp_invoker, tp_self, attr)                    DeeType_DelClassGetSetAttrStringHash(tp_invoker, tp_self, DeeString_STR(attr), DeeString_Hash(attr))
#define DeeType_DelClassGetSetAttrHash(tp_invoker, tp_self, attr, hash)          DeeType_DelClassGetSetAttrStringHash(tp_invoker, tp_self, DeeString_STR(attr), hash)
#define DeeType_DelClassGetSetAttrString(tp_invoker, tp_self, attr)              DeeType_DelClassGetSetAttrStringHash(tp_invoker, tp_self, attr, Dee_HashStr(attr))
#define DeeType_DelClassGetSetAttrStringLen(tp_invoker, tp_self, attr, attrlen)  DeeType_DelClassGetSetAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, Dee_HashPtr(attr, attrlen))


/* Set attributes from `tp_self->tp_getsets' / `tp_self->tp_class_getsets'.
 * @return:  0: Success.
 * @return: -1: An error occurred.
 * @return:  1: The attribute could not be found in `chain'. */
#ifdef __INTELLISENSE__
INTDEF WUNUSED NONNULL((1, 2, 3, 4, 6)) int (DCALL DeeType_SetGetSetAttrStringHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, DeeObject *self, char const *__restrict attr, dhash_t hash, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3, 4, 7)) int (DCALL DeeType_SetGetSetAttrStringLenHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, DeeObject *self, char const *__restrict attr, size_t attrlen, dhash_t hash, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3, 5)) int (DCALL DeeType_SetClassGetSetAttrStringHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, dhash_t hash, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3, 6)) int (DCALL DeeType_SetClassGetSetAttrStringLenHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, size_t attrlen, dhash_t hash, DeeObject *value);
#else /* __INTELLISENSE__ */
INTDEF WUNUSED NONNULL((1, 2, 3, 4, 5, 7)) int DCALL /* SET_GETSET */
type_getset_setattr_string_hash(struct Dee_membercache *cache, DeeTypeObject *decl,
                                struct type_getset const *chain, DeeObject *self,
                                char const *__restrict attr, dhash_t hash, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3, 4, 5, 8)) int DCALL /* SET_GETSET */
type_getset_setattr_string_len_hash(struct Dee_membercache *cache, DeeTypeObject *decl,
                                    struct type_getset const *chain, DeeObject *self,
                                    char const *__restrict attr, size_t attrlen,
                                    dhash_t hash, DeeObject *value);
#define DeeType_SetGetSetAttrStringHash(tp_invoker, tp_self, self, attr, hash, value)             type_getset_setattr_string_hash(&(tp_invoker)->tp_cache, tp_self, (tp_self)->tp_getsets, self, attr, hash, value)
#define DeeType_SetGetSetAttrStringLenHash(tp_invoker, tp_self, self, attr, attrlen, hash, value) type_getset_setattr_string_len_hash(&(tp_invoker)->tp_cache, tp_self, (tp_self)->tp_getsets, self, attr, attrlen, hash, value)
#define DeeType_SetClassGetSetAttrStringHash(tp_invoker, tp_self, attr, hash, value)              type_getset_setattr_string_hash(&(tp_invoker)->tp_class_cache, tp_self, (tp_self)->tp_class_getsets, (DeeObject *)(tp_invoker), attr, hash, value)
#define DeeType_SetClassGetSetAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, hash, value)  type_getset_setattr_string_len_hash(&(tp_invoker)->tp_class_cache, tp_self, (tp_self)->tp_class_getsets, (DeeObject *)(tp_invoker), attr, attrlen, hash, value)
#endif /* !__INTELLISENSE__ */
#define DeeType_SetGetSetAttr(tp_invoker, tp_self, self, attr, value)                   DeeType_SetGetSetAttrStringHash(tp_invoker, tp_self, self, DeeString_STR(attr), DeeString_Hash(attr), value)
#define DeeType_SetGetSetAttrHash(tp_invoker, tp_self, self, attr, hash, value)         DeeType_SetGetSetAttrStringHash(tp_invoker, tp_self, self, DeeString_STR(attr), hash, value)
#define DeeType_SetGetSetAttrString(tp_invoker, tp_self, self, attr, value)             DeeType_SetGetSetAttrStringHash(tp_invoker, tp_self, self, attr, Dee_HashStr(attr), value)
#define DeeType_SetGetSetAttrStringLen(tp_invoker, tp_self, self, attr, attrlen, value) DeeType_SetGetSetAttrStringLenHash(tp_invoker, tp_self, self, attr, attrlen, Dee_HashPtr(attr, attrlen), value)
#define DeeType_SetClassGetSetAttr(tp_invoker, tp_self, attr, value)                    DeeType_SetClassGetSetAttrStringHash(tp_invoker, tp_self, DeeString_STR(attr), DeeString_Hash(attr), value)
#define DeeType_SetClassGetSetAttrHash(tp_invoker, tp_self, attr, hash, value)          DeeType_SetClassGetSetAttrStringHash(tp_invoker, tp_self, DeeString_STR(attr), hash, value)
#define DeeType_SetClassGetSetAttrString(tp_invoker, tp_self, attr, value)              DeeType_SetClassGetSetAttrStringHash(tp_invoker, tp_self, attr, Dee_HashStr(attr), value)
#define DeeType_SetClassGetSetAttrStringLen(tp_invoker, tp_self, attr, attrlen, value)  DeeType_SetClassGetSetAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, Dee_HashPtr(attr, attrlen), value)


/* Get attributes from `tp_self->tp_members' / `tp_self->tp_class_members'.
 * @return: * :        The attribute value.
 * @return: NULL:      An error occurred.
 * @return: ITER_DONE: The attribute could not be found in `chain'. */
#ifdef __INTELLISENSE__
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) DREF DeeObject *(DCALL DeeType_GetMemberAttrStringHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, DeeObject *self, char const *__restrict attr, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_GetClassMemberAttrStringHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_GetInstanceMemberAttrStringHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_GetIInstanceMemberAttrStringHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) DREF DeeObject *(DCALL DeeType_GetMemberAttrStringLenHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, DeeObject *self, char const *__restrict attr, size_t attrlen, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_GetClassMemberAttrStringLenHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, size_t attrlen, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_GetInstanceMemberAttrStringLenHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, size_t attrlen, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_GetIInstanceMemberAttrStringLenHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, size_t attrlen, dhash_t hash);
#else /* __INTELLISENSE__ */
INTDEF WUNUSED NONNULL((1, 2, 3, 4, 5)) DREF DeeObject *DCALL /* GET_MEMBER */
type_member_getattr_string_hash(struct Dee_membercache *cache, DeeTypeObject *decl,
                                struct type_member const *chain, DeeObject *self,
                                char const *__restrict attr, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3, 4, 5)) DREF DeeObject *DCALL /* GET_MEMBER */
type_member_getattr_string_len_hash(struct Dee_membercache *cache, DeeTypeObject *decl,
                                    struct type_member const *chain, DeeObject *self,
                                    char const *__restrict attr, size_t attrlen, dhash_t hash);
#define DeeType_GetMemberAttrStringHash(tp_invoker, tp_self, self, attr, hash)             type_member_getattr_string_hash(&(tp_invoker)->tp_cache, tp_self, (tp_self)->tp_members, self, attr, hash)
#define DeeType_GetClassMemberAttrStringHash(tp_invoker, tp_self, attr, hash)              type_member_getattr_string_hash(&(tp_invoker)->tp_class_cache, tp_self, (tp_self)->tp_class_members, (DeeObject *)(tp_invoker), attr, hash)
#define DeeType_GetMemberAttrStringLenHash(tp_invoker, tp_self, self, attr, attrlen, hash) type_member_getattr_string_len_hash(&(tp_invoker)->tp_cache, tp_self, (tp_self)->tp_members, self, attr, attrlen, hash)
#define DeeType_GetClassMemberAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, hash)  type_member_getattr_string_len_hash(&(tp_invoker)->tp_class_cache, tp_self, (tp_self)->tp_class_members, (DeeObject *)(tp_invoker), attr, attrlen, hash)
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_GetInstanceMemberAttrStringHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_GetIInstanceMemberAttrStringHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_GetInstanceMemberAttrStringLenHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, size_t attrlen, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_GetIInstanceMemberAttrStringLenHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, size_t attrlen, dhash_t hash);
#endif /* !__INTELLISENSE__ */
#define DeeType_GetMemberAttr(tp_invoker, tp_self, self, attr)                      DeeType_GetMemberAttrStringHash(tp_invoker, tp_self, self, DeeString_STR(attr), DeeString_Hash(attr))
#define DeeType_GetMemberAttrHash(tp_invoker, tp_self, self, attr, hash)            DeeType_GetMemberAttrStringHash(tp_invoker, tp_self, self, DeeString_STR(attr), hash)
#define DeeType_GetMemberAttrString(tp_invoker, tp_self, self, attr)                DeeType_GetMemberAttrStringHash(tp_invoker, tp_self, self, attr, Dee_HashStr(attr))
#define DeeType_GetMemberAttrStringLen(tp_invoker, tp_self, self, attr, attrlen)    DeeType_GetMemberAttrStringLenHash(tp_invoker, tp_self, self, attr, attrlen, Dee_HashPtr(attr, attrlen))
#define DeeType_GetClassMemberAttr(tp_invoker, tp_self, attr)                       DeeType_GetClassMemberAttrStringHash(tp_invoker, tp_self, DeeString_STR(attr), DeeString_Hash(attr))
#define DeeType_GetClassMemberAttrHash(tp_invoker, tp_self, attr, hash)             DeeType_GetClassMemberAttrStringHash(tp_invoker, tp_self, DeeString_STR(attr), hash)
#define DeeType_GetClassMemberAttrString(tp_invoker, tp_self, attr)                 DeeType_GetClassMemberAttrStringHash(tp_invoker, tp_self, attr, Dee_HashStr(attr))
#define DeeType_GetClassMemberAttrStringLen(tp_invoker, tp_self, attr, attrlen)     DeeType_GetClassMemberAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, Dee_HashPtr(attr, attrlen))
#define DeeType_GetInstanceMemberAttr(tp_invoker, tp_self, attr)                    DeeType_GetInstanceMemberAttrStringHash(tp_invoker, tp_self, DeeString_STR(attr), DeeString_Hash(attr))
#define DeeType_GetInstanceMemberAttrHash(tp_invoker, tp_self, attr, hash)          DeeType_GetInstanceMemberAttrStringHash(tp_invoker, tp_self, DeeString_STR(attr), hash)
#define DeeType_GetInstanceMemberAttrString(tp_invoker, tp_self, attr)              DeeType_GetInstanceMemberAttrStringHash(tp_invoker, tp_self, attr, Dee_HashStr(attr))
#define DeeType_GetInstanceMemberAttrStringLen(tp_invoker, tp_self, attr, attrlen)  DeeType_GetInstanceMemberAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, Dee_HashPtr(attr, attrlen))
#define DeeType_GetIInstanceMemberAttr(tp_invoker, tp_self, attr)                   DeeType_GetIInstanceMemberAttrStringHash(tp_invoker, tp_self, DeeString_STR(attr), DeeString_Hash(attr))
#define DeeType_GetIInstanceMemberAttrHash(tp_invoker, tp_self, attr, hash)         DeeType_GetIInstanceMemberAttrStringHash(tp_invoker, tp_self, DeeString_STR(attr), hash)
#define DeeType_GetIInstanceMemberAttrString(tp_invoker, tp_self, attr)             DeeType_GetIInstanceMemberAttrStringHash(tp_invoker, tp_self, attr, Dee_HashStr(attr))
#define DeeType_GetIInstanceMemberAttrStringLen(tp_invoker, tp_self, attr, attrlen) DeeType_GetIInstanceMemberAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, Dee_HashPtr(attr, attrlen))


/* Call attributes from `tp_self->tp_members' / `tp_self->tp_class_members'.
 * @return: * :        The functions's return value.
 * @return: NULL:      An error occurred.
 * @return: ITER_DONE: The attribute could not be found in `chain'. */
#ifdef __INTELLISENSE__
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallInstanceMemberAttrStringHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, dhash_t hash, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallInstanceMemberAttrStringLenHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, size_t attrlen, dhash_t hash, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallInstanceMemberAttrStringHashKw)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, dhash_t hash, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallInstanceMemberAttrStringLenHashKw)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, size_t attrlen, dhash_t hash, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallIInstanceMemberAttrStringHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, dhash_t hash, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallIInstanceMemberAttrStringLenHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, size_t attrlen, dhash_t hash, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallIInstanceMemberAttrStringHashKw)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, dhash_t hash, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallIInstanceMemberAttrStringLenHashKw)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, size_t attrlen, dhash_t hash, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1, 2, 3, 5)) DREF DeeObject *(DCALL DeeType_VCallInstanceMemberAttrStringHashf)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, dhash_t hash, char const *__restrict format, va_list args);
#else /* __INTELLISENSE__ */
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallInstanceMemberAttrStringHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, dhash_t hash, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallInstanceMemberAttrStringLenHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, size_t attrlen, dhash_t hash, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallInstanceMemberAttrStringHashKw)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, dhash_t hash, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallInstanceMemberAttrStringLenHashKw)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, size_t attrlen, dhash_t hash, size_t argc, DeeObject *const *argv, DeeObject *kw);
#define DeeType_CallIInstanceMemberAttrStringHash(tp_invoker, tp_self, attr, hash, argc, argv)             DeeType_CallIInstanceMemberAttrStringHashKw(tp_invoker, tp_self, attr, hash, argc, argv, NULL)
#define DeeType_CallIInstanceMemberAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, hash, argc, argv) DeeType_CallIInstanceMemberAttrStringLenHashKw(tp_invoker, tp_self, attr, attrlen, hash, argc, argv, NULL)
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallIInstanceMemberAttrStringHashKw)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, dhash_t hash, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *(DCALL DeeType_CallIInstanceMemberAttrStringLenHashKw)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, size_t attrlen, dhash_t hash, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1, 2, 3, 5)) DREF DeeObject *(DCALL DeeType_VCallInstanceMemberAttrStringHashf)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, dhash_t hash, char const *__restrict format, va_list args);
#endif /* !__INTELLISENSE__ */
#define DeeType_CallInstanceMemberAttr(tp_invoker, tp_self, attr, argc, argv)                          DeeType_CallInstanceMemberAttrStringHash(tp_invoker, tp_self, DeeString_STR(attr), DeeString_Hash(attr), argc, argv)
#define DeeType_CallInstanceMemberAttrHash(tp_invoker, tp_self, attr, hash, argc, argv)                DeeType_CallInstanceMemberAttrStringHash(tp_invoker, tp_self, DeeString_STR(attr), hash, argc, argv)
#define DeeType_CallInstanceMemberAttrString(tp_invoker, tp_self, attr, argc, argv)                    DeeType_CallInstanceMemberAttrStringHash(tp_invoker, tp_self, attr, Dee_HashStr(attr), argc, argv)
#define DeeType_CallInstanceMemberAttrStringLen(tp_invoker, tp_self, attr, attrlen, argc, argv)        DeeType_CallInstanceMemberAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, Dee_HashPtr(attr, attrlen), argc, argv)
#define DeeType_CallInstanceMemberAttrKw(tp_invoker, tp_self, attr, argc, argv, kw)                    DeeType_CallInstanceMemberAttrStringHashKw(tp_invoker, tp_self, DeeString_STR(attr), DeeString_Hash(attr), argc, argv, kw)
#define DeeType_CallInstanceMemberAttrHashKw(tp_invoker, tp_self, attr, hash, argc, argv, kw)          DeeType_CallInstanceMemberAttrStringHashKw(tp_invoker, tp_self, DeeString_STR(attr), hash, argc, argv, kw)
#define DeeType_CallInstanceMemberAttrStringKw(tp_invoker, tp_self, attr, argc, argv, kw)              DeeType_CallInstanceMemberAttrStringHashKw(tp_invoker, tp_self, attr, Dee_HashStr(attr), argc, argv, kw)
#define DeeType_CallInstanceMemberAttrStringLenKw(tp_invoker, tp_self, attr, attrlen, argc, argv, kw)  DeeType_CallInstanceMemberAttrStringLenHashKw(tp_invoker, tp_self, attr, attrlen, Dee_HashPtr(attr, attrlen), argc, argv, kw)
#define DeeType_VCallInstanceMemberAttrf(tp_invoker, tp_self, attr, format, args)                      DeeType_VCallInstanceMemberAttrStringHashf(tp_invoker, tp_self, DeeString_STR(attr), DeeString_Hash(attr), format, args)
#define DeeType_VCallInstanceMemberAttrHashf(tp_invoker, tp_self, attr, hash, format, args)            DeeType_VCallInstanceMemberAttrStringHashf(tp_invoker, tp_self, DeeString_STR(attr), hash, format, args)
#define DeeType_VCallInstanceMemberAttrStringf(tp_invoker, tp_self, attr, format, args)                DeeType_VCallInstanceMemberAttrStringHashf(tp_invoker, tp_self, attr, Dee_HashStr(attr), format, args)
#define DeeType_CallIInstanceMemberAttr(tp_invoker, tp_self, attr, argc, argv)                         DeeType_CallIInstanceMemberAttrStringHash(tp_invoker, tp_self, DeeString_STR(attr), DeeString_Hash(attr), argc, argv)
#define DeeType_CallIInstanceMemberAttrHash(tp_invoker, tp_self, attr, hash, argc, argv)               DeeType_CallIInstanceMemberAttrStringHash(tp_invoker, tp_self, DeeString_STR(attr), hash, argc, argv)
#define DeeType_CallIInstanceMemberAttrString(tp_invoker, tp_self, attr, argc, argv)                   DeeType_CallIInstanceMemberAttrStringHash(tp_invoker, tp_self, attr, Dee_HashStr(attr), argc, argv)
#define DeeType_CallIInstanceMemberAttrStringLen(tp_invoker, tp_self, attr, attrlen, argc, argv)       DeeType_CallIInstanceMemberAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, Dee_HashPtr(attr, attrlen), argc, argv)
#define DeeType_CallIInstanceMemberAttrKw(tp_invoker, tp_self, attr, argc, argv, kw)                   DeeType_CallIInstanceMemberAttrStringHashKw(tp_invoker, tp_self, DeeString_STR(attr), DeeString_Hash(attr), argc, argv, kw)
#define DeeType_CallIInstanceMemberAttrHashKw(tp_invoker, tp_self, attr, hash, argc, argv, kw)         DeeType_CallIInstanceMemberAttrStringHashKw(tp_invoker, tp_self, DeeString_STR(attr), hash, argc, argv, kw)
#define DeeType_CallIInstanceMemberAttrStringKw(tp_invoker, tp_self, attr, argc, argv, kw)             DeeType_CallIInstanceMemberAttrStringHashKw(tp_invoker, tp_self, attr, Dee_HashStr(attr), argc, argv, kw)
#define DeeType_CallIInstanceMemberAttrStringLenKw(tp_invoker, tp_self, attr, attrlen, argc, argv, kw) DeeType_CallIInstanceMemberAttrStringLenHashKw(tp_invoker, tp_self, attr, attrlen, Dee_HashPtr(attr, attrlen), argc, argv, kw)


/* Check if attributes from `tp_self->tp_members' / `tp_self->tp_class_members' are bound.
 * @return:  1: The attribute is bound.
 * @return:  0: The attribute is unbound.
 * @return: -1: An error occurred.
 * @return: -2: The attribute could not be found in `chain'. */
#ifdef __INTELLISENSE__
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int (DCALL DeeType_BoundMemberAttrStringHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, DeeObject *self, char const *__restrict attr, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int (DCALL DeeType_BoundMemberAttrStringLenHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, DeeObject *self, char const *__restrict attr, size_t attrlen, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int (DCALL DeeType_BoundClassMemberAttrStringHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int (DCALL DeeType_BoundClassMemberAttrStringLenHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, size_t attrlen, dhash_t hash);
#else /* __INTELLISENSE__ */
INTDEF WUNUSED NONNULL((1, 2, 3, 4, 5)) int DCALL /* BOUND_MEMBER */
type_member_boundattr_string_hash(struct Dee_membercache *cache, DeeTypeObject *decl,
                                  struct type_member const *chain, DeeObject *self,
                                  char const *__restrict attr, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3, 4, 5)) int DCALL /* BOUND_MEMBER */
type_member_boundattr_string_len_hash(struct Dee_membercache *cache, DeeTypeObject *decl,
                                      struct type_member const *chain, DeeObject *self,
                                      char const *__restrict attr, size_t attrlen, dhash_t hash);
#define DeeType_BoundMemberAttrStringHash(tp_invoker, tp_self, self, attr, hash)             type_member_boundattr_string_hash(&(tp_invoker)->tp_cache, tp_self, (tp_self)->tp_members, self, attr, hash)
#define DeeType_BoundClassMemberAttrStringHash(tp_invoker, tp_self, attr, hash)              type_member_boundattr_string_hash(&(tp_invoker)->tp_class_cache, tp_self, (tp_self)->tp_class_members, (DeeObject *)(tp_invoker), attr, hash)
#define DeeType_BoundMemberAttrStringLenHash(tp_invoker, tp_self, self, attr, attrlen, hash) type_member_boundattr_string_len_hash(&(tp_invoker)->tp_cache, tp_self, (tp_self)->tp_members, self, attr, attrlen, hash)
#define DeeType_BoundClassMemberAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, hash)  type_member_boundattr_string_len_hash(&(tp_invoker)->tp_class_cache, tp_self, (tp_self)->tp_class_members, (DeeObject *)(tp_invoker), attr, attrlen, hash)
#endif /* !__INTELLISENSE__ */
#define DeeType_BoundMemberAttr(tp_invoker, tp_self, self, attr)                   DeeType_BoundMemberAttrStringHash(tp_invoker, tp_self, self, DeeString_STR(attr), DeeString_Hash(attr))
#define DeeType_BoundMemberAttrHash(tp_invoker, tp_self, self, attr, hash)         DeeType_BoundMemberAttrStringHash(tp_invoker, tp_self, self, DeeString_STR(attr), hash)
#define DeeType_BoundMemberAttrString(tp_invoker, tp_self, self, attr)             DeeType_BoundMemberAttrStringHash(tp_invoker, tp_self, self, attr, Dee_HashStr(attr))
#define DeeType_BoundMemberAttrStringLen(tp_invoker, tp_self, self, attr, attrlen) DeeType_BoundMemberAttrStringLenHash(tp_invoker, tp_self, self, attr, attrlen, Dee_HashPtr(attr, attrlen))
#define DeeType_BoundClassMemberAttr(tp_invoker, tp_self, attr)                    DeeType_BoundClassMemberAttrStringHash(tp_invoker, tp_self, DeeString_STR(attr), DeeString_Hash(attr))
#define DeeType_BoundClassMemberAttrHash(tp_invoker, tp_self, attr, hash)          DeeType_BoundClassMemberAttrStringHash(tp_invoker, tp_self, DeeString_STR(attr), hash)
#define DeeType_BoundClassMemberAttrString(tp_invoker, tp_self, attr)              DeeType_BoundClassMemberAttrStringHash(tp_invoker, tp_self, attr, Dee_HashStr(attr))
#define DeeType_BoundClassMemberAttrStringLen(tp_invoker, tp_self, attr, attrlen)  DeeType_BoundClassMemberAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, Dee_HashPtr(attr, attrlen))


/* Delete attributes from `tp_self->tp_members' / `tp_self->tp_class_members'.
 * @return:  0: Success.
 * @return: -1: An error occurred.
 * @return:  1: The attribute could not be found in `chain'. */
#ifdef __INTELLISENSE__
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int (DCALL DeeType_DelMemberAttrStringHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, DeeObject *self, char const *__restrict attr, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) int (DCALL DeeType_DelMemberAttrStringLenHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, DeeObject *self, char const *__restrict attr, size_t attrlen, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int (DCALL DeeType_DelClassMemberAttrStringHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int (DCALL DeeType_DelClassMemberAttrStringLenHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, size_t attrlen, dhash_t hash);
#else /* __INTELLISENSE__ */
INTDEF WUNUSED NONNULL((1, 2, 3, 4, 5)) int DCALL /* DEL_MEMBER */
type_member_delattr_string_hash(struct Dee_membercache *cache, DeeTypeObject *decl,
                                struct type_member const *chain, DeeObject *self,
                                char const *__restrict attr, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3, 4, 5)) int DCALL /* DEL_MEMBER */
type_member_delattr_string_len_hash(struct Dee_membercache *cache, DeeTypeObject *decl,
                                    struct type_member const *chain, DeeObject *self,
                                    char const *__restrict attr, size_t attrlen, dhash_t hash);
#define DeeType_DelMemberAttrStringHash(tp_invoker, tp_self, self, attr, hash)             type_member_delattr_string_hash(&(tp_invoker)->tp_cache, tp_self, (tp_self)->tp_members, self, attr, hash)
#define DeeType_DelClassMemberAttrStringHash(tp_invoker, tp_self, attr, hash)              type_member_delattr_string_hash(&(tp_invoker)->tp_class_cache, tp_self, (tp_self)->tp_class_members, (DeeObject *)(tp_invoker), attr, hash)
#define DeeType_DelMemberAttrStringLenHash(tp_invoker, tp_self, self, attr, attrlen, hash) type_member_delattr_string_len_hash(&(tp_invoker)->tp_cache, tp_self, (tp_self)->tp_members, self, attr, attrlen, hash)
#define DeeType_DelClassMemberAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, hash)  type_member_delattr_string_len_hash(&(tp_invoker)->tp_class_cache, tp_self, (tp_self)->tp_class_members, (DeeObject *)(tp_invoker), attr, attrlen, hash)
#endif /* !__INTELLISENSE__ */
#define DeeType_DelMemberAttr(tp_invoker, tp_self, self, attr)                   DeeType_DelMemberAttrStringHash(tp_invoker, tp_self, self, DeeString_STR(attr), DeeString_Hash(attr))
#define DeeType_DelMemberAttrHash(tp_invoker, tp_self, self, attr, hash)         DeeType_DelMemberAttrStringHash(tp_invoker, tp_self, self, DeeString_STR(attr), hash)
#define DeeType_DelMemberAttrString(tp_invoker, tp_self, self, attr)             DeeType_DelMemberAttrStringHash(tp_invoker, tp_self, self, attr, Dee_HashStr(attr))
#define DeeType_DelMemberAttrStringLen(tp_invoker, tp_self, self, attr, attrlen) DeeType_DelMemberAttrStringLenHash(tp_invoker, tp_self, self, attr, attrlen, Dee_HashPtr(attr, attrlen))
#define DeeType_DelClassMemberAttr(tp_invoker, tp_self, attr)                    DeeType_DelClassMemberAttrStringHash(tp_invoker, tp_self, DeeString_STR(attr), DeeString_Hash(attr))
#define DeeType_DelClassMemberAttrHash(tp_invoker, tp_self, attr, hash)          DeeType_DelClassMemberAttrStringHash(tp_invoker, tp_self, DeeString_STR(attr), hash)
#define DeeType_DelClassMemberAttrString(tp_invoker, tp_self, attr)              DeeType_DelClassMemberAttrStringHash(tp_invoker, tp_self, attr, Dee_HashStr(attr))
#define DeeType_DelClassMemberAttrStringLen(tp_invoker, tp_self, attr, attrlen)  DeeType_DelClassMemberAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, Dee_HashPtr(attr, attrlen))


/* Set attributes from `tp_self->tp_members' / `tp_self->tp_class_members'.
 * @return:  0: Success.
 * @return: -1: An error occurred.
 * @return:  1: The attribute could not be found in `chain'. */
#ifdef __INTELLISENSE__
INTDEF WUNUSED NONNULL((1, 2, 3, 4, 6)) int (DCALL DeeType_SetMemberAttrStringHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, DeeObject *self, char const *__restrict attr, dhash_t hash, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3, 4, 7)) int (DCALL DeeType_SetMemberAttrStringLenHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, DeeObject *self, char const *__restrict attr, size_t attrlen, dhash_t hash, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3, 5)) int (DCALL DeeType_SetClassMemberAttrStringHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, dhash_t hash, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3, 6)) int (DCALL DeeType_SetClassMemberAttrStringLenHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, size_t attrlen, dhash_t hash, DeeObject *value);
#else /* __INTELLISENSE__ */
INTDEF WUNUSED NONNULL((1, 2, 3, 4, 5, 7)) int DCALL /* SET_MEMBER */
type_member_setattr_string_hash(struct Dee_membercache *cache, DeeTypeObject *decl,
                                struct type_member const *chain, DeeObject *self,
                                char const *__restrict attr, dhash_t hash, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 3, 4, 5, 8)) int DCALL /* SET_MEMBER */
type_member_setattr_string_len_hash(struct Dee_membercache *cache, DeeTypeObject *decl,
                                    struct type_member const *chain, DeeObject *self,
                                    char const *__restrict attr, size_t attrlen,
                                    dhash_t hash, DeeObject *value);
#define DeeType_SetMemberAttrStringHash(tp_invoker, tp_self, self, attr, hash, value)             type_member_setattr_string_hash(&(tp_invoker)->tp_cache, tp_self, (tp_self)->tp_members, self, attr, hash, value)
#define DeeType_SetClassMemberAttrStringHash(tp_invoker, tp_self, attr, hash, value)              type_member_setattr_string_hash(&(tp_invoker)->tp_class_cache, tp_self, (tp_self)->tp_class_members, (DeeObject *)(tp_invoker), attr, hash, value)
#define DeeType_SetMemberAttrStringLenHash(tp_invoker, tp_self, self, attr, attrlen, hash, value) type_member_setattr_string_len_hash(&(tp_invoker)->tp_cache, tp_self, (tp_self)->tp_members, self, attr, attrlen, hash, value)
#define DeeType_SetClassMemberAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, hash, value)  type_member_setattr_string_len_hash(&(tp_invoker)->tp_class_cache, tp_self, (tp_self)->tp_class_members, (DeeObject *)(tp_invoker), attr, attrlen, hash, value)
#endif /* !__INTELLISENSE__ */
#define DeeType_SetMemberAttr(tp_invoker, tp_self, self, attr, value)                   DeeType_SetMemberAttrStringHash(tp_invoker, tp_self, self, DeeString_STR(attr), DeeString_Hash(attr), value)
#define DeeType_SetMemberAttrHash(tp_invoker, tp_self, self, attr, hash, value)         DeeType_SetMemberAttrStringHash(tp_invoker, tp_self, self, DeeString_STR(attr), hash, value)
#define DeeType_SetMemberAttrString(tp_invoker, tp_self, self, attr, value)             DeeType_SetMemberAttrStringHash(tp_invoker, tp_self, self, attr, Dee_HashStr(attr), value)
#define DeeType_SetMemberAttrStringLen(tp_invoker, tp_self, self, attr, attrlen, value) DeeType_SetMemberAttrStringLenHash(tp_invoker, tp_self, self, attr, attrlen, Dee_HashPtr(attr, attrlen), value)
#define DeeType_SetClassMemberAttr(tp_invoker, tp_self, attr, value)                    DeeType_SetClassMemberAttrStringHash(tp_invoker, tp_self, DeeString_STR(attr), DeeString_Hash(attr), value)
#define DeeType_SetClassMemberAttrHash(tp_invoker, tp_self, attr, hash, value)          DeeType_SetClassMemberAttrStringHash(tp_invoker, tp_self, DeeString_STR(attr), hash, value)
#define DeeType_SetClassMemberAttrString(tp_invoker, tp_self, attr, value)              DeeType_SetClassMemberAttrStringHash(tp_invoker, tp_self, attr, Dee_HashStr(attr), value)
#define DeeType_SetClassMemberAttrStringLen(tp_invoker, tp_self, attr, attrlen, value)  DeeType_SetClassMemberAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, Dee_HashPtr(attr, attrlen), value)


/* Check for the existence of specific attributes. */
#ifdef __INTELLISENSE__
INTDEF WUNUSED NONNULL((1, 2, 3)) bool (DCALL DeeType_HasMethodAttrStringHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) bool (DCALL DeeType_HasMethodAttrStringLenHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, size_t attrlen, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) bool (DCALL DeeType_HasClassMethodAttrStringHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) bool (DCALL DeeType_HasClassMethodAttrStringLenHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, size_t attrlen, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) bool (DCALL DeeType_HasInstanceMethodAttrStringHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) bool (DCALL DeeType_HasInstanceMethodAttrStringLenHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, size_t attrlen, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) bool (DCALL DeeType_HasIInstanceMethodAttrStringHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) bool (DCALL DeeType_HasIInstanceMethodAttrStringLenHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, size_t attrlen, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) bool (DCALL DeeType_HasGetSetAttrStringHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) bool (DCALL DeeType_HasGetSetAttrStringLenHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, size_t attrlen, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) bool (DCALL DeeType_HasClassGetSetAttrStringHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) bool (DCALL DeeType_HasClassGetSetAttrStringLenHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, size_t attrlen, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) bool (DCALL DeeType_HasInstanceGetSetAttrStringHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) bool (DCALL DeeType_HasInstanceGetSetAttrStringLenHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, size_t attrlen, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) bool (DCALL DeeType_HasIInstanceGetSetAttrStringHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) bool (DCALL DeeType_HasIInstanceGetSetAttrStringLenHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, size_t attrlen, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) bool (DCALL DeeType_HasMemberAttrStringHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) bool (DCALL DeeType_HasMemberAttrStringLenHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, size_t attrlen, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) bool (DCALL DeeType_HasClassMemberAttrStringHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) bool (DCALL DeeType_HasClassMemberAttrStringLenHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, size_t attrlen, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) bool (DCALL DeeType_HasInstanceMemberAttrStringHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) bool (DCALL DeeType_HasInstanceMemberAttrStringLenHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, size_t attrlen, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) bool (DCALL DeeType_HasIInstanceMemberAttrStringHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) bool (DCALL DeeType_HasIInstanceMemberAttrStringLenHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, size_t attrlen, dhash_t hash);
#else /* __INTELLISENSE__ */
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) bool DCALL /* METHOD */
type_method_hasattr_string_hash(struct Dee_membercache *cache, DeeTypeObject *decl,
                                struct type_method const *chain,
                                char const *__restrict attr, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) bool DCALL /* METHOD */
type_method_hasattr_string_len_hash(struct Dee_membercache *cache, DeeTypeObject *decl,
                                    struct type_method const *chain,
                                    char const *__restrict attr,
                                    size_t attrlen, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) bool DCALL /* GETSET */
type_getset_hasattr_string_hash(struct Dee_membercache *cache, DeeTypeObject *decl,
                                struct type_getset const *chain,
                                char const *__restrict attr, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) bool DCALL /* GETSET */
type_getset_hasattr_string_len_hash(struct Dee_membercache *cache, DeeTypeObject *decl,
                                    struct type_getset const *chain,
                                    char const *__restrict attr,
                                    size_t attrlen, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) bool DCALL /* MEMBER */
type_member_hasattr_string_hash(struct Dee_membercache *cache, DeeTypeObject *decl,
                                struct type_member const *chain,
                                char const *__restrict attr, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3, 4)) bool DCALL /* MEMBER */
type_member_hasattr_string_len_hash(struct Dee_membercache *cache, DeeTypeObject *decl,
                                    struct type_member const *chain,
                                    char const *__restrict attr,
                                    size_t attrlen, dhash_t hash);
#define DeeType_HasMethodAttrStringHash(tp_invoker, tp_self, attr, hash)                      type_method_hasattr_string_hash(&(tp_invoker)->tp_cache, tp_self, (tp_self)->tp_methods, attr, hash)
#define DeeType_HasMethodAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, hash)          type_method_hasattr_string_len_hash(&(tp_invoker)->tp_cache, tp_self, (tp_self)->tp_methods, attr, attrlen, hash)
#define DeeType_HasClassMethodAttrStringHash(tp_invoker, tp_self, attr, hash)                 type_method_hasattr_string_hash(&(tp_invoker)->tp_class_cache, tp_self, (tp_self)->tp_class_methods, attr, hash)
#define DeeType_HasClassMethodAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, hash)     type_method_hasattr_string_len_hash(&(tp_invoker)->tp_class_cache, tp_self, (tp_self)->tp_class_methods, attr, attrlen, hash)
INTDEF WUNUSED NONNULL((1, 2, 3)) bool (DCALL DeeType_HasInstanceMethodAttrStringHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) bool (DCALL DeeType_HasInstanceMethodAttrStringLenHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, size_t attrlen, dhash_t hash);
#define DeeType_HasIInstanceMethodAttrStringHash(tp_invoker, tp_self, attr, hash)             type_method_hasattr_string_hash(&(tp_invoker)->tp_cache, tp_self, (tp_self)->tp_methods, attr, hash)
#define DeeType_HasIInstanceMethodAttrStringLenHash(tp_invoker, tp_self, attr, hash)          type_method_hasattr_string_len_hash(&(tp_invoker)->tp_cache, tp_self, (tp_self)->tp_methods, attr, attrlen, hash)
#define DeeType_HasGetSetAttrStringHash(tp_invoker, tp_self, attr, hash)                      type_getset_hasattr_string_hash(&(tp_invoker)->tp_cache, tp_self, (tp_self)->tp_getsets, attr, hash)
#define DeeType_HasClassGetSetAttrStringHash(tp_invoker, tp_self, attr, hash)                 type_getset_hasattr_string_hash(&(tp_invoker)->tp_class_cache, tp_self, (tp_self)->tp_class_getsets, attr, hash)
#define DeeType_HasGetSetAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, hash)          type_getset_hasattr_string_len_hash(&(tp_invoker)->tp_cache, tp_self, (tp_self)->tp_getsets, attr, attrlen, hash)
#define DeeType_HasClassGetSetAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, hash)     type_getset_hasattr_string_len_hash(&(tp_invoker)->tp_class_cache, tp_self, (tp_self)->tp_class_getsets, attr, attrlen, hash)
INTDEF WUNUSED NONNULL((1, 2, 3)) bool (DCALL DeeType_HasInstanceGetSetAttrStringHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) bool (DCALL DeeType_HasInstanceGetSetAttrStringLenHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, size_t attrlen, dhash_t hash);
#define DeeType_HasIInstanceGetSetAttrStringHash(tp_invoker, tp_self, attr, hash)             type_getset_hasattr_string_hash(&(tp_invoker)->tp_cache, tp_self, (tp_self)->tp_getsets, attr, hash)
#define DeeType_HasIInstanceGetSetAttrStringLenHash(tp_invoker, tp_self, attr, hash)          type_getset_hasattr_string_hash(&(tp_invoker)->tp_cache, tp_self, (tp_self)->tp_getsets, attr, attrlen, hash)
#define DeeType_HasMemberAttrStringHash(tp_invoker, tp_self, attr, hash)                      type_member_hasattr_string_hash(&(tp_invoker)->tp_cache, tp_self, (tp_self)->tp_members, attr, hash)
#define DeeType_HasClassMemberAttrStringHash(tp_invoker, tp_self, attr, hash)                 type_member_hasattr_string_hash(&(tp_invoker)->tp_class_cache, tp_self, (tp_self)->tp_class_members, attr, hash)
#define DeeType_HasMemberAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, hash)          type_member_hasattr_string_len_hash(&(tp_invoker)->tp_cache, tp_self, (tp_self)->tp_members, attr, attrlen, hash)
#define DeeType_HasClassMemberAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, hash) type_member_hasattr_string_len_hash(&(tp_invoker)->tp_class_cache, tp_self, (tp_self)->tp_class_members, attr, attrlen, hash)
INTDEF WUNUSED NONNULL((1, 2, 3)) bool (DCALL DeeType_HasInstanceMemberAttrStringHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) bool (DCALL DeeType_HasInstanceMemberAttrStringLenHash)(DeeTypeObject *tp_invoker, DeeTypeObject *tp_self, char const *__restrict attr, size_t attrlen, dhash_t hash);
#define DeeType_HasIInstanceMemberAttrStringHash(tp_invoker, tp_self, attr, hash)             type_member_hasattr_string_hash(&(tp_invoker)->tp_cache, tp_self, (tp_self)->tp_members, attr, hash)
#define DeeType_HasIInstanceMemberAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, hash) type_member_hasattr_string_len_hash(&(tp_invoker)->tp_cache, tp_self, (tp_self)->tp_members, attr, attrlen, hash)
#endif /* !__INTELLISENSE__ */
#define DeeType_HasMethodAttr(tp_invoker, tp_self, attr)                            DeeType_HasMethodAttrStringHash(tp_invoker, tp_self, DeeString_STR(attr), DeeString_Hash(attr))
#define DeeType_HasMethodAttrHash(tp_invoker, tp_self, attr, hash)                  DeeType_HasMethodAttrStringHash(tp_invoker, tp_self, DeeString_STR(attr), hash)
#define DeeType_HasMethodAttrString(tp_invoker, tp_self, attr)                      DeeType_HasMethodAttrStringHash(tp_invoker, tp_self, attr, Dee_HashStr(attr))
#define DeeType_HasMethodAttrStringLen(tp_invoker, tp_self, attr, attrlen)          DeeType_HasMethodAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, Dee_HashPtr(attr, attrlen))
#define DeeType_HasClassMethodAttr(tp_invoker, tp_self, attr)                       DeeType_HasClassMethodAttrStringHash(tp_invoker, tp_self, DeeString_STR(attr), DeeString_Hash(attr))
#define DeeType_HasClassMethodAttrHash(tp_invoker, tp_self, attr, hash)             DeeType_HasClassMethodAttrStringHash(tp_invoker, tp_self, DeeString_STR(attr), hash)
#define DeeType_HasClassMethodAttrString(tp_invoker, tp_self, attr)                 DeeType_HasClassMethodAttrStringHash(tp_invoker, tp_self, attr, Dee_HashStr(attr))
#define DeeType_HasClassMethodAttrStringLen(tp_invoker, tp_self, attr, attrlen)     DeeType_HasClassMethodAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, Dee_HashPtr(attr, attrlen))
#define DeeType_HasInstanceMethodAttr(tp_invoker, tp_self, attr)                    DeeType_HasInstanceMethodAttrStringHash(tp_invoker, tp_self, DeeString_STR(attr), DeeString_Hash(attr))
#define DeeType_HasInstanceMethodAttrHash(tp_invoker, tp_self, attr, hash)          DeeType_HasInstanceMethodAttrStringHash(tp_invoker, tp_self, DeeString_STR(attr), hash)
#define DeeType_HasInstanceMethodAttrString(tp_invoker, tp_self, attr)              DeeType_HasInstanceMethodAttrStringHash(tp_invoker, tp_self, attr, Dee_HashStr(attr))
#define DeeType_HasInstanceMethodAttrStringLen(tp_invoker, tp_self, attr, attrlen)  DeeType_HasInstanceMethodAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, Dee_HashPtr(attr, attrlen))
#define DeeType_HasIInstanceMethodAttr(tp_invoker, tp_self, attr)                   DeeType_HasIInstanceMethodAttrStringHash(tp_invoker, tp_self, DeeString_STR(attr), DeeString_Hash(attr))
#define DeeType_HasIInstanceMethodAttrHash(tp_invoker, tp_self, attr, hash)         DeeType_HasIInstanceMethodAttrStringHash(tp_invoker, tp_self, DeeString_STR(attr), hash)
#define DeeType_HasIInstanceMethodAttrString(tp_invoker, tp_self, attr)             DeeType_HasIInstanceMethodAttrStringHash(tp_invoker, tp_self, attr, Dee_HashStr(attr))
#define DeeType_HasIInstanceMethodAttrStringLen(tp_invoker, tp_self, attr, attrlen) DeeType_HasIInstanceMethodAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, Dee_HashPtr(attr, attrlen))
#define DeeType_HasGetSetAttr(tp_invoker, tp_self, attr)                            DeeType_HasGetSetAttrStringHash(tp_invoker, tp_self, DeeString_STR(attr), DeeString_Hash(attr))
#define DeeType_HasGetSetAttrHash(tp_invoker, tp_self, attr, hash)                  DeeType_HasGetSetAttrStringHash(tp_invoker, tp_self, DeeString_STR(attr), hash)
#define DeeType_HasGetSetAttrString(tp_invoker, tp_self, attr)                      DeeType_HasGetSetAttrStringHash(tp_invoker, tp_self, attr, Dee_HashStr(attr))
#define DeeType_HasGetSetAttrStringLen(tp_invoker, tp_self, attr, attrlen)          DeeType_HasGetSetAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, Dee_HashPtr(attr, attrlen))
#define DeeType_HasClassGetSetAttr(tp_invoker, tp_self, attr)                       DeeType_HasClassGetSetAttrStringHash(tp_invoker, tp_self, DeeString_STR(attr), DeeString_Hash(attr))
#define DeeType_HasClassGetSetAttrHash(tp_invoker, tp_self, attr, hash)             DeeType_HasClassGetSetAttrStringHash(tp_invoker, tp_self, DeeString_STR(attr), hash)
#define DeeType_HasClassGetSetAttrString(tp_invoker, tp_self, attr)                 DeeType_HasClassGetSetAttrStringHash(tp_invoker, tp_self, attr, Dee_HashStr(attr))
#define DeeType_HasClassGetSetAttrStringLen(tp_invoker, tp_self, attr, attrlen)     DeeType_HasClassGetSetAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, Dee_HashPtr(attr, attrlen))
#define DeeType_HasInstanceGetSetAttr(tp_invoker, tp_self, attr)                    DeeType_HasInstanceGetSetAttrStringHash(tp_invoker, tp_self, DeeString_STR(attr), DeeString_Hash(attr))
#define DeeType_HasInstanceGetSetAttrHash(tp_invoker, tp_self, attr, hash)          DeeType_HasInstanceGetSetAttrStringHash(tp_invoker, tp_self, DeeString_STR(attr), hash)
#define DeeType_HasInstanceGetSetAttrString(tp_invoker, tp_self, attr)              DeeType_HasInstanceGetSetAttrStringHash(tp_invoker, tp_self, attr, Dee_HashStr(attr))
#define DeeType_HasInstanceGetSetAttrStringLen(tp_invoker, tp_self, attr, attrlen)  DeeType_HasInstanceGetSetAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, Dee_HashPtr(attr, attrlen))
#define DeeType_HasIInstanceGetSetAttr(tp_invoker, tp_self, attr)                   DeeType_HasIInstanceGetSetAttrStringHash(tp_invoker, tp_self, DeeString_STR(attr), DeeString_Hash(attr))
#define DeeType_HasIInstanceGetSetAttrHash(tp_invoker, tp_self, attr, hash)         DeeType_HasIInstanceGetSetAttrStringHash(tp_invoker, tp_self, DeeString_STR(attr), hash)
#define DeeType_HasIInstanceGetSetAttrString(tp_invoker, tp_self, attr)             DeeType_HasIInstanceGetSetAttrStringHash(tp_invoker, tp_self, attr, Dee_HashStr(attr))
#define DeeType_HasIInstanceGetSetAttrStringLen(tp_invoker, tp_self, attr, attrlen) DeeType_HasIInstanceGetSetAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, Dee_HashPtr(attr, attrlen))
#define DeeType_HasMemberAttr(tp_invoker, tp_self, attr)                            DeeType_HasMemberAttrStringHash(tp_invoker, tp_self, DeeString_STR(attr), DeeString_Hash(attr))
#define DeeType_HasMemberAttrHash(tp_invoker, tp_self, attr, hash)                  DeeType_HasMemberAttrStringHash(tp_invoker, tp_self, DeeString_STR(attr), hash)
#define DeeType_HasMemberAttrString(tp_invoker, tp_self, attr)                      DeeType_HasMemberAttrStringHash(tp_invoker, tp_self, attr, Dee_HashStr(attr))
#define DeeType_HasMemberAttrStringLen(tp_invoker, tp_self, attr, attrlen)          DeeType_HasMemberAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, Dee_HashPtr(attr, attrlen))
#define DeeType_HasClassMemberAttr(tp_invoker, tp_self, attr)                       DeeType_HasClassMemberAttrStringHash(tp_invoker, tp_self, DeeString_STR(attr), DeeString_Hash(attr))
#define DeeType_HasClassMemberAttrHash(tp_invoker, tp_self, attr, hash)             DeeType_HasClassMemberAttrStringHash(tp_invoker, tp_self, DeeString_STR(attr), hash)
#define DeeType_HasClassMemberAttrString(tp_invoker, tp_self, attr)                 DeeType_HasClassMemberAttrStringHash(tp_invoker, tp_self, attr, Dee_HashStr(attr))
#define DeeType_HasClassMemberAttrStringLen(tp_invoker, tp_self, attr, attrlen)     DeeType_HasClassMemberAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, Dee_HashPtr(attr, attrlen))
#define DeeType_HasInstanceMemberAttr(tp_invoker, tp_self, attr)                    DeeType_HasInstanceMemberAttrStringHash(tp_invoker, tp_self, DeeString_STR(attr), DeeString_Hash(attr))
#define DeeType_HasInstanceMemberAttrHash(tp_invoker, tp_self, attr, hash)          DeeType_HasInstanceMemberAttrStringHash(tp_invoker, tp_self, DeeString_STR(attr), hash)
#define DeeType_HasInstanceMemberAttrString(tp_invoker, tp_self, attr)              DeeType_HasInstanceMemberAttrStringHash(tp_invoker, tp_self, attr, Dee_HashStr(attr))
#define DeeType_HasInstanceMemberAttrStringLen(tp_invoker, tp_self, attr, attrlen)  DeeType_HasInstanceMemberAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, Dee_HashPtr(attr, attrlen))
#define DeeType_HasIInstanceMemberAttr(tp_invoker, tp_self, attr)                   DeeType_HasIInstanceMemberAttrStringHash(tp_invoker, tp_self, DeeString_STR(attr), DeeString_Hash(attr))
#define DeeType_HasIInstanceMemberAttrHash(tp_invoker, tp_self, attr, hash)         DeeType_HasIInstanceMemberAttrStringHash(tp_invoker, tp_self, DeeString_STR(attr), hash)
#define DeeType_HasIInstanceMemberAttrString(tp_invoker, tp_self, attr)             DeeType_HasIInstanceMemberAttrStringHash(tp_invoker, tp_self, attr, Dee_HashStr(attr))
#define DeeType_HasIInstanceMemberAttrStringLen(tp_invoker, tp_self, attr, attrlen) DeeType_HasIInstanceMemberAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, Dee_HashPtr(attr, attrlen))


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


/* Misc. functions here for completeness, but don't *really* make
 * sense since they only throw errors when an attribute is found. */
#define DeeType_BoundMethodAttr(tp_invoker, tp_self, self, attr)                                  (DeeType_HasMethodAttr(tp_invoker, tp_self, attr, hash) ? 1 : -2)
#define DeeType_BoundMethodAttrHash(tp_invoker, tp_self, self, attr, hash)                        (DeeType_HasMethodAttrHash(tp_invoker, tp_self, attr, hash) ? 1 : -2)
#define DeeType_BoundMethodAttrString(tp_invoker, tp_self, self, attr)                            (DeeType_HasMethodAttrString(tp_invoker, tp_self, attr, hash) ? 1 : -2)
#define DeeType_BoundMethodAttrStringHash(tp_invoker, tp_self, self, attr, hash)                  (DeeType_HasMethodAttrStringHash(tp_invoker, tp_self, attr, hash) ? 1 : -2)
#define DeeType_BoundMethodAttrStringLen(tp_invoker, tp_self, self, attr, attrlen)                (DeeType_HasMethodAttrStringLen(tp_invoker, tp_self, attr, attrlen) ? 1 : -2)
#define DeeType_BoundMethodAttrStringLenHash(tp_invoker, tp_self, self, attr, attrlen, hash)      (DeeType_HasMethodAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, hash) ? 1 : -2)
#define DeeType_DelMethodAttr(tp_invoker, tp_self, self, attr)                                    (DeeType_HasMethodAttr(tp_invoker, tp_self, attr, hash) ? err_cant_access_attribute(tp_self, attr, ATTR_ACCESS_DEL) : 1)
#define DeeType_DelMethodAttrHash(tp_invoker, tp_self, self, attr, hash)                          (DeeType_HasMethodAttrHash(tp_invoker, tp_self, attr, hash) ? err_cant_access_attribute(tp_self, attr, ATTR_ACCESS_DEL) : 1)
#define DeeType_DelMethodAttrString(tp_invoker, tp_self, self, attr)                              (DeeType_HasMethodAttrString(tp_invoker, tp_self, attr, hash) ? err_cant_access_attribute_string(tp_self, attr, ATTR_ACCESS_DEL) : 1)
#define DeeType_DelMethodAttrStringHash(tp_invoker, tp_self, self, attr, hash)                    (DeeType_HasMethodAttrStringHash(tp_invoker, tp_self, attr, hash) ? err_cant_access_attribute_string(tp_self, attr, ATTR_ACCESS_DEL) : 1)
#define DeeType_DelMethodAttrStringLen(tp_invoker, tp_self, self, attr, attrlen)                  (DeeType_HasMethodAttrStringLen(tp_invoker, tp_self, attr, attrlen) ? err_cant_access_attribute_string_len(tp_self, attr, attrlen, ATTR_ACCESS_DEL) : 1)
#define DeeType_DelMethodAttrStringLenHash(tp_invoker, tp_self, self, attr, attrlen, hash)        (DeeType_HasMethodAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, hash) ? err_cant_access_attribute_string_len(tp_self, attr, attrlen, ATTR_ACCESS_DEL) : 1)
#define DeeType_SetMethodAttr(tp_invoker, tp_self, self, attr, value)                             (DeeType_HasMethodAttr(tp_invoker, tp_self, attr, hash) ? err_cant_access_attribute(tp_self, attr, ATTR_ACCESS_SET) : 1)
#define DeeType_SetMethodAttrHash(tp_invoker, tp_self, self, attr, hash, value)                   (DeeType_HasMethodAttrHash(tp_invoker, tp_self, attr, hash) ? err_cant_access_attribute(tp_self, attr, ATTR_ACCESS_SET) : 1)
#define DeeType_SetMethodAttrString(tp_invoker, tp_self, self, attr, value)                       (DeeType_HasMethodAttrString(tp_invoker, tp_self, attr, hash) ? err_cant_access_attribute_string(tp_self, attr, ATTR_ACCESS_SET) : 1)
#define DeeType_SetMethodAttrStringHash(tp_invoker, tp_self, self, attr, hash, value)             (DeeType_HasMethodAttrStringHash(tp_invoker, tp_self, attr, hash) ? err_cant_access_attribute_string(tp_self, attr, ATTR_ACCESS_SET) : 1)
#define DeeType_SetMethodAttrStringLen(tp_invoker, tp_self, self, attr, attrlen, value)           (DeeType_HasMethodAttrStringLen(tp_invoker, tp_self, attr, attrlen) ? err_cant_access_attribute_string_len(tp_self, attr, attrlen, ATTR_ACCESS_SET) : 1)
#define DeeType_SetMethodAttrStringLenHash(tp_invoker, tp_self, self, attr, attrlen, hash, value) (DeeType_HasMethodAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, hash) ? err_cant_access_attribute_string_len(tp_self, attr, attrlen, ATTR_ACCESS_SET) : 1)

#define DeeType_BoundClassMethodAttr(tp_invoker, tp_self, attr)                                  (DeeType_HasClassMethodAttr(tp_invoker, tp_self, attr, hash) ? 1 : -2)
#define DeeType_BoundClassMethodAttrHash(tp_invoker, tp_self, attr, hash)                        (DeeType_HasClassMethodAttrHash(tp_invoker, tp_self, attr, hash) ? 1 : -2)
#define DeeType_BoundClassMethodAttrString(tp_invoker, tp_self, attr)                            (DeeType_HasClassMethodAttrString(tp_invoker, tp_self, attr, hash) ? 1 : -2)
#define DeeType_BoundClassMethodAttrStringHash(tp_invoker, tp_self, attr, hash)                  (DeeType_HasClassMethodAttrStringHash(tp_invoker, tp_self, attr, hash) ? 1 : -2)
#define DeeType_BoundClassMethodAttrStringLen(tp_invoker, tp_self, attr, attrlen)                (DeeType_HasClassMethodAttrStringLen(tp_invoker, tp_self, attr, attrlen) ? 1 : -2)
#define DeeType_BoundClassMethodAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, hash)      (DeeType_HasClassMethodAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, hash) ? 1 : -2)
#define DeeType_DelClassMethodAttr(tp_invoker, tp_self, attr)                                    (DeeType_HasClassMethodAttr(tp_invoker, tp_self, attr, hash) ? err_cant_access_attribute(tp_self, attr, ATTR_ACCESS_DEL) : 1)
#define DeeType_DelClassMethodAttrHash(tp_invoker, tp_self, attr, hash)                          (DeeType_HasClassMethodAttrHash(tp_invoker, tp_self, attr, hash) ? err_cant_access_attribute(tp_self, attr, ATTR_ACCESS_DEL) : 1)
#define DeeType_DelClassMethodAttrString(tp_invoker, tp_self, attr)                              (DeeType_HasClassMethodAttrString(tp_invoker, tp_self, attr, hash) ? err_cant_access_attribute_string(tp_self, attr, ATTR_ACCESS_DEL) : 1)
#define DeeType_DelClassMethodAttrStringHash(tp_invoker, tp_self, attr, hash)                    (DeeType_HasClassMethodAttrStringHash(tp_invoker, tp_self, attr, hash) ? err_cant_access_attribute_string(tp_self, attr, ATTR_ACCESS_DEL) : 1)
#define DeeType_DelClassMethodAttrStringLen(tp_invoker, tp_self, attr, attrlen)                  (DeeType_HasClassMethodAttrStringLen(tp_invoker, tp_self, attr, attrlen) ? err_cant_access_attribute_string_len(tp_self, attr, attrlen, ATTR_ACCESS_DEL) : 1)
#define DeeType_DelClassMethodAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, hash)        (DeeType_HasClassMethodAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, hash) ? err_cant_access_attribute_string_len(tp_self, attr, attrlen, ATTR_ACCESS_DEL) : 1)
#define DeeType_SetClassMethodAttr(tp_invoker, tp_self, attr, value)                             (DeeType_HasClassMethodAttr(tp_invoker, tp_self, attr, hash) ? err_cant_access_attribute(tp_self, attr, ATTR_ACCESS_SET) : 1)
#define DeeType_SetClassMethodAttrHash(tp_invoker, tp_self, attr, hash, value)                   (DeeType_HasClassMethodAttrHash(tp_invoker, tp_self, attr, hash) ? err_cant_access_attribute(tp_self, attr, ATTR_ACCESS_SET) : 1)
#define DeeType_SetClassMethodAttrString(tp_invoker, tp_self, attr, value)                       (DeeType_HasClassMethodAttrString(tp_invoker, tp_self, attr, hash) ? err_cant_access_attribute_string(tp_self, attr, ATTR_ACCESS_SET) : 1)
#define DeeType_SetClassMethodAttrStringHash(tp_invoker, tp_self, attr, hash, value)             (DeeType_HasClassMethodAttrStringHash(tp_invoker, tp_self, attr, hash) ? err_cant_access_attribute_string(tp_self, attr, ATTR_ACCESS_SET) : 1)
#define DeeType_SetClassMethodAttrStringLen(tp_invoker, tp_self, attr, attrlen, value)           (DeeType_HasClassMethodAttrStringLen(tp_invoker, tp_self, attr, attrlen) ? err_cant_access_attribute_string_len(tp_self, attr, attrlen, ATTR_ACCESS_SET) : 1)
#define DeeType_SetClassMethodAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, hash, value) (DeeType_HasClassMethodAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, hash) ? err_cant_access_attribute_string_len(tp_self, attr, attrlen, ATTR_ACCESS_SET) : 1)

#define DeeType_BoundInstanceMethodAttr(tp_invoker, tp_self, attr)                                  (DeeType_HasInstanceMethodAttr(tp_invoker, tp_self, attr, hash) ? 1 : -2)
#define DeeType_BoundInstanceMethodAttrHash(tp_invoker, tp_self, attr, hash)                        (DeeType_HasInstanceMethodAttrHash(tp_invoker, tp_self, attr, hash) ? 1 : -2)
#define DeeType_BoundInstanceMethodAttrString(tp_invoker, tp_self, attr)                            (DeeType_HasInstanceMethodAttrString(tp_invoker, tp_self, attr, hash) ? 1 : -2)
#define DeeType_BoundInstanceMethodAttrStringHash(tp_invoker, tp_self, attr, hash)                  (DeeType_HasInstanceMethodAttrStringHash(tp_invoker, tp_self, attr, hash) ? 1 : -2)
#define DeeType_BoundInstanceMethodAttrStringLen(tp_invoker, tp_self, attr, attrlen)                (DeeType_HasInstanceMethodAttrStringLen(tp_invoker, tp_self, attr, attrlen) ? 1 : -2)
#define DeeType_BoundInstanceMethodAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, hash)      (DeeType_HasInstanceMethodAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, hash) ? 1 : -2)
#define DeeType_DelInstanceMethodAttr(tp_invoker, tp_self, attr)                                    (DeeType_HasInstanceMethodAttr(tp_invoker, tp_self, attr, hash) ? err_cant_access_attribute(tp_self, attr, ATTR_ACCESS_DEL) : 1)
#define DeeType_DelInstanceMethodAttrHash(tp_invoker, tp_self, attr, hash)                          (DeeType_HasInstanceMethodAttrHash(tp_invoker, tp_self, attr, hash) ? err_cant_access_attribute(tp_self, attr, ATTR_ACCESS_DEL) : 1)
#define DeeType_DelInstanceMethodAttrString(tp_invoker, tp_self, attr)                              (DeeType_HasInstanceMethodAttrString(tp_invoker, tp_self, attr, hash) ? err_cant_access_attribute_string(tp_self, attr, ATTR_ACCESS_DEL) : 1)
#define DeeType_DelInstanceMethodAttrStringHash(tp_invoker, tp_self, attr, hash)                    (DeeType_HasInstanceMethodAttrStringHash(tp_invoker, tp_self, attr, hash) ? err_cant_access_attribute_string(tp_self, attr, ATTR_ACCESS_DEL) : 1)
#define DeeType_DelInstanceMethodAttrStringLen(tp_invoker, tp_self, attr, attrlen)                  (DeeType_HasInstanceMethodAttrStringLen(tp_invoker, tp_self, attr, attrlen) ? err_cant_access_attribute_string_len(tp_self, attr, attrlen, ATTR_ACCESS_DEL) : 1)
#define DeeType_DelInstanceMethodAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, hash)        (DeeType_HasInstanceMethodAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, hash) ? err_cant_access_attribute_string_len(tp_self, attr, attrlen, ATTR_ACCESS_DEL) : 1)
#define DeeType_SetInstanceMethodAttr(tp_invoker, tp_self, attr, value)                             (DeeType_HasInstanceMethodAttr(tp_invoker, tp_self, attr, hash) ? err_cant_access_attribute(tp_self, attr, ATTR_ACCESS_SET) : 1)
#define DeeType_SetInstanceMethodAttrHash(tp_invoker, tp_self, attr, hash, value)                   (DeeType_HasInstanceMethodAttrHash(tp_invoker, tp_self, attr, hash) ? err_cant_access_attribute(tp_self, attr, ATTR_ACCESS_SET) : 1)
#define DeeType_SetInstanceMethodAttrString(tp_invoker, tp_self, attr, value)                       (DeeType_HasInstanceMethodAttrString(tp_invoker, tp_self, attr, hash) ? err_cant_access_attribute_string(tp_self, attr, ATTR_ACCESS_SET) : 1)
#define DeeType_SetInstanceMethodAttrStringHash(tp_invoker, tp_self, attr, hash, value)             (DeeType_HasInstanceMethodAttrStringHash(tp_invoker, tp_self, attr, hash) ? err_cant_access_attribute_string(tp_self, attr, ATTR_ACCESS_SET) : 1)
#define DeeType_SetInstanceMethodAttrStringLen(tp_invoker, tp_self, attr, attrlen, value)           (DeeType_HasInstanceMethodAttrStringLen(tp_invoker, tp_self, attr, attrlen) ? err_cant_access_attribute_string_len(tp_self, attr, attrlen, ATTR_ACCESS_SET) : 1)
#define DeeType_SetInstanceMethodAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, hash, value) (DeeType_HasInstanceMethodAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, hash) ? err_cant_access_attribute_string_len(tp_self, attr, attrlen, ATTR_ACCESS_SET) : 1)

#define DeeType_BoundInstanceGetSetAttr(tp_invoker, tp_self, attr)                                  (DeeType_HasInstanceGetSetAttr(tp_invoker, tp_self, attr, hash) ? 1 : -2)
#define DeeType_BoundInstanceGetSetAttrHash(tp_invoker, tp_self, attr, hash)                        (DeeType_HasInstanceGetSetAttrHash(tp_invoker, tp_self, attr, hash) ? 1 : -2)
#define DeeType_BoundInstanceGetSetAttrString(tp_invoker, tp_self, attr)                            (DeeType_HasInstanceGetSetAttrString(tp_invoker, tp_self, attr, hash) ? 1 : -2)
#define DeeType_BoundInstanceGetSetAttrStringHash(tp_invoker, tp_self, attr, hash)                  (DeeType_HasInstanceGetSetAttrStringHash(tp_invoker, tp_self, attr, hash) ? 1 : -2)
#define DeeType_BoundInstanceGetSetAttrStringLen(tp_invoker, tp_self, attr, attrlen)                (DeeType_HasInstanceGetSetAttrStringLen(tp_invoker, tp_self, attr, attrlen) ? 1 : -2)
#define DeeType_BoundInstanceGetSetAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, hash)      (DeeType_HasInstanceGetSetAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, hash) ? 1 : -2)
#define DeeType_DelInstanceGetSetAttr(tp_invoker, tp_self, attr)                                    (DeeType_HasInstanceGetSetAttr(tp_invoker, tp_self, attr, hash) ? err_cant_access_attribute(tp_self, attr, ATTR_ACCESS_DEL) : 1)
#define DeeType_DelInstanceGetSetAttrHash(tp_invoker, tp_self, attr, hash)                          (DeeType_HasInstanceGetSetAttrHash(tp_invoker, tp_self, attr, hash) ? err_cant_access_attribute(tp_self, attr, ATTR_ACCESS_DEL) : 1)
#define DeeType_DelInstanceGetSetAttrString(tp_invoker, tp_self, attr)                              (DeeType_HasInstanceGetSetAttrString(tp_invoker, tp_self, attr, hash) ? err_cant_access_attribute_string(tp_self, attr, ATTR_ACCESS_DEL) : 1)
#define DeeType_DelInstanceGetSetAttrStringHash(tp_invoker, tp_self, attr, hash)                    (DeeType_HasInstanceGetSetAttrStringHash(tp_invoker, tp_self, attr, hash) ? err_cant_access_attribute_string(tp_self, attr, ATTR_ACCESS_DEL) : 1)
#define DeeType_DelInstanceGetSetAttrStringLen(tp_invoker, tp_self, attr, attrlen)                  (DeeType_HasInstanceGetSetAttrStringLen(tp_invoker, tp_self, attr, attrlen) ? err_cant_access_attribute_string_len(tp_self, attr, attrlen, ATTR_ACCESS_DEL) : 1)
#define DeeType_DelInstanceGetSetAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, hash)        (DeeType_HasInstanceGetSetAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, hash) ? err_cant_access_attribute_string_len(tp_self, attr, attrlen, ATTR_ACCESS_DEL) : 1)
#define DeeType_SetInstanceGetSetAttr(tp_invoker, tp_self, attr, value)                             (DeeType_HasInstanceGetSetAttr(tp_invoker, tp_self, attr, hash) ? err_cant_access_attribute(tp_self, attr, ATTR_ACCESS_SET) : 1)
#define DeeType_SetInstanceGetSetAttrHash(tp_invoker, tp_self, attr, hash, value)                   (DeeType_HasInstanceGetSetAttrHash(tp_invoker, tp_self, attr, hash) ? err_cant_access_attribute(tp_self, attr, ATTR_ACCESS_SET) : 1)
#define DeeType_SetInstanceGetSetAttrString(tp_invoker, tp_self, attr, value)                       (DeeType_HasInstanceGetSetAttrString(tp_invoker, tp_self, attr, hash) ? err_cant_access_attribute_string(tp_self, attr, ATTR_ACCESS_SET) : 1)
#define DeeType_SetInstanceGetSetAttrStringHash(tp_invoker, tp_self, attr, hash, value)             (DeeType_HasInstanceGetSetAttrStringHash(tp_invoker, tp_self, attr, hash) ? err_cant_access_attribute_string(tp_self, attr, ATTR_ACCESS_SET) : 1)
#define DeeType_SetInstanceGetSetAttrStringLen(tp_invoker, tp_self, attr, attrlen, value)           (DeeType_HasInstanceGetSetAttrStringLen(tp_invoker, tp_self, attr, attrlen) ? err_cant_access_attribute_string_len(tp_self, attr, attrlen, ATTR_ACCESS_SET) : 1)
#define DeeType_SetInstanceGetSetAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, hash, value) (DeeType_HasInstanceGetSetAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, hash) ? err_cant_access_attribute_string_len(tp_self, attr, attrlen, ATTR_ACCESS_SET) : 1)

#define DeeType_BoundInstanceMemberAttr(tp_invoker, tp_self, attr)                                  (DeeType_HasInstanceMemberAttr(tp_invoker, tp_self, attr, hash) ? 1 : -2)
#define DeeType_BoundInstanceMemberAttrHash(tp_invoker, tp_self, attr, hash)                        (DeeType_HasInstanceMemberAttrHash(tp_invoker, tp_self, attr, hash) ? 1 : -2)
#define DeeType_BoundInstanceMemberAttrString(tp_invoker, tp_self, attr)                            (DeeType_HasInstanceMemberAttrString(tp_invoker, tp_self, attr, hash) ? 1 : -2)
#define DeeType_BoundInstanceMemberAttrStringHash(tp_invoker, tp_self, attr, hash)                  (DeeType_HasInstanceMemberAttrStringHash(tp_invoker, tp_self, attr, hash) ? 1 : -2)
#define DeeType_BoundInstanceMemberAttrStringLen(tp_invoker, tp_self, attr, attrlen)                (DeeType_HasInstanceMemberAttrStringLen(tp_invoker, tp_self, attr, attrlen) ? 1 : -2)
#define DeeType_BoundInstanceMemberAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, hash)      (DeeType_HasInstanceMemberAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, hash) ? 1 : -2)
#define DeeType_DelInstanceMemberAttr(tp_invoker, tp_self, attr)                                    (DeeType_HasInstanceMemberAttr(tp_invoker, tp_self, attr, hash) ? err_cant_access_attribute(tp_self, attr, ATTR_ACCESS_DEL) : 1)
#define DeeType_DelInstanceMemberAttrHash(tp_invoker, tp_self, attr, hash)                          (DeeType_HasInstanceMemberAttrHash(tp_invoker, tp_self, attr, hash) ? err_cant_access_attribute(tp_self, attr, ATTR_ACCESS_DEL) : 1)
#define DeeType_DelInstanceMemberAttrString(tp_invoker, tp_self, attr)                              (DeeType_HasInstanceMemberAttrString(tp_invoker, tp_self, attr, hash) ? err_cant_access_attribute_string(tp_self, attr, ATTR_ACCESS_DEL) : 1)
#define DeeType_DelInstanceMemberAttrStringHash(tp_invoker, tp_self, attr, hash)                    (DeeType_HasInstanceMemberAttrStringHash(tp_invoker, tp_self, attr, hash) ? err_cant_access_attribute_string(tp_self, attr, ATTR_ACCESS_DEL) : 1)
#define DeeType_DelInstanceMemberAttrStringLen(tp_invoker, tp_self, attr, attrlen)                  (DeeType_HasInstanceMemberAttrStringLen(tp_invoker, tp_self, attr, attrlen) ? err_cant_access_attribute_string_len(tp_self, attr, attrlen, ATTR_ACCESS_DEL) : 1)
#define DeeType_DelInstanceMemberAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, hash)        (DeeType_HasInstanceMemberAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, hash) ? err_cant_access_attribute_string_len(tp_self, attr, attrlen, ATTR_ACCESS_DEL) : 1)
#define DeeType_SetInstanceMemberAttr(tp_invoker, tp_self, attr, value)                             (DeeType_HasInstanceMemberAttr(tp_invoker, tp_self, attr, hash) ? err_cant_access_attribute(tp_self, attr, ATTR_ACCESS_SET) : 1)
#define DeeType_SetInstanceMemberAttrHash(tp_invoker, tp_self, attr, hash, value)                   (DeeType_HasInstanceMemberAttrHash(tp_invoker, tp_self, attr, hash) ? err_cant_access_attribute(tp_self, attr, ATTR_ACCESS_SET) : 1)
#define DeeType_SetInstanceMemberAttrString(tp_invoker, tp_self, attr, value)                       (DeeType_HasInstanceMemberAttrString(tp_invoker, tp_self, attr, hash) ? err_cant_access_attribute_string(tp_self, attr, ATTR_ACCESS_SET) : 1)
#define DeeType_SetInstanceMemberAttrStringHash(tp_invoker, tp_self, attr, hash, value)             (DeeType_HasInstanceMemberAttrStringHash(tp_invoker, tp_self, attr, hash) ? err_cant_access_attribute_string(tp_self, attr, ATTR_ACCESS_SET) : 1)
#define DeeType_SetInstanceMemberAttrStringLen(tp_invoker, tp_self, attr, attrlen, value)           (DeeType_HasInstanceMemberAttrStringLen(tp_invoker, tp_self, attr, attrlen) ? err_cant_access_attribute_string_len(tp_self, attr, attrlen, ATTR_ACCESS_SET) : 1)
#define DeeType_SetInstanceMemberAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, hash, value) (DeeType_HasInstanceMemberAttrStringLenHash(tp_invoker, tp_self, attr, attrlen, hash) ? err_cant_access_attribute_string_len(tp_self, attr, attrlen, ATTR_ACCESS_SET) : 1)


/* Enumerate attributes. */
INTDEF WUNUSED NONNULL((1, 2, 4)) dssize_t DCALL type_method_enum(DeeTypeObject *__restrict tp_self, struct type_method const *chain, uint16_t flags, denum_t proc, void *arg);
INTDEF WUNUSED NONNULL((1, 2, 4)) dssize_t DCALL type_getset_enum(DeeTypeObject *__restrict tp_self, struct type_getset const *chain, uint16_t flags, denum_t proc, void *arg);
INTDEF WUNUSED NONNULL((1, 2, 4)) dssize_t DCALL type_member_enum(DeeTypeObject *__restrict tp_self, struct type_member const *chain, uint16_t flags, denum_t proc, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) dssize_t DCALL type_obmeth_enum(DeeTypeObject *__restrict tp_self, denum_t proc, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) dssize_t DCALL type_obprop_enum(DeeTypeObject *__restrict tp_self, denum_t proc, void *arg);
INTDEF WUNUSED NONNULL((1, 2)) dssize_t DCALL type_obmemb_enum(DeeTypeObject *__restrict tp_self, denum_t proc, void *arg);

/* Helper functions for accessing type attributes. */
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
DeeKwObjMethod_VCallFuncf(Dee_kwobjmethod_t funptr, DeeObject *__restrict thisarg,
                          char const *__restrict format, va_list args);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
DeeObjMethod_VCallFuncf(Dee_objmethod_t funptr, DeeObject *__restrict thisarg,
                        char const *__restrict format, va_list args);

#define type_method_get(desc, self)                                                                  \
	(((desc)->m_flag & TYPE_METHOD_FKWDS) ? DeeKwObjMethod_New((dkwobjmethod_t)(desc)->m_func, self) \
	                                      : DeeObjMethod_New((desc)->m_func, self))
#define type_method_doc(desc) DeeString_NewUtf8((desc)->m_doc, strlen((desc)->m_doc), STRING_ERROR_FIGNORE)
#define type_method_call(desc, self, argc, argv)                                                                            \
	(((desc)->m_flag & TYPE_METHOD_FKWDS) ? DeeKwObjMethod_CallFunc((dkwobjmethod_t)(desc)->m_func, self, argc, argv, NULL) \
	                                      : DeeObjMethod_CallFunc((desc)->m_func, self, argc, argv))
#define type_method_vcallf(desc, self, format, args)                                                                      \
	(((desc)->m_flag & TYPE_METHOD_FKWDS) ? DeeKwObjMethod_VCallFuncf((dkwobjmethod_t)(desc)->m_func, self, format, args) \
	                                      : DeeObjMethod_VCallFuncf((desc)->m_func, self, format, args))
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
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL type_obmeth_vcallf(DeeTypeObject *cls_type, struct type_method const *desc, char const *__restrict format, va_list args);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL type_getset_get(struct type_getset const *desc, DeeObject *__restrict self);
#define type_obprop_get(cls_type, desc) DeeClsProperty_New(cls_type, (desc)->gs_get, (desc)->gs_del, (desc)->gs_set)
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL type_obprop_call(DeeTypeObject *cls_type, struct type_getset const *desc, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL type_obprop_call_kw(DeeTypeObject *cls_type, struct type_getset const *desc, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL type_obprop_vcallf(DeeTypeObject *cls_type, struct type_getset const *desc, char const *__restrict format, va_list args);
#define type_obprop_doc(desc)           DeeString_NewUtf8((desc)->gs_doc, strlen((desc)->gs_doc), STRING_ERROR_FIGNORE)
#define type_obmemb_get(cls_type, desc) DeeClsMember_New(cls_type, desc)
#define type_obmemb_doc(desc)           DeeString_NewUtf8((desc)->m_doc, strlen((desc)->m_doc), STRING_ERROR_FIGNORE)
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL type_obmemb_call(DeeTypeObject *cls_type, struct type_member const *desc, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL type_obmemb_call_kw(DeeTypeObject *cls_type, struct type_member const *desc, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL type_obmemb_vcallf(DeeTypeObject *cls_type, struct type_member const *desc, char const *__restrict format, va_list args);
#define type_getset_doc(desc) DeeString_NewUtf8((desc)->gs_doc, strlen((desc)->gs_doc), STRING_ERROR_FIGNORE)
INTDEF WUNUSED NONNULL((1, 2)) int DCALL type_getset_del(struct type_getset const *desc, DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1, 2, 3)) int DCALL type_getset_set(struct type_getset const *desc, DeeObject *self, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL type_member_get(struct type_member const *desc, DeeObject *__restrict self);
INTDEF WUNUSED NONNULL((1, 2)) bool DCALL type_member_bound(struct type_member const *desc, DeeObject *__restrict self);
#define type_member_doc(desc)       DeeString_NewUtf8((desc)->m_doc, strlen((desc)->m_doc), STRING_ERROR_FIGNORE)
#define type_member_del(desc, self) type_member_set(desc, self, Dee_None)
INTDEF WUNUSED NONNULL((1, 2)) int DCALL type_member_set(struct type_member const *desc, DeeObject *self, DeeObject *value);


/* Static (class) attribute access for type objects. */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL DeeType_GetAttrStringHash)(DeeTypeObject *self, char const *__restrict attr, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL DeeType_GetAttrStringLenHash)(DeeTypeObject *self, char const *__restrict attr, size_t attrlen, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeType_BoundAttrStringHash)(DeeTypeObject *self, char const *__restrict attr, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeType_BoundAttrStringLenHash)(DeeTypeObject *self, char const *__restrict attr, size_t attrlen, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 3)) int (DCALL DeeType_FindAttr)(DeeTypeObject *self, struct attribute_info *__restrict result, struct attribute_lookup_rules const *__restrict rules);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL DeeType_CallAttrStringHash)(DeeTypeObject *self, char const *__restrict attr, dhash_t hash, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL DeeType_CallAttrStringLenHash)(DeeTypeObject *self, char const *__restrict attr, size_t attrlen, dhash_t hash, size_t argc, DeeObject *const *argv);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL DeeType_CallAttrStringHashKw)(DeeTypeObject *self, char const *__restrict attr, dhash_t hash, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL DeeType_CallAttrStringLenHashKw)(DeeTypeObject *self, char const *__restrict attr, size_t attrlen, dhash_t hash, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1, 2, 4)) DREF DeeObject *(DCALL DeeType_VCallAttrStringHashf)(DeeTypeObject *self, char const *__restrict attr, dhash_t hash, char const *__restrict format, va_list args);
INTDEF WUNUSED NONNULL((1, 2)) bool (DCALL DeeType_HasAttrStringHash)(DeeTypeObject *self, char const *__restrict attr, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) bool (DCALL DeeType_HasAttrStringLenHash)(DeeTypeObject *self, char const *__restrict attr, size_t attrlen, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeType_DelAttrStringHash)(DeeTypeObject *self, char const *__restrict attr, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeType_DelAttrStringLenHash)(DeeTypeObject *self, char const *__restrict attr, size_t attrlen, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 4)) int (DCALL DeeType_SetAttrStringHash)(DeeTypeObject *self, char const *__restrict attr, dhash_t hash, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2, 5)) int (DCALL DeeType_SetAttrStringLenHash)(DeeTypeObject *self, char const *__restrict attr, size_t attrlen, dhash_t hash, DeeObject *value);
INTDEF WUNUSED NONNULL((1, 2)) dssize_t (DCALL DeeType_EnumAttr)(DeeTypeObject *self, denum_t proc, void *arg);
#define DeeType_CallAttrStringHashTuple(self, attr, hash, args)                   DeeType_CallAttrStringHash(self, attr, hash, DeeTuple_SIZE(args), DeeTuple_ELEM(args))
#define DeeType_CallAttrStringLenHashTuple(self, attr, attrlen, hash, args)       DeeType_CallAttrStringLenHash(self, attr, attrlen, hash, DeeTuple_SIZE(args), DeeTuple_ELEM(args))
#define DeeType_CallAttrStringHashTupleKw(self, attr, hash, args, kw)             DeeType_CallAttrStringHashKw(self, attr, hash, DeeTuple_SIZE(args), DeeTuple_ELEM(args), kw)
#define DeeType_CallAttrStringLenHashTupleKw(self, attr, attrlen, hash, args, kw) DeeType_CallAttrStringLenHashKw(self, attr, attrlen, hash, DeeTuple_SIZE(args), DeeTuple_ELEM(args), kw)


/* Instance-only (wrapper) attribute access to the attributes of instance of types. */
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL DeeType_GetInstanceAttrStringHash)(DeeTypeObject *self, char const *__restrict attr, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) DREF DeeObject *(DCALL DeeType_CallInstanceAttrStringHashKw)(DeeTypeObject *self, char const *__restrict attr, dhash_t hash, size_t argc, DeeObject *const *argv, DeeObject *kw);
INTDEF WUNUSED NONNULL((1, 2)) bool (DCALL DeeType_HasInstanceAttrStringHash)(DeeTypeObject *self, char const *__restrict attr, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeType_BoundInstanceAttrStringHash)(DeeTypeObject *self, char const *__restrict attr, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2)) int (DCALL DeeType_DelInstanceAttrStringHash)(DeeTypeObject *self, char const *__restrict attr, dhash_t hash);
INTDEF WUNUSED NONNULL((1, 2, 4)) int (DCALL DeeType_SetInstanceAttrStringHash)(DeeTypeObject *self, char const *__restrict attr, dhash_t hash, DeeObject *value);
#define DeeType_CallInstanceAttrStringHashTupleKw(self, attr, hash, args, kw) \
	DeeType_CallInstanceAttrStringHashKw(self, attr, hash, DeeTuple_SIZE(args), DeeTuple_ELEM(args), kw)

#define DeeType_GetAttr(self, attr)                                      DeeType_GetAttrStringHash(self, DeeString_STR(attr), DeeString_Hash(attr))
#define DeeType_GetAttrHash(self, attr, hash)                            DeeType_GetAttrStringHash(self, DeeString_STR(attr), hash)
#define DeeType_GetAttrString(self, attr)                                DeeType_GetAttrStringHash(self, attr, Dee_HashStr(attr))
#define DeeType_GetAttrStringLen(self, attr, attrlen)                    DeeType_GetAttrStringLenHash(self, attr, attrlen, Dee_HashPtr(attr, attrlen))
#define DeeType_BoundAttr(self, attr)                                    DeeType_BoundAttrStringHash(self, DeeString_STR(attr), DeeString_Hash(attr))
#define DeeType_BoundAttrHash(self, attr, hash)                          DeeType_BoundAttrStringHash(self, DeeString_STR(attr), hash)
#define DeeType_BoundAttrString(self, attr)                              DeeType_BoundAttrStringHash(self, attr, Dee_HashStr(attr))
#define DeeType_BoundAttrStringLen(self, attr, attrlen)                  DeeType_BoundAttrStringLenHash(self, attr, attrlen, Dee_HashPtr(attr, attrlen))
#define DeeType_CallAttr(self, attr, argc, argv)                         DeeType_CallAttrStringHash(self, DeeString_STR(attr), DeeString_Hash(attr), argc, argv)
#define DeeType_CallAttrHash(self, attr, hash, argc, argv)               DeeType_CallAttrStringHash(self, DeeString_STR(attr), hash, argc, argv)
#define DeeType_CallAttrString(self, attr, argc, argv)                   DeeType_CallAttrStringHash(self, attr, Dee_HashStr(attr), argc, argv)
#define DeeType_CallAttrStringLen(self, attr, attrlen, argc, argv)       DeeType_CallAttrStringLenHash(self, attr, attrlen, Dee_HashPtr(attr, attrlen), argc, argv)
#define DeeType_CallAttrKw(self, attr, argc, argv, kw)                   DeeType_CallAttrStringHashKw(self, DeeString_STR(attr), DeeString_Hash(attr), argc, argv, kw)
#define DeeType_CallAttrHashKw(self, attr, hash, argc, argv, kw)         DeeType_CallAttrStringHashKw(self, DeeString_STR(attr), hash, argc, argv, kw)
#define DeeType_CallAttrStringKw(self, attr, argc, argv, kw)             DeeType_CallAttrStringHashKw(self, attr, Dee_HashStr(attr), argc, argv, kw)
#define DeeType_CallAttrStringLenKw(self, attr, attrlen, argc, argv, kw) DeeType_CallAttrStringLenHashKw(self, attr, attrlen, Dee_HashPtr(attr, attrlen), argc, argv, kw)
#define DeeType_CallAttrTuple(self, attr, args)                          DeeType_CallAttrStringHashTuple(self, DeeString_STR(attr), DeeString_Hash(attr), args)
#define DeeType_CallAttrHashTuple(self, attr, hash, args)                DeeType_CallAttrStringHashTuple(self, DeeString_STR(attr), hash, args)
#define DeeType_CallAttrStringTuple(self, attr, args)                    DeeType_CallAttrStringHashTuple(self, attr, Dee_HashStr(attr), args)
#define DeeType_CallAttrStringLenTuple(self, attr, attrlen, args)        DeeType_CallAttrStringLenHashTuple(self, attr, attrlen, Dee_HashPtr(attr, attrlen), args)
#define DeeType_CallAttrTupleKw(self, attr, args, kw)                    DeeType_CallAttrStringHashTupleKw(self, DeeString_STR(attr), DeeString_Hash(attr), args, kw)
#define DeeType_CallAttrHashTupleKw(self, attr, hash, args, kw)          DeeType_CallAttrStringHashTupleKw(self, DeeString_STR(attr), hash, args, kw)
#define DeeType_CallAttrStringTupleKw(self, attr, args, kw)              DeeType_CallAttrStringHashTupleKw(self, attr, Dee_HashStr(attr), args, kw)
#define DeeType_CallAttrStringLenTupleKw(self, attr, attrlen, args, kw)  DeeType_CallAttrStringLenHashTupleKw(self, attr, attrlen, Dee_HashPtr(attr, attrlen), args, kw)
#define DeeType_VCallAttrf(self, attr, format, args)                     DeeType_VCallAttrStringHashf(self, DeeString_STR(attr), DeeString_Hash(attr), format, args)
#define DeeType_VCallAttrHashf(self, attr, hash, format, args)           DeeType_VCallAttrStringHashf(self, DeeString_STR(attr), hash, format, args)
#define DeeType_VCallAttrStringf(self, attr, format, args)               DeeType_VCallAttrStringHashf(self, attr, Dee_HashStr(attr), format, args)
#define DeeType_HasAttr(self, attr)                                      DeeType_HasAttrStringHash(self, DeeString_STR(attr), DeeString_Hash(attr))
#define DeeType_HasAttrHash(self, attr, hash)                            DeeType_HasAttrStringHash(self, DeeString_STR(attr), hash)
#define DeeType_HasAttrString(self, attr)                                DeeType_HasAttrStringHash(self, attr, Dee_HashStr(attr))
#define DeeType_HasAttrStringLen(self, attr, attrlen)                    DeeType_HasAttrStringLenHash(self, attr, attrlen, Dee_HashPtr(attr, attrlen))
#define DeeType_DelAttr(self, attr)                                      DeeType_DelAttrStringHash(self, DeeString_STR(attr), DeeString_Hash(attr))
#define DeeType_DelAttrHash(self, attr, hash)                            DeeType_DelAttrStringHash(self, DeeString_STR(attr), hash)
#define DeeType_DelAttrString(self, attr)                                DeeType_DelAttrStringHash(self, attr, Dee_HashStr(attr))
#define DeeType_DelAttrStringLen(self, attr, attrlen)                    DeeType_DelAttrStringLenHash(self, attr, attrlen, Dee_HashPtr(attr, attrlen))
#define DeeType_SetAttr(self, attr, value)                               DeeType_SetAttrStringHash(self, DeeString_STR(attr), DeeString_Hash(attr), value)
#define DeeType_SetAttrHash(self, attr, hash, value)                     DeeType_SetAttrStringHash(self, DeeString_STR(attr), hash, value)
#define DeeType_SetAttrString(self, attr, value)                         DeeType_SetAttrStringHash(self, attr, Dee_HashStr(attr), value)
#define DeeType_SetAttrStringLen(self, attr, attrlen, value)             DeeType_SetAttrStringLenHash(self, attr, attrlen, Dee_HashPtr(attr, attrlen), value)

#define DeeType_GetInstanceAttr(self, attr)                              DeeType_GetInstanceAttrStringHash(self, DeeString_STR(attr), DeeString_Hash(attr))
#define DeeType_GetInstanceAttrHash(self, attr, hash)                    DeeType_GetInstanceAttrStringHash(self, DeeString_STR(attr), hash)
#define DeeType_GetInstanceAttrString(self, attr)                        DeeType_GetInstanceAttrStringHash(self, attr, Dee_HashStr(attr))
#define DeeType_CallInstanceAttrKw(self, attr, argc, argv, kw)           DeeType_CallInstanceAttrStringHashKw(self, DeeString_STR(attr), DeeString_Hash(attr), argc, argv, kw)
#define DeeType_CallInstanceAttrHashKw(self, attr, hash, argc, argv, kw) DeeType_CallInstanceAttrStringHashKw(self, DeeString_STR(attr), hash, argc, argv, kw)
#define DeeType_CallInstanceAttrStringKw(self, attr, argc, argv, kw)     DeeType_CallInstanceAttrStringHashKw(self, attr, Dee_HashStr(attr), argc, argv, kw)
#define DeeType_CallInstanceAttrTupleKw(self, attr, args, kw)            DeeType_CallInstanceAttrStringHashTupleKw(self, DeeString_STR(attr), DeeString_Hash(attr), args, kw)
#define DeeType_CallInstanceAttrHashTupleKw(self, hash, attr, args, kw)  DeeType_CallInstanceAttrStringHashTupleKw(self, DeeString_STR(attr), hash, args, kw)
#define DeeType_CallInstanceAttrStringTupleKw(self, attr, args, kw)      DeeType_CallInstanceAttrStringHashTupleKw(self, attr, Dee_HashStr(attr), args, kw)
#define DeeType_HasInstanceAttr(self, attr)                              DeeType_HasInstanceAttrStringHash(self, DeeString_STR(attr), DeeString_Hash(attr))
#define DeeType_HasInstanceAttrHash(self, attr, hash)                    DeeType_HasInstanceAttrStringHash(self, DeeString_STR(attr), hash)
#define DeeType_HasInstanceAttrString(self, attr)                        DeeType_HasInstanceAttrStringHash(self, attr, Dee_HashStr(attr))
#define DeeType_BoundInstanceAttr(self, attr)                            DeeType_BoundInstanceAttrStringHash(self, DeeString_STR(attr), DeeString_Hash(attr))
#define DeeType_BoundInstanceAttrHash(self, attr, hash)                  DeeType_BoundInstanceAttrStringHash(self, DeeString_STR(attr), hash)
#define DeeType_BoundInstanceAttrString(self, attr)                      DeeType_BoundInstanceAttrStringHash(self, attr, Dee_HashStr(attr))
#define DeeType_DelInstanceAttr(self, attr)                              DeeType_DelInstanceAttrStringHash(self, DeeString_STR(attr), DeeString_Hash(attr))
#define DeeType_DelInstanceAttrHash(self, attr, hash)                    DeeType_DelInstanceAttrStringHash(self, DeeString_STR(attr), hash)
#define DeeType_DelInstanceAttrString(self, attr)                        DeeType_DelInstanceAttrStringHash(self, attr, Dee_HashStr(attr))
#define DeeType_SetInstanceAttr(self, attr, value)                       DeeType_SetInstanceAttrStringHash(self, DeeString_STR(attr), DeeString_Hash(attr), value)
#define DeeType_SetInstanceAttrHash(self, attr, hash, value)             DeeType_SetInstanceAttrStringHash(self, DeeString_STR(attr), hash, value)
#define DeeType_SetInstanceAttrString(self, attr, value)                 DeeType_SetInstanceAttrStringHash(self, attr, Dee_HashStr(attr), value)


#ifndef __INTELLISENSE__
#ifndef __NO_builtin_expect
#define DeeType_DelAttrStringHash(self, attr, hash)                            __builtin_expect(DeeType_DelAttrStringHash(self, attr, hash), 0)
#define DeeType_DelAttrStringLenHash(self, attr, attrlen, hash)                __builtin_expect(DeeType_DelAttrStringLenHash(self, attr, attrlen, hash), 0)
#define DeeType_SetAttrStringHash(self, attr, hash, value)                     __builtin_expect(DeeType_SetAttrStringHash(self, attr, hash, value), 0)
#define DeeType_SetAttrStringLenHash(self, attr, attrlen, hash, value)         __builtin_expect(DeeType_SetAttrStringLenHash(self, attr, attrlen, hash, value), 0)
#define DeeType_DelInstanceAttrStringHash(self, attr, hash)                    __builtin_expect(DeeType_DelInstanceAttrStringHash(self, attr, hash), 0)
#define DeeType_DelInstanceAttrStringLenHash(self, attr, attrlen, hash)        __builtin_expect(DeeType_DelInstanceAttrStringLenHash(self, attr, attrlen, hash), 0)
#define DeeType_SetInstanceAttrStringHash(self, attr, hash, value)             __builtin_expect(DeeType_SetInstanceAttrStringHash(self, attr, hash, value), 0)
#define DeeType_SetInstanceAttrStringLenHash(self, attr, attrlen, hash, value) __builtin_expect(DeeType_SetInstanceAttrStringLenHash(self, attr, attrlen, hash, value), 0)
#endif /* !__NO_builtin_expect */
#endif /* !__INTELLISENSE__ */

#endif /* CONFIG_BUILDING_DEEMON */

DECL_END

#endif /* !GUARD_DEEMON_MRO_H */
