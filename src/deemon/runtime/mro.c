/* Copyright (c) 2018-2025 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2025 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_RUNTIME_MRO_C
#define GUARD_DEEMON_RUNTIME_MRO_C 1
#define DEE_SOURCE

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/attribute.h>
#include <deemon/class.h>
#include <deemon/error.h>
#include <deemon/file.h>
#include <deemon/format.h>
#include <deemon/instancemethod.h>
#include <deemon/kwds.h>
#include <deemon/mro.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/objmethod.h>
#include <deemon/string.h>
#include <deemon/system-features.h> /* bcmpc(), ... */
#include <deemon/tuple.h>
#include <deemon/util/atomic.h>
#include <deemon/util/lock.h>

#include <hybrid/sched/yield.h>
#include <hybrid/sequence/list.h>
#include <hybrid/typecore.h>
/**/

#include "kwlist.h"
#include "runtime_error.h"
#include "strings.h"
/**/

#include <stddef.h>
#include <stdarg.h>
#include <stdint.h>

#ifdef CONFIG_HAVE_VA_LIST_IS_NOT_ARRAY
#define VALIST_ADDR(x) (&(x))
#else /* CONFIG_HAVE_VA_LIST_IS_NOT_ARRAY */
#define VALIST_ADDR(x) (&(x)[0])
#endif /* !CONFIG_HAVE_VA_LIST_IS_NOT_ARRAY */

DECL_BEGIN

#undef byte_t
#define byte_t __BYTE_TYPE__

#ifndef CONFIG_HAVE_strcmp
#define CONFIG_HAVE_strcmp
#undef strcmp
#define strcmp dee_strcmp
DeeSystem_DEFINE_strcmp(dee_strcmp)
#endif /* !CONFIG_HAVE_strcmp */

#ifndef CONFIG_HAVE_strcmpz
#define CONFIG_HAVE_strcmpz
#undef strcmpz
#define strcmpz dee_strcmpz
DeeSystem_DEFINE_strcmpz(dee_strcmpz)
#endif /* !CONFIG_HAVE_strcmpz */

#ifndef NDEBUG
#define DBG_memset (void)memset
#else /* !NDEBUG */
#define DBG_memset(dst, byte, n_bytes) (void)0
#endif /* NDEBUG */

#ifndef CONFIG_NO_THREADS
PRIVATE Dee_atomic_lock_t membercache_list_lock = DEE_ATOMIC_LOCK_INIT;
#endif /* !CONFIG_NO_THREADS */

#define membercache_list_lock_available()  Dee_atomic_lock_available(&membercache_list_lock)
#define membercache_list_lock_acquired()   Dee_atomic_lock_acquired(&membercache_list_lock)
#define membercache_list_lock_tryacquire() Dee_atomic_lock_tryacquire(&membercache_list_lock)
#define membercache_list_lock_acquire()    Dee_atomic_lock_acquire(&membercache_list_lock)
#define membercache_list_lock_waitfor()    Dee_atomic_lock_waitfor(&membercache_list_lock)
#define membercache_list_lock_release()    Dee_atomic_lock_release(&membercache_list_lock)

LIST_HEAD(membercache_list_struct, Dee_membercache);

/* [0..1][lock(membercache_list_lock)]
 * Linked list of all existing type-member caches. */
PRIVATE struct membercache_list_struct
membercache_list = LIST_HEAD_INITIALIZER(membercache_list);

#define streq(a, b)            (strcmp(a, b) == 0)
#define streq_len(a, b, b_len) (strcmpz(a, b, b_len) == 0)

/* Finalize a given member-cache. */
INTERN NONNULL((1)) void DCALL
Dee_membercache_fini(struct Dee_membercache *__restrict self) {
	if (LIST_ISBOUND(self, mc_link)) {
		membercache_list_lock_acquire();
		if (LIST_ISBOUND(self, mc_link))
			LIST_REMOVE(self, mc_link);
		membercache_list_lock_release();
	}
	Dee_Free(self->mc_table);
	DBG_memset(self, 0xcc, sizeof(*self));
}

INTERN size_t DCALL
Dee_membercache_clearall(size_t max_clear) {
	size_t result = 0;
	struct Dee_membercache *cache;
again:
	membercache_list_lock_acquire();
	if (LIST_EMPTY(&membercache_list)) {
		membercache_list_lock_release();
	} else {
		DREF struct Dee_membercache_table *table;
		cache = LIST_FIRST(&membercache_list);

		/* Steal the table and wait for everyone to stop using it. */
		table = atomic_xch(&cache->mc_table, NULL);
		Dee_membercache_waitfor(cache);

		/* Pop this entry from the global chain. */
		LIST_UNBIND(cache, mc_link);
		membercache_list_lock_release();

		if (table) {
			/* Track how much member this operation will be freeing up. */
			result += (table->mc_mask + 1) * sizeof(struct Dee_membercache);

			/* Drop the table reference that was held by the type. */
			Dee_membercache_table_decref(table);
		}

		if (result < max_clear)
			goto again;
	}
	return result;
}


/* Try to allocate a new member-cache table. */
#define Dee_membercache_table_trycalloc(mask)                                                         \
	(struct Dee_membercache_table *)Dee_TryCallococ(offsetof(struct Dee_membercache_table, mc_table), \
	                                                (mask) + 1, sizeof(struct Dee_membercache_slot))
STATIC_ASSERT(MEMBERCACHE_UNUSED == 0);

PRIVATE NONNULL((1, 2)) void DCALL
Dee_membercache_table_do_addslot(struct Dee_membercache_table *__restrict self,
                                 struct Dee_membercache_slot const *__restrict item) {
	dhash_t i, perturb;
	struct Dee_membercache_slot *slot;
	perturb = i = Dee_membercache_table_hashst(self, item->mcs_hash);
	for (;; Dee_membercache_table_hashnx(i, perturb)) {
		slot = Dee_membercache_table_hashit(self, i);
		if (slot->mcs_type == MEMBERCACHE_UNUSED)
			break;
		ASSERTF(slot->mcs_type != MEMBERCACHE_UNINITIALIZED,
		        "We're building a new cache-table, so how can "
		        "that new table contain UNINITIALIZED items?");
	}

	/* Found a free slot -> write to it! */
	memcpy(slot, item, sizeof(struct Dee_membercache_slot));
	++self->mc_size;
}

