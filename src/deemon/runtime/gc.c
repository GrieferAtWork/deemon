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
#ifndef GUARD_DEEMON_RUNTIME_GC_C
#define GUARD_DEEMON_RUNTIME_GC_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/asm.h>
#include <deemon/bool.h>
#include <deemon/code.h>
#include <deemon/gc.h>
#include <deemon/int.h>
#include <deemon/object.h>
#include <deemon/seq.h>
#include <deemon/string.h>

#ifndef CONFIG_NO_DEX
#include <deemon/dex.h>
#endif /* !CONFIG_NO_DEX */

#ifndef CONFIG_NO_THREADS
#include <deemon/util/recursive-rwlock.h>
#include <deemon/util/rwlock.h>
#endif /* !CONFIG_NO_THREADS */

#include <deemon/util/string.h>

#include <hybrid/atomic.h>

#include <stddef.h>
#include <stdint.h>

#include "../objects/gc_inspect.h"
#include "strings.h"

DECL_BEGIN


/* Required in case new GC objects are scheduled
 * whilst tp_gc() is invoked on some GC object
 * when `gc_lock' is still being held. */
#undef CONFIG_HAVE_PENDING_GC_OBJECTS
#if 1
#define CONFIG_HAVE_PENDING_GC_OBJECTS 1
#endif

/* Use priority clear (`tp_pclear') instead of `tp_clear' */
#undef CONFIG_GC_PRIORITY_CLEAR
#if 1
#define CONFIG_GC_PRIORITY_CLEAR 1
#endif

/* Construct the dependency partners vector such that
 * it can be scanned using a bsearch-style approach. */
#undef CONFIG_GC_DEP_PARTNERS_USE_BSEARCH
#if 1
#define CONFIG_GC_DEP_PARTNERS_USE_BSEARCH 1
#endif

/* Keep track of leaf objects to speed up gc traversal. */
#undef CONFIG_GC_TRACK_LEAFS
#if 1
#define CONFIG_GC_TRACK_LEAFS 1
#endif




#ifdef CONFIG_NO_THREADS
#undef CONFIG_HAVE_PENDING_GC_OBJECTS
#endif /* CONFIG_NO_THREADS */

#ifndef CONFIG_NO_THREADS
/* _Must_ use a recursive lock for this because untracking objects
 * while they're being collected requires acquisition of the GC-lock. */
PRIVATE recursive_rwlock_t gc_lock = RECURSIVE_RWLOCK_INIT;
#define GCLOCK_TRYACQUIRE()   recursive_rwlock_trywrite(&gc_lock)
#define GCLOCK_ACQUIRE()      recursive_rwlock_write(&gc_lock)
#define GCLOCK_RELEASE()      recursive_rwlock_endwrite(&gc_lock)
#define GCLOCK_ACQUIRE_READ() recursive_rwlock_read(&gc_lock)
#define GCLOCK_RELEASE_READ() recursive_rwlock_endread(&gc_lock)
#else /* !CONFIG_NO_THREADS */
#define GCLOCK_TRYACQUIRE() 1
#define GCLOCK_ACQUIRE()   (void)0
#define GCLOCK_RELEASE()   (void)0
#endif /* CONFIG_NO_THREADS */

#ifndef NDEBUG
#if __SIZEOF_POINTER__ == 4
#define GCHEAD_ISTRACKED(h) ((h)->gc_pself != (struct gc_head **)UINT32_C(0xcccccccc))
#elif __SIZEOF_POINTER__ == 8
#define GCHEAD_ISTRACKED(h) ((h)->gc_pself != (struct gc_head **)UINT32_C(0xcccccccccccccccc))
#endif
#endif /* !NDEBUG */


/* [lock(gc_lock)][0..1] The list root of all GC objects. */
PRIVATE struct gc_head *gc_root  = NULL;
PRIVATE bool did_untrack = false;

