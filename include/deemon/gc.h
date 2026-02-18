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
/*!export DeeDbgGCObject_*alloc**/
/*!export DeeDbgGCObject_Try*alloc**/
/*!export DeeGCObject_*alloc**/
/*!export DeeGCObject_*ALLOC**/
/*!export DeeGCObject_Try*alloc**/
/*!export DeeGCObject_TRY*ALLOC**/
/*!export Dee_GC_**/
/*!export DeeGC_**/
/*!export _Dee_GC_HEAD_UNTRACKED_**/
/*!export -_Dee_PRIVATE_**/
#ifndef GUARD_DEEMON_GC_H
#define GUARD_DEEMON_GC_H 1 /*!export-*/

#include "api.h"

#include <hybrid/typecore.h> /* __SIZEOF_POINTER__ */

#include "types.h" /* DREF, DeeObject, Dee_AsObject */

#include <stdbool.h> /* bool */
#include <stddef.h>  /* NULL, size_t */
#include <stdint.h>  /* UINT32_C, UINT64_C, uintptr_t */

#ifndef __INTELLISENSE__
#include "alloc.h" /* CONFIG_FIXED_ALLOCATOR_S_IS_AUTO, DeeSlab_ENUMERATE, DeeSlab_Invoke */
#else /* !__INTELLISENSE__ */
DECL_BEGIN
#define _Dee_MalloccBufsize(elem_count, elem_size)                              ((elem_count) * (elem_size))                             /*!export-*/
#define _Dee_MallococBufsize(base_offset, elem_count, elem_size)                ((base_offset) + ((elem_count) * (elem_size)))           /*!export-*/
#define _Dee_MalloccBufsizeDbg(elem_count, elem_size, file, line)               _Dee_MalloccBufsize(elem_count, elem_size)               /*!export-*/
#define _Dee_MallococBufsizeDbg(base_offset, elem_count, elem_size, file, line) _Dee_MallococBufsize(base_offset, elem_count, elem_size) /*!export-*/
LOCAL ATTR_CONST WUNUSED size_t DCALL _Dee_MalloccBufsizeSafe(size_t elem_count, size_t elem_size);                                      /*!export-*/
LOCAL ATTR_CONST WUNUSED size_t DCALL _Dee_MallococBufsizeSafe(size_t base_offset, size_t elem_count, size_t elem_size);                 /*!export-*/
DECL_END
#endif /* __INTELLISENSE__ */

DECL_BEGIN

