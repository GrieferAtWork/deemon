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
#ifndef GUARD_DEEMON_OBJECTS_OBJECT_C
#define GUARD_DEEMON_OBJECTS_OBJECT_C 1
#define _KOS_SOURCE 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/attribute.h>
#include <deemon/bool.h>
#include <deemon/class.h>
#include <deemon/code.h>
#include <deemon/error.h>
#include <deemon/file.h>
#include <deemon/float.h>
#include <deemon/gc.h>
#include <deemon/int.h>
#include <deemon/module.h>
#include <deemon/mro.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/objmethod.h>
#include <deemon/seq.h>
#include <deemon/string.h>
#include <deemon/super.h>
#include <deemon/tuple.h>
#include <deemon/util/string.h>

#include <hybrid/align.h>
#include <hybrid/atomic.h>
#include <hybrid/sched/yield.h>
#include <hybrid/typecore.h>

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef CONFIG_NO_BADREFCNT_CHECKS
#include <stdlib.h>
#endif /* CONFIG_NO_BADREFCNT_CHECKS */

#include "../runtime/runtime_error.h"
#include "../runtime/strings.h"

DECL_BEGIN

#ifdef CONFIG_TRACE_REFCHANGES
PRIVATE void DCALL free_reftracker(struct Dee_reftracker *__restrict self);
#endif /* CONFIG_TRACE_REFCHANGES */


typedef DeeTypeObject Type;

PUBLIC int(DCALL DeeObject_AssertType)(DeeObject *__restrict self,
                                       DeeTypeObject *__restrict required_type) {
	if (DeeObject_InstanceOf(self, required_type))
		return 0;
	return DeeObject_TypeAssertFailed(self, required_type);
}

PUBLIC int(DCALL DeeObject_AssertTypeExact)(DeeObject *__restrict self,
                                            DeeTypeObject *__restrict required_type) {
	if (DeeObject_InstanceOfExact(self, required_type))
		return 0;
	return DeeObject_TypeAssertFailed(self, required_type);
}


PUBLIC ATTR_RETNONNULL DeeTypeObject *DCALL
DeeObject_Class(DeeObject *__restrict self) {
	DeeTypeObject *result;
	ASSERT_OBJECT(self);
	result = Dee_TYPE(self);
	if (result == &DeeSuper_Type)
		result = DeeSuper_TYPE(self);
	return result;
}


PUBLIC bool DCALL
DeeType_IsInherited(DeeTypeObject *__restrict test_type,
                    DeeTypeObject *__restrict inherited_type) {
	do {
		if (test_type == inherited_type)
			return true;
	} while ((test_type != test_type->tp_base) &&
	         (test_type = test_type->tp_base) != NULL);
	return false;
}



/* Inheriting the weakref support address works, however
 * it slows down object destruction more than it speeds up
 * weakref usage, so we don't actually use it!. */
#undef CONFIG_INHERIT_WEAKREF_SUPPORT_ADDRESS
#if 0
#define CONFIG_INHERIT_WEAKREF_SUPPORT_ADDRESS 1
#endif

#ifdef CONFIG_INHERIT_WEAKREF_SUPPORT_ADDRESS
#define has_noninherited_weakrefs(tp) \
	((tp)->tp_weakrefs != 0 && (!(tp)->tp_base || (tp)->tp_base->tp_weakrefs != (tp)->tp_weakrefs))
#else /* CONFIG_INHERIT_WEAKREF_SUPPORT_ADDRESS */
#define has_noninherited_weakrefs(tp) \
	((tp)->tp_weakrefs != 0)
#endif /* !CONFIG_INHERIT_WEAKREF_SUPPORT_ADDRESS */


#ifdef CONFIG_INHERIT_WEAKREF_SUPPORT_ADDRESS
PRIVATE bool DCALL
type_inherit_weakrefs(DeeTypeObject *__restrict self) {
	DeeTypeObject *base = DeeType_Base(self);
	if (!base)
		return false;
	if (!base->tp_weakrefs)
		type_inherit_weakrefs(base);
	self->tp_weakrefs = base->tp_weakrefs;
	return true;
}
#endif /* CONFIG_INHERIT_WEAKREF_SUPPORT_ADDRESS */

/* ==== Core Object API ==== */
LOCAL struct weakref_list *DCALL
weakrefs_get(DeeObject *__restrict ob) {
	DeeTypeObject *tp = Dee_TYPE(ob);
#ifdef CONFIG_INHERIT_WEAKREF_SUPPORT_ADDRESS
	if
		unlikely(!tp->tp_weakrefs)
	type_inherit_weakrefs(tp);
#else  /* CONFIG_INHERIT_WEAKREF_SUPPORT_ADDRESS */
	while (!tp->tp_weakrefs && DeeType_Base(tp))
		tp = DeeType_Base(tp);
#endif /* !CONFIG_INHERIT_WEAKREF_SUPPORT_ADDRESS */
	return (struct weakref_list *)((uintptr_t)ob + tp->tp_weakrefs);
}
#define WEAKREFS_GET(ob) weakrefs_get(ob)
#define WEAKREFS_OK(list, ob) \
	((uintptr_t)(list) != (uintptr_t)(ob))


#define PTRLOCK_ADDR_MASK (~1l)
#define PTRLOCK_LOCK_MASK 1l

#define GET_POINTER(x) ((uintptr_t)(x)&PTRLOCK_ADDR_MASK)
#define LOCK_POINTER(x) ptrlock_lock((void **)&(x))
#define TRYLOCK_POINTER(x) ptrlock_trylock((void **)&(x))
#define UNLOCK_POINTER(x) ptrlock_unlock((void **)&(x))
#define WEAKREF_LOCK(x) LOCK_POINTER((x)->wr_next)
#define WEAKREF_TRYLOCK(x) TRYLOCK_POINTER((x)->wr_next)
#define WEAKREF_UNLOCK(x) UNLOCK_POINTER((x)->wr_next)

#define PTRLOCK_LBYTE(self) ((__BYTE_TYPE__ *)(self))[0]
LOCAL bool DCALL ptrlock_trylock(void **__restrict self) {
	__BYTE_TYPE__ lold;
	do {
		lold = ATOMIC_READ(PTRLOCK_LBYTE(self));
		if (lold & PTRLOCK_LOCK_MASK)
			return false;
	} while (!ATOMIC_CMPXCH_WEAK(PTRLOCK_LBYTE(self), lold, lold | PTRLOCK_LOCK_MASK));
	return true;
}

LOCAL void DCALL ptrlock_lock(void **__restrict self) {
	__BYTE_TYPE__ lold;
again:
	do {
		lold = ATOMIC_READ(PTRLOCK_LBYTE(self));
		/* Wait while the lock already is in write-mode or has readers. */
		if (lold & PTRLOCK_LOCK_MASK) {
			SCHED_YIELD();
			goto again;
		}
	} while (!ATOMIC_CMPXCH_WEAK(PTRLOCK_LBYTE(self), lold, lold | PTRLOCK_LOCK_MASK));
}

LOCAL void DCALL ptrlock_unlock(void **__restrict self) {
#ifndef NDEBUG
	__BYTE_TYPE__ lold;
	do
		lold = ATOMIC_READ(PTRLOCK_LBYTE(self));
	while (!ATOMIC_CMPXCH_WEAK(PTRLOCK_LBYTE(self), lold, lold & ~(PTRLOCK_LOCK_MASK)));
	ASSERTF((lold & PTRLOCK_LOCK_MASK) != 0, "Pointer was not locked.");
#else  /* NDEBUG */
	ATOMIC_FETCHAND(PTRLOCK_LBYTE(self), ~(PTRLOCK_LOCK_MASK));
#endif /* !NDEBUG */
}

#if __SIZEOF_POINTER__ == 4 && __SIZEOF_LONG__ == 4
#define WEAKREF_BAD_POINTER 0xccccccccul
#elif __SIZEOF_POINTER__ == 8 && __SIZEOF_LONG__ == 8
#define WEAKREF_BAD_POINTER 0xccccccccccccccccul
#elif defined(__SIZEOF_LONG_LONG__) && \
__SIZEOF_POINTER__ == 8 && __SIZEOF_LONG_LONG__ == 8
#define WEAKREF_BAD_POINTER 0xccccccccccccccccull
#else
#define WEAKREF_BAD_POINTER -1
#endif


PUBLIC bool DCALL
_Dee_weakref_init(struct weakref *__restrict self,
                  DeeObject *__restrict ob) {
	struct weakref_list *list;
	struct weakref *next;
	ASSERT(self);
	ASSERT(ob);
	ASSERT(ob->ob_refcnt);
	ASSERT(IS_ALIGNED((uintptr_t)self, PTRLOCK_LOCK_MASK + 1));
again:
	list = WEAKREFS_GET(ob);
	if
		unlikely(!WEAKREFS_OK(list, ob))
	return false;
	LOCK_POINTER(list->wl_nodes);
	next           = (struct weakref *)GET_POINTER(list->wl_nodes);
	self->wr_obj   = ob;
	self->wr_pself = &list->wl_nodes;
	if (next) {
		ASSERT(next->wr_obj == ob);
		if
			unlikely(!WEAKREF_TRYLOCK(next))
		{
			UNLOCK_POINTER(list->wl_nodes);
			SCHED_YIELD();
			goto again;
		}
		ASSERT(next->wr_pself == &list->wl_nodes);
		next->wr_pself = &self->wr_next;
		self->wr_next  = next;
		WEAKREF_UNLOCK(next);
	} else {
		self->wr_next = NULL;
	}
	/* NOTE: This also unlocks the weakref list for writing. */
	ATOMIC_WRITE(list->wl_nodes, self);
	return true;
}

#ifdef __INTELLISENSE__
PUBLIC void(DCALL Dee_weakref_copy)(struct weakref *__restrict self,
                                    struct weakref const *__restrict other)
#else  /* __INTELLISENSE__ */
PUBLIC void(DCALL Dee_weakref_copy)(struct weakref *__restrict self,
                                    struct weakref *__restrict other)
#endif /* !__INTELLISENSE__ */
{
	ASSERT(self);
	ASSERT(other);
#ifndef NDEBUG
	ASSERT(other->wr_obj != (DeeObject *)WEAKREF_BAD_POINTER);
#endif /* !NDEBUG */
#ifndef NDEBUG
	self->wr_pself = (struct weakref **)WEAKREF_BAD_POINTER;
	self->wr_next  = (struct weakref *)WEAKREF_BAD_POINTER;
#endif /* !NDEBUG */
	self->wr_del = other->wr_del;
again:
	if (other->wr_obj) {
		WEAKREF_LOCK(other);
		COMPILER_READ_BARRIER();
		if
			likely(other->wr_obj)
		{
			if
				unlikely(!TRYLOCK_POINTER(*other->wr_pself))
			{
				WEAKREF_UNLOCK(other);
				SCHED_YIELD();
				goto again;
			}
			self->wr_pself                      = other->wr_pself;
			self->wr_next                       = (struct weakref *)other;
			((struct weakref *)other)->wr_pself = &self->wr_next;
			WEAKREF_UNLOCK(other);
			ATOMIC_WRITE(*self->wr_pself, self);
		} else {
			ATOMIC_WRITE(other->wr_next, NULL); /* WEAKREF_UNLOCK(other); */
			self->wr_obj = NULL;
		}
	} else {
		self->wr_obj = NULL;
	}
}

#ifdef __INTELLISENSE__
PUBLIC void(DCALL Dee_weakref_copyassign)(struct weakref *__restrict self,
                                          struct weakref const *__restrict other)
#else  /* __INTELLISENSE__ */
PUBLIC void(DCALL Dee_weakref_copyassign)(struct weakref *__restrict self,
                                          struct weakref *__restrict other)
#endif /* !__INTELLISENSE__ */
{
	ASSERT(self);
	ASSERT(other);
#ifndef NDEBUG
	ASSERT(self->wr_obj != (DeeObject *)WEAKREF_BAD_POINTER);
	ASSERT(other->wr_obj != (DeeObject *)WEAKREF_BAD_POINTER);
#endif /* !NDEBUG */
	if
		unlikely(self == other)
	return;
again:
	if (other->wr_obj) {
		WEAKREF_LOCK(other);
		COMPILER_READ_BARRIER();
		if
			likely(other->wr_obj)
		{
			if
				unlikely(!TRYLOCK_POINTER(*other->wr_pself))
			{
				WEAKREF_UNLOCK(other);
				SCHED_YIELD();
				goto again;
			}
			if
				unlikely(!WEAKREF_TRYLOCK(self))
			{
				struct weakref **p = other->wr_pself;
				UNLOCK_POINTER(*other->wr_pself);
				WEAKREF_UNLOCK(other);
				if
					unlikely(p == &self->wr_next)
				return;
				WEAKREF_LOCK(self);
				WEAKREF_UNLOCK(self);
				goto again;
			}
			COMPILER_READ_BARRIER();
			if
				unlikely(self->wr_obj)
			{
				struct weakref *next;
				if
					unlikely(!TRYLOCK_POINTER(*self->wr_pself))
				{
#if 0
     struct weakref **block = self->wr_pself;
#endif
					struct weakref **p       = self->wr_pself;
					struct weakref **other_p = other->wr_pself;
					WEAKREF_UNLOCK(self);
					UNLOCK_POINTER(*other->wr_pself);
					WEAKREF_UNLOCK(other);
					if
						unlikely(p == other_p || p == &other->wr_next)
					return;
					SCHED_YIELD();
#if 0 /* Potential SEGFAULT */
     LOCK_POINTER(*block);
     UNLOCK_POINTER(*block);
#endif
					goto again;
				}
				next = (struct weakref *)GET_POINTER(self->wr_next);
				if (next) {
					if
						unlikely(!WEAKREF_TRYLOCK(next))
					{
						/* Prevent a deadlock. */
						WEAKREF_UNLOCK(*self->wr_pself);
						WEAKREF_UNLOCK(self);
						UNLOCK_POINTER(*other->wr_pself);
						WEAKREF_UNLOCK(other);
						if
							unlikely(next == other)
						return;
						SCHED_YIELD();
#if 0 /* Potential SEGFAULT */
      WEAKREF_LOCK(next);
      WEAKREF_UNLOCK(next);
#endif
						goto again;
					}
					next->wr_pself = self->wr_pself;
					WEAKREF_UNLOCK(next);
				}
				ATOMIC_WRITE(*self->wr_pself, next);
			}
			self->wr_pself                      = other->wr_pself;
			((struct weakref *)other)->wr_pself = &self->wr_next;
			WEAKREF_UNLOCK(other);
			ATOMIC_WRITE(self->wr_next, other);
			ATOMIC_WRITE(*self->wr_pself, self);
		} else {
			ATOMIC_WRITE(other->wr_next, NULL); /* WEAKREF_UNLOCK(other); */
			Dee_weakref_clear(self);
		}
	} else {
		Dee_weakref_clear(self);
	}
}

PUBLIC void DCALL
Dee_weakref_moveassign(struct weakref *__restrict self,
                       struct weakref *__restrict other) {
	ASSERT(self);
	ASSERT(other);
#ifndef NDEBUG
	ASSERT(other->wr_obj != (DeeObject *)WEAKREF_BAD_POINTER);
#endif /* !NDEBUG */
	if
		unlikely(self == other)
	return;
again:
#ifndef NDEBUG
	self->wr_pself = (struct weakref **)WEAKREF_BAD_POINTER;
	self->wr_next  = (struct weakref *)WEAKREF_BAD_POINTER;
#endif /* !NDEBUG */
	if (other->wr_obj) {
		WEAKREF_LOCK(other);
		COMPILER_READ_BARRIER();
		if
			likely(other->wr_obj)
		{
			struct weakref *next;
			if (!TRYLOCK_POINTER(*other->wr_pself)) {
				WEAKREF_UNLOCK(other);
				SCHED_YIELD();
				goto again;
			}
			if
				unlikely(!WEAKREF_TRYLOCK(self))
			{
				struct weakref **p = other->wr_pself;
				UNLOCK_POINTER(*other->wr_pself);
				WEAKREF_UNLOCK(other);
				if
					unlikely(p == &self->wr_next)
				return;
				WEAKREF_LOCK(self);
				WEAKREF_UNLOCK(self);
				goto again;
			}
			COMPILER_READ_BARRIER();
			if
				unlikely(self->wr_obj)
			{
				struct weakref *next;
				if
					unlikely(!TRYLOCK_POINTER(*self->wr_pself))
				{
#if 0
     struct weakref **block = self->wr_pself;
#endif
					struct weakref **p       = self->wr_pself;
					struct weakref **other_p = other->wr_pself;
					WEAKREF_UNLOCK(self);
					UNLOCK_POINTER(*other->wr_pself);
					WEAKREF_UNLOCK(other);
					if
						unlikely(p == other_p || p == &other->wr_next)
					return;
					SCHED_YIELD();
#if 0 /* Potential SEGFAULT */
     LOCK_POINTER(*block);
     UNLOCK_POINTER(*block);
#endif
					goto again;
				}
				next = (struct weakref *)GET_POINTER(self->wr_next);
				if (next) {
					if
						unlikely(!WEAKREF_TRYLOCK(next))
					{
						/* Prevent a deadlock. */
						WEAKREF_UNLOCK(*self->wr_pself);
						WEAKREF_UNLOCK(self);
						UNLOCK_POINTER(*other->wr_pself);
						WEAKREF_UNLOCK(other);
						if
							unlikely(next == other)
						return;
						SCHED_YIELD();
#if 0 /* Potential SEGFAULT */
      WEAKREF_LOCK(next);
      WEAKREF_UNLOCK(next);
#endif
						goto again;
					}
					next->wr_pself = self->wr_pself;
					WEAKREF_UNLOCK(next);
				}
				ATOMIC_WRITE(*self->wr_pself, next);
			}
			next           = (struct weakref *)GET_POINTER(other->wr_next);
			self->wr_pself = other->wr_pself;
			self->wr_next  = next;
			if (next) {
				if
					unlikely(!WEAKREF_TRYLOCK(next))
				{
					/* Prevent a deadlock. */
					WEAKREF_UNLOCK(*other->wr_pself);
					WEAKREF_UNLOCK(other);
					SCHED_YIELD();
					goto again;
				}
				next->wr_pself = &self->wr_next;
				ATOMIC_WRITE(*self->wr_pself, self);
				WEAKREF_UNLOCK(next);
			} else {
				ATOMIC_WRITE(*self->wr_pself, self);
			}
			/*WEAKREF_UNLOCK(other);*/
		} else {
			/*WEAKREF_UNLOCK(other);*/
			self->wr_obj = NULL;
		}
	} else {
		self->wr_obj = NULL;
	}
}

PUBLIC void DCALL
Dee_weakref_move(struct weakref *__restrict dst,
                 struct weakref *__restrict src) {
	ASSERT(dst);
	ASSERT(src);
#ifndef NDEBUG
	ASSERT(src->wr_obj != (DeeObject *)WEAKREF_BAD_POINTER);
#endif /* !NDEBUG */
	dst->wr_del = src->wr_del;
again:
#ifndef NDEBUG
	dst->wr_pself = (struct weakref **)WEAKREF_BAD_POINTER;
	dst->wr_next  = (struct weakref *)WEAKREF_BAD_POINTER;
#endif /* !NDEBUG */
	if (src->wr_obj) {
		WEAKREF_LOCK(src);
		COMPILER_READ_BARRIER();
		if
			likely(src->wr_obj)
		{
			struct weakref *next;
			LOCK_POINTER(*src->wr_pself);
			next          = (struct weakref *)GET_POINTER(src->wr_next);
			dst->wr_pself = src->wr_pself;
			dst->wr_next  = next;
			if (next) {
				if
					unlikely(!WEAKREF_TRYLOCK(next))
				{
					/* Prevent a deadlock. */
					WEAKREF_UNLOCK(*src->wr_pself);
					WEAKREF_UNLOCK(src);
					SCHED_YIELD();
					goto again;
				}
				next->wr_pself = &dst->wr_next;
				ATOMIC_WRITE(*dst->wr_pself, dst);
				WEAKREF_UNLOCK(next);
			} else {
				ATOMIC_WRITE(*dst->wr_pself, dst);
			}
			/*WEAKREF_UNLOCK(src);*/
		} else {
			/*WEAKREF_UNLOCK(src);*/
			dst->wr_obj = NULL;
		}
	} else {
		dst->wr_obj = NULL;
	}
}

PUBLIC void DCALL
Dee_weakref_fini(struct weakref *__restrict self) {
	ASSERT(self);
#ifndef NDEBUG
	ASSERT(self->wr_obj != (DeeObject *)WEAKREF_BAD_POINTER);
#endif /* !NDEBUG */
again:
	if (self->wr_obj) {
		WEAKREF_LOCK(self);
		COMPILER_READ_BARRIER();
		if
			likely(self->wr_obj)
		{
			struct weakref *next;
			LOCK_POINTER(*self->wr_pself);
			next = (struct weakref *)GET_POINTER(self->wr_next);
			if (next) {
				if
					unlikely(!WEAKREF_TRYLOCK(next))
				{
					/* Prevent a deadlock. */
					WEAKREF_UNLOCK(*self->wr_pself);
					WEAKREF_UNLOCK(self);
					SCHED_YIELD();
					goto again;
				}
				next->wr_pself = self->wr_pself;
				WEAKREF_UNLOCK(next);
			}
			ATOMIC_WRITE(*self->wr_pself, next);
		}
		/*WEAKREF_UNLOCK(self);*/
	}
#ifndef NDEBUG
	self->wr_pself = (struct weakref **)WEAKREF_BAD_POINTER;
	self->wr_next  = (struct weakref *)WEAKREF_BAD_POINTER;
	self->wr_obj   = (DeeObject *)WEAKREF_BAD_POINTER;
#endif /* !NDEBUG */
}

PUBLIC bool DCALL
Dee_weakref_clear(struct weakref *__restrict self) {
	ASSERT(self);
#ifndef NDEBUG
	ASSERT(self->wr_obj != (DeeObject *)WEAKREF_BAD_POINTER);
#endif /* !NDEBUG */
again:
	if (self->wr_obj) {
		WEAKREF_LOCK(self);
		COMPILER_READ_BARRIER();
		if (self->wr_obj) {
			struct weakref *next;
			if
				unlikely(!TRYLOCK_POINTER(*self->wr_pself))
			{
				WEAKREF_UNLOCK(self);
				SCHED_YIELD();
				goto again;
			}
			next = (struct weakref *)GET_POINTER(self->wr_next);
			if (next) {
				if
					unlikely(!WEAKREF_TRYLOCK(next))
				{
					/* Prevent a deadlock. */
					WEAKREF_UNLOCK(*self->wr_pself);
					WEAKREF_UNLOCK(self);
					SCHED_YIELD();
					goto again;
				}
				next->wr_pself = self->wr_pself;
				WEAKREF_UNLOCK(next);
			}
			ATOMIC_WRITE(*self->wr_pself, next);
			self->wr_obj = NULL;
		}
#ifndef NDEBUG
		self->wr_pself = (struct weakref **)WEAKREF_BAD_POINTER;
		ATOMIC_WRITE(self->wr_next, (struct weakref *)((uintptr_t)WEAKREF_BAD_POINTER & PTRLOCK_ADDR_MASK));
#else  /* !NDEBUG */
		ATOMIC_WRITE(self->wr_next, NULL);
#endif /* NDEBUG */
		return true;
	}
	return false;
}


/* Overwrite an already initialize weak reference with the given `ob'.
 * @return: true:    Successfully overwritten the weak reference.
 * @return: false:   The given object `ob' does not support weak referencing
 *                   and the stored weak reference was not modified. */
PUBLIC bool DCALL
Dee_weakref_set(struct weakref *__restrict self,
                DeeObject *__restrict ob) {
	struct weakref_list *new_list;
	struct weakref *next;
	ASSERT(self);
	ASSERT(ob);
	ASSERT(ob->ob_refcnt);
	ASSERT(IS_ALIGNED((uintptr_t)self, PTRLOCK_LOCK_MASK + 1));
	new_list = WEAKREFS_GET(ob);
	if
		unlikely(!WEAKREFS_OK(new_list, ob))
	return false;
again:
	WEAKREF_LOCK(self);
	if
		unlikely(ob == self->wr_obj)
	{
		/* Still the same object. */
		WEAKREF_UNLOCK(self);
	} else {
		/* Delete a previously assigned object. */
		if (self->wr_obj) {
			if
				unlikely(!TRYLOCK_POINTER(*self->wr_pself))
			{
				WEAKREF_UNLOCK(self);
				SCHED_YIELD();
				goto again;
			}
			next = (struct weakref *)GET_POINTER(self->wr_next);
			if (next) {
				if
					unlikely(!WEAKREF_TRYLOCK(next))
				{
					/* Prevent a deadlock. */
					WEAKREF_UNLOCK(*self->wr_pself);
					WEAKREF_UNLOCK(self);
					SCHED_YIELD();
					goto again;
				}
				next->wr_pself = self->wr_pself;
				WEAKREF_UNLOCK(next);
			}
			ATOMIC_WRITE(*self->wr_pself, next);
		}
		/* Now to re-insert the weakref. */
		self->wr_pself = &new_list->wl_nodes;
		self->wr_obj   = ob;
		LOCK_POINTER(new_list->wl_nodes);
		next = (struct weakref *)GET_POINTER(new_list->wl_nodes);
		if (next) {
			/* Fix the self-pointer of the next object. */
			WEAKREF_LOCK(next);
			next->wr_pself = &self->wr_next;
			ATOMIC_WRITE(self->wr_next, next);
			WEAKREF_UNLOCK(next);
		} else {
			ATOMIC_WRITE(self->wr_next, next);
		}
		ATOMIC_WRITE(new_list->wl_nodes, self);
	}
	return true;
}

