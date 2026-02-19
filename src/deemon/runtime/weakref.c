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
#ifndef GUARD_DEEMON_RUNTIME_WEAKREF_C
#define GUARD_DEEMON_RUNTIME_WEAKREF_C 1

#include <deemon/api.h>

#include <deemon/none.h>         /* DeeNoneObject, DeeNone_Type */
#include <deemon/object.h>       /* ASSERT_OBJECT, ASSERT_OBJECT_OPT, DREF, DeeObject, DeeTypeObject, Dee_AsObject, Dee_IncrefIfNotZero, Dee_REFTRACKER_UNTRACKED, Dee_TYPE, Dee_WEAKREF_SUPPORT_INIT, Dee_refcnt_t, Dee_unlockinfo, Dee_unlockinfo_xunlock, Dee_weakref_list, ITER_DONE, OBJECT_HEAD */
#include <deemon/type.h>         /* DeeType_Base */
#include <deemon/util/atomic.h>  /* atomic_* */
#include <deemon/util/weakref.h> /* Dee_weakref, Dee_weakref_callback_t */

#include <hybrid/align.h>       /* IS_ALIGNED */
#include <hybrid/sched/yield.h> /* SCHED_YIELD */
#include <hybrid/typecore.h>    /* __SIZEOF_POINTER__ */

#include <stdbool.h> /* bool, false, true */
#include <stddef.h>  /* NULL, size_t */
#include <stdint.h>  /* UINT32_C, UINT64_C, uintptr_t */

#undef container_of
#define container_of COMPILER_CONTAINER_OF

