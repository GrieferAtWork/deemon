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

#include <deemon/alloc.h>              /* DeeDbgObject_*, DeeMem_ClearCaches, DeeObject_*, DeeSlab_ENUMERATE, DeeSlab_Invoke, Dee_CollectMemory, Dee_Free, Dee_TYPE_CONSTRUCTOR_INIT_FIXED, Dee_TYPE_CONSTRUCTOR_INIT_FIXED_S, Dee_TryCallocc, Dee_TryReallocc */
#include <deemon/arg.h>                /* DeeArg_Unpack*, UNPxSIZ */
#include <deemon/asm.h>                /* ASM_RET_NONE, instruction_t */
#include <deemon/bool.h>               /* return_bool, return_false, return_true */
#include <deemon/class.h>              /* instance_clear, instance_tclear */
#include <deemon/code.h>               /* DeeCodeObject, DeeCode_NAME, DeeCode_Type, DeeFunctionObject, DeeFunction_*, Dee_CODE_FFINALLY, instruction_t */
#include <deemon/computed-operators.h> /* DEFIMPL, DEFIMPL_UNSUPPORTED */
#include <deemon/exec.h>               /*  */
#include <deemon/format.h>             /* PRFuSIZ */
#include <deemon/gc.h>                 /* DeeGC_*, Dee_GC_*, Dee_gc_head */
#include <deemon/int.h>                /* DeeInt_NewSize */
#include <deemon/module.h>             /* DeeModule* */
#include <deemon/object.h>             /* ASSERT_OBJECT, ASSERT_OBJECT_TYPE, DREF, DeeObject, DeeObject_*, DeeTypeObject, DeeType_Extends, Dee_AsObject, Dee_Decref*, Dee_IncrefIfNotZero, Dee_TYPE, Dee_XDecref, Dee_XIncref, Dee_hash_t, Dee_refcnt_t, Dee_ssize_t, Dee_weakref_list, Dee_weakref_support_fini, ITER_DONE, OBJECT_HEAD, OBJECT_HEAD_INIT */
#include <deemon/seq.h>                /* DeeIterator_Type, DeeSeq_Type */
#include <deemon/serial.h>             /* DeeSerial*, Dee_seraddr_t */
#include <deemon/system-features.h>    /* bzeroc, link, memcpy*, memmovedownc, memmoveupc, memset, remove */
#include <deemon/thread.h>             /* DeeThreadObject, DeeThread_IsMultiThreaded, DeeThread_ResumeAll, DeeThread_SuspendAll, DeeThread_TrySuspendAll */
#include <deemon/type.h>               /* DeeObject_GCPriority, DeeObject_Init, DeeType_*, Dee_GC_PRIORITY_EARLY, Dee_GC_PRIORITY_LATE, Dee_TYPE_CONSTRUCTOR_INIT_FIXED, Dee_TYPE_CONSTRUCTOR_INIT_FIXED_S, Dee_XVisit, Dee_visit_t, METHOD_FNOREFESCAPE, TF_*, TP_F*, TYPE_*, type_* */
#include <deemon/util/atomic.h>        /* Dee_atomic_*, atomic_* */
#include <deemon/util/hash.h>          /* Dee_HashPointer */
#include <deemon/util/lock.h>          /* Dee_atomic_lock_* */
#include <deemon/util/rlock.h>         /* Dee_rshared_lock_* */
#include <deemon/util/weakref.h>       /* Dee_weakref_list_transfer_to_dummy */

#include <hybrid/overflow.h>    /* OVERFLOW_UADD */
#include <hybrid/sched/yield.h> /* SCHED_YIELD */
#include <hybrid/typecore.h>    /* __BYTE_TYPE__, __SIZEOF_POINTER__ */

#include "../objects/gc_inspect.h"
#include "strings.h"

#include <stdbool.h> /* bool, true */
#include <stddef.h>  /* offsetof, size_t */
#include <stdint.h>  /* UINT32_C, uint16_t, uintptr_t */

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
		ob = (DeeObject *)DeeFunction_CODE(ob);
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


#ifdef CONFIG_EXPERIMENTAL_REWORKED_GC

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
	 * interest to GC visitation. */
	if (DeeType_IsHeapType(Dee_TYPE(self)))
		(*proc)((DeeObject *)Dee_TYPE(self), arg);
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
#define gc_lock_acquire()    Dee_atomic_lock_acquire(&gc_lock)
#define gc_lock_waitfor()    Dee_atomic_lock_waitfor(&gc_lock)
#define _gc_lock_release()   Dee_atomic_lock_release(&gc_lock)

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
	bool should_collect = false;
	DeeObject *pending_remove, *removeme;
	/* Clear the must-reap flag */
	atomic_write(&gc_mustreap, false);

	/* Consume list of pending remove operations */
	pending_remove = atomic_xch(&gc_remove, NULL);

	/* Reap pending inserts for every generation */
	gc_reap_insert();

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

PRIVATE void DCALL gc_lock_reap_and_release(unsigned int flags) {
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
				return;
			break;
		default: __builtin_unreachable();
		}
	} while (atomic_read(&gc_mustreap) && gc_lock_tryacquire());
}

PRIVATE void DCALL gc_lock_reap(unsigned int flags) {
	if (gc_lock_tryacquire())
		gc_lock_reap_and_release(flags);
}

