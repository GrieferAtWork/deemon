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
/*!export DeeDbgObject_**/
/*!export DeeDbgSlab_**/
/*!export DeeDbg_**/
/*!export DeeMem_**/
/*!export DeeSlab**/
/*!export DeeSlab_**/
/*!export Dee_Alloca**/
/*!export Dee_*alloc**/
/*!export Dee_Try*alloc**/
/*!export Dee_Free**/
/*!export DeeObject_*ALLOC**/
/*!export DeeObject_TRY*ALLOC**/
/*!export DeeObject_*alloc**/
/*!export DeeObject_Try*alloc**/
/*!export DeeObject_F*alloc**/
/*!export DeeObject_FTry*alloc**/
/*!export DeeObject_FFree*/
/*!export DeeObject_Free*/
/*!export DeeObject_FREE*/
/*!export DeeObject_Slab**/
/*!export DeeObject_**/
/*!export Dee_CollectMemory**/
/*!export Dee_MALLOCA_**/
/*!export Dee_SLAB_**/
/*!export Dee_TYPE_CONSTRUCTOR_INIT_**/
/*!export _Dee_Malloc*Bufsize**/
/*!export -CONFIG_HAVE_**/
/*!export -_Dee_PRIVATE_**/
/*!export -alloca*/
#ifndef GUARD_DEEMON_ALLOC_H
#define GUARD_DEEMON_ALLOC_H 1 /*!export-*/

#include "api.h"

#include "types.h"            /* Dee_AsObject, Dee_funptr_t */
#include "util/slab-config.h" /* Dee_SLAB_CHUNKSIZE_FOREACH, Dee_SLAB_CHUNKSIZE_MAX, _Dee_PRIVATE_SLAB_SELECT */

#ifdef __CC__
#include <hybrid/__debug-alignment.h> /* __NO_hybrid_dbg_alignment, __hybrid_dbg_alignment_disable, __hybrid_dbg_alignment_enable */
#include <hybrid/__overflow.h>        /* __hybrid_overflow_uadd, __hybrid_overflow_umul */
#include <hybrid/host.h>              /* __linux__, __unix__ */
#include <hybrid/typecore.h>          /* __BYTE_TYPE__, __SIZEOF_POINTER__ */

/*!fixincludes fake_include "system-features.h" // _alloca, malloc */
/*!fixincludes fake_include "type.h"            // Dee_TYPE_CONSTRUCTOR_INIT_ALLOC */

#include <stdbool.h> /* bool */
#include <stddef.h>  /* NULL, size_t */
#include <stdint.h>  /* uint8_t, uintptr_t */

#ifdef CONFIG_NO_STRING_H
#undef CONFIG_HAVE_STRING_H
#elif !defined(CONFIG_HAVE_STRING_H) && \
      (defined(__NO_has_include) || __has_include(<string.h>))
#define CONFIG_HAVE_STRING_H
#endif

#ifdef CONFIG_NO_STRINGS_H
#undef CONFIG_HAVE_STRINGS_H
#elif !defined(CONFIG_HAVE_STRINGS_H) && \
      (__has_include(<strings.h>) || (defined(__NO_has_include) && (defined(__linux__) || \
       defined(__linux) || defined(linux) || defined(__unix__) || defined(__unix) || \
       defined(unix))))
#define CONFIG_HAVE_STRINGS_H
#endif

/* Figure out how to get alloca() (if it is even available) */
#ifdef CONFIG_NO_ALLOCA_H
#undef CONFIG_HAVE_ALLOCA_H
#elif !defined(CONFIG_HAVE_ALLOCA_H) && \
      (__has_include(<alloca.h>) || (defined(__NO_has_include) && ((defined(__CYGWIN__) || \
       defined(__CYGWIN32__)) || (defined(__linux__) || defined(__linux) || defined(linux)) || \
       defined(__KOS__))))
#define CONFIG_HAVE_ALLOCA_H
#endif

#ifdef CONFIG_NO_MALLOC_H
#undef CONFIG_HAVE_MALLOC_H
#elif !defined(CONFIG_HAVE_MALLOC_H) && \
      (__has_include(<malloc.h>) || (defined(__NO_has_include) && (defined(_MSC_VER) || \
       (defined(__CYGWIN__) || defined(__CYGWIN32__)) || (defined(__linux__) || \
       defined(__linux) || defined(linux)) || defined(__KOS__))))
#define CONFIG_HAVE_MALLOC_H
#endif

#ifdef CONFIG_HAVE_STRING_H
#include <string.h>
#endif /* CONFIG_HAVE_STRING_H */

#ifdef CONFIG_HAVE_STRINGS_H
#include <strings.h>
#endif /* CONFIG_HAVE_STRINGS_H */

#ifdef CONFIG_HAVE_ALLOCA_H
#include <alloca.h>
#endif /* CONFIG_HAVE_ALLOCA_H */

#ifdef CONFIG_HAVE_MALLOC_H
#include <malloc.h>
#endif /* CONFIG_HAVE_MALLOC_H */

#ifdef CONFIG_NO_memset
#undef CONFIG_HAVE_memset
#else
#define CONFIG_HAVE_memset
#endif

#ifdef CONFIG_NO_bzero
#undef CONFIG_HAVE_bzero
#elif !defined(CONFIG_HAVE_bzero) && \
      (defined(bzero) || defined(__bzero_defined) || defined(CONFIG_HAVE_STRINGS_H))
#define CONFIG_HAVE_bzero
#endif

#ifdef CONFIG_NO_alloca
#undef CONFIG_HAVE_alloca
#elif !defined(CONFIG_HAVE_alloca) && \
      (defined(alloca) || defined(__alloca_defined) || defined(CONFIG_HAVE_ALLOCA_H))
#define CONFIG_HAVE_alloca
#endif

#ifdef CONFIG_NO__alloca
#undef CONFIG_HAVE__alloca
#elif !defined(CONFIG_HAVE__alloca) && \
      (defined(_alloca) || defined(___alloca_defined) || defined(_MSC_VER))
#define CONFIG_HAVE__alloca
#endif

/* Try to substitute alloca() with alternatives */
#ifndef CONFIG_HAVE_alloca
#ifdef CONFIG_HAVE__alloca
#define CONFIG_HAVE_alloca
#define alloca _alloca /*!export-*/
#elif __has_builtin(__builtin_alloca)
#define alloca __builtin_alloca /*!export-*/
#else /* ... */
#include <hybrid/__alloca.h> /* __hybrid_alloca, __hybrid_alloca_IS_alloca */
#ifdef __hybrid_alloca
#define CONFIG_HAVE_alloca
#ifndef __hybrid_alloca_IS_alloca
#define alloca __hybrid_alloca /*!export-*/
#endif /* !__hybrid_alloca_IS_alloca */
#endif /* __hybrid_alloca */
#endif /* !... */
#endif /* !CONFIG_HAVE_alloca */

#endif /* __CC__ */

DECL_BEGIN

#ifdef __CC__
/* Default malloc/free functions used for heap allocation.
 * NOTE: Upon allocation failure, allocation functions call `Dee_CollectMemory()'.
 *       If memory was collected successfully, the allocation is attempted again. */
DFUNDEF ATTR_MALLOC WUNUSED ATTR_ALLOC_SIZE((1)) void *(DCALL Dee_Malloc)(size_t n_bytes);
DFUNDEF ATTR_MALLOC WUNUSED ATTR_ALLOC_SIZE((1)) void *(DCALL Dee_Calloc)(size_t n_bytes);
DFUNDEF WUNUSED void *(DCALL Dee_Realloc)(void *ptr, size_t n_bytes);

/* Same as the non-*Try* versions above, but these functions don't call `Dee_CollectMemory()' */
DFUNDEF ATTR_MALLOC WUNUSED ATTR_ALLOC_SIZE((1)) void *(DCALL Dee_TryMalloc)(size_t n_bytes);
DFUNDEF ATTR_MALLOC WUNUSED ATTR_ALLOC_SIZE((1)) void *(DCALL Dee_TryCalloc)(size_t n_bytes);
DFUNDEF WUNUSED void *(DCALL Dee_TryRealloc)(void *ptr, size_t n_bytes);

/* Free heap memory at the given "ptr". No-op when "ptr" is "NULL" */
DFUNDEF void (DCALL Dee_Free)(void *ptr);

/* Same as functions above, but instrument the allocation with file/line
 * debug information for use when `DeeHeap_DumpMemoryLeaks()' is called. */
DFUNDEF ATTR_MALLOC WUNUSED ATTR_ALLOC_SIZE((1)) void *(DCALL DeeDbg_Malloc)(size_t n_bytes, char const *file, int line);
DFUNDEF ATTR_MALLOC WUNUSED ATTR_ALLOC_SIZE((1)) void *(DCALL DeeDbg_Calloc)(size_t n_bytes, char const *file, int line);
DFUNDEF WUNUSED void *(DCALL DeeDbg_Realloc)(void *ptr, size_t n_bytes, char const *file, int line);
DFUNDEF ATTR_MALLOC WUNUSED ATTR_ALLOC_SIZE((1)) void *(DCALL DeeDbg_TryMalloc)(size_t n_bytes, char const *file, int line);
DFUNDEF ATTR_MALLOC WUNUSED ATTR_ALLOC_SIZE((1)) void *(DCALL DeeDbg_TryCalloc)(size_t n_bytes, char const *file, int line);
DFUNDEF WUNUSED void *(DCALL DeeDbg_TryRealloc)(void *ptr, size_t n_bytes, char const *file, int line);
DFUNDEF void (DCALL DeeDbg_Free)(void *ptr, char const *file, int line);

