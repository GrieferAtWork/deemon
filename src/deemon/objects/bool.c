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
#ifndef GUARD_DEEMON_OBJECTS_BOOL_C
#define GUARD_DEEMON_OBJECTS_BOOL_C 1

#include <deemon/api.h>

#include <deemon/bool.h>
#include <deemon/computed-operators.h>
#include <deemon/error-rt.h>
#include <deemon/int.h>
#include <deemon/numeric.h>
#include <deemon/object.h>
#include <deemon/string.h>

#include <hybrid/typecore.h>

#include "../runtime/runtime_error.h"
#include "../runtime/strings.h"

#include <stdint.h> /* int32_t */

DECL_BEGIN

#if (10 == 10) == 1 && (10 == 20) == 0
#define DeeBool_IsTrue_10(self) DeeBool_IsTrue(self)
#else /* ... */
#define DeeBool_IsTrue_10(self) (DeeBool_IsTrue(self) ? 1 : 0)
#endif /* !... */

PRIVATE WUNUSED DREF DeeObject *DCALL
bool_return_false(void) {
	return_false;
}

PRIVATE WUNUSED DREF DeeObject *DCALL
bool_new(size_t argc, DeeObject *const *argv) {
	if unlikely(argc != 1)
		goto err_bad_argc;
	return DeeObject_BoolOb(argv[0]);
err_bad_argc:
	err_invalid_argc("bool", argc, 1, 1);
	return NULL;
}

