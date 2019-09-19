/* Copyright (c) 2019 Griefer@Work                                            *
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
 *    in a product, an acknowledgement in the product documentation would be  *
 *    appreciated but is not required.                                        *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_ALLOC_H
#define GUARD_DEEMON_ALLOC_H 1

#include "api.h"
#ifdef __CC__
#include <hybrid/typecore.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#ifndef alloca
#if defined(_MSC_VER) && !defined(__KOS_SYSTEM_HEADERS__)
#include <malloc.h>
#elif defined(__GNUC__) || __has_builtin(__builtin_alloca)
#define alloca(x) __builtin_alloca(x)
#elif !defined(__NO_has_include) && __has_include(<alloca.h>)
#include <alloca.h>
#endif
#endif /* !alloca */
#endif /* __CC__ */

DECL_BEGIN

#ifdef __CC__
/* Default malloc/free functions used for heap allocation.
 * NOTE: Upon allocation failure caches are cleared and an `Error.NoMemory' is thrown. */
DFUNDEF WUNUSED ATTR_MALLOC void *(DCALL Dee_Malloc)(size_t n_bytes);
DFUNDEF WUNUSED ATTR_MALLOC void *(DCALL Dee_Calloc)(size_t n_bytes);
DFUNDEF WUNUSED void *(DCALL Dee_Realloc)(void *ptr, size_t n_bytes);
DFUNDEF WUNUSED ATTR_MALLOC void *(DCALL Dee_TryMalloc)(size_t n_bytes);
DFUNDEF WUNUSED ATTR_MALLOC void *(DCALL Dee_TryCalloc)(size_t n_bytes);
DFUNDEF WUNUSED void *(DCALL Dee_TryRealloc)(void *ptr, size_t n_bytes);
DFUNDEF void (DCALL Dee_Free)(void *ptr);
DFUNDEF WUNUSED ATTR_MALLOC void *(DCALL DeeDbg_Malloc)(size_t n_bytes, char const *file, int line);
DFUNDEF WUNUSED ATTR_MALLOC void *(DCALL DeeDbg_Calloc)(size_t n_bytes, char const *file, int line);
DFUNDEF WUNUSED void *(DCALL DeeDbg_Realloc)(void *ptr, size_t n_bytes, char const *file, int line);
DFUNDEF WUNUSED ATTR_MALLOC void *(DCALL DeeDbg_TryMalloc)(size_t n_bytes, char const *file, int line);
DFUNDEF WUNUSED ATTR_MALLOC void *(DCALL DeeDbg_TryCalloc)(size_t n_bytes, char const *file, int line);
DFUNDEF WUNUSED void *(DCALL DeeDbg_TryRealloc)(void *ptr, size_t n_bytes, char const *file, int line);
DFUNDEF void (DCALL DeeDbg_Free)(void *ptr, char const *file, int line);
DFUNDEF void *(DCALL DeeDbg_UntrackAlloc)(void *ptr, char const *file, int line);

#ifndef NDEBUG
#define Dee_Malloc(n_bytes)          DeeDbg_Malloc(n_bytes, __FILE__, __LINE__)
#define Dee_Calloc(n_bytes)          DeeDbg_Calloc(n_bytes, __FILE__, __LINE__)
#define Dee_Realloc(ptr, n_bytes)    DeeDbg_Realloc(ptr, n_bytes, __FILE__, __LINE__)
#define Dee_TryMalloc(n_bytes)       DeeDbg_TryMalloc(n_bytes, __FILE__, __LINE__)
#define Dee_TryCalloc(n_bytes)       DeeDbg_TryCalloc(n_bytes, __FILE__, __LINE__)
#define Dee_TryRealloc(ptr, n_bytes) DeeDbg_TryRealloc(ptr, n_bytes, __FILE__, __LINE__)
#define Dee_Free(ptr)                DeeDbg_Free(ptr, __FILE__, __LINE__)
#define Dee_UntrackAlloc(ptr)        DeeDbg_UntrackAlloc(ptr, __FILE__, __LINE__)
#else /* !NDEBUG */
#define DeeDbg_Malloc(n_bytes, file, line)          Dee_Malloc(n_bytes)
#define DeeDbg_Calloc(n_bytes, file, line)          Dee_Calloc(n_bytes)
#define DeeDbg_Realloc(ptr, n_bytes, file, line)    Dee_Realloc(ptr, n_bytes)
#define DeeDbg_TryMalloc(n_bytes, file, line)       Dee_TryMalloc(n_bytes)
#define DeeDbg_TryCalloc(n_bytes, file, line)       Dee_TryCalloc(n_bytes)
#define DeeDbg_TryRealloc(ptr, n_bytes, file, line) Dee_TryRealloc(ptr, n_bytes)
#define DeeDbg_Free(ptr, file, line)                Dee_Free(ptr)
#define Dee_UntrackAlloc(ptr)                      (ptr)
#endif /* NDEBUG */

/* Reclaim free memory by going through internal pre-allocation caches,
 * freeing up to (but potentially exceeding by a bit) `max_collect' bytes of memory.
 * The actual amount freed is returned in bytes.
 * NOTE: This function is automatically called by `Dee_TryCollectMemory()' */
DFUNDEF size_t DCALL DeeMem_ClearCaches(size_t max_collect);

/* Try to clear caches and free up available memory.
 * @return: true:  Caches were cleared. - You should try to allocate memory again.
 * @return: false: Nope. - We're completely out of memory... */
DFUNDEF ATTR_COLD bool DCALL Dee_TryCollectMemory(size_t req_bytes);

/* Same as `Dee_TryCollectMemory()', but raise an
* `Error.NoMemory' if memory could not be collected. */
DFUNDEF WUNUSED ATTR_COLD bool DCALL Dee_CollectMemory(size_t req_bytes);

/* Throw a bad-allocation error for `req_bytes' bytes.
 * @return: -1: Always returns -1. */
DFUNDEF ATTR_COLD int DCALL Dee_BadAlloc(size_t req_bytes);
#endif /* __CC__ */

/* Default malloc/free functions used for object allocation. */
#define DeeObject_Malloc        Dee_Malloc
#define DeeObject_Calloc        Dee_Calloc
#define DeeObject_Realloc       Dee_Realloc
#define DeeObject_TryMalloc     Dee_TryMalloc
#define DeeObject_TryCalloc     Dee_TryCalloc
#define DeeObject_TryRealloc    Dee_TryRealloc
#define DeeDbgObject_Malloc     DeeDbg_Malloc
#define DeeDbgObject_Calloc     DeeDbg_Calloc
#define DeeDbgObject_Realloc    DeeDbg_Realloc
#define DeeDbgObject_TryMalloc  DeeDbg_TryMalloc
#define DeeDbgObject_TryCalloc  DeeDbg_TryCalloc
#define DeeDbgObject_TryRealloc DeeDbg_TryRealloc