PRIVATE void DCALL gc_lock_force_reap(unsigned int flags) {
	gc_lock_acquire();
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
		if (atomic_read(&gc_insert) != NULL)
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
			/* NOTE: "Dee_weakref_support_fini()" is actually more of a "Dee_weakref_support_clear()",
			 *       in that it leaves the list in a status where it appears empty, which is exactly
			 *       what we want here! */
			(Dee_weakref_support_fini)(wrefs);
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




/* @return: true:  Success
 * @return: false: [allow_interrupts_and_errors] An error was thrown
 * @return: false: [!allow_interrupts_and_errors] Failed to acquire locks */
PRIVATE WUNUSED bool DCALL
gc_collect_acquire(bool allow_interrupts_and_errors) {
	DeeObject *pending_remove;
	DeeThreadObject *threads;
again:
	/* Wait for "gc_lock" and "gc_remove_modifying" to settle.
	 *
	 * During this part, also try to make it so there's a low
	 * probability of the GC needing to be reaped once we've
	 * suspended other threads below! */
	for (;;) {
		while (!gc_lock_available() || atomic_read(&gc_remove_modifying))
			SCHED_YIELD();
		if (!atomic_read(&gc_mustreap))
			break;
		gc_lock_force_reap(DeeGC_TRACK_F_NOCOLLECT);
	}

	/* Suspend all other threads (or at least *try* to) */
	if (allow_interrupts_and_errors) {
		threads = DeeThread_SuspendAll();
	} else {
		threads = DeeThread_TrySuspendAll();
	}
	if unlikely(!threads)
		goto err;

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

	return true;
resume_threads_and_try_again:
	DeeThread_ResumeAll();
	goto again;
err:
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
			if (tp_self->tp_gc->tp_clear == &instance_clear) {
				/* XXX: "instance_clear" is never actually called -- maybe merge with "Dee_TF_TPVISIT"? */
				instance_tclear(tp_self, self);
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

	if (!has_collectable_generations)
		return true; /* Nothing left to do! */
	if (!gc_collect_acquire(DeeGC_TRACK_F_NOCOLLECT))
		return false;

	/* Collect memory in GC generations */
	status = gc_collect_threshold_generations_r(&gc_gen0);
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
	if (!should_collect)
		return true;
	if (!gc_collect_acquire(DeeGC_TRACK_F_NOCOLLECT))
		return false;

	/* Collect generation #0 */
	iter = &gc_gen0;
continue_with_iter:
	if (iter->gg_collect_on < 0) {
		unsigned int flags;
		flags = GC_GENERATION_COLLECT_OR_UNLOCK__F_NORMAL;
		if (iter->gg_next)
			flags |= GC_GENERATION_COLLECT_OR_UNLOCK__F_MOVE_REACHABLE;
		status = gc_generation_collect_or_unlock(iter, &count, flags);
	} else {
		/* Stop once all overflowing objects have been settled into appropriate generations */
		status = GC_GENERATION_COLLECT_OR_UNLOCK__SUCCESS;
		gc_collect_release();
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
	unsigned int status, flags;
again:
	if (!gc_collect_acquire(true))
		goto err;

	/* Collect garbage in every generation until we hit "max_objects" or scanned them all */
	iter = &gc_gen0;
continue_with_iter:
	flags = GC_GENERATION_COLLECT_OR_UNLOCK__F_NORMAL;
#if 0 /* Don't move objects in this mode */
	if (iter->gg_next)
		flags |= GC_GENERATION_COLLECT_OR_UNLOCK__F_MOVE_REACHABLE;
#endif

	/* NOTE: This won't work properly to identify unreachable objects
	 *       that are only referenced by other objects from other generations.
	 *
	 * Instead, there needs to be a secondary gc collection function
	 * that scans and collects objects across **all** generations. */
	status = gc_generation_collect_or_unlock(iter, &count, flags);
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
				goto err;
			goto attempt_collect_all;
		}
		if (result < max_objects)
			goto again;
		break;
	case GC_GENERATION_COLLECT_OR_UNLOCK__RETRY:
		goto again;
	default: __builtin_unreachable();
	}
	if unlikely(result == (size_t)-1)
		result = (size_t)-2;
	return result;
err:
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


#else /* CONFIG_EXPERIMENTAL_REWORKED_GC */
#ifndef NDEBUG
#define DBG_memset (void)memset
#else /* !NDEBUG */
#define DBG_memset(dst, byte, n_bytes) (void)0
#endif /* NDEBUG */


/* Required in case new GC objects are scheduled
 * whilst tp_gc() is invoked on some GC object
 * when `gc_lock' is still being held. */
#undef CONFIG_HAVE_PENDING_GC_OBJECTS
#if 1
#define CONFIG_HAVE_PENDING_GC_OBJECTS
#endif

/* Use priority clear (`tp_pclear') instead of `tp_clear' */
#undef CONFIG_GC_PRIORITY_CLEAR
#if 1
#define CONFIG_GC_PRIORITY_CLEAR
#endif

/* Construct the dependency partners vector such that
 * it can be scanned using a bsearch-style approach. */
#undef CONFIG_GC_DEP_PARTNERS_USE_BSEARCH
#if 1
#define CONFIG_GC_DEP_PARTNERS_USE_BSEARCH
#endif

/* Keep track of leaf objects to speed up gc traversal. */
#undef CONFIG_GC_TRACK_LEAFS
#if 1
#define CONFIG_GC_TRACK_LEAFS
#endif




#ifdef CONFIG_NO_THREADS
#undef CONFIG_HAVE_PENDING_GC_OBJECTS
#endif /* CONFIG_NO_THREADS */

/* _Must_ use a recursive lock for this because untracking objects
 * while they're being collected requires acquisition of the GC-lock. */
#ifndef CONFIG_NO_THREADS
INTERN Dee_rshared_lock_t gc_lock = Dee_RSHARED_LOCK_INIT;
#endif /* !CONFIG_NO_THREADS */
#define GCLOCK_TRYACQUIRE()         Dee_rshared_lock_tryacquire(&gc_lock)
#define GCLOCK_ACQUIRE_NOINT()      Dee_rshared_lock_acquire_noint(&gc_lock)
#define GCLOCK_ACQUIRE()            Dee_rshared_lock_acquire(&gc_lock)
#define GCLOCK_RELEASE()            Dee_rshared_lock_release(&gc_lock)
#define GCLOCK_ACQUIRE_READ_NOINT() Dee_rshared_lock_acquire_noint(&gc_lock)
#define GCLOCK_ACQUIRE_READ()       Dee_rshared_lock_acquire(&gc_lock)
#define GCLOCK_RELEASE_READ()       Dee_rshared_lock_release(&gc_lock)

#ifndef NDEBUG
#if __SIZEOF_POINTER__ == 4
#define GCHEAD_ISTRACKED(h) ((h)->gc_pself != (struct Dee_gc_head **)UINT32_C(0xcccccccc))
#elif __SIZEOF_POINTER__ == 8
#define GCHEAD_ISTRACKED(h) ((h)->gc_pself != (struct Dee_gc_head **)UINT32_C(0xcccccccccccccccc))
#endif /* __SIZEOF_POINTER__ == ... */
#endif /* !NDEBUG */


/* [lock(gc_lock)][0..1] The list root of all GC objects. */
INTERN struct Dee_gc_head *gc_root = NULL;
PRIVATE bool did_untrack = false;

#ifdef CONFIG_HAVE_PENDING_GC_OBJECTS
PRIVATE struct Dee_gc_head *gc_pending = NULL; /* [0..1] Pending object for GC tracking. */
#define GC_PENDING_MUSTSERVICE() (atomic_read(&gc_pending) != NULL)
PRIVATE void DCALL gc_pending_service(void) {
	struct Dee_gc_head *chain;
	chain = atomic_xch(&gc_pending, NULL);
	if (chain) {
		struct Dee_gc_head *next, **p_self;
		p_self = &gc_root;
		for (next = chain;;) {
			next->gc_pself = p_self;
			p_self = &next->gc_next;
			next   = *p_self;
			if (!next)
				break;
		}
		next = COMPILER_CONTAINER_OF(p_self, struct Dee_gc_head, gc_next); /* last */
		ASSERT(next->gc_next == NULL);
		ASSERT(chain->gc_pself == &gc_root);
		/* Append the old GC chain */
		if ((next->gc_next = gc_root) != NULL)
			gc_root->gc_pself = &next->gc_next;
		gc_root = chain; /* Set the new GC chain */
	}
}

LOCAL void DCALL gc_lock_acquire_s(void) {
	GCLOCK_ACQUIRE_NOINT();
	gc_pending_service();
}
LOCAL void DCALL gc_lock_release_s(void) {
again:
	gc_pending_service();
	GCLOCK_RELEASE();
	if unlikely(GC_PENDING_MUSTSERVICE()) {
		if (GCLOCK_TRYACQUIRE())
			goto again;
	}
}
#define GCLOCK_ACQUIRE_S() gc_lock_acquire_s()
#define GCLOCK_RELEASE_S() gc_lock_release_s()
#else /* CONFIG_HAVE_PENDING_GC_OBJECTS */
#define GCLOCK_ACQUIRE_S() GCLOCK_ACQUIRE_NOINT()
#define GCLOCK_RELEASE_S() GCLOCK_RELEASE()
#endif /* !CONFIG_HAVE_PENDING_GC_OBJECTS */


/* Begin/end tracking a given GC-allocated object.
 * `DeeGC_Track()' must be called explicitly when the object
 * has been allocated using `DeeGCObject_Malloc' and friends,
 * though constructions of non-variadic GC objects don't need
 * to call this function on the passed object. - That call will
 * automatically be done when the function returns successfully.
 * @return: * : == ob */
PUBLIC ATTR_RETNONNULL NONNULL((1)) DeeObject *DCALL
DeeGC_Track(DeeObject *__restrict ob) {
	struct Dee_gc_head *head;
	ASSERT_OBJECT_TYPE(ob->ob_type, &DeeType_Type);
	ASSERT(DeeType_IsGC(Dee_TYPE(ob)));
	head = DeeGC_Head(ob);
#ifdef GCHEAD_ISTRACKED
	ASSERTF(!GCHEAD_ISTRACKED(head),
	        "Object is already being tracked");
#endif /* GCHEAD_ISTRACKED */
#ifdef CONFIG_HAVE_PENDING_GC_OBJECTS
	if (!GCLOCK_TRYACQUIRE()) {
		struct Dee_gc_head *next;
#ifdef GCHEAD_ISTRACKED
		head->gc_pself = (struct Dee_gc_head **)1;
		ASSERT(GCHEAD_ISTRACKED(head));
#endif /* GCHEAD_ISTRACKED */
		do {
			next = atomic_read(&gc_pending);
			head->gc_next = next;
			COMPILER_WRITE_BARRIER();
		} while unlikely(!atomic_cmpxch_weak_or_write(&gc_pending, next, head));
	} else {
		gc_pending_service();
		ASSERT(head != gc_root);
		if ((head->gc_next = gc_root) != NULL)
			gc_root->gc_pself = &head->gc_next;
		head->gc_pself = &gc_root;
		gc_root        = head;
		GCLOCK_RELEASE_S();
	}
#else /* CONFIG_HAVE_PENDING_GC_OBJECTS */
	GCLOCK_ACQUIRE_S();
	ASSERT(head != gc_root);
	if ((head->gc_next = gc_root) != NULL)
		gc_root->gc_pself = &head->gc_next;
	head->gc_pself = &gc_root;
	gc_root        = head;
	GCLOCK_RELEASE_S();
#endif /* !CONFIG_HAVE_PENDING_GC_OBJECTS */
	return ob;
}

PUBLIC ATTR_RETNONNULL NONNULL((1)) DeeObject *DCALL
DeeGC_Untrack(DeeObject *__restrict ob) {
	struct Dee_gc_head *head;
	ASSERT_OBJECT_TYPE(ob->ob_type, &DeeType_Type);
	ASSERT(DeeType_IsGC(Dee_TYPE(ob)));
	head = DeeGC_Head(ob);
#ifdef GCHEAD_ISTRACKED
	ASSERTF(GCHEAD_ISTRACKED(head),
	        "Object %p of type %k isn't being tracked\n"
	        "repr: `%r'",
	        ob, ob->ob_type, ob);
#endif /* GCHEAD_ISTRACKED */
	GCLOCK_ACQUIRE_S();
	/*printf("Untrack: %s:%p\n", DeeType_GetName(ob->ob_type), ob);*/
	/* Unlink the gc head from whatever chain it is apart of. */
	/* HINT: If the following line crashes, it is likely that a GC-object was
	 *       allocated using something other than the GC-object allocators:
	 *       >> DeeObject *my_gc_ob = DeeObject_Malloc(42); // Should have used `DeeGCObject_Malloc'
	 */
	if ((*head->gc_pself = head->gc_next) != NULL)
		head->gc_next->gc_pself = head->gc_pself;
	did_untrack = true;
	GCLOCK_RELEASE_S();
	DBG_memset(head, 0xcc, Dee_GC_HEAD_SIZE);
	return ob;
}

/* Track all GC objects in range [first,last], all of which have
 * already been linked together using their `struct Dee_gc_head' */
PUBLIC WUNUSED bool DCALL DeeGC_TrackMany_TryLock(void) {
	bool result = GCLOCK_TRYACQUIRE();
#ifdef CONFIG_HAVE_PENDING_GC_OBJECTS
	if (result)
		gc_pending_service();
#endif /* CONFIG_HAVE_PENDING_GC_OBJECTS */
	return result;
}

PUBLIC void DCALL DeeGC_TrackMany_Lock(void) {
	GCLOCK_ACQUIRE_S();
}

PUBLIC NONNULL((1, 2)) void DCALL DeeGC_TrackMany_Exec(DeeObject *first, DeeObject *last) {
	struct Dee_gc_head *first_head = DeeGC_Head(first);
	struct Dee_gc_head *last_head  = DeeGC_Head(last);
	if ((last_head->gc_next = gc_root) != NULL)
		gc_root->gc_pself = &last_head->gc_next;
	first_head->gc_pself = &gc_root;
	gc_root = first_head;
}

PUBLIC void DCALL DeeGC_TrackMany_Unlock(void) {
	GCLOCK_RELEASE_S();
}



struct gc_dep {
	Dee_refcnt_t    gd_extern; /* [valid_if(gd_object)] Amount of external (unaccounted) references. */
	DREF DeeObject *gd_object; /* [0..1] The object in question. */
};
struct gc_deps {
	size_t          gd_cnt;    /* Number of dependencies. */
	size_t          gd_msk;    /* [!0] Dependency mask. */
	struct gc_dep  *gd_vec;    /* [1..gd_msk+1] Hash-vector of dependencies. */
	bool            gd_err;    /* An error occurred (not enough memory; try again later...) */
};
#define VD_HASHOF(dat, ob)     (Dee_HashPointer(ob) & (dat)->gd_msk)
#define VD_HASHNX(hs, perturb) (((hs) << 2) + (hs) + (perturb) + 1)
#define VD_HASHPT(perturb)     ((perturb) >>= 5)

#ifdef CONFIG_GC_TRACK_LEAFS
struct gc_leaf {
	DREF DeeObject *gl_obj; /* [0..1] The leaf object. */
};

struct gc_leafs {
	/* GC leaf objects (i.e. objects checked, but found to not be apart of the current loop) */
	size_t          gl_cnt; /* Number of leafs. */
	size_t          gl_msk; /* [!0] Leaf mask. */
	struct gc_leaf *gl_vec; /* [1..gd_msk+1] Hash-vector of leafs. */
};
#define VL_HASHOF(dat, ob)     (Dee_HashPointer(ob) & (dat)->gl_msk)
#define VL_HASHNX(hs, perturb) (((hs) << 2) + (hs) + (perturb) + 1)
#define VL_HASHPT(perturb)     ((perturb) >>= 5)
#endif /* CONFIG_GC_TRACK_LEAFS */


struct gc_dep_partner {
	DeeObject   *dp_obj;  /* [1..1] The object that is apart of the current, unconfirmed chain of dependencies. */
	Dee_refcnt_t dp_num;  /* [!0] Amount of reference accounted for this chain entry. */
};

struct gc_dep_partners {
	Dee_refcnt_t           dp_pnum; /* Number of partner dependencies. */
	struct gc_dep_partner *dp_pvec; /* [0..dp_pnum][owned] Vector of partner dependencies. */
};

/* Try to locate a partner dependency for `obj'
 * If no such dependency exists, return `NULL' instead. */
LOCAL WUNUSED NONNULL((1, 2)) struct gc_dep_partner *DCALL
gc_dep_partners_locate(struct gc_dep_partners const *__restrict self,
                       DeeObject *__restrict obj) {
#ifdef CONFIG_GC_DEP_PARTNERS_USE_BSEARCH
	size_t lo, hi;
	lo = 0;
	hi = self->dp_pnum;
	while likely(lo < hi) {
		size_t test_index;
		DeeObject *other;
		test_index = (lo + hi) / 2;
		other      = self->dp_pvec[test_index].dp_obj;
		if (obj < other) {
			hi = test_index;
		} else if (obj > other) {
			lo = test_index + 1;
		} else {
			/* Found it! */
			return &self->dp_pvec[test_index];
		}
	}
	return NULL;
#else /* CONFIG_GC_DEP_PARTNERS_USE_BSEARCH */
	size_t i;
	for (i = 0; i < self->dp_pnum; ++i) {
		if (self->dp_pvec[i].dp_obj == obj)
			return &self->dp_pvec[i];
	}
	return NULL;
#endif /* !CONFIG_GC_DEP_PARTNERS_USE_BSEARCH */
}

/* Try to resize the given partner dependency set. */
LOCAL WUNUSED NONNULL((1)) bool DCALL
gc_dep_partners_resize(struct gc_dep_partners *__restrict self,
                       size_t new_num_objs) {
	struct gc_dep_partner *new_vec;
	new_vec = (struct gc_dep_partner *)Dee_TryReallocc(self->dp_pvec, new_num_objs,
	                                                   sizeof(struct gc_dep_partner));
	if unlikely(!new_vec) {
		DeeMem_ClearCaches((size_t)-1);
		new_vec = (struct gc_dep_partner *)Dee_TryReallocc(self->dp_pvec, new_num_objs,
		                                                   sizeof(struct gc_dep_partner));
		if unlikely(!new_vec)
			return false;
	}
	self->dp_pvec = new_vec;
	self->dp_pnum = new_num_objs;
	return true;
}

/* Insert the `nth_object' after a call to `gc_dep_partners_resize()' */
LOCAL NONNULL((1)) void DCALL
gc_dep_partners_insert_after_resize(struct gc_dep_partners *__restrict self,
                                    size_t nth_object,
                                    DeeObject *__restrict obj,
                                    Dee_refcnt_t num) {
#ifdef CONFIG_GC_DEP_PARTNERS_USE_BSEARCH
	size_t lo, hi;
	ASSERT(nth_object < self->dp_pnum);
	lo = 0;
	hi = nth_object;
	while likely(lo < hi) {
		size_t test_index;
		DeeObject *other;
		test_index = (lo + hi) / 2;
		other      = self->dp_pvec[test_index].dp_obj;
		if (obj < other) {
			hi = test_index;
		} else {
			ASSERTF(obj != other,
			        "Duplicate object %p",
			        obj);
			lo = test_index + 1;
		}
	}
	ASSERT(lo == hi);
	memmoveupc(&self->dp_pvec[lo] + 1,
	           &self->dp_pvec[lo],
	           nth_object - lo,
	           sizeof(struct gc_dep_partner));
	self->dp_pvec[lo].dp_obj = obj;
	self->dp_pvec[lo].dp_num = num;
#else /* CONFIG_GC_DEP_PARTNERS_USE_BSEARCH */
	ASSERT(nth_object < self->dp_pnum);
	self->dp_pvec[nth_object].dp_obj = obj;
	self->dp_pvec[nth_object].dp_num = num;
#endif /* !CONFIG_GC_DEP_PARTNERS_USE_BSEARCH */
}

/* NOTE: This function assumes that `vec' is sorted by object address
 *       when `CONFIG_GC_DEP_PARTNERS_USE_BSEARCH' is defined! */
LOCAL NONNULL((1)) void DCALL
gc_dep_partners_insertall_after_resize(struct gc_dep_partners *__restrict self,
                                       size_t nth_object,
                                       struct gc_dep_partner const *__restrict vec,
                                       size_t count) {
#ifdef CONFIG_GC_DEP_PARTNERS_USE_BSEARCH
	size_t dst_index, src_index;
	for (src_index = dst_index = 0; src_index < count; ++src_index) {
		DeeObject *obj;
		obj = vec[src_index].dp_obj;
		for (;;) {
			if (dst_index >= nth_object) {
				memcpyc(&self->dp_pvec[nth_object],
				        &vec[src_index],
				        count - src_index,
				        sizeof(struct gc_dep_partner));
				return;
			}
			ASSERTF(obj != self->dp_pvec[dst_index].dp_obj,
			        "Duplicate object %p",
			        obj);
			if (obj <= self->dp_pvec[dst_index].dp_obj)
				break;
			++dst_index;
		}
		/* Insert the object here. */
		memmoveupc(&self->dp_pvec[dst_index] + 1,
		           &self->dp_pvec[dst_index],
		           nth_object - dst_index,
		           sizeof(struct gc_dep_partner));
		self->dp_pvec[dst_index].dp_obj = obj;
		self->dp_pvec[dst_index].dp_num = vec[src_index].dp_num;
		++dst_index;
		++nth_object;
	}
#else /* CONFIG_GC_DEP_PARTNERS_USE_BSEARCH */
	ASSERT((nth_object + count) <= self->dp_pnum);
	ASSERT(count != 0);
	memcpyc(&self->dp_pvec[nth_object], vec,
	        count, sizeof(struct gc_dep_partner));
#endif /* !CONFIG_GC_DEP_PARTNERS_USE_BSEARCH */
}

struct gc_dep_chain {
	struct gc_dep_chain   *dc_prev; /* [0..1] Another object dependency also apart of this chain. */
	DeeObject             *dc_obj;  /* [1..1] The object that is apart of the current, unconfirmed chain of dependencies. */
	Dee_refcnt_t           dc_num;  /* [!0] Amount of reference accounted for this chain entry. */
	struct gc_dep_partners dc_part; /* Partner dependencies. */
};

struct visit_data {
	struct gc_deps       vd_deps;   /* Set of known dependencies. */
#ifdef CONFIG_GC_TRACK_LEAFS
	struct gc_leafs      vd_leafs;  /* Set of known leaf objects. */
#endif /* CONFIG_GC_TRACK_LEAFS */
	struct gc_dep_chain *vd_chain;  /* [0..1] Chain of unconfirmed dependencies. */
};

/* Fallback buffer used for when there is not enough memory
 * to create a larger buffer for tracking visited objects. */
PRIVATE struct gc_dep static_visit_buffer[1 << 8];
#ifdef CONFIG_GC_TRACK_LEAFS
PRIVATE struct gc_leaf static_leaf_buffer[1 << 7];
#endif /* CONFIG_GC_TRACK_LEAFS */


#undef GC_ASSERT_REFERENCE_COUNTS

#if !defined(NDEBUG) && 0
/* Perform some non-thread-save assertions about visit-reference counters. */
#define GC_ASSERT_REFERENCE_COUNTS 1
#endif



LOCAL bool DCALL
gc_deps_rehash(struct gc_deps *__restrict self) {
	struct gc_dep *new_vec;
	size_t i, j, perturb, new_mask = (self->gd_msk << 1) | 1;
	new_vec = (struct gc_dep *)Dee_TryCallocc(new_mask + 1,
	                                          sizeof(struct gc_dep));
	if unlikely(!new_vec)
		return false;
	/* Re-insert all objects into the new hash-vector. */
	for (i = 0; i <= self->gd_msk; ++i) {
		struct gc_dep *old_dep;
		old_dep = &self->gd_vec[i];
		if (!old_dep->gd_object)
			continue;
		perturb = j = Dee_HashPointer(old_dep->gd_object) & new_mask;
		for (;; j = VD_HASHNX(j, perturb), VD_HASHPT(perturb)) {
			struct gc_dep *dep;
			dep = &new_vec[j & new_mask];
			if (dep->gd_object)
				continue;
			/* Copy over the dependency. */
			memcpy(dep, old_dep, sizeof(struct gc_dep));
			break;
		}
	}
	if (self->gd_vec != static_visit_buffer)
		Dee_Free(self->gd_vec);
	self->gd_msk = new_mask;
	self->gd_vec = new_vec;
	return true;
}


/* Insert a dependency for `obj'
 * This function will create a new reference to `obj' */
LOCAL void DCALL
gc_deps_insert(struct gc_deps *__restrict self,
               DeeObject *__restrict obj,
               Dee_refcnt_t num_tracked_references) {
	Dee_hash_t i, perturb;
	ASSERT(num_tracked_references != 0);
#if 0 /* Can't happen */
	perturb = i = VD_HASHOF(self, obj);
	/* Check if the object had been added before. */
	for (;; i = VD_HASHNX(i, perturb), VD_HASHPT(perturb)) {
		struct gc_dep *dep;
		dep = &self->gd_vec[i & self->gd_msk];
		if (!dep->gd_object)
			break;
		if (dep->gd_object != obj)
			continue;
		/* It has! */
		if (dep->gd_extern)
			--dep->gd_extern;
		return;
	}
#endif
	if (self->gd_cnt * 2 >= self->gd_msk &&
	    !gc_deps_rehash(self)) {
		if (self->gd_cnt == self->gd_msk) {
			/* At this point, we _need_ to rehash, or we
			 * can't continue using the hash-vector. */
			DeeMem_ClearCaches((size_t)-1);
			if (!gc_deps_rehash(self)) {
				/* Well... we're screwed! */
				self->gd_err = true;
				return;
			}
		}
	}
	perturb = i = VD_HASHOF(self, obj);
	for (;; i = VD_HASHNX(i, perturb), VD_HASHPT(perturb)) {
		struct gc_dep *dep;
		dep = &self->gd_vec[i & self->gd_msk];
		ASSERT(dep->gd_object != obj);
		if (dep->gd_object)
			continue;
		/* Use this slot! */
		dep->gd_object = obj;
		dep->gd_extern = atomic_fetchinc(&obj->ob_refcnt);
		ASSERT(dep->gd_extern != 0);
#ifdef GC_ASSERT_REFERENCE_COUNTS
		ASSERT(dep->gd_extern >= num_tracked_references);
#endif /* GC_ASSERT_REFERENCE_COUNTS */
		if (dep->gd_extern >= num_tracked_references) {
			dep->gd_extern -= num_tracked_references;
		} else {
			dep->gd_extern = 0;
		}
		break;
	}
	++self->gd_cnt;
}



#ifdef CONFIG_GC_TRACK_LEAFS
LOCAL bool DCALL
gc_leafs_rehash(struct gc_leafs *__restrict self) {
	struct gc_leaf *new_vec;
	size_t i, j, perturb, new_mask;
	new_mask = (self->gl_msk << 1) | 1;
	new_vec = (struct gc_leaf *)Dee_TryCallocc(new_mask + 1,
	                                           sizeof(struct gc_leaf));
	if unlikely(!new_vec)
		return false;
	/* Re-insert all objects into the new hash-vector. */
	for (i = 0; i <= self->gl_msk; ++i) {
		struct gc_leaf *old_leaf;
		old_leaf = &self->gl_vec[i];
		if (!old_leaf->gl_obj)
			continue;
		perturb = j = Dee_HashPointer(old_leaf->gl_obj) & new_mask;
		for (;; j = VL_HASHNX(j, perturb), VL_HASHPT(perturb)) {
			struct gc_leaf *leaf;
			leaf = &new_vec[j & new_mask];
			if (leaf->gl_obj)
				continue;
			/* Copy over the dependency. */
			memcpy(leaf, old_leaf, sizeof(struct gc_leaf));
			break;
		}
	}
	if (self->gl_vec != static_leaf_buffer)
		Dee_Free(self->gl_vec);
	self->gl_msk = new_mask;
	self->gl_vec = new_vec;
	return true;
}

/* Remember `obj' as a known leaf
 * This function will create a new reference to `obj' */
LOCAL void DCALL
gc_leafs_insert(struct gc_leafs *__restrict self,
                DeeObject *__restrict obj) {
	Dee_hash_t i, perturb;
	if (self->gl_cnt * 2 >= self->gl_msk &&
	    !gc_leafs_rehash(self)) {
		if (self->gl_cnt == self->gl_msk) {
			/* At this point, we _need_ to rehash, or we
			 * can't continue using the hash-vector. */
			DeeMem_ClearCaches((size_t)-1);
			if (!gc_leafs_rehash(self)) {
				/* It's OK if this fails. - Keeping track of leaf objects
				 * is entirely optional and only done to cut down on the
				 * number of unnecessary objects getting visited. */
				return;
			}
		}
	}
	perturb = i = VL_HASHOF(self, obj);
	for (;; i = VL_HASHNX(i, perturb), VL_HASHPT(perturb)) {
		struct gc_leaf *leaf;
		leaf = &self->gl_vec[i & self->gl_msk];
		ASSERT(leaf->gl_obj != obj);
		if (!leaf->gl_obj) {
			/* Use this slot! */
			leaf->gl_obj = obj;
			break;
		}
	}
	++self->gl_cnt;
}
#endif /* CONFIG_GC_TRACK_LEAFS */


PRIVATE void DCALL
visit_object(DeeObject *__restrict self,
             struct visit_data *__restrict data) {
	/* Step #1: Check if the object is already apart
	 *          of the set of confirmed dependencies.
	 *          If it is, decrement the external-reference
	 *          counter and add all objects apart of the
	 *          current chain of unconfirmed dependencies
	 *          to the set of known dependencies, as a
	 *          loop was detected that wraps back around
	 *          to the a known object that uses the original,
	 *          starting GC-object.
	 *    NOTE: If the external-reference counter already
	 *          is ZERO(0) when we try to decrement it,
	 *          don't decrement it again (can happen due
	 *          to potential race conditions...) */
	size_t i, perturb;
	struct gc_dep_chain node;
	struct gc_dep_chain *link;
	DeeTypeObject *tp_self;
	ASSERT_OBJECT(self);
again:
	/* Optimization: An object that cannot be visited can be skipped. */
	for (tp_self = Dee_TYPE(self); likely(tp_self); tp_self = tp_self->tp_base) {
		if likely(tp_self->tp_visit)
			goto do_the_visit;
	}
	/* In case of a user-defined type, we must still check for visiting that type itself! */
	tp_self = Dee_TYPE(self);
	if ((DeeObject *)tp_self == self)
		return;
	self = (DeeObject *)tp_self;
	goto again;
do_the_visit:
	/*Dee_DPRINTF("VISIT: %k at %p (%u refs)\n", tp_self, self, self->ob_refcnt);*/

	/* Check if this is a known dependency */
	perturb = i = VD_HASHOF(&data->vd_deps, self);
	for (;; i = VD_HASHNX(i, perturb), VD_HASHPT(perturb)) {
		struct gc_dep *dep;
		dep = &data->vd_deps.gd_vec[i & data->vd_deps.gd_msk];
		if (!dep->gd_object)
			break;
		if (dep->gd_object != self)
			continue;
		/* Found it! */
#ifdef GC_ASSERT_REFERENCE_COUNTS
		ASSERT(dep->gd_extern != 0);
#endif /* GC_ASSERT_REFERENCE_COUNTS */
		if (dep->gd_extern)
			--dep->gd_extern;
		/* Add all potential dependencies encountered up until this point. */
		link           = data->vd_chain;
		data->vd_chain = NULL;
		for (; link; link = link->dc_prev) {
			size_t j;
			/* Insert the link dependency. */
			gc_deps_insert(&data->vd_deps, link->dc_obj, link->dc_num);
			/* Also insert partner dependencies. */
			for (j = 0; j < link->dc_part.dp_pnum; ++j) {
				gc_deps_insert(&data->vd_deps,
				               link->dc_part.dp_pvec[j].dp_obj,
				               link->dc_part.dp_pvec[j].dp_num);
			}
		}
		return;
	}

	/* Step #2: If not a confirmed dependency, check if the
	 *          object is already an unconfirmed dependency.
	 *          If it is, it is apart of a reference loop
	 *          that is unrelated to the one we're trying
	 *          to resolve, in which case return immediately. */
	link = data->vd_chain, i = 0;
	for (; link; link = link->dc_prev, ++i) {
		struct gc_dep_partner *partner;
		if (link->dc_obj == self) {
			/* Found another reference to this object. */
			++link->dc_num;
#ifdef GC_ASSERT_REFERENCE_COUNTS
			ASSERT(link->dc_obj->ob_refcnt >= link->dc_num);
#endif /* GC_ASSERT_REFERENCE_COUNTS */
			goto found_chain_link;
		}
		/* Must also search partners. */
		partner = gc_dep_partners_locate(&link->dc_part, self);
		if (partner) {
			/* Found a partner! */
			++partner->dp_num;
#ifdef GC_ASSERT_REFERENCE_COUNTS
			ASSERT(partner->dp_obj->ob_refcnt >= partner->dp_num);
#endif /* GC_ASSERT_REFERENCE_COUNTS */
			goto found_chain_link;
		}
		continue;
found_chain_link:
		/* Should the object at `link' get added to the effective dependency-set,
		 * then so must all other unconfirmed dependencies from `data->vd_chain'
		 * to `link':
		 *              +--------+
		 *              v        |
		 *    [a] ---> [b] ---> [c]
		 *     ^        |
		 *     |        v
		 *     +------ [d]
		 *
		 *
		 * When c's link to b is first encountered, we get here to increment the
		 * known-reference counter of b. However, as can be seen above, `b' later
		 * points to another object d, which then points back to a.
		 * If we didn't manually add all objects part of the reference loop pointing
		 * back to b, then they wouldn't appear as dependencies of a, and would have
		 * no influence of when a can be destroyed.
		 * The solution is to keep a local set of partner-dependencies in each chain
		 * entry, that are added alongside the chain entry itself in the event that
		 * the chain gets added as a true dependency.
		 * As far as influence goes, these partner-dependencies have the same weight
		 * as the base-dependency!
		 */
		if (i != 0) {
			/* Transfer partner dependencies. */
			size_t dst;
			struct gc_dep_chain *iter;
			iter = data->vd_chain;
			for (; iter != link; iter = iter->dc_prev)
				i += iter->dc_part.dp_pnum; /* Flatten inner partners. */
			dst = link->dc_part.dp_pnum;
			if unlikely(!gc_dep_partners_resize(&link->dc_part, dst + i)) {
				data->vd_deps.gd_err = true; /* What'sha gonna do? */
				return;
			}
			for (iter = data->vd_chain; iter != link; iter = iter->dc_prev) {
				gc_dep_partners_insert_after_resize(&link->dc_part,
				                                    dst,
				                                    iter->dc_obj,
				                                    iter->dc_num);
				++dst;
				/* Copy old partners. */
				if (iter->dc_part.dp_pnum) {
					gc_dep_partners_insertall_after_resize(&link->dc_part,
					                                       dst,
					                                       iter->dc_part.dp_pvec,
					                                       iter->dc_part.dp_pnum);
					dst += iter->dc_part.dp_pnum;
				}
			}
			ASSERT(dst == link->dc_part.dp_pnum);
			/* Remove partners objects from the
			 * base-chain of unconfirmed dependencies. */
			data->vd_chain = link;
		}
		ASSERT(data->vd_chain == link);
		return; /* We're already searching this one! */
	}

#ifdef CONFIG_GC_TRACK_LEAFS
	/* Optional: Check if this is a leaf object */
	perturb = i = VL_HASHOF(&data->vd_leafs, self);
	for (;; i = VL_HASHNX(i, perturb), VL_HASHPT(perturb)) {
		struct gc_leaf *leaf;
		leaf = &data->vd_leafs.gl_vec[i & data->vd_leafs.gl_msk];
		if (!leaf->gl_obj)
			break;
		if (leaf->gl_obj == self)
			return; /* Skip known leafs */
	}
#endif /* CONFIG_GC_TRACK_LEAFS */

	/* Step #3: Create a stack-allocated dependency entry for the
	 *          object and update visit_data to take it into
	 *          account before proceeding to recursively search
	 *          other dependencies. */
	node.dc_prev         = data->vd_chain;
	node.dc_obj          = self;
	node.dc_num          = 1;
	node.dc_part.dp_pnum = 0;
	node.dc_part.dp_pvec = NULL;
	data->vd_chain       = &node;

	/* Recursively search this object. */
	DeeObject_Visit(self, (Dee_visit_t)&visit_object, data);

	/* Step #4: Remove our stack-allocated dependency entry if it
	 *          is the one located on-top of the stack. */
	if (data->vd_chain == &node)
		data->vd_chain = node.dc_prev;
	/* Free the vector of partner dependencies. */
	Dee_Free(node.dc_part.dp_pvec);

#ifdef CONFIG_GC_TRACK_LEAFS
	/* Optional: Mark the visited object as a known leaf */
	gc_leafs_insert(&data->vd_leafs, self);
#endif /* CONFIG_GC_TRACK_LEAFS */
}


#define INITIAL_DYNAMIC_VISIT_BUFFER_MASK 31

PRIVATE size_t DCALL
gc_trydestroy(struct Dee_gc_head *__restrict head,
              struct gc_dep **__restrict p_dep_buffer,
              size_t *__restrict p_dep_mask
#ifdef CONFIG_GC_TRACK_LEAFS
              ,
              struct gc_leaf **__restrict p_leaf_buffer,
              size_t *__restrict p_leaf_mask
#endif /* CONFIG_GC_TRACK_LEAFS */
              ) {
	/* Step #1: Collect all objects reachable from `head' that eventually loop
	 *          back around and point back at `head' (i.e. form a loop)
	 *          At the same time, save the reference counts of all those objects.
	 * Step #2: Check the reference counts of all objects apart of such loops.
	 *          If there are any external references that are unaccounted for,
	 *          abort and don't do anything and return ZERO(0).
	 * Step #3: Using priorative clear (pclear), clear all GC objects
	 *         (NOTE: At least one gc-object will remain, as the initial
	 *                object referred to as `head' is a gc-object)
	 *          After each clear, use Dee_DecrefIfOne() on each object in the
	 *          set to remove objects that can be destroyed at that point.
	 *          Add the number of destroyed object to the resulting total.
	 * Step #4: Drop references from all remaining objects, and add the number
	 *          of objects that could be destroyed to the resulting total. */
	struct visit_data visit;
	size_t i, result = 0;
	struct gc_dep *init_dep;
	DeeObject *ob;
	visit.vd_deps.gd_cnt  = 1;
	visit.vd_deps.gd_vec  = *p_dep_buffer;
	visit.vd_deps.gd_msk  = *p_dep_mask;
	visit.vd_deps.gd_err  = false;
	visit.vd_chain        = NULL;
	bzeroc(visit.vd_deps.gd_vec,
	       visit.vd_deps.gd_msk + 1,
	       sizeof(struct gc_dep));
#ifdef CONFIG_GC_TRACK_LEAFS
	visit.vd_leafs.gl_cnt = 0;
	visit.vd_leafs.gl_vec = *p_leaf_buffer;
	visit.vd_leafs.gl_msk = *p_leaf_mask;
	bzeroc(visit.vd_leafs.gl_vec,
	       visit.vd_leafs.gl_msk + 1,
	       sizeof(struct gc_leaf));
#endif /* CONFIG_GC_TRACK_LEAFS */

	/* Add the initial dependency, that is the object being collected itself. */
	init_dep            = &visit.vd_deps.gd_vec[VD_HASHOF(&visit.vd_deps, &head->gc_object)];
	init_dep->gd_object = &head->gc_object;

	/* Capture + incref the given object's reference counter. */
	for (;;) {
		init_dep->gd_extern = atomic_read(&head->gc_object.ob_refcnt);
		if unlikely(!init_dep->gd_extern)
			return 0; /* Object is already dead! */
		if (atomic_cmpxch_weak_or_write(&head->gc_object.ob_refcnt,
		                                init_dep->gd_extern,
		                                init_dep->gd_extern + 1))
			break;
	}

	/* Recursively visit our initial dependency. */
	DeeObject_Visit(&head->gc_object, (Dee_visit_t)&visit_object, &visit);

#define OPTIMIZE_DEPS_SET()                                \
	do {                                                   \
		size_t last_used = 0;                              \
		for (i = 0; i <= visit.vd_deps.gd_msk; ++i) {      \
			size_t end_used;                               \
			if (!visit.vd_deps.gd_vec[i].gd_object)        \
				continue;                                  \
			end_used = i;                                  \
			for (end_used = i + 1;                         \
			     end_used <= visit.vd_deps.gd_msk &&       \
			     visit.vd_deps.gd_vec[end_used].gd_object; \
			     ++end_used)                               \
				;                                          \
			memmovedownc(&visit.vd_deps.gd_vec[last_used], \
			             &visit.vd_deps.gd_vec[i],         \
			             end_used - i,                     \
			             sizeof(struct gc_dep));           \
			last_used += end_used - i;                     \
			i = end_used;                                  \
		}                                                  \
		ASSERT(last_used == visit.vd_deps.gd_cnt);         \
	}	__WHILE0

#ifdef NDEBUG
	/* Check if something went wrong... */
	if (visit.vd_deps.gd_err)
		goto done;
	for (i = 0; i <= visit.vd_deps.gd_msk; ++i)
#else /* NDEBUG */
	OPTIMIZE_DEPS_SET();
	/* Check if something went wrong... */
	if (visit.vd_deps.gd_err)
		goto done;
	for (i = 0; i < visit.vd_deps.gd_cnt; ++i)
#endif /* !NDEBUG */
	{
		/* Step #2... (check external reference counts) */
		if (!visit.vd_deps.gd_vec[i].gd_object)
			continue;
		if (visit.vd_deps.gd_vec[i].gd_extern != 0)
			goto done;
	}

	/* Optimize the remainder by moving all
	 * actually used hash-vector entries together */
#ifdef NDEBUG
	OPTIMIZE_DEPS_SET();
#endif /* NDEBUG */

	/* Step #3... (Invoke tp_clear) */
#ifdef CONFIG_GC_PRIORITY_CLEAR
	for (;;) {
		unsigned int prio;
		unsigned int prio_min = Dee_GC_PRIORITY_EARLY;
		unsigned int prio_max = Dee_GC_PRIORITY_LATE;
		/* Determine the lowest GC priority of all affected objects. */
		for (i = 0; i < visit.vd_deps.gd_cnt; ++i) {
			ob = visit.vd_deps.gd_vec[i].gd_object;
			if (!ob)
				continue;
			if (visit.vd_deps.gd_vec[i].gd_extern)
				continue; /* Already partially cleared. */
			prio = DeeObject_GCPriority(ob);
			if (prio_min > prio)
				prio_min = prio;
			if (prio_max < prio)
				prio_max = prio;
		}
		if (prio_max == prio_min ||
		    prio_max == Dee_GC_PRIORITY_LATE)
			break;
		/* Go through and delete references to objects of the current priority class. */
		for (i = 0; i < visit.vd_deps.gd_cnt; ++i) {
			ob = visit.vd_deps.gd_vec[i].gd_object;
			if (!ob)
				continue;
			/* Partially (prioratively) clear this object. */
			DeeObject_PClear(ob, prio_max);
		}
		/* Try to destroy objects from the current priority class. */
		for (i = 0; i < visit.vd_deps.gd_cnt; ++i) {
			ob = visit.vd_deps.gd_vec[i].gd_object;
			if (!ob)
				continue;
			prio = DeeObject_GCPriority(ob);
			if (prio < prio_max)
				continue;
			/* Try to destroy this object _now_ */
			if (Dee_DecrefIfOne_untraced(ob)) {
				visit.vd_deps.gd_vec[i].gd_object = NULL;
				++result;
				continue;
			}
			/* Don't consider this object's priority again, thus
			 * ensuring that `prio_max' will gradually decrease. */
			visit.vd_deps.gd_vec[i].gd_extern = 1;
		}
	}
#endif /* CONFIG_GC_PRIORITY_CLEAR */

	/* Do one last full clear-pass for all remaining objects. */
	for (i = 0; i < visit.vd_deps.gd_cnt; ++i) {
		ob = visit.vd_deps.gd_vec[i].gd_object;
		if (!ob)
			continue;
		/* Clear this object. */
		DeeObject_Clear(ob);
		/* Drop the reference that we were holding to this object. */
		if (Dee_DecrefAndFetch_untraced(ob) == 0)
			++result;
	}
out:
	*p_dep_buffer  = visit.vd_deps.gd_vec;
	*p_dep_mask    = visit.vd_deps.gd_msk;
#ifdef CONFIG_GC_TRACK_LEAFS
	*p_leaf_buffer = visit.vd_leafs.gl_vec;
	*p_leaf_mask   = visit.vd_leafs.gl_msk;
#endif /* CONFIG_GC_TRACK_LEAFS */
	return result;
done:
	/* Always decref all remaining objects. */
#ifndef NDEBUG
	for (i = 0; i < visit.vd_deps.gd_cnt; ++i)
#else /* NDEBUG */
	for (i = 0; i <= visit.vd_deps.gd_msk; ++i)
#endif /* !NDEBUG */
	{
		if (!visit.vd_deps.gd_vec[i].gd_object)
			continue;
		Dee_Decref_untraced(visit.vd_deps.gd_vec[i].gd_object);
	}
	goto out;
}



/* Try to collect at most `max_objects' GC-objects,
 * returning the actual amount collected. */
PUBLIC size_t DCALL DeeGC_Collect(size_t max_objects) {
	struct Dee_gc_head *iter;
	size_t temp, result;
	struct gc_dep *dep_buffer;
	size_t dep_mask;
#ifdef CONFIG_GC_TRACK_LEAFS
	struct gc_leaf *leaf_buffer;
	size_t leaf_mask;
#endif /* CONFIG_GC_TRACK_LEAFS */
	result = 0;
	dep_buffer = static_visit_buffer;
	dep_mask   = COMPILER_LENOF(static_visit_buffer) - 1;
#ifdef CONFIG_GC_TRACK_LEAFS
	leaf_buffer = static_leaf_buffer;
	leaf_mask   = COMPILER_LENOF(static_leaf_buffer) - 1;
#endif /* CONFIG_GC_TRACK_LEAFS */
	if unlikely(!max_objects)
		goto done_nolock;
#ifdef CONFIG_HAVE_PENDING_GC_OBJECTS
collect_restart_with_pending_hint:
#endif /* CONFIG_HAVE_PENDING_GC_OBJECTS */
	GCLOCK_ACQUIRE_S();
restart:
	did_untrack = false;
	for (iter = gc_root; iter; iter = iter->gc_next) {
		ASSERT(iter != iter->gc_next);
#ifdef CONFIG_NO_THREADS
		if (iter->gc_object.ob_refcnt == 0)
			continue; /* The object may have just been decref()-ed by another thread,
			           * but that thread hadn't had the chance to untrack it, yet. */
		ASSERT_OBJECT(&iter->gc_object);
#else /* CONFIG_NO_THREADS */
		if (atomic_read(&iter->gc_object.ob_refcnt) == 0)
			continue; /* The object may have just been decref()-ed by another thread,
			           * but that thread hadn't had the chance to untrack it, yet. */
		/* This can (and has been seen to) fail when another thread is currently
		 * destroying the associated object, causing its refcnt to drop to zero
		 * NOTE: In the generic case, this is allowed to happen, since the destroying
		 *       thread will have to acquire `gc_lock' in order to untrack the object,
		 *       meaning that the thread has to wait for us to finish, and all we have
		 *       to do is to be able to deal with a dead-but-not-yet-destroyed-object,
		 *       as indicated by the object's refcnt having dropped to `0'
		 */
#if 0
		ASSERT_OBJECT(&iter->gc_object);
#else
		ASSERT_OBJECT(iter->gc_object.ob_type);
#endif
#endif /* !CONFIG_NO_THREADS */
#ifdef CONFIG_GC_CHECK_MEMORY
		Dee_CHECKMEMORY();
#endif /* CONFIG_GC_CHECK_MEMORY */
		temp = gc_trydestroy(iter,
		                     &dep_buffer,
		                     &dep_mask
#ifdef CONFIG_GC_TRACK_LEAFS
		                     ,
		                     &leaf_buffer,
		                     &leaf_mask
#endif /* CONFIG_GC_TRACK_LEAFS */
		                     );
#ifdef CONFIG_GC_CHECK_MEMORY
		Dee_CHECKMEMORY();
#endif /* CONFIG_GC_CHECK_MEMORY */
		if (temp) {
			if ((result += temp) >= max_objects) {
				GCLOCK_RELEASE_S();
				goto done_nolock;
			}
			/* Restart checking all objects, as we're unable to trace
			 * which objects were destroyed (any of which may be located
			 * anywhere else in the current gc_root-chain), meaning there'd
			 * be no way for us to safely determine where we were in the
			 * chain of gc-objects (mainly because that chain may have
			 * changed all-together at this point). */
			goto restart;
		}
		/* If something was untracked, restart because the chain may have be altered. */
		if (did_untrack)
			goto restart;
	}
	COMPILER_READ_BARRIER();
	GCLOCK_RELEASE();
#ifdef CONFIG_HAVE_PENDING_GC_OBJECTS
	if (GC_PENDING_MUSTSERVICE())
		goto collect_restart_with_pending_hint;
#endif /* CONFIG_HAVE_PENDING_GC_OBJECTS */
done_nolock:
	if (dep_buffer != static_visit_buffer)
		Dee_Free(dep_buffer);
#ifdef CONFIG_GC_TRACK_LEAFS
	if (leaf_buffer != static_leaf_buffer)
		Dee_Free(leaf_buffer);
#endif /* CONFIG_GC_TRACK_LEAFS */
	return result;
}

/* Return `true' if any GC objects with a non-zero reference
 * counter is being tracked.
 * NOTE: In addition, this function does not return `true' when
 *       all that's left are dex objects (which are destroyed
 *       at a later point during deemon shutdown, than the point
 *       when this function is called to determine if the GC must
 *       continue to run) */
INTERN bool DCALL DeeGC_IsEmptyWithoutDex(void) {
	struct Dee_gc_head *iter;
	GCLOCK_ACQUIRE_S();
	/* Also search the regular GC chain. */
	iter = gc_root;
	for (; iter; iter = iter->gc_next) {
		ASSERT(iter != iter->gc_next);
		if (!iter->gc_object.ob_refcnt)
			continue;
#ifndef CONFIG_NO_DEX
#ifdef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
		if (Dee_TYPE(&iter->gc_object) == &DeeModuleDex_Type)
			continue;
#else /* CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
		if (DeeDex_Check(&iter->gc_object))
			continue;
#endif /* !CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */
#endif /* !CONFIG_NO_DEX */
		goto is_nonempty;
	}
	GCLOCK_RELEASE_S();
	return true;
is_nonempty:
	GCLOCK_RELEASE_S();
	return false;
}


/* Used internally to prevent any further execution of user-code
 * and is called during `Dee_Shutdown()' when it becomes obvious
 * that deemon cannot be shutdown through conventional means.
 * To oversimplify such a situation, imagine user-code like this:
 * >> global global_instance;
 * >>
 * >> class SelfRevivingDestructor {
 * >>     ~this() {
 * >>         // Allowed: The destructor revives itself by generating
 * >>         //          a new reference to itself within some global,
 * >>         //          or otherwise outside variable.
 * >>         global_instance = this;
 * >>     }
 * >> }
 * >> // Kick-start the evil...
 * >> SelfRevivingDestructor();
 *
 * Now code like this might seem evil, however is 100% allowed, and even
 * when something like this is done, deemon will continue to try shutting
 * down normally, and without breaking any rules (just slightly altering
 * them in a way that remains consistent without introducing undefined
 * behavior).
 * How is this done? Well. If the name of this function hasn't
 * answered that yet, let me make it plain obvious:
 *   >> By killing all existing code objects.
 * This is quite the simple task as a matter of fact:
 *   - We use `tp_visit' to recursively visit all GC-objects,
 *     where for every `Code' object that we encounter, we
 *     simply do an atomic_write of the first instruction byte
 *     (unless the code is empty?), to set it to `ASM_RET_NONE'
 *   - `Code' objects (when in use) are referenced by `Function'
 *     objects, which are also GC objects, meaning that we can
 *     be sure that every existing piece of user-code can be
 *     reached by simply iterating all GC objects and filtering
 *     instances of `DeeFunction_Type'.
 *     >> import deemon, rt;
 *     >> for (local x: deemon.gc) {
 *     >>     if (x !is rt.Function)
 *     >>         continue;
 *     >>     local code = x.__code__;
 *     >>     ...
 *     >> }
 *   - That might seem dangerous, but consider the implications:
 *      - Assuming that the caller will continue to use `DeeThread_InterruptAndJoinAll()'
 *        to join any new threads that may appear, we can also assume
 *        that any running user-code function will eventually return
 *        to its caller. However: any further calls to other user-code
 *        functions will immediately return `none' (or throw StopIteration),
 *        meaning that while existing usercode can finish execution
 *        (unless it's assembly text describes a loop that jumps back
 *        to code address +0000, in which case the loop will terminate
 *        the next time it tries to wrap around), no new functions can
 *        be started (unless user-code _really_ _really_ tries super-hard
 *        to counteract this by somehow managing to re-compile itself?)
 *      - The caller (`Dee_Shutdown()') will have joined all existing
 *        threads, meaning that no user-code can still be running, also
 *        meaning that we are safe to modify any existing piece of user-text
 *        that might exist. (Otherwise, we'd have to do some complicated
 *        trickery by taking `DeeThread_SuspendAll()' into account)
 *   - Anyways. This way (assuming that all C code is free of reference leaks),
 *     it should be possible to stop intentional reference loops (such as
 *     the one caused by the persistent instance of `SelfRevivingDestructor'
 *     in the example above), still allowing ~actual~ destructors to do
 *     their work prior to this function being used to give the user a slap
 *     on the wrists, while all C-level cleanup functions can continue to
 *     operate normally and do all the cleanup that we can trust (or at
 *     least hope to be able to trust) not to cause something like this.
 * @return: * : The amount of code objects that were affected. */
INTERN size_t DCALL DeeExec_KillUserCode(void) {
	struct Dee_gc_head *iter;
	size_t result = 0;
#ifdef CONFIG_HAVE_PENDING_GC_OBJECTS
collect_restart_with_pending_hint:
#endif /* CONFIG_HAVE_PENDING_GC_OBJECTS */
	GCLOCK_ACQUIRE_S();
	for (iter = gc_root; iter; iter = iter->gc_next) {
		instruction_t old_instr;
		uint16_t old_flags;
		DeeCodeObject *iter_code;
		DeeFunctionObject *iter_function;
		ASSERT_OBJECT(&iter->gc_object);
		ASSERT(iter != iter->gc_next);
		if unlikely(!iter->gc_object.ob_refcnt)
			continue; /* Object is currently being destroyed. */
		if (!DeeFunction_Check(&iter->gc_object))
			continue; /* Not a function object. */
		iter_function = (DeeFunctionObject *)&iter->gc_object;
		iter_code     = iter_function->fo_code;
		if (!iter_code->co_codebytes)
			continue; /* Empty code? */

		/* Exchange the first instruction with `ASM_RET_NONE' */
		old_instr = atomic_xch(&iter_code->co_code[0], ASM_RET_NONE);

		/* One last thing: The interpreter checks the FFINALLY flag
		 *                 to see if there might be finally-handlers
		 *                 that must be invoked after a return-instruction
		 *                 has been encountered.
		 *              -- Delete that flag! We can't have the code
		 *                 re-acquiring control, simply by pre-defining
		 *                 a finally handler to guard the first text byte. */
		old_flags = atomic_fetchand(&iter_code->co_flags, ~Dee_CODE_FFINALLY);
		if (old_instr != ASM_RET_NONE || (old_flags & Dee_CODE_FFINALLY))
			++result;
	}
	GCLOCK_RELEASE();
#ifdef CONFIG_HAVE_PENDING_GC_OBJECTS
	COMPILER_READ_BARRIER();
	if (GC_PENDING_MUSTSERVICE())
		goto collect_restart_with_pending_hint;
#endif /* CONFIG_HAVE_PENDING_GC_OBJECTS */
	return result;
}

/* GC objects referring to X */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeGC_CollectGCReferred(GCSetMaker *__restrict self,
                        DeeObject *__restrict target) {
	struct Dee_gc_head *iter;
again:
	if unlikely(GCLOCK_ACQUIRE_READ())
		goto err;
	for (iter = gc_root; iter; iter = iter->gc_next) {
		DREF DeeObject *obj;
		ASSERT(iter != iter->gc_next);
		obj = &iter->gc_object;
		if (!Dee_IncrefIfNotZero(obj))
			continue;
		if (DeeGC_ReferredBy(obj, target)) {
			int error = GCSetMaker_Insert(self, obj);
			if (error == 0)
				continue;
			if (error > 0)
				goto decref_obj;
			GCLOCK_RELEASE_READ();
			Dee_Decref_unlikely(obj);
			if (Dee_CollectMemory(self->gs_err))
				goto again;
			goto err;
		} else {
decref_obj:
			if (!Dee_DecrefIfNotOne(obj)) {
				GCLOCK_RELEASE_READ();
				Dee_Decref_likely(obj);
				goto again;
			}
		}
	}
	GCLOCK_RELEASE_READ();
	return 0;
err:
	return -1;
}

typedef struct {
	OBJECT_HEAD
	DREF DeeObject   *gi_next; /* [0..1][lock(gi_lock)]
	                            * The next GC object to-be iterated, or
	                            * NULL when the iterator has been exhausted. */
#ifndef CONFIG_NO_THREADS
	Dee_atomic_lock_t gi_lock; /* Lock for `gi_next' */
#endif /* !CONFIG_NO_THREADS */
} GCIter;

#define GCIter_LockAvailable(self)  Dee_atomic_lock_available(&(self)->gi_lock)
#define GCIter_LockAcquired(self)   Dee_atomic_lock_acquired(&(self)->gi_lock)
#define GCIter_LockTryAcquire(self) Dee_atomic_lock_tryacquire(&(self)->gi_lock)
#define GCIter_LockAcquire(self)    Dee_atomic_lock_acquire(&(self)->gi_lock)
#define GCIter_LockWaitFor(self)    Dee_atomic_lock_waitfor(&(self)->gi_lock)
#define GCIter_LockRelease(self)    Dee_atomic_lock_release(&(self)->gi_lock)

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
gciter_copy(GCIter *__restrict self,
            GCIter *__restrict other) {
	GCIter_LockAcquire(other);
	self->gi_next = other->gi_next;
	Dee_XIncref(self->gi_next);
	GCIter_LockRelease(other);
	Dee_atomic_lock_init(&self->gi_lock);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
gciter_serialize(GCIter *__restrict self,
                 DeeSerial *__restrict writer,
                 Dee_seraddr_t addr) {
#define ADDROF(field) (addr + offsetof(GCIter, field))
	DREF DeeObject *self__gi_next;
	GCIter_LockAcquire(self);
	self__gi_next = self->gi_next;
	Dee_XIncref(self__gi_next);
	GCIter_LockRelease(self);
	Dee_atomic_lock_init(&DeeSerial_Addr2Mem(writer, addr, GCIter)->gi_lock);
	return DeeSerial_XPutObjectInherited(writer, ADDROF(gi_next), self__gi_next);
#undef ADDROF
}

PRIVATE NONNULL((1)) void DCALL
gciter_fini(GCIter *__restrict self) {
	Dee_XDecref(self->gi_next);
}

PRIVATE NONNULL((1, 2)) void DCALL
gciter_visit(GCIter *__restrict self, Dee_visit_t proc, void *arg) {
	Dee_XVisit(self->gi_next);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
gciter_bool(GCIter *__restrict self) {
	return atomic_read(&self->gi_next) != NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
gciter_next(GCIter *__restrict self) {
	DREF DeeObject *result;
	struct Dee_gc_head *next;
	GCIter_LockAcquire(self);
	result = self->gi_next; /* Inherit reference. */
	if unlikely(!result) {
		/* Iterator has been exhausted. */
		GCIter_LockRelease(self);
		return ITER_DONE;
	}
	if unlikely(GCLOCK_ACQUIRE_READ())
		goto err_unlock_iter;

	/* Skip ZERO-ref entries. */
	next = DeeGC_Head(result)->gc_next;
	ASSERT(DeeGC_Head(result) != next);

	/* Find the next object that we can actually incref()
	 * (The GC chain may contain dangling (aka. weak) objects) */
	while (next && !Dee_IncrefIfNotZero(&next->gc_object)) {
		ASSERT(next != next->gc_next);
		next = next->gc_next;
	}
	GCLOCK_RELEASE_READ();
	self->gi_next = next ? &next->gc_object : NULL; /* Inherit reference. */
	GCIter_LockRelease(self);

	/* Return the extracted item. */
	return result;
err_unlock_iter:
	GCIter_LockRelease(self);
	return NULL;
}


PRIVATE struct type_member tpconst gciter_members[] = {
	TYPE_MEMBER_CONST(STR_seq, &DeeGCEnumTracked_Singleton),
	TYPE_MEMBER_END
};

PRIVATE DeeTypeObject GCIter_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_GCIter",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ GCIter,
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ &gciter_copy,
			/* tp_any_ctor:    */ NULL,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &gciter_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&gciter_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&gciter_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&iterator_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&gciter_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__EFED4BCD35433C3C),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__27F47A9BEBC0B992),
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&gciter_next,
	/* .tp_iterator      = */ DEFIMPL(&default__tp_iterator__712535FF7E4C26E5),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ gciter_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL(&iterator_next),
	/* .tp_callable      = */ DEFIMPL(&default__tp_callable__83C59FA7626CABBE),
};



PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
gcenum_iter(DeeObject *__restrict UNUSED(self)) {
	DREF GCIter *result;
	struct Dee_gc_head *first;
	result = DeeObject_MALLOC(GCIter);
	if unlikely(!result)
		goto done;
	if unlikely(GCLOCK_ACQUIRE_READ())
		goto err_r;
	first = gc_root;
	/*  Find the first object that we can actually incref()
	 * (The GC chain may contain dangling (aka. weak) objects) */
	while (first && !Dee_IncrefIfNotZero(&first->gc_object)) {
		ASSERT(first != first->gc_next);
		first = first->gc_next;
	}
	GCLOCK_RELEASE_READ();
	Dee_atomic_lock_init(&result->gi_lock);
	/* Save the first object in the iterator. */
	result->gi_next = first ? &first->gc_object : NULL;
	DeeObject_Init(result, &GCIter_Type);
done:
	return Dee_AsObject(result);
err_r:
	DeeObject_FREE(result);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
gcenum_size(DeeObject *__restrict UNUSED(self)) {
	size_t result = 0;
	struct Dee_gc_head *iter;
	if unlikely(GCLOCK_ACQUIRE_READ())
		goto err;
	for (iter = gc_root; iter; iter = iter->gc_next) {
		ASSERT(iter != iter->gc_next);
		++result;
	}
	GCLOCK_RELEASE_READ();
	return DeeInt_NewSize(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
gcenum_contains(DeeObject *__restrict UNUSED(self),
                DeeObject *__restrict ob) {
	if (!DeeType_IsGC(Dee_TYPE(ob)))
		return_false;
#if defined(GCHEAD_ISTRACKED)
	return_bool(GCHEAD_ISTRACKED(DeeGC_Head(ob)));
#else /* GCHEAD_ISTRACKED */
	{
		struct Dee_gc_head *iter;
		if unlikely(GCLOCK_ACQUIRE_READ())
			goto err;
		for (iter = gc_root; iter; iter = iter->gc_next) {
			ASSERT(iter != iter->gc_next);
			if (&iter->gc_object == ob) {
				GCLOCK_RELEASE_READ();
				return_true;
			}
		}
		GCLOCK_RELEASE_READ();
	}
	return_false;
err:
	return NULL;
#endif /* !GCHEAD_ISTRACKED */
}

PRIVATE struct type_seq gcenum_seq = {
	/* .tp_iter     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&gcenum_iter,
	/* .tp_sizeob   = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&gcenum_size,
	/* .tp_contains = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&gcenum_contains,
	/* .tp_getitem                    = */ DEFIMPL(&default__seq_operator_getitem__with__seq_operator_getitem_index),
	/* .tp_delitem                    = */ DEFIMPL(&default__seq_operator_delitem__unsupported),
	/* .tp_setitem                    = */ DEFIMPL(&default__seq_operator_setitem__unsupported),
	/* .tp_getrange                   = */ DEFIMPL(&default__seq_operator_getrange__with__seq_operator_getrange_index__and__seq_operator_getrange_index_n),
	/* .tp_delrange                   = */ DEFIMPL(&default__seq_operator_delrange__unsupported),
	/* .tp_setrange                   = */ DEFIMPL(&default__seq_operator_setrange__unsupported),
	/* .tp_foreach                    = */ DEFIMPL(&default__foreach__with__iter),
	/* .tp_foreach_pair               = */ DEFIMPL(&default__foreach_pair__with__iter),
	/* .tp_bounditem                  = */ DEFIMPL(&default__seq_operator_bounditem__with__seq_operator_getitem),
	/* .tp_hasitem                    = */ DEFIMPL(&default__seq_operator_hasitem__with__seq_operator_sizeob),
	/* .tp_size                       = */ DEFIMPL(&default__size__with__sizeob),
	/* .tp_size_fast                  = */ NULL,
	/* .tp_getitem_index              = */ DEFIMPL(&default__seq_operator_getitem_index__with__seq_operator_foreach),
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ DEFIMPL(&default__seq_operator_delitem_index__unsupported),
	/* .tp_setitem_index              = */ DEFIMPL(&default__seq_operator_setitem_index__unsupported),
	/* .tp_bounditem_index            = */ DEFIMPL(&default__seq_operator_bounditem_index__with__seq_operator_getitem_index),
	/* .tp_hasitem_index              = */ DEFIMPL(&default__seq_operator_hasitem_index__with__seq_operator_size),
	/* .tp_getrange_index             = */ DEFIMPL(&default__seq_operator_getrange_index__with__seq_operator_size__and__seq_operator_iter),
	/* .tp_delrange_index             = */ DEFIMPL(&default__seq_operator_delrange_index__unsupported),
	/* .tp_setrange_index             = */ DEFIMPL(&default__seq_operator_setrange_index__unsupported),
	/* .tp_getrange_index_n           = */ DEFIMPL(&default__seq_operator_getrange_index_n__with__seq_operator_size__and__seq_operator_iter),
	/* .tp_delrange_index_n           = */ DEFIMPL(&default__seq_operator_delrange_index_n__unsupported),
	/* .tp_setrange_index_n           = */ DEFIMPL(&default__seq_operator_setrange_index_n__unsupported),
	/* .tp_trygetitem                 = */ DEFIMPL(&default__seq_operator_trygetitem__with__seq_operator_trygetitem_index),
	/* .tp_trygetitem_index           = */ DEFIMPL(&default__seq_operator_trygetitem_index__with__seq_operator_foreach),
	/* .tp_trygetitem_string_hash     = */ DEFIMPL(&default__trygetitem_string_hash__with__trygetitem),
	/* .tp_getitem_string_hash        = */ DEFIMPL(&default__getitem_string_hash__with__getitem),
	/* .tp_delitem_string_hash        = */ DEFIMPL(&default__delitem_string_hash__with__delitem),
	/* .tp_setitem_string_hash        = */ DEFIMPL(&default__setitem_string_hash__with__setitem),
	/* .tp_bounditem_string_hash      = */ DEFIMPL(&default__bounditem_string_hash__with__bounditem),
	/* .tp_hasitem_string_hash        = */ DEFIMPL(&default__hasitem_string_hash__with__hasitem),
	/* .tp_trygetitem_string_len_hash = */ DEFIMPL(&default__trygetitem_string_len_hash__with__trygetitem),
	/* .tp_getitem_string_len_hash    = */ DEFIMPL(&default__getitem_string_len_hash__with__getitem),
	/* .tp_delitem_string_len_hash    = */ DEFIMPL(&default__delitem_string_len_hash__with__delitem),
	/* .tp_setitem_string_len_hash    = */ DEFIMPL(&default__setitem_string_len_hash__with__setitem),
	/* .tp_bounditem_string_len_hash  = */ DEFIMPL(&default__bounditem_string_len_hash__with__bounditem),
	/* .tp_hasitem_string_len_hash    = */ DEFIMPL(&default__hasitem_string_len_hash__with__hasitem),
};

PRIVATE struct type_member tpconst gcenum_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &GCIter_Type),
	TYPE_MEMBER_END
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
gcenum_collect(DeeObject *UNUSED(self),
               size_t argc, DeeObject *const *argv) {
	size_t result;
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("collect", params: """
	size_t max = (size_t)-1;
""", docStringPrefix: "gcenum");]]]*/
#define gcenum_collect_params "max=!-1"
	struct {
		size_t max_;
	} args;
	args.max_ = (size_t)-1;
	DeeArg_Unpack0Or1X(err, argc, argv, "collect", &args.max_, UNPxSIZ, DeeObject_AsSizeM1);
/*[[[end]]]*/
	result = DeeGC_Collect(args.max_);
	return DeeInt_NewSize(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
gcenum_referred(DeeObject *UNUSED(self),
                size_t argc, DeeObject *const *argv) {
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("referred", params: """
	DeeObject *start;
""", docStringPrefix: "gcenum");]]]*/
#define gcenum_referred_params "start"
	struct {
		DeeObject *start;
	} args;
	DeeArg_Unpack1(err, argc, argv, "referred", &args.start);
/*[[[end]]]*/
	return Dee_AsObject(DeeGC_NewReferred(args.start));
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
gcenum_referredgc(DeeObject *UNUSED(self),
                  size_t argc, DeeObject *const *argv) {
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("referredgc", params: """
	DeeObject *start;
""", docStringPrefix: "gcenum");]]]*/
#define gcenum_referredgc_params "start"
	struct {
		DeeObject *start;
	} args;
	DeeArg_Unpack1(err, argc, argv, "referredgc", &args.start);
