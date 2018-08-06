/* Copyright (c) 2018 Griefer@Work                                            *
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
#ifndef GUARD_DEEMON_UTIL_CACHE_H
#define GUARD_DEEMON_UTIL_CACHE_H 1

#include "../api.h"
#ifndef CONFIG_NO_THREADS
#include "rwlock.h"
#endif
#include <stddef.h>
#include <string.h>

DECL_BEGIN

struct cache_struct {
    struct cache_struct *cs_next; /* [0..1][owned] Next cached structure. */
};
struct cache_object {
    struct cache_object *co_next; /* [0..1][owned] Next cached object. */
};

#define DEFINE_STRUCT_CACHE(name,alloc_type,limit) \
        DEFINE_STRUCT_CACHE_EX(name,alloc_type,sizeof(alloc_type),limit)
#define DEFINE_OBJECT_CACHE(name,alloc_type,limit) \
        DEFINE_OBJECT_CACHE_EX(name,alloc_type,sizeof(alloc_type),limit)

#ifndef NDEBUG
#define OBJECT_CACHE_IFDBG(x) x
#else
#define OBJECT_CACHE_IFDBG(x)
#endif

#ifndef CONFIG_NO_THREADS
#define DECLARE_STRUCT_CACHE(name,alloc_type) \
INTDEF rwlock_t             structcache_##name##_lock; \
INTDEF struct cache_struct *structcache_##name##_list; \
INTDEF size_t               structcache_##name##_size; \
INTDEF size_t DCALL name##_clear(size_t max_clear); \
INTDEF void DCALL name##_free(alloc_type *__restrict ob); \
INTDEF alloc_type *(DCALL name##_alloc)(void); \
OBJECT_CACHE_IFDBG( \
INTDEF alloc_type *DCALL name##_dbgalloc(char const *file, int line);)
#define DECLARE_OBJECT_CACHE(name,alloc_type) \
INTDEF rwlock_t             obcache_##name##_lock; \
INTDEF struct cache_object *obcache_##name##_list; \
INTDEF size_t               obcache_##name##_size; \
INTDEF size_t DCALL name##_clear(size_t max_clear); \
INTDEF void DCALL name##_free(alloc_type *__restrict ob); \
INTDEF alloc_type *(DCALL name##_alloc)(void); \
OBJECT_CACHE_IFDBG( \
INTDEF alloc_type *DCALL name##_dbgalloc(char const *file, int line);) \
INTDEF void DCALL name##_tp_free(void *__restrict ob); \
INTDEF void *DCALL name##_tp_alloc(void);
#define DEFINE_STRUCT_CACHE_EX(name,alloc_type,object_size,limit) \
INTERN rwlock_t             structcache_##name##_lock = RWLOCK_INIT; \
INTERN struct cache_struct *structcache_##name##_list = NULL; \
INTERN size_t               structcache_##name##_size = 0; \
INTERN size_t DCALL name##_clear(size_t max_clear) \
{ \
    size_t result = 0; \
    rwlock_write(&structcache_##name##_lock); \
    while (result < max_clear && structcache_##name##_list) \
    { \
        struct cache_struct *iter; \
        ASSERT(structcache_##name##_size != 0); \
        iter = structcache_##name##_list; \
        structcache_##name##_list = iter->cs_next; \
        --structcache_##name##_size; \
        result += object_size; \
        Dee_Free(iter); \
    } \
    rwlock_endwrite(&structcache_##name##_lock); \
    return result; \
} \
INTERN void DCALL name##_free(alloc_type *__restrict ob) \
{ \
    rwlock_write(&structcache_##name##_lock); \
    if (structcache_##name##_size < limit) { \
      ++structcache_##name##_size; \
      ((struct cache_struct *)ob)->cs_next = structcache_##name##_list; \
        structcache_##name##_list = (struct cache_struct *)ob; \
        rwlock_endwrite(&structcache_##name##_lock); \
    } else { \
        rwlock_endwrite(&structcache_##name##_lock); \
        Dee_Free(ob); \
    } \
} \
INTERN alloc_type *(DCALL name##_alloc)(void) \
{ \
    alloc_type *result; \
    rwlock_write(&structcache_##name##_lock); \
    ASSERT((structcache_##name##_size != 0) == (structcache_##name##_list != NULL)); \
    result = (alloc_type *)structcache_##name##_list; \
    if (result) { \
        structcache_##name##_list = ((struct cache_struct *)result)->cs_next; \
      --structcache_##name##_size; \
    } \
    rwlock_endwrite(&structcache_##name##_lock); \
    if (!result) \
         result = (alloc_type *)(Dee_Malloc)(object_size); \
    OBJECT_CACHE_IFDBG(else memset(result,0xcc,object_size);) \
    return result; \
} \
OBJECT_CACHE_IFDBG( \
INTERN alloc_type *DCALL name##_dbgalloc(char const *file, int line) \
{ \
    alloc_type *result; \
    /*rwlock_write(&structcache_##name##_lock); \
    ASSERT((structcache_##name##_size != 0) == (structcache_##name##_list != NULL)); \
    result = (alloc_type *)structcache_##name##_list; \
    if (result) { \
        structcache_##name##_list = ((struct cache_struct *)result)->cs_next; \
      --structcache_##name##_size; \
    } \
    rwlock_endwrite(&structcache_##name##_lock); \
    if (!result)*/ \
         result = (alloc_type *)DeeDbg_Malloc(object_size,file,line); \
    /*OBJECT_CACHE_IFDBG(else memset(result,0xcc,object_size);)*/ \
    return result; \
})
#define DEFINE_OBJECT_CACHE_EX(name,alloc_type,object_size,limit) \
INTERN rwlock_t             obcache_##name##_lock = RWLOCK_INIT; \
INTERN struct cache_object *obcache_##name##_list = NULL; \
INTERN size_t               obcache_##name##_size = 0; \
INTERN size_t DCALL name##_clear(size_t max_clear) \
{ \
    size_t result = 0; \
    rwlock_write(&obcache_##name##_lock); \
    while (result < max_clear && obcache_##name##_list) \
    { \
        struct cache_object *iter; \
        ASSERT(obcache_##name##_size != 0); \
        iter = obcache_##name##_list; \
        obcache_##name##_list = iter->co_next; \
        --obcache_##name##_size; \
        result += object_size; \
        DeeObject_Free(iter); \
    } \
    rwlock_endwrite(&obcache_##name##_lock); \
    return result; \
} \
INTERN void DCALL name##_free(alloc_type *__restrict ob) \
{ \
    rwlock_write(&obcache_##name##_lock); \
    if (obcache_##name##_size < limit) { \
      ++obcache_##name##_size; \
      ((struct cache_object *)ob)->co_next = obcache_##name##_list; \
        obcache_##name##_list = (struct cache_object *)ob; \
        rwlock_endwrite(&obcache_##name##_lock); \
    } else { \
        rwlock_endwrite(&obcache_##name##_lock); \
        DeeObject_Free(ob); \
    } \
} \
INTERN alloc_type *(DCALL name##_alloc)(void) \
{ \
    alloc_type *result; \
    rwlock_write(&obcache_##name##_lock); \
    ASSERT((obcache_##name##_size != 0) == (obcache_##name##_list != NULL)); \
    result = (alloc_type *)obcache_##name##_list; \
    if (result) { \
        obcache_##name##_list = ((struct cache_object *)result)->co_next; \
      --obcache_##name##_size; \
    } \
    rwlock_endwrite(&obcache_##name##_lock); \
    if (!result) \
         result = (alloc_type *)(DeeObject_Malloc)(object_size); \
    OBJECT_CACHE_IFDBG(else memset(result,0xcc,object_size);) \
    return result; \
} \
OBJECT_CACHE_IFDBG( \
INTERN alloc_type *DCALL name##_dbgalloc(char const *file, int line) \
{ \
    alloc_type *result; \
    /*rwlock_write(&obcache_##name##_lock); \
    ASSERT((obcache_##name##_size != 0) == (obcache_##name##_list != NULL)); \
    result = (alloc_type *)obcache_##name##_list; \
    if (result) { \
        obcache_##name##_list = ((struct cache_object *)result)->co_next; \
      --obcache_##name##_size; \
    } \
    rwlock_endwrite(&obcache_##name##_lock); \
    if (!result)*/ \
         result = (alloc_type *)DeeDbgObject_Malloc(object_size,file,line); \
    /*OBJECT_CACHE_IFDBG(else memset(result,0xcc,object_size);)*/ \
    return result; \
}) \
INTERN void DCALL name##_tp_free(void *__restrict ob) \
{ \
    name##_free((alloc_type *)ob); \
} \
INTERN void *DCALL name##_tp_alloc(void) \
{ \
    return (void *)(name##_alloc)(); \
}
#else /* !CONFIG_NO_THREADS */
#define DECLARE_STRUCT_CACHE(name,alloc_type) \
INTDEF struct cache_struct *structcache_##name##_list; \
INTDEF size_t               structcache_##name##_size; \
INTDEF size_t DCALL name##_clear(size_t max_clear); \
INTDEF void DCALL name##_free(alloc_type *__restrict ob); \
INTDEF alloc_type *(DCALL name##_alloc)(void); \
OBJECT_CACHE_IFDBG( \
INTDEF alloc_type *DCALL name##_dbgalloc(char const *file, int line);)
#define DECLARE_OBJECT_CACHE(name,alloc_type) \
INTDEF struct cache_object *obcache_##name##_list; \
INTDEF size_t               obcache_##name##_size; \
INTDEF size_t DCALL name##_clear(size_t max_clear); \
INTDEF void DCALL name##_free(alloc_type *__restrict ob); \
INTDEF alloc_type *(DCALL name##_alloc)(void); \
OBJECT_CACHE_IFDBG( \
INTDEF alloc_type *DCALL name##_dbgalloc(char const *file, int line);) \
INTDEF void DCALL name##_tp_free(void *__restrict ob); \
INTDEF void *DCALL name##_tp_alloc(void);
#define DEFINE_STRUCT_CACHE_EX(name,alloc_type,object_size,limit) \
INTERN struct cache_struct *structcache_##name##_list = NULL; \
INTERN size_t               structcache_##name##_size = 0; \
INTERN size_t DCALL name##_clear(size_t max_clear) \
{ \
    size_t result = 0; \
    while (result < max_clear && structcache_##name##_list) \
    { \
        struct cache_struct *iter; \
        ASSERT(structcache_##name##_size != 0); \
        iter = structcache_##name##_list; \
        structcache_##name##_list = iter->cs_next; \
        --structcache_##name##_size; \
        result += object_size; \
        Dee_Free(iter); \
    } \
    return result; \
} \
INTERN void DCALL name##_free(alloc_type *__restrict ob) \
{ \
    if (structcache_##name##_size < limit) { \
      ++structcache_##name##_size; \
      ((struct cache_struct *)ob)->cs_next = structcache_##name##_list; \
        structcache_##name##_list = (struct cache_struct *)ob; \
    } else { \
        Dee_Free(ob); \
    } \
} \
INTERN alloc_type *(DCALL name##_alloc)(void) \
{ \
    alloc_type *result; \
    ASSERT((structcache_##name##_size != 0) == (structcache_##name##_list != NULL)); \
    result = (alloc_type *)structcache_##name##_list; \
    if (result) { \
        structcache_##name##_list = ((struct cache_struct *)result)->cs_next; \
      --structcache_##name##_size; \
        OBJECT_CACHE_IFDBG(memset(result,0xcc,object_size);) \
    } else { \
         result = (alloc_type *)(Dee_Malloc)(object_size); \
    } \
    return result; \
} \
OBJECT_CACHE_IFDBG( \
INTERN alloc_type *DCALL name##_dbgalloc(char const *file, int line) \
{ \
    alloc_type *result; \
    /*ASSERT((structcache_##name##_size != 0) == (structcache_##name##_list != NULL)); \
    result = (alloc_type *)structcache_##name##_list; \
    if (result) { \
        structcache_##name##_list = ((struct cache_struct *)result)->cs_next; \
      --structcache_##name##_size; \
        OBJECT_CACHE_IFDBG(memset(result,0xcc,object_size);) \
    } else*/ { \
         result = (alloc_type *)DeeDbg_Malloc(object_size,file,line); \
    } \
    return result; \
})
#define DEFINE_OBJECT_CACHE_EX(name,alloc_type,object_size,limit) \
INTERN struct cache_object *obcache_##name##_list = NULL; \
INTERN size_t               obcache_##name##_size = 0; \
INTERN size_t DCALL name##_clear(size_t max_clear) \
{ \
    size_t result = 0; \
    while (result < max_clear && obcache_##name##_list) \
    { \
        struct cache_object *iter; \
        ASSERT(obcache_##name##_size != 0); \
        iter = obcache_##name##_list; \
        obcache_##name##_list = iter->co_next; \
        --obcache_##name##_size; \
        result += object_size; \
        DeeObject_Free(iter); \
    } \
    return result; \
} \
INTERN void DCALL name##_free(alloc_type *__restrict ob) \
{ \
    if (obcache_##name##_size < limit) { \
      ++obcache_##name##_size; \
      ((struct cache_object *)ob)->co_next = obcache_##name##_list; \
        obcache_##name##_list = (struct cache_object *)ob; \
    } else { \
        DeeObject_Free(ob); \
    } \
} \
INTERN alloc_type *(DCALL name##_alloc)(void) \
{ \
    alloc_type *result; \
    ASSERT((obcache_##name##_size != 0) == (obcache_##name##_list != NULL)); \
    result = (alloc_type *)obcache_##name##_list; \
    if (result) { \
        obcache_##name##_list = ((struct cache_object *)result)->co_next; \
      --obcache_##name##_size; \
        OBJECT_CACHE_IFDBG(memset(result,0xcc,object_size);) \
    } else { \
        result = (alloc_type *)(DeeObject_Malloc)(object_size); \
    } \
    return result; \
} \
OBJECT_CACHE_IFDBG( \
INTERN alloc_type *DCALL name##_dbgalloc(char const *file, int line) \
{ \
    alloc_type *result; \
    /*ASSERT((obcache_##name##_size != 0) == (obcache_##name##_list != NULL)); \
    result = (alloc_type *)obcache_##name##_list; \
    if (result) { \
        obcache_##name##_list = ((struct cache_object *)result)->co_next; \
      --obcache_##name##_size; \
        OBJECT_CACHE_IFDBG(memset(result,0xcc,object_size);) \
    } else*/ { \
        result = (alloc_type *)DeeDbgObject_Malloc(object_size,file,line); \
    } \
    return result; \
}) \
INTERN void DCALL name##_tp_free(void *__restrict ob) \
{ \
    name##_free((alloc_type *)ob); \
} \
INTERN void *DCALL name##_tp_alloc(void) \
{ \
    return (void *)(name##_alloc)(); \
}
#endif /* CONFIG_NO_THREADS */



DECL_END

#endif /* !GUARD_DEEMON_UTIL_CACHE_H */
