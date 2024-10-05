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


PRIVATE WUNUSED DREF DeeObject *DCALL
bool_return_false(void) {
	return_false;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
bool_new(size_t argc, DeeObject *const *argv) {
	bool value;
	if (DeeArg_Unpack(argc, argv, "b:bool", &value))
		goto err;
	return_bool_(value);
err:
	return NULL;
}

PRIVATE DeeObject *const bool_strings[2] = {
	(DeeObject *)&str_false,
	(DeeObject *)&str_true
};
#define bool_string(self) bool_strings[DeeBool_IsTrue(self) ? 1 : 0]

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bool_str(DeeObject *__restrict self) {
	return_reference(bool_string(self));
}

PRIVATE WUNUSED NONNULL((1, 2)) dssize_t DCALL
bool_print(DeeObject *__restrict self,
           dformatprinter printer, void *arg) {
	DeeObject *str = bool_string(self);
	return DeeString_PrintAscii(str, printer, arg);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
bool_bool(DeeObject *__restrict self) {
	return DeeBool_IsTrue(self);
}

#if __SIZEOF_INT__ == __SIZEOF_POINTER__
#define bool_hash (*(dhash_t (DCALL *)(DeeObject *__restrict))&bool_bool)
#else /* __SIZEOF_INT__ == __SIZEOF_POINTER__ */
PRIVATE WUNUSED NONNULL((1)) dhash_t DCALL
bool_hash(DeeObject *__restrict self) {
	return DeeBool_IsTrue(self);
}
#endif /* __SIZEOF_INT__ != __SIZEOF_POINTER__ */


PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
bool_int32(DeeObject *__restrict self,
           int32_t *__restrict result) {
	*result = DeeBool_IsTrue(self);
	return INT_UNSIGNED;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
bool_int64(DeeObject *__restrict self,
           int64_t *__restrict result) {
	*result = DeeBool_IsTrue(self);
	return INT_UNSIGNED;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
bool_double(DeeObject *__restrict self,
            double *__restrict result) {
	*result = DeeBool_IsTrue(self);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bool_int(DeeObject *__restrict self) {
	return_reference(DeeBool_IsTrue(self) ? DeeInt_One : DeeInt_Zero);
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bool_inv(DeeObject *__restrict self) {
	return_bool(!DeeBool_IsTrue(self));
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
bool_or(DeeObject *self, DeeObject *other) {
	int temp;
	if (DeeBool_IsTrue(self))
		return_true;
	temp = DeeObject_Bool(other);
	if unlikely(temp < 0)
		goto err;
	return_bool_(temp);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
bool_and(DeeObject *self, DeeObject *other) {
	int temp;
	if (!DeeBool_IsTrue(self))
		return_false;
	temp = DeeObject_Bool(other);
	if unlikely(temp < 0)
		goto err;
	return_bool_(temp);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
bool_xor(DeeObject *self, DeeObject *other) {
	int temp = DeeObject_Bool(other);
	if unlikely(temp < 0)
		goto err;
	return_bool(DeeBool_IsTrue(self) ^ !!temp);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
bool_div(DeeObject *self, DeeObject *other) {
	int temp = DeeObject_Bool(other);
	if unlikely(temp < 0)
		goto err;
	if unlikely(!temp) {
		err_divide_by_zero(self, other);
		goto err;
	}
	return_reference_(self);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
bool_mod(DeeObject *self, DeeObject *other) {
	int temp = DeeObject_Bool(other);
	if unlikely(temp < 0)
		goto err;
	if unlikely(!temp) {
		err_divide_by_zero(self, other);
		goto err;
	}
	return_false;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
bool_shl(DeeObject *self, DeeObject *other) {
	dssize_t shift_value;
	if (DeeObject_AsSSize(other, &shift_value))
		goto err;
	if unlikely(shift_value < 0) {
		err_shift_negative(self, other, true);
		goto err;
	}
	return_reference_(self);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
bool_shr(DeeObject *self, DeeObject *other) {
	dssize_t shift_value;
	if (DeeObject_AsSSize(other, &shift_value))
		goto err;
	if unlikely(shift_value < 0) {
		err_shift_negative(self, other, false);
		goto err;
	}
	if (shift_value != 0)
		return_false;
	return_reference_(self);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
bool_pow(DeeObject *self, DeeObject *other) {
	int temp;
	if (DeeBool_IsTrue(self))
		return_true;
	temp = DeeObject_Bool(other);
	if unlikely(temp < 0)
		goto err;
	return_bool_(!temp);
err:
	return NULL;
}

PRIVATE struct type_math bool_math = {
	/* .tp_int32  = */ &bool_int32,
	/* .tp_int64  = */ &bool_int64,
	/* .tp_double = */ &bool_double,
	/* .tp_int    = */ &bool_int,
	/* .tp_inv    = */ &bool_inv,
	/* .tp_pos    = */ &DeeObject_NewRef,
	/* .tp_neg    = */ &DeeObject_NewRef, /* -x == ~(x - 1);
	                                       *  --> ~(0 - 1) = ~-1 = 0 (0 -> 0)
	                                       *  --> ~(1 - 1) = ~0  = 1 (1 -> 1)
	                                       *  Ergo: -bool == bool */
	/* .tp_add    = */ &bool_or,
	/* .tp_sub    = */ &bool_xor, /* 0, 0 -> 0-0 =  0 = 0;
	                               * 0, 1 -> 0-1 = -1 = 1;
	                               * 1, 0 -> 1-0 =  1 = 1;
	                               * 1, 1 -> 0-0 =  0 = 0;
	                               * >> Same as xor. */
	/* .tp_mul    = */ &bool_and, /* 0, 0 -> 0*0 =  0 = 0;
	                               * 0, 1 -> 0*1 =  0 = 0;
	                               * 1, 0 -> 1*0 =  0 = 0;
	                               * 1, 1 -> 1*1 =  1 = 1;
	                               * >> Same as and. */
	/* .tp_div    = */ &bool_div, /* 0, 0 -> 0 / 0 =  ERROR;
	                               * 0, 1 -> 0 / 1 =  0 = 0;
	                               * 1, 0 -> 1 / 0 =  ERROR;
	                               * 1, 1 -> 1 / 1 =  1 = 1; */
	/* .tp_mod    = */ &bool_mod, /* 0, 0 -> 0 % 0 =  ERROR;
	                               * 0, 1 -> 0 % 1 =  0 = 0;
	                               * 1, 0 -> 1 % 0 =  ERROR;
	                               * 1, 1 -> 1 % 1 =  0 = 0; */
	/* .tp_shl    = */ &bool_shl, /* 0, 0 -> 0 << 0 = 0;
	                               * 0, 1 -> 0 << 1 = 0;
	                               * 1, 0 -> 1 << 0 = 1;
	                               * 1, 1 -> 1 << 1 = 1; */
	/* .tp_shr    = */ &bool_shr, /* 0, 0 -> 0 >> 0 = 0;
	                               * 0, 1 -> 0 >> 1 = 0;
	                               * 1, 0 -> 1 >> 0 = 1;
	                               * 1, 1 -> 1 >> 1 = 0; // No sign extension because `bool' is positive, unsigned. */
	/* .tp_and    = */ &bool_and,
	/* .tp_or     = */ &bool_or,
	/* .tp_xor    = */ &bool_xor,
	/* .tp_pow    = */ &bool_pow, /* 0, 0 -> 0 ** 0 = 1;
	                               * 0, 1 -> 0 ** 1 = 0;
	                               * 1, 0 -> 1 ** 0 = 1;
	                               * 1, 1 -> 1 ** 1 = 1; */
};


PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
bool_compare_eq(DeeObject *self, DeeObject *other) {
	int error;
	if (DeeBool_Check(other))
		return self == other ? 0 : 1;
	error = DeeObject_Bool(other);
	if unlikely(error < 0)
		goto err;
	return (!error) != DeeBool_IsTrue(self) ? 0 : 1;
err:
	return Dee_COMPARE_ERR;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
bool_compare(DeeObject *self, DeeObject *other) {
	int error;
	if (DeeBool_Check(other))
		return self == other ? 0 : 1;
	error = DeeObject_Bool(other);
	if unlikely(error < 0)
		goto err;
	Dee_return_compare(DeeBool_IsTrue(self), !!error);
err:
	return Dee_COMPARE_ERR;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
bool_eq(DeeObject *self, DeeObject *other) {
	int error = DeeObject_Bool(other);
	if unlikely(error < 0)
		goto err;
	return_bool_((!error) != DeeBool_IsTrue(self));
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
bool_ne(DeeObject *self, DeeObject *other) {
	int error = DeeObject_Bool(other);
	if unlikely(error < 0)
		goto err;
	return_bool_((!error) == DeeBool_IsTrue(self));
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
bool_lo(DeeObject *self, DeeObject *other) {
	int error;
	if (DeeBool_IsTrue(self))
		return_false; /* true < X --> false */
	error = DeeObject_Bool(other);
	if unlikely(error < 0)
		goto err;
	return_bool_(error); /* false < X --> X */
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
bool_le(DeeObject *self, DeeObject *other) {
	int error;
	if (!DeeBool_IsTrue(self))
		return_true; /* false <= X --> true */
	error = DeeObject_Bool(other);
	if unlikely(error < 0)
		goto err;
	return_bool_(error); /* true <= X --> X */
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
bool_gr(DeeObject *self, DeeObject *other) {
	int error;
	if (!DeeBool_IsTrue(self))
		return_false; /* false > X --> false */
	error = DeeObject_Bool(other);
	if unlikely(error < 0)
		goto err;
	return_bool_(error); /* true > X --> X */
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
bool_ge(DeeObject *self, DeeObject *other) {
	int error;
	if (DeeBool_IsTrue(self))
		return_true; /* true >= X --> true */
	error = DeeObject_Bool(other);
	if unlikely(error < 0)
		goto err;
	return_bool_(!error); /* false >= X --> !X */
err:
	return NULL;
}

PRIVATE struct type_cmp bool_cmp = {
	/* .tp_hash          = */ &bool_hash,
	/* .tp_compare_eq    = */ &bool_compare_eq,
	/* .tp_compare       = */ &bool_compare,
	/* .tp_trycompare_eq = */ NULL,
	/* .tp_eq            = */ &bool_eq,
	/* .tp_ne            = */ &bool_ne,
	/* .tp_lo            = */ &bool_lo,
	/* .tp_le            = */ &bool_le,
	/* .tp_gr            = */ &bool_gr,
	/* .tp_ge            = */ &bool_ge
};

PRIVATE struct type_member tpconst bool_members[] = {
	TYPE_MEMBER_CONST(STR_isfloat, Dee_False),
	TYPE_MEMBER_END
};

PRIVATE struct type_member tpconst bool_class_members[] = {
	TYPE_MEMBER_CONST(STR_true, Dee_True),
	TYPE_MEMBER_CONST(STR_false, Dee_False),
	TYPE_MEMBER_CONST(STR_isfloat, Dee_False),
	TYPE_MEMBER_END
};

PRIVATE struct type_operator const bool_operators[] = {
	TYPE_OPERATOR_FLAGS(OPERATOR_0000_CONSTRUCTOR, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST),
	TYPE_OPERATOR_FLAGS(OPERATOR_0001_COPY, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_0002_DEEPCOPY, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_0006_STR, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_0007_REPR, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_0008_BOOL, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_000B_INT, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_000C_FLOAT, METHOD_FCONSTCALL),
	TYPE_OPERATOR_FLAGS(OPERATOR_000D_INV, METHOD_FCONSTCALL),
	TYPE_OPERATOR_FLAGS(OPERATOR_000E_POS, METHOD_FCONSTCALL),
	TYPE_OPERATOR_FLAGS(OPERATOR_000F_NEG, METHOD_FCONSTCALL),
	TYPE_OPERATOR_FLAGS(OPERATOR_0010_ADD, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST),
	TYPE_OPERATOR_FLAGS(OPERATOR_0011_SUB, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST),
	TYPE_OPERATOR_FLAGS(OPERATOR_0012_MUL, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST),
	TYPE_OPERATOR_FLAGS(OPERATOR_0013_DIV, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST),
	TYPE_OPERATOR_FLAGS(OPERATOR_0014_MOD, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST),
	TYPE_OPERATOR_FLAGS(OPERATOR_0015_SHL, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST),
	TYPE_OPERATOR_FLAGS(OPERATOR_0016_SHR, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST),
	TYPE_OPERATOR_FLAGS(OPERATOR_0017_AND, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST),
	TYPE_OPERATOR_FLAGS(OPERATOR_0018_OR, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST),
	TYPE_OPERATOR_FLAGS(OPERATOR_0019_XOR, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST),
	TYPE_OPERATOR_FLAGS(OPERATOR_001A_POW, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST),
	TYPE_OPERATOR_FLAGS(OPERATOR_0028_HASH, METHOD_FCONSTCALL | METHOD_FNOTHROW),
	TYPE_OPERATOR_FLAGS(OPERATOR_0029_EQ, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST),
	TYPE_OPERATOR_FLAGS(OPERATOR_002A_NE, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST),
	TYPE_OPERATOR_FLAGS(OPERATOR_002B_LO, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST),
	TYPE_OPERATOR_FLAGS(OPERATOR_002C_LE, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST),
	TYPE_OPERATOR_FLAGS(OPERATOR_002D_GR, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST),
	TYPE_OPERATOR_FLAGS(OPERATOR_002E_GE, METHOD_FCONSTCALL | METHOD_FCONSTCALL_IF_ARGS_CONSTCAST),
};

PUBLIC DeeTypeObject DeeBool_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ DeeString_STR(&str_bool),
	/* .tp_doc      = */ DOC("()\n"
	                         "Return the $false singleton\n"
	                         "\n"

	                         "(ob)\n"
	                         "Convert the given @ob into a boolean"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FVARIABLE | TP_FFINAL | TP_FTRUNCATE | TP_FNAMEOBJECT,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeNumeric_Type,
	/* .tp_init = */ {
		{
			/* .tp_var = */ {
				/* .tp_ctor      = */ (dfunptr_t)&bool_return_false,
				/* .tp_copy_ctor = */ (dfunptr_t)&DeeObject_NewRef,
				/* .tp_deep_ctor = */ (dfunptr_t)&DeeObject_NewRef,
				/* .tp_any_ctor  = */ (dfunptr_t)&bool_new
			}
		},
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ &bool_str,
		/* .tp_repr      = */ &bool_str,
		/* .tp_bool      = */ &bool_bool,
		/* .tp_print     = */ &bool_print,
		/* .tp_printrepr = */ &bool_print
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ &bool_math,
	/* .tp_cmp           = */ &bool_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_iterator      = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ bool_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ bool_class_members,
	/* .tp_call_kw       = */ NULL,
	/* .tp_mro           = */ NULL,
	/* .tp_operators     = */ bool_operators,
	/* .tp_operators_size= */ COMPILER_LENOF(bool_operators)
};

PUBLIC DeeBoolObject Dee_FalseTrue[2] = {
	{ OBJECT_HEAD_INIT(&DeeBool_Type) }, /* `false' */
	{ OBJECT_HEAD_INIT(&DeeBool_Type) }  /* `true' */
};


DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_BOOL_C */