/* Try to construct a new member-cache table */
PRIVATE WUNUSED NONNULL((2)) DREF struct Dee_membercache_table *DCALL
Dee_membercache_table_new(struct Dee_membercache_table const *old_table,
                          struct Dee_membercache_slot const *__restrict item) {
	DREF struct Dee_membercache_table *result;
	size_t new_mask = 0;
	if (old_table != NULL)
		new_mask = old_table->mc_mask;
	new_mask = (new_mask << 1) | 1;
	if unlikely(new_mask == 1)
		new_mask = 16 - 1; /* Start out bigger than 2. */
	result = Dee_membercache_table_trycalloc(new_mask);
	if unlikely(!result)
		return NULL;

	/* Fill in the basic characteristics of the new cache-table. */
	result->mc_mask   = new_mask;
	result->mc_refcnt = 1;
	ASSERTF(result->mc_size == 0, "Should be the case because of calloc()");

	/* Migrate the old cache-table into the new one. */
	if (old_table != NULL) {
		size_t i;
		for (i = 0; i <= old_table->mc_mask; ++i) {
			uint16_t type;
			struct Dee_membercache_slot const *slot;
			slot = &old_table->mc_table[i];
			type = atomic_read(&slot->mcs_type);
			if (type == MEMBERCACHE_UNUSED)
				continue;
			if (type == MEMBERCACHE_UNINITIALIZED)
				continue; /* Don't migrate slots that aren't fully initialized. */

			/* Keep this cache-slot as part of the resulting cache-table. */
			Dee_membercache_table_do_addslot(result, slot);
		}
	}

	/* Add the caller's new item. */
	Dee_membercache_table_do_addslot(result, item);
	return result;
}


/* Try to add a slot to the given member-cache table.
 * @return:  2: Slot is being initialized by a different thread (not added)
 * @return:  1: Slot already exists in cache (not added)
 * @return:  0: Success
 * @return: -1: No more free slots (caller must allocate a new member-cache table) */
PRIVATE NONNULL((1, 2)) int DCALL
Dee_membercache_table_addslot(struct Dee_membercache_table *__restrict self,
                              struct Dee_membercache_slot const *__restrict item,
                              bool allow_bad_hash_ratios) {
	dhash_t i, perturb;
	struct Dee_membercache_slot *slot;

	/* Try to allocate a slot within the table. */
	for (;;) {
		size_t size = atomic_read(&self->mc_size);
		if (allow_bad_hash_ratios ? (size * 1) >= self->mc_mask
		                          : (size * 2) >= self->mc_mask)
			return -1; /* You should (or need to) allocate a new table. */
		if (atomic_cmpxch_or_write(&self->mc_size, size, size + 1))
			break;
	}

	/* Re-check that the named attribute isn't already in-cache. */
again_search_slots:
	perturb = i = Dee_membercache_table_hashst(self, item->mcs_hash);
	for (;; Dee_membercache_table_hashnx(i, perturb)) {
		uint16_t type;
		slot = Dee_membercache_table_hashit(self, i);
		type = atomic_read(&slot->mcs_type);
		if (type == MEMBERCACHE_UNUSED)
			break;

#ifndef CONFIG_NO_THREADS
		if (type == MEMBERCACHE_UNINITIALIZED) {
			/* Encountered an uninitialized slot along the cache-chain.
			 *
			 * This means that some other thread is currently writing an
			 * item into the cache-table that has a similar hash to the
			 * item that we're writing right now.
			 *
			 * This means that there is a chance that the other thread
			 * is caching the same item as we are, and we have to make
			 * such that no member is cached twice within the same table,
			 * so to prevent any chance of that happening, we have to
			 * assume the worst, and believe that the other thread _is_
			 * trying to cache the same item as we are.
			 *
			 * As such, the only course of action we have left is to tell
			 * our caller that their new element is already cached, even
			 * if that *may* not actually be the case. */
			atomic_dec(&self->mc_size);
			return 2;
		}
#endif /* !CONFIG_NO_THREADS */

		if (slot->mcs_hash != item->mcs_hash)
			continue;
		if (slot->mcs_method.m_name != item->mcs_name &&
		    !streq(slot->mcs_name, item->mcs_name))
			continue;

		/* Already in cache!
		 * -> Free the slot we allocated above and indicate success to our caller. */
		atomic_dec(&self->mc_size);
		return 1;
	}

	/* Not found. - Try to allocate this empty slot.
	 * If it's no longer empty, start over */
	if (!atomic_cmpxch_or_write(&slot->mcs_type,
	                            MEMBERCACHE_UNUSED,
	                            MEMBERCACHE_UNINITIALIZED))
		goto again_search_slots;

	/* At this point, we've successfully allocated a slot within the cache-table.
	 * With that in mind, we must now fill in that slot, but we have to be careful
	 * about the order here, in that we _MUST_ fill in the type-field LAST!
	 *
	 * This is because by setting the type-field to something other than
	 * `MEMBERCACHE_UNINITIALIZED', we essentially commit the new slot
	 * into the cache. */
	memcpy((byte_t *)slot + COMPILER_OFFSETAFTER(struct Dee_membercache_slot, mcs_type),
	       (byte_t *)item + COMPILER_OFFSETAFTER(struct Dee_membercache_slot, mcs_type),
	       sizeof(struct Dee_membercache_slot) -
	       COMPILER_OFFSETAFTER(struct Dee_membercache_slot, mcs_type));
	COMPILER_WRITE_BARRIER(); /* The type-field must be written last! */
	atomic_write(&slot->mcs_type, item->mcs_type);
	return 0;
}


#ifndef Dee_DPRINT_IS_NOOP
PRIVATE char const membercache_type_names[][16] = {
	/* [MEMBERCACHE_UNUSED         ] = */ "??UNUSED",
	/* [MEMBERCACHE_UNINITIALIZED  ] = */ "??UNINITIALIZED",
	/* [MEMBERCACHE_METHOD         ] = */ "method",
	/* [MEMBERCACHE_GETSET         ] = */ "getset",
	/* [MEMBERCACHE_MEMBER         ] = */ "member",
	/* [MEMBERCACHE_ATTRIB         ] = */ "attrib",
	/* [MEMBERCACHE_INSTANCE_METHOD] = */ "instance_method",
	/* [MEMBERCACHE_INSTANCE_GETSET] = */ "instance_getset",
	/* [MEMBERCACHE_INSTANCE_MEMBER] = */ "instance_member",
	/* [MEMBERCACHE_INSTANCE_ATTRIB] = */ "instance_attrib",
};

#define PRIVATE_IS_KNOWN_TYPETYPE(x) \
	((x) == &DeeType_Type || (x) == &DeeFileType_Type)
