/* Copyright (c) 2018-2020 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2020 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_RUNTIME_MRO_C
#define GUARD_DEEMON_RUNTIME_MRO_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/attribute.h>
#include <deemon/class.h>
#include <deemon/error.h>
#include <deemon/file.h>
#include <deemon/instancemethod.h>
#include <deemon/mro.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/objmethod.h>
#include <deemon/string.h>
#include <deemon/tuple.h>

#include "runtime_error.h"
#include "strings.h"

DECL_BEGIN

#ifndef CONFIG_NO_THREADS
PRIVATE DEFINE_RWLOCK(membercache_list_lock);
#endif /* !CONFIG_NO_THREADS */

/* [0..1][lock(membercache_list_lock)]
 *  Linked list of all existing type-member caches. */
PRIVATE struct membercache *membercache_list;

#define streq(a, b) (strcmp(a, b) == 0)
LOCAL NONNULL((1, 2)) bool DCALL
streq_len(char const *zero_zterminated,
          char const *comparand, size_t comparand_length) {
	if (strlen(zero_zterminated) != comparand_length)
		return false;
	return memcmp(zero_zterminated, comparand, comparand_length) == 0;
}



INTERN NONNULL((1)) void DCALL
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
#endif /* !CONFIG_NO_THREADS */
	rwlock_write(&membercache_list_lock);
	ASSERT(!self->mc_table);
	/* Check check `mc_pself != NULL' again because another thread
	 * may have cleared the tables of all member caches while collecting
	 * memory, in the process unlinking all of them from the global chain. */
	if (self->mc_pself) {
		if ((*self->mc_pself = self->mc_next) != NULL)
			self->mc_next->mc_pself = self->mc_pself;
	}
	rwlock_endwrite(&membercache_list_lock);
}

INTERN size_t DCALL
membercache_clear(size_t max_clear) {
	size_t result = 0;
	struct membercache *entry;
	rwlock_write(&membercache_list_lock);
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
		memset(&entry->mc_next, 0xcc, sizeof(void *));
#endif /* !NDEBUG */
		MEMBERCACHE_ENDWRITE(entry);
		if (result >= max_clear)
			break;
	}
	rwlock_endwrite(&membercache_list_lock);
	return result;
}

STATIC_ASSERT(MEMBERCACHE_UNUSED == 0);
PRIVATE ATTR_NOINLINE NONNULL((1)) bool DCALL
membercache_rehash(struct membercache *__restrict self) {
	struct membercache_slot *new_vector, *iter, *end;
	size_t new_mask = self->mc_mask;
	new_mask        = (new_mask << 1) | 1;
	if unlikely(new_mask == 1)
		new_mask = 16 - 1; /* Start out bigger than 2. */
	ASSERT(self->mc_size < new_mask);
	new_vector = (struct membercache_slot *)Dee_TryCalloc((new_mask + 1) *
	                                                      sizeof(struct membercache_slot));
	if unlikely(!new_vector)
		return false;
	ASSERT((self->mc_table == NULL) == (self->mc_size == 0));
	ASSERT((self->mc_table == NULL) == (self->mc_mask == 0));
	if (self->mc_table == NULL) {
		/* This is the first time that this cache is being rehashed.
		 * >> Register it in the global chain of member caches,
		 *    so we can clear it when memory gets low. */
		rwlock_write(&membercache_list_lock);
		if ((self->mc_next = membercache_list) != NULL)
			membercache_list->mc_pself = &self->mc_next;
		self->mc_pself   = &membercache_list;
		membercache_list = self;
		rwlock_endwrite(&membercache_list_lock);
	} else {
		/* Re-insert all existing items into the new table vector. */
		end = (iter = self->mc_table) + (self->mc_mask + 1);
		for (; iter != end; ++iter) {
			struct membercache_slot *item;
			dhash_t i, perturb;
			/* Skip unused entires. */
			if (iter->mcs_type == MEMBERCACHE_UNUSED)
				continue;
			perturb = i = iter->mcs_hash & new_mask;
			for (;; i = MEMBERCACHE_HASHNX(i, perturb), MEMBERCACHE_HASHPT(perturb)) {
				item = &new_vector[i & new_mask];
				if (item->mcs_type == MEMBERCACHE_UNUSED)
					break; /* Empty slot found. */
			}
			/* Transfer this object. */
			memcpy(item, iter, sizeof(struct membercache_slot));
		}
		Dee_Free(self->mc_table);
	}
	self->mc_mask  = new_mask;
	self->mc_table = new_vector;
	return true;
}



