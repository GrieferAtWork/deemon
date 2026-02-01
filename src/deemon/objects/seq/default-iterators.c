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
#ifndef GUARD_DEEMON_OBJECTS_SEQ_DEFAULT_ITERATORS_C
#define GUARD_DEEMON_OBJECTS_SEQ_DEFAULT_ITERATORS_C 1

#include <deemon/api.h>

#include <deemon/alloc.h>              /* DeeObject_MALLOC, Dee_TYPE_CONSTRUCTOR_INIT_FIXED, Dee_TYPE_CONSTRUCTOR_INIT_FIXED_GC */
#include <deemon/arg.h>                /* DeeArg_Unpack*, UNPuSIZ */
#include <deemon/computed-operators.h>
#include <deemon/error-rt.h>           /* DeeRT_ErrIndexOverflow */
#include <deemon/error.h>              /* DeeError_* */
#include <deemon/format.h>             /* PCKuSIZ */
#include <deemon/int.h>                /* DeeInt_NewSize */
#include <deemon/none-operator.h>      /* _DeeNone_retsm1_1 */
#include <deemon/none.h>               /* DeeNone_NewRef */
#include <deemon/object.h>             /* DREF, DeeObject, DeeObject_*, DeeTypeObject, Dee_AsObject, Dee_COMPARE_ERR, Dee_Compare, Dee_CompareNe, Dee_Decref, Dee_Incref, Dee_TYPE, Dee_hash_t, Dee_return_compareT, Dee_return_compare_if_ne, Dee_visit_t, ITER_DONE, ITER_ISOK, OBJECT_HEAD_INIT */
#include <deemon/operator-hints.h>     /* DeeNO_nextkey_t, DeeNO_nextpair_t, DeeType_RequireSupportedNativeOperator */
#include <deemon/seq.h>                /* DeeIterator_Type, DeeSeq_Unpack */
#include <deemon/serial.h>             /* DeeSerial*, Dee_seraddr_t */
#include <deemon/tuple.h>              /* DeeTuple* */
#include <deemon/type.h>               /* DeeObject_Init, DeeType_Type, Dee_Visit, OPERATOR_GETITEM, OPERATOR_ITERNEXT, STRUCT_*, TF_NONE, TP_F*, TYPE_*, type_* */
#include <deemon/util/atomic.h>        /* atomic_* */
#include <deemon/util/hash.h>          /* DeeObject_HashGeneric, DeeObject_Id, Dee_HashCombine */
#include <deemon/util/lock.h>          /* Dee_atomic_lock_init */

#include "../../runtime/runtime_error.h"
#include "../../runtime/strings.h"
#include "../generic-proxy.h"
#include "default-iterators.h"
#include "default-map-proxy.h"

#include <stddef.h> /* NULL, offsetof, size_t */

DECL_BEGIN

/************************************************************************/
/* DefaultIterator_WithGetItemIndex_Type                                */
/* DefaultIterator_WithSizeAndGetItemIndex_Type                         */
/* DefaultIterator_WithSizeAndGetItemIndexFast_Type                     */
/* DefaultIterator_WithSizeAndTryGetItemIndex_Type                      */
/************************************************************************/

STATIC_ASSERT(offsetof(DefaultIterator_WithGetItemIndex, digi_seq) == offsetof(DefaultIterator_WithSizeAndGetItemIndex, disgi_seq));
STATIC_ASSERT(offsetof(DefaultIterator_WithGetItemIndex, digi_index) == offsetof(DefaultIterator_WithSizeAndGetItemIndex, disgi_index));

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
di_gi_copy(DefaultIterator_WithGetItemIndex *__restrict self,
           DefaultIterator_WithGetItemIndex *__restrict other) {
	self->digi_seq = other->digi_seq;
	Dee_Incref(self->digi_seq);
	self->digi_tp_getitem_index = other->digi_tp_getitem_index;
	self->digi_index = atomic_read(&other->digi_index);
	return 0;
}

#define di_sgif_copy di_sgi_copy
#define di_stgi_copy di_sgi_copy
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
di_sgi_copy(DefaultIterator_WithSizeAndGetItemIndex *__restrict self,
            DefaultIterator_WithSizeAndGetItemIndex *__restrict other) {
	self->disgi_seq = other->disgi_seq;
	Dee_Incref(self->disgi_seq);
	self->disgi_tp_getitem_index = other->disgi_tp_getitem_index;
	self->disgi_index = atomic_read(&other->disgi_index);
	self->disgi_end   = other->disgi_end;
	return 0;
}

#define di_sgi_deepcopy  di_gi_deepcopy
#define di_sgif_deepcopy di_gi_deepcopy
#define di_stgi_deepcopy di_gi_deepcopy
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
di_gi_deepcopy(DefaultIterator_WithGetItemIndex *__restrict self,
               DefaultIterator_WithGetItemIndex *__restrict other) {
	self->digi_seq = DeeObject_DeepCopy(other->digi_seq);
	if unlikely(!self->digi_seq)
		goto err;
	self->digi_tp_getitem_index = other->digi_tp_getitem_index;
	self->digi_index = atomic_read(&other->digi_index);
	return 0;
err:
	return -1;
}

