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
#include "slab-sized.c.inl"
//#define DEFINE_LOCAL_DeeSlab_Malloc
#define DEFINE_LOCAL_DeeSlab_TryMalloc
#endif /* __INTELLISENSE__ */

#include <deemon/api.h>

#include <deemon/system-features.h> /* bzero */
#include <deemon/util/atomic.h>     /* atomic_cmpxch, atomic_read */
#include <deemon/util/slab.h>       /* Dee_slab_page, Dee_slab_page_isnormal */

#include <hybrid/sched/yield.h>   /* SCHED_YIELD */
#include <hybrid/sequence/list.h> /* LIST_* */

#include <stddef.h> /* NULL, size_t */

#if (defined(DEFINE_LOCAL_DeeSlab_Malloc) + \
     defined(DEFINE_LOCAL_DeeSlab_TryMalloc)) != 1
#error "Must #define exactly one of these"
#endif /* ... */

DECL_BEGIN

#ifdef DEFINE_LOCAL_DeeSlab_Malloc
#define LOCAL_MY_DeeSlab_Malloc LOCAL_DeeSlab_Malloc
#define LOCAL_MY_DeeSlab_Calloc LOCAL_DeeSlab_Calloc
#elif defined(DEFINE_LOCAL_DeeSlab_TryMalloc)
#define LOCAL_MY_DeeSlab_Malloc LOCAL_DeeSlab_TryMalloc
#define LOCAL_MY_DeeSlab_Calloc LOCAL_DeeSlab_TryCalloc
#define LOCAL_IS_TRY_MALLOC
#else /* ... */
#error "Invalid configuration"
#endif /* !... */

#ifdef LOCAL_IS_TRY_MALLOC
#define LOCAL_dbg_slab_page_rawmalloc dbg_slab_page_rawtrymalloc
#else /* LOCAL_IS_TRY_MALLOC */
#define LOCAL_dbg_slab_page_rawmalloc dbg_slab_page_rawmalloc
#endif /* !LOCAL_IS_TRY_MALLOC */

