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
/*!export **/
/*!export Dee_slab_page_**/
/*!export Dee_SLAB_**/
#ifndef GUARD_DEEMON_UTIL_SLAB_H
#define GUARD_DEEMON_UTIL_SLAB_H 1 /*!export-*/

#include "../api.h"

#ifdef CONFIG_EXPERIMENTAL_REWORKED_SLAB_ALLOCATOR
#include <hybrid/host.h>          /* __ARCH_PAGESIZE */
#include <hybrid/sequence/list.h> /* LIST_ENTRY */
#include <hybrid/typecore.h>      /* __*_TYPE__, __SIZEOF_*__ */

#include "../system-features.h" /* bzero */
#include "slab-config.h"        /* Dee_SLAB_CHUNKSIZE_MAX */

#include <stddef.h> /* size_t */
#include <stdint.h> /* uintptr_t */

/* ==== Discussion on slabs
 *
 * "CONFIG_EXPERIMENTAL_REWORKED_GC" now means that:
 * - "tp_free" is **always** called on an object's original type
 * - Previously "tp_free" always had to be able to also free objects of sub-classes,
 *   where there was a possibility that those objects were larger than the object
 *   some "tp_free" was actually designed for.
 * - This put a **HUGE** dampener on our slab allocator, since free functions still
 *   had to check the actual object size, and forward to a different free function
 *   if the size doesn't match
 * --> This is no longer necessary now, meaning that:
 *
 * "CONFIG_EXPERIMENTAL_REWORKED_GC" has just (inadvertently) opened the
 * floodgates for a new slab allocator implementation, and more importantly:
 * - one that doesn't need to be able to safely handle non-slab memory...
 * - ... or slab memory from slabs of greater size
 * - And thinking this a bit further: that also means there's no NO REASON
 *   AT ALL to needing to be able to detect slab memory!
 *
 * XXX: Nope nope nope... Yes, there *should* be a new slab allocator, and
 *      slab-free functions don't have to work with slabs of larger sizes,
 *      but they **DO** have to work with `DeeHeap_GetRegionOf()', since
 *      objects allocated by slabs should probably stay serializable as
 *      they are right now (iow: they need to be able to live in a DEC
 *      file mapping, meaning they have to be able to forward pointers to
 *      `Dee_Free()' (at least) when those pointers are FLAG4 heap blocks)
 *      Alternatively, dec writers need special handling for objects using
 *      slab allocators, and lay out the produced file such that offsets
 *      will line up with expected slab offset, and on-top of that, file
 *      mappings loaded via heap would also need to fulfill some minimum
 *      alignment requirements... (Ugh: none of that sounds good, though)
 * But I really want the new slab allocator to **NOT** require a function
 * like `IS_SLAB_POINTER()', which is what's preventing the current slab
 * allocator from growing beyond a specific memory region pre-defined when
 * deemon starts.
 *
 * Hmm. The more I think about it, the only real possibility here is to
 * have a slab allocation system that is also serializable, because:
 * - the only other way to have free function that would also work on
 *   a dec file mapping would be to use out-of-band data to determine
 *   if a pointer belongs to a slab, or a dec file, which would imply
 *   a new `IS_SLAB_POINTER()' function (which I don't want).
 * - However: by serializing slabs into dec files, those slabs would
 *   need to be written as whole pages, which would also imply:
 *   1. Lots of wasted space (since at least 4096 bytes for every distinct
 *      slab size used by an object written to a dec file)
 *   2. The fallback malloc()+read() impl of DeeMapFile would need to
 *      enforce proper alignment of the loaded dec file instead of being
 *      able to just use the heap's natural alignment, and the internal
 *      buffer of `DeeDecWriter' would have to do the same.
 *   - The second point isn't that big of a deal
 *   - The first point *maybe* could be resolved if the this new slab
 *     format could support mixed-size slabs (should be doable so-long
 *     the slab doesn't get a free/usable_size function, by simply always
 *     allocating (e.g.) pointer-sized slab pages, and only marking the
 *     first word of any larger allocation as in-use, even though the
 *     other words are actually also in-use)
 */




