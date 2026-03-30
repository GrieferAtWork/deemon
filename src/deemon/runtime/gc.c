/* Copyright (c) 2018-2026 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2026 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_RUNTIME_GC_C
#define GUARD_DEEMON_RUNTIME_GC_C 1
#define DEE_SOURCE

#include <deemon/api.h>

#include <deemon/alloc.h>           /* DeeDbgObject_*, DeeObject_*alloc*, DeeObject_Free, DeeSlab_ENUMERATE, DeeSlab_Invoke */
#include <deemon/code.h>            /* DeeCodeObject, DeeCode_NAME, DeeCode_Type, DeeFunction_CODE, DeeFunction_Type */
#include <deemon/format.h>          /* PRFuSIZ */
#include <deemon/gc.h>              /* DeeGC_*, Dee_GC_*, Dee_gc_head */
#include <deemon/module.h>          /* DeeModule* */
#include <deemon/object.h>          /* DREF, DeeObject, DeeTypeObject, DeeType_Extends, Dee_AsObject, Dee_TYPE, Dee_refcnt_t, Dee_ssize_t, Dee_weakref_list, Dee_weakref_support_fini, OBJECT_HEAD */
#include <deemon/system-features.h> /* memset, remove */
#include <deemon/system.h>          /* DeeSystem_GetWalltime */
#include <deemon/thread.h>          /* DeeThreadObject, DeeThread_CheckInterrupt, DeeThread_CheckInterruptNoInt, DeeThread_IsMultiThreaded, DeeThread_ResumeAll, DeeThread_Self, DeeThread_SuspendAll, DeeThread_TrySuspendAll, DeeThread_WasInterrupted, Dee_THREAD_STATE_SHUTDOWNINTR */
#include <deemon/type.h>            /* DeeType_*, Dee_visit_t, TF_TPVISIT */
#include <deemon/util/atomic.h>     /* Dee_atomic_*, atomic_* */
#include <deemon/util/lock.h>       /* Dee_atomic_lock_*, Dee_shared_lock_* */
#include <deemon/util/weakref.h>    /* Dee_weakref_list_transfer_to_dummy */

#include <hybrid/overflow.h>    /* OVERFLOW_UADD */
#include <hybrid/sched/yield.h> /* SCHED_YIELD */
#include <hybrid/typecore.h>    /* __BYTE_TYPE__ */

#include "strings.h"

#include <stdbool.h> /* bool, true */
#include <stddef.h>  /* offsetof, size_t */
#include <stdint.h>  /* uintptr_t */

#ifndef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
#ifndef CONFIG_NO_DEX
#include <deemon/dex.h> /* DeeDex_Check, DeeDex_Type */
#endif /* !CONFIG_NO_DEX */
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */

#undef byte_t
#define byte_t __BYTE_TYPE__

#ifndef NDEBUG
#define DBG_memset (void)memset
#else /* !NDEBUG */
#define DBG_memset(dst, byte, n_bytes) (void)0
#endif /* NDEBUG */

#if !defined(NDEBUG) && 0
#if 0
#define GC_TRACE(...) fprintf(stderr, __VA_ARGS__)
#else
#define GC_TRACE(...) Dee_DPRINTF(__VA_ARGS__)
#endif
#else
#define GC_TRACE(...) (void)0
#define GC_TRACE_IS_NOOP
#endif

DECL_BEGIN

#ifndef NDEBUG
PRIVATE NONNULL((1, 2)) void DCALL
gc_dprint_object_info(DeeTypeObject *tp, DeeObject *ob) {
	if (tp == &DeeType_Type) {
		Dee_DPRINTF(" {tp_name:%q}", ((DeeTypeObject *)ob)->tp_name);
	} else if (tp == &DeeCode_Type) {
#ifdef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
		DeeCodeObject *me;
		DeeModuleObject *mod;
#endif /* CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
print_code:
#ifdef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
		me  = (DeeCodeObject *)ob;
		mod = me->co_module;
		if (mod && (Dee_TYPE(mod) == &DeeModuleDee_Type ||
		            Dee_TYPE(mod) == &DeeModuleDex_Type ||
		            Dee_TYPE(mod) == &DeeModuleDir_Type)) {
			Dee_DPRINTF(" {co_module:%q, co_name:%q}",
			            DeeModule_GetShortName(mod),
			            DeeCode_NAME(ob));
		} else
#endif /* CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
		{
			Dee_DPRINTF(" {co_name:%q}", DeeCode_NAME(ob));
		}
	} else if (tp == &DeeFunction_Type) {
		ob = Dee_AsObject(DeeFunction_CODE(ob));
		if (ob && Dee_TYPE(ob) == &DeeCode_Type)
			goto print_code;
#ifdef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
	} else if (tp == &DeeModuleDee_Type ||
	           tp == &DeeModuleDex_Type ||
	           tp == &DeeModuleDir_Type) {
		Dee_DPRINTF(" {mo_absname:%q}", ((DeeModuleObject *)ob)->mo_absname);
#else  /* CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
	} else if (tp == &DeeModule_Type) {
		Dee_DPRINTF(" {mo_name:%r}", ((DeeModuleObject *)ob)->mo_name);
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
	}
}
#endif /* !NDEBUG */


/* Visit references to (potential) GC objects that are held by "self".
 * This operator is necessary to implement GC support, but is generic
 * enough to also allow it to be exposed here.
 *
 * WARNING: **NEVER** do anything that might block inside of `proc' --
 *          implementations of this operator may invoke `proc' while
 *          certain internal locks are held.
 * WARNING: Implementors of this operator must not alter the reference
 *          counters of referenced objects in this operator, and must
 *          also **NOT** perform debug checks on reference counts. This
 *          operator may be called while "self->ob_refcnt == 0", or the
 *          objects referenced by "self" have weird/non-sense reference
 *          counts (this is intentional and the result of GC operations) */
PUBLIC NONNULL((1, 2)) void DCALL
DeeObject_Visit(DeeObject *__restrict self, Dee_visit_t proc, void *arg) {
	DeeTypeObject *tp_self = Dee_TYPE(self);
	do {
		if (tp_self->tp_visit) {
			if (tp_self->tp_features & TF_TPVISIT) {
				typedef void (DCALL *tp_tvisit_t)(DeeTypeObject *tp_self, DeeObject *self,
				                                  Dee_visit_t proc, void *arg);
				(*(tp_tvisit_t)tp_self->tp_visit)(tp_self, self, proc, arg);
			} else {
				(*tp_self->tp_visit)(self, proc, arg);
			}
		}
	} while ((tp_self = DeeType_Base(tp_self)) != NULL);

	/* Only visit heap-allocated types.
	 *
	 * While technically we'd also need to visit statically allocated
	 * types, since objects *do* hold proper references to them, those
	 * types can never be destroyed, and thus *really* aren't of any
	 * interest to GC visitation (since the GC only cares about stuff
	 * that it can actually destroy/clear). */
	if (DeeType_IsHeapType(Dee_TYPE(self)))
		(*proc)(Dee_AsObject(Dee_TYPE(self)), arg);
}



struct gc_generation {
	DeeObject            *gg_objects;    /* [0..gg_count][lock(gc_lock)] Chain of GC objects within this generation */
	size_t                gg_count;      /* [lock(gc_lock)] # of objects within this generation
	                                      * (only an exact number for generation #0; in other
	                                      * generation, this represents an upper bound) */
	size_t                gg_fixed_count;/* [lock(gc_lock)][valid_if(self != &gc_gen0)] Same as `gg_count', but used to speed up `gc_generation_fixcount()' */
	Dee_ssize_t           gg_collect_on; /* [lock(gc_lock)] Decremented on each object add -- when this becomes negative, collect objects of this generation */
	Dee_ssize_t           gg_mindim;     /* [lock(gc_lock)] Lower bound for `gg_collect_on' */
	struct gc_generation *gg_next;       /* [0..1][lock(gc_lock)] Next generation (or "NULL" if this is the last one) */
};
#define GC_GENERATION_INIT(mindim, next) \
	{ NULL, 0, 0, mindim, mindim, next }

/* Use these macros to ensure that a pointer to some `struct gc_generation'
 * remains valid, even after `gc_lock_release()' has been called. Calls to
 * these macros can also be made without `gc_lock' being held. */
#define gc_generation_incref(self) (void)0
#define gc_generation_decref(self) (void)0


/* [0..1][lock(ATOMIC)] Objects that are pending insertion into "gc_gen0" (linked via `gc_next') */
PRIVATE DeeObject *gc_insert = NULL;
#define gc_insert__p_link(ob) (&DeeGC_Head(ob)->gc_next)

/* [0..1][lock(ATOMIC)] Objects that are pending removal (linked via `ob_refcnt')
 * (weakrefs of these objects have already been killed, and user-defined "tp_finalize" has
 * also already been invoked for these objects). Only used by special implementations of
 * `DeeObject_Destroy()', such that object destruction continues when this list is reaped.
 *
 * NOTE: All objects within this linked list have the `Dee_GC_FLAG_FINALIZED' flag set.
 *       As such: when trying to incref objects found in the GC, anything that has a non-
 *       zero reference count and does not have the `Dee_GC_FLAG_FINALIZED' flag set can
 *       be incref'd, without needing to reap/search the "gc_remove" list. */
PRIVATE DeeObject *gc_remove = NULL;
#define gc_remove__p_link(ob) ((DeeObject **)((byte_t *)(ob) + offsetof(DeeObject, ob_refcnt)))

/* When non-zero, some thread is in the process of adding elements to "gc_remove"
 * When this is the case (or when "gc_remove != NULL"), then objects found within
 * individual GC generations may actually be dead, even if their "ob_refcnt != 0",
 * since the "ob_refcnt" field is (ab-)used to implement the "gc_remove" chain.
 *
 * For this purpose, "DeeGC_Collect()" & friends will ensure that no thread is still
 * trying to enqueue objects to-be removed by waiting until "gc_remove_modifying"
 * becomes "0". */
PRIVATE unsigned int gc_remove_modifying = 0;

/* [lock(gc_lock)] GC generations */
PRIVATE struct gc_generation gc_genn = GC_GENERATION_INIT(64 * 1024, NULL);
/* More generations could go here... */
PRIVATE struct gc_generation gc_gen2 = GC_GENERATION_INIT(64 * 512, &gc_genn);
PRIVATE struct gc_generation gc_gen1 = GC_GENERATION_INIT(64 * 256, &gc_gen2);
PRIVATE struct gc_generation gc_gen0 = GC_GENERATION_INIT(64 * 128, &gc_gen1); /* XXX: Tune these! */

/* [lock(ATOMIC)] true if "_gc_lock_reap_and_maybe_unlock" must be called */
PRIVATE bool gc_mustreap = false;

/* Lock for the GC system */
#ifndef CONFIG_NO_THREADS
PRIVATE Dee_atomic_lock_t gc_lock = Dee_ATOMIC_LOCK_INIT;
#endif /* !CONFIG_NO_THREADS */

