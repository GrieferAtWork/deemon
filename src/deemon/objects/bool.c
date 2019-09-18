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
#ifndef GUARD_DEEMON_OBJECTS_BOOL_C
#define GUARD_DEEMON_OBJECTS_BOOL_C 1

#include <deemon/api.h>
#include <deemon/arg.h>
#include <deemon/bool.h>
#include <deemon/int.h>
#include <deemon/none.h>
#include <deemon/numeric.h>
#include <deemon/object.h>
#include <deemon/string.h>

#include "../runtime/runtime_error.h"
#include "../runtime/strings.h"

DECL_BEGIN


PRIVATE DREF DeeObject *DCALL
bool_return_false(void) {
	return_false;
}

PRIVATE DREF DeeObject *DCALL
bool_new(size_t argc, DeeObject **__restrict argv) {
	bool value;
	if (DeeArg_Unpack(argc, argv, "b:bool", &value))
		return NULL;
	return_bool_(value);
}

PRIVATE DREF DeeObject *DCALL
bool_str(DeeObject *__restrict self) {
	return_reference(DeeBool_IsTrue(self) ? &str_true : &str_false);
}

PRIVATE int DCALL
bool_bool(DeeObject *__restrict self) {
	return DeeBool_IsTrue(self);
}

#if __SIZEOF_INT__ == __SIZEOF_POINTER__
#define bool_hash (*(dhash_t(DCALL *)(DeeObject * __restrict)) & bool_bool)
#else /* __SIZEOF_INT__ == __SIZEOF_POINTER__ */
PRIVATE dhash_t DCALL
bool_hash(DeeObject *__restrict self) {
	return DeeBool_IsTrue(self);
}
#endif /* __SIZEOF_INT__ != __SIZEOF_POINTER__ */


PRIVATE int DCALL
bool_int32(DeeObject *__restrict self,
           int32_t *__restrict result) {
	*result = DeeBool_IsTrue(self);
	return INT_UNSIGNED;
}

PRIVATE int DCALL
bool_int64(DeeObject *__restrict self,
           int64_t *__restrict result) {
	*result = DeeBool_IsTrue(self);
	return INT_UNSIGNED;
}

PRIVATE int DCALL
bool_double(DeeObject *__restrict self,
            double *__restrict result) {
	*result = DeeBool_IsTrue(self);
	return 0;
}

PRIVATE DREF DeeObject *DCALL
bool_int(DeeObject *__restrict self) {
	return_reference(DeeBool_IsTrue(self) ? &DeeInt_One : &DeeInt_Zero);
}

PRIVATE DREF DeeObject *DCALL
bool_inv(DeeObject *__restrict self) {
	return_bool(!DeeBool_IsTrue(self));
}

PRIVATE DREF DeeObject *DCALL
bool_or(DeeObject *__restrict self,
        DeeObject *__restrict other) {
	int temp;
	if (DeeBool_IsTrue(self))
		return_true;
	temp = DeeObject_Bool(other);
	if
		unlikely(temp < 0)
	return NULL;
	return_bool(temp);
}

PRIVATE DREF DeeObject *DCALL
bool_and(DeeObject *__restrict self,
         DeeObject *__restrict other) {
	int temp;
	if (!DeeBool_IsTrue(self))
		return_false;
	temp = DeeObject_Bool(other);
	if
		unlikely(temp < 0)
	return NULL;
	return_bool(temp);
}

PRIVATE DREF DeeObject *DCALL
bool_xor(DeeObject *__restrict self,
         DeeObject *__restrict other) {
	int temp = DeeObject_Bool(other);
	if
		unlikely(temp < 0)
	return NULL;
	return_bool(DeeBool_IsTrue(self) ^ temp);
}

PRIVATE DREF DeeObject *DCALL
bool_div(DeeObject *__restrict self, DeeObject *__restrict other) {
	int temp = DeeObject_Bool(other);
	if
		unlikely(temp < 0)
	return NULL;
	if
		unlikely(!temp)
	{
		err_divide_by_zero(self, other);
		return NULL;
	}
	return_reference_(self);
}

PRIVATE DREF DeeObject *DCALL
bool_mod(DeeObject *__restrict self, DeeObject *__restrict other) {
	int temp = DeeObject_Bool(other);
	if
		unlikely(temp < 0)
	return NULL;
	if
		unlikely(!temp)
	{
		err_divide_by_zero(self, other);
		return NULL;
	}
	return_false;
}

