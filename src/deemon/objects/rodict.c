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
#ifndef GUARD_DEEMON_OBJECTS_RODICT_C
#define GUARD_DEEMON_OBJECTS_RODICT_C 1

#include <deemon/api.h>

#include <deemon/alloc.h>              /* DeeObject_MALLOC, Dee_TYPE_CONSTRUCTOR_INIT_FIXED */
#include <deemon/arg.h>                /* DeeArg_UnpackStruct1XOr2X, DeeArg_UnpackStructKw, UNPuSIZ, _DeeArg_AsObject */
#include <deemon/bool.h>               /* Dee_True, return_bool */
#include <deemon/computed-operators.h> /* DEFIMPL, DEFIMPL_UNSUPPORTED */
#include <deemon/dict.h>               /* DeeDictObject, DeeDict_LockEndRead, DeeDict_Type, Dee_dict_item, _DeeDict_GetRealVTab */
#include <deemon/error-rt.h>           /* DeeRT_Err* */
#include <deemon/format.h>             /* PRFuSIZ, PRFxSIZ */
#include <deemon/hashset.h>            /* DeeHashSetObject, DeeHashSet_LockEndRead, DeeHashSet_Type, Dee_hashset_item, _DeeHashSet_GetRealVTab */
#include <deemon/int.h>                /* DeeInt_NEWU, DeeInt_NewSize */
#include <deemon/map.h>                /* DeeMap_Type */
#include <deemon/method-hints.h>       /* Dee_seq_enumerate_index_t, TYPE_METHOD_HINT*, type_method_hint */
#include <deemon/none.h>               /* Dee_None */
#include <deemon/object.h>             /* DREF, DeeObject, DeeObject_*, DeeTypeObject, Dee_AsObject, Dee_BOUND_*, Dee_COMPARE_*, Dee_Decref, Dee_DecrefNokill, Dee_Incref, Dee_Incref_n, Dee_TYPE, Dee_foreach_pair_t, Dee_foreach_t, Dee_formatprinter_t, Dee_hash_t, Dee_return_compareT, Dee_return_compare_if_neT, Dee_ssize_t, ITER_DONE, ITER_ISOK, OBJECT_HEAD_INIT, return_reference, return_reference_ */
#include <deemon/operator-hints.h>     /* DeeType_HasNativeOperator */
#include <deemon/pair.h>               /* DeeSeqPair*, DeeSeq_* */
#include <deemon/rodict.h>             /* DeeRoDict*, Dee_rodict_builder*, _DeeRoDict_* */
#include <deemon/roset.h>              /* DeeRoSetObject, DeeRoSet_Type, _DeeRoSet_GetRealVTab */
#include <deemon/seq.h>                /* DeeIterator_Type */
#include <deemon/serial.h>             /* DeeSerial, Dee_seraddr_t */
#include <deemon/system-features.h>    /* memcpy, memset */
#include <deemon/type.h>               /* DeeObject_InitStatic, DeeObject_IsShared, DeeType_Type, Dee_TYPE_CONSTRUCTOR_INIT_FIXED, Dee_TYPE_CONSTRUCTOR_INIT_VAR, Dee_visit_t, METHOD_F*, OPERATOR_*, STRUCT_*, TF_KW, TF_NONE, TP_F*, TYPE_*, type_* */
#include <deemon/util/atomic.h>        /* atomic_cmpxch_or_write, atomic_read */
#include <deemon/util/hash-io.h>       /* Dee_HASH_*, Dee_SIZEOF_HASH_VIDX_T, Dee_hash_*, IF_Dee_HASH_HIDXIO_COUNT_GE_* */

#include <hybrid/overflow.h> /* OVERFLOW_UADD */
#include <hybrid/typecore.h> /* __BYTE_TYPE__ */

#include "../runtime/kwlist.h"
#include "../runtime/strings.h"
#include "dict-utils.h"
#include "generic-proxy.h"
#include "rodict.h"
#include "seq/default-compare.h"
#include "seq/default-map-proxy.h"

#include <stddef.h> /* NULL, offsetof, size_t */

#undef byte_t
#define byte_t __BYTE_TYPE__

#ifndef DICT_NDEBUG
#include <hybrid/align.h> /* IS_POWER_OF_TWO */
#endif /* !DICT_NDEBUG */

DECL_BEGIN

#ifndef NDEBUG
#define DBG_memset (void)memset
#else /* !NDEBUG */
#define DBG_memset(dst, byte, n_bytes) (void)0
#endif /* NDEBUG */

typedef DeeRoDictObject RoDict;

/************************************************************************/
/* RODICT HELPERS                                                       */
/************************************************************************/


#ifdef DICT_NDEBUG
#define rodict_verify(self) (void)0
#else /* DICT_NDEBUG */
PRIVATE ATTR_NOINLINE NONNULL((1)) void DCALL
rodict_verify(RoDict *__restrict self) {
	size_t i, real_vused;
	Dee_hash_hidxio_t hidxio;
	ASSERT(self->rd_vsize <= self->rd_hmask);
	ASSERT(IS_POWER_OF_TWO(self->rd_hmask + 1));
	ASSERT(self->rd_htab == (union Dee_hash_htab *)(_DeeRoDict_GetRealVTab(self) + self->rd_vsize));
	hidxio = Dee_HASH_HIDXIO_FROM_VALLOC(self->rd_vsize);
	ASSERT(/*hidxio >= 0 &&*/ hidxio < Dee_HASH_HIDXIO_COUNT);
	/* hidxio==0 may differ if "self" was statically initialized in a dex module,
	 * in which case `self->rd_hidxget' might point into that module's PLT/GOT. */
	ASSERT(self->rd_hidxget == Dee_hash_hidxio[hidxio].hxio_get || hidxio == 0);
	for (i = Dee_hash_vidx_tovirt(0), real_vused = 0;
	     Dee_hash_vidx_virt_lt_real(i, self->rd_vsize); ++i) {
		struct Dee_dict_item *item = &_DeeRoDict_GetVirtVTab(self)[i];
		if (item->di_key) {
#if 0 /* Cannot be asserted -- we might get here from "tp_visit", which can screw with reference counts */
			ASSERT_OBJECT(item->di_key);
			ASSERT_OBJECT(item->di_value);
#endif
			++real_vused;
		}
	}
	ASSERTF(real_vused == self->rd_vsize,
	        "RODICT: vtab key count=%" PRFuSIZ " differs from rd_vsize=%" PRFuSIZ,
	        real_vused, self->rd_vsize);
	for (i = 0; i <= self->rd_hmask; ++i) {
		Dee_hash_vidx_t vidx;
		vidx = (*self->rd_hidxget)(self->rd_htab, i);
		if (vidx == Dee_HASH_HTAB_EOF)
			continue;
		Dee_hash_vidx_virt2real(&vidx);
		ASSERTF(vidx < self->rd_vsize,
		        "RODICT: htab[%" PRFuSIZ "] points out-of-bounds: %" PRFuSIZ " >= %" PRFuSIZ,
		        i, vidx, self->rd_vsize);
	}
	for (i = 0;; ++i) {
		Dee_hash_vidx_t vtab_idx;
		ASSERTF(i <= self->rd_hmask, "RODICT: htab contains no EOF pointers (infinite loop would occur on non-present item lookup)");
		vtab_idx = (*self->rd_hidxget)(self->rd_htab, i);
		if (vtab_idx == Dee_HASH_HTAB_EOF)
			break;
	}
	for (i = Dee_hash_vidx_tovirt(0), real_vused = 0;
	     Dee_hash_vidx_virt_lt_real(i, self->rd_vsize); ++i) {
		Dee_hash_t hs, perturb;
		struct Dee_dict_item *item = &_DeeRoDict_GetVirtVTab(self)[i];
		if (!item->di_key)
			continue;
		for (_DeeRoDict_HashIdxInit(self, &hs, &perturb, item->di_hash);;
		     _DeeRoDict_HashIdxNext(self, &hs, &perturb, item->di_hash)) {
			struct Dee_dict_item *hitem;
			Dee_hash_hidx_t htab_idx = Dee_hash_hidx_ofhash(hs, self->rd_hmask);
			Dee_hash_vidx_t vtab_idx = (*self->rd_hidxget)(self->rd_htab, htab_idx); /*virt*/
			ASSERTF(vtab_idx != Dee_HASH_HTAB_EOF,
			        "RODICT: End-of-hash-chain[hash:%#" PRFxSIZ "] before item idx=%" PRFuSIZ ",count=%" PRFuSIZ " <%r:%r> was found",
			        item->di_hash, Dee_hash_vidx_toreal(i), self->rd_vsize,
			        item->di_key, item->di_value);
			hitem = &_DeeRoDict_GetVirtVTab(self)[vtab_idx];
			if (hitem == item)
				break;
		}
	}
}
#endif /* !DICT_NDEBUG */


/************************************************************************/
/* RODICT ITERATOR                                                      */
/************************************************************************/

STATIC_ASSERT(offsetof(RoDictIterator, rodi_dict) == offsetof(ProxyObject, po_obj));
#define rditer_fini      generic_proxy__fini
#define rditer_visit     generic_proxy__visit
#define rditer_serialize generic_proxy__serialize_and_wordcopy_atomic(Dee_SIZEOF_HASH_VIDX_T)