#define gc_lock_available()  Dee_atomic_lock_available(&gc_lock)
#define gc_lock_acquired()   Dee_atomic_lock_acquired(&gc_lock)
#define gc_lock_tryacquire() Dee_atomic_lock_tryacquire(&gc_lock)
#define gc_lock_waitfor()    Dee_atomic_lock_waitfor(&gc_lock)
#define _gc_lock_release()   Dee_atomic_lock_release(&gc_lock)

PRIVATE void DCALL gc_lock_acquire(void) {
	while (!gc_lock_tryacquire()) {
		DeeThread_CheckInterruptNoInt();
		SCHED_YIELD();
	}
}

/* Called to (try to) collect GC objects after "gg_collect_on" of some
 * GC generation was reached. This function is must be called without
 * the calling thread holding any (internal) locks, include locks
 * related to GC (such as `gc_lock')
 *
 * @return: true:  Success
 * @return: false: Failure (collect failed; caller must set "gc_mustreap"
 *                 to true, but not try again right now...) */
PRIVATE WUNUSED bool DCALL gc_trycollect_after_collect_on_reached(void);

/* Try to collect objects from GC generation #0
 * @return: true:  Success
 * @return: false: Collect failed */
PRIVATE WUNUSED bool DCALL gc_trycollect_gen0(void);

/* Do everything normally done by "DeeObject_Destroy" after  */
PRIVATE NONNULL((1)) void DCALL
DeeGCObject_FinishDestroyAfterUntrack(DeeObject *__restrict ob);

#ifndef __INTELLISENSE__
DECL_END
#define DEFINE_DeeGCObject_FinishDestroyAfterUntrack
#include "refcnt-destroy.c.inl"
DECL_BEGIN
#endif /* !__INTELLISENSE__ */

/* Fix the "gg_count" of a given generation */
PRIVATE NONNULL((1)) void DCALL
gc_generation_fixcount(struct gc_generation *__restrict self) {
	DeeObject *iter;
	size_t actual_count;
	ASSERTF(self != &gc_gen0, "Count of gen#0 never needs fixing");
	if (self->gg_fixed_count == self->gg_count)
		return; /* Fixed count is still up-to-date */
	actual_count = 0;
	for (iter = self->gg_objects; iter; iter = DeeGC_Head(iter)->gc_next)
		++actual_count;

	/* Only lower actual counts are allowed! */
	ASSERTF(actual_count <= self->gg_count,
	        "Bad count (actual: %" PRFuSIZ ", expected: %" PRFuSIZ ")",
	        actual_count, self->gg_count);

	/* Push back the GC collection threshold */
	self->gg_collect_on += (self->gg_count - actual_count);
	self->gg_count = actual_count;
	self->gg_fixed_count = actual_count;
}

PRIVATE NONNULL((1)) void DCALL
gc_object_set_pself(DeeObject ***p_p_self, DeeObject **p_self) {
	DeeObject **oldval, **newval;
	ASSERT(!((uintptr_t)p_self & Dee_GC_FLAG_MASK));
	do {
		oldval = atomic_read(p_p_self);
		newval = (DeeObject **)(((uintptr_t)oldval & Dee_GC_FLAG_MASK) | (uintptr_t)p_self);
	} while (!atomic_cmpxch_weak_or_write(p_p_self, oldval, newval));
}

PRIVATE void DCALL gc_reap_insert(void) {
	struct Dee_gc_head *head;
	size_t pending_count;
	DeeObject *iter, **p_iter;
	DeeObject *pending_insert = atomic_xch(&gc_insert, NULL);
	if (!pending_insert)
		return;

	/* Count pending objects and fill in their "gi_pself" pointers */
	p_iter = &gc_gen0.gg_objects;
	iter = pending_insert;
	for (pending_count = 0;;) {
		++pending_count;
		head = DeeGC_Head(iter);
		ASSERT(!(atomic_read(&head->gc_info.gi_flag) & Dee_GC_FLAG_GENN));
		gc_object_set_pself(&head->gc_info.gi_pself, p_iter);
		p_iter = &head->gc_next;
		if (!head->gc_next)
			break;
		iter = head->gc_next;
	}

	/* Link new objects into "self->gg_objects" */
	if ((head->gc_next = gc_gen0.gg_objects) != NULL) {
		head = DeeGC_Head(gc_gen0.gg_objects);
		gc_object_set_pself(&head->gc_info.gi_pself, p_iter);
	}
	gc_gen0.gg_objects = pending_insert;

	/* Adjust counters based on pending-object counts. */
	gc_gen0.gg_count += pending_count;
	gc_gen0.gg_collect_on -= pending_count;
}

PRIVATE NONNULL((1)) void DCALL
gc_do_untrack_locked(DeeObject *__restrict ob) {
	struct Dee_gc_head *head = DeeGC_Head(ob);
	DeeObject **p_self = atomic_read(&head->gc_info.gi_pself);
	if (((uintptr_t)p_self & Dee_GC_FLAG_GENN) == 0) {
		/* Object is part of generation #0 */
		ASSERT(gc_gen0.gg_count);
		--gc_gen0.gg_count;
		++gc_gen0.gg_collect_on;
	}
	p_self = (DeeObject **)((uintptr_t)p_self & ~Dee_GC_FLAG_MASK);
	if ((*p_self = head->gc_next) != NULL) {
		struct Dee_gc_head *next_head = DeeGC_Head(head->gc_next);
		gc_object_set_pself(&next_head->gc_info.gi_pself, p_self);
	}
	DBG_memset(&head->gc_info.gi_pself, 0xcc, sizeof(head->gc_info.gi_pself));
	DBG_memset(&head->gc_next, 0xcc, sizeof(head->gc_next));
}


#ifndef GenericObject_DEFINED
#define GenericObject_DEFINED
typedef struct {
	OBJECT_HEAD
} GenericObject;
#endif /* !GenericObject_DEFINED */

/* Possible return values for `_gc_lock_reap_and_maybe_unlock()' */
#define REAP_STATUS_RETAINED             0 /* Lock wasn't released */
#define REAP_STATUS_UNLOCKED             1 /* Lock was released */
#define REAP_STATUS_UNLOCKED_RETRY_LATER 2 /* Lock was released; caller should set "gc_mustreap" but not retry (unless "gc_mustreap" was already "true") */

/* @return: * : one of `REAP_STATUS_*' */
PRIVATE ATTR_NOINLINE unsigned int DCALL
_gc_lock_reap_and_maybe_unlock(unsigned int flags) {
	bool should_collect;
	DeeObject *pending_remove, *removeme;
	struct gc_generation *iter;

	/* Clear the must-reap flag */
	atomic_write(&gc_mustreap, false);

	/* Consume list of pending remove operations */
	pending_remove = atomic_xch(&gc_remove, NULL);

	/* Reap pending inserts for every generation */
	gc_reap_insert();

	/* Check if objects should be collected in any generation. */
	iter = &gc_gen0;
	should_collect = false;
	do {
		if (iter->gg_collect_on < 0)
			should_collect = true;
	} while ((iter = iter->gg_next) != NULL);

	if (!pending_remove && !should_collect)
		return REAP_STATUS_RETAINED; /* Didn't have to release lock */

	/* Perform remove operations for "pending_remove" */
	for (removeme = pending_remove; removeme;
	     removeme = *gc_remove__p_link(removeme))
		gc_do_untrack_locked(removeme);

	/* Release GC lock. */
	_gc_lock_release();

	/* Continue destruction of objects in "pending_remove" */
	while (pending_remove) {
		DeeObject *next = *gc_remove__p_link(pending_remove);
		((GenericObject *)pending_remove)->ob_refcnt = 0;
		COMPILER_WRITE_BARRIER();
		DeeGCObject_FinishDestroyAfterUntrack(pending_remove);
		pending_remove = next;
	}

	/* Collect garbage if there are generations with `gg_collect_on < 0' */
	if (should_collect) {
		if (flags & DeeGC_TRACK_F_NOCOLLECT)
			return REAP_STATUS_UNLOCKED_RETRY_LATER;
		if (!gc_trycollect_after_collect_on_reached())
			return REAP_STATUS_UNLOCKED_RETRY_LATER;
	}

	/* Did have to unlock */
	return REAP_STATUS_UNLOCKED;
}

PRIVATE void DCALL gc_lock_release(unsigned int flags) {
	if (atomic_read(&gc_mustreap)) {
		unsigned int status;
again_reap:
		status = _gc_lock_reap_and_maybe_unlock(flags);
		switch (status) {
		case REAP_STATUS_RETAINED:
			_gc_lock_release();
			break;
		case REAP_STATUS_UNLOCKED:
			break;
		case REAP_STATUS_UNLOCKED_RETRY_LATER:
			if (!atomic_xch(&gc_mustreap, true))
				return;
			break;
		default: __builtin_unreachable();
		}
	} else {
		_gc_lock_release();
	}
	if (atomic_read(&gc_mustreap) && gc_lock_tryacquire())
		goto again_reap;
}

PRIVATE bool DCALL gc_lock_reap_and_release(unsigned int flags) {
	do {
		unsigned int status;
		status = _gc_lock_reap_and_maybe_unlock(flags);
		switch (status) {
		case REAP_STATUS_RETAINED:
			_gc_lock_release();
			break;
		case REAP_STATUS_UNLOCKED:
			break;
		case REAP_STATUS_UNLOCKED_RETRY_LATER:
			if (!atomic_xch(&gc_mustreap, true))
				return false;
			break;
		default: __builtin_unreachable();
		}
	} while (atomic_read(&gc_mustreap) && gc_lock_tryacquire());
	return true;
}

PRIVATE void DCALL gc_lock_reap(unsigned int flags) {
	if (gc_lock_tryacquire())
		gc_lock_reap_and_release(flags);
}

PRIVATE void DCALL gc_lock_acquire_and_reap(unsigned int flags) {
	unsigned int status;
again:
	gc_lock_acquire();
	if (!atomic_read(&gc_mustreap))
		return; /* Nothing to reap! */
	status = _gc_lock_reap_and_maybe_unlock(flags);
	switch (status) {
	case REAP_STATUS_RETAINED:
		return;
	case REAP_STATUS_UNLOCKED:
		goto again;
	case REAP_STATUS_UNLOCKED_RETRY_LATER:
		if (atomic_xch(&gc_mustreap, true))
			goto again;
		gc_lock_acquire();
		/* In this case, everything that has happened before has been reaped! */
		return;
	default: __builtin_unreachable();
	}
}


