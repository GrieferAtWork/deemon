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
#ifndef GUARD_DEEMON_OBJECTS_SEQ_SVEC_C
#define GUARD_DEEMON_OBJECTS_SEQ_SVEC_C 1

#include "svec.h"

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/error.h>
#include <deemon/int.h>
#include <deemon/none.h>
#include <deemon/object.h>
#include <deemon/seq.h>
#include <deemon/system-features.h>
#include <deemon/util/lock.h>
#include <deemon/util/atomic.h>

#include "../../runtime/runtime_error.h"
#include "../../runtime/strings.h"

#undef SSIZE_MAX
#define SSIZE_MAX __SSIZE_MAX__

#ifdef __OPTIMIZE_SIZE__
#define NULL_IF_Os(v) NULL
#else /* __OPTIMIZE_SIZE__ */
#define NULL_IF_Os(v) v
#endif /* !__OPTIMIZE_SIZE__ */

DECL_BEGIN

#define RVI_GETPOS(x) atomic_read(&(x)->rvi_pos)

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
rveciter_copy(RefVectorIterator *__restrict self,
              RefVectorIterator *__restrict other) {
	self->rvi_vector = other->rvi_vector;
	self->rvi_pos    = RVI_GETPOS(other);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
rveciter_ctor(RefVectorIterator *__restrict self,
              size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, "o:_RefVectorIterator", &self->rvi_vector))
		goto err;
	if (DeeObject_AssertTypeExact(self->rvi_vector, &RefVector_Type))
		goto err;
	Dee_Incref(self->rvi_vector);
	self->rvi_pos = self->rvi_vector->rv_vector;
	return 0;
err:
	return -1;
}

PRIVATE NONNULL((1)) void DCALL
rveciter_fini(RefVectorIterator *__restrict self) {
	Dee_Decref(self->rvi_vector);
}

