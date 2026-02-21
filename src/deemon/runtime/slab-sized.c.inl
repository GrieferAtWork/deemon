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
#include "slab.c"
#define DEFINE_CHUNK_SIZE 32
#endif /* __INTELLISENSE__ */

#include <deemon/api.h>
#include <hybrid/typecore.h>
#include <hybrid/sequence/list.h>
#include <deemon/util/lock.h>

#undef byte_t
#define byte_t __BYTE_TYPE__

#ifndef DEFINE_CHUNK_SIZE
#error "Must #define 'DEFINE_CHUNK_SIZE' before #including this file"
#endif /* !DEFINE_CHUNK_SIZE */

#define LOCAL_SYM(x) PP_CAT2(x, DEFINE_CHUNK_SIZE)

/* Local symbol names */
#define LOCAL_slab_page      LOCAL_SYM(slab_page)
#define LOCAL_slab_page_list LOCAL_SYM(slab_page_list)
#define LOCAL_slab_pages     LOCAL_SYM(slab_pages)
#define LOCAL_slab_lock      LOCAL_SYM(slab_lock)

DECL_BEGIN

/* Select "bitword_t" data type. */
#if __SIZEOF_POINTER__ <= 1
#define LOCAL_bitword_t        __UINT_FAST8_TYPE__
#define LOCAL_SIZEOF_bitword_t __SIZEOF_INT_FAST8_T__
#elif __SIZEOF_POINTER__ <= 2
#define LOCAL_bitword_t        __UINT_FAST16_TYPE__
#define LOCAL_SIZEOF_bitword_t __SIZEOF_INT_FAST16_T__
#elif __SIZEOF_POINTER__ <= 4
#define LOCAL_bitword_t        __UINT_FAST32_TYPE__
#define LOCAL_SIZEOF_bitword_t __SIZEOF_INT_FAST32_T__
#elif __SIZEOF_POINTER__ <= 8
#define LOCAL_bitword_t        __UINT_FAST64_TYPE__
#define LOCAL_SIZEOF_bitword_t __SIZEOF_INT_FAST64_T__
#else /* __SIZEOF_POINTER__ <= ... */
#define LOCAL_bitword_t        __UINTPTR_TYPE__
#define LOCAL_SIZEOF_bitword_t __SIZEOF_POINTER__
#endif /* __SIZEOF_POINTER__ > ... */
#define LOCAL_BITSOF_bitword_t (LOCAL_SIZEOF_bitword_t * __CHAR_BIT__)


/* >> sizeof(struct slab_page::sp_meta) */
#define _LOCAL_SIZEOF__sp_meta (3 * __SIZEOF_POINTER__)

/* Calculate "LOCAL__MAX_CHUNK_COUNT" using 3 successive approximations */
#define _LOCAL_SIZEOF__sp_used__plus__sp_data (Dee_SLAB_PAGESIZE - _LOCAL_SIZEOF__sp_meta)
#define _LOCAL_UPPER__MAX_CHUNK_COUNT         (_LOCAL_SIZEOF__sp_used__plus__sp_data / DEFINE_CHUNK_SIZE)
#define _LOCAL_UPPER_ELEMOF__sp_used          CEILDIV(_LOCAL_UPPER__MAX_CHUNK_COUNT, LOCAL_BITSOF_bitword_t)
#define _LOCAL_UPPER_SIZEOF__sp_used          (_LOCAL_UPPER_ELEMOF__sp_used * LOCAL_SIZEOF_bitword_t)
#define _LOCAL_LOWER_SIZEOF__sp_data          (_LOCAL_SIZEOF__sp_used__plus__sp_data - _LOCAL_UPPER_SIZEOF__sp_used)
#define _LOCAL_LOWER__MAX_CHUNK_COUNT         (_LOCAL_LOWER_SIZEOF__sp_data / DEFINE_CHUNK_SIZE)
#define _LOCAL_LOWER_ELEMOF__sp_used          CEILDIV(_LOCAL_LOWER__MAX_CHUNK_COUNT, LOCAL_BITSOF_bitword_t)
#define _LOCAL_LOWER_SIZEOF__sp_used          (_LOCAL_LOWER_ELEMOF__sp_used * LOCAL_SIZEOF_bitword_t)
#define _LOCAL_LOWER_BITSOF__sp_used          (_LOCAL_LOWER_ELEMOF__sp_used * LOCAL_BITSOF_bitword_t)
#define _LOCAL_FINAL_SIZEOF__sp_data          (_LOCAL_SIZEOF__sp_used__plus__sp_data - _LOCAL_LOWER_SIZEOF__sp_used)
#define LOCAL__MAX_CHUNK_COUNT               (_LOCAL_FINAL_SIZEOF__sp_data / DEFINE_CHUNK_SIZE)
#if LOCAL__MAX_CHUNK_COUNT > _LOCAL_LOWER_BITSOF__sp_used
#undef LOCAL__MAX_CHUNK_COUNT
#define LOCAL__MAX_CHUNK_COUNT _LOCAL_LOWER_BITSOF__sp_used
#endif /* LOCAL__MAX_CHUNK_COUNT > _LOCAL_LOWER_BITSOF__sp_used */
#define LOCAL_ELEMOF__sp_used CEILDIV(LOCAL__MAX_CHUNK_COUNT, LOCAL_BITSOF_bitword_t)
#define LOCAL_SIZEOF__sp_data (LOCAL__MAX_CHUNK_COUNT * DEFINE_CHUNK_SIZE)
#define LOCAL_SIZEOF__sp_used (LOCAL_ELEMOF__sp_used * LOCAL_SIZEOF_bitword_t)
#define LOCAL_BITSOF__sp_used (LOCAL_ELEMOF__sp_used * LOCAL_BITSOF_bitword_t)
#define LOCAL_SIZEOF__sp_pad  (Dee_SLAB_PAGESIZE - (_LOCAL_SIZEOF__sp_meta + LOCAL_SIZEOF__sp_used + LOCAL_SIZEOF__sp_data))



