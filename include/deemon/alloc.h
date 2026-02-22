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
#include "util/slab-config.h" /* Dee_SLAB_CHUNKSIZE_FOREACH, Dee_SLAB_CHUNKSIZE_MAX */

#ifdef __CC__
#include <hybrid/__debug-alignment.h> /* __NO_hybrid_dbg_alignment, __hybrid_dbg_alignment_disable, __hybrid_dbg_alignment_enable */
#include <hybrid/__overflow.h>        /* __hybrid_overflow_uadd, __hybrid_overflow_umul */
#include <hybrid/host.h>              /* __linux__, __unix__ */
#include <hybrid/typecore.h>          /* __BYTE_TYPE__, __SIZEOF_POINTER__ */

/*!fixincludes fake_include "system-features.h" // _alloca, _msize, malloc, malloc_usable_size */
/*!fixincludes fake_include "type.h"            // CONFIG_FIXED_ALLOCATOR_S_IS_AUTO, Dee_TYPE_CONSTRUCTOR_INIT_* */

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

#ifndef CONFIG_EXPERIMENTAL_CUSTOM_HEAP
#ifdef CONFIG_NO__msize
#undef CONFIG_HAVE__msize
#elif !defined(CONFIG_HAVE__msize) && \
      (defined(_msize) || defined(___msize_defined) || (defined(CONFIG_HAVE_MALLOC_H) && \
       defined(_MSC_VER)))
#define CONFIG_HAVE__msize
#endif

#ifdef CONFIG_NO_malloc_usable_size
#undef CONFIG_HAVE_malloc_usable_size
#elif !defined(CONFIG_HAVE_malloc_usable_size) && \
      (defined(malloc_usable_size) || defined(__malloc_usable_size_defined) || \
       (defined(CONFIG_HAVE_MALLOC_H) && (defined(__linux__) || defined(__linux) || \
       defined(linux) || defined(__unix__) || defined(__unix) || defined(unix))))
#define CONFIG_HAVE_malloc_usable_size
#endif
#endif /* !CONFIG_EXPERIMENTAL_CUSTOM_HEAP */

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

#ifdef CONFIG_EXPERIMENTAL_CUSTOM_HEAP
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

#define DeeDbg_TryReallocInPlace(ptr, n_bytes, file, line) Dee_TryReallocInPlace(ptr, n_bytes)
#define Dee_MallocUsableSize(ptr)        Dee_MallocUsableSize(ptr) /* TODO: remove "#ifdef Dee_MallocUsableSize" after CONFIG_EXPERIMENTAL_CUSTOM_HEAP */
#define Dee_MallocUsableSizeNonNull(ptr) Dee_MallocUsableSize(ptr)
#else /* CONFIG_EXPERIMENTAL_CUSTOM_HEAP */
/* If supported by the OS, provide a way to determine the allocated size of an malloc-pointer. */
#undef Dee_MallocUsableSize
#ifdef CONFIG_HAVE_malloc_usable_size
#define Dee_MallocUsableSize(ptr)        malloc_usable_size(ptr)
#define Dee_MallocUsableSizeNonNull(ptr) malloc_usable_size(ptr)
#elif defined(CONFIG_HAVE__msize)
#define Dee_MallocUsableSize(ptr)        (likely(ptr) ? _msize(ptr) : 0)
#define Dee_MallocUsableSizeNonNull(ptr) _msize(ptr)
#endif /* ... */
#endif /* !CONFIG_EXPERIMENTAL_CUSTOM_HEAP */

#ifndef NDEBUG
#define Dee_Malloc(n_bytes)          DeeDbg_Malloc(n_bytes, __FILE__, __LINE__)
#define Dee_Calloc(n_bytes)          DeeDbg_Calloc(n_bytes, __FILE__, __LINE__)
#define Dee_Realloc(ptr, n_bytes)    DeeDbg_Realloc(ptr, n_bytes, __FILE__, __LINE__)
#define Dee_TryMalloc(n_bytes)       DeeDbg_TryMalloc(n_bytes, __FILE__, __LINE__)
#define Dee_TryCalloc(n_bytes)       DeeDbg_TryCalloc(n_bytes, __FILE__, __LINE__)
#define Dee_TryRealloc(ptr, n_bytes) DeeDbg_TryRealloc(ptr, n_bytes, __FILE__, __LINE__)
#define Dee_Free(ptr)                DeeDbg_Free(ptr, __FILE__, __LINE__)
#define Dee_UntrackAlloc(ptr)        DeeDbg_UntrackAlloc(ptr, __FILE__, __LINE__)
#ifdef CONFIG_EXPERIMENTAL_CUSTOM_HEAP
#define Dee_Memalign(min_alignment, n_bytes)    DeeDbg_Memalign(min_alignment, n_bytes, __FILE__, __LINE__)
#define Dee_TryMemalign(min_alignment, n_bytes) DeeDbg_TryMemalign(min_alignment, n_bytes, __FILE__, __LINE__)
#endif /* CONFIG_EXPERIMENTAL_CUSTOM_HEAP */
#else /* !NDEBUG */
#define DeeDbg_Malloc(n_bytes, file, line)          Dee_Malloc(n_bytes)
#define DeeDbg_Calloc(n_bytes, file, line)          Dee_Calloc(n_bytes)
#define DeeDbg_Realloc(ptr, n_bytes, file, line)    Dee_Realloc(ptr, n_bytes)
#define DeeDbg_TryMalloc(n_bytes, file, line)       Dee_TryMalloc(n_bytes)
#define DeeDbg_TryCalloc(n_bytes, file, line)       Dee_TryCalloc(n_bytes)
#define DeeDbg_TryRealloc(ptr, n_bytes, file, line) Dee_TryRealloc(ptr, n_bytes)
#define DeeDbg_Free(ptr, file, line)                Dee_Free(ptr)
#define Dee_UntrackAlloc(ptr)                       (ptr)
#ifdef CONFIG_EXPERIMENTAL_CUSTOM_HEAP
#define DeeDbg_Memalign(min_alignment, n_bytes, file, line)    Dee_Memalign(min_alignment, n_bytes)
#define DeeDbg_TryMemalign(min_alignment, n_bytes, file, line) Dee_TryMemalign(min_alignment, n_bytes)
#endif /* CONFIG_EXPERIMENTAL_CUSTOM_HEAP */
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
#ifdef CONFIG_EXPERIMENTAL_REWORKED_SLAB_ALLOCATOR
#define DeeObject_Free            Dee_Free
#define DeeObject_UntrackAlloc    Dee_UntrackAlloc
#endif /* CONFIG_EXPERIMENTAL_REWORKED_SLAB_ALLOCATOR */
#define DeeDbgObject_Malloc       DeeDbg_Malloc
#define DeeDbgObject_Calloc       DeeDbg_Calloc
#define DeeDbgObject_Realloc      DeeDbg_Realloc
#define DeeDbgObject_TryMalloc    DeeDbg_TryMalloc
#define DeeDbgObject_TryCalloc    DeeDbg_TryCalloc
#define DeeDbgObject_TryRealloc   DeeDbg_TryRealloc
#ifdef CONFIG_EXPERIMENTAL_REWORKED_SLAB_ALLOCATOR
#define DeeDbgObject_Free         DeeDbg_Free
#define DeeDbgObject_UntrackAlloc DeeDbg_UntrackAlloc
#endif /* CONFIG_EXPERIMENTAL_REWORKED_SLAB_ALLOCATOR */

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





#ifdef CONFIG_EXPERIMENTAL_REWORKED_SLAB_ALLOCATOR


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

#ifndef _Dee_PRIVATE_SLAB_SELECT_X
#define _Dee_PRIVATE_SLAB_SELECT_UNPACK(N, PREFIX, f, SUFFIX) N, PREFIX, f, SUFFIX
#define _Dee_PRIVATE_SLAB_SELECT_Z(n, N, PREFIX, f, SUFFIX)   n >= (N) ? (PREFIX f##n SUFFIX):
#define _Dee_PRIVATE_SLAB_SELECT_Y(args)                      _Dee_PRIVATE_SLAB_SELECT_Z args
#define _Dee_PRIVATE_SLAB_SELECT_X(n, args)                   _Dee_PRIVATE_SLAB_SELECT_Y((n, _Dee_PRIVATE_SLAB_SELECT_UNPACK args))
#endif /* !_Dee_PRIVATE_SLAB_SELECT_X */
#define _Dee_PRIVATE_SLAB_SELECT(N, PREFIX, f, SUFFIX, else) \
	(Dee_SLAB_CHUNKSIZE_FOREACH(_Dee_PRIVATE_SLAB_SELECT_X, (N, PREFIX, f, SUFFIX)) else)

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

