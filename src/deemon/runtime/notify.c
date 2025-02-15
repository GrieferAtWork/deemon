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
#ifndef GUARD_DEEMON_RUNTIME_NOTIFY_C
#define GUARD_DEEMON_RUNTIME_NOTIFY_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/error.h>
#include <deemon/module.h>
#include <deemon/none.h>
#include <deemon/notify.h>
#include <deemon/object.h>
#include <deemon/string.h>
#include <deemon/system-features.h> /* bcmpc(), ... */
#include <deemon/util/atomic.h>
#include <deemon/util/lock.h>

#include <hybrid/limitcore.h>
#include <hybrid/typecore.h>

#include "strings.h"

#ifndef CONFIG_NO_NOTIFICATIONS
DECL_BEGIN

PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
Dee_GetEnv(DeeObject *__restrict name) {
	DREF DeeObject *result;
	DREF DeeObject *posix_environ;
	posix_environ = DeeModule_GetExtern((DeeObject *)&str_posix,
	                                    (DeeObject *)&str_environ);
	if unlikely(!posix_environ)
		goto err_tryhandle;
	result = DeeObject_TryGetItem(posix_environ, name);
	Dee_Decref(posix_environ);
	if unlikely(!result)
		goto err_tryhandle;
	/* Since the posix module may be overwritten somehow,
	 * we have to assert that we actually got a string. */
	if (result != ITER_DONE) {
		if (DeeObject_AssertTypeExact(result, &DeeString_Type))
			goto err_r;
	}
	return result;
err_tryhandle:
	if (DeeError_Catch(&DeeError_ValueError) ||     /* Super-error that includes `KeyError' (thrown when a key (env_name) wasn't found) */
	    DeeError_Catch(&DeeError_AttributeError) || /* Attribute-error (thrown when `posix' doesn't export a symbol `environ') */
	    DeeError_Catch(&DeeError_UnsupportedAPI) || /* Unsupported-API (thrown when `posix' only provides a stub implementation for `environ') */
	    DeeError_Catch(&DeeError_NotImplemented))   /* Not-implemented error (thrown if `posix.environ' doesn't support lookup) */
		return ITER_DONE;
	return NULL;
err_r:
	Dee_Decref(result);
/*err:*/
	return NULL;
}


/* Use libc functions for case-insensitive UTF-8 string compare when available. */
#ifdef CONFIG_HAVE_memcasecmp
#define MEMCASEEQ(a, b, s) (memcasecmp(a, b, s) == 0)
#else /* CONFIG_HAVE_memcasecmp */
#define MEMCASEEQ(a, b, s) dee_memcaseeq((uint8_t *)(a), (uint8_t *)(b), s)
LOCAL WUNUSED NONNULL((1, 2)) bool dee_memcaseeq(uint8_t const *a, uint8_t const *b, size_t s) {
	while (s--) {
		if (DeeUni_ToLower(*a) != DeeUni_ToLower(*b))
			return false;
		++a;
		++b;
	}
	return true;
}
#endif /* !CONFIG_HAVE_memcasecmp */



struct notify_entry {
	uint16_t              nh_class; /* The notification class. */
	uint16_t              nh_pad[(sizeof(void *)-2)/2]; /* ... */
	DREF DeeStringObject *nh_name;  /* [0..1] The notification name (or NULL if the entry is unused) */
	dhash_t               nh_hash;  /* The effective hash of the name (using `Dee_HashCasePtr' when `Dee_NOTIFICATION_CLASS_FNOCASE' is set) */
	Dee_notify_t          nh_func;  /* [1..1][valid_if(nh_name)] The callback invoked for the purposes of this notification. */
	DREF DeeObject       *nh_arg;   /* [0..1][valid_if(nh_name)] The argument passed to `nh_func' */
};

PRIVATE ATTR_COLD int DCALL
dummy_notify(DeeObject *UNUSED(arg)) {
	return 0;
}

PRIVATE struct notify_entry const empty_notifications[1] = {
	{ 0, { 0, }, NULL, 0, NULL, 0 }
};