/* Sanity check the calculation above */
STATIC_ASSERT(LOCAL_BITSOF__sp_used == (LOCAL_SIZEOF__sp_used * __CHAR_BIT__));
STATIC_ASSERT(LOCAL_SIZEOF__sp_data == (LOCAL__MAX_CHUNK_COUNT * DEFINE_CHUNK_SIZE));
STATIC_ASSERT(LOCAL__MAX_CHUNK_COUNT <= LOCAL_BITSOF__sp_used);
STATIC_ASSERT((LOCAL_SIZEOF__sp_used + LOCAL_SIZEOF__sp_data +
               LOCAL_SIZEOF__sp_pad + _LOCAL_SIZEOF__sp_meta) == Dee_SLAB_PAGESIZE);

/* Define the "slab_page" structure */
struct LOCAL_slab_page {
	LOCAL_bitword_t sp_used[LOCAL_ELEMOF__sp_used]; /* Bitset of allocated chunk base addresses */
	byte_t          sp_data[LOCAL_SIZEOF__sp_data]; /* Slab payload data */
#if LOCAL_SIZEOF__sp_pad != 0
	byte_t         _sp_pad[LOCAL_SIZEOF__sp_pad];   /* Unused padding */
#endif /* LOCAL_SIZEOF__sp_pad != 0 */
	struct {
		size_t                      sm_used; /* [<= LOCAL__MAX_CHUNK_COUNT][lock(ATOMIC)] # of 1-bits in "sp_used" Must be
		                                      * holding "LOCAL_slab_lock" to change to/from 0/LOCAL__MAX_CHUNK_COUNT. */
		LIST_ENTRY(LOCAL_slab_page) sm_link; /* [0..1][lock(LOCAL_slab_lock)] Next page with free chunks,
		                                      * or ITER_DONE if some custom free mechanism must be used because
		                                      * this page exists within a dec file mapping. */
	} sp_meta; /* Slab metadata */
};

/* Sanity check the page-struct definition */
STATIC_ASSERT(sizeof(struct LOCAL_slab_page) == Dee_SLAB_PAGESIZE);
STATIC_ASSERT(offsetof(struct LOCAL_slab_page, sp_used) == 0);
STATIC_ASSERT(offsetof(struct LOCAL_slab_page, sp_data) == LOCAL_SIZEOF__sp_used);
STATIC_ASSERT(offsetof(struct LOCAL_slab_page, sp_meta) == (Dee_SLAB_PAGESIZE - _LOCAL_SIZEOF__sp_meta));
STATIC_ASSERT_MSG(LOCAL_SIZEOF__sp_pad < (DEFINE_CHUNK_SIZE + LOCAL_SIZEOF_bitword_t),
                  "More padding than this wouldn't make sense, because "
                  "then 1 extra item in `sp_data' should have been used");

LIST_HEAD(LOCAL_slab_page_list, LOCAL_slab_page);

/* Lock for this specific slab-size */
#ifndef CONFIG_NO_THREADS
PRIVATE Dee_atomic_rwlock_t LOCAL_slab_lock = Dee_ATOMIC_RWLOCK_INIT;
#endif /* !CONFIG_NO_THREADS */

/* [0..n][lock(LOCAL_slab_lock)] Pages containing at least 1 free, and at least 1 allocated chunk.
 * - fully allocated pages aren't tracked anywhere
 * - fully free pages are stored in a global free-list (so they can be used by all slab allocators) */
PRIVATE struct LOCAL_slab_page_list LOCAL_slab_pages = LIST_HEAD_INITIALIZER(LOCAL_slab_pages);


/* TODO */


#undef _LOCAL_SIZEOF__sp_used__plus__sp_data
#undef _LOCAL_UPPER__MAX_CHUNK_COUNT
#undef _LOCAL_UPPER_ELEMOF__sp_used
#undef _LOCAL_UPPER_SIZEOF__sp_used
#undef _LOCAL_LOWER_SIZEOF__sp_data
#undef _LOCAL_LOWER__MAX_CHUNK_COUNT
#undef _LOCAL_LOWER_ELEMOF__sp_used
#undef _LOCAL_LOWER_SIZEOF__sp_used
#undef _LOCAL_LOWER_BITSOF__sp_used
#undef _LOCAL_FINAL_SIZEOF__sp_data
#undef LOCAL__MAX_CHUNK_COUNT
#undef LOCAL_ELEMOF__sp_used
#undef LOCAL_SIZEOF__sp_data
#undef LOCAL_SIZEOF__sp_used
#undef LOCAL_BITSOF__sp_used
#undef LOCAL_SIZEOF__sp_pad
#undef LOCAL_bitword_t
#undef LOCAL_SIZEOF_bitword_t
#undef LOCAL_BITSOF_bitword_t

DECL_END

#undef LOCAL_slab_page
#undef LOCAL_slab_page_list
#undef LOCAL_slab_pages
#undef LOCAL_slab_lock
#undef LOCAL_SYM

#undef DEFINE_CHUNK_SIZE