#ifdef CONFIG_EXPERIMENTAL_REWORKED_GC
/* Head of an object that is GC-able.
 *
 * GC objects are managed via generations:
 * - Whenever a generation becomes larger than a configurable threshold:
 *   - for every object of the generation: "gc_info.gi_refs = 0, gc_next |= 1"
 *   - for every object of the generation: tp_visit recursive, handle reachable
 *     GC objects "r" as: "if (r.gc_next & 1) { ++r.gc_info.gi_refs; }"
 *   - for every object of the generation "r":
 *     if (r.gc_info.gi_refs >= r.gc_self.ob_refcnt) {
 *         DeeObject_Clear(&r.gc_self);
 *         RE_INSERT_INTO_SAME_GENERATION(&r); // Also resets "gi_pself" and "gc_next"
 *     } else {
 *         INSERT_INTO_NEXT_GENERATION(&r);    // Also resets "gi_pself" and "gc_next"
 *     }
 *
 * gc.collect() mock:
 * >>again:
 * >> GC_LOCK(); // Shared lock -- errors are propagated
 * >> struct Dee_gc_head *iter, **p_iter;
 * >> struct Dee_gc_head *generation = TAKE_ALL_OBJECTS_FROM_GENERATION();
 * >> struct Dee_gc_head *unreachable_fast; // w/o user-defined "~this"
 * >> struct Dee_gc_head *unreachable_slow; // w user-defined "~this"
 * >> // Must suspend all threads that could potentially modify reference counts:
 * >> //           >> GC1(ob_refcnt=1)        GC2(ob_refcnt=1)
 * >> //      Thread #1: start GC enum: read GC1.ob_refcnt=1, GC2.ob_refcnt=1  (references owned by Thread #2)
 * >> //      Thread #2: add references between GC1 and GC2:
 * >> //           >> GC1(ob_refcnt=2) <=x2=> GC2(ob_refcnt=2)
 * >> //      Thread #1: tp_visit(GC1), tp_visit(GC1) --> GC1.gi_refs=1, GC2.gi_refs=1
 * >> //                 --> discovered that each object references the other
 * >> //      Thread #2: remove references between GC1 and GC2:
 * >> //           >> GC1(ob_refcnt=1)        GC2(ob_refcnt=1)
 * >> //      Thread #1: Notices that GC1/GC2 both have ob_refcnt=1 and gi_refs=1,
 * >> //                 even though that's only the case because Thread #2 changed
 * >> //                 their reference counts during some very inconvenient moments
 * >> // NOTE: Instead of "DeeThread_SuspendAll", could also use OS-specifics like
 * >> //       "pthread_suspend_all()" here! (this doesn't need to be a synchronous
 * >> //       suspend!)
 * >> DeeThread_SuspendAll(); // Errors are propagated
 * >> for (iter = generation; iter; iter = iter->gc_next & ~3) {
 * >>     iter->gc_info.gi_refs = 0;
 * >>     iter->gc_next |= 1;
 * >> }
 * >> for (iter = generation; iter; iter = iter->gc_next & ~3) {
 * >>     DeeObject_Visit(&iter->gc_self, (DeeObject *reachable) -> {
 * >>         if (DeeType_IsGC(Dee_TYPE(reachable))) {
 * >>             struct Dee_gc_head *head = headof(reachable);
 * >>             if (head->gc_next & 1)
 * >>                 ++head->gc_info.gi_refs;
 * >>         } else if (!ALREADY_VISITED_DURING_THIS_SCAN(reachable)) {
 * >>             // XXX: ^^How can this be represented?
 * >>             //      We mustn't visit the same non-GC object multiple times:
 * >>             //        GC1 -> NON_GC <- GC2
 * >>             //         ^       |        ^
 * >>             //         |       v        |
 * >>             //         +----- GC3 ------+
 * >>             //                 ^
 * >>             //                 |
 * >>             //         EXTERNALLY_VISIBLE
 * >>             // When "NON_GC" were to be visited twice via GC1 and GC2, then
 * >>             // the "GC3.gi_refs = 2", which would match its actual reference
 * >>             // count (as GC3 is referenced 2x by NON_GC and EXTERNALLY_VISIBLE)
 * >>             // But that would be wrong, since GC3 is "EXTERNALLY_VISIBLE".
 * >>             //
 * >>             // I don't really wanna have to use a heap-based bitset here,
 * >>             // since thus far: this new GC collect impl doesn't need heap
 * >>             // memory. But I can't see any other way thus far...
 * >>
 * >>             RECURSE(reachable); // Recurse on non-GC objects (e.g. "Tuple")
 * >>             // XXX: Could even limit the search-depth here based on the GC
 * >>             //      generation index. iow: max scan depth could just equal
 * >>             //      the generation's index (except for the last generation,
 * >>             //      which needs infinite depth)
 * >>             // -> By limiting ourselves here, the scan is faster, and GC
 * >>             //    objects with long self-reference-loops simply take longer
 * >>             //    before they're actually deleted (which is fine!)
 * >>         }
 * >>     });
 * >> }
 * >> for (iter = generation, unreachable_fast = unreachable_slow = NULL; iter;) {
 * >>     struct Dee_gc_head *next = iter->gc_next & ~3;
 * >>     ASSERT(iter->gc_info.gi_refs <= iter->gc_self.ob_refcnt);
 * >>     if (iter->gc_info.gi_refs >= iter->gc_self.ob_refcnt) {
 * >>         // Unreachable
 * >>         struct Dee_gc_head **p_unreachable =
 * >>             !(iter->gc_next & 2) && // vvv aka: has user-defined "~this"
 * >>             HAS_REVIVING_DESTRUCTOR(iter->gc_self.ob_type)
 * >>             ? &unreachable_slow
 * >>             : &unreachable_fast;
 * >>         iter->gc_next = (iter->gc_next & 2) | *p_unreachable;
 * >>         *p_unreachable = iter;
 * >>     } else {
 * >>         ATOMIC_INSERT_OBJECT_INTO_GENERATION(NEXT_GENERATION, iter);
 * >>     }
 * >>     iter = next;
 * >> }
 * >>
 * >> // Deal with "unreachable_slow" (which must be resolved before anything else can happen)
 * >> if (unreachable_slow) {
 * >>     // Restore 
 * >>     if (unreachable_fast)
 * >>         ATOMIC_INSERT_ALL_OBJECTS_INTO_GENERATION(FIRST_GENERATION, unreachable_fast);
 * >>     for (p_iter = &unreachable_slow; (iter = *p_iter & ~3) != NULL; p_iter = &iter->gc_next) {
 * >>         iter->gc_info.gi_pself = p_iter;
 * >>         ++iter->gc_self.ob_refcnt;
 * >>     }
 * >>     DeeThread_ResumeAll();
 * >>     GC_UNLOCK();
 * >>     for (iter = atomic_read(&unreachable_slow); iter;) {
 * >>         struct Dee_gc_head *next = atomic_read(&iter->gc_next) & ~3;
 * >>         if (!(atomic_fetchor(&iter->gc_next, 2) & 2))
 * >>             INVOKE_REVIVING_DESTRUCTOR(&iter->gc_self);
 * >>         Dee_Decref(&iter->gc_self);
 * >>         iter = next;
 * >>     }
 * >>     ATOMIC_INSERT_ALL_OBJECTS_INTO_GENERATION(CURRENT_GENERATION, unreachable_slow);
 * >>     goto again;
 * >> }
 * >>
 * >> // Kill all weak references to objects that hold references to unreachable GC objects
 * >> for (iter = unreachable_fast; iter; iter = iter->gc_next & ~3) {
 * >>     KILL_WEAKREFS_IF_DEFINED(&iter->gc_self.ob_weakrefs);
 * >>     DeeObject_Visit(&iter->gc_self, (DeeObject *reachable) -> {
 * >>         if (DeeType_IsGC(Dee_TYPE(reachable))) {
 * >>             struct Dee_gc_head *head = headof(reachable);
 * >>             if (head->gc_next & 1)
 * >>                 return MUST_KILL_CALLER_WEAKREFS;
 * >>         } else if (MAYBE_NOT_ALREADY_VISITED_DURING_THIS_WEAKREF_KILL(reachable)) {
 * >>             if (RECURSE(reachable) == MUST_KILL_CALLER_WEAKREFS) {
 * >>                 KILL_WEAKREFS_IF_DEFINED(&iter->gc_self.ob_weakrefs);
 * >>                 return MUST_KILL_CALLER_WEAKREFS;
 * >>             }
 * >>         }
 * >>         return DONT_KILL_CALLER_WEAKREFS;
 * >>     });
 * >> }
 * >>
 * >> // Prevent objects from being destroyed in some other way (since we're about to unlock)
 * >> // NOTE: Also repair the "gi_pself" link such that "DeeGC_Untrack()" works (though instead
 * >> //       of unlinking from some specific generation, it unlinks items from our on-stack
 * >> //       list of objects)
 * >> for (p_iter = &unreachable_fast; (iter = *p_iter & ~3) != NULL; p_iter = &iter->gc_next) {
 * >>     iter->gc_info.gi_pself = p_iter;
 * >>     ++iter->gc_self.ob_refcnt;
 * >> }
 * >> DeeThread_ResumeAll();
 * >> GC_UNLOCK();
 * >>
 * >> for (iter = atomic_read(&unreachable_fast); iter;) {
 * >>     struct Dee_gc_head *next = atomic_read(&iter->gc_next) & ~3;
 * >>     DeeObject_Clear(&iter->gc_self);
 * >>     Dee_Decref(&iter->gc_self);
 * >>     iter = next;
 * >> }
 * >>
 * >> if unlikely(unreachable_fast) {
 * >>     // This really shouldn't happen (and probably indicates a programming error):
 * >>     // GC objects from a reference loop that we identified as being self-contained
 * >>     // are still alive, even after being closed. (here: handle this by re-inserting
 * >>     // those objects into the original generation)
 * >>     ATOMIC_INSERT_ALL_OBJECTS_INTO_GENERATION(FIRST_GENERATION, unreachable_fast);
 * >> }
 *
 * Requirements:
 * - Instead of having a type flag "TP_FMAYREVIVE", there needs to be a new operator
 *   "tp_finalize" that lives alongside "tp_dtor" and "tp_destroy" and (if defined)
 *   gets called by "tp_destroy" prior to the object actually being destroyed.
 * - For GC objects, this operator only gets called when "!(Dee_gc_head::gc_next & 2)"
 * - Only this operator is allowed to revive an already-destroyed object ("tp_dtor"
 *   isn't allowed to do so anymore)
 */