PRIVATE NONNULL((1, 2)) void DCALL
rveciter_visit(RefVectorIterator *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(self->rvi_vector);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rveciter_next(RefVectorIterator *__restrict self) {
	DREF DeeObject **p_result, *result;
	RefVector *vector = self->rvi_vector;
	for (;;) {
		do {
			p_result = atomic_read(&self->rvi_pos);
			if (p_result >= vector->rv_vector + vector->rv_length)
				return ITER_DONE;
		} while (!atomic_cmpxch_weak_or_write(&self->rvi_pos, p_result, p_result + 1));

		RefVector_XLockRead(vector);
		result = *p_result;
		Dee_XIncref(result);
		RefVector_XLockEndRead(vector);
		/* Skip NULL entries. */
		if (result)
			break;
	}
	return result;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
rveciter_bool(RefVectorIterator *__restrict self) {
	RefVector *vector = self->rvi_vector;
	if (RVI_GETPOS(self) >= vector->rv_vector + vector->rv_length)
		return 0;
	return 1;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
rveciter_eq(RefVectorIterator *self,
            RefVectorIterator *other) {
	if (DeeObject_AssertTypeExact(other, &RefVectorIterator_Type))
		goto err;
	return_bool(self->rvi_vector == other->rvi_vector &&
	            RVI_GETPOS(self) == RVI_GETPOS(other));
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
rveciter_ne(RefVectorIterator *self,
            RefVectorIterator *other) {
	if (DeeObject_AssertTypeExact(other, &RefVectorIterator_Type))
		goto err;
	return_bool(self->rvi_vector != other->rvi_vector ||
	            RVI_GETPOS(self) != RVI_GETPOS(other));
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
rveciter_lo(RefVectorIterator *self,
            RefVectorIterator *other) {
	if (DeeObject_AssertTypeExact(other, &RefVectorIterator_Type))
		goto err;
	return_bool(self->rvi_vector == other->rvi_vector
	            ? RVI_GETPOS(self) < RVI_GETPOS(other)
	            : self->rvi_vector < other->rvi_vector);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
rveciter_le(RefVectorIterator *self,
            RefVectorIterator *other) {
	if (DeeObject_AssertTypeExact(other, &RefVectorIterator_Type))
		goto err;
	return_bool(self->rvi_vector == other->rvi_vector
	            ? RVI_GETPOS(self) <= RVI_GETPOS(other)
	            : self->rvi_vector <= other->rvi_vector);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
rveciter_gr(RefVectorIterator *self,
            RefVectorIterator *other) {
	if (DeeObject_AssertTypeExact(other, &RefVectorIterator_Type))
		goto err;
	return_bool(self->rvi_vector == other->rvi_vector
	            ? RVI_GETPOS(self) > RVI_GETPOS(other)
	            : self->rvi_vector > other->rvi_vector);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
rveciter_ge(RefVectorIterator *self,
            RefVectorIterator *other) {
	if (DeeObject_AssertTypeExact(other, &RefVectorIterator_Type))
		goto err;
	return_bool(self->rvi_vector == other->rvi_vector
	            ? RVI_GETPOS(self) >= RVI_GETPOS(other)
	            : self->rvi_vector >= other->rvi_vector);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF RefVector *DCALL
rveciter_nii_getseq(RefVectorIterator *__restrict self) {
	return_reference_(self->rvi_vector);
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
rveciter_nii_getindex(RefVectorIterator *__restrict self) {
	return (size_t)(atomic_read(&self->rvi_pos) - self->rvi_vector->rv_vector);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
rveciter_nii_setindex(RefVectorIterator *__restrict self, size_t new_index) {
	if (new_index > self->rvi_vector->rv_length)
		new_index = self->rvi_vector->rv_length;
	atomic_write(&self->rvi_pos, self->rvi_vector->rv_vector + new_index);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
rveciter_nii_rewind(RefVectorIterator *__restrict self) {
	atomic_write(&self->rvi_pos, self->rvi_vector->rv_vector);
	return 0;
}

PRIVATE struct type_nii tpconst rveciter_nii = {
	/* .nii_class = */ TYPE_ITERX_CLASS_BIDIRECTIONAL,
	/* .nii_flags = */ TYPE_ITERX_FNORMAL,
	{
		/* .nii_common = */ {
			/* .nii_getseq   = */ (dfunptr_t)&rveciter_nii_getseq,
			/* .nii_getindex = */ (dfunptr_t)&rveciter_nii_getindex,
			/* .nii_setindex = */ (dfunptr_t)&rveciter_nii_setindex,
			/* .nii_rewind   = */ (dfunptr_t)&rveciter_nii_rewind,
			/* .nii_revert   = */ (dfunptr_t)NULL,
			/* .nii_advance  = */ (dfunptr_t)NULL,
			/* .nii_prev     = */ (dfunptr_t)NULL,
			/* .nii_next     = */ (dfunptr_t)NULL,
			/* .nii_hasprev  = */ (dfunptr_t)NULL,
			/* .nii_peek     = */ (dfunptr_t)NULL,
		}
	}
};

PRIVATE struct type_cmp rveciter_cmp = {
	/* .tp_hash          = */ NULL,
	/* .tp_compare_eq    = */ NULL,
	/* .tp_compare       = */ NULL,
	/* .tp_trycompare_eq = */ NULL,
	/* .tp_eq            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&rveciter_eq,
	/* .tp_ne            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&rveciter_ne,
	/* .tp_lo            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&rveciter_lo,
	/* .tp_le            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&rveciter_le,
	/* .tp_gr            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&rveciter_gr,
	/* .tp_ge            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&rveciter_ge,
	/* .tp_nii           = */ &rveciter_nii,
};

PRIVATE struct type_member tpconst rveciter_members[] = {
	TYPE_MEMBER_FIELD_DOC(STR_seq, STRUCT_OBJECT, offsetof(RefVectorIterator, rvi_vector), "->?Ert:RefVector"),
	TYPE_MEMBER_END
};

/* This types isn't actually be needed (Sequence would emulate it using `FastNsiIterator'),
 * but since ref-vectors are used in a couple of places, they get their own, dedicated
 * iterator type (which is a tiny bit faster than `FastNsiIterator') */
INTERN DeeTypeObject RefVectorIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_RefVectorIterator",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)&rveciter_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)&rveciter_ctor,
				TYPE_FIXED_ALLOCATOR(RefVectorIterator)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&rveciter_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&rveciter_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&rveciter_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &rveciter_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&rveciter_next,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ rveciter_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};


PRIVATE NONNULL((1)) void DCALL
rvec_fini(RefVector *__restrict self) {
	Dee_Decref(self->rv_owner);
}

PRIVATE NONNULL((1, 2)) void DCALL
rvec_visit(RefVector *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(self->rv_owner);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
rvec_bool(RefVector *__restrict self) {
	return self->rv_length != 0;
}

PRIVATE WUNUSED NONNULL((1)) DREF RefVectorIterator *DCALL
rvec_iter(RefVector *__restrict self) {
	DREF RefVectorIterator *result;
	result = DeeObject_MALLOC(RefVectorIterator);
	if unlikely(!result)
		goto done;
	DeeObject_Init(result, &RefVectorIterator_Type);
	Dee_Incref(self); /* Reference stored in `rvi_vector' */
	result->rvi_vector = self;
	result->rvi_pos    = self->rv_vector; /* Start at index 0 */
done:
	return result;
}

#ifndef __OPTIMIZE_SIZE__
PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
rvec_contains(RefVector *self, DeeObject *other) {
	size_t index;
	int temp;
	for (index = 0; index < self->rv_length; ++index) {
		DREF DeeObject *item;
		RefVector_XLockRead(self);
		item = self->rv_vector[index];
		Dee_XIncref(item);
		RefVector_XLockEndRead(self);
		if (!item)
			continue;
		temp = DeeObject_CompareEq(other, item);
		Dee_Decref(item);
		if (temp != 0) {
			if unlikely(temp < 0)
				goto err;
			return_true;
		}
	}
	return_false;
err:
	return NULL;
}
#endif /* !__OPTIMIZE_SIZE__ */

PRIVATE ATTR_COLD int DCALL err_readonly_rvec(void) {
	return DeeError_Throwf(&DeeError_ValueError,
	                       "Reference vector is not writable");
}


PRIVATE WUNUSED NONNULL((1)) size_t DCALL
rvec_size(RefVector *__restrict self) {
	ASSERT(self->rv_length != (size_t)-1);
	return self->rv_length;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rvec_getitem_index(RefVector *__restrict self, size_t index) {
	DREF DeeObject *result;
	if unlikely(index >= self->rv_length) {
		err_index_out_of_bounds((DeeObject *)self, index, self->rv_length);
		return NULL;
	}
	RefVector_XLockRead(self);
	result = self->rv_vector[index];
	if unlikely(!result) {
		RefVector_XLockEndRead(self);
		err_unbound_index((DeeObject *)self, index);
		return NULL;
	}
	Dee_Incref(result);
	RefVector_XLockEndRead(self);
	return result;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
rvec_delitem_index(RefVector *__restrict self, size_t index) {
	DREF DeeObject *oldobj;
	if unlikely(index >= self->rv_length)
		return err_index_out_of_bounds((DeeObject *)self, index, self->rv_length);
	if unlikely(!RefVector_IsWritable(self))
		return err_readonly_rvec();
	RefVector_LockWrite(self);
	oldobj = self->rv_vector[index];
	self->rv_vector[index] = NULL;
	RefVector_LockEndWrite(self);
	Dee_XDecref(oldobj);
	return 0;
}

PRIVATE NONNULL((1, 3)) int DCALL
rvec_setitem_index(RefVector *self, size_t index,
                 DeeObject *value) {
	DREF DeeObject *oldobj;
	if unlikely(index >= self->rv_length)
		return err_index_out_of_bounds((DeeObject *)self, index, self->rv_length);
	if unlikely(!RefVector_IsWritable(self))
		return err_readonly_rvec();
	Dee_Incref(value);
	RefVector_LockWrite(self);
	oldobj = self->rv_vector[index];
	self->rv_vector[index] = value;
	RefVector_LockEndWrite(self);
	Dee_XDecref(oldobj);
	return 0;
}

PRIVATE NONNULL((1)) int DCALL
rvec_bounditem_index(RefVector *self, size_t index) {
	if unlikely(index >= self->rv_length)
		return -2;
	return atomic_read(&self->rv_vector[index]) ? 1 : 0;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rvec_getitem_index_fast(RefVector *__restrict self, size_t index) {
	DREF DeeObject *result;
	ASSERT(index < self->rv_length);
	RefVector_XLockRead(self);
	result = self->rv_vector[index];
	Dee_XIncref(result);
	RefVector_XLockEndRead(self);
	return result;
}

PRIVATE WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL
rvec_nsi_xchitem(RefVector *self, size_t index, DeeObject *value) {
	DREF DeeObject *result;
	if unlikely(index >= self->rv_length) {
		err_index_out_of_bounds((DeeObject *)self, index, self->rv_length);
		goto err;
	}
	if unlikely(!RefVector_IsWritable(self)) {
		err_readonly_rvec();
		goto err;
	}
	RefVector_LockWrite(self);
	result = self->rv_vector[index];
	if unlikely(!result) {
		RefVector_LockEndWrite(self);
		err_unbound_index((DeeObject *)self, index);
		goto err;
	}
	Dee_Incref(value);
	self->rv_vector[index] = value;
	RefVector_LockEndWrite(self);
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 3)) bool DCALL
rvec_nsi_cmpdelitem(RefVector *self, size_t index,
                    DeeObject *old_value) {
	ASSERT(index < self->rv_length);
	ASSERT(RefVector_IsWritable(self));
	RefVector_LockWrite(self);
	if unlikely(self->rv_vector[index] != old_value) {
		RefVector_LockEndWrite(self);
		return false;
	}
	self->rv_vector[index] = NULL;
	RefVector_LockEndWrite(self);
	Dee_Decref(old_value);
	return true;
}

PRIVATE WUNUSED NONNULL((1, 4)) size_t DCALL
rvec_nsi_find(RefVector *self, size_t start, size_t end,
              DeeObject *keyed_search_item, DeeObject *key) {
	size_t i;
	DREF DeeObject *item;
	int temp;
	if (start > self->rv_length)
		start = self->rv_length;
	for (i = start; i < end; ++i) {
		item = rvec_getitem_index_fast(self, i);
		if (!item)
			continue; /* Unbound index */
		temp = DeeObject_CompareKeyEq(keyed_search_item, item, key);
		Dee_Decref(item);
		if (temp == 0)
			continue;
		if unlikely(temp < 0)
			goto err;
		return i;
	}
	return (size_t)-1;
err:
	return (size_t)-2;
}

PRIVATE WUNUSED NONNULL((1, 4)) size_t DCALL
rvec_nsi_rfind(RefVector *self, size_t start, size_t end,
               DeeObject *keyed_search_item, DeeObject *key) {
	size_t i;
	DREF DeeObject *item;
	int temp;
	if (start > self->rv_length)
		start = self->rv_length;
	i = end;
	while (i > start) {
		--i;
		item = rvec_getitem_index_fast(self, i);
		if (!item)
			continue; /* Unbound index */
		temp = DeeObject_CompareKeyEq(keyed_search_item, item, key);
		Dee_Decref(item);
		if (temp == 0)
			continue;
		if unlikely(temp < 0)
			goto err;
		return i;
	}
	return (size_t)-1;
err:
	return (size_t)-2;
}


PRIVATE WUNUSED NONNULL((1, 4)) int DCALL
rvec_nsi_remove(RefVector *self, size_t start, size_t end,
                DeeObject *keyed_search_item, DeeObject *key) {
	size_t i;
	DREF DeeObject *item;
	int temp;
	if (!RefVector_IsWritable(self))
		return err_readonly_rvec();
	if (start > self->rv_length)
		start = self->rv_length;
again:
	for (i = start; i < end; ++i) {
		item = rvec_getitem_index_fast(self, i);
		if (!item)
			continue; /* Unbound index */
		temp = DeeObject_CompareKeyEq(keyed_search_item, item, key);
		Dee_Decref(item);
		if (temp == 0)
			continue;
		if unlikely(temp < 0)
			goto err;
		/* Found the item. */
		if (!rvec_nsi_cmpdelitem(self, i, item))
			goto again;
		return 1;
	}
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 4)) int DCALL
rvec_nsi_rremove(RefVector *self, size_t start, size_t end,
                 DeeObject *keyed_search_item, DeeObject *key) {
	size_t i;
	DREF DeeObject *item;
	int temp;
	if (!RefVector_IsWritable(self))
		return err_readonly_rvec();
	if (start > self->rv_length)
		start = self->rv_length;
again:
	i = end;
	while (i > start) {
		--i;
		item = rvec_getitem_index_fast(self, i);
		if (!item)
			continue; /* Unbound index */
		temp = DeeObject_CompareKeyEq(keyed_search_item, item, key);
		Dee_Decref(item);
		if (temp == 0)
			continue;
		if unlikely(temp < 0)
			goto err;
		/* Found the item. */
		if (!rvec_nsi_cmpdelitem(self, i, item))
			goto again;
		return 1;
	}
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 4)) size_t DCALL
rvec_nsi_removeall(RefVector *self, size_t start, size_t end,
                   DeeObject *keyed_search_item, DeeObject *key) {
	size_t i;
	size_t result = 0;
	DREF DeeObject *item;
	int temp;
	if (!RefVector_IsWritable(self))
		return err_readonly_rvec();
	if (start > self->rv_length)
		start = self->rv_length;
again:
	for (i = start; i < end; ++i) {
		item = rvec_getitem_index_fast(self, i);
		if (!item)
			continue; /* Unbound index */
		temp = DeeObject_CompareKeyEq(keyed_search_item, item, key);
		Dee_Decref(item);
		if (temp == 0)
			continue;
		if unlikely(temp < 0)
			goto err;
		/* Found the item. */
		if (!rvec_nsi_cmpdelitem(self, i, item))
			goto again;
		++result;
		if unlikely(result == (size_t)-2)
			break; /* Prevent overflows. */
	}
	return result;
err:
	return (size_t)-1;
}

PRIVATE WUNUSED NONNULL((1, 4)) size_t DCALL
rvec_nsi_removeif(RefVector *self, size_t start, size_t end,
                  DeeObject *should_remove) {
	size_t i;
	size_t result = 0;
	int temp;
	DREF DeeObject *item, *callback_result;
	if (!RefVector_IsWritable(self))
		return err_readonly_rvec();
	if (start > self->rv_length)
		start = self->rv_length;
again:
	for (i = start; i < end; ++i) {
		item = rvec_getitem_index_fast(self, i);
		if (!item)
			continue; /* Unbound index */
		callback_result = DeeObject_Call(should_remove, 1, &item);
		Dee_Decref(item);
		if unlikely(!callback_result)
			goto err;
		temp = DeeObject_Bool(callback_result);
		Dee_Decref(callback_result);
		if (temp == 0)
			continue;
		if unlikely(temp < 0)
			goto err;
		/* Found the item. */
		if (!rvec_nsi_cmpdelitem(self, i, item))
			goto again;
		++result;
		if unlikely(result == (size_t)-2)
			break; /* Prevent overflows. */
	}
	return result;
err:
	return (size_t)-1;
}

PRIVATE NONNULL((1)) void DCALL
rvec_nsi_delitem_fast(RefVector *__restrict self, size_t index) {
	DREF DeeObject *oldobj;
	ASSERT(index < self->rv_length);
	ASSERT(RefVector_IsWritable(self));
	RefVector_LockWrite(self);
	oldobj = self->rv_vector[index];
	self->rv_vector[index] = NULL;
	RefVector_LockEndWrite(self);
	Dee_XDecref(oldobj);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
rvec_delrange_index(RefVector *__restrict self,
                    Dee_ssize_t i_begin, Dee_ssize_t i_end) {
	struct Dee_seq_range range;
	size_t i;
	if (!RefVector_IsWritable(self))
		return err_readonly_rvec();
	DeeSeqRange_Clamp(&range, i_begin, i_end, self->rv_length);
	for (i = range.sr_start; i < range.sr_end; ++i)
		rvec_nsi_delitem_fast(self, i);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
rvec_delrange_index_n(RefVector *__restrict self, Dee_ssize_t i_begin) {
#ifdef __OPTIMIZE_SIZE__
	return rvec_delrange_index(self, i_begin, SSIZE_MAX);
#else /* __OPTIMIZE_SIZE__ */
	size_t i, start;
	if (!RefVector_IsWritable(self))
		return err_readonly_rvec();
	start = DeeSeqRange_Clamp_n(i_begin, self->rv_length);
	for (i = start; i < self->rv_length; ++i)
		rvec_nsi_delitem_fast(self, i);
	return 0;
#endif /* !__OPTIMIZE_SIZE__ */
}

PRIVATE NONNULL((1, 3)) void DCALL
rvec_nsi_setitem_fast(RefVector *self, size_t index,
                      /*inherit(always)*/ DREF DeeObject *value) {
	DREF DeeObject *oldobj;
	ASSERT(index < self->rv_length);
	ASSERT(RefVector_IsWritable(self));
	RefVector_LockWrite(self);
	oldobj = self->rv_vector[index];
	self->rv_vector[index] = value;
	RefVector_LockEndWrite(self);
	Dee_XDecref(oldobj);
}

PRIVATE WUNUSED NONNULL((1, 4)) int DCALL
rvec_setrange_index(RefVector *self, Dee_ssize_t i_begin,
                    Dee_ssize_t i_end, DeeObject *values) {
	struct Dee_seq_range range;
	size_t range_size;
	size_t i, fast_length;
	DREF DeeObject *elem;
	if (!RefVector_IsWritable(self))
		return err_readonly_rvec();
	DeeSeqRange_Clamp(&range, i_begin, i_end, self->rv_length);
	range_size = range.sr_end - range.sr_start;
	fast_length = DeeFastSeq_GetSize_deprecated(values);
	if (fast_length != DEE_FASTSEQ_NOTFAST_DEPRECATED) {
		if (fast_length != range_size)
			return err_invalid_unpack_size(values, range_size, fast_length);
		for (i = range.sr_start; i < range.sr_end; ++i) {
			elem = DeeFastSeq_GetItem_deprecated(values, i - range.sr_start);
			if unlikely(!elem)
				goto err;
			rvec_nsi_setitem_fast(self, i, elem); /* Inherit reference. */
		}
	} else {
		DREF DeeObject *iterator;
		iterator = DeeObject_Iter(values); /* TODO: Use DeeObject_Foreach() */
		if unlikely(!iterator)
			goto err;
		for (i = range.sr_start; i < range.sr_end; ++i) {
			elem = DeeObject_IterNext(iterator);
			if unlikely(!ITER_ISOK(elem)) {
				if unlikely(elem == ITER_DONE) {
					err_invalid_unpack_size(values, range_size,
					                        i - range.sr_start);
				}
err_iterator:
				Dee_Decref(iterator);
				goto err;
			}
			rvec_nsi_setitem_fast(self, i, elem); /* Inherit reference. */
		}
		/* Make sure that the given iterator ends here! */
		elem = DeeObject_IterNext(iterator);
		if unlikely(elem != ITER_DONE) {
			if (elem) {
				err_invalid_unpack_iter_size(values, iterator, range_size);
				Dee_Decref(elem);
			}
			goto err_iterator;
		}
		Dee_Decref(iterator);
	}
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
rvec_setrange_index_n(RefVector *self, Dee_ssize_t start,
                      DeeObject *values) {
	return rvec_setrange_index(self, start, SSIZE_MAX, values);
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
rvec_foreach(RefVector *self, Dee_foreach_t proc, void *arg) {
	Dee_ssize_t temp, result = 0;
	size_t i;
	for (i = 0; i < self->rv_length; ++i) {
		DREF DeeObject *elem;
		RefVector_XLockRead(self);
		while ((elem = self->rv_vector[i]) == NULL) {
			++i;
			if (i >= self->rv_length) {
				RefVector_XLockEndRead(self);
				goto done;
			}
		}
		Dee_Incref(elem);
		RefVector_XLockEndRead(self);
		temp = (*proc)(arg, elem);
		Dee_Decref_unlikely(elem);
		if unlikely(temp < 0)
			goto err_temp;
		result += temp;
	}
done:
	return result;
err_temp:
	return temp;
}

PRIVATE struct type_nsi tpconst rvec_nsi = {
	/* .nsi_class   = */ TYPE_SEQX_CLASS_SEQ,
	/* .nsi_flags   = */ TYPE_SEQX_FMUTABLE,
	{
		/* .nsi_seqlike = */ {
			/* .nsi_getsize      = */ (dfunptr_t)&rvec_size,
			/* .nsi_getsize_fast = */ (dfunptr_t)&rvec_size,
			/* .nsi_getitem      = */ (dfunptr_t)&rvec_getitem_index,
			/* .nsi_delitem      = */ (dfunptr_t)&rvec_delitem_index,
			/* .nsi_setitem      = */ (dfunptr_t)&rvec_setitem_index,
			/* .nsi_getitem_fast = */ (dfunptr_t)&rvec_getitem_index_fast,
			/* .nsi_getrange     = */ (dfunptr_t)NULL,
			/* .nsi_getrange_n   = */ (dfunptr_t)NULL,
			/* .nsi_delrange     = */ (dfunptr_t)&rvec_delrange_index,
			/* .nsi_delrange_n   = */ (dfunptr_t)&rvec_delrange_index_n,
			/* .nsi_setrange     = */ (dfunptr_t)&rvec_setrange_index,
			/* .nsi_setrange_n   = */ (dfunptr_t)&rvec_setrange_index_n,
			/* .nsi_find         = */ (dfunptr_t)&rvec_nsi_find,
			/* .nsi_rfind        = */ (dfunptr_t)&rvec_nsi_rfind,
			/* .nsi_xch          = */ (dfunptr_t)&rvec_nsi_xchitem,
			/* .nsi_insert       = */ (dfunptr_t)NULL,
			/* .nsi_insertall    = */ (dfunptr_t)NULL,
			/* .nsi_insertvec    = */ (dfunptr_t)NULL,
			/* .nsi_pop          = */ (dfunptr_t)NULL,
			/* .nsi_erase        = */ (dfunptr_t)NULL,
			/* .nsi_remove       = */ (dfunptr_t)&rvec_nsi_remove,
			/* .nsi_rremove      = */ (dfunptr_t)&rvec_nsi_rremove,
			/* .nsi_removeall    = */ (dfunptr_t)&rvec_nsi_removeall,
			/* .nsi_removeif     = */ (dfunptr_t)&rvec_nsi_removeif
		}
	}
};


PRIVATE struct type_seq rvec_seq = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&rvec_iter,
	/* .tp_sizeob                     = */ NULL,
	/* .tp_contains                   = */ NULL_IF_Os((DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&rvec_contains),
	/* .tp_getitem                    = */ NULL, /* default */
	/* .tp_delitem                    = */ NULL, /* default */
	/* .tp_setitem                    = */ NULL, /* default */
	/* .tp_getrange                   = */ NULL, /* default */
	/* .tp_delrange                   = */ NULL, /* default */
	/* .tp_setrange                   = */ NULL, /* default */
	/* .tp_nsi                        = */ &rvec_nsi,
	/* .tp_foreach                    = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&rvec_foreach,
	/* .tp_foreach_pair               = */ NULL,
	/* .tp_bounditem                  = */ NULL,
	/* .tp_hasitem                    = */ NULL,
	/* .tp_size                       = */ (size_t (DCALL *)(DeeObject *__restrict))&rvec_size,
	/* .tp_size_fast                  = */ (size_t (DCALL *)(DeeObject *__restrict))&rvec_size,
	/* .tp_getitem_index              = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&rvec_getitem_index,
	/* .tp_getitem_index_fast         = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&rvec_getitem_index_fast,
	/* .tp_delitem_index              = */ (int (DCALL *)(DeeObject *, size_t))&rvec_delitem_index,
	/* .tp_setitem_index              = */ (int (DCALL *)(DeeObject *, size_t, DeeObject *))&rvec_setitem_index,
	/* .tp_bounditem_index            = */ (int (DCALL *)(DeeObject *, size_t))&rvec_bounditem_index,
	/* .tp_hasitem_index              = */ NULL, /* default */
	/* .tp_getrange_index             = */ NULL, /* default */
	/* .tp_delrange_index             = */ (int (DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t))&rvec_delrange_index,
	/* .tp_setrange_index             = */ (int (DCALL *)(DeeObject *, Dee_ssize_t, Dee_ssize_t, DeeObject *))&rvec_setrange_index,
	/* .tp_getrange_index_n           = */ NULL, /* default */
	/* .tp_delrange_index_n           = */ (int (DCALL *)(DeeObject *, Dee_ssize_t))&rvec_delrange_index_n,
	/* .tp_setrange_index_n           = */ (int (DCALL *)(DeeObject *, Dee_ssize_t, DeeObject *))&rvec_setrange_index_n,
	/* .tp_trygetitem                 = */ NULL,
	/* .tp_trygetitem_index           = */ NULL,
	/* .tp_trygetitem_string_hash     = */ NULL,
	/* .tp_getitem_string_hash        = */ NULL,
	/* .tp_delitem_string_hash        = */ NULL,
	/* .tp_setitem_string_hash        = */ NULL,
	/* .tp_bounditem_string_hash      = */ NULL,
	/* .tp_hasitem_string_hash        = */ NULL,
	/* .tp_trygetitem_string_len_hash = */ NULL,
	/* .tp_getitem_string_len_hash    = */ NULL,
	/* .tp_delitem_string_len_hash    = */ NULL,
	/* .tp_setitem_string_len_hash    = */ NULL,
	/* .tp_bounditem_string_len_hash  = */ NULL,
	/* .tp_hasitem_string_len_hash    = */ NULL,
};

PRIVATE struct type_member tpconst rvec_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &RefVectorIterator_Type),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst rvec_members[] = {
	TYPE_MEMBER_FIELD("__owner__", STRUCT_OBJECT, offsetof(RefVector, rv_owner)),
	TYPE_MEMBER_FIELD("__len__", STRUCT_CONST | STRUCT_SIZE_T, offsetof(RefVector, rv_length)),
	TYPE_MEMBER_END
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rvec_get_writable(RefVector *__restrict self) {
	return_bool(RefVector_IsWritable(self));
}

PRIVATE struct type_getset tpconst rvec_getsets[] = {
	TYPE_GETTER_F("__writable__", &rvec_get_writable, METHOD_FNOREFESCAPE, "->?Dbool"),
	TYPE_GETSET_END
};

PRIVATE WUNUSED NONNULL((1)) int DCALL
rvec_init(RefVector *__restrict self) {
	self->rv_owner = Dee_None;
	Dee_Incref(Dee_None);
	self->rv_length = 0;
	self->rv_vector = NULL;
#ifndef CONFIG_NO_THREADS
	self->rv_plock = NULL;
#else /* !CONFIG_NO_THREADS */
	self->rv_writable = false;
#endif /* CONFIG_NO_THREADS */
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
rvec_copy(RefVector *__restrict self,
          RefVector *__restrict other) {
	self->rv_owner = other->rv_owner;
	Dee_Incref(self->rv_owner);
	self->rv_length = other->rv_length;
	self->rv_vector = other->rv_vector;
#ifndef CONFIG_NO_THREADS
	self->rv_plock = other->rv_plock;
#else /* !CONFIG_NO_THREADS */
	self->rv_writable = other->rv_writable;
#endif /* CONFIG_NO_THREADS */
	return 0;
}

INTERN DeeTypeObject RefVector_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_RefVector",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&rvec_init,
				/* .tp_copy_ctor = */ (dfunptr_t)&rvec_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)NULL,
				TYPE_FIXED_ALLOCATOR(RefVector)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&rvec_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&rvec_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&rvec_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &rvec_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ rvec_getsets,
	/* .tp_members       = */ rvec_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ rvec_class_members
};

/* Construct a new reference-vector object that can be iterated
 * and used to potentially modify the elements of a given `vector'.
 * NOTE: When write-access is granted, `vector' should be `[0..1][0..length]',
 *       whereas when write-access is not possible, then the disposition of
 *       elements of `vector' doesn't matter and can either be `[0..1]' or `[1..1]'. */
PUBLIC WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeRefVector_New(DeeObject *owner, size_t length,
                 DeeObject **vector,
#ifndef CONFIG_NO_THREADS
                 Dee_atomic_rwlock_t *plock
#else /* !CONFIG_NO_THREADS */
                 bool writable
#endif /* CONFIG_NO_THREADS */
                 ) {
	DREF RefVector *result;
	ASSERT_OBJECT(owner);
	ASSERT(!length || vector);
	result = DeeObject_MALLOC(RefVector);
	if unlikely(!result)
		goto done;
	DeeObject_Init(result, &RefVector_Type);
	Dee_Incref(owner); /* Create the reference for `rv_owner' */
	result->rv_length = length;
	result->rv_vector = vector;
	result->rv_owner  = owner;
#ifndef CONFIG_NO_THREADS
	result->rv_plock = plock;
#else /* !CONFIG_NO_THREADS */
	result->rv_writable = writable;
#endif /* CONFIG_NO_THREADS */
done:
	return (DREF DeeObject *)result;
}




PRIVATE WUNUSED NONNULL((1)) int DCALL
sveciter_ctor(SharedVectorIterator *__restrict self) {
	self->si_seq = (DREF SharedVector *)DeeObject_NewDefault(&DeeSharedVector_Type);
	if unlikely(!self->si_seq)
		goto err;
	self->si_index = 0;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
sveciter_init(SharedVectorIterator *__restrict self,
              size_t argc, DeeObject *const *argv) {
	if (DeeArg_Unpack(argc, argv, "o:_SharedVectorIterator", &self->si_seq))
		goto err;
	if (DeeObject_AssertTypeExact(self->si_seq, &DeeSharedVector_Type))
		goto err;
	Dee_Incref(self->si_seq);
	self->si_index = 0;
	return 0;
err:
	return -1;
}

INTERN NONNULL((1)) void DCALL
sveciter_fini(SharedVectorIterator *__restrict self) {
	Dee_Decref(self->si_seq);
}

INTERN NONNULL((1, 2)) void DCALL
sveciter_visit(SharedVectorIterator *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(self->si_seq);
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
sveciter_next(SharedVectorIterator *__restrict self) {
	DREF DeeObject *result = ITER_DONE;
	SharedVector *vec      = self->si_seq;
	for (;;) {
		size_t index;
		SharedVector_LockRead(vec);
		index = atomic_read(&self->si_index);
		if (self->si_index >= vec->sv_length) {
			SharedVector_LockEndRead(vec);
			break;
		}
		result = vec->sv_vector[index];

		/* Acquire a reference to keep the item alive. */
		Dee_Incref(result);
		SharedVector_LockEndRead(vec);
		if (atomic_cmpxch_weak_or_write(&self->si_index, index, index + 1))
			break;

		/* If some other thread stole the index, drop their value. */
		Dee_Decref(result);
	}
	return result;
}

INTERN WUNUSED NONNULL((1)) int DCALL
sveciter_bool(SharedVectorIterator *__restrict self) {
	return (atomic_read(&self->si_index) <
	        atomic_read(&self->si_seq->sv_length));
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
sveciter_copy(SharedVectorIterator *__restrict self,
              SharedVectorIterator *__restrict other) {
	self->si_index = atomic_read(&other->si_index);
	self->si_seq = other->si_seq;
	Dee_Incref(self->si_seq);
	return 0;
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
sveciter_deep(SharedVectorIterator *__restrict self,
              SharedVectorIterator *__restrict other) {
	self->si_seq = (DREF SharedVector *)DeeObject_DeepCopy((DeeObject *)other->si_seq);
	if unlikely(self->si_seq)
		goto err;
	self->si_index = atomic_read(&other->si_index);
	return 0;
err:
	return -1;
}

PRIVATE struct type_member tpconst sveciter_members[] = {
	TYPE_MEMBER_FIELD_DOC(STR_seq, STRUCT_OBJECT, offsetof(SharedVectorIterator, si_seq), "->?Ert:SharedVector"),
	TYPE_MEMBER_FIELD("__index__", STRUCT_SIZE_T, offsetof(SharedVectorIterator, si_index)),
	TYPE_MEMBER_END
};

#define READ_INDEX(x) atomic_read(&(x)->si_index)

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
sveciter_eq(SharedVectorIterator *self,
            SharedVectorIterator *other) {
	if (DeeObject_AssertTypeExact(other, &SharedVectorIterator_Type))
		goto err;
	return_bool_(self->si_seq == other->si_seq &&
	             READ_INDEX(self) == READ_INDEX(other));
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
sveciter_ne(SharedVectorIterator *self,
            SharedVectorIterator *other) {
	if (DeeObject_AssertTypeExact(other, &SharedVectorIterator_Type))
		goto err;
	return_bool(self->si_seq != other->si_seq ||
	            READ_INDEX(self) != READ_INDEX(other));
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
sveciter_lo(SharedVectorIterator *self,
            SharedVectorIterator *other) {
	if (DeeObject_AssertTypeExact(other, &SharedVectorIterator_Type))
		goto err;
	return_bool(self->si_seq == other->si_seq
	            ? READ_INDEX(self) < READ_INDEX(other)
	            : self->si_seq < other->si_seq);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
sveciter_le(SharedVectorIterator *self,
            SharedVectorIterator *other) {
	if (DeeObject_AssertTypeExact(other, &SharedVectorIterator_Type))
		goto err;
	return_bool(self->si_seq == other->si_seq
	            ? READ_INDEX(self) <= READ_INDEX(other)
	            : self->si_seq <= other->si_seq);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
sveciter_gr(SharedVectorIterator *self,
            SharedVectorIterator *other) {
	if (DeeObject_AssertTypeExact(other, &SharedVectorIterator_Type))
		goto err;
	return_bool(self->si_seq == other->si_seq
	            ? READ_INDEX(self) > READ_INDEX(other)
	            : self->si_seq > other->si_seq);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
sveciter_ge(SharedVectorIterator *self,
            SharedVectorIterator *other) {
	if (DeeObject_AssertTypeExact(other, &SharedVectorIterator_Type))
		goto err;
	return_bool(self->si_seq == other->si_seq
	            ? READ_INDEX(self) >= READ_INDEX(other)
	            : self->si_seq >= other->si_seq);
err:
	return NULL;
}

INTERN struct type_cmp sveciter_cmp = {
	/* .tp_hash          = */ NULL,
	/* .tp_compare_eq    = */ NULL,
	/* .tp_compare       = */ NULL,
	/* .tp_trycompare_eq = */ NULL,
	/* .tp_eq            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&sveciter_eq,
	/* .tp_ne            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&sveciter_ne,
	/* .tp_lo            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&sveciter_lo,
	/* .tp_le            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&sveciter_le,
	/* .tp_gr            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&sveciter_gr,
	/* .tp_ge            = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&sveciter_ge,
};



/* This types isn't actually be needed (Sequence would emulate it using `FastNsiIterator'),
 * but since shared vectors are used by `ASM_CALL_SEQ' instructions, they get their own,
 * dedicated iterator type (which is a tiny bit faster than `FastNsiIterator') */
INTERN DeeTypeObject SharedVectorIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SharedVectorIterator",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&sveciter_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&sveciter_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&sveciter_deep,
				/* .tp_any_ctor  = */ (dfunptr_t)&sveciter_init,
				TYPE_FIXED_ALLOCATOR(SharedVectorIterator)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&sveciter_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&sveciter_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&sveciter_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &sveciter_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&sveciter_next,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ sveciter_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};

PRIVATE NONNULL((1)) void DCALL
svec_fini(SharedVector *__restrict self) {
	Dee_Decrefv(self->sv_vector, self->sv_length);
	Dee_Free((void *)self->sv_vector);
}

PRIVATE NONNULL((1, 2)) void DCALL
svec_visit(SharedVector *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visitv(self->sv_vector, self->sv_length);
}

PRIVATE WUNUSED NONNULL((1)) DREF SharedVectorIterator *DCALL
svec_iter(SharedVector *__restrict self) {
	DREF SharedVectorIterator *result;
	result = DeeObject_MALLOC(SharedVectorIterator);
	if unlikely(!result)
		goto done;
	DeeObject_Init(result, &SharedVectorIterator_Type);
	Dee_Incref(self);
	result->si_seq   = self;
	result->si_index = 0;
done:
	return result;
}

#ifndef __OPTIMIZE_SIZE__
PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
svec_contains(SharedVector *self, DeeObject *other) {
	size_t index;
	int temp;
	SharedVector_LockRead(self);
	for (index = 0; index < self->sv_length; ++index) {
		DREF DeeObject *item;
		item = self->sv_vector[index];
		Dee_Incref(item);
		SharedVector_LockEndRead(self);
		temp = DeeObject_CompareEq(other, item);
		Dee_Decref(item);
		if (temp != 0) {
			if unlikely(temp < 0)
				goto err;
			return_true;
		}
		SharedVector_LockRead(self);
	}
	SharedVector_LockEndRead(self);
	return_false;
err:
	return NULL;
}
#endif /* !__OPTIMIZE_SIZE__ */

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
svec_size(SharedVector *__restrict self) {
	ASSERT(self->sv_length != (size_t)-1);
	return atomic_read(&self->sv_length);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
svec_getitem_index(SharedVector *__restrict self, size_t index) {
	DREF DeeObject *result;
	SharedVector_LockRead(self);
	if unlikely(index >= self->sv_length) {
		size_t my_length = self->sv_length;
		SharedVector_LockEndRead(self);
		err_index_out_of_bounds((DeeObject *)self, index, my_length);
		return NULL;
	}
	result = self->sv_vector[index];
	Dee_Incref(result);
	SharedVector_LockEndRead(self);
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
svec_getitem_index_fast(SharedVector *__restrict self, size_t index) {
	DREF DeeObject *result;
	SharedVector_LockRead(self);
	/* Still to check for length==0 in case the vector got unshared. */
	if unlikely(!self->sv_length) {
		SharedVector_LockEndRead(self);
		return NULL;
	}
	ASSERT(index < self->sv_length);
	result = self->sv_vector[index];
	Dee_Incref(result);
	SharedVector_LockEndRead(self);
	return result;
}

PRIVATE WUNUSED NONNULL((1, 4)) size_t DCALL
svec_nsi_find(SharedVector *__restrict self,
              size_t start, size_t end,
              DeeObject *__restrict keyed_search_item,
              DeeObject *key) {
	size_t i = start;
	SharedVector_LockRead(self);
	for (; i < end && i < self->sv_length; ++i) {
		DREF DeeObject *item;
		int temp;
		item = self->sv_vector[i];
		Dee_Incref(item);
		SharedVector_LockEndRead(self);
		temp = DeeObject_CompareKeyEq(keyed_search_item, item, key);
		Dee_Decref(item);
		if (temp != 0) {
			if unlikely(temp < 0)
				goto err;
			return i;
		}
		SharedVector_LockRead(self);
	}
	SharedVector_LockEndRead(self);
	return (size_t)-1;
err:
	return (size_t)-2;
}

PRIVATE WUNUSED NONNULL((1, 4)) size_t DCALL
svec_nsi_rfind(SharedVector *__restrict self,
               size_t start, size_t end,
               DeeObject *__restrict keyed_search_item,
               DeeObject *key) {
	size_t i = end;
	SharedVector_LockRead(self);
	for (;;) {
		DREF DeeObject *item;
		int temp;
		if (i > self->sv_length)
			i = self->sv_length;
		if (i <= start)
			break;
		--i;
		item = self->sv_vector[i];
		Dee_Incref(item);
		SharedVector_LockEndRead(self);
		temp = DeeObject_CompareKeyEq(keyed_search_item, item, key);
		Dee_Decref(item);
		if (temp != 0) {
			if unlikely(temp < 0)
				goto err;
			return i;
		}
		SharedVector_LockRead(self);
	}
	SharedVector_LockEndRead(self);
	return (size_t)-1;
err:
	return (size_t)-2;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
svec_foreach(SharedVector *self, Dee_foreach_t proc, void *arg) {
	size_t i;
	Dee_ssize_t temp, result = 0;
	SharedVector_LockRead(self);
	for (i = 0; i < self->sv_length; ++i) {
		DREF DeeObject *list_elem;
		list_elem = self->sv_vector[i];
		Dee_Incref(list_elem);
		SharedVector_LockEndRead(self);
		temp = (*proc)(arg, list_elem);
		Dee_Decref_unlikely(list_elem);
		if unlikely(temp < 0)
			goto err;
		result += temp;
		SharedVector_LockRead(self);
	}
	SharedVector_LockEndRead(self);
	return result;
err:
	return temp;
}


PRIVATE struct type_nsi tpconst svec_nsi = {
	/* .nsi_class   = */ TYPE_SEQX_CLASS_SEQ,
	/* .nsi_flags   = */ TYPE_SEQX_FNORMAL,
	{
		/* .nsi_seqlike = */ {
			/* .nsi_getsize      = */ (dfunptr_t)&svec_size,
			/* .nsi_getsize_fast = */ (dfunptr_t)&svec_size,
			/* .nsi_getitem      = */ (dfunptr_t)&svec_getitem_index,
			/* .nsi_delitem      = */ (dfunptr_t)NULL,
			/* .nsi_setitem      = */ (dfunptr_t)NULL,
			/* .nsi_getitem_fast = */ (dfunptr_t)&svec_getitem_index_fast,
			/* .nsi_getrange     = */ (dfunptr_t)NULL,
			/* .nsi_getrange_n   = */ (dfunptr_t)NULL,
			/* .nsi_delrange     = */ (dfunptr_t)NULL,
			/* .nsi_delrange_n   = */ (dfunptr_t)NULL,
			/* .nsi_setrange     = */ (dfunptr_t)NULL,
			/* .nsi_setrange_n   = */ (dfunptr_t)NULL,
			/* .nsi_find         = */ (dfunptr_t)&svec_nsi_find,
			/* .nsi_rfind        = */ (dfunptr_t)&svec_nsi_rfind,
			/* .nsi_xch          = */ (dfunptr_t)NULL,
			/* .nsi_insert       = */ (dfunptr_t)NULL,
			/* .nsi_insertall    = */ (dfunptr_t)NULL,
			/* .nsi_insertvec    = */ (dfunptr_t)NULL,
			/* .nsi_pop          = */ (dfunptr_t)NULL,
			/* .nsi_erase        = */ (dfunptr_t)NULL,
			/* .nsi_remove       = */ (dfunptr_t)NULL,
			/* .nsi_rremove      = */ (dfunptr_t)NULL,
			/* .nsi_removeall    = */ (dfunptr_t)NULL,
			/* .nsi_removeif     = */ (dfunptr_t)NULL
		}
	}
};

PRIVATE struct type_seq svec_seq = {
	/* .tp_iter                       = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&svec_iter,
	/* .tp_sizeob                     = */ NULL,
	/* .tp_contains                   = */ NULL_IF_Os((DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&svec_contains),
	/* .tp_getitem                    = */ NULL,
	/* .tp_delitem                    = */ NULL,
	/* .tp_setitem                    = */ NULL,
	/* .tp_getrange                   = */ NULL,
	/* .tp_delrange                   = */ NULL,
	/* .tp_setrange                   = */ NULL,
	/* .tp_nsi                        = */ &svec_nsi,
	/* .tp_foreach                    = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_foreach_t, void *))&svec_foreach,
	/* .tp_foreach_pair               = */ NULL,
	/* .tp_bounditem                  = */ NULL,
	/* .tp_hasitem                    = */ NULL,
	/* .tp_size                       = */ (size_t (DCALL *)(DeeObject *__restrict))&svec_size,
	/* .tp_size_fast                  = */ (size_t (DCALL *)(DeeObject *__restrict))&svec_size,
	/* .tp_getitem_index              = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&svec_getitem_index,
	/* .tp_getitem_index_fast         = */ (DREF DeeObject *(DCALL *)(DeeObject *, size_t))&svec_getitem_index_fast,
	/* .tp_delitem_index              = */ NULL,
	/* .tp_setitem_index              = */ NULL,
	/* .tp_bounditem_index            = */ NULL,
	/* .tp_hasitem_index              = */ NULL,
	/* .tp_getrange_index             = */ NULL,
	/* .tp_delrange_index             = */ NULL,
	/* .tp_setrange_index             = */ NULL,
	/* .tp_getrange_index_n           = */ NULL,
	/* .tp_delrange_index_n           = */ NULL,
	/* .tp_setrange_index_n           = */ NULL,
	/* .tp_trygetitem                 = */ NULL,
	/* .tp_trygetitem_index           = */ NULL,
	/* .tp_trygetitem_string_hash     = */ NULL,
	/* .tp_getitem_string_hash        = */ NULL,
	/* .tp_delitem_string_hash        = */ NULL,
	/* .tp_setitem_string_hash        = */ NULL,
	/* .tp_bounditem_string_hash      = */ NULL,
	/* .tp_hasitem_string_hash        = */ NULL,
	/* .tp_trygetitem_string_len_hash = */ NULL,
	/* .tp_getitem_string_len_hash    = */ NULL,
	/* .tp_delitem_string_len_hash    = */ NULL,
	/* .tp_setitem_string_len_hash    = */ NULL,
	/* .tp_bounditem_string_len_hash  = */ NULL,
	/* .tp_hasitem_string_len_hash    = */ NULL,
};

PRIVATE struct type_getset tpconst svec_getsets[] = {
	TYPE_GETTER("frozen", &DeeObject_NewRef, "->?."),
	TYPE_GETSET_END
};

PRIVATE struct type_member tpconst svec_class_members[] = {
	TYPE_MEMBER_CONST(STR_Iterator, &SharedVectorIterator_Type),
	TYPE_MEMBER_CONST("Frozen", &DeeSharedVector_Type),
	TYPE_MEMBER_END
};

PRIVATE WUNUSED NONNULL((1)) int DCALL
svec_ctor(SharedVector *__restrict self) {
	Dee_atomic_rwlock_init(&self->sv_lock);
	self->sv_length = 0;
	self->sv_vector = NULL;
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
svec_copy(SharedVector *__restrict self,
          SharedVector *__restrict other) {
again:
	SharedVector_LockRead(other);
	self->sv_length = other->sv_length;
	self->sv_vector = (DREF DeeObject **)Dee_TryMallocc(self->sv_length,
	                                                    sizeof(DREF DeeObject *));
	if unlikely(!self->sv_vector) {
		SharedVector_LockEndRead(other);
		if (Dee_CollectMemory(self->sv_length * sizeof(DREF DeeObject *)))
			goto again;
		goto err;
	}
	Dee_Movrefv((DREF DeeObject **)self->sv_vector,
	            other->sv_vector, self->sv_length);
	SharedVector_LockEndRead(other);
	Dee_atomic_rwlock_init(&self->sv_lock);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
svec_deep(SharedVector *__restrict self,
          SharedVector *__restrict other) {
	size_t i;
	if unlikely(svec_copy(self, other))
		goto err;
	for (i = 0; i < self->sv_length; ++i) {
		if unlikely(DeeObject_InplaceDeepCopy((DREF DeeObject **)&self->sv_vector[i]))
			goto err_r;
	}
	return 0;
err_r:
	Dee_Decrefv(self->sv_vector, self->sv_length);
	Dee_Free((void *)self->sv_vector);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
svec_bool(SharedVector *__restrict self) {
	return atomic_read(&self->sv_length) != 0;
}

PUBLIC DeeTypeObject DeeSharedVector_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_SharedVector",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)&svec_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&svec_copy,
				/* .tp_deep_ctor = */ (dfunptr_t)&svec_deep,
				/* .tp_any_ctor  = */ (dfunptr_t)NULL,
				TYPE_FIXED_ALLOCATOR(SharedVector)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&svec_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&svec_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&svec_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &svec_seq,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ svec_getsets,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ svec_class_members
};


/* Create a new shared vector that will inherit elements
 * from the given vector once `DeeSharedVector_Decref()' is called.
 * NOTE: This function can implicitly inherit a reference to each item of the
 *       given vector, though does not actually inherit the vector itself:
 *       - DeeSharedVector_Decref:            The `vector' arg here is `DREF DeeObject *const *'
 *       - DeeSharedVector_DecrefNoGiftItems: The `vector' arg here is `DeeObject *const *'
 * NOTE: The returned object cannot be used to change out the elements
 *       of the given `vector', meaning that _it_ can still be [const] */
PUBLIC WUNUSED DREF DeeObject *DCALL
DeeSharedVector_NewShared(size_t length, DREF DeeObject *const *vector) {
	DREF SharedVector *result;
	result = DeeObject_MALLOC(SharedVector);
	if unlikely(!result)
		goto done;
	DeeObject_Init(result, &DeeSharedVector_Type);
	Dee_atomic_rwlock_init(&result->sv_lock);
	result->sv_length = length;
	result->sv_vector = vector;
done:
	return (DREF DeeObject *)result;
}

/* Check if the reference counter of `self' is 1. When it is,
 * simply destroy the shared vector without freeing `sv_vector',
 * but still decref() all contained objects.
 * Otherwise, try to allocate a new vector with a length of `sv_length'.
 * If doing so fails, don't raise an error but replace `sv_vector' with
 * `NULL' and `sv_length' with `0' before decref()-ing all elements
 * that that pair of members used to refer to.
 * If allocation does succeed, memcpy() all objects contained in
 * the original vector into the dynamically allocated one, thus
 * transferring ownership to that vector before writing it back
 * to the SharedVector object.
 * >> In the end, this behavior is required to implement a fast,
 *    general-purpose sequence type that can be used to implement
 *    the `ASM_CALL_SEQ' opcode, as generated for brace-initializers.
 * NOTE: During decref(), objects are destroyed in reverse order,
 *       mirroring the behavior of adjstack/pop instructions. */
PUBLIC NONNULL((1)) void DCALL
DeeSharedVector_Decref(DREF DeeObject *__restrict self) {
	size_t length;
	DREF DeeObject *const *vector;
	DREF DeeObject **vector_copy;
	SharedVector *me = (SharedVector *)self;
	ASSERT_OBJECT_TYPE_EXACT(me, &DeeSharedVector_Type);
	if (!DeeObject_IsShared(me)) {
		/* Simple case: The vector isn't being shared. */
		Dee_Decrefv(me->sv_vector, me->sv_length);
		Dee_DecrefNokill(&DeeSharedVector_Type);
		DeeObject_FreeTracker((DeeObject *)me);
		DeeObject_Free(me);
		return;
	}

	/* Difficult case: must duplicate the vector. */
	SharedVector_LockWrite(me);
	vector_copy = (DREF DeeObject **)Dee_TryMallocc(me->sv_length,
	                                                sizeof(DREF DeeObject *));
	if unlikely(!vector_copy)
		goto err_cannot_inherit;

	/* Simply copy all the elements, transferring
	 * all the references that they represent. */
	vector_copy = (DREF DeeObject **)memcpyc(vector_copy, me->sv_vector,
	                                         me->sv_length, sizeof(DREF DeeObject *));

	/* Give the SharedVector its very own copy
	 * which it will take to its grave. */
	me->sv_vector = vector_copy;
	SharedVector_LockEndWrite(me);
	Dee_Decref(me);
	return;

err_cannot_inherit:
	/* Special case: failed to create a copy that the vector may call its own. */
	vector = me->sv_vector;
	length = me->sv_length;

	/* Override with an empty vector. */
	me->sv_vector = NULL;
	me->sv_length = 0;
	SharedVector_LockEndWrite(me);

	/* Destroy the items that the caller wanted the vector to inherit. */
	Dee_Decrefv(vector, length);
	Dee_Decref(me);
}

/* Same as `DeeSharedVector_Decref()', but should be used if the caller
 * does *not* want to gift the vector references to all of its items. */
PUBLIC NONNULL((1)) void DCALL
DeeSharedVector_DecrefNoGiftItems(DREF DeeObject *__restrict self) {
	DREF DeeObject **vector_copy;
	SharedVector *me = (SharedVector *)self;
	ASSERT_OBJECT_TYPE_EXACT(me, &DeeSharedVector_Type);
	if (!DeeObject_IsShared(me)) {
		/* Simple case: The vector isn't being shared. */
		Dee_DecrefNokill(&DeeSharedVector_Type);
		DeeObject_FreeTracker((DeeObject *)me);
		DeeObject_Free(me);
		return;
	}

	/* Difficult case: must duplicate the vector. */
	SharedVector_LockWrite(me);
	vector_copy = (DREF DeeObject **)Dee_TryMallocc(me->sv_length,
	                                                sizeof(DREF DeeObject *));
	if unlikely(!vector_copy)
		goto err_cannot_inherit;

	/* Copy all the elements, but also incref them at the same time. */
	vector_copy = Dee_Movrefv(vector_copy, me->sv_vector, me->sv_length);

	/* Give the SharedVector its very own copy
	 * which it will take to its grave. */
	me->sv_vector = vector_copy;
	SharedVector_LockEndWrite(me);
	Dee_Decref(me);
	return;

err_cannot_inherit:
	/* Override with an empty vector. */
	me->sv_vector = NULL;
	me->sv_length = 0;
	SharedVector_LockEndWrite(me);
	Dee_Decref(me);
}



DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_SVEC_C */