/* =========== General-purpose, serializable slab implementation ===========
 *
 * The following describes how this slab allocator is implemented, and
 * how it is able to interface with serializability, and dec files.
 *
 * >> #define SLAB_PAGESIZE 4096 // Any number-of-2 is fine, but value must be known at compile-time
 * >>
 * >> template<size_t CHUNK_SIZE> alignas(SLAB_PAGESIZE) struct slab_page {
 * >>     // Bitset of allocated chunk base addresses
 * >>     typedef uintptr_t bitword_t; // More appropriate would be something like "uint_fastptr_t"
 * >>     bitword_t sp_used[CEILDIV(SLAB_PAGESIZE - sizeof(sp_used), CHUNK_SIZE * BITSOF(bitword_t))];
 * >>
 * >>     // Slab payload data
 * >>     byte_t sp_data[SLAB_PAGESIZE - sizeof(sp_used) - sizeof(sp_meta)];
 * >>
 * >>     // Configuration bits...
 * >>     enum { MAX_CHUNK_COUNT = sizeof(sp_data) / CHUNK_SIZE };
 * >>     STATIC_ASSERT(BITSOF(sp_used) >= MAX_CHUNK_COUNT);
 * >>     enum { UNUSED_TRAILING_BITS = BITSOF(sp_used) - MAX_CHUNK_COUNT };
 * >>     enum { USED_TRAILING_BITS = BITSOF(bitword_t) - UNUSED_TRAILING_BITS };
 * >>     enum { LAST_USED_MASK = ((bitword_t)1 << USED_TRAILING_BITS) - 1 };
 * >>
 * >>     // Metadata for the slab page
 * >>     struct {
 * >>         size_t                sm_used;   // [<= MAX_CHUNK_COUNT][lock(ATOMIC)] # of 1-bits in "sp_used"
 * >>                                          // Must be holding "slab_lock<CHUNK_SIZE>" to change to/from 0/MAX_CHUNK_COUNT
 * >>         LIST_ENTRY(slab_page) sm_link;   // [0..1][lock(slab_lock<CHUNK_SIZE>)] Next page with free chunks,
 * >>                                          // or ITER_DONE if some custom free mechanism must be used because
 * >>                                          // this page exists within a dec file mapping.
 * >>     } sp_meta;
 * >> };
 * >>
 * >> LIST_HEAD(slab_page_list, slab_page);
 * >>
 * >> // Pages containing at least 1 free, and at least 1 allocated chunk.
 * >> // - fully allocated pages aren't tracked anywhere
 * >> // - fully free pages are stored in a global free-list (so they can be used by all slab allocators)
 * >> template<size_t CHUNK_SIZE> static struct slab_page_list slab_pages = LIST_HEAD_INITIALIZER(slab_pages);
 * >> template<size_t CHUNK_SIZE> struct Dee_atomic_rwlock_t slab_lock = Dee_ATOMIC_RWLOCK_INIT;
 * >>
 * >> template<size_t CHUNK_SIZE> void *slab_malloc_in_page(slab_page<CHUNK_SIZE> *__restrict page) {
 * >>     // Caller guaranties that something is free (since they reserved a spot for us)
 * >>     // -- we just need to find that spot!
 * >>     for (;;) {
 * >>         size_t i;
 * >>         constexpr for (i = 0; i < lengthof(page->sp_used); ++i) {
 * >>             constexpr slab_page<CHUNK_SIZE>::bitword_t full_mask =
 * >>                 i >= (lengthof(page->sp_used) - 1)
 * >>                 ? LAST_USED_MASK
 * >>                 : ((slab_page<CHUNK_SIZE>::bitword_t)-1);
 * >>             slab_page<CHUNK_SIZE>::bitword_t word;
 * >> again_read_word:
 * >>             word = atomic_read(&page->sp_used[i]);
 * >>             if (word != full_mask) {
 * >>                 // There seems to be something free here!
 * >>                 size_t index, offset;
 * >>                 shift_t free_bit = CTZ(~word);
 * >>                 slab_page<CHUNK_SIZE>::bitword_t alloc_mask = (slab_page<CHUNK_SIZE>::bitword_t)1 << free_bit;
 * >>                 if (!atomic_cmpxch_weak(&page->sp_used[i], word, word | alloc_mask))
 * >>                     goto again_read_word;
 * >>                 // Got it! -- now just calculate the pointer we need to return
 * >>                 index  = i * BITSOF(slab_page<CHUNK_SIZE>::bitword_t) + free_bit;
 * >>                 offset = index * CHUNK_SIZE;
 * >>                 return page->sp_data + offset;
 * >>             }
 * >>         }
 * >>     }
 * >> }
 * >>
 * >> template<size_t CHUNK_SIZE> void *slab_malloc(void) {
 * >>     void *result;
 * >>     slab_page<CHUNK_SIZE> *page;
 * >> again:
 * >>     Dee_atomic_rwlock_read(&slab_lock<CHUNK_SIZE>);
 * >> again_locked:
 * >>     page = LIST_FIRST(&slab_pages<CHUNK_SIZE>);
 * >>     if (page) {
 * >>         size_t old__sm_used;
 * >>         for (;;) {
 * >>             old__sm_used = atomic_read(&page->sp_meta.sm_used);
 * >>             ASSERT(old__sm_used >= 1);
 * >>             ASSERT(old__sm_used <= (slab_page<CHUNK_SIZE>::MAX_CHUNK_COUNT - 1));
 * >>             if unlikely(old__sm_used >= (slab_page<CHUNK_SIZE>::MAX_CHUNK_COUNT - 1)) {
 * >>                 // About to allocate last free chunk of page
 * >>                 Dee_atomic_rwlock_endread(&slab_lock<CHUNK_SIZE>);
 * >>                 while (!Dee_atomic_rwlock_trywrite(&slab_lock<CHUNK_SIZE>)) {
 * >>                     SCHED_YIELD();
 * >>                     if (page != atomic_read(&slab_pages<CHUNK_SIZE>.lh_first))
 * >>                         goto again;
 * >>                 }
 * >>                 if (LIST_FIRST(&slab_pages<CHUNK_SIZE>) != page ||
 * >>                     !atomic_cmpxch(&page->sp_meta.sm_used, slab_page<CHUNK_SIZE>::MAX_CHUNK_COUNT - 1, slab_page<CHUNK_SIZE>::MAX_CHUNK_COUNT)) {
 * >>                     Dee_atomic_rwlock_downgrade(&slab_lock<CHUNK_SIZE>);
 * >>                     goto again_locked;
 * >>                 }
 * >>                 // Remove page from "slab_pages"
 * >>                 LIST_REMOVE(page, sp_meta.sm_link);
 * >>                 Dee_atomic_rwlock_endwrite(&slab_lock<CHUNK_SIZE>);
 * >>                 break;
 * >>             } else if (atomic_cmpxch(&page->sp_meta.sm_used, old__sm_used, old__sm_used + 1)) {
 * >>                 Dee_atomic_rwlock_endread(&slab_lock<CHUNK_SIZE>);
 * >>                 break;
 * >>             }
 * >>         }
 * >>         // Since we were able to increment "sm_used", that also means that the page
 * >>         // is **GUARANTIED** to have at least 1 0-bit in its `sp_used' bitset!
 * >>         return slab_malloc_in_page(page);
 * >>     }
 * >>     Dee_atomic_rwlock_endread(&slab_lock<CHUNK_SIZE>);
 * >>
 * >>     // Get a new page (global)
 * >>     page = SLAB_ALLOC_PAGE();
 * >>     if (!page)
 * >>         return NULL;
 * >>     bzero(page->sp_used, sizeof(page->sp_used));
 * >>     page->sp_used[0] = 1;
 * >>     page->sp_meta.sm_used = 1;
 * >>     result = &page->sp_data[0];
 * >>
 * >>     // In theory, this lock acquire could be made non-
 * >>     // blocking by having a insert-reap-list for `slab_pages'
 * >>     Dee_atomic_rwlock_write(&slab_lock<CHUNK_SIZE>);
 * >>     LIST_INSERT_HEAD(&slab_pages<CHUNK_SIZE>, page, sp_meta.sm_link);
 * >>     Dee_atomic_rwlock_endwrite(&slab_lock<CHUNK_SIZE>);
 * >>     return result;
 * >> }
 * >>
 * >> template<size_t CHUNK_SIZE> void slab_free(void *p) {
 * >>     slab_page<CHUNK_SIZE> *page = (slab_page<CHUNK_SIZE> *)((uintptr_t)p & ~(SLAB_PAGESIZE - 1));
 * >>     size_t offset = (byte_t *)p - (byte_t *)&page->sp_data;
 * >>     size_t index = offset / CHUNK_SIZE;
 * >>     size_t bit_indx = index / BITSOF(slab_page<CHUNK_SIZE>::bitword_t);
 * >>     size_t old__sm_used;
 * >>     slab_page<CHUNK_SIZE>::bitword_t bit_mask = (slab_page<CHUNK_SIZE>::bitword_t)1 << (index % BITSOF(slab_page<CHUNK_SIZE>::bitword_t));
 * >>     ASSERTF((atomic_read(&page->sp_used[bit_indx]) & bit_mask) != 0, "Pointer not allocated");
 * >>     atomic_and(&page->sp_used[bit_indx], ~bit_mask);
 * >>     do {
 * >> again_read__sm_used:
 * >>         old__sm_used = atomic_read(&page->sp_meta.sm_used);
 * >>         ASSERT(old__sm_used >= 1);
 * >>         if (old__sm_used == 1) {
 * >>             // Last chunk of page is being deleted
 * >>             // In theory, this lock acquire could be made non-blocking by having a pending-free-list
 * >>             // However, this part will be left out initially, unless it turns out that this ends up
 * >>             // being a bottleneck once the new impl is being run in a heavily parallelized environment
 * >>             Dee_atomic_rwlock_write(&slab_lock<CHUNK_SIZE>);
 * >>             if (page->sp_meta.sm_link.le_prev == ITER_DONE) {
 * >>                 Dee_atomic_rwlock_endwrite(&slab_lock<CHUNK_SIZE>);
 * >>                 // Special case: free a custom slab page within a dec file mapping
 * >>                 ...
 * >>                 return;
 * >>             }
 * >>             if (!atomic_cmpxch(&page->sp_meta.sm_used, 1, 0)) {
 * >>                 Dee_atomic_rwlock_endwrite(&slab_lock<CHUNK_SIZE>);
 * >>                 goto again_read__sm_used;
 * >>             }
 * >>             ASSERT(LIST_ISBOUND(page, sp_meta.sm_link));
 * >>             LIST_REMOVE(page, sp_meta.sm_link);
 * >>             Dee_atomic_rwlock_endwrite(&slab_lock<CHUNK_SIZE>);
 * >>             SLAB_FREE_PAGE(page); // Probably add to some global cache of free slab pages...
 * >>             break;
 * >>         } else if (old__sm_used == slab_page<CHUNK_SIZE>::MAX_CHUNK_COUNT) {
 * >>             // First free in previously fully allocated slab (may not actually
 * >>             // be the case when this is a dec chunk, but in that case, we simply
 * >>             // acquire "slab_lock<CHUNK_SIZE>" for no reason, and don't end up
 * >>             // doing anything with it below)
 * >>             Dee_atomic_rwlock_write(&slab_lock<CHUNK_SIZE>);
 * >>             if (!atomic_cmpxch(&page->sp_meta.sm_used, slab_page<CHUNK_SIZE>::MAX_CHUNK_COUNT, slab_page<CHUNK_SIZE>::MAX_CHUNK_COUNT - 1)) {
 * >>                 Dee_atomic_rwlock_endwrite(&slab_lock<CHUNK_SIZE>);
 * >>                 goto again_read__sm_used;
 * >>             }
 * >>             if (page->sp_meta.sm_link.le_prev != ITER_DONE)
 * >>                 LIST_INSERT_HEAD(&slab_pages<CHUNK_SIZE>, page, sp_meta.sm_link);
 * >>             Dee_atomic_rwlock_endwrite(&slab_lock<CHUNK_SIZE>);
 * >>             break;
 * >>         }
 * >>     } while (!atomic_cmpxch_weak(&page->sp_meta.sm_used, old__sm_used, old__sm_used - 1));
 * >> }
 *
 * Though it may not look like it, but so-long as the effective bit-positions
 * indicating that a chunk is allocated in "sp_used" don't overlap (and don't
 * conflict with 'sp_data' of some other already-allocated object), it is
 * possible to put objects of different sizes into the same "slab_page".
 * -> The dec writer can then implement an allocation function for slab memory
 *    that would allocate memory top-down within slab pages, whilst skipping
 *    chunks where previous (possibly differently-sized) allocations already
 *    caused the relevant bit in `sp_used' to be set to `1'
 * -> Since "sp_meta" is stored at the end of a page, its position is always
 *    the same, meaning that differently-sized allocations can still share the
 *    same set meta-data.
 * -> By allowing special values to be used in "sp_meta.sm_link", it becomes
 *    possible to serialize the entire page and execute custom code once all
 *    of the page's chunks have been free'd.
 */