struct Dee_gc_head {
	union {
		Dee_refcnt_t gi_refs;  /* [valid_if(GC_COLLECTING)] # of references account-for by other GC objects */
		DeeObject  **gi_pself; /* [1..1][== self][1..1][lock(INTERNAL(gc_lock))][valid_if(...)] Self-pointer in GC generation. */
	}                gc_info;  /* Misc. data, pending on context */
	DeeObject       *gc_next;  /* [0..1][lock(INTERNAL(gc_lock))] Next GC object (possibly in pending chain).
	                            * Least significant 2 bits here have special meaning:
	                            * - (gc_next & 1) == 0: Default
	                            * - (gc_next & 1) == 1: Object is part of a generation that is currently
	                            *                       being scanned, meaning that tp_visit must increment
	                            *                       "gc_info.gi_refs"
	                            * - (gc_next & 2) == 0: Default
	                            * - (gc_next & 2) == 1: A user-defined "~this" has already been invoked */
	DeeObject        gc_self;  /* The object that is being controlled by the GC. */
};
#else /* CONFIG_EXPERIMENTAL_REWORKED_GC */
struct Dee_gc_head;
struct Dee_gc_head_link {
	/* The structure that is prefixed before every GC-allocated object. */
	struct Dee_gc_head  *gc_next;   /* [0..1][lock(INTERNAL(gc_lock))] Next GC object. */
	struct Dee_gc_head **gc_pself;  /* [1..1][== self][1..1][lock(INTERNAL(gc_lock))] Self-pointer in the global chain of GC objects. */
};

struct Dee_gc_head {
	struct Dee_gc_head  *gc_next;   /* [0..1][lock(INTERNAL(gc_lock))] Next GC object. */
	struct Dee_gc_head **gc_pself;  /* [1..1][== self][1..1][lock(INTERNAL(gc_lock))] Self-pointer in the global chain of GC objects. */
	DeeObject            gc_object; /* The object that is being controlled by the GC. */
};
#endif /* !CONFIG_EXPERIMENTAL_REWORKED_GC */