/* Mark "ptr" (if non-NULL) as not-to-be-considered a memory leak, even
 * if it is never free'd. This is meant for allocations that are made for
 * the purpose of caches of (possibly statically allocated) objects. One
 * example might be alternative UTF representations of strings. Because
 * strings can be defined statically, these alternate UTF representations
 * would never be free'd, but since these representations can only ever
 * be allocated once, they shouldn't be considered a leak, either.
 *
 * @return: * : Always re-returns "ptr" */
DFUNDEF void *(DCALL DeeDbg_UntrackAlloc)(void *ptr, char const *file, int line);

/* Try to change the size of a memory block, without changing its position in memory.
 * Just like `Dee_TryRealloc()', this function can be used to both shrink a block of
 * memory, as well as grow one (though this function has a high likelihood of failing
 * in the later case, since another allocation may have already consumed memory that
 * would have been needed to grow "ptr")
 *
 * @param: ptr:     Heap pointer, as previously returned by another allocation API
 * @param: n_bytes: The intended new memory size for "ptr"
 * @return: ptr:  Success: memory block was resized in-place.
 * @return: NULL: Failure: unable to shrink/grow memory. In this case, "ptr" remains
 *                         sized at whatever it was sized at before. Note that even
 *                         when shrinking a heap pointer, this function can still
 *                         fail for any number of reasons, including the heap system
 *                         simply not wanting to shrink "ptr". */
DFUNDEF WUNUSED void *(DCALL Dee_TryReallocInPlace)(void *ptr, size_t n_bytes);
#define DeeDbg_TryReallocInPlace(ptr, n_bytes, file, line) Dee_TryReallocInPlace(ptr, n_bytes)

/* Same as `Dee_Malloc()', but the returned pointer is guaranted to be aligned by
 * at least some multiple of `min_alignment'. For this purpose, `min_alignment'
 * must be a power-of-2. The returned pointer can be free'd as normal by simply
 * passing it to `Dee_Free()'. When passed to `Dee_Realloc()', the alignment that
 * was requested here will be lost (`Dee_Realloc()' only guaranties default heap
 * alignment)
 *
 * @param: min_alignment: Minimum alignment for returned pointer (power-of-2)
 * @param: n_bytes:       Minimum usable memory size for returned pointer
 * @return: * :   Success: Base address of newly allocated memory (address is a multiple of `min_alignment')
 * @return: NULL: Failure: Insufficient memory. */
DFUNDEF ATTR_MALLOC WUNUSED ATTR_ALLOC_ALIGN(1) ATTR_ALLOC_SIZE((2)) void *(DCALL Dee_Memalign)(size_t min_alignment, size_t n_bytes);
DFUNDEF ATTR_MALLOC WUNUSED ATTR_ALLOC_ALIGN(1) ATTR_ALLOC_SIZE((2)) void *(DCALL Dee_TryMemalign)(size_t min_alignment, size_t n_bytes);
DFUNDEF ATTR_MALLOC WUNUSED ATTR_ALLOC_ALIGN(1) ATTR_ALLOC_SIZE((2)) void *(DCALL DeeDbg_Memalign)(size_t min_alignment, size_t n_bytes, char const *file, int line);
DFUNDEF ATTR_MALLOC WUNUSED ATTR_ALLOC_ALIGN(1) ATTR_ALLOC_SIZE((2)) void *(DCALL DeeDbg_TryMemalign)(size_t min_alignment, size_t n_bytes, char const *file, int line);

/* Return the usable memory size (in bytes) of a heap "ptr" returned by any of the
 * other heap functions, including `Dee_Malloc()', `Dee_Realloc()' and `Dee_Memalign()'
 * When "ptr" points at the start of the payload area of a `struct Dee_heapchunk', the
 * return value is the value that was passed to `Dee_HEAPCHUNK_HEAD()'.
 *
 * @param: ptr: Base address as returned by one of the other heap functions, or "NULL".
 *              Behavior is undefined if "ptr" isn't a heap pointer, or doesn't point
 *              at the start of a heap allocation.
 * @return: 0 : Given "ptr" is either "NULL" or doesn't have any writable bytes
 * @return: * : The amount of memory (in bytes) that the caller is free to use, starting at "ptr" */
DFUNDEF ATTR_PURE WUNUSED size_t (DCALL Dee_MallocUsableSize)(void *ptr);
DFUNDEF ATTR_PURE WUNUSED NONNULL((1)) size_t (DCALL Dee_MallocUsableSizeNonNull)(void *ptr);

#ifndef NDEBUG
#define Dee_Malloc(n_bytes)          DeeDbg_Malloc(n_bytes, __FILE__, __LINE__)
#define Dee_Calloc(n_bytes)          DeeDbg_Calloc(n_bytes, __FILE__, __LINE__)
#define Dee_Realloc(ptr, n_bytes)    DeeDbg_Realloc(ptr, n_bytes, __FILE__, __LINE__)
#define Dee_TryMalloc(n_bytes)       DeeDbg_TryMalloc(n_bytes, __FILE__, __LINE__)
#define Dee_TryCalloc(n_bytes)       DeeDbg_TryCalloc(n_bytes, __FILE__, __LINE__)
#define Dee_TryRealloc(ptr, n_bytes) DeeDbg_TryRealloc(ptr, n_bytes, __FILE__, __LINE__)
#define Dee_Free(ptr)                DeeDbg_Free(ptr, __FILE__, __LINE__)
#define Dee_UntrackAlloc(ptr)        DeeDbg_UntrackAlloc(ptr, __FILE__, __LINE__)
#define Dee_Memalign(min_alignment, n_bytes)    DeeDbg_Memalign(min_alignment, n_bytes, __FILE__, __LINE__)
#define Dee_TryMemalign(min_alignment, n_bytes) DeeDbg_TryMemalign(min_alignment, n_bytes, __FILE__, __LINE__)
#else /* !NDEBUG */
#define DeeDbg_Malloc(n_bytes, file, line)          Dee_Malloc(n_bytes)
#define DeeDbg_Calloc(n_bytes, file, line)          Dee_Calloc(n_bytes)
#define DeeDbg_Realloc(ptr, n_bytes, file, line)    Dee_Realloc(ptr, n_bytes)
#define DeeDbg_TryMalloc(n_bytes, file, line)       Dee_TryMalloc(n_bytes)
#define DeeDbg_TryCalloc(n_bytes, file, line)       Dee_TryCalloc(n_bytes)
#define DeeDbg_TryRealloc(ptr, n_bytes, file, line) Dee_TryRealloc(ptr, n_bytes)
#define DeeDbg_Free(ptr, file, line)                Dee_Free(ptr)
#define Dee_UntrackAlloc(ptr)                       (ptr)
#define DeeDbg_Memalign(min_alignment, n_bytes, file, line)    Dee_Memalign(min_alignment, n_bytes)
#define DeeDbg_TryMemalign(min_alignment, n_bytes, file, line) Dee_TryMemalign(min_alignment, n_bytes)
#endif /* NDEBUG */

/* Debug version of malloc buffer size calculation functions.
 * These will trigger an assertion failure when an overflow *does* happen.
 * As such, these functions should be used in debug builds to assert that
 * no unexpected overflows happen. */
DFUNDEF ATTR_CONST WUNUSED size_t (DCALL _Dee_MalloccBufsizeDbg)(size_t elem_count, size_t elem_size, char const *file, int line);
DFUNDEF ATTR_CONST WUNUSED size_t (DCALL _Dee_MallococBufsizeDbg)(size_t base_offset, size_t elem_count, size_t elem_size, char const *file, int line);

/* Mallocc buffer size calculation (w/ and w/o overflow handling) */
#if defined(NDEBUG) || defined(__INTELLISENSE__)
#define _Dee_MalloccBufsize(elem_count, elem_size)                              ((elem_count) * (elem_size))
#define _Dee_MallococBufsize(base_offset, elem_count, elem_size)                ((base_offset) + ((elem_count) * (elem_size)))
#define _Dee_MalloccBufsizeDbg(elem_count, elem_size, file, line)               _Dee_MalloccBufsize(elem_count, elem_size)
#define _Dee_MallococBufsizeDbg(base_offset, elem_count, elem_size, file, line) _Dee_MallococBufsize(base_offset, elem_count, elem_size)
#else /* NDEBUG */
#define _Dee_MalloccBufsize(elem_count, elem_size) \
	_Dee_MalloccBufsizeDbg(elem_count, elem_size, __FILE__, __LINE__)
#define _Dee_MallococBufsize(base_offset, elem_count, elem_size) \
	_Dee_MallococBufsizeDbg(base_offset, elem_count, elem_size, __FILE__, __LINE__)
#endif /* !NDEBUG */

FORCELOCAL ATTR_CONST WUNUSED size_t DCALL
_Dee_MalloccBufsizeSafe(size_t elem_count, size_t elem_size) {
	size_t result;
#ifndef __NO_builtin_constant_p
	if (__builtin_constant_p(elem_size) && (elem_size == 1))
		return elem_count;
	if (__builtin_constant_p(elem_count) && (elem_count == 1))
		return elem_size;
#endif /* !__NO_builtin_constant_p */
	if (__hybrid_overflow_umul(elem_count, elem_size, &result))
		result = (size_t)-1;
	return result;
}

FORCELOCAL ATTR_CONST WUNUSED size_t DCALL
_Dee_MallococBufsizeSafe(size_t base_offset, size_t elem_count, size_t elem_size) {
	size_t result;
#ifndef __NO_builtin_constant_p
	if (__builtin_constant_p(base_offset) && (base_offset == 0))
		return _Dee_MalloccBufsizeSafe(elem_count, elem_size);
	if (__builtin_constant_p(elem_size) && (elem_size == 1)) {
		if (__hybrid_overflow_uadd(elem_count, base_offset, &result))
			result = (size_t)-1;
		return result;
	}
	if (__builtin_constant_p(elem_count) && (elem_count == 1)) {
		if (__hybrid_overflow_uadd(elem_size, base_offset, &result))
			result = (size_t)-1;
		return result;
	}
#endif /* !__NO_builtin_constant_p */
	if (__hybrid_overflow_umul(elem_count, elem_size, &result))
		result = (size_t)-1;
	if (__hybrid_overflow_uadd(result, base_offset, &result))
		result = (size_t)-1;
	return result;
}

