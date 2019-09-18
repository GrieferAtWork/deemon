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
#ifndef GUARD_DEX_CTYPES_POINTER_C
#define GUARD_DEX_CTYPES_POINTER_C 1
#define DEE_SOURCE 1
#define _KOS_SOURCE 1

#include "libctypes.h"
/**/

#include <deemon/alloc.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/bytes.h>
#include <deemon/gc.h>
#include <deemon/int.h>
#include <deemon/none.h>
#include <deemon/string.h>
#include <deemon/super.h>

DECL_BEGIN


PRIVATE DREF DeeObject *DCALL
pointer_str(DeePointerTypeObject *__restrict UNUSED(tp_self),
            union pointer *self) {
	union pointer value;
	CTYPES_FAULTPROTECT(value.ptr = self->ptr,
	                    return NULL);
	return DeeString_Newf("%p", value.ptr);
}

PRIVATE DREF DeeObject *DCALL
pointer_repr(DeePointerTypeObject *__restrict tp_self,
             union pointer *self) {
	union pointer value;
	CTYPES_FAULTPROTECT(value.ptr = self->ptr,
	                    return NULL);
	return DeeString_Newf("(%k)%p", tp_self, value.ptr);
}

PRIVATE int DCALL
pointer_bool(DeePointerTypeObject *__restrict UNUSED(tp_self),
             union pointer *self) {
	union pointer value;
	CTYPES_FAULTPROTECT(value.ptr = self->ptr,
	                    return -1);
	return value.ptr != NULL;
}

#define DEFINE_POINTER_COMPARE(name, op)                          \
	PRIVATE DREF DeeObject *DCALL                                 \
	name(DeePointerTypeObject *__restrict tp_self,                \
	     union pointer *self,                                     \
	     DeeObject *__restrict other) {                           \
		union pointer value;                                      \
		if (DeeObject_AsPointer(other, tp_self->pt_orig, &value)) \
			return NULL;                                          \
		return_bool_(self->ptr op value.ptr);                     \
	}
DEFINE_POINTER_COMPARE(pointer_eq, ==)
DEFINE_POINTER_COMPARE(pointer_ne, !=)
DEFINE_POINTER_COMPARE(pointer_lo, <)
DEFINE_POINTER_COMPARE(pointer_le, <=)
DEFINE_POINTER_COMPARE(pointer_gr, >)
DEFINE_POINTER_COMPARE(pointer_ge, >=)
#undef DEFINE_POINTER_COMPARE

PRIVATE struct stype_cmp pointer_cmp = {
	/* .st_eq = */ (DREF DeeObject *(DCALL *)(DeeSTypeObject *__restrict, void *, DeeObject *__restrict))&pointer_eq,
	/* .st_ne = */ (DREF DeeObject *(DCALL *)(DeeSTypeObject *__restrict, void *, DeeObject *__restrict))&pointer_ne,
	/* .st_lo = */ (DREF DeeObject *(DCALL *)(DeeSTypeObject *__restrict, void *, DeeObject *__restrict))&pointer_lo,
	/* .st_le = */ (DREF DeeObject *(DCALL *)(DeeSTypeObject *__restrict, void *, DeeObject *__restrict))&pointer_le,
	/* .st_gr = */ (DREF DeeObject *(DCALL *)(DeeSTypeObject *__restrict, void *, DeeObject *__restrict))&pointer_gr,
	/* .st_ge = */ (DREF DeeObject *(DCALL *)(DeeSTypeObject *__restrict, void *, DeeObject *__restrict))&pointer_ge
};


PRIVATE int DCALL
pointer_init(DeePointerTypeObject *__restrict tp_self,
             union pointer *self,
             size_t argc, DeeObject **__restrict argv) {
	DeeObject *arg;
	union pointer value;
	if (DeeArg_Unpack(argc, argv, "o:pointer", &arg))
		goto err;
	if (DeeNone_Check(arg)) {
		/* none is the NULL pointer. */
		value.ptr = NULL;
		goto done;
	}
	if (DeePointer_Check(arg)) {
		/* Pointer to pointer cast. */
		value.ptr = ((struct pointer_object *)arg)->p_ptr.ptr;
		goto done;
	}
	/* Special handling for strings (which can be cast to `char *') */
	if (DeeString_Check(arg)) {
		if (tp_self->pt_orig == &DeeCChar_Type) {
			value.ptr = DeeString_AsUtf8(arg);
			if unlikely(!value.ptr)
				goto err;
			goto done;
		}
		if (tp_self->pt_orig == &DeeCWChar_Type) {
			value.ptr = DeeString_AsWide(arg);
			if unlikely(!value.ptr)
				goto err;
			goto done;
		}
		if (tp_self->pt_orig == &DeeCChar16_Type) {
			value.ptr = DeeString_AsUtf16(arg, STRING_ERROR_FREPLAC);
			if unlikely(!value.ptr)
				goto err;
			goto done;
		}
		if (tp_self->pt_orig == &DeeCChar32_Type) {
			value.ptr = DeeString_AsUtf32(arg);
			if unlikely(!value.ptr)
				goto err;
			goto done;
		}
		/* Default case: interpret the UTF-8/byte representation. */
		value.pchar = DeeString_STR(arg);
		goto done;
	}
	if (DeeBytes_Check(arg)) {
		value.ptr = DeeBytes_DATA(arg);
		goto done;
	}
	/* Fallback: cast to integer. */
	if (DeeObject_AsUIntptr(arg, &value.uint))
		goto err;
done:
	/* Fill in the pointer. */
	CTYPES_FAULTPROTECT(self->ptr = value.ptr,
	                    goto err);
	return 0;
err:
	return -1;
}

