/* Copyright (c) 2018-2024 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2024 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_OBJECTS_OBJECT_C
#define GUARD_DEEMON_OBJECTS_OBJECT_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/attribute.h>
#include <deemon/bool.h>
#include <deemon/callable.h>
#include <deemon/class.h>
#include <deemon/code.h>
#include <deemon/error.h>
#include <deemon/file.h>
#include <deemon/float.h>
#include <deemon/format.h>
#include <deemon/gc.h>
#include <deemon/int.h>
#include <deemon/kwds.h>
#include <deemon/map.h>
#include <deemon/module.h>
#include <deemon/mro.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/objmethod.h>
#include <deemon/seq.h>
#include <deemon/set.h>
#include <deemon/string.h>
#include <deemon/super.h>
#include <deemon/system-features.h> /* bzero(), ... */
#include <deemon/tuple.h>
#include <deemon/util/atomic.h>
#include <deemon/util/lock.h>

#include <hybrid/align.h>
#include <hybrid/sched/yield.h>
#include <hybrid/typecore.h>

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "../runtime/kwlist.h"
#include "../runtime/runtime_error.h"
#include "../runtime/strings.h"
#include "seq/default-api.h"
#include "seq/typemro.h"
/**/

#include "type-operators.h"

DECL_BEGIN

#ifdef CONFIG_TRACE_REFCHANGES
PRIVATE void DCALL free_reftracker(struct Dee_reftracker *__restrict self);
#endif /* CONFIG_TRACE_REFCHANGES */

#ifndef CONFIG_HAVE_memsetp
#define memsetp(dst, pointer, num_pointers) \
	dee_memsetp(dst, (__UINTPTR_TYPE__)(pointer), num_pointers)
DeeSystem_DEFINE_memsetp(dee_memsetp)
#endif // !CONFIG_HAVE_memsetp


typedef DeeTypeObject Type;

/* Assert the typing of an object (raising an `Error.TypeError' if the type wasn't expected)
 * HINT: When `required_type' isn't a type-object, these functions throw an error!
 * @return: -1: The object doesn't match the required typing.
 * @return:  0: The object matches the required typing. */
PUBLIC WUNUSED NONNULL((1, 2)) int
(DCALL DeeObject_AssertType)(DeeObject *self, DeeTypeObject *required_type) {
	if likely(DeeObject_InstanceOf(self, required_type))
		return 0;
	return DeeObject_TypeAssertFailed(self, required_type);
}

PUBLIC WUNUSED NONNULL((1, 2)) int
(DCALL DeeObject_AssertTypeOrAbstract)(DeeObject *self, DeeTypeObject *required_type) {
	if likely(DeeType_IsAbstract(required_type))
		return 0;
	if likely(DeeObject_InstanceOf(self, required_type))
		return 0;
	return DeeObject_TypeAssertFailed(self, required_type);
}

PUBLIC WUNUSED NONNULL((1, 2)) int
(DCALL DeeObject_AssertImplements)(DeeObject *self, DeeTypeObject *required_type) {
	if likely(DeeObject_Implements(self, required_type))
		return 0;
	return DeeObject_TypeAssertFailed(self, required_type);
}

PUBLIC WUNUSED NONNULL((1, 2)) int
(DCALL DeeObject_AssertTypeExact)(DeeObject *self, DeeTypeObject *required_type) {
	if likely(DeeObject_InstanceOfExact(self, required_type))
		return 0;
	return DeeObject_TypeAssertFailed(self, required_type);
}


/* Returns the class of `self', automatically
 * dereferencing super-objects and other wrappers.
 * Beyond that, this function returns the same as `Dee_TYPE()' */
PUBLIC WUNUSED ATTR_RETNONNULL NONNULL((1)) DeeTypeObject *DCALL
DeeObject_Class(DeeObject *__restrict self) {
	DeeTypeObject *result;
	ASSERT_OBJECT(self);
	result = Dee_TYPE(self);
	if (result == &DeeSuper_Type)
		result = DeeSuper_TYPE(self);
	return result;
}


/* Return true if `test_type' is equal to, or extends `extended_type'
 * NOTE: When `extended_type' is not a type, this function simply returns `false'
 * >> return test_type.extends(extended_type);
 *
 * HINT: Always returns either `0' or `1'!
 * @return: 0 : "test_type" does not inherit from `extended_type', or `extended_type' isn't a type
 * @return: 1 : "test_type" does inherit from `extended_type' */
PUBLIC WUNUSED NONNULL((1)) unsigned int DCALL
DeeType_Extends(DeeTypeObject const *test_type,
                DeeTypeObject const *extended_type) {
	do {
		if (test_type == extended_type)
			return 1;
	} while ((test_type = DeeType_Base(test_type)) != NULL);
	return 0;
}

/* Same as `DeeType_Extends()', but also check `tp_mro' for matches.
 * This function should be used when `implemented_type' is an abstract type.
 * >> return test_type.implements(implemented_type);
 *
 * HINT: Always returns either `0' or `1'!
 * @return: 0 : "test_type" does not implement "implemented_type"
 * @return: 1 : "test_type" does implement "implemented_type" */
PUBLIC WUNUSED NONNULL((1)) unsigned int DCALL
DeeType_Implements(DeeTypeObject const *test_type,
                   DeeTypeObject const *implemented_type) {
	DeeTypeMRO mro;
	test_type = DeeTypeMRO_Init(&mro, test_type);
	do {
		if (test_type == implemented_type)
			return 1;
	} while ((test_type = DeeTypeMRO_Next(&mro, test_type)) != NULL);
	return 0;
}



/* Inheriting the weakref support address works, however
 * it slows down object destruction more than it speeds
 * up weakref usage, so we don't actually use it!. */
#undef CONFIG_INHERIT_WEAKREF_SUPPORT_ADDRESS
#if 0
#define CONFIG_INHERIT_WEAKREF_SUPPORT_ADDRESS
#endif

#ifdef CONFIG_INHERIT_WEAKREF_SUPPORT_ADDRESS
#define has_noninherited_weakrefs(tp) \
	((tp)->tp_weakrefs != 0 && (!(tp)->tp_base || (tp)->tp_base->tp_weakrefs != (tp)->tp_weakrefs))
#else /* CONFIG_INHERIT_WEAKREF_SUPPORT_ADDRESS */
#define has_noninherited_weakrefs(tp) \
	((tp)->tp_weakrefs != 0)
#endif /* !CONFIG_INHERIT_WEAKREF_SUPPORT_ADDRESS */


#ifdef CONFIG_INHERIT_WEAKREF_SUPPORT_ADDRESS
PRIVATE NONNULL((1)) bool DCALL
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
LOCAL WUNUSED NONNULL((1)) struct weakref_list *DCALL
weakrefs_get(DeeObject *__restrict ob) {
	DeeTypeObject *tp = Dee_TYPE(ob);
#ifdef CONFIG_INHERIT_WEAKREF_SUPPORT_ADDRESS
	if unlikely(!tp->tp_weakrefs)
		type_inherit_weakrefs(tp);
#else /* CONFIG_INHERIT_WEAKREF_SUPPORT_ADDRESS */
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

#define GET_POINTER(x)     ((uintptr_t)(x)&PTRLOCK_ADDR_MASK)
#define LOCK_POINTER(x)    ptrlock_lock((void **)&(x))
#define TRYLOCK_POINTER(x) ptrlock_trylock((void **)&(x))
#define UNLOCK_POINTER(x)  ptrlock_unlock((void **)&(x))
#define WAITFOR_POINTER(x) ptrlock_waitfor((void **)&(x))
#define WEAKREF_LOCK(x)    LOCK_POINTER((x)->wr_next)
#define WEAKREF_TRYLOCK(x) TRYLOCK_POINTER((x)->wr_next)
#define WEAKREF_UNLOCK(x)  UNLOCK_POINTER((x)->wr_next)
#define WEAKREF_WAITFOR(x) WAITFOR_POINTER((x)->wr_next)

#define WEAKREF_PREV_TRYLOCK(x) TRYLOCK_POINTER(*(x)->wr_pself)
#define WEAKREF_PREV_UNLOCK(x)  atomic_write((x)->wr_pself, (x))

LOCAL NONNULL((1)) bool DCALL ptrlock_trylock(void **self) {
	uintptr_t lold;
	do {
		lold = atomic_read((uintptr_t *)self);
		if (lold & PTRLOCK_LOCK_MASK)
			return false;
	} while (!atomic_cmpxch_weak((uintptr_t *)self, lold, lold | PTRLOCK_LOCK_MASK));
	return true;
}

LOCAL NONNULL((1)) void DCALL ptrlock_lock(void **self) {
	uintptr_t lold;
again:
	do {
		lold = atomic_read((uintptr_t *)self);
		/* Wait while the lock already is in write-mode or has readers. */
		if (lold & PTRLOCK_LOCK_MASK) {
			SCHED_YIELD();
			goto again;
		}
	} while (!atomic_cmpxch_weak((uintptr_t *)self, lold, lold | PTRLOCK_LOCK_MASK));
}

LOCAL NONNULL((1)) void DCALL ptrlock_waitfor(void **self) {
	/* Wait while the lock already is in write-mode or has readers. */
	for (;;) {
		uintptr_t lold;
		lold = atomic_read((uintptr_t *)self);
		if (!(lold & PTRLOCK_LOCK_MASK))
			break;
		SCHED_YIELD();
	}
}

#ifdef NDEBUG
#define ptrlock_unlock(self) atomic_and((uintptr_t *)(self), ~(PTRLOCK_LOCK_MASK));
#else /* NDEBUG */
#define ptrlock_unlock(self)                                                            \
	do {                                                                                \
		uintptr_t _pl_old = atomic_fetchand((uintptr_t *)(self), ~(PTRLOCK_LOCK_MASK)); \
		ASSERTF((_pl_old & PTRLOCK_LOCK_MASK) != 0, "Pointer was not locked.");         \
	}	__WHILE0
#endif /* !NDEBUG */

#if __SIZEOF_POINTER__ == 4
#define WEAKREF_BAD_POINTER UINT32_C(0xcccccccc)
#elif __SIZEOF_POINTER__ == 8
#define WEAKREF_BAD_POINTER UINT64_C(0xcccccccccccccccc)
#else /* __SIZEOF_POINTER__ == ... */
#define WEAKREF_BAD_POINTER (-1)
#endif /* __SIZEOF_POINTER__ != ... */

#ifdef NDEBUG
#define WEAKREF_EMPTY_NEXTVAL NULL
#define WEAKREF_SETBAD(x, T)  (void)0
#else /* NDEBUG */
#define WEAKREF_EMPTY_NEXTVAL (struct weakref *)((uintptr_t)WEAKREF_BAD_POINTER & PTRLOCK_ADDR_MASK)
#define WEAKREF_SETBAD(x, T)  ((x) = (T)WEAKREF_BAD_POINTER)
#endif /* !NDEBUG */


#ifdef __INTELLISENSE__
PUBLIC NONNULL((1, 2)) bool
(DCALL Dee_weakref_init)(struct weakref *__restrict self,
                         DeeObject *__restrict ob,
                         Dee_weakref_callback_t callback)
#else /* __INTELLISENSE__ */
PUBLIC NONNULL((1, 2)) bool
(DCALL Dee_weakref_init)(struct weakref *__restrict self,
                         DeeObject *__restrict ob)
#endif /* !__INTELLISENSE__ */
{
	struct weakref_list *list;
	struct weakref *next;
	ASSERT_OBJECT(ob);
	ASSERT(IS_ALIGNED((uintptr_t)self, PTRLOCK_LOCK_MASK + 1));
#ifdef __INTELLISENSE__
	self->wr_del = callback;
#endif /* __INTELLISENSE__ */
again:
	list = WEAKREFS_GET(ob);
	if unlikely(!WEAKREFS_OK(list, ob))
		return false;
	LOCK_POINTER(list->wl_nodes);
	next = (struct weakref *)GET_POINTER(list->wl_nodes);
	self->wr_obj   = ob;
	self->wr_pself = &list->wl_nodes;
	self->wr_next  = next;
	if (next) {
		if unlikely(!WEAKREF_TRYLOCK(next)) {
			UNLOCK_POINTER(list->wl_nodes); /* WEAKREF_UNLOCK(list) */
			SCHED_YIELD();
			goto again;
		}
		ASSERT(next->wr_obj == ob);
		ASSERT(next->wr_pself == &list->wl_nodes);
		next->wr_pself = &self->wr_next;
		WEAKREF_UNLOCK(next);             /* WEAKREF_UNLOCK(next) */
	}
	atomic_write(&list->wl_nodes, self);  /* WEAKREF_UNLOCK(list) */
	return true;
}

#ifdef __INTELLISENSE__
PUBLIC NONNULL((1, 2)) void
(DCALL Dee_weakref_copy)(struct weakref *__restrict self,
                         struct weakref const *__restrict other)
#else /* __INTELLISENSE__ */
PUBLIC NONNULL((1, 2)) void
(DCALL Dee_weakref_copy)(struct weakref *__restrict self,
                         struct weakref *__restrict other)
#endif /* !__INTELLISENSE__ */
{
	ASSERT(self != other);
#ifndef NDEBUG
	ASSERT(other->wr_obj != (DeeObject *)WEAKREF_BAD_POINTER);
#endif /* !NDEBUG */
	WEAKREF_SETBAD(self->wr_pself, struct weakref **);
	self->wr_del  = other->wr_del;
again:
	if (other->wr_obj) {
		WEAKREF_LOCK(other);
		COMPILER_READ_BARRIER();
		if likely(other->wr_obj) {
			if unlikely(!WEAKREF_PREV_TRYLOCK(other)) {
				WEAKREF_UNLOCK(other);
				SCHED_YIELD();
				goto again;
			}
			self->wr_pself = other->wr_pself;
			self->wr_next  = (struct weakref *)other;
			self->wr_obj   = other->wr_obj;
			((struct weakref *)other)->wr_pself = &self->wr_next;
			WEAKREF_UNLOCK(other);     /* WEAKREF_UNLOCK(other); */
			WEAKREF_PREV_UNLOCK(self); /* WEAKREF_UNLOCK(other->PREV); */
		} else {
			ASSERT(other->wr_next == (struct Dee_weakref *)PTRLOCK_LOCK_MASK);
			atomic_write(&((struct weakref *)other)->wr_next, WEAKREF_EMPTY_NEXTVAL); /* WEAKREF_UNLOCK(other); */
			goto set_dst_empty;
		}
	} else {
set_dst_empty:
		self->wr_obj  = NULL;
		self->wr_next = WEAKREF_EMPTY_NEXTVAL;
	}
}

PUBLIC NONNULL((1, 2)) void DCALL
Dee_weakref_move(struct weakref *__restrict dst,
                 struct weakref *__restrict src) {
	ASSERT(dst != src);
#ifndef NDEBUG
	ASSERT(src->wr_obj != (DeeObject *)WEAKREF_BAD_POINTER);
#endif /* !NDEBUG */
	dst->wr_del = src->wr_del;
again:
	WEAKREF_SETBAD(dst->wr_pself, struct weakref **);
	if (src->wr_obj) {
		WEAKREF_LOCK(src);
		COMPILER_READ_BARRIER();
		if likely(src->wr_obj) {
			struct weakref *next;
			if unlikely(TRYLOCK_POINTER(*src->wr_pself)) {
				/* Prevent a deadlock. */
				WEAKREF_UNLOCK(src);
				SCHED_YIELD();
				goto again;
			}
			next          = (struct weakref *)GET_POINTER(src->wr_next);
			dst->wr_pself = src->wr_pself;
			dst->wr_next  = next;
			dst->wr_obj   = src->wr_obj;
			if (next) {
				if unlikely(!WEAKREF_TRYLOCK(next)) {
					/* Prevent a deadlock. */
					WEAKREF_UNLOCK(*src->wr_pself);
					WEAKREF_UNLOCK(src);
					SCHED_YIELD();
					goto again;
				}
				next->wr_pself = &dst->wr_next;
				WEAKREF_UNLOCK(next);         /* WEAKREF_UNLOCK(src->NEXT) */
			}
			atomic_write(dst->wr_pself, dst); /* WEAKREF_UNLOCK(src->PREV) */
			WEAKREF_UNLOCK(src);              /* WEAKREF_UNLOCK(src) */
		} else {
			WEAKREF_UNLOCK(src);
			goto set_dst_empty;
		}
	} else {
set_dst_empty:
		dst->wr_obj  = NULL;
		dst->wr_next = WEAKREF_EMPTY_NEXTVAL;
	}
}

#ifdef __INTELLISENSE__
PUBLIC NONNULL((1, 2)) void
(DCALL Dee_weakref_copyassign)(struct weakref *self,
                               struct weakref const *other)
#else /* __INTELLISENSE__ */
PUBLIC NONNULL((1, 2)) void
(DCALL Dee_weakref_copyassign)(struct weakref *self,
                               struct weakref *other)
#endif /* !__INTELLISENSE__ */
{
#ifndef NDEBUG
	ASSERT(self->wr_obj != (DeeObject *)WEAKREF_BAD_POINTER);
	ASSERT(other->wr_obj != (DeeObject *)WEAKREF_BAD_POINTER);
#endif /* !NDEBUG */
	if unlikely(self == other)
		return;
again:
	if (other->wr_obj) {
		WEAKREF_LOCK(other);
#define LOCAL_UNLOCK_other() WEAKREF_UNLOCK(other)
		COMPILER_READ_BARRIER();
		if likely(other->wr_obj) {
#define LOCAL_UNLOCK_other__prev() atomic_write(other->wr_pself, other)
			if unlikely(!WEAKREF_PREV_TRYLOCK(other)) {
				LOCAL_UNLOCK_other();
				SCHED_YIELD();
				goto again;
			}
#define LOCAL_UNLOCK_self() WEAKREF_UNLOCK(self)
			if unlikely(!WEAKREF_TRYLOCK(self)) {
				struct weakref **other_pself = other->wr_pself;
				LOCAL_UNLOCK_other__prev();
				LOCAL_UNLOCK_other();
				if (other_pself == &self->wr_next)
					return; /* Special case: `self' is the predecessor of `other' */
				WEAKREF_WAITFOR(self);
				goto again;
			}
			COMPILER_READ_BARRIER();
			if (self->wr_obj) {
				struct weakref *old_self_next;
				if unlikely(self->wr_obj == other->wr_obj) {
					/* Special case: both point to the same object -> must clear `other' */
					LOCAL_UNLOCK_self();
					LOCAL_UNLOCK_other__prev();
					LOCAL_UNLOCK_other();
					return;
				}

#define LOCAL_UNLOCK_self__prev() WEAKREF_PREV_UNLOCK(self)
				if unlikely(!WEAKREF_PREV_TRYLOCK(self)) {
					LOCAL_UNLOCK_self();
					LOCAL_UNLOCK_other__prev();
					LOCAL_UNLOCK_other();
					SCHED_YIELD();
					goto again;
				}
				old_self_next = (struct weakref *)GET_POINTER(self->wr_next);
				if (old_self_next) {
					if unlikely(!WEAKREF_TRYLOCK(old_self_next)) {
						/* Prevent a deadlock. */
						LOCAL_UNLOCK_self__prev();
						LOCAL_UNLOCK_self();
						LOCAL_UNLOCK_other__prev();
						LOCAL_UNLOCK_other();
						if unlikely(old_self_next == other)
							return;
						SCHED_YIELD();
						goto again;
					}
					old_self_next->wr_pself = self->wr_pself;
					WEAKREF_UNLOCK(old_self_next); /* LOCAL_UNLOCK_self__self(); */
				}
				atomic_write(self->wr_pself, old_self_next); /* LOCAL_UNLOCK_self__prev(); */
#undef LOCAL_UNLOCK_self__prev
			}

			/* Insert `self' before `other' */
			self->wr_pself = other->wr_pself;
			self->wr_obj   = other->wr_obj;
			((struct weakref *)other)->wr_pself = &self->wr_next;
			WEAKREF_PREV_UNLOCK(self);           /* LOCAL_UNLOCK_other__prev(); */
			LOCAL_UNLOCK_other();
			atomic_write(&self->wr_next, other); /* LOCAL_UNLOCK_self(); */
#undef LOCAL_UNLOCK_self
#undef LOCAL_UNLOCK_other__prev
		} else {
			atomic_write(&((struct weakref *)other)->wr_next, WEAKREF_EMPTY_NEXTVAL); /* LOCAL_UNLOCK_other(); */
			Dee_weakref_clear(self);
		}
#undef LOCAL_UNLOCK_other
	} else {
		Dee_weakref_clear(self);
	}
}

PUBLIC NONNULL((1, 2)) void DCALL
Dee_weakref_moveassign(struct weakref *self,
                       struct weakref *other) {
#ifndef NDEBUG
	ASSERT(self->wr_obj != (DeeObject *)WEAKREF_BAD_POINTER);
	ASSERT(other->wr_obj != (DeeObject *)WEAKREF_BAD_POINTER);
#endif /* !NDEBUG */
	if unlikely(self == other)
		return;
again:
	if (other->wr_obj) {
		WEAKREF_LOCK(other);
#define LOCAL_UNLOCK_other() WEAKREF_UNLOCK(other)
		COMPILER_READ_BARRIER();
		if likely(other->wr_obj) {
			struct weakref *next;
#define LOCAL_UNLOCK_other__prev() atomic_write(other->wr_pself, other)
			if unlikely(!WEAKREF_PREV_TRYLOCK(other)) {
				LOCAL_UNLOCK_other();
				SCHED_YIELD();
				goto again;
			}
			next = (struct weakref *)GET_POINTER(other->wr_next);
#define LOCAL_UNLOCK_other__next() do { if (next) WEAKREF_UNLOCK(next); } __WHILE0
			if (next) {
				if unlikely(!WEAKREF_TRYLOCK(next)) {
					LOCAL_UNLOCK_other__prev();
					LOCAL_UNLOCK_other();
					SCHED_YIELD();
					goto again;
				}
			}

#define LOCAL_UNLOCK_self() WEAKREF_UNLOCK(self)
			if unlikely(!WEAKREF_TRYLOCK(self)) {
				if (other->wr_pself == &self->wr_next || self == next) {
					/* Special case: `self' is a neighbor of `other' */
					if (next) {
						next->wr_pself = other->wr_pself;
						WEAKREF_UNLOCK(next); /* LOCAL_UNLOCK_other__next(); */
					}
					atomic_write(other->wr_pself, next); /* LOCAL_UNLOCK_other__prev(); */
					other->wr_obj = NULL;
					WEAKREF_SETBAD(other->wr_pself, struct weakref **);
					atomic_write(&other->wr_next, WEAKREF_EMPTY_NEXTVAL); /* LOCAL_UNLOCK_other(); */
					return;
				}
				LOCAL_UNLOCK_other__next();
				LOCAL_UNLOCK_other__prev();
				LOCAL_UNLOCK_other();
				WEAKREF_WAITFOR(self);
				goto again;
			}
			COMPILER_READ_BARRIER();
			if (self->wr_obj) {
				struct weakref *old_self_next;
				if unlikely(self->wr_obj == other->wr_obj) {
					/* Special case: both point to the same object -> must clear `other' */
					if (next) {
						next->wr_pself = other->wr_pself;
						WEAKREF_UNLOCK(next); /* LOCAL_UNLOCK_other__next(); */
					}
					atomic_write(other->wr_pself, next); /* LOCAL_UNLOCK_other__prev(); */
					other->wr_obj = NULL;
					WEAKREF_SETBAD(other->wr_pself, struct weakref **);
					atomic_write(&other->wr_next, WEAKREF_EMPTY_NEXTVAL); /* LOCAL_UNLOCK_other(); */
					LOCAL_UNLOCK_self();
					return;
				}

#define LOCAL_UNLOCK_self__prev() WEAKREF_PREV_UNLOCK(self)
				if unlikely(!WEAKREF_PREV_TRYLOCK(self)) {
					LOCAL_UNLOCK_self();
					LOCAL_UNLOCK_other__next();
					LOCAL_UNLOCK_other__prev();
					LOCAL_UNLOCK_other();
					SCHED_YIELD();
					goto again;
				}
				old_self_next = (struct weakref *)GET_POINTER(self->wr_next);
				if (old_self_next) {
					if unlikely(!WEAKREF_TRYLOCK(old_self_next)) {
						/* Prevent a deadlock. */
						LOCAL_UNLOCK_self__prev();
						LOCAL_UNLOCK_self();
						LOCAL_UNLOCK_other__next();
						LOCAL_UNLOCK_other__prev();
						LOCAL_UNLOCK_other();
						if unlikely(old_self_next == other)
							return;
						SCHED_YIELD();
						goto again;
					}
					old_self_next->wr_pself = self->wr_pself;
					WEAKREF_UNLOCK(old_self_next); /* LOCAL_UNLOCK_self__self(); */
				}
				atomic_write(self->wr_pself, old_self_next); /* LOCAL_UNLOCK_self__prev(); */
#undef LOCAL_UNLOCK_self__prev
			}

			self->wr_pself = other->wr_pself;
			self->wr_obj   = other->wr_obj;
			other->wr_obj = NULL;
			WEAKREF_SETBAD(other->wr_pself, struct weakref **);
			if (next) {
				next->wr_pself = &self->wr_next;
				WEAKREF_UNLOCK(next);                             /* LOCAL_UNLOCK_other__next(); */
			}
			WEAKREF_PREV_UNLOCK(self);                            /* LOCAL_UNLOCK_other__prev(); */
			atomic_write(&other->wr_next, WEAKREF_EMPTY_NEXTVAL); /* LOCAL_UNLOCK_other(); */
			atomic_write(&self->wr_next, next);                   /* LOCAL_UNLOCK_self(); */
#undef LOCAL_UNLOCK_self
#undef LOCAL_UNLOCK_other__next
#undef LOCAL_UNLOCK_other__prev
		} else {
			atomic_write(&other->wr_next, WEAKREF_EMPTY_NEXTVAL); /* LOCAL_UNLOCK_other(); */
			Dee_weakref_clear(self);
		}
#undef LOCAL_UNLOCK_other
	} else {
		Dee_weakref_clear(self);
	}
}

/* Finalize a given weak reference. */
PUBLIC NONNULL((1)) void DCALL
Dee_weakref_fini(struct weakref *__restrict self) {
#ifndef NDEBUG
	ASSERT(self->wr_obj != (DeeObject *)WEAKREF_BAD_POINTER);
#endif /* !NDEBUG */
again:
	if (self->wr_obj) {
		WEAKREF_LOCK(self);
		COMPILER_READ_BARRIER();
		if likely(self->wr_obj) {
			struct weakref *next;
			if unlikely(!WEAKREF_PREV_TRYLOCK(self)) {
				/* Prevent a deadlock. */
				WEAKREF_UNLOCK(self);
				SCHED_YIELD();
				goto again;
			}
			next = (struct weakref *)GET_POINTER(self->wr_next);
			if (next) {
				if unlikely(!WEAKREF_TRYLOCK(next)) {
					/* Prevent a deadlock. */
					WEAKREF_PREV_UNLOCK(self);
					WEAKREF_UNLOCK(self);
					SCHED_YIELD();
					goto again;
				}
				next->wr_pself = self->wr_pself;
				WEAKREF_UNLOCK(next);           /* WEAKREF_UNLOCK(self->NEXT) */
			}
			atomic_write(self->wr_pself, next); /* WEAKREF_UNLOCK(self->PREV) */
		}
		/*WEAKREF_UNLOCK(self);*/
	}
	WEAKREF_SETBAD(self->wr_pself, struct weakref **);
	WEAKREF_SETBAD(self->wr_next, struct weakref *);
	WEAKREF_SETBAD(self->wr_obj, DeeObject *);
}

PUBLIC NONNULL((1)) bool DCALL
Dee_weakref_clear(struct weakref *__restrict self) {
#ifndef NDEBUG
	ASSERT(self->wr_obj != (DeeObject *)WEAKREF_BAD_POINTER);
#endif /* !NDEBUG */
again:
	if (self->wr_obj) {
		WEAKREF_LOCK(self);
		COMPILER_READ_BARRIER();
		if likely(self->wr_obj) {
			struct weakref *next;
			if unlikely(!WEAKREF_PREV_TRYLOCK(self)) {
				WEAKREF_UNLOCK(self);
				SCHED_YIELD();
				goto again;
			}
			next = (struct weakref *)GET_POINTER(self->wr_next);
			if (next) {
				if unlikely(!WEAKREF_TRYLOCK(next)) {
					/* Prevent a deadlock. */
					WEAKREF_PREV_UNLOCK(self);
					WEAKREF_UNLOCK(self);
					SCHED_YIELD();
					goto again;
				}
				next->wr_pself = self->wr_pself;
				WEAKREF_UNLOCK(next);
			}
			atomic_write(self->wr_pself, next);
			self->wr_obj = NULL;
		}
		WEAKREF_SETBAD(self->wr_pself, struct weakref **);
		atomic_write(&self->wr_next, WEAKREF_EMPTY_NEXTVAL);
		return true;
	}
	return false;
}


/* Overwrite an already initialize weak reference with the given `ob'.
 * @return: true:  Successfully overwritten the weak reference.
 * @return: false: The given object `ob' does not support weak referencing
 *                 and the stored weak reference was not modified. */
PUBLIC NONNULL((1, 2)) bool DCALL
Dee_weakref_set(struct weakref *__restrict self,
                DeeObject *__restrict ob) {
	struct weakref_list *new_list;
	ASSERT_OBJECT(ob);
	ASSERT(IS_ALIGNED((uintptr_t)self, PTRLOCK_LOCK_MASK + 1));
	new_list = WEAKREFS_GET(ob);
	if unlikely(!WEAKREFS_OK(new_list, ob))
		return false;
again:
	WEAKREF_LOCK(self);
	if unlikely(ob == self->wr_obj) {
		/* Still the same object. */
		WEAKREF_UNLOCK(self);
	} else {
		struct weakref *next;
		if unlikely(!TRYLOCK_POINTER(new_list->wl_nodes)) {
			WEAKREF_UNLOCK(self);
			SCHED_YIELD();
			goto again;
		}

		next = (struct weakref *)GET_POINTER(new_list->wl_nodes);
		if (next) {
			if unlikely(!WEAKREF_TRYLOCK(next)) {
				UNLOCK_POINTER(new_list->wl_nodes);
				WEAKREF_UNLOCK(self);
				SCHED_YIELD();
				goto again;
			}
		}

		/* Delete a previously assigned object. */
		if (self->wr_obj) {
			struct weakref *old_self_next;
			if unlikely(!WEAKREF_PREV_TRYLOCK(self)) {
				if (next)
					WEAKREF_UNLOCK(next);            /* WEAKREF_UNLOCK(new_list->FIRST) */
				UNLOCK_POINTER(new_list->wl_nodes);  /* WEAKREF_UNLOCK(new_list) */
				WEAKREF_UNLOCK(self);                /* WEAKREF_UNLOCK(self) */
				SCHED_YIELD();
				goto again;
			}
			old_self_next = (struct weakref *)GET_POINTER(self->wr_next);
			if (old_self_next) {
				if unlikely(!WEAKREF_TRYLOCK(old_self_next)) {
					/* Prevent a deadlock. */
					WEAKREF_PREV_UNLOCK(self);          /* WEAKREF_UNLOCK(self->PREV) */
					if (next)
						WEAKREF_UNLOCK(next);           /* WEAKREF_UNLOCK(new_list->FIRST) */
					UNLOCK_POINTER(new_list->wl_nodes); /* WEAKREF_UNLOCK(new_list) */
					WEAKREF_UNLOCK(self);               /* WEAKREF_UNLOCK(self) */
					SCHED_YIELD();
					goto again;
				}
				old_self_next->wr_pself = self->wr_pself;
				WEAKREF_UNLOCK(old_self_next);           /* WEAKREF_UNLOCK(self->NEXT) */
			}
			atomic_write(self->wr_pself, old_self_next); /* WEAKREF_UNLOCK(self->PREV) */
		}

		/* Now to re-insert the weakref. */
		self->wr_pself = &new_list->wl_nodes;
		self->wr_obj   = ob;
		if (next) {
			next->wr_pself = &self->wr_next;
			WEAKREF_UNLOCK(next);                /* WEAKREF_UNLOCK(new_list->FIRST) */
		}
		atomic_write(&new_list->wl_nodes, self); /* WEAKREF_UNLOCK(new_list) */
		atomic_write(&self->wr_next, next);      /* WEAKREF_UNLOCK(self) */
	}
	return true;
}

/* Lock a weak reference, returning a regular reference to the pointed-to object.
 * @return: * :   A new reference to the pointed-to object.
 * @return: NULL: Failed to lock the weak reference (No error is thrown in this case). */
#ifdef __INTELLISENSE__
PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *
(DCALL Dee_weakref_lock)(struct weakref const *__restrict self)
#else /* __INTELLISENSE__ */
PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *
(DCALL Dee_weakref_lock)(struct weakref *__restrict self)
#endif /* !__INTELLISENSE__ */
{
	DREF DeeObject *result;
	ASSERT(IS_ALIGNED((uintptr_t)self, PTRLOCK_LOCK_MASK + 1));
#ifndef NDEBUG
	ASSERT(self->wr_obj != (DeeObject *)WEAKREF_BAD_POINTER);
#endif /* !NDEBUG */
	if ((result = self->wr_obj) != NULL) {
		WEAKREF_LOCK(self);
		COMPILER_READ_BARRIER();
		result = self->wr_obj; /* Re-read in case it changed. */
		if (likely(result) && !Dee_IncrefIfNotZero(result))
			result = NULL;
		WEAKREF_UNLOCK(self);
	}
	return result;
}

/* Return the state of a snapshot of `self' currently being bound. */
#ifdef __INTELLISENSE__
PUBLIC WUNUSED NONNULL((1)) bool
(DCALL Dee_weakref_bound)(struct weakref const *__restrict self)
#else /* __INTELLISENSE__ */
PUBLIC WUNUSED NONNULL((1)) bool
(DCALL Dee_weakref_bound)(struct weakref *__restrict self)
#endif /* !__INTELLISENSE__ */
{
	DeeObject *curr;
	ASSERT(IS_ALIGNED((uintptr_t)self, PTRLOCK_LOCK_MASK + 1));
#ifndef NDEBUG
	ASSERT(self->wr_obj != (DeeObject *)WEAKREF_BAD_POINTER);
#endif /* !NDEBUG */
	if ((curr = self->wr_obj) != NULL) {
		WEAKREF_LOCK(self);
		COMPILER_READ_BARRIER();
		curr = self->wr_obj; /* Re-read in case it changed. */
		if (atomic_read(&curr->ob_refcnt) == 0) {
			WEAKREF_UNLOCK(self);
			return false;
		}
		WEAKREF_UNLOCK(self);
		return true;
	}
	return false;
}

/* Do an atomic compare-exchange operation on the weak reference
 * and return a reference to the previously assigned object, or
 * `NULL' when none was assigned, or `Dee_ITER_DONE' when `new_ob'
 * does not support weak referencing functionality (in which case
 * the actual pointed-to weak object of `self' isn't changed).
 * NOTE: You may pass `NULL' for `new_ob' to clear the weakref. */
PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
Dee_weakref_cmpxch(struct weakref *__restrict self,
                   DeeObject *old_ob,
                   DeeObject *new_ob) {
	DREF DeeObject *result;
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
			if unlikely(!old_ob) {
				WEAKREF_UNLOCK(self);
			} else {
				struct weakref *next;

				/* Delete a previously assigned object. */
				if unlikely(!WEAKREF_PREV_TRYLOCK(self)) {
					WEAKREF_UNLOCK(self);
					SCHED_YIELD();
					goto again;
				}
				next = (struct weakref *)GET_POINTER(self->wr_next);
				if (next) {
					if unlikely(!WEAKREF_TRYLOCK(next)) {
						/* Prevent a deadlock. */
						WEAKREF_PREV_UNLOCK(self); /* WEAKREF_UNLOCK(self->PREV) */
						WEAKREF_UNLOCK(self);            /* WEAKREF_UNLOCK(self) */
						SCHED_YIELD();
						goto again;
					}
					next->wr_pself = self->wr_pself;
					WEAKREF_UNLOCK(next);                /* WEAKREF_UNLOCK(self->NEXT) */
				}
				atomic_write(self->wr_pself, next);      /* WEAKREF_UNLOCK(self->PREV) */

				/* Now to re-insert the weakref. */
				self->wr_obj = NULL;
				WEAKREF_SETBAD(self->wr_pself, struct weakref **);
				atomic_write(&self->wr_next, WEAKREF_EMPTY_NEXTVAL); /* WEAKREF_UNLOCK(self) */
			}
		} else {
			struct weakref_list *new_list;
			new_list = WEAKREFS_GET(new_ob);
			if unlikely(!WEAKREFS_OK(new_list, new_ob)) {
				WEAKREF_UNLOCK(self);
				/* Weak referencing is not supported by `new_ob'. */
				return ITER_DONE;
			} else if unlikely(old_ob == new_ob) {
				/* Special case: `old_ob' matches `new_ob' */
				WEAKREF_UNLOCK(self);
			} else {
				struct weakref *next;
				if (!TRYLOCK_POINTER(new_list->wl_nodes)) {
					WEAKREF_UNLOCK(self);
					SCHED_YIELD();
					goto again;
				}

				next = (struct weakref *)GET_POINTER(new_list->wl_nodes);
				if (next) {
					if (!WEAKREF_TRYLOCK(next)) {
						UNLOCK_POINTER(new_list->wl_nodes); /* WEAKREF_UNLOCK(new_list) */
						WEAKREF_UNLOCK(self);               /* WEAKREF_UNLOCK(self) */
						SCHED_YIELD();
						goto again;
					}
				}

				/* Delete a previously assigned object. */
				if (old_ob) {
					struct weakref *old_self_next;
					if (!WEAKREF_PREV_TRYLOCK(self)) {
						if (next)
							WEAKREF_UNLOCK(next);           /* WEAKREF_UNLOCK(new_list->FIRST) */
						UNLOCK_POINTER(new_list->wl_nodes); /* WEAKREF_UNLOCK(new_list) */
						WEAKREF_UNLOCK(self);               /* WEAKREF_UNLOCK(self) */
						SCHED_YIELD();
						goto again;
					}
					old_self_next = (struct weakref *)GET_POINTER(self->wr_next);
					if (old_self_next) {
						if unlikely(!WEAKREF_TRYLOCK(old_self_next)) {
							/* Prevent a deadlock. */
							WEAKREF_PREV_UNLOCK(self);    /* WEAKREF_UNLOCK(self->PREV) */
							if (next)
								WEAKREF_UNLOCK(next);           /* WEAKREF_UNLOCK(new_list->FIRST) */
							UNLOCK_POINTER(new_list->wl_nodes); /* WEAKREF_UNLOCK(new_list) */
							WEAKREF_UNLOCK(self);               /* WEAKREF_UNLOCK(self) */
							SCHED_YIELD();
							goto again;
						}
						old_self_next->wr_pself = self->wr_pself;
						WEAKREF_UNLOCK(old_self_next);           /* WEAKREF_UNLOCK(self->NEXT) */
					}
					atomic_write(self->wr_pself, old_self_next); /* WEAKREF_UNLOCK(self->PREV) */
				}

				/* Now to re-insert the weakref. */
				self->wr_pself = &new_list->wl_nodes;
				self->wr_obj   = new_ob;
				if (next) {
					next->wr_pself = &self->wr_next;
					WEAKREF_UNLOCK(next);                /* WEAKREF_UNLOCK(new_list->FIRST) */
				}
				atomic_write(&self->wr_next, next);      /* WEAKREF_UNLOCK(self) */
				atomic_write(&new_list->wl_nodes, self); /* WEAKREF_UNLOCK(new_list) */
			}
		}
	} else if (result != NULL) {
		/* Do an atomic-inc-if-not-zero on the reference counter. */
		if (!Dee_IncrefIfNotZero(result))
			result = NULL;
		WEAKREF_UNLOCK(self);
	}
	return result;
}