PRIVATE WUNUSED NONNULL((1)) int DCALL
rditer_ctor(RoDictIterator *__restrict self) {
	self->rodi_dict = (DREF RoDict *)DeeRoDict_NewEmpty();
	self->rodi_vidx = 0;
	return 0;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL /* "INTERN" because shared with "./roset.c" */
rditer_copy(RoDictIterator *__restrict self,
            RoDictIterator *__restrict other) {
	self->rodi_dict = other->rodi_dict;
	Dee_Incref(self->rodi_dict);
	self->rodi_vidx = atomic_read(&other->rodi_vidx);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
rditer_init(RoDictIterator *__restrict self, size_t argc, DeeObject *const *argv) {
/*[[[deemon (print_DeeArg_Unpack from rt.gen.unpack)("_RoDictIterator", params: "
	DeeRoDictObject *dict;
	Dee_hash_vidx_t index = 0
", docStringPrefix: "rditer");]]]*/
#define rditer__RoDictIterator_params "dict:?Ert:RoDict,index=!0"
	struct {
		DeeRoDictObject *dict;
		Dee_hash_vidx_t index;
	} args;
	args.index = 0;
	DeeArg_UnpackStruct1XOr2X(err, argc, argv, "_RoDictIterator", &args, &args.dict, "o", _DeeArg_AsObject, &args.index, UNPuSIZ, DeeObject_AsSize);
/*[[[end]]]*/
	if (DeeObject_AssertTypeExact(args.dict, &DeeRoDict_Type))
		goto err;
	Dee_Incref(args.dict);
	self->rodi_dict = args.dict;
	self->rodi_vidx = args.index;
	return 0;
err:
	return -1;
}

INTERN WUNUSED NONNULL((1)) int DCALL /* "INTERN" because shared with "./roset.c" */
rditer_bool(RoDictIterator *__restrict self) {
	return atomic_read(&self->rodi_vidx) < self->rodi_dict->rd_vsize ? 1 : 0;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL /* "INTERN" because shared with "./roset.c" */
rditer_compare(RoDictIterator *__restrict lhs, RoDictIterator *__restrict rhs) {
	if (DeeObject_AssertTypeExact(rhs, Dee_TYPE(lhs)))
		goto err;
	Dee_return_compare_if_neT(RoDict *, lhs->rodi_dict, rhs->rodi_dict);
	Dee_return_compareT(/*real*/Dee_hash_vidx_t,
	                    atomic_read(&lhs->rodi_vidx),
	                    atomic_read(&rhs->rodi_vidx));
err:
	return Dee_COMPARE_ERR;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
rditer_nextpair(RoDictIterator *__restrict self, DREF DeeObject *key_and_value[2]) {
	RoDict *dict = self->rodi_dict;
	/*real*/Dee_hash_vidx_t vidx;
	struct Dee_dict_item *item;
	do {
		vidx = atomic_read(&self->rodi_vidx);
		if (vidx >= dict->rd_vsize)
			return 1;
	} while (!atomic_cmpxch_or_write(&self->rodi_vidx, vidx, vidx + 1));
	item = &_DeeRoDict_GetRealVTab(dict)[vidx];
	key_and_value[0] = item->di_key;
	key_and_value[1] = item->di_value;
	Dee_Incref(key_and_value[0]);
	Dee_Incref(key_and_value[1]);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rditer_nextkey(RoDictIterator *__restrict self) {
	RoDict *dict = self->rodi_dict;
	/*real*/Dee_hash_vidx_t vidx;
	struct Dee_dict_item *item;
	do {
		vidx = atomic_read(&self->rodi_vidx);
		if (vidx >= dict->rd_vsize)
			return ITER_DONE;
	} while (!atomic_cmpxch_or_write(&self->rodi_vidx, vidx, vidx + 1));
	item = &_DeeRoDict_GetRealVTab(dict)[vidx];
	return_reference_(item->di_key);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rditer_nextvalue(RoDictIterator *__restrict self) {
	RoDict *dict = self->rodi_dict;
	/*real*/Dee_hash_vidx_t vidx;
	struct Dee_dict_item *item;
	do {
		vidx = atomic_read(&self->rodi_vidx);
		if (vidx >= dict->rd_vsize)
			return ITER_DONE;
	} while (!atomic_cmpxch_or_write(&self->rodi_vidx, vidx, vidx + 1));
	item = &_DeeRoDict_GetRealVTab(dict)[vidx];
	return_reference_(item->di_value);
}

INTERN WUNUSED NONNULL((1)) size_t DCALL /* "INTERN" because shared with "./roset.c" */
rditer_advance(RoDictIterator *__restrict self, size_t step) {
	RoDict *dict = self->rodi_dict;
	/*real*/Dee_hash_vidx_t old_vidx;
	/*real*/Dee_hash_vidx_t new_vidx;
	do {
		old_vidx = atomic_read(&self->rodi_vidx);
		if (OVERFLOW_UADD(old_vidx, step, &new_vidx))
			new_vidx = (Dee_hash_vidx_t)-1;
		if (new_vidx > dict->rd_vsize)
			new_vidx = dict->rd_vsize;
	} while (!atomic_cmpxch_or_write(&self->rodi_vidx, old_vidx, new_vidx));
	return (size_t)(new_vidx - old_vidx);
}

PRIVATE struct type_cmp rditer_cmp = {
	/* .tp_hash          = */ DEFIMPL_UNSUPPORTED(&default__hash__unsupported),
	/* .tp_compare_eq    = */ (int (DCALL *)(DeeObject *, DeeObject *))&rditer_compare,
	/* .tp_compare       = */ (int (DCALL *)(DeeObject *, DeeObject *))&rditer_compare,
	/* .tp_trycompare_eq = */ DEFIMPL(&default__trycompare_eq__with__compare_eq),
	/* .tp_eq            = */ DEFIMPL(&default__eq__with__compare_eq),
	/* .tp_ne            = */ DEFIMPL(&default__ne__with__compare_eq),
	/* .tp_lo            = */ DEFIMPL(&default__lo__with__compare),
	/* .tp_le            = */ DEFIMPL(&default__le__with__compare),
	/* .tp_gr            = */ DEFIMPL(&default__gr__with__compare),
	/* .tp_ge            = */ DEFIMPL(&default__ge__with__compare),
};

PRIVATE struct type_iterator rditer_iterator = {
	/* .tp_nextpair  = */ (int (DCALL *)(DeeObject *__restrict, DREF DeeObject *[2]))&rditer_nextpair,
	/* .tp_nextkey   = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&rditer_nextkey,
	/* .tp_nextvalue = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&rditer_nextvalue,
};

INTDEF struct type_method tpconst rditer_methods[]; /* "INTER" because shared with "./roset.c" */
INTERN_TPCONST struct type_method tpconst rditer_methods[] = {
	TYPE_METHOD_HINTREF(Iterator_advance),
	TYPE_METHOD_END
};

PRIVATE struct type_method_hint tpconst rditer_method_hints[] = {
	TYPE_METHOD_HINT(iter_advance, &rditer_advance),
	TYPE_METHOD_HINT_END
};

PRIVATE struct type_member tpconst rditer_members[] = {
	TYPE_MEMBER_FIELD_DOC(STR_seq, STRUCT_OBJECT_AB, offsetof(RoDictIterator, rodi_dict), "->?Ert:RoDict"),
	TYPE_MEMBER_FIELD_DOC("__index__", STRUCT_SIZE_T | STRUCT_ATOMIC, offsetof(RoDictIterator, rodi_vidx), "->?Dint"),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject RoDictIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_RoDictIterator",
	/* .tp_doc      = */ DOC("()\n"
	                         "(" rditer__RoDictIterator_params ")\n"
	                         "\n"
	                         "next->?T2?O?O"),
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ RoDictIterator,
			/* tp_ctor:        */ &rditer_ctor,
			/* tp_copy_ctor:   */ &rditer_copy,
			/* tp_any_ctor:    */ &rditer_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &rditer_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&rditer_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&rditer_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&iterator_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&rditer_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__EFED4BCD35433C3C),
	/* .tp_cmp           = */ &rditer_cmp,
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ DEFIMPL(&default__iter_next__with__nextpair),
	/* .tp_iterator      = */ &rditer_iterator,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ rditer_methods,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ rditer_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ rditer_method_hints,
	/* .tp_call          = */ DEFIMPL(&iterator_next),
	/* .tp_callable      = */ DEFIMPL(&default__tp_callable__83C59FA7626CABBE),
};







/************************************************************************/
/* RODICT                                                               */
/************************************************************************/

INTERN WUNUSED NONNULL((1)) int DCALL /* "INTERN" because shared with "./roset.c" */
rodict_bool(DeeRoDictObject *__restrict self) {
	return self->rd_vsize != 0;
}

#define rodict_size_fast rodict_size
INTERN WUNUSED NONNULL((1)) size_t DCALL /* "INTERN" because shared with "./roset.c" */
rodict_size(DeeRoDictObject *__restrict self) {
	return self->rd_vsize;
}

INTERN WUNUSED NONNULL((1)) int DCALL /* "INTERN" because shared with "./roset.c" */
rodict_nonempty_as_bound(DeeRoDictObject *__restrict self) {
	return Dee_BOUND_FROMBOOL(self->rd_vsize != 0);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL /* "INTERN" because shared with "./roset.c" */
rodict___hidxio__(DeeRoDictObject *__restrict self) {
	Dee_hash_hidxio_t result = Dee_HASH_HIDXIO_FROM_VALLOC(self->rd_vsize);
	return DeeInt_NEWU(result);
}

PRIVATE WUNUSED NONNULL((1)) DREF RoDictIterator *DCALL
rodict_iter(RoDict *__restrict self) {
	DREF RoDictIterator *result;
	result = DeeObject_MALLOC(RoDictIterator);
	if unlikely(!result)
		goto err;
	result->rodi_dict = self;
	Dee_Incref(self);
	result->rodi_vidx = 0;
	DeeObject_InitStatic(result, &RoDictIterator_Type);
	return result;
err:
	return NULL;
}

#ifdef __INTELLISENSE__
PRIVATE WUNUSED DREF RoDict *DCALL rodict_ctor(void);
PRIVATE WUNUSED DREF RoDict *DCALL rodict_init(size_t argc, DeeObject *const *argv);
PRIVATE NONNULL((1)) void DCALL rodict_fini(RoDict *__restrict self);
PRIVATE NONNULL((1, 2)) void DCALL rodict_visit(RoDict *__restrict self, Dee_visit_t proc, void *arg);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_seraddr_t DCALL rodict_serialize(RoDict *__restrict self, DeeSerial *__restrict writer);
PRIVATE WUNUSED NONNULL((1)) int DCALL rodict_bool(RoDict *__restrict self);
PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL rodict_printrepr(RoDict *__restrict self, Dee_formatprinter_t printer, void *arg);
#else /* __INTELLISENSE__ */
DECL_END
#define DEFINE_DeeRoDict
#include "rodict-impl.c.inl"
DECL_BEGIN
#endif /* !__INTELLISENSE__ */


/* Item lookup operators. */
PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL rodict_contains(RoDict *self, DeeObject *key);
PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL rodict_getitem(RoDict *self, DeeObject *key);
PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL rodict_trygetitem(RoDict *self, DeeObject *key);
PRIVATE WUNUSED NONNULL((1, 2)) DeeObject *DCALL rodict_trygetitemnr(RoDict *self, DeeObject *key);
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL rodict_bounditem(RoDict *self, DeeObject *key);

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL rodict_getitem_index(RoDict *self, size_t key);
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL rodict_trygetitem_index(RoDict *self, size_t key);
PRIVATE WUNUSED NONNULL((1)) int DCALL rodict_bounditem_index(RoDict *self, size_t key);

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL rodict_getitem_string_hash(RoDict *self, char const *key, Dee_hash_t hash);
PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL rodict_trygetitem_string_hash(RoDict *self, char const *key, Dee_hash_t hash);
PRIVATE WUNUSED NONNULL((1, 2)) DeeObject *DCALL rodict_trygetitemnr_string_hash(RoDict *self, char const *key, Dee_hash_t hash);
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL rodict_bounditem_string_hash(RoDict *self, char const *key, Dee_hash_t hash);

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL rodict_getitem_string_len_hash(RoDict *self, char const *key, size_t keylen, Dee_hash_t hash);
PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL rodict_trygetitem_string_len_hash(RoDict *self, char const *key, size_t keylen, Dee_hash_t hash);
PRIVATE WUNUSED NONNULL((1, 2)) DeeObject *DCALL rodict_trygetitemnr_string_len_hash(RoDict *self, char const *key, size_t keylen, Dee_hash_t hash);
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL rodict_bounditem_string_len_hash(RoDict *self, char const *key, size_t keylen, Dee_hash_t hash);

#define rodict_hasitem                 rodict_bounditem
#define rodict_hasitem_index           rodict_bounditem_index
#define rodict_hasitem_string_hash     rodict_bounditem_string_hash
#define rodict_hasitem_string_len_hash rodict_bounditem_string_len_hash

#ifdef __OPTIMIZE_SIZE__
#define SUBSTITUDE_rodict_contains
#define SUBSTITUDE_rodict_getitem
#define SUBSTITUDE_rodict_trygetitem
#define SUBSTITUDE_rodict_bounditem
#define SUBSTITUDE_rodict_getitem_index
#define SUBSTITUDE_rodict_bounditem_index
#define SUBSTITUDE_rodict_getitem_string_hash
#define SUBSTITUDE_rodict_trygetitem_string_hash
#define SUBSTITUDE_rodict_bounditem_string_hash
#define SUBSTITUDE_rodict_getitem_string_len_hash
#define SUBSTITUDE_rodict_trygetitem_string_len_hash
#define SUBSTITUDE_rodict_bounditem_string_len_hash
#else /* __OPTIMIZE_SIZE__ */
/* Event without -Os, substitute these operators, just because
 * the dedicated variant wouldn't really be noticeably faster. */
#define SUBSTITUDE_rodict_contains
#define SUBSTITUDE_rodict_bounditem
/*#define SUBSTITUDE_rodict_bounditem_index*/ /* Would need unnecessary incref() */
#define SUBSTITUDE_rodict_bounditem_string_hash
#define SUBSTITUDE_rodict_bounditem_string_len_hash
#endif /* !__OPTIMIZE_SIZE__ */

#ifndef __INTELLISENSE__
DECL_END
#define DEFINE_rodict_trygetitemnr
#include "rodict-getitem.c.inl"
#define DEFINE_rodict_trygetitemnr_string_hash
#include "rodict-getitem.c.inl"
#define DEFINE_rodict_trygetitemnr_string_len_hash
#include "rodict-getitem.c.inl"
#define DEFINE_rodict_trygetitem_index
#include "rodict-getitem.c.inl"

#ifndef SUBSTITUDE_rodict_contains
#define DEFINE_rodict_contains
#include "rodict-getitem.c.inl"
#endif /* !SUBSTITUDE_rodict_contains */

#ifndef SUBSTITUDE_rodict_getitem
#define DEFINE_rodict_getitem
#include "rodict-getitem.c.inl"
#endif /* !SUBSTITUDE_rodict_getitem */

#ifndef SUBSTITUDE_rodict_trygetitem
#define DEFINE_rodict_trygetitem
#include "rodict-getitem.c.inl"
#endif /* !SUBSTITUDE_rodict_trygetitem */

#ifndef SUBSTITUDE_rodict_bounditem
#define DEFINE_rodict_bounditem
#include "rodict-getitem.c.inl"
#endif /* !SUBSTITUDE_rodict_bounditem */

#ifndef SUBSTITUDE_rodict_getitem_index
#define DEFINE_rodict_getitem_index
#include "rodict-getitem.c.inl"
#endif /* !SUBSTITUDE_rodict_getitem_index */

#ifndef SUBSTITUDE_rodict_bounditem_index
#define DEFINE_rodict_bounditem_index
#include "rodict-getitem.c.inl"
#endif /* !SUBSTITUDE_rodict_bounditem_index */

#ifndef SUBSTITUDE_rodict_getitem_string_hash
#define DEFINE_rodict_getitem_string_hash
#include "rodict-getitem.c.inl"
#endif /* !SUBSTITUDE_rodict_getitem_string_hash */

#ifndef SUBSTITUDE_rodict_trygetitem_string_hash
#define DEFINE_rodict_trygetitem_string_hash
#include "rodict-getitem.c.inl"
#endif /* !SUBSTITUDE_rodict_trygetitem_string_hash */

#ifndef SUBSTITUDE_rodict_bounditem_string_hash
#define DEFINE_rodict_bounditem_string_hash
#include "rodict-getitem.c.inl"
#endif /* !SUBSTITUDE_rodict_bounditem_string_hash */

#ifndef SUBSTITUDE_rodict_getitem_string_len_hash
#define DEFINE_rodict_getitem_string_len_hash
#include "rodict-getitem.c.inl"
#endif /* !SUBSTITUDE_rodict_getitem_string_len_hash */

#ifndef SUBSTITUDE_rodict_trygetitem_string_len_hash
#define DEFINE_rodict_trygetitem_string_len_hash
#include "rodict-getitem.c.inl"
#endif /* !SUBSTITUDE_rodict_trygetitem_string_len_hash */

#ifndef SUBSTITUDE_rodict_bounditem_string_len_hash
#define DEFINE_rodict_bounditem_string_len_hash
#include "rodict-getitem.c.inl"
#endif /* !SUBSTITUDE_rodict_bounditem_string_len_hash */

DECL_BEGIN
#endif /* !__INTELLISENSE__ */

#ifdef SUBSTITUDE_rodict_contains
PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
rodict_contains(RoDict *self, DeeObject *key) {
	DeeObject *value = rodict_trygetitemnr(self, key);
	if unlikely(!value)
		goto err;
	return_bool(value != ITER_DONE);
err:
	return NULL;
}
#endif /* SUBSTITUDE_rodict_contains */

#ifdef SUBSTITUDE_rodict_getitem
PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
rodict_getitem(RoDict *self, DeeObject *key) {
	DeeObject *result = rodict_trygetitemnr(self, key);
	if likely(ITER_ISOK(result)) {
		Dee_Incref(result);
	} else if (result == ITER_DONE) {
		DeeRT_ErrUnknownKey(self, key);
		result = NULL;
	}
	return result;
}
#endif /* SUBSTITUDE_rodict_getitem */

#ifdef SUBSTITUDE_rodict_trygetitem
PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
rodict_trygetitem(RoDict *self, DeeObject *key) {
	DeeObject *result = rodict_trygetitemnr(self, key);
	if likely(ITER_ISOK(result))
		Dee_Incref(result);
	return result;
}
#endif /* SUBSTITUDE_rodict_trygetitem */

#ifdef SUBSTITUDE_rodict_bounditem
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
rodict_bounditem(RoDict *self, DeeObject *key) {
	DeeObject *value = rodict_trygetitemnr(self, key);
	if unlikely(!value)
		goto err;
	return Dee_BOUND_FROMPRESENT_BOUND(value != ITER_DONE);
err:
	return Dee_BOUND_ERR;
}
#endif /* SUBSTITUDE_rodict_bounditem */

#ifdef SUBSTITUDE_rodict_getitem_index
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rodict_getitem_index(RoDict *self, size_t key) {
	DREF DeeObject *result = rodict_trygetitem_index(self, key);
	if unlikely(result == ITER_DONE) {
		DeeRT_ErrUnknownKeyInt(self, key);
		result = NULL;
	}
	return result;
}
#endif /* SUBSTITUDE_rodict_getitem_index */

#ifdef SUBSTITUDE_rodict_bounditem_index
PRIVATE WUNUSED NONNULL((1)) int DCALL
rodict_bounditem_index(RoDict *self, size_t key) {
	DREF DeeObject *value = rodict_trygetitem_index(self, key);
	if unlikely(!value)
		return Dee_BOUND_ERR;
	if (value == ITER_DONE)
		return Dee_BOUND_MISSING;
	Dee_DecrefNokill(value);
	return Dee_BOUND_YES;
}
#endif /* SUBSTITUDE_rodict_bounditem_index */

#ifdef SUBSTITUDE_rodict_getitem_string_hash
PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
rodict_getitem_string_hash(RoDict *self, char const *key, Dee_hash_t hash) {
	DeeObject *result = rodict_trygetitemnr_string_hash(self, key, hash);
	if likely(ITER_ISOK(result)) {
		Dee_Incref(result);
	} else if (result == ITER_DONE) {
		DeeRT_ErrUnknownKey(self, key);
		result = NULL;
	}
	return result;
}
#endif /* SUBSTITUDE_rodict_getitem_string_hash */

#ifdef SUBSTITUDE_rodict_trygetitem_string_hash
PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
rodict_trygetitem_string_hash(RoDict *self, char const *key, Dee_hash_t hash) {
	DeeObject *result = rodict_trygetitemnr_string_hash(self, key, hash);
	if likely(ITER_ISOK(result))
		Dee_Incref(result);
	return result;
}
#endif /* SUBSTITUDE_rodict_trygetitem_string_hash */

#ifdef SUBSTITUDE_rodict_bounditem_string_hash
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
rodict_bounditem_string_hash(RoDict *self, char const *key, Dee_hash_t hash) {
	DeeObject *value = rodict_trygetitemnr_string_hash(self, key, hash);
	if unlikely(!value)
		goto err;
	return Dee_BOUND_FROMPRESENT_BOUND(value != ITER_DONE);
err:
	return Dee_BOUND_ERR;
}
#endif /* SUBSTITUDE_rodict_bounditem_string_hash */

#ifdef SUBSTITUDE_rodict_getitem_string_len_hash
PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
rodict_getitem_string_len_hash(RoDict *self, char const *key, size_t keylen, Dee_hash_t hash) {
	DeeObject *result = rodict_trygetitemnr_string_len_hash(self, key, keylen, hash);
	if likely(ITER_ISOK(result)) {
		Dee_Incref(result);
	} else if (result == ITER_DONE) {
		DeeRT_ErrUnknownKey(self, key);
		result = NULL;
	}
	return result;
}
#endif /* SUBSTITUDE_rodict_getitem_string_len_hash */

#ifdef SUBSTITUDE_rodict_trygetitem_string_len_hash
PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
rodict_trygetitem_string_len_hash(RoDict *self, char const *key, size_t keylen, Dee_hash_t hash) {
	DeeObject *result = rodict_trygetitemnr_string_len_hash(self, key, keylen, hash);
	if (ITER_ISOK(result))
		Dee_Incref(result);
	return result;
}
#endif /* SUBSTITUDE_rodict_trygetitem_string_len_hash */

#ifdef SUBSTITUDE_rodict_bounditem_string_len_hash
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
rodict_bounditem_string_len_hash(RoDict *self, char const *key, size_t keylen, Dee_hash_t hash) {
	DeeObject *value = rodict_trygetitemnr_string_len_hash(self, key, keylen, hash);
	if unlikely(!value)
		goto err;
	return Dee_BOUND_FROMPRESENT_BOUND(value != ITER_DONE);
err:
	return Dee_BOUND_ERR;
}
#endif /* SUBSTITUDE_rodict_bounditem_string_len_hash */


PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
rodict_foreach_pair(RoDict *__restrict self, Dee_foreach_pair_t cb, void *arg) {
	/*real*/Dee_hash_vidx_t i;
	Dee_ssize_t temp, result = 0;
	for (i = 0; i < self->rd_vsize; ++i) {
		struct Dee_dict_item *item;
		item = &_DeeRoDict_GetRealVTab(self)[i];
		temp = (*cb)(arg, item->di_key, item->di_value);
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
	}
	return result;
err_temp:
	return temp;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rodict_getfirst(RoDict *__restrict self) {
	struct Dee_dict_item *item;
	if unlikely(DeeRoDict_IsEmpty(self))
		goto err_unbound;
	item = _DeeRoDict_GetRealVTab(self);
	return DeeSeq_OfPairv(item->di_key_and_value);
err_unbound:
	return DeeRT_ErrUnboundAttr(self, &str_first);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rodict_getlast(RoDict *__restrict self) {
	struct Dee_dict_item *item;
	if unlikely(DeeRoDict_IsEmpty(self))
		goto err_unbound;
	item = _DeeRoDict_GetRealVTab(self);
	item += self->rd_vsize - 1;
	return DeeSeq_OfPairv(item->di_key_and_value);
err_unbound:
	return DeeRT_ErrUnboundAttr(self, &str_last);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rodict_trygetfirst(RoDict *__restrict self) {
	struct Dee_dict_item *item;
	if unlikely(DeeRoDict_IsEmpty(self))
		return ITER_DONE;
	item = _DeeRoDict_GetRealVTab(self);
	return DeeSeq_OfPairv(item->di_key_and_value);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rodict_trygetlast(RoDict *__restrict self) {
	struct Dee_dict_item *item;
	if unlikely(DeeRoDict_IsEmpty(self))
		return ITER_DONE;
	item = _DeeRoDict_GetRealVTab(self);
	item += self->rd_vsize - 1;
	return DeeSeq_OfPairv(item->di_key_and_value);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rodict_getfirstkey(RoDict *__restrict self) {
	if unlikely(DeeRoDict_IsEmpty(self))
		goto err_unbound;
	return_reference(_DeeRoDict_GetRealVTab(self)[0].di_key);
err_unbound:
	return DeeRT_ErrUnboundAttrCStr(self, "firstkey");
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rodict_getlastkey(RoDict *__restrict self) {
	if unlikely(DeeRoDict_IsEmpty(self))
		goto err_unbound;
	return_reference(_DeeRoDict_GetRealVTab(self)[self->rd_vsize - 1].di_key);
err_unbound:
	return DeeRT_ErrUnboundAttrCStr(self, "lastkey");
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rodict_getfirstvalue(RoDict *__restrict self) {
	if unlikely(DeeRoDict_IsEmpty(self))
		goto err_unbound;
	return_reference(_DeeRoDict_GetRealVTab(self)[0].di_value);
err_unbound:
	return DeeRT_ErrUnboundAttrCStr(self, "firstvalue");
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rodict_getlastvalue(RoDict *__restrict self) {
	if unlikely(DeeRoDict_IsEmpty(self))
		goto err_unbound;
	return_reference(_DeeRoDict_GetRealVTab(self)[self->rd_vsize - 1].di_value);
err_unbound:
	return DeeRT_ErrUnboundAttrCStr(self, "lastvalue");
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rodict_sizeof(RoDict *__restrict self) {
	size_t result = _RoDict_SizeOf(self->rd_vsize, self->rd_hmask);
	return DeeInt_NewSize(result);
}


PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
rodict_mh_seq_foreach(RoDict *__restrict self, Dee_foreach_t cb, void *arg) {
	DREF DeeSeqPairObject *key_and_value;
	Dee_hash_vidx_t i;
	Dee_ssize_t temp, result = 0;
	key_and_value = DeeSeq_NewPairUninitialized();
	if unlikely(!key_and_value)
		goto err;
	DeeSeq_InitPair_inplace(key_and_value);
	for (i = 0; i < self->rd_vsize; ++i) {
		struct Dee_dict_item *item;
		item = &_DeeRoDict_GetRealVTab(self)[i];
		DeeSeqPair_ELEM(key_and_value)[0] = item->di_key;   /* Inherit reference (shared) */
		DeeSeqPair_ELEM(key_and_value)[1] = item->di_value; /* Inherit reference (shared) */
		temp = (*cb)(arg, (DeeObject *)key_and_value);
		if unlikely(temp < 0)
			goto err_temp_key_and_value;
		result += temp;
		ASSERT(DeeSeqPair_ELEM(key_and_value)[0] == item->di_key);
		ASSERT(DeeSeqPair_ELEM(key_and_value)[1] == item->di_value);
		/* Reset "key_and_value" for another iteration */
		if unlikely(DeeObject_IsShared(key_and_value)) {
			DeeSeqPair_DecrefSymbolic(Dee_AsObject(key_and_value));
			key_and_value = DeeSeq_NewPairUninitialized();
			if unlikely(!key_and_value)
				goto err;
			DeeSeq_InitPair_inplace(key_and_value);
		} else {
			DBG_memset(DeeSeqPair_ELEM(key_and_value), 0xcc, 2 * sizeof(DREF DeeObject *));
		}
	}
	DeeSeq_FiniPair_inplace(key_and_value);
	DeeSeq_FreePairUninitialized(key_and_value);
	return result;
err_temp_key_and_value:
	DeeSeqPair_DecrefSymbolic(Dee_AsObject(key_and_value));
	return temp;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
rodict_mh_seq_foreach_reverse(RoDict *__restrict self, Dee_foreach_t cb, void *arg) {
	DREF DeeSeqPairObject *key_and_value;
	Dee_hash_vidx_t i;
	Dee_ssize_t temp, result = 0;
	key_and_value = DeeSeq_NewPairUninitialized();
	if unlikely(!key_and_value)
		goto err;
	DeeSeq_InitPair_inplace(key_and_value);
	i = self->rd_vsize;
	while (i) {
		struct Dee_dict_item *item;
		--i;
		item = &_DeeRoDict_GetRealVTab(self)[i];
		DeeSeqPair_ELEM(key_and_value)[0] = item->di_key;   /* Inherit reference (shared) */
		DeeSeqPair_ELEM(key_and_value)[1] = item->di_value; /* Inherit reference (shared) */
		temp = (*cb)(arg, (DeeObject *)key_and_value);
		if unlikely(temp < 0)
			goto err_temp_key_and_value;
		result += temp;
		ASSERT(DeeSeqPair_ELEM(key_and_value)[0] == item->di_key);
		ASSERT(DeeSeqPair_ELEM(key_and_value)[1] == item->di_value);
		/* Reset "key_and_value" for another iteration */
		if unlikely(DeeObject_IsShared(key_and_value)) {
			DeeSeqPair_DecrefSymbolic(Dee_AsObject(key_and_value));
			key_and_value = DeeSeq_NewPairUninitialized();
			if unlikely(!key_and_value)
				goto err;
			DeeSeq_InitPair_inplace(key_and_value);
		} else {
			DBG_memset(DeeSeqPair_ELEM(key_and_value), 0xcc, 2 * sizeof(DREF DeeObject *));
		}
	}
	DeeSeq_FiniPair_inplace(key_and_value);
	DeeSeq_FreePairUninitialized(key_and_value);
	return result;
err_temp_key_and_value:
	DeeSeqPair_DecrefSymbolic(Dee_AsObject(key_and_value));
	return temp;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
rodict_mh_seq_enumerate_index(RoDict *__restrict self, Dee_seq_enumerate_index_t cb,
                              void *arg, size_t start, size_t end) {
	DREF DeeSeqPairObject *key_and_value;
	Dee_ssize_t temp, result = 0;
	key_and_value = DeeSeq_NewPairUninitialized();
	if unlikely(!key_and_value)
		goto err;
	DeeSeq_InitPair_inplace(key_and_value);
	if (end > self->rd_vsize)
		end = self->rd_vsize;
	while (start < end) {
		struct Dee_dict_item *item;
		item = &_DeeRoDict_GetRealVTab(self)[start];
		DeeSeqPair_ELEM(key_and_value)[0] = item->di_key;   /* Inherit reference (shared) */
		DeeSeqPair_ELEM(key_and_value)[1] = item->di_value; /* Inherit reference (shared) */
		temp = (*cb)(arg, start, (DeeObject *)key_and_value);
		if unlikely(temp < 0)
			goto err_temp_key_and_value;
		result += temp;
		ASSERT(DeeSeqPair_ELEM(key_and_value)[0] == item->di_key);
		ASSERT(DeeSeqPair_ELEM(key_and_value)[1] == item->di_value);
		/* Reset "key_and_value" for another iteration */
		if unlikely(DeeObject_IsShared(key_and_value)) {
			DeeSeqPair_DecrefSymbolic(Dee_AsObject(key_and_value));
			key_and_value = DeeSeq_NewPairUninitialized();
			if unlikely(!key_and_value)
				goto err;
			DeeSeq_InitPair_inplace(key_and_value);
		} else {
			DBG_memset(DeeSeqPair_ELEM(key_and_value), 0xcc, 2 * sizeof(DREF DeeObject *));
		}
		++start;
	}
	DeeSeq_FiniPair_inplace(key_and_value);
	DeeSeq_FreePairUninitialized(key_and_value);
	return result;
err_temp_key_and_value:
	DeeSeqPair_DecrefSymbolic(Dee_AsObject(key_and_value));
	return temp;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
rodict_mh_seq_enumerate_index_reverse(RoDict *__restrict self, Dee_seq_enumerate_index_t cb,
                                      void *arg, size_t start, size_t end) {
	DREF DeeSeqPairObject *key_and_value;
	Dee_ssize_t temp, result = 0;
	key_and_value = DeeSeq_NewPairUninitialized();
	if unlikely(!key_and_value)
		goto err;
	DeeSeq_InitPair_inplace(key_and_value);
	if (end > self->rd_vsize)
		end = self->rd_vsize;
	while (start < end) {
		struct Dee_dict_item *item;
		--end;
		item = &_DeeRoDict_GetRealVTab(self)[end];
		DeeSeqPair_ELEM(key_and_value)[0] = item->di_key;   /* Inherit reference (shared) */
		DeeSeqPair_ELEM(key_and_value)[1] = item->di_value; /* Inherit reference (shared) */
		temp = (*cb)(arg, end, (DeeObject *)key_and_value);
		if unlikely(temp < 0)
			goto err_temp_key_and_value;
		result += temp;
		ASSERT(DeeSeqPair_ELEM(key_and_value)[0] == item->di_key);
		ASSERT(DeeSeqPair_ELEM(key_and_value)[1] == item->di_value);
		/* Reset "key_and_value" for another iteration */
		if unlikely(DeeObject_IsShared(key_and_value)) {
			DeeSeqPair_DecrefSymbolic(Dee_AsObject(key_and_value));
			key_and_value = DeeSeq_NewPairUninitialized();
			if unlikely(!key_and_value)
				goto err;
			DeeSeq_InitPair_inplace(key_and_value);
		} else {
			DBG_memset(DeeSeqPair_ELEM(key_and_value), 0xcc, 2 * sizeof(DREF DeeObject *));
		}
	}
	DeeSeq_FiniPair_inplace(key_and_value);
	DeeSeq_FreePairUninitialized(key_and_value);
	return result;
err_temp_key_and_value:
	DeeSeqPair_DecrefSymbolic(Dee_AsObject(key_and_value));
	return temp;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rodict_mh_seq_getitem_index(RoDict *__restrict self, size_t index) {
	struct Dee_dict_item *item;
	if unlikely(index >= self->rd_vsize)
		goto err_oob;
	item = &_DeeRoDict_GetRealVTab(self)[index];
	return DeeSeq_OfPairv(item->di_key_and_value);
err_oob:
	DeeRT_ErrIndexOutOfBounds(Dee_AsObject(self), index, self->rd_vsize);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rodict_mh_seq_trygetitem_index(RoDict *__restrict self, size_t index) {
	if (index >= self->rd_vsize)
		return ITER_DONE;
	return rodict_mh_seq_getitem_index(self, index);
}


struct rodict_compare_seq_foreach_data {
	RoDict                 *rdcsfd_lhs;   /* [1..1] lhs-dict. */
	/*real*/Dee_hash_vidx_t rdcsfd_index; /* Next index into "rdcsfd_lhs" to compare against. */
};
#define RODICT_COMPARE_SEQ_FOREACH_ERROR    (-1)
#define RODICT_COMPARE_SEQ_FOREACH_EQUAL    (0)
#define RODICT_COMPARE_SEQ_FOREACH_NOTEQUAL (-2)
#define RODICT_COMPARE_SEQ_FOREACH_LESS     (-2)
#define RODICT_COMPARE_SEQ_FOREACH_GREATER  (-3)

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
rodict_compare_seq_foreach(void *arg, DeeObject *rhs_item) {
	RoDict *dict;
	int cmp_result;
	struct Dee_dict_item *lhs_item;
	struct rodict_compare_seq_foreach_data *data;
	data = (struct rodict_compare_seq_foreach_data *)arg;
	dict = data->rdcsfd_lhs;
	if unlikely(data->rdcsfd_index >= dict->rd_vsize)
		return RODICT_COMPARE_SEQ_FOREACH_LESS;
	lhs_item = &_DeeRoDict_GetRealVTab(dict)[data->rdcsfd_index];
	cmp_result = seq_docompare__lhs_vector(lhs_item->di_key_and_value, 2, rhs_item);
	if (Dee_COMPARE_ISERR(cmp_result))
		goto err;
	++data->rdcsfd_index;
	if (Dee_COMPARE_ISLO(cmp_result))
		return RODICT_COMPARE_SEQ_FOREACH_LESS;
	if (Dee_COMPARE_ISGR(cmp_result))
		return RODICT_COMPARE_SEQ_FOREACH_GREATER;
	return RODICT_COMPARE_SEQ_FOREACH_EQUAL;
err:
	return RODICT_COMPARE_SEQ_FOREACH_ERROR;
}

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
rodict_compare_eq_seq_foreach(void *arg, DeeObject *rhs_item) {
	RoDict *dict;
	int cmp_result;
	struct Dee_dict_item *lhs_item;
	struct rodict_compare_seq_foreach_data *data;
	if (!DeeType_HasNativeOperator(Dee_TYPE(rhs_item), foreach))
		return RODICT_COMPARE_SEQ_FOREACH_NOTEQUAL;
	data = (struct rodict_compare_seq_foreach_data *)arg;
	dict = data->rdcsfd_lhs;
	if unlikely(data->rdcsfd_index >= dict->rd_vsize)
		return RODICT_COMPARE_SEQ_FOREACH_NOTEQUAL;
	lhs_item = &_DeeRoDict_GetRealVTab(dict)[data->rdcsfd_index];
	cmp_result = seq_docompareeq__lhs_vector(lhs_item->di_key_and_value, 2, rhs_item);
	if (Dee_COMPARE_ISERR(cmp_result))
		goto err;
	++data->rdcsfd_index;
	if (Dee_COMPARE_ISNE(cmp_result))
		return RODICT_COMPARE_SEQ_FOREACH_NOTEQUAL;
	return RODICT_COMPARE_SEQ_FOREACH_EQUAL;
err:
	return RODICT_COMPARE_SEQ_FOREACH_ERROR;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
rodict_mh_seq_compare(RoDict *lhs, DeeObject *rhs) {
	Dee_ssize_t foreach_status;
	struct rodict_compare_seq_foreach_data data;
	data.rdcsfd_index = 0;
	data.rdcsfd_lhs   = lhs;
	foreach_status    = DeeObject_Foreach(rhs, &rodict_compare_seq_foreach, &data);
	if unlikely(foreach_status == RODICT_COMPARE_SEQ_FOREACH_ERROR)
		goto err;
	if (foreach_status == RODICT_COMPARE_SEQ_FOREACH_LESS)
		return Dee_COMPARE_LO;
	if (foreach_status == RODICT_COMPARE_SEQ_FOREACH_GREATER)
		return Dee_COMPARE_GR;
	if (data.rdcsfd_index < lhs->rd_vsize)
		return Dee_COMPARE_GR;
	return Dee_COMPARE_EQ;
err:
	return Dee_COMPARE_ERR;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
rodict_mh_seq_compare_eq(RoDict *lhs, DeeObject *rhs) {
	Dee_ssize_t foreach_status;
	struct rodict_compare_seq_foreach_data data;
	data.rdcsfd_index = 0;
	data.rdcsfd_lhs   = lhs;
	foreach_status    = DeeObject_Foreach(rhs, &rodict_compare_eq_seq_foreach, &data);
	if unlikely(foreach_status == RODICT_COMPARE_SEQ_FOREACH_ERROR)
		goto err;
	if (foreach_status == RODICT_COMPARE_SEQ_FOREACH_NOTEQUAL)
		return Dee_COMPARE_NE;
	if (data.rdcsfd_index < lhs->rd_vsize)
		return Dee_COMPARE_NE;
	return Dee_COMPARE_EQ;
err:
	return Dee_COMPARE_ERR;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
rodict_mh_seq_trycompare_eq(RoDict *lhs, DeeObject *rhs) {
	if (!DeeType_HasNativeOperator(Dee_TYPE(rhs), foreach))
		return Dee_COMPARE_NE;
	return rodict_mh_seq_compare_eq(lhs, rhs);
}


struct rodict_fromkeys_data {
	struct Dee_rodict_builder rdfkd_builder; /* Dict builder. */
	DeeObject                *rdfkd_value; /* [1..1] Value to insert, or callback to generate value. */
};

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
rodict_fromkeys_with_value(void *arg, DeeObject *key) {
	struct rodict_fromkeys_data *data = (struct rodict_fromkeys_data *)arg;
	return Dee_rodict_builder_setitem(&data->rdfkd_builder, key, data->rdfkd_value);
}

PRIVATE WUNUSED NONNULL((2)) Dee_ssize_t DCALL
rodict_fromkeys_with_valuefor(void *arg, DeeObject *key) {
	struct rodict_fromkeys_data *data = (struct rodict_fromkeys_data *)arg;
	DREF DeeObject *used_value;
	data = (struct rodict_fromkeys_data *)arg;
	used_value = DeeObject_Call(data->rdfkd_value, 1, &key);
	if unlikely(!used_value)
		goto err;
	Dee_Incref(key);
	return Dee_rodict_builder_setitem_inherited(&data->rdfkd_builder, key, used_value);
err:
	return -1;
}


PRIVATE WUNUSED NONNULL((1, 2)) DREF RoDict *DCALL
rodict_fromkeys_slow(DeeObject *keys, DeeObject *value, DeeObject *valuefor) {
	Dee_ssize_t foreach_status;
	struct rodict_fromkeys_data data;
	size_t hint;
	hint = DeeObject_SizeFast(keys);
	if unlikely(hint == (size_t)-1)
		hint = DICT_FROMSEQ_DEFAULT_HINT;
	Dee_rodict_builder_init_with_hint(&data.rdfkd_builder, hint);
	if unlikely(valuefor) {
		data.rdfkd_value = valuefor;
		foreach_status = DeeObject_Foreach(keys, &rodict_fromkeys_with_valuefor, &data);
	} else {
		data.rdfkd_value = value;
		foreach_status = DeeObject_Foreach(keys, &rodict_fromkeys_with_value, &data);
	}
	if unlikely(foreach_status < 0)
		goto err_r;
	return Dee_rodict_builder_pack(&data.rdfkd_builder);
err_r:
	Dee_rodict_builder_fini(&data.rdfkd_builder);
/*err:*/
	return NULL;
}

PRIVATE NONNULL((1)) void DCALL
rodict_destroy_keysonly(RoDict *__restrict self) {
	size_t i;
	for (i = 0; i < self->rd_vsize; ++i) {
		struct Dee_dict_item *item;
		item = &_DeeRoDict_GetRealVTab(self)[i];
		Dee_Decref(item->di_key);
		/*Dee_Decref(item->di_value);*/
	}
	_RoDict_Free(self);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
rodict_init_values_after_keys(RoDict *__restrict self, DeeObject *value, DeeObject *valuefor) {
	size_t i;
	struct Dee_dict_item *vtab = _DeeRoDict_GetRealVTab(self);
	if (valuefor) {
		for (i = 0; i < self->rd_vsize; ++i) {
			DREF DeeObject *computed_value;
			struct Dee_dict_item *item = &vtab[i];
			computed_value = DeeObject_Call(valuefor, 1, &item->di_key);
			if unlikely(!computed_value)
				goto err_values_before_i;
			item->di_value = computed_value; /* Inherit reference */
		}
	} else {
		for (i = 0; i < self->rd_vsize; ++i)
			vtab[i].di_value = value; /* Inherit reference */
		Dee_Incref_n(value, self->rd_vsize);
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

PRIVATE WUNUSED NONNULL((1, 2)) DREF RoDict *DCALL
rodict_from_dict_keys(DeeDictObject *dict_keys, DeeObject *value, DeeObject *valuefor) {
	DREF RoDict *result;
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
	sizeof_result = _RoDict_SizeOf3(vsize, hmask, dst_hidxio);
	result = _RoDict_TryMalloc(sizeof_result);
	if unlikely(!result) {
		DeeDict_LockEndRead(dict_keys);
		result = _RoDict_Malloc(sizeof_result);
		if unlikely(!result)
			goto err;
		DeeDict_LockReadAndOptimize(dict_keys);
		if unlikely(vsize != dict_keys->d_vused ||
		            hmask != dict_keys->d_hmask) {
			DeeDict_LockEndRead(dict_keys);
			_RoDict_Free(result);
			goto again;
		}
	}
	/* Copy over data as-is from the dict (no need to rehash or anything). */
	result->rd_htab = (union Dee_hash_htab *)(_DeeRoDict_GetRealVTab(result) + vsize);
	for (i = 0; i < vsize; ++i) {
		struct Dee_dict_item *dst;
		struct Dee_dict_item const *src;
		dst = &_DeeRoDict_GetRealVTab(result)[i];
		src = &_DeeDict_GetRealVTab(dict_keys)[i];
		dst->di_hash = src->di_hash;
		dst->di_key  = src->di_key;
		Dee_Incref(dst->di_key);
	}
	hmask_memcpy_and_maybe_downcast(result->rd_htab, dst_hidxio,
	                                dict_keys->d_htab, src_hidxio,
	                                hmask + 1);
	DeeDict_LockEndRead(dict_keys);
	result->rd_vsize   = vsize;
	result->rd_hmask   = hmask;
	result->rd_hidxget = Dee_hash_hidxio[dst_hidxio].hxio_get;

	/* Initialize values. */
	if unlikely(rodict_init_values_after_keys(result, value, valuefor))
		goto err_r;
	DeeObject_InitStatic(result, &DeeRoDict_Type);
	return result;
err_r:
	rodict_destroy_keysonly(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF RoDict *DCALL
rodict_from_rodict_keys(DeeRoDictObject *dict_keys, DeeObject *value, DeeObject *valuefor) {
	DREF RoDict *result;
	size_t i, sizeof_result;
	Dee_hash_hidxio_t hidxio;
	union Dee_hash_htab *final_htab;
	hidxio        = Dee_HASH_HIDXIO_FROM_VALLOC(dict_keys->rd_vsize);
	sizeof_result = _RoDict_SizeOf3(dict_keys->rd_vsize, dict_keys->rd_hmask, hidxio);
	result        = _RoDict_Malloc(sizeof_result);
	if unlikely(!result)
		goto err;
	result->rd_vsize   = dict_keys->rd_vsize;
	result->rd_hmask   = dict_keys->rd_hmask;
	result->rd_hidxget = dict_keys->rd_hidxget;

	/* Copy htab */
	final_htab = (union Dee_hash_htab *)(_DeeRoDict_GetRealVTab(result) + result->rd_vsize);
	final_htab = (union Dee_hash_htab *)memcpy(final_htab, dict_keys->rd_htab, (result->rd_hmask + 1) << hidxio);
	result->rd_htab = final_htab;

	/* Copy items (keys-only) */
	for (i = 0; i < result->rd_vsize; ++i) {
		struct Dee_dict_item *dst;
		struct Dee_dict_item const *src;
		dst = &_DeeRoDict_GetRealVTab(result)[i];
		src = &_DeeRoDict_GetRealVTab(dict_keys)[i];
		dst->di_hash = src->di_hash;
		dst->di_key  = src->di_key;
		Dee_Incref(dst->di_key);
	}

	/* Initialize values. */
	if unlikely(rodict_init_values_after_keys(result, value, valuefor))
		goto err_r;
	DeeObject_InitStatic(result, &DeeRoDict_Type);
	return result;
err_r:
	rodict_destroy_keysonly(result);
err:
	return NULL;
}

#ifdef CONFIG_EXPERIMENTAL_ORDERED_HASHSET
PRIVATE WUNUSED NONNULL((1, 2)) DREF RoDict *DCALL
rodict_from_hashset(DeeHashSetObject *set, DeeObject *value, DeeObject *valuefor) {
	DREF RoDict *result;
	size_t sizeof_result;
	size_t vsize;
	size_t i, hmask;
	Dee_hash_hidxio_t src_hidxio;
	Dee_hash_hidxio_t dst_hidxio;
again:
	DeeHashSet_LockReadAndOptimize(set);
	vsize         = set->hs_vused;
	hmask         = set->hs_hmask;
	src_hidxio    = Dee_HASH_HIDXIO_FROM_VALLOC(set->hs_valloc);
	dst_hidxio    = Dee_HASH_HIDXIO_FROM_VALLOC(vsize);
	sizeof_result = _RoDict_SizeOf3(vsize, hmask, dst_hidxio);
	result = _RoDict_TryMalloc(sizeof_result);
	if unlikely(!result) {
		DeeHashSet_LockEndRead(set);
		result = _RoDict_Malloc(sizeof_result);
		if unlikely(!result)
			goto err;
		DeeHashSet_LockReadAndOptimize(set);
		if unlikely(vsize != set->hs_vused ||
		            hmask != set->hs_hmask) {
			DeeHashSet_LockEndRead(set);
			_RoDict_Free(result);
			goto again;
		}
	}
	/* Copy over data as-is from the dict (no need to rehash or anything). */
	result->rd_htab = (union Dee_hash_htab *)(_DeeRoDict_GetRealVTab(result) + vsize);
	for (i = 0; i < vsize; ++i) {
		struct Dee_dict_item *dst;
		struct Dee_hashset_item const *src;
		dst = &_DeeRoDict_GetRealVTab(result)[i];
		src = &_DeeHashSet_GetRealVTab(set)[i];
		dst->di_hash = src->hsi_hash;
		dst->di_key  = src->hsi_key;
		Dee_Incref(dst->di_key);
	}
	hmask_memcpy_and_maybe_downcast(result->rd_htab, dst_hidxio,
	                                set->hs_htab, src_hidxio,
	                                hmask + 1);
	DeeHashSet_LockEndRead(set);
	result->rd_vsize   = vsize;
	result->rd_hmask   = hmask;
	result->rd_hidxget = Dee_hash_hidxio[dst_hidxio].hxio_get;

	/* Initialize values. */
	if unlikely(rodict_init_values_after_keys(result, value, valuefor))
		goto err_r;
	DeeObject_InitStatic(result, &DeeRoDict_Type);
	return result;
err_r:
	rodict_destroy_keysonly(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF RoDict *DCALL
rodict_from_roset(DeeRoSetObject *keys, DeeObject *value, DeeObject *valuefor) {
	DREF RoDict *result;
	size_t i, sizeof_result;
	Dee_hash_hidxio_t hidxio;
	union Dee_hash_htab *final_htab;
	hidxio        = Dee_HASH_HIDXIO_FROM_VALLOC(keys->rs_vsize);
	sizeof_result = _RoDict_SizeOf3(keys->rs_vsize, keys->rs_hmask, hidxio);
	result        = _RoDict_Malloc(sizeof_result);
	if unlikely(!result)
		goto err;
	result->rd_vsize   = keys->rs_vsize;
	result->rd_hmask   = keys->rs_hmask;
	result->rd_hidxget = keys->rs_hidxget;

	/* Copy htab */
	final_htab = (union Dee_hash_htab *)(_DeeRoDict_GetRealVTab(result) + result->rd_vsize);
	final_htab = (union Dee_hash_htab *)memcpy(final_htab, keys->rs_htab, (result->rd_hmask + 1) << hidxio);
	result->rd_htab = final_htab;

	/* Copy items (keys-only) */
	for (i = 0; i < result->rd_vsize; ++i) {
		struct Dee_dict_item *dst;
		struct Dee_hashset_item const *src;
		dst = &_DeeRoDict_GetRealVTab(result)[i];
		src = &_DeeRoSet_GetRealVTab(keys)[i];
		dst->di_hash = src->hsi_hash;
		dst->di_key  = src->hsi_key;
		Dee_Incref(dst->di_key);
	}

	/* Initialize values. */
	if unlikely(rodict_init_values_after_keys(result, value, valuefor))
		goto err_r;
	DeeObject_InitStatic(result, &DeeRoDict_Type);
	return result;
err_r:
	rodict_destroy_keysonly(result);
err:
	return NULL;
}
#endif /* CONFIG_EXPERIMENTAL_ORDERED_HASHSET */



PRIVATE WUNUSED NONNULL((1, 2)) DREF RoDict *DCALL
rodict_fromkeys(DeeObject *keys, DeeObject *value, DeeObject *valuefor) {
	DeeTypeObject *tp_keys = Dee_TYPE(keys);

	/* Optimizations for special, known keys types. */
#ifdef CONFIG_EXPERIMENTAL_ORDERED_HASHSET
	if (tp_keys == &DeeHashSet_Type) /* Special optimization when "keys" is a HashSet: Duplicate its control structures */
		return rodict_from_hashset((DeeHashSetObject *)keys, value, valuefor);
	if (tp_keys == &DeeRoSet_Type) /* Special optimization when "keys" is a RoSet: Duplicate its control structures */
		return rodict_from_roset((DeeRoSetObject *)keys, value, valuefor);
#endif /* CONFIG_EXPERIMENTAL_ORDERED_HASHSET */

	if (tp_keys == &DefaultSequence_MapKeys_Type) {
		/* Special optimization when "keys" are the keys of another Dict/RoDict */
		DefaultSequence_MapProxy *proxy = (DefaultSequence_MapProxy *)keys;
		DeeObject *mapping_of_keys = proxy->dsmp_map;
		DeeTypeObject *tp_mapping_of_keys = Dee_TYPE(mapping_of_keys);
		if (tp_mapping_of_keys == &DeeDict_Type)
			return rodict_from_dict_keys((DeeDictObject *)mapping_of_keys, value, valuefor);
		if (tp_mapping_of_keys == &DeeRoDict_Type)
			return rodict_from_rodict_keys((DeeRoDictObject *)mapping_of_keys, value, valuefor);
	}

	return rodict_fromkeys_slow(keys, value, valuefor);
}

PRIVATE WUNUSED NONNULL((1)) DREF RoDict *DCALL
rodict_fromkeys_f(DeeTypeObject *UNUSED(dict), size_t argc,
                  DeeObject *const *argv, DeeObject *kw) {
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("fromkeys", params: "
	DeeObject *keys: ?DSet;
	DeeObject *value = Dee_None;
	DeeObject *valuefor:?DCallable = NULL;
", docStringPrefix: "rodict");]]]*/
#define rodict_fromkeys_params "keys:?DSet,value=!N,valuefor?:?DCallable"
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
	return rodict_fromkeys(args.keys, args.value, args.valuefor);
err:
	return NULL;
}

PRIVATE struct type_seq rodict_seq = {
	/* .tp_iter                         = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&rodict_iter,
	/* .tp_sizeob                       = */ DEFIMPL(&default__sizeob__with__size),
	/* .tp_contains                     = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&rodict_contains,
	/* .tp_getitem                      = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&rodict_getitem,
	/* .tp_delitem                      = */ DEFIMPL(&default__map_operator_delitem__unsupported),
	/* .tp_setitem                      = */ DEFIMPL(&default__map_operator_setitem__unsupported),
	/* .tp_getrange                     = */ DEFIMPL_UNSUPPORTED(&default__getrange__unsupported),
	/* .tp_delrange                     = */ DEFIMPL_UNSUPPORTED(&default__delrange__unsupported),
	/* .tp_setrange                     = */ DEFIMPL_UNSUPPORTED(&default__setrange__unsupported),
	/* .tp_foreach                      = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&rodict_mh_seq_foreach,
	/* .tp_foreach_pair                 = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_pair_t, void *))&rodict_foreach_pair,
	/* .tp_bounditem                    = */ (int (DCALL *)(DeeObject *, DeeObject *))&rodict_bounditem,
	/* .tp_hasitem                      = */ (int (DCALL *)(DeeObject *, DeeObject *))&rodict_hasitem,
	/* .tp_size                         = */ (size_t (DCALL *)(DeeObject *__restrict))&rodict_size,
	/* .tp_size_fast                    = */ (size_t (DCALL *)(DeeObject *__restrict))&rodict_size_fast,
	/* .tp_getitem_index                = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&rodict_getitem_index,
	/* .tp_getitem_index_fast           = */ NULL,
	/* .tp_delitem_index                = */ DEFIMPL(&default__map_operator_delitem_index__unsupported),
	/* .tp_setitem_index                = */ DEFIMPL(&default__map_operator_setitem_index__unsupported),
	/* .tp_bounditem_index              = */ (int (DCALL *)(DeeObject *, size_t))&rodict_bounditem_index,
	/* .tp_hasitem_index                = */ (int (DCALL *)(DeeObject *, size_t))&rodict_hasitem_index,
	/* .tp_getrange_index               = */ DEFIMPL_UNSUPPORTED(&default__getrange_index__unsupported),
	/* .tp_delrange_index               = */ DEFIMPL_UNSUPPORTED(&default__delrange_index__unsupported),
	/* .tp_setrange_index               = */ DEFIMPL_UNSUPPORTED(&default__setrange_index__unsupported),
	/* .tp_getrange_index_n             = */ DEFIMPL_UNSUPPORTED(&default__getrange_index_n__unsupported),
	/* .tp_delrange_index_n             = */ DEFIMPL_UNSUPPORTED(&default__delrange_index_n__unsupported),
	/* .tp_setrange_index_n             = */ DEFIMPL_UNSUPPORTED(&default__setrange_index_n__unsupported),
	/* .tp_trygetitem                   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&rodict_trygetitem,
	/* .tp_trygetitem_index             = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&rodict_trygetitem_index,
	/* .tp_trygetitem_string_hash       = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, Dee_hash_t))&rodict_trygetitem_string_hash,
	/* .tp_getitem_string_hash          = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, Dee_hash_t))&rodict_getitem_string_hash,
	/* .tp_delitem_string_hash          = */ DEFIMPL(&default__map_operator_delitem_string_hash__unsupported),
	/* .tp_setitem_string_hash          = */ DEFIMPL(&default__map_operator_setitem_string_hash__unsupported),
	/* .tp_bounditem_string_hash        = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&rodict_bounditem_string_hash,
	/* .tp_hasitem_string_hash          = */ (int (DCALL *)(DeeObject *, char const *, Dee_hash_t))&rodict_hasitem_string_hash,
	/* .tp_trygetitem_string_len_hash   = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&rodict_trygetitem_string_len_hash,
	/* .tp_getitem_string_len_hash      = */ (DREF DeeObject *(DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&rodict_getitem_string_len_hash,
	/* .tp_delitem_string_len_hash      = */ DEFIMPL(&default__map_operator_delitem_string_len_hash__unsupported),
	/* .tp_setitem_string_len_hash      = */ DEFIMPL(&default__map_operator_setitem_string_len_hash__unsupported),
	/* .tp_bounditem_string_len_hash    = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&rodict_bounditem_string_len_hash,
	/* .tp_hasitem_string_len_hash      = */ (int (DCALL *)(DeeObject *, char const *, size_t, Dee_hash_t))&rodict_hasitem_string_len_hash,
	/* .tp_asvector                     = */ NULL,
	/* .tp_asvector_nothrow             = */ NULL,
	/* .tp_trygetitemnr                 = */ (DeeObject *(DCALL *)(DeeObject *__restrict, /*string*/ DeeObject *__restrict))&rodict_trygetitemnr,
	/* .tp_trygetitemnr_string_hash     = */ (DeeObject *(DCALL *)(DeeObject *__restrict, char const *__restrict, Dee_hash_t))&rodict_trygetitemnr_string_hash,
	/* .tp_trygetitemnr_string_len_hash = */ (DeeObject *(DCALL *)(DeeObject *__restrict, char const *__restrict, size_t, Dee_hash_t))&rodict_trygetitemnr_string_len_hash,
};


#define D_TKey   "?O"
#define D_TValue "?O"
#define D_TItem  "?T2" D_TKey D_TValue

PRIVATE struct type_getset tpconst rodict_getsets[] = {
	TYPE_GETTER_BOUND(STR_first, &rodict_getfirst, &rodict_nonempty_as_bound, "->" D_TItem),
	TYPE_GETTER_BOUND(STR_last, &rodict_getlast, &rodict_nonempty_as_bound, "->" D_TItem),
	TYPE_GETTER_BOUND("firstkey", &rodict_getfirstkey, &rodict_nonempty_as_bound, "->" D_TKey),
	TYPE_GETTER_BOUND("lastkey", &rodict_getlastkey, &rodict_nonempty_as_bound, "->" D_TKey),
	TYPE_GETTER_BOUND("firstvalue", &rodict_getfirstvalue, &rodict_nonempty_as_bound, "->" D_TValue),
	TYPE_GETTER_BOUND("lastvalue", &rodict_getlastvalue, &rodict_nonempty_as_bound, "->" D_TValue),

	TYPE_GETTER_AB(STR_cached, &DeeObject_NewRef, "->?."),
	TYPE_GETTER_AB(STR_frozen, &DeeObject_NewRef, "->?."),
	TYPE_GETTER_AB_F("__sizeof__", &rodict_sizeof, METHOD_FNOREFESCAPE, "->?Dint"),
	TYPE_GETTER_AB_F("__hidxio__", &rodict___hidxio__, METHOD_FNOREFESCAPE,
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

INTDEF struct type_member tpconst rodict_members[]; /* "INTERN" because shared with "./roset.c" */
INTERN_TPCONST struct type_member tpconst rodict_members[] = {
	TYPE_MEMBER_FIELD_DOC("__vsize__", STRUCT_CONST | STRUCT_ATOMIC | STRUCT_SIZE_T,
	                      offsetof(RoDict, rd_vsize), "## of used vtab slots (excluding deleted slots; same as ?#{op:size})"),
	TYPE_MEMBER_FIELD_DOC("__hmask__", STRUCT_CONST | STRUCT_ATOMIC | STRUCT_SIZE_T,
	                      offsetof(RoDict, rd_hmask), "Currently active hash-mask"),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst rodict_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &RoDictIterator_Type),
	TYPE_MEMBER_CONST(STR_Frozen, &DeeRoDict_Type),
	TYPE_MEMBER_CONST("__map_getitem_always_bound__", Dee_True),
	TYPE_MEMBER_CONST("__seq_getitem_always_bound__", Dee_True), /* Must be specified because we also define "__seq_getitem__" */
	TYPE_MEMBER_END
};

PRIVATE struct type_method tpconst rodict_methods[] = {
//	TYPE_KWMETHOD("byhash", &rodict_byhash, DOC_GET(map_byhash_doc)), /* TODO */
	TYPE_METHOD_HINTREF(__seq_iter__),
	TYPE_METHOD_HINTREF(__seq_size__),
	TYPE_METHOD_HINTREF(__seq_getitem__),
	TYPE_METHOD_HINTREF(__seq_enumerate__),
	TYPE_METHOD_HINTREF(__seq_compare__),
	TYPE_METHOD_HINTREF(__seq_compare_eq__),
	TYPE_METHOD_END
};

PRIVATE struct type_method_hint tpconst rodict_method_hints[] = {
	/* Operators for "Dict as Sequence" (specifically defined because dicts are ordered) */
	TYPE_METHOD_HINT_F(seq_foreach_reverse, &rodict_mh_seq_foreach_reverse, METHOD_FNOREFESCAPE | METHOD_FCONSTCALL),
	TYPE_METHOD_HINT_F(seq_enumerate_index_reverse, &rodict_mh_seq_enumerate_index_reverse, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_SET_CONSTCONTAINS | METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_trygetfirst, &rodict_trygetfirst, METHOD_FNOREFESCAPE | METHOD_FCONSTCALL),
	TYPE_METHOD_HINT_F(seq_trygetlast, &rodict_trygetlast, METHOD_FNOREFESCAPE | METHOD_FCONSTCALL),
	TYPE_METHOD_HINT_F(set_trygetfirst, &rodict_trygetfirst, METHOD_FNOREFESCAPE | METHOD_FCONSTCALL), /* Must also be set because the "first" getset defines __seq_first__ and __set_first__ */
	TYPE_METHOD_HINT_F(set_trygetlast, &rodict_trygetlast, METHOD_FNOREFESCAPE | METHOD_FCONSTCALL),   /* Must also be set because the "last" getset defines __seq_last__ and __set_last__ */
	TYPE_METHOD_HINT_F(seq_enumerate_index, &rodict_mh_seq_enumerate_index, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_SET_CONSTCONTAINS | METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_operator_iter, &rodict_iter, METHOD_FNOREFESCAPE | METHOD_FCONSTCALL),
	TYPE_METHOD_HINT_F(seq_operator_foreach, &rodict_mh_seq_foreach, METHOD_FNOREFESCAPE | METHOD_FCONSTCALL),
	TYPE_METHOD_HINT_F(seq_operator_foreach_pair, &rodict_foreach_pair, METHOD_FNOREFESCAPE | METHOD_FCONSTCALL),
	TYPE_METHOD_HINT_F(seq_operator_size, &rodict_size, METHOD_FNOREFESCAPE | METHOD_FCONSTCALL),
	TYPE_METHOD_HINT_F(seq_operator_getitem_index, &rodict_mh_seq_getitem_index, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_SET_CONSTCONTAINS | METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_operator_trygetitem_index, &rodict_mh_seq_trygetitem_index, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_SET_CONSTCONTAINS | METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_operator_compare_eq, &rodict_mh_seq_compare_eq, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_SET_CONSTCMPEQ | METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_operator_compare, &rodict_mh_seq_compare, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_SET_CONSTCMP | METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_F(seq_operator_trycompare_eq, &rodict_mh_seq_trycompare_eq, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_SET_CONSTCMPEQ | METHOD_FNOREFESCAPE),
	TYPE_METHOD_HINT_END
};

PRIVATE struct type_method tpconst rodict_class_methods[] = {
	TYPE_KWMETHOD_F("fromkeys", &rodict_fromkeys_f, METHOD_FNOREFESCAPE,
	                "(" rodict_fromkeys_params ")->?.\n"
	                "Construct a new ?. from @keys, and @value (or ${valuefor(key)}) as value."),
	TYPE_METHOD_END
};

PRIVATE struct type_operator const rodict_operators[] = {
	TYPE_OPERATOR_FLAGS(OPERATOR_0000_CONSTRUCTOR, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST),
	TYPE_OPERATOR_FLAGS(OPERATOR_0007_REPR, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_THISELEM_CONSTREPR | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0008_BOOL, METHOD_FCONSTCALL | METHOD_FNOTHROW | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0028_HASH, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_THISELEM_CONSTHASH | METHOD_FNOREFESCAPE),
	//TODO:TYPE_OPERATOR_FLAGS(OPERATOR_0029_EQ, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_SET_CONSTCMPEQ | METHOD_FNOREFESCAPE),
	//TODO:TYPE_OPERATOR_FLAGS(OPERATOR_002A_NE, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_SET_CONSTCMPEQ | METHOD_FNOREFESCAPE),
	//TODO:TYPE_OPERATOR_FLAGS(OPERATOR_002B_LO, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_SET_CONSTCMP | METHOD_FNOREFESCAPE),
	//TODO:TYPE_OPERATOR_FLAGS(OPERATOR_002C_LE, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_SET_CONSTCMP | METHOD_FNOREFESCAPE),
	//TODO:TYPE_OPERATOR_FLAGS(OPERATOR_002D_GR, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_SET_CONSTCMP | METHOD_FNOREFESCAPE),
	//TODO:TYPE_OPERATOR_FLAGS(OPERATOR_002E_GE, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_SET_CONSTCMP | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_002F_ITER, METHOD_FCONSTCALL | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0030_SIZE, METHOD_FCONSTCALL | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0031_CONTAINS, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_SET_CONSTCONTAINS | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0032_GETITEM, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_SET_CONSTCONTAINS | METHOD_FNOREFESCAPE),
};

PUBLIC DeeTypeObject DeeRoDict_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_RoDict",
	/* .tp_doc      = */ DOC("Read-only / frozen variant of ?DDict\n"
	                         "\n"

	                         "()\n"
	                         "Returns an empty ?.\n"
	                         "\n"

	                         "(dict:?DDict)\n"
	                         "(map:?M?O?O)\n"
	                         "(seq:?ST2?O?O)\n"
	                         "Convert the given dict/map/seq into an ?. and return it"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FVARIABLE | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_KW, /* RoDict offers KW-support (because items cannot be removed, values can be loaded as NOREF) */
	/* .tp_base     = */ &DeeMap_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_VAR(
			/* tp_ctor:        */ &rodict_ctor,
			/* tp_copy_ctor:   */ &DeeObject_NewRef,
			/* tp_any_ctor:    */ &rodict_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &rodict_serialize,
			/* tp_free:        */ NULL
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&rodict_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ DEFIMPL(&object_str),
		/* .tp_repr      = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool      = */ (int (DCALL *)(DeeObject *__restrict))&rodict_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&rodict_printrepr,
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&rodict_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__2E23147A197C0EE6),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__4CF94EE41850B0EF), /* TODO: &rodict_cmp */
	/* .tp_seq           = */ &rodict_seq,
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__C6F8E138F179B5AD),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ rodict_methods,
	/* .tp_getsets       = */ rodict_getsets,
	/* .tp_members       = */ rodict_members,
	/* .tp_class_methods = */ rodict_class_methods,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ rodict_class_members,
	/* .tp_method_hints  = */ rodict_method_hints,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
	/* .tp_mro           = */ NULL,
	/* .tp_operators     = */ rodict_operators,
	/* .tp_operators_size= */ COMPILER_LENOF(rodict_operators),
};

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_RODICT_C */
