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
/*!export Dee_WEAKREF**/
/*!export Dee_weakref_**/
#ifndef GUARD_DEEMON_UTIL_WEAKREF_H
#define GUARD_DEEMON_UTIL_WEAKREF_H 1 /*!export-*/

#include "../api.h"

#include <hybrid/__atomic.h> /* __ATOMIC_ACQUIRE, __ATOMIC_RELEASE, __hybrid_atomic_load, __hybrid_atomic_store */
#include <hybrid/typecore.h> /* __SIZEOF_POINTER__, __UINT32_C, __UINT64_C */

#include "../types.h" /* DREF, DeeObject, Dee_unlockinfo */


DECL_BEGIN

struct Dee_weakref;
#ifndef Dee_weakref_callback_t_DEFINED
#define Dee_weakref_callback_t_DEFINED
/* Prototype for callbacks to-be invoked when a weakref'd object gets destroyed. */
typedef NONNULL_T((1)) void (DCALL *Dee_weakref_callback_t)(struct Dee_weakref *__restrict self);
#endif /* !Dee_weakref_callback_t_DEFINED */


/* Object weak reference tracing. */
struct Dee_weakref {
	struct Dee_weakref   **wr_pself; /* [0..1][lock(BIT0(wr_next))][valid_if(wr_pself != NULL)] Indirect self pointer. */
	struct Dee_weakref    *wr_next;  /* [0..1][lock(BIT0(wr_next))][valid_if(wr_pself != NULL)] Next weak references. */
	DeeObject             *wr_obj;   /* [0..1][lock(BIT0(wr_next))] Pointed-to object. */
	Dee_weakref_callback_t wr_del;   /* [0..1][const]
	                                  * An optional callback that is invoked when the bound object
	                                  * `wr_obj' gets destroyed, causing the weakref to become unbound.
	                                  * NOTE: If set, this callback _MUST_ invoke `DeeWeakref_UnlockCallback()'
	                                  *       in order to unlock the passed `struct Dee_weakref', after it has
	                                  *       acquired shared ownership to a containing object if it intends
	                                  *       to invoke arbitrary user-code, or drop references. */
};
#define Dee_WEAKREF_INIT { NULL, NULL, NULL, NULL }
#ifdef __cplusplus
#define Dee_WEAKREF(T) struct ::Dee_weakref
#else /* __cplusplus */
#define Dee_WEAKREF(T) struct Dee_weakref
#endif /* !__cplusplus */

/* The used NULL-pointer value for user-defined weakref callbacks.
 * It is only important that bit#0 be clear during assignment, however
 * a value distinct from NULL (or rather: 0) can be used to make it
 * easier to detect invalid uses of weakref controllers. */
#ifndef NDEBUG
#if __SIZEOF_POINTER__ == 4
#define _Dee_PRIVATE_WEAKREF_UNLOCKCALLBACK_NULLPTR \
	(__UINT32_C(0xcccccccc) & ~__UINT32_C(1))
#elif __SIZEOF_POINTER__ == 8
#define _Dee_PRIVATE_WEAKREF_UNLOCKCALLBACK_NULLPTR \
	(__UINT64_C(0xcccccccccccccccc) & ~__UINT64_C(1))
#else /* ... */
#define _Dee_PRIVATE_WEAKREF_UNLOCKCALLBACK_NULLPTR \
	((uintptr_t)-1 & ~(uintptr_t)1)
#endif /* !... */
#else /* !NDEBUG */
#define _Dee_PRIVATE_WEAKREF_UNLOCKCALLBACK_NULLPTR 0
#endif /* NDEBUG */


/* Unlock a weakref from within a `wr_del' callback.
 * An invocation of this macro is _MANDATORY_ for any custom weakref
 * callback, as it is part of the synchronization process used to prevent
 * race conditions when working with weakref callbacks.
 * A simple weakref callback that invokes another user-code function could
 * then look like this:
 * >> typedef struct {
 * >>     Dee_OBJECT_HEAD
 * >>     struct Dee_weakref o_ref; // Uses `my_callback'
 * >>     DREF DeeObject    *o_fun; // 1..1
 * >> } MyObject;
 * >>
 * >> PRIVATE void DCALL my_callback(struct Dee_weakref *__restrict self) {
 * >>     DREF MyObject *me;
 * >>     me = COMPILER_CONTAINER_OF(self, MyObject, o_ref);
 * >>     if (!Dee_IncrefIfNotZero(me)) {
 * >>         // Race condition: the weakref controller died while the
 * >>         //                 weakref'd object was being destroyed.
 * >>         DeeWeakref_UnlockCallback(self);
 * >>     } else {
 * >>         DREF DeeObject *result;
 * >>         DeeWeakref_UnlockCallback(self);
 * >>         // At this point, we've unlocked the weakref after safely acquiring
 * >>         // a reference to the controlling object, meaning we're now safe to
 * >>         // execute arbitrary code, with the only caveat being that exceptions
 * >>         // can't be propagated.
 * >>         result = DeeObject_Call(me->o_fun, 0, NULL);
 * >>         Dee_Decref(me);
 * >>         if unlikely(!result) {
 * >>             DeeError_Print("Unhandled exception in weakref callback",
 * >>                            ERROR_PRINT_DOHANDLE);
 * >>         } else {
 * >>             Dee_Decref(result);
 * >>         }
 * >>     }
 * >> }
 */
