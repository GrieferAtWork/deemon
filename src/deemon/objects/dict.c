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
#ifndef GUARD_DEEMON_OBJECTS_DICT_C
#define GUARD_DEEMON_OBJECTS_DICT_C 1

#include <deemon/api.h>

#include <deemon/alloc.h>              /* DeeObject_MALLOC, Dee_*alloc*, Dee_Free, Dee_TYPE_CONSTRUCTOR_INIT_FIXED, Dee_TYPE_CONSTRUCTOR_INIT_FIXED_GC */
#include <deemon/arg.h>                /* DeeArg_Unpack1, DeeArg_UnpackStructKw */
#include <deemon/bool.h>               /* Dee_True, return_bool */
#include <deemon/computed-operators.h> /* DEFIMPL, DEFIMPL_UNSUPPORTED */
#include <deemon/dict.h>               /* DeeDictObject, DeeDict_*, Dee_dict_item, _DeeDict_* */
#include <deemon/error-rt.h>           /* DeeRT_Err* */
#include <deemon/float.h>              /*  */
#include <deemon/format.h>             /* PRFuSIZ, PRFxSIZ */
#include <deemon/gc.h>                 /* DeeGCObject_FREE, DeeGCObject_MALLOC, Dee_TYPE_CONSTRUCTOR_INIT_FIXED_GC */
#include <deemon/hashset.h>            /* DeeHashSetObject, DeeHashSet_Type */
#include <deemon/int.h>                /* DeeInt_NEWU */
#include <deemon/map.h>                /* DeeMap_Type */
#include <deemon/method-hints.h>       /* Dee_seq_enumerate_index_t, TYPE_METHOD_HINT*, type_method_hint */
#include <deemon/none-operator.h>      /* _DeeNone_reti0_1, _DeeNone_reti0_2 */
#include <deemon/none.h>               /* Dee_None, return_none */
#include <deemon/object.h>             /* DREF, DeeObject, DeeObject_*, DeeTypeObject, Dee_AsObject, Dee_BOUND_*, Dee_COMPARE_ERR, Dee_Decref*, Dee_Incref, Dee_Incref_n, Dee_TYPE, Dee_WEAKREF_SUPPORT_ADDR, Dee_foreach_pair_t, Dee_foreach_t, Dee_formatprinter_t, Dee_hash_t, Dee_return_compareT, Dee_ssize_t, Dee_weakref_support_init, ITER_DONE, OBJECT_HEAD_INIT */
#include <deemon/pair.h>               /* DeeSeqPairObject, DeeSeqPair_ELEM, DeeSeq_* */
#include <deemon/rodict.h>             /* DeeRoDict* */
#include <deemon/roset.h>              /* DeeRoSetObject, DeeRoSet_Type */
#include <deemon/seq.h>                /* DeeIterator_Type, DeeSeq_Unpack */
#include <deemon/serial.h>             /* DeeSerial, Dee_seraddr_t */
#include <deemon/string.h>             /* DeeString_STR */
#include <deemon/system-features.h>    /* memset */
#include <deemon/type.h>               /* DeeObject_InitStatic, DeeObject_IsShared, DeeType_Type, Dee_TYPE_CONSTRUCTOR_INIT_FIXED, Dee_TYPE_CONSTRUCTOR_INIT_FIXED_GC, Dee_visit_t, METHOD_F*, OPERATOR_*, STRUCT_*, TF_NONE, TP_F*, TYPE_*, type_* */
#include <deemon/util/atomic.h>        /* atomic_cmpxch_or_write, atomic_read */
#include <deemon/util/hash-io.h>       /* Dee_HASH_*, Dee_SIZEOF_HASH_VIDX_T, Dee_hash_*, IF_Dee_HASH_HIDXIO_COUNT_GE_* */
#include <deemon/util/lock.h>          /* Dee_atomic_read_with_atomic_rwlock, Dee_atomic_rwlock_* */

#include <hybrid/align.h>    /* IS_POWER_OF_TWO */
#include <hybrid/overflow.h> /* OVERFLOW_UADD */
#include <hybrid/typecore.h> /* __BYTE_TYPE__, __SIZEOF_INT__, __SIZEOF_SIZE_T__ */

#include "../runtime/kwlist.h"
#include "../runtime/method-hint-defaults.h"
#include "../runtime/runtime_error.h"
#include "../runtime/strings.h"
#include "dict-utils.h"
#include "dict.h"
#include "generic-proxy.h"
#include "seq/default-compare.h"
#include "seq/default-map-proxy.h"

#include <stdbool.h> /* bool, false, true */
#include <stddef.h>  /* NULL, offsetof, size_t */
#include <stdint.h>  /* uintptr_t */

#undef byte_t
#define byte_t __BYTE_TYPE__

DECL_BEGIN

#ifndef NDEBUG
#define DBG_memset (void)memset
#else /* !NDEBUG */
#define DBG_memset(dst, byte, n_bytes) (void)0
#endif /* NDEBUG */

typedef DeeDictObject Dict;

/************************************************************************/
/* ITERATOR                                                             */
/************************************************************************/
STATIC_ASSERT(offsetof(DictIterator, di_dict) == offsetof(ProxyObject, po_obj));
#define diter_fini      generic_proxy__fini
#define diter_visit     generic_proxy__visit
#define diter_serialize generic_proxy__serialize_and_wordcopy_atomic(Dee_SIZEOF_HASH_VIDX_T)