#define di_sgi_serialize  di_gi_serialize
#define di_sgif_serialize di_gi_serialize
#define di_stgi_serialize di_gi_serialize
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
di_gi_serialize(DefaultIterator_WithGetItemIndex *__restrict self,
                DeeSerial *__restrict writer, Dee_seraddr_t addr) {
#define ADDROF(field) (addr + offsetof(DefaultIterator_WithGetItemIndex, field))
	int result = generic_proxy__serialize((ProxyObject *)self, writer, addr);
	if likely(result == 0)
		result = DeeSerial_PutFuncPtr(writer, ADDROF(digi_tp_getitem_index), self->digi_tp_getitem_index);
	if likely(result == 0) {
		DefaultIterator_WithGetItemIndex *out;
		out = DeeSerial_Addr2Mem(writer, addr, DefaultIterator_WithGetItemIndex);
		out->digi_index = atomic_read(&self->digi_index);
	}
	return result;
#undef ADDROF
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
di_gi_init(DefaultIterator_WithGetItemIndex *__restrict self,
           size_t argc, DeeObject *const *argv) {
	DeeTypeObject *seqtyp;
	if (DeeArg_Unpack(argc, argv, "o" UNPuSIZ ":_IterWithGetItemIndex",
	                  &self->digi_seq, &self->digi_index))
		goto err;
	seqtyp = Dee_TYPE(self->digi_seq);
	self->digi_tp_getitem_index = DeeType_RequireSupportedNativeOperator(seqtyp, getitem_index);
	if unlikely(!self->digi_tp_getitem_index)
		goto err_no_getitem;
	Dee_Incref(self->digi_seq);
	return 0;
err_no_getitem:
	err_unimplemented_operator(seqtyp, OPERATOR_GETITEM);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
di_sgi_init(DefaultIterator_WithSizeAndGetItemIndex *__restrict self,
            size_t argc, DeeObject *const *argv) {
	DeeTypeObject *seqtyp;
	self->disgi_index = 0;
	self->disgi_end   = (size_t)-1;
#define di_sgi_init_params "objWithGetItem,index=!0,end?:?Dint"
	if (DeeArg_Unpack(argc, argv, "o|" UNPuSIZ UNPuSIZ ":_IterWithSizeAndGetItemIndex",
	                  &self->disgi_seq, &self->disgi_index, &self->disgi_end))
		goto err;
	if (self->disgi_end == (size_t)-1) {
		self->disgi_end = DeeObject_Size(self->disgi_seq);
		if unlikely(self->disgi_end == (size_t)-1)
			goto err;
	}
	seqtyp = Dee_TYPE(self->disgi_seq);
	self->disgi_tp_getitem_index = DeeType_RequireSupportedNativeOperator(seqtyp, getitem_index);
	if unlikely(!self->disgi_tp_getitem_index)
		goto err_no_getitem;
	Dee_Incref(self->disgi_seq);
	return 0;
err_no_getitem:
	err_unimplemented_operator(seqtyp, OPERATOR_GETITEM);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
di_sgif_init(DefaultIterator_WithSizeAndGetItemIndex *__restrict self,
             size_t argc, DeeObject *const *argv) {
	DeeTypeObject *seqtyp;
	self->disgi_index = 0;
	self->disgi_end   = (size_t)-1;
#define di_sgif_init_params "objWithGetItemIndexFast,index=!0,end?:?Dint"
	if (DeeArg_Unpack(argc, argv, "o|" UNPuSIZ UNPuSIZ ":_IterWithSizeAndGetItemIndexFast",
	                  &self->disgi_seq, &self->disgi_index, &self->disgi_end))
		goto err;
	if (self->disgi_end == (size_t)-1) {
		self->disgi_end = DeeObject_Size(self->disgi_seq);
		if unlikely(self->disgi_end == (size_t)-1)
			goto err;
	}
	seqtyp = Dee_TYPE(self->disgi_seq);
	if (!seqtyp->tp_seq || !seqtyp->tp_seq->tp_getitem_index_fast)
		goto err_no_getitem;
	self->disgi_tp_getitem_index = seqtyp->tp_seq->tp_getitem_index_fast;
	Dee_Incref(self->disgi_seq);
	return 0;
err_no_getitem:
	err_unimplemented_operator(seqtyp, OPERATOR_GETITEM);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
di_stgi_init(DefaultIterator_WithSizeAndGetItemIndex *__restrict self,
             size_t argc, DeeObject *const *argv) {
	DeeTypeObject *seqtyp;
	if (DeeArg_Unpack(argc, argv, "o" UNPuSIZ UNPuSIZ ":_IterWithSizeAndTryGetItemIndex",
	                  &self->disgi_seq, &self->disgi_index, &self->disgi_end))
		goto err;
	seqtyp = Dee_TYPE(self->disgi_seq);
	self->disgi_tp_getitem_index = DeeType_RequireSupportedNativeOperator(seqtyp, trygetitem_index);
	if unlikely(!self->disgi_tp_getitem_index)
		goto err_no_getitem;
	Dee_Incref(self->disgi_seq);
	return 0;
err_no_getitem:
	err_unimplemented_operator(seqtyp, OPERATOR_GETITEM);
err:
	return -1;
}

#define di_gi_fini    generic_proxy__fini
#define di_sgi_fini   generic_proxy__fini
#define di_sgif_fini  generic_proxy__fini
#define di_stgi_fini  generic_proxy__fini
#define di_gi_visit   generic_proxy__visit
#define di_sgi_visit  generic_proxy__visit
#define di_sgif_visit generic_proxy__visit
#define di_stgi_visit generic_proxy__visit

#define di_sgi_hash  di_gi_hash
#define di_sgif_hash di_gi_hash
#define di_stgi_hash di_gi_hash
PRIVATE WUNUSED NONNULL((1)) Dee_hash_t DCALL
di_gi_hash(DefaultIterator_WithGetItemIndex *self) {
	return Dee_HashCombine(DeeObject_HashGeneric(self->digi_seq),
	                       atomic_read(&self->digi_index));
}

#define di_sgi_compare  di_gi_compare
#define di_sgif_compare di_gi_compare
#define di_stgi_compare di_gi_compare
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
di_gi_compare(DefaultIterator_WithGetItemIndex *self,
              DefaultIterator_WithGetItemIndex *other) {
	if (DeeObject_AssertTypeExact(other, Dee_TYPE(self)))
		goto err;
	Dee_return_compare_if_ne(DeeObject_Id(self->digi_seq),
		                     DeeObject_Id(other->digi_seq));
	Dee_return_compareT(size_t,
	                    atomic_read(&self->digi_index),
	                    atomic_read(&other->digi_index));
err:
	return Dee_COMPARE_ERR;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
di_gi_iter_next(DefaultIterator_WithGetItemIndex *__restrict self) {
	size_t old_index, new_index;
	DREF DeeObject *result;
again:
	old_index = atomic_read(&self->digi_index);
	new_index = old_index;
	for (;;) {
		result = (*self->digi_tp_getitem_index)(self->digi_seq, new_index);
		if (result)
			break;
		if (DeeError_Catch(&DeeError_UnboundItem)) {
			++new_index;
		} else if (DeeError_Catch(&DeeError_IndexError)) {
			return ITER_DONE;
		} else {
			goto err;
		}
	}
	if (!atomic_cmpxch_or_write(&self->digi_index, old_index, new_index + 1)) {
		Dee_Decref(result);
		goto again;
	}
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
di_gip_iter_next(DefaultIterator_WithGetItemIndex *__restrict self) {
	size_t old_index, new_index;
	DREF DeeObject *result;
again:
	old_index = atomic_read(&self->digi_index);
	new_index = old_index;
	for (;;) {
		result = (*self->digi_tp_getitem_index)(self->digi_seq, new_index);
		if (result)
			break;
		if (DeeError_Catch(&DeeError_UnboundItem)) {
			++new_index;
		} else if (DeeError_Catch(&DeeError_IndexError)) {
			return ITER_DONE;
		} else {
			goto err;
		}
	}
	if (!atomic_cmpxch_or_write(&self->digi_index, old_index, new_index + 1)) {
		Dee_Decref(result);
		goto again;
	}
	return DeeTuple_Newf(PCKuSIZ "O", new_index, result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
di_sgi_iter_next(DefaultIterator_WithSizeAndGetItemIndex *__restrict self) {
	size_t old_index, new_index;
	DREF DeeObject *result;
again:
	old_index = atomic_read(&self->disgi_index);
	new_index = old_index;
	for (;;) {
		if (new_index >= self->disgi_end)
			return ITER_DONE;
		result = (*self->disgi_tp_getitem_index)(self->disgi_seq, new_index);
		if (result)
			break;
		if (DeeError_Catch(&DeeError_UnboundItem)) {
			++new_index;
		} else if (DeeError_Catch(&DeeError_IndexError)) {
			return ITER_DONE;
		} else {
			goto err;
		}
	}
	if (!atomic_cmpxch_or_write(&self->disgi_index, old_index, new_index + 1)) {
		Dee_Decref(result);
		goto again;
	}
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
di_sgip_iter_next(DefaultIterator_WithSizeAndGetItemIndex *__restrict self) {
	size_t old_index, new_index;
	DREF DeeObject *result;
again:
	old_index = atomic_read(&self->disgi_index);
	new_index = old_index;
	for (;;) {
		if (new_index >= self->disgi_end)
			return ITER_DONE;
		result = (*self->disgi_tp_getitem_index)(self->disgi_seq, new_index);
		if (result)
			break;
		if (DeeError_Catch(&DeeError_UnboundItem)) {
			++new_index;
		} else if (DeeError_Catch(&DeeError_IndexError)) {
			return ITER_DONE;
		} else {
			goto err;
		}
	}
	if (!atomic_cmpxch_or_write(&self->disgi_index, old_index, new_index + 1)) {
		Dee_Decref(result);
		goto again;
	}
	return DeeTuple_Newf(PCKuSIZ "O", new_index, result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
di_sgif_iter_next(DefaultIterator_WithSizeAndGetItemIndex *__restrict self) {
	size_t old_index, new_index;
	DREF DeeObject *result;
again:
	old_index = atomic_read(&self->disgi_index);
	new_index = old_index;
	for (;;) {
		if (new_index >= self->disgi_end)
			return ITER_DONE;
		result = (*self->disgi_tp_getitem_index)(self->disgi_seq, new_index);
		if (result)
			break;
		/* Unbound item */
		++new_index;
	}
	if (!atomic_cmpxch_or_write(&self->disgi_index, old_index, new_index + 1)) {
		Dee_Decref(result);
		goto again;
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
di_sgifp_iter_next(DefaultIterator_WithSizeAndGetItemIndex *__restrict self) {
	size_t old_index, new_index;
	DREF DeeObject *result;
again:
	old_index = atomic_read(&self->disgi_index);
	new_index = old_index;
	for (;;) {
		if (new_index >= self->disgi_end)
			return ITER_DONE;
		result = (*self->disgi_tp_getitem_index)(self->disgi_seq, new_index);
		if (result)
			break;
		/* Unbound item */
		++new_index;
	}
	if (!atomic_cmpxch_or_write(&self->disgi_index, old_index, new_index + 1)) {
		Dee_Decref(result);
		goto again;
	}
	return DeeTuple_Newf(PCKuSIZ "O", new_index, result);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
di_stgi_iter_next(DefaultIterator_WithSizeAndGetItemIndex *__restrict self) {
	size_t old_index, new_index;
	DREF DeeObject *result;
again:
	old_index = atomic_read(&self->disgi_index);
	new_index = old_index;
	for (;;) {
		if (new_index >= self->disgi_end)
			return ITER_DONE;
		result = (*self->disgi_tp_getitem_index)(self->disgi_seq, new_index);
		if (result != ITER_DONE) {
			if (result)
				break;
			goto err;
		}
		++new_index;
	}
	if (!atomic_cmpxch_or_write(&self->disgi_index, old_index, new_index + 1)) {
		Dee_Decref(result);
		goto again;
	}
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
di_stgip_iter_next(DefaultIterator_WithSizeAndGetItemIndex *__restrict self) {
	size_t old_index, new_index;
	DREF DeeObject *result;
again:
	old_index = atomic_read(&self->disgi_index);
	new_index = old_index;
	for (;;) {
		if (new_index >= self->disgi_end)
			return ITER_DONE;
		result = (*self->disgi_tp_getitem_index)(self->disgi_seq, new_index);
		if (result != ITER_DONE) {
			if (result)
				break;
			goto err;
		}
		++new_index;
	}
	if (!atomic_cmpxch_or_write(&self->disgi_index, old_index, new_index + 1)) {
		Dee_Decref(result);
		goto again;
	}
	return DeeTuple_Newf(PCKuSIZ "O", new_index, result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
di_sgif_getindex(DefaultIterator_WithSizeAndGetItemIndex *__restrict self) {
	size_t index = atomic_read(&self->disgi_index);
	return DeeInt_NewSize(index);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
di_sgif_setindex(DefaultIterator_WithSizeAndGetItemIndex *self, DeeObject *value) {
	size_t index;
	if (DeeObject_AsSize(value, &index))
		goto err;
	if (index > self->disgi_end)
		index = self->disgi_end;
	atomic_write(&self->disgi_index, index);
	return 0;
err:
	DeeRT_ErrIndexOverflow(self);
	return -1;
}

#define di_sgi_cmp  di_gi_cmp
#define di_sgif_cmp di_gi_cmp
#define di_stgi_cmp di_gi_cmp
PRIVATE struct type_cmp di_gi_cmp = {
	/* .tp_hash          = */ (Dee_hash_t (DCALL *)(DeeObject *))&di_gi_hash,
	/* .tp_compare_eq    = */ (int (DCALL *)(DeeObject *, DeeObject *))&di_gi_compare,
	/* .tp_compare       = */ (int (DCALL *)(DeeObject *, DeeObject *))&di_gi_compare,
	/* .tp_trycompare_eq = */ DEFIMPL(&default__trycompare_eq__with__compare_eq),
	/* .tp_eq            = */ DEFIMPL(&default__eq__with__compare_eq),
	/* .tp_ne            = */ DEFIMPL(&default__ne__with__compare_eq),
	/* .tp_lo            = */ DEFIMPL(&default__lo__with__compare),
	/* .tp_le            = */ DEFIMPL(&default__le__with__compare),
	/* .tp_gr            = */ DEFIMPL(&default__gr__with__compare),
	/* .tp_ge            = */ DEFIMPL(&default__ge__with__compare),
};

PRIVATE struct type_member tpconst di_sgi_members[] = {
#define di_stgi_members di_sgi_members
	TYPE_MEMBER_FIELD("__end__", STRUCT_CONST | STRUCT_SIZE_T, offsetof(DefaultIterator_WithSizeAndGetItemIndex, disgi_end)),
#define di_gi_members (di_sgi_members + 1)
	TYPE_MEMBER_FIELD("__index__", STRUCT_ATOMIC | STRUCT_SIZE_T, offsetof(DefaultIterator_WithGetItemIndex, digi_index)),
	TYPE_MEMBER_FIELD(STR_seq, STRUCT_OBJECT, offsetof(DefaultIterator_WithGetItemIndex, digi_seq)),
	TYPE_MEMBER_END,
};

PRIVATE struct type_member tpconst di_sgif_members[] = {
	TYPE_MEMBER_FIELD("__end__", STRUCT_CONST | STRUCT_SIZE_T, offsetof(DefaultIterator_WithSizeAndGetItemIndex, disgi_end)),
	TYPE_MEMBER_FIELD(STR_seq, STRUCT_OBJECT, offsetof(DefaultIterator_WithGetItemIndex, digi_seq)),
	TYPE_MEMBER_END,
};

PRIVATE struct type_getset tpconst di_sgif_getsets[] = {
	TYPE_GETSET_AB("__index__", &di_sgif_getindex, NULL, &di_sgif_setindex, "->?Dint"),
	TYPE_GETSET_END,
};

INTERN DeeTypeObject DefaultIterator_WithGetItemIndex_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_IterWithGetItemIndex",
	/* .tp_doc      = */ DOC("(objWithGetItem,index:?Dint)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ DefaultIterator_WithGetItemIndex,
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ &di_gi_copy,
			/* tp_deep_ctor:   */ &di_gi_deepcopy,
			/* tp_any_ctor:    */ &di_gi_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &di_gi_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&di_gi_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ DEFIMPL(&iterator_bool),
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&iterator_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&di_gi_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__EFED4BCD35433C3C),
	/* .tp_cmp           = */ &di_gi_cmp,
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&di_gi_iter_next,
	/* .tp_iterator      = */ DEFIMPL(&default__tp_iterator__863AC70046E4B6B0),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ di_gi_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL(&iterator_next),
	/* .tp_callable      = */ DEFIMPL(&default__tp_callable__83C59FA7626CABBE),
};

INTERN DeeTypeObject DefaultIterator_WithGetItemIndexPair_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_IterWithGetItemIndexPair",
	/* .tp_doc      = */ DOC("(objWithGetItem,index:?Dint)\n"
	                         "\n"
	                         "next->?T2?Dint?O"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ DefaultIterator_WithGetItemIndex,
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ &di_gi_copy,
			/* tp_deep_ctor:   */ &di_gi_deepcopy,
			/* tp_any_ctor:    */ &di_gi_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &di_gi_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&di_gi_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ DEFIMPL(&iterator_bool),
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&iterator_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&di_gi_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__EFED4BCD35433C3C),
	/* .tp_cmp           = */ &di_gi_cmp,
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&di_gip_iter_next,
	/* .tp_iterator      = */ DEFIMPL(&default__tp_iterator__863AC70046E4B6B0),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ di_gi_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL(&iterator_next),
	/* .tp_callable      = */ DEFIMPL(&default__tp_callable__83C59FA7626CABBE),
};

INTERN DeeTypeObject DefaultIterator_WithSizeAndGetItemIndex_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_IterWithSizeAndGetItemIndex",
	/* .tp_doc      = */ DOC("(" di_sgi_init_params ")"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ DefaultIterator_WithSizeAndGetItemIndex,
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ &di_sgi_copy,
			/* tp_deep_ctor:   */ &di_sgi_deepcopy,
			/* tp_any_ctor:    */ &di_sgi_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &di_sgi_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&di_sgi_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ DEFIMPL(&iterator_bool),
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&iterator_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&di_sgi_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__EFED4BCD35433C3C),
	/* .tp_cmp           = */ &di_sgi_cmp,
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&di_sgi_iter_next,
	/* .tp_iterator      = */ DEFIMPL(&default__tp_iterator__863AC70046E4B6B0),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ di_sgi_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL(&iterator_next),
	/* .tp_callable      = */ DEFIMPL(&default__tp_callable__83C59FA7626CABBE),
};

INTERN DeeTypeObject DefaultIterator_WithSizeAndGetItemIndexPair_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_IterWithSizeAndGetItemIndexPair",
	/* .tp_doc      = */ DOC("(" di_sgi_init_params ")\n"
	                         "\n"
	                         "next->?T2?Dint?O"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ DefaultIterator_WithSizeAndGetItemIndex,
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ &di_sgi_copy,
			/* tp_deep_ctor:   */ &di_sgi_deepcopy,
			/* tp_any_ctor:    */ &di_sgi_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &di_sgi_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&di_sgi_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ DEFIMPL(&iterator_bool),
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&iterator_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&di_sgi_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__EFED4BCD35433C3C),
	/* .tp_cmp           = */ &di_sgi_cmp,
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&di_sgip_iter_next,
	/* .tp_iterator      = */ DEFIMPL(&default__tp_iterator__863AC70046E4B6B0),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ di_sgi_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL(&iterator_next),
	/* .tp_callable      = */ DEFIMPL(&default__tp_callable__83C59FA7626CABBE),
};

INTERN DeeTypeObject DefaultIterator_WithSizeAndGetItemIndexFast_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_IterWithSizeAndGetItemIndexFast",
	/* .tp_doc      = */ DOC("(" di_sgif_init_params ")"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ DefaultIterator_WithSizeAndGetItemIndex,
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ &di_sgif_copy,
			/* tp_deep_ctor:   */ &di_sgif_deepcopy,
			/* tp_any_ctor:    */ &di_sgif_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &di_sgif_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&di_sgif_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ DEFIMPL(&iterator_bool),
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&iterator_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&di_sgif_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__EFED4BCD35433C3C),
	/* .tp_cmp           = */ &di_sgif_cmp,
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&di_sgif_iter_next,
	/* .tp_iterator      = */ DEFIMPL(&default__tp_iterator__863AC70046E4B6B0),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ di_sgif_getsets,
	/* .tp_members       = */ di_sgif_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL(&iterator_next),
	/* .tp_callable      = */ DEFIMPL(&default__tp_callable__83C59FA7626CABBE),
};

INTERN DeeTypeObject DefaultIterator_WithSizeAndGetItemIndexFastPair_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_IterWithSizeAndGetItemIndexFastPair",
	/* .tp_doc      = */ DOC("(" di_sgif_init_params ")\n"
	                         "\n"
	                         "next->?T2?Dint?O"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ DefaultIterator_WithSizeAndGetItemIndex,
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ &di_sgif_copy,
			/* tp_deep_ctor:   */ &di_sgif_deepcopy,
			/* tp_any_ctor:    */ &di_sgif_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &di_sgif_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&di_sgif_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ DEFIMPL(&iterator_bool),
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&iterator_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&di_sgif_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__EFED4BCD35433C3C),
	/* .tp_cmp           = */ &di_sgif_cmp,
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&di_sgifp_iter_next,
	/* .tp_iterator      = */ DEFIMPL(&default__tp_iterator__863AC70046E4B6B0),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ di_sgif_getsets,
	/* .tp_members       = */ di_sgif_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL(&iterator_next),
	/* .tp_callable      = */ DEFIMPL(&default__tp_callable__83C59FA7626CABBE),
};

INTERN DeeTypeObject DefaultIterator_WithSizeAndTryGetItemIndex_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_IterWithSizeAndTryGetItemIndex",
	/* .tp_doc      = */ DOC("(objWithGetItem,index:?Dint,end:?Dint)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ DefaultIterator_WithSizeAndGetItemIndex,
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ &di_stgi_copy,
			/* tp_deep_ctor:   */ &di_stgi_deepcopy,
			/* tp_any_ctor:    */ &di_stgi_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &di_stgi_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&di_stgi_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ DEFIMPL(&iterator_bool),
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&iterator_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&di_stgi_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__EFED4BCD35433C3C),
	/* .tp_cmp           = */ &di_stgi_cmp,
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&di_stgi_iter_next,
	/* .tp_iterator      = */ DEFIMPL(&default__tp_iterator__863AC70046E4B6B0),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ di_stgi_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL(&iterator_next),
	/* .tp_callable      = */ DEFIMPL(&default__tp_callable__83C59FA7626CABBE),
};

INTERN DeeTypeObject DefaultIterator_WithSizeAndTryGetItemIndexPair_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_IterWithSizeAndTryGetItemIndexPair",
	/* .tp_doc      = */ DOC("(objWithGetItem,index:?Dint,end:?Dint)\n"
	                         "\n"
	                         "next->?T2?Dint?O"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ DefaultIterator_WithSizeAndGetItemIndex,
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ &di_stgi_copy,
			/* tp_deep_ctor:   */ &di_stgi_deepcopy,
			/* tp_any_ctor:    */ &di_stgi_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &di_stgi_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&di_stgi_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ DEFIMPL(&iterator_bool),
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&iterator_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&di_stgi_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__EFED4BCD35433C3C),
	/* .tp_cmp           = */ &di_stgi_cmp,
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&di_stgip_iter_next,
	/* .tp_iterator      = */ DEFIMPL(&default__tp_iterator__863AC70046E4B6B0),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ di_stgi_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL(&iterator_next),
	/* .tp_callable      = */ DEFIMPL(&default__tp_callable__83C59FA7626CABBE),
};
























/************************************************************************/
/* DefaultIterator_WithGetItem_Type                                     */
/************************************************************************/

PRIVATE WUNUSED NONNULL((1)) int DCALL
di_g_init(DefaultIterator_WithGetItem *__restrict self,
          size_t argc, DeeObject *const *argv) {
	DeeTypeObject *seqtyp;
	DeeArg_Unpack2(err, argc, argv, "_IterWithGetItem",
	                &self->dig_seq, &self->dig_index);
	seqtyp = Dee_TYPE(self->dig_seq);
	self->dig_tp_getitem = DeeType_RequireSupportedNativeOperator(seqtyp, getitem);
	if unlikely(!self->dig_tp_getitem)
		goto err_no_getitem;
	Dee_Incref(self->dig_seq);
	Dee_Incref(self->dig_index);
	Dee_atomic_lock_init(&self->dig_lock);
	return 0;
err_no_getitem:
	err_unimplemented_operator(seqtyp, OPERATOR_GETITEM);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
di_g_copy(DefaultIterator_WithGetItem *__restrict self,
          DefaultIterator_WithGetItem *__restrict other) {
	DREF DeeObject *index;
	DefaultIterator_WithGetItem_LockAcquire(other);
	index = other->dig_index;
	Dee_Incref(index);
	DefaultIterator_WithGetItem_LockRelease(other);
	if unlikely((index = DeeObject_CopyInherited(index)) == NULL)
		goto err;
	self->dig_index = index;
	Dee_atomic_lock_init(&self->dig_lock);
	self->dig_tp_getitem = other->dig_tp_getitem;
	self->dig_seq        = other->dig_seq;
	Dee_Incref(self->dig_seq);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
di_g_deepcopy(DefaultIterator_WithGetItem *__restrict self,
              DefaultIterator_WithGetItem *__restrict other) {
	DREF DeeObject *index;
	self->dig_seq = DeeObject_DeepCopy(other->dig_seq);
	if unlikely(!self->dig_seq)
		goto err;
	DefaultIterator_WithGetItem_LockAcquire(other);
	index = other->dig_index;
	Dee_Incref(index);
	DefaultIterator_WithGetItem_LockRelease(other);
	self->dig_index = index; /* Inherit reference */
	Dee_atomic_lock_init(&self->dig_lock);
	self->dig_tp_getitem = other->dig_tp_getitem;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
di_g_deepload(DefaultIterator_WithGetItem *__restrict self) {
	return DeeObject_InplaceDeepCopyWithLock(&self->dig_index, &self->dig_lock);
}

PRIVATE NONNULL((1)) void DCALL
di_g_clear(DefaultIterator_WithGetItem *__restrict self) {
	DREF DeeObject *index;
	DefaultIterator_WithGetItem_LockAcquire(self);
	index           = self->dig_index;
	self->dig_index = DeeNone_NewRef();
	DefaultIterator_WithGetItem_LockRelease(self);
	Dee_Decref(index);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
di_g_serialize(DefaultIterator_WithGetItem *__restrict self,
               DeeSerial *__restrict writer, Dee_seraddr_t addr) {
#define ADDROF(field) (addr + offsetof(DefaultIterator_WithGetItem, field))
	DREF DeeObject *index;
	Dee_atomic_lock_init(&DeeSerial_Addr2Mem(writer, addr, DefaultIterator_WithGetItem)->dig_lock);
	if unlikely(generic_proxy__serialize((ProxyObject *)self, writer, addr))
		goto err;
	if (DeeSerial_PutFuncPtr(writer, ADDROF(dig_tp_getitem), self->dig_tp_getitem))
		goto err;
	if (DeeSerial_PutObject(writer, ADDROF(dig_seq), self->dig_seq))
		goto err;
	DefaultIterator_WithGetItem_LockAcquire(self);
	index = self->dig_index;
	Dee_Incref(index);
	DefaultIterator_WithGetItem_LockRelease(self);
	return DeeSerial_PutObjectInherited(writer, ADDROF(dig_index), index);
err:
	return -1;
#undef ADDROF
}

PRIVATE NONNULL((1)) void DCALL
di_g_fini(DefaultIterator_WithGetItem *__restrict self) {
	Dee_Decref(self->dig_seq);
	Dee_Decref(self->dig_index);
}

PRIVATE NONNULL((1, 2)) void DCALL
di_g_visit(DefaultIterator_WithGetItem *__restrict self,
           Dee_visit_t proc, void *arg) {
	Dee_Visit(self->dig_seq);
	DefaultIterator_WithGetItem_LockAcquire(self);
	Dee_Visit(self->dig_index);
	DefaultIterator_WithGetItem_LockRelease(self);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
di_g_getindex(DefaultIterator_WithGetItem *__restrict self) {
	DREF DeeObject *result;
	DefaultIterator_WithGetItem_LockAcquire(self);
	result = self->dig_index;
	Dee_Incref(result);
	DefaultIterator_WithGetItem_LockRelease(self);
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
di_g_setindex(DefaultIterator_WithGetItem *self, DeeObject *index) {
	DREF DeeObject *oldindex;
	Dee_Incref(index);
	DefaultIterator_WithGetItem_LockAcquire(self);
	oldindex = self->dig_index;
	self->dig_index = index;
	DefaultIterator_WithGetItem_LockRelease(self);
	Dee_Decref(oldindex);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) Dee_hash_t DCALL
di_g_hash(DefaultIterator_WithGetItem *self) {
	DREF DeeObject *index;
	Dee_hash_t result = DeeObject_HashGeneric(self->dig_seq);
	DefaultIterator_WithGetItem_LockAcquire(self);
	index = self->dig_index;
	Dee_Incref(index);
	DefaultIterator_WithGetItem_LockRelease(self);
	result = Dee_HashCombine(result, DeeObject_HashInherited(index));
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
di_g_compare(DefaultIterator_WithGetItem *self,
             DefaultIterator_WithGetItem *other) {
	int result;
	DREF DeeObject *my_index, *ot_index;
	if (DeeObject_AssertTypeExact(other, Dee_TYPE(self)))
		goto err;
	if (self->dig_seq != other->dig_seq) {
		return Dee_CompareNe(DeeObject_Id(self->dig_seq),
		                     DeeObject_Id(other->dig_seq));
	}
	DefaultIterator_WithGetItem_LockAcquire(self);
	my_index = self->dig_index;
	Dee_Incref(my_index);
	DefaultIterator_WithGetItem_LockRelease(self);
	DefaultIterator_WithGetItem_LockAcquire(other);
	ot_index = other->dig_index;
	Dee_Incref(ot_index);
	DefaultIterator_WithGetItem_LockRelease(other);
	result = DeeObject_Compare(my_index, ot_index);
	Dee_Decref(my_index);
	Dee_Decref(ot_index);
	return result;
err:
	return Dee_COMPARE_ERR;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
di_g_iter_next(DefaultIterator_WithGetItem *__restrict self) {
	DeeObject *old_index;
	DREF DeeObject *new_index, *result;
again:
	DefaultIterator_WithGetItem_LockAcquire(self);
	old_index = self->dig_index;
	Dee_Incref(old_index);
	DefaultIterator_WithGetItem_LockRelease(self);
	new_index = old_index; /* Inherit reference */
	for (;;) {
		result = (*self->dig_tp_getitem)(self->dig_seq, new_index);
		if (result)
			break;
		if (DeeError_Catch(&DeeError_UnboundItem)) {
			if (DeeObject_Inc(&new_index))
				goto err_new_index;
		} else if (DeeError_Catch(&DeeError_IndexError)) {
			Dee_Decref(new_index);
			return ITER_DONE;
		} else {
			goto err_new_index;
		}
	}
	if (DeeObject_Inc(&new_index))
		goto err_new_index_result;
	DefaultIterator_WithGetItem_LockAcquire(self);
	if unlikely(self->dig_index != old_index) {
		DefaultIterator_WithGetItem_LockRelease(self);
		Dee_Decref(new_index);
		Dee_Decref(result);
		goto again;
	}
	self->dig_index = new_index; /* Inherit (x2) */
	DefaultIterator_WithGetItem_LockRelease(self);
	Dee_Decref(old_index);
	return result;
err_new_index_result:
	Dee_Decref(result);
err_new_index:
	Dee_Decref(new_index);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
di_gp_iter_next(DefaultIterator_WithGetItem *__restrict self) {
	DeeObject *old_index;
	DREF DeeObject *new_index, *result;
again:
	DefaultIterator_WithGetItem_LockAcquire(self);
	old_index = self->dig_index;
	Dee_Incref(old_index);
	DefaultIterator_WithGetItem_LockRelease(self);
	new_index = old_index; /* Inherit reference */
	for (;;) {
		result = (*self->dig_tp_getitem)(self->dig_seq, new_index);
		if (result)
			break;
		if (DeeError_Catch(&DeeError_UnboundItem)) {
			if (DeeObject_Inc(&new_index))
				goto err_new_index;
		} else if (DeeError_Catch(&DeeError_IndexError)) {
			Dee_Decref(new_index);
			return ITER_DONE;
		} else {
			goto err_new_index;
		}
	}
	if (DeeObject_Inc(&new_index))
		goto err_new_index_result;
	DefaultIterator_WithGetItem_LockAcquire(self);
	if unlikely(self->dig_index != old_index) {
		DefaultIterator_WithGetItem_LockRelease(self);
		Dee_Decref(new_index);
		Dee_Decref(result);
		goto again;
	}
	Dee_Incref(new_index);
	self->dig_index = new_index; /* Inherit (x2) */
	DefaultIterator_WithGetItem_LockRelease(self);
	Dee_Decref(old_index);
	return DeeTuple_Newf("OO", new_index, result);
err_new_index_result:
	Dee_Decref(result);
err_new_index:
	Dee_Decref(new_index);
	return NULL;
}

PRIVATE struct type_cmp di_g_cmp = {
	/* .tp_hash          = */ (Dee_hash_t (DCALL *)(DeeObject *))&di_g_hash,
	/* .tp_compare_eq    = */ (int (DCALL *)(DeeObject *, DeeObject *))&di_g_compare,
	/* .tp_compare       = */ (int (DCALL *)(DeeObject *, DeeObject *))&di_g_compare,
	/* .tp_trycompare_eq = */ DEFIMPL(&default__trycompare_eq__with__compare_eq),
	/* .tp_eq            = */ DEFIMPL(&default__eq__with__compare_eq),
	/* .tp_ne            = */ DEFIMPL(&default__ne__with__compare_eq),
	/* .tp_lo            = */ DEFIMPL(&default__lo__with__compare),
	/* .tp_le            = */ DEFIMPL(&default__le__with__compare),
	/* .tp_gr            = */ DEFIMPL(&default__gr__with__compare),
	/* .tp_ge            = */ DEFIMPL(&default__ge__with__compare),
};

PRIVATE struct type_gc tpconst di_g_gc = {
	/* .tp_clear = */ (void (DCALL *)(DeeObject *__restrict))&di_g_clear
};

PRIVATE struct type_member tpconst di_g_members[] = {
	TYPE_MEMBER_FIELD(STR_seq, STRUCT_OBJECT, offsetof(DefaultIterator_WithGetItem, dig_seq)),
	TYPE_MEMBER_END,
};

PRIVATE struct type_getset tpconst di_g_getsets[] = {
	TYPE_GETSET_NODOC("__index__", &di_g_getindex, NULL, &di_g_setindex),
	TYPE_GETSET_END,
};

INTERN DeeTypeObject DefaultIterator_WithGetItem_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_IterWithGetItem",
	/* .tp_doc      = */ DOC("(objWithGetItem,index)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL | TP_FGC,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED_GC(
			/* T:              */ DefaultIterator_WithGetItem,
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ &di_g_copy,
			/* tp_deep_ctor:   */ &di_g_deepcopy,
			/* tp_any_ctor:    */ &di_g_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &di_g_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&di_g_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
		/* .tp_deepload    = */ (int (DCALL *)(DeeObject *__restrict))&di_g_deepload,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ DEFIMPL(&iterator_bool),
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&iterator_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&di_g_visit,
	/* .tp_gc            = */ &di_g_gc,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__EFED4BCD35433C3C),
	/* .tp_cmp           = */ &di_g_cmp,
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&di_g_iter_next,
	/* .tp_iterator      = */ DEFIMPL(&default__tp_iterator__863AC70046E4B6B0),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ di_g_getsets,
	/* .tp_members       = */ di_g_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL(&iterator_next),
	/* .tp_callable      = */ DEFIMPL(&default__tp_callable__83C59FA7626CABBE),
};

INTERN DeeTypeObject DefaultIterator_WithGetItemPair_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_IterWithGetItemPair",
	/* .tp_doc      = */ DOC("(objWithGetItem,index)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL | TP_FGC,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED_GC(
			/* T:              */ DefaultIterator_WithGetItem,
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ &di_g_copy,
			/* tp_deep_ctor:   */ &di_g_deepcopy,
			/* tp_any_ctor:    */ &di_g_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &di_g_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&di_g_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
		/* .tp_deepload    = */ (int (DCALL *)(DeeObject *__restrict))&di_g_deepload,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ DEFIMPL(&iterator_bool),
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&iterator_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&di_g_visit,
	/* .tp_gc            = */ &di_g_gc,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__EFED4BCD35433C3C),
	/* .tp_cmp           = */ &di_g_cmp,
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&di_gp_iter_next,
	/* .tp_iterator      = */ DEFIMPL(&default__tp_iterator__863AC70046E4B6B0),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ di_g_getsets,
	/* .tp_members       = */ di_g_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL(&iterator_next),
	/* .tp_callable      = */ DEFIMPL(&default__tp_callable__83C59FA7626CABBE),
};























/************************************************************************/
/* DefaultIterator_WithSizeObAndGetItem_Type                            */
/************************************************************************/

STATIC_ASSERT(offsetof(DefaultIterator_WithSizeObAndGetItem, disg_seq) == offsetof(DefaultIterator_WithGetItem, dig_seq));
STATIC_ASSERT(offsetof(DefaultIterator_WithSizeObAndGetItem, disg_index) == offsetof(DefaultIterator_WithGetItem, dig_index));
#ifndef CONFIG_NO_THREADS
STATIC_ASSERT(offsetof(DefaultIterator_WithSizeObAndGetItem, disg_lock) == offsetof(DefaultIterator_WithGetItem, dig_lock));
#endif /* !CONFIG_NO_THREADS */

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
di_sg_copy(DefaultIterator_WithSizeObAndGetItem *__restrict self,
           DefaultIterator_WithSizeObAndGetItem *__restrict other) {
	DREF DeeObject *index;
	DefaultIterator_WithSizeAndGetItem_LockAcquire(other);
	index = other->disg_index;
	Dee_Incref(index);
	DefaultIterator_WithSizeAndGetItem_LockRelease(other);
	if unlikely((index = DeeObject_CopyInherited(index)) == NULL)
		goto err;
	self->disg_index = index;
	Dee_atomic_lock_init(&self->disg_lock);
	self->disg_tp_getitem = other->disg_tp_getitem;
	self->disg_seq        = other->disg_seq;
	Dee_Incref(self->disg_seq);
	self->disg_end = other->disg_end;
	Dee_Incref(other->disg_end);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
di_sg_init(DefaultIterator_WithSizeObAndGetItem *__restrict self,
           size_t argc, DeeObject *const *argv) {
	DeeTypeObject *seqtyp;
	DeeArg_Unpack3(err, argc, argv, "_IterWithSizeObAndGetItem",
	               &self->disg_seq, &self->disg_index, &self->disg_end);
	seqtyp = Dee_TYPE(self->disg_seq);
	self->disg_tp_getitem = DeeType_RequireSupportedNativeOperator(seqtyp, getitem);
	if unlikely(!self->disg_tp_getitem)
		goto err_no_getitem;
	Dee_atomic_lock_init(&self->disg_lock);
	Dee_Incref(self->disg_seq);
	Dee_Incref(self->disg_index);
	Dee_Incref(self->disg_end);
	return 0;
err_no_getitem:
	err_unimplemented_operator(seqtyp, OPERATOR_GETITEM);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
di_sg_deepcopy(DefaultIterator_WithSizeObAndGetItem *__restrict self,
               DefaultIterator_WithSizeObAndGetItem *__restrict other) {
	self->disg_end = DeeObject_DeepCopy(other->disg_end);
	if unlikely(!self->disg_end)
		goto err;
	self->disg_seq = DeeObject_DeepCopy(other->disg_seq);
	if unlikely(!self->disg_seq)
		goto err_end_copy;
	Dee_atomic_lock_init(&self->disg_lock);
	self->disg_tp_getitem = other->disg_tp_getitem;
	DefaultIterator_WithSizeAndGetItem_LockAcquire(other);
	self->disg_index = other->disg_index;
	Dee_Incref(self->disg_index);
	DefaultIterator_WithSizeAndGetItem_LockRelease(other);
	return 0;
err_end_copy:
	Dee_Decref(self->disg_end);
err:
	return -1;
}

STATIC_ASSERT(offsetof(DefaultIterator_WithSizeObAndGetItem, disg_index) ==
              offsetof(DefaultIterator_WithGetItem, dig_index));
#ifndef CONFIG_NO_THREADS
STATIC_ASSERT(offsetof(DefaultIterator_WithSizeObAndGetItem, disg_lock) ==
              offsetof(DefaultIterator_WithGetItem, dig_lock));
#endif /* !CONFIG_NO_THREADS */
#define di_sg_deepload di_g_deepload

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
di_sg_serialize(DefaultIterator_WithSizeObAndGetItem *__restrict self,
                DeeSerial *__restrict writer, Dee_seraddr_t addr) {
#define ADDROF(field) (addr + offsetof(DefaultIterator_WithSizeObAndGetItem, field))
	DREF DeeObject *index;
	Dee_atomic_lock_init(&DeeSerial_Addr2Mem(writer, addr, DefaultIterator_WithSizeObAndGetItem)->disg_lock);
	if unlikely(generic_proxy__serialize((ProxyObject *)self, writer, addr))
		goto err;
	if (DeeSerial_PutFuncPtr(writer, ADDROF(disg_tp_getitem), self->disg_tp_getitem))
		goto err;
	if (DeeSerial_PutObject(writer, ADDROF(disg_seq), self->disg_seq))
		goto err;
	if (DeeSerial_PutObject(writer, ADDROF(disg_end), self->disg_end))
		goto err;
	DefaultIterator_WithSizeAndGetItem_LockAcquire(self);
	index = self->disg_index;
	Dee_Incref(index);
	DefaultIterator_WithSizeAndGetItem_LockRelease(self);
	return DeeSerial_PutObjectInherited(writer, ADDROF(disg_index), index);
err:
	return -1;
#undef ADDROF
}

PRIVATE NONNULL((1)) void DCALL
di_sg_fini(DefaultIterator_WithSizeObAndGetItem *__restrict self) {
	Dee_Decref(self->disg_seq);
	Dee_Decref(self->disg_index);
	Dee_Decref(self->disg_end);
}

PRIVATE NONNULL((1, 2)) void DCALL
di_sg_visit(DefaultIterator_WithSizeObAndGetItem *__restrict self,
            Dee_visit_t proc, void *arg) {
	Dee_Visit(self->disg_seq);
	DefaultIterator_WithSizeAndGetItem_LockAcquire(self);
	Dee_Visit(self->disg_index);
	DefaultIterator_WithSizeAndGetItem_LockRelease(self);
	Dee_Visit(self->disg_end);
}

#define di_sg_compare di_g_compare

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
di_sg_iter_next(DefaultIterator_WithSizeObAndGetItem *__restrict self) {
	DeeObject *old_index;
	DREF DeeObject *new_index, *result;
again:
	DefaultIterator_WithSizeAndGetItem_LockAcquire(self);
	old_index = self->disg_index;
	Dee_Incref(old_index);
	DefaultIterator_WithSizeAndGetItem_LockRelease(self);
	new_index = old_index; /* Inherit reference */
	for (;;) {
		int temp = DeeObject_CmpGeAsBool(new_index, self->disg_end);
		if (temp != 0) {
			if unlikely(temp < 0)
				goto err_new_index;
			Dee_Decref(new_index);
			return ITER_DONE;
		}
		result = (*self->disg_tp_getitem)(self->disg_seq, new_index);
		if (result)
			break;
		if (DeeError_Catch(&DeeError_UnboundItem)) {
			if (DeeObject_Inc(&new_index))
				goto err_new_index;
		} else if (DeeError_Catch(&DeeError_IndexError)) {
			Dee_Decref(new_index);
			return ITER_DONE;
		} else {
			goto err_new_index;
		}
	}
	if (DeeObject_Inc(&new_index))
		goto err_new_index_result;
	DefaultIterator_WithSizeAndGetItem_LockAcquire(self);
	if unlikely(self->disg_index != old_index) {
		DefaultIterator_WithSizeAndGetItem_LockRelease(self);
		Dee_Decref(new_index);
		Dee_Decref(result);
		goto again;
	}
	self->disg_index = new_index; /* Inherit (x2) */
	DefaultIterator_WithSizeAndGetItem_LockRelease(self);
	Dee_Decref(old_index);
	return result;
err_new_index_result:
	Dee_Decref(result);
err_new_index:
	Dee_Decref(new_index);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
di_sgp_iter_next(DefaultIterator_WithSizeObAndGetItem *__restrict self) {
	DeeObject *old_index;
	DREF DeeObject *new_index, *result_index, *result;
again:
	DefaultIterator_WithSizeAndGetItem_LockAcquire(self);
	old_index = self->disg_index;
	Dee_Incref(old_index);
	DefaultIterator_WithSizeAndGetItem_LockRelease(self);
	new_index = old_index; /* Inherit reference */
	for (;;) {
		int temp = DeeObject_CmpGeAsBool(new_index, self->disg_end);
		if (temp != 0) {
			if unlikely(temp < 0)
				goto err_new_index;
			Dee_Decref(new_index);
			return ITER_DONE;
		}
		result = (*self->disg_tp_getitem)(self->disg_seq, new_index);
		if (result)
			break;
		if (DeeError_Catch(&DeeError_UnboundItem)) {
			if (DeeObject_Inc(&new_index))
				goto err_new_index;
		} else if (DeeError_Catch(&DeeError_IndexError)) {
			Dee_Decref(new_index);
			return ITER_DONE;
		} else {
			goto err_new_index;
		}
	}
	result_index = DeeObject_Copy(new_index);
	if unlikely(!result_index)
		goto err_new_index_result;
	if (DeeObject_Inc(&new_index))
		goto err_new_index_result_result_index;
	DefaultIterator_WithSizeAndGetItem_LockAcquire(self);
	if unlikely(self->disg_index != old_index) {
		DefaultIterator_WithSizeAndGetItem_LockRelease(self);
		Dee_Decref(result_index);
		Dee_Decref(new_index);
		Dee_Decref(result);
		goto again;
	}
	self->disg_index = new_index; /* Inherit (x2) */
	DefaultIterator_WithSizeAndGetItem_LockRelease(self);
	Dee_Decref(old_index);
	return DeeTuple_Newf("OO", result_index, result);
err_new_index_result_result_index:
	Dee_Decref(result_index);
err_new_index_result:
	Dee_Decref(result);
err_new_index:
	Dee_Decref(new_index);
	return NULL;
}

#define di_sg_cmp     di_g_cmp
#define di_sg_gc      di_g_gc
#define di_sg_members di_g_members
#define di_sg_getsets di_g_getsets

INTERN DeeTypeObject DefaultIterator_WithSizeObAndGetItem_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_IterWithSizeObAndGetItem",
	/* .tp_doc      = */ DOC("(objWithGetItem,index,end)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL | TP_FGC,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED_GC(
			/* T:              */ DefaultIterator_WithSizeObAndGetItem,
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ &di_sg_copy,
			/* tp_deep_ctor:   */ &di_sg_deepcopy,
			/* tp_any_ctor:    */ &di_sg_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &di_sg_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&di_sg_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
		/* .tp_deepload    = */ (int (DCALL *)(DeeObject *__restrict))&di_sg_deepload,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ DEFIMPL(&iterator_bool),
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&iterator_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&di_sg_visit,
	/* .tp_gc            = */ &di_sg_gc,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__EFED4BCD35433C3C),
	/* .tp_cmp           = */ &di_sg_cmp,
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&di_sg_iter_next,
	/* .tp_iterator      = */ DEFIMPL(&default__tp_iterator__863AC70046E4B6B0),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ di_sg_getsets,
	/* .tp_members       = */ di_sg_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL(&iterator_next),
	/* .tp_callable      = */ DEFIMPL(&default__tp_callable__83C59FA7626CABBE),
};

INTERN DeeTypeObject DefaultIterator_WithSizeObAndGetItemPair_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_IterWithSizeAndGetItemPair",
	/* .tp_doc      = */ DOC("(objWithGetItem,index,end)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL | TP_FGC,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED_GC(
			/* T:              */ DefaultIterator_WithSizeObAndGetItem,
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ &di_sg_copy,
			/* tp_deep_ctor:   */ &di_sg_deepcopy,
			/* tp_any_ctor:    */ &di_sg_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &di_sg_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&di_sg_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
		/* .tp_deepload    = */ (int (DCALL *)(DeeObject *__restrict))&di_sg_deepload,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ DEFIMPL(&iterator_bool),
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&iterator_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&di_sg_visit,
	/* .tp_gc            = */ &di_sg_gc,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__EFED4BCD35433C3C),
	/* .tp_cmp           = */ &di_sg_cmp,
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&di_sgp_iter_next,
	/* .tp_iterator      = */ DEFIMPL(&default__tp_iterator__863AC70046E4B6B0),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ di_sg_getsets,
	/* .tp_members       = */ di_sg_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL(&iterator_next),
	/* .tp_callable      = */ DEFIMPL(&default__tp_callable__83C59FA7626CABBE),
};























/************************************************************************/
/* DefaultIterator_WithNextAndLimit_Type                                */
/************************************************************************/

STATIC_ASSERT(offsetof(DefaultIterator_WithNextAndLimit, dinl_iter) == offsetof(DefaultIterator_WithSizeAndGetItemIndex, disgi_seq));
#define di_nl_fini  di_sgi_fini
#define di_nl_visit di_sgi_visit

PRIVATE WUNUSED NONNULL((1)) int DCALL
di_nl_init(DefaultIterator_WithNextAndLimit *__restrict self,
           size_t argc, DeeObject *const *argv) {
	DeeTypeObject *itertyp;
	if (DeeArg_Unpack(argc, argv, "o" UNPuSIZ ":_IterWithNextAndLimit",
	                  &self->dinl_iter, &self->dinl_limit))
		goto err;
	itertyp = Dee_TYPE(self->dinl_iter);
	self->dinl_tp_next = DeeType_RequireSupportedNativeOperator(itertyp, iter_next);
	if unlikely(!self->dinl_tp_next)
		goto err_no_next;
	Dee_Incref(self->dinl_iter);
	return 0;
err_no_next:
	err_unimplemented_operator(itertyp, OPERATOR_ITERNEXT);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
di_nl_copy(DefaultIterator_WithNextAndLimit *__restrict self,
           DefaultIterator_WithNextAndLimit *__restrict other) {
	self->dinl_iter = DeeObject_Copy(other->dinl_iter);
	if unlikely(!self->dinl_iter)
		goto err;
	self->dinl_tp_next = other->dinl_tp_next;
	self->dinl_limit   = atomic_read(&other->dinl_limit);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
di_nl_deepcopy(DefaultIterator_WithNextAndLimit *__restrict self,
               DefaultIterator_WithNextAndLimit *__restrict other) {
	self->dinl_iter = DeeObject_DeepCopy(other->dinl_iter);
	if unlikely(!self->dinl_iter)
		goto err;
	self->dinl_tp_next = other->dinl_tp_next;
	self->dinl_limit   = atomic_read(&other->dinl_limit);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
di_nl_serialize(DefaultIterator_WithNextAndLimit *__restrict self,
                DeeSerial *__restrict writer, Dee_seraddr_t addr) {
#define ADDROF(field) (addr + offsetof(DefaultIterator_WithNextAndLimit, field))
	DefaultIterator_WithNextAndLimit *out;
	out = DeeSerial_Addr2Mem(writer, addr, DefaultIterator_WithNextAndLimit);
	out->dinl_limit = atomic_read(&self->dinl_limit);
	if unlikely(generic_proxy__serialize((ProxyObject *)self, writer, addr))
		goto err;
	return DeeSerial_PutFuncPtr(writer, ADDROF(dinl_tp_next), self->dinl_tp_next);
err:
	return -1;
#undef ADDROF
}

PRIVATE WUNUSED NONNULL((1)) Dee_hash_t DCALL
di_nl_hash(DefaultIterator_WithNextAndLimit *self) {
	Dee_hash_t result;
	result = DeeObject_Hash(self->dinl_iter);
	result = Dee_HashCombine(result, atomic_read(&self->dinl_limit));
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
di_nl_compare(DefaultIterator_WithNextAndLimit *self,
              DefaultIterator_WithNextAndLimit *other) {
	int result;
	if (DeeObject_AssertTypeExact(other, &DefaultIterator_WithNextAndLimit_Type))
		goto err;
	result = DeeObject_Compare(self->dinl_iter, other->dinl_iter);
	if (result == 0) {
		size_t my_limit = atomic_read(&self->dinl_limit);
		size_t ot_limit = atomic_read(&other->dinl_limit);
		result = Dee_Compare(my_limit, ot_limit);
	}
	return result;
err:
	return Dee_COMPARE_ERR;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
di_nl_iter_next(DefaultIterator_WithNextAndLimit *self) {
	DREF DeeObject *result;
	if unlikely(atomic_read(&self->dinl_limit) == 0)
		return ITER_DONE;
	result = (*self->dinl_tp_next)(self->dinl_iter);
	if (ITER_ISOK(result)) {
		size_t limit;
		do {
			limit = atomic_read(&self->dinl_limit);
			if unlikely(limit == 0)
				break;
		} while (!atomic_cmpxch_or_write(&self->dinl_limit,
		                                 limit, limit - 1));
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
di_nl_bool(DefaultIterator_WithNextAndLimit *self) {
	if (atomic_read(&self->dinl_limit) == 0)
		return 0;
	return DeeObject_Bool(self->dinl_iter);
}

PRIVATE struct type_cmp di_nl_cmp = {
	/* .tp_hash          = */ (Dee_hash_t (DCALL *)(DeeObject *))&di_nl_hash,
	/* .tp_compare_eq    = */ (int (DCALL *)(DeeObject *, DeeObject *))&di_nl_compare,
	/* .tp_compare       = */ (int (DCALL *)(DeeObject *, DeeObject *))&di_nl_compare,
	/* .tp_trycompare_eq = */ DEFIMPL(&default__trycompare_eq__with__compare_eq),
	/* .tp_eq            = */ DEFIMPL(&default__eq__with__compare_eq),
	/* .tp_ne            = */ DEFIMPL(&default__ne__with__compare_eq),
	/* .tp_lo            = */ DEFIMPL(&default__lo__with__compare),
	/* .tp_le            = */ DEFIMPL(&default__le__with__compare),
	/* .tp_gr            = */ DEFIMPL(&default__gr__with__compare),
	/* .tp_ge            = */ DEFIMPL(&default__ge__with__compare),
};

STATIC_ASSERT(offsetof(DefaultIterator_WithNextAndLimit, dinl_iter) ==
              offsetof(DefaultIterator_PairSubItem, dipsi_iter));
PRIVATE struct type_member tpconst di_nl_members[] = {
	TYPE_MEMBER_FIELD("__limit__", STRUCT_ATOMIC | STRUCT_SIZE_T, offsetof(DefaultIterator_WithNextAndLimit, dinl_limit)),
#define di_nk_members (di_nl_members + 1)
#define di_nv_members di_nk_members
	TYPE_MEMBER_FIELD("__iter__", STRUCT_OBJECT, offsetof(DefaultIterator_WithNextAndLimit, dinl_iter)),
	TYPE_MEMBER_END,
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
di_nl_getseq(DefaultIterator_WithNextAndLimit *__restrict self) {
	return DeeObject_GetAttr(self->dinl_iter, Dee_AsObject(&str_seq));
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
di_nl_boundseq(DefaultIterator_WithNextAndLimit *__restrict self) {
	return DeeObject_BoundAttr(self->dinl_iter, Dee_AsObject(&str_seq));
}

PRIVATE struct type_getset tpconst di_nl_getsets[] = {
	TYPE_GETTER_BOUND(STR_seq, &di_nl_getseq, &di_nl_boundseq, "Alias for ${this.__iter__.seq}"),
	TYPE_GETSET_END
};

INTERN DeeTypeObject DefaultIterator_WithNextAndLimit_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_IterWithNextAndLimit",
	/* .tp_doc      = */ DOC("(objWithNext,limit:?Dint)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ DefaultIterator_WithNextAndLimit,
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ &di_nl_copy,
			/* tp_deep_ctor:   */ &di_nl_deepcopy,
			/* tp_any_ctor:    */ &di_nl_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &di_nl_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&di_nl_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&di_nl_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&iterator_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&di_nl_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__EFED4BCD35433C3C),
	/* .tp_cmp           = */ &di_nl_cmp,
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&di_nl_iter_next,
	/* .tp_iterator      = */ DEFIMPL(&default__tp_iterator__863AC70046E4B6B0),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ di_nl_getsets,
	/* .tp_members       = */ di_nl_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL(&iterator_next),
	/* .tp_callable      = */ DEFIMPL(&default__tp_callable__83C59FA7626CABBE),
};
























/************************************************************************/
/* DefaultIterator_WithIterKeysAndTryGetItemSeq_Type                    */
/* DefaultIterator_WithIterKeysAndTryGetItemMap_Type                    */
/************************************************************************/

PRIVATE WUNUSED NONNULL((1)) int DCALL
di_ikgim_init(DefaultIterator_WithIterKeysAndGetItem *__restrict self,
              size_t argc, DeeObject *const *argv) {
	DeeTypeObject *itertyp, *seqtyp;
	DeeArg_Unpack2(err, argc, argv, "_IterWithIterKeysAndGetItemForMap",
	                &self->diikgi_seq, &self->diikgi_iter);
	itertyp = Dee_TYPE(self->diikgi_iter);
	self->diikgi_tp_next = DeeType_RequireSupportedNativeOperator(itertyp, iter_next);
	if unlikely(!self->diikgi_tp_next)
		goto err_no_next;
	seqtyp = Dee_TYPE(self->diikgi_seq);
	self->diikgi_tp_getitem = DeeType_RequireSupportedNativeOperator(seqtyp, getitem);
	if unlikely(!self->diikgi_tp_getitem)
		goto err_no_getitem;
	Dee_Incref(self->diikgi_iter);
	Dee_Incref(self->diikgi_seq);
	return 0;
err_no_getitem:
	return err_unimplemented_operator(seqtyp, OPERATOR_GETITEM);
err_no_next:
	return err_unimplemented_operator(itertyp, OPERATOR_ITERNEXT);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
di_iktrgim_init(DefaultIterator_WithIterKeysAndGetItem *__restrict self,
                size_t argc, DeeObject *const *argv) {
	DeeTypeObject *itertyp, *seqtyp;
	DeeArg_Unpack2(err, argc, argv, "_IterWithIterKeysAndTryGetItemForMap",
	                &self->diikgi_seq, &self->diikgi_iter);
	itertyp = Dee_TYPE(self->diikgi_iter);
	self->diikgi_tp_next = DeeType_RequireSupportedNativeOperator(itertyp, iter_next);
	if unlikely(!self->diikgi_tp_next)
		goto err_no_next;
	seqtyp = Dee_TYPE(self->diikgi_seq);
	self->diikgi_tp_getitem = DeeType_RequireSupportedNativeOperator(seqtyp, trygetitem);
	if unlikely(!self->diikgi_tp_getitem)
		goto err_no_getitem;
	Dee_Incref(self->diikgi_iter);
	Dee_Incref(self->diikgi_seq);
	return 0;
err_no_getitem:
	return err_unimplemented_operator(seqtyp, OPERATOR_GETITEM);
err_no_next:
	return err_unimplemented_operator(itertyp, OPERATOR_ITERNEXT);
err:
	return -1;
}

#define di_iktrgim_copy di_ikgim_copy
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
di_ikgim_copy(DefaultIterator_WithIterKeysAndGetItem *__restrict self,
              DefaultIterator_WithIterKeysAndGetItem *__restrict other) {
	self->diikgi_iter = DeeObject_Copy(other->diikgi_iter);
	if unlikely(!self->diikgi_iter)
		goto err;
	self->diikgi_seq = other->diikgi_seq;
	Dee_Incref(self->diikgi_seq);
	self->diikgi_tp_next    = other->diikgi_tp_next;
	self->diikgi_tp_getitem = other->diikgi_tp_getitem;
	return 0;
err:
	return -1;
}

#define di_iktrgim_deepcopy di_ikgim_deepcopy
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
di_ikgim_deepcopy(DefaultIterator_WithIterKeysAndGetItem *__restrict self,
                  DefaultIterator_WithIterKeysAndGetItem *__restrict other) {
	self->diikgi_iter = DeeObject_DeepCopy(other->diikgi_iter);
	if unlikely(!self->diikgi_iter)
		goto err;
	self->diikgi_seq = DeeObject_DeepCopy(other->diikgi_seq);
	if unlikely(!self->diikgi_seq)
		goto err_iter;
	self->diikgi_tp_next    = other->diikgi_tp_next;
	self->diikgi_tp_getitem = other->diikgi_tp_getitem;
	return 0;
err_iter:
	Dee_Decref(self->diikgi_iter);
err:
	return -1;
}


#define di_iktrgim_serialize di_ikgim_serialize
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
di_ikgim_serialize(DefaultIterator_WithIterKeysAndGetItem *__restrict self,
                   DeeSerial *__restrict writer, Dee_seraddr_t addr) {
#define ADDROF(field) (addr + offsetof(DefaultIterator_WithIterKeysAndGetItem, field))
	int result = generic_proxy2__serialize((ProxyObject2 *)self, writer, addr);
	if likely(result == 0)
		result = DeeSerial_PutFuncPtr(writer, ADDROF(diikgi_tp_next), self->diikgi_tp_next);
	if likely(result == 0)
		result = DeeSerial_PutFuncPtr(writer, ADDROF(diikgi_tp_getitem), self->diikgi_tp_getitem);
	return result;
#undef ADDROF
}

STATIC_ASSERT(offsetof(DefaultIterator_WithIterKeysAndGetItem, diikgi_seq) == offsetof(ProxyObject2, po_obj1) ||
              offsetof(DefaultIterator_WithIterKeysAndGetItem, diikgi_seq) == offsetof(ProxyObject2, po_obj2));
STATIC_ASSERT(offsetof(DefaultIterator_WithIterKeysAndGetItem, diikgi_iter) == offsetof(ProxyObject2, po_obj1) ||
              offsetof(DefaultIterator_WithIterKeysAndGetItem, diikgi_iter) == offsetof(ProxyObject2, po_obj2));
#define di_ikgim_fini   generic_proxy2__fini
#define di_iktrgim_fini di_ikgim_fini

STATIC_ASSERT(offsetof(DefaultIterator_WithIterKeysAndGetItem, diikgi_seq) == offsetof(ProxyObject2, po_obj1) ||
              offsetof(DefaultIterator_WithIterKeysAndGetItem, diikgi_seq) == offsetof(ProxyObject2, po_obj2));
STATIC_ASSERT(offsetof(DefaultIterator_WithIterKeysAndGetItem, diikgi_iter) == offsetof(ProxyObject2, po_obj1) ||
              offsetof(DefaultIterator_WithIterKeysAndGetItem, diikgi_iter) == offsetof(ProxyObject2, po_obj2));
#define di_ikgim_visit   generic_proxy2__visit
#define di_iktrgim_visit di_ikgim_visit

STATIC_ASSERT(offsetof(DefaultIterator_WithIterKeysAndGetItem, diikgi_iter) == offsetof(ProxyObject, po_obj));
#define di_ikgis_hash   generic_proxy__hash_recursive
#define di_iktrgim_hash di_ikgis_hash

STATIC_ASSERT(offsetof(DefaultIterator_WithIterKeysAndGetItem, diikgi_iter) == offsetof(ProxyObject, po_obj));
#define di_ikgim_compare_eq   generic_proxy__compare_eq_recursive
#define di_iktrgim_compare_eq di_ikgim_compare_eq

STATIC_ASSERT(offsetof(DefaultIterator_WithIterKeysAndGetItem, diikgi_iter) == offsetof(ProxyObject, po_obj));
#define di_ikgim_compare   generic_proxy__compare_recursive
#define di_iktrgim_compare di_ikgim_compare

STATIC_ASSERT(offsetof(DefaultIterator_WithIterKeysAndGetItem, diikgi_iter) == offsetof(ProxyObject, po_obj));
#define di_ikgim_trycompare_eq   generic_proxy__trycompare_eq_recursive
#define di_iktrgim_trycompare_eq di_ikgim_trycompare_eq

#define di_iktrgim_cmp di_ikgim_cmp
#define di_ikgim_cmp   generic_proxy__cmp_recursive

PRIVATE struct type_member tpconst di_ikgim_members[] = {
#define di_iktrgim_members di_ikgim_members
	TYPE_MEMBER_FIELD_DOC(STR_seq, STRUCT_OBJECT, offsetof(DefaultIterator_WithIterKeysAndGetItem, diikgi_seq), "->?DSequence"),
	TYPE_MEMBER_FIELD_DOC("__iterkeys__", STRUCT_OBJECT, offsetof(DefaultIterator_WithIterKeysAndGetItem, diikgi_iter), "->?DIterator"),
	TYPE_MEMBER_END
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeTupleObject *DCALL
di_ikgim_iter_next(DefaultIterator_WithIterKeysAndGetItem *__restrict self) {
	DREF DeeTupleObject *result;
	DREF DeeObject *value;
	DREF DeeObject *key;
nextkey:
	key = (*self->diikgi_tp_next)(self->diikgi_iter);
	if unlikely(!ITER_ISOK(key))
		return (DREF DeeTupleObject *)key;
	value = (*self->diikgi_tp_getitem)(self->diikgi_seq, key);
	if unlikely(!value) {
		Dee_Decref(key);
		if (DeeError_Catch(&DeeError_UnboundItem))
			goto nextkey;
		if (DeeError_Catch(&DeeError_IndexError))
			goto nextkey;
		if (DeeError_Catch(&DeeError_KeyError))
			goto nextkey;
		goto err;
	}
	result = DeeTuple_NewUninitializedPair();
	if unlikely(!result)
		goto err_key_value;
	DeeTuple_SET(result, 0, key);
	DeeTuple_SET(result, 1, value);
	return result;
err_key_value:
	Dee_Decref(value);
/*err_key:*/
	Dee_Decref(key);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeTupleObject *DCALL
di_iktrgim_iter_next(DefaultIterator_WithIterKeysAndGetItem *__restrict self) {
	DREF DeeTupleObject *result;
	DREF DeeObject *value;
	DREF DeeObject *key;
nextkey:
	key = (*self->diikgi_tp_next)(self->diikgi_iter);
	if unlikely(!ITER_ISOK(key))
		return (DREF DeeTupleObject *)key;
	value = (*self->diikgi_tp_getitem)(self->diikgi_seq, key);
	if (!ITER_ISOK(value)) {
		if unlikely(!value)
			goto err_key;
		goto nextkey;
	}
	result = DeeTuple_NewUninitializedPair();
	if unlikely(!result)
		goto err_key_value;
	DeeTuple_SET(result, 0, key);
	DeeTuple_SET(result, 1, value);
	return result;
err_key_value:
	Dee_Decref(value);
err_key:
	Dee_Decref(key);
/*err:*/
	return NULL;
}

INTERN DeeTypeObject DefaultIterator_WithIterKeysAndGetItemMap_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_IterWithIterKeysAndGetItemForMap",
	/* .tp_doc      = */ DOC("(objWithGetItem,objWithNext)\n"
	                         "next->?T2?O?O"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ DefaultIterator_WithIterKeysAndGetItem,
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ &di_ikgim_copy,
			/* tp_deep_ctor:   */ &di_ikgim_deepcopy,
			/* tp_any_ctor:    */ &di_ikgim_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &di_ikgim_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&di_ikgim_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ DEFIMPL(&iterator_bool),
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&iterator_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&di_ikgim_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__EFED4BCD35433C3C),
	/* .tp_cmp           = */ &di_ikgim_cmp,
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&di_ikgim_iter_next,
	/* .tp_iterator      = */ DEFIMPL(&default__tp_iterator__863AC70046E4B6B0),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ di_ikgim_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL(&iterator_next),
	/* .tp_callable      = */ DEFIMPL(&default__tp_callable__83C59FA7626CABBE),
};

INTERN DeeTypeObject DefaultIterator_WithIterKeysAndTryGetItemMap_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_IterWithIterKeysAndTryGetItemForMap",
	/* .tp_doc      = */ DOC("(objWithTryGetItem,objWithNext)\n"
	                         "next->?T2?O?O"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ DefaultIterator_WithIterKeysAndGetItem,
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ &di_iktrgim_copy,
			/* tp_deep_ctor:   */ &di_iktrgim_deepcopy,
			/* tp_any_ctor:    */ &di_iktrgim_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &di_iktrgim_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&di_iktrgim_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ DEFIMPL(&iterator_bool),
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&iterator_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&di_iktrgim_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__EFED4BCD35433C3C),
	/* .tp_cmp           = */ &di_iktrgim_cmp,
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&di_iktrgim_iter_next,
	/* .tp_iterator      = */ DEFIMPL(&default__tp_iterator__863AC70046E4B6B0),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ di_iktrgim_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL(&iterator_next),
	/* .tp_callable      = */ DEFIMPL(&default__tp_callable__83C59FA7626CABBE),
};























/************************************************************************/
/* DefaultIterator_WithForeach_Type                                     */
/* DefaultIterator_WithForeachPair_Type                                 */
/* DefaultIterator_WithEnumerateSeq_Type                                */
/* DefaultIterator_WithEnumerateMap_Type                                */
/* DefaultIterator_WithEnumerateIndexSeq_Type                           */
/************************************************************************/

INTERN DeeTypeObject DefaultIterator_WithForeach_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_IterWithForeach",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ DeeObject,
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ NULL, /* TODO */
			/* tp_deep_ctor:   */ NULL, /* TODO */
			/* tp_any_ctor:    */ NULL, /* TODO */
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ NULL /* TODO */
		),
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ DEFIMPL(&iterator_bool),
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&iterator_printrepr),
	},
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__EFED4BCD35433C3C),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__439AAF00B07ABA02), /* TODO */
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ DEFIMPL((DREF DeeObject *(DCALL *)(DeeObject *__restrict))&_DeeNone_retsm1_1), /* TODO */
	/* .tp_iterator      = */ DEFIMPL(&default__tp_iterator__863AC70046E4B6B0),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL(&iterator_next),
	/* .tp_callable      = */ DEFIMPL(&default__tp_callable__83C59FA7626CABBE),
};

INTERN DeeTypeObject DefaultIterator_WithForeachPair_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_IterWithForeachPair",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ DeeObject,
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ NULL, /* TODO */
			/* tp_deep_ctor:   */ NULL, /* TODO */
			/* tp_any_ctor:    */ NULL, /* TODO */
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ NULL /* TODO */
		),
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ DEFIMPL(&iterator_bool),
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&iterator_printrepr),
	},
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__EFED4BCD35433C3C),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__439AAF00B07ABA02), /* TODO */
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ DEFIMPL((DREF DeeObject *(DCALL *)(DeeObject *__restrict))&_DeeNone_retsm1_1), /* TODO */
	/* .tp_iterator      = */ DEFIMPL(&default__tp_iterator__863AC70046E4B6B0),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL(&iterator_next),
	/* .tp_callable      = */ DEFIMPL(&default__tp_callable__83C59FA7626CABBE),
};

INTERN DeeTypeObject DefaultIterator_WithEnumerateSeq_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_IterWithEnumerateSeq",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ DeeObject,
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ NULL, /* TODO */
			/* tp_deep_ctor:   */ NULL, /* TODO */
			/* tp_any_ctor:    */ NULL, /* TODO */
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ NULL /* TODO */
		),
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ DEFIMPL(&iterator_bool),
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&iterator_printrepr),
	},
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__EFED4BCD35433C3C),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__439AAF00B07ABA02), /* TODO */
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ DEFIMPL((DREF DeeObject *(DCALL *)(DeeObject *__restrict))&_DeeNone_retsm1_1), /* TODO */
	/* .tp_iterator      = */ DEFIMPL(&default__tp_iterator__863AC70046E4B6B0),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL(&iterator_next),
	/* .tp_callable      = */ DEFIMPL(&default__tp_callable__83C59FA7626CABBE),
};

INTERN DeeTypeObject DefaultIterator_WithEnumerateMap_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_IterWithEnumerateMap",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ DeeObject,
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ NULL, /* TODO */
			/* tp_deep_ctor:   */ NULL, /* TODO */
			/* tp_any_ctor:    */ NULL, /* TODO */
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ NULL /* TODO */
		),
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ DEFIMPL(&iterator_bool),
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&iterator_printrepr),
	},
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__EFED4BCD35433C3C),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__439AAF00B07ABA02), /* TODO */
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ DEFIMPL((DREF DeeObject *(DCALL *)(DeeObject *__restrict))&_DeeNone_retsm1_1), /* TODO */
	/* .tp_iterator      = */ DEFIMPL(&default__tp_iterator__863AC70046E4B6B0),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL(&iterator_next),
	/* .tp_callable      = */ DEFIMPL(&default__tp_callable__83C59FA7626CABBE),
};

INTERN DeeTypeObject DefaultIterator_WithEnumerateIndexSeq_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_IterWithEnumerateIndexSeq",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ DeeObject,
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ NULL, /* TODO */
			/* tp_deep_ctor:   */ NULL, /* TODO */
			/* tp_any_ctor:    */ NULL, /* TODO */
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ NULL /* TODO */
		),
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ DEFIMPL(&iterator_bool),
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&iterator_printrepr),
	},
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__EFED4BCD35433C3C),
	/* .tp_cmp           = */ DEFIMPL(&default__tp_cmp__439AAF00B07ABA02), /* TODO */
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ DEFIMPL((DREF DeeObject *(DCALL *)(DeeObject *__restrict))&_DeeNone_retsm1_1), /* TODO */
	/* .tp_iterator      = */ DEFIMPL(&default__tp_iterator__863AC70046E4B6B0),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL(&iterator_next),
	/* .tp_callable      = */ DEFIMPL(&default__tp_callable__83C59FA7626CABBE),
};




