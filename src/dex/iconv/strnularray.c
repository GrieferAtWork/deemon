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
#ifndef GUARD_DEX_ICONV_STRNULARRAY_C
#define GUARD_DEX_ICONV_STRNULARRAY_C 1
#define CONFIG_BUILDING_LIBICONV
#define DEE_SOURCE

#include "libiconv.h"
/**/

#include <deemon/api.h>

#include <deemon/alloc.h>           /* DeeObject_MALLOC, Dee_TYPE_CONSTRUCTOR_INIT_FIXED */
#include <deemon/object.h>          /* DREF, DeeObject, DeeObject_AssertTypeExact, DeeTypeObject, Dee_COMPARE_ERR, Dee_return_compare, ITER_DONE, OBJECT_HEAD_INIT */
#include <deemon/seq.h>             /* DeeIterator_Type, DeeSeq_Type */
#include <deemon/serial.h>          /* DeeSerial, DeeSerial_PutPointer, Dee_seraddr_t */
#include <deemon/string.h>          /* DeeString_NewUtf8, Dee_STRING_ERROR_FIGNORE */
#include <deemon/system-features.h> /* strend */
#include <deemon/type.h>            /* DeeObject_InitStatic, DeeType_Type, Dee_TYPE_CONSTRUCTOR_INIT_FIXED, TF_NONE, TP_FFINAL, TP_FNORMAL, TYPE_MEMBER_CONST, TYPE_MEMBER_END, type_* */
#include <deemon/util/atomic.h>     /* atomic_cmpxch_weak, atomic_read */

#include <stddef.h> /* NULL, offsetof, size_t */

DECL_BEGIN

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
snai_copy(StrNulArrayIterator *__restrict self,
          StrNulArrayIterator *__restrict other) {
	self->snai_iter = atomic_read(&other->snai_iter);
	return 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
snai_serialize(StrNulArrayIterator *__restrict self,
               DeeSerial *__restrict writer,
               Dee_seraddr_t addr) {
#define ADDROF(field) (addr + offsetof(StrNulArrayIterator, field))
	return DeeSerial_PutPointer(writer, ADDROF(snai_iter), atomic_read(&self->snai_iter));
#undef ADDROF
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
snai_bool(StrNulArrayIterator *__restrict self) {
	char const *ptr = atomic_read(&self->snai_iter);
	return *ptr != '\0' ? 1 : 0;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
snai_compare(StrNulArrayIterator *lhs,
             StrNulArrayIterator *rhs) {
	char const *lhs_ptr, *rhs_ptr;
	if (DeeObject_AssertTypeExact(rhs, &StrNulArrayIterator_Type))
		goto err;
	lhs_ptr = atomic_read(&lhs->snai_iter);
	rhs_ptr = atomic_read(&rhs->snai_iter);
	Dee_return_compare(lhs_ptr, rhs_ptr);
err:
	return Dee_COMPARE_ERR;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
snai_next(StrNulArrayIterator *__restrict self) {
	char const *oldptr, *newptr;
	do {
		oldptr = atomic_read(&self->snai_iter);
		if (*oldptr == '\0')
			return ITER_DONE;
		newptr = strend(oldptr) + 1;
	} while (!atomic_cmpxch_weak(&self->snai_iter, oldptr, newptr));
	return DeeString_New(oldptr);
}

PRIVATE struct type_cmp snai_cmp = {
	/* .tp_hash       = */ NULL,
	/* .tp_compare_eq = */ NULL,
	/* .tp_compare    = */ (int (DCALL *)(DeeObject *, DeeObject *))&snai_compare,
};

INTERN DeeTypeObject StrNulArrayIterator_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_StrNulArrayIterator",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeIterator_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ StrNulArrayIterator,
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ &snai_copy,
			/* tp_any_ctor:    */ NULL,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &snai_serialize
		),
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ (int (DCALL *)(DeeObject *__restrict))&snai_bool
	},
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &snai_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&snai_next,
	/* .tp_iterator      = */ NULL,
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



STATIC_ASSERT(offsetof(StrNulArrayIterator, snai_iter) == offsetof(StrNulArray, sna_base));
#define sna_copy      snai_copy
#define sna_serialize snai_serialize

PRIVATE WUNUSED NONNULL((1)) DREF StrNulArrayIterator *DCALL
sna_iter(StrNulArray *__restrict self) {
	DREF StrNulArrayIterator *result;
	result = DeeObject_MALLOC(StrNulArrayIterator);
	if unlikely(!result)
		goto err;
	result->snai_iter = self->sna_base;
	DeeObject_InitStatic(result, &StrNulArrayIterator_Type);
	return result;
err:
	return NULL;
}

PRIVATE struct type_seq sna_seq = {
	/* .tp_iter = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&sna_iter,
};

PRIVATE struct type_member tpconst sna_class_members[] = {
	TYPE_MEMBER_CONST("Iterator", &StrNulArrayIterator_Type),
	TYPE_MEMBER_END
};

INTERN DeeTypeObject StrNulArray_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_StrNulArray",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeSeq_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
			/* T:              */ StrNulArray,
			/* tp_ctor:        */ NULL,
			/* tp_copy_ctor:   */ &sna_copy,
			/* tp_any_ctor:    */ NULL,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &sna_serialize
		),
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ NULL,
	/* .tp_seq           = */ &sna_seq,
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
	/* .tp_class_members = */ sna_class_members
};

DECL_END


#endif /* !GUARD_DEX_ICONV_STRNULARRAY_C */