#define MEMBERCACHE_GETTYPENAME(x)                                                                 \
	(PRIVATE_IS_KNOWN_TYPETYPE(COMPILER_CONTAINER_OF(x, DeeTypeObject, tp_cache)->ob_type)         \
	 ? COMPILER_CONTAINER_OF(x, DeeTypeObject, tp_cache)->tp_name                                  \
	 : PRIVATE_IS_KNOWN_TYPETYPE(COMPILER_CONTAINER_OF(x, DeeTypeObject, tp_class_cache)->ob_type) \
	   ? COMPILER_CONTAINER_OF(x, DeeTypeObject, tp_class_cache)->tp_name                          \
	   : "?")
#define MEMBERCACHE_GETCLASSNAME(x)                                                                \
	(PRIVATE_IS_KNOWN_TYPETYPE(COMPILER_CONTAINER_OF(x, DeeTypeObject, tp_cache)->ob_type)         \
	 ? "tp_cache"                                                                                  \
	 : PRIVATE_IS_KNOWN_TYPETYPE(COMPILER_CONTAINER_OF(x, DeeTypeObject, tp_class_cache)->ob_type) \
	   ? "tp_class_cache"                                                                          \
	   : "?")


PRIVATE NONNULL((1, 2)) void DCALL
Dee_membercache_addslot_log_success(struct Dee_membercache *__restrict self,
                                    struct Dee_membercache_slot const *__restrict slot) {
	Dee_DPRINTF("[RT] Cached %s `%s.%s' in `%s' (%s)\n",
	            membercache_type_names[slot->mcs_type],
	            slot->mcs_decl->tp_name, slot->mcs_attrib.a_name,
	            MEMBERCACHE_GETTYPENAME(self),
	            MEMBERCACHE_GETCLASSNAME(self));
}
#else /* !Dee_DPRINT_IS_NOOP */
#define Dee_membercache_addslot_log_success(self, slot) (void)0
#endif /* Dee_DPRINT_IS_NOOP */

PRIVATE NONNULL((1, 2)) int DCALL
Dee_membercache_addslot(struct Dee_membercache *__restrict self,
                        struct Dee_membercache_slot const *__restrict slot) {
	DREF struct Dee_membercache_table *old_table;
	DREF struct Dee_membercache_table *new_table;

	/* Get a reference to the current table. */
	Dee_membercache_tabuse_inc(self);
	old_table = atomic_read(&self->mc_table);
	if (old_table != NULL) {
		int status;
		Dee_membercache_table_incref(old_table);
		Dee_membercache_tabuse_dec(self);

		/* Try to add the slot to an existing cache-table. */
do_operate_with_old_table:
		status = Dee_membercache_table_addslot(old_table, slot, false);
		if (status >= 0) {
			Dee_membercache_table_decref(old_table);
			return status;
		}
	} else {
		Dee_membercache_tabuse_dec(self);
	}

	/* Either there isn't a table, or we're unable to add more items
	 * to the table. In either case, try to construct a new table! */
	new_table = Dee_membercache_table_new(old_table, slot);
	if (!new_table) {
		/* Failed to create a new table -> try again to add to the
		 * existing table, but ignore bad hash characteristics this
		 * time around. */
		int result = -1;
		if (old_table != NULL) {
			/* It doesn't matter if this addslot() call succeeds or not... */
			result = Dee_membercache_table_addslot(old_table, slot, true);
			Dee_membercache_table_decref(old_table);
#ifndef Dee_DPRINT_IS_NOOP
			if (result == 0)
				Dee_membercache_addslot_log_success(self, slot);
#endif /* !Dee_DPRINT_IS_NOOP */
		}
		return result;
	}

	/* Drop our original reference to the old table. */
	if (old_table != NULL)
		Dee_membercache_table_decref(old_table);

	/* Got a new table -> now to store it within the type.
	 * But: do one more check if the currently set table
	 *      might actually be better than our `new_table' */
	if (atomic_read(&self->mc_table) != old_table) {
		Dee_membercache_tabuse_inc(self);
		old_table = atomic_read(&self->mc_table);
		if (old_table == NULL) {
			Dee_membercache_tabuse_dec(self);
		} else {
			Dee_membercache_table_incref(old_table);
			Dee_membercache_tabuse_dec(self);

			/* Try not to override a larger table with a smaller one! */
			if (old_table->mc_mask > new_table->mc_mask) {
				Dee_membercache_table_destroy(new_table);
				goto do_operate_with_old_table;
			}
			Dee_membercache_table_decref(old_table);
		}
	}

	/* Store our new table within the member-cache. */
	old_table = atomic_xch(&self->mc_table, new_table); /* Inherit reference */
	Dee_membercache_waitfor(self);

	/* Destroy the old table reference. */
	if (old_table)
		Dee_membercache_table_decref(old_table);

	/* Make sure that the member-cache controller is linked into the global list. */
	if (!LIST_ISBOUND(self, mc_link)) {
		membercache_list_lock_acquire();
		if likely(!LIST_ISBOUND(self, mc_link))
			LIST_INSERT_HEAD(&membercache_list, self, mc_link);
		membercache_list_lock_release();
	}

	Dee_membercache_addslot_log_success(self, slot);
	return 0;
}




INTERN NONNULL((1, 2, 4)) int DCALL
Dee_membercache_addmethod(struct Dee_membercache *self,
                          DeeTypeObject *decl, dhash_t hash,
                          struct type_method const *method) {
	struct Dee_membercache_slot slot;
	slot.mcs_type = MEMBERCACHE_METHOD;
	slot.mcs_hash = hash;
	slot.mcs_decl = decl;
	memcpy(&slot.mcs_method, method, sizeof(struct type_method));
	return Dee_membercache_addslot(self, &slot);
}

INTERN NONNULL((1, 2, 4)) int DCALL
Dee_membercache_addinstancemethod(struct Dee_membercache *self,
                                  DeeTypeObject *decl, dhash_t hash,
                                  struct type_method const *method) {
	struct Dee_membercache_slot slot;
	ASSERT(self != &decl->tp_cache);
	slot.mcs_type = MEMBERCACHE_INSTANCE_METHOD;
	slot.mcs_hash = hash;
	slot.mcs_decl = decl;
	memcpy(&slot.mcs_method, method, sizeof(struct type_method));
	return Dee_membercache_addslot(self, &slot);
}

INTERN NONNULL((1, 2, 4)) int DCALL
Dee_membercache_addgetset(struct Dee_membercache *self,
                          DeeTypeObject *decl, dhash_t hash,
                          struct type_getset const *getset) {
	struct Dee_membercache_slot slot;
	slot.mcs_type = MEMBERCACHE_GETSET;
	slot.mcs_hash = hash;
	slot.mcs_decl = decl;
	memcpy(&slot.mcs_getset, getset, sizeof(struct type_getset));
	return Dee_membercache_addslot(self, &slot);
}