#ifndef CONFIG_NO_THREADS
PRIVATE Dee_atomic_rwlock_t notify_lock = DEE_ATOMIC_RWLOCK_INIT;
#endif /* !CONFIG_NO_THREADS */
#define notify_lock_reading()    Dee_atomic_rwlock_reading(&notify_lock)
#define notify_lock_writing()    Dee_atomic_rwlock_writing(&notify_lock)
#define notify_lock_tryread()    Dee_atomic_rwlock_tryread(&notify_lock)
#define notify_lock_trywrite()   Dee_atomic_rwlock_trywrite(&notify_lock)
#define notify_lock_canread()    Dee_atomic_rwlock_canread(&notify_lock)
#define notify_lock_canwrite()   Dee_atomic_rwlock_canwrite(&notify_lock)
#define notify_lock_waitread()   Dee_atomic_rwlock_waitread(&notify_lock)
#define notify_lock_waitwrite()  Dee_atomic_rwlock_waitwrite(&notify_lock)
#define notify_lock_read()       Dee_atomic_rwlock_read(&notify_lock)
#define notify_lock_write()      Dee_atomic_rwlock_write(&notify_lock)
#define notify_lock_tryupgrade() Dee_atomic_rwlock_tryupgrade(&notify_lock)
#define notify_lock_upgrade()    Dee_atomic_rwlock_upgrade(&notify_lock)
#define notify_lock_downgrade()  Dee_atomic_rwlock_downgrade(&notify_lock)
#define notify_lock_endwrite()   Dee_atomic_rwlock_endwrite(&notify_lock)
#define notify_lock_endread()    Dee_atomic_rwlock_endread(&notify_lock)
#define notify_lock_end()        Dee_atomic_rwlock_end(&notify_lock)

/* The usual deal: it's a hash-vector (s.a.: Dict, member_table, etc. etc.) */
PRIVATE size_t /*         */ notify_used = 0;
PRIVATE size_t /*         */ notify_size = 0;
PRIVATE size_t /*         */ notify_mask = 0;
PRIVATE struct notify_entry *notify_list = (struct notify_entry *)empty_notifications;

#define NOTIFY_HASHST(hash)        ((hash) & notify_mask)
#define NOTIFY_HASHNX(hs, perturb) (void)((hs) = ((hs) << 2) + (hs) + (perturb) + 1, (perturb) >>= 5) /* This `5' is tunable. */
#define NOTIFY_HASHIT(i)           (notify_list + ((i) & notify_mask))


/* Resize the hash size by a factor of 2 and re-insert all elements.
 * When `sizedir > 0', increase the hash; When `sizedir < 0', decrease it.
 * During this process all dummy items are discarded.
 * @return: true:  Successfully rehashed the notification map.
 * @return: false: Not enough memory. - The caller should collect some and try again. */
PRIVATE bool DCALL
notify_rehash(int sizedir) {
	struct notify_entry *new_vector, *iter, *end;
	size_t new_mask = notify_mask;
	if (sizedir > 0) {
		new_mask = (new_mask << 1) | 1;
		if unlikely(new_mask == 1)
			new_mask = 16 - 1; /* Start out bigger than 2. */
	} else if (sizedir < 0) {
		new_mask = (new_mask >> 1);
		if unlikely(!new_mask) {
			ASSERT(!notify_used);
			/* Special case: delete the vector. */
			if (notify_list != empty_notifications)
				Dee_Free(notify_list);
			notify_list = (struct notify_entry *)empty_notifications;
			notify_mask = 0;
			notify_size = 0;
			return true;
		}
	}
	ASSERT(notify_size < new_mask);
	ASSERT(notify_used <= notify_size);
	new_vector = (struct notify_entry *)Dee_TryCallocc(new_mask + 1, sizeof(struct notify_entry));
	if unlikely(!new_vector)
		return false;
	ASSERT((notify_list == empty_notifications) == (notify_mask == 0));
	ASSERT((notify_list == empty_notifications) == (notify_size == 0));
	if (notify_list != empty_notifications) {
		/* Re-insert all existing items into the new notify vector. */
		end = (iter = notify_list) + (notify_mask + 1);
		for (; iter < end; ++iter) {
			struct notify_entry *item;
			dhash_t i, perturb;
			/* Skip dummy keys. */
			if (!iter->nh_name || iter->nh_func == &dummy_notify)
				continue;
			perturb = i = iter->nh_hash & new_mask;
			for (;; NOTIFY_HASHNX(i, perturb)) {
				item = &new_vector[i & new_mask];
				if (!item->nh_name)
					break; /* Empty slot found. */
			}
			/* Transfer this object. */
			item->nh_class = iter->nh_class;
			item->nh_name  = iter->nh_name;
			item->nh_hash  = iter->nh_hash;
			item->nh_func  = iter->nh_func;
			item->nh_arg   = iter->nh_arg;
		}
		Dee_Free(notify_list);
		/* With all dummy items gone, the size now equals what is actually used. */
		notify_size = notify_used;
	}
	ASSERT(notify_size == notify_used);
	notify_mask = new_mask;
	notify_list = new_vector;
	return true;
}