DECL_BEGIN

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
LOCAL WUNUSED NONNULL((1)) struct Dee_weakref_list *DCALL
weakrefs_get(DeeObject *__restrict ob) {
	DeeTypeObject *tp = Dee_TYPE(ob);
#ifdef CONFIG_INHERIT_WEAKREF_SUPPORT_ADDRESS
	if unlikely(!tp->tp_weakrefs)
		type_inherit_weakrefs(tp);
#else /* CONFIG_INHERIT_WEAKREF_SUPPORT_ADDRESS */
	while (!tp->tp_weakrefs && DeeType_Base(tp))
		tp = DeeType_Base(tp);
#endif /* !CONFIG_INHERIT_WEAKREF_SUPPORT_ADDRESS */
	return (struct Dee_weakref_list *)((uintptr_t)ob + tp->tp_weakrefs);
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
#define WEAKREF_EMPTY_NEXTVAL ((struct Dee_weakref *)((uintptr_t)WEAKREF_BAD_POINTER & PTRLOCK_ADDR_MASK))
#define WEAKREF_SETBAD(x, T)  ((x) = (T)WEAKREF_BAD_POINTER)
#endif /* !NDEBUG */


#ifdef __INTELLISENSE__
PUBLIC NONNULL((1, 2)) bool
(DCALL Dee_weakref_init)(struct Dee_weakref *__restrict self,
                         DeeObject *__restrict ob,
                         Dee_weakref_callback_t callback)
#else /* __INTELLISENSE__ */
PUBLIC NONNULL((1, 2)) bool
(DCALL Dee_weakref_init)(struct Dee_weakref *__restrict self,
                         DeeObject *__restrict ob)
#endif /* !__INTELLISENSE__ */
{
	struct Dee_weakref_list *list;
	struct Dee_weakref *next;
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
	next = (struct Dee_weakref *)GET_POINTER(list->wl_nodes);
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

PRIVATE NONNULL((3)) bool DCALL
Dee_weakref_initmany_contains_ob(struct Dee_weakref *const *weakref_v,
                                 size_t weakref_c, DeeObject *ob) {
	size_t i;
	for (i = 0; i < weakref_c; ++i) {
		struct Dee_weakref *wref = weakref_v[i];
		if (wref->wr_obj == ob)
			return true;
	}
	return false;
}

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
PUBLIC WUNUSED bool DCALL
Dee_weakref_initmany_trylock(struct Dee_weakref *const *weakref_v, size_t weakref_c,
                             struct Dee_unlockinfo *unlock) {
	size_t i;
	for (i = 0; i < weakref_c; ++i) {
		struct Dee_weakref *wref = weakref_v[i];
		DeeObject *ob = wref->wr_obj;
		struct Dee_weakref_list *list = WEAKREFS_GET(ob);
		struct Dee_weakref *next;
		if (!TRYLOCK_POINTER(list->wl_nodes)) {
			if (Dee_weakref_initmany_contains_ob(weakref_v, weakref_c, ob))
				continue;
			Dee_weakref_initmany_unlock(weakref_v, i);
			Dee_unlockinfo_xunlock(unlock);
			WAITFOR_POINTER(list->wl_nodes);
			return false;
		}
		next = (struct Dee_weakref *)GET_POINTER(list->wl_nodes);
		if (next && unlikely(!WEAKREF_TRYLOCK(next))) {
			UNLOCK_POINTER(list->wl_nodes);
			Dee_weakref_initmany_unlock(weakref_v, i);
			Dee_unlockinfo_xunlock(unlock);
			SCHED_YIELD();
			return false;
		}
	}
	return true;
}

PUBLIC void DCALL
Dee_weakref_initmany_lock(struct Dee_weakref *const *weakref_v, size_t weakref_c) {
	while (!Dee_weakref_initmany_trylock(weakref_v, weakref_c, NULL)) {
		/* ... */
	}
}

PUBLIC void DCALL
Dee_weakref_initmany_exec(struct Dee_weakref *const *weakref_v, size_t weakref_c) {
	size_t i;
	for (i = 0; i < weakref_c; ++i) {
		struct Dee_weakref *wref = weakref_v[i];
		DeeObject *ob = wref->wr_obj;
		struct Dee_weakref_list *list = WEAKREFS_GET(ob);
		struct Dee_weakref *next = (struct Dee_weakref *)GET_POINTER(list->wl_nodes);
		wref->wr_pself = &list->wl_nodes;
		wref->wr_next = (struct Dee_weakref *)((uintptr_t)next | PTRLOCK_LOCK_MASK);
		if (next) {
			ASSERT(next->wr_pself == &list->wl_nodes);
			ASSERT(next->wr_obj == ob);
			next->wr_pself = &wref->wr_next;
			WEAKREF_UNLOCK(next);
		}
	}
}

PUBLIC DREF DeeObject **DCALL
Dee_weakref_initmany_unlock_and_inherit(struct Dee_weakref **weakref_v, size_t weakref_c) {
	DREF DeeObject **result = (DREF DeeObject **)weakref_v;
	while (weakref_c--) {
		struct Dee_weakref *wref = weakref_v[weakref_c];
		DREF DeeObject *ob = wref->wr_obj;
		struct Dee_weakref_list *list;
		struct Dee_weakref *next;
		result[weakref_c] = ob; /* Inherit reference */
		if (Dee_weakref_initmany_contains_ob(weakref_v, weakref_c, ob))
			continue; /* Happens later... */
		list = WEAKREFS_GET(ob);
		next = (struct Dee_weakref *)GET_POINTER(list->wl_nodes);
		ASSERTF(next, "How is there no node? We just added (at least) one!");
		ASSERT(next->wr_pself == &list->wl_nodes);
		ASSERT(next->wr_obj == ob);
		WEAKREF_UNLOCK(next);
		UNLOCK_POINTER(list->wl_nodes);
	}
	return result;
}

PUBLIC void DCALL
Dee_weakref_initmany_unlock(struct Dee_weakref *const *weakref_v, size_t weakref_c) {
	while (weakref_c--) {
		struct Dee_weakref *wref = weakref_v[weakref_c];
		DeeObject *ob = wref->wr_obj;
		struct Dee_weakref_list *list;
		struct Dee_weakref *next;
		if (Dee_weakref_initmany_contains_ob(weakref_v, weakref_c, ob))
			continue; /* Happens later... */
		list = WEAKREFS_GET(ob);
		next = (struct Dee_weakref *)GET_POINTER(list->wl_nodes);
		if (next)
			WEAKREF_UNLOCK(next);
		UNLOCK_POINTER(list->wl_nodes);
	}
}



#ifdef __INTELLISENSE__
PUBLIC NONNULL((1, 2)) void
(DCALL Dee_weakref_copy)(struct Dee_weakref *__restrict self,
                         struct Dee_weakref const *__restrict other)
#else /* __INTELLISENSE__ */
PUBLIC NONNULL((1, 2)) void
(DCALL Dee_weakref_copy)(struct Dee_weakref *__restrict self,
                         struct Dee_weakref *__restrict other)
#endif /* !__INTELLISENSE__ */
{
	ASSERT(self != other);
#ifndef NDEBUG
	ASSERT(other->wr_obj != (DeeObject *)WEAKREF_BAD_POINTER);
#endif /* !NDEBUG */
	WEAKREF_SETBAD(self->wr_pself, struct Dee_weakref **);
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
			self->wr_next  = (struct Dee_weakref *)other;
			self->wr_obj   = other->wr_obj;
			((struct Dee_weakref *)other)->wr_pself = &self->wr_next;
			WEAKREF_UNLOCK(other);     /* WEAKREF_UNLOCK(other); */
			WEAKREF_PREV_UNLOCK(self); /* WEAKREF_UNLOCK(other->PREV); */
		} else {
			ASSERT(other->wr_next == (struct Dee_weakref *)PTRLOCK_LOCK_MASK);
			atomic_write(&((struct Dee_weakref *)other)->wr_next, WEAKREF_EMPTY_NEXTVAL); /* WEAKREF_UNLOCK(other); */
			goto set_dst_empty;
		}
	} else {
set_dst_empty:
		self->wr_obj  = NULL;
		self->wr_next = WEAKREF_EMPTY_NEXTVAL;
	}
}

