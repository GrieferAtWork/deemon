/* Copyright (c) 2018-2025 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2025 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_UTIL_ATOMIC_REF_H
#define GUARD_DEEMON_UTIL_ATOMIC_REF_H 1

#include "../api.h"
/**/

#include "../types.h"
#include "../object.h"

#ifndef CONFIG_NO_THREADS
#include <hybrid/__atomic.h>
#include <hybrid/sched/__yield.h>
#endif /* !CONFIG_NO_THREADS */

DECL_BEGIN

/*
 * Atomic reference:
 *
 * - read:
 *   >> DREF DeeObject *result;
 *   >> atomic_inc(&ar_use);
 *   >> result = atomic_read(&ar_obj);
 *   >> Dee_XIncref(result); // Or "Dee_Incref" if known to be non-NULL
 *   >> atomic_dec(&ar_use);
 *   >> return result;
 *
 * - write:
 *   >> DREF DeeObject *oldval;
 *   >> Dee_XIncref(newval); // Or "Dee_Incref" if known to be non-NULL
 *   >> oldval = atomic_xch(&ar_obj, newval);
 *   >> while (atomic_read(&ar_use) != 0)
 *   >>     SCHED_YIELD();
 *   >> Dee_XDecref(oldval); // Or "Dee_Decref" if known to be non-NULL
 *
 * Explanation:
 * - All reads/writes to "ar_obj" happen atomically
 *   (so no intermediate values can ever be observed)
 * - After writing a new value to "ar_obj", the reference
 *   held to the previous object is *ONLY* dropped *AFTER*
 *   the `ar_use' counter drops to `0'
 * - As a consequence, for as long as `ar_use != 0', it is
 *   guarantied that `ar_obj->ob_refcnt != 0', meaning that
 *   it is always safe to:
 *   - Increment `ar_use' (thus preventing the current, or any
 *     new object currently being set from being destroyed until
 *     `ar_use' is decremented)
 *   - Atomically read the current object and Dee_Incref() it
 *   - Decrement `ar_use' (thus allowing writes to complete)
 * - More advanced write operations (like atomic_cmpxch) can
 *   be implemented analogous to regular writes.
 */

typedef struct {
#ifndef CONFIG_NO_THREADS
	Dee_refcnt_t    ar_use; /* [lock(ATOMIC)] In-use counter for atomic reference */
#endif /* !CONFIG_NO_THREADS */
	DREF DeeObject *ar_obj; /* [valid_if(ar_use > 0)][0..1][lock(ATOMIC)]
	                         * Pointed-to object. It is guarantied that the
	                         * reference counter of this object is non-zero
	                         * when `ar_use > 0' */
} Dee_atomic_ref_t;

#define Dee_ATOMIC_REF(T) Dee_atomic_ref_t

#ifdef CONFIG_NO_THREADS
#define _Dee_ATOMIC_REF_INIT_COMMON_        /* nothing */
#define _Dee_atomic_ref_init_common_(self)  /* nothing */
#define _Dee_atomic_ref_cinit_common_(self) /* nothing */
#define _Dee_atomic_ref_incuse(self)        (void)0
#define _Dee_atomic_ref_decuse(self)        (void)0
#define _Dee_atomic_ref_await(self)         (void)0
#else /* CONFIG_NO_THREADS */
#define _Dee_ATOMIC_REF_INIT_COMMON_        0,
#define _Dee_atomic_ref_init_common_(self)  (self)->ar_use = 0,
#define _Dee_atomic_ref_cinit_common_(self) Dee_ASSERT((self)->ar_use == 0),
#define _Dee_atomic_ref_incuse(self)        __hybrid_atomic_inc(&(self)->ar_use, __ATOMIC_ACQUIRE)
#define _Dee_atomic_ref_decuse(self)        __hybrid_atomic_dec(&(self)->ar_use, __ATOMIC_RELEASE)
#define _Dee_atomic_ref_await(self)                                     \
	do {                                                                \
		while (__hybrid_atomic_load(&(self)->ar_use, __ATOMIC_ACQUIRE)) \
			__hybrid_yield();                                           \
	}	__WHILE0
#endif /* !CONFIG_NO_THREADS */

