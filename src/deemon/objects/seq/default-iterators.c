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
#ifndef GUARD_DEEMON_OBJECTS_SEQ_DEFAULT_ITERATORS_C
#define GUARD_DEEMON_OBJECTS_SEQ_DEFAULT_ITERATORS_C 1

#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/class.h>
#include <deemon/error.h>
#include <deemon/int.h>
#include <deemon/object.h>
#include <deemon/seq.h>
#include <deemon/tuple.h>
#include <deemon/util/atomic.h>
/**/

#include "default-iterators.h"

/**/
#include "../../runtime/runtime_error.h"

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

PRIVATE WUNUSED NONNULL((1)) int DCALL
di_gi_init(DefaultIterator_WithGetItemIndex *__restrict self,
           size_t argc, DeeObject *const *argv) {
	DeeTypeObject *seqtyp;
	if (DeeArg_Unpack(argc, argv, "o" UNPuSIZ ":_IterWithGetItemIndex",
	                  &self->digi_seq, &self->digi_index))
		goto err;
	seqtyp = Dee_TYPE(self->digi_seq);
	if ((!seqtyp->tp_seq || !seqtyp->tp_seq->tp_getitem_index) &&
	    !DeeType_InheritGetItem(seqtyp))
		goto err_no_getitem;
	self->digi_tp_getitem_index = seqtyp->tp_seq->tp_getitem_index;
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
	if (DeeArg_Unpack(argc, argv, "o" UNPuSIZ UNPuSIZ ":_IterWithSizeAndGetItemIndex",
	                  &self->disgi_seq, &self->disgi_index, &self->disgi_end))
		goto err;
	seqtyp = Dee_TYPE(self->disgi_seq);
	if ((!seqtyp->tp_seq || !seqtyp->tp_seq->tp_getitem_index) &&
	    !DeeType_InheritGetItem(seqtyp))
		goto err_no_getitem;
	self->disgi_tp_getitem_index = seqtyp->tp_seq->tp_getitem_index;
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
	if (DeeArg_Unpack(argc, argv, "o" UNPuSIZ UNPuSIZ ":_IterWithSizeAndGetItemIndexFast",
	                  &self->disgi_seq, &self->disgi_index, &self->disgi_end))
		goto err;
	seqtyp = Dee_TYPE(self->disgi_seq);
	if ((!seqtyp->tp_seq || !seqtyp->tp_seq->tp_getitem_index_fast) &&
	    (!DeeType_InheritGetItem(seqtyp) || !seqtyp->tp_seq->tp_getitem_index_fast))
		goto err_no_getitem;
	self->disgi_tp_getitem_index = seqtyp->tp_seq->tp_getitem_index_fast;
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
	if ((!seqtyp->tp_seq || !seqtyp->tp_seq->tp_trygetitem_index) &&
	    !DeeType_InheritGetItem(seqtyp))
		goto err_no_getitem;
	self->disgi_tp_getitem_index = seqtyp->tp_seq->tp_trygetitem_index;
	return 0;
err_no_getitem:
	err_unimplemented_operator(seqtyp, OPERATOR_GETITEM);
err:
	return -1;
}

#define di_sgi_fini  di_gi_fini
#define di_sgif_fini di_gi_fini
#define di_stgi_fini di_gi_fini
PRIVATE NONNULL((1)) void DCALL
di_gi_fini(DefaultIterator_WithGetItemIndex *__restrict self) {
	Dee_Decref(self->digi_seq);
}

#define di_sgi_visit  di_gi_visit
#define di_sgif_visit di_gi_visit
#define di_stgi_visit di_gi_visit
PRIVATE NONNULL((1, 2)) void DCALL
di_gi_visit(DefaultIterator_WithGetItemIndex *__restrict self,
            dvisit_t proc, void *arg) {
	Dee_Visit(self->digi_seq);
}

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
		if (DeeError_Catch(&DeeError_IndexError))
			return ITER_DONE;
		if (!DeeError_Catch(&DeeError_UnboundItem))
			goto err;
		++new_index;
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
		if (DeeError_Catch(&DeeError_IndexError))
			return ITER_DONE;
		if (!DeeError_Catch(&DeeError_UnboundItem))
			goto err;
		++new_index;
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
		if (DeeError_Catch(&DeeError_IndexError))
			return ITER_DONE;
		if (!DeeError_Catch(&DeeError_UnboundItem))
			goto err;
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
		if (DeeError_Catch(&DeeError_IndexError))
			return ITER_DONE;
		if (!DeeError_Catch(&DeeError_UnboundItem))
			goto err;
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
	return -1;
}

#define di_sgi_cmp  di_gi_cmp
#define di_sgif_cmp di_gi_cmp
#define di_stgi_cmp di_gi_cmp
PRIVATE struct type_cmp di_gi_cmp = {
	/* .tp_hash          = */ (Dee_hash_t (DCALL *)(DeeObject *))&di_gi_hash,
	/* .tp_compare_eq    = */ (int (DCALL *)(DeeObject *, DeeObject *))&di_gi_compare,
	/* .tp_compare       = */ (int (DCALL *)(DeeObject *, DeeObject *))&di_gi_compare,
	/* .tp_trycompare_eq = */ NULL,
};

#define di_stgi_members di_sgi_members
PRIVATE struct type_member tpconst di_sgi_members[] = {
	TYPE_MEMBER_FIELD("__end__", STRUCT_CONST | STRUCT_SIZE_T, offsetof(DefaultIterator_WithSizeAndGetItemIndex, disgi_end)),
#define di_gi_members (di_sgi_members + 1)
	TYPE_MEMBER_FIELD("__index__", STRUCT_ATOMIC | STRUCT_SIZE_T, offsetof(DefaultIterator_WithGetItemIndex, digi_index)),
	TYPE_MEMBER_FIELD("__seq__", STRUCT_OBJECT, offsetof(DefaultIterator_WithGetItemIndex, digi_seq)),
	TYPE_MEMBER_END,
};

PRIVATE struct type_member tpconst di_sgif_members[] = {
	TYPE_MEMBER_FIELD("__end__", STRUCT_CONST | STRUCT_SIZE_T, offsetof(DefaultIterator_WithSizeAndGetItemIndex, disgi_end)),
	TYPE_MEMBER_FIELD("__seq__", STRUCT_OBJECT, offsetof(DefaultIterator_WithGetItemIndex, digi_seq)),
	TYPE_MEMBER_END,
};

PRIVATE struct type_getset tpconst di_sgif_getsets[] = {
	TYPE_GETSET("__index__", &di_sgif_getindex, NULL, &di_sgif_setindex, "->?Dint"),
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
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)&di_gi_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&di_gi_deepcopy,
				/* .tp_any_ctor  = */ (dfunptr_t)&di_gi_init,
				TYPE_FIXED_ALLOCATOR(DefaultIterator_WithGetItemIndex)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&di_gi_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&di_gi_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &di_gi_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&di_gi_iter_next,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ di_gi_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
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
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)&di_gi_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&di_gi_deepcopy,
				/* .tp_any_ctor  = */ (dfunptr_t)&di_gi_init,
				TYPE_FIXED_ALLOCATOR(DefaultIterator_WithGetItemIndex)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&di_gi_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&di_gi_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &di_gi_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&di_gip_iter_next,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ di_gi_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};

INTERN DeeTypeObject DefaultIterator_WithSizeAndGetItemIndex_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_IterWithSizeAndGetItemIndex",
	/* .tp_doc      = */ DOC("(objWithGetItem,index:?Dint,end:?Dint)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)&di_sgi_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&di_sgi_deepcopy,
				/* .tp_any_ctor  = */ (dfunptr_t)&di_sgi_init,
				TYPE_FIXED_ALLOCATOR(DefaultIterator_WithSizeAndGetItemIndex)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&di_sgi_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&di_sgi_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &di_sgi_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&di_sgi_iter_next,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ di_sgi_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};