/* Add/remove a notification listener for a given class and name.
 * @param:  arg: When non-NULL, an object to which the internal
 *               notification registration will keep a reference.
 *               Additionally, this object is passed to `callback'
 *               whenever it is invoked.
 * @param:  cls: One of `NOTIFICATION_CLASS_*'
 * @param: name: The name of the notification to listen for. (Must be a string; e.g.: Name of an environment variable)
 * @return:   0: Successfully registered/removed the given `callback' and `arg' for `cls' and `name'.
 * @return:   1: The given `callback' has already / hasn't been registered for `cls'
 *               and `name' with the same `arg' and was not registered again / removed.
 * @return:  -1: An error occurred (Never returned by `DeeNotify_EndListen'). */
PUBLIC WUNUSED NONNULL((2, 3)) int DCALL
DeeNotify_StartListen(uint16_t cls, DeeObject *__restrict name,
                      Dee_notify_t callback, DeeObject *arg) {
	struct notify_entry *first_dummy;
	dhash_t hash, perturb, i;
	ASSERT_OBJECT_TYPE_EXACT(name, &DeeString_Type);
	hash = (cls & Dee_NOTIFICATION_CLASS_FNOCASE)
	       ? DeeString_HashCase(name)
	       : DeeString_Hash(name);
again_lock:
	notify_lock_write();
again:
	first_dummy = NULL;
	perturb = i = NOTIFY_HASHST(hash);
	for (;; NOTIFY_HASHNX(i, perturb)) {
		struct notify_entry *entry = NOTIFY_HASHIT(i);
		if (!entry->nh_name) {
			if (!first_dummy)
				first_dummy = entry;
			break; /* Not found */
		}
		if (entry->nh_func == &dummy_notify) {
			first_dummy = entry;
			continue;
		}
		if (entry->nh_hash != hash)
			continue; /* Non-matching hash */
		if (entry->nh_class != cls)
			continue; /* Different class. */
		if (entry->nh_arg != arg)
			continue; /* Different argument. */
		if (entry->nh_func != callback)
			continue; /* Different callback. */
		if (entry->nh_name == (DREF DeeStringObject *)name ||
		    (DeeString_SIZE(entry->nh_name) == DeeString_SIZE(name) &&
		     (cls & Dee_NOTIFICATION_CLASS_FNOCASE)
		     ? MEMCASEEQ(entry->nh_name, DeeString_STR(name), DeeString_SIZE(name) * sizeof(char))
		     : 0 == bcmpc(entry->nh_name, DeeString_STR(name), DeeString_SIZE(name), sizeof(char)))) {
			/* Already exists. */
			notify_lock_endwrite();
			return 1;
		}
	}
	if (first_dummy && notify_size + 1 < notify_mask) {
		/* Fill in the target slot. */
		Dee_Incref(name);
		Dee_XIncref(arg);
		first_dummy->nh_name  = (DREF DeeStringObject *)name; /* Inherit. */
		first_dummy->nh_class = cls;
		first_dummy->nh_hash  = hash;
		first_dummy->nh_func  = callback;
		first_dummy->nh_arg   = arg;
		++notify_used;
		++notify_size;
		/* Try to keep the notify vector big at least twice as big as the element count. */
		if (notify_size * 2 > notify_mask)
			notify_rehash(1);
		notify_lock_endwrite();
		return 0;
	}
	/* Rehash the notify and try again. */
	if (notify_rehash(1))
		goto again;
	notify_lock_endwrite();
	if (Dee_CollectMemory(1))
		goto again_lock;
	return -1;
}