/* Lock a weak reference, returning a regular reference to the pointed-to object.
 * @return: * :   A new reference to the pointed-to object.
 * @return: NULL: Failed to lock the weak reference. */
#ifdef __INTELLISENSE__
PUBLIC DREF DeeObject *(DCALL Dee_weakref_lock)(struct weakref const *__restrict self)
#else  /* __INTELLISENSE__ */
PUBLIC DREF DeeObject *(DCALL Dee_weakref_lock)(struct weakref *__restrict self)
#endif /* !__INTELLISENSE__ */
{
	DREF DeeObject *result;
	ASSERT(self);
	ASSERT(IS_ALIGNED((uintptr_t)self, PTRLOCK_LOCK_MASK + 1));
#ifndef NDEBUG
	ASSERT(self->wr_obj != (DeeObject *)WEAKREF_BAD_POINTER);
#endif /* !NDEBUG */
	if ((result = self->wr_obj) != NULL) {
		WEAKREF_LOCK(self);
		COMPILER_READ_BARRIER();
		result = self->wr_obj; /* Re-read in case it changed. */
#ifdef CONFIG_NO_THREADS
		if
			likely(result->ob_refcnt != 0)
		++result->ob_refcnt;
		else result = NULL;
#else  /* CONFIG_NO_THREADS */
		{
			/* Do an atomic-inc-if-not-zero on the reference counter. */
			dref_t refcnt;
			do {
				refcnt = ATOMIC_READ(result->ob_refcnt);
				if (!refcnt) {
					result = NULL;
					break;
				}
			} while (!ATOMIC_CMPXCH_WEAK(result->ob_refcnt, refcnt, refcnt + 1));
		}
#endif /* !CONFIG_NO_THREADS */
		WEAKREF_UNLOCK(self);
	}
	return result;
}

/* Return the state of a snapshot of `self' currently being bound. */
#ifdef __INTELLISENSE__
PUBLIC bool(DCALL Dee_weakref_bound)(struct weakref const *__restrict self)
#else  /* __INTELLISENSE__ */
PUBLIC bool(DCALL Dee_weakref_bound)(struct weakref *__restrict self)
#endif /* !__INTELLISENSE__ */
{
	DeeObject *curr;
	ASSERT(self);
	ASSERT(IS_ALIGNED((uintptr_t)self, PTRLOCK_LOCK_MASK + 1));
#ifndef NDEBUG
	ASSERT(self->wr_obj != (DeeObject *)WEAKREF_BAD_POINTER);
#endif /* !NDEBUG */
	if ((curr = self->wr_obj) != NULL) {
		WEAKREF_LOCK(self);
		COMPILER_READ_BARRIER();
		curr = self->wr_obj; /* Re-read in case it changed. */
#ifdef CONFIG_NO_THREADS
		if (curr->ob_refcnt == 0)
#else  /* CONFIG_NO_THREADS */
		if (ATOMIC_READ(curr->ob_refcnt) == 0)
#endif /* !CONFIG_NO_THREADS */
		{
			WEAKREF_UNLOCK(self);
			return false;
		}
		WEAKREF_UNLOCK(self);
	}
	return true;
}

/* Do an atomic compare-exchange operation on the weak reference
 * and return a reference to the previously assigned object, or
 * `NULL' when none was assigned, or `ITER_DONE' when `new_ob'
 * does not support weak referencing functionality.
 * NOTE: You may pass `NULL' for `new_ob' to clear the the weakref. */
PUBLIC DREF DeeObject *DCALL
Dee_weakref_cmpxch(struct weakref *__restrict self,
                   DeeObject *old_ob,
                   DeeObject *new_ob) {
	DREF DeeObject *result;
	ASSERT(self);
	ASSERT_OBJECT_OPT(new_ob);
	ASSERT(IS_ALIGNED((uintptr_t)self, PTRLOCK_LOCK_MASK + 1));
again:
	WEAKREF_LOCK(self);
	result = self->wr_obj;
#ifndef NDEBUG
	ASSERT(result != (DeeObject *)WEAKREF_BAD_POINTER);
#endif /* !NDEBUG */
	if (result == old_ob) {
		/* Do the exchange. */
		if (!new_ob) {
			/* Clear the object. */
			if
				unlikely(!old_ob)
			{
				WEAKREF_UNLOCK(self);
			} else {
				struct weakref *next;
				/* Delete a previously assigned object. */
				if
					unlikely(!TRYLOCK_POINTER(*self->wr_pself))
				{
					WEAKREF_UNLOCK(self);
					SCHED_YIELD();
					goto again;
				}
				next = (struct weakref *)GET_POINTER(self->wr_next);
				if (next) {
					if
						unlikely(!WEAKREF_TRYLOCK(next))
					{
						/* Prevent a deadlock. */
						WEAKREF_UNLOCK(*self->wr_pself);
						WEAKREF_UNLOCK(self);
						SCHED_YIELD();
						goto again;
					}
					next->wr_pself = self->wr_pself;
					WEAKREF_UNLOCK(next);
				}
				ATOMIC_WRITE(*self->wr_pself, next);
				/* Now to re-insert the weakref. */
				self->wr_obj = NULL;
#ifndef NDEBUG
				self->wr_pself = (struct weakref **)WEAKREF_BAD_POINTER;
				ATOMIC_WRITE(self->wr_next, (struct weakref *)((uintptr_t)WEAKREF_BAD_POINTER & PTRLOCK_ADDR_MASK));
#else  /* !NDEBUG */
				ATOMIC_WRITE(self->wr_next, NULL);
#endif /* NDEBUG */
			}
		} else {
			struct weakref_list *new_list;
			new_list = WEAKREFS_GET(new_ob);
			if
				unlikely(!WEAKREFS_OK(new_list, new_ob))
			{
				WEAKREF_UNLOCK(self);
				/* Weak referencing is not supported. */
				return ITER_DONE;
			} else if unlikely(old_ob == new_ob)
			{
				WEAKREF_UNLOCK(self);
			} else {
				struct weakref *next;
				/* Delete a previously assigned object. */
				if (old_ob) {
					LOCK_POINTER(*self->wr_pself);
					next = (struct weakref *)GET_POINTER(self->wr_next);
					if (next) {
						if
							unlikely(!WEAKREF_TRYLOCK(next))
						{
							/* Prevent a deadlock. */
							WEAKREF_UNLOCK(*self->wr_pself);
							WEAKREF_UNLOCK(self);
							SCHED_YIELD();
							goto again;
						}
						next->wr_pself = self->wr_pself;
						WEAKREF_UNLOCK(next);
					}
					ATOMIC_WRITE(*self->wr_pself, next);
				}
				/* Now to re-insert the weakref. */
				self->wr_pself = &new_list->wl_nodes;
				self->wr_obj   = new_ob;
				LOCK_POINTER(new_list->wl_nodes);
				next = (struct weakref *)GET_POINTER(new_list->wl_nodes);
				if (next) {
					/* Fix the self-pointer of the next object. */
					WEAKREF_LOCK(next);
					next->wr_pself = &self->wr_next;
					ATOMIC_WRITE(self->wr_next, next);
					WEAKREF_UNLOCK(next);
				} else {
					ATOMIC_WRITE(self->wr_next, next);
				}
				ATOMIC_WRITE(new_list->wl_nodes, self);
			}
		}
	} else if (result != NULL) {
#if 0 /* Can't happen, because we're locking the weakref */
		COMPILER_READ_BARRIER();
		result = self->wr_obj; /* Re-read in case it changed. */
#endif
#ifdef CONFIG_NO_THREADS
		if
			likely(result->ob_refcnt != 0)
		++result->ob_refcnt;
		else {
			result = NULL;
		}
#else  /* CONFIG_NO_THREADS */
		{
			/* Do an atomic-inc-if-not-zero on the reference counter. */
			dref_t refcnt;
			do {
				refcnt = ATOMIC_READ(result->ob_refcnt);
				if (!refcnt) {
					result = NULL;
					break;
				}
			} while (!ATOMIC_CMPXCH_WEAK(result->ob_refcnt, refcnt, refcnt + 1));
		}
#endif /* !CONFIG_NO_THREADS */
		WEAKREF_UNLOCK(self);
	}
	return result;
}



PUBLIC DREF DeeObject *(DCALL DeeObject_NewRef)(DeeObject *__restrict self) {
	ASSERT_OBJECT(self);
	Dee_Incref(self);
	return self;
}

PUBLIC bool DCALL
DeeObject_UndoConstruction(DeeTypeObject *undo_start,
                           DeeObject *__restrict self) {
	if
		unlikely(!ATOMIC_CMPXCH(self->ob_refcnt, 1, 0))
	return false;
	for (;; undo_start = DeeType_Base(undo_start)) {
		if (!undo_start)
			break;
		if (undo_start->tp_init.tp_dtor) {
			/* Update the object's typing to mirror what is written here.
			 * NOTE: We're allowed to modify the type of `self' _ONLY_ because
			 *       it's reference counter is ZERO (aka: the object isn't shared
			 *       and also can't be revived by weak references, which don't
			 *       allow locking once the object's reference counter has hit ZERO). */
			self->ob_type = undo_start;
			COMPILER_WRITE_BARRIER();
			(*undo_start->tp_init.tp_dtor)(self);
			COMPILER_READ_BARRIER();
			/* Special case: The destructor managed to revive the object. */
			{
				dref_t refcnt;
				do {
					refcnt = ATOMIC_READ(self->ob_refcnt);
					if (refcnt == 0)
						goto destroy_weak;
				} while (ATOMIC_CMPXCH(self->ob_refcnt, refcnt, refcnt + 1));
				return false;
			}
		}
destroy_weak:
		/* Delete all weak references linked against this type level. */
		if (has_noninherited_weakrefs(undo_start)) {
			struct weakref *iter, *next;
			struct weakref_list *list;
			ASSERT(undo_start->tp_weakrefs >= sizeof(DeeObject));
			list = (struct weakref_list *)((uintptr_t)self + undo_start->tp_weakrefs);
restart_clear_weakrefs:
			LOCK_POINTER(list->wl_nodes);
			if ((iter = (struct weakref *)GET_POINTER(list->wl_nodes)) != NULL) {
				if (!WEAKREF_TRYLOCK(iter)) {
					/* Prevent deadlock. */
					UNLOCK_POINTER(list->wl_nodes);
					SCHED_YIELD();
					goto restart_clear_weakrefs;
				}
				ASSERT(iter->wr_pself == &list->wl_nodes);
				next = (struct weakref *)GET_POINTER(iter->wr_next);
				if (next) {
					if (!WEAKREF_TRYLOCK(next)) {
						/* Prevent deadlock. */
						WEAKREF_UNLOCK(iter);
						UNLOCK_POINTER(list->wl_nodes);
						SCHED_YIELD();
						goto restart_clear_weakrefs;
					}
					next->wr_pself = &list->wl_nodes;
					WEAKREF_UNLOCK(next);
				}
				/* Overwrite the weakly referenced object with NULL,
				 * indicating that the link has been severed. */
				ATOMIC_WRITE(iter->wr_obj, NULL);
				ATOMIC_WRITE(list->wl_nodes, next);
				if (iter->wr_del) {
					(*iter->wr_del)(iter);
				} else {
#ifndef NDEBUG
					iter->wr_pself = (struct weakref **)WEAKREF_BAD_POINTER;
					ATOMIC_WRITE(iter->wr_next, (struct weakref *)((uintptr_t)WEAKREF_BAD_POINTER & PTRLOCK_ADDR_MASK));
#else  /* !NDEBUG */
					ATOMIC_WRITE(iter->wr_next, NULL);
#endif /* NDEBUG */
				}
				goto restart_clear_weakrefs;
			}
		}
	}
	return true;
}

#ifndef CONFIG_NO_BADREFCNT_CHECKS
#ifdef _MSC_VER
#define FILE_AND_LINE_FORMAT "%s(%d) : "
#else /* _MSC_VER */
#define FILE_AND_LINE_FORMAT "%s:%d: "
#endif /* !_MSC_VER */

#ifdef CONFIG_NO_THREADS
#define BADREFCNT_BEGIN() (void)0
#define BADREFCNT_END()   (void)0
#else /* CONFIG_NO_THREADS */
PRIVATE DEFINE_RWLOCK(bad_refcnt_lock);
#define BADREFCNT_BEGIN() rwlock_write(&bad_refcnt_lock)
#define BADREFCNT_END()   rwlock_endwrite(&bad_refcnt_lock)
#endif /* !CONFIG_NO_THREADS */

PUBLIC void DCALL
DeeFatal_BadIncref(DeeObject *__restrict ob,
                   char const *file, int line) {
	DeeTypeObject *type;
	BADREFCNT_BEGIN();
	DEE_DPRINTF("\n\n\n" FILE_AND_LINE_FORMAT "BAD_INCREF(%p)\n",
	            file, line, ob);
	DEE_DPRINTF("refcnt : %Iu (%IX)\n", ob->ob_refcnt, ob->ob_refcnt);
	type = Dee_TYPE(ob);
	if (DeeObject_Check(type) && DeeType_Check(type)) {
		DEE_DPRINTF("type : %s (%p)", type->tp_name, type);
	} else {
		DEE_DPRINTF("type : <INVALID> - %p", type);
	}
	DEE_DPRINTF("\n\n\n");
	BADREFCNT_END();
	BREAKPOINT();
}

PUBLIC void DCALL
DeeFatal_BadDecref(DeeObject *__restrict ob,
                   char const *file, int line) {
	DeeTypeObject *type;
	BADREFCNT_BEGIN();
	DEE_DPRINTF("\n\n\n" FILE_AND_LINE_FORMAT "BAD_DECREF(%p)\n",
	            file, line, ob);
	DEE_DPRINTF("refcnt : %Iu (%IX)\n", ob->ob_refcnt, ob->ob_refcnt);
	type = Dee_TYPE(ob);
	if (DeeObject_Check(type) && DeeType_Check(type)) {
		DEE_DPRINTF("type : %s (%p)", type->tp_name, type);
	} else {
		DEE_DPRINTF("type : <INVALID> - %p", type);
	}
	DEE_DPRINTF("\n\n\n");
	BADREFCNT_END();
	BREAKPOINT();
}
#else
PUBLIC void DCALL
DeeFatal_BadIncref(DeeObject *__restrict UNUSED(ob),
                   char const *UNUSED(file), int UNUSED(line)) {
	abort();
}
#ifdef __NO_DEFINE_ALIAS
PUBLIC void DCALL
DeeFatal_BadDecref(DeeObject *__restrict UNUSED(ob),
                   char const *UNUSED(file), int UNUSED(line)) {
	abort();
}
#else  /* __NO_DEFINE_ALIAS */
DEFINE_PUBLIC_ALIAS(ASSEMBLY_NAME(DeeFatal_BadDecref, 12),
                    ASSEMBLY_NAME(DeeFatal_BadIncref, 12));
#endif /* !__NO_DEFINE_ALIAS */
#endif


/* Finalize weakref support */
PUBLIC void
(DCALL Dee_weakref_support_fini)(struct weakref_list *__restrict list) {
	struct weakref *iter, *next;
restart_clear_weakrefs:
	LOCK_POINTER(list->wl_nodes);
	if ((iter = (struct weakref *)GET_POINTER(list->wl_nodes)) != NULL) {
		if (!WEAKREF_TRYLOCK(iter)) {
			/* Prevent deadlock. */
			UNLOCK_POINTER(list->wl_nodes);
			SCHED_YIELD();
			goto restart_clear_weakrefs;
		}
		ASSERT(iter->wr_pself == &list->wl_nodes);
		next = (struct weakref *)GET_POINTER(iter->wr_next);
		if (next) {
			if (!WEAKREF_TRYLOCK(next)) {
				/* Prevent deadlock. */
				WEAKREF_UNLOCK(iter);
				UNLOCK_POINTER(list->wl_nodes);
				SCHED_YIELD();
				goto restart_clear_weakrefs;
			}
			next->wr_pself = &list->wl_nodes;
			WEAKREF_UNLOCK(next);
		}
		/* Overwrite the weakly referenced object with NULL,
		 * indicating that the link has been severed. */
		ATOMIC_WRITE(iter->wr_obj, NULL);
		ATOMIC_WRITE(list->wl_nodes, next);
		if (iter->wr_del) {
			(*iter->wr_del)(iter);
		} else {
#ifndef NDEBUG
			iter->wr_pself = (struct weakref **)WEAKREF_BAD_POINTER;
			ATOMIC_WRITE(iter->wr_next, (struct weakref *)((uintptr_t)WEAKREF_BAD_POINTER & PTRLOCK_ADDR_MASK));
#else /* !NDEBUG */
			ATOMIC_WRITE(iter->wr_next, NULL);
#endif /* NDEBUG */
		}
		goto restart_clear_weakrefs;
	}
#if 1
	ATOMIC_WRITE(list->wl_nodes, NULL);
#else
	UNLOCK_POINTER(list->wl_nodes);
#endif
}


#ifdef CONFIG_NO_BADREFCNT_CHECKS
PUBLIC void (DCALL DeeObject_Destroy_d)(DeeObject *__restrict self,
                                        char const *UNUSED(file), int UNUSED(line)) {
	return DeeObject_Destroy(self);
}

PUBLIC void (DCALL DeeObject_Destroy)(DeeObject *__restrict self)
#else /* CONFIG_NO_BADREFCNT_CHECKS */
PUBLIC void (DCALL DeeObject_Destroy)(DeeObject *__restrict self) {
	return DeeObject_Destroy_d(self, NULL, 0);
}

PUBLIC void (DCALL DeeObject_Destroy_d)(DeeObject *__restrict self,
                                        char const *file, int line)
#endif /* !CONFIG_NO_BADREFCNT_CHECKS */
{
#undef CONFIG_OBJECT_DESTROY_CHECK_MEMORY
	//#define CONFIG_OBJECT_DESTROY_CHECK_MEMORY 1
	DeeTypeObject *orig_type, *type;
#ifndef CONFIG_TRACE_REFCHANGES
again:
#endif /* !CONFIG_TRACE_REFCHANGES */
	orig_type = type = Dee_TYPE(self);
#ifndef CONFIG_NO_BADREFCNT_CHECKS
	if (self->ob_refcnt != 0) {
		BADREFCNT_BEGIN();
		DEE_DPRINTF("\n\n\n" FILE_AND_LINE_FORMAT "BAD_DESTROY(%p)\n",
		            file, line, self);
		DEE_DPRINTF("refcnt : %Iu (%IX)\n", self->ob_refcnt, self->ob_refcnt);
		if (DeeObject_Check(type) && DeeType_Check(type)) {
			DEE_DPRINTF("type : %s (%p)", type->tp_name, type);
		} else {
			DEE_DPRINTF("type : <INVALID> - %p", type);
		}
		DEE_DPRINTF("\n\n\n");
		BADREFCNT_END();
		BREAKPOINT();
	}
#endif /* !CONFIG_NO_BADREFCNT_CHECKS */
#if 0
#ifndef CONFIG_NO_THREADS
	/* Make sure that all threads now see this object as dead.
	 * For why this is required, see `INCREF_IF_NONZERO()' */
	__hybrid_atomic_thread_fence(__ATOMIC_ACQ_REL);
#endif /* !CONFIG_NO_THREADS */
#endif
#ifdef CONFIG_OBJECT_DESTROY_CHECK_MEMORY
	DEE_CHECKMEMORY();
#endif /* CONFIG_OBJECT_DESTROY_CHECK_MEMORY */
	if (type->tp_flags & TP_FGC) {
		/* Special handling to track/untrack GC objects during destructor calls. */
		/* Start by untracking the object in question. */
		DeeGC_Untrack(self);
		for (;;) {
			ASSERT(self->ob_refcnt == 0);
			ASSERTF(type == orig_type || !(type->tp_flags & TP_FFINAL),
			        "Final type `%s' with sub-class `%s'",
			        type->tp_name, orig_type->tp_name);
			if (type->tp_init.tp_dtor) {
				/* Update the object's typing to mirror what is written here.
				 * NOTE: We're allowed to modify the type of `self' _ONLY_
				 *       because it's reference counter is ZERO (and because
				 *       implementors of `tp_free' are aware of its volatile
				 *       nature that may only be interpreted as a free-hint).
				 * NOTE: This even applies to the slab allocators used by `DeeObject_MALLOC'! */
				self->ob_type = type;
				COMPILER_WRITE_BARRIER();
				(*type->tp_init.tp_dtor)(self);
				COMPILER_READ_BARRIER();
#ifdef CONFIG_OBJECT_DESTROY_CHECK_MEMORY
				DEE_CHECKMEMORY();
#endif /* CONFIG_OBJECT_DESTROY_CHECK_MEMORY */
				/* Special case: The destructor managed to revive the object. */
				if
					unlikely(self->ob_refcnt != 0)
				{
					Dee_Incref(type);
					ASSERTF(type->tp_flags & TP_FGC,
					        "This runtime does not implementing reviving "
					        "GC-allocated objects as non-GC objects.");
					/* Continue tracking the object. */
					DeeGC_Track(self);
					/* As part of the revival process, `tp_dtor' has us inherit a reference to `self'
					 * in order to prevent a race condition that could otherwise occur when another
					 * thread would have cleared the external reference after the destructor created
					 * it, but before we were able to read out the fact that `ob_refcnt' was now
					 * non-zero. - If that were to happen, the other thread may also attempt to destroy
					 * the object, causing it to be destroyed in multiple threads at the same time,
					 * which is something that's not allowed! */
					Dee_Decref(orig_type);
#ifndef CONFIG_TRACE_REFCHANGES
					/* Same as below, but prevent recursion (after all: we're already inside of `DeeObject_Destroy()'!) */
					{
						dref_t oldref = ATOMIC_FETCHDEC(self->ob_refcnt);
						ASSERTF(oldref != 0,
						        "Upon revival, a destructor must let the caller inherit a "
						        "reference (which may appear like a leak, but actually isn't)");
						if (oldref == 1)
							goto again;
					}
#else /* !CONFIG_TRACE_REFCHANGES */
					Dee_Decref(self);
#endif /* CONFIG_TRACE_REFCHANGES */
					return;
				}
			}
			/* Drop the reference held by this type.
			 * NOTE: Keep the reference to `orig_type' alive, though! */
			if ((type = type->tp_base) == NULL)
				break;
		}
#ifdef CONFIG_TRACE_REFCHANGES
		free_reftracker(self->ob_trace);
#endif /* CONFIG_TRACE_REFCHANGES */
		if (orig_type->tp_init.tp_alloc.tp_free)
			(*orig_type->tp_init.tp_alloc.tp_free)(self);
		else {
			DeeGCObject_Free(self);
		}
	} else {
		for (;;) {
			ASSERT(self->ob_refcnt == 0);
			ASSERTF(type == orig_type || !(type->tp_flags & TP_FFINAL),
			        "Final type `%s' with sub-class `%s'",
			        type->tp_name, orig_type->tp_name);
			ASSERTF(!(type->tp_flags & TP_FGC),
			        "non-gc type `%s' derived from gc type `%s'",
			        orig_type->tp_name, type->tp_name);
			if (type->tp_init.tp_dtor) {
				/* Update the object's typing to mirror what is written here.
				 * NOTE: We're allowed to modify the type of `self' _ONLY_
				 *       because it's reference counter is ZERO (and because
				 *       implementors of `tp_free' are aware of its volatile
				 *       nature that may only be interpreted as a free-hint). */
				self->ob_type = type;
				COMPILER_WRITE_BARRIER();
				(*type->tp_init.tp_dtor)(self);
				COMPILER_READ_BARRIER();
#ifdef CONFIG_OBJECT_DESTROY_CHECK_MEMORY
				DEE_CHECKMEMORY();
#endif /* CONFIG_OBJECT_DESTROY_CHECK_MEMORY */
				/* Special case: The destructor managed to revive the object. */
				if
					unlikely(self->ob_refcnt != 0)
				{
					/* Incref() the new type that now describes this revived object.
					 * NOTE: The fact that this type may use a different (or none at all)
					 *       tp_free function, is the reason why no GC-able type from who's
					 *       destruct a user-callback that can somehow get ahold of the
					 *       instance being destroyed (which is also possible for any
					 *       weakly referencable type), is allowed to assume that it
					 *       will actually be called, limiting its use to pre-allocated object
					 *       caches that allocate their instances using `DeeObject_Malloc'. */
					Dee_Incref(type);
					/* As part of the revival process, `tp_dtor' has us inherit a reference to `self'
					 * in order to prevent a race condition that could otherwise occur when another
					 * thread would have cleared the external reference after the destructor created
					 * it, but before we were able to read out the fact that `ob_refcnt' was now
					 * non-zero. - If that were to happen, the other thread may also attempt to destroy
					 * the object, causing it to be destroyed in multiple threads at the same time,
					 * which is something that's not allowed! */
					Dee_Decref(orig_type);
#ifndef CONFIG_TRACE_REFCHANGES
					/* Same as below, but prevent recursion (after all: we're already inside of `DeeObject_Destroy()'!) */
					{
						dref_t oldref = ATOMIC_FETCHDEC(self->ob_refcnt);
						ASSERTF(oldref != 0,
						        "Upon revival, a destructor must let the caller inherit a "
						        "reference (which may appear like a leak, but actually isn't)");
						if (oldref == 1)
							goto again;
					}
#else /* !CONFIG_TRACE_REFCHANGES */
					Dee_Decref(self);
#endif /* CONFIG_TRACE_REFCHANGES */
					return;
				}
			}
			/* Drop the reference held by this type.
			 * NOTE: Keep the reference to `orig_type' alive, though! */
			if ((type = type->tp_base) == NULL)
				break;
		}
#ifdef CONFIG_TRACE_REFCHANGES
		free_reftracker(self->ob_trace);
#endif /* CONFIG_TRACE_REFCHANGES */
		/* Invoke `tp_free' using the original type */
		if (orig_type->tp_init.tp_alloc.tp_free)
			(*orig_type->tp_init.tp_alloc.tp_free)(self);
		else {
			DeeObject_Free(self);
		}
	}
	/*done:*/
	/* Drop a reference from the original type. */
	Dee_Decref(orig_type);
}


