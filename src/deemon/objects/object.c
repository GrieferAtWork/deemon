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
#ifndef GUARD_DEEMON_OBJECTS_OBJECT_C
#define GUARD_DEEMON_OBJECTS_OBJECT_C 1

#include <deemon/api.h>

#include <deemon/alloc.h>              /* Dee_TYPE_CONSTRUCTOR_INIT_FIXED_S */
#include <deemon/arg.h>                /* DeeArg_BadArgc0, DeeArg_Unpack* */
#include <deemon/attribute.h>          /* DeeEnumAttr_Type */
#include <deemon/bool.h>               /* DeeBool_Type, Dee_return_bool01, return_bool */
#include <deemon/computed-operators.h> /* DEFAULT_OPIMP, DEFIMPL, DEFIMPL_UNSUPPORTED */
#include <deemon/error-rt.h>           /* DeeRT_ErrTUnboundAttr, DeeRT_ErrTUnboundAttrCStr */
#include <deemon/error.h>              /* DeeError_* */
#include <deemon/float.h>              /* DeeFloat_Type */
#include <deemon/format.h>             /* PRFuSIZ */
#include <deemon/int.h>                /* DeeInt_* */
#include <deemon/module.h>             /* DeeModule* */
#include <deemon/none-operator.h>      /* DeeNone_Operator* */
#include <deemon/none.h>               /* DeeNone_Check, return_none */
#include <deemon/object.h>             /* DREF, DeeObject, DeeObject_*, DeeTypeObject, DeeType_Extends, Dee_AsObject, Dee_BOUND_NO, Dee_BOUND_YES, Dee_Decref*, Dee_HAS_ISERR, Dee_Incref, Dee_TYPE, Dee_formatprinter_t, Dee_ssize_t, ITER_DONE, OBJECT_HEAD_INIT, return_reference, return_reference_ */
#include <deemon/objmethod.h>          /*  */
#include <deemon/string.h>             /* DeeString*, Dee_UNICODE_PRINTER_INIT, Dee_unicode_printer*, WSTR_LENGTH */
#include <deemon/super.h>              /* DeeSuper* */
#include <deemon/tuple.h>              /* DeeTuple_Type, Dee_EmptyTuple */
#include <deemon/type.h>               /* DeeObject_IsDeepImmutable, DeeType_*, Dee_TYPE_CONSTRUCTOR_INIT_FIXED_S, METHOD_FCONSTCALL, METHOD_FNOREFESCAPE, OPERATOR_*, STRUCT_*, TF_NONE, TP_F*, TYPE_*, type_* */
#include <deemon/util/hash.h>          /* DeeObject_Id */

#include "../runtime/runtime_error.h"
#include "../runtime/strings.h"

#include <stdbool.h> /* bool, false, true */
#include <stddef.h>  /* NULL, offsetof, size_t */

#undef container_of
#define container_of COMPILER_CONTAINER_OF

DECL_BEGIN

#ifndef NDEBUG
PRIVATE void DCALL
assert_badobject_impl(char const *check_name,
                      char const *file,
                      int line, DeeObject const *ob) {
	if (!ob) {
		_DeeAssert_Failf(check_name, file, line,
		                 "Bad object at %p is a NULL pointer",
		                 ob);
	} else if (!ob->ob_refcnt) {
		char const *type_name = "?";
		if (DeeObject_Check(ob->ob_type))
			type_name = ob->ob_type->tp_name;
		_DeeAssert_Failf(check_name, file, line,
		                 "Bad object at %p (instance of '%s') has a reference count of 0",
		                 ob, type_name);
	} else if (!ob->ob_type) {
		_DeeAssert_Failf(check_name, file, line,
		                 "Bad object at %p (%" PRFuSIZ " references) has a NULL-pointer as type",
		                 ob, ob->ob_refcnt);
	} else if (!ob->ob_type->ob_refcnt) {
		_DeeAssert_Failf(check_name, file, line,
		                 "Bad object at %p (instance of '%s', %" PRFuSIZ " references) has a type with a reference counter of 0",
		                 ob, ob->ob_type->tp_name, ob->ob_refcnt);
	} else {
		char const *type_name = "?";
		if (DeeObject_Check(ob->ob_type))
			type_name = ob->ob_type->tp_name;
		_DeeAssert_Failf(check_name, file, line,
		                 "Bad object at %p (instance of '%s', %" PRFuSIZ " references)",
		                 ob, type_name, ob->ob_refcnt);
	}
}

PRIVATE void DCALL
assert_badtype_impl(char const *check_name, char const *file,
                    int line, DeeObject const *ob, bool wanted_exact,
                    DeeTypeObject const *wanted_type) {
	char const *is_exact = wanted_exact ? " an exact " : " an ";
	if (!ob) {
		_DeeAssert_Failf(check_name, file, line,
		                 "Bad object at %p is a NULL pointer when%sinstance of '%s' was needed",
		                 ob, is_exact, wanted_type->tp_name);
	} else if (!ob->ob_refcnt) {
		char const *type_name = "?";
		if (DeeObject_Check(ob->ob_type))
			type_name = ob->ob_type->tp_name;
		_DeeAssert_Failf(check_name, file, line,
		                 "Bad object at %p (instance of '%s') has a reference "
		                 "count of 0 when%sinstance of '%s' was needed",
		                 ob, type_name, is_exact, wanted_type->tp_name);
	} else if (!ob->ob_type) {
		_DeeAssert_Failf(check_name, file, line,
		                 "Bad object at %p (%" PRFuSIZ " references) has a NULL-pointer "
		                 "as type when%sinstance of '%s' was needed",
		                 ob, ob->ob_refcnt, is_exact, wanted_type->tp_name);
	} else if (!ob->ob_type->ob_refcnt) {
		_DeeAssert_Failf(check_name, file, line,
		                 "Bad object at %p (instance of '%s', %" PRFuSIZ " references) has a type "
		                 "with a reference counter of 0 when%sinstance of '%s' was needed",
		                 ob, ob->ob_type->tp_name, ob->ob_refcnt, is_exact, wanted_type->tp_name);
	} else {
		bool include_obj_repr = false;
		char const *type_name = "?";
		if (DeeObject_Check(ob->ob_type)) {
			type_name = ob->ob_type->tp_name;
			if (ob->ob_type == &DeeString_Type || ob->ob_type == &DeeInt_Type ||
			    ob->ob_type == &DeeBool_Type || ob->ob_type == &DeeFloat_Type)
				include_obj_repr = true;
		}
		if (include_obj_repr) {
			_DeeAssert_Failf(check_name, file, line,
			                 "Bad object at %p (instance of '%s', %" PRFuSIZ " references) "
			                 "when%sinstance of '%s' was needed\n"
			                 "repr: %r",
			                 ob, type_name, ob->ob_refcnt, is_exact,
			                 wanted_type->tp_name, ob);
		} else {
			_DeeAssert_Failf(check_name, file, line,
			                 "Bad object at %p (instance of '%s', %" PRFuSIZ " references) "
			                 "when%sinstance of '%s' was needed",
			                 ob, type_name, ob->ob_refcnt, is_exact, wanted_type->tp_name);
		}
	}
}