/************************************************************************/
/* Extra iterators for default enumeration sequence types               */
/************************************************************************/

STATIC_ASSERT(offsetof(DefaultIterator_WithNextAndLimit, dinl_iter) == offsetof(DefaultIterator_WithNextAndCounter, dinc_iter));
STATIC_ASSERT(offsetof(DefaultIterator_WithNextAndLimit, dinl_tp_next) == offsetof(DefaultIterator_WithNextAndCounter, dinc_tp_next));
STATIC_ASSERT(offsetof(DefaultIterator_WithNextAndLimit, dinl_limit) == offsetof(DefaultIterator_WithNextAndCounter, dinc_counter));
STATIC_ASSERT(offsetof(DefaultIterator_WithNextAndCounterAndLimit, dincl_iter) == offsetof(DefaultIterator_WithNextAndCounter, dinc_iter));
STATIC_ASSERT(offsetof(DefaultIterator_WithNextAndCounterAndLimit, dincl_tp_next) == offsetof(DefaultIterator_WithNextAndCounter, dinc_tp_next));
STATIC_ASSERT(offsetof(DefaultIterator_WithNextAndCounterAndLimit, dincl_counter) == offsetof(DefaultIterator_WithNextAndCounter, dinc_counter));

#define di_ncp_copy      di_nl_copy
#define di_ncp_deepcopy  di_nl_deepcopy
#define di_ncp_serialize di_nl_serialize
#define di_ncp_fini      di_nl_fini
#define di_ncp_visit     di_nl_visit
#define di_ncp_hash      di_nl_hash
#define di_ncpl_fini     di_ncp_fini
#define di_ncpl_visit    di_ncp_visit
#define di_ncpl_hash     di_ncp_hash

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
di_ncpl_copy(DefaultIterator_WithNextAndCounterAndLimit *__restrict self,
             DefaultIterator_WithNextAndCounterAndLimit *__restrict other) {
	self->dincl_limit = other->dincl_limit;
	return di_ncp_copy((DefaultIterator_WithNextAndLimit *)self,
	                   (DefaultIterator_WithNextAndLimit *)other);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
di_ncpl_deepcopy(DefaultIterator_WithNextAndCounterAndLimit *__restrict self,
                 DefaultIterator_WithNextAndCounterAndLimit *__restrict other) {
	self->dincl_limit = other->dincl_limit;
	return di_ncp_deepcopy((DefaultIterator_WithNextAndLimit *)self,
	                       (DefaultIterator_WithNextAndLimit *)other);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
di_ncpl_serialize(DefaultIterator_WithNextAndCounterAndLimit *__restrict self,
                  DeeSerial *__restrict writer, Dee_seraddr_t addr) {
	DefaultIterator_WithNextAndCounterAndLimit *out;
	out = DeeSerial_Addr2Mem(writer, addr, DefaultIterator_WithNextAndCounterAndLimit);
	out->dincl_limit = self->dincl_limit;
	return di_ncp_serialize((DefaultIterator_WithNextAndLimit *)self, writer, addr);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
di_ncp_init(DefaultIterator_WithNextAndCounter *__restrict self,
            size_t argc, DeeObject *const *argv) {
	DeeTypeObject *itertyp;
	if (DeeArg_Unpack(argc, argv, "o" UNPuSIZ ":_IterWithNextAndCounter",
	                  &self->dinc_iter, &self->dinc_counter))
		goto err;
	itertyp = Dee_TYPE(self->dinc_iter);
	self->dinc_tp_next = DeeType_RequireSupportedNativeOperator(itertyp, iter_next);
	if unlikely(!self->dinc_tp_next)
		goto err_no_next;
	Dee_Incref(self->dinc_iter);
	return 0;
err_no_next:
	err_unimplemented_operator(itertyp, OPERATOR_ITERNEXT);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
di_ncpl_init(DefaultIterator_WithNextAndCounterAndLimit *__restrict self,
             size_t argc, DeeObject *const *argv) {
	DeeTypeObject *itertyp;
	if (DeeArg_Unpack(argc, argv, "o" UNPuSIZ UNPuSIZ ":_IterWithNextAndCounterAndLimit",
	                  &self->dincl_iter, &self->dincl_counter, &self->dincl_limit))
		goto err;
	itertyp = Dee_TYPE(self->dincl_iter);
	self->dincl_tp_next = DeeType_RequireSupportedNativeOperator(itertyp, iter_next);
	if unlikely(!self->dincl_tp_next)
		goto err_no_next;
	Dee_Incref(self->dincl_iter);
	return 0;
err_no_next:
	err_unimplemented_operator(itertyp, OPERATOR_ITERNEXT);
err:
	return -1;
}

#define di_ncpl_compare di_ncp_compare
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
di_ncp_compare(DefaultIterator_WithNextAndCounter *self,
               DefaultIterator_WithNextAndCounter *other) {
	int result;
	size_t my_counter, ot_counter;
	if (DeeObject_AssertTypeExact(other, &DefaultIterator_WithNextAndCounterPair_Type))
		goto err;
	my_counter = atomic_read(&self->dinc_counter);
	ot_counter = atomic_read(&other->dinc_counter);
	result = Dee_Compare(my_counter, ot_counter);
	if (result == 0)
		result = DeeObject_Compare(self->dinc_iter, other->dinc_iter);
	return result;
err:
	return Dee_COMPARE_ERR;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
di_ncp_iter_next(DefaultIterator_WithNextAndCounter *self) {
	DREF DeeObject *value;
	value = (*self->dinc_tp_next)(self->dinc_iter);
	if (ITER_ISOK(value)) {
		size_t key = atomic_fetchinc(&self->dinc_counter);
		value = DeeTuple_Newf(PCKuSIZ "O", key, value);
	}
	return value;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
di_ncpl_iter_next(DefaultIterator_WithNextAndCounterAndLimit *self) {
	DREF DeeObject *value;
	if unlikely(atomic_read(&self->dincl_limit) == 0)
		return ITER_DONE;
	value = (*self->dincl_tp_next)(self->dincl_iter);
	if (ITER_ISOK(value)) {
		size_t key, limit;
		do {
			limit = atomic_read(&self->dincl_limit);
			if unlikely(limit == 0)
				break;
		} while (!atomic_cmpxch_or_write(&self->dincl_limit,
		                                 limit, limit - 1));
		key = atomic_fetchinc(&self->dincl_counter);
		value = DeeTuple_Newf(PCKuSIZ "O", key, value);
	}
	return value;
}

STATIC_ASSERT(offsetof(DefaultIterator_WithNextAndCounter, dinc_iter) ==
              offsetof(ProxyObject, po_obj));
#define di_ncp_bool generic_proxy__bool

PRIVATE WUNUSED NONNULL((1)) int DCALL
di_ncpl_bool(DefaultIterator_WithNextAndCounterAndLimit *self) {
	if (atomic_read(&self->dincl_limit) == 0)
		return 0;
	return DeeObject_Bool(self->dincl_iter);
}

#define di_ncpl_cmp di_ncp_cmp
PRIVATE struct type_cmp di_ncp_cmp = {
	/* .tp_hash          = */ (Dee_hash_t (DCALL *)(DeeObject *))&di_ncp_hash,
	/* .tp_compare_eq    = */ (int (DCALL *)(DeeObject *, DeeObject *))&di_ncp_compare,
	/* .tp_compare       = */ (int (DCALL *)(DeeObject *, DeeObject *))&di_ncp_compare,
	/* .tp_trycompare_eq = */ DEFIMPL(&default__trycompare_eq__with__compare_eq),
	/* .tp_eq            = */ DEFIMPL(&default__eq__with__compare_eq),
	/* .tp_ne            = */ DEFIMPL(&default__ne__with__compare_eq),
	/* .tp_lo            = */ DEFIMPL(&default__lo__with__compare),
	/* .tp_le            = */ DEFIMPL(&default__le__with__compare),
	/* .tp_gr            = */ DEFIMPL(&default__gr__with__compare),
	/* .tp_ge            = */ DEFIMPL(&default__ge__with__compare),
};


PRIVATE struct type_member tpconst di_ncpl_members[] = {
	TYPE_MEMBER_FIELD("__limit__", STRUCT_SIZE_T | STRUCT_CONST, offsetof(DefaultIterator_WithNextAndCounterAndLimit, dincl_limit)),
#define di_ncp_members (di_ncpl_members + 1)
	TYPE_MEMBER_FIELD("__iter__", STRUCT_OBJECT, offsetof(DefaultIterator_WithNextAndCounter, dinc_iter)),
	TYPE_MEMBER_FIELD("__counter__", STRUCT_ATOMIC | STRUCT_SIZE_T, offsetof(DefaultIterator_WithNextAndCounter, dinc_counter)),
	TYPE_MEMBER_END,
};

INTERN DeeTypeObject DefaultIterator_WithNextAndCounterPair_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_IterWithNextAndCounterPair",
	/* .tp_doc      = */ DOC("(objWithNext,counter:?Dint)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ DefaultIterator_WithNextAndCounter,
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ &di_ncp_copy,
			/* tp_deep_ctor:   */ &di_ncp_deepcopy,
			/* tp_any_ctor:    */ &di_ncp_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &di_ncp_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&di_ncp_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&di_ncp_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&iterator_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&di_ncp_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__EFED4BCD35433C3C),
	/* .tp_cmp           = */ &di_ncp_cmp,
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&di_ncp_iter_next,
	/* .tp_iterator      = */ DEFIMPL(&default__tp_iterator__863AC70046E4B6B0),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ di_ncp_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL(&iterator_next),
	/* .tp_callable      = */ DEFIMPL(&default__tp_callable__83C59FA7626CABBE),
};

INTERN DeeTypeObject DefaultIterator_WithNextAndCounterAndLimitPair_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_IterWithNextAndCounterAndLimitPair",
	/* .tp_doc      = */ DOC("(objWithNext,counter:?Dint,limit:?Dint)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ DefaultIterator_WithNextAndCounterAndLimit,
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ &di_ncpl_copy,
			/* tp_deep_ctor:   */ &di_ncpl_deepcopy,
			/* tp_any_ctor:    */ &di_ncpl_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &di_ncpl_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&di_ncpl_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&di_ncpl_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&iterator_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&di_ncpl_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__EFED4BCD35433C3C),
	/* .tp_cmp           = */ &di_ncpl_cmp,
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&di_ncpl_iter_next,
	/* .tp_iterator      = */ DEFIMPL(&default__tp_iterator__863AC70046E4B6B0),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ di_ncpl_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL(&iterator_next),
	/* .tp_callable      = */ DEFIMPL(&default__tp_callable__83C59FA7626CABBE),
};


STATIC_ASSERT(offsetof(DefaultIterator_WithNextAndUnpackFilter, dinuf_iter) ==
              offsetof(DefaultIterator_WithNextAndCounter, dinc_iter));
#define di_nuf_bool di_ncp_bool

STATIC_ASSERT(offsetof(DefaultIterator_WithNextAndUnpackFilter, dinuf_iter) ==
              offsetof(ProxyObject, po_obj));
#define di_nuf_cmp generic_proxy__cmp_recursive

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
di_nuf_copy(DefaultIterator_WithNextAndUnpackFilter *__restrict self,
            DefaultIterator_WithNextAndUnpackFilter *__restrict other) {
	self->dinuf_iter = DeeObject_Copy(other->dinuf_iter);
	if unlikely(!self->dinuf_iter)
		goto err;
	self->dinuf_tp_next = other->dinuf_tp_next;
	Dee_Incref(other->dinuf_start);
	self->dinuf_start = other->dinuf_start;
	Dee_Incref(other->dinuf_end);
	self->dinuf_end = other->dinuf_end;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
di_nuf_deepcopy(DefaultIterator_WithNextAndUnpackFilter *__restrict self,
                DefaultIterator_WithNextAndUnpackFilter *__restrict other) {
	self->dinuf_iter = DeeObject_DeepCopy(other->dinuf_iter);
	if unlikely(!self->dinuf_iter)
		goto err;
	self->dinuf_tp_next = other->dinuf_tp_next;
	self->dinuf_start = DeeObject_DeepCopy(other->dinuf_start);
	if unlikely(!self->dinuf_start)
		goto err_iter;
	self->dinuf_end = DeeObject_DeepCopy(other->dinuf_end);
	if unlikely(!self->dinuf_end)
		goto err_iter_start;
	return 0;
err_iter_start:
	Dee_Decref(self->dinuf_start);
err_iter:
	Dee_Decref(self->dinuf_iter);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
di_nuf_serialize(DefaultIterator_WithNextAndUnpackFilter *__restrict self,
                 DeeSerial *__restrict writer, Dee_seraddr_t addr) {
#define ADDROF(field) (addr + offsetof(DefaultIterator_WithNextAndUnpackFilter, field))
	int result = DeeSerial_PutObject(writer, ADDROF(dinuf_iter), self->dinuf_iter);
	if likely(result == 0)
		result = DeeSerial_PutFuncPtr(writer, ADDROF(dinuf_tp_next), self->dinuf_tp_next);
	if likely(result == 0)
		result = DeeSerial_PutObject(writer, ADDROF(dinuf_start), self->dinuf_start);
	if likely(result == 0)
		result = DeeSerial_PutObject(writer, ADDROF(dinuf_end), self->dinuf_end);
	return result;
#undef ADDROF
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
di_nuf_init(DefaultIterator_WithNextAndUnpackFilter *__restrict self,
            size_t argc, DeeObject *const *argv) {
	DeeTypeObject *itertyp;
	DeeArg_Unpack3(err, argc, argv, "_IterWithNextAndUnpackFilter",
	                &self->dinuf_iter, &self->dinuf_start, &self->dinuf_end);
	itertyp = Dee_TYPE(self->dinuf_iter);
	self->dinuf_tp_next = DeeType_RequireSupportedNativeOperator(itertyp, iter_next);
	if unlikely(!self->dinuf_tp_next)
		goto err_no_next;
	Dee_Incref(self->dinuf_iter);
	Dee_Incref(self->dinuf_start);
	Dee_Incref(self->dinuf_end);
	return 0;
err_no_next:
	err_unimplemented_operator(itertyp, OPERATOR_ITERNEXT);
err:
	return -1;
}

PRIVATE NONNULL((1)) void DCALL
di_nuf_fini(DefaultIterator_WithNextAndUnpackFilter *__restrict self) {
	Dee_Decref(self->dinuf_iter);
	Dee_Decref(self->dinuf_start);
	Dee_Decref(self->dinuf_end);
}

PRIVATE NONNULL((1, 2)) void DCALL
di_nuf_visit(DefaultIterator_WithNextAndUnpackFilter *__restrict self, Dee_visit_t proc, void *arg) {
	Dee_Visit(self->dinuf_iter);
	Dee_Visit(self->dinuf_start);
	Dee_Visit(self->dinuf_end);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
di_nuf_iter_next(DefaultIterator_WithNextAndUnpackFilter *self) {
	int temp;
	DREF DeeObject *result;
	DREF DeeObject *key_and_value[2];
again:
	result = (*self->dinuf_tp_next)(self->dinuf_iter);
	if (ITER_ISOK(result)) {
		if (DeeSeq_Unpack(result, 2, key_and_value))
			goto err_r;
		Dee_Decref(key_and_value[1]);
		temp = DeeObject_CmpLeAsBool(self->dinuf_start, key_and_value[0]);
		if unlikely(temp <= 0)
			goto temp_err_or_again_r_key_and_value_0;
		temp = DeeObject_CmpGrAsBool(self->dinuf_end, key_and_value[0]);
		if unlikely(temp <= 0)
			goto temp_err_or_again_r_key_and_value_0;
	}
	return result;
temp_err_or_again_r_key_and_value_0:
	if unlikely(temp < 0)
		goto err_r_key_and_value_0;
/*again_r_key_and_value_0:*/
	Dee_Decref(key_and_value[0]);
	Dee_Decref(result);
	goto again;
err_r_key_and_value_0:
	Dee_Decref(key_and_value[0]);
err_r:
	Dee_Decref(result);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
di_nuf_nextpair(DefaultIterator_WithNextAndUnpackFilter *self,
                /*out*/ DREF DeeObject *key_and_value[2]) {
	int result, temp;
again:
	{
		DeeTypeObject *itertyp    = Dee_TYPE(self->dinuf_iter);
		DeeNO_nextpair_t nextpair = DeeType_RequireSupportedNativeOperator(itertyp, nextpair);
		ASSERTF(nextpair, "But we know that regular iter_next is "
		                  "supported, and nextpair has an impl for it...");
		result = (*nextpair)(self->dinuf_iter, key_and_value);
	}
	if (result == 0) {
		temp = DeeObject_CmpLeAsBool(self->dinuf_start, key_and_value[0]);
		if unlikely(temp <= 0)
			goto temp_err_or_again_key_and_value;
		temp = DeeObject_CmpGrAsBool(self->dinuf_end, key_and_value[0]);
		if unlikely(temp <= 0)
			goto temp_err_or_again_key_and_value;
	}
	return result;
temp_err_or_again_key_and_value:
	if unlikely(temp < 0)
		goto err_key_and_value;
/*again_key_and_value:*/
	Dee_Decref(key_and_value[1]);
	Dee_Decref(key_and_value[0]);
	goto again;
err_key_and_value:
	Dee_Decref(key_and_value[1]);
	Dee_Decref(key_and_value[0]);
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
di_nuf_nextkey(DefaultIterator_WithNextAndUnpackFilter *self) {
	int temp;
	DREF DeeObject *result;
again:
	{
		DeeTypeObject *itertyp    = Dee_TYPE(self->dinuf_iter);
		DeeNO_nextkey_t nextkey = DeeType_RequireSupportedNativeOperator(itertyp, nextkey);
		ASSERTF(nextkey, "But we know that regular iter_next is "
		                  "supported, and nextkey has an impl for it...");
		result = (*nextkey)(self->dinuf_iter);
	}
	if (ITER_ISOK(result)) {
		temp = DeeObject_CmpLeAsBool(self->dinuf_start, result);
		if unlikely(temp <= 0)
			goto temp_err_or_again_r_key_and_value_0;
		temp = DeeObject_CmpGrAsBool(self->dinuf_end, result);
		if unlikely(temp <= 0)
			goto temp_err_or_again_r_key_and_value_0;
	}
	return result;
temp_err_or_again_r_key_and_value_0:
	if unlikely(temp < 0)
		goto err_r;
/*again_r_key_and_value_0:*/
	Dee_Decref(result);
	goto again;
err_r:
	Dee_Decref(result);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
di_nuf_nextvalue(DefaultIterator_WithNextAndUnpackFilter *self) {
	DREF DeeObject *key_and_value[2];
	int temp;
again:
	{
		DeeTypeObject *itertyp    = Dee_TYPE(self->dinuf_iter);
		DeeNO_nextpair_t nextpair = DeeType_RequireSupportedNativeOperator(itertyp, nextpair);
		ASSERTF(nextpair, "But we know that regular iter_next is "
		                  "supported, and nextpair has an impl for it...");
		temp = (*nextpair)(self->dinuf_iter, key_and_value);
	}
	if (temp > 0)
		return ITER_DONE;
	if unlikely(temp < 0)
		goto err;
	temp = DeeObject_CmpLeAsBool(self->dinuf_start, key_and_value[0]);
	if unlikely(temp <= 0)
		goto temp_err_or_again_key_and_value;
	temp = DeeObject_CmpGrAsBool(self->dinuf_end, key_and_value[0]);
	if unlikely(temp <= 0)
		goto temp_err_or_again_key_and_value;
	Dee_Decref(key_and_value[0]);
	return key_and_value[1];
temp_err_or_again_key_and_value:
	if unlikely(temp < 0)
		goto err_key_and_value0;
/*again_key_and_value:*/
	Dee_Decref(key_and_value[1]);
	Dee_Decref(key_and_value[0]);
	goto again;
err_key_and_value0:
	Dee_Decref(key_and_value[1]);
	Dee_Decref(key_and_value[0]);
err:
	return NULL;
}

PRIVATE struct type_iterator di_nuf_iterator = {
	/* .tp_nextpair  = */ (int (DCALL *)(DeeObject *__restrict, DREF DeeObject *[2]))&di_nuf_nextpair,
	/* .tp_nextkey   = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&di_nuf_nextkey,
	/* .tp_nextvalue = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&di_nuf_nextvalue,
	/* .tp_advance   = */ DEFIMPL(&default__advance__with__nextkey),
};

PRIVATE struct type_member tpconst di_nuf_members[] = {
	TYPE_MEMBER_FIELD("__iter__", STRUCT_OBJECT, offsetof(DefaultIterator_WithNextAndUnpackFilter, dinuf_iter)),
	TYPE_MEMBER_FIELD("__startkey__", STRUCT_OBJECT, offsetof(DefaultIterator_WithNextAndUnpackFilter, dinuf_start)),
	TYPE_MEMBER_FIELD("__endkey__", STRUCT_OBJECT, offsetof(DefaultIterator_WithNextAndUnpackFilter, dinuf_end)),
	TYPE_MEMBER_END,
};

INTERN DeeTypeObject DefaultIterator_WithNextAndUnpackFilter_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_IterWithNextAndUnpackFilter",
	/* .tp_doc      = */ DOC("(objWithNext,start,end)\n"
	                         "\n"
	                         "next->?T2?O?O"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ DefaultIterator_WithNextAndUnpackFilter,
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ &di_nuf_copy,
			/* tp_deep_ctor:   */ &di_nuf_deepcopy,
			/* tp_any_ctor:    */ &di_nuf_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &di_nuf_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&di_nuf_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&di_nuf_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&iterator_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&di_nuf_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__EFED4BCD35433C3C),
	/* .tp_cmp           = */ &di_nuf_cmp,
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&di_nuf_iter_next,
	/* .tp_iterator      = */ &di_nuf_iterator,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ di_nuf_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL(&iterator_next),
	/* .tp_callable      = */ DEFIMPL(&default__tp_callable__83C59FA7626CABBE),
};




STATIC_ASSERT(offsetof(DefaultIterator_PairSubItem, dipsi_iter) ==
              offsetof(DefaultIterator_WithNextAndCounter, dinc_iter));
#define di_nk_bool  di_ncp_bool
#define di_nv_bool  di_ncp_bool
#define di_nk_fini  di_ncp_fini
#define di_nv_fini  di_ncp_fini
#define di_nk_visit di_ncp_visit
#define di_nv_visit di_ncp_visit

#define di_nv_copy di_nk_copy
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
di_nk_copy(DefaultIterator_PairSubItem *__restrict self,
           DefaultIterator_PairSubItem *__restrict other) {
	self->dipsi_iter = DeeObject_Copy(other->dipsi_iter);
	if unlikely(!self->dipsi_iter)
		goto err;
	self->dipsi_next = other->dipsi_next;
	return 0;
err:
	return -1;
}

#define di_nv_deepcopy di_nk_deepcopy
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
di_nk_deepcopy(DefaultIterator_PairSubItem *__restrict self,
               DefaultIterator_PairSubItem *__restrict other) {
	self->dipsi_iter = DeeObject_DeepCopy(other->dipsi_iter);
	if unlikely(!self->dipsi_iter)
		goto err;
	self->dipsi_next = other->dipsi_next;
	return 0;
err:
	return -1;
}

STATIC_ASSERT(offsetof(DefaultIterator_PairSubItem, dipsi_iter) == offsetof(ProxyObjectWithPointer, po_obj));
STATIC_ASSERT(offsetof(DefaultIterator_PairSubItem, dipsi_next) == offsetof(ProxyObjectWithPointer, po_ptr));
#define di_nv_serialize generic_proxy_with_pointer__serialize_atomic
#define di_nk_serialize generic_proxy_with_pointer__serialize_atomic

PRIVATE WUNUSED NONNULL((1)) int DCALL
di_nk_init(DefaultIterator_PairSubItem *__restrict self,
           size_t argc, DeeObject *const *argv) {
	DeeTypeObject *itertyp;
	DeeArg_Unpack1(err, argc, argv, "_IterWithNextKey", &self->dipsi_iter);
	itertyp          = Dee_TYPE(self->dipsi_iter);
	self->dipsi_next = DeeType_RequireSupportedNativeOperator(itertyp, nextkey);
	if unlikely(!self->dipsi_next)
		goto err_no_next;
	Dee_Incref(self->dipsi_iter);
	return 0;
err_no_next:
	err_unimplemented_operator(itertyp, OPERATOR_ITERNEXT);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
di_nv_init(DefaultIterator_PairSubItem *__restrict self,
           size_t argc, DeeObject *const *argv) {
	DeeTypeObject *itertyp;
	DeeArg_Unpack1(err, argc, argv, "_IterWithNextValue", &self->dipsi_iter);
	itertyp = Dee_TYPE(self->dipsi_iter);
	self->dipsi_next = DeeType_RequireSupportedNativeOperator(itertyp, nextvalue);
	if unlikely(!self->dipsi_next)
		goto err_no_next;
	Dee_Incref(self->dipsi_iter);
	return 0;
err_no_next:
	err_unimplemented_operator(itertyp, OPERATOR_ITERNEXT);
err:
	return -1;
}

STATIC_ASSERT(offsetof(DefaultIterator_PairSubItem, dipsi_iter) == offsetof(ProxyObject, po_obj));
#define di_nv_advance generic_proxy__iter_advance
#define di_nk_advance generic_proxy__iter_advance
#define di_nv_cmp     generic_proxy__cmp_recursive
#define di_nk_cmp     generic_proxy__cmp_recursive

#define di_nv_iter_next di_nk_iter_next
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
di_nk_iter_next(DefaultIterator_PairSubItem *__restrict self) {
	return (*self->dipsi_next)(self->dipsi_iter);
}

#define di_nv_iterator di_nk_iterator
PRIVATE struct type_iterator di_nk_iterator = {
	/* .tp_nextpair  = */ DEFIMPL(&default__nextpair__with__iter_next),
	/* .tp_nextkey   = */ DEFIMPL(&default__nextkey__with__iter_next),
	/* .tp_nextvalue = */ DEFIMPL(&default__nextvalue__with__iter_next),
	/* .tp_advance   = */ (size_t (DCALL *)(DeeObject *__restrict, size_t))&di_nk_advance,
};

PRIVATE WUNUSED NONNULL((1)) DREF DefaultSequence_MapProxy *DCALL
di_nX_getseq(DefaultIterator_WithNextAndLimit *__restrict self,
             DeeTypeObject *__restrict map_proxy_type) {
	DREF DefaultSequence_MapProxy *result;
	DREF DeeObject *map = di_nl_getseq(self);
	if unlikely(!map)
		goto err;
	result = DeeObject_MALLOC(DefaultSequence_MapProxy);
	if unlikely(!result)
		goto err_map;
	result->dsmp_map = map;
	DeeObject_Init(result, map_proxy_type);
	return result;
err_map:
	Dee_Decref(map);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DefaultSequence_MapProxy *DCALL
di_nk_getseq(DefaultIterator_WithNextAndLimit *__restrict self) {
	return di_nX_getseq(self, &DefaultSequence_MapKeys_Type);
}

PRIVATE WUNUSED NONNULL((1)) DREF DefaultSequence_MapProxy *DCALL
di_nv_getseq(DefaultIterator_WithNextAndLimit *__restrict self) {
	return di_nX_getseq(self, &DefaultSequence_MapValues_Type);
}

#define di_nk_boundseq di_nl_boundseq
#define di_nv_boundseq di_nl_boundseq

PRIVATE struct type_getset tpconst di_nk_getsets[] = {
	TYPE_GETTER_BOUND(STR_seq, &di_nk_getseq, &di_nk_boundseq, "Alias for ${rt.MapKeys(this.__iter__.seq)}"),
	TYPE_GETSET_END
};

PRIVATE struct type_getset tpconst di_nv_getsets[] = {
	TYPE_GETTER_BOUND(STR_seq, &di_nv_getseq, &di_nv_boundseq, "Alias for ${rt.MapValues(this.__iter__.seq)}"),
	TYPE_GETSET_END
};


INTERN DeeTypeObject DefaultIterator_WithNextKey = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_IterWithNextKey",
	/* .tp_doc      = */ DOC("(objWithNext)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ DefaultIterator_PairSubItem,
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ &di_nk_copy,
			/* tp_deep_ctor:   */ &di_nk_deepcopy,
			/* tp_any_ctor:    */ &di_nk_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &di_nk_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&di_nk_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&di_nk_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&iterator_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&di_nk_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__EFED4BCD35433C3C),
	/* .tp_cmp           = */ &di_nk_cmp,
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&di_nk_iter_next,
	/* .tp_iterator      = */ &di_nk_iterator,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ di_nk_getsets,
	/* .tp_members       = */ di_nk_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL(&iterator_next),
	/* .tp_callable      = */ DEFIMPL(&default__tp_callable__83C59FA7626CABBE),
};

INTERN DeeTypeObject DefaultIterator_WithNextValue = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_IterWithNextValue",
	/* .tp_doc      = */ DOC("(objWithNext)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ DefaultIterator_PairSubItem,
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ &di_nv_copy,
			/* tp_deep_ctor:   */ &di_nv_deepcopy,
			/* tp_any_ctor:    */ &di_nv_init,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &di_nv_serialize
		),
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&di_nv_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ DEFIMPL(&object_str),
		/* .tp_repr = */ DEFIMPL(&default__repr__with__printrepr),
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&di_nv_bool,
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&iterator_printrepr),
	},
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&di_nv_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL(&default__tp_math__EFED4BCD35433C3C),
	/* .tp_cmp           = */ &di_nv_cmp,
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&di_nv_iter_next,
	/* .tp_iterator      = */ &di_nv_iterator,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ di_nv_getsets,
	/* .tp_members       = */ di_nv_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL(&iterator_next),
	/* .tp_callable      = */ DEFIMPL(&default__tp_callable__83C59FA7626CABBE),
};

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_DEFAULT_ITERATORS_C */