/*[[[end]]]*/
	return Dee_AsObject(DeeGC_NewReferredGC(args.start));
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
gcenum_reachable(DeeObject *UNUSED(self),
                 size_t argc, DeeObject *const *argv) {
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("reachable", params: """
	DeeObject *start;
""", docStringPrefix: "gcenum");]]]*/
#define gcenum_reachable_params "start"
	struct {
		DeeObject *start;
	} args;
	DeeArg_Unpack1(err, argc, argv, "reachable", &args.start);
/*[[[end]]]*/
	return Dee_AsObject(DeeGC_NewReachable(args.start));
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
gcenum_reachablegc(DeeObject *UNUSED(self),
                   size_t argc, DeeObject *const *argv) {
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("reachablegc", params: """
	DeeObject *start;
""", docStringPrefix: "gcenum");]]]*/
#define gcenum_reachablegc_params "start"
	struct {
		DeeObject *start;
	} args;
	DeeArg_Unpack1(err, argc, argv, "reachablegc", &args.start);
/*[[[end]]]*/
	return Dee_AsObject(DeeGC_NewReachableGC(args.start));
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
gcenum_referring(DeeObject *UNUSED(self),
                 size_t argc, DeeObject *const *argv) {
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("referring", params: """
	DeeObject *to;
""", docStringPrefix: "gcenum");]]]*/
#define gcenum_referring_params "to"
	struct {
		DeeObject *to;
	} args;
	DeeArg_Unpack1(err, argc, argv, "referring", &args.to);