/* The size (and alignment) of slab allocator pages. */
#ifndef Dee_SLAB_PAGESIZE
#ifdef CONFIG_SLAB_PAGESIZE
#define Dee_SLAB_PAGESIZE CONFIG_SLAB_PAGESIZE
#elif defined(__ARCH_PAGESIZE)
#define Dee_SLAB_PAGESIZE __ARCH_PAGESIZE
#elif defined(__ARCH_PAGESIZE_MIN)
#define Dee_SLAB_PAGESIZE __ARCH_PAGESIZE_MIN
#else /* __ARCH_PAGESIZE_MIN */
#define Dee_SLAB_PAGESIZE 4096
#endif /* !__ARCH_PAGESIZE_MIN */
#endif /* !Dee_SLAB_PAGESIZE */

#define Dee_SIZEOF_SLAB_PAGE_META (3 * __SIZEOF_POINTER__) /* `sizeof(struct Dee_slab_page_meta)' */
#define Dee_OFFSET_SLAB_PAGE_META (Dee_SLAB_PAGESIZE - Dee_SIZEOF_SLAB_PAGE_META)

#ifdef __CC__
DECL_BEGIN

/* Type-marker for slab pages with custom "c_free" functions */
#define Dee_SLAB_PAGE_META_CUSTOM_MARKER ((void *)-1)