/* Begin tracking a given GC-allocated object. */
PUBLIC ATTR_RETNONNULL NONNULL((1)) DREF DeeObject *DCALL
DeeGC_Track(DREF DeeObject *__restrict ob) {
	bool must_collect;
	struct Dee_gc_head *head = DeeGC_Head(ob);
	ASSERT(!((uintptr_t)&head->gc_next & Dee_GC_FLAG_MASK));
#ifndef GC_TRACE_IS_NOOP
	if (atomic_read(&DeeThread_Self()->t_state) & Dee_THREAD_STATE_SHUTDOWNINTR) {
		GC_TRACE("%llu: %p: DeeGC_Track(%s, %p)\n",
		         DeeSystem_GetWalltime(), DeeThread_Self(),
		         ob->ob_type->tp_name, ob);
	}
#endif /* !GC_TRACE_IS_NOOP */

	if (!gc_lock_tryacquire()) {
		/* Setup as a pending insert */
		DeeObject *next;
		do {
			next = atomic_read(&gc_insert);
			head->gc_next = next;
			COMPILER_WRITE_BARRIER();
		} while (!atomic_cmpxch(&gc_insert, next, ob));
		atomic_write(&gc_mustreap, true);
		gc_lock_reap(DeeGC_TRACK_F_NORMAL);
		return ob;
	}

	/* Insert new GC object into generation #0 */
	head->gc_info.gi_pself = &gc_gen0.gg_objects;
	if ((head->gc_next = gc_gen0.gg_objects) != NULL) {
		struct Dee_gc_head *next_head = DeeGC_Head(gc_gen0.gg_objects);
		ASSERT((DeeObject **)((uintptr_t)atomic_read(&next_head->gc_info.gi_pself) & ~Dee_GC_FLAG_MASK) ==
		       &gc_gen0.gg_objects);
		gc_object_set_pself(&next_head->gc_info.gi_pself, &head->gc_next);
	}
	gc_gen0.gg_objects = ob;

	/* Update counters. */
	++gc_gen0.gg_count;
	--gc_gen0.gg_collect_on;

	/* Check if objects from generation #0 must be collected now */
	must_collect = gc_gen0.gg_collect_on < 0;

	/* Release GC lock */
	gc_lock_release(DeeGC_TRACK_F_NORMAL);

	/* If necessary, *try* to collect GC objects from generation #0 */
	if (must_collect) {
		if (!gc_trycollect_gen0()) {
			/* If collect failed, let someone else try again later... */
			if (atomic_xch(&gc_mustreap, true))
				gc_lock_reap(DeeGC_TRACK_F_NORMAL);
		}
	}
	return ob;
}


PUBLIC ATTR_RETNONNULL NONNULL((1)) DREF DeeObject *DCALL
DeeGC_TrackEx(DREF DeeObject *__restrict ob, unsigned int flags) {
	bool should_collect;
	struct Dee_gc_head *head = DeeGC_Head(ob);
	ASSERT(!((uintptr_t)&head->gc_next & Dee_GC_FLAG_MASK));
	if (!gc_lock_tryacquire()) {
		/* Setup as a pending insert */
		DeeObject *next;
		do {
			next = atomic_read(&gc_insert);
			head->gc_next = next;
			COMPILER_WRITE_BARRIER();
		} while (!atomic_cmpxch(&gc_insert, next, ob));
		atomic_write(&gc_mustreap, true);
		gc_lock_reap(flags);
		return ob;
	}

	/* Insert new GC object into generation #0 */
	head->gc_info.gi_pself = &gc_gen0.gg_objects;
	if ((head->gc_next = gc_gen0.gg_objects) != NULL) {
		struct Dee_gc_head *next_head = DeeGC_Head(gc_gen0.gg_objects);
		ASSERT((DeeObject **)((uintptr_t)atomic_read(&next_head->gc_info.gi_pself) & ~Dee_GC_FLAG_MASK) ==
		       &gc_gen0.gg_objects);
		gc_object_set_pself(&next_head->gc_info.gi_pself, &head->gc_next);
	}
	gc_gen0.gg_objects = ob;

	/* Update counters. */
	++gc_gen0.gg_count;
	--gc_gen0.gg_collect_on;

	/* Check if objects from generation #0 must be collected now */
	should_collect = gc_gen0.gg_collect_on < 0;
	if (should_collect && (flags & DeeGC_TRACK_F_NOCOLLECT)) {
		atomic_write(&gc_mustreap, true);
		should_collect = false;
	}

	/* Release GC lock */
	gc_lock_release(flags);

	/* If necessary, *try* to collect GC objects from generation #0 */
	if (should_collect) {
		if (!gc_trycollect_gen0()) {
			/* If collect failed, let someone else try again later... */
			if (atomic_xch(&gc_mustreap, true))
				gc_lock_reap(flags);
		}
	}
	return ob;
}


PUBLIC ATTR_RETNONNULL NONNULL((1)) DeeObject *DCALL
DeeGC_Untrack(DeeObject *__restrict ob) {
	/* Synchronous object untracking */
	gc_lock_acquire();
	gc_do_untrack_locked(ob);
	gc_lock_release(DeeGC_TRACK_F_NORMAL);
	return ob;
}

/* Try to untrack "ob" synchronously (and re-return "ob"), but if the necessary
 * locks couldn't be acquired immediately, do the untrack asynchronously, followed
 * by everything else that would have normally been done in `DeeObject_Destroy()'
 * @return: ob:   Untrack happened synchronously -- remainder of object destruction must be done by caller
 * @return: NULL: Untrack will happen asynchronously -- caller must not do any further object destruction */
INTERN NONNULL((1)) DeeObject *DCALL
DeeGC_UntrackAsync(DeeObject *__restrict ob) {
	DeeTypeObject *tp;
	DeeObject *next;

	/* Check if we can untrack "ob" synchronously */
	ASSERT(ob->ob_refcnt == 0);
	if likely(gc_lock_tryacquire()) {
		/* If there are pending inserts, they *must* be reaped first.
		 *
		 * If "ob" was a *really* short-lived object, then it may be getting
		 * untracked before it ever was being tracked for real. If that is
		 * what's happening, then in order for the untrack to even work, the
		 * object has to be tracked first. */
		if unlikely(atomic_read(&gc_insert) != NULL)
			gc_reap_insert();

		gc_do_untrack_locked(ob);
		gc_lock_release(DeeGC_TRACK_F_NORMAL);
		return ob;
	}

	/* Must untrack asynchronously.
	 *
	 * For this purpose, we (ab-) use the object's "ob_refcnt"
	 * field as a linked list pointer. However, since this will
	 * also cause the object's reference count to (appear) to
	 * be a very large value, we have to hide the object from
	 * anything that could (still) be able to see it.
	 *
	 * This essentially boils down to clearing its weakref lists! */
	tp = Dee_TYPE(ob);
	do {
		if (tp->tp_weakrefs != 0) {
			struct Dee_weakref_list *wrefs;
			wrefs = (struct Dee_weakref_list *)((byte_t *)ob + tp->tp_weakrefs);
			Dee_weakref_list_clear(wrefs);
		}
	} while ((tp = DeeType_Base(tp)) != NULL);

	/* At this point, nothing exists that can still see the object (other
	 * than the GC itself, which knows to reap pending remove operations
	 * when "gc_remove_modifying != 0") */
	ASSERT(ob->ob_refcnt == 0);
	atomic_inc(&gc_remove_modifying);
	COMPILER_BARRIER();

	/* The `Dee_GC_FLAG_FINALIZED' flag must be set for "gc_remove" to be used. */
	{
		struct Dee_gc_head *head = DeeGC_Head(ob);
		atomic_or(&head->gc_info.gi_flag, Dee_GC_FLAG_FINALIZED);
	}

	/* This is the point where we're free to (ab-) use "ob_refcnt" for other purposes! */
	do {
		next = atomic_read(&gc_remove);
		*gc_remove__p_link(ob) = next;
		COMPILER_WRITE_BARRIER();
		/* If get preempted right here, another thread using the public GC API would be
		 * able to enumerate "ob", notice that its "ob_refcnt" (appears) non-zero, and
		 * then try to incref() it (== bad, because "ob" is actually dead)
		 *
		 * This is what "gc_remove_modifying" is for. It indicates that no object found
		 * within the GC can be incref'd while "gc_remove_modifying != 0", or while the
		 * GC must be reaped:
		 * >> bool tryincref_object_from_gc(DeeObject *ob) {
		 * >>     Dee_refcnt_t refcnt;
		 * >>     do {
		 * >>         refcnt = atomic_read(&ob->ob_refcnt);
		 * >>         if (refcnt == 0)
		 * >>             return false; // Object is obviously dead
		 * >>         if (atomic_read(&gc_mustreap))
		 * >>             return false; // Check so nothing is incref'd if it might be a pending-remove
		 * >>         if (atomic_read(&gc_remove_modifying) != 0)
		 * >>             return false; // Check for our case right here (iow: "ob" might be an about-to-be-pending-remove)
		 * >>     } while (!atomic_cmpxch_weak_or_write(&ob->ob_refcnt, refcnt, refcnt + 1));
		 * >>     return true;
		 * >> } */
	} while (!atomic_cmpxch_weak_or_write(&gc_remove, next, ob));

	/* Indicate that reaping is now necessary */
	atomic_write(&gc_mustreap, true);
	atomic_dec(&gc_remove_modifying);

	/* Attempt an initial reap ourselves */
	gc_lock_reap(DeeGC_TRACK_F_NORMAL);

	/* Indicate to caller that *we're* going to deal with the object's remaining destruction */
	return NULL;
}

PRIVATE ATTR_PURE WUNUSED NONNULL((1, 2)) size_t DCALL
count_gc_objecs_in_simple_chain(DeeObject *first, DeeObject *last) {
	size_t result = 1;
	while (first != last) {
		struct Dee_gc_head *head = DeeGC_Head(first);
		ASSERT(!(head->gc_info.gi_flag & Dee_GC_FLAG_MASK));
		first = head->gc_next;
		++result;
	}
	return result;
}

/* Track all GC objects in range [first,last], all of which have
 * already been linked together using their `struct Dee_gc_head'
 * @param: flags: Set of `DeeGC_TRACK_F_*' */
PUBLIC NONNULL((1, 2)) void DCALL
DeeGC_TrackAll(DeeObject *first, DeeObject *last, unsigned int flags) {
	bool should_collect;
	size_t count;
	struct Dee_gc_head *first_head = DeeGC_Head(first);
	struct Dee_gc_head *last_head  = DeeGC_Head(last);
	ASSERT(!((uintptr_t)&first_head->gc_next & Dee_GC_FLAG_MASK));
	ASSERT(!((uintptr_t)&last_head->gc_next & Dee_GC_FLAG_MASK));
	if (!gc_lock_tryacquire()) {
		/* Setup as pending inserts */
		DeeObject *next;
		do {
			next = atomic_read(&gc_insert);
			last_head->gc_next = next;
			COMPILER_WRITE_BARRIER();
		} while (!atomic_cmpxch(&gc_insert, next, first));
		atomic_write(&gc_mustreap, true);
		gc_lock_reap(flags);
		return;
	}

	/* Insert new GC object into generation #0 */
	count = count_gc_objecs_in_simple_chain(first, last);
	first_head->gc_info.gi_pself = &gc_gen0.gg_objects;
	if ((last_head->gc_next = gc_gen0.gg_objects) != NULL) {
		struct Dee_gc_head *next = DeeGC_Head(gc_gen0.gg_objects);
		ASSERT(next->gc_info.gi_pself == &gc_gen0.gg_objects);
		next->gc_info.gi_pself = &last_head->gc_next;
	}
	gc_gen0.gg_objects = first;

	/* Update counters. */
	gc_gen0.gg_count += count;
	gc_gen0.gg_collect_on -= count;

	/* Check if objects from generation #0 must be collected now */
	should_collect = gc_gen0.gg_collect_on < 0;
	if (should_collect && (flags & DeeGC_TRACK_F_NOCOLLECT)) {
		atomic_write(&gc_mustreap, true);
		should_collect = false;
	}

	/* Release GC lock */
	gc_lock_release(flags);

	/* If necessary, *try* to collect GC objects from generation #0 */
	if (should_collect) {
		if (!gc_trycollect_gen0()) {
			/* If collect failed, let someone else try again later... */
			if (atomic_xch(&gc_mustreap, true))
				gc_lock_reap(flags);
		}
	}
}


