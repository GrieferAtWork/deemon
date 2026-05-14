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
#ifndef GUARD_DEEMON_OBJECTS_ROSET_C
#define GUARD_DEEMON_OBJECTS_ROSET_C 1

#include <deemon/api.h>

#include <deemon/alloc.h>              /* DeeObject_MALLOC, Dee_TYPE_CONSTRUCTOR_INIT_FIXED */
#include <deemon/arg.h>                /* DeeArg_UnpackStruct1XOr2X, UNPuSIZ, _DeeArg_AsObject */
#include <deemon/bool.h>               /* Dee_True, return_bool */
#include <deemon/computed-operators.h> /* DEFIMPL, DEFIMPL_UNSUPPORTED */
#include <deemon/dict.h>               /* DeeDictObject, DeeDict_LockEndRead, Dee_dict_item, _DeeDict_GetRealVTab */
#include <deemon/error-rt.h>           /* DeeRT_ErrIndexOutOfBounds, DeeRT_ErrUnboundAttr */
#include <deemon/format.h>             /* PRFuSIZ, PRFxSIZ */
#include <deemon/hashset.h>            /* Dee_hashset_item */
#include <deemon/int.h>                /* DeeInt_NewSize */
#include <deemon/method-hints.h>       /* Dee_seq_enumerate_index_t, TYPE_METHOD_HINT*, type_method_hint */
#include <deemon/object.h>             /* DREF, DeeObject, DeeObject_*, DeeTypeObject, Dee_AsObject, Dee_COMPARE_*, Dee_Incref, Dee_TYPE, Dee_foreach_t, Dee_formatprinter_t, Dee_hash_t, Dee_ssize_t, ITER_DONE, OBJECT_HEAD_INIT, return_reference, return_reference_ */
#include <deemon/operator-hints.h>     /* DeeType_HasNativeOperator */
#include <deemon/rodict.h>             /* DeeRoDictObject, _DeeRoDict_GetRealVTab */
#include <deemon/roset.h>              /* DeeRoSet*, _DeeRoSet_GetRealVTab, _DeeRoSet_GetVirtVTab, _DeeRoSet_HashIdxInit, _DeeRoSet_HashIdxNext */
#include <deemon/seq.h>                /* DeeIterator_Type */
#include <deemon/serial.h>             /* DeeSerial, Dee_seraddr_t */
#include <deemon/set.h>                /* DeeSet_Type */
#include <deemon/system-features.h>    /* memcpy, memset */
#include <deemon/type.h>               /* DeeObject_InitStatic, DeeType_Type, Dee_TYPE_CONSTRUCTOR_INIT_FIXED, Dee_TYPE_CONSTRUCTOR_INIT_VAR, Dee_visit_t, METHOD_F*, OPERATOR_*, STRUCT_*, TF_NONE, TP_F*, TYPE_*, type_* */
#include <deemon/util/atomic.h>        /* atomic_cmpxch_or_write, atomic_read */
#include <deemon/util/hash-io.h>       /* Dee_HASH_*, Dee_SIZEOF_HASH_VIDX_T, Dee_hash_*, IF_Dee_HASH_HIDXIO_COUNT_GE_* */

#include <hybrid/align.h>    /* IS_POWER_OF_TWO */
#include <hybrid/typecore.h> /* __BYTE_TYPE__ */

#include "../runtime/method-hint-defaults.h"
#include "../runtime/strings.h"
#include "dict-utils.h"
#include "generic-proxy.h"
#include "rodict.h"
#include "roset.h"

#include <stddef.h> /* NULL, offsetof, size_t */

#undef byte_t
#define byte_t __BYTE_TYPE__

DECL_BEGIN

#ifndef NDEBUG
#define DBG_memset (void)memset
#else /* !NDEBUG */
#define DBG_memset(dst, byte, n_bytes) (void)0
#endif /* NDEBUG */

typedef DeeRoSetObject RoSet;

#define EDO(err, expr)                   \
	do {                                 \
		if unlikely((temp = (expr)) < 0) \
			goto err;                    \
		result += temp;                  \
	}	__WHILE0


/************************************************************************/
/* RODICT HELPERS                                                       */
/************************************************************************/