#if __SIZEOF_INT_FAST16_T__ <= __SIZEOF_POINTER__
#define Dee_slab_page_builder_offset_t __UINT_FAST16_TYPE__
#elif __SIZEOF_INT__ <= __SIZEOF_POINTER__
#define Dee_slab_page_builder_offset_t unsigned int
#else /* ... */
#define Dee_slab_page_builder_offset_t __UINTPTR_TYPE__
#endif /* !... */

struct Dee_slab_page_builder {
	Dee_slab_page_builder_offset_t spb_unused_lo; /* byte-offset into page to start of unused memory */
	Dee_slab_page_builder_offset_t spb_unused_hi; /* byte-offset into page to end of unused memory */
};

#define Dee_SLAB_PAGE_META_FIELDS(page_type)                                                         \
	size_t  spm_used; /* [<= LOCAL__MAX_CHUNK_COUNT][lock(ATOMIC)] # of 1-bits in "sp_used" Must be  \
	                   * holding "LOCAL_slab_lock" to change to/from 0/LOCAL__MAX_CHUNK_COUNT. */    \
	union {                                                                                          \
		LIST_ENTRY(page_type)        t_link;   /* [0..1][lock(INTERNAL(LOCAL_slab_lock))]            \
		                                        * [valid_if(Dee_slab_page_isnormal(:self))]          \
		                                        * Link in list of pages with free chunks. */         \
		struct {                                                                                     \
			/* [1..1][const] Custom callback to free this page once `spm_used' hits `0' */           \
			NONNULL_T((1)) void (DCALL *c_free)(struct page_type *__restrict self);                  \
			void                       *c_marker; /* [== Dee_SLAB_PAGE_META_CUSTOM_MARKER][const] */ \
		}                            t_custom;  /* [valid_if(Dee_slab_page_iscustom(:self))] */      \
		struct Dee_slab_page_builder t_builder; /* For when you're building a custom page */         \
	}       spm_type; /* Link/type of page */