#ifdef __CC__
#ifndef CONFIG_NO_OBJECT_SLABS
DFUNDEF void (DCALL DeeObject_Free)(void *ptr);
DFUNDEF void (DCALL DeeDbgObject_Free)(void *ptr, char const *file, int line);
#ifndef NDEBUG
#define DeeObject_Free(ptr)                DeeDbgObject_Free(ptr,__FILE__,__LINE__)
#else /* !NDEBUG */
#define DeeDbgObject_Free(ptr, file, line) DeeObject_Free(ptr)
#endif /* NDEBUG */
#else /* !CONFIG_NO_OBJECT_SLABS */
#define DeeObject_Free(ptr)                Dee_Free(ptr)
#define DeeDbgObject_Free(ptr, file, line) DeeDbg_Free(ptr, file, line)
#endif /* CONFIG_NO_OBJECT_SLABS */
#else /* __CC__ */
#ifdef CONFIG_NO_OBJECT_SLABS
#define DeeObject_Free                     Dee_Free
#define DeeDbgObject_Free                  DeeDbg_Free
#endif /* CONFIG_NO_OBJECT_SLABS */
#endif /* !__CC__ */

#ifdef __CC__
/* Free the reference tracker of a given object.
 * Should be called prior to `DeeObject_Free()' for any object
 * who's reference counter was modified at any point in time. */
#ifdef CONFIG_TRACE_REFCHANGES
DFUNDEF void DCALL DeeObject_FreeTracker(DeeObject *__restrict self);
#else /* CONFIG_TRACE_REFCHANGES */
#define DeeObject_FreeTracker(self) (void)0
#endif /* !CONFIG_TRACE_REFCHANGES */
#endif /* __CC__ */


/* Free-functions, and their capabilities
 *
 * Dee_Free:                Accepts  Dee_Malloc()
 *                          Accepts  NULL
 *
 * DeeSlab_Free:            Accepts  DeeSlab_Malloc<M>()
 *                          Accepts  Dee_Malloc()
 *
 * DeeSlab_FFree(...,<N>):
 * DeeSlab_Free<N>:         Accepts  DeeSlab_Malloc<M>()         | M >= N
 *                          Accepts  Dee_Malloc()
 *
 * DeeObject_Free:          Accepts  DeeObject_SlabMalloc<M>()
 *                          Accepts  DeeObject_FMalloc(<M>)
 *                          Accepts  DeeObject_Malloc()
 *                          Accepts  NULL
 *
 * DeeObject_FFree(...,<N>):
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
 * DeeGCObject_FFree(...,<N>):
 * DeeGCObject_SlabFree<N>: Accepts  DeeGCObject_SlabMalloc<M>() | M >= N
 *                          Accepts  DeeGCObject_FMalloc(<M>)    | M >= N
 *                          Accepts  DeeGCObject_Malloc()
 *
 * The following free functions accept and ignore NULL-pointers:
 *   - Dee_Free
 *   - DeeObject_Free
 *   - DeeGCObject_Free
 * The following free functions require the caller to pass non-NULL pointers:
 *   - DeeSlab_Free
 *   - DeeSlab_FFree(...,<N>)
 *   - DeeSlab_Free<N>
 *   - DeeObject_FFree(...,<N>)
 *   - DeeObject_SlabFree<N>
 *   - DeeGCObject_FFree(...,<N>)
 *   - DeeGCObject_SlabFree<N>
 */



#ifndef CONFIG_NO_OBJECT_SLABS
/* Slab allocator functions.
 * These operate in increments of at least sizeof(void *), with the acutal
 * allocated size being `X * sizeof(void *)' for `X = <X>' in `DeeObject_SlabMalloc<X>'
 * These allocator functions can be extremely fast, but have a fixed allocation limit.
 * They are designed to be used to speed up allocation of the many small helper objects
 * which can be found in deemon, most of which have a very short life-time, while others
 * exist for quite a while.
 * Pointers to these functions are mainly found in the `tp_alloc()' and `tp_free()'
 * fields of types. However, you shouldn't invoke these functions directly but use
 * `DeeObject_FMalloc()' and `DeeObject_FFree()', as well as their helper macros instead.
 * NOTES:
 *   - You may not pass a NULL pointer to the SlabFree() functions.
 *     Unlike regular free()-like functions, doing so is illegal.
 *
 *   - Due to the type-free-with-base quirk, pointers allocated by
 *     these functions can always be freed by `DeeObject_Free()',
 *     as well as any SlabFree() function with a smaller allocation
 *     size than what was used to allocate the original pointer.
 *     Doing this will not cause a memory leak, but may lead to
 *     inefficient usage of caches, as well as overall performance
 *     degradation.
 *     >> void *p = DeeObject_SlabMalloc8();
 *     >> if (p)
 *     >>     DeeObject_SlabFree4(p); // Allowed, but should not be done intentionally
 *     This again, is required because objects being revived from
 *     their destructors may lead to that object continuing to exist
 *     with another type and free-function, which is then allowed to
 *     assume a smaller object size than what was given.
 *
 *   - When declaring a sub-class of some type who's destructor you have no
 *     control over, such as is the case for user-defined classes, you must
 *     not declare your type to use slab allocators when the base type used
 *     other other, custom protocol, or simply used automatic allocators.
 *
 *   - Because sub-classing an instance with a user-class requires
 *     knowledge of the base-type's instance size, using slab allocators
 *     for that type means that allocations may be larger than needed, with
 *     information about that additional overhead's size then becoming lost
 *     to a user-defined sub-class, leading to an unused gap of memory. */
#define Dee_SLAB_MINSIZE  4  /* Size of the smallest slab */
#define Dee_SLAB_MAXSIZE  10 /* Size of the largest slab */
#define Dee_SLAB_COUNT    5  /* # of different slabs */
#define DeeSlab_ENUMERATE(func) \
	func(0, 4)  /* 16 / 32 */   \
	func(1, 5)  /* 20 / 40 */   \
	func(2, 6)  /* 24 / 48 */   \
	func(3, 8)  /* 32 / 64 */   \
	func(4, 10) /* 40 / 80 */
#define DEE_PRIVATE_SLAB_INDEXOF_CALLBACK(index, size) <= size *__SIZEOF_POINTER__ ? index##u:
#define DEE_PRIVATE_SLAB_HASSIZE_CALLBACK(index, size) size
#define DeeSlab_IndexOf(size) (DeeSlab_ENUMERATE((size) DEE_PRIVATE_SLAB_INDEXOF_CALLBACK) 0xffu)
#define DeeSlab_HasSize(size) (0 DeeSlab_ENUMERATE(|| (size) == DEE_PRIVATE_SLAB_HASSIZE_CALLBACK))
#define DeeSlab_InvokeDyn(func, size, args, fallback)  \
	((size) <= 4*__SIZEOF_POINTER__ ? func##4 args :   \
	 (size) <= 5*__SIZEOF_POINTER__ ? func##5 args :   \
	 (size) <= 6*__SIZEOF_POINTER__ ? func##6 args :   \
	 (size) <= 8*__SIZEOF_POINTER__ ? func##8 args :   \
	 (size) <= 10*__SIZEOF_POINTER__ ? func##10 args : \
	                                   fallback)
#ifndef __NO_builtin_choose_expr
#define DeeSlab_Invoke(func, size, args, fallback)                       \
	__builtin_choose_expr((size) <= 4*__SIZEOF_POINTER__,func##4 args,   \
	__builtin_choose_expr((size) <= 5*__SIZEOF_POINTER__,func##5 args,   \
	__builtin_choose_expr((size) <= 6*__SIZEOF_POINTER__,func##6 args,   \
	__builtin_choose_expr((size) <= 8*__SIZEOF_POINTER__,func##8 args,   \
	__builtin_choose_expr((size) <= 10*__SIZEOF_POINTER__,func##10 args, \
	                                                      fallback)))))