/*[[[end]]]*/
	return Dee_AsObject(DeeGC_NewGCReferred(args.to));
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
gcenum_isreferring(DeeObject *UNUSED(self),
                   size_t argc, DeeObject *const *argv) {
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("isreferring", params: """
	DeeObject *from;
	DeeObject *to;
""", docStringPrefix: "gcenum");]]]*/
#define gcenum_isreferring_params "from,to"
	struct {
		DeeObject *from;
		DeeObject *to;
	} args;
	DeeArg_UnpackStruct2(err, argc, argv, "isreferring", &args, &args.from, &args.to);
/*[[[end]]]*/
	return_bool(DeeGC_ReferredBy(args.from, args.to));
err:
	return NULL;
}


PRIVATE struct type_method tpconst gcenum_methods[] = {
	TYPE_METHOD_F("collect", &gcenum_collect, METHOD_FNOREFESCAPE,
	              "(" gcenum_collect_params ")->?Dint\n"
	              "Try to collect at most @max GC objects and return the actual number collected\n"
	              "Note that more than @max objects may be collected if sufficiently large reference cycles exist"),
	TYPE_METHOD_F("referred", &gcenum_referred, METHOD_FNOREFESCAPE,
	              "(" gcenum_referred_params ")->?DSet\n"
	              "Returns a set of objects that are immediately referred to by @start"),
	TYPE_METHOD_F("referredgc", &gcenum_referredgc, METHOD_FNOREFESCAPE,
	              "(" gcenum_referredgc_params ")->?DSet\n"
	              "Same as ?#referred, but only include gc-objects (s.a. :Type.__isgc__)"),
	TYPE_METHOD_F("reachable", &gcenum_reachable, METHOD_FNOREFESCAPE,
	              "(" gcenum_reachable_params ")->?DSet\n"
	              "Returns a set of objects that are reachable from @start"),
	TYPE_METHOD_F("reachablegc", &gcenum_reachablegc, METHOD_FNOREFESCAPE,
	              "(" gcenum_reachablegc_params ")->?DSet\n"
	              "Same as ?#reachable, but only include gc-objects (s.a. :Type.__isgc__)"),
	TYPE_METHOD_F("referring", &gcenum_referring, METHOD_FNOREFESCAPE,
	              "(" gcenum_referring_params ")->?DSet\n"
	              "Returns a set of gc-objects (s.a. :Type.__isgc__) that are referring to @to"),
	TYPE_METHOD_F("isreferring", &gcenum_isreferring, METHOD_FNOREFESCAPE,
	              "(" gcenum_isreferring_params ")->?Dbool\n"
	              "Returns ?t if @to is referred to by @from, or ?f otherwise"),
	TYPE_METHOD_END
};

