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
#ifndef GUARD_DEX_CTYPES_ARRAY_C
#define GUARD_DEX_CTYPES_ARRAY_C 1
#define DEE_SOURCE

#include "libctypes.h"
/**/

#include <deemon/alloc.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/error.h>
#include <deemon/format.h>
#include <deemon/gc.h>
#include <deemon/int.h>
#include <deemon/none.h>
#include <deemon/seq.h>
#include <deemon/string.h>
#include <deemon/system-features.h> /* bzero() */
#include <deemon/util/atomic.h>

#include <hybrid/limitcore.h>
#include <hybrid/overflow.h>
#include <hybrid/typecore.h>

#undef SSIZE_MAX
#define SSIZE_MAX __SSIZE_MAX__
#undef byte_t
#define byte_t __BYTE_TYPE__

DECL_BEGIN

typedef struct {
	OBJECT_HEAD
	DREF DeeLValueTypeObject *ai_type;  /* [1..1][const] The l-value version of array items types.
	                                     *  This type is used to construct references to array elements. */
	size_t                    ai_siz;   /* [const][== DeeSType_Sizeof(ai_type->lt_orig)]
	                                     *  Size of a single array element. */
	union pointer             ai_begin; /* [const] Iterator starting position. */
	union pointer             ai_pos;   /* [atomic] Current iterator position. */
	union pointer             ai_end;   /* [const] Iterator end position. */
} ArrayIterator;

PRIVATE NONNULL((1)) void DCALL
aiter_fini(ArrayIterator *__restrict self) {
	Dee_Decref(DeeLValueType_AsType(self->ai_type));
}

PRIVATE NONNULL((1, 2)) void DCALL
aiter_visit(ArrayIterator *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(DeeLValueType_AsType(self->ai_type));
}