PRIVATE WUNUSED NONNULL((1)) int DCALL
diter_ctor(DictIterator *__restrict self) {
	self->di_dict = (DREF Dict *)DeeDict_New();
	if unlikely(!self->di_dict)
		goto err;
	self->di_vidx = Dee_hash_vidx_tovirt(0);
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL /* "INTERN" because shared with "./hashset.c" */
diter_copy(DictIterator *__restrict self,
           DictIterator *__restrict other) {
	self->di_dict = other->di_dict;
	Dee_Incref(self->di_dict);
	self->di_vidx = atomic_read(&other->di_vidx);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
diter_init(DictIterator *__restrict self,
           size_t argc, DeeObject *const *argv) {
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("_DictIterator", params: "
	DeeDictObject *dict;
", docStringPrefix: "diter");]]]*/
#define diter__DictIterator_params "dict:?DDict"
	struct {
		DeeDictObject *dict;
	} args;
	DeeArg_Unpack1(err, argc, argv, "_DictIterator", &args.dict);
/*[[[end]]]*/
	if (DeeObject_AssertType(args.dict, &DeeDict_Type))
		goto err;
	self->di_dict = args.dict;
	Dee_Incref(args.dict);
	self->di_vidx = Dee_hash_vidx_tovirt(0);
	return 0;
err:
	return -1;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
diter_nextkey(DictIterator *__restrict self) {
	DREF DeeObject *result;
	Dict *dict = self->di_dict;
	for (;;) {
		/*virt*/ Dee_hash_vidx_t old_vidx = atomic_read(&self->di_vidx);
		/*virt*/ Dee_hash_vidx_t new_vidx = old_vidx;
		DeeDict_LockRead(dict);
		do {
			if unlikely(new_vidx > dict->d_vsize) {
				DeeDict_LockEndRead(dict);
				return ITER_DONE;
			}
			result = _DeeDict_GetVirtVTab(dict)[new_vidx].di_key;
			++new_vidx;
		} while (!result);
		Dee_Incref(result);
		DeeDict_LockEndRead(dict);
		if (atomic_cmpxch_or_write(&self->di_vidx, old_vidx, new_vidx))
			break;
		Dee_Decref_unlikely(result);
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
diter_nextvalue(DictIterator *__restrict self) {
	DREF DeeObject *result;
	Dict *dict = self->di_dict;
	for (;;) {
		DeeObject *key;
		/*virt*/ Dee_hash_vidx_t old_vidx = atomic_read(&self->di_vidx);
		/*virt*/ Dee_hash_vidx_t new_vidx = old_vidx;
		struct Dee_dict_item *virt_vtab;
		DeeDict_LockRead(dict);
		virt_vtab = _DeeDict_GetVirtVTab(dict);
		do {
			if unlikely(new_vidx > dict->d_vsize) {
				DeeDict_LockEndRead(dict);
				return ITER_DONE;
			}
			key    = virt_vtab[new_vidx].di_key;
			result = virt_vtab[new_vidx].di_value;
			++new_vidx;
		} while (!key);
		Dee_Incref(result);
		DeeDict_LockEndRead(dict);
		if (atomic_cmpxch_or_write(&self->di_vidx, old_vidx, new_vidx))
			break;
		Dee_Decref_unlikely(result);
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
diter_nextpair(DictIterator *__restrict self,
               DREF DeeObject *key_and_value[2]) {
	Dict *dict = self->di_dict;
	for (;;) {
		/*virt*/ Dee_hash_vidx_t old_vidx = atomic_read(&self->di_vidx);
		/*virt*/ Dee_hash_vidx_t new_vidx = old_vidx;
		struct Dee_dict_item *virt_vtab;
		DeeDict_LockRead(dict);
		virt_vtab = _DeeDict_GetVirtVTab(dict);
		do {
			if unlikely(new_vidx > dict->d_vsize) {
				DeeDict_LockEndRead(dict);
				return 1;
			}
			key_and_value[0] = virt_vtab[new_vidx].di_key;
			key_and_value[1] = virt_vtab[new_vidx].di_value;
			++new_vidx;
		} while (!key_and_value[0]);
		Dee_Incref(key_and_value[0]);
		Dee_Incref(key_and_value[1]);
		DeeDict_LockEndRead(dict);
		if (atomic_cmpxch_or_write(&self->di_vidx, old_vidx, new_vidx))
			break;
		Dee_Decref_unlikely(key_and_value[1]);
		Dee_Decref_unlikely(key_and_value[0]);
	}
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
diter_advance(DictIterator *__restrict self, size_t skip) {
	/*virt*/ Dee_hash_vidx_t old_vidx, new_vidx;
	Dict *dict = self->di_dict;
	for (;;) {
		old_vidx = atomic_read(&self->di_vidx);
		new_vidx = old_vidx;
		DeeDict_LockRead(dict);
		ASSERT(dict->d_vused <= dict->d_vsize);
		if (dict->d_vused == dict->d_vsize) {
			/* Dict is optimized -> can just increment "new_vidx" */
			if (OVERFLOW_UADD(new_vidx, skip, &new_vidx))
				new_vidx = (size_t)-1;
			if (new_vidx > Dee_hash_vidx_tovirt(dict->d_vsize))
				new_vidx = Dee_hash_vidx_tovirt(dict->d_vsize);
		} else {
			/* Only keys that haven't been deleted count. */
			size_t n_skip = skip;
			struct Dee_dict_item *virt_vtab = _DeeDict_GetVirtVTab(dict);
			for (; Dee_hash_vidx_virt_lt_real(new_vidx, dict->d_vsize) && n_skip; ++new_vidx) {
				if (virt_vtab[new_vidx].di_key)
					--n_skip;
			}
		}
		DeeDict_LockEndRead(dict);
		if (atomic_cmpxch_or_write(&self->di_vidx, old_vidx, new_vidx))
			break;
	}
	return new_vidx - old_vidx;
}

PRIVATE struct type_iterator diter_iterator = {
	/* .tp_nextpair  = */ (int (DCALL *)(DeeObject *__restrict, DREF DeeObject *[2]))&diter_nextpair,
	/* .tp_nextkey   = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&diter_nextkey,
	/* .tp_nextvalue = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&diter_nextvalue,
};

PRIVATE WUNUSED NONNULL((1)) int DCALL
diter_bool(DictIterator *__restrict self) {
	Dict *dict = self->di_dict;
	/*virt*/Dee_hash_vidx_t vidx = atomic_read(&self->di_vidx);
	struct Dee_dict_item *virt_vtab;
	ASSERT(vidx >= Dee_hash_vidx_tovirt(0));
	DeeDict_LockRead(dict);
	virt_vtab = _DeeDict_GetVirtVTab(dict);
	for (; Dee_hash_vidx_virt_lt_real(vidx, dict->d_vsize); ++vidx) {
		if (virt_vtab[vidx].di_key) {
			DeeDict_LockEndRead(dict);
			return 1;
		}
	}
	DeeDict_LockEndRead(dict);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) Dee_hash_t DCALL
diter_hash(DictIterator *self) {
	return atomic_read(&self->di_vidx);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
diter_compare(DictIterator *lhs, DictIterator *rhs) {
	if (DeeObject_AssertType(rhs, Dee_TYPE(lhs)))
		goto err;
	Dee_return_compareT(Dee_hash_vidx_t, atomic_read(&lhs->di_vidx),
	                    /*            */ atomic_read(&rhs->di_vidx));
err:
	return Dee_COMPARE_ERR;
}

INTERN struct type_cmp diter_cmp = { /* "INTERN" because shared with "./hashset.c" */
	/* .tp_hash       = */ (Dee_hash_t (DCALL *)(DeeObject *))&diter_hash,
	/* .tp_compare_eq = */ DEFIMPL(&default__compare_eq__with__compare),
	/* .tp_compare    = */ (int (DCALL *)(DeeObject *, DeeObject *))&diter_compare,
	/* .tp_trycompare_eq = */ DEFIMPL(&default__trycompare_eq__with__compare_eq),
	/* .tp_eq            = */ DEFIMPL(&default__eq__with__compare_eq),
	/* .tp_ne            = */ DEFIMPL(&default__ne__with__compare_eq),
	/* .tp_lo            = */ DEFIMPL(&default__lo__with__compare),
	/* .tp_le            = */ DEFIMPL(&default__le__with__compare),
	/* .tp_gr            = */ DEFIMPL(&default__gr__with__compare),
	/* .tp_ge            = */ DEFIMPL(&default__ge__with__compare),
};


INTDEF struct type_method tpconst diter_methods[]; /* "INTERN" because shared with "./hashset.c" */
INTERN_TPCONST struct type_method tpconst diter_methods[] = {
	TYPE_METHOD_HINTREF(Iterator_advance),
	TYPE_METHOD_END
};

PRIVATE struct type_method_hint tpconst diter_method_hints[] = {
	TYPE_METHOD_HINT(iter_advance, &diter_advance),
	TYPE_METHOD_HINT_END
};

PRIVATE struct type_member tpconst diter_members[] = {
	TYPE_MEMBER_FIELD_DOC(STR_seq, STRUCT_OBJECT_AB, offsetof(DictIterator, di_dict), "->?DDict"),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject DictIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_DictIterator",
	/* .tp_doc      = */ DOC("()\n"
	                         "(" diter__DictIterator_params ")\n"
	                         "\n"
	                         "next->?T2?O?O"),
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ DictIterator,
			/* tp_ctor:        */ &diter_ctor,
			/* tp_copy_ctor:   */ &diter_copy,
			/* tp_any_ctor:    */ &diter_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &diter_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&diter_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&diter_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&iterator_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&diter_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__EFED4BCD35433C3C),
	/* .tp_cmp           = */ &diter_cmp,
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ DEFIMPL(&default__iter_next__with__nextpair),
	/* .tp_iterator      = */ &diter_iterator,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ diter_methods,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ diter_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ diter_method_hints,
	/* .tp_call          = */ DEFIMPL(&iterator_next),
	/* .tp_callable      = */ DEFIMPL(&default__tp_callable__83C59FA7626CABBE),
};















/************************************************************************/
/* DICT                                                                 */
/************************************************************************/

PUBLIC DeeObject DeeDict_Dummy = { /* DEPRECATED */
	OBJECT_HEAD_INIT(&DeeObject_Type)
};

PUBLIC_CONST byte_t const _DeeHash_EmptyTab[] = { 0 };

/* Heap functions for allocating/freeing dict tables (d_vtab + d_htab) */
#define _DeeDict_TabsMalloc(n_bytes)          Dee_Malloc(n_bytes)
#define _DeeDict_TabsCalloc(n_bytes)          Dee_Calloc(n_bytes)
#define _DeeDict_TabsRealloc(ptr, n_bytes)    Dee_Realloc(ptr, n_bytes)
#define _DeeDict_TabsTryMalloc(n_bytes)       Dee_TryMalloc(n_bytes)
#define _DeeDict_TabsTryCalloc(n_bytes)       Dee_TryCalloc(n_bytes)
#define _DeeDict_TabsTryRealloc(ptr, n_bytes) Dee_TryRealloc(ptr, n_bytes)
#define _DeeDict_TabsFree(ptr)                Dee_Free(ptr)

#define NULL_IF__DeeDict_EmptyVTab_REAL(/*real*/ p) \
	((p) == (struct Dee_dict_item *)_DeeHash_EmptyTab ? NULL : (p))

#define HAVE_dict_setitem_unlocked
#ifdef __OPTIMIZE_SIZE__
#undef HAVE_dict_setitem_unlocked
#endif /* __OPTIMIZE_SIZE__ */


#if defined(DICT_NDEBUG) && 1
#define dict_verify(self) (void)0
#else /* DICT_NDEBUG */
PRIVATE ATTR_NOINLINE NONNULL((1)) void DCALL
dict_verify(Dict *__restrict self) {
	Dee_hash_vidx_t i, real_vused;
	Dee_hash_hidxio_t hidxio;
	struct Dee_hash_hidxio_ops const *ops;
	ASSERT(self->d_vused <= self->d_vsize);
	ASSERT(self->d_vsize <= self->d_valloc);
	ASSERT(self->d_valloc <= self->d_hmask);
	ASSERT(IS_POWER_OF_TWO(self->d_hmask + 1));
	ASSERT(self->d_htab == (union Dee_hash_htab *)(_DeeDict_GetRealVTab(self) + self->d_valloc));
	hidxio = Dee_HASH_HIDXIO_FROM_VALLOC(self->d_valloc);
	ASSERT(/*hidxio >= 0 &&*/ hidxio < Dee_HASH_HIDXIO_COUNT);
	ops = self->d_hidxops;
	ASSERT(ops == &Dee_hash_hidxio[hidxio]);
	for (i = Dee_hash_vidx_tovirt(0), real_vused = 0;
	     Dee_hash_vidx_virt_lt_real(i, self->d_vsize); ++i) {
		struct Dee_dict_item *item = &_DeeDict_GetVirtVTab(self)[i];
		if (item->di_key) {
#if 0 /* Cannot be asserted -- we might get here from "tp_visit", which can screw with reference counts */
			ASSERT_OBJECT(item->di_key);
			ASSERT_OBJECT(item->di_value);
#endif
			++real_vused;
		}
	}
	ASSERTF(real_vused == self->d_vused,
	        "vtab key count=%" PRFuSIZ " differs from d_vused=%" PRFuSIZ,
	        real_vused, self->d_vused);
	for (i = 0; i <= self->d_hmask; ++i) {
		Dee_hash_vidx_t vidx;
		vidx = (*ops->hxio_get)(self->d_htab, i);
		if (vidx == Dee_HASH_HTAB_EOF)
			continue;
		Dee_hash_vidx_virt2real(&vidx);
		ASSERTF(vidx < self->d_vsize,
		        "htab[%" PRFuSIZ "] points out-of-bounds: %" PRFuSIZ " >= %" PRFuSIZ,
		        i, vidx, self->d_vsize);
	}
	for (i = 0;; ++i) {
		Dee_hash_vidx_t vtab_idx;
		ASSERTF(i <= self->d_hmask, "htab contains no EOF pointers (infinite loop would occur on non-present item lookup)");
		vtab_idx = (*ops->hxio_get)(self->d_htab, i);
		if (vtab_idx == Dee_HASH_HTAB_EOF)
			break;
	}
	for (i = Dee_hash_vidx_tovirt(0), real_vused = 0;
	     Dee_hash_vidx_virt_lt_real(i, self->d_vsize); ++i) {
		Dee_hash_t hs, perturb;
		struct Dee_dict_item *item = &_DeeDict_GetVirtVTab(self)[i];
		if (!item->di_key)
			continue;
		for (_DeeDict_HashIdxInit(self, &hs, &perturb, item->di_hash);;
		     _DeeDict_HashIdxNext(self, &hs, &perturb, item->di_hash)) {
			struct Dee_dict_item *hitem;
			Dee_hash_hidx_t htab_idx = Dee_hash_hidx_ofhash(hs, self->d_hmask);
			Dee_hash_vidx_t vtab_idx = (*ops->hxio_get)(self->d_htab, htab_idx); /*virt*/
			ASSERTF(vtab_idx != Dee_HASH_HTAB_EOF,
			        "End-of-hash-chain[hash:%#" PRFxSIZ "] before item idx=%" PRFuSIZ ",count=%" PRFuSIZ " <%r:%r> was found",
			        item->di_hash, Dee_hash_vidx_toreal(i), self->d_vsize,
			        item->di_key, item->di_value);
			hitem = &_DeeDict_GetVirtVTab(self)[vtab_idx];
			if (hitem == item)
				break;
		}
	}
}

#if 1
#undef DeeDict_LockEndWrite
#undef DeeDict_LockEndRead
#undef DeeDict_LockEnd
#define DeeDict_LockEndWrite(self)   (dict_verify(self), Dee_atomic_rwlock_endwrite(&(self)->d_lock))
#define DeeDict_LockEndRead(self)    (dict_verify(self), Dee_atomic_rwlock_endread(&(self)->d_lock))
#define DeeDict_LockEnd(self)        (dict_verify(self), Dee_atomic_rwlock_end(&(self)->d_lock))
#endif
#endif /* !DICT_NDEBUG */

#define dict_htab_decafter(self, /*virt*/ vtab_threshold) \
	(*(self)->d_hidxops->hxio_decafter)((self)->d_htab, (self)->d_hmask, vtab_threshold)
#define dict_htab_incafter(self, /*virt*/ vtab_threshold) \
	(*(self)->d_hidxops->hxio_incafter)((self)->d_htab, (self)->d_hmask, vtab_threshold)
#define dict_htab_decrange(self, /*virt*/ vtab_min, /*virt*/ vtab_max) \
	(*(self)->d_hidxops->hxio_decrange)((self)->d_htab, (self)->d_hmask, vtab_min, vtab_max)
#define dict_htab_incrange(self, /*virt*/ vtab_min, /*virt*/ vtab_max) \
	(*(self)->d_hidxops->hxio_incrange)((self)->d_htab, (self)->d_hmask, vtab_min, vtab_max)
#define dict_htab_reverse(self, /*virt*/ vmin, /*virt*/ vmax) \
	(*(self)->d_hidxops->hxio_revrange)((self)->d_htab, (self)->d_hmask, vmin, vmax)


#ifdef __INTELLISENSE__

/************************************************************************/
/************************************************************************/
/* LOW-LEVEL API                                                        */
/************************************************************************/
/************************************************************************/

PRIVATE WUNUSED DREF Dict *DCALL
dict_new_with_hint(size_t num_items, bool tryalloc, bool allow_overalloc);

/* Re-build the dict's "d_htab" (allowed to assume that "d_vtab" does not contain deleted keys) */
PRIVATE ATTR_NOINLINE NONNULL((1)) void DCALL
dict_htab_rebuild_after_optimize(Dict *__restrict self);

PRIVATE ATTR_NOINLINE NONNULL((1)) void DCALL
dict_do_optimize_vtab_without_rebuild(Dict *__restrict self);

INTDEF NONNULL((1)) void DCALL
dict_optimize_vtab(Dict *__restrict self);

PRIVATE NONNULL((1)) void DCALL
dict_htab_rebuild(Dict *__restrict self);

/* Try to make it so "d_vsize < d_valloc" by enlarging the vector.
 * Do this while the caller is holding a write-lock to "self", and
 * do so without ever releasing that lock.
 * NOTES:
 * - This function will NEVER rehash the dict or change the contents of d_htab!
 * - The caller must ensure that `_DeeDict_CanGrowVTab(self)' is true
 * @return: true:  Success
 * @return: false: Failure */
PRIVATE ATTR_NOINLINE WUNUSED NONNULL((1)) bool DCALL
dict_trygrow_vtab(Dict *__restrict self);

/* Same as `dict_trygrow_vtab()', but allowed to grow the htab
 * also, and can be used even when "!_DeeDict_CanGrowVTab(self)"
 * Tries to make it so "d_valloc >= min_valloc"
 * @return: true:  Success: "d_valloc >= min_valloc"
 * @return: false: Failure */
PRIVATE ATTR_NOINLINE NONNULL((1)) bool DCALL
dict_trygrow_vtab_and_htab_with(Dict *__restrict self,
                                Dee_hash_vidx_t min_valloc,
                                bool allow_overalloc);

/* Same as `dict_trygrow_vtab()', but allowed to grow the htab
 * also, and can be used even when "!_DeeDict_CanGrowVTab(self)"
 * @return: true:  Success
 * @return: false: Failure */
PRIVATE WUNUSED NONNULL((1)) bool DCALL
dict_trygrow_vtab_and_htab(Dict *__restrict self);

#if 0
/* Try to change "d_hmask = (d_hmask << 1) | 1",
 * and (if we want to), also increase "d_valloc"
 * @return: true:  Success
 * @return: false: Failure (allocation failed) */
PRIVATE ATTR_NOINLINE WUNUSED NONNULL((1)) bool DCALL
dict_trygrow_htab_and_maybe_vtab(Dict *__restrict self);
#endif

/* Make it so "!_DeeDict_MustGrowVTab(self)"
 * (aka: " d_vsize < d_valloc && d_valloc <= d_hmask")
 * Allowed to release+re-acquire a write-lock to "self".
 * @return: 0 : Success (lock was lost and later re-acquired)
 * @return: -1: Failure (lock was lost and an error was thrown) */
#ifdef HAVE_dict_setitem_unlocked
PRIVATE ATTR_NOINLINE WUNUSED NONNULL((1)) int DCALL
dict_grow_vtab_and_htab_and_relock(Dict *__restrict self, bool without_locks);
#else /* HAVE_dict_setitem_unlocked */
PRIVATE ATTR_NOINLINE WUNUSED NONNULL((1)) int DCALL
dict_grow_vtab_and_htab_and_relock_impl(Dict *__restrict self);
#define dict_grow_vtab_and_htab_and_relock(self, without_locks) \
	dict_grow_vtab_and_htab_and_relock_impl(self)
#endif /* !HAVE_dict_setitem_unlocked */

#if 0
/* Make it so "!_DeeDict_MustGrowHTab(self)".
 * Allowed to release+re-acquire a write-lock to "self".
 * @return: 0 : Success (lock was lost and later re-acquired)
 * @return: -1: Failure (lock was lost and an error was thrown) */
PRIVATE ATTR_NOINLINE WUNUSED NONNULL((1)) int DCALL
dict_grow_htab_and_relock(Dict *__restrict self);
#endif

/* Shrink the vtab and release a lock to "self". Must be called when:
 * - holding a write-lock
 * - _DeeDict_CanShrinkHTab(self) is true
 * - _DeeDict_ShouldShrinkHTab(self) is true (or `fully_shrink=true')
 * NOTE: After a call to this function, the caller must always rebuild the htab! */
PRIVATE NONNULL((1)) void DCALL
dict_shrink_htab(Dict *__restrict self, bool fully_shrink);

/* Shrink the vtab+htab. Must be called while:
 * - holding a write-lock
 * - _DeeDict_CanShrinkVTab(self) is true
 * - _DeeDict_ShouldShrinkVTab(self) is true (or `fully_shrink=true') */
PRIVATE ATTR_NOINLINE NONNULL((1)) void DCALL
dict_shrink_vtab_and_htab(Dict *__restrict self, bool fully_shrink);

/* Automatically shrink allocated tables of "self" if appropriate.
 * Call this function are removing elements from "self"
 * Same as the API function "dict.shrink(fully: false)" */
LOCAL NONNULL((1)) void DCALL
dict_autoshrink(Dict *__restrict self);

PRIVATE ATTR_NOINLINE NONNULL((1)) void DCALL
dict_makespace_at_impl(Dict *__restrict self, /*real*/ Dee_hash_vidx_t vtab_idx);

LOCAL NONNULL((1)) void DCALL
dict_makespace_at(Dict *__restrict self, /*real*/ Dee_hash_vidx_t vtab_idx);

#else /* __INTELLISENSE__ */
DECL_END
#define DEFINE_DeeDict
#define DEFINE_LOW_LEVEL
#include "dict-impl.c.inl"
DECL_BEGIN
#endif /* !__INTELLISENSE__ */


/************************************************************************/
/* HIGH-LEVEL API                                                       */
/************************************************************************/
PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL dict_getitem(Dict *self, DeeObject *key);
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL dict_getitem_index(Dict *self, size_t index);
PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL dict_getitem_string_hash(Dict *self, char const *key, Dee_hash_t hash);
PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL dict_getitem_string_len_hash(Dict *self, char const *key, size_t keylen, Dee_hash_t hash);

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL dict_trygetitem(Dict *self, DeeObject *key);
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL dict_trygetitem_index(Dict *self, size_t index);
PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL dict_trygetitem_string_hash(Dict *self, char const *key, Dee_hash_t hash);
PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL dict_trygetitem_string_len_hash(Dict *self, char const *key, size_t keylen, Dee_hash_t hash);

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL dict_bounditem(Dict *self, DeeObject *key);
PRIVATE WUNUSED NONNULL((1)) int DCALL dict_bounditem_index(Dict *self, size_t index);
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL dict_bounditem_string_hash(Dict *self, char const *key, Dee_hash_t hash);
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL dict_bounditem_string_len_hash(Dict *self, char const *key, size_t keylen, Dee_hash_t hash);

#define dict_hasitem                 dict_bounditem
#define dict_hasitem_index           dict_bounditem_index
#define dict_hasitem_string_hash     dict_bounditem_string_hash
#define dict_hasitem_string_len_hash dict_bounditem_string_len_hash

PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL dict_setitem(Dict *self, DeeObject *key, DeeObject *value);
PRIVATE WUNUSED NONNULL((1, 3)) int DCALL dict_setitem_index(Dict *self, size_t index, DeeObject *value);
PRIVATE WUNUSED NONNULL((1, 2, 4)) int DCALL dict_setitem_string_hash(Dict *self, char const *key, Dee_hash_t hash, DeeObject *value);
PRIVATE WUNUSED NONNULL((1, 2, 5)) int DCALL dict_setitem_string_len_hash(Dict *self, char const *key, size_t keylen, Dee_hash_t hash, DeeObject *value);
PRIVATE WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL dict_mh_setold_ex(Dict *self, DeeObject *key, DeeObject *value);
PRIVATE WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL dict_mh_setnew_ex(Dict *self, DeeObject *key, DeeObject *value);
PRIVATE WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL dict_mh_setdefault(Dict *self, DeeObject *key, DeeObject *value);
/* @param: getindex: returns the unoptimized (iow: including deleted keys) index
 *                   in "d_vtab" where the new key-value pair should be inserted.
 *                   All of the other functions above simply append at the end of
 *                   "d_vtab", which is the same as this callback returning "d_vsize"
 *                   @param: overwrite_index: When "key" already exists, the index of
 *                                            the item that will be deleted. Else, set
 *                                            to `Dee_HASH_HTAB_EOF' when "key" is new.
 *                   @param: p_value: Pointer to the value that will be written to the key
 *                                    May be changed here to some other value already within
 *                                    the dict, in order to allow atomically assigning some
 *                                    value already found within the dict.
 *                   - To throw an error, the callback should:
 *                     >> DeeDict_LockEndWrite(self)
 *                     >> DeeError_Throw(...);
 *                     >> return Dee_HASH_HTAB_EOF; */
PRIVATE WUNUSED NONNULL((1, 2, 3, 4)) int DCALL dict_setitem_at(Dict *self, DeeObject *key, DeeObject *value, /*virt*/ Dee_hash_vidx_t (DCALL *getindex)(void *cookie, Dict *self, /*virt*/ Dee_hash_vidx_t overwrite_index, DeeObject **p_value), void *getindex_cookie);
#ifndef HAVE_dict_setitem_unlocked
#define dict_setitem_unlocked dict_setitem
#else /* !HAVE_dict_setitem_unlocked */
PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL dict_setitem_unlocked(Dict *self, DeeObject *key, DeeObject *value);
PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL dict_setitem_unlocked_fast_inherited(Dict *self, /*inherit(on_success)*/ DREF DeeObject *key, /*inherit(on_success)*/ DREF DeeObject *value);
#define HAVE_dict_setitem_unlocked_fast_inherited
#endif /* HAVE_dict_setitem_unlocked */

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL dict_delitem(Dict *self, DeeObject *key);
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL dict_delitem_string_hash(Dict *self, char const *key, Dee_hash_t hash);
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL dict_delitem_string_len_hash(Dict *self, char const *key, size_t keylen, Dee_hash_t hash);
PRIVATE WUNUSED NONNULL((1)) int DCALL dict_delitem_index(Dict *self, size_t index);
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL dict_mh_remove(Dict *self, DeeObject *key);
PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL dict_mh_pop(Dict *self, DeeObject *key);
PRIVATE WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL dict_mh_pop_with_default(Dict *self, DeeObject *key, DeeObject *def);
#ifdef __OPTIMIZE_SIZE__
PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL dict_popvalue(Dict *self, DeeObject *key);
#endif /* __OPTIMIZE_SIZE__ */



#ifndef __INTELLISENSE__
DECL_END
#define DEFINE_dict_trygetitem
#include "dict-getitem-impl.c.inl"
#define DEFINE_dict_trygetitem_index
#include "dict-getitem-impl.c.inl"
#define DEFINE_dict_trygetitem_string_hash
#include "dict-getitem-impl.c.inl"
#define DEFINE_dict_trygetitem_string_len_hash
#include "dict-getitem-impl.c.inl"

#ifndef __OPTIMIZE_SIZE__
#define DEFINE_dict_getitem
#include "dict-getitem-impl.c.inl"
#define DEFINE_dict_getitem_index
#include "dict-getitem-impl.c.inl"
#define DEFINE_dict_getitem_string_hash
#include "dict-getitem-impl.c.inl"
#define DEFINE_dict_getitem_string_len_hash
#include "dict-getitem-impl.c.inl"

#define DEFINE_dict_bounditem
#include "dict-getitem-impl.c.inl"
#define DEFINE_dict_bounditem_index
#include "dict-getitem-impl.c.inl"
#define DEFINE_dict_bounditem_string_hash
#include "dict-getitem-impl.c.inl"
#define DEFINE_dict_bounditem_string_len_hash
#include "dict-getitem-impl.c.inl"
#endif /* !__OPTIMIZE_SIZE__ */

#define DEFINE_dict_setitem
#include "dict-setitem-impl.c.inl"
#define DEFINE_dict_setitem_at
#include "dict-setitem-impl.c.inl"
#define DEFINE_dict_setitem_string_hash
#include "dict-setitem-impl.c.inl"
#define DEFINE_dict_setitem_string_len_hash
#include "dict-setitem-impl.c.inl"
#define DEFINE_dict_setitem_index
#include "dict-setitem-impl.c.inl"
#ifdef HAVE_dict_setitem_unlocked
#define DEFINE_dict_setitem_unlocked
#include "dict-setitem-impl.c.inl"
#define DEFINE_dict_setitem_unlocked_fast_inherited
#endif /* HAVE_dict_setitem_unlocked */
#ifndef __OPTIMIZE_SIZE__
#include "dict-setitem-impl.c.inl"
#define DEFINE_dict_mh_setdefault
#include "dict-setitem-impl.c.inl"
#endif /* !__OPTIMIZE_SIZE__ */
#define DEFINE_dict_mh_setold_ex
#include "dict-setitem-impl.c.inl"
#define DEFINE_dict_mh_setnew_ex
#include "dict-setitem-impl.c.inl"

#define DEFINE_dict_delitem_string_hash
#include "dict-delitem-impl.c.inl"
#define DEFINE_dict_delitem_string_len_hash
#include "dict-delitem-impl.c.inl"
#define DEFINE_dict_delitem_index
#include "dict-delitem-impl.c.inl"
#ifdef __OPTIMIZE_SIZE__
#define DEFINE_dict_popvalue
#include "dict-delitem-impl.c.inl"
#else /* __OPTIMIZE_SIZE__ */
#define DEFINE_dict_delitem
#include "dict-delitem-impl.c.inl"
#define DEFINE_dict_mh_remove
#include "dict-delitem-impl.c.inl"
#define DEFINE_dict_mh_pop
#include "dict-delitem-impl.c.inl"
#define DEFINE_dict_mh_pop_with_default
#include "dict-delitem-impl.c.inl"
#endif /* !__OPTIMIZE_SIZE__ */
DECL_BEGIN
#endif /* !__INTELLISENSE__ */

#ifdef __OPTIMIZE_SIZE__
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
dict_mh_remove(Dict *self, DeeObject *key) {
	DREF DeeObject *value = dict_popvalue(self, key);
	if unlikely(!value)
		return -1;
	if (value == ITER_DONE)
		return 0;
	Dee_Decref(value);
	return 1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
dict_delitem(Dict *self, DeeObject *key) {
	DREF DeeObject *value = dict_popvalue(self, key);
	if unlikely(!value)
		return -1;
	if (value != ITER_DONE)
		Dee_Decref(value);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
dict_mh_pop(Dict *self, DeeObject *key) {
	DREF DeeObject *result = dict_popvalue(self, key);
	if unlikely(result == ITER_DONE) {
		DeeRT_ErrUnknownKey(self, key);
		result = NULL;
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
dict_mh_pop_with_default(Dict *self, DeeObject *key, DeeObject *def) {
	DREF DeeObject *result = dict_popvalue(self, key);
	if unlikely(result == ITER_DONE) {
		Dee_Incref(def);
		result = def;
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
dict_getitem(Dict *self, DeeObject *key) {
	DREF DeeObject *result = dict_trygetitem(self, DeeObject key);
	if unlikely(result == ITER_DONE) {
		DeeRT_ErrUnknownKey(self, key);
		result = NULL;
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
dict_getitem_index(Dict *self, size_t index) {
	DREF DeeObject *result = dict_trygetitem_index(self, index);
	if unlikely(result == ITER_DONE) {
		DeeRT_ErrUnknownKeyInt(Dee_AsObject(self), index);
		result = NULL;
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
dict_getitem_string_hash(Dict *self, char const *key, Dee_hash_t hash) {
	DREF DeeObject *result = dict_trygetitem_string_hash(self, key, hash);
	if unlikely(result == ITER_DONE) {
		DeeRT_ErrUnboundKeyStr(Dee_AsObject(self), key);
		result = NULL;
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
dict_getitem_string_len_hash(Dict *self, char const *key, size_t keylen, Dee_hash_t hash) {
	DREF DeeObject *result = dict_trygetitem_string_len_hash(self, key, size_t keylen, hash);
	if unlikely(result == ITER_DONE) {
		DeeRT_ErrUnknownKeyStrLen(Dee_AsObject(self), key, keylen);
		result = NULL;
	}
	return result;
}

PRIVATE WUNUSED int DCALL
bound_from_trygetitem(DREF DeeObject *value) {
	if unlikely(!value)
		return Dee_BOUND_ERR;
	if (value == ITER_DONE)
		return Dee_BOUND_MISSING;
	Dee_Decref(value);
	return Dee_BOUND_YES;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
dict_bounditem(Dict *self, DeeObject *key) {
	return bound_from_trygetitem(dict_trygetitem(self, key));
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
dict_bounditem_index(Dict *self, size_t index) {
	return bound_from_trygetitem(dict_trygetitem_index(self, index));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
dict_bounditem_string_hash(Dict *self, char const *key, Dee_hash_t hash) {
	return bound_from_trygetitem(dict_trygetitem_string_hash(self, key, hash));
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
dict_bounditem_string_len_hash(Dict *self, char const *key, size_t keylen, Dee_hash_t hash) {
	return bound_from_trygetitem(dict_trygetitem_string_len_hash(self, key, keylen, hash));
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
dict_mh_setdefault(Dict *self, DeeObject *key, DeeObject *value) {
	DREF DeeObject *result = dict_mh_setnew_ex(self, key, value);
	if (result == ITER_DONE) {
		result = value;
		Dee_Incref(result);
	}
	return result;
}
#endif /* __OPTIMIZE_SIZE__ */


#if __SIZEOF_INT__ == __SIZEOF_SIZE_T__
#define dict_fromsequence_foreach_cb (*(Dee_ssize_t (DCALL *)(void *, DeeObject *, DeeObject *))&dict_setitem_unlocked)
#else /* __SIZEOF_INT__ == __SIZEOF_SIZE_T__ */
PRIVATE WUNUSED NONNULL((2, 3)) Dee_ssize_t DCALL
dict_fromsequence_foreach_cb(void *self, DeeObject *key, DeeObject *value) {
	return dict_setitem_unlocked((Dict *)self, key, value);
}
#endif /* __SIZEOF_INT__ != __SIZEOF_SIZE_T__ */

#undef DICT_INITFROM_NEEDSLOCK
#ifndef HAVE_dict_setitem_unlocked /* Because of "#define dict_setitem_unlocked dict_setitem" */
#define DICT_INITFROM_NEEDSLOCK
#endif /* !HAVE_dict_setitem_unlocked */

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
dict_init_fromcopy(Dict *__restrict self, Dict *__restrict other);
#ifndef __OPTIMIZE_SIZE__
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
dict_init_fromcopy_keysonly(Dict *__restrict self, Dict *__restrict other);
#define HAVE_dict_init_fromcopy_keysonly
#endif /* !__OPTIMIZE_SIZE__ */
#ifdef CONFIG_EXPERIMENTAL_ORDERED_HASHSET
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
dict_init_fromhashset_keysonly(Dict *__restrict self, DeeHashSetObject *__restrict other);
#endif /* CONFIG_EXPERIMENTAL_ORDERED_HASHSET */

#ifndef __INTELLISENSE__
DECL_END
#define DEFINE_dict_init_fromcopy
#include "dict-init-copyfrom.c.inl"
#ifndef __OPTIMIZE_SIZE__
#define DEFINE_dict_init_fromcopy_keysonly
#include "dict-init-copyfrom.c.inl"
#endif /* !__OPTIMIZE_SIZE__ */
#ifdef CONFIG_EXPERIMENTAL_ORDERED_HASHSET
#define DEFINE_dict_init_fromhashset_keysonly
#include "dict-init-copyfrom.c.inl"
#endif /* CONFIG_EXPERIMENTAL_ORDERED_HASHSET */
DECL_BEGIN
#endif /* !__INTELLISENSE__ */


#ifdef __INTELLISENSE__
/************************************************************************/
/************************************************************************/
/* HIGH-LEVEL API                                                       */
/************************************************************************/
/************************************************************************/

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL dict_init_fromrodict_noincref(Dict *__restrict self, DeeRoDictObject *__restrict other);
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL dict_init_fromrodict(Dict *__restrict self, DeeRoDictObject *__restrict other);
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL dict_init_fromrodict_keysonly(Dict *__restrict self, DeeRoDictObject *__restrict other);
#ifdef CONFIG_EXPERIMENTAL_ORDERED_HASHSET
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL dict_init_fromroset_noincref(Dict *__restrict self, DeeRoSetObject *__restrict other);
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL dict_init_fromroset_keysonly(Dict *__restrict self, DeeRoSetObject *__restrict other);
#endif /* CONFIG_EXPERIMENTAL_ORDERED_HASHSET */
PRIVATE WUNUSED NONNULL((1)) DREF Dict *DCALL dict_new_copy(Dict *__restrict self);
DFUNDEF WUNUSED NONNULL((1)) DREF /*Dict*/ DeeObject *DCALL DeeDict_FromSequence(DeeObject *__restrict self);
DFUNDEF WUNUSED NONNULL((1)) DREF /*Dict*/ DeeObject *DCALL DeeDict_FromSequenceInheritedOnSuccess(/*inherit(on_success)*/ DREF DeeObject *__restrict self);
DFUNDEF WUNUSED NONNULL((1)) DREF /*Dict*/ DeeObject *DCALL DeeDict_FromRoDict(/*RoDict*/ DeeObject *__restrict self);
DFUNDEF WUNUSED DREF DeeObject *DCALL DeeDict_NewKeyValuesInherited(size_t num_items, /*inherit(on_success)*/ DREF DeeObject **key_values);
PRIVATE ATTR_NOINLINE NONNULL((1)) void DCALL dict_fini(Dict *__restrict self);
PRIVATE ATTR_NOINLINE NONNULL((1, 2)) void DCALL dict_visit(Dict *__restrict self, Dee_visit_t proc, void *arg);
LOCAL NONNULL((1)) void DCALL dict_initfrom_empty(Dict *__restrict self);
LOCAL NONNULL((1)) void DCALL dict_initfrom_hint(Dict *__restrict self, size_t num_items, bool allow_overalloc);
LOCAL WUNUSED NONNULL((1)) int DCALL dict_initfrom_seq(Dict *__restrict self, DeeObject *seq);
PRIVATE WUNUSED NONNULL((1)) int DCALL dict_assign(Dict *self, DeeObject *seq);
PRIVATE WUNUSED NONNULL((1)) int DCALL dict_moveassign(Dict *self, Dict *other);
PRIVATE WUNUSED NONNULL((1)) int DCALL dict_ctor(Dict *__restrict self);
PRIVATE WUNUSED NONNULL((1)) int DCALL dict_copy(Dict *__restrict self, Dict *__restrict other);
PRIVATE WUNUSED NONNULL((1)) int DCALL dict_init(Dict *__restrict self, size_t argc, DeeObject *const *argv);
PRIVATE WUNUSED NONNULL((1)) int DCALL dict_serialize(Dict *__restrict self, DeeSerial *__restrict writer, Dee_seraddr_t addr);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL dict_printrepr(Dict *__restrict self, Dee_formatprinter_t printer, void *arg);
PRIVATE NONNULL((1)) int DCALL dict_mh_clear(Dict *__restrict self);

PRIVATE WUNUSED NONNULL((1)) int DCALL dict_mh_seq_erase(Dict *__restrict self, size_t start, size_t count);

PRIVATE WUNUSED NONNULL((1)) int DCALL dict_mh_seq_reverse(Dict *self, size_t start, size_t end);
PRIVATE NONNULL((1)) bool DCALL dict_shrink_impl(Dict *__restrict self, bool fully_shrink);
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL dict_shrink(Dict *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL dict_reserve(Dict *__restrict self, size_t argc, DeeObject *const *argv, DeeObject *kw);
PRIVATE WUNUSED NONNULL((1)) bool DCALL dict_cc(Dict *__restrict self);

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL dict_sizeof(Dict *__restrict self);
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL dict_mh_seq_compare(Dict *lhs, DeeObject *rhs);
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL dict_mh_seq_compare_eq(Dict *lhs, DeeObject *rhs);
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL dict_mh_seq_trycompare_eq(Dict *lhs, DeeObject *rhs);
#else /* __INTELLISENSE__ */
DECL_END
#define DEFINE_HIGH_LEVEL
#define DEFINE_DeeDict
#include "dict-impl.c.inl"
DECL_BEGIN
#endif /* !__INTELLISENSE__ */


#ifdef DCALL_RETURN_COMMON
#define dict_clear (*(void (DCALL *)(Dict *__restrict))&dict_mh_clear)
#else /* DCALL_RETURN_COMMON */
PRIVATE NONNULL((1)) void DCALL
dict_clear(Dict *__restrict self) {
	dict_mh_clear(self);
}
#endif /* !DCALL_RETURN_COMMON */


INTERN WUNUSED NONNULL((1)) int DCALL /* INTERN because shared with "./hashset.c" */
dict_bool(DeeDictObject *__restrict self) {
	return DeeDict_SIZE_ATOMIC(self) != 0;
}

#define dict_size_fast dict_size
INTERN WUNUSED NONNULL((1)) size_t DCALL /* INTERN because shared with "./hashset.c" */
dict_size(DeeDictObject *__restrict self) {
	return DeeDict_SIZE_ATOMIC(self);
}

PRIVATE WUNUSED NONNULL((1)) DREF DictIterator *DCALL
dict_iter(Dict *__restrict self) {
	DREF DictIterator *result = DeeObject_MALLOC(DictIterator);
	if unlikely(!result)
		goto err;
	Dee_Incref(self);
	result->di_dict = self;
	result->di_vidx = Dee_hash_vidx_tovirt(0);
	DeeObject_InitStatic(result, &DictIterator_Type);
	return result;
err:
	return NULL;
}



PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
dict_foreach_pair(Dict *__restrict self, Dee_foreach_pair_t cb, void *arg) {
	Dee_hash_vidx_t i;
	Dee_ssize_t temp, result = 0;
	DeeDict_LockRead(self);
	for (i = Dee_hash_vidx_tovirt(0);
	     Dee_hash_vidx_virt_lt_real(i, self->d_vsize); ++i) {
		DREF DeeObject *key, *value;
		struct Dee_dict_item *item;
		item = &_DeeDict_GetVirtVTab(self)[i];
		key  = item->di_key;
		if unlikely(!key)
			continue; /* Deleted, and not-yet-optimized key */
		Dee_Incref(key);
		value = item->di_value;
		Dee_Incref(value);
		DeeDict_LockEndRead(self);
		temp = (*cb)(arg, key, value);
		Dee_Decref_unlikely(value);
		Dee_Decref_unlikely(key);
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
		DeeDict_LockRead(self);
	}
	DeeDict_LockEndRead(self);
	return result;
err_temp:
	return temp;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
dict_mh_popitem(Dict *__restrict self) {
	DREF DeeSeqPairObject *result;
	struct Dee_dict_item *item;
	/* Allocate a tuple which we're going to fill with some key-value pair. */
	result = DeeSeq_NewPairUninitialized();
	if unlikely(!result)
		goto err;
	DeeDict_LockWrite(self);
	if unlikely(!self->d_vused) {
		DeeDict_LockEndWrite(self);
		DeeSeq_FreePairUninitialized(result);
		return_none;
	}
	item = _DeeDict_GetRealVTab(self) + self->d_vsize - 1;
	while (!item->di_key) {
		ASSERT(_DeeDict_CanOptimizeVTab(self));
		ASSERT(item > _DeeDict_GetRealVTab(self));
		--item;
	}
	DeeSeq_InitPairvInherited(result, item->di_key_and_value);
	item->di_key = NULL;
	DBG_memset(&item->di_value, 0xcc, sizeof(item->di_value));
	--self->d_vused;
	dict_autoshrink(self);
	DeeDict_LockEndWrite(self);
	return Dee_AsObject(result);
err:
	return NULL;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeSeqPairObject *DCALL
dict_mh_seq_getitem_index_impl(Dict *__restrict self, size_t index, bool tryget) {
	struct Dee_dict_item *item;
	DREF DeeSeqPairObject *result;
	result = DeeSeq_NewPairUninitialized();
	if unlikely(!result)
		goto err;
	DeeDict_LockReadAndOptimize(self);
	if unlikely(index >= self->d_vused) {
		size_t real_size = self->d_vused;
		DeeDict_LockEndRead(self);
		DeeSeq_FreePairUninitialized(result);
		if (tryget)
			return (DREF DeeSeqPairObject *)ITER_DONE;
		DeeRT_ErrIndexOutOfBounds(Dee_AsObject(self), index, real_size);
		goto err;
	}
	item = &_DeeDict_GetRealVTab(self)[index];
	DeeSeq_InitPairv(result, item->di_key_and_value);
	DeeDict_LockEndRead(self);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
dict_mh_seq_delitem_index(Dict *__restrict self, size_t index) {
	struct Dee_dict_item *item;
	DREF DeeObject *old_key, *old_value;
	DeeDict_LockWrite(self);
	if unlikely(index >= self->d_vused) {
		size_t real_size = self->d_vused;
		DeeDict_LockEndWrite(self);
		DeeRT_ErrIndexOutOfBounds(Dee_AsObject(self), index, real_size);
		goto err;
	}
	if (_DeeDict_CanOptimizeVTab(self))
		dict_optimize_vtab(self);
	item = &_DeeDict_GetRealVTab(self)[index];
	old_key   = item->di_key;
	old_value = item->di_value;
	item->di_key = NULL; /* Delete item */
	DBG_memset(&item->di_value, 0xcc, sizeof(item->di_value));
	--self->d_vused;
	dict_autoshrink(self);
	DeeDict_LockEndWrite(self);
	Dee_Decref(old_key);
	Dee_Decref(old_value);
	return 0;
err:
	return -1;
}

/* Given an "index" in range `[0,d_used)', return a value
 * in range `[0,d_size)' that points to the index'th non-
 * deleted key in "d_vtab" */
PRIVATE WUNUSED NONNULL((1)) /*real*/ Dee_hash_vidx_t DCALL
dict_unoptimize_vtab_index(Dict *self, size_t index) {
	size_t result;
	ASSERT(self->d_vused <= self->d_vsize);
	ASSERT(index <= self->d_vused);
	if (self->d_vused == self->d_vsize)
		return index; /* Dict is fully optimized -> no translation needed */
	if (index <= 0)
		return 0;
	if (index >= self->d_vused)
		return self->d_vsize;
	for (result = 0; index; ++result) {
		ASSERT(result <= self->d_vsize);
		if (_DeeDict_GetRealVTab(self)[result].di_key)
			--index;
	}
	return result;
}

struct dict_mh_seq_setitem_index_impl_data {
	size_t          dsqsii_index;         /* in */
	DREF DeeObject *dsqsii_deleted_key;   /* out[1..1] */
	DREF DeeObject *dsqsii_deleted_value; /* out[1..1] */
};

PRIVATE WUNUSED NONNULL((1)) /*virt*/ Dee_hash_vidx_t DCALL
dict_mh_seq_setitem_index_impl_cb(void *arg, Dict *self,
                                  /*virt*/ Dee_hash_vidx_t overwrite_index,
                                  DeeObject **p_value) {
	/*real*/ Dee_hash_vidx_t result;
	struct dict_mh_seq_setitem_index_impl_data *data;
	struct Dee_dict_item *real_vtab;
	(void)p_value;
	data = (struct dict_mh_seq_setitem_index_impl_data *)arg;
	if unlikely(data->dsqsii_index >= self->d_vused) {
		size_t used = self->d_vused;
		DeeDict_LockEndWrite(self);
		DeeRT_ErrIndexOutOfBounds(Dee_AsObject(self), data->dsqsii_index, used);
		return Dee_HASH_HTAB_EOF;
	}
	result = dict_unoptimize_vtab_index(self, data->dsqsii_index);
	real_vtab = _DeeDict_GetRealVTab(self);
	for (;;) {
		ASSERT(result < self->d_vsize);
		if (real_vtab[result].di_key)
			break;
		++result;
	}
	ASSERT(real_vtab[result].di_key);
	ASSERT(real_vtab[result].di_value);
	if (result != Dee_hash_vidx_toreal(overwrite_index)) {
		/* Delete item that used to exist at this index (sequence setitem acts as an overwrite operation) */
		data->dsqsii_deleted_key   = real_vtab[result].di_key;
		data->dsqsii_deleted_value = real_vtab[result].di_value;
		real_vtab[result].di_key   = NULL;
		DBG_memset(&real_vtab[result].di_value, 0xcc, sizeof(real_vtab[result].di_value));
		--self->d_vused;
	} else {
		/* for xchitem_index... */
		data->dsqsii_deleted_key   = real_vtab[result].di_key;
		data->dsqsii_deleted_value = real_vtab[result].di_value;
		Dee_Incref(data->dsqsii_deleted_key);
		Dee_Incref(data->dsqsii_deleted_value);
	}
	return Dee_hash_vidx_tovirt(result);
}

PRIVATE WUNUSED NONNULL((1, 3, 4)) int DCALL
dict_mh_seq_setitem_index_impl(Dict *self, size_t index,
                               DeeObject *key, DeeObject *value) {
	int result;
	struct dict_mh_seq_setitem_index_impl_data data;
	data.dsqsii_index = index;
	data.dsqsii_deleted_key = NULL;
	result = dict_setitem_at(self, key, value, &dict_mh_seq_setitem_index_impl_cb, &data);
	if (data.dsqsii_deleted_key) {
		Dee_Decref(data.dsqsii_deleted_key);
		Dee_Decref(data.dsqsii_deleted_value);
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1, 3, 4)) DREF DeeSeqPairObject *DCALL
dict_mh_seq_xchitem_index_impl(Dict *self, size_t index,
                               DeeObject *key, DeeObject *value) {
	DREF DeeSeqPairObject *result;
	struct dict_mh_seq_setitem_index_impl_data data;
	result = DeeSeq_NewPairUninitialized();
	if unlikely(!result)
		goto err;
	data.dsqsii_index = index;
	data.dsqsii_deleted_key = NULL;
	if unlikely(dict_setitem_at(self, key, value, &dict_mh_seq_setitem_index_impl_cb, &data)) {
		if (data.dsqsii_deleted_key)
			goto err_r_kv;
		goto err_r;
	}
	ASSERT(data.dsqsii_deleted_key);
	ASSERT(data.dsqsii_deleted_value);
	DeeSeq_InitPairInherited(result, data.dsqsii_deleted_key, data.dsqsii_deleted_value);
	return result;
err_r_kv:
	Dee_Decref(data.dsqsii_deleted_key);
	Dee_Decref(data.dsqsii_deleted_value);
err_r:
	DeeSeq_FreePairUninitialized(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) /*virt*/ Dee_hash_vidx_t DCALL
dict_mh_seq_insert_impl_cb(void *arg, Dict *self,
                           /*virt*/ Dee_hash_vidx_t overwrite_index,
                           DeeObject **p_value) {
	/*real*/ Dee_hash_vidx_t result;
	(void)overwrite_index;
	(void)p_value;
	result = (/*real*/ Dee_hash_vidx_t)(size_t)(uintptr_t)arg;
	if unlikely(result >= self->d_vused) {
		size_t used = self->d_vused;
		DeeDict_LockEndWrite(self);
		DeeRT_ErrIndexOutOfBounds(Dee_AsObject(self), result, used);
		return Dee_HASH_HTAB_EOF;
	}
	result = dict_unoptimize_vtab_index(self, result);
	return Dee_hash_vidx_tovirt(result);
}

PRIVATE WUNUSED NONNULL((1, 3, 4)) int DCALL
dict_mh_seq_insert_impl(Dict *self, size_t index, DeeObject *key, DeeObject *value) {
	return dict_setitem_at(self, key, value, &dict_mh_seq_insert_impl_cb, (void *)(uintptr_t)index);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeSeqPairObject *DCALL
dict_mh_seq_getitem_index(Dict *__restrict self, size_t index) {
	return dict_mh_seq_getitem_index_impl(self, index, false);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeSeqPairObject *DCALL
dict_mh_seq_trygetitem_index(Dict *__restrict self, size_t index) {
	return dict_mh_seq_getitem_index_impl(self, index, true);
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
dict_mh_seq_setitem_index(Dict *self, size_t index, DeeObject *key_and_value_tuple) {
	int result;
	DREF DeeObject *key_and_value[2];
	if (DeeSeq_Unpack(key_and_value_tuple, 2, key_and_value))
		goto err;
	result = dict_mh_seq_setitem_index_impl(self, index, key_and_value[0], key_and_value[1]);
	Dee_Decref_unlikely(key_and_value[1]);
	Dee_Decref_unlikely(key_and_value[0]);
	return result;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 3)) DREF DeeSeqPairObject *DCALL
dict_mh_seq_xchitem_index(Dict *self, size_t index, DeeObject *key_and_value_tuple) {
	DREF DeeSeqPairObject *result;
	DREF DeeObject *key_and_value[2];
	if (DeeSeq_Unpack(key_and_value_tuple, 2, key_and_value))
		goto err;
	result = dict_mh_seq_xchitem_index_impl(self, index, key_and_value[0], key_and_value[1]);
	Dee_Decref_unlikely(key_and_value[1]);
	Dee_Decref_unlikely(key_and_value[0]);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
dict_mh_seq_insert(Dict *self, size_t index, DeeObject *key_and_value_tuple) {
	int result;
	DREF DeeObject *key_and_value[2];
	if (DeeSeq_Unpack(key_and_value_tuple, 2, key_and_value))
		goto err;
	result = dict_mh_seq_insert_impl(self, index, key_and_value[0], key_and_value[1]);
	Dee_Decref_unlikely(key_and_value[1]);
	Dee_Decref_unlikely(key_and_value[0]);
	return result;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
dict_mh_seq_pushfront(Dict *self, DeeObject *key_and_value_tuple) {
	return dict_mh_seq_insert(self, 0, key_and_value_tuple);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
dict_mh_seq_append(Dict *self, DeeObject *key_and_value_tuple) {
	int result;
	DREF DeeObject *key_and_value[2];
	if (DeeSeq_Unpack(key_and_value_tuple, 2, key_and_value))
		goto err;
	result = dict_setitem(self, key_and_value[0], key_and_value[1]);
	Dee_Decref_unlikely(key_and_value[1]);
	Dee_Decref_unlikely(key_and_value[0]);
	return result;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeSeqPairObject *DCALL
dict_mh_seq_pop(Dict *self, Dee_ssize_t index) {
	DREF DeeSeqPairObject *result;
	struct Dee_dict_item *item;
	result = DeeSeq_NewPairUninitialized();
	if unlikely(!result)
		goto err;
	DeeDict_LockWrite(self);
	if (index < 0)
		index += self->d_vused;
	if unlikely((size_t)index >= self->d_vused) {
		size_t real_size = self->d_vused;
		DeeDict_LockEndWrite(self);
		DeeRT_ErrIndexOutOfBounds(Dee_AsObject(self), (size_t)index, real_size);
		goto err_r;
	}
	if (_DeeDict_CanOptimizeVTab(self))
		dict_optimize_vtab(self);
	item = &_DeeDict_GetRealVTab(self)[index];
	DeeSeq_InitPairvInherited(result, item->di_key_and_value);
	item->di_key = NULL; /* Delete item */
	DBG_memset(&item->di_value, 0xcc, sizeof(item->di_value));
	--self->d_vused;
	dict_autoshrink(self);
	DeeDict_LockEndWrite(self);
	return result;
err_r:
	DeeSeq_FreePairUninitialized(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) size_t DCALL
dict_mh_seq_removeif(Dict *self, DeeObject *should, size_t start, size_t end, size_t max) {
	size_t result = 0;
	DREF DeeSeqPairObject *key_and_value;
	key_and_value = DeeSeq_NewPairUninitialized();
	if unlikely(!key_and_value)
		goto err;
	DeeSeq_InitPair_inplace(key_and_value);
	while (start < end && result < max) {
		DREF DeeObject *item_key;
		DREF DeeObject *item_value;
		struct Dee_dict_item *item;
		DREF DeeObject *should_result_ob;
		int should_result;
		DeeDict_LockReadAndOptimize(self);
again_index_start:
		if (end > self->d_vused) {
			end = self->d_vused;
			if (start >= end) {
				DeeDict_LockEndRead(self);
				break;
			}
		}
		item = &_DeeDict_GetRealVTab(self)[start];
		item_key = item->di_key;
		ASSERT(item_key);
		Dee_Incref(item_key);
		item_value = item->di_value;
		Dee_Incref(item_value);
		DeeDict_LockEndRead(self);
		DeeSeqPair_ELEM(key_and_value)[0] = item_key;   /* Inherit reference */
		DeeSeqPair_ELEM(key_and_value)[1] = item_value; /* Inherit reference */
		should_result_ob = DeeObject_Call(should, 1, (DeeObject *const *)&key_and_value);
		if unlikely(!should_result_ob)
			goto err_key_and_value;
		should_result = DeeObject_BoolInherited(should_result_ob);
		if unlikely(should_result < 0)
			goto err_key_and_value;
		ASSERT(DeeSeqPair_ELEM(key_and_value)[0] == item_key);
		ASSERT(DeeSeqPair_ELEM(key_and_value)[1] == item_value);

		/* Reset "key_and_value" for another iteration */
		if unlikely(DeeObject_IsShared(key_and_value)) {
			Dee_Decref_unlikely(key_and_value);
			key_and_value = DeeSeq_NewPairUninitialized();
			if unlikely(!key_and_value)
				goto err;
			DeeSeq_InitPair_inplace(key_and_value);
		} else {
			Dee_Decref_unlikely(DeeSeqPair_ELEM(key_and_value)[0]);
			Dee_Decref_unlikely(DeeSeqPair_ELEM(key_and_value)[1]);
			DBG_memset(DeeSeqPair_ELEM(key_and_value), 0xcc, 2 * sizeof(DREF DeeObject *));
		}
		if (!should_result) {
			++start;
			continue;
		}

		/* Yes: *should* delete this item */
		DeeDict_LockWrite(self);
		if unlikely(_DeeDict_CanOptimizeVTab(self))
			dict_optimize_vtab(self);
		if unlikely(item != &_DeeDict_GetRealVTab(self)[start])
			goto downgrade_and_again_index_start;
		if unlikely(item->di_key != item_key)
			goto downgrade_and_again_index_start;
		if unlikely(item->di_value != item_value)
			goto downgrade_and_again_index_start;
		item->di_key = NULL; /* Delete item */
		DBG_memset(&item->di_value, 0xcc, sizeof(item->di_value));
		--self->d_vused;
		dict_autoshrink(self);
		dict_optimize_vtab(self);
		ASSERT(self->d_vused == self->d_vsize);
		++result;
		++start;
		if (start >= end || result >= max) {
			DeeDict_LockEndWrite(self);
			break;
		}
		DeeDict_LockDowngrade(self);
		goto again_index_start;
	}
	DeeSeq_FiniPair_inplace(key_and_value);
	DeeSeq_FreePairUninitialized(key_and_value);
	return result;
downgrade_and_again_index_start:
	DeeDict_LockDowngrade(self);
	goto again_index_start;
err_key_and_value:
	Dee_Decref_likely(key_and_value);
err:
	return (size_t)-1;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
dict_mh_seq_foreach(Dict *__restrict self, Dee_foreach_t cb, void *arg) {
	DREF DeeSeqPairObject *key_and_value;
	Dee_hash_vidx_t i;
	Dee_ssize_t temp, result = 0;
	key_and_value = DeeSeq_NewPairUninitialized();
	if unlikely(!key_and_value)
		goto err;
	DeeSeq_InitPair_inplace(key_and_value);
	DeeDict_LockRead(self);
	for (i = Dee_hash_vidx_tovirt(0);
	     Dee_hash_vidx_virt_lt_real(i, self->d_vsize); ++i) {
		DREF DeeObject *item_key, *item_value;
		struct Dee_dict_item *item;
		item = &_DeeDict_GetVirtVTab(self)[i];
		item_key  = item->di_key;
		if unlikely(!item_key)
			continue; /* Deleted, and not-yet-optimized key */
		Dee_Incref(item_key);
		item_value = item->di_value;
		Dee_Incref(item_value);
		DeeDict_LockEndRead(self);
		DeeSeqPair_ELEM(key_and_value)[0] = item_key;   /* Inherit reference */
		DeeSeqPair_ELEM(key_and_value)[1] = item_value; /* Inherit reference */
		temp = (*cb)(arg, Dee_AsObject(key_and_value));
		if unlikely(temp < 0)
			goto err_temp_key_and_value;
		result += temp;
		ASSERT(DeeSeqPair_ELEM(key_and_value)[0] == item_key);
		ASSERT(DeeSeqPair_ELEM(key_and_value)[1] == item_value);
		/* Reset "key_and_value" for another iteration */
		if unlikely(DeeObject_IsShared(key_and_value)) {
			Dee_Decref_unlikely(key_and_value);
			key_and_value = DeeSeq_NewPairUninitialized();
			if unlikely(!key_and_value)
				goto err;
			DeeSeq_InitPair_inplace(key_and_value);
		} else {
			Dee_Decref_unlikely(DeeSeqPair_ELEM(key_and_value)[0]);
			Dee_Decref_unlikely(DeeSeqPair_ELEM(key_and_value)[1]);
			DBG_memset(DeeSeqPair_ELEM(key_and_value), 0xcc, 2 * sizeof(DREF DeeObject *));
		}
		DeeDict_LockRead(self);
	}
	DeeDict_LockEndRead(self);
	DeeSeq_FiniPair_inplace(key_and_value);
	DeeSeq_FreePairUninitialized(key_and_value);
	return result;
err_temp_key_and_value:
	Dee_Decref_likely(key_and_value);
	return temp;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
dict_mh_seq_foreach_reverse(Dict *__restrict self, Dee_foreach_t cb, void *arg) {
	DREF DeeSeqPairObject *key_and_value;
	Dee_hash_vidx_t i;
	Dee_ssize_t temp, result = 0;
	key_and_value = DeeSeq_NewPairUninitialized();
	if unlikely(!key_and_value)
		goto err;
	DeeSeq_InitPair_inplace(key_and_value);
	DeeDict_LockRead(self);
	i = Dee_hash_vidx_tovirt(self->d_vsize);
	while (i > Dee_hash_vidx_tovirt(0)) {
		DREF DeeObject *item_key, *item_value;
		struct Dee_dict_item *item;
		--i;
		item = &_DeeDict_GetVirtVTab(self)[i];
		item_key  = item->di_key;
		if unlikely(!item_key)
			continue; /* Deleted, and not-yet-optimized key */
		Dee_Incref(item_key);
		item_value = item->di_value;
		Dee_Incref(item_value);
		DeeDict_LockEndRead(self);
		DeeSeqPair_ELEM(key_and_value)[0] = item_key;   /* Inherit reference */
		DeeSeqPair_ELEM(key_and_value)[1] = item_value; /* Inherit reference */
		temp = (*cb)(arg, Dee_AsObject(key_and_value));
		if unlikely(temp < 0)
			goto err_temp_key_and_value;
		result += temp;
		ASSERT(DeeSeqPair_ELEM(key_and_value)[0] == item_key);
		ASSERT(DeeSeqPair_ELEM(key_and_value)[1] == item_value);
		/* Reset "key_and_value" for another iteration */
		if unlikely(DeeObject_IsShared(key_and_value)) {
			Dee_Decref_unlikely(key_and_value);
			key_and_value = DeeSeq_NewPairUninitialized();
			if unlikely(!key_and_value)
				goto err;
			DeeSeq_InitPair_inplace(key_and_value);
		} else {
			Dee_Decref_unlikely(DeeSeqPair_ELEM(key_and_value)[0]);
			Dee_Decref_unlikely(DeeSeqPair_ELEM(key_and_value)[1]);
			DBG_memset(DeeSeqPair_ELEM(key_and_value), 0xcc, 2 * sizeof(DREF DeeObject *));
		}
		DeeDict_LockRead(self);
		if unlikely(i > Dee_hash_vidx_tovirt(self->d_vsize))
			i = Dee_hash_vidx_tovirt(self->d_vsize);
	}
	DeeDict_LockEndRead(self);
	DeeSeq_FiniPair_inplace(key_and_value);
	DeeSeq_FreePairUninitialized(key_and_value);
	return result;
err_temp_key_and_value:
	Dee_Decref_likely(key_and_value);
	return temp;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
dict_mh_seq_enumerate_index(Dict *__restrict self, Dee_seq_enumerate_index_t cb,
                            void *arg, size_t start, size_t end) {
	DREF DeeSeqPairObject *key_and_value;
	Dee_ssize_t temp, result = 0;
	key_and_value = DeeSeq_NewPairUninitialized();
	if unlikely(!key_and_value)
		goto err;
	DeeSeq_InitPair_inplace(key_and_value);
	while (start < end) {
		DREF DeeObject *item_key;
		DREF DeeObject *item_value;
		struct Dee_dict_item *item;
		DeeDict_LockReadAndOptimize(self);
		if (end > self->d_vused) {
			end = self->d_vused;
			if (start >= end) {
				DeeDict_LockEndRead(self);
				break;
			}
		}
		item = &_DeeDict_GetRealVTab(self)[start];
		item_key = item->di_key;
		Dee_Incref(item_key);
		item_value = item->di_value;
		Dee_Incref(item_value);
		DeeDict_LockEndRead(self);
		DeeSeqPair_ELEM(key_and_value)[0] = item_key;   /* Inherit reference */
		DeeSeqPair_ELEM(key_and_value)[1] = item_value; /* Inherit reference */
		temp = (*cb)(arg, start, Dee_AsObject(key_and_value));
		if unlikely(temp < 0)
			goto err_temp_key_and_value;
		result += temp;
		ASSERT(DeeSeqPair_ELEM(key_and_value)[0] == item_key);
		ASSERT(DeeSeqPair_ELEM(key_and_value)[1] == item_value);
		/* Reset "key_and_value" for another iteration */
		if unlikely(DeeObject_IsShared(key_and_value)) {
			Dee_Decref_unlikely(key_and_value);
			key_and_value = DeeSeq_NewPairUninitialized();
			if unlikely(!key_and_value)
				goto err;
			DeeSeq_InitPair_inplace(key_and_value);
		} else {
			Dee_Decref_unlikely(DeeSeqPair_ELEM(key_and_value)[0]);
			Dee_Decref_unlikely(DeeSeqPair_ELEM(key_and_value)[1]);
			DBG_memset(DeeSeqPair_ELEM(key_and_value), 0xcc, 2 * sizeof(DREF DeeObject *));
		}
		++start;
	}
	DeeSeq_FiniPair_inplace(key_and_value);
	DeeSeq_FreePairUninitialized(key_and_value);
	return result;
err_temp_key_and_value:
	Dee_Decref_likely(key_and_value);
	return temp;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
dict_mh_seq_enumerate_index_reverse(Dict *__restrict self, Dee_seq_enumerate_index_t cb,
                                    void *arg, size_t start, size_t end) {
	DREF DeeSeqPairObject *key_and_value;
	Dee_ssize_t temp, result = 0;
	key_and_value = DeeSeq_NewPairUninitialized();
	if unlikely(!key_and_value)
		goto err;
	DeeSeq_InitPair_inplace(key_and_value);
	while (start < end) {
		DREF DeeObject *item_key;
		DREF DeeObject *item_value;
		struct Dee_dict_item *item;
		DeeDict_LockReadAndOptimize(self);
		if (end > self->d_vused) {
			end = self->d_vused;
			if (start >= end) {
				DeeDict_LockEndRead(self);
				break;
			}
		}
		--end;
		item = &_DeeDict_GetRealVTab(self)[end];
		item_key = item->di_key;
		Dee_Incref(item_key);
		item_value = item->di_value;
		Dee_Incref(item_value);
		DeeDict_LockEndRead(self);
		DeeSeqPair_ELEM(key_and_value)[0] = item_key;   /* Inherit reference */
		DeeSeqPair_ELEM(key_and_value)[1] = item_value; /* Inherit reference */
		temp = (*cb)(arg, end, Dee_AsObject(key_and_value));
		if unlikely(temp < 0)
			goto err_temp_key_and_value;
		result += temp;
		ASSERT(DeeSeqPair_ELEM(key_and_value)[0] == item_key);
		ASSERT(DeeSeqPair_ELEM(key_and_value)[1] == item_value);
		/* Reset "key_and_value" for another iteration */
		if unlikely(DeeObject_IsShared(key_and_value)) {
			Dee_Decref_unlikely(key_and_value);
			key_and_value = DeeSeq_NewPairUninitialized();
			if unlikely(!key_and_value)
				goto err;
			DeeSeq_InitPair_inplace(key_and_value);
		} else {
			Dee_Decref_unlikely(DeeSeqPair_ELEM(key_and_value)[0]);
			Dee_Decref_unlikely(DeeSeqPair_ELEM(key_and_value)[1]);
			DBG_memset(DeeSeqPair_ELEM(key_and_value), 0xcc, 2 * sizeof(DREF DeeObject *));
		}
	}
	DeeSeq_FiniPair_inplace(key_and_value);
	DeeSeq_FreePairUninitialized(key_and_value);
	return result;
err_temp_key_and_value:
	Dee_Decref_likely(key_and_value);
	return temp;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
dict_contains(Dict *self, DeeObject *key) {
	int result = dict_hasitem(self, key);
	if unlikely(result < 0)
		goto err;
	return_bool(result);
err:
	return NULL;
}

PRIVATE struct type_gc tpconst dict_gc = {
	/* .tp_clear = */ (void (DCALL *)(DeeObject *__restrict))&dict_clear,
	/* .tp_cc    = */ (bool (DCALL *)(DeeObject *__restrict))&dict_cc,
};

PRIVATE struct type_seq dict_seq = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&dict_iter,
	/* .tp_sizeob                     = */ DEFIMPL(&default__sizeob__with__size),
	/* .tp_contains                   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&dict_contains,
	/* .tp_getitem                    = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&dict_getitem,
	/* .tp_delitem                    = */ (int (DCALL *)(DeeObject *, DeeObject *))&dict_delitem,
	/* .tp_setitem                    = */ (int (DCALL *)(DeeObject *, DeeObject *, DeeObject *))&dict_setitem,
	/* .tp_getrange                   = */ DEFIMPL_UNSUPPORTED(&default__getrange__unsupported),
	/* .tp_delrange                   = */ DEFIMPL_UNSUPPORTED(&default__delrange__unsupported),
	/* .tp_setrange                   = */ DEFIMPL_UNSUPPORTED(&default__setrange__unsupported),
	/* .tp_foreach                    = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&dict_mh_seq_foreach,
	/* .tp_foreach_pair               = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_pair_t, void *))&dict_foreach_pair,
	/* .tp_bounditem                  = */ (int (DCALL *)(DeeObject *, DeeObject *))&dict_bounditem,
	/* .tp_hasitem                    = */ (int (DCALL *)(DeeObject *, DeeObject *))&dict_hasitem,
	/* .tp_size                       = */ (size_t (DCALL *)(DeeObject *__restrict))&dict_size,
	/* .tp_size_fast                  = */ (size_t (DCALL *)(DeeObject *__restrict))&dict_size_fast,
	/* .tp_getitem_index              = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&dict_getitem_index,
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ (int (DCALL *)(DeeObject *, size_t))&dict_delitem_index,
	/* .tp_setitem_index              = */ (int (DCALL *)(DeeObject *, size_t, DeeObject *))&dict_setitem_index,
	/* .tp_bounditem_index            = */ (int (DCALL *)(DeeObject *, size_t))&dict_bounditem_index,
	/* .tp_hasitem_index              = */ (int (DCALL *)(DeeObject *, size_t))&dict_hasitem_index,
	/* .tp_getrange_index             = */ DEFIMPL_UNSUPPORTED(&default__getrange_index__unsupported),
	/* .tp_delrange_index             = */ DEFIMPL_UNSUPPORTED(&default__delrange_index__unsupported),
	/* .tp_setrange_index             = */ DEFIMPL_UNSUPPORTED(&default__setrange_index__unsupported),
	/* .tp_getrange_index_n           = */ DEFIMPL_UNSUPPORTED(&default__getrange_index_n__unsupported),
	/* .tp_delrange_index_n           = */ DEFIMPL_UNSUPPORTED(&default__delrange_index_n__unsupported),
	/* .tp_setrange_index_n           = */ DEFIMPL_UNSUPPORTED(&default__setrange_index_n__unsupported),
	/* .tp_trygetitem                 = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&dict_trygetitem,
	/* .tp_trygetitem_index           = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&dict_trygetitem_index,
	/* .tp_trygetitem_string_hash     = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, Dee_hash_t))&dict_trygetitem_string_hash,
	/* .tp_getitem_string_hash        = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, Dee_hash_t))&dict_getitem_string_hash,
	/* .tp_delitem_string_hash        = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&dict_delitem_string_hash,
	/* .tp_setitem_string_hash        = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t, DeeObject *))&dict_setitem_string_hash,
	/* .tp_bounditem_string_hash      = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&dict_bounditem_string_hash,
	/* .tp_hasitem_string_hash        = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&dict_hasitem_string_hash,
	/* .tp_trygetitem_string_len_hash = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&dict_trygetitem_string_len_hash,
	/* .tp_getitem_string_len_hash    = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&dict_getitem_string_len_hash,
	/* .tp_delitem_string_len_hash    = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&dict_delitem_string_len_hash,
	/* .tp_setitem_string_len_hash    = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t, DeeObject *))&dict_setitem_string_len_hash,
	/* .tp_bounditem_string_len_hash  = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&dict_bounditem_string_len_hash,
	/* .tp_hasitem_string_len_hash    = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&dict_hasitem_string_len_hash,
};


INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL /* INTERN because shared with "./hashset.c" */
dict___hidxio__(DeeDictObject *__restrict self) {
	Dee_hash_vidx_t valloc = Dee_atomic_read_with_atomic_rwlock(&self->d_valloc, &self->d_lock);
	Dee_hash_hidxio_t hidxio = Dee_HASH_HIDXIO_FROM_VALLOC(valloc);
	return DeeInt_NEWU(hidxio);
}

INTERN WUNUSED NONNULL((1)) int DCALL /* INTERN because shared with "./hashset.c" */
dict_nonempty_as_bound(DeeDictObject *__restrict self) {
	size_t size = DeeDict_SIZE_ATOMIC(self);
	return Dee_BOUND_FROMBOOL(size);
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeSeqPairObject *DCALL dict_trygetfirst(Dict *__restrict self);
PRIVATE WUNUSED NONNULL((1)) DREF DeeSeqPairObject *DCALL dict_getfirst(Dict *__restrict self);
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL dict_getfirstkey(Dict *__restrict self);
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL dict_getfirstvalue(Dict *__restrict self);
PRIVATE WUNUSED NONNULL((1)) int DCALL dict_delfirst(Dict *__restrict self);
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL dict_setfirst(Dict *self, DeeObject *value);
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL dict_setfirstkey(Dict *self, DeeObject *value);
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL dict_setfirstvalue(Dict *self, DeeObject *value);
PRIVATE WUNUSED NONNULL((1)) DREF DeeSeqPairObject *DCALL dict_trygetlast(Dict *__restrict self);
PRIVATE WUNUSED NONNULL((1)) DREF DeeSeqPairObject *DCALL dict_getlast(Dict *__restrict self);
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL dict_getlastkey(Dict *__restrict self);
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL dict_getlastvalue(Dict *__restrict self);
PRIVATE WUNUSED NONNULL((1)) int DCALL dict_dellast(Dict *__restrict self);
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL dict_setlast(Dict *self, DeeObject *value);
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL dict_setlastkey(Dict *self, DeeObject *value);
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL dict_setlastvalue(Dict *self, DeeObject *value);

#ifndef __INTELLISENSE__
DECL_END
#define DEFINE_dict_first
#include "dict-firstlast-impl.c.inl"
#define DEFINE_dict_firstkey
#include "dict-firstlast-impl.c.inl"
#define DEFINE_dict_firstvalue
#include "dict-firstlast-impl.c.inl"
#define DEFINE_dict_last
#include "dict-firstlast-impl.c.inl"
#define DEFINE_dict_lastkey
#include "dict-firstlast-impl.c.inl"
#define DEFINE_dict_lastvalue
#include "dict-firstlast-impl.c.inl"
DECL_BEGIN
#endif /* !__INTELLISENSE__ */

struct dict_fromkeys_data {
	Dict      *dfkd_dict;  /* [1..1] Dict to insert stuff into. */
	DeeObject *dfkd_value; /* [1..1] Value to insert, or callback to generate value. */
};

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
dict_fromkeys_with_value(void *arg, DeeObject *key) {
	int result;
	struct dict_fromkeys_data *data;
	data = (struct dict_fromkeys_data *)arg;
#ifdef HAVE_dict_setitem_unlocked_fast_inherited
	Dee_Incref(key);
	Dee_Incref(data->dfkd_value);
	result = dict_setitem_unlocked_fast_inherited(data->dfkd_dict, key, data->dfkd_value);
	if unlikely(result) {
		Dee_DecrefNokill(key);
		Dee_DecrefNokill(data->dfkd_value);
	}
#else /* HAVE_dict_setitem_unlocked_fast_inherited */
	result = dict_setitem_unlocked(data->dfkd_dict, key, data->dfkd_value);
#endif /* !HAVE_dict_setitem_unlocked_fast_inherited */
	ASSERT(result <= 0);
	return (Dee_ssize_t)result;
}

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
dict_fromkeys_with_valuefor(void *arg, DeeObject *key) {
	int result;
	DREF DeeObject *used_value;
	struct dict_fromkeys_data *data;
	data = (struct dict_fromkeys_data *)arg;
	used_value = DeeObject_Call(data->dfkd_value, 1, &key);
	if unlikely(!used_value)
		goto err;
#ifdef HAVE_dict_setitem_unlocked_fast_inherited
	Dee_Incref(key);
	result = dict_setitem_unlocked_fast_inherited(data->dfkd_dict, key, used_value);
	if unlikely(result) {
		Dee_DecrefNokill(key);
		Dee_Decref(used_value);
	}
#else /* HAVE_dict_setitem_unlocked_fast_inherited */
	result = dict_setitem_unlocked(data->dfkd_dict, key, used_value);
	Dee_Decref_unlikely(used_value);
#endif /* !HAVE_dict_setitem_unlocked_fast_inherited */
	return result;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
dict_init_values_after_keys(Dict *__restrict self, DeeObject *value, DeeObject *valuefor) {
	Dee_hash_vidx_t i;
	struct Dee_dict_item *vtab = _DeeDict_GetRealVTab(self);
	if (valuefor) {
		for (i = 0; i < self->d_vsize; ++i) {
			DREF DeeObject *computed_value;
			struct Dee_dict_item *item = &vtab[i];
			computed_value = DeeObject_Call(valuefor, 1, &item->di_key);
			if unlikely(!computed_value)
				goto err_values_before_i;
			item->di_value = computed_value; /* Inherit reference */
		}
	} else {
		for (i = 0; i < self->d_vsize; ++i)
			vtab[i].di_value = value; /* Inherit reference */
		Dee_Incref_n(value, self->d_vsize);
	}
	return 0;
err_values_before_i:
	while (i) {
		--i;
		Dee_Decref(vtab[i].di_value);
	}
/*err:*/
	return -1;
}

PRIVATE NONNULL((1)) void DCALL
dict_fini_keysonly(Dict *__restrict self) {
	if (self->d_vtab != DeeDict_EmptyVTab) {
		Dee_hash_vidx_t i;
		struct Dee_dict_item *vtab;
		vtab = _DeeDict_GetRealVTab(self);
		for (i = 0; i < self->d_vsize; ++i)
			Dee_Decref(vtab[i].di_key);
		_DeeDict_TabsFree(vtab);
	}
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF Dict *DCALL
dict_from_dict_keys(Dict *dict_keys, DeeObject *value, DeeObject *valuefor) {
	DREF Dict *result;
	result = DeeGCObject_MALLOC(Dict);
	if unlikely(!result)
		goto err;
#ifdef HAVE_dict_init_fromcopy_keysonly
	if unlikely(dict_init_fromcopy_keysonly(result, dict_keys))
		goto err_r;
	ASSERT(result->d_vused == result->d_vsize);
#else /* HAVE_dict_init_fromcopy_keysonly */
	if unlikely(dict_init_fromcopy(result, dict_keys))
		goto err_r;
	ASSERT(result->d_vused == result->d_vsize);
	{
		Dee_hash_vidx_t i;
		struct Dee_dict_item *vtab = _DeeDict_GetRealVTab(result);
		for (i = 0; i < result->d_vsize; ++i) {
			struct Dee_dict_item *item = &vtab[i];
			Dee_Decref_unlikely(item->di_value);
			DBG_memset(&item->di_value, 0xcc, sizeof(item->di_value));
		}
	}
#endif /* !HAVE_dict_init_fromcopy_keysonly */
	if unlikely(dict_init_values_after_keys(result, value, valuefor))
		goto err_r_keysonly;
#ifndef DICT_INITFROM_NEEDSLOCK
	Dee_atomic_rwlock_init(&result->d_lock);
#endif /* !DICT_INITFROM_NEEDSLOCK */
	Dee_weakref_support_init(result);
	return result;
err_r_keysonly:
	dict_fini_keysonly(result);
err_r:
	DeeGCObject_FREE(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF Dict *DCALL
dict_from_rodict_keys(DeeRoDictObject *dict_keys, DeeObject *value, DeeObject *valuefor) {
	DREF Dict *result;
	result = DeeGCObject_MALLOC(Dict);
	if unlikely(!result)
		goto err;
	if unlikely(dict_init_fromrodict_keysonly(result, dict_keys))
		goto err_r;
	if unlikely(dict_init_values_after_keys(result, value, valuefor))
		goto err_r_keysonly;
#ifndef DICT_INITFROM_NEEDSLOCK
	Dee_atomic_rwlock_init(&result->d_lock);
#endif /* !DICT_INITFROM_NEEDSLOCK */
	Dee_weakref_support_init(result);
	return result;
err_r_keysonly:
	dict_fini_keysonly(result);
err_r:
	DeeGCObject_FREE(result);
err:
	return NULL;
}

#ifdef CONFIG_EXPERIMENTAL_ORDERED_HASHSET
PRIVATE WUNUSED NONNULL((1, 2)) DREF Dict *DCALL
dict_from_hashset_keys(DeeHashSetObject *keys, DeeObject *value, DeeObject *valuefor) {
	DREF Dict *result;
	result = DeeGCObject_MALLOC(Dict);
	if unlikely(!result)
		goto err;
	if unlikely(dict_init_fromhashset_keysonly(result, keys))
		goto err_r;
	ASSERT(result->d_vused == result->d_vsize);
	if unlikely(dict_init_values_after_keys(result, value, valuefor))
		goto err_r_keysonly;
#ifndef DICT_INITFROM_NEEDSLOCK
	Dee_atomic_rwlock_init(&result->d_lock);
#endif /* !DICT_INITFROM_NEEDSLOCK */
	Dee_weakref_support_init(result);
	return result;
err_r_keysonly:
	dict_fini_keysonly(result);
err_r:
	DeeGCObject_FREE(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF Dict *DCALL
dict_from_roset_keys(DeeRoSetObject *keys, DeeObject *value, DeeObject *valuefor) {
	DREF Dict *result;
	result = DeeGCObject_MALLOC(Dict);
	if unlikely(!result)
		goto err;
	if unlikely(dict_init_fromroset_keysonly(result, keys))
		goto err_r;
	if unlikely(dict_init_values_after_keys(result, value, valuefor))
		goto err_r_keysonly;
#ifndef DICT_INITFROM_NEEDSLOCK
	Dee_atomic_rwlock_init(&result->d_lock);
#endif /* !DICT_INITFROM_NEEDSLOCK */
	Dee_weakref_support_init(result);
	return result;
err_r_keysonly:
	dict_fini_keysonly(result);
err_r:
	DeeGCObject_FREE(result);
err:
	return NULL;
}
#endif /* CONFIG_EXPERIMENTAL_ORDERED_HASHSET */

PRIVATE WUNUSED NONNULL((1, 2)) DREF Dict *DCALL
dict_fromkeys(DeeObject *keys, DeeObject *value, DeeObject *valuefor) {
	struct dict_fromkeys_data data;
	Dee_ssize_t foreach_status;
	DeeTypeObject *tp_keys = Dee_TYPE(keys);
	size_t hint;

	/* Optimizations for special, known keys types. */
	if (tp_keys == &DefaultSequence_MapKeys_Type) {
		/* Special optimization when "keys" are the keys of another Dict/RoDict */
		DefaultSequence_MapProxy *proxy = (DefaultSequence_MapProxy *)keys;
		DeeObject *mapping_of_keys = proxy->dsmp_map;
		DeeTypeObject *tp_mapping_of_keys = Dee_TYPE(mapping_of_keys);
		if (tp_mapping_of_keys == &DeeDict_Type)
			return dict_from_dict_keys((Dict *)mapping_of_keys, value, valuefor);
		if (tp_mapping_of_keys == &DeeRoDict_Type)
			return dict_from_rodict_keys((DeeRoDictObject *)mapping_of_keys, value, valuefor);
#ifdef CONFIG_EXPERIMENTAL_ORDERED_HASHSET
	} else if (tp_keys == &DeeHashSet_Type) {
		/* Special optimization when "keys" is a HashSet: Duplicate its control structures */
		return dict_from_hashset_keys((DeeHashSetObject *)keys, value, valuefor);
	} else if (tp_keys == &DeeRoSet_Type) {
		/* Special optimization when "keys" is a RoSet: Duplicate its control structures */
		return dict_from_roset_keys((DeeRoSetObject *)keys, value, valuefor);
#endif /* CONFIG_EXPERIMENTAL_ORDERED_HASHSET */
	}

	hint = DeeObject_SizeFast(keys);
	if likely(hint != (size_t)-1) {
		data.dfkd_dict = (DREF Dict *)DeeDict_TryNewWithHint(hint);
	} else {
		data.dfkd_dict = (DREF Dict *)DeeDict_TryNewWithWeakHint(DICT_FROMSEQ_DEFAULT_HINT);
	}
	if unlikely(!data.dfkd_dict) {
		data.dfkd_dict = (DREF Dict *)DeeDict_New();
		if unlikely(!data.dfkd_dict)
			goto err;
	}
	if unlikely(valuefor) {
		data.dfkd_value = valuefor;
		foreach_status  = DeeObject_Foreach(keys, &dict_fromkeys_with_valuefor, &data);
	} else {
		data.dfkd_value = value;
		foreach_status  = DeeObject_Foreach(keys, &dict_fromkeys_with_value, &data);
	}
	if unlikely(foreach_status < 0)
		goto err_r;
	return data.dfkd_dict;
err_r:
	Dee_DecrefDokill(data.dfkd_dict);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF Dict *DCALL
dict_fromkeys_f(DeeTypeObject *UNUSED(dict), size_t argc,
                DeeObject *const *argv, DeeObject *kw) {
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("fromkeys", params: "
	DeeObject *keys: ?DSet;
	DeeObject *value = Dee_None;
	DeeObject *valuefor:?DCallable = NULL;
", docStringPrefix: "dict");]]]*/
#define dict_fromkeys_params "keys:?DSet,value=!N,valuefor?:?DCallable"
	struct {
		DeeObject *keys;
		DeeObject *value;
		DeeObject *valuefor;
	} args;
	args.value = Dee_None;
	args.valuefor = NULL;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__keys_value_valuefor, "o|oo:fromkeys", &args))
		goto err;
/*[[[end]]]*/
	return dict_fromkeys(args.keys, args.value, args.valuefor);
err:
	return NULL;
}


PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
dict_mh_setold(Dict *self, DeeObject *key, DeeObject *value) {
	DREF DeeObject *old_value;
	old_value = dict_mh_setold_ex(self, key, value);
	if (old_value == ITER_DONE)
		return 0;
	if unlikely(!old_value)
		goto err;
	Dee_Decref(old_value);
	return 1;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
dict_mh_setnew(Dict *self, DeeObject *key, DeeObject *value) {
	DREF DeeObject *old_value;
	old_value = dict_mh_setnew_ex(self, key, value);
	if (old_value == ITER_DONE)
		return 1;
	if unlikely(!old_value)
		goto err;
	Dee_Decref(old_value);
	return 0;
err:
	return -1;
}


#define D_TKey   "?O"
#define D_TValue "?O"
#define D_TItem  "?T2" D_TKey D_TValue

#ifndef CONFIG_NO_DEEMON_100_COMPAT
PRIVATE DEFINE_FLOAT(float_1_point_0, 1.0);
INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
deprecated_d100_get_maxloadfactor(DeeObject *__restrict UNUSED(self)) {
	Dee_Incref(&float_1_point_0);
	return Dee_AsObject(&float_1_point_0);
}
#define deprecated_d100_del_maxloadfactor (*(int (DCALL *)(DeeObject *))&_DeeNone_reti0_1)
#define deprecated_d100_set_maxloadfactor (*(int (DCALL *)(DeeObject *, DeeObject *))&_DeeNone_reti0_2)
#endif /* !CONFIG_NO_DEEMON_100_COMPAT */

PRIVATE struct type_getset tpconst dict_getsets[] = {
	TYPE_GETSET_BOUND(STR_first, &dict_getfirst, &dict_delfirst, &dict_setfirst, &dict_nonempty_as_bound, "->" D_TItem),
	TYPE_GETSET_BOUND(STR_last, &dict_getlast, &dict_dellast, &dict_setlast, &dict_nonempty_as_bound, "->" D_TItem),
	TYPE_GETSET_BOUND("firstkey", &dict_getfirstkey, &dict_delfirst, &dict_setfirstkey, &dict_nonempty_as_bound, "->" D_TKey),
	TYPE_GETSET_BOUND("lastkey", &dict_getlastkey, &dict_dellast, &dict_setlastkey, &dict_nonempty_as_bound, "->" D_TKey),
	TYPE_GETSET_BOUND("firstvalue", &dict_getfirstvalue, &dict_delfirst, &dict_setfirstvalue, &dict_nonempty_as_bound, "->" D_TValue),
	TYPE_GETSET_BOUND("lastvalue", &dict_getlastvalue, &dict_dellast, &dict_setlastvalue, &dict_nonempty_as_bound, "->" D_TValue),

	TYPE_GETTER_AB(STR_cached, &DeeObject_NewRef, "->?."),
	TYPE_GETTER_AB(STR_frozen, &DeeRoDict_FromDict, "->?#Frozen"),
	TYPE_GETTER_AB_F("__sizeof__", &dict_sizeof, METHOD_FNOREFESCAPE, "->?Dint"),
	TYPE_GETTER_AB_F("__hidxio__", &dict___hidxio__, METHOD_FNOREFESCAPE,
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



INTDEF struct type_member tpconst dict_members[]; /* INTERN because shared with DeeHashSet_Type */
INTERN_TPCONST struct type_member tpconst dict_members[] = {
	TYPE_MEMBER_FIELD_DOC("__valloc__", STRUCT_CONST | STRUCT_ATOMIC | STRUCT_SIZE_T,
	                      offsetof(Dict, d_valloc), "## of allocated vtab slots"),
	TYPE_MEMBER_FIELD_DOC("__vsize__", STRUCT_CONST | STRUCT_ATOMIC | STRUCT_SIZE_T,
	                      offsetof(Dict, d_vsize), "## of used vtab slots (including deleted slots)"),
	TYPE_MEMBER_FIELD_DOC("__vused__", STRUCT_CONST | STRUCT_ATOMIC | STRUCT_SIZE_T,
	                      offsetof(Dict, d_vused), "## of used vtab slots (excluding deleted slots; same as ?#{op:size})"),
	TYPE_MEMBER_FIELD_DOC("__hmask__", STRUCT_CONST | STRUCT_ATOMIC | STRUCT_SIZE_T,
	                      offsetof(Dict, d_hmask), "Currently active hash-mask"),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst dict_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &DictIterator_Type),
	TYPE_MEMBER_CONST(STR_Frozen, &DeeRoDict_Type),
	TYPE_MEMBER_CONST("__map_getitem_always_bound__", Dee_True),
	TYPE_MEMBER_CONST("__seq_getitem_always_bound__", Dee_True), /* Must be specified because we also define "__seq_getitem__" */
	TYPE_MEMBER_END
};

PRIVATE struct type_method tpconst dict_methods[] = {
//	TYPE_KWMETHOD("byhash", &dict_byhash, DOC_GET(map_byhash_doc)), /* TODO */
	TYPE_METHOD_HINTREF(Sequence_clear),
	TYPE_METHOD_HINTREF(Mapping_pop),
	TYPE_METHOD_HINTREF(Mapping_setold),
	TYPE_METHOD_HINTREF(Mapping_setnew),
	TYPE_METHOD_HINTREF(Mapping_setold_ex),
	TYPE_METHOD_HINTREF(Mapping_setnew_ex),
	TYPE_METHOD_HINTREF(Mapping_setdefault),
	TYPE_METHOD_HINTREF(Mapping_popitem),
	TYPE_METHOD_HINTREF(Mapping_remove),

	TYPE_KWMETHOD_F("shrink", &dict_shrink, METHOD_FNOREFESCAPE,
	                "(fully=!t)->?Dbool\n"
	                "#pfully{When !f, the same sort of shrinking as done automatically when an item is removed. "
	                /*   */ "When !t, force deallocation of #Iall unused items, even if that ruins hash "
	                /*   */ "characteristics / memory efficiency}"
	                "#r{Returns !t if memory was freed}"
	                "Release unused memory. In terms of implementation, after a call ${shrink(true)}, the ?. "
	                /**/ "will have been modified such that ?#__valloc__ #C{==} ?#__vused__ #C{==} ?#__vsize__, "
	                /**/ "and ?#__vmask__ will be the smallest hash-mask able to describe ?#__valloc__ items."),

	TYPE_KWMETHOD_F("reserve", &dict_reserve, METHOD_FNOREFESCAPE,
	                "(total=!0,more=!0,weak=!f)->?Dbool\n"
	                "#pweak{Should the size-hint be considered weak? (s.a. ?#{op:constructor}'s #Cweak argument)}"
	                "#r{Indicative of the ?. having sufficient preallocated space on return}"
	                "Try to preallocate buffer space for ${({#this, total} > ...) + more} items"),

	TYPE_METHOD_HINTREF_DOC(__seq_xchitem__, "(index:?Dint,item:" D_TItem ")->" D_TItem),
	TYPE_METHOD_HINTREF(__seq_erase__),
	TYPE_METHOD_HINTREF_DOC(__seq_insert__, "(index:?Dint,item:" D_TItem ")"),
	TYPE_METHOD_HINTREF_DOC(__seq_append__, "(item:" D_TItem ")"),
	TYPE_METHOD_HINTREF_DOC(__seq_pushfront__, "(item:" D_TItem ")"),
	TYPE_METHOD_HINTREF_DOC(__seq_pop__, "(index=!-1)->" D_TItem),
	TYPE_METHOD_HINTREF(__seq_removeif__),
	TYPE_METHOD_HINTREF(__seq_reverse__),
	TYPE_METHOD_HINTREF(__seq_iter__),
	TYPE_METHOD_HINTREF(__seq_enumerate__),
	TYPE_METHOD_HINTREF(__seq_getitem__),
	TYPE_METHOD_HINTREF(__seq_delitem__),
	TYPE_METHOD_HINTREF(__seq_setitem__),
	TYPE_METHOD_HINTREF(__seq_compare__),
	TYPE_METHOD_HINTREF(__seq_compare_eq__),
	TYPE_METHOD_END
};

PRIVATE struct type_method_hint tpconst dict_method_hints[] = {
	TYPE_METHOD_HINT_F(seq_clear, &dict_mh_clear, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(map_setold, &dict_mh_setold, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(map_setnew, &dict_mh_setnew, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(map_setold_ex, &dict_mh_setold_ex, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(map_setnew_ex, &dict_mh_setnew_ex, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(map_setdefault, &dict_mh_setdefault, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(map_pop, &dict_mh_pop, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(map_pop_with_default, &dict_mh_pop_with_default, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(map_popitem, &dict_mh_popitem, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(map_remove, &dict_mh_remove, METHOD_FNOREFESCAPE),

	/* Operators for "Dict as Sequence" (specifically defined because dicts are ordered) */
	TYPE_METHOD_HINT_F(seq_foreach_reverse, &dict_mh_seq_foreach_reverse, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_enumerate_index_reverse, &dict_mh_seq_enumerate_index_reverse, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_trygetfirst, &dict_trygetfirst, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_trygetlast, &dict_trygetlast, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(set_trygetfirst, &dict_trygetfirst, METHOD_FNOREFESCAPE), /* Must also be set because the "first" getset defines __seq_first__ and __set_first__ */
	TYPE_METHOD_HINT_F(set_trygetlast, &dict_trygetlast, METHOD_FNOREFESCAPE),   /* Must also be set because the "last" getset defines __seq_last__ and __set_last__ */
	TYPE_METHOD_HINT_F(seq_enumerate_index, &dict_mh_seq_enumerate_index, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_operator_iter, &dict_iter, METHOD_FNORMAL),
	TYPE_METHOD_HINT_F(seq_operator_foreach, &dict_mh_seq_foreach, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_operator_foreach_pair, &dict_foreach_pair, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_operator_hasitem_index, &default__seq_operator_hasitem_index__with__seq_operator_size, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_operator_getitem_index, &dict_mh_seq_getitem_index, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_operator_trygetitem_index, &dict_mh_seq_trygetitem_index, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_operator_delitem_index, &dict_mh_seq_delitem_index, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_operator_setitem_index, &dict_mh_seq_setitem_index, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_operator_compare_eq, &dict_mh_seq_compare_eq, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_operator_compare, &dict_mh_seq_compare, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_operator_trycompare_eq, &dict_mh_seq_trycompare_eq, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_xchitem_index, &dict_mh_seq_xchitem_index, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_erase, &dict_mh_seq_erase, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_insert, &dict_mh_seq_insert, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_pushfront, &dict_mh_seq_pushfront, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_append, &dict_mh_seq_append, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_pop, &dict_mh_seq_pop, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_removeif, &dict_mh_seq_removeif, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_reverse, &dict_mh_seq_reverse, METHOD_FNOREFESCAPE),
//TODO:	TYPE_METHOD_HINT_F(seq_sort, &dict_mh_sort, METHOD_FNOREFESCAPE),
//TODO:	TYPE_METHOD_HINT_F(seq_sort_with_key, &dict_mh_sort_with_key, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_END
};

PRIVATE struct type_method tpconst dict_class_methods[] = {
	TYPE_KWMETHOD_F("fromkeys", &dict_fromkeys_f, METHOD_FNOREFESCAPE,
	                "(" dict_fromkeys_params ")->?.\n"
	                "Construct a new ?. from @keys, and @value (or ${valuefor(key)}) as value."),
	TYPE_METHOD_END
};

PRIVATE struct type_operator const dict_operators[] = {
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
	TYPE_OPERATOR_FLAGS(OPERATOR_0032_GETITEM, METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0033_DELITEM, METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0034_SETITEM, METHOD_FNOREFESCAPE),
};

/* The main `Dict' container class */
PUBLIC DeeTypeObject DeeDict_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ DeeString_STR(&str_Dict),
	/* .tp_doc      = */ DOC("The builtin ?DMapping object for translating keys to items: ${{Key: Value}} (or ${{(Key, Value)...}})\n"
	                         "${"
	                         /**/ "import Dict, Object from deemon;\n"
	                         /**/ "\\\n"
	                         /**/ "local d: {Object: Object} = Dict();\n"
	                         /**/ "d[\"first\"] = 10;\n"
	                         /**/ "d[\"second\"] = 20;\n"
	                         /**/ "d[true] = \"yes\";\n"
	                         /**/ "\\\n"
	                         /**/ "/* Dict({ \"first\": 10, \"second\": 10, true: \"yes\" }) */\n"
	                         /**/ "print repr d;"
	                         "}\n"
	                         "Dicts also retain the order in which items are inserted, such that during "
	                         /**/ "enumeration, key-value pairs (aka. items) are enumerated from least-recently, "
	                         /**/ "to most-recently inserted.\n"
	                         "In order to easier control the order of items, certain ?DSequence functions are "
	                         /**/ "also implemented, such as ?#__seq_insert__ or ?#__seq_erase__. These should "
	                         /**/ "not be called directly, and can instead be used as ${this.insert(0, (key, value))} "
	                         /**/ "or ${(this as Sequence).insert(0, (key, value))}.\n"
	                         "\n"

	                         "()\n"
	                         "Create a new, empty ?.\n"
	                         "\n"

	                         "(hint:?Dint,weak=!f)\n"
	                         "#pweak{When true, @hint represents a lower-bound guess for how many items to allocate. "
	                         /*  */ "In this case, then runtime may allocate much more space than needed if "
	                         /*  */ "doing so would be appropriate for the hash-mask appropriate for @hint}"
	                         "Create a new ?., while trying to pre-alloc enough space for @hint items. When @hint is "
	                         /**/ "too large to pre-allocate a buffer of sufficient size, a smaller buffer, or no buffer "
	                         /**/ "at all may be pre-allocated.\n"
	                         "\n"

	                         "(items:?S" D_TItem ")\n"
	                         "(items:?M" D_TKey D_TValue ")\n"
	                         "Create a new ?., using key-value pairs extracted from @items.\n"
	                         "Iterate @items and unpack each element into 2 others, using them "
	                         /**/ "as key and value to insert into @this ?.\n"
	                         "\n"

	                         ":=(items:?S" D_TItem ")->\n"
	                         ":=(items:?M" D_TKey D_TValue ")->\n"
	                         "Replace the contents of @this ?. with @items\n"
	                         "\n"

	                         "move:=(other:?.)->\n"
	                         "Similar to ?#{op:assign}, but also clear @other at the same time\n"
	                         "\n"

	                         "iter->\n"
	                         "Enumerate key-value pairs stored in the ?., in order of being added\n"
	                         "\n"

	                         "#->\n"
	                         "Return the number of key-value pairs\n"
	                         "\n"

	                         "contains(key:" D_TKey ")->\n"
	                         "Check if the ?. contains a @key\n"
	                         "\n"

	                         "[](key:" D_TKey ")->" D_TValue "\n"
	                         "#tKeyError{Given @key doesn't exist}\n"
	                         "Return the value associated with @key\n"
	                         "\n"

	                         "del[](key:" D_TKey ")->\n"
	                         "Remove @key from @this. No-op if @key doesn't exist (s.a. ?#remove)\n"
	                         "\n"

	                         "[]=(key:" D_TKey ",value:" D_TValue ")->\n"
	                         "Insert/override @key by assigning @value"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FGC | TP_FNAMEOBJECT,
	/* .tp_weakrefs = */ Dee_WEAKREF_SUPPORT_ADDR(Dict),
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeMap_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED_GC(
			/* T:              */ Dict,
			/* tp_ctor:        */ &dict_ctor,
			/* tp_copy_ctor:   */ &dict_copy,
			/* tp_any_ctor:    */ &dict_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &dict_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&dict_fini,
		/* .tp_assign      = */ (int (DCALL *)(DeeObject *, DeeObject *))&dict_assign,
		/* .tp_move_assign = */ (int (DCALL *)(DeeObject *, DeeObject *))&dict_moveassign,
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ DEFIMPL(&object_str),
		/* .tp_repr      = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool      = */ (int (DCALL *)(DeeObject *__restrict))&dict_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&dict_printrepr,
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&dict_visit,
	/* .tp_gc            = */ &dict_gc,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__E66FA6851AAFE176),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__4CF94EE41850B0EF), /* TODO */
	/* .tp_seq           = */ &dict_seq,
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__C6F8E138F179B5AD),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ dict_methods,
	/* .tp_getsets       = */ dict_getsets,
	/* .tp_members       = */ dict_members,
	/* .tp_class_methods = */ dict_class_methods,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ dict_class_members,
	/* .tp_method_hints  = */ dict_method_hints,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
	/* .tp_mro           = */ NULL,
	/* .tp_operators     = */ dict_operators,
	/* .tp_operators_size= */ COMPILER_LENOF(dict_operators),
};

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_DICT_C */