INTERN DeeTypeObject DefaultIterator_WithSizeAndGetItemIndexPair_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_IterWithSizeAndGetItemIndexPair",
	/* .tp_doc      = */ DOC("(objWithGetItem,index:?Dint,end:?Dint)\n"
	                         "\n"
	                         "next->?T2?Dint?O"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)&di_sgi_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&di_sgi_deepcopy,
				/* .tp_any_ctor  = */ (dfunptr_t)&di_sgi_init,
				TYPE_FIXED_ALLOCATOR(DefaultIterator_WithSizeAndGetItemIndex)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&di_sgi_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&di_sgi_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &di_sgi_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&di_sgip_iter_next,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ di_sgi_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};

INTERN DeeTypeObject DefaultIterator_WithSizeAndGetItemIndexFast_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_IterWithSizeAndGetItemIndexFast",
	/* .tp_doc      = */ DOC("(objWithGetItemIndexFast,index:?Dint,end:?Dint)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)&di_sgif_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&di_sgif_deepcopy,
				/* .tp_any_ctor  = */ (dfunptr_t)&di_sgif_init,
				TYPE_FIXED_ALLOCATOR(DefaultIterator_WithSizeAndGetItemIndex)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&di_sgif_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&di_sgif_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &di_sgif_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&di_sgif_iter_next,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ di_sgif_getsets,
	/* .tp_members       = */ di_sgif_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};

INTERN DeeTypeObject DefaultIterator_WithSizeAndGetItemIndexFastPair_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_IterWithSizeAndGetItemIndexFastPair",
	/* .tp_doc      = */ DOC("(objWithGetItemIndexFast,index:?Dint,end:?Dint)\n"
	                         "\n"
	                         "next->?T2?Dint?O"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)&di_sgif_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&di_sgif_deepcopy,
				/* .tp_any_ctor  = */ (dfunptr_t)&di_sgif_init,
				TYPE_FIXED_ALLOCATOR(DefaultIterator_WithSizeAndGetItemIndex)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&di_sgif_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&di_sgif_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &di_sgif_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&di_sgifp_iter_next,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ di_sgif_getsets,
	/* .tp_members       = */ di_sgif_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
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
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)&di_stgi_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&di_stgi_deepcopy,
				/* .tp_any_ctor  = */ (dfunptr_t)&di_stgi_init,
				TYPE_FIXED_ALLOCATOR(DefaultIterator_WithSizeAndGetItemIndex)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&di_stgi_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&di_stgi_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &di_stgi_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&di_stgi_iter_next,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ di_stgi_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
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
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)&di_stgi_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&di_stgi_deepcopy,
				/* .tp_any_ctor  = */ (dfunptr_t)&di_stgi_init,
				TYPE_FIXED_ALLOCATOR(DefaultIterator_WithSizeAndGetItemIndex)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&di_stgi_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&di_stgi_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &di_stgi_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&di_stgip_iter_next,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ di_stgi_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};
























/************************************************************************/
/* DefaultIterator_WithGetItem_Type                                     */
/* DefaultIterator_WithTGetItem_Type                                    */
/************************************************************************/

STATIC_ASSERT(offsetof(DefaultIterator_WithGetItem, dig_seq) == offsetof(DefaultIterator_WithTGetItem, ditg_seq));
STATIC_ASSERT(offsetof(DefaultIterator_WithGetItem, dig_index) == offsetof(DefaultIterator_WithTGetItem, ditg_index));
#ifndef CONFIG_NO_THREADS
STATIC_ASSERT(offsetof(DefaultIterator_WithGetItem, dig_lock) == offsetof(DefaultIterator_WithTGetItem, ditg_lock));
#endif /* !CONFIG_NO_THREADS */

PRIVATE WUNUSED NONNULL((1)) int DCALL
di_g_init(DefaultIterator_WithGetItem *__restrict self,
          size_t argc, DeeObject *const *argv) {
	DeeTypeObject *seqtyp;
	if (DeeArg_Unpack(argc, argv, "oo:_IterWithGetItem",
	                  &self->dig_seq, &self->dig_index))
		goto err;
	seqtyp = Dee_TYPE(self->dig_seq);
	if ((!seqtyp->tp_seq || !seqtyp->tp_seq->tp_getitem) &&
	    !DeeType_InheritGetItem(seqtyp))
		goto err_no_getitem;
	self->dig_tp_getitem = seqtyp->tp_seq->tp_getitem;
	Dee_Incref(self->dig_seq);
	Dee_Incref(self->dig_index);
	Dee_atomic_lock_init(&self->dig_lock);
	return 0;
err_no_getitem:
	err_unimplemented_operator(seqtyp, OPERATOR_GETITEM);
err:
	return -1;
}

INTDEF WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
generic_tp_tgetitem(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index);

PRIVATE WUNUSED NONNULL((1)) int DCALL
di_tg_init(DefaultIterator_WithTGetItem *__restrict self,
           size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, "ooo:_IterWithTGetItem",
	                  &self->ditg_seq, &self->ditg_tp_seq, &self->ditg_index))
		goto err;
	if (DeeObject_AssertType(self->ditg_tp_seq, &DeeType_Type))
		goto err;
	if (DeeObject_AssertTypeOrAbstract(self->ditg_seq, self->ditg_tp_seq))
		goto err;
	if ((!self->ditg_tp_seq->tp_seq || !self->ditg_tp_seq->tp_seq->tp_getitem) &&
	    !DeeType_InheritGetItem(self->ditg_tp_seq))
		goto err_no_getitem;
	self->ditg_tp_tgetitem = DeeType_MapDefaultGetItem(self->ditg_tp_seq->tp_seq->tp_getitem, &,
	                                                   self->ditg_tp_seq->tp_seq->tp_getitem == &instance_getitem
	                                                   ? &instance_tgetitem
	                                                   : &generic_tp_tgetitem);
	Dee_Incref(self->ditg_seq);
	Dee_Incref(self->ditg_index);
	Dee_atomic_lock_init(&self->ditg_lock);
	return 0;
err_no_getitem:
	err_unimplemented_operator(self->ditg_tp_seq, OPERATOR_GETITEM);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
