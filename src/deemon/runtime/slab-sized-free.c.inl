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

#include <deemon/system-features.h> /* bzero */
#include <deemon/util/atomic.h>     /* atomic_cmpxch, atomic_read */
#include <deemon/util/slab.h>       /* Dee_slab_page_* */

#include <hybrid/sched/yield.h>   /* SCHED_YIELD */
#include <hybrid/sequence/list.h> /* LIST_* */

#include <stddef.h> /* NULL, size_t */

DECL_BEGIN

LOCAL_DECL NONNULL((1)) void DCALL
LOCAL_DeeSlab_Free(void *__restrict p LOCAL_DeeSlab_Free__DBG_PARAMS) {
	struct LOCAL_slab_page *page;
	size_t offset;
	size_t index;
	size_t bit_indx;
	bitword_t bit_mask;
	size_t old__spm_used;
#if SLAB_DEBUG_LEAKS
#ifdef LOCAL_DeeSlab_Free__DBG_PARAMS_PRESENT
	p = dbg_slab__detach(p, DEFINE_CHUNK_SIZE, file, line);
#else /* LOCAL_DeeSlab_Free__DBG_PARAMS_PRESENT */
	p = dbg_slab__detach(p, DEFINE_CHUNK_SIZE, NULL, 0);
#endif /* !LOCAL_DeeSlab_Free__DBG_PARAMS_PRESENT */
#endif /* SLAB_DEBUG_LEAKS */

	/* Figure out the slab-context of "p" */
	page     = (struct LOCAL_slab_page *)((uintptr_t)p & ~(Dee_SLAB_PAGESIZE - 1));
	offset   = (byte_t *)p - (byte_t *)&page->sp_data;
	index    = offset / DEFINE_CHUNK_SIZE;
	bit_indx = index / BITSOF_bitword_t;
	bit_mask = (bitword_t)1 << (index % BITSOF_bitword_t);
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

	/* Fill chunk with the free-memory pattern */
#ifdef SLAB_DEBUG_MEMSET_FREE
	slab_setfree_data(p, DEFINE_CHUNK_SIZE);
	COMPILER_WRITE_BARRIER();
#endif /* SLAB_DEBUG_MEMSET_FREE */

	/* Mark chunk as available */
	atomic_and(&page->sp_used[bit_indx], ~bit_mask);

	/* Update the page's `spm_used' counter. */
	slab_assert(page->sp_meta.spm_type.t_link.le_next != page);
	do {
again_read__spm_used:
		old__spm_used = atomic_read(&page->sp_meta.spm_used);
		slab_assert(old__spm_used >= 1);
		slab_assert(old__spm_used <= LOCAL_MAX_CHUNK_COUNT);
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
			if (!atomic_cmpxch(&page->sp_meta.spm_used, 1, 0)) {
				LOCAL_slab_lock_endwrite();
				goto again_read__spm_used;
			}
			slab_assert(LIST_ISBOUND(page, sp_meta.spm_type.t_link));
			LIST_REMOVE(page, sp_meta.spm_type.t_link);
			LOCAL_slab_lock_endwrite();
			/* Ensure page is now free */
			Dee_slab_page_rawfree((struct Dee_slab_page *)page);
			break;
		} else if (old__spm_used == LOCAL_MAX_CHUNK_COUNT &&
		           Dee_slab_page_isnormal(page)) {
			/* First free in previously fully allocated slab (may not actually
			 * be the case when this is a dec chunk, but in that case, we simply
			 * acquire "LOCAL_slab_lock" for no reason, and don't end up doing
			 * anything with it below) */
			LOCAL_slab_lock_write();
			if (!atomic_cmpxch(&page->sp_meta.spm_used,
			                   LOCAL_MAX_CHUNK_COUNT,
			                   LOCAL_MAX_CHUNK_COUNT - 1)) {
				LOCAL_slab_lock_endwrite();
				goto again_read__spm_used;
			}
			slab_assert(Dee_slab_page_isnormal(page));
			LIST_INSERT_HEAD(&LOCAL_slab_pages, page, sp_meta.spm_type.t_link);
			LOCAL_slab_lock_endwrite();
			break;
		}
	} while (!atomic_cmpxch_weak(&page->sp_meta.spm_used, old__spm_used, old__spm_used - 1));
}

DECL_END