#else /* !__NO_builtin_choose_expr */
#define DeeSlab_Invoke  DeeSlab_InvokeDyn
#endif /* __NO_builtin_choose_expr */

#ifdef __CC__
#define DEE_PRIVATE_DEFINE_SLAB_FUNCTIONS(index, size)                                                \
	DFUNDEF WUNUSED ATTR_MALLOC void *(DCALL DeeSlab_Malloc##size)(void);                             \
	DFUNDEF WUNUSED ATTR_MALLOC void *(DCALL DeeSlab_Calloc##size)(void);                             \
	DFUNDEF WUNUSED ATTR_MALLOC void *(DCALL DeeSlab_TryMalloc##size)(void);                          \
	DFUNDEF WUNUSED ATTR_MALLOC void *(DCALL DeeSlab_TryCalloc##size)(void);                          \
	DFUNDEF WUNUSED ATTR_MALLOC void *(DCALL DeeDbgSlab_Malloc##size)(char const *file, int line);    \
	DFUNDEF WUNUSED ATTR_MALLOC void *(DCALL DeeDbgSlab_Calloc##size)(char const *file, int line);    \
	DFUNDEF WUNUSED ATTR_MALLOC void *(DCALL DeeDbgSlab_TryMalloc##size)(char const *file, int line); \
	DFUNDEF WUNUSED ATTR_MALLOC void *(DCALL DeeDbgSlab_TryCalloc##size)(char const *file, int line); \
	DFUNDEF WUNUSED ATTR_MALLOC void *(DCALL DeeGCObject_SlabMalloc##size)(void);                     \
	DFUNDEF WUNUSED ATTR_MALLOC void *(DCALL DeeGCObject_SlabCalloc##size)(void);                     \
	DFUNDEF WUNUSED ATTR_MALLOC void *(DCALL DeeGCObject_SlabTryMalloc##size)(void);                  \
	DFUNDEF WUNUSED ATTR_MALLOC void *(DCALL DeeGCObject_SlabTryCalloc##size)(void);                  \
	DFUNDEF void (DCALL DeeSlab_Free##size)(void *__restrict ptr);                                    \
	DFUNDEF void (DCALL DeeDbgSlab_Free##size)(void *__restrict ptr, char const *file, int line);     \
	DFUNDEF void (DCALL DeeGCObject_SlabFree##size)(void *__restrict ptr);
DeeSlab_ENUMERATE(DEE_PRIVATE_DEFINE_SLAB_FUNCTIONS)
#undef DEE_PRIVATE_DEFINE_SLAB_FUNCTIONS


/* Allocate fixed-size, general-purpose slab memory.
 * NOTE: This memory must be freed by one of:
 *   - DeeSlab_FFree(return,size2)  | size2 <= size
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
#endif /* __CC__ */

/* Free any kind of pointer allocated by the general-purpose slab allocators.
 * NOTE: Do not mix with object slab allocators, even if they may appear identical! */
#ifdef __INTELLISENSE__
DFUNDEF void (DCALL DeeSlab_Free)(void *__restrict ptr);
DFUNDEF void (DCALL DeeDbgSlab_Free)(void *__restrict ptr, char const *file, int line);
#elif Dee_SLAB_MINSIZE == 4
#define DeeSlab_Free      DeeSlab_Free4
#define DeeDbgSlab_Free   DeeDbgSlab_Free4
#else /* Dee_SLAB_MINSIZE == 4 */
#define DEE_PRIVATE_DeeSlab_Free2(x) DeeSlab_Free##x
#define DEE_PRIVATE_DeeSlab_Free(x) DEE_PRIVATE_DeeSlab_Free2(x)
#define DEE_PRIVATE_DeeDbgSlab_Free2(x) DeeDbgSlab_Free##x
#define DEE_PRIVATE_DeeDbgSlab_Free(x) DEE_PRIVATE_DeeDbgSlab_Free2(x)
#define DeeSlab_Free      DEE_PRIVATE_DeeSlab_Free(Dee_SLAB_MINSIZE)
#define DeeDbgSlab_Free   DEE_PRIVATE_DeeDbgSlab_Free(Dee_SLAB_MINSIZE)
#endif /* Dee_SLAB_MINSIZE != 4 */


/* Define slab-enabled object memory allocators, and override regular
 * slab allocator function to either include, or exclude debug information. */
#if DeeSlab_HasSize(4)
#define DeeObject_SlabMalloc4    DeeSlab_Malloc4
#define DeeObject_SlabCalloc4    DeeSlab_Calloc4
#define DeeObject_SlabTryMalloc4 DeeSlab_TryMalloc4
#define DeeObject_SlabTryCalloc4 DeeSlab_TryCalloc4
#define DeeObject_SlabFree4      DeeSlab_Free4
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
#define DeeObject_SlabMalloc5    DeeSlab_Malloc5
#define DeeObject_SlabCalloc5    DeeSlab_Calloc5
#define DeeObject_SlabTryMalloc5 DeeSlab_TryMalloc5
#define DeeObject_SlabTryCalloc5 DeeSlab_TryCalloc5
#define DeeObject_SlabFree5      DeeSlab_Free5
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
#define DeeObject_SlabMalloc6    DeeSlab_Malloc6
#define DeeObject_SlabCalloc6    DeeSlab_Calloc6
#define DeeObject_SlabTryMalloc6 DeeSlab_TryMalloc6
#define DeeObject_SlabTryCalloc6 DeeSlab_TryCalloc6
#define DeeObject_SlabFree6      DeeSlab_Free6
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
#define DeeObject_SlabMalloc8    DeeSlab_Malloc8
#define DeeObject_SlabCalloc8    DeeSlab_Calloc8
#define DeeObject_SlabTryMalloc8 DeeSlab_TryMalloc8
#define DeeObject_SlabTryCalloc8 DeeSlab_TryCalloc8
#define DeeObject_SlabFree8      DeeSlab_Free8
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
#define DeeObject_SlabMalloc10    DeeSlab_Malloc10
#define DeeObject_SlabCalloc10    DeeSlab_Calloc10
#define DeeObject_SlabTryMalloc10 DeeSlab_TryMalloc10
#define DeeObject_SlabTryCalloc10 DeeSlab_TryCalloc10
#define DeeObject_SlabFree10      DeeSlab_Free10
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
#else /* !CONFIG_NO_OBJECT_SLABS */

/* Allocate fixed-size, general-purpose slab memory.
 * NOTE: This memory must be freed by one of:
 *   - DeeSlab_FFree(return,size2)  | size2 <= size
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

/* Free any kind of pointer allocated by the general-purpose slab allocators.
 * NOTE: Do not mix with object slab allocators, even if they may appear identical! */
#ifdef __INTELLISENSE__
DFUNDEF void (DCALL DeeSlab_Free)(void *__restrict ptr);
DFUNDEF void (DCALL DeeDbgSlab_Free)(void *__restrict ptr, char const *file, int line);
#else /* __INTELLISENSE__ */
#define DeeSlab_Free      Dee_Free
#define DeeDbgSlab_Free   DeeDbg_Free
#endif /* !__INTELLISENSE__ */

#endif /* CONFIG_NO_OBJECT_SLABS */



#ifdef __CC__
/* Slab memory allocators with automatic component size detection.
 * >> MyStruct *ob;
 * >> ob = DeeSlab_MALLOC(MyStruct);
 * >> if likely(ob) {
 * >>     ...
 * >>     DeeSlab_FREE(ob);
 * >> }
 */
#define DeeSlab_MALLOC(T)                       ((T *)DeeSlab_Malloc(sizeof(T)))
#define DeeSlab_CALLOC(T)                       ((T *)DeeSlab_Calloc(sizeof(T)))
#define DeeSlab_TRYMALLOC(T)                    ((T *)DeeSlab_TryMalloc(sizeof(T)))
#define DeeSlab_TRYCALLOC(T)                    ((T *)DeeSlab_TryCalloc(sizeof(T)))
#define DeeSlab_FREE(typed_ptr)                 DeeSlab_FFree((void *)(typed_ptr), sizeof(*(typed_ptr)))
#define DeeSlab_XFREE(typed_ptr)                DeeSlab_XFFree((void *)(typed_ptr), sizeof(*(typed_ptr)))
#define DeeDbgSlab_MALLOC(T, file, line)        ((T *)DeeDbgSlab_Malloc(sizeof(T), file, line))
#define DeeDbgSlab_CALLOC(T, file, line)        ((T *)DeeDbgSlab_Calloc(sizeof(T), file, line))
#define DeeDbgSlab_TRYMALLOC(T, file, line)     ((T *)DeeDbgSlab_TryMalloc(sizeof(T), file, line))
#define DeeDbgSlab_TRYCALLOC(T, file, line)     ((T *)DeeDbgSlab_TryCalloc(sizeof(T), file, line))
#define DeeDbgSlab_FREE(typed_ptr, file, line)  DeeDbgSlab_FFree((void *)(typed_ptr), sizeof(*(typed_ptr), file, line))
#define DeeDbgSlab_XFREE(typed_ptr, file, line) DeeDbgSlab_XFFree((void *)(typed_ptr), sizeof(*(typed_ptr), file, line))



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
DFUNDEF size_t DCALL DeeSlab_Stat(DeeSlabStat *info, size_t bufsize);
/* Reset the slab max-statistics to the cur-values. */
DFUNDEF void DCALL DeeSlab_ResetStat(void);


/* Allocate fixed-size, object-purposed slab memory.
 * NOTE: This memory must be freed by one of:
 *   - DeeObject_FFree(return,size2)     | size2 <= size
 *   - DeeObject_SlabFree<size2>(return) | size2 <= size
 *   - DeeObject_Free(return) */
#ifdef CONFIG_NO_OBJECT_SLABS
#define DeeObject_FMalloc(size)    DeeObject_Malloc(size)
#define DeeObject_FCalloc(size)    DeeObject_Calloc(size)
#define DeeObject_FTryMalloc(size) DeeObject_TryMalloc(size)
#define DeeObject_FTryCalloc(size) DeeObject_TryCalloc(size)
#define DeeObject_FFree(ptr, size) DeeObject_Free(ptr)
#else /* CONFIG_NO_OBJECT_SLABS */
#define DeeObject_FMalloc(size)    DeeSlab_Invoke(DeeObject_SlabMalloc, size, (), DeeObject_Malloc(size))
#define DeeObject_FCalloc(size)    DeeSlab_Invoke(DeeObject_SlabCalloc, size, (), DeeObject_Calloc(size))
#define DeeObject_FTryMalloc(size) DeeSlab_Invoke(DeeObject_SlabTryMalloc, size, (), DeeObject_TryMalloc(size))
#define DeeObject_FTryCalloc(size) DeeSlab_Invoke(DeeObject_SlabTryCalloc, size, (), DeeObject_TryCalloc(size))
#define DeeObject_FFree(ptr, size) DeeSlab_Invoke(DeeObject_SlabFree, size, (ptr), DeeObject_Free(ptr))
#endif /* !CONFIG_NO_OBJECT_SLABS */

/* Same as the regular malloc functions, but use the same allocation methods that
 * would be used by `TYPE_FIXED_ALLOCATOR' and `TYPE_FIXED_ALLOCATOR_S', meaning
 * that pointers returned by these macros have binary compatibility with them. */
#define DeeObject_MALLOC(T)       ((T *)DeeObject_FMalloc(sizeof(T)))
#define DeeObject_CALLOC(T)       ((T *)DeeObject_FCalloc(sizeof(T)))
#define DeeObject_TRYMALLOC(T)    ((T *)DeeObject_FTryMalloc(sizeof(T)))
#define DeeObject_TRYCALLOC(T)    ((T *)DeeObject_FTryCalloc(sizeof(T)))
#define DeeObject_FREE(typed_ptr)       DeeObject_FFree(typed_ptr, sizeof(*(typed_ptr)))

#ifndef DEE_TYPE_ALLOCATOR
/* Specifies a custom object allocator declaration. */
#define DEE_TYPE_ALLOCATOR(tp_malloc, tp_free) (void *)(tp_free), {(uintptr_t)(void *)(tp_malloc) }

/* Specifies an automatic object allocator. */
#define DEE_TYPE_AUTOSIZED_ALLOCATOR(size)                 NULL, {(uintptr_t)(size) }
#define DEE_TYPE_AUTOSIZED_ALLOCATOR_R(min_size, max_size) NULL, {(uintptr_t)(max_size) }
#define DEE_TYPE_AUTO_ALLOCATOR(T)                         NULL, {(uintptr_t)sizeof(T) }

/* Expose shorter variants of macros */
#ifdef DEE_SOURCE
#define TYPE_ALLOCATOR              DEE_TYPE_ALLOCATOR
#define TYPE_AUTOSIZED_ALLOCATOR    DEE_TYPE_AUTOSIZED_ALLOCATOR
#define TYPE_AUTOSIZED_ALLOCATOR_R  DEE_TYPE_AUTOSIZED_ALLOCATOR_R
#define TYPE_AUTO_ALLOCATOR         DEE_TYPE_AUTO_ALLOCATOR
#endif /* DEE_SOURCE */
#endif /* !DEE_TYPE_ALLOCATOR */

#ifdef GUARD_DEEMON_OBJECT_H
/* Specifies an allocator that may provides optimizations
 * for types with a FIXED size (which most objects have). */
#ifdef CONFIG_NO_OBJECT_SLABS
#define DEE_TYPE_SIZED_ALLOCATOR_R     TYPE_AUTOSIZED_ALLOCATOR_R
#define DEE_TYPE_SIZED_ALLOCATOR_GC_R  TYPE_AUTOSIZED_ALLOCATOR_R
#define DEE_TYPE_SIZED_ALLOCATOR       TYPE_AUTOSIZED_ALLOCATOR
#define DEE_TYPE_SIZED_ALLOCATOR_GC    TYPE_AUTOSIZED_ALLOCATOR
#define DEE_TYPE_FIXED_ALLOCATOR       TYPE_AUTO_ALLOCATOR
#define DEE_TYPE_FIXED_ALLOCATOR_GC    TYPE_AUTO_ALLOCATOR
#else /* CONFIG_NO_OBJECT_SLABS */
#define DEE_TYPE_SIZED_ALLOCATOR_R(min_size, max_size)               \
	  DeeSlab_Invoke((void *)&DeeObject_SlabFree, min_size, , NULL), \
	{ DeeSlab_Invoke((uintptr_t)(void *)&DeeObject_SlabMalloc, max_size, , max_size) }
#define DEE_TYPE_SIZED_ALLOCATOR_GC_R(min_size, max_size)              \
	  DeeSlab_Invoke((void *)&DeeGCObject_SlabFree, min_size, , NULL), \
	{ DeeSlab_Invoke((uintptr_t)(void *)&DeeGCObject_SlabMalloc, max_size, , max_size) }
#define DEE_TYPE_SIZED_ALLOCATOR(size)    DEE_TYPE_SIZED_ALLOCATOR_R(size, size)
#define DEE_TYPE_SIZED_ALLOCATOR_GC(size) DEE_TYPE_SIZED_ALLOCATOR_GC_R(size, size)
#define DEE_TYPE_FIXED_ALLOCATOR(T)       DEE_TYPE_SIZED_ALLOCATOR_R(sizeof(T), sizeof(T))
#define DEE_TYPE_FIXED_ALLOCATOR_GC(T)    DEE_TYPE_SIZED_ALLOCATOR_GC_R(sizeof(T), sizeof(T))
#endif /* !CONFIG_NO_OBJECT_SLABS */
#ifdef DEE_SOURCE
#define TYPE_SIZED_ALLOCATOR_R         DEE_TYPE_SIZED_ALLOCATOR_R
#define TYPE_SIZED_ALLOCATOR_GC_R      DEE_TYPE_SIZED_ALLOCATOR_GC_R
#define TYPE_SIZED_ALLOCATOR           DEE_TYPE_SIZED_ALLOCATOR
#define TYPE_SIZED_ALLOCATOR_GC        DEE_TYPE_SIZED_ALLOCATOR_GC
#define TYPE_FIXED_ALLOCATOR           DEE_TYPE_FIXED_ALLOCATOR
#define TYPE_FIXED_ALLOCATOR_GC        DEE_TYPE_FIXED_ALLOCATOR_GC
#endif /* DEE_SOURCE */
#endif /* GUARD_DEEMON_OBJECT_H */

/* Same as `TYPE_FIXED_ALLOCATOR()', but don't link agains dedicated
 * allocator functions when doing so would require the creation of
 * relocations that might cause loading times to become larger. */
#undef CONFIG_FIXED_ALLOCATOR_S_IS_AUTO
#if !defined(CONFIG_BUILDING_DEEMON) || defined(__PIE__) || \
     defined(__PIC__) || defined(__pie__) || defined(__pic__) || \
     defined(CONFIG_NO_OBJECT_SLABS)
#define CONFIG_FIXED_ALLOCATOR_S_IS_AUTO 1
#ifdef GUARD_DEEMON_OBJECT_H
#define DEE_TYPE_FIXED_ALLOCATOR_S(T)    DEE_TYPE_AUTO_ALLOCATOR(T)
#define DEE_TYPE_FIXED_ALLOCATOR_GC_S(T) DEE_TYPE_AUTO_ALLOCATOR(T)
#ifdef DEE_SOURCE
#define TYPE_FIXED_ALLOCATOR_S           DEE_TYPE_FIXED_ALLOCATOR_S
#define TYPE_FIXED_ALLOCATOR_GC_S        DEE_TYPE_FIXED_ALLOCATOR_GC_S
#endif /* DEE_SOURCE */
#endif /* GUARD_DEEMON_OBJECT_H */
#define DeeObject_MALLOC_S(T)    ((T *)DeeObject_Malloc(sizeof(T)))
#define DeeObject_CALLOC_S(T)    ((T *)DeeObject_Calloc(sizeof(T)))
#define DeeObject_TRYMALLOC_S(T) ((T *)DeeObject_TryMalloc(sizeof(T)))
#define DeeObject_TRYCALLOC_S(T) ((T *)DeeObject_TryCalloc(sizeof(T)))
#define DeeObject_FREE_S               DeeObject_Free
#else /* !CONFIG_BUILDING_DEEMON || __PIC__ */
#ifdef GUARD_DEEMON_OBJECT_H
#define DEE_TYPE_FIXED_ALLOCATOR_S(T)    DEE_TYPE_FIXED_ALLOCATOR(T)
#define DEE_TYPE_FIXED_ALLOCATOR_GC_S(T) DEE_TYPE_FIXED_ALLOCATOR_GC(T)
#ifdef DEE_SOURCE
#define TYPE_FIXED_ALLOCATOR_S           DEE_TYPE_FIXED_ALLOCATOR_S
#define TYPE_FIXED_ALLOCATOR_GC_S        DEE_TYPE_FIXED_ALLOCATOR_GC_S
#endif /* DEE_SOURCE */
#endif /* GUARD_DEEMON_OBJECT_H */
#define DeeObject_FMALLOC_S            DeeObject_MALLOC
#define DeeObject_FCALLOC_S            DeeObject_CALLOC
#define DeeObject_TRYFMALLOC_S         DeeObject_TRYMALLOC
#define DeeObject_TRYFCALLOC_S         DeeObject_TRYCALLOC
#define DeeObject_FREE_S               DeeObject_FREE
#endif /* CONFIG_BUILDING_DEEMON && !__PIC__ */






/* A hybrid between alloca and malloc, using alloca for
 * small allocations, but malloc() for larger ones.
 * NOTE: In all cases, 'Dee_AFree()' should be used to clean up a
 *       pointer previously allocated using 'Dee_AMalloc()' and
 *       friends. */
#if defined(CONFIG_NO_AMALLOC) || defined(CONFIG_NO_ALLOCA) || \
  (!defined(alloca) && !defined(CONFIG_HAVE_ALLOCA)) || \
   !defined(NO_DBG_ALIGNMENT)
#define Dee_AMalloc(s)    Dee_Malloc(s)
#define Dee_ACalloc(s)    Dee_Calloc(s)
#define Dee_ATryMalloc(s) Dee_TryMalloc(s)
#define Dee_ATryCalloc(s) Dee_TryCalloc(s)
#define Dee_AFree(p)      Dee_Free(p)
#define Dee_XAFree(p)     Dee_Free(p)
#else /* !alloca */
#ifndef Dee_Alloca
#ifdef NO_DBG_ALIGNMENT
#define Dee_Alloca(x)     alloca(x)
#else /* NO_DBG_ALIGNMENT */
FORCELOCAL WUNUSED void *DCALL DeeDbg_AllocaCleanup(void *ptr) {
	DBG_ALIGNMENT_ENABLE();
	return ptr;
}
#define Dee_Alloca(x) (DBG_ALIGNMENT_DISABLE(), DeeDbg_AllocaCleanup(alloca(x)))
#endif /* !NO_DBG_ALIGNMENT */
#endif /* !Dee_Alloca */


#ifdef __SIZEOF_POINTER__
/* WARNING: This makes amalloc() unsuitable for floating point allocations. */
#define DEE_AMALLOC_ALIGN    __SIZEOF_POINTER__
#else /* __SIZEOF_POINTER__ */
#define DEE_AMALLOC_ALIGN    8
#endif /* !__SIZEOF_POINTER__ */
#define DEE_AMALLOC_MAX      512

#ifdef NDEBUG
#define DEE_AMALLOC_KEY_ALLOCA        0
#define DEE_AMALLOC_KEY_MALLOC        1
#define DEE_AMALLOC_SKEW_ALLOCA(p, s) (p)
#define DEE_AMALLOC_GETKEY(p)         ((__BYTE_TYPE__ *)(p))[-DEE_AMALLOC_ALIGN]
#define DEE_AMALLOC_MUSTFREE(p)       (DEE_AMALLOC_GETKEY(p) != DEE_AMALLOC_KEY_ALLOCA)
#else /* NDEBUG */
#define DEE_AMALLOC_KEY_ALLOCA  0x7c
#define DEE_AMALLOC_KEY_MALLOC  0xb3
#define DEE_AMALLOC_GETKEY(p)   ((__BYTE_TYPE__ *)(p))[-DEE_AMALLOC_ALIGN]
#define DEE_AMALLOC_MUSTFREE(p)                                    \
	(Dee_ASSERT(DEE_AMALLOC_GETKEY(p) == DEE_AMALLOC_KEY_ALLOCA || \
	            DEE_AMALLOC_GETKEY(p) == DEE_AMALLOC_KEY_MALLOC),  \
	 DEE_AMALLOC_GETKEY(p) == DEE_AMALLOC_KEY_MALLOC)
#define DEE_AMALLOC_SKEW_ALLOCA(p, s) memset(p, 0xcd, s)
#endif /* !NDEBUG */
#ifndef __NO_XBLOCK
#define Dee_AMallocStack(s)                                            \
	XBLOCK({                                                           \
		size_t const _s_     = (s) + DEE_AMALLOC_ALIGN;                \
		__BYTE_TYPE__ *_res_ = (__BYTE_TYPE__ *)Dee_Alloca(_s_);       \
		*_res_               = DEE_AMALLOC_KEY_ALLOCA;                 \
		_res_ += DEE_AMALLOC_ALIGN;                                    \
		(void)DEE_AMALLOC_SKEW_ALLOCA(_res_, _s_ - DEE_AMALLOC_ALIGN); \
		XRETURN((void *)_res_);                                        \
	})
#define Dee_AMallocHeap(s)                                       \
	XBLOCK({                                                     \
		size_t const _s_     = (s) + DEE_AMALLOC_ALIGN;          \
		__BYTE_TYPE__ *_res_ = (__BYTE_TYPE__ *)Dee_Malloc(_s_); \
		if (_res_)                                               \
			*_res_ = DEE_AMALLOC_KEY_MALLOC;                     \
		_res_ += DEE_AMALLOC_ALIGN;                              \
		XRETURN((void *)_res_);                                  \
	})
#define Dee_ATryMallocHeap(s)                                       \
	XBLOCK({                                                        \
		size_t const _s_     = (s) + DEE_AMALLOC_ALIGN;             \
		__BYTE_TYPE__ *_res_ = (__BYTE_TYPE__ *)Dee_TryMalloc(_s_); \
		if (_res_)                                                  \
			*_res_ = DEE_AMALLOC_KEY_MALLOC;                        \
		_res_ += DEE_AMALLOC_ALIGN;                                 \
		XRETURN((void *)_res_);                                     \
	})
#define Dee_AMalloc(s)                                                     \
	XBLOCK({                                                               \
		size_t const _s_ = (s) + DEE_AMALLOC_ALIGN;                        \
		__BYTE_TYPE__ *_res_;                                              \
		if (_s_ > DEE_AMALLOC_MAX) {                                       \
			_res_ = (__BYTE_TYPE__ *)Dee_Malloc(_s_);                      \
			if (_res_) {                                                   \
				*_res_ = DEE_AMALLOC_KEY_MALLOC;                           \
				_res_ += DEE_AMALLOC_ALIGN;                                \
			}                                                              \
		} else {                                                           \
			_res_  = (__BYTE_TYPE__ *)Dee_Alloca(_s_);                     \
			*_res_ = DEE_AMALLOC_KEY_ALLOCA;                               \
			_res_ += DEE_AMALLOC_ALIGN;                                    \
			(void)DEE_AMALLOC_SKEW_ALLOCA(_res_, _s_ - DEE_AMALLOC_ALIGN); \
		}                                                                  \
		XRETURN((void *)_res_);                                            \
	})
#define Dee_ATryMalloc(s)                                                  \
	XBLOCK({                                                               \
		size_t const _s_ = (s) + DEE_AMALLOC_ALIGN;                        \
		__BYTE_TYPE__ *_res_;                                              \
		if (_s_ > DEE_AMALLOC_MAX) {                                       \
			_res_ = (__BYTE_TYPE__ *)Dee_TryMalloc(_s_);                   \
			if (_res_) {                                                   \
				*_res_ = DEE_AMALLOC_KEY_MALLOC;                           \
				_res_ += DEE_AMALLOC_ALIGN;                                \
			}                                                              \
		} else {                                                           \
			_res_  = (__BYTE_TYPE__ *)Dee_Alloca(_s_);                     \
			*_res_ = DEE_AMALLOC_KEY_ALLOCA;                               \
			_res_ += DEE_AMALLOC_ALIGN;                                    \
			(void)DEE_AMALLOC_SKEW_ALLOCA(_res_, _s_ - DEE_AMALLOC_ALIGN); \
		}                                                                  \
		XRETURN((void *)_res_);                                            \
	})
#define Dee_ACallocStack(s)                                      \
	XBLOCK({                                                     \
		size_t const _s_     = (s) + DEE_AMALLOC_ALIGN;          \
		__BYTE_TYPE__ *_res_ = (__BYTE_TYPE__ *)Dee_Alloca(_s_); \
		*_res_               = DEE_AMALLOC_KEY_ALLOCA;           \
		_res_ += DEE_AMALLOC_ALIGN;                              \
		memset(_res_, 0, _s_ - DEE_AMALLOC_ALIGN);               \
		XRETURN((void *)_res_);                                  \
	})
#define Dee_ACallocHeap(s)                                       \
	XBLOCK({                                                     \
		size_t const _s_     = (s) + DEE_AMALLOC_ALIGN;          \
		__BYTE_TYPE__ *_res_ = (__BYTE_TYPE__ *)Dee_Calloc(_s_); \
		if (_res_) {                                             \
			*_res_ = DEE_AMALLOC_KEY_MALLOC;                     \
			_res_ += DEE_AMALLOC_ALIGN;                          \
		}                                                        \
		XRETURN((void *)_res_);                                  \
	})
#define Dee_ATryCallocHeap(s)                                       \
	XBLOCK({                                                        \
		size_t const _s_     = (s) + DEE_AMALLOC_ALIGN;             \
		__BYTE_TYPE__ *_res_ = (__BYTE_TYPE__ *)Dee_TryCalloc(_s_); \
		if (_res_) {                                                \
			*_res_ = DEE_AMALLOC_KEY_MALLOC;                        \
			_res_ += DEE_AMALLOC_ALIGN;                             \
		}                                                           \
		XRETURN((void *)_res_);                                     \
	})
#define Dee_ACalloc(s)                                 \
	XBLOCK({                                           \
		size_t const _s_ = (s) + DEE_AMALLOC_ALIGN;    \
		__BYTE_TYPE__ *_res_;                          \
		if (_s_ > DEE_AMALLOC_MAX) {                   \
			_res_ = (__BYTE_TYPE__ *)Dee_Calloc(_s_);  \
			if (_res_) {                               \
				*_res_ = DEE_AMALLOC_KEY_MALLOC;       \
				_res_ += DEE_AMALLOC_ALIGN;            \
			}                                          \
		} else {                                       \
			_res_  = (__BYTE_TYPE__ *)Dee_Alloca(_s_); \
			*_res_ = DEE_AMALLOC_KEY_ALLOCA;           \
			_res_ += DEE_AMALLOC_ALIGN;                \
			memset(_res_, 0, _s_ - DEE_AMALLOC_ALIGN); \
		}                                              \
		XRETURN((void *)_res_);                        \
	})
#define Dee_ATryCalloc(s)                                \
	XBLOCK({                                             \
		size_t const _s_ = (s) + DEE_AMALLOC_ALIGN;      \
		__BYTE_TYPE__ *_res_;                            \
		if (_s_ > DEE_AMALLOC_MAX) {                     \
			_res_ = (__BYTE_TYPE__ *)Dee_TryCalloc(_s_); \
			if (_res_) {                                 \
				*_res_ = DEE_AMALLOC_KEY_MALLOC;         \
				_res_ += DEE_AMALLOC_ALIGN;              \
			}                                            \
		} else {                                         \
			_res_  = (__BYTE_TYPE__ *)Dee_Alloca(_s_);   \
			*_res_ = DEE_AMALLOC_KEY_ALLOCA;             \
			_res_ += DEE_AMALLOC_ALIGN;                  \
			memset(_res_, 0, _s_ - DEE_AMALLOC_ALIGN);   \
		}                                                \
		XRETURN((void *)_res_);                          \
	})
#define Dee_AFree(p)                                                      \
	XBLOCK({                                                              \
		void *const _p_ = (p);                                            \
		if (DEE_AMALLOC_MUSTFREE(_p_))                                    \
			Dee_Free((void *)((__BYTE_TYPE__ *)_p_ - DEE_AMALLOC_ALIGN)); \
		(void)0;                                                          \
	})
#define Dee_XAFree(p)                                                     \
	XBLOCK({                                                              \
		void *const _p_ = (p);                                            \
		if (_p_ && DEE_AMALLOC_MUSTFREE(_p_))                             \
			Dee_Free((void *)((__BYTE_TYPE__ *)_p_ - DEE_AMALLOC_ALIGN)); \
		(void)0;                                                          \
	})
#else /* !__NO_XBLOCK */
#ifdef NDEBUG
#define Dee_AMallocHeap(s)  Dee_AMallocHeap(s)
LOCAL void *(DCALL Dee_AMallocHeap)(size_t s) {
	__BYTE_TYPE__ *res;
	res = (__BYTE_TYPE__ *)Dee_Malloc(s + DEE_AMALLOC_ALIGN);
	if likely(res) {
		*res = DEE_AMALLOC_KEY_MALLOC;
		res += DEE_AMALLOC_ALIGN;
	}
	return (void *)res;
}
#define Dee_ATryMallocHeap(s)  Dee_ATryMallocHeap(s)
LOCAL void *(DCALL Dee_ATryMallocHeap)(size_t s) {
	__BYTE_TYPE__ *res;
	res = (__BYTE_TYPE__ *)Dee_TryMalloc(s + DEE_AMALLOC_ALIGN);
	if likely(res) {
		*res = DEE_AMALLOC_KEY_MALLOC;
		res += DEE_AMALLOC_ALIGN;
	}
	return (void *)res;
}
#define Dee_ACallocHeap(s)  Dee_ACallocHeap(s)
LOCAL void *(DCALL Dee_ACallocHeap)(size_t s) {
	__BYTE_TYPE__ *res;
	res = (__BYTE_TYPE__ *)Dee_Calloc(s + DEE_AMALLOC_ALIGN);
	if likely(res) {
		*res = DEE_AMALLOC_KEY_MALLOC;
		res += DEE_AMALLOC_ALIGN;
	}
	return (void *)res;
}
#define Dee_ATryCallocHeap(s)  Dee_ATryCallocHeap(s)
LOCAL void *(DCALL Dee_ATryCallocHeap)(size_t s) {
	__BYTE_TYPE__ *res;
	res = (__BYTE_TYPE__ *)Dee_TryCalloc(s + DEE_AMALLOC_ALIGN);
	if likely(res) {
		*res = DEE_AMALLOC_KEY_MALLOC;
		res += DEE_AMALLOC_ALIGN;
	}
	return (void *)res;
}
#else /* NDEBUG */
#define Dee_AMallocHeap(s) DeeDbg_AMallocHeap(s, __FILE__, __LINE__)
LOCAL WUNUSED void *(DCALL DeeDbg_AMallocHeap)(size_t s, char const *file, int line) {
	__BYTE_TYPE__ *res;
	res = (__BYTE_TYPE__ *)DeeDbg_Malloc(s + DEE_AMALLOC_ALIGN, file, line);
	if likely(res) {
		*res = DEE_AMALLOC_KEY_MALLOC;
		res += DEE_AMALLOC_ALIGN;
	}
	return (void *)res;
}
#define Dee_ATryMallocHeap(s) DeeDbg_ATryMallocHeap(s, __FILE__, __LINE__)
LOCAL WUNUSED void *(DCALL DeeDbg_ATryMallocHeap)(size_t s, char const *file, int line) {
	__BYTE_TYPE__ *res;
	res = (__BYTE_TYPE__ *)DeeDbg_TryMalloc(s + DEE_AMALLOC_ALIGN, file, line);
	if likely(res) {
		*res = DEE_AMALLOC_KEY_MALLOC;
		res += DEE_AMALLOC_ALIGN;
	}
	return (void *)res;
}
#define Dee_ACallocHeap(s) DeeDbg_ACallocHeap(s, __FILE__, __LINE__)
LOCAL WUNUSED void *(DCALL DeeDbg_ACallocHeap)(size_t s, char const *file, int line) {
	__BYTE_TYPE__ *res;
	res = (__BYTE_TYPE__ *)DeeDbg_Calloc(s + DEE_AMALLOC_ALIGN, file, line);
	if likely(res) {
		*res = DEE_AMALLOC_KEY_MALLOC;
		res += DEE_AMALLOC_ALIGN;
	}
	return (void *)res;
}
#define Dee_ATryCallocHeap(s) DeeDbg_ATryCallocHeap(s, __FILE__, __LINE__)
LOCAL WUNUSED void *(DCALL DeeDbg_ATryCallocHeap)(size_t s, char const *file, int line) {
	__BYTE_TYPE__ *res;
	res = (__BYTE_TYPE__ *)DeeDbg_TryCalloc(s + DEE_AMALLOC_ALIGN, file, line);
	if likely(res) {
		*res = DEE_AMALLOC_KEY_MALLOC;
		res += DEE_AMALLOC_ALIGN;
	}
	return (void *)res;
}
#endif /* !NDEBUG */
#define Dee_AMallocStack(s) Dee_AMallocStack_init(Dee_Alloca((s) + DEE_AMALLOC_ALIGN), (s))
#define Dee_AMalloc(s)      ((s) > DEE_AMALLOC_MAX - DEE_AMALLOC_ALIGN ? Dee_AMallocHeap(s) : Dee_AMallocStack(s))
#define Dee_ATryMalloc(s)   ((s) > DEE_AMALLOC_MAX - DEE_AMALLOC_ALIGN ? Dee_ATryMallocHeap(s) : Dee_AMallocStack(s))
#ifdef NDEBUG
#define Dee_AMallocStack_init(p, s) Dee_AMallocStack_init_(p)
LOCAL WUNUSED void *(DCALL Dee_AMallocStack_init_)(void *p) {
	*(__BYTE_TYPE__ *)p = DEE_AMALLOC_KEY_ALLOCA;
	return (__BYTE_TYPE__ *)p + DEE_AMALLOC_ALIGN;
}
#else /* NDEBUG */
LOCAL WUNUSED void *(DCALL Dee_AMallocStack_init)(void *p, size_t s) {
	*(__BYTE_TYPE__ *)p = DEE_AMALLOC_KEY_ALLOCA;
	return DEE_AMALLOC_SKEW_ALLOCA((__BYTE_TYPE__ *)p + DEE_AMALLOC_ALIGN, s);
}
#endif /* !NDEBUG */
#define Dee_ACallocStack(s) Dee_ACallocStack_init(Dee_Alloca((s) + DEE_AMALLOC_ALIGN), (s))
#define Dee_ACalloc(s)      ((s) > DEE_AMALLOC_MAX - DEE_AMALLOC_ALIGN ? Dee_ACallocHeap(s) : Dee_ACallocStack(s))
#define Dee_ATryCalloc(s)   ((s) > DEE_AMALLOC_MAX - DEE_AMALLOC_ALIGN ? Dee_ATryCallocHeap(s) : Dee_ACallocStack(s))
LOCAL WUNUSED void *(DCALL Dee_ACallocStack_init)(void *p, size_t s) {
	*(__BYTE_TYPE__ *)p = DEE_AMALLOC_KEY_ALLOCA;
	return memset((__BYTE_TYPE__ *)p + DEE_AMALLOC_ALIGN, 0, s);
}

#define Dee_AFree(p)  Dee_AFree(p)
#define Dee_XAFree(p) Dee_XAFree(p) 
LOCAL void (DCALL Dee_AFree)(void *p) {
	if (DEE_AMALLOC_MUSTFREE(p))
		Dee_Free((void *)((__BYTE_TYPE__ *)p - DEE_AMALLOC_ALIGN));
}

LOCAL void (DCALL Dee_XAFree)(void *p) {
	if (p && DEE_AMALLOC_MUSTFREE(p))
		Dee_Free((void *)((__BYTE_TYPE__ *)p - DEE_AMALLOC_ALIGN));
}
#endif /* __NO_XBLOCK */

#define Dee_AMallocNoFail(p, s)                                \
	do {                                                       \
		size_t const _s_ = (s) + DEE_AMALLOC_ALIGN;            \
		if (_s_ > DEE_AMALLOC_MAX &&                           \
		    (*(void **)&(p) = Dee_Malloc(_s_)) != __NULLPTR) { \
			*(__BYTE_TYPE__ *)(p) = DEE_AMALLOC_KEY_MALLOC;    \
			*(__BYTE_TYPE__ **)&(p) += DEE_AMALLOC_ALIGN;      \
		} else {                                               \
			*(void **)&(p)        = Dee_Alloca(_s_);           \
			*(__BYTE_TYPE__ *)(p) = DEE_AMALLOC_KEY_ALLOCA;    \
			*(__BYTE_TYPE__ **)&(p) += DEE_AMALLOC_ALIGN;      \
			memset((void *)(p), 0, _s_ - DEE_AMALLOC_ALIGN);   \
		}                                                      \
	} __WHILE0
#define Dee_ATryMallocNoFail(p, s)                                \
	do {                                                          \
		size_t const _s_ = (s) + DEE_AMALLOC_ALIGN;               \
		if (_s_ > DEE_AMALLOC_MAX &&                              \
		    (*(void **)&(p) = Dee_TryMalloc(_s_)) != __NULLPTR) { \
			*(__BYTE_TYPE__ *)(p) = DEE_AMALLOC_KEY_MALLOC;       \
			*(__BYTE_TYPE__ **)&(p) += DEE_AMALLOC_ALIGN;         \
		} else {                                                  \
			*(void **)&(p)        = Dee_Alloca(_s_);              \
			*(__BYTE_TYPE__ *)(p) = DEE_AMALLOC_KEY_ALLOCA;       \
			*(__BYTE_TYPE__ **)&(p) += DEE_AMALLOC_ALIGN;         \
			memset((void *)(p), 0, _s_ - DEE_AMALLOC_ALIGN);      \
		}                                                         \
	} __WHILE0
#define Dee_ACallocNoFail(p, s)                                                  \
	do {                                                                         \
		size_t const _s_ = (s) + DEE_AMALLOC_ALIGN;                              \
		if (_s_ > DEE_AMALLOC_MAX &&                                             \
		    (*(void **)&(p) = Dee_Calloc(_s_)) != __NULLPTR) {                   \
			*(__BYTE_TYPE__ *)(p) = DEE_AMALLOC_KEY_MALLOC;                      \
			*(__BYTE_TYPE__ **)&(p) += DEE_AMALLOC_ALIGN;                        \
		} else {                                                                 \
			*(void **)&(p)        = Dee_Alloca(_s_);                             \
			*(__BYTE_TYPE__ *)(p) = DEE_AMALLOC_KEY_ALLOCA;                      \
			*(__BYTE_TYPE__ **)&(p) += DEE_AMALLOC_ALIGN;                        \
			(void)DEE_AMALLOC_SKEW_ALLOCA((void *)(p), _s_ - DEE_AMALLOC_ALIGN); \
		}                                                                        \
	} __WHILE0
#define Dee_ATryCallocNoFail(p, s)                                               \
	do {                                                                         \
		size_t const _s_ = (s) + DEE_AMALLOC_ALIGN;                              \
		if (_s_ > DEE_AMALLOC_MAX &&                                             \
		    (*(void **)&(p) = Dee_TryCalloc(_s_)) != __NULLPTR) {                \
			*(__BYTE_TYPE__ *)(p) = DEE_AMALLOC_KEY_MALLOC;                      \
			*(__BYTE_TYPE__ **)&(p) += DEE_AMALLOC_ALIGN;                        \
		} else {                                                                 \
			*(void **)&(p)        = Dee_Alloca(_s_);                             \
			*(__BYTE_TYPE__ *)(p) = DEE_AMALLOC_KEY_ALLOCA;                      \
			*(__BYTE_TYPE__ **)&(p) += DEE_AMALLOC_ALIGN;                        \
			(void)DEE_AMALLOC_SKEW_ALLOCA((void *)(p), _s_ - DEE_AMALLOC_ALIGN); \
		}                                                                        \
	} __WHILE0
#endif /* alloca */
#endif /* __CC__ */

DECL_END

#endif /* !GUARD_DEEMON_ALLOC_H */