PRIVATE DREF DeeObject *DCALL
bool_shl(DeeObject *__restrict self, DeeObject *__restrict other) {
	dssize_t shift_value;
	if (DeeObject_AsSSize(other, &shift_value))
		return NULL;
	if (shift_value < 0) {
		err_shift_negative(self, other, true);
		return NULL;
	}
	return_reference_(self);
}

PRIVATE DREF DeeObject *DCALL
bool_shr(DeeObject *__restrict self, DeeObject *__restrict other) {
	dssize_t shift_value;
	if (DeeObject_AsSSize(other, &shift_value))
		return NULL;
	if (shift_value < 0) {
		err_shift_negative(self, other, false);
		return NULL;
	}
	if (shift_value != 0)
		return_false;
	return_reference_(self);
}

PRIVATE DREF DeeObject *DCALL
bool_pow(DeeObject *__restrict self, DeeObject *__restrict other) {
	int temp;
	if (DeeBool_IsTrue(self))
		return_true;
	temp = DeeObject_Bool(other);
	if (temp < 0)
		return NULL;
	return_bool(!temp);
}

PRIVATE struct type_math bool_math = {
	/* .tp_int32  = */ &bool_int32,
	/* .tp_int64  = */ &bool_int64,
	/* .tp_double = */ &bool_double,
	/* .tp_int    = */ &bool_int,
	/* .tp_inv    = */ &bool_inv,
	/* .tp_pos    = */ &DeeObject_NewRef,
	/* .tp_neg    = */ &DeeObject_NewRef, /* -x == ~(x-1);
	                                       *  --> ~(0-1) = ~-1 = 0 (0 -> 0)
	                                       *  --> ~(1-1) = ~0  = 1 (1 -> 1)
	                                       *  Ergo: -bool == bool */
	/* .tp_add    = */ &bool_or,
	/* .tp_sub    = */ &bool_xor, /* 0,0 -> 0-0 =  0 = 0;
	                               * 0,1 -> 0-1 = -1 = 1;
	                               * 1,0 -> 1-0 =  1 = 1;
	                               * 1,1 -> 0-0 =  0 = 0;
	                               * >> Same as xor. */
	/* .tp_mul    = */ &bool_and, /* 0,0 -> 0*0 =  0 = 0;
	                               * 0,1 -> 0*1 =  0 = 0;
	                               * 1,0 -> 1*0 =  0 = 0;
	                               * 1,1 -> 1*1 =  1 = 1;
	                               * >> Same as and. */
	/* .tp_div    = */ &bool_div, /* 0,0 -> 0 / 0 =  ERROR;
	                               * 0,1 -> 0 / 1 =  0 = 0;
	                               * 1,0 -> 1 / 0 =  ERROR;
	                               * 1,1 -> 1 / 1 =  1 = 1; */
	/* .tp_mod    = */ &bool_mod, /* 0,0 -> 0 % 0 =  ERROR;
	                               * 0,1 -> 0 % 1 =  0 = 0;
	                               * 1,0 -> 1 % 0 =  ERROR;
	                               * 1,1 -> 1 % 1 =  0 = 0;
	                               */
	/* .tp_shl    = */ &bool_shl, /* 0,0 -> 0 << 0 = 0;
	                               * 0,1 -> 0 << 1 = 0;
	                               * 1,0 -> 1 << 0 = 1;
	                               * 1,1 -> 1 << 1 = 1;
	                               */
	/* .tp_shr    = */ &bool_shr, /* 0,0 -> 0 >> 0 = 0;
	                               * 0,1 -> 0 >> 1 = 0;
	                               * 1,0 -> 1 >> 0 = 1;
	                               * 1,1 -> 1 >> 1 = 0; // No sign extension because `bool' is unsigned.
	                               */
	/* .tp_and    = */ &bool_and,
	/* .tp_or     = */ &bool_or,
	/* .tp_xor    = */ &bool_xor,
	/* .tp_pow    = */ &bool_pow, /* 0,0 -> 0 ** 0 = 1;
	                               * 0,1 -> 0 ** 1 = 0;
	                               * 1,0 -> 1 ** 0 = 1;
	                               * 1,1 -> 1 ** 1 = 1; */
};