#ifndef NDEBUG
#if __SIZEOF_POINTER__ == 4
#define _Dee_GC_HEAD_UNTRACKED_MARKER ((struct Dee_gc_head **)UINT32_C(0xcccccccc))
#elif __SIZEOF_POINTER__ == 8
#define _Dee_GC_HEAD_UNTRACKED_MARKER ((struct Dee_gc_head **)UINT64_C(0xcccccccccccccccc))
#endif /* __SIZEOF_POINTER__ == ... */
#endif /* !NDEBUG */

#ifdef _Dee_GC_HEAD_UNTRACKED_MARKER
#define _Dee_GC_HEAD_UNTRACKED_INIT NULL, _Dee_GC_HEAD_UNTRACKED_MARKER
#else /* _Dee_GC_HEAD_UNTRACKED_MARKER */
#define _Dee_GC_HEAD_UNTRACKED_INIT NULL, NULL
#endif /* !_Dee_GC_HEAD_UNTRACKED_MARKER */



#define Dee_GC_OBJECT_OFFSET COMPILER_OFFSETOF(struct Dee_gc_head, gc_object)
#define Dee_GC_HEAD_SIZE     COMPILER_OFFSETOF(struct Dee_gc_head, gc_object)
#define DeeGC_Head(ob)       ((struct Dee_gc_head *)((uintptr_t)Dee_AsObject(ob) - Dee_GC_OBJECT_OFFSET))
#define DeeGC_Object(ob)     (&(ob)->gc_object)

/* Begin/end tracking a given GC-allocated object.
 * `DeeGC_Track()' must be called explicitly when the object
 * has been allocated using `DeeGCObject_Malloc' and friends,
 * though constructions of non-variadic GC objects don't need
 * to call this function on the passed object. - That call will
 * automatically be done when the function returns successfully.
 * @return: * : == ob */
DFUNDEF ATTR_RETNONNULL NONNULL((1)) DeeObject *DCALL DeeGC_Track(DeeObject *__restrict ob);
DFUNDEF ATTR_RETNONNULL NONNULL((1)) DeeObject *DCALL DeeGC_Untrack(DeeObject *__restrict ob);

/* Track all GC objects in range [first,last], all of which have
 * already been linked together using their `struct Dee_gc_head_link' */
DFUNDEF WUNUSED bool DCALL DeeGC_TrackMany_TryLock(void);
DFUNDEF void DCALL DeeGC_TrackMany_Lock(void);
DFUNDEF NONNULL((1, 2)) void DCALL DeeGC_TrackMany_Exec(DeeObject *first, DeeObject *last);
DFUNDEF void DCALL DeeGC_TrackMany_Unlock(void);

#define DeeGC_TRACK(T, ob)   ((DREF T *)DeeGC_Track(Dee_AsObject(ob)))
#define DeeGC_UNTRACK(T, ob) ((DREF T *)DeeGC_Untrack(Dee_AsObject(ob)))

/* Try to collect at most `max_objects' GC-objects,
 * returning the actual amount collected. */
DFUNDEF size_t DCALL DeeGC_Collect(size_t max_objects);

#ifdef CONFIG_BUILDING_DEEMON
/* Return `true' if any GC objects with a non-zero reference
 * counter is being tracked.
 * NOTE: In addition, this function does not return `true' when
 *       all that's left are dex objects (which are destroyed
 *       at a later point during deemon shutdown, than the point
 *       when this function is called to determine if the GC must
 *       continue to run) */
INTDEF bool DCALL DeeGC_IsEmptyWithoutDex(void);
#endif /* CONFIG_BUILDING_DEEMON */

/* GC object alloc/free.
 * Don't you think these functions allocate some magical memory
 * that can somehow track what objects it references. - No!
 * All these do is allocate a block of memory of `n_bytes' that
 * includes some storage at negative offsets to hold a `struct Dee_gc_head',
 * as is required for objects that should later be tracked by the GC. */
DFUNDEF ATTR_MALLOC WUNUSED void *(DCALL DeeGCObject_Malloc)(size_t n_bytes);
DFUNDEF ATTR_MALLOC WUNUSED void *(DCALL DeeGCObject_Calloc)(size_t n_bytes);
DFUNDEF WUNUSED void *(DCALL DeeGCObject_Realloc)(void *p, size_t n_bytes);
DFUNDEF ATTR_MALLOC WUNUSED void *(DCALL DeeGCObject_TryMalloc)(size_t n_bytes);
DFUNDEF ATTR_MALLOC WUNUSED void *(DCALL DeeGCObject_TryCalloc)(size_t n_bytes);
DFUNDEF WUNUSED void *(DCALL DeeGCObject_TryRealloc)(void *p, size_t n_bytes);
DFUNDEF void (DCALL DeeGCObject_Free)(void *p);
DFUNDEF ATTR_MALLOC WUNUSED void *(DCALL DeeDbgGCObject_Malloc)(size_t n_bytes, char const *file, int line);
DFUNDEF ATTR_MALLOC WUNUSED void *(DCALL DeeDbgGCObject_Calloc)(size_t n_bytes, char const *file, int line);
DFUNDEF WUNUSED void *(DCALL DeeDbgGCObject_Realloc)(void *p, size_t n_bytes, char const *file, int line);
DFUNDEF ATTR_MALLOC WUNUSED void *(DCALL DeeDbgGCObject_TryMalloc)(size_t n_bytes, char const *file, int line);
DFUNDEF ATTR_MALLOC WUNUSED void *(DCALL DeeDbgGCObject_TryCalloc)(size_t n_bytes, char const *file, int line);
DFUNDEF WUNUSED void *(DCALL DeeDbgGCObject_TryRealloc)(void *p, size_t n_bytes, char const *file, int line);
DFUNDEF void (DCALL DeeDbgGCObject_Free)(void *p, char const *file, int line);

