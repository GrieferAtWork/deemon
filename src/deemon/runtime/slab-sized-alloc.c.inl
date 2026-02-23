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
#include <deemon/util/slab.h>       /* Dee_slab_page_isnormal */

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
#elif defined(DEFINE_LOCAL_DeeSlab_TryMalloc)
#define LOCAL_MY_DeeSlab_Malloc LOCAL_DeeSlab_TryMalloc
#define LOCAL_IS_TRY_MALLOC
#else /* ... */
#error "Invalid configuration"
#endif /* !... */

#ifdef LOCAL_IS_TRY_MALLOC
#define LOCAL_slab_page_malloc slab_page_trymalloc
#else /* LOCAL_IS_TRY_MALLOC */
#define LOCAL_slab_page_malloc slab_page_malloc
#endif /* !LOCAL_IS_TRY_MALLOC */

LOCAL_DECL ATTR_MALLOC WUNUSED void *DCALL
LOCAL_MY_DeeSlab_Malloc(void) {
	void *result;
	struct LOCAL_slab_page *page;
again:
	LOCAL_slab_lock_read();
again_locked:
	page = LIST_FIRST(&LOCAL_slab_pages);
	if (page) {
		size_t old__spm_used;
		ASSERT(page->sp_meta.spm_type.t_link.le_next != page);
		for (;;) {
			old__spm_used = atomic_read(&page->sp_meta.spm_used);
			ASSERT(old__spm_used >= 1);
			ASSERT(old__spm_used <= (LOCAL_MAX_CHUNK_COUNT - 1));
			if unlikely (old__spm_used >= (LOCAL_MAX_CHUNK_COUNT - 1)) {
				/* About to allocate last free chunk of page */
				LOCAL_slab_lock_endread();
				while (!LOCAL_slab_lock_trywrite()) {
					SCHED_YIELD();
					if (page != atomic_read(&LOCAL_slab_pages.lh_first))
						goto again;
				}
				if (LIST_FIRST(&LOCAL_slab_pages) != page ||
				    !atomic_cmpxch(&page->sp_meta.spm_used,
				                   LOCAL_MAX_CHUNK_COUNT - 1,
				                   LOCAL_MAX_CHUNK_COUNT)) {
					LOCAL_slab_lock_downgrade();
					goto again_locked;
				}

				/* Remove page from "LOCAL_slab_pages" */
				LIST_REMOVE(page, sp_meta.spm_type.t_link);
				LOCAL_slab_lock_endwrite();
				break;
			} else if (atomic_cmpxch(&page->sp_meta.spm_used, old__spm_used, old__spm_used + 1)) {
				LOCAL_slab_lock_endread();
				break;
			}
		}

		/* Since we were able to increment "spm_used", that also means that the page
		 * is **GUARANTIED** to have at least 1 0-bit in its `sp_used' bitset! */
#if defined(SLAB_DEBUG_MEMSET_ALLOC) || defined(SLAB_DEBUG_MEMSET_FREE)
		result = LOCAL_slab_malloc_in_page(page);
		/* Verify that the slab chunk still matches the SLAB_DEBUG_MEMSET_FREE-pattern */
		slab_chkfree_data(result, DEFINE_CHUNK_SIZE);
		goto done;
#else /* SLAB_DEBUG_MEMSET_ALLOC || SLAB_DEBUG_MEMSET_FREE */
		return LOCAL_slab_malloc_in_page(page);
#endif /* !SLAB_DEBUG_MEMSET_ALLOC && !SLAB_DEBUG_MEMSET_FREE */
	}
	LOCAL_slab_lock_endread();

	/* Get a new page (global) */
	page = (struct LOCAL_slab_page *)LOCAL_slab_page_malloc();
	if unlikely(!page)
		return NULL;
	bzero(page->sp_used, sizeof(page->sp_used));
#if LOCAL_SIZEOF__sp_pad != 0
	DBG_memset(page->_sp_pad, 0xcc, sizeof(page->_sp_pad));
#endif /* LOCAL_SIZEOF__sp_pad != 0 */
	slab_setfree_data(page->sp_data, sizeof(page->sp_data));
	page->sp_used[0]       = 1;
	page->sp_meta.spm_used = 1;
	result = page->sp_data;

	/* In theory, this lock acquire could be made non-blocking
	 * by having a insert-reap-list for `LOCAL_slab_pages'. */
	LOCAL_slab_lock_write();
	LIST_INSERT_HEAD(&LOCAL_slab_pages, page, sp_meta.spm_type.t_link);
	ASSERT(Dee_slab_page_isnormal(page));
	LOCAL_slab_lock_endwrite();
#if defined(SLAB_DEBUG_MEMSET_ALLOC) || defined(SLAB_DEBUG_MEMSET_FREE)
done:
#endif /* SLAB_DEBUG_MEMSET_ALLOC || SLAB_DEBUG_MEMSET_FREE */
	slab_setalloc_data(result, DEFINE_CHUNK_SIZE);
	return result;
}

#undef LOCAL_slab_page_malloc

#undef LOCAL_IS_TRY_MALLOC
#undef LOCAL_MY_DeeSlab_Malloc

DECL_END

#undef DEFINE_LOCAL_DeeSlab_Malloc
#undef DEFINE_LOCAL_DeeSlab_TryMalloc