PRIVATE int DCALL
pointer_assign(DeePointerTypeObject *__restrict tp_self,
               union pointer *self, DeeObject *__restrict arg) {
	union pointer value;
	if (DeeObject_AsPointer(arg, tp_self->pt_orig, &value))
		goto err;
	CTYPES_FAULTPROTECT(self->ptr = value.ptr, goto err);
	return 0;
err:
	return -1;
}

PRIVATE DREF DeeObject *DCALL
pointer_call(DeePointerTypeObject *__restrict tp_self,
             union pointer *self, size_t argc,
             DeeObject **__restrict argv) {
	union pointer ptr;
	CTYPES_FAULTPROTECT(ptr.ptr = self->ptr, return NULL);
	return DeeStruct_Call(tp_self->pt_orig, ptr.ptr, argc, argv);
}

PRIVATE DREF struct lvalue_object *DCALL
pointer_get_deref(struct pointer_object *__restrict self) {
	DREF struct lvalue_object *result;
	DREF DeeLValueTypeObject *type;
	result = DeeObject_MALLOC(struct lvalue_object);
	if unlikely(!result)
		goto done;
	/* Lookup the l-value version of the base-type. */
	type = DeeSType_LValue(((DeePointerTypeObject *)Dee_TYPE(self))->pt_orig);
	if unlikely(!type)
		goto err_r;
	/* Initialize the new l-value object. */
	DeeObject_InitNoref(result, (DeeTypeObject *)type);
	result->l_ptr.ptr = self->p_ptr.ptr;
done:
	return result;
err_r:
	DeeObject_FREE(result);
	return NULL;
}

PRIVATE int DCALL
pointer_set_deref(struct pointer_object *__restrict self,
                  DeeObject *__restrict value) {
	return DeeStruct_Assign(((DeePointerTypeObject *)Dee_TYPE(self))->pt_orig,
	                        self->p_ptr.ptr, value);
}

PRIVATE int DCALL
pointer_del_deref(struct pointer_object *__restrict self) {
	return pointer_set_deref(self, Dee_None);
}

PRIVATE struct type_getset pointer_getsets[] = {
	{ "ind",
	  (DREF DeeObject * (DCALL *)(DeeObject * __restrict))&pointer_get_deref,
	  (int(DCALL *)(DeeObject *__restrict))&pointer_del_deref,
	  (int(DCALL *)(DeeObject *__restrict, DeeObject *__restrict))&pointer_set_deref,
	  DOC("->?GLValue\nGet/clear/set the dereferenced memory location of @this pointer") },
	{ NULL }
};


#ifndef CONFIG_NO_DEEMON_100_COMPAT
PRIVATE DREF DeeObject *DCALL
struct_deref_func(DeeObject *__restrict self,
                  size_t argc, DeeObject **__restrict argv) {
	if (DeeArg_Unpack(argc, argv, ":__deref__"))
		goto err;
	return (DREF DeeObject *)pointer_get_deref((struct pointer_object *)self);
err:
	return NULL;
}

PRIVATE struct type_method pointer_methods[] = {
	/* Methods for backwards-compatibility with deemon 100+ */
	{ "__deref__", &struct_deref_func, DOC("->pointer\nDeprecated alias for #ind") },
	{ NULL }
};
#endif /* !CONFIG_NO_DEEMON_100_COMPAT */