PUBLIC NONNULL((2, 3)) int DCALL
DeeNotify_EndListen(uint16_t cls, DeeObject *__restrict name,
                    Dee_notify_t callback, DeeObject *arg) {
	dhash_t hash, perturb, i;
	ASSERT_OBJECT_TYPE_EXACT(name, &DeeString_Type);
	hash = (cls & Dee_NOTIFICATION_CLASS_FNOCASE)
	       ? DeeString_HashCase(name)
	       : DeeString_Hash(name);
	notify_lock_write();
	if unlikely(!notify_used)
		goto not_found;
	perturb = i = hash & notify_mask;
	for (;; NOTIFY_HASHNX(i, perturb)) {
		struct notify_entry *entry = NOTIFY_HASHIT(i);
		if (!entry->nh_name)
			goto not_found; /* Never registered. */
		if (entry->nh_hash != hash)
			continue; /* Different hash. */
		if (entry->nh_class != cls)
			continue; /* Different class. */
		if (entry->nh_arg != arg)
			continue; /* Different argument. */
		if (entry->nh_func != callback)
			continue; /* Different callback. */
		if (entry->nh_name == (DREF DeeStringObject *)name ||
		    (DeeString_SIZE(entry->nh_name) == DeeString_SIZE(name) &&
		     (cls & Dee_NOTIFICATION_CLASS_FNOCASE)
		     ? MEMCASEEQ(entry->nh_name, DeeString_STR(name), DeeString_SIZE(name) * sizeof(char))
		     : 0 == bcmpc(entry->nh_name, DeeString_STR(name), DeeString_SIZE(name), sizeof(char)))) {
			/* Found it! (Replace with a dummy notification) */
			entry->nh_func = &dummy_notify;
			entry->nh_arg  = NULL;
			break;
		}
	}
	/* Keep track of how many entries are in use. */
	if (--notify_used <= notify_size / 3)
		notify_rehash(-1);
	notify_lock_endwrite();
	Dee_XDecref(arg);
	return 0;
not_found:
	notify_lock_endwrite();
	return 1;
}


/* Broadcast a change notification for the given class `cls' and `name'
 * NOTE: The caller is responsible for passing a string for `name'
 * @return:  0 : Success.
 * @return: -1 : Callback invocation was stopped after a callback indicated an error. */
PRIVATE WUNUSED NONNULL((2)) int DCALL
DeeNotify_DoBroadcast(uint16_t cls,
                      char const *__restrict name,
                      size_t name_size,
                      dhash_t name_hash) {
	int result = 0;
	dhash_t perturb, i;
	size_t mask;
	struct notify_entry *list;
	notify_lock_read();
again:
	mask    = notify_mask;
	list    = notify_list;
	perturb = i = name_hash & mask;
	for (;; NOTIFY_HASHNX(i, perturb)) {
		struct notify_entry *entry = NOTIFY_HASHIT(i);
		if (!entry->nh_name)
			break;
		if (entry->nh_class != cls)
			continue; /* Check the class. */
		if (entry->nh_hash != name_hash)
			continue; /* Check the hash. */
		if (entry->nh_func == &dummy_notify)
			continue; /* Skip dummy notifications. */
		if (entry->nh_name->s_str == name ||
		    (DeeString_SIZE(entry->nh_name) == name_size &&
		     (cls & Dee_NOTIFICATION_CLASS_FNOCASE)
		     ? MEMCASEEQ(entry->nh_name, name, name_size * sizeof(char))
		     : 0 == bcmpc(entry->nh_name, name, name_size, sizeof(char)))) {
			DREF DeeObject *arg;
			Dee_notify_t func;
			func = entry->nh_func;
			arg  = entry->nh_arg;
			Dee_XIncref(arg);
			COMPILER_READ_BARRIER();

			/* Found an entry that we're supposed to invoke! */
			notify_lock_endread();

			/* Invoke the notification callback. */
			result = (*func)(arg);
			Dee_XDecref(arg);

			/* If an error occurred, forward it. */
			if unlikely(result)
				goto done;
			notify_lock_read();

			/* If the notification map changed, start
			 * over, so we can call everything. */
			if (mask != notify_mask ||
			    list != notify_list)
				goto again;
		}
	}
	notify_lock_endread();
done:
	return result;
}