#else /* CONFIG_EXPERIMENTAL_REWORKED_SLAB_ALLOCATOR */
#ifdef __CC__
#ifndef CONFIG_NO_OBJECT_SLABS
DFUNDEF void (DCALL DeeObject_Free)(void *ptr);
DFUNDEF void (DCALL DeeDbgObject_Free)(void *ptr, char const *file, int line);
DFUNDEF void *(DCALL DeeDbgObject_UntrackAlloc)(void *ptr, char const *file, int line);
#ifndef NDEBUG
#define DeeObject_Free(ptr)         DeeDbgObject_Free(ptr, __FILE__, __LINE__)
#define DeeObject_UntrackAlloc(ptr) DeeDbgObject_UntrackAlloc(ptr, __FILE__, __LINE__)
#else /* !NDEBUG */
#define DeeDbgObject_Free(ptr, file, line)         DeeObject_Free(ptr)
#define DeeDbgObject_UntrackAlloc(ptr, file, line) (ptr)
#define DeeObject_UntrackAlloc(ptr)                (ptr)
#endif /* NDEBUG */
#else /* !CONFIG_NO_OBJECT_SLABS */
#define DeeObject_Free            Dee_Free
#define DeeDbgObject_Free         DeeDbg_Free
#define DeeObject_UntrackAlloc    Dee_UntrackAlloc
#define DeeDbgObject_UntrackAlloc DeeDbg_UntrackAlloc
#endif /* CONFIG_NO_OBJECT_SLABS */
#else /* __CC__ */
#ifdef CONFIG_NO_OBJECT_SLABS
#define DeeObject_Free            Dee_Free
#define DeeDbgObject_Free         DeeDbg_Free
#define DeeObject_UntrackAlloc    Dee_UntrackAlloc
#define DeeDbgObject_UntrackAlloc DeeDbg_UntrackAlloc
#endif /* CONFIG_NO_OBJECT_SLABS */
#endif /* !__CC__ */

/* Free-functions, and their capabilities
 *
 * Dee_Free:                Accepts  Dee_Malloc()
 *                          Accepts  NULL
 *
 * DeeSlab_Free:            Accepts  DeeSlab_Malloc<M>()
 *                          Accepts  Dee_Malloc()
 *
 * DeeSlab_FFree(..., <N>):
 * DeeSlab_Free<N>:         Accepts  DeeSlab_Malloc<M>()         | M >= N
 *                          Accepts  Dee_Malloc()
 *
 * DeeObject_Free:          Accepts  DeeObject_SlabMalloc<M>()
 *                          Accepts  DeeObject_FMalloc(<M>)
 *                          Accepts  DeeObject_Malloc()
 *                          Accepts  NULL
 *
 * DeeObject_FFree(..., <N>):
 * DeeObject_SlabFree<N>:   Accepts  DeeObject_SlabMalloc<M>()   | M >= N
 *                          Accepts  DeeObject_FMalloc(<N>)      | M >= N
 *                          Accepts  DeeObject_Malloc()
 *
 *
 * DeeGCObject_Free:        Accepts  DeeGCObject_SlabMalloc<M>()
 *                          Accepts  DeeGCObject_FMalloc(<M>)
 *                          Accepts  DeeGCObject_Malloc()
 *                          Accepts  NULL
 *
 * DeeGCObject_FFree(..., <N>):
 * DeeGCObject_SlabFree<N>: Accepts  DeeGCObject_SlabMalloc<M>() | M >= N
 *                          Accepts  DeeGCObject_FMalloc(<M>)    | M >= N
 *                          Accepts  DeeGCObject_Malloc()
 *
 * The following free functions accept and ignore NULL-pointers:
 *   - Dee_Free
 *   - DeeObject_Free
 *   - DeeGCObject_Free
 *   - DeeSlab_Free
 *   - DeeSlab_FFree(..., <N>)
 *   - DeeSlab_Free<N>
 *   - DeeObject_FFree(..., <N>)
 *   - DeeObject_SlabFree<N>
 * The following free functions require the caller to pass non-NULL pointers:
 *   - DeeGCObject_FFree(..., <N>)
 *   - DeeGCObject_SlabFree<N>
 */



#ifndef CONFIG_NO_OBJECT_SLABS
/* Slab allocator functions.
 *
 * These operate in increments of at least sizeof(void *), with the acutal allocated
 * size being `X * sizeof(void *)' for `X = <X>' in `DeeObject_SlabMalloc<X>'.
 *
 * These allocator functions can be extremely fast, but have a fixed allocation limit.
 * They are designed to be used to speed up allocation of the many small helper objects
 * which can be found in deemon, most of which have a very short life-time, while others
 * exist for quite a while.
 *
 * Pointers to these functions are mainly found in the `tp_alloc()' and `tp_free()'
 * fields of types. However, you shouldn't invoke these functions directly but use
 * `DeeObject_FMalloc()' and `DeeObject_FFree()', as well as their helper macros instead.
 *
 * NOTES:
 * - You may not pass a NULL pointer to the SlabFree() functions.
 *   Unlike regular free()-like functions, doing so is illegal.
 *
 * - Due to the type-free-with-base quirk, pointers allocated by
 *   these functions can always be freed by `DeeObject_Free()',
 *   as well as any SlabFree() function with a smaller allocation
 *   size than what was used to allocate the original pointer.
 *   Doing this will not cause a memory leak, but may lead to
 *   inefficient usage of caches, as well as overall performance
 *   degradation.
 *   >> void *p = DeeObject_SlabMalloc8();
 *   >> if (p)
 *   >>     DeeObject_SlabFree4(p); // Allowed, but should not be done intentionally
 *   Again: this is required because objects being revived from
 *   their destructors may lead to that object continuing to exist
 *   with another type and free-function, which is then allowed to
 *   assume a smaller object size than what was given.
 *
 * - When declaring a sub-class of some type who's destructor you have no
 *   control over, such as is the case for user-defined classes, you must
 *   not declare your type to use slab allocators when the base type used
 *   some other, custom protocol, or simply used automatic allocators.
 *
 * - Because sub-classing an instance with a user-class requires
 *   knowledge of the base-type's instance size, using slab allocators
 *   for that type means that allocations may be larger than needed, with
 *   information about that additional overhead's size then becoming lost
 *   to a user-defined sub-class, leading to an unused gap of memory.
 */


#define _Dee_PRIVATE_SLAB_INDEXOF_CALLBACK(index, size) <= size *__SIZEOF_POINTER__ ? index##u:
#define _Dee_PRIVATE_SLAB_HASSIZE_CALLBACK(index, size) size
#define DeeSlab_IndexOf(size) (DeeSlab_ENUMERATE((size) _Dee_PRIVATE_SLAB_INDEXOF_CALLBACK) 0xffu)
#define DeeSlab_HasSize(size) (0 DeeSlab_ENUMERATE(|| (size) == _Dee_PRIVATE_SLAB_HASSIZE_CALLBACK))

/* Define slab-enabled object memory allocators, and override regular
 * slab allocator function to either include, or exclude debug information. */
