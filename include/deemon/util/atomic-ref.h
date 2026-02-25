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
/*!export Dee_atomic_ref_**/
/*!export Dee_atomic_xref_**/
/*!export Dee_ATOMIC_REF_**/
/*!export Dee_ATOMIC_XREF_**/
/*!export -_Dee_private_**/
/*!export -_Dee_PRIVATE_**/
#ifndef GUARD_DEEMON_UTIL_ATOMIC_REF_H
#define GUARD_DEEMON_UTIL_ATOMIC_REF_H 1 /*!export-*/

#include "../api.h"

#include "../object.h" /* Dee_Decref, Dee_Decref_unlikely, Dee_Incref, Dee_XDecref, Dee_XDecref_unlikely, Dee_XIncref */
#include "../thread.h" /* DeeRCU_Lock, DeeRCU_Synchronize, DeeRCU_Unlock */
#include "../types.h"  /* DREF, DeeObject, Dee_AsObject */

#include <stddef.h> /* NULL */

#ifndef CONFIG_NO_THREADS
#include <hybrid/__atomic.h> /* __ATOMIC_ACQUIRE, __ATOMIC_ACQ_REL, __ATOMIC_RELAXED, __hybrid_atomic_* */
#endif /* !CONFIG_NO_THREADS */

DECL_BEGIN

/*
 * Atomic reference:
 *
 * - read:
 *   >> DREF DeeObject *result;
 *   >> DeeRCU_Lock();
 *   >> result = atomic_read(&ar_obj);
 *   >> Dee_XIncref(result); // Or "Dee_Incref" if known to be non-NULL
 *   >> DeeRCU_Unlock();
 *   >> return result;
 *
 * - write:
 *   >> DREF DeeObject *oldval;
 *   >> Dee_XIncref(newval); // Or "Dee_Incref" if known to be non-NULL
 *   >> oldval = atomic_xch(&ar_obj, newval);
 *   >> DeeRCU_Synchronize();
 *   >> Dee_XDecref(oldval); // Or "Dee_Decref" if known to be non-NULL
 *
 * Explanation:
 * - All reads/writes to "ar_obj" happen atomically
 *   (so no intermediate values can ever be observed)
 * - After writing a new value to "ar_obj", the reference
 *   held to the previous object is *ONLY* dropped *AFTER*
 *   `DeeRCU_Synchronize()' ensured that no thread is still
 *   inside the `DeeRCU_Lock()' ... `DeeRCU_Unlock()' region
 * - As a consequence, for as long as `DeeRCU_Lock()' is held, it
 *   is guarantied that `ar_obj == NULL || ar_obj->ob_refcnt != 0',
 *   meaning that it is always safe to:
 *   - Acquire `DeeRCU_Lock()' (thus preventing the current, or any
 *     new object currently being set from being destroyed until
 *     `DeeRCU_Unlock()' is called)
 *   - Atomically read the current object and Dee_Incref() it
 *   - Call `DeeRCU_Unlock()' (thus allowing writes to complete)
 * - More advanced write operations (like atomic_cmpxch) can
 *   be implemented analogous to regular writes.
 */

#define Dee_ATOMIC_REF(T)                       \
	struct {                                    \
		DREF T *ar_obj; /* [1..1][lock(RCU)] */ \
	}
#define Dee_ATOMIC_XREF(T)                       \
	struct {                                     \
		DREF T *axr_obj; /* [0..1][lock(RCU)] */ \
	}

#define _Dee_private_atomic_ref_incuse(self)  DeeRCU_Lock()
#define _Dee_private_atomic_ref_decuse(self)  DeeRCU_Unlock()
#define _Dee_private_atomic_ref_await(self)   DeeRCU_Synchronize()
#define _Dee_private_atomic_xref_incuse(self) DeeRCU_Lock()
#define _Dee_private_atomic_xref_decuse(self) DeeRCU_Unlock()
#define _Dee_private_atomic_xref_await(self)  DeeRCU_Synchronize()

#define Dee_ATOMIC_REF_INIT(/*1..1*/ /*inherit(always)*/ obj)                 { obj }
#define Dee_atomic_ref_init_inherited(self, /*1..1*/ /*inherit(always)*/ obj) (void)((self)->ar_obj = (obj), Dee_ASSERT((self)->ar_obj))
#define Dee_atomic_ref_init(self, /*1..1*/ obj)                               (void)((self)->ar_obj = (obj), Dee_Incref((self)->ar_obj))
#define Dee_atomic_ref_fini(self)                                             Dee_Decref((self)->ar_obj)

#define Dee_ATOMIC_XREF_INIT(/*0..1*/ /*inherit(always)*/ obj)                 { obj }
#define Dee_atomic_xref_init_inherited(self, /*0..1*/ /*inherit(always)*/ obj) (void)((self)->axr_obj = (obj))
#define Dee_atomic_xref_init(self, /*0..1*/ obj)                               (void)((self)->axr_obj = (obj), Dee_XIncref((self)->axr_obj))
#define Dee_atomic_xref_fini(self)                                             Dee_XDecref((self)->axr_obj)