/* Call this function (once no more locks are held) after using `DeeGC_TRACK_F_NOCOLLECT' */
PUBLIC void DCALL DeeGC_CollectAsNecessary(void) {
	gc_lock_reap(DeeGC_TRACK_F_NORMAL);
}



/* Have a pre-lock that is acquired first prior to actual GC collect attempts.
 * This lock prevents a situation where all threads try to be the one to collect
 * memory at the same time, which does work, but also results in all threads
 * trying to re-attempt the GC collect once it has already been performed, thus
 * resulting in every thread needing to suspend every other, only to notice that
 * GC collect isn't necessary anymore, and resuming everyone else again. */
#if !defined(CONFIG_NO_THREADS) && 1
PRIVATE Dee_shared_lock_t gc_collect_prelock = Dee_SHARED_LOCK_INIT;
#define gc_collect_prelock_tryacquire() Dee_shared_lock_tryacquire(&gc_collect_prelock)
#define gc_collect_prelock_acquire()    Dee_shared_lock_acquire(&gc_collect_prelock)
#define gc_collect_prelock_release()    Dee_shared_lock_release(&gc_collect_prelock)
#else
#define gc_collect_prelock_tryacquire() 1
#define gc_collect_prelock_acquire()    0
#define gc_collect_prelock_release()    (void)0
#endif

/* @return: true:  Success
 * @return: false: [allow_interrupts_and_errors] An error was thrown
 * @return: false: [!allow_interrupts_and_errors] Failed to acquire locks */
PRIVATE WUNUSED bool DCALL
gc_collect_acquire(bool allow_interrupts_and_errors) {
	DeeObject *pending_remove;
	DeeThreadObject *threads;
	GC_TRACE("%llu: %p: %s(%d) : HERE%s\n", DeeSystem_GetWalltime(), DeeThread_Self(), __FILE__, __LINE__,
	         DeeThread_WasInterrupted(DeeThread_Self()) ? " [INTERRUPT]" : "");
again:
	/* Wait for "gc_lock" and "gc_remove_modifying" to settle.
	 *
	 * During this part, also try to make it so there's a low
	 * probability of the GC needing to be reaped once we've
	 * suspended other threads below! */
	for (;;) {
		while (!gc_lock_available() || atomic_read(&gc_remove_modifying)) {
			if (allow_interrupts_and_errors) {
				if (DeeThread_CheckInterrupt())
					goto err;
			} else {
				DeeThread_CheckInterruptNoInt();
			}
			SCHED_YIELD();
		}
		if (!atomic_read(&gc_mustreap))
			break;
		if (!gc_lock_tryacquire())
			continue;
		gc_lock_reap_and_release(DeeGC_TRACK_F_NOCOLLECT);
		break;
	}

	/* Suspend all other threads (or at least *try* to) */
	if (allow_interrupts_and_errors) {
		threads = DeeThread_SuspendAll();
	} else {
		threads = DeeThread_TrySuspendAll();
	}
	if unlikely(!threads)
		goto err;
	ASSERT(!DeeThread_IsMultiThreaded);

	/* Check if the GC become unsettled while we were suspending threads. */
	if unlikely(atomic_read(&gc_remove_modifying))
		goto resume_threads_and_try_again;
	if unlikely(!gc_lock_tryacquire())
		goto resume_threads_and_try_again;

	/* Success -> locks have been acquired and everything appears fine!
	 *
	 * However: now we have to do a *very* special reap of stuff that
	 *          may still be pending somewhere within the the GC. This
	 *          is needed because our caller requires the GC to be in
	 *          a consistent state in order for a GC collect to even
	 *          be possible!
	 *
	 * This part is somewhat similar to "_gc_lock_reap_and_maybe_unlock",
	 * but needs to be distinct because we've suspended other threads. */
	atomic_write(&gc_mustreap, false);
	pending_remove = atomic_xch(&gc_remove, NULL);

	/* Reap inserts */
	gc_reap_insert();

	/* Deal with pending remove operations (ugh -- this means we'll
	 * have to resume other threads again before we can *actually*
	 * start collecting memory) */
	if unlikely(pending_remove) {
		DeeObject *removeme;
		DeeThread_ResumeAll();

		for (removeme = pending_remove; removeme;
		     removeme = *gc_remove__p_link(removeme))
			gc_do_untrack_locked(removeme);

		/* Release GC lock. */
		_gc_lock_release();

		/* Continue destruction of objects in "pending_remove" */
		while (pending_remove) {
			DeeObject *next = *gc_remove__p_link(pending_remove);
			((GenericObject *)pending_remove)->ob_refcnt = 0;
			COMPILER_WRITE_BARRIER();
			DeeGCObject_FinishDestroyAfterUntrack(pending_remove);
			pending_remove = next;
		}
		goto again;
	}

	ASSERT(!DeeThread_IsMultiThreaded);
	GC_TRACE("%llu: %p: %s(%d) : HERE%s\n", DeeSystem_GetWalltime(), DeeThread_Self(), __FILE__, __LINE__,
	         DeeThread_WasInterrupted(DeeThread_Self()) ? " [INTERRUPT]" : "");
	return true;
resume_threads_and_try_again:
	DeeThread_ResumeAll();
	goto again;
err:
	GC_TRACE("%llu: %p: %s(%d) : HERE%s\n", DeeSystem_GetWalltime(), DeeThread_Self(), __FILE__, __LINE__,
	         DeeThread_WasInterrupted(DeeThread_Self()) ? " [INTERRUPT]" : "");
	return false;
}

PRIVATE void DCALL gc_collect_release(void) {
	_gc_lock_release();
	DeeThread_ResumeAll();
	if (atomic_read(&gc_mustreap))
		gc_lock_reap(DeeGC_TRACK_F_NOCOLLECT);
}





/* Nothing could be collected; everything from "self" appears reachable.
 * Caller may move everything still within this generation into the next.
 * - Calculate a new value for `gg_collect_on' (set to the # of reachable objects)
 * - `GC_GENERATION_COLLECT_OR_UNLOCK__F_MOVE_REACHABLE' was handled */
#define GC_GENERATION_COLLECT_OR_UNLOCK__NOTHING 0

/* Success:
 * - `gc_collect_release()' was called
 * - Calculate a new value for `gg_collect_on' (set to the # of reachable objects;
 *   when 'GC_GENERATION_COLLECT_OR_UNLOCK__F_MOVE_REACHABLE' is set == # of objects moved)
 * - `GC_GENERATION_COLLECT_OR_UNLOCK__F_MOVE_REACHABLE' was handled
 * - `*p_num_collected' was populated */
#define GC_GENERATION_COLLECT_OR_UNLOCK__SUCCESS 1

/* Had to `gc_collect_release()' for temporary (one-time) reasons; try again */
#define GC_GENERATION_COLLECT_OR_UNLOCK__RETRY 2

/* Possible flags for `gc_generation_collect_or_unlock()' */
#define GC_GENERATION_COLLECT_OR_UNLOCK__F_NORMAL         0
#define GC_GENERATION_COLLECT_OR_UNLOCK__F_MOVE_REACHABLE 1 /* Move reachable objects to next generation (caller must ensure that "gg_next != NULL") */

#if 1 /* SIngleTHReaDed_* */
#define sithrd_atomic_read(p)           *(p)
#define sithrd_atomic_write(p, value)   (void)(*(p) = (value))
#define sithrd_atomic_and(p, value)     (void)(*(p) &= (value))
#define sithrd_atomic_or(p, value)      (void)(*(p) |= (value))
#define sithrd_gc_object_set_pself(p_p_self, p_self) \
	(void)(ASSERT(!((uintptr_t)(p_self) & Dee_GC_FLAG_MASK)), \
	       *(p_p_self) = (DeeObject **)(((uintptr_t)(*(p_p_self)) & Dee_GC_FLAG_MASK) | (uintptr_t)(p_self)))
#else
#define sithrd_atomic_read(p)           Dee_atomic_read(p)
#define sithrd_atomic_write(p, value)   Dee_atomic_write(p, value)
#define sithrd_atomic_and(p, value)     Dee_atomic_and(p, value)
#define sithrd_atomic_or(p, value)      Dee_atomic_or(p, value)
#define sithrd_gc_object_set_pself      gc_object_set_pself
#endif




/* Set the "Dee_GC_FLAG_OTHERGEN" flag for objects from "self" */
PRIVATE NONNULL((1)) void DCALL
sithrd_gc_generation_set__GC_FLAG_OTHERGEN(struct gc_generation *__restrict self) {
	DeeObject *iter;
	for (iter = self->gg_objects; iter;) {
		struct Dee_gc_head *head = DeeGC_Head(iter);
		sithrd_atomic_or(&head->gc_info.gi_flag, Dee_GC_FLAG_OTHERGEN);
		iter = head->gc_next;
	}
}

/* Clear the "Dee_GC_FLAG_OTHERGEN" flag for objects from "self" */
PRIVATE NONNULL((1)) void DCALL
sithrd_gc_generation_clear__GC_FLAG_OTHERGEN(struct gc_generation *__restrict self) {
	DeeObject *iter;
	for (iter = self->gg_objects; iter;) {
		struct Dee_gc_head *head = DeeGC_Head(iter);
		sithrd_atomic_and(&head->gc_info.gi_flag, ~Dee_GC_FLAG_OTHERGEN);
		iter = head->gc_next;
	}
}

PRIVATE WUNUSED NONNULL((1)) bool DCALL
DeeType_HasFinalize(DeeTypeObject *__restrict self) {
	/* XXX: Maybe cache this property? */
	do {
		if (self->tp_init.tp_finalize)
			return true;
	} while ((self = DeeType_Base(self)) != NULL);
	return false;
}