/*[[[deemon
import * from deemon;

local SLAB_CONFIGURATIONS: {(string, {int...})...} = {
//	("defined(__i386__)", [1, 2, 3, 4, 5, 6]),
};
local DEFAULT_SLAB_CONFIGURATION: {int...} = [4, 5, 6, 8, 10];

global CURRENT_MACRO_LINES;
#define beginMacro() CURRENT_MACRO_LINES = []
#define printMacro(...) CURRENT_MACRO_LINES.append("".join({ __VA_ARGS__ }));
function endMacro(decl) {
	local defineLine = f"#define {decl}";
	CURRENT_MACRO_LINES = List(for (local x: CURRENT_MACRO_LINES) f"    {x}");
	local longestMacroLineLength = { defineLine, CURRENT_MACRO_LINES... }.each.length > ...;
	print(defineLine, " " * (longestMacroLineLength - #defineLine), " \\");
	for (local i, l: CURRENT_MACRO_LINES.enumerate()) {
		print("\t", l[4:]),;
		if (i == #CURRENT_MACRO_LINES - 1) {
			print;
		} else {
			print(" " * (longestMacroLineLength - #l)),;
			print(" \\");
		}
	}
}

function printSlabConfiguration(sizes: {int...}) {
	sizes = sizes.sorted();
	local maxWidth = #str(sizes.last);
	local cntWidth = #str(#sizes);
	print("#define Dee_SLAB_MINSIZE  ", str(sizes.first).ljust(maxWidth), " /" "* Size of the smallest slab *" "/");
	print("#define Dee_SLAB_MAXSIZE  ", str(sizes.last).ljust(maxWidth),  " /" "* Size of the largest slab *" "/");
	print("#define Dee_SLAB_COUNT    ", str(#sizes).ljust(maxWidth),      " /" "* # of different slabs *" "/");
	beginMacro();
	for (local i, s: sizes.enumerate()) {
		printMacro("func(", i, ", ", " " * (cntWidth - #str(i)), s, ")", " " * (maxWidth - #str(s)),
		           " /" "* ", s * 4, " / ", s * 8, " *" "/");
	}
	endMacro("DeeSlab_ENUMERATE(func)");
	beginMacro();
	local isFirst = true;
	for (local s: sizes) {
		local pad = " " * (maxWidth - #str(s));
		printMacro(isFirst ? "(" : " ",
		           "(size) <= ", s, pad, " * __SIZEOF_POINTER__ ? func##", s, " args :");
		isFirst = false;
	}
	printMacro("                                   ", " " * maxWidth, "fallback)");
	endMacro("DeeSlab_InvokeDyn(func, size, args, fallback)");
	print("#ifndef __NO_builtin_choose_expr");
	beginMacro();
	for (local s: sizes) {
		local pad = " " * (maxWidth - #str(s));
		printMacro("__builtin_choose_expr((size) <= ", s, pad, " * __SIZEOF_POINTER__, func##", s, " args,");
	}
	printMacro("                                                       ", " " * maxWidth, "fallback", ")" * #sizes);
	endMacro("DeeSlab_Invoke(func, size, args, fallback)");
	print("#else /" "* !__NO_builtin_choose_expr *" "/");
	print("#define DeeSlab_Invoke DeeSlab_InvokeDyn");
	print("#endif /" "* __NO_builtin_choose_expr *" "/");
}

if (SLAB_CONFIGURATIONS) {
	local isFirst = true;
	for (local ppOpt, config: SLAB_CONFIGURATIONS) {
		print("#", isFirst ? "" : "el", "if ", ppOpt);
		printSlabConfiguration(config);
		isFirst = false;
	}
	print("#else /" "* ... *" "/");
	printSlabConfiguration(DEFAULT_SLAB_CONFIGURATION);
	print("#endif /" "* !... *" "/");
} else {
	printSlabConfiguration(DEFAULT_SLAB_CONFIGURATION);
}
print;

local usedSizes = HashSet(DEFAULT_SLAB_CONFIGURATION);
for (local none, config: SLAB_CONFIGURATIONS)
	usedSizes.update(config);
for (local n: usedSizes.sorted()) {
	print("#if DeeSlab_HasSize(", n, ")");
	print("#define DeeObject_SlabMalloc", n, "       DeeSlab_Malloc", n, "");
	print("#define DeeObject_SlabCalloc", n, "       DeeSlab_Calloc", n, "");
	print("#define DeeObject_SlabTryMalloc", n, "    DeeSlab_TryMalloc", n, "");
	print("#define DeeObject_SlabTryCalloc", n, "    DeeSlab_TryCalloc", n, "");
	print("#define DeeObject_SlabFree", n, "         DeeSlab_Free", n, "");
	print("#define DeeDbgObject_SlabMalloc", n, "    DeeDbgSlab_Malloc", n, "");
	print("#define DeeDbgObject_SlabCalloc", n, "    DeeDbgSlab_Calloc", n, "");
	print("#define DeeDbgObject_SlabTryMalloc", n, " DeeDbgSlab_TryMalloc", n, "");
	print("#define DeeDbgObject_SlabTryCalloc", n, " DeeDbgSlab_TryCalloc", n, "");
	print("#define DeeDbgObject_SlabFree", n, "      DeeDbgSlab_Free", n, "");
	print("#ifndef NDEBUG");
	print("#define DeeSlab_Malloc", n, "()    DeeDbgSlab_Malloc", n, "(__FILE__, __LINE__)");
	print("#define DeeSlab_Calloc", n, "()    DeeDbgSlab_Calloc", n, "(__FILE__, __LINE__)");
	print("#define DeeSlab_TryMalloc", n, "() DeeDbgSlab_TryMalloc", n, "(__FILE__, __LINE__)");
	print("#define DeeSlab_TryCalloc", n, "() DeeDbgSlab_TryCalloc", n, "(__FILE__, __LINE__)");
	print("#define DeeSlab_Free", n, "(ptr)   DeeDbgSlab_Free", n, "(ptr, __FILE__, __LINE__)");
	print("#else /" "* !NDEBUG *" "/");
	print("#define DeeDbgSlab_Malloc", n, "(file, line)    DeeSlab_Malloc", n, "()");
	print("#define DeeDbgSlab_Calloc", n, "(file, line)    DeeSlab_Calloc", n, "()");
	print("#define DeeDbgSlab_TryMalloc", n, "(file, line) DeeSlab_TryMalloc", n, "()");
	print("#define DeeDbgSlab_TryCalloc", n, "(file, line) DeeSlab_TryCalloc", n, "()");
	print("#define DeeDbgSlab_Free", n, "(ptr, file, line) DeeSlab_Free", n, "(ptr)");
	print("#endif /" "* NDEBUG *" "/");
	print("#endif /" "* DeeSlab_HasSize(", n, ") *" "/");
}
]]]*/
#define Dee_SLAB_MINSIZE  4  /* Size of the smallest slab */
#define Dee_SLAB_MAXSIZE  10 /* Size of the largest slab */
#define Dee_SLAB_COUNT    5  /* # of different slabs */
#define DeeSlab_ENUMERATE(func) \
	func(0, 4)  /* 16 / 32 */   \
	func(1, 5)  /* 20 / 40 */   \
	func(2, 6)  /* 24 / 48 */   \
	func(3, 8)  /* 32 / 64 */   \
	func(4, 10) /* 40 / 80 */
#define DeeSlab_InvokeDyn(func, size, args, fallback)    \
	((size) <= 4  * __SIZEOF_POINTER__ ? func##4 args :  \
	 (size) <= 5  * __SIZEOF_POINTER__ ? func##5 args :  \
	 (size) <= 6  * __SIZEOF_POINTER__ ? func##6 args :  \
	 (size) <= 8  * __SIZEOF_POINTER__ ? func##8 args :  \
	 (size) <= 10 * __SIZEOF_POINTER__ ? func##10 args : \
	                                     fallback)
#ifndef __NO_builtin_choose_expr
#define DeeSlab_Invoke(func, size, args, fallback)                          \
	__builtin_choose_expr((size) <= 4  * __SIZEOF_POINTER__, func##4 args,  \
	__builtin_choose_expr((size) <= 5  * __SIZEOF_POINTER__, func##5 args,  \
	__builtin_choose_expr((size) <= 6  * __SIZEOF_POINTER__, func##6 args,  \
	__builtin_choose_expr((size) <= 8  * __SIZEOF_POINTER__, func##8 args,  \
	__builtin_choose_expr((size) <= 10 * __SIZEOF_POINTER__, func##10 args, \
	                                                         fallback)))))
#else /* !__NO_builtin_choose_expr */
#define DeeSlab_Invoke DeeSlab_InvokeDyn
#endif /* __NO_builtin_choose_expr */