PRIVATE int DCALL
object_ctor(DeeObject *__restrict UNUSED(self)) {
	return 0;
}

PRIVATE int DCALL
object_copy_ctor(DeeObject *__restrict UNUSED(self),
                 DeeObject *__restrict UNUSED(other)) {
	return 0;
}

PRIVATE int DCALL
object_any_ctor(DeeObject *__restrict UNUSED(self),
                size_t argc, DeeObject **__restrict argv) {
	return DeeArg_Unpack(argc, argv, ":Object");
}

PRIVATE DREF DeeObject *DCALL object_str(DeeObject *__restrict self) {
#if 1
	DeeTypeObject *tp_self = Dee_TYPE(self);
	if (tp_self->tp_name) {
		if (tp_self->tp_flags & TP_FNAMEOBJECT) {
			DREF DeeStringObject *result;
			result = COMPILER_CONTAINER_OF(tp_self->tp_name,
			                               DeeStringObject,
			                               s_str);
			Dee_Incref(result);
			return (DREF DeeObject *)result;
		}
		return DeeString_New(tp_self->tp_name);
	}
	Dee_Incref(&str_Object);
	return &str_Object;
#else
	if (self->ob_type != &DeeObject_Type)
		goto err_noimp;
	Dee_Incref(&str_Object);
	return &str_Object;
err_noimp:
	err_unimplemented_operator(self->ob_type, OPERATOR_STR);
	return NULL;
#endif
}

PRIVATE DREF DeeObject *DCALL object_repr(DeeObject *__restrict self) {
	if (self->ob_type != &DeeObject_Type)
		goto err_noimp;
	Dee_Incref(&str_Object);
	return &str_Object;
err_noimp:
	err_unimplemented_operator(self->ob_type, OPERATOR_REPR);
	return NULL;
}


/* Object operators through methods. */
PRIVATE char const meth_copy[]       = ":__copy__";
PRIVATE char const meth_deepcopy[]   = ":__deepcopy__";
PRIVATE char const meth_assign[]     = "o:__assign__";
PRIVATE char const meth_moveassign[] = "o:__moveassign__";
PRIVATE char const meth_str[]        = ":__str__";
PRIVATE char const meth_repr[]       = ":__repr__";
PRIVATE char const meth_bool[]       = ":__bool__";
PRIVATE char const meth_call[]       = "o:__call__";
PRIVATE char const meth_thiscall[]   = "o|o:__thiscall__";
PRIVATE char const meth_hash[]       = ":__hash__";
PRIVATE char const meth_int[]        = ":__int__";
PRIVATE char const meth_inv[]        = ":__inv__";
PRIVATE char const meth_pos[]        = ":__pos__";
PRIVATE char const meth_neg[]        = ":__neg__";
PRIVATE char const meth_add[]        = "o:__add__";
PRIVATE char const meth_sub[]        = "o:__sub__";
PRIVATE char const meth_mul[]        = "o:__mul__";
PRIVATE char const meth_div[]        = "o:__div__";
PRIVATE char const meth_mod[]        = "o:__mod__";
PRIVATE char const meth_shl[]        = "o:__shl__";
PRIVATE char const meth_shr[]        = "o:__shr__";
PRIVATE char const meth_and[]        = "o:__and__";
PRIVATE char const meth_or[]         = "o:__or__";
PRIVATE char const meth_xor[]        = "o:__xor__";
PRIVATE char const meth_pow[]        = "o:__pow__";
PRIVATE char const meth_eq[]         = "o:__eq__";
PRIVATE char const meth_ne[]         = "o:__ne__";
PRIVATE char const meth_lo[]         = "o:__lo__";
PRIVATE char const meth_le[]         = "o:__le__";
PRIVATE char const meth_gr[]         = "o:__gr__";
PRIVATE char const meth_ge[]         = "o:__ge__";
PRIVATE char const meth_size[]       = ":__size__";
PRIVATE char const meth_contains[]   = "o:__contains__";
PRIVATE char const meth_getitem[]    = "o:__getitem__";
PRIVATE char const meth_delitem[]    = "o:__delitem__";
PRIVATE char const meth_setitem[]    = "oo:__setitem__";
PRIVATE char const meth_getrange[]   = "oo:__getrange__";
PRIVATE char const meth_delrange[]   = "oo:__delrange__";
PRIVATE char const meth_setrange[]   = "ooo:__setrange__";
PRIVATE char const meth_iterself[]   = ":__iter__";
PRIVATE char const meth_iternext[]   = ":__next__";
PRIVATE char const meth_getattr[]    = "o:__getattr__";
PRIVATE char const meth_callattr[]   = "__callattr__";
PRIVATE char const meth_hasattr[]    = "o:__hasattr__";
PRIVATE char const meth_delattr[]    = "o:__delattr__";
PRIVATE char const meth_setattr[]    = "oo:__setattr__";
PRIVATE char const meth_enumattr[]   = ":__enumattr__";