#define Dee_Mallocc(elem_count, elem_size)                         Dee_Malloc(_Dee_MalloccBufsize(elem_count, elem_size))
#define Dee_Callocc(elem_count, elem_size)                         Dee_Calloc(_Dee_MalloccBufsize(elem_count, elem_size))
#define Dee_Reallocc(ptr, elem_count, elem_size)                   Dee_Realloc(ptr, _Dee_MalloccBufsize(elem_count, elem_size))
#define Dee_TryMallocc(elem_count, elem_size)                      Dee_TryMalloc(_Dee_MalloccBufsize(elem_count, elem_size))
#define Dee_TryCallocc(elem_count, elem_size)                      Dee_TryCalloc(_Dee_MalloccBufsize(elem_count, elem_size))
#define Dee_TryReallocc(ptr, elem_count, elem_size)                Dee_TryRealloc(ptr, _Dee_MalloccBufsize(elem_count, elem_size))
#define DeeDbg_Mallocc(elem_count, elem_size, file, line)          DeeDbg_Malloc(_Dee_MalloccBufsize(elem_count, elem_size), file, line)
#define DeeDbg_Callocc(elem_count, elem_size, file, line)          DeeDbg_Calloc(_Dee_MalloccBufsize(elem_count, elem_size), file, line)
#define DeeDbg_Reallocc(ptr, elem_count, elem_size, file, line)    DeeDbg_Realloc(ptr, _Dee_MalloccBufsize(elem_count, elem_size), file, line)
#define DeeDbg_TryMallocc(elem_count, elem_size, file, line)       DeeDbg_TryMalloc(_Dee_MalloccBufsize(elem_count, elem_size), file, line)
#define DeeDbg_TryCallocc(elem_count, elem_size, file, line)       DeeDbg_TryCalloc(_Dee_MalloccBufsize(elem_count, elem_size), file, line)
#define DeeDbg_TryReallocc(ptr, elem_count, elem_size, file, line) DeeDbg_TryRealloc(ptr, _Dee_MalloccBufsize(elem_count, elem_size), file, line)

#define Dee_MalloccSafe(elem_count, elem_size)                         Dee_Malloc(_Dee_MalloccBufsizeSafe(elem_count, elem_size))
#define Dee_CalloccSafe(elem_count, elem_size)                         Dee_Calloc(_Dee_MalloccBufsizeSafe(elem_count, elem_size))
#define Dee_RealloccSafe(ptr, elem_count, elem_size)                   Dee_Realloc(ptr, _Dee_MalloccBufsizeSafe(elem_count, elem_size))
#define Dee_TryMalloccSafe(elem_count, elem_size)                      Dee_TryMalloc(_Dee_MalloccBufsizeSafe(elem_count, elem_size))
#define Dee_TryCalloccSafe(elem_count, elem_size)                      Dee_TryCalloc(_Dee_MalloccBufsizeSafe(elem_count, elem_size))
#define Dee_TryRealloccSafe(ptr, elem_count, elem_size)                Dee_TryRealloc(ptr, _Dee_MalloccBufsizeSafe(elem_count, elem_size))
#define DeeDbg_MalloccSafe(elem_count, elem_size, file, line)          DeeDbg_Malloc(_Dee_MalloccBufsizeSafe(elem_count, elem_size), file, line)
#define DeeDbg_CalloccSafe(elem_count, elem_size, file, line)          DeeDbg_Calloc(_Dee_MalloccBufsizeSafe(elem_count, elem_size), file, line)
#define DeeDbg_RealloccSafe(ptr, elem_count, elem_size, file, line)    DeeDbg_Realloc(ptr, _Dee_MalloccBufsizeSafe(elem_count, elem_size), file, line)
#define DeeDbg_TryMalloccSafe(elem_count, elem_size, file, line)       DeeDbg_TryMalloc(_Dee_MalloccBufsizeSafe(elem_count, elem_size), file, line)
#define DeeDbg_TryCalloccSafe(elem_count, elem_size, file, line)       DeeDbg_TryCalloc(_Dee_MalloccBufsizeSafe(elem_count, elem_size), file, line)
#define DeeDbg_TryRealloccSafe(ptr, elem_count, elem_size, file, line) DeeDbg_TryRealloc(ptr, _Dee_MalloccBufsizeSafe(elem_count, elem_size), file, line)

#define Dee_Mallococ(base_offset, elem_count, elem_size)                         Dee_Malloc(_Dee_MallococBufsize(base_offset, elem_count, elem_size))
#define Dee_Callococ(base_offset, elem_count, elem_size)                         Dee_Calloc(_Dee_MallococBufsize(base_offset, elem_count, elem_size))
#define Dee_Reallococ(ptr, base_offset, elem_count, elem_size)                   Dee_Realloc(ptr, _Dee_MallococBufsize(base_offset, elem_count, elem_size))
#define Dee_TryMallococ(base_offset, elem_count, elem_size)                      Dee_TryMalloc(_Dee_MallococBufsize(base_offset, elem_count, elem_size))
#define Dee_TryCallococ(base_offset, elem_count, elem_size)                      Dee_TryCalloc(_Dee_MallococBufsize(base_offset, elem_count, elem_size))
#define Dee_TryReallococ(ptr, base_offset, elem_count, elem_size)                Dee_TryRealloc(ptr, _Dee_MallococBufsize(base_offset, elem_count, elem_size))
#define DeeDbg_Mallococ(base_offset, elem_count, elem_size, file, line)          DeeDbg_Malloc(_Dee_MallococBufsizeDbg(base_offset, elem_count, elem_size, file, line), file, line)
#define DeeDbg_Callococ(base_offset, elem_count, elem_size, file, line)          DeeDbg_Calloc(_Dee_MallococBufsizeDbg(base_offset, elem_count, elem_size, file, line), file, line)
#define DeeDbg_Reallococ(ptr, base_offset, elem_count, elem_size, file, line)    DeeDbg_Realloc(ptr, _Dee_MallococBufsizeDbg(base_offset, elem_count, elem_size, file, line), file, line)
#define DeeDbg_TryMallococ(base_offset, elem_count, elem_size, file, line)       DeeDbg_TryMalloc(_Dee_MallococBufsizeDbg(base_offset, elem_count, elem_size, file, line), file, line)
#define DeeDbg_TryCallococ(base_offset, elem_count, elem_size, file, line)       DeeDbg_TryCalloc(_Dee_MallococBufsizeDbg(base_offset, elem_count, elem_size, file, line), file, line)
#define DeeDbg_TryReallococ(ptr, base_offset, elem_count, elem_size, file, line) DeeDbg_TryRealloc(ptr, _Dee_MallococBufsizeDbg(base_offset, elem_count, elem_size, file, line), file, line)

#define Dee_MallococSafe(base_offset, elem_count, elem_size)                         Dee_Malloc(_Dee_MallococBufsizeSafe(base_offset, elem_count, elem_size))
#define Dee_CallococSafe(base_offset, elem_count, elem_size)                         Dee_Calloc(_Dee_MallococBufsizeSafe(base_offset, elem_count, elem_size))
#define Dee_ReallococSafe(ptr, base_offset, elem_count, elem_size)                   Dee_Realloc(ptr, _Dee_MallococBufsizeSafe(base_offset, elem_count, elem_size))
#define Dee_TryMallococSafe(base_offset, elem_count, elem_size)                      Dee_TryMalloc(_Dee_MallococBufsizeSafe(base_offset, elem_count, elem_size))
#define Dee_TryCallococSafe(base_offset, elem_count, elem_size)                      Dee_TryCalloc(_Dee_MallococBufsizeSafe(base_offset, elem_count, elem_size))
#define Dee_TryReallococSafe(ptr, base_offset, elem_count, elem_size)                Dee_TryRealloc(ptr, _Dee_MallococBufsizeSafe(base_offset, elem_count, elem_size))
#define DeeDbg_MallococSafe(base_offset, elem_count, elem_size, file, line)          DeeDbg_Malloc(_Dee_MallococBufsizeSafe(base_offset, elem_count, elem_size), file, line)
#define DeeDbg_CallococSafe(base_offset, elem_count, elem_size, file, line)          DeeDbg_Calloc(_Dee_MallococBufsizeSafe(base_offset, elem_count, elem_size), file, line)
#define DeeDbg_ReallococSafe(ptr, base_offset, elem_count, elem_size, file, line)    DeeDbg_Realloc(ptr, _Dee_MallococBufsizeSafe(base_offset, elem_count, elem_size), file, line)
#define DeeDbg_TryMallococSafe(base_offset, elem_count, elem_size, file, line)       DeeDbg_TryMalloc(_Dee_MallococBufsizeSafe(base_offset, elem_count, elem_size), file, line)
#define DeeDbg_TryCallococSafe(base_offset, elem_count, elem_size, file, line)       DeeDbg_TryCalloc(_Dee_MallococBufsizeSafe(base_offset, elem_count, elem_size), file, line)
#define DeeDbg_TryReallococSafe(ptr, base_offset, elem_count, elem_size, file, line) DeeDbg_TryRealloc(ptr, _Dee_MallococBufsizeSafe(base_offset, elem_count, elem_size), file, line)

/* Reclaim free memory by going through internal pre-allocation caches,
 * freeing up to (but potentially exceeding by a bit) `max_collect' bytes of memory.
 * The actual amount freed is returned in bytes.
 * NOTE: This function is automatically called by `Dee_CollectMemory()' */
DFUNDEF size_t DCALL DeeMem_ClearCaches(size_t max_collect);