#if DeeSlab_HasSize(4)
#define DeeObject_SlabMalloc4       DeeSlab_Malloc4
#define DeeObject_SlabCalloc4       DeeSlab_Calloc4
#define DeeObject_SlabTryMalloc4    DeeSlab_TryMalloc4
#define DeeObject_SlabTryCalloc4    DeeSlab_TryCalloc4
#define DeeObject_SlabFree4         DeeSlab_Free4
#define DeeDbgObject_SlabMalloc4    DeeDbgSlab_Malloc4
#define DeeDbgObject_SlabCalloc4    DeeDbgSlab_Calloc4
#define DeeDbgObject_SlabTryMalloc4 DeeDbgSlab_TryMalloc4
#define DeeDbgObject_SlabTryCalloc4 DeeDbgSlab_TryCalloc4
#define DeeDbgObject_SlabFree4      DeeDbgSlab_Free4
#ifndef NDEBUG
#define DeeSlab_Malloc4()    DeeDbgSlab_Malloc4(__FILE__, __LINE__)
#define DeeSlab_Calloc4()    DeeDbgSlab_Calloc4(__FILE__, __LINE__)
#define DeeSlab_TryMalloc4() DeeDbgSlab_TryMalloc4(__FILE__, __LINE__)
#define DeeSlab_TryCalloc4() DeeDbgSlab_TryCalloc4(__FILE__, __LINE__)
#define DeeSlab_Free4(ptr)   DeeDbgSlab_Free4(ptr, __FILE__, __LINE__)
#else /* !NDEBUG */
#define DeeDbgSlab_Malloc4(file, line)    DeeSlab_Malloc4()
#define DeeDbgSlab_Calloc4(file, line)    DeeSlab_Calloc4()
#define DeeDbgSlab_TryMalloc4(file, line) DeeSlab_TryMalloc4()
#define DeeDbgSlab_TryCalloc4(file, line) DeeSlab_TryCalloc4()
#define DeeDbgSlab_Free4(ptr, file, line) DeeSlab_Free4(ptr)
#endif /* NDEBUG */
#endif /* DeeSlab_HasSize(4) */
#if DeeSlab_HasSize(5)
#define DeeObject_SlabMalloc5       DeeSlab_Malloc5
#define DeeObject_SlabCalloc5       DeeSlab_Calloc5
#define DeeObject_SlabTryMalloc5    DeeSlab_TryMalloc5
#define DeeObject_SlabTryCalloc5    DeeSlab_TryCalloc5
#define DeeObject_SlabFree5         DeeSlab_Free5
#define DeeDbgObject_SlabMalloc5    DeeDbgSlab_Malloc5
#define DeeDbgObject_SlabCalloc5    DeeDbgSlab_Calloc5
#define DeeDbgObject_SlabTryMalloc5 DeeDbgSlab_TryMalloc5
#define DeeDbgObject_SlabTryCalloc5 DeeDbgSlab_TryCalloc5
#define DeeDbgObject_SlabFree5      DeeDbgSlab_Free5
#ifndef NDEBUG
#define DeeSlab_Malloc5()    DeeDbgSlab_Malloc5(__FILE__, __LINE__)
#define DeeSlab_Calloc5()    DeeDbgSlab_Calloc5(__FILE__, __LINE__)
#define DeeSlab_TryMalloc5() DeeDbgSlab_TryMalloc5(__FILE__, __LINE__)
#define DeeSlab_TryCalloc5() DeeDbgSlab_TryCalloc5(__FILE__, __LINE__)
#define DeeSlab_Free5(ptr)   DeeDbgSlab_Free5(ptr, __FILE__, __LINE__)
#else /* !NDEBUG */
#define DeeDbgSlab_Malloc5(file, line)    DeeSlab_Malloc5()
#define DeeDbgSlab_Calloc5(file, line)    DeeSlab_Calloc5()
#define DeeDbgSlab_TryMalloc5(file, line) DeeSlab_TryMalloc5()
#define DeeDbgSlab_TryCalloc5(file, line) DeeSlab_TryCalloc5()
#define DeeDbgSlab_Free5(ptr, file, line) DeeSlab_Free5(ptr)
#endif /* NDEBUG */
#endif /* DeeSlab_HasSize(5) */
#if DeeSlab_HasSize(6)
#define DeeObject_SlabMalloc6       DeeSlab_Malloc6
#define DeeObject_SlabCalloc6       DeeSlab_Calloc6
#define DeeObject_SlabTryMalloc6    DeeSlab_TryMalloc6
#define DeeObject_SlabTryCalloc6    DeeSlab_TryCalloc6
#define DeeObject_SlabFree6         DeeSlab_Free6
#define DeeDbgObject_SlabMalloc6    DeeDbgSlab_Malloc6
#define DeeDbgObject_SlabCalloc6    DeeDbgSlab_Calloc6
#define DeeDbgObject_SlabTryMalloc6 DeeDbgSlab_TryMalloc6
#define DeeDbgObject_SlabTryCalloc6 DeeDbgSlab_TryCalloc6
#define DeeDbgObject_SlabFree6      DeeDbgSlab_Free6
#ifndef NDEBUG
#define DeeSlab_Malloc6()    DeeDbgSlab_Malloc6(__FILE__, __LINE__)
#define DeeSlab_Calloc6()    DeeDbgSlab_Calloc6(__FILE__, __LINE__)
#define DeeSlab_TryMalloc6() DeeDbgSlab_TryMalloc6(__FILE__, __LINE__)
#define DeeSlab_TryCalloc6() DeeDbgSlab_TryCalloc6(__FILE__, __LINE__)
#define DeeSlab_Free6(ptr)   DeeDbgSlab_Free6(ptr, __FILE__, __LINE__)
#else /* !NDEBUG */
#define DeeDbgSlab_Malloc6(file, line)    DeeSlab_Malloc6()
#define DeeDbgSlab_Calloc6(file, line)    DeeSlab_Calloc6()
#define DeeDbgSlab_TryMalloc6(file, line) DeeSlab_TryMalloc6()
#define DeeDbgSlab_TryCalloc6(file, line) DeeSlab_TryCalloc6()
#define DeeDbgSlab_Free6(ptr, file, line) DeeSlab_Free6(ptr)
#endif /* NDEBUG */
#endif /* DeeSlab_HasSize(6) */
#if DeeSlab_HasSize(8)
#define DeeObject_SlabMalloc8       DeeSlab_Malloc8
#define DeeObject_SlabCalloc8       DeeSlab_Calloc8
#define DeeObject_SlabTryMalloc8    DeeSlab_TryMalloc8
#define DeeObject_SlabTryCalloc8    DeeSlab_TryCalloc8
#define DeeObject_SlabFree8         DeeSlab_Free8
#define DeeDbgObject_SlabMalloc8    DeeDbgSlab_Malloc8
#define DeeDbgObject_SlabCalloc8    DeeDbgSlab_Calloc8
#define DeeDbgObject_SlabTryMalloc8 DeeDbgSlab_TryMalloc8
#define DeeDbgObject_SlabTryCalloc8 DeeDbgSlab_TryCalloc8
#define DeeDbgObject_SlabFree8      DeeDbgSlab_Free8
#ifndef NDEBUG
#define DeeSlab_Malloc8()    DeeDbgSlab_Malloc8(__FILE__, __LINE__)
#define DeeSlab_Calloc8()    DeeDbgSlab_Calloc8(__FILE__, __LINE__)
#define DeeSlab_TryMalloc8() DeeDbgSlab_TryMalloc8(__FILE__, __LINE__)
#define DeeSlab_TryCalloc8() DeeDbgSlab_TryCalloc8(__FILE__, __LINE__)
#define DeeSlab_Free8(ptr)   DeeDbgSlab_Free8(ptr, __FILE__, __LINE__)
#else /* !NDEBUG */
#define DeeDbgSlab_Malloc8(file, line)    DeeSlab_Malloc8()
#define DeeDbgSlab_Calloc8(file, line)    DeeSlab_Calloc8()
#define DeeDbgSlab_TryMalloc8(file, line) DeeSlab_TryMalloc8()
#define DeeDbgSlab_TryCalloc8(file, line) DeeSlab_TryCalloc8()
#define DeeDbgSlab_Free8(ptr, file, line) DeeSlab_Free8(ptr)
#endif /* NDEBUG */
#endif /* DeeSlab_HasSize(8) */
#if DeeSlab_HasSize(10)
#define DeeObject_SlabMalloc10       DeeSlab_Malloc10
#define DeeObject_SlabCalloc10       DeeSlab_Calloc10
#define DeeObject_SlabTryMalloc10    DeeSlab_TryMalloc10
#define DeeObject_SlabTryCalloc10    DeeSlab_TryCalloc10
#define DeeObject_SlabFree10         DeeSlab_Free10
#define DeeDbgObject_SlabMalloc10    DeeDbgSlab_Malloc10
#define DeeDbgObject_SlabCalloc10    DeeDbgSlab_Calloc10
#define DeeDbgObject_SlabTryMalloc10 DeeDbgSlab_TryMalloc10
#define DeeDbgObject_SlabTryCalloc10 DeeDbgSlab_TryCalloc10
#define DeeDbgObject_SlabFree10      DeeDbgSlab_Free10
#ifndef NDEBUG
#define DeeSlab_Malloc10()    DeeDbgSlab_Malloc10(__FILE__, __LINE__)
#define DeeSlab_Calloc10()    DeeDbgSlab_Calloc10(__FILE__, __LINE__)
#define DeeSlab_TryMalloc10() DeeDbgSlab_TryMalloc10(__FILE__, __LINE__)
#define DeeSlab_TryCalloc10() DeeDbgSlab_TryCalloc10(__FILE__, __LINE__)
#define DeeSlab_Free10(ptr)   DeeDbgSlab_Free10(ptr, __FILE__, __LINE__)
#else /* !NDEBUG */
#define DeeDbgSlab_Malloc10(file, line)    DeeSlab_Malloc10()
#define DeeDbgSlab_Calloc10(file, line)    DeeSlab_Calloc10()
#define DeeDbgSlab_TryMalloc10(file, line) DeeSlab_TryMalloc10()
#define DeeDbgSlab_TryCalloc10(file, line) DeeSlab_TryCalloc10()
#define DeeDbgSlab_Free10(ptr, file, line) DeeSlab_Free10(ptr)
#endif /* NDEBUG */
#endif /* DeeSlab_HasSize(10) */
/*[[[end]]]*/