PRIVATE WUNUSED NONNULL((1)) DREF struct lvalue_object *DCALL
aiter_next(ArrayIterator *__restrict self) {
	DREF struct lvalue_object *result;
	union pointer result_pointer;
	do {
		result_pointer.uint = atomic_read(&self->ai_pos.uint);
		if (result_pointer.uint >= self->ai_end.uint)
			return (DREF struct lvalue_object *)ITER_DONE;
	} while (!atomic_cmpxch_weak_or_write(&self->ai_pos.uint,
	                                      result_pointer.uint,
	                                      result_pointer.uint + self->ai_siz));

	/* Construct an l-value object for the array item. */
	result = DeeObject_MALLOC(struct lvalue_object);
	if unlikely(!result)
		goto done;
	DeeObject_Init(result, DeeLValueType_AsType(self->ai_type));

	/* Use the item pointer which we've just extracted. */
	result->l_ptr.ptr = result_pointer.ptr;
done:
	return result;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
aiter_bool(ArrayIterator *__restrict self) {
	return atomic_read(&self->ai_pos.ptr) >= self->ai_end.ptr;
}

PRIVATE WUNUSED NONNULL((1)) DREF struct lvalue_object *DCALL
aiter_getseq(ArrayIterator *__restrict self) {
	DREF struct lvalue_object *result;
	DREF DeeArrayTypeObject *atype;
	DREF DeeLValueTypeObject *ltype;
	size_t count = (self->ai_end.uint - self->ai_begin.uint);
	if unlikely(!self->ai_siz) {
		count = 0;
	} else {
		count /= self->ai_siz;
	}

	/* Reverse engineer the proper array type. */
	atype = DeeSType_Array(self->ai_type->lt_orig, count);
	if unlikely(!atype)
		goto err;
	ltype = DeeSType_LValue(DeeArrayType_AsSType(atype));
	Dee_Decref(DeeArrayType_AsType(atype));
	if unlikely(!ltype)
		goto err;

	/* Construct an l-value object for the array base address. */
	result = DeeObject_MALLOC(struct lvalue_object);
	if unlikely(!result)
		goto err_ltype;
	DeeObject_InitNoref(result, DeeLValueType_AsType(ltype)); /* Inherit reference: ltype */
	result->l_ptr.ptr = self->ai_begin.ptr;
	return result;
err_ltype:
	Dee_Decref(DeeLValueType_AsType(ltype));
err:
	return NULL;
}

PRIVATE struct type_getset tpconst aiter_getsets[] = {
	TYPE_GETTER_F_NODOC("seq", &aiter_getseq, METHOD_FNOREFESCAPE),
	TYPE_GETSET_END
};


PRIVATE DeeTypeObject ArrayIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "ArrayIterator",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		{
			/* .tp_alloc = */ {
				/* .tp_ctor      = */ (dfunptr_t)NULL,
				/* .tp_copy_ctor = */ (dfunptr_t)NULL,
				/* .tp_deep_ctor = */ (dfunptr_t)NULL,
				/* .tp_any_ctor  = */ (dfunptr_t)NULL,
				TYPE_FIXED_ALLOCATOR_S(DeeObject)
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&aiter_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&aiter_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&aiter_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&aiter_next,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ aiter_getsets,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};


PRIVATE WUNUSED NONNULL((1)) DREF ArrayIterator *DCALL
array_iter(DeeArrayTypeObject *tp_self, void *base) {
	DREF ArrayIterator *result;

	/* Create a new array iterator. */
	result = DeeObject_MALLOC(ArrayIterator);
	if unlikely(!result)
		goto done;

	/* Construct the l-value version of the array's item type. */
	result->ai_type = DeeSType_LValue(tp_self->at_orig);
	if unlikely(!result->ai_type)
		goto err_r;
	result->ai_begin.ptr = base;
	result->ai_pos.ptr   = base;
	result->ai_siz       = DeeSType_Sizeof(tp_self->at_orig);
	result->ai_end.uint  = (uintptr_t)base + result->ai_siz * tp_self->at_count;
	DeeObject_Init(result, &ArrayIterator_Type);
done:
	return result;
err_r:
	DeeObject_FREE(result);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
array_size(DeeArrayTypeObject *tp_self, void *UNUSED(base)) {
	return DeeInt_NewSize(tp_self->at_count);
}

PRIVATE WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL
array_contains(DeeArrayTypeObject *tp_self, void *base, DeeObject *other) {
	DREF struct lvalue_object *temp = NULL;
	DREF DeeLValueTypeObject *lval_type;
	union pointer iter, end;
	size_t siz;
	int error;
	lval_type = DeeSType_LValue(tp_self->at_orig);
	if unlikely(!lval_type)
		goto err;
	siz      = DeeSType_Sizeof(tp_self->at_orig);
	iter.ptr = end.ptr = base;
	end.uint += tp_self->at_count * siz;
	while (iter.ptr < end.ptr) {
		/* Construct a temporary l-value to use in comparisons against `other'. */
		if (!temp || DeeObject_IsShared(temp)) {
			Dee_XDecref(temp);
			temp = DeeObject_MALLOC(struct lvalue_object);
			if unlikely(!temp)
				goto err_lval_type;
			DeeObject_Init(temp, DeeLValueType_AsType(lval_type));
		}
		temp->l_ptr.ptr = iter.ptr;
		iter.uint += siz;
		error = DeeObject_TryCompareEq(other, (DeeObject *)temp);
		if unlikely(error == Dee_COMPARE_ERR)
			goto err_lval_type_temp;
		if (error == 0) {
			/* Item was found! */
			Dee_Decref(temp);
			Dee_Decref(DeeLValueType_AsType(lval_type));
			return_true;
		}
	}
	Dee_XDecref(temp);
	Dee_Decref(DeeLValueType_AsType(lval_type));
	return_false;
err_lval_type_temp:
	Dee_Decref(temp);
err_lval_type:
	Dee_Decref(DeeLValueType_AsType(lval_type));
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL
array_get(DeeArrayTypeObject *tp_self, void *base, DeeObject *index_ob) {
	DREF struct lvalue_object *result;
	DREF DeeLValueTypeObject *lval_type;
	size_t index;
	if (DeeObject_AsSize(index_ob, &index))
		goto err;

	/* Check bounds. */
	if unlikely(index >= tp_self->at_count) {
		DeeError_Throwf(&DeeError_IndexError,
		                "Index `%" PRFuSIZ "' lies outside the valid bounds `0...%" PRFuSIZ "'",
		                index, tp_self->at_count);
		goto err;
	}
	lval_type = DeeSType_LValue(tp_self->at_orig);
	if unlikely(!lval_type)
		goto err;

	/* Construct an l-value object to-be returned. */
	result = DeeObject_MALLOC(struct lvalue_object);
	if unlikely(!result)
		goto err_lval_type;
	DeeObject_InitNoref(result, DeeLValueType_AsType(lval_type)); /* Inherit reference: lval_type */

	/* Calculate the resulting item address. */
	result->l_ptr.uint = (uintptr_t)base + index * DeeSType_Sizeof(tp_self->at_orig);
	return (DREF DeeObject *)result;
err_lval_type:
	Dee_Decref(DeeLValueType_AsType(lval_type));
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 3, 4)) int DCALL
array_set(DeeArrayTypeObject *tp_self, void *base,
          DeeObject *index_ob, DeeObject *value) {
	int result;
	DREF DeeObject *item;
	item = array_get(tp_self, base, index_ob);
	if unlikely(!item)
		goto err;

	/* Assign the given value to the item. */
	result = DeeObject_Assign(item, value);
	Dee_Decref(item);
	return result;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
array_del(DeeArrayTypeObject *tp_self, void *base, DeeObject *index_ob) {
	return array_set(tp_self, base, index_ob, Dee_None);
}

PRIVATE WUNUSED NONNULL((1, 3, 4)) DREF DeeObject *DCALL
array_getrange(DeeArrayTypeObject *tp_self, void *base,
               DeeObject *begin_ob, DeeObject *end_ob) {
	DREF struct lvalue_object *result;
	DREF DeeLValueTypeObject *lval_type;
	DREF DeeArrayTypeObject *array_type;
	dssize_t i_begin, i_end = (dssize_t)tp_self->at_count;
	struct Dee_seq_range range;
	size_t range_size;
	if (DeeObject_AsSSize(begin_ob, &i_begin))
		goto err;
	if (!DeeNone_Check(end_ob)) {
		if (DeeObject_AsSSize(end_ob, &i_end))
			goto err;
	}
	DeeSeqRange_Clamp(&range, i_begin, i_end, tp_self->at_count);
	range_size = range.sr_end - range.sr_start;

	/* Construct a sub-array type. */
	array_type = DeeSType_Array(tp_self->at_orig, range_size);
	if unlikely(!array_type)
		goto err;

	/* Create the associated l-value type. */
	lval_type = DeeSType_LValue(DeeArrayType_AsSType(array_type));
	Dee_Decref(DeeArrayType_AsType(array_type));
	if unlikely(!lval_type)
		goto err;
	result = DeeObject_MALLOC(struct lvalue_object);
	if unlikely(!result)
		goto err_lval_type;
	DeeObject_InitNoref(result, DeeLValueType_AsType(lval_type)); /* Inherit reference: lval_type */

	/* Set the base pointer to the start of the requested sub-range. */
	result->l_ptr.uint = ((uintptr_t)base) +
	                     (range.sr_start * DeeSType_Sizeof(tp_self->at_orig));
	return (DREF DeeObject *)result;
err_lval_type:
	Dee_Decref(DeeLValueType_AsType(lval_type));
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 3, 4)) int DCALL
array_delrange(DeeArrayTypeObject *tp_self, void *base,
               DeeObject *begin_ob, DeeObject *end_ob) {
	dssize_t i_begin, i_end = (dssize_t)tp_self->at_count;
	struct Dee_seq_range range;
	size_t range_size;
	if (DeeObject_AsSSize(begin_ob, &i_begin))
		goto err;
	if (!DeeNone_Check(end_ob)) {
		if (DeeObject_AsSSize(end_ob, &i_end))
			goto err;
	}
	DeeSeqRange_Clamp(&range, i_begin, i_end, tp_self->at_count);
	range_size = range.sr_end - range.sr_start;
	if unlikely(range_size <= 0) {
		/* Empty range . */
	} else {
		size_t item_size;
		byte_t *del_begin;
		size_t del_size;

		/* Simply zero out the described memory range. */
		item_size = DeeSType_Sizeof(tp_self->at_orig);
		del_size  = range_size * item_size;
		del_begin = (byte_t *)((uintptr_t)base + (range.sr_start * item_size));
		CTYPES_FAULTPROTECT(bzero(del_begin, del_size), goto err);
	}
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 3, 4, 5)) int DCALL
array_setrange(DeeArrayTypeObject *tp_self, void *base,
               DeeObject *begin_ob, DeeObject *end_ob, DeeObject *value) {
	dssize_t i_begin, i_end;
	struct Dee_seq_range range;
	size_t range_size;
	DREF DeeObject *iter, *elem;

	/* When `none' is passed, simply clear out affected memory. */
	if (DeeNone_Check(value))
		return array_delrange(tp_self, base, begin_ob, end_ob);
	i_end = (dssize_t)tp_self->at_count;
	if (DeeObject_AsSSize(begin_ob, &i_begin))
		goto err;
	if (!DeeNone_Check(end_ob)) {
		if (DeeObject_AsSSize(end_ob, &i_end))
			goto err;
	}
	DeeSeqRange_Clamp(&range, i_begin, i_end, tp_self->at_count);
	range_size = range.sr_end - range.sr_start;
	iter = DeeObject_Iter(value);
	if unlikely(!iter)
		goto err;
	if unlikely(range_size <= 0) {
		/* Empty range. */
	} else {
		size_t item_size;
		union pointer array_iter, array_end;
		item_size      = DeeSType_Sizeof(tp_self->at_orig);
		array_iter.ptr = (byte_t *)base + range.sr_start * item_size;
		array_end.ptr  = (byte_t *)base + range.sr_end * item_size;
		while (array_iter.ptr < array_end.ptr) {
			int error;
			elem = DeeObject_IterNext(iter);
			if unlikely(!ITER_ISOK(elem)) {
				if (elem) { /* Unexpected end of sequence. */
					size_t given_count;
					given_count = array_iter.uint - ((uintptr_t)base + range.sr_start * item_size);
					if (item_size)
						given_count /= item_size;
					DeeError_Throwf(&DeeError_UnpackError,
					                "Expected %" PRFuSIZ " object%s when only %" PRFuSIZ " w%s given",
					                tp_self->at_count, tp_self->at_count == 1 ? "" : "s",
					                given_count, given_count == 1 ? "as" : "ere");
				}
				goto err_iter;
			}

			/* Assign this item to the array element. */
			error = DeeStruct_Assign(tp_self->at_orig, array_iter.ptr, elem);
			Dee_Decref(elem);
			if unlikely(error)
				goto err_iter;

			/* Advance the iterator. */
			array_iter.uint += item_size;
		}
	}

	/* Ensure that the given sequence ends here. */
	elem = DeeObject_IterNext(iter);
	if unlikely(elem != ITER_DONE) {
		if (elem != NULL) {
			Dee_Decref(elem);
			DeeError_Throwf(&DeeError_UnpackError,
			                "Expected %" PRFuSIZ " object%s when at least %" PRFuSIZ " w%s given",
			                tp_self->at_count, tp_self->at_count == 1 ? "" : "s",
			                tp_self->at_count + 1, tp_self->at_count == 0 ? "as" : "ere");
		}
		goto err_iter;
	}
	Dee_Decref(iter);
	return 0;
err_iter:
	Dee_Decref(iter);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
array_assign(DeeArrayTypeObject *tp_self, void *base, DeeObject *value) {
	return array_setrange(tp_self, base, Dee_None, Dee_None, value);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
array_init(DeeArrayTypeObject *tp_self, void *base,
           size_t argc, DeeObject *const *argv) {
	DeeObject *arg;
	if (DeeArg_Unpack(argc, argv, "o:array", &arg))
		goto err;
	return array_assign(tp_self, base, arg);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) DREF struct pointer_object *DCALL
array_adddiff(DeeArrayTypeObject *tp_self,
              void *base, ptrdiff_t diff) {
	/* Follow C-conventions and return a pointer to the `diff's element. */
	DREF struct pointer_object *result;
	DREF DeePointerTypeObject *ptr_type;
	ptr_type = DeeSType_Pointer(tp_self->at_orig);
	if unlikely(!ptr_type)
		goto err;

	/* Construct a new pointer object. */
	result = DeeObject_MALLOC(struct pointer_object);
	if unlikely(!result)
		goto err_ptr_type;
	DeeObject_InitNoref(result, DeePointerType_AsType(ptr_type)); /* Inherit reference: ptr_type */

	/* Calculate the effect address of the `diff's element. */
	result->p_ptr.ptr = base;
	result->p_ptr.uint += diff * DeeSType_Sizeof(tp_self->at_orig);

	return result;
err_ptr_type:
	Dee_Decref(DeePointerType_AsType(ptr_type));
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 3)) DREF struct pointer_object *DCALL
array_add(DeeArrayTypeObject *tp_self, void *base, DeeObject *value) {
	ptrdiff_t diff;
	if (DeeObject_AsPtrdiff(value, &diff))
		goto err;
	return array_adddiff(tp_self, base, diff);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 3)) DREF struct pointer_object *DCALL
array_sub(DeeArrayTypeObject *tp_self, void *base, DeeObject *value) {
	ptrdiff_t diff;
	if (DeeObject_AsPtrdiff(value, &diff))
		goto err;
	return array_adddiff(tp_self, base, -diff);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
array_repr(DeeArrayTypeObject *tp_self, void *base) {
	union pointer iter, end;
	size_t item_size;
	struct ascii_printer p = ASCII_PRINTER_INIT;

	/* Print all array elements in a fashion that mimics `sequence.operator repr' */
	if unlikely(ASCII_PRINTER_PRINT(&p, "{ ") < 0)
		goto err;
	item_size = DeeSType_Sizeof(tp_self->at_orig);
	end.ptr = iter.ptr = base;
	end.uint += tp_self->at_count * item_size;
	while (iter.ptr < end.ptr) {
		DREF DeeObject *item_repr;
		dssize_t temp;
		if (iter.ptr != base &&
		    ASCII_PRINTER_PRINT(&p, ", ") < 0)
			goto err;
		item_repr = DeeStruct_Repr(tp_self->at_orig, iter.ptr);
		if unlikely(!item_repr)
			goto err;
		temp = ascii_printer_print(&p, DeeString_STR(item_repr),
		                           DeeString_SIZE(item_repr));
		Dee_Decref_likely(item_repr);
		if unlikely(temp < 0)
			goto err;
		iter.uint += item_size;
	}
	if unlikely(ASCII_PRINTER_PRINT(&p, " }") < 0)
		goto err;
	return ascii_printer_pack(&p);
err:
	ascii_printer_fini(&p);
	return NULL;
}


PRIVATE WUNUSED NONNULL((1)) int DCALL
array_bool(DeeArrayTypeObject *tp_self, void *UNUSED(base)) {
	return tp_self->at_count != 0;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
array_call(DeeArrayTypeObject *tp_self, void *base,
           size_t argc, DeeObject *const *argv) {
	/* Because arrays must behave compatible to pointers,
	 * calling an array will call its first element. */
	return DeeStruct_Call(tp_self->at_orig, base, argc, argv);
}

PRIVATE struct stype_math array_math = {
	/* .st_int32       = */ NULL,
	/* .st_int64       = */ NULL,
	/* .st_double      = */ NULL,
	/* .st_int         = */ NULL,
	/* .st_inv         = */ NULL,
	/* .st_pos         = */ NULL,
	/* .st_neg         = */ NULL,
	/* .st_add         = */ (DREF DeeObject *(DCALL *)(DeeSTypeObject *, void *, DeeObject *))&array_add,
	/* .st_sub         = */ (DREF DeeObject *(DCALL *)(DeeSTypeObject *, void *, DeeObject *))&array_sub,
	/* .st_mul         = */ NULL,
	/* .st_div         = */ NULL,
	/* .st_mod         = */ NULL,
	/* .st_shl         = */ NULL,
	/* .st_shr         = */ NULL,
	/* .st_and         = */ NULL,
	/* .st_or          = */ NULL,
	/* .st_xor         = */ NULL,
	/* .st_pow         = */ NULL,
	/* .st_inc         = */ NULL,
	/* .st_dec         = */ NULL,
	/* .st_inplace_add = */ NULL,
	/* .st_inplace_sub = */ NULL,
	/* .st_inplace_mul = */ NULL,
	/* .st_inplace_div = */ NULL,
	/* .st_inplace_mod = */ NULL,
	/* .st_inplace_shl = */ NULL,
	/* .st_inplace_shr = */ NULL,
	/* .st_inplace_and = */ NULL,
	/* .st_inplace_or  = */ NULL,
	/* .st_inplace_xor = */ NULL,
	/* .st_inplace_pow = */ NULL
};

PRIVATE struct stype_seq array_seq = {
	/* .st_iter_self = */ (DREF DeeObject *(DCALL *)(DeeSTypeObject *__restrict, void *))&array_iter,
	/* .st_size      = */ (DREF DeeObject *(DCALL *)(DeeSTypeObject *__restrict, void *))&array_size,
	/* .st_contains  = */ (DREF DeeObject *(DCALL *)(DeeSTypeObject *, void *, DeeObject *))&array_contains,
	/* .st_get       = */ (DREF DeeObject *(DCALL *)(DeeSTypeObject *, void *, DeeObject *))&array_get,
	/* .st_del       = */ (int (DCALL *)(DeeSTypeObject *, void *, DeeObject *))&array_del,
	/* .st_set       = */ (int (DCALL *)(DeeSTypeObject *, void *, DeeObject *, DeeObject *))&array_set,
	/* .st_range_get = */ (DREF DeeObject *(DCALL *)(DeeSTypeObject *, void *, DeeObject *, DeeObject *))&array_getrange,
	/* .st_range_del = */ (int (DCALL *)(DeeSTypeObject *, void *, DeeObject *, DeeObject *))&array_delrange,
	/* .st_range_set = */ (int (DCALL *)(DeeSTypeObject *, void *, DeeObject *, DeeObject *, DeeObject *))&array_setrange
};


PRIVATE struct type_member tpconst array_class_members[] = {
	TYPE_MEMBER_CONST("Iterator", &ArrayIterator_Type),
	TYPE_MEMBER_END
};

PRIVATE DeeTypeObject *tpconst array_mro[] = {
	DeeSType_AsType(&DeeStructured_Type),
	&DeeSeq_Type,
	&DeeObject_Type,
	NULL
};

INTERN DeeArrayTypeObject DeeArray_Type = {
	/* .ft_base = */ {
		/* .st_base = */ {
			OBJECT_HEAD_INIT(&DeeArrayType_Type),
			/* .tp_name     = */ "Array",
			/* .tp_doc      = */ NULL,
			/* .tp_flags    = */ TP_FNORMAL | TP_FINHERITCTOR | TP_FTRUNCATE | TP_FMOVEANY,
			/* .tp_weakrefs = */ 0,
			/* .tp_features = */ TF_NONE,
			/* .tp_base     = */ DeeSType_AsType(&DeeStructured_Type),
			/* .tp_init = */ {
				{
					/* .tp_alloc = */ {
						/* .tp_ctor      = */ (dfunptr_t)NULL,
						/* .tp_copy_ctor = */ (dfunptr_t)NULL,
						/* .tp_deep_ctor = */ (dfunptr_t)NULL,
						/* .tp_any_ctor  = */ (dfunptr_t)NULL,
						TYPE_FIXED_ALLOCATOR_S(DeeObject)
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
			/* .tp_cmp           = */ NULL,
			/* .tp_seq           = */ NULL,
			/* .tp_iter_next     = */ NULL,
			/* .tp_iterator      = */ NULL,
			/* .tp_attr          = */ NULL,
			/* .tp_with          = */ NULL,
			/* .tp_buffer        = */ NULL,
			/* .tp_methods       = */ NULL,
			/* .tp_getsets       = */ NULL,
			/* .tp_members       = */ NULL,
			/* .tp_class_methods = */ NULL,
			/* .tp_class_getsets = */ NULL,
			/* .tp_class_members = */ array_class_members,
			/* .tp_method_hints  = */ NULL,
			/* .tp_call_kw       = */ NULL,
			/* .tp_mro           = */ array_mro,
		},
#ifndef CONFIG_NO_THREADS
		/* .st_cachelock = */ DEE_ATOMIC_RWLOCK_INIT,
#endif /* !CONFIG_NO_THREADS */
		/* .st_pointer  = */ &DeePointer_Type,
		/* .st_lvalue   = */ &DeeLValue_Type,
		/* .st_array    = */ STYPE_ARRAY_INIT,
#ifndef CONFIG_NO_CFUNCTION
		/* .st_cfunction= */ STYPE_CFUNCTION_INIT,
		/* .st_ffitype  = */ &ffi_type_void,
#endif /* !CONFIG_NO_CFUNCTION */
		/* .st_sizeof   = */ 0,
		/* .st_align    = */ CONFIG_CTYPES_ALIGNOF_POINTER,
		/* .st_init     = */ (int (DCALL *)(DeeSTypeObject *, void *, size_t, DeeObject *const *))&array_init,
		/* .st_assign   = */ (int (DCALL *)(DeeSTypeObject *, void *, DeeObject *))&array_assign,
		/* .st_cast     = */ {
			/* .st_str  = */ NULL,
			/* .st_repr = */ (DREF DeeObject *(DCALL *)(DeeSTypeObject *__restrict, void *))&array_repr,
			/* .st_bool = */ (int (DCALL *)(DeeSTypeObject *__restrict, void *))&array_bool
		},
		/* .st_call     = */ (DREF DeeObject *(DCALL *)(DeeSTypeObject *, void *, size_t, DeeObject *const *))&array_call,
		/* .st_math     = */ &array_math,
		/* .st_cmp      = */ NULL,
		/* .st_seq      = */ &array_seq,
		/* .st_attr     = */ NULL
	},
	/* .at_orig  = */ &DeeStructured_Type,
	/* .at_chain = */ { NULL, NULL },
	/* .at_count = */ 0
};


PRIVATE WUNUSED NONNULL((1)) DREF DeeArrayTypeObject *DCALL
arraytype_new(DeeSTypeObject *__restrict item_type,
              size_t num_items) {
	DREF DeeArrayTypeObject *result;
	DREF DeeStringObject *name;

	result = DeeGCObject_CALLOC(DeeArrayTypeObject);
	if unlikely(!result)
		goto err;

	/* Create the name of the resulting type. */
	name = (DREF DeeStringObject *)DeeString_Newf("%k[%" PRFuSIZ "]", item_type, num_items);
	if unlikely(!name)
		goto err_r;

	/* Store a reference to the array-base type. */
	Dee_Incref(DeeSType_AsType(item_type));
	Dee_Incref(DeeArrayType_AsType(&DeeArray_Type));

	/* Initialize fields. */
	result->at_orig          = item_type;           /* Inherit reference. */
	result->at_base.st_align = item_type->st_align; /* Re-use item alignment. */
	if (OVERFLOW_UMUL(num_items, DeeSType_Sizeof(item_type),
	                  &result->at_base.st_sizeof)) {
		/* Overflow: Array is too large. */
		DeeError_Throwf(&DeeError_IntegerOverflow,
		                "Array `%k' is too large",
		                name);
		Dee_DecrefNokill(DeeArrayType_AsType(&DeeArray_Type));
		Dee_Decref(DeeSType_AsType(item_type));
		Dee_Decref(name);
		goto err_r;
	}
	result->at_base.st_base.tp_init.tp_alloc.tp_instance_size = result->at_base.st_sizeof;
	result->at_base.st_base.tp_init.tp_alloc.tp_instance_size += sizeof(DeeObject);
	result->at_base.st_base.tp_name  = DeeString_STR(name); /* Inherit reference. */
	result->at_base.st_base.tp_flags = TP_FTRUNCATE | TP_FINHERITCTOR | TP_FNAMEOBJECT | TP_FHEAP | TP_FMOVEANY;
	result->at_base.st_base.tp_base  = DeeArrayType_AsType(&DeeArray_Type); /* Inherit reference. */
	result->at_count                 = num_items;

	/* Finalize the array type. */
	DeeObject_Init(DeeArrayType_AsType(result), &DeeArrayType_Type);
	return DeeType_AsArrayType((DeeTypeObject *)DeeGC_Track(DeeArrayType_AsObject(result)));
err_r:
	DeeGCObject_FREE(result);
err:
	return NULL;
}

INTERN NONNULL((1)) bool DCALL
stype_array_rehash(DeeSTypeObject *__restrict self,
                   size_t new_mask) {
	struct array_type_list *new_map;
again:
	new_map = (struct array_type_list *)Dee_TryCallocc(new_mask + 1,
	                                                   sizeof(struct array_type_list));
	if unlikely(!new_map) {
		/* Try again with a 1-element mask. */
		if (!self->st_array.sa_list && new_mask != 0) {
			new_mask = 1;
			goto again;
		}
		return false;
	}

	/* Do the re-hash. */
	if (self->st_array.sa_size) {
		struct array_type_list *biter, *bend;
		ASSERT(self->st_array.sa_list);
		bend = (biter = self->st_array.sa_list) + (self->st_array.sa_mask + 1);
		for (; biter < bend; ++biter) {
			DeeArrayTypeObject *iter, *next;
			iter = LIST_FIRST(biter);
			while (iter) {
				struct array_type_list *dst;
				next = LIST_NEXT(iter, at_chain);
				dst  = &new_map[iter->at_count & new_mask];
				/* Insert the entry into the new hash-map. */
				LIST_INSERT_HEAD(dst, iter, at_chain);
				iter = next;
			}
		}
	}

	/* Delete the old map and install the new. */
	Dee_Free(self->st_array.sa_list);
	self->st_array.sa_mask = new_mask;
	self->st_array.sa_list = new_map;
	return true;
}



INTDEF WUNUSED NONNULL((1)) DREF DeeArrayTypeObject *DCALL
DeeSType_Array(DeeSTypeObject *__restrict self,
               size_t num_items) {
	DREF DeeArrayTypeObject *result;
	DREF struct array_type_list *bucket;
	ASSERT_OBJECT_TYPE(DeeSType_AsType(self), &DeeSType_Type);
	DeeSType_CacheLockRead(self);
	ASSERT(!self->st_array.sa_size ||
	       self->st_array.sa_mask);
	if (self->st_array.sa_size) {
		result = LIST_FIRST(&self->st_array.sa_list[num_items & self->st_array.sa_mask]);
		while (result && result->at_count != num_items)
			result = LIST_NEXT(result, at_chain);

		/* Check if we can re-use an existing type. */
		if (result && Dee_IncrefIfNotZero(DeeArrayType_AsType(result))) {
			DeeSType_CacheLockEndRead(self);
			return result;
		}
	}
	DeeSType_CacheLockEndRead(self);

	/* Construct a new array type. */
	result = arraytype_new(self, num_items);
	if unlikely(!result)
		goto done;

	/* Add the new type to the cache. */
register_type:
	DeeSType_CacheLockWrite(self);
	ASSERT(!self->st_array.sa_size ||
	       self->st_array.sa_mask);
	if (self->st_array.sa_size) {
		DREF DeeArrayTypeObject *new_result;
		new_result = LIST_FIRST(&self->st_array.sa_list[num_items & self->st_array.sa_mask]);
		while (new_result && new_result->at_count != num_items)
			new_result = LIST_NEXT(new_result, at_chain);

		/* Check if we can re-use an existing type. */
		if (new_result && Dee_IncrefIfNotZero(DeeArrayType_AsType(new_result))) {
			DeeSType_CacheLockEndRead(self);
			Dee_Decref(DeeArrayType_AsType(result));
			return new_result;
		}
	}

	/* Rehash when there are a lot of items. */
	if (self->st_array.sa_size >= self->st_array.sa_mask &&
	    (!stype_array_rehash(self, self->st_array.sa_mask ? (self->st_array.sa_mask << 1) | 1 : 16 - 1) &&
	     !self->st_array.sa_mask)) {

		/* No space at all! */
		DeeSType_CacheLockEndWrite(self);

		/* Collect enough memory for a 1-item hash map. */
		if (Dee_CollectMemory(sizeof(DeeArrayTypeObject *)))
			goto register_type;

		/* Failed to allocate the initial hash-map. */
		Dee_Decref(DeeArrayType_AsType(result));
		result = NULL;
		goto done;
	}

	/* Insert the new array type into the hash-map. */
	bucket = &self->st_array.sa_list[num_items & self->st_array.sa_mask];
	LIST_INSERT_HEAD(bucket, result, at_chain); /* Weak reference. */
	DeeSType_CacheLockEndWrite(self);
done:
	return result;
}



DECL_END


#endif /* !GUARD_DEX_CTYPES_ARRAY_C */