#define DeeGCObject_Mallocc(base_offset, elem_count, elem_size)                           DeeGCObject_Malloc(_Dee_MallococBufsize(base_offset, elem_count, elem_size))
#define DeeGCObject_Callocc(base_offset, elem_count, elem_size)                           DeeGCObject_Calloc(_Dee_MallococBufsize(base_offset, elem_count, elem_size))
#define DeeGCObject_Reallocc(p, base_offset, elem_count, elem_size)                       DeeGCObject_Realloc(p, _Dee_MallococBufsize(base_offset, elem_count, elem_size))
#define DeeGCObject_TryMallocc(base_offset, elem_count, elem_size)                        DeeGCObject_TryMalloc(_Dee_MallococBufsize(base_offset, elem_count, elem_size))
#define DeeGCObject_TryCallocc(base_offset, elem_count, elem_size)                        DeeGCObject_TryCalloc(_Dee_MallococBufsize(base_offset, elem_count, elem_size))
#define DeeGCObject_TryReallocc(p, base_offset, elem_count, elem_size)                    DeeGCObject_TryRealloc(p, _Dee_MallococBufsize(base_offset, elem_count, elem_size))
#define DeeDbgGCObject_Mallocc(base_offset, elem_count, elem_size, file, line)            DeeDbgGCObject_Malloc(_Dee_MallococBufsizeDbg(base_offset, elem_count, elem_size, file, line), file, line)
#define DeeDbgGCObject_Callocc(base_offset, elem_count, elem_size, file, line)            DeeDbgGCObject_Calloc(_Dee_MallococBufsizeDbg(base_offset, elem_count, elem_size, file, line), file, line)
#define DeeDbgGCObject_Reallocc(p, base_offset, elem_count, elem_size, file, line)        DeeDbgGCObject_Realloc(p, _Dee_MallococBufsizeDbg(base_offset, elem_count, elem_size, file, line), file, line)
#define DeeDbgGCObject_TryMallocc(base_offset, elem_count, elem_size, file, line)         DeeDbgGCObject_TryMalloc(_Dee_MallococBufsizeDbg(base_offset, elem_count, elem_size, file, line), file, line)
#define DeeDbgGCObject_TryCallocc(base_offset, elem_count, elem_size, file, line)         DeeDbgGCObject_TryCalloc(_Dee_MallococBufsizeDbg(base_offset, elem_count, elem_size, file, line), file, line)
#define DeeDbgGCObject_TryReallocc(p, base_offset, elem_count, elem_size, file, line)     DeeDbgGCObject_TryRealloc(p, _Dee_MallococBufsizeDbg(base_offset, elem_count, elem_size, file, line), file, line)
#define DeeGCObject_MalloccSafe(base_offset, elem_count, elem_size)                       DeeGCObject_Malloc(_Dee_MallococBufsizeSafe(base_offset, elem_count, elem_size))
#define DeeGCObject_CalloccSafe(base_offset, elem_count, elem_size)                       DeeGCObject_Calloc(_Dee_MallococBufsizeSafe(base_offset, elem_count, elem_size))
#define DeeGCObject_RealloccSafe(p, base_offset, elem_count, elem_size)                   DeeGCObject_Realloc(p, _Dee_MallococBufsizeSafe(base_offset, elem_count, elem_size))
#define DeeGCObject_TryMalloccSafe(base_offset, elem_count, elem_size)                    DeeGCObject_TryMalloc(_Dee_MallococBufsizeSafe(base_offset, elem_count, elem_size))
#define DeeGCObject_TryCalloccSafe(base_offset, elem_count, elem_size)                    DeeGCObject_TryCalloc(_Dee_MallococBufsizeSafe(base_offset, elem_count, elem_size))
#define DeeGCObject_TryRealloccSafe(p, base_offset, elem_count, elem_size)                DeeGCObject_TryRealloc(p, _Dee_MallococBufsizeSafe(base_offset, elem_count, elem_size))
#define DeeDbgGCObject_MalloccSafe(base_offset, elem_count, elem_size, file, line)        DeeDbgGCObject_Malloc(_Dee_MallococBufsizeSafe(base_offset, elem_count, elem_size), file, line)
#define DeeDbgGCObject_CalloccSafe(base_offset, elem_count, elem_size, file, line)        DeeDbgGCObject_Calloc(_Dee_MallococBufsizeSafe(base_offset, elem_count, elem_size), file, line)
#define DeeDbgGCObject_RealloccSafe(p, base_offset, elem_count, elem_size, file, line)    DeeDbgGCObject_Realloc(p, _Dee_MallococBufsizeSafe(base_offset, elem_count, elem_size), file, line)
#define DeeDbgGCObject_TryMalloccSafe(base_offset, elem_count, elem_size, file, line)     DeeDbgGCObject_TryMalloc(_Dee_MallococBufsizeSafe(base_offset, elem_count, elem_size), file, line)
#define DeeDbgGCObject_TryCalloccSafe(base_offset, elem_count, elem_size, file, line)     DeeDbgGCObject_TryCalloc(_Dee_MallococBufsizeSafe(base_offset, elem_count, elem_size), file, line)
#define DeeDbgGCObject_TryRealloccSafe(p, base_offset, elem_count, elem_size, file, line) DeeDbgGCObject_TryRealloc(p, _Dee_MallococBufsizeSafe(base_offset, elem_count, elem_size), file, line)