di_g_copy(DefaultIterator_WithGetItem *__restrict self,
          DefaultIterator_WithGetItem *__restrict other) {
	DREF DeeObject *index, *index_copy;
	DefaultIterator_WithGetItem_LockAcquire(other);
	index = other->dig_index;
	Dee_Incref(index);
	DefaultIterator_WithGetItem_LockRelease(other);
	index_copy = DeeObject_Copy(index);
	Dee_Decref_unlikely(index);
	if unlikely(!index_copy)
		goto err;
	self->dig_index = index_copy;
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
	DREF DeeObject *index, *index_copy;
	DefaultIterator_WithGetItem_LockAcquire(other);
	index = other->dig_index;
	Dee_Incref(index);
	DefaultIterator_WithGetItem_LockRelease(other);
	index_copy = DeeObject_DeepCopy(index);
	Dee_Decref_unlikely(index);
	if unlikely(!index_copy)
		goto err;
	self->dig_seq = DeeObject_DeepCopy(other->dig_seq);
	if unlikely(!self->dig_seq)
		goto err_index_copy;

	self->dig_index = index_copy;
	Dee_atomic_lock_init(&self->dig_lock);
	self->dig_tp_getitem = other->dig_tp_getitem;
	return 0;
err_index_copy:
	Dee_Decref(index_copy);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
di_tg_copy(DefaultIterator_WithTGetItem *__restrict self,
           DefaultIterator_WithTGetItem *__restrict other) {
	self->ditg_tp_seq = other->ditg_tp_seq;
	return di_g_copy((DefaultIterator_WithGetItem *)self,
	                 (DefaultIterator_WithGetItem *)other);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
di_tg_deepcopy(DefaultIterator_WithTGetItem *__restrict self,
               DefaultIterator_WithTGetItem *__restrict other) {
	self->ditg_tp_seq = other->ditg_tp_seq;
	return di_g_deepcopy((DefaultIterator_WithGetItem *)self,
	                     (DefaultIterator_WithGetItem *)other);
}

#define di_tg_fini di_g_fini
PRIVATE NONNULL((1)) void DCALL
di_g_fini(DefaultIterator_WithGetItem *__restrict self) {
	Dee_Decref(self->dig_seq);
	Dee_Decref(self->dig_index);
}

#define di_tg_visit di_g_visit
PRIVATE NONNULL((1, 2)) void DCALL
di_g_visit(DefaultIterator_WithGetItem *__restrict self,
           dvisit_t proc, void *arg) {
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
	result = Dee_HashCombine(result, DeeObject_Hash(index));
	Dee_Decref(index);
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
		if (DeeError_Catch(&DeeError_IndexError)) {
			Dee_Decref(new_index);
			return ITER_DONE;
		}
		if (!DeeError_Catch(&DeeError_UnboundItem))
			goto err_new_index;
		if (DeeObject_Inc(&new_index))
			goto err_new_index;
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
di_tg_iter_next(DefaultIterator_WithTGetItem *__restrict self) {
	DeeObject *old_index;
	DREF DeeObject *new_index, *result;
again:
	DefaultIterator_WithTGetItem_LockAcquire(self);
	old_index = self->ditg_index;
	Dee_Incref(old_index);
	DefaultIterator_WithTGetItem_LockRelease(self);
	new_index = old_index; /* Inherit reference */
	for (;;) {
		result = (*self->ditg_tp_tgetitem)(self->ditg_tp_seq, self->ditg_seq, new_index);
		if (result)
			break;
		if (DeeError_Catch(&DeeError_IndexError)) {
			Dee_Decref(new_index);
			return ITER_DONE;
		}
		if (!DeeError_Catch(&DeeError_UnboundItem))
			goto err_new_index;
		if (DeeObject_Inc(&new_index))
			goto err_new_index;
	}
	if (DeeObject_Inc(&new_index))
		goto err_new_index_result;
	DefaultIterator_WithTGetItem_LockAcquire(self);
	if unlikely(self->ditg_index != old_index) {
		DefaultIterator_WithTGetItem_LockRelease(self);
		Dee_Decref(new_index);
		Dee_Decref(result);
		goto again;
	}
	self->ditg_index = new_index; /* Inherit (x2) */
	DefaultIterator_WithTGetItem_LockRelease(self);
	Dee_Decref(old_index);
	return result;
err_new_index_result:
	Dee_Decref(result);
err_new_index:
	Dee_Decref(new_index);
	return NULL;
}

#define di_tg_cmp di_g_cmp
PRIVATE struct type_cmp di_g_cmp = {
	/* .tp_hash          = */ (Dee_hash_t (DCALL *)(DeeObject *))&di_g_hash,
	/* .tp_compare_eq    = */ (int (DCALL *)(DeeObject *, DeeObject *))&di_g_compare,
	/* .tp_compare       = */ (int (DCALL *)(DeeObject *, DeeObject *))&di_g_compare,
	/* .tp_trycompare_eq = */ NULL,
};

#define di_tg_members di_g_members
PRIVATE struct type_member tpconst di_g_members[] = {
	TYPE_MEMBER_FIELD("__seq__", STRUCT_OBJECT, offsetof(DefaultIterator_WithGetItem, dig_seq)),
	TYPE_MEMBER_END,
};

#define di_tg_getsets di_g_getsets
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
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)&di_g_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&di_g_deepcopy,
				/* .tp_any_ctor  = */ (dfunptr_t)&di_g_init,
				TYPE_FIXED_ALLOCATOR_GC(DefaultIterator_WithGetItem)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&di_g_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&di_g_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &di_g_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&di_g_iter_next,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ di_g_getsets,
	/* .tp_members       = */ di_g_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};

INTERN DeeTypeObject DefaultIterator_WithTGetItem_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_IterWithTGetItem",
	/* .tp_doc      = */ DOC("(objWithGetItem,objType:?DType,index)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL | TP_FGC,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)&di_tg_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&di_tg_deepcopy,
				/* .tp_any_ctor  = */ (dfunptr_t)&di_tg_init,
				TYPE_FIXED_ALLOCATOR_GC(DefaultIterator_WithTGetItem)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&di_tg_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&di_tg_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &di_tg_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&di_tg_iter_next,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ di_tg_getsets,
	/* .tp_members       = */ di_tg_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};
























/************************************************************************/
/* DefaultIterator_WithSizeObAndGetItem_Type                              */
/* DefaultIterator_WithTSizeAndGetItem_Type                             */
/************************************************************************/

STATIC_ASSERT(offsetof(DefaultIterator_WithSizeObAndGetItem, disg_seq) == offsetof(DefaultIterator_WithGetItem, dig_seq));
STATIC_ASSERT(offsetof(DefaultIterator_WithSizeObAndGetItem, disg_index) == offsetof(DefaultIterator_WithGetItem, dig_index));
#ifndef CONFIG_NO_THREADS
STATIC_ASSERT(offsetof(DefaultIterator_WithSizeObAndGetItem, disg_lock) == offsetof(DefaultIterator_WithGetItem, dig_lock));
#endif /* !CONFIG_NO_THREADS */
STATIC_ASSERT(offsetof(DefaultIterator_WithSizeObAndGetItem, disg_seq) == offsetof(DefaultIterator_WithTSizeAndGetItem, ditsg_seq));
STATIC_ASSERT(offsetof(DefaultIterator_WithSizeObAndGetItem, disg_index) == offsetof(DefaultIterator_WithTSizeAndGetItem, ditsg_index));
#ifndef CONFIG_NO_THREADS
STATIC_ASSERT(offsetof(DefaultIterator_WithSizeObAndGetItem, disg_lock) == offsetof(DefaultIterator_WithTSizeAndGetItem, ditsg_lock));
#endif /* !CONFIG_NO_THREADS */
STATIC_ASSERT(offsetof(DefaultIterator_WithSizeObAndGetItem, disg_end) == offsetof(DefaultIterator_WithTSizeAndGetItem, ditsg_end));

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
di_sg_copy(DefaultIterator_WithSizeObAndGetItem *__restrict self,
           DefaultIterator_WithSizeObAndGetItem *__restrict other) {
	DREF DeeObject *index, *index_copy;
	DefaultIterator_WithSizeAndGetItem_LockAcquire(other);
	index = other->disg_index;
	Dee_Incref(index);
	DefaultIterator_WithSizeAndGetItem_LockRelease(other);
	index_copy = DeeObject_Copy(index);
	Dee_Decref_unlikely(index);
	if unlikely(!index_copy)
		goto err;
	self->disg_index = index_copy;
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
	if (DeeArg_Unpack(argc, argv, "ooo:_IterWithSizeAndGetItem",
	                  &self->disg_seq, &self->disg_index, &self->disg_end))
		goto err;
	seqtyp = Dee_TYPE(self->disg_seq);
	if ((!seqtyp->tp_seq || !seqtyp->tp_seq->tp_getitem) &&
	    !DeeType_InheritGetItem(seqtyp))
		goto err_no_getitem;
	self->disg_tp_getitem = seqtyp->tp_seq->tp_getitem;
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

PRIVATE WUNUSED NONNULL((1)) int DCALL
di_tsg_init(DefaultIterator_WithTSizeAndGetItem *__restrict self,
            size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, "oooo:_IterWithTSizeObAndGetItem",
	                  &self->ditsg_seq, &self->ditsg_tp_seq,
	                  &self->ditsg_index, &self->ditsg_end))
		goto err;
	if (DeeObject_AssertType(self->ditsg_tp_seq, &DeeType_Type))
		goto err;
	if (DeeObject_AssertTypeOrAbstract(self->ditsg_seq, self->ditsg_tp_seq))
		goto err;
	if ((!self->ditsg_tp_seq->tp_seq || !self->ditsg_tp_seq->tp_seq->tp_getitem) &&
	    !DeeType_InheritGetItem(self->ditsg_tp_seq))
		goto err_no_getitem;
	self->ditsg_tp_tgetitem = DeeType_MapDefaultGetItem(self->ditsg_tp_seq->tp_seq->tp_getitem, &,
	                                                    self->ditsg_tp_seq->tp_seq->tp_getitem == &instance_getitem
	                                                    ? &instance_tgetitem
	                                                    : &generic_tp_tgetitem);
	Dee_atomic_lock_init(&self->ditsg_lock);
	Dee_Incref(self->ditsg_seq);
	Dee_Incref(self->ditsg_index);
	Dee_Incref(self->ditsg_end);
	return 0;
err_no_getitem:
	err_unimplemented_operator(self->ditsg_tp_seq, OPERATOR_GETITEM);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
di_sg_deepcopy(DefaultIterator_WithSizeObAndGetItem *__restrict self,
               DefaultIterator_WithSizeObAndGetItem *__restrict other) {
	DREF DeeObject *index, *index_copy, *end_copy;
	DefaultIterator_WithSizeAndGetItem_LockAcquire(other);
	index = other->disg_index;
	Dee_Incref(index);
	DefaultIterator_WithSizeAndGetItem_LockRelease(other);
	index_copy = DeeObject_DeepCopy(index);
	Dee_Decref_unlikely(index);
	if unlikely(!index_copy)
		goto err;
	end_copy = DeeObject_DeepCopy(other->disg_end);
	if unlikely(!end_copy)
		goto err_index_copy;
	self->disg_seq = DeeObject_DeepCopy(other->disg_seq);
	if unlikely(!self->disg_seq)
		goto err_index_copy_end_copy;
	self->disg_index = index_copy;
	self->disg_end   = end_copy;
	Dee_atomic_lock_init(&self->disg_lock);
	self->disg_tp_getitem = other->disg_tp_getitem;
	return 0;
err_index_copy_end_copy:
	Dee_Decref(end_copy);
err_index_copy:
	Dee_Decref(index_copy);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
di_tsg_copy(DefaultIterator_WithTSizeAndGetItem *__restrict self,
            DefaultIterator_WithTSizeAndGetItem *__restrict other) {
	self->ditsg_tp_seq = other->ditsg_tp_seq;
	return di_sg_copy((DefaultIterator_WithSizeObAndGetItem *)self,
	                  (DefaultIterator_WithSizeObAndGetItem *)other);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
di_tsg_deepcopy(DefaultIterator_WithTSizeAndGetItem *__restrict self,
                DefaultIterator_WithTSizeAndGetItem *__restrict other) {
	self->ditsg_tp_seq = other->ditsg_tp_seq;
	return di_sg_deepcopy((DefaultIterator_WithSizeObAndGetItem *)self,
	                      (DefaultIterator_WithSizeObAndGetItem *)other);
}

#define di_tsg_fini di_sg_fini
PRIVATE NONNULL((1)) void DCALL
di_sg_fini(DefaultIterator_WithSizeObAndGetItem *__restrict self) {
	Dee_Decref(self->disg_seq);
	Dee_Decref(self->disg_index);
	Dee_Decref(self->disg_end);
}

#define di_tsg_visit di_sg_visit
PRIVATE NONNULL((1, 2)) void DCALL
di_sg_visit(DefaultIterator_WithSizeObAndGetItem *__restrict self,
            dvisit_t proc, void *arg) {
	Dee_Visit(self->disg_seq);
	DefaultIterator_WithSizeAndGetItem_LockAcquire(self);
	Dee_Visit(self->disg_index);
	DefaultIterator_WithSizeAndGetItem_LockRelease(self);
	Dee_Visit(self->disg_end);
}

#define di_sg_compare  di_g_compare
#define di_tsg_compare di_g_compare

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
		if (DeeError_Catch(&DeeError_IndexError)) {
			Dee_Decref(new_index);
			return ITER_DONE;
		}
		if (!DeeError_Catch(&DeeError_UnboundItem))
			goto err_new_index;
		if (DeeObject_Inc(&new_index))
			goto err_new_index;
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
		if (DeeError_Catch(&DeeError_IndexError)) {
			Dee_Decref(new_index);
			return ITER_DONE;
		}
		if (!DeeError_Catch(&DeeError_UnboundItem))
			goto err_new_index;
		if (DeeObject_Inc(&new_index))
			goto err_new_index;
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

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
di_tsg_iter_next(DefaultIterator_WithTSizeAndGetItem *__restrict self) {
	DeeObject *old_index;
	DREF DeeObject *new_index, *result;
again:
	DefaultIterator_WithTSizeAndGetItem_LockAcquire(self);
	old_index = self->ditsg_index;
	Dee_Incref(old_index);
	DefaultIterator_WithTSizeAndGetItem_LockRelease(self);
	new_index = old_index; /* Inherit reference */
	for (;;) {
		int temp = DeeObject_CmpGeAsBool(new_index, self->ditsg_end);
		if (temp != 0) {
			if unlikely(temp < 0)
				goto err_new_index;
			Dee_Decref(new_index);
			return ITER_DONE;
		}
		result = (*self->ditsg_tp_tgetitem)(self->ditsg_tp_seq, self->ditsg_seq, new_index);
		if (result)
			break;
		if (DeeError_Catch(&DeeError_IndexError)) {
			Dee_Decref(new_index);
			return ITER_DONE;
		}
		if (!DeeError_Catch(&DeeError_UnboundItem))
			goto err_new_index;
		if (DeeObject_Inc(&new_index))
			goto err_new_index;
	}
	if (DeeObject_Inc(&new_index))
		goto err_new_index_result;
	DefaultIterator_WithTSizeAndGetItem_LockAcquire(self);
	if unlikely(self->ditsg_index != old_index) {
		DefaultIterator_WithTSizeAndGetItem_LockRelease(self);
		Dee_Decref(new_index);
		Dee_Decref(result);
		goto again;
	}
	self->ditsg_index = new_index; /* Inherit (x2) */
	DefaultIterator_WithTSizeAndGetItem_LockRelease(self);
	Dee_Decref(old_index);
	return result;
err_new_index_result:
	Dee_Decref(result);
err_new_index:
	Dee_Decref(new_index);
	return NULL;
}


#define di_sg_cmp  di_g_cmp
#define di_tsg_cmp di_g_cmp

#define di_sg_members  di_g_members
#define di_tsg_members di_g_members

#define di_sg_getsets  di_g_getsets
#define di_tsg_getsets di_g_getsets

INTERN DeeTypeObject DefaultIterator_WithSizeObAndGetItem_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_IterWithSizeAndGetItem",
	/* .tp_doc      = */ DOC("(objWithGetItem,index,end)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL | TP_FGC,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)&di_sg_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&di_sg_deepcopy,
				/* .tp_any_ctor  = */ (dfunptr_t)&di_sg_init,
				TYPE_FIXED_ALLOCATOR_GC(DefaultIterator_WithSizeObAndGetItem)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&di_sg_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&di_sg_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &di_sg_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&di_sg_iter_next,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ di_sg_getsets,
	/* .tp_members       = */ di_sg_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
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
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)&di_sg_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&di_sg_deepcopy,
				/* .tp_any_ctor  = */ (dfunptr_t)&di_sg_init,
				TYPE_FIXED_ALLOCATOR_GC(DefaultIterator_WithSizeObAndGetItem)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&di_sg_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&di_sg_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &di_sg_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&di_sgp_iter_next,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ di_sg_getsets,
	/* .tp_members       = */ di_sg_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};

INTERN DeeTypeObject DefaultIterator_WithTSizeAndGetItem_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_IterWithTSizeObAndGetItem",
	/* .tp_doc      = */ DOC("(objWithGetItem,objType:?DType,index,end)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL | TP_FGC,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)&di_tsg_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&di_tsg_deepcopy,
				/* .tp_any_ctor  = */ (dfunptr_t)&di_tsg_init,
				TYPE_FIXED_ALLOCATOR_GC(DefaultIterator_WithTSizeAndGetItem)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&di_tsg_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&di_tsg_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &di_tsg_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&di_tsg_iter_next,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ di_tsg_getsets,
	/* .tp_members       = */ di_tsg_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};
























/************************************************************************/
/* DefaultIterator_WithNextAndLimit_Type                                */
/************************************************************************/

STATIC_ASSERT(offsetof(DefaultIterator_WithNextAndLimit, dinl_iter) == offsetof(DefaultIterator_WithSizeAndGetItemIndex, disgi_seq));

PRIVATE WUNUSED NONNULL((1)) int DCALL
di_nl_init(DefaultIterator_WithNextAndLimit *__restrict self,
           size_t argc, DeeObject *const *argv) {
	DeeTypeObject *itertyp;
	if (DeeArg_Unpack(argc, argv, "o" UNPuSIZ ":_IterWithNextAndLimit",
	                  &self->dinl_iter, &self->dinl_limit))
		goto err;
	itertyp = Dee_TYPE(self->dinl_iter);
	if (!itertyp->tp_iter_next && !DeeType_InheritIterNext(itertyp))
		goto err_no_next;
	self->dinl_tp_next = itertyp->tp_iter_next;
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
	self->dinl_limit   = other->dinl_limit;
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
	self->dinl_limit   = other->dinl_limit;
	return 0;
err:
	return -1;
}

#define di_nl_fini  di_sgi_fini
#define di_nl_visit di_sgi_visit

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
	if (atomic_read(&self->dinl_limit) == 0)
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
	/* .tp_trycompare_eq = */ NULL,
};

PRIVATE struct type_member tpconst di_nl_members[] = {
	TYPE_MEMBER_FIELD("__iter__", STRUCT_OBJECT, offsetof(DefaultIterator_WithNextAndLimit, dinl_iter)),
	TYPE_MEMBER_FIELD("__limit__", STRUCT_ATOMIC | STRUCT_SIZE_T, offsetof(DefaultIterator_WithNextAndLimit, dinl_limit)),
	TYPE_MEMBER_END,
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
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)&di_nl_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&di_nl_deepcopy,
				/* .tp_any_ctor  = */ (dfunptr_t)&di_nl_init,
				TYPE_FIXED_ALLOCATOR(DefaultIterator_WithNextAndLimit)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&di_nl_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&di_nl_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&di_nl_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &di_nl_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&di_nl_iter_next,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ di_nl_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};
