struct Dee_slab_page {
	/* Slab page in-use bitset, followed by slab payload data.
	 * The size and encoding of the in-use bitset depends on the
	 * size of chunks in the page's "sp_data" area. */
#if 0
	__BYTE_TYPE__ sp_used[?]; /* Bitset of allocated chunk base addresses */
	__BYTE_TYPE__ sp_data[?]; /* Slab payload data */
#else
	__BYTE_TYPE__ sp_used_and_data[Dee_OFFSET_SLAB_PAGE_META];
#endif
	struct {
		Dee_SLAB_PAGE_META_FIELDS(Dee_slab_page)
	} sp_meta; /* Slab footer/metadata */
};

/* Helper macros to check what kind of page a give slab is. */
#define Dee_slab_page_iscustom(self) ((self)->sp_meta.spm_type.t_custom.c_marker == Dee_SLAB_PAGE_META_CUSTOM_MARKER)
#define Dee_slab_page_isnormal(self) (!Dee_slab_page_iscustom(self))


/* Helper functions for building custom slab pages. These functions
 * are used by dec file writers to generate serializable dec files:
 * >> struct Dee_slab_page *my_page = ...;
 * >> Dee_slab_page_buildinit(my_page);
 * >> void *p1 = Dee_slab_page_buildmalloc(my_page, 32);
 * >> void *p2 = Dee_slab_page_buildmalloc(my_page, 32);
 * >> void *p3 = Dee_slab_page_buildmalloc(my_page, 24);
 * >> ...
 * >>
 * >> // Pack the custom slab page into a proper one.
 * >> //
 * >> // This part may only be done when "my_page" is properly aligned,
 * >> // which is something that `Dee_slab_page_buildmalloc()' and its
 * >> // parter `Dee_slab_page_buildfree()' has NOT asserted until this
 * >> // point.
 * >> Dee_slab_page_buildpack(my_page, &free_my_page);
 * >>
 * >> DeeSlab_Free32(p1);
 * >> DeeSlab_Free32(p2);
 * >> DeeSlab_Free24(p3); // Last free will invoke "free_my_page(my_page)"
 */