#ifdef __CC__
#define _Dee_PRIVATE_DEFINE_SLAB_FUNCTIONS(index, size)                                                        \
	DFUNDEF ATTR_MALLOC WUNUSED void *(DCALL DeeSlab_Malloc##size)(void);                                     \
	DFUNDEF ATTR_MALLOC WUNUSED void *(DCALL DeeSlab_Calloc##size)(void);                                     \
	DFUNDEF ATTR_MALLOC WUNUSED void *(DCALL DeeSlab_TryMalloc##size)(void);                                  \
	DFUNDEF ATTR_MALLOC WUNUSED void *(DCALL DeeSlab_TryCalloc##size)(void);                                  \
	DFUNDEF ATTR_MALLOC WUNUSED void *(DCALL DeeDbgSlab_Malloc##size)(char const *file, int line);            \
	DFUNDEF ATTR_MALLOC WUNUSED void *(DCALL DeeDbgSlab_Calloc##size)(char const *file, int line);            \
	DFUNDEF ATTR_MALLOC WUNUSED void *(DCALL DeeDbgSlab_TryMalloc##size)(char const *file, int line);         \
	DFUNDEF ATTR_MALLOC WUNUSED void *(DCALL DeeDbgSlab_TryCalloc##size)(char const *file, int line);         \
	DFUNDEF ATTR_MALLOC WUNUSED void *(DCALL DeeGCObject_SlabMalloc##size)(void);                             \
	DFUNDEF ATTR_MALLOC WUNUSED void *(DCALL DeeGCObject_SlabCalloc##size)(void);                             \
	DFUNDEF ATTR_MALLOC WUNUSED void *(DCALL DeeGCObject_SlabTryMalloc##size)(void);                          \
	DFUNDEF ATTR_MALLOC WUNUSED void *(DCALL DeeGCObject_SlabTryCalloc##size)(void);                          \
	DFUNDEF ATTR_MALLOC WUNUSED void *(DCALL DeeDbgGCObject_SlabMalloc##size)(char const *file, int line);    \
	DFUNDEF ATTR_MALLOC WUNUSED void *(DCALL DeeDbgGCObject_SlabCalloc##size)(char const *file, int line);    \
	DFUNDEF ATTR_MALLOC WUNUSED void *(DCALL DeeDbgGCObject_SlabTryMalloc##size)(char const *file, int line); \
	DFUNDEF ATTR_MALLOC WUNUSED void *(DCALL DeeDbgGCObject_SlabTryCalloc##size)(char const *file, int line); \
	DFUNDEF void (DCALL DeeSlab_Free##size)(void *ptr);                                                       \
	DFUNDEF void (DCALL DeeDbgSlab_Free##size)(void *ptr, char const *file, int line);                        \
	DFUNDEF void (DCALL DeeGCObject_SlabFree##size)(void *__restrict ptr);                                    \
	DFUNDEF void (DCALL DeeDbgGCObject_SlabFree##size)(void *__restrict ptr, char const *file, int line);
DeeSlab_ENUMERATE(_Dee_PRIVATE_DEFINE_SLAB_FUNCTIONS)
#undef _Dee_PRIVATE_DEFINE_SLAB_FUNCTIONS


/* Allocate fixed-size, general-purpose slab memory.
 * NOTE: This memory must be freed by one of:
 *   - DeeSlab_FFree(return, size2) | size2 <= size
 *   - DeeSlab_Free<size2>(return)  | size2 <= size
 *   - DeeSlab_Free(return) */
#define DeeSlab_Malloc(size)                     DeeSlab_Invoke(DeeSlab_Malloc, size, (), Dee_Malloc(size))
#define DeeSlab_Calloc(size)                     DeeSlab_Invoke(DeeSlab_Calloc, size, (), Dee_Calloc(size))
#define DeeSlab_TryMalloc(size)                  DeeSlab_Invoke(DeeSlab_TryMalloc, size, (), Dee_TryMalloc(size))
#define DeeSlab_TryCalloc(size)                  DeeSlab_Invoke(DeeSlab_TryCalloc, size, (), Dee_TryCalloc(size))
#define DeeSlab_FFree(ptr, size)                 DeeSlab_Invoke(DeeSlab_Free, size, (ptr), Dee_Free(ptr))
#define DeeSlab_XFFree(ptr, size)                ((ptr) ? DeeSlab_FFree(ptr, size) : (void)0)
#define DeeDbgSlab_Malloc(size, file, line)      DeeSlab_Invoke(DeeDbgSlab_Malloc, size, (file, line), DeeDbg_Malloc(size, file, line))
#define DeeDbgSlab_Calloc(size, file, line)      DeeSlab_Invoke(DeeDbgSlab_Calloc, size, (file, line), DeeDbg_Calloc(size, file, line))
#define DeeDbgSlab_TryMalloc(size, file, line)   DeeSlab_Invoke(DeeDbgSlab_TryMalloc, size, (file, line), DeeDbg_TryMalloc(size, file, line))
#define DeeDbgSlab_TryCalloc(size, file, line)   DeeSlab_Invoke(DeeDbgSlab_TryCalloc, size, (file, line), DeeDbg_TryCalloc(size, file, line))
#define DeeDbgSlab_FFree(ptr, size, file, line)  DeeSlab_Invoke(DeeDbgSlab_Free, size, (ptr, file, line), DeeDbg_Free(ptr, file, line))
#define DeeDbgSlab_XFFree(ptr, size, file, line) ((ptr) ? DeeDbgSlab_FFree(ptr, size, file, line) : (void)0)

#define DeeSlab_Mallocc(elem_count, elem_size)                     DeeSlab_Malloc(_Dee_MalloccBufsize(elem_count, elem_size))
#define DeeSlab_Callocc(elem_count, elem_size)                     DeeSlab_Calloc(_Dee_MalloccBufsize(elem_count, elem_size))
#define DeeSlab_TryMallocc(elem_count, elem_size)                  DeeSlab_TryMalloc(_Dee_MalloccBufsize(elem_count, elem_size))
#define DeeSlab_TryCallocc(elem_count, elem_size)                  DeeSlab_TryCalloc(_Dee_MalloccBufsize(elem_count, elem_size))
#define DeeSlab_FFreec(ptr, elem_count, elem_size)                 DeeSlab_FFree(ptr, _Dee_MalloccBufsize(elem_count, elem_size))
#define DeeSlab_XFFreec(ptr, elem_count, elem_size)                DeeSlab_XFFree(ptr, _Dee_MalloccBufsize(elem_count, elem_size))
#define DeeDbgSlab_Mallocc(elem_count, elem_size, file, line)      DeeDbgSlab_Malloc(_Dee_MalloccBufsize(elem_count, elem_size), file, line)
#define DeeDbgSlab_Callocc(elem_count, elem_size, file, line)      DeeDbgSlab_Calloc(_Dee_MalloccBufsize(elem_count, elem_size), file, line)
#define DeeDbgSlab_TryMallocc(elem_count, elem_size, file, line)   DeeDbgSlab_TryMalloc(_Dee_MalloccBufsize(elem_count, elem_size), file, line)
#define DeeDbgSlab_TryCallocc(elem_count, elem_size, file, line)   DeeDbgSlab_TryCalloc(_Dee_MalloccBufsize(elem_count, elem_size), file, line)
#define DeeDbgSlab_FFreec(ptr, elem_count, elem_size, file, line)  DeeDbgSlab_FFree(ptr, _Dee_MalloccBufsize(elem_count, elem_size), file, line)
#define DeeDbgSlab_XFFreec(ptr, elem_count, elem_size, file, line) DeeDbgSlab_XFFree(ptr, _Dee_MalloccBufsize(elem_count, elem_size), file, line)
#endif /* __CC__ */

/* Free any kind of pointer allocated by the general-purpose slab allocators.
 * NOTE: Do not mix with object slab allocators, even if they may appear identical! */
#ifdef __INTELLISENSE__
DFUNDEF void (DCALL DeeSlab_Free)(void *ptr);
DFUNDEF void (DCALL DeeDbgSlab_Free)(void *ptr, char const *file, int line);
#elif Dee_SLAB_MINSIZE == 4
#define DeeSlab_Free      DeeSlab_Free4
#define DeeDbgSlab_Free   DeeDbgSlab_Free4
#else /* Dee_SLAB_MINSIZE == 4 */
#define _Dee_PRIVATE_DeeSlab_Free2(x) DeeSlab_Free##x
#define _Dee_PRIVATE_DeeSlab_Free(x) _Dee_PRIVATE_DeeSlab_Free2(x)
#define _Dee_PRIVATE_DeeDbgSlab_Free2(x) DeeDbgSlab_Free##x
#define _Dee_PRIVATE_DeeDbgSlab_Free(x) _Dee_PRIVATE_DeeDbgSlab_Free2(x)
#define DeeSlab_Free      _Dee_PRIVATE_DeeSlab_Free(Dee_SLAB_MINSIZE)
#define DeeDbgSlab_Free   _Dee_PRIVATE_DeeDbgSlab_Free(Dee_SLAB_MINSIZE)
#endif /* Dee_SLAB_MINSIZE != 4 */

#else /* !CONFIG_NO_OBJECT_SLABS */

/* Allocate fixed-size, general-purpose slab memory.
 * NOTE: This memory must be freed by one of:
 *   - DeeSlab_FFree(return, size2) | size2 <= size
 *   - DeeSlab_Free<size2>(return)  | size2 <= size
 *   - DeeSlab_Free(return) */
#define DeeSlab_Malloc(size)                     Dee_Malloc(size)
#define DeeSlab_Calloc(size)                     Dee_Calloc(size)
#define DeeSlab_TryMalloc(size)                  Dee_TryMalloc(size)
#define DeeSlab_TryCalloc(size)                  Dee_TryCalloc(size)
#define DeeSlab_FFree(ptr, size)                 Dee_Free(ptr)
#define DeeSlab_XFFree(ptr, size)                Dee_Free(ptr)
#define DeeDbgSlab_Malloc(size, file, line)      DeeDbg_Malloc(size, file, line)
#define DeeDbgSlab_Calloc(size, file, line)      DeeDbg_Calloc(size, file, line)
#define DeeDbgSlab_TryMalloc(size, file, line)   DeeDbg_TryMalloc(size, file, line)
#define DeeDbgSlab_TryCalloc(size, file, line)   DeeDbg_TryCalloc(size, file, line)
#define DeeDbgSlab_FFree(ptr, size, file, line)  DeeDbg_Free(ptr, file, line)
#define DeeDbgSlab_XFFree(ptr, size, file, line) DeeDbg_Free(ptr, file, line)

#define DeeSlab_Mallocc(elem_count, elem_size)                     Dee_Mallocc(elem_count, elem_size)
#define DeeSlab_Callocc(elem_count, elem_size)                     Dee_Callocc(elem_count, elem_size)
#define DeeSlab_TryMallocc(elem_count, elem_size)                  Dee_TryMallocc(elem_count, elem_size)
#define DeeSlab_TryCallocc(elem_count, elem_size)                  Dee_TryCallocc(elem_count, elem_size)
#define DeeSlab_FFreec(ptr, elem_count, elem_size)                 Dee_Free(ptr)
#define DeeSlab_XFFreec(ptr, elem_count, elem_size)                Dee_Free(ptr)
#define DeeDbgSlab_Mallocc(elem_count, elem_size, file, line)      DeeDbg_Mallocc(elem_count, elem_size, file, line)
#define DeeDbgSlab_Callocc(elem_count, elem_size, file, line)      DeeDbg_Callocc(elem_count, elem_size, file, line)
#define DeeDbgSlab_TryMallocc(elem_count, elem_size, file, line)   DeeDbg_TryMallocc(elem_count, elem_size, file, line)
#define DeeDbgSlab_TryCallocc(elem_count, elem_size, file, line)   DeeDbg_TryCallocc(elem_count, elem_size, file, line)
#define DeeDbgSlab_FFreec(ptr, elem_count, elem_size, file, line)  DeeDbg_Free(ptr, file, line)
#define DeeDbgSlab_XFFreec(ptr, elem_count, elem_size, file, line) DeeDbg_Free(ptr, file, line)

/* Free any kind of pointer allocated by the general-purpose slab allocators.
 * NOTE: Do not mix with object slab allocators, even if they may appear identical! */
#ifdef __INTELLISENSE__
DFUNDEF void (DCALL DeeSlab_Free)(void *ptr);
DFUNDEF void (DCALL DeeDbgSlab_Free)(void *ptr, char const *file, int line);
#else /* __INTELLISENSE__ */
#define DeeSlab_Free      Dee_Free
#define DeeDbgSlab_Free   DeeDbg_Free
#endif /* !__INTELLISENSE__ */

#endif /* CONFIG_NO_OBJECT_SLABS */



#ifdef __CC__
/* Slab memory allocators with automatic component size detection. */

typedef struct {
	uintptr_t si_slabstart;      /* [const] Slab starting address */
	uintptr_t si_slabend;        /* [const] Slab ending address */
	size_t    si_itemsize;       /* [const] Slab item size (in bytes) */
	size_t    si_items_per_page; /* [const] Number of items per page */
	size_t    si_totalpages;     /* [const][== si_usedpages + si_tailpages] Number of pages designated for this slab */
	size_t    si_totalitems;     /* [const][== si_totalpages * si_items_per_page] Max number of items which may be allocated by the slab */
	size_t    si_cur_alloc;      /* # of items (`si_itemsize'-sized data blocks) currently allocated */
	size_t    si_max_alloc;      /* Max # of items that were ever allocated */
	size_t    si_cur_free;       /* # of items in initialized pages currently marked as free */
	size_t    si_max_free;       /* Max # of items that were ever marked as free */
	size_t    si_cur_fullpages;  /* # of initialized pages that currently are fully in use */
	size_t    si_max_fullpages;  /* Max # of initialized pages that were ever in use at the same time */
	size_t    si_cur_freepages;  /* # of initialized pages containing unallocated items */
	size_t    si_max_freepages;  /* Max # of initialized pages containing unallocated items at any point int time */
	size_t    si_usedpages;      /* # of pages which are currently being used (si_cur_fullpages + si_cur_freepages) */
	size_t    si_tailpages;      /* # of pages which haven't been allocated, yet */
} DeeSlabInfo;

typedef struct {
	size_t      st_slabcount;      /* [const] Number of existing slabs. */
#ifdef Dee_SLAB_COUNT
	DeeSlabInfo st_slabs[Dee_SLAB_COUNT]; /* Slab-specific information */
#else /* Dee_SLAB_COUNT */
	DeeSlabInfo st_slabs[8];       /* Slab-specific information */
#endif /* !Dee_SLAB_COUNT */
} DeeSlabStat;


/* Collect slab information and write that information to `info'
 * When `bufsize' is smaller that the required buffer size to write
 * all known slab information, the contents of `info' are undefined,
 * and the required size is returned (which is then `> bufsize')
 * Otherwise, `info' is filled with slab statistic information, and
 * the used buffer size is returned (which is then `<= bufsize')
 * In no case will this function throw an error.
 * When deemon has been built with `CONFIG_NO_OBJECT_SLAB_STATS',
 * this function will be significantly slower, and all max-fields
 * are set to match the cur-fields. */
DFUNDEF NONNULL((1)) size_t DCALL
DeeSlab_Stat(DeeSlabStat *info, size_t bufsize);

/* Reset the slab max-statistics to the cur-values. */
DFUNDEF void DCALL DeeSlab_ResetStat(void);


/* Allocate fixed-size, object-purposed slab memory.
 * NOTE: This memory must be freed by one of:
 *   - DeeObject_FFree(return, size2)    | size2 <= size
 *   - DeeObject_SlabFree<size2>(return) | size2 <= size
 *   - DeeObject_Free(return) */
#ifdef CONFIG_NO_OBJECT_SLABS
#define DeeObject_FMalloc(size)                   DeeObject_Malloc(size)
#define DeeObject_FCalloc(size)                   DeeObject_Calloc(size)
#define DeeObject_FTryMalloc(size)                DeeObject_TryMalloc(size)
#define DeeObject_FTryCalloc(size)                DeeObject_TryCalloc(size)
#define DeeObject_FFree(ptr, size)                DeeObject_Free(ptr)
#define DeeDbgObject_FMalloc(size, file, line)    DeeDbgObject_Malloc(size, file, line)
#define DeeDbgObject_FCalloc(size, file, line)    DeeDbgObject_Calloc(size, file, line)
#define DeeDbgObject_FTryMalloc(size, file, line) DeeDbgObject_TryMalloc(size, file, line)
#define DeeDbgObject_FTryCalloc(size, file, line) DeeDbgObject_TryCalloc(size, file, line)
#define DeeDbgObject_FFree(ptr, size, file, line) DeeDbgObject_Free(ptr, file, line)
#else /* CONFIG_NO_OBJECT_SLABS */
#define DeeObject_FMalloc(size)                   DeeSlab_Invoke(DeeObject_SlabMalloc, size, (), DeeObject_Malloc(size))
#define DeeObject_FCalloc(size)                   DeeSlab_Invoke(DeeObject_SlabCalloc, size, (), DeeObject_Calloc(size))
#define DeeObject_FTryMalloc(size)                DeeSlab_Invoke(DeeObject_SlabTryMalloc, size, (), DeeObject_TryMalloc(size))
#define DeeObject_FTryCalloc(size)                DeeSlab_Invoke(DeeObject_SlabTryCalloc, size, (), DeeObject_TryCalloc(size))
#define DeeObject_FFree(ptr, size)                DeeSlab_Invoke(DeeObject_SlabFree, size, (ptr), DeeObject_Free(ptr))
#define DeeDbgObject_FMalloc(size, file, line)    DeeSlab_Invoke(DeeDbgObject_SlabMalloc, size, (file, line), DeeDbgObject_Malloc(size, file, line))
#define DeeDbgObject_FCalloc(size, file, line)    DeeSlab_Invoke(DeeDbgObject_SlabCalloc, size, (file, line), DeeDbgObject_Calloc(size, file, line))
#define DeeDbgObject_FTryMalloc(size, file, line) DeeSlab_Invoke(DeeDbgObject_SlabTryMalloc, size, (file, line), DeeDbgObject_TryMalloc(size, file, line))
#define DeeDbgObject_FTryCalloc(size, file, line) DeeSlab_Invoke(DeeDbgObject_SlabTryCalloc, size, (file, line), DeeDbgObject_TryCalloc(size, file, line))
#define DeeDbgObject_FFree(ptr, size, file, line) DeeSlab_Invoke(DeeDbgObject_SlabFree, size, (ptr, file, line), DeeDbgObject_Free(ptr, file, line))
#endif /* !CONFIG_NO_OBJECT_SLABS */
#define DeeObject_FUntrackAlloc(ptr, size)                DeeObject_UntrackAlloc(ptr)
#define DeeDbgObject_FUntrackAlloc(ptr, size, file, line) DeeDbgObject_UntrackAlloc(ptr, file, line)

#endif /* __CC__ */
#endif /* !CONFIG_EXPERIMENTAL_REWORKED_SLAB_ALLOCATOR */

/* Same as the regular malloc functions, but use the same allocation methods that would
 * be used by `Dee_TYPE_CONSTRUCTOR_INIT_FIXED' and `Dee_TYPE_CONSTRUCTOR_INIT_FIXED_S',
 * meaning that pointers returned by these macros have binary compatibility with them. */
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

/* Helpers for type-safe allocation of variable-length objects:
 * >> typedef struct {
 * >>     ...
 * >> } MyObjectItem;
 * >> typedef struct {
 * >>     OBJECT_HEAD
 * >>     size_t                                mo_count;
 * >>     COMPILER_FLEXIBLE_ARRAY(MyObjectItem, mo_items);
 * >> } MyObject;
 * >> 
 * >> #define MyObject_Alloc(n) DeeObject_MALLOCC(MyObject, mo_items, n)
 * >> #define MyObject_Free(p)  DeeObject_Free(p) */
#define DeeObject_MALLOCC(T, m, c)             ((T *)DeeObject_Mallocc(__builtin_offsetof(T, m), c, sizeof(*(((T *)0)->m))))
#define DeeObject_CALLOCC(T, m, c)             ((T *)DeeObject_Callocc(__builtin_offsetof(T, m), c, sizeof(*(((T *)0)->m))))
#define DeeObject_REALLOCC(p, T, m, c)         ((T *)DeeObject_Reallocc(Dee_REQUIRES_TYPE(T *, p), __builtin_offsetof(T, m), c, sizeof(*(((T *)0)->m))))
#define DeeObject_TRYMALLOCC(T, m, c)          ((T *)DeeObject_TryMallocc(__builtin_offsetof(T, m), c, sizeof(*(((T *)0)->m))))
#define DeeObject_TRYCALLOCC(T, m, c)          ((T *)DeeObject_TryCallocc(__builtin_offsetof(T, m), c, sizeof(*(((T *)0)->m))))
#define DeeObject_TRYREALLOCC(p, T, m, c)      ((T *)DeeObject_TryReallocc(Dee_REQUIRES_TYPE(T *, p), __builtin_offsetof(T, m), c, sizeof(*(((T *)0)->m))))
#define DeeObject_MALLOCC_SAFE(T, m, c)        ((T *)DeeObject_MalloccSafe(__builtin_offsetof(T, m), c, sizeof(*(((T *)0)->m))))
#define DeeObject_CALLOCC_SAFE(T, m, c)        ((T *)DeeObject_CalloccSafe(__builtin_offsetof(T, m), c, sizeof(*(((T *)0)->m))))
#define DeeObject_REALLOCC_SAFE(p, T, m, c)    ((T *)DeeObject_RealloccSafe(Dee_REQUIRES_TYPE(T *, p), __builtin_offsetof(T, m), c, sizeof(*(((T *)0)->m))))
#define DeeObject_TRYMALLOCC_SAFE(T, m, c)     ((T *)DeeObject_TryMalloccSafe(__builtin_offsetof(T, m), c, sizeof(*(((T *)0)->m))))
#define DeeObject_TRYCALLOCC_SAFE(T, m, c)     ((T *)DeeObject_TryCalloccSafe(__builtin_offsetof(T, m), c, sizeof(*(((T *)0)->m))))
#define DeeObject_TRYREALLOCC_SAFE(p, T, m, c) ((T *)DeeObject_TryRealloccSafe(Dee_REQUIRES_TYPE(T *, p), __builtin_offsetof(T, m), c, sizeof(*(((T *)0)->m))))



#ifdef __CC__
#ifdef GUARD_DEEMON_TYPE_H
/* Specifies an allocator that may provides optimizations
 * for types with a FIXED size (which most objects have). */
#ifdef CONFIG_EXPERIMENTAL_REWORKED_SLAB_ALLOCATOR
#define Dee_TYPE_CONSTRUCTOR_INIT_SIZED(tp_instance_size, tp_ctor, tp_copy_ctor, tp_any_ctor, tp_any_ctor_kw, tp_serialize)            \
	Dee_TYPE_CONSTRUCTOR_INIT_ALLOC(tp_ctor, tp_copy_ctor, tp_any_ctor, tp_any_ctor_kw, tp_serialize,                                  \
	                                DeeSlab_GetMalloc(tp_instance_size, (void *(DCALL *)(void))(void *)(uintptr_t)(tp_instance_size)), \
	                                DeeSlab_GetFree(tp_instance_size, (void (DCALL *)(void *))(Dee_funptr_t)NULL))
#define Dee_TYPE_CONSTRUCTOR_INIT_FIXED(T, tp_ctor, tp_copy_ctor, tp_any_ctor, tp_any_ctor_kw, tp_serialize) \
	Dee_TYPE_CONSTRUCTOR_INIT_SIZED(sizeof(T), tp_ctor, tp_copy_ctor, tp_any_ctor, tp_any_ctor_kw, tp_serialize)
#else /* CONFIG_EXPERIMENTAL_REWORKED_SLAB_ALLOCATOR */
#ifdef CONFIG_NO_OBJECT_SLABS
#define Dee_TYPE_CONSTRUCTOR_INIT_SIZED_R    Dee_TYPE_CONSTRUCTOR_INIT_ALLOC_AUTOSIZED_R /*!export(include("type.h"))*/
#define Dee_TYPE_CONSTRUCTOR_INIT_SIZED_GC_R Dee_TYPE_CONSTRUCTOR_INIT_ALLOC_AUTOSIZED_R /*!export(include("type.h"))*/
#define Dee_TYPE_CONSTRUCTOR_INIT_SIZED      Dee_TYPE_CONSTRUCTOR_INIT_ALLOC_AUTOSIZED   /*!export(include("type.h"))*/
#define Dee_TYPE_CONSTRUCTOR_INIT_SIZED_GC   Dee_TYPE_CONSTRUCTOR_INIT_ALLOC_AUTOSIZED   /*!export(include("type.h"))*/
#define Dee_TYPE_CONSTRUCTOR_INIT_FIXED      Dee_TYPE_CONSTRUCTOR_INIT_ALLOC_AUTO        /*!export(include("type.h"))*/
#define Dee_TYPE_CONSTRUCTOR_INIT_FIXED_GC   Dee_TYPE_CONSTRUCTOR_INIT_ALLOC_AUTO        /*!export(include("type.h"))*/
#else /* CONFIG_NO_OBJECT_SLABS */
#define Dee_TYPE_CONSTRUCTOR_INIT_SIZED_R(min_tp_instance_size, max_tp_instance_size, tp_ctor, tp_copy_ctor, tp_any_ctor, tp_any_ctor_kw, tp_serialize)              \
	Dee_TYPE_CONSTRUCTOR_INIT_ALLOC(tp_ctor, tp_copy_ctor, tp_any_ctor, tp_any_ctor_kw, tp_serialize,                                                                \
	                                DeeSlab_Invoke(&DeeObject_SlabMalloc, max_tp_instance_size, , (void *(DCALL *)(void))(void *)(uintptr_t)(max_tp_instance_size)), \
	                                DeeSlab_Invoke(&DeeObject_SlabFree, min_tp_instance_size, , (void (DCALL *)(void *))(Dee_funptr_t)NULL))
#define Dee_TYPE_CONSTRUCTOR_INIT_SIZED_GC_R(min_tp_instance_size, max_tp_instance_size, tp_ctor, tp_copy_ctor, tp_any_ctor, tp_any_ctor_kw, tp_serialize)             \
	Dee_TYPE_CONSTRUCTOR_INIT_ALLOC(tp_ctor, tp_copy_ctor, tp_any_ctor, tp_any_ctor_kw, tp_serialize,                                                                  \
	                                DeeSlab_Invoke(&DeeGCObject_SlabMalloc, max_tp_instance_size, , (void *(DCALL *)(void))(void *)(uintptr_t)(max_tp_instance_size)), \
	                                DeeSlab_Invoke(&DeeGCObject_SlabFree, min_tp_instance_size, , (void (DCALL *)(void *))(Dee_funptr_t)NULL))
#define Dee_TYPE_CONSTRUCTOR_INIT_SIZED(tp_instance_size, tp_ctor, tp_copy_ctor, tp_any_ctor, tp_any_ctor_kw, tp_serialize) \
	Dee_TYPE_CONSTRUCTOR_INIT_SIZED_R(tp_instance_size, tp_instance_size, tp_ctor, tp_copy_ctor, tp_any_ctor, tp_any_ctor_kw, tp_serialize)
#define Dee_TYPE_CONSTRUCTOR_INIT_SIZED_GC(tp_instance_size, tp_ctor, tp_copy_ctor, tp_any_ctor, tp_any_ctor_kw, tp_serialize) \
	Dee_TYPE_CONSTRUCTOR_INIT_SIZED_GC_R(tp_instance_size, tp_instance_size, tp_ctor, tp_copy_ctor, tp_any_ctor, tp_any_ctor_kw, tp_serialize)
#define Dee_TYPE_CONSTRUCTOR_INIT_FIXED(T, tp_ctor, tp_copy_ctor, tp_any_ctor, tp_any_ctor_kw, tp_serialize) \
	Dee_TYPE_CONSTRUCTOR_INIT_SIZED(sizeof(T), tp_ctor, tp_copy_ctor, tp_any_ctor, tp_any_ctor_kw, tp_serialize)
#define Dee_TYPE_CONSTRUCTOR_INIT_FIXED_GC(T, tp_ctor, tp_copy_ctor, tp_any_ctor, tp_any_ctor_kw, tp_serialize) \
	Dee_TYPE_CONSTRUCTOR_INIT_SIZED_GC(sizeof(T), tp_ctor, tp_copy_ctor, tp_any_ctor, tp_any_ctor_kw, tp_serialize)
#endif /* !CONFIG_NO_OBJECT_SLABS */
#endif /* !CONFIG_EXPERIMENTAL_REWORKED_SLAB_ALLOCATOR */

/* Same as `Dee_TYPE_CONSTRUCTOR_INIT_FIXED()', but don't link against
 * dedicated allocator functions when doing so would require the creation
 * of relocations that might cause loading times to become larger. */
#ifdef CONFIG_FIXED_ALLOCATOR_S_IS_AUTO
#define Dee_TYPE_CONSTRUCTOR_INIT_FIXED_S    Dee_TYPE_CONSTRUCTOR_INIT_ALLOC_AUTO /*!export(include("type.h"))*/
#define Dee_TYPE_CONSTRUCTOR_INIT_FIXED_GC_S Dee_TYPE_CONSTRUCTOR_INIT_ALLOC_AUTO /*!export(include("type.h"))*/
#else /* CONFIG_FIXED_ALLOCATOR_S_IS_AUTO */
#define Dee_TYPE_CONSTRUCTOR_INIT_FIXED_S    Dee_TYPE_CONSTRUCTOR_INIT_FIXED      /*!export(include("type.h"))*/
#define Dee_TYPE_CONSTRUCTOR_INIT_FIXED_GC_S Dee_TYPE_CONSTRUCTOR_INIT_FIXED_GC   /*!export(include("type.h"))*/
#endif /* !CONFIG_FIXED_ALLOCATOR_S_IS_AUTO */
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