/* Try to clear caches and free up at most "req_bytes" memory. If
 * no memory could be free'd at all, Dee_BadAlloc() is called. Note
 * that this function is also allowed to invoke arbitrary user-code!
 *
 * @return: true:  Caches were cleared. - You should try to allocate memory again.
 * @return: false: Nope. - We're completely out of memory... (error was thrown) */
DFUNDEF WUNUSED ATTR_COLD bool DCALL Dee_CollectMemory(size_t req_bytes);

/* Try to release as much memory as possible back to the host system.
 * @return: * : Amount of memory that was released back to the system.
 * @return: 0 : No memory could be released back to the system (no error was thrown) */
DFUNDEF ATTR_COLD size_t DCALL Dee_TryReleaseSystemMemory(void);

/* Same as `Dee_TryReleaseSystemMemory()', but also tries to free
 * @return: * : Amount of memory that was released back to the system.
 * @return: 0 : No memory could be released back to the system (an error was thrown) */
DFUNDEF ATTR_COLD WUNUSED size_t DCALL Dee_ReleaseSystemMemory(void);

#define Dee_CollectMemoryc(elem_count, elem_size) \
	Dee_CollectMemory(_Dee_MalloccBufsize(elem_count, elem_size))
#define Dee_CollectMemoryoc(base_offset, elem_count, elem_size) \
	Dee_CollectMemory(_Dee_MallococBufsize(base_offset, elem_count, elem_size))
#define Dee_CollectMemorycSafe(elem_count, elem_size) \
	Dee_CollectMemory(_Dee_MalloccBufsizeSafe(elem_count, elem_size))
#define Dee_CollectMemoryocSafe(base_offset, elem_count, elem_size) \
	Dee_CollectMemory(_Dee_MallococBufsizeSafe(base_offset, elem_count, elem_size))

/* Throw a bad-allocation error for `req_bytes' bytes.
 * @return: -1: Always returns -1. */
DFUNDEF ATTR_COLD int (DCALL Dee_BadAlloc)(size_t req_bytes);

#ifndef Dee_ASSUMED_VALUE_IS_NOOP
#define Dee_BadAlloc(req_bytes) Dee_ASSUMED_VALUE(Dee_BadAlloc(req_bytes), -1)
#endif /* !Dee_ASSUMED_VALUE_IS_NOOP */
#endif /* __CC__ */

/* Default malloc/free functions used for object allocation.
 * !!! Even though these are aliased to regular malloc, don't mix-and-match !!! */
#define DeeObject_Malloc          Dee_Malloc
#define DeeObject_Calloc          Dee_Calloc
#define DeeObject_Realloc         Dee_Realloc
#define DeeObject_TryMalloc       Dee_TryMalloc
#define DeeObject_TryCalloc       Dee_TryCalloc
#define DeeObject_TryRealloc      Dee_TryRealloc
#define DeeObject_Free            Dee_Free
#define DeeObject_UntrackAlloc    Dee_UntrackAlloc
#define DeeDbgObject_Malloc       DeeDbg_Malloc
#define DeeDbgObject_Calloc       DeeDbg_Calloc
#define DeeDbgObject_Realloc      DeeDbg_Realloc
#define DeeDbgObject_TryMalloc    DeeDbg_TryMalloc
#define DeeDbgObject_TryCalloc    DeeDbg_TryCalloc
#define DeeDbgObject_TryRealloc   DeeDbg_TryRealloc
#define DeeDbgObject_Free         DeeDbg_Free
#define DeeDbgObject_UntrackAlloc DeeDbg_UntrackAlloc

#define DeeObject_Mallocc        Dee_Mallococ
#define DeeObject_Callocc        Dee_Callococ
#define DeeObject_Reallocc       Dee_Reallococ
#define DeeObject_TryMallocc     Dee_TryMallococ
#define DeeObject_TryCallocc     Dee_TryCallococ
#define DeeObject_TryReallocc    Dee_TryReallococ
#define DeeDbgObject_Mallocc     DeeDbg_Mallococ
#define DeeDbgObject_Callocc     DeeDbg_Callococ
#define DeeDbgObject_Reallocc    DeeDbg_Reallococ
#define DeeDbgObject_TryMallocc  DeeDbg_TryMallococ
#define DeeDbgObject_TryCallocc  DeeDbg_TryCallococ
#define DeeDbgObject_TryReallocc DeeDbg_TryReallococ

#define DeeObject_MalloccSafe        Dee_MallococSafe
#define DeeObject_CalloccSafe        Dee_CallococSafe
#define DeeObject_RealloccSafe       Dee_ReallococSafe
#define DeeObject_TryMalloccSafe     Dee_TryMallococSafe
#define DeeObject_TryCalloccSafe     Dee_TryCallococSafe
#define DeeObject_TryRealloccSafe    Dee_TryReallococSafe
#define DeeDbgObject_MalloccSafe     DeeDbg_MallococSafe
#define DeeDbgObject_CalloccSafe     DeeDbg_CallococSafe
#define DeeDbgObject_RealloccSafe    DeeDbg_ReallococSafe
#define DeeDbgObject_TryMalloccSafe  DeeDbg_TryMallococSafe
#define DeeDbgObject_TryCalloccSafe  DeeDbg_TryCallococSafe
#define DeeDbgObject_TryRealloccSafe DeeDbg_TryReallococSafe

#ifdef __CC__
/* Free the reference tracker of a given object.
 * Should be called prior to `DeeObject_Free()' for any object
 * who's reference counter was modified at any point in time. */
#ifdef CONFIG_TRACE_REFCHANGES
struct Dee_object;
DFUNDEF NONNULL((1)) void
(DCALL DeeObject_FreeTracker)(struct Dee_object *__restrict self);
#define DeeObject_FreeTracker(self) (DeeObject_FreeTracker)(Dee_AsObject(self))
#else /* CONFIG_TRACE_REFCHANGES */
#define DeeObject_FreeTracker(self) (void)0
#endif /* !CONFIG_TRACE_REFCHANGES */
#endif /* __CC__ */





/* Free-functions, and their capabilities
 *
 * Dee_Free:                Accepts  Dee_Malloc()
 *                          Accepts  NULL
 *
 * DeeObject_Free:          Accepts  DeeObject_Malloc()
 *                          Accepts  NULL
 *
 * DeeGCObject_Free:        Accepts  DeeGCObject_Malloc()
 *                          Accepts  NULL
 *
 * DeeSlab_Free(..., N):    Accepts  DeeSlab_Malloc(N)
 * DeeSlab_Free<N>:         Accepts  DeeSlab_Malloc<N>()
 *
 * DeeGCSlab_Free(..., N):  Accepts  DeeGCSlab_Malloc(N)
 * DeeGCSlab_Free<N>:       Accepts  DeeGCSlab_Malloc<N>()
 *
 * DeeObject_FREE<T>:       Accepts  DeeObject_MALLOC(S)     | sizeof(T) == sizeof(S)
 *
 * DeeGCObject_FREE<T>:     Accepts  DeeObject_GCMALLOC(S)   | sizeof(T) == sizeof(S)
 *
 * *Dbg* versions are always interchangable with the relevant non-*Dbg* version
 */


#ifdef __CC__
/* Slab memory allocators (s.a. <deemon/slab.h>). */
#define _Dee_PRIVATE_DeeSlab_API(n, _)                                                           \
	DFUNDEF ATTR_MALLOC WUNUSED void *DCALL DeeSlab_Malloc##n(void);                             \
	DFUNDEF ATTR_MALLOC WUNUSED void *DCALL DeeSlab_Calloc##n(void);                             \
	DFUNDEF ATTR_MALLOC WUNUSED void *DCALL DeeSlab_TryMalloc##n(void);                          \
	DFUNDEF ATTR_MALLOC WUNUSED void *DCALL DeeSlab_TryCalloc##n(void);                          \
	DFUNDEF NONNULL((1)) void DCALL DeeSlab_Free##n(void *__restrict p);                         \
	DFUNDEF ATTR_MALLOC WUNUSED void *DCALL DeeDbgSlab_Malloc##n(char const *file, int line);    \
	DFUNDEF ATTR_MALLOC WUNUSED void *DCALL DeeDbgSlab_Calloc##n(char const *file, int line);    \
	DFUNDEF ATTR_MALLOC WUNUSED void *DCALL DeeDbgSlab_TryMalloc##n(char const *file, int line); \
	DFUNDEF ATTR_MALLOC WUNUSED void *DCALL DeeDbgSlab_TryCalloc##n(char const *file, int line); \
	DFUNDEF void *DCALL DeeDbgSlab_UntrackAlloc##n(void *p, char const *file, int line);         \
	DFUNDEF NONNULL((1)) void DCALL DeeDbgSlab_Free##n(void *__restrict p, char const *file, int line);
Dee_SLAB_CHUNKSIZE_FOREACH(_Dee_PRIVATE_DeeSlab_API, ~)
#undef _Dee_PRIVATE_DeeSlab_API
#endif /* __CC__ */

/* Check if an object-size of "N" is supported for slab allocation */
#ifdef Dee_SLAB_CHUNKSIZE_MAX
#define DeeSlab_IsSupported(N) ((N) <= Dee_SLAB_CHUNKSIZE_MAX)
#else /* Dee_SLAB_CHUNKSIZE_MAX */
#define DeeSlab_IsSupported(N) 0
#endif /* !Dee_SLAB_CHUNKSIZE_MAX */

/* Check if a slab size size "N" exists */
#define DeeSlab_EXISTS(N) (0 Dee_SLAB_CHUNKSIZE_FOREACH(_DeeSlab_EXISTS_CB, N))
#ifndef _DeeSlab_EXISTS_CB
#define _DeeSlab_EXISTS_CB(n, N) || n == (N) /*!export-*/
#endif /* !_DeeSlab_EXISTS_CB */