#define Dee_ATOMIC_REF_INIT(/*0..1*/ /*inherit(always)*/ obj) { _Dee_ATOMIC_REF_INIT_COMMON_ obj }
#define Dee_atomic_ref_init_inherited(self, /*0..1*/ /*inherit(always)*/ obj)  (void)(_Dee_atomic_ref_init_common_(self) (self)->ar_obj = (obj))
#define Dee_atomic_ref_cinit_inherited(self, /*0..1*/ /*inherit(always)*/ obj) (void)(_Dee_atomic_ref_cinit_common_(self) (self)->ar_obj = (obj))
#define Dee_atomic_ref_init(self, /*1..1*/ obj)   (void)(_Dee_atomic_ref_init_common_(self) (self)->ar_obj = (DREF DeeObject *)Dee_REQUIRES_OBJECT(obj), Dee_Incref((self)->ar_obj))
#define Dee_atomic_ref_cinit(self, /*1..1*/ obj)  (void)(_Dee_atomic_ref_cinit_common_(self) (self)->ar_obj = (DREF DeeObject *)Dee_REQUIRES_OBJECT(obj), Dee_Incref((self)->ar_obj))
#define Dee_atomic_ref_xinit(self, /*0..1*/ obj)  (void)(_Dee_atomic_ref_init_common_(self) (self)->ar_obj = (obj), Dee_XIncref((self)->ar_obj))
#define Dee_atomic_ref_cxinit(self, /*0..1*/ obj) (void)(_Dee_atomic_ref_cinit_common_(self) (self)->ar_obj = (obj), Dee_XIncref((self)->ar_obj))
#define Dee_atomic_ref_fini(self)  Dee_Decref((self)->ar_obj)
#define Dee_atomic_ref_xfini(self) Dee_XDecref((self)->ar_obj)


#ifdef CONFIG_NO_THREADS
#define Dee_atomic_ref_getaddr(self) (self)->ar_obj
#define Dee_atomic_ref_get(self)     (Dee_Incref((self)->ar_obj), (self)->ar_obj)
#define Dee_atomic_ref_xget(self)    (Dee_XIncref((self)->ar_obj), (self)->ar_obj)
#else /* CONFIG_NO_THREADS */
#define Dee_atomic_ref_getaddr(self) __hybrid_atomic_load(&(self)->ar_obj, __ATOMIC_ACQUIRE)

LOCAL ATTR_RETNONNULL WUNUSED NONNULL((1)) DREF DeeObject *DCALL
Dee_atomic_ref_get(Dee_atomic_ref_t *__restrict self) {
	DREF DeeObject *result;
	_Dee_atomic_ref_incuse(self);
	result = Dee_atomic_ref_getaddr(self);
	Dee_Incref(result);
	_Dee_atomic_ref_decuse(self);
	return result;
}
LOCAL WUNUSED NONNULL((1)) DREF DeeObject *DCALL
Dee_atomic_ref_xget(Dee_atomic_ref_t *__restrict self) {
	DREF DeeObject *result;
	_Dee_atomic_ref_incuse(self);
	result = Dee_atomic_ref_getaddr(self);
	Dee_XIncref(result);
	_Dee_atomic_ref_decuse(self);
	return result;
}
#endif /* !CONFIG_NO_THREADS */

LOCAL ATTR_RETNONNULL WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
Dee_atomic_ref_xch_inherited(Dee_atomic_ref_t *__restrict self,
                             /*inherit(always)*/ DREF DeeObject *__restrict newval) {
	DREF DeeObject *result;
#ifdef CONFIG_NO_THREADS
	result = self->ar_obj;
	self->ar_obj = newval;
#else /* CONFIG_NO_THREADS */
	result = __hybrid_atomic_xch(&self->ar_obj, newval, __ATOMIC_SEQ_CST);
#endif /* !CONFIG_NO_THREADS */
	_Dee_atomic_ref_await(self);
	return result;
}

LOCAL WUNUSED NONNULL((1)) DREF DeeObject *DCALL
Dee_atomic_ref_xxch_inherited(Dee_atomic_ref_t *__restrict self,
                              /*inherit(always)*/ DREF DeeObject *newval) {
	DREF DeeObject *result;
#ifdef CONFIG_NO_THREADS
	result = self->ar_obj;
	self->ar_obj = newval;
#else /* CONFIG_NO_THREADS */
	result = __hybrid_atomic_xch(&self->ar_obj, newval, __ATOMIC_SEQ_CST);
#endif /* !CONFIG_NO_THREADS */
	_Dee_atomic_ref_await(self);
	return result;
}

LOCAL NONNULL((1, 2)) void DCALL
Dee_atomic_ref_set_inherited(Dee_atomic_ref_t *__restrict self,
                             /*inherit(always)*/ DREF DeeObject *__restrict newval) {
	DREF DeeObject *oldval = Dee_atomic_ref_xch_inherited(self, newval);
	Dee_Decref(oldval);
}

LOCAL NONNULL((1)) void DCALL
Dee_atomic_ref_xset_inherited(Dee_atomic_ref_t *__restrict self,
                              /*inherit(always)*/ DREF DeeObject *newval) {
	DREF DeeObject *oldval = Dee_atomic_ref_xxch_inherited(self, newval);
	Dee_XDecref(oldval);
}