#ifdef __cplusplus
#define DeeWeakref_UnlockCallback(x) \
	__hybrid_atomic_store(&(x)->wr_next, (struct ::Dee_weakref *)_Dee_PRIVATE_WEAKREF_UNLOCKCALLBACK_NULLPTR, __ATOMIC_RELEASE)
#else /* __cplusplus */
#define DeeWeakref_UnlockCallback(x) \
	__hybrid_atomic_store(&(x)->wr_next, (struct Dee_weakref *)_Dee_PRIVATE_WEAKREF_UNLOCKCALLBACK_NULLPTR, __ATOMIC_RELEASE)
#endif /* !__cplusplus */


/* Initialize the given weak reference to NULL. */
#define Dee_weakref_initempty(self)                         \
	(void)((self)->wr_pself = NULL, (self)->wr_next = NULL, \
	       (self)->wr_obj = NULL, (self)->wr_del = NULL)

/* Weak reference functionality.
 * @assume(ob != NULL);
 * @return: true:  Successfully initialized the given weak reference.
 * @return: false: The given object `ob' does not support weak referencing. */
#ifdef __INTELLISENSE__
DFUNDEF NONNULL((1, 2)) bool DCALL
Dee_weakref_init(struct Dee_weakref *__restrict self,
                 DeeObject *__restrict ob,
                 Dee_weakref_callback_t callback);
#else /* __INTELLISENSE__ */
#define Dee_weakref_init(self, ob, callback) \
	((self)->wr_del = (callback), (Dee_weakref_init)(self, ob))
DFUNDEF NONNULL((1, 2)) bool
(DCALL Dee_weakref_init)(struct Dee_weakref *__restrict self,
                         DeeObject *__restrict ob);
#endif /* !__INTELLISENSE__ */

/* Advanced API to atomically initialize many weak references all at once:
 * >> size_t weakref_c = ...;
 * >> struct Dee_weakref **weakref_v = ...;
 * >> weakref_v[0].wr_obj = (DREF DeeObject *)obj; // Inherited by "Dee_weakref_initmany_unlock_and_inherit"
 * >> weakref_v[0].wr_del = ...;
 * >> ...
 * >> Dee_weakref_initmany_lock(weakref_v, weakref_c);
 * >> // Potentially do some stuff that needs to happen atomically...
 * >> Dee_weakref_initmany_exec(weakref_v, weakref_c);
 * >> Dee_weakref_initmany_unlock_and_inherit(weakref_v, weakref_c);
 *
 * WARNING: `Dee_weakref_initmany_unlock_and_inherit()' will clobber the contents of
 *          `weakref_v' as a vector of object references that is returned (and must
 *          be decref'd by the caller) */
DFUNDEF WUNUSED bool DCALL Dee_weakref_initmany_trylock(struct Dee_weakref *const *weakref_v, size_t weakref_c, struct Dee_unlockinfo *unlock);
DFUNDEF void DCALL Dee_weakref_initmany_lock(struct Dee_weakref *const *weakref_v, size_t weakref_c);
DFUNDEF void DCALL Dee_weakref_initmany_exec(struct Dee_weakref *const *weakref_v, size_t weakref_c);
DFUNDEF DREF DeeObject **DCALL Dee_weakref_initmany_unlock_and_inherit(struct Dee_weakref **weakref_v, size_t weakref_c);
DFUNDEF void DCALL Dee_weakref_initmany_unlock(struct Dee_weakref *const *weakref_v, size_t weakref_c);


/* Finalize a given weak reference. */
DFUNDEF NONNULL((1)) void DCALL Dee_weakref_fini(struct Dee_weakref *__restrict self);

/* Move/Copy a given weak reference into another, optionally
 * overwriting whatever object was referenced before.
 * NOTE: Assignment here does _NOT_ override a set deletion callback! */