LOCAL_DECL ATTR_MALLOC WUNUSED void *DCALL
LOCAL_MY_DeeSlab_Malloc(LOCAL_DeeSlab_Malloc_DBG_PARAMS) {
	void *result;
	struct LOCAL_slab_page *page;
again:
	LOCAL_slab_lock_read(); /* TODO: This lock is a MAJOR bottleneck in heavily parallel programs (10%) */
again_locked:
	page = LIST_FIRST(&LOCAL_slab_pages);
	if (page) {
		size_t old__spm_used;
		slab_assert(page->sp_meta.spm_type.t_link.le_next != page);
		for (;;) {
			old__spm_used = atomic_read(&page->sp_meta.spm_used);
			slab_assert(old__spm_used >= 1);
			slab_assert(old__spm_used <= (LOCAL_MAX_CHUNK_COUNT - 1));
			if unlikely(old__spm_used >= (LOCAL_MAX_CHUNK_COUNT - 1)) {
				/* About to allocate last free chunk of page */
				LOCAL_slab_lock_endread();
				while unlikely(!LOCAL_slab_lock_trywrite()) {
					SCHED_YIELD(); /* TODO: This lock is a MAJOR bottleneck in heavily parallel programs (7%) */
					if (page != atomic_read(&LOCAL_slab_pages.lh_first))
						goto again;
				}
				if unlikely(LIST_FIRST(&LOCAL_slab_pages) != page ||
				            !atomic_cmpxch(&page->sp_meta.spm_used,
				                           LOCAL_MAX_CHUNK_COUNT - 1,
				                           LOCAL_MAX_CHUNK_COUNT)) {
					LOCAL_slab_lock_downgrade();
					goto again_locked;
				}

				/* Remove page from "LOCAL_slab_pages" */
				LIST_REMOVE(page, sp_meta.spm_type.t_link);
#if SLAB_TRACK_FULL_PAGES
				LIST_INSERT_HEAD(&LOCAL_slab_fullpages, page, sp_meta.spm_type.t_link);
#endif /* SLAB_TRACK_FULL_PAGES */
				LOCAL_slab_lock_endwrite();
				break;
			} else if (atomic_cmpxch(&page->sp_meta.spm_used, old__spm_used, old__spm_used + 1)) {
				LOCAL_slab_lock_endread();
				break;
			}
		}

		/* Since we were able to increment "spm_used", that also means that the page
		 * is **GUARANTIED** to have at least 1 0-bit in its `sp_used' bitset! */
		result = LOCAL_slab_malloc_in_page(page);
		slab_assert((byte_t *)result >= (page->sp_data));
		slab_assert((byte_t *)result <= (page->sp_data + sizeof(page->sp_data) - DEFINE_CHUNK_SIZE));

#ifdef SLAB_DEBUG_MEMSET_FREE
		/* Verify that the slab chunk still matches the SLAB_DEBUG_MEMSET_FREE-pattern */
		slab_chkfree_data(result, DEFINE_CHUNK_SIZE);
#endif /* SLAB_DEBUG_MEMSET_FREE */
		goto done;
	}
	LOCAL_slab_lock_endread();

	/* Get a new page (global) */
	page = (struct LOCAL_slab_page *)LOCAL_dbg_slab_page_rawmalloc(DEFINE_CHUNK_SIZE,
	                                                               LOCAL_MAX_CHUNK_COUNT);
	if unlikely(!page)
		return NULL;
	bzero(page->sp_used, sizeof(page->sp_used));
#if LOCAL_SIZEOF__sp_pad != 0
	DBG_memset(page->_sp_pad, 0xcc, sizeof(page->_sp_pad));
#endif /* LOCAL_SIZEOF__sp_pad != 0 */
	slab_setfree_data(page->sp_data, sizeof(page->sp_data));
	page->sp_used[0] = 1;
#if !SLAB_DEBUG_LEAKS
	page->sp_meta.spm_leak = NULL; /* For binary compatibility with non-leaking builds */
#endif /* !SLAB_DEBUG_LEAKS */
	page->sp_meta.spm_used = 1;
	result = page->sp_data;

	/* In theory, this lock acquire could be made non-blocking
	 * by having a insert-reap-list for `LOCAL_slab_pages'. */
	LOCAL_slab_lock_write();
	LIST_INSERT_HEAD(&LOCAL_slab_pages, page, sp_meta.spm_type.t_link);
	slab_assert(Dee_slab_page_isnormal(page));
	LOCAL_slab_lock_endwrite();
done:
	slab_setalloc_data(result, DEFINE_CHUNK_SIZE);
#ifdef LOCAL_DeeSlab_Malloc_DBG_ARGS_PRESENT
	return dbg_slab__attach((struct Dee_slab_page *)page, result, DEFINE_CHUNK_SIZE,
	                        (size_t)(((byte_t *)result - page->sp_data) / DEFINE_CHUNK_SIZE),
	                        file, line);
#else /* LOCAL_DeeSlab_Malloc_DBG_ARGS_PRESENT */
	return dbg_slab__attach((struct Dee_slab_page *)page, result, DEFINE_CHUNK_SIZE,
	                        (size_t)(((byte_t *)result - page->sp_data) / DEFINE_CHUNK_SIZE),
	                        NULL, 0);
#endif /* !LOCAL_DeeSlab_Malloc_DBG_ARGS_PRESENT */
}

LOCAL_DECL ATTR_MALLOC WUNUSED void *DCALL
LOCAL_MY_DeeSlab_Calloc(LOCAL_DeeSlab_Malloc_DBG_PARAMS) {
	void *result = LOCAL_MY_DeeSlab_Malloc(LOCAL_DeeSlab_Malloc_DBG_ARGS);
	if likely(result)
		bzero(result, DEFINE_CHUNK_SIZE);
	return result;
}

#if 0 /* TODO: Idea for lock-less tracking of pages for slab allocators. */
DECL_END
#include <deemon/util/rcu.h>
DECL_BEGIN

union slab_status {
	uintptr_t word;
	struct {
		__UINTPTR_HALF_TYPE__ used;
		__UINTPTR_HALF_TYPE__ flags;
		/* NOTE: Each of these flags *MUST* be cleared by the thread that originally set it */
#define SLAB_STATUS_F_INSERTING 0x0001 /* Some thread is in the process of *adding* this page to `slab_pages` */
#define SLAB_STATUS_F_REMOVEING 0x0002 /* Some thread is in the process of *removing* this page to `slab_pages` */
	} data;
};

struct slab_page {
	struct slab_page *next;
	struct slab_page **p_self;
	union slab_status status;
};

