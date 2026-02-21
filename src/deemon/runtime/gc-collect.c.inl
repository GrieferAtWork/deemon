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
#ifdef __INTELLISENSE__
#include "gc.c"
#define DEFINE_gc_generation_collect_or_unlock
//#define DEFINE_gc_collectall_collect_or_unlock
#endif /* __INTELLISENSE__ */

#include <deemon/api.h>

#include <deemon/format.h>       /* PRFuSIZ */
#include <deemon/gc.h>           /* DeeGC_Head, DeeGC_TRACK_F_NOCOLLECT, Dee_GC_*, Dee_gc_head */
#include <deemon/object.h>       /* DeeObject_Visit, Dee_Decref */
#include <deemon/thread.h>       /* DeeThread_IsMultiThreaded */
#include <deemon/type.h>         /* DeeType_GetName */
#include <deemon/types.h>        /* DeeObject, Dee_TYPE, Dee_refcnt_t */
#include <deemon/util/atomic.h>  /* atomic_* */
#include <deemon/util/weakref.h> /* Dee_weakref_list_kill_dummy */

#include <hybrid/compiler.h>

#include <stdbool.h> /* bool, false */
#include <stddef.h>  /* NULL, size_t */
#include <stdint.h>  /* uintptr_t */

#if (defined(DEFINE_gc_generation_collect_or_unlock) + \
     defined(DEFINE_gc_collectall_collect_or_unlock)) != 1
#error "Must #define exactly one of these"
#endif /* ... */

DECL_BEGIN

#ifdef DEFINE_gc_generation_collect_or_unlock
#define LOCAL_gc_visit__decref__cb                      gc_visit__decref__cb
#define LOCAL_gc_visit__incref__with_weakref_detect__cb gc_visit__incref__with_weakref_detect__cb
#define LOCAL_gc_visit__incref__with_weakref_kill__cb   gc_visit__incref__with_weakref_kill__cb
#elif defined(DEFINE_gc_collectall_collect_or_unlock)
#define LOCAL_gc_visit__decref__cb                      gc_visit__decref_all__cb
#define LOCAL_gc_visit__incref__with_weakref_detect__cb gc_visit__incref_all__with_weakref_detect__cb
#define LOCAL_gc_visit__incref__with_weakref_kill__cb   gc_visit__incref_all__with_weakref_kill__cb
#endif /* ... */


/* Heart-piece of GC collect
 * @return: * : One of `GC_GENERATION_COLLECT_OR_UNLOCK__*' */
#ifdef DEFINE_gc_generation_collect_or_unlock
PRIVATE ATTR_NOINLINE WUNUSED NONNULL((1, 2)) unsigned int DCALL
gc_generation_collect_or_unlock(struct gc_generation *__restrict gen,
                                size_t *__restrict p_num_collected,
                                unsigned int flags)