INTERN NONNULL((1, 2, 4)) int DCALL
Dee_membercache_addinstancegetset(struct Dee_membercache *self,
                                  DeeTypeObject *decl, dhash_t hash,
                                  struct type_getset const *getset) {
	struct Dee_membercache_slot slot;
	ASSERT(self != &decl->tp_cache);
	slot.mcs_type = MEMBERCACHE_INSTANCE_GETSET;
	slot.mcs_hash = hash;
	slot.mcs_decl = decl;
	memcpy(&slot.mcs_getset, getset, sizeof(struct type_getset));
	return Dee_membercache_addslot(self, &slot);
}

INTERN NONNULL((1, 2, 4)) int DCALL
Dee_membercache_addmember(struct Dee_membercache *self,
                          DeeTypeObject *decl, dhash_t hash,
                          struct type_member const *member) {
	struct Dee_membercache_slot slot;
	slot.mcs_type = MEMBERCACHE_MEMBER;
	slot.mcs_hash = hash;
	slot.mcs_decl = decl;
	memcpy(&slot.mcs_member, member, sizeof(struct type_member));
	return Dee_membercache_addslot(self, &slot);
}

INTERN NONNULL((1, 2, 4)) int DCALL
Dee_membercache_addinstancemember(struct Dee_membercache *self,
                                  DeeTypeObject *decl, dhash_t hash,
                                  struct type_member const *member) {
	struct Dee_membercache_slot slot;
	ASSERT(self != &decl->tp_cache);
	slot.mcs_type = MEMBERCACHE_INSTANCE_MEMBER;
	slot.mcs_hash = hash;
	slot.mcs_decl = decl;
	memcpy(&slot.mcs_member, member, sizeof(struct type_member));
	return Dee_membercache_addslot(self, &slot);
}

INTERN NONNULL((1, 2, 4)) int DCALL
Dee_membercache_addattrib(struct Dee_membercache *self,
                          DeeTypeObject *decl, dhash_t hash,
                          struct class_attribute *attrib) {
	struct Dee_membercache_slot slot;
	slot.mcs_type          = MEMBERCACHE_ATTRIB;
	slot.mcs_hash          = hash;
	slot.mcs_decl          = decl;
	slot.mcs_attrib.a_name = DeeString_STR(attrib->ca_name);
	slot.mcs_attrib.a_attr = attrib;
	slot.mcs_attrib.a_desc = DeeClass_DESC(decl);
	return Dee_membercache_addslot(self, &slot);
}

INTERN NONNULL((1, 2, 4)) int DCALL
Dee_membercache_addinstanceattrib(struct Dee_membercache *self,
                                  DeeTypeObject *decl, dhash_t hash,
                                  struct class_attribute *attrib) {
	struct Dee_membercache_slot slot;
	ASSERT(self != &decl->tp_cache);
	slot.mcs_type          = MEMBERCACHE_INSTANCE_ATTRIB;
	slot.mcs_hash          = hash;
	slot.mcs_decl          = decl;
	slot.mcs_attrib.a_name = DeeString_STR(attrib->ca_name);
	slot.mcs_attrib.a_attr = attrib;
	slot.mcs_attrib.a_desc = DeeClass_DESC(decl);
	return Dee_membercache_addslot(self, &slot);
}


/* >> bool Dee_membercache_acquiretable(struct Dee_membercache *self, [[out]] DREF struct Dee_membercache_table **p_table);
 * >> void Dee_membercache_releasetable(struct Dee_membercache *self, [[in]] DREF struct Dee_membercache_table *table);
 * Helpers to acquire and release cache tables. */
#define Dee_membercache_acquiretable(self, p_table)                   \
	(Dee_membercache_tabuse_inc(self),                                \
	 *(p_table) = atomic_read(&(self)->mc_table),                     \
	 *(p_table) ? Dee_membercache_table_incref(*(p_table)) : (void)0, \
	 Dee_membercache_tabuse_dec(self),                                \
	 *(p_table) != NULL)
#define Dee_membercache_releasetable(self, table) \
	Dee_membercache_table_decref(table)

/* Patch a member cache slot
 * @return:  1: Slot cannot be patched like that
 * @return:  0: Success
 * @return: -1: No cache table allocated */
PRIVATE NONNULL((1, 2, 3, 6, 7)) int DCALL
Dee_membercache_patch(struct Dee_membercache *self, DeeTypeObject *decl,
                      char const *attr, Dee_hash_t hash, uintptr_t attr_type,
                      bool (DCALL *do_patch)(struct Dee_membercache_slot *slot,
                                             void const *new_data,
                                             void const *old_data),
                      void const *new_data, void const *old_data) {
	int result = 1;
	Dee_hash_t i, perturb;
	DREF struct Dee_membercache_table *table;
	if unlikely(!Dee_membercache_acquiretable(self, &table))
		return -1;
	perturb = i = Dee_membercache_table_hashst(table, hash);
	for (;; Dee_membercache_table_hashnx(i, perturb)) {
		struct Dee_membercache_slot *item;
		uint16_t type;
		item = Dee_membercache_table_hashit(table, i);
		type = atomic_read(&item->mcs_type);
		if (type == MEMBERCACHE_UNUSED)
			break;
		if (item->mcs_hash != hash)
			continue;
		if unlikely(type == MEMBERCACHE_UNINITIALIZED)
			continue; /* Don't dereference uninitialized items! */
		if (strcmp(item->mcs_name, attr) != 0)
			continue;

		/* Ensure that the attribute type matches. */
		if (type != attr_type)
			return 1;

		/* Found it! -> now patch it */
		if ((*do_patch)(item, new_data, old_data)) {
			atomic_write(&item->mcs_decl, decl);
			result = 0;
			Dee_DPRINTF("[RT] Patched %s `%s.%s' in `%s' (%s)\n",
			            membercache_type_names[attr_type],
			            decl->tp_name, attr,
			            MEMBERCACHE_GETTYPENAME(self),
			            MEMBERCACHE_GETCLASSNAME(self));
		}
		break;
	}
	Dee_membercache_releasetable(self, table);
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) bool DCALL
do_patch_method(struct Dee_membercache_slot *slot, void const *new_data, void const *old_data) {
	bool result = false;
	struct type_method const *new_method = (struct type_method const *)new_data;
	struct type_method const *old_method = (struct type_method const *)old_data;
	if ((slot->mcs_method.m_flag & TYPE_METHOD_FKWDS) !=
	    (new_method->m_flag & TYPE_METHOD_FKWDS))
		return false;
	if (old_method) {
		result = atomic_cmpxch(&slot->mcs_method.m_func,
		                       old_method->m_func,
		                       new_method->m_func);
	} else {
		atomic_write(&slot->mcs_method.m_func, new_method->m_func);
		result = true;
	}
	return result;
}