PRIVATE DeeTypeObject GCEnum_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_GCEnum",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL | TP_FABSTRACT,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_SINGLETON,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED_S(
			/* T:              */ DeeObject,
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ NULL,
			/* tp_any_ctor:    */ NULL,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ NULL /* Static singleton, so no serial needed */
		),
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ DEFIMPL(&default__seq_operator_bool__with__seq_operator_sizeob),
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&default_seq_printrepr),
	},
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__6AAE313158D20BA0),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__96C7A3207FAE93E2),
	/* .tp_seq           = */ &gcenum_seq,
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__C6F8E138F179B5AD),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ gcenum_methods,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ gcenum_class_members,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
};


/* An generic sequence singleton that can be
 * iterated to yield all tracked GC objects.
 * This object also offers a hand full of member functions
 * that user-space an invoke to trigger various GC-related
 * functionality:
 *   - collect(max: int = -1): int;
 * Also: remember that this derives from `Sequence', so you
 *       can use all its attributes, like `empty', etc.
 * NOTE: This object is exported as `gc from deemon' */
PUBLIC DeeObject DeeGCEnumTracked_Singleton = {
	OBJECT_HEAD_INIT(&GCEnum_Type)
};
#endif /* !CONFIG_EXPERIMENTAL_REWORKED_GC */