#elif defined(DEFINE_gc_collectall_collect_or_unlock)
PRIVATE ATTR_NOINLINE WUNUSED NONNULL((1)) unsigned int DCALL
gc_collectall_collect_or_unlock(size_t *__restrict p_num_collected)
#endif /* ... */
{
#ifdef DEFINE_gc_collectall_collect_or_unlock
	struct gc_generation *gen;
#define LOCAL_foreach_generation() for (gen = &gc_gen0; gen; gen = gen->gg_next)
#else /* DEFINE_gc_collectall_collect_or_unlock */
#define LOCAL_foreach_generation() if (0); else
#endif /* !DEFINE_gc_collectall_collect_or_unlock */
	DeeObject *iter, **p_iter;
	DeeObject *unreachable_slow;
	DeeObject *unreachable_fast;
	size_t num_reachable, num_unreachable;
	bool must_kill_weakrefs;
	bool must_kill_nested_weakrefs;

#ifdef DEFINE_gc_generation_collect_or_unlock
	if unlikely(!gen->gg_objects) {
		gen->gg_collect_on = gen->gg_mindim;
		return GC_GENERATION_COLLECT_OR_UNLOCK__NOTHING;
	}

	if (gen == &gc_gen0) {
		/* Flags are already in the expected state:
		 * - "Dee_GC_FLAG_OTHERGEN" is clear...
		 * - ... because it's the same bit as "Dee_GC_FLAG_GENN"...
		 * - ... which is clear because we're not GENN (but gen#0) */
	} else {
		sithrd_gc_generation_set__GC_FLAG_OTHERGEN(&gc_gen0);
		sithrd_gc_generation_clear__GC_FLAG_OTHERGEN(gen);
	}
#endif /* DEFINE_gc_generation_collect_or_unlock */

	/* Perform scan and adjust "ob_refcnt" such that anything that is only
	 * reachable from other GC objects ends up with "ob_refcnt == 0". */
	LOCAL_foreach_generation() {
		for (iter = gen->gg_objects; iter;) {
			struct Dee_gc_head *head = DeeGC_Head(iter);
			ASSERT(gc_remove_modifying == 0);
			DeeObject_Visit(iter, &LOCAL_gc_visit__decref__cb, NULL);
			iter = head->gc_next;
		}
	}

	/* Identify objects that may potentially be deletable. */
	must_kill_weakrefs = false;
	LOCAL_foreach_generation() {
		for (iter = gen->gg_objects; iter;) {
			struct Dee_gc_head *head = DeeGC_Head(iter);
			ASSERT(gc_remove_modifying == 0);
			if (iter->ob_refcnt == 0) {
				/* Temporary flag that means "object is unreachable" */
				head->gc_next = (DeeObject *)((uintptr_t)head->gc_next | 1);
				if (!must_kill_weakrefs)
					must_kill_weakrefs = sithrd_DeeObject_HasWeakrefs(iter);
			}
			iter = (DeeObject *)((uintptr_t)head->gc_next & ~1);
		}
	}

	/* Anything that was identified as unreachable above, but can
	 * be reached by something that was identified as reachable
	 * actually isn't unreachable (it's merely *only* referenced
	 * by other, reachable GC objects)
	 *
	 * Or to word it different:
	 * - The above "decref" scan identified all objects as unreachable
	 *   that aren't **EXTERNALLY** referenced (iow: don't have references
	 *   that can be explained by other GC objects referencing them)
	 * - This here filters out objects that **INTERNALLY** referenced by
	 *   other GC objects that in turn have been identified as **EXTERNALLY**
	 *   visible:
	 *   >> class MyClass1 { };
	 *   >> class MyClass2 { this = default; public member foo: MyClass1; };
	 *   >>
	 *   >> local x = MyClass1();
	 *   >> local y = MyClass2(foo: x);
	 *   >> del x;
	 *   >> // - "x" is no longer EXTERNALLY referenced, so the decref scan
	 *   >> //   will have identified it as a potentially unreachable object
	 *   >> //   (because it has "ob_refcnt == 1", and that reference can be
	 *   >> //   explained by the other GC object "y")
	 *   >> // - But because "y" is reachable (none of its references can be
	 *   >> //   explained by other GC objects), anything **it** references
	 *   >> //   is therefor actually also reachable (which includes "x",
	 *   >> //   even though "x" isn't **EXTERNALLY** referenced)
	 *   >> gc.collect();
	 */
	LOCAL_foreach_generation() {
		for (iter = gen->gg_objects; iter;) {
			struct Dee_gc_head *head = DeeGC_Head(iter);
			ASSERT(gc_remove_modifying == 0);
			if (iter->ob_refcnt != 0)
				DeeObject_Visit(iter, &gc_visit__mark_internally_reachable__cb, NULL);
			iter = (DeeObject *)((uintptr_t)head->gc_next & ~1);
		}
	}

	/* Undo refcnt changes done by previous "DeeObject_Visit" */
	must_kill_nested_weakrefs = false;
	LOCAL_foreach_generation() {
		for (iter = gen->gg_objects; iter;) {
			struct Dee_gc_head *head = DeeGC_Head(iter);
			DeeObject_Visit(iter, &LOCAL_gc_visit__incref__with_weakref_detect__cb,
			                  &must_kill_nested_weakrefs);
			iter = (DeeObject *)((uintptr_t)head->gc_next & ~1);
		}
	}

	/* Sort unreachable objects into "unreachable_fast" and "unreachable_slow",
	 * or move to next generation / keep in current generation. At this point
	 * everything unreachable is identifiable via "gc_next & 1" */
	unreachable_slow = NULL;
	unreachable_fast = NULL;
	num_reachable    = 0;
	LOCAL_foreach_generation() {
		for (p_iter = &gen->gg_objects; (iter = *p_iter) != NULL;) {
			struct Dee_gc_head *head = DeeGC_Head(iter);
			ASSERT(!((uintptr_t)iter & 1));
			ASSERT(((uintptr_t)sithrd_atomic_read(&head->gc_info.gi_pself) & ~Dee_GC_FLAG_MASK) == (uintptr_t)p_iter);
			if ((uintptr_t)head->gc_next & 1) {
				/* Unreachable object. */
				DeeObject **p_unreachable_x;
				DeeObject *next = (DeeObject *)((uintptr_t)head->gc_next & ~1);
	
				/* Remove from own generation */
				if ((*p_iter = next) != NULL) {
					struct Dee_gc_head *next_head = DeeGC_Head(next);
					sithrd_gc_object_set_pself(&next_head->gc_info.gi_pself, p_iter);
				}
				--gen->gg_count;
	
				/* Check if object requires invocation of a "tp_finalize" operator... */
				if ((sithrd_atomic_read(&head->gc_info.gi_flag) & Dee_GC_FLAG_FINALIZED) ||
				    (!DeeType_HasFinalize(Dee_TYPE(iter)))) {
					p_unreachable_x = &unreachable_fast; /* Object can be fast-destroyed! */
				} else {
					p_unreachable_x = &unreachable_slow; /* Object must be slow-destroyed */
				}
				sithrd_gc_object_set_pself(&head->gc_info.gi_pself, p_unreachable_x);
				if ((head->gc_next = *p_unreachable_x) != NULL) {
					struct Dee_gc_head *next_head = DeeGC_Head(*p_unreachable_x);
					sithrd_gc_object_set_pself(&next_head->gc_info.gi_pself, &head->gc_next);
				}
				*p_unreachable_x = iter;
			} else {
				/* Still reachable */
				++num_reachable;
#ifndef DEFINE_gc_collectall_collect_or_unlock
				if (flags & GC_GENERATION_COLLECT_OR_UNLOCK__F_MOVE_REACHABLE) {
					/* Move to next generation */
					struct gc_generation *target;
	
					/* Remove from own generation */
					if ((*p_iter = head->gc_next) != NULL) {
						struct Dee_gc_head *next_head = DeeGC_Head(head->gc_next);
						sithrd_gc_object_set_pself(&next_head->gc_info.gi_pself, p_iter);
					}
					ASSERT(gen->gg_count);
					--gen->gg_count;
	
					/* Add to next generation */
					target = gen->gg_next;
					ASSERT(target);
					ASSERT(target != &gc_gen0);
					sithrd_atomic_or(&head->gc_info.gi_flag, Dee_GC_FLAG_GENN);
					sithrd_gc_object_set_pself(&head->gc_info.gi_pself, &target->gg_objects);
					if ((head->gc_next = target->gg_objects) != NULL) {
						struct Dee_gc_head *next_head = DeeGC_Head(target->gg_objects);
						sithrd_gc_object_set_pself(&next_head->gc_info.gi_pself, &head->gc_next);
					}
					ASSERT(sithrd_atomic_read(&head->gc_info.gi_flag) & Dee_GC_FLAG_GENN);
					target->gg_objects = iter;
					++target->gg_count;
					--target->gg_collect_on;
				} else
#endif /* !DEFINE_gc_collectall_collect_or_unlock */
				{
					/* Just leave within our own generation */
					p_iter = &head->gc_next;
				}
			}
		}
	}

	/* Deal with "unreachable_slow" (which must be resolved before anything else can happen) */
	if unlikely(unreachable_slow) {
#ifndef DEFINE_gc_collectall_collect_or_unlock
		/* Restore old flags */
		if (gen != &gc_gen0) {
			sithrd_gc_generation_clear__GC_FLAG_OTHERGEN(&gc_gen0);
			sithrd_gc_generation_set__GC_FLAG_OTHERGEN(gen);
		}
#endif /* !DEFINE_gc_collectall_collect_or_unlock */

		/* Restore all "unreachable_fast" objects into some generation */
#ifdef DEFINE_gc_collectall_collect_or_unlock
#define LOCAL_gen (&gc_gen0)
#else /* DEFINE_gc_collectall_collect_or_unlock */
#define LOCAL_gen gen
#endif /* !DEFINE_gc_collectall_collect_or_unlock */
		while (unreachable_fast) {
			struct Dee_gc_head *head = DeeGC_Head(unreachable_fast);
			DeeObject *next = head->gc_next;
			sithrd_gc_object_set_pself(&head->gc_info.gi_pself, &LOCAL_gen->gg_objects);
			ASSERT(!(sithrd_atomic_read(&head->gc_info.gi_flag) & Dee_GC_FLAG_GENN));
#ifndef DEFINE_gc_collectall_collect_or_unlock
			if (LOCAL_gen != &gc_gen0)
#endif /* !DEFINE_gc_collectall_collect_or_unlock */
			{
				sithrd_atomic_or(&head->gc_info.gi_flag, Dee_GC_FLAG_GENN);
			}
			if ((head->gc_next = LOCAL_gen->gg_objects) != NULL) {
				struct Dee_gc_head *next_head = DeeGC_Head(LOCAL_gen->gg_objects);
				sithrd_gc_object_set_pself(&next_head->gc_info.gi_pself, &head->gc_next);
			}
			LOCAL_gen->gg_objects = unreachable_fast;
			++LOCAL_gen->gg_count;
			unreachable_fast = next;
		}
#undef LOCAL_gen

		/* Set the "Dee_GC_FLAG_GENN" flag for all objects from "unreachable_slow" */
		for (iter = unreachable_slow; iter;) {
			struct Dee_gc_head *head = DeeGC_Head(iter);
			sithrd_atomic_or(&head->gc_info.gi_flag, Dee_GC_FLAG_GENN);
			ASSERT(gc_remove_modifying == 0);
			ASSERT(iter->ob_refcnt != 0);
			/* Reference needed for when we're about to invoke "tp_finalize" for these objects! */
			++iter->ob_refcnt;
			iter = head->gc_next;
		}

		/* Reduce the chance of recursive GC collect happening... */
		LOCAL_foreach_generation() {
			gen->gg_collect_on = gen->gg_count;
			if (gen->gg_collect_on < gen->gg_mindim)
				gen->gg_collect_on = gen->gg_mindim;
		}

		/* At this point, "unreachable_slow" exists in a sort-of "fake" generation
		 * defined by "unreachable_slow" itself. If anything causes these objects to
		 * be destroyed after this point, `DeeGC_Untrack()' and `DeeGC_UntrackAsync()'
		 * will remove them from *that* linked list. */
		COMPILER_BARRIER();
		gc_collect_release();

		/* Enumerate slow-unreachable objects to invoke their "tp_finalize" callbacks.
		 * Note that for this purpose, we know that objects will *NOT* be removed from
		 * "unreachable_slow" until we drop the references we acquired above! */
		for (iter = atomic_read(&unreachable_slow); iter;) {
			struct Dee_gc_head *head = DeeGC_Head(iter);
			DeeObject *next = atomic_read(&head->gc_next);
			if (!(atomic_fetchor(&head->gc_info.gi_flag, Dee_GC_FLAG_FINALIZED) & Dee_GC_FLAG_FINALIZED)) {
				/* Invoke "tp_finalize"
				 * If this causes an untrack, the object will simply unlink itself from
				 * our stack-lock "unreachable_slow" (or will at least do so as soon as
				 * pending remove operations are reaped). In that sense, "unreachable_slow"
				 * behaves similar to a small, self-contained GC generation */
				DeeObject_DoInvokeFinalize(iter);
			}

			/* Drop the reference created above */
			Dee_Decref(iter);
			iter = next;
		}

		/* In the likely case that the user-defined "tp_finalize" **didn't** resolve *every*
		 * reference loop (which it could in theory do by manually breaking those loops), then
		 * we have to re-add */
		if likely(atomic_read(&unreachable_slow)) {
			DeeObject *first;
			gc_lock_acquire();
			first = atomic_read(&unreachable_slow);
			if likely(first != NULL) {
#ifdef DEFINE_gc_collectall_collect_or_unlock
				sithrd_gc_generation_insert_temp_list(&gc_gen0, first);
#else /* DEFINE_gc_collectall_collect_or_unlock */
				sithrd_gc_generation_insert_temp_list(gen, first);
#endif /* !DEFINE_gc_collectall_collect_or_unlock */
			}
			_gc_lock_release(); /* Don't reap here -- caller will just retry which will do that for us */
		}

		/* Tell caller to try again (the second time around, none of the objects we just finalized
		 * will still appear in the "unreachable_slow" queue since 'Dee_GC_FLAG_FINALIZED' has been
		 * set for every one of them). */
		return GC_GENERATION_COLLECT_OR_UNLOCK__RETRY;
	}

	/* Figure out a new threshold that must be reached before another collect happens. */
	LOCAL_foreach_generation() {
		gen->gg_collect_on = num_reachable;
		if (gen->gg_collect_on < gen->gg_mindim)
			gen->gg_collect_on = gen->gg_mindim;
	}

	if (!unreachable_fast) {
		/* Restore old flags */
#ifndef DEFINE_gc_collectall_collect_or_unlock
		if (gen != &gc_gen0) {
			sithrd_gc_generation_clear__GC_FLAG_OTHERGEN(&gc_gen0);
			sithrd_gc_generation_set__GC_FLAG_OTHERGEN(gen);
		}
#endif /* !DEFINE_gc_collectall_collect_or_unlock */

		/* Everything is reachable -> nothing to do */
		return GC_GENERATION_COLLECT_OR_UNLOCK__NOTHING;
	}

	/* At this point, generations have been updated and everything that should go away
	 * is count within "unreachable_fast". Additionally, elements of "unreachable_fast"
	 * all have bit#0 of "gc_next" set (so we can identify these objects in what we're
	 * about to do next):
	 * - If we discovered any weak references during the initial scan, then that means
	 *   that there might still be weak references out there that *could* (in theory)
	 *   revive the objects from "unreachable_fast".
	 * - We don't want that, so we have to identify those weak references (should they
	 *   actually exist) proper, and kill them **BEFORE** we can unlock the GC and
	 *   resume other threads.
	 *
	 * For this purpose, we must:
	 * - Kill weak references that may be directly attached to something from "unreachable_fast"
	 * - Kill weak references of non-GC objects that should get also destroyed also */
	if (must_kill_nested_weakrefs) {
		for (iter = unreachable_fast; iter; iter = DeeGC_Head(iter)->gc_next)
			DeeObject_Visit(iter, &LOCAL_gc_visit__decref__cb, NULL);
		for (iter = unreachable_fast; iter; iter = DeeGC_Head(iter)->gc_next)
			DeeObject_Visit(iter, &LOCAL_gc_visit__incref__with_weakref_kill__cb, NULL);
	}
	if (must_kill_weakrefs) {
		for (iter = unreachable_fast; iter; iter = DeeGC_Head(iter)->gc_next)
			DeeObject_KillWeakrefs(iter);
	}

	/* Restore old flags */
#ifndef DEFINE_gc_collectall_collect_or_unlock
	if (gen != &gc_gen0) {
		sithrd_gc_generation_clear__GC_FLAG_OTHERGEN(&gc_gen0);
		sithrd_gc_generation_set__GC_FLAG_OTHERGEN(gen);
	}
#endif /* !DEFINE_gc_collectall_collect_or_unlock */

	/* Get some references to objects that are about to be destroyed.
	 * This is needed, because similarly to "unreachable_slow", we have
	 * to invoke some operators on those objects ("tp_clear"), which
	 * just goes way easier if we're not actively suspending all other
	 * threads.
	 *
	 * This is also a pretty good spot to count "num_unreachable" */
	num_unreachable = 0;
	for (iter = unreachable_fast; iter;) {
		struct Dee_gc_head *head = DeeGC_Head(iter);
		/* We're emulating some "nth" generation, so we have to set this flag! */
#ifndef DEFINE_gc_collectall_collect_or_unlock
		ASSERT(!(sithrd_atomic_read(&head->gc_info.gi_flag) & Dee_GC_FLAG_GENN));
#endif /* !DEFINE_gc_collectall_collect_or_unlock */
		sithrd_atomic_or(&head->gc_info.gi_flag, Dee_GC_FLAG_GENN);
		++iter->ob_refcnt;
		++num_unreachable;
		iter = head->gc_next;
	}
	*p_num_collected = num_unreachable;

	/* Release lock */
	COMPILER_BARRIER();
	gc_collect_release();

	/* Invoke weakref callbacks scheduled by `Dee_weakref_list_transfer_to_dummy()'
	 * This can only happen now that the GC lock has been released, since no user-
	 * code can be executed while said lock is held. */
	if (must_kill_nested_weakrefs || must_kill_weakrefs)
		Dee_weakref_list_kill_dummy();

	/* Clear unreachable objects (this is where everything should die for real) */
	for (iter = atomic_read(&unreachable_fast); iter;) {
		struct Dee_gc_head *head = DeeGC_Head(iter);
		DeeObject *next = atomic_read(&head->gc_next);
#if !defined(NDEBUG) && 0
		Dee_DPRINTF("[gc] Clearing unreachable instance of %q at %p",
		            DeeType_GetName(Dee_TYPE(iter)), iter);
		gc_dprint_object_info(Dee_TYPE(iter), iter);
		Dee_DPRINT("\n");
#endif /* !NDEBUG */
		DeeObject_GCClear(iter);
		Dee_Decref(iter);
		iter = next;
	}

	/* At this point, our "unreachable_fast" chain may not be fully clear, yet.
	 * This can have 2 reasons:
	 *  #1: One of the objects destroyed by the clear+decref above is still
	 *      dangling in "gc_remove"
	 *  #2: Something weird is going on with the GC and somehow, some way, one
	 *      of those objects that **really** should have been destroyed, didn't
	 *      end up going away properly
	 *
	 * In either case, we can't return yet: because the address of "unreachable_fast"
	 * may still be linked into the "Dee_gc_head" of objects that have to be fully
	 * destroyed (or moved) before that can happen.
	 *
	 * Also: both of those problems can simply be solved by adding "unreachable_fast"
	 *       to "gc_gen0".
	 */
	if unlikely(atomic_read(&unreachable_fast)) {
		DeeObject *first;
		gc_lock_acquire_and_reap(DeeGC_TRACK_F_NOCOLLECT); /* Reap here *should* normally cause everything to disappear */
		COMPILER_BARRIER();
		first = atomic_read(&unreachable_fast);
		if unlikely(first != NULL) {
			size_t count = 1;
			DeeObject *last;
			struct Dee_gc_head *first_head;
			struct Dee_gc_head *last_head;
			for (last = first;;) {
				struct Dee_gc_head *head = DeeGC_Head(last);
				/* This check here is a good idea to detect bad GC behavior (which could also
				 * happen as a result of bad "tp_visit" / "tp_clear"), but fails in practice:
				 *
				 * Given the following code:
				 *    >> class MyClass { public member v; };
				 *    >> local INSTANCE = MyClass();
				 *    >> INSTANCE.v = INSTANCE;
				 *    >> del INSTANCE;
				 *    >> gc.collect();
				 * And assuming execution in a system with 2 threads, using the following sequence:
				 *
				 *  [0] Thread #1:   unreachable_fast = {INSTANCE,MyClass}
				 *  [1] Thread #1:   DeeObject_GCClear(INSTANCE)        -- This clears "INSTANCE.v", thus resolving the reference loop
				 *  [2] Thread #1:   Dee_Decref(INSTANCE);              -- ob_refcnt=0
				 *  [3] Thread #2: gc_lock_acquire();                   -- Oops: Thread #1 will have to untrack asynchronously now...
				 *  [4] Thread #1:      DeeGC_UntrackAsync(INSTANCE);
				 *  [5] Thread #1:      gc_lock_tryacquire() -> false   -- Nope: Thread #2 has that lock
				 *  [6] Thread #1:      gc_remove.insert(INSTANCE);     -- Correct: do async untrack
				 *  [7] Thread #1:   DeeObject_GCClear(MyClass);
				 *  [8] Thread #1:   Dee_Decref(MyClass);               -- ob_refcnt=1 (because still referenced by "INSTANCE->ob_type")
				 *  [9] Thread #2: _gc_lock_reap_and_maybe_unlock();
				 * [10] Thread #2: pending_remove = CONSUME(gc_remove); -- Consume "INSTANCE" from [6]
				 * [11] Thread #2: gc_do_untrack_locked(INSTANCE);      -- This removes "INSTANCE" from Thread #1's "unreachable_fast"
				 * [12] Thread #2: _gc_lock_release();
				 * [13] Thread #1:   unreachable_fast != NULL;          -- Still non-NULL because "MyClass" hasn't been removed, yet
				 * [14] Thread #1:   gc_lock_acquire_and_reap();        -- Can't reap "gc_remove" to finish destruction of "MyClass" because Thread #2 is already doing that
				 *
				 * At this point, it would appear that "MyClass" hasn't been destroyed
				 * ("MyClass->ob_refcnt == 1") when it really should have been, which
				 * is further corroborated by "unreachable_fast = {MyClass}".
				 * But that's only because destruction of it hasn't completed, yet. If
				 * we were to wait, "Thread #2" will continue its work and eventually
				 * destroy (or rather: enqueue the untracking of) "MyClass":
				 *
				 * [15] Thread #2: DeeGCObject_FinishDestroyAfterUntrack(INSTANCE);
				 * [16] Thread #2: Dee_Decref(INSTANCE->ob_type) == Dee_Decref(MyClass); -- ob_refcnt=0
				 * [17] Thread #2: DeeGC_UntrackAsync(MyClass);
				 * [18] Thread #2: gc_lock_tryacquire() -> false        -- Nope: Thread #1 acquired this in [14]
				 * [19] Thread #2: gc_remove.insert(MyClass);
				 * [20] Thread #1:   if (atomic_read(&MyClass->ob_refcnt) > 0) { WARNING(); }
				 *
				 * At step [20], the "ob_refcnt" read is actually the linked-list pointer
				 * used by "gc_remove", causing us to read a very larger number instead.
				 *
				 * Similarly, if step [20] happened between [14]-[15], we'd still be
				 * reading "MyClass->ob_refcnt = 1", because at that point "INSTANCE"
				 * hadn't been fully destroyed yet. After all: this is the kind-of
				 * situation this debug-check was supposed to detect: "an object that
				 * *should* have been destroyed, still being used by other threads"
				 * (only that the word "used" here actually means "is in the process
				 * of being destroyed [by other threads]", which is something that's
				 * allowed to be happen, but can't be differentiated from bad uses)
				 *
				 * ---
				 *
				 * So in other words:
				 * - If another thread acquires "gc_lock" while we do "DeeObject_GCClear()"...
				 * - then our thread will be forced to use "gc_remove" to complete the untrack...
				 * - and that pending remove-task can then be served by another thread...
				 * - such that the actual "tp_dtor" / "Dee_Decref(ob_type)" finalization also
				 *   happens in another thread...
				 * - which means that anything that is only decref's in "tp_dtor" (or "ob_type"
				 *   itself) may happen asynchronously to our thread, and we have no way to
				 *   ensure that any async destruction of objects from "unreachable_fast" has
				 *   already completed.
				 *
				 * ---
				 *
				 * However: luckily none of this is actually detrimental in terms of semantics.
				 *          Sure, it means we have less capabilities when it comes to debugging
				 *          bad DeeObject_GCClear() calls, but...
				 *
				 * -> The below fallback-handling of "simply moving 'unreachable_fast' to 'gc_gen0'"
				 *    really **does** still work, **and** will allow us to return since it kills
				 *    any dependency between objects being destroyed and 'unreachable_fast'.
				 *
				 * Code to reproduce:
				 * >>import * from deemon;
				 * >>function predcmp2key(cmp: Callable): Callable {
				 * >>	class KeyClass {
				 * >>		private member m_self;
				 * >>		this(self): m_self(self) {}
				 * >>		public operator <  (other) -> cmp(this.m_self, other.m_self) <  0;
				 * >>		public operator <= (other) -> cmp(this.m_self, other.m_self) <= 0;
				 * >>		public operator == (other) -> cmp(this.m_self, other.m_self) == 0;
				 * >>		public operator != (other) -> cmp(this.m_self, other.m_self) != 0;
				 * >>		public operator >  (other) -> cmp(this.m_self, other.m_self) >  0;
				 * >>		public operator >= (other) -> cmp(this.m_self, other.m_self) >= 0;
				 * >>	}
				 * >>	return KeyClass;
				 * >>}
				 * >>local threads = [];
				 * >>for (none: [:10]) {
				 * >>	local t = Thread(() -> {
				 * >>		for (;;) {
				 * >>			local items = ["Foo", "BAR", "foobar"];
				 * >>			items.sort(predcmp2key(string.casecompare));
				 * >>			assert items == {"BAR", "Foo", "foobar"};
				 * >>//			gc.collect();
				 * >>		}
				 * >>	});
				 * >>	t.start();
				 * >>	threads.append(t);
				 * >>}
				 * >>for (local t: threads)
				 * >>	t.join();
				 */
#ifndef NDEBUG
				if (!DeeThread_IsMultiThreaded) {
					/* The above race condition making this debug-check
					 * impossible can only happen in SMP-mode */
					Dee_refcnt_t refcnt = atomic_read(&last->ob_refcnt);
					if (refcnt > 0) {
						Dee_DPRINTF("[gc] Warning: instance of %q at %p has "
						            "ob_refcnt=%" PRFuSIZ " but is unreachable",
						            DeeType_GetName(Dee_TYPE(last)), last, refcnt);
						gc_dprint_object_info(Dee_TYPE(last), last);
						Dee_DPRINT("\n");
						Dee_BREAKPOINT();
						ASSERT(*p_num_collected);
						--*p_num_collected;
					}
				}
#endif /* !NDEBUG */
				ASSERT(atomic_read(&head->gc_info.gi_flag) & Dee_GC_FLAG_GENN);
				atomic_and(&head->gc_info.gi_flag, ~Dee_GC_FLAG_GENN);
				if (!head->gc_next)
					break;
				last = head->gc_next;
				++count;
			}

			/* Insert all remaining objects into "&gc_gen0" */
			first_head = DeeGC_Head(first);
			last_head  = DeeGC_Head(last);
			gc_object_set_pself(&first_head->gc_info.gi_pself, &gc_gen0.gg_objects); /* This **must** be atomic! */
			if ((last_head->gc_next = gc_gen0.gg_objects) != NULL) {
				struct Dee_gc_head *next_head = DeeGC_Head(gc_gen0.gg_objects);
				gc_object_set_pself(&next_head->gc_info.gi_pself, &last_head->gc_next); /* This **must** be atomic! */
			}
			gc_gen0.gg_objects = first;
			gc_gen0.gg_count += count;
			gc_gen0.gg_collect_on -= count;
		}
		COMPILER_BARRIER();
		gc_lock_release(DeeGC_TRACK_F_NOCOLLECT);
	}

	return GC_GENERATION_COLLECT_OR_UNLOCK__SUCCESS;
#undef LOCAL_foreach_generation
}

#undef LOCAL_gc_visit__decref__cb
#undef LOCAL_gc_visit__incref__with_weakref_detect__cb
#undef LOCAL_gc_visit__incref__with_weakref_kill__cb

DECL_END

#undef DEFINE_gc_generation_collect_or_unlock
#undef DEFINE_gc_collectall_collect_or_unlock