#ifdef DICT_NDEBUG
#define roset_verify(self) (void)0
#else /* DICT_NDEBUG */
PRIVATE ATTR_NOINLINE NONNULL((1)) void DCALL
roset_verify(RoSet *__restrict self) {
	size_t i, real_vused;
	Dee_hash_hidxio_t hidxio;
	ASSERT(self->rs_vsize <= self->rs_hmask);
	ASSERT(IS_POWER_OF_TWO(self->rs_hmask + 1));
	ASSERT(self->rs_htab == (union Dee_hash_htab *)(_DeeRoSet_GetRealVTab(self) + self->rs_vsize));
	hidxio = Dee_HASH_HIDXIO_FROM_VALLOC(self->rs_vsize);
	ASSERT(/*hidxio >= 0 &&*/ hidxio < Dee_HASH_HIDXIO_COUNT);
	/* hidxio==0 may differ if "self" was statically initialized in a dex module,
	 * in which case `self->rs_hidxget' might point into that module's PLT/GOT. */
	ASSERT(self->rs_hidxget == Dee_hash_hidxio[hidxio].hxio_get || hidxio == 0);
	for (i = Dee_hash_vidx_tovirt(0), real_vused = 0;
	     Dee_hash_vidx_virt_lt_real(i, self->rs_vsize); ++i) {
		struct Dee_hashset_item *item = &_DeeRoSet_GetVirtVTab(self)[i];
		if (item->hsi_key) {
#if 0 /* Cannot be asserted -- we might get here from "tp_visit", which can screw with reference counts */
			ASSERT_OBJECT(item->hsi_key);
#endif
			++real_vused;
		}
	}
	ASSERTF(real_vused == self->rs_vsize,
	        "RODICT: vtab key count=%" PRFuSIZ " differs from rs_vsize=%" PRFuSIZ,
	        real_vused, self->rs_vsize);
	for (i = 0; i <= self->rs_hmask; ++i) {
		Dee_hash_vidx_t vidx;
		vidx = (*self->rs_hidxget)(self->rs_htab, i);
		if (vidx == Dee_HASH_HTAB_EOF)
			continue;
		Dee_hash_vidx_virt2real(&vidx);
		ASSERTF(vidx < self->rs_vsize,
		        "RODICT: htab[%" PRFuSIZ "] points out-of-bounds: %" PRFuSIZ " >= %" PRFuSIZ,
		        i, vidx, self->rs_vsize);
	}
	for (i = 0;; ++i) {
		Dee_hash_vidx_t vtab_idx;
		ASSERTF(i <= self->rs_hmask,
		        "RODICT: htab contains no EOF pointers (infinite "
		        "loop would occur on non-present item lookup)");
		vtab_idx = (*self->rs_hidxget)(self->rs_htab, i);
		if (vtab_idx == Dee_HASH_HTAB_EOF)
			break;
	}
	for (i = Dee_hash_vidx_tovirt(0), real_vused = 0;
	     Dee_hash_vidx_virt_lt_real(i, self->rs_vsize); ++i) {
		Dee_hash_t hs, perturb;
		struct Dee_hashset_item *item = &_DeeRoSet_GetVirtVTab(self)[i];
		if (!item->hsi_key)
			continue;
		for (_DeeRoSet_HashIdxInit(self, &hs, &perturb, item->hsi_hash);;
		     _DeeRoSet_HashIdxNext(self, &hs, &perturb, item->hsi_hash)) {
			struct Dee_hashset_item *hitem;
			Dee_hash_hidx_t htab_idx = Dee_hash_hidx_ofhash(hs, self->rs_hmask);
			Dee_hash_vidx_t vtab_idx = (*self->rs_hidxget)(self->rs_htab, htab_idx); /*virt*/
			ASSERTF(vtab_idx != Dee_HASH_HTAB_EOF,
			        "RODICT: End-of-hash-chain[hash:%#" PRFxSIZ "] before item "
			        "idx=%" PRFuSIZ ",count=%" PRFuSIZ " <%r> was found",
			        item->hsi_hash, Dee_hash_vidx_toreal(i),
			        self->rs_vsize, item->hsi_key);
			hitem = &_DeeRoSet_GetVirtVTab(self)[vtab_idx];
			if (hitem == item)
				break;
		}
	}
}
#endif /* !DICT_NDEBUG */

/************************************************************************/
/* ROSET ITERATOR                                                      */
/************************************************************************/

STATIC_ASSERT(offsetof(RoSetIterator, rosi_set) == offsetof(ProxyObject, po_obj));
#define rsiter_fini      generic_proxy__fini
#define rsiter_visit     generic_proxy__visit
#define rsiter_serialize generic_proxy__serialize_and_wordcopy_atomic(Dee_SIZEOF_HASH_VIDX_T)

STATIC_ASSERT(offsetof(RoSetIterator, rosi_set) == offsetof(RoDictIterator, rodi_dict));
STATIC_ASSERT(offsetof(RoSetIterator, rosi_vidx) == offsetof(RoDictIterator, rodi_vidx));
INTDEF WUNUSED NONNULL((1, 2)) int DCALL rditer_copy(RoDictIterator *__restrict self, RoDictIterator *__restrict other);
#define rsiter_copy rditer_copy

STATIC_ASSERT(offsetof(RoSetIterator, rosi_set) == offsetof(RoDictIterator, rodi_dict));
STATIC_ASSERT(offsetof(RoSetIterator, rosi_vidx) == offsetof(RoDictIterator, rodi_vidx));
STATIC_ASSERT(offsetof(RoSet, rs_vsize) == offsetof(DeeRoDictObject, rd_vsize));
INTDEF WUNUSED NONNULL((1)) int DCALL rditer_bool(RoDictIterator *__restrict self);
#define rsiter_bool rditer_bool

STATIC_ASSERT(offsetof(RoSetIterator, rosi_set) == offsetof(RoDictIterator, rodi_dict));
STATIC_ASSERT(offsetof(RoSetIterator, rosi_vidx) == offsetof(RoDictIterator, rodi_vidx));
INTDEF WUNUSED NONNULL((1, 2)) int DCALL rditer_compare(RoDictIterator *__restrict lhs, RoDictIterator *__restrict rhs);
#define rsiter_compare rditer_compare

STATIC_ASSERT(offsetof(RoSetIterator, rosi_set) == offsetof(RoDictIterator, rodi_dict));
STATIC_ASSERT(offsetof(RoSetIterator, rosi_vidx) == offsetof(RoDictIterator, rodi_vidx));
STATIC_ASSERT(offsetof(RoSet, rs_vsize) == offsetof(DeeRoDictObject, rd_vsize));
INTDEF WUNUSED NONNULL((1)) size_t DCALL rditer_advance(RoDictIterator *__restrict self, size_t step);
#define rsiter_advance rditer_advance


