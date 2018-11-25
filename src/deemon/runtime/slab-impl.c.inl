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
#ifdef __INTELLISENSE__
#include "slab.c.inl"
#define SIZE         4
#define NEXT_LARGER  5
#endif

#define FUNC3(x,y) x ## y
#define FUNC2(x,y) FUNC3(x,y)
#define FUNC(x) FUNC2(x,SIZE)

DECL_BEGIN

#if !defined(CONFIG_NO_OBJECT_SLABS) && 0

PUBLIC WUNUSED ATTR_MALLOC void *
(DCALL FUNC(DeeObject_SlabMalloc))(void) {
 /* TODO */
}
PUBLIC WUNUSED ATTR_MALLOC void *
(DCALL FUNC(DeeObject_SlabCalloc))(void) {
 /* TODO */
}
PUBLIC WUNUSED ATTR_MALLOC void *
(DCALL FUNC(DeeObject_SlabTryMalloc))(void) {
 /* TODO */
}
PUBLIC WUNUSED ATTR_MALLOC void *
(DCALL FUNC(DeeObject_SlabTryCalloc))(void) {
 /* TODO */
}
PUBLIC WUNUSED ATTR_MALLOC void *
(DCALL FUNC(DeeDbgObject_SlabMalloc))(char const *file, int line) {
 /* TODO */
}
PUBLIC WUNUSED ATTR_MALLOC void *
(DCALL FUNC(DeeDbgObject_SlabCalloc))(char const *file, int line) {
 /* TODO */
}
PUBLIC WUNUSED ATTR_MALLOC void *
(DCALL FUNC(DeeDbgObject_SlabTryMalloc))(char const *file, int line) {
 /* TODO */
}
PUBLIC WUNUSED ATTR_MALLOC void *
(DCALL FUNC(DeeDbgObject_SlabTryCalloc))(char const *file, int line) {
 /* TODO */
}

PUBLIC void
(DCALL FUNC(DeeObject_SlabFree))(void *__restrict ptr) {
 /* TODO */
}
PUBLIC void
(DCALL FUNC(DeeDbgObject_SlabFree))(void *__restrict ptr,
                                            char const *file, int line) {
 /* TODO */
}

#else /* !CONFIG_NO_OBJECT_SLABS */
#define MY_SLAB_IS_DISABLED 1
PUBLIC WUNUSED ATTR_MALLOC void *
(DCALL FUNC(DeeObject_SlabMalloc))(void) {
 return (Dee_Malloc)(SIZE * sizeof(void *));
}
PUBLIC WUNUSED ATTR_MALLOC void *
(DCALL FUNC(DeeObject_SlabCalloc))(void) {
 return (Dee_Calloc)(SIZE * sizeof(void *));
}
PUBLIC WUNUSED ATTR_MALLOC void *
(DCALL FUNC(DeeObject_SlabTryMalloc))(void) {
 return (Dee_TryMalloc)(SIZE * sizeof(void *));
}
PUBLIC WUNUSED ATTR_MALLOC void *
(DCALL FUNC(DeeObject_SlabTryCalloc))(void) {
 return (Dee_TryCalloc)(SIZE * sizeof(void *));
}
PUBLIC WUNUSED ATTR_MALLOC void *
(DCALL FUNC(DeeDbgObject_SlabMalloc))(char const *file, int line) {
 return (DeeDbg_Malloc)(SIZE * sizeof(void *),file,line);
}
PUBLIC WUNUSED ATTR_MALLOC void *
(DCALL FUNC(DeeDbgObject_SlabCalloc))(char const *file, int line) {
 return (DeeDbg_Calloc)(SIZE * sizeof(void *),file,line);
}
PUBLIC WUNUSED ATTR_MALLOC void *
(DCALL FUNC(DeeDbgObject_SlabTryMalloc))(char const *file, int line) {
 return (DeeDbg_TryMalloc)(SIZE * sizeof(void *),file,line);
}
PUBLIC WUNUSED ATTR_MALLOC void *
(DCALL FUNC(DeeDbgObject_SlabTryCalloc))(char const *file, int line) {
 return (DeeDbg_TryCalloc)(SIZE * sizeof(void *),file,line);
}
#ifndef __NO_DEFINE_ALIAS
DEFINE_PUBLIC_ALIAS(ASSEMBLY_NAME(FUNC(DeeObject_SlabFree),4),
                    ASSEMBLY_NAME(Dee_Free,4));
DEFINE_PUBLIC_ALIAS(ASSEMBLY_NAME(FUNC(DeeDbgObject_SlabFree),12),
                    ASSEMBLY_NAME(DeeDbg_Free,12));
#else /* !__NO_DEFINE_ALIAS */
PUBLIC void (DCALL FUNC(DeeObject_SlabFree))(void *__restrict ptr) { (Dee_Free)(ptr); }
PUBLIC void (DCALL FUNC(DeeDbgObject_SlabFree))(void *__restrict ptr, char const *file, int line) { (DeeDbg_Free)(ptr,file,line); }
#endif /* __NO_DEFINE_ALIAS */
#endif /* CONFIG_NO_OBJECT_SLABS */


#if SIZE == DEEMON_SLAB_MINSIZE
#ifndef MY_SLAB_IS_DISABLED
#ifndef __NO_DEFINE_ALIAS
DEFINE_PUBLIC_ALIAS(ASSEMBLY_NAME(DeeObject_Free,4),
                    ASSEMBLY_NAME(FUNC(DeeObject_SlabFree),4));
DEFINE_PUBLIC_ALIAS(ASSEMBLY_NAME(DeeDbgObject_Free,12),
                    ASSEMBLY_NAME(FUNC(DeeDbgObject_SlabFree),12));
#else
PUBLIC void (DCALL DeeObject_Free)(void *ptr) { (FUNC(DeeObject_SlabFree))(ptr); }
PUBLIC void (DCALL DeeDbgObject_Free)(void *ptr, char const *file, int line) { (FUNC(DeeDbgObject_SlabFree))(ptr,file,line); }
#endif
#else /* !MY_SLAB_IS_DISABLED */
#ifndef __NO_DEFINE_ALIAS
DEFINE_PUBLIC_ALIAS(ASSEMBLY_NAME(DeeObject_Free,4),
                    ASSEMBLY_NAME(Dee_Free,4));
DEFINE_PUBLIC_ALIAS(ASSEMBLY_NAME(DeeDbgObject_Free,12),
                    ASSEMBLY_NAME(DeeDbg_Free,12));
#else
PUBLIC void (DCALL DeeObject_Free)(void *ptr) { Dee_Free(ptr); }
PUBLIC void (DCALL DeeDbgObject_Free)(void *ptr, char const *file, int line) { DeeDbg_Free(ptr,file,line); }
#endif
#endif /* MY_SLAB_IS_DISABLED */
#endif /* SIZE == DEEMON_SLAB_MINSIZE */

DECL_END

#undef MY_SLAB_IS_DISABLED
#undef NEXT_LARGER
#undef SIZE