#define Dee_slab_page_buildinit(self) \
	(void)(bzero(self, Dee_SLAB_PAGESIZE), (self)->sp_meta.spm_type.t_builder.spb_unused_hi = Dee_OFFSET_SLAB_PAGE_META)
#define Dee_slab_page_buildpack(self, free_fun)                                           \
	(void)(Dee_ASSERT(((uintptr_t)(self) & (Dee_SLAB_PAGESIZE - 1)) == 0),                \
	       (self)->sp_meta.spm_type.t_custom.c_marker = Dee_SLAB_PAGE_META_CUSTOM_MARKER, \
	       (self)->sp_meta.spm_type.t_custom.c_free   = (free_fun))

#ifdef Dee_SLAB_CHUNKSIZE_MAX
/* Allocate/free custom chunks within a custom slab-page.
 * Also note that using these builder functions, you can
 * even mix-and-match chunks of different sizes, and it
 * will just work (though you have to remember which chunk
 * has which size, since the chunk's size is always needed
 * in order to safely free that chunk)
 *
 * NOTE: These function intentionally do *NOT* assert alignment of "self",
 *       meaning you can use these functions to build a custom slab page
 *       in unaligned memory, before simply memcpy-ing the page into a new
 *       memory location (that is probably) aligned, and keep working with
 *       it without any other relocation-work needed (assuming that payload
 *       area being moved don't require relocations).
 *
 * WARNING: The caller of these functions is responsible to ensure that
 *          `DeeSlab_EXISTS(n)' (or `DeeGCSlab_EXISTS(n - Dee_GC_OBJECT_OFFSET)')
 *          This requirement is asserted internally, so you'll get an assert
 *          failure if you don't comply with this requirement!
 *
 * @return: * :   Pointer into `self->sp_data' to an n-byte payload area
 * @return: NULL: Insufficient memory -- given slab page "self" does not
 *                have space for another "n"-byte large slab. (you should
 *                probably allocate another ) */
DFUNDEF WUNUSED NONNULL((1)) void *DCALL
Dee_slab_page_buildmalloc(struct Dee_slab_page *__restrict self, size_t n);
DFUNDEF NONNULL((1, 2)) void DCALL
Dee_slab_page_buildfree(struct Dee_slab_page *self, void *p, size_t n);
#endif /* Dee_SLAB_CHUNKSIZE_MAX */

DECL_END
#endif /* __CC__ */

#endif /* CONFIG_EXPERIMENTAL_REWORKED_SLAB_ALLOCATOR */

#endif /* !GUARD_DEEMON_UTIL_SLAB_H */