PRIVATE NONNULL((1, 2, 4)) int DCALL
Dee_membercache_patchmethod(struct Dee_membercache *self, DeeTypeObject *decl, Dee_hash_t hash,
                            struct type_method const *new_method,
                            /*0..1*/ struct type_method const *old_method) {
#ifdef CONFIG_NO_THREADS
	int result = Dee_membercache_addmethod(self, decl, hash, new_method);
#else  /* CONFIG_NO_THREADS */
	int result;
	while ((result = Dee_membercache_addmethod(self, decl, hash, new_method)) == 2)
		SCHED_YIELD();
#endif /* !CONFIG_NO_THREADS */
	if (result > 0) {
		/* Entry already exists (try to patch it) */
		result = Dee_membercache_patch(self, decl, new_method->m_name, hash,
		                               MEMBERCACHE_METHOD, &do_patch_method,
		                               new_method, old_method);
	}
	return result;
}

PRIVATE NONNULL((1, 2, 4)) int DCALL
Dee_membercache_patchinstancemethod(struct Dee_membercache *self, DeeTypeObject *decl, Dee_hash_t hash,
                                    struct type_method const *new_method,
                                    /*0..1*/ struct type_method const *old_method) {
#ifdef CONFIG_NO_THREADS
	int result = Dee_membercache_addinstancemethod(self, decl, hash, new_method);
#else  /* CONFIG_NO_THREADS */
	int result;
	while ((result = Dee_membercache_addinstancemethod(self, decl, hash, new_method)) == 2)
		SCHED_YIELD();
#endif /* !CONFIG_NO_THREADS */
	if (result > 0) {
		/* Entry already exists (try to patch it) */
		result = Dee_membercache_patch(self, decl, new_method->m_name, hash,
		                               MEMBERCACHE_INSTANCE_METHOD, &do_patch_method,
		                               new_method, old_method);
	}
	return result;
}


PRIVATE WUNUSED NONNULL((1, 2)) bool DCALL
do_patch_getset(struct Dee_membercache_slot *slot, void const *new_data, void const *old_data) {
	bool result = false;
	struct type_getset const *new_getset = (struct type_getset const *)new_data;
	struct type_getset const *old_getset = (struct type_getset const *)old_data;
	if (old_getset) {
		if (atomic_cmpxch(&slot->mcs_getset.gs_get, old_getset->gs_get, new_getset->gs_get))
			result = true;
		if (atomic_cmpxch(&slot->mcs_getset.gs_del, old_getset->gs_del, new_getset->gs_del))
			result = true;
		if (atomic_cmpxch(&slot->mcs_getset.gs_set, old_getset->gs_set, new_getset->gs_set))
			result = true;
		if (atomic_cmpxch(&slot->mcs_getset.gs_bound, old_getset->gs_bound, new_getset->gs_bound))
			result = true;
	} else {
		atomic_write(&slot->mcs_getset.gs_get, new_getset->gs_get);
		atomic_write(&slot->mcs_getset.gs_del, new_getset->gs_del);
		atomic_write(&slot->mcs_getset.gs_set, new_getset->gs_set);
		atomic_write(&slot->mcs_getset.gs_bound, new_getset->gs_bound);
		result = true;
	}
	return result;
}


PRIVATE NONNULL((1, 2, 4)) int DCALL
Dee_membercache_patchgetset(struct Dee_membercache *self, DeeTypeObject *decl, Dee_hash_t hash,
                            struct type_getset const *new_getset,
                            /*0..1*/ struct type_getset const *old_getset) {
#ifdef CONFIG_NO_THREADS
	int result = Dee_membercache_addgetset(self, decl, hash, new_getset);
#else  /* CONFIG_NO_THREADS */
	int result;
	while ((result = Dee_membercache_addgetset(self, decl, hash, new_getset)) == 2)
		SCHED_YIELD();
#endif /* !CONFIG_NO_THREADS */
	if (result > 0) {
		/* Entry already exists (try to patch it) */
		result = Dee_membercache_patch(self, decl, new_getset->gs_name, hash,
		                               MEMBERCACHE_GETSET, &do_patch_getset,
		                               new_getset, old_getset);
	}
	return result;
}

PRIVATE NONNULL((1, 2, 4)) int DCALL
Dee_membercache_patchinstancegetset(struct Dee_membercache *self, DeeTypeObject *decl, Dee_hash_t hash,
                                    struct type_getset const *new_getset,
                                    /*0..1*/ struct type_getset const *old_getset) {
#ifdef CONFIG_NO_THREADS
	int result = Dee_membercache_addinstancegetset(self, decl, hash, new_getset);
#else  /* CONFIG_NO_THREADS */
	int result;
	while ((result = Dee_membercache_addinstancegetset(self, decl, hash, new_getset)) == 2)
		SCHED_YIELD();
#endif /* !CONFIG_NO_THREADS */
	if (result > 0) {
		/* Entry already exists (try to patch it) */
		result = Dee_membercache_patch(self, decl, new_getset->gs_name, hash,
		                               MEMBERCACHE_INSTANCE_GETSET, &do_patch_getset,
		                               new_getset, old_getset);
	}
	return result;
}



/* Try to add the specified attribute to the cache of "self".
 * - If this fails due to OOM, return `-1', but DON'T throw an exception
 * If the MRO cache already contains an entry for the named attribute:
 * - Verify that the existing entry is for the same type of attribute (`MEMBERCACHE_*'),
 *   such that it can be patched without having to alter `mcs_type' (since having to do
 *   so would result in a non-resolvable race condition where another thread is currently
 *   dereferencing the function pointers from the existing entry).
 *   If this verification fails, return `1'.
 *   - For `DeeTypeMRO_Patch*Method', it is also verified that both the
 *     old and new function pointers share a common `TYPE_METHOD_FKWDS'.
 * - If the type matches, the pre-existing cache entries pointers are patched such that
 *   they will now reference those from the given parameters.
 *   Note that for this purpose, this exchange is atomic for each individual function
 *   pointer (but not all pointers as a whole) -- in the case of `DeeTypeMRO_Patch*GetSet',
 *   another thread may invoke (e.g.) an out-dated `gs_del' after `gs_get' was already
 *   patched.
 *
 * NOTE: Generally, only use these functions for self-optimizing methods in base-classes
 *       that wish to skip certain type-dependent verification steps during future calls.
 *       (e.g. `Sequence.first', `Mapping.keys')
 *
 * @param: old_*: [0..1] When non-NULL, use these values for compare-exchange operations.
 *                       But also note that when there are many function pointers, some may
 *                       be set, while others cannot be -- here, an attempt to exchange
 *                       pointers is made for *all* pointers, and success is indicated if
 *                       at least 1 pointer could be exchanged.
 * @return:  1:   Failure (cache entry cannot be patched like this)
 * @return:  0:   Success
 * @return: -1:   Patching failed due to OOM (but no error was thrown!) */