/* Since this cache is totally optional, don't slow down when we can't get the lock. */
#ifdef CONFIG_NO_THREADS
#define MEMBERCACHE_TRYWRITE_OR_RETURN(self) \
	(void)0
#elif 1
#define MEMBERCACHE_TRYWRITE_OR_RETURN(self)    \
	do {                                        \
		if (!rwlock_trywrite(&(self)->mc_lock)) \
			return;                             \
	} __WHILE0
#else
#define MEMBERCACHE_TRYWRITE_OR_RETURN(self) \
	MEMBERCACHE_WRITE(self)
#endif


#define MEMBERCACHE_ADDENTRY(name, init)                                                      \
	{                                                                                         \
		dhash_t i, perturb;                                                                   \
		struct membercache_slot *item;                                                        \
		MEMBERCACHE_TRYWRITE_OR_RETURN(self);                                                 \
	again:                                                                                    \
		if (!self->mc_table)                                                                  \
			goto rehash_initial;                                                              \
		/* Re-check that the named attribute isn't already in-cache. */                       \
		perturb = i = MEMBERCACHE_HASHST(self, hash);                                         \
		for (;; i = MEMBERCACHE_HASHNX(i, perturb), MEMBERCACHE_HASHPT(perturb)) {            \
			item = MEMBERCACHE_HASHIT(self, i);                                               \
			if (item->mcs_type == MEMBERCACHE_UNUSED)                                         \
				break;                                                                        \
			if (item->mcs_hash != hash)                                                       \
				continue;                                                                     \
			if (item->mcs_method.m_name != (name) &&                                          \
			    !streq(item->mcs_name, (name)))                                               \
				continue;                                                                     \
			/* Already in cache! */                                                           \
			MEMBERCACHE_ENDWRITE(self);                                                       \
			return;                                                                           \
		}                                                                                     \
		if (self->mc_size + 1 >= self->mc_mask) {                                             \
		rehash_initial:                                                                       \
			if (membercache_rehash(self))                                                     \
				goto again;                                                                   \
			goto done; /* Well... We couldn't rehash the cache so we can't add this entry. */ \
		}                                                                                     \
		/* Not found. - Use this empty slot. */                                               \
		item->mcs_hash = hash;                                                                \
		item->mcs_decl = decl;                                                                \
		init;                                                                                 \
		++self->mc_size;                                                                      \
		/* Try to keep the table vector big at least twice as big as the element count. */    \
		if (self->mc_size * 2 > self->mc_mask)                                                \
			membercache_rehash(self);                                                         \
	done:                                                                                     \
		MEMBERCACHE_ENDWRITE(self);                                                           \
	}


#define PRIVATE_IS_KNOWN_TYPETYPE(x) \
	((x) == &DeeType_Type || (x) == &DeeFileType_Type)
#define MEMBERCACHE_GETTYPENAME(x)                                                                 \
	(PRIVATE_IS_KNOWN_TYPETYPE(COMPILER_CONTAINER_OF(x, DeeTypeObject, tp_cache)->ob_type)         \
	 ? COMPILER_CONTAINER_OF(x, DeeTypeObject, tp_cache)->tp_name                                  \
	 : PRIVATE_IS_KNOWN_TYPETYPE(COMPILER_CONTAINER_OF(x, DeeTypeObject, tp_class_cache)->ob_type) \
	   ? COMPILER_CONTAINER_OF(x, DeeTypeObject, tp_class_cache)->tp_name                          \
	   : "?") \


INTERN NONNULL((1, 2, 4)) void DCALL
membercache_addmethod(struct membercache *self,
                      DeeTypeObject *decl, dhash_t hash,
                      struct type_method const *method) {
	DEE_DPRINTF("[RT] Caching method `%s.%s' in `%s'\n",
	            decl->tp_name, method->m_name,
	            MEMBERCACHE_GETTYPENAME(self));
	MEMBERCACHE_ADDENTRY(method->m_name, {
		item->mcs_type   = MEMBERCACHE_METHOD;
		item->mcs_method = *method;
	});
}

INTERN NONNULL((1, 2, 4)) void DCALL
membercache_addinstancemethod(struct membercache *self,
                              DeeTypeObject *decl, dhash_t hash,
                              struct type_method const *method) {
	DEE_DPRINTF("[RT] Caching instance_method `%s.%s' in `%s'\n",
	            decl->tp_name, method->m_name,
	            MEMBERCACHE_GETTYPENAME(self));
	ASSERT(self != &decl->tp_cache);
	MEMBERCACHE_ADDENTRY(method->m_name, {
		item->mcs_type   = MEMBERCACHE_INSTANCE_METHOD;
		item->mcs_method = *method;
	});
}