PRIVATE DREF DeeObject *DCALL
object_sizeof(DeeObject *__restrict self,
              size_t argc, DeeObject **__restrict argv) {
	DeeTypeObject *type;
	if (DeeArg_Unpack(argc, argv, ":__sizeof__"))
		goto err;
	/* Individual sub-types should override this function and add the proper value.
	 * This implementation is merely used for any generic fixed-length type that
	 * doesn't do any custom heap allocations. */
	type = Dee_TYPE(self);
	/* Variable types lack a standardized way of determining their size in bytes. */
	if
		unlikely(type->tp_flags & TP_FVARIABLE)
	goto err_isvar;
	if
		unlikely(type->tp_init.tp_alloc.tp_free)
	{
#ifndef CONFIG_NO_OBJECT_SLABS
		/* Check for slab allocators. */
		size_t slab_size;
		void(DCALL * tp_free)(void *__restrict ob);
		tp_free = type->tp_init.tp_alloc.tp_free;
#define CHECK_SIZE(index, size)                     \
		if (tp_free == &DeeObject_SlabFree##size || \
		    tp_free == &DeeGCObject_SlabFree##size) \
			slab_size = size * __SIZEOF_POINTER__;  \
		else
		DeeSlab_ENUMERATE(CHECK_SIZE)
#undef CHECK_SIZE
		{
			goto err_iscustom;
		}
		return DeeInt_NewSize(slab_size);
#else  /* !CONFIG_NO_OBJECT_SLABS */
		goto err_iscustom;
#endif /* CONFIG_NO_OBJECT_SLABS */
	}
	return DeeInt_NewSize(type->tp_init.tp_alloc.tp_instance_size);
err_iscustom:
	DeeError_Throwf(&DeeError_TypeError,
	                "Cannot determine size of Type `%k' with custom allocator",
	                type);
	goto err;
err_isvar:
	DeeError_Throwf(&DeeError_TypeError,
	                "Cannot determine size of variable-length Type `%k'",
	                type);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
object_copy(DeeObject *__restrict self,
            size_t argc, DeeObject **__restrict argv) {
	if (DeeArg_Unpack(argc, argv, meth_copy))
		goto err;
	return DeeObject_Copy(self);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
object_deepcopy(DeeObject *__restrict self,
                size_t argc, DeeObject **__restrict argv) {
	if (DeeArg_Unpack(argc, argv, meth_deepcopy))
		goto err;
	return DeeObject_DeepCopy(self);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
object_assign(DeeObject *__restrict self,
              size_t argc, DeeObject **__restrict argv) {
	DeeObject *other;
	if (DeeArg_Unpack(argc, argv, meth_assign, &other))
		goto err;
	if (DeeObject_Assign(self, other))
		goto err;
	return_reference_(self);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
object_moveassign(DeeObject *__restrict self,
                  size_t argc, DeeObject **__restrict argv) {
	DeeObject *other;
	if (DeeArg_Unpack(argc, argv, meth_moveassign, &other))
		goto err;
	if (DeeObject_AssertType(other, Dee_TYPE(self)))
		goto err;
	if (DeeObject_MoveAssign(self, other))
		goto err;
	return_reference_(self);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
object_dostr(DeeObject *__restrict self,
             size_t argc, DeeObject **__restrict argv) {
	if (DeeArg_Unpack(argc, argv, meth_str))
		goto err;
	return DeeObject_Str(self);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
object_dorepr(DeeObject *__restrict self,
              size_t argc, DeeObject **__restrict argv) {
	if (DeeArg_Unpack(argc, argv, meth_repr))
		goto err;
	return DeeObject_Repr(self);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
object_bool(DeeObject *__restrict self,
            size_t argc, DeeObject **__restrict argv) {
	int result;
	if (DeeArg_Unpack(argc, argv, meth_bool))
		goto err;
	result = DeeObject_Bool(self);
	if
		unlikely(result < 0)
	goto err;
	return_bool_(result);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
object_call(DeeObject *__restrict self,
            size_t argc, DeeObject **__restrict argv) {
	DeeObject *args_tuple;
	if (DeeArg_Unpack(argc, argv, meth_call, &args_tuple))
		goto err;
	if (DeeObject_AssertTypeExact(args_tuple, &DeeTuple_Type))
		goto err;
	return DeeObject_Call(self,
	                      DeeTuple_SIZE(args_tuple),
	                      DeeTuple_ELEM(args_tuple));
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
object_thiscall(DeeObject *__restrict self,
                size_t argc, DeeObject **__restrict argv) {
	DeeObject *this_arg;
	DeeObject *args_tuple = Dee_EmptyTuple;
	if (DeeArg_Unpack(argc, argv, meth_thiscall, &this_arg, &args_tuple))
		goto err;
	if (DeeObject_AssertTypeExact(args_tuple, &DeeTuple_Type))
		goto err;
	return DeeObject_ThisCall(self, this_arg,
	                          DeeTuple_SIZE(args_tuple),
	                          DeeTuple_ELEM(args_tuple));
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
object_hash(DeeObject *__restrict self,
            size_t argc, DeeObject **__restrict argv) {
	if (DeeArg_Unpack(argc, argv, meth_hash))
		goto err;
	return DeeInt_NewSize(DeeObject_Hash(self));
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
object_int(DeeObject *__restrict self,
           size_t argc, DeeObject **__restrict argv) {
	if (DeeArg_Unpack(argc, argv, meth_int))
		goto err;
	return DeeObject_Int(self);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
object_inv(DeeObject *__restrict self,
           size_t argc, DeeObject **__restrict argv) {
	if (DeeArg_Unpack(argc, argv, meth_inv))
		goto err;
	return DeeObject_Inv(self);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
object_pos(DeeObject *__restrict self,
           size_t argc, DeeObject **__restrict argv) {
	if (DeeArg_Unpack(argc, argv, meth_pos))
		goto err;
	return DeeObject_Pos(self);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
object_neg(DeeObject *__restrict self,
           size_t argc, DeeObject **__restrict argv) {
	if (DeeArg_Unpack(argc, argv, meth_neg))
		goto err;
	return DeeObject_Neg(self);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
object_add(DeeObject *__restrict self,
           size_t argc, DeeObject **__restrict argv) {
	DeeObject *other;
	if (DeeArg_Unpack(argc, argv, meth_add, &other))
		goto err;
	return DeeObject_Add(self, other);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
object_sub(DeeObject *__restrict self,
           size_t argc, DeeObject **__restrict argv) {
	DeeObject *other;
	if (DeeArg_Unpack(argc, argv, meth_sub, &other))
		goto err;
	return DeeObject_Sub(self, other);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
object_mul(DeeObject *__restrict self,
           size_t argc, DeeObject **__restrict argv) {
	DeeObject *other;
	if (DeeArg_Unpack(argc, argv, meth_mul, &other))
		goto err;
	return DeeObject_Mul(self, other);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
object_div(DeeObject *__restrict self,
           size_t argc, DeeObject **__restrict argv) {
	DeeObject *other;
	if (DeeArg_Unpack(argc, argv, meth_div, &other))
		goto err;
	return DeeObject_Div(self, other);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
object_mod(DeeObject *__restrict self,
           size_t argc, DeeObject **__restrict argv) {
	DeeObject *other;
	if (DeeArg_Unpack(argc, argv, meth_mod, &other))
		goto err;
	return DeeObject_Mod(self, other);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
object_shl(DeeObject *__restrict self,
           size_t argc, DeeObject **__restrict argv) {
	DeeObject *other;
	if (DeeArg_Unpack(argc, argv, meth_shl, &other))
		goto err;
	return DeeObject_Shl(self, other);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
object_shr(DeeObject *__restrict self,
           size_t argc, DeeObject **__restrict argv) {
	DeeObject *other;
	if (DeeArg_Unpack(argc, argv, meth_shr, &other))
		goto err;
	return DeeObject_Shr(self, other);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
object_and(DeeObject *__restrict self,
           size_t argc, DeeObject **__restrict argv) {
	DeeObject *other;
	if (DeeArg_Unpack(argc, argv, meth_and, &other))
		goto err;
	return DeeObject_And(self, other);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
object_or(DeeObject *__restrict self,
          size_t argc, DeeObject **__restrict argv) {
	DeeObject *other;
	if (DeeArg_Unpack(argc, argv, meth_or, &other))
		goto err;
	return DeeObject_Or(self, other);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
object_xor(DeeObject *__restrict self,
           size_t argc, DeeObject **__restrict argv) {
	DeeObject *other;
	if (DeeArg_Unpack(argc, argv, meth_xor, &other))
		goto err;
	return DeeObject_Xor(self, other);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
object_pow(DeeObject *__restrict self,
           size_t argc, DeeObject **__restrict argv) {
	DeeObject *other;
	if (DeeArg_Unpack(argc, argv, meth_pow, &other))
		goto err;
	return DeeObject_Pow(self, other);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
object_eq(DeeObject *__restrict self,
          size_t argc, DeeObject **__restrict argv) {
	DeeObject *other;
	if (DeeArg_Unpack(argc, argv, meth_eq, &other))
		goto err;
	return DeeObject_CompareEqObject(self, other);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
object_ne(DeeObject *__restrict self,
          size_t argc, DeeObject **__restrict argv) {
	DeeObject *other;
	if (DeeArg_Unpack(argc, argv, meth_ne, &other))
		goto err;
	return DeeObject_CompareNeObject(self, other);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
object_lo(DeeObject *__restrict self,
          size_t argc, DeeObject **__restrict argv) {
	DeeObject *other;
	if (DeeArg_Unpack(argc, argv, meth_lo, &other))
		goto err;
	return DeeObject_CompareLoObject(self, other);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
object_le(DeeObject *__restrict self,
          size_t argc, DeeObject **__restrict argv) {
	DeeObject *other;
	if (DeeArg_Unpack(argc, argv, meth_le, &other))
		goto err;
	return DeeObject_CompareLeObject(self, other);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
object_gr(DeeObject *__restrict self,
          size_t argc, DeeObject **__restrict argv) {
	DeeObject *other;
	if (DeeArg_Unpack(argc, argv, meth_gr, &other))
		goto err;
	return DeeObject_CompareGrObject(self, other);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
object_ge(DeeObject *__restrict self,
          size_t argc, DeeObject **__restrict argv) {
	DeeObject *other;
	if (DeeArg_Unpack(argc, argv, meth_ge, &other))
		goto err;
	return DeeObject_CompareGeObject(self, other);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
object_size(DeeObject *__restrict self,
            size_t argc, DeeObject **__restrict argv) {
	if (DeeArg_Unpack(argc, argv, meth_size))
		goto err;
	return DeeObject_SizeObject(self);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
object_contains(DeeObject *__restrict self,
                size_t argc, DeeObject **__restrict argv) {
	DeeObject *other;
	if (DeeArg_Unpack(argc, argv, meth_contains, &other))
		goto err;
	return DeeObject_ContainsObject(self, other);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
object_getitem(DeeObject *__restrict self,
               size_t argc, DeeObject **__restrict argv) {
	DeeObject *other;
	if (DeeArg_Unpack(argc, argv, meth_getitem, &other))
		goto err;
	return DeeObject_GetItem(self, other);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
object_delitem(DeeObject *__restrict self,
               size_t argc, DeeObject **__restrict argv) {
	DeeObject *other;
	if (DeeArg_Unpack(argc, argv, meth_delitem, &other))
		goto err;
	if (DeeObject_DelItem(self, other))
		goto err;
	return_none;
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
object_setitem(DeeObject *__restrict self,
               size_t argc, DeeObject **__restrict argv) {
	DeeObject *other, *value;
	if (DeeArg_Unpack(argc, argv, meth_setitem, &other, &value))
		goto err;
	if (DeeObject_SetItem(self, other, value))
		goto err;
	return_reference_(value);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
object_getrange(DeeObject *__restrict self,
                size_t argc, DeeObject **__restrict argv) {
	DeeObject *begin_index, *end_index;
	if (DeeArg_Unpack(argc, argv, meth_getrange, &begin_index, &end_index))
		goto err;
	return DeeObject_GetRange(self, begin_index, end_index);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
object_delrange(DeeObject *__restrict self,
                size_t argc, DeeObject **__restrict argv) {
	DeeObject *begin_index, *end_index;
	if (DeeArg_Unpack(argc, argv, meth_delrange, &begin_index, &end_index))
		goto err;
	if (DeeObject_DelRange(self, begin_index, end_index))
		goto err;
	return_none;
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
object_setrange(DeeObject *__restrict self,
                size_t argc, DeeObject **__restrict argv) {
	DeeObject *begin_index, *end_index, *value;
	if (DeeArg_Unpack(argc, argv, meth_setrange, &begin_index, &end_index, &value))
		goto err;
	if (DeeObject_SetRange(self, begin_index, end_index, value))
		goto err;
	return_reference_(value);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
object_iterself(DeeObject *__restrict self,
                size_t argc, DeeObject **__restrict argv) {
	if (DeeArg_Unpack(argc, argv, meth_iterself))
		goto err;
	return DeeObject_IterSelf(self);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
object_iternext(DeeObject *__restrict self,
                size_t argc, DeeObject **__restrict argv) {
	DREF DeeObject *result;
	if (DeeArg_Unpack(argc, argv, meth_iternext))
		goto err;
	result = DeeObject_IterNext(self);
	if (result == ITER_DONE) {
		DeeError_Throw(&DeeError_StopIteration_instance);
		result = NULL;
	}
	return result;
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
object_getattr(DeeObject *__restrict self,
               size_t argc, DeeObject **__restrict argv) {
	DeeObject *name;
	if (DeeArg_Unpack(argc, argv, meth_getattr, &name))
		goto err;
	if (DeeObject_AssertTypeExact(name, &DeeString_Type))
		goto err;
	return DeeObject_GetAttr(self, name);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
object_callattr(DeeObject *__restrict self,
                size_t argc, DeeObject **__restrict argv) {
	if
		unlikely(!argc)
	goto err_badargc;
	if (DeeObject_AssertTypeExact(argv[0], &DeeString_Type))
		goto err;
	return DeeObject_CallAttr(self, argv[0], argc - 1, argv + 1);
err_badargc:
	err_invalid_argc_va(meth_callattr, argc, 1);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
object_hasattr(DeeObject *__restrict self,
               size_t argc, DeeObject **__restrict argv) {
	DeeObject *name;
	int result;
	if (DeeArg_Unpack(argc, argv, meth_hasattr, &name))
		goto err;
	if (DeeObject_AssertTypeExact(name, &DeeString_Type))
		goto err;
	result = DeeObject_HasAttr(self, name);
	if
		unlikely(result < 0)
	goto err;
	return_bool_(result);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
object_delattr(DeeObject *__restrict self,
               size_t argc, DeeObject **__restrict argv) {
	DeeObject *name;
	if (DeeArg_Unpack(argc, argv, meth_delattr, &name))
		goto err;
	if (DeeObject_AssertTypeExact(name, &DeeString_Type))
		goto err;
	if (DeeObject_DelAttr(self, name))
		goto err;
	return_none;
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
object_setattr(DeeObject *__restrict self,
               size_t argc, DeeObject **__restrict argv) {
	DeeObject *name, *value;
	if (DeeArg_Unpack(argc, argv, meth_setattr, &name, &value))
		goto err;
	if (DeeObject_AssertTypeExact(name, &DeeString_Type))
		goto err;
	if (DeeObject_SetAttr(self, name, value))
		goto err;
	return_reference_(value);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
object_enumattr(DeeObject *__restrict self,
                size_t argc, DeeObject **__restrict argv) {
	if (DeeArg_Unpack(argc, argv, meth_enumattr))
		goto err;
	return DeeObject_New(&DeeEnumAttr_Type, 1, (DeeObject **)&self);
err:
	return NULL;
}

INTDEF dssize_t DCALL
object_format_generic(DeeObject *__restrict self,
                      dformatprinter printer, void *arg,
                      /*utf-8*/ char const *__restrict format_str,
                      size_t format_len);

PRIVATE DREF DeeObject *DCALL
object_format_method(DeeObject *__restrict self,
                     size_t argc, DeeObject **__restrict argv) {
	DeeObject *format_str;
	char *format_utf8;
	if (DeeArg_Unpack(argc, argv, "o:__format__", &format_str))
		goto err;
	if (DeeObject_AssertTypeExact(format_str, &DeeString_Type))
		goto err;
	if
		unlikely((format_utf8 = DeeString_AsUtf8(format_str)) == NULL)
	goto err;
	{
		dssize_t error;
		struct unicode_printer printer = UNICODE_PRINTER_INIT;
		error                          = object_format_generic(self,
                                      &unicode_printer_print,
                                      &printer, format_utf8, WSTR_LENGTH(format_utf8));
		if
			unlikely(error < 0)
		goto err_printer;
		return unicode_printer_pack(&printer);
err_printer:
		unicode_printer_fini(&printer);
	}
err:
	return NULL;
}

#ifndef CONFIG_NO_DEEMON_100_COMPAT
PRIVATE DREF DeeObject *DCALL
object_not(DeeObject *__restrict self,
           size_t argc, DeeObject **__restrict argv) {
	int temp;
	if (DeeArg_Unpack(argc, argv, ":__not__"))
		goto err;
	temp = DeeObject_Bool(self);
	if
		unlikely(temp < 0)
	goto err;
	return_bool_(!temp);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
object_is(DeeObject *__restrict self,
          size_t argc, DeeObject **__restrict argv) {
	bool is_instance;
	DeeTypeObject *tp;
	if (DeeArg_Unpack(argc, argv, "o:__is__", &tp))
		goto err;
	if (DeeNone_Check((DeeObject *)tp)) {
		is_instance = DeeNone_Check(self);
	} else if (DeeSuper_Check(self)) {
		is_instance = DeeType_IsInherited(DeeSuper_TYPE(self), tp);
	} else {
		is_instance = DeeObject_InstanceOf(self, tp);
	}
	return_bool_(is_instance);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
object_inc(DeeObject *__restrict self,
           size_t argc, DeeObject **__restrict argv) {
	DREF DeeObject *selfref;
	int error;
	if (DeeArg_Unpack(argc, argv, ":__inc__"))
		goto err;
	selfref = self;
	Dee_Incref(selfref);
	error = DeeObject_Inc(&selfref);
	if
		unlikely(error)
	goto err_selfref;
	error = DeeObject_Assign(self, selfref);
	if
		unlikely(error)
	goto err_selfref;
	return selfref;
err_selfref:
	Dee_Decref(selfref);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
object_dec(DeeObject *__restrict self,
           size_t argc, DeeObject **__restrict argv) {
	DREF DeeObject *selfref;
	int error;
	if (DeeArg_Unpack(argc, argv, ":__dec__"))
		goto err;
	selfref = self;
	Dee_Incref(selfref);
	error = DeeObject_Dec(&selfref);
	if
		unlikely(error)
	goto err_selfref;
	error = DeeObject_Assign(self, selfref);
	if
		unlikely(error)
	goto err_selfref;
	return selfref;
err_selfref:
	Dee_Decref(selfref);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
object_incpost(DeeObject *__restrict self,
               size_t argc, DeeObject **__restrict argv) {
	DREF DeeObject *selfref, *result;
	int error;
	if (DeeArg_Unpack(argc, argv, ":__incpost__"))
		goto err;
	result = DeeObject_Copy(self);
	if
		unlikely(!result)
	goto err;
	selfref = self;
	Dee_Incref(selfref);
	error = DeeObject_Inc(&selfref);
	if (likely(error == 0) && selfref != self)
		error = DeeObject_Assign(self, selfref);
	Dee_Decref(selfref);
	if
		unlikely(error)
	goto err_r;
	return result;
err_r:
	Dee_Decref(result);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
object_decpost(DeeObject *__restrict self,
               size_t argc, DeeObject **__restrict argv) {
	DREF DeeObject *selfref, *result;
	int error;
	if (DeeArg_Unpack(argc, argv, ":__decpost__"))
		goto err;
	result = DeeObject_Copy(self);
	if
		unlikely(!result)
	goto err;
	selfref = self;
	Dee_Incref(selfref);
	error = DeeObject_Dec(&selfref);
	if (likely(error == 0) && selfref != self)
		error = DeeObject_Assign(self, selfref);
	Dee_Decref(selfref);
	if
		unlikely(error)
	goto err_r;
	return result;
err_r:
	Dee_Decref(result);
err:
	return NULL;
}

#define DEFINE_DEPRECATED_INPLACE_BINARY(name, func)              \
	PRIVATE DREF DeeObject *DCALL                                 \
	object_##name(DeeObject *__restrict self,                     \
	              size_t argc, DeeObject **__restrict argv) {     \
		DREF DeeObject *selfref;                                  \
		int error;                                                \
		DeeObject *other;                                         \
		if (DeeArg_Unpack(argc, argv, "o:__" #name "__", &other)) \
			goto err;                                             \
		selfref = self;                                           \
		Dee_Incref(selfref);                                      \
		error = func(&selfref, other);                            \
		if unlikely(error)                                        \
			goto err_selfref;                                     \
		error = DeeObject_Assign(self, selfref);                  \
		if unlikely(error)                                        \
			goto err_selfref;                                     \
		return selfref;                                           \
	err_selfref:                                                  \
		Dee_Decref(selfref);                                      \
	err:                                                          \
		return NULL;                                              \
	}
DEFINE_DEPRECATED_INPLACE_BINARY(iadd, DeeObject_InplaceAdd)
DEFINE_DEPRECATED_INPLACE_BINARY(isub, DeeObject_InplaceSub)
DEFINE_DEPRECATED_INPLACE_BINARY(imul, DeeObject_InplaceMul)
DEFINE_DEPRECATED_INPLACE_BINARY(idiv, DeeObject_InplaceDiv)
DEFINE_DEPRECATED_INPLACE_BINARY(imod, DeeObject_InplaceMod)
DEFINE_DEPRECATED_INPLACE_BINARY(ishl, DeeObject_InplaceShl)
DEFINE_DEPRECATED_INPLACE_BINARY(ishr, DeeObject_InplaceShr)
DEFINE_DEPRECATED_INPLACE_BINARY(iand, DeeObject_InplaceAnd)
DEFINE_DEPRECATED_INPLACE_BINARY(ior, DeeObject_InplaceOr)
DEFINE_DEPRECATED_INPLACE_BINARY(ixor, DeeObject_InplaceXor)
DEFINE_DEPRECATED_INPLACE_BINARY(ipow, DeeObject_InplacePow)
#undef DEFINE_DEPRECATED_INPLACE_BINARY

#endif /* !CONFIG_NO_DEEMON_100_COMPAT */


PRIVATE struct type_method object_methods[] = {
	/* Utility function: Return the size of a given object (in bytes) */
	{ "__sizeof__", &object_sizeof,
	  DOC("->?Dint\n"
	      "Return the size of @this object in bytes") },

	/* Operator invocation functions. */
	{ meth_copy+1,       &object_copy, DOC("->\n@return A copy of @this object") },
	{ meth_deepcopy+1,   &object_deepcopy, DOC("->\n@return A deep copy of @this object") },
	{ meth_assign+2,     &object_assign, DOC("(other)->@this\nAssigns @other to @this and") },
	{ meth_moveassign+2, &object_moveassign, DOC("(other)->@this\nMove-assign @other to @this and") },
	{ meth_str+1,        &object_dostr, DOC("->?Dstring\n@return: @this converted to a :string") },
	{ meth_repr+1,       &object_dorepr, DOC("->?Dstring\n@return: The :string representation of @this") },
	{ meth_bool+1,       &object_bool, DOC("->?Dbool\n@return: The :bool value of @this") },
	{ meth_call+2,       &object_call, DOC("(args:?DTuple)->\nCall @this using the given @args :tuple") },
	{ meth_thiscall+4,   &object_thiscall, DOC("(this_arg,args:?DTuple)->\nDo a this-call on @this using the given @this_arg and @args :tuple") },
	{ meth_hash+1,       &object_hash, DOC("->?Dint\n@return The hash-value of @this") },
	{ meth_int+1,        &object_int, DOC("->?Dint\n@return The integer-value of @this") },
	{ meth_inv+1,        &object_inv, DOC("->\n@return The result of ${this.operator ~ ()}") },
	{ meth_pos+1,        &object_pos, DOC("->\n@return The result of ${this.operator + ()}") },
	{ meth_neg+1,        &object_neg, DOC("->\n@return The result of ${this.operator - ()}") },
	{ meth_add+2,        &object_add, DOC("(other)->\n@return The result of ${this.operator + (other)}") },
	{ meth_sub+2,        &object_sub, DOC("(other)->\n@return The result of ${this.operator - (other)}") },
	{ meth_mul+2,        &object_mul, DOC("(other)->\n@return The result of ${this.operator * (other)}") },
	{ meth_div+2,        &object_div, DOC("(other)->\n@return The result of ${this.operator / (other)}") },
	{ meth_mod+2,        &object_mod, DOC("(other)->\n@return The result of ${this.operator % (other)}") },
	{ meth_shl+2,        &object_shl, DOC("(other)->\n@return The result of ${this.operator << (other)}") },
	{ meth_shr+2,        &object_shr, DOC("(other)->\n@return The result of ${this.operator >> (other)}") },
	{ meth_and+2,        &object_and, DOC("(other)->\n@return The result of ${this.operator & (other)}") },
	{ meth_or+2,         &object_or, DOC("(other)->\n@return The result of ${this.operator | (other)}") },
	{ meth_xor+2,        &object_xor, DOC("(other)->\n@return The result of ${this.operator ^ (other)}") },
	{ meth_pow+2,        &object_pow, DOC("(other)->\n@return The result of ${this.operator ** (other)}") },
	{ meth_eq+2,         &object_eq, DOC("(other)->\n@return The result of ${this.operator == (other)}") },
	{ meth_ne+2,         &object_ne, DOC("(other)->\n@return The result of ${this.operator != (other)}") },
	{ meth_lo+2,         &object_lo, DOC("(other)->\n@return The result of ${this.operator < (other)}") },
	{ meth_le+2,         &object_le, DOC("(other)->\n@return The result of ${this.operator <= (other)}") },
	{ meth_gr+2,         &object_gr, DOC("(other)->\n@return The result of ${this.operator > (other)}") },
	{ meth_ge+2,         &object_ge, DOC("(other)->\n@return The result of ${this.operator >= (other)}") },
	{ meth_size+1,       &object_size, DOC("->\n@return The result of ${this.operator # ()}") },
	{ meth_contains+2,   &object_contains, DOC("(item)->\n@return The result of ${this.operator contains (item)}") },
	{ meth_getitem+2,    &object_getitem, DOC("(index)->\n@return The result of ${this.operator [] (index)}") },
	{ meth_delitem+2,    &object_delitem, DOC("(index)\nInvokes ${this.operator del[] (index)}") },
	{ meth_setitem+3,    &object_setitem, DOC("(index,value)->\n@return Always re-returned @value\nInvokes ${this.operator []= (index,value)}") },
	{ meth_getrange+3,   &object_getrange, DOC("(start,end)->\n@return The result of ${this.operator [:] (start,end)}") },
	{ meth_delrange+3,   &object_delrange, DOC("(start,end)\nInvokes ${this.operator del[:] (start,end)}") },
	{ meth_setrange+4,   &object_setrange, DOC("(start,end,value)->\n@return Always re-returned @value\nInvokes ${this.operator [:]= (start,end,value)}") },
	{ meth_iterself+1,   &object_iterself, DOC("->\n@return The result of ${this.operator iter()}") },
	{ meth_iternext+1,   &object_iternext, DOC("->\n@return The result of ${this.operator next()}") },
	{ meth_getattr+2,    &object_getattr, DOC("(name:?Dstring)->\n@return The result of ${this.operator . (name)}") },
	{ meth_callattr,     &object_callattr, DOC("(name:?Dstring,args!)->\n@return The result of ${this.operator . (name)(args!)}") },
	{ meth_hasattr+2,    &object_hasattr, DOC("(name:?Dstring)->?Dbool\nCheck if @this object provides an attribute @name, returning :true or :false indicative of this") },
	{ meth_delattr+2,    &object_delattr, DOC("(name:?Dstring)\nInvokes ${this.operator del . (name)}") },
	{ meth_setattr+3,    &object_setattr, DOC("(name:?Dstring,value)\n@return Always re-returned @value\nInvokes ${this.operator .= (name,value)}") },
	{ meth_enumattr+1,   &object_enumattr, DOC("()->?S?DAttribute\n@return Same as ${deemon.enumattr(this)}") },
	{ DeeString_STR(&str___format__), &object_format_method,
	  DOC("(format:?Dstring)->?Dstring\nFormat @this object. (s.a. :string.format)") },
#ifndef CONFIG_NO_DEEMON_100_COMPAT
	/* Aliases for backwards compatibility with deemon < v200 */
	{ "__iterself__",    &object_iterself, DOC("->\nDeprecated alias for #__iter__") },
	{ "__iternext__",    &object_iternext, DOC("->\nDeprecated alias for #__next__") },
	/* Deprecated function for backwards compatibility with deemon < v200 */
	{ "__move__",        &object_copy, DOC("->\nDeprecated alias for #__copy__") },
	{ "__lt__",          &object_lo, DOC("(other)->\nDeprecated alias for #__lo__") },
	{ "__gt__",          &object_gr, DOC("(other)->\nDeprecated alias for #__gr__") },
	{ "__not__",         &object_not, DOC("->?Dbool\nDeprecated alias for ${!this}") },
	{ "__is__",          &object_is, DOC("(tp:?DType)->?Dbool\n(tp:?N)->?Dbool\nDeprecated alias for ${this is tp}") },
	{ "__deepequals__",  &object_eq, DOC("(other)->\nDeprecated alias for #__eq__") },
	{ "__inc__",         &object_inc, DOC("->\nDeprecated alias for ${({ local temp = this; ++temp; if (temp !== this) this := temp; this; })}") },
	{ "__dec__",         &object_dec, DOC("->\nDeprecated alias for ${({ local temp = this; --temp; if (temp !== this) this := temp; this; })}") },
	{ "__incpost__",     &object_incpost, DOC("->\nDeprecated alias for ${({ local res = copy this; local temp = this; ++temp; if (temp !== this) this := temp; res; })}") },
	{ "__decpost__",     &object_decpost, DOC("->\nDeprecated alias for ${({ local res = copy this; local temp = this; --temp; if (temp !== this) this := temp; res; })}") },
	{ "__iadd__",        &object_iadd, DOC("(other)->\nDeprecated alias for ${({ local temp = this; temp += other; if (temp !== this) this := temp; res; })}") },
	{ "__isub__",        &object_isub, DOC("(other)->\nDeprecated alias for ${({ local temp = this; temp -= other; if (temp !== this) this := temp; res; })}") },
	{ "__imul__",        &object_imul, DOC("(other)->\nDeprecated alias for ${({ local temp = this; temp *= other; if (temp !== this) this := temp; res; })}") },
	{ "__idiv__",        &object_idiv, DOC("(other)->\nDeprecated alias for ${({ local temp = this; temp /= other; if (temp !== this) this := temp; res; })}") },
	{ "__imod__",        &object_imod, DOC("(other)->\nDeprecated alias for ${({ local temp = this; temp %= other; if (temp !== this) this := temp; res; })}") },
	{ "__ishl__",        &object_ishl, DOC("(other)->\nDeprecated alias for ${({ local temp = this; temp <<= other; if (temp !== this) this := temp; res; })}") },
	{ "__ishr__",        &object_ishr, DOC("(other)->\nDeprecated alias for ${({ local temp = this; temp >>= other; if (temp !== this) this := temp; res; })}") },
	{ "__iand__",        &object_iand, DOC("(other)->\nDeprecated alias for ${({ local temp = this; temp &= other; if (temp !== this) this := temp; res; })}") },
	{ "__ior__",         &object_ior,  DOC("(other)->\nDeprecated alias for ${({ local temp = this; temp |= other; if (temp !== this) this := temp; res; })}") },
	{ "__ixor__",        &object_ixor, DOC("(other)->\nDeprecated alias for ${({ local temp = this; temp ^= other; if (temp !== this) this := temp; res; })}") },
	{ "__ipow__",        &object_ipow, DOC("(other)->\nDeprecated alias for ${({ local temp = this; temp **= other; if (temp !== this) this := temp; res; })}") },
#endif /* !CONFIG_NO_DEEMON_100_COMPAT */
	{ NULL }
};




PRIVATE DREF DeeObject *DCALL
object_class_get(DeeObject *__restrict self) {
	return_reference((DeeObject *)DeeObject_Class(self));
}

INTERN DREF DeeObject *DCALL
object_id_get(DeeObject *__restrict self) {
	return DeeInt_NewUIntptr(DeeObject_Id(self));
}

PRIVATE DEFINE_CLSPROPERTY(object_id_get_cobj, &DeeObject_Type, &object_id_get, NULL, NULL);
PRIVATE struct type_member object_class_members[] = {
	TYPE_MEMBER_CONST_DOC("id", &object_id_get_cobj, "Alias for #id to speed up expressions such as ${Object.id}"),
	TYPE_MEMBER_END
};


INTDEF DREF DeeObject *DCALL instance_get_itable(DeeObject *__restrict self);

/* Runtime-versions of compiler-intrinsic standard attributes. */
PRIVATE struct type_getset object_getsets[] = {
#ifndef CONFIG_LANGUAGE_NO_ASM
	{ DeeString_STR(&str_this),
	  &DeeObject_NewRef, NULL, NULL,
	  DOC("Always re-return @this object") },
#else
	{ "this",
	  &DeeObject_NewRef, NULL, NULL,
	  DOC("Always re-return @this object") },
#endif
	{ DeeString_STR(&str_class),
	  &object_class_get, NULL, NULL,
	  DOC("->?DType\n"
	      "Returns the class of @this Type, which is usually identical to "
	      "#type, however in the case of a super-proxy, the viewed Type is "
	      "returned, rather than the actual Type") },
	{ "super", &DeeSuper_Of, NULL, NULL,
	  DOC("->?Dsuper\n"
	      "Returns a view for the super-instance of @this object") },
	{ "__itable__", (DREF DeeObject * (DCALL *)(DeeObject * __restrict)) & instance_get_itable, NULL, NULL,
	  DOC("->?AObjectTable?Ert:ClassDescriptor\n"
	      "Returns an indexable sequence describing the instance object "
	      "table, as referenced by :rt.ClassDescriptor.attribute.addr\n"
	      "For non-user-defined classes (aka. when ${this.class.__isclass__} is :false), an empty sequence is returned\n"
	      "The class-attribute table can be accessed through :Type.__ctable__") },
	/* Helper function: `foo.id' returns a unique id for any object. */
	{ "id", &object_id_get, NULL, NULL,
	  DOC("->?Dint\n"
	      "Returns a unique id identifying this specific object instance") },
	{ NULL }
};

PRIVATE struct type_member object_members[] = {
	TYPE_MEMBER_FIELD_DOC("type", STRUCT_CONST | STRUCT_SIZE_T, offsetof(DeeObject, ob_type),
	                      "->?DType\nThe Type of @this object (same as ${type this})"),
	TYPE_MEMBER_FIELD_DOC("__refcnt__", STRUCT_CONST | STRUCT_SIZE_T, offsetof(DeeObject, ob_refcnt),
	                      "->?Dint\nThe number of references currently existing for this object"),
	TYPE_MEMBER_FIELD_DOC("__type__", STRUCT_CONST | STRUCT_SIZE_T, offsetof(DeeObject, ob_type),
	                      "->?DType\nAlias for #type"),
	TYPE_MEMBER_END
};


PUBLIC DeeTypeObject DeeObject_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ DeeString_STR(&str_Object),
	/* .tp_doc      = */ DOC("The base class of all regular objects\n"
	                        "\n"
	                        "()\n"
	                        "Construct a new object (no-op constructor)\n"
	                        "\n"
	                        "str->\n"
	                        "Returns the name of the object's Type\n"
	                        ">operator str(): string {\n"
	                        "> return str type this;\n"
	                        ">}\n"
	                        ),
	/* .tp_flags    = */ TP_FNORMAL | TP_FNAMEOBJECT | TP_FABSTRACT,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ NULL,    /* No base */
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ &object_ctor,
				/* .tp_copy_ctor = */ &object_copy_ctor,
				/* .tp_deep_ctor = */ &object_copy_ctor,
				/* .tp_any_ctor  = */ &object_any_ctor,
				TYPE_FIXED_ALLOCATOR_S(DeeObject)
			}
		},
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ &object_str,
		/* .tp_repr = */ &object_repr,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ object_methods,
	/* .tp_getsets       = */ object_getsets,
	/* .tp_members       = */ object_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ object_class_members
};


INTERN int DCALL
type_ctor(DeeTypeObject *__restrict self) {
	/* Simply re-initialize everything to ZERO and set the HEAP flag. */
	memset(&self->tp_name, 0,
	       sizeof(DeeTypeObject) -
	       offsetof(DeeTypeObject, tp_name));
	self->tp_flags |= TP_FHEAP;
	return 0;
}

PRIVATE DREF DeeObject *DCALL type_str(DeeTypeObject *__restrict self) {
	if (self->tp_flags & TP_FNAMEOBJECT) {
		DREF DeeStringObject *result;
		result = COMPILER_CONTAINER_OF(self->tp_name,
		                               DeeStringObject,
		                               s_str);
		Dee_Incref(result);
		return (DREF DeeObject *)result;
	}
	if (self->tp_name)
		return DeeString_New(self->tp_name);
	return_reference_(&str_Type);
}


INTDEF void DCALL class_fini(DeeTypeObject *__restrict self);
INTDEF void DCALL class_visit(DeeTypeObject *__restrict self, dvisit_t proc, void *arg);
INTDEF void DCALL class_clear(DeeTypeObject *__restrict self);
INTDEF void DCALL class_pclear(DeeTypeObject *__restrict self, unsigned int gc_priority);

PRIVATE void DCALL type_fini(DeeTypeObject *__restrict self) {
	/* Clear weak references and check for revival. */
	weakref_support_fini(self);
	if (Dee_IncrefIfNotZero(self))
		return;
	ASSERTF(self->tp_flags & TP_FHEAP,
	        "Non heap-allocated type %s is being destroyed (This shouldn't happen)",
	        self->tp_name);
	if (DeeType_IsClass(self))
		class_fini(self);
	/* Finalize the type's member caches. */
	membercache_fini(&self->tp_cache);
	membercache_fini(&self->tp_class_cache);
	/* Cleanup name & doc objects should those have been used. */
	if (self->tp_flags & TP_FNAMEOBJECT)
		Dee_XDecref(COMPILER_CONTAINER_OF(self->tp_name, DeeStringObject, s_str));
	if (self->tp_flags & TP_FDOCOBJECT)
		Dee_XDecref(COMPILER_CONTAINER_OF(self->tp_doc, DeeStringObject, s_str));
	Dee_XDecref(self->tp_base);
}


PRIVATE void DCALL
type_visit(DeeTypeObject *__restrict self,
           dvisit_t proc, void *arg) {
	if (DeeType_IsClass(self))
		class_visit(self, proc, arg);
	Dee_XVisit(self->tp_base);
}

PRIVATE void DCALL
type_clear(DeeTypeObject *__restrict self) {
	if (DeeType_IsClass(self))
		class_clear(self);
}

PRIVATE void DCALL
type_pclear(DeeTypeObject *__restrict self, unsigned int gc_priority) {
	if (DeeType_IsClass(self))
		class_pclear(self, gc_priority);
}

PRIVATE DREF DeeObject *DCALL
type_baseof(DeeTypeObject *__restrict self, size_t argc,
            DeeObject **__restrict argv, DeeObject *kw) {
	DeeTypeObject *other;
	PRIVATE struct keyword kwlist[] = { K(other), KEND };
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist, "o:baseof", &other))
		return NULL;
	if (!DeeType_Check((DeeObject *)other))
		return_false;
	return_bool(DeeType_IsInherited(other, self));
}

PRIVATE DREF DeeObject *DCALL
type_derivedfrom(DeeTypeObject *__restrict self, size_t argc,
                 DeeObject **__restrict argv, DeeObject *kw) {
	DeeTypeObject *other;
	PRIVATE struct keyword kwlist[] = { K(other), KEND };
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist, "o:derivedfrom", &other))
		return NULL;
	return_bool(DeeType_IsInherited(self, other));
}


PRIVATE ATTR_COLD int DCALL
err_init_var_type(DeeTypeObject *__restrict self) {
	return DeeError_Throwf(&DeeError_TypeError,
	                       "Cannot instantiate variable-length type %k",
	                       self);
}

PRIVATE ATTR_COLD int DCALL
err_missing_mandatory_init(DeeTypeObject *__restrict self) {
	return DeeError_Throwf(&DeeError_TypeError,
	                       "Missing initializer for mandatory base-type %k",
	                       self);
}

PRIVATE char const str_shared_ctor_failed[] = "Constructor of shared object failed\n";
INTDEF void DCALL
instance_clear_members(struct instance_desc *__restrict self, uint16_t size);

PRIVATE DREF DeeObject *DCALL
type_new_raw(DeeTypeObject *__restrict self) {
	DREF DeeObject *result;
	DeeTypeObject *first_base;
	if
		unlikely(self->tp_flags & TP_FVARIABLE)
	{
		err_init_var_type(self);
		goto err;
	}
	if (self->tp_init.tp_alloc.tp_free) {
		result = (DREF DeeObject *)(*self->tp_init.tp_alloc.tp_alloc)();
	} else if (self->tp_flags & TP_FGC) {
		result = (DREF DeeObject *)DeeGCObject_Malloc(self->tp_init.tp_alloc.tp_instance_size);
	} else {
		result = (DREF DeeObject *)DeeObject_Malloc(self->tp_init.tp_alloc.tp_instance_size);
	}
	if
		unlikely(!result)
	goto err;
	DeeObject_Init(result, self);
	/* Search for the first non-class base. */
	first_base = self;
	while (DeeType_IsClass(first_base)) {
		struct class_desc *desc        = DeeClass_DESC(first_base);
		struct instance_desc *instance = DeeInstance_DESC(desc, result);
		rwlock_init(&instance->id_lock);
		MEMSET_PTR(instance->id_vtab, 0, desc->cd_desc->cd_imemb_size);
		first_base = DeeType_Base(first_base);
		if (!first_base)
			break;
	}
	/* Instantiate non-base types. */
	if (!first_base || first_base == &DeeObject_Type)
		goto done;
	if (first_base->tp_init.tp_alloc.tp_ctor) {
		/* Invoke the mandatory base-type constructor. */
invoke_base_ctor:
		if
			unlikely((*first_base->tp_init.tp_alloc.tp_ctor)(result))
		goto err_r;
		goto done;
	}
	if (first_base->tp_init.tp_alloc.tp_any_ctor) {
		/* Invoke the mandatory base-type constructor. */
invoke_base_any_ctor:
		if
			unlikely((*first_base->tp_init.tp_alloc.tp_any_ctor)(result, 0, NULL))
		goto err_r;
		goto done;
	}
	if (first_base->tp_init.tp_alloc.tp_any_ctor_kw) {
		/* Invoke the mandatory base-type constructor. */
invoke_base_any_ctor_kw:
		if
			unlikely((*first_base->tp_init.tp_alloc.tp_any_ctor_kw)(result, 0, NULL, NULL))
		goto err_r;
		goto done;
	}
	if (type_inherit_constructors(first_base)) {
		if (first_base->tp_init.tp_alloc.tp_ctor)
			goto invoke_base_ctor;
		if (first_base->tp_init.tp_alloc.tp_any_ctor)
			goto invoke_base_any_ctor;
		if (first_base->tp_init.tp_alloc.tp_any_ctor_kw)
			goto invoke_base_any_ctor_kw;
	}
	err_missing_mandatory_init(first_base);
	goto err_r;
done:
	if (self->tp_flags & TP_FGC)
		DeeGC_Track(result);
	return result;
err_r:
	if (!DeeObject_UndoConstruction(first_base, result)) {
		DeeError_Print(str_shared_ctor_failed, ERROR_PRINT_DOHANDLE);
		return result;
	}
	first_base = self;
	while (DeeType_IsClass(first_base)) {
		struct class_desc *desc        = DeeClass_DESC(first_base);
		struct instance_desc *instance = DeeInstance_DESC(desc, result);
		instance_clear_members(instance, desc->cd_desc->cd_imemb_size);
		first_base = DeeType_Base(first_base);
		if (!first_base)
			break;
	}
	Dee_DecrefNokill(self);
	if (self->tp_init.tp_alloc.tp_free) {
		(*self->tp_init.tp_alloc.tp_free)(result);
	} else if (self->tp_flags & TP_FGC) {
		DeeGCObject_Free(result);
	} else {
		DeeObject_Free(result);
	}
err:
	return NULL;
}

PRIVATE int DCALL
set_basic_member(DeeTypeObject *__restrict tp_self,
                 DeeObject *__restrict self,
                 DeeStringObject *__restrict member_name,
                 DeeObject *__restrict value) {
	int temp;
	DeeTypeObject *iter   = tp_self;
	char const *attr_name = DeeString_STR(member_name);
	dhash_t attr_hash     = DeeString_Hash((DeeObject *)member_name);
	if ((temp = DeeType_SetBasicCachedAttr(tp_self, self, attr_name, attr_hash, value)) <= 0)
		goto done_temp;
	do {
		if (DeeType_IsClass(iter)) {
			struct class_attribute *attr;
			struct instance_desc *instance;
			struct class_desc *desc;
			DREF DeeObject *old_value;
			attr = DeeType_QueryAttributeWithHash(tp_self, iter,
			                                      (DeeObject *)member_name,
			                                      attr_hash);
			if (!attr)
				goto next_base;
			if (attr->ca_flag & (CLASS_ATTRIBUTE_FCLASSMEM |
			                     CLASS_ATTRIBUTE_FGETSET))
				goto next_base;
			desc     = DeeClass_DESC(iter);
			instance = DeeInstance_DESC(desc, self);
			Dee_Incref(value);
			rwlock_write(&instance->id_lock);
			old_value                        = instance->id_vtab[attr->ca_addr];
			instance->id_vtab[attr->ca_addr] = value;
			rwlock_endwrite(&instance->id_lock);
			if
				unlikely(old_value)
			Dee_Decref(old_value);
			return 0;
		}
		if (iter->tp_members &&
		    (temp = DeeType_SetMemberAttr(tp_self, iter, self, attr_name, attr_hash, value)) <= 0)
			goto done_temp;
	next_base:;
	} while ((iter = DeeType_Base(iter)) != NULL);
	return DeeError_Throwf(&DeeError_AttributeError,
	                       "Could not find member %k in %k, or its bases",
	                       member_name, tp_self);
done_temp:
	return temp;
}

PRIVATE int DCALL
set_private_basic_member(DeeTypeObject *__restrict tp_self,
                         DeeObject *__restrict self,
                         DeeStringObject *__restrict member_name,
                         DeeObject *__restrict value) {
	int temp;
	char const *attr_name = DeeString_STR(member_name);
	dhash_t attr_hash     = DeeString_Hash((DeeObject *)member_name);
	if (DeeType_IsClass(tp_self)) {
		struct class_attribute *attr;
		struct instance_desc *instance;
		struct class_desc *desc = DeeClass_DESC(tp_self);
		DREF DeeObject *old_value;
		attr = DeeClassDesc_QueryInstanceAttributeStringWithHash(desc,
		                                                         attr_name,
		                                                         attr_hash);
		if (!attr)
			goto not_found;
		if (attr->ca_flag & (CLASS_ATTRIBUTE_FCLASSMEM |
		                     CLASS_ATTRIBUTE_FGETSET))
			goto not_found;
		instance = DeeInstance_DESC(desc, self);
		Dee_Incref(value);
		rwlock_write(&instance->id_lock);
		old_value                        = instance->id_vtab[attr->ca_addr];
		instance->id_vtab[attr->ca_addr] = value;
		rwlock_endwrite(&instance->id_lock);
		if
			unlikely(old_value)
		Dee_Decref(old_value);
		return 0;
	}
	if (tp_self->tp_members &&
	    (temp = DeeType_SetMemberAttr(tp_self, tp_self, self, attr_name, attr_hash, value)) <= 0)
		goto done_temp;
not_found:
	return DeeError_Throwf(&DeeError_AttributeError,
	                       "Could not find member %k in %k",
	                       member_name, tp_self);
done_temp:
	return temp;
}

PRIVATE int DCALL
unpack_init_info(DeeObject *__restrict info,
                 DREF DeeObject **__restrict pinit_fields,
                 DREF DeeObject **__restrict pinit_args,
                 DREF DeeObject **__restrict pinit_kw) {
	DREF DeeObject *iterator;
	DREF DeeObject *sentinal;
	if
		likely(DeeTuple_Check(info))
	{
		switch (DeeTuple_SIZE(info)) {
		case 1:
			*pinit_fields = DeeTuple_GET(info, 0);
			if (DeeNone_Check(*pinit_fields))
				*pinit_fields = NULL;
			*pinit_args = Dee_EmptyTuple;
			*pinit_kw   = NULL;
			break;
		case 2:
			*pinit_fields = DeeTuple_GET(info, 0);
			*pinit_args   = DeeTuple_GET(info, 1);
			if (DeeNone_Check(*pinit_fields))
				*pinit_fields = NULL;
			if (DeeNone_Check(*pinit_args))
				*pinit_args = Dee_EmptyTuple;
			*pinit_kw = NULL;
			break;
		case 3:
			*pinit_fields = DeeTuple_GET(info, 0);
			*pinit_args   = DeeTuple_GET(info, 1);
			*pinit_kw     = DeeTuple_GET(info, 2);
			if (DeeNone_Check(*pinit_fields))
				*pinit_fields = NULL;
			if (DeeNone_Check(*pinit_args))
				*pinit_args = Dee_EmptyTuple;
			if (DeeNone_Check(*pinit_kw))
				*pinit_kw = NULL;
			break;
		default:
			return err_invalid_unpack_size_minmax(info, 1, 3, DeeTuple_SIZE(info));
		}
		if (DeeObject_AssertTypeExact(*pinit_args, &DeeTuple_Type))
			goto err;
		Dee_XIncref(*pinit_fields);
		Dee_Incref(*pinit_args);
		Dee_XIncref(*pinit_kw);
	} else {
		size_t fast_size;
		/* Use the fast-sequence iterface. */
		fast_size = DeeFastSeq_GetSize(info);
		if (fast_size != DEE_FASTSEQ_NOTFAST) {
			if (fast_size == 1) {
				*pinit_fields = DeeFastSeq_GetItem(info, 0);
				if
					unlikely(!*pinit_fields)
				goto err;
				*pinit_args = Dee_EmptyTuple;
				Dee_Incref(Dee_EmptyTuple);
				goto done_iterator_data;
			}
			if (fast_size == 2) {
				*pinit_fields = DeeFastSeq_GetItem(info, 0);
				if
					unlikely(!*pinit_fields)
				goto err;
				*pinit_args = DeeFastSeq_GetItem(info, 1);
				if
					unlikely(!*pinit_args)
				goto err_fields;
				goto done_iterator_data;
			}
			if (fast_size == 3) {
				*pinit_fields = DeeFastSeq_GetItem(info, 0);
				if
					unlikely(!*pinit_fields)
				goto err;
				*pinit_args = DeeFastSeq_GetItem(info, 1);
				if
					unlikely(!*pinit_args)
				goto err_fields;
				*pinit_kw = DeeFastSeq_GetItem(info, 2);
				if
					unlikely(!*pinit_kw)
				goto err_args;
				goto done_iterator_data;
			}
			return err_invalid_unpack_size_minmax(info, 1, 3, fast_size);
		}
		/* Fallback: use iteartors. */
		iterator = DeeObject_IterSelf(info);
		if
			unlikely(!iterator)
		goto err;
		*pinit_fields = DeeObject_IterNext(iterator);
		if
			unlikely(!ITER_ISOK(*pinit_fields))
		{
			if (*pinit_fields)
				err_invalid_unpack_size_minmax(info, 1, 3, 0);
			Dee_Decref(iterator);
			goto err;
		}
		*pinit_args = DeeObject_IterNext(iterator);
		if (*pinit_args == ITER_DONE) {
			*pinit_args = Dee_EmptyTuple;
			*pinit_kw   = NULL;
			Dee_Incref(Dee_EmptyTuple);
			goto done_iterator;
		}
		if
			unlikely(!*pinit_args)
		{
			Dee_Decref(iterator);
			goto err_fields;
		}
		*pinit_kw = DeeObject_IterNext(iterator);
		if (*pinit_kw == ITER_DONE)
			*pinit_kw = NULL;
		else if (!*pinit_kw) {
			Dee_Decref(iterator);
			goto err_args;
		}
		sentinal = DeeObject_IterNext(iterator);
		if
			unlikely(sentinal != ITER_DONE)
		{
			if (sentinal) {
				Dee_Decref(sentinal);
				err_invalid_unpack_iter_size_minmax(info, iterator, 1, 3);
			}
			Dee_XDecref(*pinit_kw);
			Dee_Decref(iterator);
			goto err_args;
		}
done_iterator:
		Dee_Decref(iterator);
done_iterator_data:
		if (DeeNone_Check(*pinit_fields))
			Dee_Clear(*pinit_fields);
		if (DeeNone_Check(*pinit_args)) {
			Dee_Decref(Dee_None);
			*pinit_args = Dee_EmptyTuple;
			Dee_Incref(Dee_EmptyTuple);
		} else {
			if (DeeObject_AssertTypeExact(*pinit_args, &DeeTuple_Type))
				goto err_kw;
		}
		if (*pinit_kw && DeeNone_Check(*pinit_kw))
			Dee_Clear(*pinit_kw);
	}
	return 0;
err_kw:
	Dee_XDecref(*pinit_kw);
err_args:
	Dee_Decref(*pinit_args);
err_fields:
	Dee_XDecref(*pinit_fields);
err:
	return -1;
}

PRIVATE DREF DeeObject *DCALL
unpack_init_info1(DeeObject *__restrict info) {
	DREF DeeObject *init_fields;
	DREF DeeObject *init_argv;
	DREF DeeObject *init_kw;
	if
		unlikely(unpack_init_info(info, &init_fields, &init_argv, &init_kw))
	return NULL;
	Dee_XDecref(init_kw);
	Dee_Decref(init_argv);
	if (!init_fields)
		init_fields = ITER_DONE;
	return init_fields;
}

INTDEF int DCALL
type_invoke_base_constructor(DeeTypeObject *__restrict tp_self,
                             DeeObject *__restrict self, size_t argc,
                             DeeObject **__restrict argv, DeeObject *kw);

PRIVATE int DCALL
assign_init_fields(DeeTypeObject *__restrict tp_self,
                   DeeObject *__restrict self,
                   DeeObject *__restrict fields) {
	DREF DeeObject *iterator, *elem;
	int temp;
	DREF DeeObject *key_and_value[2];
	iterator = DeeObject_IterSelf(fields);
	if
		unlikely(!iterator)
	goto err;
	while (ITER_ISOK(elem = DeeObject_IterNext(iterator))) {
		temp = DeeObject_Unpack(elem, 2, key_and_value);
		Dee_Decref(elem);
		if
			unlikely(temp)
		goto err_iterator;
		temp = DeeObject_AssertTypeExact(key_and_value[0], &DeeString_Type);
		if
			likely(!temp)
		temp = set_basic_member(tp_self, self, (DeeStringObject *)key_and_value[0], key_and_value[1]);
		Dee_Decref(key_and_value[1]);
		Dee_Decref(key_and_value[0]);
		if
			unlikely(temp)
		goto err_iterator;
	}
	if
		unlikely(!elem)
	goto err_iterator;
	Dee_Decref(iterator);
	return 0;
err_iterator:
	Dee_Decref(iterator);
err:
	return -1;
}

PRIVATE DREF DeeObject *DCALL
type_new_extended(DeeTypeObject *__restrict self,
                  DeeObject *__restrict initializer) {
	DREF DeeObject *result, *init_info;
	int temp;
	DREF DeeObject *init_fields, *init_args, *init_kw;
	DeeTypeObject *first_base, *iter;
	if
		unlikely(self->tp_flags & TP_FVARIABLE)
	{
		err_init_var_type(self);
		goto err;
	}
	if (self->tp_init.tp_alloc.tp_free) {
		result = (DREF DeeObject *)(*self->tp_init.tp_alloc.tp_alloc)();
	} else if (self->tp_flags & TP_FGC) {
		result = (DREF DeeObject *)DeeGCObject_Malloc(self->tp_init.tp_alloc.tp_instance_size);
	} else {
		result = (DREF DeeObject *)DeeObject_Malloc(self->tp_init.tp_alloc.tp_instance_size);
	}
	if
		unlikely(!result)
	goto err;
	DeeObject_Init(result, self);
	/* Search for the first non-class base. */
	first_base = self;
	while (DeeType_IsClass(first_base)) {
		struct class_desc *desc        = DeeClass_DESC(first_base);
		struct instance_desc *instance = DeeInstance_DESC(desc, result);
		rwlock_init(&instance->id_lock);
		MEMSET_PTR(instance->id_vtab, 0, desc->cd_desc->cd_imemb_size);
		first_base = DeeType_Base(first_base);
		if (!first_base)
			break;
	}
	/* Instantiate non-base types. */
	if (!first_base || first_base == &DeeObject_Type)
		goto done_fields;
	/* {(Type,({(string,object)...},tuple))...} */
	/* {(Type,({(string,object)...},tuple,mapping))...} */
	init_info = DeeObject_GetItemDef(initializer, (DeeObject *)first_base, Dee_None);
	if
		unlikely(!init_info)
	goto err_r;
	temp = unpack_init_info(init_info, &init_fields, &init_args, &init_kw);
	Dee_Decref(init_info);
	if
		unlikely(temp)
	goto err_r;
	/* Invoke the mandatory base-type constructor. */
	temp = type_invoke_base_constructor(first_base, result,
	                                    DeeTuple_SIZE(init_args),
	                                    DeeTuple_ELEM(init_args),
	                                    init_kw);
	Dee_XDecref(init_kw);
	Dee_Decref(init_args);
	if
		likely(!temp && init_fields)
	temp = assign_init_fields(first_base, result, init_fields);
	Dee_XDecref(init_fields);
	if
		unlikely(temp)
	goto err_r_firstbase;
done_fields:
	/* Fill in all of the fields of non-first-base types. */
	iter = self;
	do {
		if (iter == first_base)
			continue;
		init_info = DeeObject_GetItemDef(initializer,
		                                 (DeeObject *)iter,
		                                 Dee_None);
		if
			unlikely(!init_info)
		goto err_r_firstbase;
		if (DeeNone_Check(init_info)) {
			Dee_DecrefNokill(init_info);
			continue;
		}
		init_fields = unpack_init_info1(init_info);
		Dee_Decref(init_info);
		if (init_fields == ITER_DONE)
			continue;
		if
			unlikely(!init_fields)
		goto err_r_firstbase;
		temp = assign_init_fields(iter, result, init_fields);
		Dee_Decref(init_fields);
		if
			unlikely(temp)
		goto err_r_firstbase;
	} while ((iter = DeeType_Base(iter)) != NULL);

	if (self->tp_flags & TP_FGC)
		DeeGC_Track(result);
	return result;
err_r_firstbase:
	if (!DeeObject_UndoConstruction(first_base, result)) {
		DeeError_Print(str_shared_ctor_failed, ERROR_PRINT_DOHANDLE);
		return result;
	}
err_r:
	first_base = self;
	while (DeeType_IsClass(first_base)) {
		struct class_desc *desc        = DeeClass_DESC(first_base);
		struct instance_desc *instance = DeeInstance_DESC(desc, result);
		instance_clear_members(instance, desc->cd_desc->cd_imemb_size);
		first_base = DeeType_Base(first_base);
		if (!first_base)
			break;
	}
	Dee_DecrefNokill(self);
	if (self->tp_init.tp_alloc.tp_free) {
		(*self->tp_init.tp_alloc.tp_free)(result);
	} else if (self->tp_flags & TP_FGC) {
		DeeGCObject_Free(result);
	} else {
		DeeObject_Free(result);
	}
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
type_newinstance(DeeTypeObject *__restrict self, size_t argc,
                 DeeObject **__restrict argv, DeeObject *kw) {
	DREF DeeObject *result;
	DREF DeeObject *iterator, *elem;
	if (self == &DeeNone_Type)
		return_none; /* Allow `none' to be instantiated with whatever you throw at it! */
	if (kw && (!DeeKwds_Check(kw) || DeeKwds_SIZE(kw) == argc)) {
		/* Instantiate using keyword arguments. */
		result = type_new_raw(self);
		/* Fill in values for provided fields. */
		if (DeeKwds_Check(kw)) {
			size_t i;
			DeeKwdsObject *kwds = (DeeKwdsObject *)kw;
			for (i = 0; i <= kwds->kw_mask; ++i) {
				if (!kwds->kw_map[i].ke_name)
					continue;
				ASSERT(kwds->kw_map[i].ke_index <= argc);
				if
					unlikely(set_private_basic_member(self, result,
					                                  kwds->kw_map[i].ke_name,
					                                  argv[kwds->kw_map[i].ke_index]))
				goto err_r;
			}
		} else {
			iterator = DeeObject_IterSelf(kw);
			if
				unlikely(!iterator)
			goto err_r;
			while (ITER_ISOK(elem = DeeObject_IterNext(iterator))) {
				DREF DeeObject *name_and_value[2];
				int temp;
				temp = DeeObject_Unpack(elem, 2, name_and_value);
				Dee_Decref(elem);
				if
					unlikely(temp)
				goto err_r_iterator;
				temp = DeeObject_AssertTypeExact(name_and_value[0], &DeeString_Type);
				if
					likely(!temp)
				{
					temp = set_private_basic_member(self, result,
					                                (DeeStringObject *)name_and_value[0],
					                                name_and_value[1]);
				}
				Dee_Decref(name_and_value[1]);
				Dee_Decref(name_and_value[0]);
				if
					unlikely(temp)
				goto err_r_iterator;
			}
			if
				unlikely(!elem)
			goto err_r_iterator;
			Dee_Decref(iterator);
		}
		return result;
	}
	/* Without any arguments, simply construct an
	 * empty instance (with all members unbound) */
	if (!argc)
		return type_new_raw(self);
	if (argc != 1) {
		err_invalid_argc("newinstance", argc, 0, 1);
		goto err;
	}
	/* Extended constructors! */
	return type_new_extended(self, argv[0]);
err_r_iterator:
	Dee_Decref_likely(iterator);
err_r:
	Dee_Decref_likely(result);
err:
	return NULL;
}

PRIVATE char const meth_getinstanceattr[]   = "o:getinstanceattr";
PRIVATE char const meth_callinstanceattr[]  = "callinstanceattr";
PRIVATE char const meth_hasinstanceattr[]   = "o:hasinstanceattr";
PRIVATE char const meth_boundinstanceattr[] = "o|b:boundinstanceattr";
PRIVATE char const meth_delinstanceattr[]   = "o:delinstanceattr";
PRIVATE char const meth_setinstanceattr[]   = "oo:setinstanceattr";

PRIVATE struct keyword getattr_kwdlist[] = { K(name), KEND };

PRIVATE struct keyword setattr_kwdlist[] = { K(name), K(value), KEND };

PRIVATE struct keyword boundattr_kwdlist[] = { K(name), K(allow_missing), KEND };

PRIVATE DREF DeeObject *DCALL
type_getinstanceattr(DeeTypeObject *__restrict self,
                     size_t argc, DeeObject **__restrict argv,
                     DeeObject *kw) {
	DeeObject *name;
	if (DeeArg_UnpackKw(argc, argv, kw, getattr_kwdlist, meth_getinstanceattr, &name) ||
	    DeeObject_AssertTypeExact(name, &DeeString_Type))
		return NULL;
	return DeeType_GetInstanceAttrString(self,
	                                     DeeString_STR(name),
	                                     DeeString_Hash(name));
}

PRIVATE DREF DeeObject *DCALL
type_callinstanceattr(DeeTypeObject *__restrict self,
                      size_t argc, DeeObject **__restrict argv,
                      DeeObject *kw) {
	if (!argc) {
		err_invalid_argc_va(meth_callinstanceattr, argc, 1);
		return NULL;
	}
	if (DeeObject_AssertTypeExact(argv[0], &DeeString_Type))
		return NULL;
	return DeeType_CallInstanceAttrStringKw(self,
	                                        DeeString_STR(argv[0]),
	                                        DeeString_Hash(argv[0]),
	                                        argc - 1,
	                                        argv + 1,
	                                        kw);
}

PRIVATE DREF DeeObject *DCALL
type_hasinstanceattr(DeeTypeObject *__restrict self,
                     size_t argc, DeeObject **__restrict argv,
                     DeeObject *kw) {
	DeeObject *name;
	int result;
	if (DeeArg_UnpackKw(argc, argv, kw, getattr_kwdlist, meth_hasinstanceattr, &name) ||
	    DeeObject_AssertTypeExact(name, &DeeString_Type))
		return NULL;
	result = DeeType_HasInstanceAttrString(self,
	                                       DeeString_STR(name),
	                                       DeeString_Hash(name));
	if
		unlikely(result < 0)
	return NULL;
	return_bool_(result);
}

PRIVATE DREF DeeObject *DCALL
type_boundinstanceattr(DeeTypeObject *__restrict self,
                       size_t argc, DeeObject **__restrict argv,
                       DeeObject *kw) {
	DeeObject *name;
	bool allow_missing = true;
	int result;
	if (DeeArg_UnpackKw(argc, argv, kw, boundattr_kwdlist, meth_boundinstanceattr, &name, &allow_missing) ||
	    DeeObject_AssertTypeExact(name, &DeeString_Type))
		goto err;
	/* Instance attributes of types are always bound (because they're all wrappers) */
	result = DeeType_BoundInstanceAttrString(self, DeeString_STR(name), DeeString_Hash(name));
	if (result > 0)
		return_true;
	if (result == -1)
		goto err;
	if (allow_missing)
		return_false; /* Unknown attributes are unbound. */
	err_unknown_attribute(self, DeeString_STR(name), ATTR_ACCESS_GET);
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
type_delinstanceattr(DeeTypeObject *__restrict self,
                     size_t argc, DeeObject **__restrict argv,
                     DeeObject *kw) {
	DeeObject *name;
	if (DeeArg_UnpackKw(argc, argv, kw, getattr_kwdlist, meth_delinstanceattr, &name) ||
	    DeeObject_AssertTypeExact(name, &DeeString_Type) ||
	    DeeType_DelInstanceAttrString(self,
	                                  DeeString_STR(name),
	                                  DeeString_Hash(name)))
		return NULL;
	return_none;
}

PRIVATE DREF DeeObject *DCALL
type_setinstanceattr(DeeTypeObject *__restrict self,
                     size_t argc, DeeObject **__restrict argv,
                     DeeObject *kw) {
	DeeObject *name, *value;
	if (DeeArg_UnpackKw(argc, argv, kw, setattr_kwdlist, meth_setattr, &name, &value) ||
	    DeeObject_AssertTypeExact(name, &DeeString_Type) ||
	    DeeType_SetInstanceAttrString(self,
	                                  DeeString_STR(name),
	                                  DeeString_Hash(name),
	                                  value))
		return NULL;
	return_reference_(value);
}


PRIVATE bool DCALL
impl_type_hasprivateattribute(DeeTypeObject *__restrict self,
                              char const *name_str,
                              dhash_t name_hash) {
	/* TODO: Lookup the attribute in the member cache, and
	 *       see which type is set as the declaring type! */
	if (DeeType_IsClass(self)) {
		struct class_desc *desc = DeeClass_DESC(self);
		if (DeeClassDesc_QueryInstanceAttributeStringWithHash(desc, name_str, name_hash) != NULL)
			goto found;
	} else {
		if (self->tp_methods &&
		    DeeType_HasMethodAttr(self, self, name_str, name_hash))
			goto found;
		if (self->tp_getsets &&
		    DeeType_HasGetSetAttr(self, self, name_str, name_hash))
			goto found;
		if (self->tp_members &&
		    DeeType_HasMemberAttr(self, self, name_str, name_hash))
			goto found;
	}
	return 0;
found:
	return 1;
}


PRIVATE DREF DeeObject *DCALL
type_hasattribute(DeeTypeObject *__restrict self,
                  size_t argc, DeeObject **__restrict argv,
                  DeeObject *kw) {
	DeeObject *name;
	char const *name_str;
	dhash_t name_hash;
	if (DeeArg_UnpackKw(argc, argv, kw, getattr_kwdlist, "o:hasattribute", &name) ||
	    DeeObject_AssertTypeExact(name, &DeeString_Type))
		goto err;
	name_str  = DeeString_STR(name);
	name_hash = DeeString_Hash(name);
	if (!self->tp_attr) {
		DeeTypeObject *iter;
		if (DeeType_HasCachedAttr(self, name_str, name_hash))
			goto found;
		iter = self;
		for (;;) {
			if (DeeType_IsClass(iter)) {
				if (DeeType_QueryInstanceAttributeWithHash(self, iter, name, name_hash) != NULL)
					goto found;
			} else {
				if (iter->tp_methods &&
				    DeeType_HasMethodAttr(self, iter, name_str, name_hash))
					goto found;
				if (iter->tp_getsets &&
				    DeeType_HasGetSetAttr(self, iter, name_str, name_hash))
					goto found;
				if (iter->tp_members &&
				    DeeType_HasMemberAttr(self, iter, name_str, name_hash))
					goto found;
			}
			iter = DeeType_Base(iter);
			if (!iter)
				break;
			if (iter->tp_attr)
				break;
		}
	}
	return_false;
found:
	return_true;
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
type_hasprivateattribute(DeeTypeObject *__restrict self,
                         size_t argc, DeeObject **__restrict argv,
                         DeeObject *kw) {
	DeeObject *name;
	if (DeeArg_UnpackKw(argc, argv, kw, getattr_kwdlist, "o:hasprivateattribute", &name) ||
	    DeeObject_AssertTypeExact(name, &DeeString_Type))
		goto err;
	return_bool(!self->tp_attr &&
	            impl_type_hasprivateattribute(self,
	                                          DeeString_STR(name),
	                                          DeeString_Hash(name)));
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
type_hasoperator(DeeTypeObject *__restrict self,
                 size_t argc, DeeObject **__restrict argv,
                 DeeObject *kw) {
	DeeObject *name;
	uint16_t opid;
	if (DeeArg_UnpackKw(argc, argv, kw, getattr_kwdlist, "o:hasoperator", &name))
		goto err;
	if (DeeString_Check(name)) {
		opid = Dee_OperatorFromName(self, DeeString_STR(name));
		if (opid == (uint16_t)-1)
			goto nope;
	} else {
		if (DeeObject_AsUInt16(name, &opid))
			goto err;
	}
	if (DeeType_HasOperator(self, opid))
		return_true;
nope:
	return_false;
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
type_hasprivateoperator(DeeTypeObject *__restrict self,
                        size_t argc, DeeObject **__restrict argv,
                        DeeObject *kw) {
	DeeObject *name;
	uint16_t opid;
	if (DeeArg_UnpackKw(argc, argv, kw, getattr_kwdlist, "o:hasprivateoperator", &name))
		goto err;
	if (DeeString_Check(name)) {
		opid = Dee_OperatorFromName(self, DeeString_STR(name));
		if (opid == (uint16_t)-1)
			goto nope;
	} else {
		if (DeeObject_AsUInt16(name, &opid))
			goto err;
	}
	if (DeeType_HasPrivateOperator(self, opid))
		return_true;
nope:
	return_false;
err:
	return NULL;
}

#ifndef CONFIG_NO_DEEMON_100_COMPAT
PRIVATE DREF DeeObject *DCALL
type_derivedfrom_not_same(DeeTypeObject *__restrict self, size_t argc,
                          DeeObject **__restrict argv) {
	DeeTypeObject *other;
	if (DeeArg_Unpack(argc, argv, "o:derived_from", &other))
		return NULL;
	return_bool(self != other && DeeType_IsInherited(self, other));
}

PRIVATE DREF DeeObject *DCALL
type_is_vartype(DeeTypeObject *__restrict self, size_t argc,
                DeeObject **__restrict argv) {
	if (DeeArg_Unpack(argc, argv, ":is_vartype"))
		return NULL;
	return_bool(DeeType_IsVariable(self));
}

PRIVATE DREF DeeObject *DCALL
type_is_heaptype(DeeTypeObject *__restrict self, size_t argc,
                 DeeObject **__restrict argv) {
	if (DeeArg_Unpack(argc, argv, ":is_heaptype"))
		return NULL;
	return_bool(DeeType_IsCustom(self));
}

PRIVATE DREF DeeObject *DCALL
type_is_gctype(DeeTypeObject *__restrict self, size_t argc,
               DeeObject **__restrict argv) {
	if (DeeArg_Unpack(argc, argv, ":is_gctype"))
		return NULL;
	return_bool(DeeType_IsGC(self));
}

PRIVATE DREF DeeObject *DCALL
type_is_final(DeeTypeObject *__restrict self, size_t argc,
              DeeObject **__restrict argv) {
	if (DeeArg_Unpack(argc, argv, ":is_final"))
		return NULL;
	return_bool(DeeType_IsFinal(self));
}

PRIVATE DREF DeeObject *DCALL
type_is_class(DeeTypeObject *__restrict self, size_t argc,
              DeeObject **__restrict argv) {
	if (DeeArg_Unpack(argc, argv, ":is_class"))
		return NULL;
	return_bool(DeeType_IsClass(self));
}

PRIVATE DREF DeeObject *DCALL
type_is_complete(DeeTypeObject *__restrict UNUSED(self),
                 size_t argc, DeeObject **__restrict argv) {
	if (DeeArg_Unpack(argc, argv, ":is_complete"))
		return NULL;
	return_true;
}

PRIVATE DREF DeeObject *DCALL
type_is_classtype(DeeTypeObject *__restrict UNUSED(self),
                  size_t argc, DeeObject **__restrict argv) {
	if (DeeArg_Unpack(argc, argv, ":is_class_type"))
		return NULL;
	return_false;
}

PRIVATE DREF DeeObject *DCALL
type_is_ctypes_class(DeeTypeObject *__restrict self,
                     char const *__restrict name) {
	DREF DeeObject *temp;
	int error;
	temp = DeeObject_GetAttrString((DeeObject *)self, "isstructured");
	if
		unlikely(!temp)
	goto err;
	error = DeeObject_Bool(temp);
	Dee_Decref(temp);
	if
		unlikely(error < 0)
	goto err;
	if (!error)
		goto nope;
	temp = DeeObject_GetAttrString((DeeObject *)self, name);
	if
		unlikely(!temp)
	goto err;
	error = DeeObject_Bool(temp);
	Dee_Decref(temp);
	if
		unlikely(error < 0)
	goto err;
	if (error)
		return_true;
nope:
	return_false;
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
type_is_pointer(DeeTypeObject *__restrict self,
                size_t argc, DeeObject **__restrict argv) {
	if (DeeArg_Unpack(argc, argv, ":is_pointer"))
		return NULL;
	return type_is_ctypes_class(self, "ispointer");
}

PRIVATE DREF DeeObject *DCALL
type_is_lvalue(DeeTypeObject *__restrict self,
               size_t argc, DeeObject **__restrict argv) {
	if (DeeArg_Unpack(argc, argv, ":is_pointer"))
		return NULL;
	return type_is_ctypes_class(self, "islvalue");
}

PRIVATE DREF DeeObject *DCALL
type_is_structured(DeeTypeObject *__restrict self,
                   size_t argc, DeeObject **__restrict argv) {
	if (DeeArg_Unpack(argc, argv, ":is_structured"))
		return NULL;
	return DeeObject_GetAttrString((DeeObject *)self, "isstructured");
}

PRIVATE DREF DeeObject *DCALL
type_is_struct(DeeTypeObject *__restrict self,
               size_t argc, DeeObject **__restrict argv) {
	if (DeeArg_Unpack(argc, argv, ":is_struct"))
		return NULL;
	return type_is_ctypes_class(self, "isstruct");
}

PRIVATE DREF DeeObject *DCALL
type_is_array(DeeTypeObject *__restrict self,
              size_t argc, DeeObject **__restrict argv) {
	if (DeeArg_Unpack(argc, argv, ":is_array"))
		return NULL;
	return type_is_ctypes_class(self, "isarray");
}

PRIVATE DREF DeeObject *DCALL
type_is_foreign_function(DeeTypeObject *__restrict self,
                         size_t argc, DeeObject **__restrict argv) {
	if (DeeArg_Unpack(argc, argv, ":is_foreign_function"))
		return NULL;
	return type_is_ctypes_class(self, "isfunction");
}

PRIVATE DREF DeeObject *DCALL
type_is_filetype(DeeTypeObject *__restrict self, size_t argc,
                 DeeObject **__restrict argv) {
	if (DeeArg_Unpack(argc, argv, ":is_file"))
		return NULL;
	return_bool(Dee_TYPE(self) == &DeeFileType_Type);
}

PRIVATE DREF DeeObject *DCALL
type_is_superbase(DeeTypeObject *__restrict self, size_t argc,
                  DeeObject **__restrict argv) {
	if (DeeArg_Unpack(argc, argv, ":is_super_base"))
		return NULL;
	return_bool(DeeType_Base(self) == NULL);
}
#endif /* !CONFIG_NO_DEEMON_100_COMPAT */

PRIVATE struct type_method type_methods[] = {
	{ "baseof", (DREF DeeObject * (DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict)) & type_baseof,
	  DOC("(other:?DType)->?Dbool\n"
	      "Returns :true if @this Type is equal to, or a base of @other\n"
	      "If @other isn't a Type, :false is returned\n"
	      "Using baseof, the behavior of ${x is y} can be approximated as:\n"
	      ">print y.baseof(type(x)); // print x is y;"),
	  TYPE_METHOD_FKWDS },
	{ "derivedfrom", (DREF DeeObject * (DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict)) & type_derivedfrom,
	  DOC("(other:?DType)->?Dbool\n"
	      "Returns :true if @this Type is equal to, or has been derived from @other\n"
	      "If @other isn't a Type, :false is returned"),
	  TYPE_METHOD_FKWDS },
	{ "newinstance", (DREF DeeObject * (DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict)) & type_newinstance,
	  DOC("(fields!!)->\n"
	      "Allocate a new instance of @this Type and initialize members in accordance ot @fields\n"
	      ">class MyClass {\n"
	      "> member foo;\n"
	      "> this = del; /* Delete the regular constructor. */\n"
	      ">}\n"
	      ">local x = MyClass.newinstance(foo: 42);\n"
	      ">print x.foo;\n"
	      "\n"
	      "(initializer:?S?T2?DType?T1?S?T2?Dstring?O=!N)->\n"                 /* {(Type,({(string,object)...},)...} */
	      "(initializer:?S?T2?DType?T2?S?T2?Dstring?O?N=!N)->\n"               /* {(Type,({(string,object)...},none)...} */
	      "(initializer:?S?T2?DType?T2?S?T2?Dstring?O?DTuple=!N)->\n"          /* {(Type,({(string,object)...},tuple)...} */
	      "(initializer:?S?T2?DType?T3?S?T2?Dstring?O?DTuple?N=!N)->\n"        /* {(Type,({(string,object)...},tuple,none)...} */
	      "(initializer:?S?T2?DType?T3?S?T2?Dstring?O?DTuple?DMapping=!N)->\n" /* {(Type,({(string,object)...},tuple,mapping)...} */
	      "(initializer:?S?T2?DType?T2?N?DTuple=!N)->\n"                       /* {(Type,(none,tuple)...} */
	      "(initializer:?S?T2?DType?T3?N?DTuple?N=!N)->\n"                     /* {(Type,(none,tuple,none)...} */
	      "(initializer:?S?T2?DType?T3?N?DTuple?DMapping=!N)->\n"              /* {(Type,(none,tuple,mapping)...} */
	      "@throw TypeError No superargs tuple was provided for one of the Type's bases, when that base "
	      "has a mandatory constructor that can't be invoked without any arguments. "
	      "Note that a user-defined class never has a mandatory constructor, with this "
	      "only affecting builtin types such as :InstanceMethod or :property\n"
	      "A extended way of constructing and initializing a Type, that involves providing explicit "
	      "member initializers on a per-Type bases, as well as argument tuples and optional keyword "
	      "mappings to-be used for construction of one of the Type's sub-classes (allowing to provide "
	      "for explicit argument lists when one of the Type's bases has a mandatory constructor)\n"
	      ">import list from deemon;\n"
	      ">class MyList: List {\n"
	      "> this = del;\n"
	      "> member mylist_member;\n"
	      "> appendmember() {\n"
	      ">  this.append(mylist_member);\n"
	      "> }\n"
	      ">}\n"
	      ">local x = MyList.newinstance({\n"
	      "> MyList: ({ \"mylist_member\" : \"abc\" },none),\n"
	      "> list:   ({ },pack([10,20,30])),\n"
	      ">});\n"
	      ">print repr x;          /* [10,20,30] */\n"
	      ">print x.mylist_member; /* \"abc\" */\n"
	      ">x.appendmember();\n"
	      ">print repr x;          /* [10,20,30,\"abc\"] */"),
	  TYPE_METHOD_FKWDS },
	{ "hasattribute", (DREF DeeObject * (DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict)) & type_hasattribute,
	  DOC("(name:?Dstring)->?Dbool\n"
	      "Returns :true if this type, or one of its sub-classes defines an "
	      "instance-attribute @name, and doesn't define any attribute-operators. "
	      "Otherwise, return :false\n"
	      ">function hasattribute(name) {\n"
	      "> import attribute from deemon;\n"
	      "> return attribute.exists(this,name,\"ic\",\"ic\")\n"
	      ">}\n"
	      "Note that this function only searches instance-attributes, meaning that class/static "
	      "attributes/members such as :string.Iterator are not matched, whereas something like :string.find is\n"
	      "Note that this function is quite similar to #hasinstanceattr, however unlike "
	      "that function, this function will stop searching the base-classes of @this Type "
	      "when one of that types implements one of the attribute operators."),
	  TYPE_METHOD_FKWDS },
	{ "hasprivateattribute", (DREF DeeObject * (DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict)) & type_hasprivateattribute,
	  DOC("(name:?Dstring)->?Dbool\n"
	      "Similar to #hasattribute, but only looks at attributes declared by "
	      "@this Type, excluding any defined by a sub-class.\n"
	      ">function hasattribute(name) {\n"
	      "> import attribute from deemon;\n"
	      "> return attribute.exists(this,name,\"ic\",\"ic\",this)\n"
	      ">}"),
	  TYPE_METHOD_FKWDS },
	{ "hasoperator", (DREF DeeObject * (DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict)) & type_hasoperator,
	  DOC("(name:?Dint)->?Dbool\n"
	      "(name:?Dstring)->?Dbool\n"
	      "Returns :true if instances of @this Type implement an operator @name, "
	      "or :false if not, or if @name is not recognized as an operator provided "
	      "available for the Type-Type that is ${type this}\n"
	      "Note that this function also looks at the operators of "
	      "base-classes, as well as that a user-defined class that has "
	      "explicitly deleted an operator will cause this function to "
	      "return true, indicative of that operator being implemented "
	      "to cause an error to be thrown when invoked.\n"
	      "The given @name is the so-called real operator name, "
	      "as listed under Name in the following table:\n"
	      "%{table Name|Symbolical name|Prototype\n"
	      "$\"constructor\"|$\"this\"|${this(args!)}\n"
	      "$\"copy\"|$\"copy\"|${copy()}\n"
	      "$\"deepcopy\"|$\"deepcopy\"|${deepcopy()}\n"
	      "$\"destructor\"|$\"~this\"|${~this()}\n"
	      "$\"assign\"|$\":=\"|${operator := (other)}\n"
	      "$\"str\"|$\"str\"|${operator str(): string}\n"
	      "$\"repr\"|$\"repr\"|${operator repr(): string}\n"
	      "$\"bool\"|$\"bool\"|${operator bool(): bool}\n"
	      "$\"call\"|$\"()\"|${operator ()(args!): Object}\n"
	      "$\"next\"|$\"next\"|${operator next(): Object}\n"
	      "$\"int\"|$\"int\"|${operator int(): int}\n"
	      "$\"float\"|$\"float\"|${operator float(): float}\n"
	      "$\"inv\"|$\"~\"|${operator ~ (): Object}\n"
	      "$\"pos\"|$\"+\"|${operator + (): Object}\n"
	      "$\"neg\"|$\"-\"|${operator - (): Object}\n"
	      "$\"add\"|$\"+\"|${operator + (other): Object}\n"
	      "$\"sub\"|$\"-\"|${operator - (other): Object}\n"
	      "$\"mul\"|$\"*\"|${operator * (other): Object}\n"
	      "$\"div\"|$\"/\"|${operator / (other): Object}\n"
	      "$\"mod\"|$\"%\"|${operator % (other): Object}\n"
	      "$\"shl\"|$\"<<\"|${operator << (other): Object}\n"
	      "$\"shr\"|$\">>\"|${operator >> (other): Object}\n"
	      "$\"and\"|$\"&\"|${operator & (other): Object}\n"
	      "$\"or\"|${\"|\"}|${operator | (other): Object}\n"
	      "$\"xor\"|$\"^\"|${operator ^ (other): Object}\n"
	      "$\"pow\"|$\"**\"|${operator ** (other): Object}\n"
	      "$\"inc\"|$\"++\"|${operator ++ (): Object}\n"
	      "$\"dec\"|$\"--\"|${operator -- (): Object}\n"
	      "$\"iadd\"|$\"+=\"|${operator += (other): Object}\n"
	      "$\"isub\"|$\"-=\"|${operator -= (other): Object}\n"
	      "$\"imul\"|$\"*=\"|${operator *= (other): Object}\n"
	      "$\"idiv\"|$\"/=\"|${operator /= (other): Object}\n"
	      "$\"imod\"|$\"%=\"|${operator %= (other): Object}\n"
	      "$\"ishl\"|$\"<<=\"|${operator <<= (other): Object}\n"
	      "$\"ishr\"|$\">>=\"|${operator >>= (other): Object}\n"
	      "$\"iand\"|$\"&=\"|${operator &= (other): Object}\n"
	      "$\"ior\"|${\"|=\"}|${operator |= (other): Object}\n"
	      "$\"ixor\"|$\"^=\"|${operator ^= (other): Object}\n"
	      "$\"ipow\"|$\"**=\"|${operator **= (other): Object}\n"
	      "$\"hash\"|$\"hash\"|${operator hash(): int}\n"
	      "$\"eq\"|$\"==\"|${operator == (other): Object}\n"
	      "$\"ne\"|$\"!=\"|${operator != (other): Object}\n"
	      "$\"lo\"|$\"<\"|${operator < (other): Object}\n"
	      "$\"le\"|$\"<=\"|${operator <= (other): Object}\n"
	      "$\"gr\"|$\">\"|${operator > (other): Object}\n"
	      "$\"ge\"|$\">=\"|${operator >= (other): Object}\n"
	      "$\"iter\"|$\"iter\"|${operator iter(): Object}\n"
	      "$\"size\"|$\"#\"|${operator # (): Object}\n"
	      "$\"contains\"|$\"contains\"|${operator contains(item): Object}\n"
	      "$\"getitem\"|$\"[]\"|${operator [] (index): Object}\n"
	      "$\"delitem\"|$\"del[]\"|${operator del[] (index): none}\n"
	      "$\"setitem\"|$\"[]=\"|${operator []= (index,value): none}\n"
	      "$\"getrange\"|$\"[:]\"|${operator [:] (start,end): Object}\n"
	      "$\"delrange\"|$\"del[:]\"|${operator del[:] (start,end): none}\n"
	      "$\"setrange\"|$\"[:]=\"|${operator [:]= (start,end,value): none}\n"
	      "$\"getattr\"|$\".\"|${operator . (name: string): Object}\n"
	      "$\"delattr\"|$\"del.\"|${operator del. (name: string): none}\n"
	      "$\"setattr\"|$\".=\"|${operator .= (name: string, value): none}\n"
	      "$\"enumattr\"|$\"enumattr\"|${operator enumattr(): {attribute...}}\n"
	      "$\"enter\"|$\"enter\"|${operator enter(): none}\n"
	      "$\"leave\"|$\"leave\"|${operator leave(): none}\n"
	      "}"),
	  TYPE_METHOD_FKWDS },
	{ "hasprivateoperator", (DREF DeeObject * (DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict)) & type_hasprivateoperator,
	  DOC("(name:?Dint)->?Dbool\n"
	      "(name:?Dstring)->?Dbool\n"
	      "Returns :true if instances of @this Type implement an operator @name, "
	      "or :false if not, or if @name is not recognized as an operator provided "
	      "available for the Type-Type that is ${type this}\n"
	      "Note that this function intentionally don't look at operators of "
	      "base-classes (which is instead done by #hasoperator), meaning that "
	      "inherited operators are not included, with the exception of explicitly "
	      "inherited constructors\n"
	      "For a list of operator names, see #hasoperator"),
	  TYPE_METHOD_FKWDS },
	{ meth_getinstanceattr + 2, (DREF DeeObject * (DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict)) & type_getinstanceattr,
	  DOC("(name:?Dstring)->\n"
	      "Lookup an attribute @name that is implemented by instances of @this Type\n"
	      "Normally, such attributes can also be accessed using regular attribute lookup, "
	      "however in ambiguous cases where both the Type, as well as instances implement "
	      "an attribute of the same name (s.a. :Dict.c:keys vs. :Dict.i:keys), using regular "
	      "attribute lookup on the Type (as in ${Dict.keys}) will always return the Type-attribute, "
	      "rather than a wrapper around the instance attribute.\n"
	      "In such cases, this function may be used to explicitly lookup the instance variant:\n"
	      ">import Dict from deemon;\n"
	      ">local dict_keys_function = Dict.getinstanceattr(\"keys\");\n"
	      ">local my_dict_instance = Dict();\n"
	      ">my_dict_instance[\"foo\"] = \"bar\";\n"
	      ">// Same as `my_dict_instance.keys()' -- { \"foo\" }\n"
	      ">print repr dict_keys_function(my_dict_instance);\n"
	      "Note that one minor exception exists to the default lookup rule, and it relates to how "
	      "attributes of :Type itself are queried (such as in the expression ${(type_ from deemon).baseof}).\n"
	      "In this case, access is always made as an instance-bound, meaning that for this purpose, :Type "
	      "is considered an instance of :Type (typetype), rather than the type of :Type (typetype) (I know that sounds complicated, "
	      "but without this rule, ${(type_ from deemon).baseof} would return a class method object taking 2 "
	      "arguments, rather than the intended single argument)\n"
	      "Also note that the `*instanceattr' functions will not check for types that have overwritten "
	      "one of the attribute-operators, but will continue search for matching attribute names, even "
	      "if those attributes would normally have been overshadowed by attribute callbacks"),
	  TYPE_METHOD_FKWDS },
	{ meth_callinstanceattr, (DREF DeeObject * (DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict)) & type_callinstanceattr,
	  DOC("(name:?Dstring,args!)->\n"
	      "s.a. #getinstanceattr"),
	  TYPE_METHOD_FKWDS },
	{ meth_hasinstanceattr + 2, (DREF DeeObject * (DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict)) & type_hasinstanceattr,
	  DOC("(name:?Dstring)->?Dbool\n"
	      "s.a. #getinstanceattr"),
	  TYPE_METHOD_FKWDS },
	{ meth_boundinstanceattr + 4, (DREF DeeObject * (DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict)) & type_boundinstanceattr,
	  DOC("(name:?Dstring,allow_missing=!t)->?Dbool\n"
	      "s.a. #getinstanceattr"),
	  TYPE_METHOD_FKWDS },
	{ meth_delinstanceattr + 2, (DREF DeeObject * (DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict)) & type_delinstanceattr,
	  DOC("(name:?Dstring)\n"
	      "s.a. #getinstanceattr"),
	  TYPE_METHOD_FKWDS },
	{ meth_setinstanceattr + 3, (DREF DeeObject * (DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict)) & type_setinstanceattr,
	  DOC("(name:?Dstring,value)->\n"
	      "s.a. #getinstanceattr"),
	  TYPE_METHOD_FKWDS },

#ifndef CONFIG_NO_DEEMON_100_COMPAT
	/* Deprecated functions */
	{ "same_or_derived_from", (DREF DeeObject * (DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict)) & type_derivedfrom, DOC("(other:?DType)->?Dbool\nDeprecated alias for #derivedfrom"), TYPE_METHOD_FKWDS },
	{ "derived_from", (DREF DeeObject * (DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict)) & type_derivedfrom_not_same, DOC("(other:?DType)->?Dbool\nDeprecated alias for ${this !== other && this.derivedfrom(other)}") },
	{ "is_vartype", (DREF DeeObject * (DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict)) & type_is_vartype, DOC("->?Dbool\nDeprecated alias for #__isvariable__") },
	{ "is_heaptype", (DREF DeeObject * (DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict)) & type_is_heaptype, DOC("->?Dbool\nDeprecated alias for #__iscustom__") },
	{ "is_gctype", (DREF DeeObject * (DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict)) & type_is_gctype, DOC("->?Dbool\nDeprecated alias for #__isgc__") },
	{ "is_final", (DREF DeeObject * (DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict)) & type_is_final, DOC("->?Dbool\nDeprecated alias for #isfinal") },
	{ "is_class", (DREF DeeObject * (DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict)) & type_is_class, DOC("->?Dbool\nDeprecated alias for #__isclass__") },
	{ "is_complete", (DREF DeeObject * (DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict)) & type_is_complete, DOC("->?Dbool\nDeprecated (always returns :true)") },
	{ "is_classtype", (DREF DeeObject * (DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict)) & type_is_classtype, DOC("->?Dbool\nDeprecated (always returns :false)") },
	{ "is_pointer", (DREF DeeObject * (DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict)) & type_is_pointer, DOC("->?Dbool\nDeprecated alias for ${try this.isstructured && this.ispointer catch ((Error from deemon).AttributeError) false}") },
	{ "is_lvalue", (DREF DeeObject * (DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict)) & type_is_lvalue, DOC("->?Dbool\nDeprecated alias for ${try this.isstructured && this.islvalue catch ((Error from deemon).AttributeError) false}") },
	{ "is_structured", (DREF DeeObject * (DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict)) & type_is_structured, DOC("->?Dbool\nDeprecated alias for ${try this.isstructured catch ((Error from deemon).AttributeError) false}") },
	{ "is_struct", (DREF DeeObject * (DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict)) & type_is_struct, DOC("->?Dbool\nDeprecated alias for ${try this.isstructured && this.isstruct catch ((Error from deemon).AttributeError) false}") },
	{ "is_array", (DREF DeeObject * (DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict)) & type_is_array, DOC("->?Dbool\nDeprecated alias for ${try this.isstructured && this.isarray catch ((Error from deemon).AttributeError) false}") },
	{ "is_foreign_function", (DREF DeeObject * (DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict)) & type_is_foreign_function, DOC("->?Dbool\nDeprecated alias for ${try this.isstructured && this.isfunction catch ((Error from deemon).AttributeError) false}") },
	{ "is_file", (DREF DeeObject * (DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict)) & type_is_filetype, DOC("->?Dbool\nDeprecated alias for ${this is type(File from deemon)}") },
	{ "is_super_base", (DREF DeeObject * (DCALL *)(DeeObject * __restrict, size_t, DeeObject **__restrict)) & type_is_superbase, DOC("->?Dbool\nDeprecated alias for ${this.__base__ !is bound}") },
#endif /* !CONFIG_NO_DEEMON_100_COMPAT */

	{ NULL }
};

PRIVATE struct type_member type_members[] = {
	TYPE_MEMBER_FIELD_DOC("__name__", STRUCT_CONST | STRUCT_CSTR_OPT, offsetof(DeeTypeObject, tp_name), "->?X2?Dstring?N"),
	TYPE_MEMBER_FIELD_DOC("__doc__", STRUCT_CONST | STRUCT_CSTR_OPT, offsetof(DeeTypeObject, tp_doc), "->?X2?Dstring?N"),
	TYPE_MEMBER_FIELD_DOC("__base__", STRUCT_OBJECT_OPT, offsetof(DeeTypeObject, tp_base), "->?X2?DType?N"),
	TYPE_MEMBER_END
};


/*[[[deemon
import File from deemon;
local options = [];
for (local line: File.open("../../../include/deemon/object.h")) {
	local name;
	try name = "Is" + line.scanf(" # define DeeType_Is%[^(](")[0];
	catch (...)
	try name = "Has" + line.scanf(" # define DeeType_Has%[^(](")[0];
	catch (...) continue;
	options.append(name);
}
for (local o: options) {
	print "PRIVATE DREF DeeObject *DCALL";
	print "type_"+o.lower()+"(DeeObject *__restrict self) {";
	print "\treturn_bool(DeeType_"+o+"(self));";
	print "}";
}
print "#define TYPE_FEATURE_GETSETS \\";
for (local o: options) {
	local isname = o.lower();
	if (isname !in ["isfinal","isabstract","isinterrupt"])
		isname = "__" + isname + "__";
	print "\t{ "+repr(isname)+", &type_"+o.lower()+", NULL, NULL, DOC(\"->?Dbool\") }, \\";
}
print "/" "* ... *" "/";
]]]*/
PRIVATE DREF DeeObject *DCALL
type_isfinal(DeeObject *__restrict self) {
	return_bool(DeeType_IsFinal(self));
}

PRIVATE DREF DeeObject *DCALL
type_isinterrupt(DeeObject *__restrict self) {
	return_bool(DeeType_IsInterrupt(self));
}

PRIVATE DREF DeeObject *DCALL
type_isabstract(DeeObject *__restrict self) {
	return_bool(DeeType_IsAbstract(self));
}

PRIVATE DREF DeeObject *DCALL
type_isvariable(DeeObject *__restrict self) {
	return_bool(DeeType_IsVariable(self));
}

PRIVATE DREF DeeObject *DCALL
type_isgc(DeeObject *__restrict self) {
	return_bool(DeeType_IsGC(self));
}

PRIVATE DREF DeeObject *DCALL
type_isclass(DeeObject *__restrict self) {
	return_bool(DeeType_IsClass(self));
}

PRIVATE DREF DeeObject *DCALL
type_isarithmetic(DeeObject *__restrict self) {
	return_bool(DeeType_IsArithmetic(self));
}

PRIVATE DREF DeeObject *DCALL
type_iscomparable(DeeObject *__restrict self) {
	return_bool(DeeType_IsComparable(self));
}

PRIVATE DREF DeeObject *DCALL
type_issequence(DeeObject *__restrict self) {
	return_bool(DeeType_IsSequence(self));
}

PRIVATE DREF DeeObject *DCALL
type_isinttruncated(DeeObject *__restrict self) {
	return_bool(DeeType_IsIntTruncated(self));
}

PRIVATE DREF DeeObject *DCALL
type_hasmoveany(DeeObject *__restrict self) {
	return_bool(DeeType_HasMoveAny(self));
}

PRIVATE DREF DeeObject *DCALL
type_isiterator(DeeObject *__restrict self) {
	return_bool(DeeType_IsIterator(self));
}

PRIVATE DREF DeeObject *DCALL
type_istypetype(DeeObject *__restrict self) {
	return_bool(DeeType_IsTypeType(self));
}

PRIVATE DREF DeeObject *DCALL
type_iscustom(DeeObject *__restrict self) {
	return_bool(DeeType_IsCustom(self));
}

PRIVATE DREF DeeObject *DCALL
type_issuperconstructible(DeeObject *__restrict self) {
	return_bool(DeeType_IsSuperConstructible(self));
}

PRIVATE DREF DeeObject *DCALL
type_isnoargconstructible(DeeObject *__restrict self) {
	return_bool(DeeType_IsNoArgConstructible(self));
}

PRIVATE DREF DeeObject *DCALL
type_isvarargconstructible(DeeObject *__restrict self) {
	return_bool(DeeType_IsVarArgConstructible(self));
}

PRIVATE DREF DeeObject *DCALL
type_isconstructible(DeeObject *__restrict self) {
	return_bool(DeeType_IsConstructible(self));
}

PRIVATE DREF DeeObject *DCALL
type_iscopyable(DeeObject *__restrict self) {
	return_bool(DeeType_IsCopyable(self));
}
#define TYPE_FEATURE_GETSETS \
	{ "isfinal", &type_isfinal, NULL, NULL, DOC("->?Dbool") }, \
	{ "isinterrupt", &type_isinterrupt, NULL, NULL, DOC("->?Dbool") }, \
	{ "isabstract", &type_isabstract, NULL, NULL, DOC("->?Dbool") }, \
	{ "__isvariable__", &type_isvariable, NULL, NULL, DOC("->?Dbool") }, \
	{ "__isgc__", &type_isgc, NULL, NULL, DOC("->?Dbool") }, \
	{ "__isclass__", &type_isclass, NULL, NULL, DOC("->?Dbool") }, \
	{ "__isarithmetic__", &type_isarithmetic, NULL, NULL, DOC("->?Dbool") }, \
	{ "__iscomparable__", &type_iscomparable, NULL, NULL, DOC("->?Dbool") }, \
	{ "__issequence__", &type_issequence, NULL, NULL, DOC("->?Dbool") }, \
	{ "__isinttruncated__", &type_isinttruncated, NULL, NULL, DOC("->?Dbool") }, \
	{ "__hasmoveany__", &type_hasmoveany, NULL, NULL, DOC("->?Dbool") }, \
	{ "__isiterator__", &type_isiterator, NULL, NULL, DOC("->?Dbool") }, \
	{ "__istypetype__", &type_istypetype, NULL, NULL, DOC("->?Dbool") }, \
	{ "__iscustom__", &type_iscustom, NULL, NULL, DOC("->?Dbool") }, \
	{ "__issuperconstructible__", &type_issuperconstructible, NULL, NULL, DOC("->?Dbool") }, \
	{ "__isnoargconstructible__", &type_isnoargconstructible, NULL, NULL, DOC("->?Dbool") }, \
	{ "__isvarargconstructible__", &type_isvarargconstructible, NULL, NULL, DOC("->?Dbool") }, \
	{ "__isconstructible__", &type_isconstructible, NULL, NULL, DOC("->?Dbool") }, \
	{ "__iscopyable__", &type_iscopyable, NULL, NULL, DOC("->?Dbool") }, \
/* ... */
//[[[end]]]


PRIVATE DREF DeeObject *DCALL
type_isbuffer(DeeTypeObject *__restrict self) {
	do {
		if (self->tp_buffer && self->tp_buffer->tp_getbuf)
			return_true;
	} while ((self = self->tp_base) != NULL);
	return_false;
}

PRIVATE DREF DeeObject *DCALL
type_get_classdesc(DeeTypeObject *__restrict self) {
	if (!self->tp_class) {
		DeeError_Throwf(&DeeError_AttributeError,
		                "Can't access `__class__' of non-user-defined Type `%s'",
		                self->tp_name);
		return NULL;
	}
	return_reference_((DeeObject *)self->tp_class->cd_desc);
}

PRIVATE DREF DeeObject *DCALL
type_issingleton(DeeTypeObject *__restrict self) {
	if (self->tp_features & TF_SINGLETON)
		return_true; /* Alternative means of creation. */
	if (self->tp_class) {
		/* Special handling for user-defined classes. */
		if (self->tp_init.tp_alloc.tp_ctor ||
		    self->tp_init.tp_alloc.tp_any_ctor ||
		    self->tp_init.tp_alloc.tp_any_ctor_kw)
			return_true; /* The type is isn't constructible. */
	}
	return_false;
}


PRIVATE DREF DeeObject *DCALL
get_module_from_addr(struct class_desc *__restrict my_class, uint16_t addr) {
	DeeObject *slot;
	DREF DeeModuleObject *result = NULL;
	rwlock_read(&my_class->cd_lock);
	slot = my_class->cd_members[addr];
	if (slot && DeeFunction_Check(slot)) {
		result = ((DeeFunctionObject *)slot)->fo_code->co_module;
		Dee_Incref(result);
	}
	rwlock_endread(&my_class->cd_lock);
	return (DREF DeeObject *)result;
}

LOCAL DREF DeeObject *DCALL
DeeClass_GetModule(DeeTypeObject *__restrict self) {
	struct class_desc *my_class    = self->tp_class;
	DeeClassDescriptorObject *desc = my_class->cd_desc;
	DREF DeeObject *result;
	size_t i;
	/* Search through the operator bindings table. */
	for (i = 0; i <= desc->cd_clsop_mask; ++i) {
		if (desc->cd_clsop_list[i].co_name == (uint16_t)-1)
			continue;
		result = get_module_from_addr(my_class,
		                              desc->cd_clsop_list[i].co_addr);
		if (result)
			goto done;
	}
	/* Search through class attribute table. */
	for (i = 0; i <= desc->cd_cattr_mask; ++i) {
		if (!desc->cd_cattr_list[i].ca_name)
			continue;
		result = get_module_from_addr(my_class,
		                              desc->cd_cattr_list[i].ca_addr);
		if (result)
			goto done;
		if ((desc->cd_cattr_list[i].ca_flag &
		     (CLASS_ATTRIBUTE_FREADONLY | CLASS_ATTRIBUTE_FGETSET)) ==
		    (CLASS_ATTRIBUTE_FGETSET)) {
			result = get_module_from_addr(my_class,
			                              desc->cd_cattr_list[i].ca_addr + CLASS_GETSET_DEL);
			if (result)
				goto done;
			result = get_module_from_addr(my_class,
			                              desc->cd_cattr_list[i].ca_addr + CLASS_GETSET_SET);
			if (result)
				goto done;
		}
	}
	for (i = 0; i <= desc->cd_iattr_mask; ++i) {
		if (!desc->cd_iattr_list[i].ca_name)
			continue;
		if (!(desc->cd_iattr_list[i].ca_flag & CLASS_ATTRIBUTE_FCLASSMEM))
			continue;
#if CLASS_GETSET_GET == 0
		result = get_module_from_addr(my_class,
		                              desc->cd_iattr_list[i].ca_addr);
		if (result)
			goto done;
#endif
		if ((desc->cd_iattr_list[i].ca_flag &
		     (CLASS_ATTRIBUTE_FREADONLY | CLASS_ATTRIBUTE_FGETSET)) ==
		    (CLASS_ATTRIBUTE_FGETSET)) {
#if CLASS_GETSET_GET != 0
			result = get_module_from_addr(my_class,
			                              desc->cd_iattr_list[i].ca_addr + CLASS_GETSET_GET);
			if (result)
				goto done;
#endif
			result = get_module_from_addr(my_class,
			                              desc->cd_iattr_list[i].ca_addr + CLASS_GETSET_DEL);
			if (result)
				goto done;
			result = get_module_from_addr(my_class,
			                              desc->cd_iattr_list[i].ca_addr + CLASS_GETSET_SET);
			if (result)
				goto done;
		}
#if CLASS_GETSET_GET != 0
		else {
			result = get_module_from_addr(my_class,
			                              desc->cd_iattr_list[i].ca_addr);
			if (result)
				goto done;
		}
#endif
	}
	return NULL;
done:
	return result;
}

PUBLIC DREF DeeObject *DCALL
DeeType_GetModule(DeeTypeObject *__restrict self) {
	DREF DeeObject *result;
	/* - For user-defined classes: search though all the operator/method bindings
	 *   described for the class member table, testing them for functions and
	 *   returning the module that they are bound to.
	 * - For types loaded by dex modules, do some platform-specific trickery to
	 *   determine the address space bounds within which the module was loaded,
	 *   then simply compare the type pointer against those bounds.
	 * - All other types are defined as part of the builtin `deemon' module. */
again:
	if (self->tp_class)
		return DeeClass_GetModule(self);
	if (!(self->tp_flags & TP_FHEAP)) {
		/* Lookup the originating module of a statically allocated C-type. */
		result = DeeModule_FromStaticPointer(self);
		if (result)
			return result;
	}
	/* Special case for custom type-types (such
	 * as those provided by the `ctypes' module)
	 *  -> In this case, we simply return the module associated with the
	 *     typetype, thus allowing custom types to be resolved as well. */
	if (self != Dee_TYPE(self)) {
		self = Dee_TYPE(self);
		if (self != &DeeType_Type &&
		    self != &DeeFileType_Type)
			goto again;
	}
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
type_get_module(DeeTypeObject *__restrict self) {
	DREF DeeObject *result;
	result = DeeType_GetModule(self);
	if
		unlikely(!result)
	return_none;
	return result;
}

PRIVATE DREF DeeObject *DCALL
type_get_instancesize(DeeTypeObject *__restrict self) {
	if (self->tp_flags & TP_FVARIABLE)
		goto retnone;
	if (!self->tp_init.tp_alloc.tp_ctor &&
	    !self->tp_init.tp_alloc.tp_copy_ctor &&
	    !self->tp_init.tp_alloc.tp_deep_ctor &&
	    !self->tp_init.tp_alloc.tp_any_ctor &&
	    !self->tp_init.tp_alloc.tp_any_ctor_kw)
		goto retnone;
	return DeeInt_NewSize(self->tp_init.tp_alloc.tp_instance_size);
retnone:
	return_none;
}

INTDEF DREF DeeObject *DCALL type_get_operators(DeeTypeObject *__restrict self);
INTDEF DREF DeeObject *DCALL type_get_operatorids(DeeTypeObject *__restrict self);
INTDEF DREF DeeObject *DCALL type_get_ctable(DeeTypeObject *__restrict self);

PRIVATE struct type_getset type_getsets[] = {
	TYPE_FEATURE_GETSETS { "isbuffer", (DREF DeeObject * (DCALL *)(DeeObject * __restrict)) & type_isbuffer, NULL, NULL,
	                       DOC("->?Dbool\n"
	                           "Returns :true if @this Type implements the buffer interface\n"
	                           "The most prominent Type to which this applies is :Bytes, however other types also support this") },
	/* TODO: __mro__->?S?DType
	 *    - Returns a sequence proxy detailing the method
	 *     (attribute) resolution order used for @this Type:
	 *    >> property __mro__: {type_...} = {
	 *    >>     get(): {type_...} {
	 *    >>         for (local tp = this; tp !is none; tp = tp.__base__)
	 *    >>             yield tp;
	 *    >>     }
	 *    >> }
	 */
	{ "__class__", (DREF DeeObject * (DCALL *)(DeeObject * __restrict)) & type_get_classdesc, NULL, NULL,
	  DOC("->?Ert:ClassDescriptor\n"
	      "@throw AttributeError @this typeType is a user-defined class (s.a. #__isclass__)\n"
	      "Returns the internal class-descriptor descriptor for a user-defined class") },
	{ "__issingleton__", (DREF DeeObject * (DCALL *)(DeeObject * __restrict)) & type_issingleton, NULL, NULL,
	  DOC("->?Dbool\n"
	      "Check if @this Type describes a singleton object, requiring that @this type not be "
	      "implementing a constructor (or be deleting its constructor), as well as not be one "
	      "of the special internal types used to represent implementation-specific wrapper "
	      "objects for C attributes, or be generated by the compiler, such as code objects, "
	      "class descriptors or DDI information providers") },
	{ DeeString_STR(&str___module__),
	  (DREF DeeObject * (DCALL *)(DeeObject * __restrict)) & type_get_module, NULL, NULL,
	  DOC("->?X2?DModule?N\n"
	      "Return the module used to define @this Type, or :none if the module cannot "
	      "be determined, which may be the case if the Type doesn't have any defining "
	      "features such as operators, or class/instance member functions") },
	{ "__ctable__", (DREF DeeObject * (DCALL *)(DeeObject * __restrict)) & type_get_ctable, NULL, NULL,
	  DOC("->?AObjectTable?Ert:ClassDescriptor\n"
	      "Returns an indexable sequence describing the class object table, "
	      "as referenced by :rt.ClassDescriptor.attribute.addr\n"
	      "For non-user-defined classes (aka. #__isclass__ is :false), an empty sequence is returned\n"
	      "The instance-attribute table can be accessed through :Object.__itable__") },
	{ "__operators__", (DREF DeeObject * (DCALL *)(DeeObject * __restrict)) & type_get_operators, NULL, NULL,
	  DOC("->?S?X2?Dstring?Dint\n"
	      "Enumerate the names of all the operators overwritten by @this Type as a set-like sequence\n"
	      "This member functions such that the member function #hasprivateoperator can be implemented as:\n"
	      ">function hasprivateoperator(name: string | int): bool {\n"
	      "> return name in this.__operators__;\n"
	      ">}\n"
	      "Also note that this set doesn't differentiate between overwritten and deleted operators, "
	      "as for this purpose any deleted operator is considered to be implemented as throwing a "
	      ":NotImplemented exception\n"
	      "Additionally, this set also includes automatically generated operators for user-classes, "
	      "meaning that pretty much any user-class will always have its compare, assignment, as well "
	      "as constructor and destructor operators overwritten, even when the user didn't actually "
	      "define any of them\n"
	      "For the purposes of human-readable information, is is recommended to use #__class__.operators "
	      "when @this Type is a user-defined class (aka. #__isclass__ is :true), and only use #__operators__ "
	      "for all other types that this doesn't apply to") },
	{ "__operatorids__", (DREF DeeObject * (DCALL *)(DeeObject * __restrict)) & type_get_operatorids, NULL, NULL,
	  DOC("->?S?Dint\n"
	      "Enumerate the ids of all the operators overwritten by @this Type as a set-like sequence\n"
	      "This is the same as #__operators__, but the runtime will not attempt to translate known "
	      "operator ids to their user-friendly name, as described in #hasoperator") },
	{ "__instancesize__", (DREF DeeObject * (DCALL *)(DeeObject * __restrict)) & type_get_instancesize, NULL, NULL,
	  DOC("->?X2?Dint?N\n"
	      "Returns the heap allocation size of instances of @this Type, or :none when @this Type cannot "
	      "be instantiated, is a singletone (such as :none), or has variable-length instances (#isvariable)") },
#ifndef CONFIG_NO_DEEMON_100_COMPAT
	{ "__instance_size__", (DREF DeeObject * (DCALL *)(DeeObject * __restrict)) & type_get_instancesize, NULL, NULL,
	  DOC("->?X2?Dint?N\n"
	      "Deprecated alias for #__instancesize__") },
#endif /* !CONFIG_NO_DEEMON_100_COMPAT */
	{ NULL }
};


INTERN DREF DeeObject *DCALL
type_getattr(DeeObject *__restrict self,
             DeeObject *__restrict name) {
	return DeeType_GetAttrString((DeeTypeObject *)self,
	                             DeeString_STR(name),
	                             DeeString_Hash(name));
}

INTERN int DCALL
type_delattr(DeeObject *__restrict self,
             DeeObject *__restrict name) {
	return DeeType_DelAttrString((DeeTypeObject *)self,
	                             DeeString_STR(name),
	                             DeeString_Hash(name));
}

INTERN int DCALL
type_setattr(DeeObject *__restrict self,
             DeeObject *__restrict name,
             DeeObject *__restrict value) {
	return DeeType_SetAttrString((DeeTypeObject *)self,
	                             DeeString_STR(name),
	                             DeeString_Hash(name),
	                             value);
}

INTERN DREF DeeObject *DCALL
type_callattr(DeeObject *__restrict self,
              DeeObject *__restrict name,
              size_t argc, DeeObject **__restrict argv) {
	return DeeType_CallAttrString((DeeTypeObject *)self,
	                              DeeString_STR(name),
	                              DeeString_Hash(name),
	                              argc, argv);
}

INTERN DREF DeeObject *DCALL
type_callattr_kw(DeeObject *__restrict self,
                 DeeObject *__restrict name,
                 size_t argc, DeeObject **__restrict argv,
                 DeeObject *kw) {
	return DeeType_CallAttrStringKw((DeeTypeObject *)self,
	                                DeeString_STR(name),
	                                DeeString_Hash(name),
	                                argc, argv, kw);
}

INTERN dssize_t DCALL
type_enumattr(DeeTypeObject *__restrict UNUSED(tp_self),
              DeeObject *__restrict self, denum_t proc, void *arg) {
	return DeeType_EnumAttr((DeeTypeObject *)self, proc, arg);
}

PRIVATE dhash_t DCALL type_hash(DeeObject *__restrict self) {
	return Dee_HashPointer(self);
}

PRIVATE DREF DeeObject *DCALL
type_eq(DeeObject *__restrict self,
        DeeObject *__restrict some_object) {
	if (DeeObject_AssertType(some_object, &DeeType_Type))
		return NULL;
	return_bool_(self == some_object);
}

PRIVATE DREF DeeObject *DCALL
type_ne(DeeObject *__restrict self,
        DeeObject *__restrict some_object) {
	if (DeeObject_AssertType(some_object, &DeeType_Type))
		return NULL;
	return_bool_(self != some_object);
}


PRIVATE struct type_cmp type_cmp_data = {
	/* .tp_hash = */ &type_hash,
	/* .tp_eq   = */ &type_eq,
	/* .tp_ne   = */ &type_ne
};

PRIVATE struct type_attr type_attr_data = {
	/* .tp_getattr  = */ &type_getattr,
	/* .tp_delattr  = */ &type_delattr,
	/* .tp_setattr  = */ &type_setattr,
	/* .tp_enumattr = */ &type_enumattr
};

PRIVATE struct type_gc type_gc_data = {
	/* .tp_clear  = */ (void(DCALL *)(DeeObject *__restrict)) & type_clear,
	/* .tp_pclear = */ (void(DCALL *)(DeeObject *__restrict, unsigned int)) & type_pclear,
	/* .tp_gcprio = */ Dee_GC_PRIORITY_CLASS
};

PUBLIC DeeTypeObject DeeType_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ DeeString_STR(&str_Type),
	/* .tp_doc      = */ DOC("The so-called Type-Type, that is the Type of anything "
	                        "that is also a Type, such as :int or :list, and even itself"),
	/* .tp_flags    = */ TP_FGC | TP_FNAMEOBJECT,
	/* .tp_weakrefs = */ WEAKREF_SUPPORT_ADDR(DeeTypeObject),
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeObject_Type, /* class Type extends Object { ... } */
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (void *)&type_ctor,
				/* .tp_copy_ctor = */ NULL,
				/* .tp_deep_ctor = */ NULL,
				/* .tp_any_ctor  = */ NULL,
				TYPE_FIXED_ALLOCATOR_GC(DeeTypeObject)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&type_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ (DeeObject *(DCALL *)(DeeObject *__restrict))&type_str,
		/* .tp_repr = */ (DeeObject *(DCALL *)(DeeObject *__restrict))&type_str,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ (DeeObject *(DCALL *)(DeeObject *__restrict, size_t, DeeObject **__restrict))&DeeObject_New,
	/* .tp_visit         = */ (void(DCALL *)(DeeObject *__restrict, dvisit_t, void *))&type_visit,
	/* .tp_gc            = */ &type_gc_data,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &type_cmp_data,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ &type_attr_data,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ type_methods,
	/* .tp_getsets       = */ type_getsets,
	/* .tp_members       = */ type_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_call_kw       = */ (DeeObject *(DCALL *)(DeeObject *__restrict, size_t, DeeObject **__restrict, DeeObject*))&DeeObject_NewKw,
};



#ifdef CONFIG_TRACE_REFCHANGES
#ifndef CONFIG_NO_THREADS
PRIVATE DEFINE_RWLOCK(reftracker_lock);
#endif /* !CONFIG_NO_THREADS */
PRIVATE struct Dee_reftracker *reftracker_list = NULL;

PRIVATE dref_t DCALL
print_refchange(struct Dee_refchange *__restrict item,
                dref_t prev_total) {
	char mode[2] = { '+', 0 };
	dref_t count;
	if (item->rc_line < 0)
		--prev_total, mode[0] = '-';
	DEE_DPRINTF("%s(%d) : [%c][%Iu]", item->rc_file,
	            item->rc_line < 0 ? -item->rc_line : item->rc_line,
	            mode[0], prev_total);
	count = prev_total;
	if (count > 15)
		count = 15;
	while (count--)
		DEE_DPRINT(mode);
	DEE_DPRINT("\n");
	if (item->rc_line >= 0)
		++prev_total;
	return prev_total;
}

PRIVATE dref_t DCALL
print_refchanges(struct Dee_refchanges *__restrict item,
                 dref_t prev_total) {
	unsigned int i;
	if (item->rc_prev)
		prev_total = print_refchanges(item->rc_prev, prev_total);
	for (i = 0; i < COMPILER_LENOF(item->rc_chng); ++i) {
		if (!item->rc_chng[i].rc_file)
			break;
		prev_total = print_refchange(&item->rc_chng[i], prev_total);
	}
	return prev_total;
}

INTERN void DCALL
dump_reference_history(DeeObject *__restrict obj) {
	if (!obj->ob_trace)
		return;
	rwlock_read(&reftracker_lock);
	print_refchanges(obj->ob_trace->rt_last, 1);
	rwlock_endread(&reftracker_lock);
}

PUBLIC void DCALL Dee_DumpReferenceLeaks(void) {
	struct Dee_reftracker *iter;
	rwlock_read(&reftracker_lock);
	for (iter = reftracker_list; iter; iter = iter->rt_next) {
		DEE_DPRINTF("Object at %p of instance %s leaked %Iu references:\n",
		            iter->rt_obj, iter->rt_obj->ob_type->tp_name,
		            iter->rt_obj->ob_refcnt);
		print_refchanges(iter->rt_last, 1);
		DEE_DPRINT("\n");
	}
	rwlock_endread(&reftracker_lock);
}


PRIVATE void DCALL
add_reftracker(struct Dee_reftracker *__restrict self) {
	rwlock_write(&reftracker_lock);
	self->rt_pself = &reftracker_list;
	if ((self->rt_next = reftracker_list) != NULL)
		reftracker_list->rt_pself = &self->rt_next;
	reftracker_list = self;
	rwlock_endwrite(&reftracker_lock);
}

PRIVATE void DCALL
del_reftracker(struct Dee_reftracker *__restrict self) {
	rwlock_write(&reftracker_lock);
	if ((*self->rt_pself = self->rt_next) != NULL)
		self->rt_next->rt_pself = self->rt_pself;
	rwlock_endwrite(&reftracker_lock);
}

/* Reference count tracing. */
PRIVATE void DCALL
free_reftracker(struct Dee_reftracker *__restrict self) {
	if (self) {
		struct Dee_refchanges *iter, *next;
		del_reftracker(self);
		iter = self->rt_last;
		while (iter) {
			next = iter->rc_prev;
			if (iter != &self->rt_first)
				Dee_Free(iter);
			iter = next;
		}
		Dee_Free(self);
	}
}

#ifdef CONFIG_TRACE_REFCHANGES
PUBLIC void DCALL
DeeObject_FreeTracker(DeeObject *__restrict self) {
	free_reftracker(self->ob_trace);
}
#endif /* CONFIG_NO_BADREFCNT_CHECKS */


PRIVATE struct Dee_reftracker *DCALL
get_reftracker(DeeObject *__restrict self) {
	struct Dee_reftracker *result, *new_result;
	result = self->ob_trace;
	if
		likely(result)
	goto done;

	/* Allocate a new reference tracker. */
	result = (struct Dee_reftracker *)Dee_TryCalloc(sizeof(struct Dee_reftracker));
	if (!result)
		goto done;
	COMPILER_READ_BARRIER();
	result->rt_obj  = self;
	result->rt_last = &result->rt_first;
	COMPILER_WRITE_BARRIER();
	/* Setup the tracker for use by this object. */
	new_result = ATOMIC_CMPXCH_VAL(self->ob_trace, NULL, result);
	if
		unlikely(new_result != NULL)
	{
		/* Race condition... */
		Dee_Free(result);
		result = new_result;
		goto done;
	}
	/* Keep track of this tracker... */
	add_reftracker(result);
done:
	return result;
}

PRIVATE void DCALL
reftracker_addchange(DeeObject *__restrict ob,
                     char const *file, int line) {
	unsigned int i;
	struct Dee_reftracker *self;
	struct Dee_refchanges *new_changes;
	struct Dee_refchanges *last;
	self = get_reftracker(ob);
	if
		unlikely(!self || self == DEE_REFTRACKER_UNTRACKED)
	return;
again:
	last = self->rt_last;
	for (i = 0; i < COMPILER_LENOF(last->rc_chng); ++i) {
		if (!ATOMIC_CMPXCH(last->rc_chng[i].rc_file, NULL, file))
			continue;
		last->rc_chng[i].rc_line = line;
		return; /* Got it! */
	}
	/* Must allocate a new set of changes. */
	new_changes = (struct Dee_refchanges *)Dee_TryCalloc(sizeof(struct Dee_refchanges));
	if
		unlikely(!new_changes)
	return;
	new_changes->rc_chng[0].rc_file = file;
	new_changes->rc_chng[0].rc_line = line;
	new_changes->rc_prev            = last;
	/* Save the new set of changes as the latest set active. */
	if (!ATOMIC_CMPXCH(self->rt_last, last, new_changes)) {
		Dee_Free(new_changes);
		goto again;
	}
}


PUBLIC void DCALL
Dee_Incref_traced(DeeObject *__restrict ob,
                  char const *file, int line) {
#ifndef CONFIG_NO_BADREFCNT_CHECKS
	if (ATOMIC_FETCHINC(ob->ob_refcnt) == 0)
		DeeFatal_BadIncref(ob, file, line);
#else /* !CONFIG_NO_BADREFCNT_CHECKS */
	ATOMIC_FETCHINC(ob->ob_refcnt);
#endif /* CONFIG_NO_BADREFCNT_CHECKS */
	reftracker_addchange(ob, file, line);
}

PUBLIC void DCALL
Dee_Incref_n_traced(DeeObject *__restrict ob, dref_t n,
                    char const *file, int line) {
#ifndef CONFIG_NO_BADREFCNT_CHECKS
	if (ATOMIC_FETCHADD(ob->ob_refcnt, n) == 0)
		DeeFatal_BadIncref(ob, file, line);
#else /* !CONFIG_NO_BADREFCNT_CHECKS */
	ATOMIC_FETCHADD(ob->ob_refcnt, n);
#endif /* CONFIG_NO_BADREFCNT_CHECKS */
	while (n--)
		reftracker_addchange(ob, file, line);
}

PUBLIC bool DCALL
Dee_IncrefIfNotZero_traced(DeeObject *__restrict ob, char const *file, int line) {
	dref_t oldref;
	do
		if ((oldref = ATOMIC_READ(ob->ob_refcnt)) == 0)
			return false;
	while (!ATOMIC_CMPXCH_WEAK(ob->ob_refcnt, oldref, oldref + 1));
	reftracker_addchange(ob, file, line);
	return true;
}

PUBLIC void DCALL
Dee_Decref_traced(DeeObject *__restrict ob, char const *file, int line) {
	dref_t newref;
	newref = ATOMIC_FETCHDEC(ob->ob_refcnt);
#ifndef CONFIG_NO_BADREFCNT_CHECKS
	if
		unlikely(newref == 0)
	DeeFatal_BadDecref(ob, file, line);
#endif /* !CONFIG_NO_BADREFCNT_CHECKS */
	if
		unlikely(newref == 1)
	DeeObject_Destroy_d(ob, file, line);
	else {
		reftracker_addchange(ob, file, -line);
	}
}

PUBLIC void DCALL
Dee_DecrefDokill_traced(DeeObject *__restrict ob, char const *file, int line) {
#ifndef CONFIG_NO_BADREFCNT_CHECKS
	if (ATOMIC_FETCHDEC(ob->ob_refcnt) != 1)
		DeeFatal_BadDecref(ob, file, line);
#else /* !CONFIG_NO_BADREFCNT_CHECKS */
	/* Without `CONFIG_NO_BADREFCNT_CHECKS', DeeObject_Destroy doesn't
	 * care about the final reference count, so no need for us to change it. */
#endif /* CONFIG_NO_BADREFCNT_CHECKS */
	DeeObject_Destroy_d(ob, file, line);
}

PUBLIC void DCALL
Dee_DecrefNokill_traced(DeeObject *__restrict ob, char const *file, int line) {
#ifndef CONFIG_NO_BADREFCNT_CHECKS
	if (ATOMIC_FETCHDEC(ob->ob_refcnt) <= 1)
		DeeFatal_BadDecref(ob, file, line);
#else /* !CONFIG_NO_BADREFCNT_CHECKS */
	ATOMIC_FETCHDEC(ob->ob_refcnt);
#endif /* CONFIG_NO_BADREFCNT_CHECKS */
	reftracker_addchange(ob, file, -line);
}

PUBLIC bool DCALL
Dee_DecrefIfOne_traced(DeeObject *__restrict ob, char const *file, int line) {
	if (!ATOMIC_CMPXCH(ob->ob_refcnt, 1, 0))
		return false;
	DeeObject_Destroy_d(ob, file, line);
	return true;
}

PUBLIC bool DCALL
Dee_DecrefIfNotOne_traced(DeeObject *__restrict ob, char const *file, int line) {
	dref_t oldref;
	do {
		if ((oldref = ATOMIC_READ(ob->ob_refcnt)) <= 1)
			return false;
	} while (!ATOMIC_CMPXCH_WEAK(ob->ob_refcnt, oldref, oldref - 1));
	reftracker_addchange(ob, file, -line);
	return true;
}

PUBLIC bool DCALL
Dee_DecrefWasOk_traced(DeeObject *__restrict ob,
                       char const *file, int line) {
	dref_t newref;
	newref = ATOMIC_FETCHDEC(ob->ob_refcnt);
#ifndef CONFIG_NO_BADREFCNT_CHECKS
	if
		unlikely(newref == 0)
	DeeFatal_BadDecref(ob, file, line);
#endif /* !CONFIG_NO_BADREFCNT_CHECKS */
	if
		unlikely(newref == 1)
	{
		DeeObject_Destroy_d(ob, file, line);
		return true;
	}
	reftracker_addchange(ob, file, -line);
	return false;
}

#else  /* CONFIG_TRACE_REFCHANGES */

PUBLIC void (DCALL Dee_Incref_traced)(DeeObject *__restrict ob,
                                      char const *UNUSED(file),
                                      int UNUSED(line)) {
	Dee_Incref(ob);
}

PUBLIC void (DCALL Dee_Incref_n_traced)(DeeObject *__restrict ob, dref_t n,
                                        char const *UNUSED(file),
                                        int UNUSED(line)) {
	Dee_Incref_n(ob, n);
}

PUBLIC bool (DCALL Dee_IncrefIfNotZero_traced)(DeeObject *__restrict ob,
                                               char const *UNUSED(file),
                                               int UNUSED(line)) {
	return Dee_IncrefIfNotZero(ob);
}

PUBLIC void (DCALL Dee_Decref_traced)(DeeObject *__restrict ob,
                                      char const *UNUSED(file),
                                      int UNUSED(line)) {
	Dee_Decref(ob);
}

PUBLIC void (DCALL Dee_DecrefDokill_traced)(DeeObject *__restrict ob,
                                            char const *UNUSED(file),
                                            int UNUSED(line)) {
	Dee_DecrefDokill(ob);
}

PUBLIC void (DCALL Dee_DecrefNokill_traced)(DeeObject *__restrict ob,
                                            char const *UNUSED(file),
                                            int UNUSED(line)) {
	Dee_DecrefNokill(ob);
}

PUBLIC bool (DCALL Dee_DecrefIfOne_traced)(DeeObject *__restrict ob,
                                           char const *UNUSED(file),
                                           int UNUSED(line)) {
	return Dee_DecrefIfOne(ob);
}

PUBLIC bool (DCALL Dee_DecrefIfNotOne_traced)(DeeObject *__restrict ob,
                                              char const *UNUSED(file),
                                              int UNUSED(line)) {
	return Dee_DecrefIfNotOne(ob);
}

PUBLIC bool (DCALL Dee_DecrefWasOk_traced)(DeeObject *__restrict ob,
                                           char const *UNUSED(file),
                                           int UNUSED(line)) {
	return Dee_DecrefWasOk(ob);
}
DFUNDEF void(DCALL Dee_DumpReferenceLeaks)(void);
PUBLIC void(DCALL Dee_DumpReferenceLeaks)(void) {
}
#endif /* !CONFIG_TRACE_REFCHANGES */

/* Also export all the reference-control macros as functions. */
DFUNDEF void(DCALL Dee_Incref)(DeeObject *__restrict ob);
DFUNDEF void(DCALL Dee_Incref_n)(DeeObject *__restrict ob, dref_t n);
DFUNDEF bool(DCALL Dee_IncrefIfNotZero)(DeeObject *__restrict ob);
DFUNDEF void(DCALL Dee_Decref)(DeeObject *__restrict ob);
DFUNDEF void(DCALL Dee_DecrefDokill)(DeeObject *__restrict ob);
DFUNDEF void(DCALL Dee_DecrefNokill)(DeeObject *__restrict ob);
DFUNDEF bool(DCALL Dee_DecrefIfOne)(DeeObject *__restrict ob);
DFUNDEF bool(DCALL Dee_DecrefIfNotOne)(DeeObject *__restrict ob);
DFUNDEF bool(DCALL Dee_DecrefWasOk)(DeeObject *__restrict ob);

PUBLIC void(DCALL Dee_Incref)(DeeObject *__restrict ob) {
	Dee_Incref_untraced(ob);
}

PUBLIC void(DCALL Dee_Incref_n)(DeeObject *__restrict ob, dref_t n) {
	Dee_Incref_n_untraced(ob, n);
}

PUBLIC bool(DCALL Dee_IncrefIfNotZero)(DeeObject *__restrict ob) {
	return Dee_IncrefIfNotZero_untraced(ob);
}

PUBLIC void(DCALL Dee_Decref)(DeeObject *__restrict ob) {
	Dee_Decref_untraced(ob);
}

PUBLIC void(DCALL Dee_DecrefDokill)(DeeObject *__restrict ob) {
	Dee_DecrefDokill_untraced(ob);
}

PUBLIC void(DCALL Dee_DecrefNokill)(DeeObject *__restrict ob) {
	Dee_DecrefNokill_untraced(ob);
}

PUBLIC bool(DCALL Dee_DecrefIfOne)(DeeObject *__restrict ob) {
	return Dee_DecrefIfOne_untraced(ob);
}

PUBLIC bool(DCALL Dee_DecrefIfNotOne)(DeeObject *__restrict ob) {
	return Dee_DecrefIfNotOne_untraced(ob);
}

PUBLIC bool(DCALL Dee_DecrefWasOk)(DeeObject *__restrict ob) {
	return Dee_DecrefWasOk_untraced(ob);
}


#ifndef NDEBUG
PRIVATE void DCALL
assert_badobject_impl(char const *check_name,
                      char const *file,
                      int line, DeeObject *ob) {
	if (!ob) {
		_DeeAssert_Failf(check_name, file, line,
		                 "Bad object at %p is a NULL pointer",
		                 ob);
	} else if (!ob->ob_refcnt) {
		char const *type_name = "?";
		if (DeeObject_Check(ob->ob_type))
			type_name = ob->ob_type->tp_name;
		_DeeAssert_Failf(check_name, file, line,
		                 "Bad object at %p (instance of %s) has a reference count of 0",
		                 ob, type_name);
	} else if (!ob->ob_type) {
		_DeeAssert_Failf(check_name, file, line,
		                 "Bad object at %p (%Iu references) has a NULL-pointer as type",
		                 ob, ob->ob_refcnt);
	} else if (!ob->ob_type->ob_refcnt) {
		_DeeAssert_Failf(check_name, file, line,
		                 "Bad object at %p (instance of %s, %Iu references) has a type with a reference counter of 0",
		                 ob, ob->ob_type->tp_name, ob->ob_refcnt);
	} else {
		char const *type_name = "?";
		if (DeeObject_Check(ob->ob_type))
			type_name = ob->ob_type->tp_name;
		_DeeAssert_Failf(check_name, file, line,
		                 "Bad object at %p (instance of %s, %Iu references)",
		                 ob, type_name, ob->ob_refcnt);
	}
}

PRIVATE void DCALL
assert_badtype_impl(char const *check_name, char const *file,
                    int line, DeeObject *ob, bool wanted_exact,
                    DeeTypeObject *__restrict wanted_type) {
	char const *is_exact = wanted_exact ? " an exact " : " an ";
	if (!ob) {
		_DeeAssert_Failf(check_name, file, line,
		                 "Bad object at %p is a NULL pointer when%sinstance of %s was needed",
		                 ob, is_exact, wanted_type->tp_name);
	} else if (!ob->ob_refcnt) {
		char const *type_name = "?";
		if (DeeObject_Check(ob->ob_type))
			type_name = ob->ob_type->tp_name;
		_DeeAssert_Failf(check_name, file, line,
		                 "Bad object at %p (instance of %s) has a reference "
		                 "count of 0 when%sinstance of %s was needed",
		                 ob, type_name, is_exact, wanted_type->tp_name);
	} else if (!ob->ob_type) {
		_DeeAssert_Failf(check_name, file, line,
		                 "Bad object at %p (%Iu references) has a NULL-pointer "
		                 "as type when%sinstance of %s was needed",
		                 ob, ob->ob_refcnt, is_exact, wanted_type->tp_name);
	} else if (!ob->ob_type->ob_refcnt) {
		_DeeAssert_Failf(check_name, file, line,
		                 "Bad object at %p (instance of %s, %Iu references) has a type "
		                 "with a reference counter of 0 when%sinstance of %s was needed",
		                 ob, ob->ob_type->tp_name, ob->ob_refcnt, is_exact, wanted_type->tp_name);
	} else {
		bool include_obj_repr = false;
		char const *type_name = "?";
		if (DeeObject_Check(ob->ob_type)) {
			type_name = ob->ob_type->tp_name;
			if (ob->ob_type == &DeeString_Type || ob->ob_type == &DeeInt_Type ||
			    ob->ob_type == &DeeBool_Type || ob->ob_type == &DeeFloat_Type)
				include_obj_repr = true;
		}
		if (include_obj_repr) {
			_DeeAssert_Failf(check_name, file, line,
			                 "Bad object at %p (instance of %s, %Iu references) "
			                 "when%sinstance of %s was needed\n"
			                 "repr: %r",
			                 ob, type_name, ob->ob_refcnt, is_exact,
			                 wanted_type->tp_name, ob);
		} else {
			_DeeAssert_Failf(check_name, file, line,
			                 "Bad object at %p (instance of %s, %Iu references) "
			                 "when%sinstance of %s was needed",
			                 ob, type_name, ob->ob_refcnt, is_exact, wanted_type->tp_name);
		}
	}
}

PUBLIC void DCALL
DeeAssert_BadObject(char const *file, int line, DeeObject *ob) {
	assert_badobject_impl("ASSERT_OBJECT", file, line, ob);
}

PUBLIC void DCALL
DeeAssert_BadObjectOpt(char const *file, int line, DeeObject *ob) {
	assert_badobject_impl("ASSERT_OBJECT_OPT", file, line, ob);
}

PUBLIC void DCALL
DeeAssert_BadObjectType(char const *file, int line, DeeObject *ob,
                        DeeTypeObject *__restrict wanted_type) {
	assert_badtype_impl("ASSERT_OBJECT_TYPE", file, line, ob, false, wanted_type);
}

PUBLIC void DCALL
DeeAssert_BadObjectTypeOpt(char const *file, int line, DeeObject *ob,
                           DeeTypeObject *__restrict wanted_type) {
	assert_badtype_impl("ASSERT_OBJECT_TYPE_OPT", file, line, ob, false, wanted_type);
}

PUBLIC void DCALL
DeeAssert_BadObjectTypeExact(char const *file, int line, DeeObject *ob,
                             DeeTypeObject *__restrict wanted_type) {
	assert_badtype_impl("ASSERT_OBJECT_TYPE_EXACT", file, line, ob, true, wanted_type);
}

PUBLIC void DCALL
DeeAssert_BadObjectTypeExactOpt(char const *file, int line, DeeObject *ob,
                                DeeTypeObject *__restrict wanted_type) {
	assert_badtype_impl("ASSERT_OBJECT_TYPE_EXACT_OPT", file, line, ob, true, wanted_type);
}

#else /* !NDEBUG */

PUBLIC void DCALL
DeeAssert_BadObject(char const *UNUSED(file),
                    int UNUSED(line),
                    DeeObject *UNUSED(ob)) {
	/* no-op */
}

PUBLIC void DCALL
DeeAssert_BadObjectOpt(char const *UNUSED(file),
                       int UNUSED(line),
                       DeeObject *UNUSED(ob)) {
	/* no-op */
}

PUBLIC void DCALL
DeeAssert_BadObjectType(char const *UNUSED(file),
                        int UNUSED(line),
                        DeeObject *UNUSED(ob),
                        DeeTypeObject *__restrict UNUSED(wanted_type)) {
	/* no-op */
}

PUBLIC void DCALL
DeeAssert_BadObjectTypeOpt(char const *UNUSED(file),
                           int UNUSED(line),
                           DeeObject *UNUSED(ob),
                           DeeTypeObject *__restrict UNUSED(wanted_type)) {
	/* no-op */
}

PUBLIC void DCALL
DeeAssert_BadObjectTypeExact(char const *UNUSED(file),
                             int UNUSED(line),
                             DeeObject *UNUSED(ob),
                             DeeTypeObject *__restrict UNUSED(wanted_type)) {
	/* no-op */
}

PUBLIC void DCALL
DeeAssert_BadObjectTypeExactOpt(char const *UNUSED(file),
                                int UNUSED(line),
                                DeeObject *UNUSED(ob),
                                DeeTypeObject *__restrict UNUSED(wanted_type)) {
	/* no-op */
}

#endif /* NDEBUG */

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_OBJECT_C */