PUBLIC NONNULL((1, 2, 4)) int DCALL
DeeTypeMRO_PatchMethod(DeeTypeObject *self, DeeTypeObject *decl, Dee_hash_t hash,
                       struct type_method const *new_method, /*0..1*/ struct type_method const *old_method) {
	int result = Dee_membercache_patchmethod(&self->tp_cache, decl, hash, new_method, old_method);
	if likely(result == 0)
		result = Dee_membercache_patchinstancemethod(&self->tp_class_cache, decl, hash, new_method, old_method);
	return result;
}

PUBLIC NONNULL((1, 2, 4)) int DCALL
DeeTypeMRO_PatchGetSet(DeeTypeObject *self, DeeTypeObject *decl, Dee_hash_t hash,
                       struct type_getset const *new_getset, /*0..1*/ struct type_getset const *old_getset) {
	int result = Dee_membercache_patchgetset(&self->tp_cache, decl, hash, new_getset, old_getset);
	if likely(result == 0)
		result = Dee_membercache_patchinstancegetset(&self->tp_class_cache, decl, hash, new_getset, old_getset);
	return result;
}

PUBLIC NONNULL((1, 2, 4)) int DCALL
DeeTypeMRO_PatchClassMethod(DeeTypeObject *self, DeeTypeObject *decl, Dee_hash_t hash,
                            struct type_method const *new_method, /*0..1*/ struct type_method const *old_method) {
	return Dee_membercache_patchmethod(&self->tp_class_cache, decl, hash, new_method, old_method);
}

PUBLIC NONNULL((1, 2, 4)) int DCALL
DeeTypeMRO_PatchClassGetSet(DeeTypeObject *self, DeeTypeObject *decl, Dee_hash_t hash,
                            struct type_getset const *new_getset, /*0..1*/ struct type_getset const *old_getset) {
	return Dee_membercache_patchgetset(&self->tp_class_cache, decl, hash, new_getset, old_getset);
}



#ifndef CONFIG_CALLTUPLE_OPTIMIZATIONS
/* TODO: For binary compat:
 * #define DEFINE_DeeType_CallCachedAttrStringHashTuple
 * #define DEFINE_DeeType_CallCachedClassAttrStringHashTuple
 * #define DEFINE_DeeType_CallCachedAttrStringHashTupleKw
 * #define DEFINE_DeeType_CallCachedClassAttrStringHashTupleKw
 */
#endif /* !CONFIG_CALLTUPLE_OPTIMIZATIONS */


/* Reurns the object-type used to represent a given type-member. */
INTDEF WUNUSED NONNULL((1)) DeeTypeObject *DCALL
type_member_typefor(struct type_member const *__restrict self);


INTERN WUNUSED NONNULL((1, 2, 3, 5, 6)) int DCALL /* METHOD */
type_method_findattr(struct Dee_membercache *cache, DeeTypeObject *decl,
                     struct type_method const *chain, uint16_t perm,
                     struct attribute_info *__restrict result,
                     struct attribute_lookup_rules const *__restrict rules) {
	ASSERT(perm & (ATTR_IMEMBER | ATTR_CMEMBER));
	perm |= ATTR_PERMGET | ATTR_PERMCALL;
	if ((perm & rules->alr_perm_mask) != rules->alr_perm_value)
		goto nope;
	for (; chain->m_name; ++chain) {
		if (!streq(chain->m_name, rules->alr_name))
			continue;
		Dee_membercache_addmethod(cache, decl, rules->alr_hash, chain);
		ASSERT(!(perm & ATTR_DOCOBJ));
		result->a_doc      = chain->m_doc;
		result->a_decl     = (DREF DeeObject *)decl;
		result->a_perm     = perm;
		result->a_attrtype = (chain->m_flag & TYPE_METHOD_FKWDS)
		                     ? &DeeKwObjMethod_Type
		                     : &DeeObjMethod_Type;
		Dee_Incref(result->a_attrtype);
		Dee_Incref(decl);
		return 0;
	}
nope:
	return 1;
}

INTERN WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
DeeType_FindInstanceMethodAttr(DeeTypeObject *tp_invoker,
                               DeeTypeObject *tp_self,
                               struct attribute_info *__restrict result,
                               struct attribute_lookup_rules const *__restrict rules) {
	uint16_t perm;
	struct type_method const *chain;
	perm = ATTR_IMEMBER | ATTR_CMEMBER | ATTR_PERMGET | ATTR_PERMCALL | ATTR_WRAPPER;
	if ((perm & rules->alr_perm_mask) != rules->alr_perm_value)
		goto nope;
	chain = tp_self->tp_methods;
	for (; chain->m_name; ++chain) {
		if (!streq(chain->m_name, rules->alr_name))
			continue;
		Dee_membercache_addinstancemethod(&tp_invoker->tp_class_cache, tp_self, rules->alr_hash, chain);
		ASSERT(!(perm & ATTR_DOCOBJ));
		result->a_doc      = chain->m_doc;
		result->a_decl     = (DREF DeeObject *)tp_self;
		result->a_perm     = perm;
		result->a_attrtype = (chain->m_flag & TYPE_METHOD_FKWDS)
		                     ? &DeeKwObjMethod_Type
		                     : &DeeObjMethod_Type;
		Dee_Incref(result->a_attrtype);
		Dee_Incref(tp_self);
		return 0;
	}
nope:
	return 1;
}

INTERN WUNUSED NONNULL((1, 2, 3, 5, 6)) int DCALL /* GETSET */
type_getset_findattr(struct Dee_membercache *cache, DeeTypeObject *decl,
                     struct type_getset const *chain, uint16_t perm,
                     struct attribute_info *__restrict result,
                     struct attribute_lookup_rules const *__restrict rules) {
	ASSERT(perm & (ATTR_IMEMBER | ATTR_CMEMBER));
	perm |= ATTR_PROPERTY;
	for (; chain->gs_name; ++chain) {
		uint16_t flags = perm;
		if (chain->gs_get)
			flags |= ATTR_PERMGET;
		if (chain->gs_del)
			flags |= ATTR_PERMDEL;
		if (chain->gs_set)
			flags |= ATTR_PERMSET;
		if ((flags & rules->alr_perm_mask) != rules->alr_perm_value)
			continue;
		if (!streq(chain->gs_name, rules->alr_name))
			continue;
		Dee_membercache_addgetset(cache, decl, rules->alr_hash, chain);
		ASSERT(!(perm & ATTR_DOCOBJ));
		result->a_doc      = chain->gs_doc;
		result->a_perm     = flags;
		result->a_decl     = (DREF DeeObject *)decl;
		result->a_attrtype = NULL;
		Dee_Incref(decl);
		return 0;
	}
	return 1;
}