#ifndef NDEBUG
#undef DeeGCObject_Mallocc
#undef DeeGCObject_Callocc
#undef DeeGCObject_Reallocc
#undef DeeGCObject_TryMallocc
#undef DeeGCObject_TryCallocc
#undef DeeGCObject_TryReallocc
#undef DeeGCObject_MalloccSafe
#undef DeeGCObject_CalloccSafe
#undef DeeGCObject_RealloccSafe
#undef DeeGCObject_TryMalloccSafe
#undef DeeGCObject_TryCalloccSafe
#undef DeeGCObject_TryRealloccSafe
#define DeeGCObject_Malloc(n_bytes)                                          DeeDbgGCObject_Malloc(n_bytes, __FILE__, __LINE__)
#define DeeGCObject_Calloc(n_bytes)                                          DeeDbgGCObject_Calloc(n_bytes, __FILE__, __LINE__)
#define DeeGCObject_Realloc(ptr, n_bytes)                                    DeeDbgGCObject_Realloc(ptr, n_bytes, __FILE__, __LINE__)
#define DeeGCObject_TryMalloc(n_bytes)                                       DeeDbgGCObject_TryMalloc(n_bytes, __FILE__, __LINE__)
#define DeeGCObject_TryCalloc(n_bytes)                                       DeeDbgGCObject_TryCalloc(n_bytes, __FILE__, __LINE__)
#define DeeGCObject_TryRealloc(ptr, n_bytes)                                 DeeDbgGCObject_TryRealloc(ptr, n_bytes, __FILE__, __LINE__)
#define DeeGCObject_Free(ptr)                                                DeeDbgGCObject_Free(ptr, __FILE__, __LINE__)
#define DeeGCObject_Mallocc(base_offset, elem_count, elem_size)              DeeDbgGCObject_Mallocc(base_offset, elem_count, elem_size, __FILE__, __LINE__)
#define DeeGCObject_Callocc(base_offset, elem_count, elem_size)              DeeDbgGCObject_Callocc(base_offset, elem_count, elem_size, __FILE__, __LINE__)
#define DeeGCObject_Reallocc(ptr, base_offset, elem_count, elem_size)        DeeDbgGCObject_Reallocc(ptr, base_offset, elem_count, elem_size, __FILE__, __LINE__)
#define DeeGCObject_TryMallocc(base_offset, elem_count, elem_size)           DeeDbgGCObject_TryMallocc(base_offset, elem_count, elem_size, __FILE__, __LINE__)
#define DeeGCObject_TryCallocc(base_offset, elem_count, elem_size)           DeeDbgGCObject_TryCallocc(base_offset, elem_count, elem_size, __FILE__, __LINE__)
#define DeeGCObject_TryReallocc(ptr, base_offset, elem_count, elem_size)     DeeDbgGCObject_TryReallocc(ptr, base_offset, elem_count, elem_size, __FILE__, __LINE__)
#define DeeGCObject_MalloccSafe(base_offset, elem_count, elem_size)          DeeDbgGCObject_MalloccSafe(base_offset, elem_count, elem_size, __FILE__, __LINE__)
#define DeeGCObject_CalloccSafe(base_offset, elem_count, elem_size)          DeeDbgGCObject_CalloccSafe(base_offset, elem_count, elem_size, __FILE__, __LINE__)
#define DeeGCObject_RealloccSafe(ptr, base_offset, elem_count, elem_size)    DeeDbgGCObject_RealloccSafe(ptr, base_offset, elem_count, elem_size, __FILE__, __LINE__)
#define DeeGCObject_TryMalloccSafe(base_offset, elem_count, elem_size)       DeeDbgGCObject_TryMalloccSafe(base_offset, elem_count, elem_size, __FILE__, __LINE__)
#define DeeGCObject_TryCalloccSafe(base_offset, elem_count, elem_size)       DeeDbgGCObject_TryCalloccSafe(base_offset, elem_count, elem_size, __FILE__, __LINE__)
#define DeeGCObject_TryRealloccSafe(ptr, base_offset, elem_count, elem_size) DeeDbgGCObject_TryRealloccSafe(ptr, base_offset, elem_count, elem_size, __FILE__, __LINE__)
#else /* !NDEBUG */
#undef DeeDbgGCObject_Mallocc
#undef DeeDbgGCObject_Callocc
#undef DeeDbgGCObject_Reallocc
#undef DeeDbgGCObject_TryMallocc
#undef DeeDbgGCObject_TryCallocc
#undef DeeDbgGCObject_TryReallocc
#undef DeeDbgGCObject_MalloccSafe
#undef DeeDbgGCObject_CalloccSafe
#undef DeeDbgGCObject_RealloccSafe
#undef DeeDbgGCObject_TryMalloccSafe
#undef DeeDbgGCObject_TryCalloccSafe
#undef DeeDbgGCObject_TryRealloccSafe
#define DeeDbgGCObject_Malloc(n_bytes, file, line)                                          DeeGCObject_Malloc(n_bytes)
#define DeeDbgGCObject_Calloc(n_bytes, file, line)                                          DeeGCObject_Calloc(n_bytes)
#define DeeDbgGCObject_Realloc(ptr, n_bytes, file, line)                                    DeeGCObject_Realloc(ptr, n_bytes)
#define DeeDbgGCObject_TryMalloc(n_bytes, file, line)                                       DeeGCObject_TryMalloc(n_bytes)
#define DeeDbgGCObject_TryCalloc(n_bytes, file, line)                                       DeeGCObject_TryCalloc(n_bytes)
#define DeeDbgGCObject_TryRealloc(ptr, n_bytes, file, line)                                 DeeGCObject_TryRealloc(ptr, n_bytes)
#define DeeDbgGCObject_Free(ptr, file, line)                                                DeeGCObject_Free(ptr)
#define DeeDbgGCObject_Mallocc(base_offset, elem_count, elem_size, file, line)              DeeGCObject_Mallocc(base_offset, elem_count, elem_size)
#define DeeDbgGCObject_Callocc(base_offset, elem_count, elem_size, file, line)              DeeGCObject_Callocc(base_offset, elem_count, elem_size)
#define DeeDbgGCObject_Reallocc(ptr, base_offset, elem_count, elem_size, file, line)        DeeGCObject_Reallocc(ptr, base_offset, elem_count, elem_size)
#define DeeDbgGCObject_TryMallocc(base_offset, elem_count, elem_size, file, line)           DeeGCObject_TryMallocc(base_offset, elem_count, elem_size)
#define DeeDbgGCObject_TryCallocc(base_offset, elem_count, elem_size, file, line)           DeeGCObject_TryCallocc(base_offset, elem_count, elem_size)
#define DeeDbgGCObject_TryReallocc(ptr, base_offset, elem_count, elem_size, file, line)     DeeGCObject_TryReallocc(ptr, base_offset, elem_count, elem_size)
#define DeeDbgGCObject_MalloccSafe(base_offset, elem_count, elem_size, file, line)          DeeGCObject_MalloccSafe(base_offset, elem_count, elem_size)
#define DeeDbgGCObject_CalloccSafe(base_offset, elem_count, elem_size, file, line)          DeeGCObject_CalloccSafe(base_offset, elem_count, elem_size)
#define DeeDbgGCObject_RealloccSafe(ptr, base_offset, elem_count, elem_size, file, line)    DeeGCObject_RealloccSafe(ptr, base_offset, elem_count, elem_size)
#define DeeDbgGCObject_TryMalloccSafe(base_offset, elem_count, elem_size, file, line)       DeeGCObject_TryMalloccSafe(base_offset, elem_count, elem_size)
#define DeeDbgGCObject_TryCalloccSafe(base_offset, elem_count, elem_size, file, line)       DeeGCObject_TryCalloccSafe(base_offset, elem_count, elem_size)
#define DeeDbgGCObject_TryRealloccSafe(ptr, base_offset, elem_count, elem_size, file, line) DeeGCObject_TryRealloccSafe(ptr, base_offset, elem_count, elem_size)
#endif /* NDEBUG */


