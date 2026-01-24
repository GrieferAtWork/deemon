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
#ifndef GUARD_DEEMON_UTIL_CACHE_H
#define GUARD_DEEMON_UTIL_CACHE_H 1 /*!export-*/

#include "../api.h"

#include "../alloc.h" /* DeeDbgObject_Malloc, DeeDbg_Malloc, DeeObject_Free, DeeObject_Malloc, Dee_Free, Dee_Malloc, Dee_TryMalloc */
#include "lock.h"     /* Dee_ATOMIC_LOCK_INIT, Dee_atomic_lock_* */

#include <stddef.h> /* NULL, size_t */

DECL_BEGIN

#ifdef DEE_SOURCE
#define Dee_cache_struct             cache_struct
#define Dee_cache_object             cache_object
#define DEFINE_STRUCT_CACHE          DEE_DEFINE_STRUCT_CACHE
#define DEFINE_OBJECT_CACHE          DEE_DEFINE_OBJECT_CACHE
#define DECLARE_STRUCT_CACHE         DEE_DECLARE_STRUCT_CACHE
#define DECLARE_OBJECT_CACHE         DEE_DECLARE_OBJECT_CACHE
#define DEFINE_STRUCT_CACHE_TRYALLOC DEE_DEFINE_STRUCT_CACHE_TRYALLOC
#define DEFINE_STRUCT_CACHE_EX       DEE_DEFINE_STRUCT_CACHE_EX
#define DEFINE_OBJECT_CACHE_EX       DEE_DEFINE_OBJECT_CACHE_EX
#endif /* DEE_SOURCE */

struct Dee_cache_struct {
	struct Dee_cache_struct *cs_next; /* [0..1][owned] Next cached structure. */
};
struct Dee_cache_object {
	struct Dee_cache_object *co_next; /* [0..1][owned] Next cached object. */
};

#define DEE_DEFINE_STRUCT_CACHE(name, ALLOC_TYPE, limit) \
	DEE_DEFINE_STRUCT_CACHE_EX(name, ALLOC_TYPE, sizeof(ALLOC_TYPE), limit)
#define DEE_DEFINE_OBJECT_CACHE(name, ALLOC_TYPE, limit) \
	DEE_DEFINE_OBJECT_CACHE_EX(name, ALLOC_TYPE, sizeof(ALLOC_TYPE), limit)

#ifndef NDEBUG
#define DEE_OBJECT_CACHE_IFDBG(x) x
#else /* !NDEBUG */
#define DEE_OBJECT_CACHE_IFDBG(x)
#endif /* NDEBUG */