PRIVATE DeeObject *tpconst bool_strings[2] = {
	Dee_AsObject(&str_false),
	Dee_AsObject(&str_true)
};
#define bool_string(self) bool_strings[DeeBool_IsTrue_10(self)]

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bool_str(DeeObject *__restrict self) {
	return_reference(bool_string(self));
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
bool_print(DeeObject *__restrict self,
           Dee_formatprinter_t printer, void *arg) {
	DeeObject *str = bool_string(self);
	return DeeString_PrintAscii(str, printer, arg);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
bool_bool(DeeObject *__restrict self) {
	return DeeBool_IsTrue_10(self);
}

#if __SIZEOF_INT__ == Dee_SIZEOF_HASH_T
#define bool_hash_PTR ((Dee_hash_t (DCALL *)(DeeObject *__restrict))&bool_bool)
#else /* __SIZEOF_INT__ == Dee_SIZEOF_HASH_T */
#define bool_hash_PTR &bool_hash
PRIVATE WUNUSED NONNULL((1)) Dee_hash_t DCALL
bool_hash(DeeObject *__restrict self) {
	return DeeBool_IsTrue_10(self);
}
#endif /* __SIZEOF_INT__ != Dee_SIZEOF_HASH_T */


PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
bool_int32(DeeObject *__restrict self,
           int32_t *__restrict result) {
	*result = DeeBool_IsTrue_10(self);
	return INT_UNSIGNED;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
bool_int64(DeeObject *__restrict self,
           int64_t *__restrict result) {
	*result = DeeBool_IsTrue_10(self);
	return INT_UNSIGNED;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
bool_double(DeeObject *__restrict self,
            double *__restrict result) {
	*result = DeeBool_IsTrue(self) ? 1.0 : 0.0;
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bool_int(DeeObject *__restrict self) {
	Dee_return_smallint(DeeBool_IsTrue_10(self));
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
bool_inv(DeeObject *__restrict self) {
	return_bool(!DeeBool_IsTrue(self));
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
bool_or(DeeObject *self, DeeObject *other) {
	if (DeeBool_IsTrue(self))
		return_true;
	return DeeObject_BoolOb(other);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
bool_and(DeeObject *self, DeeObject *other) {
	if (!DeeBool_IsTrue(self))
		return_false;
	return DeeObject_BoolOb(other);
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
bool_xor(DeeObject *self, DeeObject *other) {
	int temp = DeeObject_Bool(other);
	if unlikely(temp < 0)
		goto err;
	return_bool(!!DeeBool_IsTrue(self) ^ !!temp);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
bool_div(DeeObject *self, DeeObject *other) {
	int temp = DeeObject_Bool(other);
	if unlikely(temp < 0)
		goto err;
	if unlikely(!temp) {
		DeeRT_ErrDivideByZero(self, other);
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
		DeeRT_ErrDivideByZero(self, other);
		goto err;
	}
	return_false;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
bool_shl(DeeObject *self, DeeObject *other) {
	size_t shift_value;
	if (DeeObject_AsSize(other, &shift_value))
		goto err;
	return_reference_(self);
err:
	DeeRT_ErrNegativeShiftOverflow(self, true);
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
bool_shr(DeeObject *self, DeeObject *other) {
	size_t shift_value;
	if (DeeObject_AsSize(other, &shift_value))
		goto err;
	if (shift_value != 0)
		return_false;
	return_reference_(self);
err:
	DeeRT_ErrNegativeShiftOverflow(self, false);
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
	return_bool(!temp);
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
	/* .tp_sub    = */ &bool_xor, /* 0, 0 -> 0 - 0 =  0 = 0;
	                               * 0, 1 -> 0 - 1 = -1 = 1;
	                               * 1, 0 -> 1 - 0 =  1 = 1;
	                               * 1, 1 -> 0 - 0 =  0 = 0;
	                               * >> Same as xor. */
	/* .tp_mul    = */ &bool_and, /* 0, 0 -> 0 * 0 =  0 = 0;
	                               * 0, 1 -> 0 * 1 =  0 = 0;
	                               * 1, 0 -> 1 * 0 =  0 = 0;
	                               * 1, 1 -> 1 * 1 =  1 = 1;
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
	/* .tp_inc         = */ DEFIMPL(&default__inc__with__add),
	/* .tp_dec         = */ DEFIMPL(&default__dec__with__sub),
	/* .tp_inplace_add = */ DEFIMPL(&default__inplace_add__with__add),
	/* .tp_inplace_sub = */ DEFIMPL(&default__inplace_sub__with__sub),
	/* .tp_inplace_mul = */ DEFIMPL(&default__inplace_mul__with__mul),
	/* .tp_inplace_div = */ DEFIMPL(&default__inplace_div__with__div),
	/* .tp_inplace_mod = */ DEFIMPL(&default__inplace_mod__with__mod),
	/* .tp_inplace_shl = */ DEFIMPL(&default__inplace_shl__with__shl),
	/* .tp_inplace_shr = */ DEFIMPL(&default__inplace_shr__with__shr),
	/* .tp_inplace_and = */ DEFIMPL(&default__inplace_and__with__and),
	/* .tp_inplace_or  = */ DEFIMPL(&default__inplace_or__with__or),
	/* .tp_inplace_xor = */ DEFIMPL(&default__inplace_xor__with__xor),
	/* .tp_inplace_pow = */ DEFIMPL(&default__inplace_pow__with__pow),
};


PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
bool_compare_eq(DeeObject *self, DeeObject *other) {
	int error;
	if (DeeBool_Check(other))
		return self == other ? 0 : 1;
	error = DeeObject_Bool(other);
	if unlikely(error < 0)
		goto err;
	return !!DeeBool_IsTrue(self) == !!error;
err:
	return Dee_COMPARE_ERR;
}

PRIVATE WUNUSED NONNULL((1, 2)) int DCALL
bool_compare(DeeObject *self, DeeObject *other) {
	int error;
	if (DeeBool_Check(other))
		Dee_return_compare(self, other);
	error = DeeObject_Bool(other);
	if unlikely(error < 0)
		goto err;
	Dee_return_compare(!!DeeBool_IsTrue(self), !!error);
err:
	return Dee_COMPARE_ERR;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
bool_eq(DeeObject *self, DeeObject *other) {
	int error = DeeObject_Bool(other);
	if unlikely(error < 0)
		goto err;
	return_bool(DeeBool_IsTrue(self) == !!error);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
bool_ne(DeeObject *self, DeeObject *other) {
	int error = DeeObject_Bool(other);
	if unlikely(error < 0)
		goto err;
	return_bool(DeeBool_IsTrue(self) != !!error);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
bool_lo(DeeObject *self, DeeObject *other) {
	if (DeeBool_IsTrue(self))
		return_false; /* true < X --> false */
	return DeeObject_BoolOb(other); /* false < X --> X */
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
bool_le(DeeObject *self, DeeObject *other) {
	if (!DeeBool_IsTrue(self))
		return_true; /* false <= X --> true */
	return DeeObject_BoolOb(other); /* true <= X --> X */
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
bool_gr(DeeObject *self, DeeObject *other) {
	int other_bool;
	if (!DeeBool_IsTrue(self))
		return_false; /* false > X --> false */
	other_bool = DeeObject_Bool(other); /* true > X --> !X */
	if unlikely(other_bool < 0)
		goto err;
	return_bool(!other_bool);
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
	return_bool(!error); /* false >= X --> !X */
err:
	return NULL;
}

PRIVATE struct type_cmp bool_cmp = {
	/* .tp_hash          = */ bool_hash_PTR,
	/* .tp_compare_eq    = */ &bool_compare_eq,
	/* .tp_compare       = */ &bool_compare,
	/* .tp_trycompare_eq = */ DEFIMPL(&default__trycompare_eq__with__compare_eq),
	/* .tp_eq            = */ &bool_eq,
	/* .tp_ne            = */ &bool_ne,
	/* .tp_lo            = */ &bool_lo,
	/* .tp_le            = */ &bool_le,
	/* .tp_gr            = */ &bool_gr,
	/* .tp_ge            = */ &bool_ge,
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
	/* .tp_flags    = */ TP_FNORMAL | TP_FVARIABLE | TP_FFINAL | TP_FTRUNCATE | TP_FNAMEOBJECT | TP_FDEEPIMMUTABLE,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeNumeric_Type,
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_VAR(
			/* tp_ctor:        */ &bool_return_false,
			/* tp_copy_ctor:   */ &DeeObject_NewRef,
			/* tp_deep_ctor:   */ &DeeObject_NewRef,
			/* tp_any_ctor:    */ &bool_new,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ NULL, /* Static singleton, so no serial needed */
			/* tp_free:        */ NULL
		),
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str       = */ &bool_str,
		/* .tp_repr      = */ &bool_str,
		/* .tp_bool      = */ &bool_bool,
		/* .tp_print     = */ &bool_print,
		/* .tp_printrepr = */ &bool_print,
	},
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ &bool_math,
	/* .tp_cmp           = */ &bool_cmp,
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__1806D264FE42CE33),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL,
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ bool_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ bool_class_members,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
	/* .tp_mro           = */ NULL,
	/* .tp_operators     = */ bool_operators,
	/* .tp_operators_size= */ COMPILER_LENOF(bool_operators),
};

PUBLIC DeeBoolObject Dee_FalseTrue[2] = {
	{ OBJECT_HEAD_INIT(&DeeBool_Type) }, /* `false' */
	{ OBJECT_HEAD_INIT(&DeeBool_Type) }  /* `true' */
};


DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_BOOL_C */
