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
#define DEE_SOURCE
#ifdef __INTELLISENSE__
#define N 1
#endif /* __INTELLISENSE__ */

#ifndef N
#error "Must #define N before including this file"
#endif /* !N */

#include "libctypes.h"
/**/

#include <deemon/api.h>

#include <deemon/alloc.h>
#include <deemon/int.h>
#include <deemon/none.h>
#include <deemon/object.h>

#include <hybrid/typecore.h>

#include <stddef.h> /* NULL, ptrdiff_t */
#include <stdint.h> /* int32_t, int64_t, uint32_t, uint64_t, uintptr_t */

#undef byte_t
#define byte_t __BYTE_TYPE__

DECL_BEGIN

#if N == 1
#define POINTER_MATH            pointer_math1
#define POINTER_SEQ             pointer_seq1
#define F(x)                    x##1
#define ITEM_SIZE(pointer_type) 1
#else /* N == ... */
#define POINTER_MATH            pointer_mathn
#define POINTER_SEQ             pointer_seqn
#define F(x)                    x##n
#define ITEM_SIZE(pointer_type) ((pointer_type)->pt_size)
#endif /* N != ... */

#ifndef GENERIC_POINTER_MATH_DEFINED
#define GENERIC_POINTER_MATH_DEFINED 1
PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
pointer_int32(DeePointerTypeObject *__restrict UNUSED(tp_self),
              union pointer *self,
              int32_t *__restrict result) {
	CTYPES_FAULTPROTECT(*(uint32_t *)result = (uint32_t)self->uint, return -1);
	return INT_UNSIGNED;
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
pointer_int64(DeePointerTypeObject *__restrict UNUSED(tp_self),
              union pointer *self,
              int64_t *__restrict result) {
	CTYPES_FAULTPROTECT(*(uint64_t *)result = (uint64_t)self->uint, return -1);
	return INT_UNSIGNED;
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
pointer_double(DeePointerTypeObject *__restrict UNUSED(tp_self),
               union pointer *self,
               double *__restrict result) {
	CTYPES_FAULTPROTECT(*result = (double)self->uint, return -1);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
pointer_int(DeePointerTypeObject *__restrict UNUSED(tp_self),
            union pointer *self) {
	uintptr_t value;
	CTYPES_FAULTPROTECT(value = self->uint, return NULL);
	return DeeInt_NewUIntptr(value);
}
#endif /* !GENERIC_POINTER_MATH_DEFINED */


PRIVATE WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL
F(pointer_add)(DeePointerTypeObject *tp_self,
               union pointer *self,
               DeeObject *some_object) {
	ptrdiff_t other;
	union pointer value;
	CTYPES_FAULTPROTECT(value.ptr = self->ptr,
	                    return NULL);
	if (DeeObject_AsPtrdiff(some_object, &other))
		goto err;
#if N == 1
	return DeePointer_New(tp_self, (void *)(value.uint + other));
#else /* N == 1 */
	return DeePointer_New(tp_self, (void *)(value.uint + other * tp_self->pt_size));
#endif /* N != 1 */
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 3)) DREF DeeObject *DCALL
F(pointer_sub)(DeePointerTypeObject *tp_self,
               union pointer *self,
               DeeObject *some_object) {
	ptrdiff_t other;
	union pointer value;
	CTYPES_FAULTPROTECT(value.ptr = self->ptr,
	                    goto err);
	if (DeePointer_Check(some_object)) {
		if (DeeType_AsPointerType(Dee_TYPE(some_object))->pt_orig !=
		    tp_self->pt_orig) {
			DeeObject_TypeAssertFailed(some_object, DeePointerType_AsType(tp_self));
			goto err;
		}
		value.uint -= ((struct pointer_object *)some_object)->p_ptr.uint;
#if N != 1
		value.uint /= tp_self->pt_size;
#endif /* N != 1 */
		return DeePointer_New(tp_self, value.ptr);
	}
	if (DeeObject_AsPtrdiff(some_object, &other))
		goto err;
#if N == 1
	return DeePointer_New(tp_self, (void *)(value.uint - other));
#else /* N == 1 */
	return DeePointer_New(tp_self, (void *)(value.uint - other * tp_self->pt_size));
#endif /* N != 1 */
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
F(pointer_inc)(DeePointerTypeObject *__restrict tp_self,
               union pointer *self) {
	(void)tp_self;
	(void)self;
#if N == 1
	CTYPES_FAULTPROTECT(++self->uint, return -1);
#else /* N == 1 */
	CTYPES_FAULTPROTECT(self->uint += ITEM_SIZE(tp_self),
	                    return -1);
#endif /* N != 1 */
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
F(pointer_dec)(DeePointerTypeObject *__restrict tp_self,
               union pointer *self) {
	(void)tp_self;
	(void)self;
#if N == 1
	CTYPES_FAULTPROTECT(--self->uint, return -1);
#else /* N == 1 */
	CTYPES_FAULTPROTECT(self->uint -= ITEM_SIZE(tp_self),
	                    return -1);
#endif /* N != 1 */
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
F(pointer_inplace_add)(DeePointerTypeObject *tp_self,
                       union pointer *self,
                       DeeObject *some_object) {
	ptrdiff_t other;
	(void)tp_self;
	(void)self;
	if (DeeObject_AsPtrdiff(some_object, &other))
		goto err;
#if N != 1
	other *= tp_self->pt_size;
#endif /* N != 1 */
	CTYPES_FAULTPROTECT(self->uint += other,
	                    goto err);
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
F(pointer_inplace_sub)(DeePointerTypeObject *tp_self,
                       union pointer *self,
                       DeeObject *some_object) {
	ptrdiff_t other;
	(void)tp_self;
	(void)self;
	if (DeeObject_AsPtrdiff(some_object, &other))
		goto err;
#if N != 1
	other *= tp_self->pt_size;
#endif /* N != 1 */
	CTYPES_FAULTPROTECT(self->uint -= other,
	                    goto err);
	return 0;
err:
	return -1;
}


PRIVATE WUNUSED NONNULL((1, 3)) DREF struct lvalue_object *DCALL
F(pointer_getitem)(DeePointerTypeObject *tp_self, union pointer *self,
                   DeeObject *index_ob) {
	byte_t *result;
	ptrdiff_t index;
	if (DeeObject_AsPtrdiff(index_ob, &index))
		goto err;
#if N != 1
	index *= tp_self->pt_size;
#endif /* N != 1 */
	CTYPES_FAULTPROTECT(result = (__BYTE_TYPE__ *)self->ptr + index, goto err);
	return (DREF struct lvalue_object *)DeeLValue_NewFor(tp_self->pt_orig, result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 3, 4)) int DCALL
F(pointer_setitem)(DeePointerTypeObject *tp_self, union pointer *self,
                   DeeObject *index_ob, DeeObject *value) {
	void *ptr;
	ptrdiff_t index;
	if (DeeObject_AsPtrdiff(index_ob, &index))
		goto err;
#if N != 1
	index *= tp_self->pt_size;
#endif /* N != 1 */
	CTYPES_FAULTPROTECT(ptr = (__BYTE_TYPE__ *)self->ptr + index, goto err);
	return DeeStruct_Assign(tp_self->pt_orig, ptr, value);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 3)) int DCALL
F(pointer_delitem)(DeePointerTypeObject *tp_self, union pointer *self,
                   DeeObject *index_ob) {
	return F(pointer_setitem)(tp_self, self, index_ob, Dee_None);
}



INTERN struct stype_math POINTER_MATH = {
	/* .st_int32       = */ (int (DCALL *)(DeeSTypeObject *__restrict, void *, int32_t *__restrict))&pointer_int32,
	/* .st_int64       = */ (int (DCALL *)(DeeSTypeObject *__restrict, void *, int64_t *__restrict))&pointer_int64,
	/* .st_double      = */ (int (DCALL *)(DeeSTypeObject *__restrict, void *, double *__restrict))&pointer_double,
	/* .st_int         = */ (DREF DeeObject *(DCALL *)(DeeSTypeObject *__restrict, void *))&pointer_int,
	/* .st_inv         = */ (DREF DeeObject *(DCALL *)(DeeSTypeObject *__restrict, void *))NULL,
	/* .st_pos         = */ (DREF DeeObject *(DCALL *)(DeeSTypeObject *__restrict, void *))NULL,
	/* .st_neg         = */ (DREF DeeObject *(DCALL *)(DeeSTypeObject *__restrict, void *))NULL,
	/* .st_add         = */ (DREF DeeObject *(DCALL *)(DeeSTypeObject *, void *, DeeObject *))&F(pointer_add),
	/* .st_sub         = */ (DREF DeeObject *(DCALL *)(DeeSTypeObject *, void *, DeeObject *))&F(pointer_sub),
	/* .st_mul         = */ NULL,
	/* .st_div         = */ NULL,
	/* .st_mod         = */ NULL,
	/* .st_shl         = */ NULL,
	/* .st_shr         = */ NULL,
	/* .st_and         = */ NULL,
	/* .st_or          = */ NULL,
	/* .st_xor         = */ NULL,
	/* .st_pow         = */ NULL,
	/* .st_inc         = */ (int (DCALL *)(DeeSTypeObject *__restrict, void *))&F(pointer_inc),
	/* .st_dec         = */ (int (DCALL *)(DeeSTypeObject *__restrict, void *))&F(pointer_dec),
	/* .st_inplace_add = */ (int (DCALL *)(DeeSTypeObject *, void *, DeeObject *))&F(pointer_inplace_add),
	/* .st_inplace_sub = */ (int (DCALL *)(DeeSTypeObject *, void *, DeeObject *))&F(pointer_inplace_sub),
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

INTERN struct stype_seq POINTER_SEQ = {
	/* .st_iter_self = */ NULL,
	/* .st_size      = */ NULL,
	/* .st_contains  = */ NULL,
	/* .st_get       = */ (DREF DeeObject *(DCALL *)(DeeSTypeObject *, void *, DeeObject *))&F(pointer_getitem),
	/* .st_del       = */ (int (DCALL *)(DeeSTypeObject *, void *, DeeObject *))&F(pointer_delitem),
	/* .st_set       = */ (int (DCALL *)(DeeSTypeObject *, void *, DeeObject *, DeeObject *))&F(pointer_setitem),
	/* .st_range_get = */ NULL,
	/* .st_range_del = */ NULL,
	/* .st_range_set = */ NULL,
};

#undef ITEM_SIZE
#undef POINTER_SEQ
#undef POINTER_MATH
#undef F
#undef N

DECL_END