PRIVATE WUNUSED NONNULL((1)) bool DCALL
sithrd_DeeObject_HasWeakrefs(DeeObject *__restrict ob) {
	DeeTypeObject *tp = Dee_TYPE(ob);
	struct Dee_weakref_list *list;
	while (!tp->tp_weakrefs && DeeType_Base(tp))
		tp = DeeType_Base(tp);
	if (!tp->tp_weakrefs)
		return false;
	list = (struct Dee_weakref_list *)((uintptr_t)ob + tp->tp_weakrefs);
	return sithrd_atomic_read(&list->wl_nodes) != NULL;
}



PRIVATE NONNULL((1)) void DCALL
DeeObject_KillWeakrefs(DeeObject *__restrict ob) {
	DeeTypeObject *tp = Dee_TYPE(ob);
	while (!tp->tp_weakrefs && DeeType_Base(tp))
		tp = DeeType_Base(tp);
	if (tp->tp_weakrefs) {
		struct Dee_weakref_list *list;
		list = (struct Dee_weakref_list *)((uintptr_t)ob + tp->tp_weakrefs);
		Dee_weakref_list_transfer_to_dummy(list);
	}
}

PRIVATE NONNULL((1)) void DCALL
DeeObject_GCClear(DeeObject *__restrict self) {
	DeeTypeObject *tp_self = Dee_TYPE(self);
	do {
		if (tp_self->tp_gc && tp_self->tp_gc->tp_clear) {
			if (tp_self->tp_features & TF_TPVISIT) {
				typedef void (DCALL *tp_tclear_t)(DeeTypeObject *tp_self, DeeObject *self);
				(*(tp_tclear_t)tp_self->tp_gc->tp_clear)(tp_self, self);
			} else {
				(*tp_self->tp_gc->tp_clear)(self);
			}
		}
	} while ((tp_self = DeeType_Base(tp_self)) != NULL);
}



PRIVATE NONNULL((1)) void DCALL
gc_visit__decref__cb(DeeObject *__restrict self, void *UNUSED(arg)) {
	ASSERT(gc_remove_modifying == 0);
	if (DeeType_IsGC(Dee_TYPE(self))) {
		struct Dee_gc_head *head = DeeGC_Head(self);
		if (!(sithrd_atomic_read(&head->gc_info.gi_flag) & Dee_GC_FLAG_OTHERGEN)) {
			ASSERT(self->ob_refcnt != 0);
			--self->ob_refcnt;
		}
	} else {
		ASSERT(self->ob_refcnt != 0);
		--self->ob_refcnt;
		if (self->ob_refcnt == 0) {
			/* All references of non-gc object account-for
			 * -> cyclic references of this object should also go away
			 *
			 * Recurse on non-GC objects (e.g. "Tuple") */
			DeeObject_Visit(self, &gc_visit__decref__cb, NULL);
		}
	}
}

PRIVATE NONNULL((1)) void DCALL
gc_visit__incref__with_weakref_detect__cb(DeeObject *__restrict self, void *arg) {
	ASSERT(gc_remove_modifying == 0);
	if (DeeType_IsGC(Dee_TYPE(self))) {
		struct Dee_gc_head *head = DeeGC_Head(self);
		if (!(sithrd_atomic_read(&head->gc_info.gi_flag) & Dee_GC_FLAG_OTHERGEN))
			++self->ob_refcnt;
	} else {
		if (self->ob_refcnt == 0) {
			/* This non-gc object will also have to be destroyed.
			 * If it has weak references, then  */
			bool *p_must_kill_nested_weakrefs = (bool *)arg;
			if (!*p_must_kill_nested_weakrefs)
				*p_must_kill_nested_weakrefs = sithrd_DeeObject_HasWeakrefs(self);
			DeeObject_Visit(self, &gc_visit__incref__with_weakref_detect__cb, arg);
		}
		++self->ob_refcnt;
	}
}

PRIVATE NONNULL((1)) void DCALL
gc_visit__incref__with_weakref_kill__cb(DeeObject *__restrict self, void *UNUSED(arg)) {
	ASSERT(gc_remove_modifying == 0);
	if (DeeType_IsGC(Dee_TYPE(self))) {
		struct Dee_gc_head *head = DeeGC_Head(self);
		if (!(sithrd_atomic_read(&head->gc_info.gi_flag) & Dee_GC_FLAG_OTHERGEN))
			++self->ob_refcnt;
	} else {
		if (self->ob_refcnt == 0) {
			/* Kill weak references of this object (if it has any) */
			DeeObject_KillWeakrefs(self);
			DeeObject_Visit(self, &gc_visit__incref__with_weakref_kill__cb, NULL);
		}
		++self->ob_refcnt;
	}
}

PRIVATE NONNULL((1)) void DCALL
gc_visit__decref_all__cb(DeeObject *__restrict self, void *UNUSED(arg)) {
	ASSERT(gc_remove_modifying == 0);
	ASSERT(self->ob_refcnt != 0);
	--self->ob_refcnt;
	if (!DeeType_IsGC(Dee_TYPE(self)) && self->ob_refcnt == 0) {
		/* All references of non-gc object account-for
		 * -> cyclic references of this object should also go away
		 *
		 * Recurse on non-GC objects (e.g. "Tuple") */
		DeeObject_Visit(self, &gc_visit__decref_all__cb, NULL);
	}
}

PRIVATE NONNULL((1)) void DCALL
gc_visit__incref_all__with_weakref_detect__cb(DeeObject *__restrict self, void *arg) {
	ASSERT(gc_remove_modifying == 0);
	if (!DeeType_IsGC(Dee_TYPE(self)) && self->ob_refcnt == 0) {
		/* This non-gc object will also have to be destroyed.
		 * If it has weak references, then  */
		bool *p_must_kill_nested_weakrefs = (bool *)arg;
		if (!*p_must_kill_nested_weakrefs)
			*p_must_kill_nested_weakrefs = sithrd_DeeObject_HasWeakrefs(self);
		DeeObject_Visit(self, &gc_visit__incref_all__with_weakref_detect__cb, arg);
	}
	++self->ob_refcnt;
}

PRIVATE NONNULL((1)) void DCALL
gc_visit__incref_all__with_weakref_kill__cb(DeeObject *__restrict self, void *UNUSED(arg)) {
	ASSERT(gc_remove_modifying == 0);
	if (!DeeType_IsGC(Dee_TYPE(self)) && self->ob_refcnt == 0) {
		/* Kill weak references of this object (if it has any) */
		DeeObject_KillWeakrefs(self);
		DeeObject_Visit(self, &gc_visit__incref_all__with_weakref_kill__cb, NULL);
	}
	++self->ob_refcnt;
}

PRIVATE NONNULL((1)) void DCALL
gc_visit__mark_internally_reachable__cb(DeeObject *__restrict self, void *UNUSED(arg)) {
	ASSERT(gc_remove_modifying == 0);
	if (self->ob_refcnt != 0)
		return; /* Already reachable... */
	if (DeeType_IsGC(Dee_TYPE(self))) {
		struct Dee_gc_head *head = DeeGC_Head(self);
		if (!((uintptr_t)head->gc_next & 1))
			return; /* Already marked as "actually-is-reachable" */
		/* Found a GC object is actually reachable in a constellation like this:
		 * >> EXTERNALLY_REFERENCED_GC_OBJECT --> NOT_EXTERNALLY_REFERENCED_GC_OBJECT
		 *
		 * Here, the initial decref scan will have correctly identified that
		 * "NOT_EXTERNALLY_REFERENCED_GC_OBJECT" is only referenced by other
		 * GC objects.
		 *
		 * Now we come into play by marking "NOT_EXTERNALLY_REFERENCED_GC_OBJECT"
		 * as actually **being** reachable, since it's referenced by another GC
		 * object that was previously found to be *externally* reachable. */
		head->gc_next = (DeeObject *)((uintptr_t)head->gc_next & ~1);
	}
	DeeObject_Visit(self, &gc_visit__mark_internally_reachable__cb, NULL);
}

PRIVATE NONNULL((1)) void DCALL
DeeObject_DoInvokeFinalize(DeeObject *__restrict self) {
	DeeTypeObject *tp_self = Dee_TYPE(self);
	do {
		if (tp_self->tp_init.tp_finalize != NULL)
			(*tp_self->tp_init.tp_finalize)(tp_self, self);
	} while ((tp_self = DeeType_Base(tp_self)) != NULL);
}


/* Insert objects starting at "first" into "self".
 * Assumes that the "Dee_GC_FLAG_GENN" flag is set for all objects. */
PRIVATE NONNULL((1, 2)) void DCALL
sithrd_gc_generation_insert_temp_list(struct gc_generation *__restrict self,
                                               DeeObject *first) {
	size_t count = 1;
	DeeObject *last;
	struct Dee_gc_head *first_head;
	struct Dee_gc_head *last_head;
	for (last = first;;) {
		struct Dee_gc_head *head = DeeGC_Head(last);
		ASSERT(sithrd_atomic_read(&head->gc_info.gi_flag) & Dee_GC_FLAG_GENN);
		if (self == &gc_gen0)
			sithrd_atomic_and(&head->gc_info.gi_flag, ~Dee_GC_FLAG_GENN);
		if (!head->gc_next)
			break;
		last = head->gc_next;
		++count;
	}

	/* Insert all remaining objects into "self" */
	first_head = DeeGC_Head(first);
	last_head  = DeeGC_Head(last);
	sithrd_gc_object_set_pself(&first_head->gc_info.gi_pself, &self->gg_objects); /* This **must** be atomic! */
	if ((last_head->gc_next = self->gg_objects) != NULL) {
		struct Dee_gc_head *next_head = DeeGC_Head(self->gg_objects);
		sithrd_gc_object_set_pself(&next_head->gc_info.gi_pself, &last_head->gc_next); /* This **must** be atomic! */
	}
	self->gg_objects = first;
	self->gg_count += count;
}


/* Heart-piece of GC collect
 * @return: * : One of `GC_GENERATION_COLLECT_OR_UNLOCK__*' */
PRIVATE ATTR_NOINLINE WUNUSED NONNULL((1, 2)) unsigned int DCALL
gc_generation_collect_or_unlock(struct gc_generation *__restrict gen,
                                size_t *__restrict p_num_collected,
                                unsigned int flags);
PRIVATE ATTR_NOINLINE WUNUSED NONNULL((1)) unsigned int DCALL
gc_collectall_collect_or_unlock(size_t *__restrict p_num_collected);

#ifndef __INTELLISENSE__
DECL_END
#define DEFINE_gc_generation_collect_or_unlock
#include "gc-collect.c.inl"
#define DEFINE_gc_collectall_collect_or_unlock
#include "gc-collect.c.inl"
DECL_BEGIN
#endif /* !__INTELLISENSE__ */



#ifndef CONFIG_NO_THREADS
PRIVATE void DCALL
gc_generation_try_precollect(struct gc_generation *__restrict self) {
	/* TODO: GC pre-collect (see "gc.h" for description) */
	/* TODO: Since this function may be rather slow, change "gc_lock" to a "Dee_shared_lock_t" */
	(void)self;
}
#endif /* !CONFIG_NO_THREADS */