/* pages with at least 1 unused slot (or pages that are currently being removed from this list)
 * NOTE: for the sake of further reducing atomic contention, there can be multiple slab page
 *       base pointers, each of which are searched when memory is allocated from slabs. When
 *       inserting a page into `slab_pages`, only 1 list-`base` is used, based on the calling
 *       thread's ID (if atomics on slab page-list linked lists don't end up accounting for
 *       a sufficient amount of atomic contention, then this `3` can be reduced to `1`, and
 *       the `GET_CURRENT_THREAD_ID` call can be removed) */
struct slab_page_base {
	struct slab_page *base;
	byte_t pad[64 - sizeof(void *)]; /* 64 == CACHELINE_SIZE */
};
struct slab_page_base slab_pages[3] = {{NULL},{NULL},{NULL}};

enum slab_page_track_status {
	SLAB_PAGE_TRACK_STATUS_ISFREE, /* Page was removed because `used == 0` */
	SLAB_PAGE_TRACK_STATUS_ISFULL, /* Page was removed because `used == LOCAL_MAX_CHUNK_COUNT` */
	SLAB_PAGE_TRACK_STATUS_ISUSED, /* Page was added because `used != 0 && used != LOCAL_MAX_CHUNK_COUNT` */
};

/* Do remove `page` from `slab_pages` and clear `SLAB_STATUS_F_REMOVEING` */
static enum slab_page_track_status slab_page_do_remove(struct slab_page *page);

static void abort_fake_remove(struct slab_page *page) {
	union slab_status old_status;
	union slab_status new_status;
	do {
		old_status.word = atomic_read(&page->status.word);
		new_status.word = old_status.word;
		ASSERT(new_status.data.flags & SLAB_STATUS_F_REMOVEING);
		if (new_status.data.used == 0) {
			enum slab_page_track_status track_status;
			track_status = slab_page_do_remove(page);
			if (track_status == SLAB_PAGE_TRACK_STATUS_ISFREE)
				Dee_slab_page_rawfree(page); /* Actually free the page. */
			return;
		}
	} while (!atomic_cmpxch_weak(&page->status.word,
	                             old_status.word,
	                             new_status.word));
}

/* Do insert `page` into `slab_pages` and clear `SLAB_STATUS_F_INSERTING`
 * While clearing `SLAB_STATUS_F_INSERTING`, also check if the page should
 * actually be removed again (in which case do so) */
static enum slab_page_track_status slab_page_do_insert(struct slab_page *page) {
	union slab_status old_status;
	union slab_status new_status;
	struct slab_page *next;
	unsigned int thread_id = GET_CURRENT_THREAD_ID;
	unsigned int pages_index = thread_id % COMPILER_LENOF(slab_pages);
	struct slab_page **p_slab_pages = &slab_pages[pages_index].base;

	page->p_self = p_slab_pages;
again:
	DeeRCU_LockDefault(); /* Lock to prevent `next` or `page` from being free'd */
again_rcu_locked:
	next = atomic_read(p_slab_pages);
	atomic_write(&page->next, next);
	if (!next) {
		/* Simple case: first page */
		if (!atomic_cmpxch_weak(p_slab_pages, next, page))
			goto again_rcu_locked;
		DeeRCU_UnlockDefault();
	} else {
		union slab_status next_old_status;
		union slab_status next_new_status;

		/* Start a (fake) REMOVE operation on `next` to prevent anything from
		 * starting another (that might read its `p_self` at an inopportune
		 * time) while we're trying to modify that field. */
		do {
			next_old_status.word = atomic_read(&next->status.word);
			next_new_status.word = next_old_status.word;
			if (next_new_status.data.flags & SLAB_STATUS_F_REMOVEING) {
				DeeRCU_UnlockDefault();
				SCHED_YIELD(); /* Wait for a real (or another fake) REMOVE operation */
				goto again;
			}
			next_new_status.data.flags |= SLAB_STATUS_F_REMOVEING;
		} while (!atomic_cmpxch_weak(&next->status.word,
		                             next_old_status.word,
		                             next_new_status.word));
		if unlikely(atomic_read(&next->p_self) != p_slab_pages) {
abort_fake_remove_on_next:
			DeeRCU_UnlockDefault();
			abort_fake_remove(next);
			goto again;
		}
		atomic_write(&next->p_self, &page->next);
		if (!atomic_cmpxch_weak(p_slab_pages, next, page)) {
			atomic_write(&next->p_self, p_slab_pages);
			goto abort_fake_remove_on_next;
		}
		DeeRCU_UnlockDefault();
		abort_fake_remove(next);
	}