/* Allocate fixed-size, gc-object-purposed slab memory.
 * NOTE: This memory must be freed by one of:
 *   - DeeGCObject_FFree(return, size2)    | size2 <= size
 *   - DeeGCObject_SlabFree<size2>(return) | size2 <= size
 *   - DeeGCObject_Free(return) */
#ifdef CONFIG_NO_OBJECT_SLABS
#define DeeGCObject_FMalloc(size)    DeeGCObject_Malloc(size)
#define DeeGCObject_FCalloc(size)    DeeGCObject_Calloc(size)
#define DeeGCObject_FTryMalloc(size) DeeGCObject_TryMalloc(size)
#define DeeGCObject_FTryCalloc(size) DeeGCObject_TryCalloc(size)
#define DeeGCObject_FFree(ptr, size) DeeGCObject_Free(ptr)
#elif defined(__INTELLISENSE__)
#define DeeGCObject_FMalloc(size)    DeeGCObject_Malloc(size)
#define DeeGCObject_FCalloc(size)    DeeGCObject_Calloc(size)
#define DeeGCObject_FTryMalloc(size) DeeGCObject_TryMalloc(size)
#define DeeGCObject_FTryCalloc(size) DeeGCObject_TryCalloc(size)
#define DeeGCObject_FFree(ptr, size) (DeeGCObject_Free(ptr), (void)(size))
#else /* CONFIG_NO_OBJECT_SLABS */
#define _Dee_PRIVATE_DEFINE_SLAB_FUNCTIONS(index, size)                              \
	DFUNDEF ATTR_MALLOC WUNUSED void *(DCALL DeeGCObject_SlabMalloc##size)(void);    \
	DFUNDEF ATTR_MALLOC WUNUSED void *(DCALL DeeGCObject_SlabCalloc##size)(void);    \
	DFUNDEF ATTR_MALLOC WUNUSED void *(DCALL DeeGCObject_SlabTryMalloc##size)(void); \
	DFUNDEF ATTR_MALLOC WUNUSED void *(DCALL DeeGCObject_SlabTryCalloc##size)(void); \
	DFUNDEF void (DCALL DeeGCObject_SlabFree##size)(void *__restrict ptr);
DeeSlab_ENUMERATE(_Dee_PRIVATE_DEFINE_SLAB_FUNCTIONS)
#undef _Dee_PRIVATE_DEFINE_SLAB_FUNCTIONS
#define DeeGCObject_FMalloc(size)    DeeSlab_Invoke(DeeGCObject_SlabMalloc, size, (), DeeGCObject_Malloc(size))
#define DeeGCObject_FCalloc(size)    DeeSlab_Invoke(DeeGCObject_SlabCalloc, size, (), DeeGCObject_Calloc(size))
#define DeeGCObject_FTryMalloc(size) DeeSlab_Invoke(DeeGCObject_SlabTryMalloc, size, (), DeeGCObject_TryMalloc(size))
#define DeeGCObject_FTryCalloc(size) DeeSlab_Invoke(DeeGCObject_SlabTryCalloc, size, (), DeeGCObject_TryCalloc(size))
#define DeeGCObject_FFree(ptr, size) DeeSlab_Invoke(DeeGCObject_SlabFree, size, (ptr), DeeGCObject_Free(ptr))
#endif /* !CONFIG_NO_OBJECT_SLABS */

/* Same as the regular malloc functions, but use the same allocation methods that would be
 * used by `Dee_TYPE_CONSTRUCTOR_INIT_FIXED_GC' and `Dee_TYPE_CONSTRUCTOR_INIT_FIXED_GC_S',
 * meaning that pointers returned by these macros have binary compatibility with them. */
#define DeeGCObject_MALLOC(T)       ((T *)DeeGCObject_FMalloc(sizeof(T)))
#define DeeGCObject_CALLOC(T)       ((T *)DeeGCObject_FCalloc(sizeof(T)))
#define DeeGCObject_TRYMALLOC(T)    ((T *)DeeGCObject_FTryMalloc(sizeof(T)))
#define DeeGCObject_TRYCALLOC(T)    ((T *)DeeGCObject_FTryCalloc(sizeof(T)))
#define DeeGCObject_FREE(typed_ptr)       DeeGCObject_FFree(typed_ptr, sizeof(*(typed_ptr)))

#ifdef CONFIG_FIXED_ALLOCATOR_S_IS_AUTO
#define DeeGCObject_MALLOC_S(T)    ((T *)DeeGCObject_Malloc(sizeof(T)))
#define DeeGCObject_CALLOC_S(T)    ((T *)DeeGCObject_Calloc(sizeof(T)))
#define DeeGCObject_TRYMALLOC_S(T) ((T *)DeeGCObject_TryMalloc(sizeof(T)))
#define DeeGCObject_TRYCALLOC_S(T) ((T *)DeeGCObject_TryCalloc(sizeof(T)))
#define DeeGCObject_FREE_S               DeeGCObject_Free
#else /* CONFIG_FIXED_ALLOCATOR_S_IS_AUTO */
#define DeeGCObject_MALLOC_S      DeeGCObject_MALLOC
#define DeeGCObject_CALLOC_S      DeeGCObject_CALLOC
#define DeeGCObject_TRYMALLOC_S   DeeGCObject_TRYMALLOC
#define DeeGCObject_TRYCALLOC_S   DeeGCObject_TRYCALLOC
#define DeeGCObject_FREE_S        DeeGCObject_FREE
#endif /* !CONFIG_FIXED_ALLOCATOR_S_IS_AUTO */

/* An generic sequence singleton that can be
 * iterated to yield all tracked GC objects.
 * This object also offers a hand full of member functions
 * that user-space an invoke to trigger various GC-related
 * functionality:
 *   - collect(max: int = -1): int;
 * Also: remember that this derives from `Sequence', so you
 *       can use all its attributes, like `empty', etc.
 * NOTE: This object is exported as `gc from deemon' */
DDATDEF DeeObject DeeGCEnumTracked_Singleton;


DECL_END

#endif /* !GUARD_DEEMON_GC_H */