#ifdef CONFIG_NO_THREADS
#define Dee_atomic_ref_getaddr(self)  (self)->ar_obj
#define Dee_atomic_xref_getaddr(self) (self)->axr_obj
#define _Dee_atomic_ref_unsynched_xch_inherited(self, T,                                     \
                                                /*inherit(always) DREF[1..1] in*/ newval,    \
                                                /*inherit(always) DREF[1..1] out*/ p_oldval) \
	(void)(*(p_oldval) = T((self)->ar_obj), (self)->ar_obj = (newval))
#define _Dee_atomic_xref_unsynched_xch_inherited(self, T,                                     \
                                                 /*inherit(always) DREF[0..1] in*/ newval,    \
                                                 /*inherit(always) DREF[0..1] out*/ p_oldval) \
	(void)(*(p_oldval) = T((self)->axr_obj), (self)->axr_obj = (newval))
#define _Dee_atomic_ref_unsynched_cmpxch_inherited(self, oldval, newval) \
	((self)->ar_obj == (oldval) ? ((self)->ar_obj = (newval), 1) : 0)
#define _Dee_atomic_xref_unsynched_cmpxch_inherited(self, oldval, newval) \
	((self)->axr_obj == (oldval) ? ((self)->axr_obj = (newval), 1) : 0)
#else /* CONFIG_NO_THREADS */
#define Dee_atomic_ref_getaddr(self)  __hybrid_atomic_load(&(self)->ar_obj, __ATOMIC_ACQUIRE)
#define Dee_atomic_xref_getaddr(self) __hybrid_atomic_load(&(self)->axr_obj, __ATOMIC_ACQUIRE)
#define _Dee_atomic_ref_unsynched_xch_inherited(self, T,                                     \
                                                /*inherit(always) DREF[1..1] in*/ newval,    \
                                                /*inherit(always) DREF[1..1] out*/ p_oldval) \
	(void)(*(p_oldval) = T(__hybrid_atomic_xch(&(self)->ar_obj, newval, __ATOMIC_ACQ_REL)))
#define _Dee_atomic_xref_unsynched_xch_inherited(self, T,                                     \
                                                 /*inherit(always) DREF[0..1] in*/ newval,    \
                                                 /*inherit(always) DREF[0..1] out*/ p_oldval) \
	(void)(*(p_oldval) = T(__hybrid_atomic_xch(&(self)->axr_obj, newval, __ATOMIC_ACQ_REL)))
#define _Dee_atomic_ref_unsynched_cmpxch_inherited(self, oldval, newval) \
	__hybrid_atomic_cmpxch(&(self)->ar_obj, oldval, newval, __ATOMIC_ACQ_REL, __ATOMIC_RELAXED)
#define _Dee_atomic_xref_unsynched_cmpxch_inherited(self, oldval, newval) \
	__hybrid_atomic_cmpxch(&(self)->axr_obj, oldval, newval, __ATOMIC_ACQ_REL, __ATOMIC_RELAXED)
#endif /* !CONFIG_NO_THREADS */

/* >> void Dee_atomic_ref_get(Dee_ATOMIC_REF(T) *self, DREF[1..1] T **p_result); */
#define Dee_atomic_ref_get(self, p_result)             \
	(void)(_Dee_private_atomic_ref_incuse(self),       \
	       *(p_result) = Dee_atomic_ref_getaddr(self), \
	       Dee_Incref(*(p_result)),                    \
	       _Dee_private_atomic_ref_decuse(self))

/* >> void Dee_atomic_xref_get(Dee_ATOMIC_XREF(T) *self, DREF[0..1] T **p_result); */
#define Dee_atomic_xref_get(self, p_result)             \
	(void)(_Dee_private_atomic_xref_incuse(self),       \
	       *(p_result) = Dee_atomic_xref_getaddr(self), \
	       Dee_XIncref(*(p_result)),                    \
	       _Dee_private_atomic_xref_decuse(self))


/* Set "newval" and store old value in `*p_oldval' */
#define Dee_atomic_ref_xch_inherited(self,                                        \
                                     /*inherit(always) DREF[1..1] in*/ newval,    \
                                     /*inherit(always) DREF[1..1] out*/ p_oldval) \
	(_Dee_atomic_ref_unsynched_xch_inherited(self, , newval, p_oldval),           \
	 _Dee_private_atomic_ref_await(self))
#define Dee_atomic_xref_xch_inherited(self,                                        \
                                      /*inherit(always) DREF[0..1] in*/ newval,    \
                                      /*inherit(always) DREF[0..1] out*/ p_oldval) \
	(_Dee_atomic_xref_unsynched_xch_inherited(self, , newval, p_oldval),           \
	 _Dee_private_atomic_xref_await(self))