PUBLIC void DCALL
DeeAssert_BadObject(DeeObject const *ob, char const *file, int line) {
	assert_badobject_impl("ASSERT_OBJECT", file, line, ob);
}

PUBLIC void DCALL
DeeAssert_BadObjectOpt(DeeObject const *ob, char const *file, int line) {
	assert_badobject_impl("ASSERT_OBJECT_OPT", file, line, ob);
}

PUBLIC void DCALL
DeeAssert_BadObjectType(DeeObject const *ob,
                        DeeTypeObject const *wanted_type,
                        char const *file, int line) {
	assert_badtype_impl("ASSERT_OBJECT_TYPE", file, line, ob, false, wanted_type);
}

PUBLIC void DCALL
DeeAssert_BadObjectTypeOpt(DeeObject const *ob,
                           DeeTypeObject const *wanted_type,
                           char const *file, int line) {
	assert_badtype_impl("ASSERT_OBJECT_TYPE_OPT", file, line, ob, false, wanted_type);
}

PUBLIC void DCALL
DeeAssert_BadObjectTypeExact(DeeObject const *ob,
                             DeeTypeObject const *wanted_type,
                             char const *file, int line) {
	assert_badtype_impl("ASSERT_OBJECT_TYPE_EXACT", file, line, ob, true, wanted_type);
}

PUBLIC void DCALL
DeeAssert_BadObjectTypeExactOpt(DeeObject const *ob,
                                DeeTypeObject const *wanted_type,
                                char const *file, int line) {
	assert_badtype_impl("ASSERT_OBJECT_TYPE_EXACT_OPT", file, line, ob, true, wanted_type);
}

#else /* !NDEBUG */

PUBLIC void DCALL
DeeAssert_BadObject(DeeObject const *ob,
                    char const *file, int line) {
	(void)ob;
	(void)file;
	(void)line;
	COMPILER_IMPURE();
	/* no-op */
}

PUBLIC void DCALL
DeeAssert_BadObjectOpt(DeeObject const *ob,
                       char const *file, int line) {
	(void)ob;
	(void)file;
	(void)line;
	COMPILER_IMPURE();
	/* no-op */
}

PUBLIC void DCALL
DeeAssert_BadObjectType(DeeObject const *ob,
                        DeeTypeObject const *wanted_type,
                        char const *file, int line) {
	(void)ob;
	(void)wanted_type;
	(void)file;
	(void)line;
	COMPILER_IMPURE();
	/* no-op */
}

PUBLIC void DCALL
DeeAssert_BadObjectTypeOpt(DeeObject const *ob,
                           DeeTypeObject const *wanted_type,
                           char const *file, int line) {
	(void)ob;
	(void)wanted_type;
	(void)file;
	(void)line;
	COMPILER_IMPURE();
	/* no-op */
}

PUBLIC void DCALL
DeeAssert_BadObjectTypeExact(DeeObject const *ob,
                             DeeTypeObject const *wanted_type,
                             char const *file, int line) {
	(void)ob;
	(void)wanted_type;
	(void)file;
	(void)line;
	COMPILER_IMPURE();
	/* no-op */
}

PUBLIC void DCALL
DeeAssert_BadObjectTypeExactOpt(DeeObject const *ob,
                                DeeTypeObject const *wanted_type,
                                char const *file, int line) {
	(void)ob;
	(void)wanted_type;
	(void)file;
	(void)line;
	COMPILER_IMPURE();
	/* no-op */
}

#endif /* NDEBUG */


PRIVATE WUNUSED NONNULL((1)) int DCALL
object_any_ctor(DeeObject *__restrict UNUSED(self),
                size_t argc, DeeObject *const *argv) {
	(void)argv;
	if unlikely(argc != 0)
		return DeeArg_BadArgc0("Object", argc);
	return 0;
}

DEFAULT_OPIMP WUNUSED NONNULL((1)) DREF DeeObject *DCALL
object_str(DeeObject *__restrict self) {
#if 1
	DeeTypeObject *tp_self = Dee_TYPE(self);
	if (tp_self->tp_name) {
		if (tp_self->tp_flags & TP_FNAMEOBJECT) {
			DREF DeeStringObject *result;
			result = container_of(tp_self->tp_name, DeeStringObject, s_str);
			Dee_Incref(result);
			return Dee_AsObject(result);
		}
		return DeeString_New(tp_self->tp_name);
	}
	return_reference(&str_Object);
#else
	if (Dee_TYPE(self) != &DeeObject_Type)
		goto err_noimp;
	return_reference(&str_Object);
err_noimp:
	err_unimplemented_operator(Dee_TYPE(self), OPERATOR_STR);
	return NULL;
#endif
}

DEFAULT_OPIMP WUNUSED NONNULL((1)) DREF DeeObject *DCALL
object_repr(DeeObject *__restrict self) {
	if (Dee_TYPE(self) == &DeeObject_Type)
		return DeeString_New("Object()");
	err_unimplemented_operator(Dee_TYPE(self), OPERATOR_REPR);
	return NULL;
}