PRIVATE DREF DeeObject *DCALL
bool_eq(DeeObject *__restrict self, DeeObject *__restrict other) {
	int error = DeeObject_Bool(other);
	if
		unlikely(error < 0)
	return NULL;
	return_bool_((!error) != DeeBool_IsTrue(self));
}

PRIVATE DREF DeeObject *DCALL
bool_ne(DeeObject *__restrict self, DeeObject *__restrict other) {
	int error = DeeObject_Bool(other);
	if
		unlikely(error < 0)
	return NULL;
	return_bool_((!error) == DeeBool_IsTrue(self));
}

PRIVATE DREF DeeObject *DCALL
bool_lo(DeeObject *__restrict self, DeeObject *__restrict other) {
	int error;
	if (DeeBool_IsTrue(self))
		return_false; /* true < X --> false */
	error = DeeObject_Bool(other);
	if
		unlikely(error < 0)
	return NULL;
	return_bool_(error); /* false < X --> X */
}

PRIVATE DREF DeeObject *DCALL
bool_le(DeeObject *__restrict self, DeeObject *__restrict other) {
	int error;
	if (!DeeBool_IsTrue(self))
		return_true; /* false <= X --> true */
	error = DeeObject_Bool(other);
	if
		unlikely(error < 0)
	return NULL;
	return_bool_(error); /* true <= X --> X */
}

PRIVATE DREF DeeObject *DCALL
bool_gr(DeeObject *__restrict self, DeeObject *__restrict other) {
	int error;
	if (!DeeBool_IsTrue(self))
		return_false; /* false > X --> false */
	error = DeeObject_Bool(other);
	if
		unlikely(error < 0)
	return NULL;
	return_bool_(error); /* true > X --> X */
}

PRIVATE DREF DeeObject *DCALL
bool_ge(DeeObject *__restrict self, DeeObject *__restrict other) {
	int error;
	if (DeeBool_IsTrue(self))
		return_true; /* true >= X --> true */
	error = DeeObject_Bool(other);
	if
		unlikely(error < 0)
	return NULL;
	return_bool_(!error); /* false >= X --> !X */
}

PRIVATE struct type_cmp bool_cmp = {
	/* .tp_hash = */ &bool_hash,
	/* .tp_eq   = */ &bool_eq,
	/* .tp_ne   = */ &bool_ne,
	/* .tp_lo   = */ &bool_lo,
	/* .tp_le   = */ &bool_le,
	/* .tp_gr   = */ &bool_gr,
	/* .tp_ge   = */ &bool_ge
};

PRIVATE DREF DeeObject *DCALL
bool_gettrue(DeeObject *__restrict UNUSED(self)) {
	return_true;
}

PRIVATE DREF DeeObject *DCALL
bool_getfalse(DeeObject *__restrict UNUSED(self)) {
	return_false;
}

PRIVATE struct type_getset bool_class_getsets[] = {
	{ DeeString_STR(&str_true), &bool_gettrue, NULL, NULL, DOC("->?Dbool") },
	{ DeeString_STR(&str_false), &bool_getfalse, NULL, NULL, DOC("->?Dbool") },
	{ NULL }
};

PUBLIC DeeTypeObject DeeBool_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ DeeString_STR(&str_bool),
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FVARIABLE | TP_FFINAL | TP_FTRUNCATE | TP_FNAMEOBJECT,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeNumeric_Type,
	/* .tp_init = */ {
		{
			/* .tp_var = */ {
				/* .tp_ctor      = */ &bool_return_false,
				/* .tp_copy_ctor = */ &DeeObject_NewRef,
				/* .tp_deep_ctor = */ &DeeObject_NewRef,
				/* .tp_any_ctor  = */ &bool_new
			}
		},
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ &bool_str,
		/* .tp_repr = */ &bool_str,
		/* .tp_bool = */ &bool_bool
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ &bool_math,
	/* .tp_cmp           = */ &bool_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ NULL,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ bool_class_getsets,
	/* .tp_class_members = */ NULL
};

PUBLIC DeeBoolObject Dee_FalseTrue[2] = {
	{ OBJECT_HEAD_INIT(&DeeBool_Type) }, /* `false' */
	{ OBJECT_HEAD_INIT(&DeeBool_Type) }  /* `true' */
};


DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_BOOL_C */