/************************************************************************/
/* DefaultIterator_WithIterKeysAndGetItemSeq_Type                       */
/* DefaultIterator_WithIterKeysAndTryGetItemSeq_Type                    */
/* DefaultIterator_WithIterKeysAndTTryGetItemSeq_Type                   */
/* DefaultIterator_WithIterKeysAndGetItemMap_Type                       */
/* DefaultIterator_WithIterKeysAndTryGetItemMap_Type                    */
/* DefaultIterator_WithIterKeysAndTTryGetItemMap_Type                   */
/************************************************************************/

STATIC_ASSERT(offsetof(DefaultIterator_WithIterKeysAndGetItem, diikgi_seq) == offsetof(DefaultIterator_WithIterKeysAndTGetItem, diiktgi_seq));
STATIC_ASSERT(offsetof(DefaultIterator_WithIterKeysAndGetItem, diikgi_iter) == offsetof(DefaultIterator_WithIterKeysAndTGetItem, diiktgi_iter));
STATIC_ASSERT(offsetof(DefaultIterator_WithIterKeysAndGetItem, diikgi_tp_next) == offsetof(DefaultIterator_WithIterKeysAndTGetItem, diiktgi_tp_next));
STATIC_ASSERT(offsetof(DefaultIterator_WithIterKeysAndGetItem, diikgi_tp_getitem) == offsetof(DefaultIterator_WithIterKeysAndTGetItem, diiktgi_tp_tgetitem));