/* Return a function pointer for the relevant slab function for allocations of size "N"
 * If the given "N" isn't supported, returns "else" instead. */
#define DeeSlab_GetMalloc(N, else)          _Dee_PRIVATE_SLAB_SELECT(N, &, DeeSlab_Malloc, , else)
#define DeeSlab_GetCalloc(N, else)          _Dee_PRIVATE_SLAB_SELECT(N, &, DeeSlab_Calloc, , else)
#define DeeSlab_GetTryMalloc(N, else)       _Dee_PRIVATE_SLAB_SELECT(N, &, DeeSlab_TryMalloc, , else)
#define DeeSlab_GetTryCalloc(N, else)       _Dee_PRIVATE_SLAB_SELECT(N, &, DeeSlab_TryCalloc, , else)
#define DeeSlab_GetFree(N, else)            _Dee_PRIVATE_SLAB_SELECT(N, &, DeeSlab_Free, , else)
#define DeeDbgSlab_GetMalloc(N, else)       _Dee_PRIVATE_SLAB_SELECT(N, &, DeeDbgSlab_Malloc, , else)
#define DeeDbgSlab_GetCalloc(N, else)       _Dee_PRIVATE_SLAB_SELECT(N, &, DeeDbgSlab_Calloc, , else)
#define DeeDbgSlab_GetTryMalloc(N, else)    _Dee_PRIVATE_SLAB_SELECT(N, &, DeeDbgSlab_TryMalloc, , else)
#define DeeDbgSlab_GetTryCalloc(N, else)    _Dee_PRIVATE_SLAB_SELECT(N, &, DeeDbgSlab_TryCalloc, , else)
#define DeeDbgSlab_GetUntrackAlloc(N, else) _Dee_PRIVATE_SLAB_SELECT(N, &, DeeDbgSlab_UntrackAlloc, , else)
#define DeeDbgSlab_GetFree(N, else)         _Dee_PRIVATE_SLAB_SELECT(N, &, DeeDbgSlab_Free, , else)

/* Invoke the relevant slab function. If no slab function exists, "else" is evaluated and returned instead. */
#define _DeeSlab_Malloc(N, else)                         _Dee_PRIVATE_SLAB_SELECT(N, , DeeSlab_Malloc, (), else)
#define _DeeSlab_Calloc(N, else)                         _Dee_PRIVATE_SLAB_SELECT(N, , DeeSlab_Calloc, (), else)
#define _DeeSlab_TryMalloc(N, else)                      _Dee_PRIVATE_SLAB_SELECT(N, , DeeSlab_TryMalloc, (), else)
#define _DeeSlab_TryCalloc(N, else)                      _Dee_PRIVATE_SLAB_SELECT(N, , DeeSlab_TryCalloc, (), else)
#define _DeeSlab_Free(N, p, else)                        _Dee_PRIVATE_SLAB_SELECT(N, , DeeSlab_Free, (p), else)
#define _DeeDbgSlab_Malloc(N, file, line, else)          _Dee_PRIVATE_SLAB_SELECT(N, , DeeDbgSlab_Malloc, (file, line), else)
#define _DeeDbgSlab_Calloc(N, file, line, else)          _Dee_PRIVATE_SLAB_SELECT(N, , DeeDbgSlab_Calloc, (file, line), else)
#define _DeeDbgSlab_TryMalloc(N, file, line, else)       _Dee_PRIVATE_SLAB_SELECT(N, , DeeDbgSlab_TryMalloc, (file, line), else)
#define _DeeDbgSlab_TryCalloc(N, file, line, else)       _Dee_PRIVATE_SLAB_SELECT(N, , DeeDbgSlab_TryCalloc, (file, line), else)
#define _DeeDbgSlab_UntrackAlloc(N, p, file, line, else) _Dee_PRIVATE_SLAB_SELECT(N, , DeeDbgSlab_UntrackAlloc, (p, file, line), else)
#define _DeeDbgSlab_Free(N, p, file, line, else)         _Dee_PRIVATE_SLAB_SELECT(N, , DeeDbgSlab_Free, (p, file, line), else)
#ifdef NDEBUG
#define DeeSlab_Malloc(N, else)                         _DeeSlab_Malloc(N, else)
#define DeeSlab_Calloc(N, else)                         _DeeSlab_Calloc(N, else)
#define DeeSlab_TryMalloc(N, else)                      _DeeSlab_TryMalloc(N, else)
#define DeeSlab_TryCalloc(N, else)                      _DeeSlab_TryCalloc(N, else)
#define DeeSlab_Free(N, p, else)                        _DeeSlab_Free(N, p, else)
#define DeeDbgSlab_Malloc(N, file, line, else)          _DeeSlab_Malloc(N, else)
#define DeeDbgSlab_Calloc(N, file, line, else)          _DeeSlab_Calloc(N, else)
#define DeeDbgSlab_TryMalloc(N, file, line, else)       _DeeSlab_TryMalloc(N, else)
#define DeeDbgSlab_TryCalloc(N, file, line, else)       _DeeSlab_TryCalloc(N, else)
#define DeeDbgSlab_UntrackAlloc(N, p, file, line, else) (p)
#define DeeDbgSlab_Free(N, p, file, line, else)         _DeeSlab_Free(N, p, else)
#else /* NDEBUG */
#define DeeSlab_Malloc(N, else)                         _DeeDbgSlab_Malloc(N, __FILE__, __LINE__, else)
#define DeeSlab_Calloc(N, else)                         _DeeDbgSlab_Calloc(N, __FILE__, __LINE__, else)
#define DeeSlab_TryMalloc(N, else)                      _DeeDbgSlab_TryMalloc(N, __FILE__, __LINE__, else)
#define DeeSlab_TryCalloc(N, else)                      _DeeDbgSlab_TryCalloc(N, __FILE__, __LINE__, else)
#define DeeSlab_Free(N, p, else)                        _DeeDbgSlab_Free(N, p, __FILE__, __LINE__, else)
#define DeeDbgSlab_Malloc(N, file, line, else)          _DeeDbgSlab_Malloc(N, file, line, else)
#define DeeDbgSlab_Calloc(N, file, line, else)          _DeeDbgSlab_Calloc(N, file, line, else)
#define DeeDbgSlab_TryMalloc(N, file, line, else)       _DeeDbgSlab_TryMalloc(N, file, line, else)
#define DeeDbgSlab_TryCalloc(N, file, line, else)       _DeeDbgSlab_TryCalloc(N, file, line, else)
#define DeeDbgSlab_UntrackAlloc(N, p, file, line, else) _DeeDbgSlab_UntrackAlloc(N, p, file, line, else)
#define DeeDbgSlab_Free(N, p, file, line, else)         _DeeDbgSlab_Free(N, p, file, line, else)
#endif /* !NDEBUG */
#define DeeSlab_UntrackAlloc(N, p, else) DeeDbgSlab_UntrackAlloc(N, p, __FILE__, __LINE__, else)

/* Allocate fixed-size, object-purposed memory (using slabs if possible, but using regular heap memory otherwise). */
#define DeeObject_FMalloc(size)                           DeeSlab_Malloc(size, DeeObject_Malloc(size))
#define DeeObject_FCalloc(size)                           DeeSlab_Calloc(size, DeeObject_Calloc(size))
#define DeeObject_FTryMalloc(size)                        DeeSlab_TryMalloc(size, DeeObject_TryMalloc(size))
#define DeeObject_FTryCalloc(size)                        DeeSlab_TryCalloc(size, DeeObject_TryCalloc(size))
#define DeeObject_FUntrackAlloc(ptr, size)                DeeSlab_UntrackAlloc(size, ptr, DeeObject_UntrackAlloc(ptr))
#define DeeObject_FFree(ptr, size)                        DeeSlab_Free(size, ptr, DeeObject_Free(ptr))
#define DeeDbgObject_FMalloc(size, file, line)            DeeDbgSlab_Malloc(size, file, line, DeeDbgObject_Malloc(size, file, line))
#define DeeDbgObject_FCalloc(size, file, line)            DeeDbgSlab_Calloc(size, file, line, DeeDbgObject_Calloc(size, file, line))
#define DeeDbgObject_FTryMalloc(size, file, line)         DeeDbgSlab_TryMalloc(size, file, line, DeeDbgObject_TryMalloc(size, file, line))
#define DeeDbgObject_FTryCalloc(size, file, line)         DeeDbgSlab_TryCalloc(size, file, line, DeeDbgObject_TryCalloc(size, file, line))
#define DeeDbgObject_FUntrackAlloc(ptr, size, file, line) DeeDbgSlab_UntrackAlloc(size, ptr, file, line, DeeDbgObject_UntrackAlloc(ptr, file, line))
#define DeeDbgObject_FFree(ptr, size, file, line)         DeeDbgSlab_Free(size, ptr, file, line, DeeDbgObject_Free(ptr, file, line))


/* Same as the regular malloc functions, but use the same allocation methods
 * that would be used by `Dee_TYPE_CONSTRUCTOR_INIT_FIXED', meaning that
 * pointers returned by these macros have binary compatibility with them. */