PUBLIC WUNUSED NONNULL((2)) int
(DCALL DeeNotify_Broadcast)(uint16_t cls, DeeObject *__restrict name) {
	size_t name_size;
	dhash_t name_hash;
	ASSERT_OBJECT_TYPE_EXACT(name, &DeeString_Type);
	name_size = DeeString_SIZE(name);
	name_hash = (cls & Dee_NOTIFICATION_CLASS_FNOCASE)
	            ? DeeString_HashCase(name)
	            : DeeString_Hash(name);
	return DeeNotify_DoBroadcast(cls, DeeString_STR(name), name_size, name_hash);
}

PUBLIC WUNUSED NONNULL((2)) int
(DCALL DeeNotify_BroadcastString)(uint16_t cls, char const *__restrict name) {
	size_t name_size;
	dhash_t name_hash;
	name_size = strlen(name);
	name_hash = (cls & Dee_NOTIFICATION_CLASS_FNOCASE)
	            ? Dee_HashCasePtr(name, name_size * sizeof(char))
	            : Dee_HashPtr(name, name_size * sizeof(char));
	return DeeNotify_DoBroadcast(cls, name, name_size, name_hash);
}

PUBLIC WUNUSED int
(DCALL DeeNotify_BroadcastClass)(uint16_t cls) {
	int result = 0;
	size_t mask, i;
	struct notify_entry *list;
	notify_lock_read();
again:
	mask = notify_mask;
	list = notify_list;
	for (i = 0; i <= mask; ++i) {
		DREF DeeObject *arg;
		Dee_notify_t func;
		struct notify_entry *entry;
		entry = &notify_list[i];
		if (!entry->nh_name)
			break;
		if (entry->nh_class != cls)
			continue; /* Check the class. */
		if (entry->nh_func == &dummy_notify)
			continue; /* Skip dummy notifications. */
		func = entry->nh_func;
		arg  = entry->nh_arg;
		Dee_XIncref(arg);
		COMPILER_READ_BARRIER();
		notify_lock_endread();

		/* Invoke the notification callback. */
		result = (*func)(arg);
		Dee_XDecref(arg);

		/* If an error occurred, forward it. */
		if unlikely(result)
			goto done;
		notify_lock_read();

		/* If the notification map changed, start
			* over, so we can call everything. */
		if (mask != notify_mask ||
			list != notify_list)
			goto again;
	}
	notify_lock_endread();
done:
	return result;
}



/* Delete all registered notification callbacks.
 * @return: true:  At least one registration was deleted.
 * @return: false: There was nothing to delete. */
INTERN bool DCALL DeeNotify_Shutdown(void) {
	size_t mask;
	struct notify_entry *list;
	notify_lock_write();
	mask        = notify_mask;
	list        = notify_list;
	notify_used = 0;
	notify_size = 0;
	notify_mask = 0;
	notify_list = (struct notify_entry *)empty_notifications;
	notify_lock_endwrite();
	COMPILER_WRITE_BARRIER();
	ASSERT((mask == 0) == (list == empty_notifications));
	if (list == empty_notifications)
		return false;
	/* Clear out the notification vector and discard it. */
	do {
		if (list[mask].nh_name) {
			Dee_Decref(list[mask].nh_name);
			Dee_XDecref(list[mask].nh_arg);
		}
	} while (--mask);
	Dee_Free(list);
	return true;
}


DECL_END
#endif /* !CONFIG_NO_NOTIFICATIONS */

#endif /* !GUARD_DEEMON_RUNTIME_NOTIFY_C */