/* Object operators through methods. */
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
object_sizeof(DeeObject *self) {
	DeeTypeObject *type;
	size_t instance_size;

	/* Individual sub-types should override this function and add the proper value.
	 * This implementation is merely used for any generic fixed-length type that
	 * doesn't do any custom heap allocations. */
	type = Dee_TYPE(self);

	/* Variable types lack a standardized way of determining their size in bytes. */
	if unlikely(DeeType_IsVariable(type))
		goto err_isvar;
	instance_size = DeeType_GetInstanceSize(type);
	if unlikely(instance_size == 0)
		goto err_iscustom;
	return DeeInt_NewSize(instance_size);
err_iscustom:
	DeeError_Throwf(&DeeError_TypeError,
	                "Cannot determine size of Type `%k' with custom allocator",
	                type);
	goto err;
err_isvar:
	DeeError_Throwf(&DeeError_TypeError,
	                "Cannot determine size of variable-length Type `%k'",
	                type);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
object_copy(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeArg_Unpack0(err, argc, argv, "__copy__");
	return DeeObject_Copy(self);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
object_deepcopy(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeArg_Unpack0(err, argc, argv, "__deepcopy__");
	return DeeObject_DeepCopy(self);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
object_assign(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *other;
	DeeArg_Unpack1(err, argc, argv, "__assign__", &other);
	if (DeeObject_Assign(self, other))
		goto err;
	return_reference_(self);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
object_moveassign(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *other;
	DeeArg_Unpack1(err, argc, argv, "__moveassign__", &other);
	if (DeeObject_AssertType(other, Dee_TYPE(self)))
		goto err;
	if (DeeObject_MoveAssign(self, other))
		goto err;
	return_reference_(self);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
object_dostr(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeArg_Unpack0(err, argc, argv, "__str__");
	return DeeObject_Str(self);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
object_dorepr(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeArg_Unpack0(err, argc, argv, "__repr__");
	return DeeObject_Repr(self);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
object_bool(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeArg_Unpack0(err, argc, argv, "__bool__");
	return DeeObject_BoolOb(self);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
object_call(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *args_tuple;
	DeeArg_Unpack1(err, argc, argv, "__call__", &args_tuple);
	if (DeeObject_AssertTypeExact(args_tuple, &DeeTuple_Type))
		goto err;
	return DeeObject_CallTuple(self, args_tuple);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
object_thiscall(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *this_arg;
	DeeObject *args_tuple = Dee_EmptyTuple;
	DeeArg_Unpack1Or2(err, argc, argv, "__thiscall__", &this_arg, &args_tuple);
	if (DeeObject_AssertTypeExact(args_tuple, &DeeTuple_Type))
		goto err;
	return DeeObject_ThisCallTuple(self, this_arg, args_tuple);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
object_hash(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeArg_Unpack0(err, argc, argv, "__hash__");
	return DeeInt_NewSize(DeeObject_Hash(self));
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
object_int(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeArg_Unpack0(err, argc, argv, "__int__");
	return DeeObject_Int(self);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
object_inv(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeArg_Unpack0(err, argc, argv, "__inv__");
	return DeeObject_Inv(self);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
object_pos(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeArg_Unpack0(err, argc, argv, "__pos__");
	return DeeObject_Pos(self);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
object_neg(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeArg_Unpack0(err, argc, argv, "__neg__");
	return DeeObject_Neg(self);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
object_add(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *other;
	DeeArg_Unpack1(err, argc, argv, "__add__", &other);
	return DeeObject_Add(self, other);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
object_sub(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *other;
	DeeArg_Unpack1(err, argc, argv, "__sub__", &other);
	return DeeObject_Sub(self, other);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
object_mul(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *other;
	DeeArg_Unpack1(err, argc, argv, "__mul__", &other);
	return DeeObject_Mul(self, other);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
object_div(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *other;
	DeeArg_Unpack1(err, argc, argv, "__div__", &other);
	return DeeObject_Div(self, other);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
object_mod(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *other;
	DeeArg_Unpack1(err, argc, argv, "__mod__", &other);
	return DeeObject_Mod(self, other);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
object_shl(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *other;
	DeeArg_Unpack1(err, argc, argv, "__shl__", &other);
	return DeeObject_Shl(self, other);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
object_shr(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *other;
	DeeArg_Unpack1(err, argc, argv, "__shr__", &other);
	return DeeObject_Shr(self, other);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
object_and(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *other;
	DeeArg_Unpack1(err, argc, argv, "__and__", &other);
	return DeeObject_And(self, other);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
object_or(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *other;
	DeeArg_Unpack1(err, argc, argv, "__or__", &other);
	return DeeObject_Or(self, other);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
object_xor(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *other;
	DeeArg_Unpack1(err, argc, argv, "__xor__", &other);
	return DeeObject_Xor(self, other);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
object_pow(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *other;
	DeeArg_Unpack1(err, argc, argv, "__pow__", &other);
	return DeeObject_Pow(self, other);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
object_eq(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *other;
	DeeArg_Unpack1(err, argc, argv, "__eq__", &other);
	return DeeObject_CmpEq(self, other);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
object_ne(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *other;
	DeeArg_Unpack1(err, argc, argv, "__ne__", &other);
	return DeeObject_CmpNe(self, other);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
object_lo(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *other;
	DeeArg_Unpack1(err, argc, argv, "__lo__", &other);
	return DeeObject_CmpLo(self, other);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
object_le(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *other;
	DeeArg_Unpack1(err, argc, argv, "__le__", &other);
	return DeeObject_CmpLe(self, other);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
object_gr(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *other;
	DeeArg_Unpack1(err, argc, argv, "__gr__", &other);
	return DeeObject_CmpGr(self, other);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
object_ge(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *other;
	DeeArg_Unpack1(err, argc, argv, "__ge__", &other);
	return DeeObject_CmpGe(self, other);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
object_size(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeArg_Unpack0(err, argc, argv, "__size__");
	return DeeObject_SizeOb(self);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
object_contains(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *other;
	DeeArg_Unpack1(err, argc, argv, "__contains__", &other);
	return DeeObject_Contains(self, other);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
object_getitem(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *other;
	DeeArg_Unpack1(err, argc, argv, "__getitem__", &other);
	return DeeObject_GetItem(self, other);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
object_delitem(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *other;
	DeeArg_Unpack1(err, argc, argv, "__delitem__", &other);
	if (DeeObject_DelItem(self, other))
		goto err;
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
object_setitem(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *other, *value;
	DeeArg_Unpack2(err, argc, argv, "__setitem__", &other, &value);
	if (DeeObject_SetItem(self, other, value))
		goto err;
	return_reference_(value);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
object_getrange(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *begin_index, *end_index;
	DeeArg_Unpack2(err, argc, argv, "__getrange__", &begin_index, &end_index);
	return DeeObject_GetRange(self, begin_index, end_index);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
object_delrange(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *begin_index, *end_index;
	DeeArg_Unpack2(err, argc, argv, "__delrange__", &begin_index, &end_index);
	if (DeeObject_DelRange(self, begin_index, end_index))
		goto err;
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
object_setrange(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *begin_index, *end_index, *value;
	DeeArg_Unpack3(err, argc, argv, "__setrange__", &begin_index, &end_index, &value);
	if (DeeObject_SetRange(self, begin_index, end_index, value))
		goto err;
	return_reference_(value);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
object_iterself(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeArg_Unpack0(err, argc, argv, "__iter__");
	return DeeObject_Iter(self);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
object_iternext(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DREF DeeObject *result;
	DeeArg_Unpack0(err, argc, argv, "__next__");
	result = DeeObject_IterNext(self);
	if (result == ITER_DONE) {
		DeeError_Throw(&DeeError_StopIteration_instance);
		result = NULL;
	}
	return result;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
object_getattr(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *name;
	DeeArg_Unpack1(err, argc, argv, "__getattr__", &name);
	if (DeeObject_AssertTypeExact(name, &DeeString_Type))
		goto err;
	return DeeObject_GetAttr(self, name);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
object_callattr(DeeObject *self, size_t argc, DeeObject *const *argv) {
	if unlikely(!argc)
		goto err_badargc;
	if (DeeObject_AssertTypeExact(argv[0], &DeeString_Type))
		goto err;
	return DeeObject_CallAttr(self, argv[0], argc - 1, argv + 1);
err_badargc:
	err_invalid_argc_va("__callattr__", argc, 1);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
object_hasattr(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *name;
	int result;
	DeeArg_Unpack1(err, argc, argv, "__hasattr__", &name);
	if (DeeObject_AssertTypeExact(name, &DeeString_Type))
		goto err;
	result = DeeObject_HasAttr(self, name);
	if unlikely(Dee_HAS_ISERR(result))
		goto err;
	return_bool(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
object_delattr(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *name;
	DeeArg_Unpack1(err, argc, argv, "__delattr__", &name);
	if (DeeObject_AssertTypeExact(name, &DeeString_Type))
		goto err;
	if (DeeObject_DelAttr(self, name))
		goto err;
	return_none;
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
object_setattr(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *name, *value;
	DeeArg_Unpack2(err, argc, argv, "__setattr__", &name, &value);
	if (DeeObject_AssertTypeExact(name, &DeeString_Type))
		goto err;
	if (DeeObject_SetAttr(self, name, value))
		goto err;
	return_reference_(value);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
object_enumattr(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeArg_Unpack0(err, argc, argv, "__enumattr__");
	return DeeObject_New(&DeeEnumAttr_Type, 1, (DeeObject **)&self);
err:
	return NULL;
}

INTDEF WUNUSED NONNULL((1, 2, 4)) Dee_ssize_t DCALL
object_format_generic(DeeObject *__restrict self,
                      Dee_formatprinter_t printer, void *arg,
                      /*utf-8*/ char const *__restrict format_str,
                      size_t format_len);

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
object_format_method(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DeeObject *format_str;
	char const *format_utf8;
	DeeArg_Unpack1(err, argc, argv, "__format__", &format_str);
	if (DeeObject_AssertTypeExact(format_str, &DeeString_Type))
		goto err;
	if unlikely((format_utf8 = DeeString_AsUtf8(format_str)) == NULL)
		goto err;
	{
		Dee_ssize_t error;
		struct Dee_unicode_printer printer = Dee_UNICODE_PRINTER_INIT;
		error = object_format_generic(self,
		                              &Dee_unicode_printer_print,
		                              &printer, format_utf8,
		                              WSTR_LENGTH(format_utf8));
		if unlikely(error < 0)
			goto err_printer;
		return Dee_unicode_printer_pack(&printer);
err_printer:
		Dee_unicode_printer_fini(&printer);
	}
err:
	return NULL;
}

#ifndef CONFIG_NO_DEEMON_100_COMPAT
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
object_not(DeeObject *self, size_t argc, DeeObject *const *argv) {
	int temp;
	DeeArg_Unpack0(err, argc, argv, "__not__");
	temp = DeeObject_Bool(self);
	if unlikely(temp < 0)
		goto err;
	return_bool(!temp);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
object_is(DeeObject *self, size_t argc, DeeObject *const *argv) {
	unsigned int is_instance;
	DeeTypeObject *tp;
	DeeArg_Unpack1(err, argc, argv, "__is__", &tp);
	if (DeeNone_Check((DeeObject *)tp)) {
		is_instance = DeeNone_Check(self) ? 1 : 0;
	} else if (DeeSuper_Check(self)) {
		is_instance = DeeType_Extends(DeeSuper_TYPE(self), tp);
	} else {
		is_instance = DeeObject_Implements(self, tp);
	}
	Dee_return_bool01(is_instance);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
object_inc(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DREF DeeObject *selfref;
	int error;
	DeeArg_Unpack0(err, argc, argv, "__inc__");
	selfref = self;
	Dee_Incref(selfref);
	error = DeeObject_Inc(&selfref);
	if unlikely(error)
		goto err_selfref;
	error = DeeObject_Assign(self, selfref);
	if unlikely(error)
		goto err_selfref;
	return selfref;
err_selfref:
	Dee_Decref(selfref);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
object_dec(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DREF DeeObject *selfref;
	int error;
	DeeArg_Unpack0(err, argc, argv, "__dec__");
	selfref = self;
	Dee_Incref(selfref);
	error = DeeObject_Dec(&selfref);
	if unlikely(error)
		goto err_selfref;
	error = DeeObject_Assign(self, selfref);
	if unlikely(error)
		goto err_selfref;
	return selfref;
err_selfref:
	Dee_Decref(selfref);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
object_incpost(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DREF DeeObject *selfref, *result;
	int error;
	DeeArg_Unpack0(err, argc, argv, "__incpost__");
	result = DeeObject_Copy(self);
	if unlikely(!result)
		goto err;
	selfref = self;
	Dee_Incref(selfref);
	error = DeeObject_Inc(&selfref);
	if (likely(error == 0) && selfref != self)
		error = DeeObject_Assign(self, selfref);
	Dee_Decref(selfref);
	if unlikely(error)
		goto err_r;
	return result;
err_r:
	Dee_Decref_likely(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
object_decpost(DeeObject *self, size_t argc, DeeObject *const *argv) {
	DREF DeeObject *selfref, *result;
	int error;
	DeeArg_Unpack0(err, argc, argv, "__decpost__");
	result = DeeObject_Copy(self);
	if unlikely(!result)
		goto err;
	selfref = self;
	Dee_Incref(selfref);
	error = DeeObject_Dec(&selfref);
	if (likely(error == 0) && selfref != self)
		error = DeeObject_Assign(self, selfref);
	Dee_Decref(selfref);
	if unlikely(error)
		goto err_r;
	return result;
err_r:
	Dee_Decref_likely(result);
err:
	return NULL;
}

#define DEFINE_DEPRECATED_INPLACE_BINARY(name, func)              \
	PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL            \
	object_##name(DeeObject *self, size_t argc,                   \
	              DeeObject *const *argv) {                       \
		DREF DeeObject *selfref;                                  \
		int error;                                                \
		DeeObject *other;                                         \
		DeeArg_Unpack1(err, argc, argv, "__" #name "__", &other); \
		selfref = self;                                           \
		Dee_Incref(selfref);                                      \
		error = func(&selfref, other);                            \
		if unlikely(error)                                        \
			goto err_selfref;                                     \
		error = DeeObject_Assign(self, selfref);                  \
		if unlikely(error)                                        \
			goto err_selfref;                                     \
		return selfref;                                           \
	err_selfref:                                                  \
		Dee_Decref(selfref);                                      \
	err:                                                          \
		return NULL;                                              \
	}
DEFINE_DEPRECATED_INPLACE_BINARY(iadd, DeeObject_InplaceAdd)
DEFINE_DEPRECATED_INPLACE_BINARY(isub, DeeObject_InplaceSub)
DEFINE_DEPRECATED_INPLACE_BINARY(imul, DeeObject_InplaceMul)
DEFINE_DEPRECATED_INPLACE_BINARY(idiv, DeeObject_InplaceDiv)
DEFINE_DEPRECATED_INPLACE_BINARY(imod, DeeObject_InplaceMod)
DEFINE_DEPRECATED_INPLACE_BINARY(ishl, DeeObject_InplaceShl)
DEFINE_DEPRECATED_INPLACE_BINARY(ishr, DeeObject_InplaceShr)
DEFINE_DEPRECATED_INPLACE_BINARY(iand, DeeObject_InplaceAnd)
DEFINE_DEPRECATED_INPLACE_BINARY(ior, DeeObject_InplaceOr)
DEFINE_DEPRECATED_INPLACE_BINARY(ixor, DeeObject_InplaceXor)
DEFINE_DEPRECATED_INPLACE_BINARY(ipow, DeeObject_InplacePow)
#undef DEFINE_DEPRECATED_INPLACE_BINARY

#endif /* !CONFIG_NO_DEEMON_100_COMPAT */


PRIVATE struct type_method tpconst object_methods[] = {
	/* Operator invocation functions. */
	TYPE_METHOD("__copy__", &object_copy,
	            "->\n"
	            "#r{A copy of @this object}"),
	TYPE_METHOD("__deepcopy__", &object_deepcopy,
	            "->\n"
	            "#r{A deep copy of @this object}"),
	TYPE_METHOD("__assign__", &object_assign,
	            "(other)->\n"
	            "Assigns @other to @this and"),
	TYPE_METHOD("__moveassign__", &object_moveassign,
	            "(other)->\n"
	            "Move-assign @other to @this and"),
	TYPE_METHOD("__str__", &object_dostr,
	            "->?Dstring\n"
	            "#r{@this converted to a ?Dstring}"),
	TYPE_METHOD("__repr__", &object_dorepr,
	            "->?Dstring\n"
	            "#r{The ?Dstring representation of @this}"),
	TYPE_METHOD("__bool__", &object_bool,
	            "->?Dbool\n"
	            "#r{The ?Dbool value of @this}"),
	TYPE_METHOD("__call__", &object_call,
	            "(args:?DTuple)->\n"
	            "Call @this using the given @args ?DTuple"),
	TYPE_METHOD("__thiscall__", &object_thiscall,
	            "(this_arg,args:?DTuple)->\n"
	            "Do a this-call on @this using the given @this_arg and @args ?DTuple"),
	TYPE_METHOD("__hash__", &object_hash,
	            "->?Dint\n"
	            "#r{The hash-value of @this}"),
	TYPE_METHOD("__int__", &object_int,
	            "->?Dint\n"
	            "#r{The integer-value of @this}"),
	TYPE_METHOD("__inv__", &object_inv,
	            "->\n"
	            "#r{The result of ${this.operator ~ ()}}"),
	TYPE_METHOD("__pos__", &object_pos,
	            "->\n"
	            "#r{The result of ${this.operator + ()}}"),
	TYPE_METHOD("__neg__", &object_neg,
	            "->\n"
	            "#r{The result of ${this.operator - ()}}"),
	TYPE_METHOD("__add__", &object_add,
	            "(other)->\n"
	            "#r{The result of ${this.operator + (other)}}"),
	TYPE_METHOD("__sub__", &object_sub,
	            "(other)->\n"
	            "#r{The result of ${this.operator - (other)}}"),
	TYPE_METHOD("__mul__", &object_mul,
	            "(other)->\n"
	            "#r{The result of ${this.operator * (other)}}"),
	TYPE_METHOD("__div__", &object_div,
	            "(other)->\n"
	            "#r{The result of ${this.operator / (other)}}"),
	TYPE_METHOD("__mod__", &object_mod,
	            "(other)->\n"
	            "#r{The result of ${this.operator % (other)}}"),
	TYPE_METHOD("__shl__", &object_shl,
	            "(other)->\n"
	            "#r{The result of ${this.operator << (other)}}"),
	TYPE_METHOD("__shr__", &object_shr,
	            "(other)->\n"
	            "#r{The result of ${this.operator >> (other)}}"),
	TYPE_METHOD("__and__", &object_and,
	            "(other)->\n"
	            "#r{The result of ${this.operator & (other)}}"),
	TYPE_METHOD("__or__", &object_or,
	            "(other)->\n"
	            "#r{The result of ${this.operator | (other)}}"),
	TYPE_METHOD("__xor__", &object_xor,
	            "(other)->\n"
	            "#r{The result of ${this.operator ^ (other)}}"),
	TYPE_METHOD("__pow__", &object_pow,
	            "(other)->\n"
	            "#r{The result of ${this.operator ** (other)}}"),
	TYPE_METHOD("__eq__", &object_eq,
	            "(other)->\n"
	            "#r{The result of ${this.operator == (other)}}"),
	TYPE_METHOD("__ne__", &object_ne,
	            "(other)->\n"
	            "#r{The result of ${this.operator != (other)}}"),
	TYPE_METHOD("__lo__", &object_lo,
	            "(other)->\n"
	            "#r{The result of ${this.operator < (other)}}"),
	TYPE_METHOD("__le__", &object_le,
	            "(other)->\n"
	            "#r{The result of ${this.operator <= (other)}}"),
	TYPE_METHOD("__gr__", &object_gr,
	            "(other)->\n"
	            "#r{The result of ${this.operator > (other)}}"),
	TYPE_METHOD("__ge__", &object_ge,
	            "(other)->\n"
	            "#r{The result of ${this.operator >= (other)}}"),
	TYPE_METHOD("__size__", &object_size,
	            "->\n"
	            "#r{The result of ${this.operator ## ()}}"),
	TYPE_METHOD("__contains__", &object_contains,
	            "(item)->\n"
	            "#r{The result of ${this.operator contains (item)}}"),
	TYPE_METHOD("__getitem__", &object_getitem,
	            "(index)->\n"
	            "#r{The result of ${this.operator [] (index)}}"),
	TYPE_METHOD("__delitem__", &object_delitem,
	            "(index)\n"
	            "Invokes ${this.operator del[] (index)}"),
	TYPE_METHOD("__setitem__", &object_setitem,
	            "(index,value)->\n"
	            "#r{Always re-returned @value}\n"
	            "Invokes ${this.operator []= (index, value)}"),
	TYPE_METHOD("__getrange__", &object_getrange,
	            "(start,end)->\n"
	            "#r{The result of ${this.operator [:] (start, end)}}"),
	TYPE_METHOD("__delrange__", &object_delrange,
	            "(start,end)\n"
	            "Invokes ${this.operator del[:] (start, end)}"),
	TYPE_METHOD("__setrange__", &object_setrange,
	            "(start,end,value)->\n"
	            "#r{Always re-returned @value}\n"
	            "Invokes ${this.operator [:]= (start, end, value)}"),
	TYPE_METHOD("__iterself__", &object_iterself,
	            "->\n"
	            "#r{The result of ${this.operator iter()}}"),
	TYPE_METHOD("__iternext__", &object_iternext,
	            "->\n"
	            "#r{The result of ${this.operator next()}}"),
	TYPE_METHOD("__getattr__", &object_getattr,
	            "(name:?Dstring)->\n"
	            "#r{The result of ${this.operator . (name)}}"),
	TYPE_METHOD("__callattr__", &object_callattr,
	            "(name:?Dstring,args!)->\n"
	            "#r{The result of ${this.operator . (name)(args!)}}"),
	TYPE_METHOD("__hasattr__", &object_hasattr,
	            "(name:?Dstring)->?Dbool\n"
	            "Check if @this object provides an attribute @name, returning ?t or ?f indicative of this"),
	TYPE_METHOD("__delattr__", &object_delattr,
	            "(name:?Dstring)\n"
	            "Invokes ${this.operator del . (name)}"),
	TYPE_METHOD("__setattr__", &object_setattr,
	            "(name:?Dstring,value)\n"
	            "#r{Always re-returned @value}\n"
	            "Invokes ${this.operator .= (name, value)}"),
	TYPE_METHOD("__enumattr__", &object_enumattr,
	            "()->?S?DAttribute\n"
	            "#r{Same as ${deemon.enumattr(this)}}"),
	TYPE_METHOD("__format__", &object_format_method,
	            "(format:?Dstring)->?Dstring\n"
	            "Format @this object. (s.a. ?Aformat?Dstring)"),
#ifndef CONFIG_NO_DEEMON_100_COMPAT
	/* Aliases for backwards compatibility with deemon < v200 */
	TYPE_METHOD("__iterself__",    &object_iterself, "->\nDeprecated alias for ?#__iter__"),
	TYPE_METHOD("__iternext__",    &object_iternext, "->\nDeprecated alias for ?#__next__"),
	/* Deprecated function for backwards compatibility with deemon < v200 */
	TYPE_METHOD("__move__",        &object_copy, "->\nDeprecated alias for ?#__copy__"),
	TYPE_METHOD("__lt__",          &object_lo, "(other)->\nDeprecated alias for ?#__lo__"),
	TYPE_METHOD("__gt__",          &object_gr, "(other)->\nDeprecated alias for ?#__gr__"),
	TYPE_METHOD("__not__",         &object_not, "->?Dbool\nDeprecated alias for ${!this}"),
	TYPE_METHOD("__is__",          &object_is, "(tp:?DType)->?Dbool\n(tp:?N)->?Dbool\nDeprecated alias for ${this is tp}"),
	TYPE_METHOD("__deepequals__",  &object_eq, "(other)->\nDeprecated alias for ?#__eq__"),
	TYPE_METHOD("__inc__",         &object_inc, "->\nDeprecated alias for ${({ local temp = this; ++temp; if (temp !== this) this := temp; this; })}"),
	TYPE_METHOD("__dec__",         &object_dec, "->\nDeprecated alias for ${({ local temp = this; --temp; if (temp !== this) this := temp; this; })}"),
	TYPE_METHOD("__incpost__",     &object_incpost, "->\nDeprecated alias for ${({ local res = copy this; local temp = this; ++temp; if (temp !== this) this := temp; res; })}"),
	TYPE_METHOD("__decpost__",     &object_decpost, "->\nDeprecated alias for ${({ local res = copy this; local temp = this; --temp; if (temp !== this) this := temp; res; })}"),
	TYPE_METHOD("__iadd__",        &object_iadd, "(other)->\nDeprecated alias for ${({ local temp = this; temp += other; if (temp !== this) this := temp; this; })}"),
	TYPE_METHOD("__isub__",        &object_isub, "(other)->\nDeprecated alias for ${({ local temp = this; temp -= other; if (temp !== this) this := temp; this; })}"),
	TYPE_METHOD("__imul__",        &object_imul, "(other)->\nDeprecated alias for ${({ local temp = this; temp *= other; if (temp !== this) this := temp; this; })}"),
	TYPE_METHOD("__idiv__",        &object_idiv, "(other)->\nDeprecated alias for ${({ local temp = this; temp /= other; if (temp !== this) this := temp; this; })}"),
	TYPE_METHOD("__imod__",        &object_imod, "(other)->\nDeprecated alias for ${({ local temp = this; temp %= other; if (temp !== this) this := temp; this; })}"),
	TYPE_METHOD("__ishl__",        &object_ishl, "(other)->\nDeprecated alias for ${({ local temp = this; temp <<= other; if (temp !== this) this := temp; this; })}"),
	TYPE_METHOD("__ishr__",        &object_ishr, "(other)->\nDeprecated alias for ${({ local temp = this; temp >>= other; if (temp !== this) this := temp; this; })}"),
	TYPE_METHOD("__iand__",        &object_iand, "(other)->\nDeprecated alias for ${({ local temp = this; temp &= other; if (temp !== this) this := temp; this; })}"),
	TYPE_METHOD("__ior__",         &object_ior,  "(other)->\nDeprecated alias for ${({ local temp = this; temp |= other; if (temp !== this) this := temp; this; })}"),
	TYPE_METHOD("__ixor__",        &object_ixor, "(other)->\nDeprecated alias for ${({ local temp = this; temp ^= other; if (temp !== this) this := temp; this; })}"),
	TYPE_METHOD("__ipow__",        &object_ipow, "(other)->\nDeprecated alias for ${({ local temp = this; temp **= other; if (temp !== this) this := temp; this; })}"),
#endif /* !CONFIG_NO_DEEMON_100_COMPAT */
	TYPE_METHOD_END
};




PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
object_class_get(DeeObject *__restrict self) {
	return_reference((DeeObject *)DeeObject_Class(self));
}

INTERN WUNUSED NONNULL((1)) DREF DeeObject *DCALL
object_id_get(DeeObject *__restrict self) {
	return DeeInt_NewUIntptr(DeeObject_Id(self));
}

PRIVATE Dee_DEFINE_CLSPROPERTY(object_id_get_cobj, &DeeObject_Type, &object_id_get, NULL, NULL, NULL);
PRIVATE struct type_member tpconst object_class_members[] = {
	TYPE_MEMBER_CONST_DOC("id", &object_id_get_cobj,
	                      "Alias for ?#{i:id} to speed up expressions such as ${Object.id}"),
	TYPE_MEMBER_END
};


#ifdef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
PRIVATE WUNUSED NONNULL((1)) DREF DeeModuleObject *DCALL
object_get_module(DeeObject *__restrict self) {
	DREF DeeModuleObject *result = DeeModule_OfObject(self);
	if unlikely(!result)
		result = (DREF DeeModuleObject *)DeeRT_ErrTUnboundAttr(&DeeObject_Type, self, &str___module__);
	return result;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
object_bound_module(DeeObject *__restrict self) {
	DREF DeeModuleObject *result = DeeModule_OfObject(self);
	if (!result)
		return Dee_BOUND_NO;
	Dee_Decref_unlikely(result);
	return Dee_BOUND_YES;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeModuleObject *DCALL
object_get_true_module(DeeObject *__restrict self) {
	DREF DeeModuleObject *result = DeeModule_OfPointer(self);
	if unlikely(!result)
		result = (DREF DeeModuleObject *)DeeRT_ErrTUnboundAttrCStr(&DeeObject_Type, self, "__true_module__");
	return result;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
object_bound_true_module(DeeObject *__restrict self) {
	DREF DeeModuleObject *result = DeeModule_OfPointer(self);
	if (!result)
		return Dee_BOUND_NO;
	Dee_Decref_unlikely(result);
	return Dee_BOUND_YES;
}
#endif /* CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */


INTDEF WUNUSED NONNULL((1)) DREF DeeObject *DCALL
instance_get_itable(DeeObject *__restrict self);

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
object_get_deep_immutable(DeeObject *__restrict self) {
	return_bool(DeeObject_IsDeepImmutable(self));
}

/* Runtime-versions of compiler-intrinsic standard attributes. */
PRIVATE struct type_getset tpconst object_getsets[] = {
	TYPE_GETTER_AB_F(STR_this, &DeeObject_NewRef,
	                 METHOD_FCONSTCALL,
	                 "Always re-return @this object"),
	TYPE_GETTER_AB_F(STR_class, &object_class_get,
	                 METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                 "->?DType\n"
	                 "Returns the class of @this Type, which is usually identical to "
	                 /**/ "?#type, however in the case of a super-proxy, the viewed "
	                 /**/ "Type is returned, rather than the actual Type."),
	TYPE_GETTER_AB_F("super", &DeeSuper_Of,
	                 METHOD_FCONSTCALL,
	                 "->?DSuper\n"
	                 "Returns a view for the super-instance of @this object"),
	TYPE_GETTER_AB_F("__itable__", &instance_get_itable,
	                 METHOD_FCONSTCALL,
	                 "->?X2?S?O?AObjectTable?Ert:ClassDescriptor\n"
	                 "Returns an indexable sequence describing the instance object "
	                 /**/ "table, as referenced by ?Aaddr?AAttribute?Ert:{ClassDescriptor}.\n"
	                 "For non-user-defined classes (aka. when ${this.class.__isclass__} is ?f), "
	                 /**/ "an empty sequence is returned.\n"
	                 "The class-attribute table can be accessed through ?A__ctable__?DType."),
	TYPE_GETTER_AB_F("__deep_immutable__", &object_get_deep_immutable,
	                 METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                 "->?Dbool\n"
	                 "True if this object is deep immutable (meaning "
	                 /**/ "that $deepcopy would return re-return $this)"),
#ifdef CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES
	TYPE_GETTER_BOUND_F(STR___module__, &object_get_module, &object_bound_module,
	                    METHOD_FCONSTCALL,
	                    "->?DModule\n"
	                    "#tUnboundAttribute{Object is dynamically allocated, as "
	                    /*              */ "opposed to a constant within some module, "
	                    /*              */ "and also does not expose its module in some "
	                    /*              */ "known, type-specific manner}"
	                    "Returns the module whose address space this object resides within, "
	                    /**/ "or the module that may be embedded within @this object via some "
	                    /**/ "type-specific method (e.g. ?A__module__?DType or ?A__module__?Ert:Code). "
	                    /**/ "Similar functionality is also provided by ?#__true_module__, which does "
	                    /**/ "the same as this property, but #Bonly looks at @this object's address "
	                    /**/ "to try and find the containing module.\n"
	                    "If @this is dynamically allocated, and does not expose a bound module "
	                    /**/ "via some type-specific method, :UnboundAttribute is thrown\n"
	                    "Be careful when relying on this property for the result of expressions "
	                    /**/ "that may or may not get evaluated at compile-time:\n"
	                    "${"
	                    /**/ "print \"foo\".__module__;           /* Always works: same as `import(\".\")' */\n"
	                    /**/ "print (\"foo\" + \"bar\").__module__; /* Only works when compiled with `-O3' */\n"
	                    "}\n"
	                    "With this in mind, the address-based determination of a linked ?DModule "
	                    /**/ "only works for true compile-time constant expressions (or more specific: "
	                    /**/ "cases where @this object is embedded within the heap of a #C{.dec} file, "
	                    /**/ "or was statically defined within a #BDEX module or the deemon core).\n"
	                    "Also note that depending on how deemon was built, and whether or not there are "
	                    /**/ "any references to outside variables, lambda functions and/or user-defined "
	                    /**/ "classes may or may not be statically allocated and thus able to query their "
	                    /**/ "linked module based on their address. However, in these cases, there is usually "
	                    /**/ "a way to access the object's relevant compile-time constant portion (which "
	                    /**/ "will then #Balways be usable with this property, such as ?A__code__?DFunction "
	                    /**/ "to return the (usually compile-time constant) ?Ert:Code of a function, or "
	                    /**/ "?A__class__?DType to return the (usually compile-time constant) ?Ert:ClassDescriptor "
	                    /**/ "of a class. But note that behavior, presence, and typing of all of these "
	                    /**/ "attributes is implementation-specific, the same as this attribute ?#__module__ "
	                    /**/ "and its sibling ?#__true_module__ are, too"),
	TYPE_GETTER_BOUND_F("__true_module__", &object_get_true_module, &object_bound_true_module,
	                    METHOD_FCONSTCALL,
	                    "->?DModule\n"
	                    "#tUnboundAttribute{Object is dynamically allocated, as "
	                    /*              */ "opposed to a constant within some module}"
	                    "Similar to ?#__module__, but #Bonly look at @this object's address in order to "
	                    /**/ "try and return the module whose address space this object resides within. "
	                    /**/ "If @this is dynamically allocated, :UnboundAttribute is thrown\n"
	                    "For more information, see ?#__module__"),
#endif /* CONFIG_EXPERIMENTAL_MODULE_DIRECTORIES */

	/* Helper function: `foo.id' returns a unique id for any object. */
	TYPE_GETTER_AB_F("id", &object_id_get,
	                 /* This one isn't CONSTCALL because IDs can change if a constant
	                  * is serialized and deserialized (as would be the case when building
	                  * and later re-loading a .dec file). Even IDs of built-in constants
	                  * like "none", "true", "false", ... can differ if ASLR causes deemon
	                  * to be loaded to different addresses between multiple runs. */
	                 METHOD_FNOREFESCAPE,
	                 "->?Dint\n"
	                 "Returns a unique id identifying @this specific object instance"),

	/* Utility function: Return the size of a given object (in bytes) */
	TYPE_GETTER_AB_F("__sizeof__", &object_sizeof,
	                 METHOD_FCONSTCALL | METHOD_FNOREFESCAPE,
	                 "->?Dint\n"
	                 "Return the size of @this object in bytes."),
	TYPE_GETSET_END
};

PRIVATE struct type_member tpconst object_members[] = {
	TYPE_MEMBER_FIELD_DOC("type", STRUCT_OBJECT_AB, offsetof(DeeObject, ob_type),
	                      "->?DType\n"
	                      "The type of @this object (same as ${type this})"),
	TYPE_MEMBER_FIELD_DOC("__refcnt__", STRUCT_CONST | STRUCT_SIZE_T, offsetof(DeeObject, ob_refcnt),
	                      "->?Dint\n"
	                      "The number of references currently existing for @this object"),
	TYPE_MEMBER_END
};

PRIVATE struct type_operator const object_operators[] = {
	TYPE_OPERATOR_FLAGS(OPERATOR_0000_CONSTRUCTOR, METHOD_FCONSTCALL),
	TYPE_OPERATOR_FLAGS(OPERATOR_0006_STR, METHOD_FCONSTCALL | METHOD_FNOREFESCAPE),
	TYPE_OPERATOR_FLAGS(OPERATOR_0007_REPR, METHOD_FCONSTCALL | METHOD_FNOREFESCAPE),
};

PUBLIC DeeTypeObject DeeObject_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ DeeString_STR(&str_Object),
	/* .tp_doc      = */ DOC("The base class of all regular objects\n"
	                         "\n"

	                         "()\n"
	                         "Construct a new object (no-op constructor)\n"
	                         "\n"

	                         "str->\n"
	                         "Returns the name of the object's Type\n"
	                         "${"
	                         /**/ "operator str(): string {\n"
	                         /**/ "	return str type this;\n"
	                         /**/ "}"
	                         "}\n"

	                         "repr->\n"
	                         "Returns $\"Object()\" if this is an exact instance of ?.\n"
	                         "${"
	                         /**/ "operator repr(): string {\n"
	                         /**/ "	if (type(this) === Object)\n"
	                         /**/ "		return \"Object()\";\n"
	                         /**/ "	throw Error.RuntimeError.NotImplemented();\n"
	                         /**/ "}"
	                         "}"),
	/* .tp_flags    = */ TP_FNORMAL | TP_FNAMEOBJECT | TP_FABSTRACT,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ NULL,    /* No base */
	/* .tp_init = */ {
		Dee_TYPE_CONSTRUCTOR_INIT_ALLOC_AUTO(
			/* T:              */ DeeObject,
			/* tp_ctor:        */ &DeeNone_OperatorCtor,
			/* tp_copy_ctor:   */ &DeeNone_OperatorCopy,
			/* tp_any_ctor:    */ &object_any_ctor,
			/* tp_any_ctor_kw: */ NULL,
			/* tp_serialize:   */ &DeeNone_OperatorSerialize
		),
		/* .tp_dtor        = */ NULL,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL,
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&object_str,
		/* .tp_repr = */ (DREF DeeObject *(DCALL *)(DeeObject *__restrict))&object_repr,
		/* .tp_bool = */ DEFIMPL_UNSUPPORTED(&default__bool__unsupported),
		/* .tp_print     = */ DEFIMPL(&default__print__with__str),
		/* .tp_printrepr = */ DEFIMPL(&default__printrepr__with__repr),
	},
	/* .tp_visit         = */ NULL,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ DEFIMPL_UNSUPPORTED(&default__tp_math__AE7A38D3B0C75E4B),
	/* .tp_cmp           = */ DEFIMPL_UNSUPPORTED(&default__tp_cmp__FA8008618F75C42A),
	/* .tp_seq           = */ DEFIMPL_UNSUPPORTED(&default__tp_seq__A0A5A432B5FA58F3),
	/* .tp_iter_next     = */ DEFIMPL_UNSUPPORTED(&default__iter_next__unsupported),
	/* .tp_iterator      = */ DEFIMPL_UNSUPPORTED(&default__tp_iterator__C6F8E138F179B5AD),
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ DEFIMPL_UNSUPPORTED(&default__tp_with__0476D7EDEFD2E7B7),
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ object_methods,
	/* .tp_getsets       = */ object_getsets,
	/* .tp_members       = */ object_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ object_class_members,
	/* .tp_method_hints  = */ NULL,
	/* .tp_call          = */ DEFIMPL_UNSUPPORTED(&default__call__unsupported),
	/* .tp_callable      = */ DEFIMPL_UNSUPPORTED(&default__tp_callable__EC3FFC1C149A47D0),
	/* .tp_mro           = */ NULL,
	/* .tp_operators     = */ object_operators,
	/* .tp_operators_size= */ COMPILER_LENOF(object_operators),
};

DECL_END

#endif /* !GUARD_DEEMON_OBJECTS_OBJECT_C */