#define di_ikgim_init di_ikgis_init
PRIVATE WUNUSED NONNULL((1)) int DCALL
di_ikgis_init(DefaultIterator_WithIterKeysAndGetItem *__restrict self,
              size_t argc, DeeObject *const *argv) {
	DeeTypeObject *itertyp, *seqtyp;
	if (DeeArg_Unpack(argc, argv, "oo:_IterWithIterKeysAndGetItemForSeq", /* And `_IterWithIterKeysAndGetItemForMap' */
	                  &self->diikgi_seq, &self->diikgi_iter))
		goto err;
	itertyp = Dee_TYPE(self->diikgi_iter);
	if (!itertyp->tp_iter_next && !DeeType_InheritIterNext(itertyp))
		goto err_no_next;
	seqtyp = Dee_TYPE(self->diikgi_seq);
	if ((!seqtyp->tp_seq || !seqtyp->tp_seq->tp_getitem) && !DeeType_InheritGetItem(seqtyp))
		goto err_no_getitem;
	self->diikgi_tp_next    = itertyp->tp_iter_next;
	self->diikgi_tp_getitem = seqtyp->tp_seq->tp_getitem;
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

#define di_iktgim_init di_iktgis_init
PRIVATE WUNUSED NONNULL((1)) int DCALL
di_iktgis_init(DefaultIterator_WithIterKeysAndTGetItem *__restrict self,
               size_t argc, DeeObject *const *argv) {
	DeeTypeObject *itertyp, *seqtyp;
	if (DeeArg_Unpack(argc, argv, "ooo:_IterWithIterKeysAndTGetItemForSeq", /* And `_IterWithIterKeysAndTGetItemForMap' */
	                  &self->diiktgi_seq, &self->diiktgi_tp_seq, &self->diiktgi_iter))
		goto err;
	if (DeeObject_AssertType(self->diiktgi_tp_seq, &DeeType_Type))
		goto err;
	if (DeeObject_AssertTypeOrAbstract(self->diiktgi_seq, self->diiktgi_tp_seq))
		goto err;
	itertyp = Dee_TYPE(self->diiktgi_iter);
	if (!itertyp->tp_iter_next && !DeeType_InheritIterNext(itertyp))
		goto err_no_next;
	seqtyp = self->diiktgi_tp_seq;
	if ((!seqtyp->tp_seq || !seqtyp->tp_seq->tp_getitem) && !DeeType_InheritGetItem(seqtyp))
		goto err_no_getitem;
	self->diiktgi_tp_next     = itertyp->tp_iter_next;
	self->diiktgi_tp_tgetitem = DeeType_MapDefaultGetItem(seqtyp->tp_seq->tp_getitem, &,
	                                                      seqtyp->tp_seq->tp_getitem == &instance_getitem
	                                                      ? &instance_tgetitem
	                                                      : &generic_tp_tgetitem);
	Dee_Incref(self->diiktgi_iter);
	Dee_Incref(self->diiktgi_seq);
	return 0;
err_no_getitem:
	return err_unimplemented_operator(seqtyp, OPERATOR_GETITEM);
err_no_next:
	return err_unimplemented_operator(itertyp, OPERATOR_ITERNEXT);
err:
	return -1;
}

#define di_iktrgim_init di_iktrgis_init
PRIVATE WUNUSED NONNULL((1)) int DCALL
di_iktrgis_init(DefaultIterator_WithIterKeysAndGetItem *__restrict self,
                size_t argc, DeeObject *const *argv) {
	DeeTypeObject *itertyp, *seqtyp;
	if (DeeArg_Unpack(argc, argv, "oo:_IterWithIterKeysAndTryGetItemForSeq", /* And `_IterWithIterKeysAndTryGetItemForMap' */
	                  &self->diikgi_seq, &self->diikgi_iter))
		goto err;
	itertyp = Dee_TYPE(self->diikgi_iter);
	if (!itertyp->tp_iter_next && !DeeType_InheritIterNext(itertyp))
		goto err_no_next;
	seqtyp = Dee_TYPE(self->diikgi_seq);
	if ((!seqtyp->tp_seq || !seqtyp->tp_seq->tp_trygetitem) && !DeeType_InheritGetItem(seqtyp))
		goto err_no_getitem;
	self->diikgi_tp_next    = itertyp->tp_iter_next;
	self->diikgi_tp_getitem = seqtyp->tp_seq->tp_trygetitem;
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

INTERN WUNUSED NONNULL((1, 2, 3)) DREF DeeObject *DCALL
generic_tp_ttrygetitem(DeeTypeObject *tp_self, DeeObject *self, DeeObject *index) {
	return (*tp_self->tp_seq->tp_trygetitem)(self, index);
}

#define di_ikttrgim_init di_ikttrgis_init
PRIVATE WUNUSED NONNULL((1)) int DCALL
di_ikttrgis_init(DefaultIterator_WithIterKeysAndTGetItem *__restrict self,
                 size_t argc, DeeObject *const *argv) {
	DeeTypeObject *itertyp, *seqtyp;
	if (DeeArg_Unpack(argc, argv, "ooo:_IterWithIterKeysAndTTryGetItemForSeq", /* And `_IterWithIterKeysAndTTryGetItemForMap' */
	                  &self->diiktgi_seq, &self->diiktgi_tp_seq, &self->diiktgi_iter))
		goto err;
	if (DeeObject_AssertType(self->diiktgi_tp_seq, &DeeType_Type))
		goto err;
	if (DeeObject_AssertTypeOrAbstract(self->diiktgi_seq, self->diiktgi_tp_seq))
		goto err;
	itertyp = Dee_TYPE(self->diiktgi_iter);
	if (!itertyp->tp_iter_next && !DeeType_InheritIterNext(itertyp))
		goto err_no_next;
	seqtyp = self->diiktgi_tp_seq;
	if ((!seqtyp->tp_seq || !seqtyp->tp_seq->tp_trygetitem) && !DeeType_InheritGetItem(seqtyp))
		goto err_no_getitem;
	self->diiktgi_tp_next     = itertyp->tp_iter_next;
	self->diiktgi_tp_tgetitem = DeeType_MapDefaultTryGetItem(seqtyp->tp_seq->tp_trygetitem, &,
	                                                         &generic_tp_ttrygetitem);
	Dee_Incref(self->diiktgi_iter);
	Dee_Incref(self->diiktgi_seq);
	return 0;
err_no_getitem:
	return err_unimplemented_operator(seqtyp, OPERATOR_GETITEM);
err_no_next:
	return err_unimplemented_operator(itertyp, OPERATOR_ITERNEXT);
err:
	return -1;
}


#define di_iktrgis_copy di_ikgis_copy
#define di_ikgim_copy   di_ikgis_copy
#define di_iktrgim_copy di_ikgis_copy
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
di_ikgis_copy(DefaultIterator_WithIterKeysAndGetItem *__restrict self,
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

#define di_iktgim_copy   di_ikttrgis_copy
#define di_iktgis_copy   di_ikttrgis_copy
#define di_ikttrgim_copy di_ikttrgis_copy
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
di_ikttrgis_copy(DefaultIterator_WithIterKeysAndTGetItem *__restrict self,
                 DefaultIterator_WithIterKeysAndTGetItem *__restrict other) {
	self->diiktgi_iter = DeeObject_Copy(other->diiktgi_iter);
	if unlikely(!self->diiktgi_iter)
		goto err;
	self->diiktgi_seq = other->diiktgi_seq;
	Dee_Incref(self->diiktgi_seq);
	self->diiktgi_tp_next     = other->diiktgi_tp_next;
	self->diiktgi_tp_tgetitem = other->diiktgi_tp_tgetitem;
	self->diiktgi_tp_seq      = other->diiktgi_tp_seq;
	return 0;
err:
	return -1;
}

#define di_iktrgis_deepcopy di_ikgis_deepcopy
#define di_iktrgim_deepcopy di_ikgis_deepcopy
#define di_ikgim_deepcopy   di_ikgis_deepcopy
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
di_ikgis_deepcopy(DefaultIterator_WithIterKeysAndGetItem *__restrict self,
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

#define di_iktgis_deepcopy   di_ikttrgis_deepcopy
#define di_iktgim_deepcopy   di_ikttrgis_deepcopy
#define di_ikttrgim_deepcopy di_ikttrgis_deepcopy
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
di_ikttrgis_deepcopy(DefaultIterator_WithIterKeysAndTGetItem *__restrict self,
                     DefaultIterator_WithIterKeysAndTGetItem *__restrict other) {
	self->diiktgi_iter = DeeObject_DeepCopy(other->diiktgi_iter);
	if unlikely(!self->diiktgi_iter)
		goto err;
	self->diiktgi_seq = DeeObject_DeepCopy(other->diiktgi_seq);
	if unlikely(!self->diiktgi_seq)
		goto err_iter;
	self->diiktgi_tp_next     = other->diiktgi_tp_next;
	self->diiktgi_tp_tgetitem = other->diiktgi_tp_tgetitem;
	self->diiktgi_tp_seq      = other->diiktgi_tp_seq;
	return 0;
err_iter:
	Dee_Decref(self->diiktgi_iter);
err:
	return -1;
}

#define di_iktrgis_fini  di_ikgis_fini
#define di_ikttrgis_fini di_ikgis_fini
#define di_ikgim_fini    di_ikgis_fini
#define di_iktrgim_fini  di_ikgis_fini
#define di_ikttrgim_fini di_ikgis_fini
#define di_iktgis_fini   di_ikgis_fini
#define di_iktgim_fini   di_ikgis_fini
PRIVATE NONNULL((1)) void DCALL
di_ikgis_fini(DefaultIterator_WithIterKeysAndGetItem *__restrict self) {
	Dee_Decref(self->diikgi_seq);
	Dee_Decref(self->diikgi_iter);
}

#define di_iktrgis_visit  di_ikgis_visit
#define di_ikttrgis_visit di_ikgis_visit
#define di_ikgim_visit    di_ikgis_visit
#define di_iktrgim_visit  di_ikgis_visit
#define di_ikttrgim_visit di_ikgis_visit
#define di_iktgis_visit   di_ikgis_visit
#define di_iktgim_visit   di_ikgis_visit
PRIVATE NONNULL((1, 2)) void DCALL
di_ikgis_visit(DefaultIterator_WithIterKeysAndGetItem *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(self->diikgi_seq);
	Dee_Visit(self->diikgi_iter);
}

#define di_iktrgis_hash  di_ikgis_hash
#define di_ikttrgis_hash di_ikgis_hash
#define di_ikgim_hash    di_ikgis_hash
#define di_iktrgim_hash  di_ikgis_hash
#define di_ikttrgim_hash di_ikgis_hash
#define di_iktgis_hash   di_ikgis_hash
#define di_iktgim_hash   di_ikgis_hash
PRIVATE WUNUSED NONNULL((1)) Dee_hash_t DCALL
di_ikgis_hash(DefaultIterator_WithIterKeysAndGetItem *__restrict self) {
	return DeeObject_Hash(self->diikgi_iter);
}

#define di_iktrgis_compare_eq  di_ikgis_compare_eq
#define di_ikttrgis_compare_eq di_ikgis_compare_eq
#define di_ikgim_compare_eq    di_ikgis_compare_eq
#define di_iktrgim_compare_eq  di_ikgis_compare_eq
#define di_ikttrgim_compare_eq di_ikgis_compare_eq
#define di_iktgis_compare_eq   di_ikgis_compare_eq
#define di_iktgim_compare_eq   di_ikgis_compare_eq
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
di_ikgis_compare_eq(DefaultIterator_WithIterKeysAndGetItem *self,
                    DefaultIterator_WithIterKeysAndGetItem *other) {
	if (DeeObject_AssertTypeExact(other, Dee_TYPE(self)))
		goto err;
	return DeeObject_CompareEq(self->diikgi_iter, other->diikgi_iter);
err:
	return Dee_COMPARE_ERR;
}

#define di_iktrgis_compare  di_ikgis_compare
#define di_ikttrgis_compare di_ikgis_compare
#define di_ikgim_compare    di_ikgis_compare
#define di_iktrgim_compare  di_ikgis_compare
#define di_ikttrgim_compare di_ikgis_compare
#define di_iktgis_compare   di_ikgis_compare
#define di_iktgim_compare   di_ikgis_compare
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
di_ikgis_compare(DefaultIterator_WithIterKeysAndGetItem *self,
                 DefaultIterator_WithIterKeysAndGetItem *other) {
	if (DeeObject_AssertTypeExact(other, Dee_TYPE(self)))
		goto err;
	return DeeObject_Compare(self->diikgi_iter, other->diikgi_iter);
err:
	return Dee_COMPARE_ERR;
}

#define di_iktrgis_trycompare_eq  di_ikgis_trycompare_eq
#define di_ikttrgis_trycompare_eq di_ikgis_trycompare_eq
#define di_ikgim_trycompare_eq    di_ikgis_trycompare_eq
#define di_iktrgim_trycompare_eq  di_ikgis_trycompare_eq
#define di_ikttrgim_trycompare_eq di_ikgis_trycompare_eq
#define di_iktgis_trycompare_eq   di_ikgis_trycompare_eq
#define di_iktgim_trycompare_eq   di_ikgis_trycompare_eq
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
di_ikgis_trycompare_eq(DefaultIterator_WithIterKeysAndGetItem *self,
                       DefaultIterator_WithIterKeysAndGetItem *other) {
	if (!DeeObject_InstanceOfExact(other, Dee_TYPE(self)))
		return 1;
	return DeeObject_TryCompareEq(self->diikgi_iter, other->diikgi_iter);
}

#define di_iktrgis_cmp  di_ikgis_cmp
#define di_ikttrgis_cmp di_ikgis_cmp
#define di_ikgim_cmp    di_ikgis_cmp
#define di_iktrgim_cmp  di_ikgis_cmp
#define di_ikttrgim_cmp di_ikgis_cmp
#define di_iktgis_cmp   di_ikgis_cmp
#define di_iktgim_cmp   di_ikgis_cmp
PRIVATE struct type_cmp di_ikgis_cmp = {
	/* .tp_hash          = */ (Dee_hash_t (DCALL *)(DeeObject *))&di_ikgis_hash,
	/* .tp_compare_eq    = */ (int (DCALL *)(DeeObject *, DeeObject *))&di_ikgis_compare_eq,
	/* .tp_compare       = */ (int (DCALL *)(DeeObject *, DeeObject *))&di_ikgis_compare,
	/* .tp_trycompare_eq = */ (int (DCALL *)(DeeObject *, DeeObject *))&di_ikgis_trycompare_eq,
};

#define di_ikttrgim_members di_ikttrgis_members
#define di_iktgis_members   di_ikttrgis_members
#define di_iktgim_members   di_ikttrgis_members
PRIVATE struct type_member tpconst di_ikttrgis_members[] = {
	TYPE_MEMBER_FIELD_DOC("__tpseq__", STRUCT_OBJECT, offsetof(DefaultIterator_WithIterKeysAndTGetItem, diiktgi_tp_seq), "->?DType"),
#define di_ikgis_members  (di_ikttrgis_members + 1)
#define di_iktrgis_members (di_ikttrgis_members + 1)
#define di_ikgim_members  (di_ikttrgis_members + 1)
#define di_iktrgim_members (di_ikttrgis_members + 1)
	TYPE_MEMBER_FIELD_DOC("__seq__", STRUCT_OBJECT, offsetof(DefaultIterator_WithIterKeysAndGetItem, diikgi_seq), "->?DSequence"),
	TYPE_MEMBER_FIELD_DOC("__iterkeys__", STRUCT_OBJECT, offsetof(DefaultIterator_WithIterKeysAndGetItem, diikgi_iter), "->?DIterator"),
	TYPE_MEMBER_END
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
di_ikgis_iter_next(DefaultIterator_WithIterKeysAndGetItem *__restrict self) {
	DREF DeeObject *value;
	DREF DeeObject *key;
nextkey:
	key = (*self->diikgi_tp_next)(self->diikgi_iter);
	if unlikely(!ITER_ISOK(key))
		return key;
	value = (*self->diikgi_tp_getitem)(self->diikgi_seq, key);
	Dee_Decref(key);
	if unlikely(!value) {
		if (DeeError_Catch(&DeeError_UnboundItem))
			goto nextkey;
		if (DeeError_Catch(&DeeError_IndexError))
			goto nextkey;
		if (DeeError_Catch(&DeeError_KeyError))
			goto nextkey;
	}
	return value;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
di_iktgis_iter_next(DefaultIterator_WithIterKeysAndTGetItem *__restrict self) {
	DREF DeeObject *value;
	DREF DeeObject *key;
nextkey:
	key = (*self->diiktgi_tp_next)(self->diiktgi_iter);
	if unlikely(!ITER_ISOK(key))
		return key;
	value = (*self->diiktgi_tp_tgetitem)(self->diiktgi_tp_seq, self->diiktgi_seq, key);
	Dee_Decref(key);
	if unlikely(!value) {
		if (DeeError_Catch(&DeeError_UnboundItem))
			goto nextkey;
		if (DeeError_Catch(&DeeError_IndexError))
			goto nextkey;
		if (DeeError_Catch(&DeeError_KeyError))
			goto nextkey;
	}
	return value;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
di_iktrgis_iter_next(DefaultIterator_WithIterKeysAndGetItem *__restrict self) {
	DREF DeeObject *value;
	DREF DeeObject *key;
nextkey:
	key = (*self->diikgi_tp_next)(self->diikgi_iter);
	if unlikely(!ITER_ISOK(key))
		return key;
	value = (*self->diikgi_tp_getitem)(self->diikgi_seq, key);
	Dee_Decref(key);
	if (value == ITER_DONE)
		goto nextkey;
	return value;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
di_ikttrgis_iter_next(DefaultIterator_WithIterKeysAndTGetItem *__restrict self) {
	DREF DeeObject *value;
	DREF DeeObject *key;
nextkey:
	key = (*self->diiktgi_tp_next)(self->diiktgi_iter);
	if unlikely(!ITER_ISOK(key))
		return key;
	value = (*self->diiktgi_tp_tgetitem)(self->diiktgi_tp_seq, self->diiktgi_seq, key);
	Dee_Decref(key);
	if (value == ITER_DONE)
		goto nextkey;
	return value;
}

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
	result = DeeTuple_NewUninitialized(2);
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
di_iktgim_iter_next(DefaultIterator_WithIterKeysAndTGetItem *__restrict self) {
	DREF DeeTupleObject *result;
	DREF DeeObject *value;
	DREF DeeObject *key;
nextkey:
	key = (*self->diiktgi_tp_next)(self->diiktgi_iter);
	if unlikely(!ITER_ISOK(key))
		return (DREF DeeTupleObject *)key;
	value = (*self->diiktgi_tp_tgetitem)(self->diiktgi_tp_seq, self->diiktgi_seq, key);
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
	result = DeeTuple_NewUninitialized(2);
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
	result = DeeTuple_NewUninitialized(2);
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

PRIVATE WUNUSED NONNULL((1)) DREF DeeTupleObject *DCALL
di_ikttrgim_iter_next(DefaultIterator_WithIterKeysAndTGetItem *__restrict self) {
	DREF DeeTupleObject *result;
	DREF DeeObject *value;
	DREF DeeObject *key;
nextkey:
	key = (*self->diiktgi_tp_next)(self->diiktgi_iter);
	if unlikely(!ITER_ISOK(key))
		return (DREF DeeTupleObject *)key;
	value = (*self->diiktgi_tp_tgetitem)(self->diiktgi_tp_seq, self->diiktgi_seq, key);
	if (!ITER_ISOK(value)) {
		if unlikely(!value)
			goto err_key;
		goto nextkey;
	}
	result = DeeTuple_NewUninitialized(2);
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

INTERN DeeTypeObject DefaultIterator_WithIterKeysAndGetItemSeq_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_IterWithIterKeysAndGetItemForSeq",
	/* .tp_doc      = */ DOC("(objWithGetItem,objWithNext)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)&di_ikgis_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&di_ikgis_deepcopy,
				/* .tp_any_ctor  = */ (dfunptr_t)&di_ikgis_init,
				TYPE_FIXED_ALLOCATOR(DefaultIterator_WithIterKeysAndGetItem)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&di_ikgis_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&di_ikgis_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &di_ikgis_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&di_ikgis_iter_next,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ di_ikgis_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};

INTERN DeeTypeObject DefaultIterator_WithIterKeysAndTGetItemSeq_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_IterWithIterKeysAndTGetItemForSeq",
	/* .tp_doc      = */ DOC("(objWithGetItem,objWithGetItemType:?DType,objWithNext)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)&di_iktgis_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&di_iktgis_deepcopy,
				/* .tp_any_ctor  = */ (dfunptr_t)&di_iktgis_init,
				TYPE_FIXED_ALLOCATOR(DefaultIterator_WithIterKeysAndTGetItem)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&di_iktgis_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&di_iktgis_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &di_iktgis_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&di_iktgis_iter_next,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ di_iktgis_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};

INTERN DeeTypeObject DefaultIterator_WithIterKeysAndTryGetItemSeq_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_IterWithIterKeysAndTryGetItemForSeq",
	/* .tp_doc      = */ DOC("(objWithTryGetItem,objWithNext)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)&di_iktrgis_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&di_iktrgis_deepcopy,
				/* .tp_any_ctor  = */ (dfunptr_t)&di_iktrgis_init,
				TYPE_FIXED_ALLOCATOR(DefaultIterator_WithIterKeysAndGetItem)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&di_iktrgis_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&di_iktrgis_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &di_iktrgis_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&di_iktrgis_iter_next,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ di_iktrgis_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};

INTERN DeeTypeObject DefaultIterator_WithIterKeysAndTTryGetItemSeq_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_IterWithIterKeysAndTTryGetItemForSeq",
	/* .tp_doc      = */ DOC("(objWithTryGetItem,objWithTryGetItemType:?DType,objWithNext)"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)&di_ikttrgis_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&di_ikttrgis_deepcopy,
				/* .tp_any_ctor  = */ (dfunptr_t)&di_ikttrgis_init,
				TYPE_FIXED_ALLOCATOR(DefaultIterator_WithIterKeysAndTGetItem)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&di_ikttrgis_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&di_ikttrgis_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &di_ikttrgis_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&di_ikttrgis_iter_next,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ di_ikttrgis_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};

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
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)&di_ikgim_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&di_ikgim_deepcopy,
				/* .tp_any_ctor  = */ (dfunptr_t)&di_ikgim_init,
				TYPE_FIXED_ALLOCATOR(DefaultIterator_WithIterKeysAndGetItem)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&di_ikgim_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&di_ikgim_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &di_ikgim_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&di_ikgim_iter_next,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ di_ikgim_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};

INTERN DeeTypeObject DefaultIterator_WithIterKeysAndTGetItemMap_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_IterWithIterKeysAndTGetItemForMap",
	/* .tp_doc      = */ DOC("(objWithGetItem,objWithGetItemType:?DType,objWithNext)\n"
	                         "next->?T2?O?O"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)&di_iktgim_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&di_iktgim_deepcopy,
				/* .tp_any_ctor  = */ (dfunptr_t)&di_iktgim_init,
				TYPE_FIXED_ALLOCATOR(DefaultIterator_WithIterKeysAndTGetItem)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&di_iktgim_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&di_iktgim_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &di_iktgim_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&di_iktgim_iter_next,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ di_iktgim_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
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
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)&di_iktrgim_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&di_iktrgim_deepcopy,
				/* .tp_any_ctor  = */ (dfunptr_t)&di_iktrgim_init,
				TYPE_FIXED_ALLOCATOR(DefaultIterator_WithIterKeysAndGetItem)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&di_iktrgim_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&di_iktrgim_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &di_iktrgim_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&di_iktrgim_iter_next,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ di_iktrgim_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};

INTERN DeeTypeObject DefaultIterator_WithIterKeysAndTTryGetItemMap_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_IterWithIterKeysAndTTryGetItemForMap",
	/* .tp_doc      = */ DOC("(objWithTryGetItem,objWithTryGetItemType:?DType,objWithNext)\n"
	                         "next->?T2?O?O"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)&di_ikttrgim_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&di_ikttrgim_deepcopy,
				/* .tp_any_ctor  = */ (dfunptr_t)&di_ikttrgim_init,
				TYPE_FIXED_ALLOCATOR(DefaultIterator_WithIterKeysAndTGetItem)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&di_ikttrgim_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&di_ikttrgim_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &di_ikttrgim_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&di_ikttrgim_iter_next,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ di_ikttrgim_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};
























/************************************************************************/
/* DefaultIterator_WithForeach_Type                                     */
/* DefaultIterator_WithForeachPair_Type                                 */
/* DefaultIterator_WithEnumerateSeq_Type                                */
/* DefaultIterator_WithEnumerateMap_Type                                */
/* DefaultIterator_WithEnumerateIndexSeq_Type                           */
/* DefaultIterator_WithEnumerateIndexMap_Type                           */
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
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)NULL, /* TODO */
				/* .tp_deep_ctor = */ (dfunptr_t)NULL, /* TODO */
				/* .tp_any_ctor  = */ (dfunptr_t)NULL, /* TODO */
				TYPE_FIXED_ALLOCATOR(DeeObject)
			}
		},
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL, /* TODO */
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL, /* TODO */
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
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
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)NULL, /* TODO */
				/* .tp_deep_ctor = */ (dfunptr_t)NULL, /* TODO */
				/* .tp_any_ctor  = */ (dfunptr_t)NULL, /* TODO */
				TYPE_FIXED_ALLOCATOR(DeeObject)
			}
		},
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL, /* TODO */
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL, /* TODO */
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
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
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)NULL, /* TODO */
				/* .tp_deep_ctor = */ (dfunptr_t)NULL, /* TODO */
				/* .tp_any_ctor  = */ (dfunptr_t)NULL, /* TODO */
				TYPE_FIXED_ALLOCATOR(DeeObject)
			}
		},
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL, /* TODO */
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL, /* TODO */
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
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
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)NULL, /* TODO */
				/* .tp_deep_ctor = */ (dfunptr_t)NULL, /* TODO */
				/* .tp_any_ctor  = */ (dfunptr_t)NULL, /* TODO */
				TYPE_FIXED_ALLOCATOR(DeeObject)
			}
		},
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL, /* TODO */
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL, /* TODO */
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
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
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)NULL, /* TODO */
				/* .tp_deep_ctor = */ (dfunptr_t)NULL, /* TODO */
				/* .tp_any_ctor  = */ (dfunptr_t)NULL, /* TODO */
				TYPE_FIXED_ALLOCATOR(DeeObject)
			}
		},
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL, /* TODO */
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL, /* TODO */
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};

INTERN DeeTypeObject DefaultIterator_WithEnumerateIndexMap_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_IterWithEnumerateIndexMap",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)NULL, /* TODO */
				/* .tp_deep_ctor = */ (dfunptr_t)NULL, /* TODO */
				/* .tp_any_ctor  = */ (dfunptr_t)NULL, /* TODO */
				TYPE_FIXED_ALLOCATOR(DeeObject)
			}
		},
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL, /* TODO */
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL, /* TODO */
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};


DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_DEFAULT_ITERATORS_C */