INTERN WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
DeeType_FindInstanceGetSetAttr(DeeTypeObject *tp_invoker,
                               DeeTypeObject *tp_self,
                               struct attribute_info *__restrict result,
                               struct attribute_lookup_rules const *__restrict rules) {
	uint16_t perm;
	struct type_getset const *chain;
	perm  = ATTR_PROPERTY | ATTR_WRAPPER | ATTR_IMEMBER | ATTR_CMEMBER;
	chain = tp_self->tp_getsets;
	for (; chain->gs_name; ++chain) {
		uint16_t flags = perm;
		if (chain->gs_get)
			flags |= ATTR_PERMGET;
		if (chain->gs_del)
			flags |= ATTR_PERMDEL;
		if (chain->gs_set)
			flags |= ATTR_PERMSET;
		if ((flags & rules->alr_perm_mask) != rules->alr_perm_value)
			continue;
		if (!streq(chain->gs_name, rules->alr_name))
			continue;
		Dee_membercache_addinstancegetset(&tp_invoker->tp_class_cache, tp_self, rules->alr_hash, chain);
		ASSERT(!(perm & ATTR_DOCOBJ));
		result->a_doc      = chain->gs_doc;
		result->a_perm     = flags;
		result->a_decl     = (DREF DeeObject *)tp_self;
		result->a_attrtype = NULL;
		Dee_Incref(tp_self);
		return 0;
	}
	return 1;
}

