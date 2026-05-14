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
#ifndef GUARD_DEEMON_OBJECTS_HASHSET_C
#define GUARD_DEEMON_OBJECTS_HASHSET_C 1

#include <deemon/api.h>

#include <deemon/alloc.h>              /* DeeObject_MALLOC, Dee_*alloc*, Dee_Free, Dee_TYPE_CONSTRUCTOR_INIT_FIXED */
#include <deemon/arg.h>                /* DeeArg_Unpack1 */
#include <deemon/bool.h>               /* Dee_True, return_bool */
#include <deemon/computed-operators.h> /* DEFIMPL, DEFIMPL_UNSUPPORTED */
#include <deemon/dict.h>               /* DeeDictObject */
#include <deemon/error-rt.h>           /* DeeRT_ErrEmptySequence, DeeRT_ErrIndexOutOfBounds */
#include <deemon/format.h>             /* PRFuSIZ, PRFxSIZ */
#include <deemon/gc.h>                 /* Dee_TYPE_CONSTRUCTOR_INIT_FIXED_GC */
#include <deemon/hashset.h>            /* DeeHashSetObject, DeeHashSet_*, Dee_hashset_item, _DeeHashSet_* */
#include <deemon/method-hints.h>       /* Dee_seq_enumerate_index_t, TYPE_METHOD_HINT*, type_method_hint */
#include <deemon/none-operator.h>      /* _DeeNone_reti0_1, _DeeNone_reti0_2 */
#include <deemon/object.h>             /* DREF, DeeObject, DeeObject_*, DeeTypeObject, Dee_AsObject, Dee_COMPARE_ERR, Dee_Decref, Dee_Decref_unlikely, Dee_HAS_ISERR, Dee_HAS_ISNO_OR_ERR, Dee_Incref, Dee_TYPE, Dee_WEAKREF_SUPPORT_ADDR, Dee_foreach_t, Dee_formatprinter_t, Dee_hash_t, Dee_return_compareT, Dee_ssize_t, ITER_DONE, OBJECT_HEAD_INIT */
#include <deemon/roset.h>              /* DeeRoSet* */
#include <deemon/seq.h>                /* DeeIterator_Type */
#include <deemon/serial.h>             /* DeeSerial, Dee_seraddr_t */
#include <deemon/set.h>                /* DeeSet_Type */
#include <deemon/string.h>             /* DeeString_STR */
#include <deemon/system-features.h>    /* memset */
#include <deemon/type.h>               /* DeeObject_InitStatic, DeeType_Type, Dee_TYPE_CONSTRUCTOR_INIT_FIXED, Dee_TYPE_CONSTRUCTOR_INIT_FIXED_GC, Dee_visit_t, METHOD_F*, OPERATOR_*, STRUCT_OBJECT_AB, TF_NONE, TP_F*, TYPE_*, type_* */
#include <deemon/util/atomic.h>        /* atomic_cmpxch_or_write, atomic_read */
#include <deemon/util/hash-io.h>       /* Dee_HASH_*, Dee_SIZEOF_HASH_VIDX_T, Dee_hash_*, IF_Dee_HASH_HIDXIO_COUNT_GE_*, _DeeHash_EmptyTab */
#include <deemon/util/lock.h>          /* Dee_atomic_rwlock_* */

#include <hybrid/align.h>    /* IS_POWER_OF_TWO */
#include <hybrid/overflow.h> /* OVERFLOW_UADD */
#include <hybrid/typecore.h> /* __SIZEOF_INT__, __SIZEOF_SIZE_T__ */

#include "../runtime/method-hint-defaults.h"
#include "../runtime/runtime_error.h"
#include "../runtime/strings.h"
#include "dict-utils.h"
#include "dict.h"
#include "generic-proxy.h"
#include "hashset.h"

#include <stdbool.h> /* bool */
#include <stddef.h>  /* NULL, offsetof, size_t */

DECL_BEGIN

#ifndef NDEBUG
#define DBG_memset (void)memset
#else /* !NDEBUG */
#define DBG_memset(dst, byte, n_bytes) (void)0
#endif /* NDEBUG */

typedef DeeHashSetObject HashSet;

/************************************************************************/
/* ITERATOR                                                             */
/************************************************************************/
STATIC_ASSERT(offsetof(HashSetIterator, hsi_set) == offsetof(ProxyObject, po_obj));
#define siter_fini      generic_proxy__fini
#define siter_visit     generic_proxy__visit
#define siter_serialize generic_proxy__serialize_and_wordcopy_atomic(Dee_SIZEOF_HASH_VIDX_T)

PRIVATE WUNUSED NONNULL((1)) int DCALL
siter_ctor(HashSetIterator *__restrict self) {
	self->hsi_set = (DREF HashSet *)DeeHashSet_New();
	if unlikely(!self->hsi_set)
		goto err;
	self->hsi_vidx = Dee_hash_vidx_tovirt(0);
	return 0;
err:
	return -1;
}

#if 1
STATIC_ASSERT(offsetof(HashSetIterator, hsi_set) == offsetof(DictIterator, di_dict));
STATIC_ASSERT(offsetof(HashSetIterator, hsi_vidx) == offsetof(DictIterator, di_vidx));
INTDEF WUNUSED NONNULL((1, 2)) int DCALL diter_copy(DictIterator *__restrict self, DictIterator *__restrict other);
#define siter_copy diter_copy
#else
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
siter_copy(HashSetIterator *__restrict self,
           HashSetIterator *__restrict other) {
	self->hsi_set = other->hsi_set;
	Dee_Incref(self->hsi_set);
	self->hsi_vidx = atomic_read(&other->hsi_vidx);
	return 0;
}
#endif

