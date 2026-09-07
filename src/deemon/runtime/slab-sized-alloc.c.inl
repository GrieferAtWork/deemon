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
#include <deemon/util/atomic.h>     /* atomic_* */
#include <deemon/util/rcu.h>        /* DeeRCU_* */
#include <deemon/util/slab.h>       /* Dee_SLAB_*, Dee_slab_page, Dee_slab_page_isnormal, Dee_slab_page_status */

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
#ifdef CONFIG_EXPERIMENTAL_LOCKLESS_SLAB_ALLOCATOR
	DeeRCU_FAST_SETUP
	DeeRCU_FAST_LockDefault();
	/* TODO: Simplification under CONFIG_NO_THREADS */
	{
		struct LOCAL_slab_page_list *list = (struct LOCAL_slab_page_list *)&LOCAL_slab.s_pages;
		for (page = atomic_read(&list->lh_first);
		     page; page = atomic_read(&page->sp_meta.spm_type.t_link.le_next)) {
			union Dee_slab_page_status old_status;
			union Dee_slab_page_status new_status;

			/* Try to allocate a slot in `page` */
again_read_page_status:
			old_status.sps_word = atomic_read(&page->sp_meta.spm_status.sps_word);
			slab_assert(old_status.sps_data.spsd_used <= LOCAL_MAX_CHUNK_COUNT);
			if (old_status.sps_data.spsd_used >= LOCAL_MAX_CHUNK_COUNT)
				continue; /* Skip this page! */
			new_status.sps_data.spsd_used = old_status.sps_data.spsd_used + 1;
			new_status.sps_data.spsd_act  = old_status.sps_data.spsd_act;
			if (new_status.sps_data.spsd_used == LOCAL_MAX_CHUNK_COUNT &&
			    new_status.sps_data.spsd_act == Dee_SLAB_PAGE_ACT_NONE) {
				/* About to allocate last chunk -> must start a REMOVE operation */
				new_status.sps_data.spsd_act = Dee_SLAB_PAGE_ACT_REMOVE;
			}
			if (!atomic_cmpxch_weak(&page->sp_meta.spm_status.sps_word,
			                        old_status.sps_word, new_status.sps_word))
				goto again_read_page_status;
			DeeRCU_UnlockDefault();
			if (new_status.sps_data.spsd_act != old_status.sps_data.spsd_act) {
				slab_assert(new_status.sps_data.spsd_act == Dee_SLAB_PAGE_ACT_REMOVE);
				slab_act_remove(&LOCAL_slab,
				                (struct Dee_slab_page_list *)list,
				                (struct Dee_slab_page *)page);
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
	}
	DeeRCU_FAST_UnlockDefault();
#else /* CONFIG_EXPERIMENTAL_LOCKLESS_SLAB_ALLOCATOR */
again:
	LOCAL_slab_lock_read(); /* XXX: This lock is a MAJOR bottleneck in heavily parallel programs (10%) */
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
					SCHED_YIELD(); /* XXX: This lock is a MAJOR bottleneck in heavily parallel programs (7%) */
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
#endif /* !CONFIG_EXPERIMENTAL_LOCKLESS_SLAB_ALLOCATOR */

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
	result = page->sp_data;
#ifdef CONFIG_EXPERIMENTAL_LOCKLESS_SLAB_ALLOCATOR
	page->sp_meta.spm_status.sps_data.spsd_used = 1;
	page->sp_meta.spm_status.sps_data.spsd_act  = Dee_SLAB_PAGE_ACT_INSERT;
	slab_act_insert(&LOCAL_slab, &LOCAL_slab.s_pages, (struct Dee_slab_page *)page);
#else /* CONFIG_EXPERIMENTAL_LOCKLESS_SLAB_ALLOCATOR */
	page->sp_meta.spm_used = 1;
	/* In theory, this lock acquire could be made non-blocking
	 * by having a insert-reap-list for `LOCAL_slab_pages'. */
	LOCAL_slab_lock_write();
	LIST_INSERT_HEAD(&LOCAL_slab_pages, page, sp_meta.spm_type.t_link);
	slab_assert(Dee_slab_page_isnormal(page));
	LOCAL_slab_lock_endwrite();
#endif /* !CONFIG_EXPERIMENTAL_LOCKLESS_SLAB_ALLOCATOR */
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

#undef LOCAL_dbg_slab_page_rawmalloc

#undef LOCAL_IS_TRY_MALLOC
#undef LOCAL_MY_DeeSlab_Malloc
#undef LOCAL_MY_DeeSlab_Calloc

DECL_END

#undef DEFINE_LOCAL_DeeSlab_Malloc
#undef DEFINE_LOCAL_DeeSlab_TryMalloc