/* Collect (in reverse order) objects from all generations
 * where "gg_collect_on < 0". If a collectible generation
 * is found, at least its "gg_collect_on" is always reset! */
PRIVATE WUNUSED NONNULL((1)) unsigned int DCALL
gc_collect_threshold_generations_r(struct gc_generation *__restrict self) {
	size_t count;
	unsigned int status, flags;
	if (self->gg_next) {
		status = gc_collect_threshold_generations_r(self->gg_next);
		if (status != GC_GENERATION_COLLECT_OR_UNLOCK__NOTHING)
			return status;
	}
	if (self->gg_collect_on >= 0)
		return GC_GENERATION_COLLECT_OR_UNLOCK__NOTHING;
	flags = GC_GENERATION_COLLECT_OR_UNLOCK__F_NORMAL;
	if (self->gg_next) {
		/* If there is a next generation, move reachable objects into it */
		flags |= GC_GENERATION_COLLECT_OR_UNLOCK__F_MOVE_REACHABLE;
	}
	return gc_generation_collect_or_unlock(self, &count, flags);
}

/* Called to (try to) collect GC objects after "gg_collect_on" of some
 * GC generation was reached. This function is must be called without
 * the calling thread holding any (internal) locks, include locks
 * related to GC (such as `gc_lock')
 *
 * @return: true:  Success
 * @return: false: Failure (collect failed; caller must set "gc_mustreap"
 *                 to true, but not try again right now...) */
PRIVATE bool DCALL gc_trycollect_after_collect_on_reached(void) {
	bool has_collectable_generations;
	unsigned int status;
	struct gc_generation *iter;
	if (!gc_collect_prelock_tryacquire())
		return false; /* Some other thread is already doing GC stuff... */
again:
	/* Check if any generation *actually* wants to be collected */
	has_collectable_generations = false;
	gc_lock_acquire();
	iter = &gc_gen0;
	do {
		if (iter->gg_collect_on < 0) {
#ifndef CONFIG_NO_THREADS
			if (DeeThread_IsMultiThreaded) {
				gc_generation_try_precollect(iter);
				if (iter->gg_collect_on >= 0)
					goto continue_with_next_generation;
			}
#endif /* !CONFIG_NO_THREADS */
			has_collectable_generations = true;
			break;
		}
#ifndef CONFIG_NO_THREADS
continue_with_next_generation:;
#endif /* !CONFIG_NO_THREADS */
	} while ((iter = iter->gg_next) != NULL);
	_gc_lock_release();

	if (!has_collectable_generations) {
		gc_collect_prelock_release();
		return true; /* Nothing left to do! */
	}
	if (!gc_collect_acquire(false)) {
		gc_collect_prelock_release();
		return false;
	}

	/* Collect memory in GC generations */
	status = gc_collect_threshold_generations_r(&gc_gen0);
	GC_TRACE("%llu: %p: %s(%d) : COLLECTED: %u\n", DeeSystem_GetWalltime(), DeeThread_Self(), __FILE__, __LINE__, status);
	switch (status) {
	case GC_GENERATION_COLLECT_OR_UNLOCK__NOTHING:
		/* Nothing could be collected (must objects were moved as they should) */
		gc_collect_release();
		break;
	case GC_GENERATION_COLLECT_OR_UNLOCK__SUCCESS:
		break;
	case GC_GENERATION_COLLECT_OR_UNLOCK__RETRY:
		break;
	default: __builtin_unreachable();
	}
	/* Check for more collectible generations */
	goto again;
}

/* Try to collect objects from GC generation #0
 * @return: true:  Success
 * @return: false: Collect failed (caller must set "gc_mustreap"
 *                 to true, but not try again right now...) */
PRIVATE ATTR_NOINLINE WUNUSED bool DCALL gc_trycollect_gen0(void) {
	bool should_collect;
	size_t count;
	unsigned int status;
	struct gc_generation *iter;
	if (!gc_collect_prelock_tryacquire())
		return false;
again:
	gc_lock_acquire();
	should_collect = gc_gen0.gg_collect_on < 0;
#ifndef CONFIG_NO_THREADS
	if (should_collect && DeeThread_IsMultiThreaded) {
		gc_generation_try_precollect(&gc_gen0);
		should_collect = gc_gen0.gg_collect_on < 0;
	}
#endif /* !CONFIG_NO_THREADS */
	_gc_lock_release();
	if (!should_collect) {
		gc_collect_prelock_release();
		return true;
	}
	if (!gc_collect_acquire(false)) {
		gc_collect_prelock_release();
		return false;
	}
	ASSERT(!DeeThread_IsMultiThreaded);

	/* Collect generation #0 */
	iter = &gc_gen0;
continue_with_iter:
	if (iter->gg_collect_on < 0) {
		unsigned int flags;
		flags = GC_GENERATION_COLLECT_OR_UNLOCK__F_NORMAL;
		if (iter->gg_next)
			flags |= GC_GENERATION_COLLECT_OR_UNLOCK__F_MOVE_REACHABLE;
		ASSERT(!DeeThread_IsMultiThreaded);
		status = gc_generation_collect_or_unlock(iter, &count, flags);
		GC_TRACE("%llu: %p: %s(%d) : COLLECTED: %u: %lu\n", DeeSystem_GetWalltime(), DeeThread_Self(), __FILE__, __LINE__, status, (unsigned long)count);
	} else {
		/* All done! */
		gc_collect_release();
		gc_collect_prelock_release();
		return true;
	}
	switch (status) {
	case GC_GENERATION_COLLECT_OR_UNLOCK__NOTHING:
		/* Nothing could be collected (must objects were moved as they should) */
		if (iter->gg_next) {
			iter = iter->gg_next;
			goto continue_with_iter;
		}
		gc_collect_release();
		break;
	case GC_GENERATION_COLLECT_OR_UNLOCK__SUCCESS:
		break;
	case GC_GENERATION_COLLECT_OR_UNLOCK__RETRY:
		break;
	default: __builtin_unreachable();
	}
	goto again;
}


/* Try to collect `max_objects' GC-objects (though more than that
 * may be collected), returning the actual amount collected.
 *
 * This function really only has 2 valid use-cases:
 * - DeeGC_Collect(1)          -- Try to collect *sometime* from some generation
 * - DeeGC_Collect((size_t)-1) -- Collect *all* generations
 *
 * @return: * :         The # of objects collected
 * @return: (size_t)-1: An error was thrown. */
PUBLIC WUNUSED size_t DCALL DeeGC_Collect(size_t max_objects) {
	size_t count, result = 0;
	struct gc_generation *iter;
	unsigned int status;
	Dee_DPRINT("[gc.collect] Starting collect\n");
	if (gc_collect_prelock_acquire())
		goto err;
again:
	if (!gc_collect_acquire(true))
		goto err_prelock;

	/* Fast-pass: if we're supposed to collect everything anyways: just start with that! */
	if (max_objects == (size_t)-1)
		goto attempt_collect_all;

	/* Collect garbage in every generation until we hit "max_objects" or scanned them all */
	iter = &gc_gen0;

	/* NOTE: This won't work properly to identify unreachable objects
	 *       that are only referenced by other objects from other generations.
	 *
	 * Instead, there needs to be a secondary gc collection function
	 * that scans and collects objects across **all** generations. */
continue_with_iter:
	status = gc_generation_collect_or_unlock(iter, &count, GC_GENERATION_COLLECT_OR_UNLOCK__F_NORMAL);
	switch (status) {
	case GC_GENERATION_COLLECT_OR_UNLOCK__NOTHING:
		/* Empty generation -> move on to next */
		if ((iter = iter->gg_next) != NULL)
			goto continue_with_iter;
		if (result != 0) {
			gc_collect_release();
		} else {
			/* Attempt a special collect across *all* generations */
attempt_collect_all:
			status = gc_collectall_collect_or_unlock(&count);
			switch (status) {
			case GC_GENERATION_COLLECT_OR_UNLOCK__NOTHING:
				gc_collect_release();
				break;
			case GC_GENERATION_COLLECT_OR_UNLOCK__SUCCESS:
				/* Track collected objects and continue if caller still wants more */
				if (OVERFLOW_UADD(result, count, &result))
					result = (size_t)-1;
				break;
			case GC_GENERATION_COLLECT_OR_UNLOCK__RETRY:
				goto again;
			default: __builtin_unreachable();
			}
		}
		break;
	case GC_GENERATION_COLLECT_OR_UNLOCK__SUCCESS:
		/* Track collected objects and continue if caller still wants more */
		if (OVERFLOW_UADD(result, count, &result))
			result = (size_t)-1;
		if unlikely(count == 0) {
			if (!gc_collect_acquire(true))
				goto err_prelock;
			goto attempt_collect_all;
		}
		if (result < max_objects)
			goto again;
		break;
	case GC_GENERATION_COLLECT_OR_UNLOCK__RETRY:
		goto again;
	default: __builtin_unreachable();
	}
	Dee_DPRINTF("[gc.collect] Done. Collected %" PRFuSIZ " objects\n", result);
	if unlikely(result == (size_t)-1)
		result = (size_t)-2;
	gc_collect_prelock_release();
	return result;
err_prelock:
	gc_collect_prelock_release();
err:
	Dee_DPRINT("[gc.collect] Error\n");
	return (size_t)-1;
}


/* Return `true' if any GC objects with a non-zero reference
 * counter is being tracked.
 * NOTE: In addition, this function does not return `true' when
 *       all that's left are dex objects (which are destroyed
 *       at a later point during deemon shutdown, than the point
 *       when this function is called to determine if the GC must
 *       continue to run) */
INTERN bool DCALL DeeGC_IsEmptyWithoutDex(void) {
	struct gc_generation *gen;
	gc_lock_acquire_and_reap(DeeGC_TRACK_F_NORMAL);
	for (gen = &gc_gen0; gen; gen = gen->gg_next) {
		DeeObject *iter;
		for (iter = gen->gg_objects; iter;
		     iter = DeeGC_Head(iter)->gc_next) {
			if (!iter->ob_refcnt)
				continue;
#ifndef CONFIG_NO_DEX
#ifdef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
			if (Dee_TYPE(iter) == &DeeModuleDex_Type)
				continue;
#else /* CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
			if (DeeDex_Check(iter))
				continue;
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
#endif /* !CONFIG_NO_DEX */
			goto no;
		}
	}
	gc_lock_release(DeeGC_TRACK_F_NORMAL);
	return true;
no:
	gc_lock_release(DeeGC_TRACK_F_NORMAL);
	return false;
}


/* Try to acquire a reference to "ob", which was found within the GC.
 * @return: true:  Success -- you've gained a reference to "ob"
 *                 CAUTION: This reference may only be dropped
 * @return: false: Failure -- object has been destroyed to the point
 *                 where no reference can be claimed anymore. */
