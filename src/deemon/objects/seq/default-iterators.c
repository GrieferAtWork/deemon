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
#include <deemon/error.h>
#include <deemon/int.h>
#include <deemon/object.h>
#include <deemon/seq.h>
#include <deemon/util/atomic.h>
/**/

#include "default-iterators.h"

DECL_BEGIN

/************************************************************************/
/* DefaultIterator_WithGetItemIndex_Type                                */
/* DefaultIterator_WithSizeAndGetItemIndex_Type                         */
/* DefaultIterator_WithSizeAndGetItemIndexFast_Type                     */
/* DefaultIterator_WithSizeAndTryGetItemIndex_Type                      */
/************************************************************************/

STATIC_ASSERT(offsetof(DefaultIterator_WithGetItemIndex, digi_seq) == offsetof(DefaultIterator_WithSizeAndGetItemIndex, disgi_seq));
STATIC_ASSERT(offsetof(DefaultIterator_WithGetItemIndex, digi_index) == offsetof(DefaultIterator_WithSizeAndGetItemIndex, disgi_index));

#define di_sgi_copy  di_gi_copy
#define di_sgif_copy di_gi_copy
#define di_stgi_copy di_gi_copy
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
di_gi_copy(DefaultIterator_WithGetItemIndex *__restrict self,
           DefaultIterator_WithGetItemIndex *__restrict other) {
	self->digi_seq = other->digi_seq;
	Dee_Incref(self->digi_seq);
	self->digi_tp_getitem_index = other->digi_tp_getitem_index;
	self->digi_index = atomic_read(&other->digi_index);
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

#define di_sgi_compare  di_gi_compare
#define di_sgif_compare di_gi_compare
#define di_stgi_compare di_gi_compare
PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
di_gi_compare(DefaultIterator_WithGetItemIndex *self,
              DefaultIterator_WithGetItemIndex *other) {
	if (DeeObject_AssertTypeExact(other, Dee_TYPE(self)))
		goto err;
	if (self->digi_seq != other->digi_seq) {
		return Dee_CompareNe(DeeObject_Id(self->digi_seq),
		                     DeeObject_Id(other->digi_seq));
	}
	return Dee_Compare(self->digi_index, other->digi_index);
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
	/* .tp_hash          = */ NULL,
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
	/* .tp_doc      = */ NULL,
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
				/* .tp_any_ctor  = */ (dfunptr_t)NULL,
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

INTERN DeeTypeObject DefaultIterator_WithSizeAndGetItemIndex_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_IterWithSizeAndGetItemIndex",
	/* .tp_doc      = */ NULL,
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
				/* .tp_any_ctor  = */ (dfunptr_t)NULL,
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

INTERN DeeTypeObject DefaultIterator_WithSizeAndGetItemIndexFast_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_IterWithSizeAndGetItemIndexFast",
	/* .tp_doc      = */ NULL,
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
				/* .tp_any_ctor  = */ (dfunptr_t)NULL,
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

INTERN DeeTypeObject DefaultIterator_WithSizeAndTryGetItemIndex_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_IterWithSizeAndTryGetItemIndex",
	/* .tp_doc      = */ NULL,
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
				/* .tp_any_ctor  = */ (dfunptr_t)NULL,
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
























/************************************************************************/
/* DefaultIterator_WithGetItem_Type                                     */
/* DefaultIterator_WithTGetItem_Type                                    */
/************************************************************************/

STATIC_ASSERT(offsetof(DefaultIterator_WithGetItem, dig_seq) == offsetof(DefaultIterator_WithTGetItem, ditg_seq));
STATIC_ASSERT(offsetof(DefaultIterator_WithGetItem, dig_index) == offsetof(DefaultIterator_WithTGetItem, ditg_index));
#ifndef CONFIG_NO_THREADS
STATIC_ASSERT(offsetof(DefaultIterator_WithGetItem, dig_lock) == offsetof(DefaultIterator_WithTGetItem, ditg_lock));
#endif /* !CONFIG_NO_THREADS */

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
		Dee_Incref(new_index);
		Dee_Incref(result);
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
		Dee_Incref(new_index);
		Dee_Incref(result);
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
	/* .tp_hash          = */ NULL,
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
	/* .tp_doc      = */ NULL,
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
				/* .tp_any_ctor  = */ (dfunptr_t)NULL,
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
	/* .tp_doc      = */ NULL,
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
				/* .tp_any_ctor  = */ (dfunptr_t)NULL,
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
/* DefaultIterator_WithSizeAndGetItem_Type                              */
/* DefaultIterator_WithTSizeAndGetItem_Type                             */
/************************************************************************/

STATIC_ASSERT(offsetof(DefaultIterator_WithSizeAndGetItem, disg_seq) == offsetof(DefaultIterator_WithGetItem, dig_seq));
STATIC_ASSERT(offsetof(DefaultIterator_WithSizeAndGetItem, disg_index) == offsetof(DefaultIterator_WithGetItem, dig_index));
#ifndef CONFIG_NO_THREADS
STATIC_ASSERT(offsetof(DefaultIterator_WithSizeAndGetItem, disg_lock) == offsetof(DefaultIterator_WithGetItem, dig_lock));
#endif /* !CONFIG_NO_THREADS */
STATIC_ASSERT(offsetof(DefaultIterator_WithSizeAndGetItem, disg_seq) == offsetof(DefaultIterator_WithTSizeAndGetItem, ditsg_seq));
STATIC_ASSERT(offsetof(DefaultIterator_WithSizeAndGetItem, disg_index) == offsetof(DefaultIterator_WithTSizeAndGetItem, ditsg_index));
#ifndef CONFIG_NO_THREADS
STATIC_ASSERT(offsetof(DefaultIterator_WithSizeAndGetItem, disg_lock) == offsetof(DefaultIterator_WithTSizeAndGetItem, ditsg_lock));
#endif /* !CONFIG_NO_THREADS */
STATIC_ASSERT(offsetof(DefaultIterator_WithSizeAndGetItem, disg_end) == offsetof(DefaultIterator_WithTSizeAndGetItem, ditsg_end));

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
di_sg_copy(DefaultIterator_WithSizeAndGetItem *__restrict self,
           DefaultIterator_WithSizeAndGetItem *__restrict other) {
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

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
di_sg_deepcopy(DefaultIterator_WithSizeAndGetItem *__restrict self,
               DefaultIterator_WithSizeAndGetItem *__restrict other) {
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
	return di_sg_copy((DefaultIterator_WithSizeAndGetItem *)self,
	                  (DefaultIterator_WithSizeAndGetItem *)other);
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
di_tsg_deepcopy(DefaultIterator_WithTSizeAndGetItem *__restrict self,
                DefaultIterator_WithTSizeAndGetItem *__restrict other) {
	self->ditsg_tp_seq = other->ditsg_tp_seq;
	return di_sg_deepcopy((DefaultIterator_WithSizeAndGetItem *)self,
	                      (DefaultIterator_WithSizeAndGetItem *)other);
}

#define di_tsg_fini di_sg_fini
PRIVATE NONNULL((1)) void DCALL
di_sg_fini(DefaultIterator_WithSizeAndGetItem *__restrict self) {
	Dee_Decref(self->disg_seq);
	Dee_Decref(self->disg_index);
	Dee_Decref(self->disg_end);
}

#define di_tsg_visit di_sg_visit
PRIVATE NONNULL((1, 2)) void DCALL
di_sg_visit(DefaultIterator_WithSizeAndGetItem *__restrict self,
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
di_sg_iter_next(DefaultIterator_WithSizeAndGetItem *__restrict self) {
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
		Dee_Incref(new_index);
		Dee_Incref(result);
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
		Dee_Incref(new_index);
		Dee_Incref(result);
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

INTERN DeeTypeObject DefaultIterator_WithSizeAndGetItem_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_IterWithSizeAndGetItem",
	/* .tp_doc      = */ NULL,
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
				/* .tp_any_ctor  = */ (dfunptr_t)NULL,
				TYPE_FIXED_ALLOCATOR_GC(DefaultIterator_WithSizeAndGetItem)
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

INTERN DeeTypeObject DefaultIterator_WithTSizeAndGetItem_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_IterWithTSizeAndGetItem",
	/* .tp_doc      = */ NULL,
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
				/* .tp_any_ctor  = */ (dfunptr_t)NULL,
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

PRIVATE struct type_cmp di_nl_cmp = {
	/* .tp_hash          = */ NULL,
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
	/* .tp_doc      = */ NULL,
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
				/* .tp_any_ctor  = */ (dfunptr_t)NULL,
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
		/* .tp_bool = */ NULL
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
/* DefaultIterator_WithForeach_Type                                     */
/* DefaultIterator_WithForeachPair_Type                                 */
/************************************************************************/

INTERN DeeTypeObject DefaultIterator_WithForeach_Type = {
	/* TODO */
};
INTERN DeeTypeObject DefaultIterator_WithForeachPair_Type = {
	/* TODO */
};


DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_DEFAULT_ITERATORS_C */