#ifndef CONFIG_NO_THREADS
#define DEE_DECLARE_STRUCT_CACHE(name, ALLOC_TYPE)             \
	INTDEF Dee_atomic_lock_t structcache_##name##_lock;        \
	INTDEF struct Dee_cache_struct *structcache_##name##_list; \
	INTDEF size_t structcache_##name##_size;                   \
	INTDEF size_t DCALL name##_clear(size_t max_clear);        \
	INTDEF void DCALL name##_free(ALLOC_TYPE *__restrict ob);  \
	INTDEF ALLOC_TYPE *(DCALL name##_alloc)(void);             \
	DEE_OBJECT_CACHE_IFDBG(                                    \
	INTDEF ALLOC_TYPE *DCALL name##_dbgalloc(char const *file, int line);)
#define DEE_DECLARE_OBJECT_CACHE(name, ALLOC_TYPE)                         \
	INTDEF Dee_atomic_lock_t obcache_##name##_lock;                        \
	INTDEF struct Dee_cache_object *obcache_##name##_list;                 \
	INTDEF size_t obcache_##name##_size;                                   \
	INTDEF size_t DCALL name##_clear(size_t max_clear);                    \
	INTDEF void DCALL name##_free(ALLOC_TYPE *__restrict ob);              \
	INTDEF ALLOC_TYPE *(DCALL name##_alloc)(void);                         \
	DEE_OBJECT_CACHE_IFDBG(                                                \
	INTDEF ALLOC_TYPE *DCALL name##_dbgalloc(char const *file, int line);) \
	INTDEF void DCALL name##_tp_free(void *__restrict ob);                 \
	INTDEF void *DCALL name##_tp_alloc(void);

#define DEE_DEFINE_STRUCT_CACHE_TRYALLOC(name, ALLOC_TYPE, object_size)               \
	INTERN ALLOC_TYPE *(DCALL name##_tryalloc)(void) {                                \
		ALLOC_TYPE *result;                                                           \
		Dee_atomic_lock_acquire(&structcache_##name##_lock);                          \
		Dee_ASSERT((structcache_##name##_size != 0) ==                                \
		           (structcache_##name##_list != NULL));                              \
		result = (ALLOC_TYPE *)structcache_##name##_list;                             \
		if (result) {                                                                 \
			structcache_##name##_list = ((struct Dee_cache_struct *)result)->cs_next; \
			--structcache_##name##_size;                                              \
		}                                                                             \
		Dee_atomic_lock_release(&structcache_##name##_lock);                          \
		if (!result)                                                                  \
			result = (ALLOC_TYPE *)(Dee_TryMalloc)(object_size);                      \
		DEE_OBJECT_CACHE_IFDBG(else memset(result, 0xcc, object_size);)               \
		return result;                                                                \
	}
#define DEE_DEFINE_STRUCT_CACHE_EX(name, ALLOC_TYPE, object_size, limit)              \
	INTERN Dee_atomic_lock_t structcache_##name##_lock        = Dee_ATOMIC_LOCK_INIT; \
	INTERN struct Dee_cache_struct *structcache_##name##_list = NULL;                 \
	INTERN size_t structcache_##name##_size                   = 0;                    \
	INTERN size_t DCALL name##_clear(size_t max_clear) {                              \
		size_t result = 0;                                                            \
		Dee_atomic_lock_acquire(&structcache_##name##_lock);                          \
		while (result < max_clear && structcache_##name##_list) {                     \
			struct Dee_cache_struct *iter;                                            \
			Dee_ASSERT(structcache_##name##_size != 0);                               \
			iter = structcache_##name##_list;                                         \
			structcache_##name##_list = iter->cs_next;                                \
			--structcache_##name##_size;                                              \
			result += object_size;                                                    \
			Dee_Free(iter);                                                           \
		}                                                                             \
		Dee_atomic_lock_release(&structcache_##name##_lock);                          \
		return result;                                                                \
	}                                                                                 \
	INTERN void DCALL name##_free(ALLOC_TYPE *__restrict ob) {                        \
		DEE_OBJECT_CACHE_IFDBG(memset(ob, 0xcc, object_size);)                        \
		Dee_atomic_lock_acquire(&structcache_##name##_lock);                          \
		if (structcache_##name##_size < limit) {                                      \
			++structcache_##name##_size;                                              \
			((struct Dee_cache_struct *)ob)->cs_next = structcache_##name##_list;     \
			structcache_##name##_list = (struct Dee_cache_struct *)ob;                \
			Dee_atomic_lock_release(&structcache_##name##_lock);                      \
		} else {                                                                      \
			Dee_atomic_lock_release(&structcache_##name##_lock);                      \
			Dee_Free(ob);                                                             \
		}                                                                             \
	}                                                                                 \
	INTERN ALLOC_TYPE *(DCALL name##_alloc)(void) {                                   \
		ALLOC_TYPE *result;                                                           \
		Dee_atomic_lock_acquire(&structcache_##name##_lock);                          \
		Dee_ASSERT((structcache_##name##_size != 0) ==                                \
		           (structcache_##name##_list != NULL));                              \
		result = (ALLOC_TYPE *)structcache_##name##_list;                             \
		if (result) {                                                                 \
			structcache_##name##_list = ((struct Dee_cache_struct *)result)->cs_next; \
			--structcache_##name##_size;                                              \
		}                                                                             \
		Dee_atomic_lock_release(&structcache_##name##_lock);                          \
		if (!result)                                                                  \
			result = (ALLOC_TYPE *)(Dee_Malloc)(object_size);                         \
		DEE_OBJECT_CACHE_IFDBG(else memset(result, 0xcc, object_size);)               \
		return result;                                                                \
	}                                                                                 \
	DEE_OBJECT_CACHE_IFDBG(                                                           \
	INTERN ALLOC_TYPE *DCALL name##_dbgalloc(char const *file, int line) {            \
		ALLOC_TYPE *result;                                                           \
		/*Dee_atomic_lock_acquire(&structcache_##name##_lock);                        \
		Dee_ASSERT((structcache_##name##_size != 0) ==                                \
		           (structcache_##name##_list != NULL));                              \
		result = (ALLOC_TYPE *)structcache_##name##_list;                             \
		if (result) {                                                                 \
			structcache_##name##_list = ((struct Dee_cache_struct *)result)->cs_next; \
			--structcache_##name##_size;                                              \
		}                                                                             \
		Dee_atomic_lock_release(&structcache_##name##_lock);                          \
		if (!result)*/ {                                                              \
			result = (ALLOC_TYPE *)DeeDbg_Malloc(object_size, file, line);            \
		}                                                                             \
		/*DEE_OBJECT_CACHE_IFDBG(else memset(result, 0xcc, object_size);)*/           \
		return result;                                                                \
	})
#define DEE_DEFINE_OBJECT_CACHE_EX(name, ALLOC_TYPE, object_size, limit)          \
	INTERN Dee_atomic_lock_t obcache_##name##_lock        = Dee_ATOMIC_LOCK_INIT; \
	INTERN struct Dee_cache_object *obcache_##name##_list = NULL;                 \
	INTERN size_t obcache_##name##_size                   = 0;                    \
	INTERN size_t DCALL name##_clear(size_t max_clear) {                          \
		size_t result = 0;                                                        \
		Dee_atomic_lock_acquire(&obcache_##name##_lock);                          \
		while (result < max_clear && obcache_##name##_list) {                     \
			struct Dee_cache_object *iter;                                        \
			Dee_ASSERT(obcache_##name##_size != 0);                               \
			iter                  = obcache_##name##_list;                        \
			obcache_##name##_list = iter->co_next;                                \
			--obcache_##name##_size;                                              \
			result += object_size;                                                \
			DeeObject_Free(iter);                                                 \
		}                                                                         \
		Dee_atomic_lock_release(&obcache_##name##_lock);                          \
		return result;                                                            \
	}                                                                             \
	INTERN void DCALL name##_free(ALLOC_TYPE *__restrict ob) {                    \
		DEE_OBJECT_CACHE_IFDBG(memset(ob, 0xcc, object_size);)                    \
		Dee_atomic_lock_acquire(&obcache_##name##_lock);                          \
		if (obcache_##name##_size < limit) {                                      \
			++obcache_##name##_size;                                              \
			((struct Dee_cache_object *)ob)->co_next = obcache_##name##_list;     \
			obcache_##name##_list = (struct Dee_cache_object *)ob;                \
			Dee_atomic_lock_release(&obcache_##name##_lock);                      \
		} else {                                                                  \
			Dee_atomic_lock_release(&obcache_##name##_lock);                      \
			DeeObject_Free(ob);                                                   \
		}                                                                         \
	}                                                                             \
	INTERN ALLOC_TYPE *(DCALL name##_alloc)(void) {                               \
		ALLOC_TYPE *result;                                                       \
		Dee_atomic_lock_acquire(&obcache_##name##_lock);                          \
		Dee_ASSERT((obcache_##name##_size != 0) ==                                \
		           (obcache_##name##_list != NULL));                              \
		result = (ALLOC_TYPE *)obcache_##name##_list;                             \
		if (result) {                                                             \
			obcache_##name##_list = ((struct Dee_cache_object *)result)->co_next; \
			--obcache_##name##_size;                                              \
		}                                                                         \
		Dee_atomic_lock_release(&obcache_##name##_lock);                          \
		if (!result)                                                              \
			result = (ALLOC_TYPE *)(DeeObject_Malloc)(object_size);               \
		DEE_OBJECT_CACHE_IFDBG(else memset(result, 0xcc, object_size);)           \
		return result;                                                            \
	}                                                                             \
	DEE_OBJECT_CACHE_IFDBG(                                                       \
	INTERN ALLOC_TYPE *DCALL name##_dbgalloc(char const *file, int line) {        \
		ALLOC_TYPE *result;                                                       \
		/*Dee_atomic_lock_acquire(&obcache_##name##_lock);                        \
		Dee_ASSERT((obcache_##name##_size != 0) ==                                \
		           (obcache_##name##_list != NULL));                              \
		result = (ALLOC_TYPE *)obcache_##name##_list;                             \
		if (result) {                                                             \
			obcache_##name##_list = ((struct Dee_cache_object *)result)->co_next; \
			--obcache_##name##_size;                                              \
		}                                                                         \
		Dee_atomic_lock_release(&obcache_##name##_lock);                          \
		if (!result)*/ {                                                          \
			result = (ALLOC_TYPE *)DeeDbgObject_Malloc(object_size, file, line);  \
		}                                                                         \
		/*DEE_OBJECT_CACHE_IFDBG(else memset(result, 0xcc, object_size);)*/       \
		return result;                                                            \
	})                                                                            \
	INTERN void DCALL name##_tp_free(void *__restrict ob) {                       \
		name##_free((ALLOC_TYPE *)ob);                                            \
	}                                                                             \
	INTERN void *DCALL name##_tp_alloc(void) {                                    \
		return (void *)(name##_alloc)();                                          \
	}
#else /* !CONFIG_NO_THREADS */
#define DEE_DECLARE_STRUCT_CACHE(name, ALLOC_TYPE)             \
	INTDEF struct Dee_cache_struct *structcache_##name##_list; \
	INTDEF size_t structcache_##name##_size;                   \
	INTDEF size_t DCALL name##_clear(size_t max_clear);        \
	INTDEF void DCALL name##_free(ALLOC_TYPE *__restrict ob);  \
	INTDEF ALLOC_TYPE *(DCALL name##_alloc)(void);             \
	DEE_OBJECT_CACHE_IFDBG(                                    \
	INTDEF ALLOC_TYPE *DCALL name##_dbgalloc(char const *file, int line);)
#define DEE_DECLARE_OBJECT_CACHE(name, ALLOC_TYPE)                         \
	INTDEF struct Dee_cache_object *obcache_##name##_list;                 \
	INTDEF size_t obcache_##name##_size;                                   \
	INTDEF size_t DCALL name##_clear(size_t max_clear);                    \
	INTDEF void DCALL name##_free(ALLOC_TYPE *__restrict ob);              \
	INTDEF ALLOC_TYPE *(DCALL name##_alloc)(void);                         \
	DEE_OBJECT_CACHE_IFDBG(                                                \
	INTDEF ALLOC_TYPE *DCALL name##_dbgalloc(char const *file, int line);) \
	INTDEF void DCALL name##_tp_free(void *__restrict ob);                 \
	INTDEF void *DCALL name##_tp_alloc(void);
#define DEE_DEFINE_STRUCT_CACHE_EX(name, ALLOC_TYPE, object_size, limit)              \
	INTERN struct Dee_cache_struct *structcache_##name##_list = NULL;                 \
	INTERN size_t structcache_##name##_size                   = 0;                    \
	INTERN size_t DCALL name##_clear(size_t max_clear) {                              \
		size_t result = 0;                                                            \
		while (result < max_clear && structcache_##name##_list) {                     \
			struct Dee_cache_struct *iter;                                            \
			Dee_ASSERT(structcache_##name##_size != 0);                               \
			iter = structcache_##name##_list;                                         \
			structcache_##name##_list = iter->cs_next;                                \
			--structcache_##name##_size;                                              \
			result += object_size;                                                    \
			Dee_Free(iter);                                                           \
		}                                                                             \
		return result;                                                                \
	}                                                                                 \
	INTERN void DCALL name##_free(ALLOC_TYPE *__restrict ob) {                        \
		DEE_OBJECT_CACHE_IFDBG(memset(ob, 0xcc, object_size);)                        \
		if (structcache_##name##_size < limit) {                                      \
			++structcache_##name##_size;                                              \
			((struct Dee_cache_struct *)ob)->cs_next = structcache_##name##_list;     \
			structcache_##name##_list = (struct Dee_cache_struct *)ob;                \
		} else {                                                                      \
			Dee_Free(ob);                                                             \
		}                                                                             \
	}                                                                                 \
	INTERN ALLOC_TYPE *(DCALL name##_alloc)(void) {                                   \
		ALLOC_TYPE *result;                                                           \
		Dee_ASSERT((structcache_##name##_size != 0) ==                                \
		           (structcache_##name##_list != NULL));                              \
		result = (ALLOC_TYPE *)structcache_##name##_list;                             \
		if (result) {                                                                 \
			structcache_##name##_list = ((struct Dee_cache_struct *)result)->cs_next; \
			--structcache_##name##_size;                                              \
			DEE_OBJECT_CACHE_IFDBG(memset(result, 0xcc, object_size);)                \
		} else {                                                                      \
			result = (ALLOC_TYPE *)(Dee_Malloc)(object_size);                         \
		}                                                                             \
		return result;                                                                \
	}                                                                                 \
	DEE_OBJECT_CACHE_IFDBG(                                                           \
	INTERN ALLOC_TYPE *DCALL name##_dbgalloc(char const *file, int line) {            \
		ALLOC_TYPE *result;                                                           \
		/*Dee_ASSERT((structcache_##name##_size != 0) ==                              \
		             (structcache_##name##_list != NULL));                            \
		result = (ALLOC_TYPE *)structcache_##name##_list;                             \
		if (result) {                                                                 \
			structcache_##name##_list = ((struct Dee_cache_struct *)result)->cs_next; \
			--structcache_##name##_size;                                              \
			DEE_OBJECT_CACHE_IFDBG(memset(result, 0xcc, object_size);)                \
		} else*/ {                                                                    \
			result = (ALLOC_TYPE *)DeeDbg_Malloc(object_size, file, line);            \
		}                                                                             \
		return result;                                                                \
	})
#define DEE_DEFINE_OBJECT_CACHE_EX(name, ALLOC_TYPE, object_size, limit)             \
	INTERN struct Dee_cache_object *obcache_##name##_list = NULL;                    \
	INTERN size_t obcache_##name##_size                   = 0;                       \
	INTERN size_t DCALL name##_clear(size_t max_clear) {                             \
		size_t result = 0;                                                           \
		while (result < max_clear && obcache_##name##_list) {                        \
			struct Dee_cache_object *iter;                                           \
			Dee_ASSERT(obcache_##name##_size != 0);                                  \
			iter = obcache_##name##_list;                                            \
			obcache_##name##_list = iter->co_next;                                   \
			--obcache_##name##_size;                                                 \
			result += object_size;                                                   \
			DeeObject_Free(iter);                                                    \
		}                                                                            \
		return result;                                                               \
	}                                                                                \
	INTERN void DCALL name##_free(ALLOC_TYPE *__restrict ob) {                       \
		DEE_OBJECT_CACHE_IFDBG(memset(ob, 0xcc, object_size);)                       \
		if (obcache_##name##_size < limit) {                                         \
			++obcache_##name##_size;                                                 \
			((struct Dee_cache_object *)ob)->co_next = obcache_##name##_list;        \
			obcache_##name##_list = (struct Dee_cache_object *)ob;                   \
		} else {                                                                     \
			DeeObject_Free(ob);                                                      \
		}                                                                            \
	}                                                                                \
	INTERN ALLOC_TYPE *(DCALL name##_alloc)(void) {                                  \
		ALLOC_TYPE *result;                                                          \
		Dee_ASSERT((obcache_##name##_size != 0) == (obcache_##name##_list != NULL)); \
		result = (ALLOC_TYPE *)obcache_##name##_list;                                \
		if (result) {                                                                \
			obcache_##name##_list = ((struct Dee_cache_object *)result)->co_next;    \
			--obcache_##name##_size;                                                 \
			DEE_OBJECT_CACHE_IFDBG(memset(result, 0xcc, object_size);)               \
		} else {                                                                     \
			result = (ALLOC_TYPE *)(DeeObject_Malloc)(object_size);                  \
		}                                                                            \
		return result;                                                               \
	}                                                                                \
	DEE_OBJECT_CACHE_IFDBG(                                                          \
	INTERN ALLOC_TYPE *DCALL name##_dbgalloc(char const *file, int line) {           \
		ALLOC_TYPE *result;                                                          \
		/*Dee_ASSERT((obcache_##name##_size != 0) ==                                 \
		             (obcache_##name##_list != NULL));                               \
		result = (ALLOC_TYPE *)obcache_##name##_list;                                \
		if (result) {                                                                \
			obcache_##name##_list = ((struct Dee_cache_object *)result)->co_next;    \
			--obcache_##name##_size;                                                 \
			DEE_OBJECT_CACHE_IFDBG(memset(result, 0xcc, object_size);)               \
		} else*/ {                                                                   \
			result = (ALLOC_TYPE *)DeeDbgObject_Malloc(object_size, file, line);     \
		}                                                                            \
		return result;                                                               \
	})                                                                               \
	INTERN void DCALL name##_tp_free(void *__restrict ob) {                          \
		name##_free((ALLOC_TYPE *)ob);                                               \
	}                                                                                \
	INTERN void *DCALL name##_tp_alloc(void) {                                       \
		return (void *)(name##_alloc)();                                             \
	}
#endif /* CONFIG_NO_THREADS */



DECL_END

#endif /* !GUARD_DEEMON_UTIL_CACHE_H */