#ifdef CONFIG_HAVE_PENDING_GC_OBJECTS
PRIVATE struct gc_head *gc_pending = NULL; /* [0..1] Pending object for GC tracking. */
#ifdef CONFIG_NO_THREADS
#define GC_PENDING_MUSTSERVICE() (gc_pending != NULL)
#else /* CONFIG_NO_THREADS */
#define GC_PENDING_MUSTSERVICE() (ATOMIC_READ(gc_pending) != NULL)
#endif /* !CONFIG_NO_THREADS */
PRIVATE void DCALL gc_pending_service(void) {
	struct gc_head *chain;
	chain = ATOMIC_XCH(gc_pending, NULL);
	if (chain) {
		struct gc_head *next, **pself;
		pself = &gc_root;
		for (next = chain;;) {
			next->gc_pself = pself;
			pself = &next->gc_next;
			next  = *pself;
			if (!next)
				break;
		}
		next = COMPILER_CONTAINER_OF(pself, struct gc_head, gc_next); /* last */
		ASSERT(next->gc_next == NULL);
		ASSERT(chain->gc_pself == &gc_root);
		/* Append the old GC chain */
		if ((next->gc_next = gc_root) != NULL)
			gc_root->gc_pself = &next->gc_next;
		gc_root = chain; /* Set the new GC chain */
	}
}