PUBLIC NONNULL((1, 2)) void DCALL
Dee_weakref_move(struct Dee_weakref *__restrict dst,
                 struct Dee_weakref *__restrict src) {
	ASSERT(dst != src);
#ifndef NDEBUG
	ASSERT(src->wr_obj != (DeeObject *)WEAKREF_BAD_POINTER);
#endif /* !NDEBUG */
	dst->wr_del = src->wr_del;
again:
	WEAKREF_SETBAD(dst->wr_pself, struct Dee_weakref **);
	if (src->wr_obj) {
		WEAKREF_LOCK(src);
		COMPILER_READ_BARRIER();
		if likely(src->wr_obj) {
			struct Dee_weakref *next;
			if unlikely(TRYLOCK_POINTER(*src->wr_pself)) {
				/* Prevent a deadlock. */
				WEAKREF_UNLOCK(src);
				SCHED_YIELD();
				goto again;
			}
			next          = (struct Dee_weakref *)GET_POINTER(src->wr_next);
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
(DCALL Dee_weakref_copyassign)(struct Dee_weakref *self,
                               struct Dee_weakref const *other)
#else /* __INTELLISENSE__ */
PUBLIC NONNULL((1, 2)) void
(DCALL Dee_weakref_copyassign)(struct Dee_weakref *self,
                               struct Dee_weakref *other)
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
				struct Dee_weakref **other_pself = other->wr_pself;
				LOCAL_UNLOCK_other__prev();
				LOCAL_UNLOCK_other();
				if (other_pself == &self->wr_next)
					return; /* Special case: `self' is the predecessor of `other' */
				WEAKREF_WAITFOR(self);
				goto again;
			}
			COMPILER_READ_BARRIER();
			if (self->wr_obj) {
				struct Dee_weakref *old_self_next;
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
				old_self_next = (struct Dee_weakref *)GET_POINTER(self->wr_next);
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
			((struct Dee_weakref *)other)->wr_pself = &self->wr_next;
			WEAKREF_PREV_UNLOCK(self);           /* LOCAL_UNLOCK_other__prev(); */
			LOCAL_UNLOCK_other();
			atomic_write(&self->wr_next, other); /* LOCAL_UNLOCK_self(); */
#undef LOCAL_UNLOCK_self
#undef LOCAL_UNLOCK_other__prev
		} else {
			atomic_write(&((struct Dee_weakref *)other)->wr_next, WEAKREF_EMPTY_NEXTVAL); /* LOCAL_UNLOCK_other(); */
			Dee_weakref_clear(self);
		}
#undef LOCAL_UNLOCK_other
	} else {
		Dee_weakref_clear(self);
	}
}