PRIVATE WUNUSED NONNULL((1)) bool DCALL
gc_tryincref(DeeObject *__restrict ob) {
	Dee_refcnt_t refcnt;
	struct Dee_gc_head *head = DeeGC_Head(ob);
again:
	do {
		refcnt = atomic_read(&ob->ob_refcnt);
		if (refcnt == 0)
			goto nope; /* Object is obviously dead */
		if (atomic_read(&gc_remove_modifying) != 0) {
			/* Object may be in the process of being async-removed */
			SCHED_YIELD();
			goto again;
		}
		if (atomic_read(&head->gc_info.gi_flag) & Dee_GC_FLAG_FINALIZED) {
			/* Object may have been enqueue within "gc_remove",
			 * in which case it must no longer be incref'd. In
			 * this case the only thing we can do is manually
			 * search through "gc_remove" */
			DeeObject *remove = atomic_read(&gc_remove);
			for (; remove; remove = *gc_remove__p_link(remove)) {
				if (remove == ob)
					goto nope;
			}
		}
		/* Try to increment the reference counter by 1. */
	} while (!atomic_cmpxch_or_write(&ob->ob_refcnt, refcnt, refcnt + 1));
	return true;
nope:
	return false;
}



/* GC object alloc/free. */
LOCAL void *gc_initob(void *ptr) {
	if likely(ptr) {
		DBG_memset(ptr, 0xcc, Dee_GC_HEAD_SIZE);
		ptr = DeeGC_Object((struct Dee_gc_head *)ptr);
	}
	return ptr;
}

/* GC object alloc/free.
 * Don't you think these functions allocate some magical memory
 * that can somehow track what objects it references. - No!
 * All these do is allocate a block of memory of `n_bytes' that
 * includes some storage at negative offsets to hold a `struct Dee_gc_head',
 * as is required for objects that should later be tracked by the GC. */
PUBLIC ATTR_MALLOC WUNUSED void *(DCALL DeeGCObject_Malloc)(size_t n_bytes) {
	size_t whole_size;
	if unlikely(OVERFLOW_UADD(Dee_GC_HEAD_SIZE, n_bytes, &whole_size))
		whole_size = (size_t)-1;
	return gc_initob((DeeObject_Malloc)(whole_size));
}

PUBLIC ATTR_MALLOC WUNUSED void *(DCALL DeeGCObject_Calloc)(size_t n_bytes) {
	size_t whole_size;
	if unlikely(OVERFLOW_UADD(Dee_GC_HEAD_SIZE, n_bytes, &whole_size))
		whole_size = (size_t)-1;
	return gc_initob((DeeObject_Calloc)(whole_size));
}

PUBLIC WUNUSED void *(DCALL DeeGCObject_Realloc)(void *p, size_t n_bytes) {
	size_t whole_size;
	if unlikely(OVERFLOW_UADD(Dee_GC_HEAD_SIZE, n_bytes, &whole_size))
		whole_size = (size_t)-1;
	if (p) {
#ifdef GCHEAD_ISTRACKED
		ASSERTF(!GCHEAD_ISTRACKED(DeeGC_Head((DeeObject *)p)),
		        "Object at %p was still being tracked", p);
#endif /* GCHEAD_ISTRACKED */
		p = (DeeObject_Realloc)(DeeGC_Head((DeeObject *)p), whole_size);
	} else {
		p = (DeeObject_Malloc)(whole_size);
	}
	return gc_initob(p);
}

PUBLIC ATTR_MALLOC WUNUSED void *
(DCALL DeeGCObject_TryMalloc)(size_t n_bytes) {
	size_t whole_size;
	if unlikely(OVERFLOW_UADD(Dee_GC_HEAD_SIZE, n_bytes, &whole_size))
		whole_size = (size_t)-1;
	return gc_initob((DeeObject_TryMalloc)(whole_size));
}

PUBLIC ATTR_MALLOC WUNUSED void *
(DCALL DeeGCObject_TryCalloc)(size_t n_bytes) {
	size_t whole_size;
	if unlikely(OVERFLOW_UADD(Dee_GC_HEAD_SIZE, n_bytes, &whole_size))
		whole_size = (size_t)-1;
	return gc_initob((DeeObject_TryCalloc)(whole_size));
}

PUBLIC WUNUSED void *
(DCALL DeeGCObject_TryRealloc)(void *p, size_t n_bytes) {
	size_t whole_size;
	if unlikely(OVERFLOW_UADD(Dee_GC_HEAD_SIZE, n_bytes, &whole_size))
		whole_size = (size_t)-1;
	if (p) {
#ifdef GCHEAD_ISTRACKED
		ASSERTF(!GCHEAD_ISTRACKED(DeeGC_Head((DeeObject *)p)),
		        "Object at %p was still being tracked", p);
#endif /* GCHEAD_ISTRACKED */
		p = (DeeObject_TryRealloc)(DeeGC_Head((DeeObject *)p), whole_size);
	} else {
		p = (DeeObject_TryMalloc)(whole_size);
	}
	return gc_initob(p);
}

#ifdef GCHEAD_ISTRACKED
#define ASSERT_UNTRACKED(p)                                \
	ASSERTF(!GCHEAD_ISTRACKED(DeeGC_Head((DeeObject *)p)), \
	        "Object at %p was still being tracked", p)
#else /* GCHEAD_ISTRACKED */
#define ASSERT_UNTRACKED(p) (void)0
#endif /* !GCHEAD_ISTRACKED */

PUBLIC void
(DCALL DeeGCObject_Free)(void *p) {
	if (p) {
		ASSERT_UNTRACKED(p);
		(DeeObject_Free)(DeeGC_Head((DeeObject *)p));
	}
}

PUBLIC ATTR_MALLOC WUNUSED void *
(DCALL DeeDbgGCObject_Malloc)(size_t n_bytes, char const *file, int line) {
	size_t whole_size;
	if unlikely(OVERFLOW_UADD(Dee_GC_HEAD_SIZE, n_bytes, &whole_size))
		whole_size = (size_t)-1;
	return gc_initob((DeeDbgObject_Malloc)(whole_size, file, line));
}

PUBLIC ATTR_MALLOC WUNUSED void *
(DCALL DeeDbgGCObject_Calloc)(size_t n_bytes, char const *file, int line) {
	size_t whole_size;
	if unlikely(OVERFLOW_UADD(Dee_GC_HEAD_SIZE, n_bytes, &whole_size))
		whole_size = (size_t)-1;
	return gc_initob((DeeDbgObject_Calloc)(whole_size, file, line));
}

PUBLIC WUNUSED void *
(DCALL DeeDbgGCObject_Realloc)(void *p, size_t n_bytes, char const *file, int line) {
	size_t whole_size;
	if unlikely(OVERFLOW_UADD(Dee_GC_HEAD_SIZE, n_bytes, &whole_size))
		whole_size = (size_t)-1;
	if (p) {
#if defined(GCHEAD_ISTRACKED) && !defined(NDEBUG)
		if (GCHEAD_ISTRACKED(DeeGC_Head((DeeObject *)p))) {
			_DeeAssert_Failf("!GCHEAD_ISTRACKED(DeeGC_Head((DeeObject *)p))", file, line,
			                 "Object at %p was still being tracked", p);
		}
#endif /* GCHEAD_ISTRACKED && !NDEBUG */
		p = (DeeDbgObject_Realloc)(DeeGC_Head((DeeObject *)p), whole_size, file, line);
	} else {
		p = (DeeDbgObject_Malloc)(whole_size, file, line);
	}
	return gc_initob(p);
}

PUBLIC ATTR_MALLOC WUNUSED void *
(DCALL DeeDbgGCObject_TryMalloc)(size_t n_bytes, char const *file, int line) {
	size_t whole_size;
	if unlikely(OVERFLOW_UADD(Dee_GC_HEAD_SIZE, n_bytes, &whole_size))
		whole_size = (size_t)-1;
	return gc_initob((DeeDbgObject_TryMalloc)(whole_size, file, line));
}

PUBLIC ATTR_MALLOC WUNUSED void *
(DCALL DeeDbgGCObject_TryCalloc)(size_t n_bytes, char const *file, int line) {
	size_t whole_size;
	if unlikely(OVERFLOW_UADD(Dee_GC_HEAD_SIZE, n_bytes, &whole_size))
		whole_size = (size_t)-1;
	return gc_initob((DeeDbgObject_TryCalloc)(whole_size, file, line));
}

PUBLIC WUNUSED void *
(DCALL DeeDbgGCObject_TryRealloc)(void *p, size_t n_bytes, char const *file, int line) {
	size_t whole_size;
	if unlikely(OVERFLOW_UADD(Dee_GC_HEAD_SIZE, n_bytes, &whole_size))
		whole_size = (size_t)-1;
	if (p) {
#if defined(GCHEAD_ISTRACKED) && !defined(NDEBUG)
		if (GCHEAD_ISTRACKED(DeeGC_Head((DeeObject *)p))) {
			_DeeAssert_Failf("!GCHEAD_ISTRACKED(DeeGC_Head((DeeObject *)p))", file, line,
			                 "Object at %p was still being tracked", p);
		}
#endif /* GCHEAD_ISTRACKED && !NDEBUG */
		p = (DeeDbgObject_TryRealloc)(DeeGC_Head((DeeObject *)p), whole_size, file, line);
	} else {
		p = (DeeDbgObject_TryMalloc)(whole_size, file, line);
	}
	return gc_initob(p);
}

PUBLIC void
(DCALL DeeDbgGCObject_Free)(void *p, char const *file, int line) {
	if (p) {
		ASSERT_UNTRACKED(p);
		(DeeDbgObject_Free)(DeeGC_Head((DeeObject *)p), file, line);
	}
}

PUBLIC void *
(DCALL DeeDbgGCObject_UntrackAlloc)(void *p, char const *file, int line) {
	if (p) {
		ASSERT_UNTRACKED(p);
		(DeeDbgObject_UntrackAlloc)(DeeGC_Head((DeeObject *)p), file, line);
	}
	return p;
}


