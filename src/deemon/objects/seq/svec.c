/* Copyright (c) 2019 Griefer@Work                                            *
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
 *    in a product, an acknowledgement in the product documentation would be  *
 *    appreciated but is not required.                                        *
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
#include <deemon/util/string.h>

#include <hybrid/atomic.h>

#include "../../runtime/runtime_error.h"

DECL_BEGIN

#ifdef CONFIG_NO_THREADS
#define RVI_GETPOS(x)            ((x)->rvi_pos)
#else /* CONFIG_NO_THREADS */
#define RVI_GETPOS(x) ATOMIC_READ((x)->rvi_pos)
#endif /* !CONFIG_NO_THREADS */

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
rveciter_copy(RefVectorIterator *__restrict self,
              RefVectorIterator *__restrict other) {
	self->rvi_vector = other->rvi_vector;
	self->rvi_pos    = RVI_GETPOS(other);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
rveciter_ctor(RefVectorIterator *__restrict self,
              size_t argc, DeeObject **argv) {
	if (DeeArg_Unpack(argc, argv, "o:_RefVectorIterator", &self->rvi_vector) ||
	    DeeObject_AssertTypeExact((DeeObject *)self->rvi_vector, &RefVector_Type))
		return -1;
	Dee_Incref(self->rvi_vector);
	self->rvi_pos = self->rvi_vector->rv_vector;
	return 0;
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
	DREF DeeObject **presult, *result;
	RefVector *vector = self->rvi_vector;
	for (;;) {
#ifdef CONFIG_NO_THREADS
		presult = self->rvi_pos;
		if (presult >= vector->rv_vector + vector->rv_length)
			return ITER_DONE;
		++self->rvi_pos;
#else /* CONFIG_NO_THREADS */
		do {
			presult = ATOMIC_READ(self->rvi_pos);
			if (presult >= vector->rv_vector + vector->rv_length)
				return ITER_DONE;
		} while (!ATOMIC_CMPXCH_WEAK(self->rvi_pos, presult, presult + 1));
#endif /* !CONFIG_NO_THREADS */
#ifndef CONFIG_NO_THREADS
		if (vector->rv_plock)
			rwlock_read(vector->rv_plock);
#endif /* !CONFIG_NO_THREADS */
		result = *presult;
		Dee_XIncref(result);
#ifndef CONFIG_NO_THREADS
		if (vector->rv_plock)
			rwlock_endread(vector->rv_plock);
#endif /* !CONFIG_NO_THREADS */
		/* Skip NULL entires. */
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
	if (DeeObject_AssertTypeExact((DeeObject *)other, &RefVectorIterator_Type))
		return NULL;
	return_bool_(self->rvi_vector == other->rvi_vector &&
	             RVI_GETPOS(self) == RVI_GETPOS(other));
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
rveciter_ne(RefVectorIterator *self,
            RefVectorIterator *other) {
	if (DeeObject_AssertTypeExact((DeeObject *)other, &RefVectorIterator_Type))
		return NULL;
	return_bool_(self->rvi_vector != other->rvi_vector ||
	             RVI_GETPOS(self) != RVI_GETPOS(other));
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
rveciter_lo(RefVectorIterator *self,
            RefVectorIterator *other) {
	if (DeeObject_AssertTypeExact((DeeObject *)other, &RefVectorIterator_Type))
		return NULL;
	return_bool_(self->rvi_vector == other->rvi_vector
	             ? RVI_GETPOS(self) < RVI_GETPOS(other)
	             : self->rvi_vector < other->rvi_vector);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
rveciter_le(RefVectorIterator *self,
            RefVectorIterator *other) {
	if (DeeObject_AssertTypeExact((DeeObject *)other, &RefVectorIterator_Type))
		return NULL;
	return_bool_(self->rvi_vector == other->rvi_vector
	             ? RVI_GETPOS(self) <= RVI_GETPOS(other)
	             : self->rvi_vector <= other->rvi_vector);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
rveciter_gr(RefVectorIterator *self,
            RefVectorIterator *other) {
	if (DeeObject_AssertTypeExact((DeeObject *)other, &RefVectorIterator_Type))
		return NULL;
	return_bool_(self->rvi_vector == other->rvi_vector
	             ? RVI_GETPOS(self) > RVI_GETPOS(other)
	             : self->rvi_vector > other->rvi_vector);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
rveciter_ge(RefVectorIterator *self,
            RefVectorIterator *other) {
	if (DeeObject_AssertTypeExact((DeeObject *)other, &RefVectorIterator_Type))
		return NULL;
	return_bool_(self->rvi_vector == other->rvi_vector
	             ? RVI_GETPOS(self) >= RVI_GETPOS(other)
	             : self->rvi_vector >= other->rvi_vector);
}

PRIVATE struct type_cmp rveciter_cmp = {
	/* .tp_hash = */ NULL,
	/* .tp_eq   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&rveciter_eq,
	/* .tp_ne   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&rveciter_ne,
	/* .tp_lo   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&rveciter_lo,
	/* .tp_le   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&rveciter_le,
	/* .tp_gr   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&rveciter_gr,
	/* .tp_ge   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&rveciter_ge,
};

PRIVATE struct type_member rveciter_members[] = {
	TYPE_MEMBER_FIELD_DOC("seq", STRUCT_OBJECT, offsetof(RefVectorIterator, rvi_vector), "->?Ert:RefVector"),
	TYPE_MEMBER_END
};

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
				/* .tp_ctor      = */ NULL,
				/* .tp_copy_ctor = */ &rveciter_copy,
				/* .tp_deep_ctor = */ NULL,
				/* .tp_any_ctor  = */ &rveciter_ctor,
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

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rvec_size(RefVector *__restrict self) {
	return DeeInt_NewSize(self->rv_length);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
rvec_contains(RefVector *self,
              DeeObject *other) {
	size_t index;
	int temp;
	for (index = 0; index < self->rv_length; ++index) {
		DREF DeeObject *item;
#ifndef CONFIG_NO_THREADS
		if (self->rv_plock)
			rwlock_read(self->rv_plock);
#endif /* !CONFIG_NO_THREADS */
		item = self->rv_vector[index];
		Dee_XIncref(item);
#ifndef CONFIG_NO_THREADS
		if (self->rv_plock)
			rwlock_endread(self->rv_plock);
#endif /* !CONFIG_NO_THREADS */
		if (!item)
			continue;
		temp = DeeObject_CompareEq(other, item);
		Dee_Decref(item);
		if (temp != 0) {
			if unlikely(temp < 0)
				return NULL;
			return_true;
		}
	}
	return_false;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
rvec_getitem(RefVector *self,
             DeeObject *index_ob) {
	size_t index;
	DREF DeeObject *result;
	if (DeeObject_AsSize(index_ob, &index))
		return NULL;
	if unlikely(index >= self->rv_length) {
		err_index_out_of_bounds((DeeObject *)self,
		                        index,
		                        self->rv_length);
		goto err;
	}
#ifndef CONFIG_NO_THREADS
	if (self->rv_plock)
		rwlock_read(self->rv_plock);
#endif /* !CONFIG_NO_THREADS */
	result = self->rv_vector[index];
	if unlikely(!result) {
#ifndef CONFIG_NO_THREADS
		if (self->rv_plock)
			rwlock_endread(self->rv_plock);
#endif /* !CONFIG_NO_THREADS */
		err_unbound_index((DeeObject *)self, index);
		goto err;
	}
	Dee_Incref(result);
#ifndef CONFIG_NO_THREADS
	if (self->rv_plock)
		rwlock_endread(self->rv_plock);
#endif /* !CONFIG_NO_THREADS */
	return result;
err:
	return NULL;
}

PRIVATE ATTR_COLD int DCALL err_readonly_rvec(void) {
	return DeeError_Throwf(&DeeError_ValueError,
	                       "Reference vector is not writable");
}


PRIVATE int DCALL
rvec_setitem(RefVector *__restrict self,
             DeeObject *__restrict index_ob,
             DeeObject *value) {
	size_t index;
	DREF DeeObject *old_item;
#ifndef CONFIG_NO_THREADS
	if (!self->rv_plock)
#else /* !CONFIG_NO_THREADS */
	if (!self->rv_writable)
#endif /* CONFIG_NO_THREADS */
	{
		err_readonly_rvec();
		goto err;
	}
	if (DeeObject_AsSize(index_ob, &index))
		return -1;
	if unlikely(index >= self->rv_length) {
		err_index_out_of_bounds((DeeObject *)self,
		                        index,
		                        self->rv_length);
		goto err;
	}
	Dee_XIncref(value); /* Value may be NULL for the delitem callback. */
#ifndef CONFIG_NO_THREADS
	rwlock_write(self->rv_plock);
#endif /* !CONFIG_NO_THREADS */
	old_item               = self->rv_vector[index];
	self->rv_vector[index] = value;
#ifndef CONFIG_NO_THREADS
	rwlock_endwrite(self->rv_plock);
#endif /* !CONFIG_NO_THREADS */
	Dee_XDecref(old_item);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
rvec_delitem(RefVector *__restrict self,
             DeeObject *__restrict index_ob) {
	return rvec_setitem(self, index_ob, NULL);
}


PRIVATE WUNUSED NONNULL((1)) size_t DCALL
rvec_nsi_getsize(RefVector *__restrict self) {
	ASSERT(self->rv_length != (size_t)-1);
	return self->rv_length;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rvec_nsi_getitem(RefVector *__restrict self, size_t index) {
	DREF DeeObject *result;
	if unlikely(index >= self->rv_length) {
		err_index_out_of_bounds((DeeObject *)self, index, self->rv_length);
		return NULL;
	}
#ifndef CONFIG_NO_THREADS
	if (self->rv_plock)
		rwlock_read(self->rv_plock);
#endif /* !CONFIG_NO_THREADS */
	result = self->rv_vector[index];
	if unlikely(!result) {
#ifndef CONFIG_NO_THREADS
		if (self->rv_plock)
			rwlock_endread(self->rv_plock);
#endif /* !CONFIG_NO_THREADS */
		err_unbound_index((DeeObject *)self, index);
		return NULL;
	}
	Dee_Incref(result);
#ifndef CONFIG_NO_THREADS
	if (self->rv_plock)
		rwlock_endread(self->rv_plock);
#endif /* !CONFIG_NO_THREADS */
	return result;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
rvec_nsi_delitem(RefVector *__restrict self, size_t index) {
	DREF DeeObject *oldobj;
	if unlikely(index >= self->rv_length)
		return err_index_out_of_bounds((DeeObject *)self, index, self->rv_length);
#ifndef CONFIG_NO_THREADS
	if (self->rv_plock)
		rwlock_write(self->rv_plock);
	else
#else /* !CONFIG_NO_THREADS */
	if (!self->rv_writable)
#endif /* CONFIG_NO_THREADS */
	{
		return err_readonly_rvec();
	}
	oldobj = self->rv_vector[index];
#ifdef CONFIG_ERROR_DELETE_UNBOUND
	if unlikely(!oldobj) {
#ifndef CONFIG_NO_THREADS
		rwlock_endwrite(self->rv_plock);
#endif /* !CONFIG_NO_THREADS */
		return err_unbound_index((DeeObject *)self, index);
	}
#endif /* CONFIG_ERROR_DELETE_UNBOUND */
	self->rv_vector[index] = NULL;
#ifndef CONFIG_NO_THREADS
	rwlock_endwrite(self->rv_plock);
#endif /* !CONFIG_NO_THREADS */
#ifdef CONFIG_ERROR_DELETE_UNBOUND
	Dee_Decref(oldobj);
#else /* CONFIG_ERROR_DELETE_UNBOUND */
	Dee_XDecref(oldobj);
#endif /* !CONFIG_ERROR_DELETE_UNBOUND */
	return 0;
}

PRIVATE NONNULL((1)) void DCALL
rvec_nsi_delitem_fast(RefVector *__restrict self, size_t index) {
	DREF DeeObject *oldobj;
	ASSERT(index < self->rv_length);
#ifndef CONFIG_NO_THREADS
	ASSERT(self->rv_plock);
	rwlock_write(self->rv_plock);
#else /* !CONFIG_NO_THREADS */
	ASSERT(self->rv_writable);
#endif /* CONFIG_NO_THREADS */
	oldobj                 = self->rv_vector[index];
	self->rv_vector[index] = NULL;
#ifndef CONFIG_NO_THREADS
	rwlock_endwrite(self->rv_plock);
#endif /* !CONFIG_NO_THREADS */
	Dee_XDecref(oldobj);
}

PRIVATE void DCALL
rvec_nsi_setitem_fast(RefVector *__restrict self, size_t index,
                      /*inherit(always)*/ DREF DeeObject *__restrict value) {
	DREF DeeObject *oldobj;
	ASSERT(index < self->rv_length);
#ifndef CONFIG_NO_THREADS
	ASSERT(self->rv_plock);
	rwlock_write(self->rv_plock);
#else /* !CONFIG_NO_THREADS */
	ASSERT(self->rv_writable);
#endif /* CONFIG_NO_THREADS */
	oldobj                 = self->rv_vector[index];
	self->rv_vector[index] = value;
#ifndef CONFIG_NO_THREADS
	rwlock_endwrite(self->rv_plock);
#endif /* !CONFIG_NO_THREADS */
	Dee_XDecref(oldobj);
}

PRIVATE int DCALL
rvec_nsi_setitem(RefVector *__restrict self, size_t index,
                 DeeObject *__restrict value) {
	DREF DeeObject *oldobj;
	if unlikely(index >= self->rv_length)
		return err_index_out_of_bounds((DeeObject *)self, index, self->rv_length);
#ifndef CONFIG_NO_THREADS
	if (self->rv_plock) {
		rwlock_write(self->rv_plock);
	} else
#else /* !CONFIG_NO_THREADS */
	if (!self->rv_writable)
#endif /* CONFIG_NO_THREADS */
	{
		return err_readonly_rvec();
	}
	Dee_Incref(value);
	oldobj                 = self->rv_vector[index];
	self->rv_vector[index] = value;
#ifndef CONFIG_NO_THREADS
	rwlock_endwrite(self->rv_plock);
#endif /* !CONFIG_NO_THREADS */
	Dee_XDecref(oldobj);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rvec_nsi_getitem_fast(RefVector *__restrict self, size_t index) {
	DREF DeeObject *result;
	ASSERT(index < self->rv_length);
#ifndef CONFIG_NO_THREADS
	if (self->rv_plock)
		rwlock_read(self->rv_plock);
#endif /* !CONFIG_NO_THREADS */
	result = self->rv_vector[index];
	Dee_XIncref(result);
#ifndef CONFIG_NO_THREADS
	if (self->rv_plock)
		rwlock_endread(self->rv_plock);
#endif /* !CONFIG_NO_THREADS */
	return result;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
rvec_nsi_xchitem(RefVector *__restrict self, size_t index,
                 DeeObject *__restrict value) {
	DREF DeeObject *result;
	if unlikely(index >= self->rv_length) {
		err_index_out_of_bounds((DeeObject *)self, index, self->rv_length);
		goto err;
	}
#ifndef CONFIG_NO_THREADS
	if (self->rv_plock) {
		rwlock_write(self->rv_plock);
	} else
#else /* !CONFIG_NO_THREADS */
	if (!self->rv_writable)
#endif /* CONFIG_NO_THREADS */
	{
		err_readonly_rvec();
		goto err;
	}
	result = self->rv_vector[index];
	if unlikely(!result) {
#ifndef CONFIG_NO_THREADS
		rwlock_endwrite(self->rv_plock);
#endif /* !CONFIG_NO_THREADS */
		err_unbound_index((DeeObject *)self, index);
		goto err;
	}
	Dee_Incref(value);
	self->rv_vector[index] = value;
#ifndef CONFIG_NO_THREADS
	rwlock_endwrite(self->rv_plock);
#endif /* !CONFIG_NO_THREADS */
	return result;
err:
	return NULL;
}

PRIVATE bool DCALL
rvec_nsi_cmpdelitem(RefVector *__restrict self, size_t index,
                    DeeObject *__restrict old_value) {
	ASSERT(index < self->rv_length);
#ifndef CONFIG_NO_THREADS
	ASSERT(self->rv_plock);
	rwlock_write(self->rv_plock);
#else /* !CONFIG_NO_THREADS */
	ASSERT(self->rv_writable);
#endif /* CONFIG_NO_THREADS */
	if unlikely(self->rv_vector[index] != old_value) {
#ifndef CONFIG_NO_THREADS
		rwlock_endwrite(self->rv_plock);
#endif /* !CONFIG_NO_THREADS */
		return false;
	}
	self->rv_vector[index] = NULL;
#ifndef CONFIG_NO_THREADS
	rwlock_endwrite(self->rv_plock);
#endif /* !CONFIG_NO_THREADS */
	Dee_Decref(old_value);
	return true;
}

PRIVATE size_t DCALL
rvec_nsi_find(RefVector *__restrict self, size_t start, size_t end,
              DeeObject *__restrict keyed_search_item, DeeObject *key) {
	size_t i;
	DREF DeeObject *item;
	int temp;
	if (start > self->rv_length)
		start = self->rv_length;
	for (i = start; i < end; ++i) {
		item = rvec_nsi_getitem_fast(self, i);
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

PRIVATE size_t DCALL
rvec_nsi_rfind(RefVector *__restrict self, size_t start, size_t end,
               DeeObject *__restrict keyed_search_item, DeeObject *key) {
	size_t i;
	DREF DeeObject *item;
	int temp;
	if (start > self->rv_length)
		start = self->rv_length;
	i = end;
	while (i > start) {
		--i;
		item = rvec_nsi_getitem_fast(self, i);
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


PRIVATE int DCALL
rvec_nsi_remove(RefVector *__restrict self, size_t start, size_t end,
                DeeObject *__restrict keyed_search_item, DeeObject *key) {
	size_t i;
	DREF DeeObject *item;
	int temp;
	if (!RefVector_IsWritable(self))
		return err_readonly_rvec();
	if (start > self->rv_length)
		start = self->rv_length;
again:
	for (i = start; i < end; ++i) {
		item = rvec_nsi_getitem_fast(self, i);
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

PRIVATE int DCALL
rvec_nsi_rremove(RefVector *__restrict self, size_t start, size_t end,
                 DeeObject *__restrict keyed_search_item, DeeObject *key) {
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
		item = rvec_nsi_getitem_fast(self, i);
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

PRIVATE size_t DCALL
rvec_nsi_removeall(RefVector *__restrict self,
                   size_t start, size_t end,
                   DeeObject *__restrict keyed_search_item,
                   DeeObject *key) {
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
		item = rvec_nsi_getitem_fast(self, i);
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

PRIVATE size_t DCALL
rvec_nsi_removeif(RefVector *__restrict self,
                  size_t start, size_t end,
                  DeeObject *__restrict should_remove) {
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
		item = rvec_nsi_getitem_fast(self, i);
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

PRIVATE WUNUSED NONNULL((1)) int DCALL
rvec_nsi_delrange(RefVector *__restrict self,
                  dssize_t start, dssize_t end) {
	size_t i;
	if (!RefVector_IsWritable(self))
		return err_readonly_rvec();
	if (start < 0)
		start += self->rv_length;
	if (end < 0)
		end += self->rv_length;
	if ((size_t)end > self->rv_length)
		end = (dssize_t)self->rv_length;
	for (i = (size_t)start; i < (size_t)end; ++i)
		rvec_nsi_delitem_fast(self, i);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
rvec_nsi_delrange_n(RefVector *__restrict self,
                    dssize_t start) {
	size_t i, end = self->rv_length;
	if (!RefVector_IsWritable(self))
		return err_readonly_rvec();
	if (start < 0)
		start += end;
	for (i = (size_t)start; i < end; ++i)
		rvec_nsi_delitem_fast(self, i);
	return 0;
}

PRIVATE int DCALL
rvec_nsi_setrange(RefVector *__restrict self,
                  dssize_t start, dssize_t end,
                  DeeObject *__restrict values) {
	size_t i, fast_length;
	DREF DeeObject *elem;
	if (DeeNone_Check(values))
		return rvec_nsi_delrange(self, start, end);
	if (!RefVector_IsWritable(self))
		return err_readonly_rvec();
	if (start < 0)
		start += self->rv_length;
	if (end < 0)
		end += self->rv_length;
	if ((size_t)end > self->rv_length)
		end = (dssize_t)self->rv_length;
	fast_length = DeeFastSeq_GetSize(values);
	if (fast_length != DEE_FASTSEQ_NOTFAST) {
		if (fast_length != ((size_t)end - (size_t)start)) {
			return err_invalid_unpack_size(values,
			                               (size_t)end - (size_t)start,
			                               fast_length);
		}
		for (i = (size_t)start; i < (size_t)end; ++i) {
			elem = DeeFastSeq_GetItem(values, i - (size_t)start);
			if unlikely(!elem)
				goto err;
			rvec_nsi_setitem_fast(self, i, elem); /* Inherit reference. */
		}
	} else {
		DREF DeeObject *iterator;
		iterator = DeeObject_IterSelf(values);
		if unlikely(!iterator)
			goto err;
		for (i = (size_t)start; i < (size_t)end; ++i) {
			elem = DeeObject_IterNext(iterator);
			if unlikely(!ITER_ISOK(elem)) {
				if unlikely(elem == ITER_DONE) {
					err_invalid_unpack_size(values,
					                        (size_t)end - (size_t)start,
					                        i - (size_t)start);
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
				err_invalid_unpack_iter_size(values, iterator, (size_t)end - (size_t)start);
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

PRIVATE int DCALL
rvec_nsi_setrange_n(RefVector *__restrict self,
                    dssize_t start,
                    DeeObject *__restrict values) {
	if (DeeNone_Check(values))
		return rvec_nsi_delrange_n(self, start);
	return rvec_nsi_setrange(self, start,
	                         (size_t)self->rv_length,
	                         values);
}

PRIVATE WUNUSED NONNULL((1, 2, 3)) int DCALL
rvec_delrange(RefVector *__restrict self,
              DeeObject *__restrict start,
              DeeObject *__restrict end) {
	dssize_t start_index;
	dssize_t end_index;
	if (DeeObject_AsSSize(start, &start_index))
		goto err;
	if (DeeNone_Check(end))
		return rvec_nsi_delrange_n(self, start_index);
	if (DeeObject_AsSSize(end, &end_index))
		goto err;
	return rvec_nsi_delrange(self, start_index, end_index);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2, 3, 4)) int DCALL
rvec_setrange(RefVector *__restrict self,
              DeeObject *__restrict start,
              DeeObject *__restrict end,
              DeeObject *__restrict values) {
	dssize_t start_index;
	dssize_t end_index;
	if (DeeObject_AsSSize(start, &start_index))
		goto err;
	if (DeeNone_Check(end))
		return rvec_nsi_setrange_n(self, start_index, values);
	if (DeeObject_AsSSize(end, &end_index))
		goto err;
	return rvec_nsi_setrange(self, start_index, end_index, values);
err:
	return -1;
}


PRIVATE struct type_nsi rvec_nsi = {
	/* .nsi_class   = */ TYPE_SEQX_CLASS_SEQ,
	/* .nsi_flags   = */ TYPE_SEQX_FMUTABLE,
	{
		/* .nsi_seqlike = */ {
			/* .nsi_getsize      = */ (void *)&rvec_nsi_getsize,
			/* .nsi_getsize_fast = */ (void *)&rvec_nsi_getsize,
			/* .nsi_getitem      = */ (void *)&rvec_nsi_getitem,
			/* .nsi_delitem      = */ (void *)&rvec_nsi_delitem,
			/* .nsi_setitem      = */ (void *)&rvec_nsi_setitem,
			/* .nsi_getitem_fast = */ (void *)&rvec_nsi_getitem_fast,
			/* .nsi_getrange     = */ (void *)NULL,
			/* .nsi_getrange_n   = */ (void *)NULL,
			/* .nsi_setrange     = */ (void *)&rvec_nsi_setrange,
			/* .nsi_setrange_n   = */ (void *)&rvec_nsi_setrange_n,
			/* .nsi_find         = */ (void *)&rvec_nsi_find,
			/* .nsi_rfind        = */ (void *)&rvec_nsi_rfind,
			/* .nsi_xch          = */ (void *)&rvec_nsi_xchitem,
			/* .nsi_insert       = */ (void *)NULL,
			/* .nsi_insertall    = */ (void *)NULL,
			/* .nsi_insertvec    = */ (void *)NULL,
			/* .nsi_pop          = */ (void *)NULL,
			/* .nsi_erase        = */ (void *)NULL,
			/* .nsi_remove       = */ (void *)&rvec_nsi_remove,
			/* .nsi_rremove      = */ (void *)&rvec_nsi_rremove,
			/* .nsi_removeall    = */ (void *)&rvec_nsi_removeall,
			/* .nsi_removeif     = */ (void *)&rvec_nsi_removeif
		}
	}
};


PRIVATE struct type_seq rvec_seq = {
	/* .tp_iter_self = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&rvec_iter,
	/* .tp_size      = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&rvec_size,
	/* .tp_contains  = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&rvec_contains,
	/* .tp_get       = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&rvec_getitem,
	/* .tp_del       = */ (int (DCALL *)(DeeObject *, DeeObject *))&rvec_delitem,
	/* .tp_set       = */ (int (DCALL *)(DeeObject *, DeeObject *, DeeObject *))&rvec_setitem,
	/* .tp_range_get = */ NULL,
	/* .tp_range_del = */ (int (DCALL *)(DeeObject *, DeeObject *, DeeObject *))&rvec_delrange,
	/* .tp_range_set = */ (int (DCALL *)(DeeObject *, DeeObject *, DeeObject *, DeeObject *))&rvec_setrange,
	/* .tp_nsi       = */ &rvec_nsi
};

PRIVATE struct type_member rvec_class_members[] = {
	TYPE_MEMBER_CONST("Iterator", &RefVectorIterator_Type),
	TYPE_MEMBER_END
};

PRIVATE struct type_member rvec_members[] = {
	TYPE_MEMBER_FIELD("__owner__", STRUCT_OBJECT, offsetof(RefVector, rv_owner)),
	TYPE_MEMBER_FIELD("__length__", STRUCT_CONST | STRUCT_SIZE_T, offsetof(RefVector, rv_length)),
	TYPE_MEMBER_END
};

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
rvec_get_writable(RefVector *__restrict self) {
#ifndef CONFIG_NO_THREADS
 return_bool_(self->rv_plock != NULL);
#else /* !CONFIG_NO_THREADS */
 return_bool_(self->rv_writable);
#endif /* CONFIG_NO_THREADS */
}

PRIVATE struct type_getset rvec_getsets[] = {
	{ "__writable__",
	  (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&rvec_get_writable,
	  NULL, NULL, DOC("->?Dbool") },
	{ NULL }
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
				/* .tp_ctor      = */ &rvec_init,
				/* .tp_copy_ctor = */ &rvec_copy,
				/* .tp_deep_ctor = */ NULL,
				/* .tp_any_ctor  = */ NULL,
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

PUBLIC WUNUSED WUNUSED NONNULL((1)) DREF DeeObject *DCALL
DeeRefVector_New(DeeObject *owner, size_t length,
                 DeeObject **vector,
#ifndef CONFIG_NO_THREADS
                 rwlock_t *plock
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
	self->si_seq = (DREF SharedVector *)DeeObject_NewDefault(&SharedVector_Type);
	if unlikely(!self->si_seq)
		goto err;
	self->si_index = 0;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
sveciter_init(SharedVectorIterator *__restrict self,
              size_t argc, DeeObject **argv) {
	if (DeeArg_Unpack(argc, argv, "o:_SharedVectorIterator", &self->si_seq))
		goto err;
	if (DeeObject_AssertTypeExact((DeeObject *)self->si_seq, &SharedVector_Type))
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
#ifndef CONFIG_NO_THREADS
	for (;;) {
		size_t index;
		rwlock_read(&vec->sv_lock);
		index = ATOMIC_READ(self->si_index);
		if (self->si_index >= vec->sv_length) {
			rwlock_endread(&vec->sv_lock);
			break;
		}
		result = vec->sv_vector[index];
		/* Acquire a reference to keep the item alive. */
		Dee_Incref(result);
		rwlock_endread(&vec->sv_lock);
		if (ATOMIC_CMPXCH(self->si_index, index, index + 1))
			break;
		/* If some other thread stole the index, drop their value. */
		Dee_Decref(result);
	}
#else /* !CONFIG_NO_THREADS */
	if (self->si_index < vec->sv_length) {
		result = vec->sv_vector[self->si_index++];
		Dee_Incref(result);
	}
#endif /* CONFIG_NO_THREADS */
	return result;
}

INTERN WUNUSED NONNULL((1)) int DCALL
sveciter_bool(SharedVectorIterator *__restrict self) {
#ifdef CONFIG_NO_THREADS
	return self->si_index < self->si_seq->sv_length;
#else /* CONFIG_NO_THREADS */
	return (ATOMIC_READ(self->si_index) <
	        ATOMIC_READ(self->si_seq->sv_length));
#endif /* !CONFIG_NO_THREADS */
}

INTERN WUNUSED NONNULL((1, 2)) int DCALL
sveciter_copy(SharedVectorIterator *__restrict self,
              SharedVectorIterator *__restrict other) {
#ifdef CONFIG_NO_THREADS
	self->si_index = other->si_index;
#else /* CONFIG_NO_THREADS */
	self->si_index = ATOMIC_READ(other->si_index);
#endif /* !CONFIG_NO_THREADS */
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
#ifdef CONFIG_NO_THREADS
	self->si_index = other->si_index;
#else /* CONFIG_NO_THREADS */
	self->si_index = ATOMIC_READ(other->si_index);
#endif /* !CONFIG_NO_THREADS */
	return 0;
err:
	return -1;
}

PRIVATE struct type_member sveciter_members[] = {
	TYPE_MEMBER_FIELD_DOC("seq", STRUCT_OBJECT, offsetof(SharedVectorIterator, si_seq), "->?Ert:SharedVector"),
	TYPE_MEMBER_FIELD("__index__", STRUCT_SIZE_T, offsetof(SharedVectorIterator, si_index)),
	TYPE_MEMBER_END
};

#ifdef CONFIG_NO_THREADS
#define READ_INDEX(x)            ((x)->si_index)
#else /* CONFIG_NO_THREADS */
#define READ_INDEX(x) ATOMIC_READ((x)->si_index)
#endif /* !CONFIG_NO_THREADS */

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
sveciter_eq(SharedVectorIterator *self,
            SharedVectorIterator *other) {
	if (DeeObject_AssertTypeExact((DeeObject *)other, &SharedVectorIterator_Type))
		goto err;
	return_bool_(self->si_seq == other->si_seq &&
	             READ_INDEX(self) == READ_INDEX(other));
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
sveciter_ne(SharedVectorIterator *self,
            SharedVectorIterator *other) {
	if (DeeObject_AssertTypeExact((DeeObject *)other, &SharedVectorIterator_Type))
		goto err;
	return_bool(self->si_seq != other->si_seq ||
	            READ_INDEX(self) != READ_INDEX(other));
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
sveciter_lo(SharedVectorIterator *self,
            SharedVectorIterator *other) {
	if (DeeObject_AssertTypeExact((DeeObject *)other, &SharedVectorIterator_Type))
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
	if (DeeObject_AssertTypeExact((DeeObject *)other, &SharedVectorIterator_Type))
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
	if (DeeObject_AssertTypeExact((DeeObject *)other, &SharedVectorIterator_Type))
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
	if (DeeObject_AssertTypeExact((DeeObject *)other, &SharedVectorIterator_Type))
		goto err;
	return_bool(self->si_seq == other->si_seq
	            ? READ_INDEX(self) >= READ_INDEX(other)
	            : self->si_seq >= other->si_seq);
err:
	return NULL;
}

INTERN struct type_cmp sveciter_cmp = {
	/* .tp_hash = */ NULL,
	/* .tp_eq   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&sveciter_eq,
	/* .tp_ne   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&sveciter_ne,
	/* .tp_lo   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&sveciter_lo,
	/* .tp_le   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&sveciter_le,
	/* .tp_gr   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&sveciter_gr,
	/* .tp_ge   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&sveciter_ge,
};



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
				/* .tp_ctor      = */ (void *)&sveciter_ctor,
				/* .tp_copy_ctor = */ (void *)&sveciter_copy,
				/* .tp_deep_ctor = */ (void *)&sveciter_deep,
				/* .tp_any_ctor  = */ (void *)&sveciter_init,
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
	DREF DeeObject **begin, **iter;
	iter = (begin = self->sv_vector) + self->sv_length;
	while (iter-- != begin)
		Dee_Decref(*iter);
	Dee_Free(begin);
}

PRIVATE NONNULL((1, 2)) void DCALL
svec_visit(SharedVector *__restrict self, dvisit_t proc, void *arg) {
	DREF DeeObject **begin, **iter;
	iter = (begin = self->sv_vector) + self->sv_length;
	while (iter-- != begin) {
		Dee_Visit(*iter);
	}
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

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
svec_size(SharedVector *__restrict self) {
#ifdef CONFIG_NO_THREADS
 size_t result = self->sv_length;
#else /* CONFIG_NO_THREADS */
 size_t result = ATOMIC_READ(self->sv_length);
#endif /* !CONFIG_NO_THREADS */
 return DeeInt_NewSize(result);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
svec_contains(SharedVector *self,
              DeeObject *other) {
	size_t index;
	int temp;
	rwlock_read(&self->sv_lock);
	for (index = 0; index < self->sv_length; ++index) {
		DREF DeeObject *item;
		item = self->sv_vector[index];
		Dee_Incref(item);
		rwlock_endread(&self->sv_lock);
		temp = DeeObject_CompareEq(other, item);
		Dee_Decref(item);
		if (temp != 0) {
			if unlikely(temp < 0)
				goto err;
			return_true;
		}
		rwlock_read(&self->sv_lock);
	}
	rwlock_endread(&self->sv_lock);
	return_false;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
svec_getitem(SharedVector *self,
             DeeObject *index_ob) {
	size_t index;
	DREF DeeObject *result;
	if (DeeObject_AsSize(index_ob, &index))
		goto err;
	rwlock_read(&self->sv_lock);
	if unlikely(index >= self->sv_length) {
		size_t my_length = self->sv_length;
		rwlock_endread(&self->sv_lock);
		err_index_out_of_bounds((DeeObject *)self, index, my_length);
		goto err;
	}
	result = self->sv_vector[index];
	Dee_Incref(result);
	rwlock_endread(&self->sv_lock);
	return result;
err:
	return NULL;
}


PRIVATE WUNUSED NONNULL((1)) size_t DCALL
svec_nsi_getsize(SharedVector *__restrict self) {
	ASSERT(self->sv_length != (size_t)-1);
#ifdef CONFIG_NO_THREADS
	return self->sv_length;
#else /* CONFIG_NO_THREADS */
	return ATOMIC_READ(self->sv_length);
#endif /* !CONFIG_NO_THREADS */
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
svec_nsi_getitem(SharedVector *__restrict self, size_t index) {
	DREF DeeObject *result;
	rwlock_read(&self->sv_lock);
	if unlikely(index >= self->sv_length) {
		size_t my_length = self->sv_length;
		rwlock_endread(&self->sv_lock);
		err_index_out_of_bounds((DeeObject *)self, index, my_length);
		return NULL;
	}
	result = self->sv_vector[index];
	Dee_Incref(result);
	rwlock_endread(&self->sv_lock);
	return result;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
svec_nsi_getitem_fast(SharedVector *__restrict self, size_t index) {
	DREF DeeObject *result;
	rwlock_read(&self->sv_lock);
	if unlikely(index >= self->sv_length) {
		rwlock_endread(&self->sv_lock);
		return NULL;
	}
	result = self->sv_vector[index];
	Dee_Incref(result);
	rwlock_endread(&self->sv_lock);
	return result;
}

PRIVATE size_t DCALL
svec_nsi_find(SharedVector *__restrict self,
              size_t start, size_t end,
              DeeObject *__restrict keyed_search_item,
              DeeObject *key) {
	size_t i = start;
	rwlock_read(&self->sv_lock);
	for (; i < end && i < self->sv_length; ++i) {
		DREF DeeObject *item;
		int temp;
		item = self->sv_vector[i];
		Dee_Incref(item);
		rwlock_endread(&self->sv_lock);
		temp = DeeObject_CompareKeyEq(keyed_search_item, item, key);
		Dee_Decref(item);
		if (temp != 0) {
			if unlikely(temp < 0)
				goto err;
			return i;
		}
		rwlock_read(&self->sv_lock);
	}
	rwlock_endread(&self->sv_lock);
	return (size_t)-1;
err:
	return (size_t)-2;
}

PRIVATE size_t DCALL
svec_nsi_rfind(SharedVector *__restrict self,
               size_t start, size_t end,
               DeeObject *__restrict keyed_search_item,
               DeeObject *key) {
	size_t i = end;
	rwlock_read(&self->sv_lock);
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
		rwlock_endread(&self->sv_lock);
		temp = DeeObject_CompareKeyEq(keyed_search_item, item, key);
		Dee_Decref(item);
		if (temp != 0) {
			if unlikely(temp < 0)
				goto err;
			return i;
		}
		rwlock_read(&self->sv_lock);
	}
	rwlock_endread(&self->sv_lock);
	return (size_t)-1;
err:
	return (size_t)-2;
}


PRIVATE struct type_nsi svec_nsi = {
	/* .nsi_class   = */ TYPE_SEQX_CLASS_SEQ,
	/* .nsi_flags   = */ TYPE_SEQX_FNORMAL,
	{
		/* .nsi_seqlike = */ {
			/* .nsi_getsize      = */ (void *)&svec_nsi_getsize,
			/* .nsi_getsize_fast = */ (void *)&svec_nsi_getsize,
			/* .nsi_getitem      = */ (void *)&svec_nsi_getitem,
			/* .nsi_delitem      = */ (void *)NULL,
			/* .nsi_setitem      = */ (void *)NULL,
			/* .nsi_getitem_fast = */ (void *)&svec_nsi_getitem_fast,
			/* .nsi_getrange     = */ (void *)NULL,
			/* .nsi_getrange_n   = */ (void *)NULL,
			/* .nsi_setrange     = */ (void *)NULL,
			/* .nsi_setrange_n   = */ (void *)NULL,
			/* .nsi_find         = */ (void *)&svec_nsi_find,
			/* .nsi_rfind        = */ (void *)&svec_nsi_rfind,
			/* .nsi_xch          = */ (void *)NULL,
			/* .nsi_insert       = */ (void *)NULL,
			/* .nsi_insertall    = */ (void *)NULL,
			/* .nsi_insertvec    = */ (void *)NULL,
			/* .nsi_pop          = */ (void *)NULL,
			/* .nsi_erase        = */ (void *)NULL,
			/* .nsi_remove       = */ (void *)NULL,
			/* .nsi_rremove      = */ (void *)NULL,
			/* .nsi_removeall    = */ (void *)NULL,
			/* .nsi_removeif     = */ (void *)NULL
		}
	}
};

PRIVATE struct type_seq svec_seq = {
	/* .tp_iter_self = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&svec_iter,
	/* .tp_size      = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&svec_size,
	/* .tp_contains  = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&svec_contains,
	/* .tp_get       = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&svec_getitem,
	/* .tp_del       = */ NULL,
	/* .tp_set       = */ NULL,
	/* .tp_range_get = */ NULL,
	/* .tp_range_del = */ NULL,
	/* .tp_range_set = */ NULL,
	/* .tp_nsi       = */ &svec_nsi,
};

PRIVATE struct type_member svec_class_members[] = {
	TYPE_MEMBER_CONST("Iterator", &SharedVectorIterator_Type),
	TYPE_MEMBER_END
};

PRIVATE WUNUSED NONNULL((1)) int DCALL
svec_ctor(SharedVector *__restrict self) {
	rwlock_init(&self->sv_lock);
	self->sv_length = 0;
	self->sv_vector = NULL;
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
svec_copy(SharedVector *__restrict self,
          SharedVector *__restrict other) {
	size_t i;
again:
	rwlock_read(&other->sv_lock);
	self->sv_length = other->sv_length;
	self->sv_vector = (DREF DeeObject **)Dee_TryMalloc(self->sv_length *
	                                                   sizeof(DREF DeeObject *));
	if unlikely(!self->sv_vector) {
		rwlock_endread(&other->sv_lock);
		if (Dee_CollectMemory(self->sv_length * sizeof(DREF DeeObject *)))
			goto again;
		goto err;
	}
	MEMCPY_PTR(self->sv_vector, other->sv_vector, self->sv_length);
	for (i = 0; i < self->sv_length; ++i)
		Dee_Incref(self->sv_vector[i]);
	rwlock_endread(&other->sv_lock);
	rwlock_init(&self->sv_lock);
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
		if unlikely(DeeObject_InplaceDeepCopy(&self->sv_vector[i]))
			goto err_r;
	}
	return 0;
err_r:
	for (i = 0; i < self->sv_length; ++i)
		Dee_Decref(self->sv_vector[i]);
	Dee_Free(self->sv_vector);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
svec_bool(SharedVector *__restrict self) {
#ifdef CONFIG_NO_THREADS
	return self->sv_length != 0;
#else /* CONFIG_NO_THREADS */
	return ATOMIC_READ(self->sv_length) != 0;
#endif /* !CONFIG_NO_THREADS */
}

INTERN DeeTypeObject SharedVector_Type = {
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
				/* .tp_ctor      = */ (void *)&svec_ctor,
				/* .tp_copy_ctor = */ (void *)&svec_copy,
				/* .tp_deep_ctor = */ (void *)&svec_deep,
				/* .tp_any_ctor  = */ (void *)NULL,
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
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ svec_class_members
};


/* Create a new shared vector that will inherit elements
 * from the given vector once `DeeSharedVector_Decref()' is called.
 * NOTE: This function implicitly inherits a reference to each item
 *       of the given vector, though does not actually inherit the
 *       vector itself! */
PUBLIC WUNUSED DREF DeeObject *DCALL
DeeSharedVector_NewShared(size_t length, DREF DeeObject **vector) {
	DREF SharedVector *result;
	result = DeeObject_MALLOC(SharedVector);
	if unlikely(!result)
		goto done;
	DeeObject_Init(result, &SharedVector_Type);
	rwlock_init(&result->sv_lock);
	result->sv_length = length;
	result->sv_vector = vector;
done:
	return (DREF DeeObject *)result;
}

/* Check if the reference counter of `self' is 1. When it is,
 * simply destroy the shared vector without freeing `sv_vector',
 * but still decref() all contained object.
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
 *    the `ASM_CALL_SEQ' opcode, as generated for brace-initializers. */
PUBLIC NONNULL((1)) void DCALL
DeeSharedVector_Decref(DREF DeeObject *__restrict self) {
	DREF DeeObject **begin, **iter;
	DREF DeeObject **vector_copy;
	SharedVector *me = (SharedVector *)self;
	ASSERT_OBJECT_TYPE_EXACT(me, &SharedVector_Type);
	if (!DeeObject_IsShared(me)) {
		/* Simple case: The vector isn't being shared. */
		iter = (begin = me->sv_vector) + me->sv_length;
		while (iter-- != begin)
			Dee_Decref(*iter);
		Dee_DecrefNokill(&SharedVector_Type);
		DeeObject_FreeTracker((DeeObject *)me);
		DeeObject_Free(me);
		return;
	}
	/* Difficult case: must duplicate the vector. */
	rwlock_write(&me->sv_lock);
	vector_copy = (DREF DeeObject **)Dee_TryMalloc(me->sv_length *
	                                               sizeof(DREF DeeObject *));
	if unlikely(!vector_copy)
		goto err_cannot_inherit;
	/* Simply copy all the elements, transferring
	 * all the references that they represent. */
	MEMCPY_PTR(vector_copy, me->sv_vector, me->sv_length);
	/* Give the SharedVector its very own copy
	 * which it will take to its grave. */
	me->sv_vector = vector_copy;
	rwlock_endwrite(&me->sv_lock);
	Dee_Decref(me);
	return;

err_cannot_inherit:
	/* Special case: failed to create a copy that the vector may call its own. */
	iter = (begin = me->sv_vector) + me->sv_length;
	/* Override with an empty vector. */
	me->sv_vector = NULL;
	me->sv_length = 0;
	rwlock_endwrite(&me->sv_lock);
	/* Destroy the items that the caller wanted the vector to inherit. */
	while (iter-- != begin)
		Dee_Decref(*iter);
	Dee_Decref(me);
}



DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_SEQ_SVEC_C */