#ifdef __INTELLISENSE__
LOCAL ATTR_RETNONNULL WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
Dee_atomic_ref_xch(Dee_atomic_ref_t *__restrict self, DeeObject *__restrict newval);
LOCAL WUNUSED NONNULL((1)) DREF DeeObject *DCALL
Dee_atomic_ref_xxch(Dee_atomic_ref_t *__restrict self, DeeObject *newval);
LOCAL NONNULL((1, 2)) void DCALL
Dee_atomic_ref_set(Dee_atomic_ref_t *__restrict self, DeeObject *__restrict newval);
LOCAL NONNULL((1)) void DCALL
Dee_atomic_ref_xset(Dee_atomic_ref_t *__restrict self, DeeObject *newval);
#else /* __INTELLISENSE__ */
#define Dee_atomic_ref_xch(self, newval) \
	(Dee_Incref(newval), Dee_atomic_ref_xch_inherited(self, newval))
#define Dee_atomic_ref_xxch(self, newval) \
	(Dee_XIncref(newval), Dee_atomic_ref_xxch_inherited(self, newval))
#define Dee_atomic_ref_set(self, newval) \
	(Dee_Incref(newval), Dee_atomic_ref_set_inherited(self, newval))
#define Dee_atomic_ref_xset(self, newval) \
	(Dee_XIncref(newval), Dee_atomic_ref_xset_inherited(self, newval))
#endif /* !__INTELLISENSE__ */


/* Set value to "newval", but only if current value is "oldval"
 * @return: true:  Value changed. Inherit reference to "newval"; gift reference to "oldval" to caller
 * @return: false: Value didn't change. */
#ifdef CONFIG_NO_THREADS
#define Dee_atomic_ref_cmpxch_inherited(self, oldval, newval) \
	((self)->ar_obj == (oldval) ? ((self)->ar_obj = (newval), 1) : 0)
#define Dee_atomic_ref_cmpxch(self, oldval, newval) \
	((self)->ar_obj == (oldval) ? (Dee_Incref(newval), (self)->ar_obj = (newval), Dee_Decref(oldval), 1) : 0)
#else /* CONFIG_NO_THREADS */
LOCAL WUNUSED NONNULL((1, 2, 3)) __BOOL DCALL
Dee_atomic_ref_cmpxch_inherited(Dee_atomic_ref_t *__restrict self,
                                /*inherit_to_caller(on_success)*/ DeeObject *oldval,
                                /*inherit(on_success)*/ DREF DeeObject *newval) {
	__BOOL result;
	result = __hybrid_atomic_cmpxch(&self->ar_obj, oldval, newval,
	                                __ATOMIC_SEQ_CST, __ATOMIC_RELAXED);
	if (result)
		_Dee_atomic_ref_await(self);
	return result;
}
LOCAL WUNUSED NONNULL((1, 2, 3)) __BOOL DCALL
Dee_atomic_ref_cmpxch(Dee_atomic_ref_t *__restrict self,
                      DeeObject *oldval, DeeObject *newval) {
	__BOOL result;
	Dee_Incref(newval);
	result = Dee_atomic_ref_cmpxch_inherited(self, oldval, newval);
	if (result) {
		Dee_Decref(oldval);
	} else {
		Dee_DecrefNokill(newval);
	}
	return result;
}

#endif /* !CONFIG_NO_THREADS */

/* Set value to "newval", but only if current value is "oldval"
 * @return: true:  Value changed. Inherit reference to "newval"; gift reference to "oldval" to caller
 * @return: false: Value didn't change. */
#ifdef CONFIG_NO_THREADS
#define Dee_atomic_ref_xcmpxch_inherited(self, oldval, newval) \
	((self)->ar_obj == (oldval) ? ((self)->ar_obj = (newval), 1) : 0)
#define Dee_atomic_ref_xcmpxch(self, oldval, newval) \
	((self)->ar_obj == (oldval) ? (Dee_XIncref(newval), (self)->ar_obj = (newval), Dee_XDecref(oldval), 1) : 0)
#else /* CONFIG_NO_THREADS */
LOCAL WUNUSED NONNULL((1)) __BOOL DCALL
Dee_atomic_ref_xcmpxch_inherited(Dee_atomic_ref_t *__restrict self,
                                 /*inherit_to_caller(on_success)*/ DeeObject *oldval,
                                 /*inherit(on_success)*/ DREF DeeObject *newval) {
	__BOOL result;
	result = __hybrid_atomic_cmpxch(&self->ar_obj, oldval, newval,
	                                __ATOMIC_SEQ_CST, __ATOMIC_RELAXED);
	if (result)
		_Dee_atomic_ref_await(self);
	return result;
}
LOCAL WUNUSED NONNULL((1)) __BOOL DCALL
Dee_atomic_ref_xcmpxch(Dee_atomic_ref_t *__restrict self,
                       DeeObject *oldval, DeeObject *newval) {
	__BOOL result;
	Dee_XIncref(newval);
	result = Dee_atomic_ref_cmpxch_inherited(self, oldval, newval);
	if (result) {
		Dee_XDecref(oldval);
	} else {
		Dee_XDecrefNokill(newval);
	}
	return result;
}
#endif /* !CONFIG_NO_THREADS */

DECL_END

#endif /* !GUARD_DEEMON_UTIL_ATOMIC_REF_H */