#define DeeObject_MALLOC(T)                               ((T *)DeeObject_FMalloc(sizeof(T)))
#define DeeObject_CALLOC(T)                               ((T *)DeeObject_FCalloc(sizeof(T)))
#define DeeObject_TRYMALLOC(T)                            ((T *)DeeObject_FTryMalloc(sizeof(T)))
#define DeeObject_TRYCALLOC(T)                            ((T *)DeeObject_FTryCalloc(sizeof(T)))
#define DeeObject_UNTRACK_ALLOC(T, ptr)                   ((T *)DeeObject_FUntrackAlloc(ptr, sizeof(T)))
#define DeeObject_FREE(typed_ptr)                         DeeObject_FFree(typed_ptr, sizeof(*(typed_ptr)))
#define DeeDbgObject_MALLOC(T, file, line)                ((T *)DeeDbgObject_FMalloc(sizeof(T), file, line))
#define DeeDbgObject_CALLOC(T, file, line)                ((T *)DeeDbgObject_FCalloc(sizeof(T), file, line))
#define DeeDbgObject_TRYMALLOC(T, file, line)             ((T *)DeeDbgObject_FTryMalloc(sizeof(T), file, line))
#define DeeDbgObject_TRYCALLOC(T, file, line)             ((T *)DeeDbgObject_FTryCalloc(sizeof(T), file, line))
#define DeeDbgObject_UNTRACK_ALLOC(typed_ptr, file, line) ((T *)DeeDbgObject_FUntrackAlloc(ptr, sizeof(T), file, line))
#define DeeDbgObject_FREE(T, ptr, file, line)             DeeDbgObject_FFree(typed_ptr, sizeof(*(typed_ptr)), file, line)


#ifdef __CC__
#ifdef GUARD_DEEMON_TYPE_H
/* Specifies an allocator that may provides optimizations
 * for types with a FIXED size (which most objects have). */
/*!export Dee_TYPE_CONSTRUCTOR_INIT_SIZED(include("type.h"))*/
/*!export Dee_TYPE_CONSTRUCTOR_INIT_FIXED(include("type.h"))*/
#define Dee_TYPE_CONSTRUCTOR_INIT_SIZED(tp_instance_size, tp_ctor, tp_copy_ctor, tp_any_ctor, tp_any_ctor_kw, tp_serialize)            \
	Dee_TYPE_CONSTRUCTOR_INIT_ALLOC(tp_ctor, tp_copy_ctor, tp_any_ctor, tp_any_ctor_kw, tp_serialize,                                  \
	                                DeeSlab_GetMalloc(tp_instance_size, (void *(DCALL *)(void))(void *)(uintptr_t)(tp_instance_size)), \
	                                DeeSlab_GetFree(tp_instance_size, (void (DCALL *)(void *))(Dee_funptr_t)NULL))
#define Dee_TYPE_CONSTRUCTOR_INIT_FIXED(T, tp_ctor, tp_copy_ctor, tp_any_ctor, tp_any_ctor_kw, tp_serialize) \
	Dee_TYPE_CONSTRUCTOR_INIT_SIZED(sizeof(T), tp_ctor, tp_copy_ctor, tp_any_ctor, tp_any_ctor_kw, tp_serialize)
#endif /* GUARD_DEEMON_TYPE_H */

/* Define the deemon api's alloca() function (if it can be implemented) */
#if !defined(Dee_Alloca) && defined(CONFIG_HAVE_alloca)
#if defined(__NO_hybrid_dbg_alignment) || defined(__INTELLISENSE__)
#define Dee_Alloca(num_bytes) alloca(num_bytes)
#else /* __NO_hybrid_dbg_alignment || __INTELLISENSE__ */
FORCELOCAL WUNUSED void *DCALL DeeDbg_AllocaCleanup(void *ptr) {
	__hybrid_dbg_alignment_enable();
	return ptr;
}
#define Dee_Alloca(num_bytes) (__hybrid_dbg_alignment_disable(), DeeDbg_AllocaCleanup(alloca(num_bytes)))
#endif /* !__NO_hybrid_dbg_alignment && !__INTELLISENSE__ */
#endif /* !Dee_Alloca && CONFIG_HAVE_alloca */
#if !defined(Dee_Allocac) && defined(Dee_Alloca)
#define Dee_Allocac(elem_count, elem_size) \
	Dee_Alloca(_Dee_MalloccBufsize(elem_count, elem_size))
#define Dee_Allocaoc(base_offset, elem_count, elem_size) \
	Dee_Alloca(_Dee_MallococBufsize(base_offset, elem_count, elem_size))
#endif /* !Dee_Allocac && Dee_Alloca */


/* A hybrid between alloca and malloc, using alloca for
 * small allocations, but malloc() for larger ones.
 * NOTE: In all cases, 'Dee_Freea()' should be used to clean up a
 *       pointer previously allocated using 'Dee_Malloca()' and
 *       friends. */
#if !defined(Dee_Alloca) || !defined(__NO_hybrid_dbg_alignment)
#define Dee_Malloca(s)    Dee_Malloc(s)
#define Dee_Calloca(s)    Dee_Calloc(s)
#define Dee_TryMalloca(s) Dee_TryMalloc(s)
#define Dee_TryCalloca(s) Dee_TryCalloc(s)
#define Dee_Freea(p)      Dee_Free(p)
#define Dee_XFreea(p)     Dee_Free(p)
#else /* !Dee_Alloca || !__NO_hybrid_dbg_alignment */

#ifdef __SIZEOF_POINTER__
/* WARNING: This makes Dee_Malloca() unsuitable for floating point allocations. */
#define Dee_MALLOCA_ALIGN __SIZEOF_POINTER__
#else /* __SIZEOF_POINTER__ */
#define Dee_MALLOCA_ALIGN 8
#endif /* !__SIZEOF_POINTER__ */
#define Dee_MALLOCA_MAX   512

#ifndef CONFIG_HAVE_memset
#define CONFIG_HAVE_memset
DECL_BEGIN
#undef memset
#define memset dee_memset /*!export-*/
LOCAL WUNUSED ATTR_OUTS(1, 3) void *
dee_memset(void *__restrict dst, int byte, size_t num_bytes) { /*!export-*/
	uint8_t *dst_p = (uint8_t *)dst;
	while (num_bytes--)
		*dst_p++ = (uint8_t)(unsigned int)byte;
	return dst;
}
DECL_END
#endif /* !CONFIG_HAVE_memset */

#ifndef CONFIG_HAVE_bzero
#define CONFIG_HAVE_bzero
#undef bzero
#define bzero(dst, num_bytes) (void)memset(dst, 0, num_bytes) /*!export-*/
#endif /* !CONFIG_HAVE_bzero */



#ifdef NDEBUG
#define Dee_MALLOCA_KEY_ALLOCA        0
#define Dee_MALLOCA_KEY_MALLOC        1
#define Dee_MALLOCA_SKEW_ALLOCA(p, s) (p)
#define Dee_MALLOCA_GETKEY(p)         ((__BYTE_TYPE__ *)(p))[-Dee_MALLOCA_ALIGN]
#define Dee_MALLOCA_MUSTFREE(p)       (Dee_MALLOCA_GETKEY(p) != Dee_MALLOCA_KEY_ALLOCA)
#else /* NDEBUG */
#define Dee_MALLOCA_KEY_ALLOCA 0x7c
#define Dee_MALLOCA_KEY_MALLOC 0xb3
#define Dee_MALLOCA_GETKEY(p)  ((__BYTE_TYPE__ *)(p))[-Dee_MALLOCA_ALIGN]
#define Dee_MALLOCA_MUSTFREE(p)                                    \
	(Dee_ASSERT(Dee_MALLOCA_GETKEY(p) == Dee_MALLOCA_KEY_ALLOCA || \
	            Dee_MALLOCA_GETKEY(p) == Dee_MALLOCA_KEY_MALLOC),  \
	 Dee_MALLOCA_GETKEY(p) == Dee_MALLOCA_KEY_MALLOC)
#define Dee_MALLOCA_SKEW_ALLOCA(p, s) memset(p, 0xcd, s)
#endif /* !NDEBUG */
#ifndef __NO_XBLOCK
#define Dee_MallocaStack(s)                                            \
	XBLOCK({                                                           \
		size_t const _s_     = (s) + Dee_MALLOCA_ALIGN;                \
		__BYTE_TYPE__ *_res_ = (__BYTE_TYPE__ *)Dee_Alloca(_s_);       \
		*_res_               = Dee_MALLOCA_KEY_ALLOCA;                 \
		_res_ += Dee_MALLOCA_ALIGN;                                    \
		(void)Dee_MALLOCA_SKEW_ALLOCA(_res_, _s_ - Dee_MALLOCA_ALIGN); \
		XRETURN((void *)_res_);                                        \
	})
#define Dee_MallocaHeap(s)                                       \
	XBLOCK({                                                     \
		size_t const _s_     = (s) + Dee_MALLOCA_ALIGN;          \
		__BYTE_TYPE__ *_res_ = (__BYTE_TYPE__ *)Dee_Malloc(_s_); \
		if (_res_)                                               \
			*_res_ = Dee_MALLOCA_KEY_MALLOC;                     \
		_res_ += Dee_MALLOCA_ALIGN;                              \
		XRETURN((void *)_res_);                                  \
	})
#define Dee_TryMallocaHeap(s)                                       \
	XBLOCK({                                                        \
		size_t const _s_     = (s) + Dee_MALLOCA_ALIGN;             \
		__BYTE_TYPE__ *_res_ = (__BYTE_TYPE__ *)Dee_TryMalloc(_s_); \
		if (_res_)                                                  \
			*_res_ = Dee_MALLOCA_KEY_MALLOC;                        \
		_res_ += Dee_MALLOCA_ALIGN;                                 \
		XRETURN((void *)_res_);                                     \
	})
#define Dee_Malloca(s)                                                     \
	XBLOCK({                                                               \
		size_t const _s_ = (s) + Dee_MALLOCA_ALIGN;                        \
		__BYTE_TYPE__ *_res_;                                              \
		if (_s_ > Dee_MALLOCA_MAX) {                                       \
			_res_ = (__BYTE_TYPE__ *)Dee_Malloc(_s_);                      \
			if (_res_) {                                                   \
				*_res_ = Dee_MALLOCA_KEY_MALLOC;                           \
				_res_ += Dee_MALLOCA_ALIGN;                                \
			}                                                              \
		} else {                                                           \
			_res_  = (__BYTE_TYPE__ *)Dee_Alloca(_s_);                     \
			*_res_ = Dee_MALLOCA_KEY_ALLOCA;                               \
			_res_ += Dee_MALLOCA_ALIGN;                                    \
			(void)Dee_MALLOCA_SKEW_ALLOCA(_res_, _s_ - Dee_MALLOCA_ALIGN); \
		}                                                                  \
		XRETURN((void *)_res_);                                            \
	})