	do {
		old_status.word = atomic_read(&page->status.word);
		ASSERT(old_status.data.flags & SLAB_STATUS_F_INSERTING);
		new_status.data.used  = old_status.data.used;
		new_status.data.flags = old_status.data.flags & ~SLAB_STATUS_F_INSERTING;
		if (new_status.data.used == 0 || new_status.data.used == LOCAL_MAX_CHUNK_COUNT)
			new_status.data.flags |= SLAB_STATUS_F_REMOVEING; /* Must start a REMOVE operation */
	} while (!atomic_cmpxch_weak(&page->status.word, old_status.word, new_status.word));

	/* If we started a REMOVE operation, complete it now */
	if ((new_status.data.flags & SLAB_STATUS_F_REMOVEING) != 0 &&
	    (old_status.data.flags & SLAB_STATUS_F_REMOVEING) == 0)
		return slab_page_do_remove(page);

	/* Is now part of `slab_pages` list */
	return SLAB_PAGE_TRACK_STATUS_ISUSED;
}

/* Do remove `page` from `slab_pages` and clear `SLAB_STATUS_F_REMOVEING`.
 * While clearing `SLAB_STATUS_F_REMOVEING`, also check if the page should
 * actually be inserted again (in which case do so) */
static enum slab_page_track_status slab_page_do_remove(struct slab_page *page) {
	union slab_status old_status;
	union slab_status new_status;

	/* REMOVE `page` form list */
	{
		struct slab_page **p_self = atomic_read(&page->p_self);
		struct slab_page *next = atomic_read(&page->next);
		atomic_write(p_self, next);
	}

	/* Sync with threads that are enumerating `slab_pages`
	 * (to ensure that no-one is still looking at our `page`) */
	DeeRCU_SynchronizeDefault();

	do {
		old_status.word = atomic_read(&page->status.word);
		ASSERT(old_status.data.flags & SLAB_STATUS_F_REMOVEING);
		new_status.data.used  = old_status.data.used;
		new_status.data.flags = old_status.data.flags & ~SLAB_STATUS_F_REMOVEING;
		if (new_status.data.used != 0 && new_status.data.used != LOCAL_MAX_CHUNK_COUNT)
			new_status.data.flags |= SLAB_STATUS_F_INSERTING; /* Must start an INSERT operation */
	} while (!atomic_cmpxch_weak(&page->status.word, old_status.word, new_status.word));

	/* If we started a INSERT operation, complete it now */
	if ((new_status.data.flags & SLAB_STATUS_F_INSERTING) != 0 &&
	    (old_status.data.flags & SLAB_STATUS_F_INSERTING) == 0)
		return slab_page_do_insert(page);

	/* No longer part of `slab_pages` list (can have 2 reasons) */
	if (new_status.data.used == 0)
		return SLAB_PAGE_TRACK_STATUS_ISFREE;
	return SLAB_PAGE_TRACK_STATUS_ISFULL;
}