INTERN WUNUSED NONNULL((1, 2, 3, 5, 6)) int DCALL /* MEMBER */
type_member_findattr(struct Dee_membercache *cache, DeeTypeObject *decl,
                     struct type_member const *chain, uint16_t perm,
                     struct attribute_info *__restrict result,
                     struct attribute_lookup_rules const *__restrict rules) {
	ASSERT(perm & (ATTR_IMEMBER | ATTR_CMEMBER));
	perm |= ATTR_PERMGET;
	for (; chain->m_name; ++chain) {
		uint16_t flags = perm;
		if (!TYPE_MEMBER_ISCONST(chain) &&
		    !(chain->m_field.m_type & STRUCT_CONST))
			flags |= (ATTR_PERMDEL | ATTR_PERMSET);
		if ((flags & rules->alr_perm_mask) != rules->alr_perm_value)
			continue;
		if (!streq(chain->m_name, rules->alr_name))
			continue;
		Dee_membercache_addmember(cache, decl, rules->alr_hash, chain);
		ASSERT(!(perm & ATTR_DOCOBJ));
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

INTERN WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
DeeType_FindInstanceMemberAttr(DeeTypeObject *tp_invoker,
                               DeeTypeObject *tp_self,
                               struct attribute_info *__restrict result,
                               struct attribute_lookup_rules const *__restrict rules) {
	uint16_t perm;
	struct type_member const *chain;
	perm  = ATTR_WRAPPER | ATTR_IMEMBER | ATTR_CMEMBER | ATTR_PERMGET;
	chain = tp_self->tp_members;
	for (; chain->m_name; ++chain) {
		uint16_t flags = perm;
		if (!TYPE_MEMBER_ISCONST(chain) &&
		    !(chain->m_field.m_type & STRUCT_CONST))
			flags |= (ATTR_PERMDEL | ATTR_PERMSET);
		if ((flags & rules->alr_perm_mask) != rules->alr_perm_value)
			continue;
		if (!streq(chain->m_name, rules->alr_name))
			continue;
		Dee_membercache_addinstancemember(&tp_invoker->tp_class_cache, tp_self, rules->alr_hash, chain);
		ASSERT(!(perm & ATTR_DOCOBJ));
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


DECL_END

/* Define cache accessor functions */
#ifndef __INTELLISENSE__
#define DEFINE_DeeType_GetCachedAttrStringHash
#include "mro-impl-cache.c.inl"
#define DEFINE_DeeType_GetCachedAttrStringLenHash
#include "mro-impl-cache.c.inl"
#define DEFINE_DeeType_GetCachedClassAttrStringHash
#include "mro-impl-cache.c.inl"
#define DEFINE_DeeType_GetCachedClassAttrStringLenHash
#include "mro-impl-cache.c.inl"
#define DEFINE_DeeType_GetCachedInstanceAttrStringHash
#include "mro-impl-cache.c.inl"
#define DEFINE_DeeType_GetCachedInstanceAttrStringLenHash
#include "mro-impl-cache.c.inl"

#define DEFINE_DeeType_BoundCachedAttrStringHash
#include "mro-impl-cache.c.inl"
#define DEFINE_DeeType_BoundCachedAttrStringLenHash
#include "mro-impl-cache.c.inl"
#define DEFINE_DeeType_BoundCachedClassAttrStringHash
#include "mro-impl-cache.c.inl"
#define DEFINE_DeeType_BoundCachedClassAttrStringLenHash
#include "mro-impl-cache.c.inl"
#define DEFINE_DeeType_BoundCachedInstanceAttrStringHash
#include "mro-impl-cache.c.inl"
#define DEFINE_DeeType_BoundCachedInstanceAttrStringLenHash
#include "mro-impl-cache.c.inl"

#define DEFINE_DeeType_HasCachedAttrStringHash
#include "mro-impl-cache.c.inl"
#define DEFINE_DeeType_HasCachedAttrStringLenHash
#include "mro-impl-cache.c.inl"
#define DEFINE_DeeType_HasCachedClassAttrStringHash
#include "mro-impl-cache.c.inl"
#define DEFINE_DeeType_HasCachedClassAttrStringLenHash
#include "mro-impl-cache.c.inl"

#define DEFINE_DeeType_DelCachedAttrStringHash
#include "mro-impl-cache.c.inl"
#define DEFINE_DeeType_DelCachedAttrStringLenHash
#include "mro-impl-cache.c.inl"
#define DEFINE_DeeType_DelCachedClassAttrStringHash
#include "mro-impl-cache.c.inl"
#define DEFINE_DeeType_DelCachedClassAttrStringLenHash
#include "mro-impl-cache.c.inl"
#define DEFINE_DeeType_DelCachedInstanceAttrStringHash
#include "mro-impl-cache.c.inl"
#define DEFINE_DeeType_DelCachedInstanceAttrStringLenHash
#include "mro-impl-cache.c.inl"

#define DEFINE_DeeType_SetCachedAttrStringHash
#include "mro-impl-cache.c.inl"
#define DEFINE_DeeType_SetCachedAttrStringLenHash
#include "mro-impl-cache.c.inl"
#define DEFINE_DeeType_SetCachedClassAttrStringHash
#include "mro-impl-cache.c.inl"
#define DEFINE_DeeType_SetCachedClassAttrStringLenHash
#include "mro-impl-cache.c.inl"
#define DEFINE_DeeType_SetCachedInstanceAttrStringHash
#include "mro-impl-cache.c.inl"
#define DEFINE_DeeType_SetCachedInstanceAttrStringLenHash
#include "mro-impl-cache.c.inl"

#define DEFINE_DeeType_SetBasicCachedAttrStringHash
#include "mro-impl-cache.c.inl"
#define DEFINE_DeeType_SetBasicCachedAttrStringLenHash
#include "mro-impl-cache.c.inl"
#define DEFINE_DeeType_SetBasicCachedClassAttrStringHash
#include "mro-impl-cache.c.inl"
#define DEFINE_DeeType_SetBasicCachedClassAttrStringLenHash
#include "mro-impl-cache.c.inl"
//#define DEFINE_DeeType_SetBasicCachedInstanceAttrStringHash
//#include "mro-impl-cache.c.inl"
//#define DEFINE_DeeType_SetBasicCachedInstanceAttrStringLenHash
//#include "mro-impl-cache.c.inl"

#define DEFINE_DeeType_CallCachedAttrStringHash
#include "mro-impl-cache.c.inl"
#define DEFINE_DeeType_CallCachedAttrStringLenHash
#include "mro-impl-cache.c.inl"
#define DEFINE_DeeType_CallCachedClassAttrStringHash
#include "mro-impl-cache.c.inl"
#define DEFINE_DeeType_CallCachedClassAttrStringLenHash
#include "mro-impl-cache.c.inl"
//#define DEFINE_DeeType_CallCachedInstanceAttrStringHash
//#include "mro-impl-cache.c.inl"
//#define DEFINE_DeeType_CallCachedInstanceAttrStringLenHash
//#include "mro-impl-cache.c.inl"

#define DEFINE_DeeType_CallCachedAttrStringHashKw
#include "mro-impl-cache.c.inl"
#define DEFINE_DeeType_CallCachedAttrStringLenHashKw
#include "mro-impl-cache.c.inl"
#define DEFINE_DeeType_CallCachedClassAttrStringHashKw
#include "mro-impl-cache.c.inl"
#define DEFINE_DeeType_CallCachedClassAttrStringLenHashKw
#include "mro-impl-cache.c.inl"
#define DEFINE_DeeType_CallCachedInstanceAttrStringHashKw
#include "mro-impl-cache.c.inl"
#define DEFINE_DeeType_CallCachedInstanceAttrStringLenHashKw
#include "mro-impl-cache.c.inl"

#ifdef CONFIG_CALLTUPLE_OPTIMIZATIONS
#define DEFINE_DeeType_CallCachedAttrStringHashTuple
#include "mro-impl-cache.c.inl"
//#define DEFINE_DeeType_CallCachedAttrStringLenHashTuple
//#include "mro-impl-cache.c.inl"
#define DEFINE_DeeType_CallCachedClassAttrStringHashTuple
#include "mro-impl-cache.c.inl"
//#define DEFINE_DeeType_CallCachedClassAttrStringLenHashTuple
//#include "mro-impl-cache.c.inl"
//#define DEFINE_DeeType_CallCachedInstanceAttrStringHashTuple
//#include "mro-impl-cache.c.inl"
//#define DEFINE_DeeType_CallCachedInstanceAttrStringLenHashTuple
//#include "mro-impl-cache.c.inl"

#define DEFINE_DeeType_CallCachedAttrStringHashTupleKw
#include "mro-impl-cache.c.inl"
//#define DEFINE_DeeType_CallCachedAttrStringLenHashTupleKw
//#include "mro-impl-cache.c.inl"
#define DEFINE_DeeType_CallCachedClassAttrStringHashTupleKw
#include "mro-impl-cache.c.inl"
//#define DEFINE_DeeType_CallCachedClassAttrStringLenHashTupleKw
//#include "mro-impl-cache.c.inl"
//#define DEFINE_DeeType_CallCachedInstanceAttrStringHashTupleKw
//#include "mro-impl-cache.c.inl"
//#define DEFINE_DeeType_CallCachedInstanceAttrStringLenHashTupleKw
//#include "mro-impl-cache.c.inl"
#endif /* CONFIG_CALLTUPLE_OPTIMIZATIONS */

#define DEFINE_DeeType_VCallCachedAttrStringHashf
#include "mro-impl-cache.c.inl"
//#define DEFINE_DeeType_VCallCachedAttrStringLenHashf
//#include "mro-impl-cache.c.inl"
#define DEFINE_DeeType_VCallCachedClassAttrStringHashf
#include "mro-impl-cache.c.inl"
//#define DEFINE_DeeType_VCallCachedClassAttrStringLenHashf
//#include "mro-impl-cache.c.inl"
//#define DEFINE_DeeType_VCallCachedInstanceAttrStringHashf
//#include "mro-impl-cache.c.inl"
//#define DEFINE_DeeType_VCallCachedInstanceAttrStringLenHashf
//#include "mro-impl-cache.c.inl"

//#define DEFINE_DeeType_FindCachedAttrInfoStringHash
//#include "mro-impl-cache.c.inl"
//#define DEFINE_DeeType_FindCachedClassAttrInfoStringHash
//#include "mro-impl-cache.c.inl"
#define DEFINE_DeeType_FindCachedAttrInfoStringLenHash
#include "mro-impl-cache.c.inl"
#define DEFINE_DeeType_FindCachedClassAttrInfoStringLenHash
#include "mro-impl-cache.c.inl"

#define DEFINE_DeeType_FindCachedAttr
#include "mro-impl-cache.c.inl"
#define DEFINE_DeeType_FindCachedClassAttr
#include "mro-impl-cache.c.inl"
#endif /* !__INTELLISENSE__ */

#ifndef __INTELLISENSE__
#include "mro-impl.c.inl"
#define DEFINE_MRO_ATTRLEN_FUNCTIONS
#include "mro-impl.c.inl"
#endif /* !__INTELLISENSE__ */

#endif /* !GUARD_DEEMON_RUNTIME_MRO_C */