#define Dee_TryMalloca(s)                                                  \
	XBLOCK({                                                               \
		size_t const _s_ = (s) + Dee_MALLOCA_ALIGN;                        \
		__BYTE_TYPE__ *_res_;                                              \
		if (_s_ > Dee_MALLOCA_MAX) {                                       \
			_res_ = (__BYTE_TYPE__ *)Dee_TryMalloc(_s_);                   \
			if (_res_) {                                                   \
				*_res_ = Dee_MALLOCA_KEY_MALLOC;                           \
				_res_ += Dee_MALLOCA_ALIGN;                                \
			}                                                              \
		} else {                                                           \
			_res_  = (__BYTE_TYPE__ *)Dee_Alloca(_s_);                     \
			*_res_ = Dee_MALLOCA_KEY_ALLOCA;                               \
			_res_ += Dee_MALLOCA_ALIGN;                                    \
			(void)Dee_MALLOCA_SKEW_ALLOCA(_res_, _s_ - Dee_MALLOCA_ALIGN); \
		}                                                                  \
		XRETURN((void *)_res_);                                            \
	})
#define Dee_CallocaStack(s)                                      \
	XBLOCK({                                                     \
		size_t const _s_     = (s) + Dee_MALLOCA_ALIGN;          \
		__BYTE_TYPE__ *_res_ = (__BYTE_TYPE__ *)Dee_Alloca(_s_); \
		*_res_               = Dee_MALLOCA_KEY_ALLOCA;           \
		_res_ += Dee_MALLOCA_ALIGN;                              \
		bzero(_res_, _s_ - Dee_MALLOCA_ALIGN);                   \
		XRETURN((void *)_res_);                                  \
	})
#define Dee_CallocaHeap(s)                                       \
	XBLOCK({                                                     \
		size_t const _s_     = (s) + Dee_MALLOCA_ALIGN;          \
		__BYTE_TYPE__ *_res_ = (__BYTE_TYPE__ *)Dee_Calloc(_s_); \
		if (_res_) {                                             \
			*_res_ = Dee_MALLOCA_KEY_MALLOC;                     \
			_res_ += Dee_MALLOCA_ALIGN;                          \
		}                                                        \
		XRETURN((void *)_res_);                                  \
	})
#define Dee_TryCallocaHeap(s)                                       \
	XBLOCK({                                                        \
		size_t const _s_     = (s) + Dee_MALLOCA_ALIGN;             \
		__BYTE_TYPE__ *_res_ = (__BYTE_TYPE__ *)Dee_TryCalloc(_s_); \
		if (_res_) {                                                \
			*_res_ = Dee_MALLOCA_KEY_MALLOC;                        \
			_res_ += Dee_MALLOCA_ALIGN;                             \
		}                                                           \
		XRETURN((void *)_res_);                                     \
	})
#define Dee_Calloca(s)                                 \
	XBLOCK({                                           \
		size_t const _s_ = (s) + Dee_MALLOCA_ALIGN;    \
		__BYTE_TYPE__ *_res_;                          \
		if (_s_ > Dee_MALLOCA_MAX) {                   \
			_res_ = (__BYTE_TYPE__ *)Dee_Calloc(_s_);  \
			if (_res_) {                               \
				*_res_ = Dee_MALLOCA_KEY_MALLOC;       \
				_res_ += Dee_MALLOCA_ALIGN;            \
			}                                          \
		} else {                                       \
			_res_  = (__BYTE_TYPE__ *)Dee_Alloca(_s_); \
			*_res_ = Dee_MALLOCA_KEY_ALLOCA;           \
			_res_ += Dee_MALLOCA_ALIGN;                \
			bzero(_res_, _s_ - Dee_MALLOCA_ALIGN);     \
		}                                              \
		XRETURN((void *)_res_);                        \
	})
#define Dee_TryCalloca(s)                                \
	XBLOCK({                                             \
		size_t const _s_ = (s) + Dee_MALLOCA_ALIGN;      \
		__BYTE_TYPE__ *_res_;                            \
		if (_s_ > Dee_MALLOCA_MAX) {                     \
			_res_ = (__BYTE_TYPE__ *)Dee_TryCalloc(_s_); \
			if (_res_) {                                 \
				*_res_ = Dee_MALLOCA_KEY_MALLOC;         \
				_res_ += Dee_MALLOCA_ALIGN;              \
			}                                            \
		} else {                                         \
			_res_  = (__BYTE_TYPE__ *)Dee_Alloca(_s_);   \
			*_res_ = Dee_MALLOCA_KEY_ALLOCA;             \
			_res_ += Dee_MALLOCA_ALIGN;                  \
			bzero(_res_, _s_ - Dee_MALLOCA_ALIGN);       \
		}                                                \
		XRETURN((void *)_res_);                          \
	})
#define Dee_Freea(p)                                                      \
	XBLOCK({                                                              \
		void *const _p_ = (p);                                            \
		if (Dee_MALLOCA_MUSTFREE(_p_))                                    \
			Dee_Free((void *)((__BYTE_TYPE__ *)_p_ - Dee_MALLOCA_ALIGN)); \
		(void)0;                                                          \
	})
#define Dee_XFreea(p)                                                     \
	XBLOCK({                                                              \
		void *const _p_ = (p);                                            \
		if (_p_ && Dee_MALLOCA_MUSTFREE(_p_))                             \
			Dee_Free((void *)((__BYTE_TYPE__ *)_p_ - Dee_MALLOCA_ALIGN)); \
		(void)0;                                                          \
	})
#else /* !__NO_XBLOCK */
#ifdef NDEBUG
#define Dee_MallocaHeap(s)  Dee_MallocaHeap(s)
LOCAL void *(DCALL Dee_MallocaHeap)(size_t s) {
	__BYTE_TYPE__ *res;
	res = (__BYTE_TYPE__ *)Dee_Malloc(s + Dee_MALLOCA_ALIGN);
	if likely(res) {
		*res = Dee_MALLOCA_KEY_MALLOC;
		res += Dee_MALLOCA_ALIGN;
	}
	return (void *)res;
}
#define Dee_TryMallocaHeap(s)  Dee_TryMallocaHeap(s)
LOCAL void *(DCALL Dee_TryMallocaHeap)(size_t s) {
	__BYTE_TYPE__ *res;
	res = (__BYTE_TYPE__ *)Dee_TryMalloc(s + Dee_MALLOCA_ALIGN);
	if likely(res) {
		*res = Dee_MALLOCA_KEY_MALLOC;
		res += Dee_MALLOCA_ALIGN;
	}
	return (void *)res;
}
#define Dee_CallocaHeap(s)  Dee_CallocaHeap(s)
LOCAL void *(DCALL Dee_CallocaHeap)(size_t s) {
	__BYTE_TYPE__ *res;
	res = (__BYTE_TYPE__ *)Dee_Calloc(s + Dee_MALLOCA_ALIGN);
	if likely(res) {
		*res = Dee_MALLOCA_KEY_MALLOC;
		res += Dee_MALLOCA_ALIGN;
	}
	return (void *)res;
}
#define Dee_TryCallocaHeap(s)  Dee_TryCallocaHeap(s)
LOCAL void *(DCALL Dee_TryCallocaHeap)(size_t s) {
	__BYTE_TYPE__ *res;
	res = (__BYTE_TYPE__ *)Dee_TryCalloc(s + Dee_MALLOCA_ALIGN);
	if likely(res) {
		*res = Dee_MALLOCA_KEY_MALLOC;
		res += Dee_MALLOCA_ALIGN;
	}
	return (void *)res;
}
#else /* NDEBUG */
#define Dee_MallocaHeap(s) DeeDbg_AMallocHeap(s, __FILE__, __LINE__)
LOCAL WUNUSED void *(DCALL DeeDbg_AMallocHeap)(size_t s, char const *file, int line) {
	__BYTE_TYPE__ *res;
	res = (__BYTE_TYPE__ *)DeeDbg_Malloc(s + Dee_MALLOCA_ALIGN, file, line);
	if likely(res) {
		*res = Dee_MALLOCA_KEY_MALLOC;
		res += Dee_MALLOCA_ALIGN;
	}
	return (void *)res;
}
#define Dee_TryMallocaHeap(s) DeeDbg_ATryMallocHeap(s, __FILE__, __LINE__)
LOCAL WUNUSED void *(DCALL DeeDbg_ATryMallocHeap)(size_t s, char const *file, int line) {
	__BYTE_TYPE__ *res;
	res = (__BYTE_TYPE__ *)DeeDbg_TryMalloc(s + Dee_MALLOCA_ALIGN, file, line);
	if likely(res) {
		*res = Dee_MALLOCA_KEY_MALLOC;
		res += Dee_MALLOCA_ALIGN;
	}
	return (void *)res;
}
#define Dee_CallocaHeap(s) DeeDbg_ACallocHeap(s, __FILE__, __LINE__)
LOCAL WUNUSED void *(DCALL DeeDbg_ACallocHeap)(size_t s, char const *file, int line) {
	__BYTE_TYPE__ *res;
	res = (__BYTE_TYPE__ *)DeeDbg_Calloc(s + Dee_MALLOCA_ALIGN, file, line);
	if likely(res) {
		*res = Dee_MALLOCA_KEY_MALLOC;
		res += Dee_MALLOCA_ALIGN;
	}
	return (void *)res;
}
#define Dee_TryCallocaHeap(s) DeeDbg_ATryCallocHeap(s, __FILE__, __LINE__)
LOCAL WUNUSED void *(DCALL DeeDbg_ATryCallocHeap)(size_t s, char const *file, int line) {
	__BYTE_TYPE__ *res;
	res = (__BYTE_TYPE__ *)DeeDbg_TryCalloc(s + Dee_MALLOCA_ALIGN, file, line);
	if likely(res) {
		*res = Dee_MALLOCA_KEY_MALLOC;
		res += Dee_MALLOCA_ALIGN;
	}
	return (void *)res;
}
#endif /* !NDEBUG */
#define Dee_MallocaStack(s) Dee_MallocaStack_init(Dee_Alloca((s) + Dee_MALLOCA_ALIGN), (s))
#define Dee_Malloca(s)      ((s) > Dee_MALLOCA_MAX - Dee_MALLOCA_ALIGN ? Dee_MallocaHeap(s) : Dee_MallocaStack(s))
#define Dee_TryMalloca(s)   ((s) > Dee_MALLOCA_MAX - Dee_MALLOCA_ALIGN ? Dee_TryMallocaHeap(s) : Dee_MallocaStack(s))
#ifdef NDEBUG
#define Dee_MallocaStack_init(p, s) _Dee_PRIVATE_MallocaStack_init_(p)
LOCAL WUNUSED NONNULL((1)) void *
(DCALL _Dee_PRIVATE_MallocaStack_init_)(void *p) {
	*(__BYTE_TYPE__ *)p = Dee_MALLOCA_KEY_ALLOCA;
	return (__BYTE_TYPE__ *)p + Dee_MALLOCA_ALIGN;
}
#else /* NDEBUG */
LOCAL WUNUSED NONNULL((1)) void *
(DCALL Dee_MallocaStack_init)(void *p, size_t s) {
	*(__BYTE_TYPE__ *)p = Dee_MALLOCA_KEY_ALLOCA;
	return Dee_MALLOCA_SKEW_ALLOCA((__BYTE_TYPE__ *)p + Dee_MALLOCA_ALIGN, s);
}
#endif /* !NDEBUG */
#define Dee_CallocaStack(s) Dee_CallocaStack_init(Dee_Alloca((s) + Dee_MALLOCA_ALIGN), (s))
#define Dee_Calloca(s)      ((s) > Dee_MALLOCA_MAX - Dee_MALLOCA_ALIGN ? Dee_CallocaHeap(s) : Dee_CallocaStack(s))
#define Dee_TryCalloca(s)   ((s) > Dee_MALLOCA_MAX - Dee_MALLOCA_ALIGN ? Dee_TryCallocaHeap(s) : Dee_CallocaStack(s))
LOCAL WUNUSED NONNULL((1)) void *
(DCALL Dee_CallocaStack_init)(void *p, size_t s) {
	void *result;
	*(__BYTE_TYPE__ *)p = Dee_MALLOCA_KEY_ALLOCA;
	result = (__BYTE_TYPE__ *)p + Dee_MALLOCA_ALIGN;
	bzero(result, s);
	return result;
}