PUBLIC NONNULL((1, 2)) void DCALL
Dee_weakref_moveassign(struct Dee_weakref *self,
                       struct Dee_weakref *other) {
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
			struct Dee_weakref *next;
#define LOCAL_UNLOCK_other__prev() atomic_write(other->wr_pself, other)
			if unlikely(!WEAKREF_PREV_TRYLOCK(other)) {
				LOCAL_UNLOCK_other();
				SCHED_YIELD();
				goto again;
			}
			next = (struct Dee_weakref *)GET_POINTER(other->wr_next);
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
					WEAKREF_SETBAD(other->wr_pself, struct Dee_weakref **);
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
				struct Dee_weakref *old_self_next;
				if unlikely(self->wr_obj == other->wr_obj) {
					/* Special case: both point to the same object -> must clear `other' */
					if (next) {
						next->wr_pself = other->wr_pself;
						WEAKREF_UNLOCK(next); /* LOCAL_UNLOCK_other__next(); */
					}
					atomic_write(other->wr_pself, next); /* LOCAL_UNLOCK_other__prev(); */
					other->wr_obj = NULL;
					WEAKREF_SETBAD(other->wr_pself, struct Dee_weakref **);
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
				old_self_next = (struct Dee_weakref *)GET_POINTER(self->wr_next);
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
			WEAKREF_SETBAD(other->wr_pself, struct Dee_weakref **);
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
Dee_weakref_fini(struct Dee_weakref *__restrict self) {
#ifndef NDEBUG
	ASSERT(self->wr_obj != (DeeObject *)WEAKREF_BAD_POINTER);
#endif /* !NDEBUG */
again:
	if (self->wr_obj) {
		WEAKREF_LOCK(self);
		COMPILER_READ_BARRIER();
		if likely(self->wr_obj) {
			struct Dee_weakref *next;
			if unlikely(!WEAKREF_PREV_TRYLOCK(self)) {
				/* Prevent a deadlock. */
				WEAKREF_UNLOCK(self);
				SCHED_YIELD();
				goto again;
			}
			next = (struct Dee_weakref *)GET_POINTER(self->wr_next);
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
	WEAKREF_SETBAD(self->wr_pself, struct Dee_weakref **);
	WEAKREF_SETBAD(self->wr_next, struct Dee_weakref *);
	WEAKREF_SETBAD(self->wr_obj, DeeObject *);
}

PUBLIC NONNULL((1)) bool DCALL
Dee_weakref_clear(struct Dee_weakref *__restrict self) {
#ifndef NDEBUG
	ASSERT(self->wr_obj != (DeeObject *)WEAKREF_BAD_POINTER);
#endif /* !NDEBUG */
again:
	if (self->wr_obj) {
		WEAKREF_LOCK(self);
		COMPILER_READ_BARRIER();
		if likely(self->wr_obj) {
			struct Dee_weakref *next;
			if unlikely(!WEAKREF_PREV_TRYLOCK(self)) {
				WEAKREF_UNLOCK(self);
				SCHED_YIELD();
				goto again;
			}
			next = (struct Dee_weakref *)GET_POINTER(self->wr_next);
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
		WEAKREF_SETBAD(self->wr_pself, struct Dee_weakref **);
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
Dee_weakref_set(struct Dee_weakref *__restrict self,
                DeeObject *__restrict ob) {
	struct Dee_weakref_list *new_list;
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
		struct Dee_weakref *next;
		if unlikely(!TRYLOCK_POINTER(new_list->wl_nodes)) {
			WEAKREF_UNLOCK(self);
			SCHED_YIELD();
			goto again;
		}

		next = (struct Dee_weakref *)GET_POINTER(new_list->wl_nodes);
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
			struct Dee_weakref *old_self_next;
			if unlikely(!WEAKREF_PREV_TRYLOCK(self)) {
				if (next)
					WEAKREF_UNLOCK(next);            /* WEAKREF_UNLOCK(new_list->FIRST) */
				UNLOCK_POINTER(new_list->wl_nodes);  /* WEAKREF_UNLOCK(new_list) */
				WEAKREF_UNLOCK(self);                /* WEAKREF_UNLOCK(self) */
				SCHED_YIELD();
				goto again;
			}
			old_self_next = (struct Dee_weakref *)GET_POINTER(self->wr_next);
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
(DCALL Dee_weakref_lock)(struct Dee_weakref const *__restrict self)
#else /* __INTELLISENSE__ */
PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *
(DCALL Dee_weakref_lock)(struct Dee_weakref *__restrict self)
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
(DCALL Dee_weakref_bound)(struct Dee_weakref const *__restrict self)
#else /* __INTELLISENSE__ */
PUBLIC WUNUSED NONNULL((1)) bool
(DCALL Dee_weakref_bound)(struct Dee_weakref *__restrict self)
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
Dee_weakref_cmpxch(struct Dee_weakref *__restrict self,
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
				struct Dee_weakref *next;

				/* Delete a previously assigned object. */
				if unlikely(!WEAKREF_PREV_TRYLOCK(self)) {
					WEAKREF_UNLOCK(self);
					SCHED_YIELD();
					goto again;
				}
				next = (struct Dee_weakref *)GET_POINTER(self->wr_next);
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
				WEAKREF_SETBAD(self->wr_pself, struct Dee_weakref **);
				atomic_write(&self->wr_next, WEAKREF_EMPTY_NEXTVAL); /* WEAKREF_UNLOCK(self) */
			}
		} else {
			struct Dee_weakref_list *new_list;
			new_list = WEAKREFS_GET(new_ob);
			if unlikely(!WEAKREFS_OK(new_list, new_ob)) {
				WEAKREF_UNLOCK(self);
				/* Weak referencing is not supported by `new_ob'. */
				return ITER_DONE;
			} else if unlikely(old_ob == new_ob) {
				/* Special case: `old_ob' matches `new_ob' */
				WEAKREF_UNLOCK(self);
			} else {
				struct Dee_weakref *next;
				if (!TRYLOCK_POINTER(new_list->wl_nodes)) {
					WEAKREF_UNLOCK(self);
					SCHED_YIELD();
					goto again;
				}

				next = (struct Dee_weakref *)GET_POINTER(new_list->wl_nodes);
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
					struct Dee_weakref *old_self_next;
					if (!WEAKREF_PREV_TRYLOCK(self)) {
						if (next)
							WEAKREF_UNLOCK(next);           /* WEAKREF_UNLOCK(new_list->FIRST) */
						UNLOCK_POINTER(new_list->wl_nodes); /* WEAKREF_UNLOCK(new_list) */
						WEAKREF_UNLOCK(self);               /* WEAKREF_UNLOCK(self) */
						SCHED_YIELD();
						goto again;
					}
					old_self_next = (struct Dee_weakref *)GET_POINTER(self->wr_next);
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


#ifndef GenericObject_DEFINED
#define GenericObject_DEFINED
typedef struct {
	OBJECT_HEAD
} GenericObject;
#endif /* !GenericObject_DEFINED */


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
			((GenericObject *)self)->ob_type = undo_start;
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
				} while unlikely(!atomic_cmpxch_weak_or_write(&self->ob_refcnt, refcnt, refcnt + 1));
				return false;
			}
		}

		/* Delete all weak references linked against this type level. */
destroy_weak:
		if (has_noninherited_weakrefs(undo_start)) {
			struct Dee_weakref *iter, *next;
			struct Dee_weakref_list *list;
			ASSERT(undo_start->tp_weakrefs >= sizeof(DeeObject));
			list = (struct Dee_weakref_list *)((uintptr_t)self + undo_start->tp_weakrefs);
restart_clear_weakrefs:
			LOCK_POINTER(list->wl_nodes);
			iter = (struct Dee_weakref *)GET_POINTER(list->wl_nodes);
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
				next = (struct Dee_weakref *)GET_POINTER(iter->wr_next);
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
				WEAKREF_SETBAD(iter->wr_pself, struct Dee_weakref **);
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


/* Finalize weakref support */
PUBLIC NONNULL((1)) void
(DCALL Dee_weakref_support_fini)(struct Dee_weakref_list *__restrict list) {
	struct Dee_weakref *iter, *next;
restart_clear_weakrefs:
	LOCK_POINTER(list->wl_nodes);
	iter = (struct Dee_weakref *)GET_POINTER(list->wl_nodes);
#ifdef __OPTIMIZE_SIZE__
	if (iter != NULL)
#else /* __OPTIMIZE_SIZE__ */
	if likely(iter != NULL)
#endif /* !__OPTIMIZE_SIZE__ */
	{
		if (!WEAKREF_TRYLOCK(iter)) {
			/* Prevent deadlock. */
			UNLOCK_POINTER(list->wl_nodes);
			SCHED_YIELD();
			goto restart_clear_weakrefs;
		}
		ASSERT(iter->wr_pself == &list->wl_nodes);
		next = (struct Dee_weakref *)GET_POINTER(iter->wr_next);
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
		WEAKREF_SETBAD(iter->wr_pself, struct Dee_weakref **);
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


/* Special "dummy" object used to  */
PRIVATE DeeNoneObject weakref_dummy = {
	/* .ob_refcnt = */ 0,
	/* .ob_type   = */ &DeeNone_Type,
#ifdef CONFIG_TRACE_REFCHANGES
	Dee_REFTRACKER_UNTRACKED,
#endif /* CONFIG_TRACE_REFCHANGES */
	Dee_WEAKREF_SUPPORT_INIT
};



/* Transfer all weak references of "self" to an always-dead dummy object.
 * Weak references with callbacks are **NOT** executed immediately, and
 * will only be invoked once `Dee_weakref_list_kill_dummy()' is called. */
INTERN NONNULL((1)) void DCALL
Dee_weakref_list_transfer_to_dummy(struct Dee_weakref_list *__restrict self) {
	struct Dee_weakref *iter, *next;
restart_clear_weakrefs:
	LOCK_POINTER(self->wl_nodes);
	iter = (struct Dee_weakref *)GET_POINTER(self->wl_nodes);
#ifdef __OPTIMIZE_SIZE__
	if (iter != NULL)
#else /* __OPTIMIZE_SIZE__ */
	if likely(iter != NULL)
#endif /* !__OPTIMIZE_SIZE__ */
	{
		if (!WEAKREF_TRYLOCK(iter)) {
			/* Prevent deadlock. */
			UNLOCK_POINTER(self->wl_nodes);
			SCHED_YIELD();
			goto restart_clear_weakrefs;
		}
		ASSERT(iter->wr_pself == &self->wl_nodes);
		next = (struct Dee_weakref *)GET_POINTER(iter->wr_next);

		if (iter->wr_del) {
			/* Transfer weakref to "weakref_dummy". Caller will
			 * eventually call "Dee_weakref_list_kill_dummy"! */
			uintptr_t nextval;
			struct Dee_weakref *next_dummy;
			if (next && !WEAKREF_TRYLOCK(next))
				goto restart__self__iter;
			if unlikely(!TRYLOCK_POINTER(weakref_dummy.ob_weakrefs.wl_nodes)) {
restart__self__iter__next:
				if (next)
					WEAKREF_UNLOCK(next);
				goto restart__self__iter;
			}
			nextval = (uintptr_t)weakref_dummy.ob_weakrefs.wl_nodes;
			ASSERT(nextval & PTRLOCK_LOCK_MASK);
			next_dummy = (struct Dee_weakref *)GET_POINTER(nextval);
			if (nextval != PTRLOCK_LOCK_MASK) {
				if unlikely(!WEAKREF_TRYLOCK(next_dummy)) {
/*restart__self__iter__next__dummy:*/
					UNLOCK_POINTER(weakref_dummy.ob_weakrefs.wl_nodes);
					goto restart__self__iter__next;
				}
				next_dummy->wr_pself = &iter->wr_next;
				WEAKREF_UNLOCK(next_dummy); /* Unlock "next_dummy" */
			}
			iter->wr_pself = &weakref_dummy.ob_weakrefs.wl_nodes;
			iter->wr_obj = Dee_AsObject(&weakref_dummy);
			atomic_write(&iter->wr_next, next_dummy);                /* Unlock "iter" */
			atomic_write(&weakref_dummy.ob_weakrefs.wl_nodes, iter); /* Unlock "dummy" */
			if (next) {
				next->wr_pself = &self->wl_nodes;
				WEAKREF_UNLOCK(next); /* Unlock "next" */
			}
			atomic_write(&self->wl_nodes, next); /* Unlock "self" */
			goto restart_clear_weakrefs;
		}

		if (next) {
			if (!WEAKREF_TRYLOCK(next)) {
				/* Prevent deadlock. */
restart__self__iter:
				WEAKREF_UNLOCK(iter);           /* WEAKREF_UNLOCK(self->FIRST) */
				UNLOCK_POINTER(self->wl_nodes); /* WEAKREF_UNLOCK(self) */
				SCHED_YIELD();
				goto restart_clear_weakrefs;
			}
			next->wr_pself = &self->wl_nodes;
			WEAKREF_UNLOCK(next);
		}

		/* Overwrite the weakly referenced object with NULL,
		 * indicating that the link has been severed. */
		atomic_write(&self->wl_nodes, next);
		WEAKREF_SETBAD(iter->wr_pself, struct Dee_weakref **);
		iter->wr_obj = NULL;
		COMPILER_WRITE_BARRIER();
		atomic_write(&iter->wr_next, WEAKREF_EMPTY_NEXTVAL);
		goto restart_clear_weakrefs;
	}
	atomic_write(&self->wl_nodes, NULL); /* WEAKREF_UNLOCK(self) */
}

INTERN void DCALL
Dee_weakref_list_kill_dummy(void) {
	Dee_weakref_support_fini(&weakref_dummy);
}


DECL_END

#endif /* !GUARD_DEEMON_RUNTIME_WEAKREF_C */