void *slab_alloc(void) {
	void *result;
	struct slab_page *page;
	unsigned int tid_index;
	DeeRCU_LockDefault();
	for (tid_index = 0; tid_index < COMPILER_LENOF(slab_pages); ++tid_index) {
		for (page = atomic_read(&slab_pages[tid_index].base);
		     page; page = atomic_read(&page->next)) {
			union slab_status old_status;
			union slab_status new_status;

			/* Try to allocate a slot in `page` */
again_read_page_status:
			old_status.word = page->status.word;
			if (old_status.data.used == 0)
				continue; /* Skip this page! */
			new_status.data.used  = old_status.data.used + 1;
			new_status.data.flags = old_status.data.flags;
			if (new_status.data.used == LOCAL_MAX_CHUNK_COUNT) {
				/* About to allocate last chunk -> must remove page from `slab_pages` */
				new_status.data.flags |= SLAB_STATUS_F_REMOVEING;
			}
			if (!atomic_cmpxch_weak(&page->status.word, old_status.word, new_status.word))
				goto again_read_page_status;
			DeeRCU_UnlockDefault();
			if ((old_status.data.flags & SLAB_STATUS_F_REMOVEING) == 0 &&
			    (new_status.data.flags & SLAB_STATUS_F_REMOVEING) != 0) {
				enum slab_page_track_status track_status;
				track_status = slab_page_do_remove(page);
				ASSERTF(track_status != SLAB_PAGE_TRACK_STATUS_ISFREE,
				        "Can't be the case because we increased `used` above by 1, "
				        "which no other thread should be able to undo (since that "
				        "`+1` is the marker for the allocation we just made)");
			}
			return ALLOCATE_KNOWN_AVAILABLE_CHUNK_IN_PAGE(page);
		}
	}
	DeeRCU_UnlockDefault();

	/* Allocate new page. */
	page = Dee_slab_page_rawmalloc();
	if unlikely(!page)
		return NULL;
	result = ALLOCATE_FIRST_CHUNK_IN_PAGE(page);
	page->status.data.used  = 1;
	page->status.data.flags = SLAB_STATUS_F_INSERTING;
	{
		enum slab_page_track_status track_status;
		track_status = slab_page_do_insert(page);
		ASSERTF(track_status != SLAB_PAGE_TRACK_STATUS_ISFREE,
		        "Could only be the case if another thread did `slab_free(result)`, "
		        "but since `result` is considered `__restrict`, that can't happen "
		        "(no other thread should even know the address of `result`)");
	}
	return result;
}


void slab_free(void *p) {
	union slab_status old_status;
	union slab_status new_status;
	struct slab_page *page = PAGE_OF(p);
	SET_ALLOCATED_IN_PAGE(page, p, false);

	for (;;) {
		old_status.word = atomic_read(&page->status.word);
		ASSERT(old_status.data.used >= 1);
		if (old_status.data.used == 1) {
			/* Last chunk of page is being deleted -> must remove from list of pages with free chunks */
			if (Dee_slab_page_iscustom(page)) {
				/* Invoke custom page-free callback */
				(*page->sp_meta.spm_type.t_custom.c_free)(page);
				return;
			}

			/* Only start a REMOVE operation if no INSERT is happening */
			if (!(old_status.data.flags & SLAB_STATUS_F_INSERTING)) {
				enum slab_page_track_status track_status;
				new_status.data.used  = 0;
				new_status.data.flags = old_status.data.flags;
				new_status.data.flags |= SLAB_STATUS_F_REMOVEING;
				if (!atomic_cmpxch_weak(&page->status.word, old_status.word, new_status.word))
					continue;
				if (old_status.data.flags & SLAB_STATUS_F_REMOVEING)
					return; /* Another thread is already removing the page */

				track_status = slab_page_do_remove(page);
				if (track_status == SLAB_PAGE_TRACK_STATUS_ISFREE)
					Dee_slab_page_rawfree(page); /* Actually free the page. */
				break;
			}
		} else if (old_status.data.used == LOCAL_MAX_CHUNK_COUNT &&
		           Dee_slab_page_isnormal(page)) {
			/* Only start a INSERT operation if no REMOVE operation is happening */
			if (!(old_status.data.flags & SLAB_STATUS_F_REMOVEING)) {
				/* First chunk of page free'd -> must add to list of pages with free chunks */
				new_status.data.used  = LOCAL_MAX_CHUNK_COUNT - 1;
				new_status.data.flags = old_status.data.flags;
				new_status.data.flags |= SLAB_STATUS_F_INSERTING;
				if (!atomic_cmpxch_weak(&page->status.word, old_status.word, new_status.word))
					continue;
				if (!(old_status.data.flags |= SLAB_STATUS_F_INSERTING)) {
					/* We're responsible for ADD-ing the slap to `slab_pages` */
					slab_page_do_insert(page);
				}
				break;
			}
		}

		/* Update "used" counter, but don't do anything else */
		new_status.data.used  = old_status.data.used - 1;
		new_status.data.flags = old_status.data.flags;
		if (atomic_cmpxch_weak(&page->status.word,
		                       old_status.word,
		                       new_status.word))
			break;
	}
}

#endif

#undef LOCAL_dbg_slab_page_rawmalloc

#undef LOCAL_IS_TRY_MALLOC
#undef LOCAL_MY_DeeSlab_Malloc
#undef LOCAL_MY_DeeSlab_Calloc

DECL_END

#undef DEFINE_LOCAL_DeeSlab_Malloc
#undef DEFINE_LOCAL_DeeSlab_TryMalloc
