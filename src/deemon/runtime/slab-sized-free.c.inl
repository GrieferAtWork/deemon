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
#endif /* __INTELLISENSE__ */

#include <deemon/api.h>

#include <deemon/util/atomic.h> /* atomic_* */
#include <deemon/util/slab.h>   /* Dee_SLAB_*, Dee_slab_page, Dee_slab_page_* */

#include <hybrid/sequence/list.h> /* LIST_* */

#include <stddef.h> /* NULL, size_t */

DECL_BEGIN

LOCAL_DECL NONNULL((1)) void DCALL
LOCAL_DeeSlab_Free(void *__restrict p LOCAL_DeeSlab_Free__DBG_PARAMS) {
	struct LOCAL_slab_page *page;
	size_t offset;
	size_t index;
	size_t bit_indx;
	slab_bitword_t bit_mask;
#ifdef CONFIG_EXPERIMENTAL_LOCKLESS_SLAB_ALLOCATOR
	union Dee_slab_page_status old_status;
	union Dee_slab_page_status new_status;
#else /* CONFIG_EXPERIMENTAL_LOCKLESS_SLAB_ALLOCATOR */
	size_t old__spm_used;
#endif /* !CONFIG_EXPERIMENTAL_LOCKLESS_SLAB_ALLOCATOR */

	/* Figure out the slab-context of "p" */
	page     = (struct LOCAL_slab_page *)((uintptr_t)p & ~(Dee_SLAB_PAGESIZE - 1));
	offset   = (size_t)((byte_t *)p - (byte_t *)page->sp_data);
	index    = offset / DEFINE_CHUNK_SIZE;
	bit_indx = index / BITSOF_slab_bitword_t;
	bit_mask = (slab_bitword_t)1 << (index % BITSOF_slab_bitword_t);
#if SLAB_DEBUG_EXTERNAL
#ifdef LOCAL_DeeSlab_Free__DBG_PARAMS_PRESENT
	if unlikely((offset % DEFINE_CHUNK_SIZE) != 0) {
		_DeeAssert_XFailf(PP_STR(LOCAL_DeeSlab_Free) "(p)", file, line,
		                  "Badly aligned slab pointer: %p", p);
	}
	if unlikely((atomic_read(&page->sp_used[bit_indx]) & bit_mask) == 0) {
		_DeeAssert_XFailf(PP_STR(LOCAL_DeeSlab_Free) "(p)", file, line,
		                  "Pointer not allocated: %p", p);
	}
#else /* LOCAL_DeeSlab_Free__DBG_PARAMS_PRESENT */
	ASSERTF((offset % DEFINE_CHUNK_SIZE) == 0,
	        PP_STR(LOCAL_DeeSlab_Free) ": Badly aligned slab pointer: %p", p);
	ASSERTF((atomic_read(&page->sp_used[bit_indx]) & bit_mask) != 0,
	        PP_STR(LOCAL_DeeSlab_Free) ": Pointer not allocated: %p", p);
#endif /* !LOCAL_DeeSlab_Free__DBG_PARAMS_PRESENT */
#endif /* SLAB_DEBUG_EXTERNAL */

#if SLAB_DEBUG_LEAKS
	/* Detach debug info from normal slab pages. */
	if (Dee_slab_page_isnormal(page)) {
#ifdef LOCAL_DeeSlab_Free__DBG_PARAMS_PRESENT
		p = dbg_slab__detach((struct Dee_slab_page *)page, p,
		                     DEFINE_CHUNK_SIZE, index, file, line);
#else /* LOCAL_DeeSlab_Free__DBG_PARAMS_PRESENT */
		p = dbg_slab__detach((struct Dee_slab_page *)page, p,
		                     DEFINE_CHUNK_SIZE, index, NULL, 0);
#endif /* !LOCAL_DeeSlab_Free__DBG_PARAMS_PRESENT */
	}
#endif /* SLAB_DEBUG_LEAKS */

	/* Fill chunk with the free-memory pattern */
#ifdef SLAB_DEBUG_MEMSET_FREE
	slab_setfree_data(p, DEFINE_CHUNK_SIZE);
	COMPILER_WRITE_BARRIER();
#endif /* SLAB_DEBUG_MEMSET_FREE */

	/* Mark chunk as available */
	atomic_and(&page->sp_used[bit_indx], ~bit_mask);

	/* Update the page's `spm_used` counter. */
#ifdef CONFIG_EXPERIMENTAL_LOCKLESS_SLAB_ALLOCATOR
again_read_status: /* TODO: Simplification under CONFIG_NO_THREADS */
	old_status.sps_word = atomic_read(&page->sp_meta.spm_status.sps_word);
	ASSERT(old_status.sps_data.spsd_used >= 1);
	if (old_status.sps_data.spsd_used == 1) {
		/* Last chunk of page is being deleted -> must remove from list of pages with free chunks */
		if (Dee_slab_page_iscustom(page)) {
			/* Invoke custom page-free callback */
			(*page->sp_meta.spm_type.t_custom.c_free)(page);
			return;
		}

		/* Only start a REMOVE operation if no other operation is happening */
		if (old_status.sps_data.spsd_act == Dee_SLAB_PAGE_ACT_NONE) {
			new_status.sps_data.spsd_used = 0;
			new_status.sps_data.spsd_act  = Dee_SLAB_PAGE_ACT_REMOVE;
			if (!atomic_cmpxch_weak(&page->sp_meta.spm_status.sps_word,
			                        old_status.sps_word, new_status.sps_word))
				goto again_read_status;
			/* Given `page` *must* be part of `s_pages` (as opposed to `s_fullpages`)
			 * because its old action was `Dee_SLAB_PAGE_ACT_NONE`, which could have
			 * only been the case if it's part of the correct list. */
			slab_act_remove(&LOCAL_slab, &LOCAL_slab.s_pages, (struct Dee_slab_page *)page, NULL);
			return;
		}
	} else if (old_status.sps_data.spsd_used == LOCAL_MAX_CHUNK_COUNT &&
	           old_status.sps_data.spsd_act == Dee_SLAB_PAGE_ACT_NONE &&
	           Dee_slab_page_isnormal(page)) {
		/* Either INSERT into `s_pages`, or REMOVE from `s_fullpages` (and then INSERT into `s_pages`) */
		new_status.sps_data.spsd_used = LOCAL_MAX_CHUNK_COUNT - 1;
#if SLAB_TRACK_FULL_PAGES
		new_status.sps_data.spsd_act = Dee_SLAB_PAGE_ACT_REMOVE; /* Remove from `s_fullpages` */
#else /* SLAB_TRACK_FULL_PAGES */
		new_status.sps_data.spsd_act = Dee_SLAB_PAGE_ACT_INSERT; /* Insert into `s_pages` */
#endif /* !SLAB_TRACK_FULL_PAGES */
		if (!atomic_cmpxch_weak(&page->sp_meta.spm_status.sps_word,
		                        old_status.sps_word, new_status.sps_word))
			goto again_read_status;
#if SLAB_TRACK_FULL_PAGES
		slab_act_remove(&LOCAL_slab, &LOCAL_slab.s_fullpages, (struct Dee_slab_page *)page, NULL);
#else /* SLAB_TRACK_FULL_PAGES */
		slab_act_insert(&LOCAL_slab, &LOCAL_slab.s_pages, (struct Dee_slab_page *)page, NULL);
#endif /* !SLAB_TRACK_FULL_PAGES */
		return;
	}

	/* Update "used" counter, but don't have to do
	 * anything else (no action needs to be started) */
	new_status.sps_data.spsd_used = old_status.sps_data.spsd_used - 1;
	new_status.sps_data.spsd_act  = old_status.sps_data.spsd_act; /* Keep current action (or `Dee_SLAB_PAGE_ACT_NONE`) going */
	if (!atomic_cmpxch_weak(&page->sp_meta.spm_status.sps_word,
	                        old_status.sps_word, new_status.sps_word))
		goto again_read_status;
#else /* CONFIG_EXPERIMENTAL_LOCKLESS_SLAB_ALLOCATOR */
	slab_assert(page->sp_meta.spm_type.t_link.le_next != page);
	do {
again_read__spm_used:
		old__spm_used = atomic_read(&page->sp_meta.spm_used);
		slab_assert(old__spm_used >= 1);
		slab_assertf(old__spm_used <= LOCAL_MAX_CHUNK_COUNT ||
		             Dee_slab_page_iscustom(page),
		             "custom slabs may be mixed-size (in which case a 'old__spm_used' may "
		             "be greater than the maximum possible for a page of our slab's size), "
		             "but for regular slabs, the max mustn't be exceeded");
		if (old__spm_used == 1) {
			/* Last chunk of page is being deleted. */
			if (Dee_slab_page_iscustom(page)) {
				/* Invoke custom page-free callback */
				(*page->sp_meta.spm_type.t_custom.c_free)(page);
				return;
			}

			/* In theory, this lock acquire could be made non-blocking by having a pending-free-list
			 * However, this part will be left out initially, unless it turns out that this ends up
			 * being a bottleneck once the new impl is being run in a heavily parallelized environment */
			LOCAL_slab_lock_write();
			slab_assert(!Dee_slab_page_iscustom(page));
			if unlikely(!atomic_cmpxch(&page->sp_meta.spm_used, 1, 0)) {
				LOCAL_slab_lock_endwrite();
				goto again_read__spm_used;
			}
			slab_assert(LIST_ISBOUND(page, sp_meta.spm_type.t_link));
			LIST_REMOVE(page, sp_meta.spm_type.t_link);
			LOCAL_slab_lock_endwrite();
			/* Ensure page is now free */
			dbg_slab_page_rawfree((struct Dee_slab_page *)page,
			                      DEFINE_CHUNK_SIZE,
			                      LOCAL_MAX_CHUNK_COUNT);
			break;
		} else if (old__spm_used == LOCAL_MAX_CHUNK_COUNT &&
		           Dee_slab_page_isnormal(page)) {
			/* First free in previously fully allocated slab (may not actually
			 * be the case when this is a dec chunk, but in that case, we simply
			 * acquire "LOCAL_slab_lock" for no reason, and don't end up doing
			 * anything with it below) */
			LOCAL_slab_lock_write(); /* XXX: This lock is a MAJOR bottleneck in heavily parallel programs (15%) */
			if unlikely(!atomic_cmpxch(&page->sp_meta.spm_used,
			                           LOCAL_MAX_CHUNK_COUNT,
			                           LOCAL_MAX_CHUNK_COUNT - 1)) {
				LOCAL_slab_lock_endwrite();
				goto again_read__spm_used;
			}
			slab_assert(Dee_slab_page_isnormal(page));
#if SLAB_TRACK_FULL_PAGES
			LIST_REMOVE(page, sp_meta.spm_type.t_link); /* Remove from "LOCAL_slab_fullpages" */
#endif /* SLAB_TRACK_FULL_PAGES */
			LIST_INSERT_HEAD(&LOCAL_slab_pages, page, sp_meta.spm_type.t_link);
			LOCAL_slab_lock_endwrite();
			break;
		}
	} while (!atomic_cmpxch_weak(&page->sp_meta.spm_used, old__spm_used, old__spm_used - 1));
#endif /* !CONFIG_EXPERIMENTAL_LOCKLESS_SLAB_ALLOCATOR */
}

DECL_END