DFUNDEF NONNULL((1, 2)) void DCALL Dee_weakref_move(struct Dee_weakref *__restrict dst, struct Dee_weakref *__restrict src);
DFUNDEF NONNULL((1, 2)) void DCALL Dee_weakref_moveassign(struct Dee_weakref *dst, struct Dee_weakref *src);
#ifdef __INTELLISENSE__
DFUNDEF NONNULL((1, 2)) void (DCALL Dee_weakref_copy)(struct Dee_weakref *__restrict self, struct Dee_weakref const *__restrict other);
DFUNDEF NONNULL((1, 2)) void (DCALL Dee_weakref_copyassign)(struct Dee_weakref *self, struct Dee_weakref const *other);
#else /* __INTELLISENSE__ */
DFUNDEF NONNULL((1, 2)) void (DCALL Dee_weakref_copy)(struct Dee_weakref *__restrict self, struct Dee_weakref *__restrict other);
DFUNDEF NONNULL((1, 2)) void (DCALL Dee_weakref_copyassign)(struct Dee_weakref *self, struct Dee_weakref *other);
#ifdef __cplusplus
#define Dee_weakref_copy(self, other) Dee_weakref_copy(self, (struct ::Dee_weakref *)(other))
#define Dee_weakref_copyassign(self, other) Dee_weakref_copyassign(self, (struct ::Dee_weakref *)(other))
#else /* __cplusplus */
#define Dee_weakref_copy(self, other) Dee_weakref_copy(self, (struct Dee_weakref *)(other))
#define Dee_weakref_copyassign(self, other) Dee_weakref_copyassign(self, (struct Dee_weakref *)(other))
#endif /* !__cplusplus */
#endif /* !__INTELLISENSE__ */

/* Overwrite an already initialize weak reference with the given `ob'.
 * @return: true:  Successfully overwritten the weak reference.
 * @return: false: The given object `ob' does not support weak referencing
 *                 and the stored weak reference was not modified. */
DFUNDEF NONNULL((1, 2)) bool DCALL
Dee_weakref_set(struct Dee_weakref *__restrict self,
                DeeObject *__restrict ob);

#if !defined(NDEBUG) && !defined(NDEBUG_ASSERT)
#define Dee_weakref_set_forced(self, ob) Dee_ASSERT(Dee_weakref_set(self, ob))
#else /* !defined(NDEBUG) && !defined(NDEBUG_ASSERT) */
#define Dee_weakref_set_forced(self, ob) Dee_weakref_set(self, ob)
#endif /* NDEBUG || NDEBUG_ASSERT */

/* Clear the weak reference `self', returning true if it used to point to an object.
 * NOTE: Upon success (return is `true'), the callback will not be
 *       executed for the previously bound object's destruction. */
DFUNDEF NONNULL((1)) bool DCALL
Dee_weakref_clear(struct Dee_weakref *__restrict self);

#define Dee_weakref_getaddr(self) ((DeeObject *)((uintptr_t)__hybrid_atomic_load(&(self)->wr_obj, __ATOMIC_ACQUIRE) & ~1))

/* Lock a weak reference, returning a regular reference to the pointed-to object.
 * @return: * :   A new reference to the pointed-to object.
 * @return: NULL: Failed to lock the weak reference (No error is thrown in this case). */
#ifdef __INTELLISENSE__
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *(DCALL Dee_weakref_lock)(struct Dee_weakref const *__restrict self);
#else /* __INTELLISENSE__ */
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *(DCALL Dee_weakref_lock)(struct Dee_weakref *__restrict self);
#ifdef __cplusplus
#define Dee_weakref_lock(self) Dee_weakref_lock((struct ::Dee_weakref *)(self))
#else /* __cplusplus */
#define Dee_weakref_lock(self) Dee_weakref_lock((struct Dee_weakref *)(self))
#endif /* !__cplusplus */
#endif /* !__INTELLISENSE__ */

/* Return the state of a snapshot of `self' currently being bound. */
#ifdef __INTELLISENSE__
DFUNDEF WUNUSED NONNULL((1)) bool (DCALL Dee_weakref_bound)(struct Dee_weakref const *__restrict self);
#else /* __INTELLISENSE__ */
DFUNDEF WUNUSED NONNULL((1)) bool (DCALL Dee_weakref_bound)(struct Dee_weakref *__restrict self);
#ifdef __cplusplus
#define Dee_weakref_bound(self) Dee_weakref_bound((struct ::Dee_weakref *)(self))
#else /* __cplusplus */
#define Dee_weakref_bound(self) Dee_weakref_bound((struct Dee_weakref *)(self))
#endif /* !__cplusplus */
#endif /* !__INTELLISENSE__ */

/* Do an atomic compare-exchange operation on the weak reference
 * and return a reference to the previously assigned object, or
 * `NULL' when none was assigned, or `Dee_ITER_DONE' when `new_ob'
 * does not support weak referencing functionality (in which case
 * the actual pointed-to weak object of `self' isn't changed).
 * NOTE: You may pass `NULL' for `new_ob' to clear the weakref. */
DFUNDEF WUNUSED NONNULL((1)) DREF DeeObject *
(DCALL Dee_weakref_cmpxch)(struct Dee_weakref *__restrict self,
                           DeeObject *old_ob, DeeObject *new_ob);

DECL_END

#endif /* !GUARD_DEEMON_UTIL_WEAKREF_H */