INTERN NONNULL((1, 2, 4)) void DCALL
membercache_addgetset(struct membercache *self,
                      DeeTypeObject *decl, dhash_t hash,
                      struct type_getset const *getset) {
	DEE_DPRINTF("[RT] Caching getset `%s.%s' in `%s'\n",
	            decl->tp_name, getset->gs_name,
	            MEMBERCACHE_GETTYPENAME(self));
	MEMBERCACHE_ADDENTRY(getset->gs_name, {
		item->mcs_type   = MEMBERCACHE_GETSET;
		item->mcs_getset = *getset;
	});
}

INTERN NONNULL((1, 2, 4)) void DCALL
membercache_addinstancegetset(struct membercache *self,
                              DeeTypeObject *decl, dhash_t hash,
                              struct type_getset const *getset) {
	DEE_DPRINTF("[RT] Caching instance_getset `%s.%s' in `%s'\n",
	            decl->tp_name, getset->gs_name,
	            MEMBERCACHE_GETTYPENAME(self));
	ASSERT(self != &decl->tp_cache);
	MEMBERCACHE_ADDENTRY(getset->gs_name, {
		item->mcs_type   = MEMBERCACHE_INSTANCE_GETSET;
		item->mcs_getset = *getset;
	});
}

INTERN NONNULL((1, 2, 4)) void DCALL
membercache_addmember(struct membercache *self,
                      DeeTypeObject *decl, dhash_t hash,
                      struct type_member const *member) {
	DEE_DPRINTF("[RT] Caching member `%s.%s' in `%s'\n",
	            decl->tp_name, member->m_name,
	            MEMBERCACHE_GETTYPENAME(self));
	MEMBERCACHE_ADDENTRY(member->m_name, {
		item->mcs_type   = MEMBERCACHE_MEMBER;
		item->mcs_member = *member;
	});
}

INTERN NONNULL((1, 2, 4)) void DCALL
membercache_addinstancemember(struct membercache *self,
                              DeeTypeObject *decl, dhash_t hash,
                              struct type_member const *member) {
	DEE_DPRINTF("[RT] Caching instance_member `%s.%s' in `%s'\n",
	            decl->tp_name, member->m_name,
	            MEMBERCACHE_GETTYPENAME(self));
	ASSERT(self != &decl->tp_cache);
	MEMBERCACHE_ADDENTRY(member->m_name, {
		item->mcs_type   = MEMBERCACHE_INSTANCE_MEMBER;
		item->mcs_member = *member;
	});
}

INTERN NONNULL((1, 2, 4)) void DCALL
membercache_addattrib(struct membercache *self,
                      DeeTypeObject *decl, dhash_t hash,
                      struct class_attribute *attrib) {
	char const *name = DeeString_STR(attrib->ca_name);
	DEE_DPRINTF("[RT] Caching attribute `%s.%s' in `%s'\n",
	            decl->tp_name, name,
	            MEMBERCACHE_GETTYPENAME(self));
	MEMBERCACHE_ADDENTRY(name, {
		item->mcs_type          = MEMBERCACHE_ATTRIB;
		item->mcs_attrib.a_name = name;
		item->mcs_attrib.a_attr = attrib;
		item->mcs_attrib.a_desc = DeeClass_DESC(decl);
	});
}

INTERN NONNULL((1, 2, 4)) void DCALL
membercache_addinstanceattrib(struct membercache *self,
                              DeeTypeObject *decl, dhash_t hash,
                              struct class_attribute *attrib) {
	char const *name = DeeString_STR(attrib->ca_name);
	DEE_DPRINTF("[RT] Caching instance_attribute `%s.%s' in `%s'\n",
	            decl->tp_name, name,
	            MEMBERCACHE_GETTYPENAME(self));
	ASSERT(self != &decl->tp_cache);
	MEMBERCACHE_ADDENTRY(name, {
		item->mcs_type          = MEMBERCACHE_INSTANCE_ATTRIB;
		item->mcs_attrib.a_name = name;
		item->mcs_attrib.a_attr = attrib;
		item->mcs_attrib.a_desc = DeeClass_DESC(decl);
	});
}

#ifndef __INTELLISENSE__
#define MRO_LEN 1
#include "mro-impl.c.inl"
#include "mro-impl.c.inl"
#endif /* !__INTELLISENSE__ */

DECL_END

#endif /* !GUARD_DEEMON_RUNTIME_MRO_C */