PRIVATE WUNUSED NONNULL((1)) int DCALL
siter_init(HashSetIterator *__restrict self,
           size_t argc, DeeObject *const *argv) {
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("_HashSetIterator", params: "
	DeeHashSetObject *set;
", docStringPrefix: "siter");]]]*/
#define siter__HashSetIterator_params "set:?DHashSet"
	struct {
		DeeHashSetObject *set;
	} args;
	DeeArg_Unpack1(err, argc, argv, "_HashSetIterator", &args.set);
/*[[[end]]]*/
	if (DeeObject_AssertType(args.set, &DeeHashSet_Type))
		goto err;
	self->hsi_set = args.set;
	Dee_Incref(args.set);
	self->hsi_vidx = Dee_hash_vidx_tovirt(0);
	return 0;
err:
	return -1;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
siter_next(HashSetIterator *__restrict self) {
	DREF DeeObject *result;
	HashSet *set = self->hsi_set;
	for (;;) {
		/*virt*/ Dee_hash_vidx_t old_vidx = atomic_read(&self->hsi_vidx);
		/*virt*/ Dee_hash_vidx_t new_vidx = old_vidx;
		DeeHashSet_LockRead(set);
		do {
			if unlikely(new_vidx > set->hs_vsize) {
				DeeHashSet_LockEndRead(set);
				return ITER_DONE;
			}
			result = _DeeHashSet_GetVirtVTab(set)[new_vidx].hsi_key;
			++new_vidx;
		} while (!result);
		Dee_Incref(result);
		DeeHashSet_LockEndRead(set);
		if (atomic_cmpxch_or_write(&self->hsi_vidx, old_vidx, new_vidx))
			break;
		Dee_Decref_unlikely(result);
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
siter_advance(HashSetIterator *__restrict self, size_t skip) {
	/*virt*/ Dee_hash_vidx_t old_vidx, new_vidx;
	HashSet *set = self->hsi_set;
	for (;;) {
		old_vidx = atomic_read(&self->hsi_vidx);
		new_vidx = old_vidx;
		DeeHashSet_LockRead(set);
		ASSERT(set->hs_vused <= set->hs_vsize);
		if (set->hs_vused == set->hs_vsize) {
			/* HashSet is optimized -> can just increment "new_vidx" */
			if (OVERFLOW_UADD(new_vidx, skip, &new_vidx))
				new_vidx = (size_t)-1;
			if (new_vidx > Dee_hash_vidx_tovirt(set->hs_vsize))
				new_vidx = Dee_hash_vidx_tovirt(set->hs_vsize);
		} else {
			/* Only keys that haven't been deleted count. */
			size_t n_skip = skip;
			struct Dee_hashset_item *virt_vtab = _DeeHashSet_GetVirtVTab(set);
			for (; Dee_hash_vidx_virt_lt_real(new_vidx, set->hs_vsize) && n_skip; ++new_vidx) {
				if (virt_vtab[new_vidx].hsi_key)
					--n_skip;
			}
		}
		DeeHashSet_LockEndRead(set);
		if (atomic_cmpxch_or_write(&self->hsi_vidx, old_vidx, new_vidx))
			break;
	}
	return new_vidx - old_vidx;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
siter_bool(HashSetIterator *__restrict self) {
	HashSet *set = self->hsi_set;
	/*virt*/Dee_hash_vidx_t vidx = atomic_read(&self->hsi_vidx);
	struct Dee_hashset_item *virt_vtab;
	ASSERT(vidx >= Dee_hash_vidx_tovirt(0));
	DeeHashSet_LockRead(set);
	virt_vtab = _DeeHashSet_GetVirtVTab(set);
	for (; Dee_hash_vidx_virt_lt_real(vidx, set->hs_vsize); ++vidx) {
		if (virt_vtab[vidx].hsi_key) {
			DeeHashSet_LockEndRead(set);
			return 1;
		}
	}
	DeeHashSet_LockEndRead(set);
	return 0;
}

#if 1
STATIC_ASSERT(offsetof(HashSetIterator, hsi_vidx) == offsetof(DictIterator, di_vidx));
INTDEF struct type_cmp diter_cmp;
#define siter_cmp diter_cmp
#else
PRIVATE WUNUSED NONNULL((1)) Dee_hash_t DCALL
siter_hash(HashSetIterator *self) {
	return atomic_read(&self->hsi_vidx);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
siter_compare(HashSetIterator *lhs, HashSetIterator *rhs) {
	if (DeeObject_AssertType(rhs, Dee_TYPE(lhs)))
		goto err;
	Dee_return_compareT(Dee_hash_vidx_t, atomic_read(&lhs->hsi_vidx),
	                    /*            */ atomic_read(&rhs->hsi_vidx));
err:
	return Dee_COMPARE_ERR;
}

PRIVATE struct type_cmp siter_cmp = {
	/* .tp_hash          = */ (Dee_hash_t (DCALL *)(DeeObject *))&siter_hash,
	/* .tp_compare_eq    = */ DEFIMPL(&default__compare_eq__with__compare),
	/* .tp_compare       = */ (int (DCALL *)(DeeObject *, DeeObject *))&siter_compare,
	/* .tp_trycompare_eq = */ DEFIMPL(&default__trycompare_eq__with__compare_eq),
	/* .tp_eq            = */ DEFIMPL(&default__eq__with__compare_eq),
	/* .tp_ne            = */ DEFIMPL(&default__ne__with__compare_eq),
	/* .tp_lo            = */ DEFIMPL(&default__lo__with__compare),
	/* .tp_le            = */ DEFIMPL(&default__le__with__compare),
	/* .tp_gr            = */ DEFIMPL(&default__gr__with__compare),
	/* .tp_ge            = */ DEFIMPL(&default__ge__with__compare),
};
#endif


#if 1
INTDEF struct type_method tpconst diter_methods[];
#define siter_methods diter_methods
#else
PRIVATE struct type_method tpconst siter_methods[] = {
	TYPE_METHOD_HINTREF(Iterator_advance),
	TYPE_METHOD_END
};
#endif

PRIVATE struct type_method_hint tpconst siter_method_hints[] = {
	TYPE_METHOD_HINT(iter_advance, &siter_advance),
	TYPE_METHOD_HINT_END
};

PRIVATE struct type_member tpconst siter_members[] = {
	TYPE_MEMBER_FIELD_DOC(STR_seq, STRUCT_OBJECT_AB, offsetof(HashSetIterator, hsi_set), "->?DHashSet"),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject HashSetIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_HashSetIterator",
	/* .tp_doc      = */ DOC("(" siter__HashSetIterator_params ")"),
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ HashSetIterator,
			/* tp_ctor:        */ &siter_ctor,
			/* tp_copy_ctor:   */ &siter_copy,
			/* tp_any_ctor:    */ &siter_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &siter_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&siter_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&siter_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&iterator_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&siter_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__EFED4BCD35433C3C),
	/* .tp_cmp           = */ &siter_cmp,
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&siter_next,
	/* .tp_iterator      = */ DEFIMPL(&default__tp_iterator__712535FF7E4C26E5),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ siter_methods,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ siter_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ siter_method_hints,
	/* .tp_call          = */ DEFIMPL(&iterator_next),
	/* .tp_callable      = */ DEFIMPL(&default__tp_callable__83C59FA7626CABBE),
};















/************************************************************************/
/* HASHSET                                                              */
/************************************************************************/

/* Heap functions for allocating/freeing hashset tables (hs_vtab + hs_htab) */
#define _DeeHashSet_TabsMalloc(n_bytes)          Dee_Malloc(n_bytes)
#define _DeeHashSet_TabsCalloc(n_bytes)          Dee_Calloc(n_bytes)
#define _DeeHashSet_TabsRealloc(ptr, n_bytes)    Dee_Realloc(ptr, n_bytes)
#define _DeeHashSet_TabsTryMalloc(n_bytes)       Dee_TryMalloc(n_bytes)
#define _DeeHashSet_TabsTryCalloc(n_bytes)       Dee_TryCalloc(n_bytes)
#define _DeeHashSet_TabsTryRealloc(ptr, n_bytes) Dee_TryRealloc(ptr, n_bytes)
#define _DeeHashSet_TabsFree(ptr)                Dee_Free(ptr)

#define NULL_IF__DeeHashSet_EmptyVTab_REAL(/*real*/ p) \
	((p) == (struct Dee_hashset_item *)_DeeHash_EmptyTab ? NULL : (p))

#define HAVE_hashset_insert_unlocked
#ifdef __OPTIMIZE_SIZE__
#undef HAVE_hashset_insert_unlocked
#endif /* __OPTIMIZE_SIZE__ */

#ifdef DICT_NDEBUG
#define hashset_verify(self) (void)0
#else /* DICT_NDEBUG */
PRIVATE ATTR_NOINLINE NONNULL((1)) void DCALL
hashset_verify(HashSet *__restrict self) {
	Dee_hash_vidx_t i, real_vused;
	Dee_hash_hidxio_t hidxio;
	struct Dee_hash_hidxio_ops const *ops;
	ASSERT(self->hs_vused <= self->hs_vsize);
	ASSERT(self->hs_vsize <= self->hs_valloc);
	ASSERT(self->hs_valloc <= self->hs_hmask);
	ASSERT(IS_POWER_OF_TWO(self->hs_hmask + 1));
	ASSERT(self->hs_htab == (union Dee_hash_htab *)(_DeeHashSet_GetRealVTab(self) + self->hs_valloc));
	hidxio = Dee_HASH_HIDXIO_FROM_VALLOC(self->hs_valloc);
	ASSERT(/*hidxio >= 0 &&*/ hidxio < Dee_HASH_HIDXIO_COUNT);
	ops = self->hs_hidxops;
	ASSERT(ops == &Dee_hash_hidxio[hidxio]);
	for (i = Dee_hash_vidx_tovirt(0), real_vused = 0;
	     Dee_hash_vidx_virt_lt_real(i, self->hs_vsize); ++i) {
		struct Dee_hashset_item *item = &_DeeHashSet_GetVirtVTab(self)[i];
		if (item->hsi_key) {
#if 0 /* Cannot be asserted -- we might get here from "tp_visit", which can screw with reference counts */
			ASSERT_OBJECT(item->hsi_key);
#endif
			++real_vused;
		}
	}
	ASSERTF(real_vused == self->hs_vused,
	        "vtab key count=%" PRFuSIZ " differs from hs_vused=%" PRFuSIZ,
	        real_vused, self->hs_vused);
	for (i = 0; i <= self->hs_hmask; ++i) {
		Dee_hash_vidx_t vidx;
		vidx = (*ops->hxio_get)(self->hs_htab, i);
		if (vidx == Dee_HASH_HTAB_EOF)
			continue;
		Dee_hash_vidx_virt2real(&vidx);
		ASSERTF(vidx < self->hs_vsize,
		        "htab[%" PRFuSIZ "] points out-of-bounds: %" PRFuSIZ " >= %" PRFuSIZ,
		        i, vidx, self->hs_vsize);
	}
	for (i = 0;; ++i) {
		Dee_hash_vidx_t vtab_idx;
		ASSERTF(i <= self->hs_hmask, "htab contains no EOF pointers (infinite loop would occur on non-present item lookup)");
		vtab_idx = (*ops->hxio_get)(self->hs_htab, i);
		if (vtab_idx == Dee_HASH_HTAB_EOF)
			break;
	}
	for (i = Dee_hash_vidx_tovirt(0), real_vused = 0;
	     Dee_hash_vidx_virt_lt_real(i, self->hs_vsize); ++i) {
		Dee_hash_t hs, perturb;
		struct Dee_hashset_item *item = &_DeeHashSet_GetVirtVTab(self)[i];
		if (!item->hsi_key)
			continue;
		for (_DeeHashSet_HashIdxInit(self, &hs, &perturb, item->hsi_hash);;
		     _DeeHashSet_HashIdxNext(self, &hs, &perturb, item->hsi_hash)) {
			struct Dee_hashset_item *hitem;
			Dee_hash_hidx_t htab_idx = Dee_hash_hidx_ofhash(hs, self->hs_hmask);
			Dee_hash_vidx_t vtab_idx = (*ops->hxio_get)(self->hs_htab, htab_idx); /*virt*/
			ASSERTF(vtab_idx != Dee_HASH_HTAB_EOF,
			        "End-of-hash-chain[hash:%#" PRFxSIZ "] before item idx=%" PRFuSIZ ",count=%" PRFuSIZ " <%r> was found",
			        item->hsi_hash, Dee_hash_vidx_toreal(i),
			        self->hs_vsize, item->hsi_key);
			hitem = &_DeeHashSet_GetVirtVTab(self)[vtab_idx];
			if (hitem == item)
				break;
		}
	}
}

#if 1
#undef DeeHashSet_LockEndWrite
#undef DeeHashSet_LockEndRead
#undef DeeHashSet_LockEnd
#define DeeHashSet_LockEndWrite(self)   (hashset_verify(self), Dee_atomic_rwlock_endwrite(&(self)->hs_lock))
#define DeeHashSet_LockEndRead(self)    (hashset_verify(self), Dee_atomic_rwlock_endread(&(self)->hs_lock))
#define DeeHashSet_LockEnd(self)        (hashset_verify(self), Dee_atomic_rwlock_end(&(self)->hs_lock))
#endif
#endif /* !DICT_NDEBUG */

#define hashset_htab_decafter(self, /*virt*/ vtab_threshold) \
	(*(self)->hs_hidxops->hxio_decafter)((self)->hs_htab, (self)->hs_hmask, vtab_threshold)
#define hashset_htab_incafter(self, /*virt*/ vtab_threshold) \
	(*(self)->hs_hidxops->hxio_incafter)((self)->hs_htab, (self)->hs_hmask, vtab_threshold)
#define hashset_htab_decrange(self, /*virt*/ vtab_min, /*virt*/ vtab_max) \
	(*(self)->hs_hidxops->hxio_decrange)((self)->hs_htab, (self)->hs_hmask, vtab_min, vtab_max)
#define hashset_htab_incrange(self, /*virt*/ vtab_min, /*virt*/ vtab_max) \
	(*(self)->hs_hidxops->hxio_incrange)((self)->hs_htab, (self)->hs_hmask, vtab_min, vtab_max)
#define hashset_htab_reverse(self, /*virt*/ vmin, /*virt*/ vmax) \
	(*(self)->hs_hidxops->hxio_revrange)((self)->hs_htab, (self)->hs_hmask, vmin, vmax)

#ifdef __INTELLISENSE__

/************************************************************************/
/************************************************************************/
/* LOW-LEVEL API                                                        */
/************************************************************************/
/************************************************************************/

PRIVATE WUNUSED DREF HashSet *DCALL
hashset_new_with_hint(size_t num_items, bool tryalloc, bool allow_overalloc);

/* Re-build the hashset's "hs_htab" (allowed to assume that "hs_vtab" does not contain deleted keys) */
PRIVATE ATTR_NOINLINE NONNULL((1)) void DCALL
hashset_htab_rebuild_after_optimize(HashSet *__restrict self);

PRIVATE ATTR_NOINLINE NONNULL((1)) void DCALL
hashset_do_optimize_vtab_without_rebuild(HashSet *__restrict self);

INTDEF NONNULL((1)) void DCALL
hashset_optimize_vtab(HashSet *__restrict self);

PRIVATE NONNULL((1)) void DCALL
hashset_htab_rebuild(HashSet *__restrict self);

/* Try to make it so "hs_vsize < hs_valloc" by enlarging the vector.
 * Do this while the caller is holding a write-lock to "self", and
 * do so without ever releasing that lock.
 * NOTES:
 * - This function will NEVER rehash the hashset or change the contents of hs_htab!
 * - The caller must ensure that `_DeeHashSet_CanGrowVTab(self)' is true
 * @return: true:  Success
 * @return: false: Failure */
PRIVATE ATTR_NOINLINE WUNUSED NONNULL((1)) bool DCALL
hashset_trygrow_vtab(HashSet *__restrict self);

/* Same as `hashset_trygrow_vtab()', but allowed to grow the htab
 * also, and can be used even when "!_DeeHashSet_CanGrowVTab(self)"
 * Tries to make it so "hs_valloc >= min_valloc"
 * @return: true:  Success: "hs_valloc >= min_valloc"
 * @return: false: Failure */
PRIVATE ATTR_NOINLINE NONNULL((1)) bool DCALL
hashset_trygrow_vtab_and_htab_with(HashSet *__restrict self,
                                   Dee_hash_vidx_t min_valloc,
                                   bool allow_overalloc);

/* Same as `hashset_trygrow_vtab()', but allowed to grow the htab
 * also, and can be used even when "!_DeeHashSet_CanGrowVTab(self)"
 * @return: true:  Success
 * @return: false: Failure */
PRIVATE WUNUSED NONNULL((1)) bool DCALL
hashset_trygrow_vtab_and_htab(HashSet *__restrict self);

#if 0
/* Try to change "hs_hmask = (hs_hmask << 1) | 1",
 * and (if we want to), also increase "hs_valloc"
 * @return: true:  Success
 * @return: false: Failure (allocation failed) */
PRIVATE ATTR_NOINLINE WUNUSED NONNULL((1)) bool DCALL
hashset_trygrow_htab_and_maybe_vtab(HashSet *__restrict self);
#endif

/* Make it so "!_DeeHashSet_MustGrowVTab(self)"
 * (aka: " hs_vsize < hs_valloc && hs_valloc <= hs_hmask")
 * Allowed to release+re-acquire a write-lock to "self".
 * @return: 0 : Success (lock was lost and later re-acquired)
 * @return: -1: Failure (lock was lost and an error was thrown) */
#ifdef HAVE_hashset_insert_unlocked
PRIVATE ATTR_NOINLINE WUNUSED NONNULL((1)) int DCALL
hashset_grow_vtab_and_htab_and_relock(HashSet *__restrict self, bool without_locks);
#else /* HAVE_hashset_insert_unlocked */
PRIVATE ATTR_NOINLINE WUNUSED NONNULL((1)) int DCALL
hashset_grow_vtab_and_htab_and_relock_impl(HashSet *__restrict self);
#define hashset_grow_vtab_and_htab_and_relock(self, without_locks) \
	hashset_grow_vtab_and_htab_and_relock_impl(self)
#endif /* !HAVE_hashset_insert_unlocked */

#if 0
/* Make it so "!_DeeHashSet_MustGrowHTab(self)".
 * Allowed to release+re-acquire a write-lock to "self".
 * @return: 0 : Success (lock was lost and later re-acquired)
 * @return: -1: Failure (lock was lost and an error was thrown) */
PRIVATE ATTR_NOINLINE WUNUSED NONNULL((1)) int DCALL
hashset_grow_htab_and_relock(HashSet *__restrict self);
#endif

/* Shrink the vtab and release a lock to "self". Must be called when:
 * - holding a write-lock
 * - _DeeHashSet_CanShrinkHTab(self) is true
 * - _DeeHashSet_ShouldShrinkHTab(self) is true (or `fully_shrink=true')
 * NOTE: After a call to this function, the caller must always rebuild the htab! */
PRIVATE NONNULL((1)) void DCALL
hashset_shrink_htab(HashSet *__restrict self, bool fully_shrink);

/* Shrink the vtab+htab. Must be called while:
 * - holding a write-lock
 * - _DeeHashSet_CanShrinkVTab(self) is true
 * - _DeeHashSet_ShouldShrinkVTab(self) is true (or `fully_shrink=true') */
PRIVATE ATTR_NOINLINE NONNULL((1)) void DCALL
hashset_shrink_vtab_and_htab(HashSet *__restrict self, bool fully_shrink);

/* Automatically shrink allocated tables of "self" if appropriate.
 * Call this function are removing elements from "self"
 * Same as the API function "hashset.shrink(fully: false)" */
LOCAL NONNULL((1)) void DCALL
hashset_autoshrink(HashSet *__restrict self);

PRIVATE ATTR_NOINLINE NONNULL((1)) void DCALL
hashset_makespace_at_impl(HashSet *__restrict self, /*real*/ Dee_hash_vidx_t vtab_idx);

LOCAL NONNULL((1)) void DCALL
hashset_makespace_at(HashSet *__restrict self, /*real*/ Dee_hash_vidx_t vtab_idx);

#else /* __INTELLISENSE__ */
DECL_END
#define DEFINE_DeeHashSet
#define DEFINE_LOW_LEVEL
#include "dict-impl.c.inl"
DECL_BEGIN
#endif /* !__INTELLISENSE__ */


/* @param: getindex: returns the unoptimized (iow: including deleted keys)
 *                   index in "hs_vtab" where the new key should be inserted.
 *                   All of the other functions above simply append at the end of
 *                   "hs_vtab", which is the same as this callback returning "hs_vsize"
 *                   @param: overwrite_index: When "key" already exists, the index of
 *                                            the item that will be deleted. Else, set
 *                                            to `Dee_HASH_HTAB_EOF' when "key" is new.
 *                   - To throw an error, the callback should:
 *                     >> DeeHashSet_LockEndWrite(self)
 *                     >> DeeError_Throw(...);
 *                     >> return Dee_HASH_HTAB_EOF; */
PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
hashset_insert_at(HashSet *self, DeeObject *key,
                  /*virt*/ Dee_hash_vidx_t (DCALL *getindex)(void *cookie, HashSet *self,
                                                             /*virt*/ Dee_hash_vidx_t overwrite_index),
                  void *getindex_cookie);
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL hashset_mh_insert(HashSet *self, DeeObject *key);
#ifndef HAVE_hashset_insert_unlocked
#define hashset_insert_unlocked hashset_mh_insert
#else /* !HAVE_hashset_insert_unlocked */
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL hashset_insert_unlocked(HashSet *self, DeeObject *key);
#endif /* HAVE_hashset_insert_unlocked */
PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL hashset_mh_unify(HashSet *self, DeeObject *key);
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL hashset_mh_contains(HashSet *self, DeeObject *key);
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL hashset_mh_contains_with_range(HashSet *self, DeeObject *key, size_t start, size_t end);
PRIVATE WUNUSED NONNULL((1, 2)) size_t DCALL hashset_mh_find(HashSet *self, DeeObject *key, size_t start, size_t end);
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL hashset_mh_remove(HashSet *self, DeeObject *key);

#ifdef __OPTIMIZE_SIZE__
#define SUBSTITUDE_hashset_mh_contains
#define SUBSTITUDE_hashset_mh_contains_with_range
#endif /* __OPTIMIZE_SIZE__ */

#ifndef __INTELLISENSE__
DECL_END
#define DEFINE_hashset_mh_find
#include "dict-getitem-impl.c.inl"
#ifndef SUBSTITUDE_hashset_mh_contains
#define DEFINE_hashset_mh_contains
#include "dict-getitem-impl.c.inl"
#endif /* !SUBSTITUDE_hashset_mh_contains */
#ifndef SUBSTITUDE_hashset_mh_contains_with_range
#define DEFINE_hashset_mh_contains_with_range
#include "dict-getitem-impl.c.inl"
#endif /* !SUBSTITUDE_hashset_mh_contains_with_range */

#define DEFINE_hashset_mh_insert
#include "dict-setitem-impl.c.inl"
#define DEFINE_hashset_insert_at
#include "dict-setitem-impl.c.inl"
#ifdef HAVE_hashset_insert_unlocked
#define DEFINE_hashset_insert_unlocked
#include "dict-setitem-impl.c.inl"
#endif /* HAVE_hashset_insert_unlocked */
#define DEFINE_hashset_mh_unify
#include "dict-setitem-impl.c.inl"

#define DEFINE_hashset_mh_remove
#include "dict-delitem-impl.c.inl"
DECL_BEGIN
#endif /* !__INTELLISENSE__ */

#ifdef SUBSTITUDE_hashset_mh_contains_with_range
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
hashset_mh_contains_with_range(RoSet *self, DeeObject *key, size_t start, size_t end) {
	size_t index = hashset_mh_find(self, key, start, end);
	if unlikely(index == (size_t)Dee_COMPARE_ERR)
		goto err;
	return index != (size_t)-1 ? 1 : 0;
err:
	return -1;
}
#endif /* SUBSTITUDE_hashset_mh_contains_with_range */

#ifdef SUBSTITUDE_hashset_mh_contains
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
hashset_mh_contains(RoSet *self, DeeObject *key) {
	return hashset_mh_contains_with_range(self, key, 0, (size_t)-1);
}
#endif /* SUBSTITUDE_hashset_mh_contains */

#if __SIZEOF_INT__ == __SIZEOF_SIZE_T__
#define hashset_fromsequence_foreach_cb (*(Dee_ssize_t (DCALL *)(void *, DeeObject *))&hashset_insert_unlocked)
#else /* __SIZEOF_INT__ == __SIZEOF_SIZE_T__ */
PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
hashset_fromsequence_foreach_cb(void *self, DeeObject *key) {
	return hashset_insert_unlocked((HashSet *)self, key);
}
#endif /* __SIZEOF_INT__ != __SIZEOF_SIZE_T__ */



#undef HASHSET_INITFROM_NEEDSLOCK
#ifndef HAVE_hashset_insert_unlocked /* Because of "#define dict_setitem_unlocked dict_setitem" */
#define HASHSET_INITFROM_NEEDSLOCK
#endif /* !HAVE_hashset_insert_unlocked */


PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
hashset_init_fromcopy(HashSet *__restrict self,
                      HashSet *__restrict other);

#ifndef __INTELLISENSE__
DECL_END
#define DEFINE_hashset_init_fromcopy
#include "dict-init-copyfrom.c.inl"
DECL_BEGIN
#endif /* !__INTELLISENSE__ */


#ifdef __INTELLISENSE__
/************************************************************************/
/************************************************************************/
/* HIGH-LEVEL API                                                       */
/************************************************************************/
/************************************************************************/
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL hashset_init_fromroset_noincref(HashSet *__restrict self, DeeRoSetObject *__restrict other);
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL hashset_init_fromroset(HashSet *__restrict self, DeeRoSetObject *__restrict other);
PRIVATE WUNUSED NONNULL((1)) DREF HashSet *DCALL hashset_new_copy(HashSet *__restrict self);
DFUNDEF WUNUSED NONNULL((1)) DREF /*HashSet*/ DeeObject *DCALL DeeHashSet_FromSequence(DeeObject *__restrict self);
DFUNDEF WUNUSED NONNULL((1)) DREF /*HashSet*/ DeeObject *DCALL DeeHashSet_FromSequenceInheritedOnSuccess(/*inherit(on_success)*/ DREF DeeObject *__restrict self);
DFUNDEF WUNUSED NONNULL((1)) DREF /*HashSet*/ DeeObject *DCALL DeeHashSet_FromRoHashSet(/*RoHashSet*/ DeeObject *__restrict self);
DFUNDEF WUNUSED DREF DeeObject *DCALL DeeHashSet_NewItemsInherited(size_t num_items, /*inherit(on_success)*/ DREF DeeObject **items);
PRIVATE ATTR_NOINLINE NONNULL((1)) void DCALL hashset_fini(HashSet *__restrict self);
PRIVATE ATTR_NOINLINE NONNULL((1, 2)) void DCALL hashset_visit(HashSet *__restrict self, Dee_visit_t proc, void *arg);
LOCAL NONNULL((1)) void DCALL hashset_initfrom_empty(HashSet *__restrict self);
LOCAL NONNULL((1)) void DCALL hashset_initfrom_hint(HashSet *__restrict self, size_t num_items, bool allow_overalloc);
LOCAL WUNUSED NONNULL((1)) int DCALL hashset_initfrom_seq(HashSet *__restrict self, DeeObject *seq);
PRIVATE WUNUSED NONNULL((1)) int DCALL hashset_assign(HashSet *self, DeeObject *seq);
PRIVATE WUNUSED NONNULL((1)) int DCALL hashset_moveassign(HashSet *self, HashSet *other);
PRIVATE WUNUSED NONNULL((1)) int DCALL hashset_ctor(HashSet *__restrict self);
PRIVATE WUNUSED NONNULL((1)) int DCALL hashset_copy(HashSet *__restrict self, HashSet *__restrict other);
PRIVATE WUNUSED NONNULL((1)) int DCALL hashset_init(HashSet *__restrict self, size_t argc, DeeObject *const *argv);
PRIVATE WUNUSED NONNULL((1)) int DCALL hashset_serialize(HashSet *__restrict self, DeeSerial *__restrict writer, Dee_seraddr_t addr);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL hashset_printrepr(HashSet *__restrict self, Dee_formatprinter_t printer, void *arg);
PRIVATE NONNULL((1)) int DCALL hashset_mh_clear(HashSet *__restrict self);

PRIVATE WUNUSED NONNULL((1)) int DCALL hashset_mh_seq_erase(HashSet *__restrict self, size_t start, size_t count);

PRIVATE WUNUSED NONNULL((1)) int DCALL hashset_mh_seq_reverse(HashSet *self, size_t start, size_t end);
PRIVATE NONNULL((1)) bool DCALL hashset_shrink_impl(HashSet *__restrict self, bool fully_shrink);
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL hashset_shrink(HashSet *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL hashset_reserve(HashSet *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE WUNUSED NONNULL((1)) bool DCALL hashset_cc(HashSet *__restrict self);

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL hashset_sizeof(HashSet *__restrict self);
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL hashset_mh_seq_compare(HashSet *lhs, DeeObject *rhs);
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL hashset_mh_seq_compare_eq(HashSet *lhs, DeeObject *rhs);
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL hashset_mh_seq_trycompare_eq(HashSet *lhs, DeeObject *rhs);
#else /* __INTELLISENSE__ */
DECL_END
#define DEFINE_HIGH_LEVEL
#define DEFINE_DeeHashSet
#include "dict-impl.c.inl"
DECL_BEGIN
#endif /* !__INTELLISENSE__ */

#ifdef DCALL_RETURN_COMMON
#define hashset_clear (*(void (DCALL *)(HashSet *__restrict))&hashset_mh_clear)
#else /* DCALL_RETURN_COMMON */
PRIVATE NONNULL((1)) void DCALL
hashset_clear(HashSet *__restrict self) {
	hashset_mh_clear(self);
}
#endif /* !DCALL_RETURN_COMMON */


PRIVATE WUNUSED NONNULL((1)) int DCALL
hashset_mh_seq_delitem_index(HashSet *__restrict self, size_t index) {
	struct Dee_hashset_item *item;
	DREF DeeObject *old_key;
	DeeHashSet_LockWrite(self);
	if unlikely(index >= self->hs_vused) {
		size_t real_size = self->hs_vused;
		DeeHashSet_LockEndWrite(self);
		DeeRT_ErrIndexOutOfBounds(Dee_AsObject(self), index, real_size);
		goto err;
	}
	if (_DeeHashSet_CanOptimizeVTab(self))
		hashset_optimize_vtab(self);
	item = &_DeeHashSet_GetRealVTab(self)[index];
	old_key       = item->hsi_key;
	item->hsi_key = NULL; /* Delete item */
	--self->hs_vused;
	hashset_autoshrink(self);
	DeeHashSet_LockEndWrite(self);
	Dee_Decref(old_key);
	return 0;
err:
	return -1;
}

/* Given an "index" in range `[0,d_used)', return a value
 * in range `[0,d_size)' that points to the index'th non-
 * deleted key in "d_vtab" */
PRIVATE WUNUSED NONNULL((1)) /*real*/ Dee_hash_vidx_t DCALL
hashset_unoptimize_vtab_index(HashSet *self, size_t index) {
	size_t result;
	ASSERT(self->hs_vused <= self->hs_vsize);
	ASSERT(index <= self->hs_vused);
	if (self->hs_vused == self->hs_vsize)
		return index; /* HashSet is fully optimized -> no translation needed */
	if (index <= 0)
		return 0;
	if (index >= self->hs_vused)
		return self->hs_vsize;
	for (result = 0; index; ++result) {
		ASSERT(result <= self->hs_vsize);
		if (_DeeHashSet_GetRealVTab(self)[result].hsi_key)
			--index;
	}
	return result;
}

struct hashset_mh_seq_setitem_index_impl_data {
	size_t          hssqsii_index;       /* in */
	DREF DeeObject *hssqsii_deleted_key; /* out[1..1] */
};

PRIVATE WUNUSED NONNULL((1)) /*virt*/ Dee_hash_vidx_t DCALL
hashset_mh_seq_setitem_index_impl_cb(void *arg, HashSet *self, /*virt*/ Dee_hash_vidx_t overwrite_index) {
	/*real*/ Dee_hash_vidx_t result;
	struct hashset_mh_seq_setitem_index_impl_data *data;
	struct Dee_hashset_item *real_vtab;
	data = (struct hashset_mh_seq_setitem_index_impl_data *)arg;
	if unlikely(data->hssqsii_index >= self->hs_vused) {
		size_t used = self->hs_vused;
		DeeHashSet_LockEndWrite(self);
		DeeRT_ErrIndexOutOfBounds(Dee_AsObject(self), data->hssqsii_index, used);
		return Dee_HASH_HTAB_EOF;
	}
	result = hashset_unoptimize_vtab_index(self, data->hssqsii_index);
	real_vtab = _DeeHashSet_GetRealVTab(self);
	for (;;) {
		ASSERT(result < self->hs_vsize);
		if (real_vtab[result].hsi_key)
			break;
		++result;
	}
	ASSERT(real_vtab[result].hsi_key);
	if (result != Dee_hash_vidx_toreal(overwrite_index)) {
		/* Delete item that used to exist at this index (sequence setitem acts as an overwrite operation) */
		data->hssqsii_deleted_key = real_vtab[result].hsi_key;
		real_vtab[result].hsi_key = NULL;
		--self->hs_vused;
	} else {
		/* for xchitem_index... */
		data->hssqsii_deleted_key = real_vtab[result].hsi_key;
		Dee_Incref(data->hssqsii_deleted_key);
	}
	return Dee_hash_vidx_tovirt(result);
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
hashset_mh_seq_setitem_index(HashSet *self, size_t index, DeeObject *key) {
	int result;
	struct hashset_mh_seq_setitem_index_impl_data data;
	data.hssqsii_index = index;
	data.hssqsii_deleted_key = NULL;
	result = hashset_insert_at(self, key, &hashset_mh_seq_setitem_index_impl_cb, &data);
	if (data.hssqsii_deleted_key)
		Dee_Decref(data.hssqsii_deleted_key);
	return result;
}

PRIVATE WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL
hashset_mh_seq_xchitem_index(HashSet *self, size_t index, DeeObject *key) {
	struct hashset_mh_seq_setitem_index_impl_data data;
	data.hssqsii_index       = index;
	data.hssqsii_deleted_key = NULL;
	if unlikely(hashset_insert_at(self, key, &hashset_mh_seq_setitem_index_impl_cb, &data)) {
		if (data.hssqsii_deleted_key)
			goto err_k;
		goto err;
	}
	ASSERT(data.hssqsii_deleted_key);
	return data.hssqsii_deleted_key;
err_k:
	Dee_Decref(data.hssqsii_deleted_key);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) /*virt*/ Dee_hash_vidx_t DCALL
hashset_mh_seq_insert_impl_cb(void *arg, HashSet *self,
                              /*virt*/ Dee_hash_vidx_t overwrite_index) {
	/*real*/ Dee_hash_vidx_t result;
	(void)overwrite_index;
	result = (/*real*/ Dee_hash_vidx_t)(size_t)(uintptr_t)arg;
	if unlikely(result >= self->hs_vused) {
		size_t used = self->hs_vused;
		DeeHashSet_LockEndWrite(self);
		DeeRT_ErrIndexOutOfBounds(Dee_AsObject(self), result, used);
		return Dee_HASH_HTAB_EOF;
	}
	result = hashset_unoptimize_vtab_index(self, result);
	return Dee_hash_vidx_tovirt(result);
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
hashset_mh_seq_insert(HashSet *self, size_t index, DeeObject *key) {
	return hashset_insert_at(self, key, &hashset_mh_seq_insert_impl_cb, (void *)(uintptr_t)index);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
hashset_mh_seq_pushfront(HashSet *self, DeeObject *key) {
	return hashset_mh_seq_insert(self, 0, key);
}

#if 1 /* Because regular "hashset_mh_insert" doesn't move already-present items to the back */
PRIVATE WUNUSED NONNULL((1)) /*virt*/ Dee_hash_vidx_t DCALL
hashset_mh_seq_append_cb(void *arg, HashSet *self,
                         /*virt*/ Dee_hash_vidx_t overwrite_index) {
	(void)arg;
	(void)overwrite_index;
	return Dee_hash_vidx_tovirt(self->hs_vsize);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
hashset_mh_seq_append(HashSet *self, DeeObject *key) {
	int result = hashset_insert_at(self, key, &hashset_mh_seq_append_cb, NULL);
	if (result > 0)
		result = 0;
	return result;
}
#else
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
hashset_mh_seq_append(HashSet *self, DeeObject *key) {
	int result = hashset_mh_insert(self, key); /* This will move "key" to back if it was already present */
	if (result > 0)
		result = 0;
	return result;
}
#endif

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
hashset_mh_seq_getitem_index(HashSet *__restrict self, size_t index) {
	DREF DeeObject *result;
	struct Dee_hashset_item *item;
	DeeHashSet_LockReadAndOptimize(self);
	if unlikely(index >= self->hs_vused) {
		size_t real_size = self->hs_vused;
		DeeHashSet_LockEndRead(self);
		DeeRT_ErrIndexOutOfBounds(Dee_AsObject(self), index, real_size);
		goto err;
	}
	item = &_DeeHashSet_GetRealVTab(self)[index];
	result = item->hsi_key;
	Dee_Incref(result);
	DeeHashSet_LockEndRead(self);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
hashset_mh_seq_trygetitem_index(HashSet *__restrict self, size_t index) {
	DREF DeeObject *result;
	struct Dee_hashset_item *item;
	DeeHashSet_LockReadAndOptimize(self);
	if unlikely(index >= self->hs_vused) {
		DeeHashSet_LockEndRead(self);
		return ITER_DONE;
	}
	item = &_DeeHashSet_GetRealVTab(self)[index];
	result = item->hsi_key;
	Dee_Incref(result);
	DeeHashSet_LockEndRead(self);
	return result;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
hashset_mh_seq_pop(HashSet *self, Dee_ssize_t index) {
	DREF DeeObject *result;
	struct Dee_hashset_item *item;
	DeeHashSet_LockWrite(self);
	if (index < 0)
		index += self->hs_vused;
	if unlikely((size_t)index >= self->hs_vused) {
		size_t real_size = self->hs_vused;
		DeeHashSet_LockEndWrite(self);
		DeeRT_ErrIndexOutOfBounds(Dee_AsObject(self), (size_t)index, real_size);
		goto err;
	}
	if (_DeeHashSet_CanOptimizeVTab(self))
		hashset_optimize_vtab(self);
	item = &_DeeHashSet_GetRealVTab(self)[index];
	result = item->hsi_key; /* Inherit reference */
	item->hsi_key = NULL; /* Delete item */
	--self->hs_vused;
	hashset_autoshrink(self);
	DeeHashSet_LockEndWrite(self);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) size_t DCALL
hashset_mh_seq_removeif(HashSet *self, DeeObject *should, size_t start, size_t end, size_t max) {
	size_t result = 0;
	while (start < end && result < max) {
		DREF DeeObject *item_key;
		struct Dee_hashset_item *item;
		DREF DeeObject *should_result_ob;
		int should_result;
		DeeHashSet_LockReadAndOptimize(self);
again_index_start:
		if (end > self->hs_vused) {
			end = self->hs_vused;
			if (start >= end) {
				DeeHashSet_LockEndRead(self);
				break;
			}
		}
		item = &_DeeHashSet_GetRealVTab(self)[start];
		item_key = item->hsi_key;
		ASSERT(item_key);
		Dee_Incref(item_key);
		DeeHashSet_LockEndRead(self);
		should_result_ob = DeeObject_Call(should, 1, &item_key);
		Dee_Decref_unlikely(item_key);
		if unlikely(!should_result_ob)
			goto err;
		should_result = DeeObject_BoolInherited(should_result_ob);
		if (Dee_HAS_ISNO_OR_ERR(should_result)) {
			if (Dee_HAS_ISERR(should_result))
				goto err;
			++start;
			continue;
		}

		/* Yes: *should* delete this item */
		DeeHashSet_LockWrite(self);
		if unlikely(_DeeHashSet_CanOptimizeVTab(self))
			hashset_optimize_vtab(self);
		if unlikely(item != &_DeeHashSet_GetRealVTab(self)[start])
			goto downgrade_and_again_index_start;
		if unlikely(item->hsi_key != item_key)
			goto downgrade_and_again_index_start;
		item->hsi_key = NULL; /* Delete item */
		--self->hs_vused;
		hashset_autoshrink(self);
		hashset_optimize_vtab(self);
		ASSERT(self->hs_vused == self->hs_vsize);
		++result;
		++start;
		if (start >= end || result >= max) {
			DeeHashSet_LockEndWrite(self);
			Dee_Decref(item_key);
			break;
		}
		DeeHashSet_LockDowngrade(self);
		Dee_Decref(item_key);
		goto again_index_start;
	}
	return result;
downgrade_and_again_index_start:
	DeeHashSet_LockDowngrade(self);
	goto again_index_start;
err:
	return (size_t)-1;
}


PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
hashset_mh_seq_foreach(HashSet *__restrict self, Dee_foreach_t cb, void *arg) {
	Dee_hash_vidx_t i;
	Dee_ssize_t temp, result = 0;
	DeeHashSet_LockRead(self);
	for (i = Dee_hash_vidx_tovirt(0);
	     Dee_hash_vidx_virt_lt_real(i, self->hs_vsize); ++i) {
		DREF DeeObject *item_key;
		struct Dee_hashset_item *item;
		item = &_DeeHashSet_GetVirtVTab(self)[i];
		item_key  = item->hsi_key;
		if unlikely(!item_key)
			continue; /* Deleted, and not-yet-optimized key */
		Dee_Incref(item_key);
		DeeHashSet_LockEndRead(self);
		temp = (*cb)(arg, item_key);
		Dee_Decref_unlikely(item_key);
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
		DeeHashSet_LockRead(self);
	}
	DeeHashSet_LockEndRead(self);
	return result;
err_temp:
	return temp;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
hashset_mh_seq_foreach_reverse(HashSet *__restrict self, Dee_foreach_t cb, void *arg) {
	Dee_hash_vidx_t i;
	Dee_ssize_t temp, result = 0;
	DeeHashSet_LockRead(self);
	i = Dee_hash_vidx_tovirt(self->hs_vsize);
	while (i > Dee_hash_vidx_tovirt(0)) {
		DREF DeeObject *item_key;
		struct Dee_hashset_item *item;
		--i;
		item = &_DeeHashSet_GetVirtVTab(self)[i];
		item_key  = item->hsi_key;
		if unlikely(!item_key)
			continue; /* Deleted, and not-yet-optimized key */
		Dee_Incref(item_key);
		DeeHashSet_LockEndRead(self);
		temp = (*cb)(arg, item_key);
		Dee_Decref_unlikely(item_key);
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
		DeeHashSet_LockRead(self);
		if unlikely(i > Dee_hash_vidx_tovirt(self->hs_vsize))
			i = Dee_hash_vidx_tovirt(self->hs_vsize);
	}
	DeeHashSet_LockEndRead(self);
	return result;
err_temp:
	return temp;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
hashset_mh_seq_enumerate_index(HashSet *__restrict self, Dee_seq_enumerate_index_t cb,
                               void *arg, size_t start, size_t end) {
	Dee_ssize_t temp, result = 0;
	while (start < end) {
		DREF DeeObject *item_key;
		struct Dee_hashset_item *item;
		DeeHashSet_LockReadAndOptimize(self);
		if (end > self->hs_vused) {
			end = self->hs_vused;
			if (start >= end) {
				DeeHashSet_LockEndRead(self);
				break;
			}
		}
		item = &_DeeHashSet_GetRealVTab(self)[start];
		item_key = item->hsi_key;
		Dee_Incref(item_key);
		DeeHashSet_LockEndRead(self);
		temp = (*cb)(arg, start, item_key);
		Dee_Decref_unlikely(item_key);
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
		++start;
	}
	return result;
err_temp:
	return temp;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
hashset_mh_seq_enumerate_index_reverse(HashSet *__restrict self, Dee_seq_enumerate_index_t cb,
                                       void *arg, size_t start, size_t end) {
	Dee_ssize_t temp, result = 0;
	while (start < end) {
		DREF DeeObject *item_key;
		struct Dee_hashset_item *item;
		DeeHashSet_LockReadAndOptimize(self);
		if (end > self->hs_vused) {
			end = self->hs_vused;
			if (start >= end) {
				DeeHashSet_LockEndRead(self);
				break;
			}
		}
		--end;
		item = &_DeeHashSet_GetRealVTab(self)[end];
		item_key = item->hsi_key;
		Dee_Incref(item_key);
		DeeHashSet_LockEndRead(self);
		temp = (*cb)(arg, end, item_key);
		Dee_Decref_unlikely(item_key);
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
	}
	return result;
err_temp:
	return temp;
}

PRIVATE WUNUSED NONNULL((1)) DREF HashSetIterator *DCALL
hashset_iter(HashSet *__restrict self) {
	DREF HashSetIterator *result = DeeObject_MALLOC(HashSetIterator);
	if unlikely(!result)
		goto err;
	Dee_Incref(self);
	result->hsi_set  = self;
	result->hsi_vidx = Dee_hash_vidx_tovirt(0);
	DeeObject_InitStatic(result, &HashSetIterator_Type);
	return result;
err:
	return NULL;
}


#if __SIZEOF_INT__ == __SIZEOF_SIZE_T__
#define hashset_mh_insertall_cb (*(Dee_ssize_t (DCALL *)(void *, DeeObject *))&hashset_mh_insert)
#define hashset_mh_removeall_cb (*(Dee_ssize_t (DCALL *)(void *, DeeObject *))&hashset_mh_remove)
#else /* __SIZEOF_INT__ == __SIZEOF_SIZE_T__ */
PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
hashset_mh_insertall_cb(void *self, DeeObject *key) {
	return (Dee_ssize_t)hashset_mh_insert((HashSet *)self, key);
}
PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
hashset_mh_removeall_cb(void *self, DeeObject *key) {
	return (Dee_ssize_t)hashset_mh_remove((HashSet *)self, key);
}
#endif /* __SIZEOF_INT__ != __SIZEOF_SIZE_T__ */

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
hashset_mh_insertall(HashSet *self, DeeObject *keys) {
	Dee_ssize_t result = DeeObject_Foreach(keys, &hashset_mh_insertall_cb, self);
	if (result > 0)
		result = 0;
	return (int)result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
hashset_mh_removeall(HashSet *self, DeeObject *keys) {
	Dee_ssize_t result = DeeObject_Foreach(keys, &hashset_mh_removeall_cb, self);
	if (result > 0)
		result = 0;
	return (int)result;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
hashset_contains(HashSet *self, DeeObject *key) {
	int result = hashset_mh_contains(self, key);
	if unlikely(result < 0)
		goto err;
	return_bool(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
hashset_mh_pop_impl(HashSet *__restrict self) {
	DREF DeeObject *result;
	struct Dee_hashset_item *item;
	DeeHashSet_LockWrite(self);
	if unlikely(!self->hs_vused) {
		DeeHashSet_LockEndWrite(self);
		return ITER_DONE;
	}
	item = _DeeHashSet_GetRealVTab(self) + self->hs_vsize - 1;
	while (!item->hsi_key) {
		ASSERT(_DeeHashSet_CanOptimizeVTab(self));
		ASSERT(item > _DeeHashSet_GetRealVTab(self));
		--item;
	}
	result = item->hsi_key; /* Inherit reference */
	item->hsi_key = NULL;
	--self->hs_vused;
	hashset_autoshrink(self);
	DeeHashSet_LockEndWrite(self);
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
hashset_mh_pop(HashSet *__restrict self) {
	DREF DeeObject *result = hashset_mh_pop_impl(self);
	if unlikely(result == ITER_DONE) {
		DeeRT_ErrEmptySequence(self);
		result = NULL;
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
hashset_mh_pop_with_default(HashSet *self, DeeObject *def) {
	DREF DeeObject *result = hashset_mh_pop_impl(self);
	if unlikely(result == ITER_DONE) {
		Dee_Incref(def);
		result = def;
	}
	return result;
}


STATIC_ASSERT(offsetof(DeeHashSetObject, hs_valloc) == offsetof(DeeDictObject, d_valloc));
#ifndef CONFIG_NO_THREADS
STATIC_ASSERT(offsetof(DeeHashSetObject, hs_lock) == offsetof(DeeDictObject, d_lock));
#endif /* !CONFIG_NO_THREADS */
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
dict___hidxio__(DeeDictObject *__restrict self);
#define hashset___hidxio__ dict___hidxio__


STATIC_ASSERT(offsetof(DeeHashSetObject, hs_vused) == offsetof(DeeDictObject, d_vused));
#ifndef CONFIG_NO_THREADS
STATIC_ASSERT(offsetof(DeeHashSetObject, hs_lock) == offsetof(DeeDictObject, d_lock));
#endif /* !CONFIG_NO_THREADS */
INTDEF WUNUSED NONNULL((1)) int DCALL dict_nonempty_as_bound(DeeDictObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL dict_bool(DeeDictObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) size_t DCALL dict_size(DeeDictObject *__restrict self);
#define hashset_nonempty_as_bound dict_nonempty_as_bound
#define hashset_bool              dict_bool
#define hashset_size              dict_size
#define hashset_size_fast         hashset_size





PRIVATE struct type_gc tpconst hashset_gc = {
	/* .tp_clear = */ (void (DCALL *)(DeeObject *__restrict))&hashset_clear,
	/* .tp_cc    = */ (bool (DCALL *)(DeeObject *__restrict))&hashset_cc,
};

PRIVATE struct type_seq hashset_seq = {
	/* .tp_iter         = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&hashset_iter,
	/* .tp_sizeob       = */ DEFIMPL(&default__sizeob__with__size),
	/* .tp_contains     = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&hashset_contains,
	/* .tp_getitem      = */ NULL,
	/* .tp_delitem      = */ NULL,
	/* .tp_setitem      = */ NULL,
	/* .tp_getrange     = */ NULL,
	/* .tp_delrange     = */ NULL,
	/* .tp_setrange     = */ NULL,
	/* .tp_foreach      = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&hashset_mh_seq_foreach,
	/* .tp_foreach_pair = */ NULL,
	/* .tp_bounditem    = */ NULL,
	/* .tp_hasitem      = */ NULL,
	/* .tp_size         = */ (size_t (DCALL *)(DeeObject *__restrict))&hashset_size,
	/* .tp_size_fast    = */ (size_t (DCALL *)(DeeObject *__restrict))&hashset_size_fast,
};


#ifdef __INTELLISENSE__
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL hashset_trygetfirst(HashSet *__restrict self);
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL hashset_trygetlast(HashSet *__restrict self);
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL hashset_getfirst(HashSet *__restrict self);
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL hashset_getlast(HashSet *__restrict self);
PRIVATE WUNUSED NONNULL((1)) int DCALL hashset_delfirst(HashSet *__restrict self);
PRIVATE WUNUSED NONNULL((1)) int DCALL hashset_dellast(HashSet *__restrict self);
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL hashset_setfirst(HashSet *self, DeeObject *value);
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL hashset_setlast(HashSet *self, DeeObject *value);
#else /* __INTELLISENSE__ */
DECL_END
#define DEFINE_hashset_first
#include "dict-firstlast-impl.c.inl"
#define DEFINE_hashset_last
#include "dict-firstlast-impl.c.inl"
DECL_BEGIN
#endif /* !__INTELLISENSE__ */


#ifndef CONFIG_NO_DEEMON_100_COMPAT
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
deprecated_d100_get_maxloadfactor(DeeObject *__restrict self);
#define deprecated_d100_del_maxloadfactor (*(int (DCALL *)(DeeObject *))&_DeeNone_reti0_1)
#define deprecated_d100_set_maxloadfactor (*(int (DCALL *)(DeeObject *, DeeObject *))&_DeeNone_reti0_2)
#endif /* !CONFIG_NO_DEEMON_100_COMPAT */

PRIVATE struct type_getset tpconst hashset_getsets[] = {
	TYPE_GETSET_BOUND_NODOC(STR_first, &hashset_getfirst, &hashset_delfirst, &hashset_setfirst, &hashset_nonempty_as_bound),
	TYPE_GETSET_BOUND_NODOC(STR_last, &hashset_getlast, &hashset_dellast, &hashset_setlast, &hashset_nonempty_as_bound),
	TYPE_GETTER_AB(STR_cached, &DeeObject_NewRef, "->?."),
	TYPE_GETTER_AB(STR_frozen, &DeeRoSet_FromHashSet, "->?#Frozen"),
	TYPE_GETTER_AB_F("__sizeof__", &hashset_sizeof, METHOD_FNOREFESCAPE, "->?Dint"),
	TYPE_GETTER_AB_F("__hidxio__", &hashset___hidxio__, METHOD_FNOREFESCAPE,
	                 "->?Dint\n"
	                 "Size shift-multipler for htab words (word size is ${1 << __hidxio__})\n"
	                 "#T{?#__hidxio__|htab word type~"
	                 /**/ "$0|?Ectypes:uint8_t"
	                 /**/ IF_Dee_HASH_HIDXIO_COUNT_GE_2("&" "$1|?Ectypes:uint16_t")
	                 /**/ IF_Dee_HASH_HIDXIO_COUNT_GE_3("&" "$2|?Ectypes:uint32_t")
	                 /**/ IF_Dee_HASH_HIDXIO_COUNT_GE_4("&" "$3|?Ectypes:uint64_t")
	                 "}"),

#ifndef CONFIG_NO_DEEMON_100_COMPAT
	TYPE_GETSET_AB_F("max_load_factor",
	                 &deprecated_d100_get_maxloadfactor,
	                 &deprecated_d100_del_maxloadfactor,
	                 &deprecated_d100_set_maxloadfactor,
	                 METHOD_FNOREFESCAPE | METHOD_FCONSTCALL,
	                 "->?Dfloat\n"
	                 "Deprecated. Always returns ${1.0}, with del/set being ignored"),
#endif /* !CONFIG_NO_DEEMON_100_COMPAT */
	TYPE_GETSET_END
};


STATIC_ASSERT(offsetof(HashSet, hs_valloc) == offsetof(DeeDictObject, d_valloc));
STATIC_ASSERT(offsetof(HashSet, hs_vsize) == offsetof(DeeDictObject, d_vsize));
STATIC_ASSERT(offsetof(HashSet, hs_vused) == offsetof(DeeDictObject, d_vused));
STATIC_ASSERT(offsetof(HashSet, hs_hmask) == offsetof(DeeDictObject, d_hmask));
INTDEF struct type_member tpconst dict_members[];
#define hashset_members dict_members


PRIVATE struct type_member tpconst hashset_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &HashSetIterator_Type),
	TYPE_MEMBER_CONST(STR_Frozen, &DeeRoSet_Type),
	TYPE_MEMBER_CONST(STR___seq_getitem_always_bound__, Dee_True), /* Must be specified because we also define "__seq_getitem__" */
	TYPE_MEMBER_END
};

PRIVATE struct type_method tpconst hashset_methods[] = {
//	TYPE_KWMETHOD("byhash", &hashset_byhash, DOC_GET(map_byhash_doc)), /* TODO */
	TYPE_METHOD_HINTREF(Sequence_clear),
	TYPE_METHOD_HINTREF(Sequence_contains),
	TYPE_METHOD_HINTREF(Set_unify),
	TYPE_METHOD_HINTREF(Set_insert),
	TYPE_METHOD_HINTREF(Set_insertall),
	TYPE_METHOD_HINTREF(Set_remove),
	TYPE_METHOD_HINTREF(Set_removeall),
	TYPE_METHOD_HINTREF(Set_pop),

	TYPE_KWMETHOD_F("shrink", &hashset_shrink, METHOD_FNOREFESCAPE,
	                "(fully=!t)->?Dbool\n"
	                "#pfully{When !f, the same sort of shrinking as done automatically when an item is removed. "
	                /*   */ "When !t, force deallocation of #Iall unused items, even if that ruins hash "
	                /*   */ "characteristics / memory efficiency}"
	                "#r{Returns !t if memory was freed}"
	                "Release unused memory. In terms of implementation, after a call ${shrink(true)}, the ?. "
	                /**/ "will have been modified such that ?#__valloc__ #C{==} ?#__vused__ #C{==} ?#__vsize__, "
	                /**/ "and ?#__vmask__ will be the smallest hash-mask able to describe ?#__valloc__ items."),

	TYPE_KWMETHOD_F("reserve", &hashset_reserve, METHOD_FNOREFESCAPE,
	                "(total=!0,more=!0,weak=!f)->?Dbool\n"
	                "#pweak{Should the size-hint be considered weak? (s.a. ?#{op:constructor}'s #Cweak argument)}"
	                "#r{Indicative of the ?. having sufficient preallocated space on return}"
	                "Try to preallocate buffer space for ${({#this, total} > ...) + more} items"),

	TYPE_METHOD_HINTREF(__seq_xchitem__),
	TYPE_METHOD_HINTREF(__seq_erase__),
	TYPE_METHOD_HINTREF(__seq_insert__),
	TYPE_METHOD_HINTREF(__seq_append__),
	TYPE_METHOD_HINTREF(__seq_pushfront__),
	TYPE_METHOD_HINTREF(__seq_pop__),
	TYPE_METHOD_HINTREF(__seq_removeif__),
	TYPE_METHOD_HINTREF(__seq_reverse__),
	TYPE_METHOD_HINTREF(__seq_iter__),
	TYPE_METHOD_HINTREF(__seq_enumerate__),
	TYPE_METHOD_HINTREF(__seq_getitem__),
	TYPE_METHOD_HINTREF(__seq_delitem__),
	TYPE_METHOD_HINTREF(__seq_setitem__),
	TYPE_METHOD_HINTREF(__seq_compare__),
	TYPE_METHOD_HINTREF(__seq_compare_eq__),
	TYPE_METHOD_HINTREF(__seq_find__),
	TYPE_METHOD_HINTREF(__seq_rfind__),
	TYPE_METHOD_END
};

PRIVATE struct type_method_hint tpconst hashset_method_hints[] = {
	TYPE_METHOD_HINT_F(seq_clear, &hashset_mh_clear, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_contains, &hashset_mh_contains, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_contains_with_key, &default__seq_contains_with_key__with__seq_operator_foreach, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_contains_with_range, &hashset_mh_contains_with_range, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_contains_with_range_and_key, &default__seq_contains_with_range_and_key__with__seq_enumerate_index, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_find, &hashset_mh_find, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_find_with_key, &default__seq_find_with_key__with__seq_enumerate_index, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_rfind, &hashset_mh_find, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_rfind_with_key, &default__seq_rfind_with_key__with__seq_enumerate_index_reverse, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(set_unify, &hashset_mh_unify, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(set_insert, &hashset_mh_insert, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(set_insertall, &hashset_mh_insertall, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(set_remove, &hashset_mh_remove, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(set_removeall, &hashset_mh_removeall, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(set_pop, &hashset_mh_pop, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(set_pop_with_default, &hashset_mh_pop_with_default, METHOD_FNOREFESCAPE),

	/* Operators for "HashSet as Sequence" (specifically defined because hashsets are ordered) */
	TYPE_METHOD_HINT_F(seq_foreach_reverse, &hashset_mh_seq_foreach_reverse, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_enumerate_index_reverse, &hashset_mh_seq_enumerate_index_reverse, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_trygetfirst, &hashset_trygetfirst, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_trygetlast, &hashset_trygetlast, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(set_trygetfirst, &hashset_trygetfirst, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(set_trygetlast, &hashset_trygetlast, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_enumerate_index, &hashset_mh_seq_enumerate_index, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_operator_iter, &hashset_iter, METHOD_FNORMAL),
	TYPE_METHOD_HINT_F(seq_operator_foreach, &hashset_mh_seq_foreach, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_operator_hasitem_index, &default__seq_operator_hasitem_index__with__seq_operator_size, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_operator_getitem_index, &hashset_mh_seq_getitem_index, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_operator_trygetitem_index, &hashset_mh_seq_trygetitem_index, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_operator_delitem_index, &hashset_mh_seq_delitem_index, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_operator_setitem_index, &hashset_mh_seq_setitem_index, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_operator_compare_eq, &hashset_mh_seq_compare_eq, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_operator_compare, &hashset_mh_seq_compare, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_operator_trycompare_eq, &hashset_mh_seq_trycompare_eq, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_xchitem_index, &hashset_mh_seq_xchitem_index, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_erase, &hashset_mh_seq_erase, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_insert, &hashset_mh_seq_insert, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_pushfront, &hashset_mh_seq_pushfront, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_append, &hashset_mh_seq_append, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_pop, &hashset_mh_seq_pop, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_removeif, &hashset_mh_seq_removeif, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_reverse, &hashset_mh_seq_reverse, METHOD_FNOREFESCAPE),
//TODO:	TYPE_METHOD_HINT_F(seq_sort, &hashset_mh_sort, METHOD_FNOREFESCAPE),
//TODO:	TYPE_METHOD_HINT_F(seq_sort_with_key, &hashset_mh_sort_with_key, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_END
};

PRIVATE struct type_operator const hashset_operators[] = {
	TYPE_OPERATOR_FLAGS(OPERATOR_0001_COPY, METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0004_ASSIGN, METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0005_MOVEASSIGN, METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0006_STR, METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0007_REPR, METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0008_BOOL, METHOD_FNOTHROW | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0028_HASH, METHOD_FNOTHROW | METHOD_FNOREFESCAPE),
//	TYPE_OPERATOR_FLAGS(OPERATOR_0029_EQ, METHOD_FNOREFESCAPE),
//	TYPE_OPERATOR_FLAGS(OPERATOR_002A_NE, METHOD_FNOREFESCAPE),
//	TYPE_OPERATOR_FLAGS(OPERATOR_002B_LO, METHOD_FNOREFESCAPE),
//	TYPE_OPERATOR_FLAGS(OPERATOR_002C_LE, METHOD_FNOREFESCAPE),
//	TYPE_OPERATOR_FLAGS(OPERATOR_002D_GR, METHOD_FNOREFESCAPE),
//	TYPE_OPERATOR_FLAGS(OPERATOR_002E_GE, METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_002F_ITER, METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0030_SIZE, METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0031_CONTAINS, METHOD_FNOREFESCAPE),
};

/* The main `HashSet' container class */
PUBLIC DeeTypeObject DeeHashSet_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ DeeString_STR(&str_HashSet),
	/* .tp_doc      = */ DOC("A mutable set-like container that uses hashing to detect/prevent duplicates. "
	                         /**/ "Similar to ?DDict, ?. also retains the order in which elements were (first) "
	                         /**/ "inserted, meaning that ?#{op:iter} yields consistent and well-defined results, "
	                         /**/ "and that operations like ?#__seq_insert__ (?Ainsert?DSequence) behave as one "
	                         /**/ "would expect them to behave.\n"
	                         "\n"

	                         "()\n"
	                         "Create a new, empty ?.\n"
	                         "\n"

	                         "(hint:?Dint,weak=!f)\n"
	                         "#pweak{When true, @hint represents a lower-bound guess for how many keys to allocate. "
	                         /*  */ "In this case, then runtime may allocate much more space than needed if "
	                         /*  */ "doing so would be appropriate for the hash-mask appropriate for @hint}"
	                         "Create a new ?., while trying to pre-alloc enough space for @hint keys. When @hint is "
	                         /**/ "too large to pre-allocate a buffer of sufficient size, a smaller buffer, or no buffer "
	                         /**/ "at all may be pre-allocated.\n"
	                         "\n"

	                         "(keys:?S?O)\n"
	                         "Create a new HashSet populated with elements from @keys\n"
	                         "\n"

	                         ":=(keys:?S?O)->\n"
	                         "Replace the contents of @this ?. with @keys\n"
	                         "\n"

	                         "move:=(other:?.)->\n"
	                         "Similar to ?#{op:assign}, but also clear @other at the same time\n"
	                         "\n"

	                         "copy->\n"
	                         "Returns a shallow copy of @this ?.\n"
	                         "\n"

	                         "deepcopy->\n"
	                         "Returns a deep copy of @this ?.\n"
	                         "\n"

	                         "bool->\n"
	                         "Returns ?t if @this ?. is non-empty\n"
	                         "\n"

	                         "iter->\n"
	                         "Enumerate keys stored in the ?., in order of being added\n"
	                         "\n"

	                         "#->\n"
	                         "Returns the number of keys within @this ?.\n"
	                         "\n"

	                         "contains->\n"
	                         "Returns ?t if @item is apart of @this ?. (s.a. ?#contains)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FGC | TP_FNAMEOBJECT,
	/* .tp_weakrefs = */ Dee_WEAKREF_SUPPORT_ADDR(HashSet),
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSet_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED_GC(
			/* T:              */ HashSet,
			/* tp_ctor:        */ &hashset_ctor,
			/* tp_copy_ctor:   */ &hashset_copy,
			/* tp_any_ctor:    */ &hashset_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &hashset_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&hashset_fini,
		/* .tp_assign      = */ (int (DCALL *)(DeeObject *, DeeObject *))&hashset_assign,
		/* .tp_move_assign = */ (int (DCALL *)(DeeObject *, DeeObject *))&hashset_moveassign,
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ DEFIMPL(&object_str),
		/* .tp_repr      = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool      = */ (int (DCALL *)(DeeObject *__restrict))&hashset_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&hashset_printrepr,
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&hashset_visit,
	/* .tp_gc            = */ &hashset_gc,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__47C97A4265F9F31F),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__48CC5897A5CA5795), /* TODO */
	/* .tp_seq           = */ &hashset_seq,
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__C6F8E138F179B5AD),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ hashset_methods,
	/* .tp_getsets       = */ hashset_getsets,
	/* .tp_members       = */ hashset_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ hashset_class_members,
	/* .tp_method_hints  = */ hashset_method_hints,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
	/* .tp_mro           = */ NULL,
	/* .tp_operators     = */ hashset_operators,
	/* .tp_operators_size= */ COMPILER_LENOF(hashset_operators),
};

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_HASHSET_C */