#define Dee_Freea(p)  Dee_Freea(p)
#define Dee_XFreea(p) Dee_XFreea(p)
LOCAL NONNULL((1)) void (DCALL Dee_Freea)(void *p) {
	if (Dee_MALLOCA_MUSTFREE(p))
		Dee_Free((void *)((__BYTE_TYPE__ *)p - Dee_MALLOCA_ALIGN));
}

LOCAL void (DCALL Dee_XFreea)(void *p) {
	if (p && Dee_MALLOCA_MUSTFREE(p))
		Dee_Free((void *)((__BYTE_TYPE__ *)p - Dee_MALLOCA_ALIGN));
}
#endif /* __NO_XBLOCK */

#define Dee_MallocaNoFail(p, s)                                                  \
	do {                                                                         \
		size_t const _s_ = (s) + Dee_MALLOCA_ALIGN;                              \
		if (_s_ > Dee_MALLOCA_MAX &&                                             \
		    (*(void **)&(p) = Dee_Malloc(_s_)) != __NULLPTR) {                   \
			*(__BYTE_TYPE__ *)(p) = Dee_MALLOCA_KEY_MALLOC;                      \
			*(void **)&(p) = (__BYTE_TYPE__ *)(p) + Dee_MALLOCA_ALIGN;           \
		} else {                                                                 \
			*(void **)&(p)        = Dee_Alloca(_s_);                             \
			*(__BYTE_TYPE__ *)(p) = Dee_MALLOCA_KEY_ALLOCA;                      \
			*(void **)&(p) = (__BYTE_TYPE__ *)(p) + Dee_MALLOCA_ALIGN;           \
			(void)Dee_MALLOCA_SKEW_ALLOCA((void *)(p), _s_ - Dee_MALLOCA_ALIGN); \
		}                                                                        \
	}	__WHILE0
#define Dee_TryMallocaNoFail(p, s)                                               \
	do {                                                                         \
		size_t const _s_ = (s) + Dee_MALLOCA_ALIGN;                              \
		if (_s_ > Dee_MALLOCA_MAX &&                                             \
		    (*(void **)&(p) = Dee_TryMalloc(_s_)) != __NULLPTR) {                \
			*(__BYTE_TYPE__ *)(p) = Dee_MALLOCA_KEY_MALLOC;                      \
			*(void **)&(p) = (__BYTE_TYPE__ *)(p) + Dee_MALLOCA_ALIGN;           \
		} else {                                                                 \
			*(void **)&(p)        = Dee_Alloca(_s_);                             \
			*(__BYTE_TYPE__ *)(p) = Dee_MALLOCA_KEY_ALLOCA;                      \
			*(void **)&(p) = (__BYTE_TYPE__ *)(p) + Dee_MALLOCA_ALIGN;           \
			(void)Dee_MALLOCA_SKEW_ALLOCA((void *)(p), _s_ - Dee_MALLOCA_ALIGN); \
		}                                                                        \
	}	__WHILE0
#define Dee_CallocaNoFail(p, s)                                        \
	do {                                                               \
		size_t const _s_ = (s) + Dee_MALLOCA_ALIGN;                    \
		if (_s_ > Dee_MALLOCA_MAX &&                                   \
		    (*(void **)&(p) = Dee_Calloc(_s_)) != __NULLPTR) {         \
			*(__BYTE_TYPE__ *)(p) = Dee_MALLOCA_KEY_MALLOC;            \
			*(void **)&(p) = (__BYTE_TYPE__ *)(p) + Dee_MALLOCA_ALIGN; \
		} else {                                                       \
			*(void **)&(p)        = Dee_Alloca(_s_);                   \
			*(__BYTE_TYPE__ *)(p) = Dee_MALLOCA_KEY_ALLOCA;            \
			*(void **)&(p) = (__BYTE_TYPE__ *)(p) + Dee_MALLOCA_ALIGN; \
			bzero((void *)(p), _s_ - Dee_MALLOCA_ALIGN);               \
		}                                                              \
	}	__WHILE0
#define Dee_TryCallocaNoFail(p, s)                                     \
	do {                                                               \
		size_t const _s_ = (s) + Dee_MALLOCA_ALIGN;                    \
		if (_s_ > Dee_MALLOCA_MAX &&                                   \
		    (*(void **)&(p) = Dee_TryCalloc(_s_)) != __NULLPTR) {      \
			*(__BYTE_TYPE__ *)(p) = Dee_MALLOCA_KEY_MALLOC;            \
			*(void **)&(p) = (__BYTE_TYPE__ *)(p) + Dee_MALLOCA_ALIGN; \
		} else {                                                       \
			*(void **)&(p)        = Dee_Alloca(_s_);                   \
			*(__BYTE_TYPE__ *)(p) = Dee_MALLOCA_KEY_ALLOCA;            \
			*(void **)&(p) = (__BYTE_TYPE__ *)(p) + Dee_MALLOCA_ALIGN; \
			bzero((void *)(p), _s_ - Dee_MALLOCA_ALIGN);               \
		}                                                              \
	}	__WHILE0
#endif /* Dee_Alloca && __NO_hybrid_dbg_alignment */

#ifdef Dee_MallocaNoFail
#define Dee_MallocaNoFailc(p, elem_count, elem_size)               Dee_MallocaNoFail(p, _Dee_MalloccBufsize(elem_count, elem_size))
#define Dee_MallocaNoFailoc(p, base_offset, elem_count, elem_size) Dee_MallocaNoFail(p, _Dee_MallococBufsize(base_offset, elem_count, elem_size))
#endif /* Dee_MallocaNoFail */
#define Dee_Mallocac(elem_count, elem_size)                  Dee_Malloca(_Dee_MalloccBufsize(elem_count, elem_size))
#define Dee_Callocac(elem_count, elem_size)                  Dee_Calloca(_Dee_MalloccBufsize(elem_count, elem_size))
#define Dee_TryMallocac(elem_count, elem_size)               Dee_TryMalloca(_Dee_MalloccBufsize(elem_count, elem_size))
#define Dee_TryCallocac(elem_count, elem_size)               Dee_TryCalloca(_Dee_MalloccBufsize(elem_count, elem_size))
#define Dee_Mallocaoc(base_offset, elem_count, elem_size)    Dee_Malloca(_Dee_MallococBufsize(base_offset, elem_count, elem_size))
#define Dee_Callocaoc(base_offset, elem_count, elem_size)    Dee_Calloca(_Dee_MallococBufsize(base_offset, elem_count, elem_size))
#define Dee_TryMallocaoc(base_offset, elem_count, elem_size) Dee_TryMalloca(_Dee_MallococBufsize(base_offset, elem_count, elem_size))
#define Dee_TryCallocaoc(base_offset, elem_count, elem_size) Dee_TryCalloca(_Dee_MallococBufsize(base_offset, elem_count, elem_size))

#endif /* __CC__ */

DECL_END

#endif /* !GUARD_DEEMON_ALLOC_H */