PRIVATE WUNUSED NONNULL((1)) int DCALL
rsiter_ctor(RoSetIterator *__restrict self) {
	self->rosi_set = (DREF RoSet *)DeeRoSet_NewEmpty();
	self->rosi_vidx = 0;
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
rsiter_init(RoSetIterator *__restrict self, size_t argc, DeeObject *const *argv) {
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("_RoSetIterator", params: "
	DeeRoSetObject *set;
	Dee_hash_vidx_t index = 0
", docStringPrefix: "rsiter");]]]*/
#define rsiter__RoSetIterator_params "set:?Ert:RoSet,index=!0"
	struct {
		DeeRoSetObject *set;
		Dee_hash_vidx_t index;
	} args;
	args.index = 0;
	DeeArg_UnpackStruct1XOr2X(err, argc, argv, "_RoSetIterator", &args, &args.set, "o", _DeeArg_AsObject, &args.index, UNPuSIZ, DeeObject_AsSize);
/*[[[end]]]*/
	if (DeeObject_AssertTypeExact(args.set, &DeeRoSet_Type))
		goto err;
	Dee_Incref(args.set);
	self->rosi_set  = args.set;
	self->rosi_vidx = args.index;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rsiter_next(RoSetIterator *__restrict self) {
	RoSet *set = self->rosi_set;
	/*real*/Dee_hash_vidx_t vidx;
	struct Dee_hashset_item *item;
	do {
		vidx = atomic_read(&self->rosi_vidx);
		if (vidx >= set->rs_vsize)
			return ITER_DONE;
	} while (!atomic_cmpxch_or_write(&self->rosi_vidx, vidx, vidx + 1));
	item = &_DeeRoSet_GetRealVTab(set)[vidx];
	return_reference_(item->hsi_key);
}

PRIVATE struct type_cmp rsiter_cmp = {
	/* .tp_hash          = */ DEFIMPL_UNSUPPORTED(&default__hash__unsupported),
	/* .tp_compare_eq    = */ (int (DCALL *)(DeeObject *, DeeObject *))&rsiter_compare,
	/* .tp_compare       = */ (int (DCALL *)(DeeObject *, DeeObject *))&rsiter_compare,
	/* .tp_trycompare_eq = */ DEFIMPL(&default__trycompare_eq__with__compare_eq),
	/* .tp_eq            = */ DEFIMPL(&default__eq__with__compare_eq),
	/* .tp_ne            = */ DEFIMPL(&default__ne__with__compare_eq),
	/* .tp_lo            = */ DEFIMPL(&default__lo__with__compare),
	/* .tp_le            = */ DEFIMPL(&default__le__with__compare),
	/* .tp_gr            = */ DEFIMPL(&default__gr__with__compare),
	/* .tp_ge            = */ DEFIMPL(&default__ge__with__compare),
};

#if 1
INTDEF struct type_method tpconst rditer_methods[]; /* "INTER" because shared with "./roset.c" */
#define rsiter_methods rditer_methods
#else
PRIVATE struct type_method tpconst rsiter_methods[] = {
	TYPE_METHOD_HINTREF(Iterator_advance),
	TYPE_METHOD_END
};
#endif

PRIVATE struct type_method_hint tpconst rsiter_method_hints[] = {
	TYPE_METHOD_HINT(iter_advance, &rsiter_advance),
	TYPE_METHOD_HINT_END
};

PRIVATE struct type_member tpconst rsiter_members[] = {
	TYPE_MEMBER_FIELD_DOC(STR_seq, STRUCT_OBJECT_AB, offsetof(RoSetIterator, rosi_set), "->?Ert:RoSet"),
	TYPE_MEMBER_FIELD_DOC("__index__", STRUCT_SIZE_T | STRUCT_ATOMIC, offsetof(RoSetIterator, rosi_vidx), "->?Dint"),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject RoSetIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_RoSetIterator",
	/* .tp_doc      = */ DOC("(" rsiter__RoSetIterator_params ")"),
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ RoSetIterator,
			/* tp_ctor:        */ &rsiter_ctor,
			/* tp_copy_ctor:   */ &rsiter_copy,
			/* tp_any_ctor:    */ &rsiter_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &rsiter_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&rsiter_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&rsiter_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&iterator_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&rsiter_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__EFED4BCD35433C3C),
	/* .tp_cmp           = */ &rsiter_cmp,
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&rsiter_next,
	/* .tp_iterator      = */ DEFIMPL(&default__tp_iterator__712535FF7E4C26E5),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ rsiter_methods,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ rsiter_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ rsiter_method_hints,
	/* .tp_call          = */ DEFIMPL(&iterator_next),
	/* .tp_callable      = */ DEFIMPL(&default__tp_callable__83C59FA7626CABBE),
};







/************************************************************************/
/* ROSET                                                                */
/************************************************************************/

/* Can re-use some operators from "RoDict" because "rs_vsize" and "rd_vsize" share the same offset. */
STATIC_ASSERT(offsetof(DeeRoSetObject, rs_vsize) == offsetof(DeeRoDictObject, rd_vsize));
INTDEF WUNUSED NONNULL((1)) int DCALL rodict_bool(DeeRoDictObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) size_t DCALL rodict_size(DeeRoDictObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) int DCALL rodict_nonempty_as_bound(DeeRoDictObject *__restrict self);
INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL rodict___hidxio__(DeeRoDictObject *__restrict self);
#define roset_bool              rodict_bool
#define roset_size_fast         rodict_size
#define roset_size              rodict_size
#define roset_nonempty_as_bound rodict_nonempty_as_bound
#define roset___hidxio__        rodict___hidxio__



PRIVATE WUNUSED NONNULL((1)) DREF RoSetIterator *DCALL
roset_iter(RoSet *__restrict self) {
	DREF RoSetIterator *result;
	result = DeeObject_MALLOC(RoSetIterator);
	if unlikely(!result)
		goto err;
	result->rosi_set = self;
	Dee_Incref(self);
	result->rosi_vidx = 0;
	DeeObject_InitStatic(result, &RoSetIterator_Type);
	return result;
err:
	return NULL;
}


PRIVATE WUNUSED NONNULL((1)) DREF RoSet *DCALL
roset_from_dict_keys(/*HashSet*/ DeeDictObject *__restrict dict_keys) {
	DREF RoSet *result;
	size_t sizeof_result;
	size_t vsize;
	size_t i, hmask;
	Dee_hash_hidxio_t src_hidxio;
	Dee_hash_hidxio_t dst_hidxio;
again:
	DeeDict_LockReadAndOptimize(dict_keys);
	vsize         = dict_keys->d_vused;
	hmask         = dict_keys->d_hmask;
	src_hidxio    = Dee_HASH_HIDXIO_FROM_VALLOC(dict_keys->d_valloc);
	dst_hidxio    = Dee_HASH_HIDXIO_FROM_VALLOC(vsize);
	sizeof_result = _RoSet_SizeOf3(vsize, hmask, dst_hidxio);
	result = _RoSet_TryMalloc(sizeof_result);
	if unlikely(!result) {
		DeeDict_LockEndRead(dict_keys);
		result = _RoSet_Malloc(sizeof_result);
		if unlikely(!result)
			goto err;
		DeeDict_LockReadAndOptimize(dict_keys);
		if unlikely(vsize != dict_keys->d_vused ||
		            hmask != dict_keys->d_hmask) {
			DeeDict_LockEndRead(dict_keys);
			_RoSet_Free(result);
			goto again;
		}
	}
	/* Copy over data as-is from the dict (no need to rehash or anything). */
	result->rs_htab = (union Dee_hash_htab *)(_DeeRoSet_GetRealVTab(result) + vsize);
	for (i = 0; i < vsize; ++i) {
		struct Dee_hashset_item *dst;
		struct Dee_dict_item const *src;
		dst = &_DeeRoSet_GetRealVTab(result)[i];
		src = &_DeeDict_GetRealVTab(dict_keys)[i];
		dst->hsi_hash = src->di_hash;
		dst->hsi_key  = src->di_key;
		Dee_Incref(dst->hsi_key);
	}
	hmask_memcpy_and_maybe_downcast(result->rs_htab, dst_hidxio,
	                                dict_keys->d_htab, src_hidxio,
	                                hmask + 1);
	DeeDict_LockEndRead(dict_keys);
	result->rs_vsize   = vsize;
	result->rs_hmask   = hmask;
	result->rs_hidxget = Dee_hash_hidxio[dst_hidxio].hxio_get;
	DeeObject_InitStatic(result, &DeeRoSet_Type);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF RoSet *DCALL
roset_from_rodict_keys(/*HashSet*/ DeeRoDictObject *__restrict dict_keys) {
	DREF RoSet *result;
	size_t i, sizeof_result;
	Dee_hash_hidxio_t hidxio;
	union Dee_hash_htab *final_htab;
	hidxio        = Dee_HASH_HIDXIO_FROM_VALLOC(dict_keys->rd_vsize);
	sizeof_result = _RoSet_SizeOf3(dict_keys->rd_vsize, dict_keys->rd_hmask, hidxio);
	result = _RoSet_Malloc(sizeof_result);
	if unlikely(!result)
		goto err;
	result->rs_vsize   = dict_keys->rd_vsize;
	result->rs_hmask   = dict_keys->rd_hmask;
	result->rs_hidxget = dict_keys->rd_hidxget;

	/* Copy htab */
	final_htab = (union Dee_hash_htab *)(_DeeRoSet_GetRealVTab(result) + result->rs_vsize);
	final_htab = (union Dee_hash_htab *)memcpy(final_htab, dict_keys->rd_htab, (result->rs_hmask + 1) << hidxio);
	result->rs_htab = final_htab;

	/* Copy items (keys-only) */
	for (i = 0; i < result->rs_vsize; ++i) {
		struct Dee_hashset_item *dst;
		struct Dee_dict_item const *src;
		dst = &_DeeRoSet_GetRealVTab(result)[i];
		src = &_DeeRoDict_GetRealVTab(dict_keys)[i];
		dst->hsi_hash = src->di_hash;
		dst->hsi_key  = src->di_key;
		Dee_Incref(dst->hsi_key);
	}
	DeeObject_InitStatic(result, &DeeRoSet_Type);
	return result;
err:
	return NULL;
}



#ifdef __INTELLISENSE__
PRIVATE WUNUSED DREF RoSet *DCALL roset_ctor(void);
PRIVATE WUNUSED DREF RoSet *DCALL roset_init(size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) void DCALL roset_fini(RoSet *__restrict self);
PRIVATE NONNULL((1, 2)) void DCALL roset_visit(RoSet *__restrict self, Dee_visit_t proc, void *arg);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_seraddr_t DCALL roset_serialize(RoSet *__restrict self, DeeSerial *__restrict writer);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL roset_printrepr(RoSet *__restrict self, Dee_formatprinter_t printer, void *arg);
#else /* __INTELLISENSE__ */
DECL_END
#define DEFINE_DeeRoSet
#include "rodict-impl.c.inl"
DECL_BEGIN
#endif /* !__INTELLISENSE__ */

/* Key-contains operators. */
PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL roset_contains(RoSet *self, DeeObject *key);
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL roset_mh_contains(RoSet *self, DeeObject *key);
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL roset_mh_contains_with_range(RoSet *self, DeeObject *key, size_t start, size_t end);
PRIVATE WUNUSED NONNULL((1, 2)) size_t DCALL roset_mh_find(RoSet *self, DeeObject *key, size_t start, size_t end);

#ifdef __OPTIMIZE_SIZE__
#define SUBSTITUDE_roset_contains
#define SUBSTITUDE_roset_mh_contains
#define SUBSTITUDE_roset_mh_contains_with_range
#endif /* __OPTIMIZE_SIZE__ */

#ifndef __INTELLISENSE__
DECL_END
#define DEFINE_roset_mh_find
#include "rodict-getitem.c.inl"
#ifndef SUBSTITUDE_roset_mh_contains
#define DEFINE_roset_mh_contains
#include "rodict-getitem.c.inl"
#endif /* !SUBSTITUDE_roset_mh_contains */
#ifndef SUBSTITUDE_roset_mh_contains_with_range
#define DEFINE_roset_mh_contains_with_range
#include "rodict-getitem.c.inl"
#endif /* !SUBSTITUDE_roset_mh_contains_with_range */
#ifndef SUBSTITUDE_roset_contains
#define DEFINE_roset_contains
#include "rodict-getitem.c.inl"
#endif /* !SUBSTITUDE_roset_contains */
DECL_BEGIN
#endif /* !__INTELLISENSE__ */

#ifdef SUBSTITUDE_roset_mh_contains_with_range
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
roset_mh_contains_with_range(RoSet *self, DeeObject *key, size_t start, size_t end) {
	size_t index = roset_mh_find(self, key, start, end);
	if unlikely(index == (size_t)Dee_COMPARE_ERR)
		goto err;
	return index != (size_t)-1 ? 1 : 0;
err:
	return -1;
}
#endif /* SUBSTITUDE_roset_mh_contains_with_range */

#ifdef SUBSTITUDE_roset_mh_contains
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
roset_mh_contains(RoSet *self, DeeObject *key) {
	return roset_mh_contains_with_range(self, key, 0, (size_t)-1);
}
#endif /* SUBSTITUDE_roset_mh_contains */

#ifdef SUBSTITUDE_roset_contains
PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
roset_contains(RoSet *self, DeeObject *key) {
	int result = roset_mh_contains(self, key);
	if unlikely(result < 0)
		goto err;
	return_bool(result);
err:
	return NULL;
}
#endif /* SUBSTITUDE_roset_contains */


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
roset_getfirst(RoSet *__restrict self) {
	struct Dee_hashset_item *item;
	if unlikely(DeeRoSet_IsEmpty(self))
		goto err_unbound;
	item = _DeeRoSet_GetRealVTab(self);
	return_reference(item->hsi_key);
err_unbound:
	return DeeRT_ErrUnboundAttr(self, &str_first);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
roset_getlast(RoSet *__restrict self) {
	struct Dee_hashset_item *item;
	if unlikely(DeeRoSet_IsEmpty(self))
		goto err_unbound;
	item = _DeeRoSet_GetRealVTab(self);
	item += self->rs_vsize - 1;
	return_reference(item->hsi_key);
err_unbound:
	return DeeRT_ErrUnboundAttr(self, &str_last);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
roset_trygetfirst(RoSet *__restrict self) {
	struct Dee_hashset_item *item;
	if unlikely(DeeRoSet_IsEmpty(self))
		return ITER_DONE;
	item = _DeeRoSet_GetRealVTab(self);
	return_reference(item->hsi_key);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
roset_trygetlast(RoSet *__restrict self) {
	struct Dee_hashset_item *item;
	if unlikely(DeeRoSet_IsEmpty(self))
		return ITER_DONE;
	item = _DeeRoSet_GetRealVTab(self);
	item += self->rs_vsize - 1;
	return_reference(item->hsi_key);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
roset_sizeof(RoSet *__restrict self) {
	size_t result = _RoSet_SizeOf(self->rs_vsize, self->rs_hmask);
	return DeeInt_NewSize(result);
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
roset_mh_seq_foreach(RoSet *__restrict self, Dee_foreach_t cb, void *arg) {
	Dee_hash_vidx_t i;
	Dee_ssize_t temp, result = 0;
	for (i = 0; i < self->rs_vsize; ++i) {
		struct Dee_hashset_item *item;
		item = &_DeeRoSet_GetRealVTab(self)[i];
		temp = (*cb)(arg, item->hsi_key);
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
	}
	return result;
err_temp:
	return temp;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
roset_mh_seq_foreach_reverse(RoSet *__restrict self, Dee_foreach_t cb, void *arg) {
	Dee_hash_vidx_t i;
	Dee_ssize_t temp, result = 0;
	i = self->rs_vsize;
	while (i) {
		struct Dee_hashset_item *item;
		--i;
		item = &_DeeRoSet_GetRealVTab(self)[i];
		temp = (*cb)(arg, item->hsi_key);
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
	}
	return result;
err_temp:
	return temp;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
roset_mh_seq_enumerate_index(RoSet *__restrict self, Dee_seq_enumerate_index_t cb,
                              void *arg, size_t start, size_t end) {
	Dee_ssize_t temp, result = 0;
	if (end > self->rs_vsize)
		end = self->rs_vsize;
	while (start < end) {
		struct Dee_hashset_item *item;
		item = &_DeeRoSet_GetRealVTab(self)[start];
		temp = (*cb)(arg, start, item->hsi_key);
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
roset_mh_seq_enumerate_index_reverse(RoSet *__restrict self, Dee_seq_enumerate_index_t cb,
                                      void *arg, size_t start, size_t end) {
	Dee_ssize_t temp, result = 0;
	if (end > self->rs_vsize)
		end = self->rs_vsize;
	while (start < end) {
		struct Dee_hashset_item *item;
		--end;
		item = &_DeeRoSet_GetRealVTab(self)[end];
		temp = (*cb)(arg, end, item->hsi_key);
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
	}
	return result;
err_temp:
	return temp;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
roset_mh_seq_getitem_index(RoSet *__restrict self, size_t index) {
	struct Dee_hashset_item *item;
	if unlikely(index >= self->rs_vsize)
		goto err_oob;
	item = &_DeeRoSet_GetRealVTab(self)[index];
	return_reference(item->hsi_key);
err_oob:
	DeeRT_ErrIndexOutOfBounds(Dee_AsObject(self), index, self->rs_vsize);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
roset_mh_seq_trygetitem_index(RoSet *__restrict self, size_t index) {
	if (index >= self->rs_vsize)
		return ITER_DONE;
	return roset_mh_seq_getitem_index(self, index);
}


struct roset_compare_seq_foreach_data {
	RoSet                  *rscsfd_lhs;   /* [1..1] lhs-set. */
	/*real*/Dee_hash_vidx_t rscsfd_index; /* Next index into "rscsfd_lhs" to compare against. */
};
#define ROSET_COMPARE_SEQ_FOREACH_ERROR    (-1)
#define ROSET_COMPARE_SEQ_FOREACH_EQUAL    (0)
#define ROSET_COMPARE_SEQ_FOREACH_NOTEQUAL (-2)
#define ROSET_COMPARE_SEQ_FOREACH_LESS     (-2)
#define ROSET_COMPARE_SEQ_FOREACH_GREATER  (-3)

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
roset_compare_seq_foreach(void *arg, DeeObject *rhs_item) {
	RoSet *set;
	int cmp_result;
	struct Dee_hashset_item *lhs_item;
	struct roset_compare_seq_foreach_data *data;
	data = (struct roset_compare_seq_foreach_data *)arg;
	set = data->rscsfd_lhs;
	if unlikely(data->rscsfd_index >= set->rs_vsize)
		return ROSET_COMPARE_SEQ_FOREACH_LESS;
	lhs_item = &_DeeRoSet_GetRealVTab(set)[data->rscsfd_index];
	cmp_result = DeeObject_Compare(lhs_item->hsi_key, rhs_item);
	if (Dee_COMPARE_ISERR(cmp_result))
		goto err;
	++data->rscsfd_index;
	if (Dee_COMPARE_ISLO(cmp_result))
		return ROSET_COMPARE_SEQ_FOREACH_LESS;
	if (Dee_COMPARE_ISGR(cmp_result))
		return ROSET_COMPARE_SEQ_FOREACH_GREATER;
	return ROSET_COMPARE_SEQ_FOREACH_EQUAL;
err:
	return ROSET_COMPARE_SEQ_FOREACH_ERROR;
}

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
roset_compare_eq_seq_foreach(void *arg, DeeObject *rhs_item) {
	RoSet *set;
	int cmp_result;
	struct Dee_hashset_item *lhs_item;
	struct roset_compare_seq_foreach_data *data;
	data = (struct roset_compare_seq_foreach_data *)arg;
	set = data->rscsfd_lhs;
	if unlikely(data->rscsfd_index >= set->rs_vsize)
		return ROSET_COMPARE_SEQ_FOREACH_NOTEQUAL;
	lhs_item = &_DeeRoSet_GetRealVTab(set)[data->rscsfd_index];
	cmp_result = DeeObject_TryCompareEq(lhs_item->hsi_key, rhs_item);
	if (Dee_COMPARE_ISERR(cmp_result))
		goto err;
	++data->rscsfd_index;
	if (Dee_COMPARE_ISNE(cmp_result))
		return ROSET_COMPARE_SEQ_FOREACH_NOTEQUAL;
	return ROSET_COMPARE_SEQ_FOREACH_EQUAL;
err:
	return ROSET_COMPARE_SEQ_FOREACH_ERROR;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
roset_mh_seq_compare(RoSet *lhs, DeeObject *rhs) {
	Dee_ssize_t foreach_status;
	struct roset_compare_seq_foreach_data data;
	data.rscsfd_index = 0;
	data.rscsfd_lhs   = lhs;
	foreach_status    = DeeObject_Foreach(rhs, &roset_compare_seq_foreach, &data);
	if unlikely(foreach_status == ROSET_COMPARE_SEQ_FOREACH_ERROR)
		goto err;
	if (foreach_status == ROSET_COMPARE_SEQ_FOREACH_LESS)
		return Dee_COMPARE_LO;
	if (foreach_status == ROSET_COMPARE_SEQ_FOREACH_GREATER)
		return Dee_COMPARE_GR;
	if (data.rscsfd_index < lhs->rs_vsize)
		return Dee_COMPARE_GR;
	return Dee_COMPARE_EQ;
err:
	return Dee_COMPARE_ERR;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
roset_mh_seq_compare_eq(RoSet *lhs, DeeObject *rhs) {
	Dee_ssize_t foreach_status;
	struct roset_compare_seq_foreach_data data;
	data.rscsfd_index = 0;
	data.rscsfd_lhs   = lhs;
	foreach_status    = DeeObject_Foreach(rhs, &roset_compare_eq_seq_foreach, &data);
	if unlikely(foreach_status == ROSET_COMPARE_SEQ_FOREACH_ERROR)
		goto err;
	if (foreach_status == ROSET_COMPARE_SEQ_FOREACH_NOTEQUAL)
		return Dee_COMPARE_NE;
	if (data.rscsfd_index < lhs->rs_vsize)
		return Dee_COMPARE_NE;
	return Dee_COMPARE_EQ;
err:
	return Dee_COMPARE_ERR;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
roset_mh_seq_trycompare_eq(RoSet *lhs, DeeObject *rhs) {
	if (!DeeType_HasNativeOperator(Dee_TYPE(rhs), foreach))
		return Dee_COMPARE_NE;
	return roset_mh_seq_compare_eq(lhs, rhs);
}



PRIVATE struct type_seq roset_seq = {
	/* .tp_iter         = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&roset_iter,
	/* .tp_sizeob       = */ DEFIMPL(&default__sizeob__with__size),
	/* .tp_contains     = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&roset_contains,
	/* .tp_getitem      = */ DEFIMPL_UNSUPPORTED(&default__getitem__unsupported),
	/* .tp_delitem      = */ DEFIMPL_UNSUPPORTED(&default__delitem__unsupported),
	/* .tp_setitem      = */ DEFIMPL_UNSUPPORTED(&default__setitem__unsupported),
	/* .tp_getrange     = */ DEFIMPL_UNSUPPORTED(&default__getrange__unsupported),
	/* .tp_delrange     = */ DEFIMPL_UNSUPPORTED(&default__delrange__unsupported),
	/* .tp_setrange     = */ DEFIMPL_UNSUPPORTED(&default__setrange__unsupported),
	/* .tp_foreach      = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&roset_mh_seq_foreach,
	/* .tp_foreach_pair = */ DEFIMPL(&default__foreach_pair__with__foreach),
	/* .tp_bounditem    = */ DEFIMPL_UNSUPPORTED(&default__bounditem__unsupported),
	/* .tp_hasitem      = */ DEFIMPL_UNSUPPORTED(&default__hasitem__unsupported),
	/* .tp_size         = */ (size_t (DCALL *)(DeeObject *__restrict))&roset_size,
	/* .tp_size_fast    = */ (size_t (DCALL *)(DeeObject *__restrict))&roset_size_fast,
	/* .tp_getitem_index              = */ DEFIMPL_UNSUPPORTED(&default__getitem_index__unsupported),
	/* .tp_getitem_index_fast         = */ NULL,
	/* .tp_delitem_index              = */ DEFIMPL_UNSUPPORTED(&default__delitem_index__unsupported),
	/* .tp_setitem_index              = */ DEFIMPL_UNSUPPORTED(&default__setitem_index__unsupported),
	/* .tp_bounditem_index            = */ DEFIMPL_UNSUPPORTED(&default__bounditem_index__unsupported),
	/* .tp_hasitem_index              = */ DEFIMPL_UNSUPPORTED(&default__hasitem_index__unsupported),
	/* .tp_getrange_index             = */ DEFIMPL_UNSUPPORTED(&default__getrange_index__unsupported),
	/* .tp_delrange_index             = */ DEFIMPL_UNSUPPORTED(&default__delrange_index__unsupported),
	/* .tp_setrange_index             = */ DEFIMPL_UNSUPPORTED(&default__setrange_index__unsupported),
	/* .tp_getrange_index_n           = */ DEFIMPL_UNSUPPORTED(&default__getrange_index_n__unsupported),
	/* .tp_delrange_index_n           = */ DEFIMPL_UNSUPPORTED(&default__delrange_index_n__unsupported),
	/* .tp_setrange_index_n           = */ DEFIMPL_UNSUPPORTED(&default__setrange_index_n__unsupported),
	/* .tp_trygetitem                 = */ DEFIMPL_UNSUPPORTED(&default__trygetitem__unsupported),
	/* .tp_trygetitem_index           = */ DEFIMPL_UNSUPPORTED(&default__trygetitem_index__unsupported),
	/* .tp_trygetitem_string_hash     = */ DEFIMPL_UNSUPPORTED(&default__trygetitem_string_hash__unsupported),
	/* .tp_getitem_string_hash        = */ DEFIMPL_UNSUPPORTED(&default__getitem_string_hash__unsupported),
	/* .tp_delitem_string_hash        = */ DEFIMPL_UNSUPPORTED(&default__delitem_string_hash__unsupported),
	/* .tp_setitem_string_hash        = */ DEFIMPL_UNSUPPORTED(&default__setitem_string_hash__unsupported),
	/* .tp_bounditem_string_hash      = */ DEFIMPL_UNSUPPORTED(&default__bounditem_string_hash__unsupported),
	/* .tp_hasitem_string_hash        = */ DEFIMPL_UNSUPPORTED(&default__hasitem_string_hash__unsupported),
	/* .tp_trygetitem_string_len_hash = */ DEFIMPL_UNSUPPORTED(&default__trygetitem_string_len_hash__unsupported),
	/* .tp_getitem_string_len_hash    = */ DEFIMPL_UNSUPPORTED(&default__getitem_string_len_hash__unsupported),
	/* .tp_delitem_string_len_hash    = */ DEFIMPL_UNSUPPORTED(&default__delitem_string_len_hash__unsupported),
	/* .tp_setitem_string_len_hash    = */ DEFIMPL_UNSUPPORTED(&default__setitem_string_len_hash__unsupported),
	/* .tp_bounditem_string_len_hash  = */ DEFIMPL_UNSUPPORTED(&default__bounditem_string_len_hash__unsupported),
	/* .tp_hasitem_string_len_hash    = */ DEFIMPL_UNSUPPORTED(&default__hasitem_string_len_hash__unsupported),
};

PRIVATE struct type_getset tpconst roset_getsets[] = {
	TYPE_GETTER_BOUND_NODOC(STR_first, &roset_getfirst, &roset_nonempty_as_bound),
	TYPE_GETTER_BOUND_NODOC(STR_last, &roset_getlast, &roset_nonempty_as_bound),

	TYPE_GETTER_AB(STR_cached, &DeeObject_NewRef, "->?."),
	TYPE_GETTER_AB(STR_frozen, &DeeObject_NewRef, "->?."),
	TYPE_GETTER_AB_F("__sizeof__", &roset_sizeof, METHOD_FNOREFESCAPE, "->?Dint"),
	TYPE_GETTER_AB_F("__hidxio__", &roset___hidxio__, METHOD_FNOREFESCAPE,
	                 "->?Dint\n"
	                 "Size shift-multipler for htab words (word size is ${1 << __hidxio__})\n"
	                 "#T{?#__hidxio__|htab word type~"
	                 /**/ "$0|?Ectypes:uint8_t"
	                 /**/ IF_Dee_HASH_HIDXIO_COUNT_GE_2("&" "$1|?Ectypes:uint16_t")
	                 /**/ IF_Dee_HASH_HIDXIO_COUNT_GE_3("&" "$2|?Ectypes:uint32_t")
	                 /**/ IF_Dee_HASH_HIDXIO_COUNT_GE_4("&" "$3|?Ectypes:uint64_t")
	                 "}"),
	TYPE_GETSET_END
};

STATIC_ASSERT(offsetof(RoSet, rs_vsize) == offsetof(DeeRoDictObject, rd_vsize));
STATIC_ASSERT(offsetof(RoSet, rs_hmask) == offsetof(DeeRoDictObject, rd_hmask));
INTDEF struct type_member tpconst rodict_members[];
#define roset_members rodict_members

PRIVATE struct type_member tpconst roset_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &RoSetIterator_Type),
	TYPE_MEMBER_CONST(STR_Frozen, &DeeRoSet_Type),
	TYPE_MEMBER_CONST(STR___seq_getitem_always_bound__, Dee_True), /* Must be specified because we also define "__seq_getitem__" */
	TYPE_MEMBER_END
};

PRIVATE struct type_method tpconst roset_methods[] = {
//	TYPE_KWMETHOD("byhash", &roset_byhash, DOC_GET(set_byhash_doc)), /* TODO */
	TYPE_METHOD_HINTREF(Sequence_contains),
	TYPE_METHOD_HINTREF(__seq_iter__),
	TYPE_METHOD_HINTREF(__seq_size__),
	TYPE_METHOD_HINTREF(__seq_getitem__),
	TYPE_METHOD_HINTREF(__seq_enumerate__),
	TYPE_METHOD_HINTREF(__seq_compare__),
	TYPE_METHOD_HINTREF(__seq_compare_eq__),
	TYPE_METHOD_HINTREF(__seq_find__),
	TYPE_METHOD_HINTREF(__seq_rfind__),
	TYPE_METHOD_END
};

PRIVATE struct type_method_hint tpconst roset_method_hints[] = {
	/* Operators for "Set as Sequence" (specifically defined because sets are ordered) */
	TYPE_METHOD_HINT_F(seq_foreach_reverse, &roset_mh_seq_foreach_reverse, METHOD_FNOREFESCAPE | METHOD_FCONSTCALL),
	TYPE_METHOD_HINT_F(seq_enumerate_index_reverse, &roset_mh_seq_enumerate_index_reverse, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_SET_CONSTCONTAINS | METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_trygetfirst, &roset_trygetfirst, METHOD_FNOREFESCAPE | METHOD_FCONSTCALL),
	TYPE_METHOD_HINT_F(seq_trygetlast, &roset_trygetlast, METHOD_FNOREFESCAPE | METHOD_FCONSTCALL),
	TYPE_METHOD_HINT_F(set_trygetfirst, &roset_trygetfirst, METHOD_FNOREFESCAPE | METHOD_FCONSTCALL), /* Must also be set because the "first" getset defines __seq_first__ and __set_first__ */
	TYPE_METHOD_HINT_F(set_trygetlast, &roset_trygetlast, METHOD_FNOREFESCAPE | METHOD_FCONSTCALL),   /* Must also be set because the "last" getset defines __seq_last__ and __set_last__ */
	TYPE_METHOD_HINT_F(seq_enumerate_index, &roset_mh_seq_enumerate_index, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_SET_CONSTCONTAINS | METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_operator_iter, &roset_iter, METHOD_FNOREFESCAPE | METHOD_FCONSTCALL),
	TYPE_METHOD_HINT_F(seq_operator_foreach, &roset_mh_seq_foreach, METHOD_FNOREFESCAPE | METHOD_FCONSTCALL),
	TYPE_METHOD_HINT_F(seq_operator_size, &roset_size, METHOD_FNOREFESCAPE | METHOD_FCONSTCALL),
	TYPE_METHOD_HINT_F(seq_operator_getitem_index, &roset_mh_seq_getitem_index, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_SET_CONSTCONTAINS | METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_operator_trygetitem_index, &roset_mh_seq_trygetitem_index, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_SET_CONSTCONTAINS | METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_operator_compare_eq, &roset_mh_seq_compare_eq, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_SET_CONSTCMPEQ | METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_operator_compare, &roset_mh_seq_compare, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_SET_CONSTCMP | METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_operator_trycompare_eq, &roset_mh_seq_trycompare_eq, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_SET_CONSTCMPEQ | METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_contains, &roset_mh_contains, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_SET_CONSTCMPEQ | METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_contains_with_key, &default__seq_contains_with_key__with__seq_operator_foreach, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_contains_with_range, &roset_mh_contains_with_range, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_SET_CONSTCMPEQ | METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_contains_with_range_and_key, &default__seq_contains_with_range_and_key__with__seq_enumerate_index, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_find, &roset_mh_find, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_SET_CONSTCMPEQ | METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_find_with_key, &default__seq_find_with_key__with__seq_enumerate_index, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_rfind, &roset_mh_find, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_SET_CONSTCMPEQ | METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_rfind_with_key, &default__seq_rfind_with_key__with__seq_enumerate_index_reverse, METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_END
};

PRIVATE struct type_operator const roset_operators[] = {
	TYPE_OPERATOR_FLAGS(OPERATOR_0000_CONSTRUCTOR, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST),
	TYPE_OPERATOR_FLAGS(OPERATOR_0001_COPY, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_0007_REPR, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_THISELEM_CONSTREPR | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0008_BOOL, METHOD_FCONSTCALL | METHOD_FNOTHROW | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0028_HASH, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_THISELEM_CONSTHASH | METHOD_FNOREFESCAPE),
	//TODO:TYPE_OPERATOR_FLAGS(OPERATOR_0029_EQ, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_SEQ_CONSTCMPEQ | METHOD_FNOREFESCAPE),
	//TODO:TYPE_OPERATOR_FLAGS(OPERATOR_002A_NE, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_SEQ_CONSTCMPEQ | METHOD_FNOREFESCAPE),
	//TODO:TYPE_OPERATOR_FLAGS(OPERATOR_002B_LO, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_SEQ_CONSTCMP | METHOD_FNOREFESCAPE),
	//TODO:TYPE_OPERATOR_FLAGS(OPERATOR_002C_LE, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_SEQ_CONSTCMP | METHOD_FNOREFESCAPE),
	//TODO:TYPE_OPERATOR_FLAGS(OPERATOR_002D_GR, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_SEQ_CONSTCMP | METHOD_FNOREFESCAPE),
	//TODO:TYPE_OPERATOR_FLAGS(OPERATOR_002E_GE, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_SEQ_CONSTCMP | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_002F_ITER, METHOD_FCONSTCALL | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0030_SIZE, METHOD_FCONSTCALL | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0031_CONTAINS, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_SET_CONSTCONTAINS | METHOD_FNOREFESCAPE),
};

PUBLIC DeeTypeObject DeeRoSet_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_RoSet",
	/* .tp_doc      = */ DOC("Read-only / frozen variant of ?DHashSet\n"
	                         "\n"

	                         "()\n"
	                         "Returns an empty ?.\n"
	                         "\n"

	                         "(set:?DHashSet)\n"
	                         "(set:?DSet)\n"
	                         "(seq:?S?O)\n"
	                         "Convert the given set/seq into an ?. and return it\n"
	                         "\n"

	                         "copy->\n"
	                         "deepcopy->\n"
	                         "Always re-returns @this\n"
	                         "\n"

	                         "bool->\n"
	                         "Returns ?t if @this ?. is non-empty\n"
	                         "\n"

	                         "iter->\n"
	                         "Enumerate keys stored in the ?., in order of being specified during construction\n"
	                         "\n"

	                         "#->\n"
	                         "Returns the number of keys within @this ?.\n"
	                         "\n"

	                         "contains->\n"
	                         "Returns ?t if @item is apart of @this ?. (s.a. ?#contains)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FVARIABLE | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSet_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_VAR(
			/* tp_ctor:        */ &roset_ctor,
			/* tp_copy_ctor:   */ &DeeObject_NewRef,
			/* tp_any_ctor:    */ &roset_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &roset_serialize,
			/* tp_free:        */ NULL
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&roset_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ DEFIMPL(&object_str),
		/* .tp_repr      = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool      = */ (int (DCALL *)(DeeObject *__restrict))&roset_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&roset_printrepr,
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&roset_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__F6E3D7B2219AE1EB),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__48CC5897A5CA5795), /* TODO: &roset_cmp */
	/* .tp_seq           = */ &roset_seq,
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__C6F8E138F179B5AD),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ roset_methods,
	/* .tp_getsets       = */ roset_getsets,
	/* .tp_members       = */ roset_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ roset_class_members,
	/* .tp_method_hints  = */ roset_method_hints,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
	/* .tp_mro           = */ NULL,
	/* .tp_operators     = */ roset_operators,
	/* .tp_operators_size= */ COMPILER_LENOF(roset_operators),
};

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_ROSET_C */