/* Set "newval" and drop reference to old value */
#define Dee_atomic_ref_set_inherited(self, /*inherit(always) DREF[1..1]*/ newval) \
	do {                                                                          \
		DREF DeeObject *_darsi_oldval;                                            \
		_Dee_atomic_ref_unsynched_xch_inherited(self, Dee_AsObject,               \
		                                        newval, &_darsi_oldval);          \
		_Dee_private_atomic_ref_await(self);                                      \
		Dee_Decref(_darsi_oldval);                                                \
	}	__WHILE0
#define Dee_atomic_xref_set_inherited(self, /*inherit(always) DREF[0..1]*/ newval) \
	do {                                                                           \
		DREF DeeObject *_daxrsi_oldval;                                            \
		_Dee_atomic_xref_unsynched_xch_inherited(self, Dee_AsObject,               \
		                                         newval, &_daxrsi_oldval);         \
		_Dee_private_atomic_xref_await(self);                                      \
		Dee_XDecref(_daxrsi_oldval);                                               \
	}	__WHILE0

/* Set "newval" and store old value in `*p_oldval' */
#define Dee_atomic_ref_xch(self, /*[1..1] in*/ newval, /*inherit(always) DREF[1..1] out*/ p_oldval) \
	(Dee_Incref(newval), Dee_atomic_ref_xch_inherited(self, newval, p_oldval))
#define Dee_atomic_xref_xch(self, /*[0..1] in*/ newval, /*inherit(always) DREF[0..1] out*/ p_oldval) \
	(Dee_XIncref(newval), Dee_atomic_xref_xch_inherited(self, newval, p_oldval))

/* Set "newval" and drop reference to old value */
#define Dee_atomic_ref_set(self, /*[1..1] in*/ newval)                  \
	do {                                                                \
		DREF DeeObject *_dars_oldval;                                   \
		Dee_Incref(newval);                                             \
		_Dee_atomic_ref_unsynched_xch_inherited(self, Dee_AsObject,     \
		                                        newval, &_dars_oldval); \
		_Dee_private_atomic_ref_await(self);                            \
		Dee_Decref(_dars_oldval);                                       \
	}	__WHILE0
#define Dee_atomic_xref_set(self, /*[1..1] in*/ newval)                   \
	do {                                                                  \
		DREF DeeObject *_daxrs_oldval;                                    \
		Dee_XIncref(newval);                                              \
		_Dee_atomic_xref_unsynched_xch_inherited(self, Dee_AsObject,      \
		                                         newval, &_daxrs_oldval); \
		_Dee_private_atomic_xref_await(self);                             \
		Dee_XDecref(_daxrs_oldval);                                       \
	}	__WHILE0

/* Clear the atomic reference */
#define Dee_atomic_xref_clear(self)                                     \
	do {                                                                \
		DREF DeeObject *_daxrc_oldval;                                  \
		_Dee_atomic_xref_unsynched_xch_inherited(self, Dee_AsObject,    \
		                                         NULL, &_daxrc_oldval); \
		_Dee_private_atomic_xref_await(self);                           \
		Dee_XDecref(_daxrc_oldval);                                     \
	}	__WHILE0



/* Set value to "newval", but only if current value is "oldval"
 * @return: true:  Value changed. Inherit reference to "newval"; gift reference to "oldval" to caller
 * @return: false: Value didn't change. */
#define Dee_atomic_ref_cmpxch_inherited(self, oldval, newval)         \
	(_Dee_atomic_ref_unsynched_cmpxch_inherited(self, oldval, newval) \
	 ? (_Dee_private_atomic_ref_await(self), 1)                       \
	 : 0)
#define Dee_atomic_xref_cmpxch_inherited(self, oldval, newval)         \
	(_Dee_atomic_xref_unsynched_cmpxch_inherited(self, oldval, newval) \
	 ? (_Dee_private_atomic_xref_await(self), 1)                       \
	 : 0)

/* Set value to "newval", but only if current value is "oldval"
 * @return: true:  Value changed. Inherit reference to "newval"; gift reference to "oldval" to caller
 * @return: false: Value didn't change. */
#define Dee_atomic_ref_cmpxch(self, oldval, newval)                   \
	(Dee_Incref(newval),                                              \
	 _Dee_atomic_ref_unsynched_cmpxch_inherited(self, oldval, newval) \
	 ? (_Dee_private_atomic_ref_await(self), Dee_Decref(oldval), 1)   \
	 : (Dee_Decref_unlikely(newval), 0))
#define Dee_atomic_xref_cmpxch(self, oldval, newval)                   \
	(Dee_XIncref(newval),                                              \
	 _Dee_atomic_xref_unsynched_cmpxch_inherited(self, oldval, newval) \
	 ? (_Dee_private_atomic_xref_await(self), Dee_XDecref(oldval), 1)  \
	 : (Dee_XDecref_unlikely(newval), 0))

DECL_END

#endif /* !GUARD_DEEMON_UTIL_ATOMIC_REF_H */