PUBLIC WUNUSED NONNULL((2)) bool DCALL
DeeObject_UndoConstruction(DeeTypeObject *undo_start,
                           DeeObject *self) {
	if unlikely(!atomic_cmpxch(&self->ob_refcnt, 1, 0))
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
				Dee_refcnt_t refcnt;
				do {
					refcnt = atomic_read(&self->ob_refcnt);
					if (refcnt == 0)
						goto destroy_weak;
				} while unlikely(atomic_cmpxch_weak_or_write(&self->ob_refcnt, refcnt, refcnt + 1));
				return false;
			}
		}

		/* Delete all weak references linked against this type level. */
destroy_weak:
		if (has_noninherited_weakrefs(undo_start)) {
			struct weakref *iter, *next;
			struct weakref_list *list;
			ASSERT(undo_start->tp_weakrefs >= sizeof(DeeObject));
			list = (struct weakref_list *)((uintptr_t)self + undo_start->tp_weakrefs);
restart_clear_weakrefs:
			LOCK_POINTER(list->wl_nodes);
			iter = (struct weakref *)GET_POINTER(list->wl_nodes);
			if (iter == NULL) {
				UNLOCK_POINTER(list->wl_nodes);
			} else {
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
						WEAKREF_UNLOCK(iter);           /* WEAKREF_UNLOCK(list->FIRST) */
						UNLOCK_POINTER(list->wl_nodes); /* WEAKREF_UNLOCK(list) */
						SCHED_YIELD();
						goto restart_clear_weakrefs;
					}
					next->wr_pself = &list->wl_nodes;
					WEAKREF_UNLOCK(next);    /* WEAKREF_UNLOCK(list->FIRST) */
				}

				/* Overwrite the weakly referenced object with NULL,
				 * indicating that the link has been severed. */
				atomic_write(&list->wl_nodes, next); /* WEAKREF_UNLOCK(list) */
				WEAKREF_SETBAD(iter->wr_pself, struct weakref **);
				iter->wr_obj = NULL;
				COMPILER_WRITE_BARRIER();
				if (iter->wr_del) {
					(*iter->wr_del)(iter);
				} else {
					atomic_write(&iter->wr_next, WEAKREF_EMPTY_NEXTVAL);
				}
				goto restart_clear_weakrefs;
			}
		}
	}
	return true;
}

#ifndef CONFIG_NO_BADREFCNT_CHECKS
#ifdef CONFIG_DEFAULT_MESSAGE_FORMAT_MSVC
#define FILE_AND_LINE_FORMAT "%s(%d) : "
#elif defined(CONFIG_DEFAULT_MESSAGE_FORMAT_GCC)
#define FILE_AND_LINE_FORMAT "%s:%d: "
#endif /* ... */

#ifndef CONFIG_NO_THREADS
PRIVATE Dee_atomic_lock_t bad_refcnt_lock = DEE_ATOMIC_LOCK_INIT;
#endif /* !CONFIG_NO_THREADS */
#define bad_refcnt_lock_available()  Dee_atomic_lock_available(&bad_refcnt_lock)
#define bad_refcnt_lock_acquired()   Dee_atomic_lock_acquired(&bad_refcnt_lock)
#define bad_refcnt_lock_tryacquire() Dee_atomic_lock_tryacquire(&bad_refcnt_lock)
#define bad_refcnt_lock_acquire()    Dee_atomic_lock_acquire(&bad_refcnt_lock)
#define bad_refcnt_lock_waitfor()    Dee_atomic_lock_waitfor(&bad_refcnt_lock)
#define bad_refcnt_lock_release()    Dee_atomic_lock_release(&bad_refcnt_lock)

PUBLIC NONNULL((1)) void DCALL
DeeFatal_BadIncref(DeeObject *ob, char const *file, int line) {
	DeeTypeObject *type;
	bad_refcnt_lock_acquire();
	Dee_DPRINTF("\n\n\n" FILE_AND_LINE_FORMAT "BAD_INCREF(%p)\n",
	            file, line, ob);
	Dee_DPRINTF("refcnt : %" PRFuSIZ " (%#" PRFXSIZ ")\n",
	            ob->ob_refcnt, ob->ob_refcnt);
	type = Dee_TYPE(ob);
	if (DeeObject_Check(type) && DeeType_Check(type)) {
		Dee_DPRINTF("type : %s (%p)", type->tp_name, type);
	} else {
		Dee_DPRINTF("type : <INVALID> - %p", type);
	}
	Dee_DPRINTF("\n\n\n");
	bad_refcnt_lock_release();
	Dee_BREAKPOINT();
}

PUBLIC NONNULL((1)) void DCALL
DeeFatal_BadDecref(DeeObject *ob, char const *file, int line) {
	DeeTypeObject *type;
	bad_refcnt_lock_acquire();
	Dee_DPRINTF("\n\n\n" FILE_AND_LINE_FORMAT "BAD_DECREF(%p)\n",
	            file, line, ob);
	Dee_DPRINTF("refcnt : %" PRFuSIZ " (%" PRFXSIZ ")\n",
	            ob->ob_refcnt, ob->ob_refcnt);
	type = Dee_TYPE(ob);
	if (DeeObject_Check(type) && DeeType_Check(type)) {
		Dee_DPRINTF("type : %s (%p)", type->tp_name, type);
	} else {
		Dee_DPRINTF("type : <INVALID> - %p", type);
	}
	Dee_DPRINTF("\n\n\n");
	bad_refcnt_lock_release();
	Dee_BREAKPOINT();
}
#else /* !CONFIG_NO_BADREFCNT_CHECKS */
PUBLIC void DCALL
DeeFatal_BadIncref(DeeObject *UNUSED(ob),
                   char const *UNUSED(file),
                   int UNUSED(line)) {
	abort();
}
#ifdef __NO_DEFINE_ALIAS
PUBLIC void DCALL
DeeFatal_BadDecref(DeeObject *UNUSED(ob),
                   char const *UNUSED(file),
                   int UNUSED(line)) {
	abort();
}
#else /* __NO_DEFINE_ALIAS */
DEFINE_PUBLIC_ALIAS(ASSEMBLY_NAME(DeeFatal_BadDecref, 12),
                    ASSEMBLY_NAME(DeeFatal_BadIncref, 12));
#endif /* !__NO_DEFINE_ALIAS */
#endif /* CONFIG_NO_BADREFCNT_CHECKS */


/* Finalize weakref support */
PUBLIC NONNULL((1)) void
(DCALL Dee_weakref_support_fini)(struct weakref_list *__restrict list) {
	struct weakref *iter, *next;
restart_clear_weakrefs:
	LOCK_POINTER(list->wl_nodes);
	iter = (struct weakref *)GET_POINTER(list->wl_nodes);
	if (iter != NULL) {
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
				WEAKREF_UNLOCK(iter);           /* WEAKREF_UNLOCK(list->FIRST) */
				UNLOCK_POINTER(list->wl_nodes); /* WEAKREF_UNLOCK(list) */
				SCHED_YIELD();
				goto restart_clear_weakrefs;
			}
			next->wr_pself = &list->wl_nodes;
			WEAKREF_UNLOCK(next);
		}

		/* Overwrite the weakly referenced object with NULL,
		 * indicating that the link has been severed. */
		atomic_write(&list->wl_nodes, next);
		WEAKREF_SETBAD(iter->wr_pself, struct weakref **);
		iter->wr_obj = NULL;
		COMPILER_WRITE_BARRIER();
		if (iter->wr_del) {
			(*iter->wr_del)(iter);
		} else {
			atomic_write(&iter->wr_next, WEAKREF_EMPTY_NEXTVAL);
		}
		goto restart_clear_weakrefs;
	}
	atomic_write(&list->wl_nodes, NULL); /* WEAKREF_UNLOCK(list) */
}


/* Destroy a given deemon object (called when its refcnt reaches `0') */
#ifdef CONFIG_NO_BADREFCNT_CHECKS
PUBLIC NONNULL((1)) void
(DCALL DeeObject_Destroy_d)(DeeObject *__restrict self,
                            char const *UNUSED(file),
                            int UNUSED(line)) {
	DeeObject_Destroy(self);
}

PUBLIC NONNULL((1)) void
(DCALL DeeObject_Destroy)(DeeObject *__restrict self)
#else /* CONFIG_NO_BADREFCNT_CHECKS */
PUBLIC NONNULL((1)) void
(DCALL DeeObject_Destroy)(DeeObject *__restrict self) {
	DeeObject_Destroy_d(self, NULL, 0);
}

PUBLIC NONNULL((1)) void
(DCALL DeeObject_Destroy_d)(DeeObject *__restrict self,
                            char const *file, int line)