LOCAL void DCALL gc_lock_acquire_s(void) {
	GCLOCK_ACQUIRE();
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
#define GCLOCK_ACQUIRE_S() GCLOCK_ACQUIRE()
#define GCLOCK_RELEASE_S() GCLOCK_RELEASE()
#endif /* !CONFIG_HAVE_PENDING_GC_OBJECTS */


/* Begin/end tracking a given GC-allocated object.
 * `DeeGC_Track()' must be called explicitly when the object
 * has been allocated using `DeeGCObject_Malloc' and friends,
 * though constructions of non-variadic GC objects don't need
 * to call this function on the passed object. - That call will
 * automatically be done when the function returns successfully. */
PUBLIC ATTR_RETNONNULL NONNULL((1)) DeeObject *DCALL
DeeGC_Track(DeeObject *__restrict ob) {
	struct gc_head *head;
	ASSERT(ob);
	ASSERT_OBJECT_TYPE(ob->ob_type, &DeeType_Type);
	ASSERT(DeeGC_Check(ob));
	head = DeeGC_Head(ob);
#ifdef GCHEAD_ISTRACKED
	ASSERTF(!GCHEAD_ISTRACKED(head),
	        "Object is already being tracked");
#endif /* GCHEAD_ISTRACKED */
#ifdef CONFIG_HAVE_PENDING_GC_OBJECTS
	if (!GCLOCK_TRYACQUIRE()) {
		struct gc_head *next;
#ifdef GCHEAD_ISTRACKED
		head->gc_pself = (struct gc_head **)1;
		ASSERT(GCHEAD_ISTRACKED(head));
#endif /* GCHEAD_ISTRACKED */
		do {
			next = ATOMIC_READ(gc_pending);
			head->gc_next = next;
			COMPILER_WRITE_BARRIER();
		} while (!ATOMIC_CMPXCH(gc_pending, next, head));
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
	struct gc_head *head;
	ASSERT(ob);
	ASSERT_OBJECT_TYPE(ob->ob_type, &DeeType_Type);
	ASSERT(DeeGC_Check(ob));
	head = DeeGC_Head(ob);
#ifdef GCHEAD_ISTRACKED
	ASSERTF(GCHEAD_ISTRACKED(head),
	        "Object %p of type %s isn't being tracked\n"
	        "repr: `%r'",
	        ob, ob->ob_type->tp_name, ob);
#endif /* GCHEAD_ISTRACKED */
	GCLOCK_ACQUIRE_S();
	/*printf("Untrack: %s:%p\n",ob->ob_type->tp_name,ob);*/
	/* Unlink the gc head from whatever chain it is apart of. */
	/* HINT: If the following line crashes, it is likely that a GC-object was
	 *       allocated using something other than the GC-object allocators:
	 *       >> DeeObject *my_gc_ob = DeeObject_Malloc(42); // Should have used `DeeGCObject_Malloc'
	 */
	if ((*head->gc_pself = head->gc_next) != NULL)
		head->gc_next->gc_pself = head->gc_pself;
	did_untrack = true;
	GCLOCK_RELEASE_S();
#ifndef NDEBUG
	memset(head, 0xcc, DEE_GC_HEAD_SIZE);
#endif /* !NDEBUG */
	return ob;
}



struct gc_dep {
	dref_t          gd_extern; /* [valid_if(gd_object)] Amount of external (unaccounted) references. */
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
	DeeObject *dp_obj;  /* [1..1] The object that is apart of the current, unconfirmed chain of dependencies. */
	dref_t     dp_num;  /* [!0] Amount of reference accounted for this chain entry. */
};

struct gc_dep_partners {
	dref_t                 dp_pnum; /* Number of partner dependencies. */
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
	new_vec = (struct gc_dep_partner *)Dee_TryRealloc(self->dp_pvec,
	                                                  new_num_objs *
	                                                  sizeof(struct gc_dep_partner));
	if unlikely(!new_vec) {
		DeeMem_ClearCaches((size_t)-1);
		new_vec = (struct gc_dep_partner *)Dee_TryRealloc(self->dp_pvec,
		                                                  new_num_objs *
		                                                  sizeof(struct gc_dep_partner));
		if unlikely(!new_vec)
			return false;
	}
	self->dp_pvec = new_vec;
	self->dp_pnum = new_num_objs;
	return true;
}

/* Insert the `nth_object' after a call to `gc_dep_partners_resize()' */
LOCAL WUNUSED NONNULL((1)) void DCALL
gc_dep_partners_insert_after_resize(struct gc_dep_partners *__restrict self,
                                    size_t nth_object,
                                    DeeObject *__restrict obj,
                                    dref_t num) {
#ifdef CONFIG_GC_DEP_PARTNERS_USE_BSEARCH
	size_t lo, hi;
	ASSERT(nth_object < self->dp_pnum);
	lo = 0;
	hi = self->dp_pnum;
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
	memmove(&self->dp_pvec[lo] + 1,
	        &self->dp_pvec[lo],
	        (nth_object - lo) *
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
LOCAL WUNUSED NONNULL((1)) void DCALL
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
				memcpy(&self->dp_pvec[nth_object],
				       &vec[src_index],
				       (count - src_index) *
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
		memmove(&self->dp_pvec[dst_index] + 1,
		        &self->dp_pvec[dst_index],
		        (nth_object - dst_index) *
		        sizeof(struct gc_dep_partner));
		self->dp_pvec[dst_index].dp_obj = obj;
		self->dp_pvec[dst_index].dp_num = vec[src_index].dp_num;
		++dst_index;
		++nth_object;
	}
#else /* CONFIG_GC_DEP_PARTNERS_USE_BSEARCH */
	ASSERT((nth_object + count) <= self->dp_pnum);
	ASSERT(count != 0);
	memcpy(&self->dp_pvec[nth_object], vec, count * sizeof(struct gc_dep_partner));
#endif /* !CONFIG_GC_DEP_PARTNERS_USE_BSEARCH */
}

struct gc_dep_chain {
	struct gc_dep_chain   *dc_prev; /* [0..1] Another object dependency also apart of this chain. */
	DeeObject             *dc_obj;  /* [1..1] The object that is apart of the current, unconfirmed chain of dependencies. */
	dref_t                 dc_num;  /* [!0] Amount of reference accounted for this chain entry. */
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
	new_vec = (struct gc_dep *)Dee_TryCalloc((new_mask + 1) *
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
               dref_t num_tracked_references) {
	dhash_t i, perturb;
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
		dep->gd_extern = ATOMIC_FETCHINC(obj->ob_refcnt);
		ASSERT(dep->gd_extern != 0);
#ifdef GC_ASSERT_REFERENCE_COUNTS
		ASSERT(dep->gd_extern >= num_tracked_references);
#endif /* GC_ASSERT_REFERENCE_COUNTS */
		if (dep->gd_extern >= num_tracked_references)
			dep->gd_extern -= num_tracked_references;
		else {
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
	new_vec = (struct gc_leaf *)Dee_TryCalloc((new_mask + 1) *
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
	dhash_t i, perturb;
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
	/*DEE_DPRINTF("VISIT: %k at %p (%u refs)\n", tp_self, self, self->ob_refcnt);*/

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
	DeeObject_Visit(self, (dvisit_t)&visit_object, data);

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
gc_trydestroy(struct gc_head *__restrict head,
              struct gc_dep **__restrict pdep_buffer,
              size_t *__restrict pdep_mask
#ifdef CONFIG_GC_TRACK_LEAFS
              ,
              struct gc_leaf **__restrict pleaf_buffer,
              size_t *__restrict pleaf_mask
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
	visit.vd_deps.gd_vec  = *pdep_buffer;
	visit.vd_deps.gd_msk  = *pdep_mask;
	visit.vd_deps.gd_err  = false;
	visit.vd_chain        = NULL;
	memset(visit.vd_deps.gd_vec, 0,
	       (visit.vd_deps.gd_msk + 1) *
	       sizeof(struct gc_dep));
#ifdef CONFIG_GC_TRACK_LEAFS
	visit.vd_leafs.gl_cnt = 0;
	visit.vd_leafs.gl_vec = *pleaf_buffer;
	visit.vd_leafs.gl_msk = *pleaf_mask;
	memset(visit.vd_leafs.gl_vec, 0,
	       (visit.vd_leafs.gl_msk + 1) *
	       sizeof(struct gc_leaf));
#endif /* CONFIG_GC_TRACK_LEAFS */

	/* Add the initial dependency, that is the object being collected itself. */
	init_dep            = &visit.vd_deps.gd_vec[VD_HASHOF(&visit.vd_deps, &head->gc_object)];
	init_dep->gd_object = &head->gc_object;
	/* Capture + incref the given object's reference counter. */
	for (;;) {
		init_dep->gd_extern = ATOMIC_READ(head->gc_object.ob_refcnt);
		if unlikely(!init_dep->gd_extern)
			return 0; /* Object is already dead! */
		if (ATOMIC_CMPXCH_WEAK(head->gc_object.ob_refcnt,
		                       init_dep->gd_extern,
		                       init_dep->gd_extern + 1))
			break;
	}
	/* Recursively visit our initial dependency. */
	DeeObject_Visit(&head->gc_object, (dvisit_t)&visit_object, &visit);

#define OPTIMIZE_DEPS_SET()                                  \
	{                                                        \
		size_t last_used = 0;                                \
		for (i = 0; i <= visit.vd_deps.gd_msk; ++i) {        \
			size_t end_used;                                 \
			if (!visit.vd_deps.gd_vec[i].gd_object)          \
				continue;                                    \
			end_used = i;                                    \
			for (end_used = i + 1;                           \
			     end_used <= visit.vd_deps.gd_msk &&         \
			     visit.vd_deps.gd_vec[end_used].gd_object;   \
			     ++end_used)                                 \
				;                                            \
			memmove(&visit.vd_deps.gd_vec[last_used],        \
			        &visit.vd_deps.gd_vec[i],                \
			        (end_used - i) * sizeof(struct gc_dep)); \
			last_used += end_used - i;                       \
			i = end_used;                                    \
		}                                                    \
		ASSERT(last_used == visit.vd_deps.gd_cnt);           \
	}

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
		if (Dee_DecrefWasOk_untraced(ob))
			++result;
	}
out:
	*pdep_buffer  = visit.vd_deps.gd_vec;
	*pdep_mask    = visit.vd_deps.gd_msk;
#ifdef CONFIG_GC_TRACK_LEAFS
	*pleaf_buffer = visit.vd_leafs.gl_vec;
	*pleaf_mask   = visit.vd_leafs.gl_msk;
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



PUBLIC size_t DCALL DeeGC_Collect(size_t max_objects) {
	struct gc_head *iter;
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
		if (ATOMIC_READ(iter->gc_object.ob_refcnt) == 0)
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
		DEE_CHECKMEMORY();
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
		DEE_CHECKMEMORY();
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

INTERN bool DCALL DeeGC_IsEmptyWithoutDex(void) {
	struct gc_head *iter;
	GCLOCK_ACQUIRE_S();
	/* Also search the regular GC chain. */
	iter = gc_root;
	for (; iter; iter = iter->gc_next) {
		ASSERT(iter != iter->gc_next);
		if (!iter->gc_object.ob_refcnt)
			continue;
#ifndef CONFIG_NO_DEX
		if (DeeDex_Check(&iter->gc_object))
			continue;
#endif /* !CONFIG_NO_DEX */
		goto is_nonempty;
	}
	GCLOCK_RELEASE_S();
	return true;
is_nonempty:
	GCLOCK_RELEASE_S();
	return false;
}


INTERN size_t DCALL DeeExec_KillUserCode(void) {
	struct gc_head *iter;
	size_t result = 0;
#ifdef CONFIG_HAVE_PENDING_GC_OBJECTS
collect_restart_with_pending_hint:
#endif /* CONFIG_HAVE_PENDING_GC_OBJECTS */
	GCLOCK_ACQUIRE_S();
	for (iter = gc_root; iter; iter = iter->gc_next) {
		instruction_t old_instr;
		uint16_t old_flags;
		ASSERT_OBJECT(&iter->gc_object);
		ASSERT(iter != iter->gc_next);
		if unlikely(!iter->gc_object.ob_refcnt)
			continue; /* Object is currently being destroyed. */
		if (!DeeCode_Check(&iter->gc_object))
			continue; /* Not a code object. */
		if (!((DeeCodeObject *)&iter->gc_object)->co_codebytes)
			continue; /* Empty code? */
		/* Exchange the first instruction with `ASM_RET_NONE' */
#ifdef CONFIG_NO_THREADS
		old_instr = ((DeeCodeObject *)&iter->gc_object)->co_code[0];
		((DeeCodeObject *)&iter->gc_object)->co_code[0] = ASM_RET_NONE;
#else /* CONFIG_NO_THREADS */
		old_instr = ATOMIC_XCH(((DeeCodeObject *)&iter->gc_object)->co_code[0],
		                       ASM_RET_NONE);
#endif /* !CONFIG_NO_THREADS */
		/* One last thing: The interpreter checks the FFINALLY flag
		 *                 to see if there might be finally-handlers
		 *                 that must be invoked after a return-instruction
		 *                 has been encountered.
		 *              -- Delete that flag! We can't have the code
		 *                 re-acquiring control, simply by pre-defining
		 *                 a finally handler to guard the first text byte. */
#ifdef CONFIG_NO_THREADS
		old_flags = ((DeeCodeObject *)&iter->gc_object)->co_flags;
		((DeeCodeObject *)&iter->gc_object)->co_flags = old_flags & ~CODE_FFINALLY;
#else /* CONFIG_NO_THREADS */
		old_flags = ATOMIC_FETCHAND(((DeeCodeObject *)&iter->gc_object)->co_flags,
		                            ~CODE_FFINALLY);
#endif /* !CONFIG_NO_THREADS */
		if (old_instr != ASM_RET_NONE ||
		    (old_flags & CODE_FFINALLY))
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


/* GC object alloc/free. */
LOCAL void *gc_initob(void *ptr) {
	if likely(ptr) {
#ifndef NDEBUG
		memset(ptr, 0xcc, DEE_GC_HEAD_SIZE);
#endif /* !NDEBUG */
		ptr = DeeGC_Object((struct gc_head *)ptr);
	}
	return ptr;
}

PUBLIC WUNUSED ATTR_MALLOC void *(DCALL DeeGCObject_Malloc)(size_t n_bytes) {
	return gc_initob(DeeObject_Malloc(DEE_GC_HEAD_SIZE + n_bytes));
}

PUBLIC WUNUSED ATTR_MALLOC void *(DCALL DeeGCObject_Calloc)(size_t n_bytes) {
	return gc_initob(DeeObject_Calloc(DEE_GC_HEAD_SIZE + n_bytes));
}

PUBLIC WUNUSED void *(DCALL DeeGCObject_Realloc)(void *p, size_t n_bytes) {
	if (p) {
#ifdef GCHEAD_ISTRACKED
		ASSERTF(!GCHEAD_ISTRACKED(DeeGC_Head((DeeObject *)p)),
		        "Object was still being tracked");
#endif /* GCHEAD_ISTRACKED */
		p = DeeObject_Realloc(DeeGC_Head((DeeObject *)p),
		                      DEE_GC_HEAD_SIZE + n_bytes);
	} else {
		p = DeeObject_Malloc(DEE_GC_HEAD_SIZE + n_bytes);
	}
	return gc_initob(p);
}

PUBLIC WUNUSED ATTR_MALLOC void *
(DCALL DeeGCObject_TryMalloc)(size_t n_bytes) {
	return gc_initob(DeeObject_TryMalloc(DEE_GC_HEAD_SIZE + n_bytes));
}

PUBLIC WUNUSED ATTR_MALLOC void *
(DCALL DeeGCObject_TryCalloc)(size_t n_bytes) {
	return gc_initob(DeeObject_TryCalloc(DEE_GC_HEAD_SIZE + n_bytes));
}

PUBLIC WUNUSED void *
(DCALL DeeGCObject_TryRealloc)(void *p, size_t n_bytes) {
	if (p) {
#ifdef GCHEAD_ISTRACKED
		ASSERTF(!GCHEAD_ISTRACKED(DeeGC_Head((DeeObject *)p)),
		        "Object was still being tracked");
#endif /* GCHEAD_ISTRACKED */
		p = DeeObject_TryRealloc(DeeGC_Head((DeeObject *)p),
		                         DEE_GC_HEAD_SIZE + n_bytes);
	} else {
		p = DeeObject_TryMalloc(DEE_GC_HEAD_SIZE + n_bytes);
	}
	return gc_initob(p);
}

#ifdef GCHEAD_ISTRACKED
#define ASSERT_UNTRACKED(p)                                \
	ASSERTF(!GCHEAD_ISTRACKED(DeeGC_Head((DeeObject *)p)), \
	        "Object was still being tracked")
#else /* GCHEAD_ISTRACKED */
#define ASSERT_UNTRACKED(p) (void)0
#endif /* !GCHEAD_ISTRACKED */

PUBLIC void
(DCALL DeeGCObject_Free)(void *p) {
	if (p) {
		ASSERT_UNTRACKED(p);
		DeeObject_Free(DeeGC_Head((DeeObject *)p));
	}
}

#ifndef CONFIG_NO_OBJECT_SLABS
#define DEFINE_GC_SLAB_FUNCTIONS(index, size)                                                                  \
	PUBLIC WUNUSED ATTR_MALLOC void *DCALL                                                                     \
	DeeGCObject_SlabMalloc##size(void) {                                                                       \
		return gc_initob(DeeSlab_Invoke(DeeObject_SlabMalloc, DEE_GC_HEAD_SIZE + size * sizeof(void *), (),    \
		                                (DeeObject_Malloc)(DEE_GC_HEAD_SIZE + size * sizeof(void *))));        \
	}                                                                                                          \
	PUBLIC WUNUSED ATTR_MALLOC void *DCALL                                                                     \
	DeeGCObject_SlabCalloc##size(void) {                                                                       \
		return gc_initob(DeeSlab_Invoke(DeeObject_SlabCalloc, DEE_GC_HEAD_SIZE + size * sizeof(void *), (),    \
		                                (DeeObject_Calloc)(DEE_GC_HEAD_SIZE + size * sizeof(void *))));        \
	}                                                                                                          \
	PUBLIC WUNUSED ATTR_MALLOC void *DCALL                                                                     \
	DeeGCObject_SlabTryMalloc##size(void) {                                                                    \
		return gc_initob(DeeSlab_Invoke(DeeObject_SlabTryMalloc, DEE_GC_HEAD_SIZE + size * sizeof(void *), (), \
		                                (DeeObject_TryMalloc)(DEE_GC_HEAD_SIZE + size * sizeof(void *))));     \
	}                                                                                                          \
	PUBLIC WUNUSED ATTR_MALLOC void *DCALL                                                                     \
	DeeGCObject_SlabTryCalloc##size(void) {                                                                    \
		return gc_initob(DeeSlab_Invoke(DeeObject_SlabTryCalloc, DEE_GC_HEAD_SIZE + size * sizeof(void *), (), \
		                                (DeeObject_TryCalloc)(DEE_GC_HEAD_SIZE + size * sizeof(void *))));     \
	}                                                                                                          \
	PUBLIC void DCALL                                                                                          \
	DeeGCObject_SlabFree##size(void *__restrict ptr) {                                                         \
		ASSERT(ptr);                                                                                           \
		ASSERT_UNTRACKED(ptr);                                                                                 \
		DeeSlab_Invoke(DeeObject_SlabFree, DEE_GC_HEAD_SIZE + size * sizeof(void *),                           \
		               (DeeGC_Head((DeeObject *)ptr)),                                                         \
		               (DeeObject_Free)(DeeGC_Head((DeeObject *)ptr)));                                        \
	}
DeeSlab_ENUMERATE(DEFINE_GC_SLAB_FUNCTIONS)
#undef DEFINE_GC_SLAB_FUNCTIONS
#endif /* !CONFIG_NO_OBJECT_SLABS */


#ifndef NDEBUG
#ifdef CONFIG_TRACE_REFCHANGES
INTDEF NONNULL((1)) void DCALL
dump_reference_history(DeeObject *__restrict obj);
#endif /* !CONFIG_TRACE_REFCHANGES */

INTERN void DCALL gc_dump_all(void) {
	struct gc_head *iter;
	for (iter = gc_root; iter; iter = iter->gc_next) {
		ASSERT(iter != iter->gc_next);
		DEE_DPRINTF("GC Object at %p: Instance of %s (%u refs)\n",
		            &iter->gc_object, iter->gc_object.ob_type->tp_name,
		            iter->gc_object.ob_refcnt);
#ifdef CONFIG_TRACE_REFCHANGES
		dump_reference_history(&iter->gc_object);
#endif /* CONFIG_TRACE_REFCHANGES */
	}
}
#endif /* !NDEBUG */

typedef struct {
	OBJECT_HEAD
#ifndef CONFIG_NO_THREADS
	rwlock_t        gi_lock; /* Lock for `gi_next' */
#endif /* !CONFIG_NO_THREADS */
	DREF DeeObject *gi_next; /* [0..1][lock(gi_lock)]
	                          * The next GC object to-be iterated, or
	                          * NULL when the iterator has been exhausted. */
} GCIter;

PRIVATE NONNULL((1)) void DCALL
gciter_fini(GCIter *__restrict self) {
	Dee_XDecref(self->gi_next);
}

PRIVATE NONNULL((1, 2)) void DCALL
gciter_visit(GCIter *__restrict self, dvisit_t proc, void *arg) {
	Dee_XVisit(self->gi_next);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
gciter_bool(GCIter *__restrict self) {
#ifdef CONFIG_NO_THREADS
	return self->gi_next != NULL;
#else /* CONFIG_NO_THREADS */
	return ATOMIC_READ(self->gi_next) != NULL;
#endif /* !CONFIG_NO_THREADS */
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
gciter_next(GCIter *__restrict self) {
	DREF DeeObject *result;
	struct gc_head *next;
	rwlock_write(&self->gi_lock);
	result = self->gi_next; /* Inherit reference. */
	if unlikely(!result) {
		/* Iterator has been exhausted. */
		rwlock_endwrite(&self->gi_lock);
		return ITER_DONE;
	}
	GCLOCK_ACQUIRE_READ();
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
	rwlock_endwrite(&self->gi_lock);
	/* Return the extracted item. */
	return result;
}


PRIVATE struct type_member gciter_members[] = {
	TYPE_MEMBER_CONST("seq", &DeeGCEnumTracked_Singleton),
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
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ NULL,
				/* .tp_copy_ctor = */ NULL,
				/* .tp_deep_ctor = */ NULL,
				/* .tp_any_ctor  = */ NULL,
				TYPE_FIXED_ALLOCATOR(GCIter)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&gciter_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&gciter_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&gciter_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&gciter_next,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ gciter_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};


PRIVATE WUNUSED DREF GCIter *DCALL
gcenum_iter(DeeObject *__restrict UNUSED(self)) {
	DREF GCIter *result;
	struct gc_head *first;
	result = DeeObject_MALLOC(GCIter);
	if unlikely(!result)
		goto done;
	GCLOCK_ACQUIRE_READ();
	first = gc_root;
	/*  Find the first object that we can actually incref()
	 * (The GC chain may contain dangling (aka. weak) objects) */
	while (first && !Dee_IncrefIfNotZero(&first->gc_object)) {
		ASSERT(first != first->gc_next);
		first = first->gc_next;
	}
	GCLOCK_RELEASE_READ();
	rwlock_init(&result->gi_lock);
	/* Save the first object in the iterator. */
	result->gi_next = first ? &first->gc_object : NULL;
	DeeObject_Init(result, &GCIter_Type);
done:
	return result;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
gcenum_size(DeeObject *__restrict UNUSED(self)) {
	size_t result = 0;
	struct gc_head *iter;
	GCLOCK_ACQUIRE_READ();
	for (iter = gc_root; iter; iter = iter->gc_next) {
		ASSERT(iter != iter->gc_next);
		++result;
	}
	GCLOCK_RELEASE_READ();
	return DeeInt_NewSize(result);
}

PRIVATE WUNUSED DREF DeeObject *DCALL
gcenum_contains(DeeObject *__restrict UNUSED(self),
                DeeObject *__restrict ob) {
	if (!DeeGC_Check(ob))
		return_false;
#if defined(GCHEAD_ISTRACKED)
	return_bool(GCHEAD_ISTRACKED(DeeGC_Head(ob)));
#else /* GCHEAD_ISTRACKED */
	{
		struct gc_head *iter;
		GCLOCK_ACQUIRE_READ();
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
#endif /* !GCHEAD_ISTRACKED */
}

PRIVATE struct type_seq gcenum_seq = {
	/* .tp_iter_self = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&gcenum_iter,
	/* .tp_size      = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&gcenum_size,
	/* .tp_contains  = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&gcenum_contains
};

PRIVATE struct type_member gcenum_class_members[] = {
	TYPE_MEMBER_CONST("Iterator", &GCIter_Type),
	TYPE_MEMBER_END
};

PRIVATE WUNUSED DREF DeeObject *DCALL
gcenum_collect(DeeObject *__restrict UNUSED(self),
               size_t argc, DeeObject **argv) {
	size_t max = (size_t)-1, result;
	if (DeeArg_Unpack(argc, argv, "|Iu:collect", &max))
		goto err;
	result = DeeGC_Collect(max);
	return DeeInt_NewSize(result);
err:
	return NULL;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
gcenum_referred(DeeObject *__restrict UNUSED(self),
                size_t argc, DeeObject **argv) {
	DeeObject *start;
	if (DeeArg_Unpack(argc, argv, "o:referred", &start))
		goto err;
	return (DREF DeeObject *)DeeGC_NewReferred(start);
err:
	return NULL;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
gcenum_referredgc(DeeObject *__restrict UNUSED(self),
                  size_t argc, DeeObject **argv) {
	DeeObject *start;
	if (DeeArg_Unpack(argc, argv, "o:referredgc", &start))
		goto err;
	return (DREF DeeObject *)DeeGC_NewReferredGC(start);
err:
	return NULL;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
gcenum_reachable(DeeObject *__restrict UNUSED(self),
                 size_t argc, DeeObject **argv) {
	DeeObject *start;
	if (DeeArg_Unpack(argc, argv, "o:reachable", &start))
		goto err;
	return (DREF DeeObject *)DeeGC_NewReachable(start);
err:
	return NULL;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
gcenum_reachablegc(DeeObject *__restrict UNUSED(self),
                   size_t argc, DeeObject **argv) {
	DeeObject *start;
	if (DeeArg_Unpack(argc, argv, "o:reachablegc", &start))
		goto err;
	return (DREF DeeObject *)DeeGC_NewReachableGC(start);
err:
	return NULL;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
gcenum_referring(DeeObject *__restrict UNUSED(self),
                 size_t argc, DeeObject **argv) {
	DeeObject *to;
	if (DeeArg_Unpack(argc, argv, "o:referring", &to))
		goto err;
	return (DREF DeeObject *)DeeGC_NewGCReferred(to);
err:
	return NULL;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
gcenum_isreferring(DeeObject *__restrict UNUSED(self),
                   size_t argc, DeeObject **argv) {
	DeeObject *from, *to;
	if (DeeArg_Unpack(argc, argv, "oo:isreferring", &from, &to))
		goto err;
	return_bool(DeeGC_ReferredBy(from, to));
err:
	return NULL;
}


PRIVATE struct type_method gcenum_methods[] = {
	{ "collect", &gcenum_collect,
	  DOC("(max=!-1)->?Dint\n"
	      "Try to collect at least @max GC objects and return the actual number collected\n"
	      "Note that more than @max objects may be collected if sufficiently large reference cycles exist") },
	{ "referred", &gcenum_referred,
	  DOC("(start)->?Dset\n"
	      "Returns a set of objects that are immediately referred to by @start") },
	{ "referredgc", &gcenum_referredgc,
	  DOC("(start)->?Dset\n"
	      "Same as #referred, but only include gc-objects (s.a. :Type.__isgc__)") },
	{ "reachable", &gcenum_reachable,
	  DOC("(start)->?Dset\n"
	      "Returns a set of objects that are reachable from @start") },
	{ "reachablegc", &gcenum_reachablegc,
	  DOC("(start)->?Dset\n"
	      "Same as #reachable, but only include gc-objects (s.a. :Type.__isgc__)") },
	{ "referring", &gcenum_referring,
	  DOC("(to)->?Dset\n"
	      "Returns a set of gc-objects (s.a. :Type.__isgc__) that are referring to @to") },
	{ "isreferring", &gcenum_isreferring,
	  DOC("(from,to)->?Dbool\n"
	      "Returns :true if @to is referred to by @from, or :false otherwise") },
	{ NULL }
};

PRIVATE DeeTypeObject GCEnum_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ DeeString_STR(&str_gc),
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL | TP_FNAMEOBJECT,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_SINGLETON,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ NULL,
				/* .tp_copy_ctor = */ NULL,
				/* .tp_deep_ctor = */ NULL,
				/* .tp_any_ctor  = */ NULL,
				TYPE_FIXED_ALLOCATOR_S(DeeObject)
			}
		},
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &gcenum_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ gcenum_methods,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ gcenum_class_members
};


PUBLIC DeeObject DeeGCEnumTracked_Singleton = {
	OBJECT_HEAD_INIT(&GCEnum_Type)
};

/* GC objects referring to X */
INTERN WUNUSED NONNULL((1, 2)) int DCALL
DeeGC_CollectGCReferred(GCSetMaker *__restrict self,
                        DeeObject *__restrict target) {
	struct gc_head *iter;
again:
	GCLOCK_ACQUIRE_READ();
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
			return -1;
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
}



DECL_END

#endif /* !GUARD_DEEMON_RUNTIME_GC_C */