/* GC object alloc/free. */
LOCAL void *gc_initob(void *ptr) {
	if likely(ptr) {
		/* NOTE: This `DBG_memset()' is important for `GCHEAD_ISTRACKED()' to work properly!
		 * The 0xcc pattern set here is checked for by `GCHEAD_ISTRACKED()', and if found:
		 * treated as meaning that the associated object isn't being tracked. */
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

DFUNDEF ATTR_MALLOC WUNUSED void *
(DCALL DeeDbgGCObject_Malloc)(size_t n_bytes, char const *file, int line) {
	size_t whole_size;
	if unlikely(OVERFLOW_UADD(Dee_GC_HEAD_SIZE, n_bytes, &whole_size))
		whole_size = (size_t)-1;
	return gc_initob((DeeDbgObject_Malloc)(whole_size, file, line));
}

DFUNDEF ATTR_MALLOC WUNUSED void *
(DCALL DeeDbgGCObject_Calloc)(size_t n_bytes, char const *file, int line) {
	size_t whole_size;
	if unlikely(OVERFLOW_UADD(Dee_GC_HEAD_SIZE, n_bytes, &whole_size))
		whole_size = (size_t)-1;
	return gc_initob((DeeDbgObject_Calloc)(whole_size, file, line));
}

DFUNDEF WUNUSED void *
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

DFUNDEF ATTR_MALLOC WUNUSED void *
(DCALL DeeDbgGCObject_TryMalloc)(size_t n_bytes, char const *file, int line) {
	size_t whole_size;
	if unlikely(OVERFLOW_UADD(Dee_GC_HEAD_SIZE, n_bytes, &whole_size))
		whole_size = (size_t)-1;
	return gc_initob((DeeDbgObject_TryMalloc)(whole_size, file, line));
}

DFUNDEF ATTR_MALLOC WUNUSED void *
(DCALL DeeDbgGCObject_TryCalloc)(size_t n_bytes, char const *file, int line) {
	size_t whole_size;
	if unlikely(OVERFLOW_UADD(Dee_GC_HEAD_SIZE, n_bytes, &whole_size))
		whole_size = (size_t)-1;
	return gc_initob((DeeDbgObject_TryCalloc)(whole_size, file, line));
}

DFUNDEF WUNUSED void *
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

DFUNDEF void
(DCALL DeeDbgGCObject_Free)(void *p, char const *file, int line) {
	if (p) {
		ASSERT_UNTRACKED(p);
		(DeeDbgObject_Free)(DeeGC_Head((DeeObject *)p), file, line);
	}
}


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


#ifndef NDEBUG
#ifdef CONFIG_TRACE_REFCHANGES
INTDEF NONNULL((1)) void DCALL
dump_reference_history(DeeObject *__restrict obj);
#endif /* !CONFIG_TRACE_REFCHANGES */

#ifdef CONFIG_EXPERIMENTAL_REWORKED_GC

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
#else /* CONFIG_EXPERIMENTAL_REWORKED_GC */
INTERN void DCALL gc_dump_all_except_dex(void) {
	struct Dee_gc_head *iter;
	for (iter = gc_root; iter; iter = iter->gc_next) {
		DeeObject *ob = &iter->gc_object;
		DeeTypeObject *ob_type;
		ASSERT(iter != iter->gc_next);
		ob_type = ob->ob_type;
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
		/* Print the name of a select set of types. */
		gc_dprint_object_info(ob_type, ob);
		Dee_DPRINT("\n");
#ifdef CONFIG_TRACE_REFCHANGES
		dump_reference_history(&iter->gc_object);
#endif /* CONFIG_TRACE_REFCHANGES */
	}
}
#endif /* !CONFIG_EXPERIMENTAL_REWORKED_GC */
#endif /* !NDEBUG */

DECL_END

#ifdef CONFIG_EXPERIMENTAL_REWORKED_GC
#ifndef __INTELLISENSE__
#include "gc-inspect.c.inl"
#endif /* !__INTELLISENSE__ */
#endif /* CONFIG_EXPERIMENTAL_REWORKED_GC */

#endif /* !GUARD_DEEMON_RUNTIME_GC_C */