#ifndef CONFIG_EXPERIMENTAL_REWORKED_SLAB_ALLOCATOR
#ifndef CONFIG_NO_OBJECT_SLABS
#define DEFINE_GC_SLAB_FUNCTIONS(index, size)                                                                               \
	PUBLIC ATTR_MALLOC WUNUSED void *DCALL                                                                                  \
	DeeGCObject_SlabMalloc##size(void) {                                                                                    \
		return gc_initob(DeeSlab_Invoke(DeeObject_SlabMalloc, Dee_GC_HEAD_SIZE + size * sizeof(void *), (),                 \
		                                (DeeObject_Malloc)(Dee_GC_HEAD_SIZE + size * sizeof(void *))));                     \
	}                                                                                                                       \
	PUBLIC ATTR_MALLOC WUNUSED void *DCALL                                                                                  \
	DeeGCObject_SlabCalloc##size(void) {                                                                                    \
		return gc_initob(DeeSlab_Invoke(DeeObject_SlabCalloc, Dee_GC_HEAD_SIZE + size * sizeof(void *), (),                 \
		                                (DeeObject_Calloc)(Dee_GC_HEAD_SIZE + size * sizeof(void *))));                     \
	}                                                                                                                       \
	PUBLIC ATTR_MALLOC WUNUSED void *DCALL                                                                                  \
	DeeGCObject_SlabTryMalloc##size(void) {                                                                                 \
		return gc_initob(DeeSlab_Invoke(DeeObject_SlabTryMalloc, Dee_GC_HEAD_SIZE + size * sizeof(void *), (),              \
		                                (DeeObject_TryMalloc)(Dee_GC_HEAD_SIZE + size * sizeof(void *))));                  \
	}                                                                                                                       \
	PUBLIC ATTR_MALLOC WUNUSED void *DCALL                                                                                  \
	DeeGCObject_SlabTryCalloc##size(void) {                                                                                 \
		return gc_initob(DeeSlab_Invoke(DeeObject_SlabTryCalloc, Dee_GC_HEAD_SIZE + size * sizeof(void *), (),              \
		                                (DeeObject_TryCalloc)(Dee_GC_HEAD_SIZE + size * sizeof(void *))));                  \
	}                                                                                                                       \
	PUBLIC NONNULL((1)) void DCALL                                                                                          \
	DeeGCObject_SlabFree##size(void *__restrict ptr) {                                                                      \
		ASSERT_UNTRACKED(ptr);                                                                                              \
		DeeSlab_Invoke(DeeObject_SlabFree, Dee_GC_HEAD_SIZE + size * sizeof(void *),                                        \
		               (DeeGC_Head((DeeObject *)ptr)),                                                                      \
		               (DeeObject_Free)(DeeGC_Head((DeeObject *)ptr)));                                                     \
	}                                                                                                                       \
	PUBLIC ATTR_MALLOC WUNUSED void *DCALL                                                                                  \
	DeeDbgGCObject_SlabMalloc##size(char const *file, int line) {                                                           \
		(void)file;                                                                                                         \
		(void)line;                                                                                                         \
		return gc_initob(DeeSlab_Invoke(DeeDbgObject_SlabMalloc, Dee_GC_HEAD_SIZE + size * sizeof(void *), (file, line),    \
		                                (DeeDbgObject_Malloc)(Dee_GC_HEAD_SIZE + size * sizeof(void *), file, line)));      \
	}                                                                                                                       \
	PUBLIC ATTR_MALLOC WUNUSED void *DCALL                                                                                  \
	DeeDbgGCObject_SlabCalloc##size(char const *file, int line) {                                                           \
		(void)file;                                                                                                         \
		(void)line;                                                                                                         \
		return gc_initob(DeeSlab_Invoke(DeeDbgObject_SlabCalloc, Dee_GC_HEAD_SIZE + size * sizeof(void *), (file, line),    \
		                                (DeeDbgObject_Calloc)(Dee_GC_HEAD_SIZE + size * sizeof(void *), file, line)));      \
	}                                                                                                                       \
	PUBLIC ATTR_MALLOC WUNUSED void *DCALL                                                                                  \
	DeeDbgGCObject_SlabTryMalloc##size(char const *file, int line) {                                                        \
		(void)file;                                                                                                         \
		(void)line;                                                                                                         \
		return gc_initob(DeeSlab_Invoke(DeeDbgObject_SlabTryMalloc, Dee_GC_HEAD_SIZE + size * sizeof(void *), (file, line), \
		                                (DeeDbgObject_TryMalloc)(Dee_GC_HEAD_SIZE + size * sizeof(void *), file, line)));   \
	}                                                                                                                       \
	PUBLIC ATTR_MALLOC WUNUSED void *DCALL                                                                                  \
	DeeDbgGCObject_SlabTryCalloc##size(char const *file, int line) {                                                        \
		(void)file;                                                                                                         \
		(void)line;                                                                                                         \
		return gc_initob(DeeSlab_Invoke(DeeDbgObject_SlabTryCalloc, Dee_GC_HEAD_SIZE + size * sizeof(void *), (file, line), \
		                                (DeeDbgObject_TryCalloc)(Dee_GC_HEAD_SIZE + size * sizeof(void *), file, line)));   \
	}                                                                                                                       \
	PUBLIC NONNULL((1)) void DCALL                                                                                          \
	DeeDbgGCObject_SlabFree##size(void *__restrict ptr, char const *file, int line) {                                       \
		ASSERT_UNTRACKED(ptr);                                                                                              \
		(void)file;                                                                                                         \
		(void)line;                                                                                                         \
		DeeSlab_Invoke(DeeDbgObject_SlabFree, Dee_GC_HEAD_SIZE + size * sizeof(void *),                                     \
		               (DeeGC_Head((DeeObject *)ptr), file, line),                                                          \
		               (DeeDbgObject_Free)(DeeGC_Head((DeeObject *)ptr), file, line));                                      \
	}
DeeSlab_ENUMERATE(DEFINE_GC_SLAB_FUNCTIONS)
#undef DEFINE_GC_SLAB_FUNCTIONS
#endif /* !CONFIG_NO_OBJECT_SLABS */
#endif /* !CONFIG_EXPERIMENTAL_REWORKED_SLAB_ALLOCATOR */


#ifndef NDEBUG
#ifdef CONFIG_TRACE_REFCHANGES
INTDEF NONNULL((1)) void DCALL
dump_reference_history(DeeObject *__restrict obj);
#endif /* !CONFIG_TRACE_REFCHANGES */

#undef DBG_PRINT_EXTERNAL_ROOTS_FIRST
#if 1
#define DBG_PRINT_EXTERNAL_ROOTS_FIRST
#endif

#ifdef DBG_PRINT_EXTERNAL_ROOTS_FIRST
PRIVATE NONNULL((1)) void DCALL
dbg__gc_visit__decref_all__cb(DeeObject *__restrict self, void *UNUSED(arg)) {
	--self->ob_refcnt;
	if (!DeeType_IsGC(Dee_TYPE(self)) && self->ob_refcnt == 0)
		DeeObject_Visit(self, &dbg__gc_visit__decref_all__cb, NULL);
}

PRIVATE NONNULL((1)) void DCALL
dbg__gc_visit__incref_all__cb(DeeObject *__restrict self, void *UNUSED(arg)) {
	if (!DeeType_IsGC(Dee_TYPE(self)) && self->ob_refcnt == 0)
		DeeObject_Visit(self, &dbg__gc_visit__incref_all__cb, NULL);
	++self->ob_refcnt;
}


/* Drop all internal object references */
PRIVATE void DCALL dbg__gc_decref_internal_references(void) {
	struct gc_generation *gen;
	for (gen = &gc_gen0; gen; gen = gen->gg_next) {
		DeeObject *iter;
		for (iter = gen->gg_objects; iter; iter = DeeGC_Head(iter)->gc_next)
			DeeObject_Visit(iter, &dbg__gc_visit__decref_all__cb, NULL);
	}
}

/* Restore all internal object references */
PRIVATE void DCALL dbg__gc_incref_internal_references(void) {
	struct gc_generation *gen;
	for (gen = &gc_gen0; gen; gen = gen->gg_next) {
		DeeObject *iter;
		for (iter = gen->gg_objects; iter; iter = DeeGC_Head(iter)->gc_next)
			DeeObject_Visit(iter, &dbg__gc_visit__incref_all__cb, NULL);
	}
}
#endif /* DBG_PRINT_EXTERNAL_ROOTS_FIRST */

INTERN void DCALL gc_dump_all_except_dex(void) {
	struct gc_generation *gen;
#ifdef DBG_PRINT_EXTERNAL_ROOTS_FIRST
	dbg__gc_decref_internal_references();
	Dee_DPRINT("=== External roots\n");
	for (gen = &gc_gen0; gen; gen = gen->gg_next) {
		DeeObject *ob;
		for (ob = gen->gg_objects; ob; ob = DeeGC_Head(ob)->gc_next) {
			DeeTypeObject *ob_type;
			ASSERT(ob != DeeGC_Head(ob)->gc_next);
			ob_type = ob->ob_type;
			if (ob->ob_refcnt == 0)
				continue; /* Internal-only */
#ifndef CONFIG_NO_DEX
#ifdef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
			if (ob_type == &DeeModuleDex_Type)
				continue;
#else /* CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
			if (DeeType_Extends(ob_type, &DeeDex_Type))
				continue;
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
#endif /* !CONFIG_NO_DEX */
			Dee_DPRINTF("%p: %q [%" PRFuSIZ " unaccounted refs]",
			            ob, ob_type->tp_name, ob->ob_refcnt);
			/* Print the name of a select set of types. */
			gc_dprint_object_info(ob_type, ob);
			Dee_DPRINT("\n");
#ifdef CONFIG_TRACE_REFCHANGES
			dump_reference_history(&iter->gc_object);
#endif /* CONFIG_TRACE_REFCHANGES */
		}
	}
	Dee_DPRINT("\n=== Internal objects\n");
#endif /* DBG_PRINT_EXTERNAL_ROOTS_FIRST */

	for (gen = &gc_gen0; gen; gen = gen->gg_next) {
		DeeObject *ob;
		for (ob = gen->gg_objects; ob; ob = DeeGC_Head(ob)->gc_next) {
			DeeTypeObject *ob_type;
			ASSERT(ob != DeeGC_Head(ob)->gc_next);
			ob_type = ob->ob_type;
#ifdef DBG_PRINT_EXTERNAL_ROOTS_FIRST
			if (ob->ob_refcnt != 0)
				continue; /* External root */
			Dee_DPRINTF("%p: %q", ob, ob_type->tp_name);
#else /* DBG_PRINT_EXTERNAL_ROOTS_FIRST */
#ifndef CONFIG_NO_DEX
#ifdef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
			if (ob_type == &DeeModuleDex_Type)
				continue;
#else /* CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
			if (DeeType_Extends(ob_type, &DeeDex_Type))
				continue;
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
#endif /* !CONFIG_NO_DEX */
			Dee_DPRINTF("GC Object at %p: Instance of %s (%" PRFuSIZ " refs)",
			            ob, ob_type->tp_name, ob->ob_refcnt);
#endif /* !DBG_PRINT_EXTERNAL_ROOTS_FIRST */
			/* Print the name of a select set of types. */
			gc_dprint_object_info(ob_type, ob);
			Dee_DPRINT("\n");
#ifdef CONFIG_TRACE_REFCHANGES
			dump_reference_history(&iter->gc_object);
#endif /* CONFIG_TRACE_REFCHANGES */
		}
	}
#ifdef DBG_PRINT_EXTERNAL_ROOTS_FIRST
	dbg__gc_incref_internal_references();
#endif /* DBG_PRINT_EXTERNAL_ROOTS_FIRST */
}
#endif /* !NDEBUG */

DECL_END

#ifndef __INTELLISENSE__
#include "gc-inspect.c.inl"
#endif /* !__INTELLISENSE__ */

#endif /* !GUARD_DEEMON_RUNTIME_GC_C */