INTERN DeePointerTypeObject DeePointer_Type = {
	/* .pt_base = */ {
		/* .st_base = */ {
			OBJECT_HEAD_INIT((DeeTypeObject *)&DeePointerType_Type),
			/* .tp_name     = */ "Pointer",
			/* .tp_doc      = */ NULL,
			/* .tp_flags    = */ TP_FNORMAL | TP_FTRUNCATE | TP_FMOVEANY | TP_FINHERITCTOR,
			/* .tp_weakrefs = */ 0,
			/* .tp_features = */ TF_NONE,
			/* .tp_base     = */ (DeeTypeObject *)&DeeStructured_Type,
			/* .tp_init = */ {
				{
					/* .tp_alloc = */ {
						/* .tp_ctor      = */ NULL,
						/* .tp_copy_ctor = */ NULL,
						/* .tp_deep_ctor = */ NULL,
						/* .tp_any_ctor  = */ NULL,
						TYPE_FIXED_ALLOCATOR(struct pointer_object)
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
			/* .tp_attr          = */ NULL,
			/* .tp_with          = */ NULL,
			/* .tp_buffer        = */ NULL,
#ifndef CONFIG_NO_DEEMON_100_COMPAT
			/* .tp_methods       = */ pointer_methods,
#else /* !CONFIG_NO_DEEMON_100_COMPAT */
			/* .tp_methods       = */ NULL,
#endif /* CONFIG_NO_DEEMON_100_COMPAT */
			/* .tp_getsets       = */ pointer_getsets,
			/* .tp_members       = */ NULL,
			/* .tp_class_methods = */ NULL,
			/* .tp_class_getsets = */ NULL,
			/* .tp_class_members = */ NULL
		},
#ifndef CONFIG_NO_THREADS
		/* .st_cachelock = */ RWLOCK_INIT,
#endif
		/* .st_pointer  = */ &DeePointer_Type,
		/* .st_lvalue   = */ &DeeLValue_Type,
		/* .st_array    = */ STYPE_ARRAY_INIT,
#ifndef CONFIG_NO_CFUNCTION
		/* .st_cfunction= */ STYPE_CFUNCTION_INIT,
		/* .st_ffitype  = */ &ffi_type_pointer,
#endif /* !CONFIG_NO_CFUNCTION */
		/* .st_align    = */ CONFIG_CTYPES_ALIGNOF_POINTER,
		/* .st_init     = */ (int(DCALL *)(DeeSTypeObject *__restrict, void *, size_t, DeeObject **__restrict))&pointer_init,
		/* .st_assign   = */ (int(DCALL *)(DeeSTypeObject *__restrict, void *, DeeObject *__restrict))&pointer_assign,
		/* .st_cast     = */ {
			/* .st_str  = */ (DREF DeeObject *(DCALL *)(DeeSTypeObject *__restrict, void *))&pointer_str,
			/* .st_repr = */ (DREF DeeObject *(DCALL *)(DeeSTypeObject *__restrict, void *))&pointer_repr,
			/* .st_bool = */ (int(DCALL *)(DeeSTypeObject *__restrict, void *))&pointer_bool
		},
		/* .st_call     = */ (DREF DeeObject *(DCALL *)(DeeSTypeObject *__restrict, void *, size_t, DeeObject **__restrict))&pointer_call,
		/* .st_math     = */ &pointer_math0,
		/* .st_cmp      = */ &pointer_cmp,
		/* .st_seq      = */ NULL,
		/* .st_attr     = */ NULL
	},
	/* .pt_orig = */ &DeeStructured_Type,
	/* .pt_size = */ 0
};


PRIVATE int DCALL
lvalue_int32(DeeLValueTypeObject *__restrict tp_self,
             union pointer *self,
             int32_t *__restrict result) {
	union pointer ptr;
	CTYPES_FAULTPROTECT(ptr.ptr = self->ptr, return -1);
	return DeeStruct_Int32(tp_self->lt_orig, ptr.ptr, result);
}

PRIVATE int DCALL
lvalue_int64(DeeLValueTypeObject *__restrict tp_self,
             union pointer *self,
             int64_t *__restrict result) {
	union pointer ptr;
	CTYPES_FAULTPROTECT(ptr.ptr = self->ptr, return -1);
	return DeeStruct_Int64(tp_self->lt_orig, ptr.ptr, result);
}

PRIVATE int DCALL
lvalue_double(DeeLValueTypeObject *__restrict tp_self,
              union pointer *self,
              double *__restrict result) {
	union pointer ptr;
	CTYPES_FAULTPROTECT(ptr.ptr = self->ptr, return -1);
	return DeeStruct_Double(tp_self->lt_orig, ptr.ptr, result);
}

#define DEFINE_UNARY_LVALUE_OPERATOR(Treturn, error_return, lvalue_xxx, DeeStruct_Xxx) \
	PRIVATE Treturn DCALL                                                              \
	lvalue_xxx(DeeLValueTypeObject *__restrict tp_self,                                \
	           union pointer *self) {                                                  \
		union pointer ptr;                                                             \
		CTYPES_FAULTPROTECT(ptr.ptr = self->ptr, return error_return);                 \
		return DeeStruct_Xxx(tp_self->lt_orig, ptr.ptr);                               \
	}
#define DEFINE_BINARY_LVALUE_OPERATOR(Treturn, error_return, lvalue_xxx, DeeStruct_Xxx) \
	PRIVATE Treturn DCALL                                                               \
	lvalue_xxx(DeeLValueTypeObject *__restrict tp_self,                                 \
	           union pointer *self,                                                     \
	           DeeObject *__restrict other) {                                           \
		union pointer ptr;                                                              \
		CTYPES_FAULTPROTECT(ptr.ptr = self->ptr, return error_return);                  \
		return DeeStruct_Xxx(tp_self->lt_orig, ptr.ptr, other);                         \
	}
#define DEFINE_TRINARY_LVALUE_OPERATOR(Treturn, error_return, lvalue_xxx, DeeStruct_Xxx) \
	PRIVATE Treturn DCALL                                                                \
	lvalue_xxx(DeeLValueTypeObject *__restrict tp_self,                                  \
	           union pointer *self,                                                      \
	           DeeObject *__restrict a, DeeObject *__restrict b) {                       \
		union pointer ptr;                                                               \
		CTYPES_FAULTPROTECT(ptr.ptr = self->ptr, return error_return);                   \
		return DeeStruct_Xxx(tp_self->lt_orig, ptr.ptr, a, b);                           \
	}
DEFINE_BINARY_LVALUE_OPERATOR(int, -1, lvalue_assign, DeeStruct_Assign)
DEFINE_UNARY_LVALUE_OPERATOR(int, -1, lvalue_bool, DeeStruct_Bool)
DEFINE_UNARY_LVALUE_OPERATOR(DREF DeeObject *, NULL, lvalue_int, DeeStruct_Int)
DEFINE_UNARY_LVALUE_OPERATOR(DREF DeeObject *, NULL, lvalue_inv, DeeStruct_Inv)
DEFINE_UNARY_LVALUE_OPERATOR(DREF DeeObject *, NULL, lvalue_pos, DeeStruct_Pos)
DEFINE_UNARY_LVALUE_OPERATOR(DREF DeeObject *, NULL, lvalue_neg, DeeStruct_Neg)
DEFINE_UNARY_LVALUE_OPERATOR(DREF DeeObject *, NULL, lvalue_str, DeeStruct_Str)
DEFINE_UNARY_LVALUE_OPERATOR(DREF DeeObject *, NULL, lvalue_repr, DeeStruct_Repr)
DEFINE_BINARY_LVALUE_OPERATOR(DREF DeeObject *, NULL, lvalue_add, DeeStruct_Add)
DEFINE_BINARY_LVALUE_OPERATOR(DREF DeeObject *, NULL, lvalue_sub, DeeStruct_Sub)
DEFINE_BINARY_LVALUE_OPERATOR(DREF DeeObject *, NULL, lvalue_mul, DeeStruct_Mul)
DEFINE_BINARY_LVALUE_OPERATOR(DREF DeeObject *, NULL, lvalue_div, DeeStruct_Div)
DEFINE_BINARY_LVALUE_OPERATOR(DREF DeeObject *, NULL, lvalue_mod, DeeStruct_Mod)
DEFINE_BINARY_LVALUE_OPERATOR(DREF DeeObject *, NULL, lvalue_shl, DeeStruct_Shl)
DEFINE_BINARY_LVALUE_OPERATOR(DREF DeeObject *, NULL, lvalue_shr, DeeStruct_Shr)
DEFINE_BINARY_LVALUE_OPERATOR(DREF DeeObject *, NULL, lvalue_and, DeeStruct_And)
DEFINE_BINARY_LVALUE_OPERATOR(DREF DeeObject *, NULL, lvalue_or, DeeStruct_Or)
DEFINE_BINARY_LVALUE_OPERATOR(DREF DeeObject *, NULL, lvalue_xor, DeeStruct_Xor)
DEFINE_BINARY_LVALUE_OPERATOR(DREF DeeObject *, NULL, lvalue_pow, DeeStruct_Pow)
DEFINE_UNARY_LVALUE_OPERATOR(int, -1, lvalue_inc, DeeStruct_Inc)
DEFINE_UNARY_LVALUE_OPERATOR(int, -1, lvalue_dec, DeeStruct_Dec)
DEFINE_BINARY_LVALUE_OPERATOR(int, -1, lvalue_inplace_add, DeeStruct_InplaceAdd)
DEFINE_BINARY_LVALUE_OPERATOR(int, -1, lvalue_inplace_sub, DeeStruct_InplaceSub)
DEFINE_BINARY_LVALUE_OPERATOR(int, -1, lvalue_inplace_mul, DeeStruct_InplaceMul)
DEFINE_BINARY_LVALUE_OPERATOR(int, -1, lvalue_inplace_div, DeeStruct_InplaceDiv)
DEFINE_BINARY_LVALUE_OPERATOR(int, -1, lvalue_inplace_mod, DeeStruct_InplaceMod)
DEFINE_BINARY_LVALUE_OPERATOR(int, -1, lvalue_inplace_shl, DeeStruct_InplaceShl)
DEFINE_BINARY_LVALUE_OPERATOR(int, -1, lvalue_inplace_shr, DeeStruct_InplaceShr)
DEFINE_BINARY_LVALUE_OPERATOR(int, -1, lvalue_inplace_and, DeeStruct_InplaceAnd)
DEFINE_BINARY_LVALUE_OPERATOR(int, -1, lvalue_inplace_or, DeeStruct_InplaceOr)
DEFINE_BINARY_LVALUE_OPERATOR(int, -1, lvalue_inplace_xor, DeeStruct_InplaceXor)
DEFINE_BINARY_LVALUE_OPERATOR(int, -1, lvalue_inplace_pow, DeeStruct_InplacePow)
DEFINE_BINARY_LVALUE_OPERATOR(DREF DeeObject *, NULL, lvalue_eq, DeeStruct_Eq)
DEFINE_BINARY_LVALUE_OPERATOR(DREF DeeObject *, NULL, lvalue_ne, DeeStruct_Ne)
DEFINE_BINARY_LVALUE_OPERATOR(DREF DeeObject *, NULL, lvalue_lo, DeeStruct_Lo)
DEFINE_BINARY_LVALUE_OPERATOR(DREF DeeObject *, NULL, lvalue_le, DeeStruct_Le)
DEFINE_BINARY_LVALUE_OPERATOR(DREF DeeObject *, NULL, lvalue_gr, DeeStruct_Gr)
DEFINE_BINARY_LVALUE_OPERATOR(DREF DeeObject *, NULL, lvalue_ge, DeeStruct_Ge)
DEFINE_UNARY_LVALUE_OPERATOR(DREF DeeObject *, NULL, lvalue_iter, DeeStruct_IterSelf)
DEFINE_UNARY_LVALUE_OPERATOR(DREF DeeObject *, NULL, lvalue_size, DeeStruct_GetSize)
DEFINE_BINARY_LVALUE_OPERATOR(DREF DeeObject *, NULL, lvalue_contains, DeeStruct_Contains)
DEFINE_BINARY_LVALUE_OPERATOR(DREF DeeObject *, NULL, lvalue_getitem, DeeStruct_GetItem)
DEFINE_BINARY_LVALUE_OPERATOR(int, -1, lvalue_delitem, DeeStruct_DelItem)
DEFINE_TRINARY_LVALUE_OPERATOR(int, -1, lvalue_setitem, DeeStruct_SetItem)
DEFINE_TRINARY_LVALUE_OPERATOR(DREF DeeObject *, NULL, lvalue_getrange, DeeStruct_GetRange)
DEFINE_TRINARY_LVALUE_OPERATOR(int, -1, lvalue_delrange, DeeStruct_DelRange)

PRIVATE int DCALL
lvalue_setrange(DeeLValueTypeObject *__restrict tp_self,
                union pointer *self, DeeObject *__restrict begin,
                DeeObject *__restrict end, DeeObject *__restrict value) {
	union pointer ptr;
	CTYPES_FAULTPROTECT(ptr.ptr = self->ptr, return -1);
	return DeeStruct_SetRange(tp_self->lt_orig, ptr.ptr, begin, end, value);
}

DEFINE_BINARY_LVALUE_OPERATOR(DREF DeeObject *, NULL, lvalue_getattr, DeeStruct_GetAttr)
DEFINE_BINARY_LVALUE_OPERATOR(int, -1, lvalue_delattr, DeeStruct_DelAttr)
DEFINE_TRINARY_LVALUE_OPERATOR(int, -1, lvalue_setattr, DeeStruct_SetAttr)

PRIVATE dssize_t DCALL
lvalue_enumattr(DeeLValueTypeObject *__restrict tp_self,
                denum_t proc, void *arg) {
	return DeeStruct_EnumAttr(tp_self->lt_orig, proc, arg);
}

/* Emulating C, the call operator of a pointer automatically dereferences its pointer. */
#define lvalue_call pointer_call
#undef DEFINE_TRINARY_LVALUE_OPERATOR
#undef DEFINE_BINARY_LVALUE_OPERATOR
#undef DEFINE_UNARY_LVALUE_OPERATOR


PRIVATE struct stype_math lvalue_math = {
	/* .st_int32       = */ (int(DCALL *)(DeeSTypeObject *__restrict, void *, int32_t *__restrict))&lvalue_int32,
	/* .st_int64       = */ (int(DCALL *)(DeeSTypeObject *__restrict, void *, int64_t *__restrict))&lvalue_int64,
	/* .st_double      = */ (int(DCALL *)(DeeSTypeObject *__restrict, void *, double *__restrict))&lvalue_double,
	/* .st_int         = */ (DREF DeeObject *(DCALL *)(DeeSTypeObject *__restrict, void *))&lvalue_int,
	/* .st_inv         = */ (DREF DeeObject *(DCALL *)(DeeSTypeObject *__restrict, void *))&lvalue_inv,
	/* .st_pos         = */ (DREF DeeObject *(DCALL *)(DeeSTypeObject *__restrict, void *))&lvalue_pos,
	/* .st_neg         = */ (DREF DeeObject *(DCALL *)(DeeSTypeObject *__restrict, void *))&lvalue_neg,
	/* .st_add         = */ (DREF DeeObject *(DCALL *)(DeeSTypeObject *__restrict, void *, DeeObject *__restrict))&lvalue_add,
	/* .st_sub         = */ (DREF DeeObject *(DCALL *)(DeeSTypeObject *__restrict, void *, DeeObject *__restrict))&lvalue_sub,
	/* .st_mul         = */ (DREF DeeObject *(DCALL *)(DeeSTypeObject *__restrict, void *, DeeObject *__restrict))&lvalue_mul,
	/* .st_div         = */ (DREF DeeObject *(DCALL *)(DeeSTypeObject *__restrict, void *, DeeObject *__restrict))&lvalue_div,
	/* .st_mod         = */ (DREF DeeObject *(DCALL *)(DeeSTypeObject *__restrict, void *, DeeObject *__restrict))&lvalue_mod,
	/* .st_shl         = */ (DREF DeeObject *(DCALL *)(DeeSTypeObject *__restrict, void *, DeeObject *__restrict))&lvalue_shl,
	/* .st_shr         = */ (DREF DeeObject *(DCALL *)(DeeSTypeObject *__restrict, void *, DeeObject *__restrict))&lvalue_shr,
	/* .st_and         = */ (DREF DeeObject *(DCALL *)(DeeSTypeObject *__restrict, void *, DeeObject *__restrict))&lvalue_and,
	/* .st_or          = */ (DREF DeeObject *(DCALL *)(DeeSTypeObject *__restrict, void *, DeeObject *__restrict))&lvalue_or,
	/* .st_xor         = */ (DREF DeeObject *(DCALL *)(DeeSTypeObject *__restrict, void *, DeeObject *__restrict))&lvalue_xor,
	/* .st_pow         = */ (DREF DeeObject *(DCALL *)(DeeSTypeObject *__restrict, void *, DeeObject *__restrict))&lvalue_pow,
	/* .st_inc         = */ (int(DCALL *)(DeeSTypeObject *__restrict, void *))&lvalue_inc,
	/* .st_dec         = */ (int(DCALL *)(DeeSTypeObject *__restrict, void *))&lvalue_dec,
	/* .st_inplace_add = */ (int(DCALL *)(DeeSTypeObject *__restrict, void *, DeeObject *__restrict))&lvalue_inplace_add,
	/* .st_inplace_sub = */ (int(DCALL *)(DeeSTypeObject *__restrict, void *, DeeObject *__restrict))&lvalue_inplace_sub,
	/* .st_inplace_mul = */ (int(DCALL *)(DeeSTypeObject *__restrict, void *, DeeObject *__restrict))&lvalue_inplace_mul,
	/* .st_inplace_div = */ (int(DCALL *)(DeeSTypeObject *__restrict, void *, DeeObject *__restrict))&lvalue_inplace_div,
	/* .st_inplace_mod = */ (int(DCALL *)(DeeSTypeObject *__restrict, void *, DeeObject *__restrict))&lvalue_inplace_mod,
	/* .st_inplace_shl = */ (int(DCALL *)(DeeSTypeObject *__restrict, void *, DeeObject *__restrict))&lvalue_inplace_shl,
	/* .st_inplace_shr = */ (int(DCALL *)(DeeSTypeObject *__restrict, void *, DeeObject *__restrict))&lvalue_inplace_shr,
	/* .st_inplace_and = */ (int(DCALL *)(DeeSTypeObject *__restrict, void *, DeeObject *__restrict))&lvalue_inplace_and,
	/* .st_inplace_or  = */ (int(DCALL *)(DeeSTypeObject *__restrict, void *, DeeObject *__restrict))&lvalue_inplace_or,
	/* .st_inplace_xor = */ (int(DCALL *)(DeeSTypeObject *__restrict, void *, DeeObject *__restrict))&lvalue_inplace_xor,
	/* .st_inplace_pow = */ (int(DCALL *)(DeeSTypeObject *__restrict, void *, DeeObject *__restrict))&lvalue_inplace_pow
};

PRIVATE struct stype_seq lvalue_seq = {
	/* .stp_iter_self = */ (DREF DeeObject *(DCALL *)(DeeSTypeObject *__restrict, void *))&lvalue_iter,
	/* .stp_size      = */ (DREF DeeObject *(DCALL *)(DeeSTypeObject *__restrict, void *))&lvalue_size,
	/* .stp_contains  = */ (DREF DeeObject *(DCALL *)(DeeSTypeObject *__restrict, void *, DeeObject *__restrict))&lvalue_contains,
	/* .stp_get       = */ (DREF DeeObject *(DCALL *)(DeeSTypeObject *__restrict, void *, DeeObject *__restrict))&lvalue_getitem,
	/* .stp_del       = */ (int             (DCALL *)(DeeSTypeObject *__restrict, void *, DeeObject *__restrict))&lvalue_delitem,
	/* .stp_set       = */ (int             (DCALL *)(DeeSTypeObject *__restrict, void *, DeeObject *__restrict, DeeObject *__restrict))&lvalue_setitem,
	/* .stp_range_get = */ (DREF DeeObject *(DCALL *)(DeeSTypeObject *__restrict, void *, DeeObject *__restrict, DeeObject *__restrict))&lvalue_getrange,
	/* .stp_range_del = */ (int             (DCALL *)(DeeSTypeObject *__restrict, void *, DeeObject *__restrict, DeeObject *__restrict))&lvalue_delrange,
	/* .stp_range_set = */ (int             (DCALL *)(DeeSTypeObject *__restrict, void *, DeeObject *__restrict, DeeObject *__restrict, DeeObject *__restrict))&lvalue_setrange
};

PRIVATE struct stype_cmp lvalue_cmp = {
	/* .st_eq = */ (DREF DeeObject *(DCALL *)(DeeSTypeObject *__restrict, void *, DeeObject *__restrict))&lvalue_eq,
	/* .st_ne = */ (DREF DeeObject *(DCALL *)(DeeSTypeObject *__restrict, void *, DeeObject *__restrict))&lvalue_ne,
	/* .st_lo = */ (DREF DeeObject *(DCALL *)(DeeSTypeObject *__restrict, void *, DeeObject *__restrict))&lvalue_lo,
	/* .st_le = */ (DREF DeeObject *(DCALL *)(DeeSTypeObject *__restrict, void *, DeeObject *__restrict))&lvalue_le,
	/* .st_gr = */ (DREF DeeObject *(DCALL *)(DeeSTypeObject *__restrict, void *, DeeObject *__restrict))&lvalue_gr,
	/* .st_ge = */ (DREF DeeObject *(DCALL *)(DeeSTypeObject *__restrict, void *, DeeObject *__restrict))&lvalue_ge
};

PRIVATE struct stype_attr lvalue_attr = {
	/* .st_getattr  = */ (DREF DeeObject *(DCALL *)(DeeSTypeObject *__restrict, void *self, DeeObject *__restrict))&lvalue_getattr,
	/* .st_delattr  = */ (int(DCALL *)(DeeSTypeObject *__restrict, void *self, DeeObject *__restrict))&lvalue_delattr,
	/* .st_setattr  = */ (int(DCALL *)(DeeSTypeObject *__restrict, void *self, DeeObject *__restrict, DeeObject *__restrict))&lvalue_setattr,
	/* .st_enumattr = */ (dssize_t(DCALL *)(DeeSTypeObject *__restrict, denum_t, void *))&lvalue_enumattr
};

PRIVATE DREF struct pointer_object *DCALL
lvalue_ref(struct lvalue_object *__restrict self) {
	DREF struct pointer_object *result;
	DREF DeePointerTypeObject *pointer_type;
	pointer_type = DeeSType_Pointer(((DeeLValueTypeObject *)Dee_TYPE(self))->lt_orig);
	if unlikely(!pointer_type)
		goto err;
	result = DeeObject_MALLOC(struct pointer_object);
	if unlikely(!result)
		goto err;
	/* Construct a new pointer with the same data-value as our l-value. */
	DeeObject_InitNoref(result, (DREF DeeTypeObject *)pointer_type); /* Inherit reference: pointer_type */
	result->p_ptr.ptr = self->l_ptr.ptr;
	return result;
err:
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
lvalue_sizeof(struct lvalue_object *__restrict self) {
	size_t result;
	result = DeeSType_Sizeof(((DeeLValueTypeObject *)Dee_TYPE(self))->lt_orig);
	return DeeInt_NewSize(result);
}

PRIVATE DREF DeeObject *DCALL
lvalue_alignof(struct lvalue_object *__restrict self) {
	size_t result;
	result = DeeSType_Alignof(((DeeLValueTypeObject *)Dee_TYPE(self))->lt_orig);
	return DeeInt_NewSize(result);
}

PRIVATE struct type_getset lvalue_getsets[] = {
	{ "ref", (DREF DeeObject * (DCALL *)(DeeObject * __restrict))&lvalue_ref, NULL, NULL,
	  DOC("->?Gpointer\nReturns a pointer for the object referred to by @this :lvalue") },
	{ "sizeof", (DREF DeeObject * (DCALL *)(DeeObject * __restrict))&lvalue_sizeof, NULL, NULL,
	  DOC("->?Dint\nReturns the size of the structured objected pointed to by @this :lvalue") },
	{ "alignof", (DREF DeeObject * (DCALL *)(DeeObject * __restrict))&lvalue_alignof, NULL, NULL,
	  DOC("->?Dint\nReturns the alignment of the structured objected pointed to by @this :lvalue") },
	{ NULL }
};


INTDEF ATTR_COLD int DCALL
err_unimplemented_operator(DeeSTypeObject *__restrict tp,
                           uint16_t operator_name);

PRIVATE DREF struct lvalue_object *DCALL lvalue_ctor(void) {
	err_unimplemented_operator((DeeSTypeObject *)&DeeLValue_Type,
	                           OPERATOR_CONSTRUCTOR);
	return NULL;
}

PRIVATE DREF DeeObject *DCALL
lvalue_copy(struct lvalue_object *__restrict self) {
	DeeObject *result;
	size_t datasize;
	uint8_t *dst, *src;
	DeeSTypeObject *orig_type = (DeeSTypeObject *)Dee_TYPE(self);
	ASSERT(!orig_type->st_base.tp_init.tp_alloc.tp_free);
	datasize = orig_type->st_base.tp_init.tp_alloc.tp_instance_size;
	if unlikely(orig_type->st_base.tp_flags & TP_FGC) {
		/* This can happen when the user creates their own
		 * classes that are derived from structured types. */
		result = (DeeObject *)DeeGCObject_Malloc(datasize);
	} else {
		result = (DeeObject *)DeeObject_Malloc(datasize);
	}
	if unlikely(!result)
		goto done;
	datasize -= sizeof(DeeObject);
	dst = (uint8_t *)DeeStruct_Data(result);
	src = (uint8_t *)self->l_ptr.ptr;
	/* Copy data into the copy of the underlying object. */
#ifdef CONFIG_HAVE_CTYPES_FAULTPROTECT
#ifdef CONFIG_HAVE_CTYPES_RECURSIVE_PROTECT
	CTYPES_FAULTPROTECT({
		memcpy(dst, src, datasize);
	}, {
		if unlikely(orig_type->st_base.tp_flags & TP_FGC) {
			DeeGCObject_Free(result);
		} else {
			DeeObject_Free(result);
		}
		return NULL;
	});
#else /* CONFIG_HAVE_CTYPES_RECURSIVE_PROTECT */
	CTYPES_FAULTPROTECT({
		while (datasize--)
			*dst++ = *src++;
	}, {
		if unlikely(orig_type->st_base.tp_flags & TP_FGC) {
			DeeGCObject_Free(result);
		} else {
			DeeObject_Free(result);
		}
		return NULL;
	});
#endif /* !CONFIG_HAVE_CTYPES_RECURSIVE_PROTECT */
#else /* CONFIG_HAVE_CTYPES_FAULTPROTECT */
	memcpy(dst, src, datasize);
#endif /* !CONFIG_HAVE_CTYPES_FAULTPROTECT */

	DeeObject_Init(result, (DeeTypeObject *)orig_type);
	/* Handle GC objects (see above) */
	if unlikely(orig_type->st_base.tp_flags & TP_FGC)
		DeeGC_Track(result);
done:
	return result;
}

PRIVATE int DCALL
lvalue_tp_assign(struct lvalue_object *__restrict self,
                 DeeObject *__restrict other) {
	return DeeStruct_Assign(((DeeLValueTypeObject *)Dee_TYPE(self))->lt_orig,
	                        self->l_ptr.ptr, other);
}

PRIVATE int DCALL
lvalue_getbuf(struct lvalue_object *__restrict self,
              DeeBuffer *__restrict info,
              unsigned int UNUSED(flags)) {
	info->bb_base = self->l_ptr.ptr;
	info->bb_size = DeeSType_Sizeof(((DeeLValueTypeObject *)Dee_TYPE(self))->lt_orig);
	return 0;
}

PRIVATE struct type_buffer lvalue_buffer = {
	/* .tp_getbuf       = */ (int(DCALL *)(DeeObject *__restrict,DeeBuffer *__restrict, unsigned int))&lvalue_getbuf,
	/* .tp_putbuf       = */ NULL,
	/* .tp_buffer_flags = */ Dee_BUFFER_TYPE_FNORMAL
};


INTERN DeeLValueTypeObject DeeLValue_Type = {
	/* .pt_base = */ {
		/* .st_base = */ {
			OBJECT_HEAD_INIT((DeeTypeObject *)&DeeLValueType_Type),
			/* .tp_name     = */ "LValue",
			/* .tp_doc      = */ NULL,
			/* .tp_flags    = */ TP_FNORMAL | TP_FVARIABLE|TP_FMOVEANY|TP_FTRUNCATE,
			/* .tp_weakrefs = */ 0,
			/* .tp_features = */ TF_NONE,
			/* .tp_base     = */ (DeeTypeObject *)&DeeStructured_Type,
			/* .tp_init = */ {
				{
					/* .tp_var = */ {
						/* .tp_ctor      = */ (void *)&lvalue_ctor,
						/* .tp_copy_ctor = */ (void *)&lvalue_copy,
						/* .tp_deep_ctor = */ NULL,
						/* .tp_any_ctor  = */ NULL,
						/* .tp_free      = */ NULL,
						{
							/* ..tp_alloc.tp_instance_size = */sizeof(struct pointer_object)
						}
					}
				},
				/* .tp_dtor        = */ NULL,
				/* .tp_assign      = */ (int(DCALL *)(DeeObject *__restrict, DeeObject *__restrict))&lvalue_tp_assign,
				/* .tp_move_assign = */ (int(DCALL *)(DeeObject *__restrict, DeeObject *__restrict))&lvalue_tp_assign
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
			/* .tp_attr          = */ NULL,
			/* .tp_with          = */ NULL,
			/* .tp_buffer        = */ &lvalue_buffer,
			/* .tp_methods       = */ NULL,
			/* .tp_getsets       = */lvalue_getsets,
			/* .tp_members       = */ NULL,
			/* .tp_class_methods = */ NULL,
			/* .tp_class_getsets = */ NULL,
			/* .tp_class_members = */ NULL
		},
#ifndef CONFIG_NO_THREADS
		/* .st_cachelock = */ RWLOCK_INIT,
#endif /* !CONFIG_NO_THREADS */
		/* .st_pointer  = */ &DeePointer_Type,
		/* .st_lvalue   = */ &DeeLValue_Type,
		/* .st_array    = */ STYPE_ARRAY_INIT,
#ifndef CONFIG_NO_CFUNCTION
		/* .st_cfunction= */ STYPE_CFUNCTION_INIT,
		/* .st_ffitype  = */ &ffi_type_pointer,
#endif /* !CONFIG_NO_CFUNCTION */
		/* .st_align    = */CONFIG_CTYPES_ALIGNOF_LVALUE,
		/* .st_init     = */ NULL,
		/* .st_assign   = */ (int(DCALL *)(DeeSTypeObject *__restrict, void *, DeeObject *__restrict))&lvalue_assign,
		/* .st_cast     = */ {
			/* .st_str  = */ (DREF DeeObject *(DCALL *)(DeeSTypeObject *__restrict, void *))&lvalue_str,
			/* .st_repr = */ (DREF DeeObject *(DCALL *)(DeeSTypeObject *__restrict, void *))&lvalue_repr,
			/* .st_bool = */ (int(DCALL *)(DeeSTypeObject *__restrict, void *))&lvalue_bool
		},
		/* .st_call     = */ (DREF DeeObject *(DCALL *)(DeeSTypeObject *__restrict, void *, size_t, DeeObject **__restrict))&lvalue_call,
		/* .st_math     = */ &lvalue_math,
		/* .st_cmp      = */ &lvalue_cmp,
		/* .st_seq      = */ &lvalue_seq,
		/* .st_attr     = */ &lvalue_attr
	},
	/* .lt_orig = */ &DeeStructured_Type
};


DECL_END


#endif /* !GUARD_DEX_CTYPES_POINTER_C */