#endif /* !CONFIG_NO_BADREFCNT_CHECKS */
{
#undef CONFIG_OBJECT_DESTROY_CHECK_MEMORY
	//#define CONFIG_OBJECT_DESTROY_CHECK_MEMORY
	DeeTypeObject *orig_type, *type;
#ifndef CONFIG_TRACE_REFCHANGES
again:
#endif /* !CONFIG_TRACE_REFCHANGES */
	orig_type = type = Dee_TYPE(self);
#ifndef CONFIG_NO_BADREFCNT_CHECKS
	if (self->ob_refcnt != 0) {
		bad_refcnt_lock_acquire();
		Dee_DPRINTF("\n\n\n" FILE_AND_LINE_FORMAT "BAD_DESTROY(%p)\n",
		            file, line, self);
		Dee_DPRINTF("refcnt : %" PRFuSIZ " (%" PRFXSIZ ")\n", self->ob_refcnt, self->ob_refcnt);
		if (DeeObject_Check(type) && DeeType_Check(type)) {
			Dee_DPRINTF("type : %s (%p)", type->tp_name, type);
		} else {
			Dee_DPRINTF("type : <INVALID> - %p", type);
		}
		Dee_DPRINTF("\n\n\n");
		bad_refcnt_lock_release();
		Dee_BREAKPOINT();
	}
#endif /* !CONFIG_NO_BADREFCNT_CHECKS */

#if 0
#ifndef CONFIG_NO_THREADS
	/* Make sure that all threads now see this object as dead.
	 * For why this is required, see `INCREF_IF_NONZERO()' */
	atomic_thread_fence(Dee_ATOMIC_ACQ_REL);
#endif /* !CONFIG_NO_THREADS */
#endif

#ifdef CONFIG_OBJECT_DESTROY_CHECK_MEMORY
	Dee_CHECKMEMORY();
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
				Dee_CHECKMEMORY();
#endif /* CONFIG_OBJECT_DESTROY_CHECK_MEMORY */

				/* Special case: The destructor managed to revive the object. */
				if unlikely(self->ob_refcnt != 0) {
					Dee_Incref(type);
					ASSERTF(type->tp_flags & TP_FGC,
					        "This runtime does not implementing reviving "
					        "GC-allocated objects as non-GC objects.");

					/* Continue tracking the object. */
					self = DeeGC_Track(self);

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
						Dee_refcnt_t oldref;
						oldref = atomic_fetchdec(&self->ob_refcnt);
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
		if (orig_type->tp_init.tp_alloc.tp_free) {
			(*orig_type->tp_init.tp_alloc.tp_free)(self);
		} else {
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
				Dee_CHECKMEMORY();
#endif /* CONFIG_OBJECT_DESTROY_CHECK_MEMORY */

				/* Special case: The destructor managed to revive the object. */
				if unlikely(self->ob_refcnt != 0) {
					/* Incref() the new type that now describes this revived object.
					 * NOTE: The fact that this type may use a different (or none at all)
					 *       tp_free function, is the reason why no GC-able type from who's
					 *       destruction a user-callback that can somehow get a hold of the
					 *       instance being destroyed (which is also possible for any weakly
					 *       referenceable type), is allowed to assume that it will actually
					 *       be called, limiting its use to pre-allocated object caches that
					 *       allocate their instances using `DeeObject_Malloc'. */
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
						Dee_refcnt_t oldref;
						oldref = atomic_fetchdec(&self->ob_refcnt);
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
		if (orig_type->tp_init.tp_alloc.tp_free) {
			(*orig_type->tp_init.tp_alloc.tp_free)(self);
		} else {
			DeeObject_Free(self);
		}
	}
/*done:*/
	/* Drop a reference from the original type. */
	Dee_Decref(orig_type);
}


PRIVATE WUNUSED NONNULL((1)) int DCALL
object_any_ctor(DeeObject *__restrict UNUSED(self),
                size_t argc, DeeObject *const *argv) {
	return DeeArg_Unpack(argc, argv, ":Object");
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeStringObject *DCALL
object_str(DeeObject *__restrict self) {
#if 1
	DeeTypeObject *tp_self = Dee_TYPE(self);
	if (tp_self->tp_name) {
		if (tp_self->tp_flags & TP_FNAMEOBJECT) {
			DREF DeeStringObject *result;
			result = COMPILER_CONTAINER_OF(tp_self->tp_name,
			                               DeeStringObject,
			                               s_str);
			Dee_Incref(result);
			return result;
		}
		return (DREF DeeStringObject *)DeeString_New(tp_self->tp_name);
	}
	Dee_Incref(&str_Object);
	return &str_Object;
#else
	if (Dee_TYPE(self) != &DeeObject_Type)
		goto err_noimp;
	Dee_Incref(&str_Object);
	return &str_Object;
err_noimp:
	err_unimplemented_operator(Dee_TYPE(self), OPERATOR_STR);
	return NULL;
#endif
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeStringObject *DCALL
object_repr(DeeObject *__restrict self) {
	if (Dee_TYPE(self) == &DeeObject_Type)
		return (DREF DeeStringObject *)DeeString_New("Object()");
	err_unimplemented_operator(Dee_TYPE(self), OPERATOR_REPR);
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

#ifndef STR___copy__
#define STR___copy__       (meth_copy + 1)
#endif /* !STR___copy__ */
#ifndef STR___deepcopy__
#define STR___deepcopy__   (meth_deepcopy + 1)
#endif /* !STR___deepcopy__ */
#ifndef STR___assign__
#define STR___assign__     (meth_assign + 2)
#endif /* !STR___assign__ */
#ifndef STR___moveassign__
#define STR___moveassign__ (meth_moveassign + 2)
#endif /* !STR___moveassign__ */
#ifndef STR___str__
#define STR___str__        (meth_str + 1)
#endif /* !STR___str__ */
#ifndef STR___repr__
#define STR___repr__       (meth_repr + 1)
#endif /* !STR___repr__ */
#ifndef STR___bool__
#define STR___bool__       (meth_bool + 1)
#endif /* !STR___bool__ */
#ifndef STR___call__
#define STR___call__       (meth_call + 2)
#endif /* !STR___call__ */
#ifndef STR___thiscall__
#define STR___thiscall__   (meth_thiscall + 4)
#endif /* !STR___thiscall__ */
#ifndef STR___hash__
#define STR___hash__       (meth_hash + 1)
#endif /* !STR___hash__ */
#ifndef STR___int__
#define STR___int__        (meth_int + 1)
#endif /* !STR___int__ */
#ifndef STR___inv__
#define STR___inv__        (meth_inv + 1)
#endif /* !STR___inv__ */
#ifndef STR___pos__
#define STR___pos__        (meth_pos + 1)
#endif /* !STR___pos__ */
#ifndef STR___neg__
#define STR___neg__        (meth_neg + 1)
#endif /* !STR___neg__ */
#ifndef STR___add__
#define STR___add__        (meth_add + 2)
#endif /* !STR___add__ */
#ifndef STR___sub__
#define STR___sub__        (meth_sub + 2)
#endif /* !STR___sub__ */
#ifndef STR___mul__
#define STR___mul__        (meth_mul + 2)
#endif /* !STR___mul__ */
#ifndef STR___div__
#define STR___div__        (meth_div + 2)
#endif /* !STR___div__ */
#ifndef STR___mod__
#define STR___mod__        (meth_mod + 2)
#endif /* !STR___mod__ */
#ifndef STR___shl__
#define STR___shl__        (meth_shl + 2)
#endif /* !STR___shl__ */
#ifndef STR___shr__
#define STR___shr__        (meth_shr + 2)
#endif /* !STR___shr__ */
#ifndef STR___and__
#define STR___and__        (meth_and + 2)
#endif /* !STR___and__ */
#ifndef STR___or__
#define STR___or__         (meth_or + 2)
#endif /* !STR___or__ */
#ifndef STR___xor__
#define STR___xor__        (meth_xor + 2)
#endif /* !STR___xor__ */
#ifndef STR___pow__
#define STR___pow__        (meth_pow + 2)
#endif /* !STR___pow__ */
#ifndef STR___eq__
#define STR___eq__         (meth_eq + 2)
#endif /* !STR___eq__ */
#ifndef STR___ne__
#define STR___ne__         (meth_ne + 2)
#endif /* !STR___ne__ */
#ifndef STR___lo__
#define STR___lo__         (meth_lo + 2)
#endif /* !STR___lo__ */
#ifndef STR___le__
#define STR___le__         (meth_le + 2)
#endif /* !STR___le__ */
#ifndef STR___gr__
#define STR___gr__         (meth_gr + 2)
#endif /* !STR___gr__ */
#ifndef STR___ge__
#define STR___ge__         (meth_ge + 2)
#endif /* !STR___ge__ */
#ifndef STR___size__
#define STR___size__       (meth_size + 1)
#endif /* !STR___size__ */
#ifndef STR___contains__
#define STR___contains__   (meth_contains + 2)
#endif /* !STR___contains__ */
#ifndef STR___getitem__
#define STR___getitem__    (meth_getitem + 2)
#endif /* !STR___getitem__ */
#ifndef STR___delitem__
#define STR___delitem__    (meth_delitem + 2)
#endif /* !STR___delitem__ */
#ifndef STR___setitem__
#define STR___setitem__    (meth_setitem + 3)
#endif /* !STR___setitem__ */
#ifndef STR___getrange__
#define STR___getrange__   (meth_getrange + 3)
#endif /* !STR___getrange__ */
#ifndef STR___delrange__
#define STR___delrange__   (meth_delrange + 3)
#endif /* !STR___delrange__ */
#ifndef STR___setrange__
#define STR___setrange__   (meth_setrange + 4)
#endif /* !STR___setrange__ */
#ifndef STR___iterself__
#define STR___iterself__   (meth_iterself + 1)
#endif /* !STR___iterself__ */
#ifndef STR___iternext__
#define STR___iternext__   (meth_iternext + 1)
#endif /* !STR___iternext__ */
#ifndef STR___getattr__
#define STR___getattr__    (meth_getattr + 2)
#endif /* !STR___getattr__ */
#ifndef STR___callattr__
#define STR___callattr__   (meth_callattr)
#endif /* !STR___callattr__ */
#ifndef STR___hasattr__
#define STR___hasattr__    (meth_hasattr + 2)
#endif /* !STR___hasattr__ */
#ifndef STR___delattr__
#define STR___delattr__    (meth_delattr + 2)
#endif /* !STR___delattr__ */
#ifndef STR___setattr__
#define STR___setattr__    (meth_setattr + 3)
#endif /* !STR___setattr__ */
#ifndef STR___enumattr__
#define STR___enumattr__   (meth_enumattr + 1)
#endif /* !STR___enumattr__ */

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
object_sizeof(DeeObject *self) {
	DeeTypeObject *type;

	/* Individual sub-types should override this function and add the proper value.
	 * This implementation is merely used for any generic fixed-length type that
	 * doesn't do any custom heap allocations. */
	type = Dee_TYPE(self);

	/* Variable types lack a standardized way of determining their size in bytes. */
	if unlikely(type->tp_flags & TP_FVARIABLE)
		goto err_isvar;
	if unlikely(type->tp_init.tp_alloc.tp_free) {
#ifndef CONFIG_NO_OBJECT_SLABS
		/* Check for slab allocators. */
		size_t slab_size;
		void (DCALL *tp_free)(void *__restrict ob);
		tp_free = type->tp_init.tp_alloc.tp_free;
#define CHECK_SIZE(index, size)                       \
		if (tp_free == &DeeObject_SlabFree##size ||   \
		    tp_free == &DeeGCObject_SlabFree##size) { \
			slab_size = size * __SIZEOF_POINTER__;    \
		} else
		DeeSlab_ENUMERATE(CHECK_SIZE)
#undef CHECK_SIZE
		{
			goto err_iscustom;
		}
		return DeeInt_NewSize(slab_size);
#else /* !CONFIG_NO_OBJECT_SLABS */
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

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
object_copy(DeeObject *self, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, meth_copy))
		goto err;
	return DeeObject_Copy(self);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
object_deepcopy(DeeObject *self, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, meth_deepcopy))
		goto err;
	return DeeObject_DeepCopy(self);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
object_assign(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *other;
	if (DeeArg_Unpack(argc, argv, meth_assign, &other))
		goto err;
	if (DeeObject_Assign(self, other))
		goto err;
	return_reference_(self);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
object_moveassign(DeeObject *self, size_t argc, DeeObject *const *argv) {
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

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
object_dostr(DeeObject *self, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, meth_str))
		goto err;
	return DeeObject_Str(self);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
object_dorepr(DeeObject *self, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, meth_repr))
		goto err;
	return DeeObject_Repr(self);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
object_bool(DeeObject *self, size_t argc, DeeObject *const *argv) {
	int result;
	if (DeeArg_Unpack(argc, argv, meth_bool))
		goto err;
	result = DeeObject_Bool(self);
	if unlikely(result < 0)
		goto err;
	return_bool_(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
object_call(DeeObject *self, size_t argc, DeeObject *const *argv) {
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

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
object_thiscall(DeeObject *self, size_t argc, DeeObject *const *argv) {
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

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
object_hash(DeeObject *self, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, meth_hash))
		goto err;
	return DeeInt_NewSize(DeeObject_Hash(self));
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
object_int(DeeObject *self, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, meth_int))
		goto err;
	return DeeObject_Int(self);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
object_inv(DeeObject *self, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, meth_inv))
		goto err;
	return DeeObject_Inv(self);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
object_pos(DeeObject *self, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, meth_pos))
		goto err;
	return DeeObject_Pos(self);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
object_neg(DeeObject *self, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, meth_neg))
		goto err;
	return DeeObject_Neg(self);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
object_add(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *other;
	if (DeeArg_Unpack(argc, argv, meth_add, &other))
		goto err;
	return DeeObject_Add(self, other);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
object_sub(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *other;
	if (DeeArg_Unpack(argc, argv, meth_sub, &other))
		goto err;
	return DeeObject_Sub(self, other);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
object_mul(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *other;
	if (DeeArg_Unpack(argc, argv, meth_mul, &other))
		goto err;
	return DeeObject_Mul(self, other);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
object_div(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *other;
	if (DeeArg_Unpack(argc, argv, meth_div, &other))
		goto err;
	return DeeObject_Div(self, other);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
object_mod(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *other;
	if (DeeArg_Unpack(argc, argv, meth_mod, &other))
		goto err;
	return DeeObject_Mod(self, other);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
object_shl(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *other;
	if (DeeArg_Unpack(argc, argv, meth_shl, &other))
		goto err;
	return DeeObject_Shl(self, other);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
object_shr(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *other;
	if (DeeArg_Unpack(argc, argv, meth_shr, &other))
		goto err;
	return DeeObject_Shr(self, other);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
object_and(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *other;
	if (DeeArg_Unpack(argc, argv, meth_and, &other))
		goto err;
	return DeeObject_And(self, other);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
object_or(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *other;
	if (DeeArg_Unpack(argc, argv, meth_or, &other))
		goto err;
	return DeeObject_Or(self, other);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
object_xor(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *other;
	if (DeeArg_Unpack(argc, argv, meth_xor, &other))
		goto err;
	return DeeObject_Xor(self, other);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
object_pow(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *other;
	if (DeeArg_Unpack(argc, argv, meth_pow, &other))
		goto err;
	return DeeObject_Pow(self, other);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
object_eq(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *other;
	if (DeeArg_Unpack(argc, argv, meth_eq, &other))
		goto err;
	return DeeObject_CmpEq(self, other);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
object_ne(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *other;
	if (DeeArg_Unpack(argc, argv, meth_ne, &other))
		goto err;
	return DeeObject_CmpNe(self, other);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
object_lo(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *other;
	if (DeeArg_Unpack(argc, argv, meth_lo, &other))
		goto err;
	return DeeObject_CmpLo(self, other);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
object_le(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *other;
	if (DeeArg_Unpack(argc, argv, meth_le, &other))
		goto err;
	return DeeObject_CmpLe(self, other);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
object_gr(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *other;
	if (DeeArg_Unpack(argc, argv, meth_gr, &other))
		goto err;
	return DeeObject_CmpGr(self, other);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
object_ge(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *other;
	if (DeeArg_Unpack(argc, argv, meth_ge, &other))
		goto err;
	return DeeObject_CmpGe(self, other);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
object_size(DeeObject *self, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, meth_size))
		goto err;
	return DeeObject_SizeOb(self);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
object_contains(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *other;
	if (DeeArg_Unpack(argc, argv, meth_contains, &other))
		goto err;
	return DeeObject_Contains(self, other);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
object_getitem(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *other;
	if (DeeArg_Unpack(argc, argv, meth_getitem, &other))
		goto err;
	return DeeObject_GetItem(self, other);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
object_delitem(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *other;
	if (DeeArg_Unpack(argc, argv, meth_delitem, &other))
		goto err;
	if (DeeObject_DelItem(self, other))
		goto err;
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
object_setitem(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *other, *value;
	if (DeeArg_Unpack(argc, argv, meth_setitem, &other, &value))
		goto err;
	if (DeeObject_SetItem(self, other, value))
		goto err;
	return_reference_(value);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
object_getrange(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *begin_index, *end_index;
	if (DeeArg_Unpack(argc, argv, meth_getrange, &begin_index, &end_index))
		goto err;
	return DeeObject_GetRange(self, begin_index, end_index);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
object_delrange(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *begin_index, *end_index;
	if (DeeArg_Unpack(argc, argv, meth_delrange, &begin_index, &end_index))
		goto err;
	if (DeeObject_DelRange(self, begin_index, end_index))
		goto err;
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
object_setrange(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *begin_index, *end_index, *value;
	if (DeeArg_Unpack(argc, argv, meth_setrange, &begin_index, &end_index, &value))
		goto err;
	if (DeeObject_SetRange(self, begin_index, end_index, value))
		goto err;
	return_reference_(value);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
object_iterself(DeeObject *self, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, meth_iterself))
		goto err;
	return DeeObject_Iter(self);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
object_iternext(DeeObject *self, size_t argc, DeeObject *const *argv) {
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

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
object_getattr(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *name;
	if (DeeArg_Unpack(argc, argv, meth_getattr, &name))
		goto err;
	if (DeeObject_AssertTypeExact(name, &DeeString_Type))
		goto err;
	return DeeObject_GetAttr(self, name);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
object_callattr(DeeObject *self, size_t argc, DeeObject *const *argv) {
	if unlikely(!argc)
		goto err_badargc;
	if (DeeObject_AssertTypeExact(argv[0], &DeeString_Type))
		goto err;
	return DeeObject_CallAttr(self, argv[0], argc - 1, argv + 1);
err_badargc:
	err_invalid_argc_va(meth_callattr, argc, 1);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
object_hasattr(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *name;
	int result;
	if (DeeArg_Unpack(argc, argv, meth_hasattr, &name))
		goto err;
	if (DeeObject_AssertTypeExact(name, &DeeString_Type))
		goto err;
	result = DeeObject_HasAttr(self, name);
	if unlikely(result < 0)
		goto err;
	return_bool_(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
object_delattr(DeeObject *self, size_t argc, DeeObject *const *argv) {
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

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
object_setattr(DeeObject *self, size_t argc, DeeObject *const *argv) {
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

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
object_enumattr(DeeObject *self, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, meth_enumattr))
		goto err;
	return DeeObject_New(&DeeEnumAttr_Type, 1, (DeeObject **)&self);
err:
	return NULL;
}

INTDEF WUNUSED NONNULL((1, 2, 4)) dssize_t DCALL
object_format_generic(DeeObject *__restrict self,
                      dformatprinter printer, void *arg,
                      /*utf-8*/ char const *__restrict format_str,
                      size_t format_len);

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
object_format_method(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *format_str;
	char *format_utf8;
	if (DeeArg_Unpack(argc, argv, "o:__format__", &format_str))
		goto err;
	if (DeeObject_AssertTypeExact(format_str, &DeeString_Type))
		goto err;
	if unlikely((format_utf8 = DeeString_AsUtf8(format_str)) == NULL)
		goto err;
	{
		dssize_t error;
		struct unicode_printer printer = UNICODE_PRINTER_INIT;
		error = object_format_generic(self,
		                              &unicode_printer_print,
		                              &printer, format_utf8,
		                              WSTR_LENGTH(format_utf8));
		if unlikely(error < 0)
			goto err_printer;
		return unicode_printer_pack(&printer);
err_printer:
		unicode_printer_fini(&printer);
	}
err:
	return NULL;
}

#ifndef CONFIG_NO_DEEMON_100_COMPAT
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
object_not(DeeObject *self, size_t argc, DeeObject *const *argv) {
	int temp;
	if (DeeArg_Unpack(argc, argv, ":__not__"))
		goto err;
	temp = DeeObject_Bool(self);
	if unlikely(temp < 0)
		goto err;
	return_bool_(!temp);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
object_is(DeeObject *self, size_t argc, DeeObject *const *argv) {
	unsigned int is_instance;
	DeeTypeObject *tp;
	if (DeeArg_Unpack(argc, argv, "o:__is__", &tp))
		goto err;
	if (DeeNone_Check((DeeObject *)tp)) {
		is_instance = DeeNone_Check(self) ? 1 : 0;
	} else if (DeeSuper_Check(self)) {
		is_instance = DeeType_Extends(DeeSuper_TYPE(self), tp);
	} else {
		is_instance = DeeObject_Implements(self, tp);
	}
	Dee_return_reference_((DeeObject *)&Dee_FalseTrue[is_instance]);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
object_inc(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DREF DeeObject *selfref;
	int error;
	if (DeeArg_Unpack(argc, argv, ":__inc__"))
		goto err;
	selfref = self;
	Dee_Incref(selfref);
	error = DeeObject_Inc(&selfref);
	if unlikely(error)
		goto err_selfref;
	error = DeeObject_Assign(self, selfref);
	if unlikely(error)
		goto err_selfref;
	return selfref;
err_selfref:
	Dee_Decref(selfref);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
object_dec(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DREF DeeObject *selfref;
	int error;
	if (DeeArg_Unpack(argc, argv, ":__dec__"))
		goto err;
	selfref = self;
	Dee_Incref(selfref);
	error = DeeObject_Dec(&selfref);
	if unlikely(error)
		goto err_selfref;
	error = DeeObject_Assign(self, selfref);
	if unlikely(error)
		goto err_selfref;
	return selfref;
err_selfref:
	Dee_Decref(selfref);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
object_incpost(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DREF DeeObject *selfref, *result;
	int error;
	if (DeeArg_Unpack(argc, argv, ":__incpost__"))
		goto err;
	result = DeeObject_Copy(self);
	if unlikely(!result)
		goto err;
	selfref = self;
	Dee_Incref(selfref);
	error = DeeObject_Inc(&selfref);
	if (likely(error == 0) && selfref != self)
		error = DeeObject_Assign(self, selfref);
	Dee_Decref(selfref);
	if unlikely(error)
		goto err_r;
	return result;
err_r:
	Dee_Decref(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
object_decpost(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DREF DeeObject *selfref, *result;
	int error;
	if (DeeArg_Unpack(argc, argv, ":__decpost__"))
		goto err;
	result = DeeObject_Copy(self);
	if unlikely(!result)
		goto err;
	selfref = self;
	Dee_Incref(selfref);
	error = DeeObject_Dec(&selfref);
	if (likely(error == 0) && selfref != self)
		error = DeeObject_Assign(self, selfref);
	Dee_Decref(selfref);
	if unlikely(error)
		goto err_r;
	return result;
err_r:
	Dee_Decref(result);
err:
	return NULL;
}

#define DEFINE_DEPRECATED_INPLACE_BINARY(name, func)              \
	PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL            \
	object_##name(DeeObject *self, size_t argc,                   \
	              DeeObject *const *argv) {                       \
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


PRIVATE struct type_method tpconst object_methods[] = {
	/* Operator invocation functions. */
	TYPE_METHOD(STR___copy__, &object_copy,
	            "->\n"
	            "#r{A copy of @this object}"),
	TYPE_METHOD(STR___deepcopy__, &object_deepcopy,
	            "->\n"
	            "#r{A deep copy of @this object}"),
	TYPE_METHOD(STR___assign__, &object_assign,
	            "(other)->\n"
	            "Assigns @other to @this and"),
	TYPE_METHOD(STR___moveassign__, &object_moveassign,
	            "(other)->\n"
	            "Move-assign @other to @this and"),
	TYPE_METHOD(STR___str__, &object_dostr,
	            "->?Dstring\n"
	            "#r{@this converted to a ?Dstring}"),
	TYPE_METHOD(STR___repr__, &object_dorepr,
	            "->?Dstring\n"
	            "#r{The ?Dstring representation of @this}"),
	TYPE_METHOD(STR___bool__, &object_bool,
	            "->?Dbool\n"
	            "#r{The ?Dbool value of @this}"),
	TYPE_METHOD(STR___call__, &object_call,
	            "(args:?DTuple)->\n"
	            "Call @this using the given @args ?DTuple"),
	TYPE_METHOD(STR___thiscall__, &object_thiscall,
	            "(this_arg,args:?DTuple)->\n"
	            "Do a this-call on @this using the given @this_arg and @args ?DTuple"),
	TYPE_METHOD(STR___hash__, &object_hash,
	            "->?Dint\n"
	            "#r{The hash-value of @this}"),
	TYPE_METHOD(STR___int__, &object_int,
	            "->?Dint\n"
	            "#r{The integer-value of @this}"),
	TYPE_METHOD(STR___inv__, &object_inv,
	            "->\n"
	            "#r{The result of ${this.operator ~ ()}}"),
	TYPE_METHOD(STR___pos__, &object_pos,
	            "->\n"
	            "#r{The result of ${this.operator + ()}}"),
	TYPE_METHOD(STR___neg__, &object_neg,
	            "->\n"
	            "#r{The result of ${this.operator - ()}}"),
	TYPE_METHOD(STR___add__, &object_add,
	            "(other)->\n"
	            "#r{The result of ${this.operator + (other)}}"),
	TYPE_METHOD(STR___sub__, &object_sub,
	            "(other)->\n"
	            "#r{The result of ${this.operator - (other)}}"),
	TYPE_METHOD(STR___mul__, &object_mul,
	            "(other)->\n"
	            "#r{The result of ${this.operator * (other)}}"),
	TYPE_METHOD(STR___div__, &object_div,
	            "(other)->\n"
	            "#r{The result of ${this.operator / (other)}}"),
	TYPE_METHOD(STR___mod__, &object_mod,
	            "(other)->\n"
	            "#r{The result of ${this.operator % (other)}}"),
	TYPE_METHOD(STR___shl__, &object_shl,
	            "(other)->\n"
	            "#r{The result of ${this.operator << (other)}}"),
	TYPE_METHOD(STR___shr__, &object_shr,
	            "(other)->\n"
	            "#r{The result of ${this.operator >> (other)}}"),
	TYPE_METHOD(STR___and__, &object_and,
	            "(other)->\n"
	            "#r{The result of ${this.operator & (other)}}"),
	TYPE_METHOD(STR___or__, &object_or,
	            "(other)->\n"
	            "#r{The result of ${this.operator | (other)}}"),
	TYPE_METHOD(STR___xor__, &object_xor,
	            "(other)->\n"
	            "#r{The result of ${this.operator ^ (other)}}"),
	TYPE_METHOD(STR___pow__, &object_pow,
	            "(other)->\n"
	            "#r{The result of ${this.operator ** (other)}}"),
	TYPE_METHOD(STR___eq__, &object_eq,
	            "(other)->\n"
	            "#r{The result of ${this.operator == (other)}}"),
	TYPE_METHOD(STR___ne__, &object_ne,
	            "(other)->\n"
	            "#r{The result of ${this.operator != (other)}}"),
	TYPE_METHOD(STR___lo__, &object_lo,
	            "(other)->\n"
	            "#r{The result of ${this.operator < (other)}}"),
	TYPE_METHOD(STR___le__, &object_le,
	            "(other)->\n"
	            "#r{The result of ${this.operator <= (other)}}"),
	TYPE_METHOD(STR___gr__, &object_gr,
	            "(other)->\n"
	            "#r{The result of ${this.operator > (other)}}"),
	TYPE_METHOD(STR___ge__, &object_ge,
	            "(other)->\n"
	            "#r{The result of ${this.operator >= (other)}}"),
	TYPE_METHOD(STR___size__, &object_size,
	            "->\n"
	            "#r{The result of ${this.operator ## ()}}"),
	TYPE_METHOD(STR___contains__, &object_contains,
	            "(item)->\n"
	            "#r{The result of ${this.operator contains (item)}}"),
	TYPE_METHOD(STR___getitem__, &object_getitem,
	            "(index)->\n"
	            "#r{The result of ${this.operator [] (index)}}"),
	TYPE_METHOD(STR___delitem__, &object_delitem,
	            "(index)\n"
	            "Invokes ${this.operator del[] (index)}"),
	TYPE_METHOD(STR___setitem__, &object_setitem,
	            "(index,value)->\n"
	            "#r{Always re-returned @value}\n"
	            "Invokes ${this.operator []= (index, value)}"),
	TYPE_METHOD(STR___getrange__, &object_getrange,
	            "(start,end)->\n"
	            "#r{The result of ${this.operator [:] (start, end)}}"),
	TYPE_METHOD(STR___delrange__, &object_delrange,
	            "(start,end)\n"
	            "Invokes ${this.operator del[:] (start, end)}"),
	TYPE_METHOD(STR___setrange__, &object_setrange,
	            "(start,end,value)->\n"
	            "#r{Always re-returned @value}\n"
	            "Invokes ${this.operator [:]= (start, end, value)}"),
	TYPE_METHOD(STR___iterself__, &object_iterself,
	            "->\n"
	            "#r{The result of ${this.operator iter()}}"),
	TYPE_METHOD(STR___iternext__, &object_iternext,
	            "->\n"
	            "#r{The result of ${this.operator next()}}"),
	TYPE_METHOD(STR___getattr__, &object_getattr,
	            "(name:?Dstring)->\n"
	            "#r{The result of ${this.operator . (name)}}"),
	TYPE_METHOD(STR___callattr__, &object_callattr,
	            "(name:?Dstring,args!)->\n"
	            "#r{The result of ${this.operator . (name)(args!)}}"),
	TYPE_METHOD(STR___hasattr__, &object_hasattr,
	            "(name:?Dstring)->?Dbool\n"
	            "Check if @this object provides an attribute @name, returning ?t or ?f indicative of this"),
	TYPE_METHOD(STR___delattr__, &object_delattr,
	            "(name:?Dstring)\n"
	            "Invokes ${this.operator del . (name)}"),
	TYPE_METHOD(STR___setattr__, &object_setattr,
	            "(name:?Dstring,value)\n"
	            "#r{Always re-returned @value}\n"
	            "Invokes ${this.operator .= (name, value)}"),
	TYPE_METHOD(STR___enumattr__, &object_enumattr,
	            "()->?S?DAttribute\n"
	            "#r{Same as ${deemon.enumattr(this)}}"),
	TYPE_METHOD(STR___format__, &object_format_method,
	            "(format:?Dstring)->?Dstring\n"
	            "Format @this object. (s.a. ?Aformat?Dstring)"),
#ifndef CONFIG_NO_DEEMON_100_COMPAT
	/* Aliases for backwards compatibility with deemon < v200 */
	TYPE_METHOD("__iterself__",    &object_iterself, "->\nDeprecated alias for ?#__iter__"),
	TYPE_METHOD("__iternext__",    &object_iternext, "->\nDeprecated alias for ?#__next__"),
	/* Deprecated function for backwards compatibility with deemon < v200 */
	TYPE_METHOD("__move__",        &object_copy, "->\nDeprecated alias for ?#__copy__"),
	TYPE_METHOD("__lt__",          &object_lo, "(other)->\nDeprecated alias for ?#__lo__"),
	TYPE_METHOD("__gt__",          &object_gr, "(other)->\nDeprecated alias for ?#__gr__"),
	TYPE_METHOD("__not__",         &object_not, "->?Dbool\nDeprecated alias for ${!this}"),
	TYPE_METHOD("__is__",          &object_is, "(tp:?DType)->?Dbool\n(tp:?N)->?Dbool\nDeprecated alias for ${this is tp}"),
	TYPE_METHOD("__deepequals__",  &object_eq, "(other)->\nDeprecated alias for ?#__eq__"),
	TYPE_METHOD("__inc__",         &object_inc, "->\nDeprecated alias for ${({ local temp = this; ++temp; if (temp !== this) this := temp; this; })}"),
	TYPE_METHOD("__dec__",         &object_dec, "->\nDeprecated alias for ${({ local temp = this; --temp; if (temp !== this) this := temp; this; })}"),
	TYPE_METHOD("__incpost__",     &object_incpost, "->\nDeprecated alias for ${({ local res = copy this; local temp = this; ++temp; if (temp !== this) this := temp; res; })}"),
	TYPE_METHOD("__decpost__",     &object_decpost, "->\nDeprecated alias for ${({ local res = copy this; local temp = this; --temp; if (temp !== this) this := temp; res; })}"),
	TYPE_METHOD("__iadd__",        &object_iadd, "(other)->\nDeprecated alias for ${({ local temp = this; temp += other; if (temp !== this) this := temp; this; })}"),
	TYPE_METHOD("__isub__",        &object_isub, "(other)->\nDeprecated alias for ${({ local temp = this; temp -= other; if (temp !== this) this := temp; this; })}"),
	TYPE_METHOD("__imul__",        &object_imul, "(other)->\nDeprecated alias for ${({ local temp = this; temp *= other; if (temp !== this) this := temp; this; })}"),
	TYPE_METHOD("__idiv__",        &object_idiv, "(other)->\nDeprecated alias for ${({ local temp = this; temp /= other; if (temp !== this) this := temp; this; })}"),
	TYPE_METHOD("__imod__",        &object_imod, "(other)->\nDeprecated alias for ${({ local temp = this; temp %= other; if (temp !== this) this := temp; this; })}"),
	TYPE_METHOD("__ishl__",        &object_ishl, "(other)->\nDeprecated alias for ${({ local temp = this; temp <<= other; if (temp !== this) this := temp; this; })}"),
	TYPE_METHOD("__ishr__",        &object_ishr, "(other)->\nDeprecated alias for ${({ local temp = this; temp >>= other; if (temp !== this) this := temp; this; })}"),
	TYPE_METHOD("__iand__",        &object_iand, "(other)->\nDeprecated alias for ${({ local temp = this; temp &= other; if (temp !== this) this := temp; this; })}"),
	TYPE_METHOD("__ior__",         &object_ior,  "(other)->\nDeprecated alias for ${({ local temp = this; temp |= other; if (temp !== this) this := temp; this; })}"),
	TYPE_METHOD("__ixor__",        &object_ixor, "(other)->\nDeprecated alias for ${({ local temp = this; temp ^= other; if (temp !== this) this := temp; this; })}"),
	TYPE_METHOD("__ipow__",        &object_ipow, "(other)->\nDeprecated alias for ${({ local temp = this; temp **= other; if (temp !== this) this := temp; this; })}"),
#endif /* !CONFIG_NO_DEEMON_100_COMPAT */
	TYPE_METHOD_END
};




PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
object_class_get(DeeObject *__restrict self) {
	return_reference((DeeObject *)DeeObject_Class(self));
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
object_id_get(DeeObject *__restrict self) {
	return DeeInt_NewUIntptr(DeeObject_Id(self));
}

PRIVATE DEFINE_CLSPROPERTY(object_id_get_cobj, &DeeObject_Type, &object_id_get, NULL, NULL);
PRIVATE struct type_member tpconst object_class_members[] = {
	TYPE_MEMBER_CONST_DOC("id", &object_id_get_cobj,
	                      "Alias for ?#{i:id} to speed up expressions such as ${Object.id}"),
	TYPE_MEMBER_END
};


INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
instance_get_itable(DeeObject *__restrict self);

/* Runtime-versions of compiler-intrinsic standard attributes. */
PRIVATE struct type_getset tpconst object_getsets[] = {
	TYPE_GETTER_F(STR_this, &DeeObject_NewRef,
	              METHOD_FCONSTCALL,
	              "Always re-return @this object"),
	TYPE_GETTER_F(STR_class, &object_class_get,
	              METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	              "->?DType\n"
	              "Returns the class of @this Type, which is usually identical to "
	              /**/ "?#type, however in the case of a super-proxy, the viewed "
	              /**/ "Type is returned, rather than the actual Type."),
	TYPE_GETTER_F("super", &DeeSuper_Of,
	              METHOD_FCONSTCALL,
	              "->?DSuper\n"
	              "Returns a view for the super-instance of @this object"),
	TYPE_GETTER_F("__itable__", &instance_get_itable,
	              METHOD_FCONSTCALL,
	              "->?X2?S?O?AObjectTable?Ert:ClassDescriptor\n"
	              "Returns an indexable sequence describing the instance object "
	              /**/ "table, as referenced by ?Aaddr?AAttribute?Ert:{ClassDescriptor}.\n"
	              "For non-user-defined classes (aka. when ${this.class.__isclass__} is ?f), "
	              /**/ "an empty sequence is returned.\n"
	              "The class-attribute table can be accessed through ?A__ctable__?DType."),

	/* Helper function: `foo.id' returns a unique id for any object. */
	TYPE_GETTER_F("id", &object_id_get,
	              /* This one isn't CONSTCALL because IDs can change if a constant
	               * is serialized and deserialized (as would be the case when building
	               * and later re-loading a .dec file). Even IDs of built-in constants
	               * like "none", "true", "false", ... can differ if ASLR causes deemon
	               * to be loaded to different addresses between multiple runs. */
	              METHOD_FNOREFESCAPE,
	              "->?Dint\n"
	              "Returns a unique id identifying @this specific object instance"),

	/* Utility function: Return the size of a given object (in bytes) */
	TYPE_GETTER_F("__sizeof__", &object_sizeof,
	              METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	              "->?Dint\n"
	              "Return the size of @this object in bytes."),
	TYPE_GETSET_END
};

PRIVATE struct type_member tpconst object_members[] = {
	TYPE_MEMBER_FIELD_DOC("type", STRUCT_OBJECT, offsetof(DeeObject, ob_type),
	                      "->?DType\n"
	                      "The type of @this object (same as ${type this})"),
	TYPE_MEMBER_FIELD_DOC("__refcnt__", STRUCT_CONST | STRUCT_SIZE_T, offsetof(DeeObject, ob_refcnt),
	                      "->?Dint\n"
	                      "The number of references currently existing for @this object"),
	TYPE_MEMBER_END
};

PRIVATE struct type_operator const object_operators[] = {
	TYPE_OPERATOR_FLAGS(OPERATOR_0000_CONSTRUCTOR, METHOD_FCONSTCALL),
	TYPE_OPERATOR_FLAGS(OPERATOR_0006_STR, METHOD_FCONSTCALL | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0007_REPR, METHOD_FCONSTCALL | METHOD_FNOREFESCAPE),
};

INTDEF int DCALL none_i1(void *UNUSED(b));
INTDEF int DCALL none_i2(void *UNUSED(b), void *UNUSED(c));

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
	                         "${"
	                         /**/ "operator str(): string {\n"
	                         /**/ "	return str type this;\n"
	                         /**/ "}"
	                         "}\n"

	                         "repr->\n"
	                         "Returns $\"Object()\" if this is an exact instance of ?.\n"
	                         "${"
	                         /**/ "operator repr(): string {\n"
	                         /**/ "	if (type(this) === Object)\n"
	                         /**/ "		return \"Object()\";\n"
	                         /**/ "	throw Error.RuntimeError.NotImplemented();\n"
	                         /**/ "}"
	                         "}"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FNAMEOBJECT | TP_FABSTRACT,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ NULL,    /* No base */
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&none_i1,
				/* .tp_copy_ctor = */ (dfunptr_t)&none_i2,
				/* .tp_deep_ctor = */ (dfunptr_t)&none_i2,
				/* .tp_any_ctor  = */ (dfunptr_t)&object_any_ctor,
				TYPE_FIXED_ALLOCATOR_S(DeeObject)
			}
		},
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&object_str,
		/* .tp_repr = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&object_repr,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ object_methods,
	/* .tp_getsets       = */ object_getsets,
	/* .tp_members       = */ object_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ object_class_members,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call_kw       = */ NULL,
	/* .tp_mro           = */ NULL,
	/* .tp_operators     = */ object_operators,
	/* .tp_operators_size= */ COMPILER_LENOF(object_operators)
};


INTERN WUNUSED NONNULL((1)) int DCALL
type_ctor(DeeTypeObject *__restrict self) {
	/* Simply initialize everything to ZERO and set the HEAP flag. */
	bzero((void *)&self->tp_name,
	      sizeof(DeeTypeObject) -
	      offsetof(DeeTypeObject, tp_name));
	self->tp_flags |= TP_FHEAP;
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeStringObject *DCALL
type_str(DeeTypeObject *__restrict self) {
	if (self->tp_flags & TP_FNAMEOBJECT) {
		DREF DeeStringObject *result;
		result = COMPILER_CONTAINER_OF(self->tp_name,
		                               DeeStringObject,
		                               s_str);
		Dee_Incref(result);
		return result;
	}
	if likely(self->tp_name)
		return (DREF DeeStringObject *)DeeString_New(self->tp_name);
	return_reference_(&str_Type);
}

INTERN WUNUSED NONNULL((1, 2)) dssize_t DCALL
DeeType_Print(DeeTypeObject *__restrict self, dformatprinter printer, void *arg) {
	if (self->tp_flags & TP_FNAMEOBJECT) {
		DREF DeeStringObject *nameob;
		nameob = COMPILER_CONTAINER_OF(self->tp_name,
		                               DeeStringObject,
		                               s_str);
		return DeeString_PrintUtf8((DeeObject *)nameob, printer, arg);
	}
	if likely(self->tp_name)
		return DeeFormat_PrintStr(printer, arg, self->tp_name);
	return DeeString_PrintAscii(&str_Type, printer, arg);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeStringObject *DCALL
type_repr(DeeTypeObject *__restrict self) {
	DREF DeeObject *mod;
	DREF DeeStringObject *result;
	DeeStringObject *modname;
	if unlikely(!self->tp_name)
		goto fallback;
	mod = DeeType_GetModule(self);
	if (!mod)
		goto fallback;
	modname = ((DeeModuleObject *)mod)->mo_name;
	result  = (DREF DeeStringObject *)DeeString_Newf("%k.%s", modname, self->tp_name);
	Dee_Decref(mod);
	return result;
fallback:
	return type_str(self);
}

PRIVATE WUNUSED NONNULL((1, 2)) dssize_t DCALL
type_printrepr(DeeTypeObject *__restrict self, dformatprinter printer, void *arg) {
	dssize_t result, temp;
	DREF DeeModuleObject *mod;
	char const *name;
	mod = (DREF DeeModuleObject *)DeeType_GetModule(self);
	if (!mod)
		goto fallback;
	result = DeeString_PrintUtf8((DeeObject *)mod->mo_name, printer, arg);
	Dee_Decref(mod);
	if unlikely(result < 0)
		goto done;
	name = self->tp_name;
	if (name == NULL)
		name = "<anonymous type>";
	temp = DeeFormat_Printf(printer, arg, ".%s", name);
	if unlikely(temp < 0)
		goto err;
	result += temp;
done:
	return result;
err:
	return temp;
fallback:
	return DeeType_Print(self, printer, arg);
}


INTDEF NONNULL((1)) void DCALL class_fini(DeeTypeObject *__restrict self);
INTDEF NONNULL((1, 2)) void DCALL class_visit(DeeTypeObject *__restrict self, dvisit_t proc, void *arg);
INTDEF NONNULL((1)) void DCALL class_clear(DeeTypeObject *__restrict self);
INTDEF NONNULL((1)) void DCALL class_pclear(DeeTypeObject *__restrict self, unsigned int gc_priority);

INTDEF WUNUSED NONNULL((1)) DeeTypeObject *DCALL /* from "../runtime/operator.c" */
DeeType_GetOpClassOrigin(DeeTypeObject *__restrict self, uint16_t oi_class);
#define DeeType_GetMathOrigin(self)   DeeType_GetOpClassOrigin(self, offsetof(DeeTypeObject, tp_math))
#define DeeType_GetCmpOrigin(self)    DeeType_GetOpClassOrigin(self, offsetof(DeeTypeObject, tp_cmp))
#define DeeType_GetSeqOrigin(self)    DeeType_GetOpClassOrigin(self, offsetof(DeeTypeObject, tp_seq))
#define DeeType_GetWithOrigin(self)   DeeType_GetOpClassOrigin(self, offsetof(DeeTypeObject, tp_with))
#define DeeType_GetBufferOrigin(self) DeeType_GetOpClassOrigin(self, offsetof(DeeTypeObject, tp_buffer))


PRIVATE NONNULL((1)) void DCALL
type_fini(DeeTypeObject *__restrict self) {
	/* Clear weak references and check for revival. */
	Dee_weakref_fini(&self->tp_module);
	weakref_support_fini(self);
	if (Dee_IncrefIfNotZero(self))
		return;
	ASSERTF(self->tp_flags & TP_FHEAP,
	        "Non heap-allocated type %s is being destroyed (This shouldn't happen)",
	        self->tp_name);

	/* Free the sequence method cache (if it was allocated and belongs to this type) */
	if (self->tp_seq && self->tp_seq->_tp_seqcache && (DeeType_GetSeqOrigin(self) == self))
		Dee_type_seq_cache_destroy(self->tp_seq->_tp_seqcache);

	/* Finalize class data if the type is actually a user-defined class. */
	if (DeeType_IsClass(self))
		class_fini(self);

	/* Finalize the type's member caches. */
	Dee_membercache_fini(&self->tp_cache);
	Dee_membercache_fini(&self->tp_class_cache);

	/* Cleanup extra MRO types */
	if (self->tp_mro != NULL) {
		size_t i;
		for (i = 0; self->tp_mro[i] != NULL; ++i)
			Dee_Decref_unlikely(self->tp_mro[i]);
		Dee_Free((void *)self->tp_mro);
	}

	/* Cleanup name & doc objects should those have been used. */
	if (self->tp_flags & TP_FNAMEOBJECT)
		Dee_XDecref_likely(COMPILER_CONTAINER_OF(self->tp_name, DeeStringObject, s_str));
	if (self->tp_flags & TP_FDOCOBJECT)
		Dee_XDecref_likely(COMPILER_CONTAINER_OF(self->tp_doc, DeeStringObject, s_str));
	Dee_XDecref_unlikely(self->tp_base);
}


PRIVATE NONNULL((1, 2)) void DCALL
type_visit(DeeTypeObject *__restrict self,
           dvisit_t proc, void *arg) {
	if (DeeType_IsClass(self))
		class_visit(self, proc, arg);
	if (self->tp_mro != NULL) {
		size_t i;
		for (i = 0; self->tp_mro[i] != NULL; ++i)
			Dee_Visit(self->tp_mro[i]);
	}
	Dee_XVisit(self->tp_base);
}

PRIVATE NONNULL((1)) void DCALL
type_clear(DeeTypeObject *__restrict self) {
	if (DeeType_IsClass(self))
		class_clear(self);
}

PRIVATE NONNULL((1)) void DCALL
type_pclear(DeeTypeObject *__restrict self,
            unsigned int gc_priority) {
	if (DeeType_IsClass(self))
		class_pclear(self, gc_priority);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
type_baseof(DeeTypeObject *self, size_t argc,
            DeeObject *const *argv, DeeObject *kw) {
	DeeTypeObject *other;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__other, "o:baseof", &other))
		goto err;
	if (!DeeType_Check((DeeObject *)other))
		return_false;
	Dee_return_reference((DeeObject *)&Dee_FalseTrue[DeeType_Extends(other, self)]);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
type_extends(DeeTypeObject *self, size_t argc,
                 DeeObject *const *argv, DeeObject *kw) {
	DeeTypeObject *other;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__other, "o:extends", &other))
		goto err;
	Dee_return_reference((DeeObject *)&Dee_FalseTrue[DeeType_Extends(self, other)]);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
type_implements(DeeTypeObject *self, size_t argc,
                DeeObject *const *argv, DeeObject *kw) {
	DeeTypeObject *other;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__other, "o:implements", &other))
		goto err;
	Dee_return_reference((DeeObject *)&Dee_FalseTrue[DeeType_Implements(self, other)]);
err:
	return NULL;
}


PRIVATE ATTR_COLD NONNULL((1)) int DCALL
err_init_var_type(DeeTypeObject *__restrict self) {
	return DeeError_Throwf(&DeeError_TypeError,
	                       "Cannot instantiate variable-length type %k",
	                       self);
}

PRIVATE ATTR_COLD NONNULL((1)) int DCALL
err_missing_mandatory_init(DeeTypeObject *__restrict self) {
	return DeeError_Throwf(&DeeError_TypeError,
	                       "Missing initializer for mandatory base-type %k",
	                       self);
}

PRIVATE char const str_shared_ctor_failed[] = "Constructor of shared object failed\n";
INTDEF NONNULL((1)) void DCALL
instance_clear_members(struct instance_desc *__restrict self, uint16_t size);

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
type_new_raw(DeeTypeObject *__restrict self) {
	DREF DeeObject *result;
	DeeTypeObject *first_base;
	if unlikely(self->tp_flags & TP_FVARIABLE) {
		err_init_var_type(self);
		goto err;
	}
	result = DeeType_AllocInstance(self);
	if unlikely(!result)
		goto err;
	DeeObject_Init(result, self);

	/* Search for the first non-class base. */
	first_base = self;
	while (DeeType_IsClass(first_base)) {
		struct class_desc *desc        = DeeClass_DESC(first_base);
		struct instance_desc *instance = DeeInstance_DESC(desc, result);
		Dee_atomic_rwlock_init(&instance->id_lock);
		bzeroc(instance->id_vtab,
		       desc->cd_desc->cd_imemb_size,
		       sizeof(DREF DeeObject *));
		first_base = DeeType_Base(first_base);
		if (!first_base)
			break;
	}

	/* Instantiate non-base types. */
	if (!first_base || first_base == &DeeObject_Type) {
done:
		if (self->tp_flags & TP_FGC)
			result = DeeGC_Track(result);
		return result;
	}
	if (first_base->tp_init.tp_alloc.tp_ctor) {
		/* Invoke the mandatory base-type constructor. */
invoke_base_ctor:
		if unlikely((*first_base->tp_init.tp_alloc.tp_ctor)(result))
			goto err_r;
		goto done;
	}
	if (first_base->tp_init.tp_alloc.tp_any_ctor) {
		/* Invoke the mandatory base-type constructor. */
invoke_base_any_ctor:
		if unlikely((*first_base->tp_init.tp_alloc.tp_any_ctor)(result, 0, NULL))
			goto err_r;
		goto done;
	}
	if (first_base->tp_init.tp_alloc.tp_any_ctor_kw) {
		/* Invoke the mandatory base-type constructor. */
invoke_base_any_ctor_kw:
		if unlikely((*first_base->tp_init.tp_alloc.tp_any_ctor_kw)(result, 0, NULL, NULL))
			goto err_r;
		goto done;
	}
	if (DeeType_InheritConstructors(first_base)) {
		if (first_base->tp_init.tp_alloc.tp_ctor)
			goto invoke_base_ctor;
		if (first_base->tp_init.tp_alloc.tp_any_ctor)
			goto invoke_base_any_ctor;
		if (first_base->tp_init.tp_alloc.tp_any_ctor_kw)
			goto invoke_base_any_ctor_kw;
	}
	err_missing_mandatory_init(first_base);
	goto err_r;
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
	DeeType_FreeInstance(self, result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
set_basic_member(DeeTypeObject *__restrict tp_self,
                 DeeObject *__restrict self,
                 DeeStringObject *__restrict member_name,
                 DeeObject *__restrict value) {
	int temp;
	DeeTypeObject *iter   = tp_self;
	char const *attr_name = DeeString_STR(member_name);
	Dee_hash_t attr_hash  = DeeString_Hash((DeeObject *)member_name);
	if ((temp = DeeType_SetBasicCachedAttrStringHash(tp_self, self, attr_name, attr_hash, value)) <= 0)
		goto done_temp;
	do {
		if (DeeType_IsClass(iter)) {
			struct class_attribute *attr;
			struct instance_desc *instance;
			struct class_desc *desc;
			DREF DeeObject *old_value;
			attr = DeeType_QueryAttributeHash(tp_self, iter,
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
			Dee_instance_desc_lock_write(instance);
			old_value = instance->id_vtab[attr->ca_addr];
			instance->id_vtab[attr->ca_addr] = value;
			Dee_instance_desc_lock_endwrite(instance);
			if unlikely(old_value)
				Dee_Decref(old_value);
			return 0;
		}
		if (iter->tp_members &&
		    (temp = DeeType_SetMemberAttrStringHash(tp_self, iter, self, attr_name, attr_hash, value)) <= 0)
			goto done_temp;
next_base:
		;
	} while ((iter = DeeType_Base(iter)) != NULL);
	return DeeError_Throwf(&DeeError_AttributeError,
	                       "Could not find member %k in %k, or its bases",
	                       member_name, tp_self);
done_temp:
	return temp;
}

PRIVATE WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
set_private_basic_member(DeeTypeObject *__restrict tp_self,
                         DeeObject *__restrict self,
                         DeeStringObject *__restrict member_name,
                         DeeObject *__restrict value) {
	int temp;
	char const *attr_name = DeeString_STR(member_name);
	Dee_hash_t attr_hash  = DeeString_Hash((DeeObject *)member_name);
	if (DeeType_IsClass(tp_self)) {
		struct class_attribute *attr;
		struct instance_desc *instance;
		struct class_desc *desc = DeeClass_DESC(tp_self);
		DREF DeeObject *old_value;
		attr = DeeClassDesc_QueryInstanceAttributeStringHash(desc,
		                                                     attr_name,
		                                                     attr_hash);
		if (!attr)
			goto not_found;
		if (attr->ca_flag & (CLASS_ATTRIBUTE_FCLASSMEM |
		                     CLASS_ATTRIBUTE_FGETSET))
			goto not_found;
		instance = DeeInstance_DESC(desc, self);
		Dee_Incref(value);
		Dee_instance_desc_lock_write(instance);
		old_value = instance->id_vtab[attr->ca_addr];
		instance->id_vtab[attr->ca_addr] = value;
		Dee_instance_desc_lock_endwrite(instance);
		if unlikely(old_value)
			Dee_Decref(old_value);
		return 0;
	}
	if (tp_self->tp_members &&
	    (temp = DeeType_SetMemberAttrStringHash(tp_self, tp_self, self, attr_name, attr_hash, value)) <= 0)
		goto done_temp;
not_found:
	return DeeError_Throwf(&DeeError_AttributeError,
	                       "Could not find member %k in %k",
	                       member_name, tp_self);
done_temp:
	return temp;
}

PRIVATE WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
unpack_init_info(DeeObject *__restrict info,
                 DREF DeeObject **__restrict p_init_fields,
                 DREF DeeObject **__restrict p_init_args,
                 DREF DeeObject **__restrict p_init_kw) {
	DREF DeeObject *fields_args_kw[3];
	size_t n_args = DeeObject_UnpackEx(info, 1, 3, fields_args_kw);
	if unlikely(n_args == (size_t)-1)
		goto err;
	ASSERT(n_args >= 1 && n_args <= 3);
	if (n_args < 3) {
		fields_args_kw[2] = NULL;
	} else if (DeeNone_Check(fields_args_kw[2])) {
		Dee_DecrefNokill(fields_args_kw[2]);
		fields_args_kw[2] = NULL;
	}
	if (n_args < 2) {
		Dee_Incref(Dee_EmptyTuple);
		fields_args_kw[1] = Dee_EmptyTuple;
	} else if (DeeNone_Check(fields_args_kw[1])) {
		Dee_DecrefNokill(fields_args_kw[1]);
		Dee_Incref(Dee_EmptyTuple);
		fields_args_kw[1] = Dee_EmptyTuple;
	}
	if (DeeNone_Check(fields_args_kw[0])) {
		Dee_DecrefNokill(fields_args_kw[0]);
		fields_args_kw[0] = NULL;
	}
	*p_init_fields = fields_args_kw[0];
	*p_init_args   = fields_args_kw[1];
	*p_init_kw     = fields_args_kw[2];
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
unpack_init_info1(DeeObject *__restrict info) {
	DREF DeeObject *init_fields;
	DREF DeeObject *init_argv;
	DREF DeeObject *init_kw;
	if unlikely(unpack_init_info(info, &init_fields, &init_argv, &init_kw))
		goto err;
	Dee_XDecref(init_kw);
	Dee_Decref(init_argv);
	if (!init_fields)
		init_fields = ITER_DONE;
	return init_fields;
err:
	return NULL;
}

INTDEF WUNUSED NONNULL((1, 2)) int DCALL
type_invoke_base_constructor(DeeTypeObject *__restrict tp_self,
                             DeeObject *__restrict self, size_t argc,
                             DeeObject *const *argv, DeeObject *kw);

struct assign_init_fields_data {
	DeeTypeObject *aifd_tp_self;
	DeeObject     *aifd_self;
};

PRIVATE WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL
assign_init_fields_foreach(void *arg, DeeObject *key, DeeObject *value) {
	struct assign_init_fields_data *me = (struct assign_init_fields_data *)arg;
	Dee_ssize_t result = DeeObject_AssertTypeExact(key, &DeeString_Type);
	if likely(result == 0) {
		result = set_basic_member(me->aifd_tp_self, me->aifd_self,
		                          (DeeStringObject *)key, value);
	}
	return result;
}
PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
assign_init_fields(DeeTypeObject *__restrict tp_self,
                   DeeObject *__restrict self,
                   DeeObject *__restrict fields) {
	struct assign_init_fields_data data;
	data.aifd_tp_self = tp_self;
	data.aifd_self    = self;
	return (int)DeeObject_ForeachPair(fields, &assign_init_fields_foreach, &data);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
type_new_extended(DeeTypeObject *self,
                  DeeObject *initializer) {
	DREF DeeObject *result, *init_info;
	int temp;
	DREF DeeObject *init_fields, *init_args, *init_kw;
	DeeTypeObject *first_base, *iter;
	if unlikely(self->tp_flags & TP_FVARIABLE) {
		err_init_var_type(self);
		goto err;
	}
	result = DeeType_AllocInstance(self);
	if unlikely(!result)
		goto err;
	DeeObject_Init(result, self);

	/* Search for the first non-class base. */
	first_base = self;
	while (DeeType_IsClass(first_base)) {
		struct class_desc *desc        = DeeClass_DESC(first_base);
		struct instance_desc *instance = DeeInstance_DESC(desc, result);
		Dee_atomic_rwlock_init(&instance->id_lock);
		bzeroc(instance->id_vtab,
		       desc->cd_desc->cd_imemb_size,
		       sizeof(DREF DeeObject *));
		first_base = DeeType_Base(first_base);
		if (!first_base)
			break;
	}

	/* Instantiate non-base types. */
	if (!first_base || first_base == &DeeObject_Type)
		goto done_fields;

	/* {(Type, ({(string, Object)...}, Tuple))...} */
	/* {(Type, ({(string, Object)...}, Tuple, Mapping))...} */
	init_info = DeeObject_TryGetItem(initializer, (DeeObject *)first_base);
	if unlikely(!init_info)
		goto err_r;
	if (init_info == ITER_DONE) {
		Dee_Incref(Dee_EmptyTuple);
		init_args   = Dee_EmptyTuple;
		init_fields = NULL;
		init_kw     = NULL;
	} else {
		temp = unpack_init_info(init_info, &init_fields, &init_args, &init_kw);
		Dee_Decref(init_info);
		if unlikely(temp)
			goto err_r;
	}

	/* Invoke the mandatory base-type constructor. */
	temp = type_invoke_base_constructor(first_base, result,
	                                    DeeTuple_SIZE(init_args),
	                                    DeeTuple_ELEM(init_args),
	                                    init_kw);
	Dee_XDecref(init_kw);
	Dee_Decref(init_args);
	if likely(!temp && init_fields)
		temp = assign_init_fields(first_base, result, init_fields);
	Dee_XDecref(init_fields);
	if unlikely(temp)
		goto err_r_firstbase;
done_fields:

	/* Fill in all of the fields of non-first-base types. */
	iter = self;
	do {
		if (iter == first_base)
			continue;
		init_info = DeeObject_TryGetItem(initializer, (DeeObject *)iter);
		if unlikely(!init_info)
			goto err_r_firstbase;
		if (init_info == ITER_DONE)
			continue;
		if (DeeNone_Check(init_info)) {
			Dee_DecrefNokill(init_info);
			continue;
		}
		init_fields = unpack_init_info1(init_info);
		Dee_Decref(init_info);
		if (init_fields == ITER_DONE)
			continue;
		if unlikely(!init_fields)
			goto err_r_firstbase;
		temp = assign_init_fields(iter, result, init_fields);
		Dee_Decref(init_fields);
		if unlikely(temp)
			goto err_r_firstbase;
	} while ((iter = DeeType_Base(iter)) != NULL);
	if (self->tp_flags & TP_FGC)
		result = DeeGC_Track(result);
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
	DeeType_FreeInstance(self, result);
err:
	return NULL;
}

struct assign_private_basic_members_data {
	DeeTypeObject *apbmd_tp_self;
	DeeObject     *apbmd_self;
};

PRIVATE WUNUSED NONNULL((1, 2, 3)) Dee_ssize_t DCALL
assign_private_basic_members_foreach(void *arg, DeeObject *key, DeeObject *value) {
	struct assign_private_basic_members_data *me = (struct assign_private_basic_members_data *)arg;
	Dee_ssize_t result = DeeObject_AssertTypeExact(key, &DeeString_Type);
	if likely(result == 0) {
		result = set_private_basic_member(me->apbmd_tp_self, me->apbmd_self,
		                                  (DeeStringObject *)key, value);
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
assign_private_basic_members(DeeTypeObject *__restrict tp_self,
                             DeeObject *__restrict self,
                             DeeObject *__restrict fields) {
	struct assign_private_basic_members_data data;
	data.apbmd_tp_self = tp_self;
	data.apbmd_self    = self;
	return (int)DeeObject_ForeachPair(fields, &assign_private_basic_members_foreach, &data);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
type_newinstance(DeeTypeObject *self, size_t argc,
                 DeeObject *const *argv, DeeObject *kw) {
	DREF DeeObject *result;
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
				if unlikely(set_private_basic_member(self, result,
				                                     kwds->kw_map[i].ke_name,
				                                     argv[kwds->kw_map[i].ke_index]))
					goto err_r;
			}
		} else {
			if unlikely(assign_private_basic_members(self, result, kw))
				goto err_r;
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

#define STR_getinstanceattr   (meth_getinstanceattr + 2)
#define STR_callinstanceattr  (meth_callinstanceattr + 0)
#define STR_hasinstanceattr   (meth_hasinstanceattr + 2)
#define STR_boundinstanceattr (meth_boundinstanceattr + 4)
#define STR_delinstanceattr   (meth_delinstanceattr + 2)
#define STR_setinstanceattr   (meth_setinstanceattr + 3)

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
type_getinstanceattr(DeeTypeObject *self, size_t argc,
                     DeeObject *const *argv, DeeObject *kw) {
	DeeObject *name;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__attr, meth_getinstanceattr, &name))
		goto err;
	if (DeeObject_AssertTypeExact(name, &DeeString_Type))
		goto err;
	return DeeType_GetInstanceAttr(self, name);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
type_callinstanceattr(DeeTypeObject *self, size_t argc,
                      DeeObject *const *argv, DeeObject *kw) {
	if unlikely(!argc) {
		err_invalid_argc_va(meth_callinstanceattr, argc, 1);
		goto err;
	}
	if (DeeObject_AssertTypeExact(argv[0], &DeeString_Type))
		goto err;
	return DeeType_CallInstanceAttrKw(self, argv[0], argc - 1, argv + 1, kw);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
type_hasinstanceattr(DeeTypeObject *self, size_t argc,
                     DeeObject *const *argv, DeeObject *kw) {
	DeeObject *name;
	int result;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__attr,
	                    meth_hasinstanceattr, &name))
		goto err;
	if (DeeObject_AssertTypeExact(name, &DeeString_Type))
		goto err;
	result = DeeType_HasInstanceAttr(self, name);
	if unlikely(result < 0)
		goto err;
	return_bool_(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
type_boundinstanceattr(DeeTypeObject *self, size_t argc,
                       DeeObject *const *argv, DeeObject *kw) {
	DeeObject *name;
	bool allow_missing = true;
	int result;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__attr_allow_missing,
	                    meth_boundinstanceattr, &name, &allow_missing))
		goto err;
	if (DeeObject_AssertTypeExact(name, &DeeString_Type))
		goto err;

	/* Instance attributes of types are always bound (because they're all wrappers) */
	result = DeeType_BoundInstanceAttr(self, name);
	if (result > 0)
		return_true;
	if (result == -1)
		goto err;
	if (allow_missing)
		return_false; /* Unknown attributes are unbound. */
	err_unknown_attribute(self, name, ATTR_ACCESS_GET);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
type_delinstanceattr(DeeTypeObject *self, size_t argc,
                     DeeObject *const *argv, DeeObject *kw) {
	DeeObject *name;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__attr,
	                    meth_delinstanceattr, &name))
		goto err;
	if (DeeObject_AssertTypeExact(name, &DeeString_Type))
		goto err;
	if (DeeType_DelInstanceAttr(self, name))
		goto err;
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
type_setinstanceattr(DeeTypeObject *self, size_t argc,
                     DeeObject *const *argv, DeeObject *kw) {
	DeeObject *name, *value;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__attr_value, meth_setattr, &name, &value))
		goto err;
	if (DeeObject_AssertTypeExact(name, &DeeString_Type))
		goto err;
	if (DeeType_SetInstanceAttr(self, name, value))
		goto err;
	return_reference_(value);
err:
	return NULL;
}


PRIVATE WUNUSED NONNULL((1, 2)) bool DCALL
impl_type_hasprivateattribute_string_hash(DeeTypeObject *__restrict self,
                                          char const *name_str,
                                          Dee_hash_t name_hash) {
	/* TODO: Lookup the attribute in the member cache, and
	 *       see which type is set as the declaring type! */
	if (DeeType_IsClass(self)) {
		struct class_desc *desc = DeeClass_DESC(self);
		if (DeeClassDesc_QueryInstanceAttributeStringHash(desc, name_str, name_hash) != NULL)
			goto found;
	} else {
		if (self->tp_methods &&
		    DeeType_HasMethodAttrStringHash(self, self, name_str, name_hash))
			goto found;
		if (self->tp_getsets &&
		    DeeType_HasGetSetAttrStringHash(self, self, name_str, name_hash))
			goto found;
		if (self->tp_members &&
		    DeeType_HasMemberAttrStringHash(self, self, name_str, name_hash))
			goto found;
	}
	return 0;
found:
	return 1;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
type_hasattribute(DeeTypeObject *self, size_t argc,
                  DeeObject *const *argv, DeeObject *kw) {
	DeeObject *name;
	char const *name_str;
	Dee_hash_t name_hash;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__attr, "o:hasattribute", &name))
		goto err;
	if (DeeObject_AssertTypeExact(name, &DeeString_Type))
		goto err;
	name_str  = DeeString_STR(name);
	name_hash = DeeString_Hash(name);
	if (!self->tp_attr) {
		DeeTypeMRO mro;
		DeeTypeObject *iter;
		if (DeeType_HasCachedAttrStringHash(self, name_str, name_hash))
			goto found;
		iter = self;
		DeeTypeMRO_Init(&mro, iter);
		for (;;) {
			if (DeeType_IsClass(iter)) {
				if (DeeType_QueryInstanceAttributeHash(self, iter, name, name_hash) != NULL)
					goto found;
			} else {
				if (iter->tp_methods &&
				    DeeType_HasMethodAttrStringHash(self, iter, name_str, name_hash))
					goto found;
				if (iter->tp_getsets &&
				    DeeType_HasGetSetAttrStringHash(self, iter, name_str, name_hash))
					goto found;
				if (iter->tp_members &&
				    DeeType_HasMemberAttrStringHash(self, iter, name_str, name_hash))
					goto found;
			}
			iter = DeeTypeMRO_Next(&mro, iter);
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

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
type_hasprivateattribute(DeeTypeObject *self, size_t argc,
                         DeeObject *const *argv, DeeObject *kw) {
	DeeObject *name;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__attr,
	                    "o:hasprivateattribute", &name))
		goto err;
	if (DeeObject_AssertTypeExact(name, &DeeString_Type))
		goto err;
	return_bool(!self->tp_attr &&
	            impl_type_hasprivateattribute_string_hash(self,
	                                                      DeeString_STR(name),
	                                                      DeeString_Hash(name)));
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
type_hasoperator(DeeTypeObject *self, size_t argc,
                 DeeObject *const *argv, DeeObject *kw) {
	DeeObject *name;
	Dee_operator_t opid;
	size_t op_argc = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__name_argc,
	                    "o|" UNPuSIZ ":hasoperator",
	                    &name, &op_argc))
		goto err;
	if (DeeString_Check(name)) {
		struct opinfo const *info;
		info = DeeTypeType_GetOperatorByName(Dee_TYPE(self),
		                                     DeeString_STR(name),
		                                     op_argc);
		if (info == NULL)
			goto nope;
		opid = info->oi_id;
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

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
type_hasprivateoperator(DeeTypeObject *self, size_t argc,
                        DeeObject *const *argv, DeeObject *kw) {
	DeeObject *name;
	Dee_operator_t opid;
	size_t op_argc = (size_t)-1;
	if (DeeArg_UnpackKw(argc, argv, kw, kwlist__name_argc,
	                    "o|" UNPuSIZ ":hasprivateoperator",
	                    &name, &op_argc))
		goto err;
	if (DeeString_Check(name)) {
		struct opinfo const *info;
		info = DeeTypeType_GetOperatorByName(Dee_TYPE(self),
		                                     DeeString_STR(name),
		                                     op_argc);
		if (info == NULL)
			goto nope;
		opid = info->oi_id;
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
PRIVATE WUNUSED DREF DeeObject *DCALL
type_extends_not_same(DeeTypeObject *self, size_t argc,
                          DeeObject *const *argv) {
	DeeTypeObject *other;
	if (DeeArg_Unpack(argc, argv, "o:derived_from", &other))
		goto err;
	return_bool(self != other && DeeType_Extends(self, other));
err:
	return NULL;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
type_is_vartype(DeeTypeObject *self, size_t argc,
                DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":is_vartype"))
		goto err;
	return_bool(DeeType_IsVariable(self));
err:
	return NULL;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
type_is_heaptype(DeeTypeObject *self, size_t argc,
                 DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":is_heaptype"))
		goto err;
	return_bool(DeeType_IsCustom(self));
err:
	return NULL;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
type_is_gctype(DeeTypeObject *self, size_t argc,
               DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":is_gctype"))
		goto err;
	return_bool(DeeType_IsGC(self));
err:
	return NULL;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
type_is_final(DeeTypeObject *self, size_t argc,
              DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":is_final"))
		goto err;
	return_bool(DeeType_IsFinal(self));
err:
	return NULL;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
type_is_class(DeeTypeObject *self, size_t argc,
              DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":is_class"))
		goto err;
	return_bool(DeeType_IsClass(self));
err:
	return NULL;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
type_is_complete(DeeTypeObject *__restrict UNUSED(self),
                 size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":is_complete"))
		goto err;
	return_true;
err:
	return NULL;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
type_is_classtype(DeeTypeObject *__restrict UNUSED(self),
                  size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":is_class_type"))
		goto err;
	return_false;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
type_is_ctypes_class(DeeTypeObject *__restrict self,
                     char const *__restrict name) {
	DREF DeeObject *temp;
	int error;
	temp = DeeObject_GetAttrString((DeeObject *)self, "isstructured");
	if unlikely(!temp)
		goto err;
	error = DeeObject_BoolInherited(temp);
	if unlikely(error < 0)
		goto err;
	if (!error)
		goto nope;
	temp = DeeObject_GetAttrString((DeeObject *)self, name);
	if unlikely(!temp)
		goto err;
	error = DeeObject_BoolInherited(temp);
	if unlikely(error < 0)
		goto err;
	if (error)
		return_true;
nope:
	return_false;
err:
	if (DeeError_Catch(&DeeError_AttributeError))
		goto nope;
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
type_is_pointer(DeeTypeObject *self, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":is_pointer"))
		goto err;
	return type_is_ctypes_class(self, "ispointer");
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
type_is_lvalue(DeeTypeObject *self, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":is_pointer"))
		goto err;
	return type_is_ctypes_class(self, "islvalue");
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
type_is_structured(DeeTypeObject *self, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":is_structured"))
		goto err;
	return DeeObject_GetAttrString((DeeObject *)self, "isstructured");
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
type_is_struct(DeeTypeObject *self, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":is_struct"))
		goto err;
	return type_is_ctypes_class(self, "isstruct");
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
type_is_array(DeeTypeObject *self, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":is_array"))
		goto err;
	return type_is_ctypes_class(self, "isarray");
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
type_is_foreign_function(DeeTypeObject *self, size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":is_foreign_function"))
		goto err;
	return type_is_ctypes_class(self, "isfunction");
err:
	return NULL;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
type_is_filetype(DeeTypeObject *self, size_t argc,
                 DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":is_file"))
		goto err;
	return_bool(Dee_TYPE(self) == &DeeFileType_Type);
err:
	return NULL;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
type_is_superbase(DeeTypeObject *self, size_t argc,
                  DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, ":is_super_base"))
		goto err;
	return_bool(DeeType_Base(self) == NULL);
err:
	return NULL;
}
#endif /* !CONFIG_NO_DEEMON_100_COMPAT */

PRIVATE struct type_method tpconst type_methods[] = {
	TYPE_KWMETHOD_F("baseof", &type_baseof, METHOD_FNOREFESCAPE,
	                "(other:?.)->?Dbool\n"
	                "Returns ?t if @this ?. is equal to, or a base of @other.\n"
	                "If @other isn't a ?., ?f is returned.\n"
	                "Using baseof, the behavior of ${x is y} can be approximated as:\n"
	                "${"
	                /**/ "print y.baseof(type(x)); /* aka: `print x is y;' */"
	                "}"),
	TYPE_KWMETHOD_F("extends", &type_extends, METHOD_FNOREFESCAPE,
	                "(other:?.)->?Dbool\n"
	                "Returns ?t if @this ?. is equal to, or has been derived from @other.\n"
	                "If @other isn't a ?., ?f is returned."),
	TYPE_KWMETHOD_F("implements", &type_implements, METHOD_FNOREFESCAPE,
	                "(other:?.)->?Dbool\n"
	                "Check if @other appears in ?#__mro__"),
	TYPE_KWMETHOD("newinstance", &type_newinstance,
	              "(fields!!)->\n"
	              "Allocate a new instance of @this ?. and initialize members in accordance to @fields.\n"
	              "${"
	              /**/ "class MyClass {\n"
	              /**/ "	member foo;\n"
	              /**/ "	this = del; /* Delete the regular constructor. */\n"
	              /**/ "}\n"
	              /**/ "local x = MyClass.newinstance(foo: 42);\n"
	              /**/ "print x.foo;\n"
	              "}"
	              "\n"

	              "(initializer:?S?T2?.?T1?S?T2?Dstring?O=!N)->\n"                 /* {(Type, ({(string,Object)...},)...} */
	              "(initializer:?S?T2?.?T2?S?T2?Dstring?O?N=!N)->\n"               /* {(Type, ({(string,Object)...}, none)...} */
	              "(initializer:?S?T2?.?T2?S?T2?Dstring?O?DTuple=!N)->\n"          /* {(Type, ({(string,Object)...}, Tuple)...} */
	              "(initializer:?S?T2?.?T3?S?T2?Dstring?O?DTuple?N=!N)->\n"        /* {(Type, ({(string,Object)...}, Tuple, none)...} */
	              "(initializer:?S?T2?.?T3?S?T2?Dstring?O?DTuple?DMapping=!N)->\n" /* {(Type, ({(string,Object)...}, Tuple, Mapping)...} */
	              "(initializer:?S?T2?.?T2?N?DTuple=!N)->\n"                       /* {(Type, (none, Tuple)...} */
	              "(initializer:?S?T2?.?T3?N?DTuple?N=!N)->\n"                     /* {(Type, (none, Tuple, none)...} */
	              "(initializer:?S?T2?.?T3?N?DTuple?DMapping=!N)->\n"              /* {(Type, (none, Tuple, Mapping)...} */
	              "#tTypeError{No superargs tuple was provided for one of the type's bases, when that base "
	              /*            */ "has a mandatory constructor that can't be invoked without any arguments. "
	              /*            */ "Note that a user-defined class never has a mandatory constructor, with this "
	              /*            */ "only affecting builtin types such as ?DInstanceMethod or ?DProperty}"
	              "A extended way of constructing and initializing a ?., that involves providing explicit "
	              /**/ "member initializers on a per-Type bases, as well as argument tuples and optional keyword "
	              /**/ "mappings to-be used for construction of one of the type's super-classes (allowing to provide "
	              /**/ "for explicit argument lists when one of the type's bases has a mandatory constructor)\n"
	              "${"
	              /**/ "import List from deemon;\n"
	              /**/ "class MyList: List {\n"
	              /**/ "	this = del; /* Delete the regular constructor. */\n"
	              /**/ "	member mylist_member;\n"
	              /**/ "	function appendmember() {\n"
	              /**/ "		this.append(mylist_member);\n"
	              /**/ "	}\n"
	              /**/ "}\n"
	              /**/ "local x = MyList.newinstance({\n"
	              /**/ "	MyList: ({ \"mylist_member\" : \"abc\" }, none),\n"
	              /**/ "	List:   ({ }, pack([10, 20, 30])),\n"
	              /**/ "});\n"
	              /**/ "print repr x;          /* [10, 20, 30] */\n"
	              /**/ "print x.mylist_member; /* \"abc\" */\n"
	              /**/ "x.appendmember();\n"
	              /**/ "print repr x;          /* [10, 20, 30, \"abc\"] */"
	              "}"),
	TYPE_KWMETHOD_F("hasattribute", &type_hasattribute, METHOD_FNOREFESCAPE,
	                "(name:?Dstring)->?Dbool\n"
	                "Returns ?t if this type, or one of its super-classes defines an "
	                /**/ "instance-attribute @name, and doesn't define any attribute-operators. "
	                /**/ "Otherwise, return ?f:\n"
	                "${"
	                /**/ "function hasattribute(name: string): bool {\n"
	                /**/ "	import attribute from deemon;\n"
	                /**/ "	return attribute.exists(this, name, \"ic\", \"ic\")\n"
	                /**/ "}"
	                "}\n"
	                "Note that this function only searches instance-attributes, meaning that class/static "
	                /**/ "attributes/members such as ?AIterator?Dstring. are not matched, whereas something \n"
	                /**/ "like ?Afind?Dstring is.\n"
	                "Note that this function is quite similar to #hasinstanceattr, however unlike "
	                /**/ "that function, this function will stop searching the base-classes of @this ?. "
	                /**/ "when one of that types implements one of the attribute operators."),
	TYPE_KWMETHOD_F("hasprivateattribute", &type_hasprivateattribute, METHOD_FNOREFESCAPE,
	                "(name:?Dstring)->?Dbool\n"
	                "Similar to #hasattribute, but only looks at attributes declared "
	                /**/ "by @this ?., excluding any defined by a super-class.\n"
	                "${"
	                /**/ "function hasprivateattribute(name: string): bool {\n"
	                /**/ "	import attribute from deemon;\n"
	                /**/ "	return attribute.exists(this, name, \"ic\", \"ic\", this)\n"
	                /**/ "}"
	                "}"),
	TYPE_KWMETHOD_F("hasoperator", &type_hasoperator, METHOD_FNOREFESCAPE,
	                "(name:?Dint)->?Dbool\n"
	                "(name:?Dstring)->?Dbool\n"
	                "(name:?Dstring,argc:?Dint)->?Dbool\n"
	                "Returns ?t if instances of @this ?. implement an operator @name, "
	                /**/ "or ?f if not, or if @name is not recognized as an operator "
	                /**/ "available for the Type-Type that is ${type this}\n"
	                "Note that this function also looks at the operators of "
	                /**/ "base-classes, as well as that a user-defined class that has "
	                /**/ "explicitly deleted an operator will cause this function to "
	                /**/ "return true, indicative of that operator being implemented "
	                /**/ "to cause an error to be thrown when invoked.\n"
	                "The given @name is the so-called real operator name, "
	                /**/ "as listed under Name in the following table:\n"
	                "#T{Name|Symbolical name|Prototype~"
	                /**/ "$\"constructor\"|$\"this\"|${this(args..., **kwds)}&"
	                /**/ "$\"copy\"|$\"copy\"|${copy()}&"
	                /**/ "$\"deepcopy\"|$\"deepcopy\"|${deepcopy()}&"
	                /**/ "$\"destructor\"|$\"#~this\"|${##~this()}&"
	                /**/ "$\"assign\"|$\":=\"|${operator := (other)}&"
	                /**/ "$\"str\"|$\"str\"|${operator str(): string}&"
	                /**/ "$\"repr\"|$\"repr\"|${operator repr(): string}&"
	                /**/ "$\"bool\"|$\"bool\"|${operator bool(): bool}&"
	                /**/ "$\"call\"|$\"()\"|${operator ()(args!): Object}&"
	                /**/ "$\"next\"|$\"next\"|${operator next(): Object}&"
	                /**/ "$\"int\"|$\"int\"|${operator int(): int}&"
	                /**/ "$\"float\"|$\"float\"|${operator float(): float}&"
	                /**/ "$\"inv\"|$\"#~\"|${operator #~ (): Object}&"
	                /**/ "$\"pos\"|$\"+\"|${operator + (): Object}&"
	                /**/ "$\"neg\"|$\"-\"|${operator - (): Object}&"
	                /**/ "$\"add\"|$\"+\"|${operator + (other): Object}&"
	                /**/ "$\"sub\"|$\"-\"|${operator - (other): Object}&"
	                /**/ "$\"mul\"|$\"*\"|${operator * (other): Object}&"
	                /**/ "$\"div\"|$\"/\"|${operator / (other): Object}&"
	                /**/ "$\"mod\"|$\"%\"|${operator % (other): Object}&"
	                /**/ "$\"shl\"|$\"<<\"|${operator << (other): Object}&"
	                /**/ "$\"shr\"|$\">>\"|${operator >> (other): Object}&"
	                /**/ "$\"and\"|$\"#&\"|${operator #& (other): Object}&"
	                /**/ "$\"or\"|$\"#|\"|${operator #| (other): Object}&"
	                /**/ "$\"xor\"|$\"^\"|${operator ^ (other): Object}&"
	                /**/ "$\"pow\"|$\"**\"|${operator ** (other): Object}&"
	                /**/ "$\"inc\"|$\"++\"|${operator ++ (): Object}&"
	                /**/ "$\"dec\"|$\"--\"|${operator -- (): Object}&"
	                /**/ "$\"iadd\"|$\"+=\"|${operator += (other): Object}&"
	                /**/ "$\"isub\"|$\"-=\"|${operator -= (other): Object}&"
	                /**/ "$\"imul\"|$\"*=\"|${operator *= (other): Object}&"
	                /**/ "$\"idiv\"|$\"/=\"|${operator /= (other): Object}&"
	                /**/ "$\"imod\"|$\"%=\"|${operator %= (other): Object}&"
	                /**/ "$\"ishl\"|$\"<<=\"|${operator <<= (other): Object}&"
	                /**/ "$\"ishr\"|$\">>=\"|${operator >>= (other): Object}&"
	                /**/ "$\"iand\"|$\"#&=\"|${operator #&= (other): Object}&"
	                /**/ "$\"ior\"|$\"#|=\"|${operator #|= (other): Object}&"
	                /**/ "$\"ixor\"|$\"^=\"|${operator ^= (other): Object}&"
	                /**/ "$\"ipow\"|$\"**=\"|${operator **= (other): Object}&"
	                /**/ "$\"hash\"|$\"hash\"|${operator hash(): int}&"
	                /**/ "$\"eq\"|$\"==\"|${operator == (other): Object}&"
	                /**/ "$\"ne\"|$\"!=\"|${operator != (other): Object}&"
	                /**/ "$\"lo\"|$\"<\"|${operator < (other): Object}&"
	                /**/ "$\"le\"|$\"<=\"|${operator <= (other): Object}&"
	                /**/ "$\"gr\"|$\">\"|${operator > (other): Object}&"
	                /**/ "$\"ge\"|$\">=\"|${operator >= (other): Object}&"
	                /**/ "$\"iter\"|$\"iter\"|${operator iter(): Object}&"
	                /**/ "$\"size\"|$\"##\"|${operator ## (): Object}&"
	                /**/ "$\"contains\"|$\"contains\"|${operator contains(item): Object}&"
	                /**/ "$\"getitem\"|$\"[]\"|${operator [] (index): Object}&"
	                /**/ "$\"delitem\"|$\"del[]\"|${operator del[] (index): none}&"
	                /**/ "$\"setitem\"|$\"[]=\"|${operator []= (index, value): none}&"
	                /**/ "$\"getrange\"|$\"[:]\"|${operator [:] (start, end): Object}&"
	                /**/ "$\"delrange\"|$\"del[:]\"|${operator del[:] (start, end): none}&"
	                /**/ "$\"setrange\"|$\"[:]=\"|${operator [:]= (start, end, value): none}&"
	                /**/ "$\"getattr\"|$\".\"|${operator . (name: string): Object}&"
	                /**/ "$\"delattr\"|$\"del.\"|${operator del. (name: string): none}&"
	                /**/ "$\"setattr\"|$\".=\"|${operator .= (name: string, value): none}&"
	                /**/ "$\"enumattr\"|$\"enumattr\"|${operator enumattr(): {attribute...}}&"
	                /**/ "$\"enter\"|$\"enter\"|${operator enter(): none}&"
	                /**/ "$\"leave\"|$\"leave\"|${operator leave(): none}"
	                "}"),
	TYPE_KWMETHOD_F("hasprivateoperator", &type_hasprivateoperator, METHOD_FNOREFESCAPE,
	                "(name:?Dint)->?Dbool\n"
	                "(name:?Dstring)->?Dbool\n"
	                "(name:?Dstring,argc:?Dint)->?Dbool\n"
	                "Returns ?t if instances of @this ?. implement an operator @name, "
	                /**/ "or ?f if not, or if @name is not recognized as an operator provided "
	                /**/ "available for the Type-Type that is ${type this}.\n"
	                "Note that this function intentionally don't look at operators of "
	                /**/ "base-classes (which is instead done by #hasoperator), meaning that "
	                /**/ "inherited operators are not included, with the exception of explicitly "
	                /**/ "inherited constructors.\n"
	                "For a list of operator names, see #hasoperator."),
	TYPE_KWMETHOD(STR_getinstanceattr, &type_getinstanceattr,
	              "(name:?Dstring)->\n"
	              "Lookup an attribute @name that is implemented by instances of @this ?.\n"
	              "Normally, such attributes can also be accessed using regular attribute lookup, "
	              /**/ "however in ambiguous cases where both the type, as well as instances implement "
	              /**/ "an attribute of the same name (s.a. ?AKeys?DDict vs. ?Akeys?DDict), using regular "
	              /**/ "attribute lookup on the type (as in ${posix.stat.isdir}) will always return the "
	              /**/ "type-attribute, rather than a wrapper around the instance attribute.\n"
	              "In such cases, this function may be used to explicitly lookup the instance variant:\n"
	              "${"
	              /**/ "import stat from posix;\n"
	              /**/ "local statIsDirProperty = stat.getinstanceattr(\"isdir\");\n"
	              /**/ "local myStatInstance = stat(\".\");\n"
	              /**/ "// Same as `myStatInstance.isdir' -- true\n"
	              /**/ "print repr statIsDirProperty(myStatInstance);"
	              "}\n"
	              "Note that one minor exception exists to the default lookup rule, and it relates to how "
	              /**/ "attributes of ?. itself are queried (such as in the expression ${(Type from deemon).baseof}).\n"
	              "In this case, access is always made as an instance-bound, meaning that for this purpose, "
	              /**/ "?. is considered an instance of ?. (typetype), rather than the type of ?. (typetype) "
	              /**/ "(I know that sounds complicated, but without this rule, ${(Type from deemon).baseof} would "
	              /**/ "return a class method object taking 2 arguments, rather than the intended single argument).\n"
	              "Also note that the `*instanceattr' functions will not check for types that have overwritten "
	              /**/ "one of the attribute-operators, but will continue search for matching attribute names, even "
	              /**/ "if those attributes would normally have been overshadowed by attribute callbacks."),
	TYPE_KWMETHOD(STR_callinstanceattr, &type_callinstanceattr, "(name:?Dstring,args!,kwds!!)->\ns.a. ?#getinstanceattr"),
	TYPE_KWMETHOD(STR_hasinstanceattr, &type_hasinstanceattr, "(name:?Dstring)->?Dbool\ns.a. ?#getinstanceattr"),
	TYPE_KWMETHOD(STR_boundinstanceattr, &type_boundinstanceattr, "(name:?Dstring,allow_missing=!t)->?Dbool\ns.a. ?#getinstanceattr"),
	TYPE_KWMETHOD(STR_delinstanceattr, &type_delinstanceattr, "(name:?Dstring)\ns.a. ?#getinstanceattr"),
	TYPE_KWMETHOD(STR_setinstanceattr, &type_setinstanceattr, "(name:?Dstring,value)->\ns.a. ?#getinstanceattr"),

	TYPE_KWMETHOD_F("derivedfrom", &type_extends, METHOD_FNOREFESCAPE,
	                "(other:?.)->?Dbool\n"
	                "Deprecated alias for ?#extends"),

#ifndef CONFIG_NO_DEEMON_100_COMPAT
	/* Deprecated functions */
	TYPE_KWMETHOD_F("same_or_derived_from", &type_extends, METHOD_FNOREFESCAPE, "(other:?.)->?Dbool\nDeprecated alias for ?#derivedfrom"),
	TYPE_METHOD_F("derived_from", &type_extends_not_same, METHOD_FNOREFESCAPE, "(other:?.)->?Dbool\nDeprecated alias for ${this !== other && this.derivedfrom(other)}"),
	TYPE_METHOD_F("is_vartype", &type_is_vartype, METHOD_FNOREFESCAPE, "->?Dbool\nDeprecated alias for ?#__isvariable__"),
	TYPE_METHOD_F("is_heaptype", &type_is_heaptype, METHOD_FNOREFESCAPE, "->?Dbool\nDeprecated alias for ?#__iscustom__"),
	TYPE_METHOD_F("is_gctype", &type_is_gctype, METHOD_FNOREFESCAPE, "->?Dbool\nDeprecated alias for ?#__isgc__"),
	TYPE_METHOD_F("is_final", &type_is_final, METHOD_FNOREFESCAPE, "->?Dbool\nDeprecated alias for ?#isfinal"),
	TYPE_METHOD_F("is_class", &type_is_class, METHOD_FNOREFESCAPE, "->?Dbool\nDeprecated alias for ?#__isclass__"),
	TYPE_METHOD_F("is_complete", &type_is_complete, METHOD_FNOREFESCAPE, "->?Dbool\nDeprecated (always returns ?t)"),
	TYPE_METHOD_F("is_classtype", &type_is_classtype, METHOD_FNOREFESCAPE, "->?Dbool\nDeprecated (always returns ?f)"),
	TYPE_METHOD("is_pointer", &type_is_pointer, "->?Dbool\nDeprecated alias for ${try this.isstructured && this.ispointer catch ((Error from deemon).AttributeError) false}"),
	TYPE_METHOD("is_lvalue", &type_is_lvalue, "->?Dbool\nDeprecated alias for ${try this.isstructured && this.islvalue catch ((Error from deemon).AttributeError) false}"),
	TYPE_METHOD("is_structured", &type_is_structured, "->?Dbool\nDeprecated alias for ${try this.isstructured catch ((Error from deemon).AttributeError) false}"),
	TYPE_METHOD("is_struct", &type_is_struct, "->?Dbool\nDeprecated alias for ${try this.isstructured && this.isstruct catch ((Error from deemon).AttributeError) false}"),
	TYPE_METHOD("is_array", &type_is_array, "->?Dbool\nDeprecated alias for ${try this.isstructured && this.isarray catch ((Error from deemon).AttributeError) false}"),
	TYPE_METHOD("is_foreign_function", &type_is_foreign_function, "->?Dbool\nDeprecated alias for ${try this.isstructured && this.isfunction catch ((Error from deemon).AttributeError) false}"),
	TYPE_METHOD_F("is_file", &type_is_filetype, METHOD_FNOREFESCAPE, "->?Dbool\nDeprecated alias for ${this is type(File from deemon)}"),
	TYPE_METHOD_F("is_super_base", &type_is_superbase, METHOD_FNOREFESCAPE, "->?Dbool\nDeprecated alias for ${this.__base__ is none}"),
#endif /* !CONFIG_NO_DEEMON_100_COMPAT */

	TYPE_METHOD_END
};


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
type_isbuffer(DeeTypeObject *__restrict self) {
	do {
		if (self->tp_buffer && self->tp_buffer->tp_getbuf)
			return_true;
	} while ((self = self->tp_base) != NULL);
	return_false;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
type_get_classdesc(DeeTypeObject *__restrict self) {
	if (!DeeType_IsClass(self)) {
		DeeError_Throwf(&DeeError_AttributeError,
		                "Can't access `__class__' of non-user-defined Type `%s'",
		                self->tp_name);
		return NULL;
	}
	return_reference_((DeeObject *)self->tp_class->cd_desc);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
type_get_seqclass(DeeTypeObject *__restrict self) {
	DREF DeeObject *result;
	unsigned int cls = DeeType_GetSeqClass(self);
	switch (cls) {
	case Dee_SEQCLASS_NONE:
		result = Dee_None;
		break;
	case Dee_SEQCLASS_SEQ:
		result = (DREF DeeObject *)&DeeSeq_Type;
		break;
	case Dee_SEQCLASS_SET:
		result = (DREF DeeObject *)&DeeSet_Type;
		break;
	case Dee_SEQCLASS_MAP:
		result = (DREF DeeObject *)&DeeMapping_Type;
		break;
	default: __builtin_unreachable();
	}
	Dee_Incref(result);
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
type_issingleton(DeeTypeObject *__restrict self) {
	if (self->tp_features & TF_SINGLETON)
		return_true; /* Alternative means of creation. */
	if (DeeType_IsClass(self)) {
		/* Special handling for user-defined classes. */
		if (!self->tp_init.tp_alloc.tp_ctor &&
		    !self->tp_init.tp_alloc.tp_any_ctor &&
		    !self->tp_init.tp_alloc.tp_any_ctor_kw)
			return_true; /* The type is isn't constructible. */
	}
	return_false;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
get_module_from_addr(struct class_desc *__restrict my_class, uint16_t addr) {
	DeeObject *slot;
	DREF DeeModuleObject *result = NULL;
	Dee_class_desc_lock_read(my_class);
	slot = my_class->cd_members[addr];
	if (slot && DeeFunction_Check(slot)) {
		DeeFunctionObject *slot_function;
		slot_function = (DeeFunctionObject *)slot;
		result        = slot_function->fo_code->co_module;
		Dee_Incref(result);
	}
	Dee_class_desc_lock_endread(my_class);
	return (DREF DeeObject *)result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeClass_GetModule(DeeTypeObject *__restrict self) {
	struct class_desc *my_class    = self->tp_class;
	DeeClassDescriptorObject *desc = my_class->cd_desc;
	DREF DeeObject *result;
	size_t i;

	/* Search through the operator bindings table. */
	for (i = 0; i <= desc->cd_clsop_mask; ++i) {
		if (desc->cd_clsop_list[i].co_name == (Dee_operator_t)-1)
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
#endif /* CLASS_GETSET_GET == 0 */
		if ((desc->cd_iattr_list[i].ca_flag &
		     (CLASS_ATTRIBUTE_FREADONLY | CLASS_ATTRIBUTE_FGETSET)) ==
		    (CLASS_ATTRIBUTE_FGETSET)) {
#if CLASS_GETSET_GET != 0
			result = get_module_from_addr(my_class,
			                              desc->cd_iattr_list[i].ca_addr + CLASS_GETSET_GET);
			if (result)
				goto done;
#endif /* CLASS_GETSET_GET != 0 */
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
#endif /* CLASS_GETSET_GET != 0 */
	}
	return NULL;
done:
	return result;
}

/* Return the module used to define a given type `self',
 * or `NULL' if that module could not be determined.
 * NOTE: When `NULL' is returned, _NO_ error is thrown! */
PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
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
	result = Dee_weakref_lock(&self->tp_module);
	if (result != NULL)
		return result;
	if (DeeType_IsClass(self)) {
		result = DeeClass_GetModule(self);
		if (result != NULL)
			Dee_weakref_set(&self->tp_module, result);
		return result;
	}
	if (!(self->tp_flags & TP_FHEAP)) {
		/* Lookup the originating module of a statically allocated C-type. */
		result = DeeModule_FromStaticPointer(self);
		if (result) {
			Dee_weakref_set(&self->tp_module, result);
			return result;
		}
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

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
type_get_module(DeeTypeObject *__restrict self) {
	DREF DeeObject *result;
	result = DeeType_GetModule(self);
	if likely(result)
		return result;
	err_unbound_attribute_string(Dee_TYPE(self), STR___module__);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
type_bound_module(DeeTypeObject *__restrict self) {
	DREF DeeObject *result;
	result = DeeType_GetModule(self);
	if likely(result) {
		Dee_Decref_unlikely(result);
		return 1;
	}
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
type_get_instancesize(DeeTypeObject *__restrict self) {
	size_t instance_size;
	if (self->tp_flags & TP_FVARIABLE)
		goto unknown;
	if (!self->tp_init.tp_alloc.tp_ctor &&
	    !self->tp_init.tp_alloc.tp_copy_ctor &&
	    !self->tp_init.tp_alloc.tp_deep_ctor &&
	    !self->tp_init.tp_alloc.tp_any_ctor &&
	    !self->tp_init.tp_alloc.tp_any_ctor_kw)
		goto unknown;
	if unlikely(self->tp_init.tp_alloc.tp_free) {
#ifndef CONFIG_NO_OBJECT_SLABS
		/* Check for slab allocators. */
		void (DCALL *tp_free)(void *__restrict ob);
		tp_free = self->tp_init.tp_alloc.tp_free;
#define CHECK_SIZE(index, size)                        \
		if (tp_free == &DeeObject_SlabFree##size ||    \
		    tp_free == &DeeGCObject_SlabFree##size) {  \
			instance_size = size * __SIZEOF_POINTER__; \
		} else
		DeeSlab_ENUMERATE(CHECK_SIZE)
#undef CHECK_SIZE
		{
			goto unknown;
		}
#else /* !CONFIG_NO_OBJECT_SLABS */
		goto unknown;
#endif /* CONFIG_NO_OBJECT_SLABS */
	} else {
		instance_size = self->tp_init.tp_alloc.tp_instance_size;
	}
	return DeeInt_NewSize(instance_size);
unknown:
	err_unbound_attribute_string(Dee_TYPE(self), "__instancesize__");
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
type_bound_instancesize(DeeTypeObject *__restrict self) {
	if (self->tp_flags & TP_FVARIABLE)
		goto unknown;
	if (!self->tp_init.tp_alloc.tp_ctor &&
	    !self->tp_init.tp_alloc.tp_copy_ctor &&
	    !self->tp_init.tp_alloc.tp_deep_ctor &&
	    !self->tp_init.tp_alloc.tp_any_ctor &&
	    !self->tp_init.tp_alloc.tp_any_ctor_kw)
		goto unknown;
	if unlikely(self->tp_init.tp_alloc.tp_free) {
#ifndef CONFIG_NO_OBJECT_SLABS
		/* Check for slab allocators. */
		void (DCALL *tp_free)(void *__restrict ob);
		tp_free = self->tp_init.tp_alloc.tp_free;
#define CHECK_SIZE(index, size)                        \
		if (tp_free == &DeeObject_SlabFree##size ||    \
		    tp_free == &DeeGCObject_SlabFree##size) {  \
		} else
		DeeSlab_ENUMERATE(CHECK_SIZE)
#undef CHECK_SIZE
		{
			goto unknown;
		}
#else /* !CONFIG_NO_OBJECT_SLABS */
		goto unknown;
#endif /* CONFIG_NO_OBJECT_SLABS */
	}
	return 1;
unknown:
	return 0;
}

INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL type_get_operators(DeeTypeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL type_get_operatorids(DeeTypeObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL type_get_ctable(DeeTypeObject *__restrict self);


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
type_istypetype(DeeTypeObject *__restrict self) {
	return_bool(DeeType_IsTypeType(self));
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
type_isvarargconstructible(DeeTypeObject *__restrict self) {
	return_bool(DeeType_IsVarArgConstructible(self));
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
type_isconstructible(DeeTypeObject *__restrict self) {
	return_bool(DeeType_IsConstructible(self));
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
type_iscopyable(DeeTypeObject *__restrict self) {
	return_bool(DeeType_IsCopyable(self));
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
type_isnamespace(DeeTypeObject *__restrict self) {
	return_bool(DeeType_IsNamespace(self));
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
type_isconstcastable(DeeTypeObject *__restrict self) {
	return_bool(DeeType_IsConstCastable(self));
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
type_gcpriority(DeeTypeObject *__restrict self) {
	return DeeInt_NewUInt(DeeType_GCPriority(self));
}

PRIVATE struct type_member tpconst type_members[] = {
	TYPE_MEMBER_FIELD_DOC(STR___name__, STRUCT_CONST | STRUCT_CSTR, offsetof(DeeTypeObject, tp_name),
	                      "->?Dstring\n"
	                      "#t{UnboundAttribute}"
	                      "Name of this type"),
	TYPE_MEMBER_FIELD_DOC(STR___doc__, STRUCT_CONST | STRUCT_CSTR_OPT, offsetof(DeeTypeObject, tp_doc),
	                      "->?X2?Dstring?N\n"
	                      "Doc string for this type, including documentation on operators, or ?N if it has none"),
	TYPE_MEMBER_FIELD_DOC("__base__", STRUCT_OBJECT, offsetof(DeeTypeObject, tp_base),
	                      "->?.\n"
	                      "#t{UnboundAttribute}"
	                      "The immediate/primary base of this type"),
	TYPE_MEMBER_BITFIELD_DOC("isfinal", STRUCT_CONST, DeeTypeObject, tp_flags, TP_FFINAL,
	                         "True if this type cannot be sub-classed"),
	TYPE_MEMBER_BITFIELD_DOC("isinterrupt", STRUCT_CONST, DeeTypeObject, tp_flags, TP_FINTERRUPT,
	                         "True if instances of this type, when thrown, can only be caught by "
	                         "an ${@[interrupt] catch (...)} catch-all expression, or by a catch "
	                         "expression that explicitly names this type"),
	TYPE_MEMBER_BITFIELD_DOC("isabstract", STRUCT_CONST, DeeTypeObject, tp_flags, TP_FABSTRACT,
	                         "True if this type is #Iabstract, meaning it can be used as an interface"),
	TYPE_MEMBER_BITFIELD_DOC("__isvariable__", STRUCT_CONST, DeeTypeObject, tp_flags, TP_FVARIABLE,
	                         "True if instances of this type have a variable size (s.a. ?#__instancesize__)"),
	TYPE_MEMBER_BITFIELD("__isgc__", STRUCT_CONST, DeeTypeObject, tp_flags, TP_FGC),
	TYPE_MEMBER_FIELD_DOC("__isclass__", STRUCT_CONST | STRUCT_BOOLPTR, offsetof(DeeTypeObject, tp_class),
	                      "True if this type is a user-defined class (s.a. ?#__class__)"),
	TYPE_MEMBER_FIELD_DOC("__isarithmetic__", STRUCT_CONST | STRUCT_BOOLPTR, offsetof(DeeTypeObject, tp_math),
	                      "True if this type implements math-related operators"),
	TYPE_MEMBER_FIELD_DOC("__iscomparable__", STRUCT_CONST | STRUCT_BOOLPTR, offsetof(DeeTypeObject, tp_cmp),
	                      "True if this type implements compare-related operators"),
	TYPE_MEMBER_FIELD_DOC("__issequence__", STRUCT_CONST | STRUCT_BOOLPTR, offsetof(DeeTypeObject, tp_seq),
	                      "True if this type implements sequence-related operators"),
	TYPE_MEMBER_BITFIELD("__isinttruncated__", STRUCT_CONST, DeeTypeObject, tp_flags, TP_FTRUNCATE),
	TYPE_MEMBER_BITFIELD("__hasmoveany__", STRUCT_CONST, DeeTypeObject, tp_flags, TP_FMOVEANY),
	TYPE_MEMBER_FIELD_DOC("__isiterator__", STRUCT_CONST | STRUCT_BOOLPTR, offsetof(DeeTypeObject, tp_iter_next),
	                      "True if this type implements the ${operator iter} operator"),
	TYPE_MEMBER_BITFIELD_DOC("__iscustom__", STRUCT_CONST, DeeTypeObject, tp_flags, TP_FHEAP,
	                         "True if this type was dynamically allocated on the heap"),
	TYPE_MEMBER_BITFIELD("__issuperconstructible__", STRUCT_CONST, DeeTypeObject, tp_flags, TP_FINHERITCTOR),
	TYPE_MEMBER_BITFIELD_DOC("__iskw__", STRUCT_CONST, DeeTypeObject, tp_features, TF_KW,
	                         "True if instances of this type can be used as keywords argument in the C API. "
	                         /**/ "When this is not true, and an instance is used as a keywords argument, the "
	                         /**/ "compiler must generate code to wrap instance of this type as a mapping that "
	                         /**/ "#Idoes support ?#__iskw__ (s.a. ?Ert:kw)"),
	TYPE_MEMBER_FIELD("__isnoargconstructible__", STRUCT_CONST | STRUCT_BOOLPTR,
	                  offsetof(DeeTypeObject, tp_init.tp_alloc.tp_ctor)),
	TYPE_MEMBER_END
};

PRIVATE struct type_getset tpconst type_getsets[] = {
	TYPE_GETTER_F("isbuffer", &type_isbuffer,
	              METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	              "->?Dbool\n"
	              "Returns ?t if @this Type implements the buffer interface\n"
	              "The most prominent Type to which this applies is ?DBytes, however other types also support this"),
	TYPE_GETTER_F("__bases__", &TypeBases_New,
	              METHOD_FCONSTCALL,
	              "->?Ert:TypeBases\n"
	              "Returns a sequence of ?. that represents all immediate bases of @this ?."),
	TYPE_GETTER_F("__mro__", &TypeMRO_New,
	              METHOD_FCONSTCALL,
	              "->?Ert:TypeMRO\n"
	              "Returns a sequence of ?. that represents the MethodResolutionOrder of @this ?."),
	TYPE_GETTER_F("__class__", &type_get_classdesc,
	              METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	              "->?Ert:ClassDescriptor\n"
	              "#tAttributeError{@this typeType is a user-defined class (s.a. ?#__isclass__)}"
	              "Returns the internal class-descriptor descriptor for a user-defined class"),
	TYPE_GETTER_F("__seqclass__", &type_get_seqclass,
	              METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	              "->?X2?.?N\n"
	              "Returns the #C{sequence classification} of this type, which is the first of the "
	              /**/ "following types to appear in ?#__mro__:"
	              "#L-{"
	              /**/ "?DSequence|"
	              /**/ "?DSet|"
	              /**/ "?DMapping"
	              "}"),
	TYPE_GETTER_F("__issingleton__", &type_issingleton,
	              METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	              "->?Dbool\n"
	              "Check if @this Type describes a singleton object, requiring that @this type not be "
	              /**/ "implementing a constructor (or be deleting its constructor), as well as not be one "
	              /**/ "of the special internal types used to represent implementation-specific wrapper "
	              /**/ "objects for C attributes, or be generated by the compiler, such as code objects, "
	              /**/ "class descriptors or DDI information providers"),
	TYPE_GETTER_BOUND_F(STR___module__, &type_get_module, &type_bound_module,
	                    METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                    "->?DModule\n"
	                    "#t{UnboundAttribute}"
	                    "Return the module used to define @this Type, or throw :UnboundAttribute if the module "
	                    /**/ "cannot be determined, which may be the case if the type doesn't have any defining "
	                    /**/ "features such as operators, or class/instance member functions"),
	TYPE_GETTER_F("__ctable__", &type_get_ctable,
	              METHOD_FCONSTCALL,
	              "->?X2?S?O?AObjectTable?Ert:ClassDescriptor\n"
	              "Returns an indexable sequence describing the class object table, "
	              /**/ "as referenced by ?Aaddr?AAttribute?Ert:ClassDescriptor\n"
	              "For non-user-defined classes (aka. ?#__isclass__ is ?f), an empty sequence is returned\n"
	              "The instance-attribute table can be accessed through ?A__itable__?DObject"),
	TYPE_GETTER_F("__operators__", &type_get_operators,
	              METHOD_FCONSTCALL,
	              "->?S?X2?Dstring?Dint\n"
	              "Enumerate the names of all the operators overwritten by @this Type as a set-like sequence\n"
	              "This member functions such that the member function ?#hasprivateoperator can be implemented as:\n"
	              "${"
	              /**/ "function hasprivateoperator(name: string | int): bool {\n"
	              /**/ "	return name in this.__operators__;\n"
	              /**/ "}"
	              "}\n"
	              "Also note that this set doesn't differentiate between overwritten and deleted operators, "
	              /**/ "as for this purpose any deleted operator is considered to be implemented as throwing a "
	              /**/ ":NotImplemented exception\n"
	              "Additionally, this set also includes automatically generated operators for user-classes, "
	              /**/ "meaning that pretty much any user-class will always have its compare, assignment, as well "
	              /**/ "as constructor and destructor operators overwritten, even when the user didn't actually "
	              /**/ "define any of them\n"
	              "For the purposes of human-readable information, is is recommended to use ?Aoperators?#__class__ "
	              /**/ "when @this Type is a user-defined class (aka. ?#__isclass__ is ?t), and only use ?#__operators__ "
	              /**/ "for all other types that this doesn't apply to"),
	TYPE_GETTER_F("__operatorids__", &type_get_operatorids,
	              METHOD_FCONSTCALL,
	              "->?S?Dint\n"
	              "Enumerate the ids of all the operators overwritten by @this Type as a set-like sequence\n"
	              "This is the same as ?#__operators__, but the runtime will not attempt to translate known "
	              /**/ "operator ids to their user-friendly name, as described in ?#hasoperator"),
	TYPE_GETTER_BOUND_F("__instancesize__", &type_get_instancesize, &type_bound_instancesize,
	                    METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                    "->?Dint\n"
	                    "#t{UnboundAttribute}"
	                    "Returns the heap allocation size of instances of @this Type, or throw :UnboundAttribute "
	                    /**/ "when @this Type cannot be instantiated, is a singleton (such as ?N), or has variable-"
	                    /**/ "length instances (?#isvariable)"),
#ifndef CONFIG_NO_DEEMON_100_COMPAT
	TYPE_GETTER_BOUND_F("__instance_size__", &type_get_instancesize, &type_bound_instancesize,
	                    METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                    "->?Dint\n"
	                    "#t{UnboundAttribute}"
	                    "Deprecated alias for ?#__instancesize__"),
#endif /* !CONFIG_NO_DEEMON_100_COMPAT */
	TYPE_GETTER_F("__istypetype__", &type_istypetype, METHOD_FCONSTCALL | METHOD_FNOREFESCAPE, "->?Dbool"),
	TYPE_GETTER_F("__isvarargconstructible__", &type_isvarargconstructible, METHOD_FCONSTCALL | METHOD_FNOREFESCAPE, "->?Dbool"),
	TYPE_GETTER_F("__isconstructible__", &type_isconstructible, METHOD_FCONSTCALL | METHOD_FNOREFESCAPE, "->?Dbool"),
	TYPE_GETTER_F("__iscopyable__", &type_iscopyable, METHOD_FCONSTCALL | METHOD_FNOREFESCAPE, "->?Dbool"),
	TYPE_GETTER_F("__isnamespace__", &type_isnamespace, METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	              "->?Dbool\n"
	              "Instance methods/getsets of this type never look at the $this argument "
	              /**/ "(allowing for optimizations in ?M_hostasm by passing undefined values for it)"),
	TYPE_GETTER_F("__isconstcastable__", &type_isconstcastable, METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	              "->?Dbool\n"
	              "Allow constant propagation when instances of this type as used as arguments "
	              /**/ "to functions marked as #C{METHOD_FCONSTCALL_IF_ARGS_CONSTCAST}."),
	TYPE_GETTER_F("__gcpriority__", &type_gcpriority, METHOD_FCONSTCALL | METHOD_FNOREFESCAPE, "->?Dint"),
	TYPE_GETSET_END
};



PRIVATE WUNUSED NONNULL((1)) Dee_hash_t DCALL
generic_object_hash(DeeObject *__restrict self) {
	return Dee_HashPointer(self);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
generic_object_compare_eq(DeeObject *self, DeeObject *some_object) {
	if (DeeObject_AssertType(some_object, Dee_TYPE(self)))
		goto err;
	return self == some_object ? 0 : 1;
err:
	return Dee_COMPARE_ERR;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
generic_object_trycompare_eq(DeeObject *self, DeeObject *some_object) {
	return self == some_object ? 0 : 1;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
generic_object_eq(DeeObject *self, DeeObject *some_object) {
	if (DeeObject_AssertType(some_object, Dee_TYPE(self)))
		goto err;
	return_bool_(self == some_object);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
generic_object_ne(DeeObject *self, DeeObject *some_object) {
	if (DeeObject_AssertType(some_object, Dee_TYPE(self)))
		goto err;
	return_bool_(self != some_object);
err:
	return NULL;
}

/* Generic operators that implement equals using `===' and hash using `Object.id()'
 * Use this instead of re-inventing the wheel in order to allow for special optimization
 * to be possible when your type appears in compare operations. */
PUBLIC struct Dee_type_cmp DeeObject_GenericCmpByAddr = {
	/* .tp_hash          = */ &generic_object_hash,
	/* .tp_compare_eq    = */ &generic_object_compare_eq,
	/* .tp_compare       = */ NULL,
	/* .tp_trycompare_eq = */ &generic_object_trycompare_eq,
	/* .tp_eq            = */ &generic_object_eq,
	/* .tp_ne            = */ &generic_object_ne,
};




#define type_hash generic_object_hash
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
type_compare_eq(DeeObject *self, DeeObject *some_object) {
	if (DeeObject_AssertType(some_object, &DeeType_Type))
		goto err;
	return self == some_object ? 0 : 1;
err:
	return Dee_COMPARE_ERR;
}

#define type_trycompare_eq generic_object_trycompare_eq
PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
type_eq(DeeObject *self, DeeObject *some_object) {
	if (DeeObject_AssertType(some_object, &DeeType_Type))
		goto err;
	return_bool_(self == some_object);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
type_ne(DeeObject *self, DeeObject *some_object) {
	if (DeeObject_AssertType(some_object, &DeeType_Type))
		goto err;
	return_bool_(self != some_object);
err:
	return NULL;
}

PRIVATE struct type_cmp type_cmp_ = {
	/* .tp_hash          = */ &type_hash,
	/* .tp_compare_eq    = */ &type_compare_eq,
	/* .tp_compare       = */ NULL,
	/* .tp_trycompare_eq = */ &type_trycompare_eq,
	/* .tp_eq            = */ &type_eq,
	/* .tp_ne            = */ &type_ne,
};



PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
type_getattr(DeeObject *self, DeeObject *name) {
	return DeeType_GetAttr((DeeTypeObject *)self, name);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
type_delattr(DeeObject *self, DeeObject *name) {
	return DeeType_DelAttr((DeeTypeObject *)self, name);
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
type_setattr(DeeObject *self, DeeObject *name, DeeObject *value) {
	return DeeType_SetAttr((DeeTypeObject *)self, name, value);
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) dssize_t DCALL
type_enumattr(DeeTypeObject *UNUSED(tp_self),
              DeeObject *self, denum_t proc, void *arg) {
	return DeeType_EnumAttr((DeeTypeObject *)self, proc, arg);
}

PRIVATE WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
type_findattr(DeeTypeObject *UNUSED(tp_self), DeeTypeObject *self,
              struct attribute_info *__restrict result,
              struct attribute_lookup_rules const *__restrict rules) {
	return DeeType_FindAttr(self, result, rules);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
type_hasattr(DeeTypeObject *self, DeeObject *name) {
	return DeeType_HasAttr(self, name);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
type_boundattr(DeeTypeObject *self, DeeObject *name) {
	return DeeType_BoundAttr(self, name);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
type_callattr(DeeTypeObject *self, DeeObject *name,
              size_t argc, DeeObject *const *argv) {
	return DeeType_CallAttr(self, name, argc, argv);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
type_callattr_kw(DeeTypeObject *self, DeeObject *name,
                 size_t argc, DeeObject *const *argv, DeeObject *kw) {
	return DeeType_CallAttrKw(self, name, argc, argv, kw);
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
type_vcallattrf(DeeTypeObject *self, DeeObject *name,
                char const *format, va_list args) {
	return DeeType_VCallAttrf(self, name, format, args);
}

PRIVATE WUNUSED NONNULL((1, 2, 3, 6)) bool DCALL
type_findattr_info_string_len_hash(DeeTypeObject *tp_self, DeeTypeObject *self,
                                   char const *__restrict attr, size_t attrlen, Dee_hash_t hash,
                                   struct Dee_attrinfo *__restrict retinfo) {
	(void)tp_self;
	return DeeType_FindAttrInfoStringLenHash(self, attr, attrlen, hash, retinfo);
}

#ifdef CONFIG_CALLTUPLE_OPTIMIZATIONS
PRIVATE WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
type_callattr_tuple(DeeTypeObject *self, /*String*/ DeeObject *name, DeeObject *args) {
	return DeeType_CallAttrTuple(self, name, args);
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
type_callattr_tuple_kw(DeeTypeObject *self, /*String*/ DeeObject *name,
                       DeeObject *args, DeeObject *kw) {
	return DeeType_CallAttrTupleKw(self, name, args, kw);
}
#endif /* CONFIG_CALLTUPLE_OPTIMIZATIONS */

PRIVATE struct type_attr type_attr_data = {
	/* .tp_getattr                       = */ (DREF DeeObject *(DCALL *)(DeeObject *, /*String*/ DeeObject *))&type_getattr,
	/* .tp_delattr                       = */ (int (DCALL *)(DeeObject *, /*String*/ DeeObject *))&type_delattr,
	/* .tp_setattr                       = */ (int (DCALL *)(DeeObject *, /*String*/ DeeObject *, DeeObject *))&type_setattr,
	/* .tp_enumattr                      = */ (dssize_t (DCALL *)(DeeTypeObject *, DeeObject *, denum_t, void *))&type_enumattr,
	/* .tp_findattr                      = */ (int (DCALL *)(DeeTypeObject *, DeeObject *, struct attribute_info *__restrict, struct attribute_lookup_rules const *__restrict))&type_findattr,
	/* .tp_hasattr                       = */ (int (DCALL *)(DeeObject *, DeeObject *))&type_hasattr,
	/* .tp_boundattr                     = */ (int (DCALL *)(DeeObject *, DeeObject *))&type_boundattr,
	/* .tp_callattr                      = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *, size_t, DeeObject *const *))&type_callattr,
	/* .tp_callattr_kw                   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *, size_t, DeeObject *const *, DeeObject *))&type_callattr_kw,
	/* .tp_vcallattrf                    = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *, char const *, va_list))&type_vcallattrf,
	/* .tp_getattr_string_hash           = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, Dee_hash_t))&DeeType_GetAttrStringHash,
	/* .tp_delattr_string_hash           = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&DeeType_DelAttrStringHash,
	/* .tp_setattr_string_hash           = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t, DeeObject *))&DeeType_SetAttrStringHash,
	/* .tp_hasattr_string_hash           = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&DeeType_HasAttrStringHash,
	/* .tp_boundattr_string_hash         = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&DeeType_BoundAttrStringHash,
	/* .tp_callattr_string_hash          = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, Dee_hash_t, size_t, DeeObject *const *))&DeeType_CallAttrStringHash,
	/* .tp_callattr_string_hash_kw       = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, Dee_hash_t, size_t, DeeObject *const *, DeeObject *))&DeeType_CallAttrStringHashKw,
	/* .tp_vcallattr_string_hashf        = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, Dee_hash_t, char const *, va_list))&DeeType_VCallAttrStringHashf,
	/* .tp_getattr_string_len_hash       = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&DeeType_GetAttrStringLenHash,
	/* .tp_delattr_string_len_hash       = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&DeeType_DelAttrStringLenHash,
	/* .tp_setattr_string_len_hash       = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t, DeeObject *))&DeeType_SetAttrStringLenHash,
	/* .tp_hasattr_string_len_hash       = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&DeeType_HasAttrStringLenHash,
	/* .tp_boundattr_string_len_hash     = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&DeeType_BoundAttrStringLenHash,
	/* .tp_callattr_string_len_hash      = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t, size_t, DeeObject *const *))&DeeType_CallAttrStringLenHash,
	/* .tp_callattr_string_len_hash_kw   = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t, size_t, DeeObject *const *, DeeObject *))&DeeType_CallAttrStringLenHashKw,
	/* .tp_findattr_info_string_len_hash = */ (bool (DCALL *)(DeeTypeObject *, DeeObject *, char const *__restrict, size_t, Dee_hash_t, struct Dee_attrinfo *__restrict))&type_findattr_info_string_len_hash,
#ifdef CONFIG_CALLTUPLE_OPTIMIZATIONS
	/* .tp_callattr_tuple                = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *, DeeObject *))&type_callattr_tuple,
	/* .tp_callattr_tuple_kw             = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *, DeeObject *, DeeObject *))&type_callattr_tuple_kw,
#endif /* CONFIG_CALLTUPLE_OPTIMIZATIONS */
};

PRIVATE struct type_gc tpconst type_gc_data = {
	/* .tp_clear  = */ (void (DCALL *)(DeeObject *__restrict))&type_clear,
	/* .tp_pclear = */ (void (DCALL *)(DeeObject *__restrict, unsigned int))&type_pclear,
	/* .tp_gcprio = */ Dee_GC_PRIORITY_CLASS
};

PRIVATE DeeTypeObject *tpconst type_mro[] = {
	&DeeObject_Type,
	&DeeCallable_Type, /* Types can be called to invoke their constructor, so have them implement deemon.Callable. */
	NULL,
};

PUBLIC DeeTypeObject DeeType_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type), /* The type of Type is Type :D */
	/* .tp_name     = */ DeeString_STR(&str_Type),
	/* .tp_doc      = */ DOC("The so-called Type-Type, that is the type of anything that "
	                         /**/ "is also a Type, such as ?Dint or ?DList, or even itself"),
	/* .tp_flags    = */ TP_FGC | TP_FNAMEOBJECT,
	/* .tp_weakrefs = */ WEAKREF_SUPPORT_ADDR(DeeTypeObject),
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeObject_Type, /* class Type extends Object { ... } */
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&type_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)NULL,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)NULL,
				TYPE_FIXED_ALLOCATOR_GC(DeeTypeObject)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&type_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ (DeeObject *(DCALL *)(DeeObject *__restrict))&type_str,
		/* .tp_repr      = */ (DeeObject *(DCALL *)(DeeObject *__restrict))&type_repr,
		/* .tp_bool      = */ NULL,
		/* .tp_print     = */ (dssize_t (DCALL *)(DeeObject *__restrict, dformatprinter, void *))&DeeType_Print,
		/* .tp_printrepr = */ (dssize_t (DCALL *)(DeeObject *__restrict, dformatprinter, void *))&type_printrepr
	},
	/* .tp_call          = */ (DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *))&DeeObject_New,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&type_visit,
	/* .tp_gc            = */ &type_gc_data,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &type_cmp_,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ &type_attr_data,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ type_methods,
	/* .tp_getsets       = */ type_getsets,
	/* .tp_members       = */ type_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call_kw       = */ (DeeObject *(DCALL *)(DeeObject *, size_t, DeeObject *const *, DeeObject *))&DeeObject_NewKw,
	/* .tp_mro           = */ type_mro,
	/* .tp_operators     = */ type_operators,
	/* .tp_operators_size= */ COMPILER_LENOF(type_operators)
};



#ifdef CONFIG_TRACE_REFCHANGES
#ifndef CONFIG_NO_THREADS
PRIVATE Dee_atomic_lock_t reftracker_lock = DEE_ATOMIC_LOCK_INIT;
#endif /* !CONFIG_NO_THREADS */
#define reftracker_lock_available()  Dee_atomic_lock_available(&reftracker_lock)
#define reftracker_lock_acquired()   Dee_atomic_lock_acquired(&reftracker_lock)
#define reftracker_lock_tryacquire() Dee_atomic_lock_tryacquire(&reftracker_lock)
#define reftracker_lock_acquire()    Dee_atomic_lock_acquire(&reftracker_lock)
#define reftracker_lock_waitfor()    Dee_atomic_lock_waitfor(&reftracker_lock)
#define reftracker_lock_release()    Dee_atomic_lock_release(&reftracker_lock)

PRIVATE struct Dee_reftracker *reftracker_list = NULL;

/* #define REFLEAK_PRINTF(...) fprintf(stderr, __VA_ARGS__) */

#ifndef REFLEAK_PRINTF
#define REFLEAK_PRINT  Dee_DPRINT
#define REFLEAK_PRINTF Dee_DPRINTF
#endif /* !REFLEAK_PRINTF */
#ifndef REFLEAK_PRINT
#define REFLEAK_PRINT(str)  REFLEAK_PRINTF("%s", str)
#define REFLEAK_PRINTS(STR) REFLEAK_PRINTF("%s", STR)
#else /* !REFLEAK_PRINT */
#define REFLEAK_PRINTS Dee_DPRINT
#endif /* REFLEAK_PRINT */

PRIVATE NONNULL((1)) size_t DCALL
print_refchange_len(struct Dee_refchange *__restrict item) {
	size_t result;
	int line = item->rc_line;
	if (line < 0)
		line = -line;
	result = strlen(item->rc_file);
	if (line > 10)
		++result;
	if (line > 100)
		++result;
	if (line > 1000)
		++result;
	if (line > 10000)
		++result;
	if (line > 100000)
		++result;
	return result;
}

PRIVATE NONNULL((1)) Dee_refcnt_t DCALL
print_refchange(struct Dee_refchange *__restrict item,
                Dee_refcnt_t prev_total, size_t maxlen) {
	char mode[2] = { '+', 0 };
	Dee_refcnt_t count, next_total;
	size_t mylen;
	next_total = prev_total;
	if (item->rc_line < 0) {
		--next_total;
		mode[0] = '-';
	} else {
		++next_total;
	}
	REFLEAK_PRINTF("%s(%d) : ", item->rc_file,
	               item->rc_line < 0 ? -item->rc_line : item->rc_line);
	mylen = print_refchange_len(item);
	if (mylen < maxlen) {
		REFLEAK_PRINTF("%*s",
		               (int)(unsigned int)(maxlen - mylen),
		               " ");
	}
	REFLEAK_PRINTF("[%c][%" PRFuSIZ "->%" PRFuSIZ "]", mode[0], prev_total, next_total);
	count = next_total;
	if (count > 15)
		count = 15;
	while (count--)
		REFLEAK_PRINT(mode);
	REFLEAK_PRINTS("\n");
	return next_total;
}

PRIVATE NONNULL((1)) size_t DCALL
print_refchanges_len(struct Dee_refchanges *__restrict item) {
	unsigned int i;
	size_t result = 0;
	if (item->rc_prev)
		result = print_refchanges_len(item->rc_prev);
	for (i = 0; i < COMPILER_LENOF(item->rc_chng); ++i) {
		size_t temp;
		if (!item->rc_chng[i].rc_file)
			break;
		temp = print_refchange_len(&item->rc_chng[i]);
		if (result < temp)
			result = temp;
	}
	return result;
}

PRIVATE NONNULL((1)) Dee_refcnt_t DCALL
do_print_refchanges(struct Dee_refchanges *__restrict item,
                    Dee_refcnt_t prev_total, size_t maxlen) {
	unsigned int i;
	if (item->rc_prev)
		prev_total = do_print_refchanges(item->rc_prev, prev_total, maxlen);
	for (i = 0; i < COMPILER_LENOF(item->rc_chng); ++i) {
		if (!item->rc_chng[i].rc_file)
			break;
		prev_total = print_refchange(&item->rc_chng[i], prev_total, maxlen);
	}
	return prev_total;
}

PRIVATE NONNULL((1)) Dee_refcnt_t DCALL
print_refchanges(struct Dee_refchanges *__restrict item,
                 Dee_refcnt_t prev_total) {
	Dee_refcnt_t result;
	size_t maxlen;
	maxlen = print_refchanges_len(item);
	result = do_print_refchanges(item, prev_total, maxlen);
	return result;
}

INTERN NONNULL((1)) void DCALL
dump_reference_history(DeeObject *__restrict obj) {
	if (!obj->ob_trace)
		return;
	reftracker_lock_acquire();
	print_refchanges(obj->ob_trace->rt_last, 1);
	reftracker_lock_release();
}

PUBLIC void DCALL Dee_DumpReferenceLeaks(void) {
	struct Dee_reftracker *iter;
	reftracker_lock_acquire();
	for (iter = reftracker_list; iter; iter = iter->rt_next) {
		REFLEAK_PRINTF("Object at %p of instance %s leaked %" PRFuSIZ " references:\n",
		               iter->rt_obj, iter->rt_obj->ob_type->tp_name,
		               iter->rt_obj->ob_refcnt);
		print_refchanges(iter->rt_last, 1);
		REFLEAK_PRINTS("\n");
	}
	reftracker_lock_release();
}


PRIVATE NONNULL((1)) void DCALL
add_reftracker(struct Dee_reftracker *__restrict self) {
	reftracker_lock_acquire();
	self->rt_pself = &reftracker_list;
	if ((self->rt_next = reftracker_list) != NULL)
		reftracker_list->rt_pself = &self->rt_next;
	reftracker_list = self;
	reftracker_lock_release();
}

PRIVATE NONNULL((1)) void DCALL
del_reftracker(struct Dee_reftracker *__restrict self) {
	reftracker_lock_acquire();
	if ((*self->rt_pself = self->rt_next) != NULL)
		self->rt_next->rt_pself = self->rt_pself;
	reftracker_lock_release();
}

/* Reference count tracing. */
PRIVATE NONNULL((1)) void DCALL
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

#define DID_DEFINE_DEEOBJECT_FREETRACKER
PUBLIC ATTR_RETNONNULL NONNULL((1)) DeeObject *DCALL
DeeObject_FreeTracker(DeeObject *__restrict self) {
	free_reftracker(self->ob_trace);
	return self;
}

PRIVATE WUNUSED NONNULL((1)) struct Dee_reftracker *DCALL
get_reftracker(DeeObject *__restrict self) {
	struct Dee_reftracker *result, *new_result;
	result = self->ob_trace;
	if likely(result)
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
	new_result = atomic_cmpxch_val(&self->ob_trace, NULL, result);
	if unlikely(new_result != NULL) {
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

PRIVATE NONNULL((1)) void DCALL
reftracker_addchange(DeeObject *__restrict ob,
                     char const *file, int line) {
	unsigned int i;
	struct Dee_reftracker *self;
	struct Dee_refchanges *new_changes;
	struct Dee_refchanges *last;
	self = get_reftracker(ob);
	if unlikely(!self || self == DEE_REFTRACKER_UNTRACKED)
		return;
again:
	last = self->rt_last;
	for (i = 0; i < COMPILER_LENOF(last->rc_chng); ++i) {
		if (!atomic_cmpxch(&last->rc_chng[i].rc_file, NULL, file))
			continue;
		last->rc_chng[i].rc_line = line;
		return; /* Got it! */
	}

	/* Must allocate a new set of changes. */
	new_changes = (struct Dee_refchanges *)Dee_TryCalloc(sizeof(struct Dee_refchanges));
	if unlikely(!new_changes)
		return;
	new_changes->rc_chng[0].rc_file = file;
	new_changes->rc_chng[0].rc_line = line;
	new_changes->rc_prev            = last;

	/* Save the new set of changes as the latest set active. */
	if unlikely(!atomic_cmpxch_weak(&self->rt_last, last, new_changes)) {
		Dee_Free(new_changes);
		goto again;
	}
}


PUBLIC NONNULL((1)) void DCALL
Dee_Incref_traced(DeeObject *__restrict ob,
                  char const *file, int line) {
#ifndef CONFIG_NO_BADREFCNT_CHECKS
	if (atomic_fetchinc(&ob->ob_refcnt) == 0)
		DeeFatal_BadIncref(ob, file, line);
#else /* !CONFIG_NO_BADREFCNT_CHECKS */
	atomic_inc(&ob->ob_refcnt);
#endif /* CONFIG_NO_BADREFCNT_CHECKS */
	reftracker_addchange(ob, file, line);
}

PUBLIC NONNULL((1)) void DCALL
Dee_Incref_n_traced(DeeObject *__restrict ob, Dee_refcnt_t n,
                    char const *file, int line) {
#ifndef CONFIG_NO_BADREFCNT_CHECKS
	if (atomic_fetchadd(&ob->ob_refcnt, n) == 0)
		DeeFatal_BadIncref(ob, file, line);
#else /* !CONFIG_NO_BADREFCNT_CHECKS */
	atomic_add(&ob->ob_refcnt, n);
#endif /* CONFIG_NO_BADREFCNT_CHECKS */
	while (n--)
		reftracker_addchange(ob, file, line);
}

PUBLIC WUNUSED NONNULL((1)) bool DCALL
Dee_IncrefIfNotZero_traced(DeeObject *__restrict ob,
                           char const *file, int line) {
	Dee_refcnt_t oldref;
	do {
		if ((oldref = atomic_read(&ob->ob_refcnt)) == 0)
			return false;
	} while (!atomic_cmpxch_weak(&ob->ob_refcnt, oldref, oldref + 1));
	reftracker_addchange(ob, file, line);
	return true;
}

PUBLIC NONNULL((1)) void DCALL
Dee_Decref_traced(DeeObject *__restrict ob,
                  char const *file, int line) {
	Dee_refcnt_t oldref;
	oldref = atomic_fetchdec(&ob->ob_refcnt);
#ifndef CONFIG_NO_BADREFCNT_CHECKS
	if unlikely(oldref == 0)
		DeeFatal_BadDecref(ob, file, line);
#endif /* !CONFIG_NO_BADREFCNT_CHECKS */
	if unlikely(oldref == 1) {
		DeeObject_Destroy_d(ob, file, line);
	} else {
		reftracker_addchange(ob, file, -line);
	}
}

PUBLIC NONNULL((1)) void DCALL
Dee_Decref_n_traced(DeeObject *__restrict ob, Dee_refcnt_t n,
                    char const *file, int line) {
	Dee_refcnt_t oldref;
	oldref = atomic_fetchsub(&ob->ob_refcnt, n);
#ifndef CONFIG_NO_BADREFCNT_CHECKS
	if unlikely(oldref < n)
		DeeFatal_BadDecref(ob, file, line);
#endif /* !CONFIG_NO_BADREFCNT_CHECKS */
	if unlikely(oldref == n) {
		DeeObject_Destroy_d(ob, file, line);
	} else {
		reftracker_addchange(ob, file, -line);
	}
}

PUBLIC NONNULL((1)) void DCALL
Dee_DecrefDokill_traced(DeeObject *__restrict ob,
                        char const *file, int line) {
#ifndef CONFIG_NO_BADREFCNT_CHECKS
	if (atomic_fetchdec(&ob->ob_refcnt) != 1)
		DeeFatal_BadDecref(ob, file, line);
#else /* !CONFIG_NO_BADREFCNT_CHECKS */
	/* Without `CONFIG_NO_BADREFCNT_CHECKS', DeeObject_Destroy doesn't
	 * care about the final reference count, so no need for us to change it. */
#endif /* CONFIG_NO_BADREFCNT_CHECKS */
	DeeObject_Destroy_d(ob, file, line);
}

PUBLIC NONNULL((1)) void DCALL
Dee_DecrefNokill_traced(DeeObject *__restrict ob,
                        char const *file, int line) {
#ifndef CONFIG_NO_BADREFCNT_CHECKS
	if (atomic_fetchdec(&ob->ob_refcnt) <= 1)
		DeeFatal_BadDecref(ob, file, line);
#else /* !CONFIG_NO_BADREFCNT_CHECKS */
	atomic_dec(&ob->ob_refcnt);
#endif /* CONFIG_NO_BADREFCNT_CHECKS */
	reftracker_addchange(ob, file, -line);
}

PUBLIC WUNUSED NONNULL((1)) bool DCALL
Dee_DecrefIfOne_traced(DeeObject *__restrict ob,
                       char const *file, int line) {
	if (!atomic_cmpxch(&ob->ob_refcnt, 1, 0))
		return false;
	DeeObject_Destroy_d(ob, file, line);
	return true;
}

PUBLIC WUNUSED NONNULL((1)) bool DCALL
Dee_DecrefIfNotOne_traced(DeeObject *__restrict ob,
                          char const *file, int line) {
	Dee_refcnt_t oldref;
	do {
		if ((oldref = atomic_read(&ob->ob_refcnt)) <= 1)
			return false;
	} while (!atomic_cmpxch_weak(&ob->ob_refcnt, oldref, oldref - 1));
	reftracker_addchange(ob, file, -line);
	return true;
}

PUBLIC WUNUSED NONNULL((1)) bool DCALL
Dee_DecrefWasOk_traced(DeeObject *__restrict ob,
                       char const *file, int line) {
	Dee_refcnt_t newref;
	newref = atomic_fetchdec(&ob->ob_refcnt);
#ifndef CONFIG_NO_BADREFCNT_CHECKS
	if unlikely(newref == 0)
		DeeFatal_BadDecref(ob, file, line);
#endif /* !CONFIG_NO_BADREFCNT_CHECKS */
	if unlikely(newref == 1) {
		DeeObject_Destroy_d(ob, file, line);
		return true;
	}
	reftracker_addchange(ob, file, -line);
	return false;
}

PUBLIC ATTR_RETNONNULL ATTR_INS(1, 2) DREF DeeObject **
(DCALL Dee_Increfv_traced)(DeeObject *const *__restrict object_vector,
                           size_t object_count, char const *file, int line) {
	size_t i;
	for (i = 0; i < object_count; ++i) {
		DREF DeeObject *ob;
		ob = object_vector[i];
		Dee_Incref_traced(ob, file, line);
	}
	return (DREF DeeObject **)object_vector;
}
PUBLIC ATTR_RETNONNULL ATTR_INS(1, 2) DeeObject **
(DCALL Dee_Decrefv_traced)(DREF DeeObject *const *__restrict object_vector,
                           size_t object_count, char const *file, int line) {
	while (object_count--) {
		DREF DeeObject *ob;
		ob = object_vector[object_count];
		Dee_Decref_traced(ob, file, line);
	}
	return (DREF DeeObject **)object_vector;
}
PUBLIC ATTR_RETNONNULL ATTR_OUTS(1, 3) ATTR_INS(2, 3) DREF DeeObject **
(DCALL Dee_Movrefv_traced)(/*out:ref*/ DeeObject **__restrict dst,
                           /*in*/ DeeObject *const *__restrict src,
                           size_t object_count, char const *file, int line) {
	size_t i;
	for (i = 0; i < object_count; ++i) {
		DREF DeeObject *ob;
		ob = src[i];
		Dee_Incref_traced(ob, file, line);
		dst[i] = ob;
	}
	return dst;
}

PUBLIC ATTR_RETNONNULL ATTR_OUTS(1, 3) NONNULL((2)) DREF DeeObject **
(DCALL Dee_Setrefv_traced)(/*out:ref*/ DeeObject **__restrict dst,
                           /*in*/ DeeObject *obj, size_t object_count,
                           char const *file, int line) {
	Dee_Incref_n_traced(obj, object_count, file, line);
	return (DREF DeeObject **)memsetp(dst, obj, object_count);
}
#else /* CONFIG_TRACE_REFCHANGES */

/* Maintain ABI compatibility by always providing traced variants of functions! */


#ifndef CONFIG_NO_BADREFCNT_CHECKS
/* Can still re-use debug information if bad-refcnt checks are enabled. */
#undef DeeObject_Destroy
#undef _DeeFatal_BadIncref
#undef _DeeFatal_BadDecref
#define DeeObject_Destroy(self) DeeObject_Destroy_d(self, file, line)
#define _DeeFatal_BadIncref(ob) DeeFatal_BadIncref((DeeObject *)(ob), file, line)
#define _DeeFatal_BadDecref(ob) DeeFatal_BadDecref((DeeObject *)(ob), file, line)
#endif /* !CONFIG_NO_BADREFCNT_CHECKS */

PUBLIC NONNULL((1)) void
(DCALL Dee_Incref_traced)(DeeObject *__restrict ob,
                          char const *file,
                          int line) {
	(void)file;
	(void)line;
	Dee_Incref(ob);
}

PUBLIC NONNULL((1)) void
(DCALL Dee_Incref_n_traced)(DeeObject *__restrict ob, Dee_refcnt_t n,
                            char const *file,
                            int line) {
	(void)file;
	(void)line;
	Dee_Incref_n(ob, n);
}

PUBLIC WUNUSED NONNULL((1)) bool
(DCALL Dee_IncrefIfNotZero_traced)(DeeObject *__restrict ob,
                                   char const *file,
                                   int line) {
	(void)file;
	(void)line;
	return Dee_IncrefIfNotZero(ob);
}

PUBLIC NONNULL((1)) void
(DCALL Dee_Decref_traced)(DeeObject *__restrict ob,
                          char const *file,
                          int line) {
	(void)file;
	(void)line;
	Dee_Decref(ob);
}

PUBLIC NONNULL((1)) void
(DCALL Dee_DecrefDokill_traced)(DeeObject *__restrict ob,
                                char const *file,
                                int line) {
	(void)file;
	(void)line;
	Dee_DecrefDokill(ob);
}

PUBLIC NONNULL((1)) void
(DCALL Dee_DecrefNokill_traced)(DeeObject *__restrict ob,
                                char const *file,
                                int line) {
	(void)file;
	(void)line;
	Dee_DecrefNokill(ob);
}

PUBLIC WUNUSED NONNULL((1)) bool
(DCALL Dee_DecrefIfOne_traced)(DeeObject *__restrict ob,
                               char const *file,
                               int line) {
	(void)file;
	(void)line;
	return Dee_DecrefIfOne(ob);
}

PUBLIC WUNUSED NONNULL((1)) bool
(DCALL Dee_DecrefIfNotOne_traced)(DeeObject *__restrict ob,
                                  char const *file,
                                  int line) {
	(void)file;
	(void)line;
	return Dee_DecrefIfNotOne(ob);
}

PUBLIC WUNUSED NONNULL((1)) bool
(DCALL Dee_DecrefWasOk_traced)(DeeObject *__restrict ob,
                               char const *file,
                               int line) {
	(void)file;
	(void)line;
	return Dee_DecrefWasOk(ob);
}

PUBLIC ATTR_RETNONNULL ATTR_INS(1, 2) DREF DeeObject **
(DCALL Dee_Increfv_traced)(DeeObject *const *__restrict object_vector,
                           size_t object_count,
                           char const *file, int line) {
#ifdef CONFIG_NO_BADREFCNT_CHECKS
	(void)file;
	(void)line;
	return Dee_Increfv(object_vector, object_count);
#else /* CONFIG_NO_BADREFCNT_CHECKS */
	size_t i;
	(void)file;
	(void)line;
	for (i = 0; i < object_count; ++i) {
		DREF DeeObject *ob;
		ob = object_vector[i];
		Dee_Incref(ob);
	}
	return (DREF DeeObject **)object_vector;
#endif /* !CONFIG_NO_BADREFCNT_CHECKS */
}

PUBLIC ATTR_RETNONNULL ATTR_INS(1, 2) DeeObject **
(DCALL Dee_Decrefv_traced)(DREF DeeObject *const *__restrict object_vector,
                           size_t object_count,
                           char const *file, int line) {
	(void)file;
	(void)line;
#ifdef CONFIG_NO_BADREFCNT_CHECKS
	return Dee_Decrefv(object_vector, object_count);
#else /* CONFIG_NO_BADREFCNT_CHECKS */
	while (object_count--) {
		DREF DeeObject *ob;
		ob = object_vector[object_count];
		Dee_Decref(ob);
	}
	return (DREF DeeObject **)object_vector;
#endif /* !CONFIG_NO_BADREFCNT_CHECKS */
}

PUBLIC ATTR_RETNONNULL ATTR_OUTS(1, 3) ATTR_INS(2, 3) DREF DeeObject **
(DCALL Dee_Movrefv_traced)(/*out:ref*/ DeeObject **__restrict dst,
                           /*in*/ DeeObject *const *__restrict src,
                           size_t object_count,
                           char const *file, int line) {
#ifdef CONFIG_NO_BADREFCNT_CHECKS
	(void)file;
	(void)line;
	return Dee_Movrefv(dst, src, object_count);
#else /* CONFIG_NO_BADREFCNT_CHECKS */
	size_t i;
	(void)file;
	(void)line;
	for (i = 0; i < object_count; ++i) {
		DREF DeeObject *ob;
		ob = src[i];
		Dee_Incref(ob);
		dst[i] = ob;
	}
	return dst;
#endif /* !CONFIG_NO_BADREFCNT_CHECKS */
}

PUBLIC ATTR_RETNONNULL ATTR_OUTS(1, 3) NONNULL((2)) DREF DeeObject **
(DCALL Dee_Setrefv_traced)(/*out:ref*/ DeeObject **__restrict dst,
                           /*in*/ DeeObject *obj, size_t object_count,
                           char const *file, int line) {
	(void)file;
	(void)line;
#ifdef CONFIG_NO_BADREFCNT_CHECKS
	return Dee_Setrefv(dst, obj, object_count);
#else /* CONFIG_NO_BADREFCNT_CHECKS */
	Dee_Incref_n_untraced(obj, object_count);
	return (DREF DeeObject **)memsetp(dst, obj, object_count);
#endif /* !CONFIG_NO_BADREFCNT_CHECKS */
}


DFUNDEF void (DCALL Dee_DumpReferenceLeaks)(void);
PUBLIC void (DCALL Dee_DumpReferenceLeaks)(void) {
	COMPILER_IMPURE();
}

#ifndef CONFIG_NO_BADREFCNT_CHECKS
#undef DeeObject_Destroy
#undef _DeeFatal_BadIncref
#undef _DeeFatal_BadDecref
#define DeeObject_Destroy(self) DeeObject_Destroy_d(self, __FILE__, __LINE__)
#define _DeeFatal_BadIncref(ob) DeeFatal_BadIncref((DeeObject *)(ob), __FILE__, __LINE__)
#define _DeeFatal_BadDecref(ob) DeeFatal_BadDecref((DeeObject *)(ob), __FILE__, __LINE__)
#endif /* !CONFIG_NO_BADREFCNT_CHECKS */
#endif /* !CONFIG_TRACE_REFCHANGES */

/* Also export all the reference-control macros as functions. */
PUBLIC NONNULL((1)) void
(DCALL Dee_Incref)(DeeObject *__restrict ob) {
	Dee_Incref_untraced(ob);
}

PUBLIC NONNULL((1)) void
(DCALL Dee_Incref_n)(DeeObject *__restrict ob, Dee_refcnt_t n) {
	Dee_Incref_n_untraced(ob, n);
}

PUBLIC WUNUSED NONNULL((1)) bool
(DCALL Dee_IncrefIfNotZero)(DeeObject *__restrict ob) {
	return Dee_IncrefIfNotZero_untraced(ob);
}

PUBLIC NONNULL((1)) void
(DCALL Dee_Decref)(DeeObject *__restrict ob) {
	Dee_Decref_untraced(ob);
}

PUBLIC NONNULL((1)) void
(DCALL Dee_DecrefDokill)(DeeObject *__restrict ob) {
	Dee_DecrefDokill_untraced(ob);
}

PUBLIC NONNULL((1)) void
(DCALL Dee_DecrefNokill)(DeeObject *__restrict ob) {
	Dee_DecrefNokill_untraced(ob);
}

PUBLIC WUNUSED NONNULL((1)) bool
(DCALL Dee_DecrefIfOne)(DeeObject *__restrict ob) {
	return Dee_DecrefIfOne_untraced(ob);
}

PUBLIC WUNUSED NONNULL((1)) bool
(DCALL Dee_DecrefIfNotOne)(DeeObject *__restrict ob) {
	return Dee_DecrefIfNotOne_untraced(ob);
}

PUBLIC WUNUSED NONNULL((1)) bool
(DCALL Dee_DecrefWasOk)(DeeObject *__restrict ob) {
	return Dee_DecrefWasOk_untraced(ob);
}


/* incref() + return `self' (may be used in type operators,
 * and receives special optimizations in some situations) */
PUBLIC ATTR_RETNONNULL NONNULL((1)) DREF DeeObject *
(DCALL DeeObject_NewRef)(DeeObject *__restrict self) {
	ASSERT_OBJECT(self);
	Dee_Incref(self);
	return self;
}

PUBLIC ATTR_RETNONNULL NONNULL((1)) DREF DeeObject *
(DCALL DeeObject_NewRef_traced)(DeeObject *__restrict self,
                                char const *file, int line) {
	(void)file;
	(void)line;
	ASSERT_OBJECT_AT(self, file, line);
	Dee_Incref_traced(self, file, line);
	return self;
}



/* Increment the reference counter of every object from `object_vector...+=object_count'
 * @return: * : Always re-returns the pointer to `object_vector' */
PUBLIC ATTR_RETNONNULL ATTR_INS(1, 2) DREF DeeObject **
(DCALL Dee_Increfv)(DeeObject *const *__restrict object_vector,
                    size_t object_count) {
	while (object_count--) {
		DREF DeeObject *ob;
		ob = object_vector[object_count];
		Dee_Incref_untraced(ob);
	}
	return (DREF DeeObject **)object_vector;
}

/* Decrement the reference counter of every object from `object_vector...+=object_count'
 * @return: * : Always re-returns the pointer to `object_vector' */
PUBLIC ATTR_RETNONNULL ATTR_INS(1, 2) DeeObject **
(DCALL Dee_Decrefv)(DREF DeeObject *const *__restrict object_vector,
                    size_t object_count) {
	size_t i;
	for (i = 0; i < object_count; ++i) {
		DREF DeeObject *ob;
		ob = object_vector[i];
		Dee_Decref_untraced(ob);
	}
	return (DREF DeeObject **)object_vector;
}

/* Copy object pointers from `src' to `dst' and increment
 * the reference counter of every object that got copied.
 * @return: * : Always re-returns the pointer to `dst' */
PUBLIC ATTR_RETNONNULL ATTR_OUTS(1, 3) ATTR_INS(2, 3) DREF DeeObject **
(DCALL Dee_Movrefv)(/*out:ref*/ DeeObject **__restrict dst,
                    /*in*/ DeeObject *const *__restrict src,
                    size_t object_count) {
	while (object_count--) {
		DREF DeeObject *ob;
		ob = src[object_count];
		Dee_Incref_untraced(ob);
		dst[object_count] = ob;
	}
	return dst;
}

/* Fill object pointers in `dst' with `obj' and increment
 * the reference counter of `obj' accordingly.
 * @return: * : Always re-returns the pointer to `dst' */
PUBLIC ATTR_RETNONNULL ATTR_OUTS(1, 3) NONNULL((2)) DREF DeeObject **
(DCALL Dee_Setrefv)(/*out:ref*/ DeeObject **__restrict dst,
                    /*in*/ DeeObject *obj, size_t object_count) {
	Dee_Incref_n_untraced(obj, object_count);
	return (DREF DeeObject **)memsetp(dst, obj, object_count);
}



#ifndef DID_DEFINE_DEEOBJECT_FREETRACKER
#define DID_DEFINE_DEEOBJECT_FREETRACKER
PUBLIC ATTR_RETNONNULL NONNULL((1)) DeeObject * /* Defined only for binary compatibility */
(DCALL DeeObject_FreeTracker)(DeeObject *__restrict self) {
	COMPILER_IMPURE();
	return self;
}
#endif /* !DID_DEFINE_DEEOBJECT_FREETRACKER */



#ifndef NDEBUG
PRIVATE void DCALL
assert_badobject_impl(char const *check_name,
                      char const *file,
                      int line, DeeObject const *ob) {
	if (!ob) {
		_DeeAssert_Failf(check_name, file, line,
		                 "Bad object at %p is a NULL pointer",
		                 ob);
	} else if (!ob->ob_refcnt) {
		char const *type_name = "?";
		if (DeeObject_Check(ob->ob_type))
			type_name = ob->ob_type->tp_name;
		_DeeAssert_Failf(check_name, file, line,
		                 "Bad object at %p (instance of '%s') has a reference count of 0",
		                 ob, type_name);
	} else if (!ob->ob_type) {
		_DeeAssert_Failf(check_name, file, line,
		                 "Bad object at %p (%" PRFuSIZ " references) has a NULL-pointer as type",
		                 ob, ob->ob_refcnt);
	} else if (!ob->ob_type->ob_refcnt) {
		_DeeAssert_Failf(check_name, file, line,
		                 "Bad object at %p (instance of '%s', %" PRFuSIZ " references) has a type with a reference counter of 0",
		                 ob, ob->ob_type->tp_name, ob->ob_refcnt);
	} else {
		char const *type_name = "?";
		if (DeeObject_Check(ob->ob_type))
			type_name = ob->ob_type->tp_name;
		_DeeAssert_Failf(check_name, file, line,
		                 "Bad object at %p (instance of '%s', %" PRFuSIZ " references)",
		                 ob, type_name, ob->ob_refcnt);
	}
}

PRIVATE void DCALL
assert_badtype_impl(char const *check_name, char const *file,
                    int line, DeeObject const *ob, bool wanted_exact,
                    DeeTypeObject const *wanted_type) {
	char const *is_exact = wanted_exact ? " an exact " : " an ";
	if (!ob) {
		_DeeAssert_Failf(check_name, file, line,
		                 "Bad object at %p is a NULL pointer when%sinstance of '%s' was needed",
		                 ob, is_exact, wanted_type->tp_name);
	} else if (!ob->ob_refcnt) {
		char const *type_name = "?";
		if (DeeObject_Check(ob->ob_type))
			type_name = ob->ob_type->tp_name;
		_DeeAssert_Failf(check_name, file, line,
		                 "Bad object at %p (instance of '%s') has a reference "
		                 "count of 0 when%sinstance of '%s' was needed",
		                 ob, type_name, is_exact, wanted_type->tp_name);
	} else if (!ob->ob_type) {
		_DeeAssert_Failf(check_name, file, line,
		                 "Bad object at %p (%" PRFuSIZ " references) has a NULL-pointer "
		                 "as type when%sinstance of '%s' was needed",
		                 ob, ob->ob_refcnt, is_exact, wanted_type->tp_name);
	} else if (!ob->ob_type->ob_refcnt) {
		_DeeAssert_Failf(check_name, file, line,
		                 "Bad object at %p (instance of '%s', %" PRFuSIZ " references) has a type "
		                 "with a reference counter of 0 when%sinstance of '%s' was needed",
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
			                 "Bad object at %p (instance of '%s', %" PRFuSIZ " references) "
			                 "when%sinstance of '%s' was needed\n"
			                 "repr: %r",
			                 ob, type_name, ob->ob_refcnt, is_exact,
			                 wanted_type->tp_name, ob);
		} else {
			_DeeAssert_Failf(check_name, file, line,
			                 "Bad object at %p (instance of '%s', %" PRFuSIZ " references) "
			                 "when%sinstance of '%s' was needed",
			                 ob, type_name, ob->ob_refcnt, is_exact, wanted_type->tp_name);
		}
	}
}

PUBLIC void DCALL
DeeAssert_BadObject(DeeObject const *ob, char const *file, int line) {
	assert_badobject_impl("ASSERT_OBJECT", file, line, ob);
}

PUBLIC void DCALL
DeeAssert_BadObjectOpt(DeeObject const *ob, char const *file, int line) {
	assert_badobject_impl("ASSERT_OBJECT_OPT", file, line, ob);
}

PUBLIC void DCALL
DeeAssert_BadObjectType(DeeObject const *ob,
                        DeeTypeObject const *wanted_type,
                        char const *file, int line) {
	assert_badtype_impl("ASSERT_OBJECT_TYPE", file, line, ob, false, wanted_type);
}

PUBLIC void DCALL
DeeAssert_BadObjectTypeOpt(DeeObject const *ob,
                           DeeTypeObject const *wanted_type,
                           char const *file, int line) {
	assert_badtype_impl("ASSERT_OBJECT_TYPE_OPT", file, line, ob, false, wanted_type);
}

PUBLIC void DCALL
DeeAssert_BadObjectTypeExact(DeeObject const *ob,
                             DeeTypeObject const *wanted_type,
                             char const *file, int line) {
	assert_badtype_impl("ASSERT_OBJECT_TYPE_EXACT", file, line, ob, true, wanted_type);
}

PUBLIC void DCALL
DeeAssert_BadObjectTypeExactOpt(DeeObject const *ob,
                                DeeTypeObject const *wanted_type,
                                char const *file, int line) {
	assert_badtype_impl("ASSERT_OBJECT_TYPE_EXACT_OPT", file, line, ob, true, wanted_type);
}

#else /* !NDEBUG */

PUBLIC void DCALL
DeeAssert_BadObject(DeeObject const *ob,
                    char const *file, int line) {
	(void)ob;
	(void)file;
	(void)line;
	COMPILER_IMPURE();
	/* no-op */
}

PUBLIC void DCALL
DeeAssert_BadObjectOpt(DeeObject const *ob,
                       char const *file, int line) {
	(void)ob;
	(void)file;
	(void)line;
	COMPILER_IMPURE();
	/* no-op */
}

PUBLIC void DCALL
DeeAssert_BadObjectType(DeeObject const *ob,
                        DeeTypeObject const *wanted_type,
                        char const *file, int line) {
	(void)ob;
	(void)wanted_type;
	(void)file;
	(void)line;
	COMPILER_IMPURE();
	/* no-op */
}

PUBLIC void DCALL
DeeAssert_BadObjectTypeOpt(DeeObject const *ob,
                           DeeTypeObject const *wanted_type,
                           char const *file, int line) {
	(void)ob;
	(void)wanted_type;
	(void)file;
	(void)line;
	COMPILER_IMPURE();
	/* no-op */
}

PUBLIC void DCALL
DeeAssert_BadObjectTypeExact(DeeObject const *ob,
                             DeeTypeObject const *wanted_type,
                             char const *file, int line) {
	(void)ob;
	(void)wanted_type;
	(void)file;
	(void)line;
	COMPILER_IMPURE();
	/* no-op */
}

PUBLIC void DCALL
DeeAssert_BadObjectTypeExactOpt(DeeObject const *ob,
                                DeeTypeObject const *wanted_type,
                                char const *file, int line) {
	(void)ob;
	(void)wanted_type;
	(void)file;
	(void)line;
	COMPILER_IMPURE();
	/* no-op */
}

#endif /* NDEBUG */

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_OBJECT_C */
